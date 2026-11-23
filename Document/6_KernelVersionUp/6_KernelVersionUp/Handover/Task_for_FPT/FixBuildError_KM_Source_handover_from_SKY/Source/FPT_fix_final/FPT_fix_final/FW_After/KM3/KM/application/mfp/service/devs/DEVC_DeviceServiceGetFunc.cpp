/**
 \file		DEVC_DeviceServiceGetFunc.cpp.
 \brief		デバイスサービス GetXXX関数.
 \author	.
 \date		2010/XX/XX		新規作成.
*/

//-----------------------------------------------------------------------------.
//インクルード・ファイル.
//-----------------------------------------------------------------------------.
#include "DEVC_DeviceService.h"
#include "system/prt/APIC_Printer.h"
#include "system/scn/APIC_Scanner.h"
#include "system/sys/APIC_NetworkDevice.h"
#include "system/sys/APIC_HddInfo.h"

// FWリファクタリング APIC_Setting.hの分割.
#include "system/nvd/APIC_Setting_MachineSetting.h"

#include "system/sys/APIC_KeyDevice.h"
#include "system/uac/APIC_AuthDeviceSetting.h"
#include "system/thr/thrcom/THRC_BSPif.h"		// DS基板.

/* 2012/05/30 S.Wakayama CQ:364359対応 Start */
#include "lib/filelib/SFLG_StorageFileLibraryIf.h"

#define DEVSRV_BULKDRV_NAMESZ	(20)					/* Max size of device name. */
#define DEVSRV_MOUNT_FILE		"/proc/mounts"			/* カーネルのマウント状態を確認するファイル. */
/* 2012/05/30 S.Wakayama CQ:364359対応 End */

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]デバイス接続情報取得処理.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：ul_DeviceId		[in] デバイスID.
//引数　　：ps_DevInfoNew	[out] デバイス接続情報.
//引数　　：ul_DetailSize	[in] デバイス詳細情報サイズ.
//戻り値　：デバイス接続情報.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
DEVS_DeviceInfo * DEVC_DeviceService::getDeviceInfoCommonGetDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, uint32_t ul_DeviceId, DEVS_DeviceInfo *ps_DevInfoNew, uint32_t ul_DetailSize)
{
	DEVS_DeviceInfo *ps_DevInfo = NULL;

	// パラメータチェック.
	if (ps_DevInfoNew != NULL) {

		switch (e_GetInfoType) {
		case DEVE_DeviceServiceGetInfoType_Init:		// 初期化時.
			// 初期化.
			memset((char *)ps_DevInfoNew, (char)DEVD_ZERO_OUT, sizeof(DEVS_DeviceInfo));
			ps_DevInfoNew->s_DeviceConnectInfo.ul_DeviceId = ul_DeviceId;

			// デバイス接続情報追加.
			DEVD_MAP_DEVICEINFO_INSERT(ul_DeviceId, *ps_DevInfoNew);

			// デバイス接続情報取得.
			ps_DevInfo = DEVD_MAP_DEVICEINFO_GET(ul_DeviceId);
			break;

		case DEVE_DeviceServiceGetInfoType_Polling:		// ポーリング時.
		case DEVE_DeviceServiceGetInfoType_DataChanged:	// 変化点通知時.
			// デバイス接続情報取得.
			ps_DevInfo = DEVD_MAP_DEVICEINFO_GET(ul_DeviceId);
			if (ps_DevInfo == NULL) { break; }

			// デバイス接続情報コピー.
			ps_DevInfoNew->s_DeviceConnectInfo.ul_DeviceId = ps_DevInfo->s_DeviceConnectInfo.ul_DeviceId;	// デバイス管理ID.
			ps_DevInfoNew->uc_DeviceConnect = ps_DevInfo->uc_DeviceConnect;	// 接続状態.
			memcpy(ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceDetailInfo, ps_DevInfo->s_DeviceConnectInfo.uc_DeviceDetailInfo, ul_DetailSize);	// デバイス詳細情報.
			ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed;	// デバイス使用状況.
			ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = ps_DevInfo->s_DeviceConnectInfo.uc_TemporaryRescueStatus;	// 暫定救済状態.
			break;

		default:
			break;
		}

	}
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//	if (ps_DevInfo == NULL) { APIC_ThrowSystemError(SYSE_SysErrGeneral, DEVD_DEBUG_EVENT_KIND, "DEVC_DeviceService getDeviceInfoCommonGetDeviceInfo() find error"); }
	if (ps_DevInfo == NULL) { APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrGeneral, DEVD_DEBUG_EVENT_KIND, "DEVC_DeviceService getDeviceInfoCommonGetDeviceInfo() find error", 0, createDetailCode(0x5010)); }
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.

	return ps_DevInfo;
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]デバイス接続情報送信,設定処理.
//引数　　：e_GetInfoType		[in] 接続情報取得種別.
//引数　　：ps_DevInfo			[out] デバイス接続情報.
//引数　　：ps_DevInfoNew		[in] デバイス接続情報.
//引数　　：ul_DetailSize		[in] デバイス詳細情報サイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getDeviceInfoCommonSendMessageToApPf(DEVE_DeviceServiceGetInfoType e_GetInfoType, DEVS_DeviceInfo *ps_DevInfo, const DEVS_DeviceInfo *ps_DevInfoNew, uint32_t ul_DetailSize)
{
	// パラメータチェック.
	if ((ps_DevInfo != NULL) && (ps_DevInfoNew != NULL)) {
		uint32_t ul_DeviceId = ps_DevInfoNew->s_DeviceConnectInfo.ul_DeviceId;	// デバイスID取得.

		if (e_GetInfoType != DEVE_DeviceServiceGetInfoType_Init) {

			DEVS_DeviceConnectInfo *ps_Info = (DEVS_DeviceConnectInfo *)&ps_DevInfo->s_DeviceConnectInfo;
			DEVS_DeviceConnectInfo *ps_InfoNew = (DEVS_DeviceConnectInfo *)&ps_DevInfoNew->s_DeviceConnectInfo;

			if (ps_DevInfoNew->uc_DeviceConnect == DEVD_DeviceConnectOff) {			// 切断状態の場合.

				if (ps_DevInfoNew->uc_DeviceConnect != ps_DevInfo->uc_DeviceConnect) {
					sendMessageToApPf(DEVE_NTF_DataID_Device_Disconnect, ul_DeviceId, ps_DevInfoNew);			// デバイス切断通知.
				}

			} else if (ps_DevInfoNew->uc_DeviceConnect == DEVD_DeviceConnectOn) {	// 接続状態の場合.

				unsigned char *puc_DetailInfo = ps_DevInfo->s_DeviceConnectInfo.uc_DeviceDetailInfo;
				const unsigned char *puc_DetailInfoNew = ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceDetailInfo;

				if ((ps_DevInfoNew->uc_DeviceConnect != ps_DevInfo->uc_DeviceConnect) || (memcmp(puc_DetailInfo, puc_DetailInfoNew, (size_t)ul_DetailSize) != 0)) {
					if (ul_DetailSize < DEVD_CONNECT_DETAILINFO_SIZE) {
						memset((void *)(puc_DetailInfoNew + ul_DetailSize), DEVD_ZERO_OUT, DEVD_CONNECT_DETAILINFO_SIZE - ul_DetailSize);
					}
					sendMessageToApPf(DEVE_NTF_DataID_Device_Connect, ul_DeviceId, ps_DevInfoNew);				// デバイス接続通知.
				} else {
					if (ps_InfoNew->uc_TemporaryRescueStatus != ps_Info->uc_TemporaryRescueStatus) {
						sendMessageToApPf(DEVE_NTF_DataID_Device_TemporaryRescue, ul_DeviceId, ps_DevInfoNew);	// 暫定救済状態変更通知.
					}
					if (ps_InfoNew->uc_DeviceUsed != ps_Info->uc_DeviceUsed) {
						sendMessageToApPf(DEVE_NTF_DataID_Device_Used, ul_DeviceId, ps_DevInfoNew);				// デバイス使用状況変更通知.
					}
				}

			}

		}

		// デバイス接続情報コピー.
		ps_DevInfo->uc_DeviceConnect = ps_DevInfoNew->uc_DeviceConnect;	// 接続状態.
		memcpy(ps_DevInfo->s_DeviceConnectInfo.uc_DeviceDetailInfo, ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceDetailInfo, ul_DetailSize);	// デバイス詳細情報.
		ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed;	// デバイス使用状況.
		ps_DevInfo->s_DeviceConnectInfo.uc_TemporaryRescueStatus = ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus;	// 暫定救済状態.
	}
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数] トレイ接続情報共通項目取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitTrayDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, uint32_t ul_DeviceId)
{
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew, sizeof(DEVS_DeviceDetailInfo_Tray));	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;
		DEVS_DeviceDetailInfo_Tray *ps_DetailNew = (DEVS_DeviceDetailInfo_Tray *)ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceDetailInfo;

		ps_DetailNew->uc_TrayType = getTrayType(ul_DeviceId);	// トレイ種別取得.
		if (ps_DetailNew->uc_TrayType == DEVD_TrayTypeNon) {
			ps_DevInfoNew->uc_DeviceConnect = DEVD_DeviceConnectOff;	// 未接続.
		} else {
			ps_DevInfoNew->uc_DeviceConnect = DEVD_DeviceConnectOn;		// 接続.
		}
		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = getDeviceUsed_Unit(ul_DeviceId);	// デバイス使用状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, sizeof(DEVS_DeviceDetailInfo_Tray)); // デバイス接続情報送信,設定処理.
	}
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]トレイ1接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitTray1DeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	getUnitTrayDeviceInfo(e_GetInfoType, DEVD_DeviceID_Tray1);	// トレイ1.

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]トレイ2接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitTray2DeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	getUnitTrayDeviceInfo(e_GetInfoType, DEVD_DeviceID_Tray2);	// トレイ2.

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]トレイ3接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitTray3DeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	getUnitTrayDeviceInfo(e_GetInfoType, DEVD_DeviceID_Tray3);	// トレイ3.

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]トレイ4接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitTray4DeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	getUnitTrayDeviceInfo(e_GetInfoType, DEVD_DeviceID_Tray4);	// トレイ4.

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

// DenebMLK:機器構成変更_No.7_5段増設プリント機能追加対応 start.
//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]トレイ5接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitTray5DeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	getUnitTrayDeviceInfo(e_GetInfoType, DEVD_DeviceID_Tray5);	// トレイ5.

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}
// DenebMLK:機器構成変更_No.7_5段増設プリント機能追加対応 end.

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]LCT(大容量給紙ユニット)接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitTrayLCTDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_TrayLCT;	// 大容量給紙ユニット（LCT）.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew, sizeof(DEVS_DeviceDetailInfo_TrayLCT));	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;
		DEVS_DeviceDetailInfo_TrayLCT *ps_DetailNew = (DEVS_DeviceDetailInfo_TrayLCT *)ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceDetailInfo;

		ps_DetailNew->uc_LCTType = getLCTType(ul_DeviceId);	// LCTトレイ種別取得.
		if (ps_DetailNew->uc_LCTType == (DEVD_LCTType)DEVD_TypeNotSet) {
			ps_DevInfoNew->uc_DeviceConnect = DEVD_DeviceConnectOff;	// 未接続.
		} else {
			ps_DevInfoNew->uc_DeviceConnect = DEVD_DeviceConnectOn;		// 接続.
		}
		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = getDeviceUsed_Unit(ul_DeviceId);	// デバイス使用状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, sizeof(DEVS_DeviceDetailInfo_TrayLCT)); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]手差しトレイ接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitTrayManualDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	getUnitTrayDeviceInfo(e_GetInfoType, DEVD_DeviceID_TrayManual);	// 手差しトレイ.

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]PIトレイ1接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitTrayPI1DeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	getUnitTrayDeviceInfo(e_GetInfoType, DEVD_DeviceID_TrayPI1);	// PIトレイ1.

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]PIトレイ2接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitTrayPI2DeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	getUnitTrayDeviceInfo(e_GetInfoType, DEVD_DeviceID_TrayPI2);	// PIトレイ2.

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]PI(カバーシート装置)接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitPIDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_PI;	// PI（カバーシート装置）.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		ps_DevInfoNew->uc_DeviceConnect = getDeviceConnect_Unit(ul_DeviceId);	// デバイス接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = getDeviceUsed_Unit(ul_DeviceId);	// デバイス使用状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]PK(パンチキット)接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitPKDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_PK;	// PK（パンチキット）.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew, sizeof(DEVS_DeviceDetailInfo_Punch));	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		getPunchInfo(ul_DeviceId, (DEVS_DeviceDetailInfo_Punch *)ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceDetailInfo);	// PUデバイス詳細情報取得.
		ps_DevInfoNew->uc_DeviceConnect = getDeviceConnect_Unit(ul_DeviceId);	// デバイス接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = getDeviceUsed_Unit(ul_DeviceId);	// デバイス使用状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, sizeof(DEVS_DeviceDetailInfo_Punch)); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]PZ(Z折り、パンチ処理装置)接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitPZDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_PZ;	// PZ（Z折り、パンチ処理装置）.
