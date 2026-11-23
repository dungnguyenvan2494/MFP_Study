//------------------------------------------------------------------------------
/**
 * @file pcgact_pA072002.cpp
 * @brief 画面クラスの実装（ベース画面）. 
 */
//------------------------------------------------------------------------------
 
//================================================================================================
//  インクルード・ファイル. 
//================================================================================================
#include "panel/plibh/plibh_CLCD/plibh_Mtui.h"
#include "panel/plibh/plibh_CLCD2/PNLC_UICustomDataManager.h"
#include <panel/pcgact/pcgact_CLCD2/pcgact_pA072002.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA072002.h>
#include <panel/pcgact/pcgact_CLCD2/pipact_pA072002.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_base.h>
#include <panel/pcgstub/pcgstub_FUM5/pigdummy_popup.h>
#include <panel/pcgact/pcgact_CLCD2/pipact_base.h>
#include <panel/pcgact/pcgact_CLCD/pigact_CLCD.h>		// [CQ.359452].
#include <panel/pcgact/pcgact_CLCD/pigactenl_CLCD.h>
#include <panel/pcgjob/pcgjob_CLCD2/pigjob_pA061000.h>
#include <panel/pcgcom/pcgcom_CLCD2/PNLC_ChangeGamenManager.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA072000.h>
#include <panel/pcgcomponent/pcgcomponentg/PQuickButtonComponent.h>
#include <panel/pcgscanfax/pcgscanfax_CLCD2/pigscanfax_pA039110.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA072001.h>
#include <panel/pcgscanfax_ext/pcgscanfax_ext_CLCD2/pigscanfax_pA039000_ext.h>
#include <panel/plibh/plibh_CLCD2/PNLC_AuthenticationManager.h>
#include <panel/plibh/plibh_CLCD2/plibh_CLCD2.h>
#include <panel/pcgwarn_ext/pcgwarn_ext_CLCD2/pigwarn_pA0A3202_ext.h>
#include <panel/pcgwarn/pcgwarn_CLCD2/PWarningcomIF.h>		// [CQ.340570].
#include <panel/pcgcom_ext/pcgcom_extg/PCGC_AplCommonSet.h>
#include <panel/pcgcom_ext/pcgcom_extg/pigBrowserFileUploadComBase.h>
#include <panel/pcgact_ext/pcgact_ext_CLCD2/pigact_pA072002_ext.h>
#include <panel/pcgbrowser_ext/pcgbrowser_ext_CLCD2/pigbrowser_pA0R0100.h>
#include <panel/plibh/plibh_CLCD2/PNLC_ScanFaxDataChangeCheckLib.h>
#include <panel/pcgcom_ext/pcgcom_extg/PCGC_TreeScreen.h>
#include <panel/pcgcom_ext/pcgcom_extg/pigAppChangeBrowser.h>
#include <panel/pcgcom_ext/pcgcom_extg/PCGC_PublicPopupDummyBase.h>
#include <panel/pcgcom_ext/pcgcom_extg/PCGC_TreeScreen.h>
#include <panel/pcgwarn_ext/pcgwarn_ext_CLCD2/pigwarn_pA0A3703_ext.h> //【CQ #363216】【Restricted the condition for ZeusMLK stream and enabled in Minerva 1st】【HCL_Joseph】 2012/04/02 Add.
#include "../../../../divlib/Common/blkapi/blkshm.h"
#include "LAPC_APP_ActiveAppManager.h"
#include "LAPC_BFW_System.h"
#include "LAPC_APP_RegistAppDataList.h"
#include "LAPC_APP_RegistAppData.h"
#include <panel/plibh/plibh_CLCD2/PNLC_FuncPatternSettingLib.h>// comment 2012/07/26 OPS1 Sky村井 [CQ366985]パターンカスタマイズ対応.
#include <panel/pcgutil/pcgutil_CLCD/pigacce_CLCD.h>
#include <panel/plibl/plibl_CLCD/tskpnldraw_CLCD.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070702.h>
#include <panel/plibh/plibh_CLCD2/PNLC_TabletConnectRestrictLib.h>
//Comment 2017/09/04 [HCL-Swathi] [IT5 4.2][RQ-686] [WP732]Multiple Network support.
#include <panel/plibh/plibh_CLCD2/PNLC_NetworkLan.h>
#include "system/sys/APIC_TabletIF.h"

#include <panel/plibh/plibh_CLCD2/PNLC_ActionUserLoginControlLib.h>
#include <panel/pcgact/pcgact_CLCD2/pcgact_pA070705.h>
#include "panel/pcgcomponent/pcgcomponentg/PGadgetComponent.h"	// 2013/11/15 s.ohba	操作性改善FUM5 ウィジェット配置対応.
#include <system/sys/APIC_FuncPatternSetting.h>
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_p10M90.h>
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_p80M01.h>
#include "system/sys/APIC_OptionSetting.h"		//	[UV-01324 サーバーレスユビキタスプリントシステム] 制御構成変更No.28対応.

#include "panel/pcgact/pcgact_CLCD2/pigact_pF072009.h"
#include "panel/pcgact/pcgact_CLCD2/pigact_pF072008.h"

#include "panel/pcgact/pcgact_CLCD2/pigact_pF073000.h"
#include "panel/plibh/plibh_CLCD2/PNLC_KeyConditionLib.h"
#include <panel/pcgutil/pcgutil_CLCD/piputility3_CLCD.h>

//comment 2015/03/02 OPS1-Sky稲井 [特ROM対応][MZV-35_機種マージ] IWSトップメニューを表示する.
#include <panel/pcgbrowser_ext/pcgbrowser_extg/PCGC_AuthOapBrowserWaitEx.h>
#include <panel/plibh/plibh_CLCD2/PNLC_SnapshotControlLib.h>
// comment 2014/11/26 OPS1-Sky浜野 [Poseidon][CQ507492] 画面遷移表示改善.
#include <panel/plibh/plibh_CLCD2/PNLC_SlideMenuControlLib.h>
// comment 2015/02/24 OPS1-Sky佐藤 [ZeusS][CQ509418] 一時言語切換え時のテキスト表示位置不正対応.
#include "panel/plibh/plibhg/plibhGuiWrapper.h"
#include <panel/pcgbrowser_ext/pcgbrowser_extg/pigCostCenterResetDummy.h>	//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/10 Add.
// comment 2015/03/21 OPS1-Sky_nozaki [MinervaSBk][UV-01457]WP32_アドレス帳追加対応.
#include <panel/pcgact/pcgact_CLCD2/pigact_pJ374000.h>
#include <panel/pcgwarn/pcgwarng/pcwrntbl.h>
// comment 2015/08/12 OPS1 Sky稲葉 [ZeusS_ZX0][RQ-383]表示対象に”MarketPlace download”キーを新規追加する.
#include <panel/pcgact/pcgact_CLCD2/pigact_pK472010.h>
#include <panel/plibh/plibh_CLCD2/plibh_CLCD2.h>
#include <panel/pcgact/pcgact_PD/pcgact_A072002900.h>
#include <panel/pdp/pdpg/pdpc_WebUIControl.h>		// comment : WebUI2 画面切替え制御 20180302 Chikuan.
#include <panel/pdp/pdpg/pdpc_WebUIDebug.h>			// comment : WebUI2 画面切替え制御 20180302 Chikuan.
// comment 2018/05/12 Sky東島 [IT6][KRQ-314-29] 目的別UI.
#include <panel/pcgbox/pcgbox_CLCD2/pigbox_base.h>
#include <panel/pcgbox_ext/pcgbox_ext_CLCD2/pigbox_pA059000.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070705.h>
#include <panel/pddgrap/pddgrap_CLCD/pddgrapg256_BASIC.h>	// comment 2018/05/01 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
// comment 2018/04/18 SCD2 Kazuo.Takei [Eagle/Sparrow][制御構成変更_No.111 P07_01_共通仕様(第6世代UI画面_左エリア)].
#include "panel/pcgcomponent/pcgcomponentg/PLeftAreaComponent.h"	// 左エリアコンポーネント.
#include <panel/plibh/plibh_CLCD2/PNLC_BoxBoxListControlLib.h>		// comment : 20180508 Chikuan : [Eagle] [KRQ-314-28] 【新UI】認証＆プリントBox対応.
// comment 2018/04/18 【IT6_Eagle】【HCLT_Pavani】【RQ-675】【WP 3465】 Download and install MarketPlace on all MFPs according to the performance.
#include <panel/plibh/plibh_CLCD2/PNLC_ActionDataLib.h>
#include <panel/pcgwarn/pcgwarn_enl_CLCD2/pigwarnenl_pP0J45.h>
// comment 2018/06/09 Sky弥村 [IT6][制御構成変更No.252] 新UIデザイン_用紙設定画面.
#include <panel/pcgact/pcgact_CLCD2/pigact_p7_000.h>
// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1】Name changing from ID card copy to Copy card for DenebMLK model.
#include <panel/pcgwarn/pcgwarn_CLCD2/pigwarn_pA0A3900.h>
// comment 2018/09/10 SCD2-太田 [Eagle H][制御構成変更No.50]待ち時間測定の自動化.
#include "system/dbg/PFMG_PerformAutoLogging.h"
//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] 起動時に画面がちらつく.
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
// comment 2023/03/06 Sky東島 [IT6_2.3][OP_BTS-41756].
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.
#include <panel/plibh/plibh_CLCD2/PNLC_QWSConnector.h>
//#endif //#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#include <panel/pcgcom_ext/pcgcom_extg/pigAppChangeUIBrowser.h>		// comment 2019/04/16 SCD2-Sky中野 [EagleH] 目的別UI_1ペイン対応.
// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
#include <panel/pcgbrowser_ext/pcgbrowser_extg/pigPullPrintOapBrowserDummy.h>
#include <panel/pcgbrowser_ext/pcgbrowser_extg/pigScanOapBrowserDummy.h>
#include <panel/pcgbrowser_ext/pcgbrowser_extg/pigGeneralOapBrowserDummy.h>
#include <panel/pcgcom_ext/pcgcom_extg/PCGC_WarningPopupDummyBase.h>
#include <panel/pcgcom_ext/pcgcom_extg/pigAppChangeScreenSaver.h>
#include <panel/pcgcom_ext/pcgcom_extg/pigAuthOapBrowserLogin.h>
// comment 2020/08/17 [特ROM対応][IT5-144(機種マージ)] 転送ファックス設定をパスワード無しで使用可能にする要望対応.
#include <panel/pcgadm/pcgadm_CLCD/pigadmin2_CLCD.h>
//comment 2023/03/29 FPT.HungDM17 [IT6_2.3]【OP_BTS-42606】Legacyのトップメニュー画面にメンテナンスモード実行中バーが表示されない問題対策.
#include <panel/pnlmngr/pnlmngrg/PNLC_PartsAccessDebugLib.h>
/*================================================================================================
  定数・型の宣言.
================================================================================================*/
extern	PCGC_AplCommonSet AplCommonSet;		// comment MinervaZeus Sky石賀 2011/11/11 [CQ358125].
extern PA072002900 gp_A072002900;
// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
#include <panel/pcgcopy/pcgcopy_PD/pcgcopy_A0201B3000.h>
extern PA0201B3000 gp_A0201B3000;
// comment 2018/10/23 [HCLT_Prem][DenebMLK] [AR1050427] Card Copy Support.
#include <panel/pcgcopy/pcgcopy_PD/pcgcopy_A020165100.h>
extern PA020165100 gp_A020165100;
// comment 2018/04/06 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
PNLE_WebUIRequestMessage PCGC_HomeTopMenu::m_e_WebUIFunction = PNLE_WebUIFunctionSize;
PNLE_WebUIFunction PCGC_HomeTopMenu::m_e_WebUIBoxFunction = PNLE_WebUIFunctionSize;		// 新UIのシステムボックス遷移先 comment : 20180508 Chikuan : [Eagle] [KRQ-314-28] 【新UI】認証＆プリントBox対応.
PNLE_WebUIFunction PCGC_HomeTopMenu::m_e_WebUICopyFunction = PNLE_WebUIFunctionSize;	// WebUIコピー遷移先.	// comment 2019/01/10 SCD2-Sky中野 [EagleH] 制御No.38_目的別UI対応.
// comment 2020/03/09 SCD2-Sky石原[IT6_2.0MR2][制御No.31]新CopyScanFAXアプリのカスタマイズ機能.
//Bool PCGC_HomeTopMenu::m_bl_WebUIFaxFlag = False;										// WebUIファクス遷移フラグ.		// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
PNLE_WebUIFunction PCGC_HomeTopMenu::m_e_WebUIFaxFunction = PNLE_WebUIFunctionSize;	// WebUIファクス遷移先.	// comment 2020/03/09 SCD2-Sky石原[IT6_2.0MR2][制御No.31]新CopyScanFAXアプリのカスタマイズ機能.

const uchar uc_OptionBrowserLauncherBoot = 10;		// BrowserLauncher起動時のprocBrowser()オプション値.		// comment 2021/12/20 Sky中野 [MinervaSSBK][OP_BTS-36843].

//2018/08/07 HCLT_Monisha [DenebMLK] build error fix.
// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1】 Card copy support for DenebMLK model.
#define PNLD_CopyProgram_CardCopy							30		//Program numebr for Card Shot.
/*================================================================================================
  列挙型変数のタグ宣言.
================================================================================================*/

/*================================================================================================
 マクロ関数の定義.
================================================================================================*/

static PNLS_Parts* pc_MessageAreaParts[] =
// comment 2015/8/31 OPS1-Sky工藤 [MinervaSBK][CQ517742].
//{tbl_c70_013_000_t1_X800_Y60,tbl_c70_013_000_t2_X800_Y60,tbl_c70_013_000_t3_X800_Y60,tbl_o00_014_000_X800_Y60,tbl_c70_013_000_t1_X800_Y60};
{tbl_ec70_000_000_t1_X800_Y60,tbl_ec70_000_000_t2_X800_Y60,tbl_ec70_000_000_t3_X800_Y60,tbl_o00_014_000_X800_Y60,tbl_ec70_000_000_t1_X800_Y60};

PCGC_ProgramKeyDataGetForTopMenu pc_ProgramKeyDataGetForTopMenu;
short PCGC_ProgramKeyDataGetForTopMenu::getProgramNoForTopMenu( uchar uc_index )
{
	short _ss_ScanFaxProgramNo = 0;
	uchar _uc_Count = 0;
	TYPS_ExtShortCutList _st_BadgeKeyList;
//Comment 2017/09/04 [HCL-Suribabu][IT5_4.2][RQ-686][WP768][詳細制御仕様書_管理No.33] supports restrict program function in Network2.
//	if( TYPE_OperationProhibit_Prohibit == PnlComGetOperationSetting()->getEnableProgram() ) { return 0; }	// プログラム機能禁止設定が「禁止」の場合は0を返す.
    if( TYPE_OperationProhibit_Prohibit == PnlComGetOperationSetting()->getEnableProgram( PNLC_NetworkLan::getInstance()->getLine() ) ) { return 0; }	// プログラム機能禁止設定が「禁止」の場合は0を返す
	memset( &_st_BadgeKeyList, 0, sizeof( _st_BadgeKeyList ) );
//Comment 2017/09/04 [HCL-Suribabu][IT5_4.2][RQ-686][WP763][詳細制御仕様書_管理No.33] main menu default settings should be support for Network2.
//	PnlComGetStandardPanelSetting()->getShortCutListSetting( &_st_BadgeKeyList );
	PnlComGetStandardPanelSetting()->getShortCutListSetting( &_st_BadgeKeyList, PNLC_NetworkLan::getInstance()->getLine() );
		for( uchar i = 0; i < TYPD_ExtShortcutMaxTopMenu; i++ ) {
		TYPE_TopMenuShortCutType _e_ShortCutType = TYPE_TopMenuShortCutTypeNone;
		short _ss_TmpScanFaxProgramNo = 0;
		if( ( ( PCGE_HuronMyPanel == PnlComGetMyPanelType() ) || ( PCGE_OapMyPanel == PnlComGetMyPanelType() ) )		// OapMyパネルとHuronパネルの場合.
		    && ( ( NULL != PnlComGetUser() ) && ( TYPE_UserTypeUser == PnlComGetUser()->getUserType() ) ) ) {			// 一般ユーザーでログイン時のみ.
			TYPC_TopMenuShortCutKeyFunction _c_ShortCutKey = PnlComGetMyPanelSetting()->getTopMenuShortCutKeyFunction( i );
			_e_ShortCutType = _c_ShortCutKey.e_TopMenuShortCutType;
			_ss_TmpScanFaxProgramNo = _c_ShortCutKey.u_ShortCutFunctionNo.ss_ScanFaxMemoryNo;
		} else {
			_e_ShortCutType = _st_BadgeKeyList.s_ShortCutKey[i].e_ShortCutType;
			_ss_TmpScanFaxProgramNo = _st_BadgeKeyList.s_ShortCutKey[i].ss_ScanFaxProgramMemoryNo;
		}
		if( TYPE_TopMenuShortCutTypeScanFaxMemory == _e_ShortCutType ) {
			if( uc_index == _uc_Count ) {
				_ss_ScanFaxProgramNo = _ss_TmpScanFaxProgramNo;
				break;
			}
			_uc_Count++;
		}
	}
	return _ss_ScanFaxProgramNo;
}
void PCGC_ProgramKeyDataGetForTopMenu::requestTopMenuProgramKey( short ss_ScanFaxProgramNo )
{
	// 検索条件指定.
	TYPS_IgcAddressSearchCondition _st_IgcAddressSearchCondition;
	// 検索条件区分.
	_st_IgcAddressSearchCondition.e_SearchDivision = TYPE_AddressSearchDivisionKey;		// プログラム宛先.
	// 検索対象宛先.
	_st_IgcAddressSearchCondition.ul_SearchTarget = TYPD_AddressSearchTargetAll;		// 全て.
	// 所有者区分.
	_st_IgcAddressSearchCondition.e_SearchOwnerKind = TYPE_ADDR_OWNER_SEARCHKIND_BOTH;	// 所有者がpublicもしくは一般ユーザー.
	// 検索結果への宛先保護レベルでのフィルタ指定.
	_st_IgcAddressSearchCondition.b_RefLicenseFiterOn = isRefLicenseFiterOn();
	// メールの暗号化"する"か.
	_st_IgcAddressSearchCondition.b_SmimeSearch = PnlComGetJobScanFax( PCGE_JobPutOther )->getJobSpec()->pc_Send->c_MailSend.getSmimeEnableCrypt();
	// ソート種別.
	_st_IgcAddressSearchCondition.e_SortType = TYPE_ADDR_SORT_TYPE_ID;					// 表示用IDでソート(MYとPUBLICで同じIDがある場合にはMY⇒PUBの順).
	// 検索条件種別.
	_st_IgcAddressSearchCondition.e_SearchType = TYPE_AddressSearchTypeProgramWithAddressType;				// プログラム宛のグループ指定有無により検索.
	// 検索条件キー.
	if( True != onlyPanel.isFaxFunctionUseCheck( PDPE_FaxFunctionMultiAddrBan ) ) {		// グループ宛先使用可否.
		_st_IgcAddressSearchCondition.u_SearchType_Key.e_GroupSetting = TYPE_AddressSearchGroupSettingOff;	// 複数宛先禁止のためグループを持つプログラムは対象外.
	} else {
		_st_IgcAddressSearchCondition.u_SearchType_Key.e_GroupSetting = TYPE_AddressSearchGroupSettingAll;	// グループを持つプログラムも検索対象.
	}
	// 宛先要求情報指定.
	TYPS_ADDRESS_REQUEST_INFO _st_AddressRequestInfo;
	// ユーザー情報取得.
	if( NULL == PnlComGetUser() ) {
		_st_AddressRequestInfo.pC_user = APIC_User::newInstance( TYPD_IndexUserAdmin );;
	} else {
		_st_AddressRequestInfo.pC_user = PnlComGetUser();
	}
	// 部門情報取得.
	_st_AddressRequestInfo.pC_track = PnlComGetTrack();
	// 要求元先 (監査ログ I/F)はパネル.
	_st_AddressRequestInfo.e_InspectLogIF = TYPE_InspectLogIfOperationPanel;
	// 検索開始（ページ指定）.
	c_AddressSearchIGC.requestStartAddressSearchPage( &_st_IgcAddressSearchCondition, 1, ss_ScanFaxProgramNo, &_st_AddressRequestInfo );
}

// comment 2017/11/21(火) SCD2-Sky長谷川(伸) [FQ-OPSD-1700523-OFP264].
/**
 *	@brief	プログラム宛先情報検索要フラグ設定.
 *	@param	bl_isNeed プログラム宛先情報検索要フラグ.
 *	@retval	void.
 *	@history	2017/11/21 長谷川(伸) 新規作成.
 */
void PCGC_ProgramKeyDataGetForTopMenu::setNeedProgramSearchFlg( Bool bl_isNeed )
{
	m_bl_isNeedSearchScanFaxProgram = bl_isNeed;
}

/**
 *	@brief	プログラム宛先情報検索要フラグ取得.
 *	@param	none.
 *	@retval	プログラム宛先情報検索要フラグ.
 *	@history	2017/11/21 長谷川(伸) 新規作成.
 */
Bool PCGC_ProgramKeyDataGetForTopMenu::getNeedProgramSearchFlg( )
{
	return m_bl_isNeedSearchScanFaxProgram;
}

/**
 *	@brief	プログラム宛先検索ステータス設定.
 *	@param	uc_ProgramSearchStatus プログラム宛先検索ステータス.
 *	@retval	void.
 *	@history	2017/11/21 長谷川(伸) 新規作成.
 */
void PCGC_ProgramKeyDataGetForTopMenu::setIGCProgramSearchStatus( uchar uc_ProgramSearchStatus )
{
	m_uc_ProgramSearchStatus = uc_ProgramSearchStatus;
}

/**
 *	@brief	プログラム宛先検索ステータス取得.
 *	@param	none.
 *	@retval	プログラム宛先検索ステータス.
 *	@history	2017/11/21 長谷川(伸) 新規作成.
 */
uchar PCGC_ProgramKeyDataGetForTopMenu::getIGCProgramSearchStatus( )
{
	return m_uc_ProgramSearchStatus;
}

// comment 2018/10/26 SCD2 T.Nishiba [Eagle][AR1064089]プログラム宛先の再検索を一度だけに制限する.
/**
 *	@brief	プログラム宛先の再検索実行済みステータス設定.
 *	@param	bl_ProgramSearchRetryExcuted プログラム宛先の再検索実行済みステータス.
 *	@retval	void.
 *	@history	2018/10/23 SCD2 T.Nishiba 新規作成.
 */
void PCGC_ProgramKeyDataGetForTopMenu::setProgramSearchRetryExcuted( Bool bl_ProgramSearchRetryExcuted )
{
	m_bl_ProgramSearchRetryExcuted = bl_ProgramSearchRetryExcuted;
}

/**
 *	@brief	プログラム宛先の再検索実行済みステータス取得.
 *	@param	none.
 *	@retval	プログラム宛先の再検索実行済みステータス.
 *	@history	2018/10/23 SCD2 T.Nishiba 新規作成.
 */
Bool PCGC_ProgramKeyDataGetForTopMenu::getProgramSearchRetryExcuted( )
{
	return m_bl_ProgramSearchRetryExcuted;
}


//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： リセットキー.
//================================================================================================
void PCGC_HomeTopMenu::procReset( uchar option )
{
	if( onlyPanel.uc_RemoteStopStatus != PM_RMPSTOPTYPE_NONE ) {
		PMessageProcCom::procReset( option );
	} else {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}
// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： リセットキー活性状態.
//================================================================================================
Bool PCGC_HomeTopMenu::resetKeyActiveCheck()
{
	if( onlyPanel.uc_RemoteStopStatus != PM_RMPSTOPTYPE_NONE ) {
		return PMessageProcCom::resetKeyActiveCheck();
	} else {
		ACTIVEKEY_RET( False );
	}
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： パラメータのリセット.
//================================================================================================
void PCGC_HomeTopMenu::resetParam( uchar option )
{
	if( onlyPanel.uc_RemoteStopStatus != PM_RMPSTOPTYPE_NONE ) {
		PMessageProcCom::procReset( option );
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： IDキー.
//================================================================================================
void PCGC_HomeTopMenu::procIDInput( uchar option )
{
// comment 2015/04/02 MSE田中優介 [MinervaSBK][RQ-278] LDAPサーバーへのカード情報登録機能対応.
	// LDAPサーバーへのカード情報登録機能の終了処理.
	endAutoCardRegist();
	Bool _bl_isUserTrackOn = True;
// Comment 2015/12/17  [HCL_Praveen] [Zeus_ZX0] [CQ522404] Changing the logic to exclude the track check for PSES authentication.
//	if( NULL == PnlComGetUser() )	{ _bl_isUserTrackOn = False; }
//	if( ( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode()	) && ( NULL == PnlComGetTrack() ) ) {
//		_bl_isUserTrackOn = False;
//	}
	
// comment 2019/03/08 SCD2-Sky中野 [EagleH][OP_BTS-3117]認証未ログイン時ホーム画面に遷移できない.
	if( ( NULL == PnlComGetUserAuthentication() )
	 || ( NULL == PnlComGetTrackManager() ) ) {
		LOGF_debug_PutEventLog(LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!");
		return;
	}

	if( NULL == PnlComGetUser() )	{
		_bl_isUserTrackOn = False; 
// comment 2019/03/08 SCD2-Sky中野 [EagleH][OP_BTS-3117]認証未ログイン時ホーム画面に遷移できない.
//	}
	} else {
// comment 2020/01/10 SCD2-Sky中野 [HeliosMLK][OP_BTS-18537].
//		if( ( TYPE_UserAuthenticationOff == PnlComGetUserAuthentication()->getUserAuthentication(PNLC_NetworkLan::getInstance()->getLine()) )	// ユーザー認証：OFF.
//		 && ( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance()->getLine()) )							// 部門管理：有効.
//		 && ( NULL == PnlComGetTrack() ) ) {																									// 部門未ログイン.
// comment 2021/04/30 Sky東島 [IT6_2.2][OP_BTS-32561].
//		if( ( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode( PNLC_NetworkLan::getInstance()->getLine() ) )						// 部門管理：有効.
//		 && ( NULL == PnlComGetTrack() ) ) {																								// 部門未ログイン.
		 if( True == PnlComGetNeedToTrackLogin() ) {																								// 部門未ログイン.
			_bl_isUserTrackOn = False;
		}
	}
//	Comment 2016/11/12 【HCLT-Ashok】 【AR-1007594】 Below conditions exclude it is available in base class.	
//	}else{
//		if( (TYPE_UserAuthenticationIntermeddiateServer != PnlComGetUser()->getLoginUserAuthenticationType())		//Authentication type check.
//			&& ( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode()	) 									//Track mode enable check.
//			&& ( NULL == PnlComGetTrack() ) ) {																		//Track user check.
//			_bl_isUserTrackOn = False;
//		}
//	}
	if( False == _bl_isUserTrackOn ) {
		rebuildGamen( 0 );
		setTurnGamenThis( NULL );
	} else {
		PMessageArea::procIDInput( option );
	}
}
// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： IDキー活性状態.
//================================================================================================
Bool PCGC_HomeTopMenu::idKeyActiveCheck()
{
	Bool _bl_isUserTrackOn = True;
	
	if( ( NULL == PnlComGetUserAuthentication() )
	 || ( NULL == PnlComGetTrackManager() ) ) {
	 	ACTIVEKEY_RET( True );
	}

	if( NULL == PnlComGetUser() )	{
		_bl_isUserTrackOn = False; 
	} else {
// comment 2021/04/30 Sky東島 [IT6_2.2][OP_BTS-32561].
//		if( ( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode( PNLC_NetworkLan::getInstance()->getLine() ) )						// 部門管理：有効.
//		 && ( NULL == PnlComGetTrack() ) ) {																								// 部門未ログイン.
		if( True == PnlComGetNeedToTrackLogin() ) {																								// 部門未ログイン.
			_bl_isUserTrackOn = False;
		}
	}
	if( False == _bl_isUserTrackOn ) {
		ACTIVEKEY_RET( True );
	} else {
		return PMessageArea::idKeyActiveCheck();
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： メニューキー.
//================================================================================================
void PCGC_HomeTopMenu::procMenu( uchar option )
{
	// Comment : 2018/07/09 Chikuan [Eagle][Accurev:1051267] 新トップメニューでのメニューキーの受付を不可にする.
	if( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() ){
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW212, 1 ) ) {		//DipSW212 bit1=ON
		if( False == m_bl_MenuEnable ) {					// メニューキー受付け不可の場合.
			setBuzzerType( PCGE_BuzzerType_InputNg );
			return;
		}
	}

	if( PnlComGetUserAccountOn() && ( NULL != PnlComGetUser() ) && ( TYPE_UserTypeAdmin == PnlComGetUser()->getUserType() ) ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	// 閉じれるときのみにする.
	PLayerControl* p_GamenInstance = dynamic_cast< PLayerControl* >( nowGamen );
	PEnlBaseGamenCom* p_EnlGamenInstance = dynamic_cast< PEnlBaseGamenCom* >( nowGamen );
	if( True ==  onlyPanel.isTopmenuMode() ) {
		// 優先アプリの設定にしたがって遷移してきた場合、閉じない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
	} else if( ( NULL == p_GamenInstance )
	           || ( NULL != p_EnlGamenInstance ) ) {
		// 旧UIもしくは拡大画面時のみ実施.
		rebuildGamen( 0 );
		setTurnGamenThis( NULL );
		setBuzzerType( PCGE_BuzzerType_Base );
	} else {
		ushort us_warntbl = onlyPanel.isAllGamenWarning( onlyPanel.cc_DisplayJob );
		Bool b_ModeDisable = False ;
		b_ModeDisable = onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, PNL_OTHER_VendorAdminInput ) ;	// ベンダー管理者パスワード入力中 .
// Comment 2015/11/21【HCLT-Harikrishnan】【CQ519679】【ID: 26289】Reverting Changes since fix not required.
//// Comment 2015/10/09【HCLT-Juned】【CQ519679】【ID: 26289】ABV.STACK.
////		if ( 0 != us_warntbl ) {
//	if ( ( ( 0 < us_warntbl ) && ( DEF_sp_GamenWarningTable > us_warntbl ) )
		if (( 0 != us_warntbl )
		    ||( True == b_ModeDisable ) ) {
			rebuildGamen( option );
		} else if( EP_PNS_Setting != onlyPanel.ep_PanelStatus ) {
			rebuildGamen( 2 );
		} else {
			PNLC_PopupListControl::newInstance()->closeTop( False );	// ウィンドウを閉じ、表示前の画面に戻る.

			// メニュー画面遷移でマネージャリセットされる.
			// メニュー画面を閉じるだけでは、マネージャブロックが0のままとなりイベント受けることができないため認証トップ画面へ遷移させる.
			uchar	seqValid	= 0;
			PnlComGetSeqCustomValid( &seqValid );
			if ( TYPD_SeqCustom_Function_VALID == seqValid ) {
				Bool	bl_isSeqGamen	= False;
				short	ss_TableNo	= onlyPanel.isDisableModeSet();
				APIC_UserAuthentication*	pc_UserAuth	= APIC_UserAuthentication::newInstance();
				TYPE_UserAuthentication	e_UserAuth	= TYPE_UserAuthenticationOff;
				if ( 0 != pc_UserAuth ) {
					e_UserAuth	= pc_UserAuth->getDefaultUserAuthentication(PNLC_NetworkLan::getInstance()->getLine());
				}
				switch ( sp_GamenDisableModeSetTable[ss_TableNo].ec_WarningName ) {
				case PNL_OTHER_UserIDNumberInput:
				case PNL_OTHER_UserIDNumberInput3:
				case PNL_OTHER_TrackIDNumberInput:
					bl_isSeqGamen	= True;
					break;
				case PNL_OTHER_UserIDNumberInput2:
					if ( TYPE_UserAuthenticationDevice == e_UserAuth ) {
						bl_isSeqGamen	= True;
					}
					break;
				default:
					break;
				}
				if ( ( True == bl_isSeqGamen ) && ( PNLE_SeqCustom_Status_Top == PnlComSeqGetStatus() ) ) {
					PnlComSeqSetStatus( PNLE_SeqCustom_Status_Act );
					PnlComSeqTransTopGamen();
				}
			}
			if( NULL == nextGamen && nowGamen == &gp_PublicPopupDummyBase && False == ScanOapExtSet.isActiveGeneralApplication() )
			{
				nextGamen = ScanOapExtSet.getTopGamen();
//  comment 2016/03/10 OPS1 Sky森山[ZeusS_ZX0][CQ525849].
				// ブラウザーランチャー使用時、ポップアップ画面を表示させるとnowGamenが「gp_PublicPopupDummyBase」に更新され、.
				// トップメニューを閉じた後の戻り先が不明となる為、ブラウザランチャーに遷移するように修正.
				Bool isActiveBrowserLauncher = ScanOapExtSet.isActiveGeneralApplication( TYPE_JobTypeCopy, PNLE_CheckType_ALL );
				if( True == isActiveBrowserLauncher ) {
					PnlComBrowserLauncherModeReboot();
					if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type3() ) {
						ScanOapExtSet.m_bTopmenuModeBackup = onlyPanel.isTopmenuMode();
					}					
// comment 2021/12/20 Sky中野 [MinervaSSBK][OP_BTS-36843].
//					procBrowser( 0 );
					procBrowser( uc_OptionBrowserLauncherBoot );
					if( &gp_AppChangeBrowser == nextGamen ) {
						// アプリ種別設定.
						ScanOapExtSet.setExtAppType( TYPE_ExtTypeBrowserLauncher );
						return;
					}
				}
			}
		}
		setBuzzerType( PCGE_BuzzerType_Base );
	}
}
// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： メニューキー活性状態.
//================================================================================================
Bool PCGC_HomeTopMenu::homeKeyActiveCheck()
{
	if( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() ){
		ACTIVEKEY_RET( False );
	}

	if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW212, 1 ) ) {		//DipSW212 bit1=ON
		if( False == m_bl_MenuEnable ) {					// メニューキー受付け不可の場合.
			ACTIVEKEY_RET( False );
		}
	}

	if( PnlComGetUserAccountOn() && ( NULL != PnlComGetUser() ) && ( TYPE_UserTypeAdmin == PnlComGetUser()->getUserType() ) ) {
		ACTIVEKEY_RET( False );
	}
	
	if( True ==  onlyPanel.isTopmenuMode() ) {
		ACTIVEKEY_RET( False );
	}
	ACTIVEKEY_RET( True );	
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： 拡大表示キー.
//================================================================================================
void PCGC_HomeTopMenu::procEnlarge( uchar option )
{
// comment : WebUI2 画面切替え制御 -> 20180302 Chikuan.
	if( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() ){
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
// comment : WebUI2 画面切替え制御 <- 20180302 Chikuan.

// 拡大機能は受付禁止.
	if( True == PNLC_HitachiICC_TopMenuCustomize::newInstance()->isHitachiICC_Mode() ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBit( 34, 5 )	// ベンダー装着時、長尺プリント使用可能の場合.
	    && True == PnlComGetSystemSetting()->getLongPicturePrint() ) {		// 長尺印刷ON.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	// comment 2015/02/12 MSE高橋 [Poseidon][CQ509510] OCR読込み中は、アプリ切り替え不可.
	if( ( EP_PNS_ReadExec == onlyPanel.ep_PanelStatus )					// 読込中.
	    || ( EP_PNS_ReadErrorStop == onlyPanel.ep_PanelStatus )				// 読込エラー.
	    || ( EP_PNS_ReadStop == onlyPanel.ep_PanelStatus )					// 読込停止中.
	    || ( EP_PNS_SeparateScanStop == onlyPanel.ep_PanelStatus )			// 分割読込停止中.
	    || ( EP_PNS_RealTimePrvExec == onlyPanel.ep_PanelStatus )			// リアルタイムプレビュー動作中.
		|| ( EP_PNS_OcrReadExec == onlyPanel.ep_PanelStatus )				// OCR読込み中.
	    || ( EP_PNS_PrintStop == onlyPanel.ep_PanelStatus ) ) {				// プリント停止中.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	PCGC_StopGamenGetJob c_StopGamenGetJob;
	short _ss_StopJob = c_StopGamenGetJob.getNowStopJob();
	for( short i = 0; i < _ss_StopJob; i++ ) {
		if( PCGE_StopJobPrint == c_StopGamenGetJob.s_MmbAryStopJob[i].e_StopJobStatus ) {
			setBuzzerType( PCGE_BuzzerType_InputNg );
			return;
		}
	}
	// 汎用アプリ有効
	if( True == AplCommonSet.isActiveGeneralApplication( TYPE_JobTypeCopy, PNLE_CheckType_ALL ) ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	if( ( NULL != comPanelJob )
	    && ( NULL != comPanelJob->getJobSpec() )
	    && ( NULL != comPanelJob->getJobSpec()->pc_Common ) ) {
		// 割り込みコピー時は拡大表示への遷移不可.
		if( TYPE_JobTypeIntCopy == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() ) {
			setBuzzerType( PCGE_BuzzerType_InputNg );
			return;
		}
	}

// comment 2015/04/02 MSE田中優介 [MinervaSBK][RQ-278] LDAPサーバーへのカード情報登録機能対応.
	// カードID登録機能が動作中でない場合、拡大表示を無効とする.
	PNLC_CardAuthDirectLink* pc_CardAuthDirectLink = PNLC_CardAuthDirectLink::newInstance();
	if ( ( NULL != pc_CardAuthDirectLink )
	  && ( True == pc_CardAuthDirectLink->isCardDirectLinkMode() )			// 特ROM機能のカードAD直接連携有効、かつ LDAP-ICカード認証サーバーへのカード登録許可設定有効.
	  && ( True == pc_CardAuthDirectLink->getAutoCardRegistState() ) ) {	// カードID登録機能有効時.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

// comment 2019/01/21 SCD2-Sky嵯峨山 SPS認証対応.
	PNLC_SPSAuthDataLib* pc_SPSAuthDataLib = PNLC_SPSAuthDataLib::newInstance();
	if ( ( NULL != pc_SPSAuthDataLib )
	&&   ( True == pc_SPSAuthDataLib->isSPSMode()
	&&   ( NULL == PnlComGetUser()  ) ) ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	APIC_SystemSetting* pc_SystemSetting = PnlComGetSystemSetting();
	if( NULL != pc_SystemSetting ) {
		// 拡大表示時設定取得（スライドメニュー設定）.
		if( True == pc_SystemSetting->getSlideMenuEnlargeDisp() ) {
// Comment 2018/01/17 [HCL-Prasanjeet] [IT5 4.2][AR1034498] should not call hide function if slider menu is open state.
			if ( False == PNLC_SlideMenuControlLib::newInstance()->isOpen() ) {
				PNLC_SlideMenuControlLib::newInstance()->hide();
			}
		}
	}
	PMessageArea::procEnlarge( option );
	if( True == PNLC_PopupListControl::newInstance()->isNowPopup( &gp_EnlAct_EnlargeModeChangeConfirm ) ) {
		nextGamenBackup = &gp_EnlAct_HomeTopMenu;
	} else {
// Comment 2018/01/17 [HCL-Prasanjeet] [IT5 4.2][AR1034498] should not call hide function if slider menu is open state.
        PNLC_SlideMenuControlLib::newInstance()->setTopMenu(True);
		nextGamen = &gp_EnlAct_HomeTopMenu;
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： コピーキー.
//================================================================================================
void PCGC_HomeTopMenu::procCopy( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	if( True == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Copy ) ) {
// comment 2022/04/28 Sky中野 [IT6_2.3] 制御No.43_システムオートリセット改善(WebUIv2/v3統合).
//// comment : WebUI2 画面切替え制御 -> 20180302 Chikuan.
//		if( ( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() )
//// comment 2019/01/10 SCD2-Sky中野 [EagleH] 制御No.38_目的別UI対応.
////		 && ( ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() )
////		   || ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionCopy ) ) ) ){
//		&& ( ( PNLE_WebUIFunctionSize != PCGC_HomeTopMenu::getWebUICopyFunction() )
//			&& ( ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() )
//				|| (  False == PNLC_WebUIControl::getInstance()->getEnableFunction( PCGC_HomeTopMenu::getWebUICopyFunction() ) ) ) ) ) {
//			setBuzzerType( PCGE_BuzzerType_InputNg );
//			return;
//		}
// comment : WebUI2 画面切替え制御 <- 20180302 Chikuan.
		// 現在の指定アプリ遷移先状態を保存し、状態を「待機中」に更新する.
		PNLE_SpecifiedAppState e_State = c_ExtPanel.getSpecifiedAppState();
//comment 2017/07/12 SCD2-Sky後藤 [IT5_4.1][1026344]ジョブ予約キー表示されない問題.
		onlyPanel.setTopMenuSpecifiedAppState( e_State );
		c_ExtPanel.ChangeSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
		PMessageArea::procCopy( option );
//comment 2017/07/12 SCD2-Sky後藤 [IT5_4.1][1026344]ジョブ予約キー表示されない問題.
		onlyPanel.setTopMenuSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
		if( NULL != nextGamen ) {					// [CQ.355542]遷移できる場合のみ.
			onlyPanel.setTopmenuMode( False );		// [CQ.346271]メニューキー受付け対応.
		}
		// 統合メニューからポップアップ表示を行う警告が発生していなかった場合のみ.
		// (画面遷移が発生していない).
		else if( False == c_ExtPanel.checkPopupWarningFromTopMenu() ) {
			c_ExtPanel.ChangeSpecifiedAppState( e_State );	// 指定アプリ遷移先状態を元に戻す.
		}
	} else {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： FAX.
//================================================================================================
void PCGC_HomeTopMenu::procFax( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
// comment 2019/10/07 Chikuan [EagleH][OP_BTS-14390] 問題発生時の切分けLog追加.
		PNL_LogManager::newInstance()->setGeneralPurposeLog("RejectKey(pcgact)", (ulong)__LINE__ );
		return;
	}
	if( True == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Fax ) ) {
// comment 2022/04/28 Sky中野 [IT6_2.3] 制御No.43_システムオートリセット改善(WebUIv2/v3統合).
//// comment : WebUI2 画面切替え制御 -> 20180302 Chikuan.
//		if( ( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() )
//		 && ( ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() )
//		   || ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionScan ) ) ) ){
//			setBuzzerType( PCGE_BuzzerType_InputNg );
//			return;
//		}
// Comment 2019/05/02 HCLT-Tarun [IT6_EagleH] [OP_BTS-4201] Remove Same mode transition clear setting restriction.
// ScanFax/Fax -> Home Menu -> ScanFax/Fax (Value clearence accept).
		TYPE_JobType e_JobType = comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType();
		if( True == PnlComGetFaxPanelSetting()->getJobClearSetting()){ // Use Admin Setting API.
			if ( ( e_JobType == TYPE_JobTypeFaxSendMemory ) || ( e_JobType == TYPE_JobTypeFaxReceive ) ) {
				onlyPanel.ClearScanFaxJobSetting();
			}
		} 
// comment : WebUI2 画面切替え制御 <- 20180302 Chikuan.
		// 現在の指定アプリ遷移先状態を保存し、状態を「待機中」に更新する.
		PNLE_SpecifiedAppState e_State = c_ExtPanel.getSpecifiedAppState();
//comment 2017/07/12 SCD2-Sky後藤 [IT5_4.1][1026344]ジョブ予約キー表示されない問題.
		onlyPanel.setTopMenuSpecifiedAppState( e_State );
		c_ExtPanel.ChangeSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
		PMessageArea::procFax( option );
//comment 2017/07/12 SCD2-Sky後藤 [IT5_4.1][1026344]ジョブ予約キー表示されない問題.
		onlyPanel.setTopMenuSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
		if( ( NULL != nextGamen ) ||				// [CQ.355542]遷移できる場合のみ.
		    ( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_IGCClearWait_popup ) ) ) {
			onlyPanel.setTopmenuMode( False );		// [CQ.346271]メニューキー受付け対応.
		}
		// 統合メニューからポップアップ表示を行う警告が発生していなかった場合のみ.
		// (画面遷移が発生していない).
		else if( ( NULL == nextGamen ) && ( False == c_ExtPanel.checkPopupWarningFromTopMenu() ) ) {
			c_ExtPanel.ChangeSpecifiedAppState( e_State );	// 指定アプリ遷移先状態を元に戻す.
		}
	} else {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： FAX.
//================================================================================================
void PCGC_HomeTopMenu::procFaxOnly( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	if( True == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Fax ) ) {
// comment : WebUI2 画面切替え制御 -> 20180302 Chikuan.
// comment 2022/04/28 Sky中野 [IT6_2.3] 制御No.43_システムオートリセット改善(WebUIv2/v3統合).
//		if( ( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() )
//		 && ( ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() )
//// comment 2019/4/12 Sky雜賀[EagleH][OP_BTS-4256].
////		   || ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionFax ) )
//// comment 2018/06/05 SCD2-Sky鳫林 「送信承認パスワード入力」がONのとき、FAXを使わせないようにする 対応.
////		   || ( TYPE_SendApprovalPasswordFuncOn == PnlComGetFaxPanelSetting()->getSendApprovalPasswordFunc() ) ) ){
//		   || ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionFax ) ) ) ){
//			setBuzzerType( PCGE_BuzzerType_InputNg );
//			return;
//		}
// Comment 2019/05/02 HCLT-Tarun [IT6_EagleH] [OP_BTS-4201] Remove Same mode transition clear setting restriction.
// ScanFax/Fax -> Home Menu -> ScanFax/Fax (Value clearence accept).
		if( True == PnlComGetFaxPanelSetting()->getJobClearSetting()){ // Use Admin Setting API.
			if ( True == onlyPanel.getFaxAppMode() ) {
				onlyPanel.ClearScanFaxJobSetting();
			}
		}
// comment : WebUI2 画面切替え制御 <- 20180302 Chikuan.
		// 現在の指定アプリ遷移先状態を保存し、状態を「待機中」に更新する.
		PNLE_SpecifiedAppState e_State = c_ExtPanel.getSpecifiedAppState();
//comment 2017/07/12 SCD2-Sky後藤 [IT5_4.1][1026344]ジョブ予約キー表示されない問題.
		onlyPanel.setTopMenuSpecifiedAppState( e_State );
		c_ExtPanel.ChangeSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
		PMessageArea::procFaxOnly( option );

//comment 2017/07/12 SCD2-Sky後藤 [IT5_4.1][1026344]ジョブ予約キー表示されない問題.
		onlyPanel.setTopMenuSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
		if( ( NULL != nextGamen ) ||				// [CQ.355542]遷移できる場合のみ.
		    ( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_IGCClearWait_popup ) ) ) {
			onlyPanel.setTopmenuMode( False );		// [CQ.346271]メニューキー受付け対応.
		}
		// 統合メニューからポップアップ表示を行う警告が発生していなかった場合のみ.
		// (画面遷移が発生していない).
		else if( ( NULL == nextGamen ) && ( False == c_ExtPanel.checkPopupWarningFromTopMenu() ) ) {
			c_ExtPanel.ChangeSpecifiedAppState( e_State );	// 指定アプリ遷移先状態を元に戻す.
		}
	} else {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： ScanToDevice.
//================================================================================================
void PCGC_HomeTopMenu::changeScanToDevice( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	if( True == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_ScanToDevice ) ) {
		// 現在の指定アプリ遷移先状態を保存し、状態を「待機中」に更新する.
		PNLE_SpecifiedAppState e_State = c_ExtPanel.getSpecifiedAppState();
//comment 2017/07/12 SCD2-Sky後藤 [IT5_4.1][1026344]ジョブ予約キー表示されない問題.
		onlyPanel.setTopMenuSpecifiedAppState( e_State );
		c_ExtPanel.ChangeSpecifiedAppState( PNLE_SpecifiedAppState_Idle );

		// パネル内部警告の原稿サイズ検出エラー(動作前)解除.
		onlyPanel.resetDocSizeDetectError_S();

		// 操作パネル上のアプリケーション表示切り換え.
		c_JobControl.changePanelAPP( TYPE_JobTypeFaxSendMemory, False, True, False, True );

// comment 2014/12/17 OPS1-mse大石 [ZeusS][RQ-232][CQ507998]ジョブ継続中の再接続時は送信中に遷移.
		APIC_AndroidPanel* pc_AndroidPanel = APIC_AndroidPanel::newInstance();
		if ( NULL == pc_AndroidPanel ){
			LOGF_debug_PutEventLog(LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!");
			return;
		}
		// 送信中画面表示状態の取得.
		Bool bl_DuringScanToAndroid = pc_AndroidPanel->getDuringScanToAndroid();
		if ( True == bl_DuringScanToAndroid ) {
			// 送信中状態に遷移.
			onlyPanel.setPanelStatus( EP_PNS_ScanToDeviceExec );
		} else {
		// パネルの状態を設定中に設定.
		onlyPanel.setPanelStatus( EP_PNS_Setting );
		}

		// ユビキタスプリントモードをoffにする.
		onlyPanel.setUbqPrintMode( False );
// Comment 2015/12/15 HCLT-Krithika [ZeusS_ZX0][制御構成変更_No 76][WP 2546] [CQ522124] option condition is not required, by default option is 0 from procGoToPage.
//// Comment 2015/10/30 HCLT-Vivek [ZeusS_ZX0][制御構成変更_No 76][WP 2546] Counter value to be updated when Panel Link Scan operation is performed using bizhub RA.
//		if (10 == option ){
//			PnlComIncreaseOperateCnt(TYPE_OperateCnt_BizhubRA_Scan);          //Counter value updation.
//		}
		PnlComIncreaseOperateCnt(TYPE_OperateCnt_BizhubRA_Scan);          //Counter value updation.
		
		// 画面遷移 拡大表示非対応.
		rebuildGamen( 0 );

		// アプリを切り換えたためturnGamenをクリア.
		setTurnGamenThis( NULL );

		// LDAP検索キーワードクリア.
		PNLC_LdapSearchManager::newInstance()->clearLdapSearchCondition();			// LDAP検索条件クリア.
		PNLC_LdapSearchManager::newInstance()->clearLdapMultiSearchCondition();		// LDAP複合検索用検索条件クリア.


//comment 2017/07/12 SCD2-Sky後藤 [IT5_4.1][1026344]ジョブ予約キー表示されない問題.
		onlyPanel.setTopMenuSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
		// アップロードファイル作成要求中の画面遷移の場合.
		if( ( nextGamen != NULL )
		    && ( True == onlyPanel.bl_BrowserFileUploadRequest ) ) {
			uchar uc_UploadType = 0;
			onlyPanel.setTopmenuMode( False );

			if( &gp_FileUploadTypeSelectExt_browser == getNowGamen() ) {	// アップロード機能選択.
				uc_UploadType = PCGD_BrowserFileUploadTypeSelect;
			}
			deleteUploadInfo( uc_UploadType );								// アップロードファイル情報削除.
		}
		// 統合メニューからポップアップ表示を行う警告が発生していなかった場合のみ.
		// (画面遷移が発生していない).
		else if( ( NULL == nextGamen ) && ( False == c_ExtPanel.checkPopupWarningFromTopMenu() ) ) {
			c_ExtPanel.ChangeSpecifiedAppState( e_State );	// 指定アプリ遷移先状態を元に戻す.
		}
	} else {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： ボックス.
//================================================================================================
void PCGC_HomeTopMenu::procBox( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
// comment 2019/10/07 Chikuan [EagleH][OP_BTS-14390] 問題発生時の切分けLog追加.
		PNL_LogManager::newInstance()->setGeneralPurposeLog("RejectKey(pcgact)", (ulong)__LINE__ );
		return;
	}
	if( True == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_BoxOperation ) ) {
		// 現在の指定アプリ遷移先状態を保存し、状態を「待機中」に更新する.
		PNLE_SpecifiedAppState e_State = c_ExtPanel.getSpecifiedAppState();
//comment 2017/07/12 SCD2-Sky後藤 [IT5_4.1][1026344]ジョブ予約キー表示されない問題.
		onlyPanel.setTopMenuSpecifiedAppState( e_State );
		c_ExtPanel.ChangeSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
		PMessageArea::procBox( option );
//comment 2017/07/12 SCD2-Sky後藤 [IT5_4.1][1026344]ジョブ予約キー表示されない問題.
		onlyPanel.setTopMenuSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
		if( NULL != nextGamen ) {					// [CQ.355542]遷移できる場合のみ.
			onlyPanel.setTopmenuMode( False );		// [CQ.346271]メニューキー受付け対応.
		}
		// 統合メニューからポップアップ表示を行う警告が発生していなかった場合のみ.
		// (画面遷移が発生していない).
		else if( False == c_ExtPanel.checkPopupWarningFromTopMenu() ) {
			c_ExtPanel.ChangeSpecifiedAppState( e_State );	// 指定アプリ遷移先状態を元に戻す.
		}
	} else {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： ブラウザ切替キー処理.
//================================================================================================
void PCGC_HomeTopMenu::procBrowser( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
// comment 2019/10/07 Chikuan [EagleH][OP_BTS-14390] 問題発生時の切分けLog追加.
		PNL_LogManager::newInstance()->setGeneralPurposeLog("RejectKey(pcgact)", (ulong)__LINE__ );
		return;
	}
	if( True == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_WebBrowser ) ) {
		// 現在の指定アプリ遷移先状態を保存し、状態を「待機中」に更新する.
		PNLE_SpecifiedAppState e_State = c_ExtPanel.getSpecifiedAppState();
//comment 2017/07/12 SCD2-Sky後藤 [IT5_4.1][1026344]ジョブ予約キー表示されない問題.
		onlyPanel.setTopMenuSpecifiedAppState( e_State );
		c_ExtPanel.ChangeSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
// comment 2019/11/29 Sky東島 [EagleZ+][OP_BTS-15714].
// comment 2021/12/20 Sky中野 [MinervaSSBK][OP_BTS-36843].
		if( uc_OptionBrowserLauncherBoot == option ) {
			// BrowserLauncherを起動するため、onlyPanel.bl_FromBrowserLauncherは無効にしない.
			option = 0;
		} else {
			onlyPanel.bl_FromBrowserLauncher = False;
		}
		PMessageArea::procBrowser( option );
//comment 2017/07/12 SCD2-Sky後藤 [IT5_4.1][1026344]ジョブ予約キー表示されない問題.
		onlyPanel.setTopMenuSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
		if( NULL != nextGamen ) {					// [CQ.355542]遷移できる場合のみ.
			onlyPanel.setTopmenuMode( False );		// [CQ.346271]メニューキー受付け対応.
		}
		// 統合メニューからポップアップ表示を行う警告が発生していなかった場合のみ.
		// (画面遷移が発生していない).
		else if( False == c_ExtPanel.checkPopupWarningFromTopMenu() ) {
			c_ExtPanel.ChangeSpecifiedAppState( e_State );	// 指定アプリ遷移先状態を元に戻す.
		}
	} else {
// comment 2020/11/09 HCL_Prabakaran [IT6_2.1][ OP_BTS-28914] Log added to confirm issue in future occurrence.
		PNL_LogManager::newInstance()->setGeneralPurposeLog("RejectKey(isFuncEnable)", (ulong)__LINE__ );
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： ぷりんと楽譜キー処理.
//================================================================================================
void PCGC_HomeTopMenu::procPrintMusic( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	if( True == PNLC_SnapshotControlLib::newInstance()->getSkipCheckHDD() )
	{
		// 電源復帰時のHDDチェックスキップ中が使用不可.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	if( True == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_WebBrowser ) ) {
		// 現在の指定アプリ遷移先状態を保存し、状態を「待機中」に更新する.
		PNLE_SpecifiedAppState e_State = c_ExtPanel.getSpecifiedAppState();
//comment 2017/07/12 SCD2-Sky後藤 [IT5_4.1][1026344]ジョブ予約キー表示されない問題.
		onlyPanel.setTopMenuSpecifiedAppState( e_State );
		c_ExtPanel.ChangeSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
		PMessageArea::procBrowser( option );
//comment 2017/07/12 SCD2-Sky後藤 [IT5_4.1][1026344]ジョブ予約キー表示されない問題.
		onlyPanel.setTopMenuSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
		// 遷移できる場合のみ.
		if( NULL != nextGamen ) {
			APIC_SystemStatus::newInstance()->setPrintScoreModeOn( True );  //  「ぷりんと楽譜」モードON.
			onlyPanel.setTopmenuMode( False );  //  メニューキー受付.
		}
		// 統合メニューからポップアップ表示を行う警告が発生していなかった場合のみ.
		// (画面遷移が発生していない).
		else if( False == c_ExtPanel.checkPopupWarningFromTopMenu() ) {
			c_ExtPanel.ChangeSpecifiedAppState( e_State );  // 指定アプリ遷移先状態を元に戻す.
		}
	} else {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： ユビキタスプリント.
//================================================================================================
void PCGC_HomeTopMenu::procUbqPrint( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	if( True == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Print ) ) {	// プリンター印字の可否チェック.
		// 現在の指定アプリ遷移先状態を保存し、状態を「待機中」に更新する.
		PNLE_SpecifiedAppState e_State = c_ExtPanel.getSpecifiedAppState();
//comment 2017/07/12 SCD2-Sky後藤 [IT5_4.1][1026344]ジョブ予約キー表示されない問題.
		onlyPanel.setTopMenuSpecifiedAppState( e_State );
		c_ExtPanel.ChangeSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
		PMessageArea::procUbqPrint( option );
//comment 2017/07/12 SCD2-Sky後藤 [IT5_4.1][1026344]ジョブ予約キー表示されない問題.
		onlyPanel.setTopMenuSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
		if( NULL != nextGamen ) {					// [CQ.355542]遷移できる場合のみ.
			onlyPanel.setTopmenuMode( False );		// [CQ.346271]メニューキー受付け対応.
		}
		// 統合メニューからポップアップ表示を行う警告が発生していなかった場合のみ.
		// (画面遷移が発生していない).
		else if( False == c_ExtPanel.checkPopupWarningFromTopMenu() ) {
			c_ExtPanel.ChangeSpecifiedAppState( e_State );	// 指定アプリ遷移先状態を元に戻す.
		}
// Comment 2016/02/10 HCLT-Krithika [ZeusS_ZX0][制御構成変更_No 76][WP 2555] [CQ522124_Reopen]Not required - already handled in PMessageArea::procUbqPrint.
//// Comment 2015/10/30 HCLT-Vivek [ZeusS_ZX0][制御構成変更_No 76][WP 2555] Counter value to be udpated for ubiquitous print.
//		PnlComIncreaseOperateCnt(TYPE_OperateCnt_Ubiquitous);				//Counter value updation.
	} else {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： コントローラ設定.
//================================================================================================
void PCGC_HomeTopMenu::procControlMenu( uchar option )
{
	// MFP基本機能使用設定が無効の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
// comment 2016/02/29 Sky竹内 [特ROM][FQ-OPSD-1500227-OFP162] ユーザー認証時、ログインしないとトップメニューのFieryアイコンを押しても反応しない.
	// OFP162の対応以降、【ログアウト状態】でもFieryを起動できるように対応しているが、.
	// ユーザー認証：ログイン、部門管理：ログアウトの場合については【ログイン状態】とも【ログアウト状態】とも言ない.
	// そのため、OFP162の対応スコープとして、ユーザー認証ログイン中（public以外）かつ、
	// 部門認証未ログインの場合はFieryを起動させないように対応する.
	if (TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication(PNLC_NetworkLan::getInstance()->getLine())	// ユーザー認証有効時.
	 && NULL != PnlComGetUser()																	// ユーザー認証：ログイン中.
	 && TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance()->getLine())							// 部門管理有効時.
	 && NULL == PnlComGetTrack())																// 部門管理：ログアウト中.
	{
		// キーを受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

// comment 2016/02/29 Sky竹内 [特ROM][FQ-OPSD-1500227-OFP162] ユーザー認証時、ログインしないとトップメニューのFieryアイコンを押しても反応しない.
	onlyPanel.bl_IsFieryKey = True;

	if( True == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_WebBrowser ) ) {
		// 現在の指定アプリ遷移先状態を保存し、状態を「待機中」に更新する.
		PNLE_SpecifiedAppState e_State = c_ExtPanel.getSpecifiedAppState();
//comment 2017/07/12 SCD2-Sky後藤 [IT5_4.1][1026344]ジョブ予約キー表示されない問題.
		onlyPanel.setTopMenuSpecifiedAppState( e_State );
		c_ExtPanel.ChangeSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
		PMessageArea::procBrowser( option );

//comment 2017/07/12 SCD2-Sky後藤 [IT5_4.1][1026344]ジョブ予約キー表示されない問題.
		onlyPanel.setTopMenuSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
		if( NULL != nextGamen ) {
			onlyPanel.setTopmenuMode( False );
		}
		// 統合メニューからポップアップ表示を行う警告が発生していなかった場合のみ.
		// (画面遷移が発生していない).
		else if( False == c_ExtPanel.checkPopupWarningFromTopMenu() ) {
			c_ExtPanel.ChangeSpecifiedAppState( e_State );	// 指定アプリ遷移先状態を元に戻す.
		}
	} else {
		// ブラウザ機能が無効の場合は無効警告を発生させる.
		// 警告の引数は確定後に差し替える.
		onlyPanel.setPanelWarning( PNL_ERROR_UserCertificate );
	}

// comment 2016/02/29 Sky竹内 [特ROM][FQ-OPSD-1500227-OFP162] ユーザー認証時、ログインしないとトップメニューのFieryアイコンを押しても反応しない.
	onlyPanel.bl_IsFieryKey = False;
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： コールバック.
//================================================================================================
void PCGC_HomeTopMenu::procCallBack( TYPE_PNL_CallBack option )
{

	if( TYPE_PNL_CallBack_ProgramKeySearchResult == option ) {		// プログラム宛先検索結果の場合.
		TYPC_ProgramKey* pc_ProgramKey = c_AddressSearchIGC.getProgramKeyDataPointer( 1 );
		PCGC_TopmenuProgramKeyData::getInstance()->setShortCutProgramKeyData( pc_ProgramKey, pc_ProgramKeyDataGetForTopMenu.getProgramGetNum() );
		pc_ProgramKeyDataGetForTopMenu.addProgramGetNum();
		short _ss_ScanFaxProgramNo = pc_ProgramKeyDataGetForTopMenu.getProgramNoForTopMenu( pc_ProgramKeyDataGetForTopMenu.getProgramGetNum() );
		if( 0 < _ss_ScanFaxProgramNo ) {
			// プログラムキーの検索が必要な場合は検索する.
			pc_ProgramKeyDataGetForTopMenu.requestTopMenuProgramKey( _ss_ScanFaxProgramNo );
		} else {
			pc_ManageIGCWaitDisplay.setDisplayFLGOff();
// comment 2016/10/06 MSE岡田 [IT5_3.3][1005694] 検索完了後プログラムキーの活性状態を更新するためフラグをたてる.
			m_bl_UpdateProgramKeyStatus = True;
// comment 2017/11/21(火) SCD2-Sky長谷川(伸) [FQ-OPSD-1700523-OFP264].
			pc_ProgramKeyDataGetForTopMenu.setIGCProgramSearchStatus( PNLD_TOPMENU_PROGRAM_IGCWAITSEARCH_SEARCH_END );
			initializeListView();		// 画面開始共通処理.
			callBackQtShow( 0 );		// Qt描画要求.
			setupGamen(0);				// 画面のチラツキ抑制の為コール.
		}
	} else if( TYPE_PNL_CallBack_TopMenu_UserChange == option ) {
		// トップメニューが表示中でも ログインしているユーザー、部門が変更されていれば更新する必要がある .
		if( this == PNLC_PopupListControl::newInstance()->getNowPopup() ) {
			short _ss_ScanFaxProgramNo = pc_ProgramKeyDataGetForTopMenu.getProgramNoForTopMenu( 0 );
			pc_ProgramKeyDataGetForTopMenu.clearProgramGetNum();
			if( 0 < _ss_ScanFaxProgramNo ) {
				pc_ProgramKeyDataGetForTopMenu.requestTopMenuProgramKey( _ss_ScanFaxProgramNo );
				pc_ManageIGCWaitDisplay.initManageIGCWaitDisplay( PCGD_IGCDisplayStandard, PCGE_IGCClearPopupDispFlgChange, this, this ) ;
				gp_HomeTopMenu.nextWaitDisplay_popup( False );
			} else {
				PNLC_PopupListControl::newInstance()->setChangePopup( this );	// A072002000_トップメニュー（統合トップメニュー）.
			}
		}
	} else if( TYPE_PNL_CallBack_RemotePanelStop == option ) {
		procActivate();
	} else if( TYPE_PNL_CallBack_QtHomeTopMenuDraw_Show == option ) {		// ビューへ表示指示.
		if( True == PNLC_PopupListControl::newInstance()->isNowPopup( this ) ) {
			QtShow();
		}
	} else if( TYPE_PNL_CallBack_LoadTopMenuCustomSuccess == option ) {	// Topメニュー背景カスタマイズ読込み完了. 2014/02/06 s.ohba 操作性改善FUM5 CQ.384817 トップメニューテーマ表示されない不具合対応.
		PNLC_UICustomDataManager* pc_UICustomMgr = PNLC_UICustomDataManager::newInstance();
		if( NULL != pc_UICustomMgr ) {
			const uchar* puc_Data = pc_UICustomMgr->getTopMenuThemeImage();
			if( NULL != puc_Data ) {
				Bak_HomeTopMenuUserThemeBackground.changeRawGraphic( puc_Data );
				refreshNowGamen();
			}
		}
	} else if( TYPE_PNL_CallBack_UICustomReqSetTopMenuTheme == option ) {
		PNLC_UICustomDataManager::newInstance()->setTopMenuTheme();
	} else {
		PPopupAnimationCom::procCallBack( option );
	}
}
//comment 2019/02/19 [HCLT-Sai] [IT6_EagleH] [OP_BTS-2425,OP_BTS-2523] Display Warning Button click issue fix.
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ：SetDispWarningkey 
//================================================================================================
void PCGC_HomeTopMenu::setDispWarningkey( Bool bl_WarningOpenDispKey )
{
		m_bl_WarningOpenDispKey = bl_WarningOpenDispKey;
}
// comment 2015/01/26 OPS1-MSE三浦 [ZeusS][CQ_509326][RQ-232] RAユーザ事前認証取得状態通知対応.
//================================================================================================.
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： オブザーバ.
//================================================================================================.
void PCGC_HomeTopMenu::procObserver( TYPE_PNL_Observer option )
{
	if( TYPE_PNL_Observer_dataChanged_AndroidPanel == option ) {	// 認証装置スキャン結果通知.
		// バッジキーに表示する情報の設定.
		initializeListView();

		// Qt描画要求.
		callBackQtShow(0);
		
		// setupGamen()をコールしちらつきを抑制
		setupGamen(0);

// comment 2016/01/30 OPS1-Sky稲葉 [ZeusS_ZX0][CQ524248]ダウンロード完了のイベント通知.
	} else if ( TYPE_PNL_Observer_dataChanged_IWSMarketPlaceAppResultInfo == option ) {	// MarketPlaceダウンロードオブザーバ : データ変化通知.
		if( True == getDBInitializeStatus() ) {			// DB初期化完了の場合.
			initRegisterApp();			// アプリ登録情報初期化.
			m_bl_isDBInit = True;
		}

		// バッジキーに表示する情報の設定.
		initializeListView();

		// Qt描画要求.
		callBackQtShow(0);
		
		// setupGamen()をコールしちらつきを抑制
		setupGamen(0);
//Comment 2019/04/09 【HCLT-Neelima】 【IT6_MR2】 【AR_1075335】 Displaying shortcut key in Basic style & Classic style screens.
		} else if(TYPE_PNL_Observer_dataChanged_PanelSetting == option) {
		// バッジキーに表示する情報の設定.
		initializeListView();
		// Qt描画要求.
		callBackQtShow(0);
		
		// setupGamen()をコールしちらつきを抑制
		setupGamen(0);
//Comment 2019/04/09 【HCLT-Arun】 【IT6_EagleBK】 【OP_BTS-11570】 Language change display support, when langauge changed in simlin.
         //} else { 
         } else if ( TYPE_PNL_Observer_notifyLanguage == option || TYPE_PNL_Observer_notifySelectLanguage == option) {
         	PPopupAnimationCom::procObserver( option );
         	// バッジキーに表示する情報の設定.
         	initializeListView();
         	// Qt描画要求.
         	callBackQtShow(0);
                                
         	// setupGamen()をコールしちらつきを抑制
         	setupGamen(0);
         } else {

		PPopupAnimationCom::procObserver( option );
	}
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： ボタン（押された.
//================================================================================================
void PCGC_HomeTopMenu::procButtonOn( uchar option )
{
	//ヘッダエリアのジョブ表示キーが押下された場合.
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//// comment 2018/05/08 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
//	if( (getBackupActiveParts() == ( PDisplayParts* )&Cmp_HomeTopMenuHeaderAreaJobList)
//	||  (getBackupActiveParts() == ( PDisplayParts* )&Cmp_HomeTopMenuHeaderAreaJobListHomeMenu)) {
	if ( getBackupActiveParts() == ( PDisplayParts* )&Cmp_HomeTopMenuHeaderAreaJobList ) {
		//画面固有の処理を行いたいのでoption値を書き換える.
		option = 1;
	}


	PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();
	if( NULL != pc_PopupListControl ) {
		short ss_XPos, ss_YPos, ss_XSize, ss_YSize = 0;
		switch( option ) {
// comment 2019/05/20 SCD2-Sky蒲原 [EagleH][OP_BTS-5051] Topメニューで一時言語切替のポップアップが出ない.
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//// comment 2018/04/18 SCD2 Kazuo.Takei [Eagle/Sparrow][制御構成変更_No.111 P07_01_共通仕様(第6世代UI画面_左エリア)].
//			case PNLE_BTNID_LeftAreaLanguage:		// 左エリアコンポーネントLanguageキー.
			case 0:
				Key_HomeTopMenuLanguage.getPosition( ss_XPos, ss_YPos );
				Key_HomeTopMenuLanguage.getSize( ss_XSize, ss_YSize );
// Comment 2015/10/26 HCLT-Vivek [ZeusS_ZX0][制御構成変更_No 76][WP 2500] Counter value to be updated.
				PnlComIncreaseOperateCnt( TYPE_OperateCnt_LanguageKey );
				pc_PopupListControl->addPopup( &gp_A072002900, this, NULL, ss_XPos, ss_YPos, ss_XSize, ss_YSize );		// A072002900_言語一時変更.
				break;
			case 1:
				//initAllSelectJobInfo( False, True );		// 全関数タイプ選択ジョブ初期化関数.
				//initJobListInfo();							// ジョブリスト初期化関数.
				//onlyPanel.b_JobListGamen = True;			// ジョブ確認画面ステータス制御.
				//Key_HomeTopMenuJobList.getPosition( ss_XPos, ss_YPos );
				//Key_HomeTopMenuJobList.getSize( ss_XSize, ss_YSize );
				Cmp_HomeTopMenuHeaderAreaJobList.procJobList();
				if( True == ScanOapExtSet.isActiveGeneralApplication()
				    && TYPE_ExtTypeScanD == ScanOapExtSet.getExtAppType()
				    && True == ScanOapExtSet.getJobExecutionState() ) {
					ScanOapExtSet.setScanJobSpecBackUp();
				}
				break;
			case 2:
				uc_nowPage--;
				setupGamen( option );
				break;
			case 3:
				uc_nowPage++;
				// ページ切り替え時のsetupGamenを判別するため、optionを渡す.
				setupGamen( option );
				break;
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//// comment 2018/04/18 SCD2 Kazuo.Takei [Eagle/Sparrow][制御構成変更_No.111 P07_01_共通仕様(第6世代UI画面_左エリア)].
//			case PNLD_BTNID_LeftAreaLoginInfo:		// 左エリアコンポーネント認証情報キー／ネットワーク選択キー.
			case PNLE_BTNID_HomeTopMenu_LogInInfo:	// 認証情報表示エリア.
// comment Sky東島 [RQ-686] 複合機の複数ネットワーク対応.
				{
					Bool bl_IsShowNWEnd = PnlComIsShowNWEndCheck();
					if( ( True == m_bl_VlanNWShow )
					 && ( False == bl_IsShowNWEnd ) ){
						// ユーザ認証中であれば、初期化する.
						if( True == PnlComGetUserAccountOn() ) {
							PnlComInitUserAccountInfo( True );
						} else {
							// 認証していない場合はネットワーク情報を初期化.
							PNLC_NetworkLan::getInstance()->setLine( TYPE_NetworkSpecific_NONE );
						}
						rebuildGamen( 0 );
					} else if( True == bl_IsShowNWEnd ){
						procIDInput( 0 );
					} else {
						Key_HomeTopMenuLogInInfo.getPosition( ss_XPos, ss_YPos );
						Key_HomeTopMenuLogInInfo.getSize( ss_XSize, ss_YSize );
						pc_PopupListControl->addPopup( &gp_UserAccountUserInfoConfirmation_act, this, NULL, ss_XPos, ss_YPos, ss_XSize, ss_YSize );	// D070704100_認証情報確認.
					}
				}
				break;
// comment : WebUI2 画面切替え制御 -> 20180302 Chikuan.
//			case 150:
//				if( PNLE_UIModeLegacyUI == PNLC_WebUIControl::getInstance()->getUIMode() ){
//					procUIMode( 0 );
//					setupGamen();
//				}
//				break;
// comment : WebUI2 画面切替え制御 <- 20180302 Chikuan.
			default :	break;
		}
	}
}

/**
 *	@brief	画面終了処理.
 *	@param	無し.
 *	@retval	void.
 */
void PCGC_HomeTopMenu::endGamen()
{
	if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW212, 1 ) ) {		//DipSW212 bit1=ON
		m_bl_MenuEnable = True;					// メニューキー受付可にする.
	}

	this->m_sl_SupportUserNo  = 0 ;
	this->m_sl_SupportTrackNo = 0 ;
	PnlComMtuiIf_TopMenuView::instance()->hide();

	//既存コンポーネントインスタンス設定.
	PnlComMtuiIf_TopMenuModel::instance()->setComponent( NULL );
	PnlComMtuiIf_TopMenuView::instance()->setComponent( NULL );

	// テーマによる背景パーツのキャッシュ削除.
	switch( PnlComGetSystemSetting()->getTopMenuCustomPattern() ) {
		case TYPE_TopMenuCustomPattern1: Bak_HomeTopMenuBackground[0].clearCache(); break;
		case TYPE_TopMenuCustomPattern2: Bak_HomeTopMenuBackground[1].clearCache(); break;
		case TYPE_TopMenuCustomPattern3: Bak_HomeTopMenuBackground[2].clearCache(); break;
		default:	break;
	}

	// 親のコール .
	PPopupAnimationCom::endGamen() ;

	if( True == PNLC_HitachiICC_TopMenuCustomize::newInstance()->isHitachiICC_Mode() ) {	// 日立ICCモード時は、トップメニューのQt表示を無効にする.
		PLBC_QtListNotUse::getInstance()->setQtListNotUseFlg( False );
	}
}
/**
 *	@brief	画面活性処理.
 *	@param	無し.
 *	@retval	void.
 *	@history	2014/12/02 OPS1-Sky古池 [Poseidon][CQ 504180] ウィジェット初期化処理追加.
 */
void PCGC_HomeTopMenu::procActivate()
{
//Comment 2019/01/11 [HCLT_Prashannaa] [AR 1071417] Opening HomeTopMenu in the admin set UI Mode.
	PNLE_UIMode	e_NowUIMode_bak = PNLC_WebUIControl::getInstance()->getUIModeBackup();
	PNLE_UIMode	e_NowUIMode = PNLC_WebUIControl::getInstance()->getUIMode(); 	// UIモード取得.
	
	if ( e_NowUIMode_bak != e_NowUIMode ) {
		PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();
		if ( NULL != pc_PopupListControl ) {				
			pc_PopupListControl->setChangePopup(&gp_HomeTopMenu );
			return;
		}
	}
	// comment 2015/10/20 OPS1-川手 [特ROM対応][FQ-OPSD-1500259-OFP185]Fax受信時に受信エラー(R27)が頻繁に発生する.
	m_bl_UpdateProgramKeyStatus = True;
	if( True == PNLC_HitachiICC_TopMenuCustomize::newInstance()->isHitachiICC_Mode() ) {	// 日立ICCモード時は、トップメニューのQt表示を無効にする.
		PLBC_QtListNotUse::getInstance()->setQtListNotUseFlg( True );
	}

// comment 2019/06/27 SCD2-Sky谷尾 [EagleZ+][OP_BTS-7397]ウィジェット再描画処理追加,
	PM_TaskID uc_PriTaskID;								// 最優先タスク取得.
	PMGC_PanelManager::getPriority( &uc_PriTaskID );
	if( PM_TASKID_PANEL != uc_PriTaskID )
	{
		m_bl_PriorityTaskBrw = True;
	}
// comment 2014/12/01 OPS1-Sky古池 [Poseidon][CQ 504180] 自画面に戻ってきた際にウィジェット再表示する為、再度初期化.
	Cmp_GadgetComponent.initialize( APIE_GadgetShowScreenTopMenu );		// ウィジェットコンポーネント初期化.
	
	// ユーザー切り替え判定.
	checkSupporNowLoginUserAndTrack();

	if( True == getDBInitializeStatus() ) {			// DB初期化完了の場合.
		initRegisterApp();			// アプリ登録情報初期化.
		m_bl_isDBInit = True;
	}
	initializeListView();

	callBackQtShow( 0 );		// Qt描画要求.

	// 親クラスのメソッドをコール.
	PPopupAnimationCom::procActivate();

//comment 2023/03/29 FPT.HungDM17 [IT6_2.3]【OP_BTS-42606】Legacyのトップメニュー画面にメンテナンスモード実行中バーが表示されない問題対策.
	if( NULL != m_pc_MaintenanceModeStatusBar )	{
		if ( True == PnlComGetSystemInfo()->getEnableMaintenanceMode() ) {
			GFWC_Common* pc_GUISelf = static_cast< GFWC_Common* >( m_pc_MaintenanceModeStatusBar->getGUIParts() );
			if( NULL != pc_GUISelf ) {
				pc_GUISelf->bringToFront();
				DEF_UPDATEDRAWPARTSINFO_GUI;
			}
		}
	}
// comment 2020/09/25 Sky中野 [IT6_2.1][OP_BTS-27503].
	if( True == gp_ConfirmProgramDestinationSelect_warn.getSelectOkFlg() ) {
		if( 0 != m_ss_ConfirmationProgramNo ) {
			// プログラム宛先選択確認警告で「はい」が選択された場合、プログラム呼び出しを実行.
			procFax( 0 );
			if( ( NULL != nextGamen )
			 || ( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_IGCClearWait_popup ) ) ) {
				TYPC_ProgramKey* pc_ProgramKey = PCGC_TopmenuProgramKeyData::getInstance()->getProgramKeyData( m_ss_ConfirmationProgramNo );
				if( NULL != pc_ProgramKey ) {
					gp_ProgramBasic_scanfax.setProgramKeyData( m_ss_ConfirmationProgramNo, pc_ProgramKey );
				}
			}
		}
	}
	m_ss_ConfirmationProgramNo = 0;
	return ;
}

/**
 *	@brief	非アクティブ状態の通知.
 *	@param	無し.
 *	@retval	void.
 */
void PCGC_HomeTopMenu::procInactivate() {
// comment 2016/01/21 OPS1-MSE秋山 [ZeusS_ZX0][CQ521597]ポップアップ表示でリストキーが非表示されているので、非表示としないように変更.
//	// 現在のスクロールバー位置(ページ番号)を保持.
//	// ビューを非表示.
//	PNLC_SnapshotControlLib* pc_SnapshotControlLib = PNLC_SnapshotControlLib::newInstance();
//	if( NULL == pc_SnapshotControlLib ) {
//		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
//		return;
//	}
//	// スナップショット作成処理では透明画面をポップアップする為、Widgetをクローズしない.
//	if( False == pc_SnapshotControlLib->getStatusSnapshotCreating() ) {
//		PnlComMtuiIf_TopMenuView::instance()->hide();
//	}
	PPopupAnimationCom::procInactivate();	//処理：親クラス処理.

	if( True == PNLC_HitachiICC_TopMenuCustomize::newInstance()->isHitachiICC_Mode() ) {	// 日立ICCモード時は、トップメニューのQt表示を無効にする.
		PLBC_QtListNotUse::getInstance()->setQtListNotUseFlg( False );
	}
// comment 2016/03/24 OPS1-Sky佐藤 [ZeusS_ZX0][CQ526692] 外部メモリーキーの非表示対応見直し.
	m_bl_isUSBInit = False;
}

const uchar PNLD_TopMenuIconTypeMax	= uchar( 24 );	// アイコン種類の数.
const uchar PNLD_TopMenuKeyDisp		= uchar( 12 );	// １画面に表示されるキー数.
// comment 2018/04/04 SCD2 K.Nishiki [Eagle][制御構成変更No.111]ホーム画面認証画面の新UI化対応.
const uchar PNLD_TopMenuKeyDisp2		= uchar( 8 );	// １画面に表示されるキー数(Homeメニュー).

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： 画面遷移ボタン処理.
//================================================================================================
void PCGC_HomeTopMenu::procGoToPage( uchar option )
{
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//// comment : WebUI2 画面切替え制御 -> 20180302 Chikuan.
//	if( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() ){
//		procGoToPageLightUI( option );
//		return;
//	}
//// comment : WebUI2 画面切替え制御 <- 20180302 Chikuan.
// comment 2023/04/17 Sky中野 [IT6_2.3][OP_BTS-42686].
	PNLC_WebUIControl* pc_WebUIControl = PNLC_WebUIControl::getInstance();
	if( NULL == pc_WebUIControl ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "[PCGC_HomeTopMenu::procGoToPage]Assert Failed!" );
		return;
	}

	// トップメニュー画面不具合修正.
// comment 2018/04/04 SCD2 K.Nishiki [Eagle][制御構成変更No.111]ホーム画面認証画面の新UI化対応.
	uchar _uc_index = static_cast<uchar>( ( uc_nowPage - 1 )*m_uc_TopMenuListKeyDisp + option );

	if( PNLE_BTNID_HomeTopMenu_FunctionSearch == option ) {	// 機能検索キーの場合.
// comment 2015/04/23 OPS1-Sky浜野 [Poseidon][CQ513081] 機能遷移不可の場合も遷移できる不具合対応.
		if( ( False == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Copy ) )
		&&	( False == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Fax ) )
		&&	( False == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_BoxOperation ) )
		&&	( False == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_WebBrowser ) ) ) {
			setBuzzerType( PCGE_BuzzerType_InputNg );
			return;
		}
		
		gp_FunctionSearch_act.setSearchMode( PNLD_SearchFunctionApp );
		// ポップアップ管理クラスのインスタンス取得.
		PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();
// comment 2015/1/07 OPS1-Sky東島 [ZeusS][CQ508080]トップメニューの機能検索画面でメニュー押下後、再度機能検索押下しても起動しない問題対応.
//		pc_PopupListControl->addPopup( &gp_FunctionSearch_act, this, NULL, 0, 0, 0, 0, True );		// 機能検索画面.
		if( True == pc_PopupListControl->isPopupLayer( &gp_FunctionSearch_act ) ){
			// 機能検索画面がレイヤー上に存在する場合はトップメニューを閉じる.
			pc_PopupListControl->closeTop( False );
		} else {
// comment 2015/04/02 MSE田中優介 [MinervaSBK][RQ-278] LDAPサーバーへのカード情報登録機能対応.
			// LDAPサーバーへのカード情報登録機能の終了処理.
			endAutoCardRegist();
// Comment 2015/10/26 HCLT-Vivek [ZeusS_ZX0][制御構成変更_No 76][WP 2501] Counter value to be updated.
          PnlComIncreaseOperateCnt(TYPE_OperateCnt_FuncSearchKey);                          //Counter value  updation.
		pc_PopupListControl->addPopup( &gp_FunctionSearch_act, this, NULL, 0, 0, 0, 0, True );		// 機能検索画面.
		}
		return ;
	}

	_uc_index = option;
	PNLE_TopMenuShortCut _e_TopMenuShortCut = st_BadgeKeyInfo[_uc_index].e_ShortCutKind;
	PNLE_ShortCutID _e_ShortCutID = PNLE_ShortCutID_NON;
	PNLC_ChangeGamenManager* pc_ChangeGamenManager = PNLC_ChangeGamenManager::newInstance();
	PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();
	if( PnlComGetUserAccountOn() && NULL != PnlComGetUser() && TYPE_UserTypeAdmin == PnlComGetUser()->getUserType()
	    && False == c_SystemControl.getAdminModeOn() ) {
		//管理者ログイン中の場合.
// comment 2020/02/04 SCD2-Sky K.Kasemura [HeliosMLK][OP_BTS-15281]管理者権限でログイン時、ベーシックスタイルキーを受付可能とする.
//		if( PNLE_TopMenuShortCut_Utility != ( _e_TopMenuShortCut & 0xFF00 ) ) {
	    if( ( PNLE_TopMenuShortCut_Utility != ( _e_TopMenuShortCut & 0xFF00 ) ) && //設定キー
	    	( PNLE_TopMenuShortCut_UIMode != ( _e_TopMenuShortCut & 0xFF00 ) ) ) { //ベーシックスタイルキー
			setBuzzerType( PCGE_BuzzerType_InputNg );
			return;
		}
	}
// comment 2015/09/01 OPS1-Sky稲井[特ROM][MZV-104_機種マージ]認証の無い環境下での操作者自身宛スキャンの実現要望対応.
	if( (True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW226 , 2 )) // DipSWNo.226 bit2 ON.
	  &&(PNLE_TopMenuShortCut_ScanFax == (_e_TopMenuShortCut & 0xFF00))	// スキャン/ファクス機能設定.
	  &&(7 == _uc_index)){												// 8番目のキー.
		APIC_ProgramKey* pc_ProgramKey = APIC_ProgramKey::newInstance(1);	//APICのインスタンス取得(引数は1とする).
		if(NULL != pc_ProgramKey){
			Bool bl_AllDeleteFlag = pc_ProgramKey->getProgramKeyAlldeleteStatsu();	//削除中状態を取得.
			if(True == bl_AllDeleteFlag){
				//プログラム宛先、一時配信削除中の場合は、画面遷移させない.
				setBuzzerType( PCGE_BuzzerType_InputNg );
				return;
			}
		}else{
			LOGF_debug_PutEventLog(LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!");
		}
	}
	std::string c_IwsAppName, s_args;
	int p_processId = 0;
	short ps_appId = -1;
	ushort us_cntIws = 1;
	char* pc_args;
	const char* IwsAppName;
	switch( _e_TopMenuShortCut & 0xFF00 ) {
		case PNLE_TopMenuShortCut_Func:				// 機能.
			switch( _e_TopMenuShortCut ) {
				case PNLE_TopMenuShortCut_FuncCopy:		procCopy( 0 );		break;	// 機能:コピー.
				case PNLE_TopMenuShortCut_FuncScanFax:	procFax( 0 );		break;	// 機能:スキャン/ファクス.
				case PNLE_TopMenuShortCut_FuncBox:		procBox( 0 );		break;	// 機能:ボックス.
				case PNLE_TopMenuShortCut_FuncUbqPrint:	procUbqPrint( 0 );	break;	// 機能:ユビキタスプリント.
				case PNLE_TopMenuShortCut_FuncBrowser:	procBrowser( 0 );	break;	// 機能:ブラウザ.
				case PNLE_TopMenuShortCut_ScanToDevice:	changeScanToDevice( 0 );	break;	// 機能:ScanToDevice.
				case PNLE_TopMenuShortCut_FuncOAPApp:	procAPPsMenu( 0 );	break;	// 機能:OpenAPIアプリ.
				case PNLE_TopMenuShortCut_HitachICC_SecurityScan:
				{
					if( ( PNLE_HitachICC_ScanFaxShortCut_Fax == PNLC_HitachiICC_TopMenuCustomize::newInstance()->getFaxScanShortCutMode() )
					    && ( NULL != cp_scanfaxJob )
					    && ( TYPE_InputStatusIdle == cp_scanfaxJob->getJobStatus()->getInputStatus() )
					    && ( TYPE_FaxJobStatusIdle == cp_scanfaxJob->getJobStatus()->getFaxJobStatus( TYPE_CommLine1 ) )
					    && ( TYPE_FaxJobStatusIdle == cp_scanfaxJob->getJobStatus()->getFaxJobStatus( TYPE_CommLine2 ) )
					    && ( TYPE_FaxJobStatusIdle == cp_scanfaxJob->getJobStatus()->getFaxJobStatus( TYPE_CommLine3 ) )
					    && ( TYPE_FaxJobStatusIdle == cp_scanfaxJob->getJobStatus()->getFaxJobStatus( TYPE_CommLine4 ) ) ) {
						c_JobControl.InitJobSetting( cp_scanfaxJob );		// スキャン/ファクスのリセット処理.
					}
					PNLC_HitachiICC_TopMenuCustomize::newInstance()->setFaxScanShortCutMode( PNLE_HitachICC_ScanFaxShortCut_SecurityScan );

					APIC_FuncPatternSetting* p_FuncPatternSettingFaxScan = APIC_FuncPatternSetting::newInstance( TYPE_PNLPTN_SETTING_TYPE_FaxScan );
					if( NULL != p_FuncPatternSettingFaxScan ) {
						TYPS_PNLPTN_InfoList s_InfoList;	// パターン情報一覧.
						p_FuncPatternSettingFaxScan->getPatternInfo( &s_InfoList );
						if( True == s_InfoList.s_Info[TYPE_PNLPTN_TYPE_Custom1].b_Enable ) {
							p_FuncPatternSettingFaxScan->setSelectPattern( TYPE_PNLPTN_TYPE_Custom1 );
						}
						procFax( 0 );
					} else {
						//例外処理のログを残す.
						LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
					}
					break;
				}
				case PNLE_TopMenuShortCut_HitachICC_Fax:
				{
					if( ( PNLE_HitachICC_ScanFaxShortCut_SecurityScan == PNLC_HitachiICC_TopMenuCustomize::newInstance()->getFaxScanShortCutMode() )
					    && ( NULL != cp_scanfaxJob )
					    && ( TYPE_InputStatusIdle == cp_scanfaxJob->getJobStatus()->getInputStatus() )
					    && ( TYPE_FaxJobStatusIdle == cp_scanfaxJob->getJobStatus()->getFaxJobStatus( TYPE_CommLine1 ) )
					    && ( TYPE_FaxJobStatusIdle == cp_scanfaxJob->getJobStatus()->getFaxJobStatus( TYPE_CommLine2 ) )
					    && ( TYPE_FaxJobStatusIdle == cp_scanfaxJob->getJobStatus()->getFaxJobStatus( TYPE_CommLine3 ) )
					    && ( TYPE_FaxJobStatusIdle == cp_scanfaxJob->getJobStatus()->getFaxJobStatus( TYPE_CommLine4 ) ) ) {
						c_JobControl.InitJobSetting( cp_scanfaxJob );		// スキャン/ファクスのリセット処理.
					}
					PNLC_HitachiICC_TopMenuCustomize::newInstance()->setFaxScanShortCutMode( PNLE_HitachICC_ScanFaxShortCut_Fax );

					APIC_FuncPatternSetting* p_FuncPatternSettingFaxScan = APIC_FuncPatternSetting::newInstance( TYPE_PNLPTN_SETTING_TYPE_FaxScan );
					if( NULL != p_FuncPatternSettingFaxScan ) {
						TYPS_PNLPTN_InfoList s_InfoList;	// パターン情報一覧.
						p_FuncPatternSettingFaxScan->getPatternInfo( &s_InfoList );
						if( True == s_InfoList.s_Info[TYPE_PNLPTN_TYPE_Custom2].b_Enable ) {
							p_FuncPatternSettingFaxScan->setSelectPattern( TYPE_PNLPTN_TYPE_Custom2 );
						}
						procFax( 0 );
					} else {
						//例外処理のログを残す.
						LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
					}
					break;
				}
				case PNLE_TopMenuShortCut_FuncFaxOnly:	procFaxOnly( 0 );		break;	// 機能:ファクス.
				default :	break;
			}
			break;
		case PNLE_TopMenuShortCut_Copy:				// コピー機能設定.
			procCopy( 0 );
			if( NULL != nextGamen ) {
				switch( _e_TopMenuShortCut ) {
					case PNLE_TopMenuShortCut_CopyColorMode:		_e_ShortCutID = PNLE_ShortCutID_COPY_Color;			break;	// コピー機能設定:カラーモード.
					case PNLE_TopMenuShortCut_CopyPaperType:		_e_ShortCutID = PNLE_ShortCutID_COPY_Paper;			break;	// コピー機能設定:用紙設定.
					case PNLE_TopMenuShortCut_CopyMag:				_e_ShortCutID = PNLE_ShortCutID_COPY_Zoom;			break;	// コピー機能設定:倍率設定.
					case PNLE_TopMenuShortCut_CopyPage:				_e_ShortCutID = PNLE_ShortCutID_COPY_DuplexCombine;	break;	// コピー機能設定:両面/ページ集約.
					case PNLE_TopMenuShortCut_CopyFinishing:		_e_ShortCutID = PNLE_ShortCutID_COPY_Finishing;		break;	// コピー機能設定:仕上がり.
					case PNLE_TopMenuShortCut_CopyDensity:			_e_ShortCutID = PNLE_ShortCutID_COPY_Density;		break;	// コピー機能設定:濃度.
					case PNLE_TopMenuShortCut_CopyOriginalImage:	_e_ShortCutID = PNLE_ShortCutID_COPY_OrgType;		break;	// コピー機能設定:原稿画質.
					default :	break;
				}
				if( NULL != pc_ChangeGamenManager ) {
					pc_ChangeGamenManager->setShortCut( _e_ShortCutID, PNLE_ShortCutKindID_Copy );
				}
			}
			break;
		case PNLE_TopMenuShortCut_CopyMemory:		// コピー設定メモリ.
			procCopy( 0 );
// comment 2018/10/23 [HCLT_Prem][DenebMLK] [AR1050427] Card Copy Support.
//			if( NULL != nextGamen ) {
//				Cmp_CopyQuickButton.setJobMemoryToJobSpec( st_BadgeKeyInfo[_uc_index].ss_ProgramNo );
//// comment 2017/11/24 SCD2-Sky溝部 [FQ-OPSD-1700500-OFP256] メッセージ警告("プログラムを呼出しました。")表示中にジョブ開始出来ない指摘対応.
//				// 簡単設定キー押下時とトップメニューからの呼び出し時で、m_uc_ProgramCallExecの値の遷移に差異が生じ.
//				// スタートキー押下ですぐにジョブ実行出来なくなるため、簡単設定キーから設定した場合と同じステータスにする必要がある。
//				onlyPanel.setProgramCallStatus( PNLE_ProgramCallExec_Continue );
//			}
			if( NULL != nextGamen ) {
//Comment 2018/12/06 HCLT_Rajkumar [IT6_DenebMLK][AR1068150] To set Card copy mode: ON by default.
				Cmp_CopyQuickButton.setJobMemoryToJobSpec( st_BadgeKeyInfo[_uc_index].ss_ProgramNo );
// comment 2017/11/24 SCD2-Sky溝部 [FQ-OPSD-1700500-OFP256] メッセージ警告("プログラムを呼出しました。")表示中にジョブ開始出来ない指摘対応.
				// 簡単設定キー押下時とトップメニューからの呼び出し時で、m_uc_ProgramCallExecの値の遷移に差異が生じ.
				// スタートキー押下ですぐにジョブ実行出来なくなるため、簡単設定キーから設定した場合と同じステータスにする必要がある。
				onlyPanel.setProgramCallStatus( PNLE_ProgramCallExec_Continue );
//Comment 2018/12/06 HCLT_Rajkumar [IT6_DenebMLK][AR1068150] Added to open Card copy shortcud Menu.				
//				PBaseGamenCom* pc_BaseGamen = dynamic_cast<PBaseGamenCom*>( nextGamen );
//				PPopupCom* pc_PopupCom = &gp_A0201B3000;
//				if( ( NULL != pc_BaseGamen ) && ( NULL != pc_PopupCom ) ) {
//					pc_BaseGamen->addOpenPopup( pc_PopupCom, True );
//					OpenPopupIndi(); 
//				}
// comment 2018/12/07 [HCLT_Neelima][IT6_DenebMLK] [AR1068496] Copy program support.
//				if( NULL != pc_ChangeGamenManager ) {
				if( ( NULL != pc_ChangeGamenManager )
					&& ( st_BadgeKeyInfo[_uc_index].ss_ProgramNo == 30 )
					&& ( True == PnlComGetSoftDipSw()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW153, 0 ) ) ){
//Comment 2019/09/26 HCLT-Taqiuddin [EagleZ+_BKZ+] [OP_BTS-8126] Calling the setBackupFromUtility Function so as to backup the Layout and Zoom values. 
					gp_A0201B3000.setBackupFromUtility();
					pc_ChangeGamenManager->setShortCut( PNLE_ShortCutID_COPY_CardCopy, PNLE_ShortCutKindID_Copy );
				}
			}
			break;
		case PNLE_TopMenuShortCut_ScanFax:			// スキャン/ファクス機能設定.
// comment 2019/07/05 Sky毛穴 [特ROM][FQ-OPSD-1900229-OFP145] DipSW226のbit2がONでトップからのスキャンファクスプログラム呼出し画面への遷移でプログラム情報が書き換わる.
			if( 7 == _uc_index				// 8番目のキー.
				&& True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW226 , 2 )) {		// DipSWNo.226 bit2 ON.
				onlyPanel.m_bl_ScanProgramBasicFromTop = True;
			}
			procFax( 0 );
// comment 2015/09/01 OPS1-Sky稲井[特ROM][MZV-104_機種マージ]認証の無い環境下での操作者自身宛スキャンの実現要望対応. 
			// トップメニューにスキャン／ファクスのプログラム宛先一覧へ遷移する機能が有効か判定.
			if( 7 == _uc_index				// 8番目のキー.
				&& True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW226 , 2 )) {		// DipSWNo.226 bit2 ON.
				pc_ChangeGamenManager->setShortCut( PNLE_ShortCutID_FAXSCAN_ProgramCall, PNLE_ShortCutKindID_ScanFax );			// A039110000_ファクス／スキャンプログラム呼出し画面へ遷移.
				onlyPanel.m_bl_ScanFaxProgramAmiFlag = True;
// comment 2019/07/05 Sky毛穴 [特ROM][FQ-OPSD-1900229-OFP145] DipSW226のbit2がONでトップからのスキャンファクスプログラム呼出し画面への遷移でプログラム情報が書き換わる.
				onlyPanel.m_bl_ScanProgramBasicFromTop = False;		// 判定終了しているのでフラグを落とす.
				break;
			}
			if( 4 == _uc_index				// 5番目のキー.
			    && True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( 193 , 0 )	) {	// DipSWNo.193 bit0 ON.
// Comment 2015/10/28 HCLT-Surajit [ZeusS_ZX0][制御構成変更_No 76][WP 2542] Counter value to be updated.
          		PnlComIncreaseOperateCnt( TYPE_OperateCnt_QRCode );                                                           //     QRコード使用回数
				nextGamen = &gp_QRCodeReadWait_scanfax;			// QRコード読取り待機中画面へ遷移.
				break;
			}
			if( ( NULL != nextGamen )
			    || ( ( NULL != pc_PopupListControl ) && ( True == pc_PopupListControl->isPopupLayer( &gp_IGCClearWait_popup ) ) ) )
			{
				switch( _e_TopMenuShortCut ) {
					case PNLE_TopMenuShortCut_ScanFaxDuplexMode:	_e_ShortCutID = PNLE_ShortCutID_FAXSCAN_DuplexSet;		break;	// スキャン/ファクス機能設定:片面/両面.
					case PNLE_TopMenuShortCut_ScanFaxResolution:	_e_ShortCutID = PNLE_ShortCutID_FAXSCAN_Resolution;		break;	// スキャン/ファクス機能設定:解像度.
					case PNLE_TopMenuShortCut_ScanFaxColor:			_e_ShortCutID = PNLE_ShortCutID_FAXSCAN_Color;			break;	// スキャン/ファクス機能設定:カラー.
					case PNLE_TopMenuShortCut_ScanFaxFileFormat:	_e_ShortCutID = PNLE_ShortCutID_FAXSCAN_FileType;		break;	// スキャン/ファクス機能設定:ファイル形式.
					case PNLE_TopMenuShortCut_ScanFaxReadSize:		_e_ShortCutID = PNLE_ShortCutID_FAXSCAN_ScanSize;		break;	// スキャン/ファクス機能設定:読込みサイズ.
					case PNLE_TopMenuShortCut_ScanFaxEMail:			_e_ShortCutID = PNLE_ShortCutID_FAXSCAN_EmailSetting;	break;	// スキャン/ファクス機能設定:E-mail設定.
// comment 2016/02/02 OPS1-Sky善[特ROM][IT5-14_機種マージ]暗号化PDF操作フロー改善要望.
					case PNLE_TopMenuShortCut_ScanFaxEncryptPDFPassword:	_e_ShortCutID = PNLE_ShortCutID_FAXSCAN_EncryptPDFPassword;	break;	// スキャン/ファクス機能設定:PDF暗号化パスワード.
					default :	break;
				}
				if( NULL != pc_ChangeGamenManager ) {
					pc_ChangeGamenManager->setShortCut( _e_ShortCutID, PNLE_ShortCutKindID_ScanFax );
				}
			}
			break;
		case PNLE_TopMenuShortCut_ScanFaxMemory:	// スキャン/ファクス設定メモリ.
			{
// comment 2020/09/25 Sky中野 [IT6_2.1][OP_BTS-27503].
				m_ss_ConfirmationProgramNo = 0;
				gp_ProgramBasic_scanfax.m_bl_clearAddress = False;
				TYPC_ProgramKey* pc_ProgramKey = PCGC_TopmenuProgramKeyData::getInstance()->getProgramKeyData( st_BadgeKeyInfo[_uc_index].ss_ProgramNo );
				if( ( NULL != PnlComGetFaxPanelSetting() )
				 && ( TYPE_FaxAddDestination_On == PnlComGetFaxPanelSetting()->getFaxAddDestination() )				// 同報宛先の連続選択：許可.
				 && ( NULL != PnlComGetJobScan() )
				 && ( NULL != PnlComGetJobScan()->getJobSpec() )
				 && ( NULL != PnlComGetJobScan()->getJobSpec()->pc_Send )
				 && ( 0 < PnlComGetJobScan()->getJobSpec()->pc_Send->c_ComSend.getSelectTotalAddressCount()) ) {	// 宛先選択総数が1以上.
					uint32_t ul_ShorteningID = 0;
					uint32_t ul_GroupID = 0;
					TYPE_SendMode e_SendMode = TYPE_SendModeNotSetting;
					char sc_ProgramKeyName[PCGD_ProgramKeyNameUTF8_73];
					memset( sc_ProgramKeyName, '\0', sizeof( sc_ProgramKeyName ) );
					if( NULL != pc_ProgramKey ) {
						ul_ShorteningID = pc_ProgramKey->getDbRecordNoOfShorteningAddress();			// 短縮宛先番号.
						ul_GroupID = pc_ProgramKey->getDbRecordNoOfGroupAddress();						// グループ宛先番号.
						e_SendMode = pc_ProgramKey->getSendMode();										// 宛先種別.
						pc_ProgramKey->getProgramKeyNameUTF8( sc_ProgramKeyName, TYPE_CharSetUTF8 );	// プログラムキー名称.
					}
					if( ( 0 != ul_ShorteningID )
					 || ( 0 != ul_GroupID )
					 || ( TYPE_SendModeNotSetting != e_SendMode ) ){
						// プログラム宛先ありの場合、プログラム宛先選択確認警告を表示する.
						m_ss_ConfirmationProgramNo = st_BadgeKeyInfo[_uc_index].ss_ProgramNo;
						gp_ConfirmProgramDestinationSelect_warn.setProgramName( sc_ProgramKeyName );
						PNLC_PopupListControl::newInstance()->addPopup( &gp_ConfirmProgramDestinationSelect_warn, this, NULL, 400, 240, 0, 0 , True );
						break;
// comment 2020/11/27 Sky東島 [IT6_2.1][OP_BTS-28868].
//					} else {
//						// プログラム宛先なしの場合、設定中の宛先をクリアする.
//						gp_ProgramBasic_scanfax.m_bl_clearAddress = True;
					}
				}
				procFax( 0 );
				if( ( NULL != nextGamen )
				    || ( ( NULL != pc_PopupListControl ) && ( True == pc_PopupListControl->isPopupLayer( &gp_IGCClearWait_popup ) ) ) )
				{
// comment 2020/09/25 Sky中野 [IT6_2.1][OP_BTS-27503].
//					TYPC_ProgramKey* pc_ProgramKey = PCGC_TopmenuProgramKeyData::getInstance()->getProgramKeyData( st_BadgeKeyInfo[_uc_index].ss_ProgramNo );
					if( NULL != pc_ProgramKey ) {
						gp_ProgramBasic_scanfax.setProgramKeyData( st_BadgeKeyInfo[_uc_index].ss_ProgramNo, pc_ProgramKey );
					}
				}
			}
			break;
		case PNLE_TopMenuShortCut_AppShortCut:	// OpenAPIアプリショートカット.
			switch( _e_TopMenuShortCut ) {
				case PNLE_TopMenuShortCut_AppShortCutOap:
// Comment 2019/12/31 [IT6_EagleBK] OP_BTS-18202 [HCLT-Santhiya]  Market Place key Display on ClassicStyle Screen. 
					procAPPOap( _uc_index );	// OpenAPIアプリショートカット:OAPアプリ.
					break;
// Comment 2019/06/08 [IT6_1.0MR2] AR1077690 [HCLT-Catherine]  New Enum type for Classic screen Market place icon.
				case PNLE_TopMenuShortCut_MarketPlaceApplication_Legacy:	// bizhub Market Place.
// Comment 2019/12/31 [IT6_EagleBK] OP_BTS-18202 [HCLT-Santhiya]  Market Place key Display on ClassicStyle Screen. 
					{
						PNLC_ActionDataLib* pc_ActionLib = PNLC_ActionDataLib::newInstance();
						if (NULL != pc_ActionLib ){
							pc_ActionLib->setMarketPlacelaunch( True );
						}
						procAPPOap( _uc_index );	// OpenAPIアプリショートカット:OAPアプリ.
// Comment 2020/07/17 HCL-Satheesh [IT6_2.0MR3SpecialROM][OP_BTS-25202] Restirct the Martket Place App while Market Place OFF in Admin and Utility navigated from Market place App.
						onlyPanel.setMarketPlaceAppStart( True );
						if (NULL != pc_ActionLib ){
							pc_ActionLib->setMarketPlacelaunch( False );
						}
						break;
					}
				case PNLE_TopMenuShortCut_AppShortCutHanyo:
					for( us_cntIws=0; us_cntIws < s_registerAppIws.size(); us_cntIws++ ) {
						s_registerAppIws[us_cntIws].getAppName( &c_IwsAppName );
						IwsAppName = c_IwsAppName.c_str();
						if( 0 == strcmp( st_BadgeKeyInfo[ _uc_index ].sc_FuncName, IwsAppName ) ) {
							s_registerAppIws[us_cntIws].getAppId( &ps_appId );
							s_registerAppIws[us_cntIws].getAppArgs( &s_args );
							pc_args = ( char* )( s_args.c_str() );
							LAPC_APP_ActiveAppManager APP_ActiveAppManager( LAPC_BFW_System::newInstance() );
							APP_ActiveAppManager.requestLaunchAppId( ps_appId, LAPE_APP_ActiveFlgON, pc_args, &p_processId );
							//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/03/24 Add Start.
							gp_CostCenterResetDummy.setAppType( PNLD_ScanOapAppSelectExt_SelectAppIws,  (uchar)ps_appId  );
							//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/03/24 Add End.
							break;
						}
					}
					procAPPHanyo( _uc_index );
					break;	// OpenAPIアプリショートカット:汎用アプリ.
				default :	break;
			}
			break;
		case PNLE_TopMenuShortCut_AppListShortCut:	// OpenAPIアプリリスト.
			procAPPsMenu( 0 );
			break;
		case PNLE_TopMenuShortCut_SysBox:			// システムボックス機能設定.
// Comment 2019/10/09 【HCLT-Krishna】【IT6_HeliosMLK】【制御構成変更_No.43】 SSDレス時の機密文書プリント対応
////Comment 2018/12/25 HCLT_Prashannaa AR 1070436 Secure print .
//#ifdef DEF_DenebMLKCus
#if defined  (DEF_DenebMLKCus) || defined ( DEF_HeliosMLKCus )
		{	
			Bool bl_SecurePrintWithoutHDD = False;
#endif
			switch( _e_TopMenuShortCut ) {
				case PNLE_TopMenuShortCut_SysBoxSecurity:
					//「Dipsw23 bit7」でかつユーザー認証中の場合.
					if( ( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DirectInputOffLdapOnNUM, 7 ) )
					    && ( True == PnlComGetUserAccountOn() ) ) {
						// システムボックス機能設定:セキュリティー文書(文書選択).
						_e_ShortCutID = PNLE_ShortCutID_BOX_DocSecretDataBeforePassPrint_Direct;
					} else {
						// システムボックス機能設定:セキュリティー文書(ID入力画面).
						_e_ShortCutID = PNLE_ShortCutID_BOX_DocSecretDataBeforePassPrint;
					}
// Comment 2019/10/09 【HCLT-Krishna】【IT6_HeliosMLK】【制御構成変更_No.43】 SSDレス時の機密文書プリント対応
////Comment 2018/12/25 HCLT_Prashannaa AR 1070436 Secure print .
//#ifdef DEF_DenebMLKCus
#if defined  (DEF_DenebMLKCus) || defined ( DEF_HeliosMLKCus )
					if((False == PnlComGetHddInfo()->getHddExist())	&& ( True == PnlComIsSecurityDoc() )){
						bl_SecurePrintWithoutHDD = True;
					}
#endif
					break;
				case PNLE_TopMenuShortCut_SysBoxForcedMemory:	_e_ShortCutID = PNLE_ShortCutID_BOX_DocumentPrintThumbnail;				break;	// システムボックス機能設定:強制メモリー受信.
				case PNLE_TopMenuShortCut_SysBoxAuthPrint:		_e_ShortCutID = PNLE_ShortCutID_BOX_TouchAndGoBoxPrint;					break;	// システムボックス機能設定:認証＆プリント.
				case PNLE_TopMenuShortCut_SysBoxBoard:			_e_ShortCutID = PNLE_ShortCutID_BOX_BulletinBoxUse;						break;	// システムボックス機能設定:掲示板.
				case PNLE_TopMenuShortCut_SysBoxPolling:		_e_ShortCutID = PNLE_ShortCutID_BOX_PrintBulletinPoll;					break;	// システムボックス機能設定:ポーリング送信.
				case PNLE_TopMenuShortCut_SysBoxFilingNumber:	_e_ShortCutID = PNLE_ShortCutID_BOX_DocFilingNumber;					break;	// システムボックス機能設定:ファイリングナンバー.
				case PNLE_TopMenuShortCut_SysBoxSendAgain:		_e_ShortCutID = PNLE_ShortCutID_BOX_DocReTransmitPrintList;				break;	// システムボックス機能設定:再送信.
				case PNLE_TopMenuShortCut_SysBoxEncryption:		_e_ShortCutID = PNLE_ShortCutID_BOX_DocumentSelectPrintListEncryptPDF;	break;	// システムボックス機能設定:パスワード暗号化PDF.
				case PNLE_TopMenuShortCut_SysBoxExternalMemory:	// 外部メモリー.
					_e_ShortCutID = PNLE_ShortCutID_BOX_USBReference;
					if( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Print )		// プリント印字禁止.
// comment 2021/10/28 Sky東島 [IT6_MinervaSSBK][OP_BTS-34376].
//					    && TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_ScanToUsb )	// ScanToUSB禁止.
					    && ( ( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_ScanToUsb ) )	// ScanToUSB禁止.
					      || ( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Scan ) ) )		// スキャン操作禁止.
					    && TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_UsbMemoryReading ) ) {	// USBメモリ読み込み禁止.
						// Public認証ログインでの機能制限時.
						if( True == PNLC_LimitationFunctionLoginUserCheckLib::newInstance()->checkUsingLoginSettting()
						    || True == PnlComIsNeedUserChangeFunction() ) {
							PNLS_AuthenticationLoginReceptKey st_ReceptKey;
							st_ReceptKey.uc_Option    = option;
							// ベンダー2と併用時はポップアップは閉じない.
							if( True == PnlComIsNeedUserChangeFunction() ) {	// 認証画面を表示する場合.
								st_ReceptKey.e_OptionType = PNLE_AuthenticationLoginReceptKeyOption_GoToPage;	// 画面/コンポーネント 遷移キー押下イベント.
// comment 2021/10/28 Sky東島 [IT6_MinervaSSBK][OP_BTS-34376].
//								PNLC_AuthenticationManager::newInstance()->authenticate( this , PNLE_LimitationFunctionType_NoCheck, &st_ReceptKey, PNLE_AuthenticatorPopupCloseType_NoClosePopup );	// ログイン画面表示.
								PNLC_AuthenticationManager::newInstance()->authenticate( this , PNLE_LimitationFunctionType_Usb, &st_ReceptKey, PNLE_AuthenticatorPopupCloseType_NoClosePopup );	// ログイン画面表示.
							} else {
								st_ReceptKey.e_OptionType = PNLE_AuthenticationLoginReceptKeyOption_ButtonOn;
								// ログイン画面表示.
								PNLC_AuthenticationManager::newInstance()->authenticate( this, PNLE_LimitationFunctionType_Box, &st_ReceptKey );
							}
						} else {
							//ありえない.
							LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
						}
						return;
					}
					break;
				case PNLE_TopMenuShortCut_SysBoxMobilePDA:	// 携帯電話/PDA.
					_e_ShortCutID = PNLE_ShortCutID_BOX_CellularPhoneMenu;
					if( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Bluetooth )		// 携帯電話/PDA使用不許可ユーザー.
					    || TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_BoxOperation ) ) {	// ボックス保存不許可ユーザー.
						// Public認証ログインでの機能制限時.
						if( True == PNLC_LimitationFunctionLoginUserCheckLib::newInstance()->checkUsingLoginSettting()
						    || True == PnlComIsNeedUserChangeFunction() ) {
							PNLS_AuthenticationLoginReceptKey st_ReceptKey;
							st_ReceptKey.uc_Option    = option;
							// ベンダー2と併用時はポップアップは閉じない.
							if( True == PnlComIsNeedUserChangeFunction() ) {	// 認証画面を表示する場合.
								st_ReceptKey.e_OptionType = PNLE_AuthenticationLoginReceptKeyOption_GoToPage;	// 画面/コンポーネント 遷移キー押下イベント.
								PNLC_AuthenticationManager::newInstance()->authenticate( this , PNLE_LimitationFunctionType_NoCheck, &st_ReceptKey, PNLE_AuthenticatorPopupCloseType_NoClosePopup );	// ログイン画面表示.
							} else {
								st_ReceptKey.e_OptionType = PNLE_AuthenticationLoginReceptKeyOption_ButtonOn;
								// ログイン画面表示.
								PNLC_AuthenticationManager::newInstance()->authenticate( this, PNLE_LimitationFunctionType_Box, &st_ReceptKey );
							}
						} else {
							//ありえない.
							LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
						}
						return;
					}
					break;
// comment 2016/12/01 SCD2-Sky浜野 [IT5_4.1][AirPrint1.7対応].
				case PNLE_TopMenuShortCut_SysBoxPinPrint:	// PINプリント.
					_e_ShortCutID = PNLE_ShortCutID_BOX_PinPrint;
					break;
				default :	break;
			}
			if( True == PNLC_HitachiICC_TopMenuCustomize::newInstance()->isHitachiICC_Mode() ) {
				_e_ShortCutID = PNLE_ShortCutID_BOX_DocSecretDataBeforePassPrint_HitachICC;
			}

// comment 2015.01.20 OPS1-SkyOPS1-Sky西原 [HeliosL][制御構成変更No.36] HDD非装着でUSB利用 procBoxの引数を外部メモリ遷移チェック用に変更.
//			procBox( 0 );	// アプリケーション切り替え.
// Comment 2019/10/09 【HCLT-Krishna】【IT6_HeliosMLK】【制御構成変更_No.43】 SSDレス時の機密文書プリント対応
////Comment 2018/12/25 HCLT_Prashannaa AR 1070436 Secure print .
//#ifdef DEF_DenebMLKCus
#if defined  (DEF_DenebMLKCus) || defined ( DEF_HeliosMLKCus )
			if( True == bl_SecurePrintWithoutHDD ){
				procBox( PNLD_SecurePrintWithoutHDD );
			}
			else {
#endif
				procBox( PNLD_CheckGotoUsbWithoutHDD );	// アプリケーション切り替え.
// Comment 2019/10/09 【HCLT-Krishna】【IT6_HeliosMLK】【制御構成変更_No.43】 SSDレス時の機密文書プリント対応
////Comment 2018/12/25 HCLT_Prashannaa AR 1070436 Secure print .
//#ifdef DEF_DenebMLKCus
#if defined  (DEF_DenebMLKCus) || defined ( DEF_HeliosMLKCus )
			}
#endif
			if( NULL != nextGamen ) {
				if( NULL != pc_ChangeGamenManager ) {
					pc_ChangeGamenManager->setShortCut( _e_ShortCutID, PNLE_ShortCutKindID_Box );
					// トップメニューから直接システムボックスへ遷移する際は、セキュリティボックスから離脱したと判断する.
					PNLC_TabletSecurityControlLib* pc_TabletSecurityControl = PNLC_TabletSecurityControlLib::newInstance();
					if( NULL != pc_TabletSecurityControl ) {
						pc_TabletSecurityControl->securityAreaExited();
					}
				}
			}
//Comment 2019/10/09 【HCLT-Krishna】【IT6_HeliosMLK】【制御構成変更_No.43】 SSDレス時の機密文書プリント対応
////Comment 2018/12/25 HCLT_Prashannaa AR 1070436 Secure print .
//#ifdef DEF_DenebMLKCus
#if defined  (DEF_DenebMLKCus) || defined ( DEF_HeliosMLKCus )
		}
#endif
			break;
// comment 2018/04/12 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
		case PNLE_TopMenuShortCut_UIMode:
			procUIMode( 0 );		
			break;
		case PNLE_TopMenuShortCut_Utility:
			switch( _e_TopMenuShortCut ) {
				case PNLE_TopMenuShortCut_Utility:	// 設定メニュー.
					procUtility( 0 );
					break;
				case PNLE_TopMenuShortCut_QRCode:
					procUtility( BTNID_GoToUtilityToQRCode );
					if( NULL != nextGamen ) {
						uchar uc_Message[ TYPD_QRPairing_JsonDataLength + 1 ];
						memset( uc_Message, '\0', sizeof( uc_Message ) );
						APIC_QRPairingSetting* cp_QRPairingSetting  = PnlComGetQRPairingSetting();
						if( NULL != cp_QRPairingSetting ) {
							cp_QRPairingSetting->getJsonData( uc_Message );
						}

						Bool bl_result = PCGC_QRCodeControl::newInstance()->getQRCodeImage( ( char* )uc_Message , &Gra_UtilityQRCode );
						if( False == bl_result ) {
							// 報知音設定
							setBuzzerType( PCGE_BuzzerType_InputNg );
							nextGamen = NULL;
							return;
						}
					}
					break;
// comment 2020/08/17 [特ROM対応][IT5-144(機種マージ)] 転送ファックス設定をパスワード無しで使用可能にする要望対応.
				case PNLE_TopMenuShortCut_ForwardFax:
					// 簡易セキュリティー設定と同様に管理者画面への遷移条件を満たしていなければ拒否音を鳴らす.
					if( True == APIC_NetworkDevice::newInstance()->isNetworkInitialing() ){
						setBuzzerType( PCGE_BuzzerType_InputNg );
						break;
					}
					if ( !( (True == onlyPanel.bl_BrowserStartUpFlag) && (False == onlyPanel.b_BrowserRebootWait) && (False == PNLC_BrowserChangeTabControl::getInstance()->isBrowserProcessRestart()) ) ) {
						setBuzzerType( PCGE_BuzzerType_InputNg );
						break;
					}
					if( False == PNLC_BrowserChangeTabControl::getInstance()->isEnableLaunchAdminTab() ){
						setBuzzerType( PCGE_BuzzerType_InputNg );
						break;
					}
					// 転送ファックス設定への画面遷移要求を行う.
					if( ( PNLE_BrowserChangeTabMode_Admin == PNLC_BrowserChangeTabControl::getInstance()->getBrowserChangeTabMode() )
					&&  ( True == PNLC_WebUIControl_v1::getInstance()->getEnableFunction() ) ) {
						PnlComGetSystemStatus()->setUserChoiceChanging( True );
						PnlComGetSystemStatus()->setShortCutForwardFaxSetting( True );
						PNLC_WebUIControl_v1::getInstance()->requestChangeScreen( PNLE_WebUIv1RequestMessageFaxTransfer );
					} else {
						if( True == PNLC_BrowserChangeTabControl::getInstance()->launchAdminTab() ){
							PnlComGetSystemStatus()->setUserChoiceChanging( True );
							PnlComGetSystemStatus()->setShortCutForwardFaxSetting( True );
							PNLC_WebUIControl_v1::getInstance()->reserveChangeScreen( PNLE_WebUIv1RequestMessageFaxTransfer );
						}
					}
					break;
				default:
					procUtility( 0 );
					break;
			}
			break;
// comment 2015/03/04 OPS1-Sky榎本 [MinervaSBk][UV-01457]トップメニューへアドレス帳キー割付.
		case PNLE_TopMenuShortCut_AddressBook:	// アドレス帳.
		 	procAddressBook( 0 );
			break;

		case PNLE_TopMenuShortCut_SoundSetting:
			procAccessibility( option );
			if( NULL != nextGamen ) {
				onlyPanel.setReturnGamenTopMenu( True );
// comment 2015/02/12 OPS1-Sky善本 [ZeusS][CQ510080].
//			 	nextGamen = &gp_AccessibilityMusicTop;	// 音設定.
//Comment 2019/08/26 [HCL_Rajesh] [EagleZPlus_BKZPlus][OP_BTS-7128] We have uncommented the <DEF_SparrowCus> compiler switch to remove the <Sound Type / Volume Setting> key.
//Comment 2018/11/30 [HCL_Krishna] [IT6_Sparrow][AR1068260]  Sound Type / Volume Setting screen transition for Sparrow Models.			
////Comment 2018/11/122 [HCL_Hepzibah] [IT6_Sparrow][AR1068369] Removing condition of Sound Type / Volume Setting screen transition for Sparrow Models.			
//#ifndef DEF_SparrowCus
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )
				APIC_Music* pc_Music = APIC_Music::newInstance();
				Bool b_Extend = pc_Music->isMusicSettingExtend();		// 音設定拡張可否.
				if( True == b_Extend ){									// 拡張有効. 
				 	nextGamen = &gp_AccessibilityMusicTop;				// 音設定画面へ.
				}else{
					nextGamen = &gp_AccessibilitySoundSetKeyReceiveDenial;	//音設定画面へ.
				}
//Comment 2019/08/26 [HCL_Rajesh] [EagleZPlus_BKZPlus][OP_BTS-7128] We have uncommented the <DEF_SparrowCus> compiler switch to remove the <Sound Type / Volume Setting> key.
//Comment 2018/11/30 [HCL_Krishna] [IT6_Sparrow][AR1068260]  Sound Type / Volume Setting screen transition for Sparrow Models.			
//#else
//					nextGamen = &gp_AccessibilitySoundSetKeyReceiveDenial;	//音設定画面へ.
//#endif
#else
//Comment 2020/03/05 [HCL_Selvaraj] [IT6_EagleZ+][OP_BTS-21148]  Sound play issue after making OFF for SparrowAIO Model.			
#if defined(DEF_SparrowAIOSFPCus)
				TYPE_MachineType e_MachineType = PnlComGetSystemInfo()->getMachineType();
				if(  TYPE_MachineTypeSparrow2AIO == e_MachineType){
					APIC_Music* pc_Music = APIC_Music::newInstance();
					Bool b_Extend = pc_Music->isMusicSettingExtend();		// 音設定拡張可否.
					if( True == b_Extend ){									// 拡張有効. 
				 		nextGamen = &gp_AccessibilityMusicTop;				// 音設定画面へ.
					}else{
						nextGamen = &gp_AccessibilitySoundSetKeyReceiveDenial;	//音設定画面へ.
					}
				} else {
					nextGamen = &gp_AccessibilitySoundSetKeyReceiveDenial;	//音設定画面へ.
				}
#else
					nextGamen = &gp_AccessibilitySoundSetKeyReceiveDenial;	//音設定画面へ.
#endif
#endif
			}
			break;
		case PNLE_TopMenuShortCut_GrpAppShortCut:					// グループアプリケーション グループ１.
			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
				procAPPGrp( _e_TopMenuShortCut );
			}
			break;
		case PNLE_TopMenuShortCut_PrintMusic:   procPrintMusic( PCGD_PRINTMUSIC );  break;  //  ぷりんと楽譜.
		case PNLE_TopMenuShortCut_SimpleSecuritySetup:			// 簡易セキュリティー設定.
//comment 2018/11/12 HCLT_Basker AR_1064695 Quick security related changes.
			if( True == APIC_NetworkDevice::newInstance()->isNetworkInitialing() ){
				setBuzzerType( PCGE_BuzzerType_InputNg );
				break;
   			}
//comment 2018/12/04 HCLT-Raja [AR1068212] The browser start-up & reboot flag has to verify after SimpleSecuritySetup key press.
// comment 2019/04/15 SCD2-Sky小畑 [EagleH] メモリフル時のフェールセーフ対応.
//			if ( !( (onlyPanel.bl_BrowserStartUpFlag == True) && (onlyPanel.b_BrowserRebootWait == False) ) ) {
			if ( !( (onlyPanel.bl_BrowserStartUpFlag == True) && (onlyPanel.b_BrowserRebootWait == False) && (False == PNLC_BrowserChangeTabControl::getInstance()->isBrowserProcessRestart()) ) ) {
				setBuzzerType( PCGE_BuzzerType_InputNg );
				break;
			}
// comment 2019/05/15 Chikuan [EagleH][OP_BTS-1792] V1遷移パフォーマンス改善.
			if( False == PNLC_BrowserChangeTabControl::getInstance()->isEnableLaunchAdminTab() ){
				setBuzzerType( PCGE_BuzzerType_InputNg );
				break;
			}
// comment 2015/04/02 MSE田中優介 [MinervaSBK][RQ-278] LDAPサーバーへのカード情報登録機能対応.
			// LDAPサーバーへのカード情報登録機能の終了処理.
			endAutoCardRegist();
			// セキュリティー警告からの遷移元情報を設定する.
			PNLC_TransitionInfoCom::setNextGamenPasswordEntry( PNLE_AdminPasswordMoveGamen_SimpleSecuritySetting );
			// 管理者パスワードの戻り画面を、基本画面に設定する.
			PNLC_TransitionInfoCom::setAdminPassGamenToDisplay( True );
			nextGamen = &gp_AdminPassword;
			break;
		case PNLE_TopMenuShortCut_Eco:					// Eco関連機能.
			switch( _e_TopMenuShortCut ) {
				case PNLE_TopMenuShortCut_EcoSetting:	// Eco設定.
					// Eco設定画面をポップアップ.
					if( NULL != pc_PopupListControl ) {
						pc_PopupListControl->addPopup( &gp_EcoSetting, this, NULL );
					}
					break;
				case PNLE_TopMenuShortCut_EcoCopy:		// Ecoコピー.
					procEcoCopy();
					break;
				case PNLE_TopMenuShortCut_EcoInfo:		// Eco指標.
// comment 2015/04/02 MSE田中優介 [MinervaSBK][RQ-278] LDAPサーバーへのカード情報登録機能対応.
					// LDAPサーバーへのカード情報登録機能の終了処理.
					endAutoCardRegist();

					// Eco指標画面に遷移.
// comment 2017/1/23 【HCL-Ashok】 【AR1012170】 Launching default screen as Graph instead of List screen.
				// nextGamen = &gp_EcoInfoEcoList_act;	// G072009300 Eco指標(12ヶ月分リスト).
				nextGamen = &gp_EcoInfoEcoGraph_act;	// G072009200 Eco指標(12ヶ月分グラフ).
					break;
				default:
					LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
					break;
			}
			break;
		case PNLE_TopMenuShortCut_GadgetSetting:		// ウィジェット設定.
// comment 2017/02/22 SCD2-Sky浜野 [IT5_3.4][1015221].
			// HDD(SSD)準備未完了であれば、受付禁止.
			if( TYPE_HddStatusFormated != PnlComGetHddInfo()->getHddStatus() ) {
				setBuzzerType( PCGE_BuzzerType_InputNg );
				return;
			}
			// ウィジェット設定画面をポップアップ.
			if( NULL != pc_PopupListControl ) {
				pc_PopupListControl->addPopup( &gp_GadgetSetting_act, this, NULL, 0, 0, 0, 0, True );
			}
			break;
		case PNLE_TopMenuShortCut_ControlMenu:		// コントローラー設定.
			procControlMenu( PCGD_CONTROLMENU );
			break;
// comment 2015/08/12 OPS1 Sky大濵 [ZeusS_ZX0][RQ-383]表示対象に”MarketPlace download”キーを新規追加する.
		case PNLE_TopMenuShortCut_MarketPlace:		// MarketPlace.
// comment 2016/03/03 OPS1-Sky佐藤 [ZeusS_ZX0][CQ525679] MarketPlaceダウンローダー不具合対応.
			if ( PNLE_IWS_Status_Bootup != PnlComGetIwsStatus() ) {
				setBuzzerType( PCGE_BuzzerType_InputNg );
				return;
			}
			if( NULL != pc_PopupListControl ) {		// MarketPlace download画面をポップアップ.
				short ss_XPos, ss_YPos, ss_XSize, ss_YSize = 0;
// comment 2018/04/04 SCD2 K.Nishiki [Eagle][制御構成変更No.111]ホーム画面認証画面の新UI化対応.
				uchar uc_IconIndex = option % m_uc_TopMenuListKeyDisp;
				Gra_HomeTopMenuBadgeIcon[uc_IconIndex].getPosition( ss_XPos, ss_YPos );
				Gra_HomeTopMenuBadgeIcon[uc_IconIndex].getSize( ss_XSize, ss_YSize );
				if( 0x00 == getSwDipSwitch( DEFSOFTWARESW172 ) ) {				// ダウンロード開始確認画面.
					pc_PopupListControl->addPopup( &gp_MarketPlaceDownloadStartConfirm_act, this, NULL, ss_XPos, ss_YPos, ss_XSize, ss_YSize );
				} else if( 0x02 == getSwDipSwitch( DEFSOFTWARESW172 ) ) {		// 接続先URL、Port入力用画面.
					pc_PopupListControl->addPopup( &gp_MarketPlaceURLInput_act, this, NULL, ss_XPos, ss_YPos, ss_XSize, ss_YSize );
				} else {														// 非表示またはグレーアウトされるためありえない.
					LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
				}
			}
			break;
// comment 2018/04/18 【IT6_Eagle】【HCLT_Pavani】【RQ-675】【WP 3465】 Download and install MarketPlace on all MFPs according to the performance.
		case PNLE_TopMenuShortCut_MarketPlaceSetup:		// Market Place Setup Key ShortCut type.
// comment 2018/10/12 【IT6_Eagle】【HCLT_Pavani】【AR1063938】 Condition check added for Market Place Application Uninstalled case.
		{
			PNLC_ActionDataLib* pc_ActionLib = PNLC_ActionDataLib::newInstance();
			short ss_InstalledStatus = 0;
			if( NULL != pc_ActionLib ) {
				ss_InstalledStatus = pc_ActionLib->getMarketPlaceDownloadState();
				if( NULL != pc_PopupListControl ) {	
					short ss_XPos, ss_YPos, ss_XSize, ss_YSize = 0;
					uchar uc_IconIndex = option % PNLD_TopMenuKeyDisp;
					Gra_HomeTopMenuBadgeIcon[uc_IconIndex].getPosition( ss_XPos, ss_YPos );
					Gra_HomeTopMenuBadgeIcon[uc_IconIndex].getSize( ss_XSize, ss_YSize );
// comment 2018/09/12 【IT6_Eagle】【HCLT_Pavani】【AR1053803】Support for Market Place Application Failure case.
//				pc_PopupListControl->addPopup( &gp_MarketPlaceSetup_PasswordInput_act, this, NULL, ss_XPos, ss_YPos, ss_XSize, ss_YSize );  // Password Input Screen.
// comment 2018/10/12 【IT6_Eagle】【HCLT_Pavani】【AR1063938】 Condition check added for Market Place Application Uninstalled case.
//				if ( False == PnlComGetIWSSetting()->getEnable() ) {
					if ( ( False == PnlComGetIWSSetting()->getEnable() ) || 
						( PCGC_MarketPlaceDownloadStateComplete == ss_InstalledStatus ) ) {
						pc_PopupListControl->addPopup( &gp_MarketPlaceSetup_PasswordInput_act, this, NULL, ss_XPos, ss_YPos, ss_XSize, ss_YSize );  // Password Input Screen.
					} else {
							pc_PopupListControl->addPopup( &gp_MarketPlaceDownloadFailure_act, this, NULL, ss_XPos, ss_YPos, ss_XSize, ss_YSize );
					}
				} else {
					LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
				}
			}
		}
			break;
// Comment 2018/10/08 [HCL-Pavani] [IT6_Eagle] [AR1060058] MarketPlace Application Not Applicable implementation commented.
//// comment 2018/04/18 【IT6_Eagle】【HCLT_Pavani】【RQ-675】【WP 3465】 Download and install MarketPlace on all MFPs according to the performance.
//		case PNLE_TopMenuShortCut_MarketPlaceApplication:	// Market Place Application Key Short.
//				// Browser launcher
//				bootMarketPlace();
//			break;
		default :	break;
	}
	
// comment 2023/04/17 Sky中野 [IT6_2.3][OP_BTS-42686].
	if( ( PNLE_TopMenuShortCut_UIModeLight != _e_TopMenuShortCut )
	 && ( PNLE_TopMenuShortCut_MarketPlaceSetup != _e_TopMenuShortCut )
	 && ( PNLE_TopMenuShortCut_Utility != ( _e_TopMenuShortCut & 0xFF00 ) )
	 && ( PNLE_TopMenuShortCut_SoundSetting != _e_TopMenuShortCut )
	 && ( PNLE_TopMenuShortCut_GadgetSetting != _e_TopMenuShortCut )
	 && ( PNLE_TopMenuShortCut_SimpleSecuritySetup != _e_TopMenuShortCut )
	 && ( PNLE_TopMenuShortCut_AddressBook != _e_TopMenuShortCut ) ) {
		if( PCGE_BuzzerType_InputNg != getBuzzerType() ){
			// クラシックスタイルでジョブ設定を初期化する対象としている画面へ遷移した場合.
			// WebUIジョブ設定の初期化要求を通知する.
			pc_WebUIControl->initWebUIJobSettings();
		}
	}
}
// comment 2018/04/18 【IT6_Eagle】【HCLT_Pavani】【RQ-675】【WP 3465】 Download and install MarketPlace on all MFPs according to the performance.
// Comment 2018/10/08 [HCL-Pavani] [IT6_Eagle] [AR1060058] MarketPlace Application Not Applicable implementation commented.
//void PCGC_HomeTopMenu::bootMarketPlace()
//{
//	PCGC_ScanOapCom::setApplicationNotifyDisconnect( False );
//
//	APIC_IWSAppInfo* pc_IWSAppInfo = APIC_IWSAppInfo::newInstance(); // Failure in acquisition of pointer to APIC_IWSAppInfo information.
//	if( NULL == pc_IWSAppInfo ) {
//		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
//		return;
//	}
//	ushort us_AppNo = 0;
//	Bool bl_FindApp = False;
//	for( int nIndex = 1; nIndex <= TYPD_IwsMaxApplication; nIndex++ ) {
//		TYPS_IwsAppInfo t_AppInfo;
//		TYPE_IWSMarketPlaceAppType e_IWSMarketPlaceAppType = TYPE_IWSApp_Others;
//		pc_IWSAppInfo->getApplication( nIndex, t_AppInfo, &e_IWSMarketPlaceAppType );
//		if( TYPE_IWSApp_MarketPlace == e_IWSMarketPlaceAppType ) {
//			us_AppNo = t_AppInfo.s_BrowserUiSetting.ss_RegistNo;
//			bl_FindApp = True;
//			break;
//		}
//	}

//	if( False == bl_FindApp ) {
//		return;
//	}
	
//	TYPE_ExtType s_ExtType = TYPE_ExtTypeScanD;		// APIC_ExtSetting インスタンス取得.
//	APIC_ExtSetting* pc_ExtSetting = APIC_ExtSetting::newInstance( us_AppNo );
//	if( NULL != pc_ExtSetting ) {
//		s_ExtType = pc_ExtSetting->getApplication().s_Version.e_Type;
//	}

	// カレントアプリNo情報設定.
//	ScanOapExtSet.setCurrentAppNo( us_AppNo );
//	ScanOapExtSet.getCurrentAplInfo();
//	ScanOapExtSet.setCheckMode( False );

	// 選択アプリタイプチェック.
//	switch( s_ExtType ) {
		// BrowserLauncher.
//		case TYPE_ExtTypeBrowserLauncher:
			// IWS機能起動準備中の場合.
//			if( PNLE_IWS_Status_Ready == PnlComGetIwsStatus() ) {
//				return;
//			}
			// ブラウザ起動モードをブラウザーランチャーとして再起動する.
//			PnlComBrowserLauncherModeReboot();
			// ブラウザーアプリへ切り替え.
//			ScanOapExtSet.m_bTopmenuModeBackup = onlyPanel.isTopmenuMode();
//			PMessageArea::procBrowser( 0 );
//			if( &gp_AppChangeBrowser == nextGamen ) {
//				ScanOapExtSet.setExtAppType( TYPE_ExtTypeBrowserLauncher );
//			}
//			break;
//		default :	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
//	}
//}	
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： Ecoコピー押下時の処理.
//================================================================================================
void PCGC_HomeTopMenu::procEcoCopy() {
	// コピーへの遷移処理.
	procCopy( 0 );
	if( NULL != nextGamen ) {
		APIC_Job* pc_JobCopy = PnlComGetJobCopy();
		if( NULL == pc_JobCopy ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return;
		};
		TYPC_JobSpec* pc_JobSpec = pc_JobCopy->getJobSpec();
		if( NULL == pc_JobSpec ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return;
		}
		// ユーザーアカウント情報取得.
		TYPC_UserAcount c_UserAcount;
		memcpy(&c_UserAcount, &(pc_JobCopy->getJobSpec()->pc_Common->c_UserAcount), sizeof(TYPC_UserAcount));

		// 工場出荷値に一旦セット.
		pc_JobSpec->setCommonJobSpec( *( c_JobSpecCopyDefult.pc_Common ) );
		pc_JobSpec->setScanJobSpec( *( c_JobSpecCopyDefult.pc_Scan ) );
		pc_JobSpec->setPrintJobSpec( *( c_JobSpecCopyDefult.pc_Print ) );
		pc_JobSpec->setFaxJobSpec( *( c_JobSpecCopyDefult.pc_Fax ) );

		// 退避した情報を復帰.
		memcpy(&(pc_JobCopy->getJobSpec()->pc_Common->c_UserAcount), &c_UserAcount, sizeof(TYPC_UserAcount));

#ifndef DEF_MonoCus
		// カラー禁止ユーザーの場合か、Ecoコピー設定でカラーモードにブラックが指定されている場合はブラック.
		if( ( TYPE_FuncLimit_OnlyBlack == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Copy ) )
		    || ( TYPE_ColorBlack == PnlComGetOperationSetting()->getEcoCopyColorType() ) ) {
			pc_JobSpec->pc_Scan->setAcsOn( False );
			pc_JobSpec->pc_Common->setColorMode( TYPE_ColorModeBlack );
		}
#endif	// DEF_MonoCus.

		TYPE_EcoCopyCombineType e_EcoCopyCombineType = PnlComGetOperationSetting()->getEcoCopyCombineType();
		switch( e_EcoCopyCombineType ) {
			case TYPE_Combine2in1:		// 2in1.
				pc_JobSpec->pc_Print->c_Nin1.setNin1( TYPE_Nin1_2in1 );
				break;
			case TYPE_Combine4in1:		// 4in1.
				pc_JobSpec->pc_Print->c_Nin1.setNin1( TYPE_Nin1_4in1 );
				break;
			case TYPE_Combine8in1:		// 8in1.
				pc_JobSpec->pc_Print->c_Nin1.setNin1( TYPE_Nin1_8in1 );
				break;
			case TYPE_CombineNothing:	// しない(しないの場合は何も設定しない).
				break;
			default:
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
				break;
		}
#ifdef DEF_A4_Cus		// [VenusMLK][CQ523174] 2015/01/27 OPS2 Sky高田(庸).
		// 集約設定ありの場合は、倍率設定もお勧め倍率に従う.
		if ( TYPE_CombineNothing != e_EcoCopyCombineType ) {
			if( PnlComGetOperationSetting()->getEnableNin1Mag() ) {
				PnlComSetNin1DefaultMag( pc_JobSpec->pc_Print, pc_JobSpec->pc_Common );
			}
		}
#endif	// DEF_A4_Cus	// [VenusMLK][CQ523174] 2015/01/27 OPS2 Sky高田(庸).

		TYPE_EcoCopySideType e_EcoCopySideType = PnlComGetOperationSetting()->getEcoCopySideType();
		switch( e_EcoCopySideType ) {
			case TYPE_SideSingleToDup:			// 片面 -> 両面.
				pc_JobSpec->pc_Print->setDuplexMode( TYPE_DuplexModeOn );
				break;
			case TYPE_SideDupToSingle:			// 両面 -> 片面.
				pc_JobSpec->pc_Scan->setDuplex( True );
				break;
			case TYPE_SideDupToDup:				// 両面 -> 両面.
				pc_JobSpec->pc_Scan->setDuplex( True );
				pc_JobSpec->pc_Print->setDuplexMode( TYPE_DuplexModeOn );
			case TYPE_SideSingleToSingle:		// 片面 -> 片面(片面 -> 片面の場合は何も設定しない).
				break;
			default:
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
				break;
		}
		// 仕上りプログラムキーの状態設定OFF.
		onlyPanel.setFinishingProgram( False );

#ifdef DEF_A4_Cus		// Comment VenusMLK -> 2016/07/12 Sky高田(庸) [CQ532415]★A3共通問題★.
		selectAutoPaperMag();           // APS/AMS処理関数.
#endif	// DEF_A4_Cus	// Comment VenusMLK <- 2016/07/12 Sky高田(庸) [CQ532415]★A3共通問題★.
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： Eco関連機能のショートカットキーへの情報設定.
//================================================================================================
void PCGC_HomeTopMenu::setEcoFuncInfo( uchar uc_Index, TYPE_TopMenuShortCutEco e_TopMenuShortCutEco )
{
	switch( e_TopMenuShortCutEco ) {
		case TYPE_TopMenuShortCutEcoSetting:	// Eco設定.
			// ショートカット構造体にEco設定を割付.
			st_BadgeKeyInfo[uc_Index].e_ShortCutKind = PNLE_TopMenuShortCut_EcoSetting;
			st_BadgeKeyInfo[uc_Index].tid_FuncName = s_pKEYActivityEcoSetting_W100H042E;
// Comment 2016/10/07 [HCLT_Sri charan] [1005414] 【IT5_3.3】 Addition of part for 10.1 inch screen.
#ifdef DEF_RES_WSVGA
			st_BadgeKeyInfo[uc_Index].puc_FuncIcon = ( uchar* )o75_002_000_X81_Y100;
#else // #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[uc_Index].puc_FuncIcon = ( uchar* )o70_002_000_X64_Y80;
#endif // #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[uc_Index].uc_IconPaletteType = PNLD_PaletteEco;
// Comment 2016/10/07 [HCLT_Sri charan] [1005414] 【IT5_3.3】 Addition of part for 10.1 inch screen.
#ifdef DEF_RES_WSVGA
			st_BadgeKeyInfo[uc_Index].puc_IconNotActivate = ( uchar* )o75_002_000_X81_Y100;
#else // #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[uc_Index].puc_IconNotActivate = ( uchar* )o70_002_000_X64_Y80;
#endif // #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[uc_Index].uc_IconPaletteNotActivate = PNLD_PaletteEco;
// 2017/06/20 SCD2-Sky Shindo [SecV2-x] Eco設定キー非表示条件追加.
			if( False != PnlComGetSystemSetting()->getSecurityLevelUpOn()	// セキュリティ強化モードが有効.
			 && NULL == PnlComGetUser()										// 認証中でない.
			) {
				st_BadgeKeyInfo[uc_Index].bl_isShow = False;
			} 
			break;
		case TYPE_TopMenuShortCutEcoCopy:		// Ecoコピー.
			// ショートカット構造体にEcoコピーを割付.
			st_BadgeKeyInfo[uc_Index].e_ShortCutKind = PNLE_TopMenuShortCut_EcoCopy;
			st_BadgeKeyInfo[uc_Index].tid_FuncName = s_pKEYActivityEcoCopy_W100H042E;
// Comment 2016/10/07 [HCLT_Sri charan] [1005414] 【IT5_3.3】 Addition of part for 10.1 inch screen.
#ifdef DEF_RES_WSVGA
			st_BadgeKeyInfo[uc_Index].puc_FuncIcon = ( uchar* )o75_001_000_X81_Y100;
#else // #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[uc_Index].puc_FuncIcon = ( uchar* )o70_001_000_X64_Y80;
#endif // #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[uc_Index].uc_IconPaletteType = PNLD_PaletteEco;
// Comment 2016/10/07 [HCLT_Sri charan] [1005414] 【IT5_3.3】 Addition of part for 10.1 inch screen.
#ifdef DEF_RES_WSVGA
			st_BadgeKeyInfo[uc_Index].puc_IconNotActivate = ( uchar* )i76_001_000_d_X81_Y100;
#else // #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[uc_Index].puc_IconNotActivate = ( uchar* )i71_001_000_d_X64_Y80;
#endif // #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[uc_Index].uc_IconPaletteNotActivate = PNLD_PaletteCommon;
			st_BadgeKeyInfo[uc_Index].bl_isShow = isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_Copy );
			break;
		case TYPE_TopMenuShortCutEcoInfo:		// Eco指標.
			// ショートカット構造体にEco指標を割付.
			st_BadgeKeyInfo[uc_Index].e_ShortCutKind = PNLE_TopMenuShortCut_EcoInfo;
			st_BadgeKeyInfo[uc_Index].tid_FuncName = s_pKEYActivityEcoInfo_W100H042E;
// Comment 2016/10/07 [HCLT_Sri charan] [1005414] 【IT5_3.3】 Addition of part for 10.1 inch screen.
#ifdef DEF_RES_WSVGA
			st_BadgeKeyInfo[uc_Index].puc_FuncIcon = ( uchar* )o75_003_000_X81_Y100;
#else // #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[uc_Index].puc_FuncIcon = ( uchar* )o70_003_000_X64_Y80;
#endif // #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[uc_Index].uc_IconPaletteType = PNLD_PaletteEco;
// Comment 2016/10/07 [HCLT_Sri charan] [1005414] 【IT5_3.3】 Addition of part for 10.1 inch screen.
#ifdef DEF_RES_WSVGA
			st_BadgeKeyInfo[uc_Index].puc_IconNotActivate = ( uchar* )o75_003_000_X81_Y100;
#else // #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[uc_Index].puc_IconNotActivate = ( uchar* )o70_003_000_X64_Y80;
#endif // #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[uc_Index].uc_IconPaletteNotActivate = PNLD_PaletteEco;
			break;
		default:
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			break;
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： Eco関連機能がショートカットに設定されているか.
//================================================================================================
void PCGC_HomeTopMenu::checkExistEcoFuncShortcut()
{
//	TYPS_ExtShortCutList st_BadgeKeyList;
//	m_bl_IsExistEcoFuncShortcut = False;
//	memset( &st_BadgeKeyList, 0, sizeof(st_BadgeKeyList) );
//	uchar uc_Index;

	uchar uc_Index;
	m_bl_IsExistEcoFuncShortcut = False;
	if( PCGE_HuronMyPanel == PnlComGetMyPanelType()
	    ||  PCGE_OapMyPanel == PnlComGetMyPanelType() ) {
		if( NULL != PnlComGetUser()
		    &&  TYPE_UserTypeUser == PnlComGetUser()->getUserType() ) {			// 一般ユーザーでログイン時のみ.
			for( uc_Index = 0; uc_Index < ( TYPD_ExtShortcutMaxTopMenu + TYPD_ExtShortcutAlwaysShowTopMenu ); uc_Index++ ) {
				if( TYPE_TopMenuShortCutTypeEco == PnlComGetMyPanelSetting()->getTopMenuShortCutKeyFunction( uc_Index ).e_TopMenuShortCutType ) {
					m_bl_IsExistEcoFuncShortcut = True;
					break;
				}
			}
		}
	} else {
		TYPS_ExtShortCutList st_BadgeKeyList;
		memset( &st_BadgeKeyList, 0, sizeof( st_BadgeKeyList ) );
//Comment 2017/09/04 [HCL-Suribabu][IT5_4.2][RQ-686][WP763][詳細制御仕様書_管理No.33] main menu default settings should be support for Network2.
//	PnlComGetStandardPanelSetting()->getShortCutListSetting( &st_BadgeKeyList );
	PnlComGetStandardPanelSetting()->getShortCutListSetting( &st_BadgeKeyList,  PNLC_NetworkLan::getInstance()->getLine() );
		for( uc_Index = 0; uc_Index < ( TYPD_ExtShortcutMaxTopMenu + TYPD_ExtShortcutAlwaysShowTopMenu ); uc_Index++ ) {
			if( TYPD_ExtShortcutMaxTopMenu <= uc_Index ) {
				if( TYPE_TopMenuShortCutTypeEco == st_BadgeKeyList.s_ShortCutKeyAlwaysShow[uc_Index-TYPD_ExtShortcutMaxTopMenu].e_ShortCutType ) {
					m_bl_IsExistEcoFuncShortcut = True;
					break;
				}
			} else {
				if( TYPE_TopMenuShortCutTypeEco == st_BadgeKeyList.s_ShortCutKey[uc_Index].e_ShortCutType ) {
					m_bl_IsExistEcoFuncShortcut = True;
					break;
				}
			}
		}
	}
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： IWS設定Off時にMarketPlaceがショートカットに設定されているかチェック.
// 履歴	：comment 2018/12/6 Chikuan [AR1067363][EagleL] 関数追加.
//================================================================================================
Bool PCGC_HomeTopMenu::checkExistMarketPlaceWithIWSOff()
{
	uchar uc_Index;
	Bool bl_Result = False;

	if( True == PnlComGetIWSSetting()->getEnable() ){
		return bl_Result;
	}
	if( 1 > ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeBrowserLauncher ) ){
		return bl_Result;
	}
	if( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() ){
		TYPS_ExtShortCutListLightUI st_BadgeKeyListLightUI;
		memset( &st_BadgeKeyListLightUI, 0, sizeof( st_BadgeKeyListLightUI ) );
		PnlComGetStandardPanelSetting()->getShortCutListSettingLightUI( &st_BadgeKeyListLightUI );
		for( uc_Index = 0; uc_Index < st_BadgeKeyListLightUI.ss_ArraySize; uc_Index++ ){
			if( TYPE_TopMenuShortCutTypeMarketPlaceSetup == st_BadgeKeyListLightUI.s_ShortCutKey[uc_Index].e_ShortCutType ){
				bl_Result = True;
				break;
			}
		}
	}else{
		if( ( PCGE_HuronMyPanel == PnlComGetMyPanelType() ) || ( PCGE_OapMyPanel == PnlComGetMyPanelType() ) ){
			if( ( NULL != PnlComGetUser() ) && ( TYPE_UserTypeUser == PnlComGetUser()->getUserType() ) ){			// 一般ユーザーでログイン時のみ.
				for( uc_Index = 0; uc_Index < ( TYPD_ExtShortcutMaxTopMenu + TYPD_ExtShortcutAlwaysShowTopMenu ); uc_Index++ ){
					if( TYPE_TopMenuShortCutTypeMarketPlaceSetup == PnlComGetMyPanelSetting()->getTopMenuShortCutKeyFunction( uc_Index ).e_TopMenuShortCutType ){
						bl_Result = True;
						break;
					}
				}
			}
		}else{
			TYPS_ExtShortCutList st_BadgeKeyList;
			memset( &st_BadgeKeyList, 0, sizeof( st_BadgeKeyList ) );
			PnlComGetStandardPanelSetting()->getShortCutListSetting( &st_BadgeKeyList, PNLC_NetworkLan::getInstance()->getLine() );
			for( uc_Index = 0; uc_Index < ( TYPD_ExtShortcutMaxTopMenu + TYPD_ExtShortcutAlwaysShowTopMenu ); uc_Index++ ){
				if( TYPD_ExtShortcutMaxTopMenu <= uc_Index ){
					if( TYPE_TopMenuShortCutTypeMarketPlaceSetup == st_BadgeKeyList.s_ShortCutKeyAlwaysShow[ uc_Index - TYPD_ExtShortcutMaxTopMenu ].e_ShortCutType ){
						bl_Result = True;
						break;
					}
				}else{
					if( TYPE_TopMenuShortCutTypeMarketPlaceSetup == st_BadgeKeyList.s_ShortCutKey[ uc_Index ].e_ShortCutType ){
						bl_Result = True;
						break;
					}
				}
			}
		}
	}
	return bl_Result;
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： Qtボタン.
//================================================================================================
void PCGC_HomeTopMenu::procQtButtonOn( ushort option ) {
	if( True == PnlComMtuiIf_TopMenuModel::instance()->isBudgeActivate( ( int32_t )option ) ) {
		// 画面遷移.
		procGoToPage( ( uchar )option );
	} else {
		// 拒否音鳴動.
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： Qtボタンダブルタップ.
//================================================================================================
void PCGC_HomeTopMenu::procDoubleTap( ushort option ) {
	procGoToPage( ( uchar )option );
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： 登録アプリケーションキー処理（OAPアプリの場合）.
//================================================================================================
// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
//void PCGC_HomeTopMenu::procAPPOap( uchar option )
void PCGC_HomeTopMenu::procAPPOap( uchar option, Bool bl_fromWebUI )
{
	if( True == existWarningGamen() ) {
		// 警告画面を再表示する
		rebuildGamen( 2 );
		return;
	}

	if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type3() ) {
		PCGC_ScanOapCom::setApplicationNotifyDisconnect( False );
	}
	m_b_ShowTopMenu = True;
	PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();  //  ポップアップ管理クラスのインスタンス取得.
	TYPE_ExtType s_ExtType = TYPE_ExtTypeScanD;		// APIC_ExtSetting インスタンス取得 .	//comment ZeusMLK【 静的解析対応 : nie : 2012.08.31 : CID446465 】.
// comment 2018/11/05 SCD2-Sky中野 [EagleH] WebUIv3FUM対応.
//	ushort us_AppNo = us_OAPAppNo[ option ];
	ushort us_AppNo = 0;
// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
//	if( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() ) {
	if( True == bl_fromWebUI ) {
		us_AppNo = PNLC_WebUIControl::getInstance()->getOapAppNo();
	} else {
		us_AppNo = us_OAPAppNo[ option ];
	}
	
	APIC_ExtSetting* pc_ExtSetting = APIC_ExtSetting::newInstance( us_AppNo );
	Bool b_ExpireDateChk;

	if( NULL != pc_ExtSetting ) {
		s_ExtType = pc_ExtSetting->getApplication().s_Version.e_Type;
	}

	PBasicMessageProc* pc_Gamen;		// OpenAPI認証管理.//【CQ380160】【Closing Browser Launcher Application on starting another Browser Launcher Application】【HCL_Vimal】2013/06/07 Add.
	if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type3() ) {
		if( True == AplCommonSet.isActiveGeneralApplication( TYPE_JobTypeCopy, PNLE_CheckType_ALL ) ) {
			AplCommonSet.notifyDisconnect( False );
			b_ExpireDateChk = ScanOapExtSet.isEnableExpireDate( &pc_Gamen,True,us_AppNo,5 );
		} else {
			b_ExpireDateChk = ScanOapExtSet.isEnableExpireDate( &pc_Gamen,True,us_AppNo,4 );
		}
	} else {
		b_ExpireDateChk = ScanOapExtSet.isEnableExpireDate( &pc_Gamen,True,us_AppNo,4 );
	}

	if( False == b_ExpireDateChk ) {		// 限定確認.
		// 指定アプリ設定中.
		if( True == c_ExtPanel.getSpecifiedAppSetting() ) {
			onlyPanel.setPanelWarning( PNL_ERROR_ExtSpecifiedAppExpireDateError );
		} else {
// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
//// comment 2019/09/17 SCD2-Sky中野 [EagleH][OP_BTS-13559].
//			if( &gp_AppChangeUIBrowser == nowGamen ) {
			if( ( &gp_AppChangeBrowser == getNowGamen() )
			 || ( &gp_AppChangeUIBrowser == getNowGamen() )
			 || ( &gp_PullPrintOapBrowserDummy == getNowGamen() )
			 || ( &gp_CostCenterResetDummy == getNowGamen() )
			 || ( &gp_ScanOapBrowserDummy == getNowGamen() )
			 || ( &gp_GeneralOapBrowserDummy == getNowGamen() )
			 || ( &gp_WarningPopupDummyBase == getNowGamen() )
			 || ( &gp_AppChangeScreenSaver== getNowGamen() )
			 || ( &gp_AuthOapBrowserLogin == getNowGamen() ) ) {
				gp_PublicPopupDummyBase.setupOpenPopup( &gp_ScanOapExpireDateErrorExt_warn );
				gp_PublicPopupDummyBase.setupNextGamen( nowGamen );
				gp_PublicPopupDummyBase.setMode( PNLE_PublicPopupDummyModeAct );
				nextGamen = &gp_PublicPopupDummyBase;
				OpenPopupIndi();
			} else {
// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
//				pc_PopupListControl->addPopup( &gp_ScanOapExpireDateErrorExt_warn, this, NULL );
				PBasicMessageProc* pc_nowGamen = NULL;
				if( True == pc_PopupListControl->isPopupOpen() ) {
					pc_nowGamen= pc_PopupListControl->getNowPopup();
				} else {
					pc_nowGamen= getNowGamen();
				}
				pc_PopupListControl->addPopup( &gp_ScanOapExpireDateErrorExt_warn, pc_nowGamen, NULL );
			}
		}
		return;
	}
	PNLE_OAP_VENDERLIMIT e_limit = PNLE_OAP_VENDERLIMIT_NONE;
	e_limit = ScanOapExtSet.checkOapVenderLimit( us_AppNo );
	if( PNLE_OAP_VENDERLIMIT_POWER_SUPPLY == e_limit ) {
		onlyPanel.setPanelWarning( PNL_ERROR_CheckVendorPowerSupplyApp ); // Venodr power is OFF state.
		if( NULL != pc_PopupListControl ) {
			pc_PopupListControl->addPopup( &gp_CheckVendorPowerSupplyAppExt_warn, this, NULL );
		}
		return;
	} else if( PNLE_OAP_VENDERLIMIT_COIN_UNSET == e_limit ) {
		onlyPanel.setPanelWarning( PNL_ERROR_CoinUnsetApp ); // Coin not inserted state.
		if( NULL != pc_PopupListControl ) {
			pc_PopupListControl->addPopup( &gp_CoinUnsetAppExt_warn, this, NULL );
		}
		return;
	}
// comment 2019/09/27 SCD2-Sky浜野 [EagleH] NodeJsアプリの起動条件追加.
	if( False == ScanOapExtSet.IsNodeJsAppEnable((uchar)us_AppNo) ){
// comment 2020/11/09 HCL_Prabakaran [IT6_2.1][ OP_BTS-28914] Log added to confirm issue in future occurrence.
		PNL_LogManager::newInstance()->setGeneralPurposeLog("RejectKey(IsNodeJsAppEnable)", (ulong)__LINE__ );
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	// カレントアプリNo情報設定.
	ScanOapExtSet.setCurrentAppNo( us_AppNo );
	ScanOapExtSet.getCurrentAplInfo();
	TYPC_ExtAplDetail* t_ExtAplDetail = ScanOapExtSet.getAplDetail();

	ScanOapExtSet.setCheckMode( False );

	// 選択アプリタイプチェック.
	switch( s_ExtType ) {
		// スキャン(TypeD).
		case TYPE_ExtTypeScanD:
			// アプリ種別設定
			ScanOapExtSet.setExtAppType( TYPE_ExtTypeScanD );
			// JobType 設定
			setJobType( TYPE_ExtTypeScanD );
			PCGC_TreeScreen::initializeNode();
			// アプリケーション接続中画面へ.
			// アプリケーション開始要求.
// Comment 2015/10/30 HCLT-Vivek [ZeusS_ZX0][制御構成変更_No 76][WP 2553] Counter value to be udpated for Open API  application.
	    	PnlComIncreaseOpenApiCnt( us_AppNo );  															// Open API counter increament.
			nextGamen = ScanOapExtSet.req_OpenAPI( PNLE_OAP_REQ_APLCONNECT );
			break;

		// スキャン(TypeS).
		case TYPE_ExtTypeScanS:
		{
			// アプリ種別設定
			ScanOapExtSet.setExtAppType( TYPE_ExtTypeScanS );
			// JobType 設定
			setJobType( TYPE_ExtTypeScanS );
			PnlComSetActiveAplNo( ( uchar )ScanOapExtSet.getCurrentAppNo(), True );		// 起動アプリNo設定.
			// 原稿設定を AppDetail→JobSpecへ設定
			ScanOapExtSet.setDefaultDocumentSetting( comPanelJob );
			// 認証画面・カスタム設定画面用かの切替設定.
			PCGC_ScanOapExtDataList* datlist = ScanOapExtSet.getExtDataList( ScanOapExtSet.getCustomNo() );
			datlist->setAuthMode( False );
			// 複数かどうかにより遷移先を変更する。
			if( ( NULL == t_ExtAplDetail ) || ( NULL == t_ExtAplDetail->getScanS() ) ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;}
			if( 1 < t_ExtAplDetail->getScanS()->s_CustomList.sc_Count ) {
				// 画面タイプを判定してnextGamenに設定
				switch( t_ExtAplDetail->getScanS()->e_ListType ) {
					case TYPE_ExtListTypeLarge:		// 複数拡張設定画面_Large
// Comment 2015/10/30 HCLT-Vivek [ZeusS_ZX0][制御構成変更_No 76][WP 2553] Counter value to be udpated for Open API  application.
				    	PnlComIncreaseOpenApiCnt( us_AppNo );  															// Open API counter increament.
						nextGamen = &gp_MultipleCustomSettingExtLarge_scanfax;
						break;
					case TYPE_ExtListTypeMiddle:	// 複数拡張設定画面_Medium
// Comment 2015/10/30 HCLT-Vivek [ZeusS_ZX0][制御構成変更_No 76][WP 2553] Counter value to be udpated for Open API  application.
				    	PnlComIncreaseOpenApiCnt( us_AppNo );  															// Open API counter increament.
						nextGamen = &gp_MultipleCustomSettingExtMedium_scanfax;
						break;
					case TYPE_ExtListTypeSmall:		// 複数拡張設定画面_Small
// Comment 2015/10/30 HCLT-Vivek [ZeusS_ZX0][制御構成変更_No 76][WP 2553] Counter value to be udpated for Open API  application.
				    	PnlComIncreaseOpenApiCnt( us_AppNo );  															// Open API counter increament.
						nextGamen = &gp_MultipleCustomSettingExtSmall_scanfax;
						break;
					default:	break;				// ここに来る事はない？
				}
			} else {
				// 内部で保持している拡張コントロールデータを取得データで初期化
				ScanOapExtSet.setCustomNo( 1 );
				datlist->setFromExtSetting( ScanOapExtSet.getCurrentAplInfo(), 1 );
				PCGC_ScanOapBasicStaticLib::b_EnableCustomSettingTypeS = True;
// Comment 2015/10/30 HCLT-Vivek [ZeusS_ZX0][制御構成変更_No 76][WP 2553] Counter value to be udpated for Open API  application.
		    	PnlComIncreaseOpenApiCnt( us_AppNo );  															// Open API counter increament.
				// カスタム設定画面.
				nextGamen = &gp_CustomSettingTypeSExt_scanfax;
			}
			break;
		}
		// PullPrint.
		case TYPE_ExtTypePullPrint:
			// アプリ種別設定
			ScanOapExtSet.setExtAppType( TYPE_ExtTypePullPrint );
			// JobType 設定
			setJobType( TYPE_ExtTypePullPrint );
			PCGC_TreeScreen::initializeNode();
			// アプリケーション接続中画面へ.
			// アプリケーション開始要求.
// Comment 2015/10/30 HCLT-Vivek [ZeusS_ZX0][制御構成変更_No 76][WP 2553] Counter value to be udpated for Open API  application.
	    	PnlComIncreaseOpenApiCnt( us_AppNo );  															// Open API counter increament.
			nextGamen = ScanOapExtSet.req_OpenAPI( PNLE_OAP_REQ_APLCONNECT );
			break;
		// JobManagement.
		case TYPE_ExtTypeJobManagement:		break;
		// Authentication.
		case TYPE_ExtTypeAuthentication:	break;
		// RightFax.
		case TYPE_ExtTypeRightFax:			
// Comment 2015/12/15 HCLT-Krithika [ZeusS_ZX0][制御構成変更_No 76][WP 2553] [CQ522124] commented right fax is not part of OpenAPI.
// Comment 2015/10/30 HCLT-Vivek [ZeusS_ZX0][制御構成変更_No 76][WP 2553] Counter value to be udpated for Open API  application.
//	    	PnlComIncreaseOpenApiCnt( us_AppNo );  															// Open API counter increament.
			break;
		// Browser(Auth).
		case TYPE_ExtTypeBrowserAuth:		break;
		// Browser(Scan).
		case TYPE_ExtTypeBrowserScan:
			// アプリ種別設定
			ScanOapExtSet.setExtAppType( TYPE_ExtTypeBrowserScan );
			// JobType 設定
			setJobType( TYPE_ExtTypeBrowserScan );
			PCGC_TreeScreen::initializeNode();
// Comment 2015/10/30 HCLT-Vivek [ZeusS_ZX0][制御構成変更_No 76][WP 2553] Counter value to be udpated for Open API  application.
	    	PnlComIncreaseOpenApiCnt( us_AppNo );  															// Open API counter increament.
			// アプリケーション接続中画面へ.
			// アプリケーション開始要求.
			nextGamen = ScanOapExtSet.req_OpenAPI( PNLE_OAP_REQ_APLCONNECT );
			break;
		// Browser(Pull).
		case TYPE_ExtTypeBrowserPull:
			// アプリ種別設定
			ScanOapExtSet.setExtAppType( TYPE_ExtTypeBrowserPull );
			// JobType 設定
			setJobType( TYPE_ExtTypeBrowserPull );
			PCGC_TreeScreen::initializeNode();
// Comment 2015/10/30 HCLT-Vivek [ZeusS_ZX0][制御構成変更_No 76][WP 2553] Counter value to be udpated for Open API  application.
	    	PnlComIncreaseOpenApiCnt( us_AppNo );  															// Open API counter increament.
			// アプリケーション接続中画面へ.
			// アプリケーション開始要求.
			nextGamen = ScanOapExtSet.req_OpenAPI( PNLE_OAP_REQ_APLCONNECT );
			break;
		// Browser(ScrSvr).
		case TYPE_ExtTypeBrowserScrSvr:		break;
		// BrowserLauncher.
		case TYPE_ExtTypeBrowserLauncher:
// comment 2019/04/16 SCD2-Sky中野 [EagleH] 目的別UI_1ペイン対応.
		// 目的別UIアプリ.
		case TYPE_ExtTypeCustomUI:
			// IWS機能起動準備中の場合.
			if( PNLE_IWS_Status_Ready == PnlComGetIwsStatus() ) {
// comment 2020/11/09 HCL_Prabakaran [IT6_2.1][ OP_BTS-28914] Log added to confirm issue in future occurrence.
				PNL_LogManager::newInstance()->setGeneralPurposeLog("RejectKey(PnlComGetIwsStatus)", (ulong)__LINE__ );
				setBuzzerType( PCGE_BuzzerType_InputNg );
				return;
			}
// Comment 2015/12/15 HCLT-Krithika [ZeusS_ZX0][制御構成変更_No 76][WP 2553] [CQ522124] commented and moved below.
// Comment 2015/10/30 HCLT-Vivek [ZeusS_ZX0][制御構成変更_No 76][WP 2553] Counter value to be udpated for Open API  application.
//	    	PnlComIncreaseOpenApiCnt( us_AppNo );  															// Open API counter increament.
// Comment 2015/10/30 HCLT-Vivek [ZeusS_ZX0][制御構成変更_No 76][WP 2551] Counter value to be udpated for IWS application at plibh.cpp it will get executed only for IWS browser UI application.
// 			PnlComIncreaseIwsCnt( us_AppNo );  															    // IWS Counter increament.
// comment 2022/07/19 Sky Kawanishi [LocalUI特ROM][TAS000000011547]DipSW有効時、アプリ起動前にcookie削除する対応.
			PnlComBrowserDeleteAllCookieWithAppStart();
			// ブラウザ起動モードをブラウザーランチャーとして再起動する。
			PnlComBrowserLauncherModeReboot();
			// ブラウザーアプリへ切り替え.
			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type3() ) {
				ScanOapExtSet.m_bTopmenuModeBackup = onlyPanel.isTopmenuMode();
			}
// comment 2021/12/20 Sky中野 [MinervaSSBK][OP_BTS-36843].
//			procBrowser( 0 );
			procBrowser( uc_OptionBrowserLauncherBoot );
// comment 2019/04/16 SCD2-Sky中野 [EagleH] 目的別UI_1ペイン対応.
//			if( &gp_AppChangeBrowser == nextGamen ) {
			if( ( &gp_AppChangeBrowser == nextGamen ) 
			 || ( &gp_AppChangeUIBrowser == nextGamen ) ){
// Comment 2015/12/15 HCLT-Krithika [ZeusS_ZX0][制御構成変更_No 76][WP 2553] [CQ522124] Counter value to be udpated for OpenAPI / IWS application.
				APIC_ExtSetting* pc_AppInfo = APIC_ExtSetting::newInstance( us_AppNo );	// インスタンス取得.
				if(NULL != pc_AppInfo){
					if( ( True == pc_AppInfo->isIwsApplication() )
						&& ( PNLE_IWS_Status_Bootup == PnlComGetIwsStatus() ) ){
						//to be udpated for IWS application at plibh.cpp it will get executed only for IWS browser UI application.
						PnlComIncreaseIwsCnt( us_AppNo );  									// IWS counter increament.
					} else {
						// to be udpated for OpenAPI application.
						PnlComIncreaseOpenApiCnt( us_AppNo );								// Open API counter increament.
					}
				}
// comment 2019/04/16 SCD2-Sky中野 [EagleH] 目的別UI_1ペイン対応.
//				ScanOapExtSet.setExtAppType( TYPE_ExtTypeBrowserLauncher );
				ScanOapExtSet.setExtAppType( s_ExtType );
			}
			break;
		case TYPE_ExtTypeBrowserGeneral:	// ブラウザ統合アプリケーション.

			// アプリ種別設定.
			ScanOapExtSet.setExtAppType( TYPE_ExtTypeBrowserGeneral );
			// JobType 設定.
			setJobType( TYPE_ExtTypeBrowserGeneral );
			// ノードを削除.
			PCGC_TreeScreen::initializeNode();
// Comment 2015/10/30 HCLT-Vivek [ZeusS_ZX0][制御構成変更_No 76][WP 2553] Counter value to be udpated for Open API  application.
	    	PnlComIncreaseOpenApiCnt( us_AppNo );  															// Open API counter increament.
			// アプリケーション開始要求.
			nextGamen = ScanOapExtSet.req_OpenAPI( PNLE_OAP_REQ_APLCONNECT );
			break;
		default :	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
	}
	// アップロードファイル作成要求中の画面遷移の場合.
	if( ( ( nextGamen != NULL )
	      ||( True == PNLC_PopupListControl::newInstance()->isNowPopup( &gp_ScanOapAuthConnectingExt_act ) )	)	// アプリケーション接続中画面がポップアップ中.
	    && ( True == onlyPanel.bl_BrowserFileUploadRequest ) ) {
		uchar uc_UploadType = 0;
		if( &gp_FileUploadTypeSelectExt_browser == getNowGamen() ) {	// アップロード機能選択.
			uc_UploadType = PCGD_BrowserFileUploadTypeSelect;
		}
		deleteUploadInfo( uc_UploadType );								// アップロードファイル情報削除.
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： ジョブタイプ設定.
//================================================================================================
void PCGC_HomeTopMenu::setJobType( TYPE_ExtType type )
{
	// OAPアプリ起動のタイミングでトップメニュー遷移フラグをバックアップ.
	// OAPアプリから抜けるタイミングでトップメニュー遷移フラグを書き戻す.
	// 上記を行わないとトップメニューからの戻り画面が存在しない場合に.
	// トップメニュー画面でのMENUキー押下にてハングなどの現象が発生する.
	ScanOapExtSet.m_bTopmenuModeBackup = onlyPanel.isTopmenuMode();

	switch( type ) {
		case TYPE_ExtTypeScanD:				// スキャン(TypeD).
		case TYPE_ExtTypeScanS:				// スキャン(TypeS).
		case TYPE_ExtTypeBrowserScan:		// Browser(Scan).
			// TypeDの場合は Scan を設定する
			c_JobControl.changePanelAPP( TYPE_JobTypeFaxSendMemory );    // 操作パネル上のアプリケーション表示を切り換える。
			c_JobControl.InitJobSetting( comPanelJob );			// ジョブの設定を初期化する。
			ScanOapExtSet.backupPanelDefaultSetting( comPanelJob );	// 読込設定の本体Default値を保存する。
			onlyPanel.ep_PanelStatus = judgePanelStatus( comPanelJob->getJobStatus()->getInputStatus(), comPanelJob->getJobStatus()->getOutputStatus(), comPanelJob->getJobStatus()->getFaxJobStatus(), comPanelJob->getJobStatus()->getRealTimePreviewStatus() );
			selectAutoPaperMag();           // APS/AMS処理関数
			break;
		case TYPE_ExtTypePullPrint:
		case TYPE_ExtTypeBrowserPull:
			onlyPanel.setTopmenuMode( False );	//トップメニューを終了.
// comment 2020/01/29 SCD2-Sky中野 [SparrowBK][OP_BTS-19103].
			// PullPrintの印刷設定画面でcp_copyJobを使用するため設定を初期化.
			c_JobControl.changePanelAPP( TYPE_JobTypeCopy );    // 操作パネル上のアプリケーション表示を切り換える。
			c_JobControl.InitJobSetting( cp_copyJob );	// ジョブの設定を初期化する。
			// BOXからアプリ起動した際にボックス使用中状態が残るため初期化.
			onlyPanel.b_nowBoxMode = False;
			onlyPanel.setUbqPrintMode( False );			// ユビキタスプリントモードをoffにする.
			sc_JobPutNo = PCGE_JobPutOther;
			break;
		case TYPE_ExtTypeBrowserGeneral:	// ブラウザ統合アプリケーション.
			// 注意）「::setJobType( TYPE_ExtTypeBrowserGeneral )」をコールする前には.
			// 必ず「PCGC_ScanOapCom::setCurrentAppNo()」で起動アプリのNo.を設定していることを確認すること.
			// PullPrintジョブが使用出来るブラウザ統合アプリケーションのみ下記処理を行う.
			if( True == ScanOapExtSet.isExtPullJob() ) {
				onlyPanel.setTopmenuMode( False );							//トップメニューを終了.
				// PullPrintの印刷設定画面でcp_copyJobを使用するため設定を初期化.
				c_JobControl.InitJobSetting( cp_copyJob );					// ジョブの設定を初期化する.
				// BOXからアプリ起動した際にボックス使用中状態が残るため初期化.
				onlyPanel.b_nowBoxMode = False;
				onlyPanel.setUbqPrintMode( False );			// ユビキタスプリントモードをoffにする.
				sc_JobPutNo = PCGE_JobPutOther;
			}
			// スキャンジョブが使用出来るブラウザ統合アプリケーションのみ下記処理を行う.
			if( True == ScanOapExtSet.isExtScanJob() ) {
				c_JobControl.changePanelAPP( TYPE_JobTypeFaxSendMemory );	// 操作パネル上のアプリケーション表示を切り換える.
				c_JobControl.InitJobSetting( comPanelJob );					// ジョブの設定を初期化する.
				ScanOapExtSet.backupPanelDefaultSetting( comPanelJob );		// 読込設定の本体Default値を保存する.
				// パネルステータスを決定する.
				onlyPanel.ep_PanelStatus = judgePanelStatus( comPanelJob->getJobStatus()->getInputStatus(), comPanelJob->getJobStatus()->getOutputStatus(), comPanelJob->getJobStatus()->getFaxJobStatus(), comPanelJob->getJobStatus()->getRealTimePreviewStatus() );
				selectAutoPaperMag();										// APS/AMS処理関数.
			}
			break;
		default:	break;
	}
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： 登録アプリケーションキー処理（汎用アプリの場合）.
//================================================================================================
// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
//void PCGC_HomeTopMenu::procAPPHanyo( uchar option )
void PCGC_HomeTopMenu::procAPPHanyo( uchar option, Bool bl_fromWebUI )
{
	m_b_ShowTopMenu = True;


	// アプリ情報取得.
	ushort us_AppNo = 0;
// comment 2018/11/05 SCD2-Sky中野 [EagleH] WebUIv3FUM対応.
//	getAppInfoFromName( st_BadgeKeyInfo[ option ].sc_FuncName, us_AppNo );
// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
//	if( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() ) {
	if( True == bl_fromWebUI ) {
		us_AppNo = PNLC_WebUIControl::getInstance()->getGeneralAppNo();
	} else {
		getAppInfoFromName( st_BadgeKeyInfo[ option ].sc_FuncName, us_AppNo );
	}

	APIC_AppInfoSetting* aplHanyo = APIC_AppInfoSetting::newInstance( us_AppNo );
	if( aplHanyo == NULL ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
		
// comment 2019/02/15 SCD2-Sky中野 [WebUIv3][1072906] 汎用アプリケーション画面へ遷移できない問題対応.
	PNLE_UIMode e_UIMode = PNLC_WebUIControl::getInstance()->getUIMode();
	if( PNLE_UIModeLightUI == e_UIMode ) {
		// 汎用アプリへの遷移時はUIモードをPNLE_UIModeLegacyUIに変更する.
		PNLC_WebUIControl::getInstance()->setUIMode( PNLE_UIModeLegacyUI );
	}
		
	c_ExtPanel.setShortcutAppBootNo( us_AppNo );	// 起動するAppNoを覚えておく.
	ScanOapExtSet.setCheckMode( False );
	onlyPanel.bl_isFromAppShortcut = True;		// [MZ-121対応] RightFax使用時のデフォルト画面変更. comment 2013/07/18 OPS1-Sky石賀 [ZM3rd][市場要望対応No.3011].
	procFax( option );							// スキャンファクスモードへ.
	onlyPanel.bl_isFromAppShortcut = False;		// [MZ-121対応] RightFax使用時のデフォルト画面変更. comment 2013/07/18 OPS1-Sky石賀 [ZM3rd][市場要望対応No.3011].
	c_ExtPanel.setShortcutAppBootNo( 0 );			// 起動するAppNoを初期化.
		
// comment 2019/02/15 SCD2-Sky中野 [WebUIv3][1072906] 汎用アプリケーション画面へ遷移できない問題対応.
	if( PNLE_UIModeLightUI == e_UIMode ) {
		// 画面遷移後はUIモードを元に戻す.
		PNLC_WebUIControl::getInstance()->setUIMode( e_UIMode );
	}
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： 登録アプリケーションメニューキー処理.
//================================================================================================
void PCGC_HomeTopMenu::procAPPsMenu( uchar option )
{
// comment 2016/09/16 MSE岡田 [IT5_3.3][1005694] 省電力復帰のHDDチェックスキップ時は受付け禁止とする.
	if( True == PNLC_SnapshotControlLib::newInstance()->getSkipCheckHDD() ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	// APP起動中.
	if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type3() ) {
		if( False == c_ExtPanel.isEnableMFPFunction() ) {
// comment 2019/04/22 Sky東島 [IT6_1.0MR2][AR1075688].
//			setBuzzerType( PCGE_BuzzerType_InputNg );
			TYPE_ExtType e_APPType = ScanOapExtSet.getAplType();		// 起動中アプリタイプを取得.
// comment 2019/06/26 Sky東島 [EagleHigh][OP_BTS-9020].
//			if( 0 == e_APPType ){
//				// アプリが起動していない場合は遷移させない.
//				setBuzzerType( PCGE_BuzzerType_InputNg );
//				return;
			if( 0 == e_APPType
			 || NULL == ScanOapExtSet.getAppTopGamen_backup() ){
				// アプリが起動していない場合は指定アプリ起動要求を行う.
				c_ExtPanel.req_SpecifiedAppStart();
				PnlComSetInitIndicationApplication();
				rebuildGamen( 0 );
				return;
			}
			onlyPanel.setTopmenuMode( False );	// 優先機能トップメニューを解除.
			onlyPanel.setBrowserMode( False );	// ブラウザモードOFF.

			BRWE_BrowserBootMode e_BrowserBootMode = 0;
			switch( e_APPType ){
				case TYPE_ExtTypeBrowserLauncher:						// ブラウザランチャ.
					e_BrowserBootMode = BRWE_BrowserBootMode_Launcher;
					onlyPanel.setBrowserMode( True );
// comment 2019/05/20 SCD2-Sky肥後 [EagleH][OP_BTS-6914] PSWCからのデータ変化通知を受けても、WebUIの再起動要求をかけない.
//					PNLC_WebUIControl::getInstance()->setWebUIStartStatus( PNLE_WebUIStartStatusNeedStart );	// ベーシックスタイルの場合gp_AppChangeUIBrowserに遷移してしまうためステータスを要起動にする.
					break;
				case TYPE_ExtTypeBrowserGeneral:						// Generalブラウザ.
					e_BrowserBootMode = BRWE_BrowserBootMode_General;
					break;
				case TYPE_ExtTypeBrowserScan:							// Scan連携ブラウザ.
					e_BrowserBootMode = BRWE_BrowserBootMode_Scan;
					break;
				case TYPE_ExtTypeBrowserPull:							// PullPrintブラウザ.
					e_BrowserBootMode = BRWE_BrowserBootMode_Pull;
					break;
				default:
					// 何もしない.
					break;
			}
			if( ( e_BrowserBootMode != onlyPanel.e_BrowserBootMode )
			 && ( 0 != e_BrowserBootMode ) ){
				// 起動中のブラウザモードと優先アプリのブラウザモードが異なる場合はrebuildGamenにてブラウザ再起動させるためフラグを設定しておく.
				onlyPanel.setPrevUtilityBrowserMode( e_BrowserBootMode );
			}
			rebuildGamen( 0 );
			return;
		}
	} else {
		const TYPS_ExtRegistration* pc_ExtRegistration = ScanOapExtSet.getApplication();
		if( True == ScanOapExtSet.isActiveGeneralApplication()
		    && NULL != pc_ExtRegistration
		    && (	( TYPE_ExtTypeScanS == pc_ExtRegistration->s_Version.e_Type )
		            || ( TYPE_ExtTypeScanD == pc_ExtRegistration->s_Version.e_Type )
		            || ( TYPE_ExtTypePullPrint == pc_ExtRegistration->s_Version.e_Type )
		            || ( TYPE_ExtTypeBrowserScan == pc_ExtRegistration->s_Version.e_Type )
		            || ( TYPE_ExtTypeBrowserPull == pc_ExtRegistration->s_Version.e_Type )
		            || ( TYPE_ExtTypeBrowserGeneral == pc_ExtRegistration->s_Version.e_Type )	) ) {

			setBuzzerType( PCGE_BuzzerType_InputNg );
			return;
		}
		// ブラウザランチャーモード時は、APPキー無効.
		if( ( True == onlyPanel.getBrowserMode() ) && ( BRWE_BrowserBootMode_Launcher == onlyPanel.e_BrowserBootMode ) ) {
			setBuzzerType( PCGE_BuzzerType_InputNg );
			return;
		}
	}//【UV-01342】【OpenAPI／ブラウザランチャー起動条件の改善】【HCL_Krishna】2013/02/19 Add.
	// 指定アプリ初期起動設定関連警告が発生している場合はキー無効.
	if( True == c_ExtPanel.checkSpecifiedInitialWarning() ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	// アプリケーション選択チェック.
	if( False == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_MAX ) ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	if( True == existWarningGamen() ) {
		// 警告画面を再表示する
		rebuildGamen( 2 );
		return;
	}

	c_ExtPanel.ChangeSpecifiedAppState( PNLE_SpecifiedAppState_Idle );	// 指定アプリ状態遷移( 待機中 ).

	m_b_ShowTopMenu = True;
// comment OAP-NUI <- Sky半田 2011/09/07 [CQ353888].
//【CQ #369837】【Connecting to application directly when register keys are set for single application from HOMETOPMENU screen】【HCL_PRAVEEN KUMAR】 2012/08/09 Mod Start.
//	nextGamen = ScanOapExtSet.getAppStartGamen();
// comment 2014/11/29 OPS1-Sky後藤 [Poseidon][CQ504420].
//	nextGamen = ScanOapExtSet.getAppStartGamen(option);
	Bool bl_checkError = False;
	nextGamen = ScanOapExtSet.getAppStartGamen(option,&bl_checkError);
	if( True == bl_checkError ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
//【CQ #369837】【Connecting to application directly when register keys are set for single application from HOMETOPMENU screen】【HCL_PRAVEEN KUMAR】 2012/08/09 Mod End.
// comment OAP-NUI -> Sky石賀 2011/08/25 [CQ346202].
	// アップロードファイル作成要求中の画面遷移の場合.
	if( ( ( nextGamen != NULL )
	      ||( True == PNLC_PopupListControl::newInstance()->isNowPopup( &gp_ScanOapAuthConnectingExt_act ) )	)	// アプリケーション接続中画面がポップアップ中.
	    && ( True == onlyPanel.bl_BrowserFileUploadRequest ) ) {
		uchar uc_UploadType = 0;
		if( &gp_FileUploadTypeSelectExt_browser == getNowGamen() ) {	// アップロード機能選択.
			uc_UploadType = PCGD_BrowserFileUploadTypeSelect;
		}
		deleteUploadInfo( uc_UploadType );								// アップロードファイル情報削除.
	}
// comment 2023/04/17 Sky中野 [IT6_2.3][OP_BTS-42686].
	if( PCGE_BuzzerType_InputNg != getBuzzerType() ){
		// WebUIジョブ設定の初期化要求を通知する.
		PNLC_WebUIControl* pc_WebUIControl = PNLC_WebUIControl::getInstance();
		if( NULL != pc_WebUIControl ) {
			pc_WebUIControl->initWebUIJobSettings();
		}
	}
}

// ===============================================================================================.
//	概要	: スライドメニュー 登録アプリケーションキー押下.
//  引数    : オプション.
//	戻り値	: なし.
// ===============================================================================================.
void PCGC_HomeTopMenu::procSpecifyApp( uchar option )
{
	// APP起動中.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	// 指定アプリ初期起動設定関連警告が発生している場合はキー無効.
	if( True == c_ExtPanel.checkSpecifiedInitialWarning() ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	// アプリケーション選択チェック.
	if( False == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_MAX ) ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	if( True == existWarningGamen() ) {
		// 警告画面を再表示する
		rebuildGamen( 2 );
		return;
	}

	c_ExtPanel.ChangeSpecifiedAppState( PNLE_SpecifiedAppState_Idle );	// 指定アプリ状態遷移( 待機中 ).

	m_b_ShowTopMenu = True;
	// comment 2014/11/29 OPS1-Sky後藤 [Poseidon][CQ504420].
	//	nextGamen = ScanOapExtSet.getAppStartGamenSpecify( option );
	Bool bl_checkError = False;
	nextGamen = ScanOapExtSet.getAppStartGamenSpecify(option,&bl_checkError);
	if( True == bl_checkError ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	// アップロードファイル作成要求中の画面遷移の場合.
	if( ( ( nextGamen != NULL )
	      ||( True == PNLC_PopupListControl::newInstance()->isNowPopup( &gp_ScanOapAuthConnectingExt_act ) )	)	// アプリケーション接続中画面がポップアップ中.
	    && ( True == onlyPanel.bl_BrowserFileUploadRequest ) ) {
		uchar uc_UploadType = 0;
		if( &gp_FileUploadTypeSelectExt_browser == getNowGamen() ) {	// アップロード機能選択.
			uc_UploadType = PCGD_BrowserFileUploadTypeSelect;
		}
		deleteUploadInfo( uc_UploadType );								// アップロードファイル情報削除.
	}
// comment 2023/04/17 Sky中野 [IT6_2.3][OP_BTS-42686].
	if( PCGE_BuzzerType_InputNg != getBuzzerType() ){
		// WebUIジョブ設定の初期化要求を通知する.
		PNLC_WebUIControl* pc_WebUIControl = PNLC_WebUIControl::getInstance();
		if( NULL != pc_WebUIControl ) {
			pc_WebUIControl->initWebUIJobSettings();
		}
	}
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： ガイドキー処理.
//================================================================================================
void PCGC_HomeTopMenu::procHelp( uchar option )
{
	setBuzzerType( PCGE_BuzzerType_InputNg );
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： ユーティリティキー処理.
// 履歴	:2018/08/10 SCD2 T.Nishiba	[Eagle][RQ-698-41]設定メニュー画面のメニュー項目変更に伴い、.
//									設定メニューキーを常に有効にする.
//================================================================================================
void PCGC_HomeTopMenu::procUtility( uchar option )
{
	// 指定アプリ起動中の場合.
	if( c_ExtPanel.CheckSpecifiedAppActive() ) {
		// 指定アプリ起動中画面から遷移する場合は再起動を行う.
		if( PNLE_SpecifiedAppState_Started == c_ExtPanel.getSpecifiedAppState() ) {
			c_ExtPanel.req_SpecifiedAppStart();
		}
		// 親クラスを呼び出し、以降の処理を行わない.
		PMessageArea::procUtility( option );
		return;
	}
	// comment 2018/08/10 SCD2 T.Nishiba [Eagle][RQ-698-41]新設定メニューのメニュー項目対応.
	const TYPS_ExtRegistration* pc_ExtRegistration = ScanOapExtSet.getApplication();
// Comment 2018/11/08 [IT6_Eagle] [AR1066455] [HCLT-Shalini] - Commented and and moved condition to User Settings and Admin Settings procGotoPage..
// Comment 2018/08/24 [IT6_Eagle] [AR1058785] [HCLT-Sam Sundar] - Resolving the timing issue to display the User Settings and Admin Settings.
//	if( True == APIC_NetworkDevice::newInstance()->isNetworkInitialing() ){
//		setBuzzerType( PCGE_BuzzerType_InputNg );
//	}else if( True == ScanOapExtSet.isActiveGeneralApplication()
	if( True == ScanOapExtSet.isActiveGeneralApplication()
	    && NULL != pc_ExtRegistration
	    && (	( TYPE_ExtTypeScanS == pc_ExtRegistration->s_Version.e_Type )
	            || ( TYPE_ExtTypeScanD == pc_ExtRegistration->s_Version.e_Type )
	            || ( TYPE_ExtTypePullPrint == pc_ExtRegistration->s_Version.e_Type )
	            || ( TYPE_ExtTypeBrowserScan == pc_ExtRegistration->s_Version.e_Type )
	            || ( TYPE_ExtTypeBrowserPull == pc_ExtRegistration->s_Version.e_Type )
	            || ( TYPE_ExtTypeBrowserGeneral == pc_ExtRegistration->s_Version.e_Type )	)
	    && ( 0 != option ) ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	} else if( ( True == onlyPanel.bl_IWSAction ) && ( BTNID_GoToUtilityToCounter != option ) && ( 0 != option ) ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	} else {
		PMessageArea::procUtility( option );
// comment 2019/06/13 Sky東島 [EagleHigh][OP_BTS-7727].
		if( ( NULL != nextGamen )
		 && ( True == AplCommonSet.isActiveGeneralApplication() )
		 && ( True == c_ExtPanel.isEnableMFPFunction() ) ){
			// 設定メニュー遷移後、起動中アプリに戻ることができないためアプリ切断する.
			AplCommonSet.notifyDisconnect( False );
// comment 2019/07/13 SCD2-Sky小畑 [EagleH][OP_BTS-7300] 設定メニュー遷移前BrowserModeをクリアするように対応.
			onlyPanel.setPrevUtilityBrowserMode( 0 );
		}
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： デフォルト背景表示部品の設定.
//================================================================================================
void PCGC_HomeTopMenu::setDefaultBackParts()
{
	TYPE_TopMenuCustomPattern e_TopMenuCustomPattern = PnlComGetSystemSetting()->getTopMenuCustomPattern();
	if( ( TYPE_TopMenuCustomPattern1 == e_TopMenuCustomPattern )
	    || ( TYPE_TopMenuCustomPattern2 == e_TopMenuCustomPattern )
	    || ( TYPE_TopMenuCustomPattern3 == e_TopMenuCustomPattern ) ) {
		PNLC_UICustomDataManager* pc_DataMan = PNLC_UICustomDataManager::newInstance();
		if( ( NULL != pc_DataMan )
		    && ( NULL == pc_DataMan->getTopMenuThemeImage() ) ) {
			// ユーザーテーマ用の画像領域が確保されていない場合.
			Bak_HomeTopMenuUserThemeBackground.changeGraphic( c71_000_000_t1_X800_Y480 );
		}
	}

	switch( e_TopMenuCustomPattern ) {
		case TYPE_TopMenuCustomPattern1:
			// ショートカットにEcoが割付されているかチェック.
			checkExistEcoFuncShortcut();
			if( True == m_bl_IsExistEcoFuncShortcut ) {
				setDefaultBackPartsInfo( ( PDisplayParts** )tbl_HomeTopMenuEcoDefault, ss_HomeTopMenuEcoDefault );
			} else {
				setDefaultBackPartsInfo( ( PDisplayParts** )tbl_HomeTopMenuTheme1Default, ss_HomeTopMenuTheme1Default );
			}
			break;
		case TYPE_TopMenuCustomPattern2:
			setDefaultBackPartsInfo( ( PDisplayParts** )tbl_HomeTopMenuTheme2Default, ss_HomeTopMenuTheme2Default );
			break;
		case TYPE_TopMenuCustomPattern3:
			setDefaultBackPartsInfo( ( PDisplayParts** )tbl_HomeTopMenuTheme3Default, ss_HomeTopMenuTheme3Default );
			break;
		case TYPE_TopMenuUserCustomPattern1:	// ユーザーテーマが選択されている場合 一度デフォルト背景を設定.
		case TYPE_TopMenuUserCustomPattern2:
		case TYPE_TopMenuUserCustomPattern3:
			Bak_HomeTopMenuUserThemeBackground.changeGraphic( c71_000_000_t1_X800_Y480 );
			setDefaultBackPartsInfo( ( PDisplayParts** )tbl_HomeTopMenuUserThemeDefault, ss_HomeTopMenuUserThemeDefault );
			break;
		default:	break;
	}
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： デフォルト背景表示部品の設定.
//================================================================================================
Bool PCGC_HomeTopMenu::getShowTopMenu( )
{
	return( m_b_ShowTopMenu );
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： デフォルト背景表示部品の設定.
//================================================================================================
void PCGC_HomeTopMenu::setShowTopMenu( Bool b_ShowTopMenu )
{
	m_b_ShowTopMenu = b_ShowTopMenu;
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： バッジキーに表示する情報の設定.
//================================================================================================
#define PNLD_MASKBIT_UNDER8				0x00FF
#define PNLD_MASKBIT_UPPER8				0xFF00
#define PNLD_TOPMENU_SHORTCUTICON_NORMAL_MAX	24	// 1,2ページ目の表示ＭＡＸ数.
// Comment 2016/10/07 [HCLT_Sri charan] [1005414] 【IT5_3.3】 Addition of part for 10.1 inch screen.
#ifdef DEF_RES_WSVGA
static PNLS_Parts puc_ProgramIcon[] = {
	{ i76_001_000_n_X81_Y100, PNLD_PaletteTopMenuBaseIcon2 },
	{ i76_002_000_n_X81_Y100, PNLD_PaletteTopMenuBaseIcon2 },
	{ i76_003_000_n_X81_Y100, PNLD_PaletteTopMenuBaseIcon2 },
	{ i76_004_000_n_X81_Y100, PNLD_PaletteTopMenuBaseIcon2 },
	{ i76_005_000_n_X81_Y100, PNLD_PaletteTopMenuBaseIcon2 },
	{ i76_006_000_n_X81_Y100, PNLD_PaletteTopMenuBaseIcon2 },
	{ i76_007_000_n_X81_Y100, PNLD_PaletteCommon },
	{ i76_008_000_n_X81_Y100, PNLD_PaletteCommon },
	{ i76_009_000_n_X81_Y100, PNLD_PaletteCommon },
	{ i76_010_000_n_X81_Y100, PNLD_PaletteCommon },
	{ i76_011_000_n_X81_Y100, PNLD_PaletteCommon },
	{ i76_012_000_n_X81_Y100, PNLD_PaletteCommon },
	{ i76_013_000_n_X81_Y100, PNLD_PaletteCommon },
	{ i76_014_000_n_X81_Y100, PNLD_PaletteCommon },
	{ i76_015_000_n_X81_Y100, PNLD_PaletteCommon },
	{ i76_016_000_n_X81_Y100, PNLD_PaletteCommon },
	{ i76_017_000_n_X81_Y100, PNLD_PaletteCommon },
	{ i76_018_000_n_X81_Y100, PNLD_PaletteCommon },
	{ i76_019_000_n_X81_Y100, PNLD_PaletteCommon },
	{ i76_020_000_n_X81_Y100, PNLD_PaletteCommon },
	{ i76_021_000_n_X81_Y100, PNLD_PaletteCommon },
	{ i76_022_000_n_X81_Y100, PNLD_PaletteCommon },
	{ i76_023_000_n_X81_Y100, PNLD_PaletteCommon },
	{ i76_024_000_n_X81_Y100, PNLD_PaletteCommon },
	{ i76_001_000_d_X81_Y100, PNLD_PaletteCommon },
	{ i71_002_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_003_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_004_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_005_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_006_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i76_007_000_d_X81_Y100, PNLD_PaletteCommon },
	{ i71_008_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i76_009_000_d_X81_Y100, PNLD_PaletteCommon },
	{ i76_010_000_d_X81_Y100, PNLD_PaletteCommon },
	{ i76_011_000_d_X81_Y100, PNLD_PaletteCommon },
	{ i76_012_000_d_X81_Y100, PNLD_PaletteCommon },
	{ i71_013_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i76_014_000_d_X81_Y100, PNLD_PaletteCommon },
	{ i76_015_000_d_X81_Y100, PNLD_PaletteCommon },
	{ i76_016_000_d_X81_Y100, PNLD_PaletteCommon },
	{ i76_017_000_d_X81_Y100, PNLD_PaletteCommon },
	{ i76_018_000_d_X81_Y100, PNLD_PaletteCommon },
	{ i76_019_000_d_X81_Y100, PNLD_PaletteCommon },
	{ i76_020_000_d_X81_Y100, PNLD_PaletteCommon },
	{ i76_021_000_d_X81_Y100, PNLD_PaletteCommon },
	{ i76_022_000_d_X81_Y100, PNLD_PaletteCommon },
	{ i76_023_000_d_X81_Y100, PNLD_PaletteCommon },
	{ i76_024_000_d_X81_Y100, PNLD_PaletteCommon },
};
#else // #ifdef DEF_RES_WSVGA.
static PNLS_Parts puc_ProgramIcon[] = {
	{ i71_001_000_n_X64_Y80, PNLD_PaletteTopMenuBaseIcon2 },
	{ i71_002_000_n_X64_Y80, PNLD_PaletteTopMenuBaseIcon2 },
	{ i71_003_000_n_X64_Y80, PNLD_PaletteTopMenuBaseIcon2 },
	{ i71_004_000_n_X64_Y80, PNLD_PaletteTopMenuBaseIcon2 },
	{ i71_005_000_n_X64_Y80, PNLD_PaletteTopMenuBaseIcon2 },
	{ i71_006_000_n_X64_Y80, PNLD_PaletteTopMenuBaseIcon2 },
	{ i71_007_000_n_X64_Y80, PNLD_PaletteCommon },
	{ i71_008_000_n_X64_Y80, PNLD_PaletteCommon },
	{ i71_009_000_n_X64_Y80, PNLD_PaletteCommon },
	{ i71_010_000_n_X64_Y80, PNLD_PaletteCommon },
	{ i71_011_000_n_X64_Y80, PNLD_PaletteCommon },
	{ i71_012_000_n_X64_Y80, PNLD_PaletteCommon },
	{ i71_013_000_n_X64_Y80, PNLD_PaletteCommon },
	{ i71_014_000_n_X64_Y80, PNLD_PaletteCommon },
	{ i71_015_000_n_X64_Y80, PNLD_PaletteCommon },
	{ i71_016_000_n_X64_Y80, PNLD_PaletteCommon },
	{ i71_017_000_n_X64_Y80, PNLD_PaletteCommon },
	{ i71_018_000_n_X64_Y80, PNLD_PaletteCommon },
	{ i71_019_000_n_X64_Y80, PNLD_PaletteCommon },
	{ i71_020_000_n_X64_Y80, PNLD_PaletteCommon },
	{ i71_021_000_n_X64_Y80, PNLD_PaletteCommon },
	{ i71_022_000_n_X64_Y80, PNLD_PaletteCommon },
	{ i71_023_000_n_X64_Y80, PNLD_PaletteCommon },
	{ i71_024_000_n_X64_Y80, PNLD_PaletteCommon },
	{ i71_001_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_002_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_003_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_004_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_005_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_006_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_007_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_008_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_009_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_010_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_011_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_012_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_013_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_014_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_015_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_016_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_017_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_018_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_019_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_020_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_021_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_022_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_023_000_d_X64_Y80, PNLD_PaletteCommon },
	{ i71_024_000_d_X64_Y80, PNLD_PaletteCommon },
};
#endif // #ifdef DEF_RES_WSVGA.
void PCGC_HomeTopMenu::setBadgeKeyData()
{
// comment 2017/11/21(火) SCD2-Sky長谷川(伸) [FQ-OPSD-1700523-OFP264].
	pc_ProgramKeyDataGetForTopMenu.setNeedProgramSearchFlg( False );
	TYPS_ExtShortCutList _st_BadgeKeyList;
	memset( &_st_BadgeKeyList, 0, sizeof( _st_BadgeKeyList ) );
// comment 2019/19/02 【HCLT_Rajesh】【OP_BTS-2291】 pass the "TYPE_NetworkSpecific_NW1" to avoid back trace.
//Comment 2017/09/04 [HCL-Suribabu][IT5_4.2][RQ-686][WP763][詳細制御仕様書_管理No.33] main menu default settings should be support for Network2.
//	PnlComGetStandardPanelSetting()->getShortCutListSetting( &_st_BadgeKeyList );
//	PnlComGetStandardPanelSetting()->getShortCutListSetting( &_st_BadgeKeyList, PNLC_NetworkLan::getInstance()->getLine() );
// comment 2019/05/28 [HCL-Naveena][EagleBK] NetworkType Default selection activity.
//	TYPE_NetworkSpecific e_NetworkSpecific = PNLC_NetworkLan::getInstance()->getLine();
	TYPE_NetworkSpecific e_NetworkSpecific = PNLC_NetworkLan::getInstance()->getLineForPanel();
	if ( e_NetworkSpecific == TYPE_NetworkSpecific_NONE ) {
		e_NetworkSpecific = TYPE_NetworkSpecific_NW1;		
	}
	PnlComGetStandardPanelSetting()->getShortCutListSetting( &_st_BadgeKeyList,  e_NetworkSpecific);
	memset( &st_BadgeKeyInfo, 0x00, sizeof( st_BadgeKeyInfo ) );

// comment 2015/12/17 OPS1-MSE田村 [ZeusS_ZX0][CQ322235] OAPアプリ番号管理配列を初期化.
	memset( &us_OAPAppNo, 0, sizeof( us_OAPAppNo ) );

	uchar _uc_index = 0;
	uc_maxPage = 1;
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	if( NULL == pc_SoftDipSw ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	for( uchar i = 0; i < PNLD_TOPMENU_SHORTCUTICON_MAX; i++ ) {
//comment 2023/04/20 FPT.HungDM17 【IT6_2.3】【SparrowSFP】【OP_BTS-42958】 クラシックスタイル画面に不要なアイコンが表示される問題対策.
		if ( ( ( NULL != PnlComGetSystemInfo() ) 
			&& ( TYPE_MachineTypeSparrow1SFP == PnlComGetSystemInfo()->getMachineType()
			|| TYPE_MachineTypeSparrow2SFP == PnlComGetSystemInfo()->getMachineType()
			|| TYPE_MachineTypeSparrowBK0SFP == PnlComGetSystemInfo()->getMachineType() ) )
			&& ( 11 != i ) ) {
			continue;
		}
		st_BadgeKeyInfo[i].bl_isShow = True;
		st_BadgeKeyInfo[i].bl_isActivate = True;
		//アイコン表示
		st_BadgeKeyInfo[i].bl_isShowIcon = True;
		if( ( ( 11 == i )
		      && ( False == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW165, 0 ) ) )	// DipSWNo.165 bit0がOFFの時は、1ページ目.
		    || ( ( TYPD_ExtShortcutMaxTopMenu == i )
		         && ( True == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW165, 0 ) ) ) ) {	// DipSWNo.165 bit0がONの時は、2ページ目(最終).
			st_BadgeKeyInfo[i].e_ShortCutKind = PNLE_TopMenuShortCut_Utility;
			st_BadgeKeyInfo[i].tid_FuncName = s_pKEYActivitySettingsMenu_W100H042E;
			st_BadgeKeyInfo[i].puc_FuncIcon = ( uchar* )i70_023_000_X64_Y80;
			st_BadgeKeyInfo[i].uc_IconPaletteType = PNLD_PaletteCommon;
//Comment 2016/10/07 【HCLT-Tharun】【IT5_3.3】【Accurev ID:1005414】Changed the PNG for 10.1 Inch Models.
#ifdef DEF_RES_WSVGA
			st_BadgeKeyInfo[i].puc_IconNotActivate = ( uchar* )i75_023_000_d_X81_Y100;
#else	// #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[i].puc_IconNotActivate = ( uchar* )i70_023_000_d_X64_Y80;
#endif	// #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[i].uc_IconPaletteNotActivate = PNLD_PaletteCommon;
			_uc_index = 1;
//comment 2023/04/20 FPT.HungDM17 【IT6_2.3】【SparrowSFP】【OP_BTS-42958】 クラシックスタイル画面に不要なアイコンが表示される問題対策.
//#if 1	// comment 2018/06/05 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 新UIカウンタ画面正式対応時に削除 Start.
// comment 2018/04/13 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
//		}else if( (10 == i) && ((TYPE_MachineTypeSparrow1SFP == PnlComGetSystemInfo()->getMachineType()) || (TYPE_MachineTypeSparrow2SFP == PnlComGetSystemInfo()->getMachineType())) ){ //10番目のキーにLegacyStyleキーを割り当て
//				st_BadgeKeyInfo[i].e_ShortCutKind = PNLE_TopMenuShortCut_UIMode;
//				st_BadgeKeyInfo[i].tid_FuncName = s_pKEYBasicStyle_W100H042E;
//				st_BadgeKeyInfo[i].puc_FuncIcon = (uchar*)i70_049_000_n_X64_Y80;
//				st_BadgeKeyInfo[i].uc_IconPaletteType = PNLD_PaletteCommon;
//				st_BadgeKeyInfo[i].puc_IconNotActivate = (uchar*)i70_049_000_d_X64_Y80;
//				st_BadgeKeyInfo[i].uc_IconPaletteNotActivate = PNLD_PaletteCommon;
//				st_BadgeKeyInfo[i].bl_isShow = True;
//#endif	// comment 2018/05/24 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 新UIカウンタ画面正式対応時に削除  End.
		} else if( 4 == i		// 5番目.
		           && True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( 193 , 0 ) ) {	// DipSWNo.193 bit0がON.
			st_BadgeKeyInfo[i].e_ShortCutKind = PNLE_TopMenuShortCut_ScanFax;
			st_BadgeKeyInfo[i].tid_FuncName = s_pKEYActivityQRCodeRead_W100H042E;
			st_BadgeKeyInfo[i].puc_FuncIcon = ( uchar* )i70_033_000_X64_Y80;
			st_BadgeKeyInfo[i].uc_IconPaletteType = PNLD_PaletteCommon;
			st_BadgeKeyInfo[i].puc_IconNotActivate = ( uchar* )i70_033_000_X64_Y80;
			st_BadgeKeyInfo[i].uc_IconPaletteNotActivate = PNLD_PaletteCommon;
// comment 2015/09/01 OPS1-Sky稲井[特ROM][MZV-104_機種マージ]認証の無い環境下での操作者自身宛スキャンの実現要望対応.
	   } else if( 7 == i			// 8番目.
	   			&& True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW226 , 2 ) ) {	// DipSWNo.226 bit2がON.
			st_BadgeKeyInfo[i].e_ShortCutKind = PNLE_TopMenuShortCut_ScanFax;
			st_BadgeKeyInfo[i].tid_FuncName = s_pKEYActivityScanFaxMemory_W100H042E;
// Comment 2016/10/07 [HCLT_Sri charan] [1005414] 【IT5_3.3】 Addition of part for 10.1 inch screen.
#ifdef DEF_RES_WSVGA
			st_BadgeKeyInfo[i].puc_FuncIcon = (uchar*)i76_004_000_n_X81_Y100;
#else // #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[i].puc_FuncIcon = (uchar*)i71_004_000_n_X64_Y80;
#endif // #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[i].uc_IconPaletteType = PNLD_PaletteTopMenuBaseIcon2;
			st_BadgeKeyInfo[i].puc_IconNotActivate = (uchar*)i71_004_000_d_X64_Y80;
			st_BadgeKeyInfo[i].uc_IconPaletteNotActivate = PNLD_PaletteCommon;
		} else {
			TYPE_TopMenuShortCutType				_e_ShortCutType ;
			TYPE_TopMenuShortCutFunction			_e_FunctionType;
			TYPE_TopMenuShortCutCopyFunction		_e_CopyFunctionType;
			TYPE_TopMenuShortCutScanFaxFunction		_e_ScanFaxFunctionType;
			TYPE_TopMenuShortCutSystemBoxFunction	_e_SystemBoxFunctionType;
			TYPE_TopMenuShortCutGroupKey			_e_TopMenuShortCutGroupKey=0;
			char									sc_OpenAPIApplicationNameUTF8[TYPD_ExtMaxAplName+1];
			unsigned short							us_ApplicationButtonShortCutIcon;
			short									_ss_CopyProgramNo;
			unsigned short							_us_CopyProgramIcon;
			short									_ss_ScanFaxProgramNo;
			unsigned short							_us_ScanFaxProgramIcon;
			TYPE_TopMenuShortCutEco					e_TopMenuShortCutEco;
			if( ( 24 == i ) || ( 25 == i ) ) {	// 3ページ目の表示の際はリスト内の別の配列を参照する.
				_e_ShortCutType = _st_BadgeKeyList.s_ShortCutKeyAlwaysShow[i-TYPD_ExtShortcutMaxTopMenu-1].e_ShortCutType;
				_e_FunctionType = _st_BadgeKeyList.s_ShortCutKeyAlwaysShow[i-TYPD_ExtShortcutMaxTopMenu-1].e_FunctionType;
				_e_CopyFunctionType = _st_BadgeKeyList.s_ShortCutKeyAlwaysShow[i-TYPD_ExtShortcutMaxTopMenu-1].e_CopyFunctionType;
				_e_ScanFaxFunctionType = _st_BadgeKeyList.s_ShortCutKeyAlwaysShow[i-TYPD_ExtShortcutMaxTopMenu-1].e_ScanFaxFunctionType;
				_e_SystemBoxFunctionType = _st_BadgeKeyList.s_ShortCutKeyAlwaysShow[i-TYPD_ExtShortcutMaxTopMenu-1].e_SystemBoxFunctionType;
				strncpy( sc_OpenAPIApplicationNameUTF8, _st_BadgeKeyList.s_ShortCutKeyAlwaysShow[i-TYPD_ExtShortcutMaxTopMenu-1].sc_OpenAPIApplicationNameUTF8, sizeof( sc_OpenAPIApplicationNameUTF8 )-1 );
				us_ApplicationButtonShortCutIcon =  _st_BadgeKeyList.s_ShortCutKeyAlwaysShow[i-TYPD_ExtShortcutMaxTopMenu-1].us_ApplicationButtonShortCutIcon;
				_ss_CopyProgramNo = _st_BadgeKeyList.s_ShortCutKeyAlwaysShow[i-TYPD_ExtShortcutMaxTopMenu-1].ss_CopyProgramMemoryNo;
				_us_CopyProgramIcon = _st_BadgeKeyList.s_ShortCutKeyAlwaysShow[i-TYPD_ExtShortcutMaxTopMenu-1].us_CopyProgramMemoryShortCutIcon;
				_ss_ScanFaxProgramNo = _st_BadgeKeyList.s_ShortCutKeyAlwaysShow[i-TYPD_ExtShortcutMaxTopMenu-1].ss_ScanFaxProgramMemoryNo;
				_us_ScanFaxProgramIcon = _st_BadgeKeyList.s_ShortCutKeyAlwaysShow[i-TYPD_ExtShortcutMaxTopMenu-1].us_ScanFaxProgramMemoryShortCutIcon;
				e_TopMenuShortCutEco = _st_BadgeKeyList.s_ShortCutKeyAlwaysShow[i-TYPD_ExtShortcutMaxTopMenu-1].e_EcoKeyType;
			} else {
				_e_ShortCutType = _st_BadgeKeyList.s_ShortCutKey[i-_uc_index].e_ShortCutType;
				_e_FunctionType = _st_BadgeKeyList.s_ShortCutKey[i-_uc_index].e_FunctionType;
				_e_CopyFunctionType = _st_BadgeKeyList.s_ShortCutKey[i-_uc_index].e_CopyFunctionType;
				_e_ScanFaxFunctionType = _st_BadgeKeyList.s_ShortCutKey[i-_uc_index].e_ScanFaxFunctionType;
				_e_SystemBoxFunctionType = _st_BadgeKeyList.s_ShortCutKey[i-_uc_index].e_SystemBoxFunctionType;
				_e_TopMenuShortCutGroupKey = _st_BadgeKeyList.s_ShortCutKey[i-_uc_index].e_GroupKeyType;
				strncpy( sc_OpenAPIApplicationNameUTF8, _st_BadgeKeyList.s_ShortCutKey[i-_uc_index].sc_OpenAPIApplicationNameUTF8, sizeof( sc_OpenAPIApplicationNameUTF8 )-1 );
				us_ApplicationButtonShortCutIcon =  _st_BadgeKeyList.s_ShortCutKey[i-_uc_index].us_ApplicationButtonShortCutIcon;
				_ss_CopyProgramNo = _st_BadgeKeyList.s_ShortCutKey[i-_uc_index].ss_CopyProgramMemoryNo;
				_us_CopyProgramIcon = _st_BadgeKeyList.s_ShortCutKey[i-_uc_index].us_CopyProgramMemoryShortCutIcon;
				_ss_ScanFaxProgramNo = _st_BadgeKeyList.s_ShortCutKey[i-_uc_index].ss_ScanFaxProgramMemoryNo;
				_us_ScanFaxProgramIcon = _st_BadgeKeyList.s_ShortCutKey[i-_uc_index].us_ScanFaxProgramMemoryShortCutIcon;
				e_TopMenuShortCutEco = _st_BadgeKeyList.s_ShortCutKey[i-_uc_index].e_EcoKeyType;
			}
			if( ( PCGE_HuronMyPanel == PnlComGetMyPanelType() ) ||
			    ( PCGE_OapMyPanel == PnlComGetMyPanelType() ) ) {		// OapMyパネルとHuronパネルの場合.
				if( ( NULL != PnlComGetUser() ) && ( TYPE_UserTypeUser == PnlComGetUser()->getUserType() ) ) {		// 一般ユーザーでログイン時のみ.
					TYPC_TopMenuShortCutKeyFunction _c_ShortCutKey = PnlComGetMyPanelSetting()->getTopMenuShortCutKeyFunction( static_cast<ushort>( i-_uc_index ) );
//Comment 2020/07/08 [HCL_Monisha][IT6_LocalUI] Panel Log Addition Activity.
					PanelLog( PNLD_LogKind_Active, "ShortCutKey[e_ShortCutType]:[%d]",_c_ShortCutKey.e_TopMenuShortCutType, 0, 0, 0, 0, 0 );
					_e_ShortCutType = _c_ShortCutKey.e_TopMenuShortCutType;
					_e_FunctionType = _c_ShortCutKey.u_ShortCutFunctionNo.e_TopMenuShortCutFunction;
					_e_CopyFunctionType = _c_ShortCutKey.u_ShortCutFunctionNo.e_TopMenuShortCutCopyFunction;
					_e_ScanFaxFunctionType = _c_ShortCutKey.u_ShortCutFunctionNo.e_TopMenuShortCutScanFaxFunction;
					_e_SystemBoxFunctionType = _c_ShortCutKey.u_ShortCutFunctionNo.e_TopMenuShortCutSystemBoxFunction;
					_ss_CopyProgramNo = _c_ShortCutKey.u_ShortCutFunctionNo.ss_CopyMemoryNo;
					_ss_ScanFaxProgramNo = _c_ShortCutKey.u_ShortCutFunctionNo.ss_ScanFaxMemoryNo;
					_e_TopMenuShortCutGroupKey = _c_ShortCutKey.u_ShortCutFunctionNo.e_TopMenuShortCutGroupKey;
					//値をマスクでとるように変更
					_us_CopyProgramIcon = ( PnlComGetMyPanelSetting()->getTopMenuShortCutKeyIcon( static_cast<ushort>( i-_uc_index ) ) & PNLD_MASKBIT_UNDER8 );
					e_TopMenuShortCutEco = _c_ShortCutKey.u_ShortCutFunctionNo.e_TopMenuShortCutEco;
					if( ( ( sizeof( puc_ProgramIcon ) / sizeof( PNLS_Parts ) ) - PNLD_TopMenuIconTypeMax +1 ) <= _us_CopyProgramIcon ) {
						_us_CopyProgramIcon = 0;
						LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
					}
					if( PCGE_OapMyPanel == PnlComGetMyPanelType() )
					{
						if( ( ( _e_ShortCutType == TYPE_TopMenuShortCutTypeCopyMemory ) ||
						      ( _e_ShortCutType == TYPE_TopMenuShortCutTypeScanFaxMemory ) ||
						      ( _e_ShortCutType == TYPE_TopMenuShortCutTypeAppListShortCut ) ) &&
						    ( 0 == _us_CopyProgramIcon ) )
						{
							st_BadgeKeyInfo[i].bl_isShowIcon = False;
						}
					}
					us_ApplicationButtonShortCutIcon = _us_CopyProgramIcon;
					_us_ScanFaxProgramIcon = _us_CopyProgramIcon;
					memset( sc_OpenAPIApplicationNameUTF8, 0, sizeof( sc_OpenAPIApplicationNameUTF8 ) );
					PnlComGetMyPanelSetting()->getOpenAPIApplicationName( static_cast<ushort>( i-_uc_index ), sc_OpenAPIApplicationNameUTF8 );
					e_TopMenuShortCutEco = _c_ShortCutKey.u_ShortCutFunctionNo.e_TopMenuShortCutEco;
				}
			}
			else {
				// MyPanelで無い場合.
// comment 2019/19/02 【HCLT_Rajesh】【OP_BTS-2291】 pass the "TYPE_NetworkSpecific_NW1" to avoid back trace.
// comment 2017/11/23 【HCL-Suganya】【IT5_4.2】【RQ686】 Multiple Network Support.
//				_us_CopyProgramIcon = PnlComGetStandardPanelSetting()->getTopMenuShortCutKeyIcon( static_cast<ushort>( i-_uc_index ) );
//				_us_CopyProgramIcon = PnlComGetStandardPanelSetting()->getTopMenuShortCutKeyIcon( static_cast<ushort>( i-_uc_index ), PNLC_NetworkLan::getInstance()->getLine() );
				_us_CopyProgramIcon = PnlComGetStandardPanelSetting()->getTopMenuShortCutKeyIcon( static_cast<ushort>( i-_uc_index ), e_NetworkSpecific );
				if( 0 != ( PNLD_MASKBIT_UPPER8 & _us_CopyProgramIcon ) ) {
					_us_CopyProgramIcon = _us_CopyProgramIcon & PNLD_MASKBIT_UNDER8;	//4096以上の場合、上位をﾏｽｸする.
				}

				if( PNLD_TOPMENU_SHORTCUTICON_NORMAL_MAX < _us_CopyProgramIcon ) {
					_us_CopyProgramIcon = 0;
				}
				if( ( ( _e_ShortCutType == TYPE_TopMenuShortCutTypeCopyMemory ) ||
				      ( _e_ShortCutType == TYPE_TopMenuShortCutTypeScanFaxMemory ) ||
				      ( _e_ShortCutType == TYPE_TopMenuShortCutTypeAppListShortCut ) ) &&
				    ( 0 == _us_CopyProgramIcon ) ) {
					st_BadgeKeyInfo[i].bl_isShowIcon = False;	//0の場合、アイコン表示をFalse.
				}
				us_ApplicationButtonShortCutIcon = _us_CopyProgramIcon;
				_us_ScanFaxProgramIcon = _us_CopyProgramIcon;
			}
			if( ( TYPE_TopMenuShortCutTypeCopyMemory == _e_ShortCutType			// 割り当てられた機能がコピープログラム.
			      || TYPE_TopMenuShortCutTypeScanFaxMemory == _e_ShortCutType )	// 割り当てられた機能がファクス/スキャンプログラム.
			    && ( ( True == PLBC_FuncVersionCheck::isOK_TypeMLK() ) && ( False == PnlComIsEnableFunction( PNLE_EnableFunctionType_Program ) ) ) ) {	// プログラム機能が使用できない.
				st_BadgeKeyInfo[i].bl_isShow = False;	// キーは非表示.
			} else {
				switch( _e_ShortCutType ) {
					case TYPE_TopMenuShortCutTypeNone:				// 未設定.
						st_BadgeKeyInfo[i].e_ShortCutKind = PNLE_TopMenuShortCut_None;
						st_BadgeKeyInfo[i].bl_isShow = False;
						break;
					case TYPE_TopMenuShortCutTypeFunction:			// 機能.
						setFunctionInfo( i, _e_FunctionType );							// 機能情報の設定.
						break;
					case TYPE_TopMenuShortCutTypeCopyFunction:		// コピー機能設定.
						setCopyFunctionInfo( i, _e_CopyFunctionType );					// コピー機能情報の設定.
						break;
					case TYPE_TopMenuShortCutTypeScanFaxFunction:	// スキャン/ファクス機能設定.
						setScanFaxFunctionInfo( i, _e_ScanFaxFunctionType );			// スキャン/ファクス機能情報の設定.
						break;
					case TYPE_TopMenuShortCutTypeCopyMemory:		// コピー設定メモリ.
						setCopyMemoryInfo( i, _ss_CopyProgramNo );						// コピー設定メモリ情報の設定.
						if( 0 < _us_CopyProgramIcon )				{ _us_CopyProgramIcon--; }
// Comment 2018/05/30 HCL-Nandhini [DenebMLK] 制御構成変更_No.68  [WP-472] - ID card copy in Home Top Menu.					
						if ( ( True == PnlComGetSoftDipSw()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW153, 0 ) )	//Dip SW 153 bit 0 - set.
							  && ( TYPD_MaxJobMemory == _ss_CopyProgramNo ) )  {									// Copy Program No. - Max (30).
							st_BadgeKeyInfo[i].puc_FuncIcon					= ( uchar* )i70_044_000_n_X64_Y80;
							st_BadgeKeyInfo[i].uc_IconPaletteType			= PNLD_PaletteCommon;
// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1】Name changing from ID card copy to Copy card for DenebMLK model.
							st_BadgeKeyInfo[i].tid_FuncName					= s_ITMCardCopy;
							st_BadgeKeyInfo[i].puc_IconNotActivate			= ( uchar* )i70_044_000_d_X64_Y80;
							st_BadgeKeyInfo[i].uc_IconPaletteNotActivate	= PNLD_PaletteCommon;
//Comment 2018/06/25 HCL-Nandhini [DenebMLK] 制御構成変更_No.68 [WP-476] - ID card copy
							st_BadgeKeyInfo[i].bl_isShowIcon = True;
						} else {
							st_BadgeKeyInfo[i].puc_FuncIcon = ( uchar* )puc_ProgramIcon[_us_CopyProgramIcon].puc_Parts;
							st_BadgeKeyInfo[i].uc_IconPaletteType = puc_ProgramIcon[_us_CopyProgramIcon].uc_PaletteType;
							st_BadgeKeyInfo[i].puc_IconNotActivate = ( uchar* )puc_ProgramIcon[PNLD_TopMenuIconTypeMax+_us_CopyProgramIcon].puc_Parts;
							st_BadgeKeyInfo[i].uc_IconPaletteNotActivate = puc_ProgramIcon[PNLD_TopMenuIconTypeMax+_us_CopyProgramIcon].uc_PaletteType;
						}
//Comment 2018/06/25 HCL-Nandhini [DenebMLK] 制御構成変更_No.68 [WP-476] - ID card copy
						st_BadgeKeyInfo[i].ss_ProgramNo = _ss_CopyProgramNo;
						break;
					case TYPE_TopMenuShortCutTypeScanFaxMemory:		// スキャン/ファクス設定メモリ.
						setScanFaxMemoryInfo( i, _ss_ScanFaxProgramNo );				// スキャン/ファクス設定メモリ情報の設定.
						if( 0 < _us_ScanFaxProgramIcon )			{ _us_ScanFaxProgramIcon--; }
						st_BadgeKeyInfo[i].puc_FuncIcon = ( uchar* )puc_ProgramIcon[_us_ScanFaxProgramIcon].puc_Parts;
						st_BadgeKeyInfo[i].uc_IconPaletteType = puc_ProgramIcon[_us_ScanFaxProgramIcon].uc_PaletteType;
						st_BadgeKeyInfo[i].puc_IconNotActivate = ( uchar* )puc_ProgramIcon[PNLD_TopMenuIconTypeMax+_us_ScanFaxProgramIcon].puc_Parts;
						st_BadgeKeyInfo[i].uc_IconPaletteNotActivate = puc_ProgramIcon[PNLD_TopMenuIconTypeMax+_us_ScanFaxProgramIcon].uc_PaletteType;
						st_BadgeKeyInfo[i].ss_ProgramNo = _ss_ScanFaxProgramNo;
						break;
					case TYPE_TopMenuShortCutTypeAppShortCut:		// OpenAPIアプリショートカット.
					case TYPE_TopMenuShortCutTypeAppListShortCut:	// OpenAPIアプリリスト.
// Comment 2018/10/08 [HCL-Pavani] [IT6_Eagle] [AR1060058] MarketPlace Application Key Display/Non-display check.
						{
							// 指定アプリMFP基本機能設定禁止の場合.
							if( False == c_ExtPanel.isEnableMFPFunction() ) {
								st_BadgeKeyInfo[i].bl_isShow = False;
								break;
							}
							if( 0 < us_ApplicationButtonShortCutIcon )	{ us_ApplicationButtonShortCutIcon--; }
							setAppFunctionInfo( i, _e_ShortCutType, sc_OpenAPIApplicationNameUTF8, us_ApplicationButtonShortCutIcon );
							// PKI認証設定時かつセキュリティー強化設定有効の場合、キーを非表示.
							if( ( True == PLBC_FuncPKICheck::isPKI() ) && ( True == PnlComGetSystemSetting()->getSecurityLevelUpOn() ) ) {
								st_BadgeKeyInfo[i].bl_isShow = False;
							}

// commemt 2019/09/03 SCD2-Sky二口 [EagleH][OP_BTS-12799] MarketPlaseがインストールされているのにSetupが表示されてしまう.
							// DataBase初期化未完了の時はAPPキー非表示.
							if ( False == m_bl_isDBInit ) {
								st_BadgeKeyInfo[i].bl_isShow = False;
							}

// Comment 2018/10/08 [HCL-Pavani] [IT6_Eagle] [AR1060058] MarketPlace Application Key Display/Non-display check.
							char c_OapAppName[TYPD_ExtMaxAplName*3+1] = {0,};
							char sc_AppName[TYPD_ExtMaxAplName*3+1] = PNLD_MarketPlaceString;
							PnlComGetUTF8Str( sc_OpenAPIApplicationNameUTF8, ( LIBE_CharSet )getCharSetForCAPI(), c_OapAppName, sizeof( c_OapAppName ) );
// Comment 2019/06/08 [IT6_1.0MR2] AR1077690 [HCLT-Catherine]  New Enum type for Classic screen Market place icon.
//							if ( ( 0 == strcmp( sc_AppName, c_OapAppName ) ) && ( False == PnlComGetIWSSetting()->getMarketPlaceAppUse()) ){
							if ( 0 == strcmp( sc_AppName, c_OapAppName ) ){
// Comment 2019/09/06 [IT6_2.0_EagleZ+] OP_BTS-12580 [HCLT-Santhiya]  Adding Market place icon.
// Comment 2019/09/24 [IT6_2.0_EagleZ+] OP_BTS-12580_Reopen [HCLT-Rajkumar]  Corrected Market place icon.
//								st_BadgeKeyInfo[i].puc_FuncIcon = ( uchar* )i70_023_000_X64_Y80;
//Comment 2019/12/03 [IT6_2.0_EagleZ+] [OP_BTS-7295_Reopen] [HCLT_Vijayakrishna] Marketplace icon enlarge issue start
//								st_BadgeKeyInfo[i].puc_FuncIcon = ( uchar* )i70_051_000_n_X64_Y80;								
//								st_BadgeKeyInfo[i].uc_IconPaletteType = PNLD_PaletteCommon;								
								if( 0xFFFF != us_OAPAppNo[i] ) {
									for( uchar uc_IconListNo = 0; uc_IconListNo < m_us_TotalCountOap; uc_IconListNo++ ) {	// OAPアプリ数回チェックする.
										if( s_registerApp[uc_IconListNo] == ( short )us_OAPAppNo[i] ) {
											const TYPS_ExtIconDetail* sIconDetailBuf = ScanOapExtSet.getIconAppSelect( uc_IconListNo );
											if( sIconDetailBuf && ( sIconDetailBuf->e_Entry != TYPE_ExtEntryOff ) && ScanOapExtSet.getIconCheck( *sIconDetailBuf ) ) {
												st_BadgeKeyInfo[i].puc_FuncIcon = ( uchar* )sIconDetailBuf->s_IconData.sc_Data;
												st_BadgeKeyInfo[i].uc_IconPaletteType = sIconDetailBuf->sc_PaletteNo;
// comment 2020/6/26 Sky東島 [IT6_2.0MR3][制御構成変更_No.37]SVGサポート対応.
// comment 2022/11/16 FPT.ThanhLH11 [IT6_2.3][ALL_MACHINE][OP_BTS-39692]クラシックのトップメニュー画面でMarketPlaceアプリのアイコンは表示しない問題対策.
												st_BadgeKeyInfo[i].us_FuncIconDataSize = sIconDetailBuf->s_IconData.ss_DataSize;
												st_BadgeKeyInfo[i].bl_SvgData = ScanOapExtSet.isExtIconFormatSvg( sIconDetailBuf->e_IconFormat );
											}
										}
									}
								}
//Comment 2019/12/03 [IT6_2.0_EagleZ+] [OP_BTS-7295_Reopen] [HCLT_Vijayakrishna] Marketplace icon enlarge issue End
								st_BadgeKeyInfo[i].e_ShortCutKind = PNLE_TopMenuShortCut_MarketPlaceApplication_Legacy;
								st_BadgeKeyInfo[i].uc_IconPaletteNotActivate = PNLD_PaletteCommon;
								st_BadgeKeyInfo[i].puc_IconNotActivate = (uchar*)i70_051_000_d_X64_Y80;
								if( False == PnlComGetIWSSetting()->getMarketPlaceAppUse()) {
									st_BadgeKeyInfo[i].bl_isShow = False;
								}
							}
							// comment 2018/11/14 SCD2 K.Seto [AR.1067363]表示対象となるアプリがMarketPlaceアプリのみの場合、APPキーを非表示とする.
							PNLC_ActionDataLib* pc_ActionDataLib = PNLC_ActionDataLib::newInstance();
							if ( NULL != pc_ActionDataLib ) {
								// OAPアプリの数をカウントする.
								uchar uc_OAPAppCnt = 0;
								uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeScanD );
								uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeScanS );
								uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypePullPrint );
								uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeBrowserScan );
								uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeBrowserLauncher );
								uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeBrowserPull );
								uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeBrowserGeneral );
								uc_OAPAppCnt += ExtSet.countApps();
								if ( True == pc_ActionDataLib->isInstalledMarketPlaceApp() ) {
									// MarketPlaceアプリはBrowserLauncherでカウントされてしまうため、インストールされている場合は1引いておく.
									uc_OAPAppCnt = uc_OAPAppCnt - 1;
								}
								
								if ( ( True == ScanOapExtSet.isRegistApplication() ) && ( 0 == uc_OAPAppCnt ) && ( True == pc_ActionDataLib->isInstalledMarketPlaceApp() ) ) {
									// OAPアプリは未インストール、かつMarketPlaceアプリがインストール済み.
									// IWSアプリのチェック.
									APIC_IWSAppInfo* pc_IWSAppInfo = APIC_IWSAppInfo::newInstance();
									if ( NULL != pc_IWSAppInfo ) {
										// IWSアプリ数を取得.
										uchar uc_IWSAppCount = ( uchar )pc_IWSAppInfo->getApplicationNum();
										if ( uc_IWSAppCount > 1 ) {
											// アプリ数が1より大きい、即ちMarketPlaceアプリ以外のIWSアプリもインストールされている.
											uchar uc_IwsAppTypeGeneralCnt = 0;
											TYPS_IwsAppInfo s_tempAppInfo;
											memset( &s_tempAppInfo, '\0', sizeof( s_tempAppInfo ) );
											// インストールされているIWSアプリのアプリタイプを取得.
											for( uchar uc_Cnt = 0; uc_Cnt < uc_IWSAppCount; uc_Cnt++ ) {
												int si_Ret = pc_IWSAppInfo->getApplication( (int)( uc_Cnt+1 ), s_tempAppInfo );
												if ( ( TYPD_IwsResult_OK == si_Ret )
												  && ( TYPE_IwsAppTypeGeneral == s_tempAppInfo.e_AppType ) ) {
													// アプリタイプがTYPE_IwsAppTypeGeneralのものは表示対象なのでカウント.
													uc_IwsAppTypeGeneralCnt++;
												}
											}
											if ( 1 == uc_IwsAppTypeGeneralCnt ) {
												// 表示対象のアプリが1、即ちMarketPlaceアプリのみのため、APPキーも表示不要.
												if ( ( 0 == strcmp( sc_AppName, c_OapAppName ) ) && ( False == PnlComGetIWSSetting()->getMarketPlaceAppUse()) ) {
													st_BadgeKeyInfo[i].bl_isShow = False;
												}
											}
										} else {
											// アプリ数が1、即ちMarketPlaceアプリのみのため、APPキーも表示不要.
											if ( ( 0 == strcmp( sc_AppName, c_OapAppName ) ) && ( False == PnlComGetIWSSetting()->getMarketPlaceAppUse()) ) {
												st_BadgeKeyInfo[i].bl_isShow = False;
											}
										}
									}
								}
							}
						}
						break;
					case TYPE_TopMenuShortCutTypeSystemBox:			// システムボックス機能設定.
						setSystemBoxInfo( i, _e_SystemBoxFunctionType );				// システムボックス情報の設定.
						break;
// comment 2015/03/04 OPS1-Sky榎本 [MinervaSBk][UV-01457]トップメニューへアドレス帳キー割付.
					case TYPE_TopMenuShortCutTypeAddressBook:		// アドレス帳キー.
						{
							st_BadgeKeyInfo[i].e_ShortCutKind				= PNLE_TopMenuShortCut_AddressBook;
							st_BadgeKeyInfo[i].tid_FuncName					= s_pKEYAddressBookMenu_W100H042E;
// Comment 2016/10/07 [HCLT_Arvind] [1005414] 【IT5_3.3】 Addition of part for 10.1 inch screen.
#ifdef DEF_RES_WSVGA
							st_BadgeKeyInfo[i].puc_FuncIcon					= (uchar*)i75_040_000_n_X81_Y100;
#else // #ifdef DEF_RES_WSVGA.
							st_BadgeKeyInfo[i].puc_FuncIcon					= (uchar*)i70_040_000_n_X64_Y80;
#endif // #ifdef DEF_RES_WSVGA.
							st_BadgeKeyInfo[i].uc_IconPaletteType			= PNLD_PaletteCommon;
#ifdef DEF_RES_WSVGA
							st_BadgeKeyInfo[i].puc_IconNotActivate			= (uchar*)i75_040_000_d_X81_Y100;
#else // #ifdef DEF_RES_WSVGA.
							st_BadgeKeyInfo[i].puc_IconNotActivate			= (uchar*)i70_040_000_d_X64_Y80;
#endif // #ifdef DEF_RES_WSVGA.
							st_BadgeKeyInfo[i].uc_IconPaletteNotActivate	= PNLD_PaletteCommon;

							Bool bl_Show = True;
// Comment 2017/11/29 【HCLT-Deeksha】【Eagle】【制御構成変更_No.4】 Removing old function version check condition and else portion.
							if ( NULL == PnlComGetUser() ) {												// ユーザ認証ユーザなしの場合.
								bl_Show = False;
// comment 2021/04/30 Sky東島 [IT6_2.2][OP_BTS-32561].
//							} else if ( ( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance()->getLine()) )	// 部門認証管理する.
//									&&	( NULL == PnlComGetTrack() ) ) {									// 部門認証なし.
							} else if ( True == PnlComGetNeedToTrackLogin() ) {									// 部門認証なし.
								bl_Show = False;
							} else {
								bl_Show = True;
							}
							st_BadgeKeyInfo[i].bl_isShow = bl_Show;
						}
						break;
					case TYPE_TopMenuShortCutTypeSoundSetting:		// 音設定.
						st_BadgeKeyInfo[i].e_ShortCutKind = PNLE_TopMenuShortCut_SoundSetting;
						st_BadgeKeyInfo[i].tid_FuncName = s_pKEYActivitySoundSetting_W080H042E;
//Comment 2016/10/07 【HCLT-Tharun】【IT5_3.3】【Accurev ID:1005414】Changed the PNG for 10.1 Inch Models.
#if defined(DEF_RES_WSVGA)
						st_BadgeKeyInfo[i].puc_FuncIcon = ( uchar* )i75_034_000_n_X81_Y100;
#else	// #if defined(DEF_RES_WSVGA).					
						st_BadgeKeyInfo[i].puc_FuncIcon = ( uchar* )i70_034_000_n_X64_Y80;
#endif	// #if defined(DEF_RES_WSVGA).					
						st_BadgeKeyInfo[i].uc_IconPaletteType = PNLD_PaletteCommon;						
//Comment 2016/10/07 【HCLT-Tharun】【IT5_3.3】【Accurev ID:1005414】Changed the PNG for 10.1 Inch Models.
// comment 2018/10/17 SCD2 K.Takei 不具合No.1063034 本体認証の場合、パブリックユーザーでログインし、WebUI画面にユニバーサルとユーティリティボタンがグレーアウトになってしまう.
// Chage 通常イコンを非活性アイコンに修正.
#if defined(DEF_RES_WSVGA)
						st_BadgeKeyInfo[i].puc_IconNotActivate = ( uchar* )i75_034_000_d_X81_Y100;
#else	// #if defined(DEF_RES_WSVGA).
						st_BadgeKeyInfo[i].puc_IconNotActivate = ( uchar* )i70_034_000_d_X64_Y80;
#endif	// #if defined(DEF_RES_WSVGA).

						st_BadgeKeyInfo[i].uc_IconPaletteNotActivate = PNLD_PaletteCommon;
						// スライドパネル装着時は音設定キーを非表示.
// comment 2015/02/06 OPS1-HCL-Vijayendran [Poseidon][CQ.509822] Poseidon/HeliosLでは 音声ICが付かないため非表示条件追加.
//						if ( True == PnlComGetSystemInfo()->isSlidePanel() ) {
//Comment 2019/01/10 [DenebMLK][HCLT_Ramkishor] [AR10171313] Adding compilar switch support for DenebMLK to display the sound settings key in HomeTop Menu.
#ifdef DEF_DenebMLKCus
						if ( True == PnlComGetSystemInfo()->isSlidePanel() ){
#else
						if ( True == PnlComGetSystemInfo()->isSlidePanel() || False == APIC_Music::newInstance()->isMusicSettingExtend() ) {
#endif
							st_BadgeKeyInfo[i].bl_isShow = False;
						}
						break;
					case TYPE_TopMenuShortCutTypeGroupKey:			// グループアプリケーション.
						if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
							setAppGrpInfo( i, _e_TopMenuShortCutGroupKey );		// グループアプリケーション情報の設定.
						} else {
							st_BadgeKeyInfo[i].bl_isShow = False;
						}
						break;
					case TYPE_TopMenuShortCutTypePrintScore:    //  ぷりんと楽譜.
						//  「ぷりんと楽譜」、ブラウザが有効であれば表示.
//comment 2018/04/16 HCLT-Haribhaskar [IT6_Eagle] [RQ-698-2] [WP-3683] Removing WebLicense Management.
//						if( ( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( 164, 1 ) ) &&
//						    ( True == isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_WebBrowser ) ) &&
//						    ( True == PnlComGetBrowserSetting()->getEnableBrowser() ) &&
//// Comment 2017/11/08【HCLT-Gadiya sai】【IT5 4.2】【RQ686】【WP412】Multiple Network support.
////						( TYPE_CategoryState_ENABLE == PnlComGetEnablerSetting()->getFEK_CotegoryState( TYPE_FEKType01 ) ) ) {
//						    ( TYPE_CategoryState_ENABLE == PnlComGetEnablerSetting()->getFEK_CotegoryState( TYPE_FEKType01 ) ) && ( TYPE_NetworkSpecific_NW2 != PNLC_NetworkLan::getInstance()->getLine() ) ) {
						if( ( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( 164, 1 ) ) 
						     && ( True == isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_WebBrowser ) )
						     && ( True == PnlComGetBrowserSetting()->getEnableBrowser() )
						     && ( TYPE_NetworkSpecific_NW2 != PNLC_NetworkLan::getInstance()->getLine() ) ) {
							st_BadgeKeyInfo[i].e_ShortCutKind = PNLE_TopMenuShortCut_PrintMusic;
							st_BadgeKeyInfo[i].tid_FuncName = s_pKEYActivity_PrintMusic_W100H042E;
							st_BadgeKeyInfo[i].puc_FuncIcon = ( uchar* )i70_036_000_n_X64_Y80;
							st_BadgeKeyInfo[i].uc_IconPaletteType = PNLD_PaletteCommon;
							st_BadgeKeyInfo[i].puc_IconNotActivate = ( uchar* )i70_036_000_d_X64_Y80;
							st_BadgeKeyInfo[i].uc_IconPaletteNotActivate = PNLD_PaletteCommon;
						} else {
							st_BadgeKeyInfo[i].bl_isShow = False;
						}
						break;
					case TYPE_TopMenuShortCutTypeSimpleSecuritySetup:				// 簡易セキュリティー設定.
					{
						st_BadgeKeyInfo[i].e_ShortCutKind			 = PNLE_TopMenuShortCut_SimpleSecuritySetup;
						st_BadgeKeyInfo[i].tid_FuncName				 = s_pKEYSimpleSecuritySettings_W100H042E;
						st_BadgeKeyInfo[i].puc_FuncIcon				 = ( uchar* )i01_443_000_X80_Y64;
						st_BadgeKeyInfo[i].uc_IconPaletteType		 = PNLD_PaletteCommon;
						st_BadgeKeyInfo[i].puc_IconNotActivate		 = ( uchar* )i01_443_000_X80_Y64;
						st_BadgeKeyInfo[i].uc_IconPaletteNotActivate = PNLD_PaletteCommon;
					}
					break;
					case TYPE_TopMenuShortCutTypeQRCode:				// QRコード表示.
					{
						if( ( NULL == PnlComGetQRPairingSetting()										// QRコード表示設定が表示しない.
						      || False == PnlComGetQRPairingSetting()->getDisplaySetup() )
						    || ( True == onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCall )		// サービスコール発生時.
						         || True == onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCallSMART )
						         || True == onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCallHEART ) )
						    || ( True == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_AuthExpireDate ) )					// 拡張サーバー認証有効期限切れ警告発生時.
						    || ( True == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_BrowserAuthUnable ) )				// Webブラウザー拡張サーバー認証アプリケーション使用不可警告発生時.
						    || ( True == onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCallFirmwareAbnormal ) )		// Firmware異常警告発生時.
						  ) {
							st_BadgeKeyInfo[i].bl_isShow = False;
						} else {
							st_BadgeKeyInfo[i].e_ShortCutKind			 = PNLE_TopMenuShortCut_QRCode;
							st_BadgeKeyInfo[i].tid_FuncName				 = s_pKEYQRCode_W100H042E;
							st_BadgeKeyInfo[i].puc_FuncIcon				 = ( uchar* )i70_038_000_n_X64_Y80;
							st_BadgeKeyInfo[i].uc_IconPaletteType		 = PNLD_PaletteCommon;
							st_BadgeKeyInfo[i].puc_IconNotActivate		 = ( uchar* )i70_038_000_d_X64_Y80;
							st_BadgeKeyInfo[i].uc_IconPaletteNotActivate = PNLD_PaletteCommon;
						}
					}
					break;
					case TYPE_TopMenuShortCutTypeEco:
						// Eco関連機能.
						setEcoFuncInfo( i, e_TopMenuShortCutEco );
						break;
					case TYPE_TopMenuShortCutTypeGadget:
						// ウィジェット設定.
						if( ( True == PnlComGetOperationSetting()->getGadgetEnable() )
						    && ( True == PLBC_MultiTouchFunction::isEnable() ) ) {
							st_BadgeKeyInfo[i].e_ShortCutKind = PNLE_TopMenuShortCut_GadgetSetting;
// comment 2015/10/07 HCL-Omkar [MinervaSBK][CQ516766] changing the symbol ID.
//							st_BadgeKeyInfo[i].tid_FuncName = s_pKEYGadgetSetting_W108H042E;
						  	st_BadgeKeyInfo[i].tid_FuncName = s_pKEYWidgetSetting_W100H042E;
							st_BadgeKeyInfo[i].puc_FuncIcon = ( uchar* )i01_297_000_n_X64_Y80;
							st_BadgeKeyInfo[i].uc_IconPaletteType = PNLD_PaletteCommon;
							st_BadgeKeyInfo[i].puc_IconNotActivate = ( uchar* )i01_297_000_d_X64_Y80;
							st_BadgeKeyInfo[i].uc_IconPaletteNotActivate = PNLD_PaletteCommon;
							st_BadgeKeyInfo[i].bl_isShow = True;
						} else {
							st_BadgeKeyInfo[i].bl_isShow = False;
						}
						break;
					case TYPE_TopMenuShortCutTypeControllerMenu:
// comment 2014/11/26 OPS1-Sky守川 [ZeusS][制御構成変更_No.25]Webパネル対応.
						// コントローラ詳細.
						if( TYPE_ExternalIFMode1 == PnlComGetSystemInfo()->getExternalIFMode() ) {
							st_BadgeKeyInfo[i].e_ShortCutKind				= PNLE_TopMenuShortCut_ControlMenu;
							st_BadgeKeyInfo[i].tid_FuncName					= s_pKEYActivityControllerMenu_W100H042E;
// comment 2014/11/26 OPS1-Sky守川 [ZeusS][制御構成変更_No.25]Webパネル対応.
							st_BadgeKeyInfo[i].puc_FuncIcon					= (uchar*)i70_042_000_n_X64_Y80;
							st_BadgeKeyInfo[i].uc_IconPaletteType			= PNLD_PaletteCommon;
// comment 2014/11/26 OPS1-Sky守川 [ZeusS][制御構成変更_No.25]Webパネル対応.
							st_BadgeKeyInfo[i].puc_IconNotActivate			= (uchar*)i70_042_000_d_X64_Y80;
							st_BadgeKeyInfo[i].uc_IconPaletteNotActivate	= PNLD_PaletteCommon;
						} else {
							// 非表示にする.
							st_BadgeKeyInfo[i].bl_isShow = False;
						}
						break;
// comment 2015/08/12 OPS1 Sky大濵 [ZeusS_ZX0][RQ-383]表示対象に”MarketPlace download”キーを新規追加する.
					case TYPE_TopMenuShortCutTypeMarketPlaceDownloader:    //  Market Place ダウンローダー.
						// 以下の条件にすべて一致した場合に限り表示.
							// DipSW172が0x00または0x02.
// Comment 2017/11/29 【HCLT-Deeksha】【Eagle】【制御構成変更_No.4】 Removing old function version check condition.
// Comment 2018/04/27 【HCLT-Tarun】【Eagle】【RQ-464】 Adding condition to display MarketPlace.
//						if( ( 0x01 != getSwDipSwitch( DEFSOFTWARESW172 ) ) &&
//							// Webブラウザー使用する.
//						    ( True == PnlComGetBrowserSetting()->getEnableBrowser() ) &&
//							// IWS使用する.
//							( True == PnlComGetIWSSetting()->getEnable() ) {
// Comment 2018/05/04 【HCLT-Prashannaa】【IT6-Sparrow】【RQ-464】 removing condition to display MarketPlace for 2AIO and SFP2.
					{
// Comment 2019/02/21 【HCLT-Naveena】【OP_BTS-2893 】【ID: 151128】[PORTING.CMPSPEC.TYPE.BOOL].
//						bool bl_MarketPlaceApp = False;
						Bool bl_MarketPlaceApp = False;
#ifdef DEF_SparrowAIOSFPCus 
 					if ((PnlComGetSystemInfo()->getMachineType() == TYPE_MachineTypeSparrow2AIO ) || 
  						(PnlComGetSystemInfo()->getMachineType() == TYPE_MachineTypeSparrow2SFP  ) ) {   
							bl_MarketPlaceApp = True;	
						}else{
							bl_MarketPlaceApp = PnlComGetIWSSetting()->getMarketPlaceAppUse();
						}
#else 
// comment 2018/11/12 【HCLT_Tarun】Kloc work Changes ID:141429 [PORTING.CMPSPEC.TYPE.BOOL].						
//					bl_MarketPlaceApp = PnlComGetIWSSetting()->getMarketPlaceAppUse();
					bl_MarketPlaceApp = ( ( 0 == PnlComGetIWSSetting()->getMarketPlaceAppUse() )? false : true ) ;						
#endif
						if( ( True == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW172, 2 ) ) &&
							// Webブラウザー使用する.
							( True == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( GENERICOROWNSWNUM, 2 ) ) &&
							// IWS使用する.
							( True == PnlComGetIWSSetting()->getEnable() ) &&
//							( True == PnlComGetIWSSetting()->getMarketPlaceAppUse())) {
							( True == bl_MarketPlaceApp)) {//Changed to avoid getMarketPlaceAppUse check for Sparrow 2AIO and SFP2
								st_BadgeKeyInfo[i].e_ShortCutKind = PNLE_TopMenuShortCut_MarketPlace;
// comment 2015/10/30 OPS1 Sky稲葉 [ZeusS_ZX0][RQ-383] ワーディング整合結果反映.
//							st_BadgeKeyInfo[i].tid_FuncName = s_pKEYMarketPlaceDownload_W100H042E;
//							st_BadgeKeyInfo[i].puc_FuncIcon = ( uchar* )i70_042_000_n_X64_Y80;			//ショートカットアイコンTBD(ToDo:アイコン決定後に要修正).
//							st_BadgeKeyInfo[i].uc_IconPaletteType = PNLD_PaletteCommon;
//							st_BadgeKeyInfo[i].puc_IconNotActivate = ( uchar* )i70_042_000_d_X64_Y80;	//ショートカットアイコンTBD(ToDo:アイコン決定後に要修正).
//							st_BadgeKeyInfo[i].uc_IconPaletteNotActivate = PNLD_PaletteCommon;
//							st_BadgeKeyInfo[i].bl_isShowIcon = False;									//ショートカットアイコンTBD(ToDo:アイコン決定後に要修正).
// Comment 2017/12/19 [ HCLT-Surya ] [AR1033578][ Change MarketPlace downloader icon ]	
						if ( True == PLBC_FuncVersionCheck::isOK_IT5_Type11() ) {
							st_BadgeKeyInfo[i].tid_FuncName = s_pKEYMarketPlace_W110H042E;
						}
						else {
							st_BadgeKeyInfo[i].tid_FuncName = s_pKEYMarketPlaceDownload_W110H042E;
						}
// Comment 2016/10/07 [HCLT_Arvind] [1005414] 【IT5_3.3. 】Checking for 10.1 inch
#ifdef DEF_RES_WSVGA
							st_BadgeKeyInfo[i].puc_FuncIcon = ( uchar* )i75_043_000_n_X81_Y100;
#else // #ifdef DEF_RES_WSVGA.
							st_BadgeKeyInfo[i].puc_FuncIcon = ( uchar* )i70_043_000_n_X64_Y80;
#endif // #ifdef DEF_RES_WSVGA.
							st_BadgeKeyInfo[i].uc_IconPaletteType = PNLD_PaletteCommon;
// Comment 2016/10/07 [HCLT_Arvind] [1005414] 【IT5_3.3. 】Checking for 10.1 inch
#ifdef DEF_RES_WSVGA
							st_BadgeKeyInfo[i].puc_IconNotActivate = ( uchar* )i75_043_000_d_X81_Y100;
#else // #ifdef DEF_RES_WSVGA.
							st_BadgeKeyInfo[i].puc_IconNotActivate = ( uchar* )i70_043_000_d_X64_Y80;
#endif // #ifdef DEF_RES_WSVGA.
							st_BadgeKeyInfo[i].uc_IconPaletteNotActivate = PNLD_PaletteCommon;
							st_BadgeKeyInfo[i].bl_isShowIcon = True;

						} else {
							st_BadgeKeyInfo[i].bl_isShow = False;
						}
						break;
					}
// comment 2018/04/18 【IT6_Eagle】【HCLT_Pavani】【RQ-675】【WP 3465】 Download and install MarketPlace on all MFPs according to the performance.
					case TYPE_TopMenuShortCutTypeMarketPlaceSetup:
					{
						Bool b_MarketPlaceInstalled = False;
						PNLC_ActionDataLib* pc_ActionDataLib = PNLC_ActionDataLib::newInstance();
						if( NULL != pc_ActionDataLib ) {
						b_MarketPlaceInstalled = pc_ActionDataLib->isInstalledMarketPlaceApp();
						}
// Comment 2019/06/08 [IT6_1.0MR2] AR1077690 [HCLT-Catherine]  AdditionalNG Fixed.
//						if ( False == b_MarketPlaceInstalled ){
							if ( ( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW172 , 2 ) ) 
// comment 2018/09/12 【IT6_Eagle】【HCLT_Pavani】【AR1053803】Support for Market Place Application Failure case.
//							&& ( False == PnlComGetIWSSetting()->getEnable() ) 
//Comment 2018/05/29 【IT6_Eagle】【HCLT_Catherine】【AR1077523】To remove Market place App use setting.
//							&& ( True  == APIC_SystemInfo::newInstance()->isOwnOrGeneric() )
//							&& ( False == APIC_SystemInfo::newInstance()->isCardLineMode() ) 
//							&& ( True == PnlComGetIWSSetting()->getMarketPlaceAppUse()) ){
//comment 2019/11/06 [HCL-Raja]  [IT6_1.0MR2][AR1077698] Add the condition to check FIPS mode ON/OFF for Market placesetup key.	
//							&& ( True  == APIC_SystemInfo::newInstance()->isOwnOrGeneric() ) ){
// Comment 2020/05/20 [HCLT-Ranjith] [IT62.0_MR3] [制御構成変更_No.35] OWN/Generic condition remove to dispaly Marketplace setup key.
//							&& ( True  == APIC_SystemInfo::newInstance()->isOwnOrGeneric() )
//Comment 2019/06/15 【IT6_1.0MR2】【HCLT_Prem】【AR1077265】Condition check for checking if Market Place Setting is enabled or not.
							&& ( True == PnlComGetIWSSetting()->getMarketPlaceAppUse() )
							&& ( TYPE_FIPSMode_Disable == APIC_FIPSSetting::newInstance()->getFIPSMode() ) ){
								st_BadgeKeyInfo[i].e_ShortCutKind = PNLE_TopMenuShortCut_MarketPlaceSetup;	// Market place setup Shortcut type.
								st_BadgeKeyInfo[i].tid_FuncName = s_pKEYMarketPlaceSetup_W110H042E;				// Market place setup Key Name.
#ifdef DEF_RES_WSVGA
								st_BadgeKeyInfo[i].puc_FuncIcon = ( uchar* )i75_043_000_n_X81_Y100;
#else // #ifdef DEF_RES_WSVGA.
								st_BadgeKeyInfo[i].puc_FuncIcon = ( uchar* )i70_043_000_n_X64_Y80;
#endif // #ifdef DEF_RES_WSVGA.
								st_BadgeKeyInfo[i].uc_IconPaletteType = PNLD_PaletteCommon;
#ifdef DEF_RES_WSVGA
								st_BadgeKeyInfo[i].puc_IconNotActivate = ( uchar* )i75_043_000_d_X81_Y100;
#else // #ifdef DEF_RES_WSVGA.
								st_BadgeKeyInfo[i].puc_IconNotActivate = ( uchar* )i70_043_000_d_X64_Y80;
#endif // #ifdef DEF_RES_WSVGA.
								st_BadgeKeyInfo[i].uc_IconPaletteNotActivate = PNLD_PaletteCommon;
								st_BadgeKeyInfo[i].bl_isShowIcon = True;
// commemt 2019/09/03 SCD2-Sky二口 [EagleH][OP_BTS-12799] MarketPlaseがインストールされているのにSetupが表示されてしまう.								
								if ( ( False == m_bl_isDBInit ) || ( True == b_MarketPlaceInstalled) ) {
									st_BadgeKeyInfo[i].bl_isShow = False;
								}
							} else {
								st_BadgeKeyInfo[i].bl_isShow = False;
							}
// Comment 2018/10/08 [HCL-Pavani] [IT6_Eagle] [AR1060058] MarketPlace Application Not Applicable implementation commented.
//						} else {
//								st_BadgeKeyInfo[i].e_ShortCutKind = PNLE_TopMenuShortCut_MarketPlaceApplication;
//								st_BadgeKeyInfo[i].tid_FuncName = s_pKEYMarketPlace_W110H042E;				// Market place application Key Name.
//#ifdef DEF_RES_WSVGA
//								st_BadgeKeyInfo[i].puc_FuncIcon = ( uchar* )i75_043_000_n_X81_Y100;			// Market place application Key Icon.
//#else // #ifdef DEF_RES_WSVGA.
//								st_BadgeKeyInfo[i].puc_FuncIcon = ( uchar* )i70_043_000_n_X64_Y80;			// Market place application Key Icon.
//#endif // #ifdef DEF_RES_WSVGA.
//								st_BadgeKeyInfo[i].uc_IconPaletteType = PNLD_PaletteCommon;
//#ifdef DEF_RES_WSVGA
//								st_BadgeKeyInfo[i].puc_IconNotActivate = ( uchar* )i75_043_000_d_X81_Y100;
//#else // #ifdef DEF_RES_WSVGA.
//								st_BadgeKeyInfo[i].puc_IconNotActivate = ( uchar* )i70_043_000_d_X64_Y80;
//#endif // #ifdef DEF_RES_WSVGA.
//								st_BadgeKeyInfo[i].uc_IconPaletteNotActivate = PNLD_PaletteCommon;
//								st_BadgeKeyInfo[i].bl_isShowIcon = True;									// Market place application Key Icon Display Status.	
// Comment 2019/06/08 [IT6_1.0MR2] AR1077690 [HCLT-Catherine]  AdditionalNG Fixed.
//						}
					}
					break;
					default:	break;
				}
			}
		}
// comment 2018/04/04 SCD2 K.Nishiki [Eagle][制御構成変更No.111]ホーム画面認証画面の新UI化対応.
		if( ( ( ( m_uc_TopMenuListKeyDisp * uc_maxPage ) <= i ) )
		    && ( True == st_BadgeKeyInfo[i].bl_isShow )
		    && ( PNLE_TopMenuShortCut_None != st_BadgeKeyInfo[i].e_ShortCutKind ) ) {
			uc_maxPage = static_cast<uchar>( i / m_uc_TopMenuListKeyDisp + 1 );
		}
	}

// comment 2020/08/17 [特ROM対応][IT5-144(機種マージ)] 転送ファックス設定をパスワード無しで使用可能にする要望対応.
	// 転送ファクス設定は、空いているショートカットキー位置を探して設定するようにする.
	// (特に要望としてどこに入れろと指定されていないことと、既存のショートカットキーやカスタマイズに影響を及ぼさないようにするため).
	// 空いている箇所が全くない場合は、23番目の位置に設定されているキーを上書きして設定する.
	if( ( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW236 , 5 ) )		// DipSWNo.236 bit5がON.
	 && ( False == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW141 , 3 ) ) ) {	// DipSWNo.141 bit3がOFF.
		// 転送ファクス設定ができない条件が成立する場合は、ショートカットキーも表示しない.
		Bool bl_ForwardFaxKeyEnable = True;
		TYPE_KeyMode e_KeyMode = APIC_KeyDevice::getKeyMode();								// 管理機能モード.
		Bool bl_DipSw39Bit1 = APIC_SoftDipSw::newInstance()->getSoftDipSwBit( 39, 1 );		// DipSW No.39 0x04 SDR-6 ベンダー接続時に無条件に禁止している機能制限を外す.
		Bool bl_DipSw33Bit1 = APIC_SoftDipSw::newInstance()->getSoftDipSwBit( 8, 6 );		// DipSW No.33 0x04 ID-6 ベンダー２使用時に受信FAXの転送機能を使用許可.
		// ベンダ/PageACSES時は非表示.
		if( ( e_KeyMode == TYPE_KeyModeVendor   &&  False == bl_DipSw39Bit1 )								// ベンダー(従来タイプ)、DipSW No.39 0x04が無効.
		 || ( e_KeyMode == TYPE_KeyModeVendorKM &&  False == bl_DipSw39Bit1 && False == bl_DipSw33Bit1 )	// ベンダー(統合タイプ)、DipSW No.39 0x04が無効(SDR-6)＋DipSW No.33 0x04が無効(ID-6).
		 || ( e_KeyMode == TYPE_KeyModePageAcses ) ) {														// PageACSES.
			bl_ForwardFaxKeyEnable = False;
		}
		// FAXボード未接続若しくはPrefix/Suffixを使用する場合.
		else if( ( False == onlyPanel.isFaxExist() ) || ( False != onlyPanel.isFaxServerEnable() ) ) {
			bl_ForwardFaxKeyEnable = False;
		}
		// FAX機能別組み合わせ禁止条件に抵触する場合は非表示.
		else if( True == isForwardFAXFuncCombError() ) {
			bl_ForwardFaxKeyEnable = False;
		}

		// 転送ファクス設定ショートカットキー、表示OK！.
		if( bl_ForwardFaxKeyEnable == True ){
			uchar uc_ForwardFaxKeyIndex = 24;		// 25番目(後ろから3つ目)の位置を初期設定.
			for( uchar x = 0; x < PNLD_TOPMENU_SHORTCUTICON_MAX; x++ ) {
				if( st_BadgeKeyInfo[x].e_ShortCutKind == PNLE_TopMenuShortCut_None		// 空いている箇所を発見.
				 && st_BadgeKeyInfo[x].bl_isShow != True ) {
					uc_ForwardFaxKeyIndex = x;
					break;
				}
			}

			st_BadgeKeyInfo[ uc_ForwardFaxKeyIndex ].bl_isShow = True;
			st_BadgeKeyInfo[ uc_ForwardFaxKeyIndex ].bl_isActivate = True;
			st_BadgeKeyInfo[ uc_ForwardFaxKeyIndex ].bl_isShowIcon = True;
			st_BadgeKeyInfo[ uc_ForwardFaxKeyIndex ].e_ShortCutKind = PNLE_TopMenuShortCut_ForwardFax;
			st_BadgeKeyInfo[ uc_ForwardFaxKeyIndex ].tid_FuncName = s_pKEYForwardFaxSetting_W108H042E;
			st_BadgeKeyInfo[ uc_ForwardFaxKeyIndex ].puc_FuncIcon = ( uchar* )i70_023_000_X64_Y80;
			st_BadgeKeyInfo[ uc_ForwardFaxKeyIndex ].uc_IconPaletteType = PNLD_PaletteCommon;
#ifdef DEF_RES_WSVGA
			st_BadgeKeyInfo[ uc_ForwardFaxKeyIndex ].puc_IconNotActivate = ( uchar* )i75_023_000_d_X81_Y100;
#else
			st_BadgeKeyInfo[ uc_ForwardFaxKeyIndex ].puc_IconNotActivate = ( uchar* )i70_023_000_d_X64_Y80;
#endif
			st_BadgeKeyInfo[ uc_ForwardFaxKeyIndex ].uc_IconPaletteNotActivate = PNLD_PaletteCommon;

			// 設定したキー位置から、MaxPageを更新する必要がある場合は更新しておく.
			if( ( ( PNLD_TopMenuKeyDisp * uc_maxPage ) <= uc_ForwardFaxKeyIndex ) ) {
				uc_maxPage = uc_ForwardFaxKeyIndex / PNLD_TopMenuKeyDisp + 1;
			}
		}
	}

	if( False ) {	// IT4_4th以前の処理を一応残しておく.
//comment 2018/04/16 HCLT-Haribhaskar [IT6_Eagle] [RQ-698-2] [WP-3683] Removing WebLicense Management.
		if( ( TYPE_TopMenuShortCutTypeNone == st_BadgeKeyInfo[9].e_ShortCutKind )	// 割付番号10番が未設定(キー番号0開始).
		    && ( False == m_bl_BrowserFlag )											// ブラウザーが未設定.
//		    && ( TYPE_CategoryState_ENABLE != PnlComGetEnablerSetting()->getFEK_CotegoryState( TYPE_FEKType01 ) )		// ライセンス無効.
		    && ( False == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW203,7 ) ) ) {		// 且つ、DipSW Off.
			st_BadgeKeyInfo[9].e_ShortCutKind = PNLE_TopMenuShortCut_FuncBrowser;
			st_BadgeKeyInfo[9].tid_FuncName = s_pKEYActivityWebBrowser_W100H042E;
			st_BadgeKeyInfo[9].puc_FuncIcon = ( uchar* )i70_028_000_n_X64_Y80;
			st_BadgeKeyInfo[9].uc_IconPaletteType = PNLD_PaletteTopMenuBaseIcon2;
			st_BadgeKeyInfo[9].puc_IconNotActivate = ( uchar* )i70_028_000_d_X64_Y80;
			st_BadgeKeyInfo[9].uc_IconPaletteNotActivate = PNLD_PaletteCommon;
			st_BadgeKeyInfo[9].bl_isShow = True;
		}
		if( ( TYPE_TopMenuShortCutTypeNone == st_BadgeKeyInfo[10].e_ShortCutKind )	// 割付番号11番が未設定(キー番号0開始).
		    && ( False == m_bl_AppFlag )													// APPキーが未設定.
		    && ( False == ScanOapExtSet.isRegistApplication() && 0 == ExtSet.countApps() ) ) {	// アプリ未登録.
			st_BadgeKeyInfo[10].e_ShortCutKind = PNLE_TopMenuShortCut_FuncOAPApp;
			st_BadgeKeyInfo[10].tid_FuncName = s_pKEYExternalApps_W100H042E;
// Comment 2016/10/07 [HCLT_Arvind] [1005414] 【IT5_3.3】 Checking for 10.1 inch
#ifdef DEF_RES_WSVGA
			st_BadgeKeyInfo[10].puc_FuncIcon = ( uchar* )i75_035_000_n_X81_Y100;
#else // #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[10].puc_FuncIcon = ( uchar* )i70_035_000_n_X64_Y80;
#endif // #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[10].uc_IconPaletteType = PNLD_PaletteCommon;
// Comment 2016/10/07 [HCLT_Arvind] [1005414] 【IT5_3.3】 Checking for 10.1 inch
#ifdef DEF_RES_WSVGA
			st_BadgeKeyInfo[10].puc_IconNotActivate = ( uchar* )i75_035_000_d_X81_Y100;
#else // #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[10].puc_IconNotActivate = ( uchar* )i70_035_000_d_X64_Y80;
#endif // #ifdef DEF_RES_WSVGA.
			st_BadgeKeyInfo[10].uc_IconPaletteNotActivate = PNLD_PaletteCommon;
			st_BadgeKeyInfo[10].bl_isShow = True;
			// PKI認証設定時かつセキュリティー強化設定有効の場合は非表示.
			if( ( True == PLBC_FuncPKICheck::isPKI() )
			    && ( True == PnlComGetSystemSetting()->getSecurityLevelUpOn() ) ) {
				st_BadgeKeyInfo[10].bl_isShow = False;
			}
			else if( False != APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFOSOFTWARE140,3 ) ) {
				st_BadgeKeyInfo[10].bl_isShow = False;
			}
		}
	}
}

// comment 2020/08/17 [特ROM対応][IT5-144(機種マージ)] 転送ファックス設定をパスワード無しで使用可能にする要望対応.
//================================================================================================.
//概要    :IT5-144 専用 転送ファクス設定のFAX機能別組み合わせ禁止確認.
//備考    :戻り値：True：組み合わせ禁止あり  False：組み合わせ禁止なし.
//================================================================================================.
Bool PCGC_HomeTopMenu::isForwardFAXFuncCombError()
{
	APIC_FaxGfpSetting* pc_FaxGfpSetting = PnlComGetFaxGfpSetting();
	APIC_FaxJobSetting* pc_FaxJobSetting = PnlComGetFaxJobSetting();

	Bool bl_CmbError = False;

	// 強制メモリ受信設定との組み合わせ確認.
	if( TYPE_ReceiveInfoApportionOn == PnlComGetFaxJobSetting()->getReceiveInfoApportionOn() ) {
		// 回線別受信がON.
		if( ( TYPE_RcvFunc_CompulsionRxOn == PnlComGetFaxGfpSetting()->getRcvFunc_CompulsionRxAll( TYPE_CommLine1 ) )
		 || ( TYPE_RcvFunc_CompulsionRxOn == PnlComGetFaxGfpSetting()->getRcvFunc_CompulsionRxAll( TYPE_CommLine2 ) )
		 || ( TYPE_RcvFunc_CompulsionRxOn == PnlComGetFaxGfpSetting()->getRcvFunc_CompulsionRxAll( TYPE_CommLine3 ) )
		 || ( TYPE_RcvFunc_CompulsionRxOn == PnlComGetFaxGfpSetting()->getRcvFunc_CompulsionRxAll( TYPE_CommLine4 ) ) ) {
			bl_CmbError = True;
		}
	} else {
		// 回線別受信がOFF.
		if( TYPE_RcvFunc_CompulsionRxOn == pc_FaxGfpSetting->getRcvFunc_CompulsionRx() ) {
			bl_CmbError = True;
		}
	}

	// PC-Fax受信設定との組み合わせ確認.
	if( False == bl_CmbError ) {
		if( TYPE_ReceiveInfoApportionOn == PnlComGetFaxJobSetting()->getReceiveInfoApportionOn() ) {
			// 回線別受信がON.
			if( ( TYPE_PcFax_RxMode0 != pc_FaxJobSetting->getPcFax_RxModeAll( TYPE_CommLine1 ) )
			 || ( TYPE_PcFax_RxMode0 != pc_FaxJobSetting->getPcFax_RxModeAll( TYPE_CommLine2 ) )
			 || ( TYPE_PcFax_RxMode0 != pc_FaxJobSetting->getPcFax_RxModeAll( TYPE_CommLine3 ) )
			 || ( TYPE_PcFax_RxMode0 != pc_FaxJobSetting->getPcFax_RxModeAll( TYPE_CommLine4 ) ) ) {
				bl_CmbError = True;
			}
		} else {
			// 回線別受信がOFF.
			if( TYPE_PcFax_RxMode0 != pc_FaxJobSetting->getPcFax_RxMode() ) {
				bl_CmbError = True;
			}
		}
	}

	// TSI受信振り分け設定との組み合わせ確認.
	if( False == bl_CmbError ) {
		if( TYPE_ReceiveInfoApportionOn == PnlComGetFaxJobSetting()->getReceiveInfoApportionOn() ) {
			// 回線別受信がON.
			if( ( TYPE_TsiRouting_On == pc_FaxJobSetting->getTsiRoutingAll( TYPE_CommLine1 ) )
			 || ( TYPE_TsiRouting_On == pc_FaxJobSetting->getTsiRoutingAll( TYPE_CommLine2 ) )
			 || ( TYPE_TsiRouting_On == pc_FaxJobSetting->getTsiRoutingAll( TYPE_CommLine3 ) )
			 || ( TYPE_TsiRouting_On == pc_FaxJobSetting->getTsiRoutingAll( TYPE_CommLine4 ) ) ) {
				bl_CmbError = True;
			}
		} else {
			// 回線別受信がOFF.
			if( TYPE_TsiRouting_On == pc_FaxJobSetting->getTsiRouting() ) {
				bl_CmbError = True;
			}
		}
	}

	return bl_CmbError;
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： 機能情報の設定.
//================================================================================================
struct PNLS_TopMenuBadgeKeyText {
	PNLE_TopMenuShortCut	e_ShortCutKind;
	TEXT_ID					tid_FuncName;
	uchar*					puc_FuncIcon;
	uchar					uc_IconPaletteType;
	uchar*					puc_IconNotActivate;
	uchar					uc_IconPaletteNotActivate;
};
//Comment 2016/10/07 【HCLT_Sri charan]】【IT5_3.3】【Accurev ID:1005414】Changed the PNG for 10.1 Inch Models.
static const PNLS_TopMenuBadgeKeyText st_TopMenuBadgeKeyFuncTbl[] = {
#if defined(DEF_RES_WSVGA)
	{ PNLE_TopMenuShortCut_FuncCopy,		s_pKEYActivityCopy_W100H042E,					( uchar* )i75_025_000_n_X81_Y100,	PNLD_PaletteTopMenuBaseIcon2,	( uchar* )i75_025_000_d_X81_Y100, PNLD_PaletteCommon },	// 機能:コピー.
	{ PNLE_TopMenuShortCut_FuncScanFax,		s_pKEYActivityFaxScan_W100H042E,				( uchar* )i75_026_000_n_X81_Y100,	PNLD_PaletteTopMenuBaseIcon2,	( uchar* )i75_026_000_d_X81_Y100, PNLD_PaletteCommon },	// 機能:スキャン/ファクス.
	{ PNLE_TopMenuShortCut_FuncBox,			s_pKEYActivityBox_W100H042E,					( uchar* )i75_027_000_n_X81_Y100,	PNLD_PaletteTopMenuBaseIcon2,	( uchar* )i75_027_000_d_X81_Y100, PNLD_PaletteCommon },	// 機能:ボックス.
	{ PNLE_TopMenuShortCut_FuncBrowser,		s_pKEYActivityWebBrowser_W100H042E,				( uchar* )i75_028_000_n_X81_Y100,	PNLD_PaletteTopMenuBaseIcon2,	( uchar* )i75_028_000_d_X81_Y100, PNLD_PaletteCommon },	// 機能:ブラウザ.
	{ PNLE_TopMenuShortCut_FuncOAPApp,		s_pKEYExternalApps_W100H042E,					( uchar* )i75_035_000_n_X81_Y100, 	PNLD_PaletteCommon, 			( uchar* )i75_035_000_d_X81_Y100, PNLD_PaletteCommon },	// 機能:OpenAPIアプリ.
	{ PNLE_TopMenuShortCut_FuncUbqPrint,	s_pKEYActivityServerlessPullPrinting_W104H042E,	( uchar* )i75_037_000_n_X81_Y100,	PNLD_PaletteCommon,		 		( uchar* )i70_037_000_d_X64_Y80, PNLD_PaletteCommon },	// 機能:ユビキタスプリント.
	{ PNLE_TopMenuShortCut_FuncFaxOnly,		s_pKEYActivityFax_W100H042E,					( uchar* )i75_041_000_n_X81_Y100, 	PNLD_PaletteTopMenuBaseIcon2, 	( uchar* )i75_041_000_d_X81_Y100, PNLD_PaletteCommon },	// 機能:ファクス.
#else // #ifdef DEF_RES_WSVGA.
	{ PNLE_TopMenuShortCut_FuncCopy,	s_pKEYActivityCopy_W100H042E,	( uchar* )i70_025_000_n_X64_Y80, PNLD_PaletteTopMenuBaseIcon2, ( uchar* )i70_025_000_d_X64_Y80, PNLD_PaletteCommon },	// 機能:コピー.
	{ PNLE_TopMenuShortCut_FuncScanFax,	s_pKEYActivityFaxScan_W100H042E,	( uchar* )i70_026_000_n_X64_Y80, PNLD_PaletteTopMenuBaseIcon2, ( uchar* )i70_026_000_d_X64_Y80, PNLD_PaletteCommon },	// 機能:スキャン/ファクス.
// comment 2019/12/05 SCD2-Sky岩渕 [OP_BTS-17431][EagleZ+] SparrowAIO機でのボックス機能の名称修正.
#ifdef DEF_SparrowAIOSFPCus
	// SparrowAIO機ではユーザーボックスが使えないため、翻訳をUserBoxから変更する.
	{ PNLE_TopMenuShortCut_FuncBox,		s_pKEYSystemBox_W100H042E,	( uchar* )i70_027_000_n_X64_Y80, PNLD_PaletteTopMenuBaseIcon2, ( uchar* )i70_027_000_d_X64_Y80, PNLD_PaletteCommon },	// 機能:ボックス.
#else // #ifdef DEF_SparrowAIOSFPCus.
	{ PNLE_TopMenuShortCut_FuncBox,		s_pKEYActivityBox_W100H042E,	( uchar* )i70_027_000_n_X64_Y80, PNLD_PaletteTopMenuBaseIcon2, ( uchar* )i70_027_000_d_X64_Y80, PNLD_PaletteCommon },	// 機能:ボックス.
#endif // #ifdef DEF_SparrowAIOSFPCus.
	{ PNLE_TopMenuShortCut_FuncBrowser,	s_pKEYActivityWebBrowser_W100H042E,	( uchar* )i70_028_000_n_X64_Y80, PNLD_PaletteTopMenuBaseIcon2, ( uchar* )i70_028_000_d_X64_Y80, PNLD_PaletteCommon },	// 機能:ブラウザ.
	{ PNLE_TopMenuShortCut_FuncOAPApp,	s_pKEYExternalApps_W100H042E,	( uchar* )i70_035_000_n_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_035_000_d_X64_Y80, PNLD_PaletteCommon },	// 機能:OpenAPIアプリ.
// [MinervaSBK][RQ-268] サーバーレスユビキタス文言変更対応 start.
//	{ PNLE_TopMenuShortCut_FuncUbqPrint,	s_pKEYActivityUbqPrint_W100H042E,	( uchar* )i70_037_000_n_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_037_000_d_X64_Y80, PNLD_PaletteCommon },	// 機能:ユビキタスプリント.
	{ PNLE_TopMenuShortCut_FuncUbqPrint,	s_pKEYActivityServerlessPullPrinting_W104H042E,	(uchar*)i70_037_000_n_X64_Y80, PNLD_PaletteCommon,		 (uchar*)i70_037_000_d_X64_Y80, PNLD_PaletteCommon },	// 機能:ユビキタスプリント.
// [MinervaSBK][RQ-268] サーバーレスユビキタス文言変更対応 end.
//	{ PNLE_TopMenuShortCut_FuncFaxOnly,	s_pKEYActivityFax_W100H042E,					(uchar*)i70_026_000_n_X64_Y80, PNLD_PaletteTopMenuBaseIcon2, (uchar*)i70_026_000_d_X64_Y80, PNLD_PaletteCommon },	// 機能:ファクス.
	{ PNLE_TopMenuShortCut_FuncFaxOnly,	s_pKEYActivityFax_W100H042E,	( uchar* )i70_041_000_n_X64_Y80, PNLD_PaletteTopMenuBaseIcon2, ( uchar* )i70_041_000_d_X64_Y80, PNLD_PaletteCommon },	// 機能:ファクス.
// comment mse平井 [RQ-232] 2014/09/23.
#endif	// #if defined(DEF_RES_WSVGA).
	{ PNLE_TopMenuShortCut_ScanToDevice,	s_pKEYActivityScanToMobile_W100H042E,		(uchar*)f10_010_000_X64_Y80, PNLD_PaletteCommon,			 (uchar*)f10_013_000_d_X64_Y80, PNLD_PaletteCommon },	// 機能:ScanToDevice.
};

//================================================================================================.
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： 機能キー表示情報設定.
// 変更履歴 ： 2014/12/8  OPS1-MSE大石 [ZeusS][RQ-232][CQ507814]ユーザ認証設定なし時はRAユーザの事前認証、スキャン制限をみないように修正.
//          ： 2014/12/18 OPS1-MSE大石 [ZeusS][RQ-232][CQ508026]ユーザ認証設定なし、部門認証時は、PanelLinkスキャン非表示に修正.
//          ： 2015/01/08 OPS1-MSE斉藤 [Poseidon][RQ-232][CQ508272]機能バージョンチェックを追加.
//          ： 2015/01/28 OPS1-MSE田中 [ZeusS][RQ-232][CQ509392]PanelLinkスキャンアイコン活性状態の初期化処理の見直し.
//================================================================================================.
void PCGC_HomeTopMenu::setFunctionInfo( uchar uc_index, TYPE_TopMenuShortCutFunction e_FunctionType )
{
	char _sc_select = -1;
	Bool bl_OapIconExit = False;

	st_BadgeKeyInfo[uc_index].bl_BMP = False;			// 通常アイコンとして初期化する.
//comment [2016/07/13] [HCLT-Kalaiyarasi] [Icarus] [CQ530098] Adding a pointer instance to get the fax setting conditions value from the common settings.
	PNLC_ScanFaxDataLib* pc_ScanFaxDataLib = PNLC_ScanFaxDataLib::newInstance();
	APIC_TabletIF* pc_TabletIF 			= NULL;								// API Common API取得.
	APIC_AndroidPanel* pc_AndroidPanel 	= NULL;								// Androidパネル関連 API取得.
//	APIE_TabletOptionState e_GoogleDriveSave  = APIE_TabletOptionDisable;	// Googleドライブ保存機能有効無効情報取得.
//	APIE_TabletOptionState e_LocalStorageSave = APIE_TabletOptionDisable;	// ローカルストレージ保存機能有効無効情報取得.
	APIE_TabletOptionState e_ScanToMobile = APIE_TabletOptionDisable;		// ScanToMobile機能有効無効情報取得.
	Bool	bl_RAUserPreAuth;												// RA事前認証結果.
	APIE_RAScanLimitColorInfo	e_RAScanLimitColorInfo; 					// RAユーザのスキャン制限.

	switch( e_FunctionType ) {
		case TYPE_TopMenuShortCutFunctionCopy:			// コピー.
			_sc_select = 0;
			st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_Copy );
			break;
		case TYPE_TopMenuShortCutFunctionScanFax:		// スキャン/ファクス.
			_sc_select = 1;
			st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_Fax );
			break;
		case TYPE_TopMenuShortCutFunctionBox:			// ボックス.
			_sc_select = 2;
			st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_BoxOperation );
			break;
		case TYPE_TopMenuShortCutFunctionBrowser:		// ブラウザ.
			_sc_select = 3;
			if( ( ( PCGE_OapMyPanel == PnlComGetMyPanelType() ) && ( 13 > uc_index ) )		// External_MyPanelの12番目まで.
			    || ( PCGE_OapMyPanel != PnlComGetMyPanelType() ) ) {						// External_MyPanelでない.
				m_bl_BrowserFlag = True;				// ブラウザが割り当てられているのでフラグを立てる.
			}
			st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_WebBrowser );
			// Webブラウザーが無効の場合、Webブラウザーキー非表示.
//comment 2018/04/16 HCLT-Haribhaskar [IT6_Eagle] [RQ-698-2] [WP-3683] Removing WebLicense Management.
//			if( ( False == PnlComGetBrowserSetting()->getEnableBrowser() )
////// Comment 2017/11/08【HCLT-Gadiya sai】【IT5 4.2】【RQ686】【WP412】Multiple Network support.
////				|| ( TYPE_CategoryState_ENABLE != PnlComGetEnablerSetting()->getFEK_CotegoryState( TYPE_FEKType01 ) ) ) {
//			    || ( TYPE_CategoryState_ENABLE != PnlComGetEnablerSetting()->getFEK_CotegoryState( TYPE_FEKType01 ) ) || ( TYPE_NetworkSpecific_NW2 == PNLC_NetworkLan::getInstance()->getLine() ) ) {
			if( ( False == PnlComGetBrowserSetting()->getEnableBrowser() )
			    || ( TYPE_NetworkSpecific_NW2 == PNLC_NetworkLan::getInstance()->getLine() )
//Comment 2018/04/23 【HCLT-Devipriya】 【IT6_Eagle】 【制御構成変更_No.100】 Condition added to Hide webbrowser when fucntion option is not enabled.
				|| ( PNLE_AppOptionStatus_Enable != PnlComGetEnableAppOptionStatus( TYPE_AppOptionBrowser )) ) {
				st_BadgeKeyInfo[uc_index].bl_isShow = False;
			}
			break;
		case TYPE_TopMenuShortCutFunctionApp:			// OpenAPIアプリ.
		{
			_sc_select = 4;
			if( ( ( PCGE_OapMyPanel == PnlComGetMyPanelType() ) && ( 13 > uc_index ) )		// External_MyPanelの12番目まで.
			    || ( PCGE_OapMyPanel != PnlComGetMyPanelType() ) ) {						// External_MyPanelでない.
				m_bl_AppFlag = True;					// APPキーが割り付けられているのでフラグを立てる
			}
			// 登録アプリケーションがない場合、またはPKI認証設定時かつセキュリティー強化設定有効の場合、登録アプリケーションメニューキーを非表示.
			if( ( False == ScanOapExtSet.isRegistApplication() && 0 == ExtSet.countApps() )
// comment 2019/04/13 【HCL-Prem】【IT6_1.0MR】【AR1075599】 Adding Support for Market place app when Web Browser is disabled.
//			    || ( False == checkShortCutDisableOap( static_cast<TYPE_TopMenuShortCutType>(e_FunctionType), NULL ) )
			    || ( False == checkShortCutDisableOap( static_cast<TYPE_TopMenuShortCutType>(e_FunctionType), NULL ) &&  s_registerAppIws.size( ) == 0)
			    || ( ( True == PLBC_FuncPKICheck::isPKI() ) && ( True == PnlComGetSystemSetting()->getSecurityLevelUpOn() ) ) ) {
				st_BadgeKeyInfo[uc_index].bl_isShow = False;
			}
			
// commemt 2019/09/03 SCD2-Sky二口 [EagleH][OP_BTS-12799] MarketPlaseがインストールされているのにSetupが表示されてしまう.
			// DataBase初期化未完了の時はAPPキー非表示.
			if ( False == m_bl_isDBInit ) {
				st_BadgeKeyInfo[uc_index].bl_isShow = False;
			}

			// comment 2018/11/14 SCD2 K.Seto [AR.1067363]表示対象となるアプリがMarketPlaceアプリのみの場合、APPキーを非表示とする.
			PNLC_ActionDataLib* pc_ActionDataLib = PNLC_ActionDataLib::newInstance();
			if ( NULL != pc_ActionDataLib ) {
				// OAPアプリの数をカウントする.
				uchar uc_OAPAppCnt = 0;
				uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeScanD );
				uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeScanS );
				uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypePullPrint );
				uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeBrowserScan );
				uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeBrowserLauncher );
				uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeBrowserPull );
				uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeBrowserGeneral );
				uc_OAPAppCnt += ExtSet.countApps();
				// comment 2018/12/6 Chikuan [AR1067363][EagleL] MarketPlaceのチェック条件を修正.
//				if ( True == pc_ActionDataLib->isInstalledMarketPlaceApp() ) {
				Bool bl_ExistMarketPlaceWithIWSOff = checkExistMarketPlaceWithIWSOff();
// Comment 2019/12/27 [HCLT_Devipriya] [IT6_EagleBK] [OP_BTS-9586] Hide APP key when IWS is off and Marketplace setup exist, no OAP app. 				
// Comment 2019/06/27 [HCLT_Ranjith] [IT6_EagleH] [OP_BTS-8738] App key display when browser launcher is installed. 
//				if ( ( True == pc_ActionDataLib->isInstalledMarketPlaceApp() ) || ( True == bl_ExistMarketPlaceWithIWSOff ) ) {
//				if ( True == pc_ActionDataLib->isInstalledMarketPlaceApp() ) {
				if ( ( True == pc_ActionDataLib->isInstalledMarketPlaceApp() ) || ( True == bl_ExistMarketPlaceWithIWSOff ) ) {
					// MarketPlaceアプリはBrowserLauncherでカウントされてしまうため、インストールされている場合は1引いておく.
					uc_OAPAppCnt = uc_OAPAppCnt - 1;
				}

				// comment 2018/12/6 Chikuan [AR1067363][EagleL] MarketPlaceのチェック条件を修正.
//				if ( ( True == ScanOapExtSet.isRegistApplication() ) && ( 0 == uc_OAPAppCnt ) && ( True == pc_ActionDataLib->isInstalledMarketPlaceApp() ) ) {
				if ( ( True == ScanOapExtSet.isRegistApplication() ) && ( 0 == uc_OAPAppCnt ) && ( ( True == pc_ActionDataLib->isInstalledMarketPlaceApp() ) || ( True == bl_ExistMarketPlaceWithIWSOff ) ) ) {
					// OAPアプリは未インストール、かつMarketPlaceアプリがインストール済み.
					// IWSアプリのチェック.
					APIC_IWSAppInfo* pc_IWSAppInfo = APIC_IWSAppInfo::newInstance();
					if ( NULL != pc_IWSAppInfo ) {
						// IWSアプリ数を取得.
						uchar uc_IWSAppCount = ( uchar )pc_IWSAppInfo->getApplicationNum();
						if ( uc_IWSAppCount > 1 ) {
							// アプリ数が1より大きい、即ちMarketPlaceアプリ以外のIWSアプリもインストールされている.
							uchar uc_IwsAppTypeGeneralCnt = 0;
							TYPS_IwsAppInfo s_tempAppInfo;
							memset( &s_tempAppInfo, '\0', sizeof( s_tempAppInfo ) );
							// インストールされているIWSアプリのアプリタイプを取得.
							for( uchar uc_Cnt = 0; uc_Cnt < uc_IWSAppCount; uc_Cnt++ ) {
								int si_Ret = pc_IWSAppInfo->getApplication( (int)( uc_Cnt+1 ), s_tempAppInfo );
								if ( ( TYPD_IwsResult_OK == si_Ret )
								  && ( TYPE_IwsAppTypeGeneral == s_tempAppInfo.e_AppType ) ) {
									// アプリタイプがTYPE_IwsAppTypeGeneralのものは表示対象なのでカウント.
									uc_IwsAppTypeGeneralCnt++;
								}
							}
							if ( 1 == uc_IwsAppTypeGeneralCnt ) {
								// 表示対象のアプリが1、即ちMarketPlaceアプリのみのため、APPキーも表示不要.
								st_BadgeKeyInfo[uc_index].bl_isShow = False;
							}
						} else {
							// アプリ数が1、即ちMarketPlaceアプリのみのため、APPキーも表示不要.
							st_BadgeKeyInfo[uc_index].bl_isShow = False;
						}
					}
				}
			}
			break;
		}
		case TYPE_TopMenuShortCutFunctionUbqPrint:			// ユビキタスプリント.
		{
			_sc_select = 5;
			if( False == PnlComCheckUbqClientUse()						||		// ユビキタスプリントクライアント機能利用不可能.
			    False == PnlComGetUbiquitousSetting()->getClientOn()	||		// クライアント機能が無効.
			    False == c_ExtPanel.isEnableMFPFunction()						// 指定アプリMFP基本機能設定禁止の場合.
			    ) {	
				st_BadgeKeyInfo[uc_index].bl_isShow = False;
			}
			break;
		}
		case TYPE_TopMenuShortCutFunctionFaxApp:
		{
			_sc_select = 6;
			// IT5機能拡張1以降.
// Comment 2017/11/29 【HCLT-Deeksha】【Eagle】【制御構成変更_No.4】 Removing old function version check condition and else portion.
			st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShowFaxOnly();
		}
		break;
		case TYPE_TopMenuShortCutFunctionScanToAndroid:		// ScanToDevice.
			// [RQ-232] ScanToDeviceを表示するか判定する.
			pc_TabletIF 		= APIC_TabletIF::newInstance();												// API Common API取得.
			pc_AndroidPanel 	= APIC_AndroidPanel::newInstance();											// Androidパネル関連 API取得.

// comment 2014/12/8 MSE大石 [RQ-232][CQ507814]ユーザ認証設定なし時はRAユーザの事前認証、スキャン制限をみない.
			// 初期化.
			st_BadgeKeyInfo[uc_index].bl_isShow = False;
// comment 2015/01/28 OPS1-MSE田中 [RQ-232][ZeusS][CQ509392]PanelLinkスキャンアイコン活性状態の初期化処理の見直し.
// PanelLinkスキャンキーの初回設定時に活性アイコンを非活性状態として設定しているため、初期値をアイコンにあわせて修正.
//			st_BadgeKeyInfo[uc_index].bl_isActivate = False;
// comment 2015/01/08 OPS1-MSE斉藤 [Poseidon][RQ-232][CQ508272]機能バージョンチェックを追加.
// Comment 2017/11/29 【HCLT-Deeksha】【Eagle】【制御構成変更_No.4】 Removing old function version version check condition.
			if ( NULL != pc_TabletIF && NULL != pc_AndroidPanel ) {
				e_ScanToMobile = pc_TabletIF->getOptionEnable( APIE_TabletScanToMobile );						// ScanToMobile機能有効無効情報取得.

				if( ( True == pc_AndroidPanel->getBizhubRASetting() )					// 管理者設定のBizhubRA使用設定が「使用する」か.
				    && ( True == pc_AndroidPanel->getEnableImageConvert() )				// 管理者設定の画像変換機能設定が「許可する」か.
				    && ( True == pc_TabletIF->getConnectState() )						// BizhubRAとの接続状態が「接続」か.
				&& ( APIE_TabletOptionEnable == e_ScanToMobile ) ) {				// RAアプリのScanToMobile機能が有効か.
					
					if ( TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication(PNLC_NetworkLan::getInstance()->getLine()) ) {			// ユーザ認証がON.

						bl_RAUserPreAuth 	= pc_AndroidPanel->getRAUserPreAuth(); 							// BizhubRAとの事前ユーザ認証が「確認済み」かどうか.
						e_RAScanLimitColorInfo = pc_AndroidPanel->getRAScanLimitColorInfo();				// RAユーザーのスキャン制限情報がスキャン不可以外(モノクロorカラー)かどうか.

						if ( ( True == bl_RAUserPreAuth )									// BizhubRAとの事前ユーザ認証が「確認済み」か.
						&& ( APIE_RAScanLimitColorInfoDisable != e_RAScanLimitColorInfo )	// RAユーザーのスキャン制限情報がスキャン不可以外(モノクロorカラー)か.
						&& ( True == PnlComGetUserAccountPublic() ) ) {						// ScanToMobileはPublicユーザー限定.
					// 表示する条件だった場合は、表示するフラグをONにする.
					_sc_select = 7;
					st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_ScanToDevice );
				}
			} else {
						// ユーザ認証設定なし時はRAユーザの事前認証、スキャン制限をみない.
// comment 2014/12/18 OPS1-MSE大石 [ZeusS][RQ-232][CQ508026]ユーザ認証設定なし、部門認証あり時はPanelLinkスキャン非表示.
						if ( TYPE_TrackModeOff == PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance()->getLine()) ) {
							// 表示するフラグをONにする.
							_sc_select = 7;
							st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_ScanToDevice );
						}
					}
				}
			}
			break;
// comment 2018/05/24 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
		// Lightモードへの切り替えキー.
		case TYPE_TopMenuShortCutFunctionSwitchLightUI:
			st_BadgeKeyInfo[uc_index].e_ShortCutKind = PNLE_TopMenuShortCut_UIMode;
			st_BadgeKeyInfo[uc_index].tid_FuncName = s_pKEYBasicStyle_W100H042E;
// comment 2018/08/10 SCD2 T.Hori [Eagle/SparrowMFP][UD評価課題対応 No.2] Light Styleアイコン変更対応.
			st_BadgeKeyInfo[uc_index].puc_FuncIcon = (uchar*)i70_049_000_n_X64_Y80;
			st_BadgeKeyInfo[uc_index].uc_IconPaletteType = PNLD_PaletteCommon;
			st_BadgeKeyInfo[uc_index].puc_IconNotActivate = (uchar*)i70_049_000_d_X64_Y80;
			st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = PNLD_PaletteCommon;
//comment 2018/8/27 SCD2-Sky鳫林 [Eagle][AR1057458]一部モード有効時、新UI操作モードの使用を不可とする対応.
//			st_BadgeKeyInfo[uc_index].bl_isShow = True;
			st_BadgeKeyInfo[uc_index].bl_isShow = PNLC_WebUIControl::getInstance()->isEnableWebUI();
			break;
				
		default:
			st_BadgeKeyInfo[uc_index].e_ShortCutKind = PNLE_TopMenuShortCut_None;
			break;
	}
	if( -1 != _sc_select ) {
		uchar uc_AppIconData[TYPD_ExtAppListKeyIconDataMaxSize];
		memset( uc_AppIconData, 0x00, sizeof( uc_AppIconData ) );		// AppIcon.
		// comment 2015/08/26 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] アイコンデータ取得APIにてデータサイズ取得の追加.
		int si_AppIconDataSize = 0;	//APIより取得したAppIconのデータサイズ.

		st_BadgeKeyInfo[uc_index].e_ShortCutKind = st_TopMenuBadgeKeyFuncTbl[( uchar )_sc_select].e_ShortCutKind;
		if( TYPE_TopMenuShortCutFunctionApp == e_FunctionType ) {
			char sc_AppListName[PCGD_ExtMaxAplNameUTF8];

			st_BadgeKeyInfo[uc_index].tid_FuncName = 0;
			memset( sc_AppListName, 0, sizeof( sc_AppListName ) );
			AplCommonSet.getAppBtnName( sc_AppListName );		// アプリリスト名取得.
			strncpy( st_BadgeKeyInfo[uc_index].sc_FuncName, sc_AppListName, sizeof( st_BadgeKeyInfo[uc_index].sc_FuncName )-1 );		// アプリリスト名設定.
			st_BadgeKeyInfo[uc_index].sc_FuncName[sizeof( st_BadgeKeyInfo[uc_index].sc_FuncName ) - 1] = '\0';
			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
				// comment 2015/08/26 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] アイコンデータ取得APIにてデータサイズ取得の追加.
				bl_OapIconExit = PnlComGetStandardPanelSetting()->getApplicationListKeyIcon( uc_AppIconData, &si_AppIconDataSize );
			}
		} else {
			if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW203, 5 ) ) {
				if( 1 == _sc_select ) {
					// スキャンファクス.
//comment [2016/07/14] [HCLT_Kalaiyarasi] [Icarus] [CQ530098] Adding condition for the Display "Scan" text.
					if( ( NULL != pc_ScanFaxDataLib ) && ( False == pc_ScanFaxDataLib->IsFaxSettingEnable() ) ) {
						st_BadgeKeyInfo[uc_index].tid_FuncName = s_pKEYActivityPCScan_W100H042E;		// ファクス/PCスキャン.
					} else {
						st_BadgeKeyInfo[uc_index].tid_FuncName = s_pKEYActivityFaxPCScan_W100H042E;		// ファクス/PCスキャン.
					}
				} else if( 2 == _sc_select ) {
					// ボックス.
					st_BadgeKeyInfo[uc_index].tid_FuncName = s_pKEYActivityScan_W100H042E;			// スキャン.
				} else {
//comment [2016/07/14] [HCLT_Kalaiyarasi] [Icarus] [CQ530098] Adding condition for the Display "Scan" text.
					if ( PNLE_TopMenuShortCut_FuncScanFax == st_TopMenuBadgeKeyFuncTbl[( uchar )_sc_select].e_ShortCutKind ) {
						if( ( NULL != pc_ScanFaxDataLib ) && ( False == pc_ScanFaxDataLib->IsFaxSettingEnable() ) ) {
							st_BadgeKeyInfo[uc_index].tid_FuncName = s_pKEYActivityScan_W100H042E;
						} else {
							st_BadgeKeyInfo[uc_index].tid_FuncName = st_TopMenuBadgeKeyFuncTbl[( uchar )_sc_select].tid_FuncName;
						}
					} else {
						st_BadgeKeyInfo[uc_index].tid_FuncName = st_TopMenuBadgeKeyFuncTbl[( uchar )_sc_select].tid_FuncName;
					}
				}
			} else {
//comment [2016/07/14] [HCLT_Kalaiyarasi] [Icarus] [CQ530098] Adding condition for the Display "Scan" text.
				if ( PNLE_TopMenuShortCut_FuncScanFax == st_TopMenuBadgeKeyFuncTbl[( uchar )_sc_select].e_ShortCutKind ) {
					if( ( NULL != pc_ScanFaxDataLib ) && ( False == pc_ScanFaxDataLib->IsFaxSettingEnable() ) ) {
						st_BadgeKeyInfo[uc_index].tid_FuncName = s_pKEYActivityScan_W100H042E;
					} else {
						st_BadgeKeyInfo[uc_index].tid_FuncName = st_TopMenuBadgeKeyFuncTbl[( uchar )_sc_select].tid_FuncName;
					}
				} else {
					st_BadgeKeyInfo[uc_index].tid_FuncName = st_TopMenuBadgeKeyFuncTbl[( uchar )_sc_select].tid_FuncName;
				}
			}
		}
		if( False == bl_OapIconExit ) {
			st_BadgeKeyInfo[uc_index].puc_FuncIcon = st_TopMenuBadgeKeyFuncTbl[( uchar )_sc_select].puc_FuncIcon;
			st_BadgeKeyInfo[uc_index].puc_IconNotActivate = st_TopMenuBadgeKeyFuncTbl[( uchar )_sc_select].puc_IconNotActivate;
			// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] 非OAPアイコン時のデータサイズ/パレット設定.独自PNG形式として設定.
			st_BadgeKeyInfo[uc_index].uc_IconPaletteType = st_TopMenuBadgeKeyFuncTbl[( uchar )_sc_select].uc_IconPaletteType;
			st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = st_TopMenuBadgeKeyFuncTbl[( uchar )_sc_select].uc_IconPaletteNotActivate;
// comment 2020/07/17 Sky東島 [IT6_2.0MR3][OP_BTS-25505].
//			st_BadgeKeyInfo[uc_index].ss_FuncIconDataSize = 0;
//			st_BadgeKeyInfo[uc_index].ss_IconNotActivateDataSize = 0;
			st_BadgeKeyInfo[uc_index].us_FuncIconDataSize = 0;
			st_BadgeKeyInfo[uc_index].us_IconNotActivateDataSize = 0;
		} else {
			// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] 標準PNG/独自PNGのCaseを追加.
			switch( PnlComGetStandardPanelSetting()->checkIconFormat( uc_AppIconData ) ) {
				case TYPE_ExtIconFormat_Bmp:	// 24bit 64×80dot Bitmap.
				case TYPE_ExtIconFormat_Png:	// 24bit 64×80dot 標準PNG形式.
					// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] 独自RAWへの変換を削除.標準BMP/標準PNGの共通処理へ修正.
					//	memset( m_ucAppIconData, 0x00, sizeof( m_ucAppIconData ) );		// AppIcon.
					//	PnlComGetRawBmpData( uc_AppIconData , ( char* )m_ucAppIconData, 64, 80 );
					//	st_BadgeKeyInfo[uc_index].bl_BMP = True;
					//	st_BadgeKeyInfo[uc_index].bl_isShowIcon = True;

					memset( m_ucAppIconData, 0x00, sizeof( m_ucAppIconData ) );		// AppIcon.
					memcpy( m_ucAppIconData, uc_AppIconData, sizeof( m_ucAppIconData ) );
					st_BadgeKeyInfo[uc_index].bl_BMP = False;
					st_BadgeKeyInfo[uc_index].bl_isShowIcon = True;
					st_BadgeKeyInfo[uc_index].uc_IconPaletteType = PNLD_PaletteNull;
					st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = PNLD_PaletteNull;
					// GFW層はデータサイズ非0時に、データ内容を標準BMP/標準PNGと判断するため、アイコン画像のデータサイズを設定する.
					// comment 2015/08/26 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] アイコンデータ取得APIにてデータサイズ取得の追加.
					// アイコンデータはshort上限サイズ以下のため、shortへキャスト.
// comment 2020/07/17 Sky東島 [IT6_2.0MR3][OP_BTS-25505].
//					st_BadgeKeyInfo[uc_index].ss_FuncIconDataSize = (short) si_AppIconDataSize;	// 標準BMP/標準PNGならばデータサイズをバッファ長にする.
//					st_BadgeKeyInfo[uc_index].ss_IconNotActivateDataSize = (short) si_AppIconDataSize;
					st_BadgeKeyInfo[uc_index].us_FuncIconDataSize = (ushort) si_AppIconDataSize;	// 標準BMP/標準PNGならばデータサイズをバッファ長にする.
					st_BadgeKeyInfo[uc_index].us_IconNotActivateDataSize = (ushort) si_AppIconDataSize;
					break;

				case TYPE_ExtIconFormat_Other:	// 従来APIから送信されるPNG形式（dat）.
				case TYPE_ExtIconFormat_OriginalPng:	// 従来APIから送信されるKM独自PNG形式（dat）.
					memset( m_ucAppIconData, 0x00, sizeof( m_ucAppIconData ) );		// AppIcon.
					memcpy( m_ucAppIconData, uc_AppIconData, sizeof( m_ucAppIconData ) );
// comment 2015/12/14 OPS1 Sky東島 [ZeusSZX0][CQ521810].
					setBilinearFlag( ( char* )m_ucAppIconData );
					st_BadgeKeyInfo[uc_index].bl_BMP = False;
					st_BadgeKeyInfo[uc_index].bl_isShowIcon = True;
					// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] データ形式毎のパレット/データサイズ設定を追加.
					st_BadgeKeyInfo[uc_index].uc_IconPaletteType = PNLD_PaletteDefault;
					st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = PNLD_PaletteDefault;
					// GFW層はデータサイズ0時に、データ内容をKM独自PNGと判断するため、データサイズ0を設定する.
// comment 2020/07/17 Sky東島 [IT6_2.0MR3][OP_BTS-25505].
//					st_BadgeKeyInfo[uc_index].ss_FuncIconDataSize = 0;
//					st_BadgeKeyInfo[uc_index].ss_IconNotActivateDataSize = 0;
					st_BadgeKeyInfo[uc_index].us_FuncIconDataSize = 0;
					st_BadgeKeyInfo[uc_index].us_IconNotActivateDataSize = 0;
					break;
// comment 2020/6/26 Sky東島 [IT6_2.0MR3][制御構成変更_No.37]SVGサポート対応.
				case TYPE_ExtIconFormat_Svg:		// SVG形式.
					memset( m_ucAppIconData, 0x00, sizeof( m_ucAppIconData ) );		// AppIcon.
					memcpy( m_ucAppIconData, uc_AppIconData, sizeof( m_ucAppIconData ) );
					st_BadgeKeyInfo[uc_index].bl_BMP = False;
					st_BadgeKeyInfo[uc_index].bl_isShowIcon = True;
					st_BadgeKeyInfo[uc_index].uc_IconPaletteType = PNLD_PaletteNull;
					st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = PNLD_PaletteNull;
// comment 2020/07/17 Sky東島 [IT6_2.0MR3][OP_BTS-25505].
//					st_BadgeKeyInfo[uc_index].ss_FuncIconDataSize = (short) si_AppIconDataSize;	// 標準BMP/標準PNGならばデータサイズをバッファ長にする.
//					st_BadgeKeyInfo[uc_index].ss_IconNotActivateDataSize = (short) si_AppIconDataSize;
					st_BadgeKeyInfo[uc_index].us_FuncIconDataSize = (ushort) si_AppIconDataSize;	// 標準BMP/標準PNGならばデータサイズをバッファ長にする.
					st_BadgeKeyInfo[uc_index].us_IconNotActivateDataSize = (ushort) si_AppIconDataSize;
					st_BadgeKeyInfo[uc_index].bl_SvgData = True;
					break;
				case TYPE_ExtIconFormat_BmpError:	// 破損データまたは未サポートデータ.
				default:
					st_BadgeKeyInfo[uc_index].bl_BMP = False;
					st_BadgeKeyInfo[uc_index].bl_isShowIcon = False;
					// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] データ形式毎のパレット/データサイズ設定を追加.
					st_BadgeKeyInfo[uc_index].uc_IconPaletteType = PNLD_PaletteNull;
					st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = PNLD_PaletteNull;
// comment 2020/07/17 Sky東島 [IT6_2.0MR3][OP_BTS-25505].
//					st_BadgeKeyInfo[uc_index].ss_FuncIconDataSize = 0;
//					st_BadgeKeyInfo[uc_index].ss_IconNotActivateDataSize = 0;
					st_BadgeKeyInfo[uc_index].us_FuncIconDataSize = 0;
					st_BadgeKeyInfo[uc_index].us_IconNotActivateDataSize = 0;
					break;
			}

			st_BadgeKeyInfo[uc_index].puc_FuncIcon = m_ucAppIconData;
			st_BadgeKeyInfo[uc_index].puc_IconNotActivate = m_ucAppIconData;
		}

		// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] 共通的なパレット設定処理を削除.キー種別＆データ形式別の処理へ移動.
		//	st_BadgeKeyInfo[uc_index].uc_IconPaletteType = st_TopMenuBadgeKeyFuncTbl[( uchar )_sc_select].uc_IconPaletteType;
		//	st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = st_TopMenuBadgeKeyFuncTbl[( uchar )_sc_select].uc_IconPaletteNotActivate;
		//	// アイコンパレットは1固定で作成される仕様のためパレットを1にする.
		//	if( False == st_BadgeKeyInfo[uc_index].bl_BMP
		//	    && True == st_BadgeKeyInfo[uc_index].bl_isShowIcon
		//	    && True == bl_OapIconExit ) {
		//		st_BadgeKeyInfo[uc_index].uc_IconPaletteType = PNLD_PaletteDefault;
		//		st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = PNLD_PaletteDefault;
		//	}
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： コピー機能情報の設定.
//================================================================================================
// Comment 2016/10/07 [HCLT_Arvind] [1005414] 【IT5_3.3. 】Checking for 10.1 inch
#ifdef DEF_RES_WSVGA
static const PNLS_TopMenuBadgeKeyText st_TopMenuBadgeKeyCopyFuncTbl[] = {
	{ PNLE_TopMenuShortCut_CopyColorMode,		 s_pKEYColor_W100H042E,					( uchar* )i75_000_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i70_000_000_d_X64_Y80, PNLD_PaletteCommon },	// コピー機能設定:カラーモード.
	{ PNLE_TopMenuShortCut_CopyPaperType,		 s_pKEYPaper_W100H042E,					( uchar* )i75_006_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_006_000_d_X81_Y100, PNLD_PaletteCommon },	// コピー機能設定:用紙設定.
	{ PNLE_TopMenuShortCut_CopyMag,				 s_pKEYMagnificationSetting_W100H042E,	( uchar* )i75_003_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_003_000_d_X81_Y100, PNLD_PaletteCommon },	// コピー機能設定:倍率設定.
	{ PNLE_TopMenuShortCut_CopyPage,			 s_pKEYDuplexPageCollection_W100H042E,	( uchar* )i75_001_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_001_000_d_X81_Y100, PNLD_PaletteCommon },	// コピー機能設定:両面/ページ集約.
	{ PNLE_TopMenuShortCut_CopyFinishing,		 s_pKEYFinishing_W100H042E,				( uchar* )i75_002_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_002_000_d_X81_Y100, PNLD_PaletteCommon },	// コピー機能設定:仕上がり.
	{ PNLE_TopMenuShortCut_CopyDensity,			 s_pKEYDensity_W100H042E,				( uchar* )i75_005_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i70_005_000_d_X64_Y80, PNLD_PaletteCommon },	// コピー機能設定:濃度.
	{ PNLE_TopMenuShortCut_CopyOriginalImage,	 s_pKEYOriginalType_W100H042E,			( uchar* )i75_004_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i70_004_000_d_X64_Y80, PNLD_PaletteCommon },	// コピー機能設定:原稿画質.
};
#else // #ifdef DEF_RES_WSVGA.
static const PNLS_TopMenuBadgeKeyText st_TopMenuBadgeKeyCopyFuncTbl[] = {
	{ PNLE_TopMenuShortCut_CopyColorMode,		 s_pKEYColor_W100H042E,	( uchar* )i70_000_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_000_000_d_X64_Y80, PNLD_PaletteCommon },	// コピー機能設定:カラーモード.
	{ PNLE_TopMenuShortCut_CopyPaperType,		 s_pKEYPaper_W100H042E,	( uchar* )i70_006_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_006_000_d_X64_Y80, PNLD_PaletteCommon },	// コピー機能設定:用紙設定.
	{ PNLE_TopMenuShortCut_CopyMag,				 s_pKEYMagnificationSetting_W100H042E,	( uchar* )i70_003_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_003_000_d_X64_Y80, PNLD_PaletteCommon },	// コピー機能設定:倍率設定.
	{ PNLE_TopMenuShortCut_CopyPage,			 s_pKEYDuplexPageCollection_W100H042E,	( uchar* )i70_001_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_001_000_d_X64_Y80, PNLD_PaletteCommon },	// コピー機能設定:両面/ページ集約.
	{ PNLE_TopMenuShortCut_CopyFinishing,		 s_pKEYFinishing_W100H042E,	( uchar* )i70_002_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_002_000_d_X64_Y80, PNLD_PaletteCommon },	// コピー機能設定:仕上がり.
	{ PNLE_TopMenuShortCut_CopyDensity,			 s_pKEYDensity_W100H042E,	( uchar* )i70_005_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_005_000_d_X64_Y80, PNLD_PaletteCommon },	// コピー機能設定:濃度.
	{ PNLE_TopMenuShortCut_CopyOriginalImage,	 s_pKEYOriginalType_W100H042E,	( uchar* )i70_004_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_004_000_d_X64_Y80, PNLD_PaletteCommon },	// コピー機能設定:原稿画質.
};
#endif // #ifdef DEF_RES_WSVGA.
void PCGC_HomeTopMenu::setCopyFunctionInfo( uchar uc_index, TYPE_TopMenuShortCutCopyFunction e_CopyFunctionType )
{
	PNLC_FuncPatternSettingLib* cp_FuncSetting = PNLC_FuncPatternSettingLib::newInstance();
	char _sc_select = -1;
	st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_Copy );
	switch( e_CopyFunctionType ) {
		case TYPE_TopMenuShortCutCopyFunctionColorMode:									// カラーモード.
#ifndef DEF_MonoCus
			if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBit( 13, 5 )		// DipSW No.71 0x02が有効.
			    || False == cp_FuncSetting->isFunctionShowCopyColor() ) {		// またはパターンカスタマイズ非表示.
				st_BadgeKeyInfo[uc_index].bl_isShow = False;	// 表示しない.
			} else {
				_sc_select = 0;
			}
#else
			st_BadgeKeyInfo[uc_index].bl_isShow = False;	// 表示しない.
#endif //DEF_MonoCus.
			break;
		case TYPE_TopMenuShortCutCopyFunctionPaperType:	// 用紙設定.
			_sc_select = 1;
			if( False == cp_FuncSetting->isFunctionShowCopyPaper() ) { 			//パターンカスタマイズ非表示時.
				st_BadgeKeyInfo[uc_index].bl_isShow = False;	// 表示しない.
			}
			break;
		case TYPE_TopMenuShortCutCopyFunctionMag:	// 倍率設定.
			_sc_select = 2;
			if( False == cp_FuncSetting->isFunctionShowCopyZoom() ) { 				//パターンカスタマイズ非表示時.
				st_BadgeKeyInfo[uc_index].bl_isShow = False;	// 表示しない.
			}
			break;
		case TYPE_TopMenuShortCutCopyFunctionPage:	// 両面/ページ集約.
			_sc_select = 3;
			if( False == cp_FuncSetting->isFunctionShowCopyDuplexCombine() ) { 	//パターンカスタマイズ非表示時.
				st_BadgeKeyInfo[uc_index].bl_isShow = False;	// 表示しない.
			}
			break;
		case TYPE_TopMenuShortCutCopyFunctionFinishing:	// 仕上がり.
			_sc_select = 4;
			if( False == cp_FuncSetting->isFunctionShowCopyFinishing() ) { 		//パターンカスタマイズ非表示時.
				st_BadgeKeyInfo[uc_index].bl_isShow = False;	// 表示しない.
			}
			break;
		case TYPE_TopMenuShortCutCopyFunctionDensity:	// 濃度.
			_sc_select = 5;
			if( False == cp_FuncSetting->isFunctionShowCopyDensity() ) { 			//パターンカスタマイズ非表示時.
				st_BadgeKeyInfo[uc_index].bl_isShow = False;	// 表示しない.
			}
			break;
		case TYPE_TopMenuShortCutCopyFunctionOriginalImageMode:	// 原稿画質.
			_sc_select = 6;
			if( False == cp_FuncSetting->isFunctionShowCopyOrgType() ) { 			//パターンカスタマイズ非表示時.
				st_BadgeKeyInfo[uc_index].bl_isShow = False;	// 表示しない.
			}
			break;
		default:	st_BadgeKeyInfo[uc_index].e_ShortCutKind = PNLE_TopMenuShortCut_None;	break;
	}
	if( -1 != _sc_select ) {
		st_BadgeKeyInfo[uc_index].e_ShortCutKind = st_TopMenuBadgeKeyCopyFuncTbl[( uchar )_sc_select].e_ShortCutKind;
		st_BadgeKeyInfo[uc_index].tid_FuncName = st_TopMenuBadgeKeyCopyFuncTbl[( uchar )_sc_select].tid_FuncName;
		st_BadgeKeyInfo[uc_index].puc_FuncIcon = st_TopMenuBadgeKeyCopyFuncTbl[( uchar )_sc_select].puc_FuncIcon;
		st_BadgeKeyInfo[uc_index].uc_IconPaletteType = st_TopMenuBadgeKeyCopyFuncTbl[( uchar )_sc_select].uc_IconPaletteType;;
		st_BadgeKeyInfo[uc_index].puc_IconNotActivate = st_TopMenuBadgeKeyCopyFuncTbl[( uchar )_sc_select].puc_IconNotActivate;
		st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = st_TopMenuBadgeKeyCopyFuncTbl[( uchar )_sc_select].uc_IconPaletteNotActivate;
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： コピー設定メモリ情報の設定.
//================================================================================================
void PCGC_HomeTopMenu::setCopyMemoryInfo( uchar uc_index, short us_JobMemoryNo )
{
	st_BadgeKeyInfo[uc_index].e_ShortCutKind = PNLE_TopMenuShortCut_CopyMemory;
	if( True ==  PnlComGetOperationSetting()->getRegistration( static_cast<uchar>( us_JobMemoryNo ) ) ) {
		PnlComGetOperationSetting()->getJobMemoryNameUTF8( ( uchar )us_JobMemoryNo, st_BadgeKeyInfo[uc_index].sc_FuncName, TYPE_CharSetUTF8 );
		st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_Copy );
	} else {
		st_BadgeKeyInfo[uc_index].bl_isShow = False;
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： スキャン/ファクス機能情報の設定.
//================================================================================================
//Comment 2016/10/07 【HCLT-Tharun】【IT5_3.3】【Accurev ID:1005414】Changed the PNG for 10.1 Inch Models.
#if defined(DEF_RES_WSVGA)
static const PNLS_TopMenuBadgeKeyText st_TopMenuBadgeKeyScanFaxFuncTbl[] = {
	{ PNLE_TopMenuShortCut_ScanFaxDuplexMode,	 s_pKEYSingleDouble_W100H042E,	( uchar* )i75_010_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_010_000_d_X81_Y100, PNLD_PaletteCommon },	// スキャン/ファクス機能設定:片面/両面.
	{ PNLE_TopMenuShortCut_ScanFaxResolution,	 s_pKEYResolution_W100H042E,	( uchar* )i75_011_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_011_000_d_X81_Y100, PNLD_PaletteCommon },	// スキャン/ファクス機能設定:解像度.
	{ PNLE_TopMenuShortCut_ScanFaxColor,		 s_pKEYColor_W100H042E,			( uchar* )i75_008_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i70_008_000_d_X64_Y80, PNLD_PaletteCommon },	// スキャン/ファクス機能設定:カラー.
	{ PNLE_TopMenuShortCut_ScanFaxFileFormat,	 s_pKEYFileType_W100H042E,		( uchar* )i75_009_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_009_000_d_X81_Y100, PNLD_PaletteCommon },	// スキャン/ファクス機能設定:ファイル形式.
	{ PNLE_TopMenuShortCut_ScanFaxReadSize,		 s_pKEYScanSize_W100H042E,		( uchar* )i75_012_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_012_000_d_X81_Y100, PNLD_PaletteCommon },	// スキャン/ファクス機能設定:読込みサイズ.
	{ PNLE_TopMenuShortCut_ScanFaxEMail,		 s_pKEYEmailSettings_W100H042E,	( uchar* )i75_007_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_007_000_d_X81_Y100, PNLD_PaletteCommon },	// スキャン/ファクス機能設定:E-mail設定.
};
#else	// #if defined(DEF_RES_WSVGA).
static const PNLS_TopMenuBadgeKeyText st_TopMenuBadgeKeyScanFaxFuncTbl[] = {
	{ PNLE_TopMenuShortCut_ScanFaxDuplexMode,	 s_pKEYSingleDouble_W100H042E,	( uchar* )i70_010_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_010_000_d_X64_Y80, PNLD_PaletteCommon },	// スキャン/ファクス機能設定:片面/両面.
	{ PNLE_TopMenuShortCut_ScanFaxResolution,	 s_pKEYResolution_W100H042E,	( uchar* )i70_011_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_011_000_d_X64_Y80, PNLD_PaletteCommon },	// スキャン/ファクス機能設定:解像度.
	{ PNLE_TopMenuShortCut_ScanFaxColor,		 s_pKEYColor_W100H042E,	( uchar* )i70_008_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_008_000_d_X64_Y80, PNLD_PaletteCommon },	// スキャン/ファクス機能設定:カラー.
	{ PNLE_TopMenuShortCut_ScanFaxFileFormat,	 s_pKEYFileType_W100H042E,	( uchar* )i70_009_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_009_000_d_X64_Y80, PNLD_PaletteCommon },	// スキャン/ファクス機能設定:ファイル形式.
	{ PNLE_TopMenuShortCut_ScanFaxReadSize,		 s_pKEYScanSize_W100H042E,	( uchar* )i70_012_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_012_000_d_X64_Y80, PNLD_PaletteCommon },	// スキャン/ファクス機能設定:読込みサイズ.
	{ PNLE_TopMenuShortCut_ScanFaxEMail,		 s_pKEYEmailSettings_W100H042E,	( uchar* )i70_007_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_007_000_d_X64_Y80, PNLD_PaletteCommon },	// スキャン/ファクス機能設定:E-mail設定.
};
#endif	// #if defined(DEF_RES_WSVGA).
void PCGC_HomeTopMenu::setScanFaxFunctionInfo( uchar uc_index, TYPE_TopMenuShortCutScanFaxFunction e_ScanFaxFunctionType )
{
	PNLC_FuncPatternSettingLib* cp_FuncSetting = PNLC_FuncPatternSettingLib::newInstance();
	char _sc_select = -1;
	st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_Fax );
	switch( e_ScanFaxFunctionType ) {
		case TYPE_TopMenuShortCutScanFaxFunctionDuplexMode:	// 片面/両面.
			_sc_select = 0;
			if( False == cp_FuncSetting->isFunctionShowFaxScanDuplexSet() ) { 			//パターンカスタマイズ非表示時.
				st_BadgeKeyInfo[uc_index].bl_isShow = False;	// 表示しない.
			}
			break;
		case TYPE_TopMenuShortCutScanFaxFunctionResolution:	// 解像度.
			_sc_select = 1;
			if( False == cp_FuncSetting->isFunctionShowFaxScanResolution() ) { 			//パターンカスタマイズ非表示時.
				st_BadgeKeyInfo[uc_index].bl_isShow = False;	// 表示しない.
			}
			break;
		case TYPE_TopMenuShortCutScanFaxFunctionColor:	// カラー.
			_sc_select = 2;
			if( False == cp_FuncSetting->isFunctionShowFaxScanColor() ) { 			//パターンカスタマイズ非表示時.
				st_BadgeKeyInfo[uc_index].bl_isShow = False;	// 表示しない.
			}
			break;
		case TYPE_TopMenuShortCutScanFaxFunctionFileFormat:	// ファイル形式.
			_sc_select = 3;
			if( False == cp_FuncSetting->isFunctionShowFaxScanFileType() ) { 			//パターンカスタマイズ非表示時.
				st_BadgeKeyInfo[uc_index].bl_isShow = False;	// 表示しない.
			}
			break;
		case TYPE_TopMenuShortCutScanFaxFunctionOriginalReadSize:	// 読込みサイズ.
			_sc_select = 4;
			if( False == cp_FuncSetting->isFunctionShowFaxScanScanSize() ) { 			//パターンカスタマイズ非表示時.
				st_BadgeKeyInfo[uc_index].bl_isShow = False;	// 表示しない.
			}
			break;
		case TYPE_TopMenuShortCutScanFaxFunctionEMail:	// E-mail設定.
			_sc_select = 5;
			if( False == cp_FuncSetting->isFunctionShowFaxScanEmailSetting() ) { 			//パターンカスタマイズ非表示時.
				st_BadgeKeyInfo[uc_index].bl_isShow = False;	// 表示しない.
			}
			break;
		default:	st_BadgeKeyInfo[uc_index].e_ShortCutKind = PNLE_TopMenuShortCut_None;	break;
	}
	if( -1 != _sc_select ) {
		st_BadgeKeyInfo[uc_index].e_ShortCutKind = st_TopMenuBadgeKeyScanFaxFuncTbl[( uchar )_sc_select].e_ShortCutKind;
		st_BadgeKeyInfo[uc_index].tid_FuncName = st_TopMenuBadgeKeyScanFaxFuncTbl[( uchar )_sc_select].tid_FuncName;
		st_BadgeKeyInfo[uc_index].puc_FuncIcon = st_TopMenuBadgeKeyScanFaxFuncTbl[( uchar )_sc_select].puc_FuncIcon;
		st_BadgeKeyInfo[uc_index].uc_IconPaletteType = st_TopMenuBadgeKeyScanFaxFuncTbl[( uchar )_sc_select].uc_IconPaletteType;
		st_BadgeKeyInfo[uc_index].puc_IconNotActivate = st_TopMenuBadgeKeyScanFaxFuncTbl[( uchar )_sc_select].puc_IconNotActivate;
		st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = st_TopMenuBadgeKeyScanFaxFuncTbl[( uchar )_sc_select].uc_IconPaletteNotActivate;;
// comment 2016/02/02 OPS1-Sky善[特ROM][IT5-14_機種マージ]暗号化PDF操作フロー改善要望.
		if((3 == _sc_select) && (True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW218, 7 )) ) {	// ファイル形式+DipSWNo.218 bit7 ON.
			//文言とショートカット種別を変更する.
			st_BadgeKeyInfo[uc_index].e_ShortCutKind = PNLE_TopMenuShortCut_ScanFaxEncryptPDFPassword;		//PDF暗号化ショートカット.
			st_BadgeKeyInfo[uc_index].tid_FuncName = s_pKEYActivityEncryptedPDF_W100H042E;					//[暗号化PDF].
		}
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： スキャン/ファクス設定メモリ情報の設定.
//================================================================================================
void PCGC_HomeTopMenu::setScanFaxMemoryInfo( uchar uc_index, short us_JobMemoryNo )
{
	st_BadgeKeyInfo[uc_index].e_ShortCutKind = PNLE_TopMenuShortCut_ScanFaxMemory;
	TYPC_ProgramKey* pc_ProgramKey = PCGC_TopmenuProgramKeyData::getInstance()->getProgramKeyData( us_JobMemoryNo );
	if( NULL != pc_ProgramKey ) {
		pc_ProgramKey->getProgramKeyNameUTF8( st_BadgeKeyInfo[uc_index].sc_FuncName, TYPE_CharSetUTF8 );
		st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_Fax );
// comment 2020/07/14 Sky中野 [IT6_2.0MR3_SpecialROM][OP_BTS-25456].
		if( True == st_BadgeKeyInfo[uc_index].bl_isShow ) {
			st_BadgeKeyInfo[uc_index].bl_isShow = getShowStatusProgramKey( us_JobMemoryNo );
		}
	} else {
		st_BadgeKeyInfo[uc_index].bl_isShow = False;
// comment 2017/11/21(火) SCD2-Sky長谷川(伸) [FQ-OPSD-1700523-OFP264].
// comment 2018/10/26 SCD2 T.Nishiba [Eagle][AR1064089]プログラム宛先の再検索を一度だけに制限する.
		if( False == pc_ProgramKeyDataGetForTopMenu.getProgramSearchRetryExcuted() ) {
			pc_ProgramKeyDataGetForTopMenu.setNeedProgramSearchFlg( True );		// 取得できないプログラム宛先があった為、再検索要とする.
		}
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： システムボックス情報の設定.
//================================================================================================
//Comment 2016/10/07 【HCLT-Tharun】【IT5_3.3】【Accurev ID:1005414】Changed the PNG for 10.1 Inch Models.
#if defined(DEF_RES_WSVGA)
static const PNLS_TopMenuBadgeKeyText st_TopMenuBadgeKeySystemBoxTbl[] = {
//Comment 2019/03/14 【HCLT-Shalini】【IT6_EagleH】【OP_BTS-4093】Correcting the display of ｾｷｭﾘﾃｨ文書 to ｾｷｭﾘﾃｨ-\文書.
//	{ PNLE_TopMenuShortCut_SysBoxSecurity,		 s_pKEYSecurityDocument_W100H042E,			( uchar* )i75_013_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_013_000_d_X81_Y100, PNLD_PaletteCommon	  },	// システムボックス機能設定:セキュリティー文書.
	{ PNLE_TopMenuShortCut_SysBoxSecurity,		 s_pKEYSecuritySetting_W100H042E,			( uchar* )i75_013_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_013_000_d_X81_Y100, PNLD_PaletteCommon	  },	// システムボックス機能設定:セキュリティー文書.
	{ PNLE_TopMenuShortCut_SysBoxForcedMemory,	 s_pKEYMemoryRXUserBox_W100H042E,			( uchar* )i75_019_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_019_000_d_X81_Y100, PNLD_PaletteCommon	  },	// システムボックス機能設定:強制メモリー受信.
	{ PNLE_TopMenuShortCut_SysBoxAuthPrint,		 s_pKEYAuthenticationAndPrintBox_W100H042E,	( uchar* )i75_024_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_024_000_d_X81_Y100, PNLD_PaletteCommon	  },	// システムボックス機能設定:認証＆プリント.
	{ PNLE_TopMenuShortCut_SysBoxBoard,			 s_pKEYBulletin_W100H042E,					( uchar* )i75_020_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_020_000_d_X81_Y100, PNLD_PaletteCommon	  },	// システムボックス機能設定:掲示板.
	{ PNLE_TopMenuShortCut_SysBoxPolling,		 s_pKEYPollingSend_W100H042E,				( uchar* )i75_016_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_016_000_d_X81_Y100, PNLD_PaletteCommon	  },	// システムボックス機能設定:ポーリング送信.
	{ PNLE_TopMenuShortCut_SysBoxFilingNumber,	 s_pKEYAnnotation_W100H042E,				( uchar* )i75_015_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_015_000_d_X81_Y100, PNLD_PaletteCommon	  },	// システムボックス機能設定:ファイリングナンバー.
	{ PNLE_TopMenuShortCut_SysBoxSendAgain,		 s_pKEYResend_W100H042E,					( uchar* )i75_017_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_017_000_d_X81_Y100, PNLD_PaletteCommon	  },	// システムボックス機能設定:再送信.
	{ PNLE_TopMenuShortCut_SysBoxEncryption,	 s_pKEYPasswordEncryptedPDF_W100H042E,		( uchar* )i75_014_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_014_000_d_X81_Y100, PNLD_PaletteCommon	  },	// システムボックス機能設定:パスワード暗号化PDF.
	{ PNLE_TopMenuShortCut_SysBoxExternalMemory, s_pKEYExternal_W100H042E,					( uchar* )i75_018_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_018_000_d_X81_Y100, PNLD_PaletteCommon	  },	// システムボックス機能設定:外部メモリー.
	{ PNLE_TopMenuShortCut_SysBoxMobilePDA,		 s_pKEYMobileConnection_W100H042E,			( uchar* )i75_021_000_X81_Y100, PNLD_PaletteCommon, ( uchar* )i75_021_000_d_X81_Y100, PNLD_PaletteCommon	  },	// システムボックス機能設定:携帯電話/PDA.
// comment 2016/12/01 SCD2-Sky浜野 [IT5_4.1][AirPrint1.7対応].
	{ PNLE_TopMenuShortCut_SysBoxPinPrint,		 s_pKEYPINPrintDocument_W100H042E,			( uchar* )i70_045_000_n_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_045_000_d_X64_Y80, PNLD_PaletteCommon	  },	// システムボックス機能設定:PINプリント.
};
#else	// #if defined(DEF_RES_WSVGA).
static const PNLS_TopMenuBadgeKeyText st_TopMenuBadgeKeySystemBoxTbl[] = {
//Comment 2019/03/14 【HCLT-Shalini】【IT6_EagleH】【OP_BTS-4093】Correcting the display of ｾｷｭﾘﾃｨ文書 to ｾｷｭﾘﾃｨ-\文書.
//	{ PNLE_TopMenuShortCut_SysBoxSecurity,		 s_pKEYSecurityDocument_W100H042E,	( uchar* )i70_013_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_013_000_d_X64_Y80, PNLD_PaletteCommon	  },	// システムボックス機能設定:セキュリティー文書.
	{ PNLE_TopMenuShortCut_SysBoxSecurity,		 s_pKEYSecuritySetting_W100H042E,	( uchar* )i70_013_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_013_000_d_X64_Y80, PNLD_PaletteCommon	  },	// システムボックス機能設定:セキュリティー文書.
	{ PNLE_TopMenuShortCut_SysBoxForcedMemory,	 s_pKEYMemoryRXUserBox_W100H042E,	( uchar* )i70_019_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_019_000_d_X64_Y80, PNLD_PaletteCommon	  },	// システムボックス機能設定:強制メモリー受信.
	{ PNLE_TopMenuShortCut_SysBoxAuthPrint,		 s_pKEYAuthenticationAndPrintBox_W100H042E,	( uchar* )i70_024_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_024_000_d_X64_Y80, PNLD_PaletteCommon	  },	// システムボックス機能設定:認証＆プリント.
	{ PNLE_TopMenuShortCut_SysBoxBoard,			 s_pKEYBulletin_W100H042E,	( uchar* )i70_020_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_020_000_d_X64_Y80, PNLD_PaletteCommon	  },	// システムボックス機能設定:掲示板.
	{ PNLE_TopMenuShortCut_SysBoxPolling,		 s_pKEYPollingSend_W100H042E,	( uchar* )i70_016_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_016_000_d_X64_Y80, PNLD_PaletteCommon	  },	// システムボックス機能設定:ポーリング送信.
	{ PNLE_TopMenuShortCut_SysBoxFilingNumber,	 s_pKEYAnnotation_W100H042E,	( uchar* )i70_015_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_015_000_d_X64_Y80, PNLD_PaletteCommon	  },	// システムボックス機能設定:ファイリングナンバー.
	{ PNLE_TopMenuShortCut_SysBoxSendAgain,		 s_pKEYResend_W100H042E,	( uchar* )i70_017_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_017_000_d_X64_Y80, PNLD_PaletteCommon	  },	// システムボックス機能設定:再送信.
	{ PNLE_TopMenuShortCut_SysBoxEncryption,	 s_pKEYPasswordEncryptedPDF_W100H042E,	( uchar* )i70_014_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_014_000_d_X64_Y80, PNLD_PaletteCommon	  },	// システムボックス機能設定:パスワード暗号化PDF.
	{ PNLE_TopMenuShortCut_SysBoxExternalMemory, s_pKEYExternal_W100H042E,	( uchar* )i70_018_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_018_000_d_X64_Y80, PNLD_PaletteCommon	  },	// システムボックス機能設定:外部メモリー.
	{ PNLE_TopMenuShortCut_SysBoxMobilePDA,		 s_pKEYMobileConnection_W100H042E,	( uchar* )i70_021_000_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_021_000_d_X64_Y80, PNLD_PaletteCommon	  },	// システムボックス機能設定:携帯電話/PDA.
// comment 2016/12/01 SCD2-Sky浜野 [IT5_4.1][AirPrint1.7対応].
	{ PNLE_TopMenuShortCut_SysBoxPinPrint,		 s_pKEYPINPrintDocument_W100H042E,	( uchar* )i70_045_000_n_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_045_000_d_X64_Y80, PNLD_PaletteCommon	  },	// システムボックス機能設定:PINプリント.
};
#endif	// #if defined(DEF_RES_WSVGA).
void PCGC_HomeTopMenu::setSystemBoxInfo( uchar uc_index, TYPE_TopMenuShortCutSystemBoxFunction e_SystemBoxFunctionType )
{
	char _sc_select = -1;
	switch( e_SystemBoxFunctionType ) {
		case TYPE_TopMenuShortCutSystemBoxFunctionSecurity:			_sc_select = 0;	break;	// セキュリティー文書.
		case TYPE_TopMenuShortCutSystemBoxFunctionForcedMemory:		_sc_select = 1;	break;	// 強制メモリー受信.
		case TYPE_TopMenuShortCutSystemBoxFunctionAuthPrint:		_sc_select = 2;	break;	// 認証＆プリント.
		case TYPE_TopMenuShortCutSystemBoxFunctionBoard:			_sc_select = 3;	break;	// 掲示板.
		case TYPE_TopMenuShortCutSystemBoxFunctionPolling:			_sc_select = 4;	break;	// ポーリング送信.
		case TYPE_TopMenuShortCutSystemBoxFunctionFilingNumber:		_sc_select = 5;	break;	// ファイリングナンバー.
		case TYPE_TopMenuShortCutSystemBoxFunctionSendAgain:		_sc_select = 6;	break;	// 再送信.
		case TYPE_TopMenuShortCutSystemBoxFunctionEncryption:		_sc_select = 7;	break;	// パスワード暗号化PDF.
		case TYPE_TopMenuShortCutSystemBoxFunctionExternalMemory:	_sc_select = 8;	break;	// 外部メモリー.
		case TYPE_TopMenuShortCutSystemBoxFunctionMobilePDA:		_sc_select = 9;	break;	// 携帯電話/PDA.
// comment 2016/12/01 SCD2-Sky浜野 [IT5_4.1][AirPrint1.7対応].
		case TYPE_TopMenuShortCutSystemBoxFunctionPinPrint:			_sc_select = 10;	break;	// PINプリント.
		default:	st_BadgeKeyInfo[uc_index].e_ShortCutKind = PNLE_TopMenuShortCut_None;	break;
	}
	if( -1 != _sc_select ) {
		st_BadgeKeyInfo[uc_index].e_ShortCutKind = st_TopMenuBadgeKeySystemBoxTbl[( uchar )_sc_select].e_ShortCutKind;
		if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW203, 5 ) ) {
			if( 2 == _sc_select ) {
				// 認証＆プリント
				st_BadgeKeyInfo[uc_index].tid_FuncName = s_pKEYActivityPrint_W100H042E;		// プリント.
			} else {
				st_BadgeKeyInfo[uc_index].tid_FuncName = st_TopMenuBadgeKeySystemBoxTbl[( uchar )_sc_select].tid_FuncName;
			}
		} else {
			st_BadgeKeyInfo[uc_index].tid_FuncName = st_TopMenuBadgeKeySystemBoxTbl[( uchar )_sc_select].tid_FuncName;
		}
		st_BadgeKeyInfo[uc_index].puc_FuncIcon = st_TopMenuBadgeKeySystemBoxTbl[( uchar )_sc_select].puc_FuncIcon;
		st_BadgeKeyInfo[uc_index].uc_IconPaletteType = st_TopMenuBadgeKeySystemBoxTbl[( uchar )_sc_select].uc_IconPaletteType;
		st_BadgeKeyInfo[uc_index].puc_IconNotActivate = st_TopMenuBadgeKeySystemBoxTbl[( uchar )_sc_select].puc_IconNotActivate;
		st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = st_TopMenuBadgeKeySystemBoxTbl[( uchar )_sc_select].uc_IconPaletteNotActivate;;
	}
	if( False == isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_BoxOperation ) ) {	// ボックス機能自体が使用不可の場合はシステムボックスのキーは非表示.
		st_BadgeKeyInfo[uc_index].bl_isShow = False;	// 非表示.
// comment 2015.01.14 OPS1-SkyOPS1-Sky西原 [HeliosL][制御構成変更No.36] HDD非装着でUSB利用.
		PNLC_SystemBoxFunctionKeyDispStateLib c_SBLib;	// システムボックス各キーの表示状態判定ライブラリ.
		switch( st_BadgeKeyInfo[uc_index].e_ShortCutKind ){
// Comment 2019/10/09 【HCLT-Krishna】【IT6_HeliosMLK】【制御構成変更_No.43】 SSDレス時の機密文書プリント対応
//// Comment 2018/11/19 【HCLT-Vandana】【IT6_DenebMLK】【制御構成変更_No.83】
//#ifdef DEF_DenebMLKCus
#if defined  (DEF_DenebMLKCus) || defined ( DEF_HeliosMLKCus ) 
			case PNLE_TopMenuShortCut_SysBoxSecurity:
				st_BadgeKeyInfo[uc_index].bl_isShow = c_SBLib.isShowSecureDocumentBox();
				break;
#endif
			case PNLE_TopMenuShortCut_SysBoxExternalMemory:
				if( True == PnlComIsSupportUsbWithoutHDD()){
					st_BadgeKeyInfo[uc_index].bl_isShow = c_SBLib.isShowExternalMemoryBox();
				}
			break;
			default: break;
		}
	} else {
		PNLC_SystemBoxFunctionKeyDispStateLib c_SBLib;	// システムボックス各キーの表示状態判定ライブラリ.
		switch( st_BadgeKeyInfo[uc_index].e_ShortCutKind ) {
			// セキュリティー文書.
			case PNLE_TopMenuShortCut_SysBoxSecurity:		st_BadgeKeyInfo[uc_index].bl_isShow = c_SBLib.isShowSecureDocumentBox();		break;
			// 強制メモリー受信.
			case PNLE_TopMenuShortCut_SysBoxForcedMemory:	st_BadgeKeyInfo[uc_index].bl_isShow = c_SBLib.isShowCompulsoryMemoryRXBox();	break;
			// 認証＆プリント.
			case PNLE_TopMenuShortCut_SysBoxAuthPrint:		st_BadgeKeyInfo[uc_index].bl_isShow = c_SBLib.isShowIDAndPrintBox();			break;
			// 掲示板.
			case PNLE_TopMenuShortCut_SysBoxBoard:			st_BadgeKeyInfo[uc_index].bl_isShow = c_SBLib.isShowBulletinBoardBox();			break;
			// ポーリング送信.
			case PNLE_TopMenuShortCut_SysBoxPolling:		st_BadgeKeyInfo[uc_index].bl_isShow = c_SBLib.isShowPollingTXBox();				break;
			// ファイリングナンバー.
			case PNLE_TopMenuShortCut_SysBoxFilingNumber:	st_BadgeKeyInfo[uc_index].bl_isShow = c_SBLib.isShowAnnotationBox();			break;
			// 再送信.
			case PNLE_TopMenuShortCut_SysBoxSendAgain:		st_BadgeKeyInfo[uc_index].bl_isShow = c_SBLib.isShowRetransmissionBox();		break;
			// パスワード暗号化PDF.
			case PNLE_TopMenuShortCut_SysBoxEncryption:		st_BadgeKeyInfo[uc_index].bl_isShow = c_SBLib.isShowPasswordEncryptedPDFBox();	break;
			// 外部メモリー.
			case PNLE_TopMenuShortCut_SysBoxExternalMemory:	st_BadgeKeyInfo[uc_index].bl_isShow = c_SBLib.isShowExternalMemoryBox();		break;
			// 携帯電話/PDA.
			case PNLE_TopMenuShortCut_SysBoxMobilePDA:		st_BadgeKeyInfo[uc_index].bl_isShow = c_SBLib.isShowMobilePDABox();				break;
// comment 2016/12/01 SCD2-Sky浜野 [IT5_4.1][AirPrint1.7対応].
			// PINプリント.
			case PNLE_TopMenuShortCut_SysBoxPinPrint:		st_BadgeKeyInfo[uc_index].bl_isShow = c_SBLib.isShowPinPrintDocumentBox();		break;
			default:	break;
		}
	}
	// 指定アプリMFP基本機能設定禁止かつシステムボックス各キーの場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		st_BadgeKeyInfo[uc_index].bl_isShow = False;
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： 登録アプリケーション情報の設定.
//================================================================================================
// Comment 2016/10/07 [HCLT_Arvind] [1005414] 【IT5_3.3】 Addition of part for 10.1 inch screen.
#ifdef DEF_RES_WSVGA
	static const PNLS_TopMenuBadgeKeyText st_TopMenuBadgeKeyAppFuncTbl[] = {
	{ PNLE_TopMenuShortCut_AppShortCutOap,		NULL,	( uchar* )i75_035_000_n_X81_Y100,   PNLD_PaletteCommon,	( uchar* )i75_035_000_d_X81_Y100,   PNLD_PaletteCommon	},	// OpenAPIアプリショートカット:OAPアプリ.
	{ PNLE_TopMenuShortCut_AppShortCutHanyo,	NULL,	( uchar* )i75_035_000_n_X81_Y100,   PNLD_PaletteCommon,	( uchar* )i75_035_000_d_X81_Y100,   PNLD_PaletteCommon	},	// OpenAPIアプリショートカット:汎用アプリ.
	{ PNLE_TopMenuShortCut_AppListShortCut,		NULL, 								NULL,   				 0,								NULL,					 0	},	// 機能:OpenAPIアプリ.
};
#else // #if defined(DEF_RES_WSVGA).
static const PNLS_TopMenuBadgeKeyText st_TopMenuBadgeKeyAppFuncTbl[] = {
	{ PNLE_TopMenuShortCut_AppShortCutOap,		NULL,	( uchar* )i70_035_000_n_X64_Y80,   PNLD_PaletteCommon,	( uchar* )i70_035_000_d_X64_Y80,   PNLD_PaletteCommon	},	// OpenAPIアプリショートカット:OAPアプリ.
	{ PNLE_TopMenuShortCut_AppShortCutHanyo,	NULL,	( uchar* )i70_035_000_n_X64_Y80,   PNLD_PaletteCommon,	( uchar* )i70_035_000_d_X64_Y80,   PNLD_PaletteCommon	},	// OpenAPIアプリショートカット:汎用アプリ.
	{ PNLE_TopMenuShortCut_AppListShortCut,		NULL, 	NULL,   					   0,					NULL,						   0					},	// 機能:OpenAPIアプリ.
};
#endif // #if defined(DEF_RES_WSVGA).
void PCGC_HomeTopMenu::setAppFunctionInfo( uchar uc_index, TYPE_TopMenuShortCutType e_ShortCutType, char* OAPAppNameUTF8, unsigned short AppBtnIcon )
{
	char _sc_select = -1;
	// アプリリスト名取得/設定用.
	char sc_AppListName[PCGD_ExtMaxAplNameUTF8];
	memset( sc_AppListName, 0, sizeof( sc_AppListName ) );
	// PKI認証設定時かつセキュリティー強化モードが有効の場合.
	if( ( True == PLBC_FuncPKICheck::isPKI() ) && ( True == PnlComGetSystemSetting()->getSecurityLevelUpOn() ) ) {
		;
	} else if( TYPE_TopMenuShortCutTypeAppShortCut == e_ShortCutType ) {		// OpenAPIアプリキーですか？.
// comment 2019/04/13 【HCL-Prem】【IT6_1.0MR】【AR1075599】 Adding case for IWS APP Type.
		ushort us_no = 0;
		PNLE_PCGC_HomeTopMenu_AppType e_AppType = getAppInfoFromName( OAPAppNameUTF8, us_no );
		
		if ( PNLD_PCGC_HomeTopMenu_AppTypeIws == e_AppType) {
			st_BadgeKeyInfo[ uc_index ].bl_isShow = True;		// ショートカットキー表示.
			st_BadgeKeyInfo[ uc_index ].bl_isActivate = True;	// ショートカットキー活性化状態.
			strncpy( st_BadgeKeyInfo[uc_index].sc_FuncName, OAPAppNameUTF8, sizeof( st_BadgeKeyInfo[uc_index].sc_FuncName )-1 ); // アプリ名設定.
		} else if( False != checkShortCutDisableOap( e_ShortCutType, OAPAppNameUTF8 ) ) {    		// ショートカットキー機能制限チェック.
//		if( False != checkShortCutDisableOap( e_ShortCutType, OAPAppNameUTF8 ) ) {    		// ショートカットキー機能制限チェック.
			// アプリ情報取得.
// comment 2019/04/13 【HCL-Prem】【IT6_1.0MR】【AR1075599】 Adding case for IWS APP Type.
//			ushort us_no = 0;
//			PNLE_PCGC_HomeTopMenu_AppType e_AppType = getAppInfoFromName( OAPAppNameUTF8, us_no );
			if( PNLD_PCGC_HomeTopMenu_AppTypeOap == e_AppType ) {
				// OAPアプリの場合.
				_sc_select = 0;
				us_OAPAppNo[ uc_index ] = us_no;
			} else if( PNLD_PCGC_HomeTopMenu_AppTypeHanyo == e_AppType ) {
				// 汎用アプリの場合.
				_sc_select = 1;
				us_OAPAppNo[ uc_index ] = 0xFFFF;
			} else {
				// 上記以外の場合.
				st_BadgeKeyInfo[ uc_index ].bl_isShow = False;	// ショートカットキー非表示.
				return;
			}
			st_BadgeKeyInfo[ uc_index ].bl_isShow = True;		// ショートカットキー表示.
			st_BadgeKeyInfo[ uc_index ].bl_isActivate = True;	// ショートカットキー活性化状態.
			strncpy( st_BadgeKeyInfo[uc_index].sc_FuncName, OAPAppNameUTF8, sizeof( st_BadgeKeyInfo[uc_index].sc_FuncName )-1 ); // アプリ名設定.
			if( 0 == us_no ) {		// アプリ番号取得不可ですか？.
				st_BadgeKeyInfo[ uc_index ].bl_isShow = False;	// ショートカットキー非表示.
			}
		} else {
			st_BadgeKeyInfo[ uc_index ].bl_isShow = False;		// ショートカットキー非表示.
		}
	} else {		// OpenAPIアプリリスト表示キーですか.
		if( False != checkShortCutDisableOap( e_ShortCutType, NULL ) ) {    					// ショートカットキー機能制限チェック.
			_sc_select = 2;
			st_BadgeKeyInfo[ uc_index ].bl_isShow = True;		// ショートカットキー表示.
			st_BadgeKeyInfo[ uc_index ].bl_isActivate = True;	// ショートカットキー活性化状態.
			st_BadgeKeyInfo[uc_index].puc_FuncIcon = ( uchar* )puc_ProgramIcon[AppBtnIcon].puc_Parts; 	// アイコン設定.
			st_BadgeKeyInfo[uc_index].uc_IconPaletteType = puc_ProgramIcon[AppBtnIcon].uc_PaletteType;	// パレット設定.
			st_BadgeKeyInfo[uc_index].puc_IconNotActivate = ( uchar* )puc_ProgramIcon[PNLD_TopMenuIconTypeMax+AppBtnIcon].puc_Parts;
			st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = puc_ProgramIcon[PNLD_TopMenuIconTypeMax+AppBtnIcon].uc_PaletteType;
			AplCommonSet.getAppBtnName( sc_AppListName );		// アプリリスト名取得.
			strncpy( st_BadgeKeyInfo[uc_index].sc_FuncName, sc_AppListName, sizeof( st_BadgeKeyInfo[uc_index].sc_FuncName ) - 1 );		// アプリリスト名設定.
			st_BadgeKeyInfo[uc_index].sc_FuncName[sizeof( st_BadgeKeyInfo[uc_index].sc_FuncName ) - 1] = '\0';
		} else {
			st_BadgeKeyInfo[ uc_index ].bl_isShow = False;		// ショートカットキー非表示.
		}
	}
	if( -1 != _sc_select ) {
		st_BadgeKeyInfo[uc_index].e_ShortCutKind = st_TopMenuBadgeKeyAppFuncTbl[( uchar )_sc_select].e_ShortCutKind;
		st_BadgeKeyInfo[uc_index].tid_FuncName = st_TopMenuBadgeKeyAppFuncTbl[( uchar )_sc_select].tid_FuncName;
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： 登録アプリケーション機能制限チェック.
//================================================================================================
Bool PCGC_HomeTopMenu::checkShortCutDisableOap( TYPE_TopMenuShortCutType e_TopMenuShortCutType, char* pc_NameSrc )
{
	Bool b_ret = False;
	if( TYPE_TopMenuShortCutTypeAppShortCut == e_TopMenuShortCutType ) {		// OAPアプリのショートカット有効チェック.
		// アプリ情報取得.
		ushort us_AppNo = 0;
		PNLE_PCGC_HomeTopMenu_AppType e_AppType = getAppInfoFromName( pc_NameSrc, us_AppNo );
// Comment 2019/01/30 [HCLT_Neelima] [AR_1071278] 【IT6_DenebMLK】Enabling Market Place Application.
				Bool b_IsMarketPlaceApplication = False;
				char sc_AppName[TYPD_ExtMaxAplName*3+1] = PNLD_MarketPlaceString;
		switch( e_AppType ) {
			case PNLD_PCGC_HomeTopMenu_AppTypeOap:		// OAPアプリの場合.
				TYPE_ExtType e_ExtType;
				PnlComGetAppTypeFromName( pc_NameSrc, e_ExtType );
// Comment 2019/01/30 [HCLT_Neelima] [AR_1071278] 【IT6_DenebMLK】Enabling Market Place Application.
//				b_ret = ScanOapExtSet.checkUserLimitToApl( e_ExtType, us_AppNo );
				if ( 0 == strcmp( sc_AppName, pc_NameSrc ) ) { 
					b_IsMarketPlaceApplication = True;
				}
// comment 2019/04/16 SCD2-Sky中野 [EagleH] 目的別UI_1ペイン対応.
//				b_ret = ScanOapExtSet.checkUserLimitToApl( e_ExtType, us_AppNo, True, b_IsMarketPlaceApplication  );
				if( TYPE_ExtTypeCustomUI == e_ExtType ) {
					// 目的別UIアプリは表示対象外.
					b_ret = False;
				} else {
					b_ret = ScanOapExtSet.checkUserLimitToApl( e_ExtType, us_AppNo, True, b_IsMarketPlaceApplication  );
				}
//Prem_End_1071278
				break;
			case PNLD_PCGC_HomeTopMenu_AppTypeHanyo:	// 汎用アプリの場合.
				b_ret = ExtSet.checkUsedAppSelect( us_AppNo );
				break;
// comment 2019/04/13 【HCL-Prem】【IT6_1.0MR】【AR1075599】 Adding case for IWS APP Type.
			case PNLD_PCGC_HomeTopMenu_AppTypeIws:		// IWS App type.
				b_ret = True;
				break;
			default:
				break;
		}
	} else if( ( TYPE_TopMenuShortCutTypeAppListShortCut == e_TopMenuShortCutType || TYPE_TopMenuShortCutFunctionApp == e_TopMenuShortCutType )
	           && ( True == ScanOapExtSet.isRegistApplication() || 0 < ExtSet.countApps() ) ) { 		// アプリリストに登録チェック.
		// OAPアプリ.
// comment 2019/04/13 【HCL-Prem】【IT6_1.0MR】【AR1075599】 Passing applicatio name as input.
		Bool b_IsMarketPlaceApplication = False;
		char sc_AppName[TYPD_ExtMaxAplName*3+1] = PNLD_MarketPlaceString;
		int AppNoOap;
		for( AppNoOap = TYPD_ExtMaxApplication; AppNoOap >= 1; AppNoOap-- ) {
			// アプリケーション毎にインスタンス生成.
			APIC_ExtSetting* aplOap = APIC_ExtSetting::newInstance( AppNoOap );
			if( NULL == aplOap ) {  // インスタンスNULLチェック.
				continue;
			}
// comment 2019/04/16 SCD2-Sky中野 [EagleH] 目的別UI_1ペイン対応.
			if( TYPE_ExtTypeCustomUI == aplOap->getApplication().s_Version.e_Type ) {
				// 目的別UIアプリは表示対象外.
				continue;
			}
// comment 2019/04/13 【HCL-Prem】【IT6_1.0MR】【AR1075599】 Passing applicatio name as input.
			if ( 0 == strcmp( sc_AppName, aplOap ->getApplication().sc_NameUTF8 ) ) { 
				b_IsMarketPlaceApplication = True;
			}

			if( ( TYPE_ExtEntryOn == aplOap->getApplication().e_Entry ) &&
// comment 2019/08/30 SCD2-Sky肥後 [EagleH][OP_BTS-12687] MarketPlaceからインストールするアプリが表示されない.
//// comment 2019/04/13 【HCL-Prem】【IT6_1.0MR】【AR1075599】 Passing applicatio name as input.
////			    ( PCGC_ScanOapCom::checkUserLimitToApl( aplOap->getApplication().s_Version.e_Type, (ushort)AppNoOap ) ) ) {
//			    ( PCGC_ScanOapCom::checkUserLimitToApl( aplOap->getApplication().s_Version.e_Type, (ushort)AppNoOap, b_IsMarketPlaceApplication ) ) ) {
			    ( PCGC_ScanOapCom::checkUserLimitToApl( aplOap->getApplication().s_Version.e_Type, (ushort)AppNoOap, True, b_IsMarketPlaceApplication ) ) ) {
				// １つでも制限がかかっていないものがあればアプリリスト表示OK.
				b_ret = True;
				break;
			}
		}
		// 汎用アプリ.
		if( 0 < ExtSet.countApps() ) {
			if( True == isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_Fax ) ) {
				b_ret = True;
			}
		}
	}
	// 指定アプリMFP基本機能設定動作禁止時は、非表示.
// comment 2019/04/22 Sky東島 [IT6_1.0MR2][AR1075688].
//	if( True == b_ret ) {
	if( True == b_ret
	 && TYPE_TopMenuShortCutFunctionApp != e_TopMenuShortCutType ){		// APPキー以外.
		b_ret = c_ExtPanel.isEnableMFPFunction();
	}
	return b_ret;		// 戻り値処理.
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： 登録アプリケーションタイプ取得(OAP or 汎用).
//================================================================================================
PNLE_PCGC_HomeTopMenu_AppType PCGC_HomeTopMenu::getAppInfoFromName( const char* puc_AppName, ushort& pus_AppNo )
{
	PNLE_PCGC_HomeTopMenu_AppType e_AppType = PNLD_PCGC_HomeTopMenu_AppTypeNone;

	// Nullチェック.
	if( NULL == puc_AppName )	{
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return e_AppType;
	}

	for( ushort us_cntOap=1; us_cntOap <= TYPD_ExtMaxApplication; us_cntOap++ ) {
		// インスタンス取得.
		APIC_ExtSetting* aplOap = APIC_ExtSetting::newInstance( us_cntOap );
		if( NULL != aplOap ) {
			if( TYPE_ExtEntryOn == aplOap->getApplication().e_Entry ) {
				// アプリ名取得.
				char c_OapAppName[TYPD_ExtMaxAplName*3+1] = {0,};
				PnlComGetUTF8Str( aplOap->getApplication().sc_NameUTF8, ( LIBE_CharSet )getCharSetForCAPI(), c_OapAppName, sizeof( c_OapAppName ) );
				if( 0 == strcmp( puc_AppName, c_OapAppName ) ) {
					// 取得したアプリ名と引数のアプリ名が同じならアプリタイプを返す.
					e_AppType = PNLD_PCGC_HomeTopMenu_AppTypeOap;
					pus_AppNo = us_cntOap;
					return e_AppType;
				}
			}
		}
	}

	for( ushort us_cntHanyo=1; us_cntHanyo <= PNLD_MaxScansendRegistApp; us_cntHanyo++ ) {
		// インスタンス取得.
		APIC_AppInfoSetting* aplHanyo = APIC_AppInfoSetting::newInstance( us_cntHanyo );
		if( NULL != aplHanyo ) {
			if( True == aplHanyo->getAppBasic().b_Entry ) {
				// アプリ名取得.
				char c_HanyoAppName[96+1] = {0,};
				PnlComGetUTF8Str( aplHanyo->getAppBasic().ss_NameUTF8, ( LIBE_CharSet )getCharSetForCAPI(), c_HanyoAppName, sizeof( c_HanyoAppName ) );
				if( 0 == strcmp( puc_AppName, c_HanyoAppName ) ) {
					// 取得したアプリ名と引数のアプリ名が同じならアプリタイプを返す.
					e_AppType = PNLD_PCGC_HomeTopMenu_AppTypeHanyo;
					pus_AppNo = us_cntHanyo;
					return e_AppType;
				}
			}
		}
	}
	std::string IwsAppName;
	const char* c_IwsAppName;
	short ps_appId = -1;
	for( ushort us_cntIws=0; us_cntIws < s_registerAppIws.size(); us_cntIws++ )
	{
		// アプリ名取得.
		s_registerAppIws[us_cntIws].getAppName( &IwsAppName );
		s_registerAppIws[us_cntIws].getAppId( &ps_appId );
		c_IwsAppName = IwsAppName.c_str();
		if( 0 == strcmp( puc_AppName, c_IwsAppName ) ) {
			// 取得したアプリ名と引数のアプリ名が同じならアプリタイプを返す.
			e_AppType = PNLD_PCGC_HomeTopMenu_AppTypeIws;
			pus_AppNo = ps_appId;
			return e_AppType;
		}
	}

	return e_AppType;		// 戻り値処理.
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： 機能毎の表示チェック.
// 変更履歴：2015/01/29 古川(日立JTE)	A3/A4共通化FUM対応 HDDレスFAX対応.
//			 HDD非装着時にFAXの応用機能オプションが「有効」の場合はボックス機能キーを表示する.
// 変更履歴：2018/01/17 sk-nakamura_nt 【制御構成変更No.204】HDDレスFAX対応 for SparrowAIO.
//================================================================================================
Bool PCGC_HomeTopMenu::isFuncShow( PNLE_ACCOUNT_STATUS_TYPE e_AccountStatusType )
{
	Bool _bl_ret = True;
// comment 2019/05/07 Sky薗﨑 [EagleH][OP_BTS-4950].
//Comment 2019/06/24 【HCLT-Neelima】 【IT6_SparrowBK】 【制御構成変更_No.06】 【WP-204】Adding Machine types for SparrowBK models.
//	TYPE_MachineType e_MachineType = PnlComGetSystemInfo()->getMachineType();	// 機種情報取得.

	if( PNLE_ACCOUNT_STATUS_TYPE_Copy == e_AccountStatusType ) {
		// ユーザー機能制限で使用禁止のアプリケーションキー非表示.
		if( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Copy ) ) {
			// コピー機能使用禁止.
			_bl_ret = False;
		}
// comment 2019/05/07 Sky薗﨑 [EagleH][OP_BTS-4950].
//Comment 2019/06/24 【HCLT-Neelima】 【IT6_SparrowBK】 【制御構成変更_No.06】 【WP-204】Adding Machine types for SparrowBK models.
//		if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
		if ( IsSFP() ) {
			// コピー機能使用禁止.
			_bl_ret = False;
		}
	}
	if( PNLE_ACCOUNT_STATUS_TYPE_Fax == e_AccountStatusType ) {
// comment [2017/01/09] [HCLT_Narsimhmaswamy][AR1033152_Reopen] For Network2 As Fax board is not applicable, checking only scan disable condition.
//		if( False == onlyPanel.isFaxModekind() ) {
		if( False == onlyPanel.isFaxModekind() || TYPE_NetworkSpecific_NW2 == PNLC_NetworkLan::getInstance()->getLine() ) {
			if( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Scan ) ) {
				// Scan機能使用禁止.
				_bl_ret = False;
			}
		} else {										// Fax基盤装着時はスキャン/ファクス機能制限両方チェック.
			if( ( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Fax ) )
			    && ( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Scan ) ) ) {
				// Fax機能使用禁止、Scan機能使用禁止.
				_bl_ret = False;
			}
		}
// comment 2019/05/07 Sky薗﨑 [EagleH][OP_BTS-4950].
//Comment 2019/06/24 【HCLT-Neelima】 【IT6_SparrowBK】 【制御構成変更_No.06】 【WP-204】Adding Machine types for SparrowBK models.
//		if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
		if ( IsSFP() ) {
			// Fax機能使用禁止、Scan機能使用禁止.
			_bl_ret = False;
		}
	}
	if( PNLE_ACCOUNT_STATUS_TYPE_BoxOperation == e_AccountStatusType ) {
// Comment 2019/10/09 【HCLT-Krishna】【IT6_HeliosMLK】【制御構成変更_No.43】 SSDレス時の機密文書プリント対応
//// Comment 2018/12/19 【HCLT-Vandana】【IT6_DenebMLK】【AR 1070436】【Hide User box in SSD less and Micro SD installed environment】
//#ifdef DEF_DenebMLKCus
#if defined  (DEF_DenebMLKCus) || defined ( DEF_HeliosMLKCus )
 		if ( True == PnlComIsSecurityDoc() ) {
   			_bl_ret = False; 
 		 } 
#endif
		if( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_BoxOperation )
		    || False == PnlComGetHddInfo()->getHddExist() ) {
			// BOX機能使用禁止.
			_bl_ret = False;
		}
#ifdef DEF_SparrowAIOSFPCus
		if( TYPE_FuncLimit_Disable != PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_BoxOperation )   // BOX機能使用可能なら.
			&& True == onlyPanel.isFaxModekind() && TYPE_MachineTypeSparrow2AIO == PnlComGetSystemInfo()->getMachineType() ) {
			// HDD装着有無にかかわらず、Fax基板装着、マシンタイプがSparrow-AIOの場合、BOX機能使用可能.
			_bl_ret = True;
		}
#endif //DEF_SparrowAIOSFPCus.
	}
	if( PNLE_ACCOUNT_STATUS_TYPE_WebBrowser == e_AccountStatusType ) {
		if( ( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_WebBrowser ) )
		    || ( ( True == PLBC_FuncPKICheck::isPKI() ) && ( True == PnlComGetSystemSetting()->getSecurityLevelUpOn() ) ) ) {
			// ブラウザ使用禁止.
			_bl_ret = False;
		}
		if( PNLE_AppOptionStatus_Enable != PnlComGetEnableAppOptionStatus( TYPE_AppOptionBrowser ) ) {
			_bl_ret = False;
		}
	}
	
// comment 2016/10/07 SCD2-Sky佐藤 [IT5_3.3][1005928] ジョブ飛び越し対応.
	// Panel Linkスキャンの場合、割り込みモード中は非表示とする.
	if( PNLE_ACCOUNT_STATUS_TYPE_ScanToDevice == e_AccountStatusType ) {
		if( EC_IntMode_Off != onlyPanel.ec_IntMode ) {
			_bl_ret = False;
		}
// comment 2020/11/17 2DWS2G-Sky肥後 クラウドSSO認証開発v1_Step2.
		APIC_User* pc_User = PnlComGetUser();
		if( pc_User != NULL ) {
			if ( True == pc_User->isIdpCacheUserNo( pc_User->getUserNo() ) ) {
				_bl_ret = False;
			}
		}
	}
	
	if( True == PnlComIsNeedUserChangeFunction() ) {	// 認証画面を表示する場合.
		// 機能使用禁止でも各機能キーを表示する.
		_bl_ret = True;
	}

	// 統合ベンダー、統合管理装置装着時、管理機能設定で禁止のアプリケーションキー非表示.
	if( ( TYPE_KeyModeVendorKM == APIC_KeyDevice::getKeyMode() ) || ( TYPE_KeyModeAdminMachineKM == APIC_KeyDevice::getKeyMode() ) ) {
		if( ( PNLE_ACCOUNT_STATUS_TYPE_Copy == e_AccountStatusType )
		    && ( TYPE_AppAtAdminMachineVendorDisable == PnlComGetSystemSetting()->getCopyAtAdminMachineVendor() ) ) {
			// コピー使用禁止.
			_bl_ret = False;
		}
		if( ( PNLE_ACCOUNT_STATUS_TYPE_Fax == e_AccountStatusType )
		    && ( TYPE_AppAtAdminMachineVendorDisable == PnlComGetSystemSetting()->getSendAtAdminMachineVendor() ) ) {
			// 送信使用禁止.
			_bl_ret = False;
		}
	}

	// 指定アプリMFP基本機能設定禁止の場合.
// comment 2019/04/22 Sky東島 [IT6_1.0MR2][AR1075688].
//	if( False == c_ExtPanel.isEnableMFPFunction() ) {
	if( False == c_ExtPanel.isEnableMFPFunction()
	 && PNLE_ACCOUNT_STATUS_TYPE_MAX != e_AccountStatusType ) {		// クラシックスタイルキー、アプリキー以外.
		_bl_ret = False;
	}

	return _bl_ret;
}

// comment 2015/10/20 OPS1-川手 [特ROM対応][FQ-OPSD-1500259-OFP185]Fax受信時に受信エラー(R27)が頻繁に発生する.
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： FuncEnableの値のキャッシュ開始処理.
// 変更履歴：2015/10/17 OPS1 Sky川手 新規作成.
//================================================================================================
void PCGC_HomeTopMenu::startCacheFuncEnable()
{
	m_bl_CacheFuncEnable = True;
	uchar uc_loop = 0;
	for (uc_loop = 0; uc_loop < PNLE_ACCOUNT_STATUS_TYPE_MAX; uc_loop++) {
		st_IsFuncEnableCacheTbl[uc_loop].bl_Cashed = False;
		st_IsFuncEnableCacheTbl[uc_loop].bl_Value = False;
	}
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： FuncEnableの値のキャッシュ終了処理.
// 変更履歴：2015/10/17 OPS1 Sky川手 新規作成.
//================================================================================================
void PCGC_HomeTopMenu::endCacheFuncEnable()
{
	m_bl_CacheFuncEnable = False;
}

// -----------------------------------------------------------------------------------------------
/**
 *	@brief	A072002000_トップメニュー（統合トップメニュー） ： ファクス専用の表示チェック.
 *	@date
 *		2014/09/17 OPS1-Sky浜野 [ZeusS][UV-01439] ファクスアプリの新設対応.
 */
// -----------------------------------------------------------------------------------------------
Bool PCGC_HomeTopMenu::isFuncShowFaxOnly()
{
	Bool bl_Ret = True;

	if( False == onlyPanel.isFaxExist() ) {
		bl_Ret = False;
	}

	if( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Fax ) ) {
		// Fax機能使用禁止、Scan機能使用禁止.
		bl_Ret = False;
	}

	if( True == PnlComIsNeedUserChangeFunction() ) {	// 認証画面を表示する場合.
		// 機能使用禁止でも各機能キーを表示する.
		bl_Ret = True;
	}

	// 統合ベンダー、統合管理装置装着時、管理機能設定で禁止のアプリケーションキー非表示.
	if( ( TYPE_KeyModeVendorKM == APIC_KeyDevice::getKeyMode() ) || ( TYPE_KeyModeAdminMachineKM == APIC_KeyDevice::getKeyMode() ) ) {
		if( TYPE_AppAtAdminMachineVendorDisable == PnlComGetSystemSetting()->getSendAtAdminMachineVendor() ) {
			// 送信使用禁止.
			bl_Ret = False;
		}
	}

	// 指定アプリMFP基本機能設定禁止の場合.
	if( False == c_ExtPanel.isEnableMFPFunction() ) {
		bl_Ret = False;
	}

	// Prefix/Suffix設定が「使用する」場合.
	if( True == onlyPanel.isFaxServerEnable() ) {
		bl_Ret = False;
	}

// comment 2015/10/13 OPS1-Sky東島 [MiSBK][CQ514960].
	// FAX送信禁止[ON](False:FAX送信禁止).
	if ( False == PnlComGetScanJobSetting()->getEnableFaxSend() ) {
		bl_Ret = False;
	}
	
// comment 2018/06/05 SCD2-Sky鳫林 「送信承認パスワード入力」がONのとき、FAXを使わせないようにする 対応.
	if ( ( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() )
	&& ( TYPE_SendApprovalPasswordFuncOn == PnlComGetFaxPanelSetting()->getSendApprovalPasswordFunc() ) ) {
		bl_Ret = False;
	}

	return bl_Ret;
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： アプリケーション選択チェック.
//			 2015/10/17 OPS1 Sky川手 実行処理を別関数化( @see isFuncEnable_Exe ).
//================================================================================================
Bool PCGC_HomeTopMenu::isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE e_AccountStatusType )
{
	Bool bl_RetValue = False;
	
//comment 2019/12/16 HCLT-Shalini [EagleBK] [OP_BTS-16194] Gray-out condition when Account Track is ON.
//// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//	if ( ( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() ) 
//	&& ( NULL != PnlComGetUser() )  ) {
//		return True;
//	}
// comment 2020/01/31 Sky東島 [EagleZ+][OP_BTS-19364].
	if ( False == PNLC_PopupListControl::newInstance()->isPopupLayer( this ) ) {
		// 本画面表示中以外にコールされる場合は判定不要.
		// ※HOME画面表示中にキー押下時にコールされるが、その場合はPCGC_HomeTopMenuのキーアクティブ状態判断は不要であるため.
		return True;
	}
	if( NULL == PnlComGetUser()) {
		return False;
	}
// comment 2021/04/30 Sky東島 [IT6_2.2][OP_BTS-32561].
//	if( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance()->getLine()) ) {
//		if( NULL == PnlComGetTrack() ) {					
	if( True == PnlComGetNeedToTrackLogin() ) {
			return False;
//		}
	}
	// キャッシュする場合.
	if (True == m_bl_CacheFuncEnable) {
		if (PNLE_ACCOUNT_STATUS_TYPE_MAX > e_AccountStatusType) {
			if (True == st_IsFuncEnableCacheTbl[e_AccountStatusType].bl_Cashed) {		// キャッシュ済みの場合.
				bl_RetValue = st_IsFuncEnableCacheTbl[e_AccountStatusType].bl_Value;	// キャッシュした値を返す.
			} else {
				bl_RetValue = isFuncEnable_Exe(e_AccountStatusType);					// 値を取得する.
				st_IsFuncEnableCacheTbl[e_AccountStatusType].bl_Value = bl_RetValue;	// キャッシュする.
				st_IsFuncEnableCacheTbl[e_AccountStatusType].bl_Cashed = True;			// キャッシュした.
			}
		} else {
			bl_RetValue = isFuncEnable_Exe(e_AccountStatusType);						// 従来通りチェック処理を行う.
		}
	// キャッシュしない場合.
	} else {
		bl_RetValue = isFuncEnable_Exe(e_AccountStatusType);							// 従来通りチェック処理を行う.
	}
	return bl_RetValue;
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： isFuncEnableの実行処理.
// 変更履歴：2015/10/17 OPS1 Sky川手 新規作成.
//================================================================================================
Bool PCGC_HomeTopMenu::isFuncEnable_Exe( PNLE_ACCOUNT_STATUS_TYPE e_AccountStatusType )
{
	if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBit( 34, 5 ) ) {
		if( True == PnlComGetSystemSetting()->getLongPicturePrint() ) {
			return False;
		}
	}

	// 未認証の場合、アプリケーションキー、ショートカットキーはグレイアウト.
//	if( NULL == PnlComGetUser() )	{ return False; }
	if( False == onlyPanel.bl_IsFieryKey && NULL == PnlComGetUser() )	{ return False; }
// comment 2021/04/30 Sky東島 [IT6_2.2][OP_BTS-32561].
//	if( ( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance()->getLine()) ) &&
//	    ( NULL == PnlComGetTrack() ) 									 &&
	 if ( ( True == PnlComGetNeedToTrackLogin() ) && 		
// comment 2016/02/29 Sky竹内 [特ROM][FQ-OPSD-1500227-OFP162] ユーザー認証時、ログインしないとトップメニューのFieryアイコンを押しても反応しない.
		( False == onlyPanel.bl_IsFieryKey ) &&			// Fieryキー押下時は無効化しない.
	    ( TYPE_UserAuthenticationIntermeddiateServer != PnlComGetLoginUserAuthenticationType() ) ) {
		return False;
	}
	
// comment 2016/10/07 SCD2-Sky佐藤 [IT5_3.3][1005928] ジョブ飛び越し対応.
	// 割込みモードの場合は、コピー以外はグレーアウト.
	if( ( EC_IntMode_Off != onlyPanel.ec_IntMode ) 
	&&	( PNLE_ACCOUNT_STATUS_TYPE_Copy != e_AccountStatusType ) ) {
		return False;
	}
	
	// comment 2015/02/12 MSE高橋 [Poseidon][CQ509510] OCR読込み中は、アプリ切り替え不可.
	if( ( EP_PNS_ReadExec == onlyPanel.ep_PanelStatus )					// 読込中.
	    || ( EP_PNS_ReadErrorStop == onlyPanel.ep_PanelStatus )				// 読込エラー.
	    || ( EP_PNS_ReadStop == onlyPanel.ep_PanelStatus )					// 読込停止中.
	    || ( EP_PNS_SeparateScanStop == onlyPanel.ep_PanelStatus )			// 分割読込停止中.
	    || ( EP_PNS_RealTimePrvExec == onlyPanel.ep_PanelStatus )			// リアルタイムプレビュー動作中.
		|| ( EP_PNS_OcrReadExec == onlyPanel.ep_PanelStatus )				// OCR読込み中.
// comment 2016/10/07 SCD2-Sky佐藤 [IT5_3.3][1005928] ジョブ飛び越し対応.
		// 停止中ジョブ判定を先頭ジョブのみに変更したため、停止中ジョブとして判定していたベンダー停止中状態を.
		// 別途、切り出し、アプリ切り替え不可条件として追加する.
		|| ( EP_PNS_VenderJobOutOfMoneyStop == onlyPanel.ep_PanelStatus )	// ベンダー停止中.
	    || ( EP_PNS_PrintStop == onlyPanel.ep_PanelStatus ) ) {				// プリント停止中.
		return False;
	}
// comment 2016/10/07 SCD2-Sky佐藤 [IT5_3.3][1005928] ジョブ飛び越し対応.
//	PCGC_StopGamenGetJob c_StopGamenGetJob;
//	short _ss_StopJob = c_StopGamenGetJob.getNowStopJob();
//	for( short i = 0; i < _ss_StopJob; i++ ) {
//		if( PCGE_StopJobPrint == c_StopGamenGetJob.s_MmbAryStopJob[i].e_StopJobStatus ) {
//			return False;
//		}
//	}
	// 実行中ジョブに停止中ジョブが存在する場合にアプリ切り替え不可とするのではなく.
	// 印刷中ジョブ（先頭ジョブ）が印刷停止中の場合は、アプリ切り替えを不可とする.
	if( ( NULL != comPrintJob ) && ( NULL != comPrintJob->getJobStatus() ) ) {
		if( TYPE_OutputStatusPrintStop == comPrintJob->getJobStatus()->getOutputStatus() ) {
			return False;
		}
	}
	// マルチジョブ禁止かつ、パネルステータスが設定中でない場合.
	if( ( True == onlyPanel.isMultiCopyJobManageOption() )
	    && ( EP_PNS_Setting != onlyPanel.ep_PanelStatus ) ) {
		return False;
	}
	// 警告チェックは何度もコールされるので、チェックタイミング変更により、処理をcheckWarningEnable()に移動.
	Bool _bl_ret = True;
	if( ( PNLE_ACCOUNT_STATUS_TYPE_Copy == e_AccountStatusType )
	    && ( True == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_OHPPaperLengthFeedDisable ) ) ) {
		_bl_ret = True;
	}
	if( False == PLBC_FuncVersionCheck::isOK_TypeMLK() )
	{
		// ベンダー装着時でベンダー電源OFF時のとき.
		TYPE_KeyMode e_keyMode = APIC_KeyDevice::getKeyMode();
		if( ( TYPE_KeyModeVendor == e_keyMode ) || ( TYPE_KeyModeVendorKM == e_keyMode ) ) {
			if( False == APIC_KeyDevice::isVendorConnect() ) {
				switch( e_AccountStatusType ) {
					case PNLE_ACCOUNT_STATUS_TYPE_WebBrowser:		// ブラウザキーのときはTrueを返す.
						_bl_ret = True;
						break;
					case PNLE_ACCOUNT_STATUS_TYPE_MAX:				// APPキーもしくはブラウザランチャーが登録アプリのテーブルに存在するときはTrueを返す.
						if( True == ScanOapExtSet.getRegistAppCheck( TYPE_ExtTypeBrowserLauncher ) ||
						    True == ScanOapExtSet.getRegistAppCheck( TYPE_ExtTypeBrowserGeneral ) ) {
							_bl_ret = True;
						} else {
							_bl_ret = False;
						}
						break;
					default:										// 上記以外のときはFalseを返す.
						_bl_ret = False;
						break;
				}
			}
		}
	}//【CQ #363216】【Restricted the condition for ZeusMLK stream and enabled in Minerva 1st】【HCL_Joseph】 2012/04/02 Add.

	// ベンダ装着時パスワード管理入力中の場合.
	if( True == onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, PNL_OTHER_VendorAdminInput ) ) {
		return False;
	}
	// 指定アプリ初期起動設定関連警告が発生している場合は、以下の条件を確認して機能有効/無効を判断する.
	if( True == c_ExtPanel.checkSpecifiedInitialWarning() ) {
		// 「ベンダー2装着」以外は課金対象時の制限無しとみなす.
		if( TYPE_KeyModeVendorKM == APIC_KeyDevice::getKeyMode() ) {
			Bool b_Connect = APIC_KeyDevice::isVendorConnect();
			Bool b_Coin = APIC_KeyDevice::isCoinExist();
			// ベンダー電源OFF、または、コイン未投入.
			if( False == b_Connect || False == b_Coin ) {
				switch( e_AccountStatusType ) {
					case PNLE_ACCOUNT_STATUS_TYPE_Copy:
						// 機能毎使用設定：コピー「管理する」の場合は機能無効.
						if( TYPE_AppAtAdminMachineVendorEnableWithCardOrCoin == PnlComGetSystemSetting()->getCopyAtAdminMachineVendor() ) {
							_bl_ret = False;
						}
						break;
					case PNLE_ACCOUNT_STATUS_TYPE_Fax:
						// 機能毎使用設定：データ送信「管理する」の場合は機能無効.
						if( TYPE_AppAtAdminMachineVendorEnableWithCardOrCoin == PnlComGetSystemSetting()->getSendAtAdminMachineVendor() ) {
							_bl_ret = False;
						}
						break;
					case PNLE_ACCOUNT_STATUS_TYPE_BoxOperation:
					case PNLE_ACCOUNT_STATUS_TYPE_Print:
						// 機能毎使用設定：PC印刷「管理する」の場合は機能無効.
						if( TYPE_AppAtAdminMachineVendorEnableWithCardOrCoin == PnlComGetSystemSetting()->getPrintAtAdminMachineVendor() ) {
							_bl_ret = False;
						}
						break;
					default:
						break;
				}
			}
		}
	}
	if( True == _bl_ret ) {	// 有効時.
		_bl_ret = bl_isWarningEnable;// 警告時の有効状態で上書きする.

// comment 2015/09/11 OPS1-Sky藤岡 [ZeusS_ZX0][CQ516675].
		// ベンダー電源OFF時、Webブラウザーは押下可、APPは押下時にチェックする.
		if( ( PNLE_ACCOUNT_STATUS_TYPE_WebBrowser == e_AccountStatusType )		// Webブラウザー.
		 || ( PNLE_ACCOUNT_STATUS_TYPE_MAX == e_AccountStatusType ) ) {			// APP.
			if( PERROR_CheckVendorPowerSupply == m_e_WarningName ) {					// ベンダー電源OFF.
				_bl_ret = True;
			}
		}
//  comment 2016/01/19 OPS1-Sky守川 [特ROM][FQ-ISD3G-1600010-OFP013].
		if( PNLE_ACCOUNT_STATUS_TYPE_MAX == e_AccountStatusType ) {				// APP.
			if( ( PERROR_KeyCounterUnset_S == m_e_WarningName )				 	// キーカウンター未挿入(設定時).
			 || ( ( True == APIC_KeyDevice::isKeyCntIFVndMode() ) 				// キーカウンタI/Fベンダーモード.
			 && ( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBit( 39, 3 ) )	// OAPとキーカウンタI/Fベンダーモード併用設定.
			 &&	( PERROR_CoinUnset == m_e_WarningName ) ) ) {					// コイン未挿入.
				_bl_ret = True;
			}
		}
// Comment 2020/11/09 【HCLT_Binesh】【IT6_2.1】【OP_BTS-28831】 To grayout copy, scanfax and box key when web browser is opened and Vendor power supply ON warning exist.
		APIC_Job* pc_Job = NULL;
		pc_Job = PnlComGetJobCopy();
		if( NULL != pc_Job ) {
			Bool bl_VendorWarningInBrowserExist = False;
			bl_VendorWarningInBrowserExist = pc_Job->isWarning( ERROR_CheckVendorPowerSupply );	//Use copy instance to check for vendor power supply warning while web browser is opened.
			//When web browser is opened and vendor power supply warning exist, check copy or scanfax or box key status and deactivate the keys.
			if( (( PNLE_ACCOUNT_STATUS_TYPE_Copy == e_AccountStatusType ) || ( PNLE_ACCOUNT_STATUS_TYPE_BoxOperation == e_AccountStatusType ) || ( PNLE_ACCOUNT_STATUS_TYPE_Fax == e_AccountStatusType )) 
				&& (( True == onlyPanel.getBrowserMode() ) && ( True == bl_VendorWarningInBrowserExist )) ){
				_bl_ret = False;
			}
		}
	}
	return _bl_ret;
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： 定期起床.
//================================================================================================
void PCGC_HomeTopMenu::procTimerWakeUp( uchar option )
{
	PPopupAnimationCom::procTimerWakeUp( option );

// comment 2017/11/21(火) SCD2-Sky長谷川(伸) [FQ-OPSD-1700523-OFP264].
	if ( ( True == pc_ProgramKeyDataGetForTopMenu.getNeedProgramSearchFlg() )
	  && ( False == pc_ManageIGCWaitDisplay.getDisplayFLG() )
	  && ( PNLD_TOPMENU_PROGRAM_IGCWAITSEARCH_SEARCHING != pc_ProgramKeyDataGetForTopMenu.getIGCProgramSearchStatus() ) 
	  && ( True == getDBInitializeStatus() ) ) {
		pc_ProgramKeyDataGetForTopMenu.setIGCProgramSearchStatus( PNLD_TOPMENU_PROGRAM_IGCWAITSEARCH_SEARCHING );
		// comment 2018/10/26 SCD2 T.Nishiba [Eagle][AR1064089]プログラム宛先の再検索を一度だけに制限する.
		pc_ProgramKeyDataGetForTopMenu.setProgramSearchRetryExcuted( True );
		PCGC_PanelMain::IGCWaitReqIndi( 0 );
	}

	bl_isWarningEnable = True;
// comment 2015/09/11 OPS1-Sky藤岡 [ZeusS_ZX0][CQ516675].
	m_e_WarningName = TYPE_WarningName_Non;

	// 次ジョブ予約不可の警告が発生している場合、アプリ選択を禁止にする.
	PWarningcomIF* pc_WarningcomIF = PWarningcomIF::newInstance();
	if( NULL != pc_WarningcomIF ) {
		// 全画面警告発生のチェック.
		ushort us_warntbl = onlyPanel.isAllGamenWarning( onlyPanel.cc_DisplayJob );
		// ログインユーザーに関連するジョブで金額不足警告が発生していた場合のみ、金額不足警告画面は表示させる.
		if( ( DEF_sp_GamenWarningTable > us_warntbl ) &&
		    ( False == onlyPanel.isShortChargeWarningOn( sp_GamenWarningTable[ us_warntbl ].ec_WarningName, onlyPanel.cc_DisplayJob ) ) ) {
			us_warntbl = 0;
		}
		if( ( 0 != us_warntbl ) &&
		    ( DEF_sp_GamenWarningTable > us_warntbl ) )  {
			Bool _is_AppChangeEnableWarning = False;
			TYPE_WarningName _e_WarningName = sp_GamenWarningTable[us_warntbl].ec_WarningName;
//comment 2019/02/19 [HCLT-Sai] [IT6_EagleH] [OP_BTS-2425,OP_BTS-2523] Display Warning Button click issue fix.
//comment 2019/04/08 SCD2-Sky西村 [IT6_EagleH] [OP_BTS-4765] ドラムユニット検出エラー警告発生の場合、トップメニュー画面に「警告表示」キーは押下不可の対応 .
//			if (True == pc_WarningcomIF->isEnableNextJobSet(_e_WarningName)
//				&& True == m_bl_WarningOpenDispKey) {
			if (True == m_bl_WarningOpenDispKey) {
				m_bl_WarningOpenDispKey = False;
				PNLC_WarningManager* pc_WarningManager = PNLC_WarningManager::newInstance();
				if( NULL != pc_WarningManager ) {
					pc_WarningManager->openWarning( us_warntbl, onlyPanel.cc_DisplayJob, this);
					return;
				}
			}
			switch( _e_WarningName ) {
				case PNL_ERROR_NoFrequency:			// 管理装置度数なし.
				case PERROR_CoinUnset:				// コピーベンダーコイン未投入.
				case PERROR_MagCardUnset:			// 管理カード（磁気カード）未挿入(プリント時).
				case PERROR_CardUnsetAndNonAuth:	// 管理カード（磁気カード）未挿入.
				case PERROR_MagCardUnset_S:			// 管理カード（磁気カード）未挿入.
				case PERROR_KeyCounterUnset_S:		// キーカウンター未挿入(設定時).
				case PERROR_CopyKitTotalLimit_S:	// コピーキットカウンタ／カラー＆ブラック上限設定値オーバー(設定時)警告.
				case PNL_ERROR_CopyJobEntryOver:	//
				case PNL_ERROR_ScanJobEntryOver:
				case PNL_ERROR_FaxJobEntryOver:
				case PNL_ERROR_TouchAndPrintJobEntryOver:
				case PNL_ERROR_JobEntryOverBox:
				case PNL_ERROR_JobEntryOverFax:
					_is_AppChangeEnableWarning = True;
					break;
				default:	break;
			}
			// 次ジョブ予約可能か？
			if( ( False == _is_AppChangeEnableWarning )
			    && ( False == pc_WarningcomIF->isEnableNextJobSet( sp_GamenWarningTable[us_warntbl].ec_WarningName ) ) ) {
				// 次ジョブ予約不可能な警告が発生している.
				bl_isWarningEnable = False;
// comment 2015/09/11 OPS1-Sky藤岡 [ZeusS_ZX0][CQ516675].
				m_e_WarningName = sp_GamenWarningTable[us_warntbl].ec_WarningName;
				return;
			}
		}
// Comment 2020/11/09 【HCLT_Binesh】【IT6_2.1】【OP_BTS-28831】 When web browser is opened and Vendor power supply ON warning exist, to open vendor power supply warning screen when Disp Warning key is message area is clicked.
		APIC_Job* pc_Job = NULL;
		pc_Job = PnlComGetJobCopy();
		if( NULL != pc_Job ) {
			Bool bl_VendorWarningInBrowserExist = False;
			bl_VendorWarningInBrowserExist = pc_Job->isWarning( ERROR_CheckVendorPowerSupply );	//Use copy instance to check for vendor power supply warning while web browser is opened.
			if( ( 0 == us_warntbl ) && ( True == onlyPanel.getBrowserMode() ) && ( True == bl_VendorWarningInBrowserExist ))  {
				if (True == m_bl_WarningOpenDispKey) {
					m_bl_WarningOpenDispKey = False;
					PNLC_WarningManager* pc_WarningManager = PNLC_WarningManager::newInstance();
					if( NULL != pc_WarningManager ) {
						for( ushort us_cnt = 0 ; us_cnt < ( ushort )( us_GamenWarningTableSize/sizeof( SP_GamenWarningTable ) ) ; us_cnt ++ ) {
							if( sp_GamenWarningTable[us_cnt].ec_WarningName == PERROR_CheckVendorPowerSupply ) {
								pc_WarningManager->openWarning( us_cnt, onlyPanel.cc_DisplayJob, this);
								return;
							}
						}
					}
				}
			}
		}
	}
	// ユーザー切り替え判定.
	checkSupporNowLoginUserAndTrack();

// comment 2019/06/27 SCD2-Sky谷尾 [EagleZ+][OP_BTS-7397]ウィジェット再描画処理追加,
	if( True == m_bl_PriorityTaskBrw ) {
		PM_TaskID uc_PriTaskID;									// 最優先タスク取得.
		PMGC_PanelManager::getPriority( &uc_PriTaskID );
		if( PM_TASKID_PANEL == uc_PriTaskID ) {					// タスクがパネルに切り替わってから再描画.
			m_bl_PriorityTaskBrw = False;
			Cmp_GadgetComponent.restart();
		}
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： バッジキー活性状態判断.
//================================================================================================
void PCGC_HomeTopMenu::setBadgeKeyActivate( uchar uc_index )
{
// comment 2020/01/10 SCD2-Sky中野 [HeliosMLK][OP_BTS-18537].
	if( NULL == PnlComGetTrackManager() ) {
		LOGF_debug_PutEventLog(LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!");
		return;
	}
	
	APIC_TabletIF* pc_TabletIF = NULL;										// API Common API取得.
	APIE_TabletOptionState e_GoogleDriveSave  = APIE_TabletOptionDisable;	// Googleドライブ保存機能有効無効情報取得.
	APIE_TabletOptionState e_LocalStorageSave = APIE_TabletOptionDisable;	// ローカルストレージ保存機能有効無効情報取得.

	PNLE_TopMenuShortCut e_TopMenuShortCut = st_BadgeKeyInfo[uc_index].e_ShortCutKind;
	// comment 2015/10/20 OPS1-川手 [特ROM対応][FQ-OPSD-1500259-OFP185]Fax受信時に受信エラー(R27)が頻繁に発生する.
	Bool bl_isActivate_Old = st_BadgeKeyInfo[uc_index].bl_isActivate;
// comment 2018/11/05 SCD2-Sky中野 [EagleH] WebUIv3FUM対応.
//	PNLE_UIMode e_NowUIMode = PNLC_WebUIControl::getInstance()->getUIMode();

	// comment 2018/08/10 SCD2 T.Nishiba [Eagle][RQ-698-41]新設定メニューのメニュー項目対応.
	if( PNLE_TopMenuShortCut_Utility == e_TopMenuShortCut ) {
		// 設定メニューキーは常に有効にする.
		st_BadgeKeyInfo[uc_index].bl_isActivate = True;
		return;
	}

// comment 2016/09/16 MSE岡田 [IT5_3.3][1005694] 省電力復帰のHDDチェックスキップ時はコピー以外をグレイアウトする.
	if( True == PNLC_SnapshotControlLib::newInstance()->getSkipCheckHDD() ) {
		if( ( PNLE_TopMenuShortCut_None != e_TopMenuShortCut )					// 未設定ではない.
		&&	( ( PNLE_TopMenuShortCut_Func != ( e_TopMenuShortCut & 0xFF00 ) )	// 機能ではない.
			||( PNLE_TopMenuShortCut_FuncCopy != e_TopMenuShortCut ) ) ) {		// 機能:コピーではない.
			st_BadgeKeyInfo[uc_index].bl_isActivate = False;
// comment 2019/02/05(火) SCD2-Sky長谷川(伸) [FQ-OPSD-1900013-OFP008].
 //			m_bl_UpdateProgramKeyStatus = True;	// HDDチェックスキップ後にプログラムキーの活性状態を更新するためフラグをたてる.
			return;
		}
	} else {
		// 以降の処理で活性/非活性の判定処理を行わない機能もあるため、活性状態に戻しておく.
		st_BadgeKeyInfo[uc_index].bl_isActivate = True;
	}

// comment 2016/07/20 ユビキタス [RQ-553][1002239]【IT評価】ユビキタス関連のAttention警告発生、ユビキタスプリントが使用できない.
//// comment 2016/06/09 ユビキタス [RQ-553][Enhancement of Max num of LK-114 devices][step2].
//	PNLC_UbqDocListLib* pc_UbqDocListLib = NULL;							// ユビキタス文書ライブラリクラスのインスタンス.
//
	switch( e_TopMenuShortCut & 0xFF00 ) {
		case PNLE_TopMenuShortCut_Func:				// 機能.
			switch( e_TopMenuShortCut ) {
				case PNLE_TopMenuShortCut_FuncCopy:		// 機能:コピー.
					st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Copy );
// comment 2018/11/05 SCD2-Sky中野 [EagleH] WebUIv3FUM対応.
//					if(PNLE_UIModeLightUI == e_NowUIMode){
//						st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnableHomeMenu( PNLE_ACCOUNT_STATUS_TYPE_Copy, PNLE_TopMenuShortCut_FuncCopy);
//					}
					break;
				case PNLE_TopMenuShortCut_FuncScanFax:	// 機能:スキャン/ファクス.
					st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Fax );
					break;
				case PNLE_TopMenuShortCut_FuncBox:		// 機能:ボックス.
					st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_BoxOperation );
					break;
				case PNLE_TopMenuShortCut_FuncBrowser:	// 機能:ブラウザ.
				// comment 2018/03/02 1PF2G石川 [AR1043947] 起動直後ブラウザキーグレーアウト対応.
					if( ( ( False == m_bl_BrowserFlag ) && ( 9 == uc_index ) )	// ブラウザーが割り当てられていない.
// comment 2019/04/15 SCD2-Sky小畑 [EagleH] メモリフル時のフェールセーフ対応.
//						||(	False == onlyPanel.bl_BrowserStartUpFlag) ){		//ブラウザの準備が出来ていない.
						||(	False == onlyPanel.bl_BrowserStartUpFlag)			//ブラウザの準備が出来ていない.
						||(	True == PNLC_BrowserChangeTabControl::getInstance()->isBrowserProcessRestart()) ){	//ブラウザプロセス再起動中.
						st_BadgeKeyInfo[uc_index].bl_isActivate = False;
					}
					else {
						st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_WebBrowser );
					}
					break;
				case PNLE_TopMenuShortCut_FuncOAPApp:	// 機能:APP.
				{
					if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type3() ) {
						// Do Nothing.
					} else {
						const TYPS_ExtRegistration* pc_ExtRegistration = ScanOapExtSet.getApplication();
						if( True == ScanOapExtSet.isActiveGeneralApplication()
						    && NULL != pc_ExtRegistration
						    && (	( TYPE_ExtTypeScanS == pc_ExtRegistration->s_Version.e_Type )
						            || ( TYPE_ExtTypeScanD == pc_ExtRegistration->s_Version.e_Type )
						            || ( TYPE_ExtTypePullPrint == pc_ExtRegistration->s_Version.e_Type )
						            || ( TYPE_ExtTypeBrowserScan == pc_ExtRegistration->s_Version.e_Type )
						            || ( TYPE_ExtTypeBrowserPull == pc_ExtRegistration->s_Version.e_Type )
						            || ( TYPE_ExtTypeBrowserGeneral == pc_ExtRegistration->s_Version.e_Type )	) ) {
							st_BadgeKeyInfo[uc_index].bl_isActivate = False;
							break;
						}

						// ブラウザランチャーモード時は、APPキー無効.
						if( ( True == onlyPanel.getBrowserMode() ) && ( BRWE_BrowserBootMode_Launcher == onlyPanel.e_BrowserBootMode ) ) {
							st_BadgeKeyInfo[uc_index].bl_isActivate = False;
							break;
						}
					}//【UV-01342】【OpenAPI／ブラウザランチャー起動条件の改善】【HCL_Krishna】2013/02/19 Add.
					// 指定アプリ初期起動設定関連警告が発生している場合はAPPキー無効.
					if( True == c_ExtPanel.checkSpecifiedInitialWarning() ) {
						st_BadgeKeyInfo[uc_index].bl_isActivate = False;
						break;
					}
					if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type3() ) {
						// Do Nothing.
					} else {
						if( ExtSet.isActiveGeneralApplication() ) {
							st_BadgeKeyInfo[uc_index].bl_isActivate = False;
							break;
						}
					}//【UV-01342】【OpenAPI／ブラウザランチャー起動条件の改善】【HCL_Krishna】2013/02/19 Add.
					if( ( False == m_bl_AppFlag ) 		// APPが割り当てられていない.
					    && ( 10 == uc_index ) ) {			// 11番目.
						st_BadgeKeyInfo[uc_index].bl_isActivate = False;
						break;
					}
					if( bl_AppButtonDisplay ) {
						st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_MAX );
					}
					else {
						st_BadgeKeyInfo[uc_index].bl_isActivate = False;
					}
				}
				break;
				case PNLE_TopMenuShortCut_HitachICC_SecurityScan:	// セキュリティスキャン.
					st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Scan );
					break;
				case PNLE_TopMenuShortCut_HitachICC_Fax:			// ファクス.
					st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Fax );
					break;
				case PNLE_TopMenuShortCut_FuncUbqPrint:			// ユビキタスプリント.
					if( False == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Print )								||		// 機能有効ではない.
// comment 2021/09/03 2DWS2G-Sky延原 [TAS000000008632] LK-114環境にて標準トップメニューキーにスキャンプログラムを追加するとE301(0xB3BB)発生.
//						TYPE_UserTypeUser != PnlComGetUser()->getUserType()									||		// 一般ユーザー以外でログイン.
						( NULL != PnlComGetUser() && 
						  TYPE_UserTypeUser != PnlComGetUser()->getUserType() )									||		// 一般ユーザー以外でログイン.
// [MinervaSBK][RQ-268] サーバーレスユビキタス拡張サーバ認証対応 start.
////[ユビキタスプリント 拡張サーバー＋本体認証 バッティング対応] Start.
//					    TYPE_UserAuthenticationIntermeddiateServer == PnlComGetLoginUserAuthenticationType()	|| // 拡張サーバー認証ユーザーの場合.
						// 機能拡張3版以降は拡張サーバー時にサーバーレスユビキタスプリント使用可能とする.
// comment 2021/09/03 2DWS2G-Sky延原 [TAS000000008632] LK-114環境にて標準トップメニューキーにスキャンプログラムを追加するとE301(0xB3BB)発生.
//						TYPE_FuncLimit_Disable == PnlComGetUser()->getFunctionLimitation().e_EnablePrint	||		// プリンター印字禁止ユーザー.
						( NULL != PnlComGetUser() && 
						  TYPE_FuncLimit_Disable == PnlComGetUser()->getFunctionLimitation().e_EnablePrint )	||		// プリンター印字禁止ユーザー.
// ユビキタス[AR1008922]【市場問題】機能バージョンが2.1/2.0だとLK-114使用時にドライバから認証指定なし印刷ができない start.
//						( False == PLBC_FuncVersionCheck::isOK_IT5_Type3() && 
						( TYPE_UbqFuncVersion3_Disable == PnlComGetUbiquitousSetting()->getUbqFuncVersion3() && 
// ユビキタス[AR1008922]【市場問題】機能バージョンが2.1/2.0だとLK-114使用時にドライバから認証指定なし印刷ができない end.
						  TYPE_UserAuthenticationIntermeddiateServer == PnlComGetLoginUserAuthenticationType() )	){ // 拡張サーバー認証ユーザーの場合.
////[ユビキタスプリント 拡張サーバー＋本体認証 バッティング対応] End.
// [MinervaSBK][RQ-268] サーバーレスユビキタス拡張サーバ認証対応 end.
						st_BadgeKeyInfo[uc_index].bl_isActivate = False;
					}
// comment 2016/07/20 ユビキタス [RQ-553][1002239]【IT評価】ユビキタス関連のAttention警告発生、ユビキタスプリントが使用できない.
//// comment 2016/06/09 ユビキタス [RQ-553][Enhancement of Max num of LK-114 devices][step2].
//					// ユビキタスプリントVer4(step2)以降の場合.
//					pc_UbqDocListLib = PNLC_UbqDocListLib::newInstance();		// ユビキタス文書ライブラリクラスのインスタンス取得.
//					if( NULL != pc_UbqDocListLib ){
//						if( PNLE_UbqFuncVersion_4_Step2 <= pc_UbqDocListLib->getUbqFuncVersion() ) {
//							if( True == onlyPanel.isWarningOn( comPanelJob, PNL_PATTENTION_UbiquitousDifferenceGroupName ) ){				// ユビキタスプリントグループ名不一致.
//								st_BadgeKeyInfo[uc_index].bl_isActivate = False;
//							}else if( True == onlyPanel.isWarningOn( comPanelJob, PNL_PATTENTION_UbiquitousDeviceOver ) ){					// ユビキタスプリント登録台数上限オーバー.
//								st_BadgeKeyInfo[uc_index].bl_isActivate = False;
//							}else if( True == onlyPanel.isWarningOn( comPanelJob, PNL_PATTENTION_UbiquitousTimeOver ) ){					// ユビキタスプリント接続タイムアウト.
//								st_BadgeKeyInfo[uc_index].bl_isActivate = False;
//							}else if( True == onlyPanel.isWarningOn( comPanelJob, PNL_PATTENTION_UbiquitousConnectError ) ){				// ユビキタスプリント通信異常.
//								st_BadgeKeyInfo[uc_index].bl_isActivate = False;
//							}
//						}
//					}
					break;
				case PNLE_TopMenuShortCut_FuncFaxOnly:
				{
					Bool bl_FaxEnable = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Fax );
					st_BadgeKeyInfo[uc_index].bl_isActivate = bl_FaxEnable;
					// comment 2018/04/16 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
// comment 2018/11/05 SCD2-Sky中野 [EagleH] WebUIv3FUM対応.
//					if(PNLE_UIModeLightUI == e_NowUIMode){
//						st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnableHomeMenu( PNLE_ACCOUNT_STATUS_TYPE_Fax, PNLE_TopMenuShortCut_FuncFaxOnly);
//					}
				}
				break;
				case PNLE_TopMenuShortCut_ScanToDevice:			// ScanToDevice.
					st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_ScanToDevice );
					pc_TabletIF = APIC_TabletIF::newInstance();													// API Common API取得.
					if( NULL != pc_TabletIF ) {
						e_GoogleDriveSave  = pc_TabletIF->getOptionEnable( APIE_TabletGoogleDriveSave );			// Googleドライブ保存機能有効無効情報取得.
						e_LocalStorageSave = pc_TabletIF->getOptionEnable( APIE_TabletLocalStorageSave );			// ローカルストレージ保存機能有効無効情報取得.

						// Googleドライブ保存機能、ローカルストレージ保存機能の両方無効の場合はグレーアウト.
						if( ( APIE_TabletOptionDisable == e_GoogleDriveSave ) && ( APIE_TabletOptionDisable == e_LocalStorageSave ) ) {
							st_BadgeKeyInfo[uc_index].bl_isActivate = False;
						}
					} else {
						st_BadgeKeyInfo[uc_index].bl_isActivate = False;
					}
					break;
				// comment 2018/04/16 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
// comment 2018/11/05 SCD2-Sky中野 [EagleH] WebUIv3FUM対応.
//				case PNLE_TopMenuShortCut_ScanToEmail:		// ScanToEmail.
//					st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnableHomeMenu( PNLE_ACCOUNT_STATUS_TYPE_Fax, PNLE_TopMenuShortCut_ScanToEmail);
//					break;
//
//				case PNLE_TopMenuShortCut_ScanToBox:		// ScanToBox.
//					st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnableHomeMenu( PNLE_ACCOUNT_STATUS_TYPE_Fax ,PNLE_TopMenuShortCut_ScanToBox);
//					break;
//				case PNLE_TopMenuShortCut_ScanToPC:			// ScanToPC.
//					st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnableHomeMenu( PNLE_ACCOUNT_STATUS_TYPE_Fax ,PNLE_TopMenuShortCut_ScanToPC);
//					break;
//				case PNLE_TopMenuShortCut_InternetFax:		// InternetFax.
//					st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnableHomeMenu( PNLE_ACCOUNT_STATUS_TYPE_Fax ,PNLE_TopMenuShortCut_InternetFax);
//					break;
//				case PNLE_TopMenuShortCut_IPAddressFax:		// IPAddressFax.
//					st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnableHomeMenu( PNLE_ACCOUNT_STATUS_TYPE_Fax ,PNLE_TopMenuShortCut_IPAddressFax);
//					break;
//// comment 2018/11/09 Sky雜賀 [EagleH][RQ-272].
//				case PNLE_TopMenuShortCut_IpFaxT38:		// IP-FAX(T.38).
//					st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnableHomeMenu( PNLE_ACCOUNT_STATUS_TYPE_Fax ,PNLE_TopMenuShortCut_IpFaxT38);
//					break;
				default :	break;
			}
			break;
		case PNLE_TopMenuShortCut_Copy:				// コピー機能設定.
		case PNLE_TopMenuShortCut_CopyMemory:		// コピー設定メモリ.
			st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Copy );
			break;
		case PNLE_TopMenuShortCut_ScanFax:			// スキャン/ファクス機能設定.
			st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Fax );
			if( True == st_BadgeKeyInfo[uc_index].bl_isActivate ) {
// comment 2016/02/02 OPS1-Sky善[特ROM][IT5-14_機種マージ]暗号化PDF操作フロー改善要望.
//				if( ( ( PNLE_TopMenuShortCut_ScanFaxResolution == e_TopMenuShortCut )		// 解像度.
//				      || ( PNLE_TopMenuShortCut_ScanFaxFileFormat == e_TopMenuShortCut ) )	// ファイル形式.
				if ( ( ( PNLE_TopMenuShortCut_ScanFaxResolution == e_TopMenuShortCut )				// 解像度.
				    || ( PNLE_TopMenuShortCut_ScanFaxFileFormat == e_TopMenuShortCut ) 				// ファイル形式.
				    || ( PNLE_TopMenuShortCut_ScanFaxEncryptPDFPassword == e_TopMenuShortCut ) )	// PDF暗号化パスワード.
				    && ( True == checkFaxDefaultAppOn() ) ) {	// ファクスデフォルトアプリ有効.
					// ファクスデフォルトアプリが有効時は、非活性表示.
					st_BadgeKeyInfo[uc_index].bl_isActivate = False;
				}
			}
			break;
		case PNLE_TopMenuShortCut_ScanFaxMemory:	// スキャン/ファクス設定メモリ.
			st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Fax );
			if( True == checkFaxDefaultAppOn() ) {
				if( False == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW201, 5 ) ) {	// DipSW:201 HEX:0x20が無効時.
					// ファクスデフォルトアプリが有効時は、非活性表示.
					st_BadgeKeyInfo[uc_index].bl_isActivate = False;
				}
			}
			if( True == st_BadgeKeyInfo[uc_index].bl_isActivate ) {
				char	ss_bufSessionID[33];		// sessionIDバックアップ.
				memset( ss_bufSessionID, 0, sizeof( ss_bufSessionID ) );
				Bool	bl_RightFaxMode = False;	// RightFaxモード判定.
				//
				if( ( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW201, 5 ) )		// DipSW:201 HEX:0x20が有効時.
				    && ( True == ExtSet.getFaxAppOn() ) && ( True == ExtSet.isRightFaxMode() ) ) {						// RightFaxモード時.
					strncpy( ss_bufSessionID, ExtSet.getSessionID(), sizeof( ss_bufSessionID ) - 1 );
					ExtSet.setSessionID( "" );
					bl_RightFaxMode = True;
				}
				// プログラムキー情報と検索結果宛先種別.
				// comment 2015/10/20 OPS1-川手 [特ROM対応][FQ-OPSD-1500259-OFP185]Fax受信時に受信エラー(R27)が頻繁に発生する.
				// プログラムキーの活性状態の変更を抑制する制御を追加する.
//				st_BadgeKeyInfo[uc_index].bl_isActivate = getActivateProgramKey( st_BadgeKeyInfo[uc_index].ss_ProgramNo );
				if (True == m_bl_UpdateProgramKeyStatus) {
					st_BadgeKeyInfo[uc_index].bl_isActivate = getActivateProgramKey( st_BadgeKeyInfo[uc_index].ss_ProgramNo );
// comment 2016/09/16 MSE岡田 [IT5_3.3][1005694] プログラムキーの更新を抑制するためのフラグを落とすタイミングを修正.
// comment 2019/02/05(火) SCD2-Sky長谷川(伸) [FQ-OPSD-1900013-OFP008].
//					m_bl_UpdateProgramKeyStatus = False;	// プログラムキーの更新を抑制するため、更新処理用のフラグを落とす.
				} else {
					st_BadgeKeyInfo[uc_index].bl_isActivate = bl_isActivate_Old;		// 更新が不要なケースでは前回値を設定する.
				}
				if( True == bl_RightFaxMode ) {				// RightFaxモード時.
					ExtSet.setSessionID( &ss_bufSessionID[0] );
				}
			} else {
				;	// 処理なし.
			}
			break;
		case PNLE_TopMenuShortCut_SysBox:			// システムボックス機能設定.
			// comment : 20180508 Chikuan : [Eagle] [KRQ-314-28] 【新UI】認証＆プリントBox対応.
// comment 2018/11/05 SCD2-Sky中野 [EagleH] WebUIv3FUM対応.
//			if( ( PNLE_UIModeLightUI == e_NowUIMode )
//			 && ( PNLE_TopMenuShortCut_SysBoxAuthPrint == st_BadgeKeyInfo[uc_index].e_ShortCutKind ) ){
//				st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnableHomeMenu( PNLE_ACCOUNT_STATUS_TYPE_BoxOperation, PNLE_TopMenuShortCut_SysBoxAuthPrint );
//			}else{
				st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_BoxOperation );
				if( True == st_BadgeKeyInfo[uc_index].bl_isActivate ) {
					getActivateSystemBoxKey( &st_BadgeKeyInfo[uc_index] );
				}
// comment 2018/11/05 SCD2-Sky中野 [EagleH] WebUIv3FUM対応.
//			}
			break;
// Comment 2019/10/09 【HCLT-Krishna】【IT6_HeliosMLK】【制御構成変更_No.43】 SSDレス時の機密文書プリント対応
//// Comment 2018/11/19 【HCLT-Vandana】【IT6_DenebMLK】【制御構成変更_No.83】
//#ifdef DEF_DenebMLKCus
#if defined  (DEF_DenebMLKCus) || defined ( DEF_HeliosMLKCus ) 
				case PNLE_TopMenuShortCut_SysBoxSecurity:
//Comment 2018/12/25 HCLT_Prashannaa AR 1070436 Secure print .
				st_BadgeKeyInfo[uc_index].bl_isActivate = ( PnlComGetHddInfo()->getHddExist() ? True: PnlComIsSecurityDoc());
				break;
#endif
		case PNLE_TopMenuShortCut_Utility:			// 設定メニュー.
		{
// comment 2016/10/07 SCD2-Sky佐藤 [IT5_3.3][1005928] ジョブ飛び越し対応.
			if( PNLE_TopMenuShortCut_QRCode == e_TopMenuShortCut ) 				// QRコード表示.
			{
				Bool bl_Activate = True;
				const TYPS_ExtRegistration* pc_ExtRegistration = ScanOapExtSet.getApplication();

				// OAPアプリ（スキャン(TypeD) 、スキャン(TypeS) 、PullPrint 、Browser(Scan) 、Browser(Pull)）動作中.
				if( ( True == ScanOapExtSet.isActiveGeneralApplication() )
				    && ( False == c_ExtPanel.getSpecifiedAppSetting() )
						 && ( NULL != pc_ExtRegistration )
						 && (	( TYPE_ExtTypeScanS == pc_ExtRegistration->s_Version.e_Type )
							 || ( TYPE_ExtTypeScanD == pc_ExtRegistration->s_Version.e_Type )
							 || ( TYPE_ExtTypePullPrint == pc_ExtRegistration->s_Version.e_Type )
							 || ( TYPE_ExtTypeBrowserScan == pc_ExtRegistration->s_Version.e_Type )
							 || ( TYPE_ExtTypeBrowserPull == pc_ExtRegistration->s_Version.e_Type )
							 || ( TYPE_ExtTypeBrowserGeneral == pc_ExtRegistration->s_Version.e_Type ) ) ){
					bl_Activate = False;
				}

				// IWSジョブ実行中.
				if( True == onlyPanel.bl_IWSAction ) {
					bl_Activate = False;
				}
				// ネットワーク初期化中.
				APIC_NetworkDevice*		pc_NetworkDevice = APIC_NetworkDevice::newInstance();
				// ネットワーク初期化中.
				if( NULL != pc_NetworkDevice
				    && True == pc_NetworkDevice->isNetworkInitialing() ) {
					bl_Activate = False;
				}
				// 割り込みモード中の場合、使用不可.
				if( EC_IntMode_Off != onlyPanel.ec_IntMode ) {
					bl_Activate = False;
				}
				st_BadgeKeyInfo[uc_index].bl_isActivate = bl_Activate;
				break;
			}

// comment 2020/08/17 [特ROM対応][IT5-144(機種マージ)] 転送ファックス設定をパスワード無しで使用可能にする要望対応.
			if( PNLE_TopMenuShortCut_ForwardFax == e_TopMenuShortCut ) {		// 転送ファクス設定キー.

				const TYPS_ExtRegistration* pc_ExtRegistration_chk = ScanOapExtSet.getApplication();
				APIC_SystemSetting* pc_SystemSetting_chk = APIC_SystemSetting::newInstance();

				// ファクス機能使用不可のときは非アクティブ.
				Bool bl_ForwardFaxKeyActivate = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Fax );

				// ユーザー認証/部門認証で未認証.
				if( ( NULL == PnlComGetUser() )
				 || (    ( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance()->getLine()) )
				      && ( TYPE_UserAuthenticationIntermeddiateServer != PnlComGetLoginUserAuthenticationType() )
				      && ( NULL == PnlComGetTrack() ) ) ) {
					bl_ForwardFaxKeyActivate = False;
				}
				// 全画面警告発生時.
				else if( False == bl_isWarningEnable ) {
					bl_ForwardFaxKeyActivate = False;
				}
				// ジョブ実行中.
				else if( True == PnlComGetSystemStatus()->isJobExist() ) {
					bl_ForwardFaxKeyActivate = False;
				}
				// ネットワーク初期化中.
				else if( True == APIC_NetworkDevice::newInstance()->isNetworkInitialing() ) {
					bl_ForwardFaxKeyActivate = False;
				}
				// 割り込みモード中の場合、使用不可.
				else if( EC_IntMode_Off != onlyPanel.ec_IntMode ) {
					bl_ForwardFaxKeyActivate = False;
				}
				// OAPアプリ（スキャン(TypeD) 、スキャン(TypeS) 、PullPrint 、Browser(Scan) 、Browser(Pull)）動作中.
				else if( ( True == ScanOapExtSet.isActiveGeneralApplication() )
					  && ( False == c_ExtPanel.getSpecifiedAppSetting() )
					  && (    NULL != pc_ExtRegistration_chk
					       && (    ( TYPE_ExtTypeScanS == pc_ExtRegistration_chk->s_Version.e_Type )
					            || ( TYPE_ExtTypeScanD == pc_ExtRegistration_chk->s_Version.e_Type )
					            || ( TYPE_ExtTypePullPrint == pc_ExtRegistration_chk->s_Version.e_Type )
					            || ( TYPE_ExtTypeBrowserScan == pc_ExtRegistration_chk->s_Version.e_Type )
					            || ( TYPE_ExtTypeBrowserPull == pc_ExtRegistration_chk->s_Version.e_Type )
					            || ( TYPE_ExtTypeBrowserGeneral == pc_ExtRegistration_chk->s_Version.e_Type ) ) ) ) {
					bl_ForwardFaxKeyActivate = False;
				}
				// IWS実行中.
				else if( True == onlyPanel.bl_IWSAction ) {
					bl_ForwardFaxKeyActivate = False;
				}
				// 長尺印刷が設定されているとき.
				else if( NULL != pc_SystemSetting_chk
					 &&  True == pc_SystemSetting_chk->getLongPicturePrint() ) {
					bl_ForwardFaxKeyActivate = False;
				}
				// ＨＤＤ装着時は、ＨＤＤが立ち上がるまで、受付禁止.
				else if( TYPE_HddStatusFormated != PnlComGetHddInfo()->getHddStatus() ) {
					bl_ForwardFaxKeyActivate = False;
				}

				st_BadgeKeyInfo[uc_index].bl_isActivate = bl_ForwardFaxKeyActivate;
				break;
			}

			const TYPS_ExtRegistration* pc_ExtRegistration = ScanOapExtSet.getApplication();
			if( True == ScanOapExtSet.isActiveGeneralApplication()
			    && NULL != pc_ExtRegistration
			    && (	( TYPE_ExtTypeScanS == pc_ExtRegistration->s_Version.e_Type )
			            || ( TYPE_ExtTypeScanD == pc_ExtRegistration->s_Version.e_Type )
			            || ( TYPE_ExtTypePullPrint == pc_ExtRegistration->s_Version.e_Type )
			            || ( TYPE_ExtTypeBrowserScan == pc_ExtRegistration->s_Version.e_Type )
			            || ( TYPE_ExtTypeBrowserPull == pc_ExtRegistration->s_Version.e_Type )
			            || ( TYPE_ExtTypeBrowserGeneral == pc_ExtRegistration->s_Version.e_Type )	) ) {
				st_BadgeKeyInfo[uc_index].bl_isActivate = False;
				// 指定アプリ設定中の場合.
				if( True == c_ExtPanel.getSpecifiedAppSetting() ) {
					st_BadgeKeyInfo[uc_index].bl_isActivate = True;
				} else {
					st_BadgeKeyInfo[uc_index].bl_isActivate = False;
				}
			} else if( True == onlyPanel.bl_IWSAction ) { 						// IWS実行中は非活性.
				st_BadgeKeyInfo[uc_index].bl_isActivate = False;
			} else {
				st_BadgeKeyInfo[uc_index].bl_isActivate = True;
			}
		}
		break;
		case PNLE_TopMenuShortCut_AppListShortCut:	// OAPアプリリスト.
		case PNLE_TopMenuShortCut_AppShortCut:		// OAPアプリ.
		{
			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type3() ) {
				// Do Nothing.
			} else {
				const TYPS_ExtRegistration* pc_ExtRegistration = ScanOapExtSet.getApplication();
				if( True == ScanOapExtSet.isActiveGeneralApplication()
				    && NULL != pc_ExtRegistration
				    && (	( TYPE_ExtTypeScanS == pc_ExtRegistration->s_Version.e_Type )
				            || ( TYPE_ExtTypeScanD == pc_ExtRegistration->s_Version.e_Type )
				            || ( TYPE_ExtTypePullPrint == pc_ExtRegistration->s_Version.e_Type )
				            || ( TYPE_ExtTypeBrowserScan == pc_ExtRegistration->s_Version.e_Type )
				            || ( TYPE_ExtTypeBrowserPull == pc_ExtRegistration->s_Version.e_Type )
				            || ( TYPE_ExtTypeBrowserGeneral == pc_ExtRegistration->s_Version.e_Type ) ) ) {
					st_BadgeKeyInfo[uc_index].bl_isActivate = False;
					break;
				}

				// ブラウザランチャーモード時は、APPキー無効.
				if( ( True == onlyPanel.getBrowserMode() ) && ( BRWE_BrowserBootMode_Launcher == onlyPanel.e_BrowserBootMode ) ) {
					st_BadgeKeyInfo[uc_index].bl_isActivate = False;
					break;
				}
			}//【UV-01342】【OpenAPI／ブラウザランチャー起動条件の改善】【HCL_Krishna】2013/02/19 Add.
			// 指定アプリ初期起動設定関連警告が発生している場合はOAPアプリキー無効.
			if( True == c_ExtPanel.checkSpecifiedInitialWarning() ) {
				st_BadgeKeyInfo[uc_index].bl_isActivate = False;
				break;
			}
			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type3() ) {
				// Do Nothing.
			} else {
				if( ExtSet.isActiveGeneralApplication() ) {
					st_BadgeKeyInfo[uc_index].bl_isActivate = False;
					break;
				}
			}//【UV-01342】【OpenAPI／ブラウザランチャー起動条件の改善】【HCL_Krishna】2013/02/19 Add.
			st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_MAX );
			{
				if( False == PLBC_FuncVersionCheck::isOK_TypeMLK() )
				{
					// ベンダー装着時でベンダー電源OFF時のとき.
					TYPE_KeyMode e_keyMode = APIC_KeyDevice::getKeyMode();
					if( ( TYPE_KeyModeVendor == e_keyMode ) || ( TYPE_KeyModeVendorKM == e_keyMode ) ) {
						if( False == APIC_KeyDevice::isVendorConnect() ) {
							if( 0xFFFF != us_OAPAppNo[ uc_index ] ) {
								// キーがOAPアプリ(ScanS/ScanD/PullPrint/ScanBrowser/PullPrintBrowser)の場合.
								APIC_ExtSetting* pc_ExtSetting = APIC_ExtSetting::newInstance( us_OAPAppNo[ uc_index ] );
								if( NULL == pc_ExtSetting ) {
									return;
								}
								TYPE_ExtType _s_ExtType = pc_ExtSetting->getApplication().s_Version.e_Type;
								if( ( TYPE_ExtTypeScanS         == _s_ExtType )
								    || ( TYPE_ExtTypeScanD       == _s_ExtType )
								    || ( TYPE_ExtTypePullPrint   == _s_ExtType )
								    || ( TYPE_ExtTypeBrowserScan == _s_ExtType )
								    || ( TYPE_ExtTypeBrowserPull == _s_ExtType ) ) {
									// キーをグレーアウトにする.
									st_BadgeKeyInfo[uc_index].bl_isActivate = False;
								}
							} else {
								// キーが汎用アプリ(RightFax)の場合.
								// キーをグレーアウトにする.
								st_BadgeKeyInfo[uc_index].bl_isActivate = False;
							}
						}
					}
				}//【CQ #363216】【Restricted the condition for ZeusMLK stream and enabled in Minerva 1st】【HCL_Joseph】 2012/04/02 Add.
			}
// Comment 2019/06/08 [IT6_1.0MR2] AR1077690 [HCLT-Catherine]  Market Place key greyed out condition.
			if ( PNLE_TopMenuShortCut_MarketPlaceApplication_Legacy == e_TopMenuShortCut ){
// Comment 2019/09/24 [EagleZ+_BK] [OP_BTS-12479] [HCLT-Naveena]  Market Place key greyed out condition if user not login.
//				if( False == bl_isWarningEnable || True == PnlComGetSystemStatus()->isJobExist() ) 	{								// If any warning Occurs.
// comment 2023/05/12 Sky中野 [IT6_2.3][OP_BTS-43388].
//				if( False == bl_isWarningEnable || True == PnlComGetSystemStatus()->isJobExist() ||( NULL == PnlComGetUser()) 
				if( False == bl_isWarningEnable ||( NULL == PnlComGetUser()) 
					|| ( ( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance()->getLine()) ) 
						&& ( NULL == PnlComGetTrack() ) && ( TYPE_UserAuthenticationIntermeddiateServer != PnlComGetLoginUserAuthenticationType() ) ) )	{								// If any warning Occurs.
					st_BadgeKeyInfo[uc_index].bl_isActivate = False;
// Comment 2019/09/27 [IT6_EagleH] [HCL_Naveena] [OP_BTS-14097] MatketPlace application key disable condition.
				} else if( ( NULL == PnlComGetUser() ) 																	// ユーザ認証ユーザなしの場合.
				|| ( ( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance()->getLine()) )							// 部門認証管理する.
				 && ( TYPE_UserAuthenticationIntermeddiateServer != PnlComGetLoginUserAuthenticationType() )	// 拡張サーバー認証ユーザー以外の場合.
				 && ( NULL == PnlComGetTrack() ) ) ) {															// 部門認証なし.
					st_BadgeKeyInfo[uc_index].bl_isActivate = False;
				} else {
					st_BadgeKeyInfo[uc_index].bl_isActivate = True;
				}
			}
		}
		break;
		case PNLE_TopMenuShortCut_GrpAppShortCut:					// グループアプリケーション グループ.
			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
				IsAppDisableCheck( uc_index );
			}
			break;
// comment 2015/03/04 OPS1-Sky榎本 [MinervaSBk][UV-01457]トップメニューへアドレス帳キー割付.
		case PNLE_TopMenuShortCut_AddressBook:
			{
				if( True == checkDisableAddressBook() ){
					// キーをグレーアウトにする.
					st_BadgeKeyInfo[uc_index].bl_isActivate = False;
				}
			}
			break;
		case PNLE_TopMenuShortCut_PrintMusic:
			st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_WebBrowser );
			break;
		case PNLE_TopMenuShortCut_SimpleSecuritySetup:				// 簡易セキュリティー設定.
		{
			Bool bl_ErrorCheck = False;
			if( EC_IntMode_On == onlyPanel.ec_IntMode ) {									// 割り込みモード中.
				bl_ErrorCheck = True;
			}
			if( NULL != PnlComGetMachineSetting() ) {
				if( TYPE_KeyModeVendorKM == PnlComGetMachineSetting()->getKeyMode() ) {		// ベンダー２装着(TYPE_KeyModeVendorKM).
					bl_ErrorCheck = True;
				}
			}
			APIC_SystemSetting* pc_SystemSetting = APIC_SystemSetting::newInstance();
			if( NULL != pc_SystemSetting ) {
				if( True == pc_SystemSetting->getLongPicturePrint() ) {						// 長尺印刷が設定されているとき.
					bl_ErrorCheck = True;
				}
			}
			if( False == PLBC_FuncVersionCheck::isOK_MiZe_Type3() ) {
				bl_ErrorCheck = True;
			}
			if( True == onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCallFirmwareAbnormal ) ) {
				bl_ErrorCheck = True;
			}
			// 非表示要因がある場合は。キーを非表示にする.
			if( True == bl_ErrorCheck ) {
				st_BadgeKeyInfo[uc_index].bl_isShow = False;
			}
		}
		break;
		case PNLE_TopMenuShortCut_Eco:
			if( PNLE_TopMenuShortCut_EcoCopy == e_TopMenuShortCut ) {
				st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Copy );
			}
			break;
		case PNLE_TopMenuShortCut_GadgetSetting:
			if( NULL == PnlComGetUser() )	{												// ユーザ認証ユーザなしの場合.
				st_BadgeKeyInfo[uc_index].bl_isActivate = False;
// comment 2021/04/30 Sky東島 [IT6_2.2][OP_BTS-32561].
//			} else if( ( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance()->getLine())	)	// 部門認証管理する.
//			           && ( NULL == PnlComGetTrack() ) ) {											// 部門認証なし.
			} else if ( True == PnlComGetNeedToTrackLogin() ) {											// 部門認証なし.
				st_BadgeKeyInfo[uc_index].bl_isActivate = False;
// comment 2016/10/07 SCD2-Sky佐藤 [IT5_3.3][1005928] ジョブ飛び越し対応.
			// 割り込みモード中の場合、使用不可.
			} else if( EC_IntMode_Off != onlyPanel.ec_IntMode ) {
				st_BadgeKeyInfo[uc_index].bl_isActivate = False;
			} else {
				st_BadgeKeyInfo[uc_index].bl_isActivate = True;
			}
			break;
// comment 2016/10/07 SCD2-Sky佐藤 [IT5_3.3][1005928] ジョブ飛び越し対応.
//		case PNLE_TopMenuShortCut_QRCode:				// QRコード表示.
//		{
//			Bool bl_Activate = True;
//// comment 2014/11/19 【HCL-Durga】【CQ-503465】Klockwork Changes ID:29382 [NPD.FUNC.MUST].				
//				const TYPS_ExtRegistration* pc_ExtRegistration = ScanOapExtSet.getApplication();
////				// OAPアプリ（スキャン(TypeD) 、スキャン(TypeS) 、PullPrint 、Browser(Scan) 、Browser(Pull)）動作中.
//////	if( ( True == ScanOapExtSet.isActiveGeneralApplication() )
//////		 && ( False == c_ExtPanel.getSpecifiedAppSetting() )
//////		bl_Activate = False;
//////	}
//
//			// OAPアプリ（スキャン(TypeD) 、スキャン(TypeS) 、PullPrint 、Browser(Scan) 、Browser(Pull)）動作中.
//			if( ( True == ScanOapExtSet.isActiveGeneralApplication() )
//			    && ( False == c_ExtPanel.getSpecifiedAppSetting() )
//					 && ( NULL != pc_ExtRegistration )
//					 && (	( TYPE_ExtTypeScanS == pc_ExtRegistration->s_Version.e_Type )
//						 || ( TYPE_ExtTypeScanD == pc_ExtRegistration->s_Version.e_Type )
//						 || ( TYPE_ExtTypePullPrint == pc_ExtRegistration->s_Version.e_Type )
//						 || ( TYPE_ExtTypeBrowserScan == pc_ExtRegistration->s_Version.e_Type )
//						 || ( TYPE_ExtTypeBrowserPull == pc_ExtRegistration->s_Version.e_Type )
//						 || ( TYPE_ExtTypeBrowserGeneral == pc_ExtRegistration->s_Version.e_Type ) ) ){
//				bl_Activate = False;
//			}
//
//			// IWSジョブ実行中.
//			if( True == onlyPanel.bl_IWSAction ) {
//				bl_Activate = False;
//			}
//			// ネットワーク初期化中.
//			APIC_NetworkDevice*		pc_NetworkDevice = APIC_NetworkDevice::newInstance();
//			// ネットワーク初期化中.
//			if( NULL != pc_NetworkDevice
//			    && True == pc_NetworkDevice->isNetworkInitialing() ) {
//				bl_Activate = False;
//			}
//			st_BadgeKeyInfo[uc_index].bl_isActivate = bl_Activate;
//
//		}
//		break;
		case PNLE_TopMenuShortCut_ControlMenu:
			if( ( False == SYSF_getAppOptionStatus( TYPE_AppOptionEXC ) )									// コンプリートモデルでない.
			    || ( True == onlyPanel.isWarningOn( comPanelJob, PNL_PATTENTION_ControllerNotAccess ) )		// コントローラ接続確立失敗警告が出ている.
			    || ( True == onlyPanel.isWarningOn( comPanelJob, PNL_PATTENTION_BrowserNonSetting ) )			// Webブラウザ無効警告が出ている.
			    || ( True == onlyPanel.isWarningOn( comPanelJob, PATTENTION_CheckControllerPowerSupply ) ) ) {	// Fiery電源OFF警告が出ている.
				//非表示とする.
				st_BadgeKeyInfo[uc_index].bl_isActivate = False;
			}
// comment 2016/10/07 SCD2-Sky佐藤 [IT5_3.3][1005928] ジョブ飛び越し対応.
			// 割り込みモード中の場合、使用不可.
			else if( EC_IntMode_Off != onlyPanel.ec_IntMode ) {
				st_BadgeKeyInfo[uc_index].bl_isActivate = False;
			}
// comment 2015/02/24 OPS1-Sky高橋 [ZeusS][CQ510436].
			else {
				st_BadgeKeyInfo[uc_index].bl_isActivate = True;
			}
			break;
// comment 2015/08/12 OPS1 Sky大濵 [ZeusS_ZX0][RQ-383]表示対象に”MarketPlace download”キーを新規追加する.
		case PNLE_TopMenuShortCut_MarketPlace:
			// ユーザー認証/部門認証で未認証.
			if( ( NULL == PnlComGetUser() ) 																	// ユーザ認証ユーザなしの場合.
				|| ( ( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance()->getLine()) )							// 部門認証管理する.
				 && ( TYPE_UserAuthenticationIntermeddiateServer != PnlComGetLoginUserAuthenticationType() )	// 拡張サーバー認証ユーザー以外の場合.
				 && ( NULL == PnlComGetTrack() ) ) ) {															// 部門認証なし.
				st_BadgeKeyInfo[uc_index].bl_isActivate = False;
			// 全画面警告発生時.
			} else if( False == bl_isWarningEnable ) {
				st_BadgeKeyInfo[uc_index].bl_isActivate = False;
			// ジョブ実行中.
			} else if( True == PnlComGetSystemStatus()->isJobExist() ) {
				st_BadgeKeyInfo[uc_index].bl_isActivate = False;
			} else {
				st_BadgeKeyInfo[uc_index].bl_isActivate = True;
				// ダウンロードキャンセル中.
				PNLC_ActionDataLib* pc_ActionLib = PNLC_ActionDataLib::newInstance();
				if( NULL != pc_ActionLib ) {
					if( PCGC_MarketPlaceDownloadStateDownloading == pc_ActionLib->getMarketPlaceDownloadState() ) {
						st_BadgeKeyInfo[uc_index].bl_isActivate = False;
					}
				}
			}
			break;
		// comment 2018/04/05 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
		// UIMode切り替えボタンの表示判定.	
		case PNLE_TopMenuShortCut_UIMode:
// 2018/10/11 SCD2-Sky鳫林 [Eagle][AR1063311]操作UIモード変更時にPanelジョブ設定をクリアする対応.
//			st_BadgeKeyInfo[uc_index].bl_isActivate = True;
			if ( EC_IntMode_On == onlyPanel.ec_IntMode ) {
				st_BadgeKeyInfo[uc_index].bl_isActivate = False;
			} else {
				st_BadgeKeyInfo[uc_index].bl_isActivate = True;
			}
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
// comment 2020/01/10 SCD2-Sky中野 [HeliosMLK][OP_BTS-18537].
//			if ( NULL == PnlComGetUser() ) {
			if( ( NULL == PnlComGetUser() )																							// ユーザー未ログイン.
// comment 2021/04/30 Sky東島 [IT6_2.2][OP_BTS-32561].
//			 || ( ( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode( PNLC_NetworkLan::getInstance()->getLine() ) )		// 部門管理：有効.
//			   && ( NULL == PnlComGetTrack() ) ) ) {																				// 部門未ログイン.
			 || ( True == PnlComGetNeedToTrackLogin() ) ) {																				// 部門未ログイン.
				st_BadgeKeyInfo[uc_index].bl_isActivate = False;
			}
//comment 2019/12/16 HCLT-Shalini [EagleBK] [OP_BTS-16194] Gray-out condition when Account Track is ON.
// comment 2021/04/30 Sky東島 [IT6_2.2][OP_BTS-32561].
//			if( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance()->getLine()) ) {
//				if( NULL == PnlComGetTrack() ) {
			if( True == PnlComGetNeedToTrackLogin() ) {
					st_BadgeKeyInfo[uc_index].bl_isActivate = False;
//				}
			}
// comment 2018/11/05 SCD2-Sky中野 [EagleH] WebUIv3FUM対応.
// comment 2022/04/28 Sky中野 [IT6_2.3] 制御No.43_システムオートリセット改善(WebUIv2/v3統合).
//			if( ( PNLE_BrowserChangeTabControlTabInitStatus_NotInit == PNLC_BrowserChangeTabControl::getInstance()->getTabInitStatus() )
//				|| ( PNLE_BrowserChangeTabControlTabInitStatus_Home == PNLC_BrowserChangeTabControl::getInstance()->getTabInitStatus() )	// Home画面タブ初期化が完了していない.
			if( ( PNLE_BrowserChangeTabControlTabInitStatus_Done != PNLC_BrowserChangeTabControl::getInstance()->getTabInitStatus() )		// タブ初期化が完了していない.
				|| ( False == PNLC_BrowserChangeTabControl::getInstance()->getEnableHomeScreenStatus() ) ) {								// Home画面へ遷移不可.
				st_BadgeKeyInfo[uc_index].bl_isActivate = False;
			}
			break;
		// comment 2018/04/18 【IT6_Eagle】【HCLT_Pavani】【RQ-675】【WP 3465】 Download and install MarketPlace on all MFPs according to the performance.
		case PNLE_TopMenuShortCut_MarketPlaceSetup:
// Comment 2018/10/08 [HCL-Pavani] [IT6_Eagle] [AR1060058] MarketPlace Application Not Applicable implementation commented.
//		case PNLE_TopMenuShortCut_MarketPlaceApplication:
			if( False == bl_isWarningEnable ) 	{								// If any warning Occurs.
				st_BadgeKeyInfo[uc_index].bl_isActivate = False;
			} else if( True == PnlComGetSystemStatus()->isJobExist() ) {		// If any job in progress.
				st_BadgeKeyInfo[uc_index].bl_isActivate = False;
			} else {
				st_BadgeKeyInfo[uc_index].bl_isActivate = True;
				PNLC_ActionDataLib* pc_ActionLib = PNLC_ActionDataLib::newInstance();
				if( NULL != pc_ActionLib ) {
// comment 2018/09/12 【IT6_Eagle】【HCLT_Pavani】【AR1053803】Support for Market Place Application Failure case.
//					if( PCGC_MarketPlaceDownloadStateDownloading == pc_ActionLib->getMarketPlaceDownloadState() ) {		// If download is in progress Setup Key should not be accepted.
					short ss_MarketPlaceDownloadState = pc_ActionLib->getMarketPlaceDownloadState();
					if( ( PCGC_MarketPlaceDownloadStateDownloading == ss_MarketPlaceDownloadState ) && ( True == PnlComGetIWSSetting()->getEnable() ) ){		// If download is in progress Setup Key should not be accepted.
						st_BadgeKeyInfo[uc_index].bl_isActivate = False;
					} else {
						st_BadgeKeyInfo[uc_index].bl_isActivate = True;
					}
				}
			}
			break;
// Comment 2019/06/08 [IT6_1.0MR2] AR1077690 [HCLT-Catherine]  Market Place key greyed out condition.
		case PNLE_TopMenuShortCut_MarketPlaceApplication:
			if( False == bl_isWarningEnable || True == PnlComGetSystemStatus()->isJobExist() ) 	{								// If any warning Occurs.
				st_BadgeKeyInfo[uc_index].bl_isActivate = False;
			} else {
				st_BadgeKeyInfo[uc_index].bl_isActivate = True;
			}
			break;				
// comment 2018/10/17 SCD2 K.Takei 不具合No.1063034 本体認証の場合、パブリックユーザーでログインし、WebUI画面にユニバーサルとユーティリティボタンがグレーアウトになってしまう.
		case PNLE_TopMenuShortCut_SoundSetting:		// 音設定.
			st_BadgeKeyInfo[uc_index].bl_isActivate = True;
			// 認証が有効で未ログイン時（Publicログインも含む）には、音設定は設定不可（グレイアウト）にする.
			if( (TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication(PNLC_NetworkLan::getInstance()->getLine())) ) {
				// ユーザー認証有効.
				APIC_User* pc_User = PnlComGetUser();
				if( (NULL == pc_User) ) {							// 未ログイン.
					st_BadgeKeyInfo[uc_index].bl_isActivate = False;
				}
			}
// comment 2021/04/30 Sky東島 [IT6_2.2][OP_BTS-32561].
//			if( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance()->getLine()) ) {
//				// 部門管理有効.
//				if( NULL == PnlComGetTrack() ) {					// 未ログイン.
			if( True == PnlComGetNeedToTrackLogin() ) {					// 未ログイン.
					st_BadgeKeyInfo[uc_index].bl_isActivate = False;
//				}
			}
			break;
		default :	break;
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： タブレット接続/切断時の処理.
//================================================================================================
void PCGC_HomeTopMenu::procTabletConnectChange( MessageType message, uchar option )
{
	// 親クラスのprocTabletConnectChange()をコールする.
	PPopupAnimationCom::procTabletConnectChange( message, option );

	// バッジキーに表示する情報の設定.
	initializeListView();

	// Qt描画要求.
	callBackQtShow( 0 );

// comment 2015/01/28 OPS1-MSE田中 [RQ-232][ZeusS][CQ509392]PanelLinkスキャンアイコン活性状態の初期化処理の見直し.
	setupGamen(0);
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： 画面開始処理.
//================================================================================================
void PCGC_HomeTopMenu::startGamen()
{
//comment 2019/02/19 [HCLT-Sai] [IT6_EagleH] [OP_BTS-2425,OP_BTS-2523] Display Warning Button click issue fix.
	m_bl_WarningOpenDispKey = False;
// comment 2019/05/20 SCD2-Sky肥後 [EagleH][OP_BTS-6914] PSWCからのデータ変化通知を受けても、WebUIの再起動要求をかけない.
////comment 2018/10/30 SCD2-Sky肥後 [Eagle][AR1061556]PSWCからの通知対応.
//	if( True == PNLC_WebUIControl::getInstance()->getPSWCDataChange() ) {
//		ObserverIndi( TYPE_PNL_Observer_WebUINotifyDataMethod );
//		PNLC_WebUIControl::getInstance()->setWebUIStartStatus(PNLE_WebUIStartStatusNeedStart);
//		PNLC_WebUIControl::getInstance()->setPSWCDataChange( False );
//	}
//// comment 2019/03/20 SCD2-4G中野 [IT6_1.0MR][1074292].
//	else if( True == PNLC_WebUIControl::getInstance()->getDeviceLockOff() ) {
//		PNLC_WebUIControl::getInstance()->setWebUIStartStatus( PNLE_WebUIStartStatusNeedStart );
//	} else {
//		// 処理なし.
//	}
//	PNLC_WebUIControl::getInstance()->setDeviceLockOff( False );

// comment 2019/5/21 Sky東島 [Eagle][AR1077047].
	onlyPanel.setBoxFromLightUIMode( False );		// ベーシックスタイルからボックスに遷移したフラグを落とす.
	
// comment 2017/11/21(火) SCD2-Sky長谷川(伸) [FQ-OPSD-1700523-OFP264].
	pc_ProgramKeyDataGetForTopMenu.setIGCProgramSearchStatus( PNLD_TOPMENU_PROGRAM_IGCWAITSEARCH_NONE );
// comment 2018/10/26 SCD2 T.Nishiba [Eagle][AR1064089]プログラム宛先の再検索を一度だけに制限する.
	pc_ProgramKeyDataGetForTopMenu.setProgramSearchRetryExcuted( False );
// comment 2018/04/04 SCD2 K.Nishiki [Eagle][制御構成変更No.111]ホーム画面認証画面の新UI化対応.
	PNLE_UIMode e_NowUIMode = PNLC_WebUIControl::getInstance()->getUIMode(); 	// UIモード取得.
 //Comment 2019/01/10 HCLT_Catherine [IT6_DenebMLK][1058157] To set the UI mode to Backup function.   
	PNLC_WebUIControl::getInstance()->setUIModeBackup(e_NowUIMode);
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//	if( PNLE_UIModeLightUI == e_NowUIMode ){									// UIモードがLightUIの場合.
//// comment 2018/09/09 SCD2-4G東島 [Eagle][1058366].
//		onlyPanel.b_nowBoxMode = False;		// ボックスモードを解除.
//		m_uc_TopMenuListKeyDisp = PNLD_TopMenuKeyDisp2;							// 1画面に表示されるキー数(Homeメニュー).
//		Cmp_TopMenuList.setTopMenuType(PNLE_MenuListTopMenuType_HomeMenu);		// Homeメニューのコンポーネント生成を指示.
//// comment 2018/09/01 SCD2 K.Nishiki [Eagle][1057237]ベーシックスタイルトップメニューの場合、ブラウザモード:OFF、トップメニューモード:ON.
//		onlyPanel.setBrowserMode( False );
//		onlyPanel.setTopmenuMode( True );
//	}else{ 																		// LegacyUIの場合.
//// comment 2018/09/07 HCL-Sai [IT6-Eagle][AR1069462].電源ON後、セキュリティー警告画面で、メニューキーを押下、WebUI画面からトップメニュー画面で、再度、メニューキーを押下、セキュリティー警告画面が表示されない.
//		if( True == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_SimpleSecurityWarning ) ) {
//			onlyPanel.setTopmenuMode( False );
//		}
//		m_uc_TopMenuListKeyDisp = PNLD_TopMenuKeyDisp;							// 1画面に表示されるキー数(標準トップメニュー).
//		Cmp_TopMenuList.setTopMenuType(PNLE_MenuListTopMenuType_TopMenu);		// 標準トップメニューのコンポーネント生成を指示.
//	}
// comment 2018/09/07 HCL-Sai [IT6-Eagle][AR1069462].電源ON後、セキュリティー警告画面で、メニューキーを押下、WebUI画面からトップメニュー画面で、再度、メニューキーを押下、セキュリティー警告画面が表示されない.
	if( True == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_SimpleSecurityWarning ) ) {
		onlyPanel.setTopmenuMode( False );
	}
	m_uc_TopMenuListKeyDisp = PNLD_TopMenuKeyDisp;							// 1画面に表示されるキー数(標準トップメニュー).
	Cmp_TopMenuList.setTopMenuType(PNLE_MenuListTopMenuType_TopMenu);		// 標準トップメニューのコンポーネント生成を指示.
	// comment 2018/09/10 SCD2-太田 [Eagle H][制御構成変更No.50]待ち時間測定の自動化.
	PFMG_WaitTimePerformLogging(PFMD_LogFormatNormal, PFMD_LogTypeScreenToTopMenu, PFMD_LogStatusScreenTransit);

// comment 2021/10/02 Sky東島 [IT6_2.2_MinervaSSBK][OP_BTS-35219].
// comment 2021/10/08 Sky東島 [IT6_MinervaSSBK][OP_BTS-34561].
	if ( ( EP_PNS_Setting == onlyPanel.ep_PanelStatus ) 
// comment 2022/04/28 Sky中野 [IT6_2.3] 制御No.43_システムオートリセット改善(WebUIv2/v3統合).
//	 &&  ( PNLE_BrowserChangeTabMode_UserTab == PNLC_BrowserChangeTabControl::getInstance()->getBrowserChangeTabMode()
	 &&  ( True == PNLC_WebUIControl::getInstance()->isDispWebUIApplication()
	    || PNLE_BrowserChangeTabMode_CustomUI == PNLC_BrowserChangeTabControl::getInstance()->getBrowserChangeTabMode() ) ) {
	  	// WebUIv2画面＞宛先登録画面＞トップメニュー画面と遷移させた場合にrebuildGamenが動作するとブラウザ画面へ遷移してしまうためトップメニューフラグを設定しておく.
		onlyPanel.setTopmenuMode( True );
	}
	if( True == onlyPanel.isTopmenuMode() ){
		// トップメニューフラグをブラウザフラグが同時に設定されている場合に,rebuildGamenでトップメニューが２重で表示されてしまうためブラウザフラグを解除しておく.
		onlyPanel.setBrowserMode( False );
	}
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//// comment 2018/09/08 SCD2-Sky水野 [IT6][AR1058855].
//	// HOME<->トップメニュー切替時にメンテナンス解除キーちらつき防止.
//	if( PNLE_UIModeLightUI == e_NowUIMode ){									// UIモードがLightUIの場合.
//		Cmp_HomeTopMenuMessageArea.showEnable( False );							// メッセージエリア非表示.
//		Cmp_HomeTopMenuMessageAreaHomeMenu.showEnable( True );					// 新UIメッセージエリア.
//	}else{ 																		// LegacyUIの場合.
//		Cmp_HomeTopMenuMessageArea.showEnable( True );							// メッセージエリア非表示.
//		Cmp_HomeTopMenuMessageAreaHomeMenu.showEnable( False );					// 新UIメッセージエリア.
//	}

// comment 2016/10/06 MSE岡田 [IT5_3.3][1005694] 宛先検索要求フラグがたっていればIGCWaitリクエスト通知を行う.
	// トップメニューに短縮宛先を使用するプログラムキーが設定されていた場合、.
	// 省電力からの復帰時間短縮のため、宛先検索要求を遷移前ではなく遷移後に行う.
	if( True == m_bl_RequestProgramKeyAddressSearch ) {
		m_bl_RequestProgramKeyAddressSearch = False;
// comment 2017/11/21(火) SCD2-Sky長谷川(伸) [FQ-OPSD-1700523-OFP264].
		pc_ProgramKeyDataGetForTopMenu.setIGCProgramSearchStatus( PNLD_TOPMENU_PROGRAM_IGCWAITSEARCH_SEARCHING );
		PCGC_PanelMain::IGCWaitReqIndi( 0 );
	}

	// comment 2015/10/20 OPS1-川手 [特ROM対応][FQ-OPSD-1500259-OFP185]Fax受信時に受信エラー(R27)が頻繁に発生する.
	m_bl_UpdateProgramKeyStatus = True;
// comment 2015/03/17 OPS1-Sky佐藤 [ZeusS][CQ511108] 言語情報の取得／設定処理見直し.
	// マシン設定が右詰め表示言語の時はメッセージのロードしなおす.
// comment 2015/06/29 OPS1-Sky佐藤 [ZeusS_ZX0][RQ-405] パネルUIのヘブライ語対応.
//	TYPE_Language ec_Type = PnlComGetMachineSetting()->getLanguage();
//	if ( TYPE_LanguageArabic == ec_Type ) {
//		loadSelectLangType_IOP( PnlComGetLanguage() );
//	}
// comment 2016/01/28 OPS1-Sky浜野 [ZeusS_ZX0][CQ524401] Myパネル対応.
//	TYPE_Language ec_Type;
//	if ( PnlComGetOperationSetting()->getLanguageChoiceSetting() ) {
//		ec_Type = onlyPanel.e_Language;
//	} else {
//		ec_Type = PnlComGetMachineSetting()->getLanguage();
//	}
//	if ( True == isRightDispLanguage( ec_Type ) ) {
//		loadSelectLangType_IOP( PnlComGetLanguage() );
//	}
	TYPE_Language e_CheckLanguage = PnlComGetLanguage( False );
	if ( True == isRightDispLanguage( e_CheckLanguage ) ) {
		loadSelectLangType_IOP( PnlComGetLanguage() );
	}

	PNLC_UICustomDataManager* pc_DataMan = PNLC_UICustomDataManager::newInstance();
	if( NULL != pc_DataMan ) {
		pc_DataMan->setTopMenuThemeForWarp(); // Topメニュー背景カスタマイズ画像設定.
	}

	if( True == PNLC_HitachiICC_TopMenuCustomize::newInstance()->isHitachiICC_Mode() ) {	// 日立ICCモード時は、トップメニューのQt表示を無効にする.
		PLBC_QtListNotUse::getInstance()->setQtListNotUseFlg( True );
	}

	if( True ==  onlyPanel.isTopmenuMode() ) {
		// 優先アプリ設定によりトップメニューへ遷移した場合は、セキュリティエリアから離脱したと判断する.
		// *) セキュリティエリアとは、パスワード付のボックスを指す.
		PNLC_TabletSecurityControlLib* pc_TabletSecurityControl = PNLC_TabletSecurityControlLib::newInstance();
		if( NULL != pc_TabletSecurityControl ) {
			pc_TabletSecurityControl->securityAreaExited();
		}
	}
	
// comment 2019/06/27 SCD2-Sky谷尾 [EagleZ+][OP_BTS-7397]ウィジェット再描画処理追加,
	PM_TaskID uc_PriTaskID;												// 最優先タスク取得.
	PMGC_PanelManager::getPriority( &uc_PriTaskID );
	if( PM_TASKID_PANEL != uc_PriTaskID )
	{
		m_bl_PriorityTaskBrw = True;
	}
	Cmp_GadgetComponent.initialize( APIE_GadgetShowScreenTopMenu );		// 2013/11/15 s.ohba 操作性改善FUM5 ウィジェット配置対応.

	// 親クラスのstartGamen()をコールする.
	PPopupAnimationCom::startGamen();
//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] 起動時に画面がちらつく.
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.
	if( PNLE_UIModeLegacyUI == e_NowUIMode ) {
		PNLC_QWSConnector* pc_QWSConnector = PNLC_QWSConnector::newInstance();
		if( NULL != pc_QWSConnector ) {
			pc_QWSConnector->setImageTransfer( True );				// 画像転送を再開する.
		}
	}
#endif //#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
	// 背景パーツのキャッシュ有効化.
	switch( PnlComGetSystemSetting()->getTopMenuCustomPattern() ) {
		case TYPE_TopMenuCustomPattern1: Bak_HomeTopMenuBackground[0].setUseCache( True ); break;
		case TYPE_TopMenuCustomPattern2: Bak_HomeTopMenuBackground[1].setUseCache( True ); break;
		case TYPE_TopMenuCustomPattern3: Bak_HomeTopMenuBackground[2].setUseCache( True ); break;
		default:	break;
	}

	//既存コンポーネントインスタンス設定.
	PnlComMtuiIf_TopMenuModel::instance()->setComponent( &Cmp_TopMenuList );
	PnlComMtuiIf_TopMenuView::instance()->setComponent( &Cmp_TopMenuList );
	bl_AppButtonDisplay = False;	//CQ:362522【ZeusMLK】【OapV4.0マージ】【360037】【IWS Browser UI application is allowed when OAP access settings is Restrict】【HCL_Amol】 2012/05/14 Add.
	this->m_sl_SupportUserNo = 0 ;
	if( PnlComGetUser() ) {
		this->m_sl_SupportUserNo = PnlComGetUser()->getUserNo() ;
	}

	this->m_sl_SupportTrackNo = 0 ;
	if( PnlComGetTrack() ) {
		this->m_sl_SupportTrackNo = PnlComGetTrack()->getTrackNo() ;
	}

	initRegisterApp();

	m_bl_BrowserFlag = False;		// ブラウザが割り付けられているか.
	m_bl_AppFlag = False;			// APPが割り付けられているか.
	API_RESULT ret = LAPE_ResultOK;

	std::vector<LAPC_APP_RegistAppData> p_AppDataList;//For passing argument
	s_registerAppIws.clear();

	LAPC_APP_RegistAppDataList APP_RegistAppDataList( LAPC_BFW_System::newInstance() );
	ret=APP_RegistAppDataList.getRegistAppList( &p_AppDataList );

	int32_t vec_size = static_cast<int32_t>( p_AppDataList.size() );

	LAPC_APP_RegistAppData APP_ResgistAppData( LAPC_BFW_System::newInstance() );
	LAPE_APP_AppKind      e_AppKind=0;

	if( ret==LAPE_ResultOK ) {
		for( int i=0; i<vec_size; i++ ) {
			p_AppDataList[i].getAppType( &e_AppKind );
			if( LAPE_APP_AppKindIWS==e_AppKind ) {
				s_registerAppIws.push_back( p_AppDataList[i] );
			}
		}
	}

	uc_nowPage = 1;			// 1ページ目を表示する.
// commemt 2019/09/03 SCD2-Sky二口 [EagleH][OP_BTS-12799] MarketPlaseがインストールされているのにSetupが表示されてしまう.
//	initializeListView();		// リスト表示.

	if( True == getDBInitializeStatus() ) {
		m_bl_isDBInit = True;
	} else {
		m_bl_isDBInit = False;
	}
// commemt 2019/09/03 SCD2-Sky二口 [EagleH][OP_BTS-12799] MarketPlaseがインストールされているのにSetupが表示されてしまう.
	initializeListView();		// リスト表示.
	
	// ビュー表示.
	callBackQtShow( 0 );		// Qt描画要求.

// comment 2023/09/14 Sky東島 [IT6_2.3][OP_BTS-44767].
	APIC_UsbMemDeviceInfo*	pc_UsbMemDeviceInfo = APIC_UsbMemDeviceInfo::newInstance();
	if( ( NULL != pc_UsbMemDeviceInfo )
	 && ( 0 == pc_UsbMemDeviceInfo->getStatus() ) ){
	 	// 外部メモリが非装着になったときの更新フラグ済みフラグを設定.
	 	// このフラグが設定されていないと,外部メモリが非装着の場合に.
	 	// 画面遷移時のsetupGamenでリスト表示/Qt描画要求が再度行われ時間がかかってしまう.
		m_bl_isUSBInit = True;
	}
	if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type3() ) {
		if( False == AplCommonSet.isActiveGeneralApplication( TYPE_JobTypeCopy, PNLE_CheckType_ALL ) ) {
			TYPE_JobType e_JobType = comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType();
			JBLE_OAP_FUNCTION_TYPE i_MenuType;
			if( True == onlyPanel.b_nowBoxMode ) {
				i_MenuType = JBLE_OAP_BOX;
			} else if( True == onlyPanel.getBrowserMode() ) { // ブラウザモード時.
				i_MenuType = JBLE_OAP_BROWSER;
			} else {
				switch( e_JobType )
				{
					case TYPE_JobTypeCopy:
					case TYPE_JobTypeIntCopy:
						i_MenuType = JBLE_OAP_COPY;
						break;

					case TYPE_JobTypeFaxSendQuick:
					case TYPE_JobTypeFaxSendMemory:
					case TYPE_JobTypeFaxSendTimer:
					case TYPE_JobTypeFaxReceivePolling:
					case TYPE_JobTypeFaxReceiveTimer:
					case TYPE_JobTypeFaxReceive:
//							i_MenuType = JBLE_OAP_SCANFAX;
						if( True == onlyPanel.getFaxAppMode() ) {
							i_MenuType = JBLE_OAP_FAXONLY;
// comment 2015/01/31 OPS1-MSE大島 [ZeusS][RQ-232][CQ509389]PanelLinkスキャン画面で登録キーから登録アプリ(PullPrint)起動後、終了ボタンを選択するとファクススキャン画面に遷移する.
							} else if( True == comPanelJob->getJobSpec()->pc_Send->c_Twain.getScanToAndroid() ) {
								i_MenuType = JBLE_OAP_SCANTODEV;
						} else {
							i_MenuType = JBLE_OAP_SCANFAX;
						}
						break;

					default:
						i_MenuType = JBLE_OAP_NONE;
						break;
				}
			}
			// RightFaxからメニューへ遷移すると、ジョブタイプがファクススキャンになるが、.
			// Scan機能制限(Disable)の状態でメニューからRightFax起動し終了すると、宛先等選べないファクススキャンに遷移するため戻り先を設定しない.
			if( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Scan )	// Scan送信禁止.
			    && True == ExtSet.isRegistGeneralApplication( TYPE_JobTypeFaxSendMemory )	// FaxApp登録有.
			    && True == ExtSet.getFaxAppOn() ) {											// RightFax(FaxAPP)起動中.
				// なにもしない.
			} else {
				PMessageArea::setAppFunctionType( i_MenuType );
			}
		}
	} else {
		if( False == ScanOapExtSet.isActiveGeneralApplication() )
		{
			TYPE_JobType e_JobType = comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType();
			JBLE_OAP_FUNCTION_TYPE i_MenuType;
			if( True == onlyPanel.b_nowBoxMode ) {
				i_MenuType = JBLE_OAP_BOX;
			} else if( True == onlyPanel.getBrowserMode() ) { // ブラウザモード時.
				i_MenuType = JBLE_OAP_BROWSER;
			} else {
				switch( e_JobType )
				{
					case TYPE_JobTypeCopy:
					case TYPE_JobTypeIntCopy:
						i_MenuType = JBLE_OAP_COPY;
						break;

					case TYPE_JobTypeFaxSendQuick:
					case TYPE_JobTypeFaxSendMemory:
					case TYPE_JobTypeFaxSendTimer:
					case TYPE_JobTypeFaxReceivePolling:
					case TYPE_JobTypeFaxReceiveTimer:
					case TYPE_JobTypeFaxReceive:
						i_MenuType = JBLE_OAP_SCANFAX;
						break;

					default:
						i_MenuType = JBLE_OAP_NONE;
						break;
				}
			}
			// RightFaxからメニューへ遷移すると、ジョブタイプがファクススキャンになるが、.
			// Scan機能制限(Disable)の状態でメニューからRightFax起動し終了すると、宛先等選べないファクススキャンに遷移するため戻り先を設定しない.
			if( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Scan )	// Scan送信禁止.
			    && True == ExtSet.isRegistGeneralApplication( TYPE_JobTypeFaxSendMemory )	// FaxApp登録有.
			    && True == ExtSet.getFaxAppOn() ) {											// RightFax(FaxAPP)起動中.
				// なにもしない.
			} else {
				PMessageArea::setAppFunctionType( i_MenuType );
			}
		}
	}//【CQ380160】【Closing Browser Launcher Application on starting another Browser Launcher Application】【HCL_Vimal】2013/05/27 Add.
	// 警告状態によるアプリケーション選択チェックをsetupGamenからprocTimerWakeUpに変更する.
	bl_isWarningEnable = True;
// comment 2015/09/11 OPS1-Sky藤岡 [ZeusS_ZX0][CQ516675].
	m_e_WarningName = TYPE_WarningName_Non;

	PWarningcomIF* pc_WarningcomIF = PWarningcomIF::newInstance();
	if( NULL != pc_WarningcomIF ) {
		// 全画面警告発生のチェック.
		ushort us_warntbl = onlyPanel.isAllGamenWarning( onlyPanel.cc_DisplayJob );
		// ログインユーザーに関連するジョブで金額不足警告が発生していた場合のみ、金額不足警告画面は表示させる.
		if( ( DEF_sp_GamenWarningTable > us_warntbl )
		    && ( False == onlyPanel.isShortChargeWarningOn( sp_GamenWarningTable[ us_warntbl ].ec_WarningName, onlyPanel.cc_DisplayJob ) ) ) {
			us_warntbl = 0;
		}
		if( 0 != us_warntbl ) {
			bl_isWarningEnable = False;
// comment 2015/09/11 OPS1-Sky藤岡 [ZeusS_ZX0][CQ516675].
			m_e_WarningName = sp_GamenWarningTable[us_warntbl].ec_WarningName;
		}
	}
	// スリープからのトップメニュー復帰時は立上げ中画面が終了してないため準備完了音を鳴動する.
	if( ( &gp_ActionInitialSetup == nowGamen )          // ベース画面が立ち上げ中画面.
	    && ( PCGE_BuzzerType_Off == getBuzzerType() ) ) {  // ブザー未設定.
		setBuzzerType( PCGE_BuzzerType_PreparationOk );
	}

	//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/10 Add Start.
	if(gp_CostCenterResetDummy.bl_HeaderArea == True){
		gp_CostCenterResetDummy.bl_HeaderArea = False;
		procButtonOn( PNLE_BTNID_HomeTopMenu_LogInInfo );
	}
	//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/10 Add End.
// comment Sky東島 [RQ-686] 複合機の複数ネットワーク対応.
	m_bl_VlanNWShow = False;
	if( True == PNLC_NetworkLan::getInstance()->getVlanSetting() ){									// VLAN設定有効.
// comment 2019/05/28 [HCL-Naveena][EagleBK] NetworkType Default selection activity.
//		if( ( TYPE_NetworkSpecific_NONE == PNLC_NetworkLan::getInstance()->getLine() )				// ネットワークが選択されていない.
		if( ( TYPE_NetworkSpecific_NONE == PNLC_NetworkLan::getInstance()->getLineForPanel() )				// ネットワークが選択されていない.
		 || ( NULL == PnlComGetUser() )																			// ユーザー未ログイン.
		 || ( ( TYPE_UserAuthenticationOff == ppc_UserAuthentication->getUserAuthentication(PNLC_NetworkLan::getInstance()->getLine()) )					// 認証設定なし.
// comment 2022/07/27 Sky中野 [IT6_2.2MR3][OP_BTS-38572].
//			  && ( TYPE_TrackModeOff == ppc_TrackManager->getTrackMode(PNLC_NetworkLan::getInstance()->getLine()) ) )
			  && ( TYPE_TrackModeOff == PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance()->getLine()) ) )
// comment 2021/04/30 Sky東島 [IT6_2.2][OP_BTS-32561].
//		 || ( ( TYPE_TrackModeOff != ppc_TrackManager->getTrackMode(PNLC_NetworkLan::getInstance()->getLine()) )											// 部門未ログイン.
//			   && ( NULL == PnlComGetTrack() ) ) ){
		 || ( True == PnlComGetNeedToTrackLogin() ) ){
			m_bl_VlanNWShow = True;
		}
	}

// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.	
//// comment 2018/09/07 SCD2-Sky中野 [Sparrow][1057471] WebUI画面でメニューキー(外キー)押下時の動作不正対応.
//	if( ( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() )
//	 && ( PNLE_WebUIStartStatusStarted == PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//		if( True == PNLC_WebUIControl::getInstance()->isNeedGotoHome() ) {
//			// GotoHome通知.
//			PNLC_WebUIControl::getInstance()->requestGoToHome();
//			PNLC_WebUIControl::getInstance()->setNeedGotoHome( False );
//		}
//	}
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： 表示.
//================================================================================================
void PCGC_HomeTopMenu::show()
{
	PPopupAnimationCom::show();

	PNLC_UICustomDataManager* pc_DataMan = PNLC_UICustomDataManager::newInstance();
	if( NULL != pc_DataMan ) {
		const uchar* puc_Data = NULL;
		switch( PnlComGetSystemSetting()->getTopMenuCustomPattern() ) {
			case TYPE_TopMenuUserCustomPattern1:	// ユーザーテーマ選択されている場合は、画像取得できていたらここで変更.
			case TYPE_TopMenuUserCustomPattern2:
			case TYPE_TopMenuUserCustomPattern3:
				puc_Data = pc_DataMan->getTopMenuThemeImage();
				if( NULL != puc_Data ) {
					Bak_HomeTopMenuUserThemeBackground.changeRawGraphic( puc_Data );
				}
				break;
			default:								// デフォルトのテーマの場合はすでに設定済みの為 特に何もしない.
				break;
		}
	}
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： 画面開始共通処理(start/activate).
//================================================================================================
void PCGC_HomeTopMenu::initializeListView()
{
	int countMax = PNLD_TOPMENU_SHORTCUTICON_MAX;
	// アイコンデータの準備.
	ScanOapExtSet.updateIconDataAppSelect( s_registerApp, m_us_TotalCountOap );
// comment 2018/04/04 SCD2 K.Nishiki [Eagle][制御構成変更No.111]ホーム画面認証画面の新UI化対応.
// LightStyleとLegacyStyleの切り替え.
// comment 2017/11/21(火) SCD2-Sky長谷川(伸) [FQ-OPSD-1700523-OFP264].
	pc_ProgramKeyDataGetForTopMenu.setNeedProgramSearchFlg( False );
// comment 2018/11/05 SCD2-Sky中野 [EagleH] WebUIv3FUM対応.
//	if( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() ){					// LightUIの場合
//		setBadgeKeyData_HomeMenu();																// LightUI用のバッジキー表示情報設定
//// comment 2018/05/24 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
//		countMax = PNLD_HOMEMENU_SHORTCUTICON_MAX;			// BasicStyle キー表示上限24キー.
//	}else{
		if( True == PNLC_HitachiICC_TopMenuCustomize::newInstance()->isHitachiICC_Mode() ) {
			setBadgeKeyData_HitachiICC();
		} else {
			setBadgeKeyData();		// バッジキーに表示する情報の設定.
		}
// comment 2018/11/05 SCD2-Sky中野 [EagleH] WebUIv3FUM対応.
//	}
// comment 2019/1/8 SCD2-Sky肥後 [EagleL] 文字列が長くなった時にフォントを変更する対応.
	Bool bl_ChangeFont = False;
	PnlComMtuiIf_TopMenuModel::instance()->clear();
	for( int idx = 0; idx < countMax; idx++ ) {
		char* pc_FuncName = st_BadgeKeyInfo[idx].sc_FuncName;
// comment 2019/1/8 SCD2-Sky肥後 [EagleL] 文字列が長くなった時にフォントを変更する対応.
		bl_ChangeFont = False;
		if( 0 != st_BadgeKeyInfo[idx].tid_FuncName ) {
			static char pc_FuncNameBuffer[255];
			memset( pc_FuncNameBuffer, 0x0, sizeof( pc_FuncNameBuffer ) );
			PnlComConvertTextIDToString( st_BadgeKeyInfo[idx].tid_FuncName, pc_FuncNameBuffer, sizeof( pc_FuncNameBuffer ) );
			pc_FuncName = pc_FuncNameBuffer;
		}
// comment 2019/1/8 SCD2-Sky肥後 [EagleL] 文字列が長くなった時にフォントを変更する対応.
		// KONICAMINOLTA MarketPlaceの文字に合わせ、25文字以上の場合はフォントを変更する.
		PLBC_CUtf8 Utf8( pc_FuncName );
		if ( PNLD_FontChangeThreshold < Utf8.GetCount() ) {
			bl_ChangeFont = True;
		}
		uchar* puc_FuncIcon = st_BadgeKeyInfo[idx].puc_FuncIcon;
		uchar uc_IconPaletteType = st_BadgeKeyInfo[idx].uc_IconPaletteType;
		// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] データサイズを取り出し.
// comment 2020/07/17 Sky東島 [IT6_2.0MR3][OP_BTS-25505].
//		short ss_FuncIconDataSize = st_BadgeKeyInfo[idx].ss_FuncIconDataSize;
		ushort us_FuncIconDataSize = st_BadgeKeyInfo[idx].us_FuncIconDataSize;
		if( 0xFFFF != us_OAPAppNo[idx] ) {
			for( uchar uc_IconListNo = 0; uc_IconListNo < m_us_TotalCountOap; uc_IconListNo++ ) {	// OAPアプリ数回チェックする.
				if( s_registerApp[uc_IconListNo] == ( short )us_OAPAppNo[idx] ) {
					const TYPS_ExtIconDetail* sIconDetailBuf = ScanOapExtSet.getIconAppSelect( uc_IconListNo );
					if( sIconDetailBuf && ( sIconDetailBuf->e_Entry != TYPE_ExtEntryOff ) && ScanOapExtSet.getIconCheck( *sIconDetailBuf ) ) {
						puc_FuncIcon = ( uchar* )sIconDetailBuf->s_IconData.sc_Data;
						uc_IconPaletteType = sIconDetailBuf->sc_PaletteNo;
						// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] アプリケーション毎アイコンのデータサイズを取り出し.
// comment 2020/07/17 Sky東島 [IT6_2.0MR3][OP_BTS-25505].
//						ss_FuncIconDataSize = sIconDetailBuf->s_IconData.ss_DataSize;
						us_FuncIconDataSize = sIconDetailBuf->s_IconData.ss_DataSize;
// comment 2020/6/26 Sky東島 [IT6_2.0MR3][制御構成変更_No.37]SVGサポート対応.
						st_BadgeKeyInfo[idx].bl_SvgData = ScanOapExtSet.isExtIconFormatSvg( sIconDetailBuf->e_IconFormat );
// comment 2019/02/27 SCD2-Sky谷尾 [Eagle]MR1.0 制御構成変更No.3 IT 登録アプリケーションを割り付けた際ホームメニューでフリーズする不具合修正.
						if( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() ){					// LightUIの場合
// comment 2019/05/17 [HCLT-Prem] [IT6_MR1.0][AR1077206]Assigning icon value of Classic screen to NULL if the open screen is basic screen.
							if( PNLE_TopMenuShortCut_MarketPlaceApplication == st_BadgeKeyInfo[idx].e_ShortCutKind ) {
								puc_FuncIcon = NULL;
							}
							st_BadgeKeyInfo[idx].bl_isShowIcon = True;
						}
					} else {
						if( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() ){					// LightUIの場合
							puc_FuncIcon = NULL;
							st_BadgeKeyInfo[idx].bl_isShowIcon = False;
						}
					}
					break;
				}
			}
		}
		
// comment 2018/04/04 SCD2 K.Nishiki [Eagle][制御構成変更No.111]ホーム画面認証画面の新UI化対応.		
		uchar* puc_FuncIcon_n = st_BadgeKeyInfo[idx].puc_FuncIcon_n;						// 非選択.
		uchar* puc_FuncIcon_s = st_BadgeKeyInfo[idx].puc_FuncIcon_s;						// 選択.
		uchar* puc_FuncIcon_d = st_BadgeKeyInfo[idx].puc_FuncIcon_d;						// 非活性.
		uchar uc_IconPaletteLightUI = st_BadgeKeyInfo[idx].uc_IconPaletteLightUI;			// パレットタイプ.
// Comment 2019/04/10 【HCL-Catherine】 【IT6_1.0MR】 【AR1075814】 To display the Market place icon.
		if( (PNLE_TopMenuShortCut_MarketPlace == st_BadgeKeyInfo[idx].e_ShortCutKind )
				&& st_BadgeKeyInfo[idx].bl_isShowIcon == False ) {
				if ( True == PnlComGetIWSSetting()->getMarketPlaceAppUse()){
					st_BadgeKeyInfo[idx].bl_isShowIcon = True; //Always Display Icon for Market Place.
					puc_FuncIcon = ( uchar* )NULL;
// comment 2020/07/17 Sky東島 [IT6_2.0MR3][OP_BTS-25505].
//					ss_FuncIconDataSize = 0;
					us_FuncIconDataSize = 0;
					uc_IconPaletteType = 0;
#if defined(DEF_LCD_7INCH)
					puc_FuncIcon_n = ( uchar* )g70_026_000_n_X108_Y108;
					puc_FuncIcon_s = ( uchar* )g70_026_000_s_X108_Y108;
					puc_FuncIcon_d = ( uchar* )g70_026_000_d_X108_Y108;
#else // 10.1inch
					puc_FuncIcon_n = ( uchar* )g70_011_000_n_X124_Y124;
					puc_FuncIcon_s = ( uchar* )g70_011_000_s_X124_Y124;
					puc_FuncIcon_d = ( uchar* )g70_011_000_d_X124_Y124;
#endif  // DEF_LCD_7INCH
					st_BadgeKeyInfo[idx].uc_IconPaletteLightUI = PNLD_PaletteNewUIApp;
				}
			} 
// comment 2019/02/08 SCD2-Sky谷尾 [Eagle]ホームトップメニューへ登録アプリケーションの割り当てを可能とする対応.
// comment 2020/07/17 Sky東島 [IT6_2.0MR3][OP_BTS-25505].
//		bool bl_IsApplicableLegacyIconLayout = PnlComMtuiIf_SelectAppListModel::instance()->isApplicableLegacyIconLayout( puc_FuncIcon, ss_FuncIconDataSize );	// 従来サイズ(48x48)のアイコン表示レイアウトを適用するか.
		bool bl_IsApplicableLegacyIconLayout = PnlComMtuiIf_SelectAppListModel::instance()->isApplicableLegacyIconLayout( puc_FuncIcon, us_FuncIconDataSize );	// 従来サイズ(48x48)のアイコン表示レイアウトを適用するか.
// comment 2020/6/26 Sky東島 [IT6_2.0MR3][制御構成変更_No.37]SVGサポート対応.
		Bool bl_SvgData = st_BadgeKeyInfo[idx].bl_SvgData;
		// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] データサイズの設定を追加.
		PnlComMtuiIf_TopMenuModel::instance()->addDataToSubmit(
		    idx
		    , st_BadgeKeyInfo[idx].e_ShortCutKind
		    , pc_FuncName
		    , puc_FuncIcon
		    , uc_IconPaletteType
		    , st_BadgeKeyInfo[idx].bl_BMP
		    , st_BadgeKeyInfo[idx].bl_isShow
		    , st_BadgeKeyInfo[idx].bl_isActivate
		    , st_BadgeKeyInfo[idx].bl_isShowIcon
// comment 2020/07/17 Sky東島 [IT6_2.0MR3][OP_BTS-25505].
//		    , ss_FuncIconDataSize
		    , us_FuncIconDataSize
// comment 2018/04/04 SCD2 K.Nishiki [Eagle][制御構成変更No.111]ホーム画面認証画面の新UI化対応.
			, puc_FuncIcon_n											// 非選択.
			, puc_FuncIcon_s											// 選択.
			, puc_FuncIcon_d											// 非活性.
			, uc_IconPaletteLightUI										// パレットタイプ.
// comment 2019/1/8 SCD2-Sky肥後 [EagleL] 文字列が長くなった時にフォントを変更する対応.
			, bl_ChangeFont												// フォント変更要否.
// comment 2019/02/08 SCD2-Sky谷尾 [Eagle]ホームトップメニューへ登録アプリケーションの割り当てを可能とする対応.
			, bl_IsApplicableLegacyIconLayout							// 従来サイズ(48x48)のアイコン表示レイアウトを適用するか.
			, bl_SvgData												// SVGファイル判定 // comment 2020/6/26 Sky東島 [IT6_2.0MR3][制御構成変更_No.37]SVGサポート対応.
		);
		// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] 旧GraphicManagerへの更新処理を削除.
		//	if( True == st_BadgeKeyInfo[idx].bl_isShowIcon ) {
		//		PnlComMtuiIf_TopMenuModel::instance()->updateImage( puc_FuncIcon, uc_IconPaletteType, st_BadgeKeyInfo[idx].bl_BMP );
		//	}
	}
// comment 2015/10/20 OPS1-川手 [特ROM対応][FQ-OPSD-1500259-OFP185]Fax受信時に受信エラー(R27)が頻繁に発生する.
// submitDatasは、setupGamenで行い、ちらつきを抑制する.
//        PnlComMtuiIf_TopMenuModel::instance()->submitDatas();
//    }
// comment 2015/09/11 OPS1-MSE竹中[MinervaSBK][CQ514099] アイコンリスト情報更新後に必ずEco設定フラグを更新するように修正.
	// トップメニューキーにEco設定キーが割り当てられているかチェックする.
	checkExistEcoFuncShortcut();
}


//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： 表示部品のセットアップ.
//				ZeusCD以降、リモートパネル接続中でもブラウザ/OAPの起動を可能とする.
//================================================================================================
void PCGC_HomeTopMenu::setupGamen( short option )
{
// comment 2018/04/11 SCD2 Y.Yamabe [Eagle][制御構成変更No.111]ホーム画面認証画面の新UI化対応.
// comment : WebUI2 画面切替え制御 -> 20180302 Chikuan.
//	// UIのモードの違いによりパーツの表示状態を切替る.
//	changeLayoutByUIMode();
//
//	if( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() ){
//		setupGamenLightUI( option );
//		return;
//	}
// comment : WebUI2 画面切替え制御 <- 20180302 Chikuan.

	if( False == m_bl_isDBInit ) {
		if( True == getDBInitializeStatus() ) {			// DB初期化完了の場合.
			initRegisterApp();			// アプリ登録情報初期化.
// commemt 2019/09/03 SCD2-Sky二口 [EagleH][OP_BTS-12799] MarketPlaseがインストールされているのにSetupが表示されてしまう.
//			initializeListView();		// 画面開始共通処理.
			m_bl_isDBInit = True;
			initializeListView();		// 画面開始共通処理.
			// comment 2015/10/20 OPS1-川手 [特ROM対応][FQ-OPSD-1500259-OFP185]Fax受信時に受信エラー(R27)が頻繁に発生する.
			m_bl_UpdateProgramKeyStatus = True;			// DB初期化完了時、プログラムキーの活性状態の更新を行う.
		}
	}
// Comment 2016/08/23 【HCLT-Harish】 【IT5_3.3】【1003031】Display and No Display of External APP key in Home Top Menu.
	if ( ( True == bl_AppButtonDisplay ) 
		&& ( ( False == ScanOapExtSet.isRegistApplication() ) && ( 0 == ExtSet.countApps() ) ) ) {
		bl_AppButtonDisplay = False;
		initializeListView();		// 画面開始共通処理.
	} else if ( ( False == bl_AppButtonDisplay ) 
		&& ( ( True == ScanOapExtSet.isRegistApplication() ) || ( 0 < ExtSet.countApps() ) ) ) {
		initRegisterApp();			// アプリ登録情報初期化.
// comment 2020/07/27 Sky東島 [IT6_2.0MR3][OP_BTS-24616].
//		initializeListView();		// 画面開始共通処理.
		if( True == bl_AppButtonDisplay ){
			// アプリキーの表示フラグが表示中になった場合のみ更新処理を行う.
			initializeListView();			// アプリ登録情報初期化.
		}
	}
// comment 2016/03/24 OPS1-Sky佐藤 [ZeusS_ZX0][CQ526692] 外部メモリーキーの非表示対応見直し.
	APIC_UsbMemDeviceInfo*	pc_UsbMemDeviceInfo;
	pc_UsbMemDeviceInfo = APIC_UsbMemDeviceInfo::newInstance();
	if ( NULL != pc_UsbMemDeviceInfo ) {
		if ( 0 == pc_UsbMemDeviceInfo->getStatus() ) {
			if ( False == m_bl_isUSBInit ) {
				initializeListView();		// バッジキーに表示する情報の設定.
				callBackQtShow( 0 );		// Qt描画要求.
				m_bl_isUSBInit = True;
			}
		} else {
			m_bl_isUSBInit = False;
		}
	}

	// 全画面警告発生(メッセージ警告ではない)の時.
	// メッセージ警告をPLayerControlのsetupGamenで検出し、ポップアップしているので、ここで抑制する.
	Bool _bl_isMessageWarning = False;
//	TYPE_KeyMode e_KeyMode = TYPE_KeyModeNone;
//	TYPE_WarningName _ec_WarningName = TYPE_WarningName_Non;

////	// Job list button should not be displayed in Vendor2 mode.
////	if ( APIC_KeyDevice::getKeyMode() == TYPE_KeyModeVendorKM ) {
////		Key_HomeTopMenuJobList.showEnable( False );
////	}
//	// Job list button should not be displayed in Vendor2 mode.
//	if ( (TYPE_KeyModeVendorKM == APIC_KeyDevice::getKeyMode())
//	if ( True == PnlComIsDisableJobOpeWithPublic() ) {		//「DispSW71(bit7)の値がON」且つ「パブリックユーザーでログイン」の場合.
//		Key_HomeTopMenuJobList.showEnable( False );	//「ジョブ表示」キー非表示.
//	} else {
//		Key_HomeTopMenuJobList.showEnable( True );	//「ジョブ表示」キー表示.
//	}


	for( ushort j = 1; j < ( ushort )( us_MessageWarningTableSize/sizeof( SP_MessageWarningTable ) ); j++ ) {
		TYPE_WarningName _e_WarningName = sp_MessageWarningTable[j].ec_WarningName;
		if( True == onlyPanel.isWarningOn( comPanelJob, _e_WarningName ) ) {
			_bl_isMessageWarning = True;
			switch( _e_WarningName ) {
				case PNL_ERROR_LowPowerModeError:		// パワーセーブモード移行不可（ローパワー）.
				case PNL_ERROR_SleepModeError:			// パワーセーブモード移行不可（スリープ）.
// comment 2019/7/25 Sky雜賀[EagleH][OP_BTS-10587].
				case PNL_ERROR_IWSForceTerminationAlert:	// 	IWSアプリケーション強制終了警告.
					_bl_isMessageWarning = False;
					break;
				default:	break;
			}
			break;
		}
	}
	if( False == _bl_isMessageWarning ) {
		// 親クラスのsetupGamen()をコールする.
		PPopupAnimationCom::setupGamen();
	} else {
		short partsNumber			= getPartsNumber();
		PDisplayParts** partsTable	= getPartsTable();
		for( short k = 0; k < partsNumber; k++ ) {
			PComponent* pc_Component = dynamic_cast<PComponent*>( partsTable[k] );
			if( NULL != pc_Component ) {
				pc_Component->setupComponent( option );
			}
		}
		PMessageArea::setupGamen( PCGD_CHANGE_TEXT_NOT_ATTENTION );	// アテンション警告を検出しない。
	}
	printLEDControl( False );				// スタートキーＬＥＤは赤.
	setupBackParts();					// 背景表示処理.
	setupFiery();						// Fiery表示処理.
	setupLogInInfo();					// ログイン情報表示.
	setupProjectCode();					// 日立ICC プロジェクトコード表示.

// comment 2018/04/13 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
	if( PNLE_UIModeLightUI != PNLC_WebUIControl::getInstance()->getUIMode()
// comment 2021/06/11 DatVQ1 [OP_BTS_33720] ジョブ表示ボタンが緑色を表示される問題対策
	||( PnlComGetUserAuthentication()->getUserAuthentication(PNLC_NetworkLan::getInstance()->getLine() != TYPE_UserAuthenticationOff)
         && PnlComGetUser() == NULL )){
			changePartsForTheme();				// テーマによるパーツ変更処理.
	} 
	Bool b_ret = True;		//【362153】 【FUM4.0 共通CQ:360403】【207_キーカウンター／ベンダーモード時のOpenAPI機能制限解除】【HCLT-Dharani】2011/10/20 Add.

// comment 2016/09/16 MSE岡田 [IT5_3.3][1005694] Language、機能検索の不要描画を行わないようにするためフラグを追加。省電力復帰のHDDチェックスキップ時の各キー非表示/グレイアウト対応。不要な処理の削除.
//	//「Language」キーの表示/非表示.
//	Bool bl_LanguageEnable = PnlComGetOperationSetting()->getLanguageChoiceSetting();
//	// PKI認証時.
//	if( True == PLBC_FuncPKICheck::isPKI() ) {
//		Key_HomeTopMenuLanguage.showEnable( False );
//		bl_LanguageEnable = False;
//	}
//	// 「機能検索」キーの表示/非表示.
//	Bool bl_FunctionSrarchEnable = False;
//	// 検索機能が1つでも有効な場合.
//	if( ( TYPE_SearchApp     == PnlComGetOperationSetting()->getSearchSettingApp() )
//	    ||( TYPE_SearchCopy    == PnlComGetOperationSetting()->getSearchSettingCopy() )
//	    ||( TYPE_SearchFaxScan == PnlComGetOperationSetting()->getSearchSettingFaxScan() ) ) {
//		bl_FunctionSrarchEnable = True;
//	}
//
//	if( ( True == bl_FunctionSrarchEnable ) && ( True == bl_LanguageEnable ) ) {
//		// 「Language」「機能検索」共に有効な場合.
//		Key_HomeTopMenuLanguage.showEnable( False );				// 「Language」(大)キー非表示.
//		Key_HomeTopMenuFunctionSearch.showEnable( False );			// 「検索機能」(大)キー非表示.
//		Key_HomeTopMenuLanguageSmall.showEnable( True );			// 「Language」(小)キー表示.
//		Key_HomeTopMenuFunctionSearchSmall.showEnable( True );		// 「検索機能」(小)キー表示.
//	} else if( ( False == bl_FunctionSrarchEnable ) && ( True == bl_LanguageEnable ) ) {
//		// 「Language」のみ有効な場合.
//		Key_HomeTopMenuFunctionSearch.showEnable( False );			// 「検索機能」(大)キー非表示.
//		Key_HomeTopMenuLanguageSmall.showEnable( False );			// 「Language」(小)キー非表示.
//		Key_HomeTopMenuFunctionSearchSmall.showEnable( False );		// 「検索機能」(小)キー非表示.
//		Key_HomeTopMenuLanguage.showEnable( True );					// 「Language」(大)キー表示.
//	} else if( ( True == bl_FunctionSrarchEnable ) && ( False == bl_LanguageEnable ) ) {
//		// 機能検索キーのみ有効な場合.
//		Key_HomeTopMenuLanguage.showEnable( False );				// 「Language」(大)キー非表示.
//		Key_HomeTopMenuLanguageSmall.showEnable( False );			// 「Language」(小)キー非表示.
//		Key_HomeTopMenuFunctionSearchSmall.showEnable( False );		// 「検索機能」(小)キー非表示.
//		Key_HomeTopMenuFunctionSearch.showEnable( True );			// 「検索機能」(大)キー非表示.
//	} else {
//		// 共に無効な場合.
//		Key_HomeTopMenuLanguage.showEnable( False );				// 「Language」(大)キー非表示.
//		Key_HomeTopMenuFunctionSearch.showEnable( False );			// 「検索機能」(大)キー非表示.
//		Key_HomeTopMenuLanguageSmall.showEnable( False );			// 「Language」(小)キー非表示.
//		Key_HomeTopMenuFunctionSearchSmall.showEnable( False );		// 「検索機能」(小)キー非表示.
//
//	}
//
//	//「Language」キーのグレイアウト.
//	Key_HomeTopMenuLanguage.activate( checkEnableUtilityMode() );
//
//	// 警告状態によるアプリケーション選択チェックをsetupGamenからprocTimerWakeUpに変更する.
//	if( 1 == uc_maxPage ) {
//	} else {
//	}
	Bool bl_LanguageShowEnable = False;					// 「Language」(大)キー表示/非表示フラグ.
	Bool bl_FunctionSearchShowEnable = False;			// 「検索機能」(大)キー表示/非表示フラグ.
	Bool bl_LanguageSmallShowEnable = False;			// 「Language」(小)キー表示/非表示フラグ.
	Bool bl_FunctionSearchSmallShowEnable = False;		// 「検索機能」(小)キー表示/非表示フラグ.
	if( True == PNLC_SnapshotControlLib::newInstance()->getSkipCheckHDD() ) {
		bl_LanguageShowEnable = False;					// 「Language」(大)キー非表示.
		bl_FunctionSearchShowEnable = False;			// 「検索機能」(大)キー非表示.
		bl_LanguageSmallShowEnable = False;				// 「Language」(小)キー非表示.
		bl_FunctionSearchSmallShowEnable = False;		// 「検索機能」(小)キー非表示.

		Key_HomeTopMenuUniversalSetting.activate( False );		// ユニバーサル設定.
		Cmp_HomeTopMenuHeaderAreaJobList.activate( False );		// ジョブ表示.
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//// comment 2018/04/18 SCD2 Kazuo.Takei [Eagle/Sparrow][制御構成変更_No.111 P07_01_共通仕様(第6世代UI画面_左エリア)].
//		Cmp_LeftAreaComponent.showEnableLanguage( False );
	} else {
		//「Language」キーの表示/非表示.
		Bool bl_LanguageEnable = PnlComGetOperationSetting()->getLanguageChoiceSetting();
		// PKI認証時.
		if( True == PLBC_FuncPKICheck::isPKI() ) {
			Key_HomeTopMenuLanguage.showEnable( False );
			bl_LanguageEnable = False;
		}
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//// comment 2018/04/18 SCD2 Kazuo.Takei [Eagle/Sparrow][制御構成変更_No.111 P07_01_共通仕様(第6世代UI画面_左エリア)].
//		Cmp_LeftAreaComponent.showEnableLanguage( bl_LanguageEnable );
//		Cmp_LeftAreaComponent.activateLanguage( checkEnableUtilityMode() );
		// 「機能検索」キーの表示/非表示.
		Bool bl_FunctionSrarchEnable = False;
		// 検索機能が1つでも有効な場合.
		if( ( TYPE_SearchApp     == PnlComGetOperationSetting()->getSearchSettingApp() )
		    ||( TYPE_SearchCopy    == PnlComGetOperationSetting()->getSearchSettingCopy() )
		    ||( TYPE_SearchFaxScan == PnlComGetOperationSetting()->getSearchSettingFaxScan() ) ) {
			bl_FunctionSrarchEnable = True;
		}

		if( ( True == bl_FunctionSrarchEnable ) && ( True == bl_LanguageEnable ) ) {
			// 「Language」「機能検索」共に有効な場合.
			bl_LanguageShowEnable = False;					// 「Language」(大)キー非表示.
			bl_FunctionSearchShowEnable = False;			// 「検索機能」(大)キー非表示.
			bl_LanguageSmallShowEnable = True;				// 「Language」(小)キー表示.
			bl_FunctionSearchSmallShowEnable = True;		// 「検索機能」(小)キー表示.
		} else if( ( False == bl_FunctionSrarchEnable ) && ( True == bl_LanguageEnable ) ) {
			// 「Language」のみ有効な場合.
			bl_FunctionSearchShowEnable = False;			// 「検索機能」(大)キー非表示.
			bl_LanguageSmallShowEnable = False;				// 「Language」(小)キー非表示.
			bl_FunctionSearchSmallShowEnable = False;		// 「検索機能」(小)キー非表示.
			bl_LanguageShowEnable = True;					// 「Language」(大)キー表示.
		} else if( ( True == bl_FunctionSrarchEnable ) && ( False == bl_LanguageEnable ) ) {
			// 機能検索キーのみ有効な場合.
			bl_LanguageShowEnable = False;					// 「Language」(大)キー非表示.
			bl_LanguageSmallShowEnable = False;				// 「Language」(小)キー非表示.
			bl_FunctionSearchSmallShowEnable = False;		// 「検索機能」(小)キー非表示.
			bl_FunctionSearchShowEnable = True;				// 「検索機能」(大)キー非表示.
		} else {
			// 共に無効な場合.
			bl_LanguageShowEnable = False;					// 「Language」(大)キー非表示.
			bl_FunctionSearchShowEnable = False;			// 「検索機能」(大)キー非表示.
			bl_LanguageSmallShowEnable = False;				// 「Language」(小)キー非表示.
			bl_FunctionSearchSmallShowEnable = False;		// 「検索機能」(小)キー非表示.
		}

		Key_HomeTopMenuUniversalSetting.activate( True );					// ユニバーサル設定.
		Key_HomeTopMenuLanguage.activate( checkEnableUtilityMode() );		// Language(大).
		Key_HomeTopMenuLanguageSmall.activate( checkEnableUtilityMode() );	// Language(小).
		Cmp_HomeTopMenuHeaderAreaJobList.activate( True );					// ジョブ表示.
	}

	// comment 2018/08/10 SCD2 T.Nishiba [Eagle][RQ-698-41]新設定メニューのメニュー項目対応.
	// 認証が有効で未ログイン時（Publicログインも含む）には、ユニバーサル設定は設定不可（グレイアウト）にする.
	if( (TYPE_UserAuthenticationOff != PnlComGetUserAuthentication()->getUserAuthentication(PNLC_NetworkLan::getInstance()->getLine())) ) {
		// ユーザー認証有効.
		APIC_User* pc_User = PnlComGetUser();
// comment 2018/10/17 SCD2 K.Takei 不具合No.1063034 本体認証の場合、パブリックユーザーでログインし、WebUI画面にユニバーサルとユーティリティボタンがグレーアウトになってしまう.
		if( (NULL == pc_User) ) {									// 未ログイン.
//		    (pc_User->getUserType() == TYPE_UserTypePublicUser) ) {	// Publicログイン.
			Key_HomeTopMenuUniversalSetting.activate( False );	// ユニバーサル設定.
		}
	}
// comment 2021/04/30 Sky東島 [IT6_2.2][OP_BTS-32561].
//	if( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance()->getLine()) ) {
//		// 部門管理有効.
//		if( NULL == PnlComGetTrack() ) {	// 未ログイン.
	if( True == PnlComGetNeedToTrackLogin() ) {	// 未ログイン.
			Key_HomeTopMenuUniversalSetting.activate( False );	// ユニバーサル設定.
//		}
	}

	// MiZe4th以降、リモートパネル接続中でもOAPを起動可能とする.
	Bool bl_OapEnabledByRmp = True;
	if( False == PLBC_FuncVersionCheck::isOK_MiZe_Type5() ) {	// MiZe4thより前.
		APIC_RemotePanelSetting* pc_RMPSetting = PnlComGetRemotePanelSetting();
		if( ( NULL != pc_RMPSetting ) && ( TYPE_RemoteStatusNone != pc_RMPSetting->getRemoteStatus() ) ) {
			bl_OapEnabledByRmp = False;
		}
	}
	// comment 2015/05/01 [FQ-ISDCS-1400532-OFP180]パフォーマンス改善.
	if (PNLE_MTUIMODE_V1 > PLBC_MultiTouchFunction::getMultiTouchMode()) {

// comment 2018/04/04 SCD2 K.Nishiki [Eagle][制御構成変更No.111]ホーム画面認証画面の新UI化対応.
	for( uchar i = 0; i < m_uc_TopMenuListKeyDisp; i++ ) {
		uchar _uc_index = static_cast<uchar>( ( uc_nowPage - 1 ) * m_uc_TopMenuListKeyDisp + i );
		if( PNLD_TOPMENU_SHORTCUTICON_MAX <= _uc_index ) {
			Key_HomeTopMenuBadge[i].showEnable( False );
			continue;										// 3画面目の時に、配列へのアクセスを防ぐ
		}
		// 処理対象のキーがシステムボックス機能の場合、表示判定の結果を取得する.
		if( ( 2 == option ) || ( 3 == option ) ) {  // 「＜」「＞」キー押下時.
			Key_HomeTopMenuBadge[i].showEnable( False );
		}
		if( APIC_KeyDevice::getKeyMode() == TYPE_KeyModeVendorKM ) {
			b_ret = checkShortCutDisableCoinExist( st_BadgeKeyInfo[_uc_index].e_ShortCutKind );
			if( False == b_ret ) {
				Key_HomeTopMenuBadge[i].showEnable( b_ret );   // Key
				Gra_HomeTopMenuBadgeIcon[i].showEnable( b_ret );  // Graphics
				continue;
			}
			else {
				Key_HomeTopMenuBadge[i].showEnable( st_BadgeKeyInfo[_uc_index].bl_isShow );
			}
		}
		if( True == st_BadgeKeyInfo[_uc_index].bl_isShow ) {
			if( 0 != st_BadgeKeyInfo[_uc_index].tid_FuncName ) {
				Key_HomeTopMenuBadge[i].changeText( &Ktx_HomeTopMenuBadge[i], st_BadgeKeyInfo[_uc_index].tid_FuncName );
			} else {
				Key_HomeTopMenuBadge[i].changeExternalText( &Ktx_HomeTopMenuBadge[i], ( uchar* )st_BadgeKeyInfo[_uc_index].sc_FuncName, sizeof( st_BadgeKeyInfo[_uc_index].sc_FuncName ) );
			}

			if( False == st_BadgeKeyInfo[_uc_index].bl_isShowIcon )
			{
				//[アイコンなし]の場合は、アイコンを表示しない
				Gra_HomeTopMenuBadgeIcon[i].showEnable( False );
			} else
			{
				if( ( PNLE_TopMenuShortCut_AppShortCutOap != st_BadgeKeyInfo[_uc_index].e_ShortCutKind )			// OAPアプリ以外.
				    && ( PNLE_TopMenuShortCut_AppShortCutHanyo != st_BadgeKeyInfo[_uc_index].e_ShortCutKind ) ) {		// かつ汎用アプリ以外.
					// ショートカット種別がOAPアプリでない場合
					// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] 登録アプリケーショングループの場合を含む未使用パーツの画像変更処理を削除.
					//	if( True == st_BadgeKeyInfo[_uc_index].bl_BMP ) {
					//		Gra_HomeTopMenuBadgeIcon[i].changeRawGraphic( ( const uchar* )st_BadgeKeyInfo[_uc_index].puc_FuncIcon );
					//	} else {
					//		Gra_HomeTopMenuBadgeIcon[i].changeGraphic( i70_025_000_n_X64_Y80 );
					//		// デフォルトのパーツにいったん変更する.
					//		PSinglePartsGraphic* p_parts = &Gra_HomeTopMenuBadgeIcon[i];
					//		p_parts->changeGraphic( ( const uchar* )st_BadgeKeyInfo[_uc_index].puc_FuncIcon,( uchar )st_BadgeKeyInfo[_uc_index].uc_IconPaletteType,True );
					//	}
					Gra_HomeTopMenuBadgeIcon[i].showEnable( True );
				} else {
					// ショートカット種別がOAPアプリの場合
					if( 0xFFFF != us_OAPAppNo[_uc_index] ) {
					// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] アプリケーション毎アイコンの場合を含む未使用パーツの画像変更処理を削除.
					//		Bool b_updateIcon = False;
					//		// アイコンの登録Noを検索.
					//		for( uchar uc_IconListNo = 0; i < TYPD_ExtMaxApplication; uc_IconListNo++ ) {
					//			// アプリケーションNoが一致.
					//			if( s_registerApp[uc_IconListNo] == ( short )us_OAPAppNo[_uc_index] ) {
					//				// アイコン設定.
					//				Gra_HomeTopMenuBadgeIcon[i].changeGraphic( i70_025_000_n_X64_Y80 );
					//				// デフォルトのパーツにいったん変更する.
					//				b_updateIcon = ScanOapExtSet.updateIcon( uc_IconListNo, &Gra_HomeTopMenuBadgeIcon[i] );
					//				break;
					//			}
					//		}
					//	Gra_HomeTopMenuBadgeIcon[i].showEnable( b_updateIcon );
					} else {
						Gra_HomeTopMenuBadgeIcon[i].showEnable( False );
					}
				}
			}
			setBadgeKeyActivate( _uc_index );
			if( _uc_index <= TYPD_ExtShortcutMaxTopMenu )
			{
				Key_HomeTopMenuBadge[i].activate( st_BadgeKeyInfo[_uc_index].bl_isActivate );	// [CQ.340570].
			}
		}
		Key_HomeTopMenuBadge[i].showEnable( st_BadgeKeyInfo[_uc_index].bl_isShow );

		if( ( ( False == bl_OapEnabledByRmp ) )
		    && ( True == isOapMenuItem( st_BadgeKeyInfo[_uc_index].e_ShortCutKind ) ) ) {
			// リモートパネル動作中は非表示.
			Key_HomeTopMenuBadge[i].showEnable( False );
		}
//		Key_HomeTopMenuBadge[i].showEnable( False );
	}
// comment 2015/05/1 [FQ-ISDCS-1400532-OFP180] パフォーマンス改善.
	} else {
// comment 2018/04/04 SCD2 K.Nishiki [Eagle][制御構成変更No.111]ホーム画面認証画面の新UI化対応.
		for ( uchar i = 0; i < m_uc_TopMenuListKeyDisp; i++ ) {
			// タッチパネルジェスチャー有効時、Qtでパーツ表示を行うため非表示.
			Key_HomeTopMenuBadge[i].showEnable(False);
		}
	}
	// comment 2015/10/20 OPS1-川手 [特ROM対応][FQ-OPSD-1500259-OFP185]Fax受信時に受信エラー(R27)が頻繁に発生する.
	startCacheFuncEnable();					// キャッシュ処理開始.
	for( int idx = 0; idx < PNLD_TOPMENU_SHORTCUTICON_MAX; idx++ ) {
		setBadgeKeyActivate( static_cast<uchar>( idx ) );
		Bool bl_IsShow = st_BadgeKeyInfo[idx].bl_isShow;
		if( PNLE_TopMenuShortCut_SimpleSecuritySetup == st_BadgeKeyInfo[idx].e_ShortCutKind ) {
			PNLC_KeyConditionInterface* pc_KeyConditionCheck = PNLC_KeyConditionFactory::newInstance()->create( PNLC_KeyConditionFactory::PNLD_KeyType_SimpleSecurity );
			if( NULL != pc_KeyConditionCheck ) {
// comment 2016/09/16 MSE岡田 [IT5_3.3][1005694] 省電力復帰のHDDチェックスキップ時は簡単セキュリティー設定をグレイアウト表示とするためキーを表示する.
//				bl_IsShow = pc_KeyConditionCheck->isKeyEnable();
				if( True == PNLC_SnapshotControlLib::newInstance()->getSkipCheckHDD() ) {
					bl_IsShow = True;
				} else {
					bl_IsShow = pc_KeyConditionCheck->isKeyEnable();
				}
			}
			if( False == PLBC_FuncVersionCheck::isOK_MiZe_Type3() ) {
				bl_IsShow = False;
			}
			if( True == onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCallFirmwareAbnormal ) ) {
				bl_IsShow = False;
			}
		}
		
// comment 2016/10/07 SCD2-Sky佐藤 [IT5_3.3][1005928] ジョブ飛び越し対応.
		// 割り込みモード中の場合、bizhub MarketPlaceを非表示にする.
		if( PNLE_TopMenuShortCut_MarketPlace == st_BadgeKeyInfo[idx].e_ShortCutKind ) {
			if( EC_IntMode_Off != onlyPanel.ec_IntMode ) {
				bl_IsShow = False;
			}
		}
// comment 2015/10/05 OPS1-Sky的場 [特ROM][IT5-12]トップメニュー画面ダウンロードアイコン非表示対応.
//            PnlComMtuiIf_TopMenuModel::instance()->addUpdateIconToSubmit(
//                idx
//                , ( True == st_BadgeKeyInfo[idx].bl_isActivate ) ? st_BadgeKeyInfo[idx].puc_FuncIcon : st_BadgeKeyInfo[idx].puc_IconNotActivate
//                , ( True == st_BadgeKeyInfo[idx].bl_isActivate ) ? st_BadgeKeyInfo[idx].uc_IconPaletteType : st_BadgeKeyInfo[idx].uc_IconPaletteNotActivate
////              , st_BadgeKeyInfo[idx].bl_isShow
//				, bl_IsShow
//                , st_BadgeKeyInfo[idx].bl_isActivate
//            );
			Bool bl_ShowIcon = st_BadgeKeyInfo[idx].bl_isShowIcon;
			if( False == checkShortCutIconDisplay( static_cast<uchar>( idx ), st_BadgeKeyInfo[idx].bl_isActivate) ){
				bl_ShowIcon = False;
			}
		// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] データサイズの設定を追加.
		PnlComMtuiIf_TopMenuModel::instance()->addUpdateIconToSubmit(
		    (int)idx
		    , ( True == st_BadgeKeyInfo[idx].bl_isActivate ) ? st_BadgeKeyInfo[idx].puc_FuncIcon : st_BadgeKeyInfo[idx].puc_IconNotActivate
		    , ( True == st_BadgeKeyInfo[idx].bl_isActivate ) ? st_BadgeKeyInfo[idx].uc_IconPaletteType : st_BadgeKeyInfo[idx].uc_IconPaletteNotActivate
		    , bl_IsShow
		    , st_BadgeKeyInfo[idx].bl_isActivate
// comment 2020/07/17 Sky東島 [IT6_2.0MR3][OP_BTS-25505].
//		    , ( True == st_BadgeKeyInfo[idx].bl_isActivate ) ? st_BadgeKeyInfo[idx].ss_FuncIconDataSize : st_BadgeKeyInfo[idx].ss_IconNotActivateDataSize
		    , ( True == st_BadgeKeyInfo[idx].bl_isActivate ) ? st_BadgeKeyInfo[idx].us_FuncIconDataSize : st_BadgeKeyInfo[idx].us_IconNotActivateDataSize
           	, bl_ShowIcon
			, st_BadgeKeyInfo[idx].bl_SvgData		// comment 2021/02/08 Sky中野 [IT6_2.2][OP_BTS-30807].
		);
	}
	PnlComMtuiIf_TopMenuModel::instance()->submitDatas();
	// comment 2015/10/20 OPS1-川手 [特ROM対応][FQ-OPSD-1500259-OFP185]Fax受信時に受信エラー(R27)が頻繁に発生する.
	endCacheFuncEnable();							// キャッシュ処理終了.
// comment 2016/09/16 MSE岡田 [IT5_3.3][1005694] プログラムキーの更新を抑制するためのフラグを落とすタイミングを修正したため削除する.
//	m_bl_UpdateProgramKeyStatus = False;			// プログラムキーの更新を抑制するため、更新処理用のフラグを落とす.
// comment 2019/02/05(火) SCD2-Sky長谷川(伸) [FQ-OPSD-1900013-OFP008].
	if ( True == PNLC_SnapshotControlLib::newInstance()->getSkipCheckHDD() ) {
		m_bl_UpdateProgramKeyStatus = True;
	} else {
		m_bl_UpdateProgramKeyStatus = False;
	}
	PNLC_SnapshotControlLib* pc_SnapshotControlLib = PNLC_SnapshotControlLib::newInstance();
	if( NULL == pc_SnapshotControlLib ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	// スナップショット作成時はLanguage、検索機能を非表示.
// comment 2016/09/16 MSE岡田 [IT5_3.3][1005694] Language、機能検索の不要描画を行わないようにするためフラグを利用し、最後に表示/非表示を行うように修正.
//	if( True == pc_SnapshotControlLib->getStatusSnapshotCreating() ) {
//		Key_HomeTopMenuLanguage.showEnable( False );				// 「Language」(大)キー非表示.
//		Key_HomeTopMenuFunctionSearch.showEnable( False );			// 「検索機能」(大)キー非表示.
//		Key_HomeTopMenuLanguageSmall.showEnable( False );			// 「Language」(小)キー非表示.
//		Key_HomeTopMenuFunctionSearchSmall.showEnable( False );		// 「検索機能」(小)キー非表示.
//	}
	if( True == pc_SnapshotControlLib->getStatusSnapshotCreating() ) {
		bl_LanguageShowEnable = False;				// 「Language」(大)キー非表示.
		bl_FunctionSearchShowEnable = False;		// 「検索機能」(大)キー非表示.
		bl_LanguageSmallShowEnable = False;			// 「Language」(小)キー非表示.
		bl_FunctionSearchSmallShowEnable = False;	// 「検索機能」(小)キー非表示.
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//// comment 2018/04/18 SCD2 Kazuo.Takei [Eagle/Sparrow][制御構成変更_No.111 P07_01_共通仕様(第6世代UI画面_左エリア)].
//		Cmp_LeftAreaComponent.showEnableLanguage( False );
	}
	Key_HomeTopMenuLanguage.showEnable( bl_LanguageShowEnable );						// Language(大).
	Key_HomeTopMenuFunctionSearch.showEnable( bl_FunctionSearchShowEnable );			// 検索機能(大).
	Key_HomeTopMenuLanguageSmall.showEnable( bl_LanguageSmallShowEnable );				// Language(小).
	Key_HomeTopMenuFunctionSearchSmall.showEnable( bl_FunctionSearchSmallShowEnable );	// 検索機能(小).
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//// comment 2018/04/25 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
//	if( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() ){
//		Key_HomeTopMenuUniversalSetting.showEnable( False );		// 「ﾕﾆﾊﾞｰｻﾙ設定」キー非表示.
//		Cmp_HomeTopMenuHeaderAreaJobList.showEnable( False );		// 「ジョブリスト」キー非表示.
//		Key_HomeTopMenuCounter.showEnable( False );					// 「カウンター」キー非表示.
//		Key_HomeTopMenuFunctionSearch.showEnable( False );			// 「機能検索(大)」キー非表示.
//		Key_HomeTopMenuFunctionSearchSmall.showEnable( False );		// 「機能検索(小)」キー非表示.
//		Key_HomeTopMenuLanguage.showEnable( False );				// 「Language」キー.
//		Key_HomeTopMenuLanguageSmall.showEnable( False );			// 「Language(小)」キー非表示.
//		Cmp_HomeTopMenuMessageArea.showEnable( False );				// メッセージエリア非表示.
//		Key_HomeTopMenuLogInInfo.showEnable( False );				// 認証情報表示エリア非表示.
//// comment 2018/05/08 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
//		Cmp_HomeTopMenuHeaderAreaJobListHomeMenu.showEnable( True );	// 新UI「ジョブリスト」キー.
//		Cmp_LeftAreaComponent.showEnable( True );						// 新UI左エリアコンポーネント.
//		Cmp_HomeTopMenuMessageAreaHomeMenu.showEnable( True );			// 新UIメッセージエリア.
//	} else {
//// comment 2018/05/08 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
//	Cmp_HomeTopMenuHeaderAreaJobListHomeMenu.showEnable( False );	// 新UI「ジョブリスト」キー.
//	Cmp_LeftAreaComponent.showEnable( False );						// 新UI左エリアコンポーネント.
//	Cmp_HomeTopMenuMessageAreaHomeMenu.showEnable( False );			// 新UIメッセージエリア.
//	}
//// comment 2018/10/11 SCD2-Sky中野 [Sparrow][1062449].
//	if( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() ) {
//// comment 2018/05/08 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
//		// コンポーネント表示タイプ.
//		// 常にテキストON表意.
//		Cmp_LeftAreaComponent.setTypeDisplay( PNLE_LeftAreaDisplayType_TextOn );		// コンポーネント表示タイプ.
//
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
//		// IDキー.
//		Bool bl_DispID = isDisplayIDKeyByAuthenticationStatus();					//IDキー表示：True ／ IDキー非表示：False
//		Cmp_LeftAreaComponent.showEnableID( bl_DispID );							// IDキー表示フラグ.
//#endif	// #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	.
//
//		//ログイン状態変更チェック.
//		Bool bl_LoginCheck = False;
//		APIC_User* pc_UserCheck = PnlComGetUser();
//		if( NULL != pc_UserCheck ){
//			//ログイン状態.
//			bl_LoginCheck = True;
//		}
//		// IDキー表示タイプ.
//		Cmp_LeftAreaComponent.setTypeID( ( False == bl_LoginCheck ? PNLE_LeftAreaIDType_NotLogin : PNLE_LeftAreaIDType_LoginIn ) );
//
//		// 認証情報キー／ネットワーク選択キー表示タイプ設定.
//		Cmp_LeftAreaComponent.setTypeLoginInfo( ( True == m_bl_VlanNWShow ? PNLE_LeftAreaLoginInfoType_lanNW : PNLE_LeftAreaLoginInfoType_User ) );
//
//		// 設定メニューキー表示設定 常に表示.
//		Cmp_LeftAreaComponent.showEnableUtility( True );
//		// 設定メニューキー活性フラグ(activateUtility)設定ははsetupGamen()で設定.
//
//// comment : WebUI2 画面切替え制御 -> 20180302 Chikuan.
////	Bool b_EnableChangeDisplay = False;
////	if( True == PNLC_WebUIDebug::getInstance()->getDebugMode( PNL_WebUIDebugModeDisplayChangeModeKey ) ){
////		b_EnableChangeDisplay = True;
////	}
////	Key_HomeTopMenuToLightUIMenu.showEnable( b_EnableChangeDisplay );
//// comment : WebUI2 画面切替え制御 <- 20180302 Chikuan.
//// comment 2018/04/18 SCD2 Kazuo.Takei [Eagle/Sparrow][制御構成変更_No.111 P07_01_共通仕様(第6世代UI画面_左エリア)].
//		// 左エリアコンポーネント：設定メニュー活性/非活性設定.
//		// comment 2018/08/10 SCD2 T.Nishiba [Eagle][RQ-698-41]新設定メニューのメニュー項目対応.
//		// 設定メニューキーは常に有効にする.
//		Cmp_LeftAreaComponent.activateUtility( True );		// 設定メニューキー活性.
//// comment 2018/10/11 SCD2-Sky中野 [Sparrow][1062449].
//	}
// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )
	PnlComFooterPartsBtnActiveChange(homeKeyActiveCheck(), idKeyActiveCheck(), resetKeyActiveCheck());
#endif		//  ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )	.
}

/*================================================================================================
［機能］	OAP関連のショートカット種別であることを確認する.
--------------------------------------------------------------------------------------------------
［引数］	ショートカット種別をあらわす.
［戻値］	引数で渡されたショートカット種別がOAP関連の場合、Trueを返す.
［履歴］	2013/07/18	Sky酒井(直)		bizhubRAセキュリティ対応(Bug#9, #10).
================================================================================================*/
Bool PCGC_HomeTopMenu::isOapMenuItem( const PNLE_TopMenuShortCut e_Item ) const
{
	Bool bl_OapItem = False;
	if( PNLE_TopMenuShortCut_FuncOAPApp         == e_Item
	    || PNLE_TopMenuShortCut_AppShortCutOap     == e_Item
	    || PNLE_TopMenuShortCut_AppShortCutHanyo   == e_Item
	    || PNLE_TopMenuShortCut_GrpAppShortCutGrp1 == e_Item
	    || PNLE_TopMenuShortCut_GrpAppShortCutGrp2 == e_Item
	    || PNLE_TopMenuShortCut_GrpAppShortCutGrp3 == e_Item
	    || PNLE_TopMenuShortCut_GrpAppShortCutGrp4 == e_Item
	    || PNLE_TopMenuShortCut_GrpAppShortCutGrp5 == e_Item
	    || PNLE_TopMenuShortCut_GrpAppShortCutGrp6 == e_Item ) {
		bl_OapItem = True;
	}
	return ( bl_OapItem );
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： 背景表示処理.
//================================================================================================
void PCGC_HomeTopMenu::setupBackParts()
{
	// テーマによる背景変更.
	uchar _uc_index = 0;
// comment 2018/04/04 SCD2 K.Nishiki [Eagle][制御構成変更No.111]ホーム画面認証画面の新UI化対応.
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//	PNLE_UIMode e_NowUIMode = PNLC_WebUIControl::getInstance()->getUIMode();
//	if( PNLE_UIModeLightUI == e_NowUIMode ){
//		_uc_index = 5;
//	}else{	
	switch( PnlComGetSystemSetting()->getTopMenuCustomPattern() ) {
		case TYPE_TopMenuCustomPattern1:
			if( True == m_bl_IsExistEcoFuncShortcut ) {
				_uc_index = 3;
			} else {
				_uc_index = 0;
			}
			break;
		case TYPE_TopMenuCustomPattern2:	_uc_index = 1;	break;
		case TYPE_TopMenuCustomPattern3:	_uc_index = 2;	break;
		case TYPE_TopMenuUserCustomPattern1:	// ユーザーテーマ選択時.
		case TYPE_TopMenuUserCustomPattern2:
		case TYPE_TopMenuUserCustomPattern3:
			_uc_index = 4;
			break;
		default:	break;
	}
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//}
	for( uchar i = 0; i < ss_HomeTopMenuBackNumber; i++ ) {
		if( _uc_index != i ) {
			Grp_HomeTopMenuBackground[i].showEnable( False );
		}
	}
	Grp_HomeTopMenuBackground[_uc_index].showEnable( True );
	// comment 2014/09/26 OPS1 森山 [Zeus5BK][CQ500594].
	// comment 2015/8/31 OPS1-Sky工藤 [MinervaSBK][CQ517742].
	Cmp_HomeTopMenuMessageArea.changeBackgroundGraphic( pc_MessageAreaParts[_uc_index] );
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： Fiery表示処理.
//================================================================================================
void PCGC_HomeTopMenu::setupFiery()
{
//				IT5用の実装に置き換え(旧コードは有効コード下部に移動).
//				FieryはIT5で表示されることはない為常に非表示とする.
	Grp_HomeTopMenuFiery.showEnable( False );
	return;

}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： テーマによるパーツ変更処理.
//================================================================================================
#ifdef DEF_LCD_7INCH
static PNLS_Parts* pc_HeaderParts[] = { tbl_c70_039_000_t1_X108_Y32, tbl_c70_039_000_t2_X108_Y32, tbl_c70_039_000_t3_X108_Y32, prt_o20_004_000_X108_Y28 };
//static PNLS_Parts *pc_PagePrev[] = { tbl_c70_041_000_X32_Y32, tbl_c70_041_t2_000_X32_Y32, tbl_c70_041_t3_000_X32_Y32, prt_c70_016_000_t1_X24_Y24 };
//static PNLS_Parts *pc_PageNext[] = { tbl_c70_040_000_X32_Y32, tbl_c70_040_t2_000_X32_Y32, tbl_c70_040_t3_000_X32_Y32, prt_c70_016_000_t1_X24_Y24 };
//static PNLS_Parts *pc_SearchKeyParts[] = { tbl_c70_045_000_n_X108_Y28, tbl_c70_045_000_n_X108_Y28, tbl_c70_045_000_n_X108_Y28, tbl_o20_005_000_n_X108_Y28 };
//static PNLS_Parts *pc_SearchSmallKeyParts[] = { tbl_c70_043_000_n_X48_Y28, tbl_c70_043_000_n_X48_Y28, tbl_c70_043_000_n_X48_Y28, tbl_o10_009_000_n_X32_Y28 };
//static PNLS_Parts *pc_LanguageSmallKeyParts[] = { tbl_c70_044_000_n_X48_Y28, tbl_c70_044_000_n_X48_Y28, tbl_c70_044_000_n_X48_Y28, tbl_o10_010_000_n_X32_Y28 };
static PNLS_Parts* pc_SearchKeyParts[] = { tbl_c70_052_001_n_X108_Y32, tbl_c70_052_001_n_X108_Y32, tbl_c70_052_001_n_X108_Y32, tbl_o20_005_001_n_X108_Y32 };
static PNLS_Parts* pc_SearchSmallKeyParts[] = { tbl_c70_043_001_n_X32_Y32, tbl_c70_043_001_n_X32_Y32, tbl_c70_043_001_n_X32_Y32, tbl_o10_009_001_n_X32_Y32 };
static PNLS_Parts* pc_LanguageSmallKeyParts[] = { tbl_c70_044_001_n_X32_Y32, tbl_c70_044_001_n_X32_Y32, tbl_c70_044_001_n_X32_Y32, tbl_o10_010_001_n_X32_Y32 };
// comment Sky東島 [RQ-686] 複合機の複数ネットワーク対応.
static PNLS_Parts* pc_LogInInfoParts[] = { tbl_c70_225_000_X184_Y32, tbl_c70_225_000_X184_Y32, tbl_c70_225_000_X184_Y32, tbl_o20_225_000_X184_Y32 };
static PNLS_Parts* pc_NWInfoParts[] = { tbl_c70_224_000_X184_Y32, tbl_c70_224_000_X184_Y32, tbl_c70_224_000_X184_Y32, tbl_o20_224_000_X184_Y32 };

#else // #ifdef DEF_LCD_7INCH.
static PNLS_Parts* pc_HeaderParts[] = { prt_c70_000_000_t1_X108_Y28, prt_c70_000_000_t2_X108_Y28, prt_c70_000_000_t3_X108_Y28, prt_o20_004_000_X108_Y28 };
static PNLS_Parts* pc_SearchKeyParts[] = { tbl_c70_045_000_n_X108_Y28, tbl_c70_045_000_n_X108_Y28, tbl_c70_045_000_n_X108_Y28, tbl_o20_005_000_n_X108_Y28 };
static PNLS_Parts* pc_SearchSmallKeyParts[] = { tbl_c70_043_000_n_X48_Y28, tbl_c70_043_000_n_X48_Y28, tbl_c70_043_000_n_X48_Y28, tbl_o10_009_000_n_X32_Y28 };
static PNLS_Parts* pc_LanguageSmallKeyParts[] = { tbl_c70_044_000_n_X48_Y28, tbl_c70_044_000_n_X48_Y28, tbl_c70_044_000_n_X48_Y28, tbl_o10_010_000_n_X32_Y28 };
// comment Sky東島 [RQ-686] 複合機の複数ネットワーク対応.
static PNLS_Parts* pc_LogInInfoParts[] = { tbl_c70_223_000_X184_Y28, tbl_c70_223_000_X184_Y28, tbl_c70_223_000_X184_Y28, tbl_o20_223_000_X184_Y28 };
static PNLS_Parts* pc_NWInfoParts[] = { tbl_c70_222_000_X184_Y28, tbl_c70_222_000_X184_Y28, tbl_c70_222_000_X184_Y28, tbl_o20_222_000_X184_Y28 };

#endif // #ifdef DEF_LCD_7INCH.

static PNLS_Parts* pc_FuncBadge[12][4] = {
	{ prt_c70_001_000_t1_X164_Y96, prt_c70_001_000_t2_X164_Y96, prt_c70_001_000_t3_X164_Y96, prt_c70_001_000_t4_X164_Y96 },
	{ prt_c70_002_000_t1_X164_Y96, prt_c70_002_000_t2_X164_Y96, prt_c70_002_000_t3_X164_Y96, prt_c70_001_000_t4_X164_Y96 },
	{ prt_c70_003_000_t1_X164_Y96, prt_c70_003_000_t2_X164_Y96, prt_c70_003_000_t3_X164_Y96, prt_c70_001_000_t4_X164_Y96 },
	{ prt_c70_004_000_t1_X164_Y96, prt_c70_004_000_t2_X164_Y96, prt_c70_004_000_t3_X164_Y96, prt_c70_001_000_t4_X164_Y96 },
	{ prt_c70_005_000_t1_X164_Y96, prt_c70_005_000_t2_X164_Y96, prt_c70_005_000_t3_X164_Y96, prt_c70_001_000_t4_X164_Y96 },
	{ prt_c70_006_000_t1_X164_Y96, prt_c70_006_000_t2_X164_Y96, prt_c70_006_000_t3_X164_Y96, prt_c70_001_000_t4_X164_Y96 },
	{ prt_c70_007_000_t1_X164_Y96, prt_c70_007_000_t2_X164_Y96, prt_c70_007_000_t3_X164_Y96, prt_c70_001_000_t4_X164_Y96 },
	{ prt_c70_008_000_t1_X164_Y96, prt_c70_008_000_t2_X164_Y96, prt_c70_008_000_t3_X164_Y96, prt_c70_001_000_t4_X164_Y96 },
	{ prt_c70_009_000_t1_X164_Y96, prt_c70_009_000_t2_X164_Y96, prt_c70_009_000_t3_X164_Y96, prt_c70_001_000_t4_X164_Y96 },
	{ prt_c70_010_000_t1_X164_Y96, prt_c70_010_000_t2_X164_Y96, prt_c70_010_000_t3_X164_Y96, prt_c70_001_000_t4_X164_Y96 },
	{ prt_c70_011_000_t1_X164_Y96, prt_c70_011_000_t2_X164_Y96, prt_c70_011_000_t3_X164_Y96, prt_c70_001_000_t4_X164_Y96 },
	{ prt_c70_012_000_t1_X164_Y96, prt_c70_012_000_t2_X164_Y96, prt_c70_012_000_t3_X164_Y96, prt_c70_001_000_t4_X164_Y96 },
};

void PCGC_HomeTopMenu::changePartsForTheme()
{
	uchar _uc_Theme = 0;
	uchar uc_BadgeId = 0;		// バッジキー用のID.
	switch( PnlComGetSystemSetting()->getTopMenuCustomPattern() ) {	// NVDからテーマの選択状態を取得.
		case TYPE_TopMenuCustomPattern1:
			if( True == m_bl_IsExistEcoFuncShortcut ) {
				_uc_Theme = 3;
			} else {
				_uc_Theme = 0;
			}
			uc_BadgeId = 0;
			break;
		case TYPE_TopMenuCustomPattern2:
			_uc_Theme = 1;
			uc_BadgeId = 1;
			break;
		case TYPE_TopMenuCustomPattern3:
			_uc_Theme = 2;
			uc_BadgeId = 2;
			break;
		case TYPE_TopMenuUserCustomPattern1:
		case TYPE_TopMenuUserCustomPattern2:
		case TYPE_TopMenuUserCustomPattern3:
			_uc_Theme = 0;
			uc_BadgeId = 3;
			break;
		default:	break;
	}
	// ヘッダーエリアキー.
	Key_HomeTopMenuUniversalSetting.changeParts( pc_HeaderParts[_uc_Theme] );
	Key_HomeTopMenuCounter.changeParts( pc_HeaderParts[_uc_Theme] );
	Key_HomeTopMenuLanguage.changeParts( pc_HeaderParts[_uc_Theme] );
//	Key_HomeTopMenuJobList.changeParts( pc_HeaderParts[_uc_Theme] );
	PNLE_HeaderAreaJobListComponentMode e_Mode = PNLE_HeaderAreaJobListComponentModeTopMenuTheme1;
	switch( _uc_Theme ) {
		case 0:	e_Mode = PNLE_HeaderAreaJobListComponentModeTopMenuTheme1; break;
		case 1: e_Mode = PNLE_HeaderAreaJobListComponentModeTopMenuTheme2; break;
		case 2: e_Mode = PNLE_HeaderAreaJobListComponentModeTopMenuTheme3; break;
		case 3: e_Mode = PNLE_HeaderAreaJobListComponentModeTopMenuTheme4; break;
		default:
			break;
	}
	Cmp_HomeTopMenuHeaderAreaJobList.changeThemeForTopMenu( e_Mode );

	Key_HomeTopMenuFunctionSearch.changeParts( pc_SearchKeyParts[_uc_Theme] );
	Key_HomeTopMenuFunctionSearchSmall.changeParts( pc_SearchSmallKeyParts[_uc_Theme] );
	Key_HomeTopMenuLanguageSmall.changeParts( pc_LanguageSmallKeyParts[_uc_Theme] );

	// ページ切替えキー.
	// バッジキー.
// comment 2018/04/13 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
	for( uchar i = 0; i < m_uc_TopMenuListKeyDisp; i++ ) {
		Key_HomeTopMenuBadge[i].changeParts( pc_FuncBadge[i][uc_BadgeId] );
	}
// comment Sky東島 [RQ-686] 複合機の複数ネットワーク対応.
	// 認証情報キー.
	Key_HomeTopMenuLogInInfo.changeParts( ( True == m_bl_VlanNWShow ? pc_NWInfoParts[_uc_Theme] : pc_LogInInfoParts[_uc_Theme] ) );
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//// comment 2018/04/18 SCD2 Kazuo.Takei [Eagle/Sparrow][制御構成変更_No.111 P07_01_共通仕様(第6世代UI画面_左エリア)].
//	Cmp_LeftAreaComponent.setTypeLoginInfo( ( True == m_bl_VlanNWShow ? PNLE_LeftAreaLoginInfoType_lanNW : PNLE_LeftAreaLoginInfoType_User ) );
}
//-----------------------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） ： システムボックスの各キーの活性化判定.
*/
//-----------------------------------------------------------------------------------------------.
Bool PCGC_HomeTopMenu::getActivateSystemBoxKey( PNLS_TopMenuBadgeKey* ps_Key )
{
	Bool b_SystemBoxKey = True;		// キーの種類がシステムボックス機能の場合、True.

	PNLC_SystemBoxFunctionKeyDispStateLib c_SBLib;	// システムボックス各キーの表示状態判定ライブラリ.

	switch( ps_Key->e_ShortCutKind ) {
		// セキュリティー文書.
		// 強制メモリー受信.
		// 認証＆プリント.
		case PNLE_TopMenuShortCut_SysBoxAuthPrint:		ps_Key->bl_isActivate = c_SBLib.isActivateIDAndPrintBox();	break;
		// 掲示板.
		// ポーリング送信.
		case PNLE_TopMenuShortCut_SysBoxPolling:		ps_Key->bl_isActivate = c_SBLib.isActivatePollingTXBox();	break;
		// ファイリングナンバー.
		// 再送信.
		// パスワード暗号化PDF.
		// 外部メモリー.
		// 携帯電話/PDA.
		case PNLE_TopMenuShortCut_SysBoxMobilePDA:		ps_Key->bl_isActivate = c_SBLib.isActivateMobilePDABox();	break;
// comment 2018/08/24 SCD2-4G東島 [Eagle][1057544].
		case PNLE_TopMenuShortCut_SysBoxExternalMemory:	// 外部メモリー.
			if( TYPE_HddStatusFormated == APIC_HddInfo::newInstance()->getHddStatus() ) {	// ﾌｫｰﾏｯﾄ済場合.
				ps_Key->bl_isActivate = True;
			} else {
				ps_Key->bl_isActivate = False;
			}
			break;
		default:										b_SystemBoxKey = False;	break;
	}

	return b_SystemBoxKey;
}
//-----------------------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） ： 使用制限機能選択状態時のログイン成功通知.
*/
//-----------------------------------------------------------------------------------------------.
void PCGC_HomeTopMenu::procLoginSuccess( PNLE_LoginSuccessType e_LoginSuccessType, const PNLS_AuthenticationLoginReceptKey* ps_ReceptKey )
{
	if( TYPE_KeyModeVendorKM == PnlComGetMachineSetting()->getKeyMode() ) {	// ベンダー2装着時(キーカウンター挿入時も同様).
		// ユーザーが変更になったら開きなおす.
		onlyPanel.setTopmenuMode( True );	// 優先機能をトップメニューへ.
		rebuildGamen( 0 );
	} else {
// comment 2018/04/04 SCD2 K.Nishiki [Eagle][制御構成変更No.111]ホーム画面認証画面の新UI化対応.
		uchar _uc_index = static_cast<uchar>( ( uc_nowPage - 1 )*m_uc_TopMenuListKeyDisp + ps_ReceptKey->uc_Option );
		PNLE_TopMenuShortCut _e_TopMenuShortCut = st_BadgeKeyInfo[_uc_index].e_ShortCutKind;

		if( PNLE_AuthenticationLoginReceptKeyOption_ModeSwitch == ps_ReceptKey->e_OptionType ) {
			// モード切替/外キー.
			PPopupAnimationCom::procLoginSuccess( e_LoginSuccessType, ps_ReceptKey );
		} else {
			// 外キーモード切替時以外.
			switch( _e_TopMenuShortCut ) {
				case PNLE_TopMenuShortCut_SysBoxExternalMemory:	// 外部メモリー.
				case PNLE_TopMenuShortCut_SysBoxMobilePDA:		// 携帯電話/PDA連携ボックス.
					if( PNLE_LoginSuccessType_LimitSolved == e_LoginSuccessType ) {
						procGoToPage( ps_ReceptKey->uc_Option );
					}
					break;
				default: break;
			}
		}
	}
}
//-----------------------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） ： 使用制限機能選択状態時のユーザーログイン後に機能制限がかかっているかどうか判定を行う.
*/
//-----------------------------------------------------------------------------------------------.
Bool PCGC_HomeTopMenu::isLoginUserLimitationFunction( APIC_User* pc_SpecifiedUser, APIC_Track* pc_SpecifiedTrack, const PNLS_AuthenticationLoginReceptKey* ps_ReceptKey )
{
	Bool bl_Ret = True;
	
// comment 2018/04/04 SCD2 K.Nishiki [Eagle][制御構成変更No.111]ホーム画面認証画面の新UI化対応.
	uchar _uc_index = static_cast<uchar>( ( uc_nowPage - 1 )*m_uc_TopMenuListKeyDisp + ps_ReceptKey->uc_Option );	
	PNLE_TopMenuShortCut _e_TopMenuShortCut = st_BadgeKeyInfo[_uc_index].e_ShortCutKind;

	if( PNLE_AuthenticationLoginReceptKeyOption_ModeSwitch == ps_ReceptKey->e_OptionType ) {
		// 外キーモード切替時.
		bl_Ret = PPopupAnimationCom::isLoginUserLimitationFunction( pc_SpecifiedUser, pc_SpecifiedTrack, ps_ReceptKey );
	} else {
		// 外キーモード切替時以外.
		switch( _e_TopMenuShortCut ) {
			case PNLE_TopMenuShortCut_SysBoxExternalMemory:	// 外部メモリー.
				if( TYPE_FuncLimit_Disable != PnlComCheckAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Print, pc_SpecifiedUser, pc_SpecifiedTrack )		// プリント印字禁止.
// comment 2021/10/28 Sky東島 [IT6_MinervaSSBK][OP_BTS-34376].
//				    ||	TYPE_FuncLimit_Disable != PnlComCheckAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_ScanToUsb, pc_SpecifiedUser, pc_SpecifiedTrack )	// ScanToUSB禁止.
				    ||	( ( TYPE_FuncLimit_Disable != PnlComCheckAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_ScanToUsb, pc_SpecifiedUser, pc_SpecifiedTrack ) )	// ScanToUSB禁止.
				       && ( TYPE_FuncLimit_Disable != PnlComCheckAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Scan, pc_SpecifiedUser, pc_SpecifiedTrack ) ) )	// スキャン操作禁止.
				    ||	TYPE_FuncLimit_Disable != PnlComCheckAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_UsbMemoryReading, pc_SpecifiedUser, pc_SpecifiedTrack ) ) {	// 外部メモリ読込禁止.
					bl_Ret = False;
				}
				break;
			case PNLE_TopMenuShortCut_SysBoxMobilePDA:		// 携帯電話/PDA連携ボックス.
				if( TYPE_FuncLimit_Disable != PnlComCheckAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Print, pc_SpecifiedUser, pc_SpecifiedTrack )
				    && (
				        TYPE_FuncLimit_Disable != PnlComCheckAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Bluetooth, pc_SpecifiedUser, pc_SpecifiedTrack )	// 携帯電話/PDA使用不許可ユーザー.
				        && (
				            TYPE_FuncLimit_Disable != PnlComCheckAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Print, pc_SpecifiedUser, pc_SpecifiedTrack )			// プリント不許可ユーザー.
				            || TYPE_FuncLimit_Disable != PnlComCheckAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_BoxOperation, pc_SpecifiedUser, pc_SpecifiedTrack )	// ボックス保存不許可ユーザー.
				        )
				    ) ) {
					bl_Ret = False;
				}
				break;
			default: break;
		}
	}

	return bl_Ret;
}
//-----------------------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） ： スキャンファクスプログラムキー表示判定制御.
* @note PCGC_ProgramBasic_scanfax::setupProgramKey()を参考にして作成.
*/
//-----------------------------------------------------------------------------------------------.
// comment 2020/03/10 SCD2-Sky中野 [IT6_2.0MR2] 制御No.30_Program登録機能.
//Bool PCGC_HomeTopMenu::getActivateProgramKey( short ss_ProgramNo )
Bool PCGC_HomeTopMenu::getActivateProgramKey( short ss_ProgramNo, Bool bl_fromWebUI )
{
	// プログラムキー情報.
	PCGC_TopmenuProgramKeyData* pc_TopmenuProgramKeyData = NULL;
	TYPC_ProgramKey* pc_ProgramKey = NULL;
	PCGS_TopmenuProgramKeyInfo* ps_ProgramKeyInfo = NULL;
	TYPD_AddressSearchTarget ul_SearchedTarget = 0;
	// プログラムキー情報を取得する。
// comment 2020/03/10 SCD2-Sky中野 [IT6_2.0MR2] 制御No.30_Program登録機能.
	if( True == bl_fromWebUI ) {
		PNLC_HomeProgramKeyData* pc_HomeProgramKeyData = PNLC_HomeProgramKeyData::getInstance();
		pc_ProgramKey = pc_HomeProgramKeyData->getProgramKeyData( ss_ProgramNo );
		ps_ProgramKeyInfo = pc_HomeProgramKeyData->getProgramKeyInfo( ss_ProgramNo );
		ul_SearchedTarget = pc_HomeProgramKeyData->getProgramKeySearchTarget( ss_ProgramNo );
	} else {
		pc_TopmenuProgramKeyData = PCGC_TopmenuProgramKeyData::getInstance();
		if( NULL != pc_TopmenuProgramKeyData ) {
			pc_ProgramKey = pc_TopmenuProgramKeyData->getProgramKeyData( ss_ProgramNo );
			ps_ProgramKeyInfo = pc_TopmenuProgramKeyData->getProgramKeyInfo( ss_ProgramNo );
			ul_SearchedTarget = pc_TopmenuProgramKeyData->getProgramKeySearchTarget( ss_ProgramNo );
		}
	}

	// NULLチェック.
	if( ( NULL == pc_ProgramKey ) || ( NULL == ps_ProgramKeyInfo ) || ( NULL == PnlComGetUser() ) ) {
		return False;
	}

	// ボックスモードでない状態で、チェックを行う為、状態をバックアップする。
	Bool b_nowBoxModeBackup = onlyPanel.b_nowBoxMode;
	PCGE_JobPutNo e_JobPutNoBackup = sc_JobPutNo;
	onlyPanel.b_nowBoxMode = False;
	sc_JobPutNo = PCGE_JobPutOther;

	Bool b_ActiveFlg = True;	// 参照許可が不可な場合にFalseをセットし、キーグレーアウト.

	do {
		// ------------------------------------------------------------------.
		// メール暗号化ON、プログラム宛先内に証明書未登録のE-mail宛先の場合.
		// ------------------------------------------------------------------.
		APIC_Job* pc_Job = PnlComGetJobScanFax();
		// NULLでない場合のみ、宛先のチェックをする.
		if( NULL != pc_Job ) {
			TYPC_Send* pc_Send = pc_Job->getJobSpec()->pc_Send;
			if( NULL != pc_Send ) {
				if( True == pc_Send->c_MailSend.getSmimeEnableCrypt() ) {  // メール暗号化ON.
					// プログラム宛先内に証明書未登録のE-mail宛先のチェック.
					if( False == ps_ProgramKeyInfo->bl_SmimeCertInfoAllLinkedAddress ) {
						b_ActiveFlg = False;	// 非アクティブ.
						break;
					}
				}
			}
		}

		// ------------------------------------------------------------------.
		// 参照許可設定.
		// ------------------------------------------------------------------.
		Bool b_ProReferGroup = ps_ProgramKeyInfo->bl_ReferLicenceGroup ;
		Bool b_ProRefer = ps_ProgramKeyInfo->bl_ReferLicence ;
		Bool b_UserRefer = PnlComGetUser()->getReferLicence() ;
		Bool b_UserReferGroup = PnlComGetUser()->getReferLicenceGroup() ;
		ushort us_ProReferGroupID = ps_ProgramKeyInfo->us_ReferLicenceGroupID ;

		Bool b_ReferLicenceCheck = False;
		TYPE_ReferLicenceLevel e_refLevel = TYPE_ReferLicenceLevel0;
		// グループによる参照レベル設定の場合は参照グループから取得する.
		if( True == ps_ProgramKeyInfo->bl_ReferLicenceGroup ) {
			// 参照許可グループをプログラム宛先の持つＩＤから作成.
			APIC_ReferLicence* pc_ReferLicence = APIC_ReferLicence::newInstance( us_ProReferGroupID );
			// 作成に成功した場合は参照レベルを取得する.
			if( NULL != pc_ReferLicence ) {
				e_refLevel = pc_ReferLicence->getReferLevel();
			}
		}
		if( b_ProRefer == True ) {					// 宛先に参照許可設定あり.
			if( b_ProReferGroup == True ) {				// 宛先に参照許可グループ設定あり.
				if( b_UserRefer == True ) {				// ユーザーに参照許可レベル設定あり.
					if( PnlComGetUser()->getReferLicenceLevel() >= e_refLevel ) {	// ユーザーのレベルが宛先のレベル以上.
						b_ReferLicenceCheck = True;
					}
				}
				if( ( b_UserReferGroup == True ) &&			// ユーザーに参照許可グループ設定有り？かつ.
				    ( b_ReferLicenceCheck != True ) ) {	// 参照許可レベルでは許可されていない?.
					// ユーザーが対象のプログラム宛先と同じグループに属してるかチェック.
					// 参照許可グループボックスの上限は20.
					ushort i ;
					for( i = 1; i <= 20; i++ ) {
						// 【ユーザー】??番目の参照許可グループボックスを参照許可に設定している且つ.
						// 【プログラム宛先＋ユーザー】参照許可グループIDが一致.
						if( ( True == PnlComGetUser()->getReferLicenceGroupNo( i ) ) &&
						    ( i == us_ProReferGroupID ) )
						{
							// プログラム宛先とユーザーの参照許可グループが一致した為.
							// ユーザーのプログラム宛先への参照は許可する.
							b_ReferLicenceCheck = True;
							break;
						}
					}
				}
			} else {									// 宛先に参照許可レベル設定あり.
				if( b_UserRefer == True ) {				// ユーザーに参照許可レベル設定あり.
					if( PnlComGetUser()->getReferLicenceLevel() >= ps_ProgramKeyInfo->e_ReferLicenceLevel ) {	// ユーザーのレベルが宛先のレベル以上.
						b_ReferLicenceCheck = True ;
					}
				}
			}
		} else {										// 宛先に参照許可設定がされていない.
			// 対象の宛先は参照許可関連の設定をしていない為、ユーザーの参照を認める.
			b_ReferLicenceCheck = True;
		}
		if( False == b_ReferLicenceCheck ) {
			b_ActiveFlg = False;	// 非アクティブ.
			break;
		}

		// オフフック中か.
		if( PNLC_ScanFaxDataChangeCheckLib::newInstance()->isNowFaxManualTxRxMode() == True ) {
			// 紐づく宛先がグループ宛先.
			if( 0 != ps_ProgramKeyInfo->ul_DbRecordNoOfGroupAddress ) {
				// 宛先情報をクリアして表示(プログラムキーに名称なし＋グレーアウトで表示する為).
				b_ActiveFlg = False;
				break;
				// 紐づく宛先がG3Faxでない.
			} else if( TYPE_SendModeFax != ps_ProgramKeyInfo->e_SendMode ) {
				// 宛先情報をクリアして表示(プログラムキーに名称なし＋グレーアウトで表示する為).
				b_ActiveFlg = False;
				break;
			}
		} else {
			// ------------------------------------------------------------------.
			// グループ宛先設定.
			// ------------------------------------------------------------------.
			if( 0 != ps_ProgramKeyInfo->ul_DbRecordNoOfGroupAddress ) {			// グループ宛先が登録されている.
				if( True != onlyPanel.isFaxFunctionUseCheck( PDPE_FaxFunctionMultiAddrBan ) ) { // 複数グループ宛先のチェック.
					b_ActiveFlg = False;	// 非アクティブ.
					break;
				}
			}

			// ------------------------------------------------------------------.
			// 送信可能な宛先設定.
			// ------------------------------------------------------------------.
// comment 2015/11/19 OPS1-Sky辻原 [FQ-OPSD-1500255-OFP182] ファクス画面からトップメニューに遷移したとき、トップメニューに設定したファクス/スキャンプログラムキーがグレーアウトする.
			//uint32_t ul_SearchTarget = PnlComGetProgramSearchTarget();			// 有効宛先種別を取得.
// comment 2022/05/16 Sky中野 [IT6_2.3] 制御No.38_アプリ割付数の拡大.
//			uint32_t ui32_SearchTarget = PnlComGetProgramSearchTarget( True );			// 有効宛先種別を取得.
			uint32_t ui32_SearchTarget = TYPD_AddressSearchTargetAll;
			// 有効宛先種別を取得.
			if( True == bl_fromWebUI ) {
				ui32_SearchTarget = PNLC_HomeProgramKeyData::getInstance()->getSearchTarget();
			} else {
				ui32_SearchTarget = PnlComGetProgramSearchTarget( True );
			}
			if( ul_SearchedTarget != ( ul_SearchedTarget & ui32_SearchTarget ) ) {	// 有効でない宛先が登録されている.
				b_ActiveFlg = False;	// 非アクティブ.
				break;
			}

			// ------------------------------------------------------------------.
			// 組禁チェック.
			// ------------------------------------------------------------------.
// comment 2020/03/10 SCD2-Sky中野 [IT6_2.0MR2] 制御No.30_Program登録機能.
//			if( SendInfoCombinationCheck( ss_ProgramNo ) == False ) {
			if( False == SendInfoCombinationCheck( ss_ProgramNo, bl_fromWebUI ) ) {
				// プログラムに登録されている通信設定の組禁をチェック.
				b_ActiveFlg = False;	// 非アクティブ.
				break;
			}
// comment 2015/11/10 OPS1-Sky浜野 [MinervaSBK][CQ520906] 白黒２値対応.
// comment 2020/03/10 SCD2-Sky中野 [IT6_2.0MR2] 制御No.30_Program登録機能.
//			if( False == OrglInfoCombinationCheck( ss_ProgramNo ) ) {
			if( False == OrglInfoCombinationCheck( ss_ProgramNo, bl_fromWebUI ) ) {
				// プログラムに登録されている原稿画質設定の組禁をチェック.
				b_ActiveFlg = False;	// 非アクティブ.
				break;
			}
		}
	} while( 0 );

	// バックアップしていた情報に戻す.
	onlyPanel.b_nowBoxMode = b_nowBoxModeBackup;
	sc_JobPutNo = e_JobPutNoBackup;

	return b_ActiveFlg;
}
//-----------------------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） ： 通信組禁のチェック
* @note PCGC_ScanFaxProgramComLib::SendInfoCombinationCheck()を移植
*/
//-----------------------------------------------------------------------------------------------.
// comment 2020/03/10 SCD2-Sky中野 [IT6_2.0MR2] 制御No.30_Program登録機能.
//Bool PCGC_HomeTopMenu::SendInfoCombinationCheck( short ss_ProgramNo )
Bool PCGC_HomeTopMenu::SendInfoCombinationCheck( short ss_ProgramNo, Bool bl_fromWebUI )
{
	// プログラムキー情報.
	PCGC_TopmenuProgramKeyData* pc_TopmenuProgramKeyData = NULL;
	TYPC_ProgramKey* pc_ProgramKey = NULL;
	PCGS_TopmenuProgramKeyInfo* ps_ProgramKeyInfo = NULL;
	// プログラムキー情報を取得する。
// comment 2020/03/10 SCD2-Sky中野 [IT6_2.0MR2] 制御No.30_Program登録機能.
	if( True == bl_fromWebUI ) {
		PNLC_HomeProgramKeyData* pc_HomeProgramKeyData = PNLC_HomeProgramKeyData::getInstance();
		pc_ProgramKey = pc_HomeProgramKeyData->getProgramKeyData( ss_ProgramNo );
		ps_ProgramKeyInfo = pc_HomeProgramKeyData->getProgramKeyInfo( ss_ProgramNo );
	} else {
		pc_TopmenuProgramKeyData = PCGC_TopmenuProgramKeyData::getInstance();
		if( NULL != pc_TopmenuProgramKeyData ) {
			pc_ProgramKey = pc_TopmenuProgramKeyData->getProgramKeyData( ss_ProgramNo );
			ps_ProgramKeyInfo = pc_TopmenuProgramKeyData->getProgramKeyInfo( ss_ProgramNo );
		}
	}
	// NULLチェック.
	if( ( NULL == pc_ProgramKey ) || ( NULL == ps_ProgramKeyInfo ) ) {
		return False;
	}

	if( NULL == PnlComGetJobScanFax() ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed PCGC_HomeTopMenu::SendInfoCombinationCheck !" );
		return( True );
	}
	TYPC_JobSpec* pc_JobSpec = PnlComGetJobScanFax()->getJobSpec();
	TYPS_SendInfo s_SendInfo;
	char sc_Hour;				// タイマー送信設定情報(時間).
	char sc_Minute;				// タイマー送信設定情報(分).
	Bool b_ScanOn = False;		// JobSpecのスキャン系宛先の設定状況.
	Bool b_NetFaxOn = False;	// JobSpecのネットファクス系宛先の設定状況.

	if( ( pc_ProgramKey == NULL ) || ( pc_JobSpec == NULL ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return( True );
	}
	if( 0 == ps_ProgramKeyInfo->ul_IgcRecordId ) {
		// 選択中のプログラムが未登録の場合.
		return( True );
	}

	// JobSpecのIPFax宛先登録状況.
	if( isIPAddressFaxSelect( pc_JobSpec ) == True ) {
		b_NetFaxOn = True;
	}
	// JobSpecのIFax宛先登録状況.
	if( isIFaxAddressFaxSelect( pc_JobSpec ) == True ) {
		b_NetFaxOn = True;
	}
	// JobSpecのEmail宛先登録状況.
	if( isEmailAddressSelect( pc_JobSpec ) == True ) {
		b_ScanOn = True;
	}
	// JobSpecのBox宛先登録状況.
	if( isBoxAddressSelect( pc_JobSpec ) == True ) {
		b_ScanOn = True;
	}
	// JobSpecのFTP宛先登録状況.
	if( isFTPAddressSelect( pc_JobSpec ) == True ) {
		b_ScanOn = True;
	}
	// JobSpecのSMB宛先登録状況.
	if( isSMBAddressSelect( pc_JobSpec ) == True ) {
		b_ScanOn = True;
	}
	// JobSpecのWebDAV宛先登録状況.
	if( isWebDAVAddressSelect( pc_JobSpec ) == True ) {
		b_ScanOn = True;
	}

	s_SendInfo = ps_ProgramKeyInfo->st_SendInfo;

	sc_Hour = s_SendInfo.c_FaxTimerAccessValue.sc_Hour;				// タイマー通信に設定されている時間を取得.
	sc_Minute = s_SendInfo.c_FaxTimerAccessValue.sc_Minute;			// タイマー通信に設定されている分を取得.
	if( ( sc_Hour != TYPD_NotSetting ) || ( sc_Minute != TYPD_NotSetting ) ) {
		// プログラムにタイマー送信が登録されている場合.
		if( ( b_NetFaxOn == True ) || ( b_ScanOn == True ) ) {
			// JobSpecにスキャン系宛先またはネットファクス系宛先が存在する場合.
			return( False );
		}
	}

	if( ( ( s_SendInfo.e_TxMode == TYPE_TxModeMailbox ) || ( s_SendInfo.e_TxMode == TYPE_TxModeRelayReq ) )
	    && ( '\0' != s_SendInfo.uc_FaxSub[ 0 ] ) ) {
		// プログラムにFコード送信が登録されている場合.
		if( b_NetFaxOn == True ) {
			// JobSpecにネットファクス系宛先が存在する場合.
			return( False );
		}
	}
	if( ( TYPE_TxModeClose == s_SendInfo.e_TxMode ) && ( '\0' != s_SendInfo.uc_FaxPwd[ 0 ] ) ) {
		// プログラムにパスワード送信が登録されている場合.
		if( b_NetFaxOn == True ) {
			// JobSpecにネットファクス系宛先が存在する場合.
			return( False );
		}
	}
	return( True );
}

// comment 2015/11/10 OPS1-Sky浜野 [MinervaSBK][CQ520906] 白黒２値対応.
// comment 2020/03/10 SCD2-Sky中野 [IT6_2.0MR2] 制御No.30_Program登録機能.
//Bool PCGC_HomeTopMenu::OrglInfoCombinationCheck( short ss_ProgramNo )
Bool PCGC_HomeTopMenu::OrglInfoCombinationCheck( short ss_ProgramNo, Bool bl_fromWebUI )
{
	Bool bl_Check = True;
	
	// プログラムキー情報.
	PCGC_TopmenuProgramKeyData *pc_TopmenuProgramKeyData = NULL;
	TYPC_ProgramKey* pc_ProgramKey = NULL;
	// プログラムキー情報を取得する。
// comment 2020/03/10 SCD2-Sky中野 [IT6_2.0MR2] 制御No.30_Program登録機能.
	if( True == bl_fromWebUI ) {
		PNLC_HomeProgramKeyData* pc_HomeProgramKeyData = PNLC_HomeProgramKeyData::getInstance();
		pc_ProgramKey = pc_HomeProgramKeyData->getProgramKeyData( ss_ProgramNo );
	} else {
		pc_TopmenuProgramKeyData = PCGC_TopmenuProgramKeyData::getInstance();
		if ( NULL != pc_TopmenuProgramKeyData ) {
			pc_ProgramKey = pc_TopmenuProgramKeyData->getProgramKeyData( ss_ProgramNo );
		}
	}
	// NULLチェック.
	if( NULL == pc_ProgramKey ) {
		return bl_Check;
	}
// comment 2015/12/24 【HCL-Harikrishnan】【CQ-519679】Klockwork Changes 【ID:33554】 [NPD.FUNC.MUST].
//	if( NULL == PnlComGetJobScanFax() ) {
	APIC_Job* pc_Job = PnlComGetJobScanFax();
	if( NULL == pc_Job ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed PCGC_HomeTopMenu::OrglInfoCombinationCheck !" );
		return bl_Check;
	}
//	TYPC_JobSpec* pc_JobSpec = PnlComGetJobScanFax()->getJobSpec();
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	TYPS_OriginalInfo	s_OrgInfo;

	if( ( NULL == pc_ProgramKey ) || ( NULL == pc_JobSpec ) ){
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return bl_Check;
	}
	if( 0 == pc_ProgramKey->getIgcRecodeId() ){
		// 選択中のプログラムが未登録の場合.
		return bl_Check;
	}

	s_OrgInfo = pc_ProgramKey->getOriginalInfo();

	// JobSpecのBox宛先登録状況.
	if( True == isBoxAddressSelect( pc_JobSpec ) ){
		// 原稿画質：カスタムが設定されている場合.
		if( TYPE_OriginalImageModeCustom == s_OrgInfo.e_OriginalImageMode ) {
			bl_Check = False;
		}
	}
	
	return bl_Check;
}

// comment MinervaZeus -> Sky本橋 2011/08/02 三河技術課題No.52決定事項 [CQ352223].
//-----------------------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） ： レイヤー番号の設定.
*/
//-----------------------------------------------------------------------------------------------.
void PCGC_HomeTopMenu::setLayerNumber( char sc_LayerNum )
{
	char	sc_LayerNumTemp = sc_LayerNum;

	if( 0 == sc_LayerNum ) {
		if( ( 0 != m_uc_LayerCount ) && ( PNLD_PopupMaxNum >= m_uc_LayerCount ) ) {
			m_sc_LayerNum[m_uc_LayerCount - 1] = 0;
			m_uc_LayerCount--;

			if( 0 != m_uc_LayerCount ) {
				sc_LayerNumTemp = m_sc_LayerNum[m_uc_LayerCount - 1];
			}
		} else {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return;
		}
	} else {
		if( PNLD_PopupMaxNum > m_uc_LayerCount ) {
			m_sc_LayerNum[m_uc_LayerCount] = sc_LayerNumTemp;
			m_uc_LayerCount++;
		} else {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return;
		}
	}

	PPopupAnimationCom::setLayerNumber( sc_LayerNumTemp );
}
//-----------------------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） ： ポップアップ起動要求.
*/
//-----------------------------------------------------------------------------------------------.
void PCGC_HomeTopMenu::opened( void ( *p_CallbackFunc )( uchar, short, void*, uint32_t ), short ss_SrcPosX, short ss_SrcPosY, short ss_SrcSizeX, short ss_SrcSizeY, Bool bl_AnimLess, Bool b_IgnoreQueueInfo )
{
	e_PopupStatus = PNLE_PopupStatus_Non;
	PPopupAnimationCom::opened( p_CallbackFunc, ss_SrcPosX, ss_SrcPosY, ss_SrcSizeX, ss_SrcSizeY, bl_AnimLess, b_IgnoreQueueInfo );
}
//-----------------------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） ： ポップアップ終了要求.
*/
//-----------------------------------------------------------------------------------------------.
void PCGC_HomeTopMenu::closed( Bool bl_Delay, Bool bl_Immediately, Bool b_IgnoreQueueInfo )
{
	PPopupAnimationCom::closed( bl_Delay, bl_Immediately, b_IgnoreQueueInfo );
	if( 0 < m_uc_LayerCount ) {
		e_PopupStatus = PNLE_PopupStatus_Open;
	}
}
//-----------------------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） ： パーツ非表示.
*/
//-----------------------------------------------------------------------------------------------.
void PCGC_HomeTopMenu::hide()
{
	// 複数の統合トップメニューを開いていた場合、最後のポップアップクローズ時以外は
	// パーツのHide()処理を行うと下の統合トップメニューに戻った際に
	// パーツ非表示と判断されてボタンが押せなくなる。
	// そのため最後の統合トップメニューである場合のみパーツのHide()を行うよう対応.
	if( 1 == m_uc_LayerCount ) {
		PPopupAnimationCom::hide();
	}
}

// ===============================================================================================
/**
@brief	.Shortcuts key function restriction check when coin not inserted in vendor.
*/
// ===============================================================================================
Bool PCGC_HomeTopMenu::checkShortCutDisableCoinExist( PNLE_TopMenuShortCut e_TopMenuShortCutType )
{
	Bool b_ret = True;
	Bool b_Coin = True;
	TYPE_AppAtAdminMachineVendor e_Copy = TYPE_AppAtAdminMachineVendorEnableWithCardOrCoin;
	TYPE_AppAtAdminMachineVendor e_Send = TYPE_AppAtAdminMachineVendorEnableWithCardOrCoin;

	b_Coin = APIC_KeyDevice::isCoinExist();		// Get whether coin exists or not.

	switch( e_TopMenuShortCutType ) {
		case PNLE_TopMenuShortCut_CopyColorMode :
		case PNLE_TopMenuShortCut_CopyPaperType :
		case PNLE_TopMenuShortCut_CopyMag :
		case PNLE_TopMenuShortCut_CopyPage :
		case PNLE_TopMenuShortCut_CopyFinishing :
		case PNLE_TopMenuShortCut_CopyDensity :
		case PNLE_TopMenuShortCut_CopyOriginalImage :
			e_Copy = PnlComGetSystemSetting()->getCopyAtAdminMachineVendor();
			if( TYPE_AppAtAdminMachineVendorEnableWithCardOrCoin == e_Copy ) {
				if( b_Coin == False ) {
					b_ret = False;  // Do not display.
				}
			}
			break;
		case PNLE_TopMenuShortCut_ScanFaxDuplexMode :
		case PNLE_TopMenuShortCut_ScanFaxResolution :
		case PNLE_TopMenuShortCut_ScanFaxColor :
		case PNLE_TopMenuShortCut_ScanFaxFileFormat :
		case PNLE_TopMenuShortCut_ScanFaxReadSize :
		case PNLE_TopMenuShortCut_ScanFaxEMail :
// comment 2016/02/02 OPS1-Sky善[特ROM][IT5-14_機種マージ]暗号化PDF操作フロー改善要望.
		case PNLE_TopMenuShortCut_ScanFaxEncryptPDFPassword:
			e_Send = PnlComGetSystemSetting()->getSendAtAdminMachineVendor();
			if( TYPE_AppAtAdminMachineVendorEnableWithCardOrCoin == e_Send ) {
				if( b_Coin == False ) {
					b_ret = False;  // Do not display.
				}
			}
			break;
		default :
			break;
	}
	return b_ret;
}

//-----------------------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） ： AppReqオブザーバからのコール.
*/
//-----------------------------------------------------------------------------------------------.
void PCGC_HomeTopMenu::procAppReqObserver( TYPE_PNL_AppReqObserver option )
{
// comment 2021/07/30 Sky Kawanishi [LocalUI特ROM]Extログ改善.
	PanelLog(PNLD_LogKind_Ext, "PCGC_HomeTopMenu::procAppReqObserver option=%d", option, 0,0,0,0,0);
	// メッセージ欄共通: AppReqオブザーバからのコール.
	PMessageArea::procAppReqObserver( option );
}
//-----------------------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） ： ファクスデフォルトアプリ判定.
*/
//-----------------------------------------------------------------------------------------------.
Bool PCGC_HomeTopMenu::checkFaxDefaultAppOn()
{
	Bool bl_isFaxAppOn = False;		// ファクスデフォルトアプリの有効判定.
	if( ( 0 < ExtSet.getDefaultAppNo() )
	    && ( True == ExtSet.isRegistGeneralApplication( TYPE_JobTypeFaxSendMemory ) )
	    && ( onlyPanel.bl_isEnlargeDisplayMode == False )
	    && ( onlyPanel.ec_IntMode == EC_IntMode_Off ) ) {
		bl_isFaxAppOn = True;
	}
	return bl_isFaxAppOn;
}
//-----------------------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） ： ユーザー切り替わり判定処理.
*/
//-----------------------------------------------------------------------------------------------.
Bool PCGC_HomeTopMenu::checkSupporNowLoginUserAndTrack()
{
	// ユーザーIDの比較 .
	int32_t i32_nowUserNo = 0 ;
	if( PnlComGetUser() ) {
		i32_nowUserNo = PnlComGetUser()->getUserNo() ;
	}
	Bool b_ChengedUser = this->m_sl_SupportUserNo != i32_nowUserNo ;

	// 部門IDの比較 .
	int32_t i32_nowTrackNo = 0 ;
	if( PnlComGetTrack() ) {
		i32_nowTrackNo = PnlComGetTrack()->getTrackNo() ;
	}
	Bool b_ChengedTrack = this->m_sl_SupportTrackNo != i32_nowTrackNo ;

	// ウィンドウが非アクティブ時にユーザー、部門が変更された場合再表示 .
	if( b_ChengedUser || b_ChengedTrack ) {
		// ユーザー情報を保持する.
		this->m_sl_SupportUserNo = i32_nowUserNo  ;
		this->m_sl_SupportTrackNo = i32_nowTrackNo ;

		// コールバック登録を行う.
		CallBackIndi( TYPE_PNL_CallBack_TopMenu_UserChange );

		return True;
	}

	return False;
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： ユーザー補助キー処理.
//================================================================================================
void PCGC_HomeTopMenu::procAccessibility( uchar option )
{
	// 指定アプリ起動中の場合.
	if( c_ExtPanel.CheckSpecifiedAppActive() ) {
		// 指定アプリ起動中画面から遷移する場合は再起動を行う.
		if( PNLE_SpecifiedAppState_Started == c_ExtPanel.getSpecifiedAppState() ) {
			c_ExtPanel.req_SpecifiedAppStart();
		}
	}
	PMessageArea::procAccessibility( option );
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： 利用可能なRightFaxアプリがあるかをチェックする処理.
//================================================================================================
Bool PCGC_HomeTopMenu::checkRightFax()
{
	ushort i = 0;
	// 有効なアプリのインデックスをセット .
	for( i = 1; i <= PNLD_MaxScansendRegistApp ; i++ ) {
		if( ExtSet.checkUsedAppSelect( i ) == True ) {	// 現在使用可能なアプリである場合.
			APIC_AppInfoSetting* pc_AppInfo = APIC_AppInfoSetting::newInstance( i );	// インスタンス取得.
			if( NULL == pc_AppInfo ) { continue; }
			TYPS_AppCommunication st_AppCommunication = pc_AppInfo->getAppCommunication();
			if( TYPE_AppCommTypeRightFax == st_AppCommunication.e_AppCommType ) {		// RightFax判定.
				return True;
			}
		}
	}
	return False;
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： QtDrawコールバック ビューへ表示指示.
//================================================================================================
void PCGC_HomeTopMenu::callBackQtShow( ushort option )
{
	procCallBack( TYPE_PNL_CallBack_QtHomeTopMenuDraw_Show );
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： Qtビューへ表示指示.
//================================================================================================
void PCGC_HomeTopMenu::QtShow()
{
	PnlComMtuiIf_TopMenuView::instance()->show( m_bl_IsExistEcoFuncShortcut );
}
//-----------------------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） ： 登録アプリケーショングループ押下処理.
*/
//-----------------------------------------------------------------------------------------------.
void PCGC_HomeTopMenu::procAPPGrp( PNLE_TopMenuShortCut e_ShortCutKind )
{
    // comment 2015/12/07 OPS1-Sky_大林 [ZeusS_ZX0][CQ521402]IT5では以下条件によりグループキーが非活性になる事が無い為、コメントアウト.
	//const TYPS_ExtRegistration* pc_ExtRegistration = ScanOapExtSet.getApplication();

	// APP起動中.
	//if( ScanOapExtSet.isActiveGeneralApplication()
	//    && NULL != pc_ExtRegistration		// comment 2012/12/18 OPS1-Sky東島 [ZeusBK][CQ376130].
	//    && (	( TYPE_ExtTypeScanS 			== pc_ExtRegistration->s_Version.e_Type )
	//            ||	( TYPE_ExtTypeScanD 			== pc_ExtRegistration->s_Version.e_Type )
	//            ||	( TYPE_ExtTypePullPrint 		== pc_ExtRegistration->s_Version.e_Type )
	//            ||	( TYPE_ExtTypeBrowserScan 		== pc_ExtRegistration->s_Version.e_Type )
	//            ||	( TYPE_ExtTypeBrowserPull 		== pc_ExtRegistration->s_Version.e_Type )
	//            ||	( TYPE_ExtTypeBrowserGeneral	== pc_ExtRegistration->s_Version.e_Type ) ) ) {
	//	setBuzzerType( PCGE_BuzzerType_InputNg );
	//	return;
	//}
	// アプリケーション使用可否チェック.
	if( False == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_MAX ) ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	m_b_ShowTopMenu = True;
	switch( e_ShortCutKind ) {
		case PNLE_TopMenuShortCut_GrpAppShortCutGrp1 : nextGamen = &gp_ScanOapAppSelectRegistAppGroupExt_scanfax1; break;
		case PNLE_TopMenuShortCut_GrpAppShortCutGrp2 : nextGamen = &gp_ScanOapAppSelectRegistAppGroupExt_scanfax2; break;
		case PNLE_TopMenuShortCut_GrpAppShortCutGrp3 : nextGamen = &gp_ScanOapAppSelectRegistAppGroupExt_scanfax3; break;
		case PNLE_TopMenuShortCut_GrpAppShortCutGrp4 : nextGamen = &gp_ScanOapAppSelectRegistAppGroupExt_scanfax4; break;
		case PNLE_TopMenuShortCut_GrpAppShortCutGrp5 : nextGamen = &gp_ScanOapAppSelectRegistAppGroupExt_scanfax5; break;
		case PNLE_TopMenuShortCut_GrpAppShortCutGrp6 : nextGamen = &gp_ScanOapAppSelectRegistAppGroupExt_scanfax6; break;
		default : break;
	}

	// アプリケーション情報初期化.
	if( NULL != nextGamen ) {
		PCGC_TreeScreen::initializeNode();
	}
}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： 登録グループアプリケーショングループ情報の設定.
//================================================================================================
static const PNLS_TopMenuBadgeKeyText st_TopMenuBadgeKeyAppGrpTbl[] = {
	{ PNLE_TopMenuShortCut_GrpAppShortCutGrp1,		 NULL,	NULL, PNLD_PaletteDefault },	// 登録アプリケーショングループ1.
	{ PNLE_TopMenuShortCut_GrpAppShortCutGrp2,		 NULL,	NULL, PNLD_PaletteDefault },	// 登録アプリケーショングループ2.
	{ PNLE_TopMenuShortCut_GrpAppShortCutGrp3,		 NULL,	NULL, PNLD_PaletteDefault },	// 登録アプリケーショングループ3 .
	{ PNLE_TopMenuShortCut_GrpAppShortCutGrp4,	 	 NULL,	NULL, PNLD_PaletteDefault },	// 登録アプリケーショングループ4.
	{ PNLE_TopMenuShortCut_GrpAppShortCutGrp5,	 	 NULL,	NULL, PNLD_PaletteDefault },	// 登録アプリケーショングループ5.
	{ PNLE_TopMenuShortCut_GrpAppShortCutGrp6,	 	 NULL,	NULL, PNLD_PaletteDefault },	// 登録アプリケーショングループ6.

};
//-----------------------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） ： 登録アプリケーショングループ情報設定.
*/
//-----------------------------------------------------------------------------------------------.
void PCGC_HomeTopMenu::setAppGrpInfo( uchar uc_index, TYPE_TopMenuShortCutFunction e_TopMenuShortCutFunction )
{
	char sc_select = -1;
	TYPE_ExtGroupKeyNumber e_ExtGroupKeyNumber = TYPE_ExtGroupKeyNumber_None;
	Bool bl_AppExit = False;

	switch( e_TopMenuShortCutFunction ) {
		case TYPE_TopMenuShortCutGroupKey_1:					// 登録アプリケーショングループ1.
			sc_select = 0;
			e_ExtGroupKeyNumber = TYPE_ExtGroupKeyNumber_1;
			break;
		case TYPE_TopMenuShortCutGroupKey_2:					// 登録アプリケーショングループ2.
			sc_select = 1;
			e_ExtGroupKeyNumber = TYPE_ExtGroupKeyNumber_2;
			break;
		case TYPE_TopMenuShortCutGroupKey_3:					// 登録アプリケーショングループ3.
			sc_select = 2;
			e_ExtGroupKeyNumber = TYPE_ExtGroupKeyNumber_3;
			break;
		case TYPE_TopMenuShortCutGroupKey_4:					// 登録アプリケーショングループ4.
			sc_select = 3;
			e_ExtGroupKeyNumber = TYPE_ExtGroupKeyNumber_4;
			break;
		case TYPE_TopMenuShortCutGroupKey_5:					// 登録アプリケーショングループ5.
			sc_select = 4;
			e_ExtGroupKeyNumber = TYPE_ExtGroupKeyNumber_5;
			break;
		case TYPE_TopMenuShortCutGroupKey_6:					// 登録アプリケーショングループ6.
			sc_select = 5;
			e_ExtGroupKeyNumber = TYPE_ExtGroupKeyNumber_6;
			break;
		default : break;	// 登録アプリケーショングループ未割当.
	}

	// OAPアプリ.
	TYPS_ExtGroupKeyItem	s_ExtGroupKeyItemOap;
	memset( &s_ExtGroupKeyItemOap, 0, sizeof( s_ExtGroupKeyItemOap ) );
	PnlComGetStandardPanelSetting()->getGroupKeyItem( e_ExtGroupKeyNumber, &s_ExtGroupKeyItemOap );

	// 汎用アプリ.
	TYPS_ExtGroupKeyItem	s_ExtGroupKeyItemGeneralPurpose;
	memset( &s_ExtGroupKeyItemGeneralPurpose, 0, sizeof( s_ExtGroupKeyItemGeneralPurpose ) );
	APIC_AppInfoSetting* pc_AppInfoSetting = APIC_AppInfoSetting::newInstance( 1 );	// インスタンス取得.
	if( NULL != pc_AppInfoSetting ) {
		pc_AppInfoSetting->getGroupKeyItem( e_ExtGroupKeyNumber, &s_ExtGroupKeyItemGeneralPurpose );

	}

	if( ( 0 != s_ExtGroupKeyItemOap.ss_ExtGroupKeyInfoCnt )
	    || ( 0 != s_ExtGroupKeyItemGeneralPurpose.ss_ExtGroupKeyInfoCnt ) ) {
		bl_AppExit = True;
	}


	// 登録アプリケーショングループにアプリケーションが所属していないまたは.
	// グループキーが無効の場合.
	if( ( 0 == bl_AppExit ) ||
	    ( False == PnlComGetStandardPanelSetting()->getGroupKeyValid( (TYPE_ExtGroupKeyNumber)e_TopMenuShortCutFunction ) ) ) {
		st_BadgeKeyInfo[uc_index].bl_isShow = False;
	}

	// 登録アプリケーションが使用不可の場合.
	if( True != checkShortCutDisableAppGrp( e_ExtGroupKeyNumber ) ) {
		st_BadgeKeyInfo[uc_index].bl_isShow = False;
	}

	// PKI認証設定時かつセキュリティー強化モードが有効の場合.
	if( ( True == PLBC_FuncPKICheck::isPKI() ) && ( True == PnlComGetSystemSetting()->getSecurityLevelUpOn() ) ) {
		st_BadgeKeyInfo[uc_index].bl_isShow = False;
	}
// comment 2019/04/22 Sky東島 [IT6_1.0MR2][AR1075688].
	if( True == c_ExtPanel.getSpecifiedAppSetting()			// 初期起動アプリ設定ON.
	 && False == c_ExtPanel.isEnableMFPFunction() ){		// 基本機能使用設定許可OFF.
		st_BadgeKeyInfo[uc_index].bl_isShow = False;
	}

	if( -1 != sc_select ) {
		st_BadgeKeyInfo[uc_index].e_ShortCutKind = st_TopMenuBadgeKeyAppGrpTbl[( uchar )sc_select].e_ShortCutKind;
		st_BadgeKeyInfo[uc_index].tid_FuncName = 0;

		memset( st_BadgeKeyInfo[uc_index].sc_FuncName, '\0', sizeof( st_BadgeKeyInfo[uc_index].sc_FuncName ) );

		// グループ名は変更なければ工場出荷値で登録される為、取得したグループ名を表示.
		PnlComGetStandardPanelSetting()->getGroupKeyName( e_ExtGroupKeyNumber, ( uchar* )st_BadgeKeyInfo[uc_index].sc_FuncName );
		memset( m_ucAppGroupIconData[( uchar )sc_select], 0 , sizeof( m_ucAppGroupIconData[( uchar )sc_select] ) );

		uchar uc_GroupIconData[TYPD_ExtAppListKeyIconDataMaxSize];
		memset( uc_GroupIconData, 0x00, sizeof( uc_GroupIconData ) );		// GroupIcon.
		// comment 2015/08/26 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] アイコンデータ取得APIにてデータサイズ取得の追加.
		int si_GroupIconDataSize = 0;	//APIより取得したGroupIconのデータサイズ.

		if( False == PnlComGetStandardPanelSetting()->getGroupKeyIcon( e_ExtGroupKeyNumber, uc_GroupIconData, &si_GroupIconDataSize ) ) {
			// グループキーのアイコンが登録無.
			st_BadgeKeyInfo[uc_index].bl_isShowIcon = False;
			// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] データサイズの設定を追加.
			st_BadgeKeyInfo[uc_index].uc_IconPaletteType = st_TopMenuBadgeKeyAppGrpTbl[( uchar )sc_select].uc_IconPaletteType;
			st_BadgeKeyInfo[uc_index].puc_IconNotActivate = st_BadgeKeyInfo[uc_index].puc_FuncIcon;
			st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = st_BadgeKeyInfo[uc_index].uc_IconPaletteType;
// comment 2020/07/17 Sky東島 [IT6_2.0MR3][OP_BTS-25505].
//			st_BadgeKeyInfo[uc_index].ss_FuncIconDataSize = 0;
//			st_BadgeKeyInfo[uc_index].ss_IconNotActivateDataSize = 0;
			st_BadgeKeyInfo[uc_index].us_FuncIconDataSize = 0;
			st_BadgeKeyInfo[uc_index].us_IconNotActivateDataSize = 0;
		} else {
			// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] 標準PNGと独自PNGの判定結果を追加.
			// グループキーのアイコンが登録有.
			switch( PnlComGetStandardPanelSetting()->checkIconFormat( uc_GroupIconData ) ) {
				case TYPE_ExtIconFormat_Bmp:	// 24bit 64×80dot Bitmap.
				case TYPE_ExtIconFormat_Png:	// 標準PNG形式.
					// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] 旧来の独自RAW変換処理を削除.
					//PnlComGetRawBmpData( uc_GroupIconData , ( char* )m_ucAppGroupIconData[( uchar )sc_select], 64, 80 );
					//st_BadgeKeyInfo[uc_index].bl_BMP = True;
					memcpy( m_ucAppGroupIconData[( uchar )sc_select], uc_GroupIconData, sizeof( m_ucAppGroupIconData[( uchar )sc_select] ) );
					st_BadgeKeyInfo[uc_index].bl_BMP = False;
					st_BadgeKeyInfo[uc_index].bl_isShowIcon = True;
					// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] 標準BMP/標準PNG時のデータサイズ/パレット設定を追加.
					st_BadgeKeyInfo[uc_index].uc_IconPaletteType = PNLD_PaletteNull;
					st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = PNLD_PaletteNull;
					// GFW層はデータサイズ非0時に、データ内容を標準BMP/標準PNGと判断するため、アイコン画像のデータサイズを設定する.
					// comment 2015/08/26 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] アイコンデータ取得APIにてデータサイズ取得の追加.
					// アイコンデータはshort上限サイズ以下のため、shortへキャスト.
// comment 2020/07/17 Sky東島 [IT6_2.0MR3][OP_BTS-25505].
//					st_BadgeKeyInfo[uc_index].ss_FuncIconDataSize =  (short) si_GroupIconDataSize;
//					st_BadgeKeyInfo[uc_index].ss_IconNotActivateDataSize =  (short) si_GroupIconDataSize;
					st_BadgeKeyInfo[uc_index].us_FuncIconDataSize =  (ushort) si_GroupIconDataSize;
					st_BadgeKeyInfo[uc_index].us_IconNotActivateDataSize =  (ushort) si_GroupIconDataSize;
					break;

				case TYPE_ExtIconFormat_Other:	// 従来APIから送信されるPNG形式（dat）.
				case TYPE_ExtIconFormat_OriginalPng:	// 従来APIから送信されるKM独自PNG形式（dat）.
					memcpy( m_ucAppGroupIconData[( uchar )sc_select], uc_GroupIconData, sizeof( m_ucAppGroupIconData[( uchar )sc_select] ) );
// comment 2015/12/14 OPS1 Sky東島 [ZeusSZX0][CQ521810].
					setBilinearFlag( ( char* )m_ucAppGroupIconData[( uchar )sc_select] );
					st_BadgeKeyInfo[uc_index].bl_BMP = False;
					st_BadgeKeyInfo[uc_index].bl_isShowIcon = True;
					// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] 独自PNG時のデータサイズ/パレット設定を追加.
					st_BadgeKeyInfo[uc_index].uc_IconPaletteType = st_TopMenuBadgeKeyAppGrpTbl[( uchar )sc_select].uc_IconPaletteType;
					st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = st_BadgeKeyInfo[uc_index].uc_IconPaletteType;
					// GFW層はデータサイズ0時に、データ内容をKM独自PNGと判断するため、データサイズ0を設定する.
// comment 2020/07/17 Sky東島 [IT6_2.0MR3][OP_BTS-25505].
//					st_BadgeKeyInfo[uc_index].ss_FuncIconDataSize = 0;
//					st_BadgeKeyInfo[uc_index].ss_IconNotActivateDataSize = 0;
					st_BadgeKeyInfo[uc_index].us_FuncIconDataSize = 0;
					st_BadgeKeyInfo[uc_index].us_IconNotActivateDataSize = 0;
					break;
// comment 2020/6/26 Sky東島 [IT6_2.0MR3][制御構成変更_No.37]SVGサポート対応.
				case TYPE_ExtIconFormat_Svg:		// SVG形式.
					memcpy( m_ucAppGroupIconData[( uchar )sc_select], uc_GroupIconData, sizeof( m_ucAppGroupIconData[( uchar )sc_select] ) );
					st_BadgeKeyInfo[uc_index].bl_BMP = False;
					st_BadgeKeyInfo[uc_index].bl_isShowIcon = True;
					st_BadgeKeyInfo[uc_index].uc_IconPaletteType = PNLD_PaletteNull;
					st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = PNLD_PaletteNull;
// comment 2020/07/17 Sky東島 [IT6_2.0MR3][OP_BTS-25505].
//					st_BadgeKeyInfo[uc_index].ss_FuncIconDataSize = (short) si_GroupIconDataSize;	// 標準BMP/標準PNGならばデータサイズをバッファ長にする.
//					st_BadgeKeyInfo[uc_index].ss_IconNotActivateDataSize = (short) si_GroupIconDataSize;
					st_BadgeKeyInfo[uc_index].us_FuncIconDataSize = (ushort) si_GroupIconDataSize;	// 標準BMP/標準PNGならばデータサイズをバッファ長にする.
					st_BadgeKeyInfo[uc_index].us_IconNotActivateDataSize = (ushort) si_GroupIconDataSize;
					st_BadgeKeyInfo[uc_index].bl_SvgData = True;
					break;
				case TYPE_ExtIconFormat_BmpError:	// 破損データまたは未サポートデータ.
				default:
					st_BadgeKeyInfo[uc_index].bl_BMP = False;
					st_BadgeKeyInfo[uc_index].bl_isShowIcon = False;
					// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] エラー時のパレット/データサイズ設定を追加.
					st_BadgeKeyInfo[uc_index].uc_IconPaletteType = PNLD_PaletteNull;
					st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = PNLD_PaletteNull;
// comment 2020/07/17 Sky東島 [IT6_2.0MR3][OP_BTS-25505].
//					st_BadgeKeyInfo[uc_index].ss_FuncIconDataSize = 0;
//					st_BadgeKeyInfo[uc_index].ss_IconNotActivateDataSize = 0;
					st_BadgeKeyInfo[uc_index].us_FuncIconDataSize = 0;
					st_BadgeKeyInfo[uc_index].us_IconNotActivateDataSize = 0;
					break;
			}


			st_BadgeKeyInfo[uc_index].puc_FuncIcon = m_ucAppGroupIconData[( uchar )sc_select];
			// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] アイコン登録時の非アクティブ画像設定を、ここに移動.
			st_BadgeKeyInfo[uc_index].puc_IconNotActivate = st_BadgeKeyInfo[uc_index].puc_FuncIcon;
		}
		// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] データ形式別のパレット設定が必要であるため、パレットの共通処理を削除.データ形式別の処理へ移動.
		// st_BadgeKeyInfo[uc_index].uc_IconPaletteType = st_TopMenuBadgeKeyAppGrpTbl[( uchar )sc_select].uc_IconPaletteType;
		// st_BadgeKeyInfo[uc_index].puc_IconNotActivate = st_BadgeKeyInfo[uc_index].puc_FuncIcon;
		// st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = st_BadgeKeyInfo[uc_index].uc_IconPaletteType;
	}
}
//-----------------------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） ： アプリケーション非活性判定処理.
*/
//-----------------------------------------------------------------------------------------------.
void PCGC_HomeTopMenu::IsAppDisableCheck( uchar uc_index )
{
    // comment 2015/12/07 OPS1-Sky_大林 [ZeusS_ZX0][CQ521402]IT5では以下条件によりグループキーが非活性になる事が無い為、コメントアウト.
	//const TYPS_ExtRegistration* pc_ExtRegistration = ScanOapExtSet.getApplication();

	//if( ScanOapExtSet.isActiveGeneralApplication()
	//    && ( NULL != pc_ExtRegistration )
	//    && (	( TYPE_ExtTypeScanS 			== pc_ExtRegistration->s_Version.e_Type )
	//            ||	( TYPE_ExtTypeScanD 			== pc_ExtRegistration->s_Version.e_Type )
	//            ||	( TYPE_ExtTypePullPrint 		== pc_ExtRegistration->s_Version.e_Type )
	//            ||	( TYPE_ExtTypeBrowserScan 		== pc_ExtRegistration->s_Version.e_Type )
	//            ||	( TYPE_ExtTypeBrowserPull 		== pc_ExtRegistration->s_Version.e_Type )
	//            ||	( TYPE_ExtTypeBrowserGeneral	== pc_ExtRegistration->s_Version.e_Type ) ) ) {
	//	st_BadgeKeyInfo[uc_index].bl_isActivate = False;
	//	return;
	//}

	// BrowserLauncher起動時はグレイアウト.
	//if( ( True == onlyPanel.getBrowserMode() ) &&								// ブラウザ起動中.
	//    ( BRWE_BrowserBootMode_Launcher == onlyPanel.e_BrowserBootMode ) ) {	// BrowserLauncherモード.
	//	st_BadgeKeyInfo[uc_index].bl_isActivate = False;
	//	return;
	//
	//}
	// RightFax起動時はグレイアウト.
	//if( ExtSet.isActiveGeneralApplication() ) {
	//	st_BadgeKeyInfo[uc_index].bl_isActivate = False;
	//	return;
	//}
	st_BadgeKeyInfo[uc_index].bl_isActivate = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_MAX );
}
//-----------------------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） ： 登録アプリケーション使用可否処理.
*/
//-----------------------------------------------------------------------------------------------.
Bool PCGC_HomeTopMenu::checkShortCutDisableAppGrp( TYPE_ExtGroupKeyNumber e_ExtGroupKeyNumber )
{
	Bool 					bl_ret 	= False;
	int						i_AppNo = 0;										// アプリ番号.
	uchar 					i 		= 0;										// カウンター.
	TYPS_ExtGroupKeyItem	s_ExtGroupKeyItem;									// グループキー情報.

	// OAPアプリケーション使用可否チェック /////////////////////////////////////////////////////.
	// グループキー情報取得.
	memset( &s_ExtGroupKeyItem, 0, sizeof( s_ExtGroupKeyItem ) );
	PnlComGetStandardPanelSetting()->getGroupKeyItem( e_ExtGroupKeyNumber, &s_ExtGroupKeyItem );

	// グループキーに登録されているOAPアプリをOAP管理データへ格納.
	for( i = 0; i < TYPD_ExtBelongToGroupKeyMaxNum; i++ ) {
		// アプリケーション番号取得.
		i_AppNo = ( int )s_ExtGroupKeyItem.ss_ExtGroupKeyInfo[i];
		// 「0」終端.
		if( 0 == i_AppNo ) {
			break;
		}

		// 登録アプリケーション情報取得
		APIC_ExtSetting* pc_ExtSetting = APIC_ExtSetting::newInstance( i_AppNo );							// アプリ番号からアプリ情報取得.

		if( NULL != pc_ExtSetting ) {
			TYPS_ExtRegistration s_ExtRegistration = pc_ExtSetting->getApplication();

// comment 2022/06/30 Sky東島 [IT6_2.3][制御No.39-42_目的別UIプリセット化].
			if( ( TYPE_ExtTypeCustomUI ==s_ExtRegistration.s_Version.e_Type )
			 && ( PNLE_UIModeLegacyUI == PNLC_WebUIControl::getInstance()->getUIMode() ) ) {
				// クラシックスタイルでは目的別UIアプリは表示対象外.
				continue;
			}
			if( ( TYPE_ExtEntryOn == s_ExtRegistration.e_Entry )											// アプリ登録済み.
			    && ( True == PCGC_ScanOapCom::checkUserLimitToApl( s_ExtRegistration.s_Version.e_Type, (ushort)i_AppNo ) ) ) { 	// アプリのユーザー権限が許可.
				bl_ret = True;
				return bl_ret;	// 使用可能であるため、以降のチェックは不要なので終了.
			}
		}
	}

	// 汎用アプリケーション使用可否チェック /////////////////////////////////////////////////////.
	APIC_AppInfoSetting* pc_AppInfoSetting = APIC_AppInfoSetting::newInstance( 1 );	// インスタンス取得.
	if( NULL == pc_AppInfoSetting ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return bl_ret;
	}

	// グループキー情報取得.
	memset( &s_ExtGroupKeyItem, 0, sizeof( s_ExtGroupKeyItem ) );
	pc_AppInfoSetting->getGroupKeyItem( e_ExtGroupKeyNumber, &s_ExtGroupKeyItem );

	// グループキーに登録されているOAPアプリをOAP管理データへ格納.
	for( i = 0; i < TYPD_ExtBelongToGroupKeyMaxNum; i++ ) {
		// 登録アプリケーション番号取得
		i_AppNo = ( int )s_ExtGroupKeyItem.ss_ExtGroupKeyInfo[i];	// アプリ番号.

		if( 0 == i_AppNo ) {	// 「0」終端.
			break;
		}

		// 登録アプリケーション情報取得.
		APIC_AppInfoSetting* pc_AppInfoSetting = APIC_AppInfoSetting::newInstance( (short)i_AppNo );		// アプリ番号からアプリ情報取得.

		if( NULL != pc_AppInfoSetting ) {
			if( True == ExtSet.checkUsedAppSelect( (ushort)i_AppNo ) ) { 		// 現在使用可能なアプリである場合.
				bl_ret = True;
				break;
			}
		}
	}
	return bl_ret;
}

//-----------------------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） ： 日立ICCショートカットキー対応.
* 2013/08/23 OPS1-Sky奥村 [特Rom対応マージ][MZ-16] 日立ICC FeliCa/MifareカードAD直接連携 追加要望Ph1.
*/
//-----------------------------------------------------------------------------------------------.
void PCGC_HomeTopMenu::setBadgeKeyData_HitachiICC()
{
	memset( &st_BadgeKeyInfo, 0x00, sizeof( st_BadgeKeyInfo ) );

	// １ページのみ.
	uc_maxPage = 1;

	// 日立の樹にかぶらないように座標位置を調整 (16ピクセル上に移動する).
	static short ss_gy = Key_HomeTopMenuBadge[0].getPositionY();
	if( ss_gy == Key_HomeTopMenuBadge[0].getPositionY() ) {
		Key_HomeTopMenuBadge[0].shiftPosition( 0, -16 );
		Key_HomeTopMenuBadge[1].shiftPosition( 0, -16 );
		Key_HomeTopMenuBadge[2].shiftPosition( 0, -16 );
		Key_HomeTopMenuBadge[3].shiftPosition( 0, -16 );
	}

	// コピー.
	// ユーザー機能制限（コピー）判定によりショートカットキーの設定を行う.
	if( TYPE_FuncLimit_Disable != PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Copy ) ) {
		st_BadgeKeyInfo[0].e_ShortCutKind				= st_TopMenuBadgeKeyFuncTbl[0].e_ShortCutKind;
		st_BadgeKeyInfo[0].tid_FuncName					= st_TopMenuBadgeKeyFuncTbl[0].tid_FuncName;
		st_BadgeKeyInfo[0].puc_FuncIcon					= st_TopMenuBadgeKeyFuncTbl[0].puc_FuncIcon;
		st_BadgeKeyInfo[0].uc_IconPaletteType			= st_TopMenuBadgeKeyFuncTbl[0].uc_IconPaletteType;
		st_BadgeKeyInfo[0].puc_IconNotActivate		 	= st_TopMenuBadgeKeyFuncTbl[0].puc_IconNotActivate;
		st_BadgeKeyInfo[0].uc_IconPaletteNotActivate	= st_TopMenuBadgeKeyFuncTbl[0].uc_IconPaletteNotActivate;
		st_BadgeKeyInfo[0].bl_isShow = True;
		st_BadgeKeyInfo[0].bl_isShowIcon = True;
	}

	// セキュリティプリント.
	// ユーザー機能制限（ボックス）判定によりショートカットキーの設定を行う.
	if( TYPE_FuncLimit_Disable != PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_BoxOperation ) ) {
		st_BadgeKeyInfo[1].e_ShortCutKind				= st_TopMenuBadgeKeySystemBoxTbl[0].e_ShortCutKind;
		st_BadgeKeyInfo[1].tid_FuncName					= s_pKEYSecurityPrint_W108H042E;
		st_BadgeKeyInfo[1].puc_FuncIcon					= st_TopMenuBadgeKeySystemBoxTbl[0].puc_FuncIcon;
		st_BadgeKeyInfo[1].uc_IconPaletteType			= st_TopMenuBadgeKeySystemBoxTbl[0].uc_IconPaletteType;
		st_BadgeKeyInfo[1].puc_IconNotActivate		 	= st_TopMenuBadgeKeySystemBoxTbl[0].puc_IconNotActivate;
		st_BadgeKeyInfo[1].uc_IconPaletteNotActivate	= st_TopMenuBadgeKeySystemBoxTbl[0].uc_IconPaletteNotActivate;
		st_BadgeKeyInfo[1].bl_isShow = True;
		st_BadgeKeyInfo[1].bl_isShowIcon = True;
	}

	// セキュリティスキャン.
	// ユーザー機能制限（スキャン）判定によりショートカットキーの設定を行う.
	if( TYPE_FuncLimit_Disable != PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Scan ) ) {
		st_BadgeKeyInfo[2].e_ShortCutKind				= PNLE_TopMenuShortCut_HitachICC_SecurityScan;
		st_BadgeKeyInfo[2].tid_FuncName					= s_pKEYSecurityScan_W108H042E;
		st_BadgeKeyInfo[2].puc_FuncIcon					= st_TopMenuBadgeKeyFuncTbl[1].puc_FuncIcon;
		st_BadgeKeyInfo[2].uc_IconPaletteType			= st_TopMenuBadgeKeyFuncTbl[1].uc_IconPaletteType;
		st_BadgeKeyInfo[2].puc_IconNotActivate		 	= st_TopMenuBadgeKeyFuncTbl[1].puc_IconNotActivate;
		st_BadgeKeyInfo[2].uc_IconPaletteNotActivate	= st_TopMenuBadgeKeyFuncTbl[1].uc_IconPaletteNotActivate;
		st_BadgeKeyInfo[2].bl_isShow = True;
		st_BadgeKeyInfo[2].bl_isShowIcon = True;
	}

	// ファクス.
	Bool bl_isFaxEnabled = True;
	APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
	if( False == pc_HddInfo->getHddExist() ) {
		bl_isFaxEnabled = False;
	} else {
		if( TYPE_HddStatusFormated != pc_HddInfo->getHddStatus() ) {									// ﾌｫｰﾏｯﾄ済でない場合.
			//FAXボード装着時.
			PNLE_IsFaxMode e_isFaxmode = PNLE_IsFaxMode_NON;										// FAX接続種別.
			onlyPanel.isFaxModekind( &e_isFaxmode );
			if( ( e_isFaxmode == PNLE_IsFaxMode_RFAX ) || ( e_isFaxmode == PNLE_IsFaxMode_NON ) ) {	// FAXﾎﾞｰﾄﾞ非装着かFaxサーバー接続時の場合.
				bl_isFaxEnabled = False;
			}
		} else {
			//フォーマット済みの場合、サービス>システム2のFAX装着状態を確認する.
			if( False == onlyPanel.isFaxExist() ) {
				//FAXボード非装着時.
				bl_isFaxEnabled = False;
			}
		}
	}

	if( True == bl_isFaxEnabled ) {
		// ユーザー機能制限（ファクス）判定によりショートカットキーの設定を行う.
		if( TYPE_FuncLimit_Disable != PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Fax ) ) {
			st_BadgeKeyInfo[3].e_ShortCutKind				= PNLE_TopMenuShortCut_HitachICC_Fax;
			st_BadgeKeyInfo[3].tid_FuncName					= s_pKEYFax_W100H042E;
			st_BadgeKeyInfo[3].puc_FuncIcon					= st_TopMenuBadgeKeyFuncTbl[1].puc_FuncIcon;
			st_BadgeKeyInfo[3].uc_IconPaletteType			= st_TopMenuBadgeKeyFuncTbl[1].uc_IconPaletteType;
			st_BadgeKeyInfo[3].puc_IconNotActivate		 	= st_TopMenuBadgeKeyFuncTbl[1].puc_IconNotActivate;
			st_BadgeKeyInfo[3].uc_IconPaletteNotActivate	= st_TopMenuBadgeKeyFuncTbl[1].uc_IconPaletteNotActivate;
			st_BadgeKeyInfo[3].bl_isShow = True;
			st_BadgeKeyInfo[3].bl_isShowIcon = True;
		}
	}

	// 設定メニュー.
	// 登録キーに設定メニューが割り当てられている場合、キーを非表示にする.
	Bool bl_registUtilityKey = False;
	// 2020/4/17 Sky [IT6-2.0MR3] MZ-16対応 add.
	TYPS_ApplicationSoftwareKeySetting s_CustomizeKeyId;	// 設定されている登録キー情報.
	memset( &s_CustomizeKeyId, 0, sizeof( s_CustomizeKeyId ) );
	if( TYPE_AppKeyResultOK == ( PnlComGetApplicationKeySetting()->getApplicationSoftwareKeyId( &s_CustomizeKeyId ) ) ) {
		for( ushort us_index=0; us_index < PNLD_MAX_CustomizeSoftKey; us_index++ ) {
			if( TYPE_ApplicationKeyIdUtility == s_CustomizeKeyId.uc_ApplicationSoftwareKeyId[us_index] ) {
				bl_registUtilityKey = True;
				break;
			}
		}
	}
	if( False == bl_registUtilityKey ) {
		// 登録キーに設定メニューが割り当てられていないので、設定キーを表示.
		st_BadgeKeyInfo[11].e_ShortCutKind = PNLE_TopMenuShortCut_Utility;
		st_BadgeKeyInfo[11].tid_FuncName = s_pKEYActivitySettingsMenu_W100H042E;
//Comment 2016/10/07 【HCLT-Tharun】【IT5_3.3】【Accurev ID:1005414】Changed the PNG for 10.1 Inch Models.
#if defined(DEF_RES_WSVGA)
		st_BadgeKeyInfo[11].puc_FuncIcon = ( uchar* )i75_023_000_X81_Y100;
#else	// #if defined(DEF_RES_WSVGA).
		st_BadgeKeyInfo[11].puc_FuncIcon = ( uchar* )i70_023_000_X64_Y80;
#endif	// #if defined(DEF_RES_WSVGA).
		st_BadgeKeyInfo[11].uc_IconPaletteType = PNLD_PaletteCommon;
//Comment 2016/10/07 【HCLT-Tharun】【IT5_3.3】【Accurev ID:1005414】Changed the PNG for 10.1 Inch Models.
#if defined(DEF_RES_WSVGA)
		st_BadgeKeyInfo[11].puc_IconNotActivate = ( uchar* )i75_023_000_X81_Y100;
#else	// #if defined(DEF_RES_WSVGA).
		st_BadgeKeyInfo[11].puc_IconNotActivate = ( uchar* )i70_023_000_X64_Y80;
#endif // #if defined(DEF_RES_WSVGA).
		st_BadgeKeyInfo[11].uc_IconPaletteNotActivate = PNLD_PaletteCommon;
		st_BadgeKeyInfo[11].bl_isShow = True;
		st_BadgeKeyInfo[11].bl_isShowIcon = True;
	}
}

//-----------------------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） ： 日立ICCプロジェクトコード表示対応.
* 2013/08/23 OPS1-Sky奥村 [特Rom対応マージ][MZ-16] 日立ICC FeliCa/MifareカードAD直接連携 追加要望Ph1.
*/
//-----------------------------------------------------------------------------------------------.
void PCGC_HomeTopMenu::setupProjectCode()
{
	Bool b_showFlag = False;
// comment 2018/10/30 SCD2-Sky中野 [Sparrow][1066121].
//	if( True == PNLC_HitachiICC_TopMenuCustomize::newInstance()->isHitachiICC_Mode() ) {
	if( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() ) {
		b_showFlag = False;
	} else if( True == PNLC_HitachiICC_TopMenuCustomize::newInstance()->isHitachiICC_Mode() ) {
		// 外部認証サーバー情報を取得.
		APIC_AuthenticationSeverSetting* pc_AuthenticationServerSetting = APIC_AuthenticationSeverSetting::newInstance();
		APIS_AuthenticationServerInfo srv_info_buf;
		APIC_User* pc_User = PnlComGetUser();
		if( NULL != pc_AuthenticationServerSetting && NULL != pc_User ) {
			// 部門情報読取り機能有無判定.
			pc_AuthenticationServerSetting->getServerInfo( PNLD_TRACNAMEINFO_INDEX, &srv_info_buf, PNLC_NetworkLan::getInstance()->getLine() );	// サーバー情報取得.
			if( TYPE_AuthenticationServerTypeNone != srv_info_buf.e_ServerType ) {	// サーバータイプが未定義以外.
				// プロジェクトコードを設定.
				char sc_String[PNLD_UserAccountTrackNameLen + 1];
				char sc_String_UTF8[PNLD_UserAccountTrackNameLen_UTF8 + 1];
				memset( sc_String, 0, sizeof( sc_String ) );
				memset( sc_String_UTF8, 0, sizeof( sc_String_UTF8 ) );
				// ログイン部門名取得.
				pc_User->getLoginTrackName( sc_String );
				if( 0 != strlen( sc_String ) ) {
					// UTF8変換(CommomAPIで使用するフォントセットを取得し、その文字列をUTF8に変換).
					uchar	seqValid	= 0;
					PnlComGetSeqCustomValid( &seqValid );
					if ( TYPD_SeqCustom_Function_VALID == seqValid ) {
						// カスタム有効時は、UACで保存されるプロジェクトコードの文字コードがUTF-8のため変更しない.
// comment 2015/09/29 MSE寺田 [ZeusS_ZX0][CQ517932]文字列丸め対応.
						chkCorrectRoundStr( sc_String );
						Tfd_HomeTopProjctCode.changeExternalText( (uchar*)sc_String, (short)strlen(sc_String) );
					} else {
						LIBF_ConvertToUTF8( ( LIBE_CharSet )getCharSetForCAPI(), sc_String, sc_String_UTF8, sizeof( sc_String_UTF8 ) );
						Tfd_HomeTopProjctCode.changeExternalText( ( uchar* )sc_String_UTF8, (short)strlen( sc_String_UTF8 ) );
					}
					b_showFlag = True;
				}
			}
		}
	}
	// プロジェクトコードを表示/非表示設定.
	Bak_HomeTopProjctCodeArea.showEnable( b_showFlag );
}

//-----------------------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） ： ログイン情報表示.
* 2013/08/23 OPS1-Sky奥村 [特Rom対応マージ][MZ-16] 日立ICC FeliCa/MifareカードAD直接連携 追加要望Ph1.
*/
//-----------------------------------------------------------------------------------------------.
void PCGC_HomeTopMenu::setupLogInInfo()
{
	// 認証情報表示エリアの表示/非表示.
	Bool bl_LogInInfo = False;
	char sc_LogInInfoString[PCGD_UserNameUTF8_193];
// comment Sky東島 [RQ-686] 複合機の複数ネットワーク対応.
//	if( True == PnlCom_isUserAccountInfoDenotation() ) {	// 認証情報表示/非表示判定.
	if( ( True == PnlCom_isUserAccountInfoDenotation() )	// 認証情報表示/非表示判定.
	 || ( True == PNLC_NetworkLan::getInstance()->getVlanSetting() ) ) {	// トップメニューはVLAN設定が有効の場合は表示する.
		bl_LogInInfo = True;
		memset( sc_LogInInfoString, '\0', sizeof( sc_LogInInfoString ) );
//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Jegan 2015/02/27 Mod Start.
	//	PnlCom_isUserAccountInfo( ( char* )sc_LogInInfoString, sizeof( sc_LogInInfoString ) - 1 ); 	// 表示用文字列を取得.
		PNLC_RotationMsgHeaderArea::newInstance()->RotationMsgHeaderArea( sc_LogInInfoString );
//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Jegan 2015/02/27 Mod End.
		Tfd_HomeTopMenuLogInInfo.changeExternalText( ( uchar* )sc_LogInInfoString );
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//// comment 2018/04/18 SCD2 Kazuo.Takei [Eagle/Sparrow][制御構成変更_No.111 P07_01_共通仕様(第6世代UI画面_左エリア)].
//		Cmp_LeftAreaComponent.changeTextStringLoginInfo( ( const uchar* )sc_LogInInfoString );
	}

	if( True == PNLC_HitachiICC_TopMenuCustomize::newInstance()->isHitachiICC_Mode() ) {
		bl_LogInInfo = False;
	}

	Key_HomeTopMenuLogInInfo.showEnable( bl_LogInInfo );
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//// comment 2018/04/18 SCD2 Kazuo.Takei [Eagle/Sparrow][制御構成変更_No.111 P07_01_共通仕様(第6世代UI画面_左エリア)].
//	Cmp_LeftAreaComponent.showEnableLoginInfo( bl_LogInInfo );
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： アプリ登録情報初期化.
//================================================================================================
void PCGC_HomeTopMenu::initRegisterApp()
{
	// OAPアプリ.
	short AppNoOap;
	m_us_TotalCountOap = 0;
	for( AppNoOap = 1; AppNoOap <= TYPD_ExtMaxApplication; AppNoOap++ ) {
		// アプリケーション毎にインスタンス生成.
		APIC_ExtSetting* aplOap = APIC_ExtSetting::newInstance( AppNoOap );
		if( NULL == aplOap ) {
			// インスタンスNULLチェック.
			continue;
		}
// comment 2019/04/09 【HCL-Ranjith】【OP_BTS-4239】【IT6_EagleH】 MarketPlace icon display.
		Bool bl_MarketPlaceApp = False;
		char sc_AppName[TYPD_ExtMaxAplName*3+1] = PNLD_MarketPlaceString;
		char sc_OapAppName[TYPD_ExtMaxAplName*3+1] = {0,};
		strncpy( sc_OapAppName, aplOap->getApplication().sc_NameUTF8, sizeof( sc_OapAppName ) - 1 );
		if  ( 0 == strcmp( sc_AppName, sc_OapAppName )) {
			bl_MarketPlaceApp = True;
		} else {
			bl_MarketPlaceApp = False;
		}
		if( ( TYPE_ExtEntryOn == aplOap->getApplication().e_Entry ) &&
// comment 2019/04/09 【HCL-Ranjith】【OP_BTS-4239】【IT6_EagleH】 MarketPlace icon display.
//		    ( PCGC_ScanOapCom::checkUserLimitToApl( aplOap->getApplication().s_Version.e_Type, (ushort)AppNoOap ) ) ) {
		    ( PCGC_ScanOapCom::checkUserLimitToApl( aplOap->getApplication().s_Version.e_Type, (ushort)AppNoOap, True , bl_MarketPlaceApp ) ) ) {

			s_registerApp[ m_us_TotalCountOap++ ] = AppNoOap;
// Comment 2017/12/19 [HCL-Catherine] [IT5 4.2][RQ-686] [ WP1377]The new API is used and to detect the current network type, new API arguments are passed.
// comment 2019/05/28 [HCL-Naveena][EagleBK] NetworkType Default selection activity.
//    TYPE_NetworkSpecific e_NetworkSpecific = PNLC_NetworkLan::getInstance()->getLine();
      TYPE_NetworkSpecific e_NetworkSpecific = PNLC_NetworkLan::getInstance()->getLineForPanel();
// comment 2019/19/02 【HCLT_Rajesh】【OP_BTS-2291】 pass the "TYPE_NetworkSpecific_NW1" to avoid back trace.
		     if ( e_NetworkSpecific == TYPE_NetworkSpecific_NONE ) {
	    		e_NetworkSpecific = TYPE_NetworkSpecific_NW1;
	    	}
//			if( False == PnlComGetOpenApiSetting()->getEnableOpenAPI() )
		    if( False == PnlComGetOpenApiSetting()->getEnableOpenAPI( e_NetworkSpecific) )
			{
				if( TYPE_ExtTypeBrowserLauncher == aplOap->getApplication().s_Version.e_Type
				    || TYPE_ExtTypeScanS == aplOap->getApplication().s_Version.e_Type ) {
					bl_AppButtonDisplay = True;
				}
			}
			else
			{
				bl_AppButtonDisplay = True;
			}
		}
	}
	if( True == ExtSet.isRegistGeneralApplication() ) {
		bl_AppButtonDisplay = True;
	}
// comment 2019/04/13 【HCL-Prem】【IT6_1.0MR】【AR1075599】 display manipulation for IWS APP Type.
	if ( False == bl_AppButtonDisplay ) {
		APIC_IWSAppInfo* pc_IWSAppInfo = APIC_IWSAppInfo::newInstance();
		if ( NULL != pc_IWSAppInfo ) {
			uchar uc_IWSAppCount = ( uchar )pc_IWSAppInfo->getApplicationNum();
			if ( uc_IWSAppCount > 1 ) {
				uchar uc_IwsAppTypeGeneralCnt = 0;
				TYPS_IwsAppInfo s_tempAppInfo;
				memset( &s_tempAppInfo, '\0', sizeof( s_tempAppInfo ) );
				// インストールされているIWSアプリのアプリタイプを取得.
				for( uchar uc_Cnt = 0; uc_Cnt < uc_IWSAppCount; uc_Cnt++ ) {
					int si_Ret = pc_IWSAppInfo->getApplication( (int)( uc_Cnt+1 ), s_tempAppInfo );
					if ( ( TYPD_IwsResult_OK == si_Ret )
						&& ( TYPE_IwsAppTypeGeneral == s_tempAppInfo.e_AppType ) ) {
						// アプリタイプがTYPE_IwsAppTypeGeneralのものは表示対象なのでカウント.
						uc_IwsAppTypeGeneralCnt++;
					}
				}
				if ( uc_IwsAppTypeGeneralCnt >0 ) {
					bl_AppButtonDisplay = True;
				}
			}
		}
	}

}
//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： OAPアプリ終了.
//================================================================================================
void PCGC_HomeTopMenu::setMenuEnable( Bool bl_MenuEnable )
{
	m_bl_MenuEnable = bl_MenuEnable;
}


//関数------------------------------------------------------------------------------------------------------------------.
//概要    ：警告画面存在チェック.
//備考    ：.
//戻値    ：True:警告画面が存在する、False:警告画面が存在しない.
//作成者  ：.
//レビュー：.
//変更履歴：.
//---------------------------------------------------------------------------------------------------------------------.
Bool PCGC_HomeTopMenu::existWarningGamen()
{
// comment 2015/09/11 OPS1-Sky藤岡 [ZeusS_ZX0][CQ516675].
	onlyPanel.clearIsWarningRet();							// 警告を再チェックするためチェック結果をクリア.
//  comment 2016/01/19 OPS1-Sky守川 [特ROM][FQ-ISD3G-1600010-OFP013].
//	onlyPanel.m_bl_SkipVendorWarning = True;				// 警告抑止(コイン未投入).
	onlyPanel.m_bl_SkipKeyCounterWarning = True;										// 警告抑止(キーカウンタ未装着).
	if ( ( True == APIC_KeyDevice::isKeyCntIFVndMode() ) 								// キーカウンタI/Fベンダーモード.
		&&	( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBit( 39, 3 ) ) ) {	// OAPとキーカウンタI/Fベンダーモード併用設定.
		onlyPanel.m_bl_SkipVendorWarning = True;										// 警告抑止(コイン未投入).
	}

	// 全画面警告発生のチェック.
	ushort us_warntbl = onlyPanel.isAllGamenWarning( onlyPanel.cc_DisplayJob );

//  comment 2016/01/19 OPS1-Sky守川 [特ROM][FQ-ISD3G-1600010-OFP013].
	onlyPanel.m_bl_SkipKeyCounterWarning = False;			// 警告抑止解除(キーカウンタ未装着).
// comment 2015/09/11 OPS1-Sky藤岡 [ZeusS_ZX0][CQ516675].
	onlyPanel.m_bl_SkipVendorWarning = False;				// 警告抑止解除(コイン未投入).

	// 10783000_ベンダー装着時管理者番号入力画面は警告画面が存在する.
	Bool b_ModeDisable = False ;
	b_ModeDisable = onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, PNL_OTHER_VendorAdminInput ) ;	// ベンダー管理者パスワード入力中 .

	if( ( 0 != us_warntbl )
	    ||( True == b_ModeDisable ) ) {
		return True;
	}

	return False;
}

// comment 2015/04/02 MSE田中優介 [MinervaSBK][RQ-278] LDAPサーバーへのカード情報登録機能対応.
//関数--------------------------------------------------------------------------.
//概要    LDAPサーバーへのカード情報登録機能の終了処理.
//備考    ：.
//戻値    ：なし.
//作成者  ：MSE田中優介.
//レビュー：.
//変更履歴：2015/04/08 MSE田中優介 [MinervaSBK][RQ-278] LDAPサーバーへのカード情報登録機能対応.
//------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） : LDAPサーバーへのカード情報登録機能の終了処理.
*/
//------------------------------------------------------------------------------.
void PCGC_HomeTopMenu::endAutoCardRegist()
{
	PNLC_CardAuthDirectLink* pc_CardAuthDirectLink = PNLC_CardAuthDirectLink::newInstance();
	if ( NULL == pc_CardAuthDirectLink ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}

	// カードID登録機能が動作中でない場合、処理を終了する.
	if ( ( False == pc_CardAuthDirectLink->isCardDirectLinkMode() )			// 特ROM機能のカードAD直接連携無効、かつ LDAP-ICカード認証サーバーへのカード登録許可設定無効.
	  || ( False == pc_CardAuthDirectLink->getAutoCardRegistState() ) ) {	// カードID登録機能無効時.
		return;
	}

	APIC_AuthDeviceCommon* pc_AuthDevCom = PnlComGetAuthDeviceCommon();
	if ( NULL == pc_AuthDevCom ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	// 登録／変更時のキャンセル通知.
	pc_AuthDevCom->AuthUserDataCancel( NULL );
	
	// 外部サーバーへのカードID登録機能OFF.
	pc_CardAuthDirectLink->setAutoCardRegistState( False );
	
	// ユーザー入力情報を保持しない.
	onlyPanel.bl_UserAccountInit = False;
	
// comment 2015/10/13 OPS1-MSE竹中 [MinervaSBK][CQ519224][RQ-278] カード終了処理の呼び出し契機で認証ポーリングを再開させる.
	// 認証ポーリングの再開.
	PnlComReqAuthPollingRestart();
	
	// 戻り先画面を初期化.
	setTurnGamenThis( NULL );
}
// comment 2015/06/04 OPS1-Sky_nozaki [MinervaSBk][UV-01457]トップメニューへアドレス帳キー割付.
//関数--------------------------------------------------------------------------.
//概要    ：アドレス帳非活性化判定処理.
//戻値    ：True:非活性化 Flase：活性化.
//変更履歴：2015/06/04 Sky_nozaki [MinervaSBK][UV-01457] トップメニューへアドレス帳キー割付.
//------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） : アドレス帳非活性化判定処理.
*/
//------------------------------------------------------------------------------.
Bool PCGC_HomeTopMenu::checkDisableAddressBook()
{
	Bool bl_Result = False;
	
	// HDD初期化済み
	APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
	if ( NULL != pc_HddInfo ) {
// comment 2017/07/11 SCD2-Sky森山 [Icarus][CQ532482] HDD非装着時はSSDの初期化完了をチェックする.
//		// HDD装着状態.
//		if( True == pc_HddInfo->getHddExist() ) {
//			// HDDフォーマット済み?.
//			if( TYPE_HddStatusFormated != pc_HddInfo->getHddStatus() ) {
//				bl_Result = True;
//			}
//		}
		// ストレージフォーマット済み?.
//Comment 2018/05/15 【HCLT-Neelima】 【IT6_Sparrow】 【制御構成変更_No.100】 【WP-2547】Hide Temporary one touch key for SSD less models.
//		if( TYPE_HddStatusFormated != pc_HddInfo->getHddStatus() ) {
		if( ( TYPE_HddStatusFormated != pc_HddInfo->getHddStatus() )
			&& ( TYPE_MachineTypeSparrow2AIO != PnlComGetSystemInfo()->getMachineType() ) ) {
		bl_Result = True;
		}
	}
	
// comment 2016/10/07 SCD2-Sky佐藤 [IT5_3.3][1005928] ジョブ飛び越し対応.
	// 割り込みモード中の場合、使用不可.
	if( EC_IntMode_Off != onlyPanel.ec_IntMode ) {
		bl_Result = True;
	}

	return bl_Result;
}

//================================================================================================
// 概要	： A072002000_トップメニュー（統合トップメニュー） ： IWSトップメニューを表示する.
// 変更履歴 : 2015/03/02 OPS1-Sky稲井 [特ROM対応][MZV-35_機種マージ] 新規作成.
//              以下のようにブラウザ再起動要求を連続でコールした場合に、最後の指定したURLでブラウザ表示されない不具合がある.
//                  再起動要求①　URL="", BootMode=BRWE_BrowserBootMode_Default.
//                  再起動要求②　URL="", BootMode=BRWE_BrowserBootMode_Reboot.
//                  再起動要求③　URL="SEWのコンテンツパス", BootMode=BRWE_BrowserBootMode_Launcher.
//              本来は、ブラウザ側での対処すべき不具合であるが、修正による影響度が高い為、再起動要求③について、.
//              再起動完了タイミングと待ち合わせする対応をパネルで実施する.
//              機種開発ストリームへの投入する場合は、正式な対応を検討すること.
//================================================================================================
//void PCGC_HomeTopMenu::showIWSTopMenu()
void PCGC_HomeTopMenu::showIWSTopMenu( Bool bl_ToDefault )
{
	Bool bl_Exec = False;							// 実行フラグ.
	TYPE_ExtType e_AppType = TYPE_ExtTypeOtherA;	// アプリタイプ.

	// IWSメニューアプリケーションを検索する.
	for( uchar i = 1; i <= TYPD_ExtMaxApplication; i++ ) {

		// アプリインスタンスの取得.
		APIC_ExtSetting* extBuf = APIC_ExtSetting::newInstance( i );
		if( !extBuf ) continue;
		// アプリ情報取得.
		TYPS_ExtRegistration extRegBuf = extBuf->getApplication(); 
	
		// IWSメニューアプリケーション名が一致.
		//SEWアプリ名変更.
//		if ( 0 == strcmp( "Public_IWS", extRegBuf.sc_NameUTF8 ) ) {
		if ( 0 == strcmp( "SEW", extRegBuf.sc_NameUTF8 ) ) {
			// アプリ実行チェック.
			if ( False == ScanOapExtSet.checkUserLimitToApl( extRegBuf.s_Version.e_Type ) ) {
				return;
			}
			bl_Exec = True;
			e_AppType = extRegBuf.s_Version.e_Type;
			break;
		}
	}

	// アプリケーション実行可.
	if ( True == bl_Exec ) {
		// ブラウザ起動可.
// comment 2019/04/15 SCD2-Sky小畑 [EagleH] メモリフル時のフェールセーフ対応.
//		if( True == onlyPanel.bl_BrowserStartUpFlag ) {
		if( ( True == onlyPanel.bl_BrowserStartUpFlag )
		 && ( False == PNLC_BrowserChangeTabControl::getInstance()->isBrowserProcessRestart() ) ){
			// 基本画面遷移かつ、既にブラウザ再起動待ちの場合は、WaitExにて再起動完了の待ち合わせを行いIWSトップメニュー画面に遷移する.
			if ( ( True == bl_ToDefault ) && ( True == onlyPanel.b_BrowserRebootWait ) ) {
				gp_AuthOapBrowserWaitEx.reqShowIWSTopMenu();
				gp_AuthOapBrowserWaitEx.changeWaitGamen( &gp_AppChangeBrowser );
				if ( &gp_AuthOapBrowserWaitEx != nowGamen ) {
					nextGamen = &gp_AuthOapBrowserWaitEx;
				}
				return;
			}

			// カレントアプリNoの設定.
			ScanOapExtSet.setCurrentAppNo( i );
			// Appボタン以外からの起動.
			ScanOapExtSet.setCheckMode( True );
			// 起動元アプリ初期化.
			PMessageArea::setAppFunctionType( JBLE_OAP_NONE );	

			PM_TaskID priTask;
			switch ( e_AppType ) {
				case TYPE_ExtTypeBrowserLauncher:
					PMGC_PanelManager::getPriority( &priTask );
					if( NULL != pc_PanelManager ) {
						if ( PM_TASKID_BROWSER == priTask && True == pc_PanelManager->getBrowserReboot() ) {
							pc_PanelManager->setBrowserReboot(False);			// ブラウザ再起動フラグ設定.						
						}
					}
					// ブラウザ起動モードをブラウザーランチャーとして再起動する.
					PnlComBrowserLauncherModeReboot();
					PUsercom::procBrowser( 0 );
					if( (NULL != pc_PanelManager) && (True == pc_PanelManager->getBrowserReboot())) {
						//ログアウト時、ブラウザ再起動要求フラグ設定が立っている為,パネル<->ブラウザ間の画面切り替えが発生した場合、
						//ブラウザ再起動要求を投げてブラウザ画面表示される。その現象を解消する為、ランチャー起動要求を投げた後は、
						//必ずブラウザ再起動要求を落とす処理を実施する.
						pc_PanelManager->setBrowserReboot(False);			// ブラウザ再起動フラグ設定.						
					}
					if( &gp_AppChangeBrowser == nextGamen ){
						ScanOapExtSet.setExtAppType( TYPE_ExtTypeBrowserLauncher );
					}

					break;
				default :
					LOGF_debug_PutEventLog(LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!");
					break;
			}
		}
	}
	return;
}

//------------------------------------------------------------------------------.
/**
* @brief A072002000_トップメニュー（統合トップメニュー） : アドレス帳画面遷移処理.
*/
//------------------------------------------------------------------------------.
void PCGC_HomeTopMenu::procAddressBook( uchar option )
{
	// 宛先件数検索条件を作成.
	TYPS_SEARCH_ADDRESS_NUM_INFO s_SearchAddressNumInfo;
	s_SearchAddressNumInfo.pC_user			= PnlComGetUser();						// ユーザ情報.
	s_SearchAddressNumInfo.e_InspectLogIF	= JLGE_InspectLogIfOperationPanel;		// 要求元先.
	s_SearchAddressNumInfo.e_Owner			= TYPE_ADDR_OWNER_KIND_PUBLIC;			// 共有宛先.
	s_SearchAddressNumInfo.e_addrDataKind	= TYPE_AddressDivisionShort;			// Address種類.
	
	// 登録済み宛先件数取得要求.
	c_RegisteredAddressNumSearchIGC.requestRegisteredAddressNumSearch( &s_SearchAddressNumInfo );

	// 初期設定.
	pc_ManageIGCWaitDisplay.initManageIGCWaitDisplay( PCGD_IGCDisplayStandard, &gp_AddressBookTop_act, getNowGamen() );

	// IGCWait画面遷移.
	nextWaitDisplay_popup();
}
// comment 2015/09/29 MSE寺田 [ZeusS_ZX0][CQ517932]文字列丸め対応.
//------------------------------------------------------------------------------
/**
*	@brief 認証情報のUTF8文字あふれ除去処理.
*	@param[in]	pc_str	対応文字列(対応サイズ：PNLD_UserAccountTrackNameLen).
*/
//------------------------------------------------------------------------------
void PCGC_HomeTopMenu::chkCorrectRoundStr( char* pc_str )
{
	char sc_Utf8One[7] = {0};		// UTF8 1文字格納.
	ushort us_Utf8OneSize;		// UTF8 1文字のサイズ.
	ushort us_sumByte = 0;		// UTF-8文字列の合計サイズ.
	char sc_dstStr[PNLD_UserAccountTrackNameLen + 1] = {'\0'};	// 文字列データの格納先.
	
	if( NULL == pc_str ){
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}

			
	PLBC_CUtf8 c_utf8( pc_str );
	// UTF-8を1文字ずつ全て処理.
	c_utf8.GetFirst( sc_Utf8One, &us_Utf8OneSize );
	while( 0 != us_Utf8OneSize ){	// 0で文字列確認完了.
		us_sumByte = static_cast<ushort>( us_sumByte + us_Utf8OneSize );
		if( us_sumByte > PNLD_UserAccountTrackNameLen ){
			// UTF-8文字列の合計サイズが文字列データの格納先を超えているのでループを抜ける.
			break;
		} else {
			strncpy( &sc_dstStr[us_sumByte - us_Utf8OneSize], sc_Utf8One, (int)us_Utf8OneSize );
		}

		// 次の文字を取得.
		c_utf8.GetNext( sc_Utf8One, &us_Utf8OneSize );
	}
	// sc_dstStrの中身を文字列に設定する.
	strncpy( pc_str, sc_dstStr, PNLD_UserAccountTrackNameLen );
}


// comment 2015/10/05 OPS1-Sky的場 [特ROM][IT5-12]トップメニュー画面ダウンロードアイコン非表示対応.
//関数------------------------------------------------------------------------------------------------------------------.
//概要    ：ショートカットアイコン表示可否.
//備考    ：.
//戻値    ：True:表示可、False:表示不可.
//作成者  ：.
//レビュー：.
//変更履歴：.
//---------------------------------------------------------------------------------------------------------------------.
Bool PCGC_HomeTopMenu::checkShortCutIconDisplay(uchar uc_index, Bool bl_active)
{
	Bool bl_ret = False;

	// まずはDipSWをチェック.
	if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo(DEFSOFTWARESW223, 0) ) {	// DipSWNo.223 bit0がON.

		// 次に対象のショートカットがアクティブが非アクティブか判定.
		if( True == bl_active ) {
			// 対象のショートカットがアクティブだった場合、無条件に表示可能と判定(既存動作).
			bl_ret = True;
		} else {
			// ショートカットを判定.
			switch(st_BadgeKeyInfo[uc_index].e_ShortCutKind)
			{
			case PNLE_TopMenuShortCut_FuncOAPApp:
				// ショートカットがAPPキーだった場合、アイコン表示不可と判定.
				bl_ret = False;
				break;
			case PNLE_TopMenuShortCut_GrpAppShortCutGrp1:
			case PNLE_TopMenuShortCut_GrpAppShortCutGrp2:
			case PNLE_TopMenuShortCut_GrpAppShortCutGrp3:
			case PNLE_TopMenuShortCut_GrpAppShortCutGrp4:
			case PNLE_TopMenuShortCut_GrpAppShortCutGrp5:
			case PNLE_TopMenuShortCut_GrpAppShortCutGrp6:
				// ショートカットがGruopキーだった場合、アイコン表示不可と判定.
				bl_ret = False;
				break;
			case PNLE_TopMenuShortCut_AppShortCutOap:
			case PNLE_TopMenuShortCut_AppShortCutHanyo:
				// ショートカットがEXTアプリだった場合、アイコン表示不可と判定.
				bl_ret = False;
				break;
			default:
				// 上記以外の場合、表示可能と判定.
				bl_ret = True;
				break;
			}
		}
	
	} else {
		// DipSWが無効だった場合、無条件に表示可能と判定(既存動作).
		bl_ret = True;
	}

	return bl_ret;
}
// comment 2016/06/07 HCL_Sairam [ZeusSBk][CQ530246]procStop function added.
void PCGC_HomeTopMenu::procStop( uchar option )
{
// comment 2016/09/16 MSE岡田 [IT5_3.3][1005694] 省電力復帰のHDDチェックスキップ時は受付け禁止とする.
	if( True == PNLC_SnapshotControlLib::newInstance()->getSkipCheckHDD() ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	if ( NULL == dynamic_cast<PBaseGamenCom*>(PNLC_PopupListControl::newInstance()->getBaseGamen())) {
		PMessageArea::procStop( option );
	} else {
// comment 2018/10/30 SCD2-4G水野 [IT-6][AR1066109] 認証画面からホーム遷移後にジョブをストップできない.
//		PPopupAnimationCom::procStop( option );
		PLayerControl::procStop( option );
	}
}

// comment 2016/09/16 MSE岡田 [IT5_3.3][1005694] 省電力復帰のHDDチェックスキップ時、スライドメニューと表示ズームは受付け禁止にする.
//================================================================================================.
/**
* @name   : PCGC_HomeTopMenu::procSlideMenu.
* @brief  : A072002000_トップメニュー(統合メニュー).
* @note   : スライドメニュー処理.
* @param  : [in] uchar : option.
* @return : void.
*/
//================================================================================================.
void PCGC_HomeTopMenu::procSlideMenu( uchar option )
{
	if( True == PNLC_SnapshotControlLib::newInstance()->getSkipCheckHDD() ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	PPopupAnimationCom::procSlideMenu( option );
}

//================================================================================================.
/**
* @name   : PCGC_HomeTopMenu::procZoomIn.
* @brief  : A072002000_トップメニュー(統合メニュー).
* @note   : 表示ズーム処理.
* @return : void.
*/
//================================================================================================.
void PCGC_HomeTopMenu::procZoomIn()
{
	if( True == PNLC_SnapshotControlLib::newInstance()->getSkipCheckHDD() ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	PPopupAnimationCom::procZoomIn();
}

// comment 2016/10/06 MSE岡田 [IT5_3.3][1005694] プログラムキーの宛先検索要求フラグ設定関数とIGCWaitリクエスト関数を追加.
//================================================================================================.
/**
* @name   : PCGC_HomeTopMenu::setRequestProgramKeyAddressSearch.
* @brief  : A072002000_トップメニュー(統合メニュー).
* @note   : プログラムキーの宛先検索要求フラグ設定.
* @param  : [in] Bool : bl_RequestProgramKeyAddressSearch.
* @return : void.
*/
//================================================================================================.
void PCGC_HomeTopMenu::setRequestProgramKeyAddressSearch( Bool bl_RequestProgramKeyAddressSearch )
{
	m_bl_RequestProgramKeyAddressSearch = bl_RequestProgramKeyAddressSearch;
}

//================================================================================================.
/**
* @name   : PCGC_HomeTopMenu::procIGCWaitRequest.
* @brief  : A072002000_トップメニュー(統合メニュー).
* @note   : IGCWaitリクエスト処理.
* @param  : [in] int32_t : sl_option.
* @return : void.
*/
//================================================================================================.
void PCGC_HomeTopMenu::procIGCWaitRequest( int32_t sl_option )
{
	short ss_ScanFaxProgramNo = pc_ProgramKeyDataGetForTopMenu.getProgramNoForTopMenu( 0 );
	pc_ProgramKeyDataGetForTopMenu.clearProgramGetNum();
	if( 0 < ss_ScanFaxProgramNo ) {
		pc_ProgramKeyDataGetForTopMenu.requestTopMenuProgramKey( ss_ScanFaxProgramNo );
		pc_ManageIGCWaitDisplay.initManageIGCWaitDisplay( PCGD_IGCDisplayStandard, PCGE_IGCClearPopupDispFlgAdd, this );
		nextWaitDisplay_popup();
	}
}
// comment 2017/04/27 HCLT-AppalaRaju [IT5_4.1][1016030] if top menu is opened, call power save.
/**
*	@brief	Request send to powersave mode  when top menu top of sleep mode screen/utility/service/accesibility.
*	@param	char .
*	@retval	 None.
*/
void PCGC_HomeTopMenu::sleepMode(uchar option)
{
	PMessageArea::procPowerSave(option);
}

//================================================================================================.
// 概要 : PCGC_HomeTopMenu : 画面遷移処理（LightUI用）.
// comment : WebUI2 画面切替え制御 20180302 Chikuan.
//================================================================================================.
// comment 2018/04/05 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//void PCGC_HomeTopMenu::procGoToPageLightUI( uchar option )
void PCGC_HomeTopMenu::procGoToPageLightUI( PNLE_TopMenuShortCut option )
{

	uchar _uc_index = static_cast<uchar>( ( uc_nowPage - 1 )*m_uc_TopMenuListKeyDisp + option );
// comment 2019/02/14 【HCL-Naveena】【IT6_EagleH】【OP_BTS-2893 】【KLOC ID:150563】PRECISION.LOSS.
//	_uc_index = option;
	_uc_index = static_cast<uchar>(option);
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//		PNLE_TopMenuShortCut _e_TopMenuShortCut = st_BadgeKeyInfo[_uc_index].e_ShortCutKind;
		PNLE_TopMenuShortCut _e_TopMenuShortCut = option;
	PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();
//2018/12/18 [HCLT_Prashannaa] [AR 1067960] Reverting returning to Home screen changes.
//2018/12/10 [HCLT_Prashannaa] [AR 1067960] returning to Home screen changes.
//	PNLE_UIMode e_NowUIMode_bak = PNLC_WebUIControl::getInstance()->getUIMode();

	if( PnlComGetUserAccountOn() && NULL != PnlComGetUser() && TYPE_UserTypeAdmin == PnlComGetUser()->getUserType()
	    && False == c_SystemControl.getAdminModeOn() ) {
		//管理者ログイン中の場合.
// comment 2020/03/16 SCD2-Sky石原[IT6_2.0MR2][制御No.31]新CopyScanFAXアプリのカスタマイズ機能.
//		if( PNLE_TopMenuShortCut_Utility != ( _e_TopMenuShortCut & 0xFF00 ) ) {
		if(  PNLE_TopMenuShortCut_Utility != ( _e_TopMenuShortCut & 0xFF00 ) 
		      && ( _e_TopMenuShortCut != PNLE_TopMenuShortCut_FuncCopyCustomize )
		      && ( _e_TopMenuShortCut !=PNLE_TopMenuShortCut_FuncCardUICopyCustomize )
		      && ( _e_TopMenuShortCut != PNLE_TopMenuShortCut_FuncFaxCustomize )
		      && ( _e_TopMenuShortCut != PNLE_TopMenuShortCut_FuncIFaxCustomize )
		      && ( _e_TopMenuShortCut != PNLE_TopMenuShortCut_FuncIPAddressFaxCustomize )
		      && ( _e_TopMenuShortCut != PNLE_TopMenuShortCut_FuncIPFaxT38Customize )
		      && ( _e_TopMenuShortCut != PNLE_TopMenuShortCut_ScanToEmailCustomize )
		      && ( _e_TopMenuShortCut != PNLE_TopMenuShortCut_ScanToPCCustomize )
		      && ( _e_TopMenuShortCut != PNLE_TopMenuShortCut_ScanToBoxCustomize ) ) {
			setBuzzerType( PCGE_BuzzerType_InputNg );
			return;
		}
	}
// comment 2018/06/23 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
// comment 2018/11/05 SCD2-Sky中野 [EagleH] WebUIv3FUM対応.
// 不要コードであるため、削除.
//	std::string c_IwsAppName, s_args;
//	int p_processId = 0;
//	short ps_appId = -1;
//	ushort us_cntIws = 1;
//	char* pc_args;
//	const char* IwsAppName;
	switch( _e_TopMenuShortCut ) {
		case PNLE_TopMenuShortCut_FuncBrowser:	procBrowser( 0 );										break;	// 機能:ブラウザ.
// comment 2019/01/10 SCD2-Sky中野 [EagleH] 制御No.38_目的別UI対応.
//		case PNLE_TopMenuShortCut_FuncCopy:		procCopy( 0 );											break;	// 機能:コピー.
		case PNLE_TopMenuShortCut_FuncCopy:		procCopyFuncLightUI( PNLE_WebUIFunctionCopy );			break;	// 機能:コピー.
		case PNLE_TopMenuShortCut_CopyCardType:	procCopyFuncLightUI( PNLE_WebUIFunctionCopyCardType );	break;	// 機能:Copy(カード型).		// comment 2019/02/22 SCD2-Sky中野 [EagleH] RQ-802_カードタイプUI.
		case PNLE_TopMenuShortCut_FuncOAPApp:	procAPPsMenu( 0 );										break;	// 機能:OpenAPIアプリ.
		case PNLE_TopMenuShortCut_ScanToEmail:	procEachScanLightUI(PNLE_WebUIFunctionScanToEmail);		break;	// ScanToEmail.
		case PNLE_TopMenuShortCut_ScanToBox:	procEachScanLightUI(PNLE_WebUIFunctionScanToBox);		break;	// ScanToBox.
		case PNLE_TopMenuShortCut_ScanToPC:		procEachScanLightUI(PNLE_WebUIFunctionScanToPC);		break;	// ScanToPC.
// comment 2019/01/10 SCD2-Sky肥後 [EagleH] 制御No.38_目的別UI対応.
		case PNLE_TopMenuShortCut_ScanToMe:		procEachScanLightUI(PNLE_WebUIFunctionScanToMe);		break;	// ScanToMe.
		case PNLE_TopMenuShortCut_InternetFax:	procEachScanLightUI(PNLE_WebUIFunctionIFax);			break;	// InternetFax.
		case PNLE_TopMenuShortCut_IPAddressFax:	procEachScanLightUI(PNLE_WebUIFunctionIPAddressFax);	break;	// IPAddressFax.
// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
//		case PNLE_TopMenuShortCut_FuncFaxOnly:	procFaxOnly( 0 );										break;	// FaxLightUI.
// comment 2020/03/09 SCD2-Sky石原[IT6_2.0MR2][制御No.31]新CopyScanFAXアプリのカスタマイズ機能.
//		case PNLE_TopMenuShortCut_FuncFaxOnly:	procFaxLightUI( 0 );									break;	// FaxLightUI.
		case PNLE_TopMenuShortCut_FuncFaxOnly:	procFaxLightUI( PNLE_WebUIFunctionFax );									break;	// FaxLightUI.
		case PNLE_TopMenuShortCut_FuncCopyCustomize:		procCopyFuncLightUI(PNLE_WebUIFunctionCopyCustomize);		break;			// CopyUserCustomize.
		case PNLE_TopMenuShortCut_FuncCardUICopyCustomize:	procCopyFuncLightUI(PNLE_WebUIFunctionCardUICopyCustomize);	break;	//CardUICopyUserCustomize.
		case PNLE_TopMenuShortCut_FuncFaxCustomize:		procFaxLightUI(PNLE_WebUIFunctionFaxCustomize);			break;	// FaxCustomize.
		case PNLE_TopMenuShortCut_FuncIFaxCustomize:		 procEachScanLightUI(PNLE_WebUIFunctionIFaxCustomize);		break;	// InternetFaxCustomize.
		case PNLE_TopMenuShortCut_FuncIPAddressFaxCustomize:	procEachScanLightUI(PNLE_WebUIFunctionIPAddressFaxCustomize);	break;	//IPAddressFax Customize.
		case PNLE_TopMenuShortCut_FuncIPFaxT38Customize:		procEachScanLightUI(PNLE_WebUIFunctionIPFaxT38Customize);		break;	//IPFaxT38 UserCustomize.
		case PNLE_TopMenuShortCut_ScanToEmailCustomize:	procEachScanLightUI(PNLE_WebUIFunctionScanToEmailCustomize);		break;	// ScanToEmail UserCustomize.
		case PNLE_TopMenuShortCut_ScanToPCCustomize:		procEachScanLightUI(PNLE_WebUIFunctionScanToPCCustomize);		break;	//ScanToPC UserCustomize.
		case PNLE_TopMenuShortCut_ScanToBoxCustomize:	procEachScanLightUI(PNLE_WebUIFunctionScanToBoxCustomize);		break;	//ScanToBox UserCustomize.
// comment 2020/03/10 SCD2-Sky中野 [IT6_2.0MR2] 制御No.30_Program登録機能.
		case PNLE_TopMenuShortCut_CopyMemoryWebUICopy:			procCopyFuncLightUI( PNLE_WebUIFunctionCopy );			break;	// コピープログラム(WebUIコピー).
		case PNLE_TopMenuShortCut_CopyMemoryCopyCardType:		procCopyFuncLightUI( PNLE_WebUIFunctionCopyCardType );	break;	// コピープログラム(シンプルコピー).
		case PNLE_TopMenuShortCut_ScanFaxMemoryWebUIFax:		procFaxLightUI( PNLE_WebUIFunctionFax );				break;	// スキャン/ファクスプログラム(WebUIファクス).
		case PNLE_TopMenuShortCut_ScanFaxMemoryIFax:			procEachScanLightUI( PNLE_WebUIFunctionIFax );			break;	// スキャン/ファクスプログラム(インターネットファクス).
		case PNLE_TopMenuShortCut_ScanFaxMemoryIPAddressFax:	procEachScanLightUI( PNLE_WebUIFunctionIPAddressFax );	break;	// スキャン/ファクスプログラム(IPアドレスファクス).
		case PNLE_TopMenuShortCut_ScanFaxMemoryIpFaxT38:		procEachScanLightUI( PNLE_WebUIFunctionIpFaxT38 );		break;	// スキャン/ファクスプログラム(IP-FAX(T.38)).
		case PNLE_TopMenuShortCut_ScanFaxMemoryScanToPC:		procEachScanLightUI( PNLE_WebUIFunctionScanToPC );		break;	// スキャン/ファクスプログラム(ScanToPC).
		case PNLE_TopMenuShortCut_ScanFaxMemoryScanToMe:		procEachScanLightUI( PNLE_WebUIFunctionScanToMe );		break;	// スキャン/ファクスプログラム(ScanToMe).
		case PNLE_TopMenuShortCut_ScanFaxMemoryScanToEmail:		procEachScanLightUI( PNLE_WebUIFunctionScanToEmail );	break;	// スキャン/ファクスプログラム(ScanToEmail).
		case PNLE_TopMenuShortCut_ScanFaxMemoryScanToBox:		procEachScanLightUI( PNLE_WebUIFunctionScanToBox );		break;	// スキャン/ファクスプログラム(ScanToBox).
				
// comment 2018/08/23 Sky雜賀【Sparrow】[1057532].
		case PNLE_TopMenuShortCut_FuncUbqPrint:	procUbqPrint( 0 );					break;	// 機能:ユビキタスプリント.
// comment 2018/11/09 Sky雜賀 [EagleH][RQ-272].
		case PNLE_TopMenuShortCut_IpFaxT38:		procEachScanLightUI( PNLE_WebUIFunctionIpFaxT38 );		break;	// 機能:IP-FAX(T.38).
		case PNLE_TopMenuShortCut_CopyPaperType:																// 用紙.
// comment 2019/04/17 SCD2-Sky肥後 [EagleH][OP_BTS-3461] 用紙画面のWebUI化対応.
//			// comment 2018/06/09 Sky弥村 [IT6][制御構成変更No.252] 新UIデザイン_用紙設定画面.
//			PNLC_PopupListControl::newInstance()->addPopup( &gp_7_0008, NULL, NULL, 0, 0, 0, 0, True );	// P7-0008_新UI用紙設定.
// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
			// 遷移先がWebUI画面の場合は、UIモードをLightUIモードに変更.
			PNLC_WebUIControl::getInstance()->setUIMode( PNLE_UIModeLightUI );
			PNLC_WebUIControl::getInstance()->reserveChangeScreen( PNLE_WebUIRequestMessagePaperSetting );
			PNLC_WebUIControl::getInstance()->requestChangeScreen();
			break;
// comment 2020/03/10 SCD2-Sky石原[HeliosMLK][OP_BTS-21285].
		case PNLE_TopMenuShortCut_DetectPaperSize:																// 手差しトレイ用紙検知.
			PNLC_WebUIControl::getInstance()->setUIMode( PNLE_UIModeLightUI );
			PNLC_WebUIControl::getInstance()->reserveChangeScreen( PNLE_WebUIRequestMessageDetectPaperSize );
			PNLC_WebUIControl::getInstance()->requestChangeScreen();
			break;
// comment 2018/06/23 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
		case PNLE_TopMenuShortCut_AppShortCutOap:
// Comment 2019/12/31 [IT6_EagleBK] OP_BTS-18202 [HCLT-Santhiya]  Market Place key Display on ClassicStyle Screen. 
			procAPPOap( _uc_index, True );	// OpenAPIアプリショートカット:OAPアプリ.
			break;
// comment 2019/05/17 [HCLT-Prem] [IT6_MR1.0][AR1077206]Market place application case for Basic style screen.
		case PNLE_TopMenuShortCut_MarketPlaceApplication:	// bizhub Market Place.
// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
//			procAPPOap( _uc_index );	// OpenAPIアプリショートカット:OAPアプリ.
// Comment 2019/12/31 [IT6_EagleBK] OP_BTS-18202 [HCLT-Santhiya]  Market Place key Display on ClassicStyle Screen. 
			{
				PNLC_ActionDataLib* pc_ActionLib = PNLC_ActionDataLib::newInstance();
				if (NULL != pc_ActionLib ){
					pc_ActionLib->setMarketPlacelaunch( True );
				}
				procAPPOap( _uc_index, True );	// OpenAPIアプリショートカット:OAPアプリ.
// Comment 2020/07/17 HCL-Satheesh [IT6_2.0MR3SpecialROM][OP_BTS-25202] Restirct the Martket Place App while Market Place OFF in Admin and Utility navigated from Market place App.
				onlyPanel.setMarketPlaceAppStart( True );
				if (NULL != pc_ActionLib ){
					pc_ActionLib->setMarketPlacelaunch( False );
				}
				break;
			}
// comment 2018/06/23 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
		case PNLE_TopMenuShortCut_AppShortCutHanyo:
// comment 2018/11/05 SCD2-Sky中野 [EagleH] WebUIv3FUM対応.
// 不要コードであるため、削除.
//			for( us_cntIws=0; us_cntIws < s_registerAppIws.size(); us_cntIws++ ) {
//				s_registerAppIws[us_cntIws].getAppName( &c_IwsAppName );
//				IwsAppName = c_IwsAppName.c_str();
//				if( 0 == strcmp( st_BadgeKeyInfo[ _uc_index ].sc_FuncName, IwsAppName ) ) {
//					s_registerAppIws[us_cntIws].getAppId( &ps_appId );
//					s_registerAppIws[us_cntIws].getAppArgs( &s_args );
//					pc_args = ( char* )( s_args.c_str() );
//					LAPC_APP_ActiveAppManager APP_ActiveAppManager( LAPC_BFW_System::newInstance() );
//					APP_ActiveAppManager.requestLaunchAppId( ps_appId, LAPE_APP_ActiveFlgON, pc_args, &p_processId );
//					gp_CostCenterResetDummy.setAppType( PNLD_ScanOapAppSelectExt_SelectAppIws,  (uchar)ps_appId  );
//					break;
//				}
//			}
// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
//			procAPPHanyo( _uc_index );
			procAPPHanyo( _uc_index, True );
			break;	// OpenAPIアプリショートカット:汎用アプリ.

// comment 2018/06/23 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
		case PNLE_TopMenuShortCut_AppListShortCut:	// OpenAPIアプリリスト.
			procAPPsMenu( 0 );
			break;

// comment 2018/06/23 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
		case PNLE_TopMenuShortCut_GrpAppShortCutGrp1:					// グループアプリケーション グループ１.
		case PNLE_TopMenuShortCut_GrpAppShortCutGrp2:					// グループアプリケーション グループ２.
		case PNLE_TopMenuShortCut_GrpAppShortCutGrp3:					// グループアプリケーション グループ３.
		case PNLE_TopMenuShortCut_GrpAppShortCutGrp4:					// グループアプリケーション グループ４.
		case PNLE_TopMenuShortCut_GrpAppShortCutGrp5:					// グループアプリケーション グループ５.
		case PNLE_TopMenuShortCut_GrpAppShortCutGrp6:					// グループアプリケーション グループ６.
			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
				procAPPGrp( _e_TopMenuShortCut );
			}
			break;
// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_】 Card copy setting support for DenebMLK model.
// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No._77-1】 Non Image Area Erase setting support for DenebMLK model.
		case PNLE_TopMenuShortCut_CopyNonImageAreaErase:																// 用紙.
			PNLC_WebUIControl::getInstance()->setUIMode(PNLE_UIModeLegacyUI);
			procCopy( 0 );
			if( True == combinationCheck( CNK_SkyShot, PnlComGetJobCopy() ) ) {
				PnlComGetJobCopy()->getJobSpec()->pc_Scan->setSkyShotOn( True );
				if( True == isOn( CNK_BookRepeat, PnlComGetJobCopy() ) ) {							// ブック原稿がONの場合.
					PnlComGetJobCopy()->getJobSpec()->pc_Scan->c_OriginalSize.setAutoSize( True );	// ブック原稿＋原稿外消去の場合、原稿サイズ設定を自動にする.
					selectAutoPaperMag();		// APS/AMS処理関数.
				}
			} else {
				gp_CombinationErrorConfirm_warn.setSettingFuncKind( PnlComGetJobCopy() );
			}
			break;
		case PNLE_TopMenuShortCut_CopyCardCopy:																// 用紙.
// comment 2019/01/10 SCD2-Sky中野 [EagleH] 制御No.38_目的別UI対応.
#ifdef DEF_DenebMLKCus
//  comment 2019/8/8 K.Sato [EagleZ+][制御構成変更No.36] IDコピーの画面遷移先を変更.
//// Comment 2019/07/22 【HCLT-Taqiuddin】 【IT6_EagleZ+】 【制御構成変更_No.40】Performing Function Version check and deciding the screen transition for DenebMLK.
//		if ( True == PLBC_FuncVersionCheck::isOK_IT6_Type1()) {
//			procCopyFuncLightUI( PNLE_WebUIFunctionIDCardCopy );
//		} else {
			if ( False == PLBC_FuncVersionCheck::isOK_IT6_Type1() ) {
				PNLC_WebUIControl::getInstance()->setUIMode(PNLE_UIModeLegacyUI);
// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
//			Cmp_CopyQuickButton.setJobMemoryToJobSpec( PNLD_CopyProgram_CardCopy );
//		onlyPanel.setProgramCallStatus( PNLE_ProgramCallExec_Continue );
// comment 2018/10/23 [HCLT_Prem][DenebMLK] [AR1050427] Card Copy Support.
//			PNLC_PopupListControl::newInstance()->addReservePopup( &gp_A0201B3000, NULL, NULL, 0, 0, 0, 0 );
//			procCopy( 0 );
//			if( NULL != pc_PopupListControl ) {		// MarketPlace download画面をポップアップ.
//				short ss_XPos, ss_YPos, ss_XSize, ss_YSize = 0;
//				uchar uc_IconIndex = option % PNLD_TopMenuKeyDisp2;
//				Gra_HomeTopMenuBadgeIcon[uc_IconIndex].getPosition( ss_XPos, ss_YPos );
//				Gra_HomeTopMenuBadgeIcon[uc_IconIndex].getSize( ss_XSize, ss_YSize );
//				pc_PopupListControl->addReservePopup( &gp_A0201B3000, NULL, NULL, 360,216,80,48 );
//			}
				procCopy( 0 );
				if( NULL != nextGamen ) {
//Comment 2018/12/06 HCLT_Rajkumar [IT6_DenebMLK][AR1068150] To set Card copy mode: ON by default.
					Cmp_CopyQuickButton.setJobMemoryToJobSpec( PNLD_CopyProgram_CardCopy );
					onlyPanel.setProgramCallStatus( PNLE_ProgramCallExec_Continue );
					PBaseGamenCom* pc_BaseGamen = dynamic_cast<PBaseGamenCom*>( nextGamen );
					PPopupCom* pc_PopupCom = &gp_A0201B3000;
					if( ( NULL != pc_BaseGamen ) && ( NULL != pc_PopupCom ) ) {
						pc_BaseGamen->addOpenPopup( pc_PopupCom, True );
						OpenPopupIndi(); 
					}
				}
			} else {
				procCopyFuncLightUI( PNLE_WebUIFunctionIDCardCopy );
			}
// comment 2019/01/10 SCD2-Sky中野 [EagleH] 制御No.38_目的別UI対応.
//  comment 2019/8/8 K.Sato [EagleZ+][制御構成変更No.36] IDコピーの画面遷移先を変更.
//// Comment 2019/07/22 【HCLT-Taqiuddin】 【IT6_EagleZ+】 【制御構成変更_No.40】Performing Function Version check and deciding the screen transition for DenebMLK.		
//		}
#else
			procCopyFuncLightUI( PNLE_WebUIFunctionIDCardCopy );
#endif
		break;
		case PNLE_TopMenuShortCut_MarketPlace:	// bizhub Market Place.
			if ( PNLE_IWS_Status_Bootup != PnlComGetIwsStatus() ) {
				setBuzzerType( PCGE_BuzzerType_InputNg );
				return;
			}
			if( NULL != pc_PopupListControl ) {		// MarketPlace download画面をポップアップ.
				short ss_XPos, ss_YPos, ss_XSize, ss_YSize = 0;
				uchar uc_IconIndex = option % m_uc_TopMenuListKeyDisp;
				Gra_HomeTopMenuBadgeIcon[uc_IconIndex].getPosition( ss_XPos, ss_YPos );
				Gra_HomeTopMenuBadgeIcon[uc_IconIndex].getSize( ss_XSize, ss_YSize );
				if( 0x00 == getSwDipSwitch( DEFSOFTWARESW172 ) ) {				// ダウンロード開始確認画面.
					pc_PopupListControl->addPopup( &gp_MarketPlaceDownloadStartConfirm_act, this, NULL, ss_XPos, ss_YPos, ss_XSize, ss_YSize );
				} else if( 0x02 == getSwDipSwitch( DEFSOFTWARESW172 ) ) {		// 接続先URL、Port入力用画面.
					pc_PopupListControl->addPopup( &gp_MarketPlaceURLInput_act, this, NULL, ss_XPos, ss_YPos, ss_XSize, ss_YSize );
				} else {														// 非表示またはグレーアウトされるためありえない.
					LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
				}
			}
			break;
// Comment 2018/06/28 [HCL-Pavani] [IT6_Eagle] [AR1048863] Download and install MarketPlace on all MFPs according to the performance in LightUI.
		case PNLE_TopMenuShortCut_MarketPlaceSetup:		// Market Place Setup Key ShortCut type.
// comment 2018/10/12 【IT6_Eagle】【HCLT_Pavani】【AR1063938】 Condition check added for Market Place Application Uninstalled case.
		{
			PNLC_ActionDataLib* pc_ActionLib = PNLC_ActionDataLib::newInstance();
			short ss_InstalledStatus = 0;
			if( NULL != pc_ActionLib ) {
				ss_InstalledStatus = pc_ActionLib->getMarketPlaceDownloadState();
				if( NULL != pc_PopupListControl ) {	
					short ss_XPos, ss_YPos, ss_XSize, ss_YSize = 0;
					uchar uc_IconIndex = option % PNLD_TopMenuKeyDisp;
					Gra_HomeTopMenuBadgeIcon[uc_IconIndex].getPosition( ss_XPos, ss_YPos );
					Gra_HomeTopMenuBadgeIcon[uc_IconIndex].getSize( ss_XSize, ss_YSize );
// comment 2018/09/12 【IT6_Eagle】【HCLT_Pavani】【AR1053803】Support for Market Place Application Failure case.
//				pc_PopupListControl->addPopup( &gp_MarketPlaceSetup_PasswordInput_act, this, NULL, ss_XPos, ss_YPos, ss_XSize, ss_YSize );  // Password Input Screen.
// comment 2018/10/12 【IT6_Eagle】【HCLT_Pavani】【AR1063938】 Condition check added for Market Place Application Uninstalled case.
//					if ( False == PnlComGetIWSSetting()->getEnable() ) {
					if ( ( False == PnlComGetIWSSetting()->getEnable() ) || 
						( PCGC_MarketPlaceDownloadStateComplete == ss_InstalledStatus ) ) {
						pc_PopupListControl->addPopup( &gp_MarketPlaceSetup_PasswordInput_act, this, NULL, ss_XPos, ss_YPos, ss_XSize, ss_YSize );  // Password Input Screen.
					} else {
						pc_PopupListControl->addPopup( &gp_MarketPlaceDownloadFailure_act, this, NULL, ss_XPos, ss_YPos, ss_XSize, ss_YSize );
					}
				} else {
					LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
				}
			}
		}
			break;
// Comment 2018/10/08 [HCL-Pavani] [IT6_Eagle] [AR1060058] MarketPlace Application Not Applicable implementation commented.
//		case PNLE_TopMenuShortCut_MarketPlaceApplication:	// Market Place Application Key Short.
//				// Browser launcher
//				bootMarketPlace();
//			break;
			
		case PNLE_TopMenuShortCut_SysBoxAuthPrint:		// システムボックス機能設定:認証＆プリント.
// comment 2019/01/10 SCD2-Sky中野 [EagleH] 制御No.38_目的別UI対応.
//			// comment : 20180508 Chikuan : [Eagle] [KRQ-314-28] 【新UI】認証＆プリントBox対応.
//			setWebUIBoxFunction( PNLE_WebUIFunctionAuthPrint );
//			procBox( PNLD_CheckGotoUsbWithoutHDD );
//			if( NULL != nextGamen ){
//				PNLC_BoxBoxListControlLib* pc_BoxListControlLib = PNLC_BoxBoxListControlLib::newInstance();
//				if( NULL != pc_BoxListControlLib ){
//					pc_BoxListControlLib->resetBox();
//
//					PNLC_TabletSecurityControlLib* pc_TabletSecurityControl = PNLC_TabletSecurityControlLib::newInstance();
//					if( NULL != pc_TabletSecurityControl ){
//						pc_TabletSecurityControl->securityAreaExited();
//					}
//				}
//			}
//			setWebUIBoxFunction( PNLE_WebUIFunctionSize );
			procBoxFuncLightUI( PNLE_WebUIFunctionAuthPrint );
			break;

		case PNLE_TopMenuShortCut_SysBoxExternalMemory:	// 外部メモリー.
// comment 2018/05/12 Sky東島 [IT6][KRQ-314-29] 目的別UI.
		{
// comment 2019/01/10 SCD2-Sky中野 [EagleH] 制御No.38_目的別UI対応.
			if( ( NULL != PnlComgetFuncVer() )
// comment 2021/01/14 Sky中野 [IT6_2.2] 制御No.36_目的別UI対応(小サイズスキャン).
//				&& ( TYPE_FuncVer_Expansion1 <= PnlComgetFuncVer()->getFuncVer2() ) ) {
				&& ( True == PLBC_FuncVersionCheck::isOK_IT6_Type1() ) ) {
				procBoxFuncLightUI( PNLE_WebUIFunctionUSBToPrint );
			} else {
				APIC_UsbMemSetting* pc_UsbMemSetting = APIC_UsbMemSetting::newInstance();
				APIC_UsbMemDeviceInfo* pc_UsbMemDeviceInfo = APIC_UsbMemDeviceInfo::newInstance();

				if( NULL == pc_UsbMemSetting
				 || NULL == pc_UsbMemDeviceInfo ){
					return;
				}

				// 外部メモリ複数検知時は遷移させない.
				if( pc_UsbMemDeviceInfo->isWarning( TYPE_UsbMemWarningTwoInsert ) ){
					setBuzzerType( PCGE_BuzzerType_InputNg );
					return;
				}
				// 管理者設定>環境設定>ボックス設定>外部メモリー機能設定 外部メモリ文書印刷設定.
				TYPE_NetworkSpecific e_NetworkSpecific  = PNLC_NetworkLan::getInstance()->getLine();
				if( False == pc_UsbMemSetting->getEnableUsbDirectPrint( e_NetworkSpecific ) ) {
					setBuzzerType( PCGE_BuzzerType_InputNg );
					return;
				}
				// 機能制限で印刷ができないユーザーの場合は遷移させない.
				if( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Print ) ) {
					setBuzzerType( PCGE_BuzzerType_InputNg );
					return;
				}

				Bool bl_AuthOapFlg = False;
				Bool bl_AuthLDAPICCardFlg = False;
				if( True == PnlComGetUserAccountOn() ) {
					if( ( False == CheckBoxAdmin() )  && ( False == PnlComGetUserAccountPublic() ) ) { // ボックス管理者でもパブリックでもない.
						switch( PnlComGetLoginUserAuthenticationType() ) {
							case TYPE_UserAuthenticationIntermeddiateServer:	// 中間サーバーでの認証.
								{
									if (False == onlyPanel.getUseAuthDevice()) {	// パネル認証時は再認証不要.
										break;
									}
									// ユーザデータの取得.
									APIC_User* pc_User = PnlComGetUser();
									if( NULL == pc_User ) {
										PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "BOXC_CellularPhoneMenu::procGoToPage", UC_PNL_ERROR_PARAMETER );
										return;
									}
									// 再認証パラメータが存在する場合は認証画面に遷移しない.
									if( pc_User->isExistSecondAuthPara() == True )
									{
										break;
									}
									// 認証デバイス（生体orカード）の設定状況取得.
									TYPE_AUTH_DEVICE_TYPE e_AuthDeviceType = PnlComGetAuthDeviceSetting()->getAuthDeviceType();
									// 認証カード装置、生体認証の場合.
									if( TYPE_AUTH_DEVICE_ICC == e_AuthDeviceType || TYPE_AuthDeviceType_LoadableIcc == e_AuthDeviceType
									    || TYPE_AuthDeviceType_Bio == e_AuthDeviceType || TYPE_AuthDeviceType_Bio2 == e_AuthDeviceType ) {	// 生体２認証に対応.
										bl_AuthOapFlg = True;
									}
									// 認証ブラウザの場合、またはネットワークログインの場合 .
									if( ( TYPE_ExtTypeBrowserAuth == PnlComGetOapAuthenticationAplType() ) || ( True == onlyPanel.getUseAuthDevice() ) ) {
										bl_AuthOapFlg = True;
									}
								}
								break;
							default:
								break;
						}
					}
				}
				if( True == bl_AuthOapFlg ){
					PNLS_BoxLoginExtParam s_BoxLoginParamTmp;
					s_BoxLoginParamTmp.e_LoginExtType = PNLE_BoxLoginExtTypeWebUIUSBPrint;
					s_BoxLoginParamTmp.pc_BackGamen = this;
					gp_AuthOapBoxLoginExt_box.setBoxLoginParam( &s_BoxLoginParamTmp );
					gp_AuthOapBoxLoginExt_box.setAuthOapBoxValueInit();
					PNLC_PopupListControl* pc_popupListControl = PNLC_PopupListControl::newInstance();
// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
//// comment 2019/09/17 SCD2-Sky中野 [EagleH][OP_BTS-13559].
//					if( &gp_AppChangeUIBrowser == nowGamen ) {
					if( ( &gp_AppChangeBrowser == getNowGamen() )
					 || ( &gp_AppChangeUIBrowser == getNowGamen() )
					 || ( &gp_PullPrintOapBrowserDummy == getNowGamen() )
					 || ( &gp_CostCenterResetDummy == getNowGamen() )
					 || ( &gp_ScanOapBrowserDummy == getNowGamen() )
					 || ( &gp_GeneralOapBrowserDummy == getNowGamen() )
					 || ( &gp_WarningPopupDummyBase == getNowGamen() )
					 || ( &gp_AppChangeScreenSaver== getNowGamen() )
					 || ( &gp_AuthOapBrowserLogin == getNowGamen() ) ) {
						gp_PublicPopupDummyBase.setupOpenPopup( &gp_AuthOapBoxLoginExt_box );
						gp_PublicPopupDummyBase.setupNextGamen( nowGamen );
						gp_PublicPopupDummyBase.setMode( PNLE_PublicPopupDummyModeAct );
						nextGamen = &gp_PublicPopupDummyBase;
						OpenPopupIndi();
					} else {
						if( NULL != pc_popupListControl ) {
// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
//							pc_popupListControl->addPopup( &gp_AuthOapBoxLoginExt_box, this, NULL, 0, 0, 0, 0, True );
							PBasicMessageProc* pc_nowGamen = NULL;
							if( True == pc_PopupListControl->isPopupOpen() ) {
								pc_nowGamen= pc_PopupListControl->getNowPopup();
							} else {
								pc_nowGamen= getNowGamen();
							}
							pc_popupListControl->addPopup( &gp_AuthOapBoxLoginExt_box, pc_nowGamen, NULL, 0, 0, 0, 0, True );
						}
					}
				} else if( True == bl_AuthLDAPICCardFlg ){
					PNLS_AuthInfoForIccLdapParam s_AuthInfoParamTmp;
					s_AuthInfoParamTmp.e_LoginExtType = PNLE_BoxLoginExtTypeWebUIUSBPrint;
					gp_AuthenticationInfoForIccLdapPopup_act.setAuthInfoParam( &s_AuthInfoParamTmp );
			
					PNLC_PopupListControl* pc_popupListControl = PNLC_PopupListControl::newInstance();
					if( NULL != pc_popupListControl ) {
						pc_popupListControl->addPopup( &gp_AuthenticationInfoForIccLdapPopup_act, this, NULL, 0, 0, 0, 0, True );
					}
				} else if( 0 == pc_UsbMemDeviceInfo->getStatus() ){
					// 接続数が0の場合はUSBトップ画面へ遷移.
					nextGamen = &gp_WebUIUSBTop_box;
				} else {
					// USB一覧画面へ遷移.
					nextGamen = &gp_WebUIUSBFileList_box;
				}
			}
		}
			break;

		case PNLE_TopMenuShortCut_AddressBook:			// アドレス帳.
			// comment 2018/08/03 SCD2-Sky竹菴 [Eagle][1048612] WebUIからのアドレス帳遷移時は閉じるキー押下での不正遷移を防止する.
			onlyPanel.setBrowserMode( False );
			onlyPanel.setTopmenuMode( True );
			procAddressBook( 0 );
			break;

		case PNLE_TopMenuShortCut_UIModeLegacy:			// レガシーへの切り替え.
			procUIMode( 0 );
			break;	
// Comment 2019/10/09 【HCLT-Krishna】【IT6_HeliosMLK】【制御構成変更_No.43】 SSDレス時の機密文書プリント対応
//// Comment 2018/11/19 【HCLT-Prashannaa】【IT6_DenebMLK】【制御構成変更_No.83】
//#ifdef DEF_DenebMLKCus
#if defined  (DEF_DenebMLKCus) || defined ( DEF_HeliosMLKCus )
			case PNLE_TopMenuShortCut_SysBoxSecurity:
			{
// comment 2022/06/30 Sky東島 [IT6_2.3][制御No.39-42_目的別UIプリセット化].
				if( &gp_AppChangeUIBrowser == nowGamen ){
					procBoxFuncLightUI(PNLE_WebUIFunctionSecurityDocument);
				} else {
					PNLE_ShortCutID _e_ShortCutID = PNLE_ShortCutID_NON;
					PNLC_ChangeGamenManager* pc_ChangeGamenManager = PNLC_ChangeGamenManager::newInstance();
					if( ( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DirectInputOffLdapOnNUM, 7 ) )
					    && ( True == PnlComGetUserAccountOn() ) ) {
						// システムボックス機能設定:セキュリティー文書(文書選択).
						_e_ShortCutID = PNLE_ShortCutID_BOX_DocSecretDataBeforePassPrint_Direct;
					} else {
						// システムボックス機能設定:セキュリティー文書(ID入力画面).
						_e_ShortCutID = PNLE_ShortCutID_BOX_DocSecretDataBeforePassPrint;
					}
					procBox( PNLD_CheckGotoUsbWithoutHDD );
					if( NULL != nextGamen ) {
						if( NULL != pc_ChangeGamenManager ) {
							PNLC_WebUIControl::getInstance()->setUIMode(PNLE_UIModeLegacyUI);
							pc_ChangeGamenManager->setShortCut( _e_ShortCutID, PNLE_ShortCutKindID_Box );
							// トップメニューから直接システムボックスへ遷移する際は、セキュリティボックスから離脱したと判断する.
							PNLC_TabletSecurityControlLib* pc_TabletSecurityControl = PNLC_TabletSecurityControlLib::newInstance();
							if( NULL != pc_TabletSecurityControl ) {
								pc_TabletSecurityControl->securityAreaExited();
							}
						}
					}
				}
			}
			break;
// comment 2022/06/30 Sky東島 [IT6_2.3][制御No.39-42_目的別UIプリセット化].
#else	//  defined  (DEF_DenebMLKCus) || defined ( DEF_HeliosMLKCus )
		case PNLE_TopMenuShortCut_SysBoxSecurity:		procBoxFuncLightUI(PNLE_WebUIFunctionSecurityDocument);	break;	// セキュリティー印刷.
#endif
// Comment 2019/04/12 SCD2-Sky二口 [EagleH][制御No.70] WebUIのHome画面見直し.
		case PNLE_TopMenuShortCut_ClassicCopy:												// Legacyのコピー画面(A020000000)に遷移
			PNLC_WebUIControl::getInstance()->setUIMode( PNLE_UIModeLegacyUI );
			procCopy( 0 );
			break;
		case PNLE_TopMenuShortCut_ClassicScan:												// Legacyのスキャン/ファクス画面(A031000000)に遷移
			PNLC_WebUIControl::getInstance()->setUIMode( PNLE_UIModeLegacyUI );
			procFax( 0 );
			break;
		case PNLE_TopMenuShortCut_ClassicFax:												// Legacyのファクス(のみ)画面(A031000000)に遷移
			PNLC_WebUIControl::getInstance()->setUIMode( PNLE_UIModeLegacyUI );
			procFaxOnly( 0 );
			break;
		case PNLE_TopMenuShortCut_ClassicBox:												// Legacyのボックス画面(A056000000)に遷移
			PNLC_WebUIControl::getInstance()->setUIMode( PNLE_UIModeLegacyUI );
			procBox( 0 );
			break;
// comment 2020/03/10 SCD2-Sky中野 [IT6_2.0MR2] 制御No.30_Program登録機能.
		case PNLE_TopMenuShortCut_CopyMemory:												// プログラム呼び出し：Legacyコピー.
			{
				PNLC_WebUIControl::getInstance()->setUIMode( PNLE_UIModeLegacyUI );
				procCopy( 0 );
				PNLC_ChangeGamenManager* pc_ChangeGamenManager = PNLC_ChangeGamenManager::newInstance();
				if( NULL != nextGamen ) {
					Cmp_CopyQuickButton.setJobMemoryToJobSpec( PNLC_WebUIControl::getInstance()->getSelectProgramNo() );
					// 簡単設定キー押下時とトップメニューからの呼び出し時で、m_uc_ProgramCallExecの値の遷移に差異が生じ.
					// スタートキー押下ですぐにジョブ実行出来なくなるため、簡単設定キーから設定した場合と同じステータスにする必要がある。
					onlyPanel.setProgramCallStatus( PNLE_ProgramCallExec_Continue );
					if( ( 30 == PNLC_WebUIControl::getInstance()->getSelectProgramNo() )
					 && ( True == PnlComGetSoftDipSw()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW153, 0 ) ) ) {
						gp_A0201B3000.setBackupFromUtility();
						pc_ChangeGamenManager->setShortCut( PNLE_ShortCutID_COPY_CardCopy, PNLE_ShortCutKindID_Copy );
					}
				}
			}
			break;
		case PNLE_TopMenuShortCut_ScanFaxMemory:											// プログラム呼び出し：Legacyスキャン.
			{
// comment 2020/09/25 Sky中野 [IT6_2.1][OP_BTS-27503].
				m_ss_ConfirmationProgramNo = 0;
				gp_ProgramBasic_scanfax.m_bl_clearAddress = False;
				TYPC_ProgramKey* pc_ProgramKey = PNLC_HomeProgramKeyData::getInstance()->getProgramKeyData( PNLC_WebUIControl::getInstance()->getSelectProgramNo() );
				if( ( NULL != PnlComGetFaxPanelSetting() )
				 && ( TYPE_FaxAddDestination_On == PnlComGetFaxPanelSetting()->getFaxAddDestination() )				// 同報宛先の連続選択：許可.
				 && ( NULL != PnlComGetJobScan() )
				 && ( NULL != PnlComGetJobScan()->getJobSpec() )
				 && ( NULL != PnlComGetJobScan()->getJobSpec()->pc_Send )
				 && ( 0 < PnlComGetJobScan()->getJobSpec()->pc_Send->c_ComSend.getSelectTotalAddressCount()) ) {	// 宛先選択総数が1以上.
					uint32_t ul_ShorteningID = 0;
					uint32_t ul_GroupID = 0;
					TYPE_SendMode e_SendMode = TYPE_SendModeNotSetting;
					char sc_ProgramKeyName[PCGD_ProgramKeyNameUTF8_73];
					memset( sc_ProgramKeyName, '\0', sizeof( sc_ProgramKeyName ) );
					if( NULL != pc_ProgramKey ) {
						ul_ShorteningID = pc_ProgramKey->getDbRecordNoOfShorteningAddress();			// 短縮宛先番号.
						ul_GroupID = pc_ProgramKey->getDbRecordNoOfGroupAddress();						// グループ宛先番号.
						e_SendMode = pc_ProgramKey->getSendMode();										// 宛先種別.
						pc_ProgramKey->getProgramKeyNameUTF8( sc_ProgramKeyName, TYPE_CharSetUTF8 );	// プログラムキー名称.
					}
					if( ( 0 != ul_ShorteningID )
					 || ( 0 != ul_GroupID )
					 || ( TYPE_SendModeNotSetting != e_SendMode ) ){
						// プログラム宛先ありの場合、プログラム宛先選択確認警告を表示する.
						m_ss_ConfirmationProgramNo = PNLC_WebUIControl::getInstance()->getSelectProgramNo();
						gp_ConfirmProgramDestinationSelect_warn.setProgramName( sc_ProgramKeyName );
						gp_PublicPopupDummyBase.setupOpenPopup( &gp_ConfirmProgramDestinationSelect_warn );
						gp_PublicPopupDummyBase.setupNextGamen( nowGamen );
						gp_PublicPopupDummyBase.setMode( PNLE_PublicPopupDummyModeAct );
						nextGamen = &gp_PublicPopupDummyBase;
						OpenPopupIndi();
						break;
// comment 2020/11/27 Sky東島 [IT6_2.1][OP_BTS-28868].
//					} else {
//						// プログラム宛先なしの場合、設定中の宛先をクリアする.
//						gp_ProgramBasic_scanfax.m_bl_clearAddress = True;
					}
				}
				PNLC_WebUIControl::getInstance()->setUIMode( PNLE_UIModeLegacyUI );
				procFax( 0 );
				if( ( NULL != nextGamen )
				 || ( ( NULL != pc_PopupListControl )
				   && ( True == pc_PopupListControl->isPopupLayer( &gp_IGCClearWait_popup ) ) ) ) {
// comment 2020/09/25 Sky中野 [IT6_2.1][OP_BTS-27503].
//					TYPC_ProgramKey* pc_ProgramKey = PNLC_HomeProgramKeyData::getInstance()->getProgramKeyData( PNLC_WebUIControl::getInstance()->getSelectProgramNo() );
					if( NULL != pc_ProgramKey ) {
						gp_ProgramBasic_scanfax.setProgramKeyData( PNLC_WebUIControl::getInstance()->getSelectProgramNo(), pc_ProgramKey );
					}
				}
			}
			break;
		case PNLE_TopMenuShortCut_FavoriteList:												// お気に入り一覧画面.
			PNLC_WebUIControl::getInstance()->setUIMode( PNLE_UIModeLightUI );
			PNLC_WebUIControl::getInstance()->reserveChangeScreen( PNLE_WebUIRequestMessageFavoriteList );
			rebuildGamen( 0 );
// comment 2020/07/14 Sky中野 [IT6_2.0MR3_SpecialROM][OP_BTS-25429].
			if( PNLE_WebUIScreenChangeStatusDuring == PNLC_WebUIControl::getInstance()->getWebUIScreenChangeStatus() ) {
				// WebUI画面へ遷移する場合は、トップメニューモードをOFFにする.
				onlyPanel.setTopmenuMode( False );
			}
			break;
// comment 2020/06/08 Sky東島 [IT6_2.0MR3][制御構成変更_No.31].
		case PNLE_TopMenuShortCut_SysBoxPkiEncryptDocBox:												// PKI暗号化ボックスに遷移
// comment 2020/08/17 SCD2-Sky石原[IT6_2.1][制御_No.67]PKI機能WebUI化.
//			procBox( 0 );
//			if(NULL!=nextGamen){
//				PNLC_WebUIControl::getInstance()->setUIMode( PNLE_UIModeLegacyUI );
//				nextGamen=&gp_DocumentSelectPrintListEncryptPKI_box;
//				PNLC_BoxDataLib::newInstance()->setSelectBoxID( DOCD_EncFile_BoxId );
//				PNLC_BoxBoxListControlLib::newInstance()->setSelectBoxID( DOCD_EncFile_BoxId );
//				PnlComGetSelectDocument()->setBoxNumber( DOCD_EncFile_BoxId );
//				// トップメニューから直接システムボックスへ遷移する際は、セキュリティボックスから離脱したと判断する.
//				PNLC_TabletSecurityControlLib* pc_TabletSecurityControl = PNLC_TabletSecurityControlLib::newInstance();
//				if( NULL != pc_TabletSecurityControl ) {
//					pc_TabletSecurityControl->securityAreaExited();
//				}
//			}
			procBoxFuncLightUI( PNLE_WebUIFunctionPkiEncryptDocBox );
			break;
// comment 2021/01/18 Sky中野 [IT6_2.2] 制御No.34_目的別UI対応(ScanToURL/ScanToHome).
		case PNLE_TopMenuShortCut_ScanFaxScanToHome:										// ScanToHome.
			procEachScanLightUI( PNLE_WebUIFunctionScanToHome );
			break;
		case PNLE_TopMenuShortCut_ScanFaxScanToURL:											// ScanToURL.
			procEachScanLightUI( PNLE_WebUIFunctionScanToURL );
			break;
// comment 2021/01/14 Sky中野 [IT6_2.2] 制御No.36_目的別UI対応(小サイズスキャン).
		case PNLE_TopMenuShortCut_ScanFaxSmallSizeScan:										// 小サイズスキャン.
			procEachScanLightUI( PNLE_WebUIFunctionSmallSizeScan );
			break;
// comment 2022/06/30 Sky東島 [IT6_2.3][制御No.39-42_目的別UIプリセット化].
		case PNLE_TopMenuShortCut_CopyBookletCopy:		procCopyFuncLightUI(PNLE_WebUIFunctionBookletCopy);			break;	// 小冊子.
		case PNLE_TopMenuShortCut_CopyShadowEraseCopy:	procCopyFuncLightUI(PNLE_WebUIFunctionShadowEraseCopy);		break;	// 影消しコピー.
		case PNLE_TopMenuShortCut_SysBoxForcedMemory:	procBoxFuncLightUI(PNLE_WebUIFunctionForcedMemory);		break;	// 強制メモリー受信.
/* // WebUI制御自動生成マクロ用処理 削除禁止 Start.
// comment YYYY/MM/DD DeveloperName [ModelName] WBSNo Title .
		case PNLE_TopMenuShortCut_XXXX:				// ModelName WBSNo Title.
// ★★★★★ 機能に応じた実装をすること ★★★★★.
//			procCopyFuncLightUI( PNLE_WebUIFunctionXXXX );
//			procEachScanLightUI( PNLE_WebUIFunctionXXXX );
//			procFaxLightUI( PNLE_WebUIFunctionXXXX );
//			procBoxFuncLightUI( PNLE_WebUIFunctionXXXX );
			break;
// WebUI制御自動生成マクロ用処理 削除禁止 End. */
		default:
			return;
	}
	
	if( PNLE_TopMenuShortCut_ScanFaxMemory == _e_TopMenuShortCut ) {
		// Legacyスキャンのプログラム呼び出し時は、以降の処理は不要.
		return;
	}

// comment 2019/03/19 SCD2-Sky水野 [EagleH][OP_BTS-2473]ジョブクリアをHOMEの機能キー押下時に実施するよう変更.
	if( PCGE_BuzzerType_InputNg != getBuzzerType() ){
		PNLC_WebUIControl::getInstance()->initPanelJobSettings();
	}
	
// comment 2019/08/09 SCD2-Sky中野 [EagleH][OP_BTS-11261].
	if( PNLE_WebUIScreenChangeStatusStart == PNLC_WebUIControl::getInstance()->getWebUIScreenChangeStatus() ) {
		// WebUIへの遷移要求を予約したがWebUIへ遷移しなかった場合、遷移要求情報をクリアする.
		PNLC_WebUIControl::getInstance()->crearChangeScreenInfo();
	}
	
//2018/12/18 [HCLT_Prashannaa] [AR 1067960] Reverting returning to Home screen changes.
//2018/12/10 [HCLT_Prashannaa] [AR 1067960] returning to Home screen changes.
//	if( ( e_NowUIMode_bak == PNLE_UIModeLightUI)
//		&& ( _e_TopMenuShortCut != PNLE_TopMenuShortCut_UIModeLegacy ) ){
//			onlyPanel.setBrowserMode( False );
//			onlyPanel.setTopmenuMode( True );
//			PNLC_WebUIControl::getInstance()->setUIMode(PNLE_UIModeLightUI);
//	}
}

//================================================================================================.
// 概要 : procEachScanFaxLightUI :  各Scan遷移処理（LightUI用）.
// comment : WebUI2 ScanFax画面切替え制御 20180405 Y.Yamabe.
//================================================================================================.
// comment 2018/04/05 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
void PCGC_HomeTopMenu::procEachScanLightUI( PNLE_WebUIFunction e_WebUIFunction )
{
// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
	PNLE_UIMode uc_UIModebak = PNLC_WebUIControl::getInstance()->getUIMode();
	PNLC_WebUIControl::getInstance()->setUIMode( PNLE_UIModeLightUI );
	
	// メンバ変数にRequestMessage設定.
	setWebUIEachScanFunc(e_WebUIFunction);

	// スキャン/ファクス機能設定.
	procFax( 0 );

	setWebUIEachScanFunc(PNLE_WebUIFunctionSize);

// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
	if( PNLE_WebUIScreenChangeStatusDuring != PNLC_WebUIControl::getInstance()->getWebUIScreenChangeStatus() ) {
		// 画面遷移しなかった場合は、UIモードを元に戻す.
		PNLC_WebUIControl::getInstance()->setUIMode( uc_UIModebak );
	}
}

// comment 2018/11/05 SCD2-Sky中野 [EagleH] WebUIv3FUM対応.
////================================================================================================
//// 概要	： A072002000_トップメニュー（統合トップメニュー） ： アプリケーション選択チェック.
//// comment 2018/04/16 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
////================================================================================================
//Bool PCGC_HomeTopMenu::isFuncEnableHomeMenu( PNLE_ACCOUNT_STATUS_TYPE e_AccountStatusType , PNLE_TopMenuShortCut e_ShortCutKind)
//{
//	Bool _bl_ret = False;
//
//	_bl_ret = isFuncEnable(e_AccountStatusType);
//	if(True == _bl_ret) {
//		switch( e_ShortCutKind ) {
//			case PNLE_TopMenuShortCut_FuncCopy:			// コピー.
//				if( ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionCopy ) )
//				 || ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//					_bl_ret = False;
//				}
//				break;
//		
//			case PNLE_TopMenuShortCut_ScanToEmail:		// ScanToEmail.
//				if( ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionScanToEmail ) )
//				 || ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//					_bl_ret = False;
//				}
//				break;
//			case PNLE_TopMenuShortCut_ScanToBox:		// ScanToBox.	
//				if( ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionScanToBox ) )
//				 || ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//					_bl_ret = False;
//				}
//				break;
//			case PNLE_TopMenuShortCut_ScanToPC:			// ScanToPC.
//				if( ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionScanToPC ) )
//				 || ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//					_bl_ret = False;
//				}
//				break;
//
//			case PNLE_TopMenuShortCut_InternetFax:		// InternetFax.
//				// InternetFax使用可否判定処理
//				if( ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionIFax ) )
//				 || ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//					_bl_ret = False;
//				}
//				break;
//				
//			case PNLE_TopMenuShortCut_IPAddressFax:		// IPAddressFax.
//				// IPAddressFax設定使用可否判定処理
//				if( ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionIPAddressFax ) )
//				 || ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//					_bl_ret = False;
//				}
//				break;
//			case PNLE_TopMenuShortCut_FuncFaxOnly:	// FaxOnly.
//				if( ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionFax ) )
//				 || ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//					_bl_ret = False;
//				}
//				break;
//			case PNLE_TopMenuShortCut_FuncBrowser:
//				break;
//			case PNLE_TopMenuShortCut_MarketPlace:
//				break;
//// Comment 2018/06/28 [HCL-Pavani] [IT6_Eagle] [AR1048863] Download and install MarketPlace on all MFPs according to the performance in LightUI.
//			case PNLE_TopMenuShortCut_MarketPlaceSetup:
//				break;
//			case PNLE_TopMenuShortCut_FuncOAPApp:
//				break;
//			case PNLE_TopMenuShortCut_AddressBook:
//				break;
//			case PNLE_TopMenuShortCut_CopyPaperType:
//				break;
//			case PNLE_TopMenuShortCut_SysBoxAuthPrint:
//				// comment : 20180508 Chikuan : [Eagle] [KRQ-314-28] 【新UI】認証＆プリントBox対応.
//				{
//					PNLC_SystemBoxFunctionKeyDispStateLib c_SBLib;	// システムボックス各キーの表示状態判定ライブラリ.
//					if( ( False == c_SBLib.isActivateIDAndPrintBox() )
//					 || ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionAuthPrint ) )
//					 || ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//						_bl_ret = False;
//					}
//				}
//				break;
//			case PNLE_TopMenuShortCut_SysBoxExternalMemory:
//				break;
//			case PNLE_TopMenuShortCut_FuncUbqPrint:
//				break;
//			case PNLE_TopMenuShortCut_UIModeLegacy:
//			case PNLE_TopMenuShortCut_UIModeLight:
//				_bl_ret = True;
//				break;
//// comment 2018/11/09 Sky雜賀 [EagleH][RQ-272].
//			case PNLE_TopMenuShortCut_IpFaxT38:
//				if( ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionIpFaxT38 ) )
//				 || ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//					_bl_ret = False;
//				}
//				break;
//			default:
//				break;
//		}
//	}
//	return _bl_ret;
//}
//
////================================================================================================.
//// 概要 : isFuncShowHomeMenu :  機能毎の表示チェック（LightUI用）.
//// comment : WebUI2 ScanFax画面切替え制御 20180405 Y.Yamabe.
////================================================================================================.
//Bool PCGC_HomeTopMenu::isFuncShowHomeMenu( PNLE_ACCOUNT_STATUS_TYPE e_AccountStatusType, PNLE_TopMenuShortCut e_ShortCutKind)
//{
//	Bool _bl_ret = False;
//	TYPE_MachineType e_MachineType = PnlComGetSystemInfo()->getMachineType();
//	TYPC_ComSend* pc_ComSend = NULL;
//
//	if(PNLE_TopMenuShortCut_FuncFaxOnly == e_ShortCutKind) {
//		_bl_ret = isFuncShowFaxOnly();
//	}
//	else {
//		_bl_ret = isFuncShow( e_AccountStatusType );
//	}
//	if(PnlComGetJobFax() != NULL) {
//		pc_ComSend = &( PnlComGetJobFax()->getJobSpec()->pc_Send->c_ComSend );
//	}
//	if(True == _bl_ret) {
//		switch( e_ShortCutKind ) {
//			case PNLE_TopMenuShortCut_FuncCopy:			// コピー.
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//					_bl_ret = False;
//				}
//				break;
//
//			case PNLE_TopMenuShortCut_ScanToEmail:		// ScanToEmail.
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(True);
//				if( ( False == isEnableSetAddress( pc_ComSend, TYPE_SendModeEmail, False, False, False, True, -1, NULL, True ) ) ) {	// 送信可否判定.
//					// E-mailキー非表示.
//					_bl_ret = False;
//				}
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(False);
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//					_bl_ret = False;
//				}
//				break;
//				
//			case PNLE_TopMenuShortCut_ScanToBox:		// ScanToBox.	
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(True);
//				if( ( False == isEnableSetAddress( pc_ComSend, TYPE_SendModeBox, False, False, False, True, -1, NULL, True ) )) {
//					// ScanToBoxキー非表示.
//					_bl_ret = False;
//				}
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(False);
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//					_bl_ret = False;
//				}
//// Comment 2018/11/19 【HCLT-Vandana】【IT6_DenebMLK】【AR 1070436】【Hide User box in SSD less and Micro SD installed environment】
//#ifdef DEF_DenebMLKCus
//			if ( True == PnlComIsSecurityDoc() ) {
//				_bl_ret = False;
//				SetFuncShowHomeMenu(False);
//			}
//#endif
//				break;
//		
//			case PNLE_TopMenuShortCut_ScanToPC:			// ScanToPC.
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(True);
//// comment 2018/09/28 SCD2-Sky飛田 [Eagle][CQ1062921] ScanToPCの表示条件を修正.
//				if( ( False == isEnableSetAddress( pc_ComSend, TYPE_SendModeSmb, False, False, False, True, -1, NULL, True ) )		// SMB設定使用可否判定処理.
//					&& ( False == isEnableSetAddress( pc_ComSend, TYPE_SendModeFtp, False, False, False, True, -1, NULL, True ) ) ) {	// FTP設定使用可否判定処理.
//					// ScanToPCキー非表示.
//					_bl_ret = False;
//				}
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(False);
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//					_bl_ret = False;
//				}
//				break;
//
//			case PNLE_TopMenuShortCut_InternetFax:		// InternetFax.
//				// InternetFax使用可否判定処理
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(True);
//				if( ( False == isEnableSetAddress( pc_ComSend, TYPE_SendModeIfax, False, False, False, True, -1, NULL, True ) )) {
//					// InternetFaxキー非表示.
//					_bl_ret = False;
//				}
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(False);
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//					_bl_ret = False;
//				}
//				break;
//				
//			case PNLE_TopMenuShortCut_IPAddressFax:		// IPAddressFax.
//				// IPAddressFax設定使用可否判定処理
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(True);
//				if( ( False == isEnableSetAddress( pc_ComSend, TYPE_SendModeIPAddressFax, False, False, False, True, -1, NULL, True ) )) {
//					// IPAddressFaxキー非表示.
//					_bl_ret = False;
//				}
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(False);
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//					_bl_ret = False;
//				}
//				break;
//			case PNLE_TopMenuShortCut_FuncFaxOnly:
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//					_bl_ret = False;
//				}
//				break;
//			case PNLE_TopMenuShortCut_FuncBrowser:
//				// Webブラウザーが無効の場合、Webブラウザーキー非表示.
//// Commment 2018/06/05 HCLT_Ajit [IT6_Eagle][AR1045587] Removing WebLicense Management.
////				if( ( False == PnlComGetBrowserSetting()->getEnableBrowser() )
////				    || ( TYPE_CategoryState_ENABLE != PnlComGetEnablerSetting()->getFEK_CotegoryState( TYPE_FEKType01 ) ) ) {
////					_bl_ret = False;
////				}
//				if( False == PnlComGetBrowserSetting()->getEnableBrowser() ) {
//					_bl_ret = False;
//				}
//				break;
//			case PNLE_TopMenuShortCut_FuncFaxOnly:	// FaxOnly.
//				if( ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionFax ) )
//				 || ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//					_bl_ret = False;
//				}
//				break;
//			case PNLE_TopMenuShortCut_FuncBrowser:
//				break;
//			case PNLE_TopMenuShortCut_MarketPlace:
//				break;
//// Comment 2018/06/28 [HCL-Pavani] [IT6_Eagle] [AR1048863] Download and install MarketPlace on all MFPs according to the performance in LightUI.
//			case PNLE_TopMenuShortCut_MarketPlaceSetup:
//				break;
//			case PNLE_TopMenuShortCut_FuncOAPApp:
//				break;
//			case PNLE_TopMenuShortCut_AddressBook:
//				break;
//			case PNLE_TopMenuShortCut_CopyPaperType:
//				break;
//			case PNLE_TopMenuShortCut_SysBoxAuthPrint:
//				// comment : 20180508 Chikuan : [Eagle] [KRQ-314-28] 【新UI】認証＆プリントBox対応.
//				{
//					PNLC_SystemBoxFunctionKeyDispStateLib c_SBLib;	// システムボックス各キーの表示状態判定ライブラリ.
//					if( ( False == c_SBLib.isActivateIDAndPrintBox() )
//					 || ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionAuthPrint ) )
//					 || ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//						_bl_ret = False;
//					}
//				}
//				break;
//			case PNLE_TopMenuShortCut_SysBoxExternalMemory:
//				break;
//			case PNLE_TopMenuShortCut_FuncUbqPrint:
//				break;
//			case PNLE_TopMenuShortCut_UIModeLegacy:
//			case PNLE_TopMenuShortCut_UIModeLight:
//				_bl_ret = True;
//				break;
//// comment 2018/11/09 Sky雜賀 [EagleH][RQ-272].
//				break;
//			case PNLE_TopMenuShortCut_IpFaxT38:		// Ip-Fax(T.38).
//				SetFuncShowHomeMenu(True);
//				if( ( False == isEnableSetAddress( pc_ComSend, TYPE_SendModeIPFax, False, False, False, True, -1, NULL, True ) ) ) {	// 送信可否判定.
//					// IP-FAX(T.38)キー非表示.
//					_bl_ret = False;
//				}
//				SetFuncShowHomeMenu(False);
//				break;
//			default:
//				break;
//		}
//	}
//	return _bl_ret;
//}
//
////================================================================================================.
//// 概要 : isFuncShowHomeMenu :  機能毎の表示チェック（LightUI用）.
//// comment : WebUI2 ScanFax画面切替え制御 20180405 Y.Yamabe.
////================================================================================================.
//Bool PCGC_HomeTopMenu::isFuncShowHomeMenu( PNLE_ACCOUNT_STATUS_TYPE e_AccountStatusType, PNLE_TopMenuShortCut e_ShortCutKind)
//{
//	Bool _bl_ret = False;
//	TYPE_MachineType e_MachineType = PnlComGetSystemInfo()->getMachineType();
//	TYPC_ComSend* pc_ComSend = NULL;
//
//	if(PNLE_TopMenuShortCut_FuncFaxOnly == e_ShortCutKind) {
//		_bl_ret = isFuncShowFaxOnly();
//	}
//	else {
//		_bl_ret = isFuncShow( e_AccountStatusType );
//	}
//	if(PnlComGetJobFax() != NULL) {
//		pc_ComSend = &( PnlComGetJobFax()->getJobSpec()->pc_Send->c_ComSend );
//	}
//	if(True == _bl_ret) {
//		switch( e_ShortCutKind ) {
//			case PNLE_TopMenuShortCut_FuncCopy:			// コピー.
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//					_bl_ret = False;
//				}
//				break;
//
//			case PNLE_TopMenuShortCut_ScanToEmail:		// ScanToEmail.
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(True);
//				if( ( False == isEnableSetAddress( pc_ComSend, TYPE_SendModeEmail, False, False, False, True, -1, NULL, True ) ) ) {	// 送信可否判定.
//					// E-mailキー非表示.
//					_bl_ret = False;
//				}
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(False);
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//					_bl_ret = False;
//				}
//				break;
//				
//			case PNLE_TopMenuShortCut_ScanToBox:		// ScanToBox.	
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(True);
//				if( ( False == isEnableSetAddress( pc_ComSend, TYPE_SendModeBox, False, False, False, True, -1, NULL, True ) )) {
//					// ScanToBoxキー非表示.
//					_bl_ret = False;
//				}
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(False);
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//					_bl_ret = False;
//				}
//// Comment 2018/11/19 【HCLT-Vandana】【IT6_DenebMLK】【AR 1070436】【Hide User box in SSD less and Micro SD installed environment】
//#ifdef DEF_DenebMLKCus
//			if ( True == PnlComIsSecurityDoc() ) {
//				_bl_ret = False;
//				SetFuncShowHomeMenu(False);
//			}
//#endif
//				break;
//		
//			case PNLE_TopMenuShortCut_ScanToPC:			// ScanToPC.
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(True);
//// comment 2018/09/28 SCD2-Sky飛田 [Eagle][CQ1062921] ScanToPCの表示条件を修正.
//				if( ( False == isEnableSetAddress( pc_ComSend, TYPE_SendModeSmb, False, False, False, True, -1, NULL, True ) )		// SMB設定使用可否判定処理.
//					&& ( False == isEnableSetAddress( pc_ComSend, TYPE_SendModeFtp, False, False, False, True, -1, NULL, True ) ) ) {	// FTP設定使用可否判定処理.
//					// ScanToPCキー非表示.
//					_bl_ret = False;
//				}
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(False);
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//					_bl_ret = False;
//				}
//				break;
//
//			case PNLE_TopMenuShortCut_InternetFax:		// InternetFax.
//				// InternetFax使用可否判定処理
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(True);
//				if( ( False == isEnableSetAddress( pc_ComSend, TYPE_SendModeIfax, False, False, False, True, -1, NULL, True ) )) {
//					// InternetFaxキー非表示.
//					_bl_ret = False;
//				}
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(False);
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//					_bl_ret = False;
//				}
//				break;
//				
//			case PNLE_TopMenuShortCut_IPAddressFax:		// IPAddressFax.
//				// IPAddressFax設定使用可否判定処理
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(True);
//				if( ( False == isEnableSetAddress( pc_ComSend, TYPE_SendModeIPAddressFax, False, False, False, True, -1, NULL, True ) )) {
//					// IPAddressFaxキー非表示.
//					_bl_ret = False;
//				}
//// comment 2018/07/30 SCD2-Sky小畑 [Eagle][CQ1048913]
//				SetFuncShowHomeMenu(False);
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//					_bl_ret = False;
//				}
//				break;
//			// 【RQ-272】仮対応.
//			case PNLE_TopMenuShortCut_IPFax:		// IPFax.
//				break;
//			case PNLE_TopMenuShortCut_FuncFaxOnly:
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//					_bl_ret = False;
//				}
//				break;
//			case PNLE_TopMenuShortCut_FuncBrowser:
//				// Webブラウザーが無効の場合、Webブラウザーキー非表示.
//// Commment 2018/06/05 HCLT_Ajit [IT6_Eagle][AR1045587] Removing WebLicense Management.
////				if( ( False == PnlComGetBrowserSetting()->getEnableBrowser() )
////				    || ( TYPE_CategoryState_ENABLE != PnlComGetEnablerSetting()->getFEK_CotegoryState( TYPE_FEKType01 ) ) ) {
////					_bl_ret = False;
////				}
//				if( False == PnlComGetBrowserSetting()->getEnableBrowser() ) {
//					_bl_ret = False;
//				}
//				break;
//			case PNLE_TopMenuShortCut_MarketPlace:
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//// comment 2018/10/12 HCL_Nandhini [Sparrow] [AR1063073] Market Place is applicable for Sparrow1SFP model.
////				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {			
//				if( TYPE_MachineTypeSparrow2SFP == e_MachineType ) {
//					_bl_ret = False;
//				}
//				break;
//// Comment 2018/06/28 [HCL-Pavani] [IT6_Eagle] [AR1048863] Download and install MarketPlace on all MFPs according to the performance in LightUI.
//			case PNLE_TopMenuShortCut_MarketPlaceSetup:
//				break;
//			case PNLE_TopMenuShortCut_FuncOAPApp:
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//				if(TYPE_MachineTypeSparrow2SFP == e_MachineType) {
//					_bl_ret = False;
//				}
//				break;
//			case PNLE_TopMenuShortCut_AddressBook:
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//					_bl_ret = False;
//				}
//				break;
//			case PNLE_TopMenuShortCut_CopyPaperType:
//				break;
//			case PNLE_TopMenuShortCut_SysBoxAuthPrint:
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//					_bl_ret = False;
//				// comment : 20180508 Chikuan : [Eagle] [KRQ-314-28] 【新UI】認証＆プリントBox対応.
//				}else{
//					PNLC_SystemBoxFunctionKeyDispStateLib c_SBLib;	// システムボックス各キーの表示状態判定ライブラリ.
//					_bl_ret = c_SBLib.isShowIDAndPrintBox();
//				}
//				break;
//			case PNLE_TopMenuShortCut_SysBoxExternalMemory:
////comment 2018/12/21 HCLT-Rajkumar [DenebMLK] Resolving jenkins error.
//			{
//// Comment 2018/12/19 [HCLT-KothaSaran][IT6_DenebMLK][AR1069313] External Memory should be hidden based on condition in DenebMLK.
//				APIC_UsbMemSetting* pc_UsbMemSetting = APIC_UsbMemSetting::newInstance();
//				if( NULL == pc_UsbMemSetting ) {
//					_bl_ret = False;
//					return _bl_ret;
//				}
//				TYPE_NetworkSpecific e_NetworkSpecific  = PNLC_NetworkLan::getInstance()->getLine();
//				if( False == pc_UsbMemSetting->getEnableUsbDirectPrint( e_NetworkSpecific ) ) {
//					_bl_ret = False;
//				}
//				break;
////comment 2018/12/21 HCLT-Rajkumar [DenebMLK] Resolving jenkins error.
//			}
//// comment 2018/05/24 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
//			case PNLE_TopMenuShortCut_FuncUbqPrint:
//				if( TYPE_MachineTypeSparrow1SFP != e_MachineType) {
//					_bl_ret = False;
//				}
//				break;
//			case PNLE_TopMenuShortCut_UIModeLegacy:
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//// Sparrow1SFP Sparrow2SFPはClassicStyle切り替えキー非表示だが、現時点で非表示にすると新UI側でカウンターキーが表示されない為.
//// サービスモードへ遷移できない為、新UIカウンターキーが表示され、サービスモードに遷移できるようになるまで非表示とする.
////				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
////					_bl_ret = False;
////				}
//				break;
//			default:
//				break;
//		}
//	}
//	return _bl_ret;
//}
//
//================================================================================================.
// 概要 : getEachScanWebUIReqMsg :  各Scan処理（LightUI用）.
// comment : WebUI2 保存したPNLE_WebUIFunctionをPNLE_WebUIRequestMessageに変換して返却処理（LightUI用）20180406 Y.Yamabe.
//================================================================================================.
// comment 2018/04/06 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
PNLE_WebUIRequestMessage PCGC_HomeTopMenu::getEachScanWebUIReqMsg()
{

// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
//	PNLE_WebUIRequestMessage e_ret = PNLE_WebUIFunctionSize;
	PNLE_WebUIRequestMessage e_ret = PNLE_WebUIRequestMessageNon;
	switch( m_e_WebUIFunction ){
		case PNLE_WebUIFunctionScan:
			e_ret = PNLE_WebUIRequestMessageScan;
			break;
		case PNLE_WebUIFunctionScanToEmail:
// comment 2020/03/10 SCD2-Sky中野 [IT6_2.0MR2] 制御No.30_Program登録機能.
//			e_ret = PNLE_WebUIRequestMessageScanToEmail;
			if( 0 == PNLC_WebUIControl::getInstance()->getSelectProgramNo() ) {
				e_ret = PNLE_WebUIRequestMessageScanToEmail;
			} else {
				e_ret = PNLE_WebUIRequestMessageScanToEmailProgram;
			}
			break;
		case PNLE_WebUIFunctionScanToBox:
// comment 2020/03/10 SCD2-Sky中野 [IT6_2.0MR2] 制御No.30_Program登録機能.
//			e_ret = PNLE_WebUIRequestMessageScanToBox;
			if( 0 == PNLC_WebUIControl::getInstance()->getSelectProgramNo() ) {
				e_ret = PNLE_WebUIRequestMessageScanToBox;
			} else {
				e_ret = PNLE_WebUIRequestMessageScanToBoxProgram;
			}
			break;
		case PNLE_WebUIFunctionScanToPC:
// comment 2020/03/10 SCD2-Sky中野 [IT6_2.0MR2] 制御No.30_Program登録機能.
//			e_ret = PNLE_WebUIRequestMessageScanToPC;
			if( 0 == PNLC_WebUIControl::getInstance()->getSelectProgramNo() ) {
				e_ret = PNLE_WebUIRequestMessageScanToPC;
			} else {
				e_ret = PNLE_WebUIRequestMessageScanToPCProgram;
			}
			break;
		case PNLE_WebUIFunctionIFax:
// comment 2020/03/10 SCD2-Sky中野 [IT6_2.0MR2] 制御No.30_Program登録機能.
//			e_ret = PNLE_WebUIRequestMessageIFax;
			if( 0 == PNLC_WebUIControl::getInstance()->getSelectProgramNo() ) {
				e_ret = PNLE_WebUIRequestMessageIFax;
			} else {
				e_ret = PNLE_WebUIRequestMessageIFaxProgram;
			}
			break;
		case PNLE_WebUIFunctionIPAddressFax:
// comment 2020/03/10 SCD2-Sky中野 [IT6_2.0MR2] 制御No.30_Program登録機能.
//			e_ret = PNLE_WebUIRequestMessageIPAddressFax;
			if( 0 == PNLC_WebUIControl::getInstance()->getSelectProgramNo() ) {
				e_ret = PNLE_WebUIRequestMessageIPAddressFax;
			} else {
				e_ret = PNLE_WebUIRequestMessageIPAddressFaxProgram;
			}
			break;
// comment 2018/11/09 Sky雜賀 [EagleH][RQ-272].
		case PNLE_WebUIFunctionIpFaxT38:
// comment 2020/03/10 SCD2-Sky中野 [IT6_2.0MR2] 制御No.30_Program登録機能.
//			e_ret = PNLE_WebUIRequestMessageIpFaxT38;
			if( 0 == PNLC_WebUIControl::getInstance()->getSelectProgramNo() ) {
				e_ret = PNLE_WebUIRequestMessageIpFaxT38;
			} else {
				e_ret = PNLE_WebUIRequestMessageIpFaxT38Program;
			}
			break;
// comment 2020/03/09 SCD2-Sky石原[IT6_2.0MR2][制御No.31]新CopyScanFAXアプリのカスタマイズ機能.
		case PNLE_WebUIFunctionIFaxCustomize:
			e_ret = PNLE_WebUIRequestMessageToIFaxCustomize;
			break;
		case  PNLE_WebUIFunctionIPAddressFaxCustomize:
			e_ret = PNLE_WebUIRequestMessageToIPAdressFaxCustomize;
			break;
		case PNLE_WebUIFunctionIPFaxT38Customize:
			e_ret =	PNLE_WebUIRequestMessageToIPFaxT38Customize;
			break;
		case PNLE_WebUIFunctionScanToEmailCustomize:
			e_ret = PNLE_WebUIRequestMessageToScanToEmailCustomize;
			break;	
		case PNLE_WebUIFunctionScanToPCCustomize:
			e_ret = PNLE_WebUIRequestMessageToScanToPCCustomize;
			break;
		case PNLE_WebUIFunctionScanToBoxCustomize:
			e_ret = PNLE_WebUIRequestMessageToScanToBoxCustomize;
			break;
// comment 2019/01/10 SCD2-Sky肥後 [EagleH] 制御No.38_目的別UI対応.
		case PNLE_WebUIFunctionScanToMe:
// comment 2019/06/04 SCD2-Sky肥後 [EagleH][OP_BTS-6284] ScanToMeでワンタッチジョブ時の警告通知対応.
			if( True == PNLC_WebUIControl::getInstance()->isScanToMeWarning() ) {
				e_ret = PNLE_WebUIRequestMessageScanToMeWarning;
			}
			else {
// comment 2020/03/10 SCD2-Sky中野 [IT6_2.0MR2] 制御No.30_Program登録機能.
//				e_ret = PNLE_WebUIRequestMessageScanToMe;
				if( 0 == PNLC_WebUIControl::getInstance()->getSelectProgramNo() ) {
					e_ret = PNLE_WebUIRequestMessageScanToMe;
				} else {
					e_ret = PNLE_WebUIRequestMessageScanToMeProgram;
				}
			}
			break;
// comment 2021/01/18 Sky中野 [IT6_2.2] 制御No.34_目的別UI対応(ScanToURL/ScanToHome).
		case PNLE_WebUIFunctionScanToHome:				// Scan To Home.
			if( True == PNLC_WebUIControl::getInstance()->getScanToHomeWarning() ) {
				e_ret = PNLE_WebUIRequestMessageScanToHomeWarning;
			} else {
				e_ret = PNLE_WebUIRequestMessageScanToHome;
			}
			break;
		case PNLE_WebUIFunctionScanToURL:				// Scan To URL.
			if( True == PNLC_WebUIControl::getInstance()->getScanToURLWarning() ) {
				e_ret = PNLE_WebUIRequestMessageScanToURLWarning;
			} else {
				e_ret = PNLE_WebUIRequestMessageScanToURL;
			}
			break;
// comment 2021/01/14 Sky中野 [IT6_2.2] 制御No.36_目的別UI対応(小サイズスキャン).
		case PNLE_WebUIFunctionSmallSizeScan:			// 小サイズスキャン.
			e_ret = PNLE_WebUIRequestMessageSmallSizeScan;
			break;
/* // WebUI制御自動生成マクロ用処理 WebUIスキャン機能 削除禁止 Start.
// comment YYYY/MM/DD DeveloperName [ModelName] WBSNo Title .
		case PNLE_WebUIFunctionXXXX:				// ModelName WBSNo Title.
			e_ret = PNLE_WebUIRequestMessageXXXX;
			break;
// WebUI制御自動生成マクロ用処理 削除禁止 End. */
		default:
			break;
	}
	return e_ret;
}

// comment 2018/11/05 SCD2-Sky中野 [EagleH] WebUIv3FUM対応.
////================================================================================================
//// 概要	： A072002000_Home(メニュー) ： 機能情報の設定.
////================================================================================================
//struct PNLS_TopMenuBadgeKeyText2 {
//	PNLE_TopMenuShortCut	e_ShortCutKind;
//	TEXT_ID					tid_FuncName;
//	uchar*					puc_FuncIcon;
//	uchar					uc_IconPaletteType;
//	uchar*					puc_IconNotActivate;
//	uchar					uc_IconPaletteNotActivate;
//	uchar*					puc_FuncIcon_n;			// キー選択(非選択状態).
//	uchar*					puc_FuncIcon_s;			// キー選択(選択状態).
//	uchar*					puc_FuncIcon_d;			// キー選択(非活性状態).
//	uchar					uc_IconPaletteLightUI;
//};	
//static const PNLS_TopMenuBadgeKeyText2 st_TopMenuBadgeKeyFuncTbl2[] = {
//#if defined(DEF_LCD_7INCH)
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_】 Card copy setting support for DenebMLK model.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No._77-1】 Non Image Area Erase setting support for DenebMLK model.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
//#if defined( DEF_DenebMLKCus )
////#if 0 //Temp Fix To Enable ClassicHome Button
//	{ PNLE_TopMenuShortCut_FuncCopy,			s_pITMCopy_W120H048E,							( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_015_000_n_X108_Y108,						( uchar* )g70_015_000_s_X108_Y108,					( uchar* )g70_015_000_d_X108_Y108,		 	 		PNLD_PaletteNewUIApp},	// 機能:コピー.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
//	{ PNLE_TopMenuShortCut_CopyNonImageAreaErase,s_pITMCardCopy_W120H048E,						( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_038_000_n_X124_Y110,						( uchar* )g70_038_000_s_X124_Y110,					( uchar* )g70_038_000_d_X124_Y110, 					PNLD_PaletteNewUIApp},	// 機能:コピー. // Frame earse.
//	{ PNLE_TopMenuShortCut_CopyCardCopy,		s_pITMOutsideDocumentErase_W120H048E,			( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_035_000_n_X124_Y110,						( uchar* )g70_035_000_s_X124_Y110,					( uchar* )g70_035_000_d_X124_Y110,  				PNLD_PaletteNewUIApp},	// 機能:コピー.	//Card copy.
//	{ PNLE_TopMenuShortCut_ScanToEmail,			s_pITMScanToEmail_W120H048E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_016_000_n_X108_Y108,						( uchar* )g70_016_000_s_X108_Y108,					( uchar* )g70_016_000_d_X108_Y108,  		 		PNLD_PaletteNewUIApp},	// 機能:ScanToE-mail.
//	{ PNLE_TopMenuShortCut_FuncFaxOnly,			s_pITMFax_W120H048E,							( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_021_000_n_X108_Y108,						( uchar* )g70_021_000_s_X108_Y108,					( uchar* )g70_021_000_d_X108_Y108,  		 		PNLD_PaletteNewUIApp},	// 機能:ファクス.
//	{ PNLE_TopMenuShortCut_UIModeLegacy,		s_pITMClassicStyle_W120H048E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_023_000_n_X108_Y108,						( uchar* )g70_023_000_s_X108_Y108,					( uchar* )g70_023_000_d_X108_Y108,  		 		PNLD_PaletteNewUIApp},	// UIモード切替.
//	{ PNLE_TopMenuShortCut_FuncBrowser,			s_pITMWebBrowser_W120H048E,						( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_022_000_n_X108_Y108,						( uchar* )g70_022_000_s_X108_Y108,					( uchar* )g70_022_000_d_X108_Y108,  		 		PNLD_PaletteNewUIApp},	// 機能:ブラウザ.
//	{ PNLE_TopMenuShortCut_MarketPlaceSetup,	s_pKEYMarketPlaceSetup_W110H042E,				( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )i70_043_000_n_X64_Y80,	( uchar* )i70_043_000_n_X64_Y80,	( uchar* )i70_043_000_d_X64_Y80,  	PNLD_PaletteNewUIApp},	// 機能:MarketPlace Setup Key.
//	{ PNLE_TopMenuShortCut_AddressBook,			s_pITMAddressBook_W120H048E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_027_000_n_X108_Y108,						( uchar* )g70_027_000_s_X108_Y108,					( uchar* )g70_027_000_d_X108_Y108,  		 		PNLD_PaletteNewUIApp},	// 機能:アドレス帳.
//	{ PNLE_TopMenuShortCut_CopyPaperType,		s_pITMPaperSetting_W120H048E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_029_000_n_X108_Y108,						( uchar* )g70_029_000_s_X108_Y108,					( uchar* )g70_029_000_d_X108_Y108,  		 		PNLD_PaletteNewUIApp},	// 用紙設定.
//	{ PNLE_TopMenuShortCut_ScanToBox,			s_pITMScanToBox_W120H048E,						( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_017_000_n_X108_Y108,						( uchar* )g70_017_000_s_X108_Y108,					( uchar* )g70_017_000_d_X108_Y108,  		 		PNLD_PaletteNewUIApp},	// 機能:ScanToBox.	
//	{ PNLE_TopMenuShortCut_ScanToPC	,			s_pITMScanToPC_W120H048E,						( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_018_000_n_X108_Y108,						( uchar* )g70_018_000_s_X108_Y108,					( uchar* )g70_018_000_d_X108_Y108,  		 		PNLD_PaletteNewUIApp},	// 機能:ScanToPC.
//	{ PNLE_TopMenuShortCut_InternetFax,			s_pITMInternetFax_W120H048E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_019_000_n_X108_Y108,						( uchar* )g70_019_000_s_X108_Y108,					( uchar* )g70_019_000_d_X108_Y108,  		 		PNLD_PaletteNewUIApp},	// 機能:InternetFax.
//	{ PNLE_TopMenuShortCut_IPAddressFax,		s_pITMIPAddressFax_W120H048E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_020_000_n_X108_Y108,						( uchar* )g70_020_000_s_X108_Y108,					( uchar* )g70_020_000_d_X108_Y108,  		 		PNLD_PaletteNewUIApp},	// 機能:IPAddressFax.
//	{ PNLE_TopMenuShortCut_SysBoxAuthPrint,		s_pITMAuthenticationAndPrintBox_W120H048E,		( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_025_000_n_X108_Y108,						( uchar* )g70_025_000_s_X108_Y108,					( uchar* )g70_025_000_d_X108_Y108,  		 		PNLD_PaletteNewUIApp},	// 認証＆プリント.
//	{ PNLE_TopMenuShortCut_SysBoxExternalMemory,s_pITMExternalMemory_W120H048E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_024_000_n_X108_Y108,						( uchar* )g70_024_000_s_X108_Y108,					( uchar* )g70_024_000_d_X108_Y108,  		 		PNLD_PaletteNewUIApp},	// 外部メモリー.
//	{ PNLE_TopMenuShortCut_FuncOAPApp,			s_pITMAPP_W120H048E,							( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_031_000_n_X108_Y108,						( uchar* )g70_031_000_s_X108_Y108,					( uchar* )g70_031_000_d_X108_Y108,  		 		PNLD_PaletteNewUIApp},	// 機能:OpenAPIアプリ.
//// Comment 2018/11/19 【HCLT-Prashannaa】【IT6_DenebMLK】【制御構成変更_No.83】
//	{ PNLE_TopMenuShortCut_SysBoxSecurity,		s_pKEYSecurityDocument_W100H042E,				( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_024_000_n_X108_Y108,						( uchar* )g70_024_000_s_X108_Y108,					( uchar* )g70_024_000_d_X108_Y108,  		 		PNLD_PaletteNewUIApp},	// Secureprint
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
////#endif
//#else //( DEF_DenebMLKCus ).
//	{ PNLE_TopMenuShortCut_FuncCopy,			s_pITMCopy_W120H050D,							( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_015_000_n_X108_Y108,	( uchar* )g70_015_000_s_X108_Y108,	( uchar* )g70_015_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:コピー.
//	{ PNLE_TopMenuShortCut_ScanToEmail,			s_pITMScanToEmail_W120H050D,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_016_000_n_X108_Y108,	( uchar* )g70_016_000_s_X108_Y108,	( uchar* )g70_016_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:ScanToE-mail.
//	{ PNLE_TopMenuShortCut_ScanToBox,			s_pITMScanToBox_W120H050D,						( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_017_000_n_X108_Y108,	( uchar* )g70_017_000_s_X108_Y108,	( uchar* )g70_017_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:ScanToBox.	
//	{ PNLE_TopMenuShortCut_ScanToPC	,			s_pITMScanFolder_W120H050D,						( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_018_000_n_X108_Y108,	( uchar* )g70_018_000_s_X108_Y108,	( uchar* )g70_018_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:ScanToPC.
//	{ PNLE_TopMenuShortCut_InternetFax,			s_pITMInternetFax_W120H050D,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_019_000_n_X108_Y108,	( uchar* )g70_019_000_s_X108_Y108,	( uchar* )g70_019_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:InternetFax.
//	{ PNLE_TopMenuShortCut_IPAddressFax,		s_pITMIPaddressFAX_W120H050D,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_020_000_n_X108_Y108,	( uchar* )g70_020_000_s_X108_Y108,	( uchar* )g70_020_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:IPAddressFax.
//	{ PNLE_TopMenuShortCut_FuncFaxOnly,			s_pITMG3FAX_W120H050D,							( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_021_000_n_X108_Y108,	( uchar* )g70_021_000_s_X108_Y108,	( uchar* )g70_021_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:ファクス.
//	{ PNLE_TopMenuShortCut_FuncBrowser,			s_pITMWebBrowser_W120H050D,						( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_022_000_n_X108_Y108,	( uchar* )g70_022_000_s_X108_Y108,	( uchar* )g70_022_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:ブラウザ.
//	{ PNLE_TopMenuShortCut_MarketPlaceSetup,	s_pITMMarketPlace_W120H050D,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_034_000_n_X108_Y108,	( uchar* )g70_034_000_s_X108_Y108,	( uchar* )g70_034_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:MarketPlace Setup Key.
//	{ PNLE_TopMenuShortCut_FuncOAPApp,			s_pITMApp_W120H050D,							( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_031_000_n_X108_Y108,	( uchar* )g70_031_000_s_X108_Y108,	( uchar* )g70_031_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:OpenAPIアプリ.
//	{ PNLE_TopMenuShortCut_AddressBook,			s_pITMAddressBook_W120H050D,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_027_000_n_X108_Y108,	( uchar* )g70_027_000_s_X108_Y108,	( uchar* )g70_027_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:アドレス帳.
//	{ PNLE_TopMenuShortCut_CopyPaperType,		s_pITMPaperSetting_W120H050D,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_029_000_n_X108_Y108,	( uchar* )g70_029_000_s_X108_Y108,	( uchar* )g70_029_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 用紙設定.
//	{ PNLE_TopMenuShortCut_SysBoxAuthPrint,		s_pITMAuthenticationAndGoBox_W120H050D,			( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_025_000_n_X108_Y108,	( uchar* )g70_025_000_s_X108_Y108,	( uchar* )g70_025_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 認証＆プリント.
//	{ PNLE_TopMenuShortCut_SysBoxExternalMemory,s_pITMUSBPrint_W120H050D,						( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_024_000_n_X108_Y108,	( uchar* )g70_024_000_s_X108_Y108,	( uchar* )g70_024_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 外部メモリー.
//	{ PNLE_TopMenuShortCut_UIModeLegacy,		s_pITMClassicStyle_W120H050D,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_023_000_n_X108_Y108,	( uchar* )g70_023_000_s_X108_Y108,	( uchar* )g70_023_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// UIモード切替.
//	{ PNLE_TopMenuShortCut_FuncUbqPrint,		s_pITMUbiquitousPrint_W120H050D,				( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_028_000_n_X108_Y108,	( uchar* )g70_028_000_s_X108_Y108,	( uchar* )g70_028_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:ユビキタスプリント.
//
//// comment 2018/07/31 Sky東島 [1056180] フォントサイズ追加対応.
////	{ PNLE_TopMenuShortCut_FuncCopy,			s_pITMCopy_W120H048E,							( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_015_000_n_X108_Y108,	( uchar* )g70_015_000_s_X108_Y108,	( uchar* )g70_015_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:コピー.
////	{ PNLE_TopMenuShortCut_ScanToEmail,			s_pITMScanToEmail_W120H048E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_016_000_n_X108_Y108,	( uchar* )g70_016_000_s_X108_Y108,	( uchar* )g70_016_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:ScanToE-mail.
////	{ PNLE_TopMenuShortCut_ScanToBox,			s_pITMScanToBox_W120H048E,						( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_017_000_n_X108_Y108,	( uchar* )g70_017_000_s_X108_Y108,	( uchar* )g70_017_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:ScanToBox.	
////	{ PNLE_TopMenuShortCut_ScanToPC	,			s_pITMScanToPC_W120H048E,						( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_018_000_n_X108_Y108,	( uchar* )g70_018_000_s_X108_Y108,	( uchar* )g70_018_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:ScanToPC.
////	{ PNLE_TopMenuShortCut_InternetFax,			s_pITMInternetFax_W120H048E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_019_000_n_X108_Y108,	( uchar* )g70_019_000_s_X108_Y108,	( uchar* )g70_019_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:InternetFax.
////	{ PNLE_TopMenuShortCut_IPAddressFax,		s_pITMIPAddressFax_W120H048E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_020_000_n_X108_Y108,	( uchar* )g70_020_000_s_X108_Y108,	( uchar* )g70_020_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:IPAddressFax.
////	{ PNLE_TopMenuShortCut_FuncFaxOnly,			s_pITMFax_W120H048E,							( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_021_000_n_X108_Y108,	( uchar* )g70_021_000_s_X108_Y108,	( uchar* )g70_021_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:ファクス.
////	{ PNLE_TopMenuShortCut_FuncBrowser,			s_pITMWebBrowser_W120H048E,						( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_022_000_n_X108_Y108,	( uchar* )g70_022_000_s_X108_Y108,	( uchar* )g70_022_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:ブラウザ.
//// Comment 2018/06/28 [HCL-Pavani] [IT6_Eagle] [AR1048863] Download and install MarketPlace on all MFPs according to the performance in LightUI.
////	{ PNLE_TopMenuShortCut_MarketPlace,			s_pITMMarketPlace_W120H048E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_026_000_n_X108_Y108,	( uchar* )g70_026_000_s_X108_Y108,	( uchar* )g70_026_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:MarketPlace download.
////	{ PNLE_TopMenuShortCut_MarketPlaceSetup,	s_pKEYMarketPlaceSetup_W110H042E,				( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )i70_043_000_n_X64_Y80,	( uchar* )i70_043_000_n_X64_Y80,	( uchar* )i70_043_000_d_X64_Y80,  	PNLD_PaletteNewUIApp},	// 機能:MarketPlace Setup Key.
//// comment 2018/05/24 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
////	{ PNLE_TopMenuShortCut_FuncOAPApp,			s_pITMAPP_W120H048E,							( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_031_000_n_X108_Y108,	( uchar* )g70_031_000_s_X108_Y108,	( uchar* )g70_031_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:OpenAPIアプリ.
////	{ PNLE_TopMenuShortCut_AddressBook,			s_pITMAddressBook_W120H048E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_027_000_n_X108_Y108,	( uchar* )g70_027_000_s_X108_Y108,	( uchar* )g70_027_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:アドレス帳.
////	{ PNLE_TopMenuShortCut_CopyPaperType,		s_pITMPaperSetting_W120H048E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_029_000_n_X108_Y108,	( uchar* )g70_029_000_s_X108_Y108,	( uchar* )g70_029_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 用紙設定.
////	{ PNLE_TopMenuShortCut_SysBoxAuthPrint,		s_pITMAuthenticationAndPrintBox_W120H048E,		( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_025_000_n_X108_Y108,	( uchar* )g70_025_000_s_X108_Y108,	( uchar* )g70_025_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 認証＆プリント.
////	{ PNLE_TopMenuShortCut_SysBoxExternalMemory,s_pITMExternalMemory_W120H048E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_024_000_n_X108_Y108,	( uchar* )g70_024_000_s_X108_Y108,	( uchar* )g70_024_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 外部メモリー.
////	{ PNLE_TopMenuShortCut_UIModeLegacy,		s_pITMClassicStyle_W120H048E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_023_000_n_X108_Y108,	( uchar* )g70_023_000_s_X108_Y108,	( uchar* )g70_023_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// UIモード切替.
////	{ PNLE_TopMenuShortCut_FuncUbqPrint,		s_pKEYActivityServerlessPullPrinting_W104H042E,	( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_028_000_n_X108_Y108,	( uchar* )g70_028_000_s_X108_Y108,	( uchar* )g70_028_000_d_X108_Y108,  PNLD_PaletteNewUIApp},	// 機能:ユビキタスプリント.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1】 Card copy setting support for DenebMLK model.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
//// comment 2018/11/09 Sky雜賀 [EagleH][RQ-272].
//	{ PNLE_TopMenuShortCut_IpFaxT38,			s_pITMIPFaxT38_W120H050D,						( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_041_000_n_X124_Y110,	( uchar* )g70_041_000_s_X124_Y110,	( uchar* )g70_041_000_d_X124_Y110,  PNLD_PaletteNewUIApp},	// 機能:IP-FAX(T.38).
//#endif	// #if defined( DEF_DenebMLKCus ).
//
//#else // 10.1inch
////	{ PNLE_TopMenuShortCut_FuncCopy,			s_pITMCopy_W144H042E,							( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_000_000_n_X124_Y124,	( uchar* )g70_000_000_s_X124_Y124,	( uchar* )g70_000_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:コピー.
////	{ PNLE_TopMenuShortCut_ScanToEmail,			s_pITMScanToEmail_W144H042E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_001_000_n_X124_Y124,	( uchar* )g70_001_000_s_X124_Y124,	( uchar* )g70_001_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:ScanToE-mail.
////	{ PNLE_TopMenuShortCut_ScanToBox,			s_pITMScanToBox_W144H042E,						( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_002_000_n_X124_Y124,	( uchar* )g70_002_000_s_X124_Y124,	( uchar* )g70_002_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:ScanToBox.	
////	{ PNLE_TopMenuShortCut_ScanToPC	,			s_pITMScanToPC_W144H042E,						( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_003_000_n_X124_Y124,	( uchar* )g70_003_000_s_X124_Y124,	( uchar* )g70_003_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:ScanToPC.
////	{ PNLE_TopMenuShortCut_InternetFax,			s_pITMInternetFax_W144H042E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_004_000_n_X124_Y124,	( uchar* )g70_004_000_s_X124_Y124,	( uchar* )g70_004_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:InternetFax.
////	{ PNLE_TopMenuShortCut_IPAddressFax,		s_pITMIPAddressFax_W144H042E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_005_000_n_X124_Y124,	( uchar* )g70_005_000_s_X124_Y124,	( uchar* )g70_005_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:IPAddressFax.
////	{ PNLE_TopMenuShortCut_FuncFaxOnly,			s_pITMFax_W144H042E,							( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_006_000_n_X124_Y124,	( uchar* )g70_006_000_s_X124_Y124,	( uchar* )g70_006_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:ファクス.
////	{ PNLE_TopMenuShortCut_FuncBrowser,			s_pITMWebBrowser_W144H042E,						( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_007_000_n_X124_Y124,	( uchar* )g70_007_000_s_X124_Y124,	( uchar* )g70_007_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:ブラウザ.
////// Comment 2018/06/28 [HCL-Pavani] [IT6_Eagle] [AR1048863] Download and install MarketPlace on all MFPs according to the performance in LightUI.
//////	{ PNLE_TopMenuShortCut_MarketPlace,			s_pITMMarketPlace_W144H042E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_011_000_n_X124_Y124,	( uchar* )g70_011_000_s_X124_Y124,	( uchar* )g70_011_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:MarketPlace download.
////#ifdef DEF_RES_WSVGA
////	{ PNLE_TopMenuShortCut_MarketPlaceSetup,	s_pKEYMarketPlaceSetup_W110H042E,				( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )i75_043_000_n_X81_Y100,	( uchar* )i75_043_000_n_X81_Y100,	( uchar* )i75_043_000_d_X81_Y100,  PNLD_PaletteNewUIApp},	// 機能:MarketPlace Setup Key.
////#else
////// comment 2018/8/7 SCD2 K.Nishiki[AR1049506].
////	{ PNLE_TopMenuShortCut_MarketPlaceSetup,	s_pKEYMarketPlaceSetup_W110H042E,				( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_036_000_n_X124_Y124,	( uchar* )g70_036_000_s_X124_Y124,	( uchar* )g70_036_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:MarketPlace Setup Key.
////#endif
////// comment 2018/05/24 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
////	{ PNLE_TopMenuShortCut_FuncOAPApp,			s_pITMAPP_W144H042E,							( uchar* )i70_035_000_n_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_035_000_d_X64_Y80, PNLD_PaletteCommon ,	( uchar* )g70_030_000_n_X124_Y124,	( uchar* )g70_030_000_s_X124_Y124,	( uchar* )g70_030_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:OpenAPIアプリ.
////	{ PNLE_TopMenuShortCut_AddressBook,			s_pITMAddressBook_W144H042E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_012_000_n_X124_Y124,	( uchar* )g70_012_000_s_X124_Y124,	( uchar* )g70_012_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:アドレス帳.
////	{ PNLE_TopMenuShortCut_CopyPaperType,		s_pITMPaperSetting_W144H042E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_014_000_n_X124_Y124,	( uchar* )g70_014_000_s_X124_Y124,	( uchar* )g70_014_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 用紙設定.
////	{ PNLE_TopMenuShortCut_SysBoxAuthPrint,		s_pITMAuthenticationAndPrintBox_W144H042E,		( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_010_000_n_X124_Y124,	( uchar* )g70_010_000_s_X124_Y124,	( uchar* )g70_010_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 認証＆プリント.
////	{ PNLE_TopMenuShortCut_SysBoxExternalMemory,s_pITMExternalMemory_W144H042E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_009_000_n_X124_Y124,	( uchar* )g70_009_000_s_X124_Y124,	( uchar* )g70_009_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 外部メモリー.
////	{ PNLE_TopMenuShortCut_UIModeLegacy,		s_pITMClassicStyle_W144H042E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_008_000_n_X124_Y124,	( uchar* )g70_008_000_s_X124_Y124,	( uchar* )g70_008_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// UIモード切替.
////	{ PNLE_TopMenuShortCut_FuncUbqPrint,		s_pKEYActivityServerlessPullPrinting_W104H042E,	( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_013_000_n_X124_Y124,	( uchar* )g70_013_000_s_X124_Y124,	( uchar* )g70_013_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:ユビキタスプリント.
//	{ PNLE_TopMenuShortCut_FuncCopy,			s_pITMCopy1_W144H042E,							( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_000_000_n_X124_Y124,	( uchar* )g70_000_000_s_X124_Y124,	( uchar* )g70_000_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:コピー.
//	{ PNLE_TopMenuShortCut_ScanToEmail,			s_pITMScanToEmail1_W144H042E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_001_000_n_X124_Y124,	( uchar* )g70_001_000_s_X124_Y124,	( uchar* )g70_001_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:ScanToE-mail.
//	{ PNLE_TopMenuShortCut_ScanToBox,			s_pITMScanToBox1_W144H042E,						( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_002_000_n_X124_Y124,	( uchar* )g70_002_000_s_X124_Y124,	( uchar* )g70_002_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:ScanToBox.	
//	{ PNLE_TopMenuShortCut_ScanToPC	,			s_pITMScanFolder_W144H042E,						( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_003_000_n_X124_Y124,	( uchar* )g70_003_000_s_X124_Y124,	( uchar* )g70_003_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:ScanToPC.
//	{ PNLE_TopMenuShortCut_InternetFax,			s_pITMInternetFax1_W144H042E,							( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_004_000_n_X124_Y124,	( uchar* )g70_004_000_s_X124_Y124,	( uchar* )g70_004_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:InternetFax.
//	{ PNLE_TopMenuShortCut_IPAddressFax,		s_pITMIPaddressFAX1_W144H042E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_005_000_n_X124_Y124,	( uchar* )g70_005_000_s_X124_Y124,	( uchar* )g70_005_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:IPAddressFax.
//	{ PNLE_TopMenuShortCut_FuncFaxOnly,			s_pITMG3FAX_W144H042E,							( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_006_000_n_X124_Y124,	( uchar* )g70_006_000_s_X124_Y124,	( uchar* )g70_006_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:ファクス.
//	{ PNLE_TopMenuShortCut_FuncBrowser,			s_pITMWebBrowser1_W144H042E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_007_000_n_X124_Y124,	( uchar* )g70_007_000_s_X124_Y124,	( uchar* )g70_007_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:ブラウザ.
//#ifdef DEF_RES_WSVGA
////	{ PNLE_TopMenuShortCut_MarketPlaceSetup,	s_pKEYMarketPlaceSetup_W110H042E,				( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )i75_043_000_n_X81_Y100,	( uchar* )i75_043_000_n_X81_Y100,	( uchar* )i75_043_000_d_X81_Y100,  PNLD_PaletteNewUIApp},	// 機能:MarketPlace Setup Key.
//	{ PNLE_TopMenuShortCut_MarketPlaceSetup,	s_pITMMarketPlace1_W144H042E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )i75_043_000_n_X81_Y100,	( uchar* )i75_043_000_n_X81_Y100,	( uchar* )i75_043_000_d_X81_Y100,  PNLD_PaletteNewUIApp},	// 機能:MarketPlace Setup Key.
//#else
////	{ PNLE_TopMenuShortCut_MarketPlaceSetup,	s_pKEYMarketPlaceSetup_W110H042E,				( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )i70_043_000_n_X64_Y80,	( uchar* )i70_043_000_n_X64_Y80,	( uchar* )i70_043_000_d_X64_Y80,  PNLD_PaletteNewUIApp},	// 機能:MarketPlace Setup Key.
//	{ PNLE_TopMenuShortCut_MarketPlaceSetup,	s_pITMMarketPlace1_W144H042E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_036_000_n_X124_Y124,	( uchar* )g70_036_000_s_X124_Y124,	( uchar* )g70_036_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:MarketPlace Setup Key.
//#endif
//	{ PNLE_TopMenuShortCut_FuncOAPApp,			s_pITMApp1_W144H042E,							( uchar* )i70_035_000_n_X64_Y80, PNLD_PaletteCommon, ( uchar* )i70_035_000_d_X64_Y80, PNLD_PaletteCommon ,	( uchar* )g70_030_000_n_X124_Y124,	( uchar* )g70_030_000_s_X124_Y124,	( uchar* )g70_030_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:OpenAPIアプリ.
//	{ PNLE_TopMenuShortCut_AddressBook,			s_pITMAddressBook1_W144H042E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_012_000_n_X124_Y124,	( uchar* )g70_012_000_s_X124_Y124,	( uchar* )g70_012_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:アドレス帳.
//	{ PNLE_TopMenuShortCut_CopyPaperType,		s_pITMPaperSetting1_W144H042E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_014_000_n_X124_Y124,	( uchar* )g70_014_000_s_X124_Y124,	( uchar* )g70_014_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 用紙設定.
//	{ PNLE_TopMenuShortCut_SysBoxAuthPrint,		s_pITMAuthenticationAndGoBox_W144H042E,			( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_010_000_n_X124_Y124,	( uchar* )g70_010_000_s_X124_Y124,	( uchar* )g70_010_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 認証＆プリント.
//	{ PNLE_TopMenuShortCut_SysBoxExternalMemory,s_pITMUSBPrint_W144H042E,						( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_009_000_n_X124_Y124,	( uchar* )g70_009_000_s_X124_Y124,	( uchar* )g70_009_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 外部メモリー.
//	{ PNLE_TopMenuShortCut_UIModeLegacy,		s_pITMClassicStyle1_W144H042E,					( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_008_000_n_X124_Y124,	( uchar* )g70_008_000_s_X124_Y124,	( uchar* )g70_008_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// UIモード切替.
//	{ PNLE_TopMenuShortCut_FuncUbqPrint,		s_pITMUbiquitousPrint1_W144H042E,				( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_013_000_n_X124_Y124,	( uchar* )g70_013_000_s_X124_Y124,	( uchar* )g70_013_000_d_X124_Y124,  PNLD_PaletteNewUIApp},	// 機能:ユビキタスプリント.
//// Comment 2018/11/19 【HCLT-Prashannaa】【IT6_DenebMLK】【制御構成変更_No.83】
//#ifdef  DEF_DenebMLKCus 
//	{ PNLE_TopMenuShortCut_SysBoxSecurity,		s_pKEYSecurityDocument_W100H042E,				( uchar* )NULL,  0,	( uchar* )NULL,  0,	( uchar* )g70_024_000_n_X108_Y108,						( uchar* )g70_024_000_s_X108_Y108,					( uchar* )g70_024_000_d_X108_Y108,  		 		PNLD_PaletteNewUIApp},	// Secureprint
//#endif7
//// comment 2018/11/09 Sky雜賀 [EagleH][RQ-272].
//	{ PNLE_TopMenuShortCut_IpFaxT38,			s_pITMIPFaxT38_W144H042E,
//#endif	// #if defined(DEF_LCD_7INCH).
//};
////================================================================================================.
//// 概要 : setBadgeKeyData_HomeMenu :  トップメニュー（統合トップメニュー） ： バッジキーに表示する情報の設定.（LightUI用）.
////================================================================================================.
//// comment 2018/04/06 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
//void PCGC_HomeTopMenu::setBadgeKeyData_HomeMenu()
//{
//	TYPE_MachineType e_MachineType = PnlComGetSystemInfo()->getMachineType();
//
//	TYPS_ExtShortCutListLightUI _st_BadgeKeyListLightUI;
//	memset( &_st_BadgeKeyListLightUI, 0, sizeof( _st_BadgeKeyListLightUI ) );
//
//	// 正規処理.
//	PnlComGetStandardPanelSetting()->getShortCutListSettingLightUI(&_st_BadgeKeyListLightUI);
//	
//	memset( &st_BadgeKeyInfo, 0x00, sizeof( st_BadgeKeyInfo ) );
//	char _sc_select = -1;
//
//	for( uchar i = 0; i < _st_BadgeKeyListLightUI.ss_ArraySize; i++ ) {
//
//		TYPE_TopMenuShortCutType				_e_ShortCutType ;
//		TYPE_TopMenuShortCutFunction			_e_FunctionType;
//// comment 2018/05/24 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
//		char									sc_OpenAPIApplicationNameUTF8[TYPD_ExtMaxAplName+1];
//		unsigned short							us_ApplicationButtonShortCutIcon;
//		TYPE_TopMenuShortCutGroupKey			_e_TopMenuShortCutGroupKey=0;
//		_e_ShortCutType = _st_BadgeKeyListLightUI.s_ShortCutKey[i].e_ShortCutType;
//		_e_FunctionType = _st_BadgeKeyListLightUI.s_ShortCutKey[i].e_FunctionType;
//// Comment 2018/12/11 【HCLT-Prem】 【IT6_DenebMLK】 【AR1068595】 Non Image Area Erase Display support.
//		TYPE_TopMenuShortCutCopyFunction		_e_CopyFunctionType;
//		_e_CopyFunctionType = _st_BadgeKeyListLightUI.s_ShortCutKey[i].e_CopyFunctionType;
//// comment 2018/05/24 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
//		strncpy( sc_OpenAPIApplicationNameUTF8, _st_BadgeKeyListLightUI.s_ShortCutKey[i].sc_OpenAPIApplicationNameUTF8, sizeof( sc_OpenAPIApplicationNameUTF8 )-1 );
//		us_ApplicationButtonShortCutIcon =  _st_BadgeKeyListLightUI.s_ShortCutKey[i].us_ApplicationButtonShortCutIcon;
//		_e_TopMenuShortCutGroupKey = _st_BadgeKeyListLightUI.s_ShortCutKey[i].e_GroupKeyType;
//
//		switch( _e_ShortCutType ) {
//			case TYPE_TopMenuShortCutTypeNone:				// 未設定.
//				break;
//			case TYPE_TopMenuShortCutTypeFunction:			// 機能.
//				setFunctionInfo_HomeMenu( i, _e_FunctionType );		// 機能情報の設定.	コピー,ScantoEmail,ScantoBox,ScantoPC,インターネットFAX,IPアドレスFAX,ファクス,Webブラウザー,App(OpenAPI).
//				break;
//// comment 2018/05/24 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
//			case TYPE_TopMenuShortCutTypeAppShortCut:		// OpenAPIアプリショートカット.
//			case TYPE_TopMenuShortCutTypeAppListShortCut:	// OpenAPIアプリリスト.
//// Comment 2018/10/08 [HCL-Pavani] [IT6_Eagle] [AR1060058] MarketPlace Application Key Display/Non-display check.
//				{
//					// 指定アプリMFP基本機能設定禁止の場合.
//					if( False == c_ExtPanel.isEnableMFPFunction() ) {
//						st_BadgeKeyInfo[i].bl_isShow = False;
//						break;
//					}
//					if( 0 < us_ApplicationButtonShortCutIcon )	{ us_ApplicationButtonShortCutIcon--; }
//					setAppFunctionInfo( i, _e_ShortCutType, sc_OpenAPIApplicationNameUTF8, us_ApplicationButtonShortCutIcon );
//// Comment 2018/10/08 [HCL-Pavani] [IT6_Eagle] [AR1060058] MarketPlace Application Key ICON correction.
////					st_BadgeKeyInfo[i].puc_FuncIcon_n = st_TopMenuBadgeKeyFuncTbl2[9].puc_FuncIcon_n;
////					st_BadgeKeyInfo[i].puc_FuncIcon_s = st_TopMenuBadgeKeyFuncTbl2[9].puc_FuncIcon_s;
////					st_BadgeKeyInfo[i].puc_FuncIcon_d = st_TopMenuBadgeKeyFuncTbl2[9].puc_FuncIcon_d;
////					st_BadgeKeyInfo[i].uc_IconPaletteLightUI = st_TopMenuBadgeKeyFuncTbl2[9].uc_IconPaletteLightUI;
//					// PKI認証設定時かつセキュリティー強化設定有効の場合、キーを非表示.
//					if( ( True == PLBC_FuncPKICheck::isPKI() ) && ( True == PnlComGetSystemSetting()->getSecurityLevelUpOn() ) ) {
//						st_BadgeKeyInfo[i].bl_isShow = False;
//					}
//					if(TYPE_MachineTypeSparrow2SFP == e_MachineType) {
//						st_BadgeKeyInfo[i].bl_isShow = False;
//					}
//// Comment 2018/10/08 [HCL-Pavani] [IT6_Eagle] [AR1060058] MarketPlace Application Key Display/Non-display check.
//					char c_OapAppName[TYPD_ExtMaxAplName*3+1] = {0,};
//					char sc_AppName[TYPD_ExtMaxAplName*3+1] = PNLD_MarketPlaceString;
//					PnlComGetUTF8Str( sc_OpenAPIApplicationNameUTF8, ( LIBE_CharSet )getCharSetForCAPI(), c_OapAppName, sizeof( c_OapAppName ) );
//					if ( 0 == strcmp( sc_AppName, c_OapAppName ) ) {
//#if defined(DEF_LCD_7INCH)
//						st_BadgeKeyInfo[i].puc_FuncIcon_n = ( uchar* )g70_026_000_n_X108_Y108;
//						st_BadgeKeyInfo[i].puc_FuncIcon_s = ( uchar* )g70_026_000_s_X108_Y108;
//						st_BadgeKeyInfo[i].puc_FuncIcon_d = ( uchar* )g70_026_000_d_X108_Y108;
//#else // 10.1inch
//						st_BadgeKeyInfo[i].puc_FuncIcon_n = ( uchar* )g70_011_000_n_X124_Y124;
//						st_BadgeKeyInfo[i].puc_FuncIcon_s = ( uchar* )g70_011_000_s_X124_Y124;
//						st_BadgeKeyInfo[i].puc_FuncIcon_d = ( uchar* )g70_011_000_d_X124_Y124;
//#endif  // DEF_LCD_7INCH
//						st_BadgeKeyInfo[i].uc_IconPaletteLightUI = PNLD_PaletteNewUIApp;
//						if ( False == PnlComGetIWSSetting()->getMarketPlaceAppUse()){
//							st_BadgeKeyInfo[i].bl_isShow = False;
//						}
//					} else {
//						st_BadgeKeyInfo[i].puc_FuncIcon_n = st_TopMenuBadgeKeyFuncTbl2[9].puc_FuncIcon_n;
//						st_BadgeKeyInfo[i].puc_FuncIcon_s = st_TopMenuBadgeKeyFuncTbl2[9].puc_FuncIcon_s;
//						st_BadgeKeyInfo[i].puc_FuncIcon_d = st_TopMenuBadgeKeyFuncTbl2[9].puc_FuncIcon_d;
//						st_BadgeKeyInfo[i].uc_IconPaletteLightUI = st_TopMenuBadgeKeyFuncTbl2[9].uc_IconPaletteLightUI;
//						
//						// comment 2018/11/14 SCD2 K.Seto [AR.1067363]表示対象となるアプリがMarketPlaceアプリのみの場合、APPキーを非表示とする.
//						PNLC_ActionDataLib* pc_ActionDataLib = PNLC_ActionDataLib::newInstance();
//						if ( NULL != pc_ActionDataLib ) {
//							// OAPアプリの数をカウントする.
//							uchar uc_OAPAppCnt = 0;
//							uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeScanD );
//							uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeScanS );
//							uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypePullPrint );
//							uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeBrowserScan );
//							uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeBrowserLauncher );
//							uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeBrowserPull );
//							uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeBrowserGeneral );
//							uc_OAPAppCnt += ExtSet.countApps();
//							// comment 2018/12/6 Chikuan [AR1067363][EagleL] MarketPlaceのチェック条件を修正.
////							if ( True == pc_ActionDataLib->isInstalledMarketPlaceApp() ) {
//							Bool bl_ExistMarketPlaceWithIWSOff = checkExistMarketPlaceWithIWSOff();
//							if ( ( True == pc_ActionDataLib->isInstalledMarketPlaceApp() ) || ( True == bl_ExistMarketPlaceWithIWSOff ) ) {
//								// MarketPlaceアプリはBrowserLauncherでカウントされてしまうため、インストールされている場合は1引いておく.
//								uc_OAPAppCnt = uc_OAPAppCnt - 1;
//							}
//
//							// comment 2018/12/6 Chikuan [AR1067363][EagleL] APPキーの表示/非表示条件を変更.
////							if ( ( True == ScanOapExtSet.isRegistApplication() ) && ( 0 == uc_OAPAppCnt ) && ( True == pc_ActionDataLib->isInstalledMarketPlaceApp() ) ) {
//							if ( ( True == ScanOapExtSet.isRegistApplication() ) && ( 0 == uc_OAPAppCnt ) && ( ( True == pc_ActionDataLib->isInstalledMarketPlaceApp() ) || ( True == bl_ExistMarketPlaceWithIWSOff ) ) ) {
//								// OAPアプリは未インストール、かつMarketPlaceアプリがインストール済み.
//								// IWSアプリのチェック.
//								APIC_IWSAppInfo* pc_IWSAppInfo = APIC_IWSAppInfo::newInstance();
//								if ( NULL != pc_IWSAppInfo ) {
//									// IWSアプリ数を取得.
//									uchar uc_IWSAppCount = ( uchar )pc_IWSAppInfo->getApplicationNum();
//									if ( uc_IWSAppCount > 1 ) {
//										// アプリ数が1より大きい、即ちMarketPlaceアプリ以外のIWSアプリもインストールされている.
//										uchar uc_IwsAppTypeGeneralCnt = 0;
//										TYPS_IwsAppInfo s_tempAppInfo;
//										memset( &s_tempAppInfo, '\0', sizeof( s_tempAppInfo ) );
//										// インストールされているIWSアプリのアプリタイプを取得.
//										for( uchar uc_Cnt = 0; uc_Cnt < uc_IWSAppCount; uc_Cnt++ ) {
//											int si_Ret = pc_IWSAppInfo->getApplication( (int)( uc_Cnt+1 ), s_tempAppInfo );
//											if ( ( TYPD_IwsResult_OK == si_Ret )
//											  && ( TYPE_IwsAppTypeGeneral == s_tempAppInfo.e_AppType ) ) {
//												// アプリタイプがTYPE_IwsAppTypeGeneralのものは表示対象なのでカウント.
//												uc_IwsAppTypeGeneralCnt++;
//											}
//										}
//										if ( 1 == uc_IwsAppTypeGeneralCnt ) {
//											// 表示対象のアプリが1、即ちMarketPlaceアプリのみのため、APPキーも表示不要.
//											st_BadgeKeyInfo[i].bl_isShow = False;
//										}
//									} else {
//										// アプリ数が1、即ちMarketPlaceアプリのみのため、APPキーも表示不要.
//										st_BadgeKeyInfo[i].bl_isShow = False;
//									}
//								}
//							}
//						}
//					}
//				}
//				break;
//// comment 2018/05/24 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
//			case TYPE_TopMenuShortCutTypeGroupKey:			// グループアプリケーション.
//				if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
//					setAppGrpInfo( i, _e_TopMenuShortCutGroupKey );		// グループアプリケーション情報の設定.
//					st_BadgeKeyInfo[i].puc_FuncIcon_n = st_TopMenuBadgeKeyFuncTbl2[9].puc_FuncIcon_n;
//					st_BadgeKeyInfo[i].puc_FuncIcon_s = st_TopMenuBadgeKeyFuncTbl2[9].puc_FuncIcon_s;
//					st_BadgeKeyInfo[i].puc_FuncIcon_d = st_TopMenuBadgeKeyFuncTbl2[9].puc_FuncIcon_d;
//					st_BadgeKeyInfo[i].uc_IconPaletteLightUI = st_TopMenuBadgeKeyFuncTbl2[9].uc_IconPaletteLightUI;
//				} else {
//					st_BadgeKeyInfo[i].bl_isShow = False;
//				}
//				if(TYPE_MachineTypeSparrow2SFP == e_MachineType) {
//					st_BadgeKeyInfo[i].bl_isShow = False;
//				}
//				break;
//
//			case TYPE_TopMenuShortCutTypeCopyFunction:		// 用紙.
//// Comment 2018/12/11 【HCLT-Prem】 【IT6_DenebMLK】 【AR1068595】 Non Image Area Erase Display support.
//#ifdef DEF_DenebMLKCus 
//				if ( TYPE_TopMenuShortCutCopyFunctionNoneImageAreaErase == _e_CopyFunctionType ){
//					_sc_select = 1;
//					st_BadgeKeyInfo[i].bl_isShow = True;
//				}
//#endif // DEF_DenebMLKCus.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】 Card copy and Non Image Area Erase settingsupport for DenebMLK model.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】 Card copy and Non Image Area Erase settingsupport for DenebMLK model.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
////#ifndef DEF_DenebMLKCus 
////				_sc_select = 11;
////#else
////				if ( TYPE_TopMenuShortCutCopyFunctionPaperType == _e_FunctionType ){
////					_sc_select = 9;
////					st_BadgeKeyInfo[( uchar )_sc_select].bl_isShow = True;
////				} else if(TYPE_TopMenuShortCutCopyFunctionSkyShot == _e_FunctionType ){
////					_sc_select = 1;
////					st_BadgeKeyInfo[( uchar )_sc_select].bl_isShow = True;
////				} else if(TYPE_TopMenuShortCutCopyFunctionCardCopy == _e_FunctionType ){
////					_sc_select = 2;
////					if ( ( True == PnlComGetSoftDipSw()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW153, 0 ) ) )	{//Dip SW 153 bit 0 - set.
////						st_BadgeKeyInfo[( uchar )_sc_select].bl_isShow = True;
////					}else {
////						st_BadgeKeyInfo[( uchar )_sc_select].bl_isShow = False;
////					}
////				}
////#endif // DEF_DenebMLKCus.
//				break;
//// comment 2018/10/23 [HCLT_Prem][DenebMLK] [AR1050427] Card Copy Support.
//			case TYPE_TopMenuShortCutTypeCopyMemory:
//				if ( ( 30 == _st_BadgeKeyListLightUI.s_ShortCutKey[i].ss_CopyProgramMemoryNo )
//// comment 2018/10/25 [HCL_Sai][DenebMLK] Build Error Support.
//				&& ( ( True == PnlComGetSoftDipSw()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW153, 0 ) ) ) ){
//					_sc_select = 2;
//					st_BadgeKeyInfo[i].bl_isShow = True;
//				}
//				break;
//			case TYPE_TopMenuShortCutTypeSystemBox:
//				// 認証プリント. 
//				if(TYPE_TopMenuShortCutSystemBoxFunctionAuthPrint == _e_FunctionType) {
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1】 Card copy setting support for DenebMLK model.
//#ifdef DEF_DenebMLKCus 
//					_sc_select = 14;
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
////					_sc_select = 12;
//#else // DEF_DenebMLKCus.
//					_sc_select = 12;
//#endif // DEF_DenebMLKCus.
//					// comment : 20180508 Chikuan : [Eagle] [KRQ-314-28] 【新UI】認証＆プリントBox対応.
//					if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//						st_BadgeKeyInfo[( uchar )_sc_select].bl_isShow = False;
//					}else{
//						st_BadgeKeyInfo[( uchar )_sc_select].bl_isShow = isFuncShowHomeMenu( PNLE_ACCOUNT_STATUS_TYPE_BoxOperation, PNLE_TopMenuShortCut_SysBoxAuthPrint );
//					}
//				}
//				// 外部メモリー印刷.
//				else if(TYPE_TopMenuShortCutSystemBoxFunctionExternalMemory == _e_FunctionType) {
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1】 Card copy setting support for DenebMLK model.
//#ifdef DEF_DenebMLKCus 
//					_sc_select = 15;
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
////					_sc_select = 13;
//#else // DEF_DenebMLKCus.
//					_sc_select = 13;
//#endif // DEF_DenebMLKCus.
//					st_BadgeKeyInfo[( uchar )_sc_select].bl_isShow = True;
//				}
//				break;
//			
//			case TYPE_TopMenuShortCutTypeAddressBook:		// アドレス帳.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1】 Card copy setting support for DenebMLK model.
//#ifdef DEF_DenebMLKCus 
//				_sc_select = 8;
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
////				_sc_select = 10;
//#else // DEF_DenebMLKCus.
//				_sc_select = 10;
//#endif // DEF_DenebMLKCus.
//				st_BadgeKeyInfo[i].bl_isShow = True;
//				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//					st_BadgeKeyInfo[i].bl_isShow = False;
//				}
//// comment Sky森本（弘） 2018/10/16 [Eagle][問題点_No.1064071] ユーザ認証及び部門認証が未認証時の対応.
//				if ( NULL == PnlComGetUser() ) {												// ユーザ認証ユーザなしの場合.
//					st_BadgeKeyInfo[i].bl_isShow = False;
//				} else if ( ( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode(PNLC_NetworkLan::getInstance()->getLine()) )	// 部門認証管理する.
//						&&	( NULL == PnlComGetTrack() ) ) {									// 部門認証なし.
//					st_BadgeKeyInfo[i].bl_isShow = False;
//				}
//				break;
//				
//			case TYPE_TopMenuShortCutTypeMarketPlaceDownloader:    // bizhub Market Place ダウンローダー.
//				_sc_select = 8;
//				st_BadgeKeyInfo[i].bl_isShow = False;
//				// 以下の条件にすべて一致した場合に限り表示.
//					// DipSW172が0x00または0x02.
//				if( ( 0x01 != getSwDipSwitch( DEFSOFTWARESW172 ) ) &&
//					// Webブラウザー使用する.
//				    ( True == PnlComGetBrowserSetting()->getEnableBrowser() ) &&
//					// IWS使用する.
//					( True == PnlComGetIWSSetting()->getEnable() ) ) {
//					if ( True == PLBC_FuncVersionCheck::isOK_IT5_Type11() ) {
//						st_BadgeKeyInfo[i].bl_isShow = True;
//					}
//					else {
//						st_BadgeKeyInfo[i].bl_isShow = False;
//					}
//				}
//				if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//					st_BadgeKeyInfo[i].bl_isShow = False;
//				}
//				break;
//// Comment 2018/06/28 [HCL-Pavani] [IT6_Eagle] [AR1048863] Download and install MarketPlace on all MFPs according to the performance in LightUI.
//			case TYPE_TopMenuShortCutTypeMarketPlaceSetup:
//				{
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】 Card copy and Non Image Area Erase settingsupport for DenebMLK model.
//#ifdef DEF_DenebMLKCus 
//				_sc_select = 7;
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
////				_sc_select = 8;
//#else // DEF_DenebMLKCus.
//				_sc_select = 8;
//#endif // DEF_DenebMLKCus.
//					Bool b_MarketPlaceInstalled = False;
//					PNLC_ActionDataLib* pc_ActionDataLib = PNLC_ActionDataLib::newInstance();
//					if( NULL != pc_ActionDataLib ) {
//					b_MarketPlaceInstalled = pc_ActionDataLib->isInstalledMarketPlaceApp();
//					}
//					if ( False == b_MarketPlaceInstalled ){
//						if ( ( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW172 , 2 ) ) 
//// comment 2018/09/12 【IT6_Eagle】【HCLT_Pavani】【AR1053803】Support for Market Place Application Failure case.
////						&& ( False == PnlComGetIWSSetting()->getEnable() ) 
//						&& ( True  == APIC_SystemInfo::newInstance()->isOwnOrGeneric() )
//						&& ( False == APIC_SystemInfo::newInstance()->isCardLineMode() ) 
//						&& ( True == PnlComGetIWSSetting()->getMarketPlaceAppUse()) ){
//// comment 2018/8/7 SCD2 K.Nishiki[AR1049506].
//							st_BadgeKeyInfo[i].bl_isShow = True;
//						} else {
//// comment 2018/8/7 SCD2 K.Nishiki[AR1049506].
//							st_BadgeKeyInfo[i].bl_isShow = False;
//						}
//// Comment 2018/10/08 [HCL-Pavani] [IT6_Eagle] [AR1060058] MarketPlace Application Not Applicable implementation commented.
////					} else {
//// comment 2018/8/7 SCD2 K.Nishiki[AR1049506].
////						st_BadgeKeyInfo[i].e_ShortCutKind = PNLE_TopMenuShortCut_MarketPlaceApplication;
////						st_BadgeKeyInfo[i].tid_FuncName = s_pKEYMarketPlace_W110H042E;				// Market place application Key Name.
////						st_BadgeKeyInfo[i].puc_FuncIcon = ( uchar* )NULL;
////						st_BadgeKeyInfo[i].ss_FuncIconDataSize = 0;
////						st_BadgeKeyInfo[i].uc_IconPaletteType = 0;
////						st_BadgeKeyInfo[i].puc_IconNotActivate = ( uchar* )NULL;
////						st_BadgeKeyInfo[i].ss_IconNotActivateDataSize = 0;
////						st_BadgeKeyInfo[i].uc_IconPaletteNotActivate = 0;
////						st_BadgeKeyInfo[i].ss_ProgramNo = 0;
////#if defined(DEF_LCD_7INCH)
////						st_BadgeKeyInfo[i].puc_FuncIcon_n = ( uchar* )g70_026_000_n_X108_Y108;
////						st_BadgeKeyInfo[i].puc_FuncIcon_s = ( uchar* )g70_026_000_s_X108_Y108;
////						st_BadgeKeyInfo[i].puc_FuncIcon_d = ( uchar* )g70_026_000_d_X108_Y108;
////#else // 10.1inch
////						st_BadgeKeyInfo[i].puc_FuncIcon_n = ( uchar* )g70_011_000_n_X124_Y124;
////						st_BadgeKeyInfo[i].puc_FuncIcon_s = ( uchar* )g70_011_000_s_X124_Y124;
////						st_BadgeKeyInfo[i].puc_FuncIcon_d = ( uchar* )g70_011_000_d_X124_Y124;
////#endif  // DEF_LCD_7INCH
////						st_BadgeKeyInfo[i].uc_IconPaletteLightUI = PNLD_PaletteNewUIApp;
////						st_BadgeKeyInfo[i].bl_isShow = True;
////						_sc_select = -1;
//					}
//				}
//				break;
//				
//			default:	break;
//		}
//		
//		if (-1 != _sc_select) {
//// comment 2018/05/24 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
//			st_BadgeKeyInfo[i].e_ShortCutKind = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].e_ShortCutKind;
//			st_BadgeKeyInfo[i].tid_FuncName = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].tid_FuncName;
//			st_BadgeKeyInfo[i].puc_FuncIcon = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].puc_FuncIcon;
//			st_BadgeKeyInfo[i].ss_FuncIconDataSize = 0;
//			st_BadgeKeyInfo[i].uc_IconPaletteType = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].uc_IconPaletteType;
//			st_BadgeKeyInfo[i].puc_IconNotActivate = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].puc_IconNotActivate;
//			st_BadgeKeyInfo[i].ss_IconNotActivateDataSize = 0;
//			st_BadgeKeyInfo[i].uc_IconPaletteNotActivate = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].uc_IconPaletteNotActivate;
//			st_BadgeKeyInfo[i].ss_ProgramNo = 0;
//			st_BadgeKeyInfo[i].puc_FuncIcon_n = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].puc_FuncIcon_n;
//			st_BadgeKeyInfo[i].puc_FuncIcon_s = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].puc_FuncIcon_s;
//			st_BadgeKeyInfo[i].puc_FuncIcon_d = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].puc_FuncIcon_d;
//			st_BadgeKeyInfo[i].uc_IconPaletteLightUI = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].uc_IconPaletteLightUI;
//			_sc_select = -1;
//		}
//	}
//#if 1	// comment 2018/06/05 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 新UIカウンタ画面正式対応時に削除 Start.
//	if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//		st_BadgeKeyInfo[7].e_ShortCutKind = st_TopMenuBadgeKeyFuncTbl2[14].e_ShortCutKind;
//		st_BadgeKeyInfo[7].tid_FuncName = st_TopMenuBadgeKeyFuncTbl2[14].tid_FuncName;
//		st_BadgeKeyInfo[7].puc_FuncIcon = st_TopMenuBadgeKeyFuncTbl2[14].puc_FuncIcon;
//		st_BadgeKeyInfo[7].ss_FuncIconDataSize = 0;
//		st_BadgeKeyInfo[7].uc_IconPaletteType = st_TopMenuBadgeKeyFuncTbl2[14].uc_IconPaletteType;
//		st_BadgeKeyInfo[7].puc_IconNotActivate = st_TopMenuBadgeKeyFuncTbl2[14].puc_IconNotActivate;
//		st_BadgeKeyInfo[7].ss_IconNotActivateDataSize = 0;
//		st_BadgeKeyInfo[7].uc_IconPaletteNotActivate = st_TopMenuBadgeKeyFuncTbl2[14].uc_IconPaletteNotActivate;
//		st_BadgeKeyInfo[7].ss_ProgramNo = 0;
//		st_BadgeKeyInfo[7].puc_FuncIcon_n = st_TopMenuBadgeKeyFuncTbl2[14].puc_FuncIcon_n;
//		st_BadgeKeyInfo[7].puc_FuncIcon_s = st_TopMenuBadgeKeyFuncTbl2[14].puc_FuncIcon_s;
//		st_BadgeKeyInfo[7].puc_FuncIcon_d = st_TopMenuBadgeKeyFuncTbl2[14].puc_FuncIcon_d;
//		st_BadgeKeyInfo[7].uc_IconPaletteLightUI = st_TopMenuBadgeKeyFuncTbl2[14].uc_IconPaletteLightUI;
//		st_BadgeKeyInfo[7].bl_isActivate = True;
//		st_BadgeKeyInfo[7].bl_isShowIcon = False;
//		st_BadgeKeyInfo[7].bl_isShow = True;
//	}
//#endif	// comment 2018/05/24 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 新UIカウンタ画面正式対応時に削除 End.
//}
//
////================================================================================================.
//// 概要	： A072002000_Home(メニュー)： 機能キー表示情報設定.
////================================================================================================.
//void PCGC_HomeTopMenu::setFunctionInfo_HomeMenu( uchar uc_index, TYPE_TopMenuShortCutFunction e_FunctionType )
//{
//	char _sc_select = -1;
//// comment 2018/05/24 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
//	TYPE_MachineType e_MachineType = PnlComGetSystemInfo()->getMachineType();
//	Bool bl_OapIconExit = False;
//
//	switch( e_FunctionType ) {
//		case TYPE_TopMenuShortCutFunctionCopyLightUI:			// コピー.
//			_sc_select = 0;
//// comment 2018/05/24 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
//			st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShowHomeMenu(PNLE_ACCOUNT_STATUS_TYPE_Copy, st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].e_ShortCutKind);
//			break;
//		case TYPE_TopMenuShortCutFunctionScanToEmail:		// Scan To E-mail.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1】 Card copy setting support for DenebMLK model.
//#ifdef DEF_DenebMLKCus 
//			_sc_select = 3;
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
////			_sc_select = 1;
//#else // DEF_DenebMLKCus.
//			_sc_select = 1;
//#endif // DEF_DenebMLKCus.
//			st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShowHomeMenu(PNLE_ACCOUNT_STATUS_TYPE_Fax, st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].e_ShortCutKind);
//			break;
//		case TYPE_TopMenuShortCutFunctionScanToBox:		// ScanToBox.	
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1】 Card copy setting support for DenebMLK model.
//#ifdef DEF_DenebMLKCus 
//			_sc_select = 10;
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
////			_sc_select = 2;
//#else // DEF_DenebMLKCus.
//			_sc_select = 2;
//#endif // DEF_DenebMLKCus.
//			st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShowHomeMenu(PNLE_ACCOUNT_STATUS_TYPE_BoxOperation, st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].e_ShortCutKind);
//			break;
//		case TYPE_TopMenuShortCutFunctionScanToPC:			// ScanToPC.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1】 Card copy setting support for DenebMLK model.
//#ifdef DEF_DenebMLKCus 
//			_sc_select = 11;
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
////			_sc_select = 3;
//#else // DEF_DenebMLKCus.
//			_sc_select = 3;
//#endif // DEF_DenebMLKCus.
//			st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShowHomeMenu(PNLE_ACCOUNT_STATUS_TYPE_Fax, st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].e_ShortCutKind);
//			break;
////2018/08/07 HCLT_Monisha [DenebMLK] build error fix.
//		case TYPE_TopMenuShortCutFunctionInternetFAX:		// InternetFax.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1】 Card copy setting support for DenebMLK model.
//#ifdef DEF_DenebMLKCus 
//			_sc_select = 12;
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
////			_sc_select = 4;
//#else // DEF_DenebMLKCus.
//			_sc_select = 4;
//#endif // DEF_DenebMLKCus.
//			st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShowHomeMenu(PNLE_ACCOUNT_STATUS_TYPE_Fax, st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].e_ShortCutKind);
//			break;
////2018/08/07 HCLT_Monisha [DenebMLK] build error fix.
//		case TYPE_TopMenuShortCutFunctionIPaddrFAX:		// IPAddressFax.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1】 Card copy setting support for DenebMLK model.
//#ifdef DEF_DenebMLKCus 
//			_sc_select = 13;
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
////			_sc_select = 5;
//#else // DEF_DenebMLKCus.
//			_sc_select = 5;
//#endif // DEF_DenebMLKCus.
////2018/08/07 HCLT_Monisha [DenebMLK] build error fix.
//			st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShowHomeMenu(PNLE_ACCOUNT_STATUS_TYPE_Fax, st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].e_ShortCutKind);
//			break;
//		// 【RQ-272】仮対応.
//		case TYPE_TopMenuShortCutFunctionIPFAXT38:		// IPFax.
//#ifdef  DEF_DenebMLKCus 
//			_sc_select = 17;
//#else
//			_sc_select = 16;
//#endif
//			st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShowHomeMenu(PNLE_ACCOUNT_STATUS_TYPE_Fax, st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].e_ShortCutKind);
//			break;
//		case TYPE_TopMenuShortCutFunctionFaxLightUI:			// ファクス.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1】 Card copy setting support for DenebMLK model.
//#ifdef DEF_DenebMLKCus 
//			_sc_select = 4;
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
////			_sc_select = 6;
//#else // DEF_DenebMLKCus.
//			_sc_select = 6;
//#endif // DEF_DenebMLKCus.
//			st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShowHomeMenu(PNLE_ACCOUNT_STATUS_TYPE_Fax, st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].e_ShortCutKind);
//			break;
//
//		case TYPE_TopMenuShortCutFunctionBrowser:				// ブラウザ.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1】 Card copy setting support for DenebMLK model.
//#ifdef DEF_DenebMLKCus
//			_sc_select = 6;
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
////			_sc_select = 7;
//#else	//	DEF_DenebMLKCus.
//			_sc_select = 7;
//#endif //	DEF_DenebMLKCus.
////Comment 2018/12/19 HCLT_Gadiya Sai [IT6_DenebMLK][1068627].
//			if( ( ( PCGE_OapMyPanel == PnlComGetMyPanelType() ) && ( 13 > uc_index ) )		// External_MyPanelの12番目まで.
//			    || ( PCGE_OapMyPanel != PnlComGetMyPanelType() ) ) {						// External_MyPanelでない.
//				m_bl_BrowserFlag = True;													// ブラウザが割り当てられているのでフラグを立てる.			    	
//			}
//			st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShowHomeMenu(PNLE_ACCOUNT_STATUS_TYPE_WebBrowser, st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].e_ShortCutKind);
//			break;
//			
//		case TYPE_TopMenuShortCutFunctionApp:			// OpenAPIアプリ.
//// comment 2018/11/20 HCL_Nandhini [DenebMLK] Build error.
//		{
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1】 Card copy setting support for DenebMLK model.
//#ifdef DEF_DenebMLKCus
//			_sc_select = 16;
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
////			_sc_select = 9;
//#else	//	DEF_DenebMLKCus.
//			_sc_select = 9;
//#endif //	DEF_DenebMLKCus.
//// comment 2018/05/24 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
//			st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShowHomeMenu(PNLE_ACCOUNT_STATUS_TYPE_MAX, st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].e_ShortCutKind);
//			if( ( ( PCGE_OapMyPanel == PnlComGetMyPanelType() ) && ( 13 > _sc_select ) )		// External_MyPanelの12番目まで.
//			    || ( PCGE_OapMyPanel != PnlComGetMyPanelType() ) ) {						// External_MyPanelでない.
//				m_bl_AppFlag = True;					// APPキーが割り付けられているのでフラグを立てる
//			}
//			// 登録アプリケーションがない場合、またはPKI認証設定時かつセキュリティー強化設定有効の場合、登録アプリケーションメニューキーを非表示.
//			if( ( False == ScanOapExtSet.isRegistApplication() && 0 == ExtSet.countApps() )
//			    || ( False == checkShortCutDisableOap( static_cast<TYPE_TopMenuShortCutType>(e_FunctionType), NULL ) )
//			    || ( ( True == PLBC_FuncPKICheck::isPKI() ) && ( True == PnlComGetSystemSetting()->getSecurityLevelUpOn() ) ) ) {
//				st_BadgeKeyInfo[uc_index].bl_isShow = False;
//			}
//			// comment 2018/11/14 SCD2 K.Seto [AR.1067363]表示対象となるアプリがMarketPlaceアプリのみの場合、APPキーを非表示とする.
//			PNLC_ActionDataLib* pc_ActionDataLib = PNLC_ActionDataLib::newInstance();
//			if ( NULL != pc_ActionDataLib ) {
//				// OAPアプリの数をカウントする.
//				uchar uc_OAPAppCnt = 0;
//				uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeScanD );
//				uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeScanS );
//				uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypePullPrint );
//				uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeBrowserScan );
//				uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeBrowserLauncher );
//				uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeBrowserPull );
//				uc_OAPAppCnt += ScanOapExtSet.getRegistAppCount( TYPE_ExtTypeBrowserGeneral );
//				uc_OAPAppCnt += ExtSet.countApps();
//				// comment 2018/12/27 SCD2 T.Nishiba [Eagle][AR1068897]APPキーのキー名称.
//				// AR1067363の対応(MarketPlaceのチェック条件の修正)をここにも適用する.
////				if ( True == pc_ActionDataLib->isInstalledMarketPlaceApp() ) {
//				Bool bl_ExistMarketPlaceWithIWSOff = checkExistMarketPlaceWithIWSOff();
//				if ( ( True == pc_ActionDataLib->isInstalledMarketPlaceApp() ) || ( True == bl_ExistMarketPlaceWithIWSOff ) ) {
//					// MarketPlaceアプリはBrowserLauncherでカウントされてしまうため、インストールされている場合は1引いておく.
//					uc_OAPAppCnt = uc_OAPAppCnt - 1;
//				}
//				
//				// comment 2018/12/27 SCD2 T.Nishiba [Eagle][AR1068897]APPキーのキー名称.
//				// AR1067363の対応(MarketPlaceのチェック条件の修正)をここにも適用する.
////				if ( ( True == ScanOapExtSet.isRegistApplication() ) && ( 0 == uc_OAPAppCnt ) && ( True == pc_ActionDataLib->isInstalledMarketPlaceApp() ) ) {
//				if ( ( True == ScanOapExtSet.isRegistApplication() ) && ( 0 == uc_OAPAppCnt ) && ( ( True == pc_ActionDataLib->isInstalledMarketPlaceApp() ) || ( True == bl_ExistMarketPlaceWithIWSOff ) ) ) {
//					// OAPアプリは未インストール、かつMarketPlaceアプリがインストール済み.
//					// IWSアプリのチェック.
//					APIC_IWSAppInfo* pc_IWSAppInfo = APIC_IWSAppInfo::newInstance();
//					if ( NULL != pc_IWSAppInfo ) {
//						// IWSアプリ数を取得.
//						uchar uc_IWSAppCount = ( uchar )pc_IWSAppInfo->getApplicationNum();
//						if ( uc_IWSAppCount > 1 ) {
//							// アプリ数が1より大きい、即ちMarketPlaceアプリ以外のIWSアプリもインストールされている.
//							uchar uc_IwsAppTypeGeneralCnt = 0;
//							TYPS_IwsAppInfo s_tempAppInfo;
//							memset( &s_tempAppInfo, '\0', sizeof( s_tempAppInfo ) );
//							// インストールされているIWSアプリのアプリタイプを取得.
//							for( uchar uc_Cnt = 0; uc_Cnt < uc_IWSAppCount; uc_Cnt++ ) {
//								int si_Ret = pc_IWSAppInfo->getApplication( (int)( uc_Cnt+1 ), s_tempAppInfo );
//								if ( ( TYPD_IwsResult_OK == si_Ret )
//								  && ( TYPE_IwsAppTypeGeneral == s_tempAppInfo.e_AppType ) ) {
//									// アプリタイプがTYPE_IwsAppTypeGeneralのものは表示対象なのでカウント.
//									uc_IwsAppTypeGeneralCnt++;
//								}
//							}
//							if ( 1 == uc_IwsAppTypeGeneralCnt ) {
//								// 表示対象のアプリが1、即ちMarketPlaceアプリのみのため、APPキーも表示不要.
//								st_BadgeKeyInfo[uc_index].bl_isShow = False;
//							}
//						} else {
//							// アプリ数が1、即ちMarketPlaceアプリのみのため、APPキーも表示不要.
//							st_BadgeKeyInfo[uc_index].bl_isShow = False;
//						}
//					}
//				}
//			}
//			break;
//		}
//// comment 2018/05/24 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
//		case TYPE_TopMenuShortCutFunctionPaperType:		// 用紙.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
//#ifdef DEF_DenebMLKCus
//			_sc_select = 9;
//#else	//	DEF_DenebMLKCus.
//			_sc_select = 11;
//#endif //	DEF_DenebMLKCus.
//			st_BadgeKeyInfo[uc_index].bl_isShow = True;
//			break;
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
//// Comment 2018/12/11 【HCLT-Prem】 【IT6_DenebMLK】 【AR1068595】 Non Image Area Erase Display support.
////		case TYPE_TopMenuShortCutCopyFunctionNoneImageAreaErase:		// 用紙.
////			_sc_select = 1;
////			st_BadgeKeyInfo[uc_index].bl_isShow = True;
////			break;
//		case TYPE_TopMenuShortCutCopyFunctionCardCopy:		// 用紙.
//			_sc_select = 2;
//			st_BadgeKeyInfo[uc_index].bl_isShow = True;
//			break;
//		case TYPE_TopMenuShortCutFunctionAuthPrint:		// 認証プリント.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
//#ifdef DEF_DenebMLKCus
//			_sc_select = 14;
//#else	//	DEF_DenebMLKCus.
//			_sc_select = 12;
//#endif //	DEF_DenebMLKCus.
//			// comment : 20180508 Chikuan : [Eagle] [KRQ-314-28] 【新UI】認証＆プリントBox対応.
//			if( (TYPE_MachineTypeSparrow1SFP == e_MachineType) || (TYPE_MachineTypeSparrow2SFP == e_MachineType)) {
//				st_BadgeKeyInfo[uc_index].bl_isShow = False;
//			}else{
//				st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShowHomeMenu( PNLE_ACCOUNT_STATUS_TYPE_BoxOperation, PNLE_TopMenuShortCut_SysBoxAuthPrint );
//			}
//			break;
//
//		case TYPE_TopMenuShortCutFunctionExtMemPrint:	// 外部メモリー印刷.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
//#ifdef DEF_DenebMLKCus
//			_sc_select = 15;
//#else	//	DEF_DenebMLKCus.
//			_sc_select = 13;
//#endif //	DEF_DenebMLKCus.
//			st_BadgeKeyInfo[uc_index].bl_isShow = True;
//// comment 2018/09/09 SCD2-4G東島 [Eagle][1058776].
//			if( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Print ) ){		// プリント印字禁止.
//				st_BadgeKeyInfo[uc_index].bl_isShow = False;
////			}
//// Comment 2018/12/19 [HCLT-KothaSaran][IT6_DenebMLK][AR1069313] External Memory should be hidden based on condition in DenebMLK
//			}else{
//				st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShowHomeMenu( PNLE_ACCOUNT_STATUS_TYPE_UsbMemoryReading, PNLE_TopMenuShortCut_SysBoxExternalMemory );
//			}
//			break;
//
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//		case TYPE_TopMenuShortCutFunctionUbqPrint:			// ユビキタスプリント.
//			_sc_select = 15;
//			st_BadgeKeyInfo[uc_index].bl_isShow = True;
//			if( False == PnlComCheckUbqClientUse()						||		// ユビキタスプリントクライアント機能利用不可能.
//			    False == PnlComGetUbiquitousSetting()->getClientOn()	||		// クライアント機能が無効.
//			    False == c_ExtPanel.isEnableMFPFunction()	) {	// 指定アプリMFP基本機能設定禁止の場合.
//				st_BadgeKeyInfo[uc_index].bl_isShow = False;
//			}
//// comment 2018/05/24 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
//			if( TYPE_MachineTypeSparrow1SFP != PnlComGetSystemInfo()->getMachineType()) {
//				st_BadgeKeyInfo[uc_index].bl_isShow = False;
//			}
//			break;
//// comment 2018/05/24 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
//		case TYPE_TopMenuShortCutFunctionClassicStyle:		// レガシーへの切り替え.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
//#ifdef DEF_DenebMLKCus 
//			_sc_select = 5;
////			_sc_select = 14;
//#else // DEF_DenebMLKCus.
//			_sc_select = 14;
//#endif // DEF_DenebMLKCus.
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//			st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShowHomeMenu(PNLE_ACCOUNT_STATUS_TYPE_MAX, st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].e_ShortCutKind);
//			break;
//			
//		case TYPE_TopMenuShortCutFunctionSwitchLightUI:		// レガシーへの切り替え.
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1】 Card copy setting support for DenebMLK model.
//#ifdef DEF_DenebMLKCus 
//			_sc_select = 5;
//// Comment 2018/07/16 【HCLT-Neelima】 【IT6_DenebMLK】 【制御構成変更_No.68-1_77-1】【1057102】MergeConflict Resolution.
////			_sc_select = 14;
//#else // DEF_DenebMLKCus.
//			_sc_select = 14;
//#endif // DEF_DenebMLKCus.
//// comment 2018/04/27 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.252_SparrowSFPホーム画面対応.
//			st_BadgeKeyInfo[( uchar )_sc_select].bl_isShow = isFuncShowHomeMenu(PNLE_ACCOUNT_STATUS_TYPE_MAX, st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].e_ShortCutKind);
//			break;
//// Comment 2018/11/19 【HCLT-Prashannaa】【IT6_DenebMLK】【制御構成変更_No.83】
//#ifdef  DEF_DenebMLKCus 
//		case TYPE_TopMenuShortCutSystemBoxFunctionSecurity:
//			{
//#ifdef DEF_LCD_7INCH
//				_sc_select = 17;
//#else
//				_sc_select = 16;
//#endif
//				PNLC_SystemBoxFunctionKeyDispStateLib c_SBLib;
//				st_BadgeKeyInfo[uc_index].bl_isShow = c_SBLib.isShowSecureDocumentBox();
//				break;
//			}
//#endif
//// comment 2018/11/09 Sky雜賀 [EagleH][RQ-272].
//		case TYPE_TopMenuShortCutFunctionIPFAXT38:			// IP-FAX(T.38).
//			_sc_select = 16;
//			st_BadgeKeyInfo[uc_index].bl_isShow = isFuncShowHomeMenu(PNLE_ACCOUNT_STATUS_TYPE_Fax, st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].e_ShortCutKind);
//			break;
//		default:
//			break;
//	}	
//
//	if (-1 != _sc_select) {
//// comment 2018/05/24 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111-1 ホーム画面認証画面の新UI化対応.
//		uchar uc_AppIconData[TYPD_ExtAppListKeyIconDataMaxSize];
//		memset( uc_AppIconData, 0x00, sizeof( uc_AppIconData ) );		// AppIcon.
//		// comment 2015/08/26 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] アイコンデータ取得APIにてデータサイズ取得の追加.
//		int si_AppIconDataSize = 0;	//APIより取得したAppIconのデータサイズ.
//
//		st_BadgeKeyInfo[uc_index].e_ShortCutKind = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].e_ShortCutKind;
//		if( TYPE_TopMenuShortCutFunctionApp == e_FunctionType ) {
//			char sc_AppListName[PCGD_ExtMaxAplNameUTF8];
//
//			st_BadgeKeyInfo[uc_index].tid_FuncName = 0;
//			memset( sc_AppListName, 0, sizeof( sc_AppListName ) );
//			AplCommonSet.getAppBtnName( sc_AppListName );		// アプリリスト名取得.
//			strncpy( st_BadgeKeyInfo[uc_index].sc_FuncName, sc_AppListName, sizeof( st_BadgeKeyInfo[uc_index].sc_FuncName )-1 );		// アプリリスト名設定.
//			st_BadgeKeyInfo[uc_index].sc_FuncName[sizeof( st_BadgeKeyInfo[uc_index].sc_FuncName ) - 1] = '\0';
//			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
//				bl_OapIconExit = PnlComGetStandardPanelSetting()->getApplicationListKeyIcon( uc_AppIconData, &si_AppIconDataSize );
//			}
//		}
//		if( False == bl_OapIconExit ) {
//			st_BadgeKeyInfo[uc_index].puc_FuncIcon = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].puc_FuncIcon;
//			st_BadgeKeyInfo[uc_index].puc_IconNotActivate = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].puc_IconNotActivate;
//			// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] 非OAP時のデータサイズ/パレット設定.独自PNG形式として設定.
//			st_BadgeKeyInfo[uc_index].uc_IconPaletteType = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].uc_IconPaletteType;
//			st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].uc_IconPaletteNotActivate;
//			st_BadgeKeyInfo[uc_index].ss_FuncIconDataSize = 0;
//			st_BadgeKeyInfo[uc_index].ss_IconNotActivateDataSize = 0;
//		} else {
//			switch( PnlComGetStandardPanelSetting()->checkIconFormat( uc_AppIconData ) ) {
//				case TYPE_ExtIconFormat_Bmp:	// 24bit 64×80dot Bitmap.
//				case TYPE_ExtIconFormat_Png:	// 24bit 64×80dot 標準PNG形式.
//					memset( m_ucAppIconData, 0x00, sizeof( m_ucAppIconData ) );		// AppIcon.
//					memcpy( m_ucAppIconData, uc_AppIconData, sizeof( m_ucAppIconData ) );
//					st_BadgeKeyInfo[uc_index].bl_BMP = False;
//					st_BadgeKeyInfo[uc_index].bl_isShowIcon = True;
//					st_BadgeKeyInfo[uc_index].uc_IconPaletteType = PNLD_PaletteNull;
//					st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = PNLD_PaletteNull;
//					// GFW層はデータサイズ非0時に、データ内容を標準BMP/標準PNGと判断するため、アイコン画像のデータサイズを設定する.
//					// アイコンデータはshort上限サイズ以下のため、shortへキャスト.
//					st_BadgeKeyInfo[uc_index].ss_FuncIconDataSize = (short) si_AppIconDataSize;	// 標準BMP/標準PNGならばデータサイズをバッファ長にする.
//					st_BadgeKeyInfo[uc_index].ss_IconNotActivateDataSize = (short) si_AppIconDataSize;
//					break;
//
//				case TYPE_ExtIconFormat_Other:	// 従来APIから送信されるPNG形式（dat）.
//				case TYPE_ExtIconFormat_OriginalPng:	// 従来APIから送信されるKM独自PNG形式（dat）.
//					memset( m_ucAppIconData, 0x00, sizeof( m_ucAppIconData ) );		// AppIcon.
//					memcpy( m_ucAppIconData, uc_AppIconData, sizeof( m_ucAppIconData ) );
//					setBilinearFlag( ( char* )m_ucAppIconData );
//					st_BadgeKeyInfo[uc_index].bl_BMP = False;
//					st_BadgeKeyInfo[uc_index].bl_isShowIcon = True;
//					st_BadgeKeyInfo[uc_index].uc_IconPaletteType = PNLD_PaletteDefault;
//					st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = PNLD_PaletteDefault;
//					// GFW層はデータサイズ0時に、データ内容をKM独自PNGと判断するため、データサイズ0を設定する.
//					st_BadgeKeyInfo[uc_index].ss_FuncIconDataSize = 0;
//					st_BadgeKeyInfo[uc_index].ss_IconNotActivateDataSize = 0;
//					break;
//
//				case TYPE_ExtIconFormat_BmpError:	// 破損データまたは未サポートデータ.
//				default:
//					st_BadgeKeyInfo[uc_index].bl_BMP = False;
//					st_BadgeKeyInfo[uc_index].bl_isShowIcon = False;
//					// comment 2015/08/05 OPS1-MSE高橋 [ZeusS_ZX0][RQ-324] データ形式毎のパレット/データサイズ設定を追加.
//					st_BadgeKeyInfo[uc_index].uc_IconPaletteType = PNLD_PaletteNull;
//					st_BadgeKeyInfo[uc_index].uc_IconPaletteNotActivate = PNLD_PaletteNull;
//					st_BadgeKeyInfo[uc_index].ss_FuncIconDataSize = 0;
//					st_BadgeKeyInfo[uc_index].ss_IconNotActivateDataSize = 0;
//					break;
//			}
//
//			st_BadgeKeyInfo[uc_index].puc_FuncIcon = m_ucAppIconData;
//			st_BadgeKeyInfo[uc_index].puc_IconNotActivate = m_ucAppIconData;
//		}
//		// comment 2018/12/27 SCD2 T.Nishiba [Eagle][AR1068897]APPキーのキー名称.
//		// APPキーの場合は、上の処理でキー名称を設定しているため、ここでは設定しない.
//		if( e_FunctionType != TYPE_TopMenuShortCutFunctionApp ) {
//			st_BadgeKeyInfo[uc_index].tid_FuncName = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].tid_FuncName;
//		}
//		st_BadgeKeyInfo[uc_index].ss_ProgramNo = 0;
//		st_BadgeKeyInfo[uc_index].puc_FuncIcon_n = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].puc_FuncIcon_n;
//		st_BadgeKeyInfo[uc_index].puc_FuncIcon_s = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].puc_FuncIcon_s;
//		st_BadgeKeyInfo[uc_index].puc_FuncIcon_d = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].puc_FuncIcon_d;
//		st_BadgeKeyInfo[uc_index].uc_IconPaletteLightUI = st_TopMenuBadgeKeyFuncTbl2[( uchar )_sc_select].uc_IconPaletteLightUI;
//	}
//}
//
//================================================================================================.
// 概要 : PCGC_HomeTopMenu : UIのモードの違いによりパーツの表示状態を切替る.
// comment : WebUI2 画面切替え制御 20180302 Chikuan.
//================================================================================================.
//void PCGC_HomeTopMenu::changeLayoutByUIMode()
//{
//	Bool b_ShowParts = False;
//	PNLE_UIMode e_UIMode = PNLC_WebUIControl::getInstance()->getUIMode();
//
//	// UIのモードの違いにより、画面テーブル内のパーツの表示／非表示状態を切替る.
//	uchar i = 0;
//	for( i = 0; partsNumber > i; i++ ){
//		if( ( NULL != partsTable ) && ( NULL != partsTable[i] ) ){
//			if( &Grp_LightUIHomeTopMenuAll == partsTable[i] ){
//				b_ShowParts = ( ( PNLE_UIModeLightUI == e_UIMode ) ? True : False );
//			}else{
//				b_ShowParts = ( ( PNLE_UIModeLightUI == e_UIMode ) ? False : True );
//			}
//			partsTable[i]->showEnable( b_ShowParts );
//		}
//	}
//}

//================================================================================================.
// 概要 : PCGC_HomeTopMenu : 表示部品のセットアップ（LightUI用）.
// comment : WebUI2 画面切替え制御 20180302 Chikuan.
//================================================================================================.
void PCGC_HomeTopMenu::setupGamenLightUI( short option )
{
//	PPopupAnimationCom::setupGamen( option );
//
//	printLEDControl( False );
//
//	Bool b_FuncShow = False;
//	Bool b_FuncActive = False;
//#if defined(DEF_RES_WSVGA)
//	const uchar* puc_Parts = i75_025_000_n_X81_Y100;
//#else	// #if defined(DEF_RES_WSVGA).
//	const uchar* puc_Parts = i70_025_000_n_X64_Y80;
//#endif	// #if defined(DEF_RES_WSVGA).
//	uchar uc_PaletteType = PNLD_PaletteTopMenuBaseIcon2;
//
//	b_FuncShow = isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_Copy );
//	if( True == b_FuncShow ){
//		b_FuncActive = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Copy );
//		if( ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionCopy ) )
//		 || ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//			b_FuncActive = False;
//		}
//#if defined(DEF_RES_WSVGA)
//		puc_Parts = ( ( True == b_FuncActive ) ? i75_025_000_n_X81_Y100 : i75_025_000_d_X81_Y100 );
//#else	// #if defined(DEF_RES_WSVGA).
//		puc_Parts = ( ( True == b_FuncActive ) ? i70_041_000_n_X64_Y80 : i70_025_000_d_X64_Y80 );
//#endif	// #if defined(DEF_RES_WSVGA).
//		uc_PaletteType = ( ( True == b_FuncActive ) ? PNLD_PaletteTopMenuBaseIcon2 : PNLD_PaletteCommon );
//		Gra_LightUIHomeTopMenuBadgeIcon[0].changeGraphic( puc_Parts, uc_PaletteType );
//		Ktx_LightUIHomeTopMenuBadge[0].changeText( s_pKEYActivityCopy_W100H042E );
//		Key_LightUIHomeTopMenuBadge[0].activate( b_FuncActive );
//	}
//	Key_LightUIHomeTopMenuBadge[0].showEnable( b_FuncShow );
//
//	b_FuncShow = isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_Fax );
//	if( True == b_FuncShow ){
//		b_FuncActive = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Fax );
//		if( ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionScan ) )
//		 || ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//			b_FuncActive = False;
//		}
//#if defined(DEF_RES_WSVGA)
//		puc_Parts = ( ( True == b_FuncActive ) ? i75_026_000_n_X81_Y100 : i75_026_000_d_X81_Y100 );
//#else	// #if defined(DEF_RES_WSVGA).
//		puc_Parts = ( ( True == b_FuncActive ) ? i70_026_000_n_X64_Y80 : i70_026_000_d_X64_Y80 );
//#endif	// #if defined(DEF_RES_WSVGA).
//		uc_PaletteType = ( ( True == b_FuncActive ) ? PNLD_PaletteTopMenuBaseIcon2 : PNLD_PaletteCommon );
//		Gra_LightUIHomeTopMenuBadgeIcon[1].changeGraphic( puc_Parts, uc_PaletteType );
//		Ktx_LightUIHomeTopMenuBadge[1].changeText( s_pKEYActivityFaxScan_W100H042E );
//		Key_LightUIHomeTopMenuBadge[1].activate( b_FuncActive );
//	}
//	Key_LightUIHomeTopMenuBadge[1].showEnable( b_FuncShow );
//
//	b_FuncShow = isFuncShowFaxOnly();
//	if( True == b_FuncShow ){
//		b_FuncActive = isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Fax );
//		if( ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionFax ) )
//		 || ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//			b_FuncActive = False;
//		}
//#if defined(DEF_RES_WSVGA)
//		puc_Parts = ( ( True == b_FuncActive ) ? i75_041_000_n_X81_Y100 : i75_041_000_d_X81_Y100 );
//#else	// #if defined(DEF_RES_WSVGA).
//		puc_Parts = ( ( True == b_FuncActive ) ? i70_041_000_n_X64_Y80 : i70_041_000_d_X64_Y80 );
//#endif	// #if defined(DEF_RES_WSVGA).
//		uc_PaletteType = ( ( True == b_FuncActive ) ? PNLD_PaletteTopMenuBaseIcon2 : PNLD_PaletteCommon );
//		Gra_LightUIHomeTopMenuBadgeIcon[2].changeGraphic( puc_Parts, uc_PaletteType );
//		Ktx_LightUIHomeTopMenuBadge[2].changeText( s_pKEYActivityFax_W100H042E );
//		Key_LightUIHomeTopMenuBadge[2].activate( b_FuncActive );
//	}
//	Key_LightUIHomeTopMenuBadge[2].showEnable( b_FuncShow );
//
//	b_FuncShow = isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_Fax );
//	if( True == b_FuncShow ){
//		b_FuncActive = True;
//		if( ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionScanToEmail ) )
//		 || ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//			b_FuncActive = False;
//		}
//#if defined(DEF_RES_WSVGA)
//		puc_Parts = ( ( True == b_FuncActive ) ? i75_026_000_n_X81_Y100 : i75_026_000_d_X81_Y100 );
//#else	// #if defined(DEF_RES_WSVGA).
//		puc_Parts = ( ( True == b_FuncActive ) ? i70_026_000_n_X64_Y80 : i70_026_000_d_X64_Y80 );
//#endif	// #if defined(DEF_RES_WSVGA).
//		uc_PaletteType = ( ( True == b_FuncActive ) ? PNLD_PaletteTopMenuBaseIcon2 : PNLD_PaletteCommon );
//		Gra_LightUIHomeTopMenuBadgeIcon[3].changeGraphic( puc_Parts, uc_PaletteType );
//		Ktx_LightUIHomeTopMenuBadge[3].changeText( s_pKEYEmail_W080H018A );
//		Key_LightUIHomeTopMenuBadge[3].activate( b_FuncActive );
//	}
//	Key_LightUIHomeTopMenuBadge[3].showEnable( b_FuncShow );
//
//	b_FuncShow = isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_Fax );
//	if( True == b_FuncShow ){
//		b_FuncActive = True;
//		if( ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionScanToBox ) )
//		 || ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//			b_FuncActive = False;
//		}
//#if defined(DEF_RES_WSVGA)
//		puc_Parts = ( ( True == b_FuncActive ) ? i75_026_000_n_X81_Y100 : i75_026_000_d_X81_Y100 );
//#else	// #if defined(DEF_RES_WSVGA).
//		puc_Parts = ( ( True == b_FuncActive ) ? i70_026_000_n_X64_Y80 : i70_026_000_d_X64_Y80 );
//#endif	// #if defined(DEF_RES_WSVGA).
//		uc_PaletteType = ( ( True == b_FuncActive ) ? PNLD_PaletteTopMenuBaseIcon2 : PNLD_PaletteCommon );
//		Gra_LightUIHomeTopMenuBadgeIcon[4].changeGraphic( puc_Parts, uc_PaletteType );
//		Ktx_LightUIHomeTopMenuBadge[4].changeText( s_pITMBox_W080H018A );
//		Key_LightUIHomeTopMenuBadge[4].activate( b_FuncActive );
//	}
//	Key_LightUIHomeTopMenuBadge[4].showEnable( b_FuncShow );
//
//	b_FuncShow = isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_Fax );
//	if( True == b_FuncShow ){
//		b_FuncActive = True;
//		if( ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionScanToPC ) )
//		 || ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//			b_FuncActive = False;
//		}
//#if defined(DEF_RES_WSVGA)
//		puc_Parts = ( ( True == b_FuncActive ) ? i75_026_000_n_X81_Y100 : i75_026_000_d_X81_Y100 );
//#else	// #if defined(DEF_RES_WSVGA).
//		puc_Parts = ( ( True == b_FuncActive ) ? i70_026_000_n_X64_Y80 : i70_026_000_d_X64_Y80 );
//#endif	// #if defined(DEF_RES_WSVGA).
//		uc_PaletteType = ( ( True == b_FuncActive ) ? PNLD_PaletteTopMenuBaseIcon2 : PNLD_PaletteCommon );
//		Gra_LightUIHomeTopMenuBadgeIcon[5].changeGraphic( puc_Parts, uc_PaletteType );
//		Ktx_LightUIHomeTopMenuBadge[5].changeText( s_pKEYActivityPCScan_W100H042E );
//		Key_LightUIHomeTopMenuBadge[5].activate( b_FuncActive );
//	}
//	Key_LightUIHomeTopMenuBadge[5].showEnable( b_FuncShow );
//
//	b_FuncShow = isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_Fax );
//	if( True == b_FuncShow ){
//		b_FuncActive = True;
//		if( ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionIFax ) )
//		 || ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//			b_FuncActive = False;
//		}
//#if defined(DEF_RES_WSVGA)
//		puc_Parts = ( ( True == b_FuncActive ) ? i75_026_000_n_X81_Y100 : i75_026_000_d_X81_Y100 );
//#else	// #if defined(DEF_RES_WSVGA).
//		puc_Parts = ( ( True == b_FuncActive ) ? i70_026_000_n_X64_Y80 : i70_026_000_d_X64_Y80 );
//#endif	// #if defined(DEF_RES_WSVGA).
//		uc_PaletteType = ( ( True == b_FuncActive ) ? PNLD_PaletteTopMenuBaseIcon2 : PNLD_PaletteCommon );
//		Gra_LightUIHomeTopMenuBadgeIcon[6].changeGraphic( puc_Parts, uc_PaletteType );
//		Ktx_LightUIHomeTopMenuBadge[6].changeText( s_pITMSendIFax_W056H028B );
//		Key_LightUIHomeTopMenuBadge[6].activate( b_FuncActive );
//	}
//	Key_LightUIHomeTopMenuBadge[6].showEnable( b_FuncShow );
//
//	b_FuncShow = isFuncShow( PNLE_ACCOUNT_STATUS_TYPE_Fax );
//	if( True == b_FuncShow ){
//		b_FuncActive = True;
//		if( ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionIPAddressFax ) )
//		 || ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
//			b_FuncActive = False;
//		}
//#if defined(DEF_RES_WSVGA)
//		puc_Parts = ( ( True == b_FuncActive ) ? i75_026_000_n_X81_Y100 : i75_026_000_d_X81_Y100 );
//#else	// #if defined(DEF_RES_WSVGA).
//		puc_Parts = ( ( True == b_FuncActive ) ? i70_026_000_n_X64_Y80 : i70_026_000_d_X64_Y80 );
//#endif	// #if defined(DEF_RES_WSVGA).
//		uc_PaletteType = ( ( True == b_FuncActive ) ? PNLD_PaletteTopMenuBaseIcon2 : PNLD_PaletteCommon );
//		Gra_LightUIHomeTopMenuBadgeIcon[7].changeGraphic( puc_Parts, uc_PaletteType );
//		Ktx_LightUIHomeTopMenuBadge[7].changeText( s_pITMIPAddressFax_W108H020A );
//		Key_LightUIHomeTopMenuBadge[7].activate( b_FuncActive );
//	}
//	Key_LightUIHomeTopMenuBadge[7].showEnable( b_FuncShow );
//
//	b_FuncShow = True;
//	b_FuncActive = True;
//#if defined(DEF_RES_WSVGA)
//	puc_Parts = i75_025_000_n_X81_Y100;
//#else	// #if defined(DEF_RES_WSVGA).
//	puc_Parts = i70_025_000_n_X64_Y80;
//#endif	// #if defined(DEF_RES_WSVGA).
//	uc_PaletteType = PNLD_PaletteTopMenuBaseIcon2;
//	Gra_LightUIHomeTopMenuBadgeIcon[8].changeGraphic( puc_Parts, uc_PaletteType );
//	Ktx_LightUIHomeTopMenuBadge[8].changeText( s_pKEYReturn_W056H024A );
//	Key_LightUIHomeTopMenuBadge[8].activate( b_FuncActive );
//	Key_LightUIHomeTopMenuBadge[8].showEnable( b_FuncShow );
}

//================================================================================================.
// 概要 : PCGC_HomeTopMenu : UIのモード切替えキー押下処理.
// comment : WebUI2 画面切替え制御 20180302 Chikuan.
//================================================================================================.
void PCGC_HomeTopMenu::procUIMode( uchar )
{
	PNLE_UIMode e_NowUIMode = PNLC_WebUIControl::getInstance()->getUIMode();
	PNLE_UIMode e_SetUIMode = PNLE_UIModeSize;
// comment 2018/11/05 SCD2-Sky中野 [EagleH] WebUIv3FUM対応.
//	PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();

	// UIのモードを切替る.
	if( PNLE_UIModeLightUI == e_NowUIMode ){
//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] 起動時に画面がちらつく.
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.
		PNLC_QWSConnector* pc_QWSConnector = PNLC_QWSConnector::newInstance();
		if( NULL != pc_QWSConnector ) {
			pc_QWSConnector->setImageTransfer( False );				// 画像転送を停止する.
//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] 起動時に画面がちらつく.
			PNLC_HomeTimerControlLib::newInstance()->startTimer( DrawingRestart );
		}
#endif //#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
		e_SetUIMode = PNLE_UIModeLegacyUI;
// comment 2018/09/09 SCD2-4G東島 [Eagle][1058366].
		onlyPanel.b_nowBoxMode = False;		// ボックスモードを解除.
	}else if( PNLE_UIModeLegacyUI == e_NowUIMode ){
		e_SetUIMode = PNLE_UIModeLightUI;

//		PNLC_WebUIControl::getInstance()->setWebUIStartStatus(PNLE_WebUIStartStatusStarted);
//		ObserverIndi( TYPE_PNL_Observer_WebUINotifyEnableCopy );
//		ObserverIndi( TYPE_PNL_Observer_WebUINotifyEnableScan );
//		ObserverIndi( TYPE_PNL_Observer_WebUINotifyEnableFax );
//		ObserverIndi( TYPE_PNL_Observer_WebUINotifyEnableScanToEmail );
//		ObserverIndi( TYPE_PNL_Observer_WebUINotifyEnableScanToBox );
//		ObserverIndi( TYPE_PNL_Observer_WebUINotifyEnableScanToPC );
//		ObserverIndi( TYPE_PNL_Observer_WebUINotifyEnableIFax );
//		ObserverIndi( TYPE_PNL_Observer_WebUINotifyEnableIPAddressFax );
	}else{
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	// comment 2018/08/01 SCD2-Sky竹菴 [Eagle][1043869] 切替え時はブラウザモードをOffにするよう修正.
	onlyPanel.setBrowserMode( False );
	onlyPanel.setTopmenuMode( True );
	PNLC_WebUIControl::getInstance()->setUIMode( e_SetUIMode );
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//	if( NULL != pc_PopupListControl ) {	
//		pc_PopupListControl->setChangePopup(&gp_HomeTopMenu );
//	}
	PnlComChangeScreenHome();
}

//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] 起動時に画面がちらつく.
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.
void PCGC_HomeTopMenu::DrawingRestart()
{
	PNLC_QWSConnector* pc_QWSConnector = PNLC_QWSConnector::newInstance();
	if( NULL != pc_QWSConnector ) {
		pc_QWSConnector->setImageTransfer( True );				// 画像転送を再開する.
		PNLC_HomeTimerControlLib::newInstance()->killTimer();
	}
}
#endif //#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.

//================================================================================================.
// 概要 : PCGC_HomeTopMenu : 新UIのシステムボックス遷移先情報の設定. 
// comment : 20180508 Chikuan : [Eagle] [KRQ-314-28] 【新UI】認証＆プリントBox対応.
//================================================================================================.
void PCGC_HomeTopMenu::setWebUIBoxFunction( PNLE_WebUIFunction e_WebUIFunction )
{
	m_e_WebUIBoxFunction = e_WebUIFunction;
}

//================================================================================================.
// 概要 : PCGC_HomeTopMenu : 新UIのシステムボックス遷移先情報の取得.
// comment : 20180508 Chikuan : [Eagle] [KRQ-314-28] 【新UI】認証＆プリントBox対応.
//================================================================================================.
PNLE_WebUIFunction PCGC_HomeTopMenu::getWebUIBoxFunction()
{
	return m_e_WebUIBoxFunction;
}

// comment 2019/01/10 SCD2-Sky中野 [EagleH] 制御No.38_目的別UI対応.
//------------------------------------------------------------------------------
/**
* @brief PCGC_HomeTopMenu : Box機能への遷移処理（LightUI用）.
*/
//------------------------------------------------------------------------------
void PCGC_HomeTopMenu::procBoxFuncLightUI( PNLE_WebUIFunction e_WebUIFunction )
{
// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
	PNLE_UIMode uc_UIModebak = PNLC_WebUIControl::getInstance()->getUIMode();
	PNLC_WebUIControl::getInstance()->setUIMode( PNLE_UIModeLightUI );
	
	setWebUIBoxFunction( e_WebUIFunction );
	
// comment 2022/07/11 Sky東島 [IT6_2.3][制御No.39-42_目的別UIプリセット化].
#if defined  (DEF_DenebMLKCus) || defined ( DEF_HeliosMLKCus )
	if( PNLE_WebUIFunctionSecurityDocument == e_WebUIFunction
	 && False == PnlComGetHddInfo()->getHddExist()
	 && True == PnlComIsSecurityDoc() ){
		procBox( PNLD_SecurePrintWithoutHDD );
	} else {
		procBox( PNLD_CheckGotoUsbWithoutHDD );
	}
#else	//  defined  (DEF_DenebMLKCus) || defined ( DEF_HeliosMLKCus )
	procBox( PNLD_CheckGotoUsbWithoutHDD );
#endif
	
	if( NULL != nextGamen ){
		PNLC_BoxBoxListControlLib* pc_BoxListControlLib = PNLC_BoxBoxListControlLib::newInstance();
		if( NULL != pc_BoxListControlLib ){
			pc_BoxListControlLib->resetBox();

			PNLC_TabletSecurityControlLib* pc_TabletSecurityControl = PNLC_TabletSecurityControlLib::newInstance();
			if( NULL != pc_TabletSecurityControl ){
				pc_TabletSecurityControl->securityAreaExited();
			}
		}
	}
	
	setWebUIBoxFunction( PNLE_WebUIFunctionSize );
	
// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
	if( PNLE_WebUIScreenChangeStatusDuring != PNLC_WebUIControl::getInstance()->getWebUIScreenChangeStatus() ) {
		// 画面遷移しなかった場合は、UIモードを元に戻す.
		PNLC_WebUIControl::getInstance()->setUIMode( uc_UIModebak );
	}
}

//------------------------------------------------------------------------------
/**
* @brief PCGC_HomeTopMenu : Copy機能への遷移処理（LightUI用）.
*/
//------------------------------------------------------------------------------
void PCGC_HomeTopMenu::procCopyFuncLightUI( PNLE_WebUIFunction e_WebUIFunction )
{
// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
	PNLE_UIMode uc_UIModebak = PNLC_WebUIControl::getInstance()->getUIMode();
	PNLC_WebUIControl::getInstance()->setUIMode( PNLE_UIModeLightUI );

	// WebUIコピー遷移先を設定.
	setWebUICopyFunction( e_WebUIFunction );
	
	procCopy( 0 );
	
	// WebUIコピー遷移先を無効値に戻す.
	setWebUICopyFunction( PNLE_WebUIFunctionSize );

// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
	if( PNLE_WebUIScreenChangeStatusDuring != PNLC_WebUIControl::getInstance()->getWebUIScreenChangeStatus() ) {
		// 画面遷移しなかった場合は、UIモードを元に戻す.
		PNLC_WebUIControl::getInstance()->setUIMode( uc_UIModebak );
	}
}

//------------------------------------------------------------------------------
/**
* @brief PCGC_HomeTopMenu : WebUIコピー遷移先設定.
*/
//------------------------------------------------------------------------------
void PCGC_HomeTopMenu::setWebUICopyFunction( PNLE_WebUIFunction e_WebUIFunction )
{
	m_e_WebUICopyFunction = e_WebUIFunction;
}

//------------------------------------------------------------------------------
/**
* @brief PCGC_HomeTopMenu : WebUIコピー遷移先取得.
*/
//------------------------------------------------------------------------------
PNLE_WebUIFunction PCGC_HomeTopMenu::getWebUICopyFunction()
{
	return m_e_WebUICopyFunction;
}

// comment 2019/10/10 SCD2-Sky中野 [EagleH] IWS遷移アプリ対応.
//------------------------------------------------------------------------------
/**
* @brief PCGC_HomeTopMenu : Faxへの遷移処理（LightUI用）.
*/
//------------------------------------------------------------------------------
void PCGC_HomeTopMenu::procFaxLightUI( PNLE_WebUIFunction e_WebUIFunction )
{
	PNLE_UIMode uc_UIModebak = PNLC_WebUIControl::getInstance()->getUIMode();
	PNLC_WebUIControl::getInstance()->setUIMode( PNLE_UIModeLightUI );

// comment 2020/03/09 SCD2-Sky石原[IT6_2.0MR2][制御No.31]新CopyScanFAXアプリのカスタマイズ機能.
//	// WebUIファクス遷移フラグを設定.
//	setWebUIFaxFlag( True );
	
	//WebUIファクス遷移先を設定
	setWebUIFaxFunction( e_WebUIFunction );
	
	procFaxOnly( 0 );
	
// comment 2020/03/09 SCD2-Sky石原[IT6_2.0MR2][制御No.31]新CopyScanFAXアプリのカスタマイズ機能.
//	// WebUIファクス遷移フラグを無効値に戻す.
//	setWebUIFaxFlag( False );
	
	// WebUIファクス遷移先を無効値に戻す.
	setWebUIFaxFunction( PNLE_WebUIFunctionSize );

	if( PNLE_WebUIScreenChangeStatusDuring != PNLC_WebUIControl::getInstance()->getWebUIScreenChangeStatus() ) {
		// 画面遷移しなかった場合は、UIモードを元に戻す.
		PNLC_WebUIControl::getInstance()->setUIMode( uc_UIModebak );
	}
}
// comment 2020/03/09 SCD2-Sky石原[IT6_2.0MR2][制御No.31]新CopyScanFAXアプリのカスタマイズ機能.
//------------------------------------------------------------------------------
/**
* @brief PCGC_HomeTopMenu : WebUIファクス遷移先設定.
*/
//------------------------------------------------------------------------------
void PCGC_HomeTopMenu::setWebUIFaxFunction( PNLE_WebUIFunction e_WebUIFunction )
{
	m_e_WebUIFaxFunction = e_WebUIFunction;
}
// comment 2020/03/09 SCD2-Sky石原[IT6_2.0MR2][制御No.31]新CopyScanFAXアプリのカスタマイズ機能.
//------------------------------------------------------------------------------
/**
* @brief PCGC_HomeTopMenu : WebUIファクス遷移先取得.
*/
//------------------------------------------------------------------------------
PNLE_WebUIFunction PCGC_HomeTopMenu::getWebUIFaxFunction()
{
	return m_e_WebUIFaxFunction;
}

// comment 2020/03/10 SCD2-Sky中野 [IT6_2.0MR2] 制御No.30_Program登録機能.
//------------------------------------------------------------------------------
/**
* @brief PCGC_HomeTopMenu : WebUIコピー遷移要求メッセージ取得.
*/
//------------------------------------------------------------------------------
PNLE_WebUIRequestMessage PCGC_HomeTopMenu::getWebUICopyReqMsg()
{
	PNLE_WebUIRequestMessage uc_ReqMsg = PNLE_WebUIRequestMessageNon;
	
	switch( m_e_WebUICopyFunction ) {
		case PNLE_WebUIFunctionCopy:
			if( 0 == PNLC_WebUIControl::getInstance()->getSelectProgramNo() ) {
				uc_ReqMsg = PNLE_WebUIRequestMessageCopy;
			} else {
				uc_ReqMsg = PNLE_WebUIRequestMessageCopyProgram;
			}
			break;
		case PNLE_WebUIFunctionIDCardCopy:
			uc_ReqMsg = PNLE_WebUIRequestMessageIDCardCopy;
			break;
		case PNLE_WebUIFunctionCopyCardType:
			if( 0 == PNLC_WebUIControl::getInstance()->getSelectProgramNo() ) {
				uc_ReqMsg = PNLE_WebUIRequestMessageCopyCardType;
			} else {
				uc_ReqMsg = PNLE_WebUIRequestMessageCopyCardTypeProgram;
			}
			break;
		case PNLE_WebUIFunctionCopyCustomize:
			uc_ReqMsg = PNLE_WebUIRequestMessageToCopyCustomize;
			break;
		case PNLE_WebUIFunctionCardUICopyCustomize:
			uc_ReqMsg = PNLE_WebUIRequestMessageToCardUICopyCustomize;
			break;
// comment 2022/06/30 Sky東島 [IT6_2.3][制御No.39-42_目的別UIプリセット化].
		case PNLE_WebUIFunctionBookletCopy:
			uc_ReqMsg = PNLE_WebUIRequestMessageBookletCopy;
			break;
		case PNLE_WebUIFunctionShadowEraseCopy:
			uc_ReqMsg = PNLE_WebUIRequestMessageShadowEraseCopy;
			break;
/* // WebUI制御自動生成マクロ用処理 WebUIコピー機能 削除禁止 Start.
// comment YYYY/MM/DD DeveloperName [ModelName] WBSNo Title .
		case PNLE_WebUIFunctionXXXX:				// ModelName WBSNo Title.
			uc_ReqMsg = PNLE_WebUIRequestMessageXXXX;
			break;
// WebUI制御自動生成マクロ用処理 削除禁止 End. */
		default:
			uc_ReqMsg = PNLE_WebUIRequestMessageNon;
			break;
	}
	
	return uc_ReqMsg;
}

//------------------------------------------------------------------------------
/**
* @brief PCGC_HomeTopMenu : WebUIファクス遷移要求メッセージ取得.
*/
//------------------------------------------------------------------------------
PNLE_WebUIRequestMessage PCGC_HomeTopMenu::getWebUIFaxReqMsg()
{
	PNLE_WebUIRequestMessage uc_ReqMsg = PNLE_WebUIRequestMessageNon;
	
	switch( m_e_WebUIFaxFunction ) {
		case PNLE_WebUIFunctionFax:
			if( 0 == PNLC_WebUIControl::getInstance()->getSelectProgramNo() ) {
				uc_ReqMsg = PNLE_WebUIRequestMessageFax;
			} else {
				uc_ReqMsg = PNLE_WebUIRequestMessageFaxProgram;
			}
			break;
		case PNLE_WebUIFunctionFaxCustomize:
			uc_ReqMsg = PNLE_WebUIRequestMessageToFaxCustomize;
			break;
/* // WebUI制御自動生成マクロ用処理 WebUIファクス機能 削除禁止 Start.
// comment YYYY/MM/DD DeveloperName [ModelName] WBSNo Title .
		case PNLE_WebUIFunctionXXXX:				// ModelName WBSNo Title.
			uc_ReqMsg = PNLE_WebUIRequestMessageXXXX;
			break;
// WebUI制御自動生成マクロ用処理 削除禁止 End. */
		default:
			uc_ReqMsg = PNLE_WebUIRequestMessageNon;
			break;
	}
	
	return uc_ReqMsg;
}

// comment 2020/07/14 Sky中野 [IT6_2.0MR3_SpecialROM][OP_BTS-25456].
//------------------------------------------------------------------------------
/**
* @brief PCGC_HomeTopMenu : スキャンファックスプログラムキーの表示/非表示判定.
*/
//------------------------------------------------------------------------------
Bool PCGC_HomeTopMenu::getShowStatusProgramKey( short ss_ProgramNo, Bool bl_fromWebUI )
{
	Bool bl_ret = True;
	
	// プログラムキーに割りついている検索宛先種別を取得.
	TYPD_AddressSearchTarget ui32_SearchTarget = TYPD_AddressSearchTargetNone;
	if( True == bl_fromWebUI ) {
		ui32_SearchTarget = PNLC_HomeProgramKeyData::getInstance()->getProgramKeySearchTarget( ss_ProgramNo );
	} else {
		ui32_SearchTarget = PCGC_TopmenuProgramKeyData::getInstance()->getProgramKeySearchTarget( ss_ProgramNo );
	}
	
	if( ( True == PLBC_FuncPKICheck::isPKI() )
	 && ( NULL != PnlComGetUser() )
	 && ( TYPE_UserTypeUser != PnlComGetUser()->getUserType() )
	 && ( TYPD_AddressSearchTargetSmb == ( TYPD_AddressSearchTargetSmb & ui32_SearchTarget ) ) ) {
		// PKI認証かつ特殊ユーザーでログイン時は、SMB宛先のプログラムキーを非表示とする.
		bl_ret = False;
	}
	
	return bl_ret;
}

// comment 2020/09/25 Sky中野 [IT6_2.1][OP_BTS-27503].
//------------------------------------------------------------------------------
/**
* @brief PCGC_HomeTopMenu : プログラム宛先選択確認中のプログラム番号取得.
*/
//------------------------------------------------------------------------------
short PCGC_HomeTopMenu::getConfirmationProgramNo()
{
	return m_ss_ConfirmationProgramNo;
}

// comment 2020/11/04 Sky中野 [IT6_2.1][OP_BTS-27703].
//------------------------------------------------------------------------------
/**
* @brief PCGC_HomeTopMenu : WebUIシステムボックス遷移要求メッセージ取得.
*/
//------------------------------------------------------------------------------
PNLE_WebUIRequestMessage PCGC_HomeTopMenu::getWebUIBoxReqMsg()
{
	PNLE_WebUIRequestMessage uc_ReqMsg = PNLE_WebUIRequestMessageNon;
	
	switch( m_e_WebUIBoxFunction ) {
		case PNLE_WebUIFunctionAuthPrint:
			uc_ReqMsg = PNLE_WebUIRequestMessageAuthPrint;
			break;
		case PNLE_WebUIFunctionUSBToPrint:
			if( True == PNLC_WebUIControl::getInstance()->getAutoChangeToWebUI() ) {
				// 自動遷移.
				uc_ReqMsg = PNLE_WebUIRequestMessageUSBToPrintAT;
			} else {
				uc_ReqMsg = PNLE_WebUIRequestMessageUSBToPrint;
			}
			break;
		case PNLE_WebUIFunctionPkiEncryptDocBox:
			uc_ReqMsg = PNLE_WebUIRequestMessagePkiEncryptDocBox;
			break;
// comment 2022/06/30 Sky東島 [IT6_2.3][制御No.39-42_目的別UIプリセット化].
		case PNLE_WebUIFunctionSecurityDocument:
			uc_ReqMsg = PNLE_WebUIRequestMessageSecurityDocument;
			break;
		case PNLE_WebUIFunctionForcedMemory:
			uc_ReqMsg = PNLE_WebUIRequestMessageForcedMemory;
			break;
/* // WebUI制御自動生成マクロ用処理 WebUIボックス機能 削除禁止 Start.
// comment YYYY/MM/DD DeveloperName [ModelName] WBSNo Title .
		case PNLE_WebUIFunctionXXXX:				// ModelName WBSNo Title.
			uc_ReqMsg = PNLE_WebUIRequestMessageXXXX;
			break;
// WebUI制御自動生成マクロ用処理 削除禁止 End. */
		default:
			uc_ReqMsg = PNLE_WebUIRequestMessageNon;
			break;
	}
	
	return uc_ReqMsg;
}

//------------------------------------------------------------------------------
/**
* @brief PCGC_HomeTopMenu : WebUIファクス遷移フラグ設定.
*/
//------------------------------------------------------------------------------
//void PCGC_HomeTopMenu::setWebUIFaxFlag( Bool bl_WebUIFaxFlag )
//{
//	m_bl_WebUIFaxFlag = bl_WebUIFaxFlag;
//}

//------------------------------------------------------------------------------
/**
* @brief PCGC_HomeTopMenu : WebUIファクス遷移フラグ取得.
*/
//------------------------------------------------------------------------------
//Bool PCGC_HomeTopMenu::getWebUIFaxFlag()
//{
//	return m_bl_WebUIFaxFlag;
//}

//================================================================================================.
// 概要 : PCGC_HomeTopMenu : Scan To E-mailキー押下処理.
// comment : WebUI2 画面切替え制御 20180302 Chikuan.
//================================================================================================.
//void PCGC_HomeTopMenu::procScanToEmail( uchar )
//{
//	if( False == c_ExtPanel.isEnableMFPFunction() ){
//		setBuzzerType( PCGE_BuzzerType_InputNg );
//		return;
//	}
//
//	if( True == sp_MessageWarningTable[onlyPanel.us_NowMessageWarning].bl_isDisableHardKey[EP_HardKey_Fax] ){
//		setBuzzerType( PCGE_BuzzerType_InputNg );
//		return;
//	}
//
//	if( True == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Fax ) ){
//		if( ( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() )
//		 && ( ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() )
//		   || ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionScanToEmail ) ) ) ){
//			setBuzzerType( PCGE_BuzzerType_InputNg );
//			return;
//		}
//
//		PNLE_SpecifiedAppState e_State = c_ExtPanel.getSpecifiedAppState();
//		onlyPanel.setTopMenuSpecifiedAppState( e_State );
//		c_ExtPanel.ChangeSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
//
//		onlyPanel.resetDocSizeDetectError_S();
//		c_JobControl.changePanelAPP( TYPE_JobTypeFaxSendMemory );
//		onlyPanel.setPanelStatus( judgePanelStatus( comPanelJob->getJobStatus()->getInputStatus(), comPanelJob->getJobStatus()->getOutputStatus(), comPanelJob->getJobStatus()->getFaxJobStatus(), comPanelJob->getJobStatus()->getRealTimePreviewStatus() ) );
//		onlyPanel.setUbqPrintMode( False );
//		if( True == onlyPanel.isWarningOn( comPanelJob, PNL_OTHER_PaperSizeMeasurement ) ){
//			onlyPanel.resetPanelWarning( PNL_OTHER_PaperSizeMeasurement );
//		}
//
//		PNLC_WebUIControl::getInstance()->reserveChangeScreen( PNLE_WebUIRequestMessageScanToEmail );
//		rebuildGamen( 0 );
//		setTurnGamenThis( NULL );
//		PNLC_LdapSearchManager::newInstance()->clearLdapSearchCondition();
//		PNLC_LdapSearchManager::newInstance()->clearLdapMultiSearchCondition();
//
//		if( ( ( nextGamen != NULL )
//			||( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_IGCClearWait_popup ) )
//			||( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_IGCWait_popup ) ) )
//		 && ( True == onlyPanel.bl_BrowserFileUploadRequest ) ){
//			uchar uc_UploadType = 0;
//			if( &gp_FileUploadTypeSelectExt_browser == getNowGamen() ){
//				uc_UploadType = PCGD_BrowserFileUploadTypeSelect;
//			}
//			deleteUploadInfo( uc_UploadType );
//		}
//
//		onlyPanel.setTopMenuSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
//		if( ( NULL != nextGamen ) ||
//		    ( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_IGCClearWait_popup ) ) ){
//			onlyPanel.setTopmenuMode( False );
//		}
//		else if( ( NULL == nextGamen ) && ( False == c_ExtPanel.checkPopupWarningFromTopMenu() ) ){
//			c_ExtPanel.ChangeSpecifiedAppState( e_State );
//		}
//	}else{
//		setBuzzerType( PCGE_BuzzerType_InputNg );
//	}
//}
//
//================================================================================================.
// 概要 : PCGC_HomeTopMenu : Scan To Boxキー押下処理.
// comment : WebUI2 画面切替え制御 20180302 Chikuan.
//================================================================================================.
//void PCGC_HomeTopMenu::procScanToBox( uchar  )
//{
//	if( False == c_ExtPanel.isEnableMFPFunction() ){
//		setBuzzerType( PCGE_BuzzerType_InputNg );
//		return;
//	}
//
//	if( True == sp_MessageWarningTable[onlyPanel.us_NowMessageWarning].bl_isDisableHardKey[EP_HardKey_Fax] ){
//		setBuzzerType( PCGE_BuzzerType_InputNg );
//		return;
//	}
//
//	if( True == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Fax ) ){
//		if( ( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() )
//		 && ( ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() )
//		   || ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionScanToBox ) ) ) ){
//			setBuzzerType( PCGE_BuzzerType_InputNg );
//			return;
//		}
//
//		PNLE_SpecifiedAppState e_State = c_ExtPanel.getSpecifiedAppState();
//		onlyPanel.setTopMenuSpecifiedAppState( e_State );
//		c_ExtPanel.ChangeSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
//
//		onlyPanel.resetDocSizeDetectError_S();
//		c_JobControl.changePanelAPP( TYPE_JobTypeFaxSendMemory );
//		onlyPanel.setPanelStatus( judgePanelStatus( comPanelJob->getJobStatus()->getInputStatus(), comPanelJob->getJobStatus()->getOutputStatus(), comPanelJob->getJobStatus()->getFaxJobStatus(), comPanelJob->getJobStatus()->getRealTimePreviewStatus() ) );
//		onlyPanel.setUbqPrintMode( False );
//		if( True == onlyPanel.isWarningOn( comPanelJob, PNL_OTHER_PaperSizeMeasurement ) ){
//			onlyPanel.resetPanelWarning( PNL_OTHER_PaperSizeMeasurement );
//		}
//
//		PNLC_WebUIControl::getInstance()->reserveChangeScreen( PNLE_WebUIRequestMessageScanToBox );
//		rebuildGamen( 0 );
//		setTurnGamenThis( NULL );
//		PNLC_LdapSearchManager::newInstance()->clearLdapSearchCondition();
//		PNLC_LdapSearchManager::newInstance()->clearLdapMultiSearchCondition();
//
//		if( ( ( nextGamen != NULL )
//			||( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_IGCClearWait_popup ) )
//			||( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_IGCWait_popup ) ) )
//		 && ( True == onlyPanel.bl_BrowserFileUploadRequest ) ){
//			uchar uc_UploadType = 0;
//			if( &gp_FileUploadTypeSelectExt_browser == getNowGamen() ){
//				uc_UploadType = PCGD_BrowserFileUploadTypeSelect;
//			}
//			deleteUploadInfo( uc_UploadType );
//		}
//
//		onlyPanel.setTopMenuSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
//		if( ( NULL != nextGamen ) ||
//		    ( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_IGCClearWait_popup ) ) ){
//			onlyPanel.setTopmenuMode( False );
//		}
//		else if( ( NULL == nextGamen ) && ( False == c_ExtPanel.checkPopupWarningFromTopMenu() ) ){
//			c_ExtPanel.ChangeSpecifiedAppState( e_State );
//		}
//	}else{
//		setBuzzerType( PCGE_BuzzerType_InputNg );
//	}
//}
//
//================================================================================================.
// 概要 : PCGC_HomeTopMenu : Scan To PCキー押下処理.
// comment : WebUI2 画面切替え制御 20180302 Chikuan.
//================================================================================================.
//void PCGC_HomeTopMenu::procScanToPC( uchar )
//{
//	if( False == c_ExtPanel.isEnableMFPFunction() ){
//		setBuzzerType( PCGE_BuzzerType_InputNg );
//		return;
//	}
//
//	if( True == sp_MessageWarningTable[onlyPanel.us_NowMessageWarning].bl_isDisableHardKey[EP_HardKey_Fax] ){
//		setBuzzerType( PCGE_BuzzerType_InputNg );
//		return;
//	}
//
//	if( True == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Fax ) ){
//		if( ( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() )
//		 && ( ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() )
//		   || ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionScanToPC ) ) ) ){
//			setBuzzerType( PCGE_BuzzerType_InputNg );
//			return;
//		}
//
//		PNLE_SpecifiedAppState e_State = c_ExtPanel.getSpecifiedAppState();
//		onlyPanel.setTopMenuSpecifiedAppState( e_State );
//		c_ExtPanel.ChangeSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
//
//		onlyPanel.resetDocSizeDetectError_S();
//		c_JobControl.changePanelAPP( TYPE_JobTypeFaxSendMemory );
//		onlyPanel.setPanelStatus( judgePanelStatus( comPanelJob->getJobStatus()->getInputStatus(), comPanelJob->getJobStatus()->getOutputStatus(), comPanelJob->getJobStatus()->getFaxJobStatus(), comPanelJob->getJobStatus()->getRealTimePreviewStatus() ) );
//		onlyPanel.setUbqPrintMode( False );
//		if( True == onlyPanel.isWarningOn( comPanelJob, PNL_OTHER_PaperSizeMeasurement ) ){
//			onlyPanel.resetPanelWarning( PNL_OTHER_PaperSizeMeasurement );
//		}
//
//		PNLC_WebUIControl::getInstance()->reserveChangeScreen( PNLE_WebUIRequestMessageScanToPC );
//		rebuildGamen( 0 );
//		setTurnGamenThis( NULL );
//		PNLC_LdapSearchManager::newInstance()->clearLdapSearchCondition();
//		PNLC_LdapSearchManager::newInstance()->clearLdapMultiSearchCondition();
//
//		if( ( ( nextGamen != NULL )
//			||( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_IGCClearWait_popup ) )
//			||( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_IGCWait_popup ) ) )
//		 && ( True == onlyPanel.bl_BrowserFileUploadRequest ) ){
//			uchar uc_UploadType = 0;
//			if( &gp_FileUploadTypeSelectExt_browser == getNowGamen() ){
//				uc_UploadType = PCGD_BrowserFileUploadTypeSelect;
//			}
//			deleteUploadInfo( uc_UploadType );
//		}
//
//		onlyPanel.setTopMenuSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
//		if( ( NULL != nextGamen ) ||
//		    ( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_IGCClearWait_popup ) ) ){
//			onlyPanel.setTopmenuMode( False );
//		}
//		else if( ( NULL == nextGamen ) && ( False == c_ExtPanel.checkPopupWarningFromTopMenu() ) ){
//			c_ExtPanel.ChangeSpecifiedAppState( e_State );
//		}
//	}else{
//		setBuzzerType( PCGE_BuzzerType_InputNg );
//	}
//}
//
//================================================================================================.
// 概要 : PCGC_HomeTopMenu : I-Faxキー押下処理.
// comment : WebUI2 画面切替え制御 20180302 Chikuan.
//================================================================================================.
//void PCGC_HomeTopMenu::procIFax( uchar )
//{
//	if( False == c_ExtPanel.isEnableMFPFunction() ){
//		setBuzzerType( PCGE_BuzzerType_InputNg );
//		return;
//	}
//
//	if( True == sp_MessageWarningTable[onlyPanel.us_NowMessageWarning].bl_isDisableHardKey[EP_HardKey_Fax] ){
//		setBuzzerType( PCGE_BuzzerType_InputNg );
//		return;
//	}
//
//	if( True == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Fax ) ){
//		if( ( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() )
//		 && ( ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() )
//		   || ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionIFax ) ) ) ){
//			setBuzzerType( PCGE_BuzzerType_InputNg );
//			return;
//		}
//
//		PNLE_SpecifiedAppState e_State = c_ExtPanel.getSpecifiedAppState();
//		onlyPanel.setTopMenuSpecifiedAppState( e_State );
//		c_ExtPanel.ChangeSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
//
//		onlyPanel.resetDocSizeDetectError_S();
//		c_JobControl.changePanelAPP( TYPE_JobTypeFaxSendMemory );
//		onlyPanel.setPanelStatus( judgePanelStatus( comPanelJob->getJobStatus()->getInputStatus(), comPanelJob->getJobStatus()->getOutputStatus(), comPanelJob->getJobStatus()->getFaxJobStatus(), comPanelJob->getJobStatus()->getRealTimePreviewStatus() ) );
//		onlyPanel.setUbqPrintMode( False );
//		if( True == onlyPanel.isWarningOn( comPanelJob, PNL_OTHER_PaperSizeMeasurement ) ){
//			onlyPanel.resetPanelWarning( PNL_OTHER_PaperSizeMeasurement );
//		}
//
//		PNLC_WebUIControl::getInstance()->reserveChangeScreen( PNLE_WebUIRequestMessageIFax );
//		rebuildGamen( 0 );
//		setTurnGamenThis( NULL );
//		PNLC_LdapSearchManager::newInstance()->clearLdapSearchCondition();
//		PNLC_LdapSearchManager::newInstance()->clearLdapMultiSearchCondition();
//
//		if( ( ( nextGamen != NULL )
//			||( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_IGCClearWait_popup ) )
//			||( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_IGCWait_popup ) ) )
//		 && ( True == onlyPanel.bl_BrowserFileUploadRequest ) ){
//			uchar uc_UploadType = 0;
//			if( &gp_FileUploadTypeSelectExt_browser == getNowGamen() ){
//				uc_UploadType = PCGD_BrowserFileUploadTypeSelect;
//			}
//			deleteUploadInfo( uc_UploadType );
//		}
//
//		onlyPanel.setTopMenuSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
//		if( ( NULL != nextGamen ) ||
//		    ( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_IGCClearWait_popup ) ) ){
//			onlyPanel.setTopmenuMode( False );
//		}
//		else if( ( NULL == nextGamen ) && ( False == c_ExtPanel.checkPopupWarningFromTopMenu() ) ){
//			c_ExtPanel.ChangeSpecifiedAppState( e_State );
//		}
//	}else{
//		setBuzzerType( PCGE_BuzzerType_InputNg );
//	}
//}
//
//================================================================================================.
// 概要 : PCGC_HomeTopMenu : IPAddress Faxキー押下処理.
// comment : WebUI2 画面切替え制御 20180302 Chikuan.
//================================================================================================.
//void PCGC_HomeTopMenu::procIPAddressFax( uchar )
//{
//	if( False == c_ExtPanel.isEnableMFPFunction() ){
//		setBuzzerType( PCGE_BuzzerType_InputNg );
//		return;
//	}
//
//	if( True == sp_MessageWarningTable[onlyPanel.us_NowMessageWarning].bl_isDisableHardKey[EP_HardKey_Fax] ){
//		setBuzzerType( PCGE_BuzzerType_InputNg );
//		return;
//	}
//
//	if( True == isFuncEnable( PNLE_ACCOUNT_STATUS_TYPE_Fax ) ){
//		if( ( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() )
//		 && ( ( PNLE_WebUIStartStatusStarted != PNLC_WebUIControl::getInstance()->getWebUIStartStatus() )
//		   || ( False == PNLC_WebUIControl::getInstance()->getEnableFunction( PNLE_WebUIFunctionIPAddressFax ) ) ) ){
//			setBuzzerType( PCGE_BuzzerType_InputNg );
//			return;
//		}
//
//		PNLE_SpecifiedAppState e_State = c_ExtPanel.getSpecifiedAppState();
//		onlyPanel.setTopMenuSpecifiedAppState( e_State );
//		c_ExtPanel.ChangeSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
//
//		onlyPanel.resetDocSizeDetectError_S();
//		c_JobControl.changePanelAPP( TYPE_JobTypeFaxSendMemory );
//		onlyPanel.setPanelStatus( judgePanelStatus( comPanelJob->getJobStatus()->getInputStatus(), comPanelJob->getJobStatus()->getOutputStatus(), comPanelJob->getJobStatus()->getFaxJobStatus(), comPanelJob->getJobStatus()->getRealTimePreviewStatus() ) );
//		onlyPanel.setUbqPrintMode( False );
//		if( True == onlyPanel.isWarningOn( comPanelJob, PNL_OTHER_PaperSizeMeasurement ) ){
//			onlyPanel.resetPanelWarning( PNL_OTHER_PaperSizeMeasurement );
//		}
//
//		PNLC_WebUIControl::getInstance()->reserveChangeScreen( PNLE_WebUIRequestMessageIPAddressFax );
//		rebuildGamen( 0 );
//		setTurnGamenThis( NULL );
//		PNLC_LdapSearchManager::newInstance()->clearLdapSearchCondition();
//		PNLC_LdapSearchManager::newInstance()->clearLdapMultiSearchCondition();
//
//		if( ( ( nextGamen != NULL )
//			||( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_IGCClearWait_popup ) )
//			||( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_IGCWait_popup ) ) )
//		 && ( True == onlyPanel.bl_BrowserFileUploadRequest ) ){
//			uchar uc_UploadType = 0;
//			if( &gp_FileUploadTypeSelectExt_browser == getNowGamen() ){
//				uc_UploadType = PCGD_BrowserFileUploadTypeSelect;
//			}
//			deleteUploadInfo( uc_UploadType );
//		}
//
//		onlyPanel.setTopMenuSpecifiedAppState( PNLE_SpecifiedAppState_Idle );
//		if( ( NULL != nextGamen ) ||
//		    ( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_IGCClearWait_popup ) ) ){
//			onlyPanel.setTopmenuMode( False );
//		}
//		else if( ( NULL == nextGamen ) && ( False == c_ExtPanel.checkPopupWarningFromTopMenu() ) ){
//			c_ExtPanel.ChangeSpecifiedAppState( e_State );
//		}
//	}else{
//		setBuzzerType( PCGE_BuzzerType_InputNg );
//	}
//}

// comment 2023/03/06 Sky東島 [IT6_2.3][OP_BTS-41756].
//================================================================================================.
// 概要 : PCPC_WebUIWait_popup : 画像転送状態の設定.
// Bool bl_status: True:画像転送が有効  False:画像転送が無効.
// 必ず画面表示後に設定すること.
//================================================================================================.
void PCPC_WebUIWait_popup::setImageTransferStatus( Bool bl_status )
{
	m_bl_ImageTransferStatus = bl_status;
}

//================================================================================================.
// 概要 : PCPC_WebUIWait_popup : 画面開始処理.
// comment 2018/10/22 SCD2-Sky竹菴 [Eagle][1061802] WebUIへの遷移準備完了通知までに透過のお待ちください画面を表示させる.
//================================================================================================.
void PCPC_WebUIWait_popup::startGamen()
{
	PPopupAnimationCom::startGamen();
// comment 2023/03/06 Sky東島 [IT6_2.3][OP_BTS-41756].
	m_bl_ImageTransferStatus = True;
	APIC_MultiAutoTest *pc_MultiAutoTest = APIC_MultiAutoTest::newInstance();
	if( NULL != pc_MultiAutoTest ){
		// EP_PNS_Setting → EP_PNS_Setting以外 の状態変化をAPIC_MultiAutoTest : DBGへ通知.
		pc_MultiAutoTest->setPanelStatus_NotSetting();
	}
}
//================================================================================================.
// 概要 : PCPC_WebUIWait_popup : 画面終了処理.
// comment 2018/10/22 SCD2-Sky竹菴 [Eagle][1061802] WebUIへの遷移準備完了通知までに透過のお待ちください画面を表示させる.
//================================================================================================.
void PCPC_WebUIWait_popup::endGamen()
{
// comment 2023/03/06 Sky東島 [IT6_2.3][OP_BTS-41756].
	if( False == m_bl_ImageTransferStatus ){
		PNLC_QWSConnector* pc_QWSConnector = PNLC_QWSConnector::newInstance();
		if( ( NULL != pc_QWSConnector )
		 && ( &gp_AppChangeUIBrowser != nextGamen )
		 && ( &gp_AuthOapBrowserWaitEx != nextGamen ) ) {
			// gp_AppChangeUIBrowser,gp_AuthOapBrowserWaitExでは画面開始時等で画像転送の制御を行っているため.
			// それ以外の画面に遷移する場合は画像転送を再開する.
			pc_QWSConnector->setImageTransfer( True );				// 画像転送を再開する.
		}
		m_bl_ImageTransferStatus = True;
	}
	APIC_MultiAutoTest *pc_MultiAutoTest = APIC_MultiAutoTest::newInstance();
	if( NULL != pc_MultiAutoTest ){
		// EP_PNS_Setting以外 → EP_PNS_Setting の状態変化をAPIC_MultiAutoTest : DBGへ通知.
		pc_MultiAutoTest->setPanelStatus_Setting();
	}
	if( PNLE_TabletConnectionRequsetState_None != onlyPanel.m_e_pendingTabletConnectionRequest ){
		// 退避していたタブレット接続/切断要求を再発行する.
		if( PNLE_TabletConnectionRequsetState_Connect == onlyPanel.m_e_pendingTabletConnectionRequest ){
			pc_PanelManager->setConnectState( True, False );
		}else if( PNLE_TabletConnectionRequsetState_Disconnect == onlyPanel.m_e_pendingTabletConnectionRequest ){
			pc_PanelManager->setConnectState( False, False );
		}
		onlyPanel.m_e_pendingTabletConnectionRequest = PNLE_TabletConnectionRequsetState_None;
	}
	PPopupAnimationCom::endGamen();
// comment 2022/10/04 Sky中野 [IT6_2.3][OP_BTS-39700].
	c_autoRestTimer.resetAll();
}
//================================================================================================.
// 概要 : PCPC_WebUIWait_popup : 表示部品のセットアップ.
// comment 2018/10/22 SCD2-Sky竹菴 [Eagle][1061802] WebUIへの遷移準備完了通知までに透過のお待ちください画面を表示させる.
//================================================================================================.
void PCPC_WebUIWait_popup::setupGamen( short )
{
	printLEDControl( False );
// comment 2022/10/04 Sky中野 [IT6_2.3][OP_BTS-39700].
	c_autoRestTimer.stopAll();

// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )
	PnlComFooterPartsBtnActiveChange(homeKeyActiveCheck(), idKeyActiveCheck(), resetKeyActiveCheck());
#endif		//  ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus)  && defined (DEF_LCD_7INCH) ) )	.
}
// comment 2023/03/06 Sky東島 [IT6_2.3][OP_BTS-41756].
//================================================================================================.
// 概要 : PCPC_WebUIWait_popup : 画面非アクティブ処理.
//================================================================================================.
void PCPC_WebUIWait_popup::procInactivate()
{
// comment 2023/02/27 Sky東島 [IT6_2.3][OP_BTS-41756].
	if( False == m_bl_ImageTransferStatus ){
		PNLC_QWSConnector* pc_QWSConnector = PNLC_QWSConnector::newInstance();
		if( NULL != pc_QWSConnector ) {
			pc_QWSConnector->setImageTransfer( True );				// 画像転送を再開する.
		}
		m_bl_ImageTransferStatus = True;
	}
	PPopupAnimationCom::procInactivate();
}

//================================================================================================.
// 概要 : PCPC_WebUIWait_popup : 自身の下階層を黒透過するかどうかを返す.
// comment 2018/10/22 SCD2-Sky竹菴 [Eagle][1061802] WebUIへの遷移準備完了通知までに透過のお待ちください画面を表示させる.
//================================================================================================.
Bool PCPC_WebUIWait_popup::getBackLayerDarkness()
{
	return False;
}
//================================================================================================.
// 概要 : PCPC_WebUIWait_popup : 全画面警告(ERROR､CAUTION)発生／解除.
// comment 2018/10/22 SCD2-Sky竹菴 [Eagle][1061802] WebUIへの遷移準備完了通知までに透過のお待ちください画面を表示させる.
//================================================================================================.
void PCPC_WebUIWait_popup::procWarningGamenExist( uchar )
{
	//本画面で警告検知はしない.
	return;
}
//================================================================================================.
// 概要 : PCPC_WebUIWait_popup : タブレット接続/切断処理.
// comment 2018/10/22 SCD2-Sky竹菴 [Eagle][1061802] WebUIへの遷移準備完了通知までに透過のお待ちください画面を表示させる.
//================================================================================================.
void PCPC_WebUIWait_popup::procTabletConnectChange( MessageType message, uchar )
{
	// 接続/切断要求を一時的に退避して、ブラウザ起動完了後に再度、接続/切断処理を行う.
	if( MS_TabletConnect == message ){
		onlyPanel.m_e_pendingTabletConnectionRequest = PNLE_TabletConnectionRequsetState_Connect;
	}else if( MS_TabletDisconnect == message ){
		onlyPanel.m_e_pendingTabletConnectionRequest = PNLE_TabletConnectionRequsetState_Disconnect;
	}
}

// comment 2022/09/15 Sky東島 [IT6_2.3][OP_BTS-38737].
//================================================================================================.
// 概要 : PCPC_WebUIWait_popup : オブザーバからのコール.
//================================================================================================.
void PCPC_WebUIWait_popup::procObserver( TYPE_PNL_Observer e_option )
{
	// PPopupComのprocObserverからはベース画面処理がコールされる.
	// ベース画面処理によっては基底のprocObserverがコールされない場合があるため.
	// PLayerControlのprocObserverをコールし処理漏れが発生しないようにする.
	PLayerControl::procObserver( e_option );
}
