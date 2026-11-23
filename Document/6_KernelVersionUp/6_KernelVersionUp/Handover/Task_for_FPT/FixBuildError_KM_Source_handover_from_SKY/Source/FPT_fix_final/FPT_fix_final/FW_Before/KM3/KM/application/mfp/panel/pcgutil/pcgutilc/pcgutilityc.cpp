//================================================================================================
//［タイトル］	ユーティリティ設定画面のメッセージ処理 
//================================================================================================
//================================================================================================
//	インクルード・ファイル
//================================================================================================
#include <panel/pcgcom/pcgcomg/pcgcom.h>	// Qt4対応:MFC-QT競合回避 
#include <system/thr/thrInclude/THRC_PortInstance.h>
#include <panel/pcgutil/pcgutil_CLCD/piputility_CLCD.h>
#include <panel/pcgutil/pcgutil_CLCD/pigutility_CLCD.h>
#include <panel/pcgutil/pcgutil_CLCD/piputilityc_CLCD.h>
#include <panel/pcgutil/pcgutil_CLCD/pigutilityc_CLCD.h>
#include <panel/pddtext/pddtext_CLCD/pstr_CLCD.h>
#include <panel/plibh/plibhg/plibhCommonKeybord.h>		// キーボード処理共通化.
#include "pctutilityc.h"
#include <panel/plibh/plibhg/plibh3.h>	// DonauLastFum【ネットワーク通信文字拡張】【キーボードバッファ対応】【13原】2009/07/13 add.
#include <panel/plibh/plibh_CLCD2/plibh_CLCD2.h>
// Comment 2017/09/06【HCLT-Rajesh】【IT5 4.2】【RQ-686】【WP 403】Multiple Network support.
#include <panel/plibh/plibh_CLCD2/PNLC_NetworkLan.h>
#include <panel/../../divlib/Common/blkapi/blkshm.h>
// comment 2014/12/10 OPS1-Sky西原 [HeliosL][制御構成変更No.1]封筒通紙の操作性改善（定着手動離間）.
#include <panel/pcgutil/pcgutil_CLCD/piputilityc_CLCD.h>
// comment 2016/01/29 OPS1延原 [ZeusS_ZX0][CQ524561]ファクス設定＞設定値リスト画面表示後に、他のリスト出力画面の片面/両面キーが表示されない.
#include <panel/pcgadm/pcgadmg/pcgadmin1.h>
// Comment 2019/01/24 【HCL_Krishna】【IT6_EagleH】【RQ-506】[WP-1710] include the header file.
#include "../../shlib/RecoveryLib/LIBC_RecoveryLib_IF.h"
//================================================================================================
//	モジュール内変数の宣言
//================================================================================================
short PUtilityApplicom::ss_RPnlMaxStringInput = 0;		// 文字入力画面の最大入力文字数
short PUtilityApplicom::ss_RPnlKeyResponseTimer = -1;	// キー入力通知からのタイマー
//================================================================================================
//	モジュール内関数のプロトタイプ宣言
//================================================================================================
//================================================================================================.
// 概要		:ユーティリティ・セールスカウンタ（コピー）: 表示画面のセットアップ.
//================================================================================================.
void PUtilityTotalCopy::setupGamen(
    short option )
{
	Bool bl_EnableHDDExistFunc = True;				// HDDレス時有効フラグ.
	if( False == PnlComIsUseSsdScanSend() ) {	// HDD/SSD使用不可.
		bl_EnableHDDExistFunc = False;
	}

	PUtilityTotalCom::setupGamen( option );
	if( onlyPanel.isFaxModekind() == True ) {
		Key_UtilityTabUnSelScanFax.changeText( s_KEYScannerFax );
	} else {
		Key_UtilityTabUnSelScanFax.changeText( s_KEYScanner_02 );
	}
	Key_UtilityTabUnSelScanFax.showEnable( bl_EnableHDDExistFunc );			// ファクス/スキャンキー.
// 2014/12/25 [MAv3][RQ-245] トータルカウンターの異常値対応 トータルカウンターの履歴を残す 日立 村上(幸).
// comment 2015/10/30 [CQ518309] セールスカウンタ履歴のレイアウト修正 日立 村上(幸).
//	if( True == onlyPanel.m_bl_MagicSequenceTotalCounterHistory ) {
//		char							sc_DateTimeBuf[PNLD_HistoryListDateTimeMaxLen];	// 日時格納バッファ.
//		TYPS_BackupRestore_TotalCounter s_HistoryList;
//		memset( &s_HistoryList, 0x00, sizeof( s_HistoryList ) );
//		memset( sc_DateTimeBuf, 0x00, sizeof( sc_DateTimeBuf ) );
//		gp_UtilityTotalCounterHistory.getTotalCounterHistory( &s_HistoryList, sc_DateTimeBuf );
//
//		// パーツの表示/非表示.
//		Txt_UtilityTotalSerialNumberMo.showEnable( False );																// シリアル番号を非表示.
//		Txt_UtilityTotalSerialNumbers_ColonMo.showEnable( False );														// 「:」を非表示.
//		Tfd_UtilityTotalSerialNumberMo.showEnable( False );																// シリアル番号を非表示.
//		Txt_UtilityTotalCounterHistory.showEnable( True );																// セールスカウンター履歴を表示.
//		Txt_UtilityTotalCounterHistory_Colon.showEnable( True );														// 「:」を表示.
//		Tfd_UtilityTotalCounterHistory.showEnable( True );																// 履歴日時を表示.
//		Tfd_UtilityTotalCounterHistory.changeExternalText( (uchar *)sc_DateTimeBuf );									// 日時.
//		Key_GotoUtilityTotalCoverageRate.showEnable( False );	// カバレッジカウンターキーを非表示.
//		Key_GotoUtilityTotalEcoIndex.showEnable( False );		// Eco指標ボタンを非表示.
//		Key_UtilityTabUnSelOther.showEnable( False );			// その他ボタンを非表示.
//
//		// 選択された履歴番号のトータルカウンタ情報を表示する.
//		// コピー・ブラック・トータルカウンタ値.
//		Nfd_UtilityTotalCopyBlackTotal			= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_CopyTotalBlack];		// コピー・トータル・ブラック.
//		// コピー・ブラック・大サイズカウンタ値.
//		Nfd_UtilityTotalCopyBlackLargeSize		= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_CopyLargeBlack];		// コピー・大サイズ・ブラック.
//		// コピー・ブラック・カラートータル（コピー＋プリンタ）カウンタ値.
//		Nfd_UtilityTotalCopyBlackCopyPrinter	= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_CopyTotalBlack]		// コピー・トータル・ブラック.
//													+ s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_PrintTotalBlack];		// プリント・トータル・ブラック.
//		// コピー・フルカラー・トータルカウンタ値.
//		Nfd_UtilityTotalCopyFullColorTotal		= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_CopyTotalFullColor];	// コピー・トータル・フルカラー.
//		// コピー・フルカラー・大サイズカウンタ値.
//		Nfd_UtilityTotalCopyFullColorLargeSize	= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_CopyLargeFullColor];	// コピー・大サイズ・フルカラー.
//		// コピー・フルカラー・カラートータル（コピー＋プリンタ）カウンタ値.
//		Nfd_UtilityTotalCopyFullColorCopyPrinter = (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_CopyTotalFullColor]	// コピー・トータル・フルカラー.
//													+ s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_PrintTotalFullColor];	// プリント・トータル・フルカラー.
//		// コピー・モノカラー・トータルカウンタ値.
//		Nfd_UtilityTotalCopyMonoColorTotal		= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_CopyTotalMonoColor];	// コピー・トータル・モノカラー.
//		// コピー・モノカラー・大サイズカウンタ値.
//		Nfd_UtilityTotalCopyMonoColorLargeSize	= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_CopyLargeMonoColor];	// コピー・大サイズ・モノカラー.
//		// コピー・２色・トータルカウンタ値.
//		Nfd_UtilityTotalCopy2ColorTotal			= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_CopyTotal2Color];	// コピー・トータル・２色.
//		// コピー・２色・大サイズカウンタ値.
//		Nfd_UtilityTotalCopy2ColorLargeSize		= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_CopyLarge2Color];	// コピー・大サイズ・２色.
//		// コピー・２色・カラートータル（コピー＋プリンタ）カウンタ値.
//		Nfd_UtilityTotalCopy2ColorCopyPrinter	= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_CopyTotal2Color]		// コピー・トータル・２色.
//													+ s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_PrintTotal2Color];		// プリント・トータル・２色.
//		// コピー・全色合計・トータルカウンタ値.
//		Nfd_UtilityTotalCopyColorTotalTotal		= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_CopyTotalTotal];		// コピー・全色合計・トータルカウンタ値.
//		// コピー・全色合計・大サイズカウンタ値.
//		Nfd_UtilityTotalCopyColorTotalLargeSize	= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_CopyLargeTotal];		// コピー・全色合計・大サイズカウンタ値.
//	}
//	else {
//		// パーツの表示/非表示.
//		Txt_UtilityTotalSerialNumberMo.showEnable( True );																// シリアル番号を表示.
//		Txt_UtilityTotalSerialNumbers_ColonMo.showEnable( True );														// 「:」を表示.
//		Tfd_UtilityTotalSerialNumberMo.showEnable( True );																// シリアル番号を表示.
//		Txt_UtilityTotalCounterHistory.showEnable( False );																// セールスカウンター履歴を非表示.
//		Txt_UtilityTotalCounterHistory_Colon.showEnable( False );														// 「:」を非表示.
//		Tfd_UtilityTotalCounterHistory.showEnable( False );																// 履歴日時を非表示.

	char uc_SerialNumberPRT[TYPD_MAX_SerialNoPrinter];
	memset( uc_SerialNumberPRT, '\0', sizeof( uc_SerialNumberPRT ) );
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	pc_SystemInfo->getSerialNoPrinter( ( char* )uc_SerialNumberPRT );
	Tfd_UtilityTotalSerialNumberMo.changeExternalText( ( uchar* )uc_SerialNumberPRT, sizeof( uc_SerialNumberPRT ) );		// シリアル番号・プリンタ
	// コピー・ブラック・トータルカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalCopyBlackTotal			= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalBlack ) 		// コピー・トータル・ブラック
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalBlack, TYPE_NetworkSpecific_NW2 ) );		// コピー・トータル・ブラック
	// コピー・ブラック・大サイズカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalCopyBlackLargeSize		= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLargeBlack ) 		// コピー・大サイズ・ブラック
            + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLargeBlack, TYPE_NetworkSpecific_NW2 ) );
	// コピー・ブラック・カラートータル（コピー＋プリンタ）カウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalCopyBlackCopyPrinter	= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalBlack )		// コピー・トータル・ブラック
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotalBlack )			// プリント・トータル・ブラック
            + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalBlack, TYPE_NetworkSpecific_NW2 )
            + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotalBlack, TYPE_NetworkSpecific_NW2 ) );
	// コピー・フルカラー・トータルカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalCopyFullColorTotal		= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalFullColor ) 	// コピー・トータル・フルカラー
            + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalFullColor, TYPE_NetworkSpecific_NW2 ) );	// コピー・トータル・フルカラー
	// コピー・フルカラー・大サイズカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalCopyFullColorLargeSize	=static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLargeFullColor ) 	// コピー・大サイズ・フルカラー
            + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLargeFullColor, TYPE_NetworkSpecific_NW2 ) );	// コピー・大サイズ・フルカラー
	// コピー・フルカラー・カラートータル（コピー＋プリンタ）カウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalCopyFullColorCopyPrinter = static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalFullColor )	// コピー・トータル・フルカラー
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotalFullColor )
            + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalFullColor, TYPE_NetworkSpecific_NW2 )
            + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotalFullColor, TYPE_NetworkSpecific_NW2 ) );
	// コピー・モノカラー・トータルカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalCopyMonoColorTotal		= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalMonoColor ) 	// コピー・トータル・モノカラー
            + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalMonoColor, TYPE_NetworkSpecific_NW2 ) );	// コピー・トータル・モノカラー
	// コピー・モノカラー・大サイズカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalCopyMonoColorLargeSize	= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLargeMonoColor ) 	// コピー・大サイズ・モノカラー
            + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLargeMonoColor, TYPE_NetworkSpecific_NW2 ) );	// コピー・大サイズ・モノカラー
	// コピー・２色・トータルカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalCopy2ColorTotal			= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotal2Color ) 		// コピー・トータル・２色
            + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotal2Color, TYPE_NetworkSpecific_NW2 ) );		// コピー・トータル・２色
	// コピー・２色・大サイズカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalCopy2ColorLargeSize		= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLarge2Color ) 		// コピー・大サイズ・２色
            + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLarge2Color, TYPE_NetworkSpecific_NW2 ) );		// コピー・大サイズ・２色
	// コピー・２色・カラートータル（コピー＋プリンタ）カウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalCopy2ColorCopyPrinter	= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotal2Color )		// コピー・トータル・２色
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotal2Color )
            + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotal2Color, TYPE_NetworkSpecific_NW2 )
            + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotal2Color, TYPE_NetworkSpecific_NW2 ) );		// プリント・トータル・２色
	// コピー・全色合計・トータルカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalCopyColorTotalTotal		= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalBlack )		// コピー・トータル・ブラック
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalFullColor )		// コピー・トータル・フルカラー
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalMonoColor )		// コピー・トータル・モノカラー
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotal2Color )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalBlack , TYPE_NetworkSpecific_NW2 )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalFullColor , TYPE_NetworkSpecific_NW2 ) // コピー・トータル・２色
            + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalMonoColor , TYPE_NetworkSpecific_NW2 )
            + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotal2Color , TYPE_NetworkSpecific_NW2 ) );
	// コピー・全色合計・大サイズカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalCopyColorTotalLargeSize	= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLargeBlack )		// コピー・大サイズ・ブラック
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLargeFullColor )		// コピー・大サイズ・フルカラー
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLargeMonoColor )		// コピー・大サイズ・モノカラー
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLarge2Color )			// コピー・大サイズ・２色
            + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLargeBlack , TYPE_NetworkSpecific_NW2 )
            + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLargeFullColor, TYPE_NetworkSpecific_NW2 )
            + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLargeMonoColor, TYPE_NetworkSpecific_NW2 )
            + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyLarge2Color , TYPE_NetworkSpecific_NW2 ) );
	if( PnlComGetMachineSetting()->getCoverageRateKeyInfo() ) {				// トナーカバレージ率の表示・非表示情報の取得処理.
		Key_GotoUtilityTotalCoverageRate.showEnable( True );					// トナーカバレージボタンを表示にする．
	}
	else {
		Key_GotoUtilityTotalCoverageRate.showEnable( False );					// トナーカバレージボタンを非表示にする．
	}
//	}
// 2017/06/20 SCD2-Sky Shindo [SecV2-x] セールスカウンター画面のカウンター値非表示対応.
	if( False != PnlComGetSystemSetting()->getSecurityLevelUpOn()
	 && NULL == PnlComGetUser()
	) {
		// セキュリティ強化モードが有効 かつ 認証中でない場合.
		Nfd_UtilityTotalCopyBlackTotal.showEnable ( False );			// コピー・ブラック・トータルカウンタ値.
		Nfd_UtilityTotalCopyBlackLargeSize.showEnable ( False );		// コピー・ブラック・大サイズカウンタ値 .
		Nfd_UtilityTotalCopyBlackCopyPrinter.showEnable ( False );		// コピー・ブラック・カラートータル（コピー＋プリンタ）カウンタ値 .
		Nfd_UtilityTotalCopyFullColorTotal.showEnable ( False );		// コピー・フルカラー・トータルカウンタ値 .
		Nfd_UtilityTotalCopyFullColorLargeSize.showEnable ( False );	// コピー・フルカラー・大サイズカウンタ値 .
		Nfd_UtilityTotalCopyFullColorCopyPrinter.showEnable ( False );	// コピー・フルカラー・カラートータル（コピー＋プリンタ）カウンタ値 .
		Nfd_UtilityTotalCopyMonoColorTotal.showEnable ( False );		// コピー・モノカラー・トータルカウンタ値 .
		Nfd_UtilityTotalCopyMonoColorLargeSize.showEnable ( False );	// コピー・モノカラー・大サイズカウンタ値 .
		Txt_UtilityTotalCopyMonoColorLargeSize.showEnable ( False );	// コピー・モノカラー・カラートータル（コピー＋プリンタ）カウンタ値  .
		Nfd_UtilityTotalCopy2ColorTotal.showEnable ( False );			// コピー・２色・トータルカウンタ値 .
		Nfd_UtilityTotalCopy2ColorLargeSize.showEnable ( False );		// コピー・２色・大サイズカウンタ値 .
		Nfd_UtilityTotalCopy2ColorCopyPrinter.showEnable ( False );		// コピー・２色・カラートータル（コピー＋プリンタ）カウンタ値 .
		Nfd_UtilityTotalCopyColorTotalTotal.showEnable ( False );		// コピー・全色合計・トータルカウンタ値 .
		Nfd_UtilityTotalCopyColorTotalLargeSize.showEnable ( False );	// コピー・全色合計・大サイズカウンタ値 .
		Txt_UtilityTotalCopyColorTotalLargeSize.showEnable ( False );	// コピー・全色合計・カラートータル（コピー＋プリンタ）カウンタ値 .
		
		Txt_UtilityTotalCopyBlackTotalHide.showEnable ( True );					// 「*****」.
		Txt_UtilityTotalCopyBlackLargeSizeHide.showEnable ( True );				// 「*****」.
		Txt_UtilityTotalCopyBlackCopyPrinterHide.showEnable ( True );			// 「*****」.
		Txt_UtilityTotalCopyFullColorTotalHide.showEnable ( True );				// 「*****」.
		Txt_UtilityTotalCopyFullColorLargeSizeHide.showEnable ( True );			// 「*****」.
		Txt_UtilityTotalCopyFullColorCopyPrinterHide.showEnable ( True );		// 「*****」.
		Txt_UtilityTotalCopyMonoColorTotalHide.showEnable ( True );				// 「*****」.
		Txt_UtilityTotalCopyMonoColorLargeSizeHide.showEnable ( True );			// 「*****」.
		Txt_UtilityTotalCopyMonoColorCopyPrinterHide.showEnable ( True );		// 「*****」.
		Txt_UtilityTotalCopy2ColorTotalHide.showEnable ( True );				// 「*****」.
		Txt_UtilityTotalCopy2ColorLargeSizeHide.showEnable ( True );			// 「*****」.
		Txt_UtilityTotalCopy2ColorCopyPrinterHide.showEnable ( True );			// 「*****」.
		Txt_UtilityTotalCopyColorTotalTotalHide.showEnable ( True );			// 「*****」.
		Txt_UtilityTotalCopyColorTotalLargeSizeHide.showEnable ( True );		// 「*****」.
		Txt_UtilityTotalCopyColorTotalCopyPrinterHide.showEnable ( True );		// 「*****」.
		
		Txt_UtilityTotalComMessage.showEnable ( True );

	} else {
		Nfd_UtilityTotalCopyBlackTotal.showEnable ( True );				// コピー・ブラック・トータルカウンタ値.
		Nfd_UtilityTotalCopyBlackLargeSize.showEnable ( True );			// コピー・ブラック・大サイズカウンタ値 .
		Nfd_UtilityTotalCopyBlackCopyPrinter.showEnable ( True );		// コピー・ブラック・カラートータル（コピー＋プリンタ）カウンタ値 .
		Nfd_UtilityTotalCopyFullColorTotal.showEnable ( True );			// コピー・フルカラー・トータルカウンタ値 .
		Nfd_UtilityTotalCopyFullColorLargeSize.showEnable ( True );		// コピー・フルカラー・大サイズカウンタ値 .
		Nfd_UtilityTotalCopyFullColorCopyPrinter.showEnable ( True );	// コピー・フルカラー・カラートータル（コピー＋プリンタ）カウンタ値 .
		Nfd_UtilityTotalCopyMonoColorTotal.showEnable ( True );			// コピー・モノカラー・トータルカウンタ値 .
		Nfd_UtilityTotalCopyMonoColorLargeSize.showEnable ( True );		// コピー・モノカラー・大サイズカウンタ値 .
		Txt_UtilityTotalCopyMonoColorLargeSize.showEnable ( True );		// コピー・モノカラー・カラートータル（コピー＋プリンタ）カウンタ値  .
		Nfd_UtilityTotalCopy2ColorTotal.showEnable ( True );			// コピー・２色・トータルカウンタ値 .
		Nfd_UtilityTotalCopy2ColorLargeSize.showEnable ( True );		// コピー・２色・大サイズカウンタ値 .
		Nfd_UtilityTotalCopy2ColorCopyPrinter.showEnable ( True );		// コピー・２色・カラートータル（コピー＋プリンタ）カウンタ値 .
		Nfd_UtilityTotalCopyColorTotalTotal.showEnable ( True );		// コピー・全色合計・トータルカウンタ値 .
		Nfd_UtilityTotalCopyColorTotalLargeSize.showEnable ( True );	// コピー・全色合計・大サイズカウンタ値 .
		Txt_UtilityTotalCopyColorTotalLargeSize.showEnable ( True );	// コピー・全色合計・カラートータル（コピー＋プリンタ）カウンタ値 .

		Txt_UtilityTotalCopyBlackTotalHide.showEnable ( False );				// 「*****」.
		Txt_UtilityTotalCopyBlackLargeSizeHide.showEnable ( False );			// 「*****」.
		Txt_UtilityTotalCopyBlackCopyPrinterHide.showEnable ( False );			// 「*****」.
		Txt_UtilityTotalCopyFullColorTotalHide.showEnable ( False );			// 「*****」.
		Txt_UtilityTotalCopyFullColorLargeSizeHide.showEnable ( False );		// 「*****」.
		Txt_UtilityTotalCopyFullColorCopyPrinterHide.showEnable ( False );		// 「*****」.
		Txt_UtilityTotalCopyMonoColorTotalHide.showEnable ( False );			// 「*****」.
		Txt_UtilityTotalCopyMonoColorLargeSizeHide.showEnable ( False );		// 「*****」.
		Txt_UtilityTotalCopyMonoColorCopyPrinterHide.showEnable ( False );		// 「*****」.
		Txt_UtilityTotalCopy2ColorTotalHide.showEnable ( False );				// 「*****」.
		Txt_UtilityTotalCopy2ColorLargeSizeHide.showEnable ( False );			// 「*****」.
		Txt_UtilityTotalCopy2ColorCopyPrinterHide.showEnable ( False );			// 「*****」.
		Txt_UtilityTotalCopyColorTotalTotalHide.showEnable ( False );			// 「*****」.
		Txt_UtilityTotalCopyColorTotalLargeSizeHide.showEnable ( False );		// 「*****」.
		Txt_UtilityTotalCopyColorTotalCopyPrinterHide.showEnable ( False );		// 「*****」.

		Txt_UtilityTotalComMessage.showEnable ( False );
	}

}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ（コピー） : 画面開始処理
//================================================================================================
void PUtilityTotalCopy::startGamen()
{
	// 親クラスの画面開始処理をコール.
	PUtilityTotalCom::startGamen();
// Comment 2019/01/24 【HCL_Krishna】【IT6_EagleH】【RQ-506】[WP-1710] Enter the highlight Display block mode.
	if( True == PJobListControl::isSelectedLeftArea( TYPD_LeftAreaSoft10Key ) ) {
		LIBG_EnterHighlightDisplayHideSection();
	}
// comment 2016/02/02 HCL_Hanuman [ZeusS_ZX0][CQ524280] Removed unnecessary code.
//	if ( False == PnlComGetSystemStatus()->getUserChoiceChanging() ) {
//		PnlComGetSystemStatus()->setUserChoiceChanging(True);
//	}
	
	setLeftAreaEnableType( TYPD_LeftAreaSalesCounter | TYPD_LeftAreaSoft10Key );
	// テンキー表示キーを上部に配置変更する.
	Key_LeftIndexSoft10Key.changePosition( 8, 8 );

	c_autoRestTimer.stopEachJobAutoResetTimer();
}

//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ（コピー） : 画面終了処理.
//================================================================================================
void PUtilityTotalCopy::endGamen()
{
// Comment 2019/01/24 【HCL_Krishna】【IT6_EagleH】【RQ-506】[WP-1710] Leave the highlight Display block mode.
// comment 2019//8/27 Sky雜賀 [EagleH][OP_TS-11728].
//	if( True == PJobListControl::isSelectedLeftArea( TYPD_LeftAreaSoft10Key ) ) {
//		LIBG_LeaveHighlightDisplayHideSection();
//	}
	LIBG_LeaveHighlightDisplayHideSection();
	Key_LeftIndexSoft10Key.changePosition( 8, 88 );
	PBasicMessageProc::endGamen();
}

