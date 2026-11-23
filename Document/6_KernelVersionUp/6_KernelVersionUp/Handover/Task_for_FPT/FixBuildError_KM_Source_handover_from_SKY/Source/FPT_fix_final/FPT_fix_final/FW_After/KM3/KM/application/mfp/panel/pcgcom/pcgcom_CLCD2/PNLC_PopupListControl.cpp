// ===============================================================================================
/**
 *	@file	PNLC_PopupListControl.cpp
 *	@brief	ポップアップ起動状態管理クラス.
 */
// ===============================================================================================
 
/*================================================================================================
	インクルード・ファイル.
================================================================================================*/
#include <panel/plibh/plibh_CLCD2/PNLC_StackManJobSpec.h>
#include "PNLC_PopupListControl.h"
#include "PNLC_TopMenuManager.h"
#include "pctChangeGamenInfo.h"
#include <panel/pcgbox/pcgbox_CLCD/pipbox_CLCD.h>
#include <panel/pcgcomponent/pcgcomponentg/PSystemTabComponent.h>
#include <panel/pcgwarn/pcgwarn_CLCD2/PPopupWarningCom.h>
#include <panel/pcgwarn/pcgwarn_CLCD2/PPopupCombinationError.h>
#include <panel/pcgstub/pcgstub_FUM5/pigstub_FUM5.h>
#include <panel/ppcg/ppcg_CLCD/ppcgtextf_CLCD.h>
#include <panel/pcgcom/pcgcomg/pimsg.h>
#include <panel/plibh/plibh_CLCD2/PNLC_OperationLog.h>

#include <panel/pcgcomponent/pcgcomponentg/PFunctionBadgeComponent.h>

#include <panel/pcgact/pcgact_CLCD2/pigact_pA070200.h>		// 伊丹Sky(鈴木)　2010/10/08 警告画面問題点対応 .
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070702.h>		// 伊丹(Sky) 国分 2010/10/12 警告画面問題点対応 No.102 .
#include <panel/pcgact/pcgact_CLCD2/pigact_pA072000.h>		// [CQ.350923]読込み停止中にIGCWait画面を経由するとメニューへ遷移できない.
#include <panel/pcgact/pcgact_CLCD2/pigact_pA072001.h>		// comment OAP-NUI Sky石賀 2011/09/26 [CQ355137].

#include <panel/pcgwarn/pcgwarn_CLCD2/pigwarn_pA0A3250.h>
#include <panel/pcgwarn/pcgwarn_CLCD2/pigwarn_pA0A3040.h>

#include <panel/pcgwarn/pcgwarn_CLCD2/pigwarn_pA0A3010.h> // 伊丹Sky(八木)  2010/11/10 CQ334380 .
#include <panel/pcgwarn/pcgwarn_CLCD2/pigwarn_pA0A8010.h>
#include <panel/pcgcom/pcgcom_CLCD/pigcom_CLCD.h>
#include <system/cnt/CNTC_CntIf.h>
extern Bool b_StapleEmptySetAvoid;

Bool b_UserPrintLimitExceed = False;

#include <panel/pcgact/pcgact_CLCD2/pigact_pA072002.h>

#include <panel/plibl/plibl_CLCD/tskpnldraw_CLCD.h>
#include <panel/pcgbox/pcgbox_CLCD2/pigbox_pA056300.h>
#include <panel/plibh/plibh_CLCD2/PNLC_TabletResetControlLib.h>
#include <panel/pcgscanfax/pcgscanfax_PD/pcgscanfax_A034102000.h>	// gp_A034102000.
#include <panel/pcgscanfax_ext/pcgscanfax_ext_PD/pcgscanfax_B034102000_EXT.h>
#include <panel/pcgbox/pcgbox_PD/pcgbox_A053902000.h>
// comment 2014/10/01 OPS1-Sk蒲原 [PZH2.1c][CQ516371] OAPの警告表示中もベースの処理を行わない(ユーティリティの上にOAP警告表示中、前ドアOpenでユーティリティに遷移する).
#include <panel/pcgwarn_ext/pcgwarn_ext_CLCD2/POAP_PopupWarningCom.h>

#include <panel/pcgwarn/pcgwarn_CLCD2/pigwarn_pA0A3200.h>
// comment 2016/08/23 SCD2-2G Sky松下 [IT5_4.0][RQ-563] 簡単セキュアプリント対応.
#include <panel/pcgbox/pcgbox_PD/pcgbox_M056300400.h>
// comment 2017/5/8 SCD2-Sky工藤 [特ROM][FQ-OPSD-1700111-OFP079] コピー画面からトップ画面に遷移した後、トップ画面に登録したファクス/スキャンプログラムを押すと登録した宛先名が表示されない.
#include <panel/pcgscanfax/pcgscanfax_CLCD2/pigscanfax_base.h>

// Comment 2017/05/08 HCL-Hanuman [IT5_4.0] [AR1021452] Show/Hide of slide menu key for Popup screens moved to PPopupCom::opened() function.
//// Comment 2017/03/21 HCL-Hanuman [IT5_4.0] [AR1017462] Added header file to show slide menu.
#include <panel/plibh/plibh_CLCD2/PNLC_SlideMenuControlLib.h>
//Comment 2017/09/04 [HCL-Swathi] [IT5 4.2][RQ-686] [WP732]Multiple Network support.
#include <panel/plibh/plibh_CLCD2/PNLC_NetworkLan.h>
#include <panel/pcgcom/pcgcomg/pcgcom.h>
#include <panel/pcgact/pcgact_CLCD/pigact_CLCD.h>
//comment 2017/07/05 SCD2-Sky肥後 [1026857] 新詳細コード対応.
#include <panel/pdp/pdpg/pdp_DetailCode.h>
#include <panel/pcgcom_ext/pcgcom_extg/PCGC_PublicPopupDummyBase.h>		// comment 2020/12/18 Sky中野 [IT6_2.1][OP_BTS-29896].
// comment 2020/11/18 Sky Kawanishi [LocalUI特ROM] SegF(スタックオーバーフロー)時ログ改善.
#include "../LocalUI/panel/Shell/DebugTask.h"
// comment 2018/10/12 Sky東島 [1062683] 2ペイン化対応.
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
#include <panel/pcgact/pcgact_PD/pcgact_A072002900.h>
extern PA072002900 gp_A072002900;
#endif // #if !defined(DEF_SparrowAIOSFPCus)	// Eagle/SparrowMFP 10.1inch.
extern PA034102000 gp_A034102000;
extern PB034102000_EXT gp_B034102000_EXT;
extern PA053902000 gp_A053902000;
// comment 2016/08/23 SCD2-2G Sky松下 [IT5_4.0][RQ-563] 簡単セキュアプリント対応.
extern PM056300400 gp_M056300400;
/*================================================================================================
	static メンバ変数の宣言.
================================================================================================*/
PNLE_ShortCutID			PNLC_PopupListControl::e_ShortCutPopupID = PNLE_ShortCutID_NON;
PNLE_ShortCutKindID		PNLC_PopupListControl::e_ShortCutPopupKindID = PNLE_ShortCutKindID_NON;
Bool					PNLC_PopupListControl::b_StartAnimation = False;
PBasicMessageProc*	PNLC_PopupListControl::p_PopupTable[ PNLD_PopupMaxNum ];	//!< ポップアップ管理リスト.
PBaseWidget*	PNLC_PopupListControl::pc_CapturePopupBaseTable[PNLD_CAPTURE_POPUP_NUM_MAX];	//!< 画面キャプチャWidget管理テーブル.
PBaseWidget*	PNLC_PopupListControl::pc_CapturePopupKeyBoard = 0;	//!<キーボード2重画面用キャプチャWidget.
char			PNLC_PopupListControl::c_PopupNum = 0;						//!< 表示中のポップアップ数.
PBasicMessageProc*	PNLC_PopupListControl::p_SrcGamen = NULL;					//!< 最上位のポップアップを起動した画面インスタンス.
PNLS_PopupListControl_OpeningInfo	PNLC_PopupListControl::s_ReserveTable[3];	//!< ポップアップ起動予約情報テーブル.
char								PNLC_PopupListControl::c_ReservedNum = 0;	//!< 予約ポップアップ数.
PNLS_PopupListControl_PopupQueue	PNLC_PopupListControl::s_PopupQueue[ PNLD_PopupQueueMax ];		//!< オープン/クローズ要求待ち.
char								PNLC_PopupListControl::sc_QueueCount;							//!< 要求待ちの数.
// ポップアップ切替対応.
PLayerControl*	PNLC_PopupListControl::p_ChangeGamen = NULL;						//!< ポップアップ切替先画面.
PLayerControl* PNLC_PopupListControl::m_pc_nextPopupInstance = NULL;
Bool			PNLC_PopupListControl::b_mRemoveAllPopup = False;					//!< removeAllPopupからコールされたか判定.
Bool			PNLC_PopupListControl::b_mSetChangePopup = False;					//!< setChangePopupからコールされたか判定.
// comment 2022/09/22 Sky東島 [IT6_2.3][制御構成変更_No.51][サービスレスメンテナンス].
Bool			PNLC_PopupListControl::m_bl_CloseSpecifiedPopup = False;			//!< closeSpecifiedPopupからコールされたか判定.
PLayerControl*	PNLC_PopupListControl::m_p_closeLowerPlusGamen = NULL;				//!< 一つ下のポップアップも同時に閉じる画面.
// comment 2015/06/04 OPS1-Sky浜野 [MinervaSBK][UV-01457] 画面ちらつき抑止.
Bool			PNLC_PopupListControl::m_bl_CapturePopupDisable = False;

// 強制遷移画面判定用構造体.
struct PNLS_WarningPopupForceMoveInfo {
	PLayerControl* p_WarnPopupInstance;
};

struct PNLC_PopupNGGamenTable {
	PLayerControl*	pc_PopupGamen;			// ポップアップする警告画面のインスタンス.
	PLayerControl*	pc_WokeGamen;			// ポップアップを抑制する警告画面のインスタンス.
	Bool			b_DeleteFlag;			// True:ポップアップ警告を解除.
};
static const PNLC_PopupNGGamenTable	s_PopupNGGamenTable[]	= {
	// ポップアップする警告画面のインスタンス											ポップアップを抑制する警告画面のインスタンス.
	{&gp_CounterLimitComSetting_warn,	&gp_SectionUserColorCounterOverMaxPrint_warn,	True},		// A0A3250100_部門／ユーザー別カラーカウンター上限値オーバー(設定時)				A0A3250300_部門／ユーザー別カラーカウンター上限値オーバー(プリント時).
	{&gp_BlackSectionMaxValue_warn,		&gp_SectionUserBlackCounterOverMaxPrint_warn,	True},		// A0A3250400_部門／ユーザー別ブラックカウンター上限値オーバー(設定時)				A0A3250600_部門／ユーザー別ブラックカウンター上限値オーバー(プリント時).
	{&gp_TotalCountMaxValue_warn,		&gp_SectionUserTotalCounterOverMaxPrint_warn,	True},		// A0A3250700_部門／ユーザー別トータルカウンター上限値オーバー(設定時)				A0A3250900_部門／ユーザー別トータルカウンター上限値オーバー(プリント時).
	{&gp_TotalCountMaxValue_warn,		&gp_SectionUserColorCounterOverMaxPrint_warn,	True},		// A0A3250700 部門／ユーザー別トータルカウンター上限値オーバー(設定時)				A0A3250300 部門／ユーザー別カラーカウンター上限値オーバー(プリント時).
	{&gp_TotalCountMaxValue_warn,		&gp_SectionUserBlackCounterOverMaxPrint_warn,	True},		// A0A3250700 部門／ユーザー別トータルカウンター上限値オーバー(設定時)				A0A3250600 部門／ユーザー別ブラックカウンター上限値オーバー(プリント時).
	{&gp_CopyKitColorLimitSet_warn,		&gp_CopykitUpperLimitCom_warn,					True},		// A0A3251900 コピーキットカウンター／カラー上限設定値オーバー(設定時)				A0A3251700 コピーキットカウンタ／カラー＆ブラック上限設定値オーバー(設定時).
	{&gp_CopyKitColorLimitSet_warn,		&gp_CopykitUpperTotalLimit_warn,				True},		// A0A3251900 コピーキットカウンター／カラー上限設定値オーバー(設定時)				A0A3252000 コピーキットカウンター／カラー上限設定値オーバー(動作中).
	{&gp_CopyKitColorLimitSet_warn,		&gp_CopyKitUpperTotalLimitSet_warn,				True},		// A0A3251900 コピーキットカウンター／カラー上限設定値オーバー(設定時)				A0A3251800 コピーキットカウンタ／カラー＆ブラック上限設定値オーバー(動作中).
	{&gp_CopyKitBlackLimitSet_warn,		&gp_CopykitUpperTotalLimitBlack_warn,			True},		// A0A3252100 コピーキットカウンター／ブラック上限設定値オーバー(設定時)			A0A3252200 コピーキットカウンター／ブラック上限設定値オーバー(動作中).
	{&gp_CopyKitBlackLimitSet_warn,		&gp_CopyKitUpperTotalLimitSet_warn,				True},		// A0A3252100 コピーキットカウンター／ブラック上限設定値オーバー(設定時)			A0A3251800 コピーキットカウンタ／カラー＆ブラック上限設定値オーバー(動作中).
	{&gp_PunchScrapsFullService_warn,	&gp_ExtWarningPunchScrapsFullUser_warn,			True},		// A0A3040500_パンチくずフル(設定時・サービス交換)									A0A3040700_パンチくずフル(プリント時・サービス交換).
	{&gp_ColorUserBumonDisable_warn,	&gp_StampPrintProhibition_warn,					True},		// A0A3251000 部門／ユーザー別カラー出力禁止										A0A3251600 部門／ユーザー別スタンプ／ページ印字(カラー)出力禁止.
	{&gp_SaddleStapleEmptyS_warn,		&gp_StapleEmpty_warn,							True},		// A0A3251600 部門／ユーザー別スタンプ／ページ印字(カラー)出力禁止.
	{&gp_SetDocOnGlassSetting_warn,		&gp_ManuscriptSetDemand2_warn,					True},		// A0A3044900 原稿台原稿セット要求(設定時)											A0A3011510 原稿外消去／原稿セット要求2.
	{&gp_APSIncompatibleFormError_warn,	&gp_PunchDirectionErrorSetting_warn,			True},
	{&gp_PunchDirectionErrorSetting_warn,	&gp_APSIncompatibleFormError_warn,			True},
};
static	int	i_PopupNGGamenTableSize = sizeof( s_PopupNGGamenTable ) / sizeof( PNLC_PopupNGGamenTable );

PLayerControl*	PNLC_PopupListControl::p_PopupInstance_BAK;							// インスタンスのバックアップ

PNLC_PopupListControl::PNLC_PopupListControl( char option )
{
	m_ss_OpeningAnimPosX = 0;
	m_ss_OpeningAnimPosY = 0;
	m_ss_OpeningAnimSizeX = 0;
	m_ss_OpeningAnimSizeY = 0;
	p_mmbCallbackFunc = NULL;
	memset( pc_CapturePopupBaseTable, 0, sizeof( pc_CapturePopupBaseTable ) );
}
class CNTC_CounterIf;

//comment 2015/01/13 OPS1-MSE秋山 [Poseidon][CQ508764] CQ500984対応漏れ対応.
// 2重キーボード画面のインスタンス.
PLayerControl*	pc_KeyBoardInstance[] = {
	&gp_A034102000,
	&gp_A053902000,
	&gp_B034102000_EXT,
};

// ===============================================================================================
/**
 *	@brief		コンストラクタ.
 */
// ===============================================================================================
PNLC_PopupListControl::PNLC_PopupListControl()
	: m_ss_OpeningAnimPosX( 0 )
	, m_ss_OpeningAnimPosY( 0 )
	, m_ss_OpeningAnimSizeX( 0 )
	, m_ss_OpeningAnimSizeY( 0 )
{
	memset( p_PopupTable, NULL, PNLD_PopupMaxNum );
	memset( pc_CapturePopupBaseTable, 0, sizeof( pc_CapturePopupBaseTable ) );
	p_PopupTable[0] = &gp_SetupGamen;	// 初期のベース画面は起動画面としておく(警告検出時にNULLアクセスで落ちるため).

	setPopupListControlFunc( PM_TASKID_PANEL, ( PopupListControlCom* ( * )() )PNLC_PopupListControl::newInstance );
	p_mmbCallbackFunc = NULL;
// comment 2019/08/20 SCD2-Sky中野 [EagleH][OP_BTS-12365].
	memset( m_bl_CaptureCreated, 0, sizeof( m_bl_CaptureCreated ) );
}

// ===============================================================================================
/**
 *	@brief		デストラクタ.
 */
// ===============================================================================================
PNLC_PopupListControl::~PNLC_PopupListControl()
{
	int i;
	for( i = 0; i < PNLD_CAPTURE_POPUP_NUM_MAX; i++ ) {
		if( pc_CapturePopupBaseTable[i] != 0 ) {
			pc_CapturePopupBaseTable[i]->endGUI();
			delete pc_CapturePopupBaseTable[i];
			pc_CapturePopupBaseTable[i]	= 0;
		}
	}
}

// ===============================================================================================
/**
 *	@brief		自クラスのインスタンス取得.
 *	@return		自クラスのインスタンスのポインタ.
 */
// ===============================================================================================
PNLC_PopupListControl* PNLC_PopupListControl::newInstance()
{
	static PNLC_PopupListControl c_PopupListControl;
	return &c_PopupListControl;
}

// ===============================================================================================
//	画面キャプチャパーツ生成.
// ===============================================================================================
void PNLC_PopupListControl::createCaptureParts()
{
	int	i;
	for( i = 0; i < PNLD_CAPTURE_POPUP_NUM_MAX; i++ ) {
		pc_CapturePopupBaseTable[i]	= new PBaseWidget( c71_000_000_t1_X800_Y480, 0, 0, 0, TYPE_LayoutFree, False );
		pc_CapturePopupBaseTable[i]->startGUI( 0 );
		pc_CapturePopupBaseTable[i]->hide();
	}
}

// ===============================================================================================
/**
 *	@brief		ポップアップ管理リストにベース画面を登録する.
 *	@param[in]	p_NowGamen			ベース画面のインスタンス.
 */
// ===============================================================================================
void PNLC_PopupListControl::setBaseGamen( PBasicMessageProc* p_NowGamen )
{
	// 渡されたインスタンスが新規UI画面インスタンスがどうかをチェック.
	p_PopupTable[0] = p_NowGamen;	// ポップアップ管理リスト先頭要素には、常に表示中のベース画面のインスタンスが入る.
// comment 2020/11/25 Sky Kawanishi [LocalUI特ROM] SegF(スタックオーバーフロー)時ログ改善.
	LUIC_DbgTsk::newInstance()->setPopupList((void**)&(p_PopupTable[0]), c_PopupNum + 1);
}

// ===============================================================================================
/**
 *	@brief		ポップアップ管理リストからベース画面を取得する.
 */
// ===============================================================================================
PBasicMessageProc* PNLC_PopupListControl::getBaseGamen( void )
{
	return p_PopupTable[0];	// ポップアップ管理リスト先頭要素には、常に表示中のベース画面のインスタンスが入る.
}