// 2019/04/03 APF OP_BTS-5372施策対応 Start.
	DEVS_DeviceInfo *ps_DevInfoNew = new DEVS_DeviceInfo;
	if(ps_DevInfoNew == NULL){
		errorLog_sysServiceDevs("ps_DevInfoNew is NULL\n");
		return;
	}
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, ps_DevInfoNew, sizeof(DEVS_DeviceDetailInfo_Punch));	// デバイス接続情報取得処理.
// 2019/04/03 APF OP_BTS-5372施策対応 End.
	if (ps_DevInfo != NULL) {

		getPunchInfo(ul_DeviceId, (DEVS_DeviceDetailInfo_Punch *)ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceDetailInfo);	// PUデバイス詳細情報取得.
		ps_DevInfoNew->uc_DeviceConnect = getDeviceConnect_Unit(ul_DeviceId);	// デバイス接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = getDeviceUsed_Unit(ul_DeviceId);	// デバイス使用状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, sizeof(DEVS_DeviceDetailInfo_Punch)); // デバイス接続情報送信,設定処理.
	}
// 2019/04/03 APF OP_BTS-5372施策対応 Start.	
	delete ps_DevInfoNew;
// 2019/04/03 APF OP_BTS-5372施策対応 End.

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]Folder(中折り、中綴じ、3つ折り装置)接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitFolderDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_Folder;	// 中折り、中綴じ、3つ折り装置.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew, sizeof(DEVS_DeviceDetailInfo_Folder));	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;
		DEVS_DeviceDetailInfo_Folder *ps_DetailNew = (DEVS_DeviceDetailInfo_Folder *)ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceDetailInfo;

		getFolderInfo(ul_DeviceId, ps_DetailNew);	// Folderデバイス詳細情報取得.
		if ((ps_DetailNew->uc_FoldZshaped == DEVD_BoolTrue) || (ps_DetailNew->uc_FoldCenterInner == DEVD_BoolTrue) || (ps_DetailNew->uc_FoldLetter == DEVD_BoolTrue)) {
			ps_DevInfoNew->uc_DeviceConnect = DEVD_DeviceConnectOn;		// 接続.
		} else {
			ps_DevInfoNew->uc_DeviceConnect = DEVD_DeviceConnectOff;	// 未接続.
		}
		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = getDeviceUsed_Unit(ul_DeviceId);	// デバイス使用状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, sizeof(DEVS_DeviceDetailInfo_Folder)); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]フィニッシャー接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitFinisherDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_Finisher;	// フィニッシャー.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew, sizeof(DEVS_DeviceDetailInfo_Finisher));	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;
		DEVS_DeviceDetailInfo_Finisher *ps_DetailNew = (DEVS_DeviceDetailInfo_Finisher *)ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceDetailInfo;

		ps_DetailNew->uc_FinisherType = getFinisherType(ul_DeviceId);	// Finisher種別取得.
		ps_DevInfoNew->uc_DeviceConnect = getDeviceConnect_Unit(ul_DeviceId);	// デバイス接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = getDeviceUsed_Unit(ul_DeviceId);	// デバイス使用状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, sizeof(DEVS_DeviceDetailInfo_Finisher)); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]RU(リレーユニット)接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitRUDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_RU;	// リレーユニット（RU）.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		ps_DevInfoNew->uc_DeviceConnect = getDeviceConnect_Unit(ul_DeviceId);	// デバイス接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = getDeviceUsed_Unit(ul_DeviceId);	// デバイス使用状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]ステープルユニット接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitStapleDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_Staple;	// ステープルユニット.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew, sizeof(DEVS_DeviceDetailInfo_Staple));	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;
		DEVS_DeviceDetailInfo_Staple *ps_DetailNew = (DEVS_DeviceDetailInfo_Staple *)ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceDetailInfo;

		getStapleInfo(ul_DeviceId, ps_DetailNew);	// Stapleデバイス詳細情報取得.
		if ((ps_DetailNew->uc_Staple1Corner == DEVD_BoolTrue) || (ps_DetailNew->uc_Staple2Side == DEVD_BoolTrue) || (ps_DetailNew->uc_Staple2SideCenter == DEVD_BoolTrue)) {
			ps_DevInfoNew->uc_DeviceConnect = DEVD_DeviceConnectOn;	// 接続.
		} else {
			ps_DevInfoNew->uc_DeviceConnect = DEVD_DeviceConnectOff;	// 未接続.
		}
		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = getDeviceUsed_Unit(ul_DeviceId);	// デバイス使用状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, sizeof(DEVS_DeviceDetailInfo_Staple)); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]ADF(原稿自動送り装置)接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitADFDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_ADF;	// ADF.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		ps_DevInfoNew->uc_DeviceConnect = getDeviceConnect_Unit(ul_DeviceId);	// デバイス接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = getDeviceUsed_Unit(ul_DeviceId);	// デバイス使用状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]IR(赤外分光装置)接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitIRDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_IR;	// IR.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		ps_DevInfoNew->uc_DeviceConnect = getDeviceConnect_Unit(ul_DeviceId);	// デバイス接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = getDeviceUsed_Unit(ul_DeviceId);	// デバイス使用状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]カラー現像器接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitColorDevDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_ColorDev;	// カラー現像器.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		ps_DevInfoNew->uc_DeviceConnect = getDeviceConnect_Unit(ul_DeviceId);	// デバイス接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = getDeviceUsed_Unit(ul_DeviceId);	// デバイス使用状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]FAXボード(FAX主回線)接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitFaxDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_Fax;	// FAXボード.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		ps_DevInfoNew->uc_DeviceConnect = getDeviceConnect_Unit(ul_DeviceId);	// デバイス接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = getDeviceUsed_Unit(ul_DeviceId);	// デバイス使用状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]FAX副回線接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitFaxSubDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_FaxSub;	// ＦＡＸ副回線.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		ps_DevInfoNew->uc_DeviceConnect = getDeviceConnect_Unit(ul_DeviceId);	// デバイス接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = getDeviceUsed_Unit(ul_DeviceId);	// デバイス使用状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]ネットワークデバイス接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitNetworkDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_Network;	// Network.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew, sizeof(DEVS_DeviceDetailInfo_Network));	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;
		DEVS_DeviceDetailInfo_Network *ps_DetailNew = (DEVS_DeviceDetailInfo_Network *)ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceDetailInfo;

		ps_DetailNew->uc_ChannelStatus = getChannelStatus(ul_DeviceId);	// ネットワーク状態取得.
		ps_DevInfoNew->uc_DeviceConnect = getDeviceConnect_Unit(ul_DeviceId);	// デバイス接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = getDeviceUsed_Unit(ul_DeviceId);	// デバイス使用状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, sizeof(DEVS_DeviceDetailInfo_Network)); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]IEEE1284接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitIEEE1284DeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_IEEE1284;	// IEEE1284.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew, sizeof(DEVS_DeviceDetailInfo_Network));	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;
		DEVS_DeviceDetailInfo_Network *ps_DetailNew = (DEVS_DeviceDetailInfo_Network *)ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceDetailInfo;

		ps_DetailNew->uc_ChannelStatus = getChannelStatus(ul_DeviceId);	// ネットワーク状態取得.
		ps_DevInfoNew->uc_DeviceConnect = getDeviceConnect_Unit(ul_DeviceId);	// デバイス接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = getDeviceUsed_Unit(ul_DeviceId);	// デバイス使用状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, sizeof(DEVS_DeviceDetailInfo_Network)); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]USB接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitUSBDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_USB;	// USB.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew, sizeof(DEVS_DeviceDetailInfo_Network));	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;
		DEVS_DeviceDetailInfo_Network *ps_DetailNew = (DEVS_DeviceDetailInfo_Network *)ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceDetailInfo;

		ps_DetailNew->uc_ChannelStatus = getChannelStatus(ul_DeviceId);	// ネットワーク状態取得.
		ps_DevInfoNew->uc_DeviceConnect = getDeviceConnect_Unit(ul_DeviceId);	// デバイス接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = getDeviceUsed_Unit(ul_DeviceId);	// デバイス使用状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, sizeof(DEVS_DeviceDetailInfo_Network)); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]プリンタコントローラ接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitPrinterControlerDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_PrinterControler;	// プリンタコントローラー.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		ps_DevInfoNew->uc_DeviceConnect = getDeviceConnect_Unit(ul_DeviceId);	// デバイス接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = getDeviceUsed_Unit(ul_DeviceId);	// デバイス使用状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]HDD接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//          14/04/22 Sky小川 HDDレス対応
/*============================================================================*/
void DEVC_DeviceService::getUnitHDDDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_HDD;	// HDD.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew, sizeof(DEVS_DeviceDetailInfo_HDD));	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;
		DEVS_DeviceDetailInfo_HDD *ps_DetailNew = (DEVS_DeviceDetailInfo_HDD *)ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceDetailInfo;

		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = getDeviceUsed_Unit(ul_DeviceId);	// デバイス使用状態取得.
		ps_DetailNew->uc_HddStatus = getHddStatus(ul_DeviceId);	// 状態取得.
		
		if (ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed == DEVD_DeviceUsedOn) {
			//	HDD接続時.
			ps_DetailNew->uc_StorageKind = DEVD_StorageKindHDD;
		} else {
			//	HDD非接続時.(HDDレス -> SSDが接続 )
			ps_DetailNew->uc_StorageKind = DEVD_StorageKindSSD;
		}
		ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOn;
// 2011.09.27 CIT不具合対応[No.220] Start.
//		if (ps_DetailNew->uc_HddStatus != DEVD_HddStatusNon) {
//			ps_DevInfoNew->uc_DeviceConnect = DEVD_DeviceConnectOn;
//		} else {
//			ps_DevInfoNew->uc_DeviceConnect = DEVD_DeviceConnectOff;
//		}
		ps_DevInfoNew->uc_DeviceConnect = DEVD_DeviceConnectOn;	// 常に接続状態とする(HDDの状態はuc_HddStatusで判定).