//================================================================================================.
// 概要		:ユーティリティ・セールスカウンタ（プリンタ）: 表示画面のセットアップ.
//================================================================================================.
void PUtilityTotalPrinter::setupGamen(
    short option )
{
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items starts
 	if ( IsSFP() ) {
    	Key_UtilityTabUnSelCopy.showEnable(False);
    	Txt_UtilityTotalPrinterFullColorCopyPrinterHide.showEnable(False);
    	Txt_UtilityTotalPrinter2ColorCopyPrinterHide.showEnable(False);
    	Txt_UtilityTotalPrinterBlackCopyPrinterHide.showEnable(False);
		Bak_UtilityTopFullColorCopyCounter.showEnable(False);
		Bak_UtilityTopFullColorCopyCounter2.showEnable(False);
		Nfd_UtilityTotalPrinterBlackCopyPrinter.changeText(s_EXPNothingBar);
		Nfd_UtilityTotalPrinterFullColorCopyPrinter.changeText(s_EXPNothingBar);
		Nfd_UtilityTotalPrinter2ColorCopyPrinter.changeText(s_EXPNothingBar);
  	} else {
    	Key_UtilityTabUnSelCopy.showEnable(True);
    	Txt_UtilityTotalPrinterBlackCopyPrinterHide.showEnable(True);
    	Txt_UtilityTotalPrinterFullColorCopyPrinterHide.showEnable(True);
    	Txt_UtilityTotalPrinter2ColorCopyPrinterHide.showEnable(True);
		Bak_UtilityTopFullColorCopyCounter.showEnable(True);
		Bak_UtilityTopFullColorCopyCounter2.showEnable(True);
 	}
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items ends
	Bool bl_EnableHDDExistFunc = True;				// HDDレス時有効フラグ.
	if( False == PnlComIsUseSsdScanSend() ) {	// HDD/SSD使用不可.
		bl_EnableHDDExistFunc = False;
	}

	PUtilityTotalCom::setupGamen( option );
	if( onlyPanel.isFaxModekind() == True ) {
		Key_UtilityTabUnSelScanFax.changeText( s_KEYScannerFax );
	} else {
		Key_UtilityTabUnSelScanFax.changeText( s_KEYScanner_02 );
	}
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items starts
 	if ( IsSFP() ) {
		Key_UtilityTabUnSelScanFax.showEnable( False );			// ファクス/スキャンキー.
	} else {
		Key_UtilityTabUnSelScanFax.showEnable( bl_EnableHDDExistFunc );			// ファクス/スキャンキー.
	}
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items ends
// 2014/12/25 [MAv3][RQ-245] トータルカウンターの異常値対応 トータルカウンターの履歴を残す 日立 村上(幸).
// comment 2015/10/30 [CQ518309] セールスカウンタ履歴のレイアウト修正 日立 村上(幸).
//	if( True == onlyPanel.m_bl_MagicSequenceTotalCounterHistory ) {
//		char							sc_DateTimeBuf[PNLD_HistoryListDateTimeMaxLen];	// 日時格納バッファ.
//		TYPS_BackupRestore_TotalCounter s_HistoryList;
//		memset( &s_HistoryList, 0x00, sizeof( s_HistoryList ) );
//		memset( sc_DateTimeBuf, 0x00, sizeof( sc_DateTimeBuf ) );
//		gp_UtilityTotalCounterHistory.getTotalCounterHistory( &s_HistoryList, sc_DateTimeBuf );
//
//		// パーツの表示/非表示.
//		Txt_UtilityTotalSerialNumberMo.showEnable( False );																// シリアル番号を非表示.
//		Txt_UtilityTotalSerialNumbers_ColonMo.showEnable( False );														// 「:」を非表示.
//		Tfd_UtilityTotalSerialNumberMo.showEnable( False );																// シリアル番号を非表示.
//		Txt_UtilityTotalCounterHistory.showEnable( True );																// セールスカウンター履歴を表示.
//		Txt_UtilityTotalCounterHistory_Colon.showEnable( True );														// 「:」を表示.
//		Tfd_UtilityTotalCounterHistory.showEnable( True );																// 履歴日時を表示.
//		Tfd_UtilityTotalCounterHistory.changeExternalText( (uchar *)sc_DateTimeBuf );									// 日時.
//		Key_GotoUtilityTotalCoverageRate.showEnable( False );	// カバレッジカウンターキーを非表示.
//		Key_GotoUtilityTotalEcoIndex.showEnable( False );		// Eco指標ボタンを非表示.
//		Key_UtilityTabUnSelOther.showEnable( False );			// その他ボタンを非表示.
//
//		// 選択された履歴番号のトータルカウンタ情報を表示する.
//		// プリンタ・ブラック・トータルカウンタ値.
//		Nfd_UtilityTotalPrinterBlackTotal			= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_PrintTotalBlack];		// プリント・トータル・ブラック.
//		// プリンタ・ブラック・大サイズカウンタ値.
//		Nfd_UtilityTotalPrinterBlackLargeSize		= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_PrintLargeBlack];		// プリント・大サイズ・ブラック.
//		// プリンタ・ブラック・カラートータル（コピー＋プリンタ）カウンタ値.
//		Nfd_UtilityTotalPrinterBlackCopyPrinter		= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_CopyTotalBlack]			// コピー・トータル・ブラック.
//														+ s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_PrintTotalBlack];			// プリント・トータル・ブラック.
//		// プリンタ・フルカラー・トータルカウンタ値.
//		Nfd_UtilityTotalPrinterFullColorTotal		= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_PrintTotalFullColor];	// プリント・トータル・フルカラー.
//		// プリンタ・フルカラー・大サイズカウンタ値.
//		Nfd_UtilityTotalPrinterFullColorLargeSize	= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_PrintLargeFullColor];	// プリント・大サイズ・フルカラー.
//		// プリンタ・フルカラー・カラートータル（コピー＋プリンタ）カウンタ値.
//		Nfd_UtilityTotalPrinterFullColorCopyPrinter	= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_CopyTotalFullColor]		// コピー・トータル・フルカラー.
//														+ s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_PrintTotalFullColor];		// プリント・トータル・フルカラー.
//		// プリンタ・２色・トータルカウンタ値.
//		Nfd_UtilityTotalPrinter2ColorTotal			= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_PrintTotal2Color];		// プリント・トータル・２色.
//		// プリンタ・２色・大サイズカウンタ値.
//		Nfd_UtilityTotalPrinter2ColorLargeSize		= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_PrintLarge2Color];		// プリント・大サイズ・２色.
//		// プリンタ・２色・カラートータル（コピー＋プリンタ）カウンタ値.
//		Nfd_UtilityTotalPrinter2ColorCopyPrinter	= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_CopyTotal2Color]			// コピー・トータル・２色.
//														+ s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_PrintTotal2Color];			// プリント・トータル・２色.
//		// プリンタ・全色合計・トータルカウンタ値.
//		Nfd_UtilityTotalPrinterColorTotalTotal		= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_PrintTotalTotal];		// プリンタ・全色合計・トータルカウンタ値.
//		// プリンタ・全色合計・大サイズカウンタ値.
//		Nfd_UtilityTotalPrinterColorTotalLargeSize	= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_PrintLargeTotal];		// プリンタ・全色合計・大サイズカウンタ値.
//	}
//	else {
//		// パーツの表示/非表示.
//		Txt_UtilityTotalSerialNumberMo.showEnable( True );																// シリアル番号を表示.
//		Txt_UtilityTotalSerialNumbers_ColonMo.showEnable( True );														// 「:」を表示.
//		Tfd_UtilityTotalSerialNumberMo.showEnable( True );																// シリアル番号を表示.
//		Txt_UtilityTotalCounterHistory.showEnable( False );																// セールスカウンター履歴を非表示.
//		Txt_UtilityTotalCounterHistory_Colon.showEnable( False );														// 「:」を非表示.
//		Tfd_UtilityTotalCounterHistory.showEnable( False );																// 履歴日時を非表示.

	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	char uc_SerialNumberPRT[TYPD_MAX_SerialNoPrinter];
	memset( uc_SerialNumberPRT, '\0', sizeof( uc_SerialNumberPRT ) );
	pc_SystemInfo->getSerialNoPrinter( ( char* )uc_SerialNumberPRT );
	Tfd_UtilityTotalSerialNumberMo.changeExternalText( ( uchar* )uc_SerialNumberPRT, sizeof( uc_SerialNumberPRT ) );		// シリアル番号・プリンタ
	// プリンタ・ブラック・トータルカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalPrinterBlackTotal			= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotalBlack ) 		// プリント・トータル・ブラック
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotalBlack,TYPE_NetworkSpecific_NW2 ) );		// プリント・トータル・ブラック
	// プリンタ・ブラック・大サイズカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalPrinterBlackLargeSize		= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntPrintLargeBlack ) 		// プリント・大サイズ・ブラック
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintLargeBlack,TYPE_NetworkSpecific_NW2 ) );		// プリント・大サイズ・ブラック
	// プリンタ・ブラック・カラートータル（コピー＋プリンタ）カウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items starts
 	if ( ! IsSFP() ) {
			Nfd_UtilityTotalPrinterBlackCopyPrinter		= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalBlack )		// コピー・トータル・ブラック
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotalBlack )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalBlack, TYPE_NetworkSpecific_NW2 )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotalBlack, TYPE_NetworkSpecific_NW2 ) );			// プリント・トータル・ブラック
    }
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items ends
	// プリンタ・フルカラー・トータルカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalPrinterFullColorTotal		= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotalFullColor ) 	// プリント・トータル・フルカラー
            + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotalFullColor,TYPE_NetworkSpecific_NW2 ) );	// プリント・トータル・フルカラー
	// プリンタ・フルカラー・大サイズカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalPrinterFullColorLargeSize	= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntPrintLargeFullColor ) 	// プリント・大サイズ・フルカラー
            + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintLargeFullColor,TYPE_NetworkSpecific_NW2 ) );	// プリント・大サイズ・フルカラー
	// プリンタ・フルカラー・カラートータル（コピー＋プリンタ）カウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items starts
 	if ( ! IsSFP() ) {
			Nfd_UtilityTotalPrinterFullColorCopyPrinter	= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalFullColor )	// コピー・トータル・フルカラー
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotalFullColor)		// プリント・トータル・フルカラー
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotalFullColor , TYPE_NetworkSpecific_NW2 )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotalFullColor, TYPE_NetworkSpecific_NW2) );
    }
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items ends
	// プリンタ・２色・トータルカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalPrinter2ColorTotal			= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotal2Color ) 		// プリント・トータル・２色
            + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotal2Color, TYPE_NetworkSpecific_NW2 ) );		// プリント・トータル・２色
	// プリンタ・２色・大サイズカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalPrinter2ColorLargeSize		= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntPrintLarge2Color ) 		// プリント・大サイズ・２色
            + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintLarge2Color,TYPE_NetworkSpecific_NW2) ) ;		// プリント・大サイズ・２色
	// プリンタ・２色・カラートータル（コピー＋プリンタ）カウン
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items starts
 	if ( ! IsSFP() ) {
			Nfd_UtilityTotalPrinter2ColorCopyPrinter	= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotal2Color )		// コピー・トータル・２色
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotal2Color )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyTotal2Color,TYPE_NetworkSpecific_NW2 )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotal2Color,TYPE_NetworkSpecific_NW2 ) );		// プリント・トータル・２色
    }
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items ends
	// プリンタ・全色合計・トータルカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalPrinterColorTotalTotal		= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotalBlack )		// プリント・トータル・ブラック
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotalFullColor )		// プリント・トータル・フルカラー
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotal2Color )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotalBlack, TYPE_NetworkSpecific_NW2 )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotalFullColor, TYPE_NetworkSpecific_NW2 )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintTotal2Color, TYPE_NetworkSpecific_NW2 ) );		// プリント・トータル・２色
	// プリンタ・全色合計・大サイズカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalPrinterColorTotalLargeSize	= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntPrintLargeBlack )		// プリント・大サイズ・ブラック
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintLargeFullColor )		// プリント・大サイズ・フルカラー
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintLarge2Color )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintLargeBlack, TYPE_NetworkSpecific_NW2 )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintLargeFullColor, TYPE_NetworkSpecific_NW2 )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintLarge2Color, TYPE_NetworkSpecific_NW2 ) );		// プリント・大サイズ・２色

	if( PnlComGetMachineSetting()->getCoverageRateKeyInfo() ) {				// トナーカバレージ率の表示・非表示情報の取得処理.
		Key_GotoUtilityTotalCoverageRate.showEnable( True );					// トナーカバレージボタンを表示にする．
	}
	else {
		Key_GotoUtilityTotalCoverageRate.showEnable( False );					// トナーカバレージボタンを非表示にする．
	}
//	}
// 2017/06/20 SCD2-Sky Shindo [SecV2-x] セールスカウンター画面のカウンター値非表示対応.
	if( False != PnlComGetSystemSetting()->getSecurityLevelUpOn()
	 && NULL == PnlComGetUser()
	) {
		// セキュリティ強化モードが有効 かつ 認証中でない場合.
		Nfd_UtilityTotalPrinterBlackTotal.showEnable( False );				// プリント・ブラック ・トータル.
		Nfd_UtilityTotalPrinterBlackLargeSize.showEnable( False );			// プリント・ブラック ・大サイズ.
		Nfd_UtilityTotalPrinterBlackCopyPrinter.showEnable( False );		// プリント・ブラック・トータル（コピー＋プリンタ）.
		Nfd_UtilityTotalPrinterFullColorTotal.showEnable( False );			// プリント・フルカラー ・トータル.
		Nfd_UtilityTotalPrinterFullColorLargeSize.showEnable( False );		// プリント・フルカラー ・大サイズ.
		Nfd_UtilityTotalPrinterFullColorCopyPrinter.showEnable( False );	// プリント・フルカラー・トータル（コピー＋プリンタ）.
		Nfd_UtilityTotalPrinter2ColorTotal.showEnable( False );				// プリント・２色 ・トータル.
		Nfd_UtilityTotalPrinter2ColorLargeSize.showEnable( False );			// プリント・２色 ・大サイズ.
		Nfd_UtilityTotalPrinter2ColorCopyPrinter.showEnable( False );		// プリント・２色 ・トータル（コピー＋プリンタ）.
		Nfd_UtilityTotalPrinterColorTotalTotal.showEnable( False );			// プリント・全色合計 ・トータル.
		Nfd_UtilityTotalPrinterColorTotalLargeSize.showEnable( False );		// プリント・全色合計 ・大サイズ.
		Txt_UtilityTotalPrinterColorTotalColorTotal.showEnable( False );	// プリント・全色合計 ・トータル（コピー＋プリンタ）.

		Txt_UtilityTotalPrinterBlackTotalHide.showEnable ( True );				// 「*****」.
		Txt_UtilityTotalPrinterBlackLargeSizeHide.showEnable ( True );			// 「*****」.
		Txt_UtilityTotalPrinterBlackCopyPrinterHide.showEnable ( True );		// 「*****」.
		Txt_UtilityTotalPrinterFullColorTotalHide.showEnable ( True );			// 「*****」.
		Txt_UtilityTotalPrinterFullColorLargeSizeHide.showEnable ( True );		// 「*****」.
		Txt_UtilityTotalPrinterFullColorCopyPrinterHide.showEnable ( True );	// 「*****」.
		Txt_UtilityTotalPrinter2ColorTotalHide.showEnable ( True );				// 「*****」.
		Txt_UtilityTotalPrinter2ColorLargeSizeHide.showEnable ( True );			// 「*****」.
		Txt_UtilityTotalPrinter2ColorCopyPrinterHide.showEnable ( True );		// 「*****」.
		Txt_UtilityTotalPrinterColorTotalTotalHide.showEnable ( True );			// 「*****」.
		Txt_UtilityTotalPrinterColorTotalLargeSizeHide.showEnable ( True );		// 「*****」.
		Txt_UtilityTotalPrinterColorTotalColorTotalHide.showEnable ( True );	// 「*****」.
		
		Txt_UtilityTotalComMessage.showEnable ( True );

	} else {
		Nfd_UtilityTotalPrinterBlackTotal.showEnable( True );				// プリント・ブラック ・トータル.
		Nfd_UtilityTotalPrinterBlackLargeSize.showEnable( True );			// プリント・ブラック ・大サイズ.
		Nfd_UtilityTotalPrinterBlackCopyPrinter.showEnable( True );			// プリント・ブラック・トータル（コピー＋プリンタ）.
		Nfd_UtilityTotalPrinterFullColorTotal.showEnable( True );			// プリント・フルカラー ・トータル.
		Nfd_UtilityTotalPrinterFullColorLargeSize.showEnable( True );		// プリント・フルカラー ・大サイズ.
		Nfd_UtilityTotalPrinterFullColorCopyPrinter.showEnable( True );		// プリント・フルカラー・トータル（コピー＋プリンタ）.
		Nfd_UtilityTotalPrinter2ColorTotal.showEnable( True );				// プリント・２色 ・トータル.
		Nfd_UtilityTotalPrinter2ColorLargeSize.showEnable( True );			// プリント・２色 ・大サイズ.
		Nfd_UtilityTotalPrinter2ColorCopyPrinter.showEnable( True );		// プリント・２色 ・トータル（コピー＋プリンタ）.
		Nfd_UtilityTotalPrinterColorTotalTotal.showEnable( True );			// プリント・全色合計 ・トータル.
		Nfd_UtilityTotalPrinterColorTotalLargeSize.showEnable( True );		// プリント・全色合計 ・大サイズ.
		Txt_UtilityTotalPrinterColorTotalColorTotal.showEnable( True );		// プリント・全色合計 ・トータル（コピー＋プリンタ）.

		Txt_UtilityTotalPrinterBlackTotalHide.showEnable ( False );				// 「*****」.
		Txt_UtilityTotalPrinterBlackLargeSizeHide.showEnable ( False );			// 「*****」.
		Txt_UtilityTotalPrinterBlackCopyPrinterHide.showEnable ( False );		// 「*****」.
		Txt_UtilityTotalPrinterFullColorTotalHide.showEnable ( False );			// 「*****」.
		Txt_UtilityTotalPrinterFullColorLargeSizeHide.showEnable ( False );		// 「*****」.
		Txt_UtilityTotalPrinterFullColorCopyPrinterHide.showEnable ( False );	// 「*****」.
		Txt_UtilityTotalPrinter2ColorTotalHide.showEnable ( False );			// 「*****」.
		Txt_UtilityTotalPrinter2ColorLargeSizeHide.showEnable ( False );		// 「*****」.
		Txt_UtilityTotalPrinter2ColorCopyPrinterHide.showEnable ( False );		// 「*****」.
		Txt_UtilityTotalPrinterColorTotalTotalHide.showEnable ( False );		// 「*****」.
		Txt_UtilityTotalPrinterColorTotalLargeSizeHide.showEnable ( False );	// 「*****」.
		Txt_UtilityTotalPrinterColorTotalColorTotalHide.showEnable ( False );	// 「*****」.
		
		Txt_UtilityTotalComMessage.showEnable ( False );
	}

}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ（プリンタ） : 画面開始処理
//================================================================================================
void PUtilityTotalPrinter::startGamen()
{
	// 親クラスの画面開始処理をコール.
	PUtilityTotalCom::startGamen();
// Comment 2019/01/24 【HCL_Krishna】【IT6_EagleH】【RQ-506】[WP-1710] Enter the highlight Display block mode.
	if( True == PJobListControl::isSelectedLeftArea( TYPD_LeftAreaSoft10Key ) ) {
		LIBG_EnterHighlightDisplayHideSection();
	}
	setLeftAreaEnableType( TYPD_LeftAreaSalesCounter | TYPD_LeftAreaSoft10Key );
	// テンキー表示キーを上部に配置変更する.
	Key_LeftIndexSoft10Key.changePosition( 8, 8 );
}

//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ（プリンタ） : 画面終了処理.
//================================================================================================
void PUtilityTotalPrinter::endGamen()
{
// Comment 2019/01/24 【HCL_Krishna】【IT6_EagleH】【RQ-506】[WP-1710] Leave the highlight Display block mode.
// comment 2019//8/27 Sky雜賀 [EagleH][OP_TS-11728].
//	if( True == PJobListControl::isSelectedLeftArea( TYPD_LeftAreaSoft10Key ) ) {
//		LIBG_LeaveHighlightDisplayHideSection();
//	}
	LIBG_LeaveHighlightDisplayHideSection();
	Key_LeftIndexSoft10Key.changePosition( 8, 88 );
	PBasicMessageProc::endGamen();
}

//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ（スキャン／ファクス）: 表示画面のセットアップ
//================================================================================================
void PUtilityTotalScanFax::setupGamen(
    short option )
{
	PUtilityTotalCom::setupGamen( option );
	if( onlyPanel.isFaxModekind() == True ) {
		Bak_UtilityTabSelScanFax.changeText( s_ITMScannerFax );
	} else {
		Bak_UtilityTabSelScanFax.changeText( s_KEYScanner_02 );
	}

// 2014/12/25 [MAv3][RQ-245] トータルカウンターの異常値対応 トータルカウンターの履歴を残す 日立 村上(幸).
// comment 2015/10/30 [CQ518309] セールスカウンタ履歴のレイアウト修正 日立 村上(幸).
//	if( True == onlyPanel.m_bl_MagicSequenceTotalCounterHistory ) {
//		char							sc_DateTimeBuf[PNLD_HistoryListDateTimeMaxLen];	// 日時格納バッファ.
//		TYPS_BackupRestore_TotalCounter s_HistoryList;
//		memset( &s_HistoryList, 0x00, sizeof( s_HistoryList ) );
//		memset( sc_DateTimeBuf, 0x00, sizeof( sc_DateTimeBuf ) );
//		gp_UtilityTotalCounterHistory.getTotalCounterHistory( &s_HistoryList, sc_DateTimeBuf );
//
//		// パーツの表示/非表示.
//		Txt_UtilityTotalSerialNumberMo.showEnable( False );																// シリアル番号を非表示.
//		Txt_UtilityTotalSerialNumbers_ColonMo.showEnable( False );														// 「:」を非表示.
//		Tfd_UtilityTotalSerialNumberMo.showEnable( False );																// シリアル番号を非表示.
//		Txt_UtilityTotalCounterHistory.showEnable( True );																// セールスカウンター履歴を表示.
//		Txt_UtilityTotalCounterHistory_Colon.showEnable( True );														// 「:」を表示.
//		Tfd_UtilityTotalCounterHistory.showEnable( True );																// 履歴日時を表示.
//		Tfd_UtilityTotalCounterHistory.changeExternalText( (uchar *)sc_DateTimeBuf );									// 日時.
//		Key_GotoUtilityTotalCoverageRate.showEnable( False );	// カバレッジカウンターキーを非表示.
//		Key_GotoUtilityTotalEcoIndex.showEnable( False );		// Eco指標ボタンを非表示.
//		Key_UtilityTabUnSelOther.showEnable( False );			// その他ボタンを非表示.
//
//		// 選択された履歴番号のトータルカウンタ情報を表示する.
//		// スキャン／ファクス・プリントブラック・トータルカウンタ値.
//		Nfd_UtilityTotalScanFaxPrintTotalBlack		= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_ScanPrintTotalBlack];		// FAX/スキャン トータル ブラック.
//		// スキャン／ファクス・プリントブラック・大サイズカウンタ値.
//		Nfd_UtilityTotalScanFaxPrintLargeSizeBlack	= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_ScanPrintLargeBlack];		// FAX/スキャン 大サイズ ブラック.
//		// スキャン／ファクス・プリントカラー・トータルカウンタ値.
//		Nfd_UtilityTotalScanFaxPrintTotalColor		= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_ScanPrintTotalFullColor];	// FAX/スキャン トータル フルカラー.
//		// スキャン／ファクス・プリントカラー・大サイズカウンタ値.
//		Nfd_UtilityTotalScanFaxPrintLargeSizeColor	= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_ScanPrintLargeFullColor];	// FAX/スキャン 大サイズ フルカラー
//		// スキャン／ファクス・読取り・トータルカウンタ値.
//		Nfd_UtilityTotalScanFaxScanTotal			= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_FaxScanTotal];				// FAX/スキャン トータル 読み取り.
//		// スキャン／ファクス・読取り・大サイズカウンタ値.
//		Nfd_UtilityTotalScanFaxScanLargeSize		= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_FaxScanLarge];				// FAX/スキャン 大サイズ 読み取り.
//														
//		// スキャン／ファクス・ファクス送信枚数・トータルカウンタ値.
//		Btx_UtilityTotalScanFaxSend					= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_FaxSendTotal];
//		// スキャン／ファクス・ファクス受信枚数・トータルカウンタ値.
//		Btx_UtilityTotalScanFaxReceive				= (float)s_HistoryList.ul_Total[TYPE_BackupRestoreTotalCnt_FaxReceiveTotal];
//
//		if ( onlyPanel.isFaxModekind() == True ){	// ファクスオプション装着時.
//			Btx_UtilityTotalScanFaxReceive.showEnable( True );				// ファクス受信枚数の背景.
//			Btx_UtilityTotalScanFaxSend.showEnable( True );					// ファクス送信枚数の背景.
//		} else {									// ファクスオプション非装着時.
//			Btx_UtilityTotalScanFaxReceive.showEnable( False );				// ファクス受信枚数の背景.
//			Btx_UtilityTotalScanFaxSend.showEnable( False );				// ファクス送信枚数の背景.
//
//		}
//	}
//	else {
//		// パーツの表示/非表示.
//		Txt_UtilityTotalSerialNumberMo.showEnable( True );																// シリアル番号を表示.
//		Txt_UtilityTotalSerialNumbers_ColonMo.showEnable( True );														// 「:」を表示.
//		Tfd_UtilityTotalSerialNumberMo.showEnable( True );																// シリアル番号を表示.
//		Txt_UtilityTotalCounterHistory.showEnable( False );																// セールスカウンター履歴を非表示.
//		Txt_UtilityTotalCounterHistory_Colon.showEnable( False );														// 「:」を非表示.
//		Tfd_UtilityTotalCounterHistory.showEnable( False );																// 履歴日時を非表示.

	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	char uc_SerialNumberPRT[TYPD_MAX_SerialNoPrinter];
	memset( uc_SerialNumberPRT, '\0', sizeof( uc_SerialNumberPRT ) );
	pc_SystemInfo->getSerialNoPrinter( ( char* )uc_SerialNumberPRT );
	Tfd_UtilityTotalSerialNumberMo.changeExternalText( ( uchar* )uc_SerialNumberPRT, sizeof( uc_SerialNumberPRT ) );		// シリアル番号・プリンタ
	// スキャン／ファクス・プリントブラック・トータルカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalScanFaxPrintTotalBlack		= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntScanPrintTotalBlack )		// スキャンプリントブラック ・トータル
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntFaxPrintTotal )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntScanPrintTotalBlack, TYPE_NetworkSpecific_NW2 )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntFaxPrintTotal, TYPE_NetworkSpecific_NW2 ) );				// ＦＡＸ・プリント・トータル
	// スキャン／ファクス・プリントブラック・大サイズカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalScanFaxPrintLargeSizeBlack	= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntScanPrintLargeBlack )		// スキャンプリントブラック・大サイズ
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntFaxPrintLarge )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntScanPrintLargeBlack, TYPE_NetworkSpecific_NW2 )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntFaxPrintLarge, TYPE_NetworkSpecific_NW2) );				// ＦＡＸ・プリント・大サイズ
	// スキャン／ファクス・プリントカラー・トータルカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalScanFaxPrintTotalColor		= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntScanPrintTotalFullColor ) 	// スキャンプリントカラー・トータル
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntScanPrintTotalFullColor, TYPE_NetworkSpecific_NW2 ) );	// スキャンプリントカラー・トータル
	// スキャン／ファクス・プリントカラー・大サイズカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalScanFaxPrintLargeSizeColor	= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntScanPrintLargeFullColor ) 	// スキャンプリントカラー・大サイズ
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntScanPrintLargeFullColor, TYPE_NetworkSpecific_NW2 ) );	// スキャンプリントカラー・大サイズ
	// スキャン／ファクス・読取り・トータルカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalScanFaxScanTotal			= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntScanTotal )					// スキャン・トータル
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntFaxScanTotal )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntScanTotal,TYPE_NetworkSpecific_NW2)
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntFaxScanTotal,TYPE_NetworkSpecific_NW2 ) );				// ＦＡＸ・読取・トータル
	// スキャン／ファクス・読取り・大サイズカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Nfd_UtilityTotalScanFaxScanLargeSize		= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntScanLarge )					// スキャン・大サイズ
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntFaxScanLarge )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntScanLarge ,TYPE_NetworkSpecific_NW2 )	
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntFaxScanLarge , TYPE_NetworkSpecific_NW2 ) );				// ＦＡＸ・読取・大サイズ

	// スキャン／ファクス・ファクス送信枚数・トータルカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Btx_UtilityTotalScanFaxSend					= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntFaxSendTotal ) )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntFaxSendTotal, TYPE_NetworkSpecific_NW2 );
	// スキャン／ファクス・ファクス受信枚数・トータルカウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	Btx_UtilityTotalScanFaxReceive				= static_cast<float>( PnlComGetTotalCnt()->get( TYPE_TotalCntFaxReceiveTotal ) )
	        + PnlComGetTotalCnt()->get( TYPE_TotalCntFaxReceiveTotal, TYPE_NetworkSpecific_NW2 );

	if( onlyPanel.isFaxModekind() == True ) {	// ファクスオプション装着時
		Btx_UtilityTotalScanFaxReceive.showEnable( True );				// ファクス受信枚数の背景.
		Btx_UtilityTotalScanFaxSend.showEnable( True );					// ファクス送信枚数の背景.
	} else {	// ファクスオプション非装着時
		Btx_UtilityTotalScanFaxReceive.showEnable( False );				// ファクス受信枚数の背景.
		Btx_UtilityTotalScanFaxSend.showEnable( False );				// ファクス送信枚数の背景.

	}

	if( PnlComGetMachineSetting()->getCoverageRateKeyInfo() ) {				// トナーカバレージ率の表示・非表示情報の取得処理.
		Key_GotoUtilityTotalCoverageRate.showEnable( True );					// トナーカバレージボタンを表示にする．
	}
	else {
		Key_GotoUtilityTotalCoverageRate.showEnable( False );					// トナーカバレージボタンを非表示にする．
	}
