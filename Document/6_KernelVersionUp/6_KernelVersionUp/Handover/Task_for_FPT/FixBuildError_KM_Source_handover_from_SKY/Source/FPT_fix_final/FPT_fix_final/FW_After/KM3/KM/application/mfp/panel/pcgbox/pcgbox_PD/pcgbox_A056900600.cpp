//クラス-----------------------------------------------------------------------------------------------------------------.
// ファイル : CHOP_A056900600_文書詳細(外部メモリー).
// Version  : PD ver.013.
//-----------------------------------------------------------------------------------------------------------------------.
#include "panel/plibh/plibh_CLCD/plibh_Mtui.h"
#include <system/saa/APIC_UsbMemDeviceInfo.h>
#include <system/prv/APIC_PreviewConverter.h>
#include <system/ats/APIC_AutoSelect.h>
#include <system/job/APIC_Job.h>
#include <system/nvd/APIC_Setting.h>
#include <system/sys/APIC_SystemServiceFunction.h>
#include <panel/plibh/plibh_CLCD2/PNLC_StackManJobSpec.h>
#include <panel/pcgbox/pcgbox_CLCD2/pipbox_popup.h>
#include <panel/pcgcom/pcgcom_CLCD2/PNLC_PopupListControl.h>
#include <panel/plibh/plibh_CLCD2/PNLC_BoxDataLib.h>
#include <panel/plibh/plibh_CLCD2/PNLC_BoxNin1ControlLib.h>
#include <panel/plibh/plibh_CLCD2/PNLC_ScanFaxDataChangeCheckLib.h>
#include <panel/plibh/plibhg/plibh.h>
#include <panel/pcgcom/pcgcomg/pcgcom.h>
#include <panel/pcgcom/pcgcom_CLCD2/PPopupAnimationCom.h>
#include <panel/plibh/plibh_CLCD2/PNLC_BoxDataLib.h>
#include <panel/plibh/plibh_CLCD2/PNLC_BoxDocInfoControlLib.h>
#include <panel/plibh/plibh_CLCD2/PNLC_BoxDocumentListInfoControlLib.h>
#include "panel/pcgcomponent/pcgcomponentg/PAppFuncGridDispComponent.h"
#include <panel/pcgbox/pcgbox_PD/pcgbox_A056900600.h>
#define  SIZE( table )    ( sizeof( table ) / sizeof( PDisplayParts* ) ) // 部品登録数を得る.
#ifdef DEF_LCD_7INCH
#include <panel/pcgbox/pcgbox_PD/PD/A056900600_7inch.cpp>
#else
#include <panel/pcgbox/pcgbox_PD/PD/A056900600.cpp>
#endif // DEF_LCD_7INCH.
//PanelDesigner対応------------------------------------------------------------------------------------------------------.
// 画面開始処理.
void PA056900600::startGamen()
{
	bl_PanelDesigner = True;	// PanelDesigner対応.
	PPopupAnimationCom::startGamen();	// 親クラス呼出し(これを行わないとPopupWindowが表示しない）.
// コールバック関数の登録.
	ATTACH_OBSERVER( PA056900600, gp_A056900600, PA056900600::prLeft, A056900600_Btn0023, Bool, Bool );		// ページ左.
	ATTACH_OBSERVER( PA056900600, gp_A056900600, PA056900600::prRight, A056900600_Btn0024, Bool, Bool );	// ページ右.
	setOkButtonForFunctionBadge( &A056900600_Btn0006 ); // 機能バッチボタン押下時に代替え処理を行うＯＫキーを登録.
// 変数等の初期化処理を記載する。
	ss_USBSelectNowNum = 1;	// USB選択ファイル値.
	PJobListControl::GetSelectDocList();
	for( short ss_Cnt = 0; ss_Cnt < PNLD_ScanDirectUSBDetail_boxMaxFileData ; ss_Cnt++ ) {
		ps_USBDetailInfo[ss_Cnt] = NULL;	// 画面開始時に毎回初期化しておく.
	}
	PNLS_DirEntryUSBTbl* pst_USBEntryTbl = PNLC_BoxDataLib::newInstance()->getUSBTbl();	// USB情報のテーブル取得.
	if( pst_USBEntryTbl != NULL ) {	// USB情報の取得.
		short ss_ListIndex = 0;
		short ss_NowListCnt = PNLC_BoxDataLib::newInstance()->getNowListCnt();
		for( short ss_TblChkCnt = 0; ss_TblChkCnt < ss_NowListCnt ; ss_TblChkCnt++ ) {
			if( True == pst_USBEntryTbl[ss_TblChkCnt].bl_select ) {
				ps_USBDetailInfo[ss_ListIndex] = &( pst_USBEntryTbl[ss_TblChkCnt].st_info );	// 選択ファイル詳細情報取得.
				if( '\0' == ps_USBDetailInfo[ss_ListIndex]->sc_name[0] ) {	// ファイル名が取れていない場合、以後文書ファイル情報が無し.
					break;
				}
				ss_ListIndex++;
			}
		}
	}
	ss_USBSelectMaxNum = PNLC_BoxDataLib::newInstance()->getDocSelectNum();	// USB選択ファイル最大数.
// ここに構成による表示条件を記載する。
	A056900600_Btn0023.activate( (1 == ss_USBSelectMaxNum) ? False : True );	// ページ左.
	A056900600_Btn0024.activate( (1 == ss_USBSelectMaxNum) ? False : True );	// ページ右.
// ここに画面の初期化処理を記載する。
	char ss_UsbDirectory[PNLD_MaxStrUsbMemDeviceName];		memset( ss_UsbDirectory, '\0', sizeof( ss_UsbDirectory ) );
	strncpy( ss_UsbDirectory, PNLC_BoxDataLib::newInstance()->getUsbMemAddress().sc_DeviceName, sizeof( ss_UsbDirectory ) -1 );
	ss_UsbDirectory[ sizeof( ss_UsbDirectory )-1 ] = '\0';
	A056900600_Txt0014.changeExternalText( reinterpret_cast<uchar*>( ss_UsbDirectory ) );	// ファイルパス.
	setupGamenAtButtonOn();	// ボタンＯＮ時の表示部品のセットアップ.
}
// ボタンＯＮ時の表示部品のセットアップ.
#define PNLD_KEY_USBFILEPASSNAME_FILENAMEEXTENTTYPEMAX	12	// ファイル形式の種類数.

