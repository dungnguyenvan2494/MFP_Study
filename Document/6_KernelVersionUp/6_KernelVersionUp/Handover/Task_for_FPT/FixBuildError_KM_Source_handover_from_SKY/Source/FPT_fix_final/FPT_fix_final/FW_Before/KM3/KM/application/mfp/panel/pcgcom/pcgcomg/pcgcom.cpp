/*================================================================================================
［タイトル］	基本メッセージ処理（抽象クラス）
================================================================================================*/
/*================================================================================================
	インクルード・ファイル 
================================================================================================*/

#include "GFWC_System.h"
#include "GFWC_Window.h"

#include <system/thr/thrInclude/THRC_LEDStatus.h>	//---【スレッドインクルード対応】thrcom → thrInclude 2008/04/17 制御12 高松.
#include <brwpnl/brwpnlg/BRWC_BasicMessageProc.h>
#include <panel/plibh/plibh_CLCD2/PNLC_AuthenticationManager.h>

#include <system/thr/PanelHW/THRC_PanelHardCom.h>
#ifdef QT_PANEL
#include <panel/plibl/plibl_CLCD/qtpanel.h>
#endif
#include <system/prt/prtg/PRCC_TrayHPMover.h>
#include <lib/ccdefine.h>
#include "system/fax/APIC_FaxDevice.h"
#include "system/ats/APIC_AutoSelect.h"
#include <system/saa/APIC_UsbMemDeviceInfo.h>
#include <panel/pcgcom/pcgcom_CLCD/pcgcom_CLCD.h>
#include <panel/pcgcom/pcgcom_CLCD/pigcom_CLCD.h>
#include <panel/pcgcom/pcgcom_CLCD/pipcom_CLCD.h>
#include <panel/plibl/plibl_CLCD/graphlib_CLCD.h>
// Comment 2019/04/30 【IT6_EagleH】【HCLT_Ranjith】【OP_BTS-6373】 New Gamen support.
#include <panel/pcgact/pcgact_CLCD/pigact_CLCD.h>
#include "pimsg.h"
#include <panel/plibh/plibhg/pdpcomb.h>
#include <panel/plibh/plibh_CLCD/pdpcomb_CLCD.h>
#include <panel/pdp/pdpg/pdpc.h>
#include <panel/plibl/pliblg/tskpnlcpu.h>
#include <panel/pcgwarn/pcgwarng/pcwrntbl.h>
#include <panel/pcgcopy/pcgcopy_CLCD/pigcopyenl_CLCD.h>
#include <panel/pcgscanfax/pcgscanfax_CLCD/pigscanfaxenl_CLCD.h>
#include <panel/pcgwarn/pcgwarng/pcwrntblenl.h>
#include <panel/pcgact/pcgact_CLCD/pigactenl_CLCD.h>
#include  <string.h>
#include <system/sys/sysg/appInit.h>
#include <panel/pcgutil/pcgutilg/pctutilityg.h>
#include "pctcomg.h"
#include <panel/pcgscansend/pcgscansendg/pcgscansend.h>
#include <panel/pcgscansend/pcgscansend_CLCD/pigscansend_CLCD.h>
#include <panel/pcgscanoap/pcgscanoapg/pccscanoap.h>
#include <panel/pcgscanoap/pcgscanoapg/pigscanoap.h>

#include <panel/plibl/plibl_CLCD/tskpnldraw_CLCD.h>
#include <panel/pnlmngr/pnlmngrg/pnllog.h>
#include "system/igl/APIC_ImageLogIF.h"

#include "panel/pdp/pdpg/pdpdebug.h"
#include <typeinfo>
extern void Browser_DebugPrint( char* str , intptr_t val1 ,intptr_t val2 , intptr_t val3 , intptr_t val4 , intptr_t val5 , intptr_t val6 ) ;
#include <panel/pcgcom_ext/pcgcom_extg/PCGC_AplCommonSet.h>
#include <panel/pcgcom_ext/pcgcom_extg/pigAuthOapBrowserLogin.h>
#include <panel/pcgbrowser_ext/pcgbrowser_extg/pigScanOapBrowserDummy.h>
#include <panel/pcgbrowser_ext/pcgbrowser_extg/pigPullPrintOapBrowserDummy.h>
#include <panel/pcgbrowser_ext/pcgbrowser_extg/pigGeneralOapBrowserDummy.h>
#include <panel/pcgbrowser_ext/pcgbrowser_extg/PCGC_AuthOapBrowserWaitEx.h>
#include <panel/pcgcom_ext/pcgcom_extg/pigAppChangeBrowser.h>
#include <panel/pcgcom_ext/pcgcom_extg/pigAppChangeScreenSaver.h>
#include <panel/pcgbrowser_ext/pcgbrowser_extg/pigAuthOapBrowserWait.h>
#include <panel/pcgcom_ext/pcgcom_extg/pigIwsAuthBrowserLogin.h>
#include <panel/pcgbrowser_ext/pcgbrowser_extg/pigIwsAuthBrowserWait.h>
extern PCGC_AplCommonSet AplCommonSet;
#include <panel/plibh/plibh_FUM2/plibh_FUM2.h>
#include <panel/pcgscanfax/pcgscanfax_FUM2/pigscanfax_FUM2.h>
#include <panel/pcgcom/pcgcomg/pcgcom2.h>
#include <panel/pcgact/pcgact_FUM2/pigactenl_FUM2.h>
#include <system/sys/sysg/appInit.h>
#include <panel/plibh/plibhg/plibhvoice.h>
#include <panel/plibh/plibhg/PNL_VoiceCursorControl.h>
#include <panel/plibh/plibhg/PNL_VoiceRequestBuffer.h>
#include <panel/pddvoice/pddvoiceg/pvoice.h>
#include <panel/pcgcom/pcgcom_FUM3/PMGC_PanelVoicePlay.h>
#include <panel/pcgact/pcgact_FUM2/pigact_FUM2.h>
#include <panel/pnlmngr/pnlmngrg/pnlmanager.h>
#include <panel/pcgutil/pcgutil_CLCD/pigacce_CLCD.h>
#include <panel/pcgserv/pcgserv_CLCD/pipserv_CLCD.h>
#include <panel/pcgutil/pcgutil_CLCD/piputility3_CLCD.h>
#include <system/thr/PanelHW/THRC_PPanelHard.h>
#include <panel/pcgcom/pcgcomg/pcgcom1.h>
#include "panel/pcgauthoap/pcgauthoap_CLCD/pcgauthoap1_CLCD.h"
#include "panel/pcgact/pcgact_FUM2/pigact_FUM2.h"
//Comment [2018/03/30] [HCL_Krishna][DeadCodeRemoval]Commented as part of Dead Code Removal.
//#include <panel/pcgact/pcgact_CLCD/pigactenl_PKI.h>
#include <system/sys/APIC_System.h>
#include <panel/pdp/pdp_CLCD/pdpc2_CLCD.h>	//【UV-00783】Sky_hosokawa.
#include <panel/plibh/plibh_CLCD2/PNLC_LimitationFunctionLoginUserCheckLib.h>

#include	"LAPC_NTF_Notify_Setting.h"
// Comment [2016/10/20] [HCLT-Kalaiyarasi] [IT5_4.0][AR1005467] Including the Header Files.
#include    "LAPC_NTF_Notify_info.h"
#include <panel/pcgcom/pcgcom_CLCD2/PNLC_NotifyEventInfo.h>
#include <panel/pcgscanfax/pcgscanfax_enl_CLCD2/pigscanfaxenl_p10H00.h>
#include <panel/pcgscanfax/pcgscanfax_enl_CLCD2/pigscanfaxenl_p10H06.h>
#include <panel/pcgcom/pcgcom_CLCD2/PNLC_ChangeGamenManager.h>
#include <panel/pcgwarn_ext/pcgwarn_ext_CLCD2/pigwarn_pJ0A9012_ext.h>//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/10 Add.

#if defined(SIMULATOR) && !defined(LINUX)
#include "SimDefAll.h"
extern SW_CSimWinView* m_hWndView;
#endif
#include <panel/pcgcom/pcgcom_CLCD2/PNLC_PopupListControl.h>
#include <panel/pcgscanfax/pcgscanfax_CLCD2/pigscanfax_base.h>
#if defined(SIMULATOR) && !defined(LINUX)
extern __declspec( dllimport ) bool m_bVxSimConnectLinux;
#endif
#if defined(LINUX) || defined(SIMULATOR)
#include <panel/pcgcopy/pcgcopy_CLCD/pctcopyenl_CLCD.h>
#endif /*LINUX*/
#include <panel/pcgbrowser/pcgbrowserg/pigbrowser1.h>
#include <panel/pcgauthoap/pcgauthoap_CLCD/pigauthoap1_CLCD.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_base.h>
#include <panel/plibh/plibh_CLCD2/PNLC_ScanFaxDataSearchLib.h>
#include <panel/pcgscanfax/pcgscanfax_CLCD2/pipscanfax_base.h>
#include <panel/plibh/plibh_CLCD2/PNLC_OperationLog.h>
#include <panel/plibh/plibh_CLCD2/PNLC_ScanFaxDataChangeCheckLib.h>
//Comment 2017/09/04 [HCL-Swathi] [IT5 4.2][RQ-686] [WP732]Multiple Network support.
#include <panel/plibh/plibh_CLCD2/PNLC_NetworkLan.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA072001.h>
#include <panel/pcgjob/pcgjob_CLCD2/pigjob_pA061000.h>
#include <panel/pcgadm/pcgadm_CLCD/pigadmin9_CLCD.h>
#include <panel/pcgwarn/pcgwarn_CLCD2/pigwarn_pA0A9080.h>
#include <panel/pcgact/pcgact_CLCD2/pipact_pA072003.h>
#include <panel/pcgutil/pcgutil_CLCD/pigutility_CLCD.h>
#include <panel/pcgcom/pcgcom_CLCD2/PNLC_WarningManager.h>
#include <panel/pcgwarn/pcgwarn_CLCD2/pigwarn_pA0A3010.h>
#include <system/nvd/APIC_Setting.h>
#include <rmp/RMP_WebDavClient.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070702.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA072002.h>

#include <browser/browserg/BRWC_Flash.h>

#include <panel/pcgcom/pcgcom_CLCD2/PNLC_EnlWarningManager.h>
#include <panel/pcgwarn/pcgwarn_enl_CLCD2/pigwarnenl_p10J00.h>
#include <panel/plibh/plibh_CLCD2/PNLC_SnapshotControlLib.h>// comment 2014/07/14 OPS1-Sky村井 [Zeus5BK][制御構成変更No.142] 省電力復帰短縮対応.


#include "../../../../divlib/Common/blkapi/blkshm.h"
//【CQ517977】【392734_Merge_PZH2.1c 市場問題マージ】【HCL_Bhargavi】 2015/09/04 Add Start.
#include <panel/pcgbox_ext/pcgbox_ext_CLCD2/pigbox_pA059000_ext.h>
//【CQ517977】【392734_Merge_PZH2.1c 市場問題マージ】【HCL_Bhargavi】 2015/09/04 Add End.
#include "panel/plibh/plibhg/plibhGuiWrapper.h"


#include <panel/pcgserv/pcgserv_CLCD/pigserv_CLCD.h>


#include "system/sys/APIC_OptionSetting.h"	// 2013/02/20 【OP-PRV-01】プレビューオプション化 日立JTE 奈良.
#include <system/sys/APIC_BGM.h>	// comment 2013/05/13 OPS1-Sky奥村 [ZM3rd][制御構成変更No1200,1220] 音声ガイダンスONで音声ガイド状態(CommonAPI)ONを設定する.
#include  "panel/plibh/plibh_CLCD2/PNLC_QWSConnector.h"
#include <panel/plibh/plibh_CLCD/plibh_CLCD1.h>
#include <system/sys/APIC_TabletIF.h>
#include <system/nvd/APIC_Setting.h>
#include "browser/browserg/BRWC_Browser.h"	// 2013/03/13 k.morita [操作性改善FUMv4]CQ.378439 ブラウザ初期化処理.
#include "panel/pcgact/pcgact_CLCD2/pigact_pC072001.h"				// 2013/05/08 k.komaba CQ378447,CQ378450.
#include "panel/pcgcom_ext/pcgcom_extg/PCGC_PublicPopupDummyBase.h"
#include <panel/pcgwarn/pcgwarn_CLCD2/pigwarn_CLCD2.h>
#include <panel/plibh/plibh_CLCD2/PNLC_TabletConnectRestrictLib.h>
#include <panel/plibh/plibh_CLCD2/PNLC_ActionUserLoginControlLib.h>
#include <panel/plibh/plibh_CLCD2/PNLC_TabletResetControlLib.h>
#include "panel/plibh/plibh_CLCD2/PNLC_RemotePanelLib.h"

// comment 2015/12/01 OPS1-Sky稲井 [特ROM][IT5-8_機種マージ]e-Mail受信時、本文を印刷する要望(他1件).
#include <panel/pcgwarn/pcgwarn_CLCD2/pigwarn_pA0A3202.h>

#include <panel/pcgcopy/pcgcopy_enl_CLCD2/pigcopyenl_p10G00.h>

#include <panel/pcgact/pcgact_enl_CLCD2/pipactenl_pCommon.h>
#include <panel/pcgwarn/pcgwarn_enl_CLCD2/pigwarnenl_p80M00.h>
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_p10M00.h>
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_p10M21.h>
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_p10M24.h>
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_p10M30.h>
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_p10M42.h>
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_p10M48.h>
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_p10M70.h>
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_p10M76.h>
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_p10M79.h>
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_p10M90.h>
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_p80M01.h>
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_p80M44.h>
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_p80M49.h>
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_p80M70.h>
#include <panel/pcgcom/pcgcom_CLCD2/PNLC_EnlVoiceGuidanceControl.h>
#include <panel/plibh/plibh_CLCD2/PNLC_ActionDataLib.h>
// comment 2015/04/09 OPS1 Sky東島 [Helios][CQ511770].
#include <panel/pcgact/pcgact_CLCD2/pigact_pA072000.h>

#include "panel/plibh/plibh_CLCD/plibh_CLCD.h"  // 2013/11/05 酒井(日立JTE) パネルのプレビュー用バッファ領域削減.
#include "panel/ppcg/ppcg_CLCD/ppcgwidget_CLCD.h"
// comment 2016/06/01 MSE大西 [ZeusSBK][CQ527296]ボタン押下判定切り替え.
#include "panel/ppcg/ppcg_CLCD/ppcgbuttoneventmanager_CLCD.h"

#include "panel/pcgcom/pcgcomg/dyncreate_debug.h"

#include "system/sys/APIC_AndroidPanel.h"

// for Gamen Cache
#include "panel/pcgcopy/pcgcopy_CLCD2/pigcopy_base.h"
#include "panel/pcgbox/pcgbox_CLCD2/pigbox_base.h"
#include "panel/pcgscanfax/pcgscanfax_CLCD2/pigscanfax_base.h"
#include <panel/plibh/plibh_CLCD2/PNLC_SlideMenuControlLib.h>
#include "panel/plibh/plibh_CLCD2/PNLC_ExtendAreaGeneralization.h"
#include "panel/pcgscanfax/pcgscanfax_CLCD2/pigscanfax_pA032000.h"

#include "panel/pcgcom/pcgcom_CLCD2/pipcom_CLCD2.h"

#include <panel/ppcg/ppcgg/ppcglanguageNewBin.h>

#include "panel/plibh/plibh_CLCD2/PNLC_SendPanelMainEventLib.h"
#include <panel/pcgbrowser_ext/pcgbrowser_extg/pigCostCenterResetDummy.h>//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/10 Add.
#include <system/uac/APIC_PASM.h>

// 認証集約FUM 新規画面インスタンス.
#include "panel/pcgact/pcgact_enl_CLCD2/pigactenl_auth_user_seqcustom.h"
// 2015/03/04 [MAv3][RQ-245]HDDリストア結果画面が表示されない 日立 村上(幸).
#include "system/sys/APIC_BackupRestore.h"

#include "panel/pcgcopy/pcgcopy_CLCD2/pigcopy_pA022000.h"
#include "panel/pcgscanfax/pcgscanfax_CLCD2/pigscanfax_pA030200.h"
#include "panel/pcgbox/pcgbox_CLCD2/pigbox_pA053700.h"
#include "panel/pcgbox/pcgbox_CLCD2/pigbox_pA052200.h"
#include "panel/pcgbox/pcgbox_CLCD2/pigbox_pA051600.h"
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_p80M01.h>
#include <panel/pcgwarn/pcgwarn_enl_CLCD2/pigwarnenl_p10J20_seqcustom.h>
// comment 2016/06/09 SCD2-Sky浜野 [ZeusSBK][CQ529698] 新詳細コード拡張対応.
#include "../LocalUI/panel/Shell/DebugTask.h"
#include <panel/pcgscanfax/pcgscanfax_PD/pcgscanfax_A032000100.h>
extern PA032000100 gp_A032000100;
#include <panel/pcgscanfax/pcgscanfax_enl_PD/pcgscanfaxenl_10H00500.h>
extern PPD_10H00500 gp_PD_10H00500;
// Comment 2017/01/20 第2システム制御開発 Sky上野 [ServiceHub] SHRQ-002 SH上でのエラー警告対応.
#include <system/sys/APIC_SHSystemControl.h>
#include <panel/plibh/plibh_CLCD2/plibh_CLCD2.h>						// クラウド接続エラー検知用.
#include <panel/pcgscanfax/pcgscanfax_CLCD/pcgscanfaxWebService_CLCD.h> // WSDSスキャンエラー検知用.
// Comment 2017/02/21 第2システム制御開発 Sky上野 [ServiceHub][SHRQ-002] 課題番号1013443対応.
#include <panel/pcgwarn/pcgwarn_CLCD2/pcgwarn_pA0A9001.h>				// PRT/IR関連異常警告テーブル参照用.
#include <system/thr/thrInclude/THRC_PortInstance.h>					// PRT/IR関連異常警告 CC_WarningStatus	comWarning参照用.
#ifdef DEF_A4_Cus
#include <panel/plibh/plibh_VenusMLK/PNLC_LoggerForPerformanceLib.h>
#endif	// DEF_A4_Cus.
#include <panel/pcgscanfax/pcgscanfax_enl_PD/pcgscanfaxenl_10H00790.h>
extern PPD_10H00790 gp_PD_10H00790;
#include <panel/pcgscanfax/pcgscanfax_enl_PD/pcgscanfaxenl_10H00480.h>
extern PPD_10H00480 gp_PD_10H00480;
#include <panel/pcgscanfax/pcgscanfax_enl_PD/pcgscanfaxenl_10H00400.h>
extern PPD_10H00400 gp_PD_10H00400;
// comment 2017/03/01 SCD2-Sky浜野 [IT5_4.1][RQ-643][1015199] SuperWarp実行条件変更対応.
#include <panel/pdp/pdpg/pdp_CauseCode.h>

// Comment -> 2017/05/12 SCD2 3G Sky瀬戸 [AccuRev.1014826]media-ready対応.
#ifdef DEF_A4_Cus
#include <panel/plibh/plibh_VenusMLK/PNLC_PaperSizeTypeSettingLib.h>
#endif // DEF_A4_Cus.
// Comment <- 2017/05/12 SCD2 3G Sky瀬戸 [AccuRev.1014826]media-ready対応.
//comment 2017/07/05 SCD2-Sky肥後 [1026857] 新詳細コード対応.
#include <panel/pdp/pdpg/pdp_DetailCode.h>

#include "panel/plibh/plibh_CLCD2/PNLC_KeyboardControlLib.h"
#include <panel/pcgcom_ext/pcgcom_extg/PCGC_BrowserFileUploadComBase.h>		// comment : WebUI2 画面切替え制御 20180302 Chikuan.
#include <panel/pcgcom_ext/pcgcom_extg/PCGC_WarningPopupDummyBase.h>		// comment : WebUI2 画面切替え制御 20180302 Chikuan.
#include <panel/pcgcom_ext/pcgcom_extg/pigAppChangeUIBrowser.h>				// comment : WebUI2 画面切替え制御 20180302 Chikuan.
#include <panel/pdp/pdpg/pdpc_WebUIControl.h>								// comment : WebUI2 画面切替え制御 20180302 Chikuan.
#include <panel/pdp/pdpg/pdpc_WebUIDebug.h>									// comment : WebUI2 画面切替え制御 20180302 Chikuan.
#include <panel/pdp/pdpg/pdpc_WebUIToken.h>									// comment : WebUI2 画面切替え制御 20180302 Chikuan.
#include <panel/pdp/pdpg/pdpc_WebUIScreenInfo.h>							// comment 2018/08/21 SCD2-Sky竹菴 [Eagle][1057602].

// comment 2018/05/22 SCD2 T.Hori [Eagle/SparrowMFP][制御構成変更_No.111_新UI対応_認証画面対応].
#include <panel/pcgact/pcgact_CLCD2/pipact_base.h>
//Comment 2018/12/06 [HCLT-Rajkumar]【IT6_DenebMLK】Conflict Manual Merge Activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
//Comment 2018/12/12 [HCLT-Prabakaran]【SparrowBuild】support.
//#if !defined(DEF_DenebMLKCus)
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
#include <panel/plibh/plibh_CLCD2/PNLC_SH3PaneMenuControlLib.h>
// comment 2018/09/10 SCD2 K.Takei 不具合No.1053638_【開発提起】1ペイン/3ペインの切り替え時、3ペインエリアとメインエリアの表示タイミングがずれており、見栄えが悪いので改善する.
#include <browser/browserg/BRWC_define.h>
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
// comment 2018/10/12 Sky東島 [1062683] 2ペイン化対応.
#include <panel/pcgwarn/pcgwarn_CLCD2/pigwarn_usercert_seqcustom.h>
#include <panel/pcgwarn/pcgwarn_CLCD2/pigwarn_pA0A3900.h>
#include <panel/pcgwarn/pcgwarn_CLCD2/pigwarn_pG0A9003.h>
#ifdef DEF_A4_Cus
#include <panel/pcgact/pcgact_VenusMLK/pigact_VenusMLK.h>
#else  // DEF_A4_Cus
#include <panel/pcgwarn/pcgwarn_CLCD2/pigwarn_pP0A9003.h>
#endif  // DEF_A4_Cus
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070200.h>
#include <panel/pcgact/pcgact_PD/pcgact_A072002900.h>
extern PA072002900 gp_A072002900;
//#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
// comment 2018/05/12 Sky東島 [IT6][KRQ-314-29] 目的別UI.
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070100.h>
// comment 2018/06/07 SCD2-Sky中野 [Eagle] Legacy追加対応.
#include <panel/pcgact/pcgact_CLCD2/pigact_p7_0012.h>

// comment 2018/05/22 SCD2 T.Nishiba [Eagle][制御構成変更No.111]新UI化対応.
#include "panel/pcgscanfax_ext/pcgscanfax_ext_CLCD2/pigscanfax_pA039000_ext.h"
#include "system/dbg/APIC_DebugLogSystem.h"		// comment 2018/05/18 SCD2-Sky東野 [IT6] [1044346] 不具合対応 .
// comment 2018/06/09 Sky東島 [IT6][KRQ-314-29] 目的別UI.
#include <panel/pcgbox/pcgbox_CLCD2/pigbox_pX-000.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070701.h>		// comment 2018/06/01 SCD2 K.Sato [Eagle][制御111]新UI化対応.
// 2018/06/09 Sky弥村 [IT6][制御構成変更No.252] 新UIデザイン_用紙設定画面.
#include <panel/pcgact/pcgact_CLCD2/pigact_p7_000.h>
// comment 2018/06/19 Sky武井 [IT6][制御構成変更No.252] 用紙設定の追加.
#include <panel/pcgact/pcgact_CLCD2/pigact_p7_001.h>
//Comment 2018/07/31 HCL_Devipriya 制御構成変更_No.104.
#include <panel/pcgserv/pcgservc/pcgserv2c.h>
#include "../../../../divlib/client/Proxy/system/sys/APIC_SystemServiceFunction.h"
// comment 2018/10/22 SCD2-4G 小畑 [Eagle][AR1063625].
#include <panel/plibh/plibh_CLCD2/PNLC_SelectPaperTrayCom_DataLib.h>
// comment 2018/09/28(金) SCD2-Sky長谷川(伸) [市場問題対応][FQ-ISD2G-1800055-OFP161].
#include <panel/pcgcom_ext/pcgcom_extg/PCGC_WarningPopupDummyBase.h>

// comment [特ROM対応][IT5-148]【SOLIMP3様】音声操作によるジョブ実行(IWS連携)要望_最終版リリース(USB検知音変更).
#include "system/sys/APIC_USBSetUnsetSound.h"

// Comment 2019/01/09 【HCL_Ranjith】【IT6_EagleH】【RQ-506】 include the header file.
#include "../../shlib/RecoveryLib/LIBC_RecoveryLib_IF.h"
//Comment [2019/03/08] [HCL_Devipriya] [EagleH] [機器構成変更No.29]  use pointer instead of table.			
// comment 2019/11/19 Sky小尾 EagleZ+ [OP_BTS-14914].
#include <panel/pcgutil/pcgutil_CLCD/pigutilityc_CLCD.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_base.h>
// Comment 2020/06/12 [HCLT-Ranjith] [IT62.0_MR3] [制御構成変更No.34] one pane display gamen 
#include <panel/pcgact/pcgact_CLCD2/pigact_pQ070000000.h>
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_pQ0M0000000.h>
// comment 2020/06/05 K.Kasemura PKI認証のBasicモード利用可能対応.
#include <panel/pcgwarn/pcgwarn_CLCD2/pigwarn_pA0A3204.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070706.h>
// comment 2020/10/16 Sky東島 [IT5-67] LDAPアプライアンスと生体認証の組み合わせでのセルフレジストの対応.
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_p10M19.h>
// comment 2021/04/07 2DWS2G-Sky毛穴 [IT6_2.2][OP_BTS-32059] カウンタフル全画面警告表示後オートリセットでパブリックログイン.
#include <panel/pcgact_ext/pcgact_ext_CLCD2/pigact_pA072002_ext.h>
// comment 2022/01/28 Sky東島 [IT6_2.2MR2][制御34]ユーザーセットアップ.
#include <panel/pcgbrowser_ext/pcgbrowser_extg/pigUserSetupBrowser.h>
// comment 2022/06/22 Sky東島 [IT6_2.3][制御構成変更_No.51][サービスレスメンテナンス].
#include <panel/pcgbrowser_ext/pcgbrowser_extg/pigPopupBrowser.h>

// 2022/03/30 生産RINNE対応 Sky start.
#include <panel/pcgcom_ext/pcgcom_extg/pigAppChangeBrowserLineMode.h>
#include <panel/pcgcom_ext/pcgcom_extg/pigAppChangeBrowserLineModeWait.h>
// 2022/03/30 生産RINNE対応 Sky end.

#include <panel/plibh/plibh_CLCD2/PNLC_SPSAuthDataLib.h>	// 2022/7/27 Sky higashino [特ROM][SPS]DipSW対応.
//comment 2022/09/20 FPT-KhaiPQ [IT6_2.3] 機器構成変更_No.47.
#include <panel/pcgwarn/pcgwarn_enl_CLCD2/pigwarnenl_p80J10.h>
// comment 2023/12/05 FPT-HungDm17 [IT6_2.3][OP_BTS-45944]ユーザー名入力画面で、システムオートリセットが働かない問題対策.
#include <panel/pcgact/pcgact_CLCD2/pigact_pA072001.h>
#include <panel/pcgact_ext/pcgact_ext_CLCD2/pcgact_pA070700.h>
// comment 2022/12/06 FPT-DuongDV11 [IT6_2.3][OP_BTS-40046]【開発提起【IT6_V102】文書ファイル処理画面でシステムオートリセット時間を待ち、スクリーンセーバー起動した画面でフッターバーが表示された
#define GFWE_WSVGA_RESOLUTION_X		(1024)
#define GFWE_WSVGA_RESOLUTION_Y		(600)
const float m_lcdSize10_1inch	= 10.1f;

WarningLabel (*mp_PrtIrWarningList)[WARNING_LIST_NUM];
struct PNLS_FuncBadge_Cache
{
	PBasicMessageProc* pc_PopGamen;
	short ss_baseGamenListCnt;
	PBasicMessageProc** ppc_BaseGamenList;
	Bool b_Start;
	TYPE_AppliedSettingDisp e_AppSetDisp;
};
// コピー.
PBasicMessageProc* gpc_CopyBaseGamen[] =
{
	&gp_CopyBase
};
short gss_CopyBaseGamenCnt = sizeof( gpc_CopyBaseGamen ) / sizeof( PBasicMessageProc* );
// ファックススキャン.
PBasicMessageProc* gpc_ScanfaxBaseGamen[] =
{
	&gp_FromRegAddress_scanfax
	,&gp_DirectInput_scanfax
	,&gp_ScanFaxHistoryAddress
	,&gp_AddressSearch_scanfax
};
short gss_ScanfaxBaseGamenCnt = sizeof( gpc_ScanfaxBaseGamen ) / sizeof( PBasicMessageProc* );
// ボックス送信.
PBasicMessageProc* gpc_BoxSendBaseGamen[] =
{
	&gp_FromRegAddress_box
	,&gp_Direct_box
	,&gp_FromTxLog_box
	,&gp_AddrSearch_box
};
short gss_BoxSendBaseGamenCnt = sizeof( gpc_BoxSendBaseGamen ) / sizeof( PBasicMessageProc* );
// ボックス印刷.
PBasicMessageProc* gpc_BoxCopyBaseGamen[] =
{
	&gp_SetUpChangeMenu_box
};
short gss_BoxCopyBaseGamenCnt = sizeof( gpc_BoxCopyBaseGamen ) / sizeof( PBasicMessageProc* );
// ボックス保存.
PBasicMessageProc* gpc_BoxSaveBaseGamen[] =
{
	&gp_DocKeep_box
};
short gss_BoxSaveBaseGamenCnt = sizeof( gp_DocKeep_box ) / sizeof( PBasicMessageProc* );
// 応用設定画面キャッシュリスト.
PNLS_FuncBadge_Cache g_FuncBadgeCacheList[] =
{
	 { &gp_Auxil_copy,						gss_CopyBaseGamenCnt,		gpc_CopyBaseGamen,		False,	TYPE_AppliedSettingDispMapScroller }
	,{ &gp_AuxilTabDisp_copy,				gss_CopyBaseGamenCnt,		gpc_CopyBaseGamen,		False,	TYPE_AppliedSettingDispTab }
	,{ &gp_ReadSetApplied_scanfax,			gss_ScanfaxBaseGamenCnt,	gpc_ScanfaxBaseGamen,	False,	TYPE_AppliedSettingDispMapScroller }
	,{ &gp_ReadSetAppliedTabDisp_scanfax,	gss_ScanfaxBaseGamenCnt,	gpc_ScanfaxBaseGamen,	False,	TYPE_AppliedSettingDispTab }
	,{ &gp_SendAppSettings_box,				gss_BoxSendBaseGamenCnt,	gpc_BoxSendBaseGamen,	False,	TYPE_AppliedSettingDispMapScroller }
	,{ &gp_SendAppSettingsTabDisp_box,		gss_BoxSendBaseGamenCnt,	gpc_BoxSendBaseGamen,	False,	TYPE_AppliedSettingDispTab }
	,{ &gp_CopyAuxilPage_box,				gss_BoxCopyBaseGamenCnt,	gpc_BoxCopyBaseGamen,	False,	TYPE_AppliedSettingDispMapScroller }
	,{ &gp_CopyAuxilPageTabDisp_box,		gss_BoxCopyBaseGamenCnt,	gpc_BoxCopyBaseGamen,	False,	TYPE_AppliedSettingDispTab }
	,{ &gp_DocSaveAppSettings_box,			gss_BoxSaveBaseGamenCnt,	gpc_BoxSaveBaseGamen,	False,	TYPE_AppliedSettingDispMapScroller }
	,{ &gp_DocSaveAppSettingsTabDisp_box,	gss_BoxSaveBaseGamenCnt,	gpc_BoxSaveBaseGamen,	False,	TYPE_AppliedSettingDispTab }
};

// GamenCacheList
const PBasicMessageProc* GamenCacheList[] =
{
	&gp_CopyBase,
	&gp_TopMenu_box,
	&gp_FromRegAddress_scanfax,
	&gp_CopyBaseDirect,
// comment 2016/05/25 MSE荒川 [ZeusSBK][CQ530053]  宛先検索キーボードポップアップ反応評価対策.
	&gp_AddressSearch_scanfax,		//!<A034000000 宛先検索.
	&gp_PublicPopupDummyBase,				// comment 2023/09/14 Sky東島 [IT6_2.3][OP_BTS-44767].
};
const PBasicMessageProc* GamenCacheListScalable[] =
{
	&gp_CopyBase,
	&gp_TopMenu_box,
	&gp_FromRegAddress_scanfax,
	&gp_CopyBaseDirect,
	
	&gp_DirectInput_scanfax,		//!<A032000000 宛先設定：直接入力.
	&gp_FaxDirectInput_scanfax,		//!<H032100000 ファクス宛先設定
	&gp_ScanFaxHistoryAddress,		//!<A033000000 履歴から選択.
	&gp_AddressSearch_scanfax,		//!<A034000000 宛先検索.
	&gp_A032000100,	//!<A032000100 直接入力：ファクス送信.
	&gp_PublicPopupDummyBase,				// comment 2023/09/14 Sky東島 [IT6_2.3][OP_BTS-44767].
};
// 起動シーケンス中キャッシュ生成画面リスト.
PBasicMessageProc* PrepareGamenCacheList[] =
{
	&gp_CopyBase,
	&gp_TopMenu_box,
	&gp_FromRegAddress_scanfax,
	&gp_CopyBaseDirect,
};
// comment 2015/04/30 OPS1-Sky佐藤 [HeliosL][CQ513137] ズームキーの登録キー割り当て対応.
struct sc_ZoomLevelTbl {
	PNLE_ZOOM_SATUS		ZoomStatus;
	qreal				ZoomLevel;
};

const sc_ZoomLevelTbl tbl_ZoomLevel[] = {
	{	PNLE_ZOOM_SATUS_MIN,	1.0	},
	{	PNLE_ZOOM_SATUS_LV1, 	1.2	},
	{	PNLE_ZOOM_SATUS_LV2, 	1.5	},
	{	PNLE_ZOOM_SATUS_MAX, 	2.0	},
};
// Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 Create an array for the less than B6 paper types.
//comment 2018/04/08 【HCLT-Prasanjet】 【IT6 EAGLE】 【AR1042477】 remove unnessery paper .
static const TYPE_SizeCode tbl_lessB6Sizes[] = {
//	TYPE_SizeCodeA5Lef,
	TYPE_SizeCodeA6Sef,
	TYPE_SizeCodeB6Sef,
//	TYPE_SizeCode16KSef,
	TYPE_SizeCodePostcard_JpSef,
//	TYPE_SizeCodeEnvelopeMornarch,
//	TYPE_SizeCodeEnvelopeMornarch,
//	TYPE_SizeCodeEnvelopeCom10,
//	TYPE_SizeCodeEnvelopeDL,
//	TYPE_SizeCodeEnvelopeC5,
//	TYPE_SizeCodeEnvelopeB5,
//	TYPE_SizeCodeEnvelopeC4,
//	TYPE_SizeCodeJpLetterY2,
//	TYPE_SizeCodeJpLetterL3,
//	TYPE_SizeCodeJpLetterL4,
//	TYPE_SizeCodeJpLetterY3,
//	TYPE_SizeCodeJpLetterY4,
//	TYPE_SizeCodeJpLetterK1,
//	TYPE_SizeCodeJpLetterK2,
//	TYPE_SizeCodeJpLetterK3,
	TYPE_SizeCodePostcard_EuSef,
	TYPE_SizeCodeIN4x6Sef,
//comment 2019/01/25 [HCL-Chinna] [EagleH] [OP_BTS-1219] Added Non standard size.
	TYPE_SizeCodeNonStandard,
};
// Comment 2016/12/20 第2システム制御開発 Sky上野 [ServiceHub] SHRQ-002 SH上でのエラー警告対応.
Bool bl_SHWarningCheckTableInitialization = False;			// SH 全警告発生状況テーブル初期化フラグ.
#define PNLD_SHWarningJobIDNon 0							// ジョブID初期値. 
#define PNLD_NowDispWarningGamenNon 500						// 現在表示中警告画面初期値.
#define PNLD_SHDeleteJobIDNon 0								// DeleteJobIDなし.
#define PNLD_SHNowWarningAllTblMax 500						// 全警告発生状況確認テーブル最大要素数.
const ushort us_GamenWarningTableMax = ( ( sizeof( sp_GamenWarningTable ) ) / ( sizeof( SP_GamenWarningTable ) ) - 1 );					// 全画面( 通常画面 ) 警告テーブル最大数.
const ushort us_MessageWarningTableMax = ( ( sizeof( sp_MessageWarningTable ) ) / ( sizeof( SP_MessageWarningTable ) ) );				// 2行警告テーブル最大数.
const ushort us_L1MessageWarningTableMax = ( ( sizeof( sp_L1MessageWarningTable ) ) / ( sizeof( SP_L1MessageWarningTable ) ) );			// 下1行警告テーブル最大数.
const ushort us_sp_Attention2WarningTableMax = ( ( sizeof( sp_Attention2WarningTable ) ) / ( sizeof( SP_Attention2WarningTable ) ) );	// ATTENTION2警告テーブル最大数.
const ushort us_sp_OtherWarningTableMax = ( ( sizeof( sp_OtherWarningTable ) ) / ( sizeof( SP_OtherWarningTable ) ) );					// Other警告テーブル最大数.
struct PNLS_NowWarningAllTbl stNowWarningAllTbl[ PNLD_SHNowWarningAllTblMax ];															// 全警告発生状況確認テーブル.
struct TYPS_SHSystemControl_WarningStatus st_SHSystemControl_WarningStatus[ TYPE_SHSystemControl_Warning_TotalNum ];					// CommonAPIへの通知用テーブル.
// Comment 2017/03/27 第2システム制御開発 Sky上野 [ServiceHub][SHRQ-002]追加対応.
const ushort us_GamenEnlWarningTableMax = ( ( us_GamenEnlWarningTableSize ) / ( sizeof( SP_GamenEnlWarningTable ) ) );					// 全画面( 拡大画面 ) 警告テーブル最大数.
struct PNLS_EnlNowWarningAllTbl stEnlNowWarningAllTbl[ PNLD_SHNowWarningAllTblMax ];													// 全警告発生状況確認テーブル( 拡大画面 ).
// Comment 2017/02/21 第2システム制御開発 Sky上野 [ServiceHub][SHRQ-002] 課題番号1013443対応.
#define PNLD_SHPrtIrWarningPage 0

// comment 2019/2/12 Sky黒田（達） [Eagle][操作性指摘_AI-8] ドロップダウン表示変更.
//// comment 2018/05/22 SCD2 T.Hori [Eagle/SparrowMFP][制御構成変更_No.111_新UI対応_認証画面対応].
//PPopupPullDownCom*		gpp_UserAccountUser_act_SelectAuthorityMode_PullDown = Key_UserAccountUser_actNewUI_SelectAuthorityMode_PullDown.getPullDownMenu();
//PPopupPullDownCom*		gpp_UserAccountUser_act_AuthenticationWay_PullDown = Key_UserAccountUser_actNewUI_AuthenticationWay_PullDown.getPullDownMenu();
//PPopupPullDownCom*		gpp_UserAccountUser_act_AuthenticationAndPrint_PullDown = Key_UserAccountUser_actNewUI_AuthenticationAndPrint_PullDown.getPullDownMenu();

//Comment 2018/12/06 [HCLT-Rajkumar]【IT6_DenebMLK】Conflict Manual Merge Activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
//Comment 2018/12/12 [HCLT-Prabakaran]【SparrowBuild】support.
//#if !defined(DEF_DenebMLKCus)
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
// comment 2018/05/22 SCD2 T.Nishiba [Eagle][制御構成変更No.111]新UI化対応.
// 1ペイン画面リスト.
// comment 2018/11/14 Sky雜賀 【Sparrow】.
//static const PBasicMessageProc*	tbl_1PaneGamen[] = {
const PBasicMessageProc*	tbl_1PaneGamen[] = {
	 &gp_ActionInitialSetup		// A072005000	立ち上げ中.
	,&gp_WebUIUSBTop_box			// X-0000_ExternalMemory.				// comment 2018/05/12 Sky東島 [IT6][KRQ-314-29] 目的別UI.
	,&gp_WebUIUSBFileList_box		// X-0001_ExternalMemory_PrintList.		// comment 2018/05/12 Sky東島 [IT6][KRQ-314-29] 目的別UI.
	,&gp_WebUIUSBPrintSetting_box	// X-0002_ExternalMemory_PrintDetail.	// comment 2018/05/12 Sky東島 [IT6][KRQ-314-29] 目的別UI.
	,&gp_WebUIUSBMemoryReading_act	// X-0003_ExternalMemory_LoadingDocument	// comment 2018/05/12 Sky東島 [IT6][KRQ-314-29] 目的別UI.
	,&gp_UtilityMenu				// 10800010 設定メニュー.
	,&gp_WebUITraySelect_box		// X-0014_ExternalMemory_PaperSetting		// comment 2018/06/09 Sky東島 [IT6][KRQ-314-29] 目的別UI.
	,&gp_WebUIFinisherPunch_box		// X-0016_ExternalMemory_StaplePunchSetting	// comment 2018/06/09 Sky東島 [IT6][KRQ-314-29] 目的別UI.
	,&gp_WebUIDuplex_box			// X-0015_ExternalMemory_DuplexSetting		// comment 2018/06/09 Sky東島 [IT6][KRQ-314-29] 目的別UI.
	,&gp_WebUICombine_box			// X-0017_ExternalMemory_Combine		// comment 2018/06/09 Sky森本 [IT6][KRQ-314-29] 目的別UI.
	,&gp_UserAccountUserSelectPopup_act_NewUI // A070701410_ユーザー認証／ユーザー名一覧(ポップアップ). // comment 2018/06/01 SCD2 K.Sato [Eagle][制御111]新UI化対応.
	,&gp_UserAccountDomainName_act_NewUI	// A070701800	サーバー名称一覧.		// comment 2018/06/15 SCD2 T.Hori [Eagle/Sparrow][制御構成変更_No.111_新UI対応_認証画面対応].
// comment 2019/04/17 SCD2-Sky肥後 [EagleH][OP_BTS-3461] 用紙画面のWebUI化対応.
//	// 2018/06/09 Sky弥村 [IT6][制御構成変更No.252] 新UIデザイン_用紙設定画面.
//	,&gp_7_0008						// 7_0008.
//	,&gp_7_0009						// 7-0009 用紙種類.						// comment 2018/06/19 SCD2-4G-Sky水野 [IT6][制御構成変更No252_第2弾] 用紙種類画面.
//	,&gp_7_0010						// 7-0010 用紙サイズ（定形サイズ)		// comment 2018/06/19 Sky武井 [IT6][制御構成変更No.252] 用紙設定の追加.
//	,&gp_7_0011						// 7_0011 用紙サイズ（不定形サイズ）.	// comment 2018/06/15 Sky弥村 [IT6][制御構成変更No.252] 新UIデザイン_用紙設定画面(不定形).
// comment 2018/06/07 SCD2-Sky中野 [Eagle] Legacy追加対応.
	,&gp_7_0012						// 7-0012 インフォメーション.
// Comment 2020/06/12 [HCLT-Ranjith] [IT62.0_MR3] [制御構成変更No.34] one pane display gamen 
	,&gp_Q070000000
	,&gp_Q0M0000000
// comment 2018/08/10 SCD2 T.Hori [Eagle/SparrowMFP][UD評価課題対応 No.17] 認証画面プルダウン表示時、グレーアウト背景が出る.
// comment 2019/2/12 Sky黒田（達） [Eagle][操作性指摘_AI-8] ドロップダウン表示変更.
////プルダウンポップアップ画面インスタンスを1ペイン画面リストへ移動.
//	,gpp_UserAccountUser_act_SelectAuthorityMode_PullDown				// A070701200	ユーザー認証 --- 操作権限プルダウンリスト.			// comment 2018/05/22 SCD2 T.Hori [Eagle/SparrowMFP][制御構成変更_No.111_新UI対応_認証画面対応].
//	,gpp_UserAccountUser_act_AuthenticationWay_PullDown					// A070701200	ユーザー認証 --- 認証方法プルダウンリスト.			// comment 2018/05/22 SCD2 T.Hori [Eagle/SparrowMFP][制御構成変更_No.111_新UI対応_認証画面対応].
//	,gpp_UserAccountUser_act_AuthenticationAndPrint_PullDown			// A070701200	ユーザー認証 --- 認証＆プリントプルダウンリスト.	// comment 2018/05/22 SCD2 T.Hori [Eagle/SparrowMFP][制御構成変更_No.111_新UI対応_認証画面対応].
// comment 2019/04/19 SCD2-Sky小畑 [EagleH][OP_BTS-5516] 低電圧復帰でHome画面へ遷移しない対応(コメントアウトしていた処理を有効化).
//// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//////comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] 起動時に画面がちらつく コピーキー押下で画面がちらつく.
	,&gp_DummyHome
// comment 2019/08/08 Sky東島 [EagleHigh][OP_BTS-11122].
//	,&gp_MenuPopupDummyBase
	,&gp_SetupGamen
// comment 2022/04/28 Sky中野 [IT6_2.3] 制御No.43_システムオートリセット改善(WebUIv2/v3統合).
//	,&gp_WebUIWait_popup												// WebUIへの遷移準備完了通知までに表示する透過画面 comment 2018/10/22 SCD2-Sky竹菴 [Eagle][1061802] WebUIへの遷移準備完了通知までに透過のお待ちください画面を表示させる.
//comment 2019/08/29 HCLT-Satheesh [OP_BTS-11917] 3 pane is not applicable for Market place screen saver App.
	,&gp_AppChangeScreenSaver								// Screen saver class added.
	,&gp_UserSetupBrowserWait							// ユーザーセットアップお待ちくださいベース.		// comment 2022/01/28 Sky東島 [IT6_2.2MR2][制御34]ユーザーセットアップ.
	,&gp_UserSetupBrowserWait_popup						// ユーザーセットアップお待ちください画面.			// comment 2022/01/28 Sky東島 [IT6_2.2MR2][制御34]ユーザーセットアップ.
	,&gp_UserSetupBrowser								// ユーザーセットアップ.							// comment 2022/01/28 Sky東島 [IT6_2.2MR2][制御34]ユーザーセットアップ.
// 2022/03/30 生産RINNE対応 Sky start.
	,&gp_AppChangeBrowserLineModeWait
	,&gp_AppChangeBrowserLineMode
// 2022/03/30 生産RINNE対応 Sky end.
	,NULL						// 終端.
};
// comment 2018/04/04 SCD2 K.Nishiki [Eagle][制御構成変更No.111]ホーム画面認証画面の新UI化対応.
// comment 2018/11/14 Sky雜賀 【Sparrow】.
// クラシックスタイル、ベーシックスタイルの操作画面に応じて、1ペイン、または、3ペイン表示を行う.
//static const PBasicMessageProc*	tbl_1or3PaneGamen[] = {
const PBasicMessageProc*	tbl_1or3PaneGamen[] = {
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//	 &gp_HomeTopMenu				// A072002000_トップメニュー.
	 &gp_UserAccountTrack_act		// A070702000	部門管理認証.
	,&gp_PincodeInput_PKI_act		// A070706010	PINコード入力.
	,&gp_UserAccountUser_PKI_act	// A070706000	PKIカード認証_ユーザー認証.
	,&gp_VLANSelect_act				// O071100100 ネットワーク選択.
	,&gp_ScanOapAppSelectExt_scanfax	// A039000191_EXT アプリケーション選択.
	,&gp_UserAccountUser_act											// A070701200	ユーザー認証.										// comment 2018/05/22 SCD2 T.Hori [Eagle/SparrowMFP][制御構成変更_No.111_新UI対応_認証画面対応].
// comment 2018/08/10 SCD2 T.Hori [Eagle/SparrowMFP][UD評価課題対応 No.17] 認証画面プルダウン表示時、グレーアウト背景が出る.
	//プルダウンポップアップ画面インスタンスを1ペイン画面リストへ移動.
// comment 2019/02/08 SCD2-Sky谷尾 [Eagle]ホームトップメニューへ登録アプリケーションの割り当てを可能とする対応.
	,&gp_ScanOapAppSelectRegistAppGroupExt_scanfax1		// B039000990_EXT_アプリケーション選択（登録アプリケーショングループ1）.
	,&gp_ScanOapAppSelectRegistAppGroupExt_scanfax2		// B039000990_EXT_アプリケーション選択（登録アプリケーショングループ2）.
	,&gp_ScanOapAppSelectRegistAppGroupExt_scanfax3		// B039000990_EXT_アプリケーション選択（登録アプリケーショングループ3）.
	,&gp_ScanOapAppSelectRegistAppGroupExt_scanfax4		// B039000990_EXT_アプリケーション選択（登録アプリケーショングループ4）.
	,&gp_ScanOapAppSelectRegistAppGroupExt_scanfax5		// B039000990_EXT_アプリケーション選択（登録アプリケーショングループ5）.
	,&gp_ScanOapAppSelectRegistAppGroupExt_scanfax6		// B039000990_EXT_アプリケーション選択（登録アプリケーショングループ6）.
//comment 2019/05/10 SCD2-Sky東野 [EagleH] [OP_BTS-4122] WebUIライク化対応.
	,&gp_IGCWait_popup						// A072000100_Wait表示中.
	,&gp_UserAccountUserNow_act				// A070702600_ユーザー認証中.
	,&gp_UserCertificate_warn				// A0A3200700 認証エラー.
	,&gp_BoxPrintHDDStatusWait_act			// A072000100 A072000100 Wait表示中(認証＆プリント一括印刷時 スリープ復帰時 HDD準備待ち).
	,&gp_BoxPrintWait_act					// A072000100 Wait表示中(認証＆プリント一括印刷時).
	,&gp_USBWritingWait_act					// A072000100 固有設定書き換え：Wait表示中.
	,&gp_UserAccountUserRegistAndAuth_act	// A072000100_Wait表示中(未登録ユーザー登録・認証).
	,&gp_BrowserLoginNoticeWait_act			// A072000100_Wait表示中(ブラウザへのログイン通知後のReboot待ち).
// comment 2020/07/31 Sky anazawa WebUIライク化画面フッター対応.
	,&gp_UserAccountUserNowForPublic_act
	,&gp_UserAccountUserNowForPublic_PKI_NowForPublic_act
// comment 2020/06/05 K.Kasemura PKI認証のBasicモード利用可能対応.
	,&gp_WarningPkicardErr_PKI_warn	//A0A3204000_PKIカード認証エラー.
	,&gp_WaitingPKICardSet_act	// A070706120_PKIカードセット待機.
	,&gp_UserAccountUser_ContactlessPKI_act		// A070706000_PKI // comment 2020/06/08 Sky東島 [IT6_2.0MR3][制御構成変更_No.31] AU DOD認証画面 WebUI化対応.
// comment 2020/06/12 Sky T.Nishiba [制御構成変更_No.31]PKI認証時ベーシックモード使用可能対応.
	,&gp_PincodeCheckNow_PKI_act				// A070706040 PINコード認証中.
	,&gp_PincodeCheckNow_PKI_NowForPublic_act	// A070706040 PINコード認証中 Public認証からのログイン.
	,&gp_WebUIWait_popup						// WebUIへの遷移準備完了通知までに表示する透過画面.	// comment 2022/04/28 Sky中野 [IT6_2.3] 制御No.43_システムオートリセット改善(WebUIv2/v3統合).
	,NULL												// 終端.
};
// comment 2018/10/12 Sky東島 [1062683] 2ペイン化対応.
// 2ペイン対象画面(※ホーム画面の上に表示される場合は1ペイン表示となる).
const PNLS_1or2PaneGamenTbl	tbl_1or2PaneGamen[] = {
	 { &gp_UserAccountUserEndExpand1_act					,True		}								// A070702500_認証終了確認.
	,{ &gp_UserAccountUserEndNotice_act						,True		}								// A070702900_認証終了通知.
	,{ &gp_UserAccountUserInfoConfirmation_act				,True		}								// D070704100_認証情報確認.
// comment 2020/07/31 Sky anazawa WebUIライク化画面フッター対応.
//	,{ &gp_UserAccountUserNow_act							,False		}								// A070702600_ユーザー認証中.
//	,{ &gp_UserCertificate_warn								,True		}								// A0A3200700_認証エラー.
	,{ &gp_UserCertificate_warn_SeqCustom					,True		}								// A0A3200700_認証エラー.
	,{ &gp_A072002900										,True		}								// A072002900_言語一時変更.
	,{ &gp_StopSoon_act										,False		}								// A070200300_もうすぐ停止中.
	,{ &gp_CombinationError_warn							,True		}								// A0A3800000_組合せ禁止（先優先）.
	,{ &gp_CombinationErrorConfirm_warn						,True		}								// A0A3900000_組合せ禁止（設定変更確認）.
	,{ &gp_ManualPaperSizeSelect_warn						,True		}								// G0A9003400_手差しトレイ用紙検出.
	,{ &gp_ManualPaperSizeSelect_nowarn						,True		}								// G0A9003400_手差しトレイ用紙検出.
	,{ &gp_WebManualPaperSizeSelect_warn					,True		}								// G0A9003400_手差しトレイ用紙検出.
	,{ &gp_WebManualPaperSizeSelect_nowarn					,True		}								// G0A9003400_手差しトレイ用紙検出.
#ifdef DEF_A4_Cus
	,{ &gp_PaperSizeTypeSetting_act_warn					,True		}								// M875014100_用紙サイズ／種類設定.
	,{ &gp_PaperSizeTypeSetting_act_nowarn					,True		}								// M875014100_用紙サイズ／種類設定.
#else		// DEF_A4_Cus.
	,{ &gp_Tray1PaperSizeSelect_warn						,True		}								// P0A9003400_トレイ1用紙検出.
	,{ &gp_Tray1PaperSizeSelect_nowarn						,True		}								// P0A9003400_トレイ1用紙検出.
	,{ &gp_WebTray1PaperSizeSelect_warn						,True		}								// P0A9003400_トレイ1用紙検出.
	,{ &gp_WebTray1PaperSizeSelect_nowarn					,True		}								// P0A9003400_トレイ1用紙検出.
#endif  // DEF_A4_Cus
	,{ NULL													,False		}								// 終端.
};
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
//#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.

// comment 2019/05/30 Sky黒田（達） [EagleH][OP_BTS-4122] WebUIライク化対応 認証終了系画面.
// 1pane画面の上に表示される場合に、1Pane化する対象画面.
const PBasicMessageProc*	tbl_WebUILikePaneGamen[] = {
	 &gp_UserAccountUserEndExpand1_act					// A070702500_認証終了確認.
	,&gp_UserAccountUserEndNotice_act					// A070702900_認証終了通知.
	,NULL												// 終端.
};

// comment 2020/03/06 SCD2-Sky薗﨑 [IT6_2.0MR2][制御No.39]_2ペイン表示対応.
// IWS/OAPアプリ表示切換設定に応じて、2ペイン、または、3ペイン表示を行う.
// スクリーンセーバー画面(gp_AppChangeScreenSaver)については、1ペインテーブル(tbl_1PaneGamen)に定義されているため、対象外.
// Webブラウザ画面は、IWSアプリ・OpenAPIアプリのWebコンテンツ表示系画面に該当しないので、対象外.
const PBasicMessageProc*	tbl_2or3PaneGamen[] = {
	 &gp_AppChangeBrowser			// Webブラウザ画面、BrowserLauncher画面.
	,&gp_AuthOapBrowserLogin		// AuthBrowser画面.
	,&gp_ScanOapBrowserDummy		// ScanBrowser画面.
	,&gp_GeneralOapBrowserDummy		// GeneralBrowser画面.
	,&gp_PullPrintOapBrowserDummy	// PullPrintBrowser画面.
	,&gp_IwsAuthBrowserLogin		// IWSアプリ画面.
// comment 2020/06/30 Sky中野 [IT6_2.0MR3][OP_BTS-24234].
	,&gp_AuthOapBrowserWait			// AuthBrowser認証切り替え中画面.
	,&gp_IwsAuthBrowserWait			// IWS認証切り替え中画面.
// comment 2021/09/11 FPT.KhaiPQ [MinervaSSBK][ALL MACHINE][OP_BTS-35338] サイドメニュー表示を設定する場合、CostCenter画面に右側のサイドメニューを表示しない問題対策.
	,&gp_CostCenterResetDummy		// CostCenter画面.
	,NULL							// 終端.
};

/*================================================================================================
	モジュール内変数の宣言
================================================================================================*/
//ideyama:仕組み追加.
PBasicMessageProc* PBasicMessageProc::beforeGamen = NULL;	// １つ前の画面
//ideyama:仕組み追加.

short	PBasicMessageProc::secondTimer = 0;												// １秒間隔タイマー
Bool	PBasicMessageProc::bl_NoChangeActionGamen = False;								// 動作中画面への遷移プロテクトフラグ（True:全画面警告しか表示しなくなる）
PBasicMessageProc* PBasicMessageProc::nowGamen = &gp_SetupGamen;						// 現在選択されている画面（初期 = セットアップ画面）
PBasicMessageProc* PBasicMessageProc::nextGamen = NULL;									// 次の画面
PBasicMessageProc* PBasicMessageProc::nextGamenBackup = NULL;							// 次の画面の退避
PBasicMessageProc* PBasicMessageProc::turnGamen = NULL;									// 前の画面（戻りたい画面をend画面で登録しておく）
PBasicMessageProc* PBasicMessageProc::turnGamenBackup = NULL;							// 前の画面の退避
PBasicMessageProc* PBasicMessageProc::turnGamenPaperSize = NULL;						// 手差し用紙サイズ設定画面からの戻り先画面.
//comment 2019/9/10 [HCL_Shalini] [EagleH][OP_BTS-11813] Assigning the prevGamen.
PBasicMessageProc* PBasicMessageProc::prevGamen = NULL;						// 手差し用紙サイズ設定画面からの戻り先画面.
// Comment -> 2017/05/12 SCD2 3G Sky瀬戸 [AccuRev.1014826]media-ready対応.
#ifdef DEF_A4_Cus
PBasicMessageProc* PBasicMessageProc::turnGamenPaperSizeTypeSetting = NULL;				// 用紙サイズ/種類設定画面からの戻り先画面.
#endif // DEF_A4_Cus.
// Comment <- 2017/05/12 SCD2 3G Sky瀬戸 [AccuRev.1014826]media-ready対応.
// comment 2014/11/28 OPS1-Sky_nozaki [Poseidon][CQ507074] 手差し用紙検出画面表示後、設定が元に戻る問題.
Bool PBasicMessageProc::m_bl_isReturnPaperSizeGamen = False;							// 手差し用紙サイズ設定画面から戻った画面かどうか.
PBasicMessageProc* PBasicMessageProc::nextGamenCmp = NULL;							// 次の画面(コンポーネント用).
Bool PBasicMessageProc::b_targetJobChange = False;
// comment 2015/04/30 OPS1-Sky佐藤 [HeliosL][CQ513137] ズームキーの登録キー割り当て対応.
PNLE_ZOOM_SATUS PBasicMessageProc::e_ZoomStatus = PNLE_ZOOM_SATUS_MIN;					// 現在のズーム倍率設定.
static Bool bl_PrintLEDGreen = False;													// プリントキー・緑
static EP_LED_BlinkMode ep_LED_BlinkMode = EP_LED_BlinkMode_Off;						// プリントキー・点滅モード
//comment 2017/02/02 【HCLT_Baskar】　【IT5_3.4】【1012252】【Warning 除去】 [-fpermissive].
//// comment 2016/11/18 SCD2-Sky浜野 [IT5_3.3].
////static Bool _bl_comPanelJobChange = False;												// comPanelJobの変わり目を示す
//static Bool PBasicMessageProc::_bl_comPanelJobChange = False;							// comPanelJobの変わり目を示す
Bool PBasicMessageProc::_bl_comPanelJobChange = False;							// comPanelJobの変わり目を示す
IMDE_CharacterCode PBasicMessageProc::e_comCharacterCode = IMDE_CharacterCode_MacRoman;	// アプリのキャラコード
APIC_Job* PBasicMessageProc::comPrePanelJob = comPanelJob;								// パネル定期起床毎のcomPanelJobのバックアップ
FAXE_CmdRs PBasicMessageProc::e_isFaxLine1CmdReq = FAXE_CmdRsDummy;						// ファクス回線1へのコマンド要求状態フラグ（FAXE_CmdRsDummy:要求中でない）
FAXE_CmdRs PBasicMessageProc::e_isFaxLine2CmdReq = FAXE_CmdRsDummy;						// ファクス回線2へのコマンド要求状態フラグ（FAXE_CmdRsDummy:要求中でない）
FAXE_CmdRs PBasicMessageProc::e_isFaxLine3CmdReq = FAXE_CmdRsDummy;						// ファクス回線3へのコマンド要求状態フラグ（FAXE_CmdRsDummy:要求中でない）.
FAXE_CmdRs PBasicMessageProc::e_isFaxLine4CmdReq = FAXE_CmdRsDummy;						// ファクス回線4へのコマンド要求状態フラグ（FAXE_CmdRsDummy:要求中でない）.
Bool PBasicMessageProc::b_isFaxLine1OffHook = False;									// ファクス回線1がオフフック状態かどうか
Bool PBasicMessageProc::b_JobListIndexOneFlag = False;									// 画面内のボタンが押されたかどうか(左エリア選択条件).
Bool PBasicMessageProc::b_DispDataCatch1 = False;										// 回線1のネーム・ナンバーディスプレイ表示内容取得状態(False：未取得)
Bool PBasicMessageProc::b_DispDataCatch2 = False;										// 回線2のネーム・ナンバーディスプレイ表示内容取得状態(False：未取得)
Bool PBasicMessageProc::bl_DispDataCatch3 = False;										// 回線3のネーム・ナンバーディスプレイ表示内容取得状態(False：未取得).
Bool PBasicMessageProc::bl_DispDataCatch4 = False;										// 回線4のネーム・ナンバーディスプレイ表示内容取得状態(False：未取得).
Bool PBasicMessageProc::b_isOffHookBtnOn = False;										// オフフックボタン押下状態フラグ
PBasicMessageProc* PBasicMessageProc::pt_GamenBackupVoice = NULL;						// 画面ポインター、音声再生完了後にsetVoiceRequestFlagで設定した画面の値が入る.
uchar PBasicMessageProc::uc_procOkVoiceOptionBackup = 0;								// OKキー押下時のオプション値保存.
ushort	PBasicMessageProc::us_PreGamenStartVoiceID[ DEF_VoiceIdBufSize ];				// 音声2度読み判定用バッファ.
Bool	PBasicMessageProc::bl_SameGamenInstanceForVoiceStartGamen = False;				// nowGamen と changeGamen が同じインスタンスかを保持する(画面切り替え時の音声ガイド用)[True：同じ].
Bool	PBasicMessageProc::m_bPopupAnimeAct = False;									// Popupアニメーション判定フラグ.
// LDAP検索管理オブジェクト（ファクス用）
extern PCGC_LdapSearchManager c_FaxLdapSearchMgr;
PNLE_FaxChainDialFlg PBasicMessageProc::e_faxChainDialflg = PNLE_FaxChainDialFlgNon;	// Faxチェーンダイアル状態フラグ.
// 外部公開関数.
DBGE_DebugLogSystemResult PBasicMessageProc::m_e_DebugLogSystemResult = DBGE_DebugLogSystemResultOK;
Bool PBasicMessageProc::bl_isExtAppOperationGamen = False;			// [CQ.343200]I/Fタスクの指定がされていないため、起動時に処理が遅くなる対応.
PBasicMessageProc* PBasicMessageProc::m_pc_SeqCusGamen = NULL;							// 認証シーケンス画面インスタンス.
//comment 2023/04/18 FPT.HungDM17 【IT6_2.3】【ALL MACHINE】【OP_BTS-42606】Fix coordinate of Maintenance Mode Status Bar depend on pane resolution.
#ifndef DEF_LCD_7INCH
Bool PBasicMessageProc::bl_JudgeStatusMaintenanceBar = False;
#endif
Bool isPrintLEDGreen()
{
	return bl_PrintLEDGreen;
}

struct PNLS_WUT_LOG {
	uint32_t			ul_tick;		// Tick.
	PNLE_WUT_LOG_KIND	e_LogKind;		// WUTログ種別.
};

#define PNLD_WUT_LOG_MAX	(5)			// ログ蓄積件数.
Bool b_wutCompleteFlg = False;			// WUT計測完了フラグ.
static PNLS_WUT_LOG	s_wutLogData[PNLD_WUT_LOG_MAX];
static uchar	uc_wutLogCnt = 0;
// 警告イベントデータ 構造体LAPS_NTF_SRV_Warningのul_WarningOn(uchar型).
typedef uchar PNLC_NTF_Warning;
const PNLC_NTF_Warning PNLC_NTF_Warning_Off = PNLC_NTF_Warning( 0 );	// 警告解除.
const PNLC_NTF_Warning PNLC_NTF_Warning_On = PNLC_NTF_Warning( 1 );	// 警告発生.
PDPC_RealTimePreviewObserver	c_RtPreviewObserver;		// PrvProcessからのプレビュー作成完了通知用オブザーバー.

short ss_NumberOfJobList = 0;
static LIBC_List<APIC_Job> c_JobList;

// comment 2018/10/08 Sky水野 [Eagle][AR1062683] フッター対応.
// comment 2018/11/07 【IT6_Sparrow】【HCLT_Taqiuddin】【AR1058205】 Warning Removal Fix.																		
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
static EP_LED_StartColor e_LEDColorState = EP_LED_StartColor_off;	// スタートＬＥＤの色.
#endif
/*================================================================================================
［機能］	WUT計測用ログ.
================================================================================================*/
void setWUTLog( PNLE_WUT_LOG_KIND eWutLog )
{
	if( PNLD_WUT_LOG_MAX <= uc_wutLogCnt ) {
		return;
	}
	uchar	uc_pos = uc_wutLogCnt;
	uc_wutLogCnt++;

	s_wutLogData[uc_pos].ul_tick	= SYSF_tickGet();
	s_wutLogData[uc_pos].e_LogKind	= eWutLog;
}

/*================================================================================================
［機能］	WUT計測用ログ出力
================================================================================================*/
void showWUTLog()
{
	uchar	uc_cnt = 0;
	static const char* const msgstr[] = {
		"",
		"Ready To Copy",
		"IR Ready",
		"Engine Ready",
		"HDD Ready",
	};

	for( uc_cnt=0; uc_cnt < uc_wutLogCnt; uc_cnt++ ) {
		printf( "[%5d] %s\n", s_wutLogData[uc_cnt].ul_tick,
		        msgstr[( uchar )s_wutLogData[uc_cnt].e_LogKind ] );
	}
}


Bool PBasicMessageProc::b_keyboard = False;
PDisplayParts* const checkParts[] = {
	&Grp_Keybord ,
};
#define PCGD_checkPartsNumber 1			// checkPartsテーブルのサイズ.

void PBasicMessageProc::checkPanelState()
{
	// フラグの初期化.
	PBasicMessageProc::b_keyboard = False;

	for( register short rs_i = 0 ; rs_i < partsNumber ; rs_i++ ) {		// 画面の保持しているすべてのパーツに対して調査する.
		for( register short rs_j = 0 ; rs_j < PCGD_checkPartsNumber ; rs_j++ ) {		// 対象パーツテーブルについて調査する.
			if( PBasicMessageProc::b_keyboard == True ) {			// すでにフラグがたっていれば処理を省略する.
				continue;
			}
			if( partsTable[ rs_i ]->checkParts( checkParts[ rs_j ] ) == True ) {
				// 対象パーツを保持していたので、フラグを立てる.
				PBasicMessageProc::b_keyboard = True;
			}
		}
	}
}

/*================================================================================================
［機能］	基本メッセージ処理 : 画面パーツの表示
================================================================================================*/
void PBasicMessageProc::show()
{

	// 表示可能状態のパーツを表示する
	for( register int i = 0; i < partsNumber; i++ )
	{
		partsTable[i]->show();
	}

	// 画面ウィジェット表示
	if( m_pc_displayParts != 0 )
	{
		m_pc_displayParts->show();
	}

	PNLC_VoiceCursorControl::newInstance()->showCursor();// カーソル表示
}
// シミュレータ用：仕向け、言語切り替え時の、画面再描画関数
// 画面再描画API.
void PBasicMessageProc::refreshNowGamen()
{
	show();
}
/*================================================================================================
［機能］	基本メッセージ処理 : 画面パーツの消去
================================================================================================*/
void PBasicMessageProc::hide()
{
	PNLC_PopupListControl* plCnt = PNLC_PopupListControl::newInstance();
	PBasicMessageProc* baseGamen = plCnt->getBaseGamen();
	PLayerControl* srcPopup = plCnt->getSrcPopup();


	if( ( srcPopup != 0 ) && ( baseGamen != srcPopup ) )
	{	// 呼び出し元ポップアップがある場合
		// 呼び出し元ポップアップの真後ろにグレーアウト画面を持ってくる
		PGrayoutWidget* grayout = PNLC_PopupListControl::newInstance()->getBaseGamen()->getGrayoutParts();
// comment 2015/12/16 MSE岡田 [ZeusS_ZX0][CQ522807]背面にグレーを持たない画面の場合、背面にグレー画面を配置しないようにする.
//		if( grayout != 0 )
		if ( ( grayout != 0 ) && ( True == srcPopup->getBackLayerDarkness() ) )
		{
			grayout->setBehind( dynamic_cast<PPopupWidget*>( srcPopup->getDisplayParts() ) );
			grayout->show();
		}
	}
	else
	{	// 呼び出し元ポップアップがない(=ベース画面から呼び出された)場合
		// グレーアウト画面消去
		PGrayoutWidget* grayout = PNLC_PopupListControl::newInstance()->getBaseGamen()->getGrayoutParts();
		if( grayout != 0 )
		{
			grayout->hide();
		}
	}

	// 画面ウィジェット消去
	if( m_pc_displayParts != 0 )
	{
		m_pc_displayParts->hide();
	}

	for( register int i = 0; i < partsNumber; i++ ) {
		partsTable[i]->hide();
	}
}
/*================================================================================================
［機能］	基本メッセージ処理 : 画面パーツのリセット
================================================================================================*/
void PBasicMessageProc::resetParts()
{
	for( register short i = 0; partsNumber > i; ++i ) {
		partsTable[ i ]->resetParts();
	}
}
/*================================================================================================
［機能］	基本メッセージ処理 : パラメータのリセット.
================================================================================================*/
void PBasicMessageProc::resetParam( uchar option )
{
	procReset( option );
}
/*================================================================================================
［機能］	基本メッセージ処理 : 画面パーツを全て表示状態にする.
================================================================================================*/
void PBasicMessageProc::showEnableAllPars()
{
	for( register int i = 0; i < partsNumber; i++ ) {
		partsTable[i]->showEnable( True );
	}
}
// comment 2015/07/13 OPS1-Sky後藤 [CQ514499] LDAP検索パフォーマンス改善.
/*================================================================================================
［機能］	基本メッセージ処理 : 画面パーツを全て非表示状態にする.
================================================================================================*/
void PBasicMessageProc::showEnableAllPartsHide()
{
	for( register int i = 0; i < partsNumber; i++ ) {
		PGroupParts* pc_Group = dynamic_cast<PGroupParts*>(partsTable[i]);
		if( NULL != pc_Group ) {
			pc_Group->showEnableAll( False );
		}
		if( NULL != partsTable[i] ) {
			partsTable[i]->showEnable( False );
		}
	}

}

/*================================================================================================
［機能］	メッセージ・ディスパッチ処理
［履歴］  @data	2012/11/27		Tadano		画面ズーム対応.
				2013/02/22		Sky籔野		[操作性改善FUM4][CT不具合対応]リモートパネルでの画面ズームOn/Off設定条件変更.
				2013/03/08		Sky籔野 	[操作性改善FUM4][CQ378290] リセットキー押下時の画面ズーム機能の初期化要求条件を変更.
				2015/11/24		Sky			システム統合SoC(S800)対応.
================================================================================================*/
void PBasicMessageProc::messageDispath( Message message, uchar option, MessageType type, int32_t i32_option, void* st_NtfEventData )
{
	static const Message keyMessage[] = {
// comment 2014/05/16 OPS1-Sky田村 [特Rom対応][MZV-1]立体コピー対応.
		MS_CapsulePaperSetting,
		//ＬＣＤ内キーメッセージ
		MS_IndexButton, MS_GoToPage, MS_Ok, MS_Cancel, MS_End, MS_GoToPaper, MS_GoToMag, MS_GoToFace, MS_GoToColor, MS_Button, MS_KeyBoard, MS_GoToScanColor, MS_GoToScanResolution, MS_GoToScanColorLevel, MS_GoToScanScanSize,
		// ワンタッチキーメッセージの追加
		MS_OneTouchKey,
		// ボックスのメッセージ追加
		MS_BoxUp, MS_BoxDown, MS_BoxIndex, MS_BoxButton, MS_DocButton,
		MS_ShortCutKey,
		//ジョブリスト用
		MS_JobUp, MS_JobDown, MS_JobDelete, MS_JobIndex, MS_JobControl, MS_JobButton,
		// 左エリア用.
		MS_LeftIndex, MS_BookMark, MS_LeftSettingGoTo, MS_LeftSettingButton, MS_LeftSettingList, MS_LeftPreviewGoTo, MS_LeftPreviewButton, MS_LeftPreviewList, MS_PreviewDetail,
		MS_ScreenJump,	// [画面ジャンプ]
		// ダイレクトキー操作によるキーメッセージ
		MS_Reset, MS_Start, MS_Stop, MS_10Key, MS_Clear, MS_ColorMode, MS_IntMode, MS_IDInput, MS_UserMode, MS_ModeCheck, MS_Service, MS_Copy, MS_Scan,
		// 追加されたダイレクトキー操作によるキーメッセージ
		MS_Enlarge, MS_SampleCopy, MS_JobMemory, MS_Utility, MS_Fax, MS_Box, MS_PowerSave, MS_Help, MS_Accessibility,
		//小ＬＣＤ用
		MS_EnergySaveReset, MS_Program,
		// サービス基板からの入力
		MS_TroubleReset, MS_Initial, MS_MemoryClear, MS_TotalClear,
		// その他
		MS_PreHeatOnByKeyAns, MS_Return, MS_Back,
		// アプリ切替キー.
		MS_AppChg,
		MS_ImageDealer, MS_MyPanel, MS_Browser, MS_AppMenu,
		// イメージディーラアニメーションボタン.
		MS_IMDButton,
		MS_USBKey,
		MS_SPCButton,
		MS_VoiceGuide, MS_FlashCommand, MS_VoicePlayEnd, MS_VoiceStopEnd
		, MS_TabIndex
		,MS_DoubleTap,
		MS_Menu, MS_APPsMenu,
		MS_Soft10Key,
		MS_IMETxtNotice,	// 2012/12/04 OPS1川手 操作性改善FUMVer4(WBS No.62-64).
		MS_ReqIMEEnd,		// IME終了要求 // 2012/12/25 Sky井柳 操作性改善FUMVer4 新規作成(WBS No.259,260).
		MS_TabletMainPanelTouchNotice,
		MS_BadgeButton,
		MS_UbqPrint,		// ユビキタスプリント.
		MS_IMETabletConnect,
		MS_IMETabletDisconnect,
		MS_StopIMENotice,
		MS_SlideMenu,
// commnet 2015/03/23 OPS1-Sky守川 [Helios][スライドメニュードラッグ対応].
		MS_SlideMenu_Press,
		MS_SpecifyApp,
		MS_TabletConnectConfirm,	// 2014/09/10	s.ohba	[ZeusS][UV-01448] 端末接続許可確認通知追加.
		MS_FaxOnly,
// comment 2014/11/14 OPS1-Sky浜野 [Poseidon][CQ503655]
// MS_QtButton追加により再生停止処理が実施されてしまうため、削除（CQ503760にて改修済みであるため、本対応は不要となる）.
//// comment 2014/10/16 【HCLT-Premkumar】 【CQ-502549】Adding the Message type MS_QtButton.
//		MS_QtButton,
// comment 2015/04/30 OPS1-Sky佐藤 [HeliosL][CQ513137] ズームキーの登録キー割り当て対応.
		MS_ZoomIn,
// [特ROM対応][IT6-43] Sky西村 認証アプリケーションのログイン画面から直接パブリックユーザーでログイン.
		MS_PublicLogin,
// 2022/08/18 [IT6-103(機種マージ)] Sky奥田 全国銀行協会による手形・小切手電子化に伴うスキャン機能カスタマイズ要望 Start.
		MS_CheckValueSet,
		MS_BillsValueSet,
// 2022/08/18 [IT6-103(機種マージ)] Sky奥田 全国銀行協会による手形・小切手電子化に伴うスキャン機能カスタマイズ要望 End.
	};
// Comment 2018/01/03 [HCLT_Abinav][RQ-686][IT5_4.2][AR1033014] changing hardkey press functionality for FAX and WebBrowser in NW2.
	TYPE_NetworkSpecific e_NetworkSpecific = PNLC_NetworkLan::getInstance()->getLine();
	static Bool b_IR_OKFlg		= False;
	static Bool b_Engine_OKFlg	= False;
	static Bool b_HDD_OKFlg		= False;
	if( False == b_wutCompleteFlg ) {
		if( False == b_IR_OKFlg ) {
			if( False == ppc_ScannerStatus->isIRNotReady() ) { // IR準備完了.
				b_IR_OKFlg = True;
				setWUTLog( PNLE_WUT_LOG_IR_RDY );
			}
		}
		if( False == b_Engine_OKFlg ) {
			if( False == onlyPanel.isPrinterWait() ) { // Engine準備完了.
				b_Engine_OKFlg = True;
				setWUTLog( PNLE_WUT_LOG_ENG_RDY );
			}
		}
		if( False == b_HDD_OKFlg ) {
			if( TYPE_HddStatusConnectEndDocWait == cp_HddInfo->getHddStatus() ) { // HDD準備完了
				b_HDD_OKFlg = True;
				setWUTLog( PNLE_WUT_LOG_HDD_RDY );
			}
		}
	}

	PBasicMessageProc* pOldPopup = PNLC_PopupListControl::newInstance()->getNowPopup();
	char sc_OldPoupCount = PNLC_PopupListControl::newInstance()->getPopupNum();

	Bool bl_ProcMessage = True;	// メッセージ処理を行う
	m_bPopupAnimeAct = False;		// ポップアップアニメーションフラグの初期化
	TYPE_PowerSaveStatus	e_PowerSaveStatus;
	register ushort i;	// Qt4対応:VC固有スコープ処理を回避
	for( i = 0; i < sizeof( keyMessage )/sizeof( Message ); i++ ) {
		if( message == keyMessage[i] ) {
			// 何かキー（ダイレクトキーまたはタッチパネル）がさわられた(離された、押し続けは除く）
			register Bool bl_Msgtrans = False;
			switch( type ) {
				case MT_Up:					// 離された
					// キー押下が機能キー１かどうか.
					// MT_Upの場合、機能キーは「message==アプリ情報」ではなく、「message==機能キー」となる.
				{
					Message uc_message = PLBC_LogOutputStatus::getInstance()->getRegistkeyApp( 0 );
					// キー押下が機能キー１かどうか(Webブラウザの場合を除く).
					if( message == uc_message )
					{
						// ログ出力状態遷移(初期状態).
						PLBC_LogOutputStatus::getInstance()->changeLogStatus( PNLE_LogOutputStatus_No );
					}
				}
					// メッセージ「操作キーＯＮ通知」 タイマーのリセットなどをマシン管理部が行なう
				if( APIC_KeyDevice::getKeyMode() == TYPE_KeyModePageAcses		// 認証装置装着時.
				    && APIC_KeyDevice::getCardStatus() == TYPE_CardStatusSet ) {	// 認証カードセット状態.
					APIC_KeyDevice::keyOperation();
				}
				e_PowerSaveStatus = ppc_Power->getPowerSaveStatus();
				if( TYPE_PowerSaveStatusIdle != e_PowerSaveStatus ) {			// 待機中以外の状態（限定解除もスリープ中とみなす）.
					if( onlyPanel.bl_IsKeyLockingKeyOn == False ) {
						// プレヒート中にキーアップした時、キーロック中にキー押下していない状態（プレヒート後にキーアップ）は、操作キーＯＮ通知をする。
						bl_Msgtrans = True;
					}
				} else {
					// プレヒートでない時は、パネルキーロック状態でない時のみ、キーアップ処理（キー押下通知）をする
					if( onlyPanel.bl_IsPreHeatKeyOnKeyLockOn == False ) {
						bl_Msgtrans = True;
					}
				}
				onlyPanel.bl_IsKeyLockingKeyOn = False;	// キー離されたので、フラグを落とす。
				break;
				case MT_DontCare:			// 気にしない
				case MT_On:					// 押された
				{
					Message uc_message = PLBC_LogOutputStatus::getInstance()->getRegistkeyApp( 0 );
					// キー押下が機能キー１かどうか(Webブラウザの場合を除く).
					if( ( message == uc_message ) && ( MS_Browser != uc_message ) && ( False == onlyPanel.getBrowserMode() ) ) {
						// ログ出力状態遷移(タイマー開始).
						PLBC_LogOutputStatus::getInstance()->changeLogStatus( PNLE_LogOutputStatus_Timer );
					}
				}
					// APIで再生中止を要求
//					if ( (getVoiceCursorMode() != TYPE_VoiceGuide_OFF ) && ::getVoicePlayNow() )
//					if ( (getVoiceCursorMode() != TYPE_VoiceGuide_OFF ) && ::getVoicePlayNow() && !(message == MS_10Key && option >= 10) )
//						::reqVoiceStop(NULL);
//					if ( (getVoiceCursorMode() != TYPE_VoiceGuide_OFF ) && ::getVoicePlayNow() && !(message == MS_10Key && option >= 10) ) {
//comment 2023/03/30 FPT-KhaiPQ [IT6_2.3] [OP_BTS-42621]動画の音声が流れる途中にスライドメニューを押すと、音声が止まった問題対策.
//				if( ( getVoiceCursorMode() != TYPE_VoiceGuide_OFF ) && ::getVoicePlayNow() && !( message == MS_10Key && option >= 10 ) && ( onlyPanel.getSkipVoice() ) ) { {
				if( ( getVoiceCursorMode() != TYPE_VoiceGuide_OFF ) && ::getVoicePlayNow() && !( ( message == MS_10Key && option >= 10 ) || ( message == MS_SlideMenu_Press ) ) && ( onlyPanel.getSkipVoice() ) ) {
					// 【ヘルプ＞目的から探す】で動画のステップ遷移ボタンが押下された時は画面遷移しない為、再生停止要求はしない.
					if( False == ::getMovPushHelpExpButtonflag() ) {
						::reqVoiceStop( NULL );
					}
				}
					if( !( m_pstVoiceGamenInfo && /*::getVoiceCursorMode() != TYPE_VoiceGuide_OFF*/ ::getVoiceCursorMode() == TYPE_VoiceGuide_Normal&&
				       ( ( message == MS_10Key && option != 5 ) ||
				         message == MS_VoiceGuide || message == MS_FlashCommand || message == MS_VoicePlayEnd || message == MS_VoiceStopEnd ) ) ) {
					// パーツなしで外部キーボード入力値を取得する画面で
					// 発生したイベントメッセージがUSBキーであるかを判定
					if( ( MS_USBKey == message )
					    && ( True == kbd_NoPartsUSBKeybord.chkNoKeybordGamen( False ) ) )
					{
						// 上記条件を満たす場合は何もしない.
					} else {
						//「キー押下で解除」する警告のチェック/解除
						if( onlyPanel.isPanelWarning( EP_PanelWarningResetType_KeyOn, True ) == True ) {
							// メッセージの処理が、小ＬＣＤ用、サービス基板からの入力以外（ＬＣＤ内、ジョブリスト、ダイレクトキー）の時メッセージ処理をしないフラグを立てる
							if( message != MS_Program && message != MS_TroubleReset && message != MS_Initial
							    && message != MS_MemoryClear && message != MS_TotalClear ) {
								bl_ProcMessage = False;	// メッセージ処理を行わない
							}
						} else {
							if( onlyPanel.bl_IsPreHeatKeyOnKeyLockOn == True ) {
								if( message != MS_Program && message != MS_TroubleReset && message != MS_Initial
								    && message != MS_MemoryClear && message != MS_TotalClear ) {
									bl_ProcMessage = False;					// メッセージ処理を行わない
									onlyPanel.bl_IsKeyLockingKeyOn = True;	// キーロック中にキー押下された
								}
							}
						}
					}
				}
				PRCC_TrayHPMover::newInstance()->panelEvent();
				break;
				case MT_StillOn:			// 押し続けている
				case MT_StillOnSlow:		// 押し続けている（ゆっくり変化）
				case MT_StillOnMiddle:		// 押し続けている（やや速く変化）
				case MT_StillOnFast:		// 押し続けている（速く変化）
					//  キー押下をPageAcsesに通知.
					// 参照先まちがい pc_MachineSettingではなく APIC_KeyDeviceが正しい.
					if( APIC_KeyDevice::getKeyMode() == TYPE_KeyModePageAcses		// 認証装置装着時.
					    && APIC_KeyDevice::getCardStatus() == TYPE_CardStatusSet ) {	// 認証カードセット状態.
						APIC_KeyDevice::keyOperation();
					}
					break;
				default:	break;
			}
			TYPE_PowerSaveStatus	e_PowerSaveStatus = ppc_Power->getPowerSaveStatus();
			if( ( e_PowerSaveStatus == TYPE_PowerSaveStatusLowPowerIn			// ローパワーモード移行中.
			      || e_PowerSaveStatus == TYPE_PowerSaveStatusSleepIn				// スリープ移行中.
			      || e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOffProc )	// 副電源OFF処理中.
			    && ( ( message == MS_Button ) || ( message != MS_Button && type == MT_On ) ) ) {	// ボタン押下、又は、ボタン押下以外のイベントで何らかのキー押下された時.
				// 電源関係の移行中は、キー操作の処理を行わないようにする.
				bl_ProcMessage = False;		// メッセージ処理を行わない.
			} else if( TYPE_VoiceGuide_EndProcessing <= getVoiceCursorMode() ) {
				// 音声ガイド終了処理中のとき(低電力、スリープモードへ移行するとき).
				if( ( type==MT_DontCare || type==MT_On ) && message==MS_VoiceStopEnd ) {
					bl_ProcMessage = False;	// メッセージ処理を行わない.
					setVoiceCursorMode( TYPE_VoiceGuide_OFF );	// 音声再生が終了したとき.
					reqLED( PNLE_LED_VoiceGuide, ( TYPE_VoiceGuide_OFF == getVoiceCursorMode() ) ? PNLE_LEDStateOff :  PNLE_LEDStateOn );
				}
			} else {
				if( bl_Msgtrans == True ) {
					if( message != MS_PowerSave ) {	// パワーセーブキー以外が押されたとき処理を行う
						ppc_Power->changePowerMode( TYPE_PowerModeEtsKeyOperation );	// その他キー操作を通知する
					}
				}
// comment 2014/12/12 OPS1-Sky中江 [ZeusS][CQ507584]タブレット接続許可確認通知の場合はキー押下処理を行わない様修正(タイマーのリセットを抑止).
// comment 2023/03/13 Sky東島 [IT6_2.3][OP_BTS-40817].
//				if( message != MS_TabletConnectConfirm ){	// タブレット接続許可確認通知以外の場合に処理を行う.
				if( ( message != MS_TabletConnectConfirm )	// タブレット接続許可確認通知、タブレット切断時以外の場合に処理を行う.
				 && ( message != MS_IMETabletDisconnect ) ){
				if( type != MT_Up ) {
					m_SomethingKeyOnMessage = message;	// procSomethingKeyOn時のMessage.
					u_SomethingKeyOnOption = option;	// procSomethingKeyOn時のOption.
					procSomethingKeyOn( 0 );			// 何らかのキーが押された
				}
				}
				if( message == MS_GoToPage
				    || message == MS_IndexButton
				    || message == MS_GoToPaper
				    || message == MS_GoToMag
				    || message == MS_GoToFace
				    || message == MS_GoToColor
				    || message == MS_Return
				    || message == MS_Button
				    || message == MS_ShortCutKey
				    || message == MS_Cancel
				    || MS_BadgeButton == message
// △△△ [新OAP対応]Sky鈴木理 2011/04/22 .
				 || message == MS_Ok ) {
// comment 2014/11/14 OPS1-Sky浜野 [Poseidon][CQ503655]
// MS_QtButton追加により再生停止処理が実施されてしまうため、削除（CQ503760にて改修済みであるため、本対応は不要となる）.
//// comment 2014/10/16 【HCLT-Premkumar】 【CQ-502549】Adding the check for the Message type MS_QtButton.
//				 || message == MS_QtButton) {
					if( type == MT_On || type == MT_DontCare ) {
						b_JobListIndexOneFlag = True;	// 画面内のボタンが押されたかどうか(左エリア選択条件).
					}
				}
			}
			break;
		}
	}

// comment [2015/05/12] [HCLT-Rajamanickam][MinervaSBK][機器構成変更_No.13] Message IDs list to notify DF.
	static const Message keyMessageADF_RaiseReq[] = {
		//LCD touch messages.
		MS_IndexButton, MS_GoToPage, MS_Ok, MS_Cancel, MS_End, MS_GoToPaper, MS_GoToMag, MS_GoToFace, MS_Button, MS_KeyBoard, MS_GoToScanColor, MS_GoToScanResolution, MS_GoToScanColorLevel, MS_GoToScanScanSize, MS_GoToColor,MS_Return, MS_Back,	MS_Execute,MS_SPCButton,MS_Program, MS_Accessibility,
		// One touch message.
		MS_OneTouchKey,
		// Box message.
		MS_BoxUp, MS_BoxDown, MS_BoxIndex, MS_BoxButton, MS_DocButton,
		//Shortcut key message.
		MS_ShortCutKey,
		//Job list message.
		MS_JobUp, MS_JobDown, MS_JobDelete, MS_JobIndex, MS_JobControl, MS_JobButton,
		//Direct key message.
		MS_Start, MS_10Key, MS_Clear, MS_ColorMode, MS_IntMode, MS_IDInput, MS_UserMode, MS_ModeCheck, MS_Copy, MS_Scan, MS_EnlargeDsp, MS_Enlarge, MS_SampleCopy, MS_JobMemory, MS_Utility, MS_Fax, MS_Box, MS_Help,MS_ImageDealer, MS_MyPanel, MS_Browser, MS_AppMenu,MS_USBKey,MS_VoiceGuide,MS_Soft10Key,MS_APPsMenu,MS_UbqPrint,MS_FaxOnly,
		// Left area message.
		MS_LeftIndex, MS_BookMark, MS_LeftSettingGoTo, MS_LeftSettingButton, MS_LeftSettingList, MS_LeftPreviewGoTo, MS_LeftPreviewButton, MS_LeftPreviewList, MS_PreviewDetail,
		//Drag Control.
		MS_TabIndex, MS_Drag, MS_Release,MS_DoubleTap,
		//QT events.
		MS_QtButton, MS_QtDoubleTap, MS_QtThumbnailImage, MS_QtAction, MS_QtFlickEnd, MS_QtBuzzer, MS_QtMovePage, MS_QtColumnMove, MS_QtRotate, MS_QtHandleAreaTouch, MS_QtEventNotice, MS_QtBroadcastPadButton, MS_QtLongTap,
		//Function Badge button press.
		MS_BadgeButton,MS_TabletMainPanelTouchNotice,MS_RMPStrInput,
		//Slide menu messages.
		MS_SlideMenu,MS_SpecifyApp,MS_SlideMenu_Press,
		//DF raise notification message.
		MS_ReqDFRaise,
	};
	
	//Iterate the message IDs list to notify DF.
	for( i = 0; i < sizeof(keyMessageADF_RaiseReq)/sizeof(Message); i++ ) {
		if( keyMessageADF_RaiseReq[i] == message ) {	//If current message matched with DF notification messages list.
			reqDFRaise();		//Notify the DF.
			if(MS_ReqDFRaise == message){	// If message is to raise DF request.
				message = MS_Dummy;			//Assign message as MS_Dummy.
			}
			break;				//Break from for loop.
		}
	}
	static const Message keyMessageLCD[] = {
		//ＬＣＤ内キーメッセージ
		MS_IndexButton, MS_GoToPage, MS_Ok, MS_Cancel, MS_End, MS_GoToPaper, MS_GoToMag, MS_GoToFace, MS_Button, MS_KeyBoard, MS_GoToScanColor, MS_GoToScanResolution, MS_GoToScanColorLevel, MS_GoToScanScanSize,
		// ワンタッチキーメッセージの追加
		MS_OneTouchKey,
		// ボックスのメッセージ追加
		MS_BoxUp, MS_BoxDown, MS_BoxIndex, MS_BoxButton, MS_DocButton,
		// ショートカットキーのメッセージ追加
		MS_ShortCutKey
		, MS_TabIndex, MS_Drag, MS_Release
		,MS_DoubleTap,
		MS_BadgeButton
	};
	for( i = 0; i < sizeof( keyMessageLCD )/sizeof( Message ); i++ ) {
		if( message == keyMessageLCD[i] && type != MT_Up ) {
			// ＬＣＤ内キーが押された時（離された時はフラグセットしない）.
			onlyPanel.bl_DisableMessageWarningWithGamen = True;	// 画面遷移メッセージ警告抑制フラグをセットする
		}
	}

	bool b_callBkFunc = False;
	if( bl_ProcMessage == True ) {	// メッセージ処理を行う

// comment 2019/06/28 Sky東島 [EagleHigh][OP_BTS-9409]  新詳細コード拡張対応.
//		if( MS_Animation2 != message ) {						// 操作性改善用アニメーションの場合はログ出力を行わない.
		if( MS_Animation2 != message && MS_CallBackNtfEvent != message ) {						// 操作性改善用アニメーションの場合はログ出力を行わない.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, message, option, type );
			if( MS_CallBack == message ){
				PNL_LogManager::newInstance()->evnetLog( this, message, (short)i32_option, type );
			} else {
				PNL_LogManager::newInstance()->evnetLog( this, message, (short)option, type );
			}

// comment 2016/06/09 SCD2-Sky浜野 [ZeusSBK][CQ529698] 新詳細コード拡張対応.
			LUIC_DbgTsk::newInstance()->setNowGamenHandle( reinterpret_cast< uint64_t >( this ) );
			LUIC_DbgTsk::newInstance()->setMessageType( (short)(message << 8 | option) );
		}
		// 検索結果通知の場合は「お待ちください」P.U.表示のタイマーをキャンセルする
		if( MS_CallBack == message ) {
			switch( i32_option ) {
				case TYPE_PNL_CallBack_ShorteningAddressSearchResult:
				case TYPE_PNL_CallBack_ShorteningGroupAddressSearchResult:
				case TYPE_PNL_CallBack_GroupAddressSearchResult:
				case TYPE_PNL_CallBack_ProgramKeySearchResult:
				case TYPE_PNL_CallBack_TempSendSearchResult:
// comment 2019/07/19 Sky毛穴 [特ROM][FQ-OPSD-1900232-OFP146] リセットキー押下後プログラム呼出しへの遷移でプログラム内容が初期化される.
					onlyPanel.m_bl_InProgress_AddressSearchCallback = False;			// 検索結果通知後のコールバック処理中のフラグを落とす.
					Bool b_isClearDisp;
					Bool b_isPopup;
					if( DelayedIGCWaitDisplayTimer::isAlive( b_isClearDisp, b_isPopup ) == True
					    && ( ( pc_ManageIGCWaitDisplay.getnextIGCGamen() != 0 ) || ( pc_ManageIGCWaitDisplay.getnextIGCPopup() != 0 ) ) ) {
						// 遅延タイマーが起動しており、遷移先画面or遷移先ポップアップが存在する場合、procCallBack()がコールされるため以降の処理でprocCallBack()をコールしないようにする.
						b_callBkFunc = True;
					}
					DelayedIGCWaitDisplayTimer::procCallBack( option );
					DelayedIGCWaitDisplayTimer::cancel();
					break;
// comment 2016/09/12 SCD2-Sky浜野 [IT5_3.3][1002611].
				case TYPE_PNL_CallBack_SetZoomOff:		// 表示ズーム等倍通知.
					PNLC_SlideMenuControlLib::newInstance()->judgeShown();
					break;
// comment 2021/01/15 OPS1 Sky東島 [IT6_2.1][OP_BTS-30450].
				case TYPE_PNL_CallBack_AudioAnimeInstall:
				case TYPE_PNL_CallBack_AudioAnimeDelete:
					gp_ServiceDataInstall_ZM3rd.procCallBack( ( TYPE_PNL_CallBack )i32_option );
					// データインストール画面のコールバック処理を固定で行い、共通のコールバック処理は行わないようにする.
					b_callBkFunc = True;
					break;
				default:
					; // 何もしない
			}
		}
		if( MT_On == type ) {	// 押されたタイミングのイベントのみを扱う.
			PNLE_OperationLogState e_OperationLogState = PNLE_OperationLogStateNormal;
			if( m_pstVoiceGamenInfo && ::getVoiceCursorMode() != TYPE_VoiceGuide_OFF ) {	// 音声ガイドが有効の場合.
				e_OperationLogState = PNLE_OperationLogStateVoiceGuide;
			}
			// 外キー押下によるログの収集.
			PNLC_OperationLog::newInstance()->setLogHardKeyPush( message, option, e_OperationLogState );
		}

		Message e_MessagePre = message;
		messageDispathPreProcessing( message, option, type, i32_option, st_NtfEventData );
		if( bl_isCmpEvent == True ) {
			PBasicMessageProc::setNextGamen( pCmpNextGamen );
			pCmpNextGamen = NULL;
			if( b_isAttachMessage == True ) {
				// メッセージをMS_Dummyに置き換え.
				message = MS_Dummy;
			}
		}
		// 音声ガイド中のカーソル消去処理.
		if( type == MT_On || type == MT_DontCare ) {
			switch( e_MessagePre ) {
				case MS_Button:				// ボタン（押された）
				case MS_IndexButton:  		// インデクスボタン.
				case MS_GoToPage:			// 画面移動.
				case MS_Ok:					// 設定終了.
				case MS_Cancel:				// 設定取消.
				case MS_KeyBoard:			// キーボード.
				case MS_OneTouchKey:		// ワンタッチキー.
				case MS_Execute:			// 実行.
				case MS_10Key:				// テンキー.
				case MS_Clear:				// クリアー.
				case MS_BadgeButton:		// 機能バッジキー.
				case MS_SPCButton:			// SPC用ボタンON.
					if( m_pstVoiceGamenInfo && ::getVoiceCursorMode() != TYPE_VoiceGuide_OFF ) {
						::hideVoiceCursor( True );	// カーソル消去.
					}
					break;
				default:	break;
			}
		}
		if( message == MS_Button ) {
			// 通常ボタン
			switch( type ) {
				case MT_On:
				{
					if( m_pstVoiceGamenInfo && ::getVoiceCursorMode() != TYPE_VoiceGuide_OFF ) {
						procButtonOnVoice( option );	break;	// ボタン（押された）
					} else {
						procButtonOn( option );	break;	// ボタン（押された）
					}
				}
				case MT_StillOnFast:	procButtonStillOnFast( option );	break;	// ボタン（押し続け、速く変化）
				default:													break;	// For Warning!
			}
#if defined( DEF_SimAutoTest ) || defined (DEF_SimEnableScreenId) //Modified by HCL for PanelTransition (23-Dec-2010)		// 自動評価で有効. 
			setAutoTestKeyNoNotice( message, option );	// SimWinへキーNo.の通知をする.
#endif	// #if defined( DEF_SimAutoTest ) || defined (DEF_SimEnableScreenId) //Modified by HCL for PanelTransition (23-Dec-2010)		// 自動評価で有効. 
			// キーボード内のキーが連続押下された場合のキーリピート対応.
		} else if( ( message == MS_KeyBoard ) && ( MT_StillOnFast == type ) ) {
			// キーボード用キーリピート.
			procKeyBoardStillOnFast( option );
		} else if( ( message == MS_USBKey ) && ( MT_StillOnFast == type ) ) {
			// USBキーボード用キーリピート.
			procUSBKeyBoardStillOnFast( option );
		} else {
			if( type == MT_On || type == MT_DontCare ) {
				// ボタンが押された時、又は、気にしないメッセージの時
				switch( message ) {
					// タッチパネル内の画面操作メッセージ
					case MS_Back:						procBack( option );									break;	// 戻る
					case MS_Return:																					// 戻る （１つ上の画面（階層）に遷移）
						procReturn( option );
						::moveVoiceCursor();
						break;
					case MS_IndexButton:  				procIndexButton( option );							break;	// インデクスボタン
					case MS_GoToPage:																				// 画面移動
						procGoToPage( option );
						break;
					case MS_Ok:																						// 設定終了
						if( m_pstVoiceGamenInfo && ::getVoiceCursorMode() != TYPE_VoiceGuide_OFF ) {
							procOkVoice( option );
							break;
						} else {
							procOk( option );
// comment : WebUI2 画面切替え制御 -> 20180302 Chikuan.
							if( ( False == PNLC_PopupListControl::newInstance()->isPopupOpen() )
							 && ( ( NULL == dynamic_cast< PUtilitycom* >( nowGamen ) )
							   || ( NULL == dynamic_cast< PServiceCom* >( nowGamen ) )
							   || ( NULL != dynamic_cast< PUtilitySalesCounterCom* >( nowGamen ) ) )
							 && ( PCGE_BuzzerType_InputNg != PBasicMessageProc::e_BuzzerType ) ){
								PNLC_WebUIControl::getInstance()->setDataCollectMethodKind( PNLE_WebUIDataCollectMethodKindAllData );
							}
// comment : WebUI2 画面切替え制御 <- 20180302 Chikuan.
							break;
						}
						break;
					case MS_Cancel:																					// 設定取消
						procCancel( option );
						break;
					case MS_End:						procEnd( option );									break;	// 終了
					case MS_GoToPaper:					procGoToPaper( option );							break;	// 用紙キー（コピー）
					case MS_GoToMag:					procGoToMag( option );								break;	// 倍率キー（コピー）
					case MS_GoToFace:					procGoToFace( option );								break;	// 片面/両面キー（コピー）
					case MS_GoToColor:					procGoToColor( option );							break;	// カラーキー（コピー）
					case MS_KeyBoard:
					{
						if( m_pstVoiceGamenInfo && ::getVoiceCursorMode() != TYPE_VoiceGuide_OFF ) {
							procKeyBoardOnVoice( option );	// キーボード上のキー
							break;
						} else {
							procKeyBoard( option );			// キーボード上のキー
							break;
						}
					}
					case MS_JobDelete:  				procJobDelete( option );							break;	// ジョブリスト消去キー
					case MS_JobButton:  				procJobButton( option );							break;	// ジョブリストのジョブ選択キー
					case MS_LeftIndex:					procLeftIndex( option );							break;	// 左エリア・インデックスキー.
					case MS_BookMark:																				// 左エリア・ブックマークキー（しおりキー）.
						procBookMark( option );
						::moveVoiceCursor();
						break;
					case MS_PreviewDetail:				procPreviewDetail( option );						break;	// プレビュー詳細.
					case MS_ScreenJump:					procScreenJump( option );							break;	// [画面ジャンプ]
					// ワンタッチキーメッセージの追加
					case MS_OneTouchKey:
					{
						if( m_pstVoiceGamenInfo && ::getVoiceCursorMode() != TYPE_VoiceGuide_OFF ) {
							procOneTouchKeyVoice( option );	// ワンタッチキー
							break;
						} else {
							procOneTouchKey( option );		// ワンタッチキー
							break;
						}
					}
					// ボックスのメッセージ追加
					case MS_BoxUp:						procBoxUp( option );								break;	// ボックス画面ページアップ
					case MS_BoxDown:					procBoxDown( option );								break;	// ボックス画面ページダウン
					case MS_BoxIndex:					procBoxIndex( option );								break;	// ドキュメント一覧インデックス
					case MS_BoxButton:					procBoxButton( option );							break;	// ボックスボタン
					case MS_DocButton:					procDocButton( option );							break;	// ドキュメント一覧リストボタン
					case MS_Execute:
						procExecute( option );		// 実行.
						break;
					// ダイレクトキー操作によるメッセージ
					case MS_Reset:																					// パネルリセット
					{
						PCGE_BuzzerType e_BuzzerType = getBuzzerType();
						procReset( option );
						// 正常終了時、画面ズームの拡大表示を等倍表示へ.
						if( ( e_BuzzerType == getBuzzerType() ) || ( PCGE_BuzzerType_Motion1 == getBuzzerType() ) ) {
							procInitZoomFactor();
						}
// comment 2020/01/06 【SCD2-sky延原】【TAS000000002304】C360i/PaperCutでカードが反応しなくなる.
						APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
						if( NULL == pc_SoftDipSw ) {
							LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
							break;
						}
						if( True == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW136, 1 ) ) {	// DipSW No.136=0x02.
// comment 2019/05/16 Sky東島 [EagleHigh][OP_BTS-4894].
							if( PCGE_BuzzerType_InputNg != getBuzzerType() ){
								if( NULL != ppc_UserAuthentication ){
									TYPE_UserAuthentication e_UserAuthentication = ppc_UserAuthentication->getUserAuthentication(PNLC_NetworkLan::getInstance()->getLine());
									if( ( ( TYPE_UserAuthenticationIntermeddiateServer == e_UserAuthentication )								// 拡張サーバー認証.
									   || ( TYPE_UserAuthenticationDeviceAndMidAuth == e_UserAuthentication ) )									// 拡張サーバー認証+本体認証
									 && ( TYPE_ExtTypeBrowserAuth == PnlComGetOapAuthenticationAplType() )										// 認証アプリタイプ「BrowserAuth」.
									 && ( NULL == PnlComGetUser() ) ){
										APIC_AuthDeviceCommon* pc_AuthDevCom = PnlComGetAuthDeviceCommon();
										if( NULL != pc_AuthDevCom ){
											pc_AuthDevCom->clearAuthLoginUserData();
										}
									}
								}
							}
						}
					}
					break;
#ifdef DEF_A4_Cus
					case MS_Start:																					// スタートキー.
					{
						PNLC_SysPerformanceLogLib* pc_SysPerformanceLogLib = PNLC_SysPerformanceLogLib::newInstance();
						if ( NULL != pc_SysPerformanceLogLib ) {
							pc_SysPerformanceLogLib->setSysPerformanceLog( PNLE_SysPerformanceLog_StartKeyOn );
						}
						procStart( option );
					}
					break;
#else	// DEF_A4_Cus.
					case MS_Start:
						THRF_StopBootPanel();	// debugSoftPerformanceLogOn( DBGE_SoftPerformanceLogPoint_KeyStart );
						procStart( option );
						break;	// スタートキー
#endif	// DEF_A4_Cus.
					case MS_Stop:		  				procStop( option );									break;	// ストップ
					case MS_10Key:
						PnlComCheckRemoteKeyboardAndIMEEnd();
						if( option < 10 ) {
							proc10KeyVoice( option );	// テンキー
						} else if( option == 10 ) {
							procShrpVoice( option );	// シャープキー
						} else if( option == 11 ) {
							procAsterVoice( option );	// アスタリスクキー
						}
						if( True == ( Bool )i32_option ) { // ソフトウェアキーの場合.
							if( PCGE_BuzzerType_InputOk == getBuzzerType() ) {
								setBuzzerType( PCGE_BuzzerType_Motion2 );
							}
						}
						break;
					case MS_Clear:                                                                                  // クリアー.
						PnlComCheckRemoteKeyboardAndIMEEnd();
						procClear( option );
						if( True == ( Bool )i32_option ) { // ソフトウェアキーの場合.
							if( PCGE_BuzzerType_InputOk == getBuzzerType() ) {
								setBuzzerType( PCGE_BuzzerType_Motion2 );
							}
						}
						break;
					case MS_ColorMode:																				// カラーモード
						procColorMode( option );
#if defined(WIN32) && defined(SIMULATOR)	// 言語対応支援. 
						procMode( option );			//■言語対応用
#endif	// defined(WIN32) && defined(SIMULATOR)	// 言語対応支援. 
						break;
					case MS_IntMode:
					{
						PCGE_BuzzerType e_BuzzerType = getBuzzerType();
						procIntMode( option );
						// 正常終了時、画面ズームの拡大表示を等倍表示へ.
						if( e_BuzzerType == getBuzzerType() || PCGE_BuzzerType_Base == getBuzzerType() ) {
							procInitZoomFactor();
						}
					}
					break;	// 割り込みモード
					case MS_IDInput:	  				procIDInputVoice( option );							break;	// 暗証番号
					case MS_Service:	  				procService( option );								break;	// サービスマンモード
					case MS_Copy:		  				procCopy( option );									break;	// コピー切り換えキー
					case MS_PreHeatOnByKeyAns:			procPreHeatOnByKeyAns( option );					break;	// プレヒートキー押下応答
					case MS_EnergySaveReset:			procEnergySaveReset( 1 );							break;	// 省エネ解除.
					case MS_Enlarge:
						if( True == PNLC_HitachiICC_TopMenuCustomize::newInstance()->isHitachiICC_Mode() ) {
							setBuzzerType( PCGE_BuzzerType_InputNg );
							break;
						}
// comment 2013/11/25 OPS1-Sky稲井 [特ROM対応][MZ2ND-81] Scan機能を使用時にはPDFまたはコンパクトPDF + 暗号化のみ使用可能にしたいとその他2件対応.
						// DipSW:202 HEX:0x02が有効時.
						if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW202, 1 ) ) {
							setBuzzerType( PCGE_BuzzerType_InputNg );	// 拒否音鳴動.
							break;
						}
						procEnlarge( option );
						break;
					case MS_SampleCopy:																				// 確認コピー.
					{
						APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
						if( NULL != pc_SoftDipSw ) {
							if( True == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW208, 0 ) ) {	// DipSW No.208=0x01.
								setBuzzerType( PCGE_BuzzerType_InputNg );
							} else {
								procSampleCopy( option );
							}
						} else {
							LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return;
						}
					}
					break;
					case MS_JobMemory:					procJobMemory( option );							break;	// ジョブメモリ
					case MS_Utility:					procUtility( option );								break;	// ユーティリティ
					case MS_Fax:						procFax( option );									break;	// FAX
// Comment 2018/01/03 [HCLT_Abinav][RQ-686][IT5_4.2][AR1033014] changing hardkey press functionality for FAX and WebBrowser in NW2.
//					case MS_FaxOnly:					procFaxOnly( option );								break;	// ファクス専用.
					case MS_FaxOnly:
						if( TYPE_NetworkSpecific_NW2 == e_NetworkSpecific ) {
							setBuzzerType( PCGE_BuzzerType_InputNg );
						} else {
							procFaxOnly( option );
						}
						break;	// ファクス専用.
					case MS_Box:						procBox( option );									break;	// ボックス
					case MS_PowerSave:					procPowerSave( option );							break;	// パワーセーブ
					case MS_Help:
						if( True == PNLC_HitachiICC_TopMenuCustomize::newInstance()->isHitachiICC_Mode() ) {
							setBuzzerType( PCGE_BuzzerType_InputNg );
						// [IT6_18]立体コピー 2SCD2G Sky add.
						} else if ( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW205, 6 ) ) {
							PBasicMessageProc* pNowGamen = getNowGamen();
							if( ( pNowGamen != &gp_CopyBase )				// コピー基本画面でない.
							 && ( pNowGamen != &gp_EnlCopy_Basic ) ) {		// コピー基本画面（拡大表示）でない. 
								setBuzzerType( PCGE_BuzzerType_InputNg );	// 受付拒否音.
							} else {
								procCapsulePaperSettingOn();
							}
						} else {
							procHelp( option );
						}
						break;
					case MS_Accessibility:				procAccessibility( option );						break;	// ユーザー補助
// comment 2015/04/30 OPS1-Sky佐藤 [HeliosL][CQ513137] ズームキーの登録キー割り当て対応.
					case MS_ZoomIn:						procZoomIn();										break;	// 表示ズームキー押下.
					case MS_JobControl:					procJobControl( option );							break;	// ジョブ操作キー
					case MS_VoiceGuide:					procVoiceGuide( option );							break;	// 音声ガイド
					case MS_SlideMenu:	// スライドメニュー.
						if( True == PNLC_SlideMenuControlLib::newInstance()->isShown() ) {
							if( False == PNLC_SlideMenuControlLib::newInstance()->isOpen() ) {
								// スライドメニューオープン.
								procSlideMenu( option );
							} else {
								// スライドメニュークローズ.
								PNLC_SlideMenuControlLib::newInstance()->close( True );
							}
						} else {
							setBuzzerType( PCGE_BuzzerType_InputNg );
						}
						break;
// commnet 2015/03/23 OPS1-Sky守川 [Helios][スライドメニュードラッグ対応].
					case MS_SlideMenu_Press: // スライドメニュータブキー押下.
						if( True == PNLC_SlideMenuControlLib::newInstance()->isShown() ) {
							procSlideMenudisplayKey( option );
						}
						break;
					case MS_SpecifyApp:					procSpecifyApp( option );							break;	// 指定アプリ起動.
					// 小ＬＣＤ用
					case MS_Program:					procProgram( option );								break;	// プログラム
					// タイマー関連のメッセージ
					// 定期起床（５００ｍｓ）と１秒タイマーが重なった時１秒タイマーメッセージを発生させる為、１秒タイマー時も定期起床処理を起動するようにする
					case MS_Second:						procSecond( option );										// １秒タイマ
// comment 2015/11/10 MSE岡田 [MinervaSBK][制御構成変更No.74]パネルハング対応.
														procSecondForForceSystemAutoReset( option );				// １秒タイマ（強制システムオートリセット用）.
						procAutoTimeCorrect( );										// 自動時刻補正　2008.04.03 Add 大庭.
						PLBC_LogOutputStatus::getInstance()->changeLogStatus( PNLE_LogOutputStatus_Output );
					case MS_TimerWakeUp:																			// 定期起床.
						// アニメーション中は定期起床をスキップする
						if( isNowPopupAnimation() != False ) {
							m_bPopupAnimeAct = True;
						} else {
							onlyPanel.clearIsWarningRet();	// 2011/04/21 ohba IsWarning 余計な処理減らし対応.
							procTimerWakeUp( option );
						}
// comment 2015/04/02 OPS1-Sky浜野 [ZeusS][CQ510790] 実行結果が表示されない不具合対応.
						// HDDミラーリング画面以外の場合.
						if( &gp_ServiceRebuild != nowGamen ) {
							// リビルド状態を更新.
							onlyPanel.updateHddRebuildStatus();
						}						
						break;
					case MS_Animation:					animateNowGamen();									break;	// アニメーション表示.
					case MS_Animation2:					animateNowGamen2();									break;	// 新規UIアニメーション.
					// 他タスクからのメッセージ
					case MS_AutoResetReq:				procAutoResetReq( option );							break;	// オートクリア要求
					case MS_JobLogChange:				procJobLogChange( option );							break;	// ジョブ破棄エラー発生通知
					case MS_IntModeAns:					procIntModeAns( option );							break;	// 割り込みモード応答
					case MS_AppliDisplayReq:			procAppliDisplayReq( option );						break;	// アプリ表示要求
					case MS_FTPConnection:				procFTPConnection( option );						break;	// ＦＴＰ接続完了
					case MS_WebDAVConnection:			procWebDAVConnection( option );						break;	// ＷｅｂＤＡＶ接続完了.
					case MS_ResetAtCountNotice:			procResetAtCountNotice( option );					break;	// カウンタ引き抜き通知
					case MS_JobStatusChange:			procPrintJobStopChange( option );					break;	// （プリント）ジョブ状態変化通知
					case MS_RPnlFunctionNotice:			procRPnlFunctionNotice( option );					break;	// リモートパネル機能通知
					case MS_RPnlCommunicationNotice:	procRPnlCommunicationNotice( option );				break;	// リモートパネル通信状態通知
					case MS_RPnlStatusNotice:			procRPnlStatusNotice( option );						break;	// リモートパネル状態表示通知
					case MS_RPnlCloseRequest:			procRPnlCloseRequest( option );						break;	// リモートパネルクローズ要求
					// コールバック関数/オブザーバーからのメッセージ
					case MS_Observer:					procObserver( ( TYPE_PNL_Observer )option );			break;	// オブザーバによる通知
					case MS_CallBack:
// comment 2016/11/18 SCD2-Sky西村(宏) [特ROM][CQ394604] Safe環境にて、ログインおよびジョブリストの表示に時間がかかる.
						if( TYPE_PNL_CallBack_AnsOpenAPI == i32_option && True == PCGC_AuthOapCommonStaticLib::b_ReqGetUserConfiguration) {
							APIC_ExtSetting *ext = PCGC_AuthOapAPIAccess::getExtSettingAPI();
							if ( NULL == ext ) {
								LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
							} else  {
								ext->semunlock();
							}
						}
// comment 2015/12/22 OPS1 Sky東島 [ZeusSZX0][CQ521680].
						if( TYPE_PNL_CallBack_OriginalSizeFix == option ){
							onlyPanel.setNowOriginalSizeRequest( False );		// 原稿サイズ確定要求中フラグを落とす.
						}
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
						if( ( TYPE_PNL_CallBack_BrowserReboot == i32_option )
						 && ( NULL != PNLC_WebUIDebug::getInstance() ) ){
							PNLC_WebUIDebug::getInstance()->outputDebugLog( "---- WebUI Debug : receive TYPE_PNL_CallBack_BrowserReboot ----.\n" );
						}
						if( b_callBkFunc == False ) {
							procCallBack( ( TYPE_PNL_CallBack )i32_option );	// コールバックによる通知
						}
						break;
					case MS_CallBackNtfEvent:			procCallBackNtfEvent( option, st_NtfEventData );		break;	// コールバックによる通知(新API通知処理用).
					// ボックスオブザーバーからのメッセージ
// comment 2019/08/19 Sky東島 [EagleHigh][1TB対応].
//					case MS_BoxObserver:				procBoxObserver( option );							break;	// ボックスオブザーバによる通知
					case MS_BoxObserver:																			// ボックスオブザーバによる通知
						procBoxObserver( option );
						// Boxオブザーバー通知処理が完了したため.
						// Boxオブザーバー処理のスキップ用フラグをクリアする.
						onlyPanel.setDataChangedBoxID( -1 );						// データ変化したBoxID初期化.
						onlyPanel.setSkipReqestListBoxID( -1 );						// リスト取得をスキップするBoxIDを初期化.
						break;
					case MS_VederCallBack:		 		procVederCallBack( option );						break;	// ベンダー側処理完了通知.
					case MS_AdminMachineKMCardNotice:	procAdminMachineKMCardNotice( option );				break;	// 統合管理装置 カード挿入/引き抜き通知.
					case MS_UsbPullingOut:				procUsbPullingOut( option );						break;	// USBメモリー引き抜き通知.
					case MS_ThumbnailCallBack:			procThumbnailCallBack( option );					break;	// サムネイル画像取得完了コールバックによる通知
					case MS_PreviewCallBack:			procPreviewCallBack( option );						break;	// プレビュー画像取得完了コールバックによる通知
#ifdef	DEF_THAMES_ICUSTOM
// Comment 2018/01/03 [HCLT_Abinav][RQ-686][IT5_4.2][AR1033014] changing hardkey press functionality for FAX and WebBrowser in NW2.
//					case MS_Browser:					procBrowser( option );								break;	// ブラウザキー押下.
					case MS_Browser:
						if( TYPE_NetworkSpecific_NW2 == e_NetworkSpecific ) {
							setBuzzerType( PCGE_BuzzerType_InputNg );
						} else {
							procBrowser( option );
						}
						break;	// ブラウザキー押下.
#endif	// DEF_THAMES_ICUSTOM
					case MS_Soft10Key:					procSoft10Key( option );							break;	// ソフトテンキーキー押下.
					case MS_TemporaryJob:				procTemporaryJob( ( TYPE_PNL_TemporaryJob )option );	break;	// 一時的な割込みJOB受信メッセージ対応処理
					case MS_CellularPhoneStatus:		procCellularPhoneStatus( ( TYPE_PNL_AppStatus )option );	break;	// 携帯連携通信状態通知.
					case MS_VoicePlayEnd:				procVoicePlayEnd( option );							break;	// 音声再生終了通知
					case MS_VoiceStopEnd:				procVoiceStopEnd( option );							break;	// 音声再生停止終了通知
					case MS_USBKey:						procUSBKeyBoard( option );							break;	// USBキーボードキー.
					case MS_SPCButton:	// SPC用ボタンON.
						if( m_pstVoiceGamenInfo && ::getVoiceCursorMode() != TYPE_VoiceGuide_OFF ) {
							procSPCButtonOnVoice( option );	break;	// ボタン（押された）
						} else {
							procSPCButtonOn( option );	break;	// ボタン（押された）
						}
					case MS_ULControlNotice:			procULControlNotice( option );						break;	// アップロード通知. // 【304362】【Webブラウザからのスキャン画像の送信】【13山本(由)】merge.
					case MS_AppReqObserver:				procAppReqObserver( ( TYPE_PNL_AppReqObserver )option ); break;	//【304362】【AppReq対応】【13山崎】merge.
					case MS_PkiCardStatusChange:		procPkiCardStatusChange( option );	break;	// PKIカード状態通知メッセージ.
					case MS_DoubleTap:					procDubleTap( option );								break;	// 【画面実装課題 No.131】ダブルタップ.
					// 外キー削除.
					case MS_Menu:						procMenu( option );									break;	// メニューキー.
					case MS_APPsMenu:					procAPPsMenu( option );								break;	// 登録アプリケーションメニューキー処理.
					case MS_UbqPrint:					procUbqPrint( option );								break;	// ユビキタスプリント.
#if !defined(DEF_MinervaPreEmu) && !defined(DEF_ZeusRevCus)
					case MS_PanelHardOnOffNotice:		procPanelHardOnOff( ( TYPE_PNL_HardOnOff )option );	break;	// パネル電源OnOffの通知(1:ON, 0:OFF).
#endif
					case MS_QtButton:					procQtButtonOn( ( ushort )i32_option );				break;	// Qtボタン押下.// [MTUI]No341633対応.
					case MS_QtDoubleTap:				procDoubleTap( ( ushort )i32_option );				break;	// Qtダブルタップ.// [MTUI]No341633対応.
					case MS_QtAction:							                                        // Qtアクション発生通知.
						procQtAction( ( ushort )i32_option );
						break;
					case MS_QtBuzzer:							                                        // Qtブザー鳴動通知.
						procQtBuzzer( ( ushort )i32_option );
						break;
					case MS_QtEventNotice:			// Qtイベント発生通知.
						procQtEventNotice( ( ushort )i32_option );
						break;
					case MS_QtLongTap:					procQtLongTap( ( ushort )i32_option );				break;	// Qtロングタップ通知.
					case MS_QtFlickEnd:					procQtFlickEnd( ( ushort )i32_option );				break;	// Qtでのフリックアニメーション終了通知.
					case MS_QtColumnMove:																			// Qt水平スクロールバー移動通知.// [MTUIv1開発]No360420対応.
						procQtColumnMove( ( ushort )i32_option );
						break;
					case MS_QtThumbnailImage:			procQtThumbnailImage( i32_option );					break;
					// comment 2018/11/30 SCD2-Sky竹菴 [Eagle][1068133] 関数コール後にデータを初期化する.
//					case MS_DetectWarning:				procDetectWarning( option );						break;	// 警告検出通知.
//					case MS_OpenPopup:					procOpenPopup( option );							break;	// ポップアップ表示通知.
					case MS_DetectWarning:																	// 警告検出通知.
// comment 2019/03/22 SCD2-4G東島 [EagleH][OP_BTS-3594].
//						procDetectWarning( option );
						if( NULL != dynamic_cast<PBaseGamenCom*>( nowGamen ) ){
							nowGamen->procDetectWarning( option );
						} else {
							procDetectWarning( option );
						}
						PNLC_AppChangeStatusFormBrowser::getInstance()->initData();
						break;
					case MS_OpenPopup:																		// ポップアップ表示通知.
// comment 2019/03/22 SCD2-4G東島 [EagleH][OP_BTS-3594].
//						procOpenPopup( option );
						if( NULL != dynamic_cast<PBaseGamenCom*>( nowGamen ) ){
							nowGamen->procOpenPopup( option );
						} else {
							procOpenPopup( option );
						}
						PNLC_AppChangeStatusFormBrowser::getInstance()->initData();
						break;
					case MS_BadgeButton:				procFunctionBudge( option );						break;	// FunctionBadgeButton押下.
					case MS_AppChgBaseGamen:			procChgBaseGamen( option );							break;	// パネル切替時のベース画面描画要求.
					case MS_QtMovePage:					procQtMovePage( static_cast<ushort>( i32_option ) );		break;	// Qtでの移動ページ通知.
					case MS_QtRotate:					procQtRotate( ( ushort )option );						break;	// Qt回転. 2011/11/07 k.nakada 遠隔操作AndroidFum Boxプレビュー自動回転制御対応.
					case MS_QtHandleAreaTouch:			procQtHandleAreaTouch( ( ushort )option );			break;	// ソフト外キーのハンドルエリアタップ通知 2012/1/18 k.morita CQ.360638対応.
					case MS_TabletDisconnect:			procTabletConnectChange( MS_TabletDisconnect, option );	break;	// タブレット接続/切断時の共通処理.
					case MS_TabletConnect:				procTabletConnectChange( MS_TabletConnect, option );	break;	// タブレット接続/切断時の共通処理.
					case MS_TabletConnectConfirm:		procTabletConnectConfirm( option ); break;					// タブレット接続許可確認通知 2014/09/10 s.ohba [ZeusS][UV-01448]追加.
					case MS_IMETxtNotice:
						PNLC_IMEManager::getInstance()->reqUpdate();	// IME文字列情報をこのタイミングで更新する.
						procIMETxt( option );							// IME文字列受信通知.
						break;
					case MS_ReqIMEEnd:
						procReqIMEEnd( option );						// IME終了要求.
						break;
					case MS_TabletMainPanelTouchNotice:	procTabletMainPanelTouchNotice( option );			break;	// 本体パネルタッチ時に切断確認通知.
					case MS_RMPStrInput:				procRMPStrInput( option );							break;	// リモートパネル文字列入力.
					case MS_IMETabletConnect:			procTabletIMEConnect( option );						break;	// IME連携用タブレット端末接続通知.
					case MS_IMETabletDisconnect:		procTabletIMEDisconnect( option );					break;	// IME連携用タブレット端末切断通知.
					case MS_StopIMENotice:				procStopIME( option );								break;	// IME文字入力停止通知.
					case MS_TabletReset:				procTabletReset( option );							break;	// タブレット接続／切断時のリセット.
					case MS_QtBroadcastPadButton:		procQtBroadcastButton( (ushort)i32_option );		break;		// 同報宛先パッド押下通知.
					case MS_DeleteBroadcastAddress:		procDeleteBroadcastAddress( static_cast<TYPE_SendMode>( option ) );	break;		// 同報宛先削除通知.
// comment 2014/05/16 OPS1-Sky田村 [特Rom対応][MZV-1]立体コピー対応.
					case MS_CapsulePaperSetting:		procCapsulePaperSettingOn();						break;	// 立体コピー専用設定への切り替え処理(コピー基本画面のみ).
// comment 2016/10/06 MSE岡田 [IT5_3.3][1005694] IGCWaitリクエストのメッセージ処理を追加.
					case MS_ReqIGCWait:					procIGCWaitRequest( i32_option );					break;	// IGCWaitリクエスト通知.
// comment 2016/12/08 MSE岡田 [IT5_3.4][1008981] スライドメニュー表示判定要求を受けての表示判定処理を追加.
					case MS_ReqSlideMenuJudgeShown:																	// スライドメニュー表示判定要求通知.
						PNLC_SlideMenuControlLib::newInstance()->judgeShown();
						break;
// [特ROM対応][IT6-43] Sky西村 認証アプリケーションのログイン画面から直接パブリックユーザーでログイン.
					case MS_PublicLogin:				procPublicLogin( option );							break;	// パブリックユーザーログイン.
// comment 2022/09/22 Sky東島 [IT6_2.3][制御構成変更_No.51][サービスレスメンテナンス].
					case MS_ClosePopupBrowser:			procClosePopupBrowser( option );					break;		// ポップアップブラウザクローズ.

// 2022/08/18 [IT6-103(機種マージ)] Sky奥田 全国銀行協会による手形・小切手電子化に伴うスキャン機能カスタマイズ要望 Start.
					case MS_CheckValueSet:				procCheckValueSet();								break;	// 小切手.
					case MS_BillsValueSet:				procBillsValueSet();								break;	// 手形.
// 2022/04/08 [IT6-103] Sky前田 全国銀行協会による手形・小切手電子化にMS_Reset_DirectKeyスキャン機能カスタマイズ要望 End.
					default:																				break;	// For Warning!
				}
				// カーソル表示
				if( PNLC_VoiceCursorControl::newInstance()->IsHideCursorFromSetupGamen() ) {
					::showVoiceCursor();// panel/plibh/plihg/plibhvoice.h
				}
#if defined( DEF_SimAutoTest ) || defined (DEF_SimEnableScreenId) //Modified by HCL for PanelTransition (23-Dec-2010)		// 自動評価で有効. 
				setAutoTestKeyNoNotice( message, option );	// SimWinへキーNo.の通知をする.
#endif	// #if defined( DEF_SimAutoTest ) || defined (DEF_SimEnableScreenId) //Modified by HCL for PanelTransition (23-Dec-2010)		// 自動評価で有効. 
			} else if( ( type == MT_Up ) && ( message == MS_Return ) && ( ( isNowGamen( &gp_WeeklyTimerInputPass )==True ) || ( isNowGamen( &gp_WeeklyTimerInputTime )==True ) ) ) {
				// ＷＴパスワード入力（G1260）画面、又は、ＷＴ使用時間設定（G1261）画面の時、MS_Returnキーが離されたら、キーアップ用のoption値でprocReturn()処理を行なう.
				procReturn( BTNID_KEY_WEEKLYTIMERCANCELKEYUP );	// 戻る （１つ上の画面（階層）に遷移）.
			}
		}
	}
	PNLS_ClipArea	s_ClipArea;
	memset( &s_ClipArea, 0, sizeof( PNLS_ClipArea ) );

	switch( message ) {
		case MS_PanelLock:		// パネルロック要求.
#if defined (QT_PANEL)
		{
			PM_TaskID priTask;
			PMGC_PanelManager::getPriority( &priTask );
			if( ( PM_TASKID_PANEL == priTask ) || ( PM_TASKID_BRWPNL == priTask ) ) {
				PMGC_PanelManager::newInstance()->updateAllWidget();				// comment 2014/10/22 OPS1-Sky石賀 [Poseidon][CQ501179].
			}
		}
#endif
// Comment 2017/11/29 【HCLT-Deeksha】【Eagle】【制御構成変更_No.4】 Removing old function version check condition.
		if( NULL != APIC_RemotePanelSetting::newInstance() ) {
			APIC_RemotePanelSetting::newInstance()->setLockStatus( True );
		}
		break;
		case MS_PanelUnLock:	// パネルアンロック要求.
		{
			// パネルロック解除.
			onlyPanel.e_PanelLock = TYPE_PanelLock_Unlocked;
			PM_TaskID e_TaskId = PM_TASKID_PANEL;
			if( 0 == PMGC_PanelManager::getPriority( &e_TaskId ) ) {
				// 最優先タスクが取得できない場合は、パネルを最優先タスクとして処理する.
				PnlComPutDebugLog( False, UC_PNL_LOGPATTERN_1, "messageDispatch", UC_PNL_ERROR_IMPOSSIBLE );
				e_TaskId = PM_TASKID_PANEL;
			}
			// ブラウザ起動中でも、リモートパネルの接続を可能とする.
			if( PM_TASKID_BROWSER == e_TaskId || PM_TASKID_BRWPNL == e_TaskId ) {
				gp_PublicPopupDummyBase.setupOpenPopup( &gp_RemotePanelNowOperatingNoPanelLock_warn );
				gp_PublicPopupDummyBase.setupNextGamen( this );
				gp_PublicPopupDummyBase.setMode( PNLE_PublicPopupDummyModeAct );
				nextGamen = &gp_PublicPopupDummyBase;
				OpenPopupIndi();
			} else {
// Comment 2017/11/29 【HCLT-Deeksha】【Eagle】【制御構成変更_No.4】 Removing old function version check and if portion.  
				// サービス画面が非表示の場合のみ、ポップアップを表示.
				if( False == ppc_SystemStatus->getServiceModeOn() ) {
					if( onlyPanel.bl_isEnlargeDisplayMode == False ) {	// 通常表示中.
						PLayerControl* p_GamenInstance = dynamic_cast< PLayerControl* >( nowGamen );
						if( p_GamenInstance != NULL ) {	// 新UI画面.
							// A0A9080100 リモートパネル操作中(パネルロック無し).
							PNLC_PopupListControl::newInstance()->addPopup( &gp_RemotePanelNowOperatingNoPanelLock_warn, NULL, NULL, 360, 216, 80, 48, False, True );
						} else {
							// 80A90000 リモートパネル操作中(パネルロック無し)に遷移.
							nextGamen = &gp_WarningRemotePanelNowOperatingNoPanelLock;
						}
					} else {
						// 80M00000 リモートパネル操作中(パネルロック無し)に遷移.
//							nextGamen = &gp_EnlWarningRemotePanelNowOperatingNoPanelLock;
						// Launch as ppopup screen.
						PNLC_PopupListControl::newInstance()->addPopup( &gp_EnlWarn_RemotePanelNowOperatingNoPanelLock );
					}
				}
// comment 2022/12/15 FPT.ThanhLH11 [IT6_2.3][ALL MACHINE][OP_BTS-40276]リモートパネル（サーバー型）で、サービス画面で本体のパネルと接続先PCの画面が同期できてしまう問題対策.
// 				else {
// 					if( NULL != APIC_RemotePanelSetting::newInstance() ) {
// 						// APIC_RemotePanelSetting::newInstance()->setLockStatus( False );
// 					}
// #if defined (QT_PANEL)
// 					{
// 						PM_TaskID priTask;
// 						PMGC_PanelManager::getPriority( &priTask );
// 						if( ( PM_TASKID_PANEL == priTask ) || ( PM_TASKID_BRWPNL == priTask ) ) {
// 							if( NULL != PMGC_PanelManager::newInstance() ) {
// 								PMGC_PanelManager::newInstance()->updateAllWidget();
// 							}
// 						}
// 					}
// #endif
// 				}
			}
			break;
		}
		case MS_RemoteStart:	// リモート開始要求.
			setupGamen();
			if( False == onlyPanel.bl_ServiceMode ) {
#if defined (QT_PANEL)
				PM_TaskID priTask;
				PMGC_PanelManager::getPriority( &priTask );
				if( ( PM_TASKID_PANEL == priTask ) || ( PM_TASKID_BRWPNL == priTask ) ) {
					PMGC_PanelManager::newInstance()->updateAllWidget();				// comment 2014/10/22 OPS1-Sky石賀 [Poseidon][CQ501179].
				}
#endif
			}
			// リモートパネル接続時では「PLBC_MultiTouchFunction::getMultiTouchMode()」の.
			// 戻り値が「PNLE_MTUIMODE_NONE」となる為、以下の条件を使用.
			// 機能拡張3以降 且つ マルチタッチ有効の場合.
			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type3()
			    && False == cp_SoftDipSw->getSoftDipSwBitForPanelSpecNo( PNLD_MTUIGESTURE, 0 ) ) {
				PNLC_QWSConnector* pc_QWSConnector = PNLC_QWSConnector::newInstance();
				APIC_OperationSetting* pc_OperationSetting = PnlComGetOperationSetting();
				if( ( NULL != pc_QWSConnector ) && ( NULL != pc_OperationSetting ) ) {
					// 画面ズーム機能の使用有無を通知する.
					if( TYPE_DispZoomEnable == pc_OperationSetting->getDispZoom() ) {
						pc_QWSConnector->setZoomMode( False );
					}
				}
			}
// comment 2020/11/30 DWS1-Sky延原 【TAS000000003634】リモートパネル使用時にパスワード入力のキー操作がハイライト表示されてしまう.
			procRemotePanelStart( option );
			break;
		case MS_RemoteStop:		// リモート停止要求.
			// パネルロック解除.
			onlyPanel.e_PanelLock = TYPE_PanelLock_Unlocked;
			if( onlyPanel.uc_RemoteStopStatus != PM_RMPSTOPTYPE_NONE ) {	// エラー発生でリモート操作停止.
				if( True == onlyPanel.bl_RemoteServiceStopRequestFromMFP ) {
					onlyPanel.bl_RemoteServiceStopRequestFromMFP = False;
					onlyPanel.uc_RemoteStopStatus = PM_RMPSTOPTYPE_NONE;
					break;
				}
				// 初期画面に戻る.
				if( True == onlyPanel.bl_ServiceMode ) {
					procEnd( 0 );
				} else {
					if( True == c_ExtPanel.req_SpecifiedAppStart() ) {
						PnlComSetInitIndicationApplication();
					} else {	// 従来処理.
						PnlComSetInitIndicationApplication();
						procReset( 0 );
					}
				}
				onlyPanel.uc_RemoteStopStatus = PM_RMPSTOPTYPE_NONE;
			}
			else {
				if( True == onlyPanel.bl_RemoteServiceStopRequestFromMFP ) {
					onlyPanel.bl_RemoteServiceStopRequestFromMFP = False;
					break;
				}
				if( True == onlyPanel.bl_ServiceMode ) {
					procEnd( 0 );
				} else if( True == isNowGamen( &gp_UtilityRemotePanelOperationStop ) ) {
					nextGamen = &gp_UtilityRemotePanelOperationStart;
				}
				else {
#if defined (QT_PANEL)
					PM_TaskID priTask;
					PMGC_PanelManager::getPriority( &priTask );
					if( ( PM_TASKID_PANEL == priTask ) || ( PM_TASKID_BRWPNL == priTask ) ) {
						PMGC_PanelManager::newInstance()->updateAllWidget();				// comment 2014/10/22 OPS1-Sky石賀 [Poseidon][CQ501179].
					}
#endif
				}
			}
			// マルチタッチV2.
			if( PNLE_MTUIMODE_V2 <= PLBC_MultiTouchFunction::getMultiTouchMode() ) {

				PNLC_QWSConnector* pc_QWSConnector = PNLC_QWSConnector::newInstance();
				APIC_OperationSetting* pc_OperationSetting = PnlComGetOperationSetting();
				if( ( NULL != pc_QWSConnector ) && ( NULL != pc_OperationSetting ) ) {
					// 画面ズーム機能の使用有無を通知する.
					if( TYPE_DispZoomEnable == pc_OperationSetting->getDispZoom() ) {
						pc_QWSConnector->setZoomMode( True );
					}
				}
			}
// comment 2020/11/30 DWS1-Sky延原 【TAS000000003634】リモートパネル使用時にパスワード入力のキー操作がハイライト表示されてしまう.
			procRemotePanelStop( option );
			break;
		case MS_PanelLockVramUpdate:	// パネルロック要求中のVRAM更新通知.
			// アプリ側画面をロック画面から元画面に戻す.
			break;
// comment 2015/03/04 OPS1-Sky浜野 [Poseidon][CQ510605] スナップショット作成要求メッセージ化.
		case MS_SnapshotCreateReq:
			PNLC_SnapshotControlLib::newInstance()->startSnapShotCreate();
			break;
		case MS_SeqCustom:
			if( TYPE_PNL_SeqCustom_Nl_Initialize == i32_option ){			// 初期化応答待ち状態の場合(通常画面).
				rebuildGamen( 1 );
			} else if( TYPE_PNL_SeqCustom_Enl_Initialize == i32_option ){	// 初期化応答待ち状態の場合(拡大画面).
				rebuildGamenEnlarge( 1 );
			}
			break;
		default:	break;
	}

// comment 2022/12/15 FPT.ThanhLH11 [IT6_2.3][ALL MACHINE][OP_BTS-40276]リモートパネル（サーバー型）で、サービス画面で本体のパネルと接続先PCの画面が同期できてしまう問題対策.
	// サービス画面で強制的にパネルロック要求する場合はロック画面を表示.
	// if( onlyPanel.e_PanelServiceLock == TYPE_PanelLock_Lock ) {
	// }
	// サービス画面限定でパネルロックしていたら、終了時にロック解除.
	if( False == ppc_SystemStatus->getServiceModeOn() && 
		( TYPE_PanelLock_Locked == onlyPanel.e_PanelServiceLock || TYPE_PanelLock_Lock == onlyPanel.e_PanelServiceLock ) ) {
		onlyPanel.e_PanelServiceLock = TYPE_PanelLock_Unlocked;
// Comment 2017/11/29 【HCLT-Deeksha】【Eagle】【制御構成変更_No.4】 Removing old function version check and if portion.  
		// サービス画面表示中に、ロック状態の変更がなかった場合、サービス画面遷移時のロック状態に戻す.
		// if( False == onlyPanel.getPanelLockServiceChgFlag() ) {
		if( ( onlyPanel.e_PanelLock != TYPE_PanelLock_Locked ) && ( onlyPanel.e_PanelLock != TYPE_PanelLock_Lock ) ) {
			APIC_RemotePanelSetting::newInstance()->setLockStatus( False );
		} else {
			APIC_RemotePanelSetting::newInstance()->setLockStatus( True );
		}
		// }
		// onlyPanel.setPanelLockServiceChgFlag( False );
	}
// ランニングモード正式対応
	fromMessageDispath();				//　下のクラスにメッセージ発生したことを伝える
	if( ( ( ( message == MS_TimerWakeUp || message == MS_Second ) && ( nextGamen == NULL ) ) || bl_ProcMessage == False ) && ( False == m_bPopupAnimeAct ) ) {	// 条件にMS_Secondを追加（１秒タイマー時も定期起床処理をしているため同様に扱う）
		// 定期起床で画面遷移しない時、または、画面遷移しないとき（警告が解除されるので表示更新する必要がある）
// Comment 2018/02/02 HCL-Hanuman [IT5_4.2] [AR1032240] Skipped one iteration when Popup screen launched.
//	if( NULL == dynamic_cast< PPopupCom* >( this )
		if( ( ( NULL == dynamic_cast< PPopupCom* >( this ) ) && ( True != PNLC_PopupListControl::newInstance()->isPopupOpen() ) )
		    || ( NULL != dynamic_cast< PPopupCom* >( this ) && this == PNLC_PopupListControl::newInstance()->getNowPopup() ) ) {
			setupGamen();	// 表示更新を行う.
		}
		if( True == PNLC_VoiceCursorControl::newInstance()->isReshowCursor()	// 音声ガイドパフォーマンス改善.
		    || True == PNLC_VoiceCursorControl::newInstance()->IsHideCursorFromSetupGamen() ) {
			if( bl_ProcMessage == False ) {	// メッセージ処理を行っていない場合.
				::setVoicePushedParts( NULL );	// 押下したパーツ情報をクリア.
			}
			::showVoiceCursor();// panel/plibh/plihg/plibhvoice.h
		}
		Bool bl_reqMsgAreaVoice = True;
		Bool bl_ScreenTransition = PNLC_EnlVoiceGuidanceControl::getScreenTransition();
		if( NULL != nextGamen ) {
			bl_ScreenTransition = True; // Full screen transition.
		} else if( sc_OldPoupCount != PNLC_PopupListControl::newInstance()->getPopupNum() ) {
			bl_ScreenTransition = True; // Popup removed or added.
		} else if( pOldPopup != PNLC_PopupListControl::newInstance()->getNowPopup() ) {
			bl_ScreenTransition = True; // One popup removed and another added.
		}

		if( True == bl_ScreenTransition ) {
			bl_reqMsgAreaVoice = False;
		} else {
			if( NULL == dynamic_cast< PPopupCom* >( this ) ) {
				if( PNLC_PopupListControl::newInstance()->isPopupOpen() ) {
					bl_reqMsgAreaVoice = False;
				}
			} else {
				if( this != PNLC_PopupListControl::newInstance()->getNowPopup() ) {
					bl_reqMsgAreaVoice = False;
				}
			}
		}

		if( bl_reqMsgAreaVoice ) {
			if( m_pstVoiceGamenInfo ) { ::reqVoiceMsgAreaVID( this ); }		// 音声対応画面の場合、メッセージエリア音声の取得(panel/plibh/plihg/plibhvoice.h).
		}
	}
	onlyPanel.endProcTimerWakeupExec(); // 2011/04/21 ohba IsWarning 余計な処理減らし対応.
}
//================================================================================================
//［機能］	拡大表示.
//================================================================================================
void PBasicMessageProc::procEnlarge( uchar option )
{
	// 汎用アプリ実行時は拡大表示しない。
	if( AplCommonSet.isActiveGeneralApplication() ) {
		return;
	}
	short ss_TableNo = onlyPanel.isDisableModeSet();		//モード設定禁止画面チェック
	if( PnlComGetOperationSetting()->getEnlargeModeChangeConfirm() == True
	    && onlyPanel.bl_nowEnlargeModeCancelConfirm == False
	    && !( ( comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() == TYPE_JobTypeCopy ) && ( onlyPanel.bl_isEnlargeDisplayMode == True ) )
	    && ss_TableNo == 0 ) {
		PLayerControl* pNextPopup = NULL;
		PNLC_PopupListControl* pPopupListControl = PNLC_PopupListControl::newInstance();
		if( False == onlyPanel.bl_isEnlargeDisplayMode ) {
			// 通常表示→拡大表示に切換える場合.
			pNextPopup = &gp_EnlAct_EnlargeModeChangeConfirm;
		} else {
			// 拡大表示→通常表示に切換える場合.
			pNextPopup = &gp_EnlAct_NormalModeChangeConfirm;
		}
		if( True == pPopupListControl->isPopupLayer( pNextPopup )
		    && False == pPopupListControl->isNowPopup( pNextPopup ) ) {
			PBasicMessageProc* pTemp = nextGamenBackup;
			pPopupListControl->closeSpecifiedPopup( pNextPopup );
			nextGamenBackup = pTemp;
		}
// comment 2020/12/18 Sky中野 [IT6_2.1][OP_BTS-29896].
//		pPopupListControl->addPopup( pNextPopup );
		pPopupListControl->addPopupExp( pNextPopup );
	} else {
		switch( comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() ) {
			case TYPE_JobTypeFaxSendQuick:		// ＦＡＸ（クイックメモリ送信）
			case TYPE_JobTypeFaxSendMemory:		// ＦＡＸ（メモリ送信）
			case TYPE_JobTypeFaxSendTimer:		// ＦＡＸ(タイマ送信）
			case TYPE_JobTypeFaxReceivePolling:	// ＦＡＸ（ポーリング送信）
			case TYPE_JobTypeFaxReceiveTimer:	// ＦＡＸ（タイマーポーリング受信）
			case TYPE_JobTypeFaxReceive:		// ＦＡＸ受信プリント
			case TYPE_JobTypeCopy:
			case TYPE_JobTypeScan:
				onlyPanel.bl_isEnlargeDisplayMode = ( onlyPanel.bl_isEnlargeDisplayMode == True ? False : True );	// モードの切り替え.
				PnlComReqAuthPollingRestart();	// ポーリング再開要求.
				// コピージョブ動作中でなければ.
				if( False == PnlComGetJobActionOn( cp_copyJob ) ) {
					if( False == PnlComGetOperationSetting()->getNormalDispSettingSucceed() ) {
						// システムオートリセット解除確認画面での「いいえ」キーもしくは表示時間経過ではモードを初期化する.
						if( onlyPanel.bl_nowEnlargeModeCancelConfirm == True ) {
							c_JobControl.InitJobSetting( cp_copyJob );				// コピージョブを初期化
						} else {
							cancelModeByEnlargeDisplay( cp_copyJob );				// 拡大表示で設定できないモードをＯＦＦする.
						}
					}
					// システムオートリセット解除確認画面での「いいえ」キーもしくは表示時間経過時.
					if( onlyPanel.bl_nowEnlargeModeCancelConfirm == True ) {
						Bool b_Language = PnlComGetOperationSetting()->getLanguageChoiceSetting();
						if( True == b_Language ) {
							// 一時変更言語設定を、元に戻す.
							onlyPanel.e_Language = PnlComGetMachineSetting()->getLanguage();
							PnlComGetOperationSetting()->setSelectLanguage( onlyPanel.e_Language );
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
							onlyPanel.b_LanguageSystemOut = True;
							// 表示言語とキーボード言語が連動している場合は、キーボード言語を元に戻す.
							// ソフトキーボード.
							if( True == PnlComGetMachineSetting()->getSoftwareKeyboardWithPanalLanguage() ) {
// comment 2015/06/29 OPS1-Sky森川(篤) [ZeusS_ZX0][RQ-405] パネルUIのヘブライ語対応(既存問題対応).
//								PnlComGetOperationSetting()->setKeyboardLanguage( onlyPanel.e_Language );
// comment 2018/12/14 Sky毛穴 [EagleHigh][RQ-258] タイ語とベトナム語の対応.
//								if ( isRightDispLanguage(onlyPanel.e_Language) ) {
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
//								PnlComGetMachineSetting()->setExtKeyboardLanguage( onlyPanel.e_Language );
// comment 2018/12/14 Sky毛穴 [EagleHigh][RQ-258] タイ語とベトナム語の対応.
//								if ( isRightDispLanguage(onlyPanel.e_Language) ) {
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
					}
				}
				// スキャンジョブ動作中でなければ.
				if( False == PnlComGetJobActionOn( cp_scanJob ) ) {
					c_JobControl.InitJobSetting( cp_scanJob );				// スキャンジョブを初期化
				}
				// FAXジョブ動作中でなければ.
				if( ( False == PnlComGetJobActionOn( cp_faxJob ) )
				    &&( False == b_isOffHookBtnOn && False == b_isFaxLine1OffHook ) ) {
					// 引継ぐ設定の場合は、同報宛先がクリアされてしまう為、設定をリセットさせない。
					if( False == PnlComGetOperationSetting()->getNormalDispSettingSucceed() ) {
						c_JobControl.InitJobSetting( cp_faxJob );				// ファックスジョブを初期化
					}
				}
				// スキャナファクスジョブ動作中でなければ.
				if( ( False == PnlComGetJobActionOn( cp_scanfaxJob ) )
				    &&( False == b_isOffHookBtnOn && False == b_isFaxLine1OffHook ) ) {
					if( False == PnlComGetOperationSetting()->getNormalDispSettingSucceed() ) {
						c_JobControl.InitJobSetting( cp_scanfaxJob );			// スキャナファックスジョブを初期化
					}
				}
				if( onlyPanel.b_ApplicationMenuMode == False ) {
					switch( comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() ) {
						case TYPE_JobTypeFaxSendQuick:			// ＦＡＸ（クイックメモリ送信）
						case TYPE_JobTypeFaxSendMemory:			// ＦＡＸ（メモリ送信）
						case TYPE_JobTypeFaxSendTimer:			// ＦＡＸ(タイマ送信）
						case TYPE_JobTypeFaxReceivePolling:		// ＦＡＸ（ポーリング送信）
						case TYPE_JobTypeFaxReceiveTimer:		// ＦＡＸ（タイマーポーリング受信）
						case TYPE_JobTypeFaxReceive:			// ＦＡＸ受信プリント
							if( False == b_isOffHookBtnOn && False == b_isFaxLine1OffHook ) {
								if( False == PnlComGetOperationSetting()->getNormalDispSettingSucceed() ) {
									resetParam( option );
								} else {
									nextMainFaxGamen();
								}
							} else {
								nextMainFaxGamen();
							}
							break;
// Comment 2020/02/20 [HCLT_Kavitha] [HeliosMLK] [OP_BTS-20332] Commented job spec reset to apply setting selected in Normal mode to Enlarge Mode and viceversa.
//Comment 2019/05/07 [HCL-Monisha] [OP_BTS-3179] [IT6_EagleH] - Initialising copy jobspec.		
						case TYPE_JobTypeCopy:			
						//	if( False == PnlComGetOperationSetting()->getNormalDispSettingSucceed() ) {
						//			resetParam( option );
						//	}
							break;	// コピージョブ.
						default: resetParam( option );	break;
					}
				}
				// セルフレジストレーション機能がOnの場合で、セルフレジストレーション用ユーザー情報が有効な場合.
				if( ( True == PnlComIsCardSelfRegistrationOn() ) && ( True == c_CardSelfRegistUserInfo.getOn() ) ) {
					PnlComGetAuthDeviceCommon()->AuthUserDataCancel( NULL );	// 登録／変更時のキャンセル通知.
					c_CardSelfRegistUserInfo.initData();	// セルフレジストレーション用ユーザー情報を初期化.
					onlyPanel.bl_UserAccountInit = False;
				}
				if( onlyPanel.bl_isEnlargeDisplayMode == True ) {
					setTurnGamenThis( NULL );		// 前画面に戻らないようにする.
					rebuildGamenEnlarge( 0 );
				} else {
					// 通常表示と拡大表示でモードを継承する仕様変更対応.
					// とりあえずコピージョブのみ継承する対象なので、コピージョブだけ見るようにする.
					if( ( comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() == TYPE_JobTypeCopy ) ||
					    ( onlyPanel.b_ApplicationMenuMode == True ) ) {
						if( False == onlyPanel.isTopmenuMode() ) {
							// 画面遷移先を決定する.
							nextGamenIndirect();
						}
					}
				}
				break;
			default: break;
		}
		// 拡大表示から通常表示に遷移するとき、かつ、システムオートリセット解除確認画面が表示中でないときに基点音を鳴らす.
		if( onlyPanel.bl_isEnlargeDisplayMode == False
		    && onlyPanel.bl_nowEnlargeModeCancelConfirm == False ) {
			setBuzzerType( PCGE_BuzzerType_Base );
		}
	}
}

// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
#define PRECHECKENLARGE_RET( retval , ...)	precheck_returnEnlarge( __LINE__, retval ,## __VA_ARGS__); return;
#define TYPE_PRECHECKENLARGE_OK														(0)
#define TYPE_PRECHECKENLARGE_NG_FlashtoNVRAMPowerDown								(1)
#define TYPE_PRECHECKENLARGE_NG_DeviceLockOn										(2)
#define TYPE_PRECHECKENLARGE_NG_ServiceCallPowerSaveModeON							(3)
#define TYPE_PRECHECKENLARGE_NG_ServiceCallPowerSaveModeOFF							(4)
#define TYPE_PRECHECKENLARGE_NG_PwrStatusSleepOutOn_LogOutputOn						(5)
#define TYPE_PRECHECKENLARGE_NG_PwrStatusSleepOutOn_LogOutputOff					(6)
#define TYPE_PRECHECKENLARGE_NG_PwrStatusSleepOutOff_LogOutputOn_SkipCheckHDDOn		(7)
#define TYPE_PRECHECKENLARGE_NG_PwrStatusSleepOutOff_LogOutputOn_SkipCheckHDDOff	(8)
#define TYPE_PRECHECKENLARGE_NG_PwrStatusSleepOutOff_LogOutputOff_SkipCheckHDDOff	(9)
#define TYPE_PRECHECKENLARGE_NG_IwsAuthDisplay										(10)
#define TYPE_PRECHECKENLARGE_NG_NewScreenLaunching									(11)
#define TYPE_PRECHECKENLARGE_NG_CahngeWarning										(12)
#define TYPE_PRECHECKENLARGE_NG_DisplayJobNewScreenLaunching						(13)
#define TYPE_PRECHECKENLARGE_NG_DisplayJobCahngeWarning								(14)
#define TYPE_PRECHECKENLARGE_NG_NoWarningGamen										(15)
#define TYPE_PRECHECKENLARGE_NG_GamenNowMovement									(16)
#define TYPE_PRECHECKENLARGE_NG_GamenDisableModeSet									(17)
#define TYPE_PRECHECKENLARGE_NG_option1												(18)
#define TYPE_PRECHECKENLARGE_NG_AdminUser											(19)
#define TYPE_PRECHECKENLARGE_NG_BootSelfCheckMode									(20)
#define TYPE_PRECHECKENLARGE_NG_AutoSelfCheck										(21)

static void precheck_returnEnlarge( int32_t line, Bool ret , ... )
{
	int32_t si_cause = 0;
	va_list cp_args;

	va_start(cp_args, ret);
	si_cause = va_arg( cp_args , int32_t );
	va_end(cp_args);

	ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut,"[precheck_returnEnlarge] %lu : %lu : %d : %d : %s"
			, SYSF_tickGet()
			, line
			, si_cause
			, (int)ret
			, (ret == True) ? "OK" : "NG"
			,0
	);
	// 始めの2byte = 行数 / 後の2byte = リターン値 となるようにInt型変数を定義する.
	PNL_LogManager* pc_LogManager = PNL_LogManager::newInstance();
	if( NULL != pc_LogManager ){
		char sc_string[LIBD_BuffLenMax];
		memset( sc_string, 0, sizeof( sc_string ) );
		snprintf( sc_string, LIBD_BuffLenMax, "[precheck_returnEnlarge] %lu: %s", (ulong)si_cause , ( ret == True ? "OK":"NG" ) );
		pc_LogManager->setGeneralPurposeLog( sc_string , (ulong)line );
	}
}

//================================================================================================
//［機能］その他　子クラスから使用する関数 : 状態に応じた画面表示関数.
// [説明] 画面移動が起こるイベントや状態に応じた画面移動の処理があった場合に実行する.
//================================================================================================
// [引数] option	3:前画面登録している画面に戻る　　　isNoWarningGamen が True の画面から通常キー操作で抜けるとき.
//					2:前画面登録している画面に戻らない　isNoWarningGamen が True の画面からパネルリセットキーで抜けるとき.
//					1:前画面登録している画面に戻る（警告／動作中などない場合）.
//					0:前画面登録している画面に戻らない.
//================================================================================================
void PBasicMessageProc::rebuildGamenEnlarge( uchar option )
{
	// タブレット接続／切断時のリセット時は、isNoWarningGamenでも、option(2)が指定された動作とする.
	if( True == onlyPanel.m_bl_procTabletReset ) {
		if( True == isNoWarningGamen ) { option = 2; }
	}

	if( onlyPanel.bl_IsKeyCounterMagCardSet == True ) {		// キーカウンタ／磁気カードセット通知受信状態のとき
		static PCGC_AutoResetTimer	c_autoRestTimer;	// オートリセットタイマー
		// キーカウンタ、磁気カードを引き抜いた後再セットしたときに引き抜く前の画面に戻らないために前画面なし状態にしておく
		onlyPanel.bl_IsKeyCounterMagCardSet = False;		// キーカウンタ／磁気カードセット通知受信状態の解除
		if( ppc_SystemStatus->getServiceModeOn() == True ) {	// サービスモード.
			// 何もしない.
		} else if( c_SystemControl.getAdminModeOn() == True ) {	// 管理者モード.
			// システムオートリセット、システムオートリセットＯＦＦ時の１分タイマーのみ.
			c_autoRestTimer.c_AutoResetAll.resetTimer( True );
			c_autoRestTimer.c_autoResetOffTimer.resetTimer( True );
		} else {	// サービスモード・管理者モード以外.
			c_autoRestTimer.resetAll();
		}
		// ログイン時、(10776000)認証終了確認画面に戻らないために.
		// ユーザ認証ログアウト画面の表示フラグに"非表示"を設定する.
		onlyPanel.ep_UserAccountEnd = EP_UserAccountEndOff;
		turnGamen = NULL;	// 前画面なし
	}
	if( onlyPanel.b_FlashtoNVRAMPowerDown == True ) {
// comment 2021/11/11 Sky東島 [IT6_2.2_MinervaSSBK][OP_BTS-36265].
		if( False == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_EnlWarn_MainPowerOffOnCall ) ){
			// 画面上に主電源OFF/ON画面が表示されてない場合は表示させる.
			PNLC_PopupListControl::newInstance()->addPopup( &gp_EnlWarn_MainPowerOffOnCall, this, NULL );
		}
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//		return;
		PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_FlashtoNVRAMPowerDown );
	}
	// OpenAPI使用中、または他アプリで管理者ログイン中はOpenAPI使用中画面へ遷移.

	TYPE_AdminModeApp e_AdminModeApp = ppc_SystemStatus->getAdminModeOnApp();
	TYPE_AdminLoginMode e_AdminLoginMode = ppc_SystemStatus->getAdminLoginMode();
	if( ppc_SystemStatus->getDeviceLockOn()
// comment 2014/12/16 OPS1-Sky浜野 [Posedion][CQ505311] 3秒ポーリンング処理中は管理者ログイン禁止対応.
	    || ( True == ppc_SystemStatus->getAdminLoginForbidden_forPanel() )
	// comment 2014/06/25 OPS1-Sky北村 [Zeus5BK][制御構成変更No.222].
//		|| (TYPE_AdminModeAppPanel != ppc_SystemStatus->getAdminModeOnApp()
//	  && TYPE_AdminModeAppNon != ppc_SystemStatus->getAdminModeOnApp()
//	  && TYPE_AdminModeAppExternal10 != ppc_SystemStatus->getAdminModeOnApp())) {
	    || ( TYPE_AdminModeAppPanel != e_AdminModeApp
	         && TYPE_AdminModeAppNon != e_AdminModeApp
	         && TYPE_AdminModeAppExternal10 != e_AdminModeApp
	         && TYPE_AdminModeAppPSWC != e_AdminModeApp
	         && TYPE_AdminLoginModePSWC != e_AdminLoginMode ) ) {
		if( TYPE_AdminModeAppExternal9 == ppc_SystemStatus->getAdminModeOnApp() ) {
		} else {
			onlyPanel.resetActivePanelWarning();
			nextGamen = &gp_EnlAct_RemoteUserAccess;
		}
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//		return;
		PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_DeviceLockOn );
	}
// トラブルが発生しているとトラブル画面へ移動.
// comment 2017/04/07 【HCLT-Ashok】 【IT5_4.0】 【AR1017463】When Firmware abnormal fatal occurs blocking other fatal errors, Since Firmware abnormal error has high priority.
// 	if( ( onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCall ) == True )
//    || ( onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCallSMART ) == True ) )  {
// Comment 2020/07/01【HCLT-Selvaraj】【IT6_2.0MR3】【OP_BTS-25295】Service call warning screen displaying after Digital Post screen.
//	if( ( True == onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCallFirmwareAbnormal ) ) &&		// Firmware異常警告発生中.
//	    ( True == PnlComGetSystemSetting()->getSecurityLevelUpOn() ) ) {							// セキュリティ強化ON.
	if( ( ( True == onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCallFirmwareAbnormal ) ) &&                      // Firmware異常警告発生中.
	  ( True == PnlComGetSystemSetting()->getSecurityLevelUpOn() ) ) ||
      ( True == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_DigitalMessageWarning ) ) ) {                                                                                                 // セキュリティ強化ON.
			//	Do nothing.
	} else if( ( onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCall ) == True )
	    || ( onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCallSMART ) == True ) )  {
		// 省電力移行から復帰までの間は警告を検知しても警告画面に遷移しない.
		// 本対応では、省電力復帰時に起動画面から立ち上がらないことがあるため今後技術課題として検討する.
		if( &gp_ActionInitialSetup == nowGamen ) {	//起動画面.
			TYPE_PowerSaveStatus e_PowerSaveStatus = ppc_Power->getPowerSaveStatus();
			if( True == PnlComIsPowerSaveMode( e_PowerSaveStatus ) ) {	// パワーセーブモード時（画面遷移不可の時）.
				nextGamen = NULL;
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//				return;
				PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_ServiceCallPowerSaveModeON );
			}
		}
		PBasicMessageProc* p_popupInstance = NULL;

		// Fatalの移動先画面を決定する
		APIC_RdsSetting* pc_RdsSetting = PnlComGetRdsSetting();
// comment 2020/01/16 【HCL-Prasanjeet】 【OP_BTS-19022】 password chage screen not open when connecting to CSRemote care .
//		if( ( pc_RdsSetting->getHostStat() == TYPE_HostStatRgst )	// センター登録状態(finished)
//		    && ( pc_RdsSetting->getMaintStat() == False )			// メンテ中でない
//		    && ( pc_RdsSetting->getTrblMode() == True ) ) {		// CSRC有効
		ushort us_FatalCode = PnlComGetSystemStatus()->getFatal().us_Code;
		TYPE_FatalLevel engLevel=PnlComGetPrinterWarning()->getFatalLevel();
        TYPE_FatalLevel iRLevel=PnlComGetScannerWarning()->getFatalLevel() ;	
		Bool bl_openAdmPswd = False;
		if ( engLevel == TYPE_FatalLevel_A || engLevel == TYPE_FatalLevel_B || iRLevel == TYPE_FatalLevel_A || iRLevel == TYPE_FatalLevel_B ) {
            // do nothing.
		}else{
              if (TYPD_TroubleCode_EF01 == us_FatalCode ) {
			     bl_openAdmPswd = True;
			  }
		}
		// 管理システム統合化開発 要求仕様2-b 2021/03/03 Sky 大橋 START.
		// CSRAデータ送付設定情報を取得する.
		// CSRA双方向.
		TYPS_CSRASetting s_CsraSetting = {};
		PnlComGetCSRASetting()->getCSRASetting( &s_CsraSetting );
		// CSRA片方向.
		TYPS_CSRASetting s_OneWayCsraSetting = {};
		PnlComGetCSRASetting()->getCSRASetting( &s_OneWayCsraSetting, TYPE_CSRASettingComMode_OneWay );
		// 管理システム統合化開発 要求仕様2-b 2021/03/03 Sky 大橋 END.
		if ( ( ( ( pc_RdsSetting->getHostStat() == TYPE_HostStatRgst )	// センター登録状態(finished).
		// 管理システム統合化開発 要求仕様2-b 2021/03/03 Sky大橋 START.
			||   ( True == s_CsraSetting.m_b_Enable ) 					// CSRA双方向有効.
			||   ( True == s_OneWayCsraSetting.m_b_Enable ) )			// CSRA片方向有効.
		// 管理システム統合化開発 要求仕様2-b 2021/03/03 Sky 大橋 END. 
		    && ( pc_RdsSetting->getMaintStat() == False )			// メンテ中でない.
			&& ( pc_RdsSetting->getTrblMode() == True ) ) 			// CSRC有効.
			&& ( ( bl_openAdmPswd == False ) ) ) {           
			   
				p_popupInstance = &gp_EnlWarn_ServiceCallCreScope; // 拡大表示・サービスコール(接続時).
		} else {
			p_popupInstance = &gp_EnlWarn_ServiceCall; // 拡大表示・サービスコール(スタンドアローン).
		}
		PLayerControl* pc_PopupInstance = dynamic_cast< PLayerControl* >( p_popupInstance );
//	次画面が現在の画面と同じならば、強注意音を鳴動させない.
		if( NULL != pc_PopupInstance && isNowPopup( pc_PopupInstance ) ) {
			setBuzzerType( PCGE_BuzzerType_Error );
		}
		PNLC_EnlWarningManager::newInstance()->openEnlGamenWarning( p_popupInstance );
		onlyPanel.ep_ForceChangeGamen = EP_ForceChangeGamen_Warning;		// 警告強制画面遷移.
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//		return;
		PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_ServiceCallPowerSaveModeOFF );
	}

//[CQ368050]パワーセーブ中は画面遷移させない.2012/08/04.Sky山内.
    APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
    TYPE_PowerSaveStatus e_PowerSaveStatus = ppc_Power->getPowerSaveStatus();
    Bool bPowerSaveStatusPrtEngOnly
         = ((TYPE_PowerSaveStatusPrtEngOnlyLowPowerCancelPanelOn == e_PowerSaveStatus) // ローパワープリントエンジン限定解除パネルON.
         || (TYPE_PowerSaveStatusPrtEngOnlySleepCancelPanelOn == e_PowerSaveStatus))   // スリーププリントエンジン限定解除パネルON.
           ? True : False;

	//comment 2015/05/24 MSE内藤 [MinervaSBK][制御構成変更_No.31] 省電力復帰時間の短縮対応.
	if ( True == onlyPanel.bl_PwrStatusSleepOutFlg ){  // 省電力復帰可能フラグがONの場合には、オブザーバ通知契機の画面遷移であるため、画面遷移可否の判定条件からPrint/Scanの初期化状態を除外する.
		if (  False == PNLC_SnapshotControlLib::newInstance()->getSkipCheckHDD() ) {// HDDチェックのスキップなし.
			if ((True == APIC_SystemSetting::newInstance()->getLogOutputOn())               // ジョブログ設定あり.
			 || (True == APIC_SystemSetting::newInstance()->getImageLogSendEnabled())) {    // 画像ログ設定あり.
				if ( TYPE_HddStatusFormated != pc_HddInfo->getHddStatus()
				  && TYPE_HddStatusNon != pc_HddInfo->getHddStatus()
				  && TYPE_HddStatusFull != pc_HddInfo->getHddStatus()
				  && TYPE_HddStatusConnectEndDocWait != pc_HddInfo->getHddStatus()
				  && TYPE_HddStatusNonFormat != pc_HddInfo->getHddStatus()
				  && TYPE_HddStatusNowUpdatingAddressVer != pc_HddInfo->getHddStatus()
				  && TYPE_HddLockStatus_Locked != pc_HddInfo->getLockStatus()
				  && False == APIC_SystemServiceFunction::newInstance()->hasFormattedHdd()
				  && False == ppc_SystemStatus->isEncryptionKeyNG() ) {
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//					return;
					PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_PwrStatusSleepOutOn_LogOutputOn );
				}
			} else {
				if ( TYPE_HddStatusSleep == pc_HddInfo->getHddStatus()
				  && TYPE_HddLockStatus_Locked != pc_HddInfo->getLockStatus()
				  && False == APIC_SystemServiceFunction::newInstance()->hasFormattedHdd()
				  && False == ppc_SystemStatus->isEncryptionKeyNG() ) {
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//					return;
					PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_PwrStatusSleepOutOn_LogOutputOff );
				}
			}
		}
	} else {	// 省電力復帰可能フラグがOFFの場合には、画面遷移可否の判定条件にPrint/Scan初期化状態を含める.
		if(  True == PNLC_SnapshotControlLib::newInstance()->getSkipCheckHDD() ) {// 電源復帰時.
			if ((False == bPowerSaveStatusPrtEngOnly
			 && True == ppc_ScannerStatus->isInitializing())
			 || True == ppc_PrinterStatus->isInitializing()
			 || False == ppc_SystemStatus->isEncryptionKeyNG()
			 || True == PnlComIsPowerSaveMode(e_PowerSaveStatus)) {
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//			    return;
				PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_PwrStatusSleepOutOff_LogOutputOn_SkipCheckHDDOn );
			}
		}else{
			if ((True == APIC_SystemSetting::newInstance()->getLogOutputOn())               // ジョブログ設定あり.
			 || (True == APIC_SystemSetting::newInstance()->getImageLogSendEnabled())) {    // 画像ログ設定あり.
				if ((False == bPowerSaveStatusPrtEngOnly
				  && True == ppc_ScannerStatus->isInitializing())
				 || True == ppc_PrinterStatus->isInitializing()
				 || (TYPE_HddStatusFormated != pc_HddInfo->getHddStatus()
				  && TYPE_HddStatusNon != pc_HddInfo->getHddStatus()
				  && TYPE_HddStatusFull != pc_HddInfo->getHddStatus()
				  && TYPE_HddStatusConnectEndDocWait != pc_HddInfo->getHddStatus()
				  && TYPE_HddStatusNonFormat != pc_HddInfo->getHddStatus()
				  && TYPE_HddStatusNowUpdatingAddressVer != pc_HddInfo->getHddStatus()
				  && TYPE_HddLockStatus_Locked != pc_HddInfo->getLockStatus()
				  && False == APIC_SystemServiceFunction::newInstance()->hasFormattedHdd()
				  && False == ppc_SystemStatus->isEncryptionKeyNG())
				 || True == PnlComIsPowerSaveMode(e_PowerSaveStatus)) {
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//					return;
					PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_PwrStatusSleepOutOff_LogOutputOn_SkipCheckHDDOff );
				}
			} else {
				if ((False == bPowerSaveStatusPrtEngOnly
				  && True == ppc_ScannerStatus->isInitializing())
				 || True == ppc_PrinterStatus->isInitializing()
				 || (TYPE_HddStatusSleep == pc_HddInfo->getHddStatus()
				  && TYPE_HddLockStatus_Locked != pc_HddInfo->getLockStatus()
				  && False == APIC_SystemServiceFunction::newInstance()->hasFormattedHdd()
				  && False == ppc_SystemStatus->isEncryptionKeyNG())
				 || True == PnlComIsPowerSaveMode(e_PowerSaveStatus)) {
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//					return;
					PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_PwrStatusSleepOutOff_LogOutputOff_SkipCheckHDDOff );
				}
			}
		}
	}

// comment 2016/03/26 OPS1南波 [ZeusS_ZX0][CQ526730]IWS認証起動時にパネル拡大認証に遷移させない.
	if( ( True != onlyPanel.bl_BrowserStartUpFlag ) 
	 && ( True == onlyPanel.checkIwsAuthDisplay() ) 
	 && ( True == cp_IwsSetting->getIwsAuthScreenStartup() ) ) {
		nextGamen = NULL;
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//		return;
		PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_IwsAuthDisplay );
	}

	APIC_InternetISW* pc_InternetISW = APIC_InternetISW::newInstance();
	TYPE_InternetIswResult e_InternetIswResult = pc_InternetISW->getInternetIswResult();
	if( e_InternetIswResult != TYPE_InternetIswResultNotExec ) {			// InternetISW実行時.
		onlyPanel.setPanelWarning( PNL_OTHER_InternetIswResult );			// InternetISW結果表示.
	}

// 2015/03/04 [MAv3][RQ-245]HDDリストア結果画面が表示されない 日立 村上(幸).
	APIC_BackupRestore* pc_BackupRestore = APIC_BackupRestore::newInstance();
	TYPE_BackupRestoreResult e_BackupRestoreResult;
	e_BackupRestoreResult = pc_BackupRestore->getRestoreResult();
	if( TYPE_BackupRestoreResult_None != e_BackupRestoreResult ) {
		onlyPanel.setPanelWarning( PNL_OTHER_DataBackupHDDRestoreResult );			// K470022100_HDDリストア結果表示.
	}

// 全画面警告発生チェックと画面移動（呼んだ関数内で行なう）
	if( isNoWarningGamen == False || option == 2 || option == 3 ) {			// 全画面警告表示しない画面の時は、チェックを飛ばす.
		Bool bl_isNewScreenLaunching = False;
		if( False == onlyPanel.judgePanelJob( onlyPanel.cc_DisplayJob ) ) {
			// cc_DisplayJobとcomPanelJobが同じでない時は、ジョブリスト表示警告をチェック
			if( True == PNLC_EnlWarningManager::newInstance()->changeWarningEnlarge( PNLE_CheckENLWarningType_JobList,
			        onlyPanel.cc_DisplayJob, bl_isNewScreenLaunching ) ) {	// ジョブリスト警告チェック内で画面移動がされた.

// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//				if( False == bl_isNewScreenLaunching ) { return; }
				if( False == bl_isNewScreenLaunching ) {
					PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_NewScreenLaunching );
				}
				onlyPanel.ep_ForceChangeGamen = EP_ForceChangeGamen_Warning;		// 警告強制画面遷移.
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//				return;
				PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_CahngeWarning );
			}
		} else {
			// 上記のどちらでもない時は通常警告をチェック.
			if( True == PNLC_EnlWarningManager::newInstance()->changeWarningEnlarge( PNLE_CheckENLWarningType_Normal,
			        comPanelJob, bl_isNewScreenLaunching ) ) {			// 全画面警告チェック内で画面移動がされた.

// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//				if( False == bl_isNewScreenLaunching ) { return; }
				if( False == bl_isNewScreenLaunching ) {
					PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_DisplayJobNewScreenLaunching );
				}

				onlyPanel.ep_ForceChangeGamen = EP_ForceChangeGamen_Warning;	// 警告強制画面遷移.
				onlyPanel.cc_DisplayJob = comPanelJob;
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//				return;
				PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_DisplayJobCahngeWarning );
			}
		}
		// ジョブリスト警告表示中に全画面警告がなくなった場合は、解除されたので表示をcomPanelJobに戻す.
		onlyPanel.cc_DisplayJob = comPanelJob;							// 全警告表示対象のジョブをcomPanelJobにする.
	} else {
		// 動作中警告＆動作中画面チェックと画面移動（呼んだ関数内で行なう）
		if( isGamenNowMovementEnlarge( True ) == True ) {				// 動作中警告＆動作中画面のチェック（停止中のみ指示）内で画面移動がされた.
			onlyPanel.ep_ForceChangeGamen = EP_ForceChangeGamen_Stop;	// 停止中強制画面遷移.
		}
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//		return;															// 警告表示しない画面の時はここで終わる.
		PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_NoWarningGamen );
	}
// 動作中警告＆動作中画面チェックと画面移動（呼んだ関数内で行なう）
	if( isGamenNowMovementEnlarge( False ) == True ) {	// 動作中警告＆動作中画面のチェック内で画面移動がされた！
		onlyPanel.ep_ForceChangeGamen = EP_ForceChangeGamen_Action;			// 動作中強制画面遷移
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//		return;
		PRECHECKENLARGE_RET( False ,TYPE_PRECHECKENLARGE_NG_GamenNowMovement  );
	}
// モード設定禁止画面チェックと画面移動（呼んだ関数内で行なう）
	if( isGamenDisableModeSetEnlarge() == True ) {								// モード設定禁止画面チェック内で画面移動された!
		if( True == PLBC_FuncPKICheck::isPKI() ) {
			if( ( PCGE_CardAccsessUserAcountNow == PCGC_PkiCardManager_PKI::getInstance()->getPkiCardAccsessStatus() )
			    || ( PCGE_CardAccsessTouchAndoPrintoNowOnly == PCGC_PkiCardManager_PKI::getInstance()->getPkiCardAccsessStatus() ) ) {
				nextGamen = &gp_EnlAct_UserAccountUserNow;
			} else if( PCGE_CardAccsessPincodeNow == PCGC_PkiCardManager_PKI::getInstance()->getPkiCardAccsessStatus() ) {
				nextGamen = &gp_EnlAct_PincodeCheckNow_PKI;
			} else if( PCGE_CardAccsessNon == PCGC_PkiCardManager_PKI::getInstance()->getPkiCardAccsessStatus() ) {
				APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();					// HDの情報を取得.
				Bool b_AccessHD = False;
				if( NULL != pc_HddInfo ) {										// 取得不可能なら処理を中断.
					TYPE_HddStatus e_HddStatus = pc_HddInfo->getHddStatus();
					switch( e_HddStatus ) {
						case TYPE_HddStatusConnectEndDocWait:					// 接続中か判断.
						case TYPE_HddStatusFormated:							// HDがフォーマット済みか判断.
						case TYPE_HddStatusConnecting:							// HDDの初期化完了からDOCの初期完了までか判断.
						{
							// ﾌｫｰﾏｯﾄ済でない場合.
							APIC_User* pc_User = APIC_User::newInstance( TYPD_IndexUserAdmin );	// 管理者ユーザー情報を取得.
							b_AccessHD = ( NULL != pc_User ) ? pc_User->isAccessSettingData() : False;
						}
						break;
						default:
							break;
					}
				}
				// PKIログインモード中かつPKIカード挿入中かつ外部サーバー登録中かつ外部サーバー準備中でない遷移先がPKIユーザー認証画面の場合.
				APID_AuthenticationServerIndex sc_AuthenticationServerIndex = APID_AuthenticationServerIndexNone;
				APIC_AuthenticationSeverSetting* pc_AuthenticationSeverSetting = APIC_AuthenticationSeverSetting::newInstance();
				if( NULL != pc_AuthenticationSeverSetting ) {
					if( False == pc_AuthenticationSeverSetting->checkServerGroup() ) {
						// デフォルトサーバーのインデックスを取得.
						sc_AuthenticationServerIndex = pc_AuthenticationSeverSetting->getDefaultServerIndex();
					} else {
						// デフォルトサーバーグループのインデックスを取得.
						sc_AuthenticationServerIndex = static_cast<APID_AuthenticationServerIndex>( pc_AuthenticationSeverSetting->getDefaultServerGroupIndex(PNLC_NetworkLan::getInstance()->getLine()) );
					}
				}
				if( ( True == b_AccessHD )		// HDがアクセス可能か判断.
				    &&	( True == PCGC_PkiCardManager_PKI::getInstance()->getPkiCardStatus() )
				    && ( False == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_ExternalAuthenticationNotReady ) )
				    && ( TYPE_ChannelStatusError != APIC_NetworkDevice::newInstance()->getChannelStatus( TYPE_ChannelTypeNetwork ) )
				    && ( False == isContactlessPKICard() )													// 非接触PKIカードではない comment 2016/11/11 Sky東島 [特ROM][AU DoD].
				    && ( ( nextGamen == &gp_EnlAct_UserAccountUser ) || ( nextGamen == &gp_EnlAct_UserAccountUser_SeqCustom ) )
				    && ( APID_AuthenticationServerIndexNone != sc_AuthenticationServerIndex ) ) {
					// PKIカードログインの為のPINコード入力.
					PCGC_PkiCardManager_PKI::getInstance()->setPincodeInputKind( DEF_INPUTPINCODE_PRINTANDLOGIN );
					nextGamen = &gp_EnlAct_PincodeInput_PKI;
				}
			}
		}
		register short ss_TableNo = 0;					// 0 : モード設定可.
		ss_TableNo = onlyPanel.isDisableModeSet();		//モード設定禁止画面チェック.
		if( ( NULL == nextGamen ) && ( PNL_OTHER_ScanFaxSendSettingContinueCheck == sp_GamenEnlDisableModeSetTable[ss_TableNo].ec_WarningName ) ) {
			PNLC_EnlWarningManager::newInstance()->changeDisableModeWarning( ss_TableNo );
		}
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//		return;
		PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_GamenDisableModeSet );
	}
// comment 2015/11/04 MSE田中優介 [ZeusS_ZX0][CQ520656] 拡大表示画面のメッセージ警告の表示タイミングを通常表示画面にあわせるため削除. 
	// メッセージ警告の表示タイミングを変更. メッセージ警告の検知を画面更新処理に移動する.
//	if( &gp_EnlAct_HomeTopMenu != turnGamen ) {
//// 画面移動つきメッセージ警告のチェックと画面移動（呼んだ関数内で行なう）.
//		if( changeMessageWarningAndGamenMoveEnlarge() == True && ppc_SystemStatus->getServiceModeOn() == False ) { return; }	// 画面移動つきメッセージ警告のチェック内で画面移動がされた！.
//	}
// comment 2015/02/10 OPS1-Sky浜野 [Poseidon][CQ509765] 警告画面からの戻り先不正対応.
//	// comment 2014/12/10 【HCLT-Deeparaj】 【CQ-507699】 close the  gamen warnig screen,if warning screen top of the panel.
//	if((1 == option ) &&(True == PNLC_EnlWarningManager::newInstance()->closeGamenWarning())){
//		return;
//	}
// optionで前画面遷移要求がある時は、前画面へ戻る.
	if( option == 1 && turnGamen != NULL )	{
		// 検索処理中は前画面に戻らないという条件文を追加.
		if( pc_ManageIGCWaitDisplay.getDisplayFLG() != True  ||
		    True == PnlComGetSystemStatus()->getUserChoiceChanging() ) {
			nextGamen = turnGamen;		// 前画面に戻る.
		}
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//		return;
		PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_option1 );
	}
	// ユーザ認証中に管理者Loginした場合はジョブ確認画面へ遷移する.
	// PnlComGetUser()　NULLチェック対応.
	if( NULL != PnlComGetUser() ) {
		if( PnlComGetUserAccountOn() && TYPE_UserTypeAdmin == PnlComGetUser()->getUserType() ) {
			// ジョブ確認画面に遷移するため、拡大表示解除.
			onlyPanel.bl_isEnlargeDisplayMode = False;			// モードの切り替え.
			reqLED( PNLE_LED_EnlargeDisp, PNLE_LEDStateOff );	// 拡大表示LEDの消灯.
			procJobControl( 10 );
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//			return;
			PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_AdminUser );
		}
	}
	//	統合管理装置装着時の管理しないアプリの場合Publicにて認証する.
	if( ( EC_IntMode_Off == onlyPanel.ec_IntMode )												&&
	    ( TYPE_KeyModeAdminMachineKM == APIC_KeyDevice::getKeyMode() )							&& 	//	統合管理装置装着時.
	    ( TYPE_UserAuthenticationDevice == ppc_UserAuthentication->getUserAuthentication(PNLC_NetworkLan::getInstance()->getLine()) )	&&
	    ( NULL == PnlComGetUser() ) ) {
		//	選択したアプリが「管理しない」指定になっているかチェック.
		Bool _bl_WithoutCardOrCoin = False;
		if( ( True == onlyPanel.b_nowBoxMode ) || ( True == onlyPanel.getBrowserMode() ) ) {
			//	Boxアプリは、PCプリント仕様禁止の場合もPublcで認証する.
			if( ( TYPE_AppAtAdminMachineVendorEnableWithoutCardOrCoin == PnlComGetSystemSetting()->getPrintAtAdminMachineVendor() )	||
			    ( TYPE_AppAtAdminMachineVendorDisable == PnlComGetSystemSetting()->getPrintAtAdminMachineVendor() ) ) {	//	管理しない.
				_bl_WithoutCardOrCoin = True;
			}
		} else if( True ==  onlyPanel.isTopmenuMode() ) {
			if( TYPD_NotSetting == APIC_KeyDevice::getCardID() ) {				// 磁気カード未挿入.
				_bl_WithoutCardOrCoin = True;
			}
		} else {
			TYPE_JobType e_JobType = comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType();
			switch( e_JobType ) {
				case TYPE_JobTypeScan:		//	ジョブタイプ（アプリ設定）がスキャナの時は、スキャンの基本画面.
					if( TYPE_AppAtAdminMachineVendorEnableWithoutCardOrCoin == PnlComGetSystemSetting()->getSendAtAdminMachineVendor() ) {	//	管理しない.
						_bl_WithoutCardOrCoin = True;
					}
					break;
				case TYPE_JobTypeFaxSendQuick:		//	ＦＡＸ（クイックメモリ送信）.
				case TYPE_JobTypeFaxSendMemory:		//	ＦＡＸ（メモリ送信）.
				case TYPE_JobTypeFaxSendTimer:		//	ＦＡＸ(タイマ送信）.
				case TYPE_JobTypeFaxReceivePolling:	//	ＦＡＸ（ポーリング送信）.
				case TYPE_JobTypeFaxReceiveTimer:	//	ＦＡＸ（タイマーポーリング受信）.
				case TYPE_JobTypeFaxReceive:		//	ＦＡＸ受信プリント.
					if( TYPE_AppAtAdminMachineVendorEnableWithoutCardOrCoin == PnlComGetSystemSetting()->getSendAtAdminMachineVendor() ) {	//	管理しない.
						_bl_WithoutCardOrCoin = True;
					}
					break;
				default:	//	上記以外はすべて、コピー基本画面とする.
					if( TYPE_AppAtAdminMachineVendorEnableWithoutCardOrCoin == PnlComGetSystemSetting()->getCopyAtAdminMachineVendor() ) {	//	管理しない.
						_bl_WithoutCardOrCoin = True;
					}
					break;
			}
		}
		if( True == _bl_WithoutCardOrCoin ) {
			//	管理しないアプリの基本画面に遷移する場合、Publicでログインする.
			PnlComPublicUserLogin();
		}
	}
// comment 2019/01/30 【IT6_EagleH】【HCLT_Pavani】【制御構成変更_No.61】Disabling the Full-Diagnosis Screen Access through Boot Check Mode.
//	if( True == PLBC_FuncVersionCheck::isOK_TypeMLK() ) {
// comment 2019/03/21 HCL-Prabakaran [IT6_2.0]OP_BTS-4308 fix for full diagnosis sub-power key mode. 
//	if( False == PLBC_FuncVersionCheck::isOK_IT6_Type1() ) {
	Bool bl_isBootSelfCheckMode = APIC_SystemInfo::newInstance()->isBootSelfCheckMode();
// comment 2016/09/22 HCLT-Omkar 【IT5_4.0】 【制御構成変更_No.45】 Replacing the 82B92710 screen instance with M0B9250000 screen instance.
	if( True == bl_isBootSelfCheckMode ) {
//			nextGamen = &gp_ServiceFullCheck;			// 82B92710_マシン状態／確認／自己診断／フルチェック.
		nextGamen = &gp_ServiceFullDiagnosis;		// M0B9250000_ﾏｼﾝ状態／確認／自己診断／フルチェック.
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//		return;
		PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_BootSelfCheckMode );
	}
// comment 2019/03/21 HCL-Prabakaran [IT6_2.0]OP_BTS-4308 fix for full diagnosis sub-power key mode.
//	}
// comment 2016/09/21 HCL-Vasu [IT5_4.0][制御構成変更_No.51] Launch Auto Self Diagnosis Screen.
	if ( True == ppc_SystemStatus->getAutoSelfCheckOn() ) { 	// if  DIPSW 163 bit 1 is ON, Rank B/C Auto Error Cancel Enabled, if Trouble Count reaches more than 3.
// comment 2020/02/07 SCD2-Sky 延原 [IT6_2.0_MR][TAS000000002112] G00-73適用後、自動的に自己診断画面に遷移する.
//// comment 2019/01/30 【IT6_EagleH】【HCLT_Pavani】【制御構成変更_No.61】Disabling the Full-Diagnosis Screen Access through Boot Check Mode.
//		if( True == PLBC_FuncVersionCheck::isOK_IT6_Type1() ) {
// comment 2019/7/8 K.Sato [EagleH][OPT_BTS-4880] 診断画面に「C-0000」が表示される問題の対応.
//// comment 2019/05/17 【IT6_EagleH】【HCLT_Sarada】【OP_BTS-4880】If trouble code exists,launch the AutoFullDiagnosis screen.
// 			if( PnlComGetSystemStatus()->getFatal().us_Code != 0
// 				|| PnlComGetPrinterWarning()->getFatal().us_Code != 0
// 				|| PnlComGetScannerWarning()->getFatal().us_Code != 0 ) {
////				nextGamen = &gp_ServiceAutoFullDiagnosis;
//// comment 2019/06/03 【IT6_EagleH】【HCLT_Ranjith】【OP_BTS-6373】New Layout display and gamen type changes.
////				PNLC_PopupListControl::newInstance()->addPopup( &gp_ActFullDiagnosis );
//				nextGamen = &gp_ActFullDiagnosis;
// 			}
			nextGamen = &gp_ActFullDiagnosis;
// comment 2020/02/07 SCD2-Sky 延原 [IT6_2.0_MR][TAS000000002112] G00-73適用後、自動的に自己診断画面に遷移する.
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//			return;
			PRECHECKENLARGE_RET( False , TYPE_PRECHECKENLARGE_NG_AutoSelfCheck );
//		} else {
//			nextGamen = &gp_ServiceFullDiagnosis;					// Launch M0B9250000.
//		}
	}
	static Bool bl_isFirst = False;
	if( True != bl_isFirst ) {
		PnlComQtSetDoubleTapInterval( getDoubleClickInterval() );
		bl_isFirst = True;
	}

// comment : WebUI2 画面切替え制御 -> 20180302 Chikuan.
	if( ( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() )
// comment 2022/04/28 Sky中野 [IT6_2.3] 制御No.43_システムオートリセット改善(WebUIv2/v3統合).
//	 && ( PNLE_WebUIStartStatusStarted == PNLC_WebUIControl::getInstance()->getWebUIStartStatus() )
	 && ( PNLE_BrowserChangeTabControlTabInitStatus_Done == PNLC_BrowserChangeTabControl::getInstance()->getTabInitStatus() )
	 && ( PNLE_WebUIScreenChangeStatusStart == PNLC_WebUIControl::getInstance()->getWebUIScreenChangeStatus() ) ){
		nextGamen = &gp_AppChangeUIBrowser;
		// Comment : 2018/07/09 Chikuan [Eagle][Accurev:1051267] WebUIV2の画面に遷移する際、ブラウザーモードをOnにするよう修正.
		onlyPanel.setBrowserMode( True );
	// アプリケーションメニュー表示中の場合はアプリケーションメニュー（拡大）に遷移させる.
	}else if( onlyPanel.b_ApplicationMenuMode == True ) {
// comment : WebUI2 画面切替え制御 <- 20180302 Chikuan.
		if( True == PNLC_SnapshotControlLib::newInstance()->getSkipCheckHDD() )
		{
			// 電源復帰時のHDDチェックスキップ中の時はコピー画面へ.
			nextGamen = &gp_EnlCopy_Basic;
		} else {
			nextGamen = &gp_ApplicationMenuEnl;
		}
	} else if( ( True == onlyPanel.isTopmenuMode() )
	           || ( True == onlyPanel.b_nowBoxMode )
	           || ( True == onlyPanel.getBrowserMode() ) ) {
// comment 2016/09/16 MSE岡田 [IT5_3.3][1005694] 省電力復帰中のHDDチェックスキップ時はトップメニューに遷移する.
//		if( True == PNLC_SnapshotControlLib::newInstance()->getSkipCheckHDD() )
//		{
//			// 電源復帰時のHDDチェックスキップ中の時はコピー画面へ.
		if( True == PNLC_SnapshotControlLib::newInstance()->getSkipCheckHDD() ) {
			// 電源復帰時のHDDチェックスキップ中.
			if( ( True == onlyPanel.isTopmenuMode() ) && ( True == onlyPanel.bl_PwrStatusSleepOutFlg ) ) {
				// トップメニューモード、かつ省電力復帰可能フラグがONの時はトップメニュー画面へ.
				nextGamen = &gp_EnlAct_HomeTopMenu;
			} else {
				// 上記条件以外の時はコピー画面へ.
				nextGamen = &gp_EnlCopy_Basic;
			}
		} else {
			nextGamen = &gp_EnlAct_HomeTopMenu;
		}
		// 拡大表示に対応していない機能（ブラウザ、ボックス、ユビキタス）から.
		// 通常トップメニュー経由で拡大トップメニューへ遷移し、拡大トップメニューを閉じる操作を行った場合の仕様は.
		// 画面仕様に明記されていなかったが、画面仕様チームより以下動作が仕様と回答あり.
		// 拡大トップメニューにて.
		//   ・メニューキー押下(拡大トップメニューを閉じる動作) → 受付禁止.
		//   ・カウンター等の画面へ遷移し、その先の画面を閉じた場合 → 拡大トップメニューへ遷移.
	} else if( True == onlyPanel.isUbqPrintMode() ) {
		// 拡大非対応の為、拡大トップメニューへ遷移.
		if( True == PNLC_SnapshotControlLib::newInstance()->getSkipCheckHDD() )
		{
			// 電源復帰時のHDDチェックスキップ中の時はコピー画面へ.
			nextGamen = &gp_EnlCopy_Basic;
		} else {
			nextGamen = &gp_EnlAct_HomeTopMenu;
		}
	} else {
		switch( comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() ) {
			case TYPE_JobTypeCopy:
				if( ( EP_PNS_Setting        == onlyPanel.ep_PanelStatus ) &&								// パネルの状態がEP_PNS_Setting.
				    ( True                  == onlyPanel.isWarningOn( comPanelJob, PERROR_CoinUnset ) ) &&	// コイン未投入警告発生.
				    ( TYPE_KeyModeVendorKM  == APIC_KeyDevice::getKeyMode() ) ) {							// ベンダー2接続環境.
					// コイン未投入時はコピー基本画面に遷移させない.
				} else {
					nextGamen = &gp_EnlCopy_Basic;		// 拡大表示(コピー)基本画面.
				}
				break;
			case TYPE_JobTypeScan:
				if( APIC_KeyDevice::getKeyMode() == TYPE_KeyModePageAcses ) {
					nextGamen = &gp_ScanPageAcsesEnl;	// 拡大表示スキャナ・ページアクセス宛先設定 (4H0001).
				} else {
					nextGamen = &gp_ScanProgramEnl;	// 拡大表示(スキャナ)プログラム.
				}
				break;
			case TYPE_JobTypeFaxSendQuick:		// ＦＡＸ（クイックメモリ送信）
			case TYPE_JobTypeFaxSendMemory:		// ＦＡＸ（メモリ送信）
			case TYPE_JobTypeFaxSendTimer:		// ＦＡＸ(タイマ送信）
			case TYPE_JobTypeFaxReceivePolling:	// ＦＡＸ（ポーリング送信）
			case TYPE_JobTypeFaxReceiveTimer:	// ＦＡＸ（タイマーポーリング受信）
			case TYPE_JobTypeFaxReceive:		// ＦＡＸ受信プリント
			{
				if( PNLC_PopupListControl::newInstance()->getNowPopup() != &gp_HomeTopMenu ) {
// comment 2015/10/13 OPS1-Sky東島 [MiSBK][CQ514960].
					// FAXアプリ時、遷移できない条件の場合はファクス/スキャンモードに切り替える.
					if( comPanelJob == cp_faxJob ){
						if( True == onlyPanel.isFaxServerEnable()
						 || False == onlyPanel.isEnableSetFaxSend( cp_faxJob , PNELD_MSK_ISFAXMODE_FAX1 )
						 || False == PnlComGetScanJobSetting()->getEnableFaxSend() ){
							c_JobControl.changePanelAPP( TYPE_JobTypeFaxSendMemory );   // 操作パネル上のアプリケーション表示を切り換える。
						}
					}
					nextMainFaxGamen();
				} else {
					// トップメニューからの遷移の場合、宛先検索を行わず拡大トップメニューに遷移する.
					nextGamen = &gp_EnlAct_HomeTopMenu;
				}
			}
			break;
			default: break;
		}
	}
	if( ppc_SystemStatus->getUserChoiceChanging() == True ) {
		// ユーザチョイス変更設定中がONなら、OFFにする
		ppc_SystemStatus->setUserChoiceChanging( False );
	}
	PnlComGetSystemStatus()->setEditUserIDClear();
	// ユーザ認証設定無し時.
// comment Sky東島 [RQ-686] 複合機の複数ネットワーク対応.
//	if( TYPE_UserAuthenticationOff == ppc_UserAuthentication->getUserAuthentication() ) {
// comment 2019/05/28 [HCL-Naveena][EagleBK] NetworkType Default selection activity.
//	if( ( TYPE_NetworkSpecific_NONE != PNLC_NetworkLan::getInstance()->getLine() )
	if( ( TYPE_NetworkSpecific_NONE != PNLC_NetworkLan::getInstance()->getLineForPanel() )
	 && ( TYPE_UserAuthenticationOff == ppc_UserAuthentication->getUserAuthentication(PNLC_NetworkLan::getInstance()->getLine()) ) ){
		// PublicユーザにLoginしていなければいけないのでユーザポインタがNULLであれば.
		if( NULL == PnlComGetUser() ) {
			// Publicでログインする.
			TYPE_UserResult e_UserResult = TYPE_UserResultPasswordNG;
			PanelLog( PNLD_LogKind_Auth, "req_login() P1[PUBLIC] P2[] P3[%d] P4[NULL] P5[%d] P6[%d] P7[%d]", TYPD_UserFunctionMask_None, e_UserResult, bl_isExtAppOperationGamen, JLGE_InspectLogIfOperationPanel, 0, 0 );
// comment Sky東島 [RQ-686] 複合機の複数ネットワーク対応.
//			PnlComSetUser( APIC_User::req_login( "PUBLIC", "\0", TYPD_UserFunctionMask_None, NULL, &e_UserResult, bl_isExtAppOperationGamen, JLGE_InspectLogIfOperationPanel ) );
			PanelLog( PNLD_LogKind_Auth, "req_login() P8[%d] P9[%p] P10[%d]", False, NULL, PNLC_NetworkLan::getInstance()->getLine() , 0 , 0 , 0 );
			PnlComSetUser( APIC_User::req_login( "PUBLIC", "\0", TYPD_UserFunctionMask_None, NULL, &e_UserResult, bl_isExtAppOperationGamen, JLGE_InspectLogIfOperationPanel, False, NULL, PNLC_NetworkLan::getInstance()->getLine() ) );
		}
	}
// comment 2017/02/01 Sky東島 [IT5-67] LDAPアプライアンスと生体認証の組み合わせでのセルフレジストの対応.
	if( ( True == c_BIOSelfRegistUserInfo.getLoginFlg() )
	 && ( True == PnlComIsBIOSelfRegistrationOn() ) 
	 && ( NULL != PnlComGetUser() )
	 && ( TYPE_UserTypeUser == PnlComGetUser()->getUserType() ) ){
		c_BIOSelfRegistUserInfo.setEntryBodyRegist( True );
		if( True == PNLC_BrowserChangeTabControl::getInstance()->launchAdminTab() ){
			PNLC_WebUIControl_v1::getInstance()->reserveChangeScreen( PNLE_WebUIv1RequestMessageUtilityBIODeveiceSet );	// WebUIv1への遷移予約（生体登録）.
		}else{
			gp_EnlAct_V1TabWait.setWaitTabInitFlag();
		}
		nextGamen = &gp_EnlAct_V1TabWait;
	}
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
	PRECHECKENLARGE_RET( True , TYPE_PRECHECKENLARGE_OK );
}
//概要    : スキャンファクスLDAP検索_サーバー選択 画面切替振り分け処理.
//備考    :
PBasicMessageProc* PBasicMessageProc::dispatchGamenScanFaxLDAPAuthenticationSelectHost()
{
	// 拡大表示の場合は、拡大表示用の画面に遷移する
	if( onlyPanel.bl_isEnlargeDisplayMode == True ) {
		return &gp_EnlScanFax_LDAPAuthenticationSelectHost;	// 10H00760   スキャンファクスLDAP検索_サーバー選択.
	} else {
		return &gp_AddressSearch_scanfax;
	}
}
//概要    :常用登録宛先 画面切替振り分け処理.
//備考    :
PBasicMessageProc* PBasicMessageProc::dispatchGamenScanFaxFromRegAddress()
{
// comment 2021/04/23 2DWS2G-Sky_肥後 クラウドSSO認証開発v1_Step2 [OP_BTS-32062].	
//	// comment 2021/02/25 2DWS2G-Sky_Nozaki クラウドSSO認証開発v1_Step2 [OP_BTS-30274].
//	if( True == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_IdPMaxCounterInfoSaveAreaMessage ) ) {
//		return NULL;
//	}

	// 拡大表示の場合は、拡大表示用の画面に遷移する
	if( onlyPanel.bl_isEnlargeDisplayMode == True ) {
		return &gp_PD_10H00400;						// 10H00400   常用登録宛先.
	} else {
		// 登録宛先から使用可否判定.
		if( True == PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_AddressBook ) ) {	// パターンカスタマイズによる表示判定.		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			return &gp_FromRegAddress_scanfax;
		}
		// 直接入力使用可否判定.
		if( False == isDisableAddressManualInput() ) {
			return &gp_DirectInput_scanfax;
		}
		// 履歴から選択使用可否判定.
		if( True == onlyPanel.isFaxFunctionUseCheck( PDPE_FaxFunctionCheckReDial ) ) {
			return &gp_ScanFaxHistoryAddress;
		}
		// 宛先検索使用可否判定.
		if( True == PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_AddressSearch ) ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			return &gp_AddressSearch_scanfax;
		}

		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	// すべての機能が使用できない場合は遷移先取得前に防ぐためあり得ない.
		return NULL;	// スキャンファクス画面遷移不可.
	}
}
//概要    :履歴から_送信履歴 画面切替振り分け処理.
//備考    :
PBasicMessageProc* PBasicMessageProc::dispatchGamenScanFaxFromTxLog()
{
	// 拡大表示の場合は、拡大表示用の画面に遷移する
	if( onlyPanel.bl_isEnlargeDisplayMode == True ) {
		return &gp_PD_10H00480;							// 10H00480   履歴から_送信履歴.
	} else {
		return &gp_ScanFaxHistoryAddress;						// A033000000 履歴から選択.
	}
}
//概要    :オフフック_常用登録宛先 画面切替振り分け処理.
//備考    :
PBasicMessageProc* PBasicMessageProc::dispatchGamenScanFaxCommonUseBasicOffHook()
{
	// 拡大表示の場合は、拡大表示用の画面に遷移する
	if( onlyPanel.bl_isEnlargeDisplayMode == True ) {
		return &gp_EnlScanFax_CommonUseBasicOffHook;							// 10H06010   拡大_オフフック_常用登録宛先.
	} else {
		if( True == PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_AddressBook ) ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			return &gp_EntryAddressOffHook_scanfax;								// A036000100 オフフック_登録宛先から.
		}
		if( True == onlyPanel.isFaxFunctionUseCheck( PDPE_FaxFunctionCheckReDial ) ) {
			return &gp_RedialSelectOffHook_scanfax;								// オフフック履歴から選択.
		}

		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	// すべての機能が使用できない場合は遷移先取得前に防ぐためあり得ない.
		return NULL;	// スキャンファクス画面遷移不可.
	}
}
//概要    :オフフック_直接入力 画面切替振り分け処理.
//備考    :
PBasicMessageProc* PBasicMessageProc::dispatchGamenScanFaxDirectFaxOffHook()
{
	// 拡大表示の場合は、拡大表示用の画面に遷移する
	if( onlyPanel.bl_isEnlargeDisplayMode == True ) {
		return &gp_EnlScanFax_DirectFaxOffHook;							// 暫定
	} else {
		return &gp_DirectInputOffHook_scanfax;	// A036000000   オフフック_直接入力
	}
}
//概要    :FAXモードにおける初期表示画面設定.
//備考    :
void PBasicMessageProc::nextMainFaxGamen()
{
// comment 2020/12/10 Sky延原 ログ強化改善.
	PM_TaskID priTask;
	PMGC_PanelManager::getPriority( &priTask );
	
	onlyPanel.endNetworkScan();		// ネットワークスキャン終了関数コール.
	if( True == onlyPanel.isTopmenuMode() ) {
// comment 2020/12/10 Sky延原 ログ強化改善.
		ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] return", priTask, 0, 0, 0, 0, 0 );
		return;
	}
	TYPE_UserResult e_UserResult = TYPE_UserResultPasswordNG;
	// ユーザ認証しない時.
	// 尚且つ、デフォルトのPublicユーザーが設定されていない時.
// comment Sky東島 [RQ-686] 複合機の複数ネットワーク対応.
//	if( TYPE_UserAuthenticationOff == ppc_UserAuthentication->getUserAuthentication()
//	    && PnlComGetUser() == NULL )
// comment 2019/05/28 [HCL-Naveena][EagleBK] NetworkType Default selection activity.
//	if( ( TYPE_NetworkSpecific_NONE != PNLC_NetworkLan::getInstance()->getLine() )		// ネットワーク未選択ではないとき .
// comment 2020/12/10 Sky延原 ログ強化改善.
	ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] PNLC_NetworkLan::getInstance()->getLineForPanel()[%d]", priTask, PNLC_NetworkLan::getInstance()->getLineForPanel(), 0, 0, 0, 0 );
	if ( NULL != ppc_UserAuthentication ) {
		ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] ppc_UserAuthentication->getUserAuthentication( PNLC_NetworkLan::getInstance()->getLine()[%d] )[%d]", priTask, PNLC_NetworkLan::getInstance()->getLine(), ppc_UserAuthentication->getUserAuthentication( PNLC_NetworkLan::getInstance()->getLine() ), 0, 0, 0 );
	}
	ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] PnlComGetUser()[%p]", priTask, PnlComGetUser(), 0, 0, 0, 0 );
	if( ( TYPE_NetworkSpecific_NONE != PNLC_NetworkLan::getInstance()->getLineForPanel() )		// ネットワーク未選択ではないとき . 
	 && ( TYPE_UserAuthenticationOff == ppc_UserAuthentication->getUserAuthentication( PNLC_NetworkLan::getInstance()->getLine() ) )
	 && ( PnlComGetUser() == NULL ) )
	{
		PanelLog( PNLD_LogKind_Auth, "req_login() P1[PUBLIC] P2[] P3[%d] P4[NULL] P5[%d] P6[%d] P7[%d]", TYPD_UserFunctionMask_None, e_UserResult, False, JLGE_InspectLogIfOperationPanel, 0, 0 );
// comment Sky東島 [RQ-686] 複合機の複数ネットワーク対応.
//		PnlComSetUser( APIC_User::req_login( "PUBLIC", "\0", TYPD_UserFunctionMask_None, NULL, &e_UserResult, False, JLGE_InspectLogIfOperationPanel ) );
		PanelLog( PNLD_LogKind_Auth, "req_login() P8[%d] P9[%p] P10[%d]", False, NULL, PNLC_NetworkLan::getInstance()->getLine() , 0 , 0 , 0 );
		PnlComSetUser( APIC_User::req_login( "PUBLIC", "\0", TYPD_UserFunctionMask_None, NULL, &e_UserResult, False, JLGE_InspectLogIfOperationPanel, False, NULL, PNLC_NetworkLan::getInstance()->getLine() ) );
		if( PnlComGetUser() == NULL )
		{
			SYSF_logMsg( "【PBasicMessageProc::nextMainFaxGamen()】**** ------- public login 失敗!\n",1,2,3,4,5,6 );
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		}
	}

	Bool b_IsClearDisp = True;	// 透明Wait画面を出す.
	// 透明Wait画面表示判定用フラグがイメージディーラーの場合.
// comment 2020/12/10 Sky延原 ログ強化改善.
	ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] getClearDispFlg()[%d]", priTask, pc_ManageIGCWaitDisplay.getClearDispFlg(), 0, 0, 0, 0 );
	if( pc_ManageIGCWaitDisplay.getClearDispFlg() == PCGE_IGCClearDispFlgIdealer ||
	    pc_ManageIGCWaitDisplay.getClearDispFlg() == PCGE_IGCClearDispFlgBrower ||
	    pc_ManageIGCWaitDisplay.getClearDispFlg() == PCGE_IGCClearDispFlgScreenSaver )
	{
		b_IsClearDisp = False;	// 透明Wait画面を出さない.
	}
	TYPE_KeyMode e_KeyMode = APIC_KeyDevice::getKeyMode();				//	キーモード.
	Kbd_ScanFaxKeybordConv.setChangeEnlKeybord( PLBD_Enl_KBD_Initial );
// comment 2020/12/10 Sky延原 ログ強化改善.
	ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen[%p]", priTask, nextGamen, 0, 0, 0, 0 );
	if( ( ScanOapExtSet.isActiveGeneralApplication() == True ) ) {
// comment 2020/12/10 Sky延原 ログ強化改善.
		ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] AplCommonSet.getActiveAplType()[%d]", priTask, AplCommonSet.getActiveAplType(), 0, 0, 0, 0 );
		if( ( AplCommonSet.getActiveAplType() == PNLE_ActiveAplTypeExternal ) ) {
			PCGC_ScanOapBasic::b_Continue = True;
			nextGamen = ScanOapExtSet.getAppTopGamen();
// comment 2020/12/10 Sky延原 ログ強化改善.
			ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>[%p]", priTask, nextGamen, 0, 0, 0, 0 );
			PPopupCom* next_popup = dynamic_cast<PPopupCom*>( nextGamen );
// comment 2020/12/10 Sky延原 ログ強化改善.
			ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] next_popup[%p]", priTask, next_popup, 0, 0, 0, 0 );
			// ポップアップ画面の場合.
			if( NULL != next_popup ) {
				// 既にポップアップリスト上に存在していれば指定されたポップアップより上階層のポップアップを閉じる.
				if( PNLC_PopupListControl::newInstance()->isPopupLayer( next_popup ) ) {
// comment 2020/12/10 Sky延原 ログ強化改善.
					ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call closeSpecifiedPopupHighLayer( next_popup, True )", priTask, 0, 0, 0, 0, 0 );
					PNLC_PopupListControl::newInstance()->closeSpecifiedPopupHighLayer( next_popup, True );
					// 新規の場合はここでポップアップ.
				} else {
// comment 2020/12/10 Sky延原 ログ強化改善.
					ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call addPopup( next_popup )", priTask, 0, 0, 0, 0, 0 );
					PNLC_PopupListControl::newInstance()->addPopup( next_popup );
				}
// comment 2020/12/10 Sky延原 ログ強化改善.
				ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>[NULL]", priTask, 0, 0, 0, 0, 0 );
				nextGamen = NULL;						// nextGamenによる遷移は行わない.
			}
// comment 2020/12/10 Sky延原 ログ強化改善.
			ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] return", priTask, 0, 0, 0, 0, 0 );
			return;
		}
	}
//	拡大モード、割り込みモードでない場合は、Faxアプリの起動チェックを行う.
	PBasicMessageProc* pc_FaxAppGamen = NULL;
// comment 2020/12/10 Sky延原 ログ強化改善.
	ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] pc_FaxAppGamen[NULL]", priTask, 0, 0, 0, 0, 0 );

// comment 2020/12/10 Sky延原 ログ強化改善.
	ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] getSoftDipSwBitForPanelSpecNo( SCANFAXSENDADDCANCEL, 2 )[%d] onlyPanel.bl_isFromAppShortcut[%d]", priTask, APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( SCANFAXSENDADDCANCEL, 2 ), onlyPanel.bl_isFromAppShortcut , 0, 0, 0 );
	ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] onlyPanel.bl_isEnlargeDisplayMode[%d]", priTask, onlyPanel.bl_isEnlargeDisplayMode, 0, 0, 0, 0 );
	ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] onlyPanel.ec_IntMode[%d]", priTask, onlyPanel.ec_IntMode, 0, 0, 0, 0 );
	if( ( ( False == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( SCANFAXSENDADDCANCEL, 2 ) )	// DipSW No.73 bit2がOFF.
	      || ( True == onlyPanel.bl_isFromAppShortcut ) )															// トップメニューからのアプリ起動での遷移.
	    && ( False == onlyPanel.bl_isEnlargeDisplayMode )										// 拡大画面ではない.
	    && ( EC_IntMode_Off == onlyPanel.ec_IntMode ) ) {	// 且つ 割り込みモードでない.
		// 起動するアプリNoを取得.
		short ss_BootFaxApp = AplCommonSet.getDefaultAppNo();	// デフォルトアプリ.
// comment 2020/12/10 Sky延原 ログ強化改善.
		ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] ss_BootFaxApp[%d]", priTask, ss_BootFaxApp, 0, 0, 0, 0 );
		ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] getShortcutAppBootNo()[%d]", priTask, c_ExtPanel.getShortcutAppBootNo(), 0, 0, 0, 0 );
		if( 0 < c_ExtPanel.getShortcutAppBootNo() ) {			// トップメニューからのショートカット指定(APP指定)あり.
// comment 2020/12/10 Sky延原 ログ強化改善.
			ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] ss_BootFaxApp[%d]=>[%d]", priTask, ss_BootFaxApp, c_ExtPanel.getShortcutAppBootNo(), 0, 0, 0 );
			ss_BootFaxApp = c_ExtPanel.getShortcutAppBootNo();
			c_ExtPanel.setShortcutAppBootNo( 0 );					// クリア.
		}
		// アプリを起動する.
		if( 0 < ss_BootFaxApp ) {
			pc_FaxAppGamen = AplCommonSet.setApplicationSelect( ss_BootFaxApp );
// comment 2020/12/10 Sky延原 ログ強化改善.
			ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] pc_FaxAppGamen=>[%p]", priTask, pc_FaxAppGamen, 0, 0, 0, 0 );
		}
	}
// comment 2019/07/05 Sky毛穴 [特ROM][FQ-OPSD-1900229-OFP145] DipSW226のbit2がONでトップからのスキャンファクスプログラム呼出し画面への遷移でプログラム情報が書き換わる.
	Bool bl_NextIsFromRegAddress = False;
	// Faxアプリを起動チェック.
	// 遷移先あり、又は、LDAP検索実行中.
// comment 2020/12/10 Sky延原 ログ強化改善.
	ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] getRightFaxLdapSearching()[%d]", priTask, c_RightFaxLdapSearch.getRightFaxLdapSearching(), 0, 0, 0, 0 );
	if( ( NULL != pc_FaxAppGamen ) ||
	    ( True == c_RightFaxLdapSearch.getRightFaxLdapSearching() ) ) {
		AplCommonSet.setFaxAppOn( True );						// FaxAPP状態.
// comment 2020/12/10 Sky延原 ログ強化改善.
		ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen[%p]=>[%p]", priTask, nextGamen, pc_FaxAppGamen, 0, 0, 0 );
		nextGamen = pc_FaxAppGamen;								// 画面遷移.
		AplCommonSet.setFowardGamen( nextGamen );					// 前画面を保持.
		AplCommonSet.setSessionID( "1" );							// アプリ仮想接続.
	} else {
		// Faxボード装着 又は デフォルトアプリが存在しない場合はScan/Fax基本画面を表示する.
		// APPなし状態に設定する.
		ExtSet.setFaxAppOn( False );
		ExtSet.setSessionID( "" );			// 接続中アプリのセッションIDをクリア.
		Bool ldap_setPara = False;					// サーバ設定確認
// comment 2020/12/10 Sky延原 ログ強化改善.
		ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] b_isFaxLine1OffHook[%d] b_isOffHookBtnOn[%d]", priTask, b_isFaxLine1OffHook, b_isOffHookBtnOn, 0, 0, 0 );
		if( ( b_isFaxLine1OffHook == False )			// 外部電話によるオフフック状態
		    && ( b_isOffHookBtnOn    == False ) ) {	// オフフックキー押下によるオフフック状態
			if( e_KeyMode == TYPE_KeyModePageAcses ) {				// 認証装置(PageACSES).
				if( onlyPanel.bl_isEnlargeDisplayMode == False ) {
// comment 2020/12/10 Sky延原 ログ強化改善.
					ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>&gp_ScanFaxAttestationDeviceTop[%p]", priTask, &gp_ScanFaxAttestationDeviceTop, 0, 0, 0, 0 );
					nextGamen = &gp_ScanFaxAttestationDeviceTop;		// 10310000 認証装置トップメニュー
				} else {
// comment 2020/12/10 Sky延原 ログ強化改善.
					ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>&gp_ScanFaxAttestationDeviceTopEnl[%p]", priTask, &gp_ScanFaxAttestationDeviceTopEnl, 0, 0, 0, 0 );
					nextGamen = &gp_ScanFaxAttestationDeviceTopEnl;		// 10H10000 認証装置トップメニュー
				}
// comment 2020/12/10 Sky延原 ログ強化改善.
				ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] return", priTask, 0, 0, 0, 0, 0 );
				return;
			}
// comment 2020/12/10 Sky延原 ログ強化改善.
			ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] PnlComGetScanFaxMainDisp()[%d]", priTask, PnlComGetScanFaxMainDisp(), 0, 0, 0, 0 );
			switch( PnlComGetScanFaxMainDisp() ) {
				case TYPE_FaxMainDispProg:			// 宛先検索.
					// 宛先検索使用可否判定.
					if( ( True == PnlComGetFunctionShowFaxScan( TYPE_PNLMENU_FAXSCAN_AddressSearch ) ) ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
						Cmp_AddressSearchList_scanfax.resetSearch();			// 2010/05/24 OSW1-Sky桐島 <実装課題No.182対応>.
						if( isEnableLDAP() == True ) {
// comment 2020/12/10 Sky延原 ログ強化改善.
							ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] bl_isEnlargeDisplayMode[%d]", priTask, onlyPanel.bl_isEnlargeDisplayMode, 0, 0, 0, 0 );
							if( onlyPanel.bl_isEnlargeDisplayMode == True ) {
								uchar uc_TotalServer,uc_curServer,uc_index;
								uc_TotalServer = 0;
								uc_curServer   = 0;
								for( uc_index = 0; uc_index < 5; uc_index++ ) {
									if( isLdapServerHostEntryCheck( static_cast<uchar>( uc_index+1 ) ) == True ) {
										uc_curServer = uc_index;
										uc_TotalServer++ ;
									}
								}
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] uc_curServer[%d] uc_TotalServer[%d]", priTask, uc_curServer, uc_TotalServer, 0, 0, 0 );
								if( 1 < uc_TotalServer ) {
// comment 2020/12/10 Sky延原 ログ強化改善.
									ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>&gp_EnlScanFax_LDAPAuthenticationSelectHost[%p]", priTask, &gp_EnlScanFax_LDAPAuthenticationSelectHost, 0, 0, 0, 0 );
									nextGamen = &gp_EnlScanFax_LDAPAuthenticationSelectHost;
								} else if( 1 == uc_TotalServer ) {
// comment 2015/04/10 OPS1-Sky浜野 [HeliosL][CQ511844] トップ画面へキー押下で画面遷移不正不具合対応.
									// ldap_setPara = PCGC_ScanFaxLDAPAuthenticationCom::setLDAPConnectParam(uc_curServer+1) ;	
									ldap_setPara = PNLC_ScanFaxStaticComLib::newInstance()->setLDAPConnectParam( static_cast<short>( uc_curServer+1 ) ); 
									if( ldap_setPara == True ) {
										c_FaxLdapSearchMgr.setLdapSearchNon( False );
										APIC_LdapSetting* pc_LdapSetting = PnlComGetLdapSetting( static_cast<uchar>( uc_curServer+1 ) );
										if( NULL == pc_LdapSetting ) {
											LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
// comment 2020/12/10 Sky延原 ログ強化改善.
											ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] return", priTask, 0, 0, 0, 0, 0 );
											return;
										}
// comment 2020/12/10 Sky延原 ログ強化改善.
										ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] pc_LdapSetting->getLdapUserAuthentication()[%d]", priTask, pc_LdapSetting->getLdapUserAuthentication(), 0, 0, 0, 0 );
										if( TYPE_LdapUserAuthenticationDynamicAuthentication == pc_LdapSetting->getLdapUserAuthentication() ) {
											c_FaxLdapSearchMgr.setLdaoAccountInitStr( True );
											c_FaxLdapSearchMgr.setLdapSearchInitOn( True );
// comment 2020/12/10 Sky延原 ログ強化改善.
											ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>&gp_PD_10H00500[%p]", priTask, &gp_PD_10H00500, 0, 0, 0, 0 );
											nextGamen = &gp_PD_10H00500;
										} else if( TYPE_LdapUserAuthenticationUserAuthentication == pc_LdapSetting->getLdapUserAuthentication() ) {
											c_FaxLdapSearchMgr.setLdapSearchInitOn( True );
// comment 2020/12/10 Sky延原 ログ強化改善.
											ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>&gp_PD_10H00790[%p]", priTask, &gp_PD_10H00790, 0, 0, 0, 0 );
// comment 2020/11/17 2DWS2G-Sky肥後 クラウドSSO認証開発v1_Step2.
//											nextGamen = &gp_PD_10H00790;
											APIC_User* pc_User = PnlComGetUser();
											if( pc_User != NULL ) {
												if ( True == pc_User->isIdpCacheUserNo( pc_User->getUserNo() ) ) {
													nextGamen = &gp_PD_10H00500;
												} else {
													nextGamen = &gp_PD_10H00790;
												}
											} else {
												nextGamen = &gp_PD_10H00790;
											}
										}
										else {
											c_FaxLdapSearchMgr.setLdapSearchInitOn( True );
// comment 2020/12/10 Sky延原 ログ強化改善.
											ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>&gp_PD_10H00790[%p]", priTask, &gp_PD_10H00790, 0, 0, 0, 0 );
											nextGamen = &gp_PD_10H00790;
										}
									}
								}
							}
							else
							{
								nextGamen = dispatchGamenScanFaxLDAPAuthenticationSelectHost();
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>[%p]", priTask, nextGamen, 0, 0, 0, 0 );
							}
						} else {
							if( True == onlyPanel.bl_isEnlargeDisplayMode ) {
// 拡大時は宛先登録に遷移するため、宛先検索を実行する。
								gp_ScanFaxConfirmMethod.setSearchConditionAPI();
								// お待ちください画面に遷移する.
								// 初期設定.
								pc_ManageIGCWaitDisplay.initManageIGCWaitDisplay( PCGD_IGCDisplayStandard,
								        &gp_PD_10H00400,
								        getNowGamen() );
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call DelayedIGCWaitDisplayTimer::start( &gp_PD_10H00400, b_IsClearDisp )", priTask, 0, 0, 0, 0, 0 );
								DelayedIGCWaitDisplayTimer::start( &gp_PD_10H00400, b_IsClearDisp );
							} else {
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>&gp_AddressSearch_scanfax[%p]", priTask, &gp_AddressSearch_scanfax, 0, 0, 0, 0 );
								nextGamen = &gp_AddressSearch_scanfax;
							}
						}
					} else {
						// 禁止の時
// comment 2020/12/10 Sky延原 ログ強化改善.
						ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d]getNowPopup()[%p] gp_RemoteUserAccess_act [%p]", priTask, PNLC_PopupListControl::newInstance()->getNowPopup(), &gp_RemoteUserAccess_act, 0, 0, 0 );
						ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] getNowGamen()[%p] gp_EnlAct_RemoteUserAccess[%p]", priTask, getNowGamen(), &gp_EnlAct_RemoteUserAccess, 0, 0, 0 );
						ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] onlyPanel.ec_IntMode[%d]", priTask, onlyPanel.ec_IntMode, 0, 0, 0, 0 );
						if( ( PNLC_PopupListControl::newInstance()->getNowPopup() == &gp_RemoteUserAccess_act )
						    || ( getNowGamen() == &gp_EnlAct_RemoteUserAccess )
						    || ( onlyPanel.ec_IntMode != EC_IntMode_Off ) ) { 		//割り込みコピー中？
							nextGamen = dispatchGamenScanFaxFromRegAddress();
// comment 2020/12/10 Sky延原 ログ強化改善.
							ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen[%p]", priTask, nextGamen, 0, 0, 0, 0 );
						} else {
							PBasicMessageProc* p_Gamen = dispatchGamenScanFaxFromRegAddress();
// comment 2020/12/10 Sky延原 ログ強化改善.
							ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] p_Gamen[%p]", priTask, p_Gamen, 0, 0, 0, 0 );
							if( p_Gamen == &gp_FromRegAddress_scanfax ) {	// コールバック内検索処理が必要かどうか.
// comment 2019/07/05 Sky毛穴 [特ROM][FQ-OPSD-1900229-OFP145] DipSW226のbit2がONでトップからのスキャンファクスプログラム呼出し画面への遷移でプログラム情報が書き換わる.
								bl_NextIsFromRegAddress = True;
								PNLC_ScanFaxDataSearchLib* pc_ScanFaxDataSearchLib = PNLC_ScanFaxDataSearchLib::newInstance();		// 検索関連クラス.
								// S/MIME機能設定.
								pc_ScanFaxDataSearchLib->SmimeSetup();
								// 宛先検索要求.
								Bool b_OftenKey = pc_ScanFaxDataSearchLib->isOftenKey();											// 常用フラグ格納用.
								TYPE_SearchKey e_SearchKey = pc_ScanFaxDataSearchLib->getSearchKindKeyword();						// 検索キー格納用.
								TYPE_FaxDefaultAddrKind e_FaxDefaultAddrKind = PnlComGetFaxDefaultAddrKind();						// 宛先種別格納用.
								Bool bl_LimitationFunction 	= PNLC_LimitationFunctionLoginUserCheckLib::newInstance()->isEnableSetAddressLimitationFunction();
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] b_OftenKey[%d] e_SearchKey[%d] e_FaxDefaultAddrKind[%d] bl_LimitationFunction[%d]", priTask, b_OftenKey, e_SearchKey, e_FaxDefaultAddrKind, bl_LimitationFunction, 0 );
								// comment 2016/12/21(水) SCD2-Sky長谷川(伸) [ERBJTS1600884406]対応.
								gp_FromRegAddress_scanfax.initPreSearch();
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call pc_ScanFaxDataSearchLib->setSearchConditionAPI( &b_OftenKey, e_SearchKey, e_FaxDefaultAddrKind, bl_LimitationFunction )", priTask, 0, 0, 0, 0, 0 );
								pc_ScanFaxDataSearchLib->setSearchConditionAPI( &b_OftenKey, e_SearchKey, e_FaxDefaultAddrKind, bl_LimitationFunction );	// 検索要求.
								// お待ち下さい画面.
								pc_ManageIGCWaitDisplay.initManageIGCWaitDisplay( PCGD_IGCDisplayStandard, p_Gamen, getNowGamen() );
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call DelayedIGCWaitDisplayTimer::start_popup( &gp_FromRegAddress_scanfax, b_IsClearDisp )", priTask, 0, 0, 0, 0, 0 );
								DelayedIGCWaitDisplayTimer::start_popup( &gp_FromRegAddress_scanfax, b_IsClearDisp );
							} else {
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen[%p]=>[%p]", priTask, nextGamen, p_Gamen, 0, 0, 0 );
								nextGamen = p_Gamen;
							}
						}
					}
					break;
				case TYPE_FaxMainDispGroup:			// 履歴.
				{
					// リダイアル機能使用可否判定
// comment 2020/12/10 Sky延原 ログ強化改善.
					ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] isFaxFunctionUseCheck( PDPE_FaxFunctionCheckReDial )[%d] onlyPanel.isFaxServerEnable()[%d]", priTask, onlyPanel.isFaxFunctionUseCheck( PDPE_FaxFunctionCheckReDial ), onlyPanel.isFaxServerEnable(), 0, 0, 0 );
					if( ( False == onlyPanel.isFaxFunctionUseCheck( PDPE_FaxFunctionCheckReDial ) )	// 禁止時
					    || ( True == onlyPanel.isFaxServerEnable() ) ) {								// Prefix/Suffixを使用する.
						// OpenAPI使用中画面表示中の場合.
// comment 2020/12/10 Sky延原 ログ強化改善.
						ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] getNowPopup()[%p]: gp_RemoteUserAccess_act[%p]", priTask, PNLC_PopupListControl::newInstance()->getNowPopup(), &gp_RemoteUserAccess_act, 0, 0, 0 );
						ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] getNowGamen()[%p]: gp_EnlAct_RemoteUserAccess[%p]", priTask, getNowGamen(), &gp_EnlAct_RemoteUserAccess, 0, 0, 0 );
						ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] ec_IntMode[%d]", priTask, onlyPanel.ec_IntMode, 0, 0, 0, 0 );
						if( ( PNLC_PopupListControl::newInstance()->getNowPopup() == &gp_RemoteUserAccess_act )
						    || ( getNowGamen() == &gp_EnlAct_RemoteUserAccess )
						    || ( onlyPanel.ec_IntMode != EC_IntMode_Off ) ) {		//割り込みコピー中？
							nextGamen = dispatchGamenScanFaxFromRegAddress();
// comment 2020/12/10 Sky延原 ログ強化改善.
							ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>[%p]", priTask, nextGamen, 0, 0, 0, 0 );
						} else {
							PBasicMessageProc* p_Gamen = dispatchGamenScanFaxFromRegAddress();
// comment 2020/12/10 Sky延原 ログ強化改善.
							ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] p_Gamen[%p] gp_FromRegAddress_scanfax[%p]", priTask, p_Gamen, &gp_FromRegAddress_scanfax, 0, 0, 0 );
							if( p_Gamen == &gp_FromRegAddress_scanfax ) {	// コールバック内検索処理が必要かどうか.
// comment 2019/07/05 Sky毛穴 [特ROM][FQ-OPSD-1900229-OFP145] DipSW226のbit2がONでトップからのスキャンファクスプログラム呼出し画面への遷移でプログラム情報が書き換わる.
								bl_NextIsFromRegAddress = True;
								PNLC_ScanFaxDataSearchLib* pc_ScanFaxDataSearchLib = PNLC_ScanFaxDataSearchLib::newInstance();		// 検索関連クラス.
								// S/MIME機能設定.
								pc_ScanFaxDataSearchLib->SmimeSetup();
								// 宛先検索要求.
								Bool b_OftenKey = pc_ScanFaxDataSearchLib->isOftenKey();											// 常用フラグ格納用.
								TYPE_SearchKey e_SearchKey = pc_ScanFaxDataSearchLib->getSearchKindKeyword();						// 検索キー格納用.
								TYPE_FaxDefaultAddrKind e_FaxDefaultAddrKind = PnlComGetFaxDefaultAddrKind();	// 宛先種別格納用.
								Bool bl_LimitationFunction 	= PNLC_LimitationFunctionLoginUserCheckLib::newInstance()->isEnableSetAddressLimitationFunction();
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] b_OftenKey[%d] e_SearchKey[%d] e_FaxDefaultAddrKind[%d] bl_LimitationFunction[%d]", priTask, b_OftenKey, e_SearchKey, e_FaxDefaultAddrKind, bl_LimitationFunction, 0 );
								// comment 2016/12/21(水) SCD2-Sky長谷川(伸) [ERBJTS1600884406]対応.
								gp_FromRegAddress_scanfax.initPreSearch();
								pc_ScanFaxDataSearchLib->setSearchConditionAPI( &b_OftenKey, e_SearchKey, e_FaxDefaultAddrKind, bl_LimitationFunction );	// 検索要求.
								// お待ち下さい画面.
								pc_ManageIGCWaitDisplay.initManageIGCWaitDisplay( PCGD_IGCDisplayStandard, p_Gamen, getNowGamen() );
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call DelayedIGCWaitDisplayTimer::start_popup( &gp_FromRegAddress_scanfax, b_IsClearDisp )", priTask, 0, 0, 0, 0, 0 );
								DelayedIGCWaitDisplayTimer::start_popup( &gp_FromRegAddress_scanfax, b_IsClearDisp );
							} else {
								nextGamen = p_Gamen;
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>[%p]", priTask, nextGamen, 0, 0, 0, 0 );
							}
						}
					} else {
						// OpenAPI使用中画面表示中の場合.
// comment 2020/12/10 Sky延原 ログ強化改善.
						ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] getNowPopup()[%p]: gp_RemoteUserAccess_act[%p]", priTask, PNLC_PopupListControl::newInstance()->getNowPopup(), &gp_RemoteUserAccess_act, 0, 0, 0 );
						ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] getNowGamen()[%p]: gp_EnlAct_RemoteUserAccess[%p]", priTask, getNowGamen(), &gp_EnlAct_RemoteUserAccess, 0, 0, 0 );
						ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] ec_IntMode[%d]", priTask, onlyPanel.ec_IntMode, 0, 0, 0, 0 );
						if( ( PNLC_PopupListControl::newInstance()->getNowPopup() == &gp_RemoteUserAccess_act )
						    || ( getNowGamen() == &gp_EnlAct_RemoteUserAccess )
						    || ( onlyPanel.ec_IntMode != EC_IntMode_Off ) ) {		//割り込みコピー中？
							nextGamen = dispatchGamenScanFaxFromTxLog();
// comment 2020/12/10 Sky延原 ログ強化改善.
							ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>[%p]", priTask, nextGamen, 0, 0, 0, 0 );
						} else {
							gp_ScanFaxFromTxLog.setRedialAddressDbId();
// comment 2020/12/10 Sky延原 ログ強化改善.
							ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] ul_RedialAddressDbId[0][%d]", priTask, gp_ScanFaxFromTxLog.ul_RedialAddressDbId[0], 0, 0, 0, 0 );
							if( gp_ScanFaxFromTxLog.ul_RedialAddressDbId[0] != 0 ) {
								// 検索対象の短縮宛先が残っている場合検索要求を出す.
								TYPS_IgcAddressSearchCondition s_IgcAddressSearchCondition;
								s_IgcAddressSearchCondition = gp_ScanFaxFromTxLog.setSearchCondition( gp_ScanFaxFromTxLog.ul_RedialAddressDbId[0] );
								// 宛先要求情報指定.
								TYPS_ADDRESS_REQUEST_INFO s_AddressRequestInfo;
								// ユーザー情報取得.
								s_AddressRequestInfo.pC_user = PnlComGetUser();
								// 部門情報取得.
								s_AddressRequestInfo.pC_track = PnlComGetTrack();
								// 要求元先 (監査ログ I/F)はパネル.
								s_AddressRequestInfo.e_InspectLogIF = JLGE_InspectLogIfOperationPanel;
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] pC_user[%p]: pC_track[%p]: e_InspectLogIF[%d]", priTask, s_AddressRequestInfo.pC_user, s_AddressRequestInfo.pC_track, s_AddressRequestInfo.e_InspectLogIF, 0, 0 );
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call requestStartAddressSearchFirst()", priTask, 0, 0, 0, 0, 0 );
								// 検索開始（初回のみ）.
								c_AddressSearchIGC.requestStartAddressSearchFirst( &s_IgcAddressSearchCondition,
								        1,
								        &s_AddressRequestInfo );
								// お待ちください画面に遷移する.
								// 初期設定.
								pc_ManageIGCWaitDisplay.initManageIGCWaitDisplay( PCGD_IGCDisplayStandard,
								        dispatchGamenScanFaxFromTxLog(),
								        getNowGamen() );
								// Wait画面表示.
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call DelayedIGCWaitDisplayTimer::start( &gp_ScanFaxFromTxLog, b_IsClearDisp )", priTask, 0, 0, 0, 0, 0 );
								DelayedIGCWaitDisplayTimer::start( &gp_ScanFaxFromTxLog, b_IsClearDisp );
							} else {
// 宛先登録件数取得はA032010000_宛先登録_宛先一覧画面の登録キー押下で行うように修正したため件数取得は削除.
								nextGamen = dispatchGamenScanFaxFromTxLog();
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>[%p]", priTask, nextGamen, 0, 0, 0, 0 );
							}
						}
					}
				}
				break;
				case TYPE_FaxMainDispAbb:			// 登録宛先.
					// OpenAPI使用中画面表示中の場合.
// comment 2020/12/10 Sky延原 ログ強化改善.
					ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] getNowPopup()[%p]: gp_RemoteUserAccess_act[%p]", priTask, PNLC_PopupListControl::newInstance()->getNowPopup(), &gp_RemoteUserAccess_act, 0, 0, 0 );
					ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] getNowGamen()[%p]: gp_EnlAct_RemoteUserAccess[%p]", priTask, getNowGamen(), &gp_EnlAct_RemoteUserAccess, 0, 0, 0 );
					ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] ec_IntMode[%d]", priTask, onlyPanel.ec_IntMode, 0, 0, 0, 0 );
					if( ( PNLC_PopupListControl::newInstance()->getNowPopup() == &gp_RemoteUserAccess_act )
					    || ( getNowGamen() == &gp_EnlAct_RemoteUserAccess )
					    || ( onlyPanel.ec_IntMode != EC_IntMode_Off ) ) {		//割り込みコピー中？
						nextGamen = dispatchGamenScanFaxFromRegAddress();
// comment 2020/12/10 Sky延原 ログ強化改善.
						ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>[%p]", priTask, nextGamen, 0, 0, 0, 0 );
					} else {
						PBasicMessageProc* p_Gamen = dispatchGamenScanFaxFromRegAddress();
// comment 2020/12/10 Sky延原 ログ強化改善.
						ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] p_Gamen[%p]: gp_FromRegAddress_scanfax[%p]", priTask, p_Gamen, &gp_FromRegAddress_scanfax, 0, 0, 0 );
						if( p_Gamen == &gp_FromRegAddress_scanfax ) {	// コールバック内検索処理が必要かどうか.
// comment 2019/07/05 Sky毛穴 [特ROM][FQ-OPSD-1900229-OFP145] DipSW226のbit2がONでトップからのスキャンファクスプログラム呼出し画面への遷移でプログラム情報が書き換わる.
							bl_NextIsFromRegAddress = True;
// comment 2015/05/11 OPS1-Sky村井 [Helios][CQ512675]特ROMマージ[特ROM][FQ-ISDCS-1400589-OFP205]標準トップメニューに登録したファックス/スキャンプログラムから宛先を選択するとスクロールバーが表示されなくなる.
							PNLC_ScanFaxDataLib::newInstance()->setIsSearchinCallBack( True );
							PNLC_ScanFaxDataSearchLib* pc_ScanFaxDataSearchLib = PNLC_ScanFaxDataSearchLib::newInstance();		// 検索関連クラス.
							// S/MIME機能設定.
							pc_ScanFaxDataSearchLib->SmimeSetup();
							Bool b_OftenKey = pc_ScanFaxDataSearchLib->isOftenKey();											// 常用フラグ格納用.
							TYPE_SearchKey e_SearchKey = pc_ScanFaxDataSearchLib->getSearchKindKeyword();						// 検索キー格納用.
							TYPE_FaxDefaultAddrKind e_FaxDefaultAddrKind = PnlComGetFaxDefaultAddrKind();	// 宛先種別格納用.
							Bool bl_LimitationFunction 	= PNLC_LimitationFunctionLoginUserCheckLib::newInstance()->isEnableSetAddressLimitationFunction();
// comment 2020/12/10 Sky延原 ログ強化改善.
							ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] b_OftenKey[%d]: e_SearchKey[%d]: e_FaxDefaultAddrKind[%d]: bl_LimitationFunction[%d]", priTask, b_OftenKey, e_SearchKey, e_FaxDefaultAddrKind, bl_LimitationFunction, 0 );
							// comment 2016/12/21(水) SCD2-Sky長谷川(伸) [ERBJTS1600884406]対応.
							gp_FromRegAddress_scanfax.initPreSearch();
// comment 2020/12/10 Sky延原 ログ強化改善.
							ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call setSearchConditionAPI( &b_OftenKey, e_SearchKey, e_FaxDefaultAddrKind, bl_LimitationFunction )", priTask, 0, 0, 0, 0, 0 );
							pc_ScanFaxDataSearchLib->setSearchConditionAPI( &b_OftenKey, e_SearchKey, e_FaxDefaultAddrKind, bl_LimitationFunction );	// 検索要求.
							// お待ち下さい画面.
							pc_ManageIGCWaitDisplay.initManageIGCWaitDisplay( PCGD_IGCDisplayStandard, p_Gamen, getNowGamen() );
// comment 2020/12/10 Sky延原 ログ強化改善.
							ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call DelayedIGCWaitDisplayTimer::start_popup( &gp_FromRegAddress_scanfax, b_IsClearDisp )", priTask, 0, 0, 0, 0, 0 );
							DelayedIGCWaitDisplayTimer::start_popup( &gp_FromRegAddress_scanfax, b_IsClearDisp );
							PM_TaskID priTask;
							PMGC_PanelManager::getPriority( &priTask );
							if( PM_TASKID_PANEL == priTask ) {
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>[NULL]", priTask, nextGamen, 0, 0, 0, 0 );
								nextGamen = NULL;
							}
						} else {
							if( True == onlyPanel.bl_isEnlargeDisplayMode ) {

								// 検索文字か宛先種別かで使うAPIを振り分け.
								gp_ScanFaxConfirmMethod.setSearchConditionAPI();
								// 初期設定.
								pc_ManageIGCWaitDisplay.initManageIGCWaitDisplay( PCGD_IGCDisplayStandard,
								        p_Gamen,
								        getNowGamen() );
								if( True == b_IsClearDisp ) {
// comment 2020/12/10 Sky延原 ログ強化改善.
									ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call DelayedIGCWaitDisplayTimer::start_popup( &gp_ScanFaxConfirmMethod, b_IsClearDisp )", priTask, 0, 0, 0, 0, 0 );
									DelayedIGCWaitDisplayTimer::start_popup( &gp_ScanFaxConfirmMethod, b_IsClearDisp );	// ベース画面チラつき対策.
								} else {
// comment 2020/12/10 Sky延原 ログ強化改善.
									ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call DelayedIGCWaitDisplayTimer::start( &gp_ScanFaxConfirmMethod, b_IsClearDisp )", priTask, 0, 0, 0, 0, 0 );
									DelayedIGCWaitDisplayTimer::start( &gp_ScanFaxConfirmMethod, b_IsClearDisp );
								}
							} else {
								nextGamen = p_Gamen;
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>[%p]", priTask, nextGamen, 0, 0, 0, 0 );
							}
						}
					}
					break;
				case TYPE_FaxMainDispTenKey:		// 直接入力.
					// 手動宛先入力禁止機能設定を判定する
// Comment 【HCL-Prem】2018/02/03【IT5_4.2】【AR1037925】Blocking Dirept input screen launch for Network2.
//					if( isDisableAddressManualInput() == True ) {	// 禁止時.
// comment 2020/12/10 Sky延原 ログ強化改善.
					ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] isDisableAddressManualInput()[%d]: getLine()[%d]", priTask, isDisableAddressManualInput(), PNLC_NetworkLan::getInstance()->getLine(), 0, 0, 0 );
					if( ( isDisableAddressManualInput() == True ) 
						|| ( PNLC_NetworkLan::getInstance()->getLine()  == TYPE_NetworkSpecific_NW2 ) ) {	// 禁止時.
						// OpenAPI使用中画面表示中の場合.
// comment 2020/12/10 Sky延原 ログ強化改善.
						ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] getNowPopup()[%p]: gp_RemoteUserAccess_act[%p]", priTask, PNLC_PopupListControl::newInstance()->getNowPopup(), &gp_RemoteUserAccess_act, 0, 0, 0 );
						ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] getNowGamen()[%p]: gp_EnlAct_RemoteUserAccess[%p]", priTask, getNowGamen(), &gp_EnlAct_RemoteUserAccess, 0, 0, 0 );
						ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] ec_IntMode[%d]", priTask, onlyPanel.ec_IntMode, 0, 0, 0, 0 );
						if( ( PNLC_PopupListControl::newInstance()->getNowPopup() == &gp_RemoteUserAccess_act )
						    || ( getNowGamen() == &gp_EnlAct_RemoteUserAccess )
						    || ( onlyPanel.ec_IntMode != EC_IntMode_Off ) ) {		//割り込みコピー中？
							nextGamen = dispatchGamenScanFaxFromRegAddress();
// comment 2020/12/10 Sky延原 ログ強化改善.
							ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>[%p]", priTask, nextGamen, 0, 0, 0, 0 );
						} else {
							PBasicMessageProc* p_Gamen = dispatchGamenScanFaxFromRegAddress();
// comment 2020/12/10 Sky延原 ログ強化改善.
							ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] p_Gamen[%p]: gp_FromRegAddress_scanfax[%p]", priTask, p_Gamen, &gp_FromRegAddress_scanfax, 0, 0, 0 );
							if( p_Gamen == &gp_FromRegAddress_scanfax ) {	// コールバック内検索処理が必要かどうか.
// comment 2019/07/05 Sky毛穴 [特ROM][FQ-OPSD-1900229-OFP145] DipSW226のbit2がONでトップからのスキャンファクスプログラム呼出し画面への遷移でプログラム情報が書き換わる.
								bl_NextIsFromRegAddress = True;
								PNLC_ScanFaxDataSearchLib* pc_ScanFaxDataSearchLib = PNLC_ScanFaxDataSearchLib::newInstance();		// 検索関連クラス.
								// S/MIME機能設定.
								pc_ScanFaxDataSearchLib->SmimeSetup();
								Bool b_OftenKey = pc_ScanFaxDataSearchLib->isOftenKey();											// 常用フラグ格納用.
								TYPE_SearchKey e_SearchKey = pc_ScanFaxDataSearchLib->getSearchKindKeyword();						// 検索キー格納用.
								TYPE_FaxDefaultAddrKind e_FaxDefaultAddrKind = PnlComGetFaxDefaultAddrKind();	// 宛先種別格納用.
								Bool bl_LimitationFunction 	= PNLC_LimitationFunctionLoginUserCheckLib::newInstance()->isEnableSetAddressLimitationFunction();
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] b_OftenKey[%d] e_SearchKey[%d] e_FaxDefaultAddrKind[%d] bl_LimitationFunction[%d]", priTask, b_OftenKey, e_SearchKey, e_FaxDefaultAddrKind, bl_LimitationFunction, 0 );
								// comment 2016/12/21(水) SCD2-Sky長谷川(伸) [ERBJTS1600884406]対応.
								gp_FromRegAddress_scanfax.initPreSearch();
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call pc_ScanFaxDataSearchLib->setSearchConditionAPI( &b_OftenKey, e_SearchKey, e_FaxDefaultAddrKind, bl_LimitationFunction )", priTask, 0, 0, 0, 0, 0 );
								pc_ScanFaxDataSearchLib->setSearchConditionAPI( &b_OftenKey, e_SearchKey, e_FaxDefaultAddrKind , bl_LimitationFunction );	// 検索要求.
								// お待ち下さい画面.
								pc_ManageIGCWaitDisplay.initManageIGCWaitDisplay( PCGD_IGCDisplayStandard, p_Gamen, getNowGamen() );
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call DelayedIGCWaitDisplayTimer::start_popup( &gp_FromRegAddress_scanfax, b_IsClearDisp )", priTask, 0, 0, 0, 0, 0 );
								DelayedIGCWaitDisplayTimer::start_popup( &gp_FromRegAddress_scanfax, b_IsClearDisp );
							} else {
// comment 2022/07/28 FPT.DungDV14 [IT6_2.3] [OP_BTS-38732] Adding enlarge screen support.								
								if( True == onlyPanel.bl_isEnlargeDisplayMode ) {
									// 検索文字か宛先種別かで使うAPIを振り分け.
									gp_ScanFaxConfirmMethod.setSearchConditionAPI();
									// 初期設定.
									pc_ManageIGCWaitDisplay.initManageIGCWaitDisplay( PCGD_IGCDisplayStandard,
											p_Gamen,
											getNowGamen() );
									if( True == b_IsClearDisp ) {
										ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call DelayedIGCWaitDisplayTimer::start_popup( &gp_ScanFaxConfirmMethod, b_IsClearDisp )", priTask, 0, 0, 0, 0, 0 );
										DelayedIGCWaitDisplayTimer::start_popup( &gp_ScanFaxConfirmMethod, b_IsClearDisp );	// ベース画面チラつき対策.
									} else {
										ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call DelayedIGCWaitDisplayTimer::start( &gp_ScanFaxConfirmMethod, b_IsClearDisp )", priTask, 0, 0, 0, 0, 0 );
										DelayedIGCWaitDisplayTimer::start( &gp_ScanFaxConfirmMethod, b_IsClearDisp );
									}
								} else {
									nextGamen = p_Gamen;
// comment 2020/12/10 Sky延原 ログ強化改善.
									ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>[%p]", priTask, nextGamen, 0, 0, 0, 0 );
								}								
							}
						}
					} else {
						// 拡大の時は直接入力からは始まらない.
						if( onlyPanel.bl_isEnlargeDisplayMode == False ) {
// comment 2020/12/10 Sky延原 ログ強化改善.
							ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>&gp_DirectInput_scanfax[%p]", priTask, &gp_DirectInput_scanfax, 0, 0, 0, 0 );
							nextGamen = &gp_DirectInput_scanfax;
						} else {
							// OpenAPI使用中画面表示中の場合.
// comment 2020/12/10 Sky延原 ログ強化改善.
							ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] getNowPopup()[%p]: gp_RemoteUserAccess_act[%p]", priTask, PNLC_PopupListControl::newInstance()->getNowPopup(), &gp_RemoteUserAccess_act, 0, 0, 0 );
							ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] getNowGamen()[%p]: gp_EnlAct_RemoteUserAccess[%p]", priTask, getNowGamen(), &gp_EnlAct_RemoteUserAccess, 0, 0, 0 );
							ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] ec_IntMode[%d]", priTask, onlyPanel.ec_IntMode, 0, 0, 0, 0 );
							if( ( PNLC_PopupListControl::newInstance()->getNowPopup() == &gp_RemoteUserAccess_act )
							    || ( getNowGamen() == &gp_EnlAct_RemoteUserAccess )
							    || ( onlyPanel.ec_IntMode != EC_IntMode_Off ) ) {		//割り込みコピー中？
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>[%p]", priTask, &gp_PD_10H00400, 0, 0, 0, 0 );
								nextGamen = &gp_PD_10H00400;
							} else {
								// 検索文字か宛先種別かで使うAPIを振り分け.
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call gp_ScanFaxConfirmMethod.setSearchConditionAPI()", priTask, 0, 0, 0, 0, 0 );
								gp_ScanFaxConfirmMethod.setSearchConditionAPI();
								// お待ちください画面に遷移する.
								// 初期設定.
								pc_ManageIGCWaitDisplay.initManageIGCWaitDisplay( PCGD_IGCDisplayStandard,
								        &gp_PD_10H00400,
								        getNowGamen() );
// comment 2020/12/10 Sky延原 ログ強化改善.
								ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call DelayedIGCWaitDisplayTimer::start( &gp_PD_10H00400, b_IsClearDisp )", priTask, 0, 0, 0, 0, 0 );
								DelayedIGCWaitDisplayTimer::start( &gp_PD_10H00400, b_IsClearDisp );
							}
						}
					}
					break;
				case TYPE_FaxMainDispIFax:
				default: break;
			}
		} else {
			// 手動宛先入力禁止機能設定を判定する
			if( isDisableAddressManualInput() == True ) {	// 禁止時.
				// OpenAPI使用中画面表示中の場合.
// comment 2020/12/10 Sky延原 ログ強化改善.
				ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] getNowPopup()[%p]: gp_RemoteUserAccess_act[%p]", priTask, PNLC_PopupListControl::newInstance()->getNowPopup(), &gp_RemoteUserAccess_act, 0, 0, 0 );
				ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] getNowGamen()[%p]: gp_EnlAct_RemoteUserAccess[%p]", priTask, getNowGamen(), &gp_EnlAct_RemoteUserAccess, 0, 0, 0 );
				ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] ec_IntMode[%d]", priTask, onlyPanel.ec_IntMode, 0, 0, 0, 0 );
				if( ( PNLC_PopupListControl::newInstance()->getNowPopup() == &gp_RemoteUserAccess_act )
				    || ( getNowGamen() == &gp_EnlAct_RemoteUserAccess )
				    || ( onlyPanel.ec_IntMode != EC_IntMode_Off ) ) {		//割り込みコピー中？
					nextGamen = dispatchGamenScanFaxCommonUseBasicOffHook();
// comment 2020/12/10 Sky延原 ログ強化改善.
					ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>[%p]", priTask, nextGamen, 0, 0, 0, 0 );
				} else {
					PBasicMessageProc* p_Gamen = dispatchGamenScanFaxCommonUseBasicOffHook();
// comment 2020/12/10 Sky延原 ログ強化改善.
					ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] p_Gamen[%p]: gp_EntryAddressOffHook_scanfax[%p]", priTask, p_Gamen, &gp_EntryAddressOffHook_scanfax, 0, 0, 0 );
					if( p_Gamen == &gp_EntryAddressOffHook_scanfax ) {	// コールバック内検索処理が必要かどうか.
						// 宛先検索要求.
						PNLC_ScanFaxDataSearchLib* pc_ScanFaxDataSearchLib = PNLC_ScanFaxDataSearchLib::newInstance();		// 検索関連クラス.
						Bool b_OftenKey = pc_ScanFaxDataSearchLib->isOftenKey();											// 常用フラグ格納用.
						TYPE_SearchKey e_SearchKey = pc_ScanFaxDataSearchLib->getSearchKindKeyword();						// 検索キー格納用.
						TYPE_FaxDefaultAddrKind e_FaxDefaultAddrKind = PnlComGetFaxDefaultAddrKind();	// 宛先種別格納用.
// comment 2020/12/10 Sky延原 ログ強化改善.
						ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] b_OftenKey[%d]: e_SearchKey[%d]: e_FaxDefaultAddrKind[%d]", priTask, b_OftenKey, e_SearchKey, e_FaxDefaultAddrKind, 0, 0 );
						ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call pc_ScanFaxDataSearchLib->setSearchConditionAPI( &b_OftenKey, e_SearchKey, e_FaxDefaultAddrKind )", priTask, 0, 0, 0, 0, 0 );
						pc_ScanFaxDataSearchLib->setSearchConditionAPI( &b_OftenKey, e_SearchKey, e_FaxDefaultAddrKind );	// 検索要求.
						// お待ち下さい画面.
						pc_ManageIGCWaitDisplay.initManageIGCWaitDisplay( PCGD_IGCDisplayStandard, p_Gamen, getNowGamen() );
// comment 2020/12/10 Sky延原 ログ強化改善.
						ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] call DelayedIGCWaitDisplayTimer::start_popup( &gp_EntryAddressOffHook_scanfax, b_IsClearDisp )", priTask, 0, 0, 0, 0, 0 );
						DelayedIGCWaitDisplayTimer::start_popup( &gp_EntryAddressOffHook_scanfax, b_IsClearDisp );
					} else {
						nextGamen = p_Gamen;
// comment 2020/12/10 Sky延原 ログ強化改善.
						ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>[%p]", priTask, nextGamen, 0, 0, 0, 0 );
					}
				}
			} else {	// 許可時
				// オフフックダイレクト入力画面へ遷移
				nextGamen = dispatchGamenScanFaxDirectFaxOffHook();
// comment 2020/12/10 Sky延原 ログ強化改善.
				ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen=>[%p]", priTask, nextGamen, 0, 0, 0, 0 );
			}
		}
	}
// comment 2019/07/05 Sky毛穴 [特ROM][FQ-OPSD-1900229-OFP145] DipSW226のbit2がONでトップからのスキャンファクスプログラム呼出し画面への遷移でプログラム情報が書き換わる.
// comment 2020/12/10 Sky延原 ログ強化改善.
	ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] nextGamen[%p]: gp_FromRegAddress_scanfax[%p]", priTask, nextGamen, &gp_FromRegAddress_scanfax, 0, 0, 0 );
	ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] bl_NextIsFromRegAddress[%d]: m_bl_ScanProgramBasicFromTop[%d]", priTask, bl_NextIsFromRegAddress, onlyPanel.m_bl_ScanProgramBasicFromTop, 0, 0, 0 );
	if (   (   &gp_FromRegAddress_scanfax == nextGamen			// 次画面が登録宛先から.
			|| True == bl_NextIsFromRegAddress )
		&& True == onlyPanel.m_bl_ScanProgramBasicFromTop		// トップからのスキャンファクスプログラム呼出し画面への遷移.
		 ) {
		onlyPanel.m_bl_SkipAddrSearchForFromRegAddress = True;
	} else {							// 条件に合わない場合はフラグを落とす.
		onlyPanel.m_bl_SkipAddrSearchForFromRegAddress = False;
	}
// comment 2020/12/10 Sky延原 ログ強化改善.
	ExpPanelLog( PNLD_ExpLogKind_NextMainFaxGamen, "[LocalUI][nextMainFaxGamen][PriTask:%d] bl_isJobStopping[%d]=>[False]", priTask, onlyPanel.bl_isJobStopping, 0, 0, 0, 0 );
	onlyPanel.bl_isJobStopping = False;
}

/*================================================================================================
［機能］	画面開始時の2度読みかどうかVoiceIDで判定する処理（音声ガイド用）.
================================================================================================*/
Bool PBasicMessageProc::checkForGamenStartVoice() {

	if( NULL == nextGamen || False == onlyPanel.bl_isEnlargeDisplayMode )	{ return True; }	// 画面切り替え時では無いもしくは拡大画面では無い.

	ushort* pus_VoiceBuffer = PNLC_VoiceRequestBuffer::newInstance()->getVoiceBuffer();

	if( False == bl_SameGamenInstanceForVoiceStartGamen ) {	// 画面インスタンス切り替え時.
		// いつ2度読みがくるか分からない為、ここでVoiceIDを取得しておく.
		// ここで更新しておかないと 拡大警告A ⇒ 通常拡大画面 ⇒ 拡大警告A のケースの時、読み上げをはじいてしまう.
		memcpy( us_PreGamenStartVoiceID, pus_VoiceBuffer, sizeof( us_PreGamenStartVoiceID ) );
		return True;

	} else {	// 表示していた画面と同じ画面インスタンスがchangeGamenに設定されていた場合.
		// 前回の画面スタート時のVoiceIDと今回の画面スタート時VoiceIDを比較する.
		for( uchar uc_LoopCnt = 0; DEF_VoiceIdBufSize > uc_LoopCnt; uc_LoopCnt++ )	{
			// VoiceIDが同じまま終端まで比較した場合（2度読み発生!）.
			if( 0 == pus_VoiceBuffer[ uc_LoopCnt ] && 0 == us_PreGamenStartVoiceID[ uc_LoopCnt ] )	{
				return False;
			}
			// 前回の画面スタート時のVoiceIDとは異なる場合.
			if( pus_VoiceBuffer[ uc_LoopCnt ] != us_PreGamenStartVoiceID[ uc_LoopCnt ] )	{
				memcpy( us_PreGamenStartVoiceID, pus_VoiceBuffer, sizeof( us_PreGamenStartVoiceID ) );
				return True;
			}
		}
	}
	return True;
}

//====================================================================================================================.
/**
* @brief  : Function for DFRaise request.
* @note   : Function to check EDH status and send DF raise request for non application screen, Scanbrowser screen.
			ScanD aplication screen and ScanS application screen.
* @param  : None.
* @return : None.
* @author : 2015/05/12 [HCLT-Rajamanickam] [MinervaSBK][機器構成変更_No.13].
*/
//====================================================================================================================.
void PBasicMessageProc::reqDFRaise()
{
	if( ( False == ScanOapExtSet.isActiveGeneralApplication() )			//Non-Application screen.
		|| ( TYPE_ExtTypeBrowserScan == ScanOapExtSet.getExtAppType()	//BrowserScan App check.
		|| TYPE_ExtTypeScanD == ScanOapExtSet.getExtAppType()			//ScanD App check.
		|| TYPE_ExtTypeScanS == ScanOapExtSet.getExtAppType() ) ) {		//ScanS App check.

		if( True == ppc_ScannerStatus->isEdhReadReady()) { // check the Read preparation command acceptance status.
			ppc_ScannerStatus->req_SendEdhReadReadyCmd(); //Send ready command to Scan.
		}
	}
	return;
}

/*================================================================================================
［機能］	Popup画面開始時の2度読みかどうかVoiceIDで判定する処理（音声ガイド用）.
================================================================================================*/
Bool PBasicMessageProc::onChangePopupGamenStartVoice( PBasicMessageProc* pCurrentVoicePlayGamen )
{
	if( NULL == pCurrentVoicePlayGamen ) {
		return False;
	}

	if( False == onlyPanel.bl_isEnlargeDisplayMode ) {
		return True;	// 画面切り替え時では無いもしくは拡大画面では無い.
	}

	ushort* pus_VoiceBuffer = PNLC_VoiceRequestBuffer::newInstance()->getVoiceBuffer();

	PBasicMessageProc* pBackUpGamen = PNLC_EnlVoiceGuidanceControl::getCurrentVoicePlayGamen();

	if( pBackUpGamen != pCurrentVoicePlayGamen ) {	// 画面インスタンス切り替え時.
		// いつ2度読みがくるか分からない為、ここでVoiceIDを取得しておく.
		// ここで更新しておかないと 拡大警告A ⇒ 通常拡大画面 ⇒ 拡大警告A のケースの時、読み上げをはじいてしまう.
		memcpy( us_PreGamenStartVoiceID, pus_VoiceBuffer, sizeof( us_PreGamenStartVoiceID ) );
		return True;

	} else {	// 表示していた画面と同じ画面インスタンスがchangeGamenに設定されていた場合.
		// 前回の画面スタート時のVoiceIDと今回の画面スタート時VoiceIDを比較する.
		for( uchar uc_LoopCnt = 0; DEF_VoiceIdBufSize > uc_LoopCnt; uc_LoopCnt++ )	{
			// VoiceIDが同じまま終端まで比較した場合（2度読み発生!）.
			if( 0 == pus_VoiceBuffer[ uc_LoopCnt ] && 0 == us_PreGamenStartVoiceID[ uc_LoopCnt ] )	{
				return False;
			}
			// 前回の画面スタート時のVoiceIDとは異なる場合.
			if( pus_VoiceBuffer[ uc_LoopCnt ] != us_PreGamenStartVoiceID[ uc_LoopCnt ] )	{
				memcpy( us_PreGamenStartVoiceID, pus_VoiceBuffer, sizeof( us_PreGamenStartVoiceID ) );
				return True;
			}
		}
	}

	return True;
}
#if defined( DEF_SimAutoTest ) && ( defined(DEF_ZeusCus) || defined(DEF_ZeusEmu) )		// 自動評価で有効. // 2010/12/15 DEF_AmurCus → DEF_ZeusCus 仁枝.
int n_ActionInitialSetupRepeat = 0;
#endif
/*================================================================================================
［機能］	メッセージ処理
@date   2010/06/25  Sky田中(利) 品質改善【対策分類No.2,3,5】画面切替.
@date	2013/02/20  Sky中江		 操作性改善FUM4 [WBS No.202][CT不具合対応]画面反転要求タイミング変更.
@date	2014/01/08  OPS1-Sky薗﨑 [3rdVerUp][制御構成変更_No.14] 画面反転機能対応.
================================================================================================*/
static int output_flg_d = 0;
extern "C" {
	void getNowGamen2_d( int i );
}
void getNowGamen2_d( int i ) {
	output_flg_d = i;
}
PBasicMessageProc* PBasicMessageProc::messageProc( Message message, uchar option, MessageType type, Bool buzzer, int32_t i32_option , void* st_NtfEventData )
{
	Message e_OriginalMessage = message;

	static Bool	bl_isPreEnlargeDisplayMode = False;			// 前回の拡大表示モード.
// comment 2015/06/15 OPS1-Sky佐藤 [ZeusS_ZX0][RQ-405] パネルUIのヘブライ語対応.
	static PM_TaskID	PrePriorityTask = PM_TASKID_PANEL;	// 前回の優先タスク.

	// comment 2018/12/26 SCD2-Sky小畑 [EagleH] WebUIv3FUM対応.
	// comment 2018/08/21 SCD2-Sky竹菴 [Eagle][1057602] UIからの再起動要求以外で再起動が掛けられた場合のデータ初期化を追加.
	//if( BRWE_BrowserBootMode_WEBUI != onlyPanel.e_BrowserBootMode ){
	//	if( PNLE_WebUIStartStatusStarted == PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ){
	//		PNLC_WebUIControl::getInstance()->initData();
	//		PNLC_WebUIMessageConvert::getInstance()->initData();
	//		PNLC_WebUIScreenInfo::getInstance()->initData();
	//	}
	//}

	setInchModeToPartsClass();				// パーツクラスにインチモードを知らせる
	if( nextGamen != &gp_AppChangeBrowser ) {	// gp_AppChangeBrowserが指定されていた場合は初期化しない.
		if( True == pc_ManageIGCWaitDisplay.getReqDisplayFLG() ) {	// 「お待ちください」P.U.表示要求中の場合.
			// 「お待ちください」P.U.表示要求フラグを下げる.
			pc_ManageIGCWaitDisplay.setReqDisplayFLGOff();
			// 検索結果通知の場合は「お待ちください」P.U.表示をキャンセルする(nextGamenを初期化する).
			if( MS_CallBack == message ) {
				switch( i32_option ) {
					case TYPE_PNL_CallBack_ShorteningAddressSearchResult:
					case TYPE_PNL_CallBack_ShorteningGroupAddressSearchResult:
					case TYPE_PNL_CallBack_GroupAddressSearchResult:
					case TYPE_PNL_CallBack_ProgramKeySearchResult:
					case TYPE_PNL_CallBack_TempSendSearchResult:
						nextGamen = NULL;
						break;
					default:
						// 処理なし.
						break;
				}
			}
		}
		else {
			nextGamen = NULL;
		}
	}

	// Rhein3用の遷移先画面のポインタ.
	PBasicMessageProc*	changeGamen;
	changeGamen = &gp_SetupGamen;	// changeGamenはNULLではなくダミー画面としておく(起動時のFatalで落ちるため).
	b_BuzzerCancel = False;					// 画面ブザー禁止フラグ True:受付音を鳴らさない False:受付音を鳴らす.
	Bool b_changeGamenFlg = False;
	// 制御構成変更209 管理No.892_1 第3世代→ポップアップ警告表示 酒井健.
	// ポップアップリスト管理クラスのインスタンスを取得.
	PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();
	if( NULL == pc_PopupListControl ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return changeGamen;
	}
//comment 2018/05/16 HCLT-HariBhaskar [IT6_Eagle] [RQ 748] .
#ifndef DEF_A4_Cus
	if ( True == onlyPanel.bl_isEnlargeDisplayMode ) {
// comment 2018/05/24 [HCL-HariBhaskar] [Eagle] [1046864] - Function name changed
		PNLC_EnlWarningManager::newInstance()->changeGamenOriginalPaperReadSizeWarningEnlarge(comPanelJob);
	}
#endif
#if defined(SIMULATOR) && !defined(LINUX)
	if( m_bVxSimConnectLinux == false ) {
#endif
		if( True == buzzer ) {
			if( MS_QtDoubleTap != message ) {	//Qtプレビュー詳細のダブルタップとピンチはブザー鳴動しない.
				setBuzzerType( PCGE_BuzzerType_InputOk );
			}
		}
		PNLC_VoiceRequestBuffer::newInstance()->clearRequest();// 再生要求バッファのクリア
		// ベース画面と表示中のポップアップへのメッセージ振り分け.
		PBasicMessageProc* pOldPopup = pc_PopupListControl->getNowPopup();
		char sc_OldPoupCount = pc_PopupListControl->getPopupNum();
		if( True == pc_PopupListControl->isPopupOpen() ) {
			pc_PopupListControl->messageDispathPopup( message, option, type, i32_option, st_NtfEventData );	// ポップアップ画面のメッセージディスパッチ呼び出し.
		} else {
			// comment 02/23/2021 FPT duongdv11 Fix OP_BTS_30723. 
			if (MS_DeleteBroadcastAddress == message && nowGamen != &gp_FaxDirectInput_scanfax ) {
				dynamic_cast<PBasicMessageProc*>(&gp_FaxDirectInput_scanfax)->messageDispath( message, option, type, i32_option, st_NtfEventData );
			}
			nowGamen->messageDispath( message, option, type, i32_option, st_NtfEventData );					// ベース画面のメッセージ・ディスパッチ処理. Base screen message dispatch processing.
		}

		if( MS_TabletReset == e_OriginalMessage ) {
			// 遠隔操作端末切断時のリセット操作については、鳴動音をならさない.
			setBuzzerType( PCGE_BuzzerType_Off, True );
		}


		// 最優先タスクIDの取得.
		PM_TaskID priTask;
		PMGC_PanelManager::getPriority( &priTask );

		if( output_flg_d ) {
			Browser_DebugPrint( "PBasicMessageProc::messageProc nextGamen:%s\n" ,( nextGamen )? ( intptr_t )typeid( *nextGamen ).name() : ( intptr_t )"NULL",2,3,4,5,6 );
			Browser_DebugPrint( "PBasicMessageProc::messageProc nowGamen: %s\n" ,( nowGamen ) ? ( intptr_t )typeid( *nowGamen ).name()  : ( intptr_t )"NULL",2,3,4,5,6 );
		}

// 2022/03/30 生産RINNE対応 Sky start.
		// Rinne画面からは既存ラインモードへの遷移しかしない.
		if( &gp_AppChangeBrowserLineMode == nowGamen ){
			if( ( &gp_FactoryAdjustModeTopMenu_MiSBK != nextGamen )
			 && ( &gp_PublicPopupDummyBase != nextGamen ) ) {
				nextGamen = NULL;
			}
		}
// 2022/03/30 生産RINNE対応 Sky end.

// comment 2015/06/15 OPS1-Sky佐藤 [ZeusS_ZX0][RQ-405] パネルUIのヘブライ語対応.
		// 優先タスクの切り替わりを検知.
		if ( PrePriorityTask != priTask ) {
			PrePriorityTask = priTask;
	
			// マシン設定が右詰め表示言語の時はメッセージを再ロードする.
// comment 2016/01/28 OPS1-Sky浜野 [ZeusS_ZX0][CQ524401] Myパネル対応.
//			TYPE_Language ec_Type;
//			if ( PnlComGetOperationSetting()->getLanguageChoiceSetting() ) {
//				ec_Type = onlyPanel.e_Language;
//			} else {
//				ec_Type = PnlComGetMachineSetting()->getLanguage();
//			}
//			if ( True == isRightDispLanguage( ec_Type ) ) {
//				loadSelectLangType_IOP( PnlComGetLanguage() );
//			}
			TYPE_Language e_CheckLanguage = PnlComGetLanguage( False );
// comment 2018/12/14 Sky毛穴 [EagleHigh][RQ-258] タイ語とベトナム語の対応.
//			if ( True == isRightDispLanguage( e_CheckLanguage ) ) {
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
				loadSelectLangType_IOP( PnlComGetLanguage() );
			}
		}

		if( nextGamen != NULL ) {
// comment 2019/10/01 Sky東島 [EagleHigh][OP_BTS-14263].
			if( ( True == onlyPanel.b_ScreenSvStart )
			 && ( &gp_AppChangeScreenSaver != nextGamen ) ){
				// スクリーンセーバー画面以外に遷移する場合はonlyPanel.b_ScreenSvStartフラグを解除する.
				onlyPanel.b_ScreenSvStart = False;
				onlyPanel.b_ScreenSvStartUpFlag = False;
			}
// comment 2015/11/10 MSE岡田 [MinervaSBK][制御構成変更No.74]パネルハング対応.
		// 画面遷移発生時に強制システムオートリセット監視カウンターをクリアする.
		PMessageProcCom::c_autoRestTimer.m_c_ForceSysAutoReset.clearTimerCount();
		PMessageProcCom::c_autoRestTimer.m_pc_MonitoredGamen = NULL;
		if( PM_TASKID_BROWSER != priTask ) {
				// 最優先アプリがブラウザ以外の時.
				if( ( nextGamen == &gp_AppChangeBrowser )
				    || ( nextGamen == &gp_AppChangeUIBrowser )		// comment : WebUI2 画面切替え制御 20180302 Chikuan.
				    || ( nextGamen == &gp_ScanOapBrowserDummy )
				    || ( nextGamen == &gp_PullPrintOapBrowserDummy )
				    || ( nextGamen == &gp_GeneralOapBrowserDummy )			// ブラウザ統合アプリケーション画面.
				    || ( nextGamen == &gp_CostCenterResetDummy)		//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Add.
				    || ( nextGamen == &gp_AppChangeScreenSaver ) ) {
					// 次画面がブラウザ制御用画面の時.
					if( nowGamen == &gp_AuthOapBrowserWaitEx ) {
						// カレント画面がブラウザ制御待ち用画面の時.
						// Browser待ち制御用画面内の遷移待ち画面情報を更新する.
						// －切替待ち制御中の他イベント割り込みによる画面遷移要求.
						Browser_DebugPrint( "PBasicMessageProc::messageProc Implement 1\n" ,1,2,3,4,5,6 );
						gp_AuthOapBrowserWaitEx.changeWaitGamen( nextGamen );
					} else if( ( nowGamen != &gp_AppChangeBrowser )
					           && ( nowGamen != &gp_AppChangeUIBrowser )		// comment : WebUI2 画面切替え制御 20180302 Chikuan.
					           && ( nowGamen != &gp_ScanOapBrowserDummy )
					           && ( nowGamen != &gp_PullPrintOapBrowserDummy )
					           && ( nowGamen != &gp_GeneralOapBrowserDummy )			// ブラウザ統合アプリケーション画面.
					           && ( nowGamen != &gp_CostCenterResetDummy)		//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Add.
					           && ( nowGamen != &gp_AppChangeScreenSaver ) ) {
						// カレント画面がブラウザ制御用画面以外の時.
						// 次画面を"Browser待ち制御用画面"に差替える.
						// －NativePanel⇒Browser切替開始時.
						Browser_DebugPrint( "PBasicMessageProc::messageProc Implement 2-1\n" ,1,2,3,4,5,6 );
// comment : WebUI2 画面切替え制御 -> 20180302 Chikuan.
						if( ( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() )
// comment 2019/05/15 Chikuan [EagleH][OP_BTS-1792] V1遷移パフォーマンス改善.
						 && ( nextGamen == &gp_AppChangeUIBrowser )
// comment 2022/04/28 Sky中野 [IT6_2.3] 制御No.43_システムオートリセット改善(WebUIv2/v3統合).
//						 && ( PNLE_WebUIStartStatusStarted == PNLC_WebUIControl::getInstance()->getWebUIStartStatus() ) ){
						 && ( PNLE_BrowserChangeTabControlTabInitStatus_Done == PNLC_BrowserChangeTabControl::getInstance()->getTabInitStatus() ) ){
// comment 2018/10/22 SCD2-Sky竹菴 [Eagle][1061802] WebUIにメッセージを通知するタイミングを変更したことで、WebUIへの通知条件を変更.
//							if( PNLE_WebUIScreenChangeStatusStart != PNLC_WebUIControl::getInstance()->getWebUIScreenChangeStatus() ){
							if( PNLE_WebUIScreenChangeStatusIdle == PNLC_WebUIControl::getInstance()->getWebUIScreenChangeStatus() ){
								PNLC_WebUIControl::getInstance()->reserveChangeScreen( PNLE_WebUIRequestMessageToBrowser );
// comment 2020/10/22 Sky中野 [IT6_2.1][OP_BTS-28575].
// comment 2022/04/28 Sky中野 [IT6_2.3] 制御No.43_システムオートリセット改善(WebUIv2/v3統合).
//								if( PNLE_BrowserChangeTabMode_Home == PNLC_BrowserChangeTabControl::getInstance()->getBrowserChangeTabMode() ) {
								if( PNLE_WebUIFunctionHome == PNLC_WebUIControl::getInstance()->getDispWebUIScreen() ) {
									// Home画面に戻る場合は、トップメニューモードをONにする.
									onlyPanel.setTopmenuMode( True );
								}
							}
							PNLC_WebUIControl::getInstance()->requestChangeScreen();
						}
// comment 2020/03/27 SCD2-Sky中野 [HeliosMLK][OP_BTS-21285].
						if( ( &gp_AppChangeBrowser == nextGamen )
						 && ( PNLE_BrowserChangeTabMode_Admin == PNLC_BrowserChangeTabControl::getInstance()->getBrowserChangeTabMode() ) ) {
							// V1画面からLegacy画面への切替時.
							PNLC_WebUIControl::getInstance()->reserveChangeScreen( PNLE_WebUIRequestMessageToBrowser );
							PNLC_WebUIControl::getInstance()->requestChangeScreen();
							PNLC_WebUIControl::getInstance()->crearChangeScreenInfo();
						}
// comment : WebUI2 画面切替え制御 <- 20180302 Chikuan.
						gp_AuthOapBrowserWaitEx.changeWaitGamen( nextGamen );
						nextGamen = &gp_AuthOapBrowserWaitEx;
						Browser_DebugPrint( "PBasicMessageProc::messageProc Implement 2-2\n" ,1,2,3,4,5,6 );
					} else {
						Browser_DebugPrint( "PBasicMessageProc::messageProc NO PG 1\n" ,1,2,3,4,5,6 );
// comment 2015/03/02 OPS1-Sky稲井 [特ROM対応][MZV-35_機種マージ] IWS APP画面の個別対応.
					if ( ( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW165, 0 ) )
					  && ( PM_TASKID_BRWPNL == priTask ) ) {
						PM_TaskID	desTskId = PM_TASKID_BROWSER;		// 通知先タスクID.
						uchar		uc_Content = PM_APPCHGSTS_START;	// 要求内容（開始/終了）.
						uchar		uc_Factor = PM_APPCHGSTS_USER;		// 要求要因.
						ushort		us_AccID	= PM_ACCID_MAX;						// 受付ID.
					
						pc_PanelMain->SendAppChgReqForOut(desTskId, uc_Content, uc_Factor, us_AccID);
					}
					}
				} else if( nextGamen == &gp_AuthOapBrowserWaitEx ) {
					// 次画面がブラウザ制御待ち用画面の時.
					Browser_DebugPrint( "PBasicMessageProc::messageProc NO PG 2\n" ,1,2,3,4,5,6 );
				} else {
					// 次画面がその他の時.
					if( nowGamen == &gp_AuthOapBrowserWaitEx ) {
						// カレント画面がブラウザ制御待ち用画面の時.
						// Browser待ち制御用画面内の遷移待ち画面情報を更新する.
						// －切替待ち制御中の他イベント割り込みによる画面遷移要求.
						Browser_DebugPrint( "PBasicMessageProc::messageProc Implement 3\n" ,1,2,3,4,5,6 );
						if( nextGamen != NULL ) {
							Browser_DebugPrint( "PBasicMessageProc::messageProc Implement 4\n" ,1,2,3,4,5,6 );
							gp_AuthOapBrowserWaitEx.changeWaitGamen( nextGamen );
						}
					} else if( ( nowGamen != &gp_AppChangeBrowser )
					           && ( nowGamen != &gp_AppChangeUIBrowser )		// comment : WebUI2 画面切替え制御 20180302 Chikuan.
					           && ( nowGamen != &gp_ScanOapBrowserDummy )
					           && ( nowGamen != &gp_PullPrintOapBrowserDummy )
					           && ( nowGamen != &gp_GeneralOapBrowserDummy )			// ブラウザ統合アプリケーション画面.
					           && ( nowGamen != &gp_CostCenterResetDummy )		//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Add.
					           && ( nowGamen != &gp_AppChangeScreenSaver ) ) {
						// カレント画面がブラウザ制御用画面以外の時.
						// －Browserに関係しないNativePanel間の画面切替時（基本ケース）.
						// 処理なし.
						Browser_DebugPrint( "PBasicMessageProc::messageProc NO PG 3\n" ,1,2,3,4,5,6 );
					} else {
						// カレント画面がブラウザ制御用画面の時.
						Browser_DebugPrint( "PBasicMessageProc::messageProc NO PG 4\n" ,1,2,3,4,5,6 );
					}
				}
			} else {
				// 最優先アプリがブラウザの時.
				if( ( nextGamen == &gp_AppChangeBrowser )
				    || ( nextGamen == &gp_AppChangeUIBrowser )		// comment : WebUI2 画面切替え制御 20180302 Chikuan.
				    || ( nextGamen == &gp_ScanOapBrowserDummy )
				    || ( nextGamen == &gp_PullPrintOapBrowserDummy )
				    || ( nextGamen == &gp_GeneralOapBrowserDummy )			// ブラウザ統合アプリケーション画面.
				    || ( nextGamen == &gp_CostCenterResetDummy)		//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Add.
				    || ( nextGamen == &gp_AppChangeScreenSaver ) ) {
// 2022/7/27 Sky higashino [特ROM][SPS]DipSW対応.
					PNLC_SPSAuthDataLib* pc_SPSAuthDataLib = PNLC_SPSAuthDataLib::newInstance();
					// 次画面がブラウザ制御用画面の時.
					if( nowGamen == &gp_AuthOapBrowserWaitEx ) {
						// カレント画面がブラウザ制御待ち用画面の時.
						// －切替制御完了後のDummy画面解除。（Browser画面表示開始）.
						Browser_DebugPrint( "PBasicMessageProc::messageProc NO PG 5\n" ,1,2,3,4,5,6 );
						// 処理なし.
					} else if( ( nowGamen == &gp_AppChangeBrowser )
					           || ( nowGamen == &gp_AppChangeUIBrowser )		// comment : WebUI2 画面切替え制御 20180302 Chikuan.
					           || ( nowGamen == &gp_ScanOapBrowserDummy )
					           || ( nowGamen == &gp_PullPrintOapBrowserDummy )
					           || ( nowGamen == &gp_GeneralOapBrowserDummy )			// ブラウザ統合アプリケーション画面.
					           || ( nowGamen == &gp_CostCenterResetDummy )		//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Add.
							   || ( ( NULL != pc_SPSAuthDataLib ) && ( True == pc_SPSAuthDataLib->isSPSMode() ) && ( nowGamen == &gp_AuthOapBrowserLogin ) )		// SPS Nakano .
					           || ( nowGamen == &gp_AppChangeScreenSaver ) ) {
						// カレント画面がブラウザ制御用画面の時.
						// －通常Browser(アプリ連携Browser)⇔アプリ連携Browser間の画面切替時.
						Browser_DebugPrint( "PBasicMessageProc::messageProc NO PG 6\n" ,1,2,3,4,5,6 );
						// 処理なし.
					} else {
// comment [2019/12/10] HCL-Devipriya [EagleZ+] [OP_BTS-16450] Do not overwrite nextGamen when nextscreen is also browserscreen.
						if ( nextGamen != &gp_AppChangeScreenSaver ) {
							// comment 2019/09/29 sky-tobita [EagleH][OP_BTS-13966].
							gp_AuthOapBrowserWaitEx.changeWaitGamen( nextGamen );
							nextGamen = &gp_AuthOapBrowserWaitEx;
							Browser_DebugPrint( "PBasicMessageProc::messageProc NO PG 7\n" ,1,2,3,4,5,6 );
						}
					}
				} else if( nextGamen == &gp_AuthOapBrowserWaitEx ) {
					// 次画面がブラウザ制御待ち用画面の時.
					if( ( nowGamen == &gp_AppChangeBrowser )
					    || ( nowGamen == &gp_AppChangeUIBrowser )		// comment : WebUI2 画面切替え制御 20180302 Chikuan.
					    || ( nowGamen == &gp_ScanOapBrowserDummy )
					    || ( nowGamen == &gp_PullPrintOapBrowserDummy )
					    || ( nowGamen == &gp_GeneralOapBrowserDummy )			// ブラウザ統合アプリケーション画面.
					    || ( nowGamen == &gp_CostCenterResetDummy )		//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Add.
					    || ( nowGamen == &gp_AppChangeScreenSaver ) ) {
						// カレント画面がブラウザ制御用画面の時.
						// －使用ケースなし。（BrowserPanelチラつきが解消されない場合、使用を検討する）.
						Browser_DebugPrint( "PBasicMessageProc::messageProc NO PG 8\n" ,1,2,3,4,5,6 );
						// 処理なし.
					} else if( nowGamen == &gp_AuthOapBrowserWaitEx ) {
						// カレント画面がブラウザ制御用画面以外の時.
						Browser_DebugPrint( "PBasicMessageProc::messageProc NO PG 9\n" ,1,2,3,4,5,6 );
					} else {
						// カレント画面がブラウザ制御用画面以外の時.
						Browser_DebugPrint( "PBasicMessageProc::messageProc NO PG 10\n" ,1,2,3,4,5,6 );
					}
// comment 2022/06/22 Sky東島 [IT6_2.3][制御構成変更_No.51][サービスレスメンテナンス].
				} else if( ( nextGamen == &gp_PublicPopupDummyBase )
				        && ( &gp_PopupBrowserWait_popup == gp_PublicPopupDummyBase.getStackPopup() ) ){
					// 何もしない.
				} else {
					// 次画面がその他の時.
					if( ( nowGamen == &gp_AppChangeBrowser )
					    || ( nowGamen == &gp_AppChangeUIBrowser )		// comment : WebUI2 画面切替え制御 20180302 Chikuan.
					    || ( nowGamen == &gp_ScanOapBrowserDummy )
					    || ( nowGamen == &gp_PullPrintOapBrowserDummy )
					    || ( nowGamen == &gp_GeneralOapBrowserDummy )			// ブラウザ統合アプリケーション画面.
					    || ( nowGamen == &gp_CostCenterResetDummy )		//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Add.
					    || ( nowGamen == &gp_AppChangeScreenSaver ) ) {
						// カレント画面がブラウザ制御用画面の時.
						// －Browser⇒NativePanel切替時.
						Browser_DebugPrint( "PBasicMessageProc::messageProc NO PG 11\n" ,1,2,3,4,5,6 );
// comment : WebUI2 画面切替え制御 -> 20180302 Chikuan.
						if( &gp_AppChangeUIBrowser == nowGamen ){
							if( PNLE_WebUIScreenChangeStatusStart != PNLC_WebUIControl::getInstance()->getWebUIScreenChangeStatus() ){
								PNLC_WebUIControl::getInstance()->reserveChangeScreen( PNLE_WebUIRequestMessageToUI );
							}
							PNLC_WebUIControl::getInstance()->requestChangeScreen();
							// comment 2018/12/05 SCD2-Sky竹菴 [Eagle][1068887] UIへ戻る時は即座にステータスを初期化し、以降の通知ができるように修正.
							PNLC_WebUIControl::getInstance()->crearChangeScreenInfo();
						}
// comment 2020/03/27 SCD2-Sky中野 [HeliosMLK][OP_BTS-21285].
						if( ( &gp_AppChangeBrowser == nowGamen )
						 && ( PNLE_BrowserChangeTabMode_Admin == PNLC_BrowserChangeTabControl::getInstance()->getBrowserChangeTabMode() ) ) {
							// V1画面からLegacy画面への切替時.
							PNLC_WebUIControl::getInstance()->reserveChangeScreen( PNLE_WebUIRequestMessageToUI );
							PNLC_WebUIControl::getInstance()->requestChangeScreen();
							PNLC_WebUIControl::getInstance()->crearChangeScreenInfo();
						}
// comment : WebUI2 画面切替え制御 <- 20180302 Chikuan.
// comment 2020/06/02 Sky中野 [IT6_2.0MR2][OP_BTS-24122].
						if( ( &gp_WarningPopupDummyBase != nextGamen ) 
						&&	( &gp_PublicPopupDummyBase != nextGamen )
						&&	( &gp_MenuPopupDummyBase != nextGamen ) ) {
							// ブラウザ画面→ブラウザ画面以外の遷移時は、ブラウザへの通知待ち状態とする.
							onlyPanel.setDrawCompleteStatus( PNLE_DrawComplete_WaitToBrowser );
						}
						// 処理なし.
					} else if( nowGamen == &gp_AuthOapBrowserWaitEx ) {
						// カレント画面がブラウザ制御待ち用画面の時.
						// －切替制御完了後のDummy画面解除（NativePanelへ再度、アプリ切替）.
						Browser_DebugPrint( "PBasicMessageProc::messageProc NO PG 12\n" ,1,2,3,4,5,6 );
						// 処理なし.
					} else {
						// カレント画面がブラウザ制御用画面以外の時.
						Browser_DebugPrint( "PBasicMessageProc::messageProc NO PG 13\n" ,1,2,3,4,5,6 );
					}
				}

			}
		}

// comment : WebUI2 画面切替え制御 -> 20180302 Chikuan.
		if( ( PNLE_UIModeLightUI == PNLC_WebUIControl::getInstance()->getUIMode() )
// comment 2022/04/28 Sky中野 [IT6_2.3] 制御No.43_システムオートリセット改善(WebUIv2/v3統合).
//		 && ( PNLE_WebUIStartStatusStarted == PNLC_WebUIControl::getInstance()->getWebUIStartStatus() )
		 && ( PNLE_BrowserChangeTabControlTabInitStatus_Done == PNLC_BrowserChangeTabControl::getInstance()->getTabInitStatus() )
		 && ( PNLE_WebUIScreenChangeStatusDuring != PNLC_WebUIControl::getInstance()->getWebUIScreenChangeStatus() )
// comment : 2018/10/02 SCD2-Sky竹菴 [Eagle][1061806] WebUIへの遷移ステータスの更新条件を追加..
//		 && ( PNLE_WebUIScreenChangeStatusDuringNoNeedNotify != PNLC_WebUIControl::getInstance()->getWebUIScreenChangeStatus() ) ){
// comment 2018/10/22 SCD2-Sky竹菴 [Eagle][1061802] ブラウザ切替え時に表示する従来のお待ちください画面でWebUIへの遷移準備完了通知を待たなくなるので条件を削除.
//		 && ( PNLE_WebUIScreenChangeStatusDuringNoNeedNotify != PNLC_WebUIControl::getInstance()->getWebUIScreenChangeStatus() )
		 && ( PNLE_WebUIScreenChangeStatusComplete != PNLC_WebUIControl::getInstance()->getWebUIScreenChangeStatus() )
// comment 2022/04/28 Sky中野 [IT6_2.3] 制御No.43_システムオートリセット改善(WebUIv2/v3統合).
		 && ( PNLE_WebUIRequestMessageToHomeScreen != PNLC_WebUIControl::getInstance()->getWebUIRequestMessage() ) ) {
			PNLC_WebUIControl::getInstance()->crearChangeScreenInfo();
		}
// comment : WebUI2 画面切替え制御 <- 20180302 Chikuan.

		// ポップアップ表示中にBrowserWait画面へ遷移する場合は、透過度を下げたレイヤをポップアップする.
		PNLC_PopupListControl* pc_PopupListCnt = PNLC_PopupListControl::newInstance();		// ポップアップ管理クラスを取得.

		if( &gp_AuthOapBrowserWait == nextGamen ) {
			if( NULL != pc_PopupListCnt && True == pc_PopupListCnt->isPopupOpen() ) {
				gp_AuthOapBrowserWait.setBrowserWaitpopupFlg( True );
			} else {
				gp_AuthOapBrowserWait.setBrowserWaitpopupFlg( False );
			}
		} else if( &gp_AuthOapBrowserWaitEx == nextGamen ) {
			if( NULL != pc_PopupListCnt && True == pc_PopupListCnt->isPopupOpen() ) {
				gp_AuthOapBrowserWaitEx.setBrowserWaitpopupFlg( True );
			} else {
				gp_AuthOapBrowserWaitEx.setBrowserWaitpopupFlg( False );
			}
		}
// Comment 2016/11/29 HCLT-Prem [IT5_3.4][1008531]Adding condition to reboot the browser based on temporary language.
		static TYPE_Language e_Language_Bkp = PnlComGetMachineSetting()->getLanguage();
		if ( ( nextGamen == &gp_AppChangeBrowser )
			|| ( nextGamen == &gp_AuthOapBrowserWaitEx ) ){
// 2017/03/22 Sprint5 A-18-00001 管理者設定キーでPSWCログイン画面へ遷移時は下記通常モードへ遷移しないように対応.
			if (  &gp_AdminNetworkSetCardAuthenticationSet != gp_AppChangeBrowser.getNextGamenForIWSContentsEnd() )  {
// comment 2018/01/25 SCD2-Sky溝部 [FQ-ISD2G-1700063-OFP258][AR1036488] ブラウザ連携アプリの初回起動時に通常ブラウザが起動する問題対応.
				// 通常ブラウザ起動かつ言語が変更されている場合はブラウザを再起動する(アプリ起動中は再起動しない).
				if ( ( e_Language_Bkp != onlyPanel.e_Language )
				&& 	 ( TYPE_ExtTypeBrowserLauncher != ScanOapExtSet.getExtAppType() )
//comment 2019/07/20 [HCL-Tarun]  [IT6_EagleH][OP_BTS-10392] Reverting [OP_BTS-9698] Fix for White Screen Display Issue.
////comment 2019/07/12 [HCL-Raja]  [IT6_EagleH][OP_BTS-9698] Add the conditon for browserboot.
				&& 	 ( BRWE_BrowserBootMode_Default == onlyPanel.e_BrowserBootMode ) ) {
//				&& 	 ( BRWE_BrowserBootMode_Default == onlyPanel.e_BrowserBootMode  || BRWE_BrowserBootMode_WEBUI == onlyPanel.e_BrowserBootMode ) ) {
					BRWS_RebootSettingInfo s_RebootSettingInfo;
// 次世代パネル 【AR課題1035241】start.
					memset( &s_RebootSettingInfo, 0x00, sizeof( BRWS_RebootSettingInfo ) );
// 次世代パネル 【AR課題1035241】end.
						s_RebootSettingInfo.e_BrowserBootMode = BRWE_BrowserBootMode_Default;	// 通常モード.
						memset( s_RebootSettingInfo.sc_BrowserURL,'\0', sizeof( s_RebootSettingInfo.sc_BrowserURL ) );
						// comment 2019/01/08 SCD2-Sky小畑 [EagleH] WebUIv3FUM対応.
						//PnlComBrowserReboot( &s_RebootSettingInfo );
						PNLC_BrowserChangeTabControl::getInstance()->changeTab( PNLE_BrowserChangeTabMode_WEB, &s_RebootSettingInfo );
						onlyPanel.b_ReqBrowserReboot = True;
					e_Language_Bkp = onlyPanel.e_Language;
				}
			}
//comment 2018/10/08 HCLT-Raja [AR1062574] Update the webUI mode as false, transition from webUI to panel and same has been handled in appchangebrowser class.				
//			onlyPanel.m_bl_isWebUI = False;
		}
#if defined( DEF_SimAutoTest ) && ( defined(DEF_ZeusCus) || defined(DEF_ZeusEmu) )		// 自動評価で有効. 		// 2010/12/15 DEF_AmurCus → DEF_ZeusCus 仁枝.
		// Amur自動評価Generic起動時に問題があるので暫定で描画間隔を空ける対応.
		APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
		// インスタンスがあって、OWNモードならば.
		if( ( NULL != pc_SystemInfo ) && ( True == pc_SystemInfo->isOwnOrGeneric() ) ) {
			// 今まで通りの処理.
			if( nextGamen == this
			    && nextGamen == &gp_ActionInitialSetup ) {
				TYPE_PowerSaveStatus e_PowerSaveStatus = ppc_Power->getPowerSaveStatus();
				if( True == PnlComIsPowerSaveMode( e_PowerSaveStatus ) ) {	// パワーセーブモード時（バックライトOFF時）.
					nextGamen = NULL;
				}
			}
		} else {
			// Genericのみ起動画面の描画間隔を延ばす.
			if( nextGamen == this
			    && nextGamen == &gp_ActionInitialSetup ) {
				// 6回ループする間は描画しない.
				if( n_ActionInitialSetupRepeat < 6 ) {
					n_ActionInitialSetupRepeat++;
					nextGamen = NULL;	//描画しない.
				} else {
					n_ActionInitialSetupRepeat = 0;
				}
			}
		}
#else
		//自動評価のAmur以外は以前のまま.
		if( nextGamen == this
		    && nextGamen == &gp_ActionInitialSetup ) {
			TYPE_PowerSaveStatus e_PowerSaveStatus = ppc_Power->getPowerSaveStatus();
			if( True == PnlComIsPowerSaveMode( e_PowerSaveStatus ) ) {	// パワーセーブモード時（画面遷移不可の時）.
				nextGamen = NULL;
			}
		}
#endif
//comment 2017/10/10 SCD2-Sky肥後 [IT5_4.2] [1030534] タスクコマンド生成対応.
//#ifdef DEF_PNL_DIRECT_KEY_DEBUG
		PNLC_SendPanelMainEventLib* pc_instance = PNLC_SendPanelMainEventLib::newInstance();
		if( NULL != pc_instance ) {
			pc_instance->checkDirectKey( message, option, nextGamen );
		}
//#endif	// #ifdef DEF_PNL_DIRECT_KEY_DEBUG.

	if( nextGamen != NULL ) {
		uchar uc_AuthSeqCustomValid = TYPD_SeqCustom_Function_INVALID;	// 認証カスタマイズ機能有効/無効値.
		PnlComGetSeqCustomValid( &uc_AuthSeqCustomValid );		// 認証カスタマイズ機能有効/無効設定取得.
		
		if( TYPD_SeqCustom_Function_VALID == uc_AuthSeqCustomValid ){		// 認証カスタマイズ機能が有効な場合.
			if( NULL != m_pc_SeqCusGamen ){	// 先頭画面取得処理で取得したインスタンスが存在する場合.
				if( nextGamen != m_pc_SeqCusGamen ){	// 次線遷移先画面インスタンスと先頭画面取得処理で取得したインスタンスが異なる場合.
					// 最終的に遷移先が変わり、認証シーケンスから抜けた処理となるためリセット処理を行う.
					TYPE_PASM_RET e_Ret = PnlComSeqReset();
					if( TYPE_PASM_RET_SUCCESS != e_Ret ) {	// 処理結果が成功以外.
						APIC_SystemStatus*	pc_Status	= PnlComGetSystemStatus();
						if ( 0 != pc_Status ) {
							printf("setProcessTroubleInfo, PnlComSeqReset() is Failed : %s(%d)\n", __FILE__, __LINE__ );
							pc_Status->setFatal( PNLD_ErrCode_Process_SeqCustom );
//comment 2017/07/05 SCD2-Sky肥後 [1026857] 新詳細コード対応.
							LAPC_BFW_System *pc_BFW_System = LAPC_BFW_System::newInstance();
							if (NULL != pc_BFW_System){
						   	 	pc_BFW_System ->registDetailCode( PNLD_DetailCode_FileIdentification_0x1900740020000000 | ( __LINE__ << 4 ) );
							}
						}
					}
				}
				PBasicMessageProc::setSeqCustomGamenIns( NULL );
			}
		}
		// テキストフィールド動的領域解放		
		PTextField::freeLayerDynamicText( 0 );

		PLayerControl* pOpeningPopup = PNLC_PopupListControl::newInstance()->getNowPopup() ;
		if( NULL != pOpeningPopup ){
			if( True == pOpeningPopup->isNowPopupAnimation() ){
				pOpeningPopup->completeGamenAnimationForced() ;
			}
		}
		PNLC_PopupListControl::newInstance()->removeAllPopup();

		// テキストフィールド動的領域解放		
		PTextField::freeLayerDynamicText( 0 );
	    
		// 登録キー押下状態をクリア.
		PLBC_RegistKeyPushStatus::getInstance()->init();
		// 画面移動を行う時
		if( nextGamen->needPrevGamenCapture() ){
			// 遷移先画面用に遷移元画面キャプチャを生成.
			nextGamen->capturePrevGamen( nowGamen );
		}
		nowGamen->hide();		// 画面パーツの消去
		nowGamen->endGamen();	// 画面終了処理
		PNLC_SlideMenuControlLib::newInstance()->judgeHide();

		Kbd_ScanFaxKeybordConv.bl_IsInUSBKeybordGamen = False;

		Kbd_ScanFaxKeybordConv.setIPv4v6Gamen( TYPE_USBKeyIPvGamen_Non );
		changeGamen = replaceGamen( nextGamen );
		changeGamen = PLBC_ReplaceCheck_PKI::replaceGamen( changeGamen );

		changeGamen = PLBC_FuncVersionCheck::replaceGamen( changeGamen );

		if((nowGamen != nextGamen ) && ( PM_TASKID_BROWSER == priTask || PM_TASKID_BRWPNL == priTask )
			&& (nextGamen != &gp_AppChangeBrowser )
			&& (nextGamen != &gp_AppChangeUIBrowser )		// comment : WebUI2 画面切替え制御 20180302 Chikuan.
			&& (nextGamen != &gp_AuthOapBrowserLogin )
			&& (nextGamen != &gp_AppChangeScreenSaver )
			&& (nextGamen != &gp_AuthOapBrowserWait )
			&& (nextGamen != &gp_AuthOapBrowserWaitEx )
			&& (nextGamen != &gp_ScanOapBrowserDummy )
			&& (nextGamen != &gp_PullPrintOapBrowserDummy )
			&& (nextGamen != &gp_GeneralOapBrowserDummy )
			&& (nextGamen != &gp_CostCenterResetDummy )		//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/24 Add.
			&& (nextGamen != &gp_IwsAuthBrowserLogin)
			&& (nextGamen != &gp_IwsAuthBrowserWait)
			&& (nextGamen != &gp_UserSetupBrowser )		// ユーザーセットアップ画面.			// comment 2022/01/28 Sky東島 [IT6_2.2MR2][制御34]ユーザーセットアップ.
			&& (nextGamen != &gp_UserSetupBrowserWait )	// ユーザーセットアップ切り替え中画面.	// comment 2022/01/28 Sky東島 [IT6_2.2MR2][制御34]ユーザーセットアップ.
		)
		{
			b_changeGamenFlg = True;
		}
		else {
// comment 2021/02/04 [IT6-38(機種マージ)] Sky東池 ジョブ削除設定(モード4設定時)の自動削除対象ジョブに認証指定なしジョブを含める要望.
			if( ( PnlComGetSoftDipSw()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW231, 0 ) )
			 && ( True == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_HistoryError ) )
			 && ( PM_TASKID_PANEL == priTask )
			 && (    nowGamen == &gp_AppChangeBrowser
			      || nowGamen == &gp_AuthOapBrowserLogin
			      || nowGamen == &gp_AppChangeScreenSaver
			      || nowGamen == &gp_AuthOapBrowserWait
			      || nowGamen == &gp_AuthOapBrowserWaitEx
			      || nowGamen == &gp_ScanOapBrowserDummy
			      || nowGamen == &gp_PullPrintOapBrowserDummy
			      || nowGamen == &gp_GeneralOapBrowserDummy
			      || nowGamen == &gp_CostCenterResetDummy
			      || nowGamen == &gp_UserSetupBrowser			// ユーザーセットアップ画面.				// comment 2022/01/28 Sky東島 [IT6_2.2MR2][制御34]ユーザーセットアップ.
			      || nowGamen == &gp_UserSetupBrowserWait		// ユーザーセットアップ切り替え中画面.		// comment 2022/01/28 Sky東島 [IT6_2.2MR2][制御34]ユーザーセットアップ.
			      || nowGamen == &gp_IwsAuthBrowserLogin
			      || nowGamen == &gp_IwsAuthBrowserWait ) ) {
				b_changeGamenFlg = True;
			}
// comment 2021/04/28 Sky中野 [IT6_2.2][OP_BTS-33065].
////comment 2020/01/30 SCD2-sky 延原[市場問題][TAS000000002475]/C308/紙詰まり発生時、警告ダイアログが表示されず警告音もならない.
//			if (  ( PM_TASKID_PANEL == priTask )							// 優先タスクがパネル.
//					&& ( True == isNowGamen( &gp_AuthOapBrowserLogin ) ) 		// 認証連携ブラウザログイン画面表示中.
//					&& ( nextGamen == &gp_PublicPopupDummyBase ) ) {			// 次の画面がダミー画面.
//				b_changeGamenFlg = True;
//			}
			if( ( PM_TASKID_PANEL == priTask )							// 優先タスクがパネル.
			 && ( ( True == isNowGamen( &gp_IwsAuthBrowserLogin ) )		// IWS認証画面表示中.
			   || ( True == isNowGamen( &gp_AuthOapBrowserLogin ) ) )	// 認証連携ブラウザログイン画面表示中.
			 && ( ( &gp_WarningPopupDummyBase == nextGamen )			// 遷移先がダミー画面.
			   || ( &gp_PublicPopupDummyBase == nextGamen )
			   || ( &gp_MenuPopupDummyBase == nextGamen ) ) ) {
				b_changeGamenFlg = True;
			}
		}

		if( changeGamen == nowGamen )	bl_SameGamenInstanceForVoiceStartGamen = True;

		// RA切断待ちの画面が遷移した場合は、待ち状態を解除する.
		if ( ( changeGamen != nowGamen ) && PNLC_TabletResetControlLib::newInstance()->isWaiting( nowGamen ) ) {
			PNLC_TabletResetControlLib::newInstance()->setStatus( nowGamen, PNLD_TABLETRESET_EXEC );
		}
//ideyama:仕組み追加
		beforeGamen = nowGamen;
//ideyama:仕組み追加
		nowGamen = changeGamen;	// 次画面切り換え

		// 画面遷移ログ保存.
		PNL_LogManager::newInstance()->changeGamenLog( this, nowGamen );

		// 標準・拡大の画面切替時.
		if ( bl_isPreEnlargeDisplayMode != onlyPanel.bl_isEnlargeDisplayMode ) {

			// 標準・拡大の画面情報を更新.
			bl_isPreEnlargeDisplayMode = onlyPanel.bl_isEnlargeDisplayMode;
			// マシン設定が右詰め表示言語の時はメッセージのロードしなおす
// comment 2015/06/29 OPS1-Sky佐藤 [ZeusS_ZX0][RQ-405] パネルUIのヘブライ語対応.
//			TYPE_Language ec_Type = PnlComGetMachineSetting()->getLanguage();
//			if ( TYPE_LanguageArabic == ec_Type ) {
//				TYPE_Language ec_GuiLang = PnlComGetLanguage();
//				PnlGuiWrapperSetGuiLanguage( ec_GuiLang );
//				loadSelectLangType_IOP( PnlComGetLanguage() );
//			}
// comment 2016/01/28 OPS1-Sky浜野 [ZeusS_ZX0][CQ524401] Myパネル対応.
//			TYPE_Language ec_Type;
//			if ( PnlComGetOperationSetting()->getLanguageChoiceSetting() ) {
//				ec_Type = onlyPanel.e_Language;
//			} else {
//				ec_Type = PnlComGetMachineSetting()->getLanguage();
//			}
//			if ( True == isRightDispLanguage( ec_Type ) ) {
//				loadSelectLangType_IOP( PnlComGetLanguage() );
//			}
			TYPE_Language e_CheckLanguage = PnlComGetLanguage( False );
// comment 2018/12/14 Sky毛穴 [EagleHigh][RQ-258] タイ語とベトナム語の対応.
//			if ( True == isRightDispLanguage( e_CheckLanguage ) ) {
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
				loadSelectLangType_IOP( PnlComGetLanguage() );
			}
		}
#ifdef SIMULATOR	// シミュレータで有効.
#ifndef LINUX
			c_fdShowPartsObserver.setSymbolInfoGamen( ( int32_t )nowGamen );
#endif	/* LINUX */
#endif	// #ifdef SIMULATOR.
			nowGamen->showEnableAllPars();	// 画面パーツを全て表示(showEnable(True))状態にする.
			allPanelInitialize();				// パネル初期化処理.
			nowGamen->resetParts();				// パーツのリセット.
			nowGamen->setDefaultBackParts();	// デフォルト背景表示部品の設定
			PNLC_PopupListControl::newInstance()->setBaseGamen( nowGamen );
			PnlComSetupHintMessageIconInit();	// ヒントアイコン表示カウント初期化.
			// 音声ガイド中で数値入力モードの場合は音声カーソルモードへ移行
			if( ::getVoiceCursorMode() == TYPE_VoiceGuide_NumInput ) { ::setVoiceCursorMode( TYPE_VoiceGuide_Normal ); }
			if( this != &gp_IGCClearWait && this != &gp_IGCWait ) { PNLC_VoiceCursorControl::newInstance()->setVoiceCursorKind(); }	// 遷移先画面のカーソル初期表示位置を設定
			PNLC_VoiceCursorControl::newInstance()->initCursor( NULL,NULL ); // 表示前の非表示対策
			nowGamen->setVoiceGamenInfo();		// 画面情報の設定
			nowGamen->checkPanelState();
// comment 2020/07/01 Sky菅野 ログ強化改善.
// comment 2022/02/18 Sky東島 [IT6_2.2MR2]パネルログ強化.
//			ExpPanelLog( PNLD_ExpLogKind_ScreenChange, "[LocalUI][BaseGamenTrans][PriTask:%d] %s -> %s", priTask, typeid( *beforeGamen ).name(), typeid( *nowGamen ).name(), 0, 0, 0 );
			ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[ChangeGamen][PriTask:%d] %s -> %s", priTask, typeid( *beforeGamen ).name(), typeid( *nowGamen ).name(), 0, 0, 0 );
			nowGamen->startGamen();				// 画面開始処理           　ここでnextGamenをセットすると表示とキーがずれてバグるので要注意
//comment 2019/9/17 [HCL_Kavitha] [EagleH][OP_BTS-13623] Corrected the priority task set.
			if( &gp_AuthOapBrowserWaitEx != nowGamen ) {
				prevGamen = NULL;
			}
			onlyPanel.bl_FromWarningChangeGamen = False;// 全画面警告画面からの遷移フラグをクリア.

			Kbd_ScanFaxKeybordConv.checkStatusUSBKeyboard();

			nowGamen->setupGamen();				// 表示部品のセットアップ　 ここでnextGamenをセットすると表示とキーがずれてバグるので要注意
#ifdef DEF_A4_Cus
			PNLC_SysPerformanceLogLib* pc_SysPerformanceLogLib = PNLC_SysPerformanceLogLib::newInstance();
			if ( NULL != pc_SysPerformanceLogLib ) {
				pc_SysPerformanceLogLib->setSysPerformanceLog( PNLE_SysPerformanceLog_ShowNormal );
			}
#endif	// DEF_A4_Cus.
// comment 2019/11/01 SCD2-Sky浜野 [EagleZ+][自動測定対応].
			// イベント記録.
			// 遷移前、後の画面が異なる場合.
			if( ( &gp_ActionInitialSetup == nowGamen ) && ( &gp_ActionInitialSetup != beforeGamen ) ) {
				APIC_SystemInfo *pc_APIC_SystemInfo = APIC_SystemInfo::newInstance();
				if( NULL != pc_APIC_SystemInfo ) {
					pc_APIC_SystemInfo->setEventRecord( TYPE_EventRecord_SwitchingScreenStartup );
				}
			}

			nowGamen->show();					// 画面パーツの表示
// comment 2020/09/04 Sky東島 [IT6_2.0MR3][OP_BTS-26713].
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
			if( 0 == PNLC_PopupListControl::newInstance()->getPopupNum() ){
				// startGamen等でポップアップを開く場合があるためポップアップが開いていない場合のみ更新を行う.
				// 画面更新後にペイン表示を更新させる.
				GFWC_Window*	window	= GFWC_System::getInstance()->getWindow();
				if ( NULL != window ) {
					window->repaint();
				}
				updateLcdPane( dynamic_cast<PBasicMessageProc*>( nowGamen ), True );	// ペイン更新
			}
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
// comment 2016/12/08 MSE岡田 [IT5_3.4][1008981] スライドメニュー表示判定要求通知を行う.
//			PNLC_SlideMenuControlLib::newInstance()->judgeShown();
// comment 2019/06/28 Sky東島 [EagleHigh][OP_BTS-9409] 描画完了通知変更.
			if( True != b_changeGamenFlg ) {
				PCGC_PanelMain::SlideMenuJudgeShownReqIndi( 0 );
			}

			// 画面切り替え手続き完了. for debug.
			PMGC_PanelManager::GamenChangeProcedureCompletion();

			if( b_changeGamenFlg )
			{
				if( PM_TASKID_BROWSER != priTask ) {
					QtPanel::newInstance()->activateQtPanel();
				}
// comment 2018/09/28(金) SCD2-Sky長谷川(伸) [市場問題対応][FQ-ISD2G-1800055-OFP161].
				else if ( PM_TASKID_BROWSER == priTask && True == isNowGamen( &gp_WarningPopupDummyBase) ) {
					QtPanel::newInstance()->activateQtPanel();
				}
// comment 2019/12/23 SCD2-sky 鷲見[TAS000000002061] サイドメニューからブラウザアプリを起動した後にキーボードから入力しても入力内容が反映されない.
				else if ( ( PM_TASKID_BROWSER == priTask || PM_TASKID_BRWPNL == priTask )
					&& ( ( True == isNowGamen( &gp_CopyBase ) ) ||
				         ( True == isNowGamen( &gp_CopyBaseDirect ) ) ||
				         ( True == isNowGamen( &gp_FromRegAddress_scanfax ) ) ||
				         ( True == isNowGamen( &gp_DirectInput_scanfax ) ) ||	
				         ( True == isNowGamen( &gp_AddressSearch_scanfax ) ) ||
				         ( True == isNowGamen( &gp_ScanFaxHistoryAddress ) ) ||
				         ( True == isNowGamen( &gp_DirectInputOffHook_scanfax ) ) ||
				         ( True == isNowGamen( &gp_HomeTopMenu ) )
				) ){
// comment 2023/09/22 Sky東島 [IT6_2.3][OP_BTS-44767].
#ifndef DEF_LCD_7INCH
						PNLC_SlideMenuControlLib* pc_SlideMenuControlLib = PNLC_SlideMenuControlLib::newInstance();
						if( NULL != pc_SlideMenuControlLib ){
							pc_SlideMenuControlLib->show();
						}
#endif // DEF_LCD_7INCH.
						QtPanel::newInstance()->activateQtPanel();					
				}
				// 描画完了通知設定をONにする.
// comment 2019/06/28 Sky東島 [EagleHigh][OP_BTS-9409] 描画完了通知変更.
//				PMGC_PanelManager::setDrawCompFlg( True );
				if( ( &gp_WarningPopupDummyBase != nowGamen ) 
				&&	( &gp_PublicPopupDummyBase != nowGamen )
				&&	( &gp_MenuPopupDummyBase != nowGamen ) )
				{
					PMGC_PanelManager::setDrawCompFlg( True );
				} else {
					onlyPanel.setDrawCompleteStatus( PNLE_DrawComplete_Wait );
				}
// comment 2015/04/30 OPS1-Sky佐藤 [HeliosL][CQ513137] ズームキーの登録キー割り当て対応.
				// 表示ズームの倍率を初期化.
				procInitZoomFactor();
			}
			QtPanel::newInstance()->initializedGraphic();//@@@ isshiki @@@
			if( 0 < nowGamen->getPartsNumber() ) {
				// 画面反転機能を有効にする.
				setDispColorReversePanel( True );
			}

			PNLC_VoiceCursorControl::newInstance()->initCursor(
			    nowGamen->getVoiceGamenInfo(),
			    nowGamen->getVoicePartsConvert()
			);// 画面情報の設定
// comment 2015/10/20 OPS1-Sky_nozaki [MinervaSBk][CQ520332] 音声ガイドの青枠が残っている問題対応.
			PNLC_VoiceCursorControl::newInstance()->forcehideCursor();		// 音声ガイド対象画面でなければカーソルを非表示にする.
			if( nowGamen != &gp_IGCClearWait && nowGamen != &gp_IGCWait ) {
				PNLE_VoiceCursorKind eVoiceCursorKind = PNLE_VCDefaultCursor;
				eVoiceCursorKind = nowGamen->getVoiceCursorKind();
				PNLC_VoiceCursorControl::newInstance()->setCurrentCursor( eVoiceCursorKind );
			}
			PNLC_VoiceCursorControl::newInstance()->showCursor();			// カーソル表示
			PNLC_VoiceRequestBuffer::newInstance()->clearMsgAreaRequest();	// メッセージエリア用バックアップをクリアする
			// 読込み中、印刷中画面に遷移する直前に再生要求をだした音声ファイルは中断しない.
			if( nowGamen != &gp_EnlAct_ActionScanPrint && nowGamen != &gp_EnlAct_ActionFax && nowGamen != &gp_EnlAct_ActionPrinting ) {
				::setVoiceRequestFlag( True );	// →ユーザー操作は現在の再生を中止する.
			}

			if( nowGamen != &gp_IGCClearWait && nowGamen != &gp_IGCWait ) { nowGamen->addReqStartVoiceID(); } // 画面開始時の音声を再生要求バッファへ格納
			onlyPanel.b_NextGamenMode = True;

			PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();
			if( NULL != pc_PopupListControl ) {
				pc_PopupListControl->checkAndAddReservedPopup();

				// ショートカット予約されている場合、予約されているポップアップを表示する。
				if( True == pc_PopupListControl->isShortCutPopup() ) {
					pc_PopupListControl->addShortCutPopup();
				}
			}
		}
// Comment 2018/01/17 [HCL-Prasanjeet] [IT5 4.2][AR1034498] update slider menu key after closed the slider menu screen..
		APIC_SystemSetting* pc_SystemSetting = PnlComGetSystemSetting();
		if( NULL != pc_SystemSetting ) {
			// 拡大表示時設定取得（スライドメニュー設定）.
			if( True == pc_SystemSetting->getSlideMenuEnlargeDisp() ) {
				if ( True == PNLC_SlideMenuControlLib::newInstance()->ChkSlideMenuOpenStatus() ) {
// comment 2021/08/30 Sky中野 [MinervaSSBK][OP_BTS-35244].
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
					if( GFWE_LCD_PANE_1 != GFWC_System::getInstance()->getLcdPane() ) {
						PNLC_SlideMenuControlLib::newInstance()->judgeShown();
					}
#else
				     PNLC_SlideMenuControlLib::newInstance()->judgeShown();
#endif
				}
			}
		}
// comment 2015/11/04 MSE田中優介 [ZeusS_ZX0][CQ520656] 拡大表示画面のメッセージ警告の表示タイミング変更.
		if ( True == onlyPanel.bl_isEnlargeDisplayMode ) {
			// 拡大表示画面の場合、メッセージ警告の表示タイミングを変更. 画面更新処理で確定したメッセージ警告の表示を行う.
			PNLC_EnlWarningManager::newInstance()->checkEnlMessageWarningPopup();
		} else {
			// 起動元画面の確定後に警告画面の起動要求をチェックし、要求があれば起動を行う.
			PNLC_WarningManager::newInstance()->checkMessageWarningPopup();
		}

// comment 2014/12/17 OPS1-Sky西原 [HeliosL][制御構成変更No.1]封筒通紙の操作性改善（定着手動離間）.
// システムオートリセットで離間警告を検知して警告画面をポップアップさせるためのパネル警告をセットするが、
// システムオートリセット時にパネル警告を同時にセットすると、リセット後の画面遷移のnextGamenと、
// パネル警告によるポップアップ要求が同時に発生してポップアップが解除されるため警告セットのタイミングを遅らせる.
// comment 2016/04/11 Sky蒲原 Poseidon封筒モード][CQ526533] 封筒モード警告表示タイミング変更.
//	if( True == onlyPanel.isFuserRetractedError() ){
//		onlyPanel.setPanelWarning( PNL_ERROR_FuserRetractedTypeErrorS );
//		onlyPanel.setFuserRetractedError( False );
//	}

		// このため、アニメーションを行っている画面でスタートキーLEDの点滅が実施されない.
		// アニメーションタイマーのメッセージでは、LEDのセットアップは行わない.
		if( MS_Animation2 != message ) {
			// トータルカウンタ抜けとメンテ中の点滅周期がおかしい。MS_TimerWakeUpが次の周期ではMS_Secondになる為、点滅周期が1000msとなっていた。 .
			nowGamen->setupLED( ( message == MS_TimerWakeUp || message == MS_Second ) ? True : False );	// ＬＥＤのセットアップ（定期起床(500ms)を点滅周期とする) .
		}
		// アプリ切り替えによるブザー制御.
		if( onlyPanel.bl_AppChangeFlag == True ) {
			// 警告発生によるアプリ切替の場合は、ブザー制御を警告に任せるため、鳴動させない.
			// (ブザーが鳴動しない警告もあるため).
			if( 0 == onlyPanel.isAllGamenWarning( onlyPanel.cc_DisplayJob ) ) {
				setBuzzerType( PCGE_BuzzerType_InputOk );
			}
		}
		if( ( onlyPanel.e_PanelLock != TYPE_PanelLock_Locked ) && ( onlyPanel.e_PanelLock != TYPE_PanelLock_Lock ) ) {
			if( True == isNeedBuzzer() ) {
				if( message == MS_PowerSave ) {
					sendBuzzerType_PowerSave();	// ブザー音要求.
				} else {
					sendBuzzerType();			// ブザー音要求.
				}
			}
		}

		Bool bl_ScreenTransition = PNLC_EnlVoiceGuidanceControl::getScreenTransition();
		if( NULL != nextGamen ) {
			bl_ScreenTransition = True;	//Full screen transition.
		} else if( sc_OldPoupCount != pc_PopupListControl->getPopupNum() ) {
			bl_ScreenTransition = True;	//Popup removed or added.
		} else if( pOldPopup != pc_PopupListControl->getNowPopup() ) {
			bl_ScreenTransition = True;	//one popup removed and another added.
		}

		PNLC_EnlVoiceGuidanceControl::setScreenTransition( False );

		if( bl_ScreenTransition ) {
			if( 0 == pc_PopupListControl->getPopupQueueCnt() ) {
				if( True == pc_PopupListControl->isPopupOpen() ) {
					if( True == onChangePopupGamenStartVoice( pc_PopupListControl->getNowPopup() ) ) {
						PNLC_EnlVoiceGuidanceControl::setCurrentVoicePlayGamen( pc_PopupListControl->getNowPopup() );
						PNLC_VoiceRequestBuffer::newInstance()->sendRequest();// 再生要求を発行
					} else {
						//Do Nothing.
					}
				} else {
					Bool bl_CallSendRequest = False;
					if( NULL != nextGamen ) {
						bl_CallSendRequest = checkForGamenStartVoice();
					} else {
						bl_CallSendRequest = onChangePopupGamenStartVoice( nowGamen );
					}

					if( True ==  bl_CallSendRequest ) {
						PNLC_EnlVoiceGuidanceControl::setCurrentVoicePlayGamen( nowGamen );
						PNLC_VoiceRequestBuffer::newInstance()->sendRequest();// 再生要求を発行
					} else {
						//Do Nothing.
					}
				}
			} else {
				//Do Nothing.
			}
		} else {
			PNLC_VoiceRequestBuffer::newInstance()->sendRequest();// 再生要求を発行
		}
		bl_SameGamenInstanceForVoiceStartGamen = False;
#if defined(SIMULATOR) && !defined(LINUX)
	}
#endif

	// bizhubRA接続／切断後のリセットの場合.
	if( MS_TabletReset == e_OriginalMessage ) {
// comment 2015/04/01 OPS1-Sky浜野 [Poseidon][CQ511887] bizhubRA順番待ちでの切断不具合対応.
//		APIC_AndroidPanel * pc_AndroidPanel = APIC_AndroidPanel::newInstance();
//		if( PNLE_TABLETRESET_REASON_CONNECT == option )
//		{
//			// 端末アンロック要求.
//			pc_AndroidPanel->req_PanelTabletLock( APIE_PanelTabletLockRequestKind_UnlockTablet );
//			PanelLog( PNLD_LogKind_Active, "APIC_AndroidPanel::req_PanelTabletLock[%d] Called.", APIE_PanelTabletLockRequestKind_UnlockTablet, 0, 0, 0, 0, 0 ); 
//		}
//		else
//		{
//			// パネルアンロック要求.
//			pc_AndroidPanel->req_PanelTabletLock( APIE_PanelTabletLockRequestKind_UnlockPanel );
//			PanelLog( PNLD_LogKind_Active, "APIC_AndroidPanel::req_PanelTabletLock[%d] Called.", APIE_PanelTabletLockRequestKind_UnlockPanel, 0, 0, 0, 0, 0 );
//		}
		PNLC_TabletResetControlLib* pc_TabetResetCtrl = PNLC_TabletResetControlLib::newInstance();
		if( NULL != pc_TabetResetCtrl ) {
			// bizhubRAの終了処理.
			pc_TabetResetCtrl->TabletEndProcess( (PNLE_TABLETRESET_REASON)option );
			
			// 処理中の接続/切断通知での終了処理.
			PNLE_TABLETRESET_REASON e_Reason = pc_TabetResetCtrl->getReason_UnlockOnly();
			pc_TabetResetCtrl->TabletEndProcess( e_Reason );
			
			pc_TabetResetCtrl->setReason_UnlockOnly( PNLE_TABLETRESET_REASON_NONE );
		}
	}
	
// comment 2018/04/20 SCD2 Yoshinori.Yamabe[Eagle]制御構成変更_No.111 ホーム画面認証画面の新UI化対応.
	PNLC_WebUIControl::getInstance()->clearPreUIMode();
// comment : WebUI2 画面切替え制御 -> 20180302 Chikuan.
	// 新UIモード用にブラウザの再起動が必要な場合、再起動を実施する.
// comment 2018/12/26 SCD2-Sky小畑 [EagleH] WebUIv3FUM対応.
//	if( True == PnlComIsEnableBrowserLightUIModeReboot() ){
//		PnlComBrowserLightUIModeReboot();
//		PNLC_WebUIDebug::getInstance()->outputDebugLog( "---- WebUI Debug : PBasicMessageProc::messageProc : PnlComBrowserLightUIModeReboot called ----.\n" );
//	}
// comment : WebUI2 画面切替え制御 <- 20180302 Chikuan.
	
// comment 2019/02/26 SCD2-Sky小畑 [WebUIv3][AR1073557] 起動時のタブ初期化でNginxサーバやEventModeuleの起動状態を判定する対応.
	if( True == PNLC_BrowserChangeTabControl::getInstance()->isNeedInitBrowserTab() ){
		if ( PNLE_BrowserChangeTabControlTabInitStatus_Done == PNLC_BrowserChangeTabControl::getInstance()->getTabInitStatus() ) {
			// ユーザ画面タブの制御情報を初期化.
			PNLC_WebUIControl::getInstance()->initData();
			PNLC_WebUIMessageConvert::getInstance()->initData();
			PNLC_WebUIScreenInfo::getInstance()->initData();
			// ブラウザタブ切替制御情報を初期化.
			PNLC_BrowserChangeTabControl::getInstance()->initData();
		}
		// ブラウザタブ初期化.
		PNLC_BrowserChangeTabControl::getInstance()->initBrowserTab();
	}
// comment 2019/05/15 Chikuan [EagleH][OP_BTS-1792] V1遷移パフォーマンス改善.
	if( True == PNLC_BrowserChangeTabControl::getInstance()->isNeedInitAdminTab() ){
		PNLC_BrowserChangeTabControl::getInstance()->initAdminTab();
	}

	nextGamen = NULL;
	return	changeGamen;	// 切り替えた画面に制御を切り替える.
}
//==============================================================================
//概要     : ポップアップ有無による、Pushメッセージ振り分け処理.
//==============================================================================
Bool PBasicMessageProc::isPushedDispatch( short x, short y, Message& message, uchar& option, Bool& buzzer )
{
	PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();	// ポップアップリスト管理クラスのインスタンスを取得.
	if( NULL != pc_PopupListControl && True == pc_PopupListControl->isPopupOpen() ) {
		return pc_PopupListControl->isPushedPopup( x, y, message, option, buzzer );		// ポップアップ起動中は、ポップアップの押下判定を呼び出す.
	} else {
		return nowGamen->isPushed( x, y, message, option, buzzer );			// ポップアップが起動していない場合は、自インスタンスの押下判定を呼び出す.
	}
}
//==============================================================================
//概要     : ポップアップ有無による、Dragメッセージ振り分け処理.
//==============================================================================
Bool PBasicMessageProc::isDragDispatch( short x, short y, Message& message, uchar& option, Bool& buzzer )
{
	PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();	// ポップアップリスト管理クラスのインスタンスを取得.
	if( NULL != pc_PopupListControl && True == pc_PopupListControl->isPopupOpen() ) {
		return pc_PopupListControl->isDragPopup( x, y, message, option, buzzer );		// ポップアップ起動中は、ポップアップの押下判定を呼び出す.
	} else {
		return nowGamen->isDrag( x, y, message, option, buzzer );				// ポップアップが起動していない場合は、自インスタンスの押下判定を呼び出す.
	}
}
//==============================================================================
//概要     : ポップアップ有無による、Releaseメッセージ振り分け処理.
//==============================================================================
Bool PBasicMessageProc::isReleasedDispatch( short x, short y, Message& message, uchar& option, Bool& buzzer )
{
	PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();	// ポップアップリスト管理クラスのインスタンスを取得.
	if( NULL != pc_PopupListControl && True == pc_PopupListControl->isPopupOpen() ) {
		return pc_PopupListControl->isReleasedPopup( x, y, message, option, buzzer );	// ポップアップ起動中は、ポップアップの押下判定を呼び出す.
	} else {
		return nowGamen->isReleased( x, y, message, option, buzzer );			// ポップアップが起動していない場合は、自インスタンスの押下判定を呼び出す.
	}
}
/*================================================================================================
［機能］	押されたかどうかの判定を行う
================================================================================================*/
Bool PBasicMessageProc::isPushed( short x, short y, Message& message, uchar& option, Bool& buzzer )
{
//ideyama:仕組み追加・・・検索順を逆にした.
	for( register int i = partsNumber-1; i >= 0; i-- ) {
		if( partsTable[i]->isNowShown() == False ) {
			continue;
		}
		if( partsTable[i]->isPushed( x, y, message, option, buzzer ) == True ) {
			setActiveParts( partsTable[i] );
			return True;
		}
	}
	return False;
}
/*================================================================================================
［機能］	ドラッグ中かどうかの判定を行う.
================================================================================================*/
Bool PBasicMessageProc::isDrag( short x, short y, Message& message, uchar& option, Bool& buzzer )
{
	for( register int i = 0; i < partsNumber; i++ ) {
		if( partsTable[i]->isDrag( x, y, message, option, buzzer ) == True ) {
			setActiveParts( partsTable[i] );
			return True;
		}
	}
	return False;
}
/*================================================================================================
［機能］	離されたかどうかの判定を行う.
================================================================================================*/
Bool PBasicMessageProc::isReleased( short x, short y, Message& message, uchar& option, Bool& buzzer )
{
	for( register int i = 0; i < partsNumber; i++ ) {
		if( partsTable[i]->isReleased( x, y, message, option, buzzer ) == True ) {
			setActiveParts( partsTable[i] );
			return True;
		}
	}
	return False;
}
/*================================================================================================
［機能］	基本メッセージ処理 : プリントキーＬＥＤの制御
================================================================================================*/
void PBasicMessageProc::printLEDControl( Bool setOk, EP_LED_BlinkMode _ep_LED_BlinkMode )
{
	bl_PrintLEDGreen = setOk;	// プリントキー・緑
	if( ppc_MechaCounter->isTotalCounterSet() == False ) {	// トータルカウンタ無し
		// トータルカウンタ引き抜き時は、強制的に赤にする
		bl_PrintLEDGreen = False;										// プリントキー・赤
		_ep_LED_BlinkMode = EP_LED_BlinkMode_Normal;					// プリントキー・通常点滅
	} else if( cp_RdsSetting->getMaintStat() == True ) {					// サービスメンテ中
		_ep_LED_BlinkMode = EP_LED_BlinkMode_Slow;						// プリントキー・ゆっくり点滅
	}
	// ポップアップアニメーション中はStartキー無効(LED:赤)に設定.
	if( ( NULL != PNLC_PopupListControl::newInstance()->getNowPopup() ) && ( True == PNLC_PopupListControl::newInstance()->getNowPopup()->isNowPopupAnimation() ) )
	{
		bl_PrintLEDGreen = False;
	}
// スライドアニメーション中はStartキー無効(LED:赤)に設定.
	if( True == m_bPopupAnimeAct ) { bl_PrintLEDGreen = False; }
	if( NULL != cp_reserveCopyJob ) {
		// コピー予約時退避ジョブが有効.
		if( APIC_JobInstanceList::isExistJob( cp_reserveCopyJob ) == False ) {
			// ジョブが終了していたら.
			cp_reserveCopyJob = NULL;
		} else if( TYPE_InputStatusScanExec == cp_reserveCopyJob->getJobStatus()->getInputStatus()
		           || True == onlyPanel.ct_Copying1Sec.isBusy()
		           || ( TYPE_OutputStatusDeleting == cp_reserveCopyJob->getJobStatus()->getOutputStatus()
		                && ( TYPE_InputStatusScanStop == cp_reserveCopyJob->getJobStatus()->getInputStatus()
		                     || TYPE_InputStatusScanErrorStop == cp_reserveCopyJob->getJobStatus()->getInputStatus()
		                     || TYPE_InputStatusMemoryInScanStop == cp_reserveCopyJob->getJobStatus()->getInputStatus() ) ) ) {
			// 読込み動作中、削除中であれば赤にする.
			bl_PrintLEDGreen = False;										// プリントキー・赤
		}
	}
// 割込み 読み込み中にジョブ削除指示した場合　ADFからの排紙までスタートLEDを赤にする.
	if( NULL != onlyPanel.cp_IntModeDeleteCopyJob ) {
		if( APIC_JobInstanceList::isExistJob( onlyPanel.cp_IntModeDeleteCopyJob ) == False ) {
			// ジョブが終了していたら.
			onlyPanel.cp_IntModeDeleteCopyJob = NULL;
		} else if( TYPE_InputStatusScanExec == onlyPanel.cp_IntModeDeleteCopyJob->getJobStatus()->getInputStatus()
		           || True == onlyPanel.ct_Copying1Sec.isBusy()
		           || ( TYPE_OutputStatusDeleting == onlyPanel.cp_IntModeDeleteCopyJob->getJobStatus()->getOutputStatus()
		                && ( TYPE_InputStatusScanStop == onlyPanel.cp_IntModeDeleteCopyJob->getJobStatus()->getInputStatus()
		                     || TYPE_InputStatusScanErrorStop == onlyPanel.cp_IntModeDeleteCopyJob->getJobStatus()->getInputStatus()
		                     || TYPE_InputStatusMemoryInScanStop == onlyPanel.cp_IntModeDeleteCopyJob->getJobStatus()->getInputStatus() ) ) ) {
			// 読込み動作中、削除中であれば赤にする.
			bl_PrintLEDGreen = False;										// プリントキー・赤
		}
	}
	ep_LED_BlinkMode = _ep_LED_BlinkMode;		// プリントキー・点滅モード
}
//================================================================================================
//［機能］	基本メッセージ処理 : 画面開始.
//  画面開始時に呼ばれるハンドラ.
//  画面再配置(デフォルト以外指定)を実行するよう処理追加.
//================================================================================================
void PBasicMessageProc::preStartGamen() {
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
	updateLcdPane( dynamic_cast<PBasicMessageProc*>( this ) );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !defined (DEF_HeliosMLKCus) )	// Eagle/SparrowMFP 10.1inch.

	relayout( False );

	short partsNumber			= getPartsNumber();
	PDisplayParts** partsTable	= getPartsTable();
	
	PPopupCom* popupGamen			= dynamic_cast<PPopupCom*>( this );
	PBaseWidget* newWidget			= dynamic_cast<PBaseWidget*>( m_pc_displayParts );

	// Check GamenCacheList
	Bool useCache = isUseCacheGamen();

	if( popupGamen != 0 )
	{	
		// ポップアップの場合キャッシュ対象外.
	}
	else
	{	// ベース画面の場合
		if( newWidget == 0 )
		{	// ベース画面ウィジェット未生成の場合
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
// comment 2018/10/12 Sky東島 [1062683] 2ペイン化対応.
//			if ( GFWE_LCD_PANE_1 == getGamenPane( (PBasicMessageProc*)this ) ) {
			GFWE_LCD_PANE e_LCDPane = getGamenPane( (PBasicMessageProc*)this );
			if( ( GFWE_LCD_PANE_1 == e_LCDPane )
			 || ( GFWE_LCD_PANE_2 == e_LCDPane )
			 || ( GFWE_LCD_PANE_1_Extention == e_LCDPane ) ) {
				// １ペイン,2ペイン,1ペイン(拡大表示)の場合.
				newWidget = new PBaseWidget(
				    m00_076_000_X1024_Y600		// const uchar* puc_Parts
					, 0							// short tx
					, 0							// short gy
					, 132						// uchar uc_PaletteType
					, TYPE_LayoutFree			// TYPE_Layout e_LayoutX = TYPE_LayoutFree
					, False						// Bool centeringY = False
					, 0							// int red = 0
					, 0							// int green = 0
					, 0							// int blue = 0
					, 0							// int alpha = 0
					, GFWE_LCD_Size_10_1inch	// GFWE_LCD_Size e_Pos = GFWE_LCD_Size_9inch
					, GFWE_LCD_Size_10_1inch	// GFWE_LCD_Size e_Size = GFWE_LCD_Size_9inch )
				);
				newWidget->startGUI( 0 );

				// グレーアウト用画面生成＆非表示
				if( m_pc_grayout == 0 ) {
					m_pc_grayout = new PGrayoutWidget(
					    m00_076_000_X1024_Y600		// const uchar* puc_Parts
						, 0							// short tx
						, 0							// short gy
						, 132						// uchar uc_PaletteType
						, TYPE_LayoutFree			// TYPE_Layout e_LayoutX = TYPE_LayoutFree
						, False						// Bool centeringY = False 
						, 0							// int red = 0
						, 0							// int green = 0
						, 0							// int blue = 0
						, 128						// int alpha = 128
						, GFWE_LCD_Size_10_1inch	// GFWE_LCD_Size e_Pos = GFWE_LCD_Size_9inch
						, GFWE_LCD_Size_10_1inch	// GFWE_LCD_Size e_Size = GFWE_LCD_Size_9inch
					);
				}
			} else {
				// １ペイン以外の場合.
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !defined (DEF_HeliosMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			const uchar* puc_BaseParts = NULL;
			//パーツテーブルにパーツが存在しない場合は、ベースウィジェットのパーツを透明パーツにする.
			if( partsNumber > 0 ) {
				puc_BaseParts = c71_000_000_t1_X800_Y480;
			} else {
				puc_BaseParts = Transparent_Parts_X800_Y480;
			}
			newWidget = new PBaseWidget(
			    puc_BaseParts
			    , 0
			    , 0
			    , 132
			    , TYPE_LayoutFree
			    , False
			);
			newWidget->startGUI( 0 );

			// グレーアウト用画面生成＆非表示
			if( m_pc_grayout == 0 ) {
				m_pc_grayout = new PGrayoutWidget(
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/09/10 SCD2 K.Takei 不具合No.1053638_【開発提起】1ペイン/3ペインの切り替え時、3ペインエリアとメインエリアの表示タイミングがずれており、見栄えが悪いので改善する.
					m00_076_000_X1024_Y600		// const uchar* puc_Parts
#else // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.
					c71_000_000_t1_X800_Y480
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.
				    , 0
				    , 0
				    , 132
				    , TYPE_LayoutFree
				    , False
				);
			}
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
			}
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			
			useCache = True; // ベース画面の場合イメージキャッシュを強制的に有効にする.
		}

		m_pc_grayout->startGUI( newWidget );
		m_pc_grayout->hide();

	}
	m_pc_displayParts = newWidget;

	_SHOW_PARTS_START();
	for( int i = 0; i < partsNumber; i++ )
	{
		_SHOW_PARTS_INFO( partsTable[i], i );
		partsTable[i]->preStartGUI( m_pc_displayParts );
		_SHOW_PARTS_GFWINFO( partsTable[i] );
		partsTable[i]->setUseCache( useCache );
	}
	_SHOW_PARTS_END();

	// 起動時の解像度情報を設定.
	GFWC_Window*	window	= GFWC_System::getInstance()->getWindow();
	if ( 0 != window ) {
		GFWC_Size	resolution	= window->getDrawLcdSize();
		
		m_us_PrevResolutionX = ( short )resolution.width();
		m_us_PrevResolutionY = ( short )resolution.height();
	}
}
//================================================================================================
//［機能］	基本メッセージ処理 : 画面開始.
//  画面開始時に呼ばれるハンドラ.
//  画面再配置(デフォルト以外指定)を実行するよう処理追加.
//================================================================================================
void PBasicMessageProc::startGamen() {
// comment 2018/12/14 Sky毛穴 [EagleHigh][RQ-258] タイ語とベトナム語の対応.
	TYPE_Language e_CheckLanguage = PnlComGetLanguage( False );
	if (   TYPE_LanguageThai 	== e_CheckLanguage			// タイ語.
		|| TYPE_LanguageVietnam == e_CheckLanguage			// ベトナム語.
// comment 2022/11/9 Sky奥田 [RQ-938] バルト三国対応.
		|| TYPE_LanguageEstonian	== e_CheckLanguage		// エストニア語.
		|| TYPE_LanguageLatvian		== e_CheckLanguage		// ラトビア語.
		|| TYPE_LanguageLithuanian	== e_CheckLanguage		// リトアニア語.
// comment 2023/2/6 Sky小熊 [IT6_2.3][RQ-970]Welsh as standard language for IT.
		|| TYPE_LanguageWelsh		== e_CheckLanguage		// ウェールズ語.
		) {
		loadSelectLangType_IOP( PnlComGetLanguage() );
	}
// comment 2018/07/30 SCD2 K.Takei 不具合No.1049643【PSWCでユーザー設定＞表示言語選択 =Englishを設定して、中間認証を管理して、パネルで文字化け表示する.
	PNLC_PanelDispMachineSettingLanguage::getInstance()->updateMachineSettingLanguage();
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
// comment 2018/09/10 SCD2 K.Takei 不具合No.1053638_【開発提起】1ペイン/3ペインの切り替え時、3ペインエリアとメインエリアの表示タイミングがずれており、見栄えが悪いので改善する.
// comment 2020/09/04 Sky東島 [IT6_2.0MR3][OP_BTS-26713].
//	updateLcdPane( dynamic_cast<PBasicMessageProc*>( this ), True );	// ペイン更新
	// 1ペインのベース画面を表示する場合は、このタイミングでフッターと右エリアの更新を行わないようにする.
	// 3ペインから1ペインに遷移する場合、このタイミングでフッターと右エリアを非表示にすると、画面裏のブラウザが表示されてしまうため.
	// nowGamen->show()の後にペインの更新を行う.
	Bool bl_dispPane = False;
	if( NULL != dynamic_cast<PPopupCom*>( this )
	 || GFWE_LCD_PANE_1 != getGamenPane( this ) ){
		bl_dispPane = True;
	}
	updateLcdPane( dynamic_cast<PBasicMessageProc*>( this ), bl_dispPane );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
// comment 2020/04/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
	PnlComFooterPartsBtnActiveChange(True, True, True);
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.

	relayout( False );  // CQ.360412 [MTUIv1]BOXプレビュー詳細Wide対応.

	short partsNumber			= getPartsNumber();
	PDisplayParts** partsTable	= getPartsTable();

	PNLC_PopupListControl* plCnt	= PNLC_PopupListControl::newInstance();
	PBasicMessageProc* baseGamen	= plCnt->getBaseGamen();
	PPopupCom* popupGamen			= dynamic_cast<PPopupCom*>( this );
	PBaseWidget* newWidget			= dynamic_cast<PBaseWidget*>( m_pc_displayParts );

// comment 2016/09/12 MSE寺田 [IT5_3.3][課題1003653] CQ509132の検討漏れ対応。startGamenでも内部Key押下状態をクリアする.
	// 画面開始時に、タッチパネル押下状態をクリア.
	PMGC_PanelManager::resetTouchPanelPosition();

	// 描画解像度が前回表示時から変更されている場合、新解像度でのパーツ再生成が必要であるため画面が保持するパーツを一旦破棄.
	GFWC_Window*	window	= GFWC_System::getInstance()->getWindow();
	if ( 0 != window ) {
		GFWC_Size	resolution	= window->getDrawLcdSize();
		
		if ( ( m_us_PrevResolutionX != resolution.width() ) || ( m_us_PrevResolutionY != resolution.height() ) ) {
			for ( int i = 0; i < partsNumber; i++ ) {
				partsTable[i]->endGUI();
			}
			m_us_PrevResolutionX	= ( short )resolution.width();
			m_us_PrevResolutionY	= ( short )resolution.height();
			if ( 0 == popupGamen ) {
				// ベース画面の場合、ベース画面の親となる画面ウィジェットも再生成必要であるため、画面ウィジェットも破棄.
				if ( 0 != m_pc_grayout ) {
					m_pc_grayout->endGUI();
					delete m_pc_grayout;
					m_pc_grayout	= 0;
				}
				if ( 0 != m_pc_displayParts ) {
					m_pc_displayParts->endGUI();
					delete m_pc_displayParts;
					m_pc_displayParts	= 0;
				}
				newWidget	= 0;
			}
		}
	}

	// Check GamenCacheList
	Bool useCache = isUseCacheGamen();

	if( popupGamen != 0 )
	{	// ポップアップの場合
		// ポップアップに関しては、アニメーションを伴うものは
		//  PPopupCom::opened→(アニメーション)→PLayerControl::startGamen→PBasicMessageProc::show
		// の順で呼ばれる。
		// アニメーション開始時点でグレーアウトする必要があるため、
		// 本処理より前にウィジェットが生成されている可能性がある
		if( newWidget == 0 )
		{	// ポップアップ画面ウィジェット未生成の場合
			// ポップアップ画面ウィジェット生成
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
// comment 2018/10/12 Sky東島 [1062683] 2ペイン化対応.
//			if ( GFWE_LCD_PANE_1 == getGamenPane( (PBasicMessageProc*)this ) ) {
			GFWE_LCD_PANE e_LCDPane = getGamenPane( (PBasicMessageProc*)this );
			if( ( GFWE_LCD_PANE_1 == e_LCDPane )
			 || ( GFWE_LCD_PANE_2 == e_LCDPane )
			 || ( GFWE_LCD_PANE_1_Extention == e_LCDPane ) ) {
				// １ペイン,2ペイン,1ペイン(拡大表示)の場合.
				newWidget = new PPopupWidget(
				    m00_076_000_X1024_Y600
					, 0							// short tx
					, 0							// short gy
					, 132						// uchar uc_PaletteType
					, TYPE_LayoutFree			// TYPE_Layout e_LayoutX = TYPE_LayoutFree
					, False						// Bool centeringY = False
					, 0							// int red = 0
					, 0							// int green = 0
					, 0							// int blue = 0
// comment 2018/08/10 SCD2 T.Hori [Eagle/SparrowMFP][UD評価課題対応 No.17] 認証画面プルダウン表示時、グレーアウト背景が出る.
					//透過パラメータ修正.
					, 0							// int alpha = 0
					, GFWE_LCD_Size_10_1inch	// GFWE_LCD_Size e_Pos = GFWE_LCD_Size_9inch
					, GFWE_LCD_Size_10_1inch	// GFWE_LCD_Size e_Size = GFWE_LCD_Size_9inch
				);

				// 親はベース画面ウィジェット(ポップアップリストの先頭)
				newWidget->startGUI( baseGamen->getDisplayParts(), ( 1024 ), ( 600 ) );	// 全画面を指定
			  } else {
				// １ペイン以外の場合.
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			newWidget = new PPopupWidget(
			    c71_000_000_t1_X800_Y480
			    , 0
			    , 0
			    , 132
			    , TYPE_LayoutFree
			    , False
			);

			// 親はベース画面ウィジェット(ポップアップリストの先頭)
			newWidget->startGUI( baseGamen->getDisplayParts(), ( 800 ), ( 480 ) );	// 全画面を指定
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
			}
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
		}
		else
		{	// ポップアップ画面ウィジェット生成済の場合
			// 生成済みのウィジェットに対して以降の処理を行う
			newWidget = dynamic_cast<PPopupWidget*>( getDisplayParts() );
		}

	}
	else
	{	// ベース画面の場合
		if( newWidget == 0 )
		{	// ベース画面ウィジェット未生成の場合
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
// comment 2018/10/12 Sky東島 [1062683] 2ペイン化対応.
//			if ( GFWE_LCD_PANE_1 == getGamenPane( (PBasicMessageProc*)this ) ) {
			GFWE_LCD_PANE e_LCDPane = getGamenPane( (PBasicMessageProc*)this );
			if( ( GFWE_LCD_PANE_1 == e_LCDPane )
			 || ( GFWE_LCD_PANE_2 == e_LCDPane )
			 || ( GFWE_LCD_PANE_1_Extention == e_LCDPane ) ) {
				// １ペイン,2ペイン,1ペイン(拡大表示)の場合.
				newWidget = new PBaseWidget(
				    m00_076_000_X1024_Y600		// const uchar* puc_Parts
					, 0							// short tx
					, 0							// short gy
					, 132						// uchar uc_PaletteType
					, TYPE_LayoutFree			// TYPE_Layout e_LayoutX = TYPE_LayoutFree
					, False						// Bool centeringY = False
					, 0							// int red = 0
					, 0							// int green = 0
					, 0							// int blue = 0
					, 0							// int alpha = 0
					, GFWE_LCD_Size_10_1inch	// GFWE_LCD_Size e_Pos = GFWE_LCD_Size_9inch
					, GFWE_LCD_Size_10_1inch	// GFWE_LCD_Size e_Size = GFWE_LCD_Size_9inch )
				);
				newWidget->startGUI( 0 );
				
				if( m_pc_grayout == 0 ) {
					m_pc_grayout = new PGrayoutWidget(
					    m00_076_000_X1024_Y600		// const uchar* puc_Parts
						, 0							// short tx
						, 0							// short gy
						, 132						// uchar uc_PaletteType
						, TYPE_LayoutFree			// TYPE_Layout e_LayoutX = TYPE_LayoutFree
						, False						// Bool centeringY = False 
						, 0							// int red = 0
						, 0							// int green = 0
						, 0							// int blue = 0
						, 128						// int alpha = 128
						, GFWE_LCD_Size_10_1inch	// GFWE_LCD_Size e_Pos = GFWE_LCD_Size_9inch
						, GFWE_LCD_Size_10_1inch	// GFWE_LCD_Size e_Size = GFWE_LCD_Size_9inch
					);
				}
			} else {
				// １ペイン以外の場合.
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			const uchar* puc_BaseParts = NULL;
			//パーツテーブルにパーツが存在しない場合は、ベースウィジェットのパーツを透明パーツにする.
			if( partsNumber > 0 ) {
				puc_BaseParts = c71_000_000_t1_X800_Y480;
			} else {
				puc_BaseParts = Transparent_Parts_X800_Y480;
			}
			newWidget = new PBaseWidget(
//										   c71_000_000_t1_X800_Y480
			    puc_BaseParts
			    , 0
			    , 0
			    , 132
			    , TYPE_LayoutFree
			    , False
			);
			newWidget->startGUI( 0 );

			// グレーアウト用画面生成＆非表示.
			if( m_pc_grayout == 0 ) {
				m_pc_grayout = new PGrayoutWidget(
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/09/10 SCD2 K.Takei 不具合No.1053638_【開発提起】1ペイン/3ペインの切り替え時、3ペインエリアとメインエリアの表示タイミングがずれており、見栄えが悪いので改善する.
					m00_076_000_X1024_Y600		// const uchar* puc_Parts
#else // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.
					c71_000_000_t1_X800_Y480
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.
			    	, 0
			    	, 0
			    	, 132
			    	, TYPE_LayoutFree
			    	, False
				);
			}
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
			}
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			
			useCache = True; // ベース画面の場合イメージキャッシュを強制的に有効にする.
		}

		m_pc_grayout->startGUI( newWidget );
		m_pc_grayout->hide();

	}

	m_pc_displayParts = newWidget;

	if(useCache == True) {
			//ブラウザ画面は一度作ったら表示されっぱなしのため、キャッシュ画面はブラウザ画面の前面に出す必要なり.
			if( newWidget != NULL )
			{
				newWidget->bringToFront();
			}
	}

	if ( useCache == False ) {
		// キャッシュ対象の応用設定画面か確認.
		TYPE_AppliedSettingDisp e_AppSettingDisp = PnlComGetOperationSetting()->getAppliedSettingDisp();
		int func_num = sizeof(g_FuncBadgeCacheList) / sizeof(PNLS_FuncBadge_Cache);
		for ( int jcnt = 0 ; jcnt < func_num ; jcnt++ ) {
			if ( (this == g_FuncBadgeCacheList[jcnt].pc_PopGamen) 
				&& (e_AppSettingDisp == g_FuncBadgeCacheList[jcnt].e_AppSetDisp) ) {
				if( newWidget != NULL ) {
					newWidget->bringToFront();
				}
				g_FuncBadgeCacheList[jcnt].b_Start = True;
				useCache = True;
				break;
			}
		}
	}

	_SHOW_PARTS_START();
	for( int i = 0; i < partsNumber; i++ )
	{
		_SHOW_PARTS_INFO( partsTable[i], i );
		partsTable[i]->startGUI( m_pc_displayParts );
		_SHOW_PARTS_GFWINFO( partsTable[i] );
		partsTable[i]->setUseCache( useCache );
	}
	_SHOW_PARTS_END();
//comment 2023/04/18 FPT.HungDM17 【IT6_2.3】【ALL MACHINE】【OP_BTS-42606】Fix coordinate of Maintenance Mode Status Bar depend on resolution of screen.
#ifdef DEF_LCD_7INCH
	if( 0 == m_pc_MaintenanceModeStatusBar )
	{
		m_pc_MaintenanceModeStatusBar = &Gra_MaintenanceModeStatusBar;
	}
#else
	if ( True == PNLC_SlideMenuControlLib::newInstance()->isShown() ){
		bl_JudgeStatusMaintenanceBar = True;
	}
	judgeStatusMaintenanceBar();
#endif
	showStatusMaintenanceBar();

	// メンテナンスモード中に表示するパーツ（赤バー）を生成.
//	if( NULL != m_pc_MaintenanceModeStatusBar ) {
//		m_pc_MaintenanceModeStatusBar->startGUI( 0 );
//		m_pc_MaintenanceModeStatusBar->showEnable( True );

// comment 2015/12/07 MSE天木 [ZeusS_ZX0][CQ521804] メンテナンスモード中のステータスバー表示条件の修正.
		// メンテナンスモード中か判断し、メンテナンスモード中なら赤バーを表示する.
		//APIC_SystemSetting* pc_SystemSetting = APIC_SystemSetting::newInstance();
		//APIC_SystemInfo*     pc_SystemInfo    = APIC_SystemInfo::newInstance();


		//if( ( 0 != pc_SystemSetting ) && ( 0 != pc_SystemInfo ) )
		//	Bool bl_AllowMaintenanceMode  = pc_SystemSetting->getAllowMaintenanceMode();	//メンテナンスモード許可する/許可しない.
		//	Bool bl_MaintenanceModeEnable = pc_SystemInfo->getEnableMaintenanceMode();		//メンテナンスモード有効/無効.
		//
		//	if( True == bl_AllowMaintenanceMode )
		//	{
		//		// メンテナンスモード許可のとき.
		//		if( True == bl_MaintenanceModeEnable )
		//		{
		//			m_pc_MaintenanceModeStatusBar->show();		// メンテナンスモード有効時は表示する.
		//		}
		//		else
		//		{
		//			m_pc_MaintenanceModeStatusBar->hide();		// メンテナンスモード無効時は表示しない.
		//		}
		//	}

//		if ( True == PnlComGetSystemInfo()->getEnableMaintenanceMode() ) {
//			m_pc_MaintenanceModeStatusBar->show();		// メンテナンスモード有効時は表示する.
//		} else {
//			m_pc_MaintenanceModeStatusBar->hide();		// メンテナンスモード無効時は表示しない.
//		}
//	}

	// ソフトウェアDipSW No.177のbit0が立っている場合は
	// 文字切れ検出機能を有効にする
	if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW_GFWDEBUG, 0 ) ) {
		GFWC_System::getInstance()->setTextFrame( GFWD_TRUE );
	} else {
		GFWC_System::getInstance()->setTextFrame( GFWD_FALSE );
	}

	// ソフトウェアDipSW No.177のbit1が立っている場合は
	// パーツはみだし検出機能を有効にする
	if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW_GFWDEBUG, 1 ) ) {
		if(  NULL != newWidget ) {
			newWidget->traceFamilyInfo();
		}
	}

	// ソフトウェアDipSW No.177のbit2が立っている場合は
	// 文字領域検出機能を有効にする
	if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW_GFWDEBUG, 2 ) ) {
		GFWC_System::getInstance()->setTextAreaFrame( GFWD_TRUE );
	} else {
		GFWC_System::getInstance()->setTextAreaFrame( GFWD_FALSE );
	}	
	
	
	// キーリピートデータ一括設定呼び出し.
// comment 2014/11/20 【HCL-Durga】【CQ-503465】Klockwork Changes ID: 24123 [NPD.CHECK.MUST] added NULL check condition.
	if(  NULL != newWidget ) {
	newWidget->setAllKeyRepeatCondition();

	}
	
// comment 2016/06/07 MSE大西 [ZeusSBK][CQ527296] ボタンオブザーバ通知追加.
	// ボタンイベントキュー初期化.
	PButtonEventManager::getInstance().initialize();
//Comment [2019/03/08] [HCL_Devipriya] [EagleH] [機器構成変更No.29]  use pointer instead of table.				
#if defined( DEF_EaglePanel_Hi ) || defined( DEF_EaglePanel_Low ) || defined( DEF_EagleCus ) 
			if( True == PnlComIsEagleHiModel() ){
				mp_PrtIrWarningList = &PrtIrWarningList_EglH[0];
			} else {
				mp_PrtIrWarningList = &PrtIrWarningList[0];
			}
#else
			mp_PrtIrWarningList = &PrtIrWarningList[0];
#endif	
		
// Comment 2016/12/20 第2システム制御開発 Sky上野 [ServiceHub] SHRQ-002 SH上でのエラー警告対応.
	startSHWarning();
// comment 2021/08/27 Sky中野 [MinervaSSBK][OP_BTS-34811] 優先タスク切り替え関連.
	resetAppChangeWaitTimer();
}
//================================================================================================
//［機能］Notification of active state.
//comment 2023/04/18 FPT.HungDM17 【IT6_2.3】【ALL MACHINE】【OP_BTS-42606】Fix coordinate of Maintenance Mode Status Bar depend on resolution of screen.
//================================================================================================
#ifndef DEF_LCD_7INCH
void PBasicMessageProc::procActivate() {
	GFWE_LCD_PANE e_LCDPane = getGamenPane( (PBasicMessageProc*)this );
	if ( GFWE_LCD_PANE_3 == e_LCDPane ){
		bl_JudgeStatusMaintenanceBar = True;
	} else {
		bl_JudgeStatusMaintenanceBar = False;
	}
	judgeStatusMaintenanceBar();
	showStatusMaintenanceBar();
}
#endif
//================================================================================================
//［機能］	基本メッセージ処理 : 画面終了.
//  画面終了時に呼ばれるハンドラ.
//  画面再配置(デフォルト指定)を実行するよう処理追加.
//［履歴］  2015/02/19 OPS1-MSE谷内 [ZeusS][CQ509132] ポップアップを閉じるキーをタップしたとき、下の画面にKeyReleaseを取られることがあるため、内部Key押下状態クリア対応.
//================================================================================================
void PBasicMessageProc::endGamen() {
// comment 2018/07/30 SCD2 K.Takei 不具合No.1049643【PSWCでユーザー設定＞表示言語選択 =Englishを設定して、中間認証を管理して、パネルで文字化け表示する.
	PNLC_PanelDispMachineSettingLanguage::getInstance()->updateMachineSettingLanguage();

	short partsNumber			= getPartsNumber();
	PDisplayParts** partsTable	= getPartsTable();

//	PNLC_PopupListControl* plCnt = PNLC_PopupListControl::newInstance();

// comment 2015/02/19 OPS1-MSE谷内 [ZeusS][CQ509132] ポップアップを閉じるキーをタップしたとき、下の画面にKeyReleaseを取られることがあるため、内部Key押下状態クリア対応.
	// 画面終了時に、タッチパネル押下状態をクリア.
	PMGC_PanelManager::resetTouchPanelPosition();

// comment 2016/03/16 OPS1-Sky金田 [Icarus][キーボード改善] キーボードの表示初期化.
	onlyPanel.setKeyboard1Line( True );

// comment 2015/10/08 MSE岡田 [MinervaSBK][CQ519901]アプリ切り替えフラグを初期化のためインスタンス取得.
	PNLC_ScanFaxDataLib* pc_ScanFaxDataLib = PNLC_ScanFaxDataLib::newInstance();
	if( NULL == pc_ScanFaxDataLib ) {
		LOGF_debug_PutEventLog(LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!");
		return;
	}

	int func_num = sizeof(g_FuncBadgeCacheList) / sizeof(PNLS_FuncBadge_Cache);
	for ( int jcnt = 0 ; jcnt < func_num ; jcnt++ ) {
		// キャッシュ対象の応用設定か確認.
		if ( this == g_FuncBadgeCacheList[jcnt].pc_PopGamen ) {
			if ( True == g_FuncBadgeCacheList[jcnt].b_Start ) {
				for ( int i = 0; i < partsNumber; i++ ) {
					partsTable[i]->clearCache();
				}
				return;
			} else {
				break;
			}
		} else {
			// キャッシュ中の応用設定画面のベース画面の終了か確認.
			if ( True == g_FuncBadgeCacheList[jcnt].b_Start ) {
				Bool b_EndGamen = False;
				Bool b_NextGamen = False;
				for ( int bcnt = 0 ; bcnt < g_FuncBadgeCacheList[jcnt].ss_baseGamenListCnt ; bcnt++ ) {
					if ( this == g_FuncBadgeCacheList[jcnt].ppc_BaseGamenList[bcnt] ) {
						b_EndGamen = True;
					}
					if ( nextGamen == g_FuncBadgeCacheList[jcnt].ppc_BaseGamenList[bcnt] ) {
						b_NextGamen = True;
					}
				}
// comment 2015/09/15 MSE田中優介 [MinervaSBK][CQ514071] ファクス/スキャン⇔ファクスアプリ切り替え時に応用設定画面の破棄条件を変更.
//				if ( (b_EndGamen == True) && (b_NextGamen == False) ) {
// comment 2015/10/08 MSE岡田 [MinervaSBK][CQ519901]インスタンス取得位置変更.
//				PNLC_ScanFaxDataLib* pc_ScanFaxDataLib = PNLC_ScanFaxDataLib::newInstance();

				if ( ( ( True == b_EndGamen )										// 表示中の画面が応用設定画面キャッシュリスト対象画面 かつ.
						&& ( False == b_NextGamen ) )									// 次の画面が応用設定画面キャッシュリスト対象画面の場合.
					|| ( ( NULL != pc_ScanFaxDataLib )
						&& ( True == pc_ScanFaxDataLib->isScanFaxAppChange() ) ) ) {	// または、ファクス/スキャン⇔ファクスオンリーアプリ切り替えの場合.
// comment 2015/10/08 MSE岡田 [MinervaSBK][CQ519901]アプリ切り替えフラグを初期化位置を変更.
//		    		pc_ScanFaxDataLib->setScanFaxAppChange( False );				// アプリ切り替えフラグを初期化.
					g_FuncBadgeCacheList[jcnt].b_Start = False;
					g_FuncBadgeCacheList[jcnt].pc_PopGamen->endGamen();
					break;
				}
			}
		}
	}

// comment 2015/10/08 MSE岡田 [MinervaSBK][CQ519901]アプリ切り替えフラグを初期化.
	// アプリ切り替えフラグを初期化.
	pc_ScanFaxDataLib->setScanFaxAppChange( False );

	// Check GamenCacheList
	if(isUseCacheGamen() == True) {
		// WVGA未満のときはパーツのキャッシュをクリアする.
		if(isResolutionLargerWVGA() == False) {
			clearCache();
		} else {
			// 画面widgetキャッシュクリア.
			PBaseWidget* baseWidget	= dynamic_cast<PBaseWidget*>( m_pc_displayParts );
			if( baseWidget ) {
				baseWidget->clearCache();
			}
		}
		return;
	}

	for( int i = 0; i < partsNumber; i++ )
	{
		partsTable[i]->endGUI();
	}

	// グレーアウト画面ウィジェットを破棄＆クリア
	if( m_pc_grayout != 0 )
	{
		m_pc_grayout->hide();
		m_pc_grayout->endGUI();
		delete m_pc_grayout;
		m_pc_grayout = 0;
	}

	// 画面ウィジェットを破棄＆クリア
	if( m_pc_displayParts != 0 )
	{
		m_pc_displayParts->hide();
		m_pc_displayParts->endGUI();
		delete m_pc_displayParts;
		m_pc_displayParts = 0;
	}

	relayout( True );  // CQ.360412 [MTUIv1]BOXプレビュー詳細Wide対応.

// comment 2014/11/28 OPS1-Sky_nozaki [Poseidon][CQ507074] 手差し用紙検出画面表示後、設定が元に戻る問題.
	m_bl_isReturnPaperSizeGamen = False;					// 画面終了時にフラグを落とす.
}

//================================================================================================
//［機能］	基本メッセージ処理 : 再配置処理関数.
// 再配置処理を行う関数.
// relayoutPartsをコールする場合、必要な情報は自身の
// クラスから取得できるよう、本関数をオーバーライドする。.
//================================================================================================
void PBasicMessageProc::relayout( Bool bl_IsDefault ) {
	// デフォルトでは、オフセット0でレイアウト変更を行う.
	relayoutParts( 0, 0, bl_IsDefault );
}

//================================================================================================
//［機能］	基本メッセージ処理 : 表示部品再配置.
//  表示部品の再配置が必要な際に呼ばれるハンドラ.
//  ・画面開始時)パラメータFalse(デフォルト以外指定)でコールされる.
//  ・画面終了時)パラメータTrue(デフォルト指定)でコールされる.
//  同一画面で表示部品の表示位置切替が必要な際、本メソッドをオーバーライドして再配置を実装する.
//  PBasicMessageProcでは空実装.
//  ※静的な表示部品の位置変更時、MR(MaintenanceRelease)対応等で2通り実装が必要なケースを想定.
//  表示部品テーブルを2面持つ対応を検討したが、以下の理由でメンテナンスコストが嵩むため,
//  本メソッド内で同一部品の再配置を実装できるよう設ける.
//  ・部品にレイアウト情報が含まれているので、位置を変える場合別部品を用意する必要がある.
//  ・別部品にした場合、同じ制御を双方の部品にする事になりメンテナンスコストが増える.
//================================================================================================
void PBasicMessageProc::relayoutParts( short ss_offsetX, short ss_offsetY, Bool bl_IsDefault ) {	// CQ.360412 [MTUIv1]BOXプレビュー詳細Wide対応にてIF変更.
}
//================================================================================================
//［機能］	基本メッセージ処理 : Qtブザー鳴動.
//  MTUI制御部からブザー鳴動を要求された際に呼ばれるハンドラ.
//================================================================================================
void PBasicMessageProc::procQtBuzzer( ushort option ) {
	setBuzzerType( ( uchar )option );
}

/*================================================================================================
［機能］	基本メッセージ処理 : ＬＥＤのセットアップ
//	パワーセーブ移行解除はパネルが行っているのではなく、パネルキー操作によってオートクリアタイマ等のタイマをクリアしたことによって解除される.
//	しかし、Ｃの状態（下図参照）では、解除シーケンスが処理されずスリープモードに入ってしまう。
//	パネルでは、操作した処理が行われてしまうので、Ｃの状態でスタートキーを押下するとジョブがスタートし、ジョブ動作中にスリープに移行してしまう.
//	上記の不具合を防ぐために、Ｃの状態ではスリープ移行した状態に見せかける。見せかけ処理は以下のとおり（ＣをＤと同じ状態にする）.
//	Ｃの状態で...
//	・バックライト OFF.
//	・スリープモードLED ON.
//	・モードLED（コピーモードキー、スキャンモードキー等のLED） OFF.
//	・スタートキーLED 赤.
//	・キー受付禁止.
//	・キー受付音 OFF.
//　パワーセーブ　スリープ　スリープＩＮ　　　　スリープ
//　キー押下　　　指示開始　(スリープモードON)　解除　　
//　　｜　　　　　｜　　　　　｜　　　　　　　　｜　　
//　Ａ↓　　Ｂ　　↓　　Ｃ　　↓　　Ｄ　　　　　↓Ａ
//－－－－－－－－－－－－－－－－－－－－－||－－－－
//　　｜←１０秒→｜←約６秒→｜　　　　　　　　｜　　
================================================================================================*/
void PBasicMessageProc::setupLED( Bool _bl_BlinkCycle )
{
// comment 2014/01/09 OPS1-Sky後藤 [Poseidon][CQ508648].	
	//static Bool	bl_displayOn = False;
	TYPE_PowerSaveStatus	e_PowerSaveStatus = ppc_Power->getPowerSaveStatus();
	TYPE_VoiceGuide_Status	eVGuide = ::getVoiceCursorMode();			// CQ303719 2008/05/28 12豊島 音声中の副電源off修正.
	Bool bSlide = APIC_SystemInfo::newInstance()->isSlidePanel();	// スライドレスパネル.
	if( ( e_PowerSaveStatus == TYPE_PowerSaveStatusSleep					// スリープ
	      || e_PowerSaveStatus == TYPE_PowerSaveStatusSleepDelay				// スリープ遅延
	      || e_PowerSaveStatus == TYPE_PowerSaveStatusSleepDelayIn			// スリープ遅延移行中
	      || e_PowerSaveStatus == TYPE_PowerSaveStatusSleepDelayOut			// スリープ遅延解除中
	      || e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOff			// 副電源OFF
	      || e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOffDelay		// 副電源OFF遅延
	      || e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOffDelayIn		// 副電源OFF遅延移行中
	      || e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOffDelayOut )	// 副電源OFF遅延解除中
	    || ( e_PowerSaveStatus == TYPE_PowerSaveStatusSleepIn ) || ( e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOffProc )	// スリープ移行中、または、副電源OFF処理中の時.
// comment 2015/01/15 OPS1 Sky溝部 [Poseidon][CQ508687] スリーププリントエンジン限定解除中もLED状態としてはスリープと同等とする.
	 || (e_PowerSaveStatus == TYPE_PowerSaveStatusPrtEngOnlySleepCancel		// スリーププリントエンジン限定解除.
	 || e_PowerSaveStatus == TYPE_PowerSaveStatusPrtEngOnlySleepCancelProc)	// スリーププリントエンジン限定解除処理中.
	    || ( eVGuide >= TYPE_VoiceGuide_EndPowerSaveKey && eVGuide <= TYPE_VoiceGuide_LowPower ) ) {	// 副電源/スリープによる音声発声中.
		// ウィークリータイマー一時起動中.
// comment 2014/01/09 OPS1-Sky後藤 [Poseidon][CQ508648].	
		//if (c_PowerObserver.nowWeeklyTimerTempWakeUp()) {
		//	g_displayOn(True);					// 表示オン.
		//	if( bl_displayOn == True ) {	// バックライト点灯時にブザー音を鳴らす.
		//		setBuzzerType( PCGE_BuzzerType_InputOk );
		//	}
		//	bl_displayOn = False;
		//} else {
		//	g_displayOn(False);					// 表示オフ.
		//	// ブザー音を鳴らす対応はパネルLEDバックライト点灯時に行う.
		//	bl_displayOn = True;
		//}
		reqLED( PNLE_LED_Accessibility,	PNLE_LEDStateOff );	// アクシビリティ.
		reqLED( PNLE_LED_IntMode,	PNLE_LEDStateOff );		// 割り込みモード.
		if( False == bSlide ) {
			// スライドレスパネルの場合.
			reqLED( PNLE_LED_Menu_SlidePanel, PNLE_LEDStateOff, False );			// メニュー.
			reqLED( PNLE_LED_Guidance_SlidePanel, PNLE_LEDStateOff, False );		// ガイダンス（ヘルプ）.
		}
		if( ( e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOff			// 副電源OFF
		      || e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOffDelay		// 副電源OFF遅延
		      || e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOffDelayIn		// 副電源OFF遅延移行中
		      || e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOffDelayOut )	// 副電源OFF遅延解除中
		    || ( e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOffProc )		// 副電源OFF処理中.
		    || ( eVGuide >= TYPE_VoiceGuide_EndPowerSaveKey && eVGuide <= TYPE_VoiceGuide_LowPower ) ) {	// 副電源/スリープによる音声発声中.
			reqLED( PNLE_LED_PrintGreen,	PNLE_LEDStateOff );		// プリント緑.
			if( e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOffProc	// 副電源OFF処理中の時.
			    || ( eVGuide >= TYPE_VoiceGuide_EndPowerSaveKey && eVGuide <= TYPE_VoiceGuide_LowPower ) ) {	// 副電源/スリープによる音声発声中.
				// 移行中はジョブスタートさせないように、スタートLEDを赤にする.
				reqLED( PNLE_LED_PrintRed,	PNLE_LEDStateOn );		// プリント赤 ON.
			} else {
				reqLED( PNLE_LED_PrintRed,	PNLE_LEDStateOff );		// プリント赤 OFF.
			}
			reqLED( PNLE_LED_EnlargeDisp,PNLE_LEDStateOff );		// 拡大表示.
			reqLEDBrightness( DEF_LEDBrightnessLevel );			// ＬＥＤ輝度設定.
		} else {
			// ウィークリータイマー一時起動中.
			if( c_PowerObserver.nowWeeklyTimerTempWakeUp() ) {
				reqLED( PNLE_LED_PrintGreen,	PNLE_LEDStateOff );		// プリント緑.
			} else {
				reqLED( PNLE_LED_PrintGreen,	PNLE_LEDStateOff );		// プリント緑 OFF.
			}
			if( e_PowerSaveStatus == TYPE_PowerSaveStatusSleepIn ) {	// スリープ移行中の時.
				// 移行中はジョブスタートさせないように、スタートLEDを赤にする.
				reqLED( PNLE_LED_PrintRed,	PNLE_LEDStateOn );		// プリント赤 ON.
			} else {
				reqLED( PNLE_LED_PrintRed,	PNLE_LEDStateOff );		// プリント赤 OFF.
			}
			reqLED( PNLE_LED_EnlargeDisp,PNLE_LEDStateOff );		// 拡大表示.
			reqLEDBrightness( DEF_LEDBrightnessLevel );			// ＬＥＤ輝度設定.
		}
	} else if( ( e_PowerSaveStatus == TYPE_PowerSaveStatusLowPower )		// プレヒート中.
	           || ( e_PowerSaveStatus == TYPE_PowerSaveStatusLowPowerDelay )		// ローパワー遅延モード.
	           || ( e_PowerSaveStatus == TYPE_PowerSaveStatusLowPowerDelayIn )	// ローパワー遅延移行中 .
	           || ( e_PowerSaveStatus == TYPE_PowerSaveStatusLowPowerDelayOut )	// ローパワー遅延解除中 .
// comment 2015/01/15 OPS1 Sky溝部 [Poseidon][CQ508687] ローパワープリントエンジン限定解除中もローパワーと同等とする.
		|| (e_PowerSaveStatus == TYPE_PowerSaveStatusPrtEngOnlyLowPowerCancel)		// ローパワープリントエンジン限定解除 .
		|| (e_PowerSaveStatus == TYPE_PowerSaveStatusPrtEngOnlyLowPowerCancelProc)	// ローパワープリントエンジン限定解除処理中.
	           || ( e_PowerSaveStatus == TYPE_PowerSaveStatusLowPowerIn ) ) {		// ローパワーモード移行中の時.
// comment 2014/01/09 OPS1-Sky後藤 [Poseidon][CQ508648].			
		//g_displayOn(False);					// 表示オフ.
		//// ブザー音を鳴らす対応はパネルLEDバックライト点灯時に行う.
		//bl_displayOn = True;
		reqLED( PNLE_LED_Accessibility,	PNLE_LEDStateOff );	// アクシビリティ.
		reqLED( PNLE_LED_EnlargeDisp,PNLE_LEDStateOff );		// 拡大表示.
		reqLED( PNLE_LED_IntMode,	PNLE_LEDStateOff );		// 割り込みモード OFF.
		if( False == bSlide ) {
			// スライドレスパネルの場合.
			reqLED( PNLE_LED_Menu_SlidePanel, PNLE_LEDStateOff, False );			// メニュー.
			reqLED( PNLE_LED_Guidance_SlidePanel, PNLE_LEDStateOff, False );		// ガイダンス（ヘルプ）.
		}
		if( e_PowerSaveStatus == TYPE_PowerSaveStatusLowPowerIn ) {	// ローパワーモード移行中の時.
			// 移行中はジョブスタートさせないように、スタートLEDを赤にする.
			reqLED( PNLE_LED_PrintGreen,	PNLE_LEDStateOff );		// プリント緑 OFF.
			reqLED( PNLE_LED_PrintRed,	PNLE_LEDStateOn );		// プリント赤 ON.
		} else {
			reqLED( PNLE_LED_PrintGreen,	PNLE_LEDStateOff );		// プリント緑 ON.
			reqLED( PNLE_LED_PrintRed,	PNLE_LEDStateOff );		// プリント赤 OFF.
		}
		reqLEDBrightness( DEF_LEDBrightnessLevel );			// ＬＥＤ輝度設定.
	} else {	// プレヒート中でない時.
// comment 2014/01/09 OPS1-Sky後藤 [Poseidon][CQ508648].			
		//// ウィークリータイマー一時復帰後だが、未チェックの場合はバックライト消灯.
		//if (	( False != c_PowerObserver.nowWeeklyTimerTempWakeUp() )
		//	 && ( False == c_PowerObserver.isCheckWeeklyTimerTempWakeUp() ) ) {
		//	g_displayOn( False );				// 表示オフ.
		//	bl_displayOn = True;
		//} else {
		//	g_displayOn(True);					// 表示オン.
		//	// ブザー音を鳴らす対応はパネルLEDバックライト点灯時に行う.
		//	if( bl_displayOn == True ) {	// バックライト点灯時にブザー音を鳴らす.
		//		setBuzzerType( PCGE_BuzzerType_InputOk );
		//	}
		//	bl_displayOn = False;
		//}
		PNLE_LEDState _e_LED_IntMode = ( onlyPanel.ec_IntMode == EC_IntMode_On ) ? PNLE_LEDStateOn: PNLE_LEDStateOff;	// 割り込みモード.
		// 拡大表示LEDを点灯させるのは待機状態の場合のみとする
		PNLE_LEDState _e_LED_Enlarge =
		    ( onlyPanel.bl_isEnlargeDisplayMode == True && e_PowerSaveStatus == TYPE_PowerSaveStatusIdle ) ? PNLE_LEDStateOn: PNLE_LEDStateOff;	// 拡大表示.
		PNLE_LEDState _e_LED_Accessibility = ( onlyPanel.b_IsAccessibilityGamen == True ) ? PNLE_LEDStateOn: PNLE_LEDStateOff;	// アクシビリティ.
		reqLED( PNLE_LED_Accessibility,	_e_LED_Accessibility );	// アクシビリティ.
		reqLED( PNLE_LED_IntMode,	_e_LED_IntMode );		// 割り込みモード.
		Bool bl_PowerSave = False;
		reqLED( PNLE_LED_EnlargeDisp,_e_LED_Enlarge );		// 拡大表示.
		if( False == bSlide ) {
			// スライドレスパネルの場合.
			PNLE_LEDState e_LED_Menu = PNLE_LEDStateOff;
			PNLE_LEDState e_LED_Guide = PNLE_LEDStateOff;

			if( ( True == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_HomeTopMenu ) )		// トップメニュー画面.
			    || ( nowGamen == &gp_EnlAct_HomeTopMenu ) // トップメニュー画面(拡大).
			  ) {
				e_LED_Menu = PNLE_LEDStateOn;
			}
			reqLED( PNLE_LED_Menu_SlidePanel, e_LED_Menu, False );			// メニュー.

			if( True == onlyPanel.bl_HelpDisp ) {
				// Guidance(Help)表示中.
				e_LED_Guide = PNLE_LEDStateOn;
			}
			reqLED( PNLE_LED_Guidance_SlidePanel, e_LED_Guide, False );		// ガイダンス（ヘルプ）.
		}
		// スタートＬＥＤ（赤、緑）の処理.
		if( TYPE_ImageLogSendType1 == PnlComGetSystemSetting()->getImageLogSendEnabled()					// 画像ログ転送(Type1).
		    && IGLE_ImageLogStatusStarted != APIC_ImageLogControll::newInstance()->getImageLogStatus() ) {	// 「画像ログ転送起動可能」ではない場合、「赤」 D-D51 2008/5/29 12柳川.
			reqLED( PNLE_LED_PrintGreen,	PNLE_LEDStateOff );		// プリント緑.
			reqLED( PNLE_LED_PrintRed,	PNLE_LEDStateOn );		// プリント赤.
		} else if( ep_LED_BlinkMode != EP_LED_BlinkMode_Off && _bl_BlinkCycle == True ) {
			PNLE_LEDState e_CPU_LEDState = ( EP_LED_BlinkMode_Normal == ep_LED_BlinkMode ) ? PNLE_LEDStateBlinkFast: PNLE_LEDStateBlinkSlow;
			if( bl_PrintLEDGreen == True ) {
				reqLED( PNLE_LED_PrintRed,	PNLE_LEDStateOff );		// プリント赤.
				reqLED( PNLE_LED_PrintGreen,	e_CPU_LEDState );		// プリント緑.
			} else {
				reqLED( PNLE_LED_PrintGreen,	PNLE_LEDStateOff );		// プリント緑.
				reqLED( PNLE_LED_PrintRed,	e_CPU_LEDState );		// プリント赤.
			}
		} else if( bl_PrintLEDGreen == True ) {
			if( True == onlyPanel.getPrintingPageAcses() ) {
				reqLED( PNLE_LED_PrintRed,	PNLE_LEDStateOn );		// プリント赤.
				reqLED( PNLE_LED_PrintGreen,	PNLE_LEDStateOff );		// プリント緑.
			} else {
				reqLED( PNLE_LED_PrintRed,	PNLE_LEDStateOff );		// プリント赤.
				reqLED( PNLE_LED_PrintGreen,	PNLE_LEDStateOn );		// プリント緑.
			}
		} else {
			reqLED( PNLE_LED_PrintGreen,	PNLE_LEDStateOff );		// プリント緑.
			reqLED( PNLE_LED_PrintRed,	PNLE_LEDStateOn );		// プリント赤.
		}
		if( False == bl_PowerSave ) {
			reqLEDBrightness( 100 );							// ＬＥＤ輝度設定.
		}
	}
	// DataLED.
	APIC_ActiveJobList* pc_ActiveJobList = APIC_ActiveJobList::newInstance();
	int32_t i32_FaxDocCount= 0;
	if( APIC_BoxRegistry::GetAccessStatus() == True ) {	// Doc初期化済みの場合.
		i32_FaxDocCount= APIC_BoxRegistry::getFaxDocumentCnt();
	}
	if( pc_ActiveJobList->isExistPrintRxJob()
	    || pc_ActiveJobList->isExistPdlAppInputProcess()
	    || cp_Interpreter->isExistInterpreterJob()				// インタープリター処理中のジョブが存在チェック.
	    || PNLE_LogOutputStatus_Output == PLBC_LogOutputStatus::getInstance()->getLogOutputStatus() ) {	// 生産ラインにてログ出力中.
		// 受信中ジョブ有り.
		reqLED( PNLE_LED_Data,	PNLE_LEDStateBlinkFast );		// データ.
	} else if( e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOff			// 副電源OFF
	           || e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOffDelay		// 副電源OFF遅延
	           || e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOnProc			// 副電源ON処理中
	           || e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOffDelayIn		// 副電源OFF遅延移行中
	           || e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOffDelayOut ) {	// 副電源OFF遅延解除中
		// 副電源OFF時.
		reqLED( PNLE_LED_Data,	PNLE_LEDStateOff );		// データ.
	} else {
		// 副電源ON時.
		if( ( pc_ActiveJobList->isExistPrintData() )
		    || ( i32_FaxDocCount > 0 )
		    || ( pc_ActiveJobList->isExistFaxData( TYPE_isExistFaxTx ) == True ) ) {
			reqLED( PNLE_LED_Data,	PNLE_LEDStateOn );		// データ.
		} else {
			reqLED( PNLE_LED_Data,	PNLE_LEDStateOff );		// データ.
		}
	}
	reqLED( PNLE_LED_VoiceGuide, ( ( TYPE_VoiceGuide_OFF == getVoiceCursorMode() ) || ( TYPE_VoiceGuide_OneTimeBoot == getVoiceCursorMode() ) ) ? PNLE_LEDStateOff : PNLE_LEDStateOn );
}
/*================================================================================================
［機能］	基本メッセージ処理 : 定期起床
 変更履歴 : 2012/07/06 OPS1-Sky桑原 [MinervaBK][CQ366195]ボックス画面で手差しトレイ受付音を鳴らさない.
            2013/02/20 日立JTE 奈良 【OP-PRV-01】プレビューオプション化対応.
                                    プレビュー動的リンク後にプレビュー作成完了通知用オブザーバーを登録する.
================================================================================================*/
void PBasicMessageProc::procTimerWakeUp( uchar option )
{
	if( bl__PnlDLGamen == True ) { setupGamen(); return; }		// ダウンロード画面表示状態情報

	PNLC_NetworkLan::getInstance()->updateVlanSetting();
//---共有データの変化チェックを行ない内部イベントを発生させる----------------------------
	register TYPE_FeedTray changeFeedCode = TYPE_FeedTrayMax;			// 給紙口変化検出用の変数
	APIC_ActiveJobList* pc_ActiveJobList = APIC_ActiveJobList::newInstance();
	comPrintJob = pc_ActiveJobList->getCurrentPrintJob();

// comment : WebUI2 画面切替え制御 -> 20180302 Chikuan.
	if( PNLE_WebUITokenPasswordStatusDone != PNLC_WebUIToken::getInstance()->getStatus() ){
		PNLC_WebUIToken::getInstance()->getPanelUserPassword();
	}
// comment 2020/01/30 SCD2-Sky浜野 [IT6] セキュリティ脆弱性のため、管理者情報保存処理を削除.
//// comment 2018/05/30 SCD2-Sky鳫林 各種アクセス情報の取得 対応.
//	if( PNLE_WebUITokenPasswordStatusDone != PNLC_WebUIToken::getInstance()->getAdminPasswordStatus() ){
//		PNLC_WebUIToken::getInstance()->getPanelAdminPassword();
//	}
	
// comment : WebUI2 画面切替え制御 <- 20180302 Chikuan.

	//【CQ517977】【392734_Merge_PZH2.1c 市場問題マージ】【HCL_Bhargavi】 2015/09/04 Add Start.
	if (True == APIC_SoftDipSw::newInstance()->getSoftDipSwBit( 74, 4 ))
	{
		if((True != gp_PrintOapFileListExt_pullprint.bl_update) && (comPrintJob != NULL) && (gp_PrintOapFileListExt_pullprint.PApp_No == comPrintJob->getJobSpec()->pc_Send->c_ComSend.getApplicationID())
	  	&&(NULL != ScanOapExtSet.getApplication()) && ( TYPE_ExtTypePullPrint == ScanOapExtSet.getApplication()->s_Version.e_Type) 
	  	&&( 0 != comPrintJob->getJobStatus()->getJobID())&& (TYPE_UserAuthenticationOff != ppc_UserAuthentication->getUserAuthentication(PNLC_NetworkLan::getInstance()->getLine())) 
	  	&&(NULL != PnlComGetUser()) && (PnlComGetUser()->getUserNo() == comPrintJob->getJobSpec()->pc_Common->c_UserAcount.getUserNo())) {
			gp_PrintOapFileListExt_pullprint.bl_update = True;
	  	}
	}
	//【CQ517977】【392734_Merge_PZH2.1c 市場問題マージ】【HCL_Bhargavi】 2015/09/04 Add End.	
//comment 2018/30/04 【HCLT-Hepzibah】 【IT6 EAGLE】 【AR1044109】 Check B6 paper is detected or not.
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
// comment 2016/10/12 SCD2-2G 後藤 [IT5_3.3][1002658] USBにデータが取り込まれていないのにUSB取り込み表示対応.
	APIC_Job* pc_Job = onlyPanel.getUSBSaveJob();
	if( NULL != pc_Job ){
		if( False == ( APIC_JobInstanceList::isExistJob( pc_Job ) ) ){	// USB保存ジョブがない場合.
			onlyPanel.setUSBSaveJob( NULL );
		}
	}

	if( APIC_JobInstanceList::isExistJob( onlyPanel.cc_DisplayJob ) == False ) {
		onlyPanel.cc_DisplayJob = comPanelJob;
	}

	if( True == PNLC_SnapshotControlLib::newInstance()->getSkipCheckHDD() ) {
		APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();					// HDの情報を取得.

		if( NULL != pc_HddInfo ) {
			if( TYPE_HddStatusFormated == pc_HddInfo->getHddStatus() ) {
				PNLC_SnapshotControlLib::newInstance()->setSkipCheckHDD( False );
			}
		}
	}

	PNLC_NotifyEventInfo* pc_NtfEventInfo = PNLC_NotifyEventInfo::newInstance();		// データの管理クラス.
	if( NULL == pc_NtfEventInfo ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	// パネルカレントジョブ変化エッジ.
	Bool b_comPanelJobChangeEdge = False;
	// 登録数オーバー時のジョブ取得処理.
	b_comPanelJobChangeEdge = c_JobControl.JobEntryOverGetJob();
	// 管理装置・カード挿入変化.
	c_KeyDeviceObserver.dataChanged();
	if( onlyPanel.b_KeyCounterPanelCall == True ) {
		// キーカウンタ引き抜き通知.
		pc_PanelMain->ResetAtKeyCountNotice( 0 );
	}
	if( True == onlyPanel.getPrintingPageAcses() ) {
		if( False == APIC_KeyDevice::isPrintingPageAcses() ) {
			onlyPanel.setPrintingPageAcses( False );
		}
	}

	short ss_PreGamenWarningNo = PNLC_WarningManager::newInstance()->getPreGamenWarningNo();
	Bool bl_ReqChangeGamen_backup = PNLC_WarningManager::newInstance()->getReqChangeGamen();

// サービスモード:警告発生中にスキャン／プリント状態の変化、プリントジョブ状態の変化(プリント停止中の変化)、.
// スキャナ配信方式の変化 (TWAIN)、スキャナ読み取り開始の変化が発生した場合に.
// サービスモード用の各処理を動作させるようします.
// 本対応はサービスモード中にサービスモード画面以外（警告画面を想定しています）に遷移した場合、サービスモード用の処理が.
// 動作しないための対応となります.



// comment 2018/05/29 SCD2-Sky東野 [IT6] [1047169] 不具合対応  Start.
	static EP_PanelStatus ep_PreStatus = -1;
	APIC_MultiAutoTest *pc_MultiAutoTest = APIC_MultiAutoTest::newInstance();
	if(NULL != pc_MultiAutoTest){
		Bool bl_wait = pc_MultiAutoTest->isWaitCheckPanelStatus();
		if ( ( ( EP_PNS_Setting == ep_PreStatus ) && ( EP_PNS_Setting != onlyPanel.ep_PanelStatus ) )
		||   ( ( EP_PNS_Setting != ep_PreStatus ) && ( EP_PNS_Setting == onlyPanel.ep_PanelStatus ) )
		||   ( False != bl_wait) ) {
			ep_PreStatus = onlyPanel.ep_PanelStatus;
			// EP_PNS_Setting     → EP_PNS_Setting以外.
			// EP_PNS_Setting以外 → EP_PNS_Setting.
			// の状態変化をAPIC_MultiAutoTest : DBGへ通知.
			if(EP_PNS_Setting == ep_PreStatus){
				pc_MultiAutoTest->setPanelStatus_Setting(True);
			}else{
				pc_MultiAutoTest->setPanelStatus_NotSetting(True);
			}
		}
	}
// comment 2018/05/29 SCD2-Sky東野 [IT6] [1047169] 不具合対応  End.
	if( ( isStatusChange() == True ) || ( b_comPanelJobChangeEdge == True ) ) {			// スキャン／プリント状態の変化.
		if( ( ppc_SystemStatus->getServiceModeOn() == True ) &&
		    ( NULL != dynamic_cast< PPopupWarningCom* >( PNLC_PopupListControl::newInstance()->getNowPopup() ) ) ) {
			// サービスモード中で警告画面表示中.
			procServiceStatusChange( _bl_comPanelJobChange == True ? 1 : 0 );
		} else {
			procStatusChange( _bl_comPanelJobChange == True ? 1 : 0 );
		}
	}
	if( isPrintJobStopChange() == True ) {												// プリントジョブ状態の変化(プリント停止中の変化).
		if( ( ppc_SystemStatus->getServiceModeOn() == True ) &&
		    ( NULL != dynamic_cast< PPopupWarningCom* >( PNLC_PopupListControl::newInstance()->getNowPopup() ) ) ) {
			// サービスモード中で警告画面表示中.
			;	// PServiceCom::procPrintJobStopChange()は処理無.
		} else {
			procPrintJobStopChange( 0 );

// comment OP1-Sky田村 [特ROM対応][MZV-72] 一般リーガル対応 .
			// PrintJobのステータス変更タイミングで、カード未挿入警告が検出できたら.
			// そのままカード未挿入のポップアップを表示したままにしておきたいので.
			// カード引き抜き時状態を作る.
			if( APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW213, 1 ) == True
			 && (    comPanelJob->isWarning( ERROR_MagCardUnset_S ) == True
			      || comPanelJob->isWarning( ERROR_MagCardUnset )   == True )
			 && ( ppc_SystemStatus->getServiceModeOn() == False )						// サービスモードでない.
			 && ( c_SystemControl.getAdminModeOn()     == False )						// 管理者モードでない.
			 && ( nextGamen != NULL ) )													// nextGamenに指定があるとき.
			{
				nextGamen = NULL;								// 画面遷移を抑制して...
				onlyPanel.bl_IsKeyCounterMagCardUnset = True;	// カード引き抜き状態を作る.
			}

		}
	}
	if( isScanJobSendChange() == True ) {												// スキャナ配信方式の変化 (TWAIN).
		if( ( ppc_SystemStatus->getServiceModeOn() == True ) &&
		    ( NULL != dynamic_cast< PPopupWarningCom* >( PNLC_PopupListControl::newInstance()->getNowPopup() ) ) ) {
			// サービスモード中で警告画面表示中.
			;	// PServiceCom::procScanJobSendChange()は処理無.
		} else {
			procScanJobSendChange( option );
		}
	}
	if( isScanJobScanStartChange()	== True ) {											// スキャナ読み取り開始の変化.
		if( ( ppc_SystemStatus->getServiceModeOn() == True ) &&
		    ( NULL != dynamic_cast< PPopupWarningCom* >( PNLC_PopupListControl::newInstance()->getNowPopup() ) ) ) {
			// サービスモード中で警告画面表示中.
			;	// PServiceCom::procScanJobScanStartChange()は処理無.
		} else {
			procScanJobScanStartChange( option );
		}
	}
	if( isActiveJobEnd()			== True ) { procActiveJobEnd( option );				}	// 動作中のジョブが無くなった
	// マシンステータスの変化
	if( isDocSizeEffective()		== True ) { procDocSizeEffective( b_targetJobChange == True ? 1: 0 );		 	}	// 原稿サイズ有効の変化
#ifdef DEF_A4_Cus
	if( isEnergySaveChange()   		== True ) {
		PNLC_SysPerformanceLogLib* pc_SysPerformanceLogLib = PNLC_SysPerformanceLogLib::newInstance();
		if ( NULL != pc_SysPerformanceLogLib ) {
			pc_SysPerformanceLogLib->setSysPerformanceLog( PNLE_SysPerformanceLog_EnergySaveChange );
		}
		procEnergySaveReset( option );	// 省エネ状態の変化.
	}
#else	// DEF_A4_Cus.
	if( isEnergySaveChange()   		== True ) { procEnergySaveReset( option );		}	// 省エネ状態の変化
#endif	// DEF_A4_Cus.
// comment 2019/09/13 SCD2-Sky浜野 [EagleH][OP_BTS-13290] タブクローズ検出対応.
	static Bool bl_PreBrowerTabOpenFlg = False;
	// ブラウザのTabが閉じられた場合、タブ状態を初期化する.
	if( ( False == onlyPanel.bl_BrowerTabOpenFlg ) && ( True == bl_PreBrowerTabOpenFlg ) ) {
// comment 2020/07/08 Sky東島 [IT6_2.0MR3][OP_BTS-21031].
// 初期化中にブラウザtabが閉じられた場合完了通知がこず、初期化が完了しないため、タブ初期化状態にかかわらず設定値の初期化を行うよう変更.
//		if( PNLE_BrowserChangeTabControlTabInitStatus_Done == PNLC_BrowserChangeTabControl::getInstance()->getTabInitStatus() ) {
		pc_ManageIGCWaitDisplay.setDisplayFLGOff();
		PNLC_WebUIControl::getInstance()->initData();
		PNLC_BrowserChangeTabControl::getInstance()->initData();
		PNLC_WebUIControl_v1::getInstance()->initData();
		PNLC_BrowserChangeTabControl::getInstance()->initAdminData();
		onlyPanel.setBrowserMode( False );
//		}
	}
	bl_PreBrowerTabOpenFlg = onlyPanel.bl_BrowerTabOpenFlg;

	static TYPE_PowerSaveStatus e_PrePowerSaveStatus = TYPE_PowerSaveStatusFirstBootConfigration;
	TYPE_PowerSaveStatus e_PowerSaveStatus = ppc_Power->getPowerSaveStatus();
	if( ( TYPE_PowerSaveStatusPrtEngOnlySleepCancelPanelOn == e_PrePowerSaveStatus ) && ( TYPE_PowerSaveStatusSleepDelayOut == e_PowerSaveStatus ) ) {
		procEnergySaveReset( option );	// 省エネ状態の変化.
	}
	e_PrePowerSaveStatus = e_PowerSaveStatus;

// comment 2016/05/17 SCD2-Sky辻 [特ROM][FQ-OPSD-1600010-OFP008]手差しトレイに用紙をセットしたままパワーセーブ移行後、復帰すると「トレイ（手差し）の設定変更」画面が表示される. 
//	if( isPaperInsertToManualFeed() == True ) { 											// 手差しトレイの状態変化 
////		if( ppc_FeedTray->getPaperStatus(TYPE_FeedTrayManual) == TYPE_PaperStatusExist) {	// 手差しトレイへの用紙挿入 
//		if( TYPE_PaperStatusExist == pc_NtfEventInfo->getPaperStatus(TYPE_FeedTrayManual) ) {	// 手差しトレイへの用紙挿入 【パフォーマンス改善】2012/08/22 第一OSW開発 酒井(健).
	PNLE_PAPER_CHANGE_STATUS e_PaperChageStatus = checkPaperInsertToManualFeed();
	if( PNLE_PAPER_CHANGE_NONE != e_PaperChageStatus ) { 					// 手差しトレイの状態変化. 
		if ( PNLE_PAPER_CHANGE_INSERT == e_PaperChageStatus ) {				// 手差しトレイへの用紙挿入.
			// ボックスまたはジョブモードの時はイベントを無視する.
			// - いずれかのジョブでプリント用紙不適合かペーパーエンプティが発生している時はイベントを受け付ける.
			LIBC_List<APIC_Job>	 c_Joblist;	// ジョブのリスト.
			short ss_JobNum		 = 0;		// リストの個数.
			APIC_Job* pc_Job	 = NULL;	// 確認用ジョブ.
			short ss_listCnt	 = 0;		// カウント.
			Bool b_WarningOn	 = False;	// 警告発生か否か.
			// 全アクティブジョブを取得.
//Comment 2017/11/22【HCLT-Sudheer】【IT5 4.2】【RQ686】 API Correction.
//			ss_JobNum = cp_ActiveJobList->getList( TYPE_ActiveJobListAll,	// アクティブジョブリスト種類.
//			                                       1,						// 開始位置（１～Ｎ）.
//			                                       TYPD_MaxActiveJob,		// 個数.
//			                                       &c_Joblist );			// リスト.
			ss_JobNum = cp_ActiveJobList->getList( TYPE_ActiveJobListAll,	// アクティブジョブリスト種類.
			                                       1,						// 開始位置（１～Ｎ）.
			                                       TYPD_MaxActiveJob,		// 個数.
			                                       &c_Joblist,
			                                       PNLC_NetworkLan::getInstance()->getLine() );			// リスト.
			for( ss_listCnt = 0; ss_listCnt < ss_JobNum; ss_listCnt++ ) {
				// リストからジョブを取得.
				if( ss_listCnt == 0 ) {
					pc_Job = c_Joblist.findFirst();	 // 先頭データを取得
				} else {
					pc_Job = c_Joblist.findNext();	 // 次データを取得
				}
				// いずれかの警告(全画面警告、メッセージ警告)発生チェック.
				b_WarningOn = ( onlyPanel.isWarningOn( pc_Job, PERROR_PrintPaperError ) == True
				                || onlyPanel.isWarningOn( pc_Job, PERROR_PaperEmpty ) == True );
				if( b_WarningOn == True ) {
					break;
				}
			}
			if( ( ( onlyPanel.b_nowBoxMode == False )
			      && ( gp_EnlCopy_Basic.e_JobDispListType == JBLE_JobListPrintControlBasic )	// 画面インスタンスは、PJobListControlを継承していてpcgcom.cppから参照可能なものを指定する.
			      && ( False == onlyPanel.isTopmenuMode() ) )
			    || ( True == b_WarningOn ) )
			{
				procPaperInsertToManualFeed( option );
			}
// VenusMLK 管理No.1434_302_2 手差しトレイ用紙検出 ST 2015.11.12 Sky瀬戸.
#ifdef DEF_A4_Cus
			// VenusMLKでは無条件で処理を行う.
			procManualPaperSizeSetting( option );	// 手差し用紙サイズ設定.
#else
			// FDセンサーが無い場合.
			if( False == PnlComGetPrinterSystem()->isManualTrayFDSizeDetect() ) {
				procManualPaperSizeSetting( option );	// 手差し用紙サイズ設定.
			}
#endif
// VenusMLK 管理No.1434_302_2 手差しトレイ用紙検出 ED 2015.11.12 Sky瀬戸.
		} else {
			procPaperPutOffToManualFeed( option );	// 手差しトレイからの用紙引抜き
		}
	}
//comment 2018/04/08 【HCLT-Prasanjet】 【IT6 EAGLE】 【AR1042477】 move code to procfeed info chage..
//// Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 Check the Paper Insert / Remove status.
//#ifndef DEF_A4_Cus
//	Bool bl_PaperChageStatus = checkPaperInsertToTray1Feed(); 
//		if ( True == bl_PaperChageStatus ) {
//			// ボックスまたはジョブモードの時はイベントを無視する.
//			// - いずれかのジョブでプリント用紙不適合かペーパーエンプティが発生している時はイベントを受け付ける.
//			LIBC_List<APIC_Job>	 c_Joblist;	// ジョブのリスト.
//			short ss_JobNum		 = 0;		// リストの個数.
//			APIC_Job* pc_Job	 = NULL;	// 確認用ジョブ.
//			short ss_listCnt	 = 0;		// カウント.
//			Bool b_WarningOn	 = False;	// 警告発生か否か.
//			// 全アクティブジョブを取得.
//			ss_JobNum = cp_ActiveJobList->getList( TYPE_ActiveJobListAll,	// アクティブジョブリスト種類.
//			                                       1,						// 開始位置（１～Ｎ）.
//			                                       TYPD_MaxActiveJob,		// 個数.
//			                                       &c_Joblist,
//			                                       PNLC_NetworkLan::getInstance()->getLine() );			// リスト.
//			for( ss_listCnt = 0; ss_listCnt < ss_JobNum; ss_listCnt++ ) {
//				// リストからジョブを取得.
//				if( ss_listCnt == 0 ) {
//					pc_Job = c_Joblist.findFirst();	 // 先頭データを取得
//				} else {
//					pc_Job = c_Joblist.findNext();	 // 次データを取得
//				}
//				// いずれかの警告(全画面警告、メッセージ警告)発生チェック.
//				b_WarningOn = ( onlyPanel.isWarningOn( pc_Job, PERROR_PrintPaperError ) == True
//				                || onlyPanel.isWarningOn( pc_Job, PERROR_PaperEmpty ) == True );
//				if( b_WarningOn == True ) {
//
//					break;
//				}
//			}
//			if( ( ( onlyPanel.b_nowBoxMode == False )
//			      && ( gp_EnlCopy_Basic.e_JobDispListType == JBLE_JobListPrintControlBasic )	// 画面インスタンスは、PJobListControlを継承していてpcgcom.cppから参照可能なものを指定する.
//			      && ( False == onlyPanel.isTopmenuMode() ) )
//			    || ( True == b_WarningOn ) )
//			{
//				procPaperInsertToTray1Feed( option );
//			}
//			//procTray1PaperSizeSetting( option );
//		}
//#endif	//DEF_A4_Cus.
// Comment -> 2017/05/12 SCD2 3G Sky瀬戸 [AccuRev.1014826]media-ready対応.
#ifdef DEF_A4_Cus
#ifdef DEF_KRONOSS_Cus
	#define PNLD_CheckTrayNum	4
#else // DEF_KRONOSS_Cus.
	#define PNLD_CheckTrayNum	1
#endif // DEF_KRONOSS_Cus.
	PNLC_PaperSizeTypeSettingLib* pc_PaperSizeTypeSettingLib = PNLC_PaperSizeTypeSettingLib::newInstance();
	PNLE_PAPER_CHANGE_STATUS e_PaperChageStatusTray = PNLE_PAPER_CHANGE_NONE;
#ifdef DEF_KRONOSS_Cus
	static const TYPE_FeedTray st_ChackTable[ PNLD_CheckTrayNum ] = { TYPE_FeedTray1, TYPE_FeedTray2, TYPE_FeedTray3, TYPE_FeedTray4 };
#else // DEF_KRONOSS_Cus.
	static const TYPE_FeedTray st_ChackTable[ PNLD_CheckTrayNum ] = { TYPE_FeedTray1 };
#endif // DEF_KRONOSS_Cus.
	
	if ( NULL != pc_PaperSizeTypeSettingLib ) {
		for ( ushort us_CheckTrayCnt = 0; us_CheckTrayCnt < PNLD_CheckTrayNum; us_CheckTrayCnt++ ) {
			e_PaperChageStatusTray = checkPaperInsertToTray( st_ChackTable[ us_CheckTrayCnt ] );
			if ( PNLE_PAPER_CHANGE_NONE != e_PaperChageStatusTray ) {
				if ( PNLE_PAPER_CHANGE_INSERT == e_PaperChageStatusTray ) {
					pc_PaperSizeTypeSettingLib->setTargetTray( st_ChackTable[ us_CheckTrayCnt ] );
				}
			}
		}
		
		if ( True == pc_PaperSizeTypeSettingLib->isExistTargetTray() ) {
			procTrayPaperSizeSetting( option );
		}
	}
#endif // DEF_A4_Cus.
// Comment <- 2017/05/12 SCD2 3G Sky瀬戸 [AccuRev.1014826]media-ready対応.

	// ファクスのステータスの変化
	FAXS_CmdRs s_CmdRs;
	if( isFaxCmdRsChange( TYPE_CommLine1, &s_CmdRs ) == True ) { procFaxCmdRsChange( option, TYPE_CommLine1, s_CmdRs ); }	// ファクス回線1からのコマンドレスポンス通知の変化
	if( isFaxCmdRsChange( TYPE_CommLine2, &s_CmdRs ) == True ) { procFaxCmdRsChange( option, TYPE_CommLine2, s_CmdRs ); }	// ファクス回線2からのコマンドレスポンス通知の変化
	if( True == isFaxCmdRsChange( TYPE_CommLine3, &s_CmdRs ) ) { procFaxCmdRsChange( option, TYPE_CommLine3, s_CmdRs ); }	// ファクス回線3からのコマンドレスポンス通知の変化.
	if( True == isFaxCmdRsChange( TYPE_CommLine4, &s_CmdRs ) ) { procFaxCmdRsChange( option, TYPE_CommLine4, s_CmdRs ); }	// ファクス回線4からのコマンドレスポンス通知の変化.
	if( isFaxLine1LineStatusChange() == True ) { procFaxLine1LineStatusChange( option );}	// ファクス回線1状態の変化
	if( isFaxNumberDispChange( TYPE_CommLine1 ) == True ) { procFaxNumberDispChange( option, TYPE_CommLine1 );}	// ファクス回線1状態のネーム・ナンバーディスプレイ表示状態変化
	if( isFaxNumberDispChange( TYPE_CommLine2 ) == True ) { procFaxNumberDispChange( option, TYPE_CommLine2 );}	// ファクス回線1状態のネーム・ナンバーディスプレイ表示状態変化
	if( True == isFaxNumberDispChange( TYPE_CommLine3 ) ) { procFaxNumberDispChange( option, TYPE_CommLine3 );}	// ファクス回線3状態のネーム・ナンバーディスプレイ表示状態変化.
	if( True == isFaxNumberDispChange( TYPE_CommLine4 ) ) { procFaxNumberDispChange( option, TYPE_CommLine4 );}	// ファクス回線4状態のネーム・ナンバーディスプレイ表示状態変化.
	if( isDocOnADF() == True ) {
		if( True == pc_NtfEventInfo->getAdfOriginalExist() ) {	// EDH原稿ON時 【パフォーマンス改善】2012/08/22 第一OSW開発 酒井(健).
#ifdef DEF_A4_Cus
			PLBC_PolygonMotorStartIF::judgePolygonMotorStart( PNLE_FcotTypeZeus );
#endif  // DEF_A4_Cus.
			setBuzzerType( PCGE_BuzzerType_InputOk );
		}
		procDocOnADF( option );		// ＡＤＦ原稿トレイの状況変化
	}
#ifdef DEF_A4_Cus
	if( True == isDocCoverOpen() ) {
		procDocCoverOpen( option );	// 原稿カバー開閉の状況変化.
		
		// カバーをあけた場合は通知を行う.
		if ( True == pc_NtfEventInfo->getPlatenCoverOpen() ) {
			PLBC_PolygonMotorStartIF::judgePolygonMotorStart( PNLE_FcotTypeZeus );
		}
	}
#else
	if( isDocCoverOpen()		 	== True ) { procDocCoverOpen( option );			 	}	// 原稿カバー開閉の状況変化
#endif  // DEF_A4_Cus.
	changeFeedCode = FeedInfoChange();														// 給紙口情報の変化
	if( changeFeedCode != TYPE_FeedTrayMax	) { procFeedInfoChange( changeFeedCode );	}

	ushort us_ChangeFeedTray = 0;
	if( True == isFeedPaperKindChange( &us_ChangeFeedTray ) ) {
		// 用紙サイズ・用紙種類変化.
		procFeedPaperKindChange( option, us_ChangeFeedTray );
	}
// comment 2016/11/24 MSE寺田 [IT5_3.3][課題1007667] オブザーバ通知で処理済の場合は処理を行わないように修正.
//	if( isComplatePWOnStart()		== True ) { procComplatePWOnStart( option );		}	// 初期立ち上げ終了
	if( (True == isComplatePWOnStart()) && (True != onlyPanel.bl_PwrStatusSleepOutFlg) ) { procComplatePWOnStart( option );		}	// 初期立ち上げ終了
	if( True == isSpecialGamenShowStatus() ) {	// 特殊画面表示の変化チェック.
		if( NULL != PnlComGetDefaultPanelSetting()
		    && TYPE_DefaultPanelSettingOn != PnlComGetDefaultPanelSetting()->getDefaultPanelSetting() )	{	// 共通Myパネル設定無効.
			PnlComApplicationKeyReset( TYPE_ApplicationKeyIdMyPanel );								// アプリケーションMyパネルリセット
		}
		procSpecialGamenShowStatus( option );								// 特殊画面表示の変化通知.
		// 以下アプリキーのLEDを制御
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
	}
	APIC_User* pc_User = PnlComGetUser();
	// カウンター情報保存領域上限2行警告を解除しておく.
	onlyPanel.resetPanelWarning( PNL_ERROR_MaxCounterInfoSaveAreaMessage );
// comment 2020/11/17 2DWS2G-Sky肥後 クラウドSSO認証開発v1_Step2.
	onlyPanel.resetPanelWarning( PNL_ERROR_IdPMaxCounterInfoSaveAreaMessage );
	if( pc_User != NULL ) {
// comment 2020/11/17 2DWS2G-Sky肥後 クラウドSSO認証開発v1_Step2.
// comment 2021/10/8 Sky肥後 [IT6_2.3] クラウドSSO認証開発v2 課金カウンター情報のクラウド通知対応.
//		if ( True == pc_User->isIdpCacheUserNo( pc_User->getUserNo() ) ) {
		if ( ( True == pc_User->isIdpCacheUserNo( pc_User->getUserNo() ) ) 
		||  ( ( TYPE_UserAuthenticationTypeExternalUser == pc_User->getAuthenticationType() ) && (True == PnlComGetUserAuthentication()->getBillingCounterCloudNotification() ) ) ) {
			APIC_IdpCounterInfo* pc_IdpCounterInfo = APIC_IdpCounterInfo::newInstance();
			if( NULL != pc_IdpCounterInfo ){
				if( True == pc_IdpCounterInfo->isIdPCounterOver() ){
// comment 2021/04/07 2DWS2G-Sky毛穴 [IT6_2.2][OP_BTS-32059] カウンタフル全画面警告表示後オートリセットでパブリックログイン.
//					if( 0 == pc_IdpCounterInfo->getStoredNum() ){
					if( ( 0 == pc_IdpCounterInfo->getStoredNum() )
						&& ( &gp_OapSpecifiedAppStarting != PBasicMessageProc::getNowGamen() )		// 初期起動アプリ起動中画面ではポップアップ警告検知しない.
						){
						onlyPanel.setPanelWarning( PNL_ERROR_IdPMaxCounterInfoSaveArea );
					} else {
						onlyPanel.setPanelWarning( PNL_ERROR_IdPMaxCounterInfoSaveAreaMessage );
					}
				}
			}
		} else {
			APIC_Account* pc_Account = APIC_Account::newInstance();
			// ジョブ投入可能確認.
			if( !pc_Account->isEnableEntryJob( pc_User->getUserNo() ) ) {	// 投入不可の場合.
				// Click情報通知状態を判定.
				if( True == pc_Account->isEnableDispClickDBFull( pc_User->getUserNo() ) ) {	// ジョブなし&&Click情報通知中でない場合.
					// カウンター情報保存領域上限警告フラグを設定する.
					onlyPanel.setPanelWarning( PNL_ERROR_MaxCounterInfoSaveArea );
				} else {
					// それ以外の場合.
					// カウンター情報保存領域上限2行警告フラグを設定する.
					onlyPanel.setPanelWarning( PNL_ERROR_MaxCounterInfoSaveAreaMessage );
				}
			}
		}
	}
	if( isNoWarningGamen == False ) {
		if( ( False == isNowPopup( dynamic_cast<PLayerControl*>( &gp_UserAccountUserEndExpand1_act ) ) )
		    && ( False == isNowPopup( dynamic_cast< PLayerControl* >( &gp_EnlAct_UserAccountUserEnd ) ) ) ) {
			if( isBoxSendJobCheck()		== True ) { procBoxSendJobCheck( 0 ); 	}	// 送信ジョブ状態の変化.
		}
	}

	// エラー情報
	if( isFatalExist()  			== True ) { procFatalExist( option ); 				}	// トラブルの発生／解除
	register Bool _bl_isRemoveDocOnGlass = True;
	if( onlyPanel.bl_isEnlargeDisplayMode == True ) {
		register Bool _bl_isGamenWarning   = isGamenWarningExist();
		register Bool _bl_isMessageWarning = isMessageWarningWithGamenExist( &_bl_isRemoveDocOnGlass );
// comment 2015/11/04 MSE田中優介 [ZeusS_ZX0][CQ520656] 拡大表示画面のメッセージ警告の表示タイミングを通常表示画面にあわせるため削除.
//		if( _bl_isGamenWarning == True
//		    || ( _bl_isGamenWarning == False && _bl_isMessageWarning == True && _bl_isRemoveDocOnGlass == False ) ) {
//			// 全画面警告発生、又は、全画面警告発生していなくてメッセージ警告が発生していて、取り忘れ警告以外の時.
//			procWarningGamenExist( 0 );
//		} else if( _bl_isGamenWarning == False && _bl_isMessageWarning == True && _bl_isRemoveDocOnGlass == True ) {
//			// 全画面警告発生していなくてメッセージ警告が発生していて、取り忘れ警告発生の時.
//			procWarningGamenExist( 1 );
//		}
		if( True == _bl_isGamenWarning ) {							// 全画面警告の更新あり.
			// 全画面警告発生時に表示更新を行う.
			procWarningGamenExist( 0 );
		} else if ( ( False == _bl_isGamenWarning )					// 全画面警告の更新なし.
				 && ( True == _bl_isMessageWarning ) ) {			// メッセージ警告の更新あり.
			// メッセージ警告(Popup以外)発生時に表示更新を行う.
			procWarningGamenExist( 1 );
		}
	} else {
		if( isGamenWarningExist()    	== True 											// 全画面警告(ERROR､CAUTION)発生／解除
		    || isMessageWarningWithGamenExist( &_bl_isRemoveDocOnGlass )== True ) { procWarningGamenExist( option );	}	// 画面遷移付きメッセージ警告発生
	}
	// 暫定対策の削除.
	if( onlyPanel.bl_IsKeyCounterMagCardSet == True ) {
		procWarningGamenExist( 0 );		// 状態に応じた画面表示 rebuildGamenをCall.
	}

	if( True == onlyPanel.bl_IsKeyCounterMagCardUnset ) {
		onlyPanel.bl_IsKeyCounterMagCardUnset = False;
		procWarningGamenExist( 0 );		// 状態に応じた画面表示 rebuildGamenをCall.
	}

// comment 2017/03/10 SCD2-Sky浜野 [IT5_4.1][RQ-643][1012954].
//// comment 2017/03/01 SCD2-Sky浜野 [IT5_4.1][RQ-643][1015199] SuperWarp実行条件変更対応.
//	APIC_SystemStatus* pc_SystemStatus = APIC_SystemStatus::newInstance();
//	if( NULL != pc_SystemStatus ){	
//		// LocalUIのモジュール番号(0x19)のSuperWarp実施不可要因を取得.
//		uint32_t ul_CheckCause = pc_SystemStatus->getSuperWarpCause( 0x19 );
//		static uint32_t ul_PreSuperWarpCause = ul_CheckCause;
//		// SuperWarp停止要因が『なし』または、『0x19000008～0x190003E8』（チェック対象）の場合.
//		if( ( 0 == ul_CheckCause )
//		||	( PNLD_SuperWarpCause_0x19000000 == ul_CheckCause )
//		||	( ( PNLD_SuperWarpCause_0x19000008 <= ul_CheckCause ) && ( PNLD_SuperWarpCause_0x190003E9 > ul_CheckCause ) ) ) {
//			uint32_t ul_SuperWarpCause = PNLD_SuperWarpCause_0x19000000;
//			// 警告IDによるSuperWarp実行不可警告発生チェック.
//			for( ushort us_i = 0; us_i < us_SuperWarpDisableWarningSize; us_i++ ) {
//				if( True == onlyPanel.isWarningOn( comPanelJob, tbl_SuperWarpDisableWarning[us_i] ) ) {
//					ul_SuperWarpCause = PNLD_SuperWarpCause_0x19000008 + us_i;
//					break;
//				}
//			}
//			// 警告画面によるSuperWarp実行不可警告発生チェック.
//			for( ushort us_j = 0; us_j < us_SuperWarpDisableGamenSize; us_j++ ) {
//				if( True == PNLC_PopupListControl::newInstance()->isPopupLayer( tbl_SuperWarpDisableGamen[us_j] ) ) {
//					ul_SuperWarpCause = PNLD_SuperWarpCause_0x190001F5 + us_j;
//					break;
//				}
//			}
//			
//			// SuperWarp実施不可有無.
//			if( ul_SuperWarpCause != ul_PreSuperWarpCause ) {
//				pc_SystemStatus->setSuperWarpCause( ul_SuperWarpCause );
//				ul_PreSuperWarpCause = ul_SuperWarpCause;
//			}
//		}
//	}
	// SuperWarp実施可否更新.
	updateSuperWarpDisable();

	onlyPanel.b_KeyCounterPanelCall = False;

	if( isEnlCombinationExist()		== True ) { procEnlCombinationExist( option );		}	// 組合せ禁止解除発生.
	if( isBuzzerWarningExist()		== True ) { procBuzzerWarningExist( option );		}	// ブザーを鳴らす警告発生
	if( isBuzzerWarning1Exist()		== True ) { procBuzzerWarningExist( 100 );			}	// 弱注意音①を鳴らす警告発生
	// モード状態変化
	if( isFeedTrayEnable()			== True ) { procFeedTrayEnable( option );			}	// ＡＰＳによる給紙口指定がチェック
	comPrePanelJob = comPanelJob;					// 今回チェックしたジョブを前回のジョブに入れる
	if( isUSBMemoryAccessErr()			== True ) { procUSBMemoryAccessErr( option );					}	// 外部メモリーアクセスエラー.
	APIC_UsbMemDeviceInfo* pc_USBMemoryInfo = APIC_UsbMemDeviceInfo::newInstance();
	if( False == pc_USBMemoryInfo->isWarning( TYPE_UsbMemWarningNeedPullOut ) ) {	// USBメモリ引き抜き要求状態.
		if( isUSBMemoryTwoInsert()	== True ) { procUSBMemoryTwoInsert( option );	}	// 外部メモリー複数検知警告.
		if( isUSBDetection()		== True ) { procUSBDetection( option );			}	// 外部メモリー検出.
		if( isUSBReadTurnChange()	== True ) { procUSBReadTurnChange( option );	}	// 外部メモリー読み込み中のエラー検出.
		setupUSBLogSystemStoreError();													// 外部メモリーデバッグ情報の蓄積不可エラー.
	}
	PNLE_LogCreateUSBMemory s_LogCreateUSBMemory = isLogCreateUSBMemoryExit();		// ログ蓄積USBメモリー引抜判定.
	if( PNLE_LogCreateUSBMemory_Exit == s_LogCreateUSBMemory ) {
		procLogUSBExit( option );													// ログ蓄積USBメモリー引抜警告設定.
	} else if( PNLE_LogCreateUSBMemory_Enter == s_LogCreateUSBMemory ) {
		procLogUSBEnter( option );													// ログ蓄積USBメモリー引抜警告解除設定.
	}

// comment 2015/03/26 OPS1-Sky新谷 [特ROM][CQ392335]強化機能対応.
	// ログ取得用の特殊なフォルダ構成になっているUSBを挿入した場合.
	if( True == isLogUSBDetection() ) {
		// USBが検知されたことを伝える音を鳴らす.
		setBuzzerType(PCGE_BuzzerType_InputOk);
	}

	short ss_NowGamenWarningNo = PNLC_WarningManager::newInstance()->getGamenWarningNo();
	if( ( ( ( 0 != ss_PreGamenWarningNo ) && ( us_MessageWarningTableSize > ss_PreGamenWarningNo ) )
	      && ( PNL_ERROR_USBMemoryWritting == sp_GamenWarningTable[ss_PreGamenWarningNo].ec_WarningName )	// USBMemory 書き込み中.
	      && ( 0 != ss_NowGamenWarningNo ) && ( ss_NowGamenWarningNo != ss_PreGamenWarningNo ) )			// 警告の切替わり.
	    && ( True == bl_ReqChangeGamen_backup ) ) {														// 画面遷移要求(バックアップ)がON.
		PNLC_WarningManager::newInstance()->setReqChangeGamen( bl_ReqChangeGamen_backup );
	}

	procUSBKeyBoardInsertState( option );		// USBキーボードプラグイン情報検知処理.



	// 仕向け設定.
	TYPE_SystemType guiType = PnlComGetMachineSetting()->getSystemType();
	PnlGuiWrapperSetGuiSystemType( guiType );

	// 言語設定.
	TYPE_Language guiLang = PnlComGetLanguage();
	PnlGuiWrapperSetGuiLanguage( guiLang );

	// キーボード設定.
	guiLang = PnlComGetLanguageForKeyboard();
	PnlGuiWrapperSetGuiLanguageForKeyboard( guiLang );

	// フォントセット設定.
	TYPE_CharSet guiCset = getCharSetForPanel();
	PnlGuiWrapperSetCharSetForPanel( guiCset );

	// 次世代パネル【AR1043942対応】FAX機能有無情報更新.
	PNLC_SlideMenuControlLib::newInstance()->updateFaxExist();

	// DB初期化状態をチェックして、スライドメニューの表示情報を更新する.
	PNLC_SlideMenuControlLib::newInstance()->setupDispInfo();

	// 拡張領域表示統括クラスへ更新要求.
	PNLC_ExtendAreaGeneralization::newInstance()->update();


//---イベント発生以外の定期起床処理---------------------------------------------------
	onlyPanel.ct_Copying1Sec.countUp( 1 );				// プリントキーが押された後、１秒間コピー中にするためのタイマ：カウントアップ
	onlyPanel.c_Deleting1Sec.countTimer( 1 );
	procHintIcon( option );
	setupStatusLEDMosel();		// 状態LEDの処理.
	setupDataLED();				// DataLEDの処理.		// LED制御変更対応.
	// PnlMgrへの警告通知処理.
	onlyPanel.procWarnNoticeToPnlMgr();
	static Bool setObserver = False;
	static TYPE_AppOptSetting isPreview = TYPE_AppOptSettingOn;
	if( ( TYPE_AppOptSettingOn == isPreview ) && ( False == setObserver ) )
	{
		isPreview = SYSF_getAppOptionSetting( TYPE_AppOptionPreview );

		// プレビューロード完了の場合、オブザーバー登録を行う.
		if( True == SYSF_getAppOptionStatus( TYPE_AppOptionPreview ) )
		{
			// バッファ領域初期化.
			c_ThumbnailPreviewControl.initThumbnailPreviewControl();

			APIC_PreviewConverter::newInstance()->attachObserver( &c_RtPreviewObserver );
			setObserver = True;
		}
	}

// Comment VenusMLK -> 2016/07/22 Sky高田(庸) [CQ531088]★A3共通問題★.
	// ジャム解除後ジョブ再開しない状態に陥った場合のリカバリ処理.
	if ( True == PnlComGetOperationSetting()->getPaperJamReleaseProcedureDisplay() ) {
		static short ss_JamRecoveryTimer = 0;
		if ( ( False == onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, PERROR_PrinterJam ) ) &&
			 ( False == onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, PNL_ERROR_MOVIEEND ) ) ) {
			if ( False != APIC_PrinterStatus::newInstance()->getJAMReleaseFlag() ) {
				if ( 30 < ss_JamRecoveryTimer++ ) {
					// 「プリンタジャム解除済み」かつ「動画警告終了が無い」にも関わらずPrintのフラグが残っている状態(異常ケース)が一定時間経過したらジョブを再開させる.
					PNLC_ControlJamReleaseFlow::newInstance()->restartPaperFeedTrayJamJob();
					ss_JamRecoveryTimer = 0;
				}
			} else {
				ss_JamRecoveryTimer = 0;
			}
		} else {
			ss_JamRecoveryTimer = 0;
		}
	}
// Comment VenusMLK <- 2016/07/22 Sky高田(庸) [CQ531088]★A3共通問題★.

// ChameleonNG[RQ-589][AR 1050749] start.
	PNLC_AuthHookControl* pc_HookControl = PNLC_AuthHookControl::newInstance();
	if( PNLE_AuthHookControlStatus_LoginHookIWSWait == pc_HookControl->getLoginHookStatus() ) {
		// まだIWS起動処理中のとき.
		if( PNLE_IWS_Status_Ready == PnlComGetIwsStatus() ) {
		}
		else
		{
			// 再度req_LoginHookProcessをコールする.
			pc_HookControl->req_LoginHookProcess( pc_HookControl->m_pc_HookUser, pc_HookControl->m_e_HookProcClass );
		}
	}
// ChameleonNG[RQ-589][AR 1050749] end.
// Comment 2016/12/20 第2システム制御開発 Sky上野 [ServiceHub] SHRQ-002 SH上でのエラー警告対応.
	checkSHWarning();

// comment 2018/03/16 SCD2 T.Hori [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_3ペインメニュー対応]. Start.
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
	//IDキー表示／非表示チェック
	Bool bl_IDKeyCheck = isDisplayIDKeyByAuthenticationStatus();

	//ログイン状態変更チェック.
	Bool bl_LoginCheck = False;

	APIC_User* pc_UserCheck = PnlComGetUser();
	if( NULL != pc_UserCheck ){
		//ログイン状態.
		bl_LoginCheck = True;
	}

	//3ペインIDキーアイコン変更.
	PNLC_SH3PaneMenuControlLib::newInstance()->setIDKeyIcon( GFWE_SH3PANEMENU_SWHARDKEY_IDINPUT,
									(True == bl_IDKeyCheck)? GFWE_SH3PANEMENU_KEYDISPSTATE_SHOW : GFWE_SH3PANEMENU_KEYDISPSTATE_HIDE,
									(True == bl_LoginCheck)? GFWE_SH3PANEMENU_LOGINSTATE_LOGIN : GFWE_SH3PANEMENU_LOGINSTATE_LOGOUT );

// comment 2018/03/16 SCD2 T.Hori [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_3ペインメニュー対応]. End.

// comment 2018/10/08 Sky水野 [Eagle][AR1062683] フッター対応.
	// 言語によるリセット文字列幅チェック.
	Bool bl_ResetKeyCheck = isWidthResetByLanguage();
	// 3ペインリセットキーアイコン変更.
	PNLC_SH3PaneMenuControlLib::newInstance()->setResetKeyDispPattern(
									(True == bl_ResetKeyCheck)? GFWE_SH3PANEMENU_RESETPTN_WIDE : GFWE_SH3PANEMENU_RESETPTN_NORMAL );
#endif //#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.
	
// comment 2018/06/07 SCD2-Sky中野 [Eagle] Legacy追加対応.
	// ベルアイコン表示状態を通知.
	PNLC_WebUIControl* pc_WebUIControl = PNLC_WebUIControl::getInstance();
	if( NULL != pc_WebUIControl ) {
		pc_WebUIControl->requestBellIconStatus( isDisplayBellIcon() );

// comment 2022/06/07 1DWS2G-Sky [SecV6][RQ-909] ウイルススキャンアイコン表示.
		TYPE_VIRUSSCAN_ICON_STATUS e_icon_status = TYPE_VIRUSSCAN_ICON_STATUS_UNDECIDED;
		if ( True == PnlComIsVirusScanEnable() ) {

			// 有効状態である場合、ステータスを取得し、GUI側の定数に変換する.
			switch( PnlComGetVirusScanStatus() ) {
			case PNLE_MESSAGEAREA_VIRUSSCAN_ACTIVE:			e_icon_status = TYPE_VIRUSSCAN_ICON_STATUS_ACTIVE;			break;
			case PNLE_MESSAGEAREA_VIRUSSCAN_DETECTION:		e_icon_status = TYPE_VIRUSSCAN_ICON_STATUS_DETECT;			break;
			case PNLE_MESSAGEAREA_VIRUSSCAN_PATTERN_NG:		e_icon_status = TYPE_VIRUSSCAN_ICON_STATUS_PATTERNERROR;	break;
			default: break;
			}
			pc_WebUIControl->requestVirusScanIconStatus(e_icon_status);
		} else {
			pc_WebUIControl->requestVirusScanIconStatus(TYPE_VIRUSSCAN_ICON_STATUS_NONDISP);
		}
	
	}
// comment 2020/03/10 SCD2-Sky中野 [IT6_2.0MR2] 制御No.30_Program登録機能.
	if( ( PNLE_HomeProgramSearchStatusWaitDBInitialize == PNLC_ProgramKeyDataGetForHome::getInstance()->getProgramSearchStatus() )
	 && ( True == getDBInitializeStatus() ) ) {
		// プログラム宛先検索状態がDB初期化完了待ち、かつ、DB初期化完了.
		// ⇒プログラム宛先検索を開始する.
		PNLC_ProgramKeyDataGetForHome::getInstance()->startSearchProgramKeyInfo();
	}
	
// comment 2020/04/09 SCD2-Sky中野 [IT6_2.0MR2] DefaultAPPの機能拡張対応.
	if( True == PNLC_BrowserChangeTabControl::getInstance()->getWaitInitializeDatabaseFlag() ) {
		if( True == APIC_DatabaseStatus::newInstance()->getInitializeDatabase() ) {
			// DefaultAPPがOapアプリの場合はデータベース初期化完了までIGCWaitを表示するため、.
			// データベース初期化完了時にタブ切替可能かチェックする.
			PNLC_BrowserChangeTabControl::getInstance()->checkEnableChangeTab();
			PNLC_BrowserChangeTabControl::getInstance()->setWaitInitializeDatabaseFlag( False );
		}
	}
}
// Comment [IT6-Eagle] 機器構成変更No.2 HCLT-Deeparaj 2018/02/20 To Check the Paper Insert / Remove status for Tray 1.
#ifndef DEF_A4_Cus
Bool PBasicMessageProc::checkPaperInsertToTray1Feed()
{
//Comment 2019/12/19 【HCLT-Paul】【IT6_EagleBk】【OP_BTS-18230】【UD評価】【追加対策】 not applicable for Dnbmlk
#if	defined(DEF_DenebMLKCus)
	return False;
#endif
	Bool bl_ExistingPaperSize  = False;
// Comment 2019/06/07 【HCLT-Harish】【IT6_EagleH】【OP_BTS-7463】Avoid setting "TYPE_SizeCodeDontCare" .
//	static TYPC_PaperSize c_PrePaperSize(TYPE_SizeCodeA3Sef) ;
//comment 2019/02/01 [HCL-Sarada] [EagleH] [OP_BTS-1219] Added Non standard size for B6 or less size.
	static TYPC_PaperSize c_B6SefPaperSize(TYPD_SizeCodeB6Sef) ;
	TYPC_PaperSize c_PaperSize ;
// Comment 2019/06/07 【HCLT-Harish】【IT6_EagleH】【OP_BTS-7463】Avoid setting "TYPE_SizeCodeDontCare" .
//	Bool bl_PrePaperSizeIsB6Less = False;
	Bool bl_ret = False;
	c_PaperSize = ppc_FeedTray->getPaperSize( TYPE_FeedTray1 );
	for( int i = 0; i < sizeof( tbl_lessB6Sizes )/sizeof( TYPE_SizeCode ); i++ ) {
		if (  c_PaperSize.e_SizeCode == tbl_lessB6Sizes[i] ) {
			if	(	TYPE_SizeCodeNonStandard != tbl_lessB6Sizes[i] ) {
				bl_ExistingPaperSize = True;
// Comment 2019/06/07 【HCLT-Harish】【IT6_EagleH】【OP_BTS-7463】Avoid setting "TYPE_SizeCodeDontCare" .
				bl_ret = True;
			}	else if	( c_PaperSize.c_SizeMM.x <= c_B6SefPaperSize.c_SizeMM.x && c_PaperSize.c_SizeMM.y <= c_B6SefPaperSize.c_SizeMM.y ){
					bl_ExistingPaperSize = True;
// Comment 2019/06/07 【HCLT-Harish】【IT6_EagleH】【OP_BTS-7463】Avoid setting "TYPE_SizeCodeDontCare" .
					bl_ret = True;
			}
           	break;
		}
	}
// Comment 2019/06/07 【HCLT-Harish】【IT6_EagleH】【OP_BTS-7463】Avoid setting "TYPE_SizeCodeDontCare" .
//	for ( i = 0; i < sizeof( tbl_lessB6Sizes )/sizeof( TYPE_SizeCode ); i++ ) {
//		if ( c_PrePaperSize.e_SizeCode == tbl_lessB6Sizes[i] ) {
//			if	(	TYPE_SizeCodeNonStandard != tbl_lessB6Sizes[i] ) {
//				bl_PrePaperSizeIsB6Less = True;
//			}	else if	( c_PrePaperSize.c_SizeMM.x <= c_B6SefPaperSize.c_SizeMM.x && c_PrePaperSize.c_SizeMM.y <= c_B6SefPaperSize.c_SizeMM.y ){                                                                                                                    
//					bl_PrePaperSizeIsB6Less = True;                                                                                                                         
//			}
//			break;                                                               
//		}
//	}
	if ( True == bl_ExistingPaperSize ) {
//comment 2018/04/08 【HCLT-Prasanjet】 【IT6 EAGLE】 【AR1042477】 remove previous and current paper size check..
//		if( c_PrePaperSize.e_SizeCode != c_PaperSize.e_SizeCode ){
// Comment 2019/06/07 【HCLT-Harish】【IT6_EagleH】【OP_BTS-7463】Avoid setting "TYPE_SizeCodeDontCare" .
//			if ( False == bl_PrePaperSizeIsB6Less){
//				APIC_SystemSetting* p_SystemSetting = PnlComGetSystemSetting();
//				p_SystemSetting->setLastMainUnitTrayEnvelopeSize(TYPE_SizeCodeDontCare);
//			}
//			bl_ret =True;
//		}		
//	}
// comment 2018/05/17 [HCL-Deeparaj] [Eagle] [AR1044529] Set the previous size for tray 1.
	} else {
		APIC_SystemSetting* p_SystemSetting = PnlComGetSystemSetting();
		p_SystemSetting->setLastMainUnitTrayEnvelopeSize(TYPE_SizeCodeDontCare);
	}
// Comment 2019/06/07 【HCLT-Harish】【IT6_EagleH】【OP_BTS-7463】Avoid setting "TYPE_SizeCodeDontCare" .
//	c_PrePaperSize.e_SizeCode = c_PaperSize.e_SizeCode;
	return bl_ret;
}
#endif	//DEF_A4_Cus.


//================================================================================================
//［機能］ データの変化エッジチェック : ファクステスト結果の変化
//================================================================================================
void PBasicMessageProc::procFaxTestResultChange( uchar option, TYPE_CommLine e_FaxLineNo, TYPS_TestResult s_TestResult )
{
	procFaxTestResult( option, e_FaxLineNo, s_TestResult ) ;	// テスト結果通知
}
/*================================================================================================
［機能］	基本メッセージ処理 :  ファクスボードからのコマンドレスポンス通知の変化
================================================================================================*/
void PBasicMessageProc::procFaxCmdRsChange( uchar option, TYPE_CommLine e_CommLine, FAXS_CmdRs s_CmdRs )
{
	// コマンド要求中状態フラグをクリアする.
	switch( e_CommLine ) {
		case TYPE_CommLine1: e_isFaxLine1CmdReq = FAXE_CmdRsDummy;	break;
		case TYPE_CommLine2: e_isFaxLine2CmdReq = FAXE_CmdRsDummy;	break;
		case TYPE_CommLine3: e_isFaxLine3CmdReq = FAXE_CmdRsDummy;	break;
		case TYPE_CommLine4: e_isFaxLine4CmdReq = FAXE_CmdRsDummy;	break;
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); break;
	}
	// コマンドレスポンスIDを判定.
	switch( s_CmdRs.e_CmdRsID ) {
		case FAXE_CmdRsKtd:	 procFaxCmdRsKtd( option, e_CommLine, s_CmdRs );	break;	// コマンドレスポンスダイアル完了.
		case FAXE_CmdRsKtm:	 procFaxTestInto( option, e_CommLine, s_CmdRs );	break;	// コマンドレスポンステストモードに入る完了.
		case FAXE_CmdRsKtm2: procFaxCmdRsKtm2( option, e_CommLine, s_CmdRs );	break;	// コマンドレスポンステストモードから抜ける完了.
		case FAXE_CmdRsKtn:	 procFaxCmdRsKtn( option, e_CommLine, s_CmdRs );	break;	// コマンドレスポンステストストップ完了.
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); break;
	}
}
/*================================================================================================
［機能］	基本メッセージ処理 :  ファクス回線1状態の変化
================================================================================================*/
void PBasicMessageProc::procFaxLine1LineStatusChange( uchar option )
{
	APIC_FaxDevice* pc_FaxDevice;
	Bool b_IsOffHook;
	pc_FaxDevice = APIC_FaxDevice::newInstance();
	if( pc_FaxDevice == NULL ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	if( ( b_isOffHookBtnOn == True )								// 手動オフフック状態判定.
	    &&( ( pc_FaxDevice->getLineStatus( TYPE_CommLine1 ) == 0 )	// 回線１の回線状態検知(Idle判定).
	        ||( b_isFaxLine1OffHook == True ) ) ) {						// 外部電話オフフック状態判定.
		// 手動オフフック状態時に、回線がIdleまたは外付け電話オフフック状態になった場合.
		b_isOffHookBtnOn = False;								// 手動オフフック状態解除.
		procFaxLine1StatueIdle( option );						// ファクス回線1状態変化（OnHook）
		return;
	}
	// 回線1に対する回線状態（外付け電話が上げられたかどうか）を取得する
	b_IsOffHook = pc_FaxDevice->isOffHook( TYPE_CommLine1 );
	if( b_IsOffHook == True ) {
		b_isFaxLine1OffHook = True;
// [KronosS] 管理No.269_3 スキャナセンサー変更 SCD2 3G Sky西場 2016/11/17 start.
// [CQ533028] オフフックの送信、受信選択 SCD2 3G Sky西場 2016/08/08 start.
// [Sparrow]　機器構成変更_No.304_スキャナセンサー変更 SCD2 4G Sky錦 2017/12/11 start.
#if defined(DEF_VenusMLK)
		PCGC_ScanFaxBaseCom::setUserSelectOffHookTxRxMode( PNLE_FaxRbtSelTxRxModeNonSel );
#endif
// [CQ533028] オフフックの送信、受信選択 SCD2 3G Sky西場 2016/08/08 end.
// [KronosS] 管理No.269_3 スキャナセンサー変更 SCD2 3G Sky西場 2016/11/17 end.
// [Sparrow] 機器構成変更_No.304_スキャナセンサー変更 SCD2 4G Sky錦 2017/12/11 end.
		// OffHook（＝外付け電話の受話器が上げられた）
		procFaxLine1StatueComm( option );	// ファクス回線1状態変化（OffHook）
	} else {
		b_isFaxLine1OffHook = False;
// comment 2015/12/01 OPS1-Sky稲井 [特ROM][IT5-8_機種マージ]e-Mail受信時、本文を印刷する要望(他1件).
//		// OnHook（＝外付け電話の受話器が置かれた）
//		procFaxLine1StatueIdle( option );	// ファクス回線1状態変化（OnHook） 
// comment 2019/01/07 [IT6_EagleH] [RQ-732] [HCLT-Satheesh] Make DipSw No.226 bit4 Functionlaity invalid and Use CommonAPI check.
		if( True == PnlComGetFaxPanelSetting()->getProhibitedExternalTEL() ){	// Use Admin Setting API.
			if(True == PNLC_PopupListControl::newInstance()->isNowPopup( &gp_WarningHandsetoffhookError_warn )){
				if(this != &gp_WarningHandsetoffhookError_warn){
					//現在警告は表示しているが、画面インスタンスのポインタがずれている場合は、
					PBasicMessageProc* p = (PBasicMessageProc*)(&gp_WarningHandsetoffhookError_warn);
					p->procFaxLine1StatueIdle( option );
				}else{
					// OnHook（＝外付け電話の受話器が置かれた。基本的にハンドセットオフ警告のidleをCALL）
					procFaxLine1StatueIdle( option );	// ファクス回線1状態変化（OnHook） 
				}
			}else{
				// OnHook（＝外付け電話の受話器が置かれた）
				procFaxLine1StatueIdle( option );	// ファクス回線1状態変化（OnHook） 
			}
		}else{
			// OnHook（＝外付け電話の受話器が置かれた） 
			procFaxLine1StatueIdle( option );	// ファクス回線1状態変化（OnHook） 
		}
	}
}
/*================================================================================================
［機能］	基本メッセージ処理 :  ネーム・ナンバーディスプレイ表示状態変化
================================================================================================*/
void PBasicMessageProc::procFaxNumberDispChange( uchar option, TYPE_CommLine e_CommLine )
{
	PDPC_FaxCommonApi*	pc_FaxCommonApi ;
	Bool	b_Line1DispRet = False;
	Bool	b_Line2DispRet = False;
	pc_FaxCommonApi = PDPC_FaxCommonApi::getInstance();
	if( pc_FaxCommonApi == NULL ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	switch( e_CommLine ) {
		case TYPE_CommLine1:	// 回線1
			if( b_DispDataCatch1 == False ) {
				// 回線1のネーム・ナンバーディスプレイ表示内容未取得の場合取得する
				b_Line1DispRet = pc_FaxCommonApi->checkFaxNumberDispKind( TYPE_CommLine1 );
				if( b_Line1DispRet == True ) {
					// ネーム・ナンバーディスプレイの表示内容を取得したなら表示ありに設定
					b_DispDataCatch1 = True;
				}
			} else {
				// 回線1のネーム・ナンバーディスプレイ表示内容取得済みの場合初期化する
				pc_FaxCommonApi->resetFaxCallDisp( TYPE_CommLine1 );
				b_DispDataCatch1 = False;
			}
			break;
		case TYPE_CommLine2:	// 回線2
			if( b_DispDataCatch2 == False ) {
				// 回線2のネーム・ナンバーディスプレイ表示内容未取得の場合取得する
				b_Line2DispRet = pc_FaxCommonApi->checkFaxNumberDispKind( TYPE_CommLine2 );
				if( b_Line2DispRet == True ) {
					// ネーム・ナンバーディスプレイの表示内容を取得したなら表示ありに設定
					b_DispDataCatch2 = True;
				}
			} else {
				// 回線2のネーム・ナンバーディスプレイ表示内容取得済みの場合初期化する
				pc_FaxCommonApi->resetFaxCallDisp( TYPE_CommLine2 );
				b_DispDataCatch2 = False;
			}
			break;
		case TYPE_CommLine3:	// 回線3.
			if( False == bl_DispDataCatch3 ) {
				// 回線3のナンバーディスプレイ表示内容未取得の場合取得する.
				if( True == pc_FaxCommonApi->checkFaxNumberDispKind( TYPE_CommLine3 ) ) {
					// ナンバーディスプレイの表示内容を取得したなら表示ありに設定.
					bl_DispDataCatch3 = True;
				}
			} else {
				// 回線3のナンバーディスプレイ表示内容取得済みの場合初期化する.
				pc_FaxCommonApi->resetFaxCallDisp( TYPE_CommLine3 );
				bl_DispDataCatch3 = False;
			}
			break;
		case TYPE_CommLine4:	// 回線4.
			if( False == bl_DispDataCatch4 ) {
				// 回線4のナンバーディスプレイ表示内容未取得の場合取得する.
				if( True == pc_FaxCommonApi->checkFaxNumberDispKind( TYPE_CommLine4 ) ) {
					// ナンバーディスプレイの表示内容を取得したなら表示ありに設定.
					bl_DispDataCatch4 = True;
				}
			} else {
				// 回線4のナンバーディスプレイ表示内容取得済みの場合初期化する.
				pc_FaxCommonApi->resetFaxCallDisp( TYPE_CommLine4 );
				bl_DispDataCatch4 = False;
			}
			break;
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	return;
	}
}
/*================================================================================================
［機能］ その他　子クラスから使用する関数 : 前の画面設定を現在の画面にする
================================================================================================*/
void PBasicMessageProc::setTurnGamenNow()
{
	turnGamen = nowGamen;
	turnGamenBackup = NULL;
}
/*================================================================================================
［機能］ その他　子クラスから使用する関数 : 前の画面設定を指定した画面にする
================================================================================================*/
void PBasicMessageProc::setTurnGamenThis( PBasicMessageProc* thisGamen )
{
	turnGamen = thisGamen;
	turnGamenBackup = NULL;
}
/*================================================================================================
［機能］ その他　子クラスから使用する関数 : 現在の画面を退避する
================================================================================================*/
void PBasicMessageProc::setTurnGamenBackupNow()
{
	turnGamenBackup = nowGamen;
}
/*================================================================================================
［機能］ その他　子クラスから使用する関数 : 指定した画面がnowGamenかどうか判断する
【履  歴】 [restoreGamen対応] 2008/09/27  桑山.
================================================================================================*/
Bool PBasicMessageProc::isNowGamen( PBasicMessageProc* thisGamen )
{
	PBasicMessageProc*	nowGamenBaseModel;

	nowGamenBaseModel = PLBC_FuncVersionCheck::restoreGamen( nowGamen );
	nowGamenBaseModel = PLBC_ReplaceCheck_PKI::restoreGamen( nowGamenBaseModel );
	nowGamenBaseModel = restoreGamen( nowGamenBaseModel );

	return ( thisGamen == nowGamenBaseModel )? True : False;
}
/*================================================================================================
［機能］ その他　子クラスから使用する関数 : 指定した画面がnowPopupかどうか判断する
【履  歴】 2010/06/02  水嶋.
================================================================================================*/
Bool PBasicMessageProc::isNowPopup( PLayerControl* thisPopup )
{
	return ( PNLC_PopupListControl::newInstance()->getNowPopup() == thisPopup ) ? True:False;
}

/*================================================================================================
［機能］ その他　子クラスから使用する関数  : 現在の画面を取得にする.
【履  歴】 [restoreGamen対応] 2008/09/27  桑山.
================================================================================================*/
PBasicMessageProc* PBasicMessageProc::getNowGamen()
{
	PBasicMessageProc*	nowGamenBaseModel;

	nowGamenBaseModel = PLBC_FuncVersionCheck::restoreGamen( nowGamen );
	nowGamenBaseModel = PLBC_ReplaceCheck_PKI::restoreGamen( nowGamenBaseModel );
	nowGamenBaseModel = restoreGamen( nowGamenBaseModel );
	return nowGamenBaseModel;

}
//================================================================================================.
// 機能    : ブラウザーモード使用中かどうかを判定.
// comment : WebUI2 画面切替え制御 20180302 Chikuan.
//================================================================================================.
Bool PBasicMessageProc::isUsingBrowserMode()
{
	PM_TaskID sc_TaskID;
	PMGC_PanelManager::getPriority( &sc_TaskID );
	if( PM_TASKID_PANEL != sc_TaskID ){
		return True;
	}

	POAP_BaseGamenCom* p_Gamen1 = dynamic_cast< POAP_BaseGamenCom* >( nowGamen );
	PCGC_BrowserFileUploadComBase* p_Gamen2 = dynamic_cast< PCGC_BrowserFileUploadComBase* >( nowGamen );
	PCGC_WarningPopupDummyBase* p_Gamen3 = dynamic_cast< PCGC_WarningPopupDummyBase* >( nowGamen );
// comment 2018/07/02 Chikuan [Eagle][Accurev1050004]IWSアプリでログインできない不具合対策 WebUIV2の再起動抑止条件を追加.
//	if( ( NULL != p_Gamen1 ) || ( NULL != p_Gamen2 ) || ( NULL != p_Gamen3 ) ){
	PCGC_IwsAuthBrowserLogin* p_Gamen4 = dynamic_cast< PCGC_IwsAuthBrowserLogin* >( nowGamen );
	PCGC_IwsAuthBrowserWait* p_Gamen5 = dynamic_cast< PCGC_IwsAuthBrowserWait* >( nowGamen );
	if( ( NULL != p_Gamen1 ) || ( NULL != p_Gamen2 ) || ( NULL != p_Gamen3 ) || ( NULL != p_Gamen4 ) || (  NULL != p_Gamen5  ) ){
		return True;
	}

	// Comment : 2018/07/09 Chikuan [Eagle][Accurev:1051267] ブラウザーモードがOnであれば、WebUIV2の再起動を抑止するよう修正.
	if( ( True == ScanOapExtSet.isActiveGeneralApplication() )
//	 || ( True == onlyPanel.bl_BrowserFileUploadRequest ) ){
	 || ( True == onlyPanel.bl_BrowserFileUploadRequest )
	 || ( True == onlyPanel.getBrowserMode() ) ){
		return True;
	}

	return False;
}

/*-----------------------------------------------------------------------------------------------------------
	 チェック＆画面移動関数（外部参照不可）
-----------------------------------------------------------------------------------------------------------*/
/*================================================================================================
注意！！
	ジョブリスト全画面警告遷移／サービス全画面警告遷移は、ジョブリストキーの押下からも呼ばれる為、
	子クラスから参照可能な関数とした為、「子クラスから参照する関数」に含まれる
================================================================================================*/
//================================================================================================
//［機能］ チェック＆画面移動関数（外部参照不可） : 動作中画面チェック（拡大表示用）
//================================================================================================
Bool PBasicMessageProc::isGamenNowMovementEnlarge( Bool _bl_StopOnly )
{
	register Bool bl_result = False;			// チェック結果.
	register Bool bl_underStopGamen = False;	// 停止中画面より優先順位が低い（True：低い）.
	PNLC_SendingGamenGetJob		pc_GetJob;		// ジョブリストからファクス送信ジョブ取得用.
// Comment 2017/05/05 HCLT-Prem [IT5_4.1][1015276]Declaring an instance for c_MmbStopGamenGetJob.
	PCGC_StopGamenGetJob c_MmbStopGamenGetJob;
	if( True == PLBC_FuncPKICheck::isPKI() && NULL == PnlComGetUser() ) {
		switch( onlyPanel.ep_PanelStatus ) {	// 読込処理中は処理が終了するまで表示させる.
			case EP_PNS_ReadExec:			// 読込中.
			case EP_PNS_ReadErrorStop:		// 読込エラー.
			case EP_PNS_ReadStop:			// 読込停止中.
			case EP_PNS_SeparateScanStop:	// 分割読込停止中.
				break;
			default:
				return False;
		}
	}
	//パネルステータスをチェックして遷移する画面を決定する.
	switch( onlyPanel.ep_PanelStatus ) {
		case EP_PNS_ReadExec :					// 読込中.
			if( _bl_StopOnly == False ) {			// 全動作中状態表示指示時のみチェックする.
				if( onlyPanel.bl_StopProcess == True
				    || onlyPanel.ec_IntMode == EC_IntMode_EnterProcess
				    || PDPE_FaxSendQuickStopProcess == c_JobStatusControl.checkFaxSendQuickStop( comPanelJob ) ) {
					// ストップ動作中の設定、又は、割り込み遷移移行中、又は、FAXクイック送信停止遷移中.
					nextGamen = &gp_EnlAct_ActionStopSoon;
				} else if( PDPE_FaxSendQuickStop == c_JobStatusControl.checkFaxSendQuickStop( comPanelJob ) ) {	// FAXクイック送信停止中.
// [CQ.339175]UV-00644：拡大表示デフォルト対応の機種対応.
//#ifdef ROM_DIV_2
//					nextGamen = (PCGC_ActionJobDeleteListEnl*)pt_gp_ActionStopEnl;	// 停止中画面(L1212)に遷移
//#else	/* #ifdef ROM_DIV_2 */
// comment UV-00731 Fax動作中にストップキーを効くようにする対応(拡大画面) Y.itoh ---->.
//					nextGamen = &gp_ActionStopEnl;									// 停止中画面(L1212)に遷移
// comment MinervaZeus -> Sky西原 2011/08/05 [CQ:350009].
//// comment CQ349083 対策 2011/06/24 1OSW 伊藤義弘 ---->.
////					if ( PNLE_FaxSendStopTrigger == onlyPanel.e_FaxJobStoped ) {
//					if ( (PNLE_FaxSendStopTrigger == onlyPanel.e_FaxJobStoped)
//					  && (nowGamen != &gp_FaxReadJobDeleteConfirmEnl ) ) {
//// comment CQ349083 対策 2011/06/24 1OSW 伊藤義弘 <----.
// comment 2015/01/14 OPS1-HCL-Vijayendran [Poseidon][CQ.508257]If there are sending jobs, then transit to 10M24000_停止中(通信).
//					if ( PNLE_FaxSendStopTrigger == onlyPanel.e_FaxJobStoped ){
					if ( False == pc_GetJob.isNowSendingJob() ){
//						// 読込み中ジョブ削除確認へ遷移.
//						nextGamen = &gp_FaxReadJobDeleteConfirmEnl;
						onlyPanel.e_FaxJobStoped = PNLE_FaxSendStopNon;
					} else {
						nextGamen = &gp_EnlAct_ActionStop;
					}
				} else {
					nextGamen = &gp_EnlAct_ActionScanPrint;
				}
				bl_result = True;				// 画面遷移した！.
			}
			break;
		case EP_PNS_PrintExec :					// プリント中.
			if( _bl_StopOnly == False ) {			// 全動作中状態表示指示時のみチェックする.
				if( onlyPanel.bl_StopProcess == True
				    || onlyPanel.ec_IntMode == EC_IntMode_EnterProcess ) {
					// ストップ動作中の設定、又は、割り込み遷移移行中.
					nextGamen = &gp_EnlAct_ActionStopSoon;
				} else {
					nextGamen = &gp_EnlAct_ActionPrinting;
				}
				bl_result = True;				// 画面遷移した！.
			}
			break;
		case EP_PNS_ReadErrorStop :				// 読込エラー.
		case EP_PNS_ReadStop :					// 読込停止中.
		case EP_PNS_PrintErrorStop :			// プリントエラー.
		case EP_PNS_PrintStop :					// プリント停止中.
		case EP_PNS_VenderJobOutOfMoneyStop:	// ベンダーお金不足停止中.
			// IDキーによるマニュアルログアウトのときは停止中画面に遷移しない.
			if( onlyPanel.ep_UserAccountEnd == EP_UserAccountEndIDInput ) {
				return False;
			}
// Comment 2017/05/05 HCLT-Prem [IT5_4.1][1015276]Adding condition to launch the read stop screen.
			if( ( 0 < c_MmbStopGamenGetJob.getNowStopJob() )
			|| ( True == pc_GetJob.isNowSendingJob() ) ) {
				nextGamen = &gp_EnlAct_ActionStop;
			}
			if( ( True == comPanelJob->isWarning( ERROR_JimonPasswordReadError ) )
			  )
			{
				bl_result = True;				// 画面遷移した！
				break;
			}
			bl_result = True;					// 画面遷移した！.
			bl_underStopGamen = True;			// 読み込み停止時プリントが停止遷移中であれば、もうすぐ止まる画面表示のため対応.
			break;
		case EP_PNS_SeparateScanStop :			// 分割読込停止中.
			nextGamen = &gp_EnlAct_ActionReadStop;
			bl_result = True;						// 画面遷移した！.
			bl_underStopGamen = True;				// 停止中画面より優先順位が低い.
			break;
		case EP_PNS_PrintWait:					// プリント待機中.
			if( _bl_StopOnly == False ) {
				nextGamen = &gp_EnlAct_ActionPrintWaiting;
				bl_result = True;				// 画面遷移した！.
				bl_underStopGamen = True;		// 停止中画面より優先順位が低い.
			}
			break;
		case EP_PNS_ExSeparateScanStop :		// 拡張分割読込停止中.
		case EP_PNS_Setting :					// 設定状態.
			if( ( comPanelJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusPrintWait || comPanelJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusOutputExec )
			    && comPanelJob->getJobStatus()->getInputStatus() == TYPE_InputStatusScanEnd
			    && comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() !=  TYPE_JobTypeTwain ) {
				nextGamen = &gp_EnlAct_ActionScanPrint;
				bl_result = True;				// 画面遷移した！.
			} else if( PNLE_FaxSendStopNon != onlyPanel.e_FaxJobStoped ) {
				if( True == pc_GetJob.isNowSendingJob() ) {
					switch( onlyPanel.e_FaxJobStoped ) {
						case PNLE_FaxSendStopTrigger:	// 停止受付時.
							// 停止中画面への遷移.
							nextGamen = &gp_EnlAct_ActionStop;
							onlyPanel.e_FaxJobStoped = PNLE_FaxSendStopJobList;
							break;
						case PNLE_FaxSendStopJobList:	// 引き続き停止中時.
							// 停止中画面への遷移.
							nextGamen = &gp_EnlAct_ActionStop;
							break;
						default:	break;
					}
					bl_result = True;				// 画面遷移した！(初回の画面遷移でなくてもステータス変化による自動遷移を防ぐ).
				} else {
					onlyPanel.e_FaxJobStoped = PNLE_FaxSendStopNon;
				}
			} else {
				bl_result = False;				// 画面遷移しなかった.
			}
			break;
		case EP_PNS_PrintStopSingleStep:
			// システムオートリセットでジョブ蓄積とリセットを同時に行う.
			// 同時に行った場合、ジョブは蓄積済みであるが onlyPanel.ep_PanelStatus = EP_PNS_PrintStopSingleStep.
			// の状態になる。ここで、ジョブのステータスをチェックして確認コピー停止中でなければ Falseを戻り値とする.
			if( comPanelJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusPrintStopSingleStep ) {
				nextGamen = &gp_EnlAct_ActionSingleCopyStop;
				bl_result = True;				// 画面遷移した！.
			} else {
				bl_result = False;				// 画面遷移しなかった
			}
			break;
		case EP_PNS_Receive:
			if( True == onlyPanel.bl_isEnlargeDisplayMode ) {
				nextGamen = &gp_EnlAct_ActionFax; // 受信中.
			} else {
			}
			bl_result = True;							// 画面遷移した！.
			bl_underStopGamen = True;					// 停止中画面より優先順位が低い.
			break;
		case EP_PNS_Send:
		case EP_PNS_Dialing:
		case EP_PNS_DialingWait:
		case EP_PNS_ReDialingWait:
			if( True == onlyPanel.bl_isEnlargeDisplayMode ) {
				if( PNLE_FaxSendStopNon != onlyPanel.e_FaxJobStoped ) {
					if( True == pc_GetJob.isNowSendingJob() ) {
						switch( onlyPanel.e_FaxJobStoped ) {
							case PNLE_FaxSendStopTrigger:	// 停止受付時.
								onlyPanel.e_FaxJobStoped = PNLE_FaxSendStopJobList;
							// breakなし.
							case PNLE_FaxSendStopJobList:	// 停止中画面.
								nextGamen = &gp_EnlAct_ActionStop; // 停止中画面への遷移.
								break;
							default:	break;
						}
						bl_result = True;				// 画面遷移した！(初回の画面遷移でなくてもステータス変化による自動遷移を防ぐ).
					} else {
						onlyPanel.e_FaxJobStoped = PNLE_FaxSendStopNon;
					}
				} else {
					nextGamen = &gp_EnlAct_ActionFax; // 受信中.
					bl_result = True;				// 画面遷移した！.
				}
			} else {
			}
			bl_underStopGamen = True;		// 停止中画面より優先順位が低い.
			break;
		case EP_PNS_BroserFileUploadExec:	// Webブラウザーファイルアップロード用ファイル作成中.
			// 何もしない.
			break;
		case EP_PNS_PKISend:				// PKI認証送信中.
			nextGamen = &gp_EnlAct_ActionSendPKICertify_PKI;
			bl_result = True;
			break;
		case EP_PNS_PKISendSmbStop:			// SMB 送信警告停止中.
			nextGamen =	&gp_EnlAct_ActionCertifySMBUser_PKI;
			PCGC_PkiCardManager_PKI::getInstance()->setSmbHostSetFlg( True );
			bl_result = True;
			break;
		case EP_PNS_PKISendSmimeStop:		// S/MIME 送信警告停止中.
			PCGC_PkiCardManager_PKI::getInstance()->setPincodeInputKind( DEF_INPUTPINCODE_SMIME );
			nextGamen = &gp_EnlAct_PincodeInput_PKI;
			bl_result = True;
			break;
		case EP_PNS_OcrReadExec:	// OCR読込み中.
			if( False == _bl_StopOnly ) {		// 全動作中状態表示指示時のみチェックする.
				if( ( True == onlyPanel.bl_StopProcess )
				    || ( EC_IntMode_EnterProcess == onlyPanel.ec_IntMode )
				    || ( PDPE_FaxSendQuickStopProcess == c_JobStatusControl.checkFaxSendQuickStop( comPanelJob ) ) ) {	// ストップ動作中の設定、又は、割り込み遷移移行中、又は、FAXクイック送信停止遷移中.
					nextGamen = &gp_EnlAct_ActionStopSoon;
				} else if( PDPE_FaxSendQuickStop == c_JobStatusControl.checkFaxSendQuickStop( comPanelJob ) ) {		// FAXクイック送信停止中.
					nextGamen = &gp_EnlAct_ActionStop;
				} else {
				//comment 2021/08/04 FPT.HungDm17 【OP_BTS-36085】「もうすぐ止ります」の画面がずっと表示された、ホームキーを押すと、「OCR読込み中です、しばらくお待ちください。」のメッセージがずっと表示される、送信できない問題対策.
					PCGC_JobSpecScanFaxBackup* pc_JobSpecScanFaxBackup = ( PCGC_JobSpecScanFaxBackup* )comPanelJob->getAppInfo();
					if( ( NULL != pc_JobSpecScanFaxBackup )
					    && ( True == pc_JobSpecScanFaxBackup->b_OCRReadingObserver )){
					    if(( TYPE_InputStatusScanEnd == comPanelJob->getJobStatus()->getInputStatus()) 
							|| (TYPE_InputStatusIdle == comPanelJob->getJobStatus()->getInputStatus())) {	// 読込みが終了している.
						PNLC_ActionDataLib::newInstance()->setInitialDocNameFlag( True );
						nextGamen = &gp_EnlAct_OCRDocNameConfirm;
						}
					} else {
						if( ( NULL != pc_JobSpecScanFaxBackup )
						    && ( False == pc_JobSpecScanFaxBackup->b_OCRReadingObserver ) ) {	// オブザーバーが返ってきていない.
							nextGamen = &gp_EnlAct_OCRReading;
						} else {	// オブザーバーが返ってきている.
							nextGamen = &gp_EnlAct_ActionScanPrint; // 動作中画面(読込中)に遷移.
						}
					}
				}
				bl_result = True;
			}
			break;
		default:		break;	// For Warning!
	}
	if( ( bl_underStopGamen == True || bl_result == False )	// 他の状態がストップよりも優先順位の低い状態の時は停止中へのチェックを行う
	    && !( TYPE_JobTypeTwain == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType()
	          && bl_result == False ) ) {								// PushTwain動作前（パネルロック状態）でない時.
		APIC_Job* pc_SerchJob = NULL;
		register Bool _bl_StopJobFind = False;			// 停止中ジョブを見つけた
		APIC_ActiveJobList* pc_ActiveJobList = APIC_ActiveJobList::newInstance();
		LIBC_List<APIC_Job> c_JobList;
//Comment 2017/11/22【HCLT-Sudheer】【IT5 4.2】【RQ686】 API Correction.
//		short ss_NumberOfJobList = pc_ActiveJobList->getList( TYPE_ActiveJobListPrint, 1, 350, &c_JobList );
		short ss_NumberOfJobList = pc_ActiveJobList->getList( TYPE_ActiveJobListPrint, 1, 350, &c_JobList, PNLC_NetworkLan::getInstance()->getLine() );
		for( register short i = 0; i < ss_NumberOfJobList; i++ ) {	// 最大マルチジョブ数まで検索
			// 検索ジョブをとり込み
			if( i == 0 ) {
				pc_SerchJob = c_JobList.findFirst();
			} else {
				pc_SerchJob	= c_JobList.findNext();
			}
			if( pc_SerchJob != NULL ) {
				if( pc_SerchJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusPrintExec
				    || pc_SerchJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusPrintStop
				    || pc_SerchJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusPrintErrorStop ) {	// プリント停止中または、プリントエラー停止中の場合
					if( pc_SerchJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusPrintExec
					    || ( pc_SerchJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusPrintErrorStop
					         && onlyPanel.isAllGamenWarningEnlarge( pc_SerchJob, True ) != 0 ) ) {
						break;
					} else {
						// 暫定救済後、停止中画面に遷移してしまうのを防ぐ.
						if( pc_SerchJob->getPrintStopCause() != TYPE_PrintStopCause_other ) {
							_bl_StopJobFind = True;		// 停止ジョブあり
						}
						break;
					}
				}
// comment 2022/01/08 FPT-LINHNV46 [MinervaSSBK][OP_BTS-37107]【SYSQA-MSSBK_036】.
// comment 2022/01/15 FPT-LINHNV46 [IT6_2.2MR1][OP_BTS-37156]【SYSQA-IT6_2.2MR1-083】.
//				if( pc_SerchJob->getJobStatus()->getInputStatus() == TYPE_InputStatusScanStop ) {
//					_bl_StopJobFind = True;		// 停止ジョブあり.
//					break;
//				}					
			}
		}
// comment 2022/01/08 FPT-LINHNV46 [MinervaSSBK][OP_BTS-37107]【SYSQA-MSSBK_036】.
// comment 2022/01/15 FPT-LINHNV46 [IT6_2.2MR1][OP_BTS-37156]【SYSQA-IT6_2.2MR1-083】.
//		LIBC_List<APIC_Job> c_JobListAll;
//		short ss_NumberOfJobListAll = pc_ActiveJobList->getList( TYPE_ActiveJobListAll, 1, 350, &c_JobListAll, PNLC_NetworkLan::getInstance()->getLine() );
//		for( register short i_ListAll = 0; i_ListAll < ss_NumberOfJobListAll; i_ListAll++ ) { // 最大マルチジョブ数まで検索
//			// 検索ジョブをとり込み
//			if( i_ListAll == 0 ) {
//				pc_SerchJob = c_JobListAll.findFirst();
//			} else {
//				pc_SerchJob = c_JobListAll.findNext();
//			}
//			if( pc_SerchJob != NULL ) {
//				if( ( pc_SerchJob->getJobStatus()->getInputStatus() == TYPE_InputStatusScanStop )){
//					_bl_StopJobFind = True; // 停止ジョブあり
//					break;
//				}
//			}
//		}
		if( NULL != cp_reserveCopyJob ) {
			// コピー予約時退避ジョブが有効.
			if( APIC_JobInstanceList::isExistJob( cp_reserveCopyJob ) == False ) {
				// ジョブが終了していたら.
				cp_reserveCopyJob = NULL;
			} else if( TYPE_OutputStatusDeleting != cp_reserveCopyJob->getJobStatus()->getOutputStatus()
			           && ( ( TYPE_InputStatusScanStop == cp_reserveCopyJob->getJobStatus()->getInputStatus()
			                  || ( TYPE_InputStatusScanErrorStop == cp_reserveCopyJob->getJobStatus()->getInputStatus()
			                       && onlyPanel.isAllGamenWarningEnlarge( cp_reserveCopyJob, True ) == 0 ) ) ) ) {
				// 読込み停止中であれば、停止中画面を表示する.
				_bl_StopJobFind = True;		// 停止ジョブあり.
				bl_underStopGamen = True;	// プリントジョブが停止遷移中の場合があるためこのフラグを変化させ、もうすぐ停止画面へ遷移させる.
			}
		}
		if( onlyPanel.ec_IntMode != EC_IntMode_On ) {
			if( _bl_StopJobFind == True ) {
				nextGamen = &gp_EnlAct_ActionStop;
				bl_result = True;				// 画面遷移した！
			}
		}
	}

// comment Sky新大阪 [特ROM対応][IT5-6] プリントエラー解除時に他人のジョブの混在を防止する対応.
// comment [特ROM対応][IT5-26] エラー発生時のジョブの削除方法を自動又は手動に設定可能とする対応.
//	if( PnlComIsOthersJobIntermingledPrevention() == True )
// comment  2021/12/21 FPT-LINHNV46 [MinervaSSBK][OP_BTS-36990][SYSQA-MSSBK_034] プリントジョブが停止状態でログアウトとすると、ログインできなくなる.
//	if( ( PnlComIsOthersJobIntermingledPrevention() == True )										// IT5-6の条件.
//	 || (    PnlComGetUserAuthentication()->getUserAuthentication(PNLC_NetworkLan::getInstance()->getLine()) != TYPE_UserAuthenticationOff	// IT5-26の条件：ユーザー認証On.
//	      && PnlComIsJudgeAutoDelete_OnLoginSetting() == True										// IT5-26の条件：ジョブ自動削除設定で「ログイン時削除」設定がある.
//	      && APIC_ActiveJobList::newInstance()->isAutoDeleteJobKind_TopJob() == True ) ){			// IT5-26の条件：ActiveJobListの先頭ジョブ（停止したジョブ）が、ジョブ自動削除種別の対象となるか？.
	if( PnlComGetUserAuthentication()->getUserAuthentication(PNLC_NetworkLan::getInstance()->getLine()) != TYPE_UserAuthenticationOff ){		// IT5-26の条件：ユーザー認証On.
// comment  2022/01/08 FPT-YenNTH8 [IT6_2.2MR1][OP_BTS-37130]【SYSQA-IT6_2.2MR1-082】.
		if( ( onlyPanel.ep_UserAccountEnd == EP_UserAccountEndIDInput ) ||							// IDキーによるマニュアルログアウトのときは停止中画面に遷移しない.
			( NULL == PnlComGetUser()) ){								// ユーザーログアウト中.
			if( nextGamen == &gp_EnlAct_ActionStop ){				// 停止中画面へ遷移しようとした.
				nextGamen = NULL;						// ログアウト中は停止中画面へ遷移しない.
				onlyPanel.bl_StopProcess = False;		// 停止シーケンスを終了させる.
		   		bl_result = False;
			}
		}
	}

	if( ( _bl_StopOnly == False )			// 全動作中状態表示指示時のみチェックする
	    && !( TYPE_JobTypeTwain == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType()
	          && bl_result == False ) ) {								// PushTwain動作前（パネルロック状態）でない時.
		if( ( bl_result == False || bl_underStopGamen == True )			// 停止画面よりも優先順位が低い時にはもうすぐとまりますの条件にも適応する
		    && onlyPanel.ec_IntMode != EC_IntMode_On ) {
			if( comPrintJob != NULL ) {
				if( ( onlyPanel.bl_IntModeStartAnsWait == True || onlyPanel.bl_StopProcess == True )
				    && comPrintJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusPrintExec ) {
					nextGamen = &gp_EnlAct_ActionStopSoon;
					bl_result = True;				// 画面遷移した！
				}
			}
			if( cp_reserveCopyJob != NULL ) {
				if( ( onlyPanel.bl_IntModeStartAnsWait == True || onlyPanel.bl_StopProcess == True )
				    && TYPE_InputStatusScanExec == cp_reserveCopyJob->getJobStatus()->getInputStatus() ) {
					nextGamen = &gp_EnlAct_ActionStopSoon;
					bl_result = True;				// 画面遷移した！
				}
			}
		}
	}
	return bl_result;
}
//================================================================================================
//［機能］ チェック＆画面移動関数（外部参照不可） : 画面移動つきメッセージ警告遷移（拡大表示用）.
//================================================================================================
Bool PBasicMessageProc::changeMessageWarningAndGamenMoveEnlarge()
{
	PEnlBaseGamenCom* p_GamenInstance = dynamic_cast< PEnlBaseGamenCom* >( nowGamen );
	if( NULL == p_GamenInstance ) {
		return False;
	}

	return ( PNLC_EnlWarningManager::newInstance()->changeMessageWarningEnlarge( comPanelJob ) );
}
//================================================================================================
//［機能］ チェック＆画面移動関数（外部参照不可） : モード設定禁止画面表示（拡大表示用）
//================================================================================================
Bool PBasicMessageProc::isGamenDisableModeSetEnlarge()
{
// comment 2020/12/10 Sky延原 ログ強化改善.
	PM_TaskID priTask;
	PMGC_PanelManager::getPriority( &priTask );

	register short ss_TableNo = 0;					// 0 : モード設定可.
	//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/10 Add Start.
	APIC_User* pc_User = PnlComGetUser();
	APIC_Account* pc_Account = APIC_Account::newInstance();
	APIC_ExtSetting* apl = APIC_ExtSetting::newInstance( PnlComGetExtRegAuthAppNo() );
	if( NULL != pc_User && NULL != pc_Account && NULL != apl && NULL != apl->getCurrentInfo()
		&& NULL!= apl->getAplDetail() && NULL != apl->getAplDetail()->getAuthentication()
	  	&&TYPE_UserAuthenticationIntermeddiateServer == PnlComGetLoginUserAuthenticationType()
		&& TYPE_ExtCostCenterUsageSettingEnable == apl->getEnableCostCenter()
		&& PNLD_CostCenterInfo_Enable == PnlComGetCostCenterInfo()											// 2022/04/28 Sky東島 [MinervaSSBK][OP_BTS-37985]Add.
		&& pc_User->getUserType() == TYPE_UserTypeUser
		&& False == gp_CostCenterResetDummy.bl_Serverdown   //server down
		&& False == gp_CostCenterResetDummy.bl_ICcard 		//ic card case//jegan
		&& NULL != PNLC_ActionDataLib::newInstance()
		&& (TYPE_CostCenterReconfigureSettingEnable == PNLC_ActionDataLib::newInstance()->getReconfigureSetting())	
		&& (True == pc_Account->isEnableEntryJob( pc_User->getUserNo()))
		&& (False == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_ExtCounterLimitError ))
		&& (getNowGamen() != &gp_CostCenterResetDummy)
		&& (onlyPanel.ep_UserAccountEnd == EP_UserAccountEndIDInput)
		&& ( TYPE_LoginAuthorityMode_User == PNLC_ActionDataLib::newInstance()->getLoginAuthorityMode())
		&& NULL != PNLC_PopupListControl::newInstance()
		&&(PNLC_PopupListControl::newInstance()->getNowPopup() != &gp_CostCenterResetFailure_warn)
		&& (gp_CostCenterResetDummy.bl_CostCenterWarning == False )
		&& False != PNLC_MidAuthServerStatus::getServerStatus()//【CQ515136】【RQ-218CostCenterの再設定】 HCL-Siva 2015/06/30 Add.
		&& NULL == onlyPanel.getAppReqAplInstance(TYPE_PNL_AppReqObserver_UserLogout)//【CQ518455& CQ518469】【J0R_22_Ext Screen Transistion issue from HometopMenu Screen】【FVT_2】 HCL-Jegan 2015/09/09 Add.
		&&  ( True != PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_UserAccountUserEndExpand1_act ) )){
			
		onlyPanel.ep_UserAccountEnd = EP_UserAccountEndOff;
// comment 2020/12/10 Sky延原 ログ強化改善.
		ExpPanelLog( PNLD_ExpLogKind_IsGamenDisableModeSet, "[LocalUI][isGamenDisableModeSetEnlarge][PriTask:%d] onlyPanel.setPanelWarning( PNL_ERROR_CostCenterReset )", priTask, 0, 0, 0, 0, 0 );
		onlyPanel.setPanelWarning( PNL_ERROR_CostCenterReset );//Stub
					//return True;
	}
	//【CQ512401】【RQ-218CostCenterの再設定】【IT-Bug】 HCL-Jegan 2015/04/25 Add Start.
		gp_CostCenterResetDummy.bl_ICcard = False;
	//【CQ512401】【RQ-218CostCenterの再設定】【IT-Bug】 HCL-Jegan 2015/04/25 Add End.
	//【CQ510665】【RQ-218CostCenterの再設定】 HCL-Surya 2014/11/10 Add End.
	ss_TableNo = onlyPanel.isDisableModeSet();		//モード設定禁止画面チェック.
// comment 2020/12/10 Sky延原 ログ強化改善.
	ExpPanelLog( PNLD_ExpLogKind_IsGamenDisableModeSet, "[LocalUI][isGamenDisableModeSetEnlarge][PriTask:%d] ss_TableNo[%d]", priTask, ss_TableNo, 0, 0, 0, 0 );
	if( ss_TableNo != 0 ) {		// モード設定禁止のとき.
// comment 2020/12/10 Sky延原 ログ強化改善.
		ExpPanelLog( PNLD_ExpLogKind_IsGamenDisableModeSet, "[LocalUI][isGamenDisableModeSetEnlarge][PriTask:%d] ec_WarningName[%d]: getKeyMode()[%d]", priTask, sp_GamenEnlDisableModeSetTable[ ss_TableNo ].ec_WarningName, APIC_KeyDevice::getKeyMode(), 0, 0, 0 );
		// 管理装置装着時、部門認証ログイン画面は表示しない.
		if( PNL_OTHER_TrackIDNumberInput == sp_GamenEnlDisableModeSetTable[ ss_TableNo ].ec_WarningName
		    && TYPE_KeyModeAdminMachine == APIC_KeyDevice::getKeyMode() ) {
// comment 2020/12/10 Sky延原 ログ強化改善.
			ExpPanelLog( PNLD_ExpLogKind_IsGamenDisableModeSet, "[LocalUI][isGamenDisableModeSetEnlarge][PriTask:%d] return False", priTask, 0, 0, 0, 0, 0 );
			return False;										// 画面移動しなかった
		}
		// 統合管理装置装着時ユーザ認証画面は表示しない 管理しないアプリならばPublicで認証/管理するアプリならば｢カードを入れてください｣になる.
		if( PNL_OTHER_UserIDNumberInput == sp_GamenEnlDisableModeSetTable[ ss_TableNo ].ec_WarningName
		    && TYPE_KeyModeAdminMachineKM == APIC_KeyDevice::getKeyMode() ) {
// comment 2020/12/10 Sky延原 ログ強化改善.
			ExpPanelLog( PNLD_ExpLogKind_IsGamenDisableModeSet, "[LocalUI][isGamenDisableModeSetEnlarge][PriTask:%d] return False", priTask, 0, 0, 0, 0, 0 );
			return False;										// 画面移動しなかった
		}
		// 中間認証サーバー用に追加 ↓start↓
		if( PNL_OTHER_UserIDNumberInput2 == sp_GamenEnlDisableModeSetTable[ ss_TableNo ].ec_WarningName
		    && TYPE_KeyModeAdminMachineKM == APIC_KeyDevice::getKeyMode() ) {
// comment 2020/12/10 Sky延原 ログ強化改善.
			ExpPanelLog( PNLD_ExpLogKind_IsGamenDisableModeSet, "[LocalUI][isGamenDisableModeSetEnlarge][PriTask:%d] return False", priTask, 0, 0, 0, 0, 0 );
			return False;										// 画面移動しなかった
		}
		// 中間認証サーバー用に追加 ↑end↑
		if( PNL_OTHER_UserIDNumberInput3 == sp_GamenEnlDisableModeSetTable[ ss_TableNo ].ec_WarningName
		    && TYPE_KeyModeAdminMachineKM == APIC_KeyDevice::getKeyMode() ) {
// comment 2020/12/10 Sky延原 ログ強化改善.
			ExpPanelLog( PNLD_ExpLogKind_IsGamenDisableModeSet, "[LocalUI][isGamenDisableModeSetEnlarge][PriTask:%d] return False", priTask, 0, 0, 0, 0, 0 );
			return False;										// 画面移動しなかった
		}
// comment 2020/12/10 Sky延原 ログ強化改善.
		ExpPanelLog( PNLD_ExpLogKind_IsGamenDisableModeSet, "[LocalUI][isGamenDisableModeSetEnlarge][PriTask:%d] PnlComIsCardSelfRegistrationOn()[%d]: c_CardSelfRegistUserInfo.getOn()[%d]", priTask, PnlComIsCardSelfRegistrationOn(), c_CardSelfRegistUserInfo.getOn(), 0, 0, 0 );
		// 遷移先がユーザー認証画面で、セルフレジストレーション機能がOnの場合で、セルフレジストレーション用ユーザー情報が有効な場合.
		if( ( PNL_OTHER_UserIDNumberInput == sp_GamenEnlDisableModeSetTable[ ss_TableNo ].ec_WarningName )
		    && ( True == PnlComIsCardSelfRegistrationOn() )
		    && ( True == c_CardSelfRegistUserInfo.getOn() ) ) {
			Bool b_ret = False;
			b_ret = isSeqCustomNextGamen();
			if( False == b_ret ){
// comment 2020/12/10 Sky延原 ログ強化改善.
				ExpPanelLog( PNLD_ExpLogKind_IsGamenDisableModeSet, "[LocalUI][isGamenDisableModeSetEnlarge][PriTask:%d] nextGamen[%p] => &gp_UserAccountEntryAndLoginEnl[%p]", priTask, nextGamen, &gp_UserAccountEntryAndLoginEnl, 0, 0, 0 );
				nextGamen = &gp_UserAccountEntryAndLoginEnl;		// MZ-77_拡大動作中新規画面 ユーザー情報の登録&ログイン画面.
			}
			onlyPanel.bl_UserAccountInit = True;
// comment 2020/12/10 Sky延原 ログ強化改善.
			ExpPanelLog( PNLD_ExpLogKind_IsGamenDisableModeSet, "[LocalUI][isGamenDisableModeSetEnlarge][PriTask:%d] return True", priTask, 0, 0, 0, 0, 0 );
			return True;
		}
		if( PNL_OTHER_UserIDNumberInput4 == sp_GamenEnlDisableModeSetTable[ ss_TableNo ].ec_WarningName
		    && TYPE_KeyModeAdminMachineKM == APIC_KeyDevice::getKeyMode() ) {
// comment 2020/12/10 Sky延原 ログ強化改善.
			ExpPanelLog( PNLD_ExpLogKind_IsGamenDisableModeSet, "[LocalUI][isGamenDisableModeSetEnlarge][PriTask:%d] return False", priTask, 0, 0, 0, 0, 0 );
			return False;
		}
// comment 2020/12/10 Sky延原 ログ強化改善.
		ExpPanelLog( PNLD_ExpLogKind_IsGamenDisableModeSet, "[LocalUI][isGamenDisableModeSetEnlarge][PriTask:%d] PnlComisEnableUserAccountFull()[%d]", priTask, PnlComisEnableUserAccountFull(), 0, 0, 0, 0 );
		if( ( PNL_OTHER_UserIDAndPassWordInput == sp_GamenEnlDisableModeSetTable[ ss_TableNo ].ec_WarningName ) &&
			 ( True == PnlComisEnableUserAccountFull() )
		) {
			Bool b_ret = False;
			b_ret = isSeqCustomNextGamen();
			if( True == b_ret ){
				// 認証カスタマイズ機能有効で認証シーケンス画面設定した.
// comment 2020/12/10 Sky延原 ログ強化改善.
				ExpPanelLog( PNLD_ExpLogKind_IsGamenDisableModeSet, "[LocalUI][isGamenDisableModeSetEnlarge][PriTask:%d] return True", priTask, 0, 0, 0, 0, 0 );
				return True;
			}
		}
		if( PNL_OTHER_ScanFaxSendSettingContinueCheck != sp_GamenEnlDisableModeSetTable[ss_TableNo].ec_WarningName ) {
// comment 2020/12/10 Sky延原 ログ強化改善.
			ExpPanelLog( PNLD_ExpLogKind_IsGamenDisableModeSet, "[LocalUI][isGamenDisableModeSetEnlarge][PriTask:%d] changeDisableModeWarning( ss_TableNo )", priTask, 0, 0, 0, 0, 0 );
			PNLC_EnlWarningManager::newInstance()->changeDisableModeWarning( ss_TableNo );
		}
// comment 2020/12/10 Sky延原 ログ強化改善.
		ExpPanelLog( PNLD_ExpLogKind_IsGamenDisableModeSet, "[LocalUI][isGamenDisableModeSetEnlarge][PriTask:%d] return True", priTask, 0, 0, 0, 0, 0 );
		return True;			// 画面移動した.
	}
// comment 2020/12/10 Sky延原 ログ強化改善.
	ExpPanelLog( PNLD_ExpLogKind_IsGamenDisableModeSet, "[LocalUI][isGamenDisableModeSetEnlarge][PriTask:%d] return False", priTask, 0, 0, 0, 0, 0 );
	return False;				// 画面移動しなかった.
}
//================================================================================================
//［機能］　その他関数集（外部参照不可） :  警告表示画面状態が変わったかどうかのチェック＆処理（拡大表示用）
//================================================================================================
void PBasicMessageProc::checkAndMoveWarningGamenEnlarge()
{
	if( onlyPanel.bl_PreNoWarningGamen != isNoWarningGamen ) {		// 前の画面の警告表示禁止状態が変わっている時.
		onlyPanel.bl_PreNoWarningGamen = isNoWarningGamen;
		if( isNoWarningGamen == False ) {								// 警告禁止画面でない時.
			Bool bl_isNewScreenLaunching = False;
			// 全画面警告発生チェックと画面移動（呼んだ関数内で行なう）.
			if( onlyPanel.cc_DisplayJob != comPanelJob &&	// cc_DisplayJobとcomPanelJobが同じでない時.
			    comPanelJob != PnlComGetFaxManualRcvJob() ) {		// 2008/07/22 12大庭 CQ306383対応.
				if( True == PNLC_EnlWarningManager::newInstance()->changeWarningEnlarge( PNLE_CheckENLWarningType_JobList,
				        onlyPanel.cc_DisplayJob, bl_isNewScreenLaunching ) ) {	// ジョブリスト警告チェック内で画面移動がされた.

					if( False == bl_isNewScreenLaunching ) { return; }

					onlyPanel.ep_ForceChangeGamen = EP_ForceChangeGamen_Warning;	// 警告強制画面遷移.
					return;
				}
			} else {														// 上記のどちらでもない時は通常警告をチェック.
				if( True == PNLC_EnlWarningManager::newInstance()->changeWarningEnlarge( PNLE_CheckENLWarningType_Normal,
				        comPanelJob, bl_isNewScreenLaunching ) ) {			// 全画面警告チェック内で画面移動がされた.
					// ジョブ詳細画面遷移後に前の画面に戻ってしまう問題対策.
					if( False == bl_isNewScreenLaunching ) { return; }
					onlyPanel.ep_ForceChangeGamen = EP_ForceChangeGamen_Warning;	// 警告強制画面遷移.
					return;
				}
			}
			// ジョブリスト警告表示中に全画面警告がなくなった場合は、解除されたので表示をcomPanelJobに戻す.
			onlyPanel.cc_DisplayJob = comPanelJob;							// 全警告表示対象のジョブをcomPanelJobにする.
		}
	}
}
/*================================================================================================
［機能］　その他関数集（外部参照不可） : インチモードフラグ（パーツクラスのクラス内static変数）のセット
================================================================================================*/
void PBasicMessageProc::setInchModeToPartsClass()
{
	Bak_Background.setInchMode( pc_MachineSetting->getSystemType() == TYPE_SystemType_US ? True : False );	// インチモード（ＭＣ仕向）の設定
	// 一つの部品に対してインチモード（ＭＣ仕向）を設定することにより、全部品がこのモードを参照可能となる。
	TYPE_UnitSystem e_UnitSystem = PnlComGetUnitSystem();
	Bak_Background.setUnitSystem( e_UnitSystem );	// 単位系の設定
}
/*-----------------------------------------------------------------------------------------------------------
	 データの変化エッジチェック関数集（全て定期起床タイマー処理(procTimerWakeUp)から呼ばれる）
-----------------------------------------------------------------------------------------------------------*/
/*================================================================================================
［機能］ データの変化エッジチェック : スキャン／プリント状態の変化
================================================================================================*/
Bool PBasicMessageProc::isStatusChange()
{
	PNLC_NotifyEventInfo* pc_NtfEventInfo = PNLC_NotifyEventInfo::newInstance();		// データの管理クラス.
	if( NULL == pc_NtfEventInfo ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}

	PCGC_VerifyThumbnailPreviewControl* pc_VerifyControl = PnlComGetVerifyPreviewControl();
	static TYPE_InputStatus PreDataCheckInput   = TYPE_InputStatusIdle;					// 前回の入力状態
	static TYPE_OutputStatus PreDataCheckOutput = TYPE_OutputStatusIdle;				// 前回の出力状態
	static TYPE_FaxJobStatus PreDataCheckFaxJob1 = TYPE_FaxJobStatusIdle;				// 前回のFAXステータス(回線1用).
	static TYPE_FaxJobStatus PreDataCheckFaxJob2 = TYPE_FaxJobStatusIdle;				// 前回のFAXステータス(回線2用).
	// 回線3,4の処理追加.
	static TYPE_FaxJobStatus PreDataCheckFaxJob3 = TYPE_FaxJobStatusIdle;				// 前回のFAXステータス(回線3用).
	static TYPE_FaxJobStatus PreDataCheckFaxJob4 = TYPE_FaxJobStatusIdle;				// 前回のFAXステータス(回線4用).
	static TYPE_ProgJobInStatus PreDataCheckProgInput = TYPE_ProgJobInStatusIdle;		// 前回のプログラムジョブ入力状態.
	static TYPE_ProgJobOutStatus PreDataCheckProgOutput = TYPE_ProgJobOutStatusIdle;	// 前回のプログラムジョブ出力状態.
	register Bool bl_result = False;						// チェック結果
	// 回線3,4の処理追加.
	bl_result = ( PreDataCheckInput != comPanelJob->getJobStatus()->getInputStatus() 						// comPanelJobのジョブステータスの画像入力状態.
	              || PreDataCheckOutput != comPanelJob->getJobStatus()->getOutputStatus()						// 　及び　画像出力状態の変化をチェック.
	              || PreDataCheckFaxJob1 != comPanelJob->getJobStatus()->getFaxJobStatus( TYPE_CommLine1 )		// 　及び　FAXステータス状態の変化をチェック(回線1用).
	              || PreDataCheckFaxJob2 != comPanelJob->getJobStatus()->getFaxJobStatus( TYPE_CommLine2 )		// 　及び　FAXステータス状態の変化をチェック(回線2用).
	              || PreDataCheckFaxJob3 != comPanelJob->getJobStatus()->getFaxJobStatus( TYPE_CommLine3 )		// 　及び　FAXステータス状態の変化をチェック(回線3用).
	              || PreDataCheckFaxJob4 != comPanelJob->getJobStatus()->getFaxJobStatus( TYPE_CommLine4 )		// 　及び　FAXステータス状態の変化をチェック(回線4用).
	              || PreDataCheckProgInput != pc_NtfEventInfo->getProgJobInStatus()					//   及び　プログラムジョブ入力状態の変化をチェック.
	              || PreDataCheckProgOutput != pc_NtfEventInfo->getProgJobOutStatus() ) ? True : False; //	 及び　プログラムジョブ出力状態の変化をチェック.
	PreDataCheckInput  = comPanelJob->getJobStatus()->getInputStatus();		// 前回の画像入力状態に格納
	PreDataCheckOutput = comPanelJob->getJobStatus()->getOutputStatus();	// 前回の画像出力状態に格納
	PreDataCheckFaxJob1 = comPanelJob->getJobStatus()->getFaxJobStatus( TYPE_CommLine1 );	// 前回のFAXステータス状態に格納(回線1用).
	PreDataCheckFaxJob2 = comPanelJob->getJobStatus()->getFaxJobStatus( TYPE_CommLine2 );	// 前回のFAXステータス状態に格納(回線2用).
	// 回線3,4の処理追加.
	PreDataCheckFaxJob3 = comPanelJob->getJobStatus()->getFaxJobStatus( TYPE_CommLine3 );	// 前回のFAXステータス状態に格納(回線3用).
	PreDataCheckFaxJob4 = comPanelJob->getJobStatus()->getFaxJobStatus( TYPE_CommLine4 );	// 前回のFAXステータス状態に格納(回線4用).
	PreDataCheckProgInput = pc_NtfEventInfo->getProgJobInStatus();		// 前回のプログラムジョブ入力状態に格納.
	PreDataCheckProgOutput = pc_NtfEventInfo->getProgJobOutStatus();	// 前回のプログラムジョブ出力状態に格納.
	static Bool pre_Copying1Sec = False;
	if( pre_Copying1Sec != ( Bool )onlyPanel.ct_Copying1Sec.isBusy() ) {
		pre_Copying1Sec = ( Bool )onlyPanel.ct_Copying1Sec.isBusy();
		if( pre_Copying1Sec == False ) {
			// プリントキーが押された後、１秒間コピー中にするためのタイマが終了した
			bl_result = True;	// チェック結果
		}
	}
	static Bool pre_FTPConnectionConfirm = False;
	if( pre_FTPConnectionConfirm != c_SystemControl.nowFTPConnection() ) {
		pre_FTPConnectionConfirm = c_SystemControl.nowFTPConnection();
		if( pre_FTPConnectionConfirm == False ) {
			// ＦＴＰ接続確認中が終了した
			bl_result = True;	// チェック結果
		}
	}
	static Bool pre_WebDAVConnectionConfirm = False;
	if( pre_WebDAVConnectionConfirm != c_SystemControl.nowWebDAVConnection() ) {
		pre_WebDAVConnectionConfirm = c_SystemControl.nowWebDAVConnection();
		if( pre_WebDAVConnectionConfirm == False ) {
			// ＷｅｂＤＡＶ接続確認中が終了した.
			bl_result = True;	// チェック結果.
		}
	}
	static Bool pre_StopProcess = False;
	if( pre_StopProcess != onlyPanel.bl_StopProcess
	    && False == onlyPanel.bl_StopProcess ) {
		bl_result = True;	// チェック結果
	}
	pre_StopProcess = onlyPanel.bl_StopProcess;
	if( ( EP_PNS_ReadExec == onlyPanel.ep_PanelStatus || EP_PNS_RealTimePrvExec == onlyPanel.ep_PanelStatus )
	    && ( comPanelJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusIdle
	         || comPanelJob->getJobStatus()->getInputStatus() == TYPE_InputStatusScanEnd )
	    &&  c_SystemControl.nowFTPConnection() == False								// FTP接続確認中は除外する
	    &&  c_SystemControl.nowWebDAVConnection() == False								// WebDAV接続確認中は除外する.
	    && TYPE_JobTypeFaxSendQuick != comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType()
	    && TYPE_JobTypeFaxSendMemory != comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType()
	    && TYPE_JobTypeFaxSendTimer != comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() ) {	// FAXジョブでない場合.
		// パネルステータスが読込中であるにも関わらず、アイドル中の場合（ステータス変化エッジを取りこぼした時のプロテクト）
		// FTP接続中の時は除外する（接続完了するまでは、ジョブのステータスはアイドル中のため)
		bl_result = True;	// チェック結果
	}
	static PDPE_FaxSendQuickStatus pre_FaxSendQuickStatus = PDPE_FaxSendQuickStopOther;
	// FAXクイック送信停止状態に変化.
	if( pre_FaxSendQuickStatus != c_JobStatusControl.checkFaxSendQuickStop( comPanelJob ) ) {
		pre_FaxSendQuickStatus = c_JobStatusControl.checkFaxSendQuickStop( comPanelJob );
		bl_result = True;	// チェック結果
	}
// 電源I/F統合（電源ｽﾃｰﾀｽ取得関数変更）.
	_bl_comPanelJobChange = False;
	if( APIC_JobInstanceList::isExistJob( comPanelJob ) == False ) {
		_bl_comPanelJobChange = True;
		bl_result = True;	// チェック結果
	} else if( TYPE_JobTypeIntCopy != comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType()
	           && TYPE_OutputStatusDeleting == comPanelJob->getJobStatus()->getOutputStatus() ) {
		cp_reserveCopyJob = comPanelJob;
		_bl_comPanelJobChange = True;
		bl_result = True;	// チェック結果
		if( True == comPanelJob->getJobSpec()->pc_Common->getVerifyPreviewOn() ) {
			if( True == pc_VerifyControl->isStartingVerifyPreview() ) {
				pc_VerifyControl->endVerify( False );
			} else {
				// 仕上りサムネイル画像の削除要求を行う.
				APIC_ThumbnailConverter* pc_ThumbnailConverter = APIC_ThumbnailConverter::newInstance();
				PanelLog( PNLD_LogKind_Active, "req_delVerify: ", 0, 0, 0, 0, 0, 0 );
				pc_ThumbnailConverter->req_delVerify();
			}
		}
	} else if( TYPE_JobTypeIntCopy == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType()
	           && TYPE_OutputStatusDeleting == comPanelJob->getJobStatus()->getOutputStatus() ) {
		// 割り込みコピーは、reserveセットせずに切り代え.
		// 読み込み中にジョブ削除指示した場合　ADFからの排紙までスタートLEDを赤にする.
		onlyPanel.cp_IntModeDeleteCopyJob = comPanelJob;
		_bl_comPanelJobChange = True;
		bl_result = True;	// チェック結果
		if( True == comPanelJob->getJobSpec()->pc_Common->getVerifyPreviewOn() ) {
			if( True == pc_VerifyControl->isStartingVerifyPreview() ) {
				pc_VerifyControl->endVerify( False );
			} else {
				// 仕上りサムネイル画像の削除要求を行う.
				APIC_ThumbnailConverter* pc_ThumbnailConverter = APIC_ThumbnailConverter::newInstance();
				PanelLog( PNLD_LogKind_Active, "req_delVerify: ", 0, 0, 0, 0, 0, 0 );
				pc_ThumbnailConverter->req_delVerify();
			}
		}
	} else if( TYPE_FaxJobStatusEnd == comPanelJob->getJobStatus()->getFaxJobStatus( c_JobControl.getFaxActJobLineStatus( comPanelJob ) )
	           && True != comPanelJob->getJobStatus()->getFaxSnedContinuation()					// Jobからの通信状態継続フラグ追加 True：継続中 False：終了
	           && TYPE_InputStatusReceiveExec != comPanelJob->getJobStatus()->getInputStatus()
	           && TYPE_InputStatusReceiveEnd != comPanelJob->getJobStatus()->getInputStatus() ) {
		// ダイヤル中のジョブ削除の場合、通信終了にて判断する＆データ受信の通信終了時プリントに移行する.
		// FAX通信が完了していて、その通信が受信では無い場合ジョブの終了とみなす.
		_bl_comPanelJobChange = True;
		bl_result = True;	// チェック結果
	} else if( True == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_CopyJobEntryOver )
	           || True == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_ScanJobEntryOver )
	           || True == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_FaxJobEntryOver )
	           || True == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_TouchAndPrintJobEntryOver ) ) {	// 伊丹 (Sky) 加藤 2010/11/13 CQ334403 対応 認証&プリントのジョブオーバーチェック.
		// 旧UIではジョブ登録オーバーは全画面警告にて表示され画面遷移が抑制されていたが、
		// 新UIで警告ポップアップ表示中に再検出されポップアップが強制的に閉じられてしまう.
		if( False == PNLC_PopupListControl::newInstance()->isPopupLayer( &gp_JobListJobOver_warn ) ) {
			// ジョブ登録オーバー時は常にジョブを取得出来るようエッジを検出する.
			_bl_comPanelJobChange = True;
			bl_result = True;	// チェック結果
		}
	} else if( onlyPanel.b_nowBoxMode == True
	           && TYPE_InputStatusScanEnd == comPanelJob->getJobStatus()->getInputStatus()
	           && ( 0 < comPanelJob->getJobSpec()->pc_Common->getJimonPattern2()->getJimonRead() ) ) {	// Boxモード且つ読込み完了且つパスワードコピー原稿検出時. // No312673対応.
		_bl_comPanelJobChange = True;
		bl_result = True;	// チェック結果.
	} else if( True == onlyPanel.b_nowBoxMode
	           && True == onlyPanel.b_DocumentSaveMode
	           && TYPE_InputStatusScanEnd == comPanelJob->getJobStatus()->getInputStatus()
	           // comment 2019/03/18 Sky東島 [EagleH][OP_BTS-2377].
	           && TYPE_OutputStatusOCRReadExec != comPanelJob->getJobStatus()->getOutputStatus()
	           && False == comPanelJob->getJobSpec()->pc_Send->c_ComSend.c_DirectAddressBook.pageUsbMemAddress( 1 )->getOn() ) {
		// BOX 文書保存の場合に読取り完了でエッジ検出する(外部メモリー保存時は除く).
		_bl_comPanelJobChange = True;
		bl_result = True;	// チェック結果.
	}
	// 設定中にカードチェンジ発生.
	if( APIC_KeyDevice::getKeyMode() == TYPE_KeyModePageAcses	// 認証装置装着時.
	    && onlyPanel.b_PageAcsessModeReset == True ) {			// カード未挿入通知を受けた.
		bl_result = True;	// チェック結果.
	}
	static TYPE_ImageForwardStatus PreDataCheckImageFrowardStatus = ImageForwardStatusIdle; // 前回の画像ログ転送状態状態.
	if( PreDataCheckImageFrowardStatus != comPanelJob->getJobStatus()->getImageForwardStatus() ) {
		if( ImageForwardStatusExec == PreDataCheckImageFrowardStatus ) {
			bl_result = True;
		}
		PreDataCheckImageFrowardStatus = comPanelJob->getJobStatus()->getImageForwardStatus();
	}
	static TYPE_RealTimePreviewStatus e_RealTimePreviewStatus = TYPE_RealTimePreviewStatusIdle;
	if( e_RealTimePreviewStatus != comPanelJob->getJobStatus()->getRealTimePreviewStatus() )	// 前回のプレビューステータスから変化あり.
	{
		bl_result = True;	// チェック結果.
		e_RealTimePreviewStatus = comPanelJob->getJobStatus()->getRealTimePreviewStatus();	// 前回値を保存.
	}
	// リアルタイムプレビュー表示中にストップキーが押された際、.
	// 一旦、もうすぐ止まります画面に遷移した後、InputStatusが分割読込み停止中に変化した場合はエッジを検出する.
	static Bool bl_PreStopProcess = onlyPanel.bl_StopProcess;
	if( True == bl_PreStopProcess && False == onlyPanel.bl_StopProcess	// ストップキーフラグのエッジ検出.
	    && TYPE_InputStatusMemoryInScanStop == comPanelJob->getJobStatus()->getInputStatus()	// 分割読込み停止中.
	    && EP_PNS_RealTimePrvExec == onlyPanel.ep_PanelStatus )	// リアルタイムプレビュー表示中.
	{
		bl_result = True;	// チェック結果.
	}
	bl_PreStopProcess = onlyPanel.bl_StopProcess;	// 前回値を保存.
	static Bool bl_ReceiveJobExistBefore = False;
	static PNLE_TouchAndGoReceiveStatus e_TouchAndGoReceiveControlBefore = onlyPanel.getTouchAndGoReceiveControl();		// 前回のステータス.
	if( NULL != PnlComGetUser() ) {	// ログインユーザーあり.
		Bool bl_ReceiveJobExistNow = APIC_ActiveJobList::newInstance()->isExistRcvUsrJob( PnlComGetUser()->getUserNo() );	// 受信中ジョブ有無の確認.
		if( ( bl_ReceiveJobExistNow != bl_ReceiveJobExistBefore )
		    || ( ( PNLE_TouchAndGoReceiveWaitingForPrint == e_TouchAndGoReceiveControlBefore ) && ( PNLE_TouchAndGoReceiveWaitingForReceiveEnd == onlyPanel.getTouchAndGoReceiveControl() ) ) ) {
			bl_result = True;		// 変化あり.
		}
		bl_ReceiveJobExistBefore = bl_ReceiveJobExistNow;
		e_TouchAndGoReceiveControlBefore = onlyPanel.getTouchAndGoReceiveControl();
	} else {
		bl_ReceiveJobExistBefore = False;	// ログインユーザーがいない場合は、前回値をなしに倒す.
	}
	if( ( onlyPanel.isMultiCopyJobManageOption() == True )
	    && ( APIC_KeyDevice::getKeyMode() == TYPE_KeyModeVendorKM ) ) {
		APIC_ExtSetting* pc_ExtSetting = ScanOapExtSet.getCurrentAplInfo();
		if( ( NULL != pc_ExtSetting )
		    && ( True == ScanOapExtSet.isActiveGeneralApplication() )
		    &&( ( TYPE_ExtTypePullPrint == pc_ExtSetting->getApplication().s_Version.e_Type )
		        || ( TYPE_ExtTypeBrowserPull == pc_ExtSetting->getApplication().s_Version.e_Type )
		        || ( TYPE_ExtTypeBrowserGeneral == pc_ExtSetting->getApplication().s_Version.e_Type ) )
		    &&	comPanelJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusPrintExec ) {
			bl_result = True;
		}
	}
	return bl_result;
}
/*================================================================================================
［機能］ データの変化エッジチェック : プリントジョブ状態の変化(プリント停止中の変化)
================================================================================================*/
Bool PBasicMessageProc::isPrintJobStopChange()
{
	static APIC_Job* PrePrintJob 	= NULL;									// 前回のプリントジョブ
	static APIC_Job* PreReserveJob = NULL;									// 前回のリザーブジョブ.
	static TYPE_OutputStatus PreOutputStatus 	= TYPE_OutputStatusIdle;	// 前回の出力状態
	static TYPE_InputStatus PreInputStatus = TYPE_InputStatusIdle;			// 前回の入力状態.
	register Bool bl_result = False;										// チェック結果
	if( comPrintJob != NULL ) {
		if( ( comPrintJob != PrePrintJob || comPrintJob->getJobStatus()->getOutputStatus() != PreOutputStatus )
		    && ( ( comPrintJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusPrintStop
		           || PreOutputStatus == TYPE_OutputStatusPrintStop
		           || comPrintJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusPrintErrorStop
		           || PreOutputStatus == TYPE_OutputStatusPrintErrorStop )
		         // プリント停止中へ変化した、またはプリント停止中から別のステータスへ変化した
		         || onlyPanel.bl_StopProcess == True ) ) {
			// ストップキーが押された後にプリントジョブの画像出力状態が変化した
			bl_result = True;
		}
	} else if( comPrintJob != PrePrintJob && onlyPanel.bl_StopProcess == True ) {
		// プリントジョブが削除or終了した場合でパネル停止待ち時にエッジを検出する.
		bl_result = True;
	}
	if( onlyPanel.bl_StopProcess == True && onlyPanel.bl_StopProcessOff == True ) {
		onlyPanel.bl_StopProcess = False;
	}
	onlyPanel.bl_StopProcessOff = False; // procStatusChange～isPrintJobStopChangeの間のみ有効なフラグとする.
	if( cp_reserveCopyJob != NULL ) {
		// ジョブが終了していたら.
		if( APIC_JobInstanceList::isExistJob( cp_reserveCopyJob ) == False ) {
			cp_reserveCopyJob = NULL;
		} else if( ( cp_reserveCopyJob != PreReserveJob || cp_reserveCopyJob->getJobStatus()->getInputStatus() != PreInputStatus )
		           && ( ( cp_reserveCopyJob->getJobStatus()->getInputStatus() == TYPE_InputStatusScanStop
		                  || PreInputStatus == TYPE_InputStatusScanStop
		                  || cp_reserveCopyJob->getJobStatus()->getInputStatus() == TYPE_InputStatusScanErrorStop
		                  || PreInputStatus == TYPE_InputStatusScanErrorStop )
		                || onlyPanel.bl_StopProcess == True ) ) {
			// ストップキーが押された後にプリントジョブの画像出力状態が変化した
			bl_result = True;
		}
	} else if( cp_reserveCopyJob != PreReserveJob && onlyPanel.bl_StopProcess == True ) {
		// プリントジョブが削除or終了した場合でパネル停止待ち時にエッジを検出する.
		bl_result = True;
	}
	PrePrintJob = comPrintJob;						// 前回のプリントジョブに格納
	if( comPrintJob != NULL ) {
		PreOutputStatus = comPrintJob->getJobStatus()->getOutputStatus();	// 前回の画像出力状態に格納
	} else {
		PreOutputStatus = TYPE_OutputStatusIdle;							// 前回の画像出力状態に格納
	}
	PreReserveJob = cp_reserveCopyJob;
	if( NULL != cp_reserveCopyJob ) {
		PreInputStatus = cp_reserveCopyJob->getJobStatus()->getInputStatus();
	} else {
		PreInputStatus = TYPE_InputStatusIdle;
	}
	// 割り込み中はcomPrintJobの警告チェックが必要ないため、ここでリターン
	if( onlyPanel.ec_IntMode == EC_IntMode_On ) {
		return bl_result;
	}
	//	プリントジョブ・次ジョブ予約キー押下後の読み込みジョブが有効かつ、警告表示用ジョブとは異なる場合
	if( ( APIC_JobInstanceList::isExistJob( comPrintJob ) == True
	      && ( onlyPanel.cc_DisplayJob != comPrintJob ) )
	    || ( APIC_JobInstanceList::isExistJob( cp_reserveCopyJob ) == True
	         && ( onlyPanel.cc_DisplayJob != cp_reserveCopyJob ) ) ) {
		Bool b_EmergencySend = False;
		//	全画面警告が発生していない時のみ処理対象（以下条件を考慮）
		//	・プリントジョブ以外で全画面警告が既に表示されている場合
		//	・プリントジョブで追い越しが発生した場合、追い越されたジョブで既に
		//	　全画面警告が表示されている場合
// ｺﾋﾟｰｷｯﾄｶｳﾝﾀ・ｶﾗｰﾓﾉｸﾛ上限設定値ｵｰﾊﾞｰ（設定時）警告はジョブリスト警告をチェックする.
// パスワードコピー原稿検出時画面は警告では無く、動作中なのでジョブリスト警告をチェックし、「警告表示」キーを表示させる.// No320945対応.
// 動作中画面であるが警告画面テーブルを使い警告として画面遷移しているため追加。本来は読込みステータスの変化により他動作中画面と同様の画面遷移をさせる必要がある.// No320945対応.
		if( ( onlyPanel.isAllGamenWarning( onlyPanel.cc_DisplayJob ) == 0
		      || True == onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, PERROR_JimonPasswordReadError )	// パスワードコピー原稿検出時.// No320945対応.
		      || onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, PERROR_CopyKitTotalLimit_S ) == True
		      || True == onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, PNL_ERROR_OAPMessage ) 	// APPメッセージ表示中.
		      || True == onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, PERROR_KeyCounterUnset_S )	// キーカウンター未挿入(設定時).
		      || True == onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, PNL_ERROR_SimpleSecurityWarning ) )	// 簡易セキュリティー警告.
		    && comPrintJob != NULL ) {
			// ジョブリストのチェックを拡大と通常で分ける.
			if( onlyPanel.bl_isEnlargeDisplayMode == True ) {	// 拡大表示モード時.
				//	対象の警告はジョブリストと同じ
				register ushort DataCheckPrintJob = onlyPanel.isWarningatJoblistEnlarge( comPrintJob );	// 今回のプリントジョブ表示警告状態
				if( 0 != DataCheckPrintJob ) {
					// 警告バックアップテーブルをチェック
					Bool b_EmergencySend = False;
					for( short i = 0; i < PDPD_EmergencySendTableNum; i++ ) {
						// 表示中ジョブと警告バックアップテーブルのジョブが一致した場合は、左下「警告表示」を表示する
						if( ( onlyPanel.s_EmergencySendTable[i].pc_Job == comPrintJob )
						    && ( onlyPanel.checkExistWarningBackupTable( onlyPanel.s_EmergencySendTable[i].ec_WarningName, sp_GamenEnlWarningTable[DataCheckPrintJob].ec_WarningName ) == True ) ) {
							b_EmergencySend = True;
						}
					}
					//	バックアップした警告表示中でなければ、プリントジョブを警告表示中ジョブに設定
					if( False == b_EmergencySend
					    && TYPE_OutputStatusDeleting != comPrintJob->getJobStatus()->getOutputStatus() ) {
						onlyPanel.cc_DisplayJob = comPrintJob;
					}
				}
			} else {
				//	対象の警告はジョブリストと同じ
				register ushort DataCheckPrintJob = onlyPanel.isWarningatJoblist( comPrintJob );	// 今回のプリントジョブ表示警告状態
				if( 0 != DataCheckPrintJob ) {
					// 警告バックアップテーブルをチェック
					Bool b_EmergencySend = False;
					for( short i = 0; i < PDPD_EmergencySendTableNum; i++ ) {
						// 表示中ジョブと警告バックアップテーブルのジョブが一致した場合は、左下「警告表示」を表示する
						if( ( onlyPanel.s_EmergencySendTable[i].pc_Job == comPrintJob )
						    && ( onlyPanel.checkExistWarningBackupTable( onlyPanel.s_EmergencySendTable[i].ec_WarningName, sp_GamenWarningTable[DataCheckPrintJob].ec_WarningName ) == True ) ) {
							b_EmergencySend = True;
						}
					}
					//	バックアップした警告表示中でなければ、プリントジョブを警告表示中ジョブに設定
					if( False == b_EmergencySend
					    && TYPE_OutputStatusDeleting != comPrintJob->getJobStatus()->getOutputStatus() ) {
						onlyPanel.cc_DisplayJob = comPrintJob;
					}
				}
			}
		}
		b_EmergencySend = False;
		if( APIC_JobInstanceList::isExistJob( cp_reserveCopyJob ) == True
		    && comPrintJob != cp_reserveCopyJob
		    && TYPE_InputStatusScanEnd != cp_reserveCopyJob->getJobStatus()->getInputStatus() ) {
			// ジョブリストのチェックを拡大と通常で分ける.
			if( onlyPanel.bl_isEnlargeDisplayMode == True ) {	// 拡大表示モード時.
				//	対象の警告は全画面警告.
				register ushort DataCheckReserveJob = onlyPanel.isWarningatJoblistEnlarge( cp_reserveCopyJob );	// 今回の次ジョブ予約押下時のリザーブジョブ表示警告状態.
				if( 0 != DataCheckReserveJob ) {
					// 警告バックアップテーブルをチェック.
					b_EmergencySend = False;
					for( short i = 0; i < PDPD_EmergencySendTableNum; i++ ) {
						// 表示中ジョブと警告バックアップテーブルのジョブが一致した場合は、左下「警告表示」を表示する.
						if( ( onlyPanel.s_EmergencySendTable[i].pc_Job == cp_reserveCopyJob )
						    && ( onlyPanel.checkExistWarningBackupTable( onlyPanel.s_EmergencySendTable[i].ec_WarningName, sp_GamenEnlWarningTable[DataCheckReserveJob].ec_WarningName ) == True ) ) {
							b_EmergencySend = True;
						}
					}
					//	バックアップした警告表示中でなければ、次ジョブ予約押下時のリザーブジョブを警告表示中ジョブに設定.
					if( False == b_EmergencySend
					    && TYPE_OutputStatusDeleting != cp_reserveCopyJob->getJobStatus()->getOutputStatus() ) {
						onlyPanel.cc_DisplayJob = cp_reserveCopyJob;
					}
				}
			} else {
				//	対象の警告は全画面警告.
				register ushort DataCheckReserveJob = onlyPanel.isWarningatJoblist( cp_reserveCopyJob );	// 今回の次ジョブ予約押下時のリザーブジョブ表示警告状態.
				if( 0 != DataCheckReserveJob ) {
					// 警告バックアップテーブルをチェック.
					b_EmergencySend = False;
					for( short i = 0; i < PDPD_EmergencySendTableNum; i++ ) {
						// 表示中ジョブと警告バックアップテーブルのジョブが一致した場合は、左下「警告表示」を表示する.
						if( ( onlyPanel.s_EmergencySendTable[i].pc_Job == cp_reserveCopyJob )
						    && ( onlyPanel.checkExistWarningBackupTable( onlyPanel.s_EmergencySendTable[i].ec_WarningName, sp_GamenWarningTable[DataCheckReserveJob].ec_WarningName ) == True ) ) {
							b_EmergencySend = True;
						}
					}
					//	バックアップした警告表示中でなければ、次ジョブ予約押下時のリザーブジョブを警告表示中ジョブに設定.
					if( False == b_EmergencySend
					    && TYPE_OutputStatusDeleting != cp_reserveCopyJob->getJobStatus()->getOutputStatus() ) {
						onlyPanel.cc_DisplayJob = cp_reserveCopyJob;
					}
				}
			}
		}
	}
	return bl_result;
}
/*================================================================================================
［機能］ データの変化エッジチェック : スキャナ配信方式の変化 (TWAIN)
================================================================================================*/
Bool PBasicMessageProc::isScanJobSendChange()
{
	PNLC_NotifyEventInfo* pc_NtfEventInfo = PNLC_NotifyEventInfo::newInstance();		// データの管理クラス.
	if( NULL == pc_NtfEventInfo ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}

	static TYPE_JobType PreDataCheckJobType = TYPE_JobTypeCopy;	// 前回の状態
	register Bool b_result = False;								// チェック結果
	// TWAINパネルロック中であれば.
	if( TYPE_TwainLockNon != pc_NtfEventInfo->getTwainLock() ) {
		// ジョブがNULLでなく現ジョブがTwainジョブでなければ.
		if( NULL != ppc_SystemStatus->getTwainJob()
		    && TYPE_JobTypeTwain != comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() ) {
//                             画像出力状態が破棄中、またはジョブが無効の時Twainジョブを取得しない.
			APIC_Job* c_Job = ppc_SystemStatus->getTwainJob();
			if( NULL == c_Job ) {
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
				return False;
			}
			if( c_Job->getJobStatus()->getOutputStatus() != TYPE_OutputStatusDeleting
			    && ( TYPE_InputStatusScanEnd != c_Job->getJobStatus()->getInputStatus() )
			    && APIC_JobInstanceList::isExistJob( c_Job ) != False ) {
				// Twainジョブ取得.
				c_JobControl.setTwainTurnJob( comPanelJob );
				comPanelJob = ppc_SystemStatus->getTwainJob();
				onlyPanel.cc_DisplayJob = comPanelJob;
				PNLC_WarningManager::newInstance()->setReqChangeGamen( True );
			}
		}
	}
	if( PreDataCheckJobType != comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType()
	    && ( PreDataCheckJobType == TYPE_JobTypeTwain
	         || TYPE_JobTypeTwain == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() ) ) {
		b_result = True;
		PreDataCheckJobType = comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType();
	}
	APIC_SoftDipSw* pc_APIC_SoftDipSw = APIC_SoftDipSw::newInstance();
	Bool bl_SoftDipSw212_bit4 = False;				// DipSWNo.212 bit4 False.
	if( NULL != pc_APIC_SoftDipSw ) {
		bl_SoftDipSw212_bit4 = pc_APIC_SoftDipSw->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW212, 4 );	// DipSWNo.212 bit4.
	}
	if( True == bl_SoftDipSw212_bit4 ) {			// DipSWNo.212 bit4 ON.
		static TYPE_SeparateScanForTwain e_PreDataCheckSeparate = TYPE_SeparateScanForTwain_Off;	// 前回の状態（連続読み込み）.
		TYPE_SeparateScanForTwain e_CurDataCheckSeparate = ppc_SystemStatus->getSeparateScanForTwain();		//	現在の状態.
		// 連続読み込みON時に読み込み終了ボタンを押下すると、連続読み込み設定が変化する.
		// それを検知して画面の切り替えを行う.
		if( e_PreDataCheckSeparate != e_CurDataCheckSeparate ) {
			e_PreDataCheckSeparate = e_CurDataCheckSeparate;
			b_result = True;
		}
	}

	return b_result;
}
/*================================================================================================
［機能］ データの変化エッジチェック : スキャナ読み込み開始変化
================================================================================================*/
Bool PBasicMessageProc::isScanJobScanStartChange()
{
	static TYPE_InputStatus ec_PreStatus = TYPE_InputStatusIdle;	// 前回の状態
	register Bool bl_result = False;								// チェック結果
	if( comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() == TYPE_JobTypeScan
	    && ec_PreStatus != comPanelJob->getJobStatus()->getInputStatus()
	    && comPanelJob->getJobStatus()->getInputStatus() == TYPE_InputStatusScanExec ) {
		// 読み込みジョブがスキャナで、読み込み中に変化した
		bl_result = True;
	}
	ec_PreStatus = comPanelJob->getJobStatus()->getInputStatus();	// 前回の状態設定
	return bl_result;
}
/*================================================================================================
［機能］ データの変化エッジチェック : 動作中ジョブ変化
================================================================================================*/
Bool PBasicMessageProc::isActiveJobEnd()
{
	static   Bool bl_PreJobActive = False;	// 前回の動作中ジョブ状態
	register Bool bl_JobActive;				// 今回の動作中ジョブ状態
	register Bool bl_JonNon;				// 動作しているジョブ無し
	if( ( comPanelJob->getJobStatus()->getInputStatus() == TYPE_InputStatusIdle
	      && comPanelJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusIdle )
	    && ( comPrintJob == NULL ) ) {
		bl_JobActive = False;
	} else {
		bl_JobActive = True;
	}
	if( ( bl_PreJobActive != bl_JobActive ) && bl_JobActive == False ) {
		// 動作中のジョブが無くなった
		bl_JonNon = True;
	} else {
		bl_JonNon = False;
	}
	bl_PreJobActive = bl_JobActive;	// 今回のジョブ状態を前回のジョブ状態へ設定
	return bl_JonNon;
}
/*================================================================================================
［機能］ データの変化エッジチェック : 原稿サイズ有効の変化
================================================================================================*/
Bool PBasicMessageProc::isDocSizeEffective()
{
	static APIC_Job* PrePrintJob = NULL;		// 対象となる前回のジョブ.
	static Bool b_PrePlatenDataCheck = False;	// 前回の原稿台原稿サイズ状態.
	static Bool b_PreAdfDataCheck = False;		// 前回のADF原稿サイズ状態.
	register Bool b_result = False;				// チェック結果.
	register Bool b_PlatenData = False;
	register Bool b_AdfData = False;
	if( comPanelJob->isPlatenOriginalSizeFixed() == True
	    && ppc_Original->getPlatenOriginalSize().e_SizeCode != TYPE_SizeCodeUnknown ) {	// 原稿台原稿サイズ有効になった場合.
		b_PlatenData = True;
	}
	if( comPanelJob->isADFOriginalSizeFixed() == True
	    && ppc_Original->getAdfOriginalSize().e_SizeCode != TYPE_SizeCodeUnknown ) {	// ADF原稿サイズ有効になった場合.
		b_AdfData = True;
	}
// EDHに原稿を置いた後、ガイド幅を変更された場合は、原稿サイズのみ変化するため、原稿サイズ変化を検知する.
	static	TYPE_SizeCode	e_preSize = TYPE_SizeCodeUnknown ;
	TYPE_SizeCode	e_detectSize ;
	if( b_AdfData == True ) {			// ADF検知有効
		e_detectSize = ppc_Original->getAdfOriginalSize().e_SizeCode ;
	} else if( b_PlatenData == True ) {	// 原稿ガラス検知有効
		e_detectSize = ppc_Original->getPlatenOriginalSize().e_SizeCode ;
	} else {							// ADF、原稿ガラスともに無効
		e_detectSize = TYPE_SizeCodeUnknown ;	// 不明
	}
	// ADF/原稿ガラスの原稿検出位置が変化、または原稿検知サイズが変化したかチェック
	if( ( b_PrePlatenDataCheck != b_PlatenData ) || ( b_PreAdfDataCheck != b_AdfData ) || ( e_preSize != e_detectSize ) ) {
		if( comPanelJob->getAutoSelect() != NULL ) {
			// AutoSelectが取得できる時のみ(すなわちJOBが生きている時のみ)変化ありとする。
			// (AutoSelectが取得できないとき(すなわちJOBがdeleteされているとき)は変化なし).
			b_result = True ;	// 変化あり.
		}
		else {
			b_result = False ;	// 変化なし.
		}
	} else {
		b_result = False ;	// 変化なし
	}

	if( b_result == True ) {
		// 変化ありの時のみ、前回の状態を更新。
		// (comPanelJobが破棄された直後にこの処理が動いた場合、原稿検知サイズ、プラテン状態、ADF状態に
		//  変化があっても無条件で変化なしにしているので、その後正常なcomPanelJobに切り替わった後.
		//  状態の変化があったことにさせたいため。)
		e_preSize = e_detectSize ;	// 原稿検知サイズを保持
		b_PrePlatenDataCheck = b_PlatenData;						// 前回の状態に格納.
		b_PreAdfDataCheck = b_AdfData;								// 前回の状態に格納.
	}

	// comPanelJob削除中ジョブ、次ジョブ予約読み込み中.
	if( APIC_JobInstanceList::isExistJob( cp_reserveCopyJob ) == True ) {
		TYPE_InputStatus e_InputStatus = cp_reserveCopyJob->getJobStatus()->getInputStatus();
		APIC_AutoSelect* cp_AutoSelect = cp_reserveCopyJob->getAutoSelect();
		if( NULL == cp_AutoSelect ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return False;
		}
		if( TYPE_OriginalSettingADF == cp_AutoSelect->chkOriginalSetting( True )
		    && ( TYPE_InputStatusScanExec == e_InputStatus
		         || TYPE_InputStatusScanStop == e_InputStatus
		         || TYPE_InputStatusScanErrorStop == e_InputStatus
		         || TYPE_InputStatusMemoryInScanStop == e_InputStatus ) ) {
			return b_result;
		}
	}
	// 対象となるジョブが変わった場合.
	b_targetJobChange = False;
	if( PrePrintJob != comPanelJob ) {
		b_result = True;
		b_targetJobChange = True;
	}
	PrePrintJob = comPanelJob;
	return b_result;
}
/*================================================================================================
［機能］ データの変化エッジチェック : 省エネ状態の変化
================================================================================================*/
Bool PBasicMessageProc::isEnergySaveChange()
{
	static Bool b_PreDataCheck = False;		// 前回の状態.
	static Bool b_PreWeeklyTimerTempWakeUpCheck = False;	// 前回のウィークリータイマー一時起動状態.
	register Bool b_status = False;			// プレヒート・スリープ状態.
	register Bool b_result = False;			// チェック結果.
	PNLC_NotifyEventInfo* pc_NtfEventInfo = PNLC_NotifyEventInfo::newInstance();		// データの管理クラス.
	if( NULL == pc_NtfEventInfo ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	// 前回とのエッジ検出.
	if( b_PreWeeklyTimerTempWakeUpCheck != c_PowerObserver.nowWeeklyTimerTempWakeUp() ) {
		b_result = True;
	}
	TYPE_PowerSaveStatus	e_PowerSaveStatus = pc_NtfEventInfo->getPowerSaveStatus();
	// スリープ・省電力モードの状態取得.
	if( ( True == PnlComIsPowerSaveMode( e_PowerSaveStatus ) )				// パワーセーブモード時（バックライトＯＦＦ時）.
	    &&( e_PowerSaveStatus != TYPE_PowerSaveStatusSubPowerOnProc ) ) {	// 副電源ON処理中
		b_status = True;
	}

	static TYPE_HddStatus e_PreHddStatus = TYPE_HddStatusNon;					// 前回のHDD状態.
	Bool bl_hdd_status = False;
	APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
	TYPE_HddStatus e_CurrentHddStatus = pc_HddInfo->getHddStatus();
	if( PnlComGetHddInfo()->getHddExist() == True ) {
		// HDDステータスをチェック.
		if( ( e_PreHddStatus != e_CurrentHddStatus )
		    && ( TYPE_HddStatusFormated == e_CurrentHddStatus ) ) {
			bl_hdd_status = True;
		}
		e_PreHddStatus = e_CurrentHddStatus;
	}

	if( True == c_PowerObserver.nowWeeklyTimerTempWakeUp()
	    && b_PreWeeklyTimerTempWakeUpCheck == c_PowerObserver.nowWeeklyTimerTempWakeUp() ) {
		// 一時起動中状態のためスリープ・省電力モード変化エッジを取得しない.
	} else {
		if( b_PreDataCheck != b_status ) {
			b_result = True;
// comment 2016/01/06 OPS1-Sky浜野 [MinervaSBK][CQ523134] .
// ウィークリータイマー一時起動中のスリープ復帰についても状態設定を行う為、設定タイミングを変更する.
//// comment 2015/11/19 OPS1-Sky浜野 [MinervaSBK][CQ521439].
//			PNLC_ActionUserLoginControlLib::newInstance()->setEnergySaveChange( b_status );
		}
		if( False == b_status ) {	// 省電力解除状態のみ
			// 拡大画面は立ち上げ時の処理が通常画面と異なり、省電力解除状態を定期起床でチェックしていない.
			// 電源状態が変化したときにHDDが準備状態でないと、procEnergySaveReset()で省電力解除時の処理が実行されないため.
			// 電源状態変化後も、HDD状態の変化をチェックしてprocEnergySaveReset()がコールされるようにする.
			// 影響範囲を抑えるため、ここで「初期表示を拡大画面とする」場合のみHDDの状態変化をチェックする.
// comment 2018/08/24 SCD2-4G東島 [Sparrow][1051906].
//			if( True == onlyPanel.bl_isEnlargeDisplayMode && True == PnlComGetOperationSetting()->getLargeDisplayInitDispSetting() ) {
			if( ( True == onlyPanel.bl_isEnlargeDisplayMode )
			 && ( False == gp_ActionInitialSetup.getFirstBoot() )			// 初期立上げではない.		// comment 2018/10/19 SCD2-4G東島 [Sparrow][1051906].
			 && ( True == PnlComIsLargeDisplayInitDisp() ) ){				// 拡大が初期表示の場合.
				if( True == bl_hdd_status ) {
					b_result = True;
					onlyPanel.setSimpleSecurityHDDchk( True );
				}
			}
		}
	}

// comment 2016/01/06 OPS1-Sky浜野 [MinervaSBK][CQ523134].
	// スリープ復帰エッジ検出.
	if (b_PreDataCheck != b_status) {
		// ユーザーログイン管理でのスリープ復帰状態を設定する.
		PNLC_ActionUserLoginControlLib::newInstance()->setEnergySaveChange( b_status );
	}

	// 前回の状態に格納.
	b_PreDataCheck = b_status;				// 前回の状態に格納.
	b_PreWeeklyTimerTempWakeUpCheck = c_PowerObserver.nowWeeklyTimerTempWakeUp();
	return b_result;
}
/*================================================================================================
［機能］ データの変化エッジチェック : 手差しトレイへの用紙挿入
================================================================================================*/
Bool PBasicMessageProc::isPaperInsertToManualFeed()
{
	static TYPE_PaperStatus PreDataCheck = TYPE_PaperStatusEmpty;	// 前回の状態
	register TYPE_PaperStatus DataCheck = TYPE_PaperStatusEmpty;	// 今回の状態
	register Bool bl_result = False;								// チェック結果
	PNLC_NotifyEventInfo* pc_NtfEventInfo = PNLC_NotifyEventInfo::newInstance();		// データの管理クラス.
	if( NULL == pc_NtfEventInfo ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}

	DataCheck = pc_NtfEventInfo->getPaperStatus( TYPE_FeedTrayManual );
	bl_result = ( PreDataCheck != DataCheck )? True : False;
	PreDataCheck = DataCheck;				// 前回の状態に格納
	Bool bl_PaperUnKnown = False;
	static Bool bl_PrePaperUnKnown = True;
	bl_PaperUnKnown = ppc_FeedTray -> getPaperUnKnownInfo( TYPE_FeedTrayManual );
	if( bl_PaperUnKnown==True ) {
		bl_result = False;
	} else if( bl_PaperUnKnown != bl_PrePaperUnKnown && bl_PrePaperUnKnown == True ) {
		bl_result = False;
	}
	bl_PrePaperUnKnown = bl_PaperUnKnown;
	return bl_result;
}
// comment 2016/05/17 SCD2-Sky辻 [特ROM][FQ-OPSD-1600010-OFP008]手差しトレイに用紙をセットしたままパワーセーブ移行後、復帰すると「トレイ（手差し）の設定変更」画面が表示される. 
/*================================================================================================
［機能］ データの変化エッジチェック : 手差しトレイへの用紙挿入  
［備考］ isPaperInsertToManualFeedを廃止して替わりに本関数をコールする.
================================================================================================*/
PNLE_PAPER_CHANGE_STATUS PBasicMessageProc::checkPaperInsertToManualFeed()
{
	static TYPE_PaperStatus PreDataCheck = TYPE_PaperStatusEmpty;			// 前回の用紙状態
	register TYPE_PaperStatus DataCheck = TYPE_PaperStatusEmpty;			// 今回の用紙状態

	static Bool bl_PrePaperUnKnown = True;									// 前回の未確定状態.
	Bool bl_PaperUnKnown = False;											// 今回の未確定状態.

	register PNLE_PAPER_CHANGE_STATUS e_result = PNLE_PAPER_CHANGE_NONE;	// チェック結果.

	// 用紙状態と未確定状態を同タイミングで一度に取得することで各状態の整合性をあわせる.
	DataCheck = ppc_FeedTray->getPaperStatusForPanel( TYPE_FeedTrayManual );

	// 状態未確定以外は状態確定(bl_PaperUnKnown = False)である.
	if ( TYPE_PaperStatusUndetermined == DataCheck ) {
		// 状態未確定はペーパー無しでもある.
		DataCheck = TYPE_PaperStatusEmpty;
		bl_PaperUnKnown = True;
	}
	
	if ( PreDataCheck != DataCheck) {
		if ( TYPE_PaperStatusExist == DataCheck ) {
			// 用紙有りに変化した.
			e_result = PNLE_PAPER_CHANGE_INSERT;
		} else {
			// 用紙無しに変化した.
			e_result = PNLE_PAPER_CHANGE_REMOVE;
		}
	}
	PreDataCheck = DataCheck;
	
	if ( bl_PaperUnKnown == True ) {
		// 今回「未確定」の場合は用紙の変化無しにする.
		e_result = PNLE_PAPER_CHANGE_NONE;
	} else if( bl_PaperUnKnown != bl_PrePaperUnKnown && bl_PrePaperUnKnown == True ) {
		// 前回「未確定」で今回「確定」の場合は用紙の変化無しにする.
		e_result = PNLE_PAPER_CHANGE_NONE;
	}
	bl_PrePaperUnKnown = bl_PaperUnKnown; 

	return e_result;
}

// Comment -> 2017/05/12 SCD2 3G Sky瀬戸 [AccuRev.1014826]media-ready対応.
#ifdef DEF_A4_Cus
/*================================================================================================
［機能］ データの変化エッジチェック : 手差しトレイへの用紙挿入  
================================================================================================*/
PNLE_PAPER_CHANGE_STATUS PBasicMessageProc::checkPaperInsertToTray( TYPE_FeedTray e_FeedTray )
{
#ifdef DEF_KRONOSS_Cus
	static const TYPE_FeedTray ChackTable[] = { TYPE_FeedTray1,	TYPE_FeedTray2, TYPE_FeedTray3, TYPE_FeedTray4 };
#else // DEF_KRONOSS_Cus.
	static const TYPE_FeedTray ChackTable[] = { TYPE_FeedTray1 };
#endif // DEF_KRONOSS_Cus.
	const ushort us_ChackTableSize = ( ushort )( sizeof( ChackTable )/sizeof( TYPE_FeedTray ) );
	
	static TYPE_PaperStatus PreDataCheck[us_ChackTableSize] = {TYPE_PaperStatusEmpty};		// 前回の用紙状態.
	register TYPE_PaperStatus DataCheck[us_ChackTableSize] = {TYPE_PaperStatusEmpty};		// 今回の用紙状態
	
	static Bool bl_PrePaperUnKnown[us_ChackTableSize] = {True};								// 前回の未確定状態.
	Bool bl_PaperUnKnown[us_ChackTableSize] = {False};										// 今回の未確定状態.
	
	register PNLE_PAPER_CHANGE_STATUS e_result = PNLE_PAPER_CHANGE_NONE;					// チェック結果.
	
	ushort us_CheckNum = 0;
	for ( ushort us_Cnt = 0; us_Cnt < us_ChackTableSize; us_Cnt++ ) {
		if ( ChackTable[us_Cnt] == e_FeedTray ) {
			us_CheckNum = us_Cnt;
			break;
		}
	}
	
	// 用紙状態と未確定状態を同タイミングで一度に取得することで各状態の整合性をあわせる.
	DataCheck[us_CheckNum] = ppc_FeedTray->getPaperStatusForPanel( ChackTable[us_CheckNum] );
	
	// 状態未確定以外は状態確定(bl_PaperUnKnown = False)である.
	if ( TYPE_PaperStatusUndetermined == DataCheck[us_CheckNum] ) {
		// 状態未確定はペーパー無しでもある.
		DataCheck[us_CheckNum] = TYPE_PaperStatusEmpty;
		bl_PaperUnKnown[us_CheckNum] = True;
	}
	
	if ( PreDataCheck[us_CheckNum] != DataCheck[us_CheckNum] ) {
		if ( TYPE_PaperStatusExist == DataCheck[us_CheckNum] ) {
			// 用紙有りに変化した.
			e_result = PNLE_PAPER_CHANGE_INSERT;
		} else {
			// 用紙無しに変化した.
			e_result = PNLE_PAPER_CHANGE_REMOVE;
		}
	}
	PreDataCheck[us_CheckNum] = DataCheck[us_CheckNum];
	
	if ( bl_PaperUnKnown[us_CheckNum] == True ) {
		// 今回「未確定」の場合は用紙の変化無しにする.
		e_result = PNLE_PAPER_CHANGE_NONE;
	} else if( bl_PaperUnKnown[us_CheckNum] != bl_PrePaperUnKnown[us_CheckNum] && bl_PrePaperUnKnown[us_CheckNum] == True ) {
		// 前回「未確定」で今回「確定」の場合は用紙の変化無しにする.
		e_result = PNLE_PAPER_CHANGE_NONE;
	}
	bl_PrePaperUnKnown[us_CheckNum] = bl_PaperUnKnown[us_CheckNum]; 
	
	return e_result;
}
#endif // DEF_A4_Cus.
// Comment <- 2017/05/12 SCD2 3G Sky瀬戸 [AccuRev.1014826]media-ready対応.

// 新アーキ支援 パフォーマンス改善 仁枝 start.
///*================================================================================================
//［機能］ データの変化エッジチェック : ＡＤＦ原稿トレイの状況変化
//================================================================================================*/
Bool PBasicMessageProc::isDocOnADF()
{
	static Bool bl_PreDataCheck	= False;	// 前回の状態
	register Bool bl_result = False;		// チェック結果
	PNLC_NotifyEventInfo* pc_NtfEventInfo = PNLC_NotifyEventInfo::newInstance();		// データの管理クラス.
	if( NULL == pc_NtfEventInfo ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return bl_result;
	}
	bl_result = ( bl_PreDataCheck != pc_NtfEventInfo->getAdfOriginalExist() ) ? True: False;	// 原稿位置情報のシングルフィード原稿あり変化をチェック.
	bl_PreDataCheck = pc_NtfEventInfo->getAdfOriginalExist();								// 前回の状態に格納.
	return bl_result;
}
///*================================================================================================
//［機能］ データの変化エッジチェック : 原稿カバー開閉の状況変化
//================================================================================================*/
Bool PBasicMessageProc::isDocCoverOpen()
{
	static Bool bl_PreDataCheck	= False;	// 前回の状態
	register Bool bl_result = False;		// チェック結果
	PNLC_NotifyEventInfo* pc_NtfEventInfo = PNLC_NotifyEventInfo::newInstance();		// データの管理クラス.
	if( NULL == pc_NtfEventInfo ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return bl_result;
	}
	bl_result = ( bl_PreDataCheck != pc_NtfEventInfo->getPlatenCoverOpen() ) ? True: False;		// 原稿カバー開閉状態のチェック.
	bl_PreDataCheck = pc_NtfEventInfo->getPlatenCoverOpen();									// 前回の状態に格納.
	return bl_result;
}
/*================================================================================================
［機能］ データの変化エッジチェック : 給紙口情報の変化
================================================================================================*/
TYPE_FeedTray PBasicMessageProc::FeedInfoChange()
{
	static const TYPE_FeedTray ChackTable[] = { TYPE_FeedTray1,	TYPE_FeedTray2, TYPE_FeedTray3, TYPE_FeedTray4, TYPE_FeedTray5, TYPE_FeedTrayPI1, TYPE_FeedTrayPI2 };
	// 給紙口チェック用のテーブル、手差しの変化はここで見ない為除外
	static TYPE_PaperStatus PreDataPaperStatus[( ushort )TYPE_FeedTrayMax] = {TYPE_PaperStatusEmpty};		// 前回の状態.
	static TYPE_TrayStatus PreDataTrayStatus[( ushort )TYPE_FeedTrayMax] = {TYPE_TrayStatusNon};			// 前回の状態.
	register TYPE_FeedTray ec_FC_result = TYPE_FeedTrayMax;							// チェック結果.
	PNLC_NotifyEventInfo* pc_NtfEventInfo = PNLC_NotifyEventInfo::newInstance();		// データの管理クラス.
	if( NULL == pc_NtfEventInfo ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return ec_FC_result;
	}
	for( register ushort i = 0; i < sizeof( ChackTable )/sizeof( TYPE_FeedTray ); i++ ) {
		if( PreDataPaperStatus[ChackTable[i]] != pc_NtfEventInfo->getPaperStatus( ChackTable[i] )			// 【パフォーマンス改善】2012/08/22 第一OSW開発 酒井(健).
		    || PreDataTrayStatus[ChackTable[i]] != pc_NtfEventInfo->getTrayStatus( ChackTable[i] ) ) {		// 【パフォーマンス改善】2012/08/22 第一OSW開発 酒井(健).
			// どの給紙口の変化があってもイベントを発生させる.
			ec_FC_result = ChackTable[i];					// 変化を検出した給紙口をresultとして格納.
			PreDataPaperStatus[ChackTable[i]] = pc_NtfEventInfo->getPaperStatus( ChackTable[i] );	// 前回の状態に格納. 【パフォーマンス改善】2012/08/22 第一OSW開発 酒井(健).
			PreDataTrayStatus[ChackTable[i]] = pc_NtfEventInfo->getTrayStatus( ChackTable[i] );	// 前回の状態に格納. 【パフォーマンス改善】2012/08/22 第一OSW開発 酒井(健).
			// 複数同時発生した場合、後の給紙口について返してしまうが良しとしておく
		}
	}
	return ec_FC_result;
}
/*================================================================================================
［機能］	データの変化エッジチェック : 用紙サイズ・用紙種類の変化
================================================================================================*/
struct TYPS_FeedPaperSizeKind {
	TYPE_FeedTray  e_FeedTray;
	ushort		   us_ChangeFeedTray;
	TYPE_SizeCode  e_SizeCode;
	float		   sf_PaperSizeX;
	float		   sf_PaperSizeY;
	Bool		   b_WideOn;
	TYPE_PaperType e_PaperType;
	Bool		   b_Pre2ndSided;
	TYPE_WidePaperImagePosition  e_PreWidePaperImagePosition;
};
Bool PBasicMessageProc::isFeedPaperKindChange( ushort* pus_ChangeFeedTray )
{
	PNLC_NotifyEventInfo* pc_NtfEventInfo = PNLC_NotifyEventInfo::newInstance();		// データの管理クラス.
	if( NULL == pc_NtfEventInfo ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	// 前回の状態.
	static TYPS_FeedPaperSizeKind s_AryFeedPaperSizeKind[] = {
		{ TYPE_FeedTrayManual, DEF_FeedTrayChangeManual, TYPE_SizeCodeUnknown, 0.0f, 0.0f, False, TYPE_PaperTypeNormal, False, TYPE_WidePaperImagePositionCenter },
		{ TYPE_FeedTray1,	   DEF_FeedTrayChange1,		 TYPE_SizeCodeUnknown, 0.0f, 0.0f, False, TYPE_PaperTypeNormal, False, TYPE_WidePaperImagePositionCenter },
		{ TYPE_FeedTray2,	   DEF_FeedTrayChange2,		 TYPE_SizeCodeUnknown, 0.0f, 0.0f, False, TYPE_PaperTypeNormal, False, TYPE_WidePaperImagePositionCenter },
		{ TYPE_FeedTray3,	   DEF_FeedTrayChange3,		 TYPE_SizeCodeUnknown, 0.0f, 0.0f, False, TYPE_PaperTypeNormal, False, TYPE_WidePaperImagePositionCenter },
		{ TYPE_FeedTray4,	   DEF_FeedTrayChange4,		 TYPE_SizeCodeUnknown, 0.0f, 0.0f, False, TYPE_PaperTypeNormal, False, TYPE_WidePaperImagePositionCenter },
		{ TYPE_FeedTray5,	   DEF_FeedTrayChange5,		 TYPE_SizeCodeUnknown, 0.0f, 0.0f, False, TYPE_PaperTypeNormal, False, TYPE_WidePaperImagePositionCenter },
		{ TYPE_FeedTrayPI1,	   DEF_FeedTrayChangePI1,	 TYPE_SizeCodeUnknown, 0.0f, 0.0f, False, TYPE_PaperTypeNormal, False, TYPE_WidePaperImagePositionCenter },
		{ TYPE_FeedTrayPI2,	   DEF_FeedTrayChangePI2,	 TYPE_SizeCodeUnknown, 0.0f, 0.0f, False, TYPE_PaperTypeNormal, False, TYPE_WidePaperImagePositionCenter },
	};
	const uchar uc_AryFeedPaperSizeKindSize = ( uchar )( sizeof( s_AryFeedPaperSizeKind ) / sizeof( TYPS_FeedPaperSizeKind ) );
	if( NULL == pus_ChangeFeedTray ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	// チェック結果の初期化.
	register Bool b_Result = False;
	*pus_ChangeFeedTray = 0;
	for( uchar uc_Tray = 0; uc_AryFeedPaperSizeKindSize > uc_Tray; uc_Tray++ ) {
		const TYPE_FeedTray	 e_FeedTray		   = s_AryFeedPaperSizeKind[uc_Tray].e_FeedTray;
		const ushort		 us_ChangeFeedTray = s_AryFeedPaperSizeKind[uc_Tray].us_ChangeFeedTray;
		const TYPE_SizeCode	 e_SizeCode		   = s_AryFeedPaperSizeKind[uc_Tray].e_SizeCode;
		const float			 sf_PaperSizeX	   = s_AryFeedPaperSizeKind[uc_Tray].sf_PaperSizeX;
		const float			 sf_PaperSizeY	   = s_AryFeedPaperSizeKind[uc_Tray].sf_PaperSizeY;
		const Bool			 b_WideOn		   = s_AryFeedPaperSizeKind[uc_Tray].b_WideOn;
		const TYPE_PaperType e_PaperType	   = s_AryFeedPaperSizeKind[uc_Tray].e_PaperType;
		const Bool			 b_Pre2ndSided	   = s_AryFeedPaperSizeKind[uc_Tray].b_Pre2ndSided;
		const TYPE_WidePaperImagePosition e_PreWidePaperImagePosition = s_AryFeedPaperSizeKind[uc_Tray].e_PreWidePaperImagePosition;
		if( TYPE_TrayStatusReady == pc_NtfEventInfo->getTrayStatus( e_FeedTray ) ) {
			// 用紙サイズの取得.
			TYPC_PaperSize c_PaperSize = ppc_FeedTray->getPaperSize( e_FeedTray );
			// 前回の状態と比較.
			if( ( e_SizeCode	  != c_PaperSize.e_SizeCode )
			    || ( sf_PaperSizeX != c_PaperSize.c_SizeMM.x )
			    || ( sf_PaperSizeY != c_PaperSize.c_SizeMM.y )
			    || ( b_WideOn	  != c_PaperSize.b_WideOn )
			    || ( e_PaperType	  != ppc_FeedTray->getPaperType( e_FeedTray ).getPaperType() )
			    || ( b_Pre2ndSided != ppc_FeedTray->getPaperType( e_FeedTray ).getFeed2ndSidedPaper() )
			    || ( e_PreWidePaperImagePosition != ppc_FeedTray->getPaperType( e_FeedTray ).getWidePaperImagePosition() ) ) {
				// 変更トレイを記憶.
				*pus_ChangeFeedTray |= us_ChangeFeedTray;
				// 1つでも変更されているトレイがあれば戻り値はTrue.
				b_Result = True;
			}
			// 前回の状態に格納.
			s_AryFeedPaperSizeKind[uc_Tray].e_SizeCode	= c_PaperSize.e_SizeCode;
			s_AryFeedPaperSizeKind[uc_Tray].sf_PaperSizeX = c_PaperSize.c_SizeMM.x;
			s_AryFeedPaperSizeKind[uc_Tray].sf_PaperSizeY = c_PaperSize.c_SizeMM.y;
			s_AryFeedPaperSizeKind[uc_Tray].b_WideOn		= c_PaperSize.b_WideOn;
			s_AryFeedPaperSizeKind[uc_Tray].e_PaperType	= ppc_FeedTray->getPaperType( e_FeedTray ).getPaperType();
			s_AryFeedPaperSizeKind[uc_Tray].b_Pre2ndSided = ppc_FeedTray->getPaperType( e_FeedTray ).getFeed2ndSidedPaper();
			s_AryFeedPaperSizeKind[uc_Tray].e_PreWidePaperImagePosition = ppc_FeedTray->getPaperType( e_FeedTray ).getWidePaperImagePosition();
		}
	}
	return b_Result;
}
//概要    ：用紙サイズ・用紙種類のチェック
const TYPS_ChangeFeedTray s_ChangeFeedTray[] = {
	{ TYPE_FeedTrayManual, DEF_FeedTrayChangeManual },
	{ TYPE_FeedTray1,	   DEF_FeedTrayChange1		},
	{ TYPE_FeedTray2,	   DEF_FeedTrayChange2		},
	{ TYPE_FeedTray3,	   DEF_FeedTrayChange3		},
	{ TYPE_FeedTray4,	   DEF_FeedTrayChange4		},
	{ TYPE_FeedTray5,	   DEF_FeedTrayChange5		},
	{ TYPE_FeedTrayPI1,	   DEF_FeedTrayChangePI1	},
	{ TYPE_FeedTrayPI2,	   DEF_FeedTrayChangePI2	},
};
const TYPS_ChangeFeedTray* const ps_ChangeFeedTray = &( s_ChangeFeedTray[0] );
const uchar uc_ChangeFeedTraySize = ( uchar )( sizeof( s_ChangeFeedTray ) / sizeof( TYPS_ChangeFeedTray ) );
Bool PBasicMessageProc::isInvalidPaperSizeKind( ushort* pus_ChangeFeedTray )
{
	if( NULL == pus_ChangeFeedTray ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	// チェック結果の初期化.
	register Bool b_Result = False;
	*pus_ChangeFeedTray = 0;
	for( uchar i = 0; uc_ChangeFeedTraySize > i; ++i ) {
		const TYPE_FeedTray e_FeedTray = s_ChangeFeedTray[i].e_FeedTray;
		if( TYPE_TrayStatusReady == ppc_FeedTray->getTrayStatus( e_FeedTray ) ) {
			// 用紙種類の取得.
			TYPC_PaperType c_PaperType = ppc_FeedTray->getPaperType( e_FeedTray );
		}
	}
	return b_Result;
}
//概要    ：特殊画面表示の変化.
Bool PBasicMessageProc::isSpecialGamenShowStatus()
{
	register Bool b_result = False;										// チェック結果
	static Bool b_PreDeviceLockOn = False;								// 前回のデバイスロック状態.
	static TYPE_AdminModeApp e_PreAdminModeApp = TYPE_AdminModeAppNon;	// 前回のパネル以外の他アプリ管理者モード使用状態.
	static Bool b_PreDeleteOn = False;									// 前回の消去中状態.
// comment 2014/12/16 OPS1-Sky浜野 [Posedion][CQ505311] 3秒ポーリンング処理中は管理者ログイン禁止対応.
	static Bool b_PreAdminLoginForbidden_forPanel = False;
// 【STR】【パフォーマンス改善】2012/03/22 第一OSW開発 酒井(健).
	PNLC_NotifyEventInfo* pc_NtfEventInfo = PNLC_NotifyEventInfo::newInstance();		// データの管理クラス.
	if( NULL == pc_NtfEventInfo ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	onlyPanel.bl_isGetUsrInfoEndOfSpeGamen = False;

	TYPE_WasteDevStat e_TYPE_WasteDevStat = PnlComGetSystemSetting()->getWasteDeviceStatus();
	if( False == b_PreDeleteOn									// 前回が消去中でない.
	    && e_TYPE_WasteDevStat != TYPE_WasteDevStatNoWaste ) {	// 消去中.
		// データ消去開始.
		b_result = True;
		b_PreDeleteOn = True;
	} else if( True == b_PreDeleteOn							// 前回が消去中である.
	           && e_TYPE_WasteDevStat == TYPE_WasteDevStatNoWaste ) {	// 消去終了.
		// データ消去終了.
		b_result = True;
		b_PreDeleteOn = False;
	}
	// OpenAPI使用中.
	Bool b_DeviceLockOn = False;		// チェック結果.
	b_DeviceLockOn = pc_NtfEventInfo->getDeviceLockOn();		//【パフォーマンス改善】2012/02/27 第一OSW開発 酒井(健).
// comment 2019/07/01 SCD2-Sky中野 [EagleH][OP_BTS-7467].
	static Bool bl_IWSSetUpStatus = False;
	
// comment 2015/10/07 OPS1-Sky浜野 [MinervaSBK][CQ519840] デバイスロック処理対応.
// comment 2019/08/26 SCD2-4G Sky 二口 [EagleH][OP_BTS-11849] .	
	b_DeviceLockOn |= pc_NtfEventInfo->getNotRunDeviceLock();
// Comment 2019/06/07 [HCL-Kavitha][IT6_1.0MR2][AR1077447] Updating setNotRunDeviceLock after remote processing end.
//	if( False != pc_NtfEventInfo->getNotRunDeviceLock() ) { b_DeviceLockOn = True; }
// Comment 2019/06/14 [HCL-Kavitha][IT6_1.0MR2][AR1078144] Updating setNotRunDeviceLock after device lock process.
// comment 2019/08/26 SCD2-4G Sky 二口 [EagleH][OP_BTS-11849] .	
	if( b_DeviceLockOn != b_PreDeviceLockOn ) {
//	if( (b_DeviceLockOn != b_PreDeviceLockOn) || ( pc_NtfEventInfo->getNotRunDeviceLock() != b_PreDeviceLockOn)) {
//		if( False != pc_NtfEventInfo->getNotRunDeviceLock() ) { b_DeviceLockOn = True; } 
// comment 2016/07/20 ★A3共通問題★ HCL-Omkar [ZeusSBK][CQ531322] closing remote access screen when device lock status is OFF.
    	if ( False == b_DeviceLockOn && True == b_PreDeviceLockOn ) {
            if( &gp_RemoteUserAccess_act == PNLC_PopupListControl::newInstance()->getNowPopup() ) {
                    PNLC_PopupListControl::newInstance()->closeSpecifiedPopup( &gp_RemoteUserAccess_act );
            }
        }
        b_result = True;
        b_PreDeviceLockOn = b_DeviceLockOn;
// comment 2019/07/01 SCD2-Sky中野 [EagleH][OP_BTS-7467].
		if( True == b_DeviceLockOn ) {
// comment 2020/05/13 Sky中野 [IT6_2.0MR2][OP_BTS-23419].
//			bl_IWSSetUpStatus = APIC_BrowserInfo::newInstance()->getIWSSetUpStatus();
			if( True == pc_NtfEventInfo->getDeviceLockOn() ) {
				bl_IWSSetUpStatus = APIC_BrowserInfo::newInstance()->getIWSSetUpStatus();
			} else {
				bl_IWSSetUpStatus = onlyPanel.getIWSSetUpStatus();
			}
		}
		
// Comment 2018/02/21 [HCL-Rengaraaj] [IT5_4.2]  [AR1039265] Auto Reset issue when performing import operation. 
//		if( False == b_DeviceLockOn ) {
//			onlyPanel.bl_isGetUsrInfoEndOfSpeGamen = True;
//		}
// comment 2019/03/14 SCD2-Sky中野 [EagleH][OP_BTS-2755]デバイスロック解除時にログアウトしない問題対応.
//       if( False == b_DeviceLockOn && False == onlyPanel.getBrowserMode() ) {
		if( ( False == b_DeviceLockOn )
// comment 2019/07/01 SCD2-Sky中野 [EagleH][OP_BTS-7467].
		 && ( False == bl_IWSSetUpStatus )
		 && ( False == c_SystemControl.getAdminModeOn() )		// comment 2020/07/14 Sky中野 [IT6_2.0MR3][OP_BTS-24936].
		 && ( False == APIC_BrowserInfo::newInstance()->getLocalPSWCFlag() ) ) {
            onlyPanel.bl_isGetUsrInfoEndOfSpeGamen = True;
        } else {
// comment 2019/03/14 SCD2-Sky中野 [EagleH][OP_BTS-2755]デバイスロック解除時にログアウトしない問題対応.
//           if ( True == b_DeviceLockOn && True == onlyPanel.getBrowserMode() ) {
			if( ( True == b_DeviceLockOn )
// comment 2019/07/01 SCD2-Sky中野 [EagleH][OP_BTS-7467].
//			 && ( True == APIC_BrowserInfo::newInstance()->getLocalPSWCFlag() ) ) {
// comment 2020/07/14 Sky中野 [IT6_2.0MR3][OP_BTS-24936].
//			 && ( ( True == APIC_BrowserInfo::newInstance()->getLocalPSWCFlag() ) || ( True == bl_IWSSetUpStatus ) ) ) {
			 && ( ( True == APIC_BrowserInfo::newInstance()->getLocalPSWCFlag() )
			   || ( True == bl_IWSSetUpStatus )
			   || ( True == c_SystemControl.getAdminModeOn() ) ) ) {
                // SystemAutoReset用Timer using.
                if ( True == PnlComGetOperationSetting()->getAutoResetSystem().b_IsOn ) {
                    PMessageProcCom::c_autoRestTimer.c_AutoResetAll.stopTimer();                     // System Auto Reset Timer.
                } else {
                    PMessageProcCom::c_autoRestTimer.c_autoResetOffTimer.stopTimer();               // AutoReset OFF Timer.
                }
                // Browser Auto Reset用Timer Using.
                if( True == PnlComGetOperationSetting()->getAutoResetBrowser().b_IsOn ) {
                    PMessageProcCom::c_autoRestTimer.c_AutoResetBrowser.stopTimer();                // Browser Auto Reset Timer.
                }
            	b_result = False;
// comment 2019/03/14 SCD2-Sky中野 [EagleH][OP_BTS-2755]デバイスロック解除時にログアウトしない問題対応.
//            } else if( False == b_DeviceLockOn && True == onlyPanel.getBrowserMode() ) {
			} else if( ( False == b_DeviceLockOn )
// comment 2019/07/01 SCD2-Sky中野 [EagleH][OP_BTS-7467].
//					&& ( True == APIC_BrowserInfo::newInstance()->getLocalPSWCFlag() ) ) {
// comment 2020/07/14 Sky中野 [IT6_2.0MR3][OP_BTS-24936].
//  			 		&& ( ( True == APIC_BrowserInfo::newInstance()->getLocalPSWCFlag() ) || ( True == bl_IWSSetUpStatus ) ) ) {
			 		&& ( ( True == APIC_BrowserInfo::newInstance()->getLocalPSWCFlag() )
			 		  || ( True == bl_IWSSetUpStatus )
			 		  || ( True == c_SystemControl.getAdminModeOn() ) ) ) {
				// Timer Start.
                if ( True == PnlComGetOperationSetting()->getAutoResetSystem().b_IsOn ) {
                    PMessageProcCom::c_autoRestTimer.c_AutoResetAll.resetTimer( True );              // System Auto Reset Timer.
                } else {
                    PMessageProcCom::c_autoRestTimer.c_autoResetOffTimer.resetTimer( True );         // System Auto Reset Timer OFF時の1 min Timer.
                }
            }
        }
// comment 2015/10/07 OPS1-Sky浜野 [MinervaSBK][CQ519840] デバイスロック処理対応.
// Comment 2019/06/07 [HCL-Kavitha][IT6_1.0MR2][AR1077447] Updating setNotRunDeviceLock after remote processing end.
// Comment 2019/06/14 [HCL-Kavitha][IT6_1.0MR2][AR1078144] Updating setNotRunDeviceLock after device lock process.
// comment 2019/08/26 SCD2-4G Sky 二口 [EagleH][OP_BTS-11849] .	
//		pc_NtfEventInfo->setNotRunDeviceLock( False );
//		pc_NtfEventInfo->setNotRunDeviceLock( b_DeviceLockOn );
// comment 2019/07/01 SCD2-Sky中野 [EagleH][OP_BTS-7467].
		if( False == b_DeviceLockOn ) {
			bl_IWSSetUpStatus = False;
		}
	}

// comment 2019/08/26 SCD2-4G Sky 二口 [EagleH][OP_BTS-11849] .	
	pc_NtfEventInfo->setNotRunDeviceLock( False );
// comment 2020/05/13 Sky中野 [IT6_2.0MR2][OP_BTS-23419].
	if( False == b_DeviceLockOn ) {
		onlyPanel.setIWSSetUpStatus( False );
	}

// comment [特ROM対応][IT5-148]【SOLIMP3様】音声操作によるジョブ実行(IWS連携)要望_最終版リリース.
	if ( True == isSpecialGamenShowStatus_IWSRemoteJob() ) {
		b_result = True;
	}

	// 管理者モードエッジ検出.
	TYPE_AdminModeApp e_AdminModeApp = pc_NtfEventInfo->getAdminModeOnApp();	//【パフォーマンス改善】2012/03/21 第一OSW開発 酒井(健).
	TYPE_AdminLoginMode e_AdminLoginMode = ppc_SystemStatus->getAdminLoginMode();

	// comment 2019/08/22 Chikuan [EagleH][OP_BTS-10864] WebUI遷移時の不正画面遷移の修正.
	if( e_PreAdminModeApp != e_AdminModeApp ){
		e_AdminModeApp = ppc_SystemStatus->getAdminModeOnApp();
	}

	if( TYPE_AdminModeAppPanel != e_AdminModeApp
	    && TYPE_AdminModeAppExternal10 != e_AdminModeApp
	    && TYPE_AdminModeAppPSWC != e_AdminModeApp
	    && TYPE_AdminLoginModePSWC != e_AdminLoginMode
	    && e_PreAdminModeApp != e_AdminModeApp ) {
		// パネル以外の他アプリ管理者モード使用中.
		b_result = True;
		e_PreAdminModeApp = e_AdminModeApp;
		if( TYPE_AdminModeAppNon == e_AdminModeApp ) {
			onlyPanel.bl_isGetUsrInfoEndOfSpeGamen = True;
		}
	}
// comment 2014/12/16 OPS1-Sky浜野 [Posedion][CQ505311] 3秒ポーリンング処理中は管理者ログイン禁止対応.
	Bool bl_AdminLoginForbidden_forPanel = ppc_SystemStatus->getAdminLoginForbidden_forPanel();
	if ( b_PreAdminLoginForbidden_forPanel != bl_AdminLoginForbidden_forPanel ) {
		b_result = True;
		b_PreAdminLoginForbidden_forPanel = bl_AdminLoginForbidden_forPanel;
		if( False == bl_AdminLoginForbidden_forPanel ) {
			onlyPanel.bl_isGetUsrInfoEndOfSpeGamen = True;
		}
	}
	return b_result;
}
/*================================================================================================
［機能］ データの変化エッジチェック : 初期立ち上げ終了
================================================================================================*/
Bool PBasicMessageProc::isComplatePWOnStart()
{
	static Bool bl_PreDataCheckENG	= True;										// 前回のエンジン立ち上げ状態
	static Bool bl_PreDataCheckIRC	= True;										// 前回のＩＲ立ち上げ状態
	static TYPE_HddStatus e_PreHddStatus = TYPE_HddStatusNon;					// 前回のHDD状態.
	static TYPE_HddLockStatus e_PreHddLockStatus = TYPE_HddLockStatus_Initial;	// 前回のHDDロック状態.
// comment 2016/03/23 OPS1-Sky蒲原 [ZeusS_ZX0][CQ526323] HDD暗号化不一致時の状態チェック.
	static Bool bl_PreDataCheckHDDEncKeyNG = False;	// HDD暗号化不一致時の状態.
	register Bool bl_result = False;											// チェック結果
	bl_result = ( bl_PreDataCheckENG != ppc_PrinterStatus->isInitializing()							// エンジン・ＮｏｔＲｅａｄｙ状態ではない
	              || bl_PreDataCheckIRC != ppc_ScannerStatus->isInitializing() )  ? True : False ;	// ＩＲＣ・ＮｏｔＲｅａｄｙ状態ではない
	bl_PreDataCheckENG = ppc_PrinterStatus->isInitializing();					// 前回のエンジン立ち上げ状態に格納
	bl_PreDataCheckIRC	= ppc_ScannerStatus->isInitializing();					// 前回のＩＲ立ち上げ状態に格納
	// HDDステータスをチェック.
	APIC_HddInfo* pc_HddInfo = APIC_HddInfo::newInstance();
	if( e_PreHddStatus != pc_HddInfo->getHddStatus() ) {
		bl_result = True;
		e_PreHddStatus = pc_HddInfo->getHddStatus();
	}
	// HDDロックステータスがロック状態(解除不可)になれば.
	if( e_PreHddLockStatus != pc_HddInfo->getLockStatus()
	    && TYPE_HddLockStatus_Locked == pc_HddInfo->getLockStatus() ) {
		bl_result = True;
		e_PreHddLockStatus = pc_HddInfo->getLockStatus();
	}
// comment 2016/03/23 OPS1-Sky蒲原 [ZeusS_ZX0][CQ526323] HDD暗号化不一致時の状態チェック.
	if ( bl_PreDataCheckHDDEncKeyNG != ppc_SystemStatus->isEncryptionKeyNG() ) {
		// 起動時、セキュリティ警告が検知されている状態でかつ、ジョブログOnにしていると、rebuildGamenPrecheckにてHDDの.
		// ステータスチェックで画面遷移しない.かつ暗号化解除でHDD暗号化不一致の場合はmountに失敗するため.
		// HDDの状態が変化せず、画面遷移のタイミングを失うため、ここで暗号化不一致のエッジ検出を行い画面遷移させる.
		bl_PreDataCheckHDDEncKeyNG = !bl_PreDataCheckHDDEncKeyNG;
		bl_result = True;
	}
	return bl_result;
}
/*================================================================================================
［機能］ データの変化エッジチェック : トラブルの発生／解除
================================================================================================*/
Bool PBasicMessageProc::isFatalExist()
{
	static Bool bl_PreDataCheck	= False;	// 前回の状態
	register Bool DataCheck = False;		// 今回の状態
	register Bool bl_result = False;		// チェック結果
	// comPanelJobのFATALの有無を変数に今回の状態に移す
	if( onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCall ) == True
	    || onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCallSMART ) == True ) {
// comment 2022/07/15 FPT_LinhNV46 [IT6_2.2MR3][OP_BTS-38522] ラインモード画面が維持され、全画面警告に遷移しない。
// comment 2022/10/18 FPT_LinhNV46 [IT6_2.2MR3][OP_BTS-40053] ラインモード中にすべての全画面警告が表示されていない。.
//		if( APIC_SystemInfo::newInstance()->isCardLineMode() == False ){
//			DataCheck = True;
//		}
		DataCheck = True;
	} else {
		DataCheck = False;
	}
	bl_result = ( bl_PreDataCheck != DataCheck ) ? True : False ;	// 前回の状態と今回の状態の変化をチェック
	bl_PreDataCheck = DataCheck;				// 前回の状態に格納
	return bl_result;
}
/*================================================================================================
［機能］ データの変化エッジチェック : 全画面警告(ERROR､CAUTION)発生／解除
================================================================================================*/
Bool PBasicMessageProc::isGamenWarningExist()
{
	static ushort PreDataCheck				= 0;		// 前回の全画面警告の状態
	static ushort PreDataCheckService		= 0;		// 前回のサービス発生警告の状態
	static ushort PreDataCheckJobList		= 0;		// 前回のジョブリスト表示警告の状態
	static ushort PreChkElseWarningNum		= 0;		// 前回の警告状態変化毎に表示切り換えする警告の状態
	static ushort PreDataCheckDisplayJob	= 0;		// 前回のcc_DisplayJobで発生している全画面警告の状態
	static ushort PreDisableModeSet			= 0;		// 前回のモード設定を禁止する画面の状態
	static Bool PreEgEmergencySend			= False;	// 前回の「警告表示」の状態
	register ushort DataCheck 				= 0;		// 今回の全画面警告の状態
	register ushort DataCheckService 		= 0;		// 今回のサービス発生警告状態
	register ushort DataCheckJobList		= 0;		// 今回のジョブリスト表示警告状態
	register ushort ChkElseWarningNum 		= 0;		// 今回の警告状態変化毎に表示切り換えする警告状態
	register ushort DataCheckDisplayJob		= 0;		// 今回のcc_DisplayJobで発生している全画面警告の状態
	register ushort DisableModeSet			= 0;		// 今回のモード設定を禁止する画面の状態
	register Bool EgEmergencySend			= False;	// 今回の「警告表示」の状態
	register Bool bl_result = False;					// チェック結果
//Comment 2019/06/05 sky-tobita WebUI遷移中は警告検知しない.
	if( PNLE_WebUIScreenChangeStatusDuring == PNLC_WebUIControl::getInstance()->getWebUIScreenChangeStatus() ) {
		return False;
	}
	//  割り込み中の場合、左ジョブリストで選択中のジョブをアクティブジョブリストから検索する
	APIC_Job* pc_selectJob = NULL;	// 割り込み中の左ジョブリストで選択中のジョブ
	if( onlyPanel.ec_IntMode == EC_IntMode_On ) {
		LIBC_List<APIC_Job>  c_Joblist;	// ジョブのリスト
		short ss_AllJobNum   = 0;		// アクティブジョブリストの総数
		short ss_JobNum      = 0;		// リストの個数
		// アクティブジョブの総数取得.
//Comment 2017/11/22【HCLT-Sudheer】【IT5 4.2】【RQ686】 API Correction.
//		ss_AllJobNum = cp_ActiveJobList->getList(
//		                   TYPE_ActiveJobListAll,	// アクティブジョブリスト種類
//		                   1,						// 開始位置（１～Ｎ）
//		                   1,						// 個数
//		                   &c_Joblist );       	// リスト
//		// 全アクティブジョブを取得.
//		ss_JobNum = cp_ActiveJobList->getList(
//		                TYPE_ActiveJobListAll,	// アクティブジョブリスト種類
//		                1,						// 開始位置（１～Ｎ）
//		                ss_AllJobNum,			// 個数
//		                &c_Joblist );       	// リスト
		ss_AllJobNum = cp_ActiveJobList->getList(
		                   TYPE_ActiveJobListAll,	// アクティブジョブリスト種類
		                   1,						// 開始位置（１～Ｎ）
		                   1,						// 個数
		                   &c_Joblist,
		                   PNLC_NetworkLan::getInstance()->getLine());       	// リスト
		// 全アクティブジョブを取得.
		ss_JobNum = cp_ActiveJobList->getList(
		                TYPE_ActiveJobListAll,	// アクティブジョブリスト種類
		                1,						// 開始位置（１～Ｎ）
		                ss_AllJobNum,			// 個数
		                &c_Joblist,
		                PNLC_NetworkLan::getInstance()->getLine());       	// リスト
		// 全アクティブジョブに対してプリント用紙不適合の発生チェック
		short ss_listCnt = 0;
		for( ss_listCnt = 0; ss_listCnt < ss_JobNum; ss_listCnt++ ) {
			// リストからジョブを取得
			if( ss_listCnt == 0 ) {
				pc_selectJob = c_Joblist.findFirst();  // 先頭データを取得
			} else {
				pc_selectJob = c_Joblist.findNext();   // 次データを取得
			}
			if( pc_selectJob != NULL ) {
				if( pc_selectJob->getJobSpec()->b_select == False ) {
					// 左ジョブリストで選択されていないため、次のジョブをチェック
					pc_selectJob = NULL;
				} else {
					// 左ジョブリストで選択されているジョブを発見！
					break;
				}
			}
		}
	}
	if( onlyPanel.bl_isEnlargeDisplayMode == True ) {	// 拡大表示モード時.
		DataCheck 		  = onlyPanel.isAllGamenWarningEnlarge( comPanelJob );				// comPanelJobの全画面警告
		DataCheckService  = onlyPanel.isWarningatService( comPanelJob );					// サービスの全画面警告
		ChkElseWarningNum = onlyPanel.isElseWarningatWarningNumEnlarge( comPanelJob );		// 警告状態変化毎に表示切り換えする警告発生数チェック
		DataCheckJobList  = onlyPanel.isWarningatJoblistEnlarge( onlyPanel.cc_DisplayJob );	// ジョブリストから表示している全画面警告
		DataCheckDisplayJob = onlyPanel.isAllGamenWarningEnlarge( onlyPanel.cc_DisplayJob );	// onlyPanel.cc_DisplayJobの全画面警告
	} else {
		DataCheck 		 = onlyPanel.isAllGamenWarning( comPanelJob );						// comPanelJobの全画面警告
		DataCheckService = onlyPanel.isWarningatService( comPanelJob );						// サービスの全画面警告
		ChkElseWarningNum = onlyPanel.isElseWarningatWarningNum( comPanelJob );				// 警告状態変化毎に表示切り換えする警告発生数チェック
		DataCheckJobList = onlyPanel.isWarningatJoblist( onlyPanel.cc_DisplayJob );			// ジョブリストから表示している全画面警告
		DataCheckDisplayJob = onlyPanel.isAllGamenWarning( onlyPanel.cc_DisplayJob );			// onlyPanel.cc_DisplayJobの全画面警告
	}
	// モード設定を禁止する画面の状態変化チェックを追加
	DisableModeSet = onlyPanel.isDisableModeSet( True );									// モード設定を禁止する画面
	// ベンダー装着時管理者番号未入力画面対応.
	if( isKeyCounterChangeUnset() == True ) {				// ベンダー装着時キーカウンタが未セット状態に変化したとき
		onlyPanel.bl_VendorAdminInputComplete = False;		// ベンダー装着時管理者番号未入力状態にする
	}
	// 警告バックアップテーブルをコピー
	struct PDPS_EmergencySendTable s_TmpTable[PDPD_EmergencySendTableNum];	// 警告バックアップテーブル(仮)
	ushort i;
	ushort j;
	for( i = 0; i < PDPD_EmergencySendTableNum; i++ ) {
		// 仮テーブルにコピー
		s_TmpTable[i].pc_Job = onlyPanel.s_EmergencySendTable[i].pc_Job;
		s_TmpTable[i].ec_WarningName = onlyPanel.s_EmergencySendTable[i].ec_WarningName;
		// 本テーブルをクリア
		onlyPanel.s_EmergencySendTable[i].pc_Job = NULL;
		onlyPanel.s_EmergencySendTable[i].ec_WarningName = TYPE_WarningName_Non;
	}
	// 警告が発生しているかをチェックし、警告バックアップテーブルを再作成
	ushort us_TblCnt = 0;
	for( i = 0; i < PDPD_EmergencySendTableNum; i++ ) {
		// バックアップテーブルに警告がバックアップされている場合
		if( s_TmpTable[i].ec_WarningName != TYPE_WarningName_Non ) {
			// ジョブ警告かをチェック
			ushort j = 0;
			Bool b_isJobWarning = False;
			for( j=0 ; j < ( ushort )( us_warningJ2PSize/sizeof( PCGS_warningJ2P ) ) ; j++ ) {
				if( s_TmpTable[i].ec_WarningName == s_warningJ2P[j].e_Warningname ) {
					b_isJobWarning = True;
					break;
				}
			}
			if( b_isJobWarning ) {
				// ジョブ警告の場合
				if( APIC_JobInstanceList::isExistJob( s_TmpTable[i].pc_Job ) == True ) {
					// ジョブが有効か？
					if( onlyPanel.isWarningOn( s_TmpTable[i].pc_Job, s_TmpTable[i].ec_WarningName ) == True ) {
						// まだ発生している場合は、テーブルにバックアップ
						onlyPanel.s_EmergencySendTable[us_TblCnt].pc_Job = s_TmpTable[i].pc_Job;
						onlyPanel.s_EmergencySendTable[us_TblCnt].ec_WarningName = s_TmpTable[i].ec_WarningName;
						us_TblCnt++;
					}
				}
			} else {
				// ジョブ警告でない(≒デバイス警告)場合
				if( onlyPanel.isWarningOn( s_TmpTable[i].pc_Job, s_TmpTable[i].ec_WarningName ) == True ) {
					// まだ発生している場合は、テーブルにバックアップ
					onlyPanel.s_EmergencySendTable[us_TblCnt].pc_Job = s_TmpTable[i].pc_Job;
					onlyPanel.s_EmergencySendTable[us_TblCnt].ec_WarningName = s_TmpTable[i].ec_WarningName;
					us_TblCnt++;
				}
			}
		}
	}
	// 発生している警告を設定
	Bool bl_changewarning = False;
	ushort us_isNowWarning = DataCheck;
	if( PreDataCheck != DataCheck )	{ bl_changewarning = True; }
	if( us_isNowWarning == 0 ) {
		us_isNowWarning = DataCheckDisplayJob;
		if( PreDataCheckDisplayJob != DataCheckDisplayJob )	{ bl_changewarning = True; }
	}
	if( us_isNowWarning == 0 ) {
		us_isNowWarning = DataCheckService;
		if( PreDataCheckService != DataCheckService )	{ bl_changewarning = True; }
	}
	if( us_isNowWarning == 0 ) {
		us_isNowWarning = DataCheckJobList;
		if( PreDataCheckJobList != DataCheckJobList )	{ bl_changewarning = True; }
	}
	if( us_isNowWarning == 0 ) {
		us_isNowWarning = ChkElseWarningNum;
		if( PreChkElseWarningNum != ChkElseWarningNum )	{ bl_changewarning = True; }
	}
	if( ( True == bl_changewarning ) && ( sizeof( sp_GamenWarningTable )/sizeof( sp_GamenWarningTable[0] ) > us_isNowWarning ) ) {
		//警告状態に変化あり、ログを残す.
		if( 0 == us_isNowWarning ) {
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->resetWarningTable( sp_GamenWarningTable[us_isNowWarning].ec_WarningName );
			if( sizeof( sp_GamenWarningTable )/sizeof( sp_GamenWarningTable[0] ) > PreDataCheck ){
				PNL_LogManager::newInstance()->resetWarningTable( sp_GamenWarningTable[PreDataCheck].ec_WarningName );
			} else {
				PNL_LogManager::newInstance()->resetWarningTable( sp_GamenWarningTable[us_isNowWarning].ec_WarningName );
			}
		}
		else {
			PNL_LogManager::newInstance()->setWarningTable( sp_GamenWarningTable[us_isNowWarning].ec_WarningName );
// comment 2022/01/28 Sky東島 [IT6_2.2MR2][制御34]ユーザーセットアップ.
			if( NULL != sp_GamenWarningTable[us_isNowWarning].p_WarningPopup ){
				// 画面表示する警告が発生した場合は.
				// ユーザーセットアップ遷移不要を設定する.
				onlyPanel.setUserSetupUnnecessary();
			}
		}
	}
	// バックアップしているジョブの警告と表示中ジョブの警告を確認
	Bool b_EmergencySend = False;
	if( us_TblCnt == 0 ) {
		// 警告が発生していない場合は、「警告表示」フラグをOff
		onlyPanel.bl_isEmergencySend = False;
	} else {
		//  警告が発生している場合は、「警告表示」フラグをOn
		onlyPanel.bl_isEmergencySend = True;
		// 警告バックアップテーブルをチェック
		for( i = 0; ( ( PDPD_EmergencySendTableNum > i ) && ( sizeof( sp_GamenWarningTable )/sizeof( sp_GamenWarningTable[0] ) > us_isNowWarning ) ); i++ ) {
			// 発生中の警告が警告バックアップテーブルの警告と一致するかをチェック
			if( onlyPanel.checkExistWarningBackupTable( onlyPanel.s_EmergencySendTable[i].ec_WarningName, sp_GamenWarningTable[us_isNowWarning].ec_WarningName ) == True ) {
				// ジョブ警告かをチェック
				Bool b_isJobWarning = False;
				for( j=0 ; j < ( ushort )( us_warningJ2PSize/sizeof( PCGS_warningJ2P ) ) ; j++ ) {
					if( sp_GamenWarningTable[us_isNowWarning].ec_WarningName == s_warningJ2P[j].e_Warningname ) {
						b_isJobWarning = True;	// ジョブ警告
						break;
					}
				}
				if( b_isJobWarning ) {	// ジョブ警告の場合
					if( onlyPanel.s_EmergencySendTable[i].pc_Job == comPanelJob ) {
						// 同一ジョブで発生している場合は、警告を発生させない
						b_EmergencySend = True;
						break;
					}
				} else {	// デバイス警告の場合
					// 警告を発生させない
					b_EmergencySend = True;
					break;
				}
			}
		}
	}
	PNLE_WarningDetectSts chgsts = 0;
	EgEmergencySend = onlyPanel.bl_isEmergencySend;
	if( ( b_EmergencySend )  && ( PreDataCheck == DataCheck ) && ( PreDataCheckDisplayJob == DataCheckDisplayJob ) && ( PreDataCheckService == DataCheckService ) && ( PreDisableModeSet == DisableModeSet ) ) {
		// 警告を発生させない場合
		bl_result = False;
	} else if( ppc_SystemStatus->getServiceModeOn() == True ) {	// サービスモードの場合、サービス用警告をチェック
// comment 2022/07/15 FPT_LinhNV46 [IT6_2.2MR3][OP_BTS-38522] ラインモード画面が維持され、全画面警告に遷移しない。
// comment 2022/10/18 FPT_LinhNV46 [IT6_2.2MR3][OP_BTS-40053] ラインモード中にすべての全画面警告が表示されていない。.
		if( APIC_SystemInfo::newInstance()->isCardLineMode() == True
			&& TYPD_TroubleCode_D073 == PnlComGetSystemStatus()->getFatal().us_Code ){
			bl_result = False;
		// 全画面警告を表示していない時は、警告発生時のみ変化エッジをとるようにする。（全画面警告画面でない時に全画面解除で画面遷移させる必要がない為）
		} else if( PreDataCheckService != DataCheckService && ( ( onlyPanel.bl_WarningChangeGamen == False && PreDataCheckService == 0 ) || onlyPanel.bl_WarningChangeGamen == True ) ) {
			bl_result = True;
			chgsts = ( DataCheckService ) ? PNLE_WarningDetectSts_AllGamenShow : PNLE_WarningDetectSts_AllGamenhide;
		} else {
			if( PreChkElseWarningNum != 0 && PreChkElseWarningNum != ChkElseWarningNum ) {
				// 警告状態変化毎に表示切り換えする警告状態で、警告状態変化毎に表示切り換えする警告数に変化があったときは、画面遷移を行う
				bl_result = True;
				chgsts = ( ChkElseWarningNum ) ? PNLE_WarningDetectSts_AllGamenShow : PNLE_WarningDetectSts_AllGamenhide;
			} else {
				bl_result = False;
			}
		}
	} else {
		if( ( PreDataCheck != DataCheck && ( ( onlyPanel.bl_WarningChangeGamen == False && PreDataCheck == 0 ) || onlyPanel.bl_WarningChangeGamen == True ) )
		  ) {
			bl_result = True;
			chgsts = ( DataCheck ) ? PNLE_WarningDetectSts_AllGamenShow : PNLE_WarningDetectSts_AllGamenhide;
			if( onlyPanel.bl_isEnlargeDisplayMode == True && PreDataCheck == 0 ) {
				onlyPanel.bl_isEnlargeWarningMessage = ( sp_GamenEnlWarningTable[ DataCheck ].bl_FirstGamenEnlarge == True ) ? True : False;
			}
		} else if( ( PreDataCheckJobList != DataCheckJobList )	// 警告状態に変化あり
		           &&	( ( onlyPanel.bl_WarningChangeGamen == True )													// 警告表示中
		                 || ( ( onlyPanel.bl_WarningChangeGamen == False ) && ( PreDataCheckJobList == 0 ) )						// 警告表示中でない 警告未発生→発生
		                 || ( ( onlyPanel.bl_WarningChangeGamen == False ) && ( onlyPanel.bl_isEmergencySend == True ) ) ) ) {	// 警告表示中でない 警告次ジョブ予約中
			// OFF→ON、割り込み中で、cc_DisplayJobが左ジョブリスト選択中ジョブでない場合、全画面警告を検出(表示)しない
			APIC_User* pc_CurrentUser = PnlComGetUser();
			uchar WarningNotForCurrentUser = 0;
			if( TYPE_UserAuthenticationOff != APIC_UserAuthentication::newInstance()->getUserAuthentication(PNLC_NetworkLan::getInstance()->getLine()) ) {	//[ZuesMLK_362522][CQ 352063,CQ 358701, 358009_Reopen][User check done only when authentication is ON] [HCL-PRAVEENKUMAR] 23/02/2012 Add.
				if( pc_CurrentUser != NULL ) {
					if( pc_CurrentUser->getUserNo() != onlyPanel.cc_DisplayJob->getJobSpec()->pc_Common->c_UserAcount.getUserNo() ) {
						if( onlyPanel.isWarningOn( comPrintJob, PERROR_ShortCharge ) ) {		//CQ:362522【ZeusMLK】【OapV4.0マージ】【363027】【Display of warning screen】【HCL_Hasmi】 2012/05/09 Add.
							WarningNotForCurrentUser = 1;
						}																	//CQ:362522【ZeusMLK】【OapV4.0マージ】【363027】【Display of warning screen】【HCL_Hasmi】 2012/05/09 Add.
					}
				}
			}																										//[ZuesMLK_362522][CQ 352063,CQ 358701, 358009_Reopen][User check done only when authentication is ON] [HCL-PRAVEENKUMAR] 23/02/2012 Add.
			if( ( ( 0 == PreDataCheckJobList ) && ( EC_IntMode_On == onlyPanel.ec_IntMode ) && ( pc_selectJob != onlyPanel.cc_DisplayJob ) ) || ( 1 == WarningNotForCurrentUser ) ) {

				bl_result = False;
			} else {
				bl_result = True;
				chgsts = ( DataCheckJobList ) ? PNLE_WarningDetectSts_AllGamenShow : PNLE_WarningDetectSts_AllGamenhide;
				if( onlyPanel.bl_isEnlargeDisplayMode == True && PreDataCheckJobList == 0 ) {
					// 拡大表示モード時、前回が警告なしの時（最初の警告発生）は、テーブルのフラグによって、最初に表示する画面（拡大ﾒｯｾｰｼﾞ画面 or 通常ﾒｯｾｰｼﾞ画面）を決定する.
					onlyPanel.bl_isEnlargeWarningMessage = ( sp_GamenEnlWarningTable[ DataCheckJobList ].bl_FirstGamenEnlarge == True ) ? True : False;
				}
			}
		} else if( ( PreDataCheckDisplayJob != DataCheckDisplayJob )	// 警告状態に変化あり
		           &&	( ( onlyPanel.bl_WarningChangeGamen == True )													// 警告表示中
		                 || ( ( onlyPanel.bl_WarningChangeGamen == False ) && ( PreDataCheckDisplayJob == 0 ) )					// 警告表示中でない 警告未発生→発生
		                 || ( ( onlyPanel.bl_WarningChangeGamen == False ) && ( onlyPanel.bl_isEmergencySend == True ) ) ) ) {	// 警告表示中でない 警告次ジョブ予約中
			// OFF→ON、割り込み中で、cc_DisplayJobが左ジョブリスト選択中ジョブでない場合、全画面警告を検出(表示)しない
			if( ( PreDataCheckDisplayJob == 0 ) && ( onlyPanel.ec_IntMode == EC_IntMode_On ) && ( pc_selectJob != onlyPanel.cc_DisplayJob ) ) {
				bl_result = False;
			} else {
// comment 2020/01/10 HCL_Monisha [SparrowBK][OP_BTS-17108] Blocked rebuildGamen call when memory overflow warning occurs.
//				if( ( onlyPanel.bl_WarningChangeGamen == False ) && ( onlyPanel.bl_isEmergencySend == True ) && ( PNL_OTHER_USBMemoryDetection == sp_GamenWarningTable[ PreDataCheckDisplayJob ].ec_WarningName ) ) {	// 警告表示中でない且つ、警告次ジョブ予約中且つ、外部メモリー検出でない場合.
				if( ( onlyPanel.bl_WarningChangeGamen == False ) 
					&& ( onlyPanel.bl_isEmergencySend == True ) 
					&& ( ( PNL_OTHER_USBMemoryDetection == sp_GamenWarningTable[ PreDataCheckDisplayJob ].ec_WarningName ) 
						|| ( PNL_ERROR_ThumbnailImageDispError == sp_GamenWarningTable[ PreDataCheckDisplayJob ].ec_WarningName ) 
						|| ( PNL_ERROR_PreviewImageDispError == sp_GamenWarningTable[ PreDataCheckDisplayJob ].ec_WarningName ) ) ) {	// 警告表示中でない且つ、警告次ジョブ予約中且つ、外部メモリー検出でない場合.
					bl_result = False;
				} else {
					bl_result = True;
					chgsts = ( DataCheckDisplayJob ) ? PNLE_WarningDetectSts_AllGamenShow : PNLE_WarningDetectSts_AllGamenhide;
				}
				if( onlyPanel.bl_isEnlargeDisplayMode == True && PreDataCheckDisplayJob == 0 ) {
					onlyPanel.bl_isEnlargeWarningMessage = ( sp_GamenEnlWarningTable[ PreDataCheckDisplayJob ].bl_FirstGamenEnlarge == True ) ? True : False;
				}
			}
		} else if( ( PreEgEmergencySend != EgEmergencySend ) && ( EgEmergencySend == False ) ) {	// 「警告表示」の状態ではなくなった
			bl_result = True;
			chgsts = PNLE_WarningDetectSts_AllGamenhide;
		} else {
			//comment 2022/09/14 FPT-KhaiPQ [IT6_2.3] 機器構成変更_No.47.
			if( ( PreDataCheckDisplayJob == DataCheckDisplayJob ) 
				&& ( ( ( onlyPanel.bl_isEnlargeDisplayMode == True )
						&& ( ( ( PERROR_StapleEmpty_S == sp_GamenEnlWarningTable[ PreDataCheckDisplayJob ].ec_WarningName ) 
								&& PNLC_PopupListControl::newInstance()->isNowPopup( &gp_EnlWarn_MesStapleEmpty_P_Detail ) )
							|| ( ( PERROR_PunchScrapsFullUser_S == sp_GamenEnlWarningTable[ PreDataCheckDisplayJob ].ec_WarningName ) 
								&& PNLC_PopupListControl::newInstance()->isNowPopup( &gp_EnlWarn_PunchScrapsFullUser_P_Detail ) ) ) ) 
					|| ( ( onlyPanel.bl_isEnlargeDisplayMode == False )
						&& ( ( ( PERROR_StapleEmpty_S == sp_GamenWarningTable[ PreDataCheckDisplayJob ].ec_WarningName ) 
								&& PNLC_PopupListControl::newInstance()->isNowPopup( &gp_StapleEmpty_warn_Detail ) )
							|| ( ( PERROR_PunchScrapsFullUser_S == sp_GamenWarningTable[ PreDataCheckDisplayJob ].ec_WarningName ) 
								&& PNLC_PopupListControl::newInstance()->isNowPopup( &gp_PunchScrapsFullUser_Detail ) ) ) ) ) ) {
				bl_result = False; 
			} else if( PreChkElseWarningNum != 0 && PreChkElseWarningNum != ChkElseWarningNum ) {
			//if( PreChkElseWarningNum != 0 && PreChkElseWarningNum != ChkElseWarningNum ) {
				// 警告状態変化毎に表示切り換えする警告状態で、警告状態変化毎に表示切り換えする警告数に変化があったときは、画面遷移を行う
				bl_result = True;
				chgsts = ( ChkElseWarningNum ) ? PNLE_WarningDetectSts_AllGamenShow : PNLE_WarningDetectSts_AllGamenhide;
			} else if( ( PreDisableModeSet != DisableModeSet )								// 警告状態に変化あり.
			           && ( PNL_OTHER_CheckControllerPowerSupply == sp_GamenDisableModeSetTable[PreDisableModeSet].ec_WarningName ) ) {	// 前回の状態がコントローラ電源確認警告だった.
				bl_result = True;	// 警告状態が解除された場合もエッジ検出させる.
				chgsts = ( DisableModeSet ) ? PNLE_WarningDetectSts_AllGamenShow : PNLE_WarningDetectSts_AllGamenhide;
			} else if( PreDisableModeSet != DisableModeSet
			           && ( PNL_OTHER_VendorAdminInput == sp_GamenDisableModeSetTable[DisableModeSet].ec_WarningName
			                || PNL_OTHER_CheckControllerPowerSupply == sp_GamenDisableModeSetTable[DisableModeSet].ec_WarningName )
			         ) {
				bl_result = True;
				chgsts = ( DisableModeSet ) ? PNLE_WarningDetectSts_AllGamenShow : PNLE_WarningDetectSts_AllGamenhide;
			} else if( (onlyPanel.bl_RAPanelTouchNotify == True )
				&& (onlyPanel.bl_isEnlargeDisplayMode == True)){
					if( PNL_ATTENTION_TabletMainPanelTouchNotice == sp_GamenEnlWarningTable[DataCheck].ec_WarningName ){
						bl_result = True;
						chgsts = ( DataCheck ) ? PNLE_WarningDetectSts_AllGamenShow : PNLE_WarningDetectSts_AllGamenhide;
						onlyPanel.bl_isEnlargeWarningMessage = ( sp_GamenEnlWarningTable[ DataCheck ].bl_FirstGamenEnlarge == True ) ? True : False;
						onlyPanel.bl_RAPanelTouchNotify = False;
					}
			} else {
				bl_result = False;
			}
		}
	}
	// 表示状態を意識せず毎回全状態を更新する（詳細は制御仕様書参照）
	PreDataCheck		= DataCheck;
	PreDataCheckService	= DataCheckService;
	PreDataCheckJobList	= DataCheckJobList;
	PreChkElseWarningNum = ChkElseWarningNum;
	PreDataCheckDisplayJob = DataCheckDisplayJob;
	PreDisableModeSet = DisableModeSet;
	PreEgEmergencySend = EgEmergencySend;	// 「警告表示」表示状態
	// 全画面警告の状態変化を設定する.
	if( True == bl_result ) {
		// サービスで警告検知時に警告画面に遷移しなかった時に、次のrebuildで画面遷移できなくなる問題の対応.
		//警告禁止画面だった場合は警告を検出(表示)しない.
		if( NULL != nowGamen && nowGamen->isNoWarningGamen ) {
			//何もしない.
		} else {
			PNLC_WarningManager::newInstance()->setWarningDetectSts( True,chgsts );
		}
	}
	return bl_result;
}
/*================================================================================================
［機能］ データの変化エッジチェック : 画面遷移付きメッセージ警告発生
================================================================================================*/
Bool PBasicMessageProc::isMessageWarningWithGamenExist( Bool* _bl_isRemoveDocOnGlass )
{
	static ushort PreDataCheck	= 0;	// 前回の警告の状態
	register ushort DataCheck = 0;		// 今回の警告の状態
	register Bool bl_result = False;	// チェック結果
	*_bl_isRemoveDocOnGlass = False;	// 「原稿取り忘れ警告」発生ﾌﾗｸﾞ
//Comment 2019/06/05 sky-tobita WebUI遷移中は警告検知しない.
	if( PNLE_WebUIScreenChangeStatusDuring == PNLC_WebUIControl::getInstance()->getWebUIScreenChangeStatus() ) {
		return False;
	}
	if( PJobListControl::b_SingleStepHideLeftArea ==  True ) {
		// ジョブリストでは[PnlComGetJobCopy()]で取得する.

		// 強制的にジョブリストのジョブスペックを取得
		DataCheck = onlyPanel.isMessageGamen( PnlComGetJobCopy( 1 ), ( onlyPanel.bl_isEnlargeDisplayMode == True ) ? True : False );	// comPanelJobの画面移動メッセージ警告の有無を変数に今回の状態に移す

	} else {
		DataCheck = onlyPanel.isMessageGamen( comPanelJob, ( onlyPanel.bl_isEnlargeDisplayMode == True ) ? True : False );	// comPanelJobの画面移動メッセージ警告の有無を変数に今回の状態に移す
	}

	// サービス以外で、キー押下直後の警告発生の時は、画面遷移しないようにする(OHP合紙を選択したときにOHPフィルムの給紙口がエンプティだと用紙画面に遷移してしまうため)
	if( onlyPanel.bl_isEnlargeDisplayMode == True ) {
		bl_result = ( DataCheck != 0 && PreDataCheck != DataCheck && ppc_SystemStatus->getServiceModeOn() == False && c_SystemControl.getAdminModeOn() == False ) ? True : False ;	// 前回の状態と今回の状態の変化をチェック
		
// comment 2015/11/04 MSE田中優介 [ZeusS_ZX0][CQ520656] Popup以外のメッセージ警告の表示タイミングを変更.
		PBasicMessageProc*  p_warningGamen = sp_MessageEnlWarningTable[ DataCheck ].WarningGamen;
		if ( NULL != p_warningGamen ) {
	 		PPopupCom* pc_NewPopupInstance = dynamic_cast< PPopupCom* >(p_warningGamen);
	 		if ( NULL != pc_NewPopupInstance ) {
		 		// メッセージ警告の遷移先画面がPopupの場合は、警告発生タイミングを変更したため警告未発生とする.
	 			bl_result = False;
	 		}
		}

		if( bl_result == True
		    && ( sp_MessageEnlWarningTable[ DataCheck ].ec_WarningName == PCAUTION_RemoveDocOnGlass || sp_MessageEnlWarningTable[ DataCheck ].ec_WarningName == PCAUTION_PaperEmpty_S ) ) {
			*_bl_isRemoveDocOnGlass = True;
		}
	} else {
		bl_result = ( DataCheck != 0 && PreDataCheck != DataCheck && ppc_SystemStatus->getServiceModeOn() == False && c_SystemControl.getAdminModeOn() == False && onlyPanel.bl_DisableMessageWarningWithGamen == False ) ? True : False ;	// 前回の状態と今回の状態の変化をチェック
		if( ( ScanOapExtSet.isActiveGeneralApplication() == True ) ) {
			if( DataCheck < ( ushort )sizeof( sp_MessageWarningTable )/sizeof( sp_MessageWarningTable[0] ) ) {
				if( ( ( NULL != ScanOapExtSet.getApplication() ) && ( ( ScanOapExtSet.getApplication()->s_Version.e_Type == TYPE_ExtTypeScanS )
				        && ( ( TYPE_WarningName )sp_MessageWarningTable[ DataCheck ].ec_WarningName == PNL_ERROR_DocSizeDetectErrorFax_S ) ) )
				    || ( ( ScanOapExtSet.getApplication()->s_Version.e_Type == TYPE_ExtTypeScanD )
				         && ( ( TYPE_WarningName )sp_MessageWarningTable[ DataCheck ].ec_WarningName == PNL_ERROR_DocSizeDetectErrorFax_S ) )
				    || ( ( ScanOapExtSet.getApplication()->s_Version.e_Type == TYPE_ExtTypeBrowserScan )
				         && ( ( TYPE_WarningName )sp_MessageWarningTable[ DataCheck ].ec_WarningName == PNL_ERROR_DocSizeDetectErrorFax_S ) )
				    // 使用中アプリケーション種別がブラウザ統合アプリケーションの場合かつ.
				    // ブラウザ統合アプリケーションのScan連携ジョブの利用が可能な場合かつ.
				    // ファクス原稿サイズ検出エラー(設定時)発生時.
				    || ( ( ScanOapExtSet.getApplication()->s_Version.e_Type == TYPE_ExtTypeBrowserGeneral )
				         && ( NULL != ScanOapExtSet.getAplDetail() )
				         && ( NULL != ScanOapExtSet.getAplDetail()->getBrowserGeneral() )
				         && ( TYPE_ExtFunctionLimitEnable == ScanOapExtSet.getAplDetail()->getBrowserGeneral()->e_ScanJobEnable )
				         && ( ( TYPE_WarningName )sp_MessageWarningTable[ DataCheck ].ec_WarningName == PNL_ERROR_DocSizeDetectErrorFax_S ) ) ) {
					// スキャン連携アプリの原稿サイズ検出エラーの時だけ画面遷移する。
				} else {
					bl_result = False;
				}
			}
		}
	}
	PreDataCheck = DataCheck;								// 前回の状態に格納
	onlyPanel.bl_DisableMessageWarningWithGamen = False;	// 画面遷移メッセージ警告抑制フラグをリセットする
	// メッセージ警告の状態変化を設定する.
	if( bl_result ) {
		PNLC_WarningManager::newInstance()->setWarningDetectSts( True,PNLE_WarningDetectSts_MessageShow );
	}
	return bl_result;
}
/*================================================================================================
［機能］ データの変化エッジチェック : ブザーを鳴らす警告発生
================================================================================================*/
Bool PBasicMessageProc::isBuzzerWarningExist()
{
	static Bool b_PrePaperEmpty = False;	// 前回の紙切れの状態
	register Bool b_PaperEmpty = False;		// 今回の紙切れの状態
	register Bool b_result = False;			// チェック結果
	// 紙切れチェック
	if( comPrintJob != NULL ) {
		if(	onlyPanel.isWarningOn( comPrintJob, PCAUTION_PaperEmpty_S ) ) {		// 設定時
			// 紙切れ発生
			b_PaperEmpty = True;
		}
	}
	// 警告発生のエッジをとる
	if(	( b_PaperEmpty && !b_PrePaperEmpty ) ) {					// 紙切れが発生
		b_result = True;
	}
	// 前回の状態に格納
	b_PrePaperEmpty = b_PaperEmpty;
	return b_result;
}
//================================================================================================
//［機能］ データの変化エッジチェック : 弱注意音①を鳴らす警告発生
//================================================================================================
Bool PBasicMessageProc::isBuzzerWarning1Exist()
{
	static Bool b_PreOutOfMoney = False;	// 前回のベンダ金額不足の状態
	register Bool b_OutOfMoney = False;		// 今回のベンダ金額不足の状態
	register Bool b_result = False;			// チェック結果
	// ベンダ金額不足チェック
	if( comPanelJob != NULL ) {
		if( onlyPanel.isWarningOn( comPanelJob, PERROR_OutOfMoney1 ) ) {	// ベンダ金額不足１（設定時以外）
			// ベンダ金額不足発生
			b_OutOfMoney = True;
		}
	}
	// 警告発生のエッジをとる
	if(	( b_OutOfMoney && !b_PreOutOfMoney ) ) {	// ベンダ金額不足が発生
		b_result = True;
	}
	// 前回の状態に格納
	b_PreOutOfMoney = b_OutOfMoney;
	return b_result;
}
//================================================================================================.
//［機能］ 	データの変化エッジチェック : 組合せ禁止解除.
//			組合せ禁止警告の発生は検知しない.
//================================================================================================.
Bool PBasicMessageProc::isEnlCombinationExist()
{
	static   short ss_PreComb = 0;	// 前回の警告の状態.
	register short ss_NowComb = 0;	// 今回の警告の状態.
	register Bool bl_result   = False;	// チェック結果.
	PNLC_NotifyEventInfo* pc_NtfEventInfo = PNLC_NotifyEventInfo::newInstance();		// データの管理クラス.
	if( NULL == pc_NtfEventInfo ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	if( onlyPanel.bl_isEnlargeDisplayMode == True && pc_NtfEventInfo->getServiceModeOn() == False ) {
		ss_NowComb = onlyPanel.isMessageGamen( comPanelJob );	// comPanelJobの画面移動メッセージ警告の有無を変数に今回の状態に移す.
// comment 2015/11/04 MSE田中優介 [ZeusS_ZX0][CQ520656] 拡大表示画面のメッセージ警告の表示タイミングを通常表示画面にあわせるため削除.
//		if( ss_PreComb != ss_NowComb
//		    && ( ( ss_PreComb == 0 && sp_MessageEnlWarningTable[ ss_NowComb ].bl_isCombination == True )	// 警告無し→組合せ禁止有りに変化.
//		         || ( ss_PreComb != 0 && sp_MessageEnlWarningTable[ ss_PreComb ].bl_isCombination == True && sp_MessageEnlWarningTable[ ss_NowComb ].bl_isCombination == False ) ) ) {	// 組合せ禁止警告→警告無しに変化.
		// 組合せ禁止警告の解除を検知する.
		// 組合せ禁止警告発生時にPopup表示する処理は通常表示画面にあわせてsetupGamenの警告チェック時に行うように変更.
		// 組合せ禁止警告の解除のみ、本検知状態のタイミングで行う.
		if ( ( ss_PreComb != ss_NowComb )												// 警告状態が前回と異なる場合.
		  && ( 0 != ss_PreComb )														// 前回警告状態が発生中の場合.
		  && ( True == sp_MessageEnlWarningTable[ ss_PreComb ].bl_isCombination )		// 前回警告が組合せ禁止警告の場合.
		  && ( False == sp_MessageEnlWarningTable[ ss_NowComb ].bl_isCombination ) ) {	// 今回警告状態が組合せ禁止警告以外の場合.
			bl_result = True;
// Comment KronosS -> 2017/05/10 SCD2_3G永田 AR1019108対応.
#ifdef DEF_KRONOSS_Cus
			// 2行メッセージ警告テーブル(拡大)を検索, 低電力移行不可警告画面ポインタの取得.
			for( register ushort i = 1; i < us_MessageEnlWarningTableSize/sizeof( SP_MessageEnlWarningTable ); i++ ) {
				if( sp_MessageEnlWarningTable[ i ].ec_WarningName == PNL_ERROR_LowPowerModeError ){
					PBasicMessageProc* p_warningGamen = sp_MessageEnlWarningTable[ i ].WarningGamen;
					if ( NULL != p_warningGamen ) {
						PPopupCom* pc_NewPopupInstance = dynamic_cast< PPopupCom* >( p_warningGamen );
						PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();
						if ( ( NULL != pc_NewPopupInstance ) && ( NULL != pc_PopupListControl ) ){
							if( ( sp_MessageEnlWarningTable[ ss_PreComb ].ec_WarningName == PNL_ERROR_LowPowerModeError )
							 && ( False == pc_PopupListControl->isNowPopup( pc_NewPopupInstance ) )){
						   		// 前回警告状態が低電力移行不可警告であり, かつ現在ポップアップしていない場合は組み合わせ禁止解除(closeTop)しない.
						   		// 本対応により, 低電力移行不可警告を裏でresetPanelWarningで閉じたときのcloseTopによる異常画面遷移を回避する.
						   		// PNLC_EnlWarningManager::detectEnlMessageWarningPopupおよびvoid PLayerControl::msgBoxErrorWarningDispで上記対応.
						   		bl_result = False;
							}
						}
					}
					break;	// 低電力移行不可警告画面を見つけたら抜ける.
				}
			}
#endif // DEF_KRONOSS_Cus.
// Comment KronosS <- 2017/05/10 SCD2_3G永田 AR1019108対応.
		}
	}
	ss_PreComb = ss_NowComb;		// 前回の状態に格納.
	return bl_result;
}
//================================================================================================.
//［機能］	データ変化イベント処理 : 組合せ禁止解除.
//			組合せ禁止警告の解除時にPopupをクローズする処理を行う.
//================================================================================================.
void PBasicMessageProc::procEnlCombinationExist( uchar option )
{
	// 拡大表示モード時のみのイベントなので、rebuildGamenEnlarge()のみを呼ぶ.
// comment 2015/02/10 OPS1-Sky浜野 [Poseidon][CQ509765] 警告画面からの戻り先不正対応.
//// comment 2014/12/10 【HCLT-Deeparaj】 【CQ-507699】When combination warning is not present block call for rebuildGamenEnlarge .
//	if(onlyPanel.ep_Combination != CNK_Size){
//		rebuildGamenEnlarge( 1 );		// 状態に応じた画面表示（前画面あり）
//	}
// comment 2015/11/04 MSE田中優介 [ZeusS_ZX0][CQ520656] 拡大表示画面のメッセージ警告の表示タイミングを通常表示画面にあわせるため組み合わせ禁止警告条件を削除.
//	rebuildGamenEnlarge( 1 );		// 状態に応じた画面表示（前画面あり）.
	// 組合せ禁止警告の解除時にメッセージ警告のクローズ処理を行う.
	PNLC_EnlWarningManager::newInstance()->closeEnlMessageWarningPopup( comPanelJob );
// comment 2019/05/17 SCD2-Sky小畑 [EagleH][OP_BTS-5313] 表示中ポップアップが組み合わせ禁止警告の対象であればクローズする対応.
//	PNLC_PopupListControl::newInstance()->closeTop( False );
	for( register ushort i = 1; i < us_MessageEnlWarningTableSize/sizeof( SP_MessageEnlWarningTable ); i++ ) {
		PBasicMessageProc* p_warningGamen = sp_MessageEnlWarningTable[ i ].WarningGamen;
		if( ( True == sp_MessageEnlWarningTable[ i ].bl_isCombination )
		 && ( NULL != p_warningGamen ) ) {
			PPopupCom* pc_NewPopupInstance = dynamic_cast< PPopupCom* >( p_warningGamen );
			if( True == PNLC_PopupListControl::newInstance()->isNowPopup( pc_NewPopupInstance ) ) {
				PNLC_PopupListControl::newInstance()->closeTop( False );
				break;
			}
		}
	}

}
/*================================================================================================
［機能］ データの変化エッジチェック : ＡＰＳによる給紙口指定がチェック
================================================================================================*/
Bool PBasicMessageProc::isFeedTrayEnable()
{
	static Bool b_PreFeedCodeEnable = False;	// 前回の状態
	register Bool b_result = False;				// 結果
	if( ( False == b_PreFeedCodeEnable ) && ( True == comPanelJob->getJobSpec()->pc_Print->c_PaperTray.getFeedTrayEnable() ) ) {	// 給紙口指定が確定した時
		b_result = True;	// 変化あり
	}
	b_PreFeedCodeEnable = comPanelJob->getJobSpec()->pc_Print->c_PaperTray.getFeedTrayEnable();	// 今回の値を前回の値とする
	return b_result;
}
/*================================================================================================
［機能］ データの変化エッジチェック : ベンダー装着時キーカウンタ未セット状態チェック
================================================================================================*/
Bool PBasicMessageProc::isKeyCounterChangeUnset()
{
	register Bool bl_result = False;			// 結果
	// ベンダ装着されて、キーカウンタが引き抜かれた場合..
	TYPE_KeyMode e_KeyModeMachineSetting = pc_MachineSetting->getKeyMode();
	TYPE_KeyMode e_KeyModeKeyDevice = APIC_KeyDevice::getKeyMode();
	if( e_KeyModeMachineSetting == TYPE_KeyModeVendor || e_KeyModeMachineSetting == TYPE_KeyModeVendorKM ) {
		if( e_KeyModeMachineSetting == e_KeyModeKeyDevice ) {
			bl_result = True;
		}
	}
	return bl_result;
}
/*================================================================================================
［機能］ データの変化エッジチェック : ファクスボードからのコマンドレスポンスの変化
================================================================================================*/
Bool PBasicMessageProc::isFaxCmdRsChange( TYPE_CommLine e_CommLine, FAXS_CmdRs* ps_CmdRs )
{
	APIC_FaxDevice* pc_FaxDevice;		// FaxDeviceインスタンスアドレス
	Bool b_isReturn;
	Bool b_result;
	pc_FaxDevice = APIC_FaxDevice::newInstance();
	if( pc_FaxDevice == NULL ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		// コマンドレスポンスなし
		return ( False );
	}
	FAXE_CmdRs e_isFaxLineCmdReq;
	switch( e_CommLine ) {
		case TYPE_CommLine1: e_isFaxLineCmdReq = e_isFaxLine1CmdReq;	break;
		case TYPE_CommLine2: e_isFaxLineCmdReq = e_isFaxLine2CmdReq;	break;
		case TYPE_CommLine3: e_isFaxLineCmdReq = e_isFaxLine3CmdReq;	break;
		case TYPE_CommLine4: e_isFaxLineCmdReq = e_isFaxLine4CmdReq;	break;
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return ( False );
	}
	// 現在、回線に対してコマンド要求中かどうかを判定.
	if( ( FAXE_CmdRsKtd != e_isFaxLineCmdReq )		// ダイアルコマンド要求中でない.
	    && ( FAXE_CmdRsFks != e_isFaxLineCmdReq )		// 通信強制終了コマンド要求中でない.
	    && ( FAXE_CmdRsKtm != e_isFaxLineCmdReq )		// テストモードに入るコマンド要求中でない.
	    && ( FAXE_CmdRsKtm2 != e_isFaxLineCmdReq )		// テストモードから抜けるコマンド要求中でない.
	    && ( FAXE_CmdRsKtn != e_isFaxLineCmdReq )		// テストNo.選択コマンド要求中でない.
	    && ( FAXE_CmdRsKss != e_isFaxLineCmdReq ) ) {	// スピーカーOn/Off設定コマンド要求中でない.
		// コマンド要求中でないため、レスポンスなしとみなす.
		return ( False );
	}
	// コマンドレスポンスがあるかどうかを取得する.
	ps_CmdRs->e_CmdRsID = e_isFaxLineCmdReq;
	b_isReturn = pc_FaxDevice->getCmdRs( ps_CmdRs, e_CommLine );
	if( False == b_isReturn ) {
		// コマンドレスポンスなし.
		b_result = False;
	} else {
		// コマンドレスポンスあり.
		b_result = True;
	}
	return ( b_result );
}
/*================================================================================================
［機能］ データの変化エッジチェック : ファクス回線1の回線状態の変化
================================================================================================*/
Bool PBasicMessageProc::isFaxLine1LineStatusChange()
{
	APIC_FaxDevice* pc_FaxDevice;
	Bool b_IsOffHook;					// 現在のファクス回線1の回線状態
	static Bool b_PreIsOffHook = False;	// 前回のファクス回線1の回線状態
	Bool b_result = False;
	pc_FaxDevice = APIC_FaxDevice::newInstance();
	if( pc_FaxDevice == NULL ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return ( False );
	}
	// 画像ログ転送設定が有効である場合、オフフック状態にしない.
	if( TYPE_ImageLogSendType2 == PnlComGetSystemSetting()->getImageLogSendEnabled() ) { return ( False ); }
	if( ( b_isOffHookBtnOn == True )								// 手動オフフック状態判定.
	    &&( ( pc_FaxDevice->getLineStatus( TYPE_CommLine1 ) == 0 )	// 回線１の回線状態検知(Idle判定).
	        ||( b_isFaxLine1OffHook == True ) ) ) {						// 外部電話オフフック状態判定.
		// 手動オフフック状態時に、回線がIdleまたは外付け電話オフフック状態になった場合.
		// 手動オフフック解除後、オンフック画面へ遷移する為、回線状態の前回値の辻褄を合わせる.
		b_PreIsOffHook = False;
		return ( True );
	}
	// 回線1に対する回線状態（外付け電話が上げられたかどうか）を取得する
	b_IsOffHook = pc_FaxDevice->isOffHook( TYPE_CommLine1 );
	// 画面遷移無しで、オフフック状態が変化したか確認
	if( ( nextGamen == NULL )
	    && ( b_IsOffHook != b_PreIsOffHook ) ) {
		// 前回値から変化あり
		b_result = True;
		b_PreIsOffHook = b_IsOffHook;		// 前回値を更新
	}
	return ( b_result );
}
/*================================================================================================
［機能］ データの変化エッジチェック : 指定ファクス回線のネーム・ナンバー取得可能状態の変化
================================================================================================*/
Bool PBasicMessageProc::isFaxNumberDispChange( TYPE_CommLine e_CommLine )
{
	APIC_FaxDevice* pc_FaxDevice;
	APIC_FaxPanelSetting* pc_FaxPanelSetting;
	FAXE_LineStatus e_FaxLineStatus = FAXE_LineStatusIdle;
	Bool	b_result = False;
	pc_FaxDevice = APIC_FaxDevice::newInstance();
	Bool bl_EnableNumberDisp = False;
	if( pc_FaxDevice == NULL ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return ( False );
	}
	pc_FaxPanelSetting = PnlComGetFaxPanelSetting();
	// Fax回線状況の取得
	e_FaxLineStatus = pc_FaxDevice->getFaxStatus( e_CommLine );
	switch( e_CommLine ) {
		case TYPE_CommLine1:		// 回線1
// 機能拡張2以前と以降で参照するAPIが異なる為、それぞれのAPIでナンバーディスプレイ設定を確認.
			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {		// ZeusBK/Zeus2.1（機能拡張2）以降.
				if( TYPE_RcvFunc_AddrNameDisplayOn == PnlComGetFaxGfpSetting()->getRcvFunc_AddrNameDisplayAll( e_CommLine ) ) {
					bl_EnableNumberDisp = True;
				}
			} else {
				if( TYPE_NumDisp_Number == pc_FaxPanelSetting->getNumDisp_DisplayKind()
				    || TYPE_NumDisp_Name == pc_FaxPanelSetting->getNumDisp_DisplayKind() ) {
					bl_EnableNumberDisp = True;
				}
			}
			if( True == bl_EnableNumberDisp ) {
				if( e_FaxLineStatus == FAXE_LineStatusKca ) {
					// ネームもしくはナンバーディスプレイ表示内容取得可能状態
					if( b_DispDataCatch1 == False ) {
						// ネームもしくはナンバーディスプレイ表示内容未取得の為、回線状態変化指定
						b_result = True;
					}
				} else {
					// ネームもしくはナンバーディスプレイ表示内容取得不可状態
					if( b_DispDataCatch1 == True ) {
						// ネームもしくはナンバーディスプレイ表示内容取得済みの為、回線状態変化指定
						b_result = True;
					}
				}
			}
			break;
		case TYPE_CommLine2:		// 回線2.
			if( False == PLBC_FuncVersionCheck::isOK_MiZe_Type2()		// ZeusBK/Zeus2.1（機能拡張2）以前.
			    && ( TYPE_NumDisp_Number_Multi == pc_FaxPanelSetting->getNumDisp_DisplayKind_Multi()
			         || TYPE_NumDisp_Name_Multi ==  pc_FaxPanelSetting->getNumDisp_DisplayKind_Multi() ) ) {
				bl_EnableNumberDisp = True;
			}
			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2()		// ZeusBK/Zeus2.1（機能拡張2）以降.
			    && TYPE_C2_RcvFunc_AddrNameDisplayOn == PnlComGetFaxGfpSetting()->getRcvFunc_AddrNameDisplayAll( e_CommLine ) ) {
				bl_EnableNumberDisp = True;
			}
			if( True == bl_EnableNumberDisp ) {
				// ネームもしくはナンバーディスプレイ表示内容取得可能状態.
				if( e_FaxLineStatus == FAXE_LineStatusKca ) {
					// ネームもしくはナンバーディスプレイ表示内容取得可能状態.
					if( b_DispDataCatch2 == False ) {
						// ネームもしくはナンバーディスプレイ表示内容未取得の為、回線状態変化指定.
						b_result = True;
					}
				} else {
					// ネームもしくはナンバーディスプレイ表示内容取得不可状態.
					if( b_DispDataCatch2 == True ) {
						// ネームもしくはナンバーディスプレイ表示内容取得済みの為、回線状態変化指定.
						b_result = True;
					}
				}
			}
			break;
		case TYPE_CommLine3:		// 回線3.
			if( TYPE_RcvFunc_AddrNameDisplayOn == PnlComGetFaxGfpSetting()->getRcvFunc_AddrNameDisplayAll( e_CommLine ) ) {	// ナンバー表示設定.
				// ナンバーディスプレイ表示内容取得可能状態.
				if( FAXE_LineStatusKca == e_FaxLineStatus ) {
					// ナンバーディスプレイ表示内容取得可能状態.
					if( False == bl_DispDataCatch3 ) {
						// ナンバーディスプレイ表示内容未取得の為、回線状態変化指定.
						b_result = True;
					}
				} else {
					// ナンバーディスプレイ表示内容取得不可状態.
					if( True == bl_DispDataCatch3 ) {
						// ナンバーディスプレイ表示内容取得済みの為、回線状態変化指定.
						b_result = True;
					}
				}
			}
			break;
		case TYPE_CommLine4:		// 回線4.
			if( TYPE_RcvFunc_AddrNameDisplayOn == PnlComGetFaxGfpSetting()->getRcvFunc_AddrNameDisplayAll( e_CommLine ) ) {	// ナンバー表示設定.
				// ナンバーディスプレイ表示内容取得可能状態.
				if( FAXE_LineStatusKca == e_FaxLineStatus ) {
					// ナンバーディスプレイ表示内容取得可能状態.
					if( False == bl_DispDataCatch4 ) {
						// ナンバーディスプレイ表示内容未取得の為、回線状態変化指定.
						b_result = True;
					}
				} else {
					// ナンバーディスプレイ表示内容取得不可状態.
					if( True == bl_DispDataCatch4 ) {
						// ナンバーディスプレイ表示内容取得済みの為、回線状態変化指定.
						b_result = True;
					}
				}
			}
			break;
		default:
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			// コマンドレスポンスなし
			b_result = False;
			break;
	}
	return ( b_result );
}
/*================================================================================================
［機能］ データの変化エッジチェック : 外部メモリー複数検知警告
================================================================================================*/
Bool PBasicMessageProc::isUSBMemoryTwoInsert()
{
	Bool b_isUSBMemoryTwoInsert = False;
	static Bool b_PreIsUSBMemoryTwoInsert = False;	// 前回の外部メモリー複数検知状態
	Bool b_result = False;
	APIC_UsbMemDeviceInfo* pc_USBMemoryInfo = APIC_UsbMemDeviceInfo::newInstance();
	PNLC_NotifyEventInfo* pc_NtfEventInfo = PNLC_NotifyEventInfo::newInstance();		// データの管理クラス.
	if( NULL == pc_NtfEventInfo ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	if( True == pc_NtfEventInfo->getUsbMemWarningTwoInsert() ) {
		b_isUSBMemoryTwoInsert = True;
	}
	if( b_isUSBMemoryTwoInsert != b_PreIsUSBMemoryTwoInsert ) {
		if( False == pc_USBMemoryInfo->isAccess( TYPE_UsbMemAccessWrite, TYPE_UsbMemAppIdNetTxProcess )
		    && False == pc_USBMemoryInfo->isAccess( TYPE_UsbMemAccessRead, TYPE_UsbMemAppIdStorageAccessApp )
		    && False == comPanelJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn() ) {
			b_result = True;
		}
	}
	b_PreIsUSBMemoryTwoInsert = b_isUSBMemoryTwoInsert;
	return b_result;
}
/*================================================================================================
［機能］ データの変化エッジチェック : 外部メモリー挿入検出
================================================================================================*/
Bool PBasicMessageProc::isUSBDetection()
{
// comment 2021/02/05 Sky東池 [IT5-140(機種マージ)] IWS画面使用中、USBを挿入したときのポップアップを非表示にする対応.
	APIC_SoftDipSw* pc_APIC_SoftDipSw = APIC_SoftDipSw::newInstance();
	if( NULL == pc_APIC_SoftDipSw ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, 3, "Assert Failed!" );
		return False;
	}
	
	// ラインカードモードの場合は外部メモリー挿入検出を行わない.
	// ラインカードがUSBとなり、外部メモリー挿入検出画面を表示すると工場調整メニューが表示されないため.
	PNLC_NotifyEventInfo* pc_NtfEventInfo = PNLC_NotifyEventInfo::newInstance();		// データの管理クラス.
	if( NULL == pc_NtfEventInfo ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	if( pc_NtfEventInfo->getCardLineMode() ) {		//ラインカードがささっている場合.
		return( False );	// 常にFalse(未検出).
	}

	// comment 2021/12/14 Sky大峰 [OP_BTS-34959] USB検知画面.
	Bool b_isUSBDetection = False;
	static Bool b_PreIsUSBDetection = False;	// 前回の外部メモリー挿入状態
	APIC_UsbMemDeviceInfo* pc_USBMemoryInfo = APIC_UsbMemDeviceInfo::newInstance();

	if( TYPD_UsbMemStatusConnect == pc_USBMemoryInfo->getStatus() ) {
		b_isUSBDetection = True;
	}

// comment 2020/09/04 Sky東島 [IT6_2.0MR3][OP_BTS-26713].
	if( ( &gp_ActionInitialSetup == nowGamen )
	 && ( 0 == PNLC_PopupListControl::newInstance()->getPopupNum() ) ){
		// 起動中画面の場合は検出させない.
		// comment 2021/12/14 Sky大峰 [OP_BTS-34959] USB検知画面.
		// USB検出中の場合は、処理させない。
		if( True == b_isUSBDetection ) {
			return False;
		}
	}

	Bool b_result = False;
	Bool b_BoxOperation = True;
	Bool b_UsbEnable = True;
	Bool b_EnablePrint = True;
	Bool b_JoblistOpen=False;
	Bool b_EnableScan = True;
	Bool b_ReadEnable = True;	// 認証OFF時はユーザー機能制限がないため、デフォルトTrueとする.
	APIC_User* p_LoginUser = PnlComGetUser() ;

	if( ( True == PnlComGetUserAccountOn() ) && ( NULL != p_LoginUser ) ) {
		// 蓄積文書操作.
		if( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_BoxOperation ) ) {
			b_BoxOperation = False;
		} else {
			b_BoxOperation = True;
		}

		// 外部メモリー保存.
		if( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_ScanToUsb ) ) {
			b_UsbEnable = False;
		} else {
			b_UsbEnable = True;
		}

		// プリンター印字.
		if( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Print ) ) {
			b_EnablePrint = False;
		} else {
			b_EnablePrint = True;
		}

		// 外部メモリー読込み.
		if( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_UsbMemoryReading ) ) {
			b_ReadEnable = False;
		} else {
			b_ReadEnable = True;
		}

		// スキャン操作.
		if( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_Scan ) ) {
			b_EnableScan = False;
		} else {
			b_EnableScan = True;
		}
	}
	if( True == PNLC_PopupListControl::newInstance()->isPopupOpen() ) {
		PBasicMessageProc* nowGamen = ( PBasicMessageProc* )PNLC_PopupListControl::newInstance()->getNowPopup();
		b_JoblistOpen = nowGamen->isNoWarningGamen;
	}
	else {
		b_JoblistOpen = PBasicMessageProc::getNowGamen()->isNoWarningGamen;
	}

	APIC_UsbMemSetting*	pc_UsbMemSetting = APIC_UsbMemSetting::newInstance();

	if( TYPE_KeyModeVendorKM == pc_NtfEventInfo->getKeyMode() ) {	b_ReadEnable = False;	}
	if( TYPE_KeyModeVendorKM == APIC_KeyDevice::getKeyMode()			// 統合ベンダー状態.
	    && TYPE_AppAtAdminMachineVendorEnableWithCardOrCoin == PnlComGetSystemSetting()->getSendAtAdminMachineVendor()	// 送信を管理する.
	    && False == APIC_KeyDevice::isCoinExist() ) {					// コイン未投入.
		b_UsbEnable = False;
	}

// comment 2021/02/05 Sky東池 [IT5-140(機種マージ)] IWS画面使用中、USBを挿入したときのポップアップを非表示にする対応.
	// 「DipSWNo.209 bit6がON」かつ「IWS使用中」かつ「ブラウザ起動モードがブラウザランチャー」ならば、USBポップアップ表示を抑制する.
	// 「DipSWNo.209 bit7がON」かつ「IWS使用中」かつ「ブラウザ起動モードがブラウザランチャー」ならば、USB検出音を抑制する.
	Bool b_USBDecDispEnable		= True;
	Bool b_USBDecBuzzerEnable	= True;
	ushort us_CurrentAppIndex = ScanOapExtSet.getCurrentAppNo();
	APIC_ExtSetting* pc_AppInfo = APIC_ExtSetting::newInstance( us_CurrentAppIndex );
	if( ( NULL != pc_AppInfo )
		&& ( True == pc_AppInfo->isIwsApplication() )
		&& ( BRWE_BrowserBootMode_Launcher == onlyPanel.e_BrowserBootMode ) ) {
		if( True ==  pc_APIC_SoftDipSw->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW209, 6 ) ) {
			b_USBDecDispEnable = False;
		}
		if( True ==  pc_APIC_SoftDipSw->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW209, 7 ) ) {
			b_USBDecBuzzerEnable = False;
		}
	}
	

//Comment 2017/09/04 [HCL-Swathi] [IT5 4.2][RQ-686] [WP721]Multiple Network support.
	TYPE_NetworkSpecific e_NetworkSpecific  = PNLC_NetworkLan::getInstance()->getLine();
	if( b_isUSBDetection != b_PreIsUSBDetection ) {
		PNLC_WebUIDebug::getInstance()->outputDebugLog( "==== b_isUSBDetection != b_PreIsUSBDetection ====.\n" );

// comment [特ROM対応][IT5-148]【SOLIMP3様】音声操作によるジョブ実行(IWS連携)要望_最終版リリース(USB検知音変更).
		APIC_USBSetUnsetSound*       pt_USBSoundAPI = APIC_USBSetUnsetSound::newInstance();
		TYPE_USBSetUnSetSoundPlayRet e_SoundPlayRet = TYPE_USBSetUnSetSoundPlayRet_OtherError;
		PNLC_NotifyEventInfo* pc_NtfEventInfo       = PNLC_NotifyEventInfo::newInstance();		// データの管理クラス.

		if( b_isUSBDetection == True ) {
			PNLC_WebUIDebug::getInstance()->outputDebugLog( "==== b_isUSBDetection == True ====.\n" );

// comment [特ROM対応][IT5-148]【SOLIMP3様】音声操作によるジョブ実行(IWS連携)要望_最終版リリース(USB検知音変更).
			if( False == PnlComIsUsbSetSoundPlayFuncEnable() ){
// comment 2021/05/06 [IT6-69] Sky竹内 USBメモリー機能設定を削除したい.
				if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW244, 2 ) ) {
					setBuzzerType( PCGE_BuzzerType_InputNg );
				} else {
// comment 2021/02/05 Sky東池 [IT5-140(機種マージ)] IWS画面使用中、USBを挿入したときのポップアップを非表示にする対応.
					if( True == b_USBDecBuzzerEnable ) {
						setBuzzerType( PCGE_BuzzerType_InputOk );
					}
				}
			}else{
				if( pt_USBSoundAPI != NULL
				 && pt_USBSoundAPI->getUSBSetSoundEnable() == True )
				{
					e_SoundPlayRet = pt_USBSoundAPI->req_USBSetUnSetSoundPlay( TYPE_USBSetUnsetSoundType_Set );
				}

				if( e_SoundPlayRet != TYPE_USBSetUnSetSoundPlayRet_OK )
				{
					setBuzzerType( PCGE_BuzzerType_InputOk );
				}
			}

		} else if( onlyPanel.b_nowUsbMode == True ) {
			onlyPanel.b_nowUsbMode = False;
			turnGamen = NULL;
			turnGamenBackup = NULL;
			pc_PanelMain->UsbPullingOut( 0 );	// USBメモリー引き抜き通知.
		}

// comment [特ROM対応][IT5-148]【SOLIMP3様】音声操作によるジョブ実行(IWS連携)要望_最終版リリース(USB検知音変更).
		if( True == PnlComIsUsbSetSoundPlayFuncEnable() ){
			if( b_isUSBDetection == False ) {
				if( pt_USBSoundAPI != NULL
				 && pt_USBSoundAPI->getUSBUnsetSoundEnable() == True ){
					e_SoundPlayRet = pt_USBSoundAPI->req_USBSetUnSetSoundPlay( TYPE_USBSetUnsetSoundType_Unset );
				}
			}
		}

		// 外部メモリー検出画面を表示しない状態
		if( False == comPanelJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn()
		    && True == b_BoxOperation
//Comment 2017/09/04 [HCL-Swathi] [IT5 4.2][RQ-686] [WP721]Multiple Network support.
//		    && ( ( ( True == b_UsbEnable && True == b_EnableScan ) && ( True == pc_UsbMemSetting->getEnableScanToUsb() ) ) ||			// 文書保存=ON、スキャン操作=ON,ユーザー機能制限=許可.
//		         ( ( True == b_EnablePrint ) && ( True == pc_UsbMemSetting->getEnableUsbDirectPrint() ) ) ||								// 文書印刷=ON,ユーザー機能制限=許可.
//		         ( ( True == b_ReadEnable && True == b_EnablePrint ) && ( True == pc_UsbMemSetting->getEnableUsbMemoryReading() ) ) )		// 文書読込=ON、プリンター印字=ON,ユーザー機能制限=許可.
			&& ( ( ( True == b_UsbEnable && True == b_EnableScan ) && ( True == pc_UsbMemSetting->getEnableScanToUsb( e_NetworkSpecific ) ) ) ||			// 文書保存=ON、スキャン操作=ON,ユーザー機能制限=許可.
			     ( ( True == b_EnablePrint ) && ( True == pc_UsbMemSetting->getEnableUsbDirectPrint( e_NetworkSpecific ) ) ) ||								// 文書印刷=ON,ユーザー機能制限=許可.
			     ( ( True == b_ReadEnable && True == b_EnablePrint ) && ( True == pc_UsbMemSetting->getEnableUsbMemoryReading( e_NetworkSpecific ) ) ) )		// 文書読込=ON、プリンター印字=ON,ユーザー機能制限=許可.
		    && False == ppc_SystemStatus->getUserChoiceChanging()
		    && False == comPanelJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn()
		    && TYPE_InputStatusIdle == comPanelJob->getJobStatus()->getInputStatus()
		    && TYPE_OutputStatusIdle == comPanelJob->getJobStatus()->getOutputStatus()
		    && False == b_JoblistOpen
// comment 2021/02/05 Sky東池 [IT5-140(機種マージ)] IWS画面使用中、USBを挿入したときのポップアップを非表示にする対応.
		    && True == b_USBDecDispEnable ) {
			b_result = True;
		}

// comment [特ROM対応][IT5-148]【SOLIMP3様】音声操作によるジョブ実行(IWS連携)要望_最終版リリース(USB検知音変更).
		if( NULL != pc_NtfEventInfo && True == pc_NtfEventInfo->getIWSRemoteJobEntry() ){
			// IWSからのRemoteJob実行中は、外部メモリー検出画面を非表示とする.
			b_result = False;
		}

	}
	b_PreIsUSBDetection = b_isUSBDetection;
	return b_result;
}

// comment 2015/03/26 OPS1-Sky新谷 [特ROM][CQ392335]強化機能対応.
/*================================================================================================
［機能］ データの変化エッジチェック : ログ取得用のUSBの挿入検出. 
================================================================================================*/
Bool PBasicMessageProc::isLogUSBDetection()
{
	Bool bl_result = False;
	Bool bl_isLogUSBDetection = False;
	static Bool bl_PreIsLogUSBDetection = False;	// 前回の外部メモリー挿入状態.

	APIC_LogUsbMemDeviceInfo* pc_LogUSBMemoryInfo = APIC_LogUsbMemDeviceInfo::newInstance();
	if ( NULL == pc_LogUSBMemoryInfo ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return bl_result;
	}

	// ログ取得用のUSBの挿入状態を取得.
	if( TYPD_UsbMemStatusConnect == pc_LogUSBMemoryInfo->getStatus() ){
		bl_isLogUSBDetection = True;
	}

	// ログ取得用USBの状態が変わった.
	if( bl_isLogUSBDetection != bl_PreIsLogUSBDetection ){
		if( bl_isLogUSBDetection == True ) {
			bl_result = True;
		}
	}
	bl_PreIsLogUSBDetection = bl_isLogUSBDetection;
	return bl_result;
}

//================================================================================================
//［機能］ データの変化エッジチェック : 外部メモリー読み込み中のエラー検出
//================================================================================================
Bool PBasicMessageProc::isUSBReadTurnChange()
{
	Bool b_isUSBReadTurnChange = False;
	static Bool b_PreIsUSBReadTurnChange = False;	// 前回の外部メモリー読み込み中状態.
	static Bool b_DoubleCheckFlg = False;			// チェックが２回目であるかの確認フラグ.
	Bool b_result = False;
	PNLC_NotifyEventInfo* pc_NtfEventInfo = PNLC_NotifyEventInfo::newInstance();		// データの管理クラス.
	if( NULL == pc_NtfEventInfo ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	b_isUSBReadTurnChange = pc_NtfEventInfo->getUSBReadTurnChange();
	if( b_isUSBReadTurnChange != b_PreIsUSBReadTurnChange || b_DoubleCheckFlg == True ) {
		if( onlyPanel.bl_WarningChangeGamen == True || onlyPanel.b_nowUsbMode == False ) {
			b_result = True;
		}
		if( b_DoubleCheckFlg == True ) {
			b_DoubleCheckFlg = False;
		} else {
			b_DoubleCheckFlg = True;
		}
	}
	b_PreIsUSBReadTurnChange = b_isUSBReadTurnChange;
	return b_result;
}
//================================================================================================.
// [機能］ データの変化エッジチェック : 外部メモリーアクセスエラー .
//================================================================================================.
Bool PBasicMessageProc::isUSBMemoryAccessErr()
{
	Bool b_isUSBMemoryAccessErr = False;
	static Bool b_PreIsUSBMemoryAccessErr = False;	// 前回の外部メモリー複数検知状態.
	Bool b_result = False;
	PNLC_NotifyEventInfo* pc_NtfEventInfo = PNLC_NotifyEventInfo::newInstance();		// データの管理クラス.
	if( NULL == pc_NtfEventInfo ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return b_result;
	}
	if( True == pc_NtfEventInfo->getUsbMemWarningNeedPullOut() ) {	// USBメモリ引き抜き要求状態.
		b_isUSBMemoryAccessErr = True;
	}
	if( b_isUSBMemoryAccessErr != b_PreIsUSBMemoryAccessErr ) {
		b_result = True;
	}
	b_PreIsUSBMemoryAccessErr = b_isUSBMemoryAccessErr;
	return b_result;
}
// ==================================================================
//概要    :外部メモリーデバッグ情報の蓄積不可エラー.
//作成者  :表.
// ==================================================================
void PBasicMessageProc::setupUSBLogSystemStoreError()
{
	static Bool b_UsbMemWarning = False;
	static Bool b_LogCreateStatusExec = False;
	if( !APIC_LogUsbMemDeviceInfo::newInstance()->isWarning( TYPE_UsbMemWarningDebugLog ) ) {
		b_UsbMemWarning = False;
	}
//	if( !DBGE_LogCreateStatusExec == APIC_DebugLogSystem::newInstance()->getLogCreateSts() ) {
	if( !( DBGE_LogCreateStatusExec == APIC_DebugLogSystem::newInstance()->getLogCreateSts() ) ) {
		b_LogCreateStatusExec = False;
	}
	if( ( !b_UsbMemWarning || ( !b_LogCreateStatusExec && DBGE_LogCreateStatusExec == APIC_DebugLogSystem::newInstance()->getLogCreateSts() ) )
	    && APIC_LogUsbMemDeviceInfo::newInstance()->isWarning( TYPE_UsbMemWarningDebugLog ) ) {
		onlyPanel.setPanelWarning( PNL_ERROR_LogSystemStoreError );
		turnGamen = this;
		b_UsbMemWarning = True;
		b_LogCreateStatusExec = True;
	}
}
//================================================================================================
//［機能］  デバッグログ蓄積中の外部メモリー検出.
//================================================================================================
PNLE_LogCreateUSBMemory PBasicMessageProc::isLogCreateUSBMemoryExit()
{
	static uchar uc_TimeCounter = 0;
	PNLE_LogCreateUSBMemory s_Result = PNLE_LogCreateUSBMemory_Non;
	static DBGE_DebugLogSystemResult e_PreDebugLogSystemResult = DBGE_DebugLogSystemResultOK;
	// 状態変化判定.
	if( e_PreDebugLogSystemResult != m_e_DebugLogSystemResult ) {
		// ログ蓄積USBメモリー引抜状態判定.
		if( DBGE_DebugLogSystemResultUSBMemPullOut == m_e_DebugLogSystemResult ) {
			s_Result = PNLE_LogCreateUSBMemory_Exit;					//ログ蓄積USBメモリー引抜.
			uc_TimeCounter = 0;
		} else {
			if( 0 == APIC_LogUsbMemDeviceInfo::newInstance()->getWarning()
			    && onlyPanel.isWarningOn( comPanelJob, PNLD_ERROR_LogSystemUsbMemoryNon ) ) {
				s_Result = PNLE_LogCreateUSBMemory_Enter;				//ログ蓄積USBメモリー挿入.
			}
		}
		e_PreDebugLogSystemResult = m_e_DebugLogSystemResult;
	} else {
		if( DBGE_DebugLogSystemResultUSBMemPullOut == e_PreDebugLogSystemResult ) {
			// 結果コードがログ蓄積USBメモリー引抜のままの場合.
			// ログ蓄積USBメモリー装着を判定する.
			if( TYPD_UsbMemStatusConnect == APIC_LogUsbMemDeviceInfo::newInstance()->getStatus()
			    && 10 <= uc_TimeCounter ) {		// 警告発生後5秒程度はUSBメモリーの装着状態のチェックは行わない.
				// 結果コードを初期化.
				m_e_DebugLogSystemResult = DBGE_DebugLogSystemResultOK;
			} else {
				if( 10 > uc_TimeCounter ) {
					uc_TimeCounter++;
				}
			}
		}
	}
	return s_Result;
}
/*================================================================================================
［機能］ ファクスボードからのコマンドレスポンスダイアル完了
================================================================================================*/
void PBasicMessageProc::procFaxCmdRsKtd( uchar option, TYPE_CommLine e_CommLine, FAXS_CmdRs s_CmdRs )
{
	PDPC_FaxCommonApi*	pc_FaxCommonApi;
	Bool				b_ret_sendtenky;
	// 回線を判定する
	if( e_CommLine != TYPE_CommLine1 ) {
		// 回線1以外に対するコマンドレスポンスの場合は何もしない
		return;
	}
	// コマンドレスポンス完了結果を判定
	if( s_CmdRs.b_CmdResult == True ) {
		if( ( b_isOffHookBtnOn == False ) && ( b_isFaxLine1OffHook == False ) ) {
			// 手動オフフック及び外部電話オフフック状態で無い場合は、手動オフフック状態に遷移
			// 「オフフック」ボタン押下による手動送受信モード遷移フラグに「True」を設定
			b_isOffHookBtnOn = True;

			// comment 2018/05/18 SCD2-Sky東野 [IT6] [1044346] 不具合対応 Start.
			APIC_MultiAutoTest *pc_MultiAutoTest = APIC_MultiAutoTest::newInstance();
			if(NULL != pc_MultiAutoTest){
				pc_MultiAutoTest->set_OffHookMode();
			}
			// comment 2018/05/18 SCD2-Sky東野 [IT6] [1044346] 不具合対応 End.

// [KronosS] 管理No.269_3 スキャナセンサー変更 SCD2 3G Sky西場 2016/11/17 start.
// [CQ533028] オフフックの送信、受信選択 SCD2 3G Sky西場 2016/08/08 start.
// [Sparrow] 機器構成変更_No.304_スキャナセンサー変更 SCD2 4G Sky錦 2017/12/11 start.
#if defined(DEF_VenusMLK)
			PCGC_ScanFaxBaseCom::setUserSelectOffHookTxRxMode( PNLE_FaxRbtSelTxRxModeNonSel );
#endif
// [CQ533028] オフフックの送信、受信選択 SCD2 3G Sky西場 2016/08/08 end.
// [KronosS] 管理No.269_3 スキャナセンサー変更 SCD2 3G Sky西場 2016/11/17 end.
// [Sparrow] 機器構成変更_No.304_スキャナセンサー変更 SCD2 4G Sky錦 2017/12/11 end.
		} else if( ( b_isOffHookBtnOn == True ) || ( b_isFaxLine1OffHook == True ) ) {
			// 手動オフフック又は外部電話オフフック状態の場合はダイアル発呼のレスポンスの為、
			// バッファリングされている宛先が無いか確認し、存在すれば発呼処理を行う
			pc_FaxCommonApi = PDPC_FaxCommonApi::getInstance();
			if( pc_FaxCommonApi != NULL ) {
				// ファクスボードに対してダイアルコマンド要求中以外は送信コマンドを発行可能_
				if( e_isFaxLine1CmdReq != FAXE_CmdRsKtd ) {
					// ダイアル送信実行
					b_ret_sendtenky = pc_FaxCommonApi->sendTenkeyDial();
					if( b_ret_sendtenky == True ) {
						// ダイアル送信を行った場合、ファクス回線1のコマンド要求中状態フラグを
						//「ダイアルコマンド要求中」にする
						e_isFaxLine1CmdReq = FAXE_CmdRsKtd;
					}
				}
			}
		} else {
			// このルートはありえない
		}
	} else {
		// コマンドレスポンスがエラーであった場合オフフック状態フラグをFalseに変更
		b_isOffHookBtnOn = False;
	}
}
/*================================================================================================
［機能］ ファクスボードからのコマンドレスポンステストモードに入る完了.
［備考］ 本クラスでファクステストの状態変更を行うため関数内の処理を追加（CQ358615）.
================================================================================================*/
void PBasicMessageProc::procFaxTestInto( uchar option, TYPE_CommLine e_FaxLineNo, FAXS_CmdRs s_CmdRs )
{
	// 正常終了かつ選択した回線と通知の回線が等しい場合.
	if( ( True == s_CmdRs.b_CmdResult ) && ( e_FaxLineNo == c_FaxTestStatus.getFaxTestCommLine() ) ) {
		c_FaxTestStatus.setFaxTestStatus( PNLD_FaxTestStatusOnStop );	// ファクステスト状態を停止に設定.
	}
}
/*================================================================================================
［機能］ ファクスボードからのコマンドレスポンステストモードから抜ける完了.
［備考］ 本クラスでファクステストの状態変更を行うため関数内の処理を追加（CQ358615）.
================================================================================================*/
void PBasicMessageProc::procFaxCmdRsKtm2( uchar option, TYPE_CommLine e_FaxLineNo, FAXS_CmdRs s_CmdRs )
{
	// 正常終了かつ選択した回線と通知の回線が等しい場合.
	if( ( True == s_CmdRs.b_CmdResult ) && ( e_FaxLineNo == c_FaxTestStatus.getFaxTestCommLine() ) ) {
		c_FaxTestStatus.setFaxTestStatus( PNLD_FaxTestStatusOff );	// ファクステスト状態を終了に設定.
	}
}
/*================================================================================================
［機能］ ファクスボードからのコマンドレスポンステストNo.選択完了.
［備考］ 本クラスでファクステストの状態変更を行うため関数内の処理を追加（CQ358615）.
================================================================================================*/
void PBasicMessageProc::procFaxCmdRsKtn( uchar option, TYPE_CommLine e_FaxLineNo, FAXS_CmdRs s_CmdRs )
{
	// 正常終了かつ選択した回線と通知の回線が等しい場合.
	if( ( True == s_CmdRs.b_CmdResult ) && ( e_FaxLineNo == c_FaxTestStatus.getFaxTestCommLine() ) ) {
		c_FaxTestStatus.setFaxTestStatus( PNLD_FaxTestStatusOnStop );	// ファクステスト状態を停止に設定.
	}
}
//================================================================================================
//概要    :画面ブザー禁止.
//備考    :[No.34159]設定が変わっていない、有効でないキーを押下時、受付音を鳴らさないための処理.
//         各画面で受付音を鳴らさない時にコールする.
//================================================================================================
// パーツ音声IDの取得
Bool PBasicMessageProc::getPartsVoiceID( short ssPosition, PNLE_VoicePartsKind ucVpartsKind, ushort* pusVoiceIDBuf, short* pssVoiceIDBufSize )
{
	// 音声非対応画面または音声対応パーツがない場合
	if( !m_pstVoiceGamenInfo || !m_pstVoiceGamenInfo->pstPartsList ) { return False; }
	// 範囲チェック
	if( ( ssPosition < 0 ) ||
	    ( ssPosition >= m_pstVoiceGamenInfo->ssPartsListCount ) ) { return False; }
	// 引数異常のチェック
	if( !pssVoiceIDBufSize ) { return False; }
	// パーツを特定
	PNLS_VoiceCursorParts* pstVcp =
	    &m_pstVoiceGamenInfo->pstPartsList[ssPosition];
	short ssSizeNext = pstVcp->pssVoiceIDCount[ucVpartsKind];
	short ssSizePrev = *pssVoiceIDBufSize;
	const ushort* pusVID = pstVcp->ppusVoiceID[ucVpartsKind];
	// 異常チェック
	*pssVoiceIDBufSize = ssSizeNext;
	if( !pusVoiceIDBuf ) { return False; }
	if( ssSizePrev < ssSizeNext ) {
		*pssVoiceIDBufSize = -1;
		return False;
	}
	// バッファへコピー
	memcpy( pusVoiceIDBuf, pusVID, USBYTE( ssSizeNext ) );
	return True;
}
// 画面説明音声IDの取得
Bool PBasicMessageProc::getGamenVoiceID( ushort* pusVoiceIDBuf, short* pssVoiceIDBufSize )
{
	// 音声対応画面でない場合
	if( !m_pstVoiceGamenInfo ) { return False; }
	// 引数異常のチェック
	if( !pssVoiceIDBufSize ) { return False; }
	short ssSizeNext = m_pstVoiceGamenInfo->ssVoiceIDCount;
	short ssSizePrev = *pssVoiceIDBufSize;
	*pssVoiceIDBufSize = ssSizeNext;// ローカルに退避して先に入れる
	if( !pusVoiceIDBuf ) { return False; } // バッファサイズのみ返す
	if( ssSizePrev < ssSizeNext ) {
		*pssVoiceIDBufSize = -1;
		return False;
	}
	// バッファへコピー
	memcpy( pusVoiceIDBuf, m_pstVoiceGamenInfo->pusVoiceID, USBYTE( ssSizeNext ) );
	return True;
}
// 音声カーソル種別取得
PNLE_VoiceCursorKind PBasicMessageProc::getVoiceCursorKind()
{
	return PNLC_VoiceCursorControl::newInstance()->getVoiceCursorKind();
}
// パーツ音声IDの格納
void PBasicMessageProc::addReqPartsVoiceID( short ssPosition,
        PNLE_VoicePartsKind eVpartsKind )
{
	PNLC_VoiceRequestBuffer* pcVrb = PNLC_VoiceRequestBuffer::newInstance();
	ushort	usBuf[DEF_VoiceIdBufSize];
	short	ssBufSize = DEF_VoiceIdBufSize;
	if( getPartsVoiceID( ssPosition, eVpartsKind, usBuf, &ssBufSize ) ) {
		pcVrb->addRequest( usBuf, ssBufSize );// 通常音声、新規音声で格納
	}
}
// 画面説明音声IDの格納
void PBasicMessageProc::addReqGamenVoiceID()
{
	PNLC_VoiceRequestBuffer* pcVrb = PNLC_VoiceRequestBuffer::newInstance();
	ushort	usBuf[DEF_VoiceIdBufSize];
	short	ssBufSize = DEF_VoiceIdBufSize;
	if( getGamenVoiceID( usBuf, &ssBufSize ) ) {
		pcVrb->addRequest( usBuf, ssBufSize );// 通常音声、新規音声で格納
	}
}
// 画面開始音声IDの格納
void PBasicMessageProc::addReqStartVoiceID()
{
	// 音声非対応画面の音声ID格納
	if( addReqNotVoiceGamen() ) { return; }
	// 画面説明音声IDを格納
	addReqGamenVoiceID();
	// メッセージエリア音声IDを格納
	addReqMessageAreaVoiceID();
	// メッセージエリア音声の重複を排除
	PNLC_VoiceRequestBuffer::newInstance()->adjustMsgAreaRequest();
	// 現在のカレントパーツ音声IDを格納
	::reqVoicePartsVID( this, PNLE_VoicePartsKindKeyCur );
}
// 音声非対応画面の音声ID格納
Bool PBasicMessageProc::addReqNotVoiceGamen()
{
	ushort	usBuf[DEF_VoiceIdBufSize];
	short	ssBufSize = 0;
	// 音声対応画面の場合
	if( m_pstVoiceGamenInfo ) { return False; }
	// 音声非対応画面の音声IDを設定します
	usBuf[ssBufSize++] = v_MSGNotCorrespondVoiceGuide;
	// 音声IDを追加します
	PNLC_VoiceRequestBuffer::newInstance()->addRequest( usBuf, ssBufSize );
	return True;
}
/*================================================================================================
［機能］	0キー処理  .
［ｳｫｰｸｽﾙｰ］	未実施：2007/07/04	VSN加太.
					2007/09/26	VSN吉田 UD2対応
================================================================================================*/
void PBasicMessageProc::vproc0Key()
{
	// テンキー説明を読み上げる。
	addReqTenkeyVoiceID();
}
// 1キー処理
void PBasicMessageProc::vproc1Key()
{
	// 画面説明音声、メッセージエリア音声の再生
	// 画面説明音声を再生要求バッファへ入れる
	addReqGamenVoiceID();
	// 保持しているメッセージエリア音声を再生要求バッファへ追加する
	PNLC_VoiceRequestBuffer* pcVrb = PNLC_VoiceRequestBuffer::newInstance();
	pcVrb->addMsgAreaRequest();
}
// 2キー処理
void PBasicMessageProc::vproc2Key()
{
	// コピー、スキャンファクス以外でOverWriteしていない機能はここを通るように修正
	setBuzzerType( PCGE_BuzzerType_InputNg );	// ビープ音を鳴らします
}
// 3キー処理
void PBasicMessageProc::vproc3Key()
{
	// 再生音声の停止
	// 音声発生中以外は受付禁止
	if( !::getVoicePlayNow() ) {
		// ビープ音を鳴らします
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	// APIで再生中止を要求
	::reqVoiceStop( this );
}
// 4キー処理
void PBasicMessageProc::vproc4Key()
{
	// カーソルを前へ移動
	PNLC_VoiceCursorControl* pcVcc = PNLC_VoiceCursorControl::newInstance();
	short ssCurrent = pcVcc->getCurrent();
	// カーソルが非表示の場合
	if( ssCurrent < 0 ) {
		// ビープ音を鳴らします
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	// カーソルを前に移動します
	pcVcc->decCursor();
	// 現在位置の音声再生
	if( pcVcc->getCurrent() == ssCurrent ) {
		// ビープ音を鳴らします
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	// 現在位置のキー選択の音声IDリストを再生要求バッファへ格納する
	addReqPartsVoiceID( pcVcc->getCurrent(), PNLE_VoicePartsKindKeyCur );
}
// 5キー処理
void PBasicMessageProc::vproc5Key()
{
	// 決定処理
	// カレントパーツの取得
	PNLC_VoiceCursorControl* pcVcc = PNLC_VoiceCursorControl::newInstance();
	short ssCurrent = pcVcc->getCurrent();
	// カーソルが非表示の場合
	if( ssCurrent < 0 ) {
		// ビープ音を鳴らします
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	if( !m_pstVoiceGamenInfo ) { return; }
	PDisplayParts* pcParts =
	    m_pstVoiceGamenInfo->pstPartsList[ssCurrent].pcParts;
	// 選択中のパーツをカーソルクラスに設定します
	::setVoicePushedParts( pcParts );// panel/plibh/plibhg/plibhvoice.h
	// イベント情報の取得
	Message	msg;
	uchar	opt;
	// イベントありパーツかつ部数キーでない場合.
	if( ( pcParts->getEventInfo( &msg, &opt ) )
	    && ( &Bak_EnlCopy_Number != pcParts ) ) {
		PNLC_OperationLog::newInstance()->setLogSoftKeyPush2( pcParts );
		vsubproc5Key_Event( msg, opt );// テンキー５のサブ処理(イベントあり)
	} else {
		vsubproc5Key_NoEvent( ssCurrent ); // テンキー５のサブ処理(イベントなし)
	}
}
// 6キー処理
void PBasicMessageProc::vproc6Key()
{
	// カーソルを次へ移動
	PNLC_VoiceCursorControl* pcVcc = PNLC_VoiceCursorControl::newInstance();
	short ssCurrent = pcVcc->getCurrent();
	// カーソル非表示が非表示の場合
	if( ssCurrent < 0 ) {
		// ビープ音を鳴らします
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	// カーソルを次へ移動します
	pcVcc->incCursor();
	// 現在位置の音声再生
	if( pcVcc->getCurrent() == ssCurrent ) {
		// ビープ音を鳴らします
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	// 現在位置のキー選択の音声IDリストを再生要求バッファへ格納する
	addReqPartsVoiceID( pcVcc->getCurrent(), PNLE_VoicePartsKindKeyCur );
}
// 8キー処理
void PBasicMessageProc::vproc8Key()
{
	// 直前の音声を再生
	// ※ 音声再生要求APIの再読み上げ関数をコール
	::reqVoiceReplay();
}
// 9キー処理
void PBasicMessageProc::vproc9Key()
{
	// カーソル位置のキー選択音声を再生
	PNLC_VoiceCursorControl* pcVcc = PNLC_VoiceCursorControl::newInstance();
	// カーソルが非表示の場合
	if( pcVcc->getCurrent() < 0 ) {
		// ビープ音を鳴らします
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	// 現在位置のキー選択の音声IDリストを再生要求バッファへ格納する
	addReqPartsVoiceID( pcVcc->getCurrent(), PNLE_VoicePartsKindKeyCur );
}
// ＃キーの処理(音声専用)
void PBasicMessageProc::vprocShrp()
{
	// 音声ガイドが再生中の場合
	if( ::getVoicePlayNow() ) {
		// 現在設定されているの音量が最大の場合
		if( PnlComGetOperationSetting()->getVoiceVolume() == TYPE_VoiceVolume9 ) { return; }
		// 音量を１つアップします
		PnlComGetOperationSetting()->setVoiceVolume( TYPE_VoiceVolume( PnlComGetOperationSetting()->getVoiceVolume() + 1 ) );
		if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Info, "PBasicMessageProc::vprocShrp Set Voice Volume [%zd]", ( intptr_t )PnlComGetOperationSetting()->getVoiceVolume() ); }
		BRWC_ReqFlashChangeVolumeNotice();
		return;
	}
	// 現在設定されている音量が最大の場合
	if( PnlComGetOperationSetting()->getVoiceVolume() == TYPE_VoiceVolume9 ) {
		// ブザーを鳴らします
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	// 音量を１つアップします
	PnlComGetOperationSetting()->setVoiceVolume( TYPE_VoiceVolume( PnlComGetOperationSetting()->getVoiceVolume() + 1 ) );
	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Info, "PBasicMessageProc::vprocShrp Set Voice Volume [%zd]", ( intptr_t )PnlComGetOperationSetting()->getVoiceVolume() ); }
	BRWC_ReqFlashChangeVolumeNotice();
}
// ＊キーの処理(音声専用)
void PBasicMessageProc::vprocAster()
{
	// 音声ガイドが再生中の場合
	if( ::getVoicePlayNow() ) {
		// 現在設定されているの音量が最小の場合
		if( PnlComGetOperationSetting()->getVoiceVolume() == TYPE_VoiceVolume1 ) { return; }
		// 音量を１つダウンします
		PnlComGetOperationSetting()->setVoiceVolume( TYPE_VoiceVolume( PnlComGetOperationSetting()->getVoiceVolume() - 1 ) );
		if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Info, "PBasicMessageProc::vprocAster Set Voice Volume [%zd]", ( intptr_t )PnlComGetOperationSetting()->getVoiceVolume() ); }
		BRWC_ReqFlashChangeVolumeNotice();
		return;
	}
	// 現在設定されている音量が最小の場合
	if( PnlComGetOperationSetting()->getVoiceVolume() == TYPE_VoiceVolume1 ) {
		// ブザーを鳴らします
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	// 音量を１つダウンします
	PnlComGetOperationSetting()->setVoiceVolume( TYPE_VoiceVolume( PnlComGetOperationSetting()->getVoiceVolume() - 1 ) );
	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Info, "PBasicMessageProc::vprocAster Set Voice Volume [%zd]", ( intptr_t )PnlComGetOperationSetting()->getVoiceVolume() ); }
	BRWC_ReqFlashChangeVolumeNotice();
}
// 数値入力モードのテンキー処理
void PBasicMessageProc::vprocNumInput( uchar option )
{
	if( isNumInput( option ) ) {
		if( option <= 9 ) {
			// PNLC_VoiceRequestBuffer::getNumberVoiceIDの最後のパラメータ
			//   0x00:グループの先頭として追加
			//   0x01:グループの一員として追加
			//   0x02:グループの先頭、かつメッセージエリアの音声として追加
			//   0x03:グループの一員、かつメッセージエリアの音声として追加
			// 数値入力処理前のブザー状態を保持します
			PCGE_BuzzerType e_BuzzerType = getBuzzerType();
			// 数値入力処理
			proc10Key( option );
			// 数値入力が正常終了した場合
			if( e_BuzzerType == getBuzzerType() ) {
				// 入力された数値の音声IDを格納します（新規）
				PNLC_VoiceRequestBuffer::
				newInstance()->getNumberVoiceID( option, NULL, NULL );
			}
			// メッセージエリアの音声取得
			// →subproc10KeyでsetupGamenがよばれるため
			::reqVoiceMsgAreaVID( this );// panel/plibh/plihg/plibhvoice.h
		}
	}
}
// 数値入力チェック
// True:処理を実行 False:処理をキャンセル
Bool PBasicMessageProc::isNumInput( uchar option )
{
	return True;
}
//	ボタン（押された）
void PBasicMessageProc::procButtonOnVoice( uchar option ,Bool bl_reqVoice )
{
// comment 2014/12/12 OPS1-Sky浜野 [Poseidon][CQ504558] 音声2度再生抑止.
	ushort us_PrevWarning = onlyPanel.us_NowMessageWarning;
	
// comment 2013/02/01 Sky増田 [CQ377979][Warning対応：02720].
//	PNLE_VoicePartsKind eVpartsKind;
	PNLE_VoicePartsKind eVpartsKind = PNLE_VoicePartsKindKeyCur;
	if( False == ::getMovPushHelpExpButtonflag() ) {
		// 再生要求設定に現在の再生中止を設定
		// →ユーザー操作は現在の再生を中止する
		::setVoiceRequestFlag( True );
		// 音声ガイド中で数値入力モードの場合は音声カーソルモードへ移行
		if( ::getVoiceCursorMode() == TYPE_VoiceGuide_NumInput ) { ::setVoiceCursorMode( TYPE_VoiceGuide_Normal ); }
		// 描画処理前に選択、非選択を判定してパーツ種別を返す
		eVpartsKind = ::getVoicePushedPartsKind();

	}

	// ボタン（押された）
	procButtonOn( option );
	if( False == ::getMovPushHelpExpButtonflag() ) {
		// procButtonOn関数内でsetupGamen関数がコールされて
		// カーソルを非表示にした場合
		if( PNLC_VoiceCursorControl::newInstance()->IsHideCursorFromSetupGamen() ) {
			// 選択パーツが非表示になった場合
			if( !PNLC_VoiceCursorControl::newInstance()->isNowShownPushedParts() ||
			    !PNLC_VoiceCursorControl::newInstance()->isNowAcitvePushedParts() ) {
				// キー選択を設定します
				eVpartsKind = PNLE_VoicePartsKindKeyCur;
			}
			::showVoiceCursor();// panel/plibh/plihg/plibhvoice.h
		}
		if( !bl_reqVoice && !PNLC_EnlVoiceGuidanceControl::getScreenTransition() ) {	// デフォルト引数のままの場合.
			// 正常終了した場合.
			if( e_BuzzerType == getBuzzerType() ) {	// 2008/8/4 問題点対策No.303721 平野.
				// 再生要求を格納する
				::reqVoicePartsVID( this, eVpartsKind );
				// 画面が更新されているのでメッセージエリアを取得
// comment 2014/12/12 OPS1-Sky浜野 [Poseidon][CQ504558] 音声2度再生抑止.
//				::reqVoiceMsgAreaVID( this );
				// 警告画面を表示する場合.
				if( ( us_PrevWarning != onlyPanel.us_NowMessageWarning )
				&&	( True == onlyPanel.bl_isEnlargeDisplayMode )
				&&	( 0 != onlyPanel.us_NowMessageWarning )
				&&	( NULL != sp_MessageEnlWarningTable[onlyPanel.us_NowMessageWarning].WarningGamen ) ) {
					// メッセージエリア音声読み上げ抑止(NOP).
					// ※警告表示前にメッセージが切り替わると音声読み上げが警告表示と重複し2重に読み上げが行われるため.
					//   警告表示しようとする場合には音声を抑止する.
				} else {
				::reqVoiceMsgAreaVID( this );
				}
			}	// 2008/8/4 問題点対策No.303721 平野.
		}
	}

	::setMovPushHelpExpButtonflag( False );
}
//	ボタン（押された）スーパークラス専用.
void PBasicMessageProc::procSPCButtonOnVoice( uchar option )
{
	PNLE_VoicePartsKind eVpartsKind;
	// 再生要求設定に現在の再生中止を設定
	// →ユーザー操作は現在の再生を中止する
	::setVoiceRequestFlag( True );
	// 音声ガイド中で数値入力モードの場合は音声カーソルモードへ移行
	if( ::getVoiceCursorMode() == TYPE_VoiceGuide_NumInput ) { ::setVoiceCursorMode( TYPE_VoiceGuide_Normal ); }
	// 描画処理前に選択、非選択を判定してパーツ種別を返す
	eVpartsKind = ::getVoicePushedPartsKind();

	PCGE_BuzzerType e_BuzzerType = getBuzzerType();	//入力処理前のブザー状態を保持.	// 2008/8/4 問題点対策No.303721 平野.

	procSPCButtonOn( option );	// ボタン（押された）

	// procSPCButtonOn関数内でsetupGamen関数がコールされて
	// カーソルを非表示にした場合
	if( PNLC_VoiceCursorControl::newInstance()->IsHideCursorFromSetupGamen() ) {
		// 選択パーツが非表示になった場合
		if( !PNLC_VoiceCursorControl::newInstance()->isNowShownPushedParts() ) {
			// キー選択を設定します
			eVpartsKind = PNLE_VoicePartsKindKeyCur;
		}
		::showVoiceCursor();// panel/plibh/plihg/plibhvoice.h
	}
	// 正常終了した場合.
	if( e_BuzzerType == getBuzzerType() ) {	// 2008/8/4 問題点対策No.303721 平野.
		// 再生要求を格納する
		::reqVoicePartsVID( this, eVpartsKind );
		// 画面が更新されているのでメッセージエリアを取得
		::reqVoiceMsgAreaVID( this );
	}	// 2008/8/4 問題点対策No.303721 平野.
}
// テンキー処理
void PBasicMessageProc::proc10KeyVoice( uchar option )
{
	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Info, "PBasicMessageProc::proc10KeyVoice option [%zd]", ( intptr_t )option ); }
	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Info, "PBasicMessageProc::proc10KeyVoice Voice Cursor Mode [%zd]", ( intptr_t )::getVoiceCursorMode() ); }
	// 音声ガイドが有効の場合
	if( m_pstVoiceGamenInfo && ::getVoiceCursorMode() != TYPE_VoiceGuide_OFF ) {
		// 音声カーソルモード
		if( ::getVoiceCursorMode() == TYPE_VoiceGuide_Normal ) {
			switch( option ) {
				case 0:vproc0Key(); break; // なし
				case 1:vproc1Key(); break; // 画面説明音声、メッセージエリア音声の再生
				case 2:vproc2Key(); break; // 画面の設定可能項目音声を再生(フラグＯＮのキー選択のみ)
				case 3:vproc3Key(); break; // 再生音声の停止
				case 4:vproc4Key(); break; // カーソルを前へ移動
				case 5:vproc5Key(); break; // 決定処理
				case 6:vproc6Key(); break; // カーソルを次へ移動
				case 7:vproc7Key(); break; // 機能に応じた設定項目を再生
				case 8:vproc8Key(); break; // 直前の音声を再生
				case 9:vproc9Key(); break; // カーソル位置のキー選択音声を再生
			}
			// 数値入力モード
		} else {
			vprocNumInput( option );
		}
		// 音声ガイドが無効の場合
	} else {
		if( isNumInput( option ) ) { proc10Key( option ); }	// テンキー
	}
}
// ＃キー処理
void PBasicMessageProc::procShrpVoice( uchar option )
{
	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Info, "PBasicMessageProc::procShrpVoice Voice Cursor Mode [%zd]", ( intptr_t )::getVoiceCursorMode() ); }
	// 音声ガイドが有効の場合
	if( m_pstVoiceGamenInfo && ::getVoiceCursorMode() != TYPE_VoiceGuide_OFF ) {
		// 音声カーソルモード
		if( ::getVoiceCursorMode() == TYPE_VoiceGuide_Normal ) {
			vprocShrp();
			// 数値入力モード
		} else {
			// 再生要求設定に現在の再生中止を設定
			// →ユーザー操作は現在の再生を中止する
			::setVoiceRequestFlag( True );// panel/plibh/plibhg/plibhvoice.h
			vprocShrpInput( option );
		}
		// 音声ガイドが無効の場合
	} else {
		procShrp( option );	// シャープキー
	}
}
// ＊キー処理
void PBasicMessageProc::procAsterVoice( uchar option )
{
	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Info, "PBasicMessageProc::procAsterVoice Voice Cursor Mode [%zd]", ( intptr_t )::getVoiceCursorMode() ); }
	// 音声ガイドが有効の場合
	if( m_pstVoiceGamenInfo && ::getVoiceCursorMode() != TYPE_VoiceGuide_OFF ) {
		// 音声カーソルモード
		if( ::getVoiceCursorMode() == TYPE_VoiceGuide_Normal ) {
			vprocAster();
			// 数値入力モード
		} else {
			// 再生要求設定に現在の再生中止を設定
			// →ユーザー操作は現在の再生を中止する
			::setVoiceRequestFlag( True );// panel/plibh/plibhg/plibhvoice.h
			vprocAsterInput( option );
		}
		// 音声ガイドが無効の場合
	} else {
		procAster( option );	// アスタリスクキー
	}
}
// OKボタン後の処理
void PBasicMessageProc::procOkVoice( uchar option )
{
	// 音声ガイド対応画面でない場合は終了
	if( !m_pstVoiceGamenInfo ) { return; }
	// 音声ガイドが無効の場合は終了
	if( ::getVoiceCursorMode() == TYPE_VoiceGuide_OFF ) { return; }

	// 音声ガイド中で数値入力モードの場合は音声カーソルモードへ移行
	if( ::getVoiceCursorMode() == TYPE_VoiceGuide_NumInput ) { ::setVoiceCursorMode( TYPE_VoiceGuide_Normal ); }

	// カーソル移動に対応
	::moveVoiceCursor();// panel/plibh/plihg/plibhvoice.h

	short ss_Current = PNLC_VoiceCursorControl::newInstance()->getCurrent();

	Bool bl_VoiceFail = False;

	if( ss_Current < 0 ) {
		bl_VoiceFail = True;
	} else {
		// パーツを特定
		PNLS_VoiceCursorParts* pstVcp = &m_pstVoiceGamenInfo->pstPartsList[ss_Current];
		// キー選択決定の音声IDが存在しない場合.
		if( !pstVcp->ppusVoiceID[PNLE_VoicePartsKindKeySel] ) {
			bl_VoiceFail = True;
		}
	}
	if( bl_VoiceFail ) {
		procOk( option );	// 音声の再生は行わないので、そのままprocOkを処理.
		return;
	}

	// オプションをバックアップ.
	uc_procOkVoiceOptionBackup = option;

	// 再生要求バッファのクリア.
	PNLC_VoiceRequestBuffer::newInstance()->clearRequest();

	// 再生要求設定に現在の再生中止を設定 & 音声の完了通知用に画面ポインター設定.
	PLayerControl* p_GamenInstance = dynamic_cast< PLayerControl* >( this );
	if( NULL != p_GamenInstance ) {
		::setVoiceRequestFlag( True, this );
	} else {
		::setVoiceRequestFlag( True, nowGamen );
	}

	// 再生終了待ちの再生要求.
	::reqVoicePartsVID( this, PNLE_VoicePartsKindKeySel );

	// 再生要求を投げる.
	PNLC_VoiceRequestBuffer::newInstance()->sendRequest();

}
// IDキー処理
void PBasicMessageProc::procIDInputVoice( uchar option )
{
	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Info, "PBasicMessageProc::procIDInputVoice Voice Cursor Mode [%zd]", ( intptr_t )::getVoiceCursorMode() ); }
	// 数値入力モードの場合
	if( ::getVoiceCursorMode() == TYPE_VoiceGuide_NumInput ) {
		// 再生要求設定に現在の再生中止を設定
		// →ユーザー操作は現在の再生を中止する
		::setVoiceRequestFlag( True );// panel/plibh/plibhg/plibhvoice.h
		// 音声ガイド有効かつ数値入力モード
		// ■処理
		// 1.部数を読み上げる
		//   →カレントカーソル位置のIDキー用パーツ音声を取得する
		//     再生要求バッファへ格納する
		PNLC_VoiceCursorControl* pcVcc = PNLC_VoiceCursorControl::newInstance();
		// 現在位置の数値入力モード中のIDキーでの
		// 音声IDリストを再生要求バッファへ格納する
		addReqPartsVoiceID( pcVcc->getCurrent(), PNLE_VoicePartsKindKeyID );
		// 2.音声カーソルモードを設定する
		::setVoiceCursorMode( TYPE_VoiceGuide_Normal );
		if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Info, "PBasicMessageProc::procIDInputVoice Set Voice Cursor Mode [%zd]", ( intptr_t )::getVoiceCursorMode() ); }
	} else {
		procIDInput( option );	// IDキーの処理
	}
}
// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.		
Bool PBasicMessageProc::idKeyActiveCheckVoice()
{
	// 数値入力モードの場合
	if( ::getVoiceCursorMode() == TYPE_VoiceGuide_NumInput ) {
		ACTIVEKEY_RET( True );
	} else {
		 return idKeyActiveCheck();
	}
}

//	キーボード上のキー.
void PBasicMessageProc::procKeyBoardOnVoice( uchar option )
{
	PNLE_VoicePartsKind eVpartsKind;
	// 再生要求設定に現在の再生中止を設定
	// →ユーザー操作は現在の再生を中止する
	::setVoiceRequestFlag( True );
	// 描画処理前に選択、非選択を判定してパーツ種別を返す
	eVpartsKind = ::getVoicePushedPartsKind();
	// キーボード上のキー.
	procKeyBoard( option );
	// procKeyBoard関数内でsetupGamen関数がコールされて
	// カーソルを非表示にした場合
	if( PNLC_VoiceCursorControl::newInstance()->IsHideCursorFromSetupGamen() ) {
		// 選択パーツが非表示になった場合
		if( !PNLC_VoiceCursorControl::newInstance()->isNowShownPushedParts() ) {
			// キー選択を設定します
			eVpartsKind = PNLE_VoicePartsKindKeyCur;
		}
		::showVoiceCursor();// panel/plibh/plihg/plibhvoice.h
	}
	// 再生要求を格納する
	::reqVoicePartsVID( this, eVpartsKind );
	// 画面が更新されているのでメッセージエリアを取得
	::reqVoiceMsgAreaVID( this );
}
//	ワンタッチキー.
void PBasicMessageProc::procOneTouchKeyVoice( uchar option )
{
	// 再生要求設定に現在の再生中止を設定
	// →ユーザー操作は現在の再生を中止する
	::setVoiceRequestFlag( True );
	// ワンタッチキー.
	procOneTouchKey( option );

	// procOneTouchKey関数内でsetupGamen関数がコールされて
	// カーソルを非表示にした場合
	if( PNLC_VoiceCursorControl::newInstance()->IsHideCursorFromSetupGamen() ) {
		::showVoiceCursor();// panel/plibh/plihg/plibhvoice.h
	}
	// 画面が更新されているのでメッセージエリアを取得
	::reqVoiceMsgAreaVID( this );
}

void PBasicMessageProc::procUSBKeyBoardStillOnFast( uchar option ) {
	char sc_USBKeyCharCode[TYPD_CharCode];							// 文字コードまたは制御コードの格納領域.
	memset( sc_USBKeyCharCode, 0, sizeof( sc_USBKeyCharCode ) );
	Kbd_ScanKeybordConv.getUSBKeyCharCode( sc_USBKeyCharCode );	// 文字コードまたは制御コードを取得する.

	if( 0 == sc_USBKeyCharCode[2] ) {	// 押下されたキーが文字コードの場合.
		// 現状キーリピートしない.

	} else {	// 押下されたキーが制御コードの場合.
		switch( sc_USBKeyCharCode[2] ) {
			case TYPE_USBkey_Yajirusi_left:	// 左シフトキー.
			case TYPE_USBkey_Yajirusi_right:// 右シフトキー.
			case TYPE_USBkey_Backspace:		// 削除キー.
			case TYPE_USBkey_Yajirusi_up:	// 上シフトキー.
			case TYPE_USBkey_Yajirusi_down:	// 下シフトキー.
				procUSBKeyBoard( option );
				break;
			default: break;
		}
	}
}

void PBasicMessageProc::procUSBKeyBoard( uchar option ) {
	char sc_USBKeyCharCode[TYPD_CharCode];							// 文字コードまたは制御コードの格納領域.
	memset( sc_USBKeyCharCode, 0, sizeof( sc_USBKeyCharCode ) );
	if( False == isUSBKeyBoardMode() ) {	// USBキーボードモードでなければキー押下処理をはじく.
		return ;
	}
	if( False == Key_USBKeybordcomOomoji.isNowActivate() ) {	// 表示非活性化時はUSBキーボード無効.
		return;
	}
	Kbd_ScanKeybordConv.getUSBKeyCharCode( sc_USBKeyCharCode );	// 文字コードまたは制御コードを取得する.
	PMGC_PanelManager* pc_PanelManager = PMGC_PanelManager::newInstance();//PanelManagerインスタンス取得.
	PNLC_OperationLog::newInstance()->setLogUSBKeyBoardPush( sc_USBKeyCharCode, TYPD_CharCode, sc_USBKeyCharCode[2] );
	PDPC_KeyMapManager* pc_KeyMapManager = PDPC_KeyMapManager::newInstance();	//【UV-00783】Sky_hosokawa.
	if( 0 == sc_USBKeyCharCode[2] ) {	// 押下されたキーが文字コードの場合.

		short ss_i = 0;
		char sc_Temp[TYPD_CharCode];
		memset( sc_Temp, 0, sizeof( sc_Temp ) );
		if( ( True == Kbd_ScanFaxKeybordConv.b_KeybordCharCodeInputMode ) || ( True == Kbd_ScanFaxKeybordConv.b_KeybordCharCodeInputModeEnl ) ) {	// 文字コード入力中の場合.
			USBKeyIPv* p_USBKeyCharCodeInputModeChar = NULL;
			short ss_MaxSize = 0;
			PLayerControl* p_GamenInstance = dynamic_cast< PLayerControl* >( nowGamen );
			if( NULL == p_GamenInstance && False == onlyPanel.bl_isEnlargeDisplayMode ) {	// 第3世代キーボード画面(非拡大画面)の場合.
				p_USBKeyCharCodeInputModeChar = tbl_USBKeyCharCodeInputModeCharWide;
				ss_MaxSize = ss_USBKeyCharCodeInputModeCharWideSize;
			} else {	// 第4世代キーボード画面、または拡大キーボード画面の場合.
				p_USBKeyCharCodeInputModeChar = tbl_USBKeyCharCodeInputModeChar;
				ss_MaxSize = ss_USBKeyCharCodeInputModeCharSize;
			}
			for( ss_i = 0; ss_i < ss_MaxSize; ss_i++ ) {
				memset( sc_Temp, 0, sizeof( sc_Temp ) );
				memcpy( sc_Temp, sc_USBKeyCharCode, sizeof( sc_Temp ) );
				if( True == ( pc_KeyMapManager->isUSBKeyBoardKeyMapFromMSbin( PnlComGetUSBKeyboardLanguage() ) ) )	{
					PnlComkeyBoardInputCharUnicodeToNativecode( sc_Temp );	// Unicodeから各国文字集合に変換.
				}
				if( 0 == strcmp( sc_Temp, p_USBKeyCharCodeInputModeChar[ss_i].sc_IPvChar ) ) {
					procKeyBoard( p_USBKeyCharCodeInputModeChar[ss_i].uc_option );
				}
			}
			// 文字コード入力中以外の場合.
		} else if( True == Kbd_ScanFaxKeybordConv.isUSBKeyCharCode( sc_USBKeyCharCode ) ) { // ここで無効文字コードをはじく. isUSBKeyCharCodeはキーボードクラスのstatic変数を参照し戻り値を返してくる為、インスタンスは何でもいい.今回はKbd_ScanFaxKeybordConvを使用する.
			// IPv4v6画面処理の切り分け.
			switch( Kbd_ScanFaxKeybordConv.isIPv4v6Gamen() ) {
				case TYPE_USBKeyIPvGamen_v4:// 現在がIPv4入力画面の場合.
					for( ss_i = 0; ss_USBKeyIPv4CharSize > ss_i; ss_i++ ) {
						memset( sc_Temp, 0, sizeof( sc_Temp ) );
						memcpy( sc_Temp, sc_USBKeyCharCode, sizeof( sc_Temp ) );
						if( True == ( pc_KeyMapManager->isUSBKeyBoardKeyMapFromMSbin( PnlComGetUSBKeyboardLanguage() ) ) )	{
							PnlComkeyBoardInputCharUnicodeToNativecode( sc_Temp );	// Unicodeから各国文字集合に変換.
						}
						if( 0 == strcmp( tbl_USBKeyIPv4Char[ss_i].sc_IPvChar, sc_Temp ) ) {
							procButtonOn( tbl_USBKeyIPv4Char[ss_i].uc_option );
							break;
						}
					}
					break;
				case TYPE_USBKeyIPvGamen_v6: // 現在がIPv6入力画面の場合.
					for( ss_i = 0; ss_USBKeyIPv6CharSize > ss_i; ss_i++ ) {
						memset( sc_Temp, 0, sizeof( sc_Temp ) );
						memcpy( sc_Temp, sc_USBKeyCharCode, sizeof( sc_Temp ) );
						if( True == ( pc_KeyMapManager->isUSBKeyBoardKeyMapFromMSbin( PnlComGetUSBKeyboardLanguage() ) ) )	{
							PnlComkeyBoardInputCharUnicodeToNativecode( sc_Temp );	// Unicodeから各国文字集合に変換.
						}
						if( 0 == strcmp( tbl_USBKeyIPv6Char[ss_i].sc_IPvChar, sc_Temp ) ) {
							procButtonOn( tbl_USBKeyIPv6Char[ss_i].uc_option );
							break;
						}
					}
					break;
				default: // 通常キーボード画面の場合.
					if( ( TYPE_USBKeyBordGamen_Input != Kbd_ScanKeybordConv.e_USBKeyBordGamen )
					    || ( True == Kbd_ScanKeybordConv.b_KeybordConvertMode ) ) {
						return ;// USBキーボード文字入力画面以外は、文字入力を受け付けない.
					}
					procKeyBoard( option );
					break;
			}
		}
	} else {	// 押下されたキーが制御コードの場合.
		// 現在がIPv4・v6入力キーボード画面の場合.
		if( TYPE_USBKeyIPvGamen_Non != Kbd_ScanFaxKeybordConv.isIPv4v6Gamen() ) {
			switch( sc_USBKeyCharCode[2] ) {
				case TYPE_USBkey_Yajirusi_up:
				case TYPE_USBkey_Yajirusi_left:	// 左シフトキー.
					if( True == pc_PanelManager->getUSBKeyboardScrollLock() )
					{
						return;
					}
					procButtonOn( 4 );
					break ;
				case TYPE_USBkey_Yajirusi_down:
				case TYPE_USBkey_Yajirusi_right:// 右シフトキー.
					if( True == pc_PanelManager->getUSBKeyboardScrollLock() )
					{
						return;
					}
					procButtonOn( 5 );
					break ;
				case TYPE_USBkey_Backspace:		// 削除キー.
					procButtonOn( 3 );
					break;
				case TYPE_USBkey_Enter:			// Enterキー.
					procOk( option );
					break ;
				case TYPE_USBkey_Numlock:
					pc_PanelManager->setUSBKeyboardNumLock( !( pc_PanelManager->getUSBKeyboardNumLock() ) );// PanelManagerにモード変更通知を行う.
					pc_PanelManager->USBKeyLEDNotice( TYPE_KEYBDLED_INFO_NUM, pc_PanelManager->getUSBKeyboardNumLock() ); // USBキーボードキーのNUMLOCKランプ点灯・消灯処理を行う.
					break;
				case TYPE_USBkey_Scrolllock:
					pc_PanelManager->setUSBKeyboardScrollLock( !( pc_PanelManager->getUSBKeyboardScrollLock() ) );// PanelManagerにモード変更通知を行う.
					pc_PanelManager->USBKeyLEDNotice( TYPE_KEYBDLED_INFO_SCR, pc_PanelManager->getUSBKeyboardScrollLock() ); // USBキーボードキーのSCROLLLOCKランプ点灯・消灯処理を行う.
					break;
				default: break;
			}
			return ;
		}
		if( True == Kbd_ScanFaxKeybordConv.isUSBKeyControlCode( sc_USBKeyCharCode ) ) { // ここで無効制御コードをはじく.isUSBKeyControlCodeはキーボードクラスのstatic変数を参照し戻り値を返してくる為、インスタンスは何でもいい.今回はKbd_ScanFaxKeybordConvを使用する.
			switch( sc_USBKeyCharCode[2] ) {
				case TYPE_USBkey_Enter:
					if( ( True == Kbd_ScanFaxKeybordConv.b_KeybordConvertMode )
					    || ( True == Kbd_ScanFaxKeybordConvEnl.b_KeybordConvertModeEnl ) ) {
						procButtonOn( 228 );// 変換画面でのEnter.
					} else if( ( ( True == Kbd_ScanFaxKeybordConv.b_KeybordCharCodeInputMode ) || ( True == Kbd_ScanFaxKeybordConv.b_KeybordCharCodeInputModeEnl ) )
					           && ( ( '\0' != Kbd_BoxKeybordConv.c_PreString[0] )// ここではキーボードインスタンスが特定できない
					                || ( '\0' != Kbd_ScanKeybordConv.c_PreString[0] )//　為、全てのキーボードインスタンスを確認する.
					                || ( '\0' != Kbd_ScanFaxKeybordConv.c_PreString[0] )
					                || ( '\0' != Kbd_ScanFaxKeybordConv2.c_PreString[0] )
					                || ( '\0' != pt_Kbd_ServiceInputKeybord && ( ( PLBC_ConvertKeybord* )pt_Kbd_ServiceInputKeybord )->c_PreString[0] )
					                || ( '\0' != pt_Kbd_UtilityHostAddressInputKeybordConv && ( ( PLBC_ConvertKeybord* )pt_Kbd_UtilityHostAddressInputKeybordConv )->c_PreString[0] )
					                || ( '\0' != pt_Kbd_SvcsKeybordConv && ( ( PLBC_ConvertKeybord* )pt_Kbd_SvcsKeybordConv )->c_PreString[0] )
					                || ( '\0' != pt_Kbd_EnlKeybordActConv && ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_EnlKeybordActConv )->c_PreString[0] )
					                || ( '\0' != pt_Kbd_ScanFaxKeybordConvEnl && ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_ScanFaxKeybordConvEnl )->c_PreString[0] )
					                || ( '\0' != pt_Kbd_ScanFaxKeybordConv2Enl && ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_ScanFaxKeybordConv2Enl )->c_PreString[0] )
					                || ( '\0' != Kbd_ScanOapKeybordConv.c_PreString[0] )
					                || ( '\0' != Kbd_EnlAct_KeybordConv.c_PreString[0] )						// comment 2023/10/16 Sky東島 [IT6_2.3][OP_BTS-44864].
					                || ( '\0' != Kbd_UtilityInputKeybordConv.c_PreString[0] )
					                || ( '\0' != Kbd_ComKeybordConv.c_PreString[0] ) ) ) {
						procKeyBoard( 152 );// 文字コード入力画面で未確定文字がある場合のEnter.
					} else {
						if( TYPE_USBKeyBordGamen_Input != Kbd_ScanKeybordConv.e_USBKeyBordGamen ) {
							if( ( True == onlyPanel.bl_isEnlargeDisplayMode ) || ( PLBD_Enl_KBD_Enlarge == Kbd_ScanKeybordConv.getChangeEnlKeybord() ) ) {
								procButtonOn( 252 );// // EnlageDisp、拡大ON表示中USBキーボードの入力モード設定画面(文字入力タイプ、大文字・小文字、入力方式)でのEnter.
							} else {
								procButtonOn( 245 );// USBキーボードの入力モード設定画面(文字入力タイプ、大文字・小文字、入力方式)でのEnter.
							}
						} else {	// USBキーボード入力画面でのEnter.
							procOk( 227 );
						}
					}
					break;
				case TYPE_USBkey_Space:	// 未確定文字列が入力済みの場合.
					if( ( TYPE_USBKeyBordGamen_Input != Kbd_ScanKeybordConv.e_USBKeyBordGamen )
					    || ( True == Kbd_ScanKeybordConv.b_KeybordConvertMode ) ) {
						return ;// USBキーボード文字入力画面以外は、文字入力を受け付けない.
					}

					if( ( '\0' != Kbd_BoxKeybordConv.c_PreString[0] ) // ここではキーボードインスタンスが特定できない
					    || ( '\0' != Kbd_ScanKeybordConv.c_PreString[0] )//　為、全てのキーボードインスタンスを確認する.
					    || ( '\0' != Kbd_ScanFaxKeybordConv.c_PreString[0] )
					    || ( '\0' != Kbd_ScanFaxKeybordConv2.c_PreString[0] )
					    || ( '\0' != pt_Kbd_ServiceInputKeybord && ( ( PLBC_ConvertKeybord* )pt_Kbd_ServiceInputKeybord )->c_PreString[0] )
					    || ( '\0' != pt_Kbd_UtilityHostAddressInputKeybordConv && ( ( PLBC_ConvertKeybord* )pt_Kbd_UtilityHostAddressInputKeybordConv )->c_PreString[0] )
					    || ( '\0' != pt_Kbd_SvcsKeybordConv && ( ( PLBC_ConvertKeybord* )pt_Kbd_SvcsKeybordConv )->c_PreString[0] )
					    || ( '\0' != pt_Kbd_EnlKeybordActConv && ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_EnlKeybordActConv )->c_PreString[0] )
					    || ( '\0' != pt_Kbd_ScanFaxKeybordConvEnl && ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_ScanFaxKeybordConvEnl )->c_PreString[0] )
					    || ( '\0' != pt_Kbd_ScanFaxKeybordConv2Enl && ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_ScanFaxKeybordConv2Enl )->c_PreString[0] )
					    || ( '\0' != Kbd_ScanOapKeybordConv.c_PreString[0] )
					    || ( '\0' != Kbd_UtilityInputKeybordConv.c_PreString[0] ) ) {
						procButtonOn( 226 );// 変換する.
					} else {
						if( ( TYPE_LanguageJapanese == PnlComGetUSBKeyboardLanguage() )// 現在がひらがな、カタカナ入力の場合は英数(半)へ切り替える.
						    && ( ( TYPE_USBKeyInput_Country == pc_PanelManager->getUSBKeyBoardCountry() )
						         || ( TYPE_USBKeyInput_Katakana == pc_PanelManager->getUSBKeyBoardCountry() ) ) ) {
							procKeyBoard( 153 );// 一文字空白キー.
						} else {
							char sc_TempChar[TYPD_CharCode] = {0,0,0};
							sc_TempChar[0] = 0x20;
							if( False == Kbd_ScanFaxKeybordConv.isDisableKeyCharCode( sc_TempChar ) ) {
								procKeyBoard( 152 );// 一文字spaceキー.
							}
						}
					}
					break;
				case TYPE_USBkey_Henkan:
					if( ( TYPE_USBKeyBordGamen_Input != Kbd_ScanKeybordConv.e_USBKeyBordGamen )
					    || ( True == Kbd_ScanKeybordConv.b_KeybordConvertMode ) ) {
						return ;// USBキーボード文字入力画面以外は、文字入力を受け付けない.
					}
					procButtonOn( 226 ); // 変換する.
					break;
				case TYPE_USBkey_Muhenkan:
					if( ( TYPE_USBKeyBordGamen_Input != Kbd_ScanKeybordConv.e_USBKeyBordGamen )
					    || ( True == Kbd_ScanKeybordConv.b_KeybordConvertMode ) ) {
						return ;// USBキーボード文字入力画面以外は、文字入力を受け付けない.
					}
					procButtonOn( 227 ); // 無変換処理をする.
					break;
				case TYPE_USBkey_Pageup:
					// 今回はKbd_ScanFaxKeybordConvにする.
					if( ( True == Kbd_ScanFaxKeybordConv.b_KeybordConvertMode ) // 漢字変換画面の場合、処理を行う.
					    || ( pt_Kbd_EnlKeybordActConv && True == ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_EnlKeybordActConv )->b_KeybordConvertModeEnl ) ) {
						// 現在ページが1ページ目なら、処理をはじく.
						if( ( 1 != Kbd_BoxKeybordConv.us_PageNumberSelect )	// ここではキーボードインスタンスが特定で.
						    || ( 1 != Kbd_ScanKeybordConv.us_PageNumberSelect )	// きないため全てのキーボードインスタンスの.
						    || ( 1 != Kbd_ScanFaxKeybordConv.us_PageNumberSelect ) //変数を確認.
						    || ( 1 != Kbd_ScanFaxKeybordConv2.us_PageNumberSelect )
						    || ( pt_Kbd_ServiceInputKeybord && 1 != ( ( PLBC_GeneralKeybord* )pt_Kbd_ServiceInputKeybord )->us_PageNumberSelect )
						    || ( pt_Kbd_UtilityHostAddressInputKeybordConv && 1 != ( ( PLBC_GeneralKeybord* )pt_Kbd_UtilityHostAddressInputKeybordConv )->us_PageNumberSelect )
						    || ( pt_Kbd_SvcsKeybordConv && 1 != ( ( PLBC_GeneralKeybord* )pt_Kbd_SvcsKeybordConv )->us_PageNumberSelect )
						    || ( pt_Kbd_EnlKeybordActConv && 1 != ( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->us_PageNumberSelect )
						    || ( pt_Kbd_ScanFaxKeybordConvEnl && 1 != ( ( PLBC_GeneralKeybord* )pt_Kbd_ScanFaxKeybordConvEnl )->us_PageNumberSelect )
						    || ( pt_Kbd_ScanFaxKeybordConv2Enl && 1 != ( ( PLBC_GeneralKeybord* )pt_Kbd_ScanFaxKeybordConv2Enl )->us_PageNumberSelect )
						    || ( 1 != Kbd_ScanOapKeybordConv.us_PageNumberSelect )
						    || ( 1 != Kbd_UtilityInputKeybordConv.us_PageNumberSelect ) ) {
							procButtonOn( 230 );
						}
					}
					break;
				case TYPE_USBkey_Pagedown:
					// 今回はKbd_ScanFaxKeybordConvにする.
					if( ( True == Kbd_ScanFaxKeybordConv.b_KeybordConvertMode ) // 漢字変換画面の場合、処理を行う.
					    || ( pt_Kbd_EnlKeybordActConv && True == ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_EnlKeybordActConv )->b_KeybordConvertModeEnl ) ) {
						// 現在ページが最後のページなら、処理をはじく.
						if( ( Kbd_BoxKeybordConv.us_TotalPage != Kbd_BoxKeybordConv.us_PageNumberSelect )
						    || ( Kbd_ScanKeybordConv.us_TotalPage != Kbd_ScanKeybordConv.us_PageNumberSelect )
						    || ( Kbd_ScanFaxKeybordConv.us_TotalPage != Kbd_ScanFaxKeybordConv.us_PageNumberSelect )
						    || ( Kbd_ScanFaxKeybordConv2.us_TotalPage != Kbd_ScanFaxKeybordConv2.us_PageNumberSelect )
						    || ( pt_Kbd_ServiceInputKeybord && ( ( ( PLBC_ConvertKeybord* )pt_Kbd_ServiceInputKeybord )->us_TotalPage != ( ( PLBC_GeneralKeybord* )pt_Kbd_ServiceInputKeybord )->us_PageNumberSelect ) )
						    || ( pt_Kbd_UtilityHostAddressInputKeybordConv && ( ( ( PLBC_ConvertKeybord* )pt_Kbd_UtilityHostAddressInputKeybordConv )->us_TotalPage != ( ( PLBC_GeneralKeybord* )pt_Kbd_UtilityHostAddressInputKeybordConv )->us_PageNumberSelect ) )
						    || ( pt_Kbd_SvcsKeybordConv && ( ( ( PLBC_ConvertKeybord* )pt_Kbd_SvcsKeybordConv )->us_TotalPage != ( ( PLBC_GeneralKeybord* )pt_Kbd_SvcsKeybordConv )->us_PageNumberSelect ) )
						    || ( pt_Kbd_EnlKeybordActConv && ( ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_EnlKeybordActConv )->us_TotalPage != ( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->us_PageNumberSelect ) )
						    || ( pt_Kbd_ScanFaxKeybordConvEnl && ( ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_ScanFaxKeybordConvEnl )->us_TotalPage != ( ( PLBC_GeneralKeybord* )pt_Kbd_ScanFaxKeybordConvEnl )->us_PageNumberSelect ) )
						    || ( pt_Kbd_ScanFaxKeybordConv2Enl && ( ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_ScanFaxKeybordConv2Enl )->us_TotalPage != ( ( PLBC_GeneralKeybord* )pt_Kbd_ScanFaxKeybordConv2Enl )->us_PageNumberSelect ) )
						    || ( Kbd_ScanOapKeybordConv.us_TotalPage !=  Kbd_ScanOapKeybordConv.us_PageNumberSelect )
						    || ( Kbd_UtilityInputKeybordConv.us_TotalPage  != Kbd_UtilityInputKeybordConv.us_PageNumberSelect ) ) {
							procButtonOn( 231 );
						}
					}
					break;
				case TYPE_USBkey_Yajirusi_up:
					if( True == pc_PanelManager->getUSBKeyboardScrollLock() )
					{
						return;
					}
					if( ( True == Kbd_ScanFaxKeybordConv.b_KeybordConvertMode ) // 漢字変換画面の場合.
					    || ( pt_Kbd_EnlKeybordActConv && True == ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_EnlKeybordActConv )->b_KeybordConvertModeEnl ) ) {
						if( ( PLBD_Enl_KBD_Enlarge != Kbd_ScanFaxKeybordConv.getChangeEnlKeybord() ) && ( False == onlyPanel.bl_isEnlargeDisplayMode ) ) {	// 通常モードならば.
							if( False == Kbd_ScanFaxKeybordConv.b_KeybordKohoSelect ) {	// 変換候補が未選択の場合.
								procButtonOn( 112 );	// 一番左上のキーを選択状態にする.
							}
							else {
								if( 2 < Kbd_ScanFaxKeybordConv.uc_OptionSelect )  {
									//Kbd_ScanFaxKeybordConv.uc_OptionSelect -= 3;
									Kbd_ScanFaxKeybordConv.uc_OptionSelect = static_cast<ushort>( Kbd_ScanFaxKeybordConv.uc_OptionSelect - 3 );
									procButtonOn( (uchar)(112 + Kbd_ScanFaxKeybordConv.uc_OptionSelect) ); // 一つ上のキーを選択状態にする.
								}
							}
						} else if( ( PLBD_Enl_KBD_Enlarge == Kbd_ScanFaxKeybordConv.getChangeEnlKeybord() ) && ( False == onlyPanel.bl_isEnlargeDisplayMode ) ) {	// 切り替え拡大モードならば.
							if( False == Kbd_ScanFaxKeybordConv.b_KeybordKohoSelect ) {	// 変換候補が未選択の場合.
								procButtonOn( 112 );	// 一番左上のキーを選択状態にする.
							}
							else {
								if( ( 0 != Kbd_ScanFaxKeybordConv.uc_OptionSelect ) && ( 5 != Kbd_ScanFaxKeybordConv.uc_OptionSelect ) )  {
									//Kbd_ScanFaxKeybordConv.uc_OptionSelect -= 1;
									Kbd_ScanFaxKeybordConv.uc_OptionSelect = static_cast<ushort>( Kbd_ScanFaxKeybordConv.uc_OptionSelect - 1 );
									procButtonOn( (uchar)(112 + Kbd_ScanFaxKeybordConv.uc_OptionSelect) ); // 一つ上のキーを選択状態にする.
								}
							}
						} else if( ( PLBD_Enl_KBD_Enlarge == Kbd_ScanFaxKeybordConv.getChangeEnlKeybord() ) && ( True == onlyPanel.bl_isEnlargeDisplayMode ) )	{ // 拡大キーボードならば.
							if( ( False == Kbd_ScanFaxKeybordConv.b_KeybordKohoSelect ) && ( pt_Kbd_EnlKeybordActConv && False == ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_EnlKeybordActConv )->b_KeybordKohoSelect ) ) {	// EnlargeDisp時に、PCGC_ConvertKeyBoardクラスのインスタンスを使用している画面がある為.
								procButtonOn( 112 );	// 一番左上のキーを選択状態にする.
							}
							else {
								if( pt_Kbd_EnlKeybordActConv && True == ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_EnlKeybordActConv )->b_KeybordConvertModeEnl ) { 	// PCGC_ConvertKeyBoardEnlを使用している場合.
									if( ( 0 != ( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect ) && ( 5 != ( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect ) )  {
										//( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect -= 1;
										( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect = static_cast<ushort>( ( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect -1 );
										procButtonOn( (uchar)(112 + ( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect) ); // 一つ下のキーを選択状態にする.
									}
								} else if( True == Kbd_ScanFaxKeybordConv.b_KeybordConvertMode ) {	// PCGC_ConvertKeyBoardを使用している場合.
									if( ( 0 != Kbd_ScanFaxKeybordConv.uc_OptionSelect ) && ( 5 != Kbd_ScanFaxKeybordConv.uc_OptionSelect ) )  {
										//Kbd_ScanFaxKeybordConv.uc_OptionSelect -= 1;
										Kbd_ScanFaxKeybordConv.uc_OptionSelect = static_cast<ushort>( Kbd_ScanFaxKeybordConv.uc_OptionSelect - 1 );
										procButtonOn( (uchar)(112 + Kbd_ScanFaxKeybordConv.uc_OptionSelect) ); // 一つ下のキーを選択状態にする.
									}
								}
							}
						}
					} else {
						procKeyBoard( 149 );
					}
					break;
				case TYPE_USBkey_Yajirusi_down:
					if( True == pc_PanelManager->getUSBKeyboardScrollLock() )
					{
						return;
					}
					if( ( True == Kbd_ScanFaxKeybordConv.b_KeybordConvertMode ) // 漢字変換画面の場合.
					    || ( True == ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_EnlKeybordActConv )->b_KeybordConvertModeEnl ) ) {
						if( ( PLBD_Enl_KBD_Enlarge != Kbd_ScanFaxKeybordConv.getChangeEnlKeybord() ) && ( False == onlyPanel.bl_isEnlargeDisplayMode ) ) {	// 通常モードならば.
							if( False == Kbd_ScanFaxKeybordConv.b_KeybordKohoSelect ) {	// 変換候補が未選択の場合.
								procButtonOn( 112 );	// 一番左上のキーを選択状態にする.
							}
							else {
								if( ( PLBD_KBD_KohoMax - 3 > Kbd_ScanFaxKeybordConv.uc_OptionSelect )
								    && ( True == Key_Convert[ Kbd_ScanFaxKeybordConv.uc_OptionSelect + 3 ].isNowShowEnable() ) ) {
									//Kbd_ScanFaxKeybordConv.uc_OptionSelect += 3;
									Kbd_ScanFaxKeybordConv.uc_OptionSelect = static_cast<ushort>( Kbd_ScanFaxKeybordConv.uc_OptionSelect + 3 );
									procButtonOn( (uchar)(112 + Kbd_ScanFaxKeybordConv.uc_OptionSelect) ); // 一つ上のキーを選択状態にする.
								}
							}
						} else if( ( PLBD_Enl_KBD_Enlarge == Kbd_ScanFaxKeybordConv.getChangeEnlKeybord() ) && ( False == onlyPanel.bl_isEnlargeDisplayMode ) ) {	// 切り替え拡大モードならば.
							if( False == Kbd_ScanFaxKeybordConv.b_KeybordKohoSelect ) {	// 変換候補が未選択の場合.
								procButtonOn( 112 );	// 一番左上のキーを選択状態にする.
							}
							else {
								if( ( 4 != Kbd_ScanFaxKeybordConv.uc_OptionSelect ) && ( 9 != Kbd_ScanFaxKeybordConv.uc_OptionSelect ) && ( True == Key_EnlConvert[( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect + 1 ].isNowShowEnable() ) ) {
									//Kbd_ScanFaxKeybordConv.uc_OptionSelect += 1;
									Kbd_ScanFaxKeybordConv.uc_OptionSelect = static_cast<ushort>( Kbd_ScanFaxKeybordConv.uc_OptionSelect + 1 );
									procButtonOn( (uchar)(112 + Kbd_ScanFaxKeybordConv.uc_OptionSelect) ); // 一つ上のキーを選択状態にする.
								}
							}
						} else if( ( PLBD_Enl_KBD_Enlarge == Kbd_ScanFaxKeybordConv.getChangeEnlKeybord() ) && ( True == onlyPanel.bl_isEnlargeDisplayMode ) )	{ // 拡大キーボードならば.
							if( ( False == Kbd_ScanFaxKeybordConv.b_KeybordKohoSelect ) && ( False == ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_EnlKeybordActConv )->b_KeybordKohoSelect ) ) {	// EnlargeDisp時に、PCGC_ConvertKeyBoardクラスのインスタンスを使用している画面がある為.
								procButtonOn( 112 );
							}
							else {
								if( pt_Kbd_EnlKeybordActConv && True == ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_EnlKeybordActConv )->b_KeybordConvertModeEnl ) { 	// PCGC_ConvertKeyBoardEnlを使用している場合.
									if( ( 4 != ( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect ) && ( 9 != ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect ) && ( True == Key_EnlConvert[( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect + 1 ].isNowShowEnable() ) ) {
										//( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect += 1;
										( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect = static_cast<ushort>( ( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect + 1 );
										procButtonOn( (uchar)(112 + ( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect) ); // 一つ上のキーを選択状態にする.
									}
								} else if( Kbd_ScanFaxKeybordConv.b_KeybordConvertMode ) {	 // PCGC_ConvertKeyBoardを使用している場合.
									if( ( 4 != Kbd_ScanFaxKeybordConv.uc_OptionSelect ) && ( 9 != Kbd_ScanFaxKeybordConv.uc_OptionSelect ) && ( True == Key_EnlConvert[ Kbd_ScanFaxKeybordConv.uc_OptionSelect + 1 ].isNowShowEnable() ) ) {
										//Kbd_ScanFaxKeybordConv.uc_OptionSelect += 1;
										Kbd_ScanFaxKeybordConv.uc_OptionSelect = static_cast<ushort>( Kbd_ScanFaxKeybordConv.uc_OptionSelect + 1 );
										procButtonOn( (uchar)(112 + Kbd_ScanFaxKeybordConv.uc_OptionSelect) ); // 一つ上のキーを選択状態にする.
									}
								}
							}
						}
					} else {
						procKeyBoard( 150 );
					}
					break;
				case TYPE_USBkey_Yajirusi_left:
					if( True == pc_PanelManager->getUSBKeyboardScrollLock() )
					{
						return;
					}
					if( ( True == Kbd_ScanFaxKeybordConv.b_KeybordConvertMode ) // 漢字変換画面の場合.
					    || ( True == ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_EnlKeybordActConv )->b_KeybordConvertModeEnl ) ) {
						if( ( PLBD_Enl_KBD_Enlarge != Kbd_ScanFaxKeybordConv.getChangeEnlKeybord() ) && ( False == onlyPanel.bl_isEnlargeDisplayMode ) ) {	// 通常モードならば.
							if( False == Kbd_ScanFaxKeybordConv.b_KeybordKohoSelect ) {	// 変換候補が未選択の場合.
								procButtonOn( 112 );	// 一番左上のキーを選択状態にする.
							}
							else {
								if( 0 != Kbd_ScanFaxKeybordConv.uc_OptionSelect % 3 ) {
									//Kbd_ScanFaxKeybordConv.uc_OptionSelect -= 1;
									Kbd_ScanFaxKeybordConv.uc_OptionSelect = static_cast<ushort>( Kbd_ScanFaxKeybordConv.uc_OptionSelect - 1 );
									procButtonOn( (uchar)(112 + Kbd_ScanFaxKeybordConv.uc_OptionSelect) ); // 一つ上のキーを選択状態にする.
								}
							}
						} else if( ( PLBD_Enl_KBD_Enlarge == Kbd_ScanFaxKeybordConv.getChangeEnlKeybord() ) && ( False == onlyPanel.bl_isEnlargeDisplayMode ) ) {	// 切り替え拡大モードならば.
							if( False == Kbd_ScanFaxKeybordConv.b_KeybordKohoSelect ) {	// 変換候補が未選択の場合.
								procButtonOn( 112 );	// 一番左上のキーを選択状態にする.
							}
							else {
								if( 4 <  Kbd_ScanFaxKeybordConv.uc_OptionSelect ) {
									//Kbd_ScanFaxKeybordConv.uc_OptionSelect -= 5;
									Kbd_ScanFaxKeybordConv.uc_OptionSelect = static_cast<ushort>( Kbd_ScanFaxKeybordConv.uc_OptionSelect - 5 );
									procButtonOn( (uchar)(112 + Kbd_ScanFaxKeybordConv.uc_OptionSelect) ); // 一つ上のキーを選択状態にする.
								}
							}
						} else if( ( PLBD_Enl_KBD_Enlarge == Kbd_ScanFaxKeybordConv.getChangeEnlKeybord() ) && ( True == onlyPanel.bl_isEnlargeDisplayMode ) )	{ // 拡大キーボードならば.
							if( ( False == Kbd_ScanFaxKeybordConv.b_KeybordKohoSelect ) && ( False == ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_EnlKeybordActConv )->b_KeybordKohoSelect ) ) {	// EnlargeDisp時に、PCGC_ConvertKeyBoardクラスのインスタンスを使用している画面がある為.
								procButtonOn( 112 );	// 一番左上のキーを選択状態にする.
							}
							else {
								if( pt_Kbd_EnlKeybordActConv && True == ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_EnlKeybordActConv )->b_KeybordConvertModeEnl ) { 	// PCGC_ConvertKeyBoardEnlを使用している場合.
									if( 4 < ( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect ) {
           								//( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect -= 5;
										( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect =  static_cast<ushort>( ( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect- 5 );
										procButtonOn( (uchar)(112 + ( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect) ); // 一つ左のキーを選択状態にする.
									}
								} else if( True == Kbd_ScanFaxKeybordConv.b_KeybordConvertMode ) {	 // PCGC_ConvertKeyBoardを使用している場合.
									if( 4 <  Kbd_ScanFaxKeybordConv.uc_OptionSelect ) {
										//Kbd_ScanFaxKeybordConv.uc_OptionSelect -= 5;
										Kbd_ScanFaxKeybordConv.uc_OptionSelect = static_cast<ushort>( Kbd_ScanFaxKeybordConv.uc_OptionSelect - 5 );
										procButtonOn( (uchar)(112 + Kbd_ScanFaxKeybordConv.uc_OptionSelect) ); // 一つ左のキーを選択状態にする.
									}
								}
							}
						}
					} else {
						procKeyBoard( 149 );
					}
					break;
				case TYPE_USBkey_Yajirusi_right:
					if( True == pc_PanelManager->getUSBKeyboardScrollLock() )
					{
						return;
					}
					if( ( True == Kbd_ScanFaxKeybordConv.b_KeybordConvertMode ) // 漢字変換画面の場合.
					    || ( True == ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_EnlKeybordActConv )->b_KeybordConvertModeEnl ) ) {
						if( ( PLBD_Enl_KBD_Enlarge != Kbd_ScanFaxKeybordConv.getChangeEnlKeybord() ) && ( False == onlyPanel.bl_isEnlargeDisplayMode ) ) {	// 通常モードならば.
							if( False == Kbd_ScanFaxKeybordConv.b_KeybordKohoSelect ) {	// 変換候補が未選択の場合.
								procButtonOn( 112 );	// 一番左上のキーを選択状態にする.
							}
							else {
								if( ( 2 !=  Kbd_ScanFaxKeybordConv.uc_OptionSelect % 3 ) &&
								    ( True == Key_Convert[ Kbd_ScanFaxKeybordConv.uc_OptionSelect + 1 ].isNowShowEnable() ) ) {
									//Kbd_ScanFaxKeybordConv.uc_OptionSelect += 1;
									Kbd_ScanFaxKeybordConv.uc_OptionSelect = static_cast<ushort>( Kbd_ScanFaxKeybordConv.uc_OptionSelect + 1 );
									procButtonOn( (uchar)(112 + Kbd_ScanFaxKeybordConv.uc_OptionSelect) ); // 一つ上のキーを選択状態にする.
								}
							}
						} else if( ( PLBD_Enl_KBD_Enlarge == Kbd_ScanFaxKeybordConv.getChangeEnlKeybord() ) && ( False == onlyPanel.bl_isEnlargeDisplayMode ) ) {	// 切り替え拡大モードならば.
							if( False == Kbd_ScanFaxKeybordConv.b_KeybordKohoSelect ) {	// 変換候補が未選択の場合.
								procButtonOn( 112 );	// 一番左上のキーを選択状態にする.
							}
							else {
								if( ( 5 > Kbd_ScanFaxKeybordConv.uc_OptionSelect ) &&
								    ( True == Key_EnlConvert[ Kbd_ScanFaxKeybordConv.uc_OptionSelect + 5 ].isNowShowEnable() ) ) {
									//Kbd_ScanFaxKeybordConv.uc_OptionSelect += 5;
									Kbd_ScanFaxKeybordConv.uc_OptionSelect = static_cast<ushort>( Kbd_ScanFaxKeybordConv.uc_OptionSelect + 5 );
									procButtonOn( (uchar)(112 + Kbd_ScanFaxKeybordConv.uc_OptionSelect) ); // 一つ上のキーを選択状態にする.
								}
							}
						} else if( ( PLBD_Enl_KBD_Enlarge == Kbd_ScanFaxKeybordConv.getChangeEnlKeybord() ) && ( True == onlyPanel.bl_isEnlargeDisplayMode ) )	{ // 拡大キーボードならば.
							if( ( False == Kbd_ScanFaxKeybordConv.b_KeybordKohoSelect ) && ( False == ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_EnlKeybordActConv )->b_KeybordKohoSelect ) ) {	// EnlargeDisp時に、PCGC_ConvertKeyBoardクラスのインスタンスを使用している画面がある為.
								procButtonOn( 112 );	// 一番左上のキーを選択状態にする.
							}
							else {
								if( pt_Kbd_EnlKeybordActConv && True == ( ( PLBC_ConvertKeybordEnl* )pt_Kbd_EnlKeybordActConv )->b_KeybordConvertModeEnl ) { 	// PCGC_ConvertKeyBoardEnlを使用している場合.
									if( ( 5 > ( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect ) && ( True == Key_EnlConvert[( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect + 5 ].isNowShowEnable() ) ) {
           								//( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect += 5;
										( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect = static_cast<ushort>( ( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect + 5 );
										procButtonOn( (uchar)(112 + ( ( PLBC_GeneralKeybord* )pt_Kbd_EnlKeybordActConv )->uc_OptionSelect) ); // 一つ左のキーを選択状態にする.
									}
								} else if( True == Kbd_ScanFaxKeybordConv.b_KeybordConvertMode ) {	 // PCGC_ConvertKeyBoardを使用している場合.
									if( ( 5 > Kbd_ScanFaxKeybordConv.uc_OptionSelect ) && ( True == Key_EnlConvert[ Kbd_ScanFaxKeybordConv.uc_OptionSelect + 5 ].isNowShowEnable() ) ) {
										//Kbd_ScanFaxKeybordConv.uc_OptionSelect += 5;
										Kbd_ScanFaxKeybordConv.uc_OptionSelect = static_cast<ushort>( Kbd_ScanFaxKeybordConv.uc_OptionSelect + 5 );
										procButtonOn( (uchar)(112 + Kbd_ScanFaxKeybordConv.uc_OptionSelect) ); // 一つ左のキーを選択状態にする.
									}
								}
							}
						}
					} else {
						procKeyBoard( 150 );
					}
					break;
				case TYPE_USBkey_Numlock:
					pc_PanelManager->setUSBKeyboardNumLock( !( pc_PanelManager->getUSBKeyboardNumLock() ) );// PanelManagerにモード変更通知を行う.
					pc_PanelManager->USBKeyLEDNotice( TYPE_KEYBDLED_INFO_NUM, pc_PanelManager->getUSBKeyboardNumLock() ); // USBキーボードキーのNUMLOCKランプ点灯・消灯処理を行う.
					break;
				case TYPE_USBkey_Scrolllock:
					pc_PanelManager->setUSBKeyboardScrollLock( !( pc_PanelManager->getUSBKeyboardScrollLock() ) );// PanelManagerにモード変更通知を行う.
					pc_PanelManager->USBKeyLEDNotice( TYPE_KEYBDLED_INFO_SCR, pc_PanelManager->getUSBKeyboardScrollLock() ); // USBキーボードキーのSCROLLLOCKランプ点灯・消灯処理を行う.
					break;
				case TYPE_USBkey_Backspace:
					if( ( TYPE_USBKeyBordGamen_Input != Kbd_ScanKeybordConv.e_USBKeyBordGamen )
					    || ( True == Kbd_ScanKeybordConv.b_KeybordConvertMode ) ) {
						return ;// USBキーボード文字入力画面以外は、文字入力を受け付けない.
					}
					procKeyBoard( 151 );// 一文字消去.
					break;

				case TYPE_USBkey_HankakuZenkaku:
					if( ( TYPE_USBKeyBordGamen_Input != Kbd_ScanKeybordConv.e_USBKeyBordGamen )
					    || ( True == Kbd_ScanKeybordConv.b_KeybordConvertMode ) ) {
						return ;// USBキーボード文字入力画面以外は、文字入力を受け付けない.
					}
					Kbd_ScanKeybordConv.setKeyBoardPartsNotDispFlag( False );
					if( ( TYPE_LanguageJapanese == PnlComGetUSBKeyboardLanguage() )// 現在がひらがな、カタカナ入力の場合は英数(半)へ切り替える.
					    && ( ( TYPE_USBKeyInput_Country == pc_PanelManager->getUSBKeyBoardCountry() )
					         || ( TYPE_USBKeyInput_Katakana == pc_PanelManager->getUSBKeyBoardCountry() ) ) ) {
						if( ( True == onlyPanel.bl_isEnlargeDisplayMode ) || ( PLBD_Enl_KBD_Enlarge == Kbd_ScanKeybordConv.getChangeEnlKeybord() ) ) {
							procButtonOn( 224 );	// EnlargeDisp、拡大表示の場合.
						} else {
							procButtonOn( 233 );	// 通常画面の場合.
						}
					} else if( ( TYPE_LanguageJapanese == PnlComGetUSBKeyboardLanguage() )// ひらがなキーが有り、英数入力時である場合はひらがな入力へ切り替える.
					           && ( True == Kbd_ScanKeybordConv.bl_Key_KeybordcomLanguage )
					           && ( TYPE_USBKeyInput_English == pc_PanelManager->getUSBKeyBoardCountry() ) ) {
						if( ( True == onlyPanel.bl_isEnlargeDisplayMode ) || ( PLBD_Enl_KBD_Enlarge == Kbd_ScanKeybordConv.getChangeEnlKeybord() ) ) {
							procButtonOn( 232 );	// EnlargeDisp、拡大表示中の場合.
						} else {
							procButtonOn( 224 );	// 通常画面の場合.
						}
					}
					break;
				case TYPE_USBkey_Oomoji:
					if( ( TYPE_USBKeyBordGamen_Input != Kbd_ScanKeybordConv.e_USBKeyBordGamen )
					    || ( True == Kbd_ScanKeybordConv.b_KeybordConvertMode ) ) {
						return ;// USBキーボード文字入力画面以外は、文字入力を受け付けない.
					}
					Kbd_ScanKeybordConv.setKeyBoardPartsNotDispFlag( False );
					procButtonOn( 223 );// CapsLock(大文字へ切替).
//2019/03/05 HCL_Nandhini [IT6_MR] AR1071414 - USB keyboard CAP Lock ON/OFF setting issue.
					pc_PanelManager->setUSBKeyBoardOomojiKomoji( TYPE_USBKeyInput_Oomoji ); //PanelManagerへモード変更通知
					pc_PanelManager->USBKeyLEDNotice( TYPE_KEYBDLED_INFO_CAP , True ); // USBキーボードキーのCapsLockランプ点灯要求処理を行う.
					break;
				case TYPE_USBkey_Komoji:
					if( ( TYPE_USBKeyBordGamen_Input != Kbd_ScanKeybordConv.e_USBKeyBordGamen )
					    || ( True == Kbd_ScanKeybordConv.b_KeybordConvertMode ) ) {
						return ;// USBキーボード文字入力画面以外は、文字入力を受け付けない.
					}
					Kbd_ScanKeybordConv.setKeyBoardPartsNotDispFlag( False );
					procButtonOn( 223 );// CapsLock(小文字へ切替).
//2019/03/05 HCL_Nandhini [IT6_MR] AR1071414 - USB keyboard CAP Lock ON/OFF setting issue.
					pc_PanelManager->setUSBKeyBoardOomojiKomoji( TYPE_USBKeyInput_Komoji ); //PanelManagerへモード変更通知
					pc_PanelManager->USBKeyLEDNotice( TYPE_KEYBDLED_INFO_CAP , False ); // USBキーボードキーのCapsLockランプ消灯処理を行う.
					break;
				case TYPE_USBkey_Kana:
					if( ( TYPE_USBKeyBordGamen_Input != Kbd_ScanKeybordConv.e_USBKeyBordGamen )
					    || ( True == Kbd_ScanKeybordConv.b_KeybordConvertMode ) ) {
						return ;// USBキーボード文字入力画面以外は、文字入力を受け付けない.
					}
					Kbd_ScanKeybordConv.setKeyBoardPartsNotDispFlag( False );
					if( ( True == onlyPanel.bl_isEnlargeDisplayMode ) || ( PLBD_Enl_KBD_Enlarge == Kbd_ScanKeybordConv.getChangeEnlKeybord() ) ) {
						procButtonOn( 243 );// EnlargeDisp、拡大表示中の場合.
					} else {
						procButtonOn( 236 );// 通常画面の場合.
					}

					break;
				case TYPE_USBkey_Roma:
					if( ( TYPE_USBKeyBordGamen_Input != Kbd_ScanKeybordConv.e_USBKeyBordGamen )
					    || ( True == Kbd_ScanKeybordConv.b_KeybordConvertMode ) ) {
						return ;// USBキーボード文字入力画面以外は、文字入力を受け付けない.
					}
					Kbd_ScanKeybordConv.setKeyBoardPartsNotDispFlag( False );
					if( ( True == onlyPanel.bl_isEnlargeDisplayMode ) || ( PLBD_Enl_KBD_Enlarge == Kbd_ScanKeybordConv.getChangeEnlKeybord() ) ) {
						procButtonOn( 243 );// EnlargeDisp、拡大表示中の場合.
					} else {
						procButtonOn( 236 );// 通常画面の場合.
					}

					break;
				case TYPE_USBkey_Katakana:
					if( ( TYPE_USBKeyBordGamen_Input != Kbd_ScanKeybordConv.e_USBKeyBordGamen )
					    || ( True == Kbd_ScanKeybordConv.b_KeybordConvertMode ) ) {
						return ;// USBキーボード文字入力画面以外は、文字入力を受け付けない.
					}

					Kbd_ScanKeybordConv.setKeyBoardPartsNotDispFlag( False );

					pc_PanelManager->setUSBKeyBoardZenkakuHankaku( TYPE_USBKeyInput_Hankaku ); // USBキーからのカタカナへの切替時は半角にする.
					if( ( TYPE_LanguageJapanese == PnlComGetUSBKeyboardLanguage() )	// ひらがなローマ字入力中.
					    && ( TYPE_USBKeyInput_Country == pc_PanelManager->getUSBKeyBoardCountry() )
					    && ( TYPE_USBKeyInput_Roma == pc_PanelManager->getUSBKeyBoardRomaKana() ) ) {
						if( ( True == onlyPanel.bl_isEnlargeDisplayMode ) || ( PLBD_Enl_KBD_Enlarge == Kbd_ScanKeybordConv.getChangeEnlKeybord() ) ) {
							procButtonOn( 233 );	// カタカナ入力へ切替.
							Kbd_ScanFaxKeybordConv.b_KeybordCharZenkakuEnl = False;
							Kbd_ScanFaxKeybordConv.b_KeybordHalfCharEnl = True;
						} else {
							procButtonOn( 225 );	// カタカナ入力へ切替.
							Kbd_ScanFaxKeybordConv.b_KeybordCharZenkakuMode = False;
							Kbd_ScanFaxKeybordConv.b_KeybordHalfChar = True;
						}
					} else {	// ひらがな・かな入力中.
						if( ( True == onlyPanel.bl_isEnlargeDisplayMode ) || ( PLBD_Enl_KBD_Enlarge == Kbd_ScanKeybordConv.getChangeEnlKeybord() ) ) {
							procButtonOn( 233 );// EnlargeDisp、拡大表示中の場合.
						} else {
							procButtonOn( 225 );// 通常画面の場合.
						}
					}

					break;
				case TYPE_USBkey_Hiragana:
					if( ( TYPE_USBKeyBordGamen_Input != Kbd_ScanKeybordConv.e_USBKeyBordGamen )
					    || ( True == Kbd_ScanKeybordConv.b_KeybordConvertMode ) ) {
						return ;// USBキーボード文字入力画面以外は、文字入力を受け付けない.
					}


					Kbd_ScanKeybordConv.setKeyBoardPartsNotDispFlag( False );
					if( ( True == onlyPanel.bl_isEnlargeDisplayMode ) || ( PLBD_Enl_KBD_Enlarge == Kbd_ScanKeybordConv.getChangeEnlKeybord() ) ) {
						procButtonOn( 232 );	// ひらがな入力へ切替.
					} else {
						procButtonOn( 224 );	// ひらがな入力へ切替.
					}
					break;
				default: break;
			}
		}
	}
}
//================================================================================================
//概要    :画面毎の表示部品テーブルを取得する.
//備考    :.
//================================================================================================
PDisplayParts** PBasicMessageProc::getPartsTable() {
	return partsTable;
}
//================================================================================================
//概要    :画面毎の表示部品テーブルを取得する.
//備考    :.
//================================================================================================
short PBasicMessageProc::getPartsNumber() {
	return partsNumber;
}
//================================================================================================
//概要    :USBキーボードプラグイン情報検知処理.
//備考    :.
//================================================================================================
void PBasicMessageProc::procUSBKeyBoardInsertState( uchar option )

{

	static Bool	bl_WarningDispON = False;	// 警告を表示済みかのフラグ.	True:表示済み.
#ifdef QT_PANEL
	static TYPE_UsbKbdStateInfo	 e_UsbKbdStatusInfoBack = TYPE_UsbKbdStateInfo_None;			// USBキーボード接続状態退避用.
#endif

#if defined(SIMULATOR) && !defined(LINUX)
	if( False == m_hWndView->b_PanelDebugUSBOn )
#else
	if( TYPE_UsbKbdStateInfo_None == PnlComGetMachineSetting()->getUsbStatus() )
#endif
	{
		if( True == onlyPanel.b_USBKeyBoardMode
		    && False == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_USBKeyBoardDisconnected )
		    && False == bl_WarningDispON )
		{
			// キーボードパーツなし外部キーボード入力値取得画面では警告は発生させない為.
			// キーボードパーツなし外部キーボード入力値取得画面のチェックを行い.
			// 外部キーボードが取得できる状態かのチェックは行わない.
			if( False == kbd_NoPartsUSBKeybord.chkNoKeybordGamen( False ) ) {
				onlyPanel.setPanelWarning( PNL_ERROR_USBKeyBoardDisconnected );
				bl_WarningDispON = True;
			}
			PnlComGetMachineSetting()->setDisableUsbKbd();
		}

		// 外付けキーボードキーのNumLock,CapsLockとScrollLockランプ消灯処理を行う.
		pc_PanelManager->USBKeyLEDNoticeAll( False );

	}
	else
	{
		bl_WarningDispON = False;
		if( True == onlyPanel.b_USBKeyBoardMode )
		{
			if( True == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_USBKeyBoardDisconnected ) ) {
				onlyPanel.resetPanelWarning( PNL_ERROR_USBKeyBoardDisconnected );
			}
			PnlComGetMachineSetting()->setEnableUsbKbd();
		}
		// キーボードパーツなし画面で外部キーボードの入力を受付ける画面の場合.
		else if( True == kbd_NoPartsUSBKeybord.chkNoKeybordGamen( True ) ) {
			onlyPanel.b_USBKeyBoardMode = True;
		}

		TYPE_PowerSaveStatus e_PowerSaveStatus = ppc_Power->getPowerSaveStatus();
		if( True == PnlComIsPowerSaveMode( e_PowerSaveStatus ) )
		{
			// パワーセーブモード時（バックライトOFF時）.
#if defined(SIMULATOR) && !defined(LINUX)
			if( True == m_hWndView->b_PanelDebugUSBOn )
#else
			if( TYPE_UsbKbdStateInfo_One == PnlComGetMachineSetting()->getUsbStatus() )
#endif
			{
				// 外付けキーボードキーのNumLock,CapsLockとScrollLockランプ消灯処理を行う.
				pc_PanelManager->USBKeyLEDNoticeAll( False );
			}
		}
	}
#if defined (QT_PANEL) && defined(DEF_PANELDRV)
	// USB接続状態の変化あり.
	if( PnlComGetMachineSetting()->getUsbStatus() != e_UsbKbdStatusInfoBack ) {
		PLBC_PanelLcdDraw::getQtPanelInstance()->initUsbKbd();				// USBキーボードキー入力データの初期化.
		e_UsbKbdStatusInfoBack = PnlComGetMachineSetting()->getUsbStatus();	// 前回のUSBキーボード接続状態を更新.
	}
#endif
	PNLC_KeyboardControlLib::newInstance()->setUSBKeyboardStatus( PnlComGetMachineSetting()->getUsbStatus() );
}
#if defined( DEF_SimAutoTest ) || defined (DEF_SimEnableScreenId) //Modified by HCL for PanelTransition (23-Dec-2010)		// 自動評価で有効. 
//================================================================================================
//概要    :キーNo.設定.
//備考    :自動評価用　SimWinへキーNo.を通知する為、必要なキーNo.の設定を行う.
//作成者  :鈴木希.
//レビュー:
//変更履歴:
//================================================================================================
void PBasicMessageProc::setAutoTestKeyNoNotice( Message message, uchar option )
{
	switch( message )
	{
		// タッチパネル内の画面操作メッセージ.
		case MS_IndexButton:					// インデクスボタン.
		case MS_GoToPage:						// 画面移動.
		case MS_Ok:								// 設定終了.
		case MS_Cancel:							// 設定取消.
		case MS_End:							// 終了キー.
		case MS_GoToPaper:						// 用紙画面移動（コピー）.
		case MS_GoToMag:						// 倍率画面移動（コピー）.
		case MS_GoToFace:						// 片面/両面画面移動（コピー）.
		case MS_Button:							// 通常ボタン.
		case MS_KeyBoard:						// キーボード上のボタン.
		case MS_GoToScanColor:					// カラー／下地への遷移（スキャナ）.
		case MS_GoToScanResolution:				// 解像度画面への遷移（スキャナ）.
		case MS_GoToScanColorLevel:				// 下地除去画面への遷移（スキャナ）.
		case MS_GoToScanScanSize:				// 読込サイズ画面への遷移（スキャナ）.
		// ワンタッチキーメッセージの追加.
		case MS_OneTouchKey:					// ワンタッチキー.
		// ボックスのメッセージ追加.
		case MS_BoxUp:							// ボックス画面ページアップ.
		case MS_BoxDown:						// ボックス画面ページダウン.
		case MS_BoxIndex:						// ドキュメント一覧インデックス.
		case MS_BoxButton:						// ボックスボタン.
		case MS_DocButton:						// ドキュメント一覧リストボタン.
		//ジョブリスト用.
		case MS_JobUp:							// ジョブリストのページを進める.
		case MS_JobDown:						// ジョブリストのページを戻す.
		case MS_JobDelete:						// ジョブ消去キー.
		case MS_JobIndex:						// ジョブ操作インデックスキー.
		case MS_JobControl:						// ジョブ操作に入る.
		case MS_JobButton:						// ジョブリストのジョブ選択キー.
		// ダイレクトキー操作によるメッセージ.
		case MS_Reset:							// パネルリセット.
		case MS_Start:							// スタートキー.
		case MS_Stop:							// ストップ.
		case MS_10Key:							// テンキー.
		case MS_Clear:							// クリアー.
		case MS_ColorMode:						// カラーモード.
		case MS_IntMode:						// 割り込みモード.
		case MS_IDInput:						// 暗証番号入力.
		case MS_UserMode:						// ユーザーモード.
		case MS_ModeCheck:						// 設定確認.
		case MS_Service:						// サービスマンモード.
		case MS_Copy:							// コピーアプリキー.
		case MS_Scan:							// スキャナアプリキー.
		case MS_EnlargeDsp:						// 拡大表示キー .
		// ダイレクトキーメッセージの追加.
		case MS_Enlarge:						// 拡大表示キー.
		case MS_SampleCopy:						// 確認コピー.
		case MS_JobMemory:						// ジョブメモリ.
		case MS_Utility:						// ユーティリティ.
		case MS_Fax:							// FAX.
		case MS_FaxOnly:						// ファクス専用.
		case MS_Box:							// ボックス.
		case MS_PowerSave:						// パワーセーブ.
		case MS_Help:							// ヘルプ.
		case MS_Accessibility:					// ユーザー補助.
// comment 2015/04/30 OPS1-Sky佐藤 [HeliosL][CQ513137] ズームキーの登録キー割り当て対応.
		case MS_ZoomIn:							// 表示ズーム.
		//小ＬＣＤ用.
		case MS_EnergySaveReset:				// 省エネ解除.
		case MS_Program:						// プログラム呼出.
		// サービス基板からの入力.
		case MS_TroubleReset:					// トラブルリセット.
		case MS_Initial:						// イニシャル.
		case MS_MemoryClear:					// メモリクリア.
		case MS_TotalClear:						// トータルクリア.
		// その他.
		case MS_PreHeatOnByKeyAns:				// プレヒートキー押下応答.
		case MS_Return:							// 戻る（上の階層に戻る）.
		case MS_Back:							// 戻る.
		case MS_Execute:						// 実行.
		case MS_LeftIndex:						// 左エリア・インデックスキー.
		case MS_BookMark:						// 左エリア・ブックマークキー（しおりキー）.
		case MS_LeftSettingGoTo:				// 左エリア・設定内容画面遷移キー.
		case MS_LeftSettingButton:				// 左エリア・設定内容通常キー.
		case MS_LeftSettingList:				// 左エリア・設定内容リストキー.
		case MS_LeftPreviewGoTo:				// 左エリア・プレビュー画面遷移キー.
		case MS_LeftPreviewButton:				// 左エリア・プレビュー通常キー.
		case MS_LeftPreviewList:				// 左エリア・プレビューリストキー.
		case MS_PreviewDetail:					// プレビュー詳細.
		case MS_ShortCutKey:					// ショートカットキー押下.
		case MS_BadgeButton:					// 機能バッジ押下.
			onlyPanel.ss_NoticeKeyNo = ( ( ( message << 0x8 ) & 0xff00 ) | option );
			break;
		default:
			break;
	}
}
#endif	// #if defined( DEF_SimAutoTest ) || defined (DEF_SimEnableScreenId) //Modified by HCL for PanelTransition (23-Dec-2010)		// 自動評価で有効. 

/*-----------------------------------------------------------------------------------------------------------
	 外部参照関数
-----------------------------------------------------------------------------------------------------------*/
static ushort us_LedPrintRed = PNLE_LEDStateOff;								// プリント赤.

#if defined(DEF_THAMES_ICUSTOM) && defined(LINUX)
static ushort ledData[][2] = {
	{ ( ushort )PNLE_LED_IntMode,	( ushort )PNLE_LEDStateOff },
	{ ( ushort )PNLE_LED_Scaner,	( ushort )PNLE_LEDStateOff },
	{ ( ushort )PNLE_LED_Copy,	( ushort )PNLE_LEDStateOff },
	{ ( ushort )PNLE_LED_EnlargeDisp,	( ushort )PNLE_LEDStateOff },
	{ ( ushort )PNLE_LED_Box,	( ushort )PNLE_LEDStateOff },
	{ ( ushort )PNLE_LED_Fax,	( ushort )PNLE_LEDStateOff },
	{ ( ushort )PNLE_LED_Data,	( ushort )PNLE_LEDStateOff },
	{ ( ushort )PNLE_LED_PrintGreen,	( ushort )PNLE_LEDStateOff },
	{ ( ushort )PNLE_LED_PrintRed,	( ushort )PNLE_LEDStateOff },
	{ ( ushort )PNLE_LED_Accessibility,( ushort )PNLE_LEDStateOff },
	{ ( ushort )PNLE_LED_VoiceGuide,	( ushort )PNLE_LEDStateOff },			// [CQ.342958]音声ガイドLEDを追加.
	{ ( ushort )PNLE_LED_ImageDealer,	( ushort )PNLE_LEDStateOff },
	{ ( ushort )PNLE_LED_MyPanel,	( ushort )PNLE_LEDStateOff },
	{ ( ushort )PNLE_LED_Browser,	( ushort )PNLE_LEDStateOff },
	{ ( ushort )PNLE_LED_AppMenu,	( ushort )PNLE_LEDStateOff },
	{ ( ushort )PNLE_LED_Enforcement,	( ushort )PNLE_LEDStateOff },
	{ ( ushort )PNLE_LED_AppSingle,	( ushort )PNLE_LEDStateOff },
};
/*================================================================================================
［機能］ 外部参照関数 : 実際のLED情報とパネルで保持しているLED情報をチェックする。
================================================================================================*/
void checkLED() {
	TYPE_ApplicationKeyId	e_ApplicationKeyId[4];	// アプリ情報取得バッファ.
	PNLE_LEDKind us_ledpos;
	Bool flg = False;

	TYPS_ApplicationKeySetting ps_ApplicationKeyId;
	memset( &ps_ApplicationKeyId, 0x00, sizeof( ps_ApplicationKeyId ) );

	// アプリケーションキーの状態を取得する（取得できない場合は終了）.
	if( ( PnlComGetApplicationKeySetting()->getApplicationKeyId( &ps_ApplicationKeyId ) ) != TYPE_AppKeyResultOK )
	{
		return;
	}

	memcpy( e_ApplicationKeyId, &( ps_ApplicationKeyId.uc_ApplicationKeyId ), sizeof( e_ApplicationKeyId ) );

	// アプリ割付とＬＥＤの対応マップを作成する.
	ushort us_convMap[][2] = {
		{ ( ushort )TYPE_ApplicationKeyIdCopy,	( ushort )PNLE_LED_Copy },
		{ ( ushort )TYPE_ApplicationKeyIdFaxScan,	( ushort )PNLE_LED_Fax },
		{ ( ushort )TYPE_ApplicationKeyIdBox,	( ushort )PNLE_LED_Box },
		{ ( ushort )TYPE_ApplicationKeyIdImageDealer,	( ushort )PNLE_LED_ImageDealer },
		{ ( ushort )TYPE_ApplicationKeyIdMyPanel,	( ushort )PNLE_LED_MyPanel },
		{ ( ushort )TYPE_ApplicationKeyIdBrowser,	( ushort )PNLE_LED_Browser },
		{ ( ushort )TYPE_ApplicationKeyIdApplicationMenu,	( ushort )PNLE_LED_AppMenu },
		{ ( ushort )TYPE_ApplicationKeyIdAppSingle,	( ushort )PNLE_LED_AppSingle }
	};

	// テーブル中のパネル内部定義と照合、commonAPI定義を検出.
	for( uchar uc_key1=0; uc_key1<7; uc_key1++ )
	{
		// commonAPI定義内容とアプリケーションキーの設定から定義されているか検査.
		for( uchar uc_key2=0; uc_key2<sizeof( e_ApplicationKeyId ); uc_key2++ )
		{
			if( e_ApplicationKeyId[uc_key2] ==  us_convMap[uc_key1][0] ) {
				flg = True;
				break;
			}
		}
		if( flg ) {
			// 検出位置に応じたＬＥＤを指定する.
			if( uc_key2 == 0 ) { us_ledpos = PNLE_LED_Box; }	// アプリキー０番目.
			else if( uc_key2 == 1 ) { us_ledpos = PNLE_LED_Fax; }	// アプリキー１番目.
			else if( uc_key2 == 2 ) { us_ledpos = PNLE_LED_Copy; }	// アプリキー２番目.
			else { us_ledpos = 0; }

			if( us_ledpos != 0 ) {
				// 実際のLED状態取得
				PNLE_LEDState i_LEDState = pc_PanelCpuIF->getLED( us_ledpos );	// LED設定
				if( i_LEDState == PNLE_LedStateNone ) {
					return;
				}
				uchar uc_Size = ( uchar )( sizeof( ledData ) / sizeof( ledData[0] ) );
				uchar i;
				// LED状態に差異があれば更新
				for( i = 0; uc_Size > i; i++ ) {
					if( us_convMap[uc_key1][1] == ledData[i][0] ) {
						if( i_LEDState != ledData[i][1] ) {
							ledData[i][1] = i_LEDState;
						}
					}
				}
			}
		}
	}
}
#endif // #if defined(DEF_THAMES_ICUSTOM) && defined(LINUX)
/*================================================================================================
［機能］ 外部参照関数 : ＬＥＤのＯＮ／ＯＦＦを行う
［履歴］2013/12/17	Sky酒井(直)	ZeusCD UV-01387(リモートパネルの改善).
			ZeusCD以降は、LCDの状態変更をRMPへ通知しない.
================================================================================================*/
void reqLED( PNLE_LEDKind led, PNLE_LEDState e_LEDState, Bool bSlide )
{
	if( False == APIC_SystemInfo::newInstance()->isSlidePanel() ) {
		//スライドレスパネルの場合.
		if( True == bSlide ) {
			// 従来処理からの呼び出しの場合.
			if( ( led == PNLE_LED_Menu_SlidePanel )		// PNLE_LED_Copy.
			    || ( led == PNLE_LED_Guidance_SlidePanel )	// PNLE_LED_Fax.
			  ) {
				// PNLE_LED_Menu_SlidePanel、PNLE_LED_Guidance_SlidePanelと同じ値となっているため、
				// ここで処理を終了させる
				return;
			}
		} else {
			// スライドレス処理からの呼び出しの場合.
			// 何も処理しない.
		}
	}
	PNLE_LEDKind e_wkLED = led;
#if !defined(LINUX)
	static ushort ledData[][2] = {
		{ ( ushort )PNLE_LED_IntMode,	( ushort )PNLE_LEDStateOff },
		{ ( ushort )PNLE_LED_Scaner,	( ushort )PNLE_LEDStateOff },
		{ ( ushort )PNLE_LED_Copy,	( ushort )PNLE_LEDStateOff },
		{ ( ushort )PNLE_LED_EnlargeDisp,	( ushort )PNLE_LEDStateOff },
		{ ( ushort )PNLE_LED_Box,	( ushort )PNLE_LEDStateOff },
		{ ( ushort )PNLE_LED_Fax,	( ushort )PNLE_LEDStateOff },
		{ ( ushort )PNLE_LED_Data,	( ushort )PNLE_LEDStateOff },
		{ ( ushort )PNLE_LED_PrintGreen,	( ushort )PNLE_LEDStateOff },
		{ ( ushort )PNLE_LED_PrintRed,	( ushort )PNLE_LEDStateOff },
		{ ( ushort )PNLE_LED_Accessibility,( ushort )PNLE_LEDStateOff },
#ifdef	  DEF_THAMES_ICUSTOM
		{ ( ushort )PNLE_LED_ImageDealer,	( ushort )PNLE_LEDStateOff },
		{ ( ushort )PNLE_LED_MyPanel,	( ushort )PNLE_LEDStateOff },
		{ ( ushort )PNLE_LED_Browser,	( ushort )PNLE_LEDStateOff },
		{ ( ushort )PNLE_LED_AppMenu,	( ushort )PNLE_LEDStateOff },
		{ ( ushort )PNLE_LED_Enforcement,	( ushort )PNLE_LEDStateOff },
		{ ( ushort )PNLE_LED_AppSingle,	( ushort )PNLE_LEDStateOff },
	};
#endif
#endif	// #if !defined(LINUX)	
#ifdef	  DEF_THAMES_ICUSTOM
	TYPE_ApplicationKeyId	e_ApplicationKeyId[4];	// アプリ情報取得バッファ.

	TYPS_ApplicationKeySetting ps_ApplicationKeyId;
	memset( &ps_ApplicationKeyId, 0x00, sizeof( ps_ApplicationKeyId ) );

	// アプリ割付とＬＥＤの対応マップを作成する.
	ushort us_convMap[][2] = {
		{ ( ushort )TYPE_ApplicationKeyIdCopy,	( ushort )PNLE_LED_Copy },
		{ ( ushort )TYPE_ApplicationKeyIdFaxScan,	( ushort )PNLE_LED_Fax },
		{ ( ushort )TYPE_ApplicationKeyIdBox,	( ushort )PNLE_LED_Box },
		{ ( ushort )TYPE_ApplicationKeyIdImageDealer,	( ushort )PNLE_LED_ImageDealer },
		{ ( ushort )TYPE_ApplicationKeyIdMyPanel,	( ushort )PNLE_LED_MyPanel },
		{ ( ushort )TYPE_ApplicationKeyIdBrowser,	( ushort )PNLE_LED_Browser },
		{ ( ushort )TYPE_ApplicationKeyIdApplicationMenu,	( ushort )PNLE_LED_AppMenu },
		{ ( ushort )TYPE_ApplicationKeyIdAppSingle,	( ushort )PNLE_LED_AppSingle }
	};
	if( ( e_wkLED & PNLE_LED_Enforcement ) == 0 ) {
		// アプリケーションキーの状態を取得する（取得できない場合は終了）.
		if( ( PnlComGetApplicationKeySetting()->getApplicationKeyId( &ps_ApplicationKeyId ) ) != TYPE_AppKeyResultOK ) {
			return;
		}

		memcpy( e_ApplicationKeyId, &( ps_ApplicationKeyId.uc_ApplicationKeyId ), sizeof( e_ApplicationKeyId ) );

		// テーブル中のパネル内部定義と照合、commonAPI定義を検出.
		uchar uc_key1=0;
		for( uc_key1=0; uc_key1<7; uc_key1++ ) {
			if( us_convMap[uc_key1][1] == e_wkLED ) { break; }
		}
		// commonAPI定義内容とアプリケーションキーの設定から定義されているか検査.
		uchar uc_key2=0;
		for( uc_key2=0; ( uc_key2<sizeof( e_ApplicationKeyId )&&( uc_key1 < ( sizeof( us_convMap )/sizeof( us_convMap[0] ) ) ) ); uc_key2++ ) {

			if( e_ApplicationKeyId[uc_key2] ==  us_convMap[uc_key1][0] ) { break; }
		}
		// ledData表からパラメタで指定されている機能を検索.
		for( uchar uc_key3=0; uc_key3<16; uc_key3++ ) {
			if( ledData[uc_key3][0] == e_wkLED ) { break; }
		}
	} else {
		// アプリ割付判断の為のみの情報なので判定が終了したら消しておく.
		e_wkLED = ( PNLE_LEDKind )( e_wkLED - PNLE_LED_Enforcement );
	}
#else	// #ifdef	  DEF_THAMES_ICUSTOM
};
#endif // DEF_THAMES_ICUSTOM	// アプリ割付.
	uchar uc_Size = ( uchar )( sizeof( ledData ) / sizeof( ledData[0] ) );
	Bool b_Result = False;

	uchar i;	// Qt4対応:VC固有スコープ処理を回避
	for( i = 0; uc_Size > i; i++ ) {
		if( ledData[i][0] == ( ushort )e_wkLED ) {
			if( ledData[i][1] != ( ushort )e_LEDState ) {
				b_Result = True;
			} else {
#ifndef SIMULATOR
				// DataLEDの要求を行わない(パネルのステータスと要求内容が同じ)場合でも、.
				// 他モジュールで変更されている場合はパネルのステータスで再要求を行う.
				if( PNLE_LED_Data == ( short )e_wkLED ) {
					Bool b_LED_State = CNVC_PanelHard::newInstance()->getLED( LED_Data );
					if( ( True  == b_LED_State && PNLE_LedStateOff == ledData[i][1] )
					    || ( False == b_LED_State && PNLE_LedStateOn  == ledData[i][1] ) ) {
						// LED点灯中で消灯要求の場合か、LED消灯中で点灯要求の場合.
						b_Result = True;	// 要求を行う.
					}
				}
				else if( PNLE_LED_PrintGreen == ( short )e_wkLED ) {
					Bool b_LED_State = CNVC_PanelHard::newInstance()->getLED( LED_PrintGreen );
					if( ( True  == b_LED_State && PNLE_LedStateOff == ledData[i][1] )
					    || ( False == b_LED_State && PNLE_LedStateOn  == ledData[i][1] ) ) {
						// LED点灯中で消灯要求の場合か、LED消灯中で点灯要求の場合.
						b_Result = True;	// 要求を行う.
					}
				}
				else if( PNLE_LED_PrintRed   == ( short )e_wkLED ) {
					Bool b_LED_State = CNVC_PanelHard::newInstance()->getLED( LED_PrintRed );
					if( ( True  == b_LED_State && PNLE_LedStateOff == ledData[i][1] )
					    || ( False == b_LED_State && PNLE_LedStateOn  == ledData[i][1] ) ) {
						// LED点灯中で消灯要求の場合か、LED消灯中で点灯要求の場合.
						b_Result = True;	// 要求を行う.
					}
				}
#endif
			}
			break;
		}
	}
	if( b_Result ) {
		ledData[i][1] = ( ushort )e_LEDState;
		if( ledData[i][0] == PNLE_LED_PrintRed ) {						// プリント赤.
			us_LedPrintRed = ledData[i][1];
		}
		// ＬＥＤ状態が変化する時
		pc_PanelManager->reqLED( PM_TASKID_PANEL, e_wkLED, e_LEDState );	// LED点灯／消灯処理.

		APIC_RemotePanelSetting* pc_RMPSetting = PnlComGetRemotePanelSetting();
		RMP_WebDAVClient* pc_RMPClient = RMP_WebDAVClient::newInstance();
		if( NULL == pc_RMPSetting || NULL == pc_RMPClient ) {
			PnlComPutDebugLog( False, UC_PNL_LOGPATTERN_1, "reqLED", UC_PNL_ERROR_IMPOSSIBLE );
			return;
		}

		if( pc_RMPSetting->getRemoteStatus() == TYPE_RemoteStatusClient ) {
			if( True == PNLC_RMPStrInputMng::newInstance()->getNowKbd() ) {		// キーボード画面.
				if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type5() ) {
					pc_RMPClient->InfoStatusChanged( RMP_LED_CHANGED );			// LED状態変化あり.
					// QtLibへテキスト入力できることを通知する.
					PNLC_RMPQtConnector::newInstance()->textInputEnabled( True );
				} else {
					pc_RMPClient->InfoStatusChanged( RMP_TXT_LED_CHANGED );		// LCD(テキスト入力可)&LED状態変化あり.
				}
			} else {
				pc_RMPClient->InfoStatusChanged( RMP_LED_CHANGED );				// LED状態変化あり.
				if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type5() ) {
					// QtLibへテキスト入力できないことを通知する.
					PNLC_RMPQtConnector::newInstance()->textInputEnabled( False );
				}
			}
		}

	}
}
/*================================================================================================
［機能］ 外部参照関数 : ＬＥＤ輝度設定.
================================================================================================*/
void reqLEDBrightness( uchar uc_LEDLevel )
{
	static uchar ledLevel = 0;
	if( 100 >= uc_LEDLevel ) {
		if( ledLevel != uc_LEDLevel ) {
			// ＬＥＤ輝度設定が変化した時.
			pc_PanelCpuIF->setLEDBrightness( uc_LEDLevel );	// LED輝度レベル設定.
			ledLevel = uc_LEDLevel;
		}
	} else { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed! - reqLEDBrightness Range Over -" ); }
}
/*================================================================================================
［機能］ 外部参照関数 : ビープ（ブザー）音のＯＮ／ＯＦＦを行う
================================================================================================*/
void reqBuzzer( Bool bl_On, EP_BuzzerType ep_BuzzerType )
{
}
/*================================================================================================
	Rhein3用画面への遷移テーブル＆遷移先検索.
================================================================================================*/
PBasicMessageProc*	replaceGamen( PBasicMessageProc* nextGamenBaseModel )
{
	PBasicMessageProc*	nextGamenSpecifyModel;
	nextGamenSpecifyModel = nextGamenBaseModel;
	short tableCount = 0;
	if( ( PnlComGetPrinterSystem()->getFinisherType() == TYPE_FinisherTypeCB ) || ( TYPE_FinisherTypeFS527 == PnlComGetPrinterSystem()->getFinisherType() ) ) {
		while( changegamenCBFNTable[tableCount].pc_nextGamen != NULL ) {
			if( changegamenCBFNTable[tableCount].pc_nextGamen == nextGamenBaseModel ) {
				nextGamenSpecifyModel = changegamenCBFNTable[tableCount].pc_changeGamen;
				break;
			}
			tableCount++;
		}
		if( nextGamenSpecifyModel == nextGamenBaseModel ) {
			// utility/serviceの置き換えTableをサーチする.
			tableCount = 0;
			STRUCT_ReplaceGamenTable* p_changeTable = NULL;
			p_changeTable = ( STRUCT_ReplaceGamenTable* )pt_utilityChangegamenCBFNTable;
			while( p_changeTable[tableCount].pc_nextGamen != NULL ) {
				if( p_changeTable[tableCount].pc_nextGamen == nextGamenBaseModel ) {
					nextGamenSpecifyModel = p_changeTable[tableCount].pc_changeGamen;
					break;
				}
				tableCount++;
			}
		}
		if( nextGamenSpecifyModel == nextGamenBaseModel ) {
			// 拡大表示/helpの置き換えTableをサーチする.
			tableCount = 0;
			STRUCT_ReplaceGamenTable* p_changeTable = NULL;
			p_changeTable = ( STRUCT_ReplaceGamenTable* )pt_enlhelpChangegamenCBFNTable;
			while( p_changeTable[tableCount].pc_nextGamen != NULL ) {
				if( p_changeTable[tableCount].pc_nextGamen == nextGamenBaseModel ) {
					nextGamenSpecifyModel = p_changeTable[tableCount].pc_changeGamen;
					break;
				}
				tableCount++;
			}
		}
		// CB-FN用のテーブルで入れ替え画面が見つかったら画面を返す
		if( nextGamenSpecifyModel != nextGamenBaseModel ) {
			return nextGamenSpecifyModel;
		}
	}
	tableCount = 0;
	while( ReplaceGamenTable[tableCount].pc_nextGamen != NULL ) {
		if( ReplaceGamenTable[tableCount].pc_nextGamen == nextGamenBaseModel ) {
			nextGamenSpecifyModel = ReplaceGamenTable[tableCount].pc_changeGamen;
			break;
		}
		tableCount++;
	}
	if( nextGamenSpecifyModel == nextGamenBaseModel ) {
		// utility/serviceの置き換えTableをサーチする.
		tableCount = 0;
		STRUCT_ReplaceGamenTable* p_changeTable = NULL;
		p_changeTable = ( STRUCT_ReplaceGamenTable* )pt_utilityReplaceGamenTable;
		while( p_changeTable[tableCount].pc_nextGamen != NULL ) {
			if( p_changeTable[tableCount].pc_nextGamen == nextGamenBaseModel ) {
				nextGamenSpecifyModel = p_changeTable[tableCount].pc_changeGamen;
				break;
			}
			tableCount++;
		}
	}
	if( nextGamenSpecifyModel == nextGamenBaseModel ) {
		// 拡大表示/helpの置き換えTableをサーチする.
		tableCount = 0;
		STRUCT_ReplaceGamenTable* p_changeTable = NULL;
		p_changeTable = ( STRUCT_ReplaceGamenTable* )pt_enlhelpReplaceGamenTable;
		while( p_changeTable[tableCount].pc_nextGamen != NULL ) {
			if( p_changeTable[tableCount].pc_nextGamen == nextGamenBaseModel ) {
				nextGamenSpecifyModel = p_changeTable[tableCount].pc_changeGamen;
				break;
			}
			tableCount++;
		}
	}
	return nextGamenSpecifyModel;
}
/*================================================================================================
	Rhein3用画面にいる場合、Rhein1の画面を戻す.
================================================================================================*/
PBasicMessageProc*	restoreGamen( PBasicMessageProc* nowGamenSpecifyModel )
{
	PBasicMessageProc*	nowGamenBaseModel;
	nowGamenBaseModel = nowGamenSpecifyModel;
	short tableCount = 0;
	if( ( PnlComGetPrinterSystem()->getFinisherType() == TYPE_FinisherTypeCB ) || ( TYPE_FinisherTypeFS527 == PnlComGetPrinterSystem()->getFinisherType() ) ) {
		while( changegamenCBFNTable[tableCount].pc_changeGamen != NULL ) {
			if( changegamenCBFNTable[tableCount].pc_changeGamen == nowGamenSpecifyModel ) {
				nowGamenBaseModel = changegamenCBFNTable[tableCount].pc_nextGamen;
				break;
			}
			tableCount++;
		}
		if( nowGamenBaseModel == nowGamenSpecifyModel ) {
			// utility/serviceの置き換えTableをサーチする
			tableCount = 0;
			STRUCT_ReplaceGamenTable* p_changeTable = NULL;
			p_changeTable = ( STRUCT_ReplaceGamenTable* )pt_utilityChangegamenCBFNTable;
			while( p_changeTable[tableCount].pc_changeGamen != NULL ) {
				if( p_changeTable[tableCount].pc_changeGamen == nowGamenSpecifyModel ) {
					nowGamenBaseModel = p_changeTable[tableCount].pc_nextGamen;
					break;
				}
				tableCount++;
			}
		}
		if( nowGamenBaseModel == nowGamenSpecifyModel ) {
			tableCount = 0;
			STRUCT_ReplaceGamenTable* p_changeTable = NULL;
			p_changeTable = ( STRUCT_ReplaceGamenTable* )pt_enlhelpChangegamenCBFNTable;
			while( p_changeTable[tableCount].pc_changeGamen != NULL ) {
				if( p_changeTable[tableCount].pc_changeGamen == nowGamenSpecifyModel ) {
					nowGamenBaseModel = p_changeTable[tableCount].pc_nextGamen;
					break;
				}
				tableCount++;
			}
		}
		// CB-FN用のテーブルで入れ替え画面が見つかったら画面を返す
		if( nowGamenBaseModel != nowGamenSpecifyModel ) {
			return nowGamenBaseModel;
		}
	}
	tableCount = 0;
	while( ReplaceGamenTable[tableCount].pc_changeGamen != NULL ) {
		if( ReplaceGamenTable[tableCount].pc_changeGamen == nowGamenSpecifyModel ) {
			nowGamenBaseModel = ReplaceGamenTable[tableCount].pc_nextGamen;
			break;
		}
		tableCount++;
	}
	if( nowGamenBaseModel == nowGamenSpecifyModel ) {
		// utility/serviceの置き換えTableをサーチする.
		tableCount = 0;
		STRUCT_ReplaceGamenTable* p_changeTable = NULL;
		p_changeTable = ( STRUCT_ReplaceGamenTable* )pt_utilityReplaceGamenTable;
		while( p_changeTable[tableCount].pc_changeGamen != NULL ) {
			if( p_changeTable[tableCount].pc_changeGamen == nowGamenSpecifyModel ) {
				nowGamenBaseModel = p_changeTable[tableCount].pc_nextGamen;
				break;
			}
			tableCount++;
		}
	}
	if( nowGamenBaseModel == nowGamenSpecifyModel ) {
		tableCount = 0;
		STRUCT_ReplaceGamenTable* p_changeTable = NULL;
		p_changeTable = ( STRUCT_ReplaceGamenTable* )pt_enlhelpReplaceGamenTable;
		while( p_changeTable[tableCount].pc_changeGamen != NULL ) {
			if( p_changeTable[tableCount].pc_changeGamen == nowGamenSpecifyModel ) {
				nowGamenBaseModel = p_changeTable[tableCount].pc_nextGamen;
				break;
			}
			tableCount++;
		}
	}
	return nowGamenBaseModel;
}
// ===============================================================================================
/**
@name	isEnabledLedPrintRed.
@brief	外部参照関数：プリント赤の状態判定.
*/
// ===============================================================================================
Bool isEnabledLedPrintRed()
{
	if( us_LedPrintRed != PNLE_LEDStateOff ) { return True; }		// プリント赤 ON状態.
	else { return False; }											// プリント赤 OFF状態.
}
/*================================================================================================
［機能］	基本メッセージ処理 : 音声ガイドキー押下.
								 音声ガイド開始／音声ガイド終了
［変更履歴］：2013/05/13 OPS1-Sky奥村 [ZM3rd][制御構成変更No1200,1220] 音声ガイド状態(CommonAPI)設定、ジョブ中BGM再生完了通知用オブザーバー登録.
================================================================================================*/
void PBasicMessageProc::procVoiceGuide( uchar option )
{
	if( !pt_BRWC_ReqFlashPlayStart ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Crit, "PBasicMessageProc::procVoiceGuide pt_BRWC_ReqFlashPlayStart is NULL. can't start voice guide." ); }
		return;
	}
	TYPE_VoiceGuide_Status ssVGStatus = getVoiceCursorMode();
	// 音声ガイド状態が、終了処理中のときは、何も .
	if( ssVGStatus >= TYPE_VoiceGuide_EndProcessing ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Warning, "PBasicMessageProc::procVoiceGuide getVoiceCursorMode is EndProcessing (%zd)", (intptr_t)TYPE_VoiceGuide_EndProcessing ); }
		return;
	}
	if( True == PNLC_SnapshotControlLib::newInstance()->getSkipCheckHDD() )
	{
		// 電源復帰時のHDDチェックスキップ中は使用不可.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	if( ssVGStatus == TYPE_VoiceGuide_OFF ) {
		// 音声ガイド中でないときは、設定条件により音声ガイドを開始にする.

		// 音声ガイド設定がOFFのときは何もしない.
		if( !PnlComGetOperationSetting()->getVoiceGuide()			// 管理者設定の音声ガイド設定がOFFのとき.
		    || !PnlComGetOperationSetting()->getVoiceVocalize() 	// ユニバーサル設定の音声ガイド設定がOFFのとき.
		    || TYPE_CategoryState_ENABLE != PnlComGetEnablerSetting()->getFEK_CotegoryState( TYPE_FEKType09 )
		    || PnlComGetMachineSetting()->getVoiceInstallFlgInfo() == False ) {	// 音声ガイダンス.
			setBuzzerType( PCGE_BuzzerType_InputNg );						// 受付け禁止キーのため入力無効音の処理を行う.
			if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Warning, "PBasicMessageProc::procVoiceGuide Voice Guide is OFF" ); }
			return;
		}

		if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW164, 0 ) ) {	// BGM再生機能、サウンドマスキング機能の使用可否.
			// ジョブ中BGMが再生中なら音声ガイド開始せずに再生完了を待つ.
			if( True == APIC_BGM::newInstance()->isBGMJobPlay() ) {
				APIC_BGM::newInstance()->attachObserver( &c_BGMObserver );	// ジョブ中BGM再生完了通知用オブザーバーの登録.
				APIC_BGM::newInstance()->setVoiceGuideStatus( True );		// 音声ガイド状態設定（ON）、ジョブ中BGMの停止要求.
				return;
			}
			APIC_BGM::newInstance()->setVoiceGuideStatus( True );	// 音声ガイド状態設定（ON）.
		}

// comment [特ROM対応][IT5-148]【SOLIMP3様】音声操作によるジョブ実行(IWS連携)要望_最終版リリース(USB検知音変更).
		if( True == PnlComIsUsbSetSoundPlayFuncEnable() ){
			APIC_USBSetUnsetSound*       pt_USBSoundAPI = APIC_USBSetUnsetSound::newInstance();
			if( pt_USBSoundAPI != NULL )
			{
				while( True ){
					if( pt_USBSoundAPI->isNowSoundPlay() == True ){
						SYSF_taskDelay( 500 );		//500ms待つ
					}else{
						break;
					}
				}
				pt_USBSoundAPI->setVoiceGuideStatus( True );	// 音声ガイド状態設定（ON）.
			}
		}

		// 音声ガイド非対象画面のとき.
		if( !m_pstVoiceGamenInfo ) {
			// 音声ガイド設定がOFFのときは何もしない.
			ushort	usVID = v_MSGErrorVoiceGuide; /* 仮 */					// 音声ガイド非対象画面の音声ＩＤをデフォルト値としてセットする.
			if( True == onlyPanel.bl_isEnlargeDisplayMode ) { usVID = v_MSGDisableNow; }	//拡大画面の非対象画面は「現在使用できません」.

			if( PnlComGetOperationSetting()->getVoiceLanguage() != APIC_MachineSetting::newInstance()->getLanguage() ) {
				//マシン言語と音声出力言語が不一致.
				if( TYPE_LanguageJapanese == APIC_MachineSetting::newInstance()->getLanguage() ) {
					//日本語.
					PnlComGetOperationSetting()->setVoiceLanguage( TYPE_LanguageJapanese );
				} else {
					//英語.
					PnlComGetOperationSetting()->setVoiceLanguage( TYPE_LanguageAmerican );
				}
			}

			// 音声ガイド非対象画面を読み上げる.
			reqVoicePlay( &usVID, 1, True, NULL );							// 音声再生要求(再生中止する：True)
			// 音声ガイドを終了する. パネルマネージャ→音声ガイド終了
			// 音声ガイド画面を更新する.
			UpdateVoiceGamen( False );
			return;
		}
		// 音声ガイド設定がOFFのときは何もしない.
		PMGC_PanelVoicePlay::newInstance()->initMemberVariable( True );
		// 音声ガイドを開始する. パネルマネージャ→音声ガイド開始
		PLayerControl* p_GamenInstance = dynamic_cast< PLayerControl* >( this );
		if( NULL != p_GamenInstance ) {
			PMGC_PanelManager::newInstance()->reqVoiceGuideStart( this->getVoiceCursorDefaultMode() );
		} else {
			PMGC_PanelManager::newInstance()->reqVoiceGuideStart( nowGamen->getVoiceCursorDefaultMode() );
		}
		// 音声ガイド画面を更新する.
		UpdateVoiceGamen( True );
	} else {
		// 音声ガイド中のときは、音声ガイドを終了する.
		if( ssVGStatus == TYPE_VoiceGuide_EndProcessing ) {
			;
		} else {
			if( ssVGStatus == TYPE_VoiceGuide_AutoTransition ) {
				// 音声ガイド状態の設定（パネルマネージャのラッパー関数）
				setVoiceCursorMode( TYPE_VoiceGuide_Normal );
			} else if( ssVGStatus < TYPE_VoiceGuide_Processing && ssVGStatus != TYPE_VoiceGuide_Processing ) {
				// 音声ガイドを終了する. パネルマネージャ→音声ガイド終了
				PMGC_PanelManager::newInstance()->reqVoiceGuideEnd( False );
				//終了要求後の音声ガイドステータスはパネルマネージャーで管理するため不要.
				// 音声ガイド状態の設定（パネルマネージャのラッパー関数）
			}
			// 音声ガイド画面を更新する.
			UpdateVoiceGamen( False );
		}
	}
}
/*================================================================================================
［機能］	基本メッセージ処理 : 音声ガイド終了
================================================================================================*/
void PBasicMessageProc::procVoiceGuide_End( TYPE_VoiceGuide_Status ssVGStatus )
{
	if( !pt_BRWC_ReqFlashPlayStart ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Crit, "PBasicMessageProc::procVoiceGuide_End pt_BRWC_ReqFlashPlayStart is NULL. can't start voice guide." ); }
		return;
	}
	setVoiceCursorMode( ssVGStatus );
	PMGC_PanelManager::newInstance()->reqVoiceGuideEnd( False );		// 音声ガイド終了
	//終了要求後の音声ガイドステータスはパネルマネージャーで管理するため不要.
	UpdateVoiceGamen( False );										// 音声ガイド画面を更新する.
}
/*================================================================================================
［機能］	基本メッセージ処理 : 音声ガイド画面更新用関数.
								 音声ガイド開始／音声ガイド終了
================================================================================================*/
void PBasicMessageProc::UpdateVoiceGamen( Bool bCurShow )
{
	// 画面情報を設定します
	PLayerControl* p_GamenInstance = dynamic_cast< PLayerControl* >( this );
	if( NULL != p_GamenInstance ) {
		this->setVoiceGamenInfo();
	} else {
		nowGamen->setVoiceGamenInfo();
	}
	// 音声ガイド一時起動対応.
	TYPE_VoiceGuide_Status	eVGuide = getVoiceCursorMode();		//音声ガイド最新のステータスを記憶.

	// 音声ガイドOFFの場合
	if( !bCurShow ) {
		setVoiceCursorMode( TYPE_VoiceGuide_Normal );							// 音声カーソルを非表示にするために音声ガイドを有効にします
		PNLC_VoiceCursorControl::newInstance()->endCursor();				// 音声カーソルを表示を終了する. 音声ガイドパフォーマンス改善.
		//終了要求後の音声ガイドステータスはパネルマネージャーで管理するため元に戻す.
		setVoiceCursorMode( eVGuide );										// 音声ガイドのステータスを元に戻す.
		return;
	}

	// 音声カーソルを表示します
	PNLC_VoiceCursorControl::newInstance()->initCursor( NULL,NULL );			// カーソルを初期化します
	if( NULL != p_GamenInstance ) {
		setupGamen(); // 表示更新を行う.
		show(); // 画面パーツの表示.
		PNLC_VoiceCursorControl::newInstance()->initCursor( this->getVoiceGamenInfo(),this->getVoicePartsConvert() );	// カーソルに画面情報を設定します.

	} else {
		nowGamen->setupGamen(); // 表示更新を行う.                                                                                                                                                                                                   // 表示部品をセットアップします
		nowGamen->show();  // 画面パーツの表示.
		PNLC_VoiceCursorControl::newInstance()->initCursor( nowGamen->getVoiceGamenInfo(),nowGamen->getVoicePartsConvert() );	// カーソルに画面情報を設定します.
	}
	PNLC_VoiceCursorControl::newInstance()->showCursor();									// 音声カーソルを表示します

	// 音声ガイド開始メッセージを読み上げます
	PNLC_VoiceRequestBuffer::newInstance()->clearMsgAreaRequest();	// メッセージエリア再生要求バッファのクリア
	if( NULL != p_GamenInstance ) {
		this->addReqStartVoiceID();
	} else {
		nowGamen->addReqStartVoiceID();									// 画面開始時の音声を再生要求バッファへ格納 .
	}
	PNLC_VoiceRequestBuffer::newInstance()->sendRequest();			// 再生要求を発行
	addReqTenkeyStartVoiceID();										// 「テンキー説明」を読み上げる(音声ガイド開始)
	PNLC_VoiceRequestBuffer::newInstance()->sendRequest();			// 再生要求を発行

}
/*================================================================================================
［機能］	テンキー説明を読み上げる。(音声ガイド開始)
================================================================================================*/
void PBasicMessageProc::addReqTenkeyStartVoiceID()
{
	ushort	usBuf[DEF_VoiceIdBufSize];
	short	ssBufSize = 0;
	// 音声非対応画面の音声IDを設定します
	usBuf[ssBufSize++] = v_MSGStartKeyVoiceGuide;	// テンキー説明
	// 音声IDを追加
	PNLC_VoiceRequestBuffer::newInstance()->addRequest( usBuf, ssBufSize );
}
/*================================================================================================
［機能］	テンキー説明を読み上げる。(1～9Key)
================================================================================================*/
void PBasicMessageProc::addReqTenkeyVoiceID()
{
#define 	TenkeyVoiceIDMAX	13		// テンキー説明ID 1～9Key,*,#,ID
	ushort keyMessage[TenkeyVoiceIDMAX] = {
		v_MSGTenKeyVoiceGuide, v_MSG1KeyVoiceGuide, v_MSG2KeyVoiceGuide, v_MSG3KeyVoiceGuide,
		v_MSG4KeyVoiceGuide,   v_MSG5KeyVoiceGuide, v_MSG6KeyVoiceGuide, v_MSG7KeyVoiceGuide,
		v_MSG8KeyVoiceGuide,   v_MSG9KeyVoiceGuide, v_MSGAsteriskKeyVoiceGuide, v_MSGShrapKeyVoiceGuide, v_MSGIDKeyVoiceGuide
	};
	ushort	usBuf[DEF_VoiceIdBufSize];
	short	ssBufSize = 0;
	short	n;
	// 音声非対応画面の音声IDを設定します
	for( n = 0; n < TenkeyVoiceIDMAX; n++ ) {
		usBuf[ssBufSize++] = keyMessage[n];		// テンキー説明 1～9Key
	}
	// 音声IDを追加
	PNLC_VoiceRequestBuffer::newInstance()->addRequest( usBuf, ssBufSize );
	// 再生要求を発行
	PNLC_VoiceRequestBuffer::newInstance()->sendRequest();
}
/*================================================================================================
［機能］	基本メッセージ処理 : 音声カーソルの初期モード取得.
================================================================================================*/
TYPE_VoiceGuide_Status PBasicMessageProc::getVoiceCursorDefaultMode( void )
{
	// 画面遷移直後、または、音声ガイド開始時の音声カーソルのモードを返す.
	return TYPE_VoiceGuide_DefaultMode;
	// オートトランジション画面のときは、各画面クラスでこの関数をオーバーライドして
	// オートトランジションモードを返すようにして下さい。
}
/*================================================================================================
［機能］	音声再生終了イベント処理.
================================================================================================*/
void PBasicMessageProc::procVoicePlayEnd( uchar/*, PBasicMessageProc*	pcClass */ )
{
	if( &gp_SetupGamen == pt_GamenBackupVoice ) {
		setVoiceCursorMode( TYPE_VoiceGuide_OFF );				// 音声ガイド状態をTYPE_VoiceGuide_OFFにセットする.
	}
	else if( NULL != pt_GamenBackupVoice ) {
		PLayerControl* p_GamenInstance = dynamic_cast< PLayerControl* >( pt_GamenBackupVoice );
		if( NULL != p_GamenInstance ) {
			pt_GamenBackupVoice->procOk( uc_procOkVoiceOptionBackup );
		} else {
			procOk( uc_procOkVoiceOptionBackup );
		}
	}

	pt_GamenBackupVoice = NULL;

}
/*================================================================================================
［機能］	音声再生要求（パネルマネージャのラッパー関数）.
================================================================================================*/
TYPE_VoicePlayStatus reqVoicePlay( ushort* ucReqVoiceID, ushort ucReqVoiceCnt, Bool bflag, PBasicMessageProc* pcReqGamen )
{
// comment 2015/04/09 OPS1 Sky東島 [Helios][CQ511770].
	if( True == PNLC_PopupListControl::newInstance()->isNowPopup( &gp_IGCClearWait_popup ) ){
		// IGCClearWaitポップアップ表示中は音声再生させない.
		return TYPE_VoicePlay_End;
	}
	// パネルマネージャクラス→音声再生要求（タスクIDの引数あり）
	return	PMGC_PanelManager::newInstance()->reqVoicePlay( ucReqVoiceID, ucReqVoiceCnt, bflag, pcReqGamen );
}
/*================================================================================================
［機能］	音声停止要求（パネルマネージャのラッパー関数）.
================================================================================================*/
TYPE_VoicePlayStatus reqVoiceStop( PBasicMessageProc* pcReqGamen )
{
	// パネルマネージャ→音声再生の停止要求
	return	PMGC_PanelManager::newInstance()->reqVoiceStop( pcReqGamen );
}
/*================================================================================================
［機能］	音声ガイド状態の取得（パネルマネージャのラッパー関数）.
================================================================================================*/
TYPE_VoiceGuide_Status getVoiceCursorMode( void )
{
	// パネルマネージャクラス→音声ガイド状態の取得
	return	PMGC_PanelManager::newInstance()->getVoiceCursorMode();
}
/*================================================================================================
［機能］	音声ガイド状態の設定（パネルマネージャのラッパー関数）.
================================================================================================*/
void setVoiceCursorMode( TYPE_VoiceGuide_Status ucVoiceCsrMode )
{
	// パネルマネージャクラス→音声ガイド状態の設定
	PMGC_PanelManager::newInstance()->setVoiceCursorMode( ucVoiceCsrMode );
}
/*================================================================================================
［機能］	音声再生状態取得（パネルマネージャのラッパー関数）.
================================================================================================*/
Bool getVoicePlayNow( void )
{
	// パネルマネージャクラス→音声再生状態取得
	return	PMGC_PanelManager::newInstance()->getVoicePlayNow();
}
/*================================================================================================
［機能］	再読み上げの音声再生要求（パネルマネージャのラッパー関数）.
================================================================================================*/
TYPE_VoicePlayStatus reqVoiceReplay()
{
	// パネルマネージャクラス→再読み上げの音声再生要求
	return	PMGC_PanelManager::newInstance()->reqVoiceReplay();
}
/*================================================================================================
［機能］	音声ログ出力（パネルマネージャのラッパー関数）
================================================================================================*/
Bool isVoiceLog( void )
{
	return PMGC_PanelManager::newInstance()->isVoiceLog();
}
/*================================================================================================
［機能］	音声ログ出力（パネルマネージャのラッパー関数）
================================================================================================*/
void outputVoiceLog( TYPE_VoiceLog_Mode ucVoiceLogMode, char* message,
                     intptr_t val1/* = 0*/, intptr_t val2/* = 0*/, intptr_t val3/* = 0*/,
                     intptr_t val4/* = 0*/, intptr_t val5/* = 0*/, intptr_t val6/* = 0*/ )
{
	PMGC_PanelManager::newInstance()->outputVoiceLog( ucVoiceLogMode, message, val1, val2, val3, val4, val5, val6 );
}
/*================================================================================================
［機能］	音声言語設定.
 [履歴]		2014/02/26 s.ohba 操作性改善FUM5 CQ.384853 新規作成.
================================================================================================*/
void setVoiceLanguage()
{
	APIC_OperationSetting* pc_Operation = PnlComGetOperationSetting();
	APIC_MachineSetting* pc_Machine = APIC_MachineSetting::newInstance();

	if( pc_Operation->getVoiceLanguage() != pc_Machine->getLanguage() ) { //マシン言語と音声出力言語が不一致.
		if( TYPE_LanguageJapanese == pc_Machine->getLanguage() ) {		//日本語.
			pc_Operation->setVoiceLanguage( TYPE_LanguageJapanese );
		} else {														//英語.
			pc_Operation->setVoiceLanguage( TYPE_LanguageAmerican );
		}
	}
}
/*================================================================================================
［機能］	ボックス送信中ジョブのチェック
================================================================================================*/
Bool PBasicMessageProc::isBoxSendJobCheck()
{
	LIBC_List<APIC_Job> c_Joblist;			// ジョブのリスト.
	short ss_listCnt 			 = 0;		// カウント.
	short ss_JobNum 			 = 0;		// リストの個数.
	APIC_Job* pc_Job		 	 = NULL;	// 確認用ジョブ.
	Bool bRet = False;
	static Bool b_Pre_ShortCharge = False;
	static Bool b_Pre_ShortChargeNoJob = False;
	Bool b_ShortCharge = False;;
	Bool b_ShortChargeNoJob = False;;
	static APIC_Job* cc_Pre_DisplayJob = NULL;

	if( APIC_ExtSetting::getNotifyType() != TYPE_ExtNotifyTypeCreditEachJob ) {	// 課金管理(ベンダータイプB)以外の場合.
		return bRet;
	}
	APIC_User* pUserBuf = PnlComGetUser();
	if( ( pUserBuf == NULL )												// ユーザーが無効.
	    || ( pUserBuf->isMidAuthServerLoginUserReqGetCounterTicket() ) ) {	// または、不足金額取得中の場合.
		return bRet;
	}
	if( ( EP_PNS_ReadExec == onlyPanel.ep_PanelStatus ) || ( EP_PNS_RealTimePrvExec == onlyPanel.ep_PanelStatus ) || ( EP_PNS_ReadErrorStop == onlyPanel.ep_PanelStatus )
	    || ( onlyPanel.ep_PanelStatus == EP_PNS_ReadStop ) || ( onlyPanel.ep_PanelStatus == EP_PNS_SeparateScanStop ) )
	{
		// 読み込み中
		return bRet;
	}

	int32_t i32_UserNo = pUserBuf->getUserNo(); // 2013/11/01 【US-20U-PN3G】20万人対応.

	// 警告検知対象のジョブ(cc_DisplayJob)が、ログインしているユーザーのジョブであるか否かを判断する.
	if( NULL != onlyPanel.cc_DisplayJob && i32_UserNo == onlyPanel.cc_DisplayJob->getJobSpec()->pc_Common->c_UserAcount.getUserNo() ) {
		// 警告検知対象のジョブ(cc_DisplayJob)が課金不足警告であるか否かを判断する.
		b_ShortCharge = onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, PERROR_ShortCharge );
	}
	// 警告検知対象のジョブ(cc_DisplayJob)が課金不足警告である場合.
	// 前回の状態と比較し、状態の変化があれば課金不足警告の変化エッジを検知したとして呼び出し元にTrueを返す.
	// 呼び出し元は、課金不足警告の変化エッジを検知すると画面更新(rebuildGamen)を要求する.
	//
	// 警告検知対象のジョブ(cc_DisplayJob)が、ログインしているユーザーの課金不足警告ジョブではない場合.
	// 本関数は、有効な全ジョブ(ActiveJobList)を走査して、ログインしているユーザーの課金不足ジョブが存在していないことを確認する.
	// 有効な全ジョブ(ActiveJobList)の中に、ログインしているユーザーの課金不足ジョブが存在する場合.
	// 本関数内で、警告検知対象のジョブ(cc_DisplayJob)を切り替える.
	if( b_ShortCharge == True ) {
		if( cc_Pre_DisplayJob == onlyPanel.cc_DisplayJob ) {
			if( b_Pre_ShortCharge == False ) {
				b_Pre_ShortCharge = b_ShortCharge;
				bRet = True;
			}
		}
		cc_Pre_DisplayJob = onlyPanel.cc_DisplayJob;
		return bRet;
	}

	// 全アクティブジョブを取得.
//Comment 2017/11/22【HCLT-Sudheer】【IT5 4.2】【RQ686】 API Correction.
//	ss_JobNum = cp_ActiveJobList->getList( TYPE_ActiveJobListAll, 	// アクティブジョブリスト種類.
//	                                       1, 						// 開始位置（１～Ｎ）.
//	                                       TYPD_MaxActiveJob, 		// 個数.
//	                                       &c_Joblist );			// リスト.
	ss_JobNum = cp_ActiveJobList->getList( TYPE_ActiveJobListAll, 	// アクティブジョブリスト種類.
	                                       1, 						// 開始位置（１～Ｎ）.
	                                       TYPD_MaxActiveJob, 		// 個数.
	                                       &c_Joblist,
	                                       PNLC_NetworkLan::getInstance()->getLine() );			// リスト.
	for( ss_listCnt = 0; ss_listCnt < ss_JobNum; ss_listCnt++ ) {
		// リストからジョブを取得.
		if( ss_listCnt == 0 ) {
			pc_Job = c_Joblist.findFirst();		// 先頭データを取得.
		} else {
			pc_Job = c_Joblist.findNext();		// 次データを取得.
		}
		if( pc_Job == NULL ) {					// ジョブのNULLチェック.
			return False;
		}
		// ジョブがcomPanelJobかcomPrintJobと同じなら警告発生チェックを省略する.
		if( pc_Job != comPanelJob ) {
			if( i32_UserNo == pc_Job->getJobSpec()->pc_Common->c_UserAcount.getUserNo() ) {	// 【OAPExFUM3】【課金MAP】【309595】【金額不足警告修正】【13開発笹岡】2008/10/23 add.
				if( TYPE_OutputStatusDeleting != pc_Job->getJobStatus()->getOutputStatus() ) {			// ジョブ破棄中でない.
					// 警告発生チェック.
					b_ShortCharge = onlyPanel.isWarningOn( pc_Job, PERROR_ShortCharge );
					b_ShortChargeNoJob = onlyPanel.isWarningOn( pc_Job, PERROR_ShortChargeNoJob );
					if( b_ShortCharge == True ) {	// 残金不足警告発生の場合.
						if( onlyPanel.bl_isEnlargeDisplayMode == True ) {	// 拡大表示モード時.
							//	対象の警告はジョブリストと同じ
							register ushort DataCheckSendJob = onlyPanel.isWarningatJoblistEnlarge( pc_Job );	// 今回のプリントジョブ表示警告状態

							if( 0 != DataCheckSendJob ) {
								Bool b_EmergencySend = False;
								for( short i = 0; i < PDPD_EmergencySendTableNum; i++ ) {
									// 表示中ジョブと警告バックアップテーブルのジョブが一致した場合は、左下「警告表示」を表示する
									if( ( onlyPanel.s_EmergencySendTable[i].pc_Job == pc_Job )
									    && ( onlyPanel.checkExistWarningBackupTable( onlyPanel.s_EmergencySendTable[i].ec_WarningName, sp_GamenEnlWarningTable[DataCheckSendJob].ec_WarningName ) == True ) )
									{
										b_EmergencySend = True;
									}
								}

								//	バックアップした警告表示中でなければ、送信ジョブを警告表示中ジョブに設定.
								if( False == b_EmergencySend
								    &&  TYPE_OutputStatusDeleting != pc_Job->getJobStatus()->getOutputStatus() ) {
									// 警告発生ジョブをcomSendJobに設定.
									if( pc_Job != comPrintJob ) {
										if( TYPE_JobTypeFaxSendMemory == pc_Job->getJobSpec()->pc_Common->c_JobType.getJobType() ) {	// ジョブタイプ:ＦＡＸ（メモリ送信）
											comSendJob = pc_Job;
										}
									}
									onlyPanel.cc_DisplayJob = pc_Job;

									bRet = True;
									break;
								}
							}
						} else {
							register ushort DataCheckSendJob = onlyPanel.isWarningatJoblist( pc_Job );	// 今回の送信ジョブ表示警告状態
							if( 0 != DataCheckSendJob ) {
								Bool b_EmergencySend = False;
								for( short i = 0; i < PDPD_EmergencySendTableNum; i++ ) {
									// 表示中ジョブと警告バックアップテーブルのジョブが一致した場合は、左下「警告表示」を表示する.
									if( ( onlyPanel.s_EmergencySendTable[i].pc_Job == pc_Job )
									    && ( onlyPanel.checkExistWarningBackupTable( onlyPanel.s_EmergencySendTable[i].ec_WarningName, sp_GamenWarningTable[DataCheckSendJob].ec_WarningName ) == True ) ) {
										onlyPanel.s_EmergencySendTable[i].pc_Job = NULL;
										onlyPanel.s_EmergencySendTable[i].ec_WarningName = TYPE_WarningName_Non;
										break;
									}
								}
								//	バックアップした警告表示中でなければ、送信ジョブを警告表示中ジョブに設定.
								if( False == b_EmergencySend
								    &&  TYPE_OutputStatusDeleting != pc_Job->getJobStatus()->getOutputStatus() ) {
									// 警告発生ジョブをcomSendJobに設定.
									if( pc_Job != comPrintJob ) {
										if( TYPE_JobTypeFaxSendMemory == pc_Job->getJobSpec()->pc_Common->c_JobType.getJobType() ) {	// ジョブタイプ:ＦＡＸ（メモリ送信）
											comSendJob = pc_Job;
										}
									}
									onlyPanel.cc_DisplayJob = pc_Job;

									bRet = True;
									break;
								}
							}
						}
					} else if( b_Pre_ShortChargeNoJob == False && b_ShortChargeNoJob == True ) {	// 残金不足警告(ジョブ未投入)発生の場合.
						if( comSendJob != pc_Job ) {										// 警告発生中でない場合.
							// 警告発生ジョブをcomSendJobに設定.
							comSendJob = pc_Job;
							onlyPanel.cc_DisplayJob = comSendJob;

							bRet = True;
							break;
						}
					} else {																// 警告未発生.
						comSendJob = NULL;												// comSendJobを初期化.
					}
				}
			}		// 【OAPExFUM3】【課金MAP】【309595】【金額不足警告修正】【13開発笹岡】2008/10/23 add.
		}
	}

	b_Pre_ShortCharge = b_ShortCharge;
	b_Pre_ShortChargeNoJob = b_ShortChargeNoJob;
	cc_Pre_DisplayJob = onlyPanel.cc_DisplayJob;
	return bRet;
}
// ===============================================================================================
/**
 *	@brief		ポップアップ上の全パーツに対して階層番号を指定する.
 *	@param[in]	sc_LayerNum			階層番号.
 */
// ===============================================================================================
void PBasicMessageProc::setLayerNumber( char sc_LayerNum )
{
	sc_LayerNumber = sc_LayerNum;	// メンバ変数に階層番号を保持する.

	// パーツテーブルのポインタとパーツ数を取得する.
	PDisplayParts** p_PartsTable = getPartsTable();
	const short ss_PartsNumber = getPartsNumber();

	// パーツテーブルに登録されている全パーツに対して階層番号を設定する.
	for( register short i=0; i<ss_PartsNumber; ++i ) {
		p_PartsTable[i]->setLayerNumber( sc_LayerNumber );
	}
}

/*================================================================================================
［機能］	基本メッセージ処理 : 画面パーツの再表示.
 [備考] 	レイヤー破棄後に下層レイヤーを復帰させる場合に限り本関数を使用すること.
================================================================================================*/
void PBasicMessageProc::redraw()
{
	for( short i = 0; partsNumber > i; ++i ) {
		partsTable[ i ]->show();
	}
}
/*================================================================================================
［機能］	基本メッセージ処理 : パネル切替時のベース画面描画要求.
 [備考] 	アプリ切替のタイミングでは、優先アプリがパネルとなっておらず、描画がスキップされるため.
 			切替のタイミングで描画を更新する.
================================================================================================*/
void PBasicMessageProc::procChgBaseGamen( uchar option )
{
	show();
	setupGamen();
}

struct PCGS_DeviceWarning {
	LAPE_NTF_Event      e_EventId;     // イベントID.
	PNLE_NtfEventDeviceWarning e_DeviceWarning;   // デバイス警告種類.
} st_DeviceWarning[] = {
	{ LAPE_NTF_NOTICE_StateFatal, 		PNLE_NtfEventDeviceWarning_FatalGenerated },//	(uint32_t)0x0005000C   ///<    ＦＡＴＡＬが発生しているか.
	{ LAPE_NTF_NOTICE_FatalGenerated,		PNLE_NtfEventDeviceWarning_FatalGenerated 		},//	(uint32_t)0x0005000C   ///<    ＦＡＴＡＬが発生しているか.
	{ LAPE_NTF_NOTICE_SysStsOccurFatal, 	PNLE_NtfEventDeviceWarning_SysStsOccurFatal 	},//	(uint32_t)0x0004006A   ///<    FATALが発生しているか.
	{ LAPE_NTF_NOTICE_TheAdfJamGenerated, PNLE_NtfEventDeviceWarning_TheAdfJamGenerated 	},//	(uint32_t)0x00050011   ///<    ＡＤＦジャムが発生しているか.
	{ LAPE_NTF_NOTICE_TheDoorEitherOpen, 	PNLE_NtfEventDeviceWarning_TheDoorEitherOpen 	},//	(uint32_t)0x00050014   ///<    何れかのドアが開いているか.
};
const uchar uc_DeviceWarning = ( uchar )( sizeof( st_DeviceWarning ) / sizeof( PCGS_DeviceWarning ) );
// ==================================================================
//概要    :オブザーバからのコール(新API通知処理用).
//        :詳細データはgetWarningInfo( e_EventID, &winfo )で取得する必要がある.
//作成者  :第一OSW 仁枝.
// ==================================================================
void PBasicMessageProc::procCallBackNtfEvent( uchar option, void* st_NtfEventData )
{
	if( NULL == st_NtfEventData ) {
		SYSF_logMsg( "*** panel *** %s:(%d) [%s] ===Assert Failed!\n", __FILE__, 0, __FUNCTION__, 4, 5, 6 );
		return;
	}
	PNLS_NtfEventData* st_EventData = ( PNLS_NtfEventData* )st_NtfEventData;
	if( NULL == st_EventData ) {
		SYSF_logMsg( "*** panel *** %s:(%d) [%s] ===Assert Failed!\n", __FILE__, 0, __FUNCTION__, 4, 5, 6 );
		return;
	}
	PNLC_NotifyEventManager* pc_NotifyEventManager = PNLC_NotifyEventManager::newInstance();
	if( NULL == pc_NotifyEventManager ) {
		SYSF_logMsg( "*** panel *** %s:(%d) [%s] ===Assert Failed!\n", __FILE__, 0, __FUNCTION__, 4, 5, 6 );
		return;
	}
	LAPE_NTF_Event e_EventID = st_EventData->event_id;

	PNLC_NotifyEventInfo* pc_NtfEventInfo = PNLC_NotifyEventInfo::newInstance();		// データの管理クラス.
	if( NULL == pc_NtfEventInfo ) {
		SYSF_logMsg( "*** panel *** %s:(%d) [%s] ===Assert Failed!\n", __FILE__, 0, __FUNCTION__, 4, 5, 6 );
		return;
	}


	//LAPE_NTF_Group01～12 データ取得領域.
	LAPC_BFW_System* pc_BFWSystem = LAPC_BFW_System::newInstance();
	if( NULL == pc_BFWSystem ) {
		SYSF_logMsg( "*** panel *** %s:(%d) [%s] ===Assert Failed!\n", __FILE__, 0, __FUNCTION__, 4, 5, 6 );
		return;
	}
	LAPC_NTF_Notify_Setting ntfy( pc_BFWSystem );
	LAPS_NTF_warningInfo winfo;	//取得内容.
	API_RESULT e_WinfoRes = False;	//取得結果.
	API_BOOL bl_Info = False;		// データ取得取得用変数.
	//取得例.
	//API_BOOL bl_ThePlatenCoverOpen = False;					// 取得したい型で詳細データ取得用変数を宣言.
	//winfo.event_id = e_EventID;								// 取得したいイベントのID指定.
	//winfo.info_len = sizeof(bl_ThePlatenCoverOpen);			// 取得したい型のサイズ指定.
	//winfo.info = &bl_ThePlatenCoverOpen;						// 詳細データ取得用変数のアドレス指定.
	//e_WinfoRes = ntfy.getWarningInfo( e_EventID, &winfo );	// 詳細データ取得API.
	//以後、取得用変数からデータにアクセス可能.
// Comment [2016/10/20] [HCLT-Kalaiyarasi] [IT5_4.0][AR1005467] declaration of structure.
	LAPS_NTF_SysStatusFatal st_SysStatusFatal;

// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
	LUIC_DbgTsk::newInstance()->setNowGamenHandle( reinterpret_cast< uint64_t >( this ) );
	LUIC_DbgTsk::newInstance()->setMessageType( (short)(MS_CallBackNtfEvent << 8 | option) );

	//イベントごとの処理.
	switch( e_EventID )
	{
		case LAPE_NTF_NOTICE_ADFHasOriginalDecision:		// ＡＤＦ・原稿有り 判定.
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 1, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]",SYSF_tickGet(),e_EventID,0,0,0,0);
			// ADF原稿Set処理.
			pc_NtfEventInfo->setAdfOriginalExist( ppc_Original->isAdfOriginalExist() );
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, pc_NtfEventInfo->getAdfOriginalExist() , 1 );

			break;
		case LAPE_NTF_NOTICE_ThePlatenCoverOpen:
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 2, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]",SYSF_tickGet(),e_EventID,0,0,0,0);
			// 原稿カバー開閉Set処理.
			pc_NtfEventInfo->setPlatenCoverOpen( ppc_ScannerWarning->isPlatenCoverOpen() );
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, pc_NtfEventInfo->getPlatenCoverOpen() , 2 );

			break;
		case LAPE_NTF_NOTICE_ERROR_PrinterJamDetection:			///<	0x000000A1 プリンタ関連:プリンタジャム検出.
		case LAPE_NTF_NOTICE_ERROR_ScannerAdfjam:				///<	0x000000FC スキャナー関連:ＡＤＦＪＡＭ.
		case LAPE_NTF_NOTICE_ERROR_YellowColorTonerEmptyStop:	///<	0x000000A4 プリンタ関連:イエロー色トナーエンプティストップ.
		case LAPE_NTF_NOTICE_ERROR_MagentaColorTonerEmptyStop:	///<	0x000000A3 プリンタ関連:マゼンタ色トナーエンプティストップ.
		case LAPE_NTF_NOTICE_ERROR_CyanogenColorTonerEmptyStop:	///<	0x000000A2 プリンタ関連:シアン色トナーエンプティストップ.
		case LAPE_NTF_NOTICE_ERROR_BlackColorTonerEmptyStop:	///<	0x000000A5 プリンタ関連:ブラック色トナーエンプティストップ.
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 3, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID and Occurance-Release flag.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X] WarningExist[%d]",SYSF_tickGet(),e_EventID,st_EventData->info.e_Api_Bool,0,0,0 );
			if( True == ( Bool )st_EventData->info.e_Api_Bool ) {
				procBuzzerWarningExist( option );					// ブザーを鳴らす警告発生.
			}
			onlyPanel.setWarningNtfEvent( e_EventID , ( Bool )st_EventData->info.e_Api_Bool );
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, st_EventData->info.e_Api_Bool , 3 );
			break;
		case LAPE_NTF_NOTICE_ERROR_UsbNeedPullOut:		///<	0x00000154 USB関連:USBメモリ引き抜き要求状態.
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 4, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]",SYSF_tickGet(),e_EventID,0,0,0,0);
			// 外部メモリー引き抜き要求状態Set処理.
			APIC_UsbMemDeviceInfo* pc_USBMemoryInfo = APIC_UsbMemDeviceInfo::newInstance();
			if( NULL == pc_USBMemoryInfo ) {
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
				break;
			}
			pc_NtfEventInfo->setUsbMemWarningNeedPullOut( pc_USBMemoryInfo->isWarning( TYPE_UsbMemWarningNeedPullOut ) );
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, pc_NtfEventInfo->getUsbMemWarningNeedPullOut() , 4 );
			break;
		}
		case LAPE_NTF_NOTICE_ERROR_UsbDoingUsb2:		///<	0x0000010D USB関連:USB２つ挿し状態.
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 5, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID and Occurance-Release flag.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X] WarningExist[%d]",SYSF_tickGet(),e_EventID,st_EventData->info.e_Api_Bool,0,0,0 );
			// 外部メモリー複数検知警告Set処理.
			APIC_UsbMemDeviceInfo* pc_USBMemoryInfo = APIC_UsbMemDeviceInfo::newInstance();
			if( NULL == pc_USBMemoryInfo ) {
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
				break;
			}
			pc_NtfEventInfo->setUsbMemWarningTwoInsert( ( Bool )st_EventData->info.e_Api_Bool );
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, pc_NtfEventInfo->getUsbMemWarningTwoInsert() , 5 );
			break;
		}
		// FAXデバイス警告イベント.
		case LAPE_NTF_NOTICE_ERROR_LineDial:			//	(uint32_t)0x0000006B	///<	FAX関連: 回線1ダイアルエラー.
		case LAPE_NTF_NOTICE_ERROR_LineTransmission:	//	(uint32_t)0x0000006D	///<	FAX関連: 回線1送信中エラー.
		case LAPE_NTF_NOTICE_ERROR_LineReception:		//	(uint32_t)0x0000006F	///<	FAX関連: 回線1受信中エラー.
		case LAPE_NTF_NOTICE_ERROR_LineTest:			//	(uint32_t)0x00000071	///<	FAX関連: 回線1テストのエラー.
		case LAPE_NTF_NOTICE_ERROR_LineIsw:				//	(uint32_t)0x00000073	///<	FAX関連: 回線1Iswのエラー.
		case LAPE_NTF_NOTICE_ERROR_WritingLineNvRam:	//	(uint32_t)0x00000075	///<	FAX関連: 回線1NvRam書き込み中のエラー.
		case LAPE_NTF_NOTICE_ERROR_LineInitialization:	//	(uint32_t)0x00000077	///<	FAX関連: 回線1初期化中のエラー.
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 6, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]",SYSF_tickGet(),e_EventID,0,0,0,0);
			// 警告発生のため情報をＦａｘ警告情報へセット.
			pc_NtfEventInfo->setForFaxWarningLine( TYPE_CommLine1 );
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNLS_FaxWarningDetail st_FaxWarningDetail;		// FAX警告情報.
			pc_NtfEventInfo->getFaxWarningDetail( TYPE_CommLine1 , st_FaxWarningDetail );
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, st_FaxWarningDetail.e_Warning , 6 );
			break;
		}
		case LAPE_NTF_NOTICE_ERROR_Line2Dial:			//	(uint32_t)0x0000006C	///<	FAX関連: 回線2ダイアルエラー.
		case LAPE_NTF_NOTICE_ERROR_Line2Transmission:	//	(uint32_t)0x0000006E	///<	FAX関連: 回線2送信中エラー.
		case LAPE_NTF_NOTICE_ERROR_Line2Reception:		//	(uint32_t)0x00000070	///<	FAX関連: 回線2受信中エラー.
		case LAPE_NTF_NOTICE_ERROR_Line2Test:			//	(uint32_t)0x00000072	///<	FAX関連: 回線2テストのエラー.
		case LAPE_NTF_NOTICE_ERROR_Line2Isw:			//	(uint32_t)0x00000074	///<	FAX関連: 回線2Iswのエラー.
		case LAPE_NTF_NOTICE_ERROR_WritingLine2NvRam:	//	(uint32_t)0x00000076	///<	FAX関連: 回線2NvRam書き込み中のエラー.
		case LAPE_NTF_NOTICE_ERROR_Line2Initialization:	//	(uint32_t)0x00000078	///<	FAX関連: 回線2初期化中のエラー.
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 7, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]",SYSF_tickGet(),e_EventID,0,0,0,0);
			// 警告発生のため情報をＦａｘ警告情報へセット.
			pc_NtfEventInfo->setForFaxWarningLine( TYPE_CommLine2 );
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNLS_FaxWarningDetail st_FaxWarningDetail;		// FAX警告情報.
			pc_NtfEventInfo->getFaxWarningDetail( TYPE_CommLine2 , st_FaxWarningDetail );
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, st_FaxWarningDetail.e_Warning , 7 );
			break;
		}

		case LAPE_NTF_NOTICE_ERROR_Line3Dial:			// (uint32_t)(0x000001AC)	///<	FAX関連: 回線3ダイアルエラー.
		case LAPE_NTF_NOTICE_ERROR_Line3Transmission:	// (uint32_t)(0x000001AE)	///<	FAX関連: 回線3送信中エラー.
		case LAPE_NTF_NOTICE_ERROR_Line3Reception:		// (uint32_t)(0x000001B0)	///<	FAX関連: 回線3受信中エラー.
		case LAPE_NTF_NOTICE_ERROR_Line3Test:			// (uint32_t)(0x000001B2)	///<	FAX関連: 回線3テストのエラー.
		case LAPE_NTF_NOTICE_ERROR_Line3Isw:			// (uint32_t)(0x000001B4)	///<	FAX関連: 回線3Iswのエラー.
		case LAPE_NTF_NOTICE_ERROR_WritingLine3NvRam:	// (uint32_t)(0x000001B6)	///<	FAX関連: 回線3NvRam書き込み中のエラー.
		case LAPE_NTF_NOTICE_ERROR_Line3Initialization:	// (uint32_t)(0x000001B8)	///<	FAX関連: 回線3初期化中のエラー.
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 8, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]",SYSF_tickGet(),e_EventID,0,0,0,0);
			pc_NtfEventInfo->setForFaxWarningLine( TYPE_CommLine3 );
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNLS_FaxWarningDetail st_FaxWarningDetail;		// FAX警告情報.
			pc_NtfEventInfo->getFaxWarningDetail( TYPE_CommLine3 , st_FaxWarningDetail );
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, st_FaxWarningDetail.e_Warning , 8 );
			break;
		}
		case LAPE_NTF_NOTICE_ERROR_Line4Dial:			// (uint32_t)(0x000001AD)	///<	FAX関連: 回線4ダイアルエラー.
		case LAPE_NTF_NOTICE_ERROR_Line4Transmission:	// (uint32_t)(0x000001AF)	///<	FAX関連: 回線4送信中エラー.
		case LAPE_NTF_NOTICE_ERROR_Line4Reception:		// (uint32_t)(0x000001B1)	///<	FAX関連: 回線4受信中エラー.
		case LAPE_NTF_NOTICE_ERROR_Line4Test:			// (uint32_t)(0x000001B3)	///<	FAX関連: 回線4テストのエラー.
		case LAPE_NTF_NOTICE_ERROR_Line4Isw:			// (uint32_t)(0x000001B5)	///<	FAX関連: 回線4Iswのエラー.
		case LAPE_NTF_NOTICE_ERROR_WritingLine4NvRam:	// (uint32_t)(0x000001B7)	///<	FAX関連: 回線4NvRam書き込み中のエラー.
		case LAPE_NTF_NOTICE_ERROR_Line4Initialization:	// (uint32_t)(0x000001B9)	///<	FAX関連: 回線4初期化中のエラー.
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 9, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]",SYSF_tickGet(),e_EventID,0,0,0,0);
			pc_NtfEventInfo->setForFaxWarningLine( TYPE_CommLine4 );
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNLS_FaxWarningDetail st_FaxWarningDetail;		// FAX警告情報.
			pc_NtfEventInfo->getFaxWarningDetail( TYPE_CommLine4 , st_FaxWarningDetail );
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, st_FaxWarningDetail.e_Warning , 9 );
			break;
		}

		case LAPE_NTF_NOTICE_StateFatal:	                                                         //	(uint32_t)0x0007005D   ///<    ＦＡＴＡＬ状態通知.
		case LAPE_NTF_NOTICE_FatalGenerated:	                                                     //	(uint32_t)0x0005000C   ///<    ＦＡＴＡＬが発生しているか.
		case LAPE_NTF_NOTICE_SysStsOccurFatal:	                                                     //	(uint32_t)0x0004006A   ///<    FATALが発生しているか.
		case LAPE_NTF_NOTICE_TheAdfJamGenerated:	                                                 //	(uint32_t)0x00050011   ///<    ＡＤＦジャムが発生しているか.
		case LAPE_NTF_NOTICE_TheDoorEitherOpen:	                                                     //	(uint32_t)0x00050014   ///<    何れかのドアが開いているか.
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 10, 0 );
			PNLE_NtfEventDeviceWarning e_DeviceWarning = PNLE_NtfEventDeviceWarning_StateFatal;  // デバイス警告種類.
			winfo.event_id = e_EventID;										                     // 取得したいイベントのID指定.
			winfo.info_len = sizeof( bl_Info );			                                       // 取得したい型のサイズ指定.
			winfo.info = &bl_Info;						                                         // 詳細データ取得用変数のアドレス指定.
			e_WinfoRes = ntfy.getWarningInfo( e_EventID, &winfo );			                     // 詳細データ取得API.
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]  Bool_Info[%d] WinfoResult[%d]",SYSF_tickGet(),e_EventID,bl_Info,e_WinfoRes,0,0);
			if( LAPE_ResultOK != e_WinfoRes ) {
				break;                                                                             // 処理結果が異常終了の場合は、処理を抜ける.
			}
			for( uchar uc_Dev = 0; uc_Dev < uc_DeviceWarning; uc_Dev++ ) {
				if( st_DeviceWarning[uc_Dev].e_EventId == e_EventID ) {
					e_DeviceWarning = st_DeviceWarning[uc_Dev].e_DeviceWarning;                  // デバイス警告種類.
					break;
				}
			}
			pc_NtfEventInfo->setDeviceWarning( e_DeviceWarning, ( Bool )bl_Info );				// データ管理クラスに、初期化済みフラグとイベントデータを設定する.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, bl_Info , 10 );
// comment 2016/09/16【HCL-Sairam】【IT5_4.0】【制御構成変更_No.51】 To get the Auto Self Diagnosis value from shared Memory. 
			switch( e_EventID )
			{
			case LAPE_NTF_NOTICE_StateFatal:												//	Priner Fatal Warning Notification.
				winfo.event_id = LAPE_NTF_NOTICE_AutoSelfCheck;
				winfo.info_len = sizeof( bl_Info );
				winfo.info = &bl_Info;
				e_WinfoRes = ntfy.getWarningInfo( LAPE_NTF_NOTICE_AutoSelfCheck, &winfo );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
				PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]  Win_Info[%d] WinfoResult[%d]",SYSF_tickGet(),e_EventID,bl_Info,e_WinfoRes,0,0);
				if( LAPE_ResultOK != e_WinfoRes ) {
					break;
				}
				if( True == bl_Info ){								// Auto Self Diagnosis Mode is Enabled.
					pc_NtfEventInfo->setSelfcheckStatus( True );
				}
				break;
			case LAPE_NTF_NOTICE_FatalGenerated:												//	Scanner Fatal Warning Notification.
				winfo.event_id = LAPE_NTF_NOTICE_AcquisitionAutoSelfCheck;
				winfo.info_len = sizeof( bl_Info );
				winfo.info = &bl_Info;
				e_WinfoRes = ntfy.getWarningInfo( LAPE_NTF_NOTICE_AcquisitionAutoSelfCheck, &winfo );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
				PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]  Win_Info[%d] WinfoResult[%d]",SYSF_tickGet(),e_EventID,bl_Info,e_WinfoRes,0,0);
				if( LAPE_ResultOK != e_WinfoRes ) {
					break;
				}
				if( True == bl_Info ){								// Auto Self Diagnosis Mode is Enabled.
					pc_NtfEventInfo->setSelfcheckStatus( True );
				}
				break;
			case LAPE_NTF_NOTICE_SysStsOccurFatal:												//	System Fatal Warning Notification.
// Comment [2016/10/20] [HCLT-Kalaiyarasi] [IT5_4.0][AR1005467] Passing the address of FATAL change notification structure to warning info.
				winfo.event_id = LAPE_NTF_NOTICE_Fatal;
//				winfo.info_len = sizeof( bl_Info );
//				winfo.info = &bl_Info;
				winfo.info_len = sizeof( st_SysStatusFatal );																																																																											
				winfo.info = &st_SysStatusFatal;
				e_WinfoRes = ntfy.getWarningInfo( LAPE_NTF_NOTICE_Fatal, &winfo );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
				PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]  Win_Info[%d] WinfoResult[%d]",SYSF_tickGet(),e_EventID,bl_Info,e_WinfoRes,0,0);
				if( LAPE_ResultOK != e_WinfoRes ) {
					break;
				}
// Comment [2016/10/20] [HCLT-Kalaiyarasi] [IT5_4.0][AR1005467] When Auto Self Diagnosis is enabled, set the status.				
//				if( True == bl_Info ){								// Auto Self Diagnosis Mode is Enabled.
				if( True == st_SysStatusFatal.b_AutoSelfCheck ) {
					pc_NtfEventInfo->setSelfcheckStatus( True );
				}
				break;
			}
			break;
		}
		case LAPE_NTF_NOTICE_ERROR_ScannerEdhManuscriptGlassDirtCaution1:	//	(uint32_t)0x00000101	///<	スキャナー関連:ＥＤＨ原稿ガラス汚れ警告(CAUTION1).
		case LAPE_NTF_NOTICE_ERROR_ScannerEdhManuscriptGlassDirtCaution2:	//	(uint32_t)0x00000102	///<	スキャナー関連:ＥＤＨ原稿ガラス汚れ警告(CAUTION2).
		case LAPE_NTF_NOTICE_ERROR_ScannerEdhManuscriptGlassDirtAttention:	//	(uint32_t)0x00000103	///<	スキャナー関連:ＥＤＨ原稿ガラス汚れ警告(ATTENTION).
		case LAPE_NTF_NOTICE_ERROR_ScannerEdhBackgroundBoardDirt:			//	(uint32_t)0x00000104	///<	スキャナー関連:ＥＤＨ背景板汚れ警告.
		case LAPE_NTF_NOTICE_ERROR_ScannerCisReadingGlassDirtCaution2:		//	(uint32_t)0x00000109	///<	スキャナー関連:ＣＩＳ読取りガラス汚れ警告(CAUTION2).
		case LAPE_NTF_NOTICE_ERROR_ScannerCisReadingGlassDirtAttention:		//	(uint32_t)0x0000010A	///<	スキャナー関連:ＣＩＳ読取りガラス汚れ警告(ATTENTION).
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 11, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID and Occurance-Release flag.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X] WarningExist[%d]",SYSF_tickGet(),e_EventID,st_EventData->info.e_Api_Bool,0,0,0 );
			PNLE_NtfEventWarningType e_NtfEventWarningType = PNLE_NtfEventWarningType_Non;   // 警告種類.
			// LAPE_NTF_Eventを警告名に変換し、警告種類を取得する.
			TYPE_WarningName e_WarningName = pc_NotifyEventManager->getWarningNameFromNtfEvent( e_EventID, e_NtfEventWarningType );

			// データ管理クラスに、初期化済みフラグとイベントデータを設定する.
			pc_NtfEventInfo->setGrassWarning( e_WarningName, ( Bool )st_EventData->info.e_Api_Bool );
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, st_EventData->info.e_Api_Bool , 11 );
			break;
		}
		case LAPE_NTF_NOTICE_ERROR_RDS_Modem:	//	(uint32_t)0x00000065	///<	RDS関連: モデム異常.
		case LAPE_NTF_NOTICE_ERROR_CyanogenColorTonaNiaEmpty:	//	(uint32_t)0x00000079	///<	プリンタ関連:シアン色トナーニアエンプティ.
		case LAPE_NTF_NOTICE_ERROR_MagentaColorTonaNiaEmpty:	//	(uint32_t)0x0000007A	///<	プリンタ関連:マゼンタ色トナーニアエンプティ.
		case LAPE_NTF_NOTICE_ERROR_YellowColorTonaNiaEmpty:	//	(uint32_t)0x0000007B	///<	プリンタ関連:イエロー色トナーニアエンプティ.
		case LAPE_NTF_NOTICE_ERROR_BlackColorTonaNiaEmpty:	//	(uint32_t)0x0000007C	///<	プリンタ関連:ブラック色トナーニアエンプティ.
		case LAPE_NTF_NOTICE_ERROR_AbolitionTonaNiafull:	//	(uint32_t)0x00000081	///<	プリンタ関連:廃トナーニアフル.
		case LAPE_NTF_NOTICE_ERROR_PRTRelationWarning:	//	(uint32_t)0x00000082	///<	プリンタ関連:ＰＲＴ関連異常警告.
		case LAPE_NTF_NOTICE_ERROR_EmptyStapleInterior:	//	(uint32_t)0x00000090	///<	プリンタ関連:ステープルエンプティ1(奥).
		case LAPE_NTF_NOTICE_ERROR_EmptyStapleInside:	//	(uint32_t)0x00000091	///<	プリンタ関連:ステープルエンプティ2(手前) .
		case LAPE_NTF_NOTICE_ERROR_EmptyStaple3:	//	(uint32_t)0x00000092	///<	プリンタ関連:ステープルエンプティ3.
		case LAPE_NTF_NOTICE_ERROR_PunchFullRubbishUsrExchange:	//	(uint32_t)0x00000094	///<	プリンタ関連:パンチくずフル(ユーザー交換時) .
		case LAPE_NTF_NOTICE_ERROR_PunchFullRubbishServiceExchanged:	//	(uint32_t)0x00000095	///<	プリンタ関連:パンチくずフル(サービス交換時) .
		case LAPE_NTF_NOTICE_ERROR_TranscriptRollerUnitNiaLife:	//	(uint32_t)0x00000096	///<	プリンタ関連:転写ローラーユニットニアライフ .
		case LAPE_NTF_NOTICE_ERROR_RmvCleaner:	//	(uint32_t)0x00000097	///<	プリンタ関連:紙紛除去クリーナー／オゾンフィルタニアライフ .
		case LAPE_NTF_NOTICE_ERROR_SmokeFirutaniaLife:	//	(uint32_t)0x00000098	///<	プリンタ関連:噴煙フィルタニアライフ .
		case LAPE_NTF_NOTICE_ERROR_PMCounterOver:	//	(uint32_t)0x0000009A	///<	プリンタ関連:ＰＭカウンタオーバー(ATTENTION) .
		case LAPE_NTF_NOTICE_ERROR_PrinterSetDefectiveAttention:	//	(uint32_t)0x0000009B	///<	プリンタ関連:プリンタセット不良(ATTENTION) .
		case LAPE_NTF_NOTICE_ERROR_DCUsrExchangeCyanogen:	//	(uint32_t)0x0000009C	///<	プリンタ関連:ＤＣユーザー交換超過警告・シアン（Amurのみ).
		case LAPE_NTF_NOTICE_ERROR_DCUsrExchangeMazenda:	//	(uint32_t)0x0000009D	///<	プリンタ関連:ＤＣユーザー交換超過警告・マゼンダ（Amurのみ).
		case LAPE_NTF_NOTICE_ERROR_DCUsrExchangeYellow:	//	(uint32_t)0x0000009E	///<	プリンタ関連:ＤＣユーザー交換超過警告・イエロー（Amurのみ).
		case LAPE_NTF_NOTICE_ERROR_DCUsrExchangeBlack:	//	(uint32_t)0x0000009F	///<	プリンタ関連:ＤＣユーザー交換超過警告・ブラック（Amurのみ).
		case LAPE_NTF_NOTICE_ERROR_OHPSetProhibition:	//	(uint32_t)0x000000A0	///<	プリンタ関連:ＯＨＰ縦用紙セット禁止警告.
		case LAPE_NTF_NOTICE_ERROR_DisagreementSzForm:	//	(uint32_t)0x000000A6	///<	プリンタ関連:用紙サイズ不一致.
		case LAPE_NTF_NOTICE_ERROR_FormKindDisagreement:	//	(uint32_t)0x000000A7	///<	プリンタ関連:用紙種類不一致.
		case LAPE_NTF_NOTICE_ERROR_PaperFeedTraySetDefective:	//	(uint32_t)0x000000A8	///<	プリンタ関連:給紙トレイセット不良 .
		case LAPE_NTF_NOTICE_ERROR_PrinterSetDefective:	//	(uint32_t)0x000000A9	///<	プリンタ関連:プリンタセット不良.
		case LAPE_NTF_NOTICE_ERROR_TonerBottleImperfectSetting:	//	(uint32_t)0x000000AA	///<	プリンタ関連:廃トナーボトル（廃棄トナーボトル）セット不良.
		case LAPE_NTF_NOTICE_ERROR_TonerBottleDefectiveSetcyanogen:	//	(uint32_t)0x000000AB	///<	プリンタ関連:トナーボトルセット不良・シアン.
		case LAPE_NTF_NOTICE_ERROR_TonerBottleDefectiveSetmagenta:	//	(uint32_t)0x000000AC	///<	プリンタ関連:トナーボトルセット不良・マゼンタ.
		case LAPE_NTF_NOTICE_ERROR_TonerBottleDefectiveSetyellow:	//	(uint32_t)0x000000AD	///<	プリンタ関連:トナーボトルセット不良・イエロー.
		case LAPE_NTF_NOTICE_ERROR_TonerBottleDefectiveSetblack:	//	(uint32_t)0x000000AE	///<	プリンタ関連:トナーボトルセット不良・ブラック.
		case LAPE_NTF_NOTICE_ERROR_IUImperfectsettingCyanogen:	//	(uint32_t)0x000000BD	///<	プリンタ関連:ＩＵセット不良・シアン.
		case LAPE_NTF_NOTICE_ERROR_IUImperfectsettingMagenta:	//	(uint32_t)0x000000BE	///<	プリンタ関連:ＩＵセット不良・マゼンタ.
		case LAPE_NTF_NOTICE_ERROR_IUImperfectsettingYellow:	//	(uint32_t)0x000000BF	///<	プリンタ関連:ＩＵセット不良・イエロー.
		case LAPE_NTF_NOTICE_ERROR_IUImperfectsettingBlack:	//	(uint32_t)0x000000C0	///<	プリンタ関連:ＩＵセット不良・ブラック.
		case LAPE_NTF_NOTICE_ERROR_DCImperfectsettingCyanogen:	//	(uint32_t)0x000000C1	///<	プリンタ関連:ＤＣセット不良・シアン.
		case LAPE_NTF_NOTICE_ERROR_DCImperfectsettingMazenda:	//	(uint32_t)0x000000C2	///<	プリンタ関連:ＤＣセット不良・マゼンダ.
		case LAPE_NTF_NOTICE_ERROR_DCImperfectsettingYellow:	//	(uint32_t)0x000000C3	///<	プリンタ関連:ＤＣセット不良・イエロー.
		case LAPE_NTF_NOTICE_ERROR_DCImperfectsettingBlack:	//	(uint32_t)0x000000C4	///<	プリンタ関連:ＤＣセット不良・ブラック.
		case LAPE_NTF_NOTICE_ERROR_DevCImperfectsettingCyanogen:	//	(uint32_t)0x000000C5	///<	プリンタ関連:ＤｅｖＣセット不良・シアン.
		case LAPE_NTF_NOTICE_ERROR_DevCImperfectsettingMazenda:	//	(uint32_t)0x000000C6	///<	プリンタ関連:ＤｅｖＣセット不良・マゼンダ.
		case LAPE_NTF_NOTICE_ERROR_DevCImperfectsettingYellow:	//	(uint32_t)0x000000C7	///<	プリンタ関連:ＤｅｖＣセット不良・イエロー.
		case LAPE_NTF_NOTICE_ERROR_DevCImperfectsettingBlack:	//	(uint32_t)0x000000C8	///<	プリンタ関連:ＤｅｖＣセット不良・ブラック.
		case LAPE_NTF_NOTICE_ERROR_MisIUInstallationCyanogen:	//	(uint32_t)0x000000C9	///<	プリンタ関連:ＩＵ誤装着・シアン.
		case LAPE_NTF_NOTICE_ERROR_MisIUInstallationMagenta:	//	(uint32_t)0x000000CA	///<	プリンタ関連:ＩＵ誤装着・マゼンタ.
		case LAPE_NTF_NOTICE_ERROR_MisIUInstallationYellow:	//	(uint32_t)0x000000CB	///<	プリンタ関連:ＩＵ誤装着・イエロー.
		case LAPE_NTF_NOTICE_ERROR_MisIUInstallationBlack:	//	(uint32_t)0x000000CC	///<	プリンタ関連:ＩＵ誤装着・ブラック.
		case LAPE_NTF_NOTICE_ERROR_MisDCInstallationCyanogen:	//	(uint32_t)0x000000CD	///<	プリンタ関連:ＤＣ誤装着・シアン.
		case LAPE_NTF_NOTICE_ERROR_MisDCInstallationMazenda:	//	(uint32_t)0x000000CE	///<	プリンタ関連:ＤＣ誤装着・マゼンダ.
		case LAPE_NTF_NOTICE_ERROR_MisDCInstallationYellow:	//	(uint32_t)0x000000CF	///<	プリンタ関連:ＤＣ誤装着・イエロー.
		case LAPE_NTF_NOTICE_ERROR_MisDCInstallationBlack:	//	(uint32_t)0x000000D0	///<	プリンタ関連:ＤＣ誤装着・ブラック.
		case LAPE_NTF_NOTICE_ERROR_MisDevCInstallationCyanogen:	//	(uint32_t)0x000000D1	///<	プリンタ関連:ＤｅｖＣ誤装着・シアン.
		case LAPE_NTF_NOTICE_ERROR_MisDevCInstallationMazenda:	//	(uint32_t)0x000000D2	///<	プリンタ関連:ＤｅｖＣ誤装着・マゼンダ.
		case LAPE_NTF_NOTICE_ERROR_MisDevCInstallationYellow:	//	(uint32_t)0x000000D3	///<	プリンタ関連:ＤｅｖＣ誤装着・イエロー.
		case LAPE_NTF_NOTICE_ERROR_MisDevCInstallationBlack:	//	(uint32_t)0x000000D4	///<	プリンタ関連:ＤｅｖＣ誤装着・ブラック.
		case LAPE_NTF_NOTICE_ERROR_PrinterAbolitionTonerFull:	//	(uint32_t)0x000000D7	///<	プリンタ関連:廃トナーフル.
		case LAPE_NTF_NOTICE_ERROR_PrinterProcessingTrayFormForgetingToTake:	//	(uint32_t)0x000000D8	///<	プリンタ関連:処理トレイ用紙取り忘れ.
		case LAPE_NTF_NOTICE_ERROR_PrinterTranscriptRollerUnitLifeUsrsAreExchange:	//	(uint32_t)0x000000D9	///<	プリンタ関連:転写ローラーユニットライフ(ユーザー交換時).
		case LAPE_NTF_NOTICE_ERROR_PrinterTranscriptRollerUnitLifeServiceExchange:	//	(uint32_t)0x000000DA	///<	プリンタ関連:転写ローラーユニットライフ(サービス交換時).
		case LAPE_NTF_NOTICE_ERROR_PrinterRmvCleanerOzoneFilterLifeUsrsAreExchange:	//	(uint32_t)0x000000DB	///<	プリンタ関連:紙紛除去クリーナー／オゾンフィルタライフ(ユーザー交換時).
		case LAPE_NTF_NOTICE_ERROR_PrinterRmvCleanerOzoneFilterLifeServiceExchange:	//	(uint32_t)0x000000DC	///<	プリンタ関連:紙紛除去クリーナー／オゾンフィルタライフ(サービス交換時).
		case LAPE_NTF_NOTICE_ERROR_PrinterSmokeFilterLife:	//	(uint32_t)0x000000DD	///<	プリンタ関連:噴煙フィルタライフ .
		case LAPE_NTF_NOTICE_ERROR_PrinterPmCounterExaggerated:	//	(uint32_t)0x000000DF	///<	プリンタ関連:ＰＭカウンタオーバー .
		case LAPE_NTF_NOTICE_ERROR_PrinterMisTcInstallationCyanogen:	//	(uint32_t)0x000000E4	///<	プリンタ関連:ＴＣ誤装着・シアン.
		case LAPE_NTF_NOTICE_ERROR_PrinterMisTcInstallationMagenta:	//	(uint32_t)0x000000E5	///<	プリンタ関連:ＴＣ誤装着・マゼンタ.
		case LAPE_NTF_NOTICE_ERROR_PrinterMisTcInstallationYellow:	//	(uint32_t)0x000000E6	///<	プリンタ関連:ＴＣ誤装着・イエロー.
		case LAPE_NTF_NOTICE_ERROR_PrinterMisTcInstallationBlack:	//	(uint32_t)0x000000E7	///<	プリンタ関連:ＴＣ誤装着・ブラック.
		case LAPE_NTF_NOTICE_ERROR_PrinterProhibitionSmallSzLetterheadPaper:	//	(uint32_t)0x000000E8	///<	プリンタ関連:レターヘッド紙小サイズ禁止警告 .
		case LAPE_NTF_NOTICE_ERROR_PrinterTonerCartridgeDefectiveSetCyanogen:	//	(uint32_t)0x000000E9	///<	プリンタ関連:トナーカートリッジセット不良・シアン.
		case LAPE_NTF_NOTICE_ERROR_PrinterTonerCartridgeDefectiveSetMagenta:	//	(uint32_t)0x000000EA	///<	プリンタ関連:トナーカートリッジセット不良・マゼンタ.
		case LAPE_NTF_NOTICE_ERROR_PrinterTonerCartridgeDefectiveSetYellow:		//	(uint32_t)0x000000EB	///<	プリンタ関連:トナーカートリッジセット不良・イエロー.
		case LAPE_NTF_NOTICE_ERROR_PrinterTonerCartridgeDefectiveSetBlack:		//	(uint32_t)0x000000EC	///<	プリンタ関連:トナーカートリッジセット不良・ブラック.
		case LAPE_NTF_NOTICE_ERROR_PrinterZFoldUnitPunchFullRubbishUsrsAreExchange:	//	(uint32_t)0x000000ED	///<	プリンタ関連:Z折りユニットパンチくずフル(ユーザー交換時) .
		case LAPE_NTF_NOTICE_ERROR_PrinterZFoldUnitPunchFullRubbishServiceExchange:	//	(uint32_t)0x000000EE	///<	プリンタ関連:Z折りユニットパンチくずフル(サービス交換時) .
		case LAPE_NTF_NOTICE_ERROR_PrinterMisTcInstallationCyanogenAttentionLv:	//	(uint32_t)0x000000EF	///<	プリンタ関連:ＴＣ誤装着・シアン（ＡＴＴＥＮＴＩＯＮレベル）.
		case LAPE_NTF_NOTICE_ERROR_PrinterMisTcInstallationMagentaAttentionLv:	//	(uint32_t)0x000000F0	///<	プリンタ関連:ＴＣ誤装着・マゼンタ（ＡＴＴＥＮＴＩＯＮレベル）.
		case LAPE_NTF_NOTICE_ERROR_PrinterMisTcInstallationYellowAttentionLv:	//	(uint32_t)0x000000F1	///<	プリンタ関連:ＴＣ誤装着・イエロー（ＡＴＴＥＮＴＩＯＮレベル）.
		case LAPE_NTF_NOTICE_ERROR_PrinterMisTcInstallationBlackAttentionLv:	//	(uint32_t)0x000000F2	///<	プリンタ関連:ＴＣ誤装着・ブラック（ＡＴＴＥＮＴＩＯＮレベル）.
		case LAPE_NTF_NOTICE_ERROR_ScannerRelatedToIrRelatedToAbnormal:	//	(uint32_t)0x000000F9	///<	スキャナー関連:ＩＲ関連異常警告.
		case LAPE_NTF_NOTICE_ERROR_ScannerManuscriptForgetingToTakeOnPlaten:	//	(uint32_t)0x000000FA	///<	スキャナー関連:プラテン上原稿取り忘れ.
		case LAPE_NTF_NOTICE_ERROR_ScannerAdfDoorOpening:	//	(uint32_t)0x000000FB	///<	スキャナー関連:ＡＤＦドアオープン.
		case LAPE_NTF_NOTICE_ERROR_ScannerEdhManuscriptCapacityExaggerated:	//	(uint32_t)0x00000105	///<	スキャナー関連:ＥＤＨ原稿容量オーバー警告.
		case LAPE_NTF_NOTICE_ERROR_ScannerEdhManuscriptSetDefective:	//	(uint32_t)0x00000107	///<	スキャナー関連:ＥＤＨ原稿セット不良警告.
		case LAPE_NTF_NOTICE_ERROR_PrinterThinPaperError:	//	(uint32_t)(0x00000131)	///<	プリンタ関連:薄紙小サイズ用紙セット禁止警告.
		case LAPE_NTF_NOTICE_ERROR_PrinterFuserPlugComesOff:	//	(uint32_t)(0x00000134)	///<	プリンタ関連:定着プラグ抜け警告.
		case LAPE_NTF_NOTICE_ERROR_PrinterIUNearLifeForPNLC:	//	(uint32_t)(0x00000135)	///<	プリンタ関連:Ｃ ＩＵニアライフ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterIUNearLifeForPNLM:	//	(uint32_t)(0x00000136)	///<	プリンタ関連:Ｍ ＩＵニアライフ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterIUNearLifeForPNLY:	//	(uint32_t)(0x00000137)	///<	プリンタ関連:Ｙ ＩＵニアライフ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterIUNearLifeForPNLK:	//	(uint32_t)(0x00000138)	///<	プリンタ関連:Ｋ ＩＵニアライフ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterIULifeForPNLC:	//	(uint32_t)(0x00000139)	///<	プリンタ関連:Ｃ ＩＵライフストップ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterIULifeForPNLM:	//	(uint32_t)(0x0000013A)	///<	プリンタ関連:Ｍ ＩＵライフストップ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterIULifeForPNLY:	//	(uint32_t)(0x0000013B)	///<	プリンタ関連:Ｙ ＩＵライフストップ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterIULifeForPNLK:	//	(uint32_t)(0x0000013C)	///<	プリンタ関連:Ｋ ＩＵライフストップ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterDCNearLifeForPNLC:	//	(uint32_t)(0x0000013D)	///<	プリンタ関連:Ｃ ＤＣニアライフ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterDCNearLifeForPNLM:	//	(uint32_t)(0x0000013E)	///<	プリンタ関連:Ｍ ＤＣニアライフ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterDCNearLifeForPNLY:	//	(uint32_t)(0x0000013F)	///<	プリンタ関連:Ｙ ＤＣニアライフ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterDCNearLifeForPNLK:	//	(uint32_t)(0x00000140)	///<	プリンタ関連:Ｋ ＤＣニアライフ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterDCLifeForPNLC:	//	(uint32_t)(0x00000141)	///<	プリンタ関連:Ｃ ＤＣライフストップ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterDCLifeForPNLM:	//	(uint32_t)(0x00000142)	///<	プリンタ関連:Ｍ ＤＣライフストップ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterDCLifeForPNLY:	//	(uint32_t)(0x00000143)	///<	プリンタ関連:Ｙ ＤＣライフストップ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterDCLifeForPNLK:	//	(uint32_t)(0x00000144)	///<	プリンタ関連:Ｋ ＤＣライフストップ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterDevCNearLifeForPNLC:	//	(uint32_t)(0x00000145)	///<	プリンタ関連:Ｃ 現像ユニットニアライフ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterDevCNearLifeForPNLM:	//	(uint32_t)(0x00000146)	///<	プリンタ関連:Ｍ 現像ユニットニアライフ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterDevCNearLifeForPNLY:	//	(uint32_t)(0x00000147)	///<	プリンタ関連:Ｙ 現像ユニットニアライフ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterDevCNearLifeForPNLK:	//	(uint32_t)(0x00000148)	///<	プリンタ関連:Ｋ 現像ユニットニアライフ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterDevCLifeForPNLC:	//	(uint32_t)(0x00000149)	///<	プリンタ関連:Ｃ 現像ユニットライフストップ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterDevCLifeForPNLM:	//	(uint32_t)(0x0000014A)	///<	プリンタ関連:Ｍ 現像ユニットライフストップ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterDevCLifeForPNLY:	//	(uint32_t)(0x0000014B)	///<	プリンタ関連:Ｙ 現像ユニットライフストップ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterDevCLifeForPNLK:	//	(uint32_t)(0x0000014C)	///<	プリンタ関連:Ｋ 現像ユニットライフストップ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterDevCMaterialNearLifeForPNLK:	//	(uint32_t)(0x0000014D)	///<	プリンタ関連:Ｋ 現像剤ニアライフ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterDevCMaterialLifeForPNLK:	//	(uint32_t)(0x0000014E)	///<	プリンタ関連:Ｋ 現像剤ライフストップ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterFuserUnitNearLifeForPNL:	//	(uint32_t)(0x0000014F)	///<	プリンタ関連:定着ユニットニアライフ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterFuserUnitLifeForPNL:	//	(uint32_t)(0x00000150)	///<	プリンタ関連:定着ユニットライフストップ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterTransBeltUnitNearLifeForPNL:	//	(uint32_t)(0x00000151)	///<	プリンタ関連:転写ベルトニアライフ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterTransBeltUnitLifeForPNL:	//	(uint32_t)(0x00000152)	///<	プリンタ関連:転写ベルトライフストップ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_PrinterExcessOfLifeStopForPNL:	//	(uint32_t)(0x00000153)	///<	プリンタ関連:ライフストップ超過警告検出（オイル指しマーク表示用）.
		case LAPE_NTF_NOTICE_ERROR_YellowColortonerEmptyStop:	///<	0x0000007F プリンタ関連:イエロー色トナーエンプティストップ.[ATTENTION].
		case LAPE_NTF_NOTICE_ERROR_MagentaColortonerEmptyStop:	///<	0x0000007E プリンタ関連:マゼンタ色トナーエンプティストップ.[ATTENTION].
		case LAPE_NTF_NOTICE_ERROR_CyanogenColortonerEmptyStop:	///<	0x0000007D プリンタ関連:シアン色トナーエンプティストップ.[ATTENTION].
		case LAPE_NTF_NOTICE_ERROR_BlackColortonerEmptyStop:	///<	0x00000080 プリンタ関連:ブラック色トナーエンプティストップ.[ATTENTION].
		case LAPE_NTF_NOTICE_ERROR_PrinterRmvCleanerNearLife:	// (0x000001BA) ///<	紙粉除去クリーナーニアライフ(Paper Dust Removal Cleaner Near Life).
		case LAPE_NTF_NOTICE_ERROR_PrinterRmvCleanerLife:		// (0x000001BB)	///<	紙粉除去クリーナーライフ(Paper Dust Removal Cleaner Life).
// comment 2015/01/09 OPS1嵯峨山 [ZeusS] CQ508902 機器構成変更_No.40 DRライフ表示対応 プレニアライフの通知を追加.
		case LAPE_NTF_NOTICE_ATTENTION_DCPreNearLifeForPNLC:	//	(uint32_t)(0x000001C4)	///<	プリンタ関連:C DCプレニアライフ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ATTENTION_DCPreNearLifeForPNLM:	//	(uint32_t)(0x000001C5)	///<	プリンタ関連:M DCプレニアライフ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ATTENTION_DCPreNearLifeForPNLY:	//	(uint32_t)(0x000001C6)	///<	プリンタ関連:Y DCプレニアライフ警告検出（パネル確認用）.
		case LAPE_NTF_NOTICE_ATTENTION_DCPreNearLifeForPNLK:	//	(uint32_t)(0x000001C7)	///<	プリンタ関連:K DCプレニアライフ警告検出（パネル確認用）.
// A3A4共通化FUM CQ.511349 ST 2015.03.24 Sky瀬戸.
//【Accu_ID:1000382】【RQ436】【IWS】 HCL-Jegan 2016/09/02 Add Start.
		case LAPE_NTF_NOTICE_ATTENTION_BackgroundAppDisable:     // IWS backgroung operation warning . 
//【Accu_ID:1000382】【RQ436】【IWS】 HCL-Jegan 2016/09/02 Add End.
#ifdef DEF_A4_Cus
		case LAPE_NTF_NOTICE_ERROR_TCNewTroubleC:	//	(uint32_t)(0x000001C8)	///<	プリンタ関連:ＴＣ新品解除不良・シアン.
		case LAPE_NTF_NOTICE_ERROR_TCNewTroubleM:	//	(uint32_t)(0x000001C9)	///<	プリンタ関連:ＴＣ新品解除不良・マゼンダ.
		case LAPE_NTF_NOTICE_ERROR_TCNewTroubleY:	//	(uint32_t)(0x000001CA)	///<	プリンタ関連:ＴＣ新品解除不良・イエロー.
		case LAPE_NTF_NOTICE_ERROR_TCNewTroubleK:	//	(uint32_t)(0x000001CB)	///<	プリンタ関連:ＴＣ新品解除不良・ブラック.
		case LAPE_NTF_NOTICE_ERROR_ScannerLock:		//	(uint32_t)(0x000001D0)	///<	スキャナロックエラー警告.	// 2015/12/17 VenusMLK OPS2高田(庸) 管理No.311_2_制御構成変更No.32_スキャナロック検出エラー警告の追加.
		// 2016/2/5 Sky草柳 警告通知 新API対応 start.
//2018/10/17[HCLT_Prashannaaa][IT6_Sparrow][AR 1048141]Original size prohibit error
//		case LAPE_NTF_NOTICE_ERROR_ScanSizeDisable_S:		//	(uint32_t)(0x000001E0)	///<	スキャン禁止サイズエラー(モード設定時).
		case LAPE_NTF_NOTICE_ERROR_ScanSizeDisable:			//	(uint32_t)(0x000001E1)	///<	スキャン禁止サイズエラー(スタートキー押下時).
//2018/10/17[HCLT_Prashannaaa][IT6_Sparrow][AR 1048141]Original size prohibit error
//		case LAPE_NTF_NOTICE_ERROR_DupScanSizeDisable_S:	//	(uint32_t)(0x000001E2)	///<	両面スキャン禁止サイズエラー(モード設定時).
		case LAPE_NTF_NOTICE_ERROR_DupScanSizeDisable:		//	(uint32_t)(0x000001E3)	///<	両面スキャン禁止サイズエラー(スタートキー押下時).
	// 2016/2/5 Sky草柳 警告通知 新API対応 end.
// comment 2018/01/05 SCD2 K.Nishiki [Sparrow][制御構成変更No.285]FBの最大読取サイズ変更対応.
// comment Sky森本（弘） 2018/08/28 [Sparrow][問題点_No.1052526] ＡＤＦ原稿セット要求警告タイミング修正.
//		case LAPE_NTF_NOTICE_ERROR_SetADFDocReq2:	///<ＡＤＦ原稿セット要求警告2
// Comment VenusMLK -> 2016/01/29 OPS2 Sky瀬戸 [制御構成変更_No.48]マシン図対応.
		case LAPE_NTF_NOTICE_ERROR_IUNewTroubleC:	//	(uint32_t)(0x000001D1)	///<	プリンタ関連:ＩＵ新品解除不良・シアン.
		case LAPE_NTF_NOTICE_ERROR_IUNewTroubleM:	//	(uint32_t)(0x000001D2)	///<	プリンタ関連:ＩＵ新品解除不良・マゼンダ.
		case LAPE_NTF_NOTICE_ERROR_IUNewTroubleY:	//	(uint32_t)(0x000001D3)	///<	プリンタ関連:ＩＵ新品解除不良・イエロー.
		case LAPE_NTF_NOTICE_ERROR_IUNewTroubleK:	//	(uint32_t)(0x000001D4)	///<	プリンタ関連:ＩＵ新品解除不良・ブラック.
// Comment VenusMLK <- 2016/01/29 OPS2 Sky瀬戸 [制御構成変更_No.48]マシン図対応.
// Comment VenusMLK -> 2016/05/12 OPS2 Sky川名 CQ529099対応.
		case LAPE_NTF_NOTICE_ERROR_PrinterTC_UnsetERROR_C:	//	(uint32_t)(0x000001E5)	///<	プリンタ関連:トナーカートリッジセット不良(ERROR)・シアン.
		case LAPE_NTF_NOTICE_ERROR_PrinterTC_UnsetERROR_M:	//	(uint32_t)(0x000001E6)	///<	プリンタ関連:トナーカートリッジセット不良(ERROR)・マゼンタ.
		case LAPE_NTF_NOTICE_ERROR_PrinterTC_UnsetERROR_Y:	//	(uint32_t)(0x000001E7)	///<	プリンタ関連:トナーカートリッジセット不良(ERROR)・イエロー.
		case LAPE_NTF_NOTICE_ERROR_PrinterTC_UnsetERROR_K:	//	(uint32_t)(0x000001E8)	///<	プリンタ関連:トナーカートリッジセット不良(ERROR)・ブラック.
// Comment VenusMLK -> 2016/05/12 OPS2 Sky川名 CQ529099対応.
// Comment VenusMLK -> 2016/06/06 SCD2 3G Sky川名 CQ:530121対応.
		case LAPE_NTF_NOTICE_ERROR_PrinterEstablishedUnitSetDefective:	//	(uint32_t)0x000000D6	///<	プリンタ関連:定着ユニットセット不良.
// Comment VenusMLK -> 2016/06/06 SCD2 3G Sky川名 CQ:530121対応.
// 2016/11/21 KronosS SCD2荻堂 管理No.管理No_16_29 Lxkコード追加.
#ifdef DEF_KRONOSS_Cus
// 2017/02/28 KronosS SCD2川名 AccuRev:1015493対応.
		case LAPE_NTF_NOTICE_ERROR_MaintenanceKitLifeForPNL:			//	(ulong)(0x000001EE)	///<	メンテナンスキット_ライフ（パネル確認用）.
		case LAPE_NTF_NOTICE_ATTENTION_MaintenanceKitNearLifeForPNL:	//	(ulong)(0x000001EF)	///<	メンテナンスキット_ニアライフ（パネル確認用）.
		case LAPE_NTF_NOTICE_ERROR_FeedTrayUnset2:						//	(uint32_t)(0x000001F2)	///< 給紙トレイセット不良２.
#endif
#endif	// #ifdef DEF_A4_Cus.
// A3A4共通化FUM CQ.511349 ED 2015.03.24 Sky瀬戸.
// Comment 2018/09/26 [HCL_Ranjith][IT6_Eagle][AR1062444] Toner Bottle error support.
// Comment 2019/01/29 【HCLT-Ranjith】 【IT6-Eagle_HI】 【zsz build support】.
#if defined( DEF_EagleCus )
		case LAPE_NTF_NOTICE_ERROR_PrinterTC_UnsetERROR_C:	//	(uint32_t)(0x000001E5)	///<	プリンタ関連:トナーカートリッジセット不良(ERROR)・シアン.
		case LAPE_NTF_NOTICE_ERROR_PrinterTC_UnsetERROR_M:	//	(uint32_t)(0x000001E6)	///<	プリンタ関連:トナーカートリッジセット不良(ERROR)・マゼンタ.
		case LAPE_NTF_NOTICE_ERROR_PrinterTC_UnsetERROR_Y:	//	(uint32_t)(0x000001E7)	///<	プリンタ関連:トナーカートリッジセット不良(ERROR)・イエロー.
		case LAPE_NTF_NOTICE_ERROR_PrinterTC_UnsetERROR_K:	//	(uint32_t)(0x000001E8)	///<	プリンタ関連:トナーカートリッジセット不良(ERROR)・ブラック.
#endif	// defined ( DEF_EagleCus ) || defined ( DEF_EaglePanel_Low ) || defined ( DEF_EaglePanel_Hi ).
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 12, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID and Occurance-Release flag.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X] WarningExist[%d]",SYSF_tickGet(),e_EventID,st_EventData->info.e_Api_Bool,0,0,0 );
			onlyPanel.setWarningNtfEvent( e_EventID, ( Bool )st_EventData->info.e_Api_Bool );
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, st_EventData->info.e_Api_Bool , 12 );
			break;
//comment 2017/08/09 [HCLT-Rengaraaj] [IT5_4.2][Kiki No.2] Adding Attention message when LCT Abnormality occurs - Start.
		case LAPE_NTF_NOTICE_ATTENTION_LCCPaperAbnormalSet:      // LCT Paper Abnormality.
//comment 2017/08/09 [HCLT-Rengaraaj] [IT5_4.2][Kiki No.2] Adding Attention message when LCT Abnormality occurs _ End.
		case LAPE_NTF_NOTICE_StateJam:	//	(uint32_t)0x0007005E   ///<    ジャム状態通知.
		case LAPE_NTF_NOTICE_DoorOpeningState:	//	(uint32_t)0x00070061   ///<    ドアオープン状態通知.
//comment 2017/12/13 [HCLT-Vinod] [IT5_4.2][AR1033679] Adding the conditions without considering Tray abnormal set.
//Comment [HCL_Devipriya] [2018/12/29] [IT6_EagleH] 機器構成変更_No.7_FS540 Staple Unit - Warning support.
		case LAPE_NTF_NOTICE_ERROR_StapleDustBoxFullUser:
		case LAPE_NTF_NOTICE_ERROR_StapleDustBoxFullService:
		case LAPE_NTF_NOTICE_ATTENTION_StapleScrapsFullUser:
		case LAPE_NTF_NOTICE_ATTENTION_StapleScrapsFullService:
		case LAPE_NTF_NOTICE_ATTENTION_StapleNearEmpty:
// comment 2019/03/25 SCD2-Sky谷尾 [EagleH][RQ-720]トレイサポート外用紙セット警告追加.
		case LAPE_NTF_NOTICE_ATTENTION_NotSupportMediaTypeManual:		//	(uint32_t)(0x00000204)	///< 手差トレイ非サポート紙種セット警告.
		case LAPE_NTF_NOTICE_ATTENTION_NotSupportMediaTypeTray1:		//	(uint32_t)(0x00000205)	///< 給紙トレイ1非サポート紙種セット警告.
		case LAPE_NTF_NOTICE_ATTENTION_NotSupportMediaTypeTray2:		//	(uint32_t)(0x00000206)	///< 給紙トレイ2非サポート紙種セット警告.
		case LAPE_NTF_NOTICE_ATTENTION_NotSupportMediaTypeTray3:		//	(uint32_t)(0x00000207)	///< 給紙トレイ3非サポート紙種セット警告.
		case LAPE_NTF_NOTICE_ATTENTION_NotSupportMediaTypeTray4:		//	(uint32_t)(0x00000208)	///< 給紙トレイ4非サポート紙種セット警告.
		case LAPE_NTF_NOTICE_ATTENTION_NotSupportMediaTypeLCT:			//	(uint32_t)(0x00000209)	///< 外付けLCT非サポート紙種セット警告.
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 13, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]",SYSF_tickGet(),e_EventID,0,0,0,0);
			winfo.event_id = e_EventID;										                   // 取得したいイベントのID指定.
			winfo.info_len = sizeof( bl_Info );			                                       // 取得したい型のサイズ指定.
			winfo.info = &bl_Info;						                                       // 詳細データ取得用変数のアドレス指定.
			e_WinfoRes = ntfy.getWarningInfo( e_EventID, &winfo );			                   // 詳細データ取得API.
			if( LAPE_ResultOK != e_WinfoRes ) {
				break;                                                                         // 処理結果が異常終了の場合は、処理を抜ける.
			}
			onlyPanel.setWarningNtfEvent( e_EventID, ( Bool )bl_Info );
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, bl_Info , 13 );
			break;
		}
// Comments 2017/08/07 [HCL_Ramasamy][ IT5_4.2][RQ-693_WP-2_WP-4][AR1024652] Abnormal Tray state Error Warning.
		case LAPE_NTF_NOTICE_ERROR_TrayAbnormalSet:				// (ulong)( 0x000001F1) Abnormal Tray state Warning.
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 14, 0 );
			winfo.event_id = e_EventID;										                     // 取得したいイベントのID指定.
			winfo.info_len = sizeof( bl_Info );			                                       // 取得したい型のサイズ指定.
			winfo.info = &bl_Info;						                                         // 詳細データ取得用変数のアドレス指定.
			e_WinfoRes = ntfy.getWarningInfo( e_EventID, &winfo );			                     // 詳細データ取得API.
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]  Bool_Info[%d] WinfoResult[%d]",SYSF_tickGet(),e_EventID,bl_Info,e_WinfoRes,0,0);
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, bl_Info , 14 );
			if( LAPE_ResultOK != e_WinfoRes ) {
				break;                                                                             // 処理結果が異常終了の場合は、処理を抜ける.
			}
// Comments 2017/10/12 [HCL_Ramasamy][IT5_4.2][RQ-693_Additional_WP_1312-1324 ][AR1024652] Abnormal Tray state Warning and Image guidance support.
//			onlyPanel.setWarningNtfEvent( e_EventID, ( Bool )bl_Info );
			if( bl_Info ){
				onlyPanel.setWarningNtfEvent( e_EventID, ( Bool )bl_Info );
// comment 2018/01/09 [HCL_Ramasamy][IT5_4.2] [AR1032596] Abnormal warning gets automatically cancelled, the Job does not restart Issue Fix.
//				onlyPanel.setAbnormalTrayStateWarningStatus( True );
			} else {
// comment 2018/01/09 [HCL_Ramasamy][IT5_4.2] [AR1032596] Abnormal warning gets automatically cancelled, the Job does not restart Issue Fix.
//				onlyPanel.setAbnormalTrayStateWarningStatus( False );
				onlyPanel.setAbnormalTrayStateWarningStatus( True );
			}
			break;
		}
// comment 2018/06/01 SCD2-Sky Kawanishi [VirusScanFUM] add.
		case LAPE_NTF_NOTICE_ATTENTION_VirusDetection:
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 15, 0 );
			winfo.event_id = e_EventID;										                     // 取得したいイベントのID指定.
			winfo.info_len = sizeof( bl_Info );			                                       // 取得したい型のサイズ指定.
			winfo.info = &bl_Info;						                                         // 詳細データ取得用変数のアドレス指定.
			e_WinfoRes = ntfy.getWarningInfo( e_EventID, &winfo );			                     // 詳細データ取得API.
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent  TickValue[%lu] EventID[0x%08X]  Bool_Info[%d] WinfoResult[%d]",SYSF_tickGet(),e_EventID,bl_Info,e_WinfoRes,0,0);
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, bl_Info , 15 );
			if( LAPE_ResultOK != e_WinfoRes ) {
				break;                                                                             // 処理結果が異常終了の場合は、処理を抜ける.
			}
			if (True == bl_Info) {
//				onlyPanel.setPanelWarning(PNL_ERROR_VirusDetectAttention);
			}
			else {
				onlyPanel.resetPanelWarning(PNL_ERROR_VirusDetectAttention);
			}
			onlyPanel.setWarningNtfEvent( e_EventID, ( Bool )bl_Info );
			break;
		}
		case LAPE_NTF_NOTICE_ERROR_CAUTION_PaperEmpty:	//	(uint32_t)0x0000001B	///<	ペーパーエンプティ(動作中).
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 16, 0 );
			LAPS_NTF_SRV_Warning* st_NTF_SRV_Warning = ( LAPS_NTF_SRV_Warning* )( &st_EventData->info );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X] WarningON[%d]",SYSF_tickGet(),e_EventID,st_NTF_SRV_Warning->ul_WarningOn,0,0,0);
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, (char)( st_NTF_SRV_Warning->ul_WarningOn ) , 16 );
			if( PNLC_NTF_Warning_On == st_NTF_SRV_Warning->ul_WarningOn ) {			// 警告発生の場合.
				procBuzzerWarningExist( option );									// ブザーを鳴らす警告発生.
			}
			break;
		}
		case LAPE_NTF_NOTICE_ATTENTION_PatternFileAvailability:
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 17, 0 );
			winfo.event_id = e_EventID;										                   // 取得したいイベントのID指定.
			winfo.info_len = sizeof( bl_Info );			                                       // 取得したい型のサイズ指定.
			winfo.info = &bl_Info;						                                       // 詳細データ取得用変数のアドレス指定.
			e_WinfoRes = ntfy.getWarningInfo( e_EventID, &winfo );			                   // 詳細データ取得API.
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]  Bool_Info[%d] WinfoResult[%d]",SYSF_tickGet(),e_EventID,bl_Info,e_WinfoRes,0,0);
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, bl_Info , 17 );
			if( LAPE_ResultOK != e_WinfoRes ) {
				break;                                                                         // 処理結果が異常終了の場合は、処理を抜ける.
			}
			onlyPanel.setWarningNtfEvent( e_EventID, ( Bool )bl_Info );
			break;
		}
		case LAPE_NTF_NOTICE_SpecialModeCardLineMode:			///<	0x0004002F 起動時に特別なモード(カードラインモード)で起動したか.
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 18, 0 );
			API_BOOL is_SpecialModeCardLineMode = False;					// 取得したい型で詳細データ取得用変数を宣言.
			winfo.event_id = e_EventID;										// 取得したいイベントのID指定.
			winfo.info_len = sizeof( is_SpecialModeCardLineMode );			// 取得したい型のサイズ指定.
			winfo.info = &is_SpecialModeCardLineMode;						// 詳細データ取得用変数のアドレス指定.
			e_WinfoRes = ntfy.getWarningInfo( e_EventID, &winfo );			// 詳細データ取得API.
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]  SpecialModeCardLineMode[%d] WinfoResult[%d]",SYSF_tickGet(),e_EventID,is_SpecialModeCardLineMode,e_WinfoRes,0,0);
			pc_NtfEventInfo->setCardLineMode( ( Bool )is_SpecialModeCardLineMode );		//pc_NtfEventInfoにラインモードかどうかを保持する.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, is_SpecialModeCardLineMode , 18 );
			break;
		}
		case LAPE_NTF_NOTICE_DevKeyMode:			///<	0x0008006A キーデバイスモード.
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 19, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]",SYSF_tickGet(),e_EventID,0,0,0,0);
			pc_NtfEventInfo->setKeyMode( APIC_KeyDevice::getKeyMode() );
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, pc_NtfEventInfo->getKeyMode() , 19 );
			break;
		}
		case LAPE_NTF_NOTICE_AccessStateChangeUnitApplicationProgram:	///<	0x00080009 アクセス状態変化通知（アプリ単位）.
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 20, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]",SYSF_tickGet(),e_EventID,0,0,0,0);
			APIC_UsbMemDeviceInfo* pc_USBMemoryInfo = APIC_UsbMemDeviceInfo::newInstance();
			if( NULL == pc_USBMemoryInfo ) {
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
				break;
			}
			pc_NtfEventInfo->setUSBReadTurnChange( pc_USBMemoryInfo->isAccess( TYPE_UsbMemAccessRead, TYPE_UsbMemAppIdStorageAccessApp ) );		//USB読み込み状態保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, pc_NtfEventInfo->getUSBReadTurnChange() , 20 );
			break;
		}
		case LAPE_NTF_NOTICE_PaperStatus:				// 用紙状態
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 21, 0 );
			TYPE_PaperStatus e_beforeStatus = TYPE_PaperStatusEmpty;
			TYPE_PaperStatus e_nowStatus = TYPE_PaperStatusEmpty;
			TYPE_PaperStatus e_ChangeStatus = TYPE_PaperStatusEmpty;
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]",SYSF_tickGet(),e_EventID,0,0,0,0);
			// 用紙状態Set処理.
			for( ushort us_Tray = 0 ; us_Tray < TYPE_FeedTrayMax ; us_Tray++ ) {
				TYPE_FeedTray e_FeedTray = e_AryShowFeedTrayTbl2[us_Tray];
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
				e_beforeStatus = pc_NtfEventInfo->getPaperStatus( e_FeedTray );
				pc_NtfEventInfo->setPaperStatus( e_FeedTray, ppc_FeedTray->getPaperStatus( e_FeedTray ) );
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
				e_nowStatus = pc_NtfEventInfo->getPaperStatus( e_FeedTray );
				if( e_beforeStatus != e_nowStatus ){
					e_ChangeStatus = e_nowStatus;
				}
			}
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, e_ChangeStatus , 21 );
			break;
		}
		case LAPE_NTF_NOTICE_TrayStatus:				// 給紙トレイ状態の変化
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 22, 0 );
			TYPE_TrayStatus e_beforeStatus = TYPE_TrayStatusNon;
			TYPE_TrayStatus e_nowStatus = TYPE_TrayStatusNon;
			TYPE_TrayStatus e_ChangeStatus = TYPE_TrayStatusNon;
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]",SYSF_tickGet(),e_EventID,0,0,0,0);
			// トレイ状態Set処理.
			for( uchar uc_Tray = 0; TYPE_FeedTrayMax > uc_Tray; uc_Tray++ ) {
				TYPE_FeedTray e_FeedTray = e_AryShowFeedTrayTbl2[uc_Tray];
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
				e_beforeStatus = pc_NtfEventInfo->getTrayStatus( e_FeedTray );
				pc_NtfEventInfo->setTrayStatus( e_FeedTray, ppc_FeedTray->getTrayStatus( e_FeedTray ) );
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
				e_nowStatus = pc_NtfEventInfo->getTrayStatus( e_FeedTray );
				if( e_beforeStatus != e_nowStatus ){
					e_ChangeStatus = e_nowStatus;
				}
			}
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			// いずれかのトレイが使用可能に変化した場合にON状態を記録させる.
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, e_ChangeStatus , 22 );
			break;
		}
		case LAPE_NTF_NOTICE_ServiceModeOn:				// サービスモード.
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 23, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]",SYSF_tickGet(),e_EventID,0,0,0,0);
			// isEnlCombinationExist()を一部イベント化.
			Bool b_ServiceModeOn = ppc_SystemStatus->getServiceModeOn();
			pc_NtfEventInfo->setServiceModeOn( b_ServiceModeOn );		// データの管理クラスにサービスモードを保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, b_ServiceModeOn , 23 );
		}
		break;
		case LAPE_NTF_NOTICE_DeviceLockOn:		// デバイスロック中.
		{

// comment 2020/12/16 2DWS2G-Sky Nakano クラウドSSO認証開発v1_Step2 [OP_BTS-29855].
			PnlComSetUserAuthenticationMode();

// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 24, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]",SYSF_tickGet(),e_EventID,0,0,0,0);
			// PBasicMessageProc::isSpecialGamenShowStatus()の一部イベント化.
			Bool b_DeviceLockOn = ppc_SystemStatus->getDeviceLockOn();
// comment 2019/05/22 SCD2-4G Sky 二口 [EagleH][OP_BTS-3133] パワーセーブから復帰後、OpenAPI使用中画面が終了しない.	
			Bool b_PanelDeviceLockOn = pc_NtfEventInfo->getDeviceLockOn();		// パネルのデバイスロック状態を取得する.
			pc_NtfEventInfo->setDeviceLockOn( b_DeviceLockOn );			// データの管理クラスにデバイスロック状態を保存.
// comment 2023/12/05 FPT-HungDm17 [IT6_2.3][OP_BTS-45944]ユーザー名入力画面で、システムオートリセットが働かない問題対策.
			if( True == b_DeviceLockOn ) {
				PNLC_PopupListControl* popupList = PNLC_PopupListControl::newInstance();
				if( NULL != popupList
				 && ( NULL != dynamic_cast<PCPC_UserAccountUserInputName_act*>( popupList->getNowPopup() )
				 || NULL != dynamic_cast<PCPC_UserAccountDomainName_act*>( popupList->getNowPopup() )
				 || NULL != dynamic_cast<PCPC_PrintJobListRunnigList_job*>( popupList->getNowPopup() )
				 || NULL != dynamic_cast<PCPC_AuthOapInputTextExt_act*>( popupList->getNowPopup() ) ) ){
					popupList->addPopup( &gp_RemoteUserAccess_act, NULL, NULL, 140, 138, 280, 276, False, True );
					nextGamen = NULL;
				}
			}
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, b_DeviceLockOn , 24 );
// comment 2020/05/13 Sky中野 [IT6_2.0MR2][OP_BTS-23419].
			Bool bl_IWSSetUpStatus = APIC_BrowserInfo::newInstance()->getIWSSetUpStatus();
// comment 2015/10/07 OPS1-Sky浜野 [MinervaSBK][CQ519840] デバイスロック処理対応.
			if( True == b_DeviceLockOn ) {
// comment 2019/05/22 SCD2-4G Sky 二口 [EagleH][OP_BTS-3133] パワーセーブから復帰後、OpenAPI使用中画面が終了しない.	
				if ( False == b_PanelDeviceLockOn ) {
					pc_NtfEventInfo->setNotRunDeviceLock( True );
				}
// comment 2020/05/13 Sky中野 [IT6_2.0MR2][OP_BTS-23419].
				// デバイスロック開始時のIWS設定状態を保持しておく.
				onlyPanel.setIWSSetUpStatus( bl_IWSSetUpStatus );
			}
// comment 2018/10/22 SCD2-4G 小畑 [Eagle][AR1063625].
			else
			{
// comment 2019/05/22 SCD2-4G Sky 二口 [EagleH][OP_BTS-3133] パワーセーブから復帰後、OpenAPI使用中画面が終了しない.	
// comment 2019/08/26 SCD2-4G Sky 二口 [EagleH][OP_BTS-11849] .	
//				pc_NtfEventInfo->setNotRunDeviceLock( False );
				// 給紙トレイ情報への反映.
				// トレイ選択テーブル.
				TYPE_FeedTray e_FeedTray[] = {
					TYPE_FeedTrayManual,	// トレイ選択：手差し.
					TYPE_FeedTray1,			// トレイ選択：トレイ1.
					TYPE_FeedTray2,			// トレイ選択：トレイ2.
					TYPE_FeedTray3,			// トレイ選択：トレイ3.
					TYPE_FeedTray4,			// トレイ選択：トレイ4.
					TYPE_FeedTray5,			// トレイ選択：トレイ5.
				};
				for( ushort i=0; i<sizeof( e_FeedTray )/sizeof( TYPE_FeedTray ); i++ ) {
					TYPC_PaperType c_PaperType = PnlComGetFeedTray()->getPaperType( e_FeedTray[i] );	// 用紙種類取得.
					// 給紙トレイ情報のカスタム用紙属性が未選択でないとき.
					if( TYPE_CustomPaperType_UnSelect != c_PaperType.getCustomPaperType() ) {
						PNLC_SelectPaperTrayCom_DataLib* pDataLib = PNLC_SelectPaperTrayCom_DataLib::newInstance();
						if ( NULL != pDataLib ) {
							// カスタム用紙属性から用紙種類取得.
							TYPE_PaperType e_PaperType = pDataLib->getPaperTypeFromCustomPaperType( c_PaperType.getCustomPaperType() );
// comment 2021/07/21 Sky東島 [IT6_2.2][OP_BTS-34788].
							Bool bl_PaperTypeNormalEx = False;
							if( TYPE_PaperTypeNormalEx == e_PaperType ) {				// 普通紙(Ex).
								e_PaperType = TYPE_PaperTypeNormal;
								bl_PaperTypeNormalEx = True;
							}
							if( e_PaperType != c_PaperType.getPaperType() ){
								// 用紙種類が変更されている場合は両面2面目を解除する.
								c_PaperType.setFeed2ndSidedPaper( False );
							}
							c_PaperType.setPaperTypeNormalEx( bl_PaperTypeNormalEx );
							c_PaperType.setPaperType( e_PaperType );
							PnlComGetFeedTray()->setPaperType( e_FeedTray[i] , c_PaperType );
						}
					}
				}
// comment 2020/05/13 Sky中野 [IT6_2.0MR2][OP_BTS-23419].
				if( ( True == pc_NtfEventInfo->getNotRunDeviceLock() )
				 && ( False == onlyPanel.getIWSSetUpStatus() )
				 && ( onlyPanel.getIWSSetUpStatus() != bl_IWSSetUpStatus ) ) {
					// デバイスロックON時にIWS設定状態を取得できなかったため、再取得.
					onlyPanel.setIWSSetUpStatus( bl_IWSSetUpStatus );
				}
			}
			break;
		}
		case LAPE_NTF_NOTICE_AdminModeOnApp:		// 管理者モード中（使用アプリ）.
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 25, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent  EventID[0x%08X]",e_EventID,0,0,0,0,0);
			// PBasicMessageProc::isSpecialGamenShowStatus()の管理者モードエッジ検出処理を一部イベント化
			TYPE_AdminModeApp e_AdminModeApp = ppc_SystemStatus->getAdminModeOnApp();
			pc_NtfEventInfo->setAdminModeOnApp( e_AdminModeApp );			// データの管理クラスに管理者モード中（使用アプリ）を保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, e_AdminModeApp , 25 );
			break;
		}
		case LAPE_NTF_NOTICE_InputStatusForProgramJob:		// プログラムジョブ用Inputステータス.
		case LAPE_NTF_NOTICE_OutputStatusForProgramJob:		// プログラムジョブ用Outputステータス.
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 26, 0 );

//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]",SYSF_tickGet(),e_EventID,0,0,0,0);
			// isStatusChange()の一部イベント化
			TYPE_ProgJobInStatus DataCheckProgInput		= PnlComGetSystemStatus()->getProgJobInStatus();	// プログラムジョブ入力状態.
			pc_NtfEventInfo->setProgJobInStatus( DataCheckProgInput );		//データの管理クラスにプログラムジョブ入力状態を保存.

			TYPE_ProgJobOutStatus DataCheckProgOutput	= PnlComGetSystemStatus()->getProgJobOutStatus();	// プログラムジョブ出力状態.
			pc_NtfEventInfo->setProgJobOutStatus( DataCheckProgOutput );		//データの管理クラスにプログラムジョブ出力状態を保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			if( LAPE_NTF_NOTICE_InputStatusForProgramJob == e_EventID ){
				PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, DataCheckProgInput , 26 );
			} else if( LAPE_NTF_NOTICE_OutputStatusForProgramJob == e_EventID  ){
				PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, DataCheckProgOutput , 26 );
			}
			break;
		}

		case LAPE_NTF_NOTICE_ChangePowerManagementState:		// 電源管理状態変化通知.
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 27, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]",SYSF_tickGet(),e_EventID,0,0,0,0);
			// isEnergySaveChange()のイベント化.
			TYPE_PowerSaveStatus e_PowerSaveStatus = ppc_Power->getPowerSaveStatus();
			pc_NtfEventInfo->setPowerSaveStatus( e_PowerSaveStatus );		//データの管理クラスに電源管理ステータスを保存.
// Comment 2017/03/10 HCLT-Omkar [IT5_4.0] [AR1009653] Flag bl_ServiceMode is reset when the MFP is returned from the sub power off mode.
			Bool bl_isBootSelfCheckMode = APIC_SystemInfo::newInstance()->isBootSelfCheckMode();
			if ( ( e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOnProc ) && ( False == bl_isBootSelfCheckMode ) ) {
				onlyPanel.bl_ServiceMode = False;
			}
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, e_PowerSaveStatus , 27 );

			break;
		}
		case LAPE_NTF_NOTICE_TwainLock:			// Twainロック状態.
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 28, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]",SYSF_tickGet(),e_EventID,0,0,0,0);
			// isScanJobSendChange()を一部イベント化.

			TYPE_TwainLock e_TwainLock = ppc_SystemStatus->getTwainLock();
			pc_NtfEventInfo->setTwainLock( e_TwainLock );			// データの管理クラスに管理者モード中（使用アプリ）を保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, e_TwainLock , 28 );

			break;
		}
		case LAPE_NTF_NOTICE_BoxReady:	// データ整合状態チェック完了状態.
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 29, 0 );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]",SYSF_tickGet(),e_EventID,0,0,0,0);
			pc_NtfEventInfo->setBoxReady( True );	// 完了状態.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, True , 29 );
			break;
// comment 2018/03/23 HCLT-Ravi [IT6_Eagle][制御構成変更_No.82] Checking Attention for Fax error message display.
		case LAPE_NTF_NOTICE_ATTENTION_FaxResourceWarning:
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 30, 0 );
			PNLE_ATTENTION_ResourceWarning us_FaxResourceWarning;
//			winfo.event_id = e_EventID;
			winfo.event_id = LAPE_NTF_NOTICE_FaxResourceWarning;
			winfo.info_len = sizeof( us_FaxResourceWarning );
			winfo.info = &us_FaxResourceWarning;
//			e_WinfoRes = ntfy.getWarningInfo( e_EventID, &winfo );
			e_WinfoRes = ntfy.getWarningInfo( LAPE_NTF_NOTICE_FaxResourceWarning, &winfo );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]  FaxResourceWarning[%d] WinfoResult[%d]",SYSF_tickGet(),e_EventID,us_FaxResourceWarning,e_WinfoRes,0,0);
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, (char)us_FaxResourceWarning , 30 );
			if( LAPE_ResultOK != e_WinfoRes ) {
				break;
			}
			pc_NtfEventInfo->setResourceWarning( us_FaxResourceWarning );
			break;
		}
//Comment 2018/07/31 HCL_Devipriya 制御構成変更_No.104 Callback received for TestPatternPrint request from NewAPI.
		case LAPE_NTF_NOTICE_TestPatternPrintRequest:
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 31, 0 );
			TYPS_TestPatternPrintInfo st_TestPatternPrintInfo;
			PNLS_JobPrintSettingData* p_st_JobPrintSettingData = gp_FactoryJobPrintSetting.getJobPrintSettingData();
			// To do call test pattern request API here.
			winfo.event_id = e_EventID;										// 取得したいイベントのID指定.
			winfo.info_len = sizeof( TYPS_TestPatternPrintInfo );			// 取得したい型のサイズ指定.
			winfo.info = &st_TestPatternPrintInfo;						// 詳細データ取得用変数のアドレス指定.
			e_WinfoRes = ntfy.getWarningInfo( e_EventID, &winfo );			// 詳細データ取得API.
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X] WinfoResult[%d]",SYSF_tickGet(),e_EventID,e_WinfoRes,0,0,0);
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, e_WinfoRes , 31 );
			if( LAPE_ResultOK != e_WinfoRes ) {
				break;
			}
			p_st_JobPrintSettingData->us_pattern 			= static_cast<ushort> ( st_TestPatternPrintInfo.ul_Pattern );
			p_st_JobPrintSettingData->us_numOfCopies 		= static_cast<ushort> ( st_TestPatternPrintInfo.ul_JobSetting );
			p_st_JobPrintSettingData->us_singleMulti 		= static_cast<ushort> ( st_TestPatternPrintInfo.uc_SingleMultiType );
			p_st_JobPrintSettingData->us_duplexMode 		= static_cast<ushort> ( st_TestPatternPrintInfo.uc_DuplexSet );
			p_st_JobPrintSettingData->us_printFace 			= static_cast<ushort> ( st_TestPatternPrintInfo.uc_SurfaceSet );
			p_st_JobPrintSettingData->us_paperTray 			= static_cast<ushort> ( st_TestPatternPrintInfo.uc_PaperPortSet );
			p_st_JobPrintSettingData->us_paperSize 			= static_cast<ushort> ( st_TestPatternPrintInfo.uc_PaperSize );
			p_st_JobPrintSettingData->us_paperType 			= static_cast<ushort> ( st_TestPatternPrintInfo.uc_PaperType );
			p_st_JobPrintSettingData->us_limosMode 			= static_cast<ushort> ( st_TestPatternPrintInfo.uc_LIMOSSelection );
			p_st_JobPrintSettingData->us_screenPattern 		= static_cast<ushort> ( st_TestPatternPrintInfo.uc_ScreenPatter );
			p_st_JobPrintSettingData->us_gradation 			= static_cast<ushort> ( st_TestPatternPrintInfo.uc_Gradation );
			p_st_JobPrintSettingData->us_color 				= static_cast<ushort> ( st_TestPatternPrintInfo.uc_ColorType );
			p_st_JobPrintSettingData->us_direction 			= static_cast<ushort> ( st_TestPatternPrintInfo.uc_Orientation );
			p_st_JobPrintSettingData->us_angle 				= static_cast<ushort> ( st_TestPatternPrintInfo.uc_AngleSet );
			p_st_JobPrintSettingData->us_onOffRatio 		= static_cast<ushort> ( st_TestPatternPrintInfo.uc_RatioMode );
			p_st_JobPrintSettingData->us_reverse 			= static_cast<ushort> ( st_TestPatternPrintInfo.uc_ReverseMode );
			p_st_JobPrintSettingData->us_colorMode 			= static_cast<ushort> ( st_TestPatternPrintInfo.uc_ColorMode );
			p_st_JobPrintSettingData->us_dpi 				= static_cast<ushort> ( st_TestPatternPrintInfo.uc_Resolution );
			p_st_JobPrintSettingData->us_lineMode 			= static_cast<ushort> ( st_TestPatternPrintInfo.uc_numberModeLine );
			p_st_JobPrintSettingData->us_repeatWidth 		= static_cast<ushort> ( st_TestPatternPrintInfo.uc_Repetition );
			p_st_JobPrintSettingData->us_density 			= static_cast<ushort> ( st_TestPatternPrintInfo.uc_Density );
			p_st_JobPrintSettingData->us_backgroundDensity 	= static_cast<ushort> ( st_TestPatternPrintInfo.uc_BgDensity );
			p_st_JobPrintSettingData->us_cdWidth 			= static_cast<ushort> ( st_TestPatternPrintInfo.us_LineWidthLandscape );
			p_st_JobPrintSettingData->us_fdWidth 			= static_cast<ushort> ( st_TestPatternPrintInfo.uc_LineWidthPortrait );
			p_st_JobPrintSettingData->us_dotSpace 			= static_cast<ushort> ( st_TestPatternPrintInfo.uc_DotInterVal );
			p_st_JobPrintSettingData->us_dotGain 			= static_cast<ushort> ( st_TestPatternPrintInfo.uc_DotGain );			
			if ( False == gp_FactoryJobPrintSetting.readRecordFromCsv( True)) { // Execute test patternprint.
				printf("TestPatternPrint NG\n");
			}
			break;
		}
//comment 2018/12/05 SCD2-Sky武井 [Eagle][AR1069370] 【開発提起】【Eagle】IT6の3pain表示対応実機においてIWSのAPIよりパネル上のスタートキー表示を有効にできない。.
		case LAPE_NTF_NOTICE_LedStatus:		// LED状態通知（フッター用）.
			{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 32, 0 );
				LAPS_NTF_LedStatus* pst_LedStatus = ( LAPS_NTF_LedStatus* )( &st_EventData->info );
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID and LEDStatus.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X] LEDState[%d] LEDKind[%d]",SYSF_tickGet(),e_EventID,pst_LedStatus->ss_LEDKind,pst_LedStatus->sc_LEDState,0,0);
				
				// フッターのLED更新.
				PMGC_PanelManager::newInstance()->reqFooterLED( ( PNLE_LEDKind )pst_LedStatus->ss_LEDKind, ( PNLE_LEDState )pst_LedStatus->sc_LEDState );
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
				PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, pst_LedStatus->sc_LEDState , 32 );
			}
			break;

// comment [特ROM対応][IT5-148]【SOLIMP3様】音声操作によるジョブ実行(IWS連携)要望_最終版リリース.
		case LAPE_NTF_NOTICE_IWSRemoteJobEntry:		// IWS_WebAPIからのジョブ投入.
		{
//Comment 2020/08/05 【HCLT_Hemachander】【IT6_2.1】【OP_BTS-26910】【Log Addition Activity】Added logs to display EventID.
			PanelLog( PNLD_LogKind_Active, "LAPE Event Notification-PBasicMessageProc::procCallBackNtfEvent TickValue[%lu] EventID[0x%08X]",SYSF_tickGet(),e_EventID,0,0,0,0);
			if( True == PnlComIsIWSRemoteJobFuncEnable() ) {
				Bool b_IWSRemoteJobEntry = ppc_SystemStatus->getIWSRemoteJobEntry();

				pc_NtfEventInfo->setIWSRemoteJobEntry( b_IWSRemoteJobEntry );			// データの管理クラスにIWSジョブ実行中状態を保存.

				if( True == b_IWSRemoteJobEntry ) {
					pc_NtfEventInfo->setNotRunIWSRemoteJobEntryLock( True );

					// 拡大モード中.
					if( onlyPanel.bl_isEnlargeDisplayMode == True ){
						// 拡大表示解除.
						onlyPanel.bl_isEnlargeDisplayMode = False;			// モードの切り替え.
						reqLED( PNLE_LED_EnlargeDisp, PNLE_LEDStateOff );	// 拡大表示LEDの消灯.
					}
				}
			}
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, pc_NtfEventInfo->getNotRunIWSRemoteJobEntryLock() , 33 );

			break;
		}

		default:
		{
// comment 2019/06/28 SCD2-Sky東島 [EagleHigh][OP_BTS-9409]. 新詳細コード拡張対応.
			// ログ保存.
// comment 2021/08/31 Sky東島 [IT6_MinervaSSBK][OP_BTS-34811].
//			PNL_LogManager::newInstance()->evnetLog( this, MS_CallBackNtfEvent, 255, 0 );
			PNL_LogManager::newInstance()->NtfEventLog( this, e_EventID, False , 255 );
			char c_LogfMessageDef[128] = {};
			snprintf( c_LogfMessageDef, sizeof(c_LogfMessageDef), "LAPE_NTF_NOTICE...default. e_EventID:0x%08X", e_EventID );
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_INFO, c_LogfMessageDef );
			break;
		}
	}
}

/*================================================================================================
［機能］	基本メッセージ処理 : タブレット接続通知.
================================================================================================*/
void PBasicMessageProc::procTabletConnect( uchar option )
{
}

/*================================================================================================
［機能］	基本メッセージ処理 : タブレット切断通知.
================================================================================================*/
void PBasicMessageProc::procTabletDisconnect( uchar option )
{
}

/*================================================================================================
［機能］	基本メッセージ処理 : 本体パネルタッチ時に切断確認通知.
================================================================================================*/
void PBasicMessageProc::procTabletMainPanelTouchNotice( uchar option )
{
	APIC_TabletIF* pc_TabletIF = APIC_TabletIF::newInstance();
	APIC_OperationSetting* pc_Setting = APIC_OperationSetting::newInstance();
	if( NULL == pc_TabletIF || NULL == pc_Setting ) {
		return;
	}
	// 機能拡張3以降で管理者設定(継続接続確認表示)ON、端末接続中に本体パネル通知されたとき.
	if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type3()										// 機能拡張3以降.
	    && TYPE_KeepTabletOperationConfirmDispOn == pc_Setting->getKeepTabletOperation()		// タブレット継続操作確認表示あり.
	    && True == pc_TabletIF->getConnectState() ) {											// 端末接続中.
		if( False == onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, PNL_ATTENTION_TabletMainPanelTouchNotice ) ) {
			onlyPanel.setPanelWarning( PNL_ATTENTION_TabletMainPanelTouchNotice );				// PNL_ATTENTION_TabletMainPanelTouchNotice警告セット.
		}
	}
}

//================================================================================================
// 概要		：画面色反転機能指定の設定関数.
// 戻り値	：なし.
//================================================================================================
// [引数] bl_isReference	True : 画面色反転機能の使用する/しないのチョイスを取得し、Qtへ通知する.
//							False: 反転機能を「使用しない」をQtへ通知する.
//================================================================================================
void PBasicMessageProc::setDispColorReversePanel( Bool bl_isReference )
{
	if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type3() ) {					// 機能拡張3以降.
		Bool bl_isInvertMode = False;
		if( True == bl_isReference ) {
			APIC_OperationSetting* pc_OperationSetting = PnlComGetOperationSetting();
			if( NULL != pc_OperationSetting ) {
				bl_isInvertMode = ( ( TYPE_DispColorReverseDisable == pc_OperationSetting->getDispColorReverse() ) ? False : True );
			}
		}
		// パネル描画タスクへ処理を移動.
		PNLC_QWSConnector* pc_QWSConnector = PNLC_QWSConnector::newInstance();
		if( 0 != pc_QWSConnector )
		{
			pc_QWSConnector->setInvertMode( bl_isInvertMode );
		}
	}
}

/*================================================================================================
//概要    :画面ズーム機能の初期化要求.
//備考    :.
================================================================================================*/
void PBasicMessageProc::procInitZoomFactor()
{
	// QtListを使用しない画面では「PLBC_MultiTouchFunction::getMultiTouchMode()」の.
	// 戻り値が「PNLE_MTUIMODE_NONE」となる為、以下の条件を使用.
	// 機能拡張3以降 且つ マルチタッチ有効の場合.
	if( ( True == PLBC_FuncVersionCheck::isOK_MiZe_Type3() )
	    && ( False == cp_SoftDipSw->getSoftDipSwBitForPanelSpecNo( PNLD_MTUIGESTURE, 0 ) ) ) {
		PNLC_QWSConnector* pc_QWSConnector = PNLC_QWSConnector::newInstance();
		APIC_OperationSetting* pc_OperationSetting = PnlComGetOperationSetting();
		if( ( NULL != pc_QWSConnector ) && ( NULL != pc_OperationSetting ) ) {
			// 画面ズーム機能を使用する場合のみ.
			if( TYPE_DispZoomEnable == pc_OperationSetting->getDispZoom() ) {
				// 画面ズーム機能の倍率およびオフセットの初期化を要求する.
				pc_QWSConnector->initZoomFactor();
// comment 2015/04/30 OPS1-Sky佐藤 [HeliosL][CQ513137] ズームキーの登録キー割り当て対応.
				// 外キーによる現在のズーム倍率設定を初期化する.
				e_ZoomStatus = PNLE_ZOOM_SATUS_MIN;
			}
		}
	}
}

void PBasicMessageProc::procRMPStrInput( uchar option ) {
	PNLC_RMPStrInputMng::newInstance()->StrToKbd();
	setupGamen();
}

// comment 2015/04/30 OPS1-Sky佐藤 [HeliosL][CQ513137] ズームキーの登録キー割り当て対応.
/*================================================================================================
//概要    : 画面ズーム倍率の設定要求.
//備考    :.
//変更履歴: 2015/04/30	Sky佐藤 ズームキーの登録キー割り当て対応 新規作成.
================================================================================================*/
void PBasicMessageProc::procZoomIn()
{
	qreal dl_ZoomStatus = 1.0;
	Bool bl_result = False;
	PNLE_ZOOM_SATUS e_ZoomStatusTemp = PNLE_ZOOM_SATUS_MIN;

	PNLC_QWSConnector* pc_QWSConnector = PNLC_QWSConnector::newInstance();
	APIC_OperationSetting* pc_OperationSetting = PnlComGetOperationSetting();

	if ( ( NULL == pc_QWSConnector ) || ( NULL == pc_OperationSetting ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}

	// 画面ズーム機能を使用する場合のみ.
	if ( TYPE_DispZoomEnable == pc_OperationSetting->getDispZoom() ) {
		// 表示倍率を変更(以下の倍率をローテーションする).
		// 1.0倍 → 1.2倍 → 1.5倍 → 2.0倍.
		if ( PNLE_ZOOM_SATUS_MAX == e_ZoomStatus ) {
			// 現在のズーム倍率が最大の場合は等倍に戻る.
			e_ZoomStatusTemp = tbl_ZoomLevel[0].ZoomStatus;
		} else {
			e_ZoomStatusTemp = tbl_ZoomLevel[e_ZoomStatus+1].ZoomStatus;
		}

		// ズーム倍率を取得する.
		dl_ZoomStatus = tbl_ZoomLevel[e_ZoomStatusTemp].ZoomLevel;

		// 画面ズーム倍率の設定を要求する.
		bl_result = pc_QWSConnector->setZoomIn( dl_ZoomStatus );

		// 画面ズームが正常におこなわれた場合は現在のズーム倍率を更新する.
		if ( True == bl_result ) {
			e_ZoomStatus = e_ZoomStatusTemp;
		} else {
			// ビープ音を鳴らす.
			setBuzzerType( PCGE_BuzzerType_InputNg );
			return;
		}
	} else {
		// ビープ音を鳴らす.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
}


// ==================================================================
//概要    :次画面インスタンス設定.
// ==================================================================
void PBasicMessageProc::setNextGamen( PBasicMessageProc* pt_NextGamen )
{
	if( b_GUICmpEventFlag == True ) {
		nextGamenCmp = pt_NextGamen;
	}
	else {
		nextGamen = pt_NextGamen;
	}
}

// ==================================================================
//概要    :次画面インスタンス(コンポーネント用)の取得.
// ==================================================================
PBasicMessageProc* PBasicMessageProc::getNextGamenCmp( void )
{
	PBasicMessageProc* tmp_nextGamen = nextGamenCmp;
	nextGamenCmp = NULL;
	return tmp_nextGamen;
}
	
// comment 2015/06/08 HCL-Suresh[Helios][CQ512799] Double Time Launch of PERROR_UserTotalLimit_S Message Warning(A0A3250700) is arrested by changing the gamen warning status in one iteration before.
// ===============================================================================================.
//	Overview		: Full Gamen warning status is updated in this function.
//  Arguments   	: None.
//	Return Value	: void.
//	Change History	: 2015/06/08 HCL-Suresh[Helios][CQ512799].
// ===============================================================================================.
void PBasicMessageProc::updateWarningStatus()
{
	isGamenWarningExist();										// Update Gamen Warning Status.
}

// 当該画面の画面ウィジェットを取得する
PDisplayParts* PBasicMessageProc::getDisplayParts( void )
{
	return m_pc_displayParts;
}

/*================================================================================================
//概要    :IME連携用タブレット端末接続通知.
//備考    :.
================================================================================================*/
void PBasicMessageProc::procTabletIMEConnect( uchar option )
{
	PLBC_GeneralKeybord::startKbdIMESetting( PNLD_IMERUNMODE_REMOTEKEYBOARD );
}
/*================================================================================================
//概要    :IME連携用タブレット端末切断通知.
//備考    :.
================================================================================================*/
void PBasicMessageProc::procTabletIMEDisconnect( uchar option )
{
	PLBC_GeneralKeybord::endKbdIMESetting();
}
/*================================================================================================
//概要    :IME文字入力停止通知.
//備考    :.
================================================================================================*/
void PBasicMessageProc::procStopIME( uchar option )
{
	PLBC_ConvertKeybord* pc_ConvKeyboard = PnlComGetConvertKeybord();
	if( NULL != pc_ConvKeyboard ) {
		pc_ConvKeyboard->reqIMEEnd();
	}
}

// 当該画面のグレーアウト用widgetを取得する
PGrayoutWidget* PBasicMessageProc::getGrayoutParts( void )
{
	return m_pc_grayout;
}
/*================================================================================================
//概要    : タブレット接続/切断時のリセット.
//備考    :.
================================================================================================*/
void PBasicMessageProc::procTabletReset( uchar option )
{
// comment 2015/04/01 OPS1-Sky浜野 [Poseidon][CQ511887] bizhubRA順番待ちでの切断不具合対応.
//	if ( PNLE_TABLETRESET_REASON_CONNECT == option ) {			// 接続時.
//		PLBC_GeneralKeybord::startKbdIMESetting(PNLD_IMERUNMODE_TABLETCONNECT);					// IME開始;
//		onlyPanel.m_bl_TabletMultiTouchEnableState = True;			// ステイタス更新.
//	}
//	else if ( PNLE_TABLETRESET_REASON_DISCONNECT == option ) {	// 切断時.
//		PLBC_GeneralKeybord::endKbdIMESetting();					// IMEを終了する.
//		PMGC_PanelManager::reqIMEEnd();
//		onlyPanel.m_bl_TabletMultiTouchEnableState = False;			// ステイタス更新.
//	}

	// 強制オンフック.
	if( True == PBasicMessageProc::b_isOffHookBtnOn ) {
		// スキャンファクスアプリ以外でのオンフック/オフフックキー押下でのオフフック状態.
		PDPC_FaxCommonApi* pc_FaxCommonApi = PDPC_FaxCommonApi::getInstance();
		if( pc_FaxCommonApi != NULL ) {
			// オンフック（回線断）実行
			pc_FaxCommonApi->OnHook( TYPE_CommLine1 );
			// ファクス回線1へのコマンド要求状態フラグ（FAXE_CmdRsDummy:要求中でない）
			PBasicMessageProc::e_isFaxLine1CmdReq = FAXE_CmdRsDummy;
			PBasicMessageProc::b_isOffHookBtnOn = False;
			// オンフックキー押下での復帰情報をセット
			PNLC_ScanFaxDataLib::newInstance()->setChangeOnHook( True );
		}
	}

	// 認証パラメータを初期化する.
	if( False == PnlComGetUserAccountOn() ) {
		// 拡張サーバー認証の場合
		TYPE_UserAuthentication e_UserAuthentication = PnlComGetUserAuthentication()->getUserAuthentication(PNLC_NetworkLan::getInstance()->getLine());
		if( ( TYPE_UserAuthenticationIntermeddiateServer == e_UserAuthentication )	// 拡張サーバー認証.
		    || ( ( TYPE_UserAuthenticationDeviceAndMidAuth == e_UserAuthentication ) ) ) {	// 外部サーバー＋拡張サーバー
			// ブラウザ認証連携の初期化.
			gp_AuthOapBrowserLogin.resetBrowserAuth();
			// 認証連携BrowserアプリTop画面に次回戻るタイミングで、ブラウザを再起動する.
			gp_AuthOapBrowserWait.setBrowserAuthReoot( True );
			PCGC_AuthOapLibrary::comAutoReset();
			PnlComReqAuthModuleRestart();								// 認証ポーリングリスタートリクエスト.
			PnlComReqAuthPollingRestart();								// 認証ポーリング再開要求.
		}
	}

	PNLC_TabletSecurityControlLib* pc_TabletSecurityControl = PNLC_TabletSecurityControlLib::newInstance();
	if( NULL != pc_TabletSecurityControl ) {
		pc_TabletSecurityControl->securityAreaExited();
	}

	// リセット制御を初期化する.
	PNLC_TabletResetControlLib::newInstance()->clearWait();
}


/*================================================================================================
//概要    : タブレット接続/切断時の共通処理.
//備考    :.
================================================================================================*/
void PBasicMessageProc::procTabletConnectChange( MessageType message, uchar option )
{
	// リセット制御ライブラリインスタンス.
	PNLC_TabletResetControlLib* pTabletResetControlLib = PNLC_TabletResetControlLib::newInstance();
	if( NULL == pTabletResetControlLib ) {
		return;
	}

	// 状態が合わない場合は、処理を実施しない.
	if( ( ( message == MS_TabletDisconnect ) && ( False == onlyPanel.m_bl_TabletMultiTouchEnableState ) )	// 切断.
	    || ( ( message == MS_TabletConnect ) && ( True == onlyPanel.m_bl_TabletMultiTouchEnableState ) ) ) {	// 接続.
// comment 2015/04/01 OPS1-Sky浜野 [Poseidon][CQ511887] bizhubRA順番待ちでの切断不具合対応.
		if ( PNLE_TABLETRESET_REASON_NONE != pTabletResetControlLib->getReason() ) {
			// 接続状態を変更する.
			if ( message == MS_TabletConnect ) {
				pTabletResetControlLib->setReason_UnlockOnly( PNLE_TABLETRESET_REASON_CONNECT );	// ロック解除：接続.
			} else if ( message == MS_TabletDisconnect ) {
				pTabletResetControlLib->setReason_UnlockOnly( PNLE_TABLETRESET_REASON_DISCONNECT );	// ロック解除：切断..
			}
		}
		return;
	}

// comment 2015/04/01 OPS1-Sky浜野 [Poseidon][CQ511887] bizhubRA順番待ちでの切断不具合対応.
	// アイドル時に通知を受けた場合は、リセット処理中に通知された接続・切断に対してのロック解除をクリアする.
	if ( PNLD_TABLETRESET_IDLE == pTabletResetControlLib->getStatus() ) {
		pTabletResetControlLib->setReason_UnlockOnly( PNLE_TABLETRESET_REASON_NONE );
	}


	// ポップアップアニメーションを強制終了.
	PLayerControl* pOpeningPopup = PNLC_PopupListControl::newInstance()->getNowPopup() ;
	if( NULL != pOpeningPopup ) {
		if( True == pOpeningPopup->isNowPopupAnimation() ) {
			pOpeningPopup->completeGamenAnimationForced();
		}
	}

	// イベント処理.
	if( message == MS_TabletConnect ) {
// Comment 2019/01/09 【HCL_Ranjith】【IT6_EagleH】【RQ-506】 Notify sys:Manager for stop Highlight display.
//comment 2019/05/03 [HCL-Ranjith] [EagleH] [OP_BTS-4579] Highlighting the bizhubRA Display.
		if( LIBG_IsHighlightDisplayVisible()) { 
			LIBG_EnterHighlightDisplayHideSection();
		}
		pTabletResetControlLib->setReason( PNLE_TABLETRESET_REASON_CONNECT );	// 要因：接続.
		// 既にリセット待ちでない場合は、下位レイヤへ通知.
		if( True == pTabletResetControlLib->isWaitFinished( this ) ) {
			PNLC_PopupListControl::newInstance()->procTabletConnect( option, sc_LayerNumber );
		} else {
			procTabletConnect( option );											// タブレット端末接続通知.
		}
	} else if( message == MS_TabletDisconnect ) {
// Comment 2019/01/09 【HCL_Ranjith】【IT6_EagleH】【RQ-506】 Notify sys:Manager for start Highlight display.
//comment 2019/05/03 [HCL-Ranjith] [EagleH] [OP_BTS-4579] Highlighting the bizhubRA Display.
//		LIBG_LeaveHighlightDisplayHideSection();
		LIBG_ResetHighlightDisplayHide();
		pTabletResetControlLib->setReason( PNLE_TABLETRESET_REASON_DISCONNECT );// 要因：切断.
		// 既にリセット待ちでない場合は、下位レイヤへ通知.
		if( True == pTabletResetControlLib->isWaitFinished( this ) ) {
			PNLC_PopupListControl::newInstance()->procTabletDisconnect( option, sc_LayerNumber );
		} else {
			procTabletDisconnect( option );											// タブレット端末切断通知.
		}
	}

	// リセット制御の状態を取得.
	PNLD_TABLETRESET_STATE e_ResetState = pTabletResetControlLib->getStatus();
	if( PNLD_TABLETRESET_WAIT != e_ResetState ) {
		// リセット不要であるかのチェック.
		if( False == pTabletResetControlLib->getNoReset() ) {
			// MS_TabletResetをメッセージ登録する.
			PMGS_PnlMgrParam param;
			memset( &param, 0x00, sizeof( PMGS_PnlMgrParam ) );		// パラメータ初期化.
			param.taskID  = PM_TASKID_PANEL;						// LocalUIタスク.
			param.message = MS_TabletReset;							// リセット.
			param.type = MT_On;										// メッセージタイプ：ON.
			param.option = pTabletResetControlLib->getReason();		// リセット要因.
			pc_PanelMain->DirectKeyFunc( &param );					// メッセージ送信.
		} else {
			// リセットしない場合は、ここでロック解除要求のコマンドを投入する.
// comment 2015/04/01 OPS1-Sky浜野 [Poseidon][CQ511887] bizhubRA順番待ちでの切断不具合対応.
//			APIC_AndroidPanel *pc_AndroidPanel = APIC_AndroidPanel::newInstance(); // 遠隔操作Androidパネル関連 API取得.
//			if ( 0 != pc_AndroidPanel )
//			{
//				if( PNLE_TABLETRESET_REASON_CONNECT == pTabletResetControlLib->getReason() )
//				{
//					// パネルアンロック要求.
//					pc_AndroidPanel->req_PanelTabletLock( APIE_PanelTabletLockRequestKind_UnlockPanel );
//				}
//				else
//				{
//					// 端末アンロック要求.
//					pc_AndroidPanel->req_PanelTabletLock( APIE_PanelTabletLockRequestKind_UnlockTablet );
//				}
			// bizhubRAの終了処理.
			PNLE_TABLETRESET_REASON e_Reason = pTabletResetControlLib->getReason();
			pTabletResetControlLib->TabletEndProcess( e_Reason );
			
			// 処理中の接続/切断通知での終了処理.
			e_Reason = pTabletResetControlLib->getReason_UnlockOnly();
			pTabletResetControlLib->TabletEndProcess( e_Reason );
			
// comment 2015/04/08 OPS1-Sky浜野 [Poseidon][CQ511887] bizhubRA順番待ちでの切断不具合対応[再提起].
//			pTabletResetControlLib->setReason_UnlockOnly( PNLE_TABLETRESET_REASON_NONE );			}
			pTabletResetControlLib->setReason_UnlockOnly( PNLE_TABLETRESET_REASON_NONE );

			// リセット制御を初期化する.
			pTabletResetControlLib->clearWait();
		}
	}
}

/*================================================================================================
//概要    : タブレット接続許可確認通知.
//備考    :.
================================================================================================*/
void PBasicMessageProc::procTabletConnectConfirm( uchar option )
{
	APIC_AndroidPanel* pc_AndroidPanel = APIC_AndroidPanel::newInstance();
// comment 2015/11/12 OPS1-Sky榎本 [MinervaSBK][CQ515617] 無操作中の遠隔操作タブレット接続通知の非表示対応.
	if( NULL == pc_AndroidPanel ) {
		return;
	}
	
// comment 2015/12/01 OPS1-Sky浜野 [MinervaSBK][CQ515617] .
	ushort us_WarningNo = onlyPanel.isAllGamenWarning( onlyPanel.cc_DisplayJob );
	if( True == isFatalExist() || 0 != us_WarningNo ) {	// 警告発生中なら.
		// すでに接続通知画面を表示している場合.
		if( PNL_PATTENTION_TabletConnectConfirm == sp_GamenWarningTable[us_WarningNo].ec_WarningName ) {
			PMessageProcCom::c_TimerTabletConnect.resetTimer(True);		// タイマーリセット.
			pc_AndroidPanel->setConfirmScreenDisplayResult( True );		// 確認画面表示結果：表示.
		} else {
			pc_AndroidPanel->setConnectEnable( False );					// 接続拒否.
			onlyPanel.m_bl_TabletConnectConfirm = False;
		}
		return;
	}
	
// comment 2014/12/22 OPS1-Sky溝部 [ZeusS][CQ507582] タブレット接続許可確認判断における無操作状態判定処理の変更.
//	if( True == PMessageProcCom::c_TimerFaxReceive.isLimit(20) ) {	// 無操作20秒なら（FAX受信無操作タイマーを使用).
//		pc_AndroidPanel->setConnectEnable( True );					// 接続許可.
//		return;
//	}
// comment 2016.05.26 OPS1-Sky西原 [ZeusS_ZX0][CQ530091].
	if( False == PNLC_PopupListControl::newInstance()->isPopupOpen()	// 現在表示中の最前面画面が.
	&&  True  == isNowGamen( &gp_ActionInitialSetup ) ){				// 起動中画面であれば.
		pc_AndroidPanel->setConfirmScreenDisplayResult( False );		// 確認画面表示結果：非表示.
		return;
	} else if ( False == onlyPanel.bl_isEnlargeDisplayMode ) {	// 通常画面時は、20秒.
//	if( False == onlyPanel.bl_isEnlargeDisplayMode ) {	// 通常画面時は、20秒.
		if( True == PMessageProcCom::c_TimerTabletConnect.isLimit(PNLD_NoOperationTimer_NormalDisp) ) {
// comment 2015/11/12 OPS1-Sky榎本 [MinervaSBK][CQ515617] 無操作中の遠隔操作タブレット接続通知の非表示対応.
////		pc_AndroidPanel->setConnectEnable( True );					// 接続許可.
//			onlyPanel.setTabletConnectWithoutConfirm( True );
//			onlyPanel.m_bl_TabletConnectConfirm = True;						// 接続許可確認通知発生.
			pc_AndroidPanel->setConfirmScreenDisplayResult( False );		// 確認画面表示結果：非表示.
			return;
		}
	} else {											// 拡大画面時は、30秒.
		if( True == PMessageProcCom::c_TimerTabletConnect.isLimit(PNLD_NoOperationTimer_LargeDisp) ) {
// comment 2015/11/12 OPS1-Sky榎本 [MinervaSBK][CQ515617] 無操作中の遠隔操作タブレット接続通知の非表示対応.
////		pc_AndroidPanel->setConnectEnable( True );					// 接続許可.
//			onlyPanel.setTabletConnectWithoutConfirm( True );
//			onlyPanel.m_bl_TabletConnectConfirm = True;						// 接続許可確認通知発生.
			pc_AndroidPanel->setConfirmScreenDisplayResult( False );		// 確認画面表示結果：非表示.
			return;
		}
	}

// comment 2015/12/01 OPS1-Sky浜野 [MinervaSBK][CQ515617] .
//	if( True == isFatalExist() || 0 != onlyPanel.isAllGamenWarning( onlyPanel.cc_DisplayJob ) ) {	// 警告発生中なら.
//// comment 2015/11/12 OPS1-Sky榎本 [MinervaSBK][CQ515617] 無操作中の遠隔操作タブレット接続通知の非表示対応.
////		pc_AndroidPanel->setConnectEnable( False );					// 接続拒否.
//		pc_AndroidPanel->setConfirmScreenDisplayResult( False );		// 確認画面表示結果：非表示.
//		return;
//	}
	onlyPanel.m_bl_TabletConnectConfirm = True;						// 接続許可確認通知発生.
// comment 2015/11/12 OPS1-Sky榎本 [MinervaSBK][CQ515617] 無操作中の遠隔操作タブレット接続通知の非表示対応.
//	onlyPanel.setTabletConnectWithoutConfirm( False );
}

// 遷移元画面キャプチャを生成.
void PBasicMessageProc::capturePrevGamen( PBasicMessageProc* pc_PrevGamen )
{
	if( 0 == pc_PrevGamen ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "pc_PrevGamen is NULL!" );
		return;
	}

// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
	updateLcdPane(  pc_PrevGamen );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.

	PBaseWidget* newWidget = dynamic_cast<PBaseWidget*>( m_pc_displayParts );

	if( 0 == newWidget ) {
		// 画面Widgetを生成.
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
// comment 2018/10/12 Sky東島 [1062683] 2ペイン化対応.
//		if ( GFWE_LCD_PANE_1 == getGamenPane( pc_PrevGamen ) ) {
		GFWE_LCD_PANE e_LCDPane = getGamenPane( pc_PrevGamen );
		if( ( GFWE_LCD_PANE_1 == e_LCDPane )
		 || ( GFWE_LCD_PANE_2 == e_LCDPane )
		 || ( GFWE_LCD_PANE_1_Extention == e_LCDPane ) ) {
			// １ペイン,2ペイン,1ペイン(拡大表示)の場合.
			newWidget = new PBaseWidget(
			    m00_076_000_X1024_Y600		// const uchar* puc_Parts
				, 0							// short tx
				, 0							// short gy
				, 132						// uchar uc_PaletteType
				, TYPE_LayoutFree			// TYPE_Layout e_LayoutX = TYPE_LayoutFree
				, False						// Bool centeringY = False
				, 0							// int red = 0
				, 0							// int green = 0
				, 0							// int blue = 0
				, 0							// int alpha = 0
				, GFWE_LCD_Size_10_1inch	// GFWE_LCD_Size e_Pos = GFWE_LCD_Size_9inch
				, GFWE_LCD_Size_10_1inch	// GFWE_LCD_Size e_Size = GFWE_LCD_Size_9inch )
			);

			newWidget->startGUI( 0 );

		// グレーアウト画面Widgetを生成.
			m_pc_grayout = new PGrayoutWidget(
			    m00_076_000_X1024_Y600		// const uchar* puc_Parts
				, 0							// short tx
				, 0							// short gy
				, 132						// uchar uc_PaletteType
				, TYPE_LayoutFree			// TYPE_Layout e_LayoutX = TYPE_LayoutFree
				, False						// Bool centeringY = False 
				, 0							// int red = 0
				, 0							// int green = 0
				, 0							// int blue = 0
				, 128						// int alpha = 128
				, GFWE_LCD_Size_10_1inch	// GFWE_LCD_Size e_Pos = GFWE_LCD_Size_9inch
				, GFWE_LCD_Size_10_1inch	// GFWE_LCD_Size e_Size = GFWE_LCD_Size_9inch
			);
		} else {
			// １ペイン以外の場合.
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
		newWidget = new PBaseWidget(
		    c71_000_000_t1_X800_Y480
		    , 0
		    , 0
		    , 132
		    , TYPE_LayoutFree
		    , False
		);

		newWidget->startGUI( 0 );

		// グレーアウト画面Widgetを生成.
		m_pc_grayout = new PGrayoutWidget(
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/09/10 SCD2 K.Takei 不具合No.1053638_【開発提起】1ペイン/3ペインの切り替え時、3ペインエリアとメインエリアの表示タイミングがずれており、見栄えが悪いので改善する.
			m00_076_000_X1024_Y600		// const uchar* puc_Parts
#else // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.
			c71_000_000_t1_X800_Y480
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.
		    , 0
		    , 0
		    , 132
		    , TYPE_LayoutFree
		    , False
		);
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
		}
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
	}

	// 遷移元画面のキャプチャを生成.
// comment 2018/04/18 SCD2 Kazuo.Takei [Eagle/Sparrow][制御構成変更_No.111 P07_01_共通仕様(第6世代UI画面_左エリア)].
	newWidget->createCaptureImage( static_cast<PBaseWidget*>( pc_PrevGamen->getDisplayParts() ), False, True  );

	// 画面Widgetを保存.
	m_pc_displayParts = newWidget;
}

// ===============================================================================================.
//	概要	: スライドメニュー
//  引数    : オプション.
//	戻り値	: なし.
// ===============================================================================================.
void PBasicMessageProc::procSlideMenu( uchar option )
{
	// スライドメニュー表示処理.
	PNLC_SlideMenuControlLib::newInstance()->open( True );
}
// ===============================================================================================.
//	概要	: スライドメニュー
//  引数    : オプション.
//			   true:ウィジェット(ソフト10キー)の表示.
//	戻り値	: なし.
// ===============================================================================================.
void PBasicMessageProc::procSlideMenu_TenkeyPadShow( uchar option, Bool bl_TenkeyPadShow )
{
	PNLC_SlideMenuControlLib::newInstance()->open( True, bl_TenkeyPadShow );
}

/*================================================================================================
［機能］	状態LEDの処理
================================================================================================*/
void PBasicMessageProc::setupStatusLEDMosel()
{
	// comment 2023/11/30 Sky浦田 [IT6_2.3][OP_BTS-46026].
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	if( NULL == pc_SystemInfo ) {
		return;
	}

	APIC_ActiveJobList* pc_ActiveJobList = APIC_ActiveJobList::newInstance();
//Comment 2017/11/22【HCLT-Sudheer】【IT5 4.2】【RQ686】 API Correction.
//	ss_NumberOfJobList = pc_ActiveJobList->getList( TYPE_ActiveJobListPrint, 1, JBED_JobWorkNum, &c_JobList );
	ss_NumberOfJobList = pc_ActiveJobList->getList( TYPE_ActiveJobListPrint, 1, JBED_JobWorkNum, &c_JobList, PNLC_NetworkLan::getInstance()->getLine() );

	TYPE_PowerSaveStatus	e_PowerSaveStatus = ppc_Power->getPowerSaveStatus();
	// FATAL発生チェック.
	Bool b_isFatal = False;
	if( ( True == onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCall ) )
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PFATAL_ServiceCallSMART ) ) ) {
		b_isFatal = True;
	}
	// ATTETION発生チェック.
	ushort i = 0;
	Bool b_isAttention = False;
	Bool b_isNearLife = False;
	PNLS_IlluminationLEDCtrlFactor sFactor = { False, False };												// comment 2012/09/03 OPS1-Sky西原 [ZeusMLK][機器要望案件 No.2008] お知らせイルミネーション機能対応.
	ushort us_SlidePanelOption = APIC_SystemInfo::newInstance()->getSlidelessPanelOptionInfo();		// comment 2012/09/03 OPS1-Sky西原 [ZeusMLK][機器要望案件 No.2008] お知らせイルミネーション機能対応.

	for( i = 0; i < us_L1MessageWarningTableSize/sizeof( SP_L1MessageWarningTable ); i++ ) {	// ATTETIONテーブル全チェック.
		if( onlyPanel.isWarningOn( comPanelJob, sp_L1MessageWarningTable[i].ec_WarningName ) == True ) {	// ATTETIONが発生している場合.
			Bool b_isNearLifeCheck = False;	// 今チェックしている警告がNearLifeかどうかフラグ // 問題点78909対応 2007/05/31 12仁枝.
			ushort j;
			for( j = 0; j < sizeof( st_NearLifeTable ) / sizeof( st_NearLifeTable[0] ); j++ ) {	// NearLifeテーブル全チェック.
				if( sp_L1MessageWarningTable[i].ec_WarningName == st_NearLifeTable[j].ec_NearLifeName ) {	// ATTENTIONがNearLifeか確認.
					//NearLifeが発生している.
					b_isNearLife = True;
					b_isNearLifeCheck = True;
					break;
				}
			}
			if( b_isNearLifeCheck == False ) {	// 今チェックしている警告がNearLifeでない.
				//ＡＴＴＥＮＴＩＯＮが発生している.
				b_isAttention = True;
			}
			if( b_isNearLife == True && b_isAttention == True ) {
				break;
			}
		}
	}
	// PRT関連異常・IR関連異常発生チェック.
	Bool b_isPRTIRWarning = False;
	if( ( True == onlyPanel.isWarningOn( comPanelJob, PATTENTION_PRTWarning ) )
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PATTENTION_IRWarning ) )
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PATTENTION_GarbageDetection3 ) )		// 原稿読取りガラス清掃警告.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PATTENTION_ReadGuideCleaning ) ) ) {	// 読取りガイド清掃警告.
		b_isPRTIRWarning = True;
	}
	// エンジンが動作不可となるERRORの発生チェック（ようするにプリントエラー停止中のジョブ）.
	Bool b_isStopEngineError = isPrintErrorJobMosel();
//Comment [HCL_Devipriya] [2018/12/29] [IT6_EagleH] 機器構成変更_No.7_FS540 Staple Unit - Warning support.
	if( ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_IULifeC	) )	// ＩＵライフ.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_IULifeM	) )	// ＩＵライフ.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_IULifeY	) )	// ＩＵライフ.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_IULifeK	) )	// ＩＵライフ.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_UsedTonerBottleFull	) )	// 廃トナーフル.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_TransBeltUnitLife	) )	// 転写ベルトユニットライフ.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_FuserUnitLife	) )	// 定着ユニットライフ.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_CTonerEmptyStop	) )	// トナーエンプティストップ.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_MTonerEmptyStop	) )	// トナーエンプティストップ.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_YTonerEmptyStop	) )	// トナーエンプティストップ.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_KTonerEmptyStop	) )	// トナーエンプティストップ.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_IUUnsetC	) )	// ＩＵセット不良・シアン.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_IUUnsetM	) )	// ＩＵセット不良・マゼンタ.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_IUUnsetY	) )	// ＩＵセット不良・イエロー.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_IUUnsetK	) )	// ＩＵセット不良・ブラック.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_IUMissetC	) )	// ＩＵ誤装着・シアン.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_IUMissetM	) )	// ＩＵ誤装着・マゼンタ.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_IUMissetY	) )	// ＩＵ誤装着・イエロー.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_IUMissetK	) )	// ＩＵ誤装着・ブラック.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_UsedTonerBottleUnset ) )	// 廃トナーボトル（廃棄トナーボトル）セット不良.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_PrinterJam	) )	// プリンタジャム検出.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_PunchScrapsFullUser	) )	// パンチくずフル（動作時）(ユーザー交換時) .
//	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_PrinterDoorOpen	) ) ) {	// プリンタセット不良.
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_PrinterDoorOpen) )	// プリンタセット不良.	  
	    || ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_StapleDustBoxFullUser) ) ) {
		// エンジンが動作不可となるERROR（旧ERROR2）が発生した場合は、LED赤（アンバー）点灯させる.
		b_isStopEngineError = True;
	}
	// IRが動作不可となるERRORの発生チェック（ようするに読み取りエラー停止中のジョブ）.
	Bool b_isStopIRError = isScanErrorJobMosel();
	if( True == onlyPanel.isWarningOn( comPanelJob, PERROR_ADFJam	) )	{		// ADFジャム検出.
		b_isStopIRError = True;
	}
	// プリント動作中ジョブのチェック.
	Bool b_isPrintAction = isPrintJobMosel();
	sFactor.bl_PrintAction = b_isPrintAction;		// プリント開始要因セット	comment 2012/09/03 OPS1-Sky西原 [ZeusMLK][機器要望案件 No.2008] お知らせイルミネーション機能対応.
	// 各ＬＥＤの点灯処理.
	THRC_LEDStatus*	ptc_LEDStatus = THRC_LEDStatus::newInstance();
	// 青の点灯処理.
	THRE_LEDStatus LED_Status_Blue	  = THRE_LEDStatus_OFF;		// 青の現在のＬＥＤステータス.
	static THRE_LEDStatus Pre_LED_Status_Blue = ( THRE_LEDStatus )255;	// 青の前回のＬＥＤステータス.	最初のエッヂを取るため無理やり255をセットする.
	if( True == b_isPrintAction ) {	// プリント中.
		LED_Status_Blue = THRE_LEDStatus_BLINK;
//comment 2023/03/02 FPT-HungDM17【IT6_2.3】 【OP_BTS-42240】紙種自動検出OFFで印刷ジョブ実行中、ターミナルにFATALが表示されパネル遷移できなくなる問題対策.
//comment 2023/02/08 FPT-HungDM17【IT6_2.3】 【OP_BTS-36033】FS-539/540装着時の排紙部LED点灯制御変更問題対策.
//if ( TYPE_FinisherTypeFS533 == PnlComGetPrinterSystem()->getFinisherType()
//		 || TYPE_FinisherTypeFS542 == PnlComGetPrinterSystem()->getFinisherType()
//		 || ( ( TYPE_FinisherTypeFS539 == PnlComGetPrinterSystem()->getFinisherType() || TYPE_FinisherTypeFS540 == PnlComGetPrinterSystem()->getFinisherType() )
//			&& TYPE_OutputTraySub3 != comPrintJob->getJobSpec()->pc_Print->c_OutputTray.getOutputTray() ) )
//		{
//			LED_Status_Blue = THRE_LEDStatus_OFF;
//		}
		switch( PnlComGetPrinterSystem()->getFinisherType() ){
			case TYPE_FinisherTypeFS539:
			case TYPE_FinisherTypeFS540:
				if( NULL == comPrintJob || NULL == comPrintJob->getJobSpec() || NULL == comPrintJob->getJobSpec()->pc_Print
					|| TYPE_OutputTraySub3 == comPrintJob->getJobSpec()->pc_Print->c_OutputTray.getOutputTray() ){
					break;
				}
			case TYPE_FinisherTypeFS533:
			case TYPE_FinisherTypeFS542:
				LED_Status_Blue = THRE_LEDStatus_OFF;
				break;
			default: break;
		}
	} else {
//Comment 2018/07/03 【HCLT-Devipriya】 【IT6_Eagle】 【制御構成変更_No.124】 After print completion start Print LED timer and let the LED glow till the time set in Utility.
// Comment 2019/01/29 【HCLT-Ranjith】 【IT6-Eagle_HI】 【zsz build support】.
// #if defined( DEF_EagleCus )
#if defined ( DEF_EagleCus ) || defined ( DEF_EaglePanel_Low ) || defined ( DEF_EaglePanel_Hi )
		// Print completed. Set the LED status to ON.
		if ( THRE_LEDStatus_BLINK == Pre_LED_Status_Blue && PnlComGetPrinterSystem()->isExistFinisher() == False ) {
				LED_Status_Blue = THRE_LEDStatus_ON; 					// Print completed, Change LED status from BLINK->ON. 
		} else if ( THRE_LEDStatus_ON == Pre_LED_Status_Blue ) {
			LED_Status_Blue = THRE_LEDStatus_ON; 					// Maintain LEd status to ON, OFF will be taken care by thread API after timeout.
			if ( True == ptc_LEDStatus->isStatusChange( THRE_LEDID_MOTION, LED_Status_Blue )) {
				LED_Status_Blue = ptc_LEDStatus->getStatus( THRE_LEDID_MOTION );  // If LED status changed by thread, update here.
			}
		}
#else
		LED_Status_Blue = THRE_LEDStatus_OFF;
#endif
	}
#ifdef SIMULATOR
	if( LED_Status_Blue != Pre_LED_Status_Blue ) {	TRACE( "PBasicMessageProc::setupStatusLED() : LED_Status_Blue (%d-->%d) \n", Pre_LED_Status_Blue, LED_Status_Blue );	}
#else
	if( LED_Status_Blue != Pre_LED_Status_Blue ) {	SYSF_logMsg( "PBasicMessageProc::setupStatusLED() : LED_Status_Blue (%d-->%d) \n", Pre_LED_Status_Blue, LED_Status_Blue, 3,4,5,6 );	}
#endif	// #ifdef SIMULATOR.
	// 前回値とステータスが違っていたら、関数をコールする.
	if( LED_Status_Blue != Pre_LED_Status_Blue ) {
		if( 0 == ( us_SlidePanelOption & TYPD_PanelOptionIllumiLED ) ) {						// イルミネーションユニット非装着時のみ.			
//Comment 2018/07/03 【HCLT-Devipriya】 【IT6_Eagle】 【制御構成変更_No.124】 After print completion start Print LED timer and let the LED glow till the time set in Utility.
// Comment 2019/01/29 【HCLT-Ranjith】 【IT6-Eagle_HI】 【zsz build support】.
// #if defined( DEF_EagleCus )
#if defined ( DEF_EagleCus ) || defined ( DEF_EaglePanel_Low ) || defined ( DEF_EaglePanel_Hi )
			if ( LED_Status_Blue == THRE_LEDStatus_ON ) {
				ptc_LEDStatus->statusChangeIF( THRE_LEDID_MOTION, LED_Status_Blue , PnlComGetSystemSetting()->getLEDLightingTimeSetting()*1000);
			} else {
				ptc_LEDStatus->statusChangeIF( THRE_LEDID_MOTION, LED_Status_Blue );
			}
#else 
			ptc_LEDStatus->statusChangeIF( THRE_LEDID_MOTION, LED_Status_Blue );			// 従来のLED制御.
#endif
		}
	}
	Pre_LED_Status_Blue = LED_Status_Blue;	// 現在の値を前回値とする.

	// イルミネーションユニットの点灯制御.
	if( TYPD_PanelOptionIllumiLED == ( us_SlidePanelOption & TYPD_PanelOptionIllumiLED ) ) {		// イルミネーションユニット装着の場合.
		setupIlluminationLED( sFactor );													// イルミネーションユニットのLED制御.
	}

	// アンバー（赤）の点灯処理.
	THRE_LEDStatus LED_Status_Amber		= THRE_LEDStatus_OFF;	// アンバーの現在のＬＥＤステータス.
	static THRE_LEDStatus Pre_LED_Status_Amber	= ( THRE_LEDStatus )255;	// アンバーの前回のＬＥＤステータス.	最初のエッヂを取るため無理やり255をセットする.
	struct ST_StateLEDSettingParameter {
		Bool b_isFatal;				// Fatal発生時.
		Bool b_isStopEngineError;	// プリントエラー発生時.
		Bool b_isStopIRError;		// 読取りエラー停止中.
		Bool b_isAttention;			// ATTENTION発生時.
		Bool b_isNearLife;			// NearLife発生時.
		Bool b_isPRTIRWarning;		// 状態表示LED設定がtype1且つ、IR,PRT異常発生時.
	};
	static ST_StateLEDSettingParameter stateLEDSettingParameterTbl[] = {
		// type1.
		{	True,		// Fatal発生時.
			True,		// プリントエラー発生時.
			True,		// 読取りエラー停止中.
			True,		// ATTENTION発生時.
			True,		// NearLife発生時.
			False,
		},	// IR,PRT異常発生時.
		// type2.
		{	True,		// Fatal発生時.
			True,		// プリントエラー発生時.
			True,		// 読取りエラー停止中.
			True,		// ATTENTION発生時.
			False,	// NearLife発生時.
			False,
		},	// IR,PRT異常発生時.

	};
	APIC_LEDStatusSetting* pc_LEDStatusSetting = APIC_LEDStatusSetting::newInstance(); // インスタンス取得.
	short ss_StateLEDSettingType = -1;	// 状態表示LED設定タイプ.
	if( TYPE_LEDStatus_Type1 == pc_LEDStatusSetting->getWarningStatus() ) {	// 状態表示LED設定タイプ.
		ss_StateLEDSettingType = 0;	// type1.
	} else {	// 状態表示LED設定タイプ.
		ss_StateLEDSettingType = 1;	// type2.
	}
	if( ( TYPE_PowerSaveStatusIdle								== e_PowerSaveStatus )
	    || ( TYPE_PowerSaveStatusPrtEngOnlyLowPowerCancel			== e_PowerSaveStatus )
	    || ( TYPE_PowerSaveStatusPrtEngOnlyLowPowerCancelPanelOn	== e_PowerSaveStatus )
	    || ( TYPE_ProcPowerSaveDuringPrtEngPowerSaveSleepCancel		== e_PowerSaveStatus )
	    || ( TYPE_PowerSaveStatusPrtEngOnlySleepCancelPanelOn		== e_PowerSaveStatus ) ) {
		if( ( True == stateLEDSettingParameterTbl[ss_StateLEDSettingType].b_isFatal           && True == b_isFatal )				// Fatal発生時.
		    || ( True == stateLEDSettingParameterTbl[ss_StateLEDSettingType].b_isStopEngineError && True == b_isStopEngineError )		// プリントエラー発生時.
		    || ( True == stateLEDSettingParameterTbl[ss_StateLEDSettingType].b_isStopIRError     && True == b_isStopIRError ) ) {		// 読取りエラー停止中.
			LED_Status_Amber = THRE_LEDStatus_ON;		// 点灯.
		} else if( ( True == stateLEDSettingParameterTbl[ss_StateLEDSettingType].b_isAttention	&& True == b_isAttention )			// ATTENTION発生時.
		           || ( True == stateLEDSettingParameterTbl[ss_StateLEDSettingType].b_isNearLife		&& True == b_isNearLife )			// NearLife発生時.
		           || ( True == pc_SystemInfo->isCardLineMode()											&& True == onlyPanel.isOilMarkDisp() )	// カードラインモード起動且つ、オイルさしマーク表示.	// comment 2023/11/30 Sky浦田 [IT6_2.3][OP_BTS-46026].
		           || ( True == stateLEDSettingParameterTbl[ss_StateLEDSettingType].b_isPRTIRWarning	&& True == b_isPRTIRWarning ) ) {	// 状態表示LED設定がtype1且つ、IR,PRT異常発生時.
			LED_Status_Amber = THRE_LEDStatus_BLINK;	// 点滅.
		} else {
			LED_Status_Amber = THRE_LEDStatus_OFF;		// 消灯.
		}
	} else {
		LED_Status_Amber = THRE_LEDStatus_OFF;			// 消灯.
	}
#ifdef SIMULATOR
	if( LED_Status_Amber != Pre_LED_Status_Amber ) {	TRACE( "PBasicMessageProc::setupStatusLED() : LED_Status_Amber (%d-->%d) \n", Pre_LED_Status_Amber, LED_Status_Amber );	}
#else
	if( LED_Status_Amber != Pre_LED_Status_Amber ) {	SYSF_logMsg( "PBasicMessageProc::setupStatusLED() : LED_Status_Amber (%d-->%d) \n", Pre_LED_Status_Amber, LED_Status_Amber, 3, 4, 5, 6 );	}
#endif
	// 前回値とステータスが違っていたら、関数をコールする.
	if( LED_Status_Amber != Pre_LED_Status_Amber ) {
		ptc_LEDStatus->statusChangeIF( THRE_LEDID_ATTENTION, LED_Status_Amber );
	}
	Pre_LED_Status_Amber = LED_Status_Amber;	// 現在の値を前回値とする.
}
/*================================================================================================
［機能］	イルミネーションLEDの処理
 [変更履歴]	2012/09/03 OPS1-Sky西原 [ZeusMLK][機器要望案件 No.2008] お知らせイルミネーション機能対応 新規作成.
			2012/09/18 OPS1-Sky西原 [ZeusMLK][機器要望案件 No.2008] DipSW設定 bit1-0に対応.
================================================================================================*/
void PBasicMessageProc::setupIlluminationLED( PNLS_IlluminationLEDCtrlFactor sFactor )
{
	uchar uc_LED_Status_Illumination	  = THRD_IllumiLEDStatus_PRINTEND;		// イルミネーションユニットの現在のＬＥＤステータス.
	static uchar uc_Pre_LED_Status_Illumination = ( unsigned char )THRD_IllumiLEDStatus_PRINTEND;
	static Bool bl_initialized = False;												// True:初期化済み.

	uchar uc_DipSW = ( APIC_SoftDipSw::newInstance()->getSoftDipSwForPanelSpecNo( 188 ) & 0x03 );

	if( False == bl_initialized ) {													// 起動直後に消灯を送らないようにイルミネーション前回値を設定する.
		if( 0x01 == uc_DipSW ) {		// 点滅表示（従来仕様）.
			uc_Pre_LED_Status_Illumination = THRD_IllumiLEDStatus_1stOFF;
		}
		else {
			uc_Pre_LED_Status_Illumination = THRD_IllumiLEDStatus_PRINTEND;
		}
		bl_initialized = True;
	}

	switch( uc_DipSW ) {
		case 0x00:		// イルミネーション表示.
			if( True == sFactor.bl_PrintAction ) {
				uc_LED_Status_Illumination = THRD_IllumiLEDStatus_PRINTSTART;					// プリント開始時.
			}
			else {
				uc_LED_Status_Illumination = THRD_IllumiLEDStatus_PRINTEND;						// プリント終了時.
			}
			break;
		case 0x01:		// 点滅表示（従来仕様）.
			if( True == sFactor.bl_PrintAction ) {
				uc_LED_Status_Illumination = THRD_IllumiLEDStatus_1stBlink;						// 従来仕様(点滅).
			}
			else {
				uc_LED_Status_Illumination = THRD_IllumiLEDStatus_1stOFF;						// 従来仕様(消灯).
			}
			break;
		case 0x02:		// 消灯表示.
			uc_LED_Status_Illumination = THRD_IllumiLEDStatus_PRINTEND;							// 常時消灯.
			break;
		case 0x03:		// プリント動作のみイルミネーション表示.
			if( True == sFactor.bl_PrintAction ) {
				uc_LED_Status_Illumination = THRD_IllumiLEDStatus_PRINTSTART;					// プリント開始時.
			}
			else {
				uc_LED_Status_Illumination = THRD_IllumiLEDStatus_PRINTEND;						// プリント終了時.
			}
			break;
		default:
			break;
	}
#ifdef SIMULATOR
	if( uc_LED_Status_Illumination != uc_Pre_LED_Status_Illumination ) {	TRACE( "PBasicMessageProc::setupIlluminationLED() : uc_LED_Status_Illumination (%d-->%d) \n", uc_Pre_LED_Status_Illumination, uc_LED_Status_Illumination );	}
#else
	if( uc_LED_Status_Illumination != uc_Pre_LED_Status_Illumination ) {	SYSF_logMsg( "PBasicMessageProc::setupIlluminationLED() : uc_LED_Status_Illumination (%d-->%d) \n", uc_Pre_LED_Status_Illumination, uc_LED_Status_Illumination, 3,4,5,6 );	}
#endif	// #ifdef SIMULATOR.
	// 前回値とステータスが違っていたら、関数をコールする.
	if( uc_LED_Status_Illumination != uc_Pre_LED_Status_Illumination ) {
		CNVC_PanelHard::newInstance()->setIllumiLED( uc_LED_Status_Illumination );										// イルミネーションユニットへのLED制御.
	}
	uc_Pre_LED_Status_Illumination = uc_LED_Status_Illumination;	// 現在の値を前回値とする.
}
/*================================================================================================
［機能］	プリント中ジョブの有無.
================================================================================================*/
Bool PBasicMessageProc::isPrintJobMosel()
{
	Bool  b_Find = False;	// プリントジョブ見つけた.
	short i = 0;
	APIC_Job*			pc_SerchJob      = NULL;
	for( i = 0; i < ss_NumberOfJobList; i++ ) {	// 最大マルチジョブ数まで検索
		// 検索ジョブをとり込み
		if( i == 0 ) {
			pc_SerchJob = c_JobList.findFirst();
		} else {
			pc_SerchJob	= c_JobList.findNext();
		}
		if( pc_SerchJob != NULL ) {
			// プリント中の場合は、フラグを立ててループを抜ける.
			if( ( pc_SerchJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusPrintExec ) ) {
				b_Find = True;
				break;
			}
		}
	}
	return b_Find;
}
/*================================================================================================
［機能］	プリントエラー停止中ジョブの有無.
================================================================================================*/
Bool PBasicMessageProc::isPrintErrorJobMosel()
{
	Bool  b_Find = False;	// プリントエラー停止中ジョブ見つけた.
	short i = 0;
	APIC_Job*			pc_SerchJob      = NULL;
	for( i = 0; i < ss_NumberOfJobList; i++ ) {	// 最大マルチジョブ数まで検索
		// 検索ジョブをとり込み
		if( i == 0 ) {
			pc_SerchJob = c_JobList.findFirst();
		} else {
			pc_SerchJob	= c_JobList.findNext();
		}
		if( pc_SerchJob != NULL ) {
			// プリントエラー停止中の場合は、フラグを立ててループを抜ける.
			if( ( pc_SerchJob->getJobStatus()->getOutputStatus() == TYPE_OutputStatusPrintErrorStop ) ) {
				b_Find = True;
				break;
			}
		}
	}
	return b_Find;
}
/*================================================================================================
［機能］	読取りエラー停止中ジョブの有無.
================================================================================================*/
Bool PBasicMessageProc::isScanErrorJobMosel()
{
	Bool  b_Find = False;	// 読取りエラー停止中ジョブ見つけた.
	short i = 0;
	APIC_Job*			pc_SerchJob      = NULL;
	for( i = 0; i < ss_NumberOfJobList; i++ ) {	// 最大マルチジョブ数まで検索
		// 検索ジョブをとり込み
		if( i == 0 ) {
			pc_SerchJob = c_JobList.findFirst();
		} else {
			pc_SerchJob	= c_JobList.findNext();
		}
		if( pc_SerchJob != NULL ) {
			// 読み取り警告停止中の場合は、フラグを立ててループを抜ける.
			if( ( pc_SerchJob->getJobStatus()->getInputStatus() == TYPE_InputStatusScanErrorStop ) ) {
				b_Find = True;
				break;
			}
		}
	}
	return b_Find;
}
// LED制御変更対応.
//================================================================================================
//［機能］	DataLEDの処理.
//================================================================================================
void PBasicMessageProc::setupDataLED()
{
	// DataLEDの点灯処理.
	THRE_LEDStatus e_LEDStatus_Data	  = THRE_LEDStatus_OFF;		// 現在のＬＥＤステータス.
	static THRE_LEDStatus e_PreLEDStatus_Data = ( THRE_LEDStatus )255;	// 前回のＬＥＤステータス.	最初のエッヂを取るため無理やり255をセットする.

	int32_t i32_FaxDocCount= 0;
#ifdef TORNADO
	if( True == APIC_BoxRegistry::GetAccessStatus() ) {	// Doc初期化済みの場合.
		i32_FaxDocCount= APIC_BoxRegistry::getFaxDocumentCnt();
	}
#else	// #ifdef TORNADO
	// SimWinでHDDの初期化シーケンス変更により例外発生するので強制的に0にする.
#endif	// #ifdef TORNADO
	APIC_ActiveJobList*		pc_ActiveJobList = APIC_ActiveJobList::newInstance();
	TYPE_PowerSaveStatus	e_PowerSaveStatus = ppc_Power->getPowerSaveStatus();
	if( ( True == pc_ActiveJobList->isExistPrintRxJob() )
	    || ( True == pc_ActiveJobList->isExistPdlAppInputProcess() )
	    || ( True == cp_Interpreter->isExistInterpreterJob() )				// インタープリター処理中のジョブが存在チェック.
	    || ( True == PnlComGetSystemInfo()->getLedFlashFlag() )				// identify-PrinterコマンドFlash要求有り.
	    || ( PNLE_LogOutputStatus_Output == PLBC_LogOutputStatus::getInstance()->getLogOutputStatus() ) ) {	// 生産ラインにてログ出力中.
		// 受信中ジョブ有り.
		e_LEDStatus_Data = THRE_LEDStatus_BLINK;		// 点滅.
	} else if(	( TYPE_PowerSaveStatusSubPowerOff == e_PowerSaveStatus )				// 副電源OFF.
	            || ( TYPE_PowerSaveStatusSubPowerOffDelay == e_PowerSaveStatus )			// 副電源OFF遅延.
	            || ( TYPE_PowerSaveStatusSubPowerOnProc == e_PowerSaveStatus )				// 副電源ON処理中.
	            || ( TYPE_PowerSaveStatusSubPowerOffDelayIn == e_PowerSaveStatus )			// 副電源OFF遅延移行中.
	            || ( TYPE_PowerSaveStatusSubPowerOffDelayOut == e_PowerSaveStatus ) ) {	// 副電源OFF遅延解除中.

		e_LEDStatus_Data = THRE_LEDStatus_OFF;		// 消灯.
	} else {
		// 副電源ON時.
		if( ( True == pc_ActiveJobList->isExistPrintData() )
		    || ( 0 < i32_FaxDocCount )
		    || ( True == pc_ActiveJobList->isExistFaxData( TYPE_isExistFaxTx ) ) ) {
			e_LEDStatus_Data = THRE_LEDStatus_ON;		// 点灯.
		} else {
			e_LEDStatus_Data = THRE_LEDStatus_OFF;		// 消灯.
		}
	}
	// 前回値とステータスが違っていたら、関数をコールする.
	if( e_LEDStatus_Data != e_PreLEDStatus_Data ) {
#ifdef SIMULATOR
		TRACE( "PBasicMessageProc::setupDataLED() : LED_Status_Data (%d-->%d) \n", e_PreLEDStatus_Data, e_LEDStatus_Data );
#else
		SYSF_logMsg( "PBasicMessageProc::setupDataLED() : LED_Status_Data (%d-->%d) \n", e_PreLEDStatus_Data, e_LEDStatus_Data, 3,4,5,6 );
#endif	// #ifdef SIMULATOR.
		// ＬＥＤの点灯処理.
		THRC_LEDStatus::newInstance()->statusChangeIF( THRE_LEDID_DATA, e_LEDStatus_Data );
	}
	e_PreLEDStatus_Data = e_LEDStatus_Data;				// 現在の値を前回値とする.
}
//================================================================================================
//［機能］	ブザー要求必要性の判定.
//［引数］	無し.
//［戻値］	Bool : 判定結果（True：必要／False：不要）.
//================================================================================================
Bool PBasicMessageProc::isNeedBuzzer()
{
	return ( ( PCGE_BuzzerType_Off == PBasicMessageProc::e_BuzzerType ) ? False : True );
}
//================================================================================================
//［機能］	ブザー要求.
//［引数］	無し.
//［戻値］	無し.
//［ｳｫｰｸｽﾙｰ］未実施.
//================================================================================================
void PBasicMessageProc::sendBuzzerType()
{
	if( PCGE_BuzzerType_Off == PBasicMessageProc::e_BuzzerType ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	reqInformationBuzzer( PBasicMessageProc::e_BuzzerType );
	PBasicMessageProc::e_BuzzerType = PCGE_BuzzerType_Off;
}
//================================================================================================
//［機能］	ブザー要求.
//［引数］	無し.
//［戻値］	無し.
//［ｳｫｰｸｽﾙｰ］未実施.
//================================================================================================
void PBasicMessageProc::sendBuzzerType_PowerSave()
{
	if( PCGE_BuzzerType_Off == PBasicMessageProc::e_BuzzerType ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}

	// 電源キートリガーのブザー音に限り、
	// 電力モード移行中でも音を鳴らすために
	// reqInformationBuzzerではなく、reqBuzzerを直接コールする。

	reqBuzzer( PBasicMessageProc::e_BuzzerType );
	PBasicMessageProc::e_BuzzerType = PCGE_BuzzerType_Off;
}
//================================================================================================
//［機能］	ブザー先行要求.
//［概要］	ブザーが鳴動するまでの処理時間が大きいとき.
//			ユーザーに対して処理が受付られたことを認識させるためにコールする.
//================================================================================================
void PBasicMessageProc::callsendBuzzerType()
{
	if( True == isNeedBuzzer() ) {
		sendBuzzerType();
	}
}

//================================================================================================
//［機能］	ブザータイプの設定.
//［引数］	const PCGE_BuzzerType e_BuzzerType：ブザータイプ.
//			Bool b_Overwrite : True	: 強制的にブザータイプを上書きする
//							   False: 優先順位に従う.(デフォルト).
//================================================================================================
static const struct ST_BuzzerPriority_PCGE {
	PCGE_BuzzerType e_BuzzerType;		// ブザータイプ.
	uchar			priority;			// 優先順位.
} st_BuzzerPriority_PCGE[] = {
	{ PCGE_BuzzerType_Error,			1	},	// 強注意音.
	{ PCGE_BuzzerType_Caution3,			2	},	// 弱注意音③.
	{ PCGE_BuzzerType_AuthNg,			3	},	// 認証失敗.
	{ PCGE_BuzzerType_Caution2,			4	},	// 弱注意音②.
	{ PCGE_BuzzerType_Caution1,			5	},	// 弱注意音①.
	{ PCGE_BuzzerType_InputNg,			6	},	// 入力無効音.
	{ PCGE_BuzzerType_PreparationOk,	7	},	// 準備完了音.
	{ PCGE_BuzzerType_AuthOk,			8	},	// 認証成功.
	{ PCGE_BuzzerType_FaxPrint,			9	},	// FAXプリント完了音.
	{ PCGE_BuzzerType_PcPrint,			10	},	// PCプリント完了音.
	{ PCGE_BuzzerType_CommunicationEnd,	11	},	// 通信正常終了音.
	{ PCGE_BuzzerType_OperationEnd,		12	},	// 操作終了音.
	{ PCGE_BuzzerType_Delete,			13	},	// 削除音.
	{ PCGE_BuzzerType_Base,				14	},	// 基点音.
	{ PCGE_BuzzerType_Motion1,			15	},	// モーション１（ポップアップ）.
	{ PCGE_BuzzerType_Motion2,			16	},	// モーション２（キーボード）.
	{ PCGE_BuzzerType_InputOk,			17	},	// 入力確認音.
	{ PCGE_BuzzerType_StopBuzzer,		18	},	// ブザー停止.
	{ PCGE_BuzzerType_Off,				19	},	// オフ.OFF.
};
void PBasicMessageProc::setBuzzerType( const PCGE_BuzzerType pcge_BuzzerType, Bool b_Overwrite )
{
//comment 2017/10/10 SCD2-Sky肥後 [IT5_4.2] [1030534] タスクコマンド生成対応.
//// comment 2014/09/05 OSW1-Sky前田 UT効率化－受付禁止外キー試験(キー押下相当のコマンドを送信).
//#ifdef DEF_PNL_DIRECT_KEY_DEBUG
	PNLC_SendPanelMainEventLib *pc_instance = PNLC_SendPanelMainEventLib::newInstance();
	if ( NULL != pc_instance ) {
		pc_instance->checkBuzzerType(pcge_BuzzerType);
	}
//#endif	// #ifdef DEF_PNL_DIRECT_KEY_DEBUG.

	// コンポーネント内部からのNG音設定が固定でPMessageProcのsetBuzzerTypeを呼んでしまっており.
	// BrwPnl起動中+コンポーネント使用画面でのNG音処理で受付音(BrwPnl)、NG音(Panel)が両方鳴ってしまうため.
	// BrwPnlが優先タスクの場合はBrwPnlのsetBuzzerTypeを呼ぶ様に修正.
	PM_TaskID	priTask;		// 最優先タスクID.
	ushort		priSetCnt = 0;	// 優先順位登録数.
	priSetCnt = PMGC_PanelManager::getPriority( &priTask );

	// BrwPanelからのアプリ切替依頼の場合、Panelでキーイベントを処理する為、.
	// 音もPanelで鳴動させる(BrwPanelに設定すると、鳴動しない場合がある為).
	if( ( PM_TASKID_BRWPNL == priTask ) && ( False == onlyPanel.getBrwpnlToPnl_AppChangeReq() ) ) {
		BRWC_BasicMessageProc::setBuzzerType( pcge_BuzzerType, b_Overwrite );
		return;
	}

	// 強制的に上書きする.
	if( True == b_Overwrite ) {
		PBasicMessageProc::e_BuzzerType = pcge_BuzzerType;
		return;
	}
	PCGE_BuzzerType	e_NewBuzzer    = pcge_BuzzerType;
	PCGE_BuzzerType	e_PreBuzzer    = PBasicMessageProc::e_BuzzerType;
	PCGE_BuzzerType	e_ResultBuzzer = PCGE_BuzzerType_Off;
// st_BuzzerPriority_PCGEに優先順位を設定されていない引数の場合、
// prePriorityがPCGE_BuzzerType_Offの場合のみ設定を有効にする仕組みの為、10→11に変更.
	uchar			newPriority = 11;
	uchar			prePriority = 11;
	uchar			i;
	uchar			loop = sizeof( st_BuzzerPriority_PCGE )/sizeof( ST_BuzzerPriority_PCGE );
	// 新しく設定するブザー音のプライオリティの取得.
	for( i=0; i<loop; i++ ) {
		if( e_NewBuzzer == st_BuzzerPriority_PCGE[i].e_BuzzerType ) {
			newPriority = st_BuzzerPriority_PCGE[i].priority;
			break;
		}
	}
	// 先に設定されているブザー音のプライオリティの取得.
	for( i=0; i<loop; i++ ) {
		if( e_PreBuzzer == st_BuzzerPriority_PCGE[i].e_BuzzerType ) {
			prePriority = st_BuzzerPriority_PCGE[i].priority;
			break;
		}
	}
	// ブザー音量の取得.
	TYPE_BuzzerVolume e_newVolume = getBuzzerVolume_PCGE( e_NewBuzzer );
	TYPE_BuzzerVolume e_preVolume = getBuzzerVolume_PCGE( e_PreBuzzer );
	// プライオリティの高い、かつ、ブザー音量が設定されているものをセットさせる.
	if( prePriority == newPriority ) {
		// 優先順位が同じなので、Preの値を返す.
		e_ResultBuzzer = e_PreBuzzer;
	} else if( ( prePriority > newPriority ) && ( TYPE_BuzzerVolumeOff != e_newVolume ) ) {
		// Newのほうが優先順位が高い場合は、ブザー音が設定（OFF以外）である場合のみセットする.
		e_ResultBuzzer = e_NewBuzzer;
	} else {
		// Preのほうが優先順位高い場合は、Preが音量セットされているかチェックする.
		if( TYPE_BuzzerVolumeOff != e_preVolume ) {
			// 音量セットされているので、Preをセットする.
			e_ResultBuzzer = e_PreBuzzer;
		} else {
			// 音量セットされていないので、Newをセットする.
			e_ResultBuzzer = e_NewBuzzer;
		}
	}
	PBasicMessageProc::e_BuzzerType = e_ResultBuzzer;
}
//================================================================================================
//［機能］	ブザータイプの取得.
//［引数］	無し.
//［戻値］	const PCGE_BuzzerType e_BuzzerType：ブザータイプ.
//================================================================================================
PCGE_BuzzerType PBasicMessageProc::getBuzzerType()
{
	return PBasicMessageProc::e_BuzzerType;
}
//================================================================================================
//［機能］	全画面警告時のブザーセット.
//［引数］	警告テーブル番号.
//［戻値］	なし.
//================================================================================================
void PBasicMessageProc::setGamenWarningTableBuzzer( short ss_TableNo )
{
	TYPE_WarningName _ec_WarningName = TYPE_WarningName_Non;		// 警告種類.
	PCGE_BuzzerType _e_BuzzerType = PCGE_BuzzerType_Off;			// 警告音種類.
	if( onlyPanel.bl_isEnlargeDisplayMode == False ) {
		// 通常表示.
		_ec_WarningName = sp_GamenWarningTable[ ss_TableNo ].ec_WarningName;
		_e_BuzzerType = sp_GamenWarningTable[ ss_TableNo ].e_BuzzerType;
	} else {
		_ec_WarningName = sp_GamenEnlWarningTable[ ss_TableNo ].ec_WarningName;
		_e_BuzzerType = sp_GamenEnlWarningTable[ ss_TableNo ].e_BuzzerType;
	}
	// TYPE_WarningName_Nonは鳴らさない.
	if( _ec_WarningName != TYPE_WarningName_Non ) {
		if( _ec_WarningName == PERROR_YTonerEmptyStop || _ec_WarningName == PERROR_MTonerEmptyStop
		    || _ec_WarningName == PERROR_CTonerEmptyStop || _ec_WarningName == PERROR_KTonerEmptyStop ) {
			// トナーエンプティ.
			// サービス交換 or ユーザー交換.
			setBuzzerType(  ( PnlComGetPrinterServiceSetting()->getPrinterAdjust( TYPE_PrinterAdjustUnitChangeTonerBtl ) == True) ? PCGE_BuzzerType_Caution3 : PCGE_BuzzerType_Caution2 );
		} else {
			// その他.
			setBuzzerType( _e_BuzzerType );
		}
	}
}
/*================================================================================================
［機能］	Copy基本画面選択
================================================================================================*/
void PBasicMessageProc::nextMainCopyGamen()
{
	if( onlyPanel.bl_isEnlargeDisplayMode == True ) {
		nextGamen = &gp_EnlCopy_Basic;
	} else {
	if( PnlComGetCopyMainDisp() ) {
		nextGamen = &gp_CopyBaseDirect;
	} else {
		nextGamen = &gp_CopyBase;
	}
	}
}
//==================================================================================================
//概要		：procSomethingKeyOn時のMessageを取得する関数.
//==================================================================================================
Message PBasicMessageProc::getSomethingKeyOnMessage()
{
	return m_SomethingKeyOnMessage;
}
//==================================================================================================
//概要		：procSomethingKeyOn時のOptionを取得する関数.
//==================================================================================================
uchar PBasicMessageProc::getSomethingKeyOnOption()
{
	return u_SomethingKeyOnOption;
}

void PBasicMessageProc::callStartGamen()
{
	startGamen();
}

//アニメーション用定周期タイマーから呼び出される
void PBasicMessageProc::animateNowGamen()
{

	for( register int i = 0; i < partsNumber; i++ )
	{
		partsTable[i]->animateParts();	// アニメーションのコマ送り.
	}
	nowGamen->animateGamen();				// 画面のアニメーション実行.
}

#ifdef	DEF_THAMES_ICUSTOM
/**
@name	PBasicMessageProc::procBrowser
@brief	基本メッセージ処理：ブラウザ切替キー処理.
@param	uchar	option(I)	：オプション.
@return	なし.
@note	[レビュ－]	未実施.
@note	[備考]		なし.
@date	2007/3/13	VSN宮部	新規作成.
*/
void PBasicMessageProc::procBrowser( uchar option )
{
	// ブラウザユーザー情報管理機能使用可能の場合.
	if( False == PNLC_BrowUserInfoManager::getInstance()->isBrowUserInfoEnable() ) {
		// ブラウザが未起動状態.
		if( onlyPanel.bl_BrowserStartUpFlag != True )
		{
			// 受付不可.
			setBuzzerType( PCGE_BuzzerType_InputNg );
			return;
		}
	}

// 2019/04/01 Sky-tobita 【OP_BTS-4437】拡大表示の際は拡大警告テーブルを参照する. 
// comment 2021/09/21 Sky中野 [MinervaSSBK][OP_BTS-35459].
//	if( PCGD_ADMINPASS != option && PCGD_USERPASS != option) {
	// 警告でのキー受付チェック.
	if( True == sp_MessageWarningTable[onlyPanel.us_NowMessageWarning].bl_isDisableHardKey[EP_HardKey_Browser] )
	{
		// 受付不可.
		setBuzzerType( PCGE_BuzzerType_InputNg );
// comment 2019/10/07 Chikuan [EagleH][OP_BTS-14390] 問題発生時の切分けLog追加.
		PNL_LogManager::newInstance()->setGeneralPurposeLog("RejectKey(pcgcom)", (ulong)__LINE__ );
		return;
	}
// comment 2021/09/21 Sky中野 [MinervaSSBK][OP_BTS-35459].
//	} else {
//		if ( onlyPanel.bl_isEnlargeDisplayMode == True ) {
//			// 警告でのキー受付チェック.
//			if( True == sp_MessageEnlWarningTable[onlyPanel.us_NowMessageWarning].bl_isDisableHardKey[EP_HardKey_Utility] ) {
//				// 受付不可.
//				setBuzzerType( PCGE_BuzzerType_InputNg );
//				return;
//			}
//		} else {
//			// 警告でのキー受付チェック.
//			if( True == sp_MessageWarningTable[onlyPanel.us_NowMessageWarning].bl_isDisableHardKey[EP_HardKey_Utility] ) {
//				// 受付不可.
//				setBuzzerType( PCGE_BuzzerType_InputNg );
//				return;
//			}
//		}
//	}

	// 割り込み中/拡大モード中は遷移不可.
//comment 2018/08/22 HCL-Raja [Eagle][AR1047173] To launch the admin/user setting webUI irrespective of enlarge mode.
//	if( ( onlyPanel.ec_IntMode == EC_IntMode_On )
//	    || ( onlyPanel.bl_isEnlargeDisplayMode == True ) )
//	{
//		setBuzzerType( PCGE_BuzzerType_InputNg );
//		return ;
//	}
//comment 2018/08/22 HCL-Raja [Eagle][AR1047173] To launch the admin/user setting webUI irrespective of enlarge mode.
// comment 2021/09/21 Sky中野 [MinervaSSBK][OP_BTS-35459].
//	if( PCGD_ADMINPASS != option && PCGD_USERPASS != option) {
	if( ( onlyPanel.ec_IntMode == EC_IntMode_On )
	    || ( onlyPanel.bl_isEnlargeDisplayMode == True ) )
	{
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return ;
	}
//	}

	// 起動確認
	TYPE_KeyMode e_KeyMode = APIC_KeyDevice::getKeyMode();

	if( e_KeyMode == TYPE_KeyModePageAcses )
	{	// 認証装置1装着時は、起動しない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

// Comment 2019/12/31 [IT6_EagleBK] OP_BTS-18202 [HCLT-Santhiya]  Market Place key Display on ClassicStyle Screen. 
	PNLC_ActionDataLib* pc_ActionLib = PNLC_ActionDataLib::newInstance();
	if ( (NULL != pc_ActionLib )
	&& ( False == pc_ActionLib->getMarketPlacelaunch( ) ) ) {
		if( ( False == onlyPanel.bl_FromBrowserLauncher )														// BrowserLauncher起動ではない.
// comment 2021/09/21 Sky中野 [MinervaSSBK][OP_BTS-35459].
//		    &&( PCGD_USERPASS != option ) 																		// ユーティリティからのv1遷移の場合はWebブラウザの使用可否をチェックしない // comment 2018/11/09 Sky東島 [Sparrow][1067019].
			&&( PCGD_CONTENTS != option )																		// コンテンツ表示の場合はWebブラウザの使用可否をチェックしない.	// comment 2022/01/13 Sky中野 [IT6_2.2MR2] 制御No.33_サービスレスメンテナンス対応.
		    &&( ( True == PLBC_FuncVersionCheck::isOK_Type1() )
		        &&( ( False == PnlComGetBrowserSetting()->getEnableBrowser() )										// 管理者でWebBrowserが無効.
		            ||( TYPE_FuncLimit_Disable == PnlComCheckOperationAccountStatus( PNLE_ACCOUNT_STATUS_TYPE_WebBrowser ) ) ) ) ) {	// ユーザーがWebBrowserを許可されていない.
			if( True == PnlComIsNeedUserChangeFunction() ) {	// 認証画面を表示する場合.
				PNLS_AuthenticationLoginReceptKey st_ReceptKey;
				st_ReceptKey.e_OptionType = PNLE_AuthenticationLoginReceptKeyOption_ModeSwitch;	// モード(機能)切替.
				st_ReceptKey.uc_Option = PNLE_AuthenticationReceptKeyType_WebBrowser;	// Webブラウザー機能への切り替え.
				PNLC_AuthenticationManager::newInstance()->authenticate( this , PNLE_LimitationFunctionType_NoCheck, &st_ReceptKey, PNLE_AuthenticatorPopupCloseType_NoClosePopup );	// ログイン画面表示.
			} else {
				setBuzzerType( PCGE_BuzzerType_InputNg );
			}
			return;
		}
	}
		
	if( onlyPanel.ec_IntMode == EC_IntMode_Off
	    && APIC_KeyDevice::getKeyMode() == TYPE_KeyModeAdminMachineKM	// 統合管理装置装着時.
	    && TYPE_UserAuthenticationDevice == ppc_UserAuthentication->getUserAuthentication(PNLC_NetworkLan::getInstance()->getLine()) ) {
		if( PnlComGetSystemSetting()->getPrintAtAdminMachineVendor() == TYPE_AppAtAdminMachineVendorEnableWithoutCardOrCoin
		    || PnlComGetSystemSetting()->getPrintAtAdminMachineVendor() ==  TYPE_AppAtAdminMachineVendorDisable ) {
			// 管理しない.
			if( NULL == PnlComGetUser() ) {
				//認証ユーザはいない.
				TYPE_UserResult e_UserResult = PnlComPublicUserLogin();
				if( TYPE_TrackResultOk != e_UserResult ) {
					setBuzzerType( PCGE_BuzzerType_InputNg );
					return;
				}

			}
		}
		if( PnlComGetSystemSetting()->getPrintAtAdminMachineVendor() == TYPE_AppAtAdminMachineVendorEnableWithCardOrCoin ) {
			// 管理する.
			if( True == PnlComGetUserAccountPublic() ) {
				//Publicユーザで認証している.
				PnlComInitUserAccountInfo( False );		//認証を解除.
			}
		}
	}



	// 最優先アプリの確認.
	PM_TaskID	priTask;		// 最優先タスクID.
	PMGC_PanelManager::getPriority( &priTask );
	PMGS_PnlMgrPriority s_Priority[MAX_PM_TASKID*5];
	memset( &s_Priority, 0x00, sizeof( s_Priority ) );
	// 最優先がキーボードで次優先がブラウザ.
	ushort us_TotalPri = pc_PanelManager->getPriorityTable( s_Priority );
	if( ( 1< us_TotalPri ) && ( us_TotalPri < ( sizeof( s_Priority ) / sizeof( s_Priority[0] ) ) ) ) {
		if( ( s_Priority[us_TotalPri-1].taskID == PM_TASKID_KEYBOARD &&
		      s_Priority[us_TotalPri-2].taskID == PM_TASKID_BROWSER )   ||
		    ( s_Priority[us_TotalPri-1].taskID == PM_TASKID_KEYBOARD &&
		      s_Priority[us_TotalPri-2].taskID == PM_TASKID_BRWPNL ) )
		{
			return;
		}
	}



	// 通知先タスクIDの判定（登録されているタスクであるかの判定）.
	if( pc_PanelManager->isRegistApplication( PM_TASKID_BROWSER ) != True )
	{
		// 登録されていないタスクであった場合.
		return;
	}
	// パネル内部警告の原稿サイズ検出エラー(動作前)解除
	onlyPanel.resetDocSizeDetectError_S();
	if( TYPE_JobTypeCopy == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() ) {
		onlyPanel.bl_ChangeAplBeforeCopy = True;
	}
	if( ( False == onlyPanel.b_nowBoxMode )
	    && ( ( TYPE_JobTypeFaxSendMemory == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() )
	         || ( TYPE_JobTypeFaxSendQuick == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() )
	         || ( TYPE_JobTypeFaxSendTimer == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() )
	         || ( TYPE_JobTypeFaxReceivePolling == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() )
	         || ( TYPE_JobTypeFaxReceiveTimer == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() )
	         || ( TYPE_JobTypeIfaxReceive == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() )
	         || ( TYPE_JobTypeFaxReceive == comPanelJob->getJobSpec()->pc_Common->c_JobType.getJobType() ) ) )
	{
		onlyPanel.bl_ChangeAplBeforeScanFax = True;
	}

	nextGamen = &gp_AppChangeBrowser;

	// サムネイル・プレビュー管理クラス（仕上り）を取得する.
	PCGC_VerifyThumbnailPreviewControl* pc_VerifyControl = PnlComGetVerifyPreviewControl();
	if( NULL == pc_VerifyControl )
	{
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}
	else
	{
		// 仕上りプレビューを終了する.
		pc_VerifyControl->endVerify( False );
	}
}


#endif	// DEF_THAMES_ICUSTOM
// ===============================================================================================.
//	概要	: スライドメニュー
//  引数    : オプション.
//	戻り値	: なし.
//	変更履歴:2015/03/23 OPS1-Sky守川 [Helios][スライドメニュードラッグ対応].
// ===============================================================================================.
void PBasicMessageProc::procSlideMenudisplayKey( uchar option )
{
	// 10キー表示可否.
	Bool bl_Show10Key = IsShowSlide10Key();
	PNLC_SlideMenuControlLib::newInstance()->updateSlideMenu10key( bl_Show10Key );

	if(True == bl_Show10Key){
		// 10キー入力表示.
		sendSlide10KeyData( True );
	}
}

// commnet 2015/03/23 OPS1-Sky守川 [Helios][スライドメニュードラッグ対応].
// ===============================================================================================.
//	概要	: スライドメニュー
//  引数    : なし.
//	戻り値	: テンキー表示可否
//             true：テンキー表示可.
//	変更履歴:2015/03/23 OPS1-Sky守川 [Helios][スライドメニュードラッグ対応].
// ===============================================================================================.
Bool PBasicMessageProc::IsShowSlide10Key()
{
	return False;
}
// ===============================================================================================.
//	概要	: スライドメニュー入力エリアデータ送信.
//  引数    : True：初期化あり、False：初期化なし.
//	戻り値	: なし.
//	変更履歴:2015/04/06 OPS1-Sky西原 [Helios][スライドメニュードラッグ対応].
// ===============================================================================================.
void PBasicMessageProc::sendSlide10KeyData(Bool bl_Flag)
{
}


// ===============================================================================================.
//	概要	: LCD解像度がWVGA以上かチェック.
//  LCD解像度が、WVGA以上ならTrueを返す.
//================================================================================================.
Bool PBasicMessageProc::isResolutionLargerWVGA()
{
	const unsigned short WVGA_WIDTH  = 800;
	const unsigned short WVGA_HEIGHT = 480;

	unsigned short cx = 0;
	unsigned short cy = 0;

	GFWC_System* pc_GfwSystem = GFWC_System::getInstance();
	if( NULL != pc_GfwSystem ){
		pc_GfwSystem->getResolution( cx, cy );
	}

	if((cx > WVGA_WIDTH) || (cy > WVGA_HEIGHT)) {
		return True;
	}
	
	return False;
}

// ===============================================================================================.
//	概要	: キャッシュ画面チェック.
//  画面が、キャッシュ画面対象リストに含まれているかをチェックする.
//  LCD解像度が、WVGA以上のときとWVGA未満のときでキャッシュ画面対象リストは異なる.
//================================================================================================.
Bool PBasicMessageProc::isUseCacheGamen()
{
	const PBasicMessageProc** ppGamenList = NULL;
	int nGamenNum = 0;
	if(isResolutionLargerWVGA() == True) {
		// WVGA以上のとき用のキャッシュ画面リストを選択.
		ppGamenList = GamenCacheListScalable;
		nGamenNum = sizeof(GamenCacheListScalable) / sizeof(PBasicMessageProc*);
	}
	else {
		// WVGA未満のとき用のキャッシュ画面リストを選択.
		ppGamenList = GamenCacheList;
		nGamenNum = sizeof(GamenCacheList) / sizeof(PBasicMessageProc*);
	}
	
	// 画面キャッシュ対象画面かをチェック.
	for(int ii=0; ii<nGamenNum; ii++) {
		if(ppGamenList[ii] == this) {
			return True;
		}
	}
	
	return False;
}

// ===============================================================================================.
//	概要	: パーツのキャッシュをクリア.
//================================================================================================.
void PBasicMessageProc::clearCache()
{
	short partsNumber			= getPartsNumber();
	PDisplayParts** partsTable	= getPartsTable();
	for( int i = 0; i < partsNumber; i++ )
	{
		partsTable[i]->clearCache();
	}
}

// ===============================================================================================.
//	概要	: 認証シーケンス画面設定処理(通常用).
//  引数    : なし.
//	戻り値	: なし.
//	変更履歴:.
// ===============================================================================================.
Bool PBasicMessageProc::changeSeqCustomNextGamen()
{
	Bool                  b_ret               = False;								// 遷移先設定結果.

	// 警告発生している場合は既存画面を表示させる.
	ushort	warnNo	= onlyPanel.isAllGamenWarning( onlyPanel.cc_DisplayJob );
	if ( 0 != warnNo ) {
		return( False );
	}
	
	register short ss_TableNo = 0;														// 0 : モード設定可.
	ss_TableNo = onlyPanel.isDisableModeSet();											// モード設定禁止画面チェック.
	APIC_UserAuthentication* pc_UserAuth = APIC_UserAuthentication::newInstance();		// ユーザ認証クラスポインタ.
	TYPE_UserAuthentication  e_UserAuth = TYPE_UserAuthenticationOff;					// ユーザ認証方式.
	if( NULL == pc_UserAuth ){
		PnlComGetSystemStatus()->setFatal( PNLD_ErrCode_Process_SeqCustom );
//comment 2017/07/05 SCD2-Sky肥後 [1026857] 新詳細コード対応.
		LAPC_BFW_System *pc_BFW_System = LAPC_BFW_System::newInstance();
		if (NULL != pc_BFW_System){
			pc_BFW_System ->registDetailCode( PNLD_DetailCode_FileIdentification_0x1900740020000000 | ( __LINE__ << 4 ) );
		}
		return b_ret;
	}
	e_UserAuth = pc_UserAuth->getDefaultUserAuthentication(PNLC_NetworkLan::getInstance()->getLine());

	if( ( sp_GamenDisableModeSetTable[ ss_TableNo ].ec_WarningName == PNL_OTHER_UserIDNumberInput  ) ||		// ユーザ認証入力の場合.
		( ( sp_GamenDisableModeSetTable[ ss_TableNo ].ec_WarningName == PNL_OTHER_UserIDNumberInput2 ) && 
		  ( e_UserAuth == TYPE_UserAuthenticationDevice )                                          ) ||		// ユーザ認証入力（中間認証サーバー用）の場合 かつ デフォルト認証方式が本体認証の場合.
		( sp_GamenDisableModeSetTable[ ss_TableNo ].ec_WarningName == PNL_OTHER_UserIDNumberInput3 ) ||		// ユーザ認証入力（ブラウザ中間認証用）の場合.
		( sp_GamenDisableModeSetTable[ ss_TableNo ].ec_WarningName == PNL_OTHER_TrackIDNumberInput )		// 部門管理入力の場合.
	){
		if( PNLE_SeqCustom_Status_Top == PnlComSeqGetStatus() ){
			PnlComSeqTransTopGamen();
			// 先頭画面を取得したため認証シーケンス管理の状態を動作中とする.
			PnlComSeqSetStatus( PNLE_SeqCustom_Status_Act );
		} else {
			ushort	tblNo;
			ushort	tblSize	= static_cast<ushort>( us_MessageWarningTableSize / sizeof( SP_MessageWarningTable ) );
			PNLC_PopupListControl*	pc_PopupList	= PNLC_PopupListControl::newInstance();
			if ( 0 != pc_PopupList ) {
				for ( tblNo = 0; tblNo < tblSize; tblNo++ ) {
					if ( 0 != sp_MessageWarningTable[tblNo].p_PopupWarnMsgWindow ) {
						if ( True == pc_PopupList->isPopupLayer( sp_MessageWarningTable[tblNo].p_PopupWarnMsgWindow ) ) {
							pc_PopupList->closeSpecifiedPopup( sp_MessageWarningTable[tblNo].p_PopupWarnMsgWindow );
						}
					}
				}
			}
		}
		return( True );
	}
	return b_ret;
}

// ===============================================================================================.
//	概要	: 認証シーケンス画面設定処理(拡大用).
//  引数    : なし.
//	戻り値	: なし.
//	変更履歴:.
// ===============================================================================================.
Bool PBasicMessageProc::changeSeqCustomNextEnlGamen()
{
	Bool                  b_ret               = False;								// 遷移先設定結果.

	// 警告発生している場合は既存画面を表示させる.
	ushort	warnNo	= onlyPanel.isAllGamenWarning( onlyPanel.cc_DisplayJob );
	if ( 0 != warnNo ) {
		return( False );
	}

	register short ss_DisableModeSetNo = 0;												// 0 : モード設定可.
	ss_DisableModeSetNo = onlyPanel.isDisableModeSet();									// モード設定禁止画面チェック.
	APIC_UserAuthentication* pc_UserAuth = APIC_UserAuthentication::newInstance();		// ユーザ認証クラスポインタ.
	TYPE_UserAuthentication  e_UserAuth = TYPE_UserAuthenticationOff;					// ユーザ認証方式.
	if( NULL == pc_UserAuth ){
		PnlComGetSystemStatus()->setFatal( PNLD_ErrCode_Process_SeqCustom );
//comment 2017/07/05 SCD2-Sky肥後 [1026857] 新詳細コード対応.
		LAPC_BFW_System *pc_BFW_System = LAPC_BFW_System::newInstance();
		if (NULL != pc_BFW_System){
			pc_BFW_System ->registDetailCode( PNLD_DetailCode_FileIdentification_0x1900740020000000 | ( __LINE__ << 4 ) );
		}
		return b_ret;
	}
	e_UserAuth = pc_UserAuth->getDefaultUserAuthentication(PNLC_NetworkLan::getInstance()->getLine());

	if( ( sp_GamenEnlDisableModeSetTable[ss_DisableModeSetNo].ec_WarningName == PNL_OTHER_UserIDNumberInput  ) ||	// ユーザ認証入力の場合.
		( ( sp_GamenEnlDisableModeSetTable[ss_DisableModeSetNo].ec_WarningName == PNL_OTHER_UserIDNumberInput2 ) &&
		  ( e_UserAuth == TYPE_UserAuthenticationDevice )                                                    ) ||	// ユーザ認証入力（中間認証サーバー用）の場合 かつ デフォルト認証方式が本体認証の場合.
		( sp_GamenEnlDisableModeSetTable[ss_DisableModeSetNo].ec_WarningName == PNL_OTHER_UserIDNumberInput3 ) ||	// ユーザ認証入力（ブラウザ中間認証用）の場合.
		( sp_GamenEnlDisableModeSetTable[ss_DisableModeSetNo].ec_WarningName == PNL_OTHER_TrackIDNumberInput ) ||	// 部門管理入力の場合.
		( (sp_GamenEnlDisableModeSetTable[ss_DisableModeSetNo].ec_WarningName == PNL_OTHER_UserIDAndPassWordInput ) &&	// ユーザー名、パスワード入力画面かつ DipSW 53 bit2 特ROM対応 DA-197.
		  ( True == PnlComisEnableUserAccountFull()	) )
	){
		if( PNLE_SeqCustom_Status_Top == PnlComSeqGetStatus() ){
			// 認証エラー発生中判定(既存の画面遷移と同等にするため).
			if( ( True == PCPC_EnlWarn_UserCertificate_Custom_SeqCustom::isAuthError() )
			||  ( True == PCPC_EnlWarn_UserCertificate_SeqCustom::isAuthError() ) ) {
				PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();
				if( 0 != pc_PopupListControl ) {
					pc_PopupListControl->removeAllPopup_ToPrintSetupChangeMenu();
					// 認証エラー発生中の場合の判定を認証エラー画面クラスのボタン押下判定で行う.
					// 認証エラーが発生していた場合は再度認証エラー画面に遷移する.
					if( True == PCPC_EnlWarn_UserCertificate_Custom_SeqCustom::isAuthError() ) {
						PNLC_PopupListControl::newInstance()->addPopup( &gp_EnlWarn_UserCertificate_Custom_SeqCustom );
					} else {
						PNLC_PopupListControl::newInstance()->addPopup( &gp_EnlWarn_UserCertificate_SeqCustom );
					}
				} else {
					PnlComSeqTransTopGamen();
					// 先頭画面を取得したため認証シーケンス管理の状態を動作中とする.
					PnlComSeqSetStatus( PNLE_SeqCustom_Status_Act );
				}
			} else {
				PnlComSeqTransTopGamen();
				// 先頭画面を取得したため認証シーケンス管理の状態を動作中とする.
				PnlComSeqSetStatus( PNLE_SeqCustom_Status_Act );
			}
		} else {
			ushort	tblNo;
			ushort	tblSize	= static_cast<ushort>( us_GamenEnlWarningTableSize / sizeof( SP_GamenEnlWarningTable ) );
			PNLC_PopupListControl*	pc_PopupList	= PNLC_PopupListControl::newInstance();
			if ( 0 != pc_PopupList ) {
				for ( tblNo = 0; tblNo < tblSize; tblNo++ ) {
					PLayerControl*	pop	= dynamic_cast<PLayerControl*>( sp_GamenEnlWarningTable[tblNo].WarningGamenEnl );
					if ( 0 != pop ) {
						if ( True == pc_PopupList->isPopupLayer( pop ) ) {
							pc_PopupList->closeSpecifiedPopup( pop );
						}
					}
				}
			}
			if( ( True == PCPC_EnlWarn_UserCertificate_Custom_SeqCustom::isAuthError() )
			||  ( True == PCPC_EnlWarn_UserCertificate_SeqCustom::isAuthError() ) ) {
				// 認証エラー発生中の場合の判定を認証エラー画面クラスのボタン押下判定で行う.
				// 認証エラーが発生していた場合は再度認証エラー画面に遷移する.
				if( True == PCPC_EnlWarn_UserCertificate_Custom_SeqCustom::isAuthError() ) {
					PNLC_PopupListControl::newInstance()->addPopup( &gp_EnlWarn_UserCertificate_Custom_SeqCustom );
				} else {
					PNLC_PopupListControl::newInstance()->addPopup( &gp_EnlWarn_UserCertificate_SeqCustom );
				}
			} else {
				if( NULL != turnGamen ) {
					nextGamen = turnGamen;
				}
			}
		}
		return( True );
	}
	return b_ret;
}

// ===============================================================================================.
//	概要	: 状態に応じた画面遷移処理(通常用/拡大用).
//  引数    : なし.
//	戻り値	: なし.
//	変更履歴:.
// ===============================================================================================.
void PBasicMessageProc::rebuildGamenSeqCustom()
{
	PnlComSeqTransTopGamen();
}

// ===============================================================================================.
//	概要	: 認証カスタマイズ機能有効/無効値を参照し遷移先画面を設定.
//  引数    : なし.
//	戻り値	: Bool (True:認証カスタマイズ機能で設定 / False:既存動作で設定).
//	変更履歴:.
// ===============================================================================================.
Bool PBasicMessageProc::isSeqCustomNextGamen()
{
	Bool  b_ret                 = False;							// 処理結果.
	uchar uc_AuthSeqCustomValid = TYPD_SeqCustom_Function_INVALID;	// 認証カスタマイズ機能有効/無効値.

	PnlComGetSeqCustomValid( &uc_AuthSeqCustomValid );		// 認証カスタマイズ機能有効/無効設定取得.

	switch( uc_AuthSeqCustomValid ){
		case TYPD_SeqCustom_Function_VALID:		// 認証カスタマイズ機能が有効の場合.
			b_ret = PBasicMessageProc::changeSeqCustomNextEnlGamen();
			break;
		case TYPD_SeqCustom_Function_UNSET:		// 認証カスタマイズ機能が初期状態の場合.
		case TYPD_SeqCustom_Function_INVALID:	// 認証カスタマイズ機能が無効の場合.
		default:
			break;
	}
	return b_ret;
}

// ===============================================================================================.
//	概要	: 認証カスタマイズ機能有効/無効値を参照し遷移先画面を設定.
//  引数    : なし.
//	戻り値	: Bool (True:認証カスタマイズ機能で設定 / False:既存動作で設定).
//	変更履歴:.
// ===============================================================================================.
void PBasicMessageProc::setSeqCustomGamenIns( PBasicMessageProc* pc_SeqGamen )
{
	m_pc_SeqCusGamen = pc_SeqGamen;
}

// commen 2016/06/17 SCD2-Sky浜野 [ZeusSBK][CQ530484].
/*================================================================================================
［機能］	原稿サイズ変化、更新処理.
================================================================================================*/
Bool PBasicMessageProc::updateDocSizeEffective()
{
#ifndef DEF_A4_Cus		// A4機は原稿自動検出無しの為、原稿サイズ確定のチェックは不要.
	// 原稿サイズ変化の検知が定期起床となり.
	// ジョブ実行時に原稿サイズ未確定でADFからのスキャンを実施するため.
	// ジョブ実行時に原稿サイズに変化がある場合は、原稿サイズ確定処理を実施する.
	
	// 原稿サイズの変化エッジ検出.
	if( True == isDocSizeEffective() ) {
		// 原稿サイズ更新処理.
		procDocSizeEffective( b_targetJobChange == True ? 1 : 0 );
	} else {
		// 原稿サイズが検出できている場合.
		if( ( True == comPanelJob->isADFOriginalSizeFixed() ) 
		&&	( TYPE_SizeCodeUnknown != ppc_Original->getAdfOriginalSize().e_SizeCode ) ) {
			// NOP.
		} else {
			// 原稿未確定のため、ジョブ実行しない.
			return False;
		}
	}
#endif //DEF_A4_Cus.	// A4機は原稿自動検出無しの為、原稿サイズ確定のチェックは不要.
	
	return True;
}

// comment 2017/03/10 SCD2-Sky浜野 [IT5_4.1][RQ-643][1012954].
// SuperWarp実施可否更新.
void PBasicMessageProc::updateSuperWarpDisable()
{
	APIC_SystemStatus* pc_SystemStatus = APIC_SystemStatus::newInstance();
	if( NULL == pc_SystemStatus ){	
		return;
	}

	// LocalUIのモジュール番号(0x19)のSuperWarp実施不可要因を取得.
	uint32_t ul_CheckCause = pc_SystemStatus->getSuperWarpCause( 0x19 );
	static uint32_t ul_PreSuperWarpCause = ul_CheckCause;
	// SuperWarp停止要因が『なし』または、『0x19000008～0x190003EB』（チェック対象）の場合.
	if( ( 0 == ul_CheckCause )
	||	( PNLD_SuperWarpCause_0x19000000 == ul_CheckCause )
	||	( ( PNLD_SuperWarpCause_0x19000008 <= ul_CheckCause ) && ( PNLD_SuperWarpCause_0x190003EB > ul_CheckCause ) ) ) {
		uint32_t ul_SuperWarpCause = PNLD_SuperWarpCause_0x19000000;
		// 警告IDによるSuperWarp実行不可警告発生チェック.
		for( ushort us_i = 0; us_i < us_SuperWarpDisableWarningSize; us_i++ ) {
			if( True == onlyPanel.isWarningOn( comPanelJob, tbl_SuperWarpDisableWarning[us_i] ) ) {
				ul_SuperWarpCause = PNLD_SuperWarpCause_0x19000008 + us_i;
				break;
			}
		}
		// 警告画面によるSuperWarp実行不可警告発生チェック.
		for( ushort us_j = 0; us_j < us_SuperWarpDisableGamenSize; us_j++ ) {
// comment 2017/04/26【HCLT-vijay】[IT5_4.1][AR1019954][ID:6732] dynamic casting base to derived.
			PLayerControl* p_GamenInstance = dynamic_cast< PLayerControl* >( ( tbl_SuperWarpDisableGamen[us_j] ) );
			if ( NULL != p_GamenInstance ) {
//				if( True == PNLC_PopupListControl::newInstance()->isPopupLayer( tbl_SuperWarpDisableGamen[us_j] ) ) {
				if( True == PNLC_PopupListControl::newInstance()->isPopupLayer( p_GamenInstance ) ) {
					ul_SuperWarpCause = PNLD_SuperWarpCause_0x190001F5 + us_j;
					break;
				}
            }
	    }

// comment 2017/03/10 SCD2-Sky浜野 [IT5_4.1][RQ-643][1013151].
		// FTP、WebDAV接続確認中.
		if( ( True == c_SystemControl.nowFTPConnection() )
		||	( True == c_SystemControl.nowWebDAVConnection() ) ) {
			ul_SuperWarpCause = PNLD_SuperWarpCause_0x190003EA;
		}
		
		// ユーザー認証中（部門＋ユーザー認証も含む）かつ、Publicユーザーログイン中でない場合.
		// または、部門管理のみかつ、部門ログイン中（部門のみでPublicはありえない）.
		if( ( ( True == PnlComGetUserAccountOn() ) && ( False == PnlComGetUserAccountPublic() ) )
		||	( ( True == PnlComGetTrackOn() ) && ( True == PnlComGetTrackOnlyOn() ) ) ) {
			ul_SuperWarpCause = PNLD_SuperWarpCause_0x190003E9;
		}
		
		// SuperWarp実施不可有無.
		if( ul_SuperWarpCause != ul_PreSuperWarpCause ) {
			pc_SystemStatus->setSuperWarpCause( ul_SuperWarpCause );
			ul_PreSuperWarpCause = ul_SuperWarpCause;
		}
	}
}
// Comment 2016/12/20 第2システム制御開発 Sky上野 [ServiceHub] SHRQ-002 SH上でのエラー警告対応.
/*================================================================================================
［機能］	 SH_テーブル初期化関数.
================================================================================================*/
void PBasicMessageProc::startSHWarning()
{
	if( False == bl_SHWarningCheckTableInitialization ){
		ushort us_TableAllCnt = 0;	// 全警告発生状況確認テーブルカウント用.
		ushort us_TableCnt = 0;		// 既存の全画面/2行/下1行テーブルカウント用.
// Comment 2017/03/27 第2システム制御開発 Sky上野 [ServiceHub][SHRQ-002]追加対応.
		ushort us_EnlTableAllCnt = 0;	// 拡大画面全警告発生状況確認テーブルカウント用.
		
		// 全画面警告情報を全警告発生状況確認テーブルと発生警告詳細通知テーブルへ格納.
		for( us_TableCnt = 0; us_TableCnt < us_GamenWarningTableMax; us_TableCnt++){		
			if( PNLD_SHNowWarningAllTblMax <= us_TableAllCnt ){
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Counter Overe Max!" );
				break;
			}			
			// CommonAPIに定義されいる警告、且つ警告テーブルが空でないもののみを格納する.
			if( ( PNLD_SHWarningNon != sp_GamenWarningTable[ us_TableCnt ].us_SHWarningID ) && ( NULL != sp_GamenWarningTable[ us_TableCnt ].p_WarningName ) ){
				stNowWarningAllTbl[ us_TableAllCnt ].uc_WarningType = TYPE_WarningType_FullScreen;							// 全警告発生状況確認テーブルへ画面種別を格納.
				stNowWarningAllTbl[ us_TableAllCnt ].bl_WarningOn = False;													// 警告発生状況をFalseで初期化.
				stNowWarningAllTbl[ us_TableAllCnt ].us_TableNo = us_TableCnt;												// 発生警告詳細通知テーブルへ既存全画面テーブルの配列番号を格納.
				stNowWarningAllTbl[ us_TableAllCnt ].ul_JobID = PNLD_SHWarningJobIDNon;										// ジョブIDを0で初期化.
				stNowWarningAllTbl[ us_TableAllCnt ].us_SHWarningID = sp_GamenWarningTable[ us_TableCnt ].us_SHWarningID;		// CommonAPIに定義されいる警告IDを格納.
				us_TableAllCnt++;
			}
		}
		// 2行メッセージ警告情報を全警告発生状況確認テーブルへ格納.
		for( us_TableCnt = 0; us_TableCnt < us_MessageWarningTableMax; us_TableCnt++){			
			if( PNLD_SHNowWarningAllTblMax <= us_TableAllCnt ){
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Counter Overe Max!" );
				break;
			}
			// CommonAPIに定義されいる警告、且つ警告テーブルが空でないもののみを格納する.
			if( ( PNLD_SHWarningNon != sp_MessageWarningTable[ us_TableCnt ].us_SHWarningID ) && ( NULL != sp_MessageWarningTable[ us_TableCnt ].p_WarningName ) ) {
				stNowWarningAllTbl[ us_TableAllCnt ].uc_WarningType = TYPE_WarningType_Message;								// 全警告発生状況確認テーブルへ画面種別を格納.
				stNowWarningAllTbl[ us_TableAllCnt ].bl_WarningOn = False;													// 警告発生状況をFalseで初期化.
				stNowWarningAllTbl[ us_TableAllCnt ].us_TableNo = us_TableCnt;												// 発生警告詳細通知テーブルへ既存全画面テーブルの配列番号を格納.
				stNowWarningAllTbl[ us_TableAllCnt ].ul_JobID = PNLD_SHWarningJobIDNon;										// ジョブIDを0で初期化.
				stNowWarningAllTbl[ us_TableAllCnt ].us_SHWarningID = sp_MessageWarningTable[ us_TableCnt ].us_SHWarningID;	// CommonAPIに定義されいる警告IDを格納.
				us_TableAllCnt++;
			}
		}
		// 下1行メッセージ警告情報を全警告発生状況確認テーブルへ格納.
		for( us_TableCnt = 0; us_TableCnt < us_L1MessageWarningTableMax; us_TableCnt++){			
			if( PNLD_SHNowWarningAllTblMax <= us_TableAllCnt ){
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Counter Overe Max!" );
				break;
			}
			// CommonAPIに定義されいる警告、且つ警告テーブルが空でないもののみを格納する.
			if( ( PNLD_SHWarningNon != sp_L1MessageWarningTable[ us_TableCnt ].us_SHWarningID ) && ( NULL != sp_L1MessageWarningTable[ us_TableCnt ].p_WarningName ) ) {
				stNowWarningAllTbl[ us_TableAllCnt ].uc_WarningType = TYPE_WarningType_Line;									// 全警告発生状況確認テーブルへ画面種別を格納.
				stNowWarningAllTbl[ us_TableAllCnt ].bl_WarningOn = False;														// 警告発生状況をFalseで初期化.
				stNowWarningAllTbl[ us_TableAllCnt ].us_TableNo = us_TableCnt;													// 発生警告詳細通知テーブルへ既存全画面テーブルの配列番号を格納.
				stNowWarningAllTbl[ us_TableAllCnt ].ul_JobID = PNLD_SHWarningJobIDNon;											// ジョブIDを0で初期化.
				stNowWarningAllTbl[ us_TableAllCnt ].us_SHWarningID = sp_L1MessageWarningTable[ us_TableCnt ].us_SHWarningID;		// CommonAPIに定義されいる警告IDを格納.
				us_TableAllCnt++;
			}
		}
		
		// ATTENTION2警告情報を全警告発生状況確認テーブルへ格納.
		for( us_TableCnt = 0; us_TableCnt < us_sp_Attention2WarningTableMax; us_TableCnt++){
			if( PNLD_SHNowWarningAllTblMax <= us_TableAllCnt ){
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Counter Overe Max!" );
				break;
			}
			// CommonAPIに定義されいる警告が空でないもののみを格納する.
			if( PNLD_SHWarningNon != sp_Attention2WarningTable[ us_TableCnt ].us_SHWarningID ){
				stNowWarningAllTbl[ us_TableAllCnt ].uc_WarningType = PNLD_WarningType_Attention2;								// 全警告発生状況確認テーブルへ画面種別を格納.
				stNowWarningAllTbl[ us_TableAllCnt ].bl_WarningOn = False;														// 警告発生状況をFalseで初期化.
				stNowWarningAllTbl[ us_TableAllCnt ].us_TableNo = us_TableCnt;													// 発生警告詳細通知テーブルへ既存全画面テーブルの配列番号を格納.
				stNowWarningAllTbl[ us_TableAllCnt ].ul_JobID = PNLD_SHWarningJobIDNon;											// ジョブIDを0で初期化.
				stNowWarningAllTbl[ us_TableAllCnt ].us_SHWarningID = sp_Attention2WarningTable[ us_TableCnt ].us_SHWarningID;	// CommonAPIに定義されいる警告IDを格納.				
				us_TableAllCnt++;
			}
		}
		
		// Other警告情報を全警告発生状況確認テーブルへ格納.
		for( us_TableCnt = 0; us_TableCnt < us_sp_OtherWarningTableMax; us_TableCnt++ ){
			if( PNLD_SHNowWarningAllTblMax <= us_TableAllCnt ){
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Counter Overe Max!" );
				break;
			}
			// CommonAPIに定義されいる警告が空でないもののみを格納する.
			if( PNLD_SHWarningNon != sp_OtherWarningTable[ us_TableCnt ].us_SHWarningID ){
				stNowWarningAllTbl[ us_TableAllCnt ].uc_WarningType = PNLD_WarningType_Other;								// 全警告発生状況確認テーブルへテーブル種別を格納.
				stNowWarningAllTbl[ us_TableAllCnt ].bl_WarningOn = False;													// 警告発生状況をFalseで初期化.
				stNowWarningAllTbl[ us_TableAllCnt ].us_TableNo = us_TableCnt;												// 発生警告詳細通知テーブルへ既存全画面テーブルの配列番号を格納.
				stNowWarningAllTbl[ us_TableAllCnt ].ul_JobID = PNLD_SHWarningJobIDNon;										// ジョブIDを0で初期化.
				stNowWarningAllTbl[ us_TableAllCnt ].us_SHWarningID = sp_OtherWarningTable[ us_TableCnt ].us_SHWarningID;		// CommonAPIに定義されいる警告IDを格納.
				us_TableAllCnt++;
			}
		}

//	Comment 2017/02/21 第2システム制御開発 Sky上野 [ServiceHub][SHRQ-002] 課題番号1013443対応.
		// PRT／IR関連異常警告情報を全警告発生状況確認テーブルへ格納.
		for( us_TableCnt = 0; us_TableCnt < WARNING_LIST_NUM; us_TableCnt++ ){
			if( PNLD_SHNowWarningAllTblMax <= us_TableAllCnt ){
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Counter Overe Max!" );
				break;
			}
//Comment [2019/03/08] [HCL_Devipriya] [EagleH] [機器構成変更No.29]  use pointer instead of table.			
			// CommonAPIに定義されいる警告が空でないもののみを格納する.
//			if( PNLD_SHWarningNon != PrtIrWarningList[ PNLD_SHPrtIrWarningPage ][ us_TableCnt ].us_SHWarningID ){ 
//				stNowWarningAllTbl[ us_TableAllCnt ].uc_WarningType = TYPE_WarningType_PRT_IR;														// 全警告発生状況確認テーブルへテーブル種別を格納.
//				stNowWarningAllTbl[ us_TableAllCnt ].bl_WarningOn = False;																			// 警告発生状況をFalseで初期化.
//				stNowWarningAllTbl[ us_TableAllCnt ].us_TableNo = us_TableCnt;																		// 発生警告詳細通知テーブルへ既存全画面テーブルの配列番号を格納.
//				stNowWarningAllTbl[ us_TableAllCnt ].ul_JobID = PNLD_SHWarningJobIDNon;																// ジョブIDを0で初期化.
//				stNowWarningAllTbl[ us_TableAllCnt ].us_SHWarningID = PrtIrWarningList[ PNLD_SHPrtIrWarningPage ][ us_TableCnt ].us_SHWarningID;		// CommonAPIに定義されいる警告IDを格納.
//				us_TableAllCnt++;
//			}
			if( PNLD_SHWarningNon != mp_PrtIrWarningList[ PNLD_SHPrtIrWarningPage ][ us_TableCnt ].us_SHWarningID ){ 
				stNowWarningAllTbl[ us_TableAllCnt ].uc_WarningType = TYPE_WarningType_PRT_IR;														// 全警告発生状況確認テーブルへテーブル種別を格納.
				stNowWarningAllTbl[ us_TableAllCnt ].bl_WarningOn = False;																			// 警告発生状況をFalseで初期化.
				stNowWarningAllTbl[ us_TableAllCnt ].us_TableNo = us_TableCnt;																		// 発生警告詳細通知テーブルへ既存全画面テーブルの配列番号を格納.
				stNowWarningAllTbl[ us_TableAllCnt ].ul_JobID = PNLD_SHWarningJobIDNon;																// ジョブIDを0で初期化.
				stNowWarningAllTbl[ us_TableAllCnt ].us_SHWarningID = mp_PrtIrWarningList[ PNLD_SHPrtIrWarningPage ][ us_TableCnt ].us_SHWarningID;		// CommonAPIに定義されいる警告IDを格納.
				us_TableAllCnt++;
			}
		}

		if( PNLD_SHNowWarningAllTblMax > us_TableAllCnt ){													// 全警告発生状況確認テーブルに空きがある場合.
			for( us_TableCnt = us_TableAllCnt; us_TableCnt < PNLD_SHNowWarningAllTblMax; us_TableCnt++){
				stNowWarningAllTbl[ us_TableCnt ].us_SHWarningID = PNLD_SHWarningNon;							// 空きテーブルの警告IDに255を格納する.
				stNowWarningAllTbl[ us_TableCnt ].us_SHWarningID = PNLD_WarningTypeNon;						// 空きテーブルの警告種別に99を格納する.
			}
		}

// Comment 2017/03/27 第2システム制御開発 Sky上野 [ServiceHub][SHRQ-002]追加対応.
		// 拡大画面警告情報を拡大用全警告発生状況確認テーブルへ格納.
		for( us_TableCnt = 0; us_TableCnt < us_GamenEnlWarningTableMax; us_TableCnt++ ){
			// CommonAPIに定義されいる警告が空でないもののみを格納する.
			if( PNLD_SHWarningNon != sp_GamenEnlWarningTable[ us_TableCnt ].us_SHWarningID ){
				stEnlNowWarningAllTbl[ us_EnlTableAllCnt ].ec_WarningName = sp_GamenEnlWarningTable[ us_TableCnt ].ec_WarningName;	// 警告コード( WarningName )を格納.
				stEnlNowWarningAllTbl[ us_EnlTableAllCnt ].us_TableNo = us_TableCnt;												// 発生警告詳細通知テーブルへ既存拡大全画面テーブルの配列番号を格納.
				stEnlNowWarningAllTbl[ us_EnlTableAllCnt ].us_SHWarningID = sp_GamenEnlWarningTable[ us_TableCnt ].us_SHWarningID;		// CommonAPIに定義されいる警告IDを格納.
				us_EnlTableAllCnt++;
			}
		}
		if( PNLD_SHNowWarningAllTblMax > us_EnlTableAllCnt ){													// 拡大画面全警告発生状況確認テーブルに空きがある場合.
			for( us_TableCnt = us_EnlTableAllCnt; us_TableCnt < PNLD_SHNowWarningAllTblMax; us_TableCnt++ ){
				stEnlNowWarningAllTbl[ us_TableCnt ].ec_WarningName = TYPE_WarningName_Non;						// 空きテーブルの警告コードをコードなし( -1 )を格納する.
				stEnlNowWarningAllTbl[ us_TableCnt ].us_SHWarningID = PNLD_SHWarningNon;							// 空きテーブルの警告IDに255を格納する.
			}
		}
		bl_SHWarningCheckTableInitialization = True;	// 初期化フラグをTrueにし、以降初期化処理を行わない,
	}
}

// Comment 2016/12/20 第2システム制御開発 Sky上野 [ServiceHub] SHRQ-002 SH上でのエラー警告対応.
/*================================================================================================
［機能］	 SH_全警告発生状況チェック関数.
================================================================================================*/
void PBasicMessageProc::checkSHWarning()
{
	ushort us_TableCnt = 0;		// 既存の全画面/2行/下1行テーブルカウント用.
	ushort us_NowDispWarning = PNLD_NowDispWarningGamenNon;		// 現在表示中警告のテーブルカウンタ格納用.
	ulong ul_GetJobIDccDisplayJob = 0;							// ジョブID cc_DisplayJob.
	ulong ul_GetJobIDcomPanelJob = 0;							// ジョブID comPanelJob.
	ulong ul_GetJobIDcomPrintJob = 0;							// ジョブID comPrintJob.

	// 全警告発生状況確認テーブルを初期化する.
	for( us_TableCnt = 0; us_TableCnt < PNLD_SHNowWarningAllTblMax; us_TableCnt++ ){
		if( PNLD_SHWarningNon == stNowWarningAllTbl[ us_TableCnt ].us_SHWarningID ){	// 通知対象警告以外に達した場合はループを抜ける.
			break;
		}
		stNowWarningAllTbl[ us_TableCnt ].bl_WarningOn = False;
		stNowWarningAllTbl[ us_TableCnt ].ul_JobID = PNLD_SHWarningJobIDNon;
	}
	
	// 通知用テーブルの初期化をする.
	for( us_TableCnt = 0; us_TableCnt < TYPE_SHSystemControl_Warning_TotalNum; us_TableCnt++ ){
		st_SHSystemControl_WarningStatus[ us_TableCnt ].m_b_WarningStatus = False;
		st_SHSystemControl_WarningStatus[ us_TableCnt ].m_e_WarningType = PNLD_WarningTypeNon;		// 警告種別を初期化.
	}
	
	PLayerControl* gp_NowWarningGamen = PNLC_PopupListControl::newInstance() -> getNowPopup();		// 現在表示中画面のインスタンスを取得.
	if( NULL == gp_NowWarningGamen ){
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "gp_NowWarningGamen is NULL!" );
		return;
	}
	
	// ジョブスペックを取得する.
	if( ( NULL == onlyPanel.cc_DisplayJob ) || ( NULL == onlyPanel.cc_DisplayJob -> getJobStatus() ) || ( NULL == onlyPanel.cc_DisplayJob -> getJobStatus() -> getJobID() ) ){
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "onlyPanel.cc_DisplayJob is NULL!" );
		ul_GetJobIDccDisplayJob = 0;
	} else{
		ul_GetJobIDccDisplayJob = onlyPanel.cc_DisplayJob -> getJobStatus() -> getJobID();
	}
	
	if( ( NULL == comPanelJob ) || ( NULL == comPanelJob -> getJobStatus() ) || ( comPanelJob -> getJobStatus() -> getJobID() ) ){
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "comPanelJob is NULL!" );
		ul_GetJobIDcomPanelJob = 0;
	} else{
		ul_GetJobIDcomPanelJob = comPanelJob -> getJobStatus() -> getJobID();
	}
	
	if( ( NULL == comPrintJob ) || ( NULL == comPrintJob -> getJobStatus() ) || ( NULL == comPrintJob -> getJobStatus() -> getJobID() ) ){
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "comPrintJob is NULL!" );
		ul_GetJobIDcomPrintJob = 0;
	} else{
		ul_GetJobIDcomPrintJob = comPrintJob -> getJobStatus() -> getJobID();
	}
	
	for( us_TableCnt = 0; us_TableCnt < PNLD_SHNowWarningAllTblMax; us_TableCnt++ ){
		if( PNLD_SHWarningNon == stNowWarningAllTbl[ us_TableCnt ].us_SHWarningID ){	// 通知対象警告以外に達した場合はループを抜ける.
			break;
		}
		switch( stNowWarningAllTbl[ us_TableCnt ].uc_WarningType ){
			case TYPE_WarningType_FullScreen :							// 全画面警告の場合.
				// A0A6090100	動画警告終了警告時は、「紙づまり解除手順表示設定」が「表示する」の場合のみ、警告検知チェックを行なう.
				if( TYPE_SHSystemControl_Warning_E_ReleaseProcedureJam == stNowWarningAllTbl[ us_TableCnt ].us_SHWarningID ){			// チェック対象警告がA0A6090100	動画警告終了警告である場合.
// Comment 2017/02/22 第2システム制御開発 Sky上野 [ServiceHub][SHRQ-002] 課題番号1013443対応 A0A6090100	動画警告終了警告チェック処理をメソッド化.
					if( False == checkSHIsShowPaperJamCancellationProcedure() ){													// 用紙セット状況確認画面が表示されていない場合は、警告検知処理をスキップする.
						break;
					}
				}
				if( True == ( onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, sp_GamenWarningTable[ stNowWarningAllTbl[ us_TableCnt ].us_TableNo ].ec_WarningName ) ) ){
					if( PNLD_SHDeleteJobIDNon == ul_GetJobIDccDisplayJob ){
						stNowWarningAllTbl[ us_TableCnt ].ul_JobID = onlyPanel.ul_DeleteJobJobID;					// ジョブが削除済みである場合は、バックアップジョブIDの値を格納.
					} else{
						stNowWarningAllTbl[ us_TableCnt ].ul_JobID = ul_GetJobIDccDisplayJob;						// ジョブIDの取得.
					}
					stNowWarningAllTbl[ us_TableCnt ].bl_WarningOn = True;											// ステータスをTrueへ変更.
// Comment 2017/03/27 第2システム制御開発 Sky上野 [ServiceHub][SHRQ-002]追加対応.
					if( PNLD_NowDispWarningGamenNon == us_NowDispWarning ){											// 現在表示中画面のステータスが初期値である場合.
						us_NowDispWarning = us_TableCnt;		// 現在参照中のテーブル番号の警告が表示中の場合は、現在のカウンタを格納する.
					}
				} else if( True == ( onlyPanel.isWarningOn( comPanelJob, sp_GamenWarningTable[ stNowWarningAllTbl[ us_TableCnt ].us_TableNo ].ec_WarningName ) ) ){
					if( PNLD_SHDeleteJobIDNon == ul_GetJobIDcomPanelJob ){
						stNowWarningAllTbl[ us_TableCnt ].ul_JobID = onlyPanel.ul_DeleteJobJobID;					// ジョブが削除済みである場合は、バックアップジョブIDの値を格納.
					} else{
						stNowWarningAllTbl[ us_TableCnt ].ul_JobID = ul_GetJobIDcomPanelJob;						// ジョブIDの取得.
					}
					stNowWarningAllTbl[ us_TableCnt ].bl_WarningOn = True;											// ステータスをTrueへ変更.
// Comment 2017/03/27 第2システム制御開発 Sky上野 [ServiceHub][SHRQ-002]追加対応.
					if( PNLD_NowDispWarningGamenNon == us_NowDispWarning ){											// 現在表示中画面のステータスが初期値である場合.
						us_NowDispWarning = us_TableCnt;		// 現在参照中のテーブル番号の警告が表示中の場合は、現在のカウンタを格納する.
					}
				} else if( True == ( onlyPanel.isWarningOn( comPrintJob, sp_GamenWarningTable[ stNowWarningAllTbl[ us_TableCnt ].us_TableNo ].ec_WarningName ) ) ){
					if( PNLD_SHDeleteJobIDNon == ul_GetJobIDcomPrintJob ){
						stNowWarningAllTbl[ us_TableCnt ].ul_JobID = onlyPanel.ul_DeleteJobJobID;					// ジョブが削除済みである場合は、バックアップジョブIDの値を格納.
					} else{
						stNowWarningAllTbl[ us_TableCnt ].ul_JobID = ul_GetJobIDcomPrintJob;						// ジョブIDの取得.
					}
					stNowWarningAllTbl[ us_TableCnt ].bl_WarningOn = True;											// ステータスをTrueへ変更.
// Comment 2017/03/27 第2システム制御開発 Sky上野 [ServiceHub][SHRQ-002]追加対応.
					if( PNLD_NowDispWarningGamenNon == us_NowDispWarning ){											// 現在表示中画面のステータスが初期値である場合.
						us_NowDispWarning = us_TableCnt;		// 現在参照中のテーブル番号の警告が表示中の場合は、現在のカウンタを格納する.
					}
				} else {
				}
				break;
					
			case TYPE_WarningType_Message :											// 2行メッセージ警告の場合.
				if( True == ( onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, sp_MessageWarningTable[ stNowWarningAllTbl[ us_TableCnt ].us_TableNo ].ec_WarningName ) ) ){
					stNowWarningAllTbl[ us_TableCnt ].ul_JobID = ul_GetJobIDccDisplayJob;	// ジョブIDの取得.
					stNowWarningAllTbl[ us_TableCnt ].bl_WarningOn = True;
				}
				break;
					
			case TYPE_WarningType_Line :											// 下1行メッセージ警告の場合.
				if( True == ( onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, sp_L1MessageWarningTable[ stNowWarningAllTbl[ us_TableCnt ].us_TableNo ].ec_WarningName ) ) ){
					stNowWarningAllTbl[ us_TableCnt ].ul_JobID = ul_GetJobIDccDisplayJob;	// ジョブIDの取得.
					stNowWarningAllTbl[ us_TableCnt ].bl_WarningOn = True;
				}
				break;
			case PNLD_WarningType_Attention2 :										// ATTENTION2警告の場合.
// Comment 2017/02/22 第2システム制御開発 Sky上野 [ServiceHub][SHRQ-002] 課題番号1013443対応 ATTENTION2警告チェック処理をメソッド化.
				if( True == checkSHAttention2( stNowWarningAllTbl[ us_TableCnt ].us_SHWarningID, us_TableCnt ) ){
					stNowWarningAllTbl[ us_TableCnt ].ul_JobID = ul_GetJobIDccDisplayJob;	// ジョブIDの取得.
					stNowWarningAllTbl[ us_TableCnt ].bl_WarningOn = True;
				}
				break;
			case PNLD_WarningType_Other:											// Other警告の場合.
// Comment 2017/02/22 第2システム制御開発 Sky上野 [ServiceHub][SHRQ-002] 課題番号1013443対応 Other警告チェック処理をメソッド化.
// Comment 2017/06/XX   第2システム制御開発 Sky上野 [ServiceHub] 課題番号1022958対応 引数追加.
				if( True == checkSHOtherWarning( stNowWarningAllTbl[ us_TableCnt ].us_SHWarningID, sp_OtherWarningTable[ stNowWarningAllTbl[ us_TableCnt ].us_TableNo ].p_WarningPopup ) ){
					stNowWarningAllTbl[ us_TableCnt ].ul_JobID = ul_GetJobIDccDisplayJob;			// ジョブIDの取得.
					stNowWarningAllTbl[ us_TableCnt ].bl_WarningOn = True;							// ステータスをTrueへ変更.
// Comment 2017/03/27 第2システム制御開発 Sky上野 [ServiceHub][SHRQ-002]追加対応.
					if( PNLD_NowDispWarningGamenNon == us_NowDispWarning ){							// 現在表示中画面のステータスが初期値である場合.
						us_NowDispWarning = us_TableCnt;											// 現在参照中のテーブル番号の警告が表示中の場合は、現在のカウンタを格納する ( 現在表示中の全画面警告用 ).
					}
				}
			break;
// Comment 2017/02/22 第2システム制御開発 Sky上野 [ServiceHub][SHRQ-002] 課題番号1013443対応.
			case TYPE_WarningType_PRT_IR :											// PRT／IR関連異常警告の場合.			
//Comment [2019/03/08] [HCL_Devipriya] [EagleH] [機器構成変更No.29] Use pointer instead of Table directly.
//				if( True == checkSHWarningType_PRT_IR( 																		// PRT/IR関連異常警告のそれぞれの警告状況をチェック.
//					PrtIrWarningList[ PNLD_SHPrtIrWarningPage ][ stNowWarningAllTbl[ us_TableCnt ].us_TableNo ].type,
//					PrtIrWarningList[ PNLD_SHPrtIrWarningPage ][ stNowWarningAllTbl[ us_TableCnt ].us_TableNo ].code ) ){
//					stNowWarningAllTbl[ us_TableCnt ].ul_JobID = ul_GetJobIDccDisplayJob;	// ジョブIDの取得.
//					stNowWarningAllTbl[ us_TableCnt ].bl_WarningOn = True;
//				}
				if( True == checkSHWarningType_PRT_IR( 																		// PRT/IR関連異常警告のそれぞれの警告状況をチェック.
					mp_PrtIrWarningList[ PNLD_SHPrtIrWarningPage ][ stNowWarningAllTbl[ us_TableCnt ].us_TableNo ].type,
					mp_PrtIrWarningList[ PNLD_SHPrtIrWarningPage ][ stNowWarningAllTbl[ us_TableCnt ].us_TableNo ].code ) ){
					stNowWarningAllTbl[ us_TableCnt ].ul_JobID = ul_GetJobIDccDisplayJob;	// ジョブIDの取得.
					stNowWarningAllTbl[ us_TableCnt ].bl_WarningOn = True;
				}
				break;
			default:
				break;
		}
	}
	sendWarningAll( us_NowDispWarning );		// CommonAPIへ全ての警告状態を通知.
}

// Comment 2016/12/20 第2システム制御開発 Sky上野 [ServiceHub] SHRQ-002 SH上でのエラー警告対応.
/*================================================================================================
［機能］	 SH_全警告発生状況をCommonAPIへ通知する.
================================================================================================*/
void PBasicMessageProc::sendWarningAll( ushort us_NowDispWarningGamen )
{
	for( ushort i = 0; i < PNLD_SHNowWarningAllTblMax; i++ ){
		if( PNLD_SHWarningNon == stNowWarningAllTbl[ i ].us_SHWarningID ){			// 通知対象警告以外に達した場合はループを抜ける.
			break;
		}

		if( PNLD_WarningTypeNon == st_SHSystemControl_WarningStatus[ stNowWarningAllTbl[ i ].us_SHWarningID ].m_e_WarningType ){							// 通知用テーブルに値がセットされていない場合のみ処理する.
			st_SHSystemControl_WarningStatus[ stNowWarningAllTbl[ i ].us_SHWarningID ].m_b_WarningStatus = stNowWarningAllTbl[ i ].bl_WarningOn;			// True/Falseを格納.
			if( i == us_NowDispWarningGamen ){																											// 現在表示中の警告である場合.
				st_SHSystemControl_WarningStatus[ stNowWarningAllTbl[ i ].us_SHWarningID ].m_e_WarningType = TYPE_WarningType_DisplayScreen;				// 警告種別に「全画面表示中」を格納.
			} else if( PNLD_WarningType_Attention2 == stNowWarningAllTbl[ i ].uc_WarningType ){
				st_SHSystemControl_WarningStatus[ stNowWarningAllTbl[ i ].us_SHWarningID ].m_e_WarningType = TYPE_WarningType_Line;						// ATTENTION2警告の場合は、下1行警告として通知する.
			} else if( PNLD_WarningType_Other == stNowWarningAllTbl[ i ].uc_WarningType ){
				st_SHSystemControl_WarningStatus[ stNowWarningAllTbl[ i ].us_SHWarningID ].m_e_WarningType = sp_OtherWarningTable[ stNowWarningAllTbl[ i ].us_TableNo ].uc_WarningType;	// Other警告時はOther警告テーブルで保持している警告種別を格納.
			} else{
				st_SHSystemControl_WarningStatus[ stNowWarningAllTbl[ i ].us_SHWarningID ].m_e_WarningType = stNowWarningAllTbl[ i ].uc_WarningType;		// 警告種別を格納.
			}
		} else{
			if( False == st_SHSystemControl_WarningStatus[ stNowWarningAllTbl[ i ].us_SHWarningID ].m_b_WarningStatus ){									// 重複しているパターンであっても、送信用ステータスがTrueであった場合を考慮し、上書きする.
				st_SHSystemControl_WarningStatus[ stNowWarningAllTbl[ i ].us_SHWarningID ].m_b_WarningStatus = stNowWarningAllTbl[ i ].bl_WarningOn;		// True/Falseを格納.
				if( i == us_NowDispWarningGamen ){																										// 現在表示中の警告である場合.
					st_SHSystemControl_WarningStatus[ stNowWarningAllTbl[ i ].us_SHWarningID ].m_e_WarningType = TYPE_WarningType_DisplayScreen;			// 警告種別に「全画面表示中」を格納.
				} else if( PNLD_WarningType_Attention2 == stNowWarningAllTbl[ i ].uc_WarningType ){
					st_SHSystemControl_WarningStatus[ stNowWarningAllTbl[ i ].us_SHWarningID ].m_e_WarningType = TYPE_WarningType_Line;					// ATTENTION2警告の場合は、下1行警告として通知する.
				} else if( PNLD_WarningType_Other == stNowWarningAllTbl[ i ].uc_WarningType ){
					st_SHSystemControl_WarningStatus[ stNowWarningAllTbl[ i ].us_SHWarningID ].m_e_WarningType = sp_OtherWarningTable[ stNowWarningAllTbl[ i ].us_TableNo ].uc_WarningType;	// Other警告時はOther警告テーブルで保持している警告種別を格納.
				} else{
					st_SHSystemControl_WarningStatus[ stNowWarningAllTbl[ i ].us_SHWarningID ].m_e_WarningType = stNowWarningAllTbl[ i ].uc_WarningType;	// 警告種別を格納.
				}
			}
		}
	}
	APIC_SHSystemControl::newInstance() -> setWarningStatus( st_SHSystemControl_WarningStatus );				// CommonAPIへ警告状況を通知.
}

// Comment 2017/02/22 第2システム制御開発 Sky上野 [ServiceHub][SHRQ-002] 課題番号1013443対応.
/*================================================================================================
［機能］	 SH_紙づまり解除手順画面表示中であるかをチェックする.
================================================================================================*/
Bool PBasicMessageProc::checkSHIsShowPaperJamCancellationProcedure()
{
	Bool  bl_ret = True;
	
	if( NULL == PnlComGetOperationSetting() ){
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "PnlComGetOperationSetting() is NULL!" );
		bl_ret = False;
	} else{
		if ( False == PnlComGetOperationSetting() -> getPaperJamReleaseProcedureDisplay() ) {		// 紙詰まり解除手順表示設定が「表示しない」であるかをチェックする.
			bl_ret = False;
		} else{
			if( False == PNLC_WarningDataLib::newInstance() -> getJamReleaseProcedure() ){			// 用紙セット状況確認画面が表示中でないかをチェックする.
				bl_ret = False;
			}
		}
	}
	
	return bl_ret;
}
// Comment 2017/02/22 第2システム制御開発 Sky上野 [ServiceHub][SHRQ-002] 課題番号1013443対応.
/*================================================================================================
［機能］	 SH_ATTENTION2警告チェック.
================================================================================================*/
Bool PBasicMessageProc::checkSHAttention2( ushort us_SHWarningID, ushort us_TableCnt )
{
	Bool  bl_ret = False;
	
	switch( us_SHWarningID ){
		case TYPE_SHSystemControl_Warning_A_RemotePanelWarning : 		// リモートパネル接続エラー.
		{
			TYPS_RemotePanelErrCodeNICFUM st_RemotePanelError = {0};
			// ネットワークエラーが発生している.
			if( PnlComGetRemotePanelSetting()->getRemotePanelErrCodeNICFUM( &st_RemotePanelError ) ) {
				bl_ret = True;
			}
		}	
		break;
		case TYPE_SHSystemControl_Warning_A_EmailIFaxWarning :			// E-mail・I-FAX自動受信エラー.
		case TYPE_SHSystemControl_Warning_A_EmailIWarning	 :			// E-mail受信エラー.
		{
			int32_t sl_ErrorResult = 0;
			if( NULL == APIC_NetworkDevice::newInstance() ){
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "APIC_NetworkDevice::newInstance is NULL!" );
				break;
			}
			if( True == APIC_NetworkDevice::newInstance()->getPopErrorCodeNICFUM( &sl_ErrorResult ) ){
				if( ( NULL == onlyPanel.cc_DisplayJob -> getJobSpec() ) || ( NULL == onlyPanel.cc_DisplayJob -> getJobSpec() -> pc_Send ) ){
					LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "c_ComSend is NULL!" );
					break;
				}
				
				TYPC_ComSend* pc_ComSend = &( onlyPanel.cc_DisplayJob -> getJobSpec() -> pc_Send -> c_ComSend );
				if( True == isEnableSetAddress( pc_ComSend, TYPE_SendModeIfax, False, False, False, True, -1, NULL, True ) ) { 	// インターネットファクスの有効有無.
					if( TYPE_SHSystemControl_Warning_A_EmailIFaxWarning == us_SHWarningID ){		// E-mail/I-FAX受信エラー時.
						bl_ret = True;
					}
				} else{
					if( TYPE_SHSystemControl_Warning_A_EmailIWarning == us_SHWarningID ){			// E-mail受信エラー時.
						bl_ret = True;
					}
				}
			}
		}	
		break;
		case TYPE_SHSystemControl_Warning_A_NetworkAttentionCloudWarning :	// クラウド接続エラー.
		{
			if( NULL == APIC_SaaSGatewayInfo::newInstance() ){
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "APIC_SaaSGatewayInfo::newInstance is NULL!" );
				break;
			}
			TYPE_SGW_STATUS c_SgwStatus = APIC_SaaSGatewayInfo::newInstance()->getStatus();
			if( TYPE_SGW_STATUS_OK != c_SgwStatus ) {
				bl_ret = True;
			}
		}
		break;
		case TYPE_SHSystemControl_Warning_A_PRT_IR_Warning:					// PRT/IR関連異常警告の場合
			if( ( True == ( onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, PATTENTION_PRTWarning ) ) ) ||			// PRT関連異常警告が発生している.
				( True == ( onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, PATTENTION_IRWarning ) ) ) || 			// IR関連異常警告が発生している.
			 	( True == ( onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, PATTENTION_GarbageDetection3 ) ) ) || 	// 原稿読取りガラス清掃警告が発生している(IR関連異常警告で検知出来ない).
				( True == ( onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, PATTENTION_ReadGuideCleaning ) ) ) ) {	// 読取りガイド清掃警告が発生している(IR関連異常警告で検知出来ない).
				bl_ret = True;
			}
			break;
		default :
			if( True == ( onlyPanel.isWarningOn( onlyPanel.cc_DisplayJob, sp_Attention2WarningTable[ stNowWarningAllTbl[ us_TableCnt ].us_TableNo ].ec_WarningName ) ) ){
				bl_ret = True;
			}
			break;
	}
	
	return bl_ret;
}
// Comment 2017/02/22 第2システム制御開発 Sky上野 [ServiceHub][SHRQ-002] 課題番号1013443対応.
// Comment 2017/06/XX   第2システム制御開発 Sky上野 [ServiceHub] 課題番号1022958対応 引数追加.
/*================================================================================================
［機能］	 SH_Other警告チェック.
================================================================================================*/
Bool PBasicMessageProc::checkSHOtherWarning( ushort us_SHWarningID, PLayerControl* p_WarningPopup )
{
	Bool  bl_ret = False;
	
	switch( us_SHWarningID ){
		case TYPE_SHSystemControl_Warning_E_DPWSConnectError :														// DPWSスキャンエラー.
		{	
			PNLE_PushScan e_WSDSScanStatus = 0;																		// DPWSスキャンステータス格納用.
			PCGC_ScanFaxWSDScanSetting* pc_WSDss = PCGC_ScanFaxWSDScanSetting::getInstance();						// PCGC_ScanFaxWSDScanSettingクラスのインスタンス取得.
			e_WSDSScanStatus = pc_WSDss -> getPanelPushScanStatus();												// DPWSスキャンのステータス取得.
			if( ( PNLE_PushScanWaitAns != e_WSDSScanStatus ) && ( PNLE_PushScanWaitVista != e_WSDSScanStatus ) ){	// 接続要求応答待ちとVista応答待ち以外のステータス時はエラーと判断.
				bl_ret = True;
			}
		}
		break;
// Comment 2017/06/XX   第2システム制御開発 Sky上野 [ServiceHub] 課題番号1022958対応.
		case TYPE_SHSystemControl_Warning_E_AppliedReadSize :														// 原稿サイズ検出エラー時の読み込みサイズ設定.
		case TYPE_SHSystemControl_Warning_E_UploadFileExistError :													// アップロードファイル選択_再選択警告.
		case TYPE_SHSystemControl_Warning_E_MfpConnectError :														// 接続エラー.
		case TYPE_SHSystemControl_Warning_E_RegionCodeError : 														// 登録アプリ_地域コード不一致エラー.
		case TYPE_SHSystemControl_Warning_E_PasswordNotUseErr :														// パスワード使用不可エラー.
		case TYPE_SHSystemControl_Warning_E_ConfirmContinualOperation :												// 操作継続確認(OCR読込み前).
		case TYPE_SHSystemControl_Warning_C_AddressChange :															// 宛先変更警告.
		{
			PNLC_PopupListControl* p_ChkWarningGamenDisp = PNLC_PopupListControl::newInstance();
			if( NULL == p_ChkWarningGamenDisp ){
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "p_ChkWarningGamenDisp is NULL!" );
				break;
			}
			bl_ret = p_ChkWarningGamenDisp -> isPopupLayer( p_WarningPopup );										// 該当画面がポップアップ管理リストに存在有無を格納.
			break;
		}
		default :
			break;
	}

	return bl_ret;
}
// Comment 2017/02/22 第2システム制御開発 Sky上野 [ServiceHub][SHRQ-002] 課題番号1013443対応.
/*================================================================================================
［機能］	 SH_PRT/IR関連異常警告チェック.
================================================================================================*/
Bool PBasicMessageProc::checkSHWarningType_PRT_IR( uchar uc_type, char sc_code )
{
	Bool  bl_ret = False;
	
	APIC_ScannerWarning* pc_ScannerWarning = PnlComGetScannerWarning();
	if( NULL == pc_ScannerWarning ){
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "pc_ScannerWarning is NULL!" );
		bl_ret = False;
	} else{
		switch( uc_type ) {
			case PRTIRTYPE_PRT:				// PRTの場合.
				// PRT関連異常警告が発生しているか.
				bl_ret = PnlComGetPrinterWarning()->isPRTWarning( ( TYPE_PRTWarningCode )sc_code );
				break;
			case PRTIRTYPE_IR:				// IRの場合.
				switch( sc_code ) {
					case PRTIRCODE_IR_S1:	// S-1：CCDクランプゲイン調整異常.
						bl_ret = comWarning.getCCDGeinAbnormal();
						break;
					case PRTIRCODE_IR_S2:	// S-2：CISゲイン調整不良.
						bl_ret = comWarning.getCISGainAbnormal();
						break;
					case PRTIRCODE_IR_D1:	// D-1：筋検出異常.
						bl_ret = pc_ScannerWarning->isWarning( ATTENTION_DirtyEDHGrass );	// EDH原稿ガラス汚れ警告(ATTENTION).
						break;
					case PRTIRCODE_IR_D3:	// D-3：CIS読み取り部ガラス汚れ.
						bl_ret = pc_ScannerWarning->isWarning( ATTENTION_DirtyCISGrass );	// CIS読取りガラス汚れ警告(ATTENTION).
						break;
					default:				// 上記以外.
						break;
				}
				break;
			default:						// 上記以外.
				break;
		}
	}
	
	return bl_ret;
}

// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
/*================================================================================================
［機能］	指定画面の表示ペインを取得する.
================================================================================================*/
// comment 2018/09/10 SCD2 K.Takei 不具合No.1053638_【開発提起】1ペイン/3ペインの切り替え時、3ペインエリアとメインエリアの表示タイミングがずれており、見栄えが悪いので改善する.
extern BRWS_RebootSettingInfo s_PanelRebootSettingInfo;

GFWE_LCD_PANE PBasicMessageProc::getGamenPane( PBasicMessageProc* pt_Gamen )
{
	ushort	us_offset = 0;
	GFWE_LCD_PANE uc_LcdPane = GFWE_LCD_PANE_3;
//comment 2020/07/11 HCLT-Prabakaran [OP_BTS-25348] placed in common to use in mutiple place.
	APIC_SystemSetting* pc_SystemSetting = APIC_SystemSetting::newInstance();
	if( NULL == pc_SystemSetting ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return GFWE_LCD_PANE_UNKNOWN;
	}
	// WebUI Vi,WebUI V2画面チェック.
// comment 2018/09/10 SCD2 K.Takei 不具合No.1053638_【開発提起】1ペイン/3ペインの切り替え時、3ペインエリアとメインエリアの表示タイミングがずれており、見栄えが悪いので改善する.
// comment 2019/06/28 Sky東島 [EagleHigh][OP_BTS-9409].
//	if ( ( &gp_AppChangeUIBrowser == pt_Gamen )
//	  || ( &gp_AppChangeBrowser == pt_Gamen )
	if ( ( &gp_AppChangeBrowser == pt_Gamen )
// comment 2020/10/20 Sky中野 [IT6_2.1][OP_BTS-28328].
//	  || ( &gp_AuthOapBrowserWaitEx == pt_Gamen )
	  || ( ( &gp_AuthOapBrowserWaitEx == pt_Gamen ) && ( False == onlyPanel.m_bl_KIOSKAndIWSAppStart ) )
	  || ( &gp_BrowserWait_popup == pt_Gamen )
	  || ( &gp_BrowserWaitMsg_popup == pt_Gamen )
// comment 2019/12/05 [HCL-Aruna] [EagleZ+][OP_BTS-15620] 
// comment 2019/12/05 [HCL-Aruna] [EagleZ+][OP_BTS-18357] 
//	  || ( &gp_AuthOapBrowserWait == pt_Gamen )   
// comment 2020/10/06 Sky東島 [IT6_2.0MR3][OP_BTS-27838]No11.
//	  || ( (&gp_AuthOapBrowserWait == pt_Gamen) && (beforeGamen == &gp_ActionInitialSetup)) 
// comment 2019/05/15 Chikuan [EagleH][OP_BTS-1792] V1遷移パフォーマンス改善.
//// comment 2018/11/10 SCD2-Sky竹菴 [Eagle][1064861] WebUIv1遷移時に常時お待ちくださいを表示させる.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
//	  || ( &gp_BrowserWaitMsg1Pane_popup == pt_Gamen )
//#endif		// ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) ).
	   ) {
		if( BRWE_BrowserBootMode_WEBUI == s_PanelRebootSettingInfo.e_BrowserBootMode ){
			uc_LcdPane = GFWE_LCD_PANE_1;
		}
// comment 2019/09/14 SCD2-Sky肥後 [EagleH][OP_BTS-13290] ブラウザ1ペイン表示対応.
		if( PNLE_BrowserChangeTabMode_WEB == PNLC_BrowserChangeTabControl::getInstance()->getBrowserChangeTabMode() ){
			uc_LcdPane = GFWE_LCD_PANE_3;
		}
	}
	
//comment 2019/08/29 HCLT-Satheesh [OP_BTS-11917] 3 pane is not applicable for Market place screen saver App.
	if ( ( &gp_AuthOapBrowserWaitEx == pt_Gamen  )
		 || ( &gp_BrowserWait_popup == pt_Gamen )
	 	 || ( &gp_BrowserWaitMsg_popup == pt_Gamen )
		) {
		if( BRWE_BrowserBootMode_ScrSvr == s_PanelRebootSettingInfo.e_BrowserBootMode ){
			uc_LcdPane = GFWE_LCD_PANE_1;
		}
	}
// comment 2020/10/06 Sky東島 [IT6_2.0MR3][OP_BTS-27838]No11.
	PM_TaskID e_taskID = PM_TASKID_PANEL;
	PMGC_PanelManager::getPriority( &e_taskID );
// comment 2019/12/12 SCD2-Sky東島 [EagleZ+][OP_BTS-11715].
	if( &gp_BrowserWaitMsg_popup == pt_Gamen ){
		if( &gp_AuthOapBrowserWait == nowGamen
		 || &gp_IwsAuthBrowserWait == nowGamen ){
// comment 2020/10/06 Sky東島 [IT6_2.0MR3][OP_BTS-27838]No11.
			if( PM_TASKID_BROWSER != e_taskID ){
				uc_LcdPane = GFWE_LCD_PANE_3;
			} else {
//comment 2020/07/11 HCLT-Prabakaran [OP_BTS-25348] 3 Pane display based on the IWS/OAP admin settings for slide menu.
				if ( True == pc_SystemSetting->getAppDisplayChangeSetting() ){
				// ブラウザ認証画面,IWS認証は3ペインのためブラウザ認証切り替え中画面の場合は3ペイン表示にする.
					uc_LcdPane = GFWE_LCD_PANE_3;
				} else {
					uc_LcdPane = GFWE_LCD_PANE_2;
				}
			}
		}
	}else if( (beforeGamen != &gp_ActionInitialSetup) 
		&& ( &gp_AuthOapBrowserWait == pt_Gamen	|| &gp_IwsAuthBrowserWait == pt_Gamen )  ) {
// comment 2020/10/06 Sky東島 [IT6_2.0MR3][OP_BTS-27838]No11.
// comment 2022/20/05 FPT-DuongDV11 [OP_BTS-38123][No.2] FVT4Reg軽微問題纏め問題対応.
			if( ( PM_TASKID_BROWSER != e_taskID ) && (beforeGamen != &gp_AuthOapBrowserLogin) && (beforeGamen != &gp_AuthOapBrowserWait) ){
				uc_LcdPane = GFWE_LCD_PANE_3;
			} else {
				// 3 pane display based on the system settings.
				if ( True == pc_SystemSetting->getAppDisplayChangeSetting() ){
					uc_LcdPane = GFWE_LCD_PANE_3;
				} else {
					uc_LcdPane = GFWE_LCD_PANE_2;
				}
			}
	}
// comment 2022/06/22 Sky東島 [IT6_2.3][制御構成変更_No.51][サービスレスメンテナンス].
	if( &gp_PopupBrowser_popup == pt_Gamen ){
		// ポップアップブラウザは2ペイン固定にする.
		return GFWE_LCD_PANE_2;
	}
	if( &gp_PopupBrowserEndWait_popup == pt_Gamen){
		// ポップアップブラウザ終了中画面は終了中画面から取得したペインにする.
		// ※ポップアップブラウザ遷移時に記録したペインと同じにするため.
		return gp_PopupBrowserEndWait_popup.getLcdPane();
	}
	if( &gp_PopupBrowserWait_popup == pt_Gamen ){
		// ポップアップブラウザ切り替え中画面は表示前のペインに依存.
		// 現在1ペインの場合は1ペイン、それ以外は3ペインとする.
		if( True == gp_PopupBrowserWait_popup.getConfirmPane() ){
			return gp_PopupBrowserWait_popup.getLcdPane();
		} else {
			// ポップアップブラウザ切り替え中画面は表示前のペインに依存.
			// 現在1ペインの場合は1ペイン、それ以外は3ペインとする.
			if( GFWE_LCD_PANE_1 == GFWC_System::getInstance()->getLcdPane() ){
				uc_LcdPane = GFWE_LCD_PANE_1;
			} else {
				uc_LcdPane = GFWE_LCD_PANE_3;
			}
		}
		return uc_LcdPane;
	}
// comment 2021/04/16 Sky Morikawa[LocalUI特ROM][TAS000000005334] サイドメニューを表示しない設定時にユーザーログアウトすると画面がちらつく問題対応.
// 下記の条件の場合、現在のOpenAPI・IWSアプリ表示設定を判定して2ペインに切り替える.
// 遷移先がA072002200_EXT アプリケーション接続中画面の場合.
// 遷移先がブラウザ再起動を待つ間に表示される待機画面の場合.
// 遷移先がブラウザの待機画面の場合且つ、遷移元がブラウザ再起動を待つ間に表示される待機画面である場合.
// 遷移先が「gp_BrowserWaitMsg_popup」の場合且つ、遷移元がブラウザ認証切り替え中画面である場合.

// comment 2021/12/06 Sky増田 [MinervaSSBK][OP_BTS-36549].
//	if( ( &gp_ScanOapAuthConnectingExt_act == pt_Gamen ) ||
	if( 
// comment 2021/09/22 Sky増田 [MinervaSSBK][OP_BTS-35075].
//		( &gp_AuthOapBrowserWaitEx == pt_Gamen ) ||
		( ( &gp_AuthOapBrowserWaitEx == pt_Gamen ) && ( PM_TASKID_BROWSER == e_taskID ) ) ||
// comment 2021/12/06 Sky増田 [MinervaSSBK][OP_BTS-36549].
//		( ( &gp_BrowserWait_popup == pt_Gamen ) && ( &gp_AuthOapBrowserWaitEx == nowGamen ) ) ||
//		( ( &gp_BrowserWaitMsg_popup == pt_Gamen ) && ( &gp_AuthOapBrowserWait == nowGamen ) ) ){
		( ( &gp_BrowserWaitMsg_popup == pt_Gamen ) && ( &gp_AuthOapBrowserWait == nowGamen ) && ( GFWE_LCD_PANE_3 != GFWC_System::getInstance()->getLcdPane() ) ) 
	){
		if  ( ( uc_LcdPane == GFWE_LCD_PANE_3 ) && 
		    ( False == pc_SystemSetting->getAppDisplayChangeSetting() ) ){
			// ここまでの処理で3ペインと判定されていて、サイドバー表示しない設定の時.
			uc_LcdPane = GFWE_LCD_PANE_2;	// 2ペインに変更する.
		}
	}
// comment 2021/09/11 FPT.KhaiPQ [MinervaSSBK][ALL MACHINE][OP_BTS-35338] サイドメニュー表示を設定する場合、CostCenter画面に右側のサイドメニューを表示しない問題対策.	
// comment 2021/08/11 FPT.ThanhLH11 [MinervaSSBK][ALL MACHINE][OP_BTS-34724] 拡張JavaScriptが有効のAuthBrowser認証管理の場合、
// CostCenter画面でキーボードを表示し、Tabキーを押下、ブラウザが広がってしまった
//	if( &gp_CostCenterResetDummy == pt_Gamen ){
//		uc_LcdPane = GFWE_LCD_PANE_2;
//	}


// comment 2019/06/28 Sky東島 [EagleHigh][OP_BTS-9409].
	if( &gp_AppChangeUIBrowser == pt_Gamen ){
		uc_LcdPane = GFWE_LCD_PANE_1;
	}

	// １ペイン画面チェック
	for( us_offset = 0; NULL != tbl_1PaneGamen[us_offset]; us_offset++ ) {
		if ( pt_Gamen == tbl_1PaneGamen[us_offset] ) {
			uc_LcdPane = GFWE_LCD_PANE_1;
			break;
		}
	}
	// １ペイン・３ペイン画面インスタンス共通チェック.
// comment 2018/04/04 SCD2 K.Nishiki [Eagle][制御構成変更No.111]ホーム画面認証画面の新UI化対応.
	if( GFWE_LCD_PANE_3 == uc_LcdPane ){
		for( us_offset = 0; NULL != tbl_1or3PaneGamen[us_offset]; us_offset++ ) {
			if ( pt_Gamen == tbl_1or3PaneGamen[us_offset] ) {
				PNLE_UIMode e_NowUIMode = PNLC_WebUIControl::getInstance()->getUIMode();
				if( PNLE_UIModeLightUI == e_NowUIMode ){
// comment 2019/06/28 Sky東島 [EagleHigh][OP_BTS-9409].
					if( NULL != dynamic_cast<PPopupCom*>( pt_Gamen ) ){
						PBasicMessageProc* pc_NowPopup = PNLC_PopupListControl::newInstance()->getNowPopup();
						if( pc_NowPopup == pt_Gamen ){
							char sc_PopupNum = PNLC_PopupListControl::newInstance()->getPopupNum();
							if( sc_PopupNum > 0 ){
								uc_LcdPane = getGamenPane( PNLC_PopupListControl::newInstance()->getPopupInstance( sc_PopupNum - 1 ) );
							} else {
								uc_LcdPane = GFWE_LCD_PANE_1;
							}
						} else {
							uc_LcdPane = GFWC_System::getInstance()->getLcdPane();
						}
						break;
// comment 2022/11/17 Sky東島 [IT6_2.3][OP_BTS-38854].
					} else {
						// 対象がポップアップ画面ではない(ベース画面)場合.
						if( PNLC_PopupListControl::newInstance()->getPopupNum() > 0 ){
							// ポップアップが表示されている場合は現在のペインを返却する.
							uc_LcdPane = GFWC_System::getInstance()->getLcdPane();
							break;
						}
					}
					// comment 2018/10/17 SCD2 K.Seto [AR.1064302]本体+拡張サーバー認証時、認証関連画面はクラシック表示とする.
					if ( ( ( &gp_UserAccountUser_act != pt_Gamen ) && ( &gp_UserAccountTrack_act != pt_Gamen ) )
					|| ( ( ( &gp_UserAccountUser_act == pt_Gamen ) || ( &gp_UserAccountTrack_act == pt_Gamen ) )
					  && ( TYPE_UserAuthenticationDeviceAndMidAuth != PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance()->getLine() ) ) ) ) {
						uc_LcdPane = GFWE_LCD_PANE_1;
					}
				}
				break;
			}
		}
	}
// comment 2018/10/12 Sky東島 [1062683] 2ペイン化対応.
	if( GFWE_LCD_PANE_3 == uc_LcdPane
	 && NULL != dynamic_cast< PPopupCom* >( pt_Gamen ) ){		// 対象画面がポップアップ画面.
		// 対象画面の前の画面のペインをチェックする.
		Bool bl_isNowPopup					= PNLC_PopupListControl::newInstance()->isNowPopup( dynamic_cast< PLayerControl* >( pt_Gamen ) );
		GFWE_LCD_PANE uc_BeforeLcdPane		= GFWE_LCD_PANE_3;
		char sc_PopupNum					= PNLC_PopupListControl::newInstance()->getPopupNum();
		// 対象が現在最前面に表示中であれば、1つ下の画面、表示されていない場合は最前面の画面を対象にする.
		PBasicMessageProc* pc_BeforeGamen	= PNLC_PopupListControl::newInstance()->getPopupInstance( True == bl_isNowPopup ? sc_PopupNum - 1 : sc_PopupNum );
		for( us_offset = 0; NULL != tbl_1PaneGamen[us_offset]; us_offset++ ) {
			if ( pc_BeforeGamen == tbl_1PaneGamen[us_offset] ) {
				uc_BeforeLcdPane = GFWE_LCD_PANE_1;
				break;
			}
		}
		if( GFWE_LCD_PANE_3 == uc_BeforeLcdPane ){
			// 遷移前画面が1ペインか3ペイン画面化をチェックする.
			for( us_offset = 0; NULL != tbl_1or3PaneGamen[us_offset]; us_offset++ ) {
				if ( pc_BeforeGamen == tbl_1or3PaneGamen[us_offset] ) {
					PNLE_UIMode e_NowUIMode = PNLC_WebUIControl::getInstance()->getUIMode();
					if( PNLE_UIModeLightUI == e_NowUIMode ){
						// comment 2018/10/17 SCD2 K.Seto [AR.1064302]本体+拡張サーバー認証時、認証関連画面はクラシック表示とする.
						if ( ( ( &gp_UserAccountUser_act != pc_BeforeGamen ) && ( &gp_UserAccountTrack_act != pc_BeforeGamen ) )
						|| ( ( ( &gp_UserAccountUser_act == pc_BeforeGamen ) || ( &gp_UserAccountTrack_act == pc_BeforeGamen ) )
						  && ( TYPE_UserAuthenticationDeviceAndMidAuth != PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance()->getLine() ) ) ) ) {
							uc_BeforeLcdPane = GFWE_LCD_PANE_1;
						}
					}
					break;
				}
			}
		}
		if( GFWE_LCD_PANE_1 == uc_BeforeLcdPane ){		// 前画面が1ペインの場合.
			for( us_offset = 0; NULL != tbl_1or2PaneGamen[us_offset].pc_1or2PaneGamen; us_offset++ ) {
				if ( pt_Gamen == tbl_1or2PaneGamen[us_offset].pc_1or2PaneGamen ) {
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//					if( &gp_HomeTopMenu == pc_BeforeGamen ){
//						// 前画面がHOMEメニューの場合は1ペインで表示を拡大させる.
//						uc_LcdPane = GFWE_LCD_PANE_1_Extention;
//					} else {
//						// 2ペイン表示.
//						uc_LcdPane = GFWE_LCD_PANE_2;
//					}
					if ( True == onlyPanel.isTopmenuMode() ) {
					// HOME画面表示時の場合は1ペインで表示を拡大させる.
						uc_LcdPane = GFWE_LCD_PANE_1_Extention;
					} else {
						// 2ペイン表示.
						uc_LcdPane = GFWE_LCD_PANE_2;
					}
					break;
				}
			}
		}
	}
// comment 2019/05/30 Sky黒田（達） [EagleH][OP_BTS-4122] WebUIライク化対応 認証終了系画面.
	for( us_offset = 0; NULL != tbl_WebUILikePaneGamen[us_offset]; us_offset++ ) {
		if ( pt_Gamen == tbl_WebUILikePaneGamen[us_offset] ) {
			if( True == onlyPanel.getFromWebUILikeScreen() ){
				uc_LcdPane = GFWE_LCD_PANE_1;
			}
			break;
		}
	}

// comment 2020/03/06 SCD2-Sky薗﨑 [IT6_2.0MR2][制御No.39]_2ペイン表示対応.
//comment 2020/07/11 HCLT-Prabakaran [OP_BTS-25348] commented and moved to top.
//	APIC_SystemSetting* pc_SystemSetting = APIC_SystemSetting::newInstance();
//	if( NULL == pc_SystemSetting ) {
//		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
//		return GFWE_LCD_PANE_UNKNOWN;
//	}

// comment 2020/06/30 Sky中野 [IT6_2.0MR3][OP_BTS-24234].
// comment 2020/10/06 Sky東島 [IT6_2.0MR3][OP_BTS-27838]No11.
//	PM_TaskID e_taskID = PM_TASKID_PANEL;
	PMGC_PanelManager::getPriority( &e_taskID );
	
	for( us_offset = 0; NULL != tbl_2or3PaneGamen[us_offset]; us_offset++ ) {
		// IWSアプリ・OpenAPIアプリのWebコンテンツ表示系画面かどうかの比較.
		if( pt_Gamen == tbl_2or3PaneGamen[us_offset] ) {
// comment 2020/05/08 Sky東島 [IT6_2.0MR2][OP_BTS-22548].
			// 管理者/ユーティリティの場合はIWS/OAPアプリ表示切換設定に依存しない.
			if( ( pt_Gamen == &gp_AppChangeBrowser )
			 && ( PNLE_BrowserChangeTabMode_Admin == PNLC_BrowserChangeTabControl::getInstance()->getBrowserChangeTabMode() ) ){
//comment 2022/01/17 FPT-DuongDV11 [OP_BTS-37160] add log.
				printf("[LocalUI] [%lu][%d] getGamenPane [%d] \n",SYSF_tickGet(), __LINE__, uc_LcdPane);
				break;
			}
			// 通常ブラウザ画面表示中の場合、BrowserLauncher画面のみ、ペイン設定の値を変更する.
// comment 2020/06/28 Sky東島 [IT6_2.0MR2][OP_BTS-25049].
//			if( ( pt_Gamen == &gp_AppChangeBrowser ) &&
//				( ScanOapExtSet.getExtAppType() != TYPE_ExtTypeBrowserLauncher ) ) {
			if( ( pt_Gamen == &gp_AppChangeBrowser ) &&
				( ( ScanOapExtSet.getExtAppType() != TYPE_ExtTypeBrowserLauncher ) || 
// comment 2020/10/20 Sky中野 [IT6_2.1][OP_BTS-28328].
//				  ( False == ScanOapExtSet.isActiveBrowserLauncher() ) ) ) {
				  ( BRWE_BrowserBootMode_Launcher != PNLC_BrowserChangeTabControl::getInstance()->getBrowserBootModeOnDisp() ) ) ) {
				continue;
			}
// comment 2020/06/30 Sky中野 [IT6_2.0MR3][OP_BTS-24234].
			// ブラウザ認証切り替え中画面の場合、優先アプリがブラウザの場合のみペイン設定の値を変更する.
			if( ( ( &gp_AuthOapBrowserWait == pt_Gamen ) || ( &gp_IwsAuthBrowserWait == pt_Gamen ) )
			 && ( PM_TASKID_BROWSER != e_taskID ) ) {
//comment 2022/01/17 FPT-DuongDV11 [OP_BTS-37160] add log.
				printf("[LocalUI] [%lu][%d] getGamenPane [%d] \n",SYSF_tickGet(), __LINE__, uc_LcdPane);
				break;
			}
			
			// IWS/OAPアプリ表示切換設定が2Paneの場合.
// comment 2023/04/13 Sky東島 [IT6_2.3][OP_BTS-42696].
//			if( False == pc_SystemSetting->getAppDisplayChangeSetting() ) {
			if( ( False == pc_SystemSetting->getAppDisplayChangeSetting() )
			 && ( PM_TASKID_BRWPNL != e_taskID ) ) {							// ブラウザパネルは一律3ペインのため.
				uc_LcdPane = GFWE_LCD_PANE_2;
			} else {
				uc_LcdPane = GFWE_LCD_PANE_3;
			}
//comment 2022/01/17 FPT-DuongDV11 [OP_BTS-37160] add log.
			printf("[LocalUI] [%lu][%d] getGamenPane [%d] \n",SYSF_tickGet(), __LINE__, uc_LcdPane);
			break;
		}
	}

	return uc_LcdPane;
}

/*================================================================================================
［機能］	ペイン更新.
 *	@param[in]	pt_Gamen			ペイン更新画面インスタンス.
 *	@param[in]	bl_dispPane			True：ペインの解像度変更+３ペインエリア表示/非表示.
 *									False:ペインの解像度変更(Default).
================================================================================================*/
// comment 2018/09/10 SCD2 K.Takei 不具合No.1053638_【開発提起】1ペイン/3ペインの切り替え時、3ペインエリアとメインエリアの表示タイミングがずれており、見栄えが悪いので改善する.
void PBasicMessageProc::updateLcdPane( PBasicMessageProc* pt_Gamen, Bool bl_dispPane )
{
// comment 2018/10/12 Sky東島 [1062683] 2ペイン化対応.
	if( NULL != dynamic_cast<PWarningcomIF*>(pt_Gamen) ){
		// PWarningcomIFは画面上に表示する画面クラスではないため,PWarningcomIFのインスタンスの場合はペイン変更しない.
		return;
	}

// comment 2018/10/26 SCD2 T.Nishiba [Eagle][AR1064089]プログラム宛先の再検索を一度だけに制限する.
	if( &gp_IGCClearWait_popup == pt_Gamen ) {
		// 透明画面のためペイン変更しない.
		return;
	}

	GFWE_LCD_PANE uc_LcdPane = getGamenPane( pt_Gamen );
// Comment 2019/07/28 [HCL-Kavitha]	OP_BTS-10477 Impact Fix for transition from WebUIHomeTop to Sleep.
	if( ( &gp_ActionInitialSetup == PBasicMessageProc::nowGamen ) && ( NULL == PNLC_PopupListControl::newInstance()->getNowPopup() ) ){
		uc_LcdPane = GFWE_LCD_PANE_1;
	}
//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] 3ペイン⇒1ペインで画面がちらつく.
	if (NULL == PNLC_PopupListControl::newInstance()->getNowPopup()) {
		//なにもしない.
	} else if (GFWE_LCD_PANE_1 == uc_LcdPane) {
// comment 2019/06/18 SCD2-Sky中野 [EagleH][OP_BTS-7964].
		PM_TaskID e_taskID = PM_TASKID_PANEL;
		PMGC_PanelManager::getPriority( &e_taskID );
// comment 2021/08/30 Sky中野 [MinervaSSBK][OP_BTS-35244].
//		if( PM_TASKID_BRWPNL == e_taskID ) {
		if( ( NULL == dynamic_cast<PPopupCom*>( pt_Gamen ) )
		 && ( &gp_AppChangeBrowser != pt_Gamen )
		 && ( &gp_AuthOapBrowserWaitEx != pt_Gamen )
		 && ( &gp_AppChangeUIBrowser != pt_Gamen )
		 && ( &gp_DummyHome != pt_Gamen )
		 && ( &gp_SetupGamen != pt_Gamen )
		 && ( &gp_AppChangeScreenSaver != pt_Gamen ) ) {
			// LocalUI画面へ遷移する場合は、画面ペインを更新する.
// comment 2022/06/22 Sky東島 [IT6_2.3][制御構成変更_No.51][サービスレスメンテナンス].
		} else if( &gp_PopupBrowserEndWait_popup == pt_Gamen ){
			// ポップアップブラウザでブラウザパネルを表示し、ポップアップブラウザ終了待機中画面を表示する場合は.
			// 画面更新をする必要があるためここでは何もしない.
// comment 2022/12/06 FPT-DuongDV11 [IT6_2.3][OP_BTS-40046]【開発提起【IT6_V102】文書ファイル処理画面でシステムオートリセット時間を待ち、スクリーンセーバー起動した画面でフッターバーが表示された
		} else if( PM_TASKID_BRWPNL == e_taskID ) {
			if( ( True == bl_dispPane ) && ( &gp_AppChangeScreenSaver == pt_Gamen )){
				GFWC_System::getInstance()->setResolution( GFWE_WSVGA_RESOLUTION_X, GFWE_WSVGA_RESOLUTION_Y );
				GFWC_System::getInstance()->setLcdDeviceInfo( m_lcdSize10_1inch );
			}
			onlyPanel.setDelayedUpdateLcdPane( True );
			return;
		}
	}else if ( PNLC_PopupListControl::newInstance()->getNowPopup() != pt_Gamen) {
		//最前面だけ更新する.
		return;
// comment 2020/12/25 OPS1 Sky東島 [IT6_2.1][OP_BTS-29934].
	} else if( uc_LcdPane == GFWC_System::getInstance()->getLcdPane()
	        && &gp_AuthOapBrowserLogin == pt_Gamen ){
		PM_TaskID e_taskID = PM_TASKID_PANEL;
		PMGC_PanelManager::getPriority( &e_taskID );
		if( PM_TASKID_BROWSER == e_taskID ){
// comment 2022/05/14 FPT.ThanhLH11 【MinervaSSBKMR】【課題OP_BTS-37180別件提起】【SYSQA-MSSBK_041】ログイン画面でソフトキーボードの表示が正常なサイズより小さく表示される問題対応.
			IMFE_3PaneMenu e_3PaneMenu = PNLC_KeyboardControlLib::newInstance()->get3PaneMenuEnable();
			if(( GFWE_LCD_PANE_2 == uc_LcdPane && IMFE_3PaneMenu_UnSet == e_3PaneMenu )
			|| ( GFWE_LCD_PANE_3 == uc_LcdPane && IMFE_3PaneMenu_Set == e_3PaneMenu )){
				return;
			}
		}
	}
	
// comment 2019/06/18 SCD2-Sky中野 [EagleH][OP_BTS-7964].
	onlyPanel.setDelayedUpdateLcdPane( False );
// Comment 2020/06/12 [HCLT-Ranjith] [IT62.0_MR3] [制御構成変更No.34] one pane display gamen 
//	if ( True == PNLC_PopupListControl::newInstance()->isNowPopup( &gp_7_0012 )){
	if ( ( True == PNLC_PopupListControl::newInstance()->isNowPopup( &gp_7_0012 ) ) 
	 || ( True == PNLC_PopupListControl::newInstance()->isNowPopup( &gp_Q070000000 ) ) 
	 || ( True == PNLC_PopupListControl::newInstance()->isNowPopup( &gp_Q0M0000000 )) ){
		if (GFWE_LCD_PANE_1 != GFWC_System::getInstance()->getLcdPane()){
     		usleep(400*1000);
		}
    }
	// 解像度変更.
	GFWC_System::getInstance()->setLcdPane( uc_LcdPane );
// comment 2022/06/22 Sky東島 [IT6_2.3][制御構成変更_No.51][サービスレスメンテナンス].
	if( ( &gp_PopupBrowserWait_popup == pt_Gamen )
	 || ( ( &gp_PublicPopupDummyBase == pt_Gamen )
	   && ( &gp_PopupBrowserWait_popup == gp_PublicPopupDummyBase.getStackPopup() ) ) ){
	   	// ポップアップブラウザへ遷移する場合はサイドメニューおよびフッターエリアの更新を行わない.
	   	// ※ダミーベースに遷移する場合はブラウザが前面表示のため更新を行うとちらついてしまう.
	   	// ※ポップアップブラウザへの切り替え中画面はポップアップ元と同じ表示となるため更新の必要がない.
		return;
	}
// comment 2022/10/31 Sky中野 [IT6_2.2MR4][OP_BTS-40320].
	// キーボード解像度変更.
	IMFE_3PaneMenu e_3PaneMenu = IMFE_3PaneMenu_Unknown;
	switch( uc_LcdPane ) {
		case GFWE_LCD_PANE_1:
		case GFWE_LCD_PANE_2:
		case GFWE_LCD_PANE_1_Extention:
			e_3PaneMenu = IMFE_3PaneMenu_UnSet;
			break;
		default:
			e_3PaneMenu = IMFE_3PaneMenu_Set;
			break;
	}
	PNLC_KeyboardControlLib::newInstance()->set3PaneMenuEnable( e_3PaneMenu );
//comment 2023/04/18 FPT.HungDM17 【IT6_2.3】【ALL MACHINE】【OP_BTS-42606】Fix coordinate of Maintenance Mode Status Bar depend on pane resolution.
#ifndef DEF_LCD_7INCH
	bl_JudgeStatusMaintenanceBar = False;
#endif
//comment 2018/10/24 HCLT_Jagadeesh [Eagle][AR1064877] Supporting keyboard gap removal and adjustment.
//	IMFE_3PaneMenu e_3PaneMenu = IMFE_3PaneMenu_Unknown;
// comment 2018/09/10 SCD2 K.Takei 不具合No.1053638_【開発提起】1ペイン/3ペインの切り替え時、3ペインエリアとメインエリアの表示タイミングがずれており、見栄えが悪いので改善する.
	if ( True == bl_dispPane ) {
		// ペイン表示/非表示設定.
		if ( GFWE_LCD_PANE_1 == uc_LcdPane ) {
//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] 1ペイン⇒3ペインで画面がちらつく.
//			PNLC_SH3PaneMenuControlLib::newInstance()->hide();
// comment 2020/05/09 Sky吉村(魁) [市場問題対応][TAS000000002878] SystemAutoReset契機による呼出し時、ちらつきが発生する.
			if ( ( False == onlyPanel.isTopmenuMode() ) || ( False ==  PBasicMessageProc::nowGamen->isNowGamen( &gp_DummyHome ) ) || ( 0 < PNLC_PopupListControl::newInstance()->getPopupNum() ) || ( PBasicMessageProc::nextGamen != NULL ) ){
// comment 2023/01/12 Sky東島 [IT6_2.3][OP_BTS-38875].
				PM_TaskID e_taskID = PM_TASKID_PANEL;
				PMGC_PanelManager::getPriority( &e_taskID );
				if( PM_TASKID_BROWSER == e_taskID
				&& False == isBrowserGamen( pt_Gamen ) ){
					// 優先タスクがブラウザかつ対象画面がブラウザ画面でない場合は右エリアを非表示にしない.
				} else {
					PNLC_SH3PaneMenuControlLib::newInstance()->sendToBack();
				}
			}
// comment 2023/09/22 Sky東島 [IT6_2.3][OP_BTS-44767].
			PNLC_SlideMenuControlLib* pc_SlideMenuControlLib = PNLC_SlideMenuControlLib::newInstance();
			if( NULL != pc_SlideMenuControlLib ){
				pc_SlideMenuControlLib->hide();
			}
//comment 2018/11/07 HCLT_Jagadeesh [Eagle][AR1064877] Supporting keyboard gap removal and adjustment.
//			e_3PaneMenu = IMFE_3PaneMenu_UnSet;
//			PNLC_KeyboardControlLib::newInstance()->set3PaneMenuEnable( e_3PaneMenu );
// comment 2018/10/12 Sky東島 [1062683] 2ペイン化対応.
		} else if( GFWE_LCD_PANE_2 == uc_LcdPane ){
			PNLC_SH3PaneMenuControlLib::newInstance()->show2Pane();
// comment 2023/09/22 Sky東島 [IT6_2.3][OP_BTS-44767].
			PNLC_SlideMenuControlLib* pc_SlideMenuControlLib = PNLC_SlideMenuControlLib::newInstance();
			if( NULL != pc_SlideMenuControlLib ){
				pc_SlideMenuControlLib->hide();
			}
//comment 2018/11/07 HCLT_Jagadeesh [Eagle][AR1064877] Supporting keyboard gap removal and adjustment.
//			e_3PaneMenu = IMFE_3PaneMenu_UnSet;
//			PNLC_KeyboardControlLib::newInstance()->set3PaneMenuEnable( e_3PaneMenu );
		} else if( GFWE_LCD_PANE_1_Extention == uc_LcdPane ){
//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] 1ペイン⇒3ペインで画面がちらつく.
//			PNLC_SH3PaneMenuControlLib::newInstance()->hide();
			PNLC_SH3PaneMenuControlLib::newInstance()->sendToBack();
//-------------------Revert-AR1064877-2018/11/06-Start----------------------
//			PNLC_SH3PaneMenuControlLib::newInstance()->hide();
//comment 2018/10/24 HCLT_Jagadeesh [Eagle][AR1064877] Supporting keyboard gap removal and adjustment.
//			e_3PaneMenu = IMFE_3PaneMenu_UnSet;
//			PNLC_KeyboardControlLib::newInstance()->set3PaneMenuEnable( e_3PaneMenu );
//-------------------Revert-AR1064877-2018/11/06-End----------------------
//comment 2018/11/07 HCLT_Jagadeesh [Eagle][AR1064877] Supporting keyboard gap removal and adjustment.
//			e_3PaneMenu = IMFE_3PaneMenu_UnSet;
//			PNLC_KeyboardControlLib::newInstance()->set3PaneMenuEnable( e_3PaneMenu );
		} else {
// comment 2019/08/08 Sky東島 [EagleHigh][OP_BTS-11122].
//			PNLC_SH3PaneMenuControlLib::newInstance()->show();
			Bool bl_show3Pane = True;
			PBaseGamenCom* pc_BaseGamen = dynamic_cast<PBaseGamenCom*>( pt_Gamen );
			if( NULL != pc_BaseGamen ){
				if( True == pc_BaseGamen->isOpenPopup() ){
					// ポップアップ予約がある場合は3ペインを表示させない.
					bl_show3Pane = False;
				}
			}
			if( True == bl_show3Pane ){
//comment 2023/04/18 FPT.HungDM17 【IT6_2.3】【ALL MACHINE】【OP_BTS-42606】Fix coordinate of Maintenance Mode Status Bar depend on pane resolution.
#ifndef DEF_LCD_7INCH
				bl_JudgeStatusMaintenanceBar = True;
#endif
				PNLC_SH3PaneMenuControlLib::newInstance()->show();
			}
//-------------------Revert-AR1064877-2018/11/06-Start----------------------
//comment 2018/10/24 HCLT_Jagadeesh [Eagle][AR1064877] Supporting keyboard gap removal and adjustment.
//			e_3PaneMenu = IMFE_3PaneMenu_Set;
//			PNLC_KeyboardControlLib::newInstance()->set3PaneMenuEnable( e_3PaneMenu );			
//-------------------Revert-AR1064877-2018/11/06-End----------------------
//comment 2018/11/07 HCLT_Jagadeesh [Eagle][AR1064877] Supporting keyboard gap removal and adjustment.
//			e_3PaneMenu = IMFE_3PaneMenu_Set;
//			PNLC_KeyboardControlLib::newInstance()->set3PaneMenuEnable( e_3PaneMenu );
		}
	}
}
//======================================================================================================================
//［機能］: Judge Status of Maintenance Bar.
//comment 2023/04/18 FPT.HungDM17 【IT6_2.3】【ALL MACHINE】【OP_BTS-42606】Fix coordinate of Maintenance Mode Status Bar depend on resolution of screen.
//======================================================================================================================
#ifndef DEF_LCD_7INCH
void PBasicMessageProc::judgeStatusMaintenanceBar()
{
	if( False == bl_JudgeStatusMaintenanceBar )
	{
		m_pc_MaintenanceModeStatusBar = &Gra_MaintenanceModeStatusBar;
	} else {
		m_pc_MaintenanceModeStatusBar = &Gra_MaintenanceModeStatusBar1;
	}	
}
#endif
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !defined (DEF_HeliosMLKCus) )	// Eagle/SparrowMFP 10.1inch.
//======================================================================================================================
//［機能］: Show/Hide Maintenance Bar.
//comment 2023/04/18 FPT.HungDM17 【IT6_2.3】【ALL MACHINE】【OP_BTS-42606】Fix coordinate of Maintenance Mode Status Bar depend on resolution of screen.
//======================================================================================================================
void PBasicMessageProc::showStatusMaintenanceBar()
{
	if( NULL != m_pc_MaintenanceModeStatusBar ) {
		m_pc_MaintenanceModeStatusBar->startGUI( 0 );
		m_pc_MaintenanceModeStatusBar->showEnable( True );
		if ( True == PnlComGetSystemInfo()->getEnableMaintenanceMode() ) {
			m_pc_MaintenanceModeStatusBar->show();		// メンテナンスモード有効時は表示する.
		} else {
			m_pc_MaintenanceModeStatusBar->hide();		// メンテナンスモード無効時は表示しない.
		}
	}
}
// comment 2018/03/16 SCD2 T.Hori [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_3ペインメニュー対応]. Start.
// comment 2019/04/06 Sky東島 [EagleHigh][OP_BTS-4953].
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !defined (DEF_HeliosMLKCus) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.

Bool PBasicMessageProc::isDisplayIDKeyByAuthenticationStatus()
{
	Bool bl_Disp = True;	//IDキー表示：True ／ IDキー非表示：False

// comment 2019/11/20 SCD2 Sky小尾 [EagleZ+][OP_BTS-14914].
	// IDキーは、ユーティリティ・管理者画面では常に受付不可となる為、非表示にする.
	// 同様にEco指標系画面もユーティリティ系画面とみなし、非表示にする.
	// それ以外の画面では認証状態のチェック結果に応じて表示を切り替える.
	if( 0 == PNLC_PopupListControl::newInstance()->getPopupNum() ){
		// 警告ポップアップが重なっていないベース画面だけの状態を判定する,
		PBasicMessageProc* chk_screen = getNowGamen();
		if( ( NULL != dynamic_cast< PUtilitycom* >(chk_screen) )
		 || ( NULL != dynamic_cast< PCGC_EcoInfoCom* >(chk_screen) ) ){
			return False;
		}
	}
// comment 2022/09/22 Sky東島 [IT6_2.3][制御構成変更_No.51][サービスレスメンテナンス].
	if( 0 == PNLC_PopupListControl::newInstance()->getPopupNum()
	 && PNLE_BrowserChangeTabMode_Admin == PNLC_BrowserChangeTabControl::getInstance()->getBrowserChangeTabMode()
	 && &gp_AppChangeBrowser == nowGamen ){
		// v1画面表示中はIDキーを非表示.
		// ※v1画面ではv1にてフッターエリアを表示するため、戻り値に関わらずIDキーは非表示.
		//   非表示にしておかないことでポップアップブラウザを表示した際にIDキーがちらついて表示されてしまうため,Falseを返却するよう変更.
		return False;
	}

	//複数回線機能の取得
	Bool bl_VlanSetting = PNLC_NetworkLan::getInstance()->getVlanSetting();

	//ユーザー認証方式の取得
	TYPE_UserAuthentication sc_UserAuthentication = APIC_UserAuthentication::newInstance()->getUserAuthentication();
	
	//部門管理モードの取得
	TYPE_TrackMode sc_TrackMode = APIC_TrackManager::newInstance()->getTrackMode();

	//複数回線機能が「無効」 かつ ユーザー認証が「認証しない」 かつ 部門管理が「管理しない」 場合、IDキーを非表示にする
	if( (False == bl_VlanSetting) &&
		 ( TYPE_UserAuthenticationOff == sc_UserAuthentication ) &&
		 ( TYPE_TrackModeOff == sc_TrackMode ) ){
		bl_Disp = False;
	}
	
// comment 2020/07/03 Sky中野 [IT6_2.0MR3_SpecialROM][OP_BTS-25275].
	if( ( True == PLBC_FuncPKICheck::isPKI() )								// PKI認証時.
	 && ( False == isContactlessPKICard() )									// AU DoD無効.
	 && ( False == isUseContactlessPKICard_MFPAuthentication() ) ) {		// ユーザーID/パスワードによるログイン禁止時.
		if( NULL != PnlComGetUser() ) {
			if( TYPE_UserTypeUser == PnlComGetUser()->getUserType() ) {		// 一般ユーザーでログイン.
				// IDキー押下を受け付けないため、キーを非表示にする.
				bl_Disp = False;
			}
		}
	}
	
	return bl_Disp;
}

// comment 2018/03/16 SCD2 T.Hori [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_3ペインメニュー対応]. End.
// comment 2019/04/06 Sky東島 [EagleHigh][OP_BTS-4953].
//#endif //#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.

// comment 2018/10/08 Sky水野 [Eagle][AR1062683] フッター対応.
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
/*================================================================================================
［機能］	言語によるリセット文字列幅チェック.
================================================================================================*/
Bool PBasicMessageProc::isWidthResetByLanguage()
{
	Bool bl_Wide = False;	// リセットキー幅 大：True ／ リセットキー幅 通常：False.

	TYPE_Language e_Language = PnlComGetLanguage();	// 表示言語を取得.
	// 文字数が大きい言語を判定する（9文字以上）.
//2020/08/27 HCL_Monisha [OP_BTS-26685] Resize for Italian language has been blocked.
	if( ( TYPE_LanguageGerman  == e_Language )		// ドイツ語.
	 || ( TYPE_LanguageFrench  == e_Language )		// フランス語.
//	 || ( TYPE_LanguageItalian == e_Language )		// イタリア語.
	 || ( TYPE_LanguageSpanish == e_Language )		// スペイン語.
	) {
		bl_Wide = True;		// リセットキー幅 大.
	}

	return bl_Wide;
}
#endif //#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !defined (DEF_HeliosMLKCus) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.

// comment 2018/10/08 Sky水野 [Eagle][AR1062683] フッター対応.
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
/*================================================================================================
［機能］	フッターパーツセットアップ処理.
================================================================================================*/
// comment 2019/03/20 SCD2-Sky K.Kasemura [IT6_2.0MR2][OP_BTS-18772] フッターエリア対応.
//void PBasicMessageProc::setupBasicStyleFooterParts( PNLS_FOOTER_SETTINGPARTS* pst_Footer ).
void PBasicMessageProc::setupBasicStyleFooterParts(PNLS_FOOTER_SETTINGPARTS* pst_Footer,Bool bl_HomeKeyIsActive, Bool bl_IDKeyIsActive, Bool bl_ResetKeyIsActive)
{
	if( NULL == pst_Footer ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "pst_Footer Failed!" );
		return;
	}

	// フッター スタートキー表示切替.
	if( NULL != pst_Footer->pc_StartKey ) {
		// スタートＬＥＤ緑.
		if( EP_LED_StartColor_Green == e_LEDColorState ) {
			pst_Footer->pc_StartKey->changeParts( tbl_s71_001_000_X168_Y50 );
			pst_Footer->pc_StartKey->activate( True );
		}
		// スタートＬＥＤ赤.
		else if( EP_LED_StartColor_Red == e_LEDColorState ) {
			pst_Footer->pc_StartKey->changeParts( tbl_s71_001_000_X168_Y50 );
			pst_Footer->pc_StartKey->activate( False );
		}
		// スタートＬＥＤ消灯.
		else if( EP_LED_StartColor_off == e_LEDColorState ) {
			pst_Footer->pc_StartKey->changeParts( tbl_i02_085_000_X168_Y50 );
			pst_Footer->pc_StartKey->activate( True );
		}
		else {
			// 何もしない.
		}
	}

	// フッター リセットキー表示切替.
	if( ( NULL != pst_Footer->pc_ResetLongKey ) && ( NULL != pst_Footer->pc_ResetKey ) ) {
		if ( True == isWidthResetByLanguage() ) {	// 言語によるリセット文字列幅チェック.
			pst_Footer->pc_ResetLongKey->showEnable( True );	// リセット(大)キー.
			pst_Footer->pc_ResetKey->showEnable( False );		// リセットキー.
		} else {
			pst_Footer->pc_ResetLongKey->showEnable( False );	// リセット(大)キー.
			pst_Footer->pc_ResetKey->showEnable( True );		// リセットキー.
		}
	}

	// フッター IDキー表示切替.
	if( NULL != pst_Footer->pc_IDKey ) {
		if ( True == isDisplayIDKeyByAuthenticationStatus() ) {	// IDキー表示／非表示チェック.
			// アイコン設定.
			Bool bl_UserCheck  = PnlComGetUserAccountOn();		// ユーザー認証ログイン状態チェック.
			Bool bl_TrackCheck = PnlComGetTrackOn();			// 部門認証ログイン状態チェック.
			if ( True == bl_UserCheck || True == bl_TrackCheck ) {
				// ログイン中の場合ログアウトアイコン設定.
				pst_Footer->pc_IDKey->changeParts( tbl_i02_081_000_X30_Y30, False );
			} else {
				// 未ログインの場合ログインアイコン設定.
				pst_Footer->pc_IDKey->changeParts( tbl_i02_082_000_X30_Y30, False );
			}
			pst_Footer->pc_IDKey->showEnable( True );		// IDキー表示.
		} else {
			pst_Footer->pc_IDKey->showEnable( False );		// IDキー非表示.
		}
	}
// comment 2019/03/20 SCD2-Sky K.Kasemura [IT6_2.0MR2][OP_BTS-18772] フッターエリア対応.
	PnlComFooterPartsBtnActiveChange(bl_HomeKeyIsActive, bl_IDKeyIsActive,bl_ResetKeyIsActive, pst_Footer);
}
#endif //#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !defined (DEF_HeliosMLKCus) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.

// comment 2018/03/16 SCD2 T.Hori [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_3ペインメニュー対応]. End.
// comment 2018/10/08 Sky水野 [Eagle][AR1062683] フッター対応.
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
/*================================================================================================
［機能］	LED更新処理.
================================================================================================*/
void PBasicMessageProc::updateLED( PNLE_LEDKind e_LEDKind, PNLE_LEDState e_LEDState )
{
//comment 2018/11/21 SCD2-Sky武井 [Eagle][AR1068247] 【IT6_EagleL】【IT6_Sparrow】【QE】Webブラウザーから印刷するとき、スタートキー表示が濃い青にならない.
static PNLE_LEDState	m_e_LEDState_PrintGreen = PNLE_LEDStateOff;
static PNLE_LEDState	m_e_LEDState_PrintRed = PNLE_LEDStateOff;

	do {
		// ＬＥＤの種類 プリント緑.
		if( PNLE_LED_PrintGreen == e_LEDKind ) {
			if( e_LEDState == PNLE_LEDStateOn || e_LEDState ==  PNLE_LEDStateBlinkSlow || e_LEDState == PNLE_LEDStateBlinkFast) {
				if ( PNLE_LEDStateOff == m_e_LEDState_PrintRed) {
					e_LEDColorState = EP_LED_StartColor_Green;					// スタートＬＥＤ緑.
				} else {
					e_LEDColorState = EP_LED_StartColor_Red;					// スタートＬＥＤ赤 (PNLE_LED_PrintRed ON優先).
				}
			}
			else{
				if ( PNLE_LEDStateOff == m_e_LEDState_PrintRed) {
					e_LEDColorState = EP_LED_StartColor_off;					// スタートＬＥＤ消灯.
				} else {
					e_LEDColorState = EP_LED_StartColor_Red;					// スタートＬＥＤ赤.
				}
			}
			m_e_LEDState_PrintGreen = e_LEDState;								// PNLE_LED_PrintGreen状態保存.
		}
		// ＬＥＤの種類 プリント赤.
		else if( PNLE_LED_PrintRed == e_LEDKind ) {
			if( e_LEDState == PNLE_LEDStateOn || e_LEDState ==  PNLE_LEDStateBlinkSlow || e_LEDState == PNLE_LEDStateBlinkFast) {
				e_LEDColorState = EP_LED_StartColor_Red;						// スタートＬＥＤ赤 (PNLE_LED_PrintRed ON優先).
			}
			else {
				if ( PNLE_LEDStateOff == m_e_LEDState_PrintGreen) {
					e_LEDColorState = EP_LED_StartColor_off;					// スタートＬＥＤ消灯.
				} else {
					e_LEDColorState = EP_LED_StartColor_Green;					// スタートＬＥＤ緑.
				}
			}
			m_e_LEDState_PrintRed = e_LEDState;									// PNLE_LED_PrintRed状態保存.
		}
		else {
			break;																// 何もしない.
		}
	} while(0);

}
#endif //#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !defined (DEF_HeliosMLKCus) )	// Eagle/SparrowMFP 10.1inch -> コンパイル対象.

// comment 2018/06/07 SCD2-Sky中野 [Eagle] Legacy追加対応.
//------------------------------------------------------------------------------
/**
* @brief 基本メッセージ処理 : ベルアイコン表示・非表示判定.
*/
//------------------------------------------------------------------------------
Bool PBasicMessageProc::isDisplayBellIcon()
{
	PNLC_SnapshotControlLib* pc_SnapshotControlLib = PNLC_SnapshotControlLib::newInstance();
	if( NULL == pc_SnapshotControlLib ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	
	// Attention警告、USB接続、PRT/IR関連異常、警告表示の有無を確認 ⇒ どれか一つでもある場合はベルアイコンを表示する.
	// Attention警告有無の確認.
	if( False == pc_SnapshotControlLib->getSkipCheckHDD() ) {
		Bool bl_isWarningOn = False;
		register ushort i = 0;
		for( i = PNLD_MessageAreaComponentInitValue; ( us_L1MessageWarningTableSize / sizeof(SP_L1MessageWarningTable) ) > i; ++i ) {
			// Attention一覧表示対象の警告を探す.
			if( True == sp_L1MessageWarningTable[i].b_AttentionList ) {
				if( ( ( PATTENTION_CopyKitColorAlarm_S == sp_L1MessageWarningTable[i].ec_WarningName ) && ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_CopyKitColorLimit_S ) ) )
				||	( ( PATTENTION_CopyKitBlackAlarm_S == sp_L1MessageWarningTable[i].ec_WarningName ) && ( True == onlyPanel.isWarningOn( comPanelJob, PERROR_CopyKitBlackLimit_S ) ) ) ) {
					// コピーキットカウンタアラーム値オーバーは対象外.
				} else {
					if( True == onlyPanel.isWarningOn( comPanelJob, sp_L1MessageWarningTable[i].ec_WarningName ) ) {
						bl_isWarningOn = True;
						break;
					}
				}
			}
		}

		Bool bl_isShowAttention = False;
		if( True == bl_isWarningOn ) {
			if( True == onlyPanel.checkL1MessageWarningDisplayEnabled( sp_L1MessageWarningTable[i].ec_WarningName ) ) {
				bl_isShowAttention = True;
			}
		}

		if ( TYPE_RemoteStatusNone != APIC_RemotePanelSetting::newInstance()->getRemoteStatus() ) {
			bl_isShowAttention = True;
		}

// comment 2022/06/07 1DWS2G-Sky [SecV6][RQ-909] Attention3追加.
		//Attention3チェックを行う.
		for(i = 0; us_Attention3WarningTableSize / sizeof(SP_Attention3WarningTable) > i; i++ ) {
			if( True == onlyPanel.isWarningOn(comPanelJob, sp_Attention3WarningTable[i].ec_WarningName) ) {
				bl_isShowAttention = True;
				break;
			}
		}
		
		if( ( True == bl_isShowAttention ) || ( 0 < onlyPanel.getNetworkAttentionWarningNum() ) ) {
			// Attention警告有り.
			return True;
		}
	}
	
	// USB接続確認.
	if( False == pc_SnapshotControlLib->getSkipCheckHDD() ) {
		APIC_SystemStatus*	pc_SystemStatus = PnlComGetSystemStatus();
		APIC_UsbMemDeviceInfo* pc_USBMemoryInfo = APIC_UsbMemDeviceInfo::newInstance();
		if( (NULL == pc_SystemStatus) ||
			(NULL == pc_USBMemoryInfo) ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return False;
		}
	
		if( (True == pc_SystemStatus->getNotSupportUSBConnectBsp())		// 未対応USB情報（BSP用） 取得(Ture：未対応USB接続状態).
		||	(True == pc_SystemStatus->getNotSupportUSBConnectSaa()) ) {	// 未対応USB情報（SAA用） 取得(Ture：未対応USB接続状態).
			// USB接続NGの場合、ベルアイコン非表示.
		} else if( TYPD_UsbMemStatusConnect == pc_USBMemoryInfo->getStatus() ) {	//サポートされているUSBがささっている.
			APIC_UsbMemSetting* pc_UsbMemSetting = APIC_UsbMemSetting::newInstance();
			PNLC_NetworkLan* pc_NetworkLan = PNLC_NetworkLan::getInstance();
			if( (NULL == pc_UsbMemSetting) ||
				(NULL == pc_NetworkLan) ) {
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
				return False;
			}
			
			// USBメモリーのユーザ機能（文書保存、文書印刷、外部メモリー文書読み込み）が使用可能な場合.
			TYPE_NetworkSpecific e_NetworkSpecific = pc_NetworkLan->getLine();
			if( (True == pc_UsbMemSetting->getEnableScanToUsb( e_NetworkSpecific )) ||
				(True == pc_UsbMemSetting->getEnableUsbDirectPrint( e_NetworkSpecific )) ||
				(True == pc_UsbMemSetting->getEnableUsbMemoryReading( e_NetworkSpecific )) ) {
				// USB接続有り.
				return True;
			}
		}
	}
	
	// PRT/IR関連異常有無確認.
	if( False == pc_SnapshotControlLib->getSkipCheckHDD() ) {
		APIC_SoftDipSw* pc_SoftDispSw = APIC_SoftDipSw::newInstance();
		if( NULL == pc_SoftDispSw ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return False;
		}
		
		Bool bl_isWarningOn = onlyPanel.isOilMarkDisp();
		if( False == pc_SoftDispSw->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW156, 1 ) ) {
			bl_isWarningOn = False;
		}
		
		if( True == bl_isWarningOn ) {
			// PRT/IR関連異常有り.
			return True;
		}
	}
	
	// 警告表示有無確認.
	Bool bl_isShowCaution = onlyPanel.bl_isEmergencySend;
	if( True == onlyPanel.b_UtilityAdminInWarning ) {
		// ユーティリティ・管理者で警告表示キーを非表示とするためのユーティリティ・管理者判定フラグが有効.
		// ⇒ 警告表示キーは非表示.
		bl_isShowCaution = False;
	}
	
	// 金額不足警告の場合、表示条件を満たすか判定.
	Bool bl_isDispShortCharge = False;
	for( short j = ( PDPD_EmergencySendTableNum - 1 ); 0 <= j; --j ) {
		if( NULL != onlyPanel.s_EmergencySendTable[j].pc_Job ) {
			if( False == onlyPanel.isShortChargeWarningOn( onlyPanel.s_EmergencySendTable[j].ec_WarningName, onlyPanel.s_EmergencySendTable[j].pc_Job ) ) {
				// 金額不足警告の表示条件を満たさない.
				bl_isDispShortCharge = True;
			} else {
				// 金額不足警告以外 or 金額不足警告の表示条件を満たす.
				bl_isDispShortCharge = False;
				break;
			}
		}
	}
	
	if( True == bl_isDispShortCharge ) {
		// 金額不足警告のみ、かつ、金額不足警告の表示条件を満たさない ⇒ 警告表示キーは非表示.
		bl_isShowCaution = False;
	}

	if( True == bl_isShowCaution ) {
		// 警告表示常有り.
		return True;
	}
	
	// ベルアイコン非表示
	return False;
}

// comment 2023/01/12 Sky東島 [IT6_2.3][OP_BTS-38875].
//------------------------------------------------------------------------------
/**
* @brief 基本メッセージ処理 : 対象画面がブラウザ画面か判定する.
* @param  PBasicMessageProc* pt_Gamen		対象画面.
* @param  Bool bl_includeWaitGamen			ブラウザ切替中画面を対象にするか True：対象にする(default) False：対象にしない.
* @return Bool True ブラウザ画面 False ブラウザ画面ではない .
*/
//------------------------------------------------------------------------------
Bool PBasicMessageProc::isBrowserGamen( PBasicMessageProc* pt_Gamen , Bool bl_includeWaitGamen )
{
	Bool bl_result = False;
	if( ( &gp_AppChangeScreenSaver == pt_Gamen )
	 || ( &gp_PullPrintOapBrowserDummy == pt_Gamen ) 
	 || ( &gp_GeneralOapBrowserDummy == pt_Gamen ) 
	 || ( &gp_ScanOapBrowserDummy == pt_Gamen )
	 || ( &gp_AuthOapBrowserLogin == pt_Gamen )
	 || ( &gp_CostCenterResetDummy == pt_Gamen )
	 || ( &gp_UserSetupBrowser == pt_Gamen )		// ユーザーセットアップ画面.
	 || ( &gp_AppChangeBrowserLineMode == pt_Gamen )	// ラインモード(ブラウザ)画面.
	 || ( &gp_PopupBrowser_popup == pt_Gamen ) 		// ポップアップブラウザ.
	 || ( &gp_IwsAuthBrowserLogin == pt_Gamen )
	 || ( &gp_AppChangeBrowser == pt_Gamen )
	 || ( &gp_AppChangeUIBrowser == pt_Gamen ) ){
		bl_result = True;
	}
	if( True == bl_includeWaitGamen ){
		if( ( &gp_IwsAuthBrowserWait == pt_Gamen )
		 || ( &gp_AuthOapBrowserWait == pt_Gamen ) 
		 || ( &gp_AppChangeBrowserLineModeWait == pt_Gamen ) 
		 || ( &gp_PopupBrowserWait_popup == pt_Gamen )
		 || ( &gp_PopupBrowserEndWait_popup == pt_Gamen )
		 || ( &gp_WebUIWait_popup == pt_Gamen )					// WebUIへの遷移準備完了通知までに表示する透過画面.		// comment 2023/01/27 Sky東島 [IT6_2.3][OP_BTS-40908].
		 || ( &gp_BrowserWait_popup == pt_Gamen )				// comment 2023/11/02 Sky東島 [IT6_2.3][OP_BTS-45330].
		 || ( &gp_AuthOapBrowserWaitEx == pt_Gamen )		// ユーザーセットアップ画面.
		 || ( &gp_UserSetupBrowserWait == pt_Gamen ) ){		// ラインモード(ブラウザ)画面.
			bl_result = True;
		}
	}
	return bl_result;
}

// ===============================================================================================.
//	概要	: 特殊画面表示の変化_IWSRemoteJob判断用関数.
//  引数    : なし.
//	戻り値	: True：変化有り、False：変化なし.
//	変更履歴:2019/10/31 2SCD2G-Sky菅野 [特ROM対応][IT5-148]【SOLIMP3様】音声操作によるジョブ実行(IWS連携)要望_最終版リリース.
// ===============================================================================================.
Bool PBasicMessageProc::isSpecialGamenShowStatus_IWSRemoteJob()
{
	Bool b_result = False;

	if( True == PnlComIsIWSRemoteJobFuncEnable() ) {

		static Bool b_PreIWSRemoteJobEntry = False;											// 前回のIWS_WebAPIジョブ実行中ロック状態.
		PNLC_NotifyEventInfo* pc_NtfEventInfo = PNLC_NotifyEventInfo::newInstance();		// データの管理クラス.
		if( NULL == pc_NtfEventInfo ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return False;
		}
		// IWS_WebAPIからのジョブ実行中.
		Bool b_IWSRemoteJobEntry = False;
		b_IWSRemoteJobEntry = pc_NtfEventInfo->getIWSRemoteJobEntry();
		b_IWSRemoteJobEntry |= pc_NtfEventInfo->getNotRunIWSRemoteJobEntryLock();
		if( b_IWSRemoteJobEntry != b_PreIWSRemoteJobEntry ) {
			if ( False == b_IWSRemoteJobEntry && True == b_PreIWSRemoteJobEntry ) {
				PNLC_PopupListControl::newInstance()->closeSpecifiedPopup( &gp_IWSRemoteJobEntry_act );

				if ( True == onlyPanel.isWarningOn( comPanelJob, PNL_ERROR_HistoryError ) ) {
					// 履歴警告画面が表示されるケースは、コピー画面への遷移はできないため、コピージョブでの初期化はしない.
				} else {
					// ジョブ終了したので、新たにcmoPanelJobジョブ生成.
					c_JobControl.InitJobSetting( cp_copyJob );				// コピージョブを初期化.
					comPanelJob = cp_copyJob;
					// IGCWati表示フラグが立っていると履歴警告が閉じない問題が起きる為、コピー遷移させるタイミングで.
					// フラグを落とす処理を追加.
					if ( True == pc_ManageIGCWaitDisplay.getDisplayFLG() ) {
						pc_ManageIGCWaitDisplay.setDisplayFLGOff();
					}
				}
			}
			b_result = True;
			b_PreIWSRemoteJobEntry = b_IWSRemoteJobEntry;
			if( c_SystemControl.getAdminModeOn() == True ){						// 管理者モード中.
				if( False == b_IWSRemoteJobEntry ) {
					onlyPanel.bl_isGetUsrInfoEndOfSpeGamen = True;
				}
			}
			pc_NtfEventInfo->setNotRunIWSRemoteJobEntryLock( False );

		}else{
			if( True == pc_NtfEventInfo->getIWSRemoteJobEntry() ){
				PMessageProcCom::c_autoRestTimer.c_AutoResetCopy.resetTimer( True );		// コピーオートリセットタイマーを初期化.
				PMessageProcCom::c_autoRestTimer.c_AutoResetScan.resetTimer( True );		// スキャンオートリセットタイマーを初期化.
				PMessageProcCom::c_autoRestTimer.c_AutoResetFax.resetTimer( True );			// ＦＡＸオートリセットタイマーを初期化.
				PMessageProcCom::c_autoRestTimer.c_AutoResetOnlyFax.resetTimer( True );		// ファクス専用オートリセットタイマーを初期化 .
				PMessageProcCom::c_autoRestTimer.c_AutoResetBox.resetTimer( True );			// ＢＯＸオートリセットタイマーを初期化.
				PMessageProcCom::c_autoRestTimer.c_AutoResetBrowser.resetTimer( True );		// ＷＥＢブラウザオートリセットタイマーを初期化.
				PMessageProcCom::c_autoRestTimer.c_AutoResetAll.resetTimer( True );			// システムのタイマーをリセットする.

				APIC_Job* pc_CheckJob = NULL;
				PNLC_JobListLib::getInstance()->getOneJobFromActiveAllJobList( &pc_CheckJob );
				if ( NULL != pc_CheckJob ) {
					comPanelJob = pc_CheckJob;
				} else {
					SYSF_logMsg("PNLC_JobListLib::getOneJobFromActiveAllJobList return NULL \n", 0, 0, 0, 0, 0, 0);
				}

				// 全画面警告表示中.
				if( ( pc_CheckJob == NULL )
				 || (    pc_CheckJob != NULL
				      && 0 != onlyPanel.isAllGamenWarningCom( pc_CheckJob ) )
				 ){
					// 対象警告が動画終了警告且つ、スキャン・プリント停止中以外の場合.
					if( ( pc_CheckJob != NULL )
					 && ( True  == onlyPanel.isWarningOn( pc_CheckJob, PNL_ERROR_MOVIEEND ) )
					 && ( False == APIC_PrinterStatus::newInstance()->getJAMReleaseFlag() )
					 && (    EP_PNS_ReadErrorStop  != onlyPanel.ep_PanelStatus
					      && EP_PNS_ReadStop       != onlyPanel.ep_PanelStatus
					      && EP_PNS_PrintWait      != onlyPanel.ep_PanelStatus
					      && EP_PNS_PrintErrorStop != onlyPanel.ep_PanelStatus
					      && EP_PNS_PrintStop      != onlyPanel.ep_PanelStatus )
					  ){
						// 実行中画面再表示する条件(ここでは処理なし).
					} else  {
						if( &gp_IWSRemoteJobEntry_act == PNLC_PopupListControl::newInstance()->getNowPopup() ) {
							PNLC_PopupListControl::newInstance()->closeSpecifiedPopup( &gp_IWSRemoteJobEntry_act );
							b_result = True;	// 実行中を消去するので画面表示の変化ありにする.
						}
					}
				}
			}
		}
	}
	return b_result;
}

// comment 2021/08/27 Sky中野 [MinervaSSBK][OP_BTS-34811] 優先タスク切り替え関連.
//------------------------------------------------------------------------------
/**
* @brief 基本メッセージ処理 : 優先タスク切り替え応答待ちタイマーリセット.
*/
//------------------------------------------------------------------------------
void PBasicMessageProc::resetAppChangeWaitTimer()
{
	m_uc_AppChangeWaitTimer = 0;
}

//------------------------------------------------------------------------------
/**
* @brief 基本メッセージ処理 : 優先タスク切り替え応答待ちタイマー確認.
*/
//------------------------------------------------------------------------------
const uchar uc_WaitAppChangeMaxCount = 15;
void PBasicMessageProc::checkAppChangeWaitTimer()
{
	PM_TaskID e_TaskId = PM_TASKID_PANEL;
	PMGC_PanelManager::getPriority( &e_TaskId );
	if( PM_TASKID_PANEL == e_TaskId ) {
		m_uc_AppChangeWaitTimer = 0;
	} else {
		++m_uc_AppChangeWaitTimer;
	}
	
	if( uc_WaitAppChangeMaxCount >= m_uc_AppChangeWaitTimer ) {
		return;
	}
	
	// 優先タスク切り替えが一定時間以上完了しなかった.
	// 新詳細コード出力.
	LAPC_BFW_System *pc_BFW_System = LAPC_BFW_System::newInstance();
	if (NULL != pc_BFW_System){    
	    pc_BFW_System ->registDetailCode( PNLD_DetailCode_FileIdentification_0x1900740020000000 | ( __LINE__ << 4 ) );
	}
	m_uc_AppChangeWaitTimer = 0;
}

//------------------------------------------------------------------------------
/**
* @brief 基本メッセージ処理 : DrawComplete通知確認.
*/
//------------------------------------------------------------------------------
const uchar uc_WaitDrawCompleteNoticeMaxCount = 15;
void PBasicMessageProc::checkDrawCompleteNotice()
{
	static uchar uc_WaitDrawCompleteNotice = 0;
	if( True == onlyPanel.getWaitDrawCompleteNotice() ) {
		++uc_WaitDrawCompleteNotice;
	} else {
		uc_WaitDrawCompleteNotice = 0;
	}
	
	if( uc_WaitDrawCompleteNoticeMaxCount >= uc_WaitDrawCompleteNotice ) {
		return;
	}
	
	printf( "[LocalUI][PBasicMessageProc::checkDrawCompleteNotice] Send PM_APPCHGSTS_DRAWCOMPLETE(Tick:%lu)\n", SYSF_tickGet() );
	onlyPanel.setDrawCompleteStatus( PNLE_DrawComplete_Ready );
	uc_WaitDrawCompleteNotice = 0;
}

// comment 2021/12/20 Sky西場 [MinervaSSBK][OP_BTS-34811]しばおま画面のFreezeチェック.
// comment 2022/09/09 Sky西場 [IT6_2.2MR4][制御構成変更_No.36]しばおま画面のFreezeチェック.
//------------------------------------------------------------------------------
/**
* @brief 基本メッセージ処理 : しばおま画面のFreezeチェック設定.
*/
//------------------------------------------------------------------------------
void PBasicMessageProc::setFreezeCheckSetting( ushort us_CounterMax, ushort us_Val1 )
{
	ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[PBasicMessageProc::setFreezeCheckSetting] us_CounterMax:[%d] us_Val1:[%u] [%s]", us_CounterMax, us_Val1, typeid(*this).name(), 0, 0, 0 );

	m_us_FreezeCheckCounterMax = us_CounterMax;
	m_us_FreezeCheckVal1 = us_Val1;
	setFreezeCheckVal2( 0 );
	if( m_us_FreezeCheckCounterMax <= 0 ) {
		setFreezeCheckEnable( False );
	} else {
		setFreezeCheckEnable( True );
	}
	clearFreezeCheckCounter();
}

//------------------------------------------------------------------------------
/**
* @brief 基本メッセージ処理 : Freezeチェック用カウンタクリア.
*/
//------------------------------------------------------------------------------
void PBasicMessageProc::clearFreezeCheckCounter( void )
{
	m_us_FreezeCheckCounter = 0;
}

//------------------------------------------------------------------------------
/**
* @brief 基本メッセージ処理 : Freezeチェック有効/無効設定.
*/
//------------------------------------------------------------------------------
void PBasicMessageProc::setFreezeCheckEnable( Bool bl_Enable )
{
	if( m_bl_FreezeCheckEnable != bl_Enable ) {
		m_bl_FreezeCheckEnable = bl_Enable;
		clearFreezeCheckCounter();
	}
}

//------------------------------------------------------------------------------
/**
* @brief 基本メッセージ処理 : m_uc_FreezeCheckVal2設定.
*/
//------------------------------------------------------------------------------
void PBasicMessageProc::setFreezeCheckVal2( uchar uc_Val )
{
	m_uc_FreezeCheckVal2 = uc_Val;
}

// comment 2022/09/09 Sky西場 [IT6_2.2MR4][制御構成変更_No.36]しばおま画面のFreezeチェック.
//------------------------------------------------------------------------------
/**
* @brief 基本メッセージ処理 : m_uc_FreezeCheckVal2の指定bitに0/1を設定する.
* @param[in] uc_Bit		設定するbitの位置(0 - 7).
* @param[in] bl_Val		指定bitに設定する値(False:0、True:1).
*/
//------------------------------------------------------------------------------
void PBasicMessageProc::setFreezeCheckVal2Bit( uchar uc_Bit, Bool bl_Val )
{
	if( 7 < uc_Bit ) {
		// 範囲外.
		return;
	}

	uchar uc_temp = static_cast<uchar>(0x01 << uc_Bit);

	if( bl_Val == True ) {
		m_uc_FreezeCheckVal2 = static_cast<uchar>(m_uc_FreezeCheckVal2 | uc_temp);
	} else {
		m_uc_FreezeCheckVal2 = static_cast<uchar>(m_uc_FreezeCheckVal2 & (~uc_temp));
	}
}

//------------------------------------------------------------------------------
/**
* @brief 基本メッセージ処理 : m_uc_FreezeCheckVal2取得.
*/
//------------------------------------------------------------------------------
uchar PBasicMessageProc::getFreezeCheckVal2( void )
{
	return m_uc_FreezeCheckVal2;
}

//------------------------------------------------------------------------------
/**
* @brief 基本メッセージ処理 : Freezeチェック.
*/
//------------------------------------------------------------------------------
void PBasicMessageProc::executeFreezeCheck( void )
{
	if( (m_us_FreezeCheckCounterMax > 0) && (True == m_bl_FreezeCheckEnable) ) {
		TYPE_PowerSaveStatus e_PowerSaveStatus = ppc_Power->getPowerSaveStatus();
		if( TYPE_PowerSaveStatusIdle == e_PowerSaveStatus ) {
			// Freezeチェック有効.
			m_us_FreezeCheckCounter++;
			if( m_us_FreezeCheckCounter > m_us_FreezeCheckCounterMax ) {
				// 指定時間以上画面が表示されたままのためFreezeと判断する.
				setFreezeCheckEnable( False );	// チェックを繰り返さないためにOFFにする.

				uint32_t ui32_index = 0;
				for( ui32_index = 0; sim_screenjumptablePopupGamen[ui32_index].pt_bpGamen != NULL; ui32_index++ ) {
					if( this == sim_screenjumptablePopupGamen[ui32_index].pt_bpGamen ) {
						break;
					}
				}

// comment 2022/09/09 Sky西場 [IT6_2.2MR4][制御構成変更_No.36]しばおま画面のFreezeチェック.
				LAPC_BFW_System *pc_BFW_System = LAPC_BFW_System::newInstance();
				if( NULL != pc_BFW_System ) {
					// 新詳細コード.
					// 1  -  6文字目：190077固定.
					// 7  - 10文字目：ポップアップ画面Jump用テーブル(sim_screenjumptablePopupGamen)のindex.
					// 11 - 14文字目：m_us_FreezeCheckVal1(主な用途：IGCWait等1つの画面を複数ケースで使用する場合に区別する).
					// 15 - 16文字目：m_uc_FreezeCheckVal2(主な用途：画面内で複数イベントを待つ等ケースでどのイベント待ちかを区別する).
					uint64_t ui64_DetailCode = PNLD_DetailCode_Message_0x1900770000000000;
					uint64_t ui64_temp = 0;
					if( sim_screenjumptablePopupGamen[ui32_index].pt_bpGamen == NULL ) {
						ui64_temp = 0xFFFF;
					} else if( ui32_index > 0xFFFF ) {
						ui64_temp = 0xFFFF;					// 念のため4桁を超えないようにする.
					} else {
						ui64_temp = ui32_index;
					}
					ui64_DetailCode = ui64_DetailCode | (ui64_temp << 24);
					ui64_temp = m_us_FreezeCheckVal1;
					ui64_DetailCode = ui64_DetailCode | (ui64_temp << 8);
					ui64_temp = m_uc_FreezeCheckVal2;
					ui64_DetailCode = ui64_DetailCode | ui64_temp;
					pc_BFW_System->registDetailCode( ui64_DetailCode );
				}
				ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[PBasicMessageProc::executeFreezeCheck] freeze. [%s] [0x%08X][%s]", sim_screenjumptablePopupGamen[ui32_index].sc_strScreenNumber, this, typeid(*this).name(), 0, 0, 0 );
				if( PnlComGetMachineSetting()->getRebootSettingForCRankFATAL() == TYPE_RebootSettingTypeReboot ) {
					// comment 2023/02/02 Sky浦田 [IT6_2.3][OP_BTS-41324].
					onlyPanel.forceRebootRequestForSpecifyTime();					// 再起動要求を行う.
					APIC_SystemServiceFunction::newInstance()->reboot( True );		// 再起動実行.
				}
			}
		} else {
			clearFreezeCheckCounter();
		}
	}
}