//	}
// 2017/06/20 SCD2-Sky Shindo [SecV2-x] セールスカウンター画面のカウンター値非表示対応.
	if( False != PnlComGetSystemSetting()->getSecurityLevelUpOn()
	 && NULL == PnlComGetUser()
	) {
		// セキュリティ強化モードが有効 かつ 認証中でない場合.
		Nfd_UtilityTotalScanFaxPrintTotalBlack.showEnable ( False );		// スキャン／ファクス・プリントブラック・トータルカウンタ値 .
		Nfd_UtilityTotalScanFaxPrintLargeSizeBlack.showEnable ( False );	// スキャン／ファクス・プリントブラック・大サイズカウンタ値 .
		Nfd_UtilityTotalScanFaxPrintTotalColor.showEnable ( False );		// スキャン／ファクス・プリントカラー・トータルカウンタ値 .
		Nfd_UtilityTotalScanFaxPrintLargeSizeColor.showEnable ( False );	// スキャン／ファクス・プリントカラー・大サイズカウンタ値 .
		Nfd_UtilityTotalScanFaxScanTotal.showEnable ( False );				// スキャン／ファクス・読取り・トータルカウンタ値 .
		Nfd_UtilityTotalScanFaxScanLargeSize.showEnable ( False );			// スキャン／ファクス・読取り・大サイズカウンタ値 .
		Nfd_UtilityTotalScanFaxSendTotal.showEnable ( False );				// スキャン／ファクス・ファクス送信枚数・トータルカウンタ値 .
		Txt_UtilityTotalScanFaxSendLargeSize.showEnable ( False );			// スキャン／ファクス・ファクス送信枚数・大サイズカウンタ値 .
		Nfd_UtilityTotalScanFaxReceiveTotal.showEnable ( False );			// スキャン／ファクス・ファクス受信枚数・トータルカウンタ値 .
		Txt_UtilityTotalScanFaxReceiveLargeSize.showEnable ( False );		// スキャン／ファクス・ファクス受信枚数・大サイズカウンタ値 .
		
		Txt_UtilityTotalScanFaxPrintTotalBlackHide.showEnable ( True );		// 「*****」.
		Txt_UtilityTotalScanFaxPrintLargeSizeBlackHide.showEnable ( True );	// 「*****」.
		Txt_UtilityTotalScanFaxPrintTotalColorHide.showEnable ( True );		// 「*****」.
		Txt_UtilityTotalScanFaxPrintLargeSizeColorHide.showEnable ( True );	// 「*****」.
		Txt_UtilityTotalScanFaxScanTotalHide.showEnable ( True );			// 「*****」.
		Txt_UtilityTotalScanFaxScanLargeSizeHide.showEnable ( True );		// 「*****」.
		if ( onlyPanel.isFaxModekind() == True ){	// ファクスオプション装着時 
			Txt_UtilityTotalScanFaxSendTotalHide.showEnable ( True );			// 「*****」.
			Txt_UtilityTotalScanFaxSendLargeSizeHide.showEnable ( True );		// 「*****」.
			Txt_UtilityTotalScanFaxReceiveTotalHide.showEnable ( True );		// 「*****」.
			Txt_UtilityTotalScanFaxReceiveLargeSizeHide.showEnable ( True );	// 「*****」.
		} else {
			Txt_UtilityTotalScanFaxSendTotalHide.showEnable ( False );			// 「*****」.
			Txt_UtilityTotalScanFaxSendLargeSizeHide.showEnable ( False );		// 「*****」.
			Txt_UtilityTotalScanFaxReceiveTotalHide.showEnable ( False );		// 「*****」.
			Txt_UtilityTotalScanFaxReceiveLargeSizeHide.showEnable ( False );	// 「*****」.			
		}
		
		Txt_UtilityTotalComMessage.showEnable ( True );

	} else {
		Nfd_UtilityTotalScanFaxPrintTotalBlack.showEnable ( True );			// スキャン／ファクス・プリントブラック・トータルカウンタ値 .
		Nfd_UtilityTotalScanFaxPrintLargeSizeBlack.showEnable ( True );		// スキャン／ファクス・プリントブラック・大サイズカウンタ値 .
		Nfd_UtilityTotalScanFaxPrintTotalColor.showEnable ( True );			// スキャン／ファクス・プリントカラー・トータルカウンタ値 .
		Nfd_UtilityTotalScanFaxPrintLargeSizeColor.showEnable ( True );		// スキャン／ファクス・プリントカラー・大サイズカウンタ値 .
		Nfd_UtilityTotalScanFaxScanTotal.showEnable ( True );				// スキャン／ファクス・読取り・トータルカウンタ値 .
		Nfd_UtilityTotalScanFaxScanLargeSize.showEnable ( True );			// スキャン／ファクス・読取り・大サイズカウンタ値 .
		Nfd_UtilityTotalScanFaxSendTotal.showEnable ( True );				// スキャン／ファクス・ファクス送信枚数・トータルカウンタ値 .
		Txt_UtilityTotalScanFaxSendLargeSize.showEnable ( True );			// スキャン／ファクス・ファクス送信枚数・大サイズカウンタ値 .
		Nfd_UtilityTotalScanFaxReceiveTotal.showEnable ( True );			// スキャン／ファクス・ファクス受信枚数・トータルカウンタ値 .
		Txt_UtilityTotalScanFaxReceiveLargeSize.showEnable ( True );		// スキャン／ファクス・ファクス受信枚数・大サイズカウンタ値 .
		
		Txt_UtilityTotalScanFaxPrintTotalBlackHide.showEnable ( False );		// 「*****」.
		Txt_UtilityTotalScanFaxPrintLargeSizeBlackHide.showEnable ( False );	// 「*****」.
		Txt_UtilityTotalScanFaxPrintTotalColorHide.showEnable ( False );		// 「*****」.
		Txt_UtilityTotalScanFaxPrintLargeSizeColorHide.showEnable ( False );	// 「*****」.
		Txt_UtilityTotalScanFaxScanTotalHide.showEnable ( False );				// 「*****」.
		Txt_UtilityTotalScanFaxScanLargeSizeHide.showEnable ( False );			// 「*****」.
		Txt_UtilityTotalScanFaxSendTotalHide.showEnable ( False );				// 「*****」.
		Txt_UtilityTotalScanFaxSendLargeSizeHide.showEnable ( False );			// 「*****」.
		Txt_UtilityTotalScanFaxReceiveTotalHide.showEnable ( False );			// 「*****」.
		Txt_UtilityTotalScanFaxReceiveLargeSizeHide.showEnable ( False );		// 「*****」.

		Txt_UtilityTotalComMessage.showEnable ( False );
	}

}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ（スキャン／ファクス） : 画面開始処理
//================================================================================================
void PUtilityTotalScanFax::startGamen()
{
	// 親クラスの画面開始処理をコール.
	PUtilityTotalCom::startGamen();
// Comment 2019/01/24 【HCL_Krishna】【IT6_EagleH】【RQ-506】[WP-1710] Enter the highlight Display block mode.
	if( True == PJobListControl::isSelectedLeftArea( TYPD_LeftAreaSoft10Key ) ) {
		LIBG_EnterHighlightDisplayHideSection();
	}
	// 左エリア種類の有効設定（ジョブ表示（通常） + セールスカウンター）.
	setLeftAreaEnableType( TYPD_LeftAreaSalesCounter | TYPD_LeftAreaSoft10Key );
	// テンキー表示キーを上部に配置変更する.
	Key_LeftIndexSoft10Key.changePosition( 8, 8 );
}

//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ（スキャン／ファクス） : 画面終了処理.
//================================================================================================
void PUtilityTotalScanFax::endGamen()
{
// Comment 2019/01/24 【HCL_Krishna】【IT6_EagleH】【RQ-506】[WP-1710] Leave the highlight Display block mode.
// comment 2019//8/27 Sky雜賀 [EagleH][OP_TS-11728].
//	if( True == PJobListControl::isSelectedLeftArea( TYPD_LeftAreaSoft10Key ) ) {
//		LIBG_LeaveHighlightDisplayHideSection();
//	}
	LIBG_LeaveHighlightDisplayHideSection();
	Key_LeftIndexSoft10Key.changePosition( 8, 88 );
	PBasicMessageProc::endGamen();
}
//================================================================================================.
// 概要		:ユーティリティ・セールスカウンタ（その他）: 表示画面のセットアップ.
//================================================================================================.
void PUtilityTotalOther::setupGamen(
    short option )
{
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items starts
 	if ( IsSFP() ) {
    	Key_UtilityTabUnSelCopy.showEnable(False);
		Bak_UtilityTopFullColorCopyCounter.showEnable(False);
		Bak_UtilityTopFullColorCopyCounter2.showEnable(False);
    } else {
    	Key_UtilityTabUnSelCopy.showEnable(True);
		Bak_UtilityTopFullColorCopyCounter.showEnable(True);
		Bak_UtilityTopFullColorCopyCounter2.showEnable(True);
   	}
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items ends
	Bool bl_EnableHDDExistFunc = True;				// HDDレス時有効フラグ.
	if( False == PnlComIsUseSsdScanSend() ) {	// HDD/SSD使用不可.
		bl_EnableHDDExistFunc = False;
	}

	PUtilityTotalCom::setupGamen( option );
	if( onlyPanel.isFaxModekind() == True ) {
		Key_UtilityTabUnSelScanFax.changeText( s_KEYScannerFax );
	} else {
		Key_UtilityTabUnSelScanFax.changeText( s_KEYScanner_02 );
	}
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items starts
 	if ( IsSFP() ) {
		Key_UtilityTabUnSelScanFax.showEnable( False );			// ファクス/スキャンキー.
	} else {
		Key_UtilityTabUnSelScanFax.showEnable( bl_EnableHDDExistFunc );			// ファクス/スキャンキー.
	}
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items ends

	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	char uc_SerialNumberPRT[TYPD_MAX_SerialNoPrinter];
	memset( uc_SerialNumberPRT, '\0', sizeof( uc_SerialNumberPRT ) );
	pc_SystemInfo->getSerialNoPrinter( ( char* )uc_SerialNumberPRT );
	Tfd_UtilityTotalSerialNumberMo.changeExternalText( ( uchar* )uc_SerialNumberPRT, sizeof( uc_SerialNumberPRT ) );		// シリアル番号・プリンタ
// 2017/06/20 SCD2-Sky Shindo [SecV2-x] セールスカウンター画面のカウンター値非表示対応.
	if( False != PnlComGetSystemSetting()->getSecurityLevelUpOn()
	 && NULL == PnlComGetUser()
	) {
//		Tfd_UtilityTotalOtherManuscript.changeText(s_pITMHideAsterisk);		// 原稿枚数.
		Tfd_UtilityTotalOtherManuscript.changeText( s_pITMHideAsterisk_W128H016I);		// 原稿枚数.
		Tfd_UtilityTotalOtherPaper.changeText(s_pITMHideAsterisk_W128H016I);			// 用紙枚数.
		Tfd_UtilityTotalOther2SideTotal.changeText(s_pITMHideAsterisk_W128H016I);		// 両面トータル .
		Tfd_UtilityTotalOtherPrintPaperNum.changeText(s_pITMHideAsterisk_W128H016I);	// 印刷枚数カウンター .
// comment 2022/08/24 FPT.DungDV14 [IT6_2.3] [機器構成変更_No.21] セールスカウンター画面のカウンター値非表示対応.
		Tfd_UtilityTotalOtherCumulativeCounter.changeText(s_pITMHideAsterisk_W128H016I);	// 累積カウンタ.	
		Txt_UtilityTotalComMessage.showEnable ( True );
	} else {

	// 原稿枚数カウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	uint32_t ui32_Manuscript = PnlComGetTotalCnt()->get( TYPE_TotalCntPage )
	                       + PnlComGetTotalCnt()->get( TYPE_TotalCntPage, TYPE_NetworkSpecific_NW2 );	// 原稿枚数
	char _sc_Manuscript[8+1];
	memset( _sc_Manuscript, '\0',sizeof( _sc_Manuscript ) );
	snprintf( _sc_Manuscript, sizeof( _sc_Manuscript ),"%8d", ui32_Manuscript );
	Tfd_UtilityTotalOtherManuscript.changeExternalText( ( uchar* )_sc_Manuscript, sizeof( _sc_Manuscript ) );
	// 用紙枚数カウンタ値
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	uint32_t ui32_Paper = PnlComGetTotalCnt()->get( TYPE_TotalCntPaper )
	                    + PnlComGetTotalCnt()->get( TYPE_TotalCntPaper, TYPE_NetworkSpecific_NW2 );		// 用紙枚数
	char _sc_Paper[8+1];
	memset( _sc_Paper, '\0',sizeof( _sc_Paper ) );
	snprintf( _sc_Paper, sizeof( _sc_Paper ),"%8d", ui32_Paper );
	Tfd_UtilityTotalOtherPaper.changeExternalText( ( uchar* )_sc_Paper, sizeof( _sc_Paper ) );
	// 両面トータルカウンタ値(コピー/プリント合計)
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	uint32_t ui32_2Side = PnlComGetTotalCnt()->get( TYPE_TotalCntCopyDuplex )	// コピー　・両面
	                  + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintDuplex )
                      + PnlComGetTotalCnt()->get( TYPE_TotalCntCopyDuplex, TYPE_NetworkSpecific_NW2 )
	                  + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintDuplex, TYPE_NetworkSpecific_NW2 ) ;	// プリント・両面
	char _sc_2Side[8+1];
	memset( _sc_2Side, '\0',sizeof( _sc_2Side ) );
	snprintf( _sc_2Side, sizeof( _sc_2Side ), "%8d", ui32_2Side );
	Tfd_UtilityTotalOther2SideTotal.changeExternalText( ( uchar* )_sc_2Side, sizeof( _sc_2Side ) );
// Comment 2017/10/11 [HCLT-Rajesh] [IT5_4.2] [RQ-686] [WP 403] Multiple Network Support.
	uint32_t ui32_PrintPaperNum = PnlComGetTotalCnt()->get( TYPE_TotalCntPrintPage )
	                            + PnlComGetTotalCnt()->get( TYPE_TotalCntPrintPage, TYPE_NetworkSpecific_NW2 );		// 印刷枚数カウンター .
	char _sc_PrintPaperNum[8+1];
	memset( _sc_PrintPaperNum, '\0',sizeof( _sc_PrintPaperNum ) );
	snprintf( _sc_PrintPaperNum, sizeof( _sc_PrintPaperNum ), "%8d", ui32_PrintPaperNum );
	Tfd_UtilityTotalOtherPrintPaperNum.changeExternalText( ( uchar* )_sc_PrintPaperNum, sizeof( _sc_PrintPaperNum ) );
// comment 2022/08/24 FPT.DungDV14 [IT6_2.3] [機器構成変更_No.21] Getting [累積カウンタ] value.
	uint32_t ui32_CumulativeCounter = PnlComGetTotalCnt()->get( TYPE_TotalCntSoftwareMachanical );		// 累積カウンタ.
	char _sc_CumulativeCounter[7+1];
	memset( _sc_CumulativeCounter, '\0',sizeof( _sc_CumulativeCounter ) );
	snprintf( _sc_CumulativeCounter, sizeof( _sc_CumulativeCounter ), "%7d", ui32_CumulativeCounter );
	Tfd_UtilityTotalOtherCumulativeCounter.changeExternalText( ( uchar* )_sc_CumulativeCounter, sizeof( _sc_CumulativeCounter ) );
// 2017/06/20 SCD2-Sky Shindo [SecV2-x] セールスカウンター画面のカウンター値非表示対応.
	Txt_UtilityTotalComMessage.showEnable ( False );
	
	}

	if( PnlComGetMachineSetting()->getCoverageRateKeyInfo() ) {				// トナーカバレージ率の表示・非表示情報の取得処理.
		Key_GotoUtilityTotalCoverageRate.showEnable( True );					// トナーカバレージボタンを表示にする．
	}
	else {
		Key_GotoUtilityTotalCoverageRate.showEnable( False );					// トナーカバレージボタンを非表示にする．
	}
// comment 2022/08/24 FPT.DungDV14 [IT6_2.3] [機器構成変更_No.21] Hide [累積カウンタ] parts for unsupport models.
	uchar uc_Destination = PnlComGetDestination();	// 仕向設定内容取得.
	TYPE_ModelType 	 e_ModelType   = PnlComGetSystemInfo()->getModelType();
	TYPE_MachineType e_MachineType = PnlComGetSystemInfo()->getMachineType();
// comment 2023/04/18 FPT.QuyenTT9 [IT6_2.3] [OP_BTS-41599] Hide [累積カウンタ] part when [DipSW164 bit7] is OFF.
	if( ( TYPE_SystemType_JP != uc_Destination )
		|| ( False == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW164, 7 ) )
		|| ( TYPE_ModelTypeIII != e_ModelType )
		|| ( TYPE_ModelTypeIII == e_ModelType &&  TYPE_MachineTypeEagle1 		!= e_MachineType	&&	TYPE_MachineTypeEagle2		!= e_MachineType	&&	TYPE_MachineTypeEagle2_5 != e_MachineType 	// EagleL III
			&& TYPE_MachineTypeEagleZ  		!= e_MachineType	&&	TYPE_MachineTypeEagleX		!= e_MachineType	&&	TYPE_MachineTypeEagle0	 != e_MachineType	// EagleH III
			&& TYPE_MachineTypeEagleBKZ 	!= e_MachineType	&&	TYPE_MachineTypeEagleBKX	!= e_MachineType	&&  TYPE_MachineTypeEagleBK0  != e_MachineType	// EagleHBK III
			&& TYPE_MachineTypeEagleBK1 	!= e_MachineType	&&	TYPE_MachineTypeEagleBK2	!= e_MachineType	&&	TYPE_MachineTypeEagleZplus != e_MachineType	&&	TYPE_MachineTypeEagleBKZplus != e_MachineType  ) ) {	// EagleLBK III/Z III/ZBK III
		Bak_UtilityTotalOtherCumulativeCounter.showEnable( False );
		Txt_TitleUtilityTotalOtherCumulativeCounter.showEnable( False );
		Tfd_UtilityTotalOtherCumulativeCounter.showEnable( False );
	}	
}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ（その他） : 画面開始処理
//================================================================================================
void PUtilityTotalOther::startGamen()
{
	// 親クラスの画面開始処理をコール.
	PUtilityTotalCom::startGamen();
// Comment 2019/01/24 【HCL_Krishna】【IT6_EagleH】【RQ-506】[WP-1710] Enter the highlight Display block mode.
	if( True == PJobListControl::isSelectedLeftArea( TYPD_LeftAreaSoft10Key ) ) {
		LIBG_EnterHighlightDisplayHideSection();
	}
	setLeftAreaEnableType( TYPD_LeftAreaSalesCounter | TYPD_LeftAreaSoft10Key );
	// テンキー表示キーを上部に配置変更する.
	Key_LeftIndexSoft10Key.changePosition( 8, 8 );
}

