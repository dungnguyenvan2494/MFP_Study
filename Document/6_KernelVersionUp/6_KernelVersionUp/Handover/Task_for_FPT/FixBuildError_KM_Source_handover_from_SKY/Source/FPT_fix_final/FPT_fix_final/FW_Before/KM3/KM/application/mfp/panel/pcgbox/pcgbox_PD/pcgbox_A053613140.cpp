//クラス-----------------------------------------------------------------------------------------------------------------.
// ファイル : CHOP_A053613140_サーチャブルPDF.
//-----------------------------------------------------------------------------------------------------------------------.
#define	SIZE( table )	( sizeof( table ) / sizeof( PDisplayParts* ) )
#include <panel/plibh/plibh_CLCD2/PNLC_StackManJobSpec.h>
#include <panel/plibh/plibh_CLCD2/PNLC_BoxDataKeyControlLib.h>
#include <panel/plibh/plibhg/pdpcomb.h>
#include <panel/ppcg/ppcg_PD/ppcg_PD.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070500.h>
#include <panel/pcgbox/pcgbox_CLCD2/pigbox_base.h>
#include <panel/pcgbox/pcgbox_PD/pcgbox_A053613140.h>
#ifdef DEF_LCD_7INCH
#include "PD/A053613140_7inch.cpp"
#else
#include "PD/A053613140.cpp"
#endif // DEF_LCD_7INCH.
//PanelDesigner対応------------------------------------------------------------------------------------------------------.
// 画面開始処理.
void PA053613140::startGamen()
{
	bl_PanelDesigner = True;																							// PanelDesigner対応.
	PCGC_ScanFaxReadSetDetailComLib::startGamen();																		// 親クラス呼出し(これを行わないとPopupWindowが表示しない).
// コールバック関数の登録.
	ATTACH_OBSERVER( PA053613140, gp_A053613140, PA053613140::prSearchablePdf, A053613140_Rbt1001, Bool, short );		//	サーチャブルPDF・ON/OFF.
	ATTACH_OBSERVER( PA053613140, gp_A053613140, PA053613140::prLanguage, A053613140_Rbt1002, Bool, short );			//	言語選択.
	ATTACH_OBSERVER( PA053613140, gp_A053613140, PA053613140::prRotationCorrection, A053613140_Btn0008, Bool, Bool );	//	回転補正.
	ATTACH_OBSERVER( PA053613140, gp_A053613140, PA053613140::prFileNameAutoSampling, A053613140_Btn0010, Bool, Bool );	//	文書名自動抽出.
	ATTACH_OBSERVER( PA053613140, gp_A053613140, PA053613140::prCancel, A053613140_Btn0004, Bool, Bool );				//	キャンセル.
	setOkButtonForFunctionBadge( &A053613140_Btn0006 ); // 機能バッチボタン押下時に代替え処理を行うＯＫキーを登録.
// 変数等の初期化処理を記載する.
	m_bl_Backup = False;
	APIC_Job* pc_Job = PnlComGetJobScanFax();
	if( NULL == pc_Job ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if( NULL == pc_JobSpec ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_Send* pc_Send = pc_JobSpec->pc_Send;
	if( NULL == pc_Send ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }																								// 画面表示中にOnからOffへの切替えが行われた事を記録しておく.
	TYPC_SearchablePdf c_SearchablePdf = pc_Send->getSearchablePdf();
	if( False == combinationCheckSearchablePDF( CNK_SearchablePDF_RotationCorrection, pc_Job, True, False ) ) {
		c_SearchablePdf.setRotationCorrection( False );																	// 回転組禁の場合は落とす.
		pc_Send->setSearchablePdf( c_SearchablePdf );
	}
	TYPE_LanguageOcr e_LanguageOcr[ TYPD_LanguageListNum ];
	for( ushort i = 0; i < TYPD_LanguageListNum; i++ ) {
		e_LanguageOcr[ i ] = TYPE_LanguageOcrNone;																		// 選択可能言語の取得領域の初期化.
	}
	PnlComGetSearchablePDFSetting()->getDownloadCompLangList( e_LanguageOcr );											// 選択可能言語の取得.
	for( i = 0; i < TYPD_LanguageListNum; i++ ) {
		m_pst_LangTbl[ i ] = NULL;																						// 画面側領域テーブルの初期化.
	}
	static COnOffButtonObjectAdd* pLanguageButton[] = { &A053613140_Btn0019, &A053613140_Btn0021, &A053613140_Btn0023, &A053613140_Btn0025, &A053613140_Btn0027, &A053613140_Btn0029, &A053613140_Btn0031, &A053613140_Btn0033, &A053613140_Btn0035, };
	for( i = 0; i < TYPD_LanguageListNum; i++ ) {
		pLanguageButton[ i ]->showEnable( False );
		if( e_LanguageOcr[ i ] == TYPE_LanguageOcrNone ) {
			continue;
		}
		for( ushort j = 0; j < us_SearchableLanguageIdSize; j++ ) {
			if( st_SearchableLanguageId[ j ].e_Language == e_LanguageOcr[ i ] ) {
				m_pst_LangTbl[ i ] = const_cast<struct PNLS_SearchableLanguage*>( &st_SearchableLanguageId[ j ] );
				pLanguageButton[ i ]->showEnable( True );
				pLanguageButton[ i ]->changeText( m_pst_LangTbl[ i ]->us_TextId );
			}
		}
	}
// ここに構成による表示条件を記載する.
	Bool bl_ShowEnable = True;
	if( True == PnlComGetScanJobSetting()->getSearchablePDFCreateFileName() && !( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBit( 16, 3 ) && TYPE_ImageLogSendType2 == PnlComGetSystemSetting()->getImageLogSendEnabled() ) ) {	//!< 管理者設定 サーチャブルPDF 文書名自動抽出有効かつ.// またはソフトウェアSWの状態がType2有効かつ.// 画像ログ転送設定「する」設定でない時使用可能.
		if( True == ScanOapExtSet.isActiveGeneralApplication() && False == ScanOapExtSet.isEnableSearchablePdfAutoExtrFileName() ) {	// [OAP]MFP本体設定が有効な場合に限り、アプリの設定を参照する.// スキャン連携アプリが動作中.// サーチャブルPDFファイル名自動生成機能OFF.
			bl_ShowEnable = False;
		}
	} else {
		bl_ShowEnable = False;
	}
	A053613140_Btn0010.showEnable( bl_ShowEnable );																		// 文書名自動抽出.
// ここに画面の初期化処理を記載する.
	A053613140_Rbt1001.setN( ( True == c_SearchablePdf.getOffOn() ) ? 0 : 1, True );									// サーチャブルPDF・ON/OFF.
	A053613140_Cmp1000.setNowPopupGamen( PNLE_BoxSearcherBullPDF );
	setupGamenAtButtonOn();																								//	ボタンＯＮ時の表示部品のセットアップ.
}
//	ボタンＯＮ時の表示部品のセットアップ.
void PA053613140::setupGamenAtButtonOn()
{
	APIC_Job* pc_Job = PnlComGetJobScanFax();
	if( NULL == pc_Job ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if( NULL == pc_JobSpec ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_Send* pc_Send = pc_JobSpec->pc_Send;
	if( NULL == pc_Send ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
// comment 2016/11/04 Sky川嵜 [IT5-51_機種マージ] OCR速度改善と修正済不具合のマージ対応要望【丸紅様対応】.
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	if( NULL == pc_SoftDipSw ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_SearchablePdf c_SearchablePdf = pc_Send->getSearchablePdf();
// comment 2016/11/04 Sky川嵜 [IT5-51_機種マージ] OCR速度改善と修正済不具合のマージ対応要望【丸紅様対応】.
	if(True == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW229, 1 )){
#ifdef DEF_LCD_7INCH
		A053613140_Btn0008.changeText(s_pKEYAllPageAdjustRotation_W160H040A);											// 7inch版回転補正キーの文言を全回転補正に変更.
#else // #ifdef DEF_LCD_7INCH.
		A053613140_Btn0008.changeText(s_pKEYAllPageAdjustRotation_W132H036A);											// 9inch版回転補正キーの文言を全回転補正に変更.
#endif // #ifdef DEF_LCD_7INCH.
	} else {
#ifdef DEF_LCD_7INCH
		A053613140_Btn0008.changeText(s_pKEYAdjustRotation_W160H040A);													// 7inch版回転補正キーの文言を回転補正に変更.
#else // #ifdef DEF_LCD_7INCH.
		A053613140_Btn0008.changeText(s_pKEYAdjustRotation_W132H036A);													// 9inch版回転補正キーの文言を回転補正に変更.
#endif // #ifdef DEF_LCD_7INCH.
	}
	A053613140_Btn0008.setN( c_SearchablePdf.getRotationCorrection() );													// 回転補正.
	A053613140_Btn0010.setN( c_SearchablePdf.getAutoExtractionValueSetting() );											// 文書名自動抽出.
	A053613140_Rbt1002.setN( -1 );																						// 言語選択.
	for( short i = 0; i < TYPD_LanguageListNum; i++ ) {																	//	言語キーの表示.
		if( NULL == m_pst_LangTbl[ i ] ) {
			continue;
		}
		if( m_pst_LangTbl[ i ]->e_Language == c_SearchablePdf.getLanguage() ) {
			A053613140_Rbt1002.setN( i );																				// 言語選択.
			break;
		}
	}
}
//	サーチャブルPDF・ON/OFF.
Bool PA053613140::prSearchablePdf( short n )
{
	APIC_Job* pc_Job = PnlComGetJobScanFax();
	if( NULL == pc_Job ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return False; }
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if( NULL == pc_JobSpec ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return False; }
	TYPC_Send* pc_Send = pc_JobSpec->pc_Send;
	if( NULL == pc_Send ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return False; }
// comment 2016/11/04 Sky川嵜 [IT5-51_機種マージ] OCR速度改善と修正済不具合のマージ対応要望【丸紅様対応】.
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	if( NULL == pc_SoftDipSw ){ LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return False; }
	TYPC_SearchablePdf c_SearchablePdf = pc_Send->getSearchablePdf();
	if( n == 0 ) {
		if( False == combinationCheck( CNK_SearchablePDF, pc_Job, False, True ) || False == combinationCheckSearchablePDF( CNK_SearchablePDF, pc_Job, False, True ) ) {	// 組合せ禁止チェックで先優先があればNGで返す.
			return	False;
		}
		combinationCheck( CNK_SearchablePDF, pc_Job, True, False );										// 上記では後優先機能の解除をしない設定で組合せ禁止チェックを行っているため、再度コールし後優先機能の解除を行う.
		combinationCheckSearchablePDF( CNK_SearchablePDF, pc_Job, True, False );
		if( False == m_bl_Backup ) {																					// 本画面表示後に、初めて設定をOnにする場合（バックアップしているサーチャブルPDF設定が無い場合）.
			TYPE_Language e_Language = PnlComGetLanguage();																// 表示言語を取得.
			if( ( True == PnlComGetOperationSetting()->getLanguageChoiceSetting() ) && ( False == PnlComIsMyPanel() ) ) {					// 言語一時変更が有効な場合は、一時変更の言語を優先.
				e_Language = onlyPanel.e_Language;
			}
			TYPE_LanguageOcr e_LanguageOcr = TYPE_LanguageOcrNone;														// 設定するOcr言語を初期化.
			for( short i = 0; TYPD_LanguageListNum > i; ++i ) {
				if( NULL == m_pst_LangTbl[ i ] ) {
					continue;
				}
				if( e_Language == m_pst_LangTbl[ i ]->e_MachineLanguage ) {												// 表示言語と一致するOcr言語を設定する.
					e_LanguageOcr = m_pst_LangTbl[ i ]->e_Language;
					break;
				}
			}
			Bool bl_RotationCorrection = True;
			if( False == combinationCheckSearchablePDF( CNK_SearchablePDF_RotationCorrection, pc_Job, True, False ) ) {		// 回転補正ができない場合はOffにする.
				bl_RotationCorrection = False;
			}
// comment 2016/11/04 Sky川嵜 [IT5-51_機種マージ] OCR速度改善と修正済不具合のマージ対応要望【丸紅様対応】.
			if (True == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW230, 4 )) {
//DipSW ONでOFF⇒ONの場合（初回時のみ）は、回転補正はデフォルトOFFにする.
				bl_RotationCorrection = False;																			// 初期回転補正をOFFに設定.
			}
			c_SearchablePdf.setOffOn( True );																			// サーチャブルPDFを作成するか否かを設定.
			c_SearchablePdf.setLanguage( e_LanguageOcr );																// 言語選択設定.
			c_SearchablePdf.setRotationCorrection( bl_RotationCorrection );												// 回転補正をするか否かを設定.
			c_SearchablePdf.setAutoExtractionValueSetting( False );														// ファイル名自動抽出設定値設定.
		} else {																										// 本画面表示後に、設定On/Offしてから再度設定Onにする場合（バックアップしているサーチャブルPDF設定が有る場合）.
			c_SearchablePdf = m_c_SearchablePdf;																		// 設定On時のサーチャブルPDF設定データを設定.
		}
		pc_Send->setSearchablePdf( c_SearchablePdf );								// サーチャブルPDF作成情報設定.
	} else {
		m_bl_Backup = True;																								// 画面表示中にOnからOffへの切替えが行われた事を記録しておく.
		m_c_SearchablePdf = c_SearchablePdf;																			// 設定On時のサーチャブルPDF設定データを保存.
		cancel( CNK_SearchablePDF, CNK_Size, pc_Job );													// サーチャブルPDF設定をOff.
	}
	return	True;
}
//	言語選択.
Bool PA053613140::prLanguage( short n )
{
	APIC_Job* pc_Job = PnlComGetJobScanFax();
	if( NULL == pc_Job ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return False; }
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if( NULL == pc_JobSpec ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return False; }
	TYPC_Send* pc_Send = pc_JobSpec->pc_Send;
	if( NULL == pc_Send ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return False; }
	TYPC_SearchablePdf c_SearchablePdf = pc_Send->getSearchablePdf();
	c_SearchablePdf.setLanguage( m_pst_LangTbl[ n ]->e_Language );
	pc_Send->setSearchablePdf( c_SearchablePdf );
	return	True;
}
//	回転補正.
Bool PA053613140::prRotationCorrection( Bool on )
{
	APIC_Job* pc_Job = PnlComGetJobScanFax();
	if( NULL == pc_Job ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return False; }
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if( NULL == pc_JobSpec ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return False; }
	TYPC_Send* pc_Send = pc_JobSpec->pc_Send;
	if( NULL == pc_Send ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return False; }
	if( on == True && False == combinationCheckSearchablePDF( CNK_SearchablePDF_RotationCorrection, pc_Job ) ) {
		return	False;
	}
	TYPC_SearchablePdf c_SearchablePdf = pc_Send->getSearchablePdf();
	c_SearchablePdf.setRotationCorrection( on );
	pc_Send->setSearchablePdf( c_SearchablePdf );
	return	True;
}
//	文書名自動抽出.
Bool PA053613140::prFileNameAutoSampling( Bool on )
{
	APIC_Job* pc_Job = PnlComGetJobScanFax();
	if( NULL == pc_Job ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return False; }
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if( NULL == pc_JobSpec ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return False; }
	TYPC_Send* pc_Send = pc_JobSpec->pc_Send;
	if( NULL == pc_Send ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return False; }
	TYPC_SearchablePdf c_SearchablePdf = pc_Send->getSearchablePdf();
	c_SearchablePdf.setAutoExtractionValueSetting( on );
	pc_Send->setSearchablePdf( c_SearchablePdf );
	return	True;
}
//	キャンセル.
Bool PA053613140::prCancel( Bool on )
{
	PNLC_StackManJobSpec::newInstance()->rollbackJobSpec();																// 設定を元に戻す.
	return	True;
}
//従来処理(画面外処理)---------------------------------------------------------------------------------------------------.
//	表示部品のセットアップ.
void PA053613140::setupGamen( short option )
{
	PCGC_ScanFaxReadSetDetailComLib::setupGamen( option );
	if( ScanOapExtSet.isActiveGeneralApplication() ) {
		printLEDControl( False );																						// スタートキー無効.
	}
	if( isNowGamen( &gp_DocKeepUSBMemoryExport_box ) ) {
		PMessageArea::setupPrintLEDBoxToUSB();																			// BoxToUSB用PrintLEDのセットアップ.
	}
// comment 2022/08/02 HungDM17-FPT [OP_BTS-38936] 詳細設定画面に「スタート」キーが青になる問題対策。
// 	else{
// 		PMessageArea::setupPrintLEDFax();																				// 宛先件数0件の場合は、LEDを赤にする.
// 	}
}
//	スタート.
void PA053613140::procStart( uchar option )
{
	if( ScanOapExtSet.isActiveGeneralApplication() ) {																	// OAP.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	onlyPanel.checkWarningAdminMachineForBox( PnlComGetJobScan( PCGE_JobPutJBLBox ) );									// 統合管理装置装着時警告制御処理.
	if( True == printStartDisable() ) {																					//	スタートキーアンバーの時は受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	if( &gp_BoxDocKeepUSBMemory_box == PBasicMessageProc::getNowGamen() ) {												// 外部メモリへの保存時.
		PNLC_BoxDataKeyControlLib::newInstance()->procStartBoxDocKeepUSBMemory();										// 外部メモリへの保存スタート.
	} else if( &gp_DocKeepUSBMemoryExport_box == PBasicMessageProc::getNowGamen() ) {									// ボックスから外部メモリへ.
		if( True == onlyPanel.isWarningOn( PnlComGetJobScan( PCGE_JobPutJBLBox ), PNL_ERROR_NoFrequency_Box ) || True == onlyPanel.isWarningOn( PnlComGetJobScan( PCGE_JobPutJBLBox ), PNL_ERROR_CardUnset_Box ) || True == onlyPanel.isWarningOn( PnlComGetJobScan( PCGE_JobPutJBLBox ), PNL_ERROR_AdminMachinePowerSupply_Box ) || True == onlyPanel.isWarningOn( PnlComGetJobScan( PCGE_JobPutJBLBox ), PNL_ERROR_AdminMachineRestrictedFunction_Box) ) {		// 管理装置度数なしチェック.// 管理カード未挿入チェック.// 管理装置電源確認警告チェック.// 機能毎使用設定：使用禁止チェック.
			return;																										//統合管理装置の警告が発生している場合は、関数終了.
		}
		gp_USBMemoryExport_actExport.startWait();
		if( False == printStartDisable() ) {																			// 3項演算子にするとエラーになる.
			PNLC_PopupListControl::newInstance()->addPopup( &gp_USBMemoryExport_actExport, this, NULL, 360, 216, 80, 48 );
		} else {
			setBuzzerType( PCGE_BuzzerType_InputNg );
		}
	} else if( &gp_DocKeep_box == PBasicMessageProc::getNowGamen() ) {													// ボックスへ保存.
		PNLC_BoxDataKeyControlLib::newInstance()->procStartBoxDocKeep();												// 文書保存.
	} else {
		PNLC_BoxDataKeyControlLib::newInstance()->procStartDocNetSendNetSend( option );									// 送信スタート.
	}
}
//	ファクス.
void PA053613140::procFax( uchar option )
{
	if( ScanOapExtSet.isActiveGeneralApplication() ) {																	// OAP.
		JBT_InitOapBoxUtil( JBLE_OAP_SCANFAX );
		procAppKey( option, JBLE_OAP_SCANFAX );
	} else {																											// localUI.
		PUsercom::procFax( option );
	}
}
//	ファクス.
void PA053613140::procFaxOnly( uchar option )
{
	if( ScanOapExtSet.isActiveGeneralApplication() ) {																	// OAP.
		JBT_InitOapBoxUtil( JBLE_OAP_FAXONLY );
		procAppKey( option, JBLE_OAP_FAXONLY );
	} else {																											// localUI.
		PUsercom::procFaxOnly( option );
	}
}
//	ボックス.
void PA053613140::procBox( uchar option )
{
	if( ScanOapExtSet.isActiveGeneralApplication() ) {																	// OAP.
		JBT_InitOapBoxUtil( JBLE_OAP_BOX );
		procAppKey( option, JBLE_OAP_BOX );
	} else {																											// localUI.
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}
//	ユビキタスプリント.
void PA053613140::procUbqPrint( uchar option )
{
	if( ScanOapExtSet.isActiveGeneralApplication() ) {																	// OAP.
		JBT_InitOapBoxUtil( JBLE_OAP_UBQPRINT );
		procAppKey( option, JBLE_OAP_UBQPRINT );
	} else {																											// localUI.
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}
//	コピー.
void PA053613140::procCopy( uchar option )
{
	if( ScanOapExtSet.isActiveGeneralApplication() ) {																	// OAP.
		JBT_InitOapBoxUtil( JBLE_OAP_COPY );
		procAppKey( option, JBLE_OAP_COPY );
	} else {																											// localUI.
		PCGC_ScanFaxReadSetDetailComLib::procCopy( option );
	}
}
//	ブラウザ.
void PA053613140::procBrowser( uchar option )
{
	if( ScanOapExtSet.isActiveGeneralApplication() ) {																	// OAP.
		JBT_InitOapBoxUtil( JBLE_OAP_BROWSER );
		procAppKey( option, JBLE_OAP_BROWSER );
	} else {																											// localUI.
		PCGC_ScanFaxReadSetDetailComLib::procBrowser( option );
	}
}
//画面固有処理-----------------------------------------------------------------------------------------------------------.