// 2011.09.27 CIT不具合対応[No.220] End.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, sizeof(DEVS_DeviceDetailInfo_HDD)); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]キーカウンタ接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitKeyCounterDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_KeyCounter;	// キーカウンター.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		getKeyCounterConnect(ul_DeviceId, ps_DevInfoNew);	// キーカウンタ接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]ベンダー(従来タイプ)接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitVendorDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_Vendor;	// ベンダー（従来タイプ）.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		getVendorConnect(ul_DeviceId, ps_DevInfoNew);	// ベンダー装置接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]ベンダー(統合タイプ)接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitVendorKMDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_VendorKM;	// ベンダー（統合タイプ）.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		getVendorConnect(ul_DeviceId, ps_DevInfoNew);	// ベンダー装置接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]管理装置(従来タイプ)接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitAdminMachineDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_AdminMachine;	// 管理装置（従来タイプ）.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		getAdminMachineConnect(ul_DeviceId, ps_DevInfoNew);	// 管理装置接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]管理装置(統合タイプ)接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitAdminMachineKMDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_AdminMachineKM;	// 管理装置（統合タイプ）.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		getAdminMachineConnect(ul_DeviceId, ps_DevInfoNew);	// 管理装置接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]ページアクセス接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitPageAcsesDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_PageAcses;	// ページアクセス.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		getVendorConnect(ul_DeviceId, ps_DevInfoNew);	// ベンダー装置接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]認証装置2(カード認証)接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitAuthDeviceDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_AuthDevice;	// 認証装置2.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		getAdminMachineConnect(ul_DeviceId, ps_DevInfoNew);	// 管理装置接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]ICカードリーダ接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitICCDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_AuthICC;	// ICカードリーダ.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		getAuthConnect(ul_DeviceId, ps_DevInfoNew);	// 認証装置接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]静脈リーダ接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitBIDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_AuthBIO;	// 静脈リーダ.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		getAuthConnect(ul_DeviceId, ps_DevInfoNew);	// 認証装置接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]ローダブルICカード認証装置接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitLodableICCDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_AuthLodable_ICC;	// ローダブルICカード認証データ.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		getAuthConnect(ul_DeviceId, ps_DevInfoNew);	// 認証装置接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]ローダブル生体認証装置接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitLodabeBIODeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_AuthLodable_BIO;	// ローダブル生体認証データ.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		getAuthConnect(ul_DeviceId, ps_DevInfoNew);	// 認証装置接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]静脈リーダ(AU-102)接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitBIO2DeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_AuthBIO2;	// 静脈リーダ(AU-102).
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		getAuthConnect(ul_DeviceId, ps_DevInfoNew);	// 認証装置接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}