//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ（その他） : 画面終了処理.
//================================================================================================
void PUtilityTotalOther::endGamen()
{
// Comment 2019/01/24 【HCL_Krishna】【IT6_EagleH】【RQ-506】[WP-1710] Leave the highlight Display block mode.
// comment 2019//8/27 Sky雜賀 [EagleH][OP_TS-11728].
//	if( True == PJobListControl::isSelectedLeftArea( TYPD_LeftAreaSoft10Key ) ) {
//		LIBG_LeaveHighlightDisplayHideSection();
//	}
	LIBG_LeaveHighlightDisplayHideSection();
	Key_LeftIndexSoft10Key.changePosition( 8, 88 );
	PBasicMessageProc::endGamen();
}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : 画面移動
//================================================================================================
void PUtilityTotalCoverageRateColor::procGoToPage(
    uchar option )
{
	switch( option ) {
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items starts
		case 0:
		 	if ( ! IsSFP() ) {
				nextGamen = &gp_UtilityTotalCopy;					// ユーティリティ・セールスカウンタ（コピー）
		    }else{
				nextGamen = &gp_UtilityTotalPrinter;					// ユーティリティ・セールスカウンタ（コピー）
		    }
			break;
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items ends
		default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); break;
	}
}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート: 表示画面のセットアップ
//================================================================================================
void PUtilityTotalCoverageRateColor::setupGamen(
    short option )
{
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items starts
 	if ( IsSFP() ) {
    		Nfd_UtilityTotalCoverageCopyC.changeText(s_EXPNothingBar);
        	Nfd_UtilityTotalCoverageCopyM.changeText(s_EXPNothingBar);
        	Nfd_UtilityTotalCoverageCopyY.changeText(s_EXPNothingBar);
        	Nfd_UtilityTotalCoverageCopyBk.changeText(s_EXPNothingBar);
       	 	Nfd_UtilityTotalCoverageCopyBlackBk.changeText(s_EXPNothingBar);
  	}
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items ends
	PUtilitySalesCounterCom::setupGamen( option );
	if( onlyPanel.isFaxModekind() == True ) {
		// ファクスオプション装着時
		Txt_UtilityTotalCoverageScan.changeText( s_ITMScannerFaxPer );
	} else {
		// ファクスオプション非装着時
		Txt_UtilityTotalCoverageScan.changeText( s_ITMScannerPer );
	}
	// HDD装着時のみ表示するようにする.
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items starts
 	if ( ! IsSFP() ) {
		if( PnlComGetHddInfo()->getHddExist() == True ) {
			Bak_79X304Y296W472H028.showEnable( True );
			Bak_86X184Y296W112H028.showEnable( True );
			Nfd_UtilityTotalCoverageScanFaxC.showEnable( True );
			Nfd_UtilityTotalCoverageScanFaxM.showEnable( True );
			Nfd_UtilityTotalCoverageScanFaxY.showEnable( True );
			Nfd_UtilityTotalCoverageScanFaxBk.showEnable( True );
			Nfd_UtilityTotalCoverageScanFaxBlackBk.showEnable( True );
			Txt_UtilityTotalCoverageScan.showEnable( True );
		}
		else {
			Bak_79X304Y296W472H028.showEnable( False );
			Bak_86X184Y296W112H028.showEnable( False );
			Nfd_UtilityTotalCoverageScanFaxC.showEnable( False );
			Nfd_UtilityTotalCoverageScanFaxM.showEnable( False );
			Nfd_UtilityTotalCoverageScanFaxY.showEnable( False );
			Nfd_UtilityTotalCoverageScanFaxBk.showEnable( False );
			Nfd_UtilityTotalCoverageScanFaxBlackBk.showEnable( False );
			Txt_UtilityTotalCoverageScan.showEnable( False );
		}
	} else {
		Nfd_UtilityTotalCoverageScanFaxC.changeText(s_EXPNothingBar);
		Nfd_UtilityTotalCoverageScanFaxM.changeText(s_EXPNothingBar);
		Nfd_UtilityTotalCoverageScanFaxY.changeText(s_EXPNothingBar);
		Nfd_UtilityTotalCoverageScanFaxBk.changeText(s_EXPNothingBar);
		Nfd_UtilityTotalCoverageScanFaxBlackBk.changeText(s_EXPNothingBar);
	}
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items ends
	Nfd_UtilityTotalCoverageCopyC		= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_CopyC );	// カラーカバレッジレート・シアン・コピーカバレッジレート値
	Nfd_UtilityTotalCoverageCopyM		= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_CopyM );	// カラーカバレッジレート・マゼンタ・コピーカバレッジレート値
	Nfd_UtilityTotalCoverageCopyY		= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_CopyY );	// カラーカバレッジレート・イエロー・コピーカバレッジレート値
	Nfd_UtilityTotalCoverageCopyBk		= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_CopyK );	// カラーカバレッジレート・ブラック・コピーカバレッジレート値
	Nfd_UtilityTotalCoveragePrinterC	= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_PrintC );	// カラーカバレッジレート・シアン・プリンタカバレッジレート値
	Nfd_UtilityTotalCoveragePrinterM	= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_PrintM );	// カラーカバレッジレート・マゼンタ・プリンタカバレッジレート値
	Nfd_UtilityTotalCoveragePrinterY	= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_PrintY );	// カラーカバレッジレート・イエロー・プリンタカバレッジレート値
	Nfd_UtilityTotalCoveragePrinterBk	= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_PrintK );	// カラーカバレッジレート・ブラック・プリンタカバレッジレート値
	Nfd_UtilityTotalCoverageScanFaxC	= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_ScanFaxC );	// カラーカバレッジレート・シアン・スキャンカバレッジレート値
	Nfd_UtilityTotalCoverageScanFaxM	= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_ScanFaxM );	// カラーカバレッジレート・マゼンタ・スキャンカバレッジレート値
	Nfd_UtilityTotalCoverageScanFaxY	= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_ScanFaxY );	// カラーカバレッジレート・イエロー・スキャンカバレッジレート値
	Nfd_UtilityTotalCoverageScanFaxBk	= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_ScanFaxK );	// カラーカバレッジレート・ブラック・スキャンカバレッジレート値
	Nfd_UtilityTotalCoverageTotalC		= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_TotalC );	// カラーカバレッジレート・シアン・トータルカバレッジレート値
	Nfd_UtilityTotalCoverageTotalM		= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_TotalM );	// カラーカバレッジレート・マゼンタ・トータルカバレッジレート値
	Nfd_UtilityTotalCoverageTotalY		= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_TotalY );	// カラーカバレッジレート・イエロー・トータルカバレッジレート値
	Nfd_UtilityTotalCoverageTotalBk		= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_TotalK );	// カラーカバレッジレート・ブラック・トータルカバレッジレート値
	Nfd_UtilityTotalCoverageCopyBlackBk		= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_CopyBk );		// ブラックカバレッジレート・コピーカバレッジレート値
	Nfd_UtilityTotalCoveragePrinterBlackBk	= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_PrintBk );		// ブラックカバレッジレート・プリンタカバレッジレート値
	Nfd_UtilityTotalCoverageScanFaxBlackBk	= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_ScanFaxBk );	// ブラックカバレッジレート・スキャン／ファクスカバレッジレート値
	Nfd_UtilityTotalCoverageTotalBlackBk	= PnlComGetCoverageCnt()->getCoverageRate( TYPE_CoverageCnt_TotalBk );		// ブラックカバレッジレート・トータルカバレッジレート値
}

//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : 画面開始処理
//================================================================================================
void PUtilityTotalCoverageRateColor::startGamen()
{
	// 親クラスの画面開始処理をコール.
	//comment 2019/09/04 K.Sato [EagleH][OP_BTS-12802] カウンタ-印刷後パネルダウンの対応.
	//PUtilityToMenuCom::startGamen();
	PUtilitySalesCounterCom::startGamen();
	setLeftAreaEnableType( TYPD_LeftAreaSalesCounter | TYPD_LeftAreaSoft10Key );
	// テンキー表示キーを上部に配置変更する.
	Key_LeftIndexSoft10Key.changePosition( 8, 8 );

}

//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : 画面終了処理.
//================================================================================================
void PUtilityTotalCoverageRateColor::endGamen()
{
	Key_LeftIndexSoft10Key.changePosition( 8, 88 );
	PBasicMessageProc::endGamen();
}