// ===============================================================================================
/**
 *	@brief		ポップアップ起動要求.
 *	@param[in]	p_PopupInstance												起動するポップアップのインスタンス.
 *	@param[in]	p_SrcInstance										= NULL	起動元画面（ポップアップ）のインスタンス.
 *	@param[in]	( *p_CallbackFunc )( uchar, short, void*, uint32_t )	= NULL	コールバック関数ポインタ.
 *	@param[in]	ss_SrcPosX											= 0		起動元（主に起動ボタン）の位置.
 *	@param[in]	ss_SrcPosY											= 0		起動元（主に起動ボタン）の位置.
 *	@param[in]	ss_SrcSizeX											= 0		起動元（主に起動ボタン）のサイズ.
 *	@param[in]	ss_SrcSizeY											= 0		起動元（主に起動ボタン）のサイズ.
 *	@param[in]	bl_AnimLess											= False	起動アニメーション強制OFFフラグ（True：OFFする、False：OFFしない）.
 *	@param[in]	b_ForcedChange										= False	アニメーション中の画面・起動予約の画面を強制的に表示完了させるか否か（True：表示完了させる、False：表示完了させない）.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::addPopup(
    PLayerControl* p_PopupInstance,
    PBasicMessageProc* p_SrcInstance,							/* = NULL */
    void ( *p_CallbackFunc )( uchar, short, void*, uint32_t ),	/* = NULL */
    short ss_SrcPosX,											/* = 0 */
    short ss_SrcPosY,											/* = 0 */
    short ss_SrcSizeX,											/* = 0 */
    short ss_SrcSizeY,											/* = 0 */
    Bool  bl_AnimLess,											/* = False */
    Bool  b_ForcedChange										/* = False */
) {

	// ベース画面はポップアップしない.
	PBaseGamenCom* pc_Base = dynamic_cast<PBaseGamenCom*>( p_PopupInstance );
	if( NULL != pc_Base ) {
		PBasicMessageProc::setNextGamen( p_PopupInstance );			// sp_GamenWarningTableに記載されている警告画面ではない画面表示用.
		return False;
	}

	uchar uc_Valid = 0;
	PnlComGetSeqCustomValid( &uc_Valid );
	if( TYPD_SeqCustom_Function_VALID == uc_Valid ){			// 機能が有効の場合.
		if( PNLE_SeqCustom_Status_Act == PnlComSeqGetStatus() ){			// 認証シーケンス管理状態：動作中の場合.
			// 認証失敗ポップアップを表示する場合、カスタム対象画面のため既存の画面は表示させない.
			if( &gp_UserCertificate_warn == p_PopupInstance ){
				// 認証エラー失敗は認証カスタマイズ機能側で表示するためここでは表示を行わせない.
				return False;
			}
		}
	}

	// 起動するポップアップインスタンスのNULLチェック.
	if( NULL == p_PopupInstance ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}

	// 指定されたインスタンスが既に起動済みかどうかをチェック.
	for( register uchar i=0; i<=c_PopupNum; ++i ) {
		if( ( p_PopupInstance == p_PopupTable[i] )
		    && ( &gp_HomeTopMenu != p_PopupInstance ) ) {
			return False;															// 同一インスタンスが登録済みの場合、起動不可.
		}
	}
	// ポップアップしようとしている画面が起動中かどうかもチェックする.
	PPopupCom* pc_PopupCom = dynamic_cast< PPopupCom* >( p_PopupInstance );
	if( ( NULL != pc_PopupCom ) && ( False != pc_PopupCom->isOpened() ) && ( &gp_HomeTopMenu != p_PopupInstance ) ) { return False; }

// comment 2016.04.04 OPS1-Sky西原 [ZeusS_ZX0][CQ527395].
	// トップメニューの真上に、トップメニューはポップアップさせない.
	if( &gp_HomeTopMenu == p_PopupInstance
// Comment 2016/09/16  【HCLT-Krishna】 【IT5_3.3】【Accurev ID:1004076】【Warning Removal Activity】Type casting char to uchar due to array subscript data type mismatch.
//	&&  &gp_HomeTopMenu == p_PopupTable[ c_PopupNum ] ){
	&&  &gp_HomeTopMenu == p_PopupTable[( uchar ) c_PopupNum ] ){		
		return False;
	}

	//既にポップアップ予約済みの画面か？.
	if( True == isEnqueuedPopup( p_PopupInstance ) ) {
		// 予約されている場合は予約テーブルから削除し、改めてaddPopup()の処理をする.
		enqueuePopup( UC_POPUP_QUEUE_CLOSE, 0, 0, p_PopupInstance );
	}

	if( ( True == b_StapleEmptySetAvoid ) && ( p_PopupInstance == &gp_SaddleStapleEmptyS_warn ) )
	{
		sp_MessageWarningOkStatusTable[onlyPanel.us_NowMessageWarning] = PNLD_OkButtonPushed;
		return False;
	}

	// 指定されたインスタンスが起動不可でないかをチェック.
	for( uchar j = 0; j < i_PopupNGGamenTableSize; j++ ) {
		if( p_PopupInstance == s_PopupNGGamenTable[j].pc_PopupGamen ) {				// 起動しようとしている画面が起動不可画面テーブルにある.
			for( uchar k = 0; k <= c_PopupNum; ++k ) {
				if( p_PopupTable[k] == s_PopupNGGamenTable[j].pc_WokeGamen			// 起動を抑制する画面がすでに起動している.
				    || p_PopupInstance_BAK == s_PopupNGGamenTable[j].pc_WokeGamen ) {
					if( s_PopupNGGamenTable[j].b_DeleteFlag == True ) {
						sp_MessageWarningOkStatusTable[onlyPanel.us_NowMessageWarning] = PNLD_OkButtonPushed;		// 起動しようとしている警告をキャンセル.
					}
					return False;													// 起動不可.
				}
			}
		}
	}

	// 現在のポップアップ表示数をチェック.
	if( PNLD_PopupMaxNum <= ( char )( c_PopupNum + 1 ) ) {
		removeAllPopup( True );
		PnlComGetSystemStatus()->setFatal( 0xCC00 );
		CNTC_CounterIf::newInstance()->countTrouble( ( ushort )0xCC00 );				// インスタンス取得(CNTC_Counter) .
//comment 2017/07/05 SCD2-Sky肥後 [1026857] 新詳細コード対応.
		LAPC_BFW_System *pc_BFW_System = LAPC_BFW_System::newInstance();
		if (NULL != pc_BFW_System){
	   	 	pc_BFW_System ->registDetailCode( PNLD_DetailCode_FileIdentification_0x1900740022000000 | ( __LINE__ << 4 ) );
		}
		return False;																// 最大表示数以上の場合は、これ以上起動不可.
	}

	// ベース画面が旧UIの場合、アニメーションしない.
	if( NULL == dynamic_cast< PLayerControl* >( p_PopupTable[ 0 ] ) ) {
		if( False == isPopupLayer( &gp_HomeTopMenu ) ) {
			bl_AnimLess = True;
		}
	}

// comment 2015/09/01 OPS1-Sky稲井[特ROM][MZV-104_機種マージ]認証の無い環境下での操作者自身宛スキャンの実現要望対応. 
	if ( ( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW226 , 2 ) )	// DipSWNo.226 bit2 ON.
	  && ( True == onlyPanel.m_bl_ScanFaxProgramAmiFlag ) ) {												//ショートカットキー押下された.
		//ショートカットキー押下時は、アニメーションしないようにする.
		bl_AnimLess = True;
	}

	// 最上位画面がベース画面の場合にアニメーション中かどうかの状態を取得する.
	PBaseGamenCom* pc_NowBase = dynamic_cast< PBaseGamenCom* >( p_PopupTable[( uchar )c_PopupNum ] );
	Bool b_NowBaseAnimation = False;
	if( NULL != pc_NowBase ) {
		b_NowBaseAnimation = pc_NowBase->isNowAnimation();
	}
	// 最上位画面がポップアップ画面の場合にアニメーション中かどうかの状態を取得する.
	PPopupCom* pc_NowPopup = dynamic_cast< PPopupCom* >( p_PopupTable[( uchar )c_PopupNum ] );
	Bool b_NowPopupAnimation = False;
	if( NULL != pc_NowPopup ) {
		b_NowPopupAnimation = pc_NowPopup->isNowPopupAnimation();
	}
	// 強制的に表示完了させない場合で、尚且つアニメーション中の場合は起動予約する.
	if( ( False == b_ForcedChange ) && ( ( True == b_NowBaseAnimation ) || ( True == b_NowPopupAnimation ) ) ) {
		Bool bl_Reserve = enqueuePopup(
		                      UC_POPUP_QUEUE_OPEN,		// オープン要求.
		                      bl_AnimLess,				// 起動アニメーション強制OFFフラグ（True：OFFする、False：OFFしない）.
		                      False,						// 起動アニメーションをディレイさせるかどうか.
		                      p_PopupInstance,			// 起動するポップアップのインスタンス.
		                      p_SrcInstance,				// 起動元画面（ポップアップ）のインスタンス.
		                      p_CallbackFunc,				// コールバック関数ポインタ.
		                      ss_SrcPosX,					// 起動元（主に起動ボタン）の位置.
		                      ss_SrcPosY,					// 起動元（主に起動ボタン）の位置.
		                      ss_SrcSizeX,				// 起動元（主に起動ボタン）のサイズ.
		                      ss_SrcSizeY					// 起動元（主に起動ボタン）のサイズ.
		                  );
		return bl_Reserve;
	}

	// タッチ位置取得.
	Message message = NULL;
	uchar option = 0;
	Bool buzzer = False;
	if( ( 0 != us_RevTouchePanelX ) && ( 0 != us_RevTouchePanelY ) ) {
		// 遷移元画面の押下ボタン位置を元に戻す.
		p_PopupTable[( uchar )c_PopupNum ]->isReleased( us_RevTouchePanelX, us_RevTouchePanelY, message, option, buzzer );
	}

	// 強制的に表示完了させる場合は、アニメーション中の最上位画面の表示を完了させた後、起動・終了予約の画面の表示も完了させる.
	if( True == b_ForcedChange ) {
		// アニメーション中の最上位画面を表示完了させる.
		if( ( True == b_NowBaseAnimation ) || ( True == b_NowPopupAnimation ) ) {
			p_PopupTable[( uchar )c_PopupNum ]->completeGamenAnimationForced();
		}
		// 起動・終了予約の画面がある場合、画面表示・画面消去処理を完了させる.
		if( 0 < sc_QueueCount ) {
			for( uchar k = 0; PNLD_PopupQueueMax > k; ++k ) {
				// ポップアップの起動予約の場合は、画面表示を完了させる.
				if( UC_POPUP_QUEUE_OPEN == s_PopupQueue[ k ].uc_QueueType ) {
					addFixedPopup( s_PopupQueue[ k ].p_PopupInstance,	// 起動するポップアップのインスタンス.
					               s_PopupQueue[ k ].p_SrcInstance,		// 起動元画面（ポップアップ）のインスタンス.
					               s_PopupQueue[ k ].p_CallbackFunc,	// コールバック関数ポインタ.
					               s_PopupQueue[ k ].ss_SrcPosX,		// 起動元（主に起動ボタン）の位置.
					               s_PopupQueue[ k ].ss_SrcPosY,		// 起動元（主に起動ボタン）の位置.
					               s_PopupQueue[ k ].ss_SrcSizeX,		// 起動元（主に起動ボタン）のサイズ.
					               s_PopupQueue[ k ].ss_SrcSizeY,		// 起動元（主に起動ボタン）のサイズ.
					               True,								// アニメーションは無し.
					               True );								// 起動・終了予約は無視する.
					// ポップアップの終了予約の場合は、画面消去を完了させる.
				} else if( UC_POPUP_QUEUE_CLOSE == s_PopupQueue[ k ].uc_QueueType ) {
					if( 1 <= c_PopupNum ) {
						p_PopupTable[( uchar )c_PopupNum ]->closed( False, True, True );
					} else {
						LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
					}
				} else {
					break;
				}
				memset( &s_PopupQueue[ k ] , 0, sizeof( PNLS_PopupListControl_PopupQueue ) );
				--sc_QueueCount;
			}
		}
	}

	// 以降の処理は関数化する.
	addFixedPopup( p_PopupInstance, p_SrcInstance, p_CallbackFunc, ss_SrcPosX, ss_SrcPosY, ss_SrcSizeX, ss_SrcSizeY, bl_AnimLess, b_ForcedChange );

	return True;
}

// comment 2020/12/18 Sky中野 [IT6_2.1][OP_BTS-29896].
// ===============================================================================================
/**
 *	@brief		ポップアップ起動要求(拡張版).
 *	@param[in]	p_PopupInstance												起動するポップアップのインスタンス.
 *	@param[in]	p_SrcInstance										= NULL	起動元画面（ポップアップ）のインスタンス.
 *	@param[in]	( *p_CallbackFunc )( uchar, short, void*, uint32_t )	= NULL	コールバック関数ポインタ.
 *	@param[in]	ss_SrcPosX											= 0		起動元（主に起動ボタン）の位置.
 *	@param[in]	ss_SrcPosY											= 0		起動元（主に起動ボタン）の位置.
 *	@param[in]	ss_SrcSizeX											= 0		起動元（主に起動ボタン）のサイズ.
 *	@param[in]	ss_SrcSizeY											= 0		起動元（主に起動ボタン）のサイズ.
 *	@param[in]	bl_AnimLess											= False	起動アニメーション強制OFFフラグ（True：OFFする、False：OFFしない）.
 *	@param[in]	bl_SpecifyNextGamen									= False	ダミーベース遷移時に次画面を設定するか(True：設定する、False：設定しない).
 *	@param[in]	pc_NextGamenForDummy								= NULL	ダミーベース遷移時の次画面.
 *	@retval		True														起動成功.
 *	@retval		False														起動失敗.
 *	@note		優先アプリがブラウザまたはブラウザパネルの場合は.
 *				ベース画面をダミーベースに切換えてからポップアップを表示する.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::addPopupExp(
	PLayerControl* p_PopupInstance,
	PBasicMessageProc* p_SrcInstance,							/* = NULL */
	void ( *p_CallbackFunc )( uchar, short, void*, uint32_t ),	/* = NULL */
	short ss_SrcPosX,											/* = 0 */
	short ss_SrcPosY,											/* = 0 */
	short ss_SrcSizeX,											/* = 0 */
	short ss_SrcSizeY,											/* = 0 */
	Bool  bl_AnimLess,											/* = False */
	Bool  b_ForcedChange,										/* = False */
	Bool bl_SpecifyNextGamen,									/* = False */
	PBasicMessageProc* pc_NextGamenForDummy						/* = NULL */ )
{
	if( NULL == p_PopupInstance ) {
		return False;
	}
	
	Bool bl_ret = False;
	PM_TaskID e_TaskId = PM_TASKID_PANEL;
	if( 0 == PMGC_PanelManager::getPriority( &e_TaskId ) ) {
		// 最優先タスクが取得できない場合は、パネルを最優先タスクとして処理する.
		e_TaskId = PM_TASKID_PANEL;
	}
	
	if( ( PM_TASKID_BROWSER == e_TaskId )
	 || ( PM_TASKID_BRWPNL == e_TaskId ) ) {
		// 優先タスクがブラウザまたはブラウザパネルの場合、ダミーベース上にポップアップを表示する.
		PBasicMessageProc* pc_NextGamen = NULL;
		if( True == bl_SpecifyNextGamen ) {
			pc_NextGamen = pc_NextGamenForDummy;
		} else {
			pc_NextGamen = PBasicMessageProc::getNowGamen();
		}
		gp_PublicPopupDummyBase.setupOpenPopup( p_PopupInstance );
		gp_PublicPopupDummyBase.setupNextGamen( pc_NextGamen );
		gp_PublicPopupDummyBase.setMode( PNLE_PublicPopupDummyModeAct );
		PBasicMessageProc::setNextGamen( &gp_PublicPopupDummyBase );
		OpenPopupIndi();
		bl_ret = True;
	} else {
		// 上記以外の場合はnowGamen上にポップアップを表示する.
		bl_ret = addPopup( p_PopupInstance, p_SrcInstance, p_CallbackFunc, ss_SrcPosX, ss_SrcPosY, ss_SrcSizeX, ss_SrcSizeY, bl_AnimLess, b_ForcedChange );
	}
	
	return bl_ret;
}

// ===============================================================================================
/**
 *	@brief		ポップアップ起動予約.
 *	@param[in]	p_PopupInstance												起動する警告ポップアップのインスタンス.
 *	@param[in]	p_SrcInstance										= NULL	起動元画面（ポップアップ）のインスタンス.
 *	@param[in]	( *p_CallbackFunc )( uchar, short, void*, uint32_t )	= NULL	コールバック関数ポインタ.
 *	@param[in]	ss_SrcPosX											= 0		起動元（主に起動ボタン）の位置.
 *	@param[in]	ss_SrcPosY											= 0		起動元（主に起動ボタン）の位置.
 *	@param[in]	ss_SrcSizeX											= 0		起動元（主に起動ボタン）のサイズ.
 *	@param[in]	ss_SrcSizeY											= 0		起動元（主に起動ボタン）のサイズ.
 *	@retval		True														予約完了.
 *	@retval		False														予約失敗.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::addReservePopup(
    PLayerControl*	p_PopupInstance,
    PBasicMessageProc*	p_SrcInstance,
    void	( *p_CallbackFunc )( uchar, short, void*, uint32_t ),
    short			ss_SrcPosX,
    short			ss_SrcPosY,
    short			ss_SrcSizeX,
    short			ss_SrcSizeY
) {
	for( short i = 0; i < PNLD_ReservePopupMaxNum ; i++ ) {	// 既に予約されているかチェックする. // comment 209_3 2011/2/22 柳川.
		if( s_ReserveTable[ i ].p_PopupInstance == p_PopupInstance ) {
			return True;	// 既に予約されている場合、予約は正しくされているので、Trueを返す.
		}
	}

	// 起動予約数のチェック.
	if( PNLD_ReservePopupMaxNum > c_ReservedNum ) {
		PNLS_PopupListControl_OpeningInfo s_OpeningInfo = {
			p_PopupInstance,
			p_SrcInstance,
			p_CallbackFunc,
			ss_SrcPosX,
			ss_SrcPosY,
			ss_SrcSizeX,
			ss_SrcSizeY
		};
		// 最大予約数未満であれば、起動予約テーブルに予約を追加する.
		s_ReserveTable[( uchar )c_ReservedNum ] = s_OpeningInfo;
		++c_ReservedNum;
		return True;
	}
	return False;
}

// ===============================================================================================
/**
 *	@brief		起動予約されているポップアップを表示する.
 *	@retval		True				表示完了.
 *	@retval		False				起動予約なし.
 *	@note		予約分を全てopenした状態で表示.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::checkAndAddReservedPopup()
{
	PNLC_TopMenuManager* pc_TopMenuManager = PNLC_TopMenuManager::newInstance();
	if( NULL == pc_TopMenuManager ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}

	pc_TopMenuManager->initTopMenuFunctionPos();

	for( register uchar i=0; i<c_ReservedNum; ++i ) {
		// 起動するポップアップインスタンスのNULLチェック.
		if( NULL == s_ReserveTable[i].p_PopupInstance ) {
			return False;
		}

		// 現在のポップアップ表示数をチェック.
		if( PNLD_PopupMaxNum <= ( char )( c_PopupNum + 1 ) ) {
			removeAllPopup( True );
			PnlComGetSystemStatus()->setFatal( 0xCC00 );
			CNTC_CounterIf::newInstance()->countTrouble( ( ushort )0xCC00 );				// インスタンス取得(CNTC_Counter) .
//comment 2017/07/05 SCD2-Sky肥後 [1026857] 新詳細コード対応.
			LAPC_BFW_System *pc_BFW_System = LAPC_BFW_System::newInstance();
			if (NULL != pc_BFW_System){
		   	 	pc_BFW_System ->registDetailCode( PNLD_DetailCode_FileIdentification_0x1900740022000000 | ( __LINE__ << 4 ) );
			}
			return False;																// 最大表示数以上の場合は、これ以上起動不可.
		}

		// 指定されたインスタンスが既に起動済みかどうかをチェック.
		for( register uchar j=0; j<=c_PopupNum; ++j ) {
			if( s_ReserveTable[i].p_PopupInstance == p_PopupTable[j] ) {
				return False;															// 同一インスタンスが登録済みの場合、起動不可.
			}
		}

		if( 1 <= c_PopupNum && True == p_PopupTable[( uchar )c_PopupNum ]->isNowPopupAnimation() ) {
			return False;																// 最上位ポップアップがアニメーション中の場合は起動不可.
		}
// comment 2015/02/18 OPS1-Sky倉田 [MinervaSBK][UV-01457] ファクス/スキャン宛先検索・登録の操作性改善.
//		if( 0 == i ) {
//			const short ss_PartsNumber = p_PopupTable[ (uchar)c_PopupNum ]->getPartsNumber();		// 表示パーツ数取得.
//			PDisplayParts** p_PartsTable = p_PopupTable[ (uchar)c_PopupNum ]->getPartsTable();		// 表示パーツテーブル取得.
//
// 2015/02/16 OPS1嵯峨山 ZeusS CQ510041 ループ回数をパーツ数分に変更.
//			for( register short  ss_PartsCounter=0; ss_PartsCounter <= ss_PartsNumber; ss_PartsCounter++ ){
//			for( register short  ss_PartsCounter=0; ss_PartsCounter < ss_PartsNumber; ss_PartsCounter++ ){
//				PFunctionBadgeComponent* p_FunctionBadgeComponent = dynamic_cast< PFunctionBadgeComponent* >( p_PartsTable[ ss_PartsCounter ] );
//				// ベース画面に、機能バッチコンポーネントがあるか判定.
//				if( NULL != p_FunctionBadgeComponent ) {
//					// 0番目のポップアップに割り当てられている機能ID（NVD）から、基本設定か応用設定かを取得する.
//					if( False == pc_TopMenuManager->isBasicFunction( s_ReserveTable[i].p_PopupInstance->getFunctionID() ) ) {
//						++c_PopupNum;
//						if( ( char )( sizeof( p_PopupTable )/sizeof( p_PopupTable[0] ) ) <= c_PopupNum ) {
//							return False;
//						}
//						p_PopupTable[( uchar )c_PopupNum ] = &gp_StubPopup_002;								// 応用設定であれば、応用機能ウィンドウを開く.
//						PNLC_StackManJobSpec::newInstance()->saveJobSpec( c_PopupNum );						// ジョブスペックを保存(ポップアップ中止時に設定を戻すため)
//
//						p_PopupTable[( uchar )c_PopupNum ]->setLayerNumber( c_PopupNum );					// 階層番号を設定.
//						p_PopupTable[( uchar )c_PopupNum ]->openedByChangeGamen( NULL, 704, 432, 88, 32 );	// 応用ウィンドウの起動処理.
//					}
//				}
//			}
//		}

		++c_PopupNum;

		p_PopupTable[( uchar )c_PopupNum ] = s_ReserveTable[i].p_PopupInstance;

		PNLC_StackManJobSpec::newInstance()->saveJobSpec( c_PopupNum );						// ジョブスペックを保存(ポップアップ中止時に設定を戻すため)

		p_PopupTable[( uchar )c_PopupNum ]->setLayerNumber( c_PopupNum );						// 追加したポップアップに、確定した階層情報を設定.

		short ss_offsetX = 0, ss_offsetY = 0;
		if( 1 < c_PopupNum ) {
			p_PopupTable[ c_PopupNum - 1 ]->getPosition( &ss_offsetX, &ss_offsetY );	// ひとつ下の階層のポップアップ表示位置をオフセットとして取得.
		}

		p_SrcGamen = s_ReserveTable[i].p_SrcInstance;	// 起動元画面インスタンスを保持（起動完了通知呼び出し用）. // comment 209_3 2011/2/17 柳川.

		// 管理バッファにコピー（移動）したポップアップの起動処理.
		p_PopupTable[( uchar )c_PopupNum ]->openedByChangeGamen(
		    s_ReserveTable[i].p_CallbackFunc,
		    static_cast<short>( s_ReserveTable[i].ss_SrcPosX + ss_offsetX ),
		    static_cast<short>( s_ReserveTable[i].ss_SrcPosY + ss_offsetY ),
		    s_ReserveTable[i].ss_SrcSizeX,
		    s_ReserveTable[i].ss_SrcSizeY
		);
	}

	c_ReservedNum = 0;

	memset( s_ReserveTable, NULL, sizeof( s_ReserveTable ) );

// comment 2020/11/25 Sky Kawanishi [LocalUI特ROM] SegF(スタックオーバーフロー)時ログ改善.
	LUIC_DbgTsk::newInstance()->setPopupList((void**)&(p_PopupTable[0]), c_PopupNum + 1);
	// 画面変更によるログの収集.
	PNLC_OperationLog::newInstance()->setLogChangePanel( p_PopupTable[( uchar )c_PopupNum ] );

	return True;
}

// ===============================================================================================
/**
 *	@brief		オープン/クローズ要求待ち.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::enqueuePopup(
    UC_POPUP_QUEUE	uc_QueueType,
    Bool			bl_AnimLess,
    Bool			bl_Delay,
    PLayerControl*	p_PopupInstance,
    PBasicMessageProc*	p_SrcInstance,
    void	( *p_CallbackFunc )( uchar, short, void*, uint32_t ),
    short			ss_SrcPosX,
    short			ss_SrcPosY,
    short			ss_SrcSizeX,
    short			ss_SrcSizeY
)
{
	uchar m, n;						// ループカウンタ.

	// ベース画面が旧UIの場合、アニメーションしない.
	if( NULL == dynamic_cast< PLayerControl* >( p_PopupTable[ 0 ] ) ) {
		bl_AnimLess = True;
	}

	switch( uc_QueueType ) {
		case UC_POPUP_QUEUE_OPEN:
			// 指定された画面が既に要求済みかチェック.
			for( m = 0; m < sc_QueueCount; ++m ) {
				if( p_PopupInstance == s_PopupQueue[m].p_PopupInstance ) {
					return False;															// 既に要求済みの場合は待ちにしない.
				}
			}
			break;
		case UC_POPUP_QUEUE_CLOSE:
			// 画面指定している場合はその画面を要求待ちから削除(該当しなければ何もしない).
			if( NULL != p_PopupInstance ) {
				for( m = 0; m < sc_QueueCount; ++m ) {
					// 該当すればオープン予約から削除しておく.
					if( ( UC_POPUP_QUEUE_OPEN == s_PopupQueue[m].uc_QueueType ) && ( p_PopupInstance == s_PopupQueue[m].p_PopupInstance ) ) {
						for( n = m; n < sc_QueueCount; n++ ) {
							if( n == sc_QueueCount - 1 ) {													// 削除.
								memset( &s_PopupQueue[n], 0, sizeof( PNLS_PopupListControl_PopupQueue ) );
							} else {
								s_PopupQueue[n] = s_PopupQueue[n+1];
							}
						}
						sc_QueueCount--;
					}
				}
				return False;
			} else {
				// 画面指定がない場合はトップのオープン要求待ちを削除(該当しなければクローズの要求待ちとして登録)
				if( ( 0 < sc_QueueCount ) && ( UC_POPUP_QUEUE_OPEN == s_PopupQueue[0].uc_QueueType ) ) {
					memset( &s_PopupQueue[sc_QueueCount - 1], 0, sizeof( PNLS_PopupListControl_PopupQueue ) );
					sc_QueueCount--;
					return False;
				}
			}
			break;
		default:
			break;
	}

	// 起動予約数のチェック.
	if( PNLD_PopupQueueMax <= sc_QueueCount ) {
		return False;
	}

	// 最大予約数未満であれば、要求待ち.
	PNLS_PopupListControl_PopupQueue s_OneQueue = {
		uc_QueueType,
		bl_AnimLess,
		bl_Delay,
		p_PopupInstance,
		p_SrcInstance,
		p_CallbackFunc,
		ss_SrcPosX,
		ss_SrcPosY,
		ss_SrcSizeX,
		ss_SrcSizeY,
	};
	s_PopupQueue[( uchar )sc_QueueCount ] = s_OneQueue;
	++sc_QueueCount;

	return True;
}

// ===============================================================================================
/**
 *	@brief		予約されているポップアップ動作を実行(連続ポップアップ動作).
 */