//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]PKI認証接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitAuthPKIDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_AuthPKI;	// PKI認証.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (ps_DevInfo != NULL) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;

		getAuthConnect(ul_DeviceId, ps_DevInfoNew);	// 認証装置接続状態取得.
		ps_DevInfoNew->s_DeviceConnectInfo.uc_TemporaryRescueStatus = getTemporaryRescueStatus(ul_DeviceId);	// 暫定救済状態取得.

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE2, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]USB機器(USBバルク以外)接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitUSBDevDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, uint32_t ul_DeviceId ,const void *pv_Data, uint32_t ul_Size)
{
	do {
		if (e_GetInfoType == DEVE_DeviceServiceGetInfoType_Init) {	// 初期化時.

			// デバイス情報初期化.
			DEVS_DeviceInfo s_DevinfoNew;
			DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
			if (ps_DevInfo != NULL) {
				ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOn; // 接続設定が無い為、常にON設定とする.
				ps_DevInfo->s_DeviceConnectInfo.uc_TemporaryRescueStatus = DEVD_TemporaryRescueStatusReset; // 暫定救済不可の為、常に暫定救済していない状態を設定する.
			}
			break;
		}

		// 変化点通知時のみ実行.
		if (e_GetInfoType != DEVE_DeviceServiceGetInfoType_DataChanged) { break; }

		if ((pv_Data == NULL) || (ul_Size < sizeof(DEVS_UsbChangeData))) {
			DEVSRV_LOG(DEVD_LOGLEVEL_ERROR, "Bad Parameter! Nullpointer [pv_Data=%p, ul_Size=%ld]", pv_Data, ul_Size);
			break;
		}

		// デバイス情報取得.
		DEVS_DeviceInfo *ps_DevInfo = DEVD_MAP_DEVICEINFO_GET(ul_DeviceId);
		if (ps_DevInfo == NULL) {
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//			APIC_ThrowSystemError(SYSE_SysErrGeneral, DEVD_DEBUG_EVENT_KIND, "DEVC_DeviceService getUnitUSBDevDeviceInfo() find error");
			APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrGeneral, DEVD_DEBUG_EVENT_KIND, "DEVC_DeviceService getUnitUSBDevDeviceInfo() find error", 0, createDetailCode(0x5011));
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
			break;
		}
		DEVS_DeviceDetailInfo_Usb *ps_Detail = (DEVS_DeviceDetailInfo_Usb*)ps_DevInfo->s_DeviceConnectInfo.uc_DeviceDetailInfo;

		DEVS_UsbChangeData *ps_Data = (DEVS_UsbChangeData *)pv_Data;
		if (ps_Data->uc_AddDev == DEVD_BoolTrue) {			// 接続データ検知.

			if (ps_Detail->ul_devNum >= DEVD_USBDEVCONNECT_MAX) {
				DEVSRV_LOG(DEVD_LOGLEVEL_ERROR, "Bad Parameter! DeviceID max over [ul_devNum = %ld]", ps_Detail->ul_devNum);
				break; // 最大DEVD_USBDEVCONNECT_MAXまでしか管理しない.
			}

			bool b_IsAddFlag = true;
			for (uint32_t i = 0; i < ps_Detail->ul_devNum; i++) {
				if (strncmp(ps_Detail->s_associateInfo[i].sc_devName, ps_Data->sc_devName, DEVD_USBDEVNAME_MAXLENGTH) == 0) {
					b_IsAddFlag = false;
					DEVSRV_LOG(DEVD_LOGLEVEL_ERROR, "Bad Parameter! [sc_devName = %s]", ps_Data->sc_devName);
					break;
				}
			}

			if (b_IsAddFlag == true) {

				DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE1, "Usb Add [sc_devName = %s]", ps_Data->sc_devName);

				// USBデバイス関連情報更新.
				memcpy(ps_Detail->s_associateInfo[ps_Detail->ul_devNum].sc_devName, ps_Data->sc_devName, DEVD_USBDEVNAME_MAXLENGTH);	// デバイス名設定.

				// デバイス情報更新.
				ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOn;	// 接続ON.
				ps_Detail->ul_devNum++;	// 接続数インクリメント.

				sendMessageToApPf(DEVE_NTF_DataID_Device_Connect, ul_DeviceId, ps_DevInfo);	// AP-PFにデータ送信.
			}

		} else if (ps_Data->uc_AddDev == DEVD_BoolFalse) {	// 切断データ検知.

// 2011.09.19 [USB切断通知仕様変更] Start.
//			DEVE_NTF_DataID e_dataid = DEVE_NTF_DataID_Device_Connect;
			DEVE_NTF_DataID e_dataid = DEVE_NTF_DataID_Device_Disconnect;
// 2011.09.19 [USB切断通知仕様変更] End.
			for (uint32_t i = 0; i < ps_Detail->ul_devNum; i++) {
				if (strncmp(ps_Detail->s_associateInfo[i].sc_devName, ps_Data->sc_devName, DEVD_USBDEVNAME_MAXLENGTH) == 0) {

					DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE1, "Usb Del [sc_devName = %s]", ps_Data->sc_devName);

					// デバイス情報更新.
					ps_Detail->ul_devNum--;	// 接続数デクリメント.
					if (ps_Detail->ul_devNum == 0){	// 接続数が0の場合に切断とする.
						ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOff;	// 接続OFF.
						e_dataid = DEVE_NTF_DataID_Device_Disconnect;			// 切断通知設定.
					}

// 2011.09.19 [USB切断通知仕様変更] Start.
					DEVS_DeviceInfo s_DevInfoDel;
					s_DevInfoDel = *ps_DevInfo;
					memset((char *)&s_DevInfoDel.s_DeviceConnectInfo.uc_DeviceDetailInfo, (char)DEVD_ZERO_OUT, sizeof(s_DevInfoDel.s_DeviceConnectInfo.uc_DeviceDetailInfo));
					DEVS_DeviceDetailInfo_Usb *ps_DetailDel = (DEVS_DeviceDetailInfo_Usb*)s_DevInfoDel.s_DeviceConnectInfo.uc_DeviceDetailInfo;
					ps_DetailDel->ul_devNum = 1ul;
					ps_DetailDel->s_associateInfo[0] = ps_Detail->s_associateInfo[i];
					sendMessageToApPf(e_dataid, ul_DeviceId, &s_DevInfoDel, true);	// AP-PFにデータ送信.
// 2011.09.19 [USB切断通知仕様変更] End.

					// USBデバイス関連情報更新.
					uint32_t j;
					for (j = i; j < ps_Detail->ul_devNum; j++) {
						memcpy(&ps_Detail->s_associateInfo[j], &ps_Detail->s_associateInfo[j+1], sizeof(DEVS_UsbAssociateInfo));
					}
					memset(&ps_Detail->s_associateInfo[j], DEVD_ZERO_OUT ,sizeof(DEVS_UsbAssociateInfo));

// 2011.09.19 [USB切断通知仕様変更] Start.
//					sendMessageToApPf(e_dataid, ul_DeviceId, ps_DevInfo);	// AP-PFにデータ送信.
// 2011.09.19 [USB切断通知仕様変更] End.
					break;
				}
			}
		}
	} while (0);
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]USBキーボード接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitKeyboardDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(e_GetInfoType == DEVE_DeviceServiceGetInfoType_Polling ? DEVD_LOGLEVEL_TRACE2: DEVD_LOGLEVEL_TRACE1, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	getUnitUSBDevDeviceInfo(e_GetInfoType, DEVD_DeviceID_UsbKeyboard, pv_Data, ul_Size);

	DEVSRV_TRACE(e_GetInfoType == DEVE_DeviceServiceGetInfoType_Polling ? DEVD_LOGLEVEL_TRACE2: DEVD_LOGLEVEL_TRACE1, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]USBマウス接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitMouseDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(e_GetInfoType == DEVE_DeviceServiceGetInfoType_Polling ? DEVD_LOGLEVEL_TRACE2: DEVD_LOGLEVEL_TRACE1, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	getUnitUSBDevDeviceInfo(e_GetInfoType, DEVD_DeviceID_UsbMouse, pv_Data, ul_Size);

	DEVSRV_TRACE(e_GetInfoType == DEVE_DeviceServiceGetInfoType_Polling ? DEVD_LOGLEVEL_TRACE2: DEVD_LOGLEVEL_TRACE1, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]USBバルクデバイス(USBメモリ)接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitBulkDeviceDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(e_GetInfoType == DEVE_DeviceServiceGetInfoType_Polling ? DEVD_LOGLEVEL_TRACE2: DEVD_LOGLEVEL_TRACE1, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_UsbBulk;

	do {
		if (e_GetInfoType == DEVE_DeviceServiceGetInfoType_Init) {	// 初期化時.

			// デバイス情報初期化.
			DEVS_DeviceInfo s_DevinfoNew;
			DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
			if (ps_DevInfo != NULL) {
				ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOn; // 接続設定が無い為、常にON設定とする.
				ps_DevInfo->s_DeviceConnectInfo.uc_TemporaryRescueStatus = DEVD_TemporaryRescueStatusReset; // 暫定救済不可の為、常に暫定救済していない状態を設定する.
			}
			break;
		}

		// 変化点通知時のみ実行.
		if (e_GetInfoType != DEVE_DeviceServiceGetInfoType_DataChanged) { break; }

		// パラメータチェック.
		if ((pv_Data == NULL) || (ul_Size < sizeof(DEVS_UsbChangeData))) {
			DEVSRV_LOG(DEVD_LOGLEVEL_ERROR, "Bad Parameter! Nullpointer [pv_Data=%p, ul_Size=%ld]", pv_Data, ul_Size);
			break;
		}

		// デバイス情報取得.
		DEVS_DeviceInfo *ps_DevInfo = DEVD_MAP_DEVICEINFO_GET(ul_DeviceId);
		if (ps_DevInfo == NULL) {
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード s.
//			APIC_ThrowSystemError(SYSE_SysErrGeneral, DEVD_DEBUG_EVENT_KIND, "DEVC_DeviceService getUnitBulkDeviceDeviceInfo() find error");
			APIC_ThrowSystemErrorNewDetailCode(SYSE_SysErrGeneral, DEVD_DEBUG_EVENT_KIND, "DEVC_DeviceService getUnitBulkDeviceDeviceInfo() find error", 0, createDetailCode(0x5012));
// 2015.10.16 ZeusSZX0 制御76-1 新詳細コード e.
			break;
		}
		DEVS_DeviceDetailInfo_UsbBulk *ps_Detail = (DEVS_DeviceDetailInfo_UsbBulk*)ps_DevInfo->s_DeviceConnectInfo.uc_DeviceDetailInfo;

		DEVS_UsbChangeData *ps_Data = (DEVS_UsbChangeData *)pv_Data;
		if (ps_Data->uc_AddDev == DEVD_BoolTrue) {			// 接続データ検知.

			if (ps_Detail->ul_devNum >= DEVD_USBDEVCONNECT_MAX) {
				DEVSRV_LOG(DEVD_LOGLEVEL_ERROR, "Bad Parameter! DeviceID max over [ul_devNum = %ld]", ps_Detail->ul_devNum);
				break; // 最大DEVD_USBDEVCONNECT_MAXまでしか管理しない.
			}

			bool b_IsAddFlag = true;
			for (uint32_t i = 0; i < ps_Detail->ul_devNum; i++) {
				if (strncmp(ps_Detail->s_associateInfo[i].sc_devName, ps_Data->sc_devName, DEVD_USBDEVNAME_MAXLENGTH_BULK) == 0) {
					b_IsAddFlag = false;
					DEVSRV_LOG(DEVD_LOGLEVEL_ERROR, "Bad Parameter! [sc_devName = %s]", ps_Data->sc_devName);
					break;
				}
			}

			if (b_IsAddFlag == true) {

				DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE1, "Usb Add [sc_devName = %s]", ps_Data->sc_devName);

				// DataChanged.
				dataChanged(ps_Data->sc_devName, ps_Data->sc_mountPath);

				// USBバルク(USBメモリ)デバイス関連情報更新.
				memcpy(ps_Detail->s_associateInfo[ps_Detail->ul_devNum].sc_devName, ps_Data->sc_devName, DEVD_USBDEVNAME_MAXLENGTH_BULK);		// デバイス名設定.
/* 2012/05/30 S.Wakayama CQ:364359対応 Start */
//				ps_Detail->s_associateInfo[ps_Detail->ul_devNum].uc_mountStatus = DEVD_BoolFalse;												// マウント状態設定.
				ps_Detail->s_associateInfo[ps_Detail->ul_devNum].uc_mountStatus = checkMountStatus(ps_Data->sc_mountPath);						// マウント状態設定.
/* 2012/05/30 S.Wakayama CQ:364359対応 End */
				memcpy(ps_Detail->s_associateInfo[ps_Detail->ul_devNum].sc_mountPath, ps_Data->sc_mountPath, DEVD_USBDEVMOUNTPATH_MAXLENGTH);	// マウントパス設定.

				// デバイス情報更新.
				ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOn;	// 接続ON.
				ps_Detail->ul_devNum++;	// 接続数インクリメント.

				sendMessageToApPf(DEVE_NTF_DataID_Device_Connect, ul_DeviceId, ps_DevInfo);

			}

		} else if (ps_Data->uc_AddDev == DEVD_BoolFalse) {	// 切断データ検知.

// 2011.09.19 [USB切断通知仕様変更] Start.
//			DEVE_NTF_DataID e_dataid = DEVE_NTF_DataID_Device_Connect;
			DEVE_NTF_DataID e_dataid = DEVE_NTF_DataID_Device_Disconnect;
// 2011.09.19 [USB切断通知仕様変更] End.
			for (uint32_t i = 0; i < ps_Detail->ul_devNum; i++) {
				if (strncmp(ps_Detail->s_associateInfo[i].sc_devName, ps_Data->sc_devName, DEVD_USBDEVNAME_MAXLENGTH_BULK) == 0) {

					DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE1, "Usb Del [sc_devName = %s]", ps_Data->sc_devName);

					// DataChanged.
					dataChanged("", ps_Data->sc_mountPath);

					// デバイス情報更新.
					ps_Detail->ul_devNum--;	// 接続数デクリメント.
					if (ps_Detail->ul_devNum == 0){	// 接続数が0の場合に切断とする.
						ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOff;	// 接続OFF.
						e_dataid = DEVE_NTF_DataID_Device_Disconnect;			// 切断通知設定.
					}
// 2011.09.19 [USB切断通知仕様変更] Start.
					DEVS_DeviceInfo s_DevInfoDel;
					s_DevInfoDel = *ps_DevInfo;
					memset((char *)&s_DevInfoDel.s_DeviceConnectInfo.uc_DeviceDetailInfo, (char)DEVD_ZERO_OUT, sizeof(s_DevInfoDel.s_DeviceConnectInfo.uc_DeviceDetailInfo));
					DEVS_DeviceDetailInfo_UsbBulk *ps_DetailDel = (DEVS_DeviceDetailInfo_UsbBulk*)s_DevInfoDel.s_DeviceConnectInfo.uc_DeviceDetailInfo;
					ps_DetailDel->ul_devNum = 1ul;
					ps_DetailDel->s_associateInfo[0] = ps_Detail->s_associateInfo[i];
					ps_DetailDel->s_associateInfo[0].uc_mountStatus = DEVD_BoolFalse;	// マウント状態設定.
					sendMessageToApPf(e_dataid, ul_DeviceId, &s_DevInfoDel, true);	// AP-PFにデータ送信.
// 2011.09.19 [USB切断通知仕様変更] End.

					// USBバルク(USBメモリ)デバイス関連情報更新.
					uint32_t j;
					for (j = i; j < ps_Detail->ul_devNum; j++) {
						memcpy(&ps_Detail->s_associateInfo[j], &ps_Detail->s_associateInfo[j+1], sizeof(DEVS_UsbAssociateInfo_Bulk));
					}
					memset(&ps_Detail->s_associateInfo[j],DEVD_ZERO_OUT ,sizeof(DEVS_UsbAssociateInfo_Bulk));

// 2011.09.19 [USB切断通知仕様変更] Start.
//					sendMessageToApPf(e_dataid, ul_DeviceId, ps_DevInfo);	// AP-PFにデータ送信.
// 2011.09.19 [USB切断通知仕様変更] End.
					break;
				}
			}

		}
	} while (0);

	DEVSRV_TRACE(e_GetInfoType == DEVE_DeviceServiceGetInfoType_Polling ? DEVD_LOGLEVEL_TRACE2: DEVD_LOGLEVEL_TRACE1, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]USBスピーカー接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitSpeakerDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{
	DEVSRV_TRACE(e_GetInfoType == DEVE_DeviceServiceGetInfoType_Polling ? DEVD_LOGLEVEL_TRACE2: DEVD_LOGLEVEL_TRACE1, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	getUnitUSBDevDeviceInfo(e_GetInfoType, DEVD_DeviceID_UsbSpeaker, pv_Data, ul_Size);

	DEVSRV_TRACE(e_GetInfoType == DEVE_DeviceServiceGetInfoType_Polling ? DEVD_LOGLEVEL_TRACE2: DEVD_LOGLEVEL_TRACE1, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]DS基板接続情報取得.
//引数　　：e_GetInfoType	[in] 接続情報取得種別.
//引数　　：pv_Data			[in] 受信データ.
//引数　　：ul_Size			[in] 受信データサイズ.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getUnitDSBoardDeviceInfo(DEVE_DeviceServiceGetInfoType e_GetInfoType, const void *pv_Data, uint32_t ul_Size)
{						 
	DEVSRV_TRACE(e_GetInfoType == DEVE_DeviceServiceGetInfoType_Polling ? DEVD_LOGLEVEL_TRACE2: DEVD_LOGLEVEL_TRACE1, "Start(e_GetInfoType=%d, pv_Data=%p, ul_Size=%ld)", e_GetInfoType, pv_Data, ul_Size);

	uint32_t ul_DeviceId = DEVD_DeviceID_DSBoard;	// DS基板.
	DEVS_DeviceInfo s_DevinfoNew;
	DEVS_DeviceInfo *ps_DevInfo = getDeviceInfoCommonGetDeviceInfo(e_GetInfoType, ul_DeviceId, &s_DevinfoNew);	// デバイス接続情報取得処理.
	if (NULL != ps_DevInfo) {
		DEVS_DeviceInfo *ps_DevInfoNew = &s_DevinfoNew;
		
		if( DEVD_BoolTrue == isAsic_2Exist() ){
// 2015/09/30 Sky西村 DS基盤ありでjpeg変換できない問題の対応.
//			ps_DevInfoNew->uc_DeviceConnect = DEVD_DeviceConnectOn;
//			ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOn; // DS基板ありのためON設定.
			if(DEVE_DeviceServiceGetInfoType_Polling == e_GetInfoType) {
				ps_DevInfoNew->uc_DeviceConnect = DEVD_DeviceConnectOn;
				ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOn; // DS基板ありのためON設定.
			}
		}else{
			ps_DevInfoNew->uc_DeviceConnect = DEVD_DeviceConnectOff;
			ps_DevInfoNew->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOff; // DS基板なしの為、OFF設定.
		}

		getDeviceInfoCommonSendMessageToApPf(e_GetInfoType, ps_DevInfo, ps_DevInfoNew, 0); // デバイス接続情報送信,設定処理.
	}

	DEVSRV_TRACE(e_GetInfoType == DEVE_DeviceServiceGetInfoType_Polling ? DEVD_LOGLEVEL_TRACE2: DEVD_LOGLEVEL_TRACE1, "End");
}
/**
* \note Device接続情報取得Sub関数.
*/
//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]デバイス接続状態取得処理.
//引数　　：ul_DeviceId		[in] デバイスID.
//戻り値　：DEVD_DeviceConnect	デバイス接続状態.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
DEVD_DeviceConnect DEVC_DeviceService::getDeviceConnect_Unit(uint32_t ul_DeviceId)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "Start(ul_DeviceId=0x%x)", ul_DeviceId);

	DEVD_DeviceConnect uc_DeviceConnect = DEVD_DeviceConnectOff;	// OFF.

	APIC_PrinterSystem *pc_PrinterSystem = APIC_PrinterSystem::newInstance();	// インスタンス取得.
	APIC_ScannerSystem *pc_ScannerSystem = APIC_ScannerSystem::newInstance();	// インスタンス取得.
	APIC_ScannerStatus *pc_ScannerStatus = APIC_ScannerStatus::newInstance();	// インスタンス取得.
	APIC_SystemInfo *pc_SystemInfo = APIC_SystemInfo::newInstance();			// インスタンス取得.

	switch (ul_DeviceId) {
		case DEVD_DeviceID_PI:			// PI（カバーシート装置）.
			if ((pc_PrinterSystem != NULL) && (pc_PrinterSystem->isCoverSheetFeeder() == True)) {
				uc_DeviceConnect = DEVD_DeviceConnectOn;	// ON.
			}
			break;
		case DEVD_DeviceID_PK:			// PK（パンチキット）.
			if ((pc_PrinterSystem != NULL) && (pc_PrinterSystem->isExistPunchKitOption() == True)) {
				uc_DeviceConnect = DEVD_DeviceConnectOn;	// ON.
			}
			break;
		case DEVD_DeviceID_PZ:			// PZ（Z折り、パンチ処理装置）.
			if ((pc_PrinterSystem != NULL) && (pc_PrinterSystem->isExistPunchUnitOption() == True)) {
				uc_DeviceConnect = DEVD_DeviceConnectOn;	// ON.
			}
			break;
		case DEVD_DeviceID_Finisher:	// フィニッシャー.
			if ((pc_PrinterSystem != NULL) && (pc_PrinterSystem->isExistFinisher() == True)) {
				uc_DeviceConnect = DEVD_DeviceConnectOn;	// ON.
			}
			break;
		case DEVD_DeviceID_RU:			// リレーユニット（RU）.
			if ((pc_PrinterSystem != NULL) && (pc_PrinterSystem->isExistRUOption() == True)) {
				uc_DeviceConnect = DEVD_DeviceConnectOn;	// ON.
			}
			break;
		case DEVD_DeviceID_ADF:			// ADF.
			if ((pc_ScannerSystem != NULL) && (pc_ScannerSystem->getADFType() != TYPE_ADFTypeNon)) {
				uc_DeviceConnect = DEVD_DeviceConnectOn;	// ON.
			}
			break;
		case DEVD_DeviceID_IR:			// IR.
			if ((pc_ScannerStatus != NULL) && (pc_ScannerStatus->isIRNotReady() != True)) {
				uc_DeviceConnect = DEVD_DeviceConnectOn;	// ON.
			}
			break;
		case DEVD_DeviceID_Fax:			// FAXボード.
			if ((pc_SystemInfo != NULL) && (pc_SystemInfo->isFax1Exist() == True)) {
				uc_DeviceConnect = DEVD_DeviceConnectOn;	// ON.
			}
			break;
		case DEVD_DeviceID_FaxSub:		// ＦＡＸ副回線.
			if ((pc_SystemInfo != NULL) && (pc_SystemInfo->isFax2Exist() == True)) {
				uc_DeviceConnect = DEVD_DeviceConnectOn;	// ON.
			}
			break;
		case DEVD_DeviceID_USB:			// USB.
			if ((pc_SystemInfo != NULL) && (pc_SystemInfo->isUSBHostExist() == True)) {
				uc_DeviceConnect = DEVD_DeviceConnectOn;	// ON.
			}
			break;
		case DEVD_DeviceID_ColorDev:			uc_DeviceConnect = DEVD_DeviceConnectOn;	break;	// カラー現像器.
		case DEVD_DeviceID_Network:				uc_DeviceConnect = DEVD_DeviceConnectOn;	break;	// Network.
		case DEVD_DeviceID_IEEE1284:			uc_DeviceConnect = DEVD_DeviceConnectOn;	break;	// IEEE1284.
		case DEVD_DeviceID_PrinterControler:	uc_DeviceConnect = DEVD_DeviceConnectOn;	break;	// プリンタコントローラー.
		default:																			break;	// default.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "End(uc_DeviceConnect=%d)", uc_DeviceConnect);
	return uc_DeviceConnect;
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]デバイス使用状態取得処理.
//引数　　：ul_DeviceId		[in] デバイスID.
//戻り値　：DEVD_DeviceUsed	デバイス使用状況.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
DEVD_DeviceUsed DEVC_DeviceService::getDeviceUsed_Unit(uint32_t ul_DeviceId)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "Start(ul_DeviceId=0x%x)", ul_DeviceId);

	DEVD_DeviceUsed uc_DeviceUsed = DEVD_DeviceUsedOn;	// ON設定(接続設定がないものについては、常にONとする).

	APIC_HddInfo *pc_HddInfo = APIC_HddInfo::newInstance();	// インスタンス取得.
	APIC_SystemInfo *pc_SystemInfo = APIC_SystemInfo::newInstance();			// インスタンス取得.

	switch (ul_DeviceId) {
		case DEVD_DeviceID_HDD:		// HDD.
			if ((pc_HddInfo == NULL) || (pc_HddInfo->getHddExist() != True)) {
				uc_DeviceUsed = DEVD_DeviceUsedOff;	// OFF設定.
			}
			break;
		case DEVD_DeviceID_Fax:		// FAXボード.
			if ((pc_SystemInfo == NULL) || (pc_SystemInfo->getFax1Exist() != True)) {
				uc_DeviceUsed = DEVD_DeviceUsedOff;	// OFF設定.
			}
			break;
		case DEVD_DeviceID_FaxSub:	// ＦＡＸ副回線.
			if ((pc_SystemInfo == NULL) || (pc_SystemInfo->getFax2Exist() != True)) {
				uc_DeviceUsed = DEVD_DeviceUsedOff;	// OFF設定.
			}
			break;
		case DEVD_DeviceID_Tray1:						break;	// トレイ1.
		case DEVD_DeviceID_Tray2:						break;	// トレイ2.
		case DEVD_DeviceID_Tray3:						break;	// トレイ3.
		case DEVD_DeviceID_Tray4:						break;	// トレイ4.
// DenebMLK:機器構成変更_No.7_5段増設プリント機能追加対応 start.
		case DEVD_DeviceID_Tray5:						break;	// トレイ5.
// DenebMLK:機器構成変更_No.7_5段増設プリント機能追加対応 end.
		case DEVD_DeviceID_TrayLCT:						break;	// 大容量給紙ユニット（LCT）.
		case DEVD_DeviceID_TrayManual:					break;	// 手差しトレイ.
		case DEVD_DeviceID_TrayPI1:						break;	// PIトレイ1.
		case DEVD_DeviceID_TrayPI2:						break;	// PIトレイ2.
		case DEVD_DeviceID_PI:							break;	// PI（カバーシート装置）.
		case DEVD_DeviceID_PK:							break;	// PK（パンチキット）.
		case DEVD_DeviceID_PZ:							break;	// PZ（Z折り、パンチ処理装置）.
		case DEVD_DeviceID_Folder:						break;	// 中折り、中綴じ、3つ折り装置.
		case DEVD_DeviceID_Finisher:					break;	// フィニッシャー.
		case DEVD_DeviceID_RU:							break;	// リレーユニット（RU）.
		case DEVD_DeviceID_Staple:						break;	// ステープルユニット.
		case DEVD_DeviceID_ADF:							break;	// ADF.
		case DEVD_DeviceID_IR:							break;	// IR.
		case DEVD_DeviceID_ColorDev:					break;	// カラー現像器.
		case DEVD_DeviceID_Network:						break;	// Network.
		case DEVD_DeviceID_IEEE1284:					break;	// IEEE1284.
		case DEVD_DeviceID_USB:							break;	// USB.
		case DEVD_DeviceID_PrinterControler:			break;	// プリンタコントローラー.
		default:	uc_DeviceUsed = DEVD_DeviceUsedOff; break;	// default.
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "End(uc_DeviceUsed=%d)", uc_DeviceUsed);
	return uc_DeviceUsed;
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]暫定救済状態取得処理.
//引数　　：ul_DeviceId		[in] デバイスID.
//戻り値　：DEVD_TemporaryRescueStatus	暫定救済状態.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
DEVD_TemporaryRescueStatus DEVC_DeviceService::getTemporaryRescueStatus(uint32_t ul_DeviceId)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "Start(ul_DeviceId=0x%x)", ul_DeviceId);

	// パラメータ変換用Tbl.
	struct {
		uint32_t ul_DeviceId;
		TYPE_TemporaryRescueUnit e_TemporaryRescue;
	} static s_tbl_temporary_rescue[] = {
		{DEVD_DeviceID_Tray1,			TYPE_TemporaryRescueUnitTray1},				// トレイ1.
		{DEVD_DeviceID_Tray2,			TYPE_TemporaryRescueUnitTray2},				// トレイ2.
		{DEVD_DeviceID_Tray3,			TYPE_TemporaryRescueUnitTray3},				// トレイ3.
		{DEVD_DeviceID_Tray4,			TYPE_TemporaryRescueUnitTray4},				// トレイ4.
// DenebMLK:機器構成変更_No.7_5段増設プリント機能追加対応 start.
		{DEVD_DeviceID_Tray5,			TYPE_TemporaryRescueUnitTray5},				// トレイ5.
// DenebMLK:機器構成変更_No.7_5段増設プリント機能追加対応 end.
		{DEVD_DeviceID_TrayLCT,			TYPE_TemporaryRescueUnitTrayLCT},			// 大容量給紙ユニット（LCT）.
		{DEVD_DeviceID_TrayManual,		TYPE_TemporaryRescueUnitTrayManual},		// 手差しトレイ.
		{DEVD_DeviceID_PI,				TYPE_TemporaryRescueUnitPI},				// PI（カバーシート装置）.
		{DEVD_DeviceID_PK,				TYPE_TemporaryRescueUnitPK},				// PK（パンチキット）.
		{DEVD_DeviceID_PZ,				TYPE_TemporaryRescueUnitPZ},				// PZ（Z折り、パンチ処理装置）.
		{DEVD_DeviceID_Folder,			TYPE_TemporaryRescueUnitFolder},			// 中折り、中綴じ、3つ折り装置.
		{DEVD_DeviceID_Finisher,		TYPE_TemporaryRescueUnitFinisher},			// フィニッシャー.
		{DEVD_DeviceID_RU,				TYPE_TemporaryRescueUnitNone},				// リレーユニット（RU）.
		{DEVD_DeviceID_Staple,			TYPE_TemporaryRescueUnitStaple},			///< ステープルユニット.
		{DEVD_DeviceID_ADF,				TYPE_TemporaryRescueUnitADF},				///< ADF.
		{DEVD_DeviceID_IR,				TYPE_TemporaryRescueUnitIR},				//< IR.
		{DEVD_DeviceID_ColorDev,		TYPE_TemporaryRescueUnitColorDev},			// カラー現像器.
		{DEVD_DeviceID_Fax,				TYPE_TemporaryRescueUnitFax},				// FAXボード.
		{DEVD_DeviceID_FaxSub,			TYPE_TemporaryRescueUnitFaxSub},			// ＦＡＸ副回線.
		{DEVD_DeviceID_Network,			TYPE_TemporaryRescueUnitNetwork},			// Network.
		{DEVD_DeviceID_IEEE1284,		TYPE_TemporaryRescueUnit1284},				// IEEE1284.
		{DEVD_DeviceID_USB,				TYPE_TemporaryRescueUnitUSB},				// USB.
		{DEVD_DeviceID_PrinterControler,TYPE_TemporaryRescueUnitPrinterControler},	// プリンタコントローラー.
		{DEVD_DeviceID_HDD,				TYPE_TemporaryRescueUnitHDD},				// HDD.
	};

	// 設定値変換用Tbl.
	struct {
		TYPE_TemporaryRescueStatus e_TemporaryRescueStatus;
		DEVD_TemporaryRescueStatus uc_TemporaryRescueStatus;
	} static s_tbl_temporary_rescue_status[] = {
		{TYPE_TemporaryRescueStatusReset,	DEVD_TemporaryRescueStatusReset},	// 切離しユニット後との暫定救済状態：暫定救済してない状態.
		{TYPE_TemporaryRescueStatusSet,		DEVD_TemporaryRescueStatusSet},		// 切離しユニット後との暫定救済状態：暫定救済中状態.
		{TYPE_TemporaryRescueStatusPreSet,	DEVD_TemporaryRescueStatusPreSet},	// 切離しユニット後との暫定救済状態：仮暫定救済中状態.
		{TYPE_TemporaryRescueStatusOneJob,	DEVD_TemporaryRescueStatusOneJob},	// 切離しユニット後との暫定救済状態：実行ジョブのみカラー出力暫定救済中状態.
		{TYPE_TemporaryRescueStatusAllJob,	DEVD_TemporaryRescueStatusAllJob},	// 切離しユニット後との暫定救済状態：常時カラー出力暫定救済中状態.
		{TYPE_TemporaryRescueStatusJobEnd,	DEVD_TemporaryRescueStatusJobEnd},	// 切離しユニット後との暫定救済状態：ジョブ終了状態を通知する定義.
		{TYPE_TemporaryRescueStatusChange,	DEVD_TemporaryRescueStatusChange},	// 切離しユニット後との暫定救済状態：TC/IUの状態がErrorStopから以外に変化した時の通知.
	};

	DEVD_TemporaryRescueStatus uc_TemporaryRescueStatus = DEVD_TemporaryRescueStatusReset;	// 暫定救済してない状態(暫定救済設定がないものについては、常にこの値を設定する).

	APIC_SystemStatus *pc_Instance = APIC_SystemStatus::newInstance();	// インスタンス取得.
	if (pc_Instance != NULL) {

		// パラメータ変換.
		uint32_t ul_tbl_size = sizeof(s_tbl_temporary_rescue) / sizeof(s_tbl_temporary_rescue[0]);
		for (uint32_t i = 0; i < ul_tbl_size; i++) {
			if (s_tbl_temporary_rescue[i].ul_DeviceId == ul_DeviceId) {

				TYPE_TemporaryRescueStatus e_TemporaryRescueStatus = pc_Instance->getTemporaryRescueUserStatus(s_tbl_temporary_rescue[i].e_TemporaryRescue);

				// 設定値変換.
				ul_tbl_size = sizeof(s_tbl_temporary_rescue_status) / sizeof(s_tbl_temporary_rescue_status[0]);
				for (i = 0; i < ul_tbl_size; i++) {
					if (s_tbl_temporary_rescue_status[i].e_TemporaryRescueStatus == e_TemporaryRescueStatus) {
						uc_TemporaryRescueStatus = s_tbl_temporary_rescue_status[i].uc_TemporaryRescueStatus;
						break;
					}
				}
				break;
			}
		}

	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "End(uc_TemporaryRescueStatus=%d)", uc_TemporaryRescueStatus);
	return uc_TemporaryRescueStatus;
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]PUデバイス詳細情報取得処理.
//引数　　：ul_DeviceId		[in] デバイスID.
//引数　　：ps_DetailInfo	[out] デバイス詳細情報.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getPunchInfo(uint32_t ul_DeviceId, DEVS_DeviceDetailInfo_Punch *ps_DetailInfo)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "Start(ul_DeviceId=0x%x, ps_DetailInfo=%p)", ul_DeviceId, ps_DetailInfo);

	if (ps_DetailInfo != NULL) {

		// 初期値設定.
		ps_DetailInfo->uc_Punch2Hole = DEVD_BoolFalse;   ///< 2穴.
		ps_DetailInfo->uc_Punch3Hole = DEVD_BoolFalse;   ///< 3穴.
		ps_DetailInfo->uc_Punch4Hole = DEVD_BoolFalse;   ///< 4穴.
		ps_DetailInfo->uc_Punch4HoleSWE = DEVD_BoolFalse;  ///< スウェーデン式4穴.

		APIC_PrinterSystem *pc_Instance = APIC_PrinterSystem::newInstance();	// インスタンス取得.
		if (pc_Instance != NULL) {

			TYPS_PunchFunction st_PunchInfo = pc_Instance->getPunchFunction();

			// 設定値変換.
			ps_DetailInfo->uc_Punch2Hole = (st_PunchInfo.b_Punch2Hole == True) ? DEVD_BoolTrue: DEVD_BoolFalse;			// 2穴.
			ps_DetailInfo->uc_Punch3Hole = (st_PunchInfo.b_Punch3Hole == True) ? DEVD_BoolTrue: DEVD_BoolFalse;			// 3穴.
			ps_DetailInfo->uc_Punch4Hole = (st_PunchInfo.b_Punch4Hole == True) ? DEVD_BoolTrue: DEVD_BoolFalse;			// 4穴.
			ps_DetailInfo->uc_Punch4HoleSWE = (st_PunchInfo.b_Punch4HoleSWE == True) ? DEVD_BoolTrue: DEVD_BoolFalse;	// スウェーデン式4穴.

		}

		DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "uc_Punch2Hole=%d, uc_Punch3Hole=%d, uc_Punch4Hole=%d, uc_Punch4HoleSWE=%d",
										ps_DetailInfo->uc_Punch2Hole, ps_DetailInfo->uc_Punch3Hole, ps_DetailInfo->uc_Punch4Hole, ps_DetailInfo->uc_Punch4HoleSWE);

	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]Folderデバイス詳細情報取得処理.
