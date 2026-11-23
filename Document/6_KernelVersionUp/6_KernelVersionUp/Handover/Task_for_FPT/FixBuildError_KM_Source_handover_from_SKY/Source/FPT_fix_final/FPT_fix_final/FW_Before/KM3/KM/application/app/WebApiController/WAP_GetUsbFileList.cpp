//-----------------------------------------------------------------------------.
// @file		WAP_GetUsbFileList.cpp.
//-----------------------------------------------------------------------------.
#include <stdio.h>
#include <string.h>
#include <memory.h>

#include "SYSC_OSWrapper.h"
#include "json/json.h"
#include "json/reader.h"
#include "json/writer.h"
#include "json/value.h"

#include "system/sys/sysg/LOGF_debug.h"

#include "system/job/APIC_SecureList.h"
#include "system/doc/APIC_Doc.h"
#include "system/doc/DOCC_BoxIdDef.h"
#include "system/nvd/APIC_Setting_MachineSetting.h"
#include "system/nvd/APIC_Setting.h"
#include "system/sys/APIC_OptionSetting.h"	
#include <printApp/APIC_USBPrintIf.h>
#include "WAP_plugintrace.h"
#include "WAP_BoxCommonLib.h"
#include "WAP_AsyncResponder.h"

#include "WAP_GetUsbFileList.h"
#include "WAP_Power.h"

//-----------------------------------------------------------------------------.
// definitions.
//-----------------------------------------------------------------------------.
Bool bl_semTaken = False; // 2020/06/26 [IT6_2.0MR3] [OP_BTS-24543] HCL-Gayathri Added.	

//-----------------------------------------------------------------------------.
// テーブル定義.
//-----------------------------------------------------------------------------.

//-----------------------------------------------------------------------------.
// WAPC_GetUsbFileListクラス.
//-----------------------------------------------------------------------------.
/**
 * @brief		コンストラクタ.
 * @return		なし.
* @author		
   @since		2018/12/27.	
 */
WAPC_GetUsbFileList::WAPC_GetUsbFileList()
{
	// インスタンス取得.
	
}

/**
 * @brief		デストラクタ.
 * @return		なし.
* @author		
   @since		2018/12/27.	
 */
WAPC_GetUsbFileList::~WAPC_GetUsbFileList()
{
	
}

/**
 * @brief		コントローラの初期化.
 * @return 		初期化の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
* @author		
   @since		2018/12/27.	
 */
int32_t WAPC_GetUsbFileList::Initialize()
{
	return WAPD_NoError;
}

/**
 * @brief		コントローラの終了.
 * @return 		初期化の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
* @author		
   @since		2018/12/27.	
 */
int32_t WAPC_GetUsbFileList::Finalize()
{
	return WAPD_NoError;
}

/**
 * @brief		コントローラのレスポンス生成.
 * @param[in]	ps_Param パラメータ構造体.
 * @return		レスポンス生成の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
* @author		
   @since		2018/12/27.
 * @date		
 */