// ===============================================================================================
Bool PNLC_PopupListControl::actReservedPopup()
{
	// 予約が無ければ処理しない.
	if( 0 >= sc_QueueCount || UC_POPUP_QUEUE_NON == s_PopupQueue[0].uc_QueueType ) {
		return False;
	}

	// 予約画面を取り出す
	// (アニメーション無し画面の場合、addPopupから呼び出されるので先に予約領域の整理をしておく).
	PNLS_PopupListControl_PopupQueue s_TempPopupInfo = s_PopupQueue[0];
	for( uchar i = 0; i < sc_QueueCount; i++ ) {
		if( i == sc_QueueCount - 1 ) {				// 最後の予約領域は初期化.
			memset( &s_PopupQueue[i], 0, sizeof( PNLS_PopupListControl_PopupQueue ) );
		} else {
			s_PopupQueue[i] = s_PopupQueue[i+1];
		}
	}
	sc_QueueCount--;

	// 予約画面の起動(オープン/クローズ).
	Bool bl_result = False;									// 要求待ちを実行できたか？.
	switch( s_TempPopupInfo.uc_QueueType ) {
		case UC_POPUP_QUEUE_OPEN:								// オープンの場合.
		{
			// 起動するポップアップインスタンスのNULLチェック.
			if( NULL == s_TempPopupInfo.p_PopupInstance ) {
				return False;
			}
			// ポップアップ要求.
			bl_result = addPopup(
			                s_TempPopupInfo.p_PopupInstance,			// 起動するポップアップのインスタンス.
			                s_TempPopupInfo.p_SrcInstance,				// 起動元画面（ポップアップ）のインスタンス.
			                s_TempPopupInfo.p_CallbackFunc,				// コールバック関数ポインタ.
			                s_TempPopupInfo.ss_SrcPosX,					// 起動元（主に起動ボタン）の位置.
			                s_TempPopupInfo.ss_SrcPosY,					// 起動元（主に起動ボタン）の位置.
			                s_TempPopupInfo.ss_SrcSizeX,				// 起動元（主に起動ボタン）のサイズ.
			                s_TempPopupInfo.ss_SrcSizeY,				// 起動元（主に起動ボタン）のサイズ.
			                s_TempPopupInfo.bl_AnimLess					// 起動アニメーション強制OFFフラグ（True：OFFする、False：OFFしない）.
			            );
		}
		break;
		case UC_POPUP_QUEUE_CLOSE:								// クローズの場合.
		{
			// クローズ要求.
			bl_result = closeTop( s_TempPopupInfo.bl_Delay, s_TempPopupInfo.bl_AnimLess );
		}
		break;
		default:
			break;
	}

	return bl_result;
}

// ===============================================================================================
/**
 *	@brief		オープン/クローズ要求待ちチェック.
 */
// ===============================================================================================
char PNLC_PopupListControl::getPopupQueueCnt()
{
	return sc_QueueCount;
}
// ===============================================================================================
/**
 *	@brief		オープン/クローズ要求待ち情報を初期化.
 */
// ===============================================================================================
void PNLC_PopupListControl::clearPopupQueue()
{
	memset( s_PopupQueue, 0, sizeof( s_PopupQueue ) );
	sc_QueueCount = 0;
	// ポップアップ要求待ち初期化された時点で初期化しておく.
	b_mRemoveAllPopup = False;
// comment 2022/09/22 Sky東島 [IT6_2.3][制御構成変更_No.51][サービスレスメンテナンス].
	m_bl_CloseSpecifiedPopup = False;
}
// ===============================================================================================.
/**
 *	[概要]		最上位のポップアップ終了要求.
 *	[引数]		なし.
 *	[戻り値]	True:終了させた. False:終了失敗.
 *	[履歴]
 *	2013/08/12 OPS1-Sky嵯峨山 [ZM3rd][CQ382114] レイヤー番号が「0」の場合、処理を行わないように修正.
 *		2013/03/21 OPS1yusaku【操作性改善FUM4】warning除去対応.
 *		2013/02/13 OPS1 iyanagi [操作性改善FUM4] 最上位となるレイヤーのNULLチェックを追加 CT:No54対策.
 *		2012/10/05 OPS1 yusaku [操作性改善FUM4]リソース削減 管理No.173,174,175,176. 背景VRAMバッファ領域1層化対応.
 *		 背景仮想VRAMバッファ領域を10層→1層分へ変更したため、レイヤー終了時に次に最上位となるレイヤーの再描画処理を追加.
 *		2011/09/26 OAP-NUI Sky石賀 [CQ355137]. IGCnoActive通知の条件修正.
 *		XXXX/XX/XX CQ.350923と同様の対応が必要となった為、実装を統合する.
 *		XXXX/XX/XX [CQ.350923]読込み停止中にIGCWait画面を経由するとメニューへ遷移できない.
 *		2011/03/28【MinverZeus制御構成変更209（管理No.892_11_2） : OSW1竹菴: アニメーションの強制終了】.
 *		 引数b_ActiveNotice追加.
 *		2011/02/15【MinverZeus制御構成変更210（管理No.893_1） : OSW1竹菴: : 複数レイヤー表示の上限不足対応】.
 *		 レイヤー復帰処理および画面再描画処理追加.
 *		XXXX/XX/XX 制御構成変更209 管理No.892_1,2 第3世代→ポップアップ警告表示 酒井健.
 *		2010/11/10 p_PopupInstance_BAK ポップアップのバックアップを消去 伊丹Sky(八木) CQ334380.
 *		2010/10/15 DJG2 伊丹Sky_Sagayama [FUM5][パフォーマンス] .
 *		2010/07/24 Sky_高橋(直) CQ330962対応. b_mSetChangePopup追加.
 *		XXXX/XX/XX ColorUIFUMv5 Sky津村 UV-00639 送信操作ログ対応.
*/
// ===============================================================================================.
Bool PNLC_PopupListControl::removePopup( void )
{
	if( 1 > c_PopupNum ) {
		return False;									// ポップアップが起動していない場合は、False返却.
	}

	PNLC_StackManJobSpec::newInstance()->deleteJobSpec( c_PopupNum );	// ジョブスペックを削除(ポップアップ中止時に設定を戻すため)

	PTextField::freeLayerDynamicText( c_PopupNum );			// テキストフィールド動的領域解放

	p_PopupTable[( uchar )c_PopupNum ]->setLayerNumber( 0 );		// 消去するポップアップの階層番号を0で初期化する.

	p_PopupTable[( uchar )c_PopupNum ] = NULL;					// ポップアップ管理リストの最後尾を削除する.

	--c_PopupNum;										// ポップアップ表示数をデクリメント.

	// 復帰させる必要があるレイヤーが存在する場合は、レイヤーの復帰要求および画面パーツの再描画を行う.
	if( 1 <= c_PopupNum ) {
		// ポップアップ全クローズ時および、最上位ポップアップの差換え時は画面パーツの再描画を行わない.
		if( ( False == b_mRemoveAllPopup ) && ( False == b_mSetChangePopup ) ) {
			p_PopupTable[( uchar )c_PopupNum ]->redraw();												// 画面パーツの再表示.
		}
	}
	// 次に最上位となるレイヤーの再描画処理を行う.
	// ただし、ポップアップ全クローズ時は最下層以外再描画しない.
	if( 0 == c_PopupNum || ( False == b_mRemoveAllPopup ) ) {			// 次に最上位となるレイヤーが最下層もしくは、ポップアップ全クローズでないとき.
		if( NULL != p_PopupTable[( uchar )c_PopupNum ] ) {				// NULLチェック追加 2013/02/13 OPS1 iyanagi [操作性改善FUM4] CT:No54対策.
			p_PopupTable[( uchar )c_PopupNum ]->redraw();				// 次に最上位となるレイヤーのパーツを再表示.
		}
	}

// comment 2020/11/25 Sky Kawanishi [LocalUI特ROM] SegF(スタックオーバーフロー)時ログ改善.
	LUIC_DbgTsk::newInstance()->setPopupList((void**)&(p_PopupTable[0]), c_PopupNum + 1);

	return True;
}

// ===============================================================================================
/**
 *	@brief		上位ポップアップ削除後の処理.
 *	@param[in]	bl_ActLess = True	画面更新 OFF フラグ (True; 更新しない、False: 更新する)
 *	@retval		True				終了させた.
 *	@retval		False				終了失敗.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::completedRemovePopup( Bool b_ActiveNotice )
{
	// 特定ポップアップ画面の場合は、procActivateを通知しない.
	if( ( ( &gp_IGCWait_popup == p_PopupTable[( uchar )c_PopupNum ] || &gp_IGCClearWait_popup == p_PopupTable[( uchar )c_PopupNum ] )
	      && ( PCGE_IGCClearPopupDispFlgAdd == pc_ManageIGCWaitDisplay.getIGCClearPopupDispFlg() ) )			// IGCWait画面 [CQ.350923].
	    || ( &gp_BrowserAccessSetting_act == p_PopupTable[( uchar )c_PopupNum ] )										// A072001700 Webブラウザー設定アクセス中.
	  ) {
		b_ActiveNotice = False;
	}

	// 第3世代→ポップアップ警告表示
	p_PopupInstance_BAK = dynamic_cast< PLayerControl* >( p_PopupTable[( uchar )c_PopupNum ] );
	if( ( True == b_ActiveNotice ) && ( False == b_mRemoveAllPopup && False == b_mSetChangePopup ) ) {
// comment 2022/02/18 Sky東島 [IT6_2.2MR2]パネルログ強化.
		PM_TaskID priTask;
		PMGC_PanelManager::getPriority( &priTask );
// comment 2023/04/06 Sky東島 [IT6_2.3]ログ改善対応.
//		ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[GamenActive][PriTask:%d] %s", priTask, typeid( *(p_PopupTable[( uchar )c_PopupNum ]) ).name(), 0, 0, 0, 0 );
		ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[ChangeGamen][GamenActive][PriTask:%d] %s", priTask, typeid( *(p_PopupTable[( uchar )c_PopupNum ]) ).name(), 0, 0, 0, 0 );
// comment 2021/03/17 Sky石原[IT6_2.2][OP_BTS-30905]フッターエリアキーが非活性なのに押下できる.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
		PnlComFooterPartsBtnActiveChange(True, True, True);
#endif //#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) ).
		p_PopupTable[( uchar )c_PopupNum ]->procActivate();		// アクティブ状態の通知.
	}
	b_mRemoveAllPopup = False;
	b_mSetChangePopup = False;
	// 画面変更によるログの収集.
	PNLC_OperationLog::newInstance()->setLogChangePanel( p_PopupTable[( uchar )c_PopupNum ] );
	return True;
}

// ===============================================================================================
/**
 *	@brief		表示中の全ポップアップ終了要求.
 *	@param[in]	bl_ActLess = True	画面更新 OFF フラグ (True; 更新しない、False: 更新する)
 *	@retval		True				終了させた.
 *	@retval		False				終了失敗.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::removeAllPopup( Bool bl_ActLess /* = True */ )
{
	b_mRemoveAllPopup = False;		// 初期化.

	// 要求待ちを初期化.
	clearPopupQueue();

	DelayedIGCWaitDisplayTimer::destroy();

	if( 1 > c_PopupNum ) {
		return False;								// ポップアップが起動していない場合は、False返却.
	}

	for( register char i=c_PopupNum; i>0; --i ) {
		b_mRemoveAllPopup = bl_ActLess;
// comment 2014/01/16 OPS1-Sky浜野 [Poseidon][CQ508992] NULLチェック追加.
//		p_PopupTable[(uchar)i]->closed( False, True );		// ベース画面は除いて、上の階層から順番に、直ちに終了を要求する.
		if ( NULL != p_PopupTable[(uchar)i] ) {
			p_PopupTable[(uchar)i]->closed( False, True );		// ベース画面は除いて、上の階層から順番に、直ちに終了を要求する.
		}
	}
	// ポップアップ終了(removePopup)まで呼ばれなかった場合にb_mRemoveAllPopupが落ちずにprocActivateが呼ばれないケースがある.
	// ポップアップ要求待ちに入った場合は要求待ちの実行時と要求待ちの初期化(clearPopupQueue)で初期化を行う.
	if( 0 != getPopupQueueCnt() ) {
		b_mRemoveAllPopup = False;
	}

// トップメニュー起動予約を削除(多重起動情報).
	PNLC_WarningManager::newInstance()->setTopMenuMitipleStatus( False );
// Comment 2017/05/08 HCL-Hanuman [IT5_4.0] [AR1021452] Show/Hide of slide menu key for Popup screens moved to PPopupCom::opened() function.
	if(&gp_ActionInitialSetup == PBasicMessageProc::nowGamen ) {
		PNLC_SlideMenuControlLib::newInstance()->judgeHide();
	}
	return True;
}

// ===============================================================================================
/**
 *	@brief		表示中の全ポップアップ終了要求(ただし途中にプリント設定変更メニュー画面、またはスライド画面があれば止める).
 *	@retval		True				終了させた.
 *	@retval		False				終了失敗.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::removeAllPopup_ToPrintSetupChangeMenu()
{
	// 要求待ちを初期化.
	clearPopupQueue();

	if( 1 > c_PopupNum ) {
		return False;								// ポップアップが起動していない場合は、False返却.
	}

	for( register char i=c_PopupNum; i>0; --i ) {
		// プリント設定変更メニュー画面、またはスライド画面があれば途中でクローズを止める.
		if( ( isPrintSetupChangeMenu( p_PopupTable[( uchar )i] ) )	// プリント設定変更メニュー.
		    ||( isSlideInGamen( p_PopupTable[( uchar )i] ) ) ) {		// スライド画面.
			break;	// ポップアップクローズ終了.
		}
		p_PopupTable[( uchar )i]->closed( False, True );		// ベース画面は除いて、上の階層から順番に、直ちに終了を要求する.
	}

	return True;
}

// ===============================================================================================
/**
 *	@brief		最上位のポップアップに対して終了処理を開始させる.
 *	@param[in]	bl_Delay			終了アニメーションをディレイさせるかどうか.
 *	@param[in]	bl_AnimLess	= False	終了アニメーション強制OFFフラグ（True：OFFする、False：OFFしない）.
 *	@retval		True				成功.
 *	@retval		False				失敗.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::closeTop( Bool bl_Delay, Bool bl_AnimLess /* = False */ )
{
	if( 1 > c_PopupNum ) {
		return False;										// ポップアップが起動していない場合は、False返却.
	}
	// ベース画面が旧UIの場合、アニメーションしない.
	if( NULL == dynamic_cast< PLayerControl* >( p_PopupTable[ 0 ] ) ) {
		if( False == isPopupLayer( &gp_HomeTopMenu ) ) {
			bl_AnimLess = True;
		}
	}
	PPopupCom* cp_PopupGamen = dynamic_cast< PPopupCom* >( p_PopupTable[ c_PopupNum - 1 ] );
	if( ( m_p_closeLowerPlusGamen == p_PopupTable[( uchar )c_PopupNum ] )									// 一つ下のポップアップも同時に閉じる画面.
	    && ( NULL != cp_PopupGamen ) ) {																		// 一つ下の画面がポップアップ画面.
		closeSpecifiedPopup( cp_PopupGamen );																// 一つ下の画面から上の画面を閉じる.
	} else {
		p_PopupTable[( uchar )c_PopupNum ]->closed( bl_Delay, bl_AnimLess );	// 最上位のポップアップに対して終了要求する.
	}

	return True;
}

// ===============================================================================================
/**
 *	@brief		指定ポップアップと、それより上位にあるポップアップを全て即時クローズする.
 *	@param[in]	p_PopupInstance		閉じるポップアップのインスタンス.
 *	@retval		True				成功.
 *	@retval		False				失敗.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::closeSpecifiedPopup( PLayerControl* p_PopupInstance )
{
	// 要求待ちを初期化.
	clearPopupQueue();
//	Comment 2018/09/01 HCLT_Prabakaran [IT6_Eagle][AR1053915] The popup table index max check added.
//	if( 1 > c_PopupNum || NULL == p_PopupInstance ) {
	if( 1 > c_PopupNum || NULL == p_PopupInstance || PNLD_PopupMaxNum <= c_PopupNum) {
		return False;										// パラメータ不正、またはポップアップが起動していない場合は、False返却.
	}

	char c_SpecifiedIndex = 0;								// 検索用インデックス初期化.

	for( register char i=1; i<=c_PopupNum; ++i ) {
		if( p_PopupInstance == p_PopupTable[( uchar )i] ) {			// 指定されたインスタンスとリストを比較.
			c_SpecifiedIndex = i;							// 一致したときは、インデックスを保持.
			break;
		}
	}

	if( 0 != c_SpecifiedIndex ) {							// 指定されたインスタンスが表示中かどうか.
// comment 2022/09/22 Sky東島 [IT6_2.3][制御構成変更_No.51][サービスレスメンテナンス].
		m_bl_CloseSpecifiedPopup = True;
		for( register char i=c_PopupNum; i>=c_SpecifiedIndex; --i ) {
// comment 2018/08/24 SCD2-4G東島 [Eagle][1047602].
			if( NULL == p_PopupTable[( uchar )i] ){
				continue;
			}
			p_PopupTable[( uchar )i]->closed( False, True );			// 最上位から順番にクローズしていく.
		}
// comment 2022/09/22 Sky東島 [IT6_2.3][制御構成変更_No.51][サービスレスメンテナンス].
		m_bl_CloseSpecifiedPopup = False;
		return True;
	} else {
		return False;										// 指定されたインスタンスが表示中でない場合.
	}
}

// ===============================================================================================
/**
 *	@brief		指定ポップアップを含まない、それより上位にあるポップアップを全て即時クローズする.
 *	@param[in]	p_PopupInstance		指定ポップアップのインスタンス(このポップアップは閉じない).
 *	@retval		True				成功.
 *	@retval		False				失敗.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::closeSpecifiedPopupHighLayer( PLayerControl* p_PopupInstance, Bool bl_AnimLess /* = False */ )
{
	// 要求待ちを初期化.
	clearPopupQueue();

	Bool b_FindFlg = False;		// 見つかったフラグ.

	if( 1 > c_PopupNum || NULL == p_PopupInstance ) {
		return False;										// パラメータ不正、またはポップアップが起動していない場合は、False返却.
	}

	// ベース画面が旧UIの場合、アニメーションしない.
	if( NULL == dynamic_cast< PLayerControl* >( p_PopupTable[ 0 ] ) ) {
		bl_AnimLess = True;
	}

// comment 2022/09/22 Sky東島 [IT6_2.3][制御構成変更_No.51][サービスレスメンテナンス].
	m_bl_CloseSpecifiedPopup = True;
	for( register char i=c_PopupNum; i>0; --i ) {
		// 指定されたポップアップかあれば、ループを抜けて処理終了.
		if( p_PopupInstance == p_PopupTable[( uchar )i] ) {
			b_FindFlg = True;
			break;	// ポップアップクローズ終了.
		}
		if( NULL != p_PopupTable[( uchar )i] ) {
			p_PopupTable[( uchar )i]->closed( False, bl_AnimLess );	// ベース画面は除いて、上の階層から順番に、直ちに終了を要求する.
		}
	}
// comment 2022/09/22 Sky東島 [IT6_2.3][制御構成変更_No.51][サービスレスメンテナンス].
	m_bl_CloseSpecifiedPopup = False;
	return b_FindFlg;
}

// ===============================================================================================
/**
 *	@brief		ポップアップの起動処理が完了したことを起動元に通知する.
 *	@param[in]	option = 0			オプション値.
 */
// ===============================================================================================
void PNLC_PopupListControl::notifyPopupOpened( uchar option /* = 0 */ )
{
	// 起動元画面インスタンスが登録されているか.
	if( NULL != p_SrcGamen ) {
		p_SrcGamen->procPopupOpened( option );	// 登録されているインスタンスの起動完了通知呼び出し.
	}
}

// ===============================================================================================
/**
 *	@brief		起動元画面の終了処理を呼び出すための通知.
 */
// ===============================================================================================
void PNLC_PopupListControl::notifyendGamen()
{
	if( NULL != p_SrcGamen ) {	// 起動元画面インスタンスが登録されている場合.
		p_SrcGamen->notifyendGamen();	// endGamen()を呼び出す.
	}
}

// ===============================================================================================
/**
 *	@brief		ポップアップが1つ以上起動しているかどうかを取得.
 *	@retval		True				起動している.
 *	@retval		False				起動していない.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::isPopupOpen()
{
	if( 1 <= c_PopupNum ) {
		return True;	// ポップアップが1つ以上起動していればTrue返却.
	} else {
		return False;	// 未起動ならFalse返却.
	}
}

// ===============================================================================================
/**
 *	@brief		表示中の全ポップアップに対してsetupGamen()をコールする.
 *	@param[in]	option			オプション値.
 */
// ===============================================================================================
void PNLC_PopupListControl::setupGamen( short option )
{
	if( 1 > c_PopupNum ) {
		return;										// ポップアップが起動していない場合は、処理終了.
	}

	for( register uchar i=1; i<=c_PopupNum; ++i ) {
		PLayerControl* p_PopupGamen = dynamic_cast< PLayerControl* >( p_PopupTable[i] );
		if( NULL != p_PopupGamen ) {
			// ベース画面とアニメーション中のポップアップを除いて、下の階層から順番にsetupGamen_IF()を呼び出す.
			if( True != p_PopupGamen->isNowPopupAnimation() ) {
				p_PopupGamen->setupGamen_IF( option );
			}
		}
	}
}

// ===============================================================================================
/**
 *	@brief		最上位のポップアップにメッセージをディスパッチする.
 *	@param[in]	message			メッセージ.
 *	@param[in]	option			オプション値.
 *	@param[in]	type			種別.
 *	@param[in]	sl_option		.
 *	@param[in]	st_NtfEventData	NTFイベントデータのアドレス.
 */
// ===============================================================================================
void PNLC_PopupListControl::messageDispathPopup( Message message, uchar option, MessageType type , int32_t sl_option, void* st_NtfEventData )
{
	if( 1 > c_PopupNum ) {
		return;																	// ポップアップが起動していない場合は、処理終了.
	}

	PLayerControl* p_PopupGamen = dynamic_cast< PLayerControl* >( p_PopupTable[( uchar )c_PopupNum ] );
	if( NULL != p_PopupGamen ) {
		p_PopupGamen->messageDispathPopup( message, option, type, sl_option, st_NtfEventData );	// ポップアップ起動中は、ポップアップのメッセージディスパッチを呼び出し.
	}
}