void PA056900600::setupGamenAtButtonOn()
{
	if( ps_USBDetailInfo[ss_USBSelectNowNum-1] == NULL ) {
		return;
	}
	A056900600_Txt0004.setN( ss_USBSelectNowNum );	// USB選択ファイル値.
	A056900600_Txt0003.setN( ss_USBSelectMaxNum );	// USB選択ファイル最大数.

	char cExtension[6];	memset( cExtension, '\0', sizeof( cExtension ) );
	char cFileName[PNLD_MaxStrUsbMemDeviceName];	memset( cFileName, '\0', sizeof( cFileName ) );
	short sTextLen = getNativeCodeByteCount( reinterpret_cast<char*>( ps_USBDetailInfo[ss_USBSelectNowNum-1]->sc_name ) );
	short sTextLenMax = static_cast<short>( strlen( ps_USBDetailInfo[ss_USBSelectNowNum-1]->sc_name ) );
	for( int32_t i = sTextLenMax; i > 0; i--, sTextLen-- ) {
		if( '.' == ps_USBDetailInfo[ss_USBSelectNowNum-1]->sc_name[i] ) {
			setupInputStringUTF8( 1, sTextLen, ps_USBDetailInfo[ss_USBSelectNowNum-1]->sc_name, cFileName, 0 );
			for( int32_t j = 0; j < 6; j++ ) {
				// Comment 2017/03/10 HCL_Sasaki IT5_4.0 Klockwork Changes ID:72735.
// comment 2018/01/23【HCLT_Neelima】【IT5_4.2】Warning removal activity.
//				if( (0 <= i) && (sizeof(ps_USBDetailInfo[ss_USBSelectNowNum-1]->sc_name) > i) ){
				if( (0 <= i) && (sizeof(ps_USBDetailInfo[ss_USBSelectNowNum-1]->sc_name) > ( unsigned int ) i ) ){
					cExtension[j] = ps_USBDetailInfo[ss_USBSelectNowNum-1]->sc_name[i];
				}else{
					LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
				}
				if( i != sTextLenMax ) {
					i++;
				} else {
					break;
				}
			}
			break;
		}
	}
	for( uchar k = 0; k < strlen(cExtension); k++ ) {
		cExtension[k] = static_cast<char>( tolower( cExtension[k] ) );
	}
	char* sc_FileNameExtent[] = {".pdf", ".jpg", ".jpeg", ".tif", ".tiff", ".xps", ".docx", ".xlsx", ".pptx", ".ppml", ".vdx", ".zip"};
	char* sc_FileNameExtentGraph[] = {"PDF", "JPEG", "JPEG", "TIFF", "TIFF", "XPS", "DOCX", "XLSX", "PPTX", "PPML", "VDX", "ZIP"};
	char cExtensionGraph[6];	memset( cExtensionGraph, '\0', sizeof( cExtensionGraph ) );
	for(char i = 0; i < PNLD_KEY_USBFILEPASSNAME_FILENAMEEXTENTTYPEMAX; i++ ) {
		if( strcmp( cExtension , sc_FileNameExtent[i] ) == 0 ) {
			memcpy( &cExtensionGraph, sc_FileNameExtentGraph[i], sizeof( cExtensionGraph ) );
		}
	}
	int32_t i32_USBDetailSize = static_cast<int32_t>( ps_USBDetailInfo[ss_USBSelectNowNum-1]->s_stat.st_size / 1024 );
	if( 0 != ( ps_USBDetailInfo[ss_USBSelectNowNum-1]->s_stat.st_size % 1024 ) ) {
		i32_USBDetailSize = i32_USBDetailSize + 1;
	}
  // 更新日時.
	struct tm st_Time;	memset( &st_Time, '\0', sizeof( st_Time ) );
	SYSF_localtime_r( &ps_USBDetailInfo[ss_USBSelectNowNum-1]->s_stat.st_mtime, &st_Time );
	st_Time.tm_year += 1900;	// 年を整形.
	char sc_tmpDay[JBLD_DAYDISP_MAX + JBLD_TIMEDISP_MAX + 7];		//日にち格納バッファ.
// Comment 2016/09/06 [HCLT_SunilNP] [1002990] 【IT5_3.3】Klocwork Changes ID: 69244 [UNINIT.STACK.ARRAY.MIGHT] initialized.
	memset( sc_tmpDay, '\0', sizeof( sc_tmpDay ) );
	switch( PnlComGetMachineSetting()->getSystemType() ) {
// Comment 2016/09/06 [HCLT_SunilNP] [1002990] 【IT5_3.3】Klocwork Changes ID: 69245, 69246, 69247 [SV.STRBO.UNBOUND_SPRINTF] used snprintf .
//		case TYPE_SystemType_US: sprintf( sc_tmpDay, "%02d/%02d/%04d", st_Time.tm_mon+1, st_Time.tm_mday, st_Time.tm_year ); break;	// ＭＣ仕向け.
//		case TYPE_SystemType_JP: sprintf( sc_tmpDay, "%04d/%02d/%02d", st_Time.tm_year, st_Time.tm_mon+1, st_Time.tm_mday ); break;	// 日本仕向け.
//		case TYPE_SystemType_EU: sprintf( sc_tmpDay, "%02d/%02d/%04d", st_Time.tm_mday, st_Time.tm_mon+1, st_Time.tm_year ); break;	// ＥＵ仕向け.
		case TYPE_SystemType_US: snprintf( sc_tmpDay, sizeof( sc_tmpDay ), "%02d/%02d/%04d", st_Time.tm_mon+1, st_Time.tm_mday, st_Time.tm_year ); break;	// ＭＣ仕向け.
		case TYPE_SystemType_JP: snprintf( sc_tmpDay, sizeof( sc_tmpDay ), "%04d/%02d/%02d", st_Time.tm_year, st_Time.tm_mon+1, st_Time.tm_mday ); break;	// 日本仕向け.
		case TYPE_SystemType_EU: snprintf( sc_tmpDay, sizeof( sc_tmpDay ), "%02d/%02d/%04d", st_Time.tm_mday, st_Time.tm_mon+1, st_Time.tm_year ); break;	// ＥＵ仕向け.
	}
	char sc_tmpTime[JBLD_TIMEDISP_MAX + 2 + 1];		//時間格納バッファ.
// Comment 2016/09/06 [HCLT_SunilNP] [1002990] 【IT5_3.3】Klocwork Changes ID: 69239, 69240 [SV.STRBO.UNBOUND_COPY] initialized and snprintf is used.
//	sprintf( sc_tmpTime, "%02d:%02d",st_Time.tm_hour,st_Time.tm_min );	// 時間.
	memset( sc_tmpTime, '\0', sizeof( sc_tmpTime ) );
	snprintf( sc_tmpTime, sizeof( sc_tmpTime ), "%02d:%02d",st_Time.tm_hour,st_Time.tm_min );	// 時間.
	char sc_DateTime[ JBLD_TIMEDISP_MAX + JBLD_DAYDISP_MAX + JBLD_TIMEDISP_MAX + 10];
	strncpy( sc_DateTime, sc_tmpDay, sizeof( sc_DateTime )-1 );	//ドキュメント登録日 + ドキュメント時間.
	sc_DateTime[ sizeof( sc_DateTime )-1 ] = '\0';
// Comment 2016/09/06 [HCLT_SunilNP] [1002990] 【IT5_3.3】Klocwork Changes ID: 69248 [SV.STRBO.UNBOUND_COPY] strncat is used.
//	strcat( sc_DateTime, reinterpret_cast<char*>( " " ) );
	strncat( sc_DateTime, reinterpret_cast<const char*>( " " ), ( sizeof( sc_DateTime ) - strlen( sc_DateTime ) ) - 1 );
//	strcat( sc_DateTime, sc_tmpTime );
	strncat( sc_DateTime, sc_tmpTime, ( sizeof( sc_DateTime ) - strlen( sc_DateTime ) ) - 1 );
	A056900600_Txt0016.changeExternalText( reinterpret_cast<uchar*>( sc_DateTime ) );	// 更新日時.
  // 文書名.
	A056900600_Txt0018.changeExternalText( reinterpret_cast<uchar*>( cFileName ) );	// 文書名.
  // サイズ.
	char cFileSize[20+1];	memset( cFileSize, '\0', sizeof( cFileSize ) );
	snprintf( cFileSize, sizeof(cFileSize), "%dKB", i32_USBDetailSize );
	char sc_docName[APID_DocFileNameMAX + 2];	memset( sc_docName,'\0',sizeof(sc_docName) );
	setupInputString( 1, 20, cFileSize, sc_docName, 0, getCharSetForPanel() );	//ドキュメント表示用13文字取り出す.
	A056900600_Txt0020.changeExternalText( reinterpret_cast<uchar*>( sc_docName ), static_cast<short>( strlen(sc_docName) ), getCharSetForPanel() );	// サイズ.
  // ファイル形式.
	memset( sc_docName,'\0',sizeof(sc_docName) );
	setupInputString( 1, 20, cExtensionGraph, sc_docName, 0, getCharSetForPanel() );	//ドキュメント表示用13文字取り出す.
	A056900600_Txt0022.changeExternalText( reinterpret_cast<uchar*>( sc_docName ), static_cast<short>( strlen(sc_docName) ), getCharSetForPanel() );	// ファイル形式.
}
// ページ左.
Bool PA056900600::prLeft( Bool on )
{
	if( 1 < ss_USBSelectNowNum ) {		//２ページ以上の時.
		ss_USBSelectNowNum--;		//ページを１つ減らす.
	} else {		//最大ページにする.
		ss_USBSelectNowNum = ss_USBSelectMaxNum;
	}
	return	True;
}
// ページ右.
Bool PA056900600::prRight( Bool on )
{
	if( ss_USBSelectMaxNum > ss_USBSelectNowNum ) {		//最大ページ－1以下の時.
		ss_USBSelectNowNum++;		//ページを１つ増やす.
	} else {		//1ページ目にする.
		ss_USBSelectNowNum = 1;
	}
	return	True;
}