//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : サービス認証のロック解除タイマー処理.
//================================================================================================
Bool PUtilityTotalCoverageRateColor::checkServiceModeLockCancel( EP_Service_KeyCode _ep_KeyCode, short _ss_option )
{
	// 仕様変更により本画面ではマジックシーケンスは受け付けない.
	return ( False );
}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : スタート.
//================================================================================================
void PUtilityTotalCoverageRateColor::procStart(
    uchar option )
{
	if( checkServiceModeLockCancel( EP_Service_KeyCode_Start, option ) == False ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : ストップ.
//================================================================================================
void PUtilityTotalCoverageRateColor::procStop(
    uchar option )
{
	checkServiceModeLockCancel( EP_Service_KeyCode_Stop, option );
	setBuzzerType( PCGE_BuzzerType_InputNg );
}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : テンキー.
//================================================================================================
void PUtilityTotalCoverageRateColor::proc10Key(
    uchar option )
{
	checkServiceModeLockCancel( EP_Service_KeyCode_10Key, option );
	setBuzzerType( PCGE_BuzzerType_InputNg );
}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : クリア.
//================================================================================================
void PUtilityTotalCoverageRateColor::procClear(
    uchar option )
{
	setBuzzerType( PCGE_BuzzerType_InputNg );
	_ss_CommandOkCount = 0;		// コマンドをクリアする.
}
/*================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : カラーモードキー.
================================================================================================*/
void PUtilityTotalCoverageRateColor::procColorMode( uchar option )
{
	checkServiceModeLockCancel( EP_Service_KeyCode_ColorMode, option );
}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : リセットキー.
//================================================================================================
void PUtilityTotalCoverageRateColor::procReset(
    uchar option
)
{
	checkServiceModeLockCancel( EP_Service_KeyCode_Reset, option );
	PUtilitySalesCounterCom::procReset( option );
}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : 割込みキー.
//================================================================================================
void PUtilityTotalCoverageRateColor::procIntMode(
    uchar option )
{
	if( checkServiceModeLockCancel( EP_Service_KeyCode_Int, option ) == False ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : 確認コピー.
//================================================================================================
void PUtilityTotalCoverageRateColor::procSampleCopy(
    uchar option )
{
	if( checkServiceModeLockCancel( EP_Service_KeyCode_SampleCopy, option ) == False ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : 暗証番号入力.
//================================================================================================
void PUtilityTotalCoverageRateColor::procIDInput(
    uchar option )
{
	if( checkServiceModeLockCancel( EP_Service_KeyCode_Id, option ) == False ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : ジョブメモリ.
//================================================================================================
void PUtilityTotalCoverageRateColor::procJobMemory(
    uchar option )
{
	if( checkServiceModeLockCancel( EP_Service_KeyCode_JobMemory, option ) == False ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : ユーティリティ.
//================================================================================================
void PUtilityTotalCoverageRateColor::procUtility(
    uchar option )
{
	checkServiceModeLockCancel( EP_Service_KeyCode_Util, option );
	PUtilitySalesCounterCom::procUtility( option );
}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : コピーキー.
//================================================================================================
void PUtilityTotalCoverageRateColor::procCopy(
    uchar option )
{
	checkServiceModeLockCancel( EP_Service_KeyCode_Copy, option );
	PUtilitySalesCounterCom::procCopy( option );
}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : FAX.
//================================================================================================
void PUtilityTotalCoverageRateColor::procFax(
    uchar option )
{
	checkServiceModeLockCancel( EP_Service_KeyCode_Fax, option );
	PUtilitySalesCounterCom::procFax( option );
}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : FAX.
//================================================================================================
void PUtilityTotalCoverageRateColor::procFaxOnly(
    uchar option )
{
	checkServiceModeLockCancel( EP_Service_KeyCode_Fax, option );
	PUtilitySalesCounterCom::procFaxOnly( option );
}

//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : ボックス.
//================================================================================================
void PUtilityTotalCoverageRateColor::procBox(
    uchar option )
{
	checkServiceModeLockCancel( EP_Service_KeyCode_Box, option );
	PUtilitySalesCounterCom::procBox( option );
}
//================================================================================================.
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : ユビキタスプリント.
//================================================================================================.
void PUtilityTotalCoverageRateColor::procUbqPrint(
    uchar option )
{
	checkServiceModeLockCancel( EP_Service_KeyCode_UbqPrint, option );
	PUtilitySalesCounterCom::procUbqPrint( option );
}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : ヘルプ.
//================================================================================================
void PUtilityTotalCoverageRateColor::procHelp(
    uchar option )
{
	if( checkServiceModeLockCancel( EP_Service_KeyCode_Help, option ) == False ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : 拡大表示.
//================================================================================================
void PUtilityTotalCoverageRateColor::procEnlarge(
    uchar option )
{
	if( checkServiceModeLockCancel( EP_Service_KeyCode_Enlarge, option ) == False ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : ユーザー補助.
//================================================================================================
void PUtilityTotalCoverageRateColor::procAccessibility(
    uchar option
)
{
	PUtilitySalesCounterCom::procAccessibility( option );
}
#ifdef	  DEF_THAMES_ICUSTOM	// アプリ割付.
//================================================================================================
// 概要		:ユーティリティ・セールスカウンタ・カバレッジレート : ブラウザキー.
// 戻り値	:なし.
// 備考		:.
//================================================================================================
void PUtilityTotalCoverageRateColor::procBrowser( uchar option )
{
	// マジックシーケンスにイメージディーラ、マイパネル、ブラウザ、アプリメニュー、.
	// コピー、スキャンファクスなどは使用されていないのでキーコード無しとしておく.
	checkServiceModeLockCancel( EP_Service_KeyCode_None, option );
	PUtilitySalesCounterCom::procBrowser( option );
}
#endif // DEF_THAMES_ICUSTOM	// アプリ割付.
//================================================================================================
// 概要		:ユーティリティ・ユニットチェック : 画面移動
//================================================================================================
void PUtilityUnitCheck::procGoToPage(
    uchar option )
{
	if( option == 0 ) {
		// ユーティリティメニューキー受け付け可状態でかつ、ベンダーのみによる管理状態でないとき
		if( isWarningFatal() == False && PnlComGetSystemSetting()->getLongPicturePrint() == False ) {
			// FATALが発生していない ＋ 長尺紙ﾓｰﾄﾞではない.

			APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();	// DipSw（API）・インスタンス取得.
			if( NULL == pc_SoftDipSw ) {
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
				return;
			}

			if( True == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( 71, 4 ) ) {
				chkNeedAdminPasswordInput( &gp_UtilityUnitCheckPrint, TYPD_LeftAreaBookmark );
			} else {
// comment 2016/01/29 OPS1延原 [ZeusS_ZX0][CQ524561]ファクス設定＞設定値リスト画面表示後に、他のリスト出力画面の片面/両面キーが表示されない.
				// PAdminSystemDepartListPrintListPrint::e_ListPrintを設定して、PSPC_FeedTraySelectListPrint::SPCsetupGamen()内の表示条件を満たす
				PAdminSystemDepartListPrintListPrint::e_ListPrint = TYPE_ListPrintUnitCheck;
				nextGamen = &gp_UtilityUnitCheckPrint;		// ユーティリティ・ユニットチェック・プリント
			}

		}
	}
}
//================================================================================================
// 概要		:ユーティリティ・ユニットチェック : 表示画面のセットアップ
//================================================================================================
void PUtilityUnitCheck::setupGamen(
    short option )
{
// ZeusX/0非表示対応.
	TYPE_MachineType e_MachineType = PnlComGetSystemInfo()->getMachineType();	// 機種情報取得.

	PUtilityToMenuCom::setupGamen( option );
#ifdef DEF_Zeus5BkCus
//	if(True == PLBC_FuncVersionCheck::isOK_IT5_Type1()){
//		unitCheckTbl[ 6 ].uc_PageNum = 1;
//		Bak_UtilityUnitCheckP2OzonFilter.showEnable(False);
//		Bak_UtilityUnitCheckP2OzonFilterGraphic.showEnable(False);
//		Rot_UtilityUnitCheckP2OzonFilter.showEnable(False);
//		Bak_UtilityUnitCheckP2PaperDustCleaner.changePosition(488,308);
//		Txt_UtilityUnitCheckP2PaperDustCleaner.changePosition(503,316);
//		Bak_UtilityUnitCheckP2PaperDustCleanerGraphic.changePosition(518,340);
//		Rot_UtilityUnitCheckP2PaperDustCleaner.changePosition(574,348);
//		Txt_UtilityUnitCheckP2PercentPaperDustCleaner.changePosition(710+24,348);
//		Num_PercentPaperDustCleaner.changePosition(710, 348+4);
//		Txt_UtilityUnitCheckP2PaperDustCleanerFull.changePosition(558, 348);
//		Txt_UtilityUnitCheckP2PaperDustCleanerEmpty.changePosition(710, 348);
//// comment 2014/11/05 【HCL-Hema】 【CQ-506393】 Modifying the Coordinates of Paper Dust Remover Background levelguage for start and end date.
//		//Num_DateTimePaperDustCleanerStartDate.changePosition(534, 364);
//		//Num_DateTimePaperDustCleanerEndDate.changePosition(694, 364);
//		Num_DateTimePaperDustCleanerStartDate.changePosition(534-8, 364+4);
//		Num_DateTimePaperDustCleanerEndDate.changePosition(694-24, 364+4);
//	} else {
// comment 2015/06/15 OPS1酒井純 [Helios][CQ514489]Poseidonでのオゾンフィルター表示対応.
		// 紙粉除去クリーナーを2ページに表示する.
		unitCheckTbl[ 6 ].uc_PageNum = 2;
		Bak_UtilityUnitCheckP2OzonFilter.showEnable( True );
		Bak_UtilityUnitCheckP2OzonFilterGraphic.showEnable( True );
		Rot_UtilityUnitCheckP2OzonFilter.showEnable( True );
// comment 2014/11/05 【HCL-Hema】 【CQ-506393】 Hidding the remodified parts of Paper Dust Remover Background levelguage to use default values.
	//Bak_UtilityUnitCheckP2PaperDustCleaner.changePosition(176,136);
	//Txt_UtilityUnitCheckP2PaperDustCleaner.changePosition(190,144);
	//Bak_UtilityUnitCheckP2PaperDustCleanerGraphic.changePosition(206,168);
	//Rot_UtilityUnitCheckP2PaperDustCleaner.changePosition(262,176);
	//Txt_UtilityUnitCheckP2PercentPaperDustCleaner.changePosition(398+24,176);
	//Num_PercentPaperDustCleaner.changePosition(398, 176+4);
// comment 2014/10/27 OPS1-Sky浜野 [Poseidon][CQ504602] DipSWによる表示切替削除対応.
// comment 2014/11/05 【HCL-Hema】 【CQ-506393】 Hidding the remodified parts of Paper Dust Remover Background levelguage to use default values.
	//Txt_UtilityUnitCheckP2PaperDustCleanerFull.changePosition(246, 176);
	//Txt_UtilityUnitCheckP2PaperDustCleanerEmpty.changePosition(398, 176);
	//Num_DateTimePaperDustCleanerStartDate.changePosition(222, 192);
	//Num_DateTimePaperDustCleanerEndDate.changePosition(382, 192);
//	}
#endif
//	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();	// DipSw（API）・インスタンス取得.
	// IT5以降でDipSW140 bit0の使用は削除されるため、デフォルトFalse（カウントダウン）とする.
	Bool bl_isCountUpMode = False;
	register uint32_t calcParcent = 0;

	// 配列要素数可変対応.
// CQ511218 不具合改修 Sky hattori start.
//	// AS-PVD-39 ユーティリティ画面-消耗品確認 Sky hattori start.
//	#ifdef DEF_A4_Cus
//		// トナーカートリッジ消耗率取得.
//		TYPS_TonerQuantity	st_TonerValue;
//		PnlComGetTonerQuantity( &st_TonerValue );
//	#endif // DEF_A4_Cus.
//	// AS-PVD-39 ユーティリティ画面-消耗品確認 Sky hattori end.
#ifdef DEF_A4_Cus
	//プリンタデバイスクラスのインスタンスを取得する.
	APIC_PrinterWarning* pc_PrinterImpColor = APIC_PrinterWarning::newInstance();
	if ( NULL == pc_PrinterImpColor ) {
		//デバッグログを作成する.
		PnlComPutDebugLog( True, UC_PNL_LOGPATTERN_1, "PUtilityUnitCheck::setupGamen", UC_PNL_ERROR_USE );
		return;
	}
#endif // #ifdef DEF_A4_Cus.
// CQ511218 不具合改修 Sky hattori end.
	for( register ushort i = 0; NULL != unitCheckTbl[ i ].parts; i++ ) {
		switch( unitCheckTbl[ i ].lifeUser ) {
			case PERROR_IULifeC:					calcParcent = comENG.getCyanIULife_ConsumptionRate();				break;
			case PERROR_IULifeM:					calcParcent = comENG.getMagentaIULife_ConsumptionRate();			break;
			case PERROR_IULifeY:					calcParcent = comENG.getYellowIULife_ConsumptionRate();				break;
			case PERROR_IULifeK:					calcParcent = comENG.getBlackIULife_ConsumptionRate();				break;
			case PERROR_FuserUnitLife:				calcParcent = comENG.getFuserUnitLife_ConsumptionRate();			break;
			case PERROR_TransRollerUnitLifeUser:	calcParcent = comENG.get2ndTransRollerUnitLife_ConsumptionRate();	break;
// AS-PVD-39 ユーティリティ画面-消耗品確認 Sky hattori start.
#ifdef DEF_A4_Cus
			// トナーカートリッジ消耗率取得.
// CQ511218 不具合改修 Sky hattori start.
//			case PERROR_TCLifeC:					calcParcent = st_TonerValue.sc_CTonerQuantity;							break;
//			case PERROR_TCLifeM:					calcParcent = st_TonerValue.sc_MTonerQuantity;							break;
//			case PERROR_TCLifeY:					calcParcent = st_TonerValue.sc_YTonerQuantity;							break;
//			case PERROR_TCLifeK:					calcParcent = st_TonerValue.sc_KTonerQuantity;							break;
			// get*TonerQuantity()は残量を返す。他項目とあわせるため、消耗率を算出する.
			case PERROR_TCLifeC:					calcParcent = 100 - pc_PrinterImpColor->getCTonerQuantity();			break;
			case PERROR_TCLifeM:					calcParcent = 100 - pc_PrinterImpColor->getMTonerQuantity();			break;
			case PERROR_TCLifeY:					calcParcent = 100 - pc_PrinterImpColor->getYTonerQuantity();			break;
			case PERROR_TCLifeK:					calcParcent = 100 - pc_PrinterImpColor->getKTonerQuantity();			break;
// CQ511218 不具合改修 Sky hattori end.
#endif // DEF_A4_Cus.
// AS-PVD-39 ユーティリティ画面-消耗品確認 Sky hattori end.
#ifndef DEF_Zeus5BkCus
			case PERROR_TransBeltUnitLife:
				calcParcent = comENG.getTransferBeltUnitLife_ConsumptionRate();	// Transfer belt unit life.
#else
			case PERROR_PaperDustRemovalCleanerLife :
				calcParcent = comENG.getPaperDustRemovalCleanerLife_ConsumptionRate(); // Paper dust remover cleaner.
#endif // DEF_Zeus5BkCus
				break;
			case PERROR_OzonFilterLifeUser:
// comment 2015/06/15 OPS1酒井純 [Helios][CQ514489]Poseidonでのオゾンフィルター表示対応.
//				if( !( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) ) {
					calcParcent = comENG.getOzonFilterUnitLife_ConsumptionRate();
// comment 2015/06/15 OPS1酒井純 [Helios][CQ514489]Poseidonでのオゾンフィルター表示対応.
//				}
				break;
			// 消耗品確認：2P構成、IUブラックをDevC/DCに分割、トナーフィルター追加対応.
			case PERROR_DevCLifeK:					calcParcent = comENG.getBlackDevCLife_ConsumptionRate();			break;	// DevC:K.
			case PERROR_DCLifeK:					calcParcent = comENG.getBlackDCLife_ConsumptionRate();				break;	// DC:K.
			case PERROR_SmokeFilterUnitLife:		calcParcent = comENG.getSmokeFilterUnitLife_ConsumptionRate();		break;	// トナーフィルター.
			case PERROR_DevCLifeC:					calcParcent = comENG.getCyanDevCLife_ConsumptionRate();				break;	// 現像ユニット(C).
			case PERROR_DevCLifeM:					calcParcent = comENG.getMagentaDevCLife_ConsumptionRate();			break;	// 現像ユニット(M).
			case PERROR_DevCLifeY:					calcParcent = comENG.getYellowDevCLife_ConsumptionRate();			break;	// 現像ユニット(Y).
			case PERROR_DCLifeC:					calcParcent = comENG.getCyanDCLife_ConsumptionRate();				break;	// ドラムユニット(C).
			case PERROR_DCLifeM:					calcParcent = comENG.getMagentaDCLife_ConsumptionRate();			break;	// ドラムユニット(M).
			case PERROR_DCLifeY:					calcParcent = comENG.getYellowDCLife_ConsumptionRate();				break;	// ドラムユニット(Y).
			case PERROR_TransCleanerUnitLife:
			default: calcParcent = 0; break;
		}
//		if( 1 == uc_NowPageNum ){
//			if( 1 == unitCheckTbl[ i ].uc_PageNum ){
//				if ( onlyPanel.isWarningOn( comPanelJob, unitCheckTbl[i].lifeUser ) == True
//				 || onlyPanel.isWarningOn( comPanelJob, unitCheckTbl[i].lifeService ) == True
//				 || onlyPanel.isWarningOn( comPanelJob, unitCheckTbl[i].nearLife ) == True ) {
//					// ライフ状態、ニアライフ状態（ＡＴＴＥＮＴＩＯＮ状態）の時は、１００％表示にしておく
//				} else {
//					if ( calcParcent >= (uint32_t)90 ) {
//						// 算出結果が９０％以上の時は、９０％表示にしておく
//					} else {
//					}
//				}
//			}
//		}else{
//			if( 2 == unitCheckTbl[ i ].uc_PageNum ){
//				if ( onlyPanel.isWarningOn( comPanelJob, unitCheckTbl[i].lifeUser ) == True
//				 || onlyPanel.isWarningOn( comPanelJob, unitCheckTbl[i].lifeService ) == True
//				 || onlyPanel.isWarningOn( comPanelJob, unitCheckTbl[i].nearLife ) == True ) {
//					// ライフ状態、ニアライフ状態（ＡＴＴＥＮＴＩＯＮ状態）の時は、１００％表示にしておく
//				} else {
//					if ( calcParcent >= (uint32_t)90 ) {
//						// 算出結果が９０％以上の時は、９０％表示にしておく
//					} else {
//					}
//				}
//			}
//		}
// comment 2015/06/15 OPS1酒井純 [Helios][CQ514489]Poseidonでのオゾンフィルター表示対応.
//		if( !( ( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) && ( PERROR_OzonFilterLifeUser == unitCheckTbl[ i ].lifeUser ) ) ) {
			if( uc_NowPageNum == unitCheckTbl[ i ].uc_PageNum ) {
				if( !bl_isCountUpMode ) {		//カウントダウン形式.
					unitCheckTbl[ i ].baseParts->set( unitCheckTbl[ i ].UnitPercentParts, static_cast<float>( 100 - calcParcent ), True );	// 計算値表示.
					( ( PRotationGraphic* )( unitCheckTbl[ i ].parts ) )->changeGraphic( uchar( ( ( 100 - calcParcent ) + 9 ) /10 ) );		// 計算値表示(切り上げ).
				}
				else {		//カウントアップ形式.
					if( onlyPanel.isWarningOn( comPanelJob, unitCheckTbl[i].lifeUser ) == True
					    || onlyPanel.isWarningOn( comPanelJob, unitCheckTbl[i].lifeService ) == True
					    || onlyPanel.isWarningOn( comPanelJob, unitCheckTbl[i].nearLife ) == True ) {
						// ライフ状態、ニアライフ状態（ＡＴＴＥＮＴＩＯＮ状態）の時は、１００％表示にしておく
						( ( PRotationGraphic* )( unitCheckTbl[ i ].parts ) )->changeGraphic( 10 );		// １００％表示
					} else {
						if( calcParcent >= ( uint32_t )90 ) {
							// 算出結果が９０％以上の時は、９０％表示にしておく
							( ( PRotationGraphic* )( unitCheckTbl[ i ].parts ) )->changeGraphic( 9 );		// ９０％表示
						} else {
							( ( PRotationGraphic* )( unitCheckTbl[ i ].parts ) )->changeGraphic( uchar( calcParcent/10 ) );	// 計算値表示
						}
					}
				}
			}
// comment 2015/06/15 OPS1酒井純 [Helios][CQ514489]Poseidonでのオゾンフィルター表示対応.
//		}

		if( PERROR_SmokeFilterUnitLife == unitCheckTbl[ i ].lifeUser ) {		// ﾄﾅｰﾌｨﾙﾀｰの場合.
			if( ( TYPE_MachineTypeZeus1   == e_MachineType ) || ( TYPE_MachineTypeZeus2   == e_MachineType ) || ( TYPE_MachineTypeZeus3   == e_MachineType )
			    ||( TYPE_MachineTypeZeus1BK == e_MachineType ) || ( TYPE_MachineTypeZeus2BK == e_MachineType ) || ( TYPE_MachineTypeZeus3BK == e_MachineType )
			   ||( TYPE_MachineTypeZeusS1  == e_MachineType ) || ( TYPE_MachineTypeZeusS2  == e_MachineType ) || ( TYPE_MachineTypeZeusS2_5 == e_MachineType ) 	// ZeusS/1/2/2_5.
// 2014/11/18【HeliosL 】 【HCLT-Chandrasekar】 【制御構成変更_No.1】 Adding Machine support for HeliosL Start.
			   ||( TYPE_MachineTypeHeliosL2 == e_MachineType ) || ( TYPE_MachineTypeHeliosL3  == e_MachineType ) // 製品タイプ：HeliosL2 / 製品タイプ：HeliosL3.
// comment 2015/07/22【HCLT-Shalini】【ZeusS_ZX0】【制御構成変更_No.7】 Addition of ZeusS_Z_EU,ZeusS_X_EU,ZeusS_Z_US_JP,ZeusS_X_US_JP Machine type.
//// comment 2015/06/29【HCLT-Senthil】【ZeusS_ZX0】【制御構成変更_No.5】 Addition of ZeusZX0 Machine type.
//		 	   || ( TYPE_MachineTypeZeusSZ == e_MachineType ) || ( TYPE_MachineTypeZeusSX == e_MachineType ) || ( TYPE_MachineTypeZeusS0 == e_MachineType ) ) { // 製品タイプ：ZeusSZ/X/0. 
// comment 2015/07/22【HCLT-Shalini】【ZeusS_ZX0】【制御構成変更_No.7】 Addition of ZeusS_Z_EU,ZeusS_X_EU,ZeusS_Z_US_JP,ZeusS_X_US_JP,ZeusS0 Machine type.
// comment 2015/12/15 OPS1-MSE井上 [ZeusS_ZX0][CQ518770] ZeusS ZX0ではトナーフィルターを表示するように対応.
//			  || (TYPE_MachineTypeZeusSZ_US_JP == e_MachineType ) || ( TYPE_MachineTypeZeusSZ_EU == e_MachineType ) || ( TYPE_MachineTypeZeusSX_US_JP == e_MachineType )||( TYPE_MachineTypeZeusSX_EU == e_MachineType )||( TYPE_MachineTypeZeusS0 == e_MachineType ) ) {
// Comment 2017/01/18 HCLT_Anand [IT5_4.1] [制御構成変更_No.6] Added Machine condition for ZeusSBK2nd model.
			  || ( TYPE_MachineTypeZeusSBK2nd1 == e_MachineType )
			  || ( TYPE_MachineTypeZeusSBK2nd2 == e_MachineType ) 
			  ) {
				( ( PRotationGraphic* )( unitCheckTbl[ i ].parts ) )->showEnable( False );						// 残量表示■Graphic非表示処理.
				unitCheckTbl[ i ].baseParts->showEnableAdd( unitCheckTbl[ i ].FullParts,        False );	//「Ｆ」の表示非表示処理.
				unitCheckTbl[ i ].baseParts->showEnableAdd( unitCheckTbl[ i ].EmptyParts,       False );	//「Ｅ」の表示非表示処理.
				unitCheckTbl[ i ].baseParts->showEnableAdd( unitCheckTbl[ i ].PercentParts,     False );	//「%」の表示非表示処理.
				unitCheckTbl[ i ].baseParts->showEnableAdd( unitCheckTbl[ i ].UnitPercentParts, False );	//残量(%)の表示非表示処理.
			}
		}
	}
	TYPE_KeyMode e_KeyMode = APIC_KeyDevice::getKeyMode();				//	キーモード.
// comment 2017/05/19 Sky東島 [IT5_4.0][1021890] .
//	if( isWarningFatal() == False && PnlComGetSystemSetting()->getLongPicturePrint() == False 		// FATALが発生していない ＋ 長尺紙ﾓｰﾄﾞではない.
//	    && ( e_KeyMode != TYPE_KeyModeVendor && e_KeyMode != TYPE_KeyModeVendorKM && e_KeyMode != TYPE_KeyModePageAcses ) 	// ベンダー（従来・統合タイプ）/PageACSES装着時でない.
//	    && ( onlyPanel.isWarningOn( comPanelJob, PNL_OTHER_VendorAdminInput ) == False ) )	{	// ベンダー装着時管理者番号入力が発生していない.
//		// FATALが発生していない ＋ 長尺紙ﾓｰﾄﾞではない.
	Bool bl_CheckPrintShow = True;
	if( isWarningFatal() == False && PnlComGetSystemSetting()->getLongPicturePrint() == False 		// FATALが発生していない ＋ 長尺紙ﾓｰﾄﾞではない.
	    && ( e_KeyMode != TYPE_KeyModeVendor && e_KeyMode != TYPE_KeyModeVendorKM && e_KeyMode != TYPE_KeyModePageAcses ) 	// ベンダー（従来・統合タイプ）/PageACSES装着時でない.
	    && ( onlyPanel.isWarningOn( comPanelJob, PNL_OTHER_VendorAdminInput ) == False ) )	{	// ベンダー装着時管理者番号入力が発生していない.
		bl_CheckPrintShow = True;
	} else {
		bl_CheckPrintShow = False;
	}
	if( True == PnlComGetSystemSetting()->getSecurityLevelUpOn() ){		// セキュリティー強化モードの場合.
		APIC_User* pc_user = PnlComGetUser();
		if( NULL != pc_user ){
			if( TYPE_UserTypeAdmin != pc_user->getUserType() ){		// 管理者ユーザーではない.
				bl_CheckPrintShow = False;
			}
		} else {
			bl_CheckPrintShow = False;
		}
	}
	if( True == bl_CheckPrintShow ){
		Key_GotoUtilityUnitCheckPrint.showEnable( True );
	} else {
		// FATALが発生時、
		Key_GotoUtilityUnitCheckPrint.showEnable( False );
	}
	// リストプリントできない警告が発生していた場合、または、パネルステータスが設定状態以外のときは、印刷キーはグレーアウト.
	if( onlyPanel.isWarningatListPrintDisable( onlyPanel.cc_DisplayJob ) == True	// リストプリントできない警告が発生していた場合.
	    || onlyPanel.ep_PanelStatus != EP_PNS_Setting )	{							// パネルステータスが設定状態以外.
		Key_GotoUtilityUnitCheckPrint.activate( False );
	} else {
		Key_GotoUtilityUnitCheckPrint.activate( True );
	}

	// 消耗品確認：2P構成、IUブラックをDevC/DCに分割、トナーフィルター追加対応.
	// ライフ使用開始日/到達予定日取得.
	TYPC_Time c_time( 0,0,0,0,0,0 );
	APIC_PrinterSystem* pc_PrinterSystem = PnlComGetPrinterSystem();
	uchar uc_UTCnt = 0;
	// 配列要素数可変対応.
	for( uc_UTCnt = 0; NULL != stUnitCheckDateTbl[uc_UTCnt].baseParts; uc_UTCnt++ ) { // ユニット数分行う.
		pc_PrinterSystem->getConsumptionRateStartTime( stUnitCheckDateTbl[uc_UTCnt].e_ConsumableKind, &c_time );	// ライフ使用開始日取得.
		stUnitCheckDateTbl[uc_UTCnt].ss_StartYear = c_time.ss_Year;	// ライフ使用開始年取得.
		if( bl_isCountUpMode ) {		//カウントアップ形式.
			SetDayTimeDisp( ( PNumberField* )( stUnitCheckDateTbl[uc_UTCnt].startParts ), &Num_DateTimeUnDispTime, &c_time, JBLE_TIME_DISP_TYPE_QUARTER, JBLE_YEAR_DISP_TYPE_NORMAL );	// 開始日(左側)パーツへライフ使用開始日を設定.
		} else {		//カウントダウン形式.
			SetDayTimeDisp( ( PNumberField* )( stUnitCheckDateTbl[uc_UTCnt].endParts ), &Num_DateTimeUnDispTime, &c_time, JBLE_TIME_DISP_TYPE_QUARTER, JBLE_YEAR_DISP_TYPE_NORMAL );	// 終了日(右側)パーツへライフ使用開始日を設定.
		}
		pc_PrinterSystem->getConsumptionRateEndTime( stUnitCheckDateTbl[uc_UTCnt].e_ConsumableKind, &c_time );		// ライフ到達予定日取得.
		stUnitCheckDateTbl[uc_UTCnt].ss_EndYear = c_time.ss_Year;		// ライフ使用開始年取得.
		if( bl_isCountUpMode ) {		//カウントアップ形式.
			SetDayTimeDisp( ( PNumberField* )( stUnitCheckDateTbl[uc_UTCnt].endParts ), &Num_DateTimeUnDispTime, &c_time, JBLE_TIME_DISP_TYPE_QUARTER, JBLE_YEAR_DISP_TYPE_NORMAL );	// 終了日(右側)パーツへライフ到達予定日を設定.
		} else {		//カウントダウン形式.
			SetDayTimeDisp( ( PNumberField* )( stUnitCheckDateTbl[uc_UTCnt].startParts ), &Num_DateTimeUnDispTime, &c_time, JBLE_TIME_DISP_TYPE_QUARTER, JBLE_YEAR_DISP_TYPE_NORMAL );	// 開始日(左側)パーツへライフ到達予定日を設定.
		}
	}


	for( uc_UTCnt = 0; NULL != unitCheckTbl[uc_UTCnt].baseParts; uc_UTCnt++ ) {
		if( uc_NowPageNum == unitCheckTbl[ uc_UTCnt ].uc_PageNum ) {
			if( bl_isCountUpMode ) {	//カウントアップ形式.
				unitCheckTbl[uc_UTCnt].baseParts->showEnableAdd( unitCheckTbl[uc_UTCnt].PercentParts, False );	//「%」の表示非表示処理.
				unitCheckTbl[uc_UTCnt].baseParts->showEnableAdd( unitCheckTbl[uc_UTCnt].UnitPercentParts, False );	//残量(%)の表示非表示処理.
				unitCheckTbl[uc_UTCnt].baseParts->showEnableAdd( unitCheckTbl[uc_UTCnt].FullParts, True );	//「Ｆ」の表示非表示処理.
				unitCheckTbl[uc_UTCnt].baseParts->showEnableAdd( unitCheckTbl[uc_UTCnt].EmptyParts, True );	//「Ｅ」の表示非表示処理.
			} else {		//カウントダウン形式.
				unitCheckTbl[uc_UTCnt].baseParts->showEnableAdd( unitCheckTbl[uc_UTCnt].FullParts, False );	//「Ｆ」の表示非表示処理.
				unitCheckTbl[uc_UTCnt].baseParts->showEnableAdd( unitCheckTbl[uc_UTCnt].EmptyParts, False );	//「Ｅ」の表示非表示処理.
				unitCheckTbl[uc_UTCnt].baseParts->showEnableAdd( unitCheckTbl[uc_UTCnt].PercentParts, True );	//「%」の表示非表示処理.
				unitCheckTbl[uc_UTCnt].baseParts->showEnableAdd( unitCheckTbl[uc_UTCnt].UnitPercentParts, True );	//残量(%)の表示非表示処理.
			}
		}
	}
	// ライフ使用開始日/到達予定日が決定されていない場合非表示.
	// 配列要素数可変対応.
	for( uc_UTCnt = 0; NULL != stUnitCheckDateTbl[uc_UTCnt].baseParts; uc_UTCnt++ ) { // ユニット数分行う.
		Bool bl_ShowStartParts = True;
		Bool bl_ShowEndParts = True;
		if( 0 == stUnitCheckDateTbl[uc_UTCnt].ss_StartYear ) {
			if( bl_isCountUpMode ) {		//カウントアップ形式.
				bl_ShowStartParts = False;
			} else {		//カウントダウン形式.
				bl_ShowEndParts = False;
			}
		}
		if( 0 == stUnitCheckDateTbl[uc_UTCnt].ss_EndYear ) {
			if( bl_isCountUpMode ) {		//カウントアップ形式.
				bl_ShowEndParts = False;
			} else {		//カウントダウン形式.
				bl_ShowStartParts = False;
			}
		}
		if( TYPE_ConsumableKind_TonerFilter == stUnitCheckDateTbl[uc_UTCnt].e_ConsumableKind ) {	// ﾄﾅｰﾌｨﾙﾀｰの場合.
			if( ( TYPE_MachineTypeZeus1   == e_MachineType ) || ( TYPE_MachineTypeZeus2   == e_MachineType ) || ( TYPE_MachineTypeZeus3   == e_MachineType )			// Zeus1/2/3非表示対応.
			    ||( TYPE_MachineTypeZeus1BK == e_MachineType ) || ( TYPE_MachineTypeZeus2BK == e_MachineType ) || ( TYPE_MachineTypeZeus3BK == e_MachineType ) 			// ZeusBK1/2/3非表示対応.
			   ||( TYPE_MachineTypeZeusS1  == e_MachineType ) || ( TYPE_MachineTypeZeusS2  == e_MachineType ) || ( TYPE_MachineTypeZeusS2_5 == e_MachineType ) 		// ZeusS/1/2/2_5.
// 2014/11/18【HeliosL 】 【HCLT-Chandrasekar】 【制御構成変更_No.1】 Adding Machine support for HeliosL Start.
			   ||( TYPE_MachineTypeHeliosL2 == e_MachineType ) || ( TYPE_MachineTypeHeliosL3  == e_MachineType ) 	// 製品タイプ：HeliosL2 / 製品タイプ：HeliosL3.
// comment 2015/07/22【HCLT-Shalini】【ZeusS_ZX0】【制御構成変更_No.7】 Addition of ZeusS_Z_EU,ZeusS_X_EU,ZeusS_Z_US_JP,ZeusS_X_US_JP,ZeusS0 Machine type.
//// comment 2015/06/29【HCLT-Senthil】【ZeusS_ZX0】【制御構成変更_No.5】 Addition of ZeusZX0 Machine type.
//		 	   || ( TYPE_MachineTypeZeusSZ == e_MachineType ) || ( TYPE_MachineTypeZeusSX == e_MachineType ) || ( TYPE_MachineTypeZeusS0 == e_MachineType ) ) { // 製品タイプ：ZeusSZ/X/0. 
// comment 2015/07/22【HCLT-Shalini】【ZeusS_ZX0】【制御構成変更_No.7】 Addition of ZeusS_Z_EU,ZeusS_X_EU,ZeusS_Z_US_JP,ZeusS_X_US_JP,ZeusS0 Machine type.
// comment 2015/12/18 OPS1-Sky福永 [ZeusS_ZX0][CQ522397] 機種の判定を変更する.
//				|| (TYPE_MachineTypeZeusSZ_US_JP == e_MachineType ) || ( TYPE_MachineTypeZeusSZ_EU == e_MachineType ) || ( TYPE_MachineTypeZeusSX_US_JP == e_MachineType )||( TYPE_MachineTypeZeusSX_EU == e_MachineType )||( TYPE_MachineTypeZeusS0 == e_MachineType ) ) {
// Comment 2017/01/18 HCLT_Anand [IT5_4.1] [制御構成変更_No.6] Added Machine condition for ZeusSBK2bnd 1,2 model to hide Toner Filter.
				|| ( TYPE_MachineTypeZeusSBK2nd1 == e_MachineType )
				|| ( TYPE_MachineTypeZeusSBK2nd2 == e_MachineType ) 
				) {		// ZeusS Z/X/0 は非表示対象から外す(表示する)
				bl_ShowStartParts = False;
				bl_ShowEndParts = False;

				Txt_UtilityUnitCheckP2TonauFilter.showEnable( False );		// 「トナーフィルター」ﾀｲﾄﾙﾃｷｽﾄ非表示.
				Bak_UtilityUnitCheckP2TonauFilter.showEnable( False );		// トナーフィルターの背景 非表示.
				Bak_UtilityUnitCheckP2TonauFilterGraphic.showEnable( False );	// トナーフィルター残量グラフィック背景 非表示.
			}
		}
		if( uc_NowPageNum != unitCheckTbl[uc_UTCnt].uc_PageNum ) {			// 表示中のページに配置するパーツのみパーツ描画を行う.
			bl_ShowStartParts = False;
			bl_ShowEndParts = False;
		}
		( ( PActivatableAdd* )( stUnitCheckDateTbl[uc_UTCnt].baseParts ) )->showEnableAdd( ( PNumberField* )( stUnitCheckDateTbl[uc_UTCnt].startParts ), bl_ShowStartParts );	//開始日ﾊﾟｰﾂ非表示.
		( ( PActivatableAdd* )( stUnitCheckDateTbl[uc_UTCnt].baseParts ) )->showEnableAdd( ( PNumberField* )( stUnitCheckDateTbl[uc_UTCnt].endParts ),   bl_ShowEndParts );	//終了日ﾊﾟｰﾂ非表示.
	}

	// 他のパーツの描画更新によりメーターが非表示となってしまうため、最後にメーターのグラフィックパーツの再描画を行う.
	for( register ushort j = 0; NULL != unitCheckTbl[ j ].parts; j++ ) {
		if( uc_NowPageNum == unitCheckTbl[ j ].uc_PageNum ) {
			unitCheckTbl[ j ].parts->showEnableShow( True );
		}
		if( PERROR_SmokeFilterUnitLife == unitCheckTbl[ j ].lifeUser ) {		// ﾄﾅｰﾌｨﾙﾀｰの場合.
			if( ( TYPE_MachineTypeZeus1   == e_MachineType ) || ( TYPE_MachineTypeZeus2   == e_MachineType ) || ( TYPE_MachineTypeZeus3   == e_MachineType ) 			// Zeus1/2/3非表示対応.
			    ||( TYPE_MachineTypeZeus1BK == e_MachineType ) || ( TYPE_MachineTypeZeus2BK == e_MachineType ) || ( TYPE_MachineTypeZeus3BK == e_MachineType ) 			// ZeusBK1/2/3非表示対応.
			   ||( TYPE_MachineTypeZeusS1  == e_MachineType ) || ( TYPE_MachineTypeZeusS2  == e_MachineType ) || ( TYPE_MachineTypeZeusS2_5 == e_MachineType ) 		// ZeusS/1/2/2_5非表示対応. ZeusS/1/2/2_5 Hide support.
// 2014/11/18【HeliosL 】 【HCLT-Chandrasekar】 【制御構成変更_No.1】 Adding Machine support for HeliosL Start.
			   ||( TYPE_MachineTypeHeliosL2 == e_MachineType ) || ( TYPE_MachineTypeHeliosL3  == e_MachineType ) // 製品タイプ：HeliosL2 / 製品タイプ：HeliosL3.
// comment 2015/07/22【HCLT-Shalini】【ZeusS_ZX0】【制御構成変更_No.7】 Addition of ZeusS_Z_EU,ZeusS_X_EU,ZeusS_Z_US_JP,ZeusS_X_US_JP,ZeusS0 Machine type.
//// comment 2015/06/29【HCLT-Senthil】【ZeusS_ZX0】【制御構成変更_No.5】 Addition of ZeusZX0 Machine type.
//		 	   || ( TYPE_MachineTypeZeusSZ == e_MachineType ) || ( TYPE_MachineTypeZeusSX == e_MachineType ) || ( TYPE_MachineTypeZeusS0 == e_MachineType ) ) { // 製品タイプ：ZeusSZ/X/0. 
// comment 2015/07/22【HCLT-Shalini】【ZeusS_ZX0】【制御構成変更_No.7】 Addition of ZeusS_Z_EU,ZeusS_X_EU,ZeusS_Z_US_JP,ZeusS_X_US_JP,ZeusS0 Machine type.
// comment 2015/12/18 OPS1-Sky福永 [ZeusS_ZX0][CQ522397] 機種の判定を変更する.
//				|| (TYPE_MachineTypeZeusSZ_US_JP == e_MachineType ) || ( TYPE_MachineTypeZeusSZ_EU == e_MachineType ) || ( TYPE_MachineTypeZeusSX_US_JP == e_MachineType )||( TYPE_MachineTypeZeusSX_EU == e_MachineType )||( TYPE_MachineTypeZeusS0 == e_MachineType ) ) {
// Comment 2017/01/18 HCLT_Anand [IT5_4.1] [制御構成変更_No.6] Added Machine condition for ZeusSBK2bnd 1,2 model to hide Toner Filter.
				|| ( TYPE_MachineTypeZeusSBK2nd1 == e_MachineType )
				|| ( TYPE_MachineTypeZeusSBK2nd2 == e_MachineType ) 
				) {		// ZeusS Z/X/0 は非表示対象から外す(表示する)

				( ( PRotationGraphic* )( unitCheckTbl[ j ].parts ) )->showEnable( False );						// 残量表示■Graphic非表示処理.
			}
		}
	}

// 画面更新処理は関数の最後に移動.
	// 1P/2P切り替え表示.
	if( 1 == uc_NowPageNum ) {	// 1P.
		if( True == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) {
// Comment 2015/01/30 HCLT-Venkataramesh [Compilation Switch ID: 200] for HeliosL support.
//#if DEF_Zeus5BkCus
#ifdef DEF_Zeus5BkCus
// comment 2015/06/15 OPS1酒井純 [Helios][CQ514489]Poseidonでのオゾンフィルター表示対応.
//			Grp_UtilityUnitCheck2P.showEnable( True );	// 消耗品確認・2P.
			Grp_UtilityUnitCheck2P.showEnable( False );	// 消耗品確認・2P.
#else
			Grp_UtilityUnitCheck2P.showEnable( False );	// 消耗品確認・2P.
#endif
			Grp_UtilityUnitCheck1P.showEnable( True );	// 消耗品確認・1P.
		} else {
			Grp_UtilityUnitCheck2P.showEnable( False );	// 消耗品確認・2P.
			Grp_UtilityUnitCheck1P.showEnable( True );	// 消耗品確認・1P.
		}
	} else {	// 2P.
		Grp_UtilityUnitCheck1P.showEnable( False );	// 消耗品確認・1P.
		Grp_UtilityUnitCheck2P.showEnable( True );	// 消耗品確認・2P.
	}

	// ページ表示.
#if defined(DEF_Zeus5BkCus) || defined(DEF_MonoCus)
//	if(True == PLBC_FuncVersionCheck::isOK_IT5_Type1()) {
//		Btx_UtilityNowPageDispErea.showEnable(False);
//		Key_UtilityUnitCheckPagePrevious.showEnable(False);
//		Key_UtilityUnitCheckPageNext.showEnable(False);
//	} else {
// comment 2015/06/15 OPS1酒井純 [Helios][CQ514489]Poseidonでのオゾンフィルター表示対応.
//comment 2017/05/10 【HCLT_Jagadeesh】　【IT5_4.1】 【1020552】 Hiding Page display information for ZeusSBK2.
//		Btx_UtilityNowPageDispErea.showEnable( True );
//		Key_UtilityUnitCheckPagePrevious.showEnable( True );
//		Key_UtilityUnitCheckPageNext.showEnable( True );
// comment 2015/06/15 OPS1酒井純 [Helios][CQ514489]Poseidonでのオゾンフィルター表示対応.	
//	}
// 2017/02/07 HCLT-Pushp Raj CQ[1008670], Screen No:- 10873000_KrS_消耗品確認, Hiding extra pages. STARTS.
#ifndef DEF_KRONOSS_Cus
#if defined (DEF_ZEUSSBK12_CUS) || defined(DEF_ZEUSSBKX0_CUS) || defined(DEF_ZEUSSBK2ND_12_CUS) || defined(DEF_ZEUSSBK2ND_ZX0_CUS)
		Btx_UtilityNowPageDispErea.showEnable( False );
		Key_UtilityUnitCheckPagePrevious.showEnable( False );
		Key_UtilityUnitCheckPageNext.showEnable( False );
#else //#if defined (DEF_ZEUSSBK12_CUS) || defined(DEF_ZEUSSBKX0_CUS) || defined(DEF_ZEUSSBK2ND_12_CUS) || defined(DEF_ZEUSSBK2ND_ZX0_CUS)	
		Btx_UtilityNowPageDispErea.showEnable( True );
		Key_UtilityUnitCheckPagePrevious.showEnable( True );
		Key_UtilityUnitCheckPageNext.showEnable( True );
#endif //#if defined (DEF_ZEUSSBK12_CUS) || defined(DEF_ZEUSSBKX0_CUS) || defined(DEF_ZEUSSBK2ND_12_CUS) || defined(DEF_ZEUSSBK2ND_ZX0_CUS)
#endif	//#ifndef DEF_KRONOSS_Cus
#endif
// 2017/02/07 HCLT-Pushp Raj CQ[1008670], Screen No:- 10873000_KrS_消耗品確認, Removal of page count display. STARTS.
#ifndef DEF_KRONOSS_Cus
	Btx_UtilityNowPageDispErea.set( &Num_UtilityUnitCheckNowPage, uc_NowPageNum, True );	// 現在のページ.
	Btx_UtilityNowPageDispErea.set( &Num_UtilityUnitCheckAllPage, uc_TotalPageNum, True );	// ページ総数.
#endif	//#ifndef DEF_KRONOSS_Cus
// 2017/02/07 HCLT-Pushp Raj CQ[1008670], Screen No:- 10873000_KrS_消耗品確認. ENDS.
// 2017/03/09 HCLT Vandana Modification for WP 349_5 Start
#ifdef DEF_KRONOSS_Cus
		if ( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( 227, 7 ) ) {
			Bak_UtilityUnitCheckP2FuserUnit.showEnable( False );
			Bak_UtilityUnitCheckP2FuserUnitGraphic.showEnable( False );
			Rot_UtilityUnitCheckP2FuserUnit.showEnable( False );
		} else {
			Bak_UtilityUnitCheckP2FuserUnit.showEnable( True );
			Bak_UtilityUnitCheckP2FuserUnitGraphic.showEnable( True );
			Rot_UtilityUnitCheckP2FuserUnit.showEnable( True );
			}			
#endif
// 2017/03/09 HCLT Vandana Modification for WP 349_5 End
}
//================================================================================================
// 概要		:ユーティリティ・消耗品確認 : 画面開始処理.
//================================================================================================
void PUtilityUnitCheck::startGamen()
{
	// 親クラスの画面開始処理をコール.
	PUtilityToMenuCom::startGamen();

	uc_NowPageNum = 1;		// 現在のページ.
	uc_TotalPageNum = 2;	// 総ページ.
	setLeftAreaEnableType( TYPD_LeftAreaJobList | TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );

	c_SystemControl.requestPolygonMotorStartFuserOn();
}
// ==================================================================
// PUtilityUnitCheck : 10873000_Do 消耗品確認: ボタン（押された）.
// ==================================================================
void PUtilityUnitCheck::procButtonOn( uchar option )
{
	if( option == 0 || option == 1 ) {	// ←前画面キー、次画面→キーの場合.
		if( option == 0 )			{ uc_NowPageNum--; }
		if( option == 1 )			{ uc_NowPageNum++; }
		if( uc_NowPageNum < 1 )	{ uc_NowPageNum = 2; }
		if( uc_NowPageNum > 2 )	{ uc_NowPageNum = 1; }
	} else {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}
	setupGamen();
}
//================================================================================================
// 概要		:ユーティリティ・ユニットチェック・プリント : コールバックイベント
//================================================================================================
void PUtilityUnitCheckPrint::procCallBack(
    TYPE_PNL_CallBack option )
{
	// 上クラスの処理をコールする
	PUtilityToMenuCom::procCallBack( option );
	if( option == TYPE_PNL_CallBack_functionEnd ) {
		pc_FeedTraySelectListPrint.setSPCPrintBusy( False );
		c_autoRestTimer.c_AutoResetAll.resetTimer( True );		// システムオートリセットのタイマー.
		c_autoRestTimer.c_autoResetOffTimer.resetTimer( True );	// オートリセットOFFタイマー.
	}
	setupGamen();
}
//================================================================================================
// 概要		:ユーティリティ・消耗品確認・プリント ：しおりの遷移可能不可能状態.
//================================================================================================
Bool PUtilityUnitCheckPrint::isEnableNextGamenBookmark()
{
	if( pc_FeedTraySelectListPrint.getSPCPrintBusy() == True ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
		// プリント中は、受付禁止.
		return False;
	}
	return True;
}
//================================================================================================
// 概要		:ユーティリティ・消耗品確認・プリント : 中止処理.
//================================================================================================
void PUtilityUnitCheckPrint::procCancel( uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }
	nextGamen = &gp_UtilityUnitCheck;	// ユーティリティ・消耗品確認.
}
//================================================================================================
// 概要		:ユーティリティ・ユニットチェック・プリント : スタート　
//================================================================================================
static void ans_startUnitCheckPrint(
    ushort us_Result )
{
	CallBackIndi( TYPE_PNL_CallBack_functionEnd );
}
void PUtilityUnitCheckPrint::procStart(
    uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }
	if( printStartDisable() == False ) {
		// システム状態（ＡＰＩ）・インスタンス取得
		APIC_SystemStatus* pc_SystemStatus = APIC_SystemStatus::newInstance();
		if( pc_SystemStatus->checkJobReserve() == False ) { // ジョブ予約不可のときは何もせずリターン.
			setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
			return;
		}
		// プリント許可の時
		APIC_ListPrint*	pc_ListPrint = APIC_ListPrint::newInstance();
// Comment 2017/10/25【HCLT-Narsimhmaswamy】【IT5 4.2】【RQ686】【WP1215】Multiple Network support.
//		pc_ListPrint->req_startListPrint( TYPE_ListPrintUnitCheck, ( ulong )pc_FeedTraySelectListPrint.getSPCFeedTray(), ans_startUnitCheckPrint, -1, 0, NULL, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel );
		pc_ListPrint->req_startListPrint( TYPE_ListPrintUnitCheck, ( uint32_t )pc_FeedTraySelectListPrint.getSPCFeedTray(), ans_startUnitCheckPrint, -1, 0, NULL, pc_FeedTraySelectListPrint.getSPCDuplexOn(), TYPE_JobEntryIF_Panel, 0, PNLC_NetworkLan::getInstance() ->getLine( ) );
		pc_FeedTraySelectListPrint.setSPCPrintBusy( True );
		setupGamen();
	} else {
		setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
	}
}
//================================================================================================
// 概要		:ユーティリティ・ユニットチェック・プリント : リセットキー
//================================================================================================
void PUtilityUnitCheckPrint::procReset(
    uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }
	PUtilityToMenuCom::procReset( option );
}
//================================================================================================
// 概要		:ユーティリティ・ユニットチェック・プリント : ユーティリティ
//================================================================================================
void PUtilityUnitCheckPrint::procUtility(
    uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }
	PUtilityToMenuCom::procUtility( option );
}
//================================================================================================
// 概要		:ユーティリティ・ユニットチェック・プリント : パワーセーブ
//================================================================================================
void PUtilityUnitCheckPrint::procPowerSave(
    uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }
	PUtilityToMenuCom::procPowerSave( option );
}
//================================================================================================
// 概要		:ユーティリティ・ユニットチェック・プリント : ユーザー補助
//================================================================================================
void PUtilityUnitCheckPrint::procAccessibility(
    uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }
	PUtilityToMenuCom::procAccessibility( option );
}
//================================================================================================
// 概要		:ユーティリティ・ユニットチェック・プリント : ボタン（押された）
//================================================================================================
void PUtilityUnitCheckPrint::procButtonOn(
    uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }
	pc_FeedTraySelectListPrint.SPCprocButtonOn( option );
	setupGamen();
}
//================================================================================================
// 概要		:ユーティリティ・ユニットチェック・プリント : ＯＫ
//================================================================================================
void PUtilityUnitCheckPrint::procOk(
    uchar option )
{
	if( pc_FeedTraySelectListPrint.checkSPCAnyKeyOn() == False ) { return; }
	nextGamen = &gp_UtilityUnitCheck;	// ユーティリティ・ユニットチェック
}
//================================================================================================
// 概要		:ユーティリティ・ユニットチェック・プリント : 画面開始処理
//================================================================================================
void PUtilityUnitCheckPrint::startGamen()
{
	PUtilityToMenuCom::startGamen();
	pc_FeedTraySelectListPrint.initSPC( PNLE_FeedTrayPatternListPrint );
	comENG.getENGDataRequest_PMCounter();
	if( APIC_KeyDevice::getKeyMode() == TYPE_KeyModeAdminMachineKM ) {
		onlyPanel.b_EnableStartAdminMachineKM = True;
	}
	if( False == PnlComGetSystemStatus()->getUserChoiceChanging() ) {
		PnlComGetSystemStatus()->setUserChoiceChanging( True );		// ユーザチョイス変更設定中.	// comment 2013/05/17 OPS1 nie 【ZM3rd】【UV-01273】.
	}
	setLeftAreaEnableType( TYPD_LeftAreaJobList | TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );
}
//================================================================================================
// 概要		:ユーティリティ・ユニットチェック・プリント : 画面終了処理
//================================================================================================
void PUtilityUnitCheckPrint::endGamen()
{
	if( APIC_KeyDevice::getKeyMode() == TYPE_KeyModeAdminMachineKM ) {
		onlyPanel.b_EnableStartAdminMachineKM = False;
	}
	if( True == PnlComGetSystemStatus()->getUserChoiceChanging() ) {
		PnlComGetSystemStatus()->setUserChoiceChanging( False );		// ユーザチョイス変更設定中解除.	// comment 2013/05/17 OPS1 nie 【ZM3rd】【UV-01273】.
	}
	PUtilityToMenuCom::endGamen();
}
//================================================================================================
// 概要		:ユーティリティ・ユニットチェック・プリント : 表示画面のセットアップ
//================================================================================================
void PUtilityUnitCheckPrint::setupGamen(
    short option )
{
	PUtilityToMenuCom::setupGamen( option );
	printLEDControl( pc_FeedTraySelectListPrint.checkSPCStartLED() );		// スタートキーＬＥＤの制御.
	pc_FeedTraySelectListPrint.SPCsetupGamen();			// 画面表示処理.
// comment 2014/12/10 OPS1-Sky西原 [HeliosL][制御構成変更No.1]封筒通紙の操作性改善（定着手動離間）.
	if( True == PnlComIsFuserRetracted() ){		// 定着離間中は印刷不可のメッセージに切り替え.
		Txt_SPC_MessageListPrint.showEnable( False );
		Txt_UtilityFuserRetractedErrorMessage.showEnable( True );
	} else {
		Txt_UtilityFuserRetractedErrorMessage.showEnable( False );
		Txt_SPC_MessageListPrint.showEnable( True );
	}
}
#ifdef	  DEF_THAMES_ICUSTOM	// アプリ割付.
//================================================================================================
// 概要		:ユーティリティ・ユニットチェック・プリント : ブラウザキー.
// 戻り値	:なし.
// 備考		:
//================================================================================================
void PUtilityUnitCheckPrint::procBrowser( uchar option )
{
	setBuzzerType( PCGE_BuzzerType_InputNg );		// 報知音設定.
}
#endif // DEF_THAMES_ICUSTOM	// アプリ割付.
//================================================================================================
// 概要		:ユーティリティ・アプリ詳細共通 : コンストラクタ
//================================================================================================
PUtilityApplicom::PUtilityApplicom(
    PDisplayParts** partsTable,
    short partsNumber,
    Bool isNoWarningGamen )
	: PUtilityToMenuCom( partsTable, partsNumber, isNoWarningGamen )
	,libe_CharSet( LIBE_CharSetASCII )
{
	ss_RPnlMaxStringInput = IMDD_TextInputArea;		// 文字入力画面の最大入力文字数
	ss_RPnlKeyResponseTimer = -1;					// キー入力通知からのタイマー
	ep_APPCharSet = EP_CharSet_Non;					// アプリ詳細表示の指定キャラクタ（アプリのシステムとして決定されている）k.inui 02/02/07 SJIS/macRoman対応により追加
	memset( &ps_preRequestDisplay, 0, sizeof( ps_preRequestDisplay ) );

	memset( sc_AppNameAndTitleArea, '\0', sizeof( sc_AppNameAndTitleArea ) );
	memset( sc_Message1Area, '\0', sizeof( sc_Message1Area ) );
	memset( sc_Message2Area, '\0', sizeof( sc_Message2Area ) );
	memset( sc_KeyMessageArea, '\0', sizeof( sc_KeyMessageArea ) );
}
//================================================================================================
// 概要		:ユーティリティ・アプリ詳細共通 : アプリ表示要求メッセージ処理
//================================================================================================
void PUtilityApplicom::procAppliDisplayReq(
    uchar option )
{
	// アプリ表示応答処理
	ss_RPnlKeyResponseTimer = -1;		// 表示要求を受け取ったのでタイマーを停止
	switch( onlyPanel.ps_RequestDisplay->e_Screen ) {
		case IMDE_Screen_Base:			nextGamen = &gp_UtilityAppliBasic;			break;	// ユーティリティ・コントローラ詳細（基本）
		case IMDE_Screen_InputNumber:	nextGamen = &gp_UtilityAppliNumberInput;	break;	// ユーティリティ・コントローラ詳細（数値入力）
		case IMDE_Screen_InputChar:		nextGamen = &gp_UtilityAppliTextInput;		break;	// ユーティリティ・コントローラ詳細（文字入力）
		default:	break;
	}
}
//================================================================================================
// 概要		:ユーティリティ・アプリ詳細共通 : リモートパネルクローズ要求
//================================================================================================
void PUtilityApplicom::procRPnlCloseRequest(
    uchar option )
{
	// クローズアンサー処理を終了ボタン押下と同じ処理を使う
	procOk( option );
}
//================================================================================================
// 概要		:ユーティリティ・アプリ詳細共通 : １秒タイマ
//================================================================================================
void PUtilityApplicom::procSecond(
    uchar option )
{
	PUtilityToMenuCom::procSecond( option );
	//[debug]リモートパネルデバッグする時は、以下if処理を無効にしてください。
	if( ss_RPnlKeyResponseTimer >= 0 ) {
		ss_RPnlKeyResponseTimer++;
		if( ss_RPnlKeyResponseTimer >= 3 ) {
			// カウント２だと１．５秒ぐらいになるので、カウント３とする
			// キー入力通知から２秒たっても応答がないので、クローズレポートを送信して画面を抜ける
			procEnd( option );
		}
		if( ss_RPnlKeyResponseTimer >= 60 ) {
			ss_RPnlKeyResponseTimer = 10;
		}
	}
}
//================================================================================================
// 概要		:ユーティリティ・アプリ詳細共通 : 終了
//================================================================================================
void PUtilityApplicom::procOk(
    uchar option )
{
	// 表示クローズ通知
	comRemotePanel->notifyCloseDisplay();
	PUtilityToMenuCom::procOk( option );
}
/*================================================================================================
［機能］	ユーティリティ・アプリ詳細共通 : オートリセット要求
================================================================================================*/
void PUtilityApplicom::procAutoResetReq( uchar option )
{
	// 表示クローズ通知
	comRemotePanel->notifyCloseDisplay();
	PUtilityToMenuCom::procAutoResetReq( option );
}
/*================================================================================================
［機能］	ユーティリティ・アプリ詳細共通 : 省エネ解除.
================================================================================================*/
void PUtilityApplicom::procEnergySaveReset( uchar option )
{
	// 表示クローズ通知
	comRemotePanel->notifyCloseDisplay();
	PUtilityToMenuCom::procEnergySaveReset( option );
}
/*================================================================================================
［機能］	ユーティリティ・アプリ詳細共通 : カウンタ引き抜き通知
================================================================================================*/
void PUtilityApplicom::procResetAtCountNotice( uchar option )
{
	// 表示クローズ通知
	comRemotePanel->notifyCloseDisplay();
	PUtilityToMenuCom::procResetAtCountNotice( option );
}
/*================================================================================================
［機能］	ユーティリティ・アプリ詳細共通 : リセットキー
［ｳｫｰｸｽﾙｰ］	未実施
================================================================================================*/
void PUtilityApplicom::procReset( uchar option )
{
	// 表示クローズ通知
	comRemotePanel->notifyCloseDisplay();
	PUtilityToMenuCom::procReset( option );
}
//================================================================================================
// 概要		:ユーティリティ・アプリ詳細共通 : 画面開始処理
//================================================================================================
void PUtilityApplicom::startGamen()
{
	PUtilityToMenuCom::startGamen();
	ss_RPnlKeyResponseTimer = -1;	// キー入力通知からのタイマー（－1はまだタイマーが起動していない）
	// アプリのキャラコードからパネルに通知するキャラコードを決定する
	switch( e_comCharacterCode ) {
		case IMDE_CharacterCode_MacRoman:	ep_APPCharSet = EP_CharSet_macRoman;	break;	// Mac Roman
		case IMDE_CharacterCode_ShiftJIS:	ep_APPCharSet = EP_CharSet_SJIS;		break;	// Shift JIS
		case IMDE_CharacterCode_USASCII:	ep_APPCharSet = EP_CharSet_USASCII;		break;	// USASCII
		case IMDE_CharacterCode_ISO8859_1:	ep_APPCharSet = EP_CharSet_ISO8859_1;	break;	// ISO8859-1(Latin1)
		case IMDE_CharacterCode_ISO8859_2:	ep_APPCharSet = EP_CharSet_ISO8859_2;	break;	// ISO8859-2(Latin2)
		default:	break;
	}
	if( onlyPanel.ps_RequestDisplay == NULL ) {
		onlyPanel.ps_RequestDisplay = &ps_preRequestDisplay;
	}
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );
}
//================================================================================================
// 概要		:ユーティリティ・アプリ詳細共通: 表示画面のセットアップ
//================================================================================================
void PUtilityApplicom::setupGamen(
    short option )
{
	PUtilityToMenuCom::setupGamen();					// ユーティリティメニューへの遷移共通
	// タイトル
	if( True == PLBC_FuncVersionCheck::isOK_Type3() ) {	// Donau拡張機能3、Amur機能拡張2以降.
		char sc_TitleArea[IMDD_AppNameAndTitleArea+1];
		memset( sc_TitleArea, '\0', sizeof( sc_TitleArea ) );
		strncpy( sc_TitleArea, onlyPanel.ps_RequestDisplay->sc_AppNameAndTitleArea, sizeof( sc_TitleArea ) - 1 );
		Tfd_TitleUtilityApplication.changeExternalText( ( uchar* )sc_TitleArea, 0, ep_APPCharSet, False, True );
	} else {
		Tfd_TitleUtilityApplication.changeExternalText( ( uchar* )onlyPanel.ps_RequestDisplay->sc_AppNameAndTitleArea, IMDD_AppNameAndTitleArea, ep_APPCharSet );
	}
	// メッセージ覧１
	char sc_resultString[IMDD_Message1Area * IMDD_Message1Line];	// 元データと同じ分だけのバッファを確保する
	memset( sc_Message1Area, 0, sizeof( sc_Message1Area ) );			// DonauLastFum【ネットワーク通信文字拡張】【第２電情技加藤た】CQ320873 2009/09/28.
	char sc_splitTextMessage[IMDD_Message1Area+1];		// Fieryのメッセージからメッセージ文字列の切り出し用バッファ.
	memset( sc_splitTextMessage, '\0', sizeof( sc_splitTextMessage ) );
	if( True == PLBC_FuncVersionCheck::isOK_Type3() ) {	// Donau拡張機能3、Amur機能拡張2以降.
		strncpy( sc_splitTextMessage, &( onlyPanel.ps_RequestDisplay->sc_Message1Area[0] ), sizeof( sc_splitTextMessage ) - 1 );
		Tfd_MessageUtilityApplication11.changeExternalText( ( uchar* )sc_splitTextMessage, 0, ep_APPCharSet, False, True );
	} else {
		setupInputString( 1, IMDD_Message1Area, onlyPanel.ps_RequestDisplay->sc_Message1Area,						sc_resultString, 0, ep_APPCharSet );	// 表示用文字列に整形
		Tfd_MessageUtilityApplication11.changeExternalText( ( uchar* )sc_resultString, 0, ep_APPCharSet );
	}
	if( True == PLBC_FuncVersionCheck::isOK_Type3() ) {	// Donau拡張機能3、Amur機能拡張2以降.
		strncpy( sc_splitTextMessage, &( onlyPanel.ps_RequestDisplay->sc_Message1Area[IMDD_Message1Area] ), sizeof( sc_splitTextMessage ) - 1 );
		Tfd_MessageUtilityApplication12.changeExternalText( ( uchar* )sc_splitTextMessage, 0, ep_APPCharSet, False, True );
	} else {
		setupInputString( 1, IMDD_Message1Area, &onlyPanel.ps_RequestDisplay->sc_Message1Area[IMDD_Message1Area],	sc_resultString, 0, ep_APPCharSet );	// 表示用文字列に整形
		Tfd_MessageUtilityApplication12.changeExternalText( ( uchar* )sc_resultString, 0, ep_APPCharSet );
	}
	/*================================================================================================
		 ： Fiery対応
	================================================================================================*/
	printLEDControl( False );	// スタートキーＬＥＤの制御
}
//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（基本）: ボタン（押された）
//================================================================================================
void PUtilityAppliBasic::procButtonOn(
    uchar option )
{
	comRemotePanel->notifyOerationKey( option );
	ss_RPnlKeyResponseTimer = 0;	// キー入力通知からのタイマー起動
	setupGamen();	// 表示部品のセットアップ
}

//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（基本）: 画面開始処理.
//================================================================================================
void PUtilityAppliBasic::startGamen()
{
	PUtilityApplicom::startGamen();
	setLeftAreaEnableType( TYPD_LeftAreaSoft10Key );
	selectLeftAreaType( TYPD_LeftAreaSoft10Key );
	PJobListControl::b_JobListIndexFlag = True;

}

//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（基本）: 表示画面のセットアップ
//================================================================================================
void PUtilityAppliBasic::setupGamen(
    short option )
{
	PUtilityApplicom::setupGamen( option );
	// メッセージ覧２
	char sc_resultString[IMDD_Message2Area * IMDD_Message2Line];	// 元データと同じ分だけのバッファを確保する
	memset( sc_resultString, 0, sizeof( sc_resultString ) );
	memset( sc_Message2Area, 0, sizeof( sc_Message2Area ) );			// DonauLastFum【ネットワーク通信文字拡張】【第２電情技加藤た】CQ320873 2009/09/28.
	char sc_splitTextMessage[IMDD_Message2Area+1];		// Fieryのメッセージからメッセージ文字列の切り出し用バッファ.
	char sc_splitTextKey[IMDD_KeyMessageArea+1];
	memset( sc_splitTextMessage, '\0', sizeof( sc_splitTextMessage ) );
	memset( sc_splitTextKey, '\0', sizeof( sc_splitTextKey ) );

	if( True == PLBC_FuncVersionCheck::isOK_Type3() ) {	// Donau拡張機能3、Amur機能拡張2以降.
		strncpy( sc_splitTextMessage, &( onlyPanel.ps_RequestDisplay->sc_Message2Area[0] ), sizeof( sc_splitTextMessage ) - 1 );
		Tfd_UtilityAppliBasicMessage21.changeExternalText( ( uchar* )sc_splitTextMessage, 0, ep_APPCharSet, False, True );
	} else {
		setupInputString( 1, IMDD_Message2Area, onlyPanel.ps_RequestDisplay->sc_Message2Area,						sc_resultString, 0, ep_APPCharSet );	// 表示用文字列に整形
		Tfd_UtilityAppliBasicMessage21.changeExternalText( ( uchar* )sc_resultString, 0, ep_APPCharSet );
	}
	if( True == PLBC_FuncVersionCheck::isOK_Type3() ) {	// Donau拡張機能3、Amur機能拡張2以降.
		strncpy( sc_splitTextMessage, &( onlyPanel.ps_RequestDisplay->sc_Message2Area[IMDD_Message2Area] ), sizeof( sc_splitTextMessage ) - 1 );
		Tfd_UtilityAppliBasicMessage22.changeExternalText( ( uchar* )sc_splitTextMessage, 0, ep_APPCharSet, False, True );
	} else {
		setupInputString( 1, IMDD_Message2Area, &onlyPanel.ps_RequestDisplay->sc_Message2Area[IMDD_Message2Area  ],	sc_resultString, 0, ep_APPCharSet );	// 表示用文字列に整形
		Tfd_UtilityAppliBasicMessage22.changeExternalText( ( uchar* )sc_resultString, 0, ep_APPCharSet );
	}
	if( True == PLBC_FuncVersionCheck::isOK_Type3() ) {	// Donau拡張機能3、Amur機能拡張2以降.
		strncpy( sc_splitTextMessage, &( onlyPanel.ps_RequestDisplay->sc_Message2Area[IMDD_Message2Area*2] ), sizeof( sc_splitTextMessage ) - 1 );
		Tfd_UtilityAppliBasicMessage23.changeExternalText( ( uchar* )sc_splitTextMessage, 0, ep_APPCharSet, False, True );
	} else {
		setupInputString( 1, IMDD_Message2Area, &onlyPanel.ps_RequestDisplay->sc_Message2Area[IMDD_Message2Area*2],	sc_resultString, 0, ep_APPCharSet );	// 表示用文字列に整形
		Tfd_UtilityAppliBasicMessage23.changeExternalText( ( uchar* )sc_resultString, 0, ep_APPCharSet );
	}
	if( True == PLBC_FuncVersionCheck::isOK_Type3() ) {	// Donau拡張機能3、Amur機能拡張2以降.
		strncpy( sc_splitTextMessage, &( onlyPanel.ps_RequestDisplay->sc_Message2Area[IMDD_Message2Area*3] ), sizeof( sc_splitTextMessage ) - 1 );
		Tfd_UtilityAppliBasicMessage24.changeExternalText( ( uchar* )sc_splitTextMessage, 0, ep_APPCharSet, False, True );
	} else {
		setupInputString( 1, IMDD_Message2Area, &onlyPanel.ps_RequestDisplay->sc_Message2Area[IMDD_Message2Area*3],	sc_resultString, 0, ep_APPCharSet );	// 表示用文字列に整形
		Tfd_UtilityAppliBasicMessage24.changeExternalText( ( uchar* )sc_resultString, 0, ep_APPCharSet );
	}
	// キー
	if( True == PLBC_FuncVersionCheck::isOK_Type3() ) {	// Donau拡張機能3、Amur機能拡張2以降.
		strncpy( sc_splitTextKey, onlyPanel.ps_RequestDisplay->sc_KeyMessageArea[0], sizeof( sc_splitTextKey ) - 1 );
		Key_UtilityAppliBasicKey1.changeExternalText( ( uchar* )sc_splitTextKey, 0, ep_APPCharSet, False, True );		// 右側・上のキー
	} else {
		Key_UtilityAppliBasicKey1.changeExternalText( ( uchar* )onlyPanel.ps_RequestDisplay->sc_KeyMessageArea[0], IMDD_KeyMessageArea, ep_APPCharSet );		// 右側・上のキー
	}
	if( True == PLBC_FuncVersionCheck::isOK_Type3() ) {	// Donau拡張機能3、Amur機能拡張2以降.
		strncpy( sc_splitTextKey, onlyPanel.ps_RequestDisplay->sc_KeyMessageArea[1], sizeof( sc_splitTextKey ) - 1 );
		Key_UtilityAppliBasicKey2.changeExternalText( ( uchar* )sc_splitTextKey, 0, ep_APPCharSet, False, True );		// 右側・上から２番目のキー
	} else {
		Key_UtilityAppliBasicKey2.changeExternalText( ( uchar* )onlyPanel.ps_RequestDisplay->sc_KeyMessageArea[1], IMDD_KeyMessageArea, ep_APPCharSet );		// 右側・上から２番目のキー
	}
	if( True == PLBC_FuncVersionCheck::isOK_Type3() ) {	// Donau拡張機能3、Amur機能拡張2以降.
		strncpy( sc_splitTextKey, onlyPanel.ps_RequestDisplay->sc_KeyMessageArea[2], sizeof( sc_splitTextKey ) - 1 );
		Key_UtilityAppliBasicKey3.changeExternalText( ( uchar* )sc_splitTextKey, 0, ep_APPCharSet, False, True );		// 右側・上から３番目のキー
	} else {
		Key_UtilityAppliBasicKey3.changeExternalText( ( uchar* )onlyPanel.ps_RequestDisplay->sc_KeyMessageArea[2], IMDD_KeyMessageArea, ep_APPCharSet );		// 右側・上から３番目のキー
	}
	if( True == PLBC_FuncVersionCheck::isOK_Type3() ) {	// Donau拡張機能3、Amur機能拡張2以降.
		strncpy( sc_splitTextKey, onlyPanel.ps_RequestDisplay->sc_KeyMessageArea[3], sizeof( sc_splitTextKey ) - 1 );
		Key_UtilityAppliBasicKey4.changeExternalText( ( uchar* )sc_splitTextKey, 0, ep_APPCharSet, False, True );		// 右側・下のキー
	} else {
		Key_UtilityAppliBasicKey4.changeExternalText( ( uchar* )onlyPanel.ps_RequestDisplay->sc_KeyMessageArea[3], IMDD_KeyMessageArea, ep_APPCharSet );		// 右側・下のキー
	}
	if( True == PLBC_FuncVersionCheck::isOK_Type3() ) {	// Donau拡張機能3、Amur機能拡張2以降.
		strncpy( sc_splitTextKey, onlyPanel.ps_RequestDisplay->sc_KeyMessageArea[4], sizeof( sc_splitTextKey ) - 1 );
		Key_UtilityAppliBasicKey5.changeExternalText( ( uchar* )sc_splitTextKey, 0, ep_APPCharSet, False, True );		// 下側・左端のキー
	} else {
		Key_UtilityAppliBasicKey5.changeExternalText( ( uchar* )onlyPanel.ps_RequestDisplay->sc_KeyMessageArea[4], IMDD_KeyMessageArea, ep_APPCharSet );		// 下側・左端のキー
	}
	if( True == PLBC_FuncVersionCheck::isOK_Type3() ) {	// Donau拡張機能3、Amur機能拡張2以降.
		strncpy( sc_splitTextKey, onlyPanel.ps_RequestDisplay->sc_KeyMessageArea[5], sizeof( sc_splitTextKey ) - 1 );
		Key_UtilityAppliBasicKey6.changeExternalText( ( uchar* )sc_splitTextKey, 0, ep_APPCharSet, False, True );		// 下側・左から２番目のキー
	} else {
		Key_UtilityAppliBasicKey6.changeExternalText( ( uchar* )onlyPanel.ps_RequestDisplay->sc_KeyMessageArea[5], IMDD_KeyMessageArea, ep_APPCharSet );		// 下側・左から２番目のキー
	}
	if( True == PLBC_FuncVersionCheck::isOK_Type3() ) {	// Donau拡張機能3、Amur機能拡張2以降.
		strncpy( sc_splitTextKey, onlyPanel.ps_RequestDisplay->sc_KeyMessageArea[6], sizeof( sc_splitTextKey ) - 1 );
		Key_UtilityAppliBasicKey7.changeExternalText( ( uchar* )sc_splitTextKey, 0, ep_APPCharSet, False, True );		// 下側・左から３番目のキー
	} else {
		Key_UtilityAppliBasicKey7.changeExternalText( ( uchar* )onlyPanel.ps_RequestDisplay->sc_KeyMessageArea[6], IMDD_KeyMessageArea, ep_APPCharSet );		// 下側・左から３番目のキー
	}
	if( True == PLBC_FuncVersionCheck::isOK_Type3() ) {	// Donau拡張機能3、Amur機能拡張2以降.
		strncpy( sc_splitTextKey, onlyPanel.ps_RequestDisplay->sc_KeyMessageArea[7], sizeof( sc_splitTextKey ) - 1 );
		Key_UtilityAppliBasicKey8.changeExternalText( ( uchar* )sc_splitTextKey, 0, ep_APPCharSet, False, True );		// 下側・右端のキー
	} else {
		Key_UtilityAppliBasicKey8.changeExternalText( ( uchar* )onlyPanel.ps_RequestDisplay->sc_KeyMessageArea[7], IMDD_KeyMessageArea, ep_APPCharSet );		// 下側・右端のキー
	}
	// キーの反転表示
	Key_UtilityAppliBasicKey1.select( onlyPanel.ps_RequestDisplay->e_KeyMessageAreaStatus[0] == IMDE_KeyMessageAreaStatus_Reverse ? True : False );		// 右側・上のキー
	Key_UtilityAppliBasicKey2.select( onlyPanel.ps_RequestDisplay->e_KeyMessageAreaStatus[1] == IMDE_KeyMessageAreaStatus_Reverse ? True : False );		// 右側・上から２番目のキー
	Key_UtilityAppliBasicKey3.select( onlyPanel.ps_RequestDisplay->e_KeyMessageAreaStatus[2] == IMDE_KeyMessageAreaStatus_Reverse ? True : False );		// 右側・上から３番目のキー
	Key_UtilityAppliBasicKey4.select( onlyPanel.ps_RequestDisplay->e_KeyMessageAreaStatus[3] == IMDE_KeyMessageAreaStatus_Reverse ? True : False );		// 右側・下のキー
	Key_UtilityAppliBasicKey5.select( onlyPanel.ps_RequestDisplay->e_KeyMessageAreaStatus[4] == IMDE_KeyMessageAreaStatus_Reverse ? True : False );		// 下側・左端のキー
	Key_UtilityAppliBasicKey6.select( onlyPanel.ps_RequestDisplay->e_KeyMessageAreaStatus[5] == IMDE_KeyMessageAreaStatus_Reverse ? True : False );		// 下側・左から２番目のキー
	Key_UtilityAppliBasicKey7.select( onlyPanel.ps_RequestDisplay->e_KeyMessageAreaStatus[6] == IMDE_KeyMessageAreaStatus_Reverse ? True : False );		// 下側・左から３番目のキー
	Key_UtilityAppliBasicKey8.select( onlyPanel.ps_RequestDisplay->e_KeyMessageAreaStatus[7] == IMDE_KeyMessageAreaStatus_Reverse ? True : False );		// 下側・右端のキー
	// キーの表示／非表示
	Key_UtilityAppliBasicKey1.showEnable( onlyPanel.ps_RequestDisplay->e_KeyMessageAreaStatus[0] == IMDE_KeyMessageAreaStatus_Disable ? False : True );		// 右側・上のキー
	Key_UtilityAppliBasicKey2.showEnable( onlyPanel.ps_RequestDisplay->e_KeyMessageAreaStatus[1] == IMDE_KeyMessageAreaStatus_Disable ? False : True );		// 右側・上から２番目のキー
	Key_UtilityAppliBasicKey3.showEnable( onlyPanel.ps_RequestDisplay->e_KeyMessageAreaStatus[2] == IMDE_KeyMessageAreaStatus_Disable ? False : True );		// 右側・上から３番目のキー
	Key_UtilityAppliBasicKey4.showEnable( onlyPanel.ps_RequestDisplay->e_KeyMessageAreaStatus[3] == IMDE_KeyMessageAreaStatus_Disable ? False : True );		// 右側・下のキー
	Key_UtilityAppliBasicKey5.showEnable( onlyPanel.ps_RequestDisplay->e_KeyMessageAreaStatus[4] == IMDE_KeyMessageAreaStatus_Disable ? False : True );		// 下側・左端のキー
	Key_UtilityAppliBasicKey6.showEnable( onlyPanel.ps_RequestDisplay->e_KeyMessageAreaStatus[5] == IMDE_KeyMessageAreaStatus_Disable ? False : True );		// 下側・左から２番目のキー
	Key_UtilityAppliBasicKey7.showEnable( onlyPanel.ps_RequestDisplay->e_KeyMessageAreaStatus[6] == IMDE_KeyMessageAreaStatus_Disable ? False : True );		// 下側・左から３番目のキー
	Key_UtilityAppliBasicKey8.showEnable( onlyPanel.ps_RequestDisplay->e_KeyMessageAreaStatus[7] == IMDE_KeyMessageAreaStatus_Disable ? False : True );		// 下側・右端のキー
}
//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（基本）(2250)  : 左エリア・しおりの遷移可能不可能状態.
//================================================================================================
Bool PUtilityAppliBasic::isEnableNextGamenBookmark()
{
	return False;
}

//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（文字入力）: コンストラクタ
//================================================================================================
PUtilityAppliTextInput::PUtilityAppliTextInput(
    PDisplayParts** partsTable,
    short partsNumber,
    Bool isNoWarningGamen )
	: PUtilityApplicom( partsTable, partsNumber, isNoWarningGamen )
	,b_MmbKeybordCaps( False )
{
	sc_MmbText[0]     = '\0';
	ep_MmbCharSet     = EP_CharSet_Non;
	pc_MmbDispField   = &Btx_UtilityAddressRegInputKeybord;
	ss_MmbMaxInputNum = IMDD_TextInputArea;
}
//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（文字入力）: テンキー
//================================================================================================
void PUtilityAppliTextInput::proc10Key(
    uchar option )
{
	char    sc_Num[PCGD_NumberKeyCode];
	int32_t	i32_InputNum = 0;
	Kbd_UtilityInputKeybordConv.proc10Key( &option );			// 10キー入力からシングルタップ変換処理.
	if( 255 == option ) {
		setupGamen();
		return;
	}
	// 10キー入力値の取得.
	i32_InputNum = proc10KeyInput( i32_InputNum, PCGD_MaxKyebordInput, option, False, EP_InputMaxLimitPattern_Refuse );
	if( ( 0 <= i32_InputNum ) && ( 9 >= i32_InputNum ) ) {
		// 入力されたのが0-9キーの時のみ処理を受け付ける.
		snprintf( sc_Num, sizeof(sc_Num), "%d", i32_InputNum );
		// 文字を追加.
		c_SlideTextMgr.insertStr( sc_MmbText, sc_Num );
	} else {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
	setupGamen();
}
//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（文字入力）: クリアー
//================================================================================================
void PUtilityAppliTextInput::procClear(
    uchar option )
{
	// 文字列の初期化.
	memset( sc_MmbText, 0, sizeof( sc_MmbText ) );
	// スライド文字列管理クラスの初期化.
	c_SlideTextMgr.init_CLCD( sc_MmbText, PCGD_KeyboardDispLenMax, ss_MmbMaxInputNum, pc_MmbDispField, getCharSetForPanel() );
	setupGamen();
}
//================================================================================================
// 概要		:管理者設定・宛先登録・スキャナ登録・キーボード共通 : シャープキー
//================================================================================================
void PUtilityAppliTextInput::procShrp(
    uchar option )
{
	char    sc_Num[PCGD_NumberKeyCode];
	// 入力された#の表示を行う.
	sc_Num[0] = '#';
	sc_Num[1] = '\0';
	c_SlideTextMgr.insertStr( sc_MmbText, sc_Num );     // 文字を追加.
	setupGamen();
}
//================================================================================================
// 概要		:管理者設定・宛先登録・スキャナ登録・キーボード共通 : アスタリスクキー
//================================================================================================
void PUtilityAppliTextInput::procAster(
    uchar option )
{
	char    sc_Num[PCGD_NumberKeyCode];
	// 入力された*の表示を行う.
	sc_Num[0] = '*';
	sc_Num[1] = '\0';
	c_SlideTextMgr.insertStr( sc_MmbText, sc_Num );     // 文字を追加.
	setupGamen();
}
//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（文字入力） : キーボード
//================================================================================================
void PUtilityAppliTextInput::procKeyBoard(
    uchar option )
{
	Kbd_UtilityInputKeybordConv.procCursorPosition( ( char* )&c_SlideTextMgr, sc_MmbText, option, False );	// カーソル位置処理
	setupGamen();
}
//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（文字入力）: ボタン（押された）
//================================================================================================
void PUtilityAppliTextInput::procButtonOn(
    uchar option )
{
	// 標準⇔拡大切換えキー押下処理.
	if( 100 == option ) {
		// キーボード初期状態判定フラグをTrueにする.
		onlyPanel.b_InitKeybord = True;
		if( PLBD_Enl_KBD_Normal == Kbd_UtilityInputKeybordConv.setInitKeybordStatus2() )	{ nextGamen = this; }	// startGamenから描画するため、nextGamenに自画面を設定.
		else																				{ nextGamen = this; }	// startGamenから描画するため、nextGamenに自画面を設定.
		return;
	} else {
		switch( option ) {
			case 0:			// 確定するキー
				setupInputString( 1, ss_MmbMaxInputNum, sc_MmbText, &onlyPanel.ps_RequestDisplay->sc_Message1Area[IMDD_Message1Area], 0, ep_MmbCharSet );
				DivCommon_DivlibShareShm::Shm_RequestDisplay_set( onlyPanel.ps_RequestDisplay );
				// 操作キーＯＮ通知
				comRemotePanel->notifyOerationKey( 19 );
				ss_RPnlKeyResponseTimer = 0;	// キー入力通知からのタイマー起動
				break;
			case 1:			// 確定しないキー
				// 操作キーＯＮ通知
				comRemotePanel->notifyOerationKey( 20 );
				ss_RPnlKeyResponseTimer = 0;	// キー入力通知からのタイマー起動
				break;
			default: break;
		}
	}
	// ユーティリティ専用キーボード共通ボタン（押された）.
	procButtonOnKeybordUtility( option );
	setupGamen();
}
//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（文字入力）: 画面開始処理
//================================================================================================
void PUtilityAppliTextInput::startGamen()
{
	// カーソルクラスへパーツポインタを渡す.
	Kbd_UtilityInputKeybordConv.GetCursor( &Btx_UtilityAddressRegInputKeybord );
	// 入力文字表示領域を変更.
	pc_MmbDispField = &Btx_UtilityAddressRegInputKeybord;
	// キーボード初期状態の場合の処理.
	if( False == onlyPanel.b_InitKeybord ) {
		Kbd_UtilityInputKeybordConv = PLBC_ConvertKeybord( PLBD_KBT_ASCII_S, 192, 152 );	// 位置をずらす.
		// 上記位置ずらす際にカーソルクラスのポインタがクリアされてしまうので、再度セットをする.
		Kbd_UtilityInputKeybordConv.GetCursor( &Btx_UtilityAddressRegInputKeybord );
		// キーボード切換え状態を初期状態とする.
		Kbd_UtilityInputKeybordConv.setChangeEnlKeybord( PLBD_Enl_KBD_Initial );
		//格納領域初期化
		memset( sc_MmbText,'\0',sizeof( sc_MmbText ) );
		setupInputString( 1, IMDD_TextInputArea, &onlyPanel.ps_RequestDisplay->sc_Message1Area[IMDD_Message1Area], sc_MmbText, 0, getCharSetForPanel() );
	}
	// 拡大表示の場合、入力文字表示領域を変更.
	if( True == Kbd_UtilityInputKeybordConv.setDispFieldEnl( &Btx_EnlChangeKeybord ) ) { pc_MmbDispField = &Btx_EnlChangeKeybord; }
	// ユーティリティ専用キーボード共通開始処理.
	startGamenKeybordUtility9E();
	// フォントセット(キャラクタセット)を取得.
	ep_MmbCharSet = getCharSetForPanel();
	// スライド文字列管理クラスの初期化.
	c_SlideTextMgr.init_CLCD( sc_MmbText, ( char )Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MmbMaxInputNum, pc_MmbDispField, ep_MmbCharSet );
	PUtilityApplicom::startGamen();
	// キーボード初期状態判定フラグをFalseとする.
	onlyPanel.b_InitKeybord = False;
}
//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（文字入力）: 表示画面のセットアップ
//================================================================================================
void PUtilityAppliTextInput::setupGamen(
    short option )
{
	// タイトル
	if( True == PLBC_FuncVersionCheck::isOK_Type3() ) {	// Donau拡張機能3、Amur機能拡張2以降.
		char sc_TitleArea[IMDD_AppNameAndTitleArea+1];
		memset( sc_TitleArea, '\0', sizeof( sc_TitleArea ) );
		strncpy( sc_TitleArea, onlyPanel.ps_RequestDisplay->sc_AppNameAndTitleArea, sizeof( sc_TitleArea ) - 1 );
		Tfd_TitleUtilityApplication.changeExternalText( ( uchar* )sc_TitleArea, 0, ep_APPCharSet, False, True );
	} else {
		Tfd_TitleUtilityApplication.changeExternalText( ( uchar* )onlyPanel.ps_RequestDisplay->sc_AppNameAndTitleArea, IMDD_AppNameAndTitleArea, ep_APPCharSet );
	}
	// メッセージ覧１
	char sc_resultString[IMDD_Message1Area+1];
	memset( sc_resultString, '\0', sizeof( sc_resultString ) );
	if( True == PLBC_FuncVersionCheck::isOK_Type3() ) {	// Donau拡張機能3、Amur機能拡張2以降.
		strncpy( sc_resultString, onlyPanel.ps_RequestDisplay->sc_Message1Area, sizeof( sc_resultString ) - 1 );
		Tfd_UtilityAppliTextMessage.changeExternalText( ( uchar* )sc_resultString, 0, ep_APPCharSet, False, True );
	} else {
		setupInputString( 1, IMDD_Message1Area, onlyPanel.ps_RequestDisplay->sc_Message1Area, sc_resultString, 0, ep_APPCharSet );		// 表示用文字列に整形
		Tfd_UtilityAppliTextMessage.changeExternalText( ( uchar* )sc_resultString, IMDD_Message1Area, ep_APPCharSet );
	}
	// キーボード標準状態のsetup処理.
	if( PLBD_Enl_KBD_Normal == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
		PUtilityApplicom::setupGamen( option );
	}
	// キーボード拡大状態のsetup処理.
	else {
		// 標準のキーボード画面固有パーツを消す.
		Grp_KeybordTotalPartsO5.showEnable( False );
		Btx_UtilityAddressRegInputKeybord.showEnable( False );
		Tfd_TitleUtilityApplication.showEnable( False );
		Tfd_UtilityAppliTextMessage.showEnable( False );
	}
	// ユーティリティ専用キーボード共通セットアップ.
	setupGamenKeybordUtility();
	c_SlideTextMgr.drawStr( sc_MmbText );
	// スタートLED点灯
	printLEDControl( False );
	Kbd_UtilityInputKeybordConv.setDisableKeyCharCode( KC_DisableKeyPatternNon );
}
//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（文字入力）(2251)  : 左エリア・しおりの遷移可能不可能状態.
//================================================================================================
Bool PUtilityAppliTextInput::isEnableNextGamenBookmark()
{
	return False;
}
//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（文字入力） : 背景画面のセット.
//================================================================================================
void PUtilityAppliTextInput::setDefaultBackParts()
{
	if( ( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
	    && ( True == onlyPanel.b_InitKeybord ) ) {	// 拡大用デフォルト背景.
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_UtilityAppliTextInput2Default, ss_UtilityAppliTextInput2Default );
	} else {	// 標準用デフォルト背景.
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_UtilityAppliTextInputDefault, ss_UtilityAppliTextInputDefault );
	}
}
//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（文字入力） : 画面終了処理.
//================================================================================================
void PUtilityAppliTextInput::endGamen()
{
	PUtilityApplicom::endGamen();
	if( onlyPanel.b_InitKeybord == False ) {
		Kbd_UtilityInputKeybordConv = PLBC_ConvertKeybord( PLBD_KBT_ASCII_S, 192, 168 );	// 位置を標準値に戻す.
		Kbd_UtilityInputKeybordConv.setChangeEnlKeybord( PLBD_Enl_KBD_Initial );
	}
}
//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（数値入力）: コンストラクタ
//================================================================================================
PUtilityAppliNumberInput::PUtilityAppliNumberInput(
    PDisplayParts** partsTable,
    short partsNumber,
    Bool isNoWarningGamen )
	: PUtilityApplicom( partsTable, partsNumber, isNoWarningGamen )
{
	ss_digit = 0;
	for( register short i = 0; i < IMDD_NumberInputArea+2; i++ ) {
		sc_TargetString[i] = '\0';		// NULLクリアする
	}
}
//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（数値入力）: テンキー
//================================================================================================
void PUtilityAppliNumberInput::proc10Key(
    uchar option )
{
	if( option <= 9 && ss_digit < IMDD_NumberInputArea
	    && ( ( sc_TargetString[ss_digit] >= '0' && sc_TargetString[ss_digit] <= '9' ) || sc_TargetString[ss_digit] == '_' || sc_TargetString[ss_digit] == ' ' ) ) {
		// 入力値が9以下、かつ、最大入力文字数以下、かつ、入力位置の文字列が数値かアンダーバーかスペースの時（入力がゆるされるのは数値かアンダーバーのみ）
		sc_TargetString[ss_digit] = static_cast<char>( option + '0' );		// 入力位置に数字を入れる
		ss_digit++;
		while( ( sc_TargetString[ss_digit] < '0' || sc_TargetString[ss_digit] > '9' )
		       && sc_TargetString[ss_digit] != '_' && sc_TargetString[ss_digit] != ' ' && sc_TargetString[ss_digit] != '\0' && ss_digit < IMDD_NumberInputArea ) {
			// 0～9以外、アンダーバー以外、スペース以外、NULL以外、最大入力文字数以下はポインタを進める
			ss_digit++;
		}
	}
	setupGamen();
}
//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（数値入力）: クリアー
//================================================================================================
void PUtilityAppliNumberInput::procClear(
    uchar option )
{
	for( register short i = 0; i < IMDD_NumberInputArea+2; i++ ) {
		sc_TargetString[i] = '\0';		// NULLクリアする
	}
	setupInputString( 1, IMDD_NumberInputArea, onlyPanel.ps_RequestDisplay->sc_NumberInputArea, sc_TargetString, 0, ep_APPCharSet );
	ss_digit = 0;
	// アンダーバー以外、NULL以外はポインタを進める
	while( sc_TargetString[ss_digit] != '_' && sc_TargetString[ss_digit] != '\0' && ss_digit < IMDD_NumberInputArea ) {
		ss_digit++;
	}
	setupGamen();
}
//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（数値入力）: ボタン（押された）
//================================================================================================
void PUtilityAppliNumberInput::procButtonOn(
    uchar option )
{
	register short i = 0;
	register Bool bl_find = False;
	switch( option ) {
		case 0:			// 確定するキー
			// 未入力エリアがないかチェックする
			while( sc_TargetString[i] != '\0' ) {
				if( sc_TargetString[i] == '_' ) {
					bl_find = True;
					break;
				}
				i++;
			}
			// 未入力エリア（アンダーバー）が見つからなかったら、「確定する」を受け付ける
			if( bl_find == False ) {
				setupInputString( 1, IMDD_NumberInputArea, sc_TargetString, onlyPanel.ps_RequestDisplay->sc_NumberInputArea, 0, ep_APPCharSet );
				// 操作キーＯＮ通知
				comRemotePanel->notifyOerationKey( 18 );
				ss_RPnlKeyResponseTimer = 0;	// キー入力通知からのタイマー起動
			}
			break;
		case 1:			// 確定しないキー
			// 操作キーＯＮ通知
			comRemotePanel->notifyOerationKey( 20 );
			ss_RPnlKeyResponseTimer = 0;	// キー入力通知からのタイマー起動
			break;
		case 2:			// 削除キー
			if( ss_digit > 0 ) {
				ss_digit--;
				while( ( sc_TargetString[ss_digit] < '0' || sc_TargetString[ss_digit] > '9' )
				       && sc_TargetString[ss_digit] != '_'
				       && sc_TargetString[ss_digit] != ' '
				       && sc_TargetString[ss_digit] != '\0'
				       && ss_digit > 0 ) {
					ss_digit--;
				}
				sc_TargetString[ss_digit] = '_';
			}
			break;
		default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); break;
	}
	setupGamen();
}
//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（数値入力）: 画面開始処理
//================================================================================================
void PUtilityAppliNumberInput::startGamen()
{
	PUtilityApplicom::startGamen();
	setupInputString( 1, IMDD_NumberInputArea,	onlyPanel.ps_RequestDisplay->sc_NumberInputArea, sc_TargetString, 0, ep_APPCharSet );
	ss_digit = 0;
	// アンダーバー以外、NULL以外はポインタを進める
	while( sc_TargetString[ss_digit] != '_' && sc_TargetString[ss_digit] != '\0' && ss_digit < IMDD_NumberInputArea ) {
		ss_digit++;
	}
	setLeftAreaEnableType( TYPD_LeftAreaSoft10Key );
	selectLeftAreaType( TYPD_LeftAreaSoft10Key );
	PJobListControl::b_JobListIndexFlag = True;
}
//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（数値入力）: 表示画面のセットアップ
//================================================================================================
void PUtilityAppliNumberInput::setupGamen(
    short option )
{
	PUtilityApplicom::setupGamen( option );
	Btx_UtilityAppliNumberInput.changeExternalText( ( uchar* )sc_TargetString, IMDD_NumberInputArea, ep_APPCharSet );
}

//================================================================================================
// 概要		:ユーティリティ・コントローラ詳細（数値入力）(2252)  : 左エリア・しおりの遷移可能不可能状態.
//================================================================================================
Bool PUtilityAppliNumberInput::isEnableNextGamenBookmark()
{
	return False;
}
//====================================================================== .
// 10872010コメント206.
//====================================================================== .
void PUtilityAppliTextInputWide::startGamen()
{
	PLBC_WideKeyboard::getInstance()->startWideKeyboard( PNLE_WideKeyboard_Normal, &Txt_UtilityAndAdminComTitle, PNLE_WideKeyboardExNone ) ;
	PUtilityAppliTextInput::startGamen();
	PLBC_WideKeyboard::getInstance()->initInputArea( &Kbd_UtilityInputKeybordConv,&c_SlideTextMgr,sc_MmbText, (char)Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MmbMaxInputNum, &Btx_InputDspAreaWide, getCharSetForPanel() ) ;	// スライド文字列管理クラスの初期化 .
	PLBC_TempChangePositionAdd::getInstance()->setChangeInfo( &Key_UtilityApplicationDecisionDo,	200, 400 );	// 確定するキー.
	PLBC_TempChangePositionAdd::getInstance()->setChangeInfo( &Key_UtilityApplicationDecisionNotDo,	392, 400 );	// 確定しないキー.
	PLBC_TempChangePositionAdd::getInstance()->changePosition() ;
}

//====================================================================== .
// 10872010コメント206.
//====================================================================== .
void PUtilityAppliTextInputWide::endGamen()
{
	PLBC_WideKeyboard::getInstance()->endWideKeyboard() ;
	PLBC_TempChangePositionAdd::getInstance()->resetPosition() ;
	PUtilityAppliTextInput::endGamen() ;
}

//====================================================================== .
// 10872010コメント206.
//====================================================================== .
void PUtilityAppliTextInputWide::procReturn( uchar option )
{
	PUtilityAppliTextInput::procReturn( option );
	PLBC_WideKeyboard::getInstance()->initInputArea_Return() ;
	setupGamen();
}

//====================================================================== .
// 10872010コメント206.
//====================================================================== .
void PUtilityAppliTextInputWide::procClear( uchar option )
{
	PUtilityAppliTextInput::procClear( option );
	PLBC_WideKeyboard::getInstance()->initInputArea_Clear() ;
	setupGamen();
}

//====================================================================== .
// 10872010コメント206.
//====================================================================== .
void PUtilityAppliTextInputWide::setDefaultBackParts()
{
	if( ( PLBD_Enl_KBD_Enlarge == Kbd_ScanKeybordConv.getChangeEnlKeybord() ) && ( True == onlyPanel.b_InitKeybord ) ) {
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_UtilityAppliTextInputDefaultWideEnl, ss_UtilityAppliTextInputDefaultWideEnl );	// 拡大用デフォルト背景.
	} else {
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_UtilityAppliTextInputDefaultWide, ss_UtilityAppliTextInputDefaultWide );	// 標準用デフォルト背景.
	}
}

// 2014/12/25 [MAv3][RQ-245] トータルカウンターの異常値対応 トータルカウンターの履歴を残す 日立 村上(幸).
//====================================================================================================================.
// 概要		:K480025000 セールスカウンター_履歴 : 画面開始処理.
// 戻り値	:なし.
// 備考		:なし.
//====================================================================================================================.
void PUtilityTotalCounterHistory::startGamen()
{
	PUtilitySalesCounterCom::startGamen();

	// セールスカウンター履歴構造体のメンバー初期化.
	memset( m_s_UtilityTotalCounterHistory.s_HistoryList, 0x00, sizeof( m_s_UtilityTotalCounterHistory.s_HistoryList ) );		// 履歴リスト.
	memset( m_s_UtilityTotalCounterHistory.sc_DateTime, 0x00, sizeof( m_s_UtilityTotalCounterHistory.sc_DateTime ) );			// 履歴リストの日時.
	m_s_UtilityTotalCounterHistory.sc_CurrentHistoryList		= -1;															// 選択されているリストのインデックス.
	m_s_UtilityTotalCounterHistory.uc_HistoryListPage			= 1;															// 履歴の現在のページ.
	m_s_UtilityTotalCounterHistory.uc_HistoryListTotalPage		= 1;															// 履歴の総ページ数.
	m_s_UtilityTotalCounterHistory.uc_HistoryListCount			= 0;															// 履歴の件数.
	
	TYPE_SystemType e_SystemType = PnlComGetMachineSetting()->getSystemType();		//現在の仕向けを取得.
	
	APIC_BackupRestore* pc_BackupRestore = APIC_BackupRestore::newInstance();
	if( NULL == pc_BackupRestore ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); 
		return;
	}
	// 履歴のトータルカウンタ情報を取得.
	TYPC_Time	c_Time;	// 履歴 リストの日時.

	for( ushort us_Num = 0; us_Num < PNLD_HistoryListMaxLen; us_Num++ ) {
		pc_BackupRestore->getTotalCounterHistory( us_Num, &m_s_UtilityTotalCounterHistory.s_HistoryList[us_Num], &c_Time );
		// 履歴のデータが無い.
		if( ( 0 == c_Time.ss_Year ) || ( 0 == c_Time.sc_Month ) || ( 0 == c_Time.sc_Day ) ) {
			break;
		}
		// 仕向けにより日時表示文字列を変更する.
		switch( e_SystemType ) {
			case TYPE_SystemType_US: 	// US仕向け.
// Comment 2016/09/08 [HCLT_SunilNP] [1002990] 【IT5_3.3】Klocwork Changes ID: 75443 [ABV.GENERAL] replacing sprintf with snprintf.
//				sprintf( &m_s_UtilityTotalCounterHistory.sc_DateTime[us_Num][0], "%02d/%02d/%04d", c_Time.sc_Month, c_Time.sc_Day, c_Time.ss_Year );
				snprintf( &m_s_UtilityTotalCounterHistory.sc_DateTime[us_Num][0], sizeof( m_s_UtilityTotalCounterHistory.sc_DateTime[us_Num] ), "%02d/%02d/%04d", c_Time.sc_Month, c_Time.sc_Day, c_Time.ss_Year );
				break;
			case TYPE_SystemType_JP: 	// 日本仕向け.
// Comment 2016/09/08 [HCLT_SunilNP] [1002990] 【IT5_3.3】Klocwork Changes ID: 75444 [ABV.GENERAL]  replacing sprintf with snprintf.
//				sprintf( &m_s_UtilityTotalCounterHistory.sc_DateTime[us_Num][0], "%04d/%02d/%02d", c_Time.ss_Year, c_Time.sc_Month, c_Time.sc_Day );
				snprintf( &m_s_UtilityTotalCounterHistory.sc_DateTime[us_Num][0], sizeof( m_s_UtilityTotalCounterHistory.sc_DateTime[us_Num] ), "%04d/%02d/%02d", c_Time.ss_Year, c_Time.sc_Month, c_Time.sc_Day );
				break;
			case TYPE_SystemType_EU:    // ＥＵ仕向け.
			default: 
// Comment 2016/09/08 [HCLT_SunilNP] [1002990] 【IT5_3.3】Klocwork Changes ID: 75445 [ABV.GENERAL]  replacing sprintf with snprintf.
//				sprintf( &m_s_UtilityTotalCounterHistory.sc_DateTime[us_Num][0], "%02d/%02d/%04d", c_Time.sc_Day, c_Time.sc_Month, c_Time.ss_Year );
				snprintf( &m_s_UtilityTotalCounterHistory.sc_DateTime[us_Num][0], sizeof( m_s_UtilityTotalCounterHistory.sc_DateTime[us_Num] ), "%02d/%02d/%04d", c_Time.sc_Day, c_Time.sc_Month, c_Time.ss_Year );
				 break;
		}
	}
	m_s_UtilityTotalCounterHistory.uc_HistoryListCount = static_cast<uchar>( us_Num );				// 履歴件数を取得.
	if( 0 < m_s_UtilityTotalCounterHistory.uc_HistoryListCount ) {
		uchar uc_page = m_s_UtilityTotalCounterHistory.uc_HistoryListCount % PNLD_HistoryListMaxAtPage;
		if( 0 == uc_page ) {
			m_s_UtilityTotalCounterHistory.uc_HistoryListTotalPage = m_s_UtilityTotalCounterHistory.uc_HistoryListCount / PNLD_HistoryListMaxAtPage;		// 総ページ数.
		}
		else {
			m_s_UtilityTotalCounterHistory.uc_HistoryListTotalPage = static_cast<uchar>( ( m_s_UtilityTotalCounterHistory.uc_HistoryListCount / PNLD_HistoryListMaxAtPage ) + 1 );// 総ページ数.
		}
	}

// comment 2015/10/30 [CQ518309] セールスカウンタ履歴のレイアウト修正 日立 村上(幸).
	ushort us_LeftArea;

// 元の画面で、セールスカウンターキーを選択している場合、選択状態を変更させない対応.
	us_LeftArea = getSelectedLeftAreaType();										// 前回の左エリアの設定値を取得.
//	setLeftAreaEnableType( TYPD_LeftAreaSalesCounter | TYPD_LeftAreaSoft10Key );
	setLeftAreaEnableType( TYPD_LeftAreaSoft10Key );
	if( us_LeftArea != getSelectedLeftAreaType()) {									// 前回と同じでなければ.
		selectLeftAreaType(us_LeftArea);											// 前回の値を設定.
	}
	
	// テンキー表示キーを上部に配置変更する.
	Key_LeftIndexSoft10Key.changePosition( 8, 8 );
}
//====================================================================================================================.
// 概要		:K480025000 セールスカウンター_履歴 : 画面セットアップ処理.
// 戻り値	:なし.
// 備考		:なし.
// comment 2015/10/30 [CQ518309] 個別印刷キーの追加対応 日立 村上(幸).
//====================================================================================================================.
void PUtilityTotalCounterHistory::setupGamen( short option )
{
	PUtilitySalesCounterCom::setupGamen( option );

	Txt_UtilityAndAdminComTitle.changeText( s_ITMUtilityTotalCounterHistory ); // タイトル(セールスカウンター履歴).

	Btx_UtilityTotalCounterHistoryPage.set( &Nfd_UtilityTotalCounterHistoryListDspPage, static_cast<float>( m_s_UtilityTotalCounterHistory.uc_HistoryListPage ), True );			// 現在のページ.
	Btx_UtilityTotalCounterHistoryPage.set( &Nfd_UtilityTotalCounterHistoryListTotalPage, static_cast<float>( m_s_UtilityTotalCounterHistory.uc_HistoryListTotalPage ), True );	// 総ページ数.

	if( 0 < m_s_UtilityTotalCounterHistory.uc_HistoryListCount ) {		// 履歴件数:1件以上.
		if( 1 == m_s_UtilityTotalCounterHistory.uc_HistoryListTotalPage ) {	// 総ページ数:1.
			Key_UtilityTotalCounterHistoryPageUp.showEnable( False );	// ↑キー 非表示.
			Key_UtilityTotalCounterHistoryPageDown.showEnable( False );	// ↓キー 非表示.
		}
		else {
			Key_UtilityTotalCounterHistoryPageUp.showEnable( True );	// ↑キー 表示.
			Key_UtilityTotalCounterHistoryPageDown.showEnable( True );	// ↓キー 表示.
		}
		Rbt_UtilityTotalCounterHistoryList.showEnable( True );			// 履歴キー 表示.

		// 現在のページの履歴件数を取得.
		uchar uc_MaxCount = 0;
		if( m_s_UtilityTotalCounterHistory.uc_HistoryListPage < m_s_UtilityTotalCounterHistory.uc_HistoryListTotalPage ) {
			// 現在のページ < 総ページ数の場合は、5件固定.
			uc_MaxCount = PNLD_HistoryListMaxAtPage;
		}
		else {
			uc_MaxCount = m_s_UtilityTotalCounterHistory.uc_HistoryListCount % PNLD_HistoryListMaxAtPage;
			if( 0 == uc_MaxCount) {
				uc_MaxCount = PNLD_HistoryListMaxAtPage;
			}
		}

		// リスト描画(値設定、選択状態変更).
		uchar	uc_staIndex = static_cast<uchar>( PNLD_HistoryListMaxAtPage * ( m_s_UtilityTotalCounterHistory.uc_HistoryListPage - 1 ) );// 開始位置のインデックス.
		uchar	uc_endIndex = static_cast<uchar>( uc_staIndex + uc_MaxCount - 1 );										// 終了位置のインデックス.
		uchar	uc_Pos = 0;																								// 描画対象位置.
		char	sc_SelectPos = -1;																						// 選択位置.

		for( uchar uc_index = uc_staIndex; uc_index < (uc_staIndex + PNLD_HistoryListMaxAtPage); uc_index++ ) {
			Key_UtilityTotalCounterHistoryList[uc_Pos].showEnable( False );
			if( uc_endIndex >= uc_index ) {	// データ有り.
				Nfd_UtilityTotalCounterHistoryListNo[uc_Pos].set( static_cast<float>(uc_index + 1), True, 2 );	// No.
				Tfd_UtilityTotalCounterHistoryListDate[uc_Pos].changeExternalText( (uchar *)&m_s_UtilityTotalCounterHistory.sc_DateTime[uc_index][0] );	// 日時.

				// 選択状態変更.
				if( m_s_UtilityTotalCounterHistory.sc_CurrentHistoryList == uc_index ) {
					// 選択.
					sc_SelectPos = (char)uc_Pos;
				}
				Key_UtilityTotalCounterHistoryList[uc_Pos].showEnable( True );
			}
			uc_Pos++;
		}
		Rbt_UtilityTotalCounterHistoryList.select( sc_SelectPos );
	}
	else {		// 履歴件数:0件.
		Rbt_UtilityTotalCounterHistoryList.showEnable( False );			// 履歴キー 非表示.
		Key_UtilityTotalCounterHistoryPageUp.showEnable( False );		// ↑キー 非表示.
		Key_UtilityTotalCounterHistoryPageDown.showEnable( False );		// ↓キー 非表示.
		Key_UtilityTotalCounterHistoryPrint.activate( False );			// 全履歴印刷キー グレーアウト.
	}

	// 履歴が選択されていなければ、詳細キー、個別印刷キーはグレーアウト.
	if( -1 != m_s_UtilityTotalCounterHistory.sc_CurrentHistoryList ) {
		Key_UtilityTotalCounterHistoryDetail.activate( True );
		Key_UtilityTotalCounterHistoryIndividualPrint.activate( True );		// 個別印刷キー.
	}
	else {
		Key_UtilityTotalCounterHistoryDetail.activate( False );
		Key_UtilityTotalCounterHistoryIndividualPrint.activate( False );	// 個別印刷キー.
	}

	// 個別印刷キー、全履歴印刷キーの表示/非表示.
	TYPE_KeyMode e_KeyMode = APIC_KeyDevice::getKeyMode();				//	キーモード.
	if( ( False == isWarningFatal() ) && ( False == PnlComGetSystemSetting()->getLongPicturePrint() ) ) {
		// PageACSESのときは、常に非表示.
		// ベンダ単独で使用している時は、非表示.
		if( ( TYPE_KeyModeVendor == e_KeyMode ) || ( TYPE_KeyModeVendorKM == e_KeyMode ) || ( TYPE_KeyModePageAcses == e_KeyMode ) ) {
			Key_UtilityTotalCounterHistoryPrint.showEnable( False );
			Key_UtilityTotalCounterHistoryIndividualPrint.showEnable( False );	// 個別印刷キー.
		}
		else if( True == onlyPanel.isWarningOn(comPanelJob, PNL_OTHER_VendorAdminInput) ) {
			// ベンダ＋キーカウンタで使用している時は、管理者番号が未入力の場合は、プリントキー非表示にし、管理者番号が入力状態である場合は、表示.
			Key_UtilityTotalCounterHistoryPrint.showEnable( False );
			Key_UtilityTotalCounterHistoryIndividualPrint.showEnable( False );	// 個別印刷キー.
		}
		else {
			// FATALが発生していない + 長尺紙ﾌﾟﾘﾝﾄﾓｰﾄﾞではない.
			Key_UtilityTotalCounterHistoryPrint.showEnable( True );
			Key_UtilityTotalCounterHistoryIndividualPrint.showEnable( True );	// 個別印刷キー.
		}
	}
	else {
		// FATALが発生時.
		Key_UtilityTotalCounterHistoryPrint.showEnable( False );
		Key_UtilityTotalCounterHistoryIndividualPrint.showEnable( False );	// 個別印刷キー.
	}
}
//====================================================================================================================.
// 概要		:K480025000 セールスカウンター_履歴 : キー押下処理.
// 戻り値	:なし.
// 備考		:なし.
//====================================================================================================================.
void PUtilityTotalCounterHistory::procButtonOn( uchar option )
{
	switch( option ) {
		case BTNID_UtilityTotalCounterHistoryList1:		// 履歴の1番目.
		case BTNID_UtilityTotalCounterHistoryList2:		// 履歴の2番目.
		case BTNID_UtilityTotalCounterHistoryList3:		// 履歴の3番目.
		case BTNID_UtilityTotalCounterHistoryList4:		// 履歴の4番目.
		case BTNID_UtilityTotalCounterHistoryList5:		// 履歴の5番目.
// comment 2015/10/30 [CQ518309] セールスカウンター履歴_レイアウト修正(7inch/9inch対応) 日立 村上(幸).
		case BTNID_UtilityTotalCounterHistoryList6:		// 履歴の6番目.
		case BTNID_UtilityTotalCounterHistoryList7:		// 履歴の7番目.
			// 選択されている履歴のインデックスを保持する.
			m_s_UtilityTotalCounterHistory.sc_CurrentHistoryList = static_cast<char>( option + ( PNLD_HistoryListMaxAtPage * ( m_s_UtilityTotalCounterHistory.uc_HistoryListPage - 1 ) ) - 1 );
			break;
		case BTNID_UtilityTotalCounterHistoryListPageUp:	// ↑キー.
			if( 1 >= m_s_UtilityTotalCounterHistory.uc_HistoryListPage ) {
				m_s_UtilityTotalCounterHistory.uc_HistoryListPage = m_s_UtilityTotalCounterHistory.uc_HistoryListTotalPage;
			}
			else {
				m_s_UtilityTotalCounterHistory.uc_HistoryListPage--;
			}
			break;
		case BTNID_UtilityTotalCounterHistoryListPageDown:	// ↓キー.
			if( m_s_UtilityTotalCounterHistory.uc_HistoryListTotalPage <= m_s_UtilityTotalCounterHistory.uc_HistoryListPage ) {
				m_s_UtilityTotalCounterHistory.uc_HistoryListPage = 1;
			}
			else {
				m_s_UtilityTotalCounterHistory.uc_HistoryListPage++;
			}
			break;
		default:
			break;
	}

	setupGamen();
}
//====================================================================================================================.
// 概要		:K480025000 セールスカウンター_履歴 : 画面遷移処理.
// 戻り値	:なし.
// 備考		:なし.
// comment 2015/10/30 [CQ518309] 個別印刷キーの追加対応 日立 村上(幸).
//====================================================================================================================.
void PUtilityTotalCounterHistory::procGoToPage( uchar option )
{
	switch( option ) {
		case BTNID_UtilityTotalCounterHistoryListDetail:	// 詳細キー.
//			nextGamen = &gp_UtilityTotalCopy;				// 10801000_Mi セールスカウンター_コピー.
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items starts
 	if ( IsSFP() ) {
			nextGamen = &gp_UtilityTotalPrinterHistory;						
	} else {
			nextGamen = &gp_UtilityTotalCopyHistory;						// K480025200_セールスカウンター_履歴_コピー.
	}
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items ends
			break;
		case BTNID_UtilityTotalCounterHistoryListIndividualPrint:			// 個別印刷キー.
			gp_UtilityTotalCounterPrint.setSelectAllHistory( False );		// 個別印刷キーから画面遷移したことを設定.
			nextGamen = &gp_UtilityTotalCounterPrint;						// 10804020_セールスカウンタープリント.
			break;
		case BTNID_UtilityTotalCounterHistoryListPrint:						// 全履歴印刷キー.
//			gp_UtilityTotalCounterPrint.setFromHistoryList( True );			// 履歴印刷から画面遷移したことを設定.
			gp_UtilityTotalCounterPrint.setSelectAllHistory( True );		// 全履歴印刷キーから画面遷移したことを設定.
			nextGamen = &gp_UtilityTotalCounterPrint;						// 10804020_セールスカウンタープリント.
			break;
		default:
			break;
	}
}
//====================================================================================================================.
// 概要		:K480025000 セールスカウンター_履歴 : 閉じるキー押下処理.
// 戻り値	:なし.
// 備考		:なし.
//====================================================================================================================.
void PUtilityTotalCounterHistory::procOk( uchar option )
{
	onlyPanel.m_bl_MagicSequenceTotalCounterHistory = False;
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items starts
 	if ( IsSFP() ) {
		nextGamen = &gp_UtilityTotalPrinter;	// 10801000_Mi セールスカウンター_コピー.
    } else {
		nextGamen = &gp_UtilityTotalCopy;	// 10801000_Mi セールスカウンター_コピー.
    }
//2018/04/18 HCLT_Monisha for SparrowSFP [制御構成変更_No.206] removing copy, scan fax related items ends
}
//====================================================================================================================.
// 概要		:K480025000 セールスカウンター_履歴 : 画面終了処理.
// 戻り値	:なし.
// 備考		:なし.
//====================================================================================================================.
void PUtilityTotalCounterHistory::endGamen()
{
	// テンキー表示キーを下部に戻す.
	Key_LeftIndexSoft10Key.changePosition( 8, 88 );
// comment 2015/9/11 [CQ517594] 操作しても音声が鳴らない 村上(幸).
	PBasicMessageProc::endGamen();
}
//====================================================================================================================.
// 概要		:K480025000 セールスカウンター_履歴 : セールスカウンター履歴の取得.
// 戻り値	:選択されているリストのインデックス.
// 備考		:なし.
//====================================================================================================================.
char PUtilityTotalCounterHistory::getTotalCounterHistory( TYPS_BackupRestore_TotalCounter* ps_HistoryList, char* psc_Time )
{
	*ps_HistoryList = m_s_UtilityTotalCounterHistory.s_HistoryList[(uchar)m_s_UtilityTotalCounterHistory.sc_CurrentHistoryList];
	strncpy( psc_Time, (char*)&m_s_UtilityTotalCounterHistory.sc_DateTime[(uchar)m_s_UtilityTotalCounterHistory.sc_CurrentHistoryList][0], PNLD_HistoryListDateTimeMaxLen-1 );
	return m_s_UtilityTotalCounterHistory.sc_CurrentHistoryList;
}