// ===============================================================================================
/**
 *	@brief		最上位のポップアップに対して押下判定を行う.
 *	@param[in]	x				x座標.
 *	@param[in]	y				y座標.
 *	@param[out]	message			メッセージ.
 *	@param[out]	option			オプション値.
 *	@param[out]	type			種別.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::isPushedPopup( short x, short y, Message& message, uchar& option, Bool& buzzer )
{
	if( 1 > c_PopupNum ) {
		return False;																	// ポップアップが起動していない場合は、False返却.
	}
	// ポップアップ起動中でアニメーション中か?
	if( ( 0 < c_PopupNum ) && ( True == p_PopupTable[( uchar )c_PopupNum ]->isNowPopupAnimation() ) )
	{
		// ポップアップ表示タイミングにより押下イベントが来る可能性があるため拒否音は鳴動しない.
		return False;																	// ポップアップが起動していない場合は、False返却.
	}
	PLayerControl* p_PopupGamen = dynamic_cast< PLayerControl* >( p_PopupTable[( uchar )c_PopupNum ] );
	if( NULL != p_PopupGamen ) {
		return p_PopupGamen->isPushedPopup( x, y, message, option, buzzer );	// ポップアップ起動中は、ポップアップの押下判定を呼び出し.
	} else {
		return False;
	}
}

// ===============================================================================================
/**
 *	@brief		最上位のポップアップに対してドラッグ判定を行う.
 *	@param[in]	x				x座標.
 *	@param[in]	y				y座標.
 *	@param[out]	message			メッセージ.
 *	@param[out]	option			オプション値.
 *	@param[out]	type			種別.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::isDragPopup( short x, short y, Message& message, uchar& option, Bool& buzzer )
{
	if( 1 > c_PopupNum ) {
		return False;																	// ポップアップが起動していない場合は、False返却.
	}
	PLayerControl* p_PopupGamen = dynamic_cast< PLayerControl* >( p_PopupTable[( uchar )c_PopupNum ] );
	if( NULL != p_PopupGamen ) {
		return p_PopupGamen->isDragPopup( x, y, message, option, buzzer );	// ポップアップ起動中は、ポップアップのドラッグ判定を呼び出し.
	} else {
		return False;
	}
}

// ===============================================================================================
/**
 *	@brief		最上位のポップアップに対してリリース判定を行う.
 *	@param[in]	x				x座標.
 *	@param[in]	y				y座標.
 *	@param[out]	message			メッセージ.
 *	@param[out]	option			オプション値.
 *	@param[out]	type			種別.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::isReleasedPopup( short x, short y, Message& message, uchar& option, Bool& buzzer )
{
	if( 1 > c_PopupNum ) {
		return False;																	// ポップアップが起動していない場合は、False返却.
	}
	PLayerControl* p_PopupGamen = dynamic_cast< PLayerControl* >( p_PopupTable[( uchar )c_PopupNum ] );
	if( NULL != p_PopupGamen ) {
		return p_PopupGamen->isReleasedPopup( x, y, message, option, buzzer );	// ポップアップ起動中は、ポップアップのリリース判定を呼び出し.
	} else {
		return False;
	}
}

// ===============================================================================================
/**
 *	@brief		組み合わせ禁止ポップアップのひとつ下の階層に、先設定変更許可を通知する.
 *	@retval		True				通知成功.
 *	@retval		False				通知失敗.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::changePreSettingOk( uchar option )
{
	if( 1 > c_PopupNum ) {
		return False;	// ポップアップが起動していない場合は、False返却.
	}

	// 最上位が組み合わせ禁止ポップアップかどうかを確認する.
	PPopupCombinationErrorCom* pc_Instance = dynamic_cast< PPopupCombinationErrorCom* >( p_PopupTable[( uchar )c_PopupNum ] );
	if( NULL != pc_Instance ) {
		// 先設定と組合せ不可の機能をキャンセルする.
		PNLC_CombinationAct* cp_CombinationAct = PNLC_CombinationAct::getInstance();
		cp_CombinationAct->actCmbCancel();
		// ひとつ下の階層に先設定変更許可を通知する.
		p_PopupTable[ c_PopupNum - 1 ]->procChangePreSettingOk( option );
		// 組合せ禁止の解除機能情報を初期化.
		cp_CombinationAct->clearCmbCancel();

	} else {
		return False;	// 最上位が組み合わせ禁止ポップアップではないときは、Falseを返却.
	}

	return True;
}

// ===============================================================================================
/**
 *	@brief		ヒントアイコン処理.
 */
