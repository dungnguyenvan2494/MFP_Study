//クラス-----------------------------------------------------------------------------------------------------------------.
// ファイル : CHOP_A053613200_文字認識設定.
// Version  : PD ver.012.
//-----------------------------------------------------------------------------------------------------------------------.
#define	SIZE( table )	( sizeof( table ) / sizeof( PDisplayParts* ) )
#include <panel/plibh/plibh_CLCD2/PNLC_StackManJobSpec.h>
#include <panel/plibh/plibhg/pdpcomb.h>
#include <panel/ppcg/ppcg_PD/ppcg_PD.h>
#include <panel/plibh/plibh_CLCD2/PNLC_BoxDataKeyControlLib.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070500.h>
#include <panel/pcgbox/pcgbox_CLCD2/pigbox_base.h>
#include <panel/pcgbox/pcgbox_PD/pcgbox_A053613200.h>
#ifdef DEF_LCD_7INCH
#include "PD/A053613200_7inch.cpp"
#else
#include "PD/A053613200.cpp"
#endif // DEF_LCD_7INCH.
//PanelDesigner対応------------------------------------------------------------------------------------------------------.
// 画面開始処理.
void PA053613200::startGamen()
{
	bl_PanelDesigner = True;																								// PanelDesigner対応.
	PCGC_ScanFaxReadSetDetailComLib::startGamen();																			// 親クラス呼出し(これを行わないとPopupWindowが表示しない).
// コールバック関数の登録.
	ATTACH_OBSERVER( PA053613200, gp_A053613200, PA053613200::prOnOff, A053613200_Rbt1001, Bool, short );					// ON/OFF.
	ATTACH_OBSERVER( PA053613200, gp_A053613200, PA053613200::prLanguage, A053613200_Rbt1002, Bool, short );				// 言語選択.
	ATTACH_OBSERVER( PA053613200, gp_A053613200, PA053613200::prOutputKindDOCX, A053613200_Rbt1003, Bool, short );			// 出力形式(DOCX).
	ATTACH_OBSERVER( PA053613200, gp_A053613200, PA053613200::prOutputKindXLSX, A053613200_Rbt1004, Bool, short );			// 出力形式(XLSX).
	ATTACH_OBSERVER( PA053613200, gp_A053613200, PA053613200::prRotationCorrect, A053613200_Btn0009, Bool, Bool );			// 回転補正.
	ATTACH_OBSERVER( PA053613200, gp_A053613200, PA053613200::prOk, A053613200_Btn0007, Bool, Bool );						// OK.
	ATTACH_OBSERVER( PA053613200, gp_A053613200, PA053613200::prCancel, A053613200_Btn0005, Bool, Bool );					// キャンセル.
	setOkButtonForFunctionBadge( &A053613200_Btn0007 ); // 機能バッチボタン押下時に代替え処理を行うＯＫキーを登録.
// 変数等の初期化処理を記載する.
// comment 2016/08/29 【HCLT_Tharun】【Accurev ID:1002990】Kloc work Changes ID:68779 [NPD.FUNC.MUST] NULL check modified.
//	TYPC_OfficeOpenXml c_OfficeOpenXml = PnlComGetJobFax()->getJobSpec()->pc_Send->getOfficeOpenXml();
	APIC_Job* pc_Job = PnlComGetJobFax();
	if ( NULL == pc_Job ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if ( NULL == pc_JobSpec ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	TYPC_Send* pc_Send = pc_JobSpec->pc_Send;
	if ( NULL == pc_Send ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	TYPC_OfficeOpenXml c_OfficeOpenXml = pc_Send->getOfficeOpenXml();
	TYPE_LanguageOcr e_LanguageOcr[ TYPD_LanguageListNum ];																	//	選択可能言語の取得領域の初期化.
	for( short i = 0; i < TYPD_LanguageListNum; i++ ) {
		e_LanguageOcr[ i ] = TYPE_LanguageOcrNone;
	}
	PnlComGetSearchablePDFSetting()->getDownloadCompLangList( e_LanguageOcr );												//	選択可能言語の取得.
	for( short i = 0; i < TYPD_LanguageListNum; i++ ) {																			//	画面側領域テーブルの初期化.
		m_pst_LangTbl[ i ] = NULL;
	}
	for( short i = 0; i < TYPD_LanguageListNum; i++ ) {
		if ( e_LanguageOcr[ i ] == TYPE_LanguageOcrNone ) {
			continue;
		}
		for( short j = 0; j < us_SearchableLanguageIdSize; j++ ) {
			if( st_SearchableLanguageId[ j ].e_Language == e_LanguageOcr[ i ] ) {
				m_pst_LangTbl[ i ] = const_cast<struct PNLS_SearchableLanguage*>( &st_SearchableLanguageId[ j ] );
			}
		}
	}
	m_bl_Backup = False;
// ここに構成による表示条件を記載する.
	static COnOffButtonObjectAdd* const tbl_LanguageButton[] = { &A053613200_Btn0017, &A053613200_Btn0019, &A053613200_Btn0021, &A053613200_Btn0023, &A053613200_Btn0025, &A053613200_Btn0027, &A053613200_Btn0029, &A053613200_Btn0031, &A053613200_Btn0033, };
	for( short i = 0; i < TYPD_LanguageListNum; i++ ) {																			//	言語キーの表示/非表示.
		if ( NULL == m_pst_LangTbl[ i ] ) {
			tbl_LanguageButton[ i ]->showEnable( False );
		} else {
			tbl_LanguageButton[ i ]->showEnable( True );
			tbl_LanguageButton[ i ]->changeText( m_pst_LangTbl[ i ]->us_TextId );
		}
	}
//comment2021/04/16 Sky奥田[IT6_2.2][OP_BTS-30704]PPTX選択時出力形式を選択できるように修正.
	A053613200_Rbt1003.showEnable( False );																					// 出力形式(DOCX,PPTX).
	A053613200_Rbt1004.showEnable( False );																					// 出力形式(XLSX).
	A053613200_Gra0004.showEnable( True );	A053613200_Txt0045.showEnable( True );
// comment 2016/08/29 【HCLT_Tharun】【Accurev ID:1002990】Kloc work Changes ID:68780 [NPD.FUNC.MUST] NULL check modified.
//	switch( PnlComGetJobFax()->getJobSpec()->pc_Send->c_ComSend.getFileFormat() ) {
	switch( pc_Send->c_ComSend.getFileFormat() ) {
		case TYPE_FileFormatDOCX:	A053613200_Rbt1003.showEnable( True );	break;											// 出力形式(DOCX).
		case TYPE_FileFormatXLSX:	A053613200_Rbt1004.showEnable( True );	break;											// 出力形式(XLSX).
//comment2021/04/16 Sky奥田[IT6_2.2][OP_BTS-30704]PPTX選択時出力形式を選択できるように修正.
		case TYPE_FileFormatOfficeOpenXML:																					// 出力形式(PPTX).
			A053613200_Rbt1003.showEnable( True );
			A053613200_Btn0035.showEnable( False );
			break;
		default:					A053613200_Gra0004.showEnable( False );	A053613200_Txt0045.showEnable( False );	break;
	}
// ここに画面の初期化処理を記載する.
	A053613200_Rbt1001.setN( ( True == c_OfficeOpenXml.getOcrOn() ) ? 0 : 1, True );
	A053613200_Cmp1000.setNowPopupGamen( PNLE_BoxCharacterRecognition );
	setupGamenAtButtonOn();																									// ボタンＯＮ時の表示部品のセットアップ.
}
// ボタンＯＮ時の表示部品のセットアップ.
void PA053613200::setupGamenAtButtonOn()
{
// comment 2016/08/29 【HCLT_Tharun】【Accurev ID:1002990】Kloc work Changes ID:68781 [NPD.FUNC.MUST] NULL check modified.
//	TYPC_OfficeOpenXml c_OfficeOpenXml = PnlComGetJobFax()->getJobSpec()->pc_Send->getOfficeOpenXml();
	APIC_Job* pc_Job = PnlComGetJobFax();
	if ( NULL == pc_Job ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if ( NULL == pc_JobSpec ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	TYPC_Send* pc_Send = pc_JobSpec->pc_Send;
	if ( NULL == pc_Send ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	TYPC_OfficeOpenXml c_OfficeOpenXml = pc_Send->getOfficeOpenXml();
	A053613200_Rbt1002.setN( -1 );																							// 言語選択.
	for( short i = 0; i < TYPD_LanguageListNum; i++ ) {
		if( NULL == m_pst_LangTbl[ i ] ) {
			continue;
		}
		if( m_pst_LangTbl[ i ]->e_Language == c_OfficeOpenXml.getLanguage() ) {
			A053613200_Rbt1002.setN( i );																					// 言語選択.
			break;
		}
	}
	switch( c_OfficeOpenXml.getOutPutFormat() ) {																			// 出力形式の選択表示.
		case TYPE_OutPutFormat_Text_Priority:	A053613200_Rbt1003.setN( 0 );	A053613200_Rbt1004.setN( -1 );				//	文字優先.
//comment2021/04/16 Sky奥田[IT6_2.2][OP_BTS-30704]PPTX選択時出力形式を選択できるように修正.
			if(TYPE_FileFormatOfficeOpenXML == pc_Send->c_ComSend.getFileFormat()){
				A053613200_Rbt1003.setN( -1 );
			}
			break;
		case TYPE_OutPutFormat_Image_Priority:	A053613200_Rbt1003.setN( 1 );	A053613200_Rbt1004.setN( -1 );	break;		//	画像優先.
		case TYPE_OutPutFormat_Image_Text:		A053613200_Rbt1003.setN( 2 );	A053613200_Rbt1004.setN( 0 );	break;		//	画像と文字.
		case TYPE_OutPutFormat_Text:			A053613200_Rbt1003.setN( -1 );	A053613200_Rbt1004.setN( 1 );	break;		//	文字のみ.
	}
	A053613200_Btn0009.setN( c_OfficeOpenXml.getRotationCorrection() );														//	補正する・しないキー.
}
// ON/OFF.
Bool PA053613200::prOnOff( short n )
{
// comment 2016/08/29 【HCLT_Tharun】【Accurev ID:1002990】Kloc work Changes ID:68782 [NPD.FUNC.MUST] NULL check modified.
//	TYPC_OfficeOpenXml c_OfficeOpenXml = PnlComGetJobFax()->getJobSpec()->pc_Send->getOfficeOpenXml();
//	if( n == 0 ) {
//		if( False == setOfficeOpenXmlOn( PnlComGetJobFax(), m_pst_LangTbl, &m_c_OfficeOpenXml, m_bl_Backup ) ) {			// OCR文字認識の設定をOnにする.
	APIC_Job* pc_Job = PnlComGetJobFax();
	if ( NULL == pc_Job ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if ( NULL == pc_JobSpec ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	TYPC_Send* pc_Send = pc_JobSpec->pc_Send;
	if ( NULL == pc_Send ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	TYPC_OfficeOpenXml c_OfficeOpenXml = pc_Send->getOfficeOpenXml();
	if ( 0 == n ) {
		if ( False == setOfficeOpenXmlOn( pc_Job, m_pst_LangTbl, &m_c_OfficeOpenXml, m_bl_Backup ) ) {						// OCR文字認識の設定をOnにする.
			return	False;
		}
	} else {
		m_bl_Backup = True;																									// 画面表示中にOnからOffへの切替えが行われた事を記録しておく.
		m_c_OfficeOpenXml = c_OfficeOpenXml;																				// 設定On時のOfficeOpenXMLデータを保存.
//		cancel( CNK_OCRSetting, CNK_Size, PnlComGetJobFax() );																// OCR文字認識設定をOff.
		cancel( CNK_OCRSetting, CNK_Size, pc_Job );																// OCR文字認識設定をOff.
	}
	return	True;
}
// 言語選択.
Bool PA053613200::prLanguage( short n )
{
// comment 2016/08/29 【HCLT_Tharun】【Accurev ID:1002990】Kloc work Changes ID:68783 [NPD.FUNC.MUST] NULL check modified.
//	TYPC_OfficeOpenXml c_OfficeOpenXml = PnlComGetJobFax()->getJobSpec()->pc_Send->getOfficeOpenXml();
	APIC_Job* pc_Job = PnlComGetJobFax();
	if ( NULL == pc_Job ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if ( NULL == pc_JobSpec ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	TYPC_Send* pc_Send = pc_JobSpec->pc_Send;
	if ( NULL == pc_Send ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	TYPC_OfficeOpenXml c_OfficeOpenXml = pc_Send->getOfficeOpenXml();
	c_OfficeOpenXml.setLanguage( m_pst_LangTbl[ n ]->e_Language );
// comment 2016/08/29 【HCLT_Tharun】【Accurev ID:1002990】Kloc work Changes ID:68784 [NPD.FUNC.MUST] NULL check modified.
//	PnlComGetJobFax()->getJobSpec()->pc_Send->setOfficeOpenXml( c_OfficeOpenXml );
	pc_Send->setOfficeOpenXml( c_OfficeOpenXml );
	return	True;
}
// 出力形式(DOCX).
Bool PA053613200::prOutputKindDOCX( short n )
{
// comment 2016/08/29 【HCLT_Tharun】【Accurev ID:1002990】Kloc work Changes ID:68785 [NPD.FUNC.MUST] NULL check modified.
//	TYPC_OfficeOpenXml c_OfficeOpenXml = PnlComGetJobFax()->getJobSpec()->pc_Send->getOfficeOpenXml();
	APIC_Job* pc_Job = PnlComGetJobFax();
	if ( NULL == pc_Job ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if ( NULL == pc_JobSpec ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	TYPC_Send* pc_Send = pc_JobSpec->pc_Send;
	if ( NULL == pc_Send ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); 
		return False;
	}
	TYPC_OfficeOpenXml c_OfficeOpenXml = pc_Send->getOfficeOpenXml();
	switch( n ) {
		case 0:	c_OfficeOpenXml.setOutPutFormat( TYPE_OutPutFormat_Text_Priority );		break;								// 文字優先.
		case 1:	c_OfficeOpenXml.setOutPutFormat( TYPE_OutPutFormat_Image_Priority );	break;								// 画像優先.
		case 2:	c_OfficeOpenXml.setOutPutFormat( TYPE_OutPutFormat_Image_Text );		break;								// 画像と文字.
	}
// comment 2016/08/29 【HCLT_Tharun】【Accurev ID:1002990】Kloc work Changes ID:68786 [NPD.FUNC.MUST] NULL check modified.
//	PnlComGetJobFax()->getJobSpec()->pc_Send->setOfficeOpenXml( c_OfficeOpenXml );
	pc_Send->setOfficeOpenXml( c_OfficeOpenXml );
	return	True;
}
// 出力形式(XLSX).
Bool PA053613200::prOutputKindXLSX( short n )
{
// comment 2016/08/29 【HCLT_Tharun】【Accurev ID:1002990】Kloc work Changes ID:68787 [NPD.FUNC.MUST] NULL check modified.
//	TYPC_OfficeOpenXml c_OfficeOpenXml = PnlComGetJobFax()->getJobSpec()->pc_Send->getOfficeOpenXml();
	APIC_Job* pc_Job = PnlComGetJobFax();
	if ( NULL == pc_Job ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if ( NULL == pc_JobSpec ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	TYPC_Send* pc_Send = pc_JobSpec->pc_Send;
	if ( NULL == pc_Send ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	TYPC_OfficeOpenXml c_OfficeOpenXml = pc_Send->getOfficeOpenXml();
	switch( n ) {
		case 0:	c_OfficeOpenXml.setOutPutFormat( TYPE_OutPutFormat_Image_Text );	break;									//	画像と文字.
		case 1:	c_OfficeOpenXml.setOutPutFormat( TYPE_OutPutFormat_Text );			break;									//	文字.
	}
// comment 2016/08/29 【HCLT_Tharun】【Accurev ID:1002990】Kloc work Changes ID:68788 [NPD.FUNC.MUST] NULL check modified.
//	PnlComGetJobFax()->getJobSpec()->pc_Send->setOfficeOpenXml( c_OfficeOpenXml );
	pc_Send->setOfficeOpenXml( c_OfficeOpenXml );
	return	True;
}
// 回転補正.
Bool PA053613200::prRotationCorrect( Bool on )
{
// comment 2016/08/29 【HCLT_Tharun】【Accurev ID:1002990】Kloc work Changes ID:68789 [NPD.FUNC.MUST] NULL check modified.
//	TYPC_OfficeOpenXml c_OfficeOpenXml = PnlComGetJobFax()->getJobSpec()->pc_Send->getOfficeOpenXml();
	APIC_Job* pc_Job = PnlComGetJobFax();
	if ( NULL == pc_Job ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if ( NULL == pc_JobSpec ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	TYPC_Send* pc_Send = pc_JobSpec->pc_Send;
	if ( NULL == pc_Send ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	TYPC_OfficeOpenXml c_OfficeOpenXml = pc_Send->getOfficeOpenXml();
	c_OfficeOpenXml.setRotationCorrection( on );
// comment 2016/08/29 【HCLT_Tharun】【Accurev ID:1002990】Kloc work Changes ID:68790 [NPD.FUNC.MUST] NULL check modified.
//	PnlComGetJobFax()->getJobSpec()->pc_Send->setOfficeOpenXml( c_OfficeOpenXml );
	pc_Send->setOfficeOpenXml( c_OfficeOpenXml );
	return	True;
}
// OK.
Bool PA053613200::prOk( Bool on )
{
	procOkSetup();																											//	設定終了共通処理.
	return	True;
}
// キャンセル.
Bool PA053613200::prCancel( Bool on )
{
	PNLC_StackManJobSpec::newInstance()->rollbackJobSpec();																	//	設定を元に戻す.
	return	True;
}
//従来処理(画面外処理)---------------------------------------------------------------------------------------------------.
//	表示部品のセットアップ.
void PA053613200::setupGamen( short option )
{
	PCGC_ScanFaxReadSetDetailComLib::setupGamen( option );
	if( ScanOapExtSet.isActiveGeneralApplication() ) {
		printLEDControl( False );																							// スタートキー無効.
	}
	if( isNowGamen( &gp_DocKeepUSBMemoryExport_box ) ) {
		PMessageArea::setupPrintLEDBoxToUSB();																			// BoxToUSB用PrintLEDのセットアップ.
	}
// comment 2022/08/02 HungDM17-FPT [OP_BTS-38936] 詳細設定画面に「スタート」キーが青になる問題対策。
// 	else{
// 		PMessageArea::setupPrintLEDFax();																				// 宛先件数0件の場合は、LEDを赤にする.
// 	}
}
//	ファクス.
void PA053613200::procFax( uchar option )
{
	if( ( True == ScanOapExtSet.isActiveGeneralApplication() ) && ( NULL !=  ScanOapExtSet.getApplication() ) && ( TYPE_ExtTypeScanD == ScanOapExtSet.getApplication()->s_Version.e_Type ) ) {		// OpenAPIスキャンアプリ(Tyde-D)使用時.
		JBT_InitOapBoxUtil( JBLE_OAP_SCANFAX );																				// スキャン連携ボックスユーティリティ初期化.
		PCGC_ScanFaxReadSetDetailComLib::procAppKey( option, JBLE_OAP_SCANFAX );											// 親クラスのアプリケーションキー選択処理実行.
	} else {																												// LocalUI.
		PUsercom::procFax( option );
	}
}
//	ファクス.
void PA053613200::procFaxOnly( uchar option )
{
	if( ( True == ScanOapExtSet.isActiveGeneralApplication() ) && ( NULL != ScanOapExtSet.getApplication() ) && ( TYPE_ExtTypeScanD == ScanOapExtSet.getApplication()->s_Version.e_Type ) ) {		// OpenAPIスキャンアプリ(Tyde-D)使用時.
		JBT_InitOapBoxUtil( JBLE_OAP_FAXONLY );																				// スキャン連携ボックスユーティリティ初期化.
		PCGC_ScanFaxReadSetDetailComLib::procAppKey( option, JBLE_OAP_FAXONLY );											// 親クラスのアプリケーションキー選択処理実行.
	} else {																												// LocalUI.
		PUsercom::procFaxOnly( option );
	}
}
//	スタート.
void PA053613200::procStart( uchar option )
{
	if( ( True == ScanOapExtSet.isActiveGeneralApplication() ) && ( NULL !=  ScanOapExtSet.getApplication() ) && ( TYPE_ExtTypeScanD == ScanOapExtSet.getApplication()->s_Version.e_Type ) ) {		// OpenAPIスキャンアプリ(Tyde-D)使用時.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	onlyPanel.checkWarningAdminMachineForBox( PnlComGetJobScan( PCGE_JobPutJBLBox ) );										// 統合管理装置装着時警告制御処理.
	if( True == printStartDisable() ) {																						//	スタートキーアンバーの時は受け付けない.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	procOkSetup();																											//	設定終了共通処理.
	if( &gp_BoxDocKeepUSBMemory_box == PBasicMessageProc::getNowGamen() ) {													// 外部メモリへの保存時.
		PNLC_BoxDataKeyControlLib::newInstance()->procStartBoxDocKeepUSBMemory();											// 外部メモリへの保存スタート.
	}else if( &gp_DocKeepUSBMemoryExport_box == PBasicMessageProc::getNowGamen() ) {										// ボックスから外部メモリへ.
		if( True == onlyPanel.isWarningOn( PnlComGetJobScan( PCGE_JobPutJBLBox ), PNL_ERROR_NoFrequency_Box ) || True == onlyPanel.isWarningOn( PnlComGetJobScan( PCGE_JobPutJBLBox ), PNL_ERROR_CardUnset_Box ) || True == onlyPanel.isWarningOn( PnlComGetJobScan( PCGE_JobPutJBLBox ), PNL_ERROR_AdminMachinePowerSupply_Box ) || True == onlyPanel.isWarningOn( PnlComGetJobScan( PCGE_JobPutJBLBox ), PNL_ERROR_AdminMachineRestrictedFunction_Box ) ) {		// 管理装置度数なしチェック.// 管理カード未挿入チェック.// 管理装置電源確認警告チェック.// 機能毎使用設定：使用禁止チェック.
			return;																											//統合管理装置の警告が発生している場合は、関数終了.
		}
		gp_USBMemoryExport_actExport.startWait();
		if( False == printStartDisable() ) {
			PNLC_PopupListControl::newInstance()->addPopup( &gp_USBMemoryExport_actExport, this, NULL, 360, 216, 80, 48 );	// 次画面を自分自身に設定する.
		} else {
			setBuzzerType( PCGE_BuzzerType_InputNg );																		// 開始不可能 ブザー鳴動.
		}
	} else if( &gp_DocKeep_box == PBasicMessageProc::getNowGamen() ) {														// ボックスへ保存.
		PNLC_BoxDataKeyControlLib::newInstance()->procStartBoxDocKeep();													// 文書保存.
	} else {
		PNLC_BoxDataKeyControlLib::newInstance()->procStartDocNetSendNetSend( option );										// 送信スタート.
	}
}
//	ストップ.
void PA053613200::procStop( uchar option )
{
	if( ( True == ScanOapExtSet.isActiveGeneralApplication() ) && ( NULL !=  ScanOapExtSet.getApplication() ) && ( TYPE_ExtTypeScanD == ScanOapExtSet.getApplication()->s_Version.e_Type ) ) {		// OpenAPIスキャンアプリ(Tyde-D)使用時.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	procOkSetup();																											//	設定終了共通処理.
	PCGC_ScanFaxReadSetDetailComLib::procStop( option );
}
//	ボックス.
void PA053613200::procBox( uchar option )
{
	if( ( True == ScanOapExtSet.isActiveGeneralApplication() ) && ( NULL !=  ScanOapExtSet.getApplication() ) && ( TYPE_ExtTypeScanD == ScanOapExtSet.getApplication()->s_Version.e_Type ) ) {		// OpenAPIスキャンアプリ(Tyde-D)使用時.
		JBT_InitOapBoxUtil( JBLE_OAP_BOX );																					// スキャン連携ボックスユーティリティ初期化.
		PCGC_ScanFaxReadSetDetailComLib::procAppKey( option, JBLE_OAP_BOX );												// 親クラスのアプリケーションキー選択処理実行.
	} else {																												// LocalUI.
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}
//	ユビキタスプリント.
void PA053613200::procUbqPrint( uchar option )
{
	if( ( True == ScanOapExtSet.isActiveGeneralApplication() ) && ( NULL !=  ScanOapExtSet.getApplication() ) && ( TYPE_ExtTypeScanD == ScanOapExtSet.getApplication()->s_Version.e_Type ) ) {		// OpenAPIスキャンアプリ(Tyde-D)使用時.
		JBT_InitOapBoxUtil( JBLE_OAP_UBQPRINT );																			// スキャン連携ボックスユーティリティ初期化.
		PCGC_ScanFaxReadSetDetailComLib::procAppKey( option, JBLE_OAP_UBQPRINT );											// 親クラスのアプリケーションキー選択処理実行.
	} else {																												// LocalUI.
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}
//	コピー.
void PA053613200::procCopy( uchar option )
{
	if( ( True == ScanOapExtSet.isActiveGeneralApplication() ) && ( NULL !=  ScanOapExtSet.getApplication() ) && ( TYPE_ExtTypeScanD == ScanOapExtSet.getApplication()->s_Version.e_Type ) ) {		// OpenAPIスキャンアプリ(Tyde-D)使用時.
		JBT_InitOapBoxUtil( JBLE_OAP_COPY );																				// スキャン連携ボックスユーティリティ初期化.
		PCGC_ScanFaxReadSetDetailComLib::procAppKey( option, JBLE_OAP_COPY );												// 親クラスのアプリケーションキー選択処理実行.
	} else {																												// LocalUI.
		PCGC_ScanFaxReadSetDetailComLib::procCopy( option );
	}
}
//	ブラウザ.
void PA053613200::procBrowser( uchar option )
{
	if( ( True == ScanOapExtSet.isActiveGeneralApplication() ) && ( NULL != ScanOapExtSet.getApplication() ) && ( TYPE_ExtTypeScanD == ScanOapExtSet.getApplication()->s_Version.e_Type ) ) {		// OpenAPIスキャンアプリ(Tyde-D)使用時.
		JBT_InitOapBoxUtil( JBLE_OAP_BROWSER );																				// スキャン連携ボックスユーティリティ初期化.
		PCGC_ScanFaxReadSetDetailComLib::procAppKey( option, JBLE_OAP_BROWSER );											// 親クラスのアプリケーションキー選択処理実行.
	} else {																												// LocalUI.
		PCGC_ScanFaxReadSetDetailComLib::procBrowser( option );																// 親クラスコール.
	}
}
//画面固有処理-----------------------------------------------------------------------------------------------------------.
//	設定終了共通内部設定処理.
void PA053613200::procOkSetup()
{
// comment 2016/08/29 【HCLT_Tharun】【Accurev ID:1002990】Kloc work Changes ID:68791 [NPD.FUNC.MUST] NULL check modified.
//	if( False == PnlComGetJobFax()->getJobSpec()->pc_Send->getOfficeOpenXml().getOcrOn() ) {
	APIC_Job* pc_Job = PnlComGetJobFax();
	if ( NULL == pc_Job ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if ( NULL == pc_JobSpec ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	TYPC_Send* pc_Send = pc_JobSpec->pc_Send;
	if ( NULL == pc_Send ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); 
		return;
	}
	if ( False == pc_Send->getOfficeOpenXml().getOcrOn() ) {
		cancel( CNK_OCRSetting, CNK_Size, PnlComGetJobScanFax() );
	}
}