//引数　　：ul_DeviceId		[in] デバイスID.
//引数　　：ps_DetailInfo	[out] デバイス詳細情報.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getFolderInfo(uint32_t ul_DeviceId, DEVS_DeviceDetailInfo_Folder *ps_DetailInfo)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "Start(ul_DeviceId=0x%x, ps_DetailInfo=%p)", ul_DeviceId, ps_DetailInfo);

	if (ps_DetailInfo != NULL) {

		// 初期値設定.
		ps_DetailInfo->uc_FoldZshaped = DEVD_BoolFalse;		///< Ｚ折り機能.
		ps_DetailInfo->uc_FoldCenterInner = DEVD_BoolFalse;	///< 内中折り.
		ps_DetailInfo->uc_FoldLetter = DEVD_BoolFalse;		///< 内三つ折り.

		APIC_PrinterSystem *pc_Instance = APIC_PrinterSystem::newInstance();	// インスタンス取得.
		if (pc_Instance != NULL) {

			TYPS_FoldFunction st_PunchInfo = pc_Instance->getFoldFunction();

			// 設定値変換.
			ps_DetailInfo->uc_FoldZshaped = (st_PunchInfo.b_FoldZshaped == True) ? DEVD_BoolTrue: DEVD_BoolFalse;			// Ｚ折り機能.
			ps_DetailInfo->uc_FoldCenterInner = (st_PunchInfo.b_FoldCenterInner == True) ? DEVD_BoolTrue: DEVD_BoolFalse;	// 内中折り.
			ps_DetailInfo->uc_FoldLetter = (st_PunchInfo.b_FoldLetter == True) ? DEVD_BoolTrue: DEVD_BoolFalse;				// 内三つ折り.

		}

		DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "uc_FoldZshaped=%d, uc_FoldCenterInner=%d, uc_FoldLetter=%d",
										ps_DetailInfo->uc_FoldZshaped, ps_DetailInfo->uc_FoldCenterInner, ps_DetailInfo->uc_FoldLetter);

	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]Stapleデバイス詳細情報取得処理.