// ===============================================================================================
void PNLC_PopupListControl::procHintIcon( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procHintIcon( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		[画面ジャンプ].
 */
// ===============================================================================================
void PNLC_PopupListControl::procScreenJump( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procScreenJump( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		実行ボタン.
 */
// ===============================================================================================
void PNLC_PopupListControl::procExecute( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procExecute( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		パネルリセット.
 */
// ===============================================================================================
void PNLC_PopupListControl::procReset( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procReset( option );
			break;
		}
	}
}
// comment 2019/03/20 SCD2-Sky K.Kasemura [IT6_2.0MR2][OP_BTS-18772] フッターエリア対応.
// ===============================================================================================
/**
 *	@brief		リセットキー活性状態確認.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::resetKeyActiveCheck()
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			ACTIVEKEY_RET( True );
		}
	}
	ACTIVEKEY_RET( True );
}

// ===============================================================================================
/**
 *	@brief		スタート.
 */
// ===============================================================================================
void PNLC_PopupListControl::procStart( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procStart( option );
			break;
		}
	}
}
//	Comment 2016/06/14 HCLT_Naveen [ZeusSBK][CQ528889] Job send Implementation after Domain Filtering for RQ-260.
//====================================================================================================================.
/**
* @brief  : Call to before screen start function..
* @note   : None
* @param  : uchar.
* @return : None.
* @author :	2016/06/14 HCLT_Naveen [ZeusSBK][CQ528889].
*/
//====================================================================================================================.
void PNLC_PopupListControl::procSendStart( uchar option )
{
// Comment 2016/09/16  【HCLT-Krishna】 【IT5_3.3】【Accurev ID:1004076】【Warning Removal Activity】Type casting char to uchar due to array subscript data type mismatch.
//	PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ c_PopupNum ] );
	PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[( uchar ) c_PopupNum ] );	
	if( NULL != p ) {
		p->procStart( option );
	}

}
// ===============================================================================================
/**
 *	@brief		ストップ.
 */
// ===============================================================================================
void PNLC_PopupListControl::procStop( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procStop( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		カラーモードキー.
 */
// ===============================================================================================
void PNLC_PopupListControl::procColorMode( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procColorMode( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		割り込みモード.
 */
// ===============================================================================================
void PNLC_PopupListControl::procIntMode( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procIntMode( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		暗証番号入力.
 */
// ===============================================================================================
void PNLC_PopupListControl::procIDInput( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procIDInput( option );
			break;
		}
	}
}
// comment 2020/07/14 SCD2-Sky 石原 [IT6_2.0MR2][OP_BTS-25362] フッターエリア対応.
// ===============================================================================================
/**
 *	@brief		IDキー活性状態確認.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::idKeyActiveCheck()
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			return p->idKeyActiveCheck();
		}
	}
	ACTIVEKEY_RET( True );
}
// ===============================================================================================
/**
 *	@brief		サービスマンモード.
 */
// ===============================================================================================
void PNLC_PopupListControl::procService( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procService( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		コピーキー.
 */
// ===============================================================================================
void PNLC_PopupListControl::procCopy( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procCopy( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		プレヒートキー押下応答.
 */
// ===============================================================================================
void PNLC_PopupListControl::procPreHeatOnByKeyAns( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
// comment 2020/09/25 Sky東島 [IT6_2.1][OP_BTS-27378].
// 設定メニュー画面などIT4より前に作成された画面の場合に処理が行われないため、画面テーブルの一番下がポップアップでない場合でも処理するよう変更.
// ※画面の一番下がポップアップは通常ありえないが念のためチェックする.
//   ポップアップに対して処理を行うとPPopupCom::procPreHeatOnByKeyAnsと本関数で無限ループとなるため.
//		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
		if( ( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) )		// ベース画面判定.
		 || ( ( 0 == ss_PopupTablNo )
		   && ( NULL == dynamic_cast< PPopupCom* >( p_PopupTable[ss_PopupTablNo] ) ) ) ) {	// ベースとなっている画面がポップアップでない場合.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
// comment 2020/09/25 Sky東島 [IT6_2.1][OP_BTS-27378].
			if( NULL == p ){
				break;
			}
			p->procPreHeatOnByKeyAns( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		省エネ解除.
 */
// ===============================================================================================
void PNLC_PopupListControl::procEnergySaveReset( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
// comment 2019/08/27 Sky東島 [EagleHigh][OP_BTS-12004].
//		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
		if( ( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) )		// ベース画面判定.
		 || ( &gp_ActionInitialSetup == p_PopupTable[ss_PopupTablNo] ) ) {					// 起動中画面.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procEnergySaveReset( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		拡大表示.
 */
// ===============================================================================================
void PNLC_PopupListControl::procEnlarge( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procEnlarge( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		確認コピー.
 */
// ===============================================================================================
void PNLC_PopupListControl::procSampleCopy( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procSampleCopy( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		ジョブメモリ.
 */
// ===============================================================================================
void PNLC_PopupListControl::procJobMemory( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procJobMemory( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		ユーティリティ.
 */
// ===============================================================================================
void PNLC_PopupListControl::procUtility( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procUtility( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		FAX.
 */
// ===============================================================================================
void PNLC_PopupListControl::procFax( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procFax( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		FAX.
 */
// ===============================================================================================
void PNLC_PopupListControl::procFaxOnly( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procFaxOnly( option );
			break;
		}
	}
}

// ===============================================================================================
/**
 *	@brief		ボックス.
 */
// ===============================================================================================
void PNLC_PopupListControl::procBox( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procBox( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		ユビキタスプリント.
 */
// ===============================================================================================
void PNLC_PopupListControl::procUbqPrint( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procUbqPrint( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		パワーセーブ.
 */
// ===============================================================================================
void PNLC_PopupListControl::procPowerSave( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procPowerSave( option );
			break;
// comment 2017/04/27 HCLT-AppalaRaju [IT5_4.1][1016030]  Added the condition to enter into sleep mode when top menu top of sleep mode screen/utility/service/accesibility.
		} else if (&gp_HomeTopMenu == p_PopupTable[( uchar ) ss_PopupTablNo ]) {
			gp_HomeTopMenu.sleepMode(option);
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		ヘルプ.
 */
// ===============================================================================================
void PNLC_PopupListControl::procHelp( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procHelp( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		ユーザー補助.
 */
// ===============================================================================================
void PNLC_PopupListControl::procAccessibility( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procAccessibility( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		音声ガイド終了関数.
 */
// ===============================================================================================
void PNLC_PopupListControl::procVoiceGuide_End( TYPE_VoiceGuide_Status e_Status )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procVoiceGuide_End( e_Status );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		プログラム呼出.
 */
// ===============================================================================================
void PNLC_PopupListControl::procProgram( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procProgram( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		１秒タイマ.
 */
// ===============================================================================================
void PNLC_PopupListControl::procSecond( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procSecond( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		定期起床.
 */
// ===============================================================================================
void PNLC_PopupListControl::procTimerWakeUp( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procTimerWakeUp( option );
			break;
		} else {
			// ベース画面が旧UI画面（サービス、Utility、Helpなど・・）の場合.
			if( 0 == ss_PopupTablNo ) {
				// 警告表示中は、警告ポップアップ共通処理で独自の定期起床処理を実施しているので、ここでは呼ばない.
// comment 2015/10/01 OPS1-Sk蒲原 [PZH2.1c][CQ516371] OAPの警告表示中もベースの処理を行わない(ユーティリティの上にOAP警告表示中、前ドアOpenでユーティリティに遷移する).
//				if( NULL == dynamic_cast< PPopupWarningCom* >( p_PopupTable[( uchar )c_PopupNum] ) ) {
				if ( NULL == dynamic_cast< PPopupWarningCom* >( p_PopupTable[( uchar )c_PopupNum] )
				  && NULL == dynamic_cast< POAP_PopupWarningCom* >( p_PopupTable[( uchar )c_PopupNum] ) ) { 
					PBasicMessageProc* pc_BasicMessageProc = ( PBasicMessageProc* )( p_PopupTable[0] );
					pc_BasicMessageProc->procTimerWakeUp( option );
				}
			}
		}
	}
}
// ===============================================================================================
/**
 *	@brief		オートリセット要求.
 */
// ===============================================================================================
void PNLC_PopupListControl::procAutoResetReq( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( ( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ ss_PopupTablNo ] ) )
		    || ( NULL == dynamic_cast< PLayerControl* >( p_PopupTable[ ss_PopupTablNo ] ) ) ) {
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procAutoResetReq( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		オートリセットOFF時の１分タイマー処理要求.
 */
// ===============================================================================================
void PNLC_PopupListControl::procAutoResetOffTimerReq()
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( ( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ ss_PopupTablNo ] ) )
		    || ( NULL == dynamic_cast< PLayerControl* >( p_PopupTable[ ss_PopupTablNo ] ) ) ) {
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procAutoResetOffTimerReq();
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		ジョブ破棄エラー変化通知.
 */
// ===============================================================================================
void PNLC_PopupListControl::procJobLogChange( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procJobLogChange( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		割り込みモード応答.
 */
// ===============================================================================================
void PNLC_PopupListControl::procIntModeAns( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procIntModeAns( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		アプリ表示要求.
 */
// ===============================================================================================
void PNLC_PopupListControl::procAppliDisplayReq( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procAppliDisplayReq( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		ＦＴＰ接続完了.
 */
// ===============================================================================================
void PNLC_PopupListControl::procFTPConnection( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procFTPConnection( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		ＷｅｂＤＡＶ接続完了.
 */
// ===============================================================================================
void PNLC_PopupListControl::procWebDAVConnection( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procWebDAVConnection( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		カウンタ引き抜き通知.
 */
// ===============================================================================================
void PNLC_PopupListControl::procResetAtCountNotice( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procResetAtCountNotice( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		リモートパネル状態表示通知.
 */
// ===============================================================================================
void PNLC_PopupListControl::procRPnlStatusNotice( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procRPnlStatusNotice( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		リモートパネルクローズ要求.
 */
// ===============================================================================================
void PNLC_PopupListControl::procRPnlCloseRequest( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procRPnlCloseRequest( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		スキャン／プリント状態の変化.
 */
// ===============================================================================================
void PNLC_PopupListControl::procStatusChange( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procStatusChange( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		プリントジョブ状態の変化(プリント停止中の変化).
 */
// ===============================================================================================
void PNLC_PopupListControl::procPrintJobStopChange( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procPrintJobStopChange( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		スキャナ配信方式の変化 (TWAIN).
 */
// ===============================================================================================
void PNLC_PopupListControl::procScanJobSendChange( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procScanJobSendChange( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		スキャナ読み取り開始の変化.
 */
// ===============================================================================================
void PNLC_PopupListControl::procScanJobScanStartChange( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procScanJobScanStartChange( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		動作中が無くなった.
 */
// ===============================================================================================
void PNLC_PopupListControl::procActiveJobEnd( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procActiveJobEnd( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		原稿サイズ有効の変化.
 */
// ===============================================================================================
void PNLC_PopupListControl::procDocSizeEffective( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procDocSizeEffective( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		手差しトレイへの用紙挿入.
 */
// ===============================================================================================
void PNLC_PopupListControl::procPaperInsertToManualFeed( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procPaperInsertToManualFeed( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		手差しトレイからの用紙引抜き.
 */
// ===============================================================================================
void PNLC_PopupListControl::procPaperPutOffToManualFeed( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procPaperPutOffToManualFeed( option );
			break;
		}
	}
}
//comment 2018/02/23 【HCLT-AppalaRaju】 【IT6 EAGLE】 【機器構成変更No.2】 procPaperInsertToTray1Feed Function.
/**
* @brief	Insert Paper to Tray1.
* @note		none.
* @param	[in]	uchar option.
* @return	none.
*/
#ifndef DEF_A4_Cus
void PNLC_PopupListControl::procPaperInsertToTray1Feed( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procPaperInsertToTray1Feed( option );
			break;
		}
	}
}
/**
* @brief	Remove Paper to Tray1.
* @note		none.
* @param	[in]	uchar option.
* @return	none.
*/
void PNLC_PopupListControl::procPaperPutOffToTray1Feed( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procPaperPutOffToTray1Feed( option );
			break;
		}
	}
}	
#endif
// ===============================================================================================
/**
 *	@brief		ＡＤＦ原稿トレイの状況変化.
 */
// ===============================================================================================
void PNLC_PopupListControl::procDocOnADF( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procDocOnADF( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		原稿カバー開閉の状況変化.
 */
// ===============================================================================================
void PNLC_PopupListControl::procDocCoverOpen( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procDocCoverOpen( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		給紙口情報の変化.
 */
// ===============================================================================================
void PNLC_PopupListControl::procFeedInfoChange( TYPE_FeedTray e_FeedTray )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procFeedInfoChange( e_FeedTray );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		初期立ち上げ終了.
 */
// ===============================================================================================
void PNLC_PopupListControl::procComplatePWOnStart( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procComplatePWOnStart( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		用紙サイズ・用紙種類変化.
 */
// ===============================================================================================
void PNLC_PopupListControl::procFeedPaperKindChange( uchar option, const ushort kind )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procFeedPaperKindChange( option, kind );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		特殊画面表示の変化通知.
 */
// ===============================================================================================
void PNLC_PopupListControl::procSpecialGamenShowStatus( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procSpecialGamenShowStatus( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		ファクスボードからのコマンドレスポンスダイアル完了.
 */
// ===============================================================================================
void PNLC_PopupListControl::procFaxCmdRsKtd( uchar option, TYPE_CommLine e_CommLine, FAXS_CmdRs s_CmdRs )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procFaxCmdRsKtd( option, e_CommLine, s_CmdRs );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		ファクス回線1状態変化（OnHook）.
 */
// ===============================================================================================
void PNLC_PopupListControl::procFaxLine1StatueIdle( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procFaxLine1StatueIdle( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		ファクス回線1状態変化（OffHook）.
 */
// ===============================================================================================
void PNLC_PopupListControl::procFaxLine1StatueComm( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procFaxLine1StatueComm( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		テスト結果通知.
 */
// ===============================================================================================
void PNLC_PopupListControl::procFaxTestResult( uchar option, TYPE_CommLine e_FaxLineNo, TYPS_TestResult s_TestResult )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procFaxTestResult( option, e_FaxLineNo, s_TestResult );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		トラブルの発生／解除.
 */
// ===============================================================================================
void PNLC_PopupListControl::procFatalExist( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procFatalExist( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		全画面警告(ERROR､CAUTION)発生／解除.
 */
// ===============================================================================================
void PNLC_PopupListControl::procWarningGamenExist( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procWarningGamenExist( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		ブザーを鳴らす警告発生.
 */
// ===============================================================================================
void PNLC_PopupListControl::procBuzzerWarningExist( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procBuzzerWarningExist( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		組合せ禁止発生／解除.
 */
// ===============================================================================================
void PNLC_PopupListControl::procEnlCombinationExist( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procEnlCombinationExist( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		ファンクション終了.
 */
// ===============================================================================================
void PNLC_PopupListControl::procFunctionEnd( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procFunctionEnd( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		ＡＰＳによる給紙口指定が確定.
 */
// ===============================================================================================
void PNLC_PopupListControl::procFeedTrayEnable( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procFeedTrayEnable( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		オブザーバからのコール.
 */
// ===============================================================================================
void PNLC_PopupListControl::procObserver( TYPE_PNL_Observer option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procObserver( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		オブザーバからのコール.
 */
// ===============================================================================================
void PNLC_PopupListControl::procObserver( TYPE_PNL_Observer option, char c_LayerNum )
{
	char sc_ChoiceLayerNum = 0;

	sc_ChoiceLayerNum = getLayerchoice( c_LayerNum );
	if( NULL != p_PopupTable[( uchar )sc_ChoiceLayerNum ] ) {
		PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[( uchar )sc_ChoiceLayerNum ] );
		p->procObserver( option );
	}
}
// ===============================================================================================
/**
 *	@brief		ボックスオブザーバからのコール.
 */
// ===============================================================================================
void PNLC_PopupListControl::procBoxObserver( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procBoxObserver( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		ボックスオブザーバからのコール.
 */
// ===============================================================================================
void PNLC_PopupListControl::procBoxObserver( uchar option, char c_LayerNum )
{
	char sc_ChoiceLayerNum = 0;

	sc_ChoiceLayerNum = getLayerchoice( c_LayerNum );
	if( NULL != p_PopupTable[( uchar )sc_ChoiceLayerNum ] ) {
		PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[( uchar )sc_ChoiceLayerNum ] );
		p->procBoxObserver( option );
	}
}

// ===============================================================================================
/**
 *	@brief		ベンダー側処理完了通知のコール.
 */
// ===============================================================================================
void PNLC_PopupListControl::procVederCallBack( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procVederCallBack( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		統合管理装置 カード挿入/引き抜き通知.
 */
// ===============================================================================================
void PNLC_PopupListControl::procAdminMachineKMCardNotice( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procAdminMachineKMCardNotice( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		サムネイル画像取得完了コールバックによる通知.
 */
// ===============================================================================================
void PNLC_PopupListControl::procThumbnailCallBack( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procThumbnailCallBack( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		サムネイル画像取得完了コールバックによる通知.
 */
// ===============================================================================================
void PNLC_PopupListControl::procThumbnailCallBack( uchar option, char c_LayerNum )
{
	char sc_ChoiceLayerNum = 0;

	sc_ChoiceLayerNum = getLayerchoice( c_LayerNum );
	if( NULL != p_PopupTable[( uchar )sc_ChoiceLayerNum ] ) {
		PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[( uchar )sc_ChoiceLayerNum ] );
		p->procThumbnailCallBack( option );
	}
}
// ===============================================================================================
/**
 *	@brief		プレビュー画像取得完了コールバックによる通知.
 */
// ===============================================================================================
void PNLC_PopupListControl::procPreviewCallBack( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procPreviewCallBack( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		プレビュー画像取得完了コールバックによる通知.
 */
// ===============================================================================================
void PNLC_PopupListControl::procPreviewCallBack( uchar option, char c_LayerNum )
{
	char sc_ChoiceLayerNum = 0;

	sc_ChoiceLayerNum = getLayerchoice( c_LayerNum );
	if( NULL != p_PopupTable[( uchar )sc_ChoiceLayerNum ] ) {
		PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[( uchar )sc_ChoiceLayerNum ] );
		p->procPreviewCallBack( option );
	}
}

// ===============================================================================================
/**
 *	@brief		携帯連携通信状態通知からのコール.
 */
// ===============================================================================================
void PNLC_PopupListControl::procCellularPhoneStatus( TYPE_PNL_AppStatus option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procCellularPhoneStatus( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		【304362】【Webブラウザからのスキャン画像の送信】【13山本(由)】merge.
 */
// ===============================================================================================
void PNLC_PopupListControl::procULControlNotice( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procULControlNotice( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		【304362】【AppReq対応】【13山崎】merge.
 */
// ===============================================================================================
void PNLC_PopupListControl::procAppReqObserver( TYPE_PNL_AppReqObserver option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procAppReqObserver( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		何らかのキーが押された.
 */
// ===============================================================================================
void PNLC_PopupListControl::procSomethingKeyOn( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procSomethingKeyOn( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		外部メモリー複数検知警告.
 */
// ===============================================================================================
void PNLC_PopupListControl::procUSBMemoryTwoInsert( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procUSBMemoryTwoInsert( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		外部メモリー検出時.
 */
// ===============================================================================================
void PNLC_PopupListControl::procUSBDetection( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procUSBDetection( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		外部メモリー読み込み中のエラー検出.
 */
// ===============================================================================================
void PNLC_PopupListControl::procUSBReadTurnChange( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procUSBReadTurnChange( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		USBメモリー引き抜き通知.
 */
// ===============================================================================================
void PNLC_PopupListControl::procUsbPullingOut( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procUsbPullingOut( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		SPC用ボタン（押された）.
 */
// ===============================================================================================
void PNLC_PopupListControl::procSPCButtonOn( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procSPCButtonOn( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		音声再生完了通知.
 */
// ===============================================================================================
void PNLC_PopupListControl::procVoicePlayEnd( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procVoicePlayEnd( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		音声再生停止完了通知.
 */
// ===============================================================================================
void PNLC_PopupListControl::procVoiceStopEnd( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procVoiceStopEnd( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		自動時刻補正実行要求.
 */
// ===============================================================================================
void PNLC_PopupListControl::procAutoTimeCorrect()
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procAutoTimeCorrect();
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		残金不足警告検出.
 */
// ===============================================================================================
void PNLC_PopupListControl::procBoxSendJobCheck( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procBoxSendJobCheck( option );
			break;
		}
	}
}
#if defined(WIN32) && defined(SIMULATOR)	// 言語対応支援. 
// ===============================================================================================
/**
 *	@brief		モードキー　■言語対応用.
 */
// ===============================================================================================
void PNLC_PopupListControl::procMode( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procMode( option );
			break;
		}
	}
}
#endif	// defined(WIN32) && defined(SIMULATOR)
// ===============================================================================================
/**
 *	@brief		キーボード上のキー.
 */
// ===============================================================================================
void PNLC_PopupListControl::procKeyBoardOnVoice( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procKeyBoardOnVoice( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		ワンタッチキー.
 */
// ===============================================================================================
void PNLC_PopupListControl::procOneTouchKeyVoice( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procOneTouchKeyVoice( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		2008/6/17 Donau音声対応 平野.
 */
// ===============================================================================================
void PNLC_PopupListControl::procSPCButtonOnVoice( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procSPCButtonOnVoice( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		コールバック.
 */
// ===============================================================================================
void PNLC_PopupListControl::procCallBack( TYPE_PNL_CallBack option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procCallBack( option );
			break;
		}
	}
}
// ===============================================================================================
/**
 *	@brief		コールバック.
 */
// ===============================================================================================
void PNLC_PopupListControl::procCallBack( TYPE_PNL_CallBack option, char c_LayerNum )
{
	char sc_ChoiceLayerNum = 0;

	sc_ChoiceLayerNum = getLayerchoice( c_LayerNum );
	if( NULL != p_PopupTable[( uchar )sc_ChoiceLayerNum ] ) {
// comment 2017/5/8 SCD2-Sky工藤 [特ROM][FQ-OPSD-1700111-OFP079] コピー画面からトップ画面に遷移した後、トップ画面に登録したファクス/スキャンプログラムを押すと登録した宛先名が表示されない.
//		PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[( uchar )sc_ChoiceLayerNum ] );
//		p->procCallBack( option );
		if( ( TYPE_PNL_CallBack_JobAddressSetResult == option ) && ( NULL == dynamic_cast<PCGC_ScanFaxBaseCom*>( p_PopupTable[( uchar )sc_ChoiceLayerNum ] ) ) ) {
				gp_FromRegAddress_scanfax.setCallBackJobAddressSetResultFlg( True );
				return;
		}
		else {
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[( uchar )sc_ChoiceLayerNum ] );
			p->procCallBack( option );
		}
	}
}
// ===============================================================================================
/**
 *	@brief		Amur_GCC4.1対応_No002.
 */
// ===============================================================================================
void PNLC_PopupListControl::procBrowser( uchar option )
{
	short ss_PopupTablNo;

	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( NULL != dynamic_cast< PBaseGamenCom* >( p_PopupTable[ss_PopupTablNo] ) ) {		// ベース画面判定.
			PBasicMessageProc* p = ( PBasicMessageProc* )( p_PopupTable[ss_PopupTablNo] );
			p->procBrowser( option );
			break;
		}
	}
}

// ===============================================================================================.
/**
 *	@brief		タブレット切断通知.
 */
// ===============================================================================================.
void PNLC_PopupListControl::procTabletDisconnect( uchar option, char c_LayerNum )
{
	if( 0 >= c_LayerNum ) { return; }

	char sc_ChoiceLayerNum = 0;
	sc_ChoiceLayerNum = getLayerchoice( c_LayerNum );
	if( NULL != p_PopupTable[( uchar )sc_ChoiceLayerNum ] ) {
		PBasicMessageProc* p = p_PopupTable[( uchar )sc_ChoiceLayerNum ];
		if( False == PNLC_TabletResetControlLib::newInstance()->isWaitFinished( p ) ) {
			p->procTabletDisconnect( option );
		} else {
			// 再帰呼び出し.
			procTabletDisconnect( option, sc_ChoiceLayerNum );
		}
	}
}
// ===============================================================================================.
/**
 *	@brief		タブレット接続通知.
 */
// ===============================================================================================.
void PNLC_PopupListControl::procTabletConnect( uchar option, char c_LayerNum )
{
	if( 0 >= c_LayerNum ) { return; }

	char sc_ChoiceLayerNum = 0;
	sc_ChoiceLayerNum = getLayerchoice( c_LayerNum );
	if( NULL != p_PopupTable[( uchar )sc_ChoiceLayerNum ] ) {
		PBasicMessageProc* p = p_PopupTable[( uchar )sc_ChoiceLayerNum ];
		if( False == PNLC_TabletResetControlLib::newInstance()->isWaitFinished( p ) ) {
			p->procTabletConnect( option );
		} else {
			// 再帰呼び出し.
			procTabletConnect( option, sc_ChoiceLayerNum );
		}
	}
}

// ===============================================================================================
/**
 *	@brief		ポップアップ数取得.
 */
// ===============================================================================================
char PNLC_PopupListControl::getPopupNum( void )
{
	return c_PopupNum;
}

// ===============================================================================================
/**
 *	@brief		ハードキー無効判定.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::isDisableHardKeyEvent( Message& message, uchar& option, MessageType& type )
{
	// ポップアップ起動中でアニメーション中か?
	if( ( 0 < c_PopupNum ) && ( True == p_PopupTable[( uchar )c_PopupNum ]->isNowPopupAnimation() ) )
	{
		PNLC_ComEventKey event( message, option, type );
		message = MS_Dummy;
		p_PopupTable[( uchar )c_PopupNum ]->setBuzzerType( PCGE_BuzzerType_InputNg );
		return True;
	}
	return False;
}

// ===============================================================================================
/**
 *	@brief		タッチパネル無効判定.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::isDisableTouchPanel( Message& message, uchar& option, MessageType& type )
{
	// ポップアップ起動中でアニメーション中か?
	if( ( 0 < c_PopupNum ) && ( True == p_PopupTable[( uchar )c_PopupNum ]->isNowPopupAnimation() ) )
	{
		PNLC_ComEventTouch event( message, option, type, PnlComGetTouchPanelPointBackup().x, PnlComGetTouchPanelPointBackup().y );
		message = MS_Dummy;
		p_PopupTable[( uchar )c_PopupNum ]->setBuzzerType( PCGE_BuzzerType_InputNg );
		return True;
	}
	return False;
}
// ===============================================================================================
/**
 *	@brief		USBキーボード無効判定.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::isDisableUSBKeyEvent( Message& message, uchar& option, MessageType& type,  int32_t sl_Option )
{
	// ポップアップ起動中でアニメーション中か?
	if( ( 0 < c_PopupNum ) && ( True == p_PopupTable[( uchar )c_PopupNum ]->isNowPopupAnimation() ) )
	{
		PNLC_ComEventOther event( message, option, type, sl_Option );
		message = MS_Dummy;
		return True;
	}
	return False;
}

PLayerControl* PNLC_PopupListControl::getNowPopup()
{
	if( 0 >= c_PopupNum ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}
	PLayerControl* pc_NowPopup = dynamic_cast< PLayerControl* >( p_PopupTable[( uchar )c_PopupNum ] );
	return pc_NowPopup;
}

// ===============================================================================================.
/**
 *	@brief		現在表示中のポップアップを返す(プルダウンの場合は、その下位のレイヤのポップアップを返す).
 */
// ===============================================================================================.
PLayerControl* PNLC_PopupListControl::getNowPopupNonPullDown()
{
	if( 0 >= c_PopupNum ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}
	PLayerControl* pNowPopup =  dynamic_cast<  PLayerControl* >( p_PopupTable[( uchar )c_PopupNum ] );
	// プルダウンの場合は、その下位のレイヤのインスタンスを返す.
	if( NULL != dynamic_cast< PPopupPullDownCom* >( pNowPopup ) ) {
		pNowPopup = dynamic_cast<  PLayerControl* >( p_PopupTable[ c_PopupNum -1 ] );
	}
	return pNowPopup;
}

// ===============================================================================================
/**
 *	@brief		現在のポップアップの発生元を返す.
 */
// ===============================================================================================
PLayerControl* PNLC_PopupListControl::getSrcPopup()
{
	if( 1 >= c_PopupNum ) {
		// ポップアップが１階層の場合はNULLを返す.
		return NULL;
	}
	PLayerControl* pc_SrcPopup = dynamic_cast< PLayerControl* >( p_PopupTable[ c_PopupNum - 1 ] );
	return pc_SrcPopup;
}
// ===============================================================================================
/**
 *	@brief		次に表示されるポップアップを返す.
 */
// ===============================================================================================
PLayerControl* PNLC_PopupListControl::getNextPopup()
{
	return m_pc_nextPopupInstance;
}

// ===============================================================================================
/**
 *	@brief		指定した階層のポップアップ画面インスタンスを取得.
 */
// ===============================================================================================
PBasicMessageProc* PNLC_PopupListControl::getPopupInstance( char c_PopupLayer )
{
	// 範囲外の場合はNULLを返す.
	if( ( c_PopupLayer < 0 ) || ( c_PopupNum < c_PopupLayer ) ) {
		return NULL;
	}
// comment 2015/03/30 MSE永野 [Helios][CQ510921] Warning除去対応.
//	return p_PopupTable[ c_PopupLayer ];
	return p_PopupTable[ (uchar)c_PopupLayer ];
}

// ===============================================================================================
/**
 *	@brief		ポップアップ切替処理.
 */
// ===============================================================================================
void PNLC_PopupListControl::ChangePopup()
{
	// ベース画面はポップアップしない.
	PBaseGamenCom* pc_Base = dynamic_cast<PBaseGamenCom*>( p_ChangeGamen );
	if( NULL != pc_Base ) { return; }

	// 現在のポップアップ表示数をチェック.
	if( PNLD_PopupMaxNum <= ( char )( c_PopupNum + 1 ) ) {
		removeAllPopup( True );
		PnlComGetSystemStatus()->setFatal( 0xCC00 );
		CNTC_CounterIf::newInstance()->countTrouble( ( ushort )0xCC00 );				// インスタンス取得(CNTC_Counter) .
		p_ChangeGamen = NULL;
//comment 2017/07/05 SCD2-Sky肥後 [1026857] 新詳細コード対応.
		LAPC_BFW_System *pc_BFW_System = LAPC_BFW_System::newInstance();
		if (NULL != pc_BFW_System){
	   	 	pc_BFW_System ->registDetailCode( PNLD_DetailCode_FileIdentification_0x1900740022000000 | ( __LINE__ << 4 ) );
		}
		return;																				// 最大表示数以上の場合は、これ以上起動不可.
	}

	// 指定されたインスタンスが既に起動済みかどうかをチェック.
	for( register uchar i=0; i<=c_PopupNum; ++i ) {
//comment 2019/05/28 SCD2-Sky東野 [IT6_1.0MR2] [1077536] 切替先がホーム画面の場合はポップアップ切替を許可するよう修正。.
//		if( p_ChangeGamen == p_PopupTable[i] ) {
		if( ( p_ChangeGamen == p_PopupTable[i] )
		    && ( &gp_HomeTopMenu != p_ChangeGamen ) ) {
			p_ChangeGamen = NULL;
			return;																			// 同一インスタンスが登録済みの場合、起動不可.
		}
	}
	// ポップアップしようとしている画面が起動中かどうかもチェックする.
	PPopupCom* pc_PopupCom = dynamic_cast< PPopupCom* >( p_ChangeGamen );
	if( ( NULL != pc_PopupCom ) && ( False != pc_PopupCom->isOpened() ) ) { return; }

	if( 1 <= c_PopupNum && True == p_PopupTable[( uchar )c_PopupNum ]->isNowPopupAnimation() ) {
		return;																				// 最上位ポップアップがアニメーション中の場合は起動不可.
	}

	++c_PopupNum;																			// ポップアップ表示数をインクリメント.

	p_PopupTable[( uchar )c_PopupNum ] = p_ChangeGamen;												// 指定されたインスタンスをポップアップ管理リストに追加.

	p_PopupTable[( uchar )c_PopupNum ]->updateFuncBadgeButtonStatus();								// ポップアップアニメーション前に機能バッジのアイコンステータスを更新する.

	PNLC_StackManJobSpec::newInstance()->saveJobSpec( c_PopupNum );							// ジョブスペックを保存(ポップアップ中止時に設定を戻すため)

	p_PopupTable[( uchar )c_PopupNum ]->setLayerNumber( c_PopupNum );								// 追加したポップアップに、確定した階層情報を設定.

	p_ChangeGamen = NULL;

	p_PopupTable[( uchar )c_PopupNum ]->opened( p_mmbCallbackFunc, m_ss_OpeningAnimPosX, m_ss_OpeningAnimPosY, m_ss_OpeningAnimSizeX, m_ss_OpeningAnimSizeY, True );	// 追加したポップアップの起動処理を呼び出す.

// comment 2020/11/25 Sky Kawanishi [LocalUI特ROM] SegF(スタックオーバーフロー)時ログ改善.
	LUIC_DbgTsk::newInstance()->setPopupList((void**)&(p_PopupTable[0]), c_PopupNum + 1);
	// 画面変更によるログの収集を有効にする
	PNLC_OperationLog::newInstance()->setDisableChangePanel( False );
	// 画面変更によるログの収集.
	PNLC_OperationLog::newInstance()->setLogChangePanel( p_PopupTable[( uchar )c_PopupNum ] );
}

// ===============================================================================================
/**
 *	@brief		変更画面(ポップアップ)設定関数.
 */
// ===============================================================================================
void PNLC_PopupListControl::setChangePopup( PLayerControl*	p_PopupInstance, void ( *p_CallbackFunc )( uchar, short, void*, uint32_t ) )
{
	// ベース画面はポップアップしない.
	PBaseGamenCom* pc_Base = dynamic_cast<PBaseGamenCom*>( p_ChangeGamen );
	if( NULL != pc_Base ) { return; }

	//														該データメンバをChangePopup()内でコールされるopen()のパラメータに設定する.
	PPopupAnimationCom* p_toplayerscreen = dynamic_cast< PPopupAnimationCom* >( p_PopupTable[( uchar )c_PopupNum ] );
	if( NULL != p_toplayerscreen ) {
		p_toplayerscreen->getOpeningAnimPosSize( m_ss_OpeningAnimPosX, m_ss_OpeningAnimPosY, m_ss_OpeningAnimSizeX, m_ss_OpeningAnimSizeY );
	}

	b_mSetChangePopup = True;
	// 画面変更によるログの収集を無効にする
	PNLC_OperationLog::newInstance()->setDisableChangePanel( True );
	p_ChangeGamen = p_PopupInstance;
	p_mmbCallbackFunc = p_CallbackFunc;
	closeTop( False, True );
}

// ===============================================================================================
/**
 *	@brief		子クラスから使用する関数 : 指定したポップアップが表示中ポップアップかどうか判断する.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::isNowPopup( PLayerControl* p_PopupInstance )
{
	if( NULL == p_PopupInstance		// 起動するポップアップインスタンスのNULLチェック.
	    || PNLD_PopupMaxNum <= c_PopupNum		// 現在のポップアップ表示数をチェック.
	    || 1 > c_PopupNum ) {
		return False;
	}
	PLayerControl* pc_NowPopup = dynamic_cast< PLayerControl* >( p_PopupTable[( uchar )c_PopupNum] );
	if( NULL == pc_NowPopup ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	return ( p_PopupInstance == p_PopupTable[( uchar )c_PopupNum] )? True : False;
}

// ===============================================================================================
/**
 *	@brief		指定したポップアップがポップアップ管理リストに存在するかを判断する.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::isPopupLayer( PLayerControl* p_PopupInstance )
{
	if( NULL == p_PopupInstance ) {
		return False;
	}

	short ss_PopupTablNo = 0;
	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		if( p_PopupInstance == p_PopupTable[ss_PopupTablNo] ) {
			return True;
		}
	}

	// 「しばらくお待ちください」P.U.の場合は遅延表示予定かチェック
	Bool isClearDisp, isPopup;
	if( DelayedIGCWaitDisplayTimer::isAlive( isClearDisp, isPopup ) != False ) {
		if( ( p_PopupInstance == &gp_IGCWait_popup ) &&
		    ( isClearDisp == False ) && ( isPopup == True ) ) {
			return True;
		}
		else if( ( p_PopupInstance == &gp_IGCClearWait_popup ) &&
		         ( isClearDisp != False ) && ( isPopup == True ) ) {
			return True;
		}
		else {
			; // 処理なし
		}
	}

	return False;
}
// ===============================================================================================
/**
 *	@brief		指定したポップアップがポップアップ起動予約情報テーブルに存在するかを判断する.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::isReservePopup( PLayerControl* p_PopupInstance )
{
	if( NULL == p_PopupInstance ) {
		return False;
	}

	for( short i = 0; i < c_ReservedNum; i++ ) {
		if( p_PopupInstance == s_ReserveTable[i].p_PopupInstance ) {
			return True;
		}
	}
	return False;
}

// ===============================================================================================
/**
 *	@brief		既にポップアップ予約が行われているか判定.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::isEnqueuedPopup( PLayerControl*	p_PopupInstance )
{
	for( uchar m = 0; m < sc_QueueCount; ++m ) {
		if( p_PopupInstance == s_PopupQueue[m].p_PopupInstance ) {
			return True;
		}
	}
	return False;

}
// ===============================================================================================
/**
 *	@brief		ショートカット予約されているか判定する.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::isShortCutPopup()
{
	// ショートカット予約されていればTrueを返す.
	if( PNLE_ShortCutID_NON != e_ShortCutPopupID ) {
		return True;
	}
	return False;
}

// ===============================================================================================
/**
 *	@brief		ショートカット予約する.
 */
// ===============================================================================================
void PNLC_PopupListControl::setReserveShortCut( PNLE_ShortCutID e_ShortCutID, PNLE_ShortCutKindID e_ShortCutKindID )
{
	// ショートカット識別ID、ショートカット種別IDを登録する.
	e_ShortCutPopupID = e_ShortCutID;
	e_ShortCutPopupKindID  = e_ShortCutKindID;
}

// ===============================================================================================
/**
 *	@brief		ショートカット予約されているポップアップを表示する.
 */
// ===============================================================================================
Bool PNLC_PopupListControl::addShortCutPopup()
{
	TYPE_PNLGRP1_COPY				e_FunctionID_Copy			= TYPE_PNLGRP1_COPY_NON;					//機能ID_コピー.
	PNLE_FUNCTIONTYPE_COPY			e_FunctionBudgeID_Copy		= PNLE_FUNCTIONTYPE_COPY_MAX;				//機能バッヂID_コピー.
	TYPE_PNLGRP1_FAXSCAN			e_FunctionID_ScanFax		= TYPE_PNLGRP1_FAXSCAN_NON;				//機能ID_スキャン/ファクス.
	PNLE_FUNCTIONTYPE_FAXSCAN		e_FunctionBudgeID_ScanFax	= PNLE_FUNCTIONTYPE_FAXSCAN_MAX;			//機能バッヂID_スキャン/ファクス.
	PLayerControl*			p_PopupInstance[PNLD_ReservePopupMaxNum];	//起動するポップアップのインスタンス1～3.
	PBasicMessageProc*		p_SrcInstance[PNLD_ReservePopupMaxNum];		//起動元画面のインスタンス1～3.
	memset( p_PopupInstance, NULL, sizeof( p_PopupInstance ) );
	memset( p_SrcInstance, NULL, sizeof( p_SrcInstance ) );
	short					ss_SrcPosX = 360;							//<! 起動元（主に起動ボタン）の位置.
	short					ss_SrcPosY = 216;							//<! 起動元（主に起動ボタン）の位置.
	short					ss_SrcSizeX = 0;							//<! 起動元（主に起動ボタン）のサイズ.
	short					ss_SrcSizeY = 0;							//<! 起動元（主に起動ボタン）のサイズ.

	PNLC_FunctionBadgeLib* pc_FunctionBadgeLib = PNLC_FunctionBadgeLib::newInstance();
	if( NULL == pc_FunctionBadgeLib ) {	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	return False; }

	Bool bl_Other = False;
	for( uchar uc_other = 0; uc_other < uc_ShortCutOtherInfoTableSize; uc_other++ ) {
		// ショートカット遷移先情報テーブルとIDが同じ場合、ショートカット画面遷移情報を初期化する.
// comment 2015/09/01 OPS1-Sky稲井[特ROM][MZV-104_機種マージ]認証の無い環境下での操作者自身宛スキャンの実現要望対応. 
//		if( e_ShortCutPopupID == st_ShortCutOtherInfoTable[uc_other].e_ShortCutID ) {
//			p_PopupInstance[0] = st_ShortCutOtherInfoTable[uc_other].p_PopupInstance1;		//起動するポップアップのインスタンス1.;
//			p_PopupInstance[1] = NULL;
//			p_PopupInstance[2] = NULL;
//			p_SrcInstance[0]   = PBasicMessageProc::nowGamen;
//			p_SrcInstance[1]   = (PBasicMessageProc*)p_PopupInstance[0];
//			p_SrcInstance[2]   = (PBasicMessageProc*)p_PopupInstance[1];
//			bl_Other = True;
//			break;
//		}
		if( e_ShortCutPopupID == st_ShortCutOtherInfoTable[uc_other].e_ShortCutID ) {
			if((True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW226 , 2 ))
				&&(PNLE_ShortCutID_FAXSCAN_ProgramCall == e_ShortCutPopupID)){ 
				//プログラム宛先に遷移する場合、プログラム宛先のデータ検索を実施させる為、ポップアップ設定を実施しない。
				//代わりに検索処理を実施させる。引数は以降の処理では見ていないので、0としている。
				PNLC_ScanFaxDataKeyControlLib::newInstance()->procJobMemory(0);
				// 以降の処理でp_PopupInstance=NULLのチェックにてreturnで終わり、予約解除処理がない為、
				// ショートカット予約を解除する契機をここで行う.
				setReserveShortCut( PNLE_ShortCutID_NON, PNLE_ShortCutKindID_NON );
				break;
			}else{
				p_PopupInstance[0] = st_ShortCutOtherInfoTable[uc_other].p_PopupInstance1;		//起動するポップアップのインスタンス1.;
				p_PopupInstance[1] = NULL;
				p_PopupInstance[2] = NULL;
				p_SrcInstance[0]   = PBasicMessageProc::nowGamen;
				p_SrcInstance[1]   = ( PBasicMessageProc* )p_PopupInstance[0];
				p_SrcInstance[2]   = ( PBasicMessageProc* )p_PopupInstance[1];
				bl_Other = True;
				break;
			}
		}
	}
// comment 2017/02/15 [HCLT-Jagadeesh] [IT5_4.0][AR1009797] Moved the topmenuManager instance creation place from bottom for supporting island window screens.
	PNLC_TopMenuManager* pc_TopMenuManager = PNLC_TopMenuManager::newInstance();

	if( False == bl_Other ) {	// 特殊処理でない場合.
		if( PNLE_ShortCutKindID_Copy == e_ShortCutPopupKindID ) {	// コピー.
			for( uchar uc_copy = 0; uc_copy < uc_ShortCutInfoCopyTableSize; uc_copy++ ) {
				// ショートカット情報テーブル_コピーとIDが同じ場合、ショートカット画面遷移情報を初期化する.
				if( e_ShortCutPopupID == st_ShortCutInfoCopyTable[uc_copy].e_ShortCutID ) {
					e_FunctionID_Copy      = st_ShortCutInfoCopyTable[uc_copy].e_FunctionID;
					e_FunctionBudgeID_Copy = st_ShortCutInfoCopyTable[uc_copy].e_FunctionBudgeID;
					if( PNLE_FUNCTIONTYPE_COPY_MAX == e_FunctionBudgeID_Copy ) {
						p_PopupInstance[0] = NULL;
					} else {
// comment 2017/02/15 [HCLT-Jagadeesh] [IT5_4.0][AR1009797] Launch Island window popup instance if available separately.	
//						p_PopupInstance[0] = pc_FunctionBadgeLib->getPopupInstance( PNLE_APPTYPE_COPY, e_FunctionBudgeID_Copy );
						if( (NULL != pc_TopMenuManager) && (False == pc_TopMenuManager->isBasicFunctionCopy( e_FunctionID_Copy ) )) {
							p_PopupInstance[0] = pc_FunctionBadgeLib->getIslandPopupInstance( PNLE_APPTYPE_COPY, e_FunctionBudgeID_Copy );
						}
						if( NULL == p_PopupInstance[0] ) {
							p_PopupInstance[0] = pc_FunctionBadgeLib->getPopupInstance( PNLE_APPTYPE_COPY, e_FunctionBudgeID_Copy );
						}
					}
					p_PopupInstance[1] = st_ShortCutInfoCopyTable[uc_copy].p_PopupInstance2;
					p_PopupInstance[2] = st_ShortCutInfoCopyTable[uc_copy].p_PopupInstance3;
					p_SrcInstance[0]   = PBasicMessageProc::nowGamen;
					p_SrcInstance[1]   = ( PBasicMessageProc* )p_PopupInstance[0];
					p_SrcInstance[2]   = ( PBasicMessageProc* )p_PopupInstance[1];
					break;
				}
			}
		} else if( PNLE_ShortCutKindID_ScanFax == e_ShortCutPopupKindID ) {	// スキャン/ファクス.
			for( uchar uc_scanfax = 0; uc_scanfax < uc_ShortCutInfoScanFaxTableSize; uc_scanfax++ ) {
				// ショートカット情報テーブル_スキャン/ファクスとIDが同じ場合、ショートカット画面遷移情報を初期化する.
				if( e_ShortCutPopupID == st_ShortCutInfoScanFaxTable[uc_scanfax].e_ShortCutID ) {
					e_FunctionID_ScanFax      = st_ShortCutInfoScanFaxTable[uc_scanfax].e_FunctionID;
					e_FunctionBudgeID_ScanFax = st_ShortCutInfoScanFaxTable[uc_scanfax].e_FunctionBudgeID;
					if( PNLE_FUNCTIONTYPE_FAXSCAN_MAX == e_FunctionBudgeID_ScanFax ) {
						p_PopupInstance[0] = NULL;
					} else {
// comment 2017/02/15 [HCLT-Jagadeesh] [IT5_4.0][AR1009797] Launch Island window popup instance if available separately.	
//						p_PopupInstance[0] = pc_FunctionBadgeLib->getPopupInstance( PNLE_APPTYPE_FAXSCAN, e_FunctionBudgeID_ScanFax );
						if( (NULL != pc_TopMenuManager) && (False == pc_TopMenuManager->isBasicFunction( e_FunctionID_ScanFax ) )) {
							p_PopupInstance[0] = pc_FunctionBadgeLib->getIslandPopupInstance( PNLE_APPTYPE_FAXSCAN, e_FunctionBudgeID_ScanFax ); //To fetch Island window popup.
						}
						if( NULL == p_PopupInstance[0] ) {
							p_PopupInstance[0] = pc_FunctionBadgeLib->getPopupInstance( PNLE_APPTYPE_FAXSCAN, e_FunctionBudgeID_ScanFax );
						}
					}
					p_PopupInstance[1] = st_ShortCutInfoScanFaxTable[uc_scanfax].p_PopupInstance2;
					p_PopupInstance[2] = st_ShortCutInfoScanFaxTable[uc_scanfax].p_PopupInstance3;
					p_SrcInstance[0]   = PBasicMessageProc::nowGamen;
					p_SrcInstance[1]   = ( PBasicMessageProc* )p_PopupInstance[0];
					p_SrcInstance[2]   = ( PBasicMessageProc* )p_PopupInstance[1];
					break;
				}
			}
		} else if( PNLE_ShortCutKindID_Box == e_ShortCutPopupKindID ) {		// ボックス.
			for( uchar uc_box = 0; uc_box < uc_ShortCutInfoBoxTableSize; uc_box++ ) {
				// ショートカット情報テーブル_ボックスとIDが同じ場合、ショートカット画面遷移情報を初期化する.
				if( e_ShortCutPopupID == st_ShortCutInfoBoxTable[uc_box].e_ShortCutID ) {
					p_PopupInstance[0] = getBoxShortCutPopupInstance( uc_box );
					p_PopupInstance[1] = NULL;
					p_PopupInstance[2] = NULL;
					p_SrcInstance[0]   = PBasicMessageProc::nowGamen;
					p_SrcInstance[1]   = ( PBasicMessageProc* )p_PopupInstance[0];
					p_SrcInstance[2]   = ( PBasicMessageProc* )p_PopupInstance[1];
					break;
				}
			}
		} else if( PNLE_ShortCutKindID_Job == e_ShortCutPopupKindID ) {		// ジョブ.
			for( uchar uc_job = 0; uc_job < uc_ShortCutInfoJobTableSize; uc_job++ ) {
				// ショートカット情報テーブル_ボックスとIDが同じ場合、ショートカット画面遷移情報を初期化する.
				if( e_ShortCutPopupID == st_ShortCutInfoJobTable[uc_job].e_ShortCutID ) {
					p_PopupInstance[0] = st_ShortCutInfoJobTable[uc_job].p_PopupInstance1;
					if( PNLE_ShortCutID_JOB_JobListHistoryList == st_ShortCutInfoJobTable[uc_job].e_ShortCutID ) {
						PNLC_JobListLib::getInstance()->setStartSelectedTab( PNLC_StartSelectecTabHistory );	// タブの初期設定を履歴リストに設定.
					}
					p_PopupInstance[1] = NULL;
					p_PopupInstance[2] = NULL;
					p_SrcInstance[0]   = PBasicMessageProc::nowGamen;
					p_SrcInstance[1]   = ( PBasicMessageProc* )p_PopupInstance[0];
					p_SrcInstance[2]   = ( PBasicMessageProc* )p_PopupInstance[1];
					break;
				}
			}
		} else if( PNLE_ShortCutKindID_Utility == e_ShortCutPopupKindID ) {	// ユーティリティ.
			// ユーティリティは、現状、ポップアップなし.
		} else if( PNLE_ShortCutKindID_Auth == e_ShortCutPopupKindID ) {	// 認証.
			for( uchar uc_Auth = 0; uc_ShortCutInfoAuthTableSize > uc_Auth; ++uc_Auth ) {
				// ショートカット情報テーブル_認証とIDが同じ場合、ショートカット画面遷移情報を初期化する.
				if( e_ShortCutPopupID == st_ShortCutInfoAuthTable[uc_Auth].e_ShortCutID ) {
					p_PopupInstance[0] = st_ShortCutInfoAuthTable[uc_Auth].p_PopupInstance1;
					p_PopupInstance[1] = NULL;
					p_PopupInstance[2] = NULL;
					p_SrcInstance[0]   = PBasicMessageProc::nowGamen;
					p_SrcInstance[1]   = ( PBasicMessageProc* )p_PopupInstance[0];
					p_SrcInstance[2]   = ( PBasicMessageProc* )p_PopupInstance[1];
					break;
				}
			}
		}
	}
// comment 2017/02/15 [HCLT-Jagadeesh] [IT5_4.0][AR1009797] Moved the topmenuManager instance creation to above for supporting island window screens.
//	PNLC_TopMenuManager* pc_TopMenuManager = PNLC_TopMenuManager::newInstance();
//	if( NULL == pc_TopMenuManager ) {	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	return False; }

	PPopupCom* p_PopupGamen = dynamic_cast< PPopupCom* >( p_PopupInstance[0] );
	if( NULL == p_PopupGamen ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return False; }

	// コピー.
	if( ( PNLE_ShortCutKindID_Copy == e_ShortCutPopupKindID ) && ( PNLE_FUNCTIONTYPE_COPY_MAX != e_FunctionBudgeID_Copy ) ) {
// comment 2017/02/15 [HCLT-Jagadeesh] [IT5_4.0][AR1009797] Added null check condition.
//		if( True == pc_TopMenuManager->isBasicFunctionCopy( e_FunctionID_Copy ) ) {	// ポップアップする画面が、基本設定に割り当てられている場合.
		if((NULL != pc_TopMenuManager) && (True == pc_TopMenuManager->isBasicFunctionCopy( e_FunctionID_Copy ) )) {	// ポップアップする画面が、基本設定に割り当てられている場合.
			// 機能タイプ,起動元設定処理.

			pc_FunctionBadgeLib->setFunctionBudgeType( PNLE_APPTYPE_COPY, e_FunctionBudgeID_Copy, True );

			pc_FunctionBadgeLib->initBasicFunctionBadgeFromTopMenuCopy( e_FunctionID_Copy );

			// 機能バッジ表示位置問い合わせ.
			PNLS_Rect s_Rect = { 0, 0, 0, 0 };
			pc_TopMenuManager->getFunctionPosCopyBasic( e_FunctionID_Copy, &s_Rect );
			ss_SrcPosX = s_Rect.ss_PosX;
			ss_SrcPosY = s_Rect.ss_PosY;
			ss_SrcSizeX = s_Rect.ss_Width;
			ss_SrcSizeY = s_Rect.ss_Height;

			// ポップアップ機能バッジ更新処理.
			p_PopupGamen->setFunctionBadgeShowFlg( True );
		} else {	// 応用設定の場合.
			// 機能バッジタイプ設定.
			pc_FunctionBadgeLib->setFunctionBudgeType( PNLE_APPTYPE_COPY, e_FunctionBudgeID_Copy, True );
			// 機能バッヂ非表示フラグ設定.
			p_PopupGamen->setFunctionBadgeShowFlg( False );
		}
	}

	// スキャン/ファクス.
	if( ( PNLE_ShortCutKindID_ScanFax == e_ShortCutPopupKindID ) && ( PNLE_FUNCTIONTYPE_FAXSCAN_MAX != e_FunctionBudgeID_ScanFax ) ) {
// comment 2017/02/15 [HCLT-Jagadeesh] [IT5_4.0][AR1009797] Added null check condition.
//		if( True == pc_TopMenuManager->isBasicFunction( e_FunctionID_ScanFax ) ) {	// ポップアップする画面が、基本設定に割り当てられている場合.
		if( (NULL != pc_TopMenuManager) && (True == pc_TopMenuManager->isBasicFunction( e_FunctionID_ScanFax ) )) {	// ポップアップする画面が、基本設定に割り当てられている場合.

			// 機能タイプ,起動元設定処理.
			pc_FunctionBadgeLib->setFunctionBudgeType( PNLE_APPTYPE_FAXSCAN, e_FunctionBudgeID_ScanFax, True );

			pc_FunctionBadgeLib->initBasicFunctionBadgeFromTopMenuScanFax( e_FunctionID_ScanFax );

			// 機能バッジ表示位置問い合わせ.
			PNLS_Rect s_Rect = { 0, 0, 0, 0 };
			pc_TopMenuManager->getFunctionPosScanFaxBasic( e_FunctionID_ScanFax, &s_Rect );
			ss_SrcPosX = s_Rect.ss_PosX;
			ss_SrcPosY = s_Rect.ss_PosY;
			ss_SrcSizeX = s_Rect.ss_Width;
			ss_SrcSizeY = s_Rect.ss_Height;

			// 機能バッヂ表示フラグ設定.
			p_PopupGamen->setFunctionBadgeShowFlg( True );
		} else {	// 応用設定の場合.
			// 機能バッジタイプ設定.
			pc_FunctionBadgeLib->setFunctionBudgeType( PNLE_APPTYPE_FAXSCAN, e_FunctionBudgeID_ScanFax, True );
			// 機能バッヂ非表示フラグ設定.
			p_PopupGamen->setFunctionBadgeShowFlg( False );
		}
	}

	Bool bl_result = False;									// ポップアップ起動要求.
	uchar uc_PopupNum = PNLD_ReservePopupMaxNum;
	// 1つ目のポップアップインスタンスがない場合は、処理を抜ける.
	if( NULL == p_PopupInstance[0] ) { return False; }
	// ポップアップの階層を取得する.
	else if( NULL == p_PopupInstance[1] ) { uc_PopupNum = 1; }
	else if( NULL == p_PopupInstance[2] ) { uc_PopupNum = 2; }

	for( uchar k = 0; k < uc_PopupNum; k++ ) {
		// ポップアップするインスタンスを決定する.
		// ポップアップ起動要求を行う.
		bl_result = addPopup(
		                p_PopupInstance[k],				// 起動するポップアップのインスタンス.
		                p_SrcInstance[k],				// 起動元画面（ポップアップ）のインスタンス.
		                NULL,							// コールバック関数ポインタ.
		                ss_SrcPosX,						// 起動元（主に起動ボタン）の位置.
		                ss_SrcPosY,						// 起動元（主に起動ボタン）の位置.
		                ss_SrcSizeX,					// 起動元（主に起動ボタン）のサイズ.
		                ss_SrcSizeY,					// 起動元（主に起動ボタン）のサイズ.
		                True							// 起動アニメーション強制OFFフラグ（True：OFFする、False：OFFしない）.
		            );

	}

	// ショートカット予約を解除する.
	setReserveShortCut( PNLE_ShortCutID_NON, PNLE_ShortCutKindID_NON );

	return bl_result;
}

// ===============================================================================================
/**
 *	@brief		起動可能な事が確実なポップアップの起動要求（PNLC_PopupListControl::addPopupの起動可否判定後の処理を関数化）.
 *	@param[in]	p_PopupInstance										起動するポップアップのインスタンス.
 *	@param[in]	p_SrcInstance										起動元画面（ポップアップ）のインスタンス.
 *	@param[in]	( *p_CallbackFunc )( uchar, short, void*, uint32_t )	コールバック関数ポインタ.
 *	@param[in]	ss_SrcPosX											起動元（主に起動ボタン）の位置.
 *	@param[in]	ss_SrcPosY											起動元（主に起動ボタン）の位置.
 *	@param[in]	ss_SrcSizeX											起動元（主に起動ボタン）のサイズ.
 *	@param[in]	ss_SrcSizeY											起動元（主に起動ボタン）のサイズ.
 *	@param[in]	bl_AnimLess											起動アニメーション強制OFFフラグ（True：OFFする、False：OFFしない）.
 *	@param[in]	b_ForcedChange										アニメーション中の画面・起動予約の画面を強制的に表示完了させるか否か（True：表示完了させる、False：表示完了させない）.
 */
// ===============================================================================================
void PNLC_PopupListControl::addFixedPopup(
    PLayerControl* p_PopupInstance,
    PBasicMessageProc* p_SrcInstance,
    void ( *p_CallbackFunc )( uchar, short, void*, uint32_t ),
    short ss_SrcPosX,
    short ss_SrcPosY,
    short ss_SrcSizeX,
    short ss_SrcSizeY,
    Bool  bl_AnimLess,
    Bool  b_ForcedChange )
{
	// パラメータチェック.
	PNLC_StackManJobSpec* pc_StackManJobSpec = PNLC_StackManJobSpec::newInstance();
	PNLC_OperationLog* pc_OperationLog = PNLC_OperationLog::newInstance();
	if( ( NULL == p_PopupInstance ) || ( NULL == pc_StackManJobSpec ) || ( NULL == pc_OperationLog ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	if( NULL != p_PopupTable[( uchar )c_PopupNum ] ) {
		m_pc_nextPopupInstance = p_PopupInstance;
		p_PopupTable[( uchar )c_PopupNum ]->procInactivate();
		m_pc_nextPopupInstance = NULL;
	}

	++c_PopupNum;																	// ポップアップ表示数をインクリメント.

	p_PopupTable[( uchar )c_PopupNum ] = p_PopupInstance;									// 指定されたインスタンスをポップアップ管理リストに追加.

	if( NULL == p_PopupTable[( uchar )c_PopupNum ] ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}

	p_PopupTable[( uchar )c_PopupNum ]->updateFuncBadgeButtonStatus();						// ポップアップアニメーション前に機能バッジのアイコンステータスを更新する.

	pc_StackManJobSpec->saveJobSpec( c_PopupNum );									// ジョブスペックを保存(ポップアップ中止時に設定を戻すため).

	p_PopupTable[( uchar )c_PopupNum ]->setLayerNumber( c_PopupNum );						// 追加したポップアップに、確定した階層情報を設定.

	// 表示するポップアップウインドウ背景サイズの変更.
	p_PopupInstance->resizePopup();

// comment 2018/10/12 Sky東島 [1062683] 2ペイン化対応.
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
	// 3ペイン画面を表示解像度合わせた表示座標補正させる.
	Bool bl_changePane = changePopupPositionFrom3Pane( p_PopupInstance );
	if( True == bl_changePane ){
		// 画面キャプチャの生成(ポップアップ表示前).
		createCaptureBeforeOpenPopup( p_PopupInstance );
	}
#endif // #if !defined(DEF_SparrowAIOSFPCus)	// Eagle/SparrowMFP 10.1inch.

	p_SrcGamen = p_SrcInstance;														// 起動元画面インスタンスを保持しておく（起動完了通知呼び出し用）.

	// 追加したポップアップの起動処理を呼び出す.
	p_PopupTable[( uchar )c_PopupNum ]->opened( p_CallbackFunc, ss_SrcPosX, ss_SrcPosY, ss_SrcSizeX, ss_SrcSizeY, bl_AnimLess, b_ForcedChange );

// comment 2020/11/25 Sky Kawanishi [LocalUI特ROM] SegF(スタックオーバーフロー)時ログ改善.
	LUIC_DbgTsk::newInstance()->setPopupList((void**)&(p_PopupTable[0]), c_PopupNum + 1);
	// 画面変更によるログの収集.
	pc_OperationLog->setLogChangePanel( p_PopupTable[( uchar )c_PopupNum ] );

	p_PopupInstance_BAK = p_PopupInstance;											// 現在のインスタンスを保持.
}

/**
 * @brief	言語一時変更フラグ設定.
 * @param[in]	option				言語一時変更フラグ.
 * @note		表示中の全レイヤーにフラグを設定する.
 **/
void PNLC_PopupListControl::setMomentLanguage( Bool option )
{
	short ss_PopupTablNo = 0;
	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		PLayerControl* pc_Gamen = dynamic_cast<PLayerControl*>( p_PopupTable[ss_PopupTablNo] );
		if( NULL != pc_Gamen ) {
			pc_Gamen->setMomentLanguage( option );
		}
	}
}

/**
 * @brief	Language update after Maintenance Mode end.
 * @param[in]	option				言語一時変更フラグ.
 * @note		表示中の全レイヤーにフラグを設定する.
 **/
void PNLC_PopupListControl::setUserLanguage( Bool option )
{
	short ss_PopupTablNo = 0;
	for( ss_PopupTablNo = c_PopupNum; 0 <= ss_PopupTablNo; ss_PopupTablNo-- ) {
		PLayerControl* pc_Gamen = dynamic_cast<PLayerControl*>( p_PopupTable[ss_PopupTablNo] );
		if( NULL != pc_Gamen ) {
			pc_Gamen->setUserLanguage( option );
		}
	}
}

// ショートカット遷移先ポップアップを条件によって変更する(BOX用).
// 引数 	uchar			:st_ShortCutInfoBoxTableのインデックス番号.
// 戻り値	PLayerControl*	:ポップアップ画面インスタンス.
PLayerControl* PNLC_PopupListControl::getBoxShortCutPopupInstance( uchar uc_TableIndex ) {
	if( uc_TableIndex >= uc_ShortCutInfoBoxTableSize ) {	// テーブルサイズチェック.呼び出し元で確認しているはずだが、念のため.
		return NULL;
	}
	PLayerControl* pc_RetPopupInstance
	    = st_ShortCutInfoBoxTable[uc_TableIndex].p_PopupInstance1;	// 初期値はテーブルの要素から取得する.

	switch( st_ShortCutInfoBoxTable[uc_TableIndex].e_ShortCutID ) {			//ショートカット識別ID.
		case PNLE_ShortCutID_BOX_DocSecretDataBeforePassPrint:				// セキュリティー文書ボックス.
		{
// comment 2016/08/23 SCD2-2G Sky松下 [IT5_4.0][RQ-563] 簡単セキュアプリント対応.
			// IT5_4.0 以降.
// Comment 2017/11/29 【HCLT-Deeksha】【Eagle】【制御構成変更_No.4】 Removing old function version check condition and else portion.
// comment 2017/01/12 SCD2-2G Sky谷光 [IT5_4.0][1005011].
			//if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( 78, 4 )
			// || True == APIC_SystemSetting::newInstance()->getSimpleSecurePrint() ) {
			if( ( (True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( 78, 4 )) 					// DipSW 78 bit4.
					&& (TYPE_SecurePrintCertification1 == PnlComGetSystemSetting()->getSecurePrintCertification()) )		// 機密文書ｱｸｾｽ方式 モード1.	// 特ROM:ON、または簡単セキュアプリント:ONの場合は M056300400 セキュリティーID一覧画面へ遷移.
//Comment 2017/09/04 [HCL-Swathi] [IT5 4.2][RQ-686] [WP732]Multiple Network support.					
//				|| (True == APIC_SystemSetting::newInstance()->getSimpleSecurePrint()) ) {								// セキュリティー文書設定「簡単印刷する」.	//c_SecretDocIDListInfo.setSecretDocIDListInfoForAllSearch();	// 全検索用パラメータ設定処理.
				|| (True == APIC_SystemSetting::newInstance()->getSimpleSecurePrint( PNLC_NetworkLan::getInstance()->getLine() )) ) {								// セキュリティー文書設定「簡単印刷する」.	//c_SecretDocIDListInfo.setSecretDocIDListInfoForAllSearch();	// 全検索用パラメータ設定処理.
				// 特ROM:ON、または簡単セキュアプリント:ONの場合は M056300400 セキュリティーID一覧画面へ遷移.
				//c_SecretDocIDListInfo.setSecretDocIDListInfoForAllSearch();	// 全検索用パラメータ設定処理.
				//c_SecretDocIDListInfo.updateSecretIDList();					// リスト更新要求.
				pc_RetPopupInstance = &gp_M056300400;
			}
		}
			break;
		default: break;
	}
	return pc_RetPopupInstance;
}
// ===============================================================================================
/**
 *	@brief		送り先レイヤー番号の選択.
 *  @作成背景	レイヤー階層の設定において、重複したインスタンスを設定することができてしまう状態に対し、
 *				下位のレイヤーへ処理を渡す際に自身または一度処理が行われたレイヤーを指定しないよう、
 *				適切なレイヤー番号を渡す関数を作成.
 *	@処理内容	引数(送り元レイヤー番号)から下位のレイヤーに対して、送り元と同じインスタンスではないかを確認.
 *				同じインスタンスではない場合、送り元より上位にも同じインスタンスがないかを確認し、
 *				なければ送り先レイヤーとしてレイヤー番号を返す.
 *	@param[in]	c_LayerNum			送り元のレイヤー番号.
 *	@return		送り先レイヤー番号
 */
// ===============================================================================================
char PNLC_PopupListControl::getLayerchoice( char c_LayerNum )
{
	if( 0 == c_LayerNum ) {
		return c_LayerNum;		//最下位レイヤーなら、そのまま返す.
	}

	char sc_ChoiceLayerNum = 0;	// 送り先レイヤー番号.
	short ss_PopupTablNo = 0;	// 最上位レイヤーから送り元までのレイヤー番号比較変数.

	// 送り元レイヤーから下位レイヤーのインスタンスをチェックし、同じでなければと送り先レイヤー候補とする.
	for( sc_ChoiceLayerNum = static_cast<char>( c_LayerNum-1 ); 0 <= sc_ChoiceLayerNum; sc_ChoiceLayerNum-- ) {
		if( p_PopupTable[( uchar )c_LayerNum] != p_PopupTable[( uchar )sc_ChoiceLayerNum] ) {								// インスタンスが違えば、上位をチェック.
			// 最上位レイヤーから送り元レイヤーまでの中に、送り先レイヤーと同じインスタンスがないかをチェックする.
			for( ss_PopupTablNo = c_PopupNum; c_LayerNum <= ss_PopupTablNo; ss_PopupTablNo-- ) {
				if( p_PopupTable[( uchar )sc_ChoiceLayerNum] == p_PopupTable[ss_PopupTablNo] ) {					// 上位ですでに同じインスタンスがないか.
					break;																				// あれば重複と判断しチェック処理を抜ける.
				}
				if( ss_PopupTablNo == c_LayerNum ) {				// 同じインスタンスがなければ.
					return sc_ChoiceLayerNum;						// 送り先のレイヤー番号を確定.
				}
			}
		}
	}
	return 0;		// ベースを通知する. ここにくることはありえません.
}

// ===============================================================================================.
//	画面キャプチャWidgetインスタンス取得.
// ===============================================================================================.
PBaseWidget* PNLC_PopupListControl::getCapturePopupImage( Bool b_popupStatus )
{
	PBaseWidget*	widget	= 0;

	unsigned char	popNum	= getPopupNum();

	if( 0 >= popNum ) {
		return 0;
	}

	if( True == b_popupStatus ) {	//ポップアップopen.
		if( 1 == popNum ) {
			//ポップアップが1枚の場合、ベース画面Widgetを返す.
			widget	= dynamic_cast<PBaseWidget*>( getBaseGamen()->getDisplayParts() );
		}
		else if( 2 == popNum ) {
			//ポップアップが2枚の場合、1枚目の画面キャプチャWidgetを返す.
			widget	= pc_CapturePopupBaseTable[0];
		}
		else {
			//以降はPNLD_CAPTURE_POPUP_MNG_NUM_MAXごとに次の画面キャプチャWidgetを返す.
			widget	= pc_CapturePopupBaseTable[( PNLD_CAPTURE_POPUP_MNG_NUM_MAX + popNum - 3 ) / PNLD_CAPTURE_POPUP_MNG_NUM_MAX];
		}
	}
	else {	//close.
		if( 1 == popNum ) {
			//ポップアップが1枚の場合、1枚目の画面キャプチャWidgetを返す.
			widget	= pc_CapturePopupBaseTable[0];
		}
		else {
			//以降はPNLD_CAPTURE_POPUP_MNG_NUM_MAXごとに次の画面キャプチャWidgetを返す.
			widget	= pc_CapturePopupBaseTable[( ( popNum - 2 ) / PNLD_CAPTURE_POPUP_MNG_NUM_MAX ) + 1];
		}
	}

	return( widget );
}

// ===============================================================================================.
//	画面キャプチャイメージの生成.
// ===============================================================================================.
void PNLC_PopupListControl::createCapturePopupImage( Bool b_popupStatus )
{
	if ( b_StartAnimation == True ) {
		// アニメーション開始要求中は、再要求.
		PCGC_PanelMain::createCapturePopupImageNotice( (b_popupStatus == True) ? PNLD_CreateCaptureType_Open : PNLD_CreateCaptureType_Close );
		return;
	}

	releaseCapturePopupImage();

	char	popNum	= getPopupNum();
	PBasicMessageProc*	base	= getBaseGamen();
	Bool isKeyBoard;
	PDisplayParts*	baseParts	= base->getDisplayParts();
	if( 0 == baseParts ) {
		// ベース画面WidgetがNULLの場合、ポップアップWidgetの親が存在しないため処理終了.
		return;
	}

	PGrayoutWidget*	gray	= base->getGrayoutParts();
	if( 0 == gray ) {
		// グレー画像Widgetが存在しない場合、ポップアップ表示が行われていないため処理終了.
		return;
	}

	PBasicMessageProc*	pop	= getPopupInstance( popNum );
	if( 0 == pop ) {
		// ポップアップインスタンスがNULLの場合、表示するポップアップが存在しないと判断し処理終了.
		return;
	}

	PBaseWidget*	popParts	= dynamic_cast<PBaseWidget*>( pop->getDisplayParts() );
	if( 0 == popParts ) {
		// ポップアップWidgetが存在しない場合、表示するポップアップが存在しないと判断し処理終了.
		return;
	}

	//グレーWidgetをキャプチャに含まないように非表示にしておく.
	gray->hide();
	
	if ( True == b_popupStatus ) {	//ポップアップopen.
		if ( 0 == popNum ) {
			// ポップアップ0枚の場合、キャプチャ処理不要のため何もしない.
		}
		else if ( 1 == popNum ) {
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
			base->updateLcdPane( base );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			//ポップアップ1枚の場合、ベースWidgetをキャプチャ.
// comment 2018/04/18 SCD2 Kazuo.Takei [Eagle/Sparrow][制御構成変更_No.111 P07_01_共通仕様(第6世代UI画面_左エリア)].
			pc_CapturePopupBaseTable[0]->createCaptureImage( baseParts, False, True );
// commemt 2022/10/11 FPT.YenNTH8 [IT6_2.3][ALL MACHINE][OP_BTS-38739] Don't hide first capture。
//			baseParts->hide();
			PBaseWidget* baseWidget	= dynamic_cast<PBaseWidget*>( baseParts );
			if( baseWidget ) {
				baseWidget->clearCache();
			}
			pc_CapturePopupBaseTable[0]->show();
			
			//グレーWidgetを表示.
// comment 2023/01/27 Sky東島 [IT6_2.3][OP_BTS-40908].
//			gray->show();
			if( True == pop->getBackLayerDarkness() ) {
				// ポップアップがグレー背景を表示する指定の場合はグレーWidgetを表示する.
				gray->show();
			}

			//ポップアップを表示.
			if ( 0 != popParts ) {
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
				pop->updateLcdPane( pop );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
				popParts->show();
			}
		}
		else {
			//以降はPNLD_CAPTURE_POPUP_MNG_NUM_MAX毎にキャプチャ切り替え.
			unsigned char	widgetIdx	= ( PNLD_CAPTURE_POPUP_MNG_NUM_MAX + popNum - 2 ) / PNLD_CAPTURE_POPUP_MNG_NUM_MAX;

			//キャプチャ必要なポップアップの親をキャプチャするWidgetに変更.
			int	i;
			for( i = ( widgetIdx - 1 ) * PNLD_CAPTURE_POPUP_MNG_NUM_MAX + 1; i < popNum; i++ ) {
// Comment 2019/03/22 [Hcl-Sam Sundar] [OP_BTS-4061] - Adding Assert log for improving backtrace information.
				if( widgetIdx > PNLD_CAPTURE_POPUP_NUM_MAX || widgetIdx < 0 ) {
					LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
				}
				isKeyBoard = False;
				PBasicMessageProc*	pop	= getPopupInstance( (char)i );
				if( 0 == pop ) {
					// 表示されているはずのポップアップ画面のインスタンスが取得できていない.
					LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
					break;
				}

// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
				pop->updateLcdPane( pop );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.

				unsigned char ucInsntanceCnt;
				for( ucInsntanceCnt = 0; ucInsntanceCnt < ( sizeof( pc_KeyBoardInstance ) / sizeof( PLayerControl* ) ); ucInsntanceCnt++ ) {
					if( pop == pc_KeyBoardInstance[ucInsntanceCnt] ) {
						isKeyBoard = True;
						break;
					}
				}

				if( isKeyBoard == True ) {
					if( i == popNum - 1 ) {
						// キーボード画面を含む画面のキャプチャ生成(初回).
						if( 0 == pc_CapturePopupKeyBoard ) {
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
// comment 2018/10/12 Sky東島 [1062683] 2ペイン化対応.
//							if ( GFWE_LCD_PANE_1 == pop->getGamenPane( pop ) ) {
							GFWE_LCD_PANE e_LCDPane = pop->getGamenPane( pop );
							if( ( GFWE_LCD_PANE_1 == e_LCDPane )
							 || ( GFWE_LCD_PANE_2 == e_LCDPane )
							 || ( GFWE_LCD_PANE_1_Extention == e_LCDPane ) ) {
								// １ペイン,2ペイン,1ペイン(拡大表示)の場合.
								pc_CapturePopupKeyBoard	= new PBaseWidget(
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
							} else {
								// １ペイン以外の場合.
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
							pc_CapturePopupKeyBoard	= new PBaseWidget( c71_000_000_t1_X800_Y480, 0, 0, 0, TYPE_LayoutFree, False );
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
							}
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
							pc_CapturePopupKeyBoard->startGUI( pc_CapturePopupBaseTable[widgetIdx - 1] );
						}
// comment 2018/04/18 SCD2 Kazuo.Takei [Eagle/Sparrow][制御構成変更_No.111 P07_01_共通仕様(第6世代UI画面_左エリア)].
						pc_CapturePopupKeyBoard->createCaptureImage( dynamic_cast<PBaseWidget*>( pop->getDisplayParts() ), False, True );
						pc_CapturePopupKeyBoard->show();
						// ポップアップインスタンスの親を一時的に違うWidgetに設定(ベース画面).
						PBaseWidget* popParts = dynamic_cast<PBaseWidget*>( pop->getDisplayParts() );
						popParts->setParentParts( dynamic_cast<PBaseWidget*>( getBaseGamen()->getDisplayParts() ) );
						popParts->hide();
						popParts->clearCache();
						
					} else {
// comment 2016/1/28 MSE海田 [ZeusS_ZX0][CQ523861] NULLチェックを追加.
						if( 0 != pc_CapturePopupKeyBoard ) {
							// キーボード画面を含む画面のキャプチャ親子関係更新.
							pc_CapturePopupKeyBoard->setParentParts( pc_CapturePopupBaseTable[widgetIdx - 1] );
							pc_CapturePopupKeyBoard->show();
						}
					}
				} else {
					if( False == pop->getBackLayerDarkness() ) {
						// キーボードを含まない画面については管理されているインスタンスを使用して親子関係更新.
						PDisplayParts*	popParts	= pop->getDisplayParts();
						if( 0 != popParts ) {
							popParts->setParentParts( pc_CapturePopupBaseTable[widgetIdx - 1] );
							popParts->show();
						}
						else {
							// 表示されているはずのポップアップ画面のポップアップWidgetが取得できていない.
							LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
							break;
						}
					}
				}
			}
// comment 2015/01/13 MSE竹中 [Poseidon][CQ508151] 画面キャプチャWidgetを生成しない条件を追加.
			// 画面キャプチャwidgetが切り替わらないタイミング、またはポップアップ背面にグレーWidgetを置く場合にキャプチャ生成する.
// comment 2023/01/27 Sky東島 [IT6_2.3][OP_BTS-40788].
//			if ( ( 0 != ( ( popNum - 2 ) % PNLD_CAPTURE_POPUP_MNG_NUM_MAX ) )
//				|| ( True == pop->getBackLayerDarkness() ) ) {
			if( ( &gp_WebUIWait_popup != pop )
			 && ( ( 0 != ( ( popNum - 2 ) % PNLD_CAPTURE_POPUP_MNG_NUM_MAX ) )
				|| ( True == pop->getBackLayerDarkness() ) ) ) {
// comment 2016/03/21 OPS1-Sky浜野 [ZeusS_ZX0][CQ526366] .
				// 一つ下のレイヤーのポップアップの親を切り替える.
				PBasicMessageProc*	prevPop	= getPopupInstance( static_cast<char>( popNum - 1 ) );
				if( 0 != prevPop ) {
					PBaseWidget*	prevPopParts	= dynamic_cast<PBaseWidget*>( prevPop->getDisplayParts() );
					if ( 0 != prevPopParts ) {
						prevPopParts->setParentParts( pc_CapturePopupBaseTable[widgetIdx -1] );
					}
				}
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/04/13 【HCL-Rajesh】 [IT6 Eagle]  Klockwork Changes ID: 2546902,2546900 [NPD.CHECK.MUST].
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
				if( NULL != prevPop ) {
					prevPop->updateLcdPane( prevPop );	// ペイン更新
				}
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
// comment 2018/04/18 SCD2 Kazuo.Takei [Eagle/Sparrow][制御構成変更_No.111 P07_01_共通仕様(第6世代UI画面_左エリア)].
// comment 2019/2/12 Sky雜賀[EagleH][OP_BTS-1756].
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
				if( NULL != prevPop ) {
					GFWC_System::getInstance()->setLcdPane( prevPop->getGamenPane( prevPop ) );					// ペイン更新
					PBaseWidget*	prevPopParts	= dynamic_cast<PBaseWidget*>( prevPop->getDisplayParts() );

					// キャプチャーされる一番下のレイヤーのみは再生成し、2枚目、3枚目は前のキャプチャーを保持したまま合成する.
					if ( 0 != prevPopParts ) {
						if ( 0 == ( ( popNum - 2 ) % PNLD_CAPTURE_POPUP_MNG_NUM_MAX ) ) {						// キャプチャーされる一番下のレイヤー( １、４、７、・・・）.
							pc_CapturePopupBaseTable[widgetIdx]->createCaptureImage( pc_CapturePopupBaseTable[widgetIdx - 1], False, True, True );
						} else {																				// キャプチャーの二枚目、三枚目（２、３、５、６、・・・).
// comment 2019/08/20 SCD2-Sky中野 [EagleH][OP_BTS-12365].
							if( False == m_bl_CaptureCreated[ popNum - 2 ] ) {
								PBasicMessageProc*	pc_2prevPop	= getPopupInstance( static_cast<char>( popNum - 2 ) );
								if( NULL != pc_2prevPop ){
									PBaseWidget* pc_2prevPopParts = dynamic_cast<PBaseWidget*>( pc_2prevPop->getDisplayParts() );
									pc_CapturePopupBaseTable[widgetIdx]->createCaptureImage( pc_2prevPopParts, False, True, False );
								}
								m_bl_CaptureCreated[ popNum - 2 ] = True;
							}
							pc_CapturePopupBaseTable[widgetIdx]->createCaptureImage( prevPopParts, False, True, False );
						}
					}
				}
#else																// SparrowAIO/SparrowSFP/DenebMLK 7inch.
				pc_CapturePopupBaseTable[widgetIdx]->createCaptureImage( pc_CapturePopupBaseTable[widgetIdx - 1], False, True );
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
				pc_CapturePopupBaseTable[widgetIdx - 1]->hide();
				pc_CapturePopupBaseTable[widgetIdx - 1]->clearCache();
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
// comment 2018/10/12 【HCL-Nadeem】 [IT6 Eagle]  Klockwork Changes ID: 811005 [NPD.CHECK.MUST].					
				if( NULL != prevPop ) {
					prevPop->updateLcdPane( prevPop );	// ペイン更新
				}
/// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
				pc_CapturePopupBaseTable[widgetIdx]->show();
			}

// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
// comment 2023/01/27 Sky東島 [IT6_2.3][OP_BTS-40788].
			if( &gp_WebUIWait_popup != pop ){
				base->updateLcdPane( base );	// ペイン更新
			}
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			//グレーWidgetを表示.
			gray->show();
			
			if ( False == pop->getBackLayerDarkness() ) {
				// ポップアップ背面にグレーWidgetを置かない場合、キャプチャWidget→グレーWidget→前popWidget→popWidgetとする必要がある.
				// 前popWidgetはhideされているため、showする.
				PBasicMessageProc* prevPop = getPopupInstance( static_cast<char>( popNum - 1 ) );
// comment 2014/12/08 【HCL-Rajeev】【CQ-506924】  Klockwork Changes ID:16011 [NPD.FUNC.MUST].
				if( NULL != prevPop ) {
					PBaseWidget* prevPopParts  = dynamic_cast<PBaseWidget*>( prevPop->getDisplayParts() );
					
					if ( 0 != prevPopParts ) {
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
					prevPop->updateLcdPane( prevPop );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
					prevPopParts->show();
				}
			}
			}

			//ポップアップを表示.
			if ( 0 != popParts ) {
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
				pop->updateLcdPane( pop );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
				popParts->show();
			}
		}
// comment 2019/08/20 SCD2-Sky中野 [EagleH][OP_BTS-12365].
		if( 0 < popNum ) {
			m_bl_CaptureCreated[ popNum - 1 ] = True;
		}
	}
	else {
		if( 0 == popNum ) {
			// キーボード用キャプチャ画像が存在する場合、画像の破棄を行う.
			if( pc_CapturePopupKeyBoard != 0 ) {
				pc_CapturePopupKeyBoard->releaseCaptureImage();
				pc_CapturePopupKeyBoard->hide();
			}
			//ポップアップがすべてなくなった場合、キャプチャを非表示にし、ベースWidgetを表示.
			pc_CapturePopupBaseTable[0]->hide();
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
			base->updateLcdPane( base );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			baseParts->show();
		}
		else if( 1 == popNum ) {
			// キーボード用キャプチャ画像が存在する場合、画像の破棄を行う.
			if( pc_CapturePopupKeyBoard != 0 ) {
				pc_CapturePopupKeyBoard->releaseCaptureImage();
				pc_CapturePopupKeyBoard->hide();
			}
			//ポップアップ1枚の場合、ベースWidgetをキャプチャ.
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
			base->updateLcdPane( base );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
// comment 2018/04/18 SCD2 Kazuo.Takei [Eagle/Sparrow][制御構成変更_No.111 P07_01_共通仕様(第6世代UI画面_左エリア)].
			pc_CapturePopupBaseTable[0]->createCaptureImage( baseParts, False, True );
			pc_CapturePopupBaseTable[1]->hide();
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
			GFWC_System::getInstance()->setLcdPane( getWidgetPane( pc_CapturePopupBaseTable[0] ) );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			pc_CapturePopupBaseTable[0]->show();

// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
//			base->updateLcdPane( base );	// ペイン更新
//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] 起動時に画面がちらつく.
//			GFWC_System::getInstance()->setLcdPane( getWidgetPane( gray ) );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			//グレーWidgetを表示.
			gray->show();

			//ポップアップを表示.
			if( 0 != popParts ) {
				popParts->setParentParts( pc_CapturePopupBaseTable[0] );
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
				pop->updateLcdPane( pop );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
				popParts->show();
			}
		}
		else {
			// キーボード画面が最前面に移動した場合.
			PBasicMessageProc*	pop	= getNowPopup();
			PBasicMessageProc*	topPop	= getNowPopup();
			unsigned char ucInsntanceCnt;
// comment 2016/1/28 MSE海田 [ZeusS_ZX0][CQ523861] NULLチェックを追加.
			if( 0 != pc_CapturePopupKeyBoard ) {
				for( ucInsntanceCnt = 0; ucInsntanceCnt < ( sizeof( pc_KeyBoardInstance ) / sizeof( PLayerControl* ) ); ucInsntanceCnt++ ) {
					if( pop == pc_KeyBoardInstance[ucInsntanceCnt] ) {
						// 最前面にキーボード画面が存在する場合、キャッシュした画面を破棄する.
						pc_CapturePopupKeyBoard->releaseCaptureImage();
						break;
					}
				}
			}

			//以降はPNLD_CAPTURE_POPUP_MNG_NUM_MAX毎にキャプチャ切り替え.
			unsigned char	widgetIdx	= (unsigned char)(( PNLD_CAPTURE_POPUP_MNG_NUM_MAX + popNum - 2 ) / PNLD_CAPTURE_POPUP_MNG_NUM_MAX);

			//キャプチャ必要なポップアップの親をキャプチャするWidgetに変更.
			int	i;
			for( i = ( widgetIdx - 1 ) * PNLD_CAPTURE_POPUP_MNG_NUM_MAX + 1; i < popNum; i++ ) {
				isKeyBoard = False;
				pop	= getPopupInstance( (char)i );
				if( 0 == pop ) {
					// 表示されているはずのポップアップ画面のインスタンスが取得できていない.
					LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
					break;
				}
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
				pop->updateLcdPane( pop );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
				// 2重キーボード画面検索処理.
				for( ucInsntanceCnt = 0; ucInsntanceCnt < ( sizeof( pc_KeyBoardInstance ) / sizeof( PLayerControl* ) ); ucInsntanceCnt++ ) {
					if( pop == pc_KeyBoardInstance[ucInsntanceCnt] ) {
						isKeyBoard = True;
						break;
					}
				}
				// 2重キーボード画面の場合のみ以下の処理を行わない.
				if( True != isKeyBoard ) {
					// キーボードを含まない画面では管理テーブルで管理しているインスタンスを使用する.
					if ( ( True == topPop->getBackLayerDarkness() ) || ( i != popNum - 1 ) ) {
						PDisplayParts*	popParts	= pop->getDisplayParts();
						if( 0 != popParts ) {
							popParts->setParentParts( pc_CapturePopupBaseTable[widgetIdx - 1] );
							popParts->show();
						}
						else {
							// 表示されているはずのポップアップ画面のポップアップWidgetが取得できていない.
							LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
							break;
						}
					}
				} else {
// comment 2016/1/28 MSE海田 [ZeusS_ZX0][CQ523861] NULLチェックを追加.
					if( 0 != pc_CapturePopupKeyBoard ) {
						// キーボードを含む画面では作成したキャプチャを使用する.
						pc_CapturePopupKeyBoard->setParentParts( pc_CapturePopupBaseTable[widgetIdx - 1] );
						pc_CapturePopupKeyBoard->show();
					}
				}
			}
			if( False == topPop->getBackLayerDarkness() ) {
				PBasicMessageProc* prevPop = getPopupInstance( static_cast<char>( popNum - 1 ) );
				if( 0 != prevPop ) {
					PBaseWidget* prevPopParts  = dynamic_cast<PBaseWidget*>( prevPop->getDisplayParts() );
					if( 0 != prevPopParts ) {
						prevPopParts->hide();
					}
				}
			}
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
			PBasicMessageProc* prevPop = getPopupInstance( static_cast<char>( popNum - 1 ) );
// comment 2018/04/13 【HCL-Rajesh】 [IT6 Eagle]  Klockwork Changes ID: 2546901 [NPD.FUNC.MUST].
			if ( NULL != prevPop) {
				prevPop->updateLcdPane( prevPop );	// ペイン更新
			}
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
// comment 2018/04/18 SCD2 Kazuo.Takei [Eagle/Sparrow][制御構成変更_No.111 P07_01_共通仕様(第6世代UI画面_左エリア)].
// comment 2019/2/12 Sky雜賀[EagleH][OP_BTS-1756].
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.																// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
			uchar uc_PopIdxStart = ( widgetIdx - 1 ) * PNLD_CAPTURE_POPUP_MNG_NUM_MAX + 1;

			// キャプチャーされるポップアップすべてが同じペインの場合には、まとめてキャプチャーする.
			if ( False == isEnableCaptureBlock( uc_PopIdxStart, static_cast<char>( popNum - 1 ) ) ) {									// ペインが同じ場合.
				pc_CapturePopupBaseTable[widgetIdx]->createCaptureImage( pc_CapturePopupBaseTable[widgetIdx - 1], False, True, True );
			} else {																													// 異なるペインが混じっている場合.
				for( i = uc_PopIdxStart; i < popNum; i++ ) {
					pop = getPopupInstance( static_cast<char>( i ) );
					if ( NULL != pop ) {
						GFWC_System::getInstance()->setLcdPane( pop->getGamenPane( pop ) );
						PBaseWidget*	PopParts	= dynamic_cast<PBaseWidget*>( pop->getDisplayParts() );

						// キャプチャーされる一番下のレイヤーのみは再生成し、2枚目、3枚目は前のキャプチャーを保持したまま合成する.
						if ( 0 != PopParts ) {
							if ( i == uc_PopIdxStart ) {						// キャプチャーされる一番下のレイヤー( １、４、７、・・・）.
								pc_CapturePopupBaseTable[widgetIdx]->createCaptureImage( PopParts, False, True, True );
							} else {											// キャプチャーの二枚目、三枚目（２、３、５、６、・・・).
								pc_CapturePopupBaseTable[widgetIdx]->createCaptureImage( PopParts, False, True, False );
							}
						}
					}
				}
			}
#else
			pc_CapturePopupBaseTable[widgetIdx]->createCaptureImage( pc_CapturePopupBaseTable[widgetIdx - 1], False, True );
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.	

			pc_CapturePopupBaseTable[widgetIdx + 1]->hide();
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
// comment 2018/10/14 【HCL-Nadeem】 [IT6 Eagle]  Klockwork Changes ID: 811006 [NPD.FUNC.MUST].
			if ( NULL != prevPop) {
				prevPop->updateLcdPane( prevPop );	// ペイン更新
			}
//			GFWC_System::getInstance()->setLcdPane( getWidgetPane( pc_CapturePopupBaseTable[widgetIdx] ) );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			pc_CapturePopupBaseTable[widgetIdx]->show();

			//グレーWidgetを表示.
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
			GFWC_System::getInstance()->setLcdPane( getWidgetPane( gray ) );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			gray->show();
			if( False == topPop->getBackLayerDarkness() ) {
				PBasicMessageProc* prevPop = getPopupInstance( static_cast<char>( popNum - 1 ) );
				if( 0 != prevPop ) {
					PBaseWidget* prevPopParts  = dynamic_cast<PBaseWidget*>( prevPop->getDisplayParts() );
					if( 0 != prevPopParts ) {
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
						prevPop->updateLcdPane( prevPop );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
						prevPopParts->show();
					}
				}
			}

			//ポップアップを表示.
			if( 0 != popParts ) {
				popParts->setParentParts( pc_CapturePopupBaseTable[widgetIdx] );
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
				topPop->updateLcdPane( topPop );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
				popParts->show();
			}
		}
	}
}

// comment 2019/2/12 Sky雜賀[EagleH][OP_BTS-1756].
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// ===============================================================================================.
//	一枚づつポップアップを作るか.
// ===============================================================================================.
Bool PNLC_PopupListControl::isEnableCaptureBlock( uchar uc_PopIdxStart, uchar uc_PopIdxEnd )
{
	Bool b_Result = False;
	PBasicMessageProc* pop = getPopupInstance( uc_PopIdxStart );
	if ( NULL == pop ) {
		return b_Result;
	}

	GFWE_LCD_PANE e_Lcd_Pane = pop->getGamenPane( pop );

	for( char i = uc_PopIdxStart + 1; i <= uc_PopIdxEnd; i++ ) {
		pop = getPopupInstance( i );

		if ( NULL != pop ) {
			if ( e_Lcd_Pane != pop->getGamenPane( pop ) ) {
				b_Result = True;
				break;
			}
		}
	}

	return b_Result;
}
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.

// ===============================================================================================.
//	画面キャプチャイメージの破棄.
// ===============================================================================================.
void PNLC_PopupListControl::releaseCapturePopupImage()
{
	char	popNum	= getPopupNum();
	unsigned char	widgetIdx	= (unsigned char)(( PNLD_CAPTURE_POPUP_MNG_NUM_MAX + popNum - 2 ) / PNLD_CAPTURE_POPUP_MNG_NUM_MAX);
	PBaseWidget*	capture;
	int i;

	if( popNum == 0 ) {
		// ポップアップ表示枚数が0の場合は全ての画面キャプチャイメージを破棄する.
		for( i = 0; i < PNLD_CAPTURE_POPUP_NUM_MAX; i++ ) {
			capture	= pc_CapturePopupBaseTable[i];
			capture->releaseCaptureImage();
			capture->hide();
		}
// comment 2019/08/20 SCD2-Sky中野 [EagleH][OP_BTS-12365].
		// キャプチャ作成済みフラグをクリア.
		for( ushort j = 0; PNLD_PopupMaxNum > j; ++j ) {
			m_bl_CaptureCreated[j] = False;
		}
	}
	else {
		// ポップアップ表示枚数が1枚以上の場合、使用される画面キャプチャWidget以外のWidgetのイメージをクリアする.
		for( i = ( PNLD_CAPTURE_POPUP_NUM_MAX - 1 ); i > widgetIdx; i-- ) {
			capture	= pc_CapturePopupBaseTable[i];
			capture->releaseCaptureImage();
			capture->hide();
		}
// comment 2019/08/20 SCD2-Sky中野 [EagleH][OP_BTS-12365].
		// キャプチャ作成済みフラグをクリア.
		for( ushort j = ( PNLD_PopupMaxNum - 1 ); popNum <= j; --j ) {
			m_bl_CaptureCreated[j] = False;
		}
	}
}

// ===============================================================================================.
//	アニメーション開始要求中フラグを設定する.
// ===============================================================================================.
void PNLC_PopupListControl::setAnimationFlg( Bool b_Animation )
{
	b_StartAnimation = b_Animation;
}

// ===============================================================================================.
//	次画面ポップアップの準備.
// ===============================================================================================.
void PNLC_PopupListControl::prepareNextPopup( Bool b_popupStatus )
{
	char	popNum	= getPopupNum();
	PBasicMessageProc*	base	= getBaseGamen();
	PDisplayParts*	baseParts	= base->getDisplayParts();
	if( 0 == baseParts ) {
		// ベース画面WidgetがNULLの場合、ポップアップWidgetの親が存在しないため処理終了.
		return;
	}
	PGrayoutWidget*	gray	= base->getGrayoutParts();
	if( 0 == gray ) {
		// グレー画像Widgetが存在しない場合、ポップアップ表示が行われていないため処理終了.
		return;
	}

	PBasicMessageProc*	pop	= getPopupInstance( popNum );
	if( 0 == pop ) {
		// ポップアップインスタンスがNULLの場合、表示するポップアップが存在しないと判断し処理終了.
		return;
	}

	PBaseWidget*	popParts	= dynamic_cast<PBaseWidget*>( pop->getDisplayParts() );
	if( 0 == popParts ) {
		// ポップアップWidgetが存在しない場合、表示するポップアップが存在しないと判断し処理終了.
		return;
	}

	if( True == b_popupStatus ) {	//ポップアップopen.
		if( 1 == popNum ) {
			pc_CapturePopupBaseTable[0]->bringToFront();
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] 起動時に画面がちらつく.
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
//			GFWC_System::getInstance()->setLcdPane( getWidgetPane( pc_CapturePopupBaseTable[0] ) );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			pc_CapturePopupBaseTable[0]->show();
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
			base->updateLcdPane( base );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			baseParts->show();

			if( True == pop->getBackLayerDarkness() ) {
				// 本ポップアップの背面にグレーWidgetを表示させない場合.
				// 前ポップアップの背面にグレーWidgetを表示させたままにする必要があるためレイヤ移動はしない.
				gray->setParentParts( pc_CapturePopupBaseTable[0] );
				gray->bringToFront();
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
			GFWC_System::getInstance()->setLcdPane( getWidgetPane( gray ) );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
				gray->show();
			}

			if( 0 != popParts ) {
				popParts->setParentParts( pc_CapturePopupBaseTable[0] );
				popParts->bringToFront();
				popParts->hide();
			}
		}
		else {
			//以降はPNLD_CAPTURE_POPUP_MNG_NUM_MAX毎にキャプチャ切り替え.
			unsigned char	widgetIdx	= (unsigned char)(( PNLD_CAPTURE_POPUP_MNG_NUM_MAX + popNum - 2 ) / PNLD_CAPTURE_POPUP_MNG_NUM_MAX);

			pc_CapturePopupBaseTable[widgetIdx]->bringToFront();
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] 3ペイン⇒1ペインで画面がちらつく.
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
//			GFWC_System::getInstance()->setLcdPane( getWidgetPane( pc_CapturePopupBaseTable[widgetIdx] ) );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			pc_CapturePopupBaseTable[widgetIdx]->show();
			if ( ( ( PNLD_CAPTURE_POPUP_MNG_NUM_MAX + popNum - 2 ) % PNLD_CAPTURE_POPUP_MNG_NUM_MAX ) == 0 ) {
				// 画面キャプチャwidgetが切り替わったタイミングは透明Widgetのままなのでひとつ下の画面キャプチャWidgetを表示する.
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] 3ペイン⇒1ペインで画面がちらつく.
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
//				GFWC_System::getInstance()->setLcdPane( getWidgetPane( pc_CapturePopupBaseTable[widgetIdx - 1] ) );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
				pc_CapturePopupBaseTable[widgetIdx - 1]->show();
			}

			gray->setParentParts( pc_CapturePopupBaseTable[widgetIdx] );
			if( True == pop->getBackLayerDarkness() ) {
				// 本ポップアップの背面にグレーWidgetを表示させない場合.
				// 前ポップアップの背面にグレーWidgetを表示させたままにする必要があるためレイヤ移動はしない.
				gray->setParentParts( pc_CapturePopupBaseTable[widgetIdx] );
				gray->bringToFront();
			} else {
				PBasicMessageProc*	prevPop	= getPopupInstance( static_cast<char>( popNum - 1 ) );
				if( 0 != prevPop ) {
					PBaseWidget*	prevPopParts	= dynamic_cast<PBaseWidget*>( prevPop->getDisplayParts() );
					if ( 0 != prevPopParts ) {
						prevPopParts->setParentParts( pc_CapturePopupBaseTable[widgetIdx] );
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
						prevPop->updateLcdPane( prevPop );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
						prevPopParts->show();
					}
				}
			}
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] 3ペイン⇒1ペインで画面がちらつく.
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
//			GFWC_System::getInstance()->setLcdPane( getWidgetPane( gray ) );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			gray->show();

			if( 0 != popParts ) {
				popParts->setParentParts( pc_CapturePopupBaseTable[widgetIdx] );
				popParts->bringToFront();
				popParts->hide();
			}
		}
	}
	else {
		// 最前面に表示される不要な画面キャプチャWidgetを削除.
		releaseCapturePopupImage();

		if( 0 == popNum ) {
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] コピーキー押下で画面がちらつく.
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
//			base->updateLcdPane( base );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			baseParts->show();
		}
		else if( 1 == popNum ) {
			pc_CapturePopupBaseTable[1]->hide();
			pc_CapturePopupBaseTable[0]->bringToFront();
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] 3ペイン⇒1ペインで画面がちらつく.
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
//			GFWC_System::getInstance()->setLcdPane( getWidgetPane( pc_CapturePopupBaseTable[0] ) );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			pc_CapturePopupBaseTable[0]->show();

// comment 2015/12/16 MSE岡田 [ZeusS_ZX0][CQ522807]最前面ポップアップが背面にグレーを持たない場合、グレー画面を再配置しない.
//			gray->setParentParts( pc_CapturePopupBaseTable[0] );
//			gray->bringToFront();
//			gray->show();
			// 最前面ポップアップが背面にグレーを持たない場合、グレー画面を再配置しない.
			if ( True == pop->getBackLayerDarkness() ) {
				gray->setParentParts( pc_CapturePopupBaseTable[0] );
				gray->bringToFront();
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] 3ペイン⇒1ペインで画面がちらつく.
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
//				GFWC_System::getInstance()->setLcdPane( getWidgetPane( gray ) );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
				gray->show();
			}

			if( 0 != popParts ) {
				popParts->setParentParts( 0 );
				popParts->bringToFront();
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] コピーキー押下で画面がちらつく.
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
//				pop->updateLcdPane( pop );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
				popParts->show();
			}
		}
		else {
			//以降はPNLD_CAPTURE_POPUP_MNG_NUM_MAX毎にキャプチャ切り替え.
			unsigned char	widgetIdx	= (unsigned char)(( PNLD_CAPTURE_POPUP_MNG_NUM_MAX + popNum - 2 ) / PNLD_CAPTURE_POPUP_MNG_NUM_MAX);

			pc_CapturePopupBaseTable[widgetIdx + 1]->hide();
			pc_CapturePopupBaseTable[widgetIdx]->bringToFront();
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] 3ペイン⇒1ペインで画面がちらつく.
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
//			GFWC_System::getInstance()->setLcdPane( getWidgetPane( pc_CapturePopupBaseTable[widgetIdx] ) );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			pc_CapturePopupBaseTable[widgetIdx]->show();

			//グレーWidgetを表示.グレー画面の親を表示するポップアップと同じ親にするためここではフラグチェックをしない.
			gray->setParentParts( pc_CapturePopupBaseTable[widgetIdx] );
			gray->bringToFront();
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] 3ペイン⇒1ペインで画面がちらつく.
//// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
//			GFWC_System::getInstance()->setLcdPane( getWidgetPane( gray ) );	// ペイン更新
//// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			gray->show();
			
// comment 2015/12/16 MSE岡田 [ZeusS_ZX0][CQ522807]ポップアップ削除後に本関数を実行するように変更したことでpopNumの値が修正前と変わることへの対応.
			// 新たに最前面になるポップアップは背面にグレーを持つかどうかをチェック.
			// 最前面ポップアップが背面にグレーを持たない場合、もう一つ前のポップアップ背面にグレーが来るようにレイヤ設定.
//			PBasicMessageProc*	tmpPop	= getPopupInstance( popNum - 1 );
			PBasicMessageProc*	tmpPop	= getPopupInstance( popNum );	// 最前面ポップアップ.
			if ( tmpPop && ( False == tmpPop->getBackLayerDarkness() ) ) {
//				PBasicMessageProc*	prevPop	= getPopupInstance( popNum - 2 );
				PBasicMessageProc*	prevPop	= getPopupInstance( static_cast<char>( popNum - 1 ) );	// 一つ前のポップアップ
				if( 0 != prevPop ) {
					PBaseWidget*	prevPopParts	= dynamic_cast<PBaseWidget*>( prevPop->getDisplayParts() );
					if ( 0 != prevPopParts ) {
						// この時点でグレー画面は最前面に配置されている.
						// 一つ前のポップアップを最前面にすることでグレー画面を一つ前のポップアップの背面に配置する.
						prevPopParts->setParentParts( pc_CapturePopupBaseTable[widgetIdx] );
						prevPopParts->bringToFront();
					}
				}
			}

			//ポップアップを表示.
			if( 0 != popParts ) {
				popParts->setParentParts( 0 );
				popParts->bringToFront();
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
//comment 2018/10/5 SCD2-Sky肥後 [Eagle][AR1061802] コピーキー押下で画面がちらつく.
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
//				pop->updateLcdPane( pop );	// ペイン更新
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
				popParts->show();
			}
		}
	}

	// comment 2018/09/14 SCD2 T.Nishiba [Eagle][Sparrow][AR1047776]スライドメニュー対応.
	PNLC_SlideMenuControlLib::newInstance()->judgeHide();
}

// comment 2015/06/04 OPS1-Sky浜野 [MinervaSBK][UV-01457] 画面ちらつき抑止.
// ===============================================================================================.
/**
 * @brief	ポップアップキャプチャ無効設定.
 * @param[in]	bl_CapturePopup		キャプチャ取得無効有無.
 **/
// ===============================================================================================.
void PNLC_PopupListControl::setCapturePopupDisable( Bool bl_CapturePopup )
{
	m_bl_CapturePopupDisable = bl_CapturePopup;
}

// ===============================================================================================.
/**
 * @brief	ポップアップキャプチャ無効取得.
 * @retval	キャプチャ取得無効有無(True：無効、False：有効).
 **/
// ===============================================================================================.
Bool PNLC_PopupListControl::isCapturePopupDisable()
{
	return m_bl_CapturePopupDisable;
}
// comment 2015/07/13 OPS1-Sky後藤 [CQ514499] LDAP検索パフォーマンス改善.
// ===============================================================================================.
//	上位にある画面のパーツを表示/非表示にする.
// ===============================================================================================.
void PNLC_PopupListControl::showEnableParts( Bool bl_Enable /* = True */ )
{
	char	sc_popNum = getPopupNum();

	if( 1 > sc_popNum ) {
		return;											// パラメータ不正、またはポップアップが起動していない場合は、False返却.
	}	
	
	for( uchar i = static_cast<uchar>(sc_popNum - 1 ); i > 0; --i ){			// 上位にある画面を表示/非表示にする.
	
		if( NULL != p_PopupTable[i]){
			if( True == bl_Enable ) {					// bl_EnableがTrueの場合、表示.
// Comment 2020/10/27 [HCLT-Ranjith] [IT6_2.1] [OP_BTS-28301] Display and hide the parts based on screen class.
//				p_PopupTable[i]->resetParts();		
//				p_PopupTable[i]->showEnableAllPars();		
				p_PopupTable[i]->show();
			} else {									// bl_EnableがFalseの場合、非表示.
//				p_PopupTable[i]->showEnableAllPartsHide();
				p_PopupTable[i]->hide();
			}
		}
	}
}

// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. Start.
// ===============================================================================================.
//	指定Widgetの表示ペインを取得する.
// ===============================================================================================.
GFWE_LCD_PANE PNLC_PopupListControl::getWidgetPane( PBaseWidget* pt_Widget )
{
	PBaseWidget*	pt_BaseWidget	= static_cast<PBaseWidget*>( pt_Widget );
	
	if ( NULL ==pt_BaseWidget ) {
		return GFWE_LCD_PANE_3;
	}
	GFWC_Common*	pt_GFWC_Common	= static_cast<GFWC_Common*>( pt_BaseWidget->getGUIParts() );
	if ( NULL ==pt_GFWC_Common ) {
		return GFWE_LCD_PANE_3;
	}

	GFWC_Size	s_GFWC_Size = pt_GFWC_Common->getSize();
	if ( ( 800 == s_GFWC_Size.width() ) && ( 480 ==  s_GFWC_Size.height() ) ) {
		return GFWE_LCD_PANE_3;
	} else {
		return GFWE_LCD_PANE_1;
	}
}
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更]. End.
// comment 2018/10/12 Sky東島 [1062683] 2ペイン化対応.
#define PNLD_From3PaneTo1PaneCorrectX ((1024 - 902)/2)			// 3ペイン画面から1ペイン画面への補正値X( 画面解像度X(1024) - Legacy表示部X(902) ) /2.
#define PNLD_From3PaneTo1PaneCorrectY ((600 - 530)/2)			// 3ペイン画面から1ペイン画面への補正値Y( 画面解像度Y(600) - Legacy表示部Y(530) ) /2.
#define PNLD_A072002900_PosY 		  (32)			// 3ペイン画面での言語一時変更のY座標.
// 3ペインから表示位置を変更する .
//	[引数]		PLayerControl* p_PopupInstance 対象画面.
//	[戻り値]	True:ペイン切り替えがある. False:ペイン切り替えがない.
Bool PNLC_PopupListControl::changePopupPositionFrom3Pane( PLayerControl* p_PopupInstance )
{
	Bool bl_changePane = False;
	PPopupAnimationCom* pc_PopupInstance = dynamic_cast< PPopupAnimationCom* >(p_PopupInstance);
	if( NULL != pc_PopupInstance ){
		for( ushort us_offset = 0; NULL != tbl_1or2PaneGamen[us_offset].pc_1or2PaneGamen; us_offset++ ) {
			if ( pc_PopupInstance == tbl_1or2PaneGamen[us_offset].pc_1or2PaneGamen ) {

				GFWE_LCD_PANE e_LCDPane = pc_PopupInstance->getGamenPane( pc_PopupInstance );
				short ss_posX = 0;
				short ss_posY = 0;
				short ss_Width = 0;
				short ss_Height = 0;
				pc_PopupInstance->getPopupSize( &ss_Width , &ss_Height );
				
				ss_posX = PNLD_POPUP_START_X( ss_Width );
				ss_posY = PNLD_POPUP_START_Y( ss_Height , ( True == tbl_1or2PaneGamen[us_offset].bl_showCloseButton ? PNLD_POPUP_BUTTON_EXIST : PNLD_POPUP_BUTTON_NOTEXIST ) );
				
				if( GFWE_LCD_PANE_1_Extention == e_LCDPane ){
					// 1ペイン(拡大表示)の場合,下エリア、右エリア分を補正する.
					ss_posX += PNLD_From3PaneTo1PaneCorrectX;
					ss_posY += PNLD_From3PaneTo1PaneCorrectY;
					bl_changePane = True;
				} else if( GFWE_LCD_PANE_2 == e_LCDPane ){
					// 2ペインの場合,右エリア分を補正する.
					ss_posX += PNLD_From3PaneTo1PaneCorrectX;
					bl_changePane = True;
				} else if( ( GFWE_LCD_PANE_3 == e_LCDPane )
						&& ( pc_PopupInstance == &gp_A072002900 ) ){
					// 言語一時変更画面は画面中央表示ではないため個別の補正をする.
					ss_posY = PNLD_A072002900_PosY;
				}
				pc_PopupInstance->setPosition( ss_posX,ss_posY );
				
				break;
			}
		}
	}
	return bl_changePane;
}

// comment 2018/10/12 Sky東島 [1062683] 2ペイン化対応.
// 画面キャプチャの生成(ポップアップ表示前).
//	[引数]		PLayerControl* p_PopupInstance 対象画面.
//	[戻り値]	なし.
void PNLC_PopupListControl::createCaptureBeforeOpenPopup( PLayerControl* p_PopupInstance )
{
	if( NULL == p_PopupInstance ){
		return;
	}

	// ペイン変更が発生する場合,画面表示前に表示前画面が拡大されてしまうため先にキャプチャを生成しておく.
	if( False == p_PopupInstance->getBackLayerDarkness() ){
		// グレー背景を表示しないポップアップの場合はキャプチャ生成しない.
		return;
	}

	char	popNum	= getPopupNum();
	PBasicMessageProc*	prevPop	= getPopupInstance( static_cast<char>( popNum - 1 ) );
	if( NULL == prevPop ){
		return;
	}

	if( 1 == popNum ){
		PDisplayParts*	baseParts	= prevPop->getDisplayParts();
		if( NULL == baseParts
		 || NULL == pc_CapturePopupBaseTable[0] ) {
			return;
		}
		pc_CapturePopupBaseTable[0]->createCaptureImage( baseParts, False, True );
		baseParts->hide();
		pc_CapturePopupBaseTable[0]->show();
	} else {
		unsigned char	widgetIdx	= ( PNLD_CAPTURE_POPUP_MNG_NUM_MAX + popNum - 2 ) / PNLD_CAPTURE_POPUP_MNG_NUM_MAX;
		if( NULL == pc_CapturePopupBaseTable[widgetIdx]
		 || NULL == pc_CapturePopupBaseTable[widgetIdx - 1]){
			return;
		}
		// 一つ下のレイヤーのポップアップの親を切り替える.
		PBaseWidget*	prevPopParts	= dynamic_cast<PBaseWidget*>( prevPop->getDisplayParts() );
		if ( 0 != prevPopParts ) {
			prevPopParts->setParentParts( pc_CapturePopupBaseTable[widgetIdx -1] );
		}
		// キャプチャを生成する.
		pc_CapturePopupBaseTable[widgetIdx]->createCaptureImage( pc_CapturePopupBaseTable[widgetIdx - 1], False, True );
		pc_CapturePopupBaseTable[widgetIdx - 1]->hide();
		pc_CapturePopupBaseTable[widgetIdx]->show();
	}
}
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