int32_t WAPC_GetUsbFileList::ResponseGenerate( WAPS_SubController* ps_Param )
{
	int32_t si32_Ret = WAPD_AnyError;

	// パラメータNULLチェック.
	if( NULL == ps_Param ) {
		WAPG_API_trace("[ERR]%s NULL Parameter!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// 管理データアクセス用インスタンス取得.
	WAPC_GetUsbFileList_DataMng*	pc_DataMng = WAPC_GetUsbFileList_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetUsbFileList_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// リクエスト処理の排他用セマフォ取得.
	pc_DataMng->semTake();

	// リクエストメソッド分岐.
	switch( ps_Param->Method ) {
		case WAPD_HTTP_POST:
			// WebAPI電源管理.
			WAPC_Power* pc_Power = WAPC_Power::newInstance();
			if ( NULL == pc_Power ) {
			    WAPG_API_trace("%s(%d) WAPC_Power::newInstance NULL 1\n", __FILE__, __LINE__);
			    LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, WAP_LOGF_LEVEL_FATAL, "WAPC_Power::newInstance NULL 1");
			}

			// HDDデータアクセス用電源管理の省電力抑止.
//2019/06/19 [IT6_EagleH] [OP_BTS-7748] HCL_Vijayaraghavan Modification Start.
//			int32_t si32_Ret = pc_Power->waitStartAccessHDD();
			int si32_ret = pc_Power->waitStartAccessHDDforPowerStatus();		// 2021/08/04 FPT-TuanBV1 IT6_2.2 OP_BTS-34595.
			if( WAPD_AnyError == si32_ret ){
				// 2021/08/04 FPT-TuanBV1 IT6_2.2 OP_BTS-34595 Start.
				m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;	//500
				printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__);
				WAPG_API_trace("[ERR] WAPC_Power::waitStartAccessHDDforInit Error\n");
				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
				// HDDデータアクセス用電源管理の省電力抑止解除.
				si32_Ret = pc_Power->EndAccessHDD();
				if( WAPD_AnyError == si32_Ret ){
					WAPG_API_trace("%s(%d) WAPC_Power::EndAccessHDD Error\n", __FILE__, __LINE__);
				}
				// リクエスト処理の排他用セマフォ開放.
				pc_DataMng->semGive();
				return WAPD_AnyError;
				// 2021/08/04 FPT-TuanBV1 IT6_2.2 OP_BTS-34595 End.
			}

			si32_Ret = ExecPostRequest(ps_Param);

			// HDDデータアクセス用電源管理の省電力抑止解除.
			si32_ret = pc_Power->EndAccessHDD();
			if( WAPD_AnyError == si32_ret ){
				WAPG_API_trace("%s(%d) WAPC_Power::EndAccessHDD Error\n", __FILE__, __LINE__);
			}
//2019/06/19 [IT6_EagleH] [OP_BTS-7748] HCL_Vijayaraghavan Modification End.

			break;

		case WAPD_HTTP_GET:
		case WAPD_HTTP_PUT:
		case WAPD_HTTP_DELETE:
		default:
			// [リクエスト不正].
			WAPG_API_trace("[ERR]%s Request Error!.\n", __PRETTY_FUNCTION__ );
			// クライアントへ結果返却.
			printf("405:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //405->ErrorStatus[WAPD_STATUS_METHOD_NOT_ALLOWED] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_METHOD_NOT_ALLOWED;
			break;
	}

	// リクエスト処理の排他用セマフォ開放.
	pc_DataMng->semGive();

	return si32_Ret;
}

/**
 * @brief		コントローラの非同期レスポンス生成.
 * @param[in]	ps_Param パラメータ構造体.
 * @return		レスポンス生成の成否, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
* @author		
   @since		2018/12/27.
 */
int32_t WAPC_GetUsbFileList::AsyncResponseGenerate( WAPS_SubController* ps_Param )
{
	int32_t si32_Ret = WAPD_AnyError;

	// パラメータNULLチェック.
	if( NULL == ps_Param ) {
		WAPG_API_trace("[ERR]%s NULL Parameter!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// 管理データアクセス用インスタンス取得.
	WAPC_GetUsbFileList_DataMng*	pc_DataMng = WAPC_GetUsbFileList_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetUsbFileList_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// リクエスト処理の排他用セマフォ取得.
	pc_DataMng->semTake();

	// リクエストメソッド分岐.
	switch( ps_Param->Method ) {
		case WAPD_HTTP_POST:
			si32_Ret = ExecPostResponse(ps_Param);
			break;

		default:
			// [リクエスト不正].
			WAPG_API_trace("[ERR]%s Request Error!.\n", __PRETTY_FUNCTION__ );
			// クライアントへ結果返却.
			printf("405:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //405->ErrorStatus[WAPD_STATUS_METHOD_NOT_ALLOWED] in WAP_defs.h
			m_ResponseHeader->Status = WAPD_STATUS_METHOD_NOT_ALLOWED;
			break;
	}

	// 初期状態に遷移した場合.
	WAPC_AsyncResponse* pc_AsyncResponse = dynamic_cast<WAPC_AsyncResponse*>(WAPC_AsyncResponseContainer::newInstance()->getByHandle( m_pv_Handle ));
	if( NULL != pc_AsyncResponse ) {
		// 非同期レスポンスクラスのリストから削除.
		WAPC_AsyncResponseContainer::newInstance()->erase( pc_AsyncResponse->getReqId() );
		delete pc_AsyncResponse;
	}
	
	// リクエスト処理の排他用セマフォ開放.
	pc_DataMng->semGive();

	return si32_Ret;
}

/**
 * @brief		個別コントローラの共通非同期機能実行.		
 * @return		非同期機能実行結果, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
* @author		
   @since		2018/12/27.	
 */
int32_t WAPC_GetUsbFileList::FunctionExecute()
{
	// メソッド実行後に後処理が必要な場合にはここで実施.

	return WAPD_NoError;
}



/**
 * @brief			USBファイル取得コールバック関数.
 * @return		none.
 * @author		
   @since		2018/12/27.		.
 */
void WAPC_GetUsbFileList::ansUSBGetDirEntry( TYPS_DirEntryInfo* s_DirEntryInfo, short ss_num )
{

	WAPC_GetUsbFileList_DataMng*	pc_DataMng = WAPC_GetUsbFileList_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetUsbFileList_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		//m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
// Comment 2019/04/25 【HCL-Suribabu】【IT6_EagleH] Klockwork Changes 【ID:122284】[VOIDRET].
//		return WAPD_AnyError;
		return;
	}

		// リクエスト処理の排他用セマフォ取得.
	// 2020/06/26 [IT6_2.0MR3] [OP_BTS-24543] HCL-Gayathri Modified Start.	
	if( False == bl_semTaken ) {
		pc_DataMng->semTake();
	}
	bl_semTaken = True;
	
//	// レスポンスクラスの取得.
//	WAPC_AsyncResponse* pc_AsyncResponse = dynamic_cast<WAPC_AsyncResponse*>(WAPC_AsyncResponseContainer::newInstance()->getById( reinterpret_cast<uint64_t>(WAPC_GetUsbFileList::ansUSBGetDirEntry) ));
//	if( NULL == pc_AsyncResponse ) {
//		WAPG_API_trace("[ERR]%s pc_AsyncResponse is NULL!.\n", __PRETTY_FUNCTION__ );
//		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
//		// リクエスト処理の排他用セマフォ開放.
//		pc_DataMng->semGive();
//// Comment 2019/04/25 【HCL-Suribabu】【IT6_EagleH] Klockwork Changes 【ID:122285】[VOIDRET].
////		return WAPD_AnyError;
//		return;
//	}

	WAPS_DirEntryUSBTbl* s_USBEntryTbl = pc_DataMng->getUSBTbl();
	TYPS_DirEntryInfo* s_mlocalinfo = pc_DataMng->getlocalinfo();
	short ss_NowListCnt = pc_DataMng->getNowListCnt();
	short ss_FileNotFound = pc_DataMng->getFileNotFoundListCnt();
	
	if( NULL == s_USBEntryTbl || NULL == s_mlocalinfo ) {
		WAPG_API_trace("[ERR]%s NULL Parameter!.\n", __PRETTY_FUNCTION__ );
// Comment 2019/04/25 【HCL-Suribabu】【IT6_EagleH] Klockwork Changes 【ID:122286】[VOIDRET].
//		return WAPD_AnyError;
		return;
	}

	if( NULL != s_DirEntryInfo ){
		char sc_Extension[WAPD_WebUIUSB_File_ExtensionSize];
		char sc_fileName[WAPD_MaxStrUsbMemDeviceName];
// 2019/05/03 【IT6_EagleH】【Klock work ID:122279 [NNTS.MUST]】 HCLT-Vijayaraghavan Modification Start.
		memset( sc_fileName, '\0', sizeof( sc_fileName ) );
// 2019/05/03 【IT6_EagleH】【Klock work ID:122279 [NNTS.MUST]】 HCLT-Vijayaraghavan Modification End.
		char* sc_FileNameExtent[WAPD_ScanDirectUSBReferemce_FileNameExtentTypeMax] = {".pdf", ".jpg", ".jpeg", ".tif", ".tiff", ".xps", ".docx", ".xlsx", ".pptx", ".ppml", ".vdx", ".zip"};
		memset( sc_Extension, '\0', sizeof( sc_Extension ) );

		for( int32_t num=0; num< ss_num ; num++ ) {
			if( ss_NowListCnt >= WAPD_SelectDocListUSBTblMAX ) {	// 255行目以降のリストの見ようとしているか. // Upto 255 files is possible.
				WAPD_DEBUG("WAPC_GetUsbFileList:: getNowListCnt : out of bounds\n");
//// Comment 2019/04/25 【HCL-Suribabu】【IT6_EagleH] Klockwork Changes 【ID:122287】[VOIDRET].
////				return WAPD_AnyError;
//				return;
//				// 255行目以降は見えない（抜ける）.
				break;
			}
			
			if( s_mlocalinfo[num].s_stat.st_mode & S_IFDIR ) {		// ファイル名がフォルダの場合.
				s_USBEntryTbl[ss_NowListCnt].st_info = s_mlocalinfo[num];		// infoを設定する.
				ss_NowListCnt++;
			} else {		// ファイル名がフォルダのではない.
				memset( sc_Extension, '\0', sizeof( sc_Extension ) );
				memcpy( &sc_fileName, &s_mlocalinfo[num].sc_name, sizeof( sc_fileName ) );
				getExtension( sc_Extension, sc_fileName );		// 拡張子を取得.
				Bool b_FileExists = False;
				for( short j = 0; j < WAPD_ScanDirectUSBReferemce_FileNameExtentTypeMax; j++ ) {
					if( strcmp( sc_Extension, sc_FileNameExtent[j] ) == 0 ) {		// sc_Extensionとsc_FileNameExtent[j]を比べます.
						s_USBEntryTbl[ss_NowListCnt].st_info = s_mlocalinfo[num];		// infoを設定する.
						ss_NowListCnt++;
						b_FileExists = True;
						break;
					}
				}
				if( False == b_FileExists ){
					ss_FileNotFound++;		// Increments if unsupported files found. 
				}
			}
		}
		pc_DataMng->setNowListCnt(ss_NowListCnt);
		pc_DataMng->setFileNotFoundListCnt(ss_FileNotFound);
	} 
		
	TYPS_UsbMemAddress st_UsbMemAddress;
	st_UsbMemAddress = pc_DataMng->getUsbMemAddress();
	short ss_MaxListCnt = pc_DataMng->getMaxList();
	short ss_totalFileCnt = ss_NowListCnt + ss_FileNotFound; 
	Bool b_reqComp = True;
	
	// File acquisition stopped if supported files reached either 255 or total number of files in the current directory of USB ( whichever is less ). 
	if ( (ss_NowListCnt != 0) && (ss_NowListCnt < WAPD_SelectDocListUSBTblMAX ) 		// MR3 OP_BTS-25815 HCL-Naveen 20200724 Modified.
		&& ( ss_NowListCnt < ss_MaxListCnt ) ) {
		if( ss_totalFileCnt < ss_MaxListCnt){ // File acquisition stopped if supported + unsupported files reaches the total number of files in the current directory of USB. 
			
			char	sc_dir[WAPD_MaxStrUsbMemDeviceName];
			memset( sc_dir, '\0', sizeof( sc_dir ) );
			strncpy( sc_dir, st_UsbMemAddress.sc_DeviceName, sizeof( sc_dir )-1 );
			memset( s_mlocalinfo, 0, sizeof( *s_mlocalinfo ) );
			
			APIF_SaaGetDirEntryUsb( TYPE_CharSetUTF8, sc_dir, s_mlocalinfo, ss_totalFileCnt, 100, &WAPC_GetUsbFileList::ansUSBGetDirEntry );
			b_reqComp = False; // Acquisition is not yet completed. 
		}
		
	} 
	if ( True == b_reqComp ) { 
	
		// レスポンスクラスの取得.
		WAPC_AsyncResponse* pc_AsyncResponse = dynamic_cast<WAPC_AsyncResponse*>(WAPC_AsyncResponseContainer::newInstance()->getById( reinterpret_cast<uint64_t>(WAPC_GetUsbFileList::ansUSBGetDirEntry) ));
		if( NULL == pc_AsyncResponse ) {
			WAPG_API_trace("[ERR]%s pc_AsyncResponse is NULL!.\n", __PRETTY_FUNCTION__ );
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
			pc_DataMng->semGive();
			bl_semTaken = False;
			return;
		}

		// メッセージ送信.
		WAPC_AsyncResponder::newInstance()->postMessage( pc_AsyncResponse->getHandle(), WAPD_NoError );
		// 状態遷移[→POST(非同期レスポンス)待ち].
		pc_DataMng->setStatus( WAPE_USBFileListStatus_WaitResp );

		// リクエスト処理の排他用セマフォ開放.
		pc_DataMng->semGive();
		bl_semTaken = False;
		//return;	
	}
	// 2020/06/26 [IT6_2.0MR3] [OP_BTS-24543] HCL-Gayathri Modified End.	

}


/**
 * @brief		POST(非同期リクエスト)処理.
 * @param[in]	ps_Param HTTPリクエストヘッダ、ボディ.
 * @return		処理結果, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		日立 田中.
   @since		2018/12/27.		
 * @date		
 * @date		
 */
int32_t WAPC_GetUsbFileList::ExecPostRequest( const WAPS_SubController* ps_Param )
{
	int32_t	si32_ListOrder = 0;	// リスト開始位置.
	//char* sc_Folderpath;
	char sc_Folderpath[WAPD_MaxStrUsbMemDeviceName+1] = {0};
	Bool bl_Sort = False;
	Bool b_Abbr = False;	// 省略有無.
	Bool b_Ret = False;		// 要求結果.
	
	// 同期処理に初期化しておく.
	m_ResponseHeader->Async = WAPD_ASYNC_DISABLE;
	
		if( NULL == ps_Param ) {
		WAPG_API_trace("[ERR]%s NULL Parameter!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	// 管理データアクセス用インスタンス取得.
	WAPC_GetUsbFileList_DataMng*	pc_DataMng = WAPC_GetUsbFileList_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetUsbFileList_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}
	
	WAPC_BoxCommonLib*	cp_WAPC_BoxCommonLib = WAPC_BoxCommonLib::newInstance();
	if( NULL == cp_WAPC_BoxCommonLib ) {
		WAPG_API_trace("[ERR]%s WAPC_BoxCommonLib::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}

	
	b_Ret = cp_WAPC_BoxCommonLib->getRequestParam_ListOffset( ps_Param->Param, &si32_ListOrder, &b_Abbr );
	if( False == b_Ret ) {
		// パラメータ不正の場合.
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		return WAPD_AnyError;
	}
	if( True == b_Abbr ) {
		// "list_offset"省略.
		si32_ListOrder = 0;
	}
	
	b_Ret = cp_WAPC_BoxCommonLib->getRequestParam_USBSort(ps_Param->Param, &bl_Sort);
	if( False == b_Ret ) {
		// パラメータ不正の場合.
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		return WAPD_AnyError;
	}
	pc_DataMng->setSortType(bl_Sort);
	
	b_Ret = cp_WAPC_BoxCommonLib->getRequestParam_USBFolder(ps_Param->Param, &sc_Folderpath[0]);
	if( False == b_Ret ) {
		// パラメータ不正の場合.
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
		return WAPD_AnyError;
	}

	pc_DataMng->setFolderpath(sc_Folderpath);
	
// Comment 2019/04/16 【IT6_EagleH】[Klock Work] HCLT-Suribabu[ID:122304][PRECISION.LOSS.CALL].
//	getUSBFileList(si32_ListOrder);
	getUSBFileList(static_cast<short>(si32_ListOrder));
	
	m_ResponseHeader->Async = WAPD_ASYNC_ENABLE;

	m_ResponseHeader->Status = WAPD_STATUS_OK;
	
	return WAPD_NoError;
}

/**
 * @brief		POST(非同期レスポンス)処理.
 * @param[in]	ps_Param HTTPリクエストヘッダ、ボディ.
 * @return		処理結果, WAPD_NoError(0)：成功、WAPD_AnyError(1)：失敗.
 * @author		
   @since		2018/12/27.		
 * @date		
 */
int32_t WAPC_GetUsbFileList::ExecPostResponse( const WAPS_SubController* ps_Param )
{
	if( NULL == ps_Param ) {
		WAPG_API_trace("[ERR]%s NULL Parameter!.\n", __PRETTY_FUNCTION__ );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
		return WAPD_AnyError;
	}
	
	setUSBFileList();
	
	return WAPD_NoError;
}


/**
 * @brief			USBファイル取得処理.
 * @param[in]		
 * @return			none.
 * @author		
   @since		2018/12/27.		
 */
void WAPC_GetUsbFileList::getUSBFileList( short ss_offset )
{	
	WAPC_GetUsbFileList_DataMng*	pc_DataMng = WAPC_GetUsbFileList_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetUsbFileList_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
// Comment 2019/04/25 【HCL-Suribabu】【IT6_EagleH] Klockwork Changes 【ID:122288】[VOIDRET].
//		return WAPD_AnyError;
		return;
	}
	WAPC_AsyncResponse* pc_AsyncResponse = new WAPC_AsyncResponse;
	if( NULL == pc_AsyncResponse ) {
		WAPG_API_trace("[ERR] WAPC_GetUsbFileList::getUSBFileList() new WAPC_AsyncResponse() is NULL!.\n");
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
// Comment 2019/04/25 【HCL-Suribabu】【IT6_EagleH] Klockwork Changes 【ID:122289】[VOIDRET].
//		return WAPD_AnyError;
		return;
	}
	pc_AsyncResponse->setHandle( m_pv_Handle );
	pc_AsyncResponse->setReqId( reinterpret_cast<uint64_t>(WAPC_GetUsbFileList::ansUSBGetDirEntry) );
	WAPC_AsyncResponseContainer::newInstance()->push( pc_AsyncResponse );

	APIC_USBPrintIf* pc_USBPrintIf = APIC_USBPrintIf::newInstance();
	if( NULL != pc_USBPrintIf ){
		pc_USBPrintIf->initUSBFileNameTable();		// USBモジュールに対して初期化要求.
	}
	
	TYPS_DirEntryInfo* s_mlocalinfo = pc_DataMng->getlocalinfo();
	
	WAPS_DirEntryUSBTbl* s_USBEntryTbl = pc_DataMng->getUSBTbl();
	if( NULL == s_USBEntryTbl ) {
// Comment 2019/04/25 【HCL-Suribabu】【IT6_EagleH] Klockwork Changes 【ID:122290】[VOIDRET].
//		return WAPD_AnyError;
		return;
	}
	
	pc_DataMng->setNowListCnt(0);
	// 2020/06/26 [IT6_2.0MR3] [OP_BTS-24543] HCL-Gayathri Added.	
	pc_DataMng->setFileNotFoundListCnt(0);
	pc_DataMng->setMaxList(0);
	TYPS_UsbMemAddress st_UsbMemAddress;
	//memset( st_UsbMemAddress, '\0', sizeof( st_UsbMemAddress ) );	
	st_UsbMemAddress = pc_DataMng->getUsbMemAddress();
	
	short ss_MaxListCnt = APIF_SaaNumOfDirEntryUsb( TYPE_CharSetUTF8, st_UsbMemAddress.sc_DeviceName );		// 指定したファイル数を取得.
	pc_DataMng->setMaxList(ss_MaxListCnt); // 2020/06/26 [IT6_2.0MR3] [OP_BTS-24543] HCL-Gayathri Added. 

	if ((ss_offset*100) > ss_MaxListCnt) {
		printf("400:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //400->ErrorStatus[WAPD_STATUS_BAD_REQUEST] in WAP_defs.h
		m_ResponseHeader->Status = WAPD_STATUS_BAD_REQUEST;
// Comment 2019/04/25 【HCL-Suribabu】【IT6_EagleH] Klockwork Changes 【ID:122291】[VOIDRET].
//		return WAPD_AnyError;
		return;
	}
	
	char	sc_dir[WAPD_MaxStrUsbMemDeviceName];
	memset( sc_dir, '\0', sizeof( sc_dir ) );
	strncpy( sc_dir, st_UsbMemAddress.sc_DeviceName, sizeof( sc_dir )-1 );		// s_mUsbMemAddress.sc_DeviceNameをコピー.
	
	pc_DataMng->setUsbMemAddress(st_UsbMemAddress);
	
	APIF_SaaGetDirEntryUsb( TYPE_CharSetUTF8, sc_dir, s_mlocalinfo, ss_offset*100, 100, &WAPC_GetUsbFileList::ansUSBGetDirEntry );
	
}
/**
 * @brief			ファイル名から拡張子を取得する.		
 * @return			none.	
   @since			2018/12/27.		
 */

void WAPC_GetUsbFileList::getExtension( char* sc_Extension, char* sc_fileName )
{
	short sTextLenMax = ( strlen( sc_fileName ) );
	short ss_LoopCnt, ss_LoopCnt2;
	for( ss_LoopCnt = sTextLenMax ; ss_LoopCnt > 0 ; ss_LoopCnt-- ) {
		if( sc_fileName[ss_LoopCnt] == '.' ) {
			for( ss_LoopCnt2 = 0 ; ss_LoopCnt2 < WAPD_USB_File_ExtensionSize ; ss_LoopCnt2++ ) {
				sc_Extension[ss_LoopCnt2] = sc_fileName[ss_LoopCnt];
				if( ss_LoopCnt != sTextLenMax ) {
					ss_LoopCnt++;
				} else {
					break;
				}
			}
			for( ss_LoopCnt2 = 0 ; ss_LoopCnt2 < ( uchar )( strlen( sc_Extension ) ) ; ss_LoopCnt2++ ) {
				sc_Extension[ss_LoopCnt2] = ( char )tolower( sc_Extension[ss_LoopCnt2] );
			}
			ss_LoopCnt = 0;
		}
	}
}
/**
 * @brief			 時間文字列取得.		
 * @return			none.	
 * @since			2018/12/27.		
 */
void WAPC_GetUsbFileList::getDayOrTimeDisp( char* sc_DateTime, uchar uc_strLen, time_t* ptime_Time )
{
	char sc_Tmp_DateTime[WAPD_WebUIUSB_File_DateTimeSize] = {0};
	struct tm st_Time;
	memset( &st_Time, '\0', sizeof( st_Time ) );
	if( ERROR == SYSF_localtime_r( ptime_Time, &st_Time ) ) {
// Comment 2019/04/25 【HCL-Suribabu】【IT6_EagleH] Klockwork Changes 【ID:122292】[VOIDRET].
//		return WAPD_AnyError;
		return;
	}
	// 現在の仕向けを判定.
	APIC_MachineSetting*	pc_MachineSetting   = APIC_MachineSetting::newInstance();
		switch( pc_MachineSetting->getSystemType() ) {
		case TYPE_SystemType_US: 
		snprintf( sc_Tmp_DateTime, sizeof(sc_Tmp_DateTime),"%02d/%02d/%04d %02d:%02d", ( st_Time.tm_mon + 1 ), st_Time.tm_mday , (st_Time.tm_year+1900) , st_Time.tm_hour, st_Time.tm_min );	
		
		break;	// US仕向け..
		
		case TYPE_SystemType_JP: 
		snprintf( sc_Tmp_DateTime, sizeof(sc_Tmp_DateTime),"%04d/%02d/%02d %02d:%02d", (st_Time.tm_year+1900) , ( st_Time.tm_mon + 1 ), st_Time.tm_mday , st_Time.tm_hour, st_Time.tm_min );
		break;	// 日本仕向け.
		
		case TYPE_SystemType_EU:																					// ＥＵ仕向け.
		default: 				 snprintf( sc_Tmp_DateTime, sizeof(sc_Tmp_DateTime), "%02d/%02d/%04d %02d:%02d", st_Time.tm_mday, ( st_Time.tm_mon + 1 ) , (st_Time.tm_year+1900) , st_Time.tm_hour, st_Time.tm_min );	
		break;		// その他仕向け
	}
	strncpy( sc_DateTime, sc_Tmp_DateTime, uc_strLen );
}




/**
 * @brief			 USBファイルリストの設定..		
 * @return			none.	
   @since			2018/12/27.		
 */
void WAPC_GetUsbFileList::setUSBFileList()
{
	
	WAPC_GetUsbFileList_DataMng*	pc_DataMng = WAPC_GetUsbFileList_DataMng::newInstance();
	if( NULL == pc_DataMng ) {
		WAPG_API_trace("[ERR]%s WAPC_GetUsbFileList_DataMng::newInstance() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
// Comment 2019/04/25 【HCL-Suribabu】【IT6_EagleH] Klockwork Changes 【ID:122293】[VOIDRET].
//		return WAPD_AnyError;
		return;
	}
	pc_DataMng->sortUSBTbl();
	
	WAPS_DirEntryUSBTbl* s_USBEntryTbl = pc_DataMng->getUSBTbl();
//	char sc_DateTime[ WAPD_WebUIUSB_File_DateTimeSize + 2 ];				// 登録時間文字列.
	WAPD_USBFileExtension e_Extension = WAPD_USBFileExtension_Folder;					// 拡張子.
	char sc_Extension[WAPD_WebUIUSB_File_ExtensionSize];								// 拡張子文字列
//	char sc_FileSize[20+1];																// サイズ
	struct USBExtensionTable{
		char* sc_FileExtention;
		WAPD_USBFileExtension e_ExtensionType;
	} st_FileNameExtent[WAPD_ScanDirectUSBReferemce_FileNameExtentTypeMax] = {
		{ ".pdf"	,			WAPD_USBFileExtension_pdf	},
		{ ".jpg"	,			WAPD_USBFileExtension_jpg	},
		{ ".jpeg"	,			WAPD_USBFileExtension_jpeg	},
		{ ".tiff"	,			WAPD_USBFileExtension_tiff	},
		{ ".tif"	,			WAPD_USBFileExtension_tif	},
		{ ".xps"	,			WAPD_USBFileExtension_xps	},
		{ ".docx"	,			WAPD_USBFileExtension_docx	},
		{ ".xlsx"	,			WAPD_USBFileExtension_xlsx	},
		{ ".pptx"	,			WAPD_USBFileExtension_pptx	},
		{ ".ppml"	,			WAPD_USBFileExtension_ppml	},
		{ ".vdx"	,			WAPD_USBFileExtension_vdx	},
		{ ".zip"	,			WAPD_USBFileExtension_zip	}
	};
	TYPS_UsbMemAddress st_UsbMemAddress;
	st_UsbMemAddress = pc_DataMng->getUsbMemAddress();
			
	m_Response = new Json::Value;
	if( NULL == m_Response ) {
		WAPG_API_trace("[ERR]%s new Json::Value() is NULL!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
		// インスタンス破棄.
		//pc_Doc->deleteInstance();
		// クライアントへ結果返却.
		m_ResponseHeader->Status = WAPD_STATUS_INTERNAL_SERVER_ERROR;
		printf("500:: File:%s, Line:%d, Function:%s \n",  __FILE__, __LINE__, __func__); //500->ErrorStatus[WAPD_STATUS_INTERNAL_SERVER_ERROR] in WAP_defs.h
// Comment 2019/04/25 【HCL-Suribabu】【IT6_EagleH] Klockwork Changes 【ID:122294】[VOIDRET].
//		return WAPD_AnyError;
		return;
	}
	
	//IT issue
	(*m_Response)[WAPD_RespBodyParamGetUsbFileList_list_offset] = pc_DataMng->getListOffset();
	(*m_Response)[WAPD_RespBodyParamGetUsbFileList_list_cnt] = pc_DataMng->getNowListCnt();
	(*m_Response)[WAPD_RespBodyParamGetUsbFileList_folderpath] = st_UsbMemAddress.sc_DeviceName;
	
	//IT issue
	
	
	Json::Value& cResult = ( *m_Response )[WAPD_RespBodyParamGetUsbFileList_DocData];
	cResult[WAPD_RespBodyParamGetUsbFileList_DocListSize] = pc_DataMng->getNowListCnt();	// 文書リスト数をレスポンスボディにセット.
	Json::Value& cDocList = cResult[WAPD_RespBodyParamGetUsbFileList_DocList];
		
	for( short ss_i = 0; ss_i <pc_DataMng->getNowListCnt() ; ss_i++ ){


//		memset( sc_DateTime, '\0', sizeof( sc_DateTime ) );
//		getDayOrTimeDisp( sc_DateTime, ( WAPD_WebUIUSB_File_DateTimeSize ), &s_USBEntryTbl[ss_i].st_info.s_stat.st_mtime );
		
	
		if( s_USBEntryTbl[ss_i].st_info.s_stat.st_mode & S_IFDIR ){

			e_Extension = WAPD_USBFileExtension_Folder;
		} else {

			memset( sc_Extension, '\0', sizeof( sc_Extension ) );
			getExtension( sc_Extension, s_USBEntryTbl[ss_i].st_info.sc_name );		
			for( short ss_j = 0; ss_j < WAPD_ScanDirectUSBReferemce_FileNameExtentTypeMax; ss_j++ ) {
				if( strcmp( sc_Extension, st_FileNameExtent[ss_j].sc_FileExtention ) == 0 ) {		
					e_Extension = st_FileNameExtent[ss_j].e_ExtensionType;
					break;
				}	
			}
		}

		// ikari [S]
//		int32_t i32_USBDetailSize = static_cast<int32_t>( s_USBEntryTbl[ss_i].st_info.s_stat.st_size / 1024 );
		int32_t i32_USBDetailSize = s_USBEntryTbl[ss_i].st_info.s_stat.st_size;
		// ikari [E]

//		if( 0 != ( s_USBEntryTbl[ss_i].st_info.s_stat.st_size % 1024 ) ) {
//			i32_USBDetailSize = i32_USBDetailSize + 1;
//		}

//		memset( sc_FileSize, '\0', sizeof( sc_FileSize ) );
//		snprintf( sc_FileSize, sizeof(sc_FileSize), "%dKB", i32_USBDetailSize );
		
			
			cDocList[ss_i][WAPD_RespBodyParamGetUsbFileList_DocName]	= (s_USBEntryTbl[ss_i].st_info.sc_name);
			cDocList[ss_i][WAPD_RespBodyParamGetUsbFileList_DocDate]	= static_cast<int32_t>(s_USBEntryTbl[ss_i].st_info.s_stat.st_mtime);
			
			switch(e_Extension) 
			{
			case WAPD_USBFileExtension_Folder:
			cDocList[ss_i][WAPD_RespBodyParamGetUsbFileList_FileFormat] = WAPD_GetUsbFileListType_folder;
					break;
			case WAPD_USBFileExtension_pdf:
			cDocList[ss_i][WAPD_RespBodyParamGetUsbFileList_FileFormat] = WAPD_GetUsbFileListType_pdf;
					break;
			case WAPD_USBFileExtension_jpg:
			cDocList[ss_i][WAPD_RespBodyParamGetUsbFileList_FileFormat] = WAPD_GetUsbFileListType_jpg;
					break;
			case WAPD_USBFileExtension_jpeg:
			cDocList[ss_i][WAPD_RespBodyParamGetUsbFileList_FileFormat] = WAPD_GetUsbFileListType_jpeg;
					break;
			case WAPD_USBFileExtension_tiff:
			cDocList[ss_i][WAPD_RespBodyParamGetUsbFileList_FileFormat] = WAPD_GetUsbFileListType_tiff;
				break;
			case WAPD_USBFileExtension_tif:
			cDocList[ss_i][WAPD_RespBodyParamGetUsbFileList_FileFormat] = WAPD_GetUsbFileListType_tif;
					break;
			case WAPD_USBFileExtension_xps:
			cDocList[ss_i][WAPD_RespBodyParamGetUsbFileList_FileFormat] = WAPD_GetUsbFileListType_xps;
					break;
			case WAPD_USBFileExtension_docx:
			cDocList[ss_i][WAPD_RespBodyParamGetUsbFileList_FileFormat] = WAPD_GetUsbFileListType_docx;
					break;
			case WAPD_USBFileExtension_xlsx:
			cDocList[ss_i][WAPD_RespBodyParamGetUsbFileList_FileFormat] = WAPD_GetUsbFileListType_xlsx;
					break;
			case WAPD_USBFileExtension_pptx:
			cDocList[ss_i][WAPD_RespBodyParamGetUsbFileList_FileFormat] = WAPD_GetUsbFileListType_pptx;
					break;
			case WAPD_USBFileExtension_ppml:
			cDocList[ss_i][WAPD_RespBodyParamGetUsbFileList_FileFormat] = WAPD_GetUsbFileListType_ppml;
					break;		
			case WAPD_USBFileExtension_vdx:
			cDocList[ss_i][WAPD_RespBodyParamGetUsbFileList_FileFormat] = WAPD_GetUsbFileListType_vdx;
					break;
			case WAPD_USBFileExtension_zip:
			cDocList[ss_i][WAPD_RespBodyParamGetUsbFileList_FileFormat] = WAPD_GetUsbFileListType_zip;
					break;
		
			}
		cDocList[ss_i][WAPD_RespBodyParamGetUsbFileList_FileSize]	= i32_USBDetailSize;
	}
	
	m_ResponseHeader->Status = WAPD_STATUS_OK;
	m_ResponseHeader->ContentType       = const_cast<char *>("application/json");
	m_ResponseHeader->ContentTypeLength = static_cast<uint32_t>(strlen(m_ResponseHeader->ContentType));

	// 状態遷移[→初期状態].
	pc_DataMng->setStatus( WAPE_USBFileListStatus_Initialize );
}

//-----------------------------------------------------------------------------.
// WAPC_GetUsbFileList_DataMngクラス.
//	GetUsbFileList個別コントローラとコールバック処理間で共有するデータを格納する.
//-----------------------------------------------------------------------------.
/**
 * @brief		コンストラクタ.
 * @return		なし.
 * @author		日立 田中.
   @since		2018/12/27.	
 */
WAPC_GetUsbFileList_DataMng::WAPC_GetUsbFileList_DataMng()
{
// Comment 2019/04/25 【IT6_EagleH】[Klock Work] HCLT-Suribabu[ID:122282][UNINIT.CTOR.MUST]
	l_mListGetFlg = 0;
// 2019/04/25 【IT6_EagleH】【Klock work ID:122312 [UNINIT.CTOR.MUST]】 HCLT-Vijayaraghavan Modification Start.
	ss_mNowListCnt = 0;
	// 2020/06/26 [IT6_2.0MR3] [OP_BTS-24543] HCL-Gayathri Added start.	
	ss_mMaxList = 0;
	ss_mFileNotFoundListCnt = 0;
	// 2020/06/26 [IT6_2.0MR3] [OP_BTS-24543] HCL-Gayathri Added end.	
// 2019/04/25 【IT6_EagleH】【Klock work ID:122312 [UNINIT.CTOR.MUST]】 HCLT-Vijayaraghavan Modification End.	
// 2019/05/03 【IT6_EagleH】【Klock work ID:122307 [UNINIT.CTOR.MUST]】 HCLT-Vijayaraghavan Modification Start.	
	bl_Sort = False;
// 2019/05/03 【IT6_EagleH】【Klock work ID:122307 [UNINIT.CTOR.MUST]】 HCLT-Vijayaraghavan Modification End.
	// セマフォ生成.
	m_SemID = SYSF_semMCreate( SEM_Q_PRIORITY | SEM_DELETE_SAFE | SEM_INVERSION_SAFE );
	if ( NULL == m_SemID ) {
		WAPG_API_trace("[ERR]%s SYSF_semMCreate() Error!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
	}

	// 管理情報クリア.
	m_e_Status = WAPE_USBFileListStatus_Initialize;
}

/**
 * @brief		デストラクタ.
 * @return		なし.
 * @author		日立 田中.
   @since		2018/12/27.	
 */
WAPC_GetUsbFileList_DataMng::~WAPC_GetUsbFileList_DataMng()
{
	// セマフォ削除.
	if ( NULL != m_SemID ) {
		SYSF_semDelete(m_SemID) ;
	}
}

/**
 * @brief		インスタンス取得.
 * @return		なし.
 * @author		日立 田中.
   @since		2018/12/27.	
 */
WAPC_GetUsbFileList_DataMng* WAPC_GetUsbFileList_DataMng::newInstance()
{
	static WAPC_GetUsbFileList_DataMng c_WAPC_GetUsbFileList_DataMng;
	return &c_WAPC_GetUsbFileList_DataMng;
}

/**
 * @brief		
 * @param[in]	
 * @return 			無し.
 * @author		
   @since		2018/12/27.		
 */
void WAPC_GetUsbFileList_DataMng::setStatus( WAPE_USBFileListStatus e_Status )
{
	m_e_Status = e_Status;

}

/**
 * @brief			 時間文字列取得.		
 * @return			none.	
   @since			2018/12/27.			
 */
WAPE_USBFileListStatus WAPC_GetUsbFileList_DataMng::getStatus( )
{
	return m_e_Status;
}


/**
 * @brief			localinfoを取得する	
   @since			2018/12/27.		
 */
TYPS_DirEntryInfo* WAPC_GetUsbFileList_DataMng::getlocalinfo()
{

	return mlocalinfo;
}

/**
 * @brief		s_UsbMemAddressを取得する	
   @since		2018/12/27.		
 */
TYPS_UsbMemAddress WAPC_GetUsbFileList_DataMng::getUsbMemAddress()
{
	return s_mUsbMemAddress;
}
/**
 * @brief		s_UsbMemAddressを設定する
 * @param[in]	s_UsbMemAddress			USB情報格納先アドレス値
 * @return 			無し.	
 * @since		2018/12/27.		
 */

void WAPC_GetUsbFileList_DataMng::setUsbMemAddress(TYPS_UsbMemAddress s_UsbMemAddress){
	s_mUsbMemAddress = s_UsbMemAddress;	
}
/**
 * @brief		return the list of the files.	
 * @since		2018/12/27.		
 */
int32_t WAPC_GetUsbFileList_DataMng::getListOffset()
{

	return l_mListGetFlg;
}

/**
 * @brief		USBTblを取得する.
   @since		2018/12/27.		
 */
WAPS_DirEntryUSBTbl* WAPC_GetUsbFileList_DataMng::getUSBTbl( )	
{
	return st_mDirEntryTbl;
}
/*void WAPC_GetUsbFileList_DataMng::setListOffset( int32_t l_ListGetFlg )
{
	l_mListGetFlg = l_ListGetFlg;
}*/

/**
 * @brief		ソートタイプを設定する.
 * @return 			無し.
   @since		2018/12/27.		
 */

void WAPC_GetUsbFileList_DataMng::setSortType( Bool bl_SortType )
{
	bl_Sort = bl_SortType;
}
/*Bool WAPC_GetUsbFileList_DataMng::getSortType()
{
	return bl_Sort;
}*/

/**
 * @brief			ソートタイプを設定する.
 * @param[in]		Bool b_SortType : ソートタイプ.
 * @return 			無し.
 * @author		
   @since		2018/12/27.		
 */
void WAPC_GetUsbFileList_DataMng::setFolderpath( char* dir )
{
	APIC_UsbMemDeviceInfo* pc_UsbMemDeviceInfo = APIC_UsbMemDeviceInfo::newInstance();
	TYPS_UsbMemAddress st_UsbMemAddress;
	if( NULL != pc_UsbMemDeviceInfo ){
		st_UsbMemAddress = pc_UsbMemDeviceInfo->getUsbMemAddress();
	} else {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
// Comment 2019/04/25 【HCL-Suribabu】【IT6_EagleH] Klockwork Changes 【ID:122295】[VOIDRET].
//		return False;
		return;
	}
	char sc_dir[WAPD_MaxStrUsbMemDeviceName+1];
	memset( sc_dir, '\0', sizeof( sc_dir ) );
	
	if (dir[0] != '\0') {
		strncpy( sc_dir, st_UsbMemAddress.sc_DeviceName, sizeof( sc_dir )-1 );
		if (dir[0] != '/') {
			strncat( sc_dir, "/" , ( sizeof( sc_dir ) - strlen( sc_dir ) ) - 1 );
		}
		strncat( sc_dir, dir, ( sizeof( sc_dir ) - 1 ) );	// 余っている領域から終端文字を引いた文字数を連結.
		memset( st_UsbMemAddress.sc_DeviceName, '\0', sizeof( st_UsbMemAddress.sc_DeviceName ) );
		strncpy( st_UsbMemAddress.sc_DeviceName, sc_dir, sizeof( st_UsbMemAddress.sc_DeviceName ) - 1 );
	} else {
		strncpy( st_UsbMemAddress.sc_DeviceName, dir, sizeof( st_UsbMemAddress.sc_DeviceName )-1 );	
	}
	
	setUsbMemAddress(st_UsbMemAddress);

}

/**
 * @brief		USBTblをソートする.	
   @since		2018/12/27.		
 */
void WAPC_GetUsbFileList_DataMng::sortUSBTbl()
{
	Bool b_Swapped = True;		// ソートフラグ.
	short ss_MatchCount  = 0;
	short ss_NowListCnt = getNowListCnt();		// 表示できるUSBのリスト数.
	WAPS_DirEntryUSBTbl tempUSBInfo;			// 一時格納構造体.
	WAPS_DirEntryUSBTbl* st_SortDirEntryTbl = getUSBTbl();
	while( b_Swapped ) {
		b_Swapped = False;
		ss_MatchCount++;
		for( short ss_Iterate = 0; ss_Iterate < ss_NowListCnt - ss_MatchCount; ss_Iterate++ ) {
			if( ( True == bl_Sort  && ( st_SortDirEntryTbl[ss_Iterate].st_info.s_stat.st_mtime > st_SortDirEntryTbl[ss_Iterate + 1].st_info.s_stat.st_mtime ) )		// 昇順ソート.
			    || ( False == bl_Sort && ( st_SortDirEntryTbl[ss_Iterate].st_info.s_stat.st_mtime < st_SortDirEntryTbl[ss_Iterate + 1].st_info.s_stat.st_mtime ) ) ) {	// 降順ソート.
				memset( &tempUSBInfo, '\0', sizeof( tempUSBInfo ) );
				memcpy( &tempUSBInfo, &st_SortDirEntryTbl[ss_Iterate], sizeof( st_SortDirEntryTbl[ss_Iterate] ) );
				memset( &st_SortDirEntryTbl[ss_Iterate], '\0', sizeof( st_SortDirEntryTbl[ss_Iterate] ) );
				memcpy( &st_SortDirEntryTbl[ss_Iterate], &st_SortDirEntryTbl[ss_Iterate+1], sizeof( st_SortDirEntryTbl[ss_Iterate+1] ) );
				memset( &st_SortDirEntryTbl[ss_Iterate+1], '\0', sizeof( st_SortDirEntryTbl[ss_Iterate+1] ) );
				memcpy( &st_SortDirEntryTbl[ss_Iterate+1], &tempUSBInfo, sizeof( tempUSBInfo ) );
				b_Swapped = True;
			}
		}
	}
}

/**
 * @brief		ss_NowListCntを取得する.	
   @since		2018/12/27.		
 */
short WAPC_GetUsbFileList_DataMng::getNowListCnt()
{
	if( ss_mNowListCnt > WAPD_SelectDocListUSBTblMAX ) {
		ss_mNowListCnt = WAPD_SelectDocListUSBTblMAX;
	}

	return ss_mNowListCnt;
}
/**
 * @brief		ss_NowListCntを設定する.	
 * @param[in]	ss_NowListCnt			使用可能なファイル、フォルダの件数
   @since		2018/12/27.		
 */

void WAPC_GetUsbFileList_DataMng::setNowListCnt(short count)
{
	ss_mNowListCnt = count;
}
// 2020/06/26 [IT6_2.0MR3] [OP_BTS-24543] HCL-Gayathri Added Start. 
short WAPC_GetUsbFileList_DataMng::getMaxList(){
	return ss_mMaxList;
}
	
void WAPC_GetUsbFileList_DataMng::setMaxList(short count){
	ss_mMaxList = count;
}

short WAPC_GetUsbFileList_DataMng::getFileNotFoundListCnt(){
	return ss_mFileNotFoundListCnt;
}
	
void WAPC_GetUsbFileList_DataMng::setFileNotFoundListCnt(short count){
	ss_mFileNotFoundListCnt = count;
}
// 2020/06/26 [IT6_2.0MR3] [OP_BTS-24543] HCL-Gayathri Added End. 
	
/**
 * @brief		アクセスセマフォ取得.
 * @param[in]	なし.
 * @return 		なし.
 * @author		
 * @since		2018/12/27.
 */
void WAPC_GetUsbFileList_DataMng::semTake( void )
{
	STATUS ret = SYSF_semTake( m_SemID, WAIT_FOREVER );
	if ( ERROR == ret ) {
		WAPG_API_trace("[ERR]%s SYSF_semTake() Error!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
	}

	return;
}

/**
 * @brief		アクセスセマフォ開放.
 * @param[in]	なし.
 * @return 		なし.
 * @author		
 * @since		2018/12/27.
 */
void WAPC_GetUsbFileList_DataMng::semGive( void )
{
	STATUS ret = SYSF_semGive( m_SemID );
	if ( ERROR == ret ) {
		WAPG_API_trace("[ERR]%s SYSF_semGive() Error!.\n", __PRETTY_FUNCTION__ );
		// Fatal通知.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );
	}

	return;
}