//引数　　：ul_DeviceId		[in] デバイスID.
//引数　　：ps_DetailInfo	[out] デバイス詳細情報.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getStapleInfo(uint32_t ul_DeviceId, DEVS_DeviceDetailInfo_Staple *ps_DetailInfo)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "Start(ul_DeviceId=0x%x, ps_DetailInfo=%p)", ul_DeviceId, ps_DetailInfo);

	if (ps_DetailInfo != NULL) {

		// 初期値設定.
		ps_DetailInfo->uc_Staple1Corner = DEVD_BoolFalse;		// 1点コーナー.
		ps_DetailInfo->uc_Staple2Side = DEVD_BoolFalse;			// 2点サイド.
		ps_DetailInfo->uc_Staple2SideCenter = DEVD_BoolFalse;	// 2点サイド＋センター.
		ps_DetailInfo->uc_StapleMaxNum = 0;						// ステープル可能枚数.

		APIC_PrinterSystem *pc_Instance = APIC_PrinterSystem::newInstance();	// インスタンス取得.
		if (pc_Instance != NULL) {

			TYPS_StapleFunction st_StapleInfo = pc_Instance->getStapleFunction();

			// 設定値変換.
			ps_DetailInfo->uc_Staple1Corner = (st_StapleInfo.b_Staple1Corner == True) ? DEVD_BoolTrue: DEVD_BoolFalse;			// 1点コーナー.
			ps_DetailInfo->uc_Staple2Side = (st_StapleInfo.b_Staple2Side == True) ? DEVD_BoolTrue: DEVD_BoolFalse;				// 2点サイド.
			ps_DetailInfo->uc_Staple2SideCenter = (st_StapleInfo.b_Staple2SideCenter == True) ? DEVD_BoolTrue: DEVD_BoolFalse;	// 2点サイド＋センター.
			ps_DetailInfo->uc_StapleMaxNum = st_StapleInfo.uc_StapleMaxNum;	// ステープル可能枚数.

		}

		DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "uc_Staple1Corner=%d, uc_Staple2Side=%d, uc_Staple2SideCenter=%d, uc_StapleMaxNum=%d",
										ps_DetailInfo->uc_Staple1Corner, ps_DetailInfo->uc_Staple2Side, ps_DetailInfo->uc_Staple2SideCenter, ps_DetailInfo->uc_StapleMaxNum);

	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]トレイ種別取得処理.
//引数　　：ul_DeviceId		[in] デバイスID.
//戻り値　：DEVD_TrayType	トレイ種別.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
DEVD_TrayType DEVC_DeviceService::getTrayType(uint32_t ul_DeviceId)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "Start(ul_DeviceId=0x%x)", ul_DeviceId);

	// パラメータ変換用Tbl.
	struct {
		uint32_t ul_DeviceId;
		TYPE_FeedTray e_DeviceId;
	} static s_tbl_deviceid[] = {
		{DEVD_DeviceID_Tray1,		TYPE_FeedTray1},		// トレイ1.
		{DEVD_DeviceID_Tray2,		TYPE_FeedTray2},		//< トレイ2.
		{DEVD_DeviceID_Tray3,		TYPE_FeedTray3},		//< トレイ3.
		{DEVD_DeviceID_Tray4,		TYPE_FeedTray4},		//< トレイ4.
// DenebMLK:機器構成変更_No.7_5段増設プリント機能追加対応 start.
		{DEVD_DeviceID_Tray5,		TYPE_FeedTray5},		//< トレイ5.
// DenebMLK:機器構成変更_No.7_5段増設プリント機能追加対応 end.
		{DEVD_DeviceID_TrayManual,	TYPE_FeedTrayManual},	//< 手差しトレイ.
		{DEVD_DeviceID_TrayPI1,		TYPE_FeedTrayPI1},		//< PIトレイ1.
		{DEVD_DeviceID_TrayPI2,		TYPE_FeedTrayPI2},		//< PIトレイ2.
	};

	// 設定値変換用Tbl.
	struct {
		TYPE_TrayType e_Type;
		DEVD_TrayType uc_Type;
	} static s_tbl_type[] = {
		{TYPE_TrayTypeNon,			DEVD_TrayTypeNon},		// 給紙トレータイプ：給紙トレイなし.
		{TYPE_TrayTypeFix,			DEVD_TrayTypeFix},		// 給紙トレータイプ：固定用紙サイズ.
		{TYPE_TrayTypeUniversal,	DEVD_TrayTypeUniversal},// 給紙トレータイプ：ユニバーサル.
		{TYPE_TrayTypeMPC,			DEVD_TrayTypeMPC},		// 給紙トレータイプ：マルチパーパス.
		{TYPE_TrayTypeLCC,			DEVD_TrayTypeLCC},		// 給紙トレータイプ：LCC.
		{TYPE_TrayTypeLCT,			DEVD_TrayTypeLCT},		// 給紙トレータイプ：LCT.
		{TYPE_TrayTypeMultiMan,		DEVD_TrayTypeMultiMan},	// 給紙トレータイプ：マルチ手差し.
		{TYPE_TrayTypeSingleMan,	DEVD_TrayTypeSingleMan},// 給紙トレータイプ：シングル手差し.
		{TYPE_TrayTypeUnset,		DEVD_TrayTypeUnset},	// 給紙トレータイプ：トレイセット不良.
		{TYPE_TrayTypePI,			DEVD_TrayTypePI},		// 給紙トレータイプ：PIトレイ.
	};

	DEVD_TrayType uc_Type = DEVD_TrayTypeNon;	// 給紙トレータイプ：給紙トレイなし.

	APIC_FeedTray *pc_Instance = APIC_FeedTray::newInstance();	// インスタンス取得.
	if (pc_Instance != NULL) {

		// パラメータ変換.
		uint32_t ul_tbl_size = sizeof(s_tbl_deviceid) / sizeof(s_tbl_deviceid[0]);
		for (uint32_t i = 0; i < ul_tbl_size; i++) {
			if (s_tbl_deviceid[i].ul_DeviceId == ul_DeviceId) {

				TYPE_TrayType e_Type = pc_Instance->getTrayType(s_tbl_deviceid[i].e_DeviceId);

				// 設定値変換.
				ul_tbl_size = sizeof(s_tbl_type) / sizeof(s_tbl_type[0]);
				for (i = 0; i < ul_tbl_size; i++) {
					if (s_tbl_type[i].e_Type == e_Type) {
						uc_Type = s_tbl_type[i].uc_Type;
						break;
					}
				}
				break;
			}
		}

	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "End(uc_Type=%d)", uc_Type);
	return uc_Type;
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]LCTトレイ種別取得処理.
//引数　　：ul_DeviceId		[in] デバイスID.
//戻り値　：DEVD_LCTType	LCTトレイ種別.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
DEVD_LCTType DEVC_DeviceService::getLCTType(uint32_t ul_DeviceId)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "Start(ul_DeviceId=0x%x)", ul_DeviceId);

	// 設定値変換用Tbl.
	struct {
		TYPE_LCTType e_Type;
		DEVD_LCTType uc_Type;
	} static s_tbl_type[] = {
		{TYPE_LCTTypeLT402,	DEVD_LCTTypeLT402},	// ＬＣＴタイプ：LT-402.
		{TYPE_LCTTypeLT412,	DEVD_LCTTypeLT412},	// ＬＣＴタイプ：LT-412.
	};

	DEVD_TrayType uc_Type = (DEVD_LCTType)DEVD_TypeNotSet;	// ＬＣＴタイプ未設定.

	APIC_PrinterSystem *pc_Instance = APIC_PrinterSystem::newInstance();	// インスタンス取得.
	if (pc_Instance != NULL) {

		TYPE_LCTType e_Type = pc_Instance->getLCTType();

		// 設定値変換.
		uint32_t ul_tbl_size = sizeof(s_tbl_type) / sizeof(s_tbl_type[0]);
		for (uint32_t i = 0; i < ul_tbl_size; i++) {
			if (s_tbl_type[i].e_Type == e_Type) {
				uc_Type = s_tbl_type[i].uc_Type;
				break;
			}
		}
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "End(uc_Type=%d)", uc_Type);
	return uc_Type;
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]Finisher種別取得処理.
//引数　　：ul_DeviceId			[in] デバイスID.
//戻り値　：DEVD_FinisherType	Finisher種別.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
DEVD_FinisherType DEVC_DeviceService::getFinisherType(uint32_t ul_DeviceId)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "Start(ul_DeviceId=0x%x)", ul_DeviceId);

	// 設定値変換用Tbl.
	struct {
		TYPE_FinisherType e_Type;
		DEVD_FinisherType uc_Type;
	} static s_tbl_type[] = {
		{TYPE_FinisherTypeX0,		DEVD_FinisherTypeX0},		// フィニッシャ名：ＦＮ－Ｘ０.
		{TYPE_FinisherTypeX1,		DEVD_FinisherTypeX1},		// フィニッシャ名：ＦＮ－Ｘ１.
		{TYPE_FinisherTypeX3,		DEVD_FinisherTypeX3},		// フィニッシャ名：ＦＮ－Ｘ３.
		{TYPE_FinisherTypeFS504,	DEVD_FinisherTypeFS504},	// フィニッシャ名：ＦＳ５０４.
		{TYPE_FinisherTypeFS602,	DEVD_FinisherTypeFS602},	// フィニッシャ名：ＦＳ６０２.
		{TYPE_FinisherTypeFS505,	DEVD_FinisherTypeFS505},	// フィニッシャ名：ＦＳ５０５.
		{TYPE_FinisherTypeFS506,	DEVD_FinisherTypeFS506},	// フィニッシャ名：ＦＳ５０６.
		{TYPE_FinisherTypeFS601,	DEVD_FinisherTypeFS601},	// フィニッシャ名：ＦＳ６０１.
		{TYPE_FinisherTypeCB	,	DEVD_FinisherTypeCB},		// フィニッシャ名：CB_FN.
		{TYPE_FinisherTypeFS517,	DEVD_FinisherTypeFS517},	// フィニッシャ名：ＦＳ５１７（５０枚ステープル）.
		{TYPE_FinisherTypeFS518,	DEVD_FinisherTypeFS518},	// フィニッシャ名：ＦＳ５１８（１００枚ステープル）.
		{TYPE_FinisherTypeFS608,	DEVD_FinisherTypeFS608},	// フィニッシャ名：ＦＳ６０８（サドル）.
		{TYPE_FinisherTypeJS,		DEVD_FinisherTypeJS},		// フィニッシャ名：ジョブセパレータ.
		{TYPE_FinisherTypeFS526,	DEVD_FinisherTypeFS526},	// Donau1/2 Finisher(Elbe Finisher).
		{TYPE_FinisherTypeFS527,	DEVD_FinisherTypeFS527},	// Donau3 Finisher(Slim Finisher).
		{TYPE_FinisherTypeFS529,	DEVD_FinisherTypeFS529},	// Amur1/2/3 Finisher.
		{TYPE_FinisherTypeJS506,	DEVD_FinisherTypeJS506},	// ZeusX/0/1/2/3 Finisher(JS後継機).
		{TYPE_FinisherTypeFS533,	DEVD_FinisherTypeFS533},	// ZeusX/0/1/2/3 Finisher(FS-529後継機＋パンチ).
		{TYPE_FinisherTypeFS534,	DEVD_FinisherTypeFS534},	// Minerva0/1, ZeusX/0/1/2/3 Finisher(FS-527後継機＋三折り).
		{TYPE_FinisherTypeFS535,	DEVD_FinisherTypeFS535},	// Minerva0/1, ZeusX Finisher(FS-526後継機).
#if defined(DEF_A3A4COMM)
		{TYPE_FinisherTypeVenusFNS, DEVD_FinisherTypeVenusFNS}, // VenusFNS.
#endif //defined(DEF_A3A4COMM)
	};

	DEVD_FinisherType uc_Type = DEVD_FinisherTypeX0;	// フィニッシャ名：ＦＮ－Ｘ０.

	APIC_PrinterSystem *pc_Instance = APIC_PrinterSystem::newInstance();	// インスタンス取得.
	if (pc_Instance != NULL) {

		TYPE_FinisherType e_Type = pc_Instance->getFinisherType();

		// 設定値変換.
		uint32_t ul_tbl_size = sizeof(s_tbl_type) / sizeof(s_tbl_type[0]);
		for (uint32_t i = 0; i < ul_tbl_size; i++) {
			if (s_tbl_type[i].e_Type == e_Type) {
				uc_Type = s_tbl_type[i].uc_Type;
				break;
			}
		}
	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "End(uc_Type=%d)", uc_Type);
	return uc_Type;
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]チャネル状態取得処理.
//引数　　：ul_DeviceId			[in] デバイスID.
//戻り値　：DEVD_ChannelStatus	チャネル状態.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
DEVD_ChannelStatus DEVC_DeviceService::getChannelStatus(uint32_t ul_DeviceId)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "Start(ul_DeviceId=0x%x)", ul_DeviceId);

	// パラメータ変換用Tbl.
	struct {
		uint32_t ul_DeviceId;
		TYPE_ChannelType e_DeviceId;
	} static s_tbl_deviceid[] = {
		{DEVD_DeviceID_IEEE1284,	TYPE_ChannelTypeIEEE1284,	},	// チャネルタイプ：ＩＥＥＥ１２８４.
		{DEVD_DeviceID_USB,			TYPE_ChannelTypeUSB,		},	// チャネルタイプ：ＵＳＢ.
		{DEVD_DeviceID_Network,		TYPE_ChannelTypeNetwork,	},	// チャネルタイプ：Ｎｅｔｗｏｒｋ.
	};

	// 設定値変換用Tbl.
	struct {
		TYPE_ChannelStatus e_Type;
		DEVD_ChannelStatus uc_Type;
	} static s_tbl_type[] = {
		{TYPE_ChannelStatusNotInstalled,		DEVD_ChannelStatusNotInstalled},		// ネットワーク状態：未インストール.
		{TYPE_ChannelStatusInitializeing,		DEVD_ChannelStatusInitializeing},		// ネットワーク状態：初期化中.
		{TYPE_ChannelStatusReady,				DEVD_ChannelStatusReady},				// ネットワーク状態：接続中.
		{TYPE_ChannelStatusOffLine,				DEVD_ChannelStatusOffLine},				// ネットワーク状態：オフライン.
		{TYPE_ChannelStatusDisconnect,			DEVD_ChannelStatusDisconnect},			// ネットワーク状態：未接続.
		{TYPE_ChannelStatusError,				DEVD_ChannelStatusError},				// ネットワーク状態：ネットワーク異常.
		{TYPE_ChannelStatusDisabled,			DEVD_ChannelStatusDisabled},			// ネットワーク状態：無効.
		{TYPE_ChannelStatusSeachingNetwareEntry,DEVD_ChannelStatusSeachingNetwareEntry},// ネットワーク状態：ＮｅｔＷａｒｅエントリ検索中.
	};

	DEVD_ChannelStatus uc_Type = DEVD_ChannelStatusNotInstalled;	// 未インストール.

	APIC_NetworkDevice *pc_Instance = APIC_NetworkDevice::newInstance();	// インスタンス取得.
	if (pc_Instance != NULL) {

		// パラメータ変換.
		uint32_t ul_tbl_size = sizeof(s_tbl_deviceid) / sizeof(s_tbl_deviceid[0]);
		for (uint32_t i = 0; i < ul_tbl_size; i++) {
			if (s_tbl_deviceid[i].ul_DeviceId == ul_DeviceId) {

				TYPE_ChannelStatus e_Type = pc_Instance->getChannelStatus(s_tbl_deviceid[i].e_DeviceId);

				// 設定値変換.
				ul_tbl_size = sizeof(s_tbl_type) / sizeof(s_tbl_type[0]);
				for (i = 0; i < ul_tbl_size; i++) {
					if (s_tbl_type[i].e_Type == e_Type) {
						uc_Type = s_tbl_type[i].uc_Type;
						break;
					}
				}
				break;
			}
		}

	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "End(uc_Type=%d)", uc_Type);
	return uc_Type;
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]HDD状態取得処理.
//引数　　：ul_DeviceId			[in] デバイスID.
//戻り値　：DEVD_HddStatus		HDD状態.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
DEVD_HddStatus DEVC_DeviceService::getHddStatus(uint32_t ul_DeviceId)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "Start(ul_DeviceId=0x%x)", ul_DeviceId);

	// 設定値変換用Tbl.
	struct {
		TYPE_HddStatus e_Type;
		DEVD_HddStatus uc_Type;
	} static s_tbl_type[] = {
		{TYPE_HddStatusNon,						DEVD_HddStatusNon},						// ＨＤＤ状態：ＨＤＤ無し.
		{TYPE_HddStatusConnecting,				DEVD_HddStatusConnecting},				// ＨＤＤ状態：接続中.
		{TYPE_HddStatusNonFormat,				DEVD_HddStatusNonFormat},				// ＨＤＤ状態：未フォーマット.
		{TYPE_HddStatusFormated,				DEVD_HddStatusFormated},				// ＨＤＤ状態：フォーマット済み.
		{TYPE_HddStatusFull,					DEVD_HddStatusFull},					// ＨＤＤ状態：ＨＤＤ容量ＦＵＬＬ.
		{TYPE_HddStatusSleep,					DEVD_HddStatusSleepz},					// ＨＤＤ状態：ＨＤＤスリープ中（この時はHDDにアクセス出来ない）.
		{TYPE_HddStatusConnectEndDocWait,		DEVD_HddStatusConnectEndDocWait},		// ＨＤＤ状態：HDDの初期化完了からDOCの初期完了まで.
		{TYPE_HddStatusNowUpdatingAddressVer,	DEVD_HddStatusNowUpdatingAddressVer},	// ＨＤＤ状態：アドレスバージョンアップ中.
	};

	DEVD_HddStatus uc_Type = DEVD_HddStatusNon;	// ＨＤＤ無し.

	APIC_HddInfo *pc_Instance = APIC_HddInfo::newInstance();	// インスタンス取得.
	if (pc_Instance != NULL) {

		TYPE_HddStatus e_Type = pc_Instance->getHddStatus();

		// 設定値変換.
		uint32_t ul_tbl_size = sizeof(s_tbl_type) / sizeof(s_tbl_type[0]);
		for (uint32_t i = 0; i < ul_tbl_size; i++) {
			if (s_tbl_type[i].e_Type == e_Type) {
				uc_Type = s_tbl_type[i].uc_Type;
				break;
			}
		}

	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "End(uc_Type=%d)", uc_Type);
	return uc_Type;
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]キーカウンタ接続状態取得処理.
//引数　　：ul_DeviceId			[in] デバイスID.
//引数　　：ps_DevInfo			[out] デバイス接続情報.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getKeyCounterConnect(uint32_t ul_DeviceId, DEVS_DeviceInfo *ps_DevInfo)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "Start(ul_DeviceId=0x%x, ps_DevInfo=%p)", ul_DeviceId, ps_DevInfo);

	if (ps_DevInfo != NULL) {

		// 初期値設定.
		ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOff;	// 未接続.
		ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOff;	// OFF設定.

		APIC_MachineSetting *pc_MachineSetting = APIC_MachineSetting::newInstance();	// インスタンス取得.
		if ((pc_MachineSetting != NULL) && (ul_DeviceId == DEVD_DeviceID_KeyCounter)) {

			Bool bIsConnect = APIC_KeyDevice::isKeyCounterCopyEnable();
			TYPE_KeyMode e_KeyMode = pc_MachineSetting->getKeyMode();

			if (bIsConnect == True) { ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOn; }	// 接続.
			if (e_KeyMode == TYPE_KeyModeKeyCounter) {ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOn; }	// ON設定.

		}

		DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "uc_DeviceConnect=%d, uc_DeviceUsed=%d", ps_DevInfo->uc_DeviceConnect, ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed);

	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]ベンダー装置接続状態取得処理.
//引数　　：ul_DeviceId			[in] デバイスID.
//引数　　：ps_DevInfo			[out] デバイス接続情報.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getVendorConnect(uint32_t ul_DeviceId, DEVS_DeviceInfo *ps_DevInfo)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "Start(ul_DeviceId=0x%x, ps_DevInfo=%p)", ul_DeviceId, ps_DevInfo);

	if (ps_DevInfo != NULL) {

		// 初期値設定.
		ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOff;	// 未接続.
		ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOff;	// OFF設定.

		APIC_MachineSetting *pc_MachineSetting = APIC_MachineSetting::newInstance();	// インスタンス取得.
		if (pc_MachineSetting != NULL) {

			Bool bIsConnect = APIC_KeyDevice::isVendorConnect();
			TYPE_KeyMode e_KeyMode = pc_MachineSetting->getKeyMode();

			switch (e_KeyMode) {
				case TYPE_KeyModeVendor:			// ベンダー（従来タイプ）.
					if (ul_DeviceId != DEVD_DeviceID_Vendor) { break; }
					if (bIsConnect == True) { ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOn; }	// 接続.
					ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOn;	// ON設定.
					break;
				case TYPE_KeyModeVendorKM:			// ベンダー（統合タイプ）.
					if (ul_DeviceId != DEVD_DeviceID_VendorKM) { break; }
					if (bIsConnect == True) { ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOn; }	// 接続.
					ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOn;	// ON設定.
					break;
				case TYPE_KeyModePageAcses:			// ページアクセス.
					if (ul_DeviceId != DEVD_DeviceID_PageAcses) { break; }
					if (bIsConnect == True) { ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOn; }	// 接続.
					ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOn;	// ON設定.
					break;
				default:
					break;
			}

		}

		DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "uc_DeviceConnect=%d, uc_DeviceUsed=%d", ps_DevInfo->uc_DeviceConnect, ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed);

	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]管理装置接続状態取得処理.
//引数　　：ul_DeviceId			[in] デバイスID.
//引数　　：ps_DevInfo			[out] デバイス接続情報.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getAdminMachineConnect(uint32_t ul_DeviceId, DEVS_DeviceInfo *ps_DevInfo)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "Start(ul_DeviceId=0x%x, ps_DevInfo=%p)", ul_DeviceId, ps_DevInfo);

	if (ps_DevInfo != NULL) {

		// 初期値設定.
		ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOff;	// 未接続.
		ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOff;	// OFF設定.

		APIC_MachineSetting *pc_MachineSetting = APIC_MachineSetting::newInstance();	// インスタンス取得.
		if (pc_MachineSetting != NULL) {

			Bool bIsConnect = APIC_KeyDevice::isAdminMachineConnect();
			TYPE_KeyMode e_KeyMode = pc_MachineSetting->getKeyMode();

			switch (e_KeyMode) {
				case TYPE_KeyModeAdminMachine:		// 管理装置（従来タイプ）.
					if (ul_DeviceId != DEVD_DeviceID_AdminMachine) { break; }
					if (bIsConnect == True) { ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOn; }	// 接続.
					ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOn;	// ON設定.
					break;
				case TYPE_KeyModeAdminMachineKM:	// 管理装置（統合タイプ）.
					if (ul_DeviceId != DEVD_DeviceID_AdminMachineKM) { break; }
					if (bIsConnect == True) { ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOn; }	// 接続.
					ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOn;	// ON設定.
					break;
				case TYPE_KeyModeAuthDevice:		// 認証装置2.
					if (ul_DeviceId != DEVD_DeviceID_AuthDevice) { break; }
					ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOn;	// 接続判定がない為 設定がONの場合 Onとする.
					ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOn;	// ON設定.
					break;
				default:
					break;
			}

		}

		DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "uc_DeviceConnect=%d, uc_DeviceUsed=%d", ps_DevInfo->uc_DeviceConnect, ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed);

	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "End");
}

//関数-------------------------------------------------------------------------.
//概要　　：[Device接続情報取得関数]認証装置接続状態取得処理.
//引数　　：ul_DeviceId			[in] デバイスID.
//引数　　：ps_DevInfo			[out] デバイス接続情報.
//戻り値　：なし.
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
void DEVC_DeviceService::getAuthConnect(uint32_t ul_DeviceId, DEVS_DeviceInfo *ps_DevInfo)
{
	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "Start(ul_DeviceId=0x%x, ps_DevInfo=%p)", ul_DeviceId, ps_DevInfo);

	if (ps_DevInfo != NULL) {

		// 初期値設定.
		ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOff;	// 未接続.
		ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOff;	// OFF設定.

		APIC_AuthDeviceSetting *pc_MachineSetting = APIC_AuthDeviceSetting::newInstance();	// インスタンス取得.
		if (pc_MachineSetting != NULL) {

			Bool bIsConnect = True;	// 認証装置は接続の判断ができない為，常にOnとする.
			TYPE_AUTH_DEVICE_TYPE e_DeviceType = pc_MachineSetting->getAuthDeviceType();

			switch (e_DeviceType) {
				case TYPE_AUTH_DEVICE_NONE:			// デバイス無し.
					break;
				case TYPE_AUTH_DEVICE_ICC:			// ICカードリーダ.
					if (ul_DeviceId != DEVD_DeviceID_AuthICC) { break; }
					if (bIsConnect == True) { ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOn; }	// 接続.
					ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOn;	// ON設定.
					break;
				case TYPE_AUTH_DEVICE_BIO:			// 静脈リーダ.
					if (ul_DeviceId != DEVD_DeviceID_AuthBIO) { break; }
					if (bIsConnect == True) { ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOn; }	// 接続.
					ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOn;	// ON設定.
					break;
				case TYPE_AUTH_DEVICE_BOTH:			// ICカードリーダ + 静脈リーダ.
					if ((ul_DeviceId != DEVD_DeviceID_AuthICC) && (ul_DeviceId != DEVD_DeviceID_AuthBIO)) { break; }
					if (bIsConnect == True) { ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOn; }	// 接続.
					ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOn;	// ON設定.
					break;
				case TYPE_AUTH_DEVICE_LOADABLE_ICC:	// ローダブルICカード認証データ.
					if (ul_DeviceId != DEVD_DeviceID_AuthLodable_ICC) { break; }
					if (bIsConnect == True) { ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOn; }	// 接続.
					ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOn;	// ON設定.
					break;
				case TYPE_AUTH_DEVICE_LOADABLE_BIO:	// ローダブル生体認証データ.
					if (ul_DeviceId != DEVD_DeviceID_AuthLodable_BIO) { break; }
					if (bIsConnect == True) { ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOn; }	// 接続.
					ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOn;	// ON設定.
					break;
				case TYPE_AUTH_DEVICE_BIO2:			// 静脈リーダ(AU-102).
					if (ul_DeviceId != DEVD_DeviceID_AuthBIO2) { break; }
					if (bIsConnect == True) { ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOn; }	// 接続.
					ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOn;	// ON設定.
					break;
				case TYPE_AUTH_DEVICE_PKI:			// PKI認証.
					if (ul_DeviceId != DEVD_DeviceID_AuthPKI) { break; }
					if (bIsConnect == True) { ps_DevInfo->uc_DeviceConnect = DEVD_DeviceConnectOn; }	// 接続.
					ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed = DEVD_DeviceUsedOn;	// ON設定.
					break;
				default:
					break;
			}

		}

		DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "uc_DeviceConnect=%d, uc_DeviceUsed=%d", ps_DevInfo->uc_DeviceConnect, ps_DevInfo->s_DeviceConnectInfo.uc_DeviceUsed);

	}

	DEVSRV_TRACE(DEVD_LOGLEVEL_TRACE3, "End");
}

/* 2012/05/30 S.Wakayama CQ:364359対応 Start */

//関数-------------------------------------------------------------------------.
//概要　　：USBバルクデバイスマウント状態取得処理.
//引数　　：cp_MountPath		[in] チェックパス.
//戻り値　：DEVD_BoolTrue(マウント済)/DEVD_BoolFalse(非マウント).
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
DEVD_Bool DEVC_DeviceService::checkMountStatus( char* cp_MountPath )
{

	FILE*		fp;
	DEVD_Bool	bRet = DEVD_BoolFalse;
	char		mountPathName[DEVSRV_BULKDRV_NAMESZ*2];
	char		*psc_devName, *psc_tmp;
	// reentrant 対応 s.
	char		*psc_Point = NULL;
	// reentrant 対応 e.

	fp  = SFLG_Fopen( DEVSRV_MOUNT_FILE, "r" ) ;
	if ( fp != NULL ) {
	    while ((SFLG_Fgets(mountPathName, sizeof(mountPathName), fp)) != NULL) {	// Coverity Check OK.
			// マウントパス名に完全一致する文字列を探す.
	        if ( NULL != (psc_devName = strstr( mountPathName, cp_MountPath ))) {
				// reentrant 対応 s.
				//psc_tmp = strtok( psc_devName, " ");		// スペースを区切り文字に検索.
				psc_tmp = strtok_r( psc_devName, " ", &psc_Point);		// スペースを区切り文字に検索.
				// reentrant 対応 e.
				if ( psc_tmp != NULL ) {
					if ( strcmp( psc_tmp, cp_MountPath ) == 0 ) {
						bRet = DEVD_BoolTrue;							// マウント状態設定.
						break ;
					}
				}
			}
		}
		SFLG_Fclose( fp ) ;
	}
	return bRet;
}

//関数-------------------------------------------------------------------------.
//概要　　：DS基板接続状態取得処理.
//引数　　：なし.
//戻り値　：DEVD_BoolTrue(接続)/DEVD_BoolFalse(非接続).
//作成者　：.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
/*============================================================================*/
DEVD_Bool DEVC_DeviceService::isAsic_2Exist( void )
{
	DEVD_Bool	bRet = DEVD_BoolFalse;
	// DS基板接続状態取得.
	if( True == THRF_IsAsic_2Exist() ){
		bRet = DEVD_BoolTrue;
	}
	return bRet;
}
/* 2012/05/30 S.Wakayama CQ:364359対応 End */

