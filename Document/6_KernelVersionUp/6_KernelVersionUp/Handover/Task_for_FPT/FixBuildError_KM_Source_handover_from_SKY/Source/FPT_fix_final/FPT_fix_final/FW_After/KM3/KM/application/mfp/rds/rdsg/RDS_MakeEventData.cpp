/*
 * ファイル名	：
 *		RDS_MakeEventData.cpp
 * 機能			：
 *		イベントデータ作成処理クラス
 *
 *
 *
 */
 
 
/***************************************************************************************************
 *   インクルードファイル
 ***************************************************************************************************/
#include <string.h>

/* IJC ヘッダーファイル		---------------------------------*/

// FWリファクタリング APIC_Setting.hの分割.
#include "system/nvd/APIC_Setting_EnablerSetting.h"
#include "system/nvd/APIC_Setting_Management.h"
#include "system/nvd/APIC_Setting_OperationSetting.h"
#include "system/nvd/APIC_Setting_RdsSetting.h"
#include "system/nvd/nvdg/NVDC_Setting_RdsSetting.h"

#include "system/nvd/APIC_Counter.h"
#include "system/prt/APIC_Printer.h"
#include "system/scn/APIC_Scanner.h"
#include "system/cnt/CNTC_CntIf.h"
#include "system/uac/APIC_UserAcount.h"
#include "system/uac/APIC_AuthDeviceSetting.h"
// 2017/06/07 kajitani [MAv5][RQ-XXX] CSRA接続 Start.
#include "system/nvd/APIC_CSRASetting.h"
// 2017/06/07 kajitani [MAv5][RQ-XXX] CSRA接続 End.

/* KMRDS ヘッダーファイル	---------------------------------*/
#include "RDS_MakeEventData.h"
#include "RDS_VRmsc.h"
#include "RDS_Instance.h"
#include "RDS_TskCommMIO.h"
#include "RDS_FwNWDL.h"					/* 02/05/27 */
#include "RDS_WebDAVClient.h"			// HHTP通信　08/02/06 寺内 新規追加 
#include "RDS_WebDAVTask.h"			    // HHTP通信　08/02/06 寺内 新規追加 
#include "RDS_ExportImport.h"			// 2010/07/29 yito UV-00886 Exp/Imp.
#include "RDS_RcvText.h"				// CQNo366843対応 新規追加 sone 2012/07/03.
// 2017/6/27 MAv5 CSRA接続 start.
#include "RDS_CsraConnectionManager.h"
// 2017/6/27 MAv5 CSRA接続 end.
#include "RDS_Util.h"	// 2017/08/08 MAv5 1026700.
// 管理システム統合化開発 RQ-811 要求仕様1 2020/07/30 Sky二口 START.
#include "RDS_CsrcConnectionManager.h"
// 管理システム統合化開発 RQ-811 要求仕様1 2020/07/30 Sky二口 END.
// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/22 Sky梶谷 START.
#include "RDS_RptDataManager.h"
// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/22 Sky梶谷 END.


/***************************************************************************************************
 *   実体の定義 
 ***************************************************************************************************/
RDSC_mkText_ModeSqrmbl			c_mkText_ModeSqrmbl;			// 緊急発信 
RDSC_mkText_ModeSqrmblRecv		c_mkText_ModeSqrmblRecv;		// 緊急回復発信 
RDSC_mkText_ModeWarn			c_mkText_ModeWarn;				// 本体警告発信 
RDSC_mkText_ModeWarnRecvBody	c_mkText_ModeWarnRecvBody;		// 本体警告解除発信 
RDSC_mkText_ModeIni				c_mkText_ModeIni;				// 初期発信 
RDSC_mkText_ModeMnteStrt		c_mkText_ModeMnteStrt;			// メンテ開始発信 
RDSC_mkText_ModeMnteEnd			c_mkText_ModeMnteEnd;			// メンテ終了発信 
RDSC_mkText_ModeUserCall		c_mkText_ModeUserCall;			// 管理者発信 
RDSC_mkText_ModeTimeOut			c_mkText_ModeTimeOut;			// 期日指定発信 
RDSC_mkText_ModeResRDS			c_mkText_ModeResRDS;			// 応答発信 
RDSC_mkText_ModeResOther		c_mkText_ModeResOther;			// 応答発信 
RDSC_mkText_AutoReTrans			c_mkText_ModeAutoReTrans;		// 自動再発信 
RDSC_mkText_ResTimeOut			c_mkText_ModeResTimeOut;		// 応答タイムアウトによる再発信 
RDSC_mkText_ResInternetISW		c_mkText_ModeResInternetISW;	// InternetISW の応答発信 04/11/19 浦崎 
RDSC_mkText_TxResultI_ISW		c_mkText_ModeTxResultI_ISW;		// InternetISW の結果通知発信  
RDSC_mkText_ModePartsReplace	c_mkText_ModePartsReplace;		// 新品交換検出.
RDSC_mkText_ModeAuthInfoChange	c_mkText_ModeAuthInfoChange;	// MFP認証情報変更発信 Minerva UV-00884 MFP認証情報変更発信追加対応 sone.
RDSC_mkText_FinishProCer		c_mkText_FinishProCer;			// プロダクト認証設定の変更完了通知 Minerva UV-00781 プロダクト認証対応 sone.
RDSC_mkText_ModeDebugLogSave	c_mkText_ModeDebugLogSave;		// デバッグログ手動蓄積完了報告発信.	// 2010/08/24 higuchi Minerva UV-00794 【デバッグログ収集】.
RDSC_mkText_ModeExpImpComplete	c_mkText_ModeExpImpComplete;	// エクスポート・インポート完了通知発信	 // 2010/07/29 yito UV-00886 Exp/Imp.
RDSC_mkText_ModeTrblResetStatus	c_mkText_ModeTrblResetStatus;	// トラブル解除状態通知発信	2010/09/13 Minerva UV-00898追加対応 kita 追加.
RDSC_mkText_Result_Lic			c_mkText_Result_Lic;			// ライセンス機能の完了通知 Minerva UV-00683 sone.
RDSC_mkText_InternetISWExStatus	c_mkText_ModeIISWExStatus;		// InternetISW実行状態通知 2010/11/19 Minerva UV-00971 kita 追加.
RDSC_mkText_ModeTemporaryRescue			c_mkText_ModeTemporaryRescue0(TYPE_RdsTrgKindTemporaryRescue0);			// 不良箇所切り離し発信			// 2011/12/01 amiya ZeusMLK UV-01170 暫定救済中のCSRCへの通知.
RDSC_mkText_ModeTemporaryRescue			c_mkText_ModeTemporaryRescue1(TYPE_RdsTrgKindTemporaryRescue1);			// 不良箇所切り離し発信			// 2011/12/01 amiya ZeusMLK UV-01170 暫定救済中のCSRCへの通知.
RDSC_mkText_ModeTemporaryRescueRelease	c_mkText_ModeTemporaryRescueRelease;	// 不良箇所切り離し解除発信		// 2011/12/01 amiya ZeusMLK UV-01170 暫定救済中のCSRCへの通知.
RDSC_mkText_ModeDebugLogNetResult		c_mkText_ModeDebugLogNetResult;			// デバッグログネット配信結果報告発信.	// 2012/03/19 higuchi Minerva/Zeus UV-01225 【デバッグログ拡張機能】.
RDSC_mkText_ModeRmpStartResult	c_mkText_ModeRmpStartResult;	// リモートパネル起動結果発信 2013/12/05 OPS1前田(Sky) ZeusCD UV-01387_リモートパネルの改善(WP213).
RDSC_mkText_ModeRemoteLogResult			c_mkText_ModeRemoteLogResult;			// 2013/10/19 mtoyoda UV-01382 遠隔でのデバッグログ取得の改善対応.
// 2014/09/03 ktakahashi ZeusS 制御構成変更 No.4 HDDミラーリング Start.
RDSC_mkText_ModeWarnRedunParts		c_mkText_ModeWarnRedunParts;		// 冗長化パーツ警告発信.
RDSC_mkText_ModeWarnRecvRedunParts	c_mkText_ModeWarnRecvRedunParts;	// 冗長化パーツ警告解除発信.
// 2014/09/03 ktakahashi ZeusS 制御構成変更 No.4 HDDミラーリング End.
// 2015/06/05 mtoyoda Helios KRQ-101_認証機能有効化対応 START.
RDSC_mkText_ModeAuthFunctionError	c_mkText_ModeAuthFunctionError;		// 認証機能有効化のエラー通知発信.
// 2015/06/05 mtoyoda Helios KRQ-101_認証機能有効化対応 END.
// 2016/09/08 T.Komatsu IT5_4.0 RQ-505_バックアップ機能強化(WP267) START.
RDSC_mkText_ExtMfpBackupServerInfo	c_mkText_ExtMfpBackupServerInfo;
// 2016/09/08 T.Komatsu IT5_4.0 RQ-505_バックアップ機能強化(WP267) END.
RDSC_mkText_ModeCsraSettingState	c_mkText_ModeCsraSettingState;	// CSRA設定状態発信.	// 2017/07/20 [MAv5][RQ-XXX] CSRA設定状態発信.
// 2017/08/08 [MAv5][RQ-XXX] CSRC・CSRAデータチェック Start.
RDSC_mkText_ModeCmdGenError			c_mkText_ModeCmdGenError_CSRC(TYPE_RdsTrgKindCmdGenError_CSRC);					// コマンド生成エラー通知（CSRC）.
RDSC_mkText_ModeCmdGenError			c_mkText_ModeCmdGenError_CSRA_Daily(TYPE_RdsTrgKindCmdGenError_CSRA_Daily);		// コマンド生成エラー通知（CSRA日毎）.
RDSC_mkText_ModeCmdGenError			c_mkText_ModeCmdGenError_CSRA_Weekly(TYPE_RdsTrgKindCmdGenError_CSRA_Weekly);	// コマンド生成エラー通知（CSRA週毎）.
RDSC_mkText_ModeCmdGenError			c_mkText_ModeCmdGenError_CSRA_Monthly(TYPE_RdsTrgKindCmdGenError_CSRA_Monthly);	// コマンド生成エラー通知（CSRA月毎）.
// 2017/08/08 [MAv5][RQ-XXX] CSRC・CSRAデータチェック End.

uint32_t RDSC_TrblMode::ul_LastTotalCnt = 0;
// 2016/03/23 ktakahashi [ZeusS_ZX0]CQ526327_E-mail自動初期発信ありでCSRC設定のインポート・エクスポート後にボタンクリック不可 Start.
uchar RDSC_MakeEventData::uc_TrgEventID[RDSD_EVENTID_SIZE+1] = {0};
// 2016/03/23 ktakahashi [ZeusS_ZX0]CQ526327_E-mail自動初期発信ありでCSRC設定のインポート・エクスポート後にボタンクリック不可 End.


ushort BitData_2_Array( uchar* pSrc, uchar* pDst, uchar* puc_StartTopBit );
ushort Array_2_BitData( uchar* pSrc, uchar* pDst, uchar* puc_StartTopBit );
// 2018/10/18 FPT-DucDH4 IT6_1.0[Eagle] 機器構成変更_No.92 Start.
RDSS_WarnData s_TempData;
uint32_t getTotalPrintCounter();
uint32_t getTotalCopyCounter();
uint32_t getTotalColorCounter();
ushort getTCRvalue(uchar uc_EDIType);
const uint32_t PAR_DATA_SIZE = 11;
// 2018/12/05 FPT-ChiNM IT6_1.0 [Eagle] 機器構成変更_No.92 Added Start.
Bool isMakeTMData = False;
// 2018/12/05 FPT-ChiNM IT6_1.0 [Eagle] 機器構成変更_No.92 Added End.
// 2018/10/18 FPT-DucDH4 IT6_1.0[Eagle] 機器構成変更_No.92 End.

/*==================================================================================================*/
//   警告文字列構造体
/*--------------------------------------------------------------------------------------------------*/
// 04/11/04 浦崎  コマンドとパラメータが割り付けられたため、テーブル中の文字列修正。
//						定着/ICライフCMYK/転写ベルト/オゾン/転写ローラ 
// 04/12/07 浦崎  廃トナーボトルフルの警告発信文字列を修正。ユーザーデータ部に何も入れていなかった。 
// 04/12/09 浦崎  CCDクランプゲイン警告発信文字列を修正。ニアとエラーの区別の無いデータだったのに、 
//                エラーがあるものとして作ってしまっていた。  
/*==================================================================================================*/
// 2014/12/24 ktakahashi ZeusS 機器構成変更_No.40 Start.
#define	RDSD_WarnStat_Num	4	// ワーニング状態種別数.
static const char* _WarnString[][RDSD_WarnStat_Num] =
{
	/* 解除,	ニア,		エラー		プレニア */
	{"TS002",	"TN002",	"TN003",	"---"},		/* RDSE_WarnTrgTonerEmptyY			Yトナーエンプティ							*/
	{"TS004",	"TN004",	"TN005",	"---"},		/* RDSE_WarnTrgTonerEmptyM			Mトナーエンプティ							*/
	{"TS006",	"TN006",	"TN007",	"---"},		/* RDSE_WarnTrgTonerEmptyC			Cトナーエンプティ							*/
	{"TS000",	"TN000",	"TN001",	"---"},		/* RDSE_WarnTrgTonerEmptyK			Kトナーエンプティ							*/
	//---
	{"TR000",	"TO000",	"TO001",	"---"},		/* RDSE_WarnTrgBottleFull			廃トナーボトルフル							*/
	
	#ifdef DEF_TaigaCus //Taigaの場合.
		{"TQ001C",	"TP001C",	"TP011C",	"---"},	/* RDSE_WarnTrgFuserUnitLife		定着ユニットライフ						*/
	#else               //Taiga以外の場合.
		#ifdef DEF_KRONOSS_Cus	//2016/11/07 [KronosS] [WBS 249] [消耗品カウンタ対応] HCL_Pawan Start.
		{"TQ001D",	"TP001D",	"TP011D",	"---"},		/* RDSE_WarnTrgFuserUnitLife	メンテナンスキット						*/
		#else
		{"TQ000B",	"TP000B",	"TP010B",	"---"},	/* RDSE_WarnTrgFuserUnitLife		定着ユニットライフ						*/
		#endif	//2016/11/07 [KronosS] [WBS 249] [消耗品カウンタ対応] HCL_Pawan End.
	#endif
	
	{"YY000",	"YY000",	"YY000",	"---"},		/* RDSE_WarnTrgSpecificCopy			特定原稿									*/
	//---
// 2017/04/28 HCL-Vinay IT5_4.1 AR1016857 Modified Start.
	{"TQ0008",	"TP0008",	"TP0108",	"---"},	/* RDSE_WarnTrgICLifeY				IUライフY									*/
	{"TQ0007",	"TP0007",	"TP0107",	"---"},	/* RDSE_WarnTrgICLifeM				IUライフM									*/
	{"TQ0006",	"TP0006",	"TP0106",	"---"},	/* RDSE_WarnTrgICLifeC				IUライフC									*/
// 2017/04/28 HCL-Vinay IT5_4.1 AR1016857 Modified End.
	{"TQ0009",	"TP0009",	"TP0109",	"TP0309"},	/* RDSE_WarnTrgICLifeK				IUライフK									*/
	//---
	{"---",		"---",		"---",		"---"	},	/* RDSE_WarnTrgTransCleanLife		転写クリーナユニットライフ					*/
	{"TQ000E",	"TP000E",	"TP010E",	"---"},		/* RDSE_WarnTrgTransBeltLife		転写ベルトユニットライフ					*/
	
	#ifdef DEF_TaigaCus //Taigaの場合.
		{"TQ0052",	"TP0052",	"TP0152",	"---"},	/* RDSE_WarnTrgOzonFilterLife		オゾンフィルタライフ						*/
	#else               //Taiga以外の場合.
		{"TQ000C",	"TP000C",	"TP010C",	"---"},	/* RDSE_WarnTrgOzonFilterLife		紙粉除去クリーナ/オゾンフィルタライフ		*/
	#endif
	
	{"TQ000D",	"TP000D",	"TP010D",	"---"},		/* RDSE_WarnTrgTransRollerLife		転写ローラユニットライフ					*/
	//---
	{"TU000D",	"TT000D",	"TT000D",	"---"},		/* RDSE_WarnTrgIRBrightness			CCDクランプゲイン(ニアとエラーで区別なし)	*/
	{"H100",	"G200",		"G200"	,	"---"},		/* RDSE_WarnTrgPM					PMカウンタ警告								*/
	{"H0",		"G0",		"G0"	,	"---"},		/* RDSE_WarnTrgPartsReplacing		任意パーツ交換警告							*/
	
	#ifdef DEF_TaigaCus //Taigaの場合.
		{"TQ0050",	"TP0050",	"TP0150",	"---"},	/* RDSE_WarnTrgICLifeKDevc			DevCライフ(K)								*/
	#else               //Taiga以外の場合.
		{"TQ001B",	"TP001B",	"TP011B",	"---"},	/* RDSE_WarnTrgICLifeKDevc			DevCライフ(K)								*/
	#endif
	
	{"TU001F",	"TT001F",	"TT001F",	"---"},		/* RDSE_WarnTrgFuserTempTrouble		定着センサ温度検出不良警告					*/
	{"---",		"---",		"JF00"	,	"---"},		/* RDSE_WarnTrgMCBJFrequecy			用紙系JAM多発警告							*/
	{"---",		"---",		"JF01"	,	"---"},		/* RDSE_WarnTrgMOBJFrequecy			原稿系JAM多発警告							*/
	{"TQ0013",	"TP0013",	"TP0113",	"---"},		/* RDSE_WarnTrgTonerFilter			トナーフィルタ警告							*/
	//---.
	{"TQ0008",	"TP0008",	"TP0108",	"TP0308"},	/* RDSE_WarnTrgICLifeYDC			DCライフ(Y)									*/
	{"TQ0007",	"TP0007",	"TP0107",	"TP0307"},	/* RDSE_WarnTrgICLifeMDC			DCライフ(M)									*/
	{"TQ0006",	"TP0006",	"TP0106",	"TP0306"},	/* RDSE_WarnTrgICLifeCDC			DCライフ(C)									*/
	//---
	{"TQ001A",	"TP001A",	"TP011A",	"---"},		/* RDSE_WarnTrgICLifeYDevC			DevCライフ(Y)								*/
	{"TQ0019",	"TP0019",	"TP0119",	"---"},		/* RDSE_WarnTrgICLifeMDevC			DevCライフ(M)								*/
	{"TQ0018",	"TP0018",	"TP0118",	"---"},		/* RDSE_WarnTrgICLifeCDevC			DevCライフ(C)								*/
	//---
	{"TQ003E",	"TP003E",	"TP013E",	"---"},		/* RDSE_WarnTrgDevCMaterialLifeC	現像剤ライフ(C)								*/
	{"TQ003F",	"TP003F",	"TP013F",	"---"},		/* RDSE_WarnTrgDevCMaterialLifeM	現像剤ライフ(M)								*/
	{"TQ0040",	"TP0040",	"TP0140",	"---"},		/* RDSE_WarnTrgDevCMaterialLifeY	現像剤ライフ(Y)								*/
	{"TQ0041",	"TP0041",	"TP0141",	"---"},		/* RDSE_WarnTrgDevCMaterialLifeK	現像剤ライフ(K)								*/
	//---
	{"TQ0053",	"TP0053",	"TP0153",	"---"},		/* RDSE_WarnTrgPaperDustCleaner		紙粉除去クリーナー							*/
// Minerva UV-00929 kita 2010/10/20 Start.
	{"TW000",	"---",		"TV000"	,	"---"},		/* RDSE_WarnTrgLicenseManagementError		ライセンス管理異常					*/
// Minerva UV-00929 kita 2010/10/20 End.
// 2017/01/05 nakano [MAv5][RQ-710] CSRA接続 RTRUコマンドの通知をDrop Start.
// 2017/07/12 kajitani [MAv5][KRQ-335][AccuRev.1025859] CSRA接続 Start.
//// 2017/06/07 kajitani [MAv5][RQ-XXX] CSRA接続 Start.
//	{"RT000",	"---",		"RU000"	,	"---"},		/* RDSE_WarnTrgPdmPreChk			Pdmシステム警告（プレチェック）				*/
//	{"RT001",	"---",		"RU001"	,	"---"},		/* RDSE_WarnTrgPdmDataSnd			Pdmシステム警告（データ転送）				*/
//// 2017/06/07 kajitani [MAv5][RQ-XXX] CSRA接続 End.
//	{"RU000",	"---",		"RT000"	,	"---"},		/* RDSE_WarnTrgPdmPreChk			Pdmシステム警告（プレチェック）				*/
//	{"RU001",	"---",		"RT001"	,	"---"},		/* RDSE_WarnTrgPdmDataSnd			Pdmシステム警告（データ転送）				*/
// 2017/07/12 kajitani [MAv5][KRQ-335][AccuRev.1025859] CSRA接続 End.
// 2017/01/05 nakano [MAv5][RQ-710] CSRA接続 RTRUコマンドの通知をDrop End.
	{"---",		"---",		"FW"	,	"---"},		/* RDSE_WarnTrgRetryFrequency		リトライ多発警告						*/  // 2019/01/07 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-1916】 Modification Added
// 2022/11/21 FPT-TinhNT16 【IT6_2.2MR4】【OP_BTS-40197】 RTCの時刻ずれ補正対応 Start.
	{"---",		"TA001",	"TA002"	,	"---"},		/*RDSE_WarnTrgRTCTime*/
// 2022/11/21 FPT-TinhNT16 【IT6_2.2MR4】【OP_BTS-40197】 RTCの時刻ずれ補正対応 End.
};
// 2014/12/24 ktakahashi ZeusS 機器構成変更_No.40 End.


/***************************************************************************************************
 *   extern 宣言 
 ***************************************************************************************************/
extern APIC_TotalCnt*     cp_TotalCnt;
extern	uint32_t	RDS_modem_Kind;		// CQ385137 対応 2014/02/03 add by tomiyasu
// 2015/11/25 mtoyoda MinervaSBK CQ521926 CSRC用PHS通信料高額請求対策 Start.
extern Bool RDS_getSoftDipSwBit( const ushort us_SoftDipSwNo, const uchar uc_SoftDipSwBitNo );
// 2015/11/25 mtoyoda MinervaSBK CQ521926 CSRC用PHS通信料高額請求対策 End.

/***************************************************************************************************
 *   関数の定義
 ***************************************************************************************************/
/*==================================================================================================
//	ユーザーデータ作成時のサイズとポインタインクリメント
==================================================================================================*/
inline void _INC_SIZE( uchar** uc_Ptr, ushort* us_Len, ushort us_Size )
{
	*uc_Ptr += us_Size;
	*us_Len = (ushort)((*us_Len) + us_Size);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト START.
}


/*==================================================================================================*/
//	電源 ON 後、KMRDS 動作開始時の処理 
/*==================================================================================================*/
void RDSC_TrblMode::actPort()
{
	// トータルカウンタ(コピー/プリント合計、ブラック/フルカラー/モノカラー／２色合計)
	ul_LastTotalCnt = RDSC_MakeEventData::getTotalCnt();
}


/*==================================================================================================*/
//	CommonAPI のデータ変化を検出する。
/*--------------------------------------------------------------------------------------------------*/
// 04/03/16 浦崎  CommonAPI のトラブル変化を検出するか否かの設定を参照するようにした。
// 04/03/26 浦崎  問題点 3730 対策。メンテ中であっても緊急回復発信は行わないといけなかったのに、
//				  緊急回復発信条件を検出してなかった。 
// 04/03/29 浦崎  ！！！！！ 上記に対して仕様変更 ！！！！！
//                メンテ中の発信制限の仕様が変更され、メンテ中は全ての発信を行わない事になった。
//                よって、04/03/26 の処理は有効とならないようにしないといけない事になったが、
//                この関数コールによって緊急回復発信トリガをイベント処理部に通知しても、
//                発信開始の起動チェックが行われないため、本関数内での対応は特に不要である。 
// 04/04/09 浦崎  4405 対策。スキャナ系のトラブルに関してもチェックするようにした。 
// 04/04/19 浦崎  システムステータスも見るようにした。
// 04/07/21 浦崎  緊急回復発信イベント検出条件のミス修正。
//                (1)「プリンタ、スキャナの両方にトラブル発生していない時」と、しなくてはいけないところ、
//                   「プリンタ、スキャナのどちらかにトラブルが発生していない時」としてしまっていたため
//                    修正(条件を || から && に修正した)。 
//                (2)システムステータスを見るようにした。 
/*==================================================================================================*/
Bool RDSC_TrblMode::chkChanged()
{
	APIC_PrinterWarning*	pc_PrintWarn = APIC_PrinterWarning::newInstance();		// プリンタ警告 
	APIC_ScannerWarning*	pc_ScanWarn = APIC_ScannerWarning::newInstance();		// スキャナ警告 
	APIC_SystemStatus*		pc_SystemStatus	= APIC_SystemStatus::newInstance();		// システムステータス 

	if( False == b_firstChkFlag )
	{
		upDateLastTotalCounter();									// 初回コール時は、CommonAPI のデータを読み出してセット 
		b_firstChkFlag = True;
	}


	// トラブル発生のチェック  
	// ・メンテナンス中はチェックしない。 
	// ・DipSW の設定がトラブル検出しない時はチェックしない。
	if(    True == pc_RdsSetting->getTrblMode()						// トラブルの変化を検出するか？ 
		&& False == pc_RdsSetting->getMaintStat() )					// メンテナンス中ではないか？ 
	{
		uchar uc_Changed = 0;
		/* プリンタエンジン系トラブル */
		if( True == pc_PrintWarn->isOccurFatal() )
		{
			ushort us_Code = pc_PrintWarn->getFatal().us_Code;
			uc_Changed = (uchar)(uc_Changed | chkOccurrTrbl( us_Code ));	// 2010/07/17 sone Minerva warning 対応 ucharにキャスト.
		}
		/* スキャナ系トラブル */
		if( True == pc_ScanWarn->isOccurFatal() )
		{
			ushort us_Code = pc_ScanWarn->getFatal().us_Code;
			uc_Changed = (uchar)(uc_Changed | chkOccurrTrbl( us_Code ));	// 2010/07/17 sone Minerva warning 対応 ucharにキャスト.
		}
		/* システム系トラブル */
		if( True == pc_SystemStatus->isOccurFatal() )
		{
			ushort us_Code = pc_SystemStatus->getFatal().us_Code;
			uc_Changed = (uchar)(uc_Changed | chkOccurrTrbl( us_Code ));	// 2010/07/17 sone Minerva warning 対応 ucharにキャスト.
		}

		if( 0 != uc_Changed )
		{
			upDateLastTotalCounter();								// カウント値更新 → 即ち、トラブルが発生した時のカウント値。   
		}
	}

// 2010/09/14 Minerva UV-00898追加対応 kita Start.
	// インスタンスの取得.
	APIC_MachineSetting* pc_MachineSetting = APIC_MachineSetting::newInstance();
	
	// トラブル解除のチェック 
	// ・トラブル発生の時とは異なり、常にチェックする。 
	if(    True != pc_PrintWarn->isOccurFatal()
		&& True != pc_ScanWarn->isOccurFatal()						/* 04/04/09 浦崎 */
		&& True != pc_SystemStatus->isOccurFatal()    )				/* 04/07/21 浦崎 */
	{
		// CSRCからトラブルリセットが行われた場合.
		if( TYPE_CSRCRemotReset_NotRcvYX != pc_MachineSetting->getCSRCTroubleReset() )
		{
			// トラブル解除状態通知発信が発信された場合.
			if(	True == tskRdsEvent.isTrblRemoteResetTrans()
			&&	TYPE_CSRCRemotReset_NeedTransSutatus == pc_MachineSetting->getCSRCTroubleReset()	)
			{
				// 緊急回復発信を発信する.
				if( c_mkText_ModeSqrmbl.isNotRecover() )
				{
					tskRdsEvent.indTrg( TYPE_RdsTrgKindTrblRecv, 0 );
				}
			}
		}
		// CSRCから遠隔トラブルリセットを行っていない場合.
		else
		{
			// 緊急回復発信条件をチェック 04/01/21 浦崎 
			// トータルカウンタ(コピー/プリント合計、ブラック/フルカラー/モノカラー／２色合計) 
			uint32_t ul_Data = RDSC_MakeEventData::getTotalCnt();
			
			if(	c_mkText_ModeSqrmbl.isNotRecover()				// 緊急回復発信を行う必要がある状態？.
			&&	ul_LastTotalCnt != ul_Data			)			// トラブルが発生した時のカウント値と比較したところ、トータルカウンタの変化を検出！.
			{
				tskRdsEvent.indTrg( TYPE_RdsTrgKindTrblRecv, 0 );
			}
		}
	}

	// CSRCからトラブルリセットが行われた場合.
	if( TYPE_CSRCRemotReset_NotRcvYX != pc_MachineSetting->getCSRCTroubleReset() )
	{
		// トラブル解除状態通知発信が発信された場合.
		if(	True == tskRdsEvent.isTrblRemoteResetTrans()
		&&	TYPE_CSRCRemotReset_NeedTransSutatus == pc_MachineSetting->getCSRCTroubleReset()	)
		{
			// CSRCからトラブルリセットが行われたかのステータスを
			// TYPE_CSRCRemotReset_NotRcvYX(YX000を受信していない状態)に更新する.
			pc_MachineSetting->lock();		// ロック.
			pc_MachineSetting->setCSRCTroubleReset( TYPE_CSRCRemotReset_NotRcvYX );
			pc_MachineSetting->unlock();	// ロック解除.
		
			// トラブル解除状態通知発信タイマーのステータスをタイマー未実行にする.
			tskRdsEvent.e_TRRTransWaitTimerSt = RDSE_TrblRemotResetTransWaitTimerSt_Non;
		}
		else
		{
			// ステータスがTYPE_CSRCRemotReset_RcvYX(YX000を受信している状態)の時且つ、
			// トラブル解除状態通知発信待ちタイマーが起動されていない且つ、
			// トラブルリセット実行後の場合.
			// トラブル解除状態通知発信を発信する必要がある為、タイマーを起動する.
			if(	TYPE_CSRCRemotReset_RcvYX == pc_MachineSetting->getCSRCTroubleReset()
			&&	RDSE_TrblRemotResetTransWaitTimerSt_Non == tskRdsEvent.e_TRRTransWaitTimerSt
			&&	False == tskRdsEvent.b_notifyTrblRemoteResetForSystem )
			{
				// トラブル解除状態通知発信待ちタイマー起動.
				tskRdsEvent.start_TrblRemoteResetWaitTimer();
			}
		}
	}
// 2010/09/14 Minerva UV-00898追加対応 kita End.


	return( True );
}

/*==================================================================================================*/
//	発生中のトラブルを取り込むか否かの判断。
//		プリンタ系のトラブル、スキャナ系のトラブルを一つの関数で処理するためのルーチン。 
/*--------------------------------------------------------------------------------------------------*/
// 04/04/09 浦崎  
/*==================================================================================================*/
Bool RDSC_TrblMode::chkOccurrTrbl( ushort us_Code )
{
	ushort us_TrblCode0 = pc_RdsSetting->getTrblCode( 0 );
	ushort us_TrblCode1 = pc_RdsSetting->getTrblCode( 1 );

	Bool   b_Changed = False;
	if(    us_Code != us_TrblCode1							/* 1 でバックアップしているものと違うかつ、 */
		&& Trb_EMP == us_TrblCode0							/* トラブルコード0が空の場合 かつ、 */
		&& us_Code != us_TrblCode0    )						/* バックアップしているコードと違う場合*/
	{
		b_Changed = True;
	}
	if(    False == b_Changed
		&& us_Code != us_TrblCode0							/* 0 でバックアップしているものと違うかつ、 */
		&& Trb_EMP == us_TrblCode1							/* トラブルコード1が空の場合かつ、 */
		&& us_Code != us_TrblCode1    )						/* バックアップしているコードと違う場合*/
	{
		b_Changed = True;
	}

//2021/11/19 FPT-TinhNT16 【IT6-2.2MR1】【OP_BTS-36534】 Start.
	APIC_SystemInfo* pc_SystemInfo	= APIC_SystemInfo::newInstance();
	pc_SystemInfo->lock();
	TYPE_MachineType e_Type = pc_SystemInfo->getMachineType();
	pc_SystemInfo->unlock();
//2021/11/19 FPT-TinhNT16 【IT6-2.2MR1】【OP_BTS-36534】 End.
	if( True == b_Changed )
	{
//2019/11/21 【FPT-LucNH2】【IT6_2.0】【EAGLEH】 【RQ-845_初期使用時の管理者パスワード変更】 Start.
		if( TYPD_TroubleCode_EF01 == us_Code )
		{
			// Do not notify to CSRC server
			return False;
		}

//2021/11/19 FPT-TinhNT16 【IT6-2.2MR1】【OP_BTS-36534】 Start.
		if(( TYPE_MachineTypeEagleZ == e_Type )
		|| ( TYPE_MachineTypeEagleX == e_Type )
		|| ( TYPE_MachineTypeEagle0 == e_Type )
		|| ( TYPE_MachineTypeEagleBKZ == e_Type )
		|| ( TYPE_MachineTypeEagleBKX == e_Type )
		|| ( TYPE_MachineTypeEagleBK0 == e_Type )
		|| ( TYPE_MachineTypeEagleZplus == e_Type )
		|| ( TYPE_MachineTypeEagleBKZplus == e_Type )
		|| ( TYPE_MachineTypeMinervaSSBKX == e_Type )
		|| ( TYPE_MachineTypeMinervaSSBK0_US == e_Type )
		|| ( TYPE_MachineTypeMinervaSSBK0_EU == e_Type )) {
			//Nop.
		} else {
			if( (TYPD_TroubleCode_1124 == us_Code) || (TYPD_TroubleCode_1130 == us_Code) )
			{
				// Do not notify to CSRC server.
				return False;
			}
		}
//2021/11/19 FPT-TinhNT16 【IT6-2.2MR1】【OP_BTS-36534】 End.
//2019/11/21 【FPT-LucNH2】【IT6_2.0】【EAGLEH】 【RQ-845_初期使用時の管理者パスワード変更】 End.
		tskRdsEvent.indTrg( TYPE_RdsTrgKindTrbl, us_Code );		// トラブルコードはこの関数の中で不揮発に記憶される。同時に緊急発信トリガも発生される。 
	}

	return( b_Changed );
}

//==================================================================================================.
// トラブルコードとパーツ情報を保存する関数の空いているindexを探す関数(不良箇所切り離し発信用).
//==================================================================================================.
static RDSE_TemporaryRescueSend SearchEmptyIndex()
{
	EPN_DBG_MESSAGE((uchar*)"EVENT", (uint32_t)0, (uchar*)"##### SearchEmptyIndex() START ##### !");
	RDSE_TemporaryRescueSend e_TemporaryRescueSend = RDSE_TemporaryRescueSend_None;					// 戻り値用変数.
	
	// NVDに格納する用の構造体(index0/index1).
	TYPS_TemporaryRescueNotifyInfo	s_TemporaryRescueNotifyInfo_index0;
	memset( &s_TemporaryRescueNotifyInfo_index0, 0, sizeof(TYPS_TemporaryRescueNotifyInfo) );		// 初期化.
	TYPS_TemporaryRescueNotifyInfo	s_TemporaryRescueNotifyInfo_index1;
	memset( &s_TemporaryRescueNotifyInfo_index1, 0, sizeof(TYPS_TemporaryRescueNotifyInfo) );		// 初期化.

	// NVDから保存しているトラブルコードとパーツ情報を取得.
	s_TemporaryRescueNotifyInfo_index0 = pc_RdsSetting->getTemporaryRescueCSRCNoticeUnit( 0 );		// index0.
	s_TemporaryRescueNotifyInfo_index1 = pc_RdsSetting->getTemporaryRescueCSRCNoticeUnit( 1 );		// index1.

	// index0が空いている場合.
	if( s_TemporaryRescueNotifyInfo_index0.e_TemporaryRescue == TYPE_TemporaryRescueUnitNone )
	{
		e_TemporaryRescueSend = RDSE_TemporaryRescueSend_Index0;
	}
	// index0が空いておらず、index1が空いている場合.
	else if( s_TemporaryRescueNotifyInfo_index1.e_TemporaryRescue == TYPE_TemporaryRescueUnitNone )
	{
		e_TemporaryRescueSend = RDSE_TemporaryRescueSend_Index1;
	}
	else	// index0もindex1も空いていない場合.
	{
		e_TemporaryRescueSend = RDSE_TemporaryRescueSend_None;
	}
	 
	 return e_TemporaryRescueSend;
}

/*==================================================================================================*/
//	トラブル発生通知（トラブルコードのセット）
/*--------------------------------------------------------------------------------------------------*/
// 04/03/12 浦崎  受付時間を引数にて通知するように変更。トリガ単位で記憶する時間と合わせるため。
/*==================================================================================================*/
Bool RDSC_TrblMode::occurrTrbl( ushort us_Code, RDSC_Time c_rdsTime )
{
	EPN_DBG_MESSAGE((uchar*)"RDSC_TrblMode::occurrTrbl", (uint32_t)0, (uchar*)"##### START ##### !");
	uchar		bcd_TrgTime[6+1];
	register Bool	ret = False;
	ushort	us_TrblCode0 = pc_RdsSetting->getTrblCode( 0 );
	ushort	us_TrblCode1 = pc_RdsSetting->getTrblCode( 1 );


	if(    Trb_EMP == us_TrblCode0							/* トラブルコード0が空の場合 かつ、 */
		&& us_Code != us_TrblCode1    )						/* 1 で バックアップしているコードと違う場合*/
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", us_Code, (uchar*)" トラブルコード[0]受付け" );


		/* 受付時間は、引数で渡される。 */
		c_rdsTime.cnvTime_2_String( bcd_TrgTime, RDSE_T2S_BCDyymmddhhmmss );

#if 1 // 緊急発信トリガ時間テスト 
		printEpTime( (RDSS_DetailTime)c_rdsTime,			(uchar*)"occurrTrbl()の時間" );
#endif
		pc_RdsSetting->setResIDTrbl0( bcd_TrgTime );


		pc_RdsSetting->setTrblCode( 0, us_Code );								/* トラブルコードセット */
		pc_RdsSetting->setTrblNotRecverFlag( 0, False );						/* トラブルコード未通知状態をセット */
		b_preTransEnd[0] = False;
		pc_RdsSetting->setNeedTrblRecoverFlag( True );							/* 緊急回復発信が必要な状態となったよ */


		ret = True;
	}


	// デバッグのために変数に一度値をセットするようにした。 
	us_TrblCode0 = pc_RdsSetting->getTrblCode( 0 );
	us_TrblCode1 = pc_RdsSetting->getTrblCode( 1 );
	if(    Trb_EMP == us_TrblCode1							/* トラブルコード1が空の場合かつ、 */
		&& us_Code != us_TrblCode0    )						/* 0 で バックアップしているコードと違う場合*/
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", us_Code, (uchar*)" トラブルコード[1]受付け" );


		/* 受付時間 */
		c_rdsTime.cnvTime_2_String( bcd_TrgTime, RDSE_T2S_BCDyymmddhhmmss );
		pc_RdsSetting->setResIDTrbl1( bcd_TrgTime );


#if 1 // 緊急発信トリガ時間テスト 
		printEpTime( (RDSS_DetailTime)c_rdsTime,			(uchar*)"occurrTrbl()の時間" );
#endif


		pc_RdsSetting->setTrblCode( 1, us_Code );								/* トラブルコードセット */
		pc_RdsSetting->setTrblNotRecverFlag( 1, False );						/* トラブルコード未通知状態をセット */
		b_preTransEnd[1] = False;

		pc_RdsSetting->setNeedTrblRecoverFlag( True );							/* 緊急回復発信が必要な状態となったよ */

		
		ret = True;
	}
	
// 2010/08/20 Minerva CQ332195 kita Start.
	// 不揮発にトラブルコードが登録された時に緊急発信が開始されるのでここで.
	// 緊急発信状態を送信中に変更する.
	if( True == ret )
	{
// 2010/09/06 Minerva CQ332769 UV-00134追加対応 kita Start.
		// 2011/12/01 amiya ZeusMLK UV-01170 暫定救済中のCSRCへの通知対応 START.
		// MLK以降のみ発信可能.				// 2012/01/26 amiya 【CQ360949】DipSWを25-00にしてトラブルを発生させて、暫定救済を行うと不良箇所切り離し通知（TXコマンド）が発信される対応.
		// 2015/04/09 mtoyoda Helios warning除去対応.
		//APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
		TYPS_TemporaryRescueNotifyInfo	s_TemporaryRescueNotifyInfo;							// NVDに格納する用の構造体.
		memset( &s_TemporaryRescueNotifyInfo, 0, sizeof(TYPS_TemporaryRescueNotifyInfo) );		// 初期化.
		TYPE_TemporaryRescueUnit e_TemporaryRescueUnit = TYPE_TemporaryRescueUnitNone;			// 一時切り離し対象情報設定(Index0とIndex1で使用する).
		
		e_TemporaryRescueUnit = pc_RdsSysSts->chkTemporaryRescueTroubleWarning( us_Code, 0 );		// 暫定救済可能パーツを取得.
		EPN_DBG_MESSAGE((uchar*)"occurrTrbl()", (uint32_t)e_TemporaryRescueUnit, (uchar*)"= e_TemporaryRescueUnit !");
		EPN_DBG_MESSAGE((uchar*)"occurrTrbl()", (uint32_t)us_Code, (uchar*)"= us_Code !");
		
		if( e_TemporaryRescueUnit != TYPE_TemporaryRescueUnitNone )			// 暫定救済可能パーツがある場合.
		{
			s_TemporaryRescueNotifyInfo.e_TemporaryRescue = e_TemporaryRescueUnit;		// 暫定救済可能パーツに取得したパーツを格納.
			s_TemporaryRescueNotifyInfo.us_TrblCode = us_Code;							// トラブルコードに発生したトラブルコードを格納.
			s_TemporaryRescueNotifyInfo.bl_Send = False;								// 送信済みフラグにFalseを格納.
			
			RDSE_TemporaryRescueSend e_TemporaryRescueSend = SearchEmptyIndex();		// 空いているindexを取得.
			EPN_DBG_MESSAGE((uchar*)"occurrTrbl()", (int32_t)e_TemporaryRescueSend, (uchar*)"= e_TemporaryRescueSend");
			switch( e_TemporaryRescueSend )
			{
				case RDSE_TemporaryRescueSend_Index0:	pc_RdsSetting->setTemporaryRescueCSRCNoticeUnit( 0, s_TemporaryRescueNotifyInfo );		break;		// index0.
				case RDSE_TemporaryRescueSend_Index1:	pc_RdsSetting->setTemporaryRescueCSRCNoticeUnit( 1, s_TemporaryRescueNotifyInfo );		break;		// index1.
				default:	break;
			}
		}
		// 2011/12/01 amiya ZeusMLK UV-01170 暫定救済中のCSRCへの通知対応 END.
	
		// インスタンスの取得.
		APIC_MachineSetting* pc_MachineSetting = APIC_MachineSetting::newInstance();
		
		// トラブル解除可能状態をNGへ変更する.
		pc_MachineSetting->lock();		// ロック.
		pc_MachineSetting->setFatalCancelEnabled( TYPE_FatalCancelEnabledNg );
		pc_MachineSetting->unlock();	// ロック解除.
// 2010/09/06 Minerva CQ332769 UV-00134追加対応 kita End.
		
		// 自動FATAL解除用の緊急発信状態変更：送信していない状態→送信中の状態.
		tskRdsEvent.changeATRTransStatus(
					RSDE_AuteTrblReset_TransSt_Non,
					RSDE_AuteTrblReset_TransSt_Sending
		);
	}
// 2010/08/20 Minerva CQ332195 kita End.

	return( ret );
}

/*==================================================================================================*/
//	トラブル解除通知受付け
/*--------------------------------------------------------------------------------------------------*/
// 04/03/19 浦崎  E-mail版は、応答の確認が取れている場合のみクリアするようにした。 
// 04/03/26 浦崎  E-mail版でも、発信が完了しているトラブルコードに関しては、緊急回復発信イベントにて
//				  破棄するようにした。 
// 05/05/23 浦崎  Tigris問題点5093対策。緊急発信がされる前に緊急回復発信イベントが発生した時の不具合
//                修正。 
/*==================================================================================================*/
Bool RDSC_TrblMode::recoverTrbl( void )
{
	Bool	b_Clear;
	uchar	bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.

	for( register short indx=0; indx<2; indx++ )
	{
		b_Clear = False;

		// トラブルコードをセンターに通知していた場合、それをクリアする。  
		if(    Trb_EMP != pc_RdsSetting->getTrblCode( indx )				// トラブルコード保持状態 
			&& True == pc_RdsSetting->getTrblNotRecverFlag( indx )   )		// かつ、送信済みが記憶されている 
		{
			// 04/03/26 浦崎 
			//if( TYPE_RdsModeMail == pc_RdsSetting->getRdsMode() )
			//{
			//	b_Clear = (True == isIDCleared( indx ) ? False : True);			/ 送信が不要ならば、クリア可能。  
			//}
			//else
			//{
			//	b_Clear = True;
			//}

			b_Clear = True;			// 04/03/26 浦崎 
			memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
			if( True == b_Clear )
			{
				EPN_DBG_MESSAGE( (uchar*)"CSRC", indx, (uchar*)" 送信済みラブルコードをクリア" );
				pc_RdsSetting->setTrblCode( indx, Trb_EMP );
				pc_RdsSetting->setTrblNotRecverFlag( indx, False );		/* 受付け状態解除 */
				b_preTransEnd[indx] = False;

				if( 0 == indx )
				{
					pc_RdsSetting->setResIDTrbl0( bcd_bkEventID );
				}
				else
				{
					pc_RdsSetting->setResIDTrbl1( bcd_bkEventID );
				}
			}
		}
	}
	
	// 2012/02/03 amiya CQ360944 トレイ３／４を暫定救済した場合に、不良箇所切り離し通知発信が発信されない START.
	TYPS_TemporaryRescueNotifyInfo	s_TemporaryRescueNotifyInfo0;							// NVDに格納する用の構造体(index0).
	s_TemporaryRescueNotifyInfo0 = pc_RdsSetting->getTemporaryRescueCSRCNoticeUnit( 0 );	// index0.
	TYPS_TemporaryRescueNotifyInfo	s_TemporaryRescueNotifyInfo1;							// NVDに格納する用の構造体(index1).
	s_TemporaryRescueNotifyInfo1 = pc_RdsSetting->getTemporaryRescueCSRCNoticeUnit( 1 );	// index1.
	
	// 緊急回復発信を行うときにindex0、index1の両方を発信していないときは、
	// トラブル発生後に、暫定救済を行わず通常回復と判断し、NVDに保存している暫定救済情報を初期化する.
	if( ( s_TemporaryRescueNotifyInfo0.bl_Send == False ) && ( s_TemporaryRescueNotifyInfo1.bl_Send == False ) )
	{
		EPN_DBG_MESSAGE((uchar*)"RDSC_TrblMode::recoverTrbl()", (int32_t)0, (uchar*)"NoticeUnitを初期化");
		memset( &s_TemporaryRescueNotifyInfo0, 0, sizeof(TYPS_TemporaryRescueNotifyInfo) );	// 初期化.
		pc_RdsSetting->setTemporaryRescueCSRCNoticeUnit( 0, s_TemporaryRescueNotifyInfo0 );		// NVDの値を初期化.
		pc_RdsSetting->setTemporaryRescueCSRCNoticeUnit( 1, s_TemporaryRescueNotifyInfo0 );		// NVDの値を初期化.
	}
	// 2012/02/03 amiya CQ360944 トレイ３／４を暫定救済した場合に、不良箇所切り離し通知発信が発信されない END.

	// 応答待ち ID が２つともクリア状態であった場合、緊急発信の応答を待つ必要は無いということになる。 
	if(    True == isIDCleared( 0 )
		&& True == isIDCleared( 1 )    )
	{
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( TYPE_RdsTrgKindTrbl, bcd_bkEventID );		// 緊急発信の ID を消去 

		// 発信要求マップの[済]を消す。 
		theTrgMap.resetMapByTrgAllPage( TYPE_RdsTrgKindTrbl );				// 05/02/28 浦崎 resetMapByTrg( , 0 ) → resetMapByTrgAllPage() 

		return( True );	//  
	}

	//pc_RdsSetting->setNeedTrblRecoverFlag( False ); 05/05/23 浦崎 ここではなく、緊急回復発信を発信要求マップに書き込めた時に False にする。  
	return( False );
}

/*==================================================================================================*/
//	緊急回復発信の必要あり？ 
/*==================================================================================================*/
Bool RDSC_TrblMode::isNotRecover( void )
{
	return( pc_RdsSetting->getNeedTrblRecoverFlag() );
}

/*==================================================================================================*/
//	送信が必要なデータか？ 
//		基本的に E-mail版でのみ使用する。
//		日時が登録されていないデータは、センターからの応答待ちデータと判断出来る。 
/*==================================================================================================*/
Bool RDSC_TrblMode::isIDCleared( ushort num )
{
	uchar	bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	if( 0 == num )
	{
		pc_RdsSetting->getResIDTrbl0( bcd_bkEventID );	// BCD で yymmddhhmmss 
	}
	else
	{
		pc_RdsSetting->getResIDTrbl1( bcd_bkEventID );	// BCD で yymmddhhmmss 
	}
	// 2010/07/17 sone Minerva warning 対応 charにキャスト.
	return( (char)(bcd_bkEventID[1] == 0 ? True : False) );				// 月(mm)が 0 と言う事は ID は登録されていないと言うこと。 
}

/*==================================================================================================*/
//	トラブルコードの取得(テキスト作成時のみコールすること)
/*==================================================================================================*/
ushort RDSC_TrblMode::getCode( ushort num )
{
	if( num < 2 )
	{
		if(    False == pc_RdsSetting->getTrblNotRecverFlag( num )
			&& Trb_EMP != pc_RdsSetting->getTrblCode( num )    )
		{
			EPN_DBG_MESSAGE( (uchar*)"CSRC", num, (uchar*)" トラブルコード<仮>送信済みとする。" );
			b_preTransEnd[num] = True;												// 仮送信済みとする。 

			// EPN_DBG_MESSAGE( (uchar*)"CSRC", num, (uchar*)" トラブルコード送信済みとする。" );
			// 【問題点3323】pc_RdsSetting->setTrblNotRecverFlag( num, True );		// 送信済みとする。 
		}

		// ↑のif文は、b_preTransEnd[num] をセットするためのもの。トラブルコード自体は、
		// 必ずゲット出来る。
		return( pc_RdsSetting->getTrblCode( num ) );
	}

	return( Trb_EMP );
}

/*==================================================================================================*/
//	トータルカウンタ更新：緊急回復発信イベント検出用 
/*==================================================================================================*/
void RDSC_TrblMode::upDateLastTotalCounter()
{
	ul_LastTotalCnt = RDSC_MakeEventData::getTotalCnt();		// 初回コール時は、CommonAPI のデータを読み出してセット 
}

/*==================================================================================================*/
// 仮送信済みであったものを送信済みにする。
/*--------------------------------------------------------------------------------------------------*/
// 04/03/18 浦崎  問題点 3323 対策。
/*==================================================================================================*/
void RDSC_TrblMode::JobNormalEnd( void )
{
	// 仮送信済みであったものを送信済みにする。 
	for( register short indx=0; indx<2; indx++ )
	{
		if( True == b_preTransEnd[indx] )
		{
			pc_RdsSetting->setTrblNotRecverFlag( indx, True );		/* 送信済みとする。 */
			EPN_DBG_MESSAGE( (uchar*)"CSRC", indx, (uchar*)" トラブルコード送信済みとする。" );
			b_preTransEnd[indx] = False;							// 仮送信済みはリセット 
		}
	}
}

/*==================================================================================================*/
//	RAMクリア時のクリア
/*==================================================================================================*/
void RDSC_TrblMode::init( void )
{
	pc_RdsSetting->setNeedTrblRecoverFlag( False );
	for( register short indx=0; indx<2; indx++ )
	{
		pc_RdsSetting->setTrblNotRecverFlag( indx, False );			/* 受付け状態解除 */
		pc_RdsSetting->setTrblCode( indx, Trb_EMP );				/* トラブルコードクリア */
		b_preTransEnd[indx] = False;
	}
}


/***************************************************************************************************
 *   警告
 ***************************************************************************************************/
/*==================================================================================================*/
//	コンストラクタ
/*==================================================================================================*/
RDSC_WarnData::RDSC_WarnData()
{
	registRing( &s_Data[0], _S_WarnData );	/* リングバッファ登録 */
	memset( uc_warnData, 0, sizeof(uc_warnData) );	// 2011/02/07 higuchi Minerva/Zeus 【静的解析 影響度高 CID_023769】.
	_ss_dataNum = 0;								// 2011/02/07 higuchi Minerva/Zeus 【静的解析 影響度高 CID_023769】.
	// FQ****** 警告多発対応 yito 2011/09/01 start.
	b_RepeatedOccured = False;			// 同一警告多発状態を示す.
	b_DisableOccuredCheck = False;		// 同一警告チェックをしない.
	uc_EmptyWarnBufferCount = 0;
	memset( s_chkWarnData, 0, sizeof(s_chkWarnData) );
	// FQ****** 警告多発対応 yito 2011/09/01 end  .
	pc_FixedValueDefine = NULL;	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_431757.
	b_dbgFalg = False;	// BSKK要望のデバッグコマンド sone 2012/09/12. 2012/11/28 tadamasa ZeusBK 静的解析 影響度高 CID_431757.

	// 2015/11/25 mtoyoda MinervaSBK CQ521926 CSRC用PHS通信料高額請求対策 Start.
	b_PCallSendFuserTempTrouble = False;		// 警告送信（発信）済みフラグ（P-32 定着センサ温度検出不良）.
	ul_PCallSendLimitResetTime  = 0UL;			// Pコール警告の送信制限リセット日時（基準時刻からの秒数）.
	// 2015/11/25 mtoyoda MinervaSBK CQ521926 CSRC用PHS通信料高額請求対策 End.
}

/*==================================================================================================*/
//	バッファのクリア
/*==================================================================================================*/
void RDSC_WarnData::clearCurrentBuff( void )
{
	memset( wt, 0, sizeof(RDSS_WarnData) );
}
/*==================================================================================================*/
//	RAMクリア時のクリア
/*==================================================================================================*/
void RDSC_WarnData::init( void )
{
	// 2015/04/09 mtoyoda Helios warning除去対応.
	//int test = 0;
	//test = sizeof(s_Data);
	//test = sizeof(uc_warnData);
	memset( s_Data, 0, sizeof(s_Data) );
	// FQ****** 警告多発対応 yito 2011/09/01 start.
	b_RepeatedOccured = False;	// 同一警告多発状態を示す.
	uc_EmptyWarnBufferCount = 0;
	memset( s_chkWarnData, 0, sizeof(s_chkWarnData) );
	// FQ****** 警告多発対応 yito 2011/09/01 end  .
	memset( uc_warnData, 0, sizeof(uc_warnData) );
	
	__saveCommonAPI( E_WarnALL );

	// JAM多発状態のクリア.
	CNTC_CounterIf::newInstance()->JamDailyCounterClearReq();
	pc_RdsSetting->setRdsMCBJStat( TYPE_RdsWarnStatNormal );
	pc_RdsSetting->setRdsMOBJStat( TYPE_RdsWarnStatNormal );
	
	/*uc_warnDataの初期化処理を追加 2008/4/28 山形*/
	uc_warnData[RDSE_WarnTrgMCBJFrequecy] = TYPE_RdsWarnStatNormal;
	uc_warnData[RDSE_WarnTrgMOBJFrequecy] = TYPE_RdsWarnStatNormal;
	__saveCommonAPI( E_WarnStat );
	
	// 2015/11/25 mtoyoda MinervaSBK CQ521926 CSRC用PHS通信料高額請求対策 Start.
	b_PCallSendFuserTempTrouble = False;		// 警告送信（発信）済みフラグ（P-32 定着センサ温度検出不良）.
	// リセット日時については、RAMクリア時に初期化しない.
	// （電源投入後（再起動後）24時間ごとにリセットという仕様となっているため）.
	// 2015/11/25 mtoyoda MinervaSBK CQ521926 CSRC用PHS通信料高額請求対策 Start.
}

// 2015/11/25 mtoyoda MinervaSBK CQ521926 CSRC用PHS通信料高額請求対策 Start.
/*--------------------------------------------------------------------------------------------------*/
//	Pコール警告発信の制限（送信済みフラグ、リセット日時）をリセットする.
/*--------------------------------------------------------------------------------------------------*/
void RDSC_WarnData::resetPCallSendLimit( void )
{
	RDSC_Time c_Now = theVRmsc.getNowTime();				// 現在時刻（CSRC時刻）を取得する.

	// リセット日時
	ul_PCallSendLimitResetTime  = c_Now.rdsTimeToSec();		// リセット日時を現在時刻にセット.

	// 送信済みフラグ.
	b_PCallSendFuserTempTrouble = False;					// 警告送信（発信）済みフラグ（P-32 定着センサ温度検出不良）.
}
/*--------------------------------------------------------------------------------------------------*/
// Pコール警告発信の制限の設定値を取得する.
// 戻り値が True  なら「制限する」.
// 戻り値が False なら「制限しない」.
/*--------------------------------------------------------------------------------------------------*/
Bool RDSC_WarnData::chkPCallSendLimitSetting( void )
{
	Bool b_DipSwBit = False;
	Bool b_Setting  = False;

	// DipSW No.174 bit0 の値を取得する.
	// 0x00：制限する（デフォルト）.
	// 0x01：制限しない.
	b_DipSwBit = RDS_getSoftDipSwBit( RDSD_PCALL_SEND_DIPSW_NVRAM_NO, RDSD_PCALL_SEND_DIPSW_NVRAM_BIT );

	if( b_DipSwBit == False )		// 制限する.
	{
		b_Setting = True;
	}
	else							// 制限しない.
	{
		b_Setting = False;
	}
	
	return b_Setting;
}
/*--------------------------------------------------------------------------------------------------*/
// 前回のリセット日時からの経過時刻を計算する.
/*--------------------------------------------------------------------------------------------------*/
uint32_t RDSC_WarnData::calcPCallElapsedTime()
{
	uint32_t ul_ElapsedTime = 0UL;
	RDSC_Time c_Now      = theVRmsc.getNowTime();
	
	// 現在時刻を秒数に変換し、前回のリセット日時との差分を計算する.
	ul_ElapsedTime = c_Now.rdsTimeToSec() - ul_PCallSendLimitResetTime;

	// 次のAPIにて、現在時刻を秒数でダイレクトに取得することも可能である。.
	// time_t SYSC_RDSTimer::getCurrentTime().
	// しかしながら、本APIの戻り値の型は time_t 型となっている。.
	// time_t 型のサイズは環境に依存するため不採用とした。.
	
	return ul_ElapsedTime;
}
/*--------------------------------------------------------------------------------------------------*/
// Pコール警告の送信済みフラグを取得する.
/*--------------------------------------------------------------------------------------------------*/
Bool RDSC_WarnData::getPCallSendFlag( RDSE_WarnTrg e_kind )
{
	Bool b_Send = False;
	
	if( e_kind == RDSE_WarnTrgFuserTempTrouble )	// P-32 定着センサ温度検出不良警告発信.
	{
		b_Send = b_PCallSendFuserTempTrouble;
	}
	
	return b_Send;
}
/*--------------------------------------------------------------------------------------------------*/
// Pコール警告の送信済みフラグをセットする.
/*--------------------------------------------------------------------------------------------------*/
void RDSC_WarnData::setPCallSendFlag( RDSE_WarnTrg e_kind )
{
	if( e_kind == RDSE_WarnTrgFuserTempTrouble )	// P-32 定着センサ温度検出不良警告発信.
	{
		b_PCallSendFuserTempTrouble = True;
	}
}
// 2015/11/25 mtoyoda MinervaSBK CQ521926 CSRC用PHS通信料高額請求対策 End.

/*==================================================================================================*/
//	CommonAPI のデータ変化を検出する。
/*==================================================================================================*/
//--------------------------------------------------------------------------------------------------
// CommonAPI の列挙子に変化があった場合や、要素数変化があった場合などのダメージを減らすため、
// テーブルは作らない。
// また、CommonAPI の型を引数に使用せず、uint32_t 型としたのは、CommonAPI に対応できない
// 機械があった場合に関数の型を変更せずに対応が出来るため。
//--------------------------------------------------------------------------------------------------
/*--------------------------------------------------------------------------------------------------*/
// SOD 状態 
/*--------------------------------------------------------------------------------------------------*/
void RDSC_WarnData::sub_chkSODStat( RDSE_WarnTrg e_kind, Bool b_Occurr )
{
// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/22 Sky梶谷 START.
//	// センター登録完了状態のみ 
//	if( TYPE_HostStatRgst != pc_RdsSetting->getHostStat() )
	// センター登録完了状態、またはCSRAレポート発信が利用可能な場合、処理する.
	if ( ( TYPE_HostStatRgst != pc_RdsSetting->getHostStat() ) &&
		 ( True != RDSC_RptDataManager::newInstance()->isAvailableSendReport() ) )
// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/22 Sky梶谷 END.
	{
		return;
	}
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"SOD通知キタ。" );

	if( True == b_Occurr )
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", 0, (uchar*)" SOD " );


		/* 受付時間 */
		//c_rdsTime.cnvTime_2_String( bcd_TrgTime, RDSE_T2S_BCDyymmddhhmmss );
		//pc_RdsSetting->setResIDTrbl1( bcd_TrgTime );


		//printEpTime( (RDSS_DetailTime)c_rdsTime,			(uchar*)"occurrTrbl()の時間" );


		// pc_RdsSetting->setSpecialStat( True );			/ SOD 発生状態を記憶。 
	}

	chkWarn( e_kind, TYPE_RdsWarnStatErr );
}
/*--------------------------------------------------------------------------------------------------*/
// PMカウンタ警告状態 
/*--------------------------------------------------------------------------------------------------*/
void RDSC_WarnData::sub_chkPMStat( RDSE_WarnTrg e_kind, RDSE_PMWarn e_Stat )
{
	APIC_PmCnt*			pc_PmCnt = APIC_PmCnt::newInstance();
	TYPE_RdsWarnStat	e_Level;
	uint32_t				ul_Counter;
	uchar				uc_DetailData[4];


	if(    (True == pc_RdsSetting->getPMStat()  && RDSE_PMWarn_ZeroReset == e_Stat)
		|| (True == pc_RdsSetting->getPMStat()  && RDSE_PMWarn_Normal    == e_Stat)
		|| (False == pc_RdsSetting->getPMStat() && RDSE_PMWarn_Error     == e_Stat)    )
	{

		// 発生 → 解除 もしくは、解除 → 発生 に変化した場合のみ受け付ける。 


		memset( uc_DetailData, 0, sizeof(uc_DetailData) );
		switch( e_Stat )
		{
		case RDSE_PMWarn_ZeroReset :	// ゼロリセット：カウンタ値はALLゼロをセットする。 
			e_Level = TYPE_RdsWarnStatNear;
			pc_RdsSetting->setPMStat( False );
			break;

		case RDSE_PMWarn_Normal :		// 通常解除検出：PMカウンタしきい値の値を合わせてセンターに送信するらしい。 
			e_Level = TYPE_RdsWarnStatNormal;
			ul_Counter = pc_PmCnt->get( TYPE_PmCntLimit );
			DWORD_DIV_BYTE( uc_DetailData, ul_Counter );		// uint32_t のデータを 4バイトの uchar データに分割。 
			pc_RdsSetting->setPMStat( False );
			break;

		case RDSE_PMWarn_Error :		// 警告発生検出：PMカウンタの値を合わせてセンターに送信する。 
			e_Level = TYPE_RdsWarnStatErr;
			ul_Counter = pc_PmCnt->get( TYPE_PmCntPM );
			DWORD_DIV_BYTE( uc_DetailData, ul_Counter );		// uint32_t のデータを 4バイトの uchar データに分割。 
			pc_RdsSetting->setPMStat( True );
			break;
		default :
			return;
			break;
		}


		chkWarn( e_kind, e_Level, uc_DetailData );
	}
}
/*--------------------------------------------------------------------------------------------------*/
// CCDクランプゲイン状態 
/*--------------------------------------------------------------------------------------------------*/
void RDSC_WarnData::sub_chkCCDGeinStat( RDSE_WarnTrg e_kind, Bool b_Occurr )
{
	TYPE_RdsWarnStat	e_Level;
	if( True == b_Occurr )		// 発生：PMカウンタの値を合わせてセンターに送信する。  
	{
		e_Level = TYPE_RdsWarnStatErr;
	}
	else						// 解除：PMカウンタしきい値の値を合わせてセンターに送信するらしい。 
	{
		e_Level = TYPE_RdsWarnStatNormal;
	}

	chkWarn( e_kind, e_Level );
}

/*--------------------------------------------------------------------------------------------------*/
//用紙系、原稿系JAM多発
/*--------------------------------------------------------------------------------------------------*/
void RDSC_WarnData::sub_chkRdsJAMFrequecy( RDSE_WarnTrg e_kind )
{	
	RDSC_Time c_rdsNowTime = theVRmsc.getNowTime();
	RDSC_Time c_rdsRESTTime;
	uchar uc_tmpResetDateJAMFrequecy[6];

	// 文字列から時刻へ変換 
	pc_RdsSetting->getResetDateJAMFrequecy(uc_tmpResetDateJAMFrequecy );
	c_rdsRESTTime.cnvString_2_Time( uc_tmpResetDateJAMFrequecy, RDSE_T2S_BCDyyyymmddhhmm );

	//日付が変わっていた場合 
	if( c_rdsRESTTime.year != c_rdsNowTime.year ||
	    c_rdsRESTTime.month != c_rdsNowTime.month ||
	    c_rdsRESTTime.day != c_rdsNowTime.day )
	{
		// For Debug 080428 
		c_rdsNowTime.cnvTime_2_String( uc_tmpResetDateJAMFrequecy, RDSE_T2S_BCDyyyymmddhhmm );
		pc_RdsSetting->setResetDateJAMFrequecy( uc_tmpResetDateJAMFrequecy );
		CNTC_CounterIf::newInstance()->JamDailyCounterClearReq();
		pc_RdsSetting->setRdsMCBJStat( TYPE_RdsWarnStatNormal );
		pc_RdsSetting->setRdsMOBJStat( TYPE_RdsWarnStatNormal );
		
		/*uc_warnDataの初期化処理を追加 2008/4/28 山形*/
		uc_warnData[RDSE_WarnTrgMCBJFrequecy] = TYPE_RdsWarnStatNormal;
		__saveCommonAPI( E_WarnStat );
		uc_warnData[RDSE_WarnTrgMOBJFrequecy] = TYPE_RdsWarnStatNormal;
		__saveCommonAPI( E_WarnStat );
	}
	else//日付が変わってない場合 
	{
		ushort us_retMCBJ,us_retMOBJ ;
		uchar uc_DipSw21 = pc_RdsSetting->getDipSw(20);
		// 2010/07/17 sone Minerva warning 対応 ushortにキャスト START.
		us_retMCBJ = (ushort)(uc_DipSw21 & (RDSD_BIT0 ));
		us_retMOBJ = (ushort)(uc_DipSw21 & (RDSD_BIT1 ));
		// 2010/07/17 sone Minerva warning 対応 ushortにキャスト START.
		TYPE_RdsWarnStat	e_Level;
		
		switch (e_kind)
		{
			
			case RDSE_WarnTrgMCBJFrequecy :
				if(us_retMCBJ != 0)
				{
					if(TYPE_RdsWarnStatNormal == pc_RdsSetting->getRdsMCBJStat())
					{
						uint32_t lValue = APIC_JamDailyCnt::newInstance()->get( TYPE_JamDailyCntPaper );
						uchar uc_DipSw22 = pc_RdsSetting->getDipSw(21);
						
						if( lValue >= (uint32_t)uc_DipSw22)
						{
							e_Level = TYPE_RdsWarnStatErr;
						}
						else
						{
							e_Level = TYPE_RdsWarnStatNormal;
						}

						chkWarn( e_kind, e_Level );
					
					}
				}
				
				break;
				
			case RDSE_WarnTrgMOBJFrequecy :
				if(us_retMOBJ != 0)
				{
					if(TYPE_RdsWarnStatNormal == pc_RdsSetting->getRdsMOBJStat())
					{
						uint32_t lValue = APIC_JamDailyCnt::newInstance()->get( TYPE_JamDailyCntDocument );
						uchar uc_DipSw23 = pc_RdsSetting->getDipSw(22);
						
						if( lValue >= (uint32_t)uc_DipSw23)
						{
							e_Level = TYPE_RdsWarnStatErr;
						}
						else
						{
							e_Level = TYPE_RdsWarnStatNormal;
						}
						
						chkWarn( e_kind, e_Level );
					
					}
				}
				
				break;
				
			default: 
				
				break; 
		}
	}
}
// 任意交換パーツ状態 
/*--------------------------------------------------------------------------------------------------*/
// 04/09/10 浦崎 正式対応。 
/*--------------------------------------------------------------------------------------------------*/
void RDSC_WarnData::sub_chkPartsReplacingStat( RDSE_WarnTrg e_kind )
{
	static ushort us_ChkCycle = 0;

	if( 5 > us_ChkCycle )
	{
		us_ChkCycle++;
		return;	// 毎回は処理しない。 
	}
	us_ChkCycle = 0;


	// 任意交換パーツ警告のみ、ここで見ておく必要がある。 
	if( RDSE_WarnTrgSpecificCopy == wt->uc_Kind )
	{
		// バッファフル、かつ、最終データが SOD 時は、警告取り込み保留。 
		return;
	}
	APIC_OptionPartsCnt *pc_OptionPartsCnt = APIC_OptionPartsCnt::newInstance();		// 任意パーツカウンタのインスタンスを取得
	TYPS_OptionPartsCnt	s_Data;

	if( 30 != (int)TYPE_OptionPartsCntSize )
	{
		// サイズが 30以上の時は処理しないようにしないと、領域外のバッファに
		// アクセスする事になるので、念のためプロテクトをかける。
		return;
	}


	// まず、現在値を取得。  
	uchar	uc_TempNewStat[8]; // 2020/07/09 FPT-ThanhPC7 Address Sanitizerの不正メモリ参照・メモリ破壊問題 Modification.
	memset( uc_TempNewStat, 0x00, sizeof(uc_TempNewStat) );
	//Declared us_Indx outside for loop, for VC6 to VS2K8 Porting. 2009/10/29 HCLT Start.
	//for( ushort us_Indx=0; us_Indx<30; us_Indx++ )
	  ushort us_Indx=0;
	  for( us_Indx=0; us_Indx<30; us_Indx++ )
    //Declared us_Indx outside for loop, for VC6 to VS2K8 Porting. 2009/10/29 HCLT End.
	{
		uchar uc_bit;
		s_Data = pc_OptionPartsCnt->get( (TYPE_OptionPartsCnt)us_Indx );
		if(    ((0 != s_Data.ul_Limit) && (0 != s_Data.ul_OptionPartsCnt))
			&& (s_Data.ul_Limit <= s_Data.ul_OptionPartsCnt)    )
		{
			uc_bit = 1;		// 警告発生状態。 
		}
		else
		{
			uc_bit = 0;
		}
		// 2010/07/17 sone Minerva warning 対応 ucharにキャスト START.
		uc_TempNewStat[us_Indx/8] = (uchar)(uc_TempNewStat[us_Indx/8] | (uc_bit<<(us_Indx%8)));
	}




	uchar	uc_ClearedStat[4];		// 解除状態が変化したものをビットで記録。 
	uchar	uc_OccuredStat[4];		// 発生状態が変化したものをビットで記録。 
	memset( uc_ClearedStat, 0x00, sizeof(uc_ClearedStat) );
	memset( uc_OccuredStat, 0x00, sizeof(uc_OccuredStat) );

	uchar uc_LastStat[8]; // 2018/05/05 【IT6_Eagle】【1044550】HCLT_Prabhu Modification.
	pc_RdsSetting->getPartsReplacingStat( uc_LastStat );
	uchar  uc_Changed = 0;
	for( us_Indx=0; us_Indx<30; us_Indx++ )
	{
		uchar uc_NewBit;
		uchar uc_LastBit;
		uc_NewBit  = (uchar)(uc_TempNewStat[us_Indx/8] & (0x01<<(us_Indx%8)));
		uc_LastBit = (uchar)(uc_LastStat[us_Indx/8]    & (0x01<<(us_Indx%8)));
		// ● 前回からの【変化】を検出する。 
		if( uc_NewBit == uc_LastBit )
		{
			// 等しい(前の状態と変わってない) 
		}
		else if( uc_NewBit > uc_LastBit )
		{
			// ビットが立っている。⇒ 警告発生 
			uc_Changed |= 0x01;
			uc_OccuredStat[us_Indx/8] = (uchar)(uc_OccuredStat[us_Indx/8] | (0x01<<(us_Indx%8)));	// 2010/07/17 sone Minerva warning 対応 ucharにキャスト.
		}
		else
		{
			// ビットがクリアされている。 ⇒ 警告解除 
			uc_Changed |= 0x02;
			uc_ClearedStat[us_Indx/8] = (uchar)(uc_ClearedStat[us_Indx/8] | (0x01<<(us_Indx%8)));	// 2010/07/17 sone Minerva warning 対応 ucharにキャスト.
		}
	}

	if( uc_Changed & 0x01 )
	{
		chkWarn( e_kind, TYPE_RdsWarnStatErr, uc_OccuredStat );
	}
	if( uc_Changed & 0x02 )
	{
		chkWarn( e_kind, TYPE_RdsWarnStatNormal, uc_ClearedStat );
	}

	// 新しい内容をここで保存する。  
	pc_RdsSetting->setPartsReplacingStat( uc_TempNewStat );

}
//Minerva UV-00929 kita 2010/10/20 Start.
/*--------------------------------------------------------------------------------------------------*/
// ライセンス管理異常状態 
/*--------------------------------------------------------------------------------------------------*/
void RDSC_WarnData::sub_chkRdsLicenseManagementErrorStat( RDSE_WarnTrg e_kind, Bool b_Occurr )
{
	TYPE_RdsWarnStat	e_Level = TYPE_RdsWarnStatNormal;
	if( True == b_Occurr )		// ライセンス管理異常発生.
	{
		e_Level = TYPE_RdsWarnStatErr;
	}
	else						// 解除.
	{
		e_Level = TYPE_RdsWarnStatNormal;
	}

	chkWarn( e_kind, e_Level );
}
// 2018/12/03 FPT-DucDH4 【IT6_2.0】【EagleH】【機器構成変更_No.1】 リトライ多発警告 Start.
void RDSC_WarnData::sub_chkRdsRetryFrequency(RDSE_WarnTrg e_kind)
{
	//EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)e_kind, (uchar*)"##[00]void RDSC_WarnData::sub_chkRdsRetryFrequency(RDSE_WarnTrg e_kind)");
// 2019/01/07 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-1916】 Modification Start
// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される Modidfy Start.
//	static TYPS_PaperFeedRetryCnt s_PrevPaperFeedRetryCnt; 
	TYPS_PaperFeedRetryCnt s_PrevPaperFeedRetryCnt;
// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される Modidfy End.
// 2019/03/10 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-3232】 Modification Start
	TYPS_PaperFeedRetryCnt s_NowPaperFeedRetryCnt; 
// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される Modidfy Start.
//	static TYPS_AlarmStatusInfo s_PrevAlarmStatusInfo;
	TYPS_AlarmStatusInfo s_AlarmStatusInfo;
	//TYPS_AlarmStatusInfo s_NowAlarmStatusInfo; 
// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される Modidfy End.
	APIC_LifeCnt* pc_LifeCnt = APIC_LifeCnt::newInstance();

	// get Parper feeding retry counter
	pc_LifeCnt->lock();
	s_NowPaperFeedRetryCnt.ul_PaperFeedRetry1Cnt		= pc_LifeCnt->get(TYPE_LifeCntPaperFeedRetry1);					//1st stage paper feed retry counter.
	s_NowPaperFeedRetryCnt.ul_PaperFeedRetry2Cnt		= pc_LifeCnt->get(TYPE_LifeCntPaperFeedRetry2);					//2nd stage paper feed retry counter.
	s_NowPaperFeedRetryCnt.ul_PaperFeedRetry3_LCTCnt	= pc_LifeCnt->get(TYPE_LifeCntPaperFeedRetry3_LCT);				//3rd row / LCT paper feed retry counter.
	s_NowPaperFeedRetryCnt.ul_PaperFeedRetry4Cnt		= pc_LifeCnt->get(TYPE_LifeCntPaperFeedRetry4);					//4th sheet feed retry counter.
	s_NowPaperFeedRetryCnt.ul_PaperFeedRetryManualCnt	= pc_LifeCnt->get(TYPE_LifeCntPaperFeedRetryManual); 			//Manual feed paper retry counter.
	s_NowPaperFeedRetryCnt.ul_PaperFeedRetryExtLCTCnt	= pc_LifeCnt->get(TYPE_LifeCntPaperFeedRetryExtLCT); 			//External LCT paper feed retry counter.
	pc_LifeCnt->unlock();
// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される Start.
	//pc_RdsSetting->setPaperFeedRetryCnt(&s_NowPaperFeedRetryCnt);
	pc_RdsSetting->lock();
	pc_RdsSetting->getPaperFeedRetryCnt(&s_PrevPaperFeedRetryCnt);
	pc_RdsSetting->unlock();
// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される End.

	//EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)s_NowPaperFeedRetryCnt.ul_PaperFeedRetry1Cnt, (uchar*)"##[00.1] s_NowPaperFeedRetryCnt.ul_PaperFeedRetry1Cnt");
	//EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry1Cnt, (uchar*)"##[00.1] s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry1Cnt");

	// Update Date/Time
	APIC_SystemSetting* pSystemSetting = APIC_SystemSetting::newInstance();

// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される Start.
	time_t 	t_NowTime = 0;
	tm 		t_NowTimeTm = {};
	TYPC_Time c_NowTime;
	
	//Get current time.
	pSystemSetting->lock();
//	TYPC_Time c_NowTime = pSystemSetting->getSystemTime();
	t_NowTime = pSystemSetting->getSystemTime().getTime();
	pSystemSetting->unlock();
	//Convert current time (time_t -> tm).
	if( False == RDSC_TimeUtil::convertLocalTime((const time_t)t_NowTime,&t_NowTimeTm)){
		return;
	}
	//Convert tm -> TYPC_Time.
	RDSC_TimeUtil::convertTmToTypeC( &t_NowTimeTm, &c_NowTime );


	//Assign checking time = time now
	//EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)"##[01] Time now checking warning");
//	s_NowAlarmStatusInfo.c_PreviousDateChecking.ss_Year			= c_NowTime.ss_Year ;
//	s_NowAlarmStatusInfo.c_PreviousDateChecking.sc_Month		= c_NowTime.sc_Month;
//	s_NowAlarmStatusInfo.c_PreviousDateChecking.sc_Day			= c_NowTime.sc_Day;
//	s_NowAlarmStatusInfo.c_PreviousDateChecking.sc_Hour			= c_NowTime.sc_Hour ;
//	s_NowAlarmStatusInfo.c_PreviousDateChecking.sc_Minute		= c_NowTime.sc_Minute ;
//	s_NowAlarmStatusInfo.b_WarningStatus = False;
	//pc_RdsSetting->setAlarmStatusInfo(&s_NowAlarmStatusInfo);
	pc_RdsSetting->lock();
	pc_RdsSetting->getAlarmStatusInfo(&s_AlarmStatusInfo);
	pc_RdsSetting->unlock();
// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される End.

	TYPE_RdsWarnStat e_Level = TYPE_RdsWarnStatNormal;
// 2019/01/18 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-1916】 Modification Start
// 2019/02/27 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-3235】 Modification Start
	uchar uc_DipSw29 = pc_RdsSetting->getDipSw(28);
	uchar uc_DipSw30 = pc_RdsSetting->getDipSw(29);
	// uchar uc_DipSw29 = pc_RdsSetting->getDipSw(29);
	// uchar uc_DipSw30 = pc_RdsSetting->getDipSw(30);
// 2019/02/27 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-3235】 Modification End
// 2019/01/18 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-1916】 Modification End
	ushort us_retONRetryFrequency = (ushort)(uc_DipSw29 & (RDSD_BIT0));

	//EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)uc_DipSw29, (uchar*)"##[02] uc_DipSw29", (uint32_t)uc_DipSw29);
	Bool b_IsRequiredSendRetryWarning = False;
	
// 2019/05/06 FPT-DongND4 【OP_BTS-5019】【管理No.30】【機器_No.1】 Modification Start.
	// Check time system, only send warning after 23:30
	// Or on case: MFP - Power OFF and Power ON after 00:00 
// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される Modify Start.
	tm t_PreviousDateCheckingTm = {};  // Date previous Checking.
	time_t t_PreviousDateChecking = 0;

	uint32_t ui_ElapsedDay = 0;			// Elapsed day betwen current time and previous time.

	// Convert TYPC_Time -> tm.
	RDSC_TimeUtil::convertTypeCToTm( &s_AlarmStatusInfo.c_PreviousDateChecking, &t_PreviousDateCheckingTm);
	t_PreviousDateChecking = mktime( &t_PreviousDateCheckingTm );
	// Elapsed day.
	ui_ElapsedDay = RDSC_TimeUtil:: getElapsedDay( &t_PreviousDateCheckingTm, &t_NowTimeTm );
	
	RDSD_DBG_MESSAGE_POLLING_STR( "Status =", (uint32_t) t_NowTime, (uint32_t) t_PreviousDateChecking, (uint32_t) ui_ElapsedDay );

	if((c_NowTime.sc_Hour == 23 && c_NowTime.sc_Minute >= 30 && s_AlarmStatusInfo.c_PreviousDateChecking.sc_Day != c_NowTime.sc_Day) ||  //check if after 23:30 to notify.
//	(s_PrevAlarmStatusInfo.c_PreviousDateChecking.sc_Day > 0 && (s_NowAlarmStatusInfo.c_PreviousDateChecking.sc_Day - s_PrevAlarmStatusInfo.c_PreviousDateChecking.sc_Day) >= 1)) 
	(s_AlarmStatusInfo.c_PreviousDateChecking.sc_Day > 0 && ui_ElapsedDay >= 2)) //check if prev day don't notify.
// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される Modify End.
// 2019/05/06 FPT-DongND4 【OP_BTS-5019】【管理No.30】【機器_No.1】 Modification End.
	{
		EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)"##[01] Checking OK! Time to send retry warning");
		b_IsRequiredSendRetryWarning = True;
	}
	//EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)"##[01] Time to checking and send retry warning");
	if (us_retONRetryFrequency != 0 && b_IsRequiredSendRetryWarning == True)
	{
// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される Remove Start.
// 2019/01/18 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-1909】 Modification Start
//		EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)s_NowPaperFeedRetryCnt.ul_PaperFeedRetry1Cnt, (uchar*)"## s_NowPaperFeedRetryCnt.ul_PaperFeedRetry1Cnt", (uint32_t)e_kind);
//		EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry1Cnt, (uchar*)"## s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry1Cnt");
//		//Parper feeding retry counter previous and now.
//		if((s_NowPaperFeedRetryCnt.ul_PaperFeedRetry1Cnt	<= s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry1Cnt) &&
//		(s_NowPaperFeedRetryCnt.ul_PaperFeedRetry2Cnt		<= s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry2Cnt) &&
//		(s_NowPaperFeedRetryCnt.ul_PaperFeedRetry3_LCTCnt	<= s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry3_LCTCnt) &&
//		(s_NowPaperFeedRetryCnt.ul_PaperFeedRetry4Cnt		<= s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry4Cnt) &&
//		(s_NowPaperFeedRetryCnt.ul_PaperFeedRetryManualCnt 	<= s_PrevPaperFeedRetryCnt.ul_PaperFeedRetryManualCnt) &&
//		(s_NowPaperFeedRetryCnt.ul_PaperFeedRetryExtLCTCnt 	<= s_PrevPaperFeedRetryCnt.ul_PaperFeedRetryExtLCTCnt))
//		{
//			uc_warnData[RDSE_WarnTrgRetryFrequency] = TYPE_RdsWarnStatNormal;
//			__saveCommonAPI(E_WarnStat);
//			EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)e_Level, (uchar*)"##[04.1] TYPE_RdsWarnStatNormal");
//			return;
//		}
// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される Remove End.
		ushort ul_ThresholdPaperFeedRetry1Cnt		= s_NowPaperFeedRetryCnt.ul_PaperFeedRetry1Cnt		- s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry1Cnt;
		ushort ul_ThresholdPaperFeedRetry2Cnt		= s_NowPaperFeedRetryCnt.ul_PaperFeedRetry2Cnt		- s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry2Cnt;
		ushort ul_ThresholdPaperFeedRetry3_LCTCnt	= s_NowPaperFeedRetryCnt.ul_PaperFeedRetry3_LCTCnt	- s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry3_LCTCnt;
		ushort ul_ThresholdPaperFeedRetry4Cnt		= s_NowPaperFeedRetryCnt.ul_PaperFeedRetry4Cnt		- s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry4Cnt;
		ushort ul_ThresholdPaperFeedRetryManualCnt	= s_NowPaperFeedRetryCnt.ul_PaperFeedRetryManualCnt - s_PrevPaperFeedRetryCnt.ul_PaperFeedRetryManualCnt;
		ushort ul_ThresholdPaperFeedRetryExtLCTCnt	= s_NowPaperFeedRetryCnt.ul_PaperFeedRetryExtLCTCnt - s_PrevPaperFeedRetryCnt.ul_PaperFeedRetryExtLCTCnt;	
// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される Start.
	RDSD_DBG_MESSAGE_POLLING_STR( "s_NowPaperFeedRetryCnt[1] =", (uint32_t) s_NowPaperFeedRetryCnt.ul_PaperFeedRetry1Cnt, (uint32_t) s_NowPaperFeedRetryCnt.ul_PaperFeedRetry2Cnt, (uint32_t) s_NowPaperFeedRetryCnt.ul_PaperFeedRetry3_LCTCnt );
	RDSD_DBG_MESSAGE_POLLING_STR( "s_NowPaperFeedRetryCnt[2] =", (uint32_t) s_NowPaperFeedRetryCnt.ul_PaperFeedRetry4Cnt, (uint32_t) s_NowPaperFeedRetryCnt.ul_PaperFeedRetryManualCnt, (uint32_t) s_NowPaperFeedRetryCnt.ul_PaperFeedRetryExtLCTCnt );
	RDSD_DBG_MESSAGE_POLLING_STR( "s_PrevPaperFeedRetryCnt[1] =", (uint32_t) s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry1Cnt, (uint32_t) s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry2Cnt, (uint32_t) s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry3_LCTCnt );
	RDSD_DBG_MESSAGE_POLLING_STR( "s_PrevPaperFeedRetryCnt[2] =", (uint32_t) s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry4Cnt, (uint32_t) s_PrevPaperFeedRetryCnt.ul_PaperFeedRetryManualCnt, (uint32_t) s_PrevPaperFeedRetryCnt.ul_PaperFeedRetryExtLCTCnt );
// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される End.
		if ((ul_ThresholdPaperFeedRetry1Cnt			< (ushort)uc_DipSw30) &&
			(ul_ThresholdPaperFeedRetry2Cnt			< (ushort)uc_DipSw30) &&
			(ul_ThresholdPaperFeedRetry3_LCTCnt		< (ushort)uc_DipSw30) &&
			(ul_ThresholdPaperFeedRetry4Cnt			< (ushort)uc_DipSw30) &&
			(ul_ThresholdPaperFeedRetryManualCnt	< (ushort)uc_DipSw30) &&
			(ul_ThresholdPaperFeedRetryExtLCTCnt	< (ushort)uc_DipSw30))
		{
			uc_warnData[RDSE_WarnTrgRetryFrequency] = TYPE_RdsWarnStatNormal;
// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される Start.
			// Update Alarm status info.
			s_AlarmStatusInfo.c_PreviousDateChecking.ss_Year 	= c_NowTime.ss_Year;
			s_AlarmStatusInfo.c_PreviousDateChecking.sc_Month 	= c_NowTime.sc_Month;
			s_AlarmStatusInfo.c_PreviousDateChecking.sc_Day 	= c_NowTime.sc_Day;
			s_AlarmStatusInfo.c_PreviousDateChecking.sc_Hour 	= c_NowTime.sc_Hour;
			s_AlarmStatusInfo.c_PreviousDateChecking.sc_Minute 	= c_NowTime.sc_Minute;
			// Assign now Paper Feeding counter to previous Paper Feeding counter.
			s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry1Cnt		= s_NowPaperFeedRetryCnt.ul_PaperFeedRetry1Cnt;
			s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry2Cnt		= s_NowPaperFeedRetryCnt.ul_PaperFeedRetry2Cnt;
			s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry3_LCTCnt	= s_NowPaperFeedRetryCnt.ul_PaperFeedRetry3_LCTCnt;
			s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry4Cnt		= s_NowPaperFeedRetryCnt.ul_PaperFeedRetry4Cnt;
			s_PrevPaperFeedRetryCnt.ul_PaperFeedRetryManualCnt	= s_NowPaperFeedRetryCnt.ul_PaperFeedRetryManualCnt;
			s_PrevPaperFeedRetryCnt.ul_PaperFeedRetryExtLCTCnt	= s_NowPaperFeedRetryCnt.ul_PaperFeedRetryExtLCTCnt;	
			// Save counter and status.
			pc_RdsSetting->lock();
			pc_RdsSetting->setPaperFeedRetryCnt(&s_PrevPaperFeedRetryCnt);
			pc_RdsSetting->setAlarmStatusInfo(&s_AlarmStatusInfo);
			pc_RdsSetting->unlock();
			EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)e_Level, (uchar*)"##[04.1] TYPE_RdsWarnStatNormal");
// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される End.
			__saveCommonAPI(E_WarnStat);
			return;
		}
		e_Level = TYPE_RdsWarnStatErr;

		// Date and time of frequent warning transmission.
		EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)e_Level, (uchar*)"##[04] Update time");
		// s_NowAlarmStatusInfo.c_PreviousDateWarning.ss_Year 		= s_NowAlarmStatusInfo.c_PreviousDateChecking.ss_Year;
		// s_NowAlarmStatusInfo.c_PreviousDateWarning.sc_Month 	= s_NowAlarmStatusInfo.c_PreviousDateChecking.sc_Month;
		// s_NowAlarmStatusInfo.c_PreviousDateWarning.sc_Day 		= s_NowAlarmStatusInfo.c_PreviousDateChecking.sc_Day;
		// s_NowAlarmStatusInfo.c_PreviousDateWarning.sc_Hour 		= s_NowAlarmStatusInfo.c_PreviousDateChecking.sc_Hour;
		// s_NowAlarmStatusInfo.c_PreviousDateWarning.sc_Minute 	= s_NowAlarmStatusInfo.c_PreviousDateChecking.sc_Minute;
		// s_NowAlarmStatusInfo.b_WarningStatus = True;
// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される Modify Start.
		// Update Alarm status info.
		s_AlarmStatusInfo.c_PreviousDateChecking.ss_Year 	= c_NowTime.ss_Year;
		s_AlarmStatusInfo.c_PreviousDateChecking.sc_Month 	= c_NowTime.sc_Month;
		s_AlarmStatusInfo.c_PreviousDateChecking.sc_Day 	= c_NowTime.sc_Day;
		s_AlarmStatusInfo.c_PreviousDateChecking.sc_Hour 	= c_NowTime.sc_Hour;
		s_AlarmStatusInfo.c_PreviousDateChecking.sc_Minute 	= c_NowTime.sc_Minute;

// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される Modify End.
		// Assign now Paper Feeding counter to previous Paper Feeding counter
		s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry1Cnt		= s_NowPaperFeedRetryCnt.ul_PaperFeedRetry1Cnt;
		s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry2Cnt		= s_NowPaperFeedRetryCnt.ul_PaperFeedRetry2Cnt;
		s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry3_LCTCnt	= s_NowPaperFeedRetryCnt.ul_PaperFeedRetry3_LCTCnt;
		s_PrevPaperFeedRetryCnt.ul_PaperFeedRetry4Cnt		= s_NowPaperFeedRetryCnt.ul_PaperFeedRetry4Cnt;
		s_PrevPaperFeedRetryCnt.ul_PaperFeedRetryManualCnt	= s_NowPaperFeedRetryCnt.ul_PaperFeedRetryManualCnt;
		s_PrevPaperFeedRetryCnt.ul_PaperFeedRetryExtLCTCnt	= s_NowPaperFeedRetryCnt.ul_PaperFeedRetryExtLCTCnt;	

// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される Start.
		// Save counter and status.
		pc_RdsSetting->lock();
		pc_RdsSetting->setPaperFeedRetryCnt(&s_PrevPaperFeedRetryCnt);
		pc_RdsSetting->setAlarmStatusInfo(&s_AlarmStatusInfo);
		pc_RdsSetting->unlock();
// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される End.
		EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)e_Level, (uchar*)"##[04] TYPE_RdsWarnStatErr");
		chkWarn(e_kind, e_Level);
// 2019/01/18 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-1909】 Modification End
	}
	else
	{
		//EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)us_retONRetryFrequency, (uchar*)"##[05] uc_warnData[RDSE_WarnTrgRetryFrequency] = TYPE_RdsWarnStatNormal");
		TYPE_RdsWarnStat e_RdsWarnStat = getRdsRetryWarningCounterState();

		if( TYPE_RdsWarnStatNormal != e_RdsWarnStat )
		{
			uc_warnData[RDSE_WarnTrgRetryFrequency] = TYPE_RdsWarnStatNormal;
			__saveCommonAPI(E_WarnStat);
		}
	}
// 2019/03/10 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-3232】 Modification End
// 2019/01/07 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-1916】 Modification End
}
// 2018/12/03 FPT-DucDH4 【IT6_2.0】【EagleH】【機器構成変更_No.1】 リトライ多発警告 End.
//Minerva UV-00929 kita 2010/10/20 End.

// 2022/11/21 FPT-TinhNT16 【IT6_2.2MR4】【OP_BTS-40197】 RTCの時刻ずれ補正対応 Start.
/*--------------------------------------------------------------------------------------------------*/
// CSRCセンター時計の時刻.
/*--------------------------------------------------------------------------------------------------*/
void RDSC_WarnData::getTimeOfCSRCServer(RDSE_WarnTrg e_kind, Bool b_Notify)
{
	EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)e_kind, (uchar*)" Start." );
	TYPE_RdsWarnStat e_Level;
	if( True == b_Notify )
	{
		e_Level = TYPE_RdsWarnStatNear;
	}
	else
	{
		e_Level = TYPE_RdsWarnStatNormal;
	}

	chkWarn( e_kind, e_Level );
}

/*--------------------------------------------------------------------------------------------------*/
// 警告発信 (RTCの時刻ずれ補正).
/*--------------------------------------------------------------------------------------------------*/
void RDSC_WarnData::correctRTCTime(RDSE_WarnTrg e_kind, Bool b_Notify)
{
	EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)e_kind, (uchar*)" Start." );
	TYPE_RdsWarnStat e_Level;
	if( True == b_Notify )
	{
		e_Level = TYPE_RdsWarnStatErr;
	}
	else
	{
		e_Level = TYPE_RdsWarnStatNormal;
	}

	chkWarn( e_kind, e_Level );
}
// 2022/11/21 FPT-TinhNT16 【IT6_2.2MR4】【OP_BTS-40197】 RTCの時刻ずれ補正対応 End.

// 2017/01/05 nakano [MAv5][RQ-710] CSRA接続 RTRUコマンドの通知をDrop Start.
// 2017/06/07 kajitani [MAv5][RQ-XXX] CSRA接続 Start.
/*--------------------------------------------------------------------------------------------------*/
// Pdmシステム状態
/*--------------------------------------------------------------------------------------------------*/
//void RDSC_WarnData::sub_chkRdsPdmStat( RDSE_WarnTrg e_kind, uchar uc_Cnt )
//{
//	TYPE_RdsWarnStat	e_Level = TYPE_RdsWarnStatNormal;
// 2017/07/12 kajitani [MAv5][KRQ-335][AccuRev.1025859] CSRA接続 Start.
//	if( 0 <= uc_Cnt )	// 送信エラー発生時.
//	if( 0 < uc_Cnt )	// 送信エラー発生時.
// 2017/07/12 kajitani [MAv5][KRQ-335][AccuRev.1025859] CSRA接続 End.
//	{
//		e_Level = TYPE_RdsWarnStatErr;
//	}

//	chkWarn( e_kind, e_Level );
//}
// 2017/06/07 kajitani [MAv5][RQ-XXX] CSRA接続 End.
// 2017/01/05 nakano [MAv5][RQ-710] CSRA接続 RTRUコマンドの通知をDrop End.
/*==================================================================================================*/
// 04/03/16 浦崎  CommonAPIの トナー系、廃トナーボックスFULL の警告変化を検出するか否かを
//                参照するようにした。 
// 04/07/07 浦崎  (Rhine1固有)ＣＣＤクランプゲイン調整異常 に対応。 
// 04/07/09 浦崎  PMカウンタ警告に対応。 
// 05/05/30 浦崎  ・トナー（廃トナー含む）系の警告情報は、CSRC用のメソッドをコールして状態を検出 
//                  するようにした。 
//                ・CSRCが警告状態を取得してもいいタイミングか否かを判定した上で OK である場合のみ
//                  APIのデータを参照するようにした。
// 14/06/03 ktakahashi	Zeus5BK 機器構成変更No.4(WP1689).
/*==================================================================================================*/
Bool RDSC_WarnData::chkChanged()
{

	APIC_PrinterStatus*	pc_PrinterStatus;									// プリンタのステータス 
	APIC_SystemInfo* pc_SystemInfo	= APIC_SystemInfo::newInstance();		// 2015/12/04 M.Adachi [ZeusS_ZX0]CQ522420_ドラム寿命警告がCSRCに発信されない.
	pc_PrinterStatus = APIC_PrinterStatus::newInstance();
	if( False == pc_PrinterStatus->getCsrcWarnInfoEnable() )
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", 0, (uchar*)"本体が警告状態のチェックを許可していない。" );
		return( False );
	}
// 2015/12/04 M.Adachi [ZeusS_ZX0]CQ522420_ドラム寿命警告がCSRCに発信されない Start.
// Bool LIBC_FixedValueDefine::isXXXX()にて機種判定していた個所をマシンタイプを参照するように変更.
	pc_SystemInfo->lock();
	TYPE_MachineType e_Type = pc_SystemInfo->getMachineType();
	pc_SystemInfo->unlock();
// 2015/12/04 M.Adachi [ZeusS_ZX0]CQ522420_ドラム寿命警告がCSRCに発信されない End.

	// CommonAPI の対象の警告内容をチェックする。
	// 通信中はチェックしないようにする。→ 排他処理のため。イベント処理部の動作と通信シーケンスは非同期であるため、
	// リングバッファのライトポインタとリードポインタが交差するような時があると異常動作をする可能性がある。
	APIC_PrinterWarning*	pc_PrintWarn = APIC_PrinterWarning::newInstance();		// プリンタ警告.
	pc_FixedValueDefine = pc_FixedValueDefine->newInstance();						// Libのインスタンス作成.

	if( True == pc_RdsSetting->getTonerMode() )		// トナー系の警告変化を検出するか否か  
	{
		// BSKK要望のデバッグコマンド sone 2012/09/12 start.
		if( b_dbgFalg == False )	// デバッグフラグが無効の場合は、本当のデータを参照させる.
		{
			/* Ｙトナーエンプティ */
			if( True != pc_PrintWarn->isYellowTonerUnset() )
			{
				sub_chkTonerStat( RDSE_WarnTrgTonerEmptyY, pc_PrintWarn->getYellowTonerStatusForCSRC() );		// 05/05/30 浦崎 CSRC 用の情報を見るように対応した。 
			}
			/* Ｍトナーエンプティ */
			if( True != pc_PrintWarn->isMagentaTonerUnset() )
			{
				sub_chkTonerStat( RDSE_WarnTrgTonerEmptyM, pc_PrintWarn->getMagentaTonerStatusForCSRC() );		// 05/05/30 浦崎 CSRC 用の情報を見るように対応した。 
			}
			/* Ｃトナーエンプティ */
			if( True != pc_PrintWarn->isCyanTonerUnset() )
			{
				sub_chkTonerStat( RDSE_WarnTrgTonerEmptyC, pc_PrintWarn->getCyanTonerStatusForCSRC() );			// 05/05/30 浦崎 CSRC 用の情報を見るように対応した。 
			}
			/* Ｋトナーエンプティ */
			if( True != pc_PrintWarn->isBlackTonerUnset() )
			{
				sub_chkTonerStat( RDSE_WarnTrgTonerEmptyK, pc_PrintWarn->getBlackTonerStatusForCSRC() );		// 05/05/30 浦崎 CSRC 用の情報を見るように対応した。 
			}
		}
		// BSKK要望のデバッグコマンド sone 2012/09/12 end.
	}
	if( True == pc_RdsSetting->getUsedTonerBottleMode() )
	{
		// BSKK要望のデバッグコマンド sone 2012/09/12 start.
		if( b_dbgFalg == False )	// デバッグフラグが無効の場合は、本当のデータを参照させる.
		{
			/* 廃トナーボトルフル */
			// 05/06/02 浦崎 sub_chkTonerStat() → sub_chkTonerBottleStat() 
			sub_chkTonerBottleStat( RDSE_WarnTrgBottleFull, pc_PrintWarn->getTonerBottleStatusForCSRC() );			// 05/05/30 浦崎 CSRC 用の情報を見るように対応した。 
		}
		// BSKK要望のデバッグコマンド sone 2012/09/12 end.
	}

	// IC 系 
	if( True == pc_RdsSetting->getIULifeMode() )				// IC系の警告変化を検出するか否か 04/07/14 浦崎 
	{
		// Zeus対応 sone 2010/11/04.
		// Donau、Amur、Taigaについての処理は削除.
// 2015/12/04 M.Adachi [ZeusS_ZX0]CQ522420_ドラム寿命警告がCSRCに発信されない Start.
// Bool LIBC_FixedValueDefine::isXXXX()にて機種判定していた個所をマシンタイプを参照するように変更.
		if( ( TYPE_MachineTypeZeusS1 == e_Type )		// ZeusS1.
	 	 || ( TYPE_MachineTypeZeusS2 == e_Type )		// ZeusS2.
	 	 || ( TYPE_MachineTypeZeusS2_5 == e_Type ) 		// ZeusS2.5.
		 || ( TYPE_MachineTypeZeusSZ_US_JP == e_Type )	// ZeusS Z(US).
		 || ( TYPE_MachineTypeZeusSZ_EU == e_Type )		// ZeusS Z(EU).
		 || ( TYPE_MachineTypeZeusSX_US_JP == e_Type )	// ZeusS X(US/JP).
		 || ( TYPE_MachineTypeZeusSX_EU == e_Type )		// ZeusS X(EU).
		 || ( TYPE_MachineTypeZeusS0 == e_Type )		// ZeusS 0.
		 || ( TYPE_MachineTypeEagle1 == e_Type )		// Eagle1.	// 2017/10/24 HCLT-Vinay IT6_1.0 制御構成変更_No.6 Start.
		 || ( TYPE_MachineTypeEagle2 == e_Type )		// Eagle2.
		 || ( TYPE_MachineTypeEagle2_5 == e_Type )		// Eagle2.5.
		 || ( TYPE_MachineTypeEagleZ == e_Type )		// EagleZ.
		 || ( TYPE_MachineTypeEagleX == e_Type )		// EagleX.
		 || ( TYPE_MachineTypeEagle0 == e_Type )		// Eagle0.	// 2017/10/24 HCLT-Vinay IT6_1.0 制御構成変更_No.6 End.
		 || ( TYPE_MachineTypeEagleZplus == e_Type ) )	// EagleZ+.	// 2020/01/28 日立 I.Morimoto [EagleZ+]OP_BTS-19647 ドラムユニット(CMY)のライフストップ警告が通知されない.
// 2015/12/04 M.Adachi [ZeusS_ZX0]CQ522420_ドラム寿命警告がCSRCに発信されない End.
		{
// 2014/11/13 ktakahashi ZeusS CQ506949 ターミナルから"setLife 21,1"入力後CSRCに[パーツライフ(DevC_C) 1st Call]警告発信されない Start.
			/* DCライフY */
			sub_chkUnitLifeStat( RDSE_WarnTrgICLifeYDC, pc_PrintWarn->getYellowDCLife() );
			/* DCライフM */
			sub_chkUnitLifeStat( RDSE_WarnTrgICLifeMDC, pc_PrintWarn->getMagentaDCLife() );
			/* DCライフC */
			sub_chkUnitLifeStat( RDSE_WarnTrgICLifeCDC, pc_PrintWarn->getCyanDCLife() );

			/* DevCライフY */
			sub_chkUnitLifeStat( RDSE_WarnTrgICLifeYDevC, pc_PrintWarn->getYellowDevCLife() );
			/* DevCライフM */
			sub_chkUnitLifeStat( RDSE_WarnTrgICLifeMDevC, pc_PrintWarn->getMagentaDevCLife() );
			/* DevCライフC */
			sub_chkUnitLifeStat( RDSE_WarnTrgICLifeCDevC, pc_PrintWarn->getCyanDevCLife() );
// 2014/11/13 ktakahashi ZeusS CQ506949 ターミナルから"setLife 21,1"入力後CSRCに[パーツライフ(DevC_C) 1st Call]警告発信されない End.
		}
		else
		{
			/* HeliosL/MinervaMLKはIUライフを参照する */
			/* ICライフY */
			sub_chkUnitLifeStat( RDSE_WarnTrgICLifeY, pc_PrintWarn->getYellowIULife() );
			/* ICライフM */
			sub_chkUnitLifeStat( RDSE_WarnTrgICLifeM, pc_PrintWarn->getMagentaIULife() );
			/* ICライフC */
			sub_chkUnitLifeStat( RDSE_WarnTrgICLifeC, pc_PrintWarn->getCyanIULife() );
		}

		/* Donau以降、黒IUは黒DC(ドラムカートリッジ)と黒Devc(感光体ユニット)に分離された。
		   そのため、エンプティ状態を示す値を取得する関数もDC、Devcに分ける必要がある */

		/* DCライフK */
#ifdef DEF_A4_Cus //【VenusMLK】 CQ531377 HCLT_Pawan 2016/06/28 Start.
		sub_chkUnitLifeStat( RDSE_WarnTrgICLifeK, pc_PrintWarn->getBlackIULife() );
#else
//2021/07/16 FPT-TinhNT16 【IT6_MinervaSSBK】 【OP-BTS-34715】 Start.
// 2021/11/06 FPT-TrungHV7 【IT6_MinervaSSBK】 【OP-BTS-36210】 Start.
//	if( ( TYPE_MachineTypeMinervaSSBKX == e_Type )		//MinervaSSBK X/0.
//	 || ( TYPE_MachineTypeMinervaSSBK0_US == e_Type )
//	 || ( TYPE_MachineTypeMinervaSSBK0_EU == e_Type )){
//		 //NOP.
//	}
//	else {
		sub_chkUnitLifeStat( RDSE_WarnTrgICLifeK, pc_PrintWarn->getBlackDCLife() );
//	}
// 2021/11/06 FPT-TrungHV7 【IT6_MinervaSSBK】 【OP-BTS-36210】 End.
//2021/07/16 FPT-TinhNT16 【IT6_MinervaSSBK】 【OP-BTS-34715】 End.
#endif //【VenusMLK】 CQ531377 HCLT_Pawan 2016/06/28 End.
		/* DevcライフK */
		sub_chkUnitLifeStat( RDSE_WarnTrgICLifeKDevC, pc_PrintWarn->getBlackDevCLife() );//BlackDevcの状態を戻す。
	}

	/* 定着ユニットライフ */
	sub_chkUnitLifeStat( RDSE_WarnTrgFuserUnitLife, pc_PrintWarn->getFuserUnitLife() );

	/* トナーフィルタライフ */
	sub_chkUnitLifeStat( RDSE_WarnTrgTonerFilter, pc_PrintWarn->getSmokeFilterUnitLife() );

	/* 転写ベルトユニットライフ */
// 2014/06/03 ktakahashi Zeus5BK 機器構成変更No.4(WP1689) Start.
// 18/06/01 HCLT-Vinay 【IT6_1.0】【 DenebMLK 】 制御構成変更_No.6_マシンタイプ対応 Added Start.
	// Zeus5BK1/2/3&DenebMLK1/2/3/4の場合はセットしない.
// 2015/12/04 M.Adachi [ZeusS_ZX0]CQ522420_ドラム寿命警告がCSRCに発信されない Start.
// Bool LIBC_FixedValueDefine::isXXXX()にて機種判定していた個所をマシンタイプを参照するように変更.
	if( ( TYPE_MachineTypeZeus5BK1 != e_Type )
	 && ( TYPE_MachineTypeZeus5BK2 != e_Type )
	 && ( TYPE_MachineTypeZeus5BK3 != e_Type )
	 && ( TYPE_MachineTypeDenebMLK1 != e_Type )		//DenebMLK.
	 && ( TYPE_MachineTypeDenebMLK2 != e_Type )
	 && ( TYPE_MachineTypeDenebMLK3 != e_Type )
	 && ( TYPE_MachineTypeDenebMLK4 != e_Type ) )
// 18/06/01 HCLT-Vinay 【IT6_1.0】【 DenebMLK 】 制御構成変更_No.6_マシンタイプ対応 Added End.
// 2015/12/04 M.Adachi [ZeusS_ZX0]CQ522420_ドラム寿命警告がCSRCに発信されない End.
	{
		sub_chkUnitLifeStat( RDSE_WarnTrgTransBeltLife, pc_PrintWarn->getTransferBeltUnitLife() );
	}
// 2014/06/03 ktakahashi Zeus5BK 機器構成変更No.4(WP1689) End.

	/* オゾンフィルタ、紙粉除去クリーナーの警告は、Donau/Amurまでは同一の警告に丸め込まれていたが、
	   Taigaではそれぞれ分離され、かつニアライフ/ライフのステータスではなく、警告発生 or 通常状態で
	   警告を判定する仕様となっているため、サブチェック関数も呼び分ける必要がある。 */
	if( True == pc_FixedValueDefine->isTaiga() )
	{
		/* オゾンフィルタ累積回転時間超過警告発生状態 */
		sub_chkWarnPartsStat(RDSE_WarnTrgOzonFilterLife, TYPE_PRTWarningCode_OzonFilterExcessTrouble);
		
		/* 紙粉除去クリーナ累積回転時間超過警告発生状態 */
		sub_chkWarnPartsStat(RDSE_WarnTrgPaperDustCleaner, TYPE_PRTWarningCode_PaperDustRemovalCleanerExcessTrouble);
		
		/* 転写ローラユニット累積回転時間超過警告発生状態 */
		sub_chkWarnPartsStat(RDSE_WarnTrgTransRollerLife, TYPE_PRTWarningCode_TtransferRollerExcessTrouble);
	}
	else
	{
// 2015/12/04 M.Adachi [ZeusS_ZX0]CQ522420_ドラム寿命警告がCSRCに発信されない Start.
//   [ZeusS] 制御構成変更No.3(WP355)にてオゾンフィルタライフの警告発信処理がDEF_Zeus5BkCusで括られていたため、.
//   MinervaSBKでは警告発信されないことが判明した。そのため、コンパイルSWで括るのをやめ、マシンタイプを参照し、.
//   MinervaSBKでも発信できるよう修正.
// 18/06/01 HCLT-Vinay 【IT6_1.0】【 DenebMLK 】 制御構成変更_No.6_マシンタイプ対応 Added Start.
		if( ( TYPE_MachineTypeZeus5BK1 == e_Type )		// Poseidon.
	 	 || ( TYPE_MachineTypeZeus5BK2 == e_Type )
	 	 || ( TYPE_MachineTypeZeus5BK3 == e_Type )
		 || ( TYPE_MachineTypeMinervaSBKX == e_Type )	// MinervaSBK.
		 || ( TYPE_MachineTypeMinervaSBK0_US == e_Type )
		 || ( TYPE_MachineTypeMinervaSBK0_EU == e_Type )
		 || ( TYPE_MachineTypeMinervaSBK0_JP == e_Type )
//2021/07/16 FPT-TinhNT16 【IT6_MinervaSSBK】 【OP-BTS-34715】 Start.
		 || ( TYPE_MachineTypeMinervaSSBKX == e_Type )		//MinervaSSBK X/0.
		 || ( TYPE_MachineTypeMinervaSSBK0_US == e_Type )
		 || ( TYPE_MachineTypeMinervaSSBK0_EU == e_Type )
//2021/07/16 FPT-TinhNT16 【IT6_MinervaSSBK】 【OP-BTS-34715】 End.
// 18/06/05 HCLT-Vinay 【IT6_1.0】【 DenebMLK 】 機器構成変更_No.15_オゾンフィルタ非搭載対応 Comment Start.
/*		 || ( TYPE_MachineTypeDenebMLK1 == e_Type )		// DenebMLK.
		 || ( TYPE_MachineTypeDenebMLK2 == e_Type )
		 || ( TYPE_MachineTypeDenebMLK3 == e_Type )
		 || ( TYPE_MachineTypeDenebMLK4 == e_Type )*/ )
// 18/06/05 HCLT-Vinay 【IT6_1.0】【 DenebMLK 】 機器構成変更_No.15_オゾンフィルタ非搭載対応 Comment End.
// 18/06/01 HCLT-Vinay 【IT6_1.0】【 DenebMLK 】 制御構成変更_No.6_マシンタイプ対応 Added End.
		{
			/* オゾンフィルタライフ */
			sub_chkUnitLifeStat( RDSE_WarnTrgOzonFilterLife, pc_PrintWarn->getOzonFilterUnitLife() );
		}
// 2015/12/04 M.Adachi [ZeusS_ZX0]CQ522420_ドラム寿命警告がCSRCに発信されない End.
		/* 転写ローラユニットライフ */
		sub_chkUnitLifeStat( RDSE_WarnTrgTransRollerLife, pc_PrintWarn->get2ndTransRollerUnitLife() );
	}
	
//2021/06/28 FPT-TinhNT16 【IT6_MinervaSSBK】【 OP_BTS-34372 】 Start.
	/* 定着センサ温度検出不良警告 */
	if( ( e_Type == TYPE_MachineTypeMinervaSSBKX )
	 || ( e_Type == TYPE_MachineTypeMinervaSSBK0_US )
	 || ( e_Type == TYPE_MachineTypeMinervaSSBK0_EU ) )
	{
		//NOP.
	}
	else {
		sub_chkWarnPartsStat( RDSE_WarnTrgFuserTempTrouble, TYPE_PRTWarningCode_FuserTempTrouble);
	}
//2021/06/28 FPT-TinhNT16 【IT6_MinervaSSBK】【 OP_BTS-34372 】 End.
	/* 用紙系、原稿系JAM多発検知 */
	sub_chkRdsJAMFrequecy( RDSE_WarnTrgMCBJFrequecy); //用紙系.
	sub_chkRdsJAMFrequecy( RDSE_WarnTrgMOBJFrequecy); //原稿系.

	/* SOD ： SOD は、本体からイベントとして通知してもらう。 */
	//sub_chkSODStat( RDSE_WarnTrgSpecificCopy, pc_RdsSetting->getSpecialStat() );

	/* 転写クリーナユニットライフ */
	//08/08/07 芳賀 Donau以降、他のパーツにカウンタが統合されたため警告発生の必要なし.
	//sub_chkUnitLifeStat( RDSE_WarnTrgTransCleanLife, pc_PrintWarn->getTransCleanUnitLife() );
	
	if( True == pc_RdsSetting->getCCDGeinMode() )				// 警告変化を検出するか否か 04/07/14 浦崎 
	{
		// BSKK要望のデバッグコマンド sone 2012/09/12 start.
		if( b_dbgFalg == False )	// デバッグフラグが無効の場合は、本当のデータを参照させる.
		{
			APIC_ScannerWarning*	pc_ScanWarn = APIC_ScannerWarning::newInstance();		// スキャナ警告 
			/* ＣＣＤクランプゲイン調整異常 */
			sub_chkCCDGeinStat( RDSE_WarnTrgCCDGeinStat, pc_ScanWarn->isWarning( ATTENTION_IRWarning ) );
		}
	}
	
// Minerva UV-00929 kira 2010/10/20 Start.
	APIC_Enabler*	pc_Enabler = APIC_Enabler::newInstance();
//2021/06/02 【IT6_2.2】【OP_BTS-33588】 Start.
	if( pc_Enabler->isEngStarttingComplete() )
	{
		// ライセンス管理異常状態確認.
		sub_chkRdsLicenseManagementErrorStat( RDSE_WarnTrgLicenseManagementError, pc_Enabler->isLicenseManagementAbnormal() );
	}
//2021/06/02 【IT6_2.2】【OP_BTS-33588】 End.
// Minerva UV-00929 kira 2010/10/20 End.

// 2019/06/28 EagleZ+ 制御構成変更_No.36 MIF機対応 日立 江上 MOD Start.
	if( ( TYPE_MachineTypeDenebMLK1 != e_Type )
	 && ( TYPE_MachineTypeDenebMLK2 != e_Type )
	 && ( TYPE_MachineTypeDenebMLK3 != e_Type )
	 && ( TYPE_MachineTypeDenebMLK4 != e_Type ) )
	 {
// 2018/12/03 FPT-DucDH4 【IT6_2.0】【EagleH】【機器構成変更_No.1】 リトライ多発警告 Start
		sub_chkRdsRetryFrequency(RDSE_WarnTrgRetryFrequency);
// 2018/12/03 FPT-DucDH4 【IT6_2.0】【EagleH】【機器構成変更_No.1】 リトライ多発警告 End
	}
// 2019/06/28 EagleZ+ 制御構成変更_No.36 MIF機対応 日立 江上 MOD End.

// 2017/01/05 nakano [MAv5][RQ-710] CSRA接続 RTRUコマンドの通知をDrop Start.
// 2017/06/07 kajitani [MAv5][RQ-XXX] CSRA接続 Start.
	// Pdmシステム警告状態確認.
//	APIC_CSRASetting	*pc_CSRASetting = APIC_CSRASetting::newInstance();
//	if ( NULL != pc_CSRASetting ) {
//		TYPS_CSRASendInfo s_CSRASendInfo = {0};
//		pc_CSRASetting->getCSRASendInfo( &s_CSRASendInfo );									// CSRAデータ送付設定情報 取得.
//		sub_chkRdsPdmStat( RDSE_WarnTrgPdmPreChk,	s_CSRASendInfo.m_uc_PrecheckRetryCnt );	// プレチェック通信リトライ回数.
//		sub_chkRdsPdmStat( RDSE_WarnTrgPdmDataSnd,	s_CSRASendInfo.m_uc_SendingRetryCnt	 );	// データ転送通信  リトライ回数.
//	}
//	else {
//		EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)"APIC_CSRASetting is null");
//	}
// 2017/06/07 kajitani [MAv5][RQ-XXX] CSRA接続 End.
// 2017/01/05 nakano [MAv5][RQ-710] CSRA接続 RTRUコマンドの通知をDrop End.

	/* PMカウンタ警告 */
	if( True == pc_RdsSetting->getRegularServicePMMode() )		// 警告変化を検出するか否か 04/07/14 浦崎 
	{
		APIC_PmCnt*	pc_PmCnt			= APIC_PmCnt::newInstance();
		uint32_t		ul_tmpPmCnt			= pc_PmCnt->get(TYPE_PmCntPM);
		uint32_t		ul_tmpPmCntLimit	= pc_PmCnt->get(TYPE_PmCntLimit);
		RDSE_PMWarn	e_PMWarnStat		= RDSE_PMWarn_ZeroReset;
		if(    0 != ul_tmpPmCntLimit
			&& ul_tmpPmCnt >= ul_tmpPmCntLimit    )
		{
			e_PMWarnStat = RDSE_PMWarn_Error;					// ＰＭカウンタ上限値に達している時 
		}
		else
		{
			if( 0 == ul_tmpPmCnt ){
				e_PMWarnStat = RDSE_PMWarn_ZeroReset;			// ゼロリセット解除状態（本来はシステムから別ルートで通知されるが、ここでも念のため検出する） 
			} else {
				e_PMWarnStat = RDSE_PMWarn_Normal;				// 通常解除状態 
			}
		}
		sub_chkPMStat( RDSE_WarnTrgPM, e_PMWarnStat );
	}

	/* 固定交換パーツ警告 */
	if( True == pc_RdsSetting->getPartsReplaceMode() )			// 警告変化を検出するか否か 04/07/14 浦崎 
	{
		sub_chkPartsReplacingStat( RDSE_WarnTrgPartsReplacing );
	}

	//(不要)__saveCommonAPI();
	return( False );
}

/*==================================================================================================*/
//	本体警告発生（ビットセット）
//
//		OUT :	True	： 発信要求マップに発信要求がセットされる
//				False	： 発信要求マップに発信要求はセットされない
/*--------------------------------------------------------------------------------------------------*/
// 04/01/05 浦崎  警告解除状態の取り込み処理に対応。
// 04/03/01 浦崎  解除手前状態から再度警告状態に復帰してしまった場合、解除手前状態をキャンセルし、
//				  警告状態とするようにした。
// 04/05/28 浦崎  警告発信の応答発信を正しく受け取れなかった問題を修正。
//                ※警告発生検出後、一度でも電源をOFF→ONすれば問題なく処理できる。
//                ※原因は、センターから通知されるイベントIDの年部分は「yy」と言う形式で
//                  通知されるにも関わらず、複写機側は「yyyy」として比較していたため、不一致が起こっていた。
//                ※電源OFF→ON後正しく処理できる理由は、不揮発からのデータロード処理時に
//                  「yy」としてデータをロードするため。 
// 04/06/01 浦崎  トナー系警告解除の復旧時間を、不揮発から読み出して設定するようにした。 
/*==================================================================================================*/
/**/
RDSE_WarnTrg s_ChkRecoverWarnKind[] =
{
	RDSE_WarnTrgTonerEmptyK,		//概要　　：警告発生時刻(トナーエンプティ[K色])  
	RDSE_WarnTrgTonerEmptyC,		//概要　　：警告発生時刻(トナーエンプティ[C色]) 
	RDSE_WarnTrgTonerEmptyM,		//概要　　：警告発生時刻(トナーエンプティ[M色]) 
	RDSE_WarnTrgTonerEmptyY,		//概要　　：警告発生時刻(トナーエンプティ[Y色]) 
	RDSE_WarnTrgBottleFull			//概要　　：廃トナーボトルFULL
};
/**/

extern TYPE_RdsWarnStat e_CurRdsWarnStat[RDSE_WarnTrgBottleFull + 1];

Bool RDSC_WarnData::chkWarn( RDSE_WarnTrg e_kind, TYPE_RdsWarnStat e_Level, uchar* pDetailData )
{
//	EPN_DBG_MESSAGE((uchar*)"[CSRC]", (uint32_t)e_kind, (uchar*)"Bool RDSC_WarnData::chkWarn( RDSE_WarnTrg e_kind, TYPE_RdsWarnStat e_Level, uchar* pDetailData )");			//2019/01/25 FPT-DucDH4 Remove log debug
// 2010/07/17 higuchi Minerva warning 対応 Start.
	uchar uc_kind = e_kind;		// warning 対策のため、char を unsigned char に変換して使用する.
// 2010/07/17 higuchi Minerva warning 対応 End.

	/*--------------------------------------------------------------------------------------------------*/
	// 警告状態取り込み前のチェック 
// 2010/07/17 higuchi Minerva warning 対応 Start.
//	if(    ( e_kind == RDSE_WarnTrgTonerEmptyK )		//概要　　：警告発生時刻(トナーエンプティ[K色])  
//		|| ( e_kind == RDSE_WarnTrgTonerEmptyC )		//概要　　：警告発生時刻(トナーエンプティ[C色]) 
//		|| ( e_kind == RDSE_WarnTrgTonerEmptyM )		//概要　　：警告発生時刻(トナーエンプティ[M色]) 
//		|| ( e_kind == RDSE_WarnTrgTonerEmptyY )		//概要　　：警告発生時刻(トナーエンプティ[Y色]) 
//		|| ( e_kind == RDSE_WarnTrgBottleFull )    )	//概要　　：廃トナーボトルFULL
	if(    ( uc_kind == RDSE_WarnTrgTonerEmptyK )		//概要　　：警告発生時刻(トナーエンプティ[K色]) .
		|| ( uc_kind == RDSE_WarnTrgTonerEmptyC )		//概要　　：警告発生時刻(トナーエンプティ[C色]) .
		|| ( uc_kind == RDSE_WarnTrgTonerEmptyM )		//概要　　：警告発生時刻(トナーエンプティ[M色]) .
		|| ( uc_kind == RDSE_WarnTrgTonerEmptyY )		//概要　　：警告発生時刻(トナーエンプティ[Y色]) .
		|| ( uc_kind == RDSE_WarnTrgBottleFull )    )	//概要　　：廃トナーボトルFULL.
// 2010/07/17 higuchi Minerva warning 対応 End.
	{
		// EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)e_Level,  (uchar*)"RDSC_WarnData::chkWarn() e_Level", (uint32_t)e_Level);			//2018/10/18 FPT-DucDH4 Add log debug
		e_CurRdsWarnStat[uc_kind] = e_Level;	// 最新の状態のみログに残す.
	
		// チェックする警告であれば、チェック 
		if( TYPE_RdsWarnStatNormal != e_Level )				// 発生状態
		{
// 2010/07/17 higuchi Minerva warning 対応 Start.
//			if( 0 != c_RecoverWarnTrgTime[e_kind].month   )	// 解除状態検出時間がセットされていたら。※ 年データ（.year）では判断しない事。 
			if( 0 != c_RecoverWarnTrgTime[uc_kind].month   )	// 解除状態検出時間がセットされていたら。※ 年データ（.year）では判断しない事。 
// 2010/07/17 higuchi Minerva warning 対応 End.
			{
				// 発生なら 
				//    受付時間をクリア、
// 2010/07/17 higuchi Minerva warning 対応 Start.
//				EPN_DBG_MESSAGE( (uchar*)"警告処理", (uint32_t)e_kind,  (uchar*)" (解除手前状態キャンセル)" );
//				c_RecoverWarnTrgTime[e_kind].reset();
				EPN_DBG_MESSAGE( (uchar*)"警告処理", (uint32_t)uc_kind,  (uchar*)" (解除手前状態キャンセル)" );
				c_RecoverWarnTrgTime[uc_kind].reset();
// 2010/07/17 higuchi Minerva warning 対応 End.
				__saveCommonAPI( E_WarnTrgTime );
			}
		}
		else
		{
// 2010/07/17 higuchi Minerva warning 対応 Start.
//			if( e_Level == uc_warnData[e_kind] )
			if( e_Level == uc_warnData[uc_kind] )
// 2010/07/17 higuchi Minerva warning 対応 End.
			{
				return( False );
			}

			// 解除なら
// 2010/07/17 higuchi Minerva warning 対応 Start.
//			if( 0 == c_RecoverWarnTrgTime[e_kind].month )	// 年データ（.year）では判断しない事。 
			if( 0 != c_RecoverWarnTrgTime[uc_kind].month )	// 年データ（.year）では判断しない事。 // 2018/10/18 FPT-DucDH4 IT6_1.0[Eagle] 機器構成変更_No.92 Modification
// 2010/07/17 higuchi Minerva warning 対応 End.
			{
				//    トリガ受付時刻が 0 であれば、受付時間をセットしてリターン
// 2010/07/17 higuchi Minerva warning 対応 Start.
//				c_RecoverWarnTrgTime[e_kind] = theVRmsc.getNowTime();
//				__saveCommonAPI( E_WarnTrgTime );
//				EPN_DBG_MESSAGE( (uchar*)"警告処理", (uint32_t)e_kind,  (uchar*)" 解除手前" );
				c_RecoverWarnTrgTime[uc_kind] = theVRmsc.getNowTime();
				__saveCommonAPI( E_WarnTrgTime );
				EPN_DBG_MESSAGE( (uchar*)"警告処理", (uint32_t)uc_kind,  (uchar*)" 解除手前" );
// 2010/07/17 higuchi Minerva warning 対応 End.
				return( False );
			}
			else
			{
				RDSC_Time			c_TimeOutTime;
				RDSC_Time			c_TempNowTime;

// 2010/07/17 higuchi Minerva warning 対応 Start.
//				c_TimeOutTime = c_RecoverWarnTrgTime[e_kind];		// トリガ受付時間を取得し．．． 
				c_TimeOutTime = c_RecoverWarnTrgTime[uc_kind];		// トリガ受付時間を取得し．．． 
// 2010/07/17 higuchi Minerva warning 対応 End.
				uint32_t ul_Value = pc_RdsSetting->getWaitEmpty_Value();
				uchar uc_Unit  = pc_RdsSetting->getWaitEmpty_Unit();
				switch( uc_Unit )
				{
				case 0x00:	// 秒 
					// 単位は秒数のため、取得変数 ul_Value の加工は必要なし.			// 2011/02/15 higuchi Minerva/Zeus 【静的解析 影響度中 CID_140751】.
					break;
				case 0x01:	// 分 
					ul_Value = ul_Value*60;
					break;
				case 0x02:	// 時間 
					ul_Value = ul_Value*60*60;
					break;
				case 0x03:	// 日 
					ul_Value = ul_Value*60*60*24;
					break;
				default :
					break;
				}
				c_TimeOutTime.chgEpNetTime( ul_Value );				// ずらす。 
				c_TempNowTime = theVRmsc.getNowTime();				// 現在時刻の取得 

				if( c_TimeOutTime <= c_TempNowTime )
				{
					//    トリガ受付時刻が 0 でなければ、「トリガ受付時刻+しきい値」が
					//    タイムアウトしていないかどうか確認し、タイムアウトしていれば、
					//    状態を取り込む。

					// (処理なし)
// 2010/07/17 higuchi Minerva warning 対応 Start.
//					if( e_Level != uc_warnData[e_kind] )
//					{
//						EPN_DBG_MESSAGE( (uchar*)"警告処理", (uint32_t)e_kind,  (uchar*)" 解除成立" );
//					}
					if( e_Level != uc_warnData[uc_kind] )
					{
						EPN_DBG_MESSAGE( (uchar*)"警告処理", (uint32_t)uc_kind,  (uchar*)" 解除成立" );
					}
// 2010/07/17 higuchi Minerva warning 対応 End.
				}
				else
				{
					//    タイムアウトしていなければ、リターン 
					return( False );
				}
			}
		}

	}
	/*--------------------------------------------------------------------------------------------------*/

	
	/*--------------------------------------------------------------------------------------------------*/
	// 以降、共通処理  
// 2010/07/17 higuchi Minerva warning 対応 Start.
//	if(    e_Level == uc_warnData[e_kind] 
//		&& RDSE_WarnTrgPartsReplacing != e_kind			// 任意パーツ交換警告、 
//		&& RDSE_WarnTrgSpecificCopy   != e_kind    )	// SOD 警告だけは特殊。変化があった場合のみ、関数をコールしてくるため、ここは素通し。 
	if(    e_Level == uc_warnData[uc_kind] 
		&& RDSE_WarnTrgPartsReplacing != uc_kind			// 任意パーツ交換警告、 
		&& RDSE_WarnTrgSpecificCopy   != uc_kind    )	// SOD 警告だけは特殊。変化があった場合のみ、関数をコールしてくるため、ここは素通し。 
// 2010/07/17 higuchi Minerva warning 対応 End.
	{
		return( False );	// 変化なし 
	}

	// 2015/11/25 mtoyoda MinervaSBK CQ521926 CSRC用PHS通信料高額請求対策 Start.
	if( True == chkPCallSendLimitSetting() )
	{
		// Pコール警告発信の送信制限のリセット条件を満たしたかどうかチェックする.
		// 電源ON時から24時間（86400秒）経っていたら、すべてのPコール警告発信の送信済みフラグと、基準時刻をリセットする.
		if( calcPCallElapsedTime() >= RDSD_PCALL_SEND_LIMIT_INTERVAL )
		{
			resetPCallSendLimit();
		}
		
		// 「警告なし」から「警告あり」に変化したときのみチェックする.
		if( TYPE_RdsWarnStatNormal != e_Level )
		{
			// 対象の警告がセンターに送信済みかどうかチェックする.
			// 対象の警告が増えるようなら条件文に追記する.
			if( RDSE_WarnTrgFuserTempTrouble == e_kind )			// P-32 定着センサ温度検出不良警告発信.
			{
				// 送信済みの場合は、次の制限リセットのタイミングまで送信しない.
				// 未送信の場合は、ここで送信済みフラグをセットする.
				if( True == getPCallSendFlag( e_kind ) )
				{
					return( False );	// 変化なしと同等として、関数を抜ける.
				}
				else
				{
					setPCallSendFlag( e_kind );
				}
			}
		}
	}
	// 2015/11/25 mtoyoda MinervaSBK CQ521926 CSRC用PHS通信料高額請求対策 End.

// 2010/07/17 higuchi Minerva warning 対応 Start.
//	EPN_DBG_MESSAGE( (uchar*)"本体状態変化検出", (uint32_t)e_kind,  (uchar*)" RDSE_WarnTrg" );
	EPN_DBG_MESSAGE( (uchar*)"本体状態変化検出", (uint32_t)uc_kind,  (uchar*)" RDSE_WarnTrg" );
// 2010/07/17 higuchi Minerva warning 対応 End.
	EPN_DBG_MESSAGE( (uchar*)"本体状態変化検出", (uint32_t)e_Level, (uchar*)" TYPE_RdsWarnStat" );
	EPN_DBG_MESSAGE( (uchar*)"本体状態変化検出", (uint32_t)wtPtr,   (uchar*)" -- wtPtr --" );


	if( RDSE_WarnTrgSpecificCopy == wt->uc_Kind )
	{
		// バッファフル、かつ、最終データが SOD 時は、警告取り込み保留。 
		return( False );	// 変化なし 
	}

	// FQ****** 警告多発対応 sone 2011/09/19 start.
	uchar uc_OldwarnData = uc_warnData[uc_kind];

// 2010/07/17 higuchi Minerva warning 対応 Start.
//	uc_warnData[e_kind] = e_Level;
	uc_warnData[uc_kind] = e_Level;
// 2010/07/17 higuchi Minerva warning 対応 End.
	tskRdsEvent.setLocalWarnData( e_kind, uc_warnData[uc_kind], uc_OldwarnData );
	// FQ****** 警告多発対応 sone 2011/09/19 end.
	__saveCommonAPI( E_WarnStat );

	// ■ 警告バッファに登録 
	wt->uc_Map   = E_New;
	wt->uc_Level = (uchar)e_Level;
// 2010/07/17 higuchi Minerva warning 対応 Start.
//	wt->uc_Kind  = (uchar)e_kind;
	wt->uc_Kind  = (uchar)uc_kind;
// 2010/07/17 higuchi Minerva warning 対応 End.
	wt->c_Time   = theVRmsc.getNowTime();
	wt->c_Time.year = wt->c_Time.year % 100;			/* 04/05/28 浦崎 警告発信の応答発信を正しく受け取れなかった問題を修正。yymmddhhmmss の形式で保存する */
	if( NULL != pDetailData )
	{
		memcpy( wt->uc_Detail, pDetailData, 4 ); 
	}

	updateWtPtr();										/* ライトポインタ更新 */
/*	RDS_modem_Kind |= 0x00000008;		*/	// 発着信種別 警告発信bitON CQ385137 対応 2014/02/03 add by tomiyasu
	__saveCommonAPI( E_WarnBuff );
	
// 2022/11/21 FPT-TinhNT16 【IT6_2.2MR4】【OP_BTS-40197】 RTCの時刻ずれ補正対応 Start.
	if( ( RDSE_WarnTrgRTCTime == uc_kind ) &&
		( TYPE_RdsWarnStatNormal == e_Level ) ){
		EPN_DBG_MESSAGE( (uchar*)"Clear data ", (uint32_t)uc_kind,  (uchar*)" Start." );
		adjustWarnState(E_New, E_TransEnd);
		clearTransEndData();
		updateWtPtr();										/* ライトポインタ更新 */
		__saveCommonAPI( E_WarnBuff );
		return( True );
	}
// 2022/11/21 FPT-TinhNT16 【IT6_2.2MR4】【OP_BTS-40197】 RTCの時刻ずれ補正対応 End.
	// 03/12/21 ：KMRDS では、警告の発生/解除ともに「本体警告に関するイベントの一つ」
	//            として扱い、双方とも、「TYPE_RdsTrgKindWarnBody」にてイベント処理部に
	//            通知する。
	tskRdsEvent.indTrg( TYPE_RdsTrgKindWarnBody, 0 );

	return( True );
}

// FQ****** 警告多発対応 yito 2011/09/01 start.
/*==================================================================================================*/
//	警告イベントバッファをログ出力.
/*==================================================================================================*/
void RDSC_WarnData::logWarn( int32_t sl_out )
{
	char sc_temp[256];
	RDSS_WarnData s_warnTemp;
	
	EPN_DBG_MESSAGE( (uchar*)"logWarn", (uint32_t)sl_out,  (uchar*)"-----start-------------------------" );
	for( register int32_t indx = 0; indx < _S_WarnData; indx++ )
	{
		Bool b_print = False;
		// -1なら全項目を無条件で出力.
		if( -1 == sl_out )			b_print = True;
		else if(indx == sl_out) 	b_print = True;
		
		if( True == b_print )
		{
			memcpy(&s_warnTemp, &s_Data[indx], sizeof(s_warnTemp));
			sprintf( sc_temp, "map:%d level:%d kind:%d date:%04d/%02d/%02d %02d:%02d:%02d ",
				s_warnTemp.uc_Map, s_warnTemp.uc_Level,s_warnTemp.uc_Kind,
				s_warnTemp.c_Time.year, s_warnTemp.c_Time.month, s_warnTemp.c_Time.day, 		// yyyy/mm/dd.
				s_warnTemp.c_Time.hour, s_warnTemp.c_Time.minute, s_warnTemp.c_Time.second ); 	// hh:mm:ss.
				
			EPN_DBG_MESSAGE( (uchar*)"logWarn", (uint32_t)indx,  (uchar*)sc_temp );
		}
	}
	EPN_DBG_MESSAGE( (uchar*)"logWarn", (uint32_t)0,  (uchar*)"-----end  -------------------------" );
}
/*==================================================================================================*/
//	警告イベントバッファをログ出力.
/*==================================================================================================*/
void RDSC_WarnData::logChkWarn( int32_t sl_out )
{
	char sc_temp[256];
	RDSS_ChkWarnData s_Temp;
	RDSS_WarnData s_warnTemp;
	
	EPN_DBG_MESSAGE( (uchar*)"logChkWarn", (uint32_t)sl_out,  (uchar*)"-----start-------------------------" );
	for( register int32_t indx = 0; indx < _S_WarnData; indx++ )
	{
		Bool b_print = False;
		// -1なら全項目を無条件で出力.
		if( -1 == sl_out )			b_print = True;
		else if(indx == sl_out) 	b_print = True;
		
		if( True == b_print )
		{
			memcpy(&s_Temp, &s_chkWarnData[indx], sizeof(s_Temp));
			memcpy(&s_warnTemp, &(s_Temp.s_chkWarnData), sizeof(s_warnTemp));
			sprintf( sc_temp, "fg:%d cnt:%d map:%d level:%d kind:%d date:%04d/%02d/%02d %02d:%02d:%02d ",
				s_Temp.b_Exist, s_Temp.uc_cnt, 
				s_warnTemp.uc_Map, s_warnTemp.uc_Level,s_warnTemp.uc_Kind,
				s_warnTemp.c_Time.year, s_warnTemp.c_Time.month, s_warnTemp.c_Time.day, 		// yyyy/mm/dd.
				s_warnTemp.c_Time.hour, s_warnTemp.c_Time.minute, s_warnTemp.c_Time.second ); 	// hh:mm:ss.
				
			EPN_DBG_MESSAGE( (uchar*)"logChkWarn", (uint32_t)indx,  (uchar*)sc_temp );
		}
	}
	EPN_DBG_MESSAGE( (uchar*)"logChkWarn", (uint32_t)0,  (uchar*)"-----end  -------------------------" );
}

/*==================================================================================================*/
//	新規警告をすべて覚える.
/*==================================================================================================*/
void RDSC_WarnData::recordWarn()
{
	EPN_DBG_MESSAGE( (uchar*)"RDSC_WarnData::recordWarn", (uint32_t)0,  (uchar*)"start:" );

	// step0: 現状の警告バッファを状態ログを出す.
	logWarn();
	logChkWarn();

	// step1: ヒットしない警告バッファをチェック用テーブルの登録フラグを落とす。
	deleteCheckWarn();
	
	// step2:警告バッファをチェックする。（発生カウントアップ).
	EPN_DBG_MESSAGE( (uchar*)"recordWarn", (uint32_t)0,  (uchar*)"step2:" );
	RDSS_WarnData s_warnTemp;
	
	for( int32_t sl_indx = 0; sl_indx < _S_WarnData; sl_indx++ )
	{
		Bool b_TableExist = False;	// チェックテーブルに存在する.
		memcpy(&s_warnTemp, &s_Data[sl_indx], sizeof(s_warnTemp));
		
		// 解除が不要な警告は、登録しない.
		if( RDSE_WarnTrgSpecificCopy == s_warnTemp.uc_Kind			// 特定原稿コピー(SOD警告).
			|| RDSE_WarnTrgMCBJFrequecy == s_warnTemp.uc_Kind		// 用紙系JAM.
			|| RDSE_WarnTrgMOBJFrequecy == s_warnTemp.uc_Kind		// 原稿系JAM.
			 )
		{
			continue;
		}

		// Mapの状態は発信中のみ多発状態チェック有効.
		if( E_TransNow == s_warnTemp.uc_Map )
		{
			// １．すでに登録されていたら、uc_cntをカウントアップする.
			for( int32_t sl_chkIndex = 0; sl_chkIndex < _S_WarnData; sl_chkIndex++ )
			{
				// b_ExistがTrueなら処理を続ける。
				if( False == s_chkWarnData[sl_chkIndex].b_Exist )
				{
					continue;
				}
				// 同一（レベル、種類、時間）だったら、カウントをあげる。
				if( ( s_warnTemp.uc_Level == s_chkWarnData[sl_chkIndex].s_chkWarnData.uc_Level ) &&
					( s_warnTemp.uc_Kind == s_chkWarnData[sl_chkIndex].s_chkWarnData.uc_Kind ) &&
					( s_warnTemp.c_Time == s_chkWarnData[sl_chkIndex].s_chkWarnData.c_Time ) )
				{
					b_TableExist = True;	// 登録済み.
					if( s_chkWarnData[sl_chkIndex].uc_cnt != 0xFF )
					{
						s_chkWarnData[sl_chkIndex].uc_cnt++;
					}
					EPN_DBG_MESSAGE( (uchar*)"recordWarn", (uint32_t)sl_chkIndex,  (uchar*)"複数回通過" );
					logWarn(sl_indx);
					break;	// カウントアップしたら抜ける.
				}
			}
			
			// ２．未登録なら、テーブルに登録.
			if( False == b_TableExist )
			{
				for( int32_t sl_chkIndex = 0; sl_chkIndex < _S_WarnData; sl_chkIndex++ )
				{
					if( False == s_chkWarnData[sl_chkIndex].b_Exist)
					{
						s_chkWarnData[sl_chkIndex].b_Exist = True;
						memcpy(&(s_chkWarnData[sl_chkIndex].s_chkWarnData), &s_warnTemp, sizeof(s_warnTemp) );
						s_chkWarnData[sl_chkIndex].uc_cnt = 1;		// 登録時に1にする.
						EPN_DBG_MESSAGE( (uchar*)"recordWarn", (uint32_t)sl_chkIndex,  (uchar*)"チェック用に登録" );
						logWarn(sl_indx);
						break;	// 登録したら抜ける.
					}
				}
			}
		}
	}

	// step3:比較テーブルのカウントが再送信回数＋２回の場合、ログ保存する.
	EPN_DBG_MESSAGE( (uchar*)"recordWarn", (uint32_t)0,  (uchar*)"step3:" );
	{
		Bool b_LogFlag = False;		// True:ログ保存する, False:しない.
		uchar uc_limitCnt = 0;
		if((RDSE_TSK_ID_TELL == theRdsApl.e_CommDevice) || (RDSE_TSK_ID_FAX == theRdsApl.e_CommDevice))
		{
			uc_limitCnt = (uchar)(pc_RdsSetting->getModemRetryCount() + 2);		// モデム/FAX用.
		}
		else
		{
			uc_limitCnt = (uchar)(pc_RdsSetting->getMailRetryCount() + 2);		// Email/HTTP用.
		}
		
		int32_t sl_chkIndex;
		for( sl_chkIndex = 0; sl_chkIndex < _S_WarnData; sl_chkIndex++ )
		{
			if( True == s_chkWarnData[sl_chkIndex].b_Exist )
			{
				if( uc_limitCnt == s_chkWarnData[sl_chkIndex].uc_cnt )
				{
					b_LogFlag = True;
				}
			}
		}
		if( True == b_LogFlag )
		{	// ★★ログ保存.
			EPN_DBG_MESSAGE( (uchar*)"recordWarn", (uint32_t)sl_chkIndex,  (uchar*)"step3:log save!" );
			tskRdsEvent.DBGLog( 4 );
		}
	}

	
	// getでE_TransNowのイベントを見ていくため準備しておく。
	RDSC_WarnData::setTargetPtr( E_TransNow );
	EPN_DBG_MESSAGE( (uchar*)"RDSC_WarnData::recordWarn", (uint32_t)0,  (uchar*)"end:" );
}
/*==================================================================================================*/
//	警告イベントバッファに存在するもの以外、比較テーブルから削除.
/*==================================================================================================*/
void RDSC_WarnData::deleteCheckWarn()
{
	EPN_DBG_MESSAGE( (uchar*)"deleteCheckWarn", (uint32_t)0,  (uchar*)"start:" );

	RDSS_WarnData s_warnTemp;
	// ヒットしない警告バッファを見て、チェック用の登録フラグを落とす。
	// 		s_chkWarnData：有、s_Data：無　→s_chkWarnDataから登録削除する。
	for( int32_t sl_chkIndex = 0; sl_chkIndex < _S_WarnData; sl_chkIndex++ )
	{
		if( False == s_chkWarnData[sl_chkIndex].b_Exist )
		{
			continue;
		}
		
		Bool b_TableExist = False;	// チェックテーブルに存在するかどうか.
		for( int32_t sl_indx = 0; sl_indx < _S_WarnData; sl_indx++ )
		{
			memcpy(&s_warnTemp, &s_Data[sl_indx], sizeof(s_warnTemp));
			if( E_NoData != s_warnTemp.uc_Map )
			{
				if( ( s_warnTemp.uc_Level == s_chkWarnData[sl_chkIndex].s_chkWarnData.uc_Level ) &&
					( s_warnTemp.uc_Kind == s_chkWarnData[sl_chkIndex].s_chkWarnData.uc_Kind ) &&
					( s_warnTemp.c_Time == s_chkWarnData[sl_chkIndex].s_chkWarnData.c_Time ) )
				{
					b_TableExist = True;
				}
			}
		}
		
		// 警告テーブルにないため、b_Existを落とす.
		if( False == b_TableExist )
		{				
			s_chkWarnData[sl_chkIndex].b_Exist = False;
			memset( &(s_chkWarnData[sl_chkIndex].s_chkWarnData), 0, sizeof(RDSS_WarnData) );
			s_chkWarnData[sl_chkIndex].uc_cnt = 0;
		}
	}

	EPN_DBG_MESSAGE( (uchar*)"deleteCheckWarn", (uint32_t)0,  (uchar*)"end:" );
}
/*==================================================================================================*/
//	警告多発状態フラグをOFF/ONへの変更を判断する.
/*==================================================================================================*/
Bool RDSC_WarnData::checkRepeatedOccured( Bool b_EnableOffToOn )
{
	Bool b_TempRepeatFlag = False;		// True:多発状態の警告を発見, False:未発見.
	
	// 同一警告チェックをしない.
	if( True == b_DisableOccuredCheck )
	{
		EPN_DBG_MESSAGE( (uchar*)"checkRepeatedOccured", (uint32_t)0,  (uchar*)"b_DisableOccuredCheck:True" );
		b_DisableOccuredCheck = False;
		return False;
	}
	
	for( int32_t sl_chkIndex = 0; sl_chkIndex < _S_WarnData; sl_chkIndex++ )
	{
		if( True == s_chkWarnData[sl_chkIndex].b_Exist )
		{
			if( RDSD_ChkWarnRepeatCount <= s_chkWarnData[sl_chkIndex].uc_cnt )	// ★TODO:RDSD_ChkWarnRepeatCount値、現状1.
			{
				// 多発状態を検知!!!.
				EPN_DBG_MESSAGE( (uchar*)"checkRepeatedOccured", (uint32_t)s_chkWarnData[sl_chkIndex].s_chkWarnData.uc_Kind,  (uchar*)"uc_Kind:" );
				b_TempRepeatFlag = True;
			}
		}
	}
	if( True == b_EnableOffToOn )
	{
		// 警告多発状態フラグに設定.
		if( True == b_TempRepeatFlag )
		{
			if( False == b_RepeatedOccured )	// OFF→ONのとき.
			{
				tskRdsEvent.DBGLog( 3 );		//★★ログ保存を行う。
			}	
			EPN_DBG_MESSAGE( (uchar*)"checkRepeatedOccured", (uint32_t)0, (uchar*)"b_RepeatedOccured = True" );
			b_RepeatedOccured = True;
		}
	}
	if( False == b_TempRepeatFlag )
	{
		EPN_DBG_MESSAGE( (uchar*)"checkRepeatedOccured", (uint32_t)0, (uchar*)"b_RepeatedOccured = Flase" );
		b_RepeatedOccured = False;
	}
	
	return (b_RepeatedOccured); 
}

/*==================================================================================================*/
//	多発警告かどうかのチェック.
/*==================================================================================================*/
Bool RDSC_WarnData::isRepeatedWarn( RDSS_WarnData *ps_warnData )
{
	if( NULL == ps_warnData )		// ポインタがNULL（バカよけ）.
	{
		return False;
	}

	for( int32_t sl_chkIndex = 0; sl_chkIndex < _S_WarnData; sl_chkIndex++ )
	{
		if( True == s_chkWarnData[sl_chkIndex].b_Exist )
		{
			// 同じレベル、同じ種類、同じ時間、
			if( ( ps_warnData->uc_Level == s_chkWarnData[sl_chkIndex].s_chkWarnData.uc_Level ) &&
				( ps_warnData->uc_Kind == s_chkWarnData[sl_chkIndex].s_chkWarnData.uc_Kind ) &&
				( ps_warnData->c_Time == s_chkWarnData[sl_chkIndex].s_chkWarnData.c_Time ) )
			{
//				if( RDSD_ChkWarnRepeatCount <= s_chkWarnData[sl_chkIndex].uc_cnt)
				if( RDSD_ChkWarnRepeatCount < s_chkWarnData[sl_chkIndex].uc_cnt)		// テーブル登録時の1回は有効とする。
				{
					return True;
				}
			}
		}
	}

	return False;
}
/*==================================================================================================*/
//	本体警告データの個数を取得.
/*==================================================================================================*/
int32_t RDSC_WarnData::getNowWarnCount()
{
	int32_t sl_warnCnt = 0;
	RDSS_WarnData s_warnTemp;
	
	for( int32_t sl_indx = 0; sl_indx < _S_WarnData; sl_indx++ )
	{
		memcpy(&s_warnTemp, &s_Data[sl_indx], sizeof(s_warnTemp));
		
		if( E_NoData != s_warnTemp.uc_Map )
		{
			sl_warnCnt++;
		}
	}
	return sl_warnCnt;
}
/*==================================================================================================*/
//	本体警告データの個数を取得.
/*==================================================================================================*/
Bool RDSC_WarnData::isRepeatedOccuredAllNowWarn()
{
	int32_t sl_warnCnt = 0;
	int32_t sl_RepeatedOccuredCnt = 0;		// 多発状態の警告数。
	RDSS_WarnData s_warnTemp;
	
	for( int32_t sl_indx = 0; sl_indx < _S_WarnData; sl_indx++ )
	{
		memcpy(&s_warnTemp, &s_Data[sl_indx], sizeof(s_warnTemp));
		
		if( E_NoData != s_warnTemp.uc_Map )
		{
			sl_warnCnt++;
			// 多発警告と同一警告かのチェック。
			for( int32_t sl_chkIndex = 0; sl_chkIndex < _S_WarnData; sl_chkIndex++ )
			{
				if( True == s_chkWarnData[sl_chkIndex].b_Exist )
				{
					// 同じレベル、同じ種類、同じ時間、
					if( ( s_warnTemp.uc_Level == s_chkWarnData[sl_chkIndex].s_chkWarnData.uc_Level ) &&
						( s_warnTemp.uc_Kind == s_chkWarnData[sl_chkIndex].s_chkWarnData.uc_Kind ) &&
						( s_warnTemp.c_Time == s_chkWarnData[sl_chkIndex].s_chkWarnData.c_Time ) )
					{
						if( RDSD_ChkWarnRepeatCount <= s_chkWarnData[sl_chkIndex].uc_cnt)
						{
							sl_RepeatedOccuredCnt++;
						}
					}
				}
			}
		}
	}
	// 一つも警告がない場合、問題なし。
	if( 0 == sl_warnCnt )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDSC_WarnData::isRepeatedOccuredAllNowWarn", (uint32_t)0,  (uchar*)"sl_warnCnt:" );
		return False;
	}
	// 発生した警告数と同じ場合、。
	if( sl_RepeatedOccuredCnt == sl_warnCnt )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDSC_WarnData::isRepeatedOccuredAllNowWarn", (uint32_t)sl_RepeatedOccuredCnt,  (uchar*)"sl_RepeatedOccuredCnt:" );
		return True;
	}
	
	EPN_DBG_MESSAGE( (uchar*)"RDSC_WarnData::isRepeatedOccuredAllNowWarn", (uint32_t)sl_warnCnt,  (uchar*)"sl_warnCnt:" );
	return False;
}
// FQ****** 警告多発対応 yito 2011/09/01 end  .
/*==================================================================================================*/
//	本体警告データゲット
/*==================================================================================================*/
Bool RDSC_WarnData::get( RDSS_WarnData* pData )
{
	EPN_DBG_MESSAGE( (uchar*)"RDSC_WarnData::get", (uint32_t)_ss_dataNum,  (uchar*)"_ss_dataNum:" );
	Bool	result = False;												// データ無しで初期化

	b_dataExist = True;		// 04/03/03 浦崎 警告バッファのリングバッファでは、空の要素であってもリードポインタを更新できないといけないため、ここでデータ有り状態としておく。  

	if( 0 < _ss_dataNum )
	{
		memcpy( pData, rd, sizeof(RDSS_WarnData) );
		// FQ****** 警告多発対応 sone 2011/09/13 start.
		if( tskRdsEvent.ss_DBG_Flag == 0 )	// デバッグ機能が無効.
		{
			rd->uc_Map = E_TransEnd;										// 状態を取得済みにする。
		}
		else
		{
			switch( tskRdsEvent.ss_DBG_Flag )
			{
				case 3:
					EPN_DBG_MESSAGE( (uchar*)"EVENT", (uint32_t)0, (uchar*)" ★警告発信で、「発信MAPと警告イベントを発信中のまま」なのでE_TransEndにしない" );
					break;
				case 1:
				case 2:
				case 4:
				default:
				{
					rd->uc_Map = E_TransEnd;										// 状態を取得済みにする。
				}
					break;
			}
		}
		// FQ****** 警告多発対応 sone 2011/09/13 end.

		updateRdPtr();													/* リードポインタ更新 */

		// 残データの確認

		// ◆ 方法１・・・×
		// リードポインタを退避
		// リフレッシュリードポインタ
		// E_TransNow があるかサーチ
		// あったら True
		// なかったら False
		// 最後にリードポインタを復帰
		//	return( isDataExist() );
		//
		// ◆ 方法２・・・○
		// transStart() の時点で E_TransNow にした数を覚えておいて get() の度に減算していく？
		// で、残り数を返す。

		_ss_dataNum--;
		result = True;
	}
	b_dataExist = True;

	//(不要)__saveCommonAPI();
	return( result );
}

/*==================================================================================================*/
//	送信処理開始→全ての要素を[発信中]に移動
/*==================================================================================================*/
void RDSC_WarnData::transStart( void )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"#[39]RDSC_WarnData::transStart[st]" );
	adjustWarnState( E_New,      E_TransNow );				/* 発信要求マップ２→１ */
	adjustWarnState( E_TransNow, E_TransEnd );				/* 発信要求マップ１→０ */
	adjustWarnState( E_TransEnd, E_TransNow );				/* 発信要求マップ０→１ */
	_ss_dataNum = 0;

	b_dataExist = True;		// 04/03/03 浦崎 警告バッファのリングバッファでは、空の要素であってもリードポインタを更新できないといけないため、ここでデータ有り状態としておく。  

	resetRdPtr();
	for( register int indx = 0; indx < _S_WarnData; indx++ )
	{
		if( E_TransNow == rd->uc_Map )
		{
			_ss_dataNum++;
		}
		updateRdPtr();										/* リードポインタ更新 */
	}
	b_dataExist = True;

	// リードポインタを、[発信中] の要素のところにもっていかないといけない。
	RDSC_WarnData::setTargetPtr( E_TransNow );
	EPN_DBG_MESSAGE( (uchar*)"RDSC_WarnData::transStart()", (uint32_t)rdPtr,   (uchar*)" 読み出し位置セット" );
	EPN_DBG_MESSAGE( (uchar*)"RDSC_WarnData::transStart()", (uint32_t)_ss_dataNum,   (uchar*)" 送信警告データ数" );

	//(不要)__saveCommonAPI();
}
/*==================================================================================================*/
//	便乗発信：折り返しの送信開始→[新規]を[発信中]に移動(テレフォ専用）
/*==================================================================================================*/
void RDSC_WarnData::reTransStart( void )
{
	adjustWarnState( E_New, E_TransNow );					/* 発信要求マップ２→１ */

	b_dataExist = True;		// 04/03/03 浦崎 警告バッファのリングバッファでは、空の要素であってもリードポインタを更新できないといけないため、ここでデータ有り状態としておく。  

	_ss_dataNum = 0;
	resetRdPtr();
	for( register int indx = 0; indx < _S_WarnData; indx++ )
	{
		if( E_TransNow == rd->uc_Map )
		{
			_ss_dataNum++;
		}
		updateRdPtr();										/* リードポインタ更新 */
	}
	b_dataExist = True;

	// リードポインタを、[発信中] の要素のところにもっていかないといけない。
	RDSC_WarnData::setTargetPtr( E_TransNow );
	//(不要)__saveCommonAPI();
}
/*==================================================================================================*/
//	送信中→送信済み（応答待ち）に移動
/*==================================================================================================*/
void RDSC_WarnData::transEnd()
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"#[53]RDSC_WarnData::transEnd[st]" );
	adjustWarnState( E_TransNow, E_TransEnd );				/* 発信要求マップ１→０ */
	//(不要)__saveCommonAPI();
}
/*==================================================================================================*/
//	応答発信終了→応答発信が完了したときにクリアするデータをクリア
/*==================================================================================================*/
void RDSC_WarnData::resEnd( void )
{
	/* 内容は何もなし */
	//(不要)__saveCommonAPI();
}
/*==================================================================================================*/
//	応答タイムアウト検出→全ての要素を[新規]に移動
/*==================================================================================================*/
void RDSC_WarnData::refresh( void )
{
	adjustWarnState( E_TransNow, E_New );					/* 発信要求マップ１→２ */
	adjustWarnState( E_TransEnd, E_New );					/* 発信要求マップ０→２ */
	//(不要)__saveCommonAPI();
}
/*==================================================================================================*/
//	センターに伝わったと判断できた時点で行う処理 
//    E-mail版以外で、送信正常終了時にコールされる。 
/*==================================================================================================*/
void RDSC_WarnData::JobNormalEnd( void )
{
// 2022/11/21 FPT-TinhNT16 【IT6_2.2MR4】【OP_BTS-40197】 RTCの時刻ずれ補正対応 Start.
	TYPE_RdsMode e_RdsMode = pc_RdsSetting->getRdsMode();
	Bool b_Method = pc_RdsSetting->getCsrcCommunicationMethod();
	
	if ( (TYPE_RdsModeMailForOne == e_RdsMode )
	|| ( (TYPE_RdsModeHttp == e_RdsMode) && (False == b_Method) ) )			// One-way communication.
	{
		RDSS_WarnData s_warnTemp;
		for( int32_t sl_indx = 0; sl_indx < _S_WarnData; sl_indx++ )
		{
			memcpy(&s_warnTemp, &s_Data[sl_indx], sizeof(s_warnTemp));
			if ( (RDSE_WarnTrgRTCTime == s_warnTemp.uc_Kind ) && ( TYPE_RdsWarnStatErr == s_warnTemp.uc_Level ) )
			{
				EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)s_warnTemp.uc_Kind, (uchar*)" notifyWarning2() : Release" );
				Bool b_SendNotify = False;
				c_mkText_ModeWarn.correctRTCTime(RDSE_WarnTrgRTCTime, b_SendNotify);	// Release warning.
			}
		}
	}
// 2022/11/21 FPT-TinhNT16 【IT6_2.2MR4】【OP_BTS-40197】 RTCの時刻ずれ補正対応 End.

	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"#[57]RDSC_WarnData::JobNormalEnd[st]" );
	if(    (RDSE_TSK_ID_TELL == theRdsApl.e_CommDevice)
		|| (RDSE_TSK_ID_FAX == theRdsApl.e_CommDevice)
		|| (RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice)
		|| (RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice)	// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
		|| ((RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice)  && ( True == theRdsApl.isSimplex())) )//【CSRC Email片方向通信対応】08/08/05 芳賀. 
	{
		// uc_Stat が E_TransEnd のデータをクリアする。
		clearTransEndData();
	}
	else
	{
		// E-mail版では、特に処理なし
	}
	//(不要)__saveCommonAPI();
}

/*==================================================================================================*/
//	センターからの応答があったとき、等しいイベントID のイベントがあればクリアする。
/*--------------------------------------------------------------------------------------------------*/
// 04/06/14 浦崎 個々のイベントに対して応答を確認するようにした。 
/*==================================================================================================*/
void RDSC_WarnData::chkAns( uchar* pData, uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	
	// ID が同じものをクリアする。

	RDSC_Time			c_rcvTime;

	// BCD から時刻へ変換 
	c_rcvTime.cnvString_2_Time( bcd_rcvEventID, RDSE_T2S_BCDyymmddhhmmss );

	printEpTime( (RDSS_DetailTime)c_rcvTime,			(uchar*)"(応答発信のID)" );




	// 応答が確認出来た場合、E_TransEnd のデータを全て削除する。
	// 本体 F/W は応答の確認が取れていないデータに関しては
	// 便乗して全て送信するため、E_TransEnd となっているデータに 
	// 関しては全てホストに通知出来たと考える事が出来る。
	b_dataExist = True;		// 04/03/03 浦崎 警告バッファのリングバッファでは、空の要素であってもリードポインタを更新できないといけないため、ここでデータ有り状態としておく。  

	resetRdPtr();
	//Declared indx outside for loop, for VC6 to VS2K8 Porting. 2009/10/29 HCLT Start.
    //for( register int indx = 0; indx < _S_WarnData; indx++ )
	  register int indx = 0;
	  for( indx = 0; indx < _S_WarnData; indx++ )
    //Declared indx outside for loop, for VC6 to VS2K8 Porting. 2009/10/29 HCLT End.
	{

		uchar uc_TempWarnDataString[60];
		ushort us_Size;
		//  
		memset( uc_TempWarnDataString, 0, sizeof(uc_TempWarnDataString) );
		us_Size = (ushort)(strlen( _WarnString[rd->uc_Kind][rd->uc_Level] ));	// 2010/07/17 sone Minerva warning 対応 ucharにキャスト.
		memcpy( uc_TempWarnDataString, _WarnString[rd->uc_Kind][rd->uc_Level], us_Size );



		if(    c_rcvTime == rd->c_Time 
			&& True == EpLib_datacmp( (pData+1), uc_TempWarnDataString, us_Size )    )
		{

			EPN_DBG_MESSAGE( (uchar*)"RDSC_WarnData", (uint32_t)0,  (uchar*)" ID 一致!!!!!!" );

			// rd を退避。 
			ushort us_tmpRdIndx = getRdIndx();

			// 消す。 
			if( E_TransEnd != rd->uc_Map ){ break; }	// 送信完了していなければ、break。 
			memset( rd, 0, sizeof(RDSS_WarnData) );

			// 整理。 
			if(    0 == indx
				&& getWtIndx() == us_tmpRdIndx )
			{
				// indx が 0 で ID が等しい場合は、データフル状態での応答受信と言うこと。
				// よって、ポインタ操作不要 → break; 
				break;
			}
			else if( getWtIndx() == getRdIndx() )
			{
				// indx が 0 じゃなくて、
				// リードポインタとライトポインタが同じであれば、バッファを１周したと言うこと。
				// よって、ポインタ操作不要 → break; 
				break;
			}
			else
			{
				RDSS_WarnData s_CopyData;
				for( int indx2=0; indx2 < indx; indx2++ )
				{
					// ※カレント領域はクリアされている領域。 
					// rd を退避。 
					ushort us_tmpRdIndx2 = getRdIndx();
					backRdPtr();										/* 一個前のものをコピーしておいて．．． */
					if( E_NoData == rd->uc_Map )						/* データが入ってなければ処理不要 */
					{
						break;
					}

					s_CopyData = *rd;									/* 一個前のものを退避しておいて．．． */
					memset( rd, 0, sizeof(RDSS_WarnData) );				/* (退避させた時点で消しておく。)*/
					// rd を復帰。 
					setRdIndx( us_tmpRdIndx2 );
					*rd = s_CopyData;									/* 詰める。 */
					backRdPtr();										/* ポインタ更新 */
					if( getWtIndx() == getRdIndx() )
					{
						break;
					}
				}
			}

			break;

		}
		updateRdPtr();										/* リードポインタ更新 */
	}

	resetRdPtr();											/*  */
	b_dataExist = True;



	// 全ての NowTrans、TransEnd の発信に対して応答の確認が取れたら、警告発信の発信要求マップ[済]を
	// 削除する。 
	Bool	b_Remain = False;	// 応答待ち警告無しにて初期化。 
	resetRdPtr();
	for( indx = 0; indx < _S_WarnData; indx++ )
	{
		if(    E_TransEnd == rd->uc_Map
			|| E_TransNow == rd->uc_Map   )
		{
			// まだ、応答待ちの発信がある。 
			b_Remain = True;
			EPN_DBG_MESSAGE( (uchar*)"RDSC_WarnData", (uint32_t)indx,  (uchar*)" まだ、応答待ちの警告発信がある。" );
			break;
		}
		updateRdPtr();										/* リードポインタ更新 */
	}
	resetRdPtr();											/*  */
	b_dataExist = True;

	// FQ****** 警告多発対応 yito 2011/09/01 start.
	// 警告イベントバッファに無い項目をチェック用テーブルから削除.
	c_mkText_ModeWarn.deleteCheckWarn();
	c_mkText_ModeWarn.checkRepeatedOccured( False );		// 警告多発フラグON→OFF(OFF→OFFも行う)のチェックを行う。
	// FQ****** 警告多発対応 yito 2011/09/01 end  .

	if( False == b_Remain )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDSC_WarnData", (uint32_t)0,  (uchar*)" 全警告のイベントに対する応答受信完了！" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す。 
		theTrgMap.resetMapByTrgAllPage( TYPE_RdsTrgKindWarnBody );	// 05/02/28 浦崎 resetMapByTrg( , 0 ) → resetMapByTrgAllPage() 
		// ID も消す。 
		uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( TYPE_RdsTrgKindWarnBody, bcd_bkEventID );
	}


	__saveCommonAPI( E_WarnALL );
}
/*==================================================================================================*/
//	センターに通知完了したイベントデータ「uc_Stat が E_TransEnd の」データをクリアする。
/*==================================================================================================*/
void RDSC_WarnData::clearTransEndData()
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"#[58]RDSC_WarnData::clearTransEndData[st]" );
	// uc_Stat が E_TransEnd のデータをクリアする。
	b_dataExist = True;		// 04/03/03 浦崎 警告バッファのリングバッファでは、空の要素であってもリードポインタを更新できないといけないため、ここでデータ有り状態としておく。  

	resetRdPtr();
	for( register int indx = 0; indx < _S_WarnData; indx++ )
	{
		if( E_TransEnd == rd->uc_Map )
		{
			memset( rd, 0, sizeof(RDSS_WarnData) );
		}
		updateRdPtr();										/* リードポインタ更新 */
	}
	resetRdPtr();
	b_dataExist = True;

	__saveCommonAPI( E_WarnBuff );
}


/*==================================================================================================*/
//	警告情報通知状態を、src の状態から dst の状態にする。
/*==================================================================================================*/
void RDSC_WarnData::adjustWarnState( RDSE_WarnMapStat src, RDSE_WarnMapStat dst )
{
	b_dataExist = True;		// 04/03/03 浦崎 警告バッファのリングバッファでは、空の要素であってもリードポインタを更新できないといけないため、ここでデータ有り状態としておく。  

	resetRdPtr();
	for( register int indx = 0; indx < _S_WarnData; indx++ )
	{
		if( (uchar)src == rd->uc_Map )
		{
			rd->uc_Map = (uchar)dst;
		}
		updateRdPtr();										/* リードポインタ更新 */
	}
	resetRdPtr();
	b_dataExist = True;

	__saveCommonAPI( E_WarnBuff );
}

/*==================================================================================================*/
//	リードポインタ位置をセット
//		引数 target で指定されたステータスの最も古いデータを指す。
/*==================================================================================================*/
void RDSC_WarnData::setTargetPtr( RDSE_WarnMapStat target )
{
	b_dataExist = True;		// 04/03/03 浦崎 警告バッファのリングバッファでは、空の要素であってもリードポインタを更新できないといけないため、ここでデータ有り状態としておく。  

	resetRdPtr();											/* リードポインタをライトポインタと同じ位置にする。 */
	for( register int indx = 0; indx < _S_WarnData; indx++ )
	{
		if( target == rd->uc_Map )
		{
			return;
		}
		updateRdPtr();										/* リードポインタ更新 */
	}
	b_dataExist = True;

	//(不要)__saveCommonAPI();
}

/*--------------------------------------------------------------------------------------------------*/
//
/*--------------------------------------------------------------------------------------------------*/
ushort BitData_2_Array( uchar* pSrc, uchar* pDst, uchar* puc_StartTopBit )
{
	int si_NowBit;
	int si_Shift;
	ushort indx_Src = 0;
	ushort indx_Dst = 0;
	ushort indx_ALL = 0;
	uchar uc_bit = 0;


	si_NowBit = *puc_StartTopBit;

	uchar uc_temp = 30%8;
	si_Shift = (8 + uc_temp - 1) % 8;


	//si_Shift = 7;
	for( indx_ALL=0; indx_ALL<30; indx_ALL++ )
	{

		uc_bit = (uchar)(0 == (pSrc[indx_Src] & (0x01<<si_NowBit)) ? 0x00 : 0x01);
		pDst[indx_Dst] = (uchar)(pDst[indx_Dst] | (uc_bit << si_Shift));	// 2010/07/17 sone Minerva warning 対応 ucharにキャスト.

		si_NowBit--;
		if( si_NowBit < 0 ){
			si_NowBit = 7;
			indx_Src++;
		}
		si_Shift--;
		if( si_Shift < 0 ){
			si_Shift = 7;
			indx_Dst++;
		}

	}

	*puc_StartTopBit = (uchar)si_NowBit;
	return( indx_Src );
}
/*--------------------------------------------------------------------------------------------------*/
//
/*--------------------------------------------------------------------------------------------------*/
ushort Array_2_BitData( uchar* pSrc, uchar* pDst, uchar* puc_StartTopBit )
{
	int si_NowBit;
	int si_Shift;
	ushort indx_Src = 0;
	ushort indx_Dst = 0;
	ushort indx_ALL = 0;
	uchar uc_bit = 0;


	si_NowBit = *puc_StartTopBit;

	// 有効ビット長は30。有効データの開始ビットを算出。 
	uchar uc_temp = 30%8;
	si_Shift = (8 + uc_temp - 1) % 8;

	for( indx_ALL=0; indx_ALL<30; indx_ALL++ )
	{
		// 2010/07/17 sone Minerva warning 対応 ucharにキャスト START.
		uc_bit = (uchar)(0 == (pSrc[indx_Src] & (0x01<<si_Shift)) ? 0x00 : 0x01);
		pDst[indx_Dst] = (uchar)(pDst[indx_Dst] | (uc_bit << si_NowBit));
		// 2010/07/17 sone Minerva warning 対応 ucharにキャスト END.

		si_NowBit--;
		if( si_NowBit < 0 ){
			si_NowBit = 7;
			indx_Dst++;
		}
		si_Shift--;
		if( si_Shift < 0 ){
			si_Shift = 7;
			indx_Src++;
		}

	}

	*puc_StartTopBit = (uchar)si_NowBit;
	return( indx_Dst );
}

/*==================================================================================================*/
//	CommonAPI にデータセーブ
/*--------------------------------------------------------------------------------------------------*/
// 04/01/06 浦崎  警告解除受付時間追加。
// 04/08/04 浦崎  PMカウンタ警告状態のセット処理で、転写ローラユニットライフの状態を参照してしまっていた
//                ため修正。
/*==================================================================================================*/
void RDSC_WarnData::__saveCommonAPI( RDSC_WarnData::RDSE_C_AccessType e_Type )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"#RDSC_WarnData::__saveCommonAPI[st]", (uint32_t)e_Type );
	RDSS_WarnData*	p_WarnData;
	uchar			tmpWarnData[160];
	uchar			tmpDetailPart[64+10];		// 04/04/09 浦崎 
	uchar*			pSrc;
	uchar*			pDetailPart;

	ushort us_DetailPartIndx = 0;
	ushort us_AddIndx = 0;
	uchar  uc_bitInfo = 7;

	/* 警告バッファ（160byte） */
	memset( tmpDetailPart, 0, sizeof(tmpDetailPart) );
	if(    (E_WarnBuff == e_Type)		//警告イベントバッファを保存する場合、
		|| (E_WarnALL == e_Type)   )	//もしくは警告関連すべてを保存する場合。
	{
		pSrc = tmpWarnData;
		pDetailPart = tmpDetailPart;
		// 04/04/09 浦崎 ループ最大値を RDSE_WarnTrg_NumOfTrg から _S_WarnData に修正。 
		//               RDSE_WarnTrg_NumOfTrg を増やそうとしたところ、ここで使用している 
		//               定数にミスがある事に気付いた。
		//               ※ 双方ともサイズは等しかったため、問題はなかった。 
		for( int indx = 0; indx < _S_WarnData; indx++ )
		{
			p_WarnData = &s_Data[indx];

			*pSrc = p_WarnData->uc_Map;			// MAP(0,1,2) （1byte）
			pSrc++;
			*pSrc = p_WarnData->uc_Level;		// 警告レベル（TYPE_RdsWarnStat）（1byte）
			pSrc++;
			*pSrc = p_WarnData->uc_Kind;		// 警告種類（1byte）
			pSrc++;
			//発生時間（イベントIDとして使用される）（6byte）
			pSrc += p_WarnData->c_Time.cnvTime_2_String( pSrc, RDSE_T2S_BCDyymmddhhmmss );
			// Detail部分 04/04/09 浦崎 
#if 1
			us_AddIndx = Array_2_BitData( p_WarnData->uc_Detail, &pDetailPart[us_DetailPartIndx], &uc_bitInfo );
			us_DetailPartIndx = (ushort)(us_DetailPartIndx + us_AddIndx);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.
#else
			memcpy( pDetailPart, p_WarnData->uc_Detail, 4 );
			pDetailPart += 4;
#endif
		}

		pc_RdsSetting->setWarnEvent( tmpWarnData );
		pc_RdsSetting->setWarnEVDetailData( tmpDetailPart );
		pc_RdsSetting->setWarnRdPtr( (uchar)rdPtr );
		pc_RdsSetting->setWarnWtPtr( (uchar)wtPtr );
	}

	/* 警告イベント */
	if(    (E_WarnStat == e_Type)		//警告状態管理データを保存する場合、
		|| (E_WarnALL == e_Type)   )	//もしくは警告関連すべてを保存する場合。
	{
		pc_RdsSetting->setTonerStatYellow		( uc_warnData[RDSE_WarnTrgTonerEmptyY]		 );								/* Yトナーエンプティ						 */
		pc_RdsSetting->setTonerStatMagenta		( uc_warnData[RDSE_WarnTrgTonerEmptyM]		 );								/* Mトナーエンプティ						 */
		pc_RdsSetting->setTonerStatCyan			( uc_warnData[RDSE_WarnTrgTonerEmptyC]		 );								/* Cトナーエンプティ						 */
		pc_RdsSetting->setTonerStatBlack		( uc_warnData[RDSE_WarnTrgTonerEmptyK]		 );								/* Kトナーエンプティ						 */
		pc_RdsSetting->setUseTonerStat			( uc_warnData[RDSE_WarnTrgBottleFull]		 );								/* 廃トナーボトルフル						 */
		pc_RdsSetting->setFuserStat				( uc_warnData[RDSE_WarnTrgFuserUnitLife]	 );								/* 定着ユニットライフ						 */
		pc_RdsSetting->setRdsTonerFilterStat	( uc_warnData[RDSE_WarnTrgTonerFilter]		 );								/* トナーフィルタ警告						 */

// 2010.10.18 管理No.675対応 Iwata Start .
#if !defined(DEF_A4_Cus) && !defined(DEF_HeliosMLKCus)		// 2019.10.09 【HeliosMLK】機器構成変更_No.9 ht_kurihara.

		pc_RdsSetting->setICStatYellow			( uc_warnData[RDSE_WarnTrgICLifeYDC]		 );								/* DCライフY								 */
		pc_RdsSetting->setICStatMagenta			( uc_warnData[RDSE_WarnTrgICLifeMDC]		 );								/* DCライフM								 */
		pc_RdsSetting->setICStatCyan			( uc_warnData[RDSE_WarnTrgICLifeCDC]		 );								/* DCライフC								 */

		pc_RdsSetting->setStatYellowDevC		( uc_warnData[RDSE_WarnTrgICLifeYDevC]		 );								/* DevCライフY								 */
		pc_RdsSetting->setStatMagentaDevC		( uc_warnData[RDSE_WarnTrgICLifeMDevC]		 );								/* DevCライフM								 */
		pc_RdsSetting->setStatCyanDevC			( uc_warnData[RDSE_WarnTrgICLifeCDevC]		 );								/* DevCライフC								 */
#else
		//デフォルトではMinerva用の設定を記載する。
		pc_RdsSetting->setICStatYellow			( uc_warnData[RDSE_WarnTrgICLifeY]			 );								/* ICライフY								 */
		pc_RdsSetting->setICStatMagenta			( uc_warnData[RDSE_WarnTrgICLifeM]			 );								/* ICライフM								 */
		pc_RdsSetting->setICStatCyan			( uc_warnData[RDSE_WarnTrgICLifeC]			 );								/* ICライフC								 */
// 2010.10.18 管理No.675対応 Iwata End .
#endif

		pc_RdsSetting->setICStatBlack			( uc_warnData[RDSE_WarnTrgICLifeK]			 );								/* DCライフK								 */
		pc_RdsSetting->setICStatBlackDevC		( uc_warnData[RDSE_WarnTrgICLifeKDevC]		 );								/* DevCライフK								 */ 

		pc_RdsSetting->setTransBStat			( uc_warnData[RDSE_WarnTrgTransBeltLife]	 );								/* 転写ベルトユニットライフ					 */
		pc_RdsSetting->setOzonFStat				( uc_warnData[RDSE_WarnTrgOzonFilterLife]	 );								/* 紙粉除去クリーナ/オゾンフィルタライフ	 */
		pc_RdsSetting->setTransRStat			( uc_warnData[RDSE_WarnTrgTransRollerLife]	 );								/* 転写ローラユニットライフ					 */
		pc_RdsSetting->setIRBrightnessStat		( uc_warnData[RDSE_WarnTrgCCDGeinStat]		 );								/* ＣＣＤクランプゲイン調整異常				 */
		pc_RdsSetting->setPMStat				( (char)((TYPE_RdsWarnStatNormal == uc_warnData[RDSE_WarnTrgPM]) ? False : True) );	/* PMカウンタ警告					 */	// 2010/07/17 sone Minerva warning 対応 charにキャスト.

		pc_RdsSetting->setRdsFuserTempStat		( uc_warnData[RDSE_WarnTrgFuserTempTrouble]	 );								/* 定着センサ温度検出不良警告				 */
		pc_RdsSetting->setRdsMCBJStat			( uc_warnData[RDSE_WarnTrgMCBJFrequecy]		 );								/* 用紙系JAM多発警告						 */
		pc_RdsSetting->setRdsMOBJStat			( uc_warnData[RDSE_WarnTrgMOBJFrequecy]		 );								/* 原稿系JAM多発警告						 */
// Minerva UV-00929 kita 2010/10/20 Start.
		pc_RdsSetting->setRdsLicenseManagementErrorStat( uc_warnData[RDSE_WarnTrgLicenseManagementError] );					/* ライセンス管理異常警告					 */
// Minerva UV-00929 kita 2010/10/20 End.

// 2019/01/22 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS_1909】 Modification Start
		//EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)uc_warnData[RDSE_WarnTrgRetryFrequency], (uchar*)"##setRdsRetryWarningCounterState uc_warnData[RDSE_WarnTrgRetryFrequency]");
		setRdsRetryWarningCounterState(uc_warnData[RDSE_WarnTrgRetryFrequency]);
// 2019/01/22 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS_1909】 Modification End
	}


	/* 粉系警告解除受付時刻 */
	if(    (E_WarnTrgTime == e_Type)	//警告解除受付時刻を保存する場合、
		|| (E_WarnALL == e_Type)   )	//もしくは警告関連すべてを保存する場合。
	{
		// BCD8桁 mm,dd,hh,MM（4byte）：デフォルト値：ALL0
		uchar	uc_BCD[12];

		//概要　　：警告発生時刻(トナーエンプティ[K色])  
		c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyK].cnvTime_2_String( uc_BCD, RDSE_T2S_BCDmmddhhmm );
		pc_RdsSetting->setTonerEmptyTimeK( uc_BCD );

		//概要　　：警告発生時刻(トナーエンプティ[C色]) 
		c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyC].cnvTime_2_String( uc_BCD, RDSE_T2S_BCDmmddhhmm );
		pc_RdsSetting->setTonerEmptyTimeC( uc_BCD );

		//概要　　：警告発生時刻(トナーエンプティ[M色]) 
		c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyM].cnvTime_2_String( uc_BCD, RDSE_T2S_BCDmmddhhmm );
		pc_RdsSetting->setTonerEmptyTimeM( uc_BCD );
		
		//概要　　：警告発生時刻(トナーエンプティ[Y色]) 
		c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyY].cnvTime_2_String( uc_BCD, RDSE_T2S_BCDmmddhhmm );
		pc_RdsSetting->setTonerEmptyTimeY( uc_BCD );

		//概要　　：廃トナーボトルFULL
		c_RecoverWarnTrgTime[RDSE_WarnTrgBottleFull].cnvTime_2_String( uc_BCD, RDSE_T2S_BCDmmddhhmm );
		pc_RdsSetting->setUseTonerEmptyTime( uc_BCD );
	}
}
/*==================================================================================================*/
//	CommonAPI のデータを内部変数にロード
/*--------------------------------------------------------------------------------------------------*/
// 04/01/06 浦崎  警告解除受付時間追加。
// 04/03/15 浦崎  電源OFF→ON時に警告状態が変化していた時、その内容を正しく不揮発にセットできない問題が
//                あったため修正。
//                → 不揮発からリード、ライトポインタを読み出した時、リングバッファ制御クラスの配列の
//                   インデックスを直接書き換えてしまっていたため、要素を差すポインタがインデックスと
//                   等しい位置になっていなかった。
/*==================================================================================================*/
void RDSC_WarnData::__loadCommonAPI()
{
	RDSS_WarnData*	p_WarnData;
	uchar			tmpWarnData[160];
	uchar			tmpDetailPart[64+10];		// 04/04/09 浦崎 
	uchar*			pDetailPart;
	uchar*			pSrc;

ushort us_DetailPartIndx = 0;
ushort us_AddIndx = 0;
uchar  uc_bitInfo = 7;

	EPN_DBG_MESSAGE( (uchar*)"RDSC_WarnData", (uint32_t)0,  (uchar*)" __loadCommonAPI()" );

	/* 警告バッファ（160byte） */
	pSrc = tmpWarnData;
	pDetailPart = tmpDetailPart;
	pc_RdsSetting->getWarnEvent( tmpWarnData );
	pc_RdsSetting->getWarnEVDetailData( tmpDetailPart ); 
	// 04/04/09 浦崎 ループ最大値を RDSE_WarnTrg_NumOfTrg から _S_WarnData に修正。 
	//               RDSE_WarnTrg_NumOfTrg を増やそうとしたところ、ここで使用している 
	//               定数にミスがある事に気付いた。
	//               ※ 双方ともサイズは等しかったため、問題はなかった。 
	for( int indx = 0; indx < _S_WarnData; indx++ )
	{
		p_WarnData = &s_Data[indx];

		p_WarnData->uc_Map   = *pSrc;		// MAP(0,1,2) （1byte）
		pSrc++;
		p_WarnData->uc_Level = *pSrc;		// 警告レベル（TYPE_RdsWarnStat）（1byte）
		pSrc++;
		p_WarnData->uc_Kind  = *pSrc;		// 警告種類（1byte）
		pSrc++;
		//発生時間（イベントIDとして使用される）（6byte）
		pSrc += p_WarnData->c_Time.cnvString_2_Time( pSrc, RDSE_T2S_BCDyymmddhhmmss );
		// Detail部分 04/04/09 浦崎 
#if 1
		us_AddIndx = BitData_2_Array( &pDetailPart[us_DetailPartIndx], p_WarnData->uc_Detail, &uc_bitInfo );
		us_DetailPartIndx = (ushort)(us_DetailPartIndx + us_AddIndx);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.
#else
		memcpy( p_WarnData->uc_Detail, pDetailPart, 4 );
		pDetailPart += 4;
#endif
	}

	ushort us_tmpRdPtr = pc_RdsSetting->getWarnRdPtr();		// 04/03/15 浦崎  
	setRdIndx( us_tmpRdPtr );
	ushort us_tmpWtPtr = pc_RdsSetting->getWarnWtPtr();		// 04/03/15 浦崎 
	setWtIndx( us_tmpWtPtr );


	/* 警告イベント */
	uc_warnData[ RDSE_WarnTrgTonerEmptyY		] = pc_RdsSetting->getTonerStatYellow();													/* Yトナーエンプティ						 */
	uc_warnData[ RDSE_WarnTrgTonerEmptyM		] = pc_RdsSetting->getTonerStatMagenta();													/* Mトナーエンプティ						 */
	uc_warnData[ RDSE_WarnTrgTonerEmptyC		] = pc_RdsSetting->getTonerStatCyan();														/* Cトナーエンプティ						 */
	uc_warnData[ RDSE_WarnTrgTonerEmptyK		] = pc_RdsSetting->getTonerStatBlack();														/* Kトナーエンプティ						 */
	uc_warnData[ RDSE_WarnTrgBottleFull			] = pc_RdsSetting->getUseTonerStat();														/* 廃トナーボトルフル						 */
	uc_warnData[ RDSE_WarnTrgFuserUnitLife		] = pc_RdsSetting->getFuserStat();															/* 定着ユニットライフ						 */
	uc_warnData[ RDSE_WarnTrgTonerFilter		] = pc_RdsSetting->getRdsTonerFilterStat();													/* トナーフィルタ警告						 */

// 2010.10.18 管理No.675対応 Iwata Start .
#if !defined(DEF_A4_Cus) && !defined(DEF_HeliosMLKCus)		// 2019.10.09 【HeliosMLK】機器構成変更_No.9 ht_kurihara.
	uc_warnData[ RDSE_WarnTrgICLifeCDC			] = pc_RdsSetting->getICStatCyan();															/* DCライフC								 */
	uc_warnData[ RDSE_WarnTrgICLifeMDC			] = pc_RdsSetting->getICStatMagenta();														/* DCライフM								 */
	uc_warnData[ RDSE_WarnTrgICLifeYDC			] = pc_RdsSetting->getICStatYellow();														/* DCライフY								 */

	uc_warnData[ RDSE_WarnTrgICLifeCDevC		] = pc_RdsSetting->getStatCyanDevC();														/* DevCライフC								 */
	uc_warnData[ RDSE_WarnTrgICLifeMDevC		] = pc_RdsSetting->getStatMagentaDevC();													/* DevCライフM								 */
	uc_warnData[ RDSE_WarnTrgICLifeYDevC		] = pc_RdsSetting->getStatYellowDevC();														/* DevCライフY								 */

#else
		//デフォルトではMinerva用の設定を記載する。
	uc_warnData[ RDSE_WarnTrgICLifeC			] = pc_RdsSetting->getICStatCyan();															/* IUライフC								 */
	uc_warnData[ RDSE_WarnTrgICLifeM			] = pc_RdsSetting->getICStatMagenta();														/* IUライフM								 */
	uc_warnData[ RDSE_WarnTrgICLifeY			] = pc_RdsSetting->getICStatYellow();														/* IUライフY								 */
// 2010.10.18 管理No.675対応 Iwata End .
#endif

	uc_warnData[ RDSE_WarnTrgICLifeK			] = pc_RdsSetting->getICStatBlack();														/* DCライフK								 */
	uc_warnData[ RDSE_WarnTrgICLifeKDevC		] = pc_RdsSetting->getICStatBlackDevC();													/* DevCライフK								 */

	uc_warnData[ RDSE_WarnTrgTransBeltLife		] = pc_RdsSetting->getTransBStat();															/* 転写ベルトユニットライフ					 */
	uc_warnData[ RDSE_WarnTrgOzonFilterLife		] = pc_RdsSetting->getOzonFStat();															/* 紙粉除去クリーナ/オゾンフィルタライフ	 */
	uc_warnData[ RDSE_WarnTrgTransRollerLife	] = pc_RdsSetting->getTransRStat();															/* 転写ローラユニットライフ					 */
	uc_warnData[ RDSE_WarnTrgCCDGeinStat		] = pc_RdsSetting->getIRBrightnessStat();													/* CCDクランプゲイン調整異常				 */
	uc_warnData[ RDSE_WarnTrgPM					] = (TYPE_RdsWarnStat)((False == pc_RdsSetting->getPMStat()) ? TYPE_RdsWarnStatNormal : TYPE_RdsWarnStatErr);	/* PMカウンタ警告							 */
	uc_warnData[ RDSE_WarnTrgFuserTempTrouble	] = pc_RdsSetting->getRdsFuserTempStat();													/* 定着センサ温度検出不良					 */
	uc_warnData[ RDSE_WarnTrgMCBJFrequecy		] = pc_RdsSetting->getRdsMCBJStat();														/* 用紙系JAM多発警告						 */
	uc_warnData[ RDSE_WarnTrgMOBJFrequecy		] = pc_RdsSetting->getRdsMOBJStat();														/* 原稿系JAM多発警告						 */
// Minerva UV-00929 kita 2010/10/20 Start.
	uc_warnData[ RDSE_WarnTrgLicenseManagementError ] = pc_RdsSetting->getRdsLicenseManagementErrorStat();									/* ライセンス管理異常警告					 */
// Minerva UV-00929 kita 2010/10/20 End.

	uc_warnData[RDSE_WarnTrgRetryFrequency		] = getRdsRetryWarningCounterState();		// 2019/01/22 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS_1909】 Modification Added
	/* 粉系警告解除受付時刻 */
	// BCD8桁 mm,dd,hh,MM（4byte）：デフォルト値：ALL0
	uchar	uc_BCD[12];
	RDSC_Time			c_TempNowTime;

	c_TempNowTime = theVRmsc.getNowTime();				// 現在時刻の取得 


	//概要　　：警告発生時刻(トナーエンプティ[K色])  
	pc_RdsSetting->getTonerEmptyTimeK( uc_BCD );
	c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyK].cnvString_2_Time( uc_BCD, RDSE_T2S_BCDmmddhhmm );
	if( c_TempNowTime.month < c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyK].month )
	{
		c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyK].year = (ushort)(c_TempNowTime.year - 1);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.
	}
	else
	{
		c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyK].year = c_TempNowTime.year;
	}

	//概要　　：警告発生時刻(トナーエンプティ[C色]) 
	pc_RdsSetting->getTonerEmptyTimeC( uc_BCD );
	c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyC].cnvString_2_Time( uc_BCD, RDSE_T2S_BCDmmddhhmm );
	if( c_TempNowTime.month < c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyC].month )
	{
		c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyC].year = (ushort)(c_TempNowTime.year - 1);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.
	}
	else
	{
		c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyC].year = c_TempNowTime.year;
	}

	//概要　　：警告発生時刻(トナーエンプティ[M色]) 
	pc_RdsSetting->getTonerEmptyTimeM( uc_BCD );
	c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyM].cnvString_2_Time( uc_BCD, RDSE_T2S_BCDmmddhhmm );
	if( c_TempNowTime.month < c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyM].month )
	{
		c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyM].year = (ushort)(c_TempNowTime.year - 1);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.
	}
	else
	{
		c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyM].year = c_TempNowTime.year;
	}

	
	//概要　　：警告発生時刻(トナーエンプティ[Y色]) 
	pc_RdsSetting->getTonerEmptyTimeY( uc_BCD );
	c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyY].cnvString_2_Time( uc_BCD, RDSE_T2S_BCDmmddhhmm );
	if( c_TempNowTime.month < c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyY].month )
	{
		c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyY].year = (ushort)(c_TempNowTime.year - 1);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.
	}
	else
	{
		c_RecoverWarnTrgTime[RDSE_WarnTrgTonerEmptyY].year = c_TempNowTime.year;
	}


	//概要　　：廃トナーボトルFULL
	pc_RdsSetting->getUseTonerEmptyTime( uc_BCD );
	c_RecoverWarnTrgTime[RDSE_WarnTrgBottleFull].cnvString_2_Time( uc_BCD, RDSE_T2S_BCDmmddhhmm );
	if( c_TempNowTime.month < c_RecoverWarnTrgTime[RDSE_WarnTrgBottleFull].month )
	{
		c_RecoverWarnTrgTime[RDSE_WarnTrgBottleFull].year = (ushort)(c_TempNowTime.year - 1);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.
	}
	else
	{
		c_RecoverWarnTrgTime[RDSE_WarnTrgBottleFull].year = c_TempNowTime.year;
	}

// 2018/12/03 FPT-DucDH4 【IT6_2.0】【EagleH】【機器構成変更_No.1】 リトライ多発警告 Start.
//	uc_warnData[RDSE_WarnTrgRetryFrequency] = (TYPE_RdsWarnStat)((RDSD_THRESHOLD_DEFAULT < pc_RdsSetting->getResTimeOutThreshold()) ? TYPE_RdsWarnStatNormal : TYPE_RdsWarnStatErr);	// 2019/01/07 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-1916】 Remove assign warnData
// 2018/12/03 FPT-DucDH4 【IT6_2.0】【EagleH】【機器構成変更_No.1】 リトライ多発警告 End.
}



/***************************************************************************************************
 *   KMRDS送信テキストバッファ
 ***************************************************************************************************/
/*==================================================================================================*/
//	コンストラクタ
/*==================================================================================================*/
RDSC_EventData::RDSC_EventData()
{
	registRing( &txText[0], _S_EventDataNum );		/* リングバッファ登録 */
}

/*==================================================================================================*/
//	バッファのクリア
/*==================================================================================================*/
void RDSC_EventData::clearCurrentBuff( void )
{
	memset( &wt->uc_Buff[0], 0, _S_EventDataBuff );
	wt->us_Len   = 0;
	wt->e_Kind   = TYPE_RdsTrgKindUnKnowin;
}
//==================================================================================================.
//	テキストファイルの取得用関数.
//==================================================================================================.
void RDSC_EventData::getMakeTextData( short ss_Num, char* sc_Data )
{
	memcpy( sc_Data, &txText[ss_Num].uc_Buff[0], _S_EventDataBuff );
}


/***************************************************************************************************
 *   テキスト作成処理クラス：初期発信
 ***************************************************************************************************/
/*==================================================================================================*/
//	イベントデータ作成関数「初期発信」
/*--------------------------------------------------------------------------------------------------*/
// 07/04/16 佐久間 [Mosel_Vista]フレキシブルな初期発信対応 
/*==================================================================================================*/
void RDSC_mkText_ModeIni::makeData( uint32_t para )
{
	RDSS_EventData*			pDst;
	RDSC_Time				c_rdsTime;
	register uchar*			pBuff;
//	uchar					uc_Temp[_S_Tel+1];		// フレキシブルな初期発信 
	uchar					uc_Temp[_S_Adr+1];		// フレキシブルな初期発信 
	ushort					us_TempLen;

	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];											// イベントデータバッファへのポインタを取得

	c_EvevntDataBuf.clearCurrentBuff();									/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */


	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）
	pDst->e_Kind = e_txtKind;

	if(    RDSE_TSK_ID_TELL == theRdsApl.e_CommDevice
		|| RDSE_TSK_ID_FAX == theRdsApl.e_CommDevice    )	//【CSRC FAX対応】05/09/29 浦崎 
	{

		// ◆ モデム版

		//-------------------------------------------------------------
		// "\r\n5400 A100yymmddhhmmww B000######## I102##……##\r\n" 
		//-------------------------------------------------------------

		// ■ 日時：A100yymmddhhmmww 
		memcpy( pBuff, "A100", 4 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

		c_rdsTime = theVRmsc.getNowTime();
		us_TempLen = c_rdsTime.cnvTime_2_String( pBuff, RDSE_T2S_yymmddhhmmss );
		_INC_SIZE( &pBuff, &pDst->us_Len, us_TempLen );
		memcpy( pBuff, " ", 1 );											// スペース
		_INC_SIZE( &pBuff, &pDst->us_Len, 1 );


		// ■トータルカウンタ：B000######## 
		memcpy( pBuff, "B000", 4 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

		APIC_TotalCnt*	p = APIC_TotalCnt::newInstance();

		// トータルカウンタ(コピー/プリント合計、ブラック/フルカラー/モノカラー／２色合計)
		// 05/06/30 佐久間 問題点No.592295カウンタ不一致問題対応 
		//uint32_t ul_Data =   p->get( TYPE_TotalCntCopyTotalFullColor )			// コピー　・トータル・フルカラー
		//				+ p->get( TYPE_TotalCntCopyTotalMonoColor )			// コピー　・トータル・モノカラー
		//				+ p->get( TYPE_TotalCntCopyTotalBlack )				// コピー　・トータル・ブラック
		//				+ p->get( TYPE_TotalCntCopyTotal2Color )			// コピー　・トータル・２色
		//				+ p->get( TYPE_TotalCntPrintTotalFullColor )		// プリント・トータル・フルカラー
		//				+ p->get( TYPE_TotalCntPrintTotalBlack )			// プリント・トータル・ブラック
		//				+ p->get( TYPE_TotalCntPrintTotal2Color );			// プリント・トータル・２色
		uint32_t ul_Data = p->get( TYPE_TotalCntTotal );		// トータルカウンタ取得 

		EpLib_ltoa( ul_Data, pBuff, 8 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 8 );

		// スペース付加
		*pBuff = ' ';
		_INC_SIZE( &pBuff, &pDst->us_Len, 1 );

		// ■ 電話番号：I102##……##
		memcpy( pBuff, "I102", 4 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

		pc_RdsSetting->getMachineDial( uc_Temp );
		us_TempLen = (ushort)(strlen( (char*)uc_Temp ));	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

		memcpy( pBuff, uc_Temp, us_TempLen );
		_INC_SIZE( &pBuff, &pDst->us_Len, us_TempLen );
		
		// CQNo366843 S9コマンドを付加する sone 2012/07/03 START.
		// モデム通信の場合、S9コマンドの処理をここで付加する.(FAX通信の場合はFAX通信処理部で付加する).
		if( RDSE_TSK_ID_TELL == theRdsApl.e_CommDevice )
		{
			int32_t	ul_datasize = 0;					// 書き込んだデータサイズ.
			unsigned char	uc_Commandbuf[] = {" S9"};	// Readしたいコマンド.
			unsigned char	uc_Data[1000] = {""};		// 書き込んだデータ.

			// クラスの定義(コンストラクタ用の引数はモデム用のマトリクス).
			RDSC_Inquiry mInquiry(RDSC_CommandMatrix::GetCommandMatrixForTelephony());
			mInquiry.Init( uc_Commandbuf, (int32_t)strlen((char*)uc_Commandbuf) );
			mInquiry.InquiryNonVolatile( &uc_Data[0], (int32_t)500, &(ul_datasize) );
			
			memcpy( pBuff, uc_Data, ul_datasize );
			_INC_SIZE( &pBuff, &pDst->us_Len, static_cast<ushort>(ul_datasize) );		
		}
		// CQNo366843 S9コマンドを付加する sone 2012/07/03 END.
// 2017/07/20 [MAv5][RQ-XXX] CSRA設定状態発信 Start.
		memcpy( pBuff, " RV00", 5 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 5 );
// 2020/09/25 RQ-776 CWH 1-way通信方式の追加 start.
		TYPE_CSRASettingComMode e_CSRACommMode = APIC_CSRASetting::newInstance()->getCSRASettingComMode();
//		TYPS_CSRASetting s_CSRASetting = {};
//		APIC_CSRASetting::newInstance()->getCSRASetting( &s_CSRASetting );
//		if ( True == s_CSRASetting.m_b_Enable ) {
		if ( TYPE_CSRASettingComMode_TwoWay == e_CSRACommMode ) {
// 2020/09/25 RQ-776 CWH 1-way通信方式の追加 end.
			memcpy( pBuff, "01", 2 );
// 2020/09/25 RQ-776 CWH 1-way通信方式の追加 start.
		} else if( TYPE_CSRASettingComMode_OneWay == e_CSRACommMode ) {
			memcpy( pBuff, "02", 2 );
//2021/10/14 FPT-TinhNT16 【IT6-2.2MR1】 【制御構成変更_No.38】 Start.
		} else if( TYPE_CSRASettingComMode_OneWay_CN == e_CSRACommMode ) {
			memcpy( pBuff, "03", 2 );
//2021/10/14 FPT-TinhNT16 【IT6-2.2MR1】 【制御構成変更_No.38】 End.
		}
// 2020/09/25 RQ-776 CWH 1-way通信方式の追加 end.
		else {
			memcpy( pBuff, "00", 2 );
		}
		_INC_SIZE( &pBuff, &pDst->us_Len, 2 );
// 2017/07/20 [MAv5][RQ-XXX] CSRA設定状態発信 End.
	}
	else
	{
// 2016/03/23 ktakahashi [ZeusS_ZX0]CQ526327_E-mail自動初期発信ありでCSRC設定のインポート・エクスポート後にボタンクリック不可 Start.
		// 初期発信のセンターへの送信時間をEventIDに変換.
		uchar uc_EventID[RDSD_EVENTID_SIZE+1] = {0};
		theMakeText.getTrgEventID( uc_EventID );	// 保持していたEventIDを取得する.
		// 保持していた初期発信のセンターへの送信時間を、初期発信の「トリガ時間」として再設定する.
		pc_RdsSetting->setTrgTime( e_txtKind, uc_EventID );
// 2016/03/23 ktakahashi [ZeusS_ZX0]CQ526327_E-mail自動初期発信ありでCSRC設定のインポート・エクスポート後にボタンクリック不可 End.
		// ◆ E-mail版 では「初期発信」は無い。 
		// ◆ 片方向通信の場合はある。 06/11/30 佐久間 
			ushort us_Len = c_mkText_ModeResRDS.make_US( pBuff );
			_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |=  0x00000001;	*/			// 発着信種別 初期発信bitON  2014/02/03 add by tomiyasu
}
/*==================================================================================================*/
//	発信トリガ処理「初期発信」
/*--------------------------------------------------------------------------------------------------*/
// 04/08/09 浦崎  トリガ受信時に「ボタンON状態」をセットするようにした。 
// 06/11/30 佐久間　片方向通信対応 
// 07/04/16 佐久間 [Mosel_Vista]フレキシブルな初期発信対応 
/*==================================================================================================*/
void RDSC_mkText_ModeIni::rcvTrg( uint32_t para )
{
//	if(    RDSE_TSK_ID_TELL == theRdsApl.isRdsSystem()
//		|| RDSE_TSK_ID_FAX == theRdsApl.isRdsSystem()    )	//【CSRC FAX対応】05/09/29 浦崎 
//	{
		uchar	uc_Temp[_S_CenterID+1];
		pc_RdsSetting->getHostID( uc_Temp );
		if(    (TYPE_HostStatClear == pc_RdsSetting->getHostStat())
			&& (_S_CenterID == strlen( (char*)uc_Temp )) )
		{
			EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"初期発信トリガ受信" );
			tskRdsEvent.c_ButtonSTS.rcvTrg( e_txtKind );	// 04/08/09 浦崎 
			cmnTrgAction();
		}
//	}
}
#if 1		// フレキシブルな初期発信【応答の確認処理】 

/*==================================================================================================*/
//	ID の等しいデータがあればクリアする(応答の確認)
/*==================================================================================================*/
Bool RDSC_mkText_ModeIni::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return( False );
	}

	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)" 初期発信の応答【受信】" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す。 
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		// ID も消す。 
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
		return( True );
	}
	
	return( False );
}
#endif





/***************************************************************************************************
 *   テキスト作成処理クラス：緊急発信
 ***************************************************************************************************/
/*==================================================================================================*/
//	イベントデータ作成関数「緊急発信」
/*--------------------------------------------------------------------------------------------------*/
// 04/03/01 浦崎  問題点xxx対応。トラブルコードを２つ送信する処理をコーディング。 
// 04/03/03 浦崎  2439対策。ライトポインタ更新後、pDst と、pBuff を更新していなかったため、
//                更新前のライトポインタ位置にデータを書き込んでしまっていた。
/*==================================================================================================*/
void RDSC_mkText_ModeSqrmbl::makeData( uint32_t para )
{
	RDSS_EventData*			pDst;
	register uchar*			pBuff;
	ushort					us_Code;

// 2010/09/06 Minerva CQ332769 UV-00134追加対応 kita Start.
	// 緊急発信未送信且つ再起動後、トラブル発生中でセンターとの接続が回復していた場合の考慮.
	// インスタンスの取得.
	APIC_MachineSetting* pc_MachineSetting = APIC_MachineSetting::newInstance();
	
	if( TYPE_FatalCancelEnabledOk == pc_MachineSetting->getFatalCancelEnabled() )
	{
		// トラブル解除可能状態をNGへ変更する.
		pc_MachineSetting->lock();		// ロック.
		pc_MachineSetting->setFatalCancelEnabled( TYPE_FatalCancelEnabledNg );
		pc_MachineSetting->unlock();	// ロック解除.
		
		// 自動FATAL解除用の緊急発信状態変更：送信していない状態→送信中の状態.
		tskRdsEvent.changeATRTransStatus(
				RSDE_AuteTrblReset_TransSt_Non,
				RSDE_AuteTrblReset_TransSt_Sending
		);
	}
// 2010/09/06 Minerva CQ332769 UV-00134追加対応 kita End.

	// ■ モデム、FAX、E-mail 共通 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	for( int indx=0; indx<2; indx++ )
	{
		// トラブルコード：ushort データをASCII(4桁)にしてセット 
		// コードは２つまで記憶している。 
		Bool b_TransEnd = pc_RdsSetting->getTrblNotRecverFlag( indx );							// 送信済み？ 
// 2017/08/07 MAv5 CSRA接続 start.
//		TYPE_RdsMode		e_RdsMode		= pc_RdsSetting->getRdsMode();
//		TYPE_RdsMode		e_RdsMode		= theRdsApl.getRdsModeEx();         //[AR : 1029580] 【 Eagle 】【Build error fix】 2018/03/09 [HCL_Raghu]
// 2017/08/07 MAv5 CSRA接続 end.		
// 管理システム統合化開発 RQ-893(OP_BTS-32474) 2021/04/20 Sky梶谷 START.
//		us_Code = getCode( (ushort)indx );	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.
//		if(    Trb_EMP != us_Code																// トラブルコードが空では無い事が大前提。 
//			&& (    (TYPE_RdsModeMail == pc_RdsSetting->getRdsMode() && False == isIDCleared( (ushort)indx ))		// E-mail版は常に許可。  2010/07/17 sone Minerva warning 対応 ushortにキャスト.
//			     || (TYPE_RdsModeMail != pc_RdsSetting->getRdsMode() && False == b_TransEnd)  )				// E-mail版以外は、通知済みトラブルは通知しない。
//		   )
		Bool b_SendRequired = False;
		if ( True == RDSC_RptDataManager::newInstance()->isAvailableSendReport() ) {							// レポート発信が有効な場合.
			if ( False == b_TransEnd ) {																		// 通知済みトラブルは通知しない.
				b_SendRequired = True;
			}
		}
		else {
			if ( ( TYPE_RdsModeMail == pc_RdsSetting->getRdsMode() && False == isIDCleared( (ushort)indx ) )	// E-mail版は常に許可。
			||   ( TYPE_RdsModeMail != pc_RdsSetting->getRdsMode() && False == b_TransEnd)                 ) {	// E-mail版以外は、通知済みトラブルは通知しない。
				b_SendRequired = True;
			}
		}
		us_Code = getCode( (ushort)indx );
		if ( Trb_EMP != us_Code	&& True == b_SendRequired )		// トラブルコードが空では無く、送信が必要なとき. 
// 管理システム統合化開発 RQ-893(OP_BTS-32474) 2021/04/20 Sky梶谷 END.
		{
			pDst  = c_EvevntDataBuf.wt;
			pBuff = &pDst->uc_Buff[0];											// イベントデータバッファへのポインタを取得

			c_EvevntDataBuf.clearCurrentBuff();									/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */


			// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）
			pDst->e_Kind = e_txtKind;

			// ■ E-mail 固有 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
			if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
			 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )		// E-mail版 / HTTP版（08/01/31 寺内).
			 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
			{
				// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。 
				uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
				//pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindTrbl, uc_BcdEventID );
				// ↑ これは×。↓にて、バックアップトラブブコード０、１ごとの発生時刻を取得する。  
				if( 0 == indx )
				{
					pc_RdsSetting->getResIDTrbl0( uc_BcdEventID );
				}
				else
				{
					pc_RdsSetting->getResIDTrbl1( uc_BcdEventID );
				}

				ushort us_Len = RDSC_MakeEventData::makeM501
					(
						pBuff,				// ユーザーデータバッファ 
						uc_BcdEventID,		// イベントID 
						(uchar*)"EC"		// 発信固有文字列 
					);
				_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
			}


			//  SC00#### 
			memcpy( pBuff, "SC00", 4 );
			_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

			sprintf( (char*)pBuff, "%04x", us_Code );
			_INC_SIZE( &pBuff, &pDst->us_Len, 4 );
			EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)us_Code, (uchar*)"SC00 作成" );



			/* ライトポインタ更新 */
			/* 04/03/03 浦崎  2439対策。ライトポインタ更新後、pDst と、pBuff を更新していなかったため、 */
			/*                更新前のライトポインタ位置にデータを書き込んでしまっていた。 */
			c_EvevntDataBuf.updateWtPtr();
/*			RDS_modem_Kind |=  0x00000002;	*/			// 発着信種別 緊急発信bitON  2014/02/03 add by tomiyasu
		}
		//else
		//{
		//	break;
		//}
	}
}
/*==================================================================================================*/
//	発信トリガ処理「緊急発信」
/*--------------------------------------------------------------------------------------------------*/
// 04/03/12 浦崎 
/*==================================================================================================*/
void RDSC_mkText_ModeSqrmbl::rcvTrg( uint32_t para )
{
	if( False == pc_RdsSetting->getMaintStat() )					/* メンテ中は緊急発信のトリガは完全に無視する */
	{
		RDSC_Time	c_rdsTime = theVRmsc.getNowTime();

#if 1 // 緊急発信トリガ時間テスト 
		printEpTime( (RDSS_DetailTime)c_rdsTime,			(uchar*)"緊急発信トリガ受け付けた時の時間" );
#endif
		
		
		if( True == occurrTrbl( (ushort)para, c_rdsTime ) )			/* バッファに空きがあって、正しく保存出来た場合 */
		{
			EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"緊急発信トリガ受信" );
			cmnTrgAction( &c_rdsTime );
		}
	}
}
/*==================================================================================================*/
//	ID の等しいデータがあればクリアする(応答の確認) 
/*--------------------------------------------------------------------------------------------------*/
// 04/06/14 浦崎 個々のイベントに対して応答を確認するようにした。 
/*==================================================================================================*/
void RDSC_mkText_ModeSqrmbl::chkAns( uchar* pData, uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}
	// 緊急発信トリガ時間テスト 
	RDSC_Time				c_rdsTime;
	c_rdsTime.cnvString_2_Time( bcd_rcvEventID, RDSE_T2S_BCDyymmddhhmmss );
	printEpTime( (RDSS_DetailTime)c_rdsTime,			(uchar*)"緊急発信の応答(  受   信  )" );

	//pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );
	// トラブルコードを２個バックアップしていたとしても、 
	// 最新のものに対する応答のみを確認できれば良い。 

	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	ushort us_bkTrblCode;
	ushort us_RcvTrblCode;
	uint32_t  ul_tempData=0;
//AR_1021298 mod Start
//	sscanf( (char*)pData, "%04lx", &ul_tempData );
	sscanf( (char*)pData, "%04x", &ul_tempData );
//AR_1021298 mod End
	us_RcvTrblCode = (ushort)ul_tempData;
	EPN_DBG_MESSAGE( (uchar*)"CSRC", us_RcvTrblCode, (uchar*)" 受信コード" );
	for( uchar indx=0; indx<2; indx++ )
	{
		if( 0 == indx )
		{
			pc_RdsSetting->getResIDTrbl0( bcd_bkEventID );
			c_rdsTime.cnvString_2_Time( bcd_bkEventID, RDSE_T2S_BCDyymmddhhmmss );
		}
		else
		{
			pc_RdsSetting->getResIDTrbl1( bcd_bkEventID );
			c_rdsTime.cnvString_2_Time( bcd_bkEventID, RDSE_T2S_BCDyymmddhhmmss );
		}
		printEpTime( (RDSS_DetailTime)c_rdsTime,			(uchar*)"緊急発信の応答(バックアップ)" );

		us_bkTrblCode = pc_RdsSetting->getTrblCode( indx );
		EPN_DBG_MESSAGE( (uchar*)"CSRC", us_bkTrblCode, (uchar*)" バックアップコード" );
		if( Trb_EMP != us_bkTrblCode )
		{
			if(    True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 )
				&& us_RcvTrblCode == us_bkTrblCode    )
			{
				// ID を消す。 
				memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
				if( 0 == indx )
				{
					pc_RdsSetting->setResIDTrbl0( bcd_bkEventID );
				}
				else
				{
					pc_RdsSetting->setResIDTrbl1( bcd_bkEventID );
				}
			}
		}
	}


	uchar uc_BcdEventID_0[6+1];
	uchar uc_BcdEventID_1[6+1];
	uchar bcd_EventID_ZeroString[6+1];
	pc_RdsSetting->getResIDTrbl0( uc_BcdEventID_0 );
	pc_RdsSetting->getResIDTrbl1( uc_BcdEventID_1 );
	memset( bcd_EventID_ZeroString, 0, sizeof(bcd_EventID_ZeroString) );

	if(    True == EpLib_datacmp( bcd_EventID_ZeroString, uc_BcdEventID_0, 6 )
		&& True == EpLib_datacmp( bcd_EventID_ZeroString, uc_BcdEventID_1, 6 )    )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)" 全ての緊急発信の応答受信" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す。 
		theTrgMap.resetMapByTrgAllPage( e_txtKind );	// 05/02/28 浦崎 resetMapByTrg( , 0 ) → resetMapByTrgAllPage() 

		// ID を消す。 
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
		pc_RdsSetting->setResIDTrbl0( bcd_bkEventID );
		pc_RdsSetting->setResIDTrbl1( bcd_bkEventID );

		// 下記は消してはダメ！！！
		// 緊急回復発信トリガ検出時に消す！ 
		// RDSC_TrblMode::recoverTrbl() 参照。
		// × pc_RdsSetting->setTrblCode( indx, Trb_EMP );
		// × pc_RdsSetting->setTrblNotRecverFlag( indx, False );
	}
}


/***************************************************************************************************
 *   テキスト作成処理クラス：緊急回復発信
 ***************************************************************************************************/
/*==================================================================================================*/
//	イベントデータ作成関数「緊急回復発信」
/*==================================================================================================*/
void RDSC_mkText_ModeSqrmblRecv::makeData( uint32_t para )
{
	RDSS_EventData*			pDst;
	register uchar*			pBuff;

	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];											// イベントデータバッファへのポインタを取得

	c_EvevntDataBuf.clearCurrentBuff();									/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */


	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）
	pDst->e_Kind = e_txtKind;


	// ■ E-mail 固有 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )		// E-mail版 / HTTP版　寺内(08/01/31) 新規追加.
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。 
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindTrblRecv, uc_BcdEventID );
		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ 
				uc_BcdEventID,		// イベントID 
				(uchar*)"TR"		// 発信固有文字列 
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}


	// ■ モデム、FAX、E-mail 共通 
	memcpy( pBuff, "SR000", 5 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 5 );


	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |=  0x00000004;	*/			// 発着信種別 緊急回復発信bitON  2014/02/03 add by tomiyasu
}
/*==================================================================================================*/
//	発信トリガ処理「緊急回復発信」
/*--------------------------------------------------------------------------------------------------*/
// 04/02/27 浦崎 問題点xxx対策。 
// 04/04/09 浦崎 問題点 4405 対策。スキャナ系トラブルも解除されていることを条件に加えた。 
// 05/05/23 浦崎 recoverTrbl() により緊急回復発信が正式に取り込まれた時に False にする。 
/*==================================================================================================*/
void RDSC_mkText_ModeSqrmblRecv::rcvTrg( uint32_t para )
{
	/* トラブルコード未通知フラグを「クリア可能」にする（バックアップ） */
	APIC_PrinterWarning*	pc_PrintWarn = APIC_PrinterWarning::newInstance();		// プリンタ警告 
	APIC_ScannerWarning*	pc_ScanWarn = APIC_ScannerWarning::newInstance();		// スキャナ警告 

	if(    True != pc_PrintWarn->isOccurFatal()								/* トラブル中でない場合 */
		|| True != pc_ScanWarn->isOccurFatal()    )							/* トラブル中でない場合 */
	{
		if( True == c_mkText_ModeSqrmbl.isNotRecover() )					/* 緊急発信を受け付けて、まだ緊急回復発信を行っていない場合 */
		{
			if( True == c_mkText_ModeSqrmbl.recoverTrbl() )					/* トラブル解除時の処理 */
			{
				EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)pc_RdsSetting->getTrblMode(), (uchar*)"緊急回復発信トリガ受信" );
				pc_RdsSetting->setNeedTrblRecoverFlag( False );	// 05/05/23 浦崎 
				cmnTrgAction();
			}
		}
	}
}
/*==================================================================================================*/
//	ID の等しいデータがあればクリアする(応答の確認)
/*==================================================================================================*/
void RDSC_mkText_ModeSqrmblRecv::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	//if( True == EpLib_strcmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )	// 04/03/24 浦崎 BCD なので、0時、等のデータであると、0x00 であるため、それをNULLとみなすため、正しく比較が出来ていなかった。 
	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)" 緊急回復発信の応答受信" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す。 
		theTrgMap.resetMapByTrgAllPage( e_txtKind );	// 05/02/28 浦崎 resetMapByTrg( , 0 ) → resetMapByTrgAllPage() 
		// ID も消す。 
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}



/***************************************************************************************************
 *   テキスト作成処理クラス：本体警告発信
 ***************************************************************************************************/
/*==================================================================================================*/
//   任意パーツカウンタ警告発信のパラメータ及び、ユーザーデータを作成する。
/*--------------------------------------------------------------------------------------------------*/
// 04/09/10 浦崎 新規。   
/*==================================================================================================*/
ushort RDSC_mkText_ModeWarn::makePartsReplacingData( RDSS_WarnData* s_WarnInfo, uchar* pDst )
{
	APIC_OptionPartsCnt *pc_OptionPartsCnt = APIC_OptionPartsCnt::newInstance();		// 任意パーツカウンタのインスタンスを取得
	TYPS_OptionPartsCnt	s_Data;


	uint32_t	ul_Len = 0;
	Bool	b_Top = False;

	for( uint32_t indx=0; indx<30; indx++ )
	{
		if( TYPE_RdsWarnStatNormal != s_WarnInfo->uc_Kind )
		{
			if( s_WarnInfo->uc_Detail[indx/8] & (0x01<<(indx%8)) )
			{
				if( False == b_Top )
				{
					b_Top = True;
				}
				else
				{
					pDst[ul_Len] = ',';								// [,]をセットする。 
					ul_Len ++;
				}

				sprintf( (char*)&pDst[ul_Len], "%02lX", indx );		// パラメータをセットする。 
				ul_Len += 2;
				s_Data = pc_OptionPartsCnt->get( (TYPE_OptionPartsCnt)indx );
				sprintf( (char*)&pDst[ul_Len], "%08ld", s_Data.ul_Limit % 100000000 );
				ul_Len += 8;
			}
		}
		else
		{
			if( s_WarnInfo->uc_Detail[indx/8] & (0x01<<(indx%8)) )
			{
				if( False == b_Top )
				{
					b_Top = True;
				}
				else
				{
					pDst[ul_Len] = ',';								// [,]をセットする。 
					ul_Len ++;
				}

				sprintf( (char*)&pDst[ul_Len], "%02lX", indx );		// パラメータをセットする。 
				ul_Len += 2;
				memcpy( (char*)&pDst[ul_Len], (const char*)"00000000", 8 );
				ul_Len += 8;
			}
		}
	}

	return( (ushort)ul_Len );
}
// 2018/12/03 FPT-DucDH4 【IT6_2.0】【EagleH】【機器構成変更_No.1】 リトライ多発警告 Start
ushort RDSC_mkText_ModeWarn::makeRetryFrequencyData(RDSS_EventData * pDst, uchar * pBuffer)
{
	TYPS_PaperFeedRetryCnt s_PaperFeedRetryCnt;
// 2019/02/28 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-3432】 Modification Start
	// TYPS_AlarmStatusInfo s_AlarmStatusInfo;
	pc_RdsSetting->getPaperFeedRetryCnt(&s_PaperFeedRetryCnt);
	// pc_RdsSetting->getAlarmStatusInfo(&s_AlarmStatusInfo);
// 2019/01/07 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-1916】 Modification Start
	// sprintf((char*)pBuffer, "%02X%04d%02d%02d%02d%02d", 0x00, s_AlarmStatusInfo.c_PreviousDateChecking.ss_Year, s_AlarmStatusInfo.c_PreviousDateChecking.sc_Month,
		// s_AlarmStatusInfo.c_PreviousDateChecking.sc_Day, s_AlarmStatusInfo.c_PreviousDateChecking.sc_Hour, s_AlarmStatusInfo.c_PreviousDateChecking.sc_Minute);
// 2019/01/18 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-1908】 Modification Start
//	_INC_SIZE(&pBuffer, &pDst->us_Len, PAR_DATA_SIZE + 4);
	// _INC_SIZE(&pBuffer, &pDst->us_Len, PAR_DATA_SIZE + 3);
// 2019/01/18 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-1908】 Modification End

	// sprintf((char*)pBuffer, ",%02X%04d%02d%02d%02d%02d", 0x01, s_AlarmStatusInfo.c_PreviousDateWarning.ss_Year, s_AlarmStatusInfo.c_PreviousDateWarning.sc_Month,
		// s_AlarmStatusInfo.c_PreviousDateWarning.sc_Day, s_AlarmStatusInfo.c_PreviousDateWarning.sc_Hour, s_AlarmStatusInfo.c_PreviousDateWarning.sc_Minute);
	// _INC_SIZE(&pBuffer, &pDst->us_Len, PAR_DATA_SIZE + 4);
// 2019/01/07 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-1916】 Modification End
	// sprintf((char*)pBuffer, ",%02X%08ld", 0x02, (s_AlarmStatusInfo.b_WarningStatus == True)? 1: 2);
	// _INC_SIZE(&pBuffer, &pDst->us_Len, PAR_DATA_SIZE);
// 2019/02/28 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-3715】 Modification Start	
	uchar uc_DipSw30 = pc_RdsSetting->getDipSw(29);
	EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)uc_DipSw30, (uchar*)"##[00.6] makeRetryFrequencyData uc_DipSw30");

// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される Modify Start.
	uint32_t ui_PaperFeedRetry[6];
	char c_TmpParamNumber[3];
	char c_TmpPaperFeedRetryStr[9];

	bool b_LastPara;
	short s_Idx;
	short s_SubIdx;

	//Set default value variable local.
	memset(&ui_PaperFeedRetry[0],0x00, sizeof(ui_PaperFeedRetry));
	memset(&c_TmpParamNumber[0],0x00, sizeof(c_TmpParamNumber));
	memset(&c_TmpPaperFeedRetryStr[0], 0x00, sizeof(c_TmpPaperFeedRetryStr));
	b_LastPara = True;
	s_Idx = 0;
	s_SubIdx = 0;

	ui_PaperFeedRetry[0] = s_PaperFeedRetryCnt.ul_PaperFeedRetry1Cnt;
	ui_PaperFeedRetry[1] = s_PaperFeedRetryCnt.ul_PaperFeedRetry2Cnt;
	ui_PaperFeedRetry[2] = s_PaperFeedRetryCnt.ul_PaperFeedRetry3_LCTCnt;
	ui_PaperFeedRetry[3] = s_PaperFeedRetryCnt.ul_PaperFeedRetry4Cnt;
	ui_PaperFeedRetry[4] = s_PaperFeedRetryCnt.ul_PaperFeedRetryManualCnt;
	ui_PaperFeedRetry[5] = s_PaperFeedRetryCnt.ul_PaperFeedRetryExtLCTCnt;

	for(s_Idx = 0; s_Idx < 6; s_Idx ++){
		if(ui_PaperFeedRetry[s_Idx] >= (ushort)uc_DipSw30){
			
			//add param.
			snprintf(&c_TmpParamNumber[0], 3 , "%02X" , s_Idx);
			memcpy(pBuffer, &c_TmpParamNumber[0], sizeof(c_TmpParamNumber)-1 );
			_INC_SIZE(&pBuffer, &pDst->us_Len, 2);

			//add data.
			snprintf(&c_TmpPaperFeedRetryStr[0], 9 , "%08u", ui_PaperFeedRetry[s_Idx]);
			memcpy(pBuffer, &c_TmpPaperFeedRetryStr[0], sizeof(c_TmpPaperFeedRetryStr)-1 );
			_INC_SIZE(&pBuffer, &pDst->us_Len, 8);

			// From the current parameter number, scrutinize the array that stores the boolean value of whether or not it was exchanged.
			// If there are parts that have been replaced after the current parameter, add a comma.
			// If it is the last part, there is no question and answer and no comma is added.
			b_LastPara = True;
			
			if ( s_Idx < 6 )
			{
				for ( s_SubIdx = s_Idx + 1; s_SubIdx < 6 ; s_SubIdx++)
				{
					if ( ui_PaperFeedRetry[s_SubIdx] >= (ushort)uc_DipSw30)
					{
						b_LastPara = False;
						break;
					}
				}
			}
			
			//add a comma.
			if ( b_LastPara == False )
			{
				memcpy(pBuffer, ",", 1 );
				_INC_SIZE( &pBuffer, &pDst->us_Len, 1 );
			}

		}
	}

//	if (s_PaperFeedRetryCnt.ul_PaperFeedRetry1Cnt		>= (ushort)uc_DipSw30)
//	{
//		sprintf((char*)pBuffer, "%02X%08ld,", 0x00, s_PaperFeedRetryCnt.ul_PaperFeedRetry1Cnt);
//		_INC_SIZE(&pBuffer, &pDst->us_Len, PAR_DATA_SIZE);
//	}
		
//	if (s_PaperFeedRetryCnt.ul_PaperFeedRetry2Cnt		>= (ushort)uc_DipSw30)
//	{
//		sprintf((char*)pBuffer, "%02X%08ld,", 0x01, s_PaperFeedRetryCnt.ul_PaperFeedRetry2Cnt);
//		_INC_SIZE(&pBuffer, &pDst->us_Len, PAR_DATA_SIZE);
//	}
		
//	if (s_PaperFeedRetryCnt.ul_PaperFeedRetry3_LCTCnt	>= (ushort)uc_DipSw30)
//	{
//		sprintf((char*)pBuffer, "%02X%08ld,", 0x02, s_PaperFeedRetryCnt.ul_PaperFeedRetry3_LCTCnt);
//		_INC_SIZE(&pBuffer, &pDst->us_Len, PAR_DATA_SIZE);
//	}
//	if (s_PaperFeedRetryCnt.ul_PaperFeedRetry4Cnt		>= (ushort)uc_DipSw30)
//	{
//		sprintf((char*)pBuffer, "%02X%08ld,", 0x03, s_PaperFeedRetryCnt.ul_PaperFeedRetry4Cnt);
//		_INC_SIZE(&pBuffer, &pDst->us_Len, PAR_DATA_SIZE);
//	}
//	if (s_PaperFeedRetryCnt.ul_PaperFeedRetryManualCnt	>= (ushort)uc_DipSw30)
//	{
//		sprintf((char*)pBuffer, "%02X%08ld,", 0x04, s_PaperFeedRetryCnt.ul_PaperFeedRetryManualCnt);
//		_INC_SIZE(&pBuffer, &pDst->us_Len, PAR_DATA_SIZE);
//	}
//	if (s_PaperFeedRetryCnt.ul_PaperFeedRetryExtLCTCnt	>= (ushort)uc_DipSw30)
//	{
//		sprintf((char*)pBuffer, "%02X%08ld", 0x05, s_PaperFeedRetryCnt.ul_PaperFeedRetryExtLCTCnt);
//		_INC_SIZE(&pBuffer, &pDst->us_Len, PAR_DATA_SIZE - 1);
//	}
// 2020/12/24 FPT-ThanhPC7 (TAS000000005562) Eagle(C650i)で夜中に「手差し給紙リトライ多発警告」が送信される End.
// 2019/02/28 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-3715】 Modification End
// 2019/02/28 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-3432】 Modification End
	return	(ushort)strlen(reinterpret_cast<const char*>(pBuffer));
}
// 2018/12/03 FPT-DucDH4 【IT6_2.0】【EagleH】【機器構成変更_No.1】 リトライ多発警告 End
/*==================================================================================================*/
//   関数（イベントデータ作成関数）
/*--------------------------------------------------------------------------------------------------*/
// 04/03/29 浦崎 警告発信と警告解除発信で、逆のTypeコードとしてしまっていたため修正。  
// 04/09/10 浦崎 任意交換パーツ警告発信に対応。
/*==================================================================================================*/
void RDSC_mkText_ModeWarn::makeWarnData( TYPE_RdsTrgKind e_TrgKind )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"#[47]RDSC_mkText_ModeWarn::makeWarnData[st]", (uint32_t)e_TrgKind );
	RDSS_WarnData			s_Data;
	RDSS_EventData*			pDst;
	register uchar*			pBuff;
	ushort					us_Size;
	ushort					us_TmpLen;
	uint32_t 				ul_Value = 0;	// 原稿系/用紙系 JAM多発警告カウンタ格納変数 080425 寺内. 
// FQ****** 警告多発対応 yito 2011/09/01 start.
	if( isRepeatedWarnProtect() )
	{
		// 無条件で、イベントバッファをクリアする.
		c_EvevntDataBuf.clearCurrentBuff();

		// 現在の警告を覚える. 
		// 単発もの（解除不要の警告）は覚えない＆多発状態も検知しない.
		recordWarn();
	}
// FQ****** 警告多発対応 yito 2011/09/01 end  .

	while( RDSC_WarnData::get( &s_Data ) )
	{
		memcpy(&s_TempData, &s_Data, sizeof(RDSS_WarnData));
		EPN_DBG_MESSAGE( (uchar*)"[CSRC]", (uint32_t)s_Data.uc_Kind,  (uchar*)" e_kind" );			//2018/10/18 FPT-DucDH4 Modification log debug
		EPN_DBG_MESSAGE( (uchar*)"[CSRC]", (uint32_t)s_Data.uc_Level, (uchar*)" e_Level" );			//2018/10/18 FPT-DucDH4 Modification log debug
		EPN_DBG_MESSAGE( (uchar*)"[CSRC]", (uint32_t)s_Data.uc_Map,   (uchar*)" uc_Map" );			//2018/10/18 FPT-DucDH4 Modification log debug

		if(    (s_Data.uc_Kind  < RDSE_WarnTrg_NumOfTrg)
// 2014/12/24 ktakahashi ZeusS 機器構成変更_No.40 Start.
			&& (s_Data.uc_Level < RDSD_WarnStat_Num) )
// 2014/12/24 ktakahashi ZeusS 機器構成変更_No.40 End.
		{
// 2018/10/18 FPT-DucDH4 IT6_1.0[Eagle] 機器構成変更_No.92 Start.
			if ((RDSE_WarnTrgTonerEmptyY == s_Data.uc_Kind)
				|| (RDSE_WarnTrgTonerEmptyM == s_Data.uc_Kind)
				|| (RDSE_WarnTrgTonerEmptyC == s_Data.uc_Kind)
				|| (RDSE_WarnTrgTonerEmptyK == s_Data.uc_Kind))
			{
				isMakeTMData = True;
			}
// 2018/10/18 FPT-DucDH4 IT6_1.0[Eagle] 機器構成変更_No.92 End.

			pDst  = c_EvevntDataBuf.wt;
			pBuff = &pDst->uc_Buff[0];										// イベントデータバッファへのポインタを取得

			c_EvevntDataBuf.clearCurrentBuff();								/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */

				EPN_DBG_MESSAGE( (uchar*)"[CSRC]", (uint32_t)s_Data.uc_Level , (uchar*)"#[47]RDSC_mkText_ModeWarn::makeWarnData[1] uc_Level");			//2018/10/18 FPT-DucDH4 Add log debug
			// イベントデータバッファに「発信種別」をセット
			if( TYPE_RdsWarnStatNormal == (TYPE_RdsWarnStat)s_Data.uc_Level )
			{
				pDst->e_Kind = TYPE_RdsTrgKindWarnRecvBody;					// 警告解除発信 
				theMakeText.warnSubjectInfo |= RDSD_WarnSubInfRecover;
				// ■ E-mail 固有 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
				if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
				 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )		// E-mail版 / HTTP版　寺内(08/01/31) 新規追加.
				 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
				{
					theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindWarnRecvBody );		// 問題点77306対応 
					// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。 
					uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
					s_Data.c_Time.cnvTime_2_String( uc_BcdEventID, RDSE_T2S_BCDyymmddhhmmss );		// yymmddhhmmss  04/03/29 浦崎 重大なミス修正！！！ 
					ushort us_Len = RDSC_MakeEventData::makeM501
						(
							pBuff,				// ユーザーデータバッファ 
							uc_BcdEventID,		// イベントID 
							(uchar*)"WC"		// 発信固有文字列 04/03/29 「WO」から修正。 
						);
					_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
				}
			}
			else
			{
				pDst->e_Kind = TYPE_RdsTrgKindWarnBody;						// 警告発信 
				theMakeText.warnSubjectInfo |= RDSD_WarnSubInfOccur;
				// ■ E-mail 固有 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
				if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
				 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )		// E-mail版 / HTTP版  寺内(08/01/31)　新規追加.
				 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
				{
					theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindWarnBody );			// 問題点77306 
					// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。 
					uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
					s_Data.c_Time.cnvTime_2_String( uc_BcdEventID, RDSE_T2S_BCDyymmddhhmmss );		// yymmddhhmmss 
					ushort us_Len = RDSC_MakeEventData::makeM501
						(
							pBuff,				// ユーザーデータバッファ 
							uc_BcdEventID,		// イベントID 
							(uchar*)"WO"		// 発信固有文字列 04/03/29 「WC」から修正。 
						);
					_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
					
					// 原稿系JAM多発警告 または 用紙系JAM多発警告の場合.
					if( ( ( s_Data.uc_Kind == RDSE_WarnTrgMCBJFrequecy ) || ( s_Data.uc_Kind == RDSE_WarnTrgMOBJFrequecy ) )
						&& ( 0 != ( pc_RdsSetting->getDipSw(20) & RDSD_BIT2 ) ) )
					{
						b_JAMFrequencyFlag = True;
					}
				}
//*******************  add start by tomiyasu 2008/06/17  ******************************//
				else
				{
					if( ( ( s_Data.uc_Kind == RDSE_WarnTrgMCBJFrequecy ) || ( s_Data.uc_Kind == RDSE_WarnTrgMOBJFrequecy ) )
						&& ( 0 != ( pc_RdsSetting->getDipSw(20) & RDSD_BIT2 ) ) )
					{
						b_JAMFrequencyFlag = True;
					}
				}
//*******************  add end by tomiyasu 2008/06/17  ******************************//
			}

			// ■ モデム、FAX、E-mail 共通 
			us_Size = (ushort)(strlen( _WarnString[s_Data.uc_Kind][s_Data.uc_Level] ));	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.
			memcpy( pBuff, _WarnString[s_Data.uc_Kind][s_Data.uc_Level], us_Size );
			_INC_SIZE( &pBuff, &pDst->us_Len, us_Size );

			EPN_DBG_MESSAGE( (uchar*)"[CSRC]", (uint32_t)0, (uchar*)pBuff );			//2018/10/18 FPT-DucDH4 Add log debug
			switch( s_Data.uc_Kind )
			{
			case RDSE_WarnTrgPM :				// PMカウンタ警告 
				if( TYPE_RdsWarnStatNormal == s_Data.uc_Level )
				{
//11/26/2021 FPT-HaDV8 [MinervaSSBK] [OP_BTS-36394] Start.
					//セマフォをロックしてPMカウンタ上限値取得後、セマフォを解放する.
					APIC_PmCnt*	pc_PmCntLimit = APIC_PmCnt::newInstance();

					pc_PmCntLimit->lock();
					uint32_t	ul_TempCounter = pc_PmCntLimit->get(TYPE_PmCntLimit);
					pc_PmCntLimit->unlock();
					//uint32_t ul_TempCounter =  CONNECT_BYTE_2_DWORD( s_Data.uc_Detail );
//11/26/2021 FPT-HaDV8 [MinervaSSBK] [OP_BTS-36394] End.
					sprintf( (char*)pBuff, "%02ld", ((ul_TempCounter/10000) % 100) );
					_INC_SIZE( &pBuff, &pDst->us_Len, 2 );
				}
				else
				{
//11/26/2021 FPT-HaDV8 [MinervaSSBK] [OP_BTS-36394] Start.
					//セマフォをロックしてPMカウンタ上限値取得後、セマフォを解放する.
					APIC_PmCnt*	pc_PmCntLimit = APIC_PmCnt::newInstance();

					pc_PmCntLimit->lock();
					uint32_t	ul_TempCounter = pc_PmCntLimit->get(TYPE_PmCntPM);
					pc_PmCntLimit->unlock();
					//uint32_t ul_TempCounter =  CONNECT_BYTE_2_DWORD( s_Data.uc_Detail );
//11/26/2021 FPT-HaDV8 [MinervaSSBK] [OP_BTS-36394] End.
					sprintf( (char*)pBuff, "%06ld", (ul_TempCounter%1000000) );
					_INC_SIZE( &pBuff, &pDst->us_Len, 6 );
				}
				break;

			case RDSE_WarnTrgPartsReplacing :	// 任意交換パーツ警告 04/09/10 浦崎 
				us_TmpLen = makePartsReplacingData( &s_Data, pBuff );
				_INC_SIZE( &pBuff, &pDst->us_Len, us_TmpLen );
				EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)us_TmpLen,  (uchar*)" 任意パーツデータサイズ" );
				break;

			case RDSE_WarnTrgMCBJFrequecy :		// 用紙系JAM多発警告 08/04/25 寺内. 
				ul_Value = APIC_JamDailyCnt::newInstance()->get( TYPE_JamDailyCntPaper );		// 用紙系JAM多発カウンター値取得. 
				sprintf( (char*)pBuff, "%08ld", ul_Value );
				_INC_SIZE( &pBuff, &pDst->us_Len, 8 );
				break;

			case RDSE_WarnTrgMOBJFrequecy :		// 原稿系JAM多発警告 08/04/25 寺内. 
				ul_Value = APIC_JamDailyCnt::newInstance()->get( TYPE_JamDailyCntDocument );	// 原稿系JAM多発カウンター値取得. 
				sprintf( (char*)pBuff, "%08ld", ul_Value );
				_INC_SIZE( &pBuff, &pDst->us_Len, 8 );
				break;
// 2018/12/03 FPT-DucDH4 【IT6_2.0】【EagleH】【機器構成変更_No.1】 リトライ多発警告 Start
			case RDSE_WarnTrgRetryFrequency:		// リトライ多発警告
				us_TmpLen = makeRetryFrequencyData(pDst, pBuff);
				_INC_SIZE(&pBuff, &pDst->us_Len, us_TmpLen);
				EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)us_TmpLen, (uchar*)" RDSE_WarnTrgRetryFrequency");
				break;
// 2018/12/03 FPT-DucDH4 【IT6_2.0】【EagleH】【機器構成変更_No.1】 リトライ多発警告 End
			default :
				break;
			}

			c_EvevntDataBuf.updateWtPtr();
/*			RDS_modem_Kind |=  0x00000008;	*/			// 発着信種別 警告発信bitON  2014/02/03 add by tomiyasu
		}
	}
}

// 2018/10/18 FPT-DucDH4 IT6_1.0[Eagle] 機器構成変更_No.92 Start.
//関数-------------------------------------------------------------------------------------------------------------------
//概要　　：get TCR value.
//作成者　：.
//レビュー： .
ushort getTCRvalue(uchar uc_EDIType)
{
	if (uc_EDIType < 0 || uc_EDIType >= TYPE_EDI_MaxString)	/* 2018/11/01 Eagle 静的解析対応: ID2764321 条件式変更 */
	{
		return 0;
	}
	ushort us_EDI[TYPE_EDI_MaxString];
	APIC_PrinterStatus::newInstance()->getEngDataInfo(us_EDI);
	return us_EDI[uc_EDIType];
}

//関数-------------------------------------------------------------------------------------------------------------------
//概要　　：Ther Total Counter Value
//作成者　：.
//レビュー： .The Total Color Counter(カラートータルカウント値) = Printer Full Color + Printer 2 Color + Copy Full Color + Copy Two Color+ Copy Single Color
uint32_t getTotalColorCounter()
{
	//the copy full color counter								  
	uint32_t ul_CopyFullColor = APIC_TotalCnt::newInstance()->get(TYPE_TotalCntCopyTotalFullColor, TYPE_NetworkSpecific_NW1)
		+ APIC_TotalCnt::newInstance()->get(TYPE_TotalCntCopyTotalFullColor, TYPE_NetworkSpecific_NW2);
	//the Print full color counter								  
	uint32_t ul_Printer_FullColor = APIC_TotalCnt::newInstance()->get(TYPE_TotalCntPrintTotalFullColor, TYPE_NetworkSpecific_NW1)
		+ APIC_TotalCnt::newInstance()->get(TYPE_TotalCntPrintTotalFullColor, TYPE_NetworkSpecific_NW2);
	//the Copy single color counter
	uint32_t ul_Copy_Mono_TOTAL = APIC_TotalCnt::newInstance()->get(TYPE_TotalCntCopyTotalMonoColor, TYPE_NetworkSpecific_NW1)
		+ APIC_TotalCnt::newInstance()->get(TYPE_TotalCntCopyTotalMonoColor, TYPE_NetworkSpecific_NW2);
	//the Copy two color counter					
	uint32_t ul_Copy_Two_Color_TOTAL = APIC_TotalCnt::newInstance()->get(TYPE_TotalCntCopyTotal2Color, TYPE_NetworkSpecific_NW1)
		+ APIC_TotalCnt::newInstance()->get(TYPE_TotalCntCopyTotal2Color, TYPE_NetworkSpecific_NW2);
	//the Print two color counter						
	uint32_t ul_Printer_Two_Color_TOTAL = APIC_TotalCnt::newInstance()->get(TYPE_TotalCntPrintTotal2Color, TYPE_NetworkSpecific_NW1)
		+ APIC_TotalCnt::newInstance()->get(TYPE_TotalCntPrintTotal2Color, TYPE_NetworkSpecific_NW2);

	uint32_t ul_COLOR_TOTAL = ul_Printer_FullColor + ul_Printer_Two_Color_TOTAL + ul_CopyFullColor + ul_Copy_Mono_TOTAL + ul_Copy_Two_Color_TOTAL;
	return ul_COLOR_TOTAL;
}
//関数-------------------------------------------------------------------------------------------------------------------
//概要　　：get TCR value.
//作成者　：.
//レビュー： .//Copy Total = Full Color + Black  + Single Color + 2 Color	(エンプティ解除時のコピートータルカウント値)
uint32_t getTotalCopyCounter()
{

	//the back copy counter									  
	uint32_t ul_COPY_BLACK = APIC_TotalCnt::newInstance()->get(TYPE_TotalCntCopyTotalBlack, TYPE_NetworkSpecific_NW1)
		+ APIC_TotalCnt::newInstance()->get(TYPE_TotalCntCopyTotalBlack, TYPE_NetworkSpecific_NW2);
	//the full color copy counter									  
	uint32_t ul_CopyFullColor = APIC_TotalCnt::newInstance()->get(TYPE_TotalCntCopyTotalFullColor, TYPE_NetworkSpecific_NW1)
		+ APIC_TotalCnt::newInstance()->get(TYPE_TotalCntCopyTotalFullColor, TYPE_NetworkSpecific_NW2);
	//the single color copy counter
	uint32_t ul_Copy_Mono_TOTAL = APIC_TotalCnt::newInstance()->get(TYPE_TotalCntCopyTotalMonoColor, TYPE_NetworkSpecific_NW1)
		+ APIC_TotalCnt::newInstance()->get(TYPE_TotalCntCopyTotalMonoColor, TYPE_NetworkSpecific_NW2);
	//the two color copy counter					
	uint32_t ul_Copy_Two_Color_TOTAL = APIC_TotalCnt::newInstance()->get(TYPE_TotalCntCopyTotal2Color, TYPE_NetworkSpecific_NW1)
		+ APIC_TotalCnt::newInstance()->get(TYPE_TotalCntCopyTotal2Color, TYPE_NetworkSpecific_NW2);

	uint32_t ul_COPY_TOTAL = ul_COPY_BLACK + ul_CopyFullColor + ul_Copy_Mono_TOTAL + ul_Copy_Two_Color_TOTAL;
	return ul_COPY_TOTAL;
}
//関数-------------------------------------------------------------------------------------------------------------------
//概要　　：get TCR value.
//作成者 ：
//レビュー ： Print Total = Full Color + Black  + Single Color + 2 Color (エンプティ解除時のプリンタトータルカウント値)		
uint32_t getTotalPrintCounter()
{
	//the black print counter									  									  
	uint32_t ul_PrinterBlack = APIC_TotalCnt::newInstance()->get(TYPE_TotalCntPrintTotalBlack, TYPE_NetworkSpecific_NW1)
		+ APIC_TotalCnt::newInstance()->get(TYPE_TotalCntPrintTotalBlack, TYPE_NetworkSpecific_NW2);
	//the full color print counter								  
	uint32_t ul_Printer_FullColor = APIC_TotalCnt::newInstance()->get(TYPE_TotalCntPrintTotalFullColor, TYPE_NetworkSpecific_NW1)
		+ APIC_TotalCnt::newInstance()->get(TYPE_TotalCntPrintTotalFullColor, TYPE_NetworkSpecific_NW2);
	//the two color print counter					
	uint32_t ul_Printer_Two_Color_TOTAL = APIC_TotalCnt::newInstance()->get(TYPE_TotalCntPrintTotal2Color, TYPE_NetworkSpecific_NW1)
		+ APIC_TotalCnt::newInstance()->get(TYPE_TotalCntPrintTotal2Color, TYPE_NetworkSpecific_NW2);


	uint32_t ul_PRINTER_TOTAL = ul_Printer_FullColor + ul_PrinterBlack + ul_Printer_Two_Color_TOTAL;
	return ul_PRINTER_TOTAL;
}
// 2018/07/17 【Eagle 機器No.92】 HIENG END.
char getTonerQuantity(uchar	uc_Kind)
{
	switch (uc_Kind)
	{
	case RDSE_WarnTrgTonerEmptyY:
		return APIC_PrinterWarning::newInstance()->getYTonerQuantity();
		break;
	case RDSE_WarnTrgTonerEmptyM:
		return APIC_PrinterWarning::newInstance()->getMTonerQuantity();
		break;
	case RDSE_WarnTrgTonerEmptyC:
		return APIC_PrinterWarning::newInstance()->getCTonerQuantity();
		break;
	case RDSE_WarnTrgTonerEmptyK:
		return APIC_PrinterWarning::newInstance()->getKTonerQuantity();
		break;
	case RDSE_WarnTrgBottleFull:
		return 100;
		break;
	default:
		return -1;	/* 2018/11/01 Eagle 静的解析対応: ID2764336 戻り値追加 */
		break;
	}
// 2018/11/01 FPT-DatNT30 IT6_1.0 【AR1029580】 fix warning Start
	//uc_Kind is invalid --> return -1;
	return -1;
// 2018/11/01 FPT-DatNT30 IT6_1.0 【AR1029580】 fix warning End
}
// 2018/12/05 FPT-ChiNM IT6_1.0 【機器構成変更_No.92】 Modification Start.
void RDSC_mkText_ModeWarn::getCoverageXML(RDSS_EventData* pEventData, uchar* pBuffer, uchar ucKind)
{
// 2019/03/14 FPT-DucDH4 IT6_1.0 【1073924】 Modification Start.
	const char *XML_HEADER = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n";

	const char *XML_COVERAGE_COUNTER[] = {
		"<CCI>\n", 
			"<CpCnt>\n",
				"<C1>\n", 
					"<Bw>\n", " ", "</Bw>\n", 
				"</C1>\n",
				"<C3>\n", 
					"<Yw>\n", " ","</Yw>\n", 
					"<Mg>\n", " ", "</Mg>\n", 
					"<Cy>\n", " ","</Cy>\n", 
				"</C3>\n",
				"<C4>\n", 
					"<Yw>\n", " ", "</Yw>\n", 
					"<Mg>\n", " ", "</Mg>\n", 
					"<Cy>\n", " ", "</Cy>\n", 
					"<CMYK>\n", " ", "</CMYK>\n", 
				"</C4>\n", 
			"</CpCnt>\n", 
			"<PrCnt>\n", 
				"<C1>\n", 
					"<Bw>\n", " ", "</Bw>\n", 
				"</C1>\n", 
				"<C4>\n", 
					"<Yw>\n", " ", "</Yw>\n", 
					"<Mg>\n", " ", "</Mg>\n", 
					"<Cy>\n", " ", "</Cy>\n", 
					"<CMYK>\n", " ", "</CMYK>\n", 
				"</C4>\n", 
			"</PrCnt>\n", 
			"<SFCnt>\n", 
				"<C1>\n", 
					"<Bw>\n", " ", "</Bw>\n",
				"</C1>\n", 
				"<C4>\n", 
					"<Yw>\n", " ", "</Yw>\n", 
					"<Mg>\n", " " , "</Mg>\n", 
					"<Cy>\n", " " ,"</Cy>\n", 
					"<CMYK>\n", " ", "</CMYK>\n", 
				"</C4>\n", 
			"</SFCnt>\n", 
		"</CCI>\n", " "
		};

	const int MAX_CUMULATIVE = 40;
	const char *XML_CUMULATIVE_COUNTER[] = {
			"<P0>",  "</P0>\n",  "<P1>",  "</P1>\n",  "<P2>",  "</P2>\n",  "<P3>",  "</P3>\n",  "<P4>",   "</P4>\n",
			"<P5>",  "</P5>\n",  "<P6>",  "</P6>\n",  "<P7>",  "</P7>\n",  "<P8>",  "</P8>\n",  "<P9>",   "</P9>\n",
			"<P10>", "</P10>\n", "<P11>", "</P11>\n", "<P12>", "</P12>\n", "<P13>", "</P13>\n", "<P14>",  "</P14>\n",
			"<P15>", "</P15>\n", "<P16>", "</P16>\n", "<P17>", "</P17>\n", "<P18>", "</P18>\n", "<P19>",  "</P19>\n",
			"<P20>", "</P20>\n", "<P21>", "</P21>\n", "<P22>", "</P22>\n", "<P23>", "</P23>\n", "<P24>",  "</P24>\n",
			"<P25>", "</P25>\n", "<P26>", "</P26>\n", "<P27>", "</P27>\n", "<P28>", "</P28>\n", "<P29>",  "</P29>\n",
			"<P30>", "</P30>\n", "<P35>", "</P35>\n", "<P40>", "</P40>\n", "<P45>", "</P45>\n", "<P50>",  "</P50>\n",
			"<P60>", "</P60>\n", "<P70>", "</P70>\n", "<P80>", "</P80>\n", "<P90>", "</P90>\n", "<P100>", "</P100>\n"
		};

	TYPS_CSRCCoverageCnt sCoverageCnt = {};

	APIC_CSRCCoverageCnt *pCoverageCnt = APIC_CSRCCoverageCnt::newInstance();

	EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)ucKind, (uchar*)"ucKind");

	// Add xml header
	ushort usXMLHeaderLen = (ushort)strlen(XML_HEADER);
	sprintf((char*)pBuffer, "%s", XML_HEADER);
	_INC_SIZE(&pBuffer, &pEventData->us_Len, usXMLHeaderLen);

	ushort ucCoverageTypeIndex = 0;
	ushort ucCounterIndex = 0;

	uint32_t lCounter = 0;
	int counterLen = 0;
	
	const ushort MAX_COVERAGE_TYPE = 18;
	const TYPE_CSRCCoverageCntType COVERAGE_TYPE[] = {
		// Copy 
		TYPE_CSRCCoverageCntType_Copy_1C_K,
		TYPE_CSRCCoverageCntType_Copy_3C_Y,			TYPE_CSRCCoverageCntType_Copy_3C_M,			TYPE_CSRCCoverageCntType_Copy_3C_C,
		TYPE_CSRCCoverageCntType_Copy_Other_Y,		TYPE_CSRCCoverageCntType_Copy_Other_M,		TYPE_CSRCCoverageCntType_Copy_Other_C,		TYPE_CSRCCoverageCntType_Copy_Other_K,
		// PcPrint 
		TYPE_CSRCCoverageCntType_PC_1C_K,
		TYPE_CSRCCoverageCntType_PC_Other_Y,		TYPE_CSRCCoverageCntType_PC_Other_M,		TYPE_CSRCCoverageCntType_PC_Other_C,		TYPE_CSRCCoverageCntType_PC_Other_K,
		// ScanFax 
		TYPE_CSRCCoverageCntType_ScanFax_1C_K,
		TYPE_CSRCCoverageCntType_ScanFax_Other_Y,	TYPE_CSRCCoverageCntType_ScanFax_Other_M,	TYPE_CSRCCoverageCntType_ScanFax_Other_C,	TYPE_CSRCCoverageCntType_ScanFax_Other_K
		};
		
	 for (ucCounterIndex = 0; ucCounterIndex < MAX_COVERAGE_TYPE; ucCounterIndex++)
	 {
		// Loop until cumulative element [" "]
		while (strlen(XML_COVERAGE_COUNTER[ucCoverageTypeIndex]) > 1)
		{
			sprintf((char*)pBuffer, XML_COVERAGE_COUNTER[ucCoverageTypeIndex]);
			_INC_SIZE(&pBuffer, &pEventData->us_Len, (ushort)strlen(XML_COVERAGE_COUNTER[ucCoverageTypeIndex]));
			ucCoverageTypeIndex++;
		}

		// Get coverage cumulative counter and append to XML buffer
		pCoverageCnt->lock();
		pCoverageCnt->getCSRCCoverageCnt(COVERAGE_TYPE[ucCounterIndex], &sCoverageCnt);
		pCoverageCnt->unlock();

		int cumulativeItem = 0;	 // Start with <P0> node
		for (int pxIndex = 0; pxIndex < MAX_CUMULATIVE; pxIndex++)
		{
			// Start with <Px> node
			sprintf((char*)pBuffer, XML_CUMULATIVE_COUNTER[cumulativeItem]);
			_INC_SIZE(&pBuffer, &pEventData->us_Len, (ushort)strlen(XML_CUMULATIVE_COUNTER[cumulativeItem]));

			// Get value for <Px> node
			lCounter = sCoverageCnt.uc_CoverageCnt[pxIndex][0] * 256 * 256 + sCoverageCnt.uc_CoverageCnt[pxIndex][1] * 256 + sCoverageCnt.uc_CoverageCnt[pxIndex][2];
			counterLen = sprintf((char*)pBuffer, "%ld", lCounter);
			_INC_SIZE(&pBuffer, &pEventData->us_Len, counterLen);

			// Next to </Px> node
			cumulativeItem++;
			sprintf((char*)pBuffer, XML_CUMULATIVE_COUNTER[cumulativeItem]);
			_INC_SIZE(&pBuffer, &pEventData->us_Len, (ushort)strlen(XML_CUMULATIVE_COUNTER[cumulativeItem]));

			// Next to <Px++> node
			cumulativeItem++;
		 }

		 // Increase index to move next element
		 ucCoverageTypeIndex++;

		 // Loop until latest element [" "]
		 while (strlen(XML_COVERAGE_COUNTER[ucCoverageTypeIndex]) > 1)
		 {
			sprintf((char*)pBuffer, XML_COVERAGE_COUNTER[ucCoverageTypeIndex]);
			_INC_SIZE(&pBuffer, &pEventData->us_Len, (ushort)strlen(XML_COVERAGE_COUNTER[ucCoverageTypeIndex]));
			ucCoverageTypeIndex++;
		 }
	 }
	 EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)pEventData->us_Len, (uchar*)"[5. getCoverageXML()]pEventData->us_Len");
// 2019/03/14 FPT-DucDH4 IT6_1.0 【1073924】 Modification End.
}
// 2018/12/05 FPT-ChiNM IT6_1.0 【機器構成変更_No.92】 Modification
void makeLogDataTMCommand(RDSS_EventData* pDst, uchar* pBuffCoverXML, uchar uc_Kind)
{
	EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)"#[50]makeLogDataTMCommand[st]", (uint32_t)uc_Kind);

	ushort us_EnvHistoryInfoData[EDI_EnvHistorySize] = {};
	APIC_PrinterStatus* pc_PrinterStatus = APIC_PrinterStatus::newInstance();
	pc_PrinterStatus->lock();
	pc_PrinterStatus->getEnvironmentalHistoryInfoData(us_EnvHistoryInfoData);
	pc_PrinterStatus->unlock();

	NVDC_RdsSetting* pNVDC_RdsSetting = NVDC_RdsSetting::newInstance();
	NVDS_EmptyTonerLogSettingK st_EmptyTonerLogSettingK = {};
	pNVDC_RdsSetting->lock();
	pNVDC_RdsSetting->getEmptyTonerInfoK(&st_EmptyTonerLogSettingK);
	pNVDC_RdsSetting->unlock();

//	char *pBuffTM = (char *)pBuffCoverXML;
	uchar *pBuffTM = pBuffCoverXML;			// 2019/01/09 FPT-DucDH4 IT6_1.0 【DenebMLK】 Fix Warning modification

	uint32_t ul_GRAND_TOTAL = APIC_TotalCnt::newInstance()->get(TYPE_TotalCntTotal, TYPE_NetworkSpecific_NW1) + APIC_TotalCnt::newInstance()->get(TYPE_TotalCntTotal, TYPE_NetworkSpecific_NW2);
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x02, ul_GRAND_TOTAL);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);
// 2019/03/14 FPT-DucDH4 IT6_1.0 【1073924】 Modification Start.
	uint32_t ul_COLOR_TOTAL = getTotalColorCounter();
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x03, ul_COLOR_TOTAL);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);
	
	uint32_t ul_COPY_TOTAL = getTotalCopyCounter();
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x04, ul_COPY_TOTAL);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);
	
	uint32_t ul_COPY_BLACK = APIC_TotalCnt::newInstance()->get(TYPE_TotalCntCopyTotalBlack, TYPE_NetworkSpecific_NW1) + APIC_TotalCnt::newInstance()->get(TYPE_TotalCntCopyTotalBlack, TYPE_NetworkSpecific_NW2);
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x05, ul_COPY_BLACK);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_COPY_FULLCOLOR = APIC_TotalCnt::newInstance()->get(TYPE_TotalCntCopyTotalFullColor, TYPE_NetworkSpecific_NW1) + APIC_TotalCnt::newInstance()->get(TYPE_TotalCntCopyTotalFullColor, TYPE_NetworkSpecific_NW2);
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x06, ul_COPY_FULLCOLOR);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_PRINTER_TOTAL = getTotalPrintCounter();
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x07, ul_PRINTER_TOTAL);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);
	
	uint32_t ul_PRINTER_BLACK = APIC_TotalCnt::newInstance()->get(TYPE_TotalCntPrintTotalBlack, TYPE_NetworkSpecific_NW1) + APIC_TotalCnt::newInstance()->get(TYPE_TotalCntPrintTotalBlack, TYPE_NetworkSpecific_NW2);
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x08, ul_PRINTER_BLACK);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_PRINTER_FULLCOLOR = APIC_TotalCnt::newInstance()->get(TYPE_TotalCntPrintTotalFullColor, TYPE_NetworkSpecific_NW1) + APIC_TotalCnt::newInstance()->get(TYPE_TotalCntPrintTotalFullColor, TYPE_NetworkSpecific_NW2);
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x09, ul_PRINTER_FULLCOLOR);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_GASGAGE = getTonerQuantity(uc_Kind);			//Kゲージ
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x0A, ul_GASGAGE);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_TCR_K_STANDARD = getTCRvalue(TYPE_TCR_K_STANDARD);			//TCR　基準値
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x0B, ul_TCR_K_STANDARD);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_TCR_K_CURRENT = getTCRvalue(TYPE_TCR_K_CURRENT);				//TCR　現状の値
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x0C, ul_TCR_K_CURRENT);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	char c_SUPPLY = 100 - getTonerQuantity(uc_Kind);			// 既補給量
// 2019/03/14 FPT-DucDH4 IT6_1.0 【1073924】 Modification End.
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x0D, c_SUPPLY);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

// 2019/04/09 FPT-DucDH4 IT6_1.0 【1073924】 Modification Start.
	// ushort us_Value = us_EnvHistoryInfoData[0x0E];
	ushort us_Value = us_EnvHistoryInfoData[0x00];
	ushort us_HUMIDITY = (ushort)((us_Value >> 6) & 0x7F);			//HUMIDITY
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x0E, us_HUMIDITY);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	// us_Value = us_EnvHistoryInfoData[0x0F];
	ushort us_TEMPERATURE = (ushort)(us_Value & 0x3F);			//TEMPERATURE
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x0F, us_TEMPERATURE);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);
// 2019/04/09 FPT-DucDH4 IT6_1.0 【1073924】 Modification End.

	uint32_t ul_DRUM_UNIT_ROTATION_TIME_C = APIC_LifeCnt::newInstance()->get(TYPE_LifeCntIU_C);						//DC_C 回転時間
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x10, ul_DRUM_UNIT_ROTATION_TIME_C);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_DRUM_UNIT_ROTATION_TIME_M = APIC_LifeCnt::newInstance()->get(TYPE_LifeCntIU_M);						//DC_M 回転時間
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x11, ul_DRUM_UNIT_ROTATION_TIME_M);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_DRUM_UNIT_ROTATION_TIME_Y = APIC_LifeCnt::newInstance()->get(TYPE_LifeCntIU_Y);						//DC_Y 回転時間
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x12, ul_DRUM_UNIT_ROTATION_TIME_Y);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_DRUM_UNIT_ROTATION_TIME_K = APIC_LifeCnt::newInstance()->get(TYPE_LifeCntIU_K);						//DC_K回転時間
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x13, ul_DRUM_UNIT_ROTATION_TIME_K);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_DEVOLOPING_UNIT_PAGE_COUNT_C = APIC_LifeCnt::newInstance()->get(TYPE_LifeCntIU_C_PrintNum);				//DevC_C 印字枚数
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x14, ul_DEVOLOPING_UNIT_PAGE_COUNT_C);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_DEVOLOPING_UNIT_PAGE_COUNT_M = APIC_LifeCnt::newInstance()->get(TYPE_LifeCntIU_M_PrintNum);				//DevC_M 印字枚数
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x15, ul_DEVOLOPING_UNIT_PAGE_COUNT_M);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_DEVOLOPING_UNIT_PAGE_COUNT_Y = APIC_LifeCnt::newInstance()->get(TYPE_LifeCntIU_Y_PrintNum);				//DevC_Y 印字枚数
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x16, ul_DEVOLOPING_UNIT_PAGE_COUNT_Y);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_DEVOLOPING_UNIT_PAGE_COUNT_K = APIC_LifeCnt::newInstance()->get(TYPE_LifeCntDevC_K_PrintNum);				//DevC_K 印字枚数	// 2019/03/14 FPT-DucDH4 IT6_1.0 【1073924】 Modification.
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x17, ul_DEVOLOPING_UNIT_PAGE_COUNT_K);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_COLOR_1P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt1pjColor);			//P/Jカウンター カラー 1P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x18, ul_P_J_COUNTER_COLOR_1P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_COLOR_2P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt2pjColor);			//P/Jカウンター カラー 2P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x19, ul_P_J_COUNTER_COLOR_2P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_COLOR_3P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt3pjColor);			//P/Jカウンター カラー 3P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x1A, ul_P_J_COUNTER_COLOR_3P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_COLOR_4P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt4pjColor);			//P/Jカウンター カラー 4P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x1B, ul_P_J_COUNTER_COLOR_4P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_COLOR_5P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt5pjColor);			//P/Jカウンター カラー 5P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x1C, ul_P_J_COUNTER_COLOR_5P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_COLOR_6P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt6pjColor);			//P/Jカウンター カラー 6P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x1D, ul_P_J_COUNTER_COLOR_6P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_COLOR_7P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt7pjColor);			//P/Jカウンター カラー 7P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x1E, ul_P_J_COUNTER_COLOR_7P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_COLOR_8P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt8pjColor);			//P/Jカウンター カラー 8P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x1F, ul_P_J_COUNTER_COLOR_8P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_COLOR_9P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt9pjColor);			//P/Jカウンター カラー 9P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x20, ul_P_J_COUNTER_COLOR_9P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_COLOR_10P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt10pjColor);			//P/Jカウンター カラー 10P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x21, ul_P_J_COUNTER_COLOR_10P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_COLOR_11P_J_AND_MORE = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCntMoreThan10pjColor);	//P/Jカウンター カラー 11P/J以上
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x22, ul_P_J_COUNTER_COLOR_11P_J_AND_MORE);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_MONOCHROME_1P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt1pjMono);			//P/Jカウンター モノクロ 1P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x23, ul_P_J_COUNTER_MONOCHROME_1P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_MONOCHROME_2P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt2pjMono);			//P/Jカウンター モノクロ 2P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x24, ul_P_J_COUNTER_MONOCHROME_2P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_MONOCHROME_3P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt3pjMono);			//P/Jカウンター モノクロ 3P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x25, ul_P_J_COUNTER_MONOCHROME_3P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_MONOCHROME_4P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt4pjMono);			//P/Jカウンター モノクロ 4P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x26, ul_P_J_COUNTER_MONOCHROME_4P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_MONOCHROME_5P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt5pjMono);			//P/Jカウンター モノクロ 5P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x27, ul_P_J_COUNTER_MONOCHROME_5P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_MONOCHROME_6P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt6pjMono);			//P/Jカウンター モノクロ 6P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x28, ul_P_J_COUNTER_MONOCHROME_6P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_MONOCHROME_7P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt7pjMono);			//P/Jカウンター モノクロ 7P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x29, ul_P_J_COUNTER_MONOCHROME_7P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_MONOCHROME_8P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt8pjMono);			//P/Jカウンター モノクロ 8P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x2A, ul_P_J_COUNTER_MONOCHROME_8P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_MONOCHROME_9P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt9pjMono);			//P/Jカウンター モノクロ 9P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x2B, ul_P_J_COUNTER_MONOCHROME_9P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_MONOCHROME_10P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt10pjMono);			//P/Jカウンター モノクロ 10P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x2C, ul_P_J_COUNTER_MONOCHROME_10P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t P_J_COUNTER_MONOCHROME_11P_J_AND_MORE = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCntMoreThan10pjMono);	//P/Jカウンター モノクロ 11P/J以上
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x2D, P_J_COUNTER_MONOCHROME_11P_J_AND_MORE);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_ACS_1P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt1pjAcs);				//P/Jカウンター ACS 1P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x2E, ul_P_J_COUNTER_ACS_1P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_ACS_2P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt2pjAcs);				//P/Jカウンター ACS 2P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x2F, ul_P_J_COUNTER_ACS_2P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_ACS_3P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt3pjAcs);				//P/Jカウンター ACS 3P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x30, ul_P_J_COUNTER_ACS_3P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_ACS_4P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt4pjAcs);				//P/Jカウンター ACS 4P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x31, ul_P_J_COUNTER_ACS_4P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_ACS_5P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt5pjAcs);				//P/Jカウンター ACS 5P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x32, ul_P_J_COUNTER_ACS_5P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_ACS_6P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt6pjAcs);				//P/Jカウンター ACS 6P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x33, ul_P_J_COUNTER_ACS_6P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_ACS_7P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt7pjAcs);				//P/Jカウンター ACS 7P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x34, ul_P_J_COUNTER_ACS_7P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_ACS_8P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt8pjAcs);				//P/Jカウンター ACS 8P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x35, ul_P_J_COUNTER_ACS_8P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_ACS_9P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt9pjAcs);				//P/Jカウンター ACS 9P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x36, ul_P_J_COUNTER_ACS_9P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_ACS_10P_J = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCnt10pjAcs);			//P/Jカウンター ACS 10P/J
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x37, ul_P_J_COUNTER_ACS_10P_J);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);

	uint32_t ul_P_J_COUNTER_ACS_11P_J_AND_MORE = APIC_CopyModeCnt::newInstance()->get(TYPE_CopyModeCntMoreThan10pjAcs);	//P/Jカウンター ACS 11P/J以上
	sprintf((char*)pBuffTM, ",%02X%08ld", 0x38, ul_P_J_COUNTER_ACS_11P_J_AND_MORE);
	_INC_SIZE(&pBuffTM, &pDst->us_Len, PAR_DATA_SIZE);
}
// 2018/12/05 FPT-ChiNM IT6_1.0 【機器構成変更_No.92】 Modification
void RDSC_mkText_ModeWarn::makeWarnDataTM()
{
	EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)s_TempData.uc_Kind, (uchar*)"#[48.1]RDSC_mkText_ModeWarn::makeWarnDataTM[st]", (uint32_t)s_TempData.uc_Level);
	RDSS_EventData*			pDst;
	register uchar*			pBuffTM;
	ushort					us_Len = 0;
	short					writtenSize = 0;

	if ((s_TempData.uc_Kind < RDSE_WarnTrg_NumOfTrg)
		&& (s_TempData.uc_Level < RDSD_WarnStat_Num))
	{
		pDst = c_EvevntDataBuf.wt;
		pBuffTM = &pDst->uc_Buff[0];										// イベントデータバッファへのポインタを取得
		c_EvevntDataBuf.clearCurrentBuff();								/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */
		pDst->e_Kind = TYPE_RdsTrgKindWarnBody;						// 警告発信 
		//theMakeText.warnSubjectInfo |= 0x01;			// 2019/04/22 FPT-DucDH4 【IT6_2.0】【EagleH】【OP_BTS-5013】 Modification.
// 2019.11.27 y.hotta EagleZ+ OP_BTS-14725 本体警告発信/本体警告解除発信の際にStatusManagerに「WOAS」「WCAS」が出る Add Start.
		theMakeText.warnSubjectInfo |= RDSD_WarnSubInfAsync;
// 2019.11.27 y.hotta EagleZ+ OP_BTS-14725 本体警告発信/本体警告解除発信の際にStatusManagerに「WOAS」「WCAS」が出る Add End.
		// ■ E-mail 固有 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
		if ((RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice)
			|| (RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice)		// E-mail版 / HTTP版  寺内(08/01/31)　新規追加.
			|| (RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice))		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
		{
			theVRcontroller.c_MailLog.setKind(TYPE_RdsTrgKindWarnBody);			// 問題点77306 
																				// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。 
			uchar uc_BcdEventID[6 + 1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
			s_TempData.c_Time.cnvTime_2_String(uc_BcdEventID, RDSE_T2S_BCDyymmddhhmmss);		// yymmddhhmmss 

			EPN_DBG_MESSAGE((uchar*)"[CSRC]", (uint32_t)0, (uchar*)uc_BcdEventID);

			us_Len = RDSC_MakeEventData::makeM501
			(
				pBuffTM,			// ユーザーデータバッファ 
				uc_BcdEventID,		// イベントID 
				(uchar*)"AS"		// 発信固有文字列 04/03/29 「WC」から修正。 
			);
			_INC_SIZE(&pBuffTM, &pDst->us_Len, us_Len);
			writtenSize += us_Len;
		}

		EPN_DBG_MESSAGE((uchar*)"[CSRC]", (uint32_t)0, (uchar*)"#[48.2]RDSC_mkText_ModeWarn::makeWarnDataTM");
		memcpy(pBuffTM, "TM00", 4);
		_INC_SIZE(&pBuffTM, &pDst->us_Len, 4);
		writtenSize += 4;

		// ■ モデム、FAX、E-mail 共通 
		us_Len = (ushort)(strlen(_WarnString[s_TempData.uc_Kind][s_TempData.uc_Level]));	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.
		memcpy(pBuffTM, _WarnString[s_TempData.uc_Kind][s_TempData.uc_Level], us_Len);
		_INC_SIZE(&pBuffTM, &pDst->us_Len, us_Len);
		writtenSize += us_Len;

		// Make Coverage XML Data
		memcpy(pBuffTM, ",01", 3);
		_INC_SIZE(&pBuffTM, &pDst->us_Len, 3);
		writtenSize += 3;
// 2019/01/09 FPT-DucDH4 IT6_1.0 【DenebMLK】 Fix Warning modification Start
		EPN_DBG_MESSAGE((uchar*)"[CSRC] pBuffTM(1): ", (uint32_t)us_Len, (uchar*)pBuffTM);
		EPN_DBG_MESSAGE((uchar*)"[CSRC] c_EvevntDataBuf.wt->uc_Buff(1): ", (uint32_t)writtenSize, (uchar*)c_EvevntDataBuf.wt->uc_Buff);
// 2019/01/09 FPT-DucDH4 IT6_1.0 【DenebMLK】 Fix Warning modification End
		getCoverageXML(pDst, pBuffTM, s_TempData.uc_Kind);

		us_Len = (int)strlen((char *)pBuffTM);

		writtenSize += us_Len;
// 2019/01/09 FPT-DucDH4 IT6_1.0 【DenebMLK】 Fix Warning modification Start
		EPN_DBG_MESSAGE((uchar*)"[CSRC] pBuffTM(2): ", (uint32_t)us_Len, (uchar*)pBuffTM);
		EPN_DBG_MESSAGE((uchar*)"[CSRC] c_EvevntDataBuf.wt->uc_Buff(2): ", (uint32_t)writtenSize, (uchar*)c_EvevntDataBuf.wt->uc_Buff);
// 2019/01/09 FPT-DucDH4 IT6_1.0 【DenebMLK】 Fix Warning modification End
		RDSC_CommandMatrix::ConvertReservedToRegulated(pBuffTM, writtenSize);

		pBuffTM += us_Len;

		EPN_DBG_MESSAGE((uchar*)"[CSRC] ConvertReservedToRegulated() c_EvevntDataBuf.wt->uc_Buff(2): ", (uint32_t)writtenSize, (uchar*)c_EvevntDataBuf.wt->uc_Buff);		// 2019/01/09 FPT-DucDH4 IT6_1.0 【DenebMLK】 Fix Warning modification

		// Make log data info
		makeLogDataTMCommand(pDst, pBuffTM, s_TempData.uc_Kind);
		us_Len = (int)strlen((char *)pBuffTM);
		writtenSize += us_Len;
// 2019/01/09 FPT-DucDH4 IT6_1.0 【DenebMLK】 Fix Warning modification Start
		EPN_DBG_MESSAGE((uchar*)"[CSRC] pBuffTM(3): ", (uint32_t)us_Len, (uchar*)pBuffTM);
		EPN_DBG_MESSAGE((uchar*)"[CSRC] c_EvevntDataBuf.wt->uc_Buff(3): ", (uint32_t)writtenSize, (uchar*)c_EvevntDataBuf.wt->uc_Buff);
// 2019/01/09 FPT-DucDH4 IT6_1.0 【DenebMLK】 Fix Warning modification End
		c_EvevntDataBuf.updateWtPtr();
	}
}
// 2018/12/05 FPT-ChiNM IT6_1.0 【機器構成変更_No.92】 Modification End.

/*==================================================================================================*/
//	イベントデータ作成関数「本体警告発信」
/*==================================================================================================*/
void RDSC_mkText_ModeWarn::makeData( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"#[46]RDSC_mkText_ModeWarn::makeData[st]", (uint32_t)e_txtKind);			//2018/10/18 FPT-DucDH4 Modification log debug
	RDSC_mkText_ModeWarn::makeWarnData( e_txtKind );
}

/*==================================================================================================*/
//	発信トリガ処理「本体警告発信」
/*==================================================================================================*/
void RDSC_mkText_ModeWarn::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"#[18]RDSC_mkText_ModeWarn::rcvTrg[st]", (uint32_t)para );
//	if( True == c_WarnData.occurrWarn( para ) )
	{																/* 受付けられた場合 */
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"本体警告発信トリガ受信" );
		cmnTrgAction();
	}
}
void RDSC_mkText_ModeWarn::reset( void )
{
	b_JAMFrequencyFlag = False;
}



/***************************************************************************************************
 *   テキスト作成処理クラス：本体警告解除発信
 ***************************************************************************************************/
/*==================================================================================================*/
//	イベントデータ作成関数「本体警告解除発信」
/*==================================================================================================*/
void RDSC_mkText_ModeWarnRecvBody::makeData( uint32_t para )
{
	// KMRDS では警告発信と警告解除発信は、同クラスで扱う
	EPN_DBG_MESSAGE( (uchar*)"注意！", 0,  (uchar*)" このメッセージが出たらおかしい。" );
//	RDSC_mkText_ModeWarn::makeWarnData( e_txtKind );
}
/*==================================================================================================*/
//	発信トリガ処理「本体警告解除発信」
/*==================================================================================================*/
void RDSC_mkText_ModeWarnRecvBody::rcvTrg( uint32_t para )
{
//	if( True == c_WarnData.recoverWarn( para ) )
	{																/* 受付けられた場合 */
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"本体警告解除発信トリガ受信" );
		cmnTrgAction();
	}
}

/***************************************************************************************************
 *   テキスト作成処理クラス：メンテ開始発信
 ***************************************************************************************************/
/*==================================================================================================*/
//	イベントデータ作成関数「メンテ開始発信」
/*==================================================================================================*/
void RDSC_mkText_ModeMnteStrt::makeData( uint32_t para )
{
	RDSS_EventData*			pDst;
	register uchar*			pBuff;

	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];												// イベントデータバッファへのポインタを取得

	c_EvevntDataBuf.clearCurrentBuff();										/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */


	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）
	pDst->e_Kind = e_txtKind;


	// ■ E-mail 固有 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )		// E-mail版 / HTTP版 寺内(08/01/31) 新規追加.
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。 
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindMnteStart, uc_BcdEventID );
		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ 
				uc_BcdEventID,		// イベントID 
				(uchar*)"RQ"		// 発信固有文字列 
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	// ■ モデム、FAX、E-mail 共通 
	memcpy( pBuff, "MS00", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

	// ■ サービスマンID：
	pc_RdsSetting->getServiceID( pBuff );
	_INC_SIZE( &pBuff, &pDst->us_Len, _S_ServiceID );

	

	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |=  0x00000020;	*/			// 発着信種別 メンテ開始発信bitON  2014/02/03 add by tomiyasu
}

/*==================================================================================================*/
//	発信トリガ処理「メンテ開始発信」
/*==================================================================================================*/
void RDSC_mkText_ModeMnteStrt::rcvTrg( uint32_t para )
{
	if( False == pc_RdsSetting->getMaintStat() )
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"メンテ開始発信トリガ受信" );
		cmnTrgAction();
		pc_RdsSetting->setMaintStat( True );											/* メンテナンス中セット */
	}
}
/*==================================================================================================*/
//	ID の等しいデータがあればクリアする(応答の確認)
/*==================================================================================================*/
void RDSC_mkText_ModeMnteStrt::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	//if( True == EpLib_strcmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )	// 04/03/24 浦崎 BCD なので、0時、等のデータであると、0x00 であるため、それをNULLとみなすため、正しく比較が出来ていなかった。 
	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す。 
		theTrgMap.resetMapByTrgAllPage( e_txtKind );	// 05/02/28 浦崎 resetMapByTrg( , 0 ) → resetMapByTrgAllPage() 
		// ID も消す。 
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}


/***************************************************************************************************
 *   テキスト作成処理クラス：メンテ終了発信
 ***************************************************************************************************/
/*==================================================================================================*/
//	イベントデータ作成関数「メンテ終了発信」
/*==================================================================================================*/
void RDSC_mkText_ModeMnteEnd::makeData( uint32_t para )
{
	RDSS_EventData*			pDst;
	register uchar*			pBuff;

	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];												// イベントデータバッファへのポインタを取得

	c_EvevntDataBuf.clearCurrentBuff();										/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */


	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）
	pDst->e_Kind = e_txtKind;


	// ■ E-mail 固有 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )		// E-mail版 / HTTP版 寺内(08/01/31) 新規追加.
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。 
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindMnteEnd, uc_BcdEventID );
		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ 
				uc_BcdEventID,		// イベントID 
				(uchar*)"RQ"		// 発信固有文字列 
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	// ■ モデム、FAX、E-mail 共通 
	memcpy( pBuff, "MT00", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

	// ■ サービスマンID：
	pc_RdsSetting->getServiceID( pBuff );
	_INC_SIZE( &pBuff, &pDst->us_Len, _S_ServiceID );


	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |=  0x00000040;	*/			// 発着信種別 メンテ終了発信bitON  2014/02/03 add by tomiyasu
}
/*==================================================================================================*/
//	発信トリガ処理「メンテ終了発信」
/*--------------------------------------------------------------------------------------------------*/
// 04/04/19  浦崎 問題点 4671 対策。
//                メンテ前に発生していた緊急発信済みのトラブルをメンテ中に解除してコピーを取り、その後
//                メンテを終了した場合、直ちに緊急回復発信が行われてしまう場合があるため、
//                メンテ終了時に一度緊急回復発信判断用のトータルカウンタ値を更新するようにした。
/*==================================================================================================*/
void RDSC_mkText_ModeMnteEnd::rcvTrg( uint32_t para )
{
	if( True == pc_RdsSetting->getMaintStat() )
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"メンテ終了発信トリガ受信" );
		cmnTrgAction();
		RDSC_TrblMode::upDateLastTotalCounter();					/* 緊急回復発信イベント検出用トータルカウンタ値更新 *//* 04/04/19 浦崎 問題点 4671 対策。 */
		pc_RdsSetting->setMaintStat( False );						/* メンテナンス中解除 */
		//pc_RdsSetting->setServiceID(  );							/* サービスマンIDクリア *//* KMRDS ではここではクリアしない。 */
	}
}
/*==================================================================================================*/
//	ID の等しいデータがあればクリアする(応答の確認)
/*==================================================================================================*/
void RDSC_mkText_ModeMnteEnd::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	//if( True == EpLib_strcmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )	// 04/03/24 浦崎 BCD なので、0時、等のデータであると、0x00 であるため、それをNULLとみなすため、正しく比較が出来ていなかった。 
	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す。 
		theTrgMap.resetMapByTrgAllPage( e_txtKind );	// 05/02/28 浦崎 resetMapByTrg( , 0 ) → resetMapByTrgAllPage() 
		// ID も消す。 
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}


/***************************************************************************************************
 *   テキスト作成処理クラス：管理者発信
 ***************************************************************************************************/
/*==================================================================================================*/
//	イベントデータ作成関数「管理者発信」
/*==================================================================================================*/
void RDSC_mkText_ModeUserCall::makeData( uint32_t para )
{
	RDSS_EventData*			pDst;
	register uchar*			pBuff;

	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];												// イベントデータバッファへのポインタを取得

	c_EvevntDataBuf.clearCurrentBuff();										/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */


	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）
	pDst->e_Kind = e_txtKind;


	// ■ E-mail 固有 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )		// E-mail版 / HTTP版 寺内(08/01/31) 新規追加.
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。 
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindManage, uc_BcdEventID );
		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ 
				uc_BcdEventID,		// イベントID 
				(uchar*)"RQ"		// 発信固有文字列 
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	// ■ モデム、FAX、E-mail 共通 
	memcpy( pBuff, "RQ000", 5 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 5 );


	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |=  0x00000080;	*/			// 発着信種別 管理者発信bitON  2014/02/03 add by tomiyasu
}
/*==================================================================================================*/
//	発信トリガ処理「管理者発信」
/*==================================================================================================*/
void RDSC_mkText_ModeUserCall::rcvTrg( uint32_t para )
{
//	if( False == pc_Rds->getManageTransStat() )
	if( 1 )
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"管理者発信トリガ受信" );
		tskRdsEvent.c_ButtonSTS.rcvTrg( e_txtKind );	// 04/08/09 浦崎 
		//pc_Rds->setManageTransStat( True );						/* 管理者発信中 */
		cmnTrgAction();
	}
}
/*==================================================================================================*/
//	ID の等しいデータがあればクリアする(応答の確認)
/*==================================================================================================*/
void RDSC_mkText_ModeUserCall::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	//if( True == EpLib_strcmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )	// 04/03/24 浦崎 BCD なので、0時、等のデータであると、0x00 であるため、それをNULLとみなすため、正しく比較が出来ていなかった。 
	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)" 管理者発信の応答受信" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す。 
		theTrgMap.resetMapByTrgAllPage( e_txtKind );	// 05/02/28 浦崎 resetMapByTrg( , 0 ) → resetMapByTrgAllPage() 
		// ID も消す。 
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}

/***************************************************************************************************
 *   テキスト作成処理クラス：期日指定発信
 ***************************************************************************************************/
/*==================================================================================================*/
//	イベントデータ作成関数「期日指定発信」
/*--------------------------------------------------------------------------------------------------*/
// 04/02/25 浦崎 問題点2000 対応。問題点 589 の修正による副作用であった。 
/*==================================================================================================*/
void RDSC_mkText_ModeTimeOut::makeData( uint32_t para )
{
	RDSS_EventData*			pDst;
	register uchar*			pBuff;

	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];												// イベントデータバッファへのポインタを取得

	c_EvevntDataBuf.clearCurrentBuff();										/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */

	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）
	pDst->e_Kind = e_txtKind;


	// ■ E-mail 固有 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )		// E-mail版 / HTTP版 寺内(08/01/31) 新規追加.
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。 
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindTimeOut, uc_BcdEventID );
		// 07/04/12 佐久間 E-mail片方向通信対応 
		if( True == theRdsApl.isSimplex()){									// 片方向通信の場合 
					pc_RdsSetting->setHostEventID( uc_BcdEventID );
		}

		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ 
				uc_BcdEventID,		// イベントID 
				(uchar*)"PC"		// 発信固有文字列 
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	
		
	// ■ モデム、FAX、E-mail 共通 
	// A500yymmddhhmm 
	memcpy( pBuff, "A500", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

	// yymmddhhmm 
	unsigned char		buf[10+4];												// 保険で大きめにした 
	RDSC_Time			c_TempTime;
	pc_RdsSetting->lock();														// セマフォのロック
	pc_RdsSetting->getLimitTm( buf );											// yyyymmddhhmm の形式で保存されている。 
	pc_RdsSetting->unlock();													// セマフォのアンロック 

	c_TempTime.cnvString_2_Time( buf,   RDSE_T2S_BCDyyyymmddhhmm );	// まず BCD を RDSC_Time に変換して．．． 
	c_TempTime.cnvTime_2_String( pBuff, RDSE_T2S_yymmddhhmm );		// ．．．yymmddhhmm の文字列にする。
	_INC_SIZE( &pBuff, &pDst->us_Len, 10 );


	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |=  0x00000100;	*/			// 発着信種別 期日指定発信bitON  2014/02/03 add by tomiyasu
}
/*==================================================================================================*/
//	発信トリガ処理「期日指定発信」
/*==================================================================================================*/
void RDSC_mkText_ModeTimeOut::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"期日指定発信トリガ受信" );
	cmnTrgAction();
}
/*==================================================================================================*/
//	ID の等しいデータがあればクリアする(応答の確認)
/*==================================================================================================*/
void RDSC_mkText_ModeTimeOut::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	//if( True == EpLib_strcmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )	// 04/03/24 浦崎 BCD なので、0時、等のデータであると、0x00 であるため、それをNULLとみなすため、正しく比較が出来ていなかった。 
	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)" 期日指定発信の応答受信" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す。 
		theTrgMap.resetMapByTrgAllPage( e_txtKind );	// 05/02/28 浦崎 resetMapByTrg( , 0 ) → resetMapByTrgAllPage() 
		// ID も消す。 
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}

/*==================================================================================================*/
//	期日指定発信をタイマーに登録する共通処理 
/*--------------------------------------------------------------------------------------------------*/
// 04/05/28 浦崎 新規 
/*==================================================================================================*/
Bool RDSC_mkText_ModeTimeOut::setLimTimer( RDSC_Time c_NowTime, RDSC_Time c_SetTime )
{
	// タイマ登録 
	/**/
	printEpTime( (RDSS_DetailTime)c_NowTime,			(uchar*)" RDS時計(現在時刻)" );
	/**/

	pc_RdsSetting->setLimitTimeDetectStat( False );				// 期日が設定出来た／出来ないに関わらず、この時点で検出済みをリセットする。 04/06/01 浦崎 
	if( c_SetTime <= c_NowTime )
	{
		// センターの時計の方が遅れている場合、★ 仮に ★ パラメータ不正とする。 
		EPN_DBG_MESSAGE( (uchar*)"CSRC", 0,  (uchar*)" 登録要求された時計は遅れている。" );
		return( False );										// セットする期日指定時刻が、現在時刻よりも過去だった。  
	}


	SYSS_ReqTime s_ReqTime;
	int _test_Rc;
	pc_RdsClock->getCurrentTime( &s_ReqTime.sAbsTime );


	s_ReqTime.eTimerType = SYSE_USERTC;							// 04/05/26 浦崎 SYSE_ABSOLUTE より変更。 
	s_ReqTime.sAbsTime.year  = c_SetTime.year;
	s_ReqTime.sAbsTime.month = c_SetTime.month;
	s_ReqTime.sAbsTime.day   = c_SetTime.day;
	s_ReqTime.sAbsTime.hour  = c_SetTime.hour;
	s_ReqTime.sAbsTime.min   = c_SetTime.minute;
	s_ReqTime.sAbsTime.sec   = c_SetTime.second;

	/**/
	printEpTime( (RDSS_DetailTime)c_SetTime,			(uchar*)" RDS時計(期日時刻)" );
	/**/

	
// 2020/01/30 y.hotta HeliosMLK OP_BTS-18699 ディープスリープ状態時に期日指定発信ができない Add Start.
	s_ReqTime.m_bNoWakeup = False;	// ディープスリープから復帰対象(Falseをセット).
// 2020/01/30 y.hotta HeliosMLK OP_BTS-18699 ディープスリープ状態時に期日指定発信ができない Add End.
	// 期日指定発信タイマーセット 
	if( OK != (_test_Rc = pc_RdsClock->req_notifyWhen( &s_ReqTime, tskRdsEvent.wdog_ID_EventTimer )) )
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", 0, (uchar*)" 期日指定発信タイマー登録失敗。" );
		// タイマーの登録が失敗するのは、RDS 時計よりセンターが指定した時間の方が過去のものであった場合の 
		// ようであるため、期日指定発信トリガをかける。 
		tskRdsEvent.indTrg( TYPE_RdsTrgKindTimeOut, 0 );
		return( True );
	}

	return( True );
}


/***************************************************************************************************
 *   テキスト作成処理クラス：応答発信（E-mail版ONLY） 
 ***************************************************************************************************/
/*==================================================================================================*/
//	イベントデータ作成関数「応答発信」	E-mail版のみ 
/*--------------------------------------------------------------------------------------------------*/
// 04/02/10 浦崎  KMRDS 用のコーディングを行った。 
// 04/06/04 浦崎  通信ログ作成処理のために追加した「センターからの応答発信ビット」を落としておく処理を追加。 
/*==================================================================================================*/
void RDSC_mkText_ModeResRDS::makeData( uint32_t para )
{
	RDSS_EventData*			pDst;
	register uchar*			pBuff;
	ushort					us_Len = 0;


	// 「M501yymmddhhmmss ?? ...」 


	// 応答の文字列を作成  
	uint32_t ul_TypeCode = pc_RdsSetting->getEmailResultTypeCode();
	uint32_t ul_ErrCode =  pc_RdsSetting->getEmailResultErrCode();



	// RD、WR への応答、Subject ul_TypeCode」が 0 の 
	// 場合がある??? 

	
	ul_TypeCode &= ~RDSD_ResType_Res;											// センターからの応答発信ビットを落としておく。 


	while( ul_TypeCode || ul_ErrCode )
	{
		pDst  = c_EvevntDataBuf.wt;
		pBuff = &pDst->uc_Buff[0];												// イベントデータバッファへのポインタを取得

		c_EvevntDataBuf.clearCurrentBuff();										/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */

		// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）
		// ただし、ZS コマンドの処理の場合には、「TYPE_RdsTrgKindIni」に置き換えられる。 
		// ただし、ER コマンドの処理の場合には、「？？？」に置き換えられる。 
		pDst->e_Kind = e_txtKind;

		// ホストイベントIDを取得。 
		// yymmddhhmmss 
		memcpy( pBuff, "M500", 4 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

		uchar	uc_HostEventID[6+1];
		pc_RdsSetting->getHostEventID( uc_HostEventID );

		RDSC_Time				c_rdsTime;
		c_rdsTime.cnvString_2_Time( uc_HostEventID, RDSE_T2S_BCDyymmddhhmmss );

		ushort us_TempLen = c_rdsTime.cnvTime_2_String( pBuff, RDSE_T2S_yymmddhhmmss );
		_INC_SIZE( &pBuff, &pDst->us_Len, us_TempLen );
		memcpy( pBuff, " ", 1 );											// スペース 
		_INC_SIZE( &pBuff, &pDst->us_Len, 1 );

		// 応答の文字列を作成 
		if( ul_TypeCode & RDSD_ResType_ER ) 						// "ER ",Subject の TYPE CODE 異常時の応答 
		{
			us_Len = make_ER( pBuff, ul_ErrCode );
			_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
			ul_TypeCode &= ~RDSD_ResType_ER;
			c_EvevntDataBuf.updateWtPtr();
			EPN_DBG_MESSAGE( (uchar*)"makeData 1", (ushort)ul_ErrCode, (uchar*)"  <ul_TypeCode & RDSD_ResType_ER>" );
			break;
		}
		else
		{
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			// エラー応答 
			if( 0 != ul_ErrCode )
			{
				// RDSD_ResType_ER 以外でもエラーはある。RDSD_ResType_ER は、Subjectエラーを表す。 
				us_Len = make_ER( pBuff, ul_ErrCode );
				_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
				c_EvevntDataBuf.updateWtPtr();
				EPN_DBG_MESSAGE( (uchar*)"makeData 2", (uint32_t)ul_ErrCode, (uchar*)" <0 != ul_ErrCode>" );
				break;
			}
			//~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~
			// 通常の応答 
			else
			{
				if( ul_TypeCode & RDSD_ResType_ZS ){ 				// "ZS ",ホストらからの初期接続要求の応答 
					pDst->e_Kind = TYPE_RdsTrgKindIni;				// 発信種別を初期発信にする。  
					us_Len = make_ZS( pBuff );
					_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
					ul_TypeCode &= ~RDSD_ResType_ZS;

					c_EvevntDataBuf.updateWtPtr();
				}
				// WR の RD Type 専用処理関数にてセットされる。 
				else if( ul_TypeCode & RDSD_ResType_ZW ){ 			// "ZW ",ホストからのデータ書き込み要求の応答 
					ul_TypeCode &= ~RDSD_ResType_ZW;
					//c_EvevntDataBuf.updateWtPtr(); × データは別のところで作るため、ここではライトポインタ更新しない。 
				}
				// RD の RD Type 専用処理関数にてセットされる。 
				else if( ul_TypeCode & RDSD_ResType_ZR ){ 			// "ZR ",ホストからのデータ要求の応答 
					ul_TypeCode &= ~RDSD_ResType_ZR;
					//c_EvevntDataBuf.updateWtPtr(); × データは別のところで作るため、ここではライトポインタ更新しない。 
				}
				else {
					ul_TypeCode &= ~RDSD_ResType_OtherER;
					break;															// 処理データなし 
				}
			}
		}

	}
}
/*==================================================================================================*/
//	発信トリガ処理「応答発信：ZS 作成処理」	E-mail版のみ
/*==================================================================================================*/
ushort RDSC_mkText_ModeResRDS::make_ZS( uchar* _pBuff )
{
	uchar*	pBuff;
	ushort	us_Len = 0;
	uchar	uc_Temp[_S_Adr+1];		// この関数内で使う最大値の配列を宣言 

	pBuff = _pBuff;

	// ■ 応答の種類(文字列)  
	memcpy( &pBuff[us_Len], "ZS ", 3 );
	us_Len = (ushort)(us_Len + 3);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

	// ■ センターID I000xxxxx
	memcpy( &pBuff[us_Len], "I000", 4 );
	us_Len = (ushort)(us_Len + 4);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

	pc_RdsSetting->getHostID( uc_Temp );
	memcpy( &pBuff[us_Len], uc_Temp, 5 );
	us_Len = (ushort)(us_Len + 5);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

	memcpy( &pBuff[us_Len], " ", 1 );											// スペース
	us_Len = (ushort)(us_Len + 1);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.


	// ■ ホストE-mailアドレス(E-mail)
	memcpy( &pBuff[us_Len], "M400", 4 );
	us_Len = (ushort)(us_Len + 4);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

	if( ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )		// HTTP通信の場合固定アドレスを使用する。 08/02/04 寺内　新規追加.
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		strcpy( (char*)uc_Temp, "host@csrc.konicaminolta.com");
	}
	else
	{
		pc_RdsSetting->getHostMailAdr( uc_Temp );
	}
	short	ss_Len = (short)strlen( (char*)uc_Temp );	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.
	memcpy( &pBuff[us_Len], uc_Temp, ss_Len );
	us_Len = (ushort)(us_Len + ss_Len);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

	memcpy( &pBuff[us_Len], ",", 1 );											// ","
	us_Len = (ushort)(us_Len + 1);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

	// ■ 応答タイムアウト 
	memcpy( &pBuff[us_Len], "02", 2 );
	us_Len = (ushort)(us_Len + 2);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

	uchar	timeoutValue = pc_RdsSetting->getResponseTime();
	sprintf( (char*)&pBuff[us_Len], "%03d", (int)timeoutValue );
	us_Len = (ushort)(us_Len + 3);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

	return( us_Len );
}

/*==================================================================================================*/
//	発信トリガ処理「応答発信：ER 作成処理」	E-mail版のみ
/*==================================================================================================*/
ushort RDSC_mkText_ModeResRDS::make_ER( uchar* _pBuff, uint32_t ul_ErrCode )
{
	uchar*	pBuff;
	ushort	us_Len = 0;

	pBuff = _pBuff;

	// ■ 応答の種類(文字列)  
	memcpy( &pBuff[us_Len], "ER ", 3 );
	us_Len = (ushort)(us_Len + 3);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

	// エラーコードをセット 
//	EpLib_ltoa( ul_ErrCode, &pBuff[us_Len], 2 );
	sprintf( (char*)&pBuff[us_Len], "%02X", (int)ul_ErrCode );
	us_Len = (ushort)(us_Len + 2);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.


	return( us_Len );
}

/*==================================================================================================*/
//	発信トリガ処理「応答発信」	E-mail版のみ
/*==================================================================================================*/
void RDSC_mkText_ModeResRDS::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"応答発信(RDS)トリガ受信" );
	cmnTrgAction();
}

/*==================================================================================================*/
//	発信トリガ処理「応答発信：ZS 作成処理」	E-mail版のみ
/*--------------------------------------------------------------------------------------------------*/
// 07/09/13 佐久間 送信するメールアドレスをホストアドレス→受信アドレスに変更（問題点NO.84544対応） 
/*==================================================================================================*/
ushort RDSC_mkText_ModeResRDS::make_US( uchar* _pBuff )
{
	uchar*	pBuff;
	ushort	us_Len = 0;
	uchar	uc_Temp[_S_Adr+1];		// この関数内で使う最大値の配列を宣言 
	
	pBuff = _pBuff;

	// ■ 応答の種類(文字列)  
	memcpy( &pBuff[us_Len], "US ", 3 );
	us_Len = (ushort)(us_Len +3);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

	// ■ センターID I000xxxxx
	memcpy( &pBuff[us_Len], "I000", 4 );
	us_Len = (ushort)(us_Len +4);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

	pc_RdsSetting->getHostID( uc_Temp );
	memcpy( &pBuff[us_Len], uc_Temp, 5 );
	us_Len = (ushort)(us_Len +5);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

	memcpy( &pBuff[us_Len], " ", 1 );											// スペース
	us_Len = (ushort)(us_Len +1);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.


	// ■ ホストE-mailアドレス(E-mail)
	memcpy( &pBuff[us_Len], "M403", 4 );
	us_Len = (ushort)(us_Len +4);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

	if( RDSE_TSK_ID_HTTP != theRdsApl.e_CommDevice )		// 通信形態がHTTPでない場合 
	{
		pc_RdsSetting->getPOP3MailAdr( uc_Temp );
	}
	else
	{
		strcpy( (char*)uc_Temp, "device@csrc.konicaminolta.com" );
	}
	short	ss_Len = (short)strlen( (char*)uc_Temp );	// 2010/07/17 sone Minerva warning 対応 shortにキャスト.
	memcpy( &pBuff[us_Len], uc_Temp, ss_Len );
	us_Len = (ushort)(us_Len + ss_Len);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.


	if( RDSE_TSK_ID_HTTP != theRdsApl.e_CommDevice )		// 通信形態がHTTPでない場合 
	{

		memcpy( &pBuff[us_Len], ",", 1 );											// ","
		us_Len = (ushort)(us_Len +1);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

		// ■ 応答タイムアウト 
		memcpy( &pBuff[us_Len], "02", 2 );
		us_Len = (ushort)(us_Len +2);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

		uchar	timeoutValue = pc_RdsSetting->getResponseTime();
		sprintf( (char*)&pBuff[us_Len], "%03d", (int)timeoutValue );
		us_Len = (ushort)(us_Len +3);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.
	}
	
// 2017/07/20 [MAv5][RQ-XXX] CSRA設定状態発信 Start.
	memcpy( &pBuff[us_Len], " ", 1 );
	us_Len = (ushort)(us_Len +1);

	memcpy( &pBuff[us_Len], "RV00", 4 );
	us_Len = (ushort)(us_Len +4);
// 2020/09/25 RQ-776 CWH 1-way通信方式の追加 start.
	TYPE_CSRASettingComMode e_CSRACommMode = APIC_CSRASetting::newInstance()->getCSRASettingComMode();
//	TYPS_CSRASetting s_CSRASetting = {};
//	APIC_CSRASetting::newInstance()->getCSRASetting( &s_CSRASetting );
//	if ( True == s_CSRASetting.m_b_Enable ) {
	if ( TYPE_CSRASettingComMode_TwoWay == e_CSRACommMode ) {
// 2020/09/25 RQ-776 CWH 1-way通信方式の追加 end.
		memcpy( &pBuff[us_Len], "01", 2 );
// 2020/09/25 RQ-776 CWH 1-way通信方式の追加 start.
	} else if( TYPE_CSRASettingComMode_OneWay == e_CSRACommMode ) {
		memcpy( &pBuff[us_Len], "02", 2 );
//2021/10/14 FPT-TinhNT16 【IT6-2.2MR1】 【制御構成変更_No.38】 Start.
	} else if( TYPE_CSRASettingComMode_OneWay_CN == e_CSRACommMode ) {
		memcpy( &pBuff[us_Len], "03", 2 );
//2021/10/14 FPT-TinhNT16 【IT6-2.2MR1】 【制御構成変更_No.38】 End.
	}
// 2020/09/25 RQ-776 CWH 1-way通信方式の追加 end.
	else {
		memcpy( &pBuff[us_Len], "00", 2 );
	}
	us_Len = (ushort)(us_Len +2);
// 2017/07/20 [MAv5][RQ-XXX] CSRA設定状態発信 End.

	EPN_DBG_MESSAGE( (uchar*)"make_USがコールされた", (uint32_t)0, (uchar*)&pBuff[0] );

	return( us_Len );
}

			

/***************************************************************************************************
 *   テキスト作成処理クラス：応答発信
 ***************************************************************************************************/
/*==================================================================================================*/
//	イベントデータ作成関数「応答発信」	E-mail版のみ	★ KMRDS以外用
/*==================================================================================================*/
void RDSC_mkText_ModeResOther::makeData( uint32_t para )
{
	// ダミー関数。中身無し。
}
/*==================================================================================================*/
//	発信トリガ処理「応答発信」	E-mail版のみ	★ KMRDS以外用
/*==================================================================================================*/
void RDSC_mkText_ModeResOther::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"応答発信(KMRDS以外)トリガ受信" );
	cmnTrgAction();
}

/***************************************************************************************************
 *	RDSC_TrgActAutoReTrans : KMRDS発信トリガ処理「自動再発信」
 ***************************************************************************************************/
/*==================================================================================================*/
//	イベントデータ作成関数「自動再発信」
/*==================================================================================================*/
void RDSC_mkText_AutoReTrans::makeData( uint32_t para )
{
	// ダミー関数。中身無し。
}
/*==================================================================================================*/
//	発信トリガ処理「自動再発信」
/*==================================================================================================*/
void RDSC_mkText_AutoReTrans::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"自動再発信トリガ受信" );
	cmnTrgAction();
}

/***************************************************************************************************
 *	RDSC_TrgActResTimeOut : KMRDS発信トリガ処理「応答タイムアウトによる再発信」
 ***************************************************************************************************/
/*==================================================================================================*/
//	「応答タイムアウトによる再発信」
/*==================================================================================================*/
void RDSC_mkText_ResTimeOut::makeData( uint32_t para )
{
	// ダミー関数。中身無し。
}
/*==================================================================================================*/
//	発信トリガ処理「応答タイムアウトによる再発信」
/*==================================================================================================*/
void RDSC_mkText_ResTimeOut::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"応答タイムアウトによる再発信トリガ受信" );
	/* 登録状態に関わらず受付ける */
	if(    RDSE_TSK_ID_TELL != theRdsApl.isRdsSystem()
		&& RDSE_TSK_ID_FAX != theRdsApl.isRdsSystem()    )	//【CSRC FAX対応】05/09/29 浦崎 
	{
		theRdsEmail.c_ResChk.reachTimeOut();								/* この時点で再発信済みとする */
		theTrgMap.refresh();												/* 応答タイムアウト検出→全ての要素を[新規]に移動 */
		c_FND.refresh();													/* 応答タイムアウト検出→全ての要素を[新規]に移動 */
	}
	cmnTrgAction();
}



#if 1

/***************************************************************************************************
 *   テキスト作成処理クラス：InternetISW の応答発信 
 ***************************************************************************************************/
/*==================================================================================================*/
//	イベントデータ作成関数「InternetISW の応答発信」 
/*==================================================================================================*/
void RDSC_mkText_ResInternetISW::makeData( uint32_t para )
{
	RDSS_EventData*			pDst;
	register uchar*			pBuff;

	// 応答データ作成 
	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];												// イベントデータバッファへのポインタを取得

	c_EvevntDataBuf.clearCurrentBuff();										/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */


	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）
	pDst->e_Kind = TYPE_RdsTrgKindResI_ISW;	// ★課題→通信ログの記録もな。  


	// ● M500作成。 
	// ホストイベントIDを取得。 
	// yymmddhhmmss 
	memcpy( pBuff, "M500", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

	uchar	uc_HostEventID[6+1];
	pc_RdsSetting->getHostEventID( uc_HostEventID );

	RDSC_Time				c_rdsTime;
	c_rdsTime.cnvString_2_Time( uc_HostEventID, RDSE_T2S_BCDyymmddhhmmss );

	ushort us_TempLen = c_rdsTime.cnvTime_2_String( pBuff, RDSE_T2S_yymmddhhmmss );
	_INC_SIZE( &pBuff, &pDst->us_Len, us_TempLen );
	memcpy( pBuff, " ", 1 );											// スペース 
	_INC_SIZE( &pBuff, &pDst->us_Len, 1 );



	// ● 応答用のデータ作成。  
	// 2013.12.19 H.Wada [ZeusCD][UV-01385] WP73.
	int32_t	sl_Data;
	if( True == c_FND.getReceiveWriteT6() )
	{
		//T6
		sl_Data = (int32_t)c_FND.getResT6_ErrCode();
	}
	else
	{
		//TG
		sl_Data = (int32_t)c_FND.getResTG_ErrCode();
	}
	// 2013.12.19 H.Wada [ZeusCD][UV-01385] WP73.

	if( 0 == sl_Data )
	{
		// ■ 応答の種類(文字列)  
		memcpy( pBuff, "ZD ", 3 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 3 );

		ushort us_Len = 0;
// 2010/11/25 Minerva UV-00971 kita Start.
		// TJコマンドを受け付けた場合とTGコマンドを受け付けた場合で
		// 取得する文字列を切り替える.
		if( True == c_FND.getReceiveWriteTJ() )
		{
			c_RdsIISW_TrnMgr.getReturnTJString( pBuff, &us_Len );
			
			if( '\0' == *pBuff )
			{
				EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"応答文字列なしエラー" );
			}
		}
		else
		{
			// 2013.12.19 H.Wada [ZeusCD][UV-01385] WP73.
			//TG or T6
			if( True == c_FND.getReceiveWriteT6() )
			{
				//T6
				c_FND.getReturnT6String( pBuff, &us_Len );
			}
			else
			{
				//TG
				c_FND.getReturnTGString( pBuff, &us_Len );
			}
			// 2013.12.19 H.Wada [ZeusCD][UV-01385] WP73.
		}
// 2010/11/25 Minerva UV-00971 kita End.
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)us_Len, (uchar*)"ISW 応答コード作成" );
	}
	else
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)sl_Data, (uchar*)"ISW エラー応答コード作成" );
//★課題：通常の ER 応答発信と同じでいいなら、ここで特別な処理をする必要はない。 
//★課題：ちなみに現在は、エラーコードの桁数はテキトウ。エラーコードは、I_ISWモジュールから通知されたものをそのままセットしているだけ。 

		// ■ 応答の種類(文字列)  
		memcpy( pBuff, "ER ", 3 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 3 );

		// エラーコードをセット 
		sprintf( (char*)pBuff, "%02X", (int)sl_Data );
		_INC_SIZE( &pBuff, &pDst->us_Len, 2 );
	}
	

	c_EvevntDataBuf.updateWtPtr();

}
/*==================================================================================================*/
//	発信トリガ処理「InternetISW の応答発信」
/*--------------------------------------------------------------------------------------------------*/
// 04/11/19 浦崎 初版。 
/*==================================================================================================*/
void RDSC_mkText_ResInternetISW::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"InternetISW の発信" );
	cmnTrgAction();
}
/*==================================================================================================*/
//	ID の等しいデータがあればクリアする(応答の確認)
/*==================================================================================================*/
void RDSC_mkText_ResInternetISW::chkAns( uchar* bcd_rcvEventID )
{
	// 処理なし。 
	return;
}


/***************************************************************************************************
 *   テキスト作成処理クラス：InternetISW の結果通知発信 
 ***************************************************************************************************/
/*==================================================================================================*/
//	イベントデータ作成関数「InternetISW の結果通知発信」 
/*==================================================================================================*/
void RDSC_mkText_TxResultI_ISW::makeData( uint32_t para )
{
	int32_t	sl_Data = 0;
	
	//FW強制更新の場合には強制更新のエラーを渡す.
	sl_Data = (int32_t)pc_RdsSetting->getRdsISWMode_ResultErrCode();

//************************  add start by tomiyasu 2008/05/21  **************************//

	if( RDSE_TSK_ID_TELL == theRdsApl.e_CommDevice ||
					RDSE_TSK_ID_FAX == theRdsApl.e_CommDevice )
	{

		RDSS_EventData*			pDst;
		register uchar*			pBuff;

		pDst  = c_EvevntDataBuf.wt;
		pBuff = &pDst->uc_Buff[0];											// イベントデータバッファへのポインタを取得

		c_EvevntDataBuf.clearCurrentBuff();									/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */

		// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）
		pDst->e_Kind = e_txtKind;

		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)sl_Data, (uchar*)"ISW結果通知 エラーコード作成" );

		// ■ 応答の種類(文字列)  
		memcpy( pBuff, "TF00", 4 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

		// エラーコードをセット 
		sprintf( (char*)pBuff, "%02X", (int)sl_Data );
		_INC_SIZE( &pBuff, &pDst->us_Len, 2 );
		
		// CQNo366843 S9コマンドを付加する sone 2012/07/03 START.
		// モデム通信の場合、S9コマンドの処理をここで付加する.(FAX通信の場合はFAX通信処理部で付加する).
		if( 0 == sl_Data )
		{
			if( RDSE_TSK_ID_TELL == theRdsApl.e_CommDevice )
			{
				int32_t	ul_datasize = 0;					// 書き込んだデータサイズ.
				unsigned char	uc_Commandbuf[] = {" S9"};	// Readしたいコマンド.
				unsigned char	uc_Data[1000] = {""};		// 書き込んだデータ.

				// クラスの定義(コンストラクタ用の引数はモデム用のマトリクス).
				RDSC_Inquiry mInquiry(RDSC_CommandMatrix::GetCommandMatrixForTelephony());
				mInquiry.Init( uc_Commandbuf, (int32_t)strlen((char*)uc_Commandbuf) );
				mInquiry.InquiryNonVolatile( &uc_Data[0], (int32_t)500, &(ul_datasize) );
				
				memcpy( pBuff, uc_Data, ul_datasize );
				_INC_SIZE( &pBuff, &pDst->us_Len, static_cast<ushort>(ul_datasize) );	
			}
		}
		// CQNo366843 S9コマンドを付加する sone 2012/07/03 END.

		c_EvevntDataBuf.updateWtPtr();
/*		RDS_modem_Kind |= 0x00000400;	*/			// 発着信種別 IISW結果通知発信bitON  2014/02/03 add by tomiyasu
		return;
	}
	
//************************  add end by tomiyasu 2008/05/21  **************************//

	if( 0 == sl_Data )
	{
		// 結果通知発信では S7 コマンドのリード内容を返送しないといけないが、 
		// S7 のリード内容はイベントデータバッファの最大値を超える可能性がある 
		// ため、ここでは作らず、通信処理部が送信直前でコマンドのリード処理を
		// 通す事で送信データを作成する。 
	}
	else
	{
		// エラー結果通知の準備。 
		RDSS_EventData*			pDst;
		register uchar*			pBuff;

		pDst  = c_EvevntDataBuf.wt;
		pBuff = &pDst->uc_Buff[0];											// イベントデータバッファへのポインタを取得

		c_EvevntDataBuf.clearCurrentBuff();									/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */

		// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）
		pDst->e_Kind = e_txtKind;



		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)sl_Data, (uchar*)"ISW結果通知 エラーコード作成" );


		// ■ E-mail 固有 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
		if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
		 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )		// E-mail版 / HTTP版 寺内(08/01/31) 新規追加.
		 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
		{
			// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。 
			uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
			c_FND.getTrgTime( TYPE_RdsTrgKindTxResultI_ISW, uc_BcdEventID );
			ushort us_Len = RDSC_MakeEventData::makeM501
				(
					pBuff,				// ユーザーデータバッファ 
					uc_BcdEventID,		// イベントID 
					(uchar*)"FD"		// 発信固有文字列 
				);
			_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
		}

		// ■ 応答の種類(文字列)  
		memcpy( pBuff, "ER ", 3 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 3 );

		// エラーコードをセット 
		sprintf( (char*)pBuff, "%02X", (int)sl_Data );
		_INC_SIZE( &pBuff, &pDst->us_Len, 2 );


		c_EvevntDataBuf.updateWtPtr();
	}
}
/*==================================================================================================*/
//	発信トリガ処理「InternetISW の結果通知発信」
/*--------------------------------------------------------------------------------------------------*/
// 04/11/19 浦崎 初版。 
/*==================================================================================================*/
void RDSC_mkText_TxResultI_ISW::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"InternetISW の発信" );
	cmnTrgAction();
}
/*==================================================================================================*/
//	ID の等しいデータがあればクリアする(応答の確認) 
/*==================================================================================================*/
void RDSC_mkText_TxResultI_ISW::chkAns( uchar* bcd_rcvEventID )
{
	// c_FND で処理されるため、ここでは処理しない。  
	return;
}

#endif


/***************************************************************************************************
 *   テキスト作成処理クラス：新品交換発信
 ***************************************************************************************************/
/*==================================================================================================*/
//	イベントデータ作成関数「新品交換発信」
/*==================================================================================================*/
void RDSC_mkText_ModePartsReplace::makeData( uint32_t para )
{
	RDSS_EventData*			pDst;
	register uchar*			pBuff;

	EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"新品交換発信用イベントデータ作成");
	
	// イベントデータバッファへのポインタを取得
	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];

	/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */
	c_EvevntDataBuf.clearCurrentBuff();

	// イベントデータバッファに「発信種別」をセット.
	// ただしこの情報は、RDSC_mkText_ModePartsReplaceが継承する.
	// RDSC_mkText_Modeのコンストラクタで既に変数にセットされている.
	pDst->e_Kind = e_txtKind;

	para = tskRdsEvent.e_TmpLifeIndex;

	// ■ E-mail、HTTP版CSRC固有：イベントIDの作成.
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。 
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getPartsReplaceEventID( uc_BcdEventID );

		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ 
				uc_BcdEventID,		// イベントID 
				(uchar*)"NP"		// 発信固有文字列 
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	// ■ モデム、FAX、E-mail 共通 
	memcpy( pBuff, "PP", 2 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 2 );

	// イベントデータ作成用変数を定義.
	uchar TmpPartsReplaceTime[14];
	char  TmpParameterNumber[3];		// 2014/07/14 mtoyoda CQ501391対応（バッファサイズ不足のため、要素数 2→3）.
	char  TmpPartsReplaceCntStr[9];		// 2014/07/14 mtoyoda CQ501391対応（バッファサイズ不足のため、要素数 8→9）.

	uint32_t TmpPartsReplaceCnt;

	bool  b_LastPara;
// 2010/07/17 higuchi Minerva warning 対応 Start.
//	char  c_PartsIndex;
//	char  c_PartsSubIndex;
	uchar  c_PartsIndex;
	uchar  c_PartsSubIndex;
// 2010/07/17 higuchi Minerva warning 対応 End.

	// 関数内変数を初期化.
	memset(&TmpPartsReplaceTime[0],   0x00, sizeof(TmpPartsReplaceTime)  );
	memset(&TmpParameterNumber[0],    0x00, sizeof(TmpParameterNumber)   );
	memset(&TmpPartsReplaceCntStr[0], 0x00, sizeof(TmpPartsReplaceCntStr));

	TmpPartsReplaceCnt = 0;
	b_LastPara = True;
	c_PartsIndex = 0;
	c_PartsSubIndex = 0;
	
	// まずは交換されたパーツがどれかを確認して、Bool型のクラス配列に値を設定.
	for( c_PartsIndex = 0; c_PartsIndex < TYPE_PartsLifeIndex_TotalNum - 1; c_PartsIndex++)
	{
		// データがあるかどうかを確認するため、パーツ交換日時を取得.
		// ここでオブザーバコールにより渡されたデータを見ないのは仕様としてちょっとまずいか？.
		pc_RdsSetting->getPartsReplaceDataSeparate_Time((short)(c_PartsIndex + 1), &TmpPartsReplaceTime[0]);	// 2010/07/17 sone Minerva warning 対応 shortにキャスト.
		
		//パーツ交換日時に値が設定されていれば、イベントデータに追加.
		if( 0x00 != TmpPartsReplaceTime[0] )
		{
			//ここでイベントデータを作成したパーツの直前ライフカウンタのみ、
			//NVDから消してよい。そのための制御を行うフラグをここで設定する。
			//実際に使用するのは、RDSC_mkText_ModePartsReplace::JobNormalEnd。
			b_PartsIndexTransComplete[c_PartsIndex] = True;
		}
		else
		{
			b_PartsIndexTransComplete[c_PartsIndex] = False;
		}
	}
	checkParamCommandPP();			//12/10/2020 FPT-HaDV8 [IT6_2.1] [OP_BTS-27225] Modify.
	
	// Bool配列の値に応じて、イベントデータを作成する..
	for( c_PartsIndex = 0; c_PartsIndex < TYPE_PartsLifeIndex_TotalNum - 1; c_PartsIndex++)
	{
		if ( b_PartsIndexTransComplete[c_PartsIndex] == True )
		{
			// 配列に格納された値を参照して、データを作成するかどうかを決定。
			// この際、交換されていないパーツに関してはパラメータ番号も付与しないこと、
			// 最後のパラメータにはカンマを付与しないため注意すること。

			// 例) パラメータ00と、0Dのパーツが交換された場合…
			//   PP00YYYYMMDDHHMMSS00000000,0DYYYYMMDDHHMMSS00000000
			//   ↑仮にパラメータが0Eまであっても、作成するデータの最後のパラメータである0Dの後ろにはカンマを付与しない制御が必要となる。

			// パラメータ番号を設定して、ポインタを進める.
			sprintf(&TmpParameterNumber[0], "%02X", c_PartsIndex);
			memcpy(pBuff, &TmpParameterNumber[0], sizeof(TmpParameterNumber)-1 );	// 2014/07/14 mtoyoda CQ501391対応（他の発信種別のデータ作成の方針に従い、文字列終端はコピーしないように修正）.
			_INC_SIZE( &pBuff, &pDst->us_Len, 2 );
		
			// 次に、パラメータで指定されたパーツのタイムスタンプとライフカウンタの値を設定する。
			// タイムスタンプの設定.
			pc_RdsSetting->getPartsReplaceDataSeparate_Time((short)(c_PartsIndex + 1), &TmpPartsReplaceTime[0]);	// 2010/07/17 sone Minerva warning 対応 shortにキャスト.
			memcpy(pBuff, &TmpPartsReplaceTime[0], sizeof(TmpPartsReplaceTime) );
			_INC_SIZE( &pBuff, &pDst->us_Len, 14 );

			// ライフカウンタの設定.
			TmpPartsReplaceCnt = pc_RdsSetting->getPartsReplaceDataSeparate_Cnt ((short)(c_PartsIndex + 1));	// 2010/07/17 sone Minerva warning 対応 shortにキャスト.
			sprintf(&TmpPartsReplaceCntStr[0], "%08ld", TmpPartsReplaceCnt);
			memcpy(pBuff, &TmpPartsReplaceCntStr[0], sizeof(TmpPartsReplaceCntStr)-1 );		// 2014/07/14 mtoyoda CQ501391対応（他の発信種別のデータ作成の方針に従い、文字列終端はコピーしないように修正）.
			_INC_SIZE( &pBuff, &pDst->us_Len, 8 );
			
			// 現在のパラメータ番号から、交換されたかどうかの真偽値を格納している配列を精査して、
			// もし現在のパラメータ以降にも交換されたパーツがあれば、カンマを付与する。
			// 最後のパーツであれば、問答無用でカンマは付与しない。
			b_LastPara = True;
			
			if ( c_PartsIndex < (TYPE_PartsLifeIndex_TotalNum - 1) )
			{
				for ( c_PartsSubIndex = (char)(c_PartsIndex + 1); c_PartsSubIndex < TYPE_PartsLifeIndex_TotalNum - 1; c_PartsSubIndex++)	// 2010/07/17 sone Minerva warning 対応 charにキャスト.
				{
					if ( b_PartsIndexTransComplete[c_PartsSubIndex] != False )
					{
						b_LastPara = False;
						break;
					}
				}
			}

			// 配列を精査して、残りのパラメータがあれば、パラメータをセパレートするためのカンマを付与.
			if ( b_LastPara == False )
			{
				memcpy(pBuff, ",", 1 );
				_INC_SIZE( &pBuff, &pDst->us_Len, 1 );
			}
		}
	}
	
	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |= 0x00000800;	*/			// 発着信種別 新品検出発信bitON  2014/02/03 add by tomiyasu
}
/*==================================================================================================*/
//	発信トリガ処理「新品交換発信」
/*==================================================================================================*/
void RDSC_mkText_ModePartsReplace::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"新品交換発信トリガ受信" );
	cmnTrgAction();
}
/*==================================================================================================*/
//	新品交換発信  メールIDの等しいデータがあればクリアする(応答の確認)
/*==================================================================================================*/
void RDSC_mkText_ModePartsReplace::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	pc_RdsSetting->getPartsReplaceEventID( bcd_bkEventID );

	// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消して、イベントIDも消す。 
	// さらにここでは、CSRC専用のNVD領域に確保しているライフカウンタ関連情報も初期化する。
	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)" 新品交換発信の応答受信" );
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setPartsReplaceEventID( bcd_bkEventID );

		// 新品交換発信関連の、ライフカウンタ関連情報の初期化.
		uint32_t ul_PartsLifeBeforeReplace = (uint32_t)0x00;
		uchar uc_TimeStampPartsReplace[14];
		memset(&uc_TimeStampPartsReplace[0], 0, sizeof( uc_TimeStampPartsReplace ));
		short ss_loopCnt = (short)0x00;
		
		for( ss_loopCnt = 0 ; ss_loopCnt < TYPE_PartsLifeIndex_TotalNum ; ss_loopCnt++)
		{
			// 新品交換発信のパーツの交換日時の設定.
			pc_RdsSetting->setPartsReplaceDataSeparate_Time( (short)(ss_loopCnt + 1), &uc_TimeStampPartsReplace[0]);	// 2010/07/17 sone Minerva warning 対応 shortにキャスト.
			// 新品交換発信のパーツのカウンタの設定.
			pc_RdsSetting->setPartsReplaceDataSeparate_Cnt ( (short)(ss_loopCnt + 1), ul_PartsLifeBeforeReplace);	// 2010/07/17 sone Minerva warning 対応 shortにキャスト.
		}
	}
}

/*==================================================================================================*/
//12/10/2020 FPT-HaDV8 [IT6_2.1] [OP_BTS-27225] Modify.
//Check param of command PP
/*==================================================================================================*/
void RDSC_mkText_ModePartsReplace::checkParamCommandPP()
{
	// マシンタイプ取得.
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	pc_SystemInfo->lock();
	TYPE_MachineType e_Type = pc_SystemInfo->getMachineType();
	pc_SystemInfo->unlock();
	
	b_PartsIndexTransComplete[0x09] = False;
	b_PartsIndexTransComplete[0x0C] = False;
	b_PartsIndexTransComplete[0x0F] = False;
	b_PartsIndexTransComplete[0x10] = False;
	b_PartsIndexTransComplete[0x11] = False;
	b_PartsIndexTransComplete[0x12] = False;
	b_PartsIndexTransComplete[0x14] = False;
	b_PartsIndexTransComplete[0x15] = False;
	b_PartsIndexTransComplete[0x16] = False;
	b_PartsIndexTransComplete[0x17] = False;
	b_PartsIndexTransComplete[0x18] = False;

	if ((TYPE_MachineTypeDenebMLK1 == e_Type)
		|| (TYPE_MachineTypeDenebMLK2 == e_Type)
		|| (TYPE_MachineTypeDenebMLK3 == e_Type)
		|| (TYPE_MachineTypeDenebMLK4 == e_Type)
		|| (TYPE_MachineTypeEagleBKZ == e_Type)
		|| (TYPE_MachineTypeEagleBKX == e_Type)
		|| (TYPE_MachineTypeEagleBK0 == e_Type)
		|| (TYPE_MachineTypeEagleBK1 == e_Type)
		|| (TYPE_MachineTypeEagleBK2 == e_Type)
		|| (TYPE_MachineTypeEagleBKZplus == e_Type)
//2021/03/09 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 制御構成変更_No.4 Start.
		|| (TYPE_MachineTypeMinervaSSBKX == e_Type)
		|| (TYPE_MachineTypeMinervaSSBK0_US == e_Type)
		|| (TYPE_MachineTypeMinervaSSBK0_EU == e_Type)
//2021/03/09 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 制御構成変更_No.4 End.
		|| (TYPE_MachineTypeSparrowBK0MFP == e_Type)
		|| (TYPE_MachineTypeSparrowBK1MFP == e_Type)
		|| (TYPE_MachineTypeSparrowBK0SFP == e_Type)) 
	{
		b_PartsIndexTransComplete[0x00] = False;
		b_PartsIndexTransComplete[0x01] = False;
		b_PartsIndexTransComplete[0x02] = False;
		b_PartsIndexTransComplete[0x04] = False;
		b_PartsIndexTransComplete[0x05] = False;
		b_PartsIndexTransComplete[0x06] = False;
		
		if ((TYPE_MachineTypeEagleBKZ == e_Type)
			|| (TYPE_MachineTypeEagleBKX == e_Type)
			|| (TYPE_MachineTypeEagleBK0 == e_Type)
			|| (TYPE_MachineTypeEagleBK1 == e_Type)
			|| (TYPE_MachineTypeEagleBK2 == e_Type)
			|| (TYPE_MachineTypeEagleBKZplus == e_Type)
//2021/03/09 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 制御構成変更_No.4 Start.
			|| (TYPE_MachineTypeMinervaSSBKX == e_Type)
			|| (TYPE_MachineTypeMinervaSSBK0_US == e_Type)
			|| (TYPE_MachineTypeMinervaSSBK0_EU == e_Type)
//2021/03/09 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 制御構成変更_No.4 End.
			|| (TYPE_MachineTypeSparrowBK0MFP == e_Type)
			|| (TYPE_MachineTypeSparrowBK1MFP == e_Type)
			|| (TYPE_MachineTypeSparrowBK0SFP == e_Type)) 
		{
			b_PartsIndexTransComplete[0x13] = False;
		}

		if ((TYPE_MachineTypeDenebMLK1 == e_Type)
			|| (TYPE_MachineTypeDenebMLK2 == e_Type)
			|| (TYPE_MachineTypeDenebMLK3 == e_Type)
			|| (TYPE_MachineTypeDenebMLK4 == e_Type))
		{
			b_PartsIndexTransComplete[0x0A] = False;
			b_PartsIndexTransComplete[0x0B] = False;
		}
	}
	else
	{
		b_PartsIndexTransComplete[0x13] = False;
	}

	if ((TYPE_MachineTypeEagle1 == e_Type)
		|| (TYPE_MachineTypeEagle2 == e_Type)
		|| (TYPE_MachineTypeEagle2_5 == e_Type)
		|| (TYPE_MachineTypeEagleBK1 == e_Type)
		|| (TYPE_MachineTypeEagleBK2 == e_Type)
		|| (TYPE_MachineTypeHeliosMLK2_5 == e_Type)
		|| (TYPE_MachineTypeHeliosMLK2 == e_Type)
		|| (TYPE_MachineTypeHeliosMLK3 == e_Type)
		|| (TYPE_MachineTypeHeliosMLK3_CN == e_Type)
		|| (TYPE_MachineTypeHeliosMLK2_5_CN == e_Type)
		|| (TYPE_MachineTypeHeliosMLK2_5_CNDF == e_Type)
		|| (TYPE_MachineTypeHeliosMLK2_CNDF == e_Type) ) 
	{
		b_PartsIndexTransComplete[0x0E] = False;
	}
}
/*==================================================================================================*/
//	新品交換発信  モデム、FAX、E-mail(片方向)、HTTP通信終了時の処理.
//
//	変更履歴：
//　		   2010/12/06 higuchi Minerva CQ:335919 【PPコマンドでのセグメンテーションフォルト対応】.
/*==================================================================================================*/
void RDSC_mkText_ModePartsReplace::JobNormalEnd( void )
{
	// 新品交換発信関連の、ライフカウンタ関連情報の初期化.
	uint32_t ul_PartsLifeBeforeReplace = (uint32_t)0x00;
	uchar uc_TimeStampPartsReplace[14];
	memset(&uc_TimeStampPartsReplace[0], 0, sizeof( uc_TimeStampPartsReplace ));

	short ss_loopCnt = (short)0x00;
	for( ss_loopCnt = 0 ; ss_loopCnt < (TYPE_PartsLifeIndex_TotalNum - 1) ; ss_loopCnt++)		// 2010/12/06 higuchi Minervs CQ:335919 【PPコマンドでのセグメンテーションフォルト対応】.
	{
		EPN_DBG_MESSAGE((uchar*)"DBG 新品フラグ初期化!", (uint32_t)ss_loopCnt, (uchar*)" ", (uint32_t)b_PartsIndexTransComplete[ss_loopCnt]);		// 2010/12/06 higuchi Minervs CQ:335919 【PPコマンドでのセグメンテーションフォルト対応】.
		
		if( b_PartsIndexTransComplete[ss_loopCnt] )
		{
			// 新品交換発信のパーツの交換日時の設定.
			pc_RdsSetting->setPartsReplaceDataSeparate_Time( (short)(ss_loopCnt + 1), &uc_TimeStampPartsReplace[0]);	// 2010/07/17 sone Minerva warning 対応 shortにキャスト.
			// 新品交換発信のパーツのカウンタの設定.
			pc_RdsSetting->setPartsReplaceDataSeparate_Cnt ( (short)(ss_loopCnt + 1), ul_PartsLifeBeforeReplace);	// 2010/07/17 sone Minerva warning 対応 shortにキャスト.
		}
	}
	
}
// Minerva UV-00884 MFP認証情報変更発信追加対応 sone 2010/6/21 START.
// 2011/12/01 amiya ZeusMLK UV-01170 暫定救済中のCSRCへの通知 START.
//***************************************************************************************************.
//   テキスト作成処理クラス：不良箇所切り離し発信.
//***************************************************************************************************.
//==================================================================================================.
//	イベントデータ作成関数「不良箇所切り離し発信」.
//==================================================================================================.
void RDSC_mkText_ModeTemporaryRescue::makeData(
	uint32_t para
)
{
	EPN_DBG_MESSAGE((uchar*)"MakeEvent", (uint32_t)0, (uchar*)"##### RDSC_mkText_ModeTemporaryRescue::makeData() START #####");
	RDSS_EventData*			pDst;
	register uchar*			pBuff;

	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];											// イベントデータバッファへのポインタを取得

	c_EvevntDataBuf.clearCurrentBuff();									/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */


	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）
	pDst->e_Kind = e_txtKind;


	// ■ E-mail 固有 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。 
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( e_txtKind, uc_BcdEventID );
		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ 
				uc_BcdEventID,		// イベントID 
				(uchar*)"EC"		// 発信固有文字列 
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}


	// ■ モデム、FAX、E-mail 共通 
	memcpy( pBuff, "TX", 2 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 2 );
	
	// NVDに保存しているトラブルコード＋パーツ情報を取得.
	TYPS_TemporaryRescueNotifyInfo s_TemporaryRescueNotifyInfo;
	memset( &s_TemporaryRescueNotifyInfo, 0, sizeof(TYPS_TemporaryRescueNotifyInfo) );			// 初期化.
	if( e_txtKind == TYPE_RdsTrgKindTemporaryRescue0 )
	{
		s_TemporaryRescueNotifyInfo = pc_RdsSetting->getTemporaryRescueCSRCNoticeUnit( 0 );		// index0の暫定救済保存コードとパーツを取得.
	}
	else if( e_txtKind == TYPE_RdsTrgKindTemporaryRescue1 )
	{
		s_TemporaryRescueNotifyInfo = pc_RdsSetting->getTemporaryRescueCSRCNoticeUnit( 1 );		// index1の暫定救済保存コードとパーツを取得.
	}
	else
	{
	}
	
	EPN_DBG_MESSAGE( (uchar*)"makeData()", (uint32_t)s_TemporaryRescueNotifyInfo.e_TemporaryRescue, (uchar*)"= s_TemporaryRescueNotifyInfo.e_TemporaryRescue" );
	uchar usc_Unit = 0;				// 救済するパーツに対応するパラメータ取得用変数.
	usc_Unit = convertUnit( s_TemporaryRescueNotifyInfo.e_TemporaryRescue );		// 救済するパーツに対応するパラメータ取得.
	EPN_DBG_MESSAGE( (uchar*)"makeData()", (uint32_t)usc_Unit, (uchar*)"= usc_Unit" );
	sprintf( (char*)pBuff, "%02x", usc_Unit );
	// パラメータ分をずらす.
	_INC_SIZE( &pBuff, &pDst->us_Len, 2 );
	
	// トラブルコードを取得.
	sprintf( (char*)pBuff, "%04x", s_TemporaryRescueNotifyInfo.us_TrblCode );
	EPN_DBG_MESSAGE( (uchar*)"makeData()", (uint32_t)s_TemporaryRescueNotifyInfo.us_TrblCode, (uchar*)"= s_TemporaryRescueNotifyInfo.us_TrblCode" );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );
	
	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |= 0x00200000;	*/			// 発着信種別 不良箇所切り離し通知発信bitON  2014/02/03 add by tomiyasu
}

//==================================================================================================.
//	発信トリガ処理「不良箇所切り離し発信」.
//==================================================================================================.
void RDSC_mkText_ModeTemporaryRescue::rcvTrg(
	uint32_t para
)
{
	EPN_DBG_MESSAGE( (uchar*)"MakeEvent", (uint32_t)para, (uchar*)"RDSC_mkText_ModeTemporaryRescue::rcvTrg() 不良箇所切り離し発信トリガ受信" );
	cmnTrgAction();
}

//==================================================================================================.
//	不良箇所切り離し発信  メールIDの等しいデータがあればクリアする(応答の確認)
//==================================================================================================.
void RDSC_mkText_ModeTemporaryRescue::chkAns(
	uchar* bcd_rcvEventID
)
{
	EPN_DBG_MESSAGE( (uchar*)"MakeEvent", (uint32_t)0, (uchar*)"##### RDSC_mkText_ModeTemporaryRescue::chkAns() START #####" );
	if( NULL == bcd_rcvEventID )
	{
		return;
	}
	
	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)"★★★不良箇所切り離し発信の応答受信★★★" );
		// 不良箇所切り離し発信が完了後処理.
		if( e_txtKind == TYPE_RdsTrgKindTemporaryRescue0 )	// index0.
		{
			tskRdsEvent.TemporaryRescueEndDealing( 0 );
		}
		if( e_txtKind == TYPE_RdsTrgKindTemporaryRescue1 )	// index1.
		{
			tskRdsEvent.TemporaryRescueEndDealing( 1 );
		}
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す。 
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		// ID も消す。 
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}


//==================================================================================================.
// パーツをパラメータに対応する値に変更.
//==================================================================================================.
uchar RDSC_mkText_ModeTemporaryRescue::convertUnit( TYPE_TemporaryRescueUnit e_TemporaryRescueUnit )
{
	EPN_DBG_MESSAGE( (uchar*)"MakeEvent", (uint32_t)e_TemporaryRescueUnit, (uchar*)"##### RDSC_mkText_ModeTemporaryRescue::convertUnit() START #####" );
	EPN_DBG_MESSAGE( (uchar*)"convertUnit()", (uint32_t)e_TemporaryRescueUnit, (uchar*)"= e_TemporaryRescueUnit" );
	uchar usc_Return = 0xFF;
	switch( e_TemporaryRescueUnit )
	{
		case TYPE_TemporaryRescueUnitTray1:				usc_Return = 0x00;		break;	// トレイ1.
		case TYPE_TemporaryRescueUnitTray2:				usc_Return = 0x01;		break;	// トレイ2.
		case TYPE_TemporaryRescueUnitTray3:				usc_Return = 0x02;		break;	// トレイ3.
		case TYPE_TemporaryRescueUnitTray4:				usc_Return = 0x03;		break;	// トレイ4.
		case TYPE_TemporaryRescueUnitTrayLCT:			usc_Return = 0x04;		break;	// LCT.
		case TYPE_TemporaryRescueUnitADF:				usc_Return = 0x05;		break;	// ADF.
		case TYPE_TemporaryRescueUnitPI:				usc_Return = 0x06;		break;	// PI（カバーシート装置）.
		case TYPE_TemporaryRescueUnitPK:				usc_Return = 0x07;		break;	// PK（パンチキット）.
		case TYPE_TemporaryRescueUnitPZ:				usc_Return = 0x08;		break;	// PZ（Z折り、パンチ処理装置）.
		case TYPE_TemporaryRescueUnitFolder:			usc_Return = 0x09;		break;	// 中折り、中綴じ、3つ折り装置.
		case TYPE_TemporaryRescueUnitTrayManual:		usc_Return = 0x0A;		break;	// 手差しトレイ.
		case TYPE_TemporaryRescueUnitStaple:			usc_Return = 0x0B;		break;	// ステープルユニット.
		case TYPE_TemporaryRescueUnitIR:				usc_Return = 0x0C;		break;	// スキャナー.
		case TYPE_TemporaryRescueUnitTray3_4:			usc_Return = 0x0D;		break;	// トレイ３／４.
		case TYPE_TemporaryRescueUnitPZ_PK:				usc_Return = 0x0E;		break;	// Ｚ折りユニット＋パンチキット.
// 2015.06.16 ktakahashi [MinervaSBK] KRQ-94 重送検知センサー(暫定救済) Start.
		case TYPE_TemporaryRescueUnitMultiFeedDetect:	usc_Return = 0x0F;		break;	// 重送検知センサー.
// 2015.06.16 ktakahashi [MinervaSBK] KRQ-94 重送検知センサー(暫定救済) End.
// 2015/11/04 HCL VenusMLK CSRC WBS 1433 Modification Start.
#ifdef DEF_A4_Cus
		case TYPE_TemporaryRescueUnitFinisher:			usc_Return = 0x10;		break;	// Finisher.
#endif
// 2015/11/04 HCL VenusMLK CSRC WBS 1433 Modification End.
// 2018/08/11 FPT-DucDH4 IT6_1.0 [EAGLE] 制御構成変更_No.12-2 Eagle 5051 (#1) Start.
// #if defined(DEF_A4_Cus) && defined(DEF_KRONOSS_Cus) // 2017/04/11 KronosS IT5_4.0  [Accurev ID - 1015225] HCL_Pawan Start.
		case TYPE_TemporaryRescueUnitTray2_3:			usc_Return = 0x11;		break;	//Tray2/3 
		case TYPE_TemporaryRescueUnitTray2_3_4:			usc_Return = 0x12;		break;	// Tray2/3/4.
// #endif // 2017/04/11 KronosS IT5_4.0  [Accurev ID - 1015225] HCL_Pawan End.
		case TYPE_TemporaryRescueUnitTray2_LCT:			usc_Return = 0x13;		break;	// トレイ2/LCT.
		case TYPE_TemporaryRescueUnitTray2_3_LCT:		usc_Return = 0x14;		break;	// トレイ2/3/LCT.
		case TYPE_TemporaryRescueUnitTray2_3_4_LCT:		usc_Return = 0x15;		break;	// トレイ2/3/4/LCT.
// 2018/08/11 FPT-DucDH4 IT6_1.0 [EAGLE] 制御構成変更_No.12-2 Eagle 5051 (#1) End.
// 2021/08/25 FPT-TrungHV7 [IT6_2.2][MinervaSSBK] 機器構成変更_No.25 Start.
#ifdef DEF_MinervaSSBKCus
		case TYPE_TemporaryRescueUnitTray1_2_3_4:		usc_Return = 0x16;		break;	// トレイ1/2/3/4.
#endif
// 2021/08/25 FPT-TrungHV7 [IT6_2.2][MinervaSSBK] 機器構成変更_No.25 End.
		
		default:										usc_Return = 0xFF;		break;	// 異常だから0xFFにしとく。後で2つずらすし。。。.
	}
	return usc_Return;
}
//***************************************************************************************************.
//   テキスト作成処理クラス：不良箇所切り離し解除発信.
//***************************************************************************************************.
//==================================================================================================.
//	イベントデータ作成関数「不良箇所切り離し解除発信」.
//==================================================================================================.
void RDSC_mkText_ModeTemporaryRescueRelease::makeData(
	uint32_t para
)
{
	EPN_DBG_MESSAGE((uchar*)"MakeEvent", (uint32_t)0, (uchar*)"##### RDSC_mkText_ModeTemporaryRescueRelease::makeData() START #####");
	RDSS_EventData*			pDst;
	register uchar*			pBuff;

	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];											// イベントデータバッファへのポインタを取得

	c_EvevntDataBuf.clearCurrentBuff();									/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */


	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）
	pDst->e_Kind = e_txtKind;


	// ■ E-mail 固有 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。 
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindTemporaryRescueRelease, uc_BcdEventID );
		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ 
				uc_BcdEventID,		// イベントID 
				(uchar*)"TR"		// 発信固有文字列 
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}


	// ■ モデム、FAX、E-mail 共通 
	memcpy( pBuff, "TY000", 5 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 5 );

	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |= 0x00400000;	*/			// 発着信種別 不良箇所切り離し解除通知発信bitON  2014/02/03 add by tomiyasu
}
//==================================================================================================.
//	発信トリガ処理「不良箇所切り離し解除発信」.
//==================================================================================================.
void RDSC_mkText_ModeTemporaryRescueRelease::rcvTrg(
	uint32_t para
)
{
	EPN_DBG_MESSAGE( (uchar*)"MakeEvent", (uint32_t)para, (uchar*)"RDSC_mkText_ModeTemporaryRescueRelease::rcvTrg() 不良箇所切り離し解除発信トリガ受信" );
	cmnTrgAction();
}
//==================================================================================================.
//	不良箇所切り離し解除発信  メールIDの等しいデータがあればクリアする(応答の確認)
//==================================================================================================.
void RDSC_mkText_ModeTemporaryRescueRelease::chkAns(
	uchar* bcd_rcvEventID
)
{
	EPN_DBG_MESSAGE((uchar*)"MakeEvent", (uint32_t)0, (uchar*)"##### RDSC_mkText_ModeTemporaryRescueRelease::chkAns() START #####");
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)"★★★不良箇所切り離し解除発信の応答受信★★★" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す。 
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		// ID も消す。 
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}
// 2011/12/01 amiya ZeusMLK UV-01170 暫定救済中のCSRCへの通知 END.
/***************************************************************************************************
 *   テキスト作成処理クラス：MFP認証情報変更発信
 ***************************************************************************************************/
//==================================================================================================
//	関数	：RDSC_AuthXmlGene.
//	機能	：コンストラクタ／デストラクタ.
//	概要	：自クラスの生成・消滅（消滅は未使用）.
//==================================================================================================
RDSC_AuthXmlGene::RDSC_AuthXmlGene(RDSS_AuthXmlData *p_XmlData)
{
	sl_level = -1;
	p_write = p_XmlData->ptr;
	for(int i = 0; i < AUTH_XML_LEVEL_MAX ; i++)
	{
		memset( (char*)(&str_LevelName[i][0]), 0, sizeof(char) * AUTH_XML_TAG_MAXLEN);
	}
	sl_DataSize = 0;
	bk_XmlData = p_XmlData;
}

RDSC_AuthXmlGene::~RDSC_AuthXmlGene()
{
	bk_XmlData->l_DataSize = sl_DataSize;
}

//==================================================================================================
//	関数	：write.
//	機能	：サイズ分のコピー.
//	概要	：サイズ分のコピー＆書き込みポインタを移動.
//==================================================================================================
int32_t RDSC_AuthXmlGene::write(char *p_data, int32_t l_size)
{
	// NULLポインタはエラー.
	if( NULL == p_data )
	{
		return 0;
	}
	
	// マスターへコピー.
	memcpy( p_write, p_data, l_size);	// データをコピー.
	p_write += l_size;					// ポインタを移動.
	sl_DataSize += l_size;				// データサイズ分加算.
	return l_size;
}

//==================================================================================================
//	関数	：RDSC_AuthXmlGene::startRoot().
//	機能	：rootタグのstartタグ生成.
//	概要	：.
//==================================================================================================
int32_t RDSC_AuthXmlGene::startRoot()
{
	// XMLタグの生成処理.
	char temp[AUTH_SIZE_TAG] = {0};
	int32_t l_size = 0;

	const char *XML_HEADDER = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n\n";

	// XML生成.
	sprintf(temp, "%s<%s>\n", XML_HEADDER, "ChangeNotice" );

	l_size = (int32_t)strlen(temp);	// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning

	// マスターへコピー.
	return write(temp, l_size);
}

//==================================================================================================
//	関数	：RDSC_AuthXmlGene::endRoot().
//	機能	：rootタグのendタグ生成.
//	概要	：.
//==================================================================================================
int32_t RDSC_AuthXmlGene::endRoot()
{
	// XMLタグの生成処理.
	char temp[AUTH_SIZE_TAG] = {0};
	int32_t l_size = 0;

	// XML生成.
	sprintf(temp, "</%s>\n", "ChangeNotice");

	l_size = (int32_t)strlen(temp);	// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning

	// マスターへコピー.
	return write(temp, l_size);
}

//==================================================================================================
//	関数	：RDSC_AuthXmlGene::up().
//	機能	：XMLの階層を深くし、startタグを書き込む.
//	概要	：.
//	戻り値	：書き込みをしたバイト数.
//==================================================================================================
int32_t RDSC_AuthXmlGene::up(char* tag)
{
	if( (sl_level + 1) >= AUTH_XML_LEVEL_MAX )
	{
		// 階層が最大を超えたため、エラー.
		return 0;
	}
	// 階層を１つ深くする.
	sl_level++;
	strcpy( str_LevelName[sl_level], tag );

	// XMLタグの生成処理.
	char temp[AUTH_SIZE_TAG] = {0};
	int32_t l_size = 0;

	sprintf(temp, "\t<%s>\n", tag);

	l_size = (int32_t)strlen(temp);	// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning

	// マスターへコピー.
	return write(temp, l_size);
}

//==================================================================================================
//	関数	：RDSC_AuthXmlGene::down().
//	機能	：XMLの階層を浅くし、endタグを書き込む.
//	概要	：.
//	戻り値	：書き込みをしたバイト数.
//==================================================================================================
int32_t RDSC_AuthXmlGene::down()
{
	if( sl_level < 0 )
	{
		// 階層が最小を超えたため、エラー.
		return 0;
	}
	// XMLタグの生成処理.
	char temp[AUTH_SIZE_TAG] = {0};
	int32_t l_size = 0;

	sprintf(temp, "\t</%s>\n", str_LevelName[sl_level]);

	l_size = (int32_t)strlen(temp);	// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
	
	// マスターへコピー.
	write(temp, l_size);
	
	// 階層データを初期化.
	memset( str_LevelName[sl_level], 0, sizeof(char) * AUTH_XML_TAG_MAXLEN);

	// 階層を１つ浅くする。
	sl_level--;
	return l_size;
	
}

//==================================================================================================
//	関数	：RDSC_AuthXmlGene::set().
//	機能	：XMLの1項目分のデータを書き込む(startタグ/endタグを同時に書く).
//	概要	：.
//	戻り値	：書き込みをしたバイト数.
//==================================================================================================
int32_t RDSC_AuthXmlGene::set(char* tag, char* data)
{
	// tagのNULLチェック.
	if( NULL == tag || NULL == data)
	{
		return 0;
	}
	// 文字列長のチェック.
	if( strlen(data) > (AUTH_SIZE_TAG - AUTH_XML_TAG_MAXLEN - AUTH_XML_TAG_MAXLEN) )
	{
		return 0;
	}
	
	// XMLタグの生成処理.
	char temp[AUTH_SIZE_TAG] = {0};
	int32_t l_size = 0;
	
	if( 0 == strcmp(tag,"device") 
		|| 0 == strcmp(tag,"time") )
	{
		sprintf(temp, "\t<%s>%s</%s>\n", tag ,data, tag);
	}
	else
	{
		sprintf(temp, "\t\t<%s>%s</%s>\n", tag ,data, tag);
	}
	l_size = (int32_t)strlen(temp);	// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning

	// マスターへコピー.
	return write(temp, l_size);
}

//=================================================================================
//	関数	：RDSC_AuthXmlGene::getLevel().
//	機能	：現在のLevel（XMLの階層）を取得する.
//	概要	：.
//	戻り値	：現在のLevel.
//=================================================================================
int32_t RDSC_AuthXmlGene::getLevel()
{
	return sl_level;
}

//=================================================================================
//	関数	：RDSC_AuthXmlGene::closeXML().
//	機能	：rootタグまで閉じる.
//	概要	：.
//=================================================================================
void RDSC_AuthXmlGene::closeXML()
{
	while(getLevel() >= 0)
	{
		down();
	}
}

//=================================================================================
//	関数	：RDSC_AuthXmlGene::com().
//	機能	：共通部分の生成.
//	概要	：.
//=================================================================================
void RDSC_AuthXmlGene::com()
{	
	// deviceタグの生成.
	char sc_devideID[TYPD_MAX_SerialNoPrinter] = "";			// デバイスID用(13文字).
	APIC_RdsSetting* p = APIC_RdsSetting::newInstance();
	p->lock();
	p->getMachineID( (uchar *)sc_devideID );		// A0コマンドと同じCommonAPIをコール.
	p->unlock();
	set( "device", sc_devideID );
	
	// time タグの生成.
	char sc_deviceTime[TYPD_MAX_SerialNoPrinter] = "";			// 時間用用(13文字).
	SYSS_AbsTime	deviceTime;
	SYSC_RDSTimer::getCurrentTime(&deviceTime);
	
	sprintf(sc_deviceTime, "%02d" "%02d" "%02d" "%02d" "%02d", deviceTime.year % 100, deviceTime.month, deviceTime.day, deviceTime.hour, deviceTime.min);
	
	set( "time", sc_deviceTime );
}
//==================================================================================================
//	関数	：CreateAuthChangeXml.
//	機能	：認証方式変更通知用のXMLデータの生成.
//	概要	：.
//==================================================================================================
int32_t RDSC_mkText_ModeAuthInfoChange::CreateAuthChangeXml(RDSS_AuthXmlData *p_XmlData )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"CreateAuthChangeXml" );
	RDSC_AuthXmlGene AuthChange(p_XmlData);

	// rootタグ生成.
	AuthChange.startRoot();
	
	// 共通部の生成.
	AuthChange.com();
	
	// 固有部の生成.
	// dataタグ内生成.
	AuthChange.up( "Data" );
	AuthChange.set( "Type", RDSE_Flag_DEVICE );
	AuthChange.set( "DevNotice", RDSE_Type_CHANGE_METHOD );
	
	// rootの手前までタグを閉じる.
	AuthChange.closeXML();
	// rootタグを閉じる.
	AuthChange.endRoot();

	// 成功時.
	return 1;
}

//==================================================================================================
//	関数	：CreateUserDeleteXml.
//	機能	：ユーザー削除通知用のXMLデータの生成.
//	概要	：.
//==================================================================================================
int32_t RDSC_mkText_ModeAuthInfoChange::CreateUserDeleteXml(RDSS_AuthXmlData *p_XmlData )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"CreateUserDeleteXml" );
	
	APIC_OperationSetting *pc_OperationSetting = APIC_OperationSetting::newInstance();
	
	RDSC_AuthXmlGene AuthChange(p_XmlData);
	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 START.
	TYPS_CsrcSendData s_DelUserInfo;
	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 END.
	memset( &s_DelUserInfo, 0, sizeof(s_DelUserInfo) );

	Bool b_CardIDFlag;
	
	// rootタグ生成.
	AuthChange.startRoot();
	
	// 共通部の生成.
	AuthChange.com();
	
	// 保持していたユーザ情報を取得.
	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 START.
	pc_RdsSetting->getCsrcSendData(&s_DelUserInfo);
	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 END.

// 07/28/2020 FPT-HaDV8 [IT6_2.0_MR3] [OP_BTS-25542] Start.
//	EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)s_DelUserInfo.sc_UserName);
//	EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)s_DelUserInfo.sc_CardId);
// 07/28/2020 FPT-HaDV8 [IT6_2.0_MR3] [OP_BTS-25542] End.
	
	// 固有部の生成.
	// dataタグ内生成.
	AuthChange.up( "Data" );
	AuthChange.set( "Type", RDSE_Flag_USER );
	AuthChange.set( "UsName", s_DelUserInfo.sc_UserName );
	
	// 管理者設定の認証カードIDの通知が「通知する」の場合のみXMLに付加する.
	b_CardIDFlag = pc_OperationSetting->getCardIdAddition();
	if( b_CardIDFlag == True )
	{
		// カードIDがNULLではない場合のみ(登録されている場合)はタグを生成する.
		if( s_DelUserInfo.sc_CardId[0] != '\0' )		// 2011/02/15 higuchi Minerva/Zeus 【静的解析 影響度中 CID_288474】.
		{
			AuthChange.set( "CardID", s_DelUserInfo.sc_CardId );
		}
	}
	AuthChange.set( "UsTrNotice", RDSE_Type_DELETE );
	
	// rootの手前までタグを閉じる.
	AuthChange.closeXML();
	// rootタグを閉じる.
	AuthChange.endRoot();
	
	// 成功時.
	return 1;
}

//==================================================================================================
//	関数	：CreateTrackDeleteXml.
//	機能	：部門削除通知用のXMLデータの生成.
//	概要	：.
//==================================================================================================
int32_t RDSC_mkText_ModeAuthInfoChange::CreateTrackDeleteXml(RDSS_AuthXmlData *p_XmlData )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"CreateTrackDeleteXml" );
	APIC_TrackManager *pc_TrackManager = APIC_TrackManager::newInstance();

	RDSC_AuthXmlGene AuthChange(p_XmlData);
	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 START.
	TYPS_CsrcSendData s_DelTrackInfo;
	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 END.
	memset( &s_DelTrackInfo, 0, sizeof(s_DelTrackInfo) );
	char sc_TrackNo[TAG_SIZE+1] = {0,};	// 2013/10/24 【US-20U】20万人対応.
	
	// rootタグ生成.
	AuthChange.startRoot();
	
	// 共通部の生成.
	AuthChange.com();
	
	// 保持していた部門情報を取得.
	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 START.
	pc_RdsSetting->getCsrcSendData(&s_DelTrackInfo);
	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 END.
	
	// 部門番号を文字列に変換.
	sprintf(sc_TrackNo, "%ld", s_DelTrackInfo.sl_TrackNo);	// 2013/10/23 【US-20U】20万人対応 部門は最大1000件.
	
	// 固有部の生成.
	// dataタグ内生成.
	AuthChange.up( "Data" );

	// 	部門管理モード 取得.
	TYPE_TrackMode e_TrackMode = pc_TrackManager->getTrackMode();

	if( TYPE_TrackModeOnlyPassword == e_TrackMode )
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"パスワードで管理の部門" );
		// パスワードで管理している部門の削除.
		AuthChange.set( "Type", RDSE_Flag_TRACK_NO );
		AuthChange.set( "TrNum", sc_TrackNo );
		AuthChange.set( "TrPass", s_DelTrackInfo.sc_TrackPass );
		
		EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)sc_TrackNo);
//		EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)s_DelTrackInfo.sc_TrackPass);			// 07/28/2020 FPT-HaDV8 [IT6_2.0_MR3] [OP_BTS-25542] Modified.
	}
	else
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"パスワード/部門名で管理の部門" );
		// パスワード+部門で管理している部門の削除.
		AuthChange.set( "Type", RDSE_Flag_TRACK_PA );
		AuthChange.set( "TrPass", s_DelTrackInfo.sc_TrackPass );
		AuthChange.set( "TrName", s_DelTrackInfo.sc_TrackName );
		
// 07/28/2020 FPT-HaDV8 [IT6_2.0_MR3] [OP_BTS-25542] Start.
//		EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)s_DelTrackInfo.sc_TrackPass);
//		EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)s_DelTrackInfo.sc_TrackName);
// 07/28/2020 FPT-HaDV8 [IT6_2.0_MR3] [OP_BTS-25542] End.
	}

	AuthChange.set( "UsTrNotice", RDSE_Type_DELETE );
	
	// rootの手前までタグを閉じる.
	AuthChange.closeXML();
	// rootタグを閉じる.
	AuthChange.endRoot();
	
	// 成功時.
	return 1;
}

//==================================================================================================
//	関数	：CreateUserCounterXml.
//	機能	：ユーザーカウンタクリア通知用のXMLデータの生成.
//	概要	：.
//==================================================================================================
int32_t RDSC_mkText_ModeAuthInfoChange::CreateUserCounterXml(RDSS_AuthXmlData *p_XmlData )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"CreateUserCounterXml" );
	
	APIC_OperationSetting *pc_OperationSetting = APIC_OperationSetting::newInstance();
	RDSC_AuthXmlGene AuthChange(p_XmlData);
	int32_t sl_UserNo = 0; // 2013/09/10 【US-20U】20万人対応.
	char sc_Username[UACD_MAX_UserName] = {0,};
	char sc_UserCardID[APID_MaxLengthOfUserCardId] = {0,};
	Bool b_CardIDFlag;
	int32_t sl_Count = 0; // 2013/09/10 【US-20U】20万人対応.
	char sc_Count[TAG_SIZE+1] = {0,};	// 2011/05/24 yito CQ342661 .// 2013/10/24 【US-20U】20万人対応.
	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 START.
	TYPS_CsrcSendData s_DelUserInfo;
	memset( &s_DelUserInfo, 0, sizeof(s_DelUserInfo) );
	pc_RdsSetting->getCsrcSendData(&s_DelUserInfo);
	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 END.

	// rootタグ生成.
	AuthChange.startRoot();
	
	// 共通部の生成.
	AuthChange.com();
	
	// 固有部の生成.
	// dataタグ内生成.
	// 一括クリアか判断.
	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 START.
	if( s_DelUserInfo.bl_TotalFlag )	// 一括クリア.
	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 END.
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"一括クリア" );
		AuthChange.up( "Data" );
		AuthChange.set( "Type", RDSE_Flag_USER );
		AuthChange.set( "UsTrNotice", RDSE_Type_CLEAR );
		
		// クリアされた人数取得.
		// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 START.
		sl_Count = s_DelUserInfo.sl_ClearCount; // 2013/09/10 【US-20U】20万人対応.
		// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 END.
		// 文字列に変換.
		sprintf(sc_Count, "%ld", sl_Count);
		AuthChange.set( "UsTrNumber", sc_Count );
		AuthChange.down();
	}
	else	// 個別クリア.
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"個別クリア" );
		AuthChange.up( "Data" );
		
		// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 START.
		sl_UserNo = s_DelUserInfo.sl_UserNo; // 2013/09/10 【US-20U】20万人対応.
		// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 END.
		// sl_UserNoが設定されてない場合はユーザはもういない.
		if( sl_UserNo == 0) // 2013/09/10 【US-20U】20万人対応.
		{
			EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"ユーザはいない" );
		}
		else
		{
			AuthChange.set( "Type", RDSE_Flag_USER );
			// ユーザ番号のインスタンス取得.
// 静的解析 IDNo291788 NULLチェックを追加 kita 2010/12/20 Start.
			APIC_User *pc_User = NULL;
			pc_User = APIC_User::newInstance( sl_UserNo ); // 2013/09/10 【US-20U】20万人対応.
			if( NULL == pc_User )
			{
				EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)"pc_User is null");
				return 0;	// 失敗時.
			}
// 静的解析 IDNo291788 NULLチェックを追加 kita 2010/12/20 End.
			
			pc_User->getUserNameUTF8( sc_Username );
			pc_User->getUserCardId( sc_UserCardID );
// 07/28/2020 FPT-HaDV8 [IT6_2.0_MR3] [OP_BTS-25542] Start.
//			EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)sc_Username);
//			EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)sc_UserCardID);
// 07/28/2020 FPT-HaDV8 [IT6_2.0_MR3] [OP_BTS-25542] End.
		
			AuthChange.set( "UsName", (char*)sc_Username );
			
			// 管理者設定の認証カードIDの通知が「通知する」の場合のみXMLに付加する.
			b_CardIDFlag = pc_OperationSetting->getCardIdAddition();
			if( b_CardIDFlag == True )
			{
				// カードIDがNULLではない場合のみ(登録されている場合)はタグを生成する.
				if( sc_UserCardID[0] != '\0' )			// 2011/02/15 higuchi Minerva/Zeus 【静的解析 影響度中 CID_288475】.
				{
					AuthChange.set( "CardID", sc_UserCardID );
				}
			}
			AuthChange.set( "UsTrNotice", RDSE_Type_CLEAR );
			AuthChange.down();
		}
	}
	
	// rootの手前までタグを閉じる.
	AuthChange.closeXML();
	// rootタグを閉じる.
	AuthChange.endRoot();

	// 成功時.
	return 1;
}

//==================================================================================================
//	関数	：CreateTrackCounterXml.
//	機能	：部門カウンタクリア通知用のXMLデータの生成.
//	概要	：.
//==================================================================================================
int32_t RDSC_mkText_ModeAuthInfoChange::CreateTrackCounterXml(RDSS_AuthXmlData *p_XmlData )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"CreateTrackCounterXml" );
	
	APIC_TrackManager *pc_TrackManager = APIC_TrackManager::newInstance();

	RDSC_AuthXmlGene AuthChange(p_XmlData);
	int32_t sl_TrackNo; // 2013/09/10 【US-20U】20万人対応.
// 2011/05/24 yito CQ342661 start.
	// UACが確保している領域+1バイトの保存領域を確保.
	char sc_Trackname[192+1] = {0,};
	char sc_TrackPass[192+1] = {0,};	// パスワードにウムラウト設定が可能なので、64*3=192+1byte確保.
	char sc_TrackNo[TAG_SIZE+1] = {0,};
	int32_t sl_Count = 0;
	char sc_Count[TAG_SIZE+1] = {0,};	// 2013/10/24 【US-20U】20万人対応.
// 2011/05/24 yito CQ342661 end.

	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 START.
	TYPS_CsrcSendData s_DelTrackInfo;
	memset( &s_DelTrackInfo, 0, sizeof(s_DelTrackInfo) );
	pc_RdsSetting->getCsrcSendData(&s_DelTrackInfo);
	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 END.
	
	// rootタグ生成.
	AuthChange.startRoot();
	
	// 共通部の生成.
	AuthChange.com();
	
	// 固有部の生成.
	// dataタグ内生成.
	// 一括クリアか判断.
	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 START.
	if( s_DelTrackInfo.bl_TotalFlag )	// 一括クリア.
	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 END.
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"一括クリア" );
		AuthChange.up( "Data" );
		
		// 	部門管理モード 取得.
		TYPE_TrackMode e_TrackMode = pc_TrackManager->getTrackMode();
		if( TYPE_TrackModeOnlyPassword == e_TrackMode )	// パスワードで管理.
		{
			AuthChange.set( "Type", RDSE_Flag_TRACK_NO );
		}
		else	// パスワード/部門名.
		{
			AuthChange.set( "Type", RDSE_Flag_TRACK_PA );
		}
		
		AuthChange.set( "UsTrNotice", RDSE_Type_CLEAR );
		
		// クリアされた人数取得.
		// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 START.
		sl_Count = s_DelTrackInfo.sl_ClearCount; // 2013/09/10 【US-20U】20万人対応.
		// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 END.
		// 文字列に変換.
		sprintf(sc_Count, "%ld", sl_Count);
		AuthChange.set( "UsTrNumber", sc_Count );
		AuthChange.down();
	}
	else	// 個別クリア.
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"個別クリア" );
		AuthChange.up( "Data" );
		// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 START.
		sl_TrackNo = s_DelTrackInfo.sl_TrackNo;// 2013/09/10 【US-20U】20万人対応.
		// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 END.
		// sl_TrackNoが設定されていない場合は部門はもういない.
		if( sl_TrackNo == 0 ) // 2013/09/10 【US-20U】20万人対応.
		{
			EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"部門はいない" );
		}
		else
		{
			// 部門番号を用いてその部門のインスタンスを生成.
// 静的解析 IDNo291789 NULLチェックを追加 kita 2010/12/20 Start.
			APIC_Track *pc_Track = NULL;
			pc_Track = APIC_Track::newInstance( sl_TrackNo ); // 2013/09/10 【US-20U】20万人対応.
			if( NULL == pc_Track )
			{
				EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)"pc_Track is null");				
				return 0;	// 失敗時.
			}
// 静的解析 IDNo291789 NULLチェックを追加 kita 2010/12/20 End.
			
			// 	部門管理モード 取得.
			TYPE_TrackMode e_TrackMode = pc_TrackManager->getTrackMode();
			pc_Track->getTrackPassword( sc_TrackPass );		// 部門パスワード取得.
			
			if( TYPE_TrackModeOnlyPassword == e_TrackMode )
			{
				EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"パスワードで管理" );
				
				sprintf(sc_TrackNo, "%ld", sl_TrackNo);			// 部門番号を文字列に変換. // 2013/09/10 【US-20U】20万人対応.

				AuthChange.set( "Type", RDSE_Flag_TRACK_NO );
				AuthChange.set( "TrNum", sc_TrackNo );
				AuthChange.set( "TrPass", sc_TrackPass );
				
				EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)sc_TrackNo);
//				EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)sc_TrackPass);			// 07/28/2020 FPT-HaDV8 [IT6_2.0_MR3] [OP_BTS-25542] Modified.
			}
			else
			{
				pc_Track->getTrackName( sc_Trackname );		// 部門名の取得.
				EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"パスワード/部門名で管理" );

				AuthChange.set( "Type", RDSE_Flag_TRACK_PA );
				AuthChange.set( "TrPass", sc_TrackPass );
				AuthChange.set( "TrName", sc_Trackname );
					
// 07/28/2020 FPT-HaDV8 [IT6_2.0_MR3] [OP_BTS-25542] Start.
//				EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)sc_TrackPass);
//				EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)sc_Trackname);
// 07/28/2020 FPT-HaDV8 [IT6_2.0_MR3] [OP_BTS-25542] End.
			}
			AuthChange.set( "UsTrNotice", RDSE_Type_CLEAR );
			AuthChange.down();
		}
	}
	// rootの手前までタグを閉じる.
	AuthChange.closeXML();
	// rootタグを閉じる.
	AuthChange.endRoot();

	// 成功時.
	return 1;
}

//==================================================================================================
//	関数	：CreateTrackCntChangedXml.
//	機能	：割付個数変更通知用のXMLデータの生成.
//	概要	：.
//==================================================================================================
int32_t RDSC_mkText_ModeAuthInfoChange::CreateTrackCntChangedXml(RDSS_AuthXmlData *p_XmlData )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"CreateTrackCntChangedXml" );
	
	RDSC_AuthXmlGene AuthChange(p_XmlData);

	// rootタグ生成.
	AuthChange.startRoot();
	
	// 共通部の生成.
	AuthChange.com();
	
	// 固有部の生成.
	// dataタグ内生成.
	AuthChange.up( "Data" );
	AuthChange.set( "Type", RDSE_Flag_DEVICE );
	AuthChange.set( "DevNotice", RDSE_Type_CHANGE_CNT );
	
	// rootの手前までタグを閉じる.
	AuthChange.closeXML();
	// rootタグを閉じる.
	AuthChange.endRoot();

	// 成功時.
	return 1;
}

int32_t RDSC_mkText_ModeAuthInfoChange::GetAuthChangeXML( RDSS_AuthXmlData *pst_XmlData, RDSE_NoticeType e_NoticeType  )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"GetAuthChangeXML" );
	if(pst_XmlData == NULL)
	{
		// エラーとして返却.
		return 0;
	}
	
	// RDSS_AuthXmlData構造体の初期化.
	RDSS_AuthXmlData tmp_XmlData;
	tmp_XmlData.l_DataSize = 0;
	tmp_XmlData.e_CreateXml = RDSE_NoticeType_XML_MAX;
	tmp_XmlData.ptr = (char*)malloc(sizeof(char) * AUTH_SIZE_XML_TEMPORARY);		// 暫定で100KBを確保する.
	tmp_XmlData.l_LimitSize = AUTH_SIZE_XML_TEMPORARY;
	if( NULL == tmp_XmlData.ptr )
	{
		// メモリ確保に失敗,エラーとして返却.
		EPN_DBG_MESSAGE( (uchar*)"GetAuthChangeXML", 1, (uchar*)"err1:malloc" );
		return 0;
	}
	
	
	// 配列を初期化.
	memset(tmp_XmlData.ptr, 0, sizeof(char) * AUTH_SIZE_XML_TEMPORARY);
	
	// 生成する情報で分岐する.
	switch(e_NoticeType)
	{
		case RDSE_NoticeType_AuthenticationChanged:	// 認証方式変更.
			CreateAuthChangeXml( &tmp_XmlData );
			break;
		case RDSE_NoticeType_UserDeleteChanged:	// ユーザー削除.
			CreateUserDeleteXml( &tmp_XmlData );
			break;
		case RDSE_NoticeType_TrackDeleteChange:	// 部門削除.
			CreateTrackDeleteXml( &tmp_XmlData );
			break;
		case RDSE_NoticeType_UserCounterChanged:	// ユーザーカウンタクリア.
			CreateUserCounterXml( &tmp_XmlData );
			break;
		case RDSE_NoticeType_TrackCounterChanged:	// 部門カウンタクリア.
			CreateTrackCounterXml( &tmp_XmlData );
			break;
		case RDSE_NoticeType_TrackCntChanged:	// 割付個数変更.
			CreateTrackCntChangedXml( &tmp_XmlData );
			break;
		default:
			// 不定値、エラーとして返却.
			free(tmp_XmlData.ptr);
			tmp_XmlData.ptr = NULL;
			tmp_XmlData.l_DataSize = 0;
			tmp_XmlData.l_LimitSize = 0;
			EPN_DBG_MESSAGE( (uchar*)"GetAuthChangeXML", 2, (uchar*)"err2:enum", (int32_t)e_NoticeType );
			return 0;
			break;
	}

	// 必要な領域のみ確保し、コピー.
	char *p_real_buf = (char*)malloc( tmp_XmlData.l_DataSize + 10 );  // NULL文字も考慮し、10バイト多く確保.
	int32_t real_size = tmp_XmlData.l_DataSize + 10;
	if( NULL == p_real_buf )
	{
		// メモリ確保に失敗,エラーとして返却.
		free(tmp_XmlData.ptr);
		tmp_XmlData.ptr = NULL;
		tmp_XmlData.l_DataSize = 0;
		tmp_XmlData.l_LimitSize = 0;
		EPN_DBG_MESSAGE( (uchar*)"GetAuthChangeXML", 3, (uchar*)"err3:malloc" );
		return 0;
	}
	// 配列初期化＆データコピー.
	memset(p_real_buf, 0, real_size );
	memcpy(p_real_buf, tmp_XmlData.ptr, tmp_XmlData.l_DataSize );
	free(tmp_XmlData.ptr);				// temp領域開放.
	tmp_XmlData.ptr = p_real_buf;
	tmp_XmlData.l_LimitSize = real_size;

	
	// 返却する構造体にデータコピー.
	*pst_XmlData = tmp_XmlData;
	EPN_DBG_MESSAGE( (uchar*)"GetAuthChangeXML", 100, (uchar*)"ok:malloc",  tmp_XmlData.l_DataSize, tmp_XmlData.l_LimitSize );
	return 1;	// 正常時.
}
/*==================================================================================================*/
//	イベントデータ作成関数「MFP認証情報変更発信」
/*==================================================================================================*/
void RDSC_mkText_ModeAuthInfoChange::makeData( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"トータルカウンタ発信makeData()受信" );
	RDSS_EventData*			pDst;
	register uchar*			pBuff;
	int32_t	sl_ReadCount = 0;

	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 START.
	TYPS_CsrcSendData s_CsrcSendData;
	memset(&s_CsrcSendData, 0, sizeof(s_CsrcSendData));

	pc_RdsSetting->getCsrcSendData(&s_CsrcSendData);
	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 END.

	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];												// イベントデータバッファへのポインタを取得

	c_EvevntDataBuf.clearCurrentBuff();										/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */


	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）
	pDst->e_Kind = e_txtKind;


	// ■ E-mail 固有 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。 
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindAuthInfoChange, uc_BcdEventID );
		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ 
				uc_BcdEventID,		// イベントID 
				(uchar*)"RQ"		// 発信固有文字列 
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	// ■ 全通信共通 
	memcpy( pBuff, "DY00", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );
	
	// オブザーバ通知の最初でsetしたDeifine値をgetする.
	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 START.
	RDSE_NoticeType e_Type = s_CsrcSendData.sc_NoticeType;
	// ZeusMLK CQ372378 MFP認証情報変更発信(DY)の自動再発信で、タグがブランクになる  OPS1八代 END.

	// XML作成APIをコール.
	 RDSS_AuthXmlData	mstXmlData[RDSE_NoticeType_XML_MAX];
	 memset( mstXmlData, 0x00, sizeof(mstXmlData) );		// 2011/02/07 higuchi Minerva/Zeus 【静的解析 影響度高 CID_443272】.
	 GetAuthChangeXML( &mstXmlData[(uchar)e_Type], e_Type );			// 2010/11/05 higuchi Minerva warning 対応.
	// 構造体のサイズを一度、変数に移す
	 sl_ReadCount = mstXmlData[(uchar)e_Type].l_DataSize;				// 2010/11/05 higuchi Minerva warning 対応.
	// 予約文字を変換(pBuffとサイズが必要)
	memcpy( pBuff, mstXmlData[(uchar)e_Type].ptr, sl_ReadCount );		// 2010/11/05 higuchi Minerva warning 対応.
	 RDSC_CommandMatrix::ConvertReservedToRegulated(pBuff, sl_ReadCount);
	// ポインタをずらす
	 _INC_SIZE( &pBuff, &pDst->us_Len, (ushort)sl_ReadCount );			// 2010/11/05 higuchi Minerva warning 対応.

	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |= 0x00004000;	*/			// 発着信種別 MFP認証情報変更発信bitON  2014/02/03 add by tomiyasu
}
/*==================================================================================================*/
//	発信トリガ処理「MFP認証情報変更発信」
/*==================================================================================================*/
void RDSC_mkText_ModeAuthInfoChange::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"MFP認証情報変更発信トリガ受信" );
	cmnTrgAction();
}
/*==================================================================================================*/
//	MFP認証情報変更発信  メールIDの等しいデータがあればクリアする(応答の確認)
/*==================================================================================================*/
void RDSC_mkText_ModeAuthInfoChange::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)"MFP認証情報変更発信の応答受信" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す。 
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		// ID も消す。 
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}
// Minerva UV-00884 MFP認証情報変更発信追加対応 sone 2010/6/21 END.

// Minerva UV-00781 プロダクト認証対応 sone 2010/08/27 START.
/*==================================================================================================*/
//	イベントデータ作成関数「プロダクト認証設定の変更完了通知」
/*==================================================================================================*/
void RDSC_mkText_FinishProCer::makeData( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"プロダクト認証設定の変更完了通知受信" );
	RDSS_EventData*			pDst;
	register uchar*			pBuff;

	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];												// イベントデータバッファへのポインタを取得

	c_EvevntDataBuf.clearCurrentBuff();										/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */


	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）
	pDst->e_Kind = e_txtKind;


	// ■ E-mail 固有 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )		// E-mail版 / HTTP版 寺内(08/01/31) 新規追加.
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。 
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindProductCertification, uc_BcdEventID );
		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ 
				uc_BcdEventID,		// イベントID 
				(uchar*)"AS"		// 発信固有文字列 
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	// ■ モデム、FAX、E-mail 共通 
	memcpy( pBuff, "PF00", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

	// ■ 結果を設定
	short ss_Data = 0;	// 初期値：正常.
	TYPE_ProductCertificationResult e_ProResult = pc_RdsSetting->getProductCertificationResult();
	
	if( e_ProResult == TYPE_ProductCertificationResultNG )
	{
		ss_Data = 1;	// 失敗を設定.
	}
	
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)ss_Data, (uchar*)"応答データ作成" );

	sprintf( (char*)pBuff, "%02X", ss_Data );
	_INC_SIZE( &pBuff, &pDst->us_Len, 2 );
	
	c_EvevntDataBuf.updateWtPtr();
}

/*==================================================================================================*/
//	発信トリガ処理「プロダクト認証設定の変更完了通知」
/*==================================================================================================*/
void RDSC_mkText_FinishProCer::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"プロダクト認証設定の変更完了通知トリガ受信" );
	cmnTrgAction();
}
/*==================================================================================================*/
//	プロダクト認証設定の変更完了通知  メールIDの等しいデータがあればクリアする(応答の確認)
/*==================================================================================================*/
void RDSC_mkText_FinishProCer::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)"プロダクト認証設定の変更完了通知の応答受信" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す。 
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		// ID も消す。 
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}
// Minerva UV-00781 プロダクト認証対応 sone 2010/08/27 END.

// 2010/08/24 higuchi Minerva UV-00794 【デバッグログ収集】 Start.
/*==================================================================================================*/
//	イベントデータ作成関数「デバッグログ手動蓄積完了報告発信」.
/*==================================================================================================*/
void RDSC_mkText_ModeDebugLogSave::makeData( uint32_t para )
{
	EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"デバッグログ手動蓄積完了報告発信用イベントデータ作成");
	RDSS_EventData*			pDst;
	register uchar*			pBuff;

	// イベントデータバッファへのポインタを取得.
	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];

	// バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]）.
	c_EvevntDataBuf.clearCurrentBuff();

	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）.
	pDst->e_Kind = e_txtKind;

	// ■ E-mail、HTTP版CSRC固有：イベントIDの作成.
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る.
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindDebugLogSave, uc_BcdEventID );

		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ.
				uc_BcdEventID,		// イベントID.
				(uchar*)"AS"		// 発信固有文字列.
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	// ■ 全通信共通 
	memcpy( pBuff, "AE00", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

	uint32_t ul_DebugLogSystemLogCreateResultData = RDSE_DebugLogSystemResultOK;
	// ここでNVDからデータ取得する.
	ul_DebugLogSystemLogCreateResultData = pc_RdsSetting->getDebugLogSystemLogCreateResult();

	// オブザーバ通知で受けたエラーコードをsetする.
	sprintf( (char*)pBuff, "%02X", (int)ul_DebugLogSystemLogCreateResultData );
	// ポインタをずらす
	_INC_SIZE( &pBuff, &pDst->us_Len, 2 );

	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |= 0x0008000;	*/			// 発着信種別 デバッグログ手動蓄積完了報告発信bitON  2014/02/03 add by tomiyasu
}
/*==================================================================================================*/
//	発信トリガ処理「デバッグログ手動蓄積完了報告発信」
/*==================================================================================================*/
void RDSC_mkText_ModeDebugLogSave::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"デバッグログ手動蓄積完了報告発信トリガ受信" );
	cmnTrgAction();
}
/*==================================================================================================*/
//	デバッグログ手動蓄積完了報告発信  メールIDの等しいデータがあればクリアする(応答の確認).
/*==================================================================================================*/
void RDSC_mkText_ModeDebugLogSave::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)"デバッグログ手動蓄積完了報告発信の応答受信" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す。 
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		// ID も消す。 
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}
// 2010/08/24 higuchi Minerva UV-00794 【デバッグログ収集】 End.
// 2010/07/29 yito UV-00886 Exp/Imp start.
/*==================================================================================================*/
//	イベントデータ作成関数「エクスポート・インポート完了通知発信」
/*==================================================================================================*/
void RDSC_mkText_ModeExpImpComplete::makeData( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"ExpImp完了通知makeData()受信" );
	RDSS_EventData*			pDst = NULL;
	register uchar*			pBuff = NULL;
//	int32_t					sl_ReadCount = 0;				// 2010/11/17 higuchi Minerva warning 対応.

	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];												// イベントデータバッファへのポインタを取得
	c_EvevntDataBuf.clearCurrentBuff();										/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */

	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）。
	pDst->e_Kind = e_txtKind;

	// ■ E-mail / HTTP 固有.
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindExpImpComplete, uc_BcdEventID );
		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ 
				uc_BcdEventID,		// イベントID 
				(uchar*)"AS"		// 発信固有文字列 
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	// パラメータを取得。
	uchar uc_result[TYPD_ExportImportResultMax] = {0};
	pc_RdsSetting->getExportImportResult(uc_result);	// 遠隔でのエクスポート/インポート結果 取得API.

	// ■ 全通信共通。
	char sc_ErrCode[16] = {0};
	sprintf(sc_ErrCode, "%02x", uc_result[1]);	// 1byte:エラーコード。
	
	// エクスポート.
	if(uc_result[0] == 1)
	{
		memcpy( pBuff, "EF00", 4 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

		memcpy( pBuff, sc_ErrCode, 2 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 2 );

		char sc_CheckSum[16] = {0};
		sprintf(sc_CheckSum, "%02x", uc_result[2]);
		memcpy( pBuff, sc_CheckSum, 2 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 2 );
	}
	else	// インポート.
	{
		memcpy( pBuff, "EF01", 4 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 4 );
		memcpy( pBuff, sc_ErrCode, 2 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 2 );
	}
	
	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |= 0x00020000;	*/			// 発着信種別 エクスポート・インポート完了発信bitON  2014/02/03 add by tomiyasu
}
/*==================================================================================================*/
//	発信トリガ処理「エクスポート・インポート完了通知発信」
/*==================================================================================================*/
void RDSC_mkText_ModeExpImpComplete::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"ExpImp完了通知トリガ受信" );
	cmnTrgAction();
}
/*==================================================================================================*/
//	MFP認証情報変更発信  メールIDの等しいデータがあればクリアする(応答の確認)
/*==================================================================================================*/
void RDSC_mkText_ModeExpImpComplete::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)"ExpImp完了通知の応答受信" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す。 
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );				// ID も消す。 
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}
// 2010/07/29 yito UV-00886 Exp/Imp end  .

// 2010/09/13 Minerva UV-00898追加対応 kita Start.
/*==================================================================================================*/
//	イベントデータ作成関数「トラブル解除状態通知発信」.
/*==================================================================================================*/
void RDSC_mkText_ModeTrblResetStatus::makeData( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"トラブル解除状態通知発信makeData()受信" );
	RDSS_EventData*			pDst;
	register uchar*			pBuff;
	
	pDst	= c_EvevntDataBuf.wt;
	pBuff	= &pDst->uc_Buff[0];			// イベントデータバッファへのポインタを取得.

	c_EvevntDataBuf.clearCurrentBuff();		/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */

	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）.
	pDst->e_Kind = e_txtKind;
	
	// ■ E-mail 固有 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。.
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindTrblResetStatus, uc_BcdEventID );
		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ.
				uc_BcdEventID,		// イベントID.
				(uchar*)"AS"		// 発信固有文字列.
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}
	
	memcpy( pBuff, "YX01", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );
	
	// 発生中のトラブルコード取得.
	ushort	us_tempCode = 0;
	tskRdsEvent.getFatalLevelAndCode( &us_tempCode );
	
	// バッファにトラブルコードを書き込む.
	sprintf( (char*)pBuff, "%04X", us_tempCode );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );
	
	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |= 0x00040000;	*/			// 発着信種別 トラブル解除状態通知発信bitON  2014/02/03 add by tomiyasu
}

/*==================================================================================================*/
//	発信トリガ処理「トラブル解除状態通知発信」.
/*==================================================================================================*/
void RDSC_mkText_ModeTrblResetStatus::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"トラブル解除状態通知発信--->Set" );
	
	// 発信要求マップにトラブル解除状態通知発信をセットする.
	cmnTrgAction();

	// インスタンスの取得.
	APIC_MachineSetting* pc_MachineSetting = APIC_MachineSetting::newInstance();
	
	// CSRCからトラブルリセットを行ったかのステータスをトラブル解除状態通知発信が必要な状態にする.
	pc_MachineSetting->lock();		// ロック.
	pc_MachineSetting->setCSRCTroubleReset( TYPE_CSRCRemotReset_NeedTransSutatus );
	pc_MachineSetting->unlock();	// ロック解除.
	
	// トラブル解除状態通知発信待ちタイマーのステータスをタイマー未実行にする.
	tskRdsEvent.e_TRRTransWaitTimerSt = RDSE_TrblRemotResetTransWaitTimerSt_Non;
}

/*==================================================================================================*/
//	トラブル解除状態通知発信  メールIDの等しいデータがあればクリアする(応答の確認).
/*==================================================================================================*/
void RDSC_mkText_ModeTrblResetStatus::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)"トラブル解除状態通知発信の応答受信" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す。.
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		// ID も消す。.
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}
// 2010/09/13 Minerva UV-00898追加対応 kita End.

// Minerva UV-00683 sone START.
/*==================================================================================================*/
//	イベントデータ作成関数「ライセンス機能の完了通知」
/*==================================================================================================*/
void RDSC_mkText_Result_Lic::makeData( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"ﾗｲｾﾝｽ処理の完了通知受信" );
	RDSS_EventData*			pDst;
	register uchar*			pBuff;
	
	uchar uc_Data = 0;	// 初期値：正常.
	Bool b_errorFlag = False;

	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];												// イベントデータバッファへのポインタを取得

	c_EvevntDataBuf.clearCurrentBuff();										/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */


	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）
	pDst->e_Kind = e_txtKind;


	// ■ E-mail 固有 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。 
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindResult_Lic, uc_BcdEventID );
		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ 
				uc_BcdEventID,		// イベントID 
				(uchar*)"AS"		// 発信固有文字列 
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	// ■ モデム、FAX、E-mail 共通 
	memcpy( pBuff, "UF00", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

	// ■ 結果を設定
	// 各処理種別の要求があったか確認.
	APIC_EnablerSetting*	pc_EnablerSetting = APIC_EnablerSetting::newInstance();
	TYPE_LicenseFunction e_LicenseFunctionKind = TYPE_LicenseFunctionDisable;
	TYPE_AutoLicenseSettingResult e_LicenseResult = TYPE_AutoLicenseSettingResultNormal;

	tskRdsEvent.CheckWriteUG( &e_LicenseFunctionKind );					// 2010/11/17 higuchi Minerva warning 対応.

	// 処理結果を取得.
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)e_LicenseFunctionKind, (uchar*)"WriteUGあり" );
	e_LicenseResult = pc_EnablerSetting->getAutoLicenseSettingResult( e_LicenseFunctionKind );
	
	//結果をセンター通知用に設定.
	switch( e_LicenseResult )
	{
		case TYPE_AutoLicenseSettingResultNormal:	// 通常状態.
			// ありえない.
			break;
		case TYPE_AutoLicenseSettingResultSuccess:	// 正常終了.
			uc_Data = 0;
			break;
		case TYPE_AutoLicenseSettingResultFailure:	// 失敗.
			b_errorFlag = True;
			break;
		default:
			break;
	}	
	
	if( b_errorFlag == False )	// 成功時.
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)e_LicenseResult, (uchar*)"結果：正常" );
		sprintf( (char*)pBuff, "%02X", uc_Data );
		_INC_SIZE( &pBuff, &pDst->us_Len, 2 );
	}
	else	// 失敗の場合.
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"結果：失敗" );
		Bool b_DisablerFinish = False;	// 無効化完了通知フラグ.
		APIC_Lca*	pc_Lca = APIC_Lca::newInstance();
		
		LCAS_LcaTokenCode s_LcaTokenCode;
		// 構造体の初期化.
		memset(&s_LcaTokenCode, 0 ,sizeof(LCAS_LcaTokenCode));
		
		// トークンコードリストを取得.
		pc_EnablerSetting->getErrorTokenCodeList( &s_LcaTokenCode );
		
		// リストにコードがある場合：トークンコードエラーによる失敗.
		// リストにコードがない場合：トークンコードエラー以外による失敗.
		if (s_LcaTokenCode.sc_TokenCode[0][0] == 0 )
		{
			EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"TokenCodeなし" );
			
			// LCAのAPIからエラーコードを取得.
			char sc_ErrorCode[7] = {""};
			
			pc_Lca->getErrorCodeResult( sc_ErrorCode );
			EPN_DBG_MESSAGE( (uchar*)"ErrorCode", (uint32_t)0, (uchar*)sc_ErrorCode );
			// エラーコードの文字によって作成するCSRC用のエラーコードを作成.
			switch( sc_ErrorCode[0] )
			{
				case 'L':
					switch( sc_ErrorCode[2] )
					{
						case 'E':	// LXEXXX ホストとの通信異常(通信エラー).
							uc_Data = 0xC1;
							break;
						case 'S':	// LXSXXX シリアルナンバーNGエラー.
							uc_Data = 0xC2;
							break;
						case 'C':	// LXCXXX リクエストコードNGエラー.
							uc_Data = 0xC3;
							break;
						case 'M':	// LXMXXX 無効化完了コードエラー.
							uc_Data = 0xC4;
							b_DisablerFinish = True;
							break;
						case 'R':	// LXRXXX 修復要求エラー.
							uc_Data = 0xC5;
							break;
						case 'I':	// LXIXXX LMS内部エラー.
						default:
							uc_Data = 0xC6;
							break;
					}
					break;
				case 'M':
					switch( sc_ErrorCode[2] )
					{
						case 'E':	// MXE001 ホストとの通信異常(通信エラー).
							uc_Data = 0xC1;
							break;
						// 2012/08/08 amiya CQ368727 LMS経由で機能有効化しようとしてLMSサーバーと通信が行われているのに通信エラーと表示される START.
						case 'I':	// MXI001 MFP内部エラー.
							uc_Data = 0xC7;
						// 2012/08/08 amiya CQ368727 LMS経由で機能有効化しようとしてLMSサーバーと通信が行われているのに通信エラーと表示される END.
							break;
						default:
							uc_Data = 0xC7;
							break;
					}
					break;
				default:
					uc_Data = 0xC7;
					break;
			}
			
			// sc_ErrorCodeの2文字目が'M'の場合は無効化完了の処理でエラー.
			if( sc_ErrorCode[1] == 'M' )
			{
				// 無効化完了の通知でエラーになった場合は、強制的にエラーコードを「C4」に変更する.
				EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"無効化完了の通知エラー" );
				uc_Data = 0xC4;
				b_DisablerFinish = True;
			}
			else
			{
				EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"無効化完了以外の通知エラー" );
			}
		
			sprintf( (char*)pBuff, "%02X", uc_Data );
			_INC_SIZE( &pBuff, &pDst->us_Len, 2 );
		}
		else
		{
			EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"TokenCodeあり" );
			// ①エラーコードを記載.
			uc_Data = 0xC0;
			sprintf( (char*)pBuff, "%02X", uc_Data );
			_INC_SIZE( &pBuff, &pDst->us_Len, 2 );
			
			// ****************パラメータ01以降の処理****************.
			uchar uc_CodeListCnt = 0;	// カウント用.
			while(1)
			{
				uc_CodeListCnt++;
				
				// エラーコードがNULLになるか、最大数ループしたら抜ける.
				if( s_LcaTokenCode.sc_TokenCode[uc_CodeListCnt - 1][0] == 0
					|| uc_CodeListCnt > 10 )
				{
					EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)uc_CodeListCnt, (uchar*)"すべて終了" );
					break;
				}
				
				// リスト分、データを設定.
				// ②カンマを作成.(UF00C0,)
				*pBuff = ',';
				_INC_SIZE( &pBuff, &pDst->us_Len, 1 );
				
				// ③パラメータ分を作成.
				sprintf( (char*)pBuff, "%02x", uc_CodeListCnt );
				_INC_SIZE( &pBuff, &pDst->us_Len, 2 );
				
				
				// ④エラーとなったコード分を作成.(UF00C0,01???・・・??)
				sprintf( (char*)pBuff, "%20s", s_LcaTokenCode.sc_TokenCode[uc_CodeListCnt - 1] );
				_INC_SIZE( &pBuff, &pDst->us_Len, 20 );
			}
		}
		
		// ロック解除のタイミング.
		// 基本的には失敗時にはリブートされないためロック解除を行う必要がある.
		// しかし、無効化完了コードをMFPからLMSに通知する際、エラーになった場合はロック解除を行わない.
		// True：無効化完了通知の処理、False：無効化完了以外.
		// よってFalseの場合のみ、デバイスロックの解除を行う.
		if( b_DisablerFinish == False )
		{
			APIC_SystemStatus* pc_SystemStatus = APIC_SystemStatus::newInstance();
			pc_SystemStatus->lock();
			pc_SystemStatus->setDeviceLockOn( False );		// デバイスロック解除.
			pc_SystemStatus->setAdminModeOnApp( False, TYPE_AdminModeAppOther );	// 管理者ログイン無効.
			pc_SystemStatus->unlock();
		}
	}
			
	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |= 0x00008000;	*/			// 発着信種別 ﾗｲｾﾝｽ処理の完了通知発信bitON  2014/02/03 add by tomiyasu
}

/*==================================================================================================*/
//	発信トリガ処理「ライセンス機能の完了通知」
/*==================================================================================================*/
void RDSC_mkText_Result_Lic::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"ﾗｲｾﾝｽ処理の完了通知トリガ受信" );
	cmnTrgAction();
}
/*==================================================================================================*/
//	ライセンス機能の完了通知  メールIDの等しいデータがあればクリアする(応答の確認)
/*==================================================================================================*/
void RDSC_mkText_Result_Lic::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)"ﾗｲｾﾝｽ処理の完了通知の応答受信" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す。 
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		// ID も消す。 
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}
// Minerva UV-00683 sone END.

// 2010/11/19 Minerva UV-00971 kita Start.
/*==================================================================================================*/
//	イベントデータ作成関数「InternetISW実行状態通知発信」
/*==================================================================================================*/
void RDSC_mkText_InternetISWExStatus::makeData( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"InternetISW実行状態通知発信makeData()受信" );
	
	RDSS_EventData*			pDst;
	register uchar*			pBuff;
	
	pDst	= c_EvevntDataBuf.wt;
	pBuff	= &pDst->uc_Buff[0];			// イベントデータバッファへのポインタを取得.

	c_EvevntDataBuf.clearCurrentBuff();		/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */

	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）.
	pDst->e_Kind = e_txtKind;
	
	// ■ E-mail 固有 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~.
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。.
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindInternetISWExStatus, uc_BcdEventID );
		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ.
				uc_BcdEventID,		// イベントID.
				(uchar*)"AS"		// 発信固有文字列.
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}
	
	RDSE_InternetISWExTransType	e_IISWExType;
	
	pc_RdsSetting->lock();
	e_IISWExType = pc_RdsSetting->getInternetISWExNeedTransInfo();
	pc_RdsSetting->unlock();
	
	//優先度の値が低い順から発信する.
	// ダウンロード開始通知発信			優先度：1.
	if( e_IISWExType & RDSE_InternetISWExTransType_DownLoadStart )
	{
		memcpy( pBuff, "TL000", 5 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 5 );
	}
	// ダウンロード終了通知発信			優先度：2.
	else if( e_IISWExType & RDSE_InternetISWExTransType_DownLoadEnd )
	{
		memcpy( pBuff, "TL01", 4 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 4 );
		
		char sc_CardVersion[ TYPD_CardVersionMaxLen + 1 ];			// カードバージョン取得用配列.
		unsigned short us_len = 0;									// カードバージョン文字列数.
		
		// ダウンロードしたFWのカードバージョン取得.
		memset( sc_CardVersion, 0, sizeof( sc_CardVersion ) );
		pc_RdsSetting->getInternetISWCardVersion( sc_CardVersion );
		us_len = (ushort)strlen( sc_CardVersion );	// 2011/02/10 sone Minerva warning 対応 キャスト対応.
		
		EPN_DBG_MESSAGE( (uchar*)"TL01 us_len：CardVersion", (uint32_t)us_len, (uchar*)sc_CardVersion );
		
		// カードバージョン書き込み.
		memcpy( pBuff, sc_CardVersion, us_len );
		_INC_SIZE( &pBuff, &pDst->us_Len, us_len );
	}
	// FW更新開始通知発信				優先度：3.
	else if( e_IISWExType & RDSE_InternetISWExTransType_WriteStart )
	{
		memcpy( pBuff, "TL020", 5 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 5 );
	}
	// キャンセル通知発信(ダウンロード)	優先度：4.
	else if( e_IISWExType & RDSE_InternetISWExTransType_DownLoadCancel )
	{
		memcpy( pBuff, "TL030", 5 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 5 );
	}
	// キャンセル通知発信(FW削除)		優先度：5.
	else if( e_IISWExType & RDSE_InternetISWExTransType_FirmwearDelete )
	{
		memcpy( pBuff, "TL040", 5 );
		_INC_SIZE( &pBuff, &pDst->us_Len, 5 );
	}
	else
	{
		// 発信要求無し.
		// ここに来るのはCommonAPIから関数コールされた時に.
		// 不揮発に発信種類を設定できていない.
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"発信要求種別の不揮発保存エラー" );
	}
	
	
	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |= 0x00100000;	*/			// 発着信種別 InternetISW実行状態通知発信bitON  2014/02/03 add by tomiyasu

}

/*==================================================================================================*/
//	発信トリガ処理「InternetISW実行状態通知発信」.
/*==================================================================================================*/
void RDSC_mkText_InternetISWExStatus::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"InternetISW実行状態通知発信--->Set" );
	
	// 発信要求マップにInternetISW実行状態通知発信をセットする.
	cmnTrgAction();
}

/*==================================================================================================*/
//	InternetISW実行状態通知発信  メールIDの等しいデータがあればクリアする(応答の確認)
/*==================================================================================================*/
void RDSC_mkText_InternetISWExStatus::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)"InternetISW実行状態通知発信の応答受信" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す。 
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		// ID も消す。 
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}
// 2010/11/19 Minerva UV-00971 kita End.

// 2012/03/19 higuchi Minerva/Zeus UV-01225 【デバッグログ拡張機能】 Start.
/*==================================================================================================*/
//	イベントデータ作成関数「デバッグログネット配信結果報告発信」.
/*==================================================================================================*/
void RDSC_mkText_ModeDebugLogNetResult::makeData( uint32_t para )
{
	EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"デバッグログネット配信結果報告発信用イベントデータ作成");
	RDSS_EventData*			pDst;
	register uchar*			pBuff;

	// イベントデータバッファへのポインタを取得.
	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];

	// バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]）.
	c_EvevntDataBuf.clearCurrentBuff();

	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）.
	pDst->e_Kind = e_txtKind;

	// ■ E-mail、HTTP版CSRC固有：イベントIDの作成.
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る.
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindDebugLogNetResult, uc_BcdEventID );

		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ.
				uc_BcdEventID,		// イベントID.
				(uchar*)"AS"		// 発信固有文字列.
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	// ■ 全通信共通.
	memcpy( pBuff, "AL00", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

	// NVDからデータ取得.
	uchar uc_DebugLogSystemNetDeliveryResultData = 0;		// 正常終了.
	if( RDSE_DebugLogSystemNetDeliverResult_OK != pc_RdsSetting->getDebugLogSystemNetDeliveryResult() )
	{
		uc_DebugLogSystemNetDeliveryResultData = 1;			// 異常終了.
	}

	// NVDから発生時刻を取得.
	TYPC_Time c_Time = pc_RdsSetting->getDebugLogSystemNetDeliveryDate();

	// オブザーバ通知で受けたエラーコードとタイムスタンプをセットする.
	sprintf( (char*)pBuff, "%02X" "%02d" "%02d" "%02d" "%02d" "%02d" "%02d",
			(int)uc_DebugLogSystemNetDeliveryResultData,
			(c_Time.ss_Year % 100), c_Time.sc_Month, c_Time.sc_Day, c_Time.sc_Hour, c_Time.sc_Minute, c_Time.sc_Second);

	// ポインタをずらす.
	_INC_SIZE( &pBuff, &pDst->us_Len, 14 );

	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |= 0x00800000;	*/			// 発着信種別 デバッグログネット配信報告発信bitON  2014/02/03 add by tomiyasu

}
/*==================================================================================================*/
//	発信トリガ処理「デバッグログネット配信結果報告発信」.
/*==================================================================================================*/
void RDSC_mkText_ModeDebugLogNetResult::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"デバッグログネット配信報告発信トリガ受信" );
	cmnTrgAction();
}
/*==================================================================================================*/
//	デバッグログネット配信結果報告発信  メールIDの等しいデータがあればクリアする(応答の確認).
/*==================================================================================================*/
void RDSC_mkText_ModeDebugLogNetResult::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)"デバッグログネット配信結果報告発信の応答受信" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す.
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		// ID も消す.
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}
// 2012/03/19 higuchi Minerva/Zeus UV-01225 【デバッグログ拡張機能】 End.

// 2013/12/05 OPS1前田(Sky) ZeusCD UV-01387_リモートパネルの改善(WP213) Start.
/*==================================================================================================*/
//	イベントデータ作成関数「リモートパネル起動結果発信」.
/*==================================================================================================*/
void RDSC_mkText_ModeRmpStartResult::makeData( uint32_t para )
{
	EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"リモートパネル起動結果発信用イベントデータ作成");
	RDSS_EventData*			pDst;
	register uchar*			pBuff;
	TYPS_CsrcBootResult s_CsrcBootResult;
	memset( &s_CsrcBootResult, 0, sizeof(s_CsrcBootResult) );
	
	// イベントデータバッファへのポインタを取得.
	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];

	// バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]）.
	c_EvevntDataBuf.clearCurrentBuff();

	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）.
	pDst->e_Kind = e_txtKind;

	// ■ E-mail、HTTP版CSRC固有：イベントIDの作成.
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る.
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindRmpStartResult, uc_BcdEventID );

		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ.
				uc_BcdEventID,		// イベントID.
				(uchar*)"AS"		// 発信固有文字列.
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	// ■ 全通信共通.
	memcpy( pBuff, "RH00", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

	// NVDからデータ取得.
	pc_RdsSetting->lock();
	pc_RdsSetting->getCsrcBootResult( &s_CsrcBootResult );
	pc_RdsSetting->unlock();
	
	sprintf( (char*)pBuff, "%08lx" "%010ld" "%03d" "%04d" "%02d" "%02d" "%02d" "%02d" "%02d",
			s_CsrcBootResult.ul_ErrorFact,
			s_CsrcBootResult.sl_NetworkErrorCode,
			s_CsrcBootResult.us_HTTPStatusCode,
			s_CsrcBootResult.c_ProcessTime.ss_Year,
			s_CsrcBootResult.c_ProcessTime.sc_Month,
			s_CsrcBootResult.c_ProcessTime.sc_Day,
			s_CsrcBootResult.c_ProcessTime.sc_Hour,
			s_CsrcBootResult.c_ProcessTime.sc_Minute,
			s_CsrcBootResult.c_ProcessTime.sc_Second );

	// ポインタをずらす.
	_INC_SIZE( &pBuff, &pDst->us_Len, 35 );

	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |= 0x02000000;	*/			// 発着信種別 リモートパネル起動結果発信bitON  2014/07/10 mtoyoda Minerva5th->IT5_int手動マージ対応.
}
/*==================================================================================================*/
//	発信トリガ処理「リモートパネル起動結果発信」.
/*==================================================================================================*/
void RDSC_mkText_ModeRmpStartResult::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"リモートパネル起動結果発信トリガ受信" );
	cmnTrgAction();
}
/*==================================================================================================*/
//	リモートパネル起動結果発信  メールIDの等しいデータがあればクリアする(応答の確認).
/*==================================================================================================*/
void RDSC_mkText_ModeRmpStartResult::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)"リモートパネル起動結果発信の応答受信" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す.
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		// ID も消す.
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}
// 2013/12/05 OPS1前田(Sky) ZeusCD UV-01387_リモートパネルの改善(WP213) End.

// MA2 UV-01382 「遠隔でのデバッグログ取得の改善」対応 2013/10/19 mtoyoda START.
/*==================================================================================================*/
//	イベントデータ作成関数「遠隔ログ取得結果報告発信」.
/*==================================================================================================*/
void RDSC_mkText_ModeRemoteLogResult::makeData( uint32_t para )
{
	EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"遠隔ログ取得結果報告発信用イベントデータ作成");
	RDSS_EventData*			pDst;
	register uchar*			pBuff;
	char					sc_Result;
	TYPC_Time				c_Time;

	// イベントデータバッファへのポインタを取得.
	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];

	// バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]）.
	c_EvevntDataBuf.clearCurrentBuff();

	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）.
	pDst->e_Kind = e_txtKind;

	// ■ E-mail、HTTP版CSRC固有：イベントIDの作成.
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る.
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindRemoteLogResult, uc_BcdEventID );

		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ.
				uc_BcdEventID,		// イベントID.
				(uchar*)"AS"		// 発信固有文字列.
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	// ■ 全通信共通.
	memcpy( pBuff, "YS00", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

	// NVDからデータ取得（処理結果と処理日時）.
	pc_RdsSetting->lock();
	sc_Result = pc_RdsSetting->getDebugLogRemoteResult();
	c_Time    = pc_RdsSetting->getDebugLogRemoteResultTime();
	pc_RdsSetting->unlock();

	sprintf( (char*)pBuff, "%02X" "%04d" "%02d" "%02d" "%02d" "%02d" "%02d",
			(int)sc_Result,
			c_Time.ss_Year, c_Time.sc_Month, c_Time.sc_Day, c_Time.sc_Hour, c_Time.sc_Minute, c_Time.sc_Second);

	// ポインタをずらす.
	_INC_SIZE( &pBuff, &pDst->us_Len, 16 );

	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |= 0x01000000;	*/			// 発着信種別 遠隔ログ取得処理結果報告発信bitON  2014/07/10 mtoyoda Minerva5th->IT5_int手動マージ対応.
}
/*==================================================================================================*/
//	発信トリガ処理「遠隔ログ取得結果報告発信」.
/*==================================================================================================*/
void RDSC_mkText_ModeRemoteLogResult::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"遠隔ログ取得結果報告発信トリガ受信" );
	cmnTrgAction();
}
/*==================================================================================================*/
//	遠隔ログ取得結果報告発信  メールIDの等しいデータがあればクリアする(応答の確認).
/*==================================================================================================*/
void RDSC_mkText_ModeRemoteLogResult::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};	// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434984.
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)"遠隔ログ取得結果報告発信の応答受信" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す.
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		// ID も消す.
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}
// MA2 UV-01382 「遠隔でのデバッグログ取得の改善」対応 2013/10/19 mtoyoda END.

// 2014/09/03 ktakahashi ZeusS 制御構成変更 No.4 HDDミラーリング Start.
/*==================================================================================================*/
//	イベントデータ作成関数「冗長化パーツ警告発信」.
/*==================================================================================================*/
void RDSC_mkText_ModeWarnRedunParts::makeData( uint32_t para )
{
	EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"冗長化パーツ警告発信用イベントデータ作成");
	RDSS_EventData*			pDst;
	register uchar*			pBuff;

	// イベントデータバッファへのポインタを取得.
	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];

	// バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]）.
	c_EvevntDataBuf.clearCurrentBuff();

	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）.
	pDst->e_Kind = e_txtKind;

	// ■ E-mail、HTTP版CSRC固有：イベントIDの作成.
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る.
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindWarnRedunParts, uc_BcdEventID );

		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ.
				uc_BcdEventID,		// イベントID.
				(uchar*)"WO"		// 発信固有文字列.
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	// ■ 全通信共通.
	memcpy( pBuff, "RR00", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

	// NVDからデータ取得（冗長化パーツ警告発信種別）.
	uchar	uc_Parts = 0;
	pc_RdsSetting->lock();
	uc_Parts = pc_RdsSetting->getRedundancyPartsWarning();
	pc_RdsSetting->unlock();

	sprintf( (char*)pBuff, "%02d", uc_Parts );
	_INC_SIZE( &pBuff, &pDst->us_Len, 2 );		// ポインタをずらす.

	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |= 0x04000000;	*/			// 発着信種別 冗長化パーツ警告発信bitON.
}
/*==================================================================================================*/
//	発信トリガ処理「冗長化パーツ警告発信」.
/*==================================================================================================*/
void RDSC_mkText_ModeWarnRedunParts::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"冗長化パーツ警告発信トリガ受信" );
	cmnTrgAction();
}
/*==================================================================================================*/
//	冗長化パーツ警告発信  メールIDの等しいデータがあればクリアする(応答の確認).
/*==================================================================================================*/
void RDSC_mkText_ModeWarnRedunParts::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)"冗長化パーツ警告発信の応答受信" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す.
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		// ID も消す.
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}

/*==================================================================================================*/
//	イベントデータ作成関数「冗長化パーツ警告解除発信」.
/*==================================================================================================*/
void RDSC_mkText_ModeWarnRecvRedunParts::makeData( uint32_t para )
{
	EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"冗長化パーツ警告解除発信用イベントデータ作成");
	RDSS_EventData*			pDst;
	register uchar*			pBuff;

	// イベントデータバッファへのポインタを取得.
	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];

	// バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]）.
	c_EvevntDataBuf.clearCurrentBuff();

	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）.
	pDst->e_Kind = e_txtKind;

	// ■ E-mail、HTTP版CSRC固有：イベントIDの作成.
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る.
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindWarnRecvRedunParts, uc_BcdEventID );

		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ.
				uc_BcdEventID,		// イベントID.
				(uchar*)"WC"		// 発信固有文字列.
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	// ■ 全通信共通.
	memcpy( pBuff, "RS00", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

	// NVDからデータ取得（冗長化パーツ警告解除発信種別）.
	uchar	uc_Parts = 0;
	pc_RdsSetting->lock();
	uc_Parts = pc_RdsSetting->getRedundancyPartsRecovery();
	pc_RdsSetting->unlock();

	sprintf( (char*)pBuff, "%02d", uc_Parts );
	_INC_SIZE( &pBuff, &pDst->us_Len, 2 );		// ポインタをずらす.

	c_EvevntDataBuf.updateWtPtr();
/*	RDS_modem_Kind |= 0x08000000;	*/			// 発着信種別 冗長化パーツ警告解除発信bitON.
}
/*==================================================================================================*/
//	発信トリガ処理「冗長化パーツ警告解除発信」.
/*==================================================================================================*/
void RDSC_mkText_ModeWarnRecvRedunParts::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"冗長化パーツ警告解除発信トリガ受信" );
	cmnTrgAction();
}
/*==================================================================================================*/
//	冗長化パーツ警告解除発信  メールIDの等しいデータがあればクリアする(応答の確認).
/*==================================================================================================*/
void RDSC_mkText_ModeWarnRecvRedunParts::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)"冗長化パーツ警告解除発信の応答受信" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す.
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		// ID も消す.
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}
// 2014/09/03 ktakahashi ZeusS 制御構成変更 No.4 HDDミラーリング End.

// 2015/06/05 mtoyoda Helios KRQ-101_認証機能有効化対応 START.
/*==================================================================================================*/
//	イベントデータ作成関数「遠隔ログ取得結果報告発信」.
/*==================================================================================================*/
void RDSC_mkText_ModeAuthFunctionError::makeData( uint32_t para )
{
	EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"RDSC_mkText_ModeAuthFunctionError::makeData");
	RDSS_EventData*			pDst = NULL;
	register uchar*			pBuff = NULL;
	uchar					uc_Result = RDSE_AuthFunctionErrorNone;

	// イベントデータバッファへのポインタを取得.
	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];

	// バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]）.
	c_EvevntDataBuf.clearCurrentBuff();

	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）.
	pDst->e_Kind = e_txtKind;

	// ■ E-mail、HTTP版CSRC固有：イベントIDの作成.
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る.
		uchar uc_BcdEventID[6+1] = { 0 };					// 2020/10/01 FPT-LucNH2 fix bug OP_BTS-27867.
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindAuthFunctionError, uc_BcdEventID );

		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ.
				uc_BcdEventID,		// イベントID.
				(uchar*)"AS"		// 発信固有文字列.
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	// ■ 全通信共通.
	memcpy( pBuff, "UO00", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

	// エラーコードを取得する.
	// CSRCで発生しうるエラーコードは1種類のみのため、エラーコードを不揮発に保持する必要がない .
	// エラーが発生した場合（「TYPE_RdsTrgKindAuthFunctionError」のトリガーが起こされた状態）、.
	// getAuthFunctionErrorCode()からは固定値（RDSE_AuthFunctionErrorInspectFailed : 0x01）が  .
	// 返ってくる.
	uc_Result = tskRdsEvent.getAuthFunctionErrorCode();

	sprintf( (char*)pBuff, "%02X", uc_Result );

	// ポインタをずらす.
	_INC_SIZE( &pBuff, &pDst->us_Len, 2 );

	c_EvevntDataBuf.updateWtPtr();
}
/*==================================================================================================*/
//	発信トリガ処理「認証機能有効化のエラー通知発信」.
/*==================================================================================================*/
void RDSC_mkText_ModeAuthFunctionError::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"RDSC_mkText_ModeAuthFunctionError::rcvTrg" );
	cmnTrgAction();
}
/*==================================================================================================*/
//	認証機能有効化のエラー通知発信  メールIDの等しいデータがあればクリアする(応答の確認).
/*==================================================================================================*/
void RDSC_mkText_ModeAuthFunctionError::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)"RDSC_mkText_ModeAuthFunctionError::chkAns" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す.
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		// ID も消す.
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}
// 2015/06/05 mtoyoda Helios KRQ-101_認証機能有効化対応 END.

// 2016/09/08 T.Komatsu IT5_4.0 RQ-505_バックアップ機能強化(WP267) START.
/*==================================================================================================*/
//	イベントデータ作成関数「データバックアップエラー通知発信」.
/*==================================================================================================*/
void RDSC_mkText_ExtMfpBackupServerInfo::makeData( uint32_t para )
{
	EPN_DBG_MESSAGE((uchar*)"DBG", 0, (uchar*)"RDSC_mkText_ExtMfpBackupServerInfo::makeData");
	RDSS_EventData*			pDst = NULL;
	register uchar*			pBuff = NULL;

	// イベントデータバッファへのポインタを取得.
	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];

	// バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]）.
	c_EvevntDataBuf.clearCurrentBuff();

	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）.
	pDst->e_Kind = e_txtKind;

	// ■ E-mail、HTTP版CSRC固有：イベントIDの作成.
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る.
		uchar uc_BcdEventID[6+1] = { 0 };
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindExtMfpBackupServerInfo, uc_BcdEventID );

		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ.
				uc_BcdEventID,		// イベントID.
				(uchar*)"AS"		// 発信固有文字列.
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	// ■ 全通信共通.
	memcpy( pBuff, "UQ00", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );
	
	// NVDからデータ取得(エラーコードと発生日時).
	TYPS_RdsBackupServerInfo st_ServerInfo = { 0 };
	
	pc_RdsSetting->lock();
	pc_RdsSetting->getRdsBackupServerInfo( &st_ServerInfo );
	pc_RdsSetting->unlock();
	
	sprintf( (char*)pBuff, "%02X" "%04d" "%02d" "%02d" "%02d" "%02d",
			st_ServerInfo.uc_ErrorCode,
			st_ServerInfo.c_Date.ss_Year,
			st_ServerInfo.c_Date.sc_Month,
			st_ServerInfo.c_Date.sc_Day,
			st_ServerInfo.c_Date.sc_Hour,
			st_ServerInfo.c_Date.sc_Minute );
	
	_INC_SIZE( &pBuff, &pDst->us_Len, 14 );		// ポインタをずらす.

	c_EvevntDataBuf.updateWtPtr();
}
/*==================================================================================================*/
//	発信トリガ処理「データバックアップエラー通知発信」.
/*==================================================================================================*/
void RDSC_mkText_ExtMfpBackupServerInfo::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", para, (uchar*)"RDSC_mkText_ExtMfpBackupServerInfo::rcvTrg" );
	cmnTrgAction();
}
/*==================================================================================================*/
//	データバックアップエラー通知発信  メールIDの等しいデータがあればクリアする(応答の確認).
/*==================================================================================================*/
void RDSC_mkText_ExtMfpBackupServerInfo::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = { 0 };
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)"RDSC_mkText_ExtMfpBackupServerInfo::chkAns" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す.
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		// ID も消す.
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}
// 2016/09/08 T.Komatsu IT5_4.0 RQ-505_バックアップ機能強化(WP267) END.

// 2017/07/20 [MAv5][RQ-XXX] CSRA設定状態発信 Start.
/*==================================================================================================*/
//	イベントデータ作成関数「CSRA設定状態発信」.
/*==================================================================================================*/
void RDSC_mkText_ModeCsraSettingState::makeData( uint32_t para )
{
	EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"CSRA設定状態発信用イベントデータ作成");
	RDSS_EventData*			pDst = NULL;
	register uchar*			pBuff = NULL;
	
	// イベントデータバッファへのポインタを取得.
	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];

	// バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]）.
	c_EvevntDataBuf.clearCurrentBuff();

	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）.
	pDst->e_Kind = e_txtKind;

	// ■ E-mail、HTTP版CSRC固有：イベントIDの作成.
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る.
		uchar uc_BcdEventID[6+1] = {0};
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindCsraSettingState, uc_BcdEventID );

		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ.
				uc_BcdEventID,		// イベントID.
				(uchar*)"AS"		// 発信固有文字列.
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	// ■ 全通信共通.
	memcpy( pBuff, "RV00", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );
// 2020/09/25 RQ-776 CWH 1-way通信方式の追加 start.
	TYPE_CSRASettingComMode e_CSRACommMode = APIC_CSRASetting::newInstance()->getCSRASettingComMode();
//	TYPS_CSRASetting s_CSRASetting = {};
//	APIC_CSRASetting::newInstance()->getCSRASetting( &s_CSRASetting );
//	if ( True == s_CSRASetting.m_b_Enable ) {
	if ( TYPE_CSRASettingComMode_TwoWay == e_CSRACommMode ) {
// 2020/09/25 RQ-776 CWH 1-way通信方式の追加 end.
		memcpy( pBuff, "01", 2 );
// 2020/09/25 RQ-776 CWH 1-way通信方式の追加 start.
	} else if( TYPE_CSRASettingComMode_OneWay == e_CSRACommMode ) {
		memcpy( pBuff, "02", 2 );
//2021/10/14 FPT-TinhNT16 【IT6-2.2MR1】 【制御構成変更_No.38】 Start.
	} else if( TYPE_CSRASettingComMode_OneWay_CN == e_CSRACommMode ) {
		memcpy( pBuff, "03", 2 );
//2021/10/14 FPT-TinhNT16 【IT6-2.2MR1】 【制御構成変更_No.38】 End.
	}
// 2020/09/25 RQ-776 CWH 1-way通信方式の追加 end.
	else {
		memcpy( pBuff, "00", 2 );
	}
	_INC_SIZE( &pBuff, &pDst->us_Len, 2 );

	c_EvevntDataBuf.updateWtPtr();
}
/*==================================================================================================*/
//	発信トリガ処理「CSRA設定状態発信」.
/*==================================================================================================*/
void RDSC_mkText_ModeCsraSettingState::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"CSRA設定状態発信トリガ受信" );
	cmnTrgAction();
}
/*==================================================================================================*/
//	CSRA設定状態発信  メールIDの等しいデータがあればクリアする(応答の確認).
/*==================================================================================================*/
void RDSC_mkText_ModeCsraSettingState::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)"CSRA設定状態発信の応答受信" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す.
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		// ID も消す.
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}
// 2017/07/20 [MAv5][RQ-XXX] CSRA設定状態発信 End.

// 2017/08/08 [MAv5][RQ-XXX] CSRC・CSRAデータチェック Start.
/*==================================================================================================*/
//	イベントデータ作成関数「コマンド生成エラー通知」.
/*==================================================================================================*/
void RDSC_mkText_ModeCmdGenError::makeData( uint32_t para )
{
	EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"コマンド生成エラー通知用イベントデータ作成");
	RDSS_EventData*			pDst = NULL;
	register uchar*			pBuff = NULL;
	
	// イベントデータバッファへのポインタを取得.
	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];

	// バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]）.
	c_EvevntDataBuf.clearCurrentBuff();

	// イベントデータバッファに「発信種別」をセット（コンストラクタで変数にセットされている）.
	pDst->e_Kind = e_txtKind;

	// ■ E-mail、HTTP版CSRC固有：イベントIDの作成.
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )
	{
		// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る.
		uchar uc_BcdEventID[6+1] = {0};
		pc_RdsSetting->getTrgTime( e_txtKind, uc_BcdEventID );
		ushort us_Len = RDSC_MakeEventData::makeM501
			(
				pBuff,				// ユーザーデータバッファ.
				uc_BcdEventID,		// イベントID.
				(uchar*)"AS"		// 発信固有文字列.
			);
		_INC_SIZE( &pBuff, &pDst->us_Len, us_Len );
	}

	// ■ 全通信共通.
	int wsize = 0;
	uchar uc_ErrKind = 0;
	RDSC_CmdGenError *pc_CmdGenError = NULL;
	switch( e_txtKind ) {
	case TYPE_RdsTrgKindCmdGenError_CSRC:
		pc_CmdGenError = &tskRdsEvent.m_CmdGenError_CSRC;
		uc_ErrKind = 0;
		break;
	case TYPE_RdsTrgKindCmdGenError_CSRA_Daily:
		pc_CmdGenError = &tskRdsEvent.m_CmdGenError_CSRA_Daily;
		uc_ErrKind = 1;
		break;
	case TYPE_RdsTrgKindCmdGenError_CSRA_Weekly:
		pc_CmdGenError = &tskRdsEvent.m_CmdGenError_CSRA_Weekly;
		uc_ErrKind = 1;
		break;
	case TYPE_RdsTrgKindCmdGenError_CSRA_Monthly:
		pc_CmdGenError = &tskRdsEvent.m_CmdGenError_CSRA_Monthly;
		uc_ErrKind = 1;
		break;
	default:
		break;
	}

	if ( NULL != pc_CmdGenError ) {

		pc_CmdGenError->lock();

		memcpy( pBuff, "RW00", 4 );													// Command(RW)+パラメータ(00).
		_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

		wsize = sprintf( (char*)pBuff, "%s", pc_CmdGenError->getFileName() );		//	エラー発生ファイル名.
		_INC_SIZE( &pBuff, &pDst->us_Len, static_cast<ushort>(wsize) );

		memcpy( pBuff, ",01", 3 );													// パラメータ(01).
		_INC_SIZE( &pBuff, &pDst->us_Len, 3 );
		wsize = sprintf( (char*)pBuff, "%1d", uc_ErrKind );							// エラー種別.
		_INC_SIZE( &pBuff, &pDst->us_Len, static_cast<ushort>(wsize) );

		memcpy( pBuff, ",02", 3 );													// パラメータ(02).
		_INC_SIZE( &pBuff, &pDst->us_Len, 3 );
		wsize = sprintf( (char*)pBuff, "%s", pc_CmdGenError->getCommandList() );	// エラー発生コマンド名.
		_INC_SIZE( &pBuff, &pDst->us_Len, static_cast<ushort>(wsize) );

		pc_CmdGenError->unlock();
	}

	c_EvevntDataBuf.updateWtPtr();
}
/*==================================================================================================*/
//	発信トリガ処理「コマンド生成エラー通知」.
/*==================================================================================================*/
void RDSC_mkText_ModeCmdGenError::rcvTrg( uint32_t para )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)para, (uchar*)"コマンド生成エラー通知トリガ受信" );
	cmnTrgAction();
}
/*==================================================================================================*/
//	コマンド生成エラー通知  メールIDの等しいデータがあればクリアする(応答の確認).
/*==================================================================================================*/
void RDSC_mkText_ModeCmdGenError::chkAns( uchar* bcd_rcvEventID )
{
	if( NULL == bcd_rcvEventID )
	{
		return;
	}

	uchar bcd_bkEventID[6+1] = {""};
	pc_RdsSetting->getTrgTime( e_txtKind, bcd_bkEventID );

	if( True == EpLib_datacmp( bcd_bkEventID, bcd_rcvEventID, 6 ) )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_Email", 0, (uchar*)"コマンド生成エラー通知の応答受信" );
		// バックアップしてある時刻と比較して等しければ、発信要求マップの[済]を消す.
		theTrgMap.resetMapByTrgAllPage( e_txtKind );
		// ID も消す.
		memset( bcd_bkEventID, 0, sizeof(bcd_bkEventID) );
		pc_RdsSetting->setTrgTime( e_txtKind, bcd_bkEventID );
	}
}
// 2017/08/08 [MAv5][RQ-XXX] CSRC・CSRAデータチェック End.
/***************************************************************************************************
 *   テキスト作成処理クラス
 ***************************************************************************************************/
/*==================================================================================================*/
//	CommonAPI からトータルカウンタ取得 04/01/21 浦崎 
/*--------------------------------------------------------------------------------------------------*/
// 05/02/26 浦崎 Tigris問題点3306 対策。TYPE_TotalCntTotal にてトータルカウンタ値を取得するようにした。 
/*==================================================================================================*/
uint32_t RDSC_MakeEventData::getTotalCnt()
{
	APIC_TotalCnt*	p = APIC_TotalCnt::newInstance();

//	// トータルカウンタ(コピー/プリント合計、ブラック/フルカラー/モノカラー／２色合計) 
//	uint32_t ul_Data =   p->get( TYPE_TotalCntCopyTotalFullColor )			// コピー　・トータル・フルカラー 
//					+ p->get( TYPE_TotalCntCopyTotalMonoColor )			// コピー　・トータル・モノカラー 
//					+ p->get( TYPE_TotalCntCopyTotalBlack )				// コピー　・トータル・ブラック 
//					+ p->get( TYPE_TotalCntCopyTotal2Color )			// コピー　・トータル・２色 
//					+ p->get( TYPE_TotalCntPrintTotalFullColor )		// プリント・トータル・フルカラー 
//					+ p->get( TYPE_TotalCntPrintTotalBlack )			// プリント・トータル・ブラック 
//					+ p->get( TYPE_TotalCntPrintTotal2Color );			// プリント・トータル・２色 

	// 04/03/23 浦崎  Rhine1問題点35285対策。FAX、スキャンプリントトータルが含まれていなかったため修正。 
	// トータルカウンタ（コピートータル＋プリンタトータル＋スキャン/FAXトータル　合計値表示） 
	uint32_t ul_Data =   p->get( TYPE_TotalCntTotal );
	return( ul_Data );
}

/*==================================================================================================*/
//	M500 コマンド作成関数 
//		センターからの要求(RD、WR)に対する応答をする時にセットする。
//		使用する応答用IDは、センターからのメールの受信が正常に受信できた時に、不揮発にバックアップされる。
/*--------------------------------------------------------------------------------------------------*/
// 04/02/16 浦崎  新規 
/*==================================================================================================*/
ushort RDSC_MakeEventData::makeM500
(
	uchar* pUserDataBuff,
	uchar* pModeString		
)
{
	ushort	us_Len = 0;

	EPN_DBG_MESSAGE( (uchar*)"[CSRC]", 0, (uchar*)"RDSC_MakeEventData::makeM500" );			//2018/10/18 FPT-DucDH4 Add log debug

	// M501 コマンド 「M500yymmddhhmmss」 
	memcpy( &pUserDataBuff[us_Len], "M500", 4 );
	us_Len = (ushort)(us_Len +4);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

	uchar	uc_HostEventID[6+1];
	pc_RdsSetting->getHostEventID( uc_HostEventID );

	RDSC_Time				c_rdsTime;
	c_rdsTime.cnvString_2_Time( uc_HostEventID, RDSE_T2S_BCDyymmddhhmmss );

	ushort us_TempLen = c_rdsTime.cnvTime_2_String( &pUserDataBuff[us_Len], RDSE_T2S_yymmddhhmmss );
	//2010/12/06 CQ336227対応 Iwata Start.
	us_Len = (ushort)(us_Len + us_TempLen);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.
	//2010/12/06 CQ336227対応 Iwata End.	us_Len += us_TempLen;

	memcpy( &pUserDataBuff[us_Len], " ", 1 );											// スペース 
	us_Len = (ushort)(us_Len +1);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.


	// 発信ごとの文字列 
	memcpy( &pUserDataBuff[us_Len], pModeString, 2 );
	us_Len = (ushort)(us_Len +2);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.


	// スペース M500 コマンド作成関数では、末尾スペースは入れない。 
	//memcpy( &pUserDataBuff[us_Len], " ", 1 );
	//us_Len += 1;

	return( us_Len );
}



/*==================================================================================================*/
//	M501 コマンド作成関数 04/02/09 浦崎 
/*==================================================================================================*/
ushort RDSC_MakeEventData::makeM501
(
	uchar* pUserDataBuff,
	uchar* pBcdEventID,
	uchar* pModeString		
)
{
	ushort	us_Len = 0;
	EPN_DBG_MESSAGE((uchar*)"[CSRC RDSC_MakeEventData::makeM501]", 0, (uchar*)pBcdEventID);
	EPN_DBG_MESSAGE( (uchar*)"[CSRC]", 0, (uchar*)"RDSC_MakeEventData::makeM501" );			//2018/10/18 FPT-DucDH4 Add log debug

	// M501 コマンド 「M501yymmddhhmmss」 
	memcpy( &pUserDataBuff[us_Len], "M501", 4 );
	us_Len = (ushort)(us_Len +4);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

	// イベントID 
	RDSC_Time	c_rdsTime;
	c_rdsTime.cnvString_2_Time( pBcdEventID, RDSE_T2S_BCDyymmddhhmmss );
	us_Len = (ushort)(us_Len + c_rdsTime.cnvTime_2_String( &pUserDataBuff[us_Len], RDSE_T2S_yymmddhhmmss ));	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.
// 2020/09/11 FPT-LucNH2 【 IT6_2.0MR3 】【OP_BTS-26048】 Start.
	EPN_DBG_MESSAGE((uchar*)"[CSRC makeM501 pBcdEventID]", 12, (uchar*)pBcdEventID);
	EPN_DBG_MESSAGE((uchar*)"[CSRC makeM501 pUserDataBuff]", (uint32_t)us_Len, (uchar*)pUserDataBuff);
// 2020/09/11 FPT-LucNH2 【 IT6_2.0MR3 】【OP_BTS-26048】 End.
	// スペース 
	memcpy( &pUserDataBuff[us_Len], " ", 1 );
	us_Len = (ushort)(us_Len +1);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

	// 発信ごとの文字列 
	memcpy( &pUserDataBuff[us_Len], pModeString, 2 );
	us_Len = (ushort)(us_Len +2);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

	// スペース 
	memcpy( &pUserDataBuff[us_Len], " ", 1 );
	us_Len = (ushort)(us_Len +1);	// 2010/07/17 sone Minerva warning 対応 ushortにキャスト.

	EPN_DBG_MESSAGE((uchar*)"[CSRC pUserDataBuff 2]", 0, (uchar*)pUserDataBuff);
	return( us_Len );
}

/*==================================================================================================*/
//	E-mail版フレームNo.01 作成関数 04/02/10 浦崎 
/*--------------------------------------------------------------------------------------------------*/
// 05/02/15 浦崎  新規 
/*==================================================================================================*/
ushort RDSC_MakeEventData::makeFrameNo1
(
//	uchar* pUserDataBuff
//	uchar* pBcdEventID
)
{
	RDSS_EventData*			pDst;
	register uchar*			pBuff;
	RDSC_Time				c_rdsTime;
	uchar					uc_Temp[_S_Tel+1];

	pDst  = c_EvevntDataBuf.wt;
	pBuff = &pDst->uc_Buff[0];												// イベントデータバッファへのポインタを取得

	c_EvevntDataBuf.clearCurrentBuff();										/* バッファクリア（c_EvevntDataBuf.wt->uc_Buff[]） */

	pDst->e_Kind = TYPE_RdsTrgKindUnKnowin;
	
	ushort	us_Len = 0;


	// A000xxxxxxxxx I000xxxxx M300yymmddhhmmss B000nnnnnnnn 

	// ■ マシンID A000xxxxxxxxx
	memcpy( pBuff, "A000", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

#ifdef RDSD_MODE_SN13		//マシンID 13桁対応。 06/04/03 浦崎 
	pc_RdsSetting->getMachineID( uc_Temp );
	memcpy( pBuff, uc_Temp, _S_MachinID );
	_INC_SIZE( &pBuff, &pDst->us_Len, _S_MachinID );
#else
	pc_RdsSetting->getMachineID( uc_Temp );
	memcpy( pBuff, uc_Temp, 9 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 9 );
#endif

	memcpy( pBuff, " ", 1 );											// スペース
	_INC_SIZE( &pBuff, &pDst->us_Len, 1 );

	// ■ センターID I000xxxxx
	memcpy( pBuff, "I000", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

// 2017/08/08 MAv5 1026700 Start.
// OpenAPI通信時は固定のセンターIDを設定する.
	if( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ){
		RDSC_OapUtil::getHostID( uc_Temp );
	} else {
		pc_RdsSetting->getHostID( uc_Temp );
	}
// 2017/08/08 MAv5 1026700 End.
	memcpy( pBuff, uc_Temp, 5 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 5 );

	memcpy( pBuff, " ", 1 );											// スペース
	_INC_SIZE( &pBuff, &pDst->us_Len, 1 );

	// ■ M300yymmddhhmmss 
	memcpy( pBuff, "M300", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

	c_rdsTime = theVRmsc.getNowTime();
	ushort us_TempLen = c_rdsTime.cnvTime_2_String( pBuff, RDSE_T2S_yymmddhhmmss );
	_INC_SIZE( &pBuff, &pDst->us_Len, us_TempLen );
	memcpy( pBuff, " ", 1 );											// スペース
	_INC_SIZE( &pBuff, &pDst->us_Len, 1 );
// 2016/03/23 ktakahashi [ZeusS_ZX0]CQ526327_E-mail自動初期発信ありでCSRC設定のインポート・エクスポート後にボタンクリック不可 Start.
	// 初期発信のセンターへの送信時間をEventIDに変換.
	uchar uc_EventID[RDSD_EVENTID_SIZE+1] = {0};
	c_rdsTime.cnvTime_2_String( uc_EventID, RDSE_T2S_BCDyymmddhhmmss );
	setTrgEventID( uc_EventID );	// EventIDを保持しておく.
// 2016/03/23 ktakahashi [ZeusS_ZX0]CQ526327_E-mail自動初期発信ありでCSRC設定のインポート・エクスポート後にボタンクリック不可 End.

	// ■トータルカウンタ：B000########
	memcpy( pBuff, "B000", 4 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 4 );

	APIC_TotalCnt*	p = APIC_TotalCnt::newInstance();

	// トータルカウンタ 
	uint32_t ul_Data =   p->get( TYPE_TotalCntTotal );						// トータルカウンタ 05/02/15 浦崎 FAX・スキャンプリントトータルを含めた値を取得するようにした。 
//	uint32_t ul_Data =   p->get( TYPE_TotalCntCopyTotalFullColor )			// コピー　・トータル・フルカラー
//					+ p->get( TYPE_TotalCntCopyTotalMonoColor )			// コピー　・トータル・モノカラー
//					+ p->get( TYPE_TotalCntCopyTotalBlack )				// コピー　・トータル・ブラック
//					+ p->get( TYPE_TotalCntCopyTotal2Color )			// コピー　・トータル・２色
//					+ p->get( TYPE_TotalCntPrintTotalFullColor )		// プリント・トータル・フルカラー
//					+ p->get( TYPE_TotalCntPrintTotalBlack )			// プリント・トータル・ブラック
//					+ p->get( TYPE_TotalCntPrintTotal2Color );			// プリント・トータル・２色

	EpLib_ltoa( ul_Data, pBuff, 8 );
	_INC_SIZE( &pBuff, &pDst->us_Len, 8 );

	c_EvevntDataBuf.updateWtPtr();
	return( us_Len );
}

/*==================================================================================================*/
//	E-mail版フレームNo.と LEN 作成関数 
/*--------------------------------------------------------------------------------------------------*/
//	04/02/10 浦崎  新規 
//	04/03/24 浦崎  レングスを１０進数で作成していたため修正 
/*==================================================================================================*/
ushort RDSC_MakeEventData::makeFrameNo_and_Len
(
	uchar*	pUserDataBuff,
	uint32_t	ul_DataLen,
	uchar	uc_FrameNo,
	Bool	b_LastFlag
)
{

	// FrameNo.(nn) 
	if( True == b_LastFlag )
	{
		uc_FrameNo |= 0x80;
	}
	sprintf( (char*)&pUserDataBuff[0], "%02x", uc_FrameNo );
	memcpy( &pUserDataBuff[2], " ", 1 );											// スペース 

	// LEN(nnnnnn) 
	sprintf( (char*)&pUserDataBuff[3], "%06x", (int)ul_DataLen );
	memcpy( &pUserDataBuff[9], " ", 1 );											// スペース 

	EPN_DBG_MESSAGE( (uchar*)"[CSRC pUserDataBuff]", (uint32_t)0, (uchar*)pUserDataBuff);			//2018/10/18 FPT-DucDH4 Add log debug
	return( 10 );																	// データ長 10 にてリターン 
}

/*==================================================================================================*/
//	発信要求マップをサーチし、テキストマップを作成
/*==================================================================================================*/
Bool RDSC_MakeEventData::reqMakeData( RDSE_TaskID devID )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"#[41]RDSC_MakeEventData::reqMakeData[st]", (uint32_t)devID );
	if( devID != theRdsApl.e_CommDevice )
	{
		/* 現在通信中のタスクと同じタスクからの要求でなければ受付けない */
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)devID, (uchar*)" テキストマップ作成エラー" );
		return( False );
	}

// 2020/10/19 FPT-LucNH2 【 IT6_2.1 】 【至急】AddressSanitizer 対応依頼 Start.
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"#[42]RDSC_MakeEventData::mapClear[st]" );
// 2020/10/19 FPT-LucNH2 【 IT6_2.1 】 【至急】AddressSanitizer 対応依頼 End.
	mapClear();
	makeTextMap();															/* テキストマップ作成 */
	c_EvevntDataBuf.reset();												/* 送信テキストバッファをリセット */

// 2017/6/27 MAv5 CSRA接続 start.
// 2017/9/12 MAv5 AccuRev 1028998 start.
// 2020/10/23 Sky二口 OP_BTS-28458 [RQ-811] START.
//	if ( True == RDSC_CsraConnectionManager::newInstance()->isSending() ) {
	if ( ( True == RDSC_CsraConnectionManager::newInstance()->isSending() )
	  || ( True == RDSC_CsrcConnectionManager::newInstance( RDSE_CommunicationType_Immediate )->isSending() ) ) {
// 2020/10/23 Sky二口 OP_BTS-28458 [RQ-811] END.
// 2017/9/12 MAv5 AccuRev 1028998 end.
		//作成しない.
	} else {
		makeText();
	}
// 2017/6/27 MAv5 CSRA接続 end.
	return( True );
}

/*==================================================================================================*/
//	InternetISW用応答データ作成 
/*==================================================================================================*/
Bool RDSC_MakeEventData::reqMakeData_InternetISW( RDSE_TaskID devID )
{
	c_EvevntDataBuf.reset();												/* 送信テキストバッファをリセット */

	
	// ■ E-mail 固有 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )		// E-mail版 / HTTP版 寺内(08/01/31) 新規追加.
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		RDSC_MakeEventData::makeFrameNo1();
	}


	if( c_FND.uc_transMap[1][0] )										/* TG への応答発信 */
	{
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindResI_ISW );			// 05/03/28 浦崎 通信ログに記録するようにした。 
		c_mkText_ModeResInternetISW.makeData( 0 );								// パラメータは特になし。 
	}
	if( c_FND.uc_transMap[1][1] )										/* 結果通知発信 */
	{
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindTxResultI_ISW );		// 05/03/28 浦崎 通信ログに記録するようにした。 
		c_mkText_ModeTxResultI_ISW.makeData( 0 );								// パラメータは特になし。 
	}


	return( True );
}


/*==================================================================================================*/
//	テキストマップ作成 
//		※ もっとオブジェクト指向っぽく書きたいが、C++ 以外のシステムへの移植が考えられる為、
//		   べた書き。
/*--------------------------------------------------------------------------------------------------*/
// 02/03/19 浦崎  部門管理テキストは、テキスト要求で要求された時のみセンターに通知するように決定した為
//                対応した
// 02/04/15 浦崎  NU 対応
// 02/04/22 浦崎  各処理の子関数を作り、処理分離（Job履歴管理用、FWNWDL用の処理が増えると予想した為）
// 02/05/27 浦崎  FND 対応。
/*==================================================================================================*/
void RDSC_MakeEventData::makeTextMap( void )
{
	/*=== 警告発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][ TYPE_RdsTrgKindWarnBody ]	)			/* 警告発信 */
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"#[43]RDSC_MakeEventData::makeTextMap[st]" );
		b_Map[TYPE_RdsTrgKindWarnBody] = True;
//		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindWarnBody );		// 問題点77306対応（コメントアウト）  
	}

	/*=== 警告解除発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][ TYPE_RdsTrgKindWarnRecvBody ] ){
		b_Map[TYPE_RdsTrgKindWarnRecvBody] = True;								/* 警告解除発信 */
//		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindWarnRecvBody );	// 問題点77302対応（コメントアウト） 
	}

	/* 緊急発信の要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindTrbl] ){
		b_Map[TYPE_RdsTrgKindTrbl] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindTrbl );
	}

	/* 緊急回復発信の要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindTrblRecv] ){
		b_Map[TYPE_RdsTrgKindTrblRecv] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindTrblRecv );
	}

	/* 初期発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindIni] ){
		b_Map[TYPE_RdsTrgKindIni] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindIni );
	}

	/* メンテ開始発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindMnteStart] ){
		b_Map[TYPE_RdsTrgKindMnteStart] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindMnteStart );
	}

	/* メンテ終了発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindMnteEnd] ){
		b_Map[TYPE_RdsTrgKindMnteEnd] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindMnteEnd );
	}

	/* 管理者発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindManage] ){
		b_Map[TYPE_RdsTrgKindManage] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindManage );
	}

	/* 期日指定発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindTimeOut] ){
		b_Map[TYPE_RdsTrgKindTimeOut] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindTimeOut );
	}


	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )		/* E-mail版の通信中？ */
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )		//HTTP版　寺内(08/01/31) 新規追加.
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindRes_RDS] ){				/* 応答発信要求がある場合[E-mail版] */
			b_Map[TYPE_RdsTrgKindRes_RDS] = True;									/* 発信種別テキスト */
			/*b_Map[txtKind_ResData]	  = True;*/									/* 応答データテキスト */

			ushort us_tmpModeBit = (ushort)pc_RdsSetting->getEmailResultTypeCode();
			theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindRes_RDS, us_tmpModeBit );
		
		
		}
	}

	/* 新品交換発信要求がある場合 */
	if( True == theTrgMap.b_transMapForPartsReplace[1] ){
		b_Map[TYPE_RdsTrgKindPartsReplace] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindPartsReplace );
	}
	
	// Minerva UV-00884 MFP認証情報変更発信追加対応 sone 2010/6/21 START.
	/* MFP認証情報変更発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindAuthInfoChange] ){
		b_Map[TYPE_RdsTrgKindAuthInfoChange] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindAuthInfoChange );
	}
	// Minerva UV-00884 MFP認証情報変更発信追加対応 sone 2010/6/21 END.
	
	// Minerva UV-00781 プロダクト認証対応 sone 2010/08/27 START.
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindProductCertification] ){
		b_Map[TYPE_RdsTrgKindProductCertification] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindProductCertification );
	}
	// Minerva UV-00781 プロダクト認証対応 sone 2010/08/27 END.

// 2010/08/24 higuchi Minerva UV-00794 【デバッグログ収集】 Start.
	/* デバッグログ手動蓄積完了報告発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindDebugLogSave] ){
		b_Map[TYPE_RdsTrgKindDebugLogSave] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindDebugLogSave );
	}
// 2010/08/24 higuchi Minerva UV-00794 【デバッグログ収集】 End.
	// 2010/07/29 yito UV-00886 Exp/Imp start.
	/* エクスポート・インポート完了通知発信がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindExpImpComplete] ){
		b_Map[TYPE_RdsTrgKindExpImpComplete] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindExpImpComplete );
	}
	// 2010/07/29 yito UV-00886 Exp/Imp end  .
	
// 2010/09/15 Minerva UV-00898追加対応 kita Start.
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindTrblResetStatus] ){
		b_Map[TYPE_RdsTrgKindTrblResetStatus] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindTrblResetStatus );
	}
// 2010/09/15 Minerva UV-00898追加対応 kita End.

	// Minerva UV-00683 sone START.
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindResult_Lic] ){
		b_Map[TYPE_RdsTrgKindResult_Lic] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindResult_Lic );
	}
	// Minerva UV-00683 sone END.
	
// 2010/11/19 Minerva UV-00971 kita Start.
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindInternetISWExStatus] ){
		b_Map[TYPE_RdsTrgKindInternetISWExStatus] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindInternetISWExStatus );
	}
// 2010/11/19 Minerva UV-00971 kita End.
// 2011/12/01 amiya ZeusMLK UV-01170 暫定救済中のCSRCへの通知対応 START.
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindTemporaryRescue0] ){
		EPN_DBG_MESSAGE((uchar*)"RDSC_MakeEventData::makeTextMap()", (uint32_t)0, (uchar*)"##TemporaryRescue## !");
		b_Map[TYPE_RdsTrgKindTemporaryRescue0] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindTemporaryRescue0 );
	}
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindTemporaryRescue1] ){
		EPN_DBG_MESSAGE((uchar*)"RDSC_MakeEventData::makeTextMap()", (uint32_t)0, (uchar*)"##TemporaryRescue## !");
		b_Map[TYPE_RdsTrgKindTemporaryRescue1] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindTemporaryRescue1 );
	}
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindTemporaryRescueRelease] ){
		EPN_DBG_MESSAGE((uchar*)"RDSC_MakeEventData::makeTextMap()", (uint32_t)0, (uchar*)"##TemporaryRescueRelease## !");
		b_Map[TYPE_RdsTrgKindTemporaryRescueRelease] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindTemporaryRescueRelease );
	}
// 2011/12/01 amiya ZeusMLK UV-01170 暫定救済中のCSRCへの通知対応 END.
// 2012/03/19 higuchi Minerva/Zeus UV-01225 【デバッグログ拡張機能】 Start.
	/* デバッグログネット配信結果報告発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindDebugLogNetResult] ){
		EPN_DBG_MESSAGE((uchar*)"RDSC_MakeEventData::makeTextMap()", (uint32_t)0, (uchar*)"Debug Log Result Test");
		b_Map[TYPE_RdsTrgKindDebugLogNetResult] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindDebugLogNetResult );
	}
// 2012/03/19 higuchi Minerva/Zeus UV-01225 【デバッグログ拡張機能】 End.
// 2013/12/05 OPS1前田(Sky) ZeusCD UV-01387_リモートパネルの改善(WP213) Start.
	/* リモートパネル起動結果発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindRmpStartResult] ){
		EPN_DBG_MESSAGE((uchar*)"RDSC_MakeEventData::makeTextMap()", (uint32_t)0, (uchar*)"Rmp Start Result");
		b_Map[TYPE_RdsTrgKindRmpStartResult] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindRmpStartResult );
	}
// 2013/12/05 OPS1前田(Sky) ZeusCD UV-01387_リモートパネルの改善(WP213) End.

// MA2 UV-01382 「遠隔でのデバッグログ取得の改善」対応 2013/10/19 mtoyoda START.
	/* 遠隔ログ取得結果報告発信要求がある場合 */
	if( True == theTrgMap.b_transMap[ 1 ][ TYPE_RdsTrgKindRemoteLogResult ] ){
		EPN_DBG_MESSAGE( (uchar*)"RDSC_MakeEventData::makeTextMap()", (uint32_t)0, (uchar*)"Remote Log Result" );
		b_Map[ TYPE_RdsTrgKindRemoteLogResult ] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindRemoteLogResult );
	}
// MA2 UV-01382 「遠隔でのデバッグログ取得の改善」対応 2013/10/19 mtoyoda END.

// 2014/09/03 ktakahashi ZeusS 制御構成変更 No.4 HDDミラーリング Start.
	/* 冗長化パーツ警告発信要求がある場合 */
	if( True == theTrgMap.b_transMap[ 1 ][ TYPE_RdsTrgKindWarnRedunParts ] ){
		EPN_DBG_MESSAGE( (uchar*)"RDSC_MakeEventData::makeTextMap()", (uint32_t)0, (uchar*)"Warn Redun Parts" );
		b_Map[ TYPE_RdsTrgKindWarnRedunParts ] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindWarnRedunParts );
	}

	/* 冗長化パーツ警告解除発信要求がある場合 */
	if( True == theTrgMap.b_transMap[ 1 ][ TYPE_RdsTrgKindWarnRecvRedunParts ] ){
		EPN_DBG_MESSAGE( (uchar*)"RDSC_MakeEventData::makeTextMap()", (uint32_t)0, (uchar*)"Warn Recv Redun Parts" );
		b_Map[ TYPE_RdsTrgKindWarnRecvRedunParts ] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindWarnRecvRedunParts );
	}
// 2014/09/03 ktakahashi ZeusS 制御構成変更 No.4 HDDミラーリング End.

// 2015/06/09 mtoyoda Helios KRQ-101_認証機能有効化対応 START.
	/* 認証機能有効化のエラー通知発信要求がある場合 */
	if( True == theTrgMap.b_transMap[ 1 ][ TYPE_RdsTrgKindAuthFunctionError ] ){
		EPN_DBG_MESSAGE( (uchar*)"RDSC_MakeEventData::makeTextMap()", (uint32_t)0, (uchar*)"Auth Function Error" );
		b_Map[ TYPE_RdsTrgKindAuthFunctionError ] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindAuthFunctionError );
	}
// 2015/06/09 mtoyoda Helios KRQ-101_認証機能有効化対応 END.

// 2016/09/08 T.Komatsu IT5_4.0 RQ-505_バックアップ機能強化(WP267) START.
	/* データバックアップエラー通知発信要求がある場合 */
	if( True == theTrgMap.b_transMap[ 1 ][ TYPE_RdsTrgKindExtMfpBackupServerInfo ] ){
		EPN_DBG_MESSAGE( (uchar*)"RDSC_MakeEventData::makeTextMap()", 0, (uchar*)"Backup Server Info" );
		b_Map[ TYPE_RdsTrgKindExtMfpBackupServerInfo ] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindExtMfpBackupServerInfo );
	}
// 2016/09/08 T.Komatsu IT5_4.0 RQ-505_バックアップ機能強化(WP267) END.

// 2017/07/20 [MAv5][RQ-XXX] CSRA設定状態発信 Start.
	/* CSRA設定状態発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindCsraSettingState] ){
		EPN_DBG_MESSAGE((uchar*)"RDSC_MakeEventData::makeTextMap()", (uint32_t)0, (uchar*)"CSRA Setting Info");
		b_Map[TYPE_RdsTrgKindCsraSettingState] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindCsraSettingState );
	}
// 2017/07/20 [MAv5][RQ-XXX] CSRA設定状態発信 End.

// 2017/08/08 [MAv5][RQ-XXX] CSRC・CSRAデータチェック Start.
	/* コマンド生成エラー通知（CSRC）がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindCmdGenError_CSRC] ){
		EPN_DBG_MESSAGE((uchar*)"RDSC_MakeEventData::makeTextMap()", (uint32_t)0, (uchar*)"Command Generation Error(CSRC)");
		b_Map[TYPE_RdsTrgKindCmdGenError_CSRC] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindCmdGenError_CSRC );
	}
	/* コマンド生成エラー通知（CSRA日毎）がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindCmdGenError_CSRA_Daily] ){
		EPN_DBG_MESSAGE((uchar*)"RDSC_MakeEventData::makeTextMap()", (uint32_t)0, (uchar*)"Command Generation Error(CSRA_Daily)");
		b_Map[TYPE_RdsTrgKindCmdGenError_CSRA_Daily] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindCmdGenError_CSRA_Daily );
	}
	/* コマンド生成エラー通知（CSRA週毎）がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindCmdGenError_CSRA_Weekly] ){
		EPN_DBG_MESSAGE((uchar*)"RDSC_MakeEventData::makeTextMap()", (uint32_t)0, (uchar*)"Command Generation Error(CSRA_Weekly)");
		b_Map[TYPE_RdsTrgKindCmdGenError_CSRA_Weekly] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindCmdGenError_CSRA_Weekly );
	}
	/* コマンド生成エラー通知（CSRA月毎）がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindCmdGenError_CSRA_Monthly] ){
		EPN_DBG_MESSAGE((uchar*)"RDSC_MakeEventData::makeTextMap()", (uint32_t)0, (uchar*)"Command Generation Error(CSRA_Monthly)");
		b_Map[TYPE_RdsTrgKindCmdGenError_CSRA_Monthly] = True;
		theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindCmdGenError_CSRA_Monthly );
	}
// 2017/08/08 [MAv5][RQ-XXX] CSRC・CSRAデータチェック End.
}

/*************** add start by tomiyasu 2014/12/11 *************************************/

/*==================================================================================================*/
//	モデム用ログデータ作成 
/*--------------------------------------------------------------------------------------------------*/
// 2014/12/11 冨安  モデム通信のログが発信開始から正しくセットされるために、発信時に実行される。
/*==================================================================================================*/
void RDSC_MakeEventData::makeModemLog( void )
{
	RDS_modem_Kind = 0;

	/*=== 警告発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][ TYPE_RdsTrgKindWarnBody ]	)			/* 警告発信 */
	{
		RDS_modem_Kind |= 0x00000008;			// 発着信種別 警告発信bitON 
	}

	/*=== 警告解除発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][ TYPE_RdsTrgKindWarnRecvBody ] ){
		RDS_modem_Kind |= 0x00000010;			// 発着信種別 警告回復発信bitON 
	}

	/* 緊急発信の要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindTrbl] ){
		RDS_modem_Kind |=  0x00000002;				// 発着信種別 緊急発信bitON
	}

	/* 緊急回復発信の要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindTrblRecv] ){
		RDS_modem_Kind |=  0x00000004;				// 発着信種別 緊急回復発信bitON 
	}

	/* 初期発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindIni] ){
		RDS_modem_Kind |=  0x00000001;				// 発着信種別 初期発信bitON
	}

	/* メンテ開始発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindMnteStart] ){
	RDS_modem_Kind |=  0x00000020;				// 発着信種別 メンテ開始発信bitON
	}

	/* メンテ終了発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindMnteEnd] ){
		RDS_modem_Kind |=  0x00000040;				// 発着信種別 メンテ終了発信bitON
	}

	/* 管理者発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindManage] ){
		RDS_modem_Kind |=  0x00000080;				// 発着信種別 管理者発信bitON
	}

	/* 期日指定発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindTimeOut] ){
		RDS_modem_Kind |=  0x00000100;				// 発着信種別 期日指定発信bitON
	}

	/* 新品交換発信要求がある場合 */
	if( True == theTrgMap.b_transMapForPartsReplace[1] ){
		RDS_modem_Kind |= 0x00000800;				// 発着信種別 新品検出発信bitON
	}
	
	// Minerva UV-00884 MFP認証情報変更発信追加対応 sone 2010/6/21 START.
	/* MFP認証情報変更発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindAuthInfoChange] ){
		RDS_modem_Kind |= 0x00004000;				// 発着信種別 MFP認証情報変更発信bitON
	}
	// Minerva UV-00884 MFP認証情報変更発信追加対応 sone 2010/6/21 END.

// 2010/08/24 higuchi Minerva UV-00794 【デバッグログ収集】 Start.
	/* デバッグログ手動蓄積完了報告発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindDebugLogSave] ){
		RDS_modem_Kind |= 0x00080000;				// 発着信種別 デバッグログ手動蓄積完了報告発信bitON 
	}
// 2010/08/24 higuchi Minerva UV-00794 【デバッグログ収集】 End.
	// 2010/07/29 yito UV-00886 Exp/Imp start.
	/* エクスポート・インポート完了通知発信がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindExpImpComplete] ){
		RDS_modem_Kind |= 0x00020000;				// 発着信種別 エクスポート・インポート完了発信bitON 
	}
	// 2010/07/29 yito UV-00886 Exp/Imp end  .
	
// 2010/09/15 Minerva UV-00898追加対応 kita Start.
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindTrblResetStatus] ){
		RDS_modem_Kind |= 0x00040000;				// 発着信種別 トラブル解除状態通知発信bitON 
	}
// 2010/09/15 Minerva UV-00898追加対応 kita End.

	// Minerva UV-00683 sone START.
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindResult_Lic] ){
		RDS_modem_Kind |= 0x00008000;				// 発着信種別 ﾗｲｾﾝｽ処理の完了通知発信bitON 
	}
	// Minerva UV-00683 sone END.
	
// 2010/11/19 Minerva UV-00971 kita Start.
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindInternetISWExStatus] ){
		RDS_modem_Kind |= 0x00000400;				// 発着信種別 IISW結果通知発信bitON
	}
// 2010/11/19 Minerva UV-00971 kita End.
// 2011/12/01 amiya ZeusMLK UV-01170 暫定救済中のCSRCへの通知対応 START.
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindTemporaryRescue0] ){
		RDS_modem_Kind |= 0x00200000;				// 発着信種別 不良箇所切り離し通知発信bitON
	}
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindTemporaryRescue1] ){
		RDS_modem_Kind |= 0x00200000;				// 発着信種別 不良箇所切り離し通知発信bitON
	}
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindTemporaryRescueRelease] ){
		RDS_modem_Kind |= 0x00400000;				// 発着信種別 不良箇所切り離し解除通知発信bitON 
	}
// 2011/12/01 amiya ZeusMLK UV-01170 暫定救済中のCSRCへの通知対応 END.
// 2012/03/19 higuchi Minerva/Zeus UV-01225 【デバッグログ拡張機能】 Start.
	/* デバッグログネット配信結果報告発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindDebugLogNetResult] ){
		RDS_modem_Kind |= 0x00800000;				// 発着信種別 デバッグログネット配信報告発信bitON 
	}
// 2012/03/19 higuchi Minerva/Zeus UV-01225 【デバッグログ拡張機能】 End.
// 2013/12/05 OPS1前田(Sky) ZeusCD UV-01387_リモートパネルの改善(WP213) Start.
	/* リモートパネル起動結果発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindRmpStartResult] ){
		RDS_modem_Kind |= 0x02000000;				// 発着信種別 リモートパネル起動結果発信bitON
	}
// 2013/12/05 OPS1前田(Sky) ZeusCD UV-01387_リモートパネルの改善(WP213) End.

// MA2 UV-01382 「遠隔でのデバッグログ取得の改善」対応 2013/10/19 mtoyoda START.
	/* 遠隔ログ取得結果報告発信要求がある場合 */
	if( True == theTrgMap.b_transMap[ 1 ][ TYPE_RdsTrgKindRemoteLogResult ] ){
		RDS_modem_Kind |= 0x01000000;				// 発着信種別 遠隔ログ取得処理結果報告発信bitON
	}
// MA2 UV-01382 「遠隔でのデバッグログ取得の改善」対応 2013/10/19 mtoyoda END.

// 2014/09/03 ktakahashi ZeusS 制御構成変更 No.4 HDDミラーリング Start.
	/* 冗長化パーツ警告発信要求がある場合 */
	if( True == theTrgMap.b_transMap[ 1 ][ TYPE_RdsTrgKindWarnRedunParts ] ){
		RDS_modem_Kind |= 0x04000000;				// 発着信種別 冗長化パーツ警告発信bitON
	}

	/* 冗長化パーツ警告解除発信要求がある場合 */
	if( True == theTrgMap.b_transMap[ 1 ][ TYPE_RdsTrgKindWarnRecvRedunParts ] ){
		RDS_modem_Kind |= 0x08000000;				// 発着信種別 冗長化パーツ警告解除発信bitON.
	}
// 2014/09/03 ktakahashi ZeusS 制御構成変更 No.4 HDDミラーリング End.

// 2015/06/05 mtoyoda Helios KRQ-101_認証機能有効化対応 START.
	/* 認証機能有効化のエラー通知発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindAuthFunctionError] ){
		// 発信種別コードの枯渇防止のため、リモートパネル起動結果発信と同じ発信種別コードを割り当てる.
		RDS_modem_Kind |= 0x02000000;				// 発着信種別 認証機能有効化のエラー通知発信bitON.
	}
// 2015/06/05 mtoyoda Helios KRQ-101_認証機能有効化対応 END.
// 2016/09/08 T.Komatsu IT5_4.0 RQ-505_バックアップ機能強化(WP267) START.
	/* データバックアップエラー通知発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindExtMfpBackupServerInfo] ){
		// 発信種別コードの枯渇防止のため、エクスポート・インポート完了通知発信と同じ発信種別コードを割り当てる.
		RDS_modem_Kind |= 0x00020000;				// 発着信種別 データバックアップエラー通知発信bitON.
	}
// 2016/09/08 T.Komatsu IT5_4.0 RQ-505_バックアップ機能強化(WP267) END.
// 2017/07/20 [MAv5][RQ-XXX] CSRA設定状態発信 Start.
	/* CSRA設定状態発信要求がある場合 */
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindCsraSettingState] ){
		RDS_modem_Kind |= 0x02000000;				// 発着信種別 CSRA設定状態発信bitON.
	}
// 2017/07/20 [MAv5][RQ-XXX] CSRA設定状態発信 End.
// 2017/08/08 [MAv5][RQ-XXX] CSRC・CSRAデータチェック Start.
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindCmdGenError_CSRC] ){
		RDS_modem_Kind |= 0x02000000;				// 発着信種別 コマンド生成エラー通知bitON.
	}
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindCmdGenError_CSRA_Daily] ){
		RDS_modem_Kind |= 0x02000000;				// 発着信種別 コマンド生成エラー通知bitON.
	}
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindCmdGenError_CSRA_Weekly] ){
		RDS_modem_Kind |= 0x02000000;				// 発着信種別 コマンド生成エラー通知bitON.
	}
	if( True == theTrgMap.b_transMap[1][TYPE_RdsTrgKindCmdGenError_CSRA_Monthly] ){
		RDS_modem_Kind |= 0x02000000;				// 発着信種別 コマンド生成エラー通知bitON.
	}
// 2017/08/08 [MAv5][RQ-XXX] CSRC・CSRAデータチェック End.
}

/*************** add end by tomiyasu 2014/12/11 *************************************/

/*==================================================================================================*/
//	テキストマップクリア 
/*==================================================================================================*/
void RDSC_MakeEventData::mapClear( void )
{
// 2020/10/19 FPT-LucNH2 【 IT6_2.1 】 【至急】AddressSanitizer 対応依頼 Start.
//	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"#[42]RDSC_MakeEventData::mapClear[st]" );
// 2020/10/19 FPT-LucNH2 【 IT6_2.1 】 【至急】AddressSanitizer 対応依頼 End.
	for( register short indx=0; indx<_S_EventDataNum; indx++ ){
		b_Map[indx] = False;
	}

	warnSubjectInfo = 0;			// 04/04/01 浦崎 
// 2016/03/23 ktakahashi [ZeusS_ZX0]CQ526327_E-mail自動初期発信ありでCSRC設定のインポート・エクスポート後にボタンクリック不可 Start.
	memset( uc_TrgEventID, 0, sizeof( uc_TrgEventID ) );
// 2016/03/23 ktakahashi [ZeusS_ZX0]CQ526327_E-mail自動初期発信ありでCSRC設定のインポート・エクスポート後にボタンクリック不可 End.
}

/*==================================================================================================*/
//	RAMクリア時にコールされる 
/*==================================================================================================*/
void RDSC_MakeEventData::init( void )
{
	mapClear();													/* テキストマップクリア */
}

void RDSC_MakeEventData::reset( void )
{
	c_mkText_ModeWarn.reset();													/* テキストマップクリア */
}


/*==================================================================================================*/
//	発信トリガ受信時の共通処理 
/*--------------------------------------------------------------------------------------------------*/
// 04/03/11 浦崎  発信ごとの受付時間（E-mail版のイベントIDとして使用）を CommonAPI にセットするようにした。
// 04/03/12 浦崎  引数に時刻を設定出来るようにした。
//				  緊急発信など、別バッファにとりこんだ時刻と、トリガ受付時間が一致しないと
//				  いけないようなケースに対応するため。
/*==================================================================================================*/
void RDSC_mkText_Mode::cmnTrgAction(
									RDSC_Time* c_TrgTime		// トリガ受付時間。デフォルト引数で NULL がセットされているため省略可能。
									)
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"#[19]RDSC_mkText_Mode::cmnTrgAction[st]" );
	if( TYPE_RdsTrgKindUnKnowin != e_txtKind )
	{
		if(    TYPE_RdsTrgKindResI_ISW      == e_txtKind
			|| TYPE_RdsTrgKindTxResultI_ISW == e_txtKind
			|| e_txtKind < TYPE_RdsTrgKindNumberOfTrg    )
		{														/* TYPE_RdsTrgKindAutoReTrans と TYPE_RdsTrgKindResTimeOut は対象外 */
			uchar uc_BcdEventID[6+1];
			RDSC_Time c_rdsTime;
			if( NULL == c_TrgTime )
			{
				c_rdsTime = theVRmsc.getNowTime();
				EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"EVENT ID取得" );
			}
			else
			{
				c_rdsTime = *c_TrgTime;
				EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"EVENT ID設定" );
			}
			
			// 緊急発信トリガ時間テスト 
			printEpTime( (RDSS_DetailTime)c_rdsTime,			(uchar*)"トリガ受付時間" );
			
			c_rdsTime.cnvTime_2_String( &uc_BcdEventID[0], RDSE_T2S_BCDyymmddhhmmss );
			switch( e_txtKind )
			{
			/* InternetISW 関係 */
			case TYPE_RdsTrgKindResI_ISW :
			case TYPE_RdsTrgKindTxResultI_ISW :
				c_FND.setTrgTime( e_txtKind, uc_BcdEventID );
				tskRdsEvent.c_AutoReTrans.rcvTrg( e_txtKind );			/* 発信トリガ受信時の自動再発信状態リセットに関する処理 */
				theRdsEmail.c_ResChk.rcvTrg( e_txtKind );				/* 発信トリガ受信時の応答の確認状態リセットに関する処理 */
				break;

			/* CSRC 関係 */
			default :
				// 新品交換検出発信でない(既存)場合はこちら。
				if ( e_txtKind < TYPE_RdsTrgKindPartsReplace )
				{
					EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"新品交換発信でない");
					pc_RdsSetting->setTrgTime( e_txtKind, uc_BcdEventID );
					// 2016/03/23 ktakahashi [ZeusS_ZX0]CQ526327_E-mail自動初期発信ありでCSRC設定のインポート・エクスポート後にボタンクリック不可.
					// Memo：HTTP1/EMail1通信時の初期発信のトリガ時間設定はRDSC_mkText_ModeIni::makeData()にて再設定する.
				}
				// 新品交換発信だった場合は、NVDの対応が別領域になってしまうため、こちら。
				else if ( e_txtKind == TYPE_RdsTrgKindPartsReplace )
				{
					EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"新品交換発信だ");
					pc_RdsSetting->setPartsReplaceEventID( uc_BcdEventID );
				}
				
				theTrgMap.setMap( e_txtKind );
				tskRdsEvent.c_AutoReTrans.rcvTrg( e_txtKind );			/* 発信トリガ受信時の自動再発信状態リセットに関する処理 */
				theRdsEmail.c_ResChk.rcvTrg( e_txtKind );				/* 発信トリガ受信時の応答の確認状態リセットに関する処理 */
				
				break;
			}
		}
	}
}

/***************************************************************************************************
 *   処理関数 
 ***************************************************************************************************/
/*==================================================================================================*/
//	テキスト作成関数をコールする関数 
/*==================================================================================================*/
/*------------------------------------------------------------------------------------------------*/
RDSC_mkText_Mode* const p_ModeText_C_Array[] = 
{
	&c_mkText_ModeSqrmbl,										// 緊急発信 
	&c_mkText_ModeSqrmblRecv,									// 緊急回復発信 
	&c_mkText_ModeWarn,											// 本体警告発信 
	&c_mkText_ModeWarnRecvBody,									// 本体警告解除発信 
	&c_mkText_ModeIni,											// 初期発信 
	&c_mkText_ModeMnteStrt,										// メンテ開始発信 
	&c_mkText_ModeMnteEnd,										// メンテ終了発信 
	&c_mkText_ModeUserCall,										// 管理者発信 
	&c_mkText_ModeTimeOut,										// 期日指定発信 
	&c_mkText_ModeResOther,										// 応答発信(Dumy) 
	&c_mkText_ModeResRDS,										// 応答発信 
	&c_mkText_ModeAuthInfoChange,								// MFP認証情報変更発信 Minerva UV-00884 MFP認証情報変更発信追加対応 sone.
	&c_mkText_FinishProCer,										// プロダクト認証設定の変更完了通知 Minerva UV-00781 プロダクト認証対応 sone.
	&c_mkText_ModeDebugLogSave,									// デバッグログ手動蓄積完了報告発信.	// 2010/08/24 higuchi Minerva UV-00794 【デバッグログ収集】.
	&c_mkText_ModeExpImpComplete,								// エクスポート・インポート完了通知発信 // 2010/07/29 yito UV-00886 Exp/Imp.
	&c_mkText_ModeTrblResetStatus,								// トラブル解除状態通知発信	2010/09/13 Minerva UV-00898追加対応 kita 追加.
	&c_mkText_Result_Lic,										// ライセンス機能の完了通知 Minerva UV-00683 sone.
	&c_mkText_ModeIISWExStatus,									// InternetISW実行状態通知 2010/12/07 Minerva UV-00971 kita 追加.
	&c_mkText_ModeTemporaryRescue0,								// 不良箇所切り離し発信.		// 2011/12/01 amiya ZeusMLK UV-01170 暫定救済中のCSRCへの通知.
	&c_mkText_ModeTemporaryRescue1,								// 不良箇所切り離し発信.		// 2011/12/01 amiya ZeusMLK UV-01170 暫定救済中のCSRCへの通知.
	&c_mkText_ModeTemporaryRescueRelease,						// 不良箇所切り離し解除発信.	// 2011/12/01 amiya ZeusMLK UV-01170 暫定救済中のCSRCへの通知.
	&c_mkText_ModeDebugLogNetResult,							// デバッグログネット配信結果報告発信.	// 2012/03/19 higuchi Minerva/Zeus UV-01225 【デバッグログ拡張機能】.
	&c_mkText_ModeRmpStartResult,								// リモートパネル起動結果発信 2013/12/05 OPS1前田(Sky) ZeusCD UV-01387_リモートパネルの改善(WP213).
	&c_mkText_ModeRemoteLogResult,								// 遠隔ログ取得結果報告発信 2013/10/19 mtoyoda UV-01382 遠隔でのデバッグログ取得の改善対応.
// 2014/09/03 ktakahashi ZeusS 制御構成変更 No.4 HDDミラーリング Start.
	&c_mkText_ModeWarnRedunParts,								// 冗長化パーツ警告発信.
	&c_mkText_ModeWarnRecvRedunParts,							// 冗長化パーツ警告解除発信.
// 2014/09/03 ktakahashi ZeusS 制御構成変更 No.4 HDDミラーリング End.
// 2015/06/05 mtoyoda Helios KRQ-101_認証機能有効化対応 START.
	&c_mkText_ModeAuthFunctionError,							// 認証機能有効化のエラー通知発信.
// 2015/06/05 mtoyoda Helios KRQ-101_認証機能有効化対応 END.
// 2016/09/08 T.Komatsu IT5_4.0 RQ-505_バックアップ機能強化(WP267) START.
	&c_mkText_ExtMfpBackupServerInfo,							// データバックアップエラー通知発信.
// 2016/09/08 T.Komatsu IT5_4.0 RQ-505_バックアップ機能強化(WP267) END.
	&c_mkText_ModeCsraSettingState,								// CSRA設定状態発信.	// 2017/07/20 [MAv5][RQ-XXX] CSRA設定状態発信.
// 2017/08/08 [MAv5][RQ-XXX] CSRC・CSRAデータチェック Start.
	&c_mkText_ModeCmdGenError_CSRC,								// コマンド生成エラー通知（CSRC）.
	&c_mkText_ModeCmdGenError_CSRA_Daily,						// コマンド生成エラー通知（CSRA日毎）.
	&c_mkText_ModeCmdGenError_CSRA_Weekly,						// コマンド生成エラー通知（CSRA週毎）.
	&c_mkText_ModeCmdGenError_CSRA_Monthly,						// コマンド生成エラー通知（CSRA月毎）.
// 2017/08/08 [MAv5][RQ-XXX] CSRC・CSRAデータチェック End.

	&c_mkText_ModePartsReplace,									// 新品交換発信 
	&c_mkText_ModeResOther,										// 応答発信 
	NULL														// （終端） 
};
/*------------------------------------------------------------------------------------------------*/
void RDSC_MakeEventData::makeText()
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"#[44]RDSC_MakeEventData::makeText[st]" );
	register uchar indx=0;

	// ■ E-mail 固有 ~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~ 
	if( ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
	 || ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )		//HTTP版　寺内(08/01/31) 新規追加.
	 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		RDSC_MakeEventData::makeFrameNo1();
	}

// 2016/05/19 ktakahashi [ZeusSBK] CQ529066  緊急発信と緊急回復発信の入れ違い問題 Start.
	// 緊急発信と緊急回復発信の両方の発信マップが立っている場合に、発生時間順に並び替える対応を行う.
	// 緊急発信と緊急回復発信以外のトリガについては並び替えは行わない（局所的な対応となる）.
	Bool bSendOrder_SCSR = False;	// 緊急発信、緊急回復発信の順番入れ替えフラグ.
	// 緊急発信と緊急回復発信の両方の発信マップが立っているかチェック.
	if( ( False != b_Map[TYPE_RdsTrgKindTrbl] ) && ( False != b_Map[TYPE_RdsTrgKindTrblRecv] ) ){

		// 緊急発信のトリガ時間取得.
		uchar uc_EventID_Trbl[6+1] = { 0 };
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindTrbl, uc_EventID_Trbl );

		// 緊急発信のIDから時刻に変換.
		RDSC_Time c_rdsTime_Trbl;
		c_rdsTime_Trbl.cnvString_2_Time( uc_EventID_Trbl, RDSE_T2S_BCDyymmddhhmmss );

		// 緊急回復発信のトリガ時間取得.
		uchar uc_EventID_TrblRecv[6+1] = { 0 };
		pc_RdsSetting->getTrgTime( TYPE_RdsTrgKindTrblRecv, uc_EventID_TrblRecv );

		// 緊急回復発信のIDから時刻に変換.
		RDSC_Time c_rdsTime_TrblRecv;
		c_rdsTime_TrblRecv.cnvString_2_Time( uc_EventID_TrblRecv, RDSE_T2S_BCDyymmddhhmmss );

		printEpTime( (RDSS_DetailTime)c_rdsTime_Trbl, (uchar*)"c_rdsTime_Trbl" );			// 緊急発信時刻ログ出力.
		printEpTime( (RDSS_DetailTime)c_rdsTime_TrblRecv, (uchar*)"c_rdsTime_TrblRecv" );	// 緊急回復発信時刻ログ出力.

		// 緊急回復発信時刻をRDSS_DetailTime型に変換（時刻比較APIを使う為）.
		RDSS_DetailTime c_deTime_TrblRecv = { 0 };
		c_deTime_TrblRecv.year		= c_rdsTime_TrblRecv.year;
		c_deTime_TrblRecv.month		= c_rdsTime_TrblRecv.month;
		c_deTime_TrblRecv.day		= c_rdsTime_TrblRecv.day;
		c_deTime_TrblRecv.hour		= c_rdsTime_TrblRecv.hour;
		c_deTime_TrblRecv.minute	= c_rdsTime_TrblRecv.minute;
		c_deTime_TrblRecv.second	= c_rdsTime_TrblRecv.second;

		// 緊急発信とトリガ時間が、緊急回復発信のトリガ時間よりも後の場合.
		if( c_rdsTime_Trbl >= c_deTime_TrblRecv ){
			bSendOrder_SCSR = True;		// 「緊急発信、緊急回復発信の順番入れ替えフラグ」を立てる.
			EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)bSendOrder_SCSR, (uchar*)"#[44]RDSC_MakeEventData::makeText[bSendOrder_SCSR]" );
		}
	}
// 2016/05/19 ktakahashi [ZeusSBK] CQ529066  緊急発信と緊急回復発信の入れ違い問題 End.

	while( NULL != p_ModeText_C_Array[indx] )
	{
		if( True == b_Map[indx] ){
			if( indx != p_ModeText_C_Array[indx]->e_txtKind )	// プロテクト 
			{
				EPN_DBG_MESSAGE( (uchar*)"CSRC", indx, (uchar*)" エラー" );
				break;
			}
			EPN_DBG_MESSAGE((uchar*)"CSRC", indx, (uchar*)"p_ModeText_C_Array indx ");			//2018/10/18 FPT-DucDH4 Add log debug
// 2016/05/19 ktakahashi [ZeusSBK] CQ529066  緊急発信と緊急回復発信の入れ違い問題 Start.
			// 「緊急発信トリガ」かつ、「緊急発信、緊急回復発信の順番入れ替えフラグ」が立っている場合.
			if( ( TYPE_RdsTrgKindTrbl == indx ) && ( False != bSendOrder_SCSR ) ){
				makeData( p_ModeText_C_Array[TYPE_RdsTrgKindTrblRecv] );	// 「緊急回復発信トリガ」のデータを作成する.
			}
			// 「緊急回復発信トリガ」かつ、「緊急発信、緊急回復発信の順番入れ替えフラグ」が立っている場合.
			else if( ( TYPE_RdsTrgKindTrblRecv == indx ) && ( False != bSendOrder_SCSR ) ){
				makeData( p_ModeText_C_Array[TYPE_RdsTrgKindTrbl] );	// 「緊急発信トリガ」のデータを作成する.
			}
			// 発信トリガに応じたデータを作成する（既存処理）.
			else{
				makeData( p_ModeText_C_Array[indx] );
			}
// 2018/10/18 FPT-DucDH4 IT6_1.0[Eagle] 機器構成変更_No.92 Start.
// 2018/12/05 FPT-ChiNM IT6_1.0 【機器構成変更_No.92】 Modification Start.
			//if (p_ModeText_C_Array[indx]->e_txtKind == TYPE_RdsTrgKindWarnBody)
			if (p_ModeText_C_Array[indx]->e_txtKind == TYPE_RdsTrgKindWarnBody && isMakeTMData == True)
			{
				EPN_DBG_MESSAGE((uchar*)"CSRC", indx, (uchar*)"RDSC_MakeEventData::makeText() makeWarnDataTM");
				//c_mkText_ModeWarn.makeWarnDataTM(p_ModeText_C_Array[indx]->e_txtKind);
				isMakeTMData = False;
				c_mkText_ModeWarn.makeWarnDataTM();
			}
// 2018/12/05 FPT-ChiNM IT6_1.0 【機器構成変更_No.92】 Modification End.
// 2018/10/18 FPT-DucDH4 IT6_1.0[Eagle] 機器構成変更_No.92 End.
// 2016/05/19 ktakahashi [ZeusSBK] CQ529066  緊急発信と緊急回復発信の入れ違い問題 End.
		}
		indx++;
	}
	
	// 2012/03/08 amiya 新ログシステム START.
	char sc_Text[_S_EventDataBuff] = {0,};
	short ss_Num = c_EvevntDataBuf.isNum();						// データ数取得.
	short ss_int = 0;
	for( ss_int = 0; ss_int < ss_Num; ss_int++ )
	{
		c_EvevntDataBuf.getMakeTextData( ss_int, sc_Text );		// テキストファイルの取得.
		DBG_CsrcMakeText( ss_int, sc_Text, __FILE__ ,__LINE__ );	// .
	}
	// 2012/03/08 amiya 新ログシステム END.
}


/*==================================================================================================*/
//	トリガ受付け関数 
/*==================================================================================================*/
short RDSC_MakeEventData::indTrg( TYPE_RdsTrgKind trg, uint32_t para )
{
	/* （１）センターの登録状況に関係なく受付ける発信 */
	switch( trg )
	{
		/* 初期発信 */
		case TYPE_RdsTrgKindIni :
			rcvTrg( &c_mkText_ModeIni );
			e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
			break;
		/* [E-mail版]応答発信（KMRDS） */
		case TYPE_RdsTrgKindRes_RDS :
			rcvTrg( &c_mkText_ModeResRDS );
			e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
			break;	
		/* [E-mail版]応答発信（KMRDS以外） */
		case TYPE_RdsTrgKindUnKnowin :
			rcvTrg( &c_mkText_ModeResOther );
			e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
			break;	
		
		default :
			break;
	}
// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/22 Sky梶谷 START.
//	/* （２）センターが登録されている場合にのみ受付ける発信 */
//	if( TYPE_HostStatRgst == pc_RdsSetting->getHostStat() )
	/* （２）センターが登録されているまたはCSRAレポート発信が利用可能な場合に受付ける発信 */
	if ( ( TYPE_HostStatRgst == pc_RdsSetting->getHostStat() ) ||
		 ( True == RDSC_RptDataManager::newInstance()->isAvailableSendReport() ) )
// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/22 Sky梶谷 END.
	{
		switch( trg )
		{
			/* メンテ開始発信 */
			case TYPE_RdsTrgKindMnteStart :
				rcvTrg( &c_mkText_ModeMnteStrt );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
				break;	
			/* メンテ終了発信 */
			case TYPE_RdsTrgKindMnteEnd :
				rcvTrg( &c_mkText_ModeMnteEnd );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
				break;
			/* 管理者発信 */
			case TYPE_RdsTrgKindManage :
				rcvTrg( &c_mkText_ModeUserCall );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
				break;
			/* 期日指定発信 */
			case TYPE_RdsTrgKindTimeOut :
				rcvTrg( &c_mkText_ModeTimeOut );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
				break;
			/* 緊急発信				param : トラブルコード */
			case TYPE_RdsTrgKindTrbl :
				rcvTrg( &c_mkText_ModeSqrmbl, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
				break;
			/* 緊急回復発信 */
			case TYPE_RdsTrgKindTrblRecv :
				rcvTrg( &c_mkText_ModeSqrmblRecv );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
				break;
			/* 本体警告発信			param : 発生した警告ビット */
			case TYPE_RdsTrgKindWarnBody :
				EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"#[17]RDSC_MakeEventData::indTrg[st]", (uint32_t)para );
				rcvTrg( &c_mkText_ModeWarn, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
				break;
			/* 本体警告解除発信		param : 解除した警告ビット */
			case TYPE_RdsTrgKindWarnRecvBody :
				rcvTrg( &c_mkText_ModeWarnRecvBody, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
				break;
			/* 応答タイムアウトによる再発信 */
			case TYPE_RdsTrgKindResTimeOut :
				rcvTrg( &c_mkText_ModeResTimeOut );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
				break;
			/* InternetISW への応答 param : エラーコード 04/11/18 浦崎  */
			case TYPE_RdsTrgKindResI_ISW :
				rcvTrg( &c_mkText_ModeResInternetISW, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
				break;
			/* InternetISW の結果通知発信 */
			case TYPE_RdsTrgKindTxResultI_ISW :
				rcvTrg( &c_mkText_ModeTxResultI_ISW, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
				break;	

			/* HB発信 */
			case TYPE_RdsTrgKindHB:
				// HB以外の発信要求がある、もしくはJOB実行中があればHB発信要求は無視する 08/06/25 芳賀.
// 2020/10/23 Sky二口 OP_BTS-28458 [RQ-811] START.
//				if (theTrgMap.chkMap( RDSE_TSK_ID_HTTP ) != 0 ||
//					    c_FND.chkMap( RDSE_TSK_ID_HTTP ) != 0 )
				if ( ( True == RDSC_CsraConnectionManager::newInstance()->isSending() )
		  	      || ( True == RDSC_CsrcConnectionManager::newInstance( RDSE_CommunicationType_Immediate )->isSending() ) )
				{
					EPN_DBG_MESSAGE( (uchar*)"CSRCforWW", (uint32_t)trg, (uchar*)"Ignored HB Trigger due to an immediate request or CSRA transmission" );	
					c_HBdetect.b_TransHB = False;							// HB発信フラグ = False
				}
// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/22 Sky梶谷 START.
				else if ( True == RDSC_RptDataManager::newInstance()->isAvailableSendReport() ) {
					EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)trg, (uchar*)"Ignored HB Trigger due to an report request" );	
					c_HBdetect.b_TransHB = False;							// HB発信フラグ = False
				}
// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/22 Sky梶谷 END.
				else if ( ( theTrgMap.chkMap( RDSE_TSK_ID_HTTP ) != 0 )
				       || ( c_FND.chkMap( RDSE_TSK_ID_HTTP ) != 0 ) )
				  
// 2020/10/23 Sky二口 OP_BTS-28458 [RQ-811] END.
				{
// 2011/10/14 higuchi Minerva/Zeus CQ:357094 【HTTP双方向通信でAEコマンドが送信できない場合がある】 Start.
					EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)trg, (uchar*)" 他発信がある状態でHBトリガを検出しました" );
					if( False == c_HBdetect.b_HbStatusLock )
					{
						// FQ-CSOPS-1100426対応 sone start.
						// 最大再送回数を超えていた場合は、MAPに他の発信があってもHB発信のフラグを立てる.
						// FAX/モデム通信時の最大回数の確認は不要.
						RDSE_TaskID taskID = theRdsApl.isRdsSystem();
						if( taskID == RDSE_TSK_ID_HTTP
							&& True == tskRdsEvent.c_AutoReTrans._email.checkOverRetryCount() )
						{
							EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)trg, (uchar*)" 他の発信があるが、HBトリガを検出しました" );
							e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
							c_HBdetect.b_HbStatusLock = True;						// HBステータスロック = True.
							c_HBdetect.b_TransHB = True;							// HB発信フラグ = True.
						}
						else
						{
							EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)trg, (uchar*)" 他発信を優先したため、HBトリガを無視しました" );	
							c_HBdetect.b_TransHB = False;							// HB発信フラグ = False
						}
						// FQ-CSOPS-1100426対応 sone end.
					}
// 2011/10/14 higuchi Minerva/Zeus CQ:357094 【HTTP双方向通信でAEコマンドが送信できない場合がある】 End.
				}
				else
				{
					EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)trg, (uchar*)" HBトリガを検出しました" );	
					e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
					c_HBdetect.b_HbStatusLock = True;						// HBステータスロック = True.
					c_HBdetect.b_TransHB = True;							// HB発信フラグ = True.
				}
				break;  
			
			/* 新品交換発信 */
			case TYPE_RdsTrgKindPartsReplace :
				
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"新品交換発信rcvtrgをコール.");
				
				rcvTrg( &c_mkText_ModePartsReplace, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
				break;
				
			// Minerva UV-00884 MFP認証情報変更発信追加対応 sone 2010/6/21 START.
			case TYPE_RdsTrgKindAuthInfoChange :
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"MFP認証情報変更発信rcvtrgをコール.");
			
				rcvTrg( &c_mkText_ModeAuthInfoChange, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
				break;
			// Minerva UV-00884 MFP認証情報変更発信追加対応 sone 2010/6/21 END.
			
			// Minerva UV-00781 プロダクト認証対応 sone 2010/08/27 START.
			case TYPE_RdsTrgKindProductCertification :
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"プロダクト認証設定の変更完了通知rcvtrgをコール.");
			
				rcvTrg( &c_mkText_FinishProCer, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
				break;
			// Minerva UV-00781 プロダクト認証対応 sone 2010/08/27 END.

// 2010/08/24 higuchi Minerva UV-00794 【デバッグログ収集】 Start.
			case TYPE_RdsTrgKindDebugLogSave:				// デバッグログ手動蓄積完了報告.
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"デバッグログ手動蓄積完了報告発信rcvtrgをコール.");
				rcvTrg( &c_mkText_ModeDebugLogSave, 0 );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用).
				break;
// 2010/08/24 higuchi Minerva UV-00794 【デバッグログ収集】 End.
			// 2010/07/29 yito UV-00886 Exp/Imp start.
			case TYPE_RdsTrgKindExpImpComplete :
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"エクスポート完了通知発信rcvtrgをコール.");
			
				rcvTrg( &c_mkText_ModeExpImpComplete, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
				break;
			// 2010/07/29 yito UV-00886 Exp/Imp end  .
			
// 2010/09/13 Minerva UV-00898追加対応 kita Start.
			case TYPE_RdsTrgKindTrblResetStatus :
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"トラブル解除状態通知発信rcvtrgをコール.");
				
				rcvTrg( &c_mkText_ModeTrblResetStatus, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用).
				break;
// 2010/09/13 Minerva UV-00898追加対応 kita End.

			// Minerva UV-00683 sone START.
			case TYPE_RdsTrgKindResult_Lic :
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"ﾗｲｾﾝｽ処理の完了通知rcvtrgをコール.");
			
				rcvTrg( &c_mkText_Result_Lic, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用) 
				break;
			// Minerva UV-00683 sone END.
			
// 2010/11/19 Minerva UV-00971 kita Start.
			case TYPE_RdsTrgKindInternetISWExStatus :
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"InternetISW実行状態通知発信rcvtrgをコール.");
				
				rcvTrg( &c_mkText_ModeIISWExStatus, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用).
				break;
// 2010/11/19 Minerva UV-00971 kita End.
// 2011/12/01 amiya ZeusMLK UV-01170 暫定救済中のCSRCへの通知対応 START.
			case TYPE_RdsTrgKindTemporaryRescue0 :
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"不良箇所切り離し通知発信rcvtrgをコール.");
				
				rcvTrg( &c_mkText_ModeTemporaryRescue0, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用).
				break;
			case TYPE_RdsTrgKindTemporaryRescue1 :
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"不良箇所切り離し通知発信rcvtrgをコール.");
				
				rcvTrg( &c_mkText_ModeTemporaryRescue1, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用).
				break;
			case TYPE_RdsTrgKindTemporaryRescueRelease :
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"不良箇所切り離し解除通知発信rcvtrgをコール.");
				
				rcvTrg( &c_mkText_ModeTemporaryRescueRelease, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用).
				break;
// 2011/12/01 amiya ZeusMLK UV-01170 暫定救済中のCSRCへの通知対応 END.
// 2012/03/19 higuchi Minerva/Zeus UV-01225 【デバッグログ拡張機能】 Start.
			case TYPE_RdsTrgKindDebugLogNetResult:			// デバッグログネット配信結果報告発信.
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"デバッグログネット配信結果報告発信rcvtrgをコール.");
				rcvTrg( &c_mkText_ModeDebugLogNetResult, 0 );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用).
				break;
// 2012/03/19 higuchi Minerva/Zeus UV-01225 【デバッグログ拡張機能】 End.
// 2013/12/05 OPS1前田(Sky) ZeusCD UV-01387_リモートパネルの改善(WP213) Start.
			case TYPE_RdsTrgKindRmpStartResult:			// リモートパネル起動結果発信.
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"リモートパネル起動結果発信rcvtrgをコール.");
				rcvTrg( &c_mkText_ModeRmpStartResult, 0 );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用).
				break;
// 2013/12/05 OPS1前田(Sky) ZeusCD UV-01387_リモートパネルの改善(WP213) End.

// MA2 UV-01382 「遠隔でのデバッグログ取得の改善」対応 2013/10/19 mtoyoda START.
			case TYPE_RdsTrgKindRemoteLogResult:			// 遠隔ログ取得結果報告発信.
				EPN_DBG_MESSAGE( (uchar*)"DBG", (uint32_t)0, (uchar*)"遠隔ログ取得結果報告発信rcvtrgをコール." );
				rcvTrg( &c_mkText_ModeRemoteLogResult, 0 );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用).
				break;
// MA2 UV-01382 「遠隔でのデバッグログ取得の改善」対応 2013/10/19 mtoyoda END.

// 2014/09/03 ktakahashi ZeusS 制御構成変更 No.4 HDDミラーリング Start.
			case TYPE_RdsTrgKindWarnRedunParts:				// 冗長化パーツ警告発信.
				EPN_DBG_MESSAGE( (uchar*)"DBG", (uint32_t)0, (uchar*)"冗長化パーツ警告発信rcvtrgをコール." );
				rcvTrg( &c_mkText_ModeWarnRedunParts, 0 );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用).
				break;

			case TYPE_RdsTrgKindWarnRecvRedunParts:			// 冗長化パーツ警告解除発信.
				EPN_DBG_MESSAGE( (uchar*)"DBG", (uint32_t)0, (uchar*)"冗長化パーツ警告解除発信rcvtrgをコール." );
				rcvTrg( &c_mkText_ModeWarnRecvRedunParts, 0 );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用).
				break;
// 2014/09/03 ktakahashi ZeusS 制御構成変更 No.4 HDDミラーリング End.

// 2015/06/05 mtoyoda Helios KRQ-101_認証機能有効化対応 START.
			case TYPE_RdsTrgKindAuthFunctionError:			// 認証機能有効化のエラー通知発信.
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"Call rcvTrg(TYPE_RdsTrgKindAuthFunctionError).");
				rcvTrg( &c_mkText_ModeAuthFunctionError, 0 );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用).
				break;
// 2015/06/05 mtoyoda Helios KRQ-101_認証機能有効化対応 END.

// 2016/09/08 T.Komatsu IT5_4.0 RQ-505_バックアップ機能強化(WP267) START.
			case TYPE_RdsTrgKindExtMfpBackupServerInfo:		// データバックアップエラー通知発信.
				EPN_DBG_MESSAGE((uchar*)"DBG", 0, (uchar*)"Call rcvTrg(TYPE_RdsTrgKindExtMfpBackupServerInfo).");
				rcvTrg( &c_mkText_ExtMfpBackupServerInfo, 0 );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用).
				break;
// 2016/09/08 T.Komatsu IT5_4.0 RQ-505_バックアップ機能強化(WP267) END.

// 2017/07/20 [MAv5][RQ-XXX] CSRA設定状態発信 Start.
			case TYPE_RdsTrgKindCsraSettingState:			// CSRA設定状態発信.
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"CSRA設定状態発信rcvtrgをコール.");
				rcvTrg( &c_mkText_ModeCsraSettingState, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用).
				break;
// 2017/07/20 [MAv5][RQ-XXX] CSRA設定状態発信 End.
// 2017/08/08 [MAv5][RQ-XXX] CSRC・CSRAデータチェック Start.
			case TYPE_RdsTrgKindCmdGenError_CSRC:			// コマンド生成エラー通知（CSRC）.
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"コマンド生成エラー通知（CSRC）rcvtrgをコール.");
				rcvTrg( &c_mkText_ModeCmdGenError_CSRC, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用).
				break;
			case TYPE_RdsTrgKindCmdGenError_CSRA_Daily:		// コマンド生成エラー通知（CSRA日毎）.
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"コマンド生成エラー通知（CSRA日毎）rcvtrgをコール.");
				rcvTrg( &c_mkText_ModeCmdGenError_CSRA_Daily, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用).
				break;
			case TYPE_RdsTrgKindCmdGenError_CSRA_Weekly:	// コマンド生成エラー通知（CSRA週毎）.
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"コマンド生成エラー通知（CSRA週毎）rcvtrgをコール.");
				rcvTrg( &c_mkText_ModeCmdGenError_CSRA_Weekly, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用).
				break;
			case TYPE_RdsTrgKindCmdGenError_CSRA_Monthly:	// コマンド生成エラー通知（CSRA月毎）.
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"コマンド生成エラー通知（CSRA月毎）rcvtrgをコール.");
				rcvTrg( &c_mkText_ModeCmdGenError_CSRA_Monthly, para );
				e_Webdavtask.WebTrg = trg;	//トリガ格納 (HTTP用).
				break;
// 2017/08/08 [MAv5][RQ-XXX] CSRC・CSRAデータチェック End.

			default :
				break;
		}
	}

	return( 0 );
}

// 2016/03/23 ktakahashi [ZeusS_ZX0]CQ526327_E-mail自動初期発信ありでCSRC設定のインポート・エクスポート後にボタンクリック不可 Start.
/*==================================================================================================*/
// トリガー時間のEventIDを設定する.
/*==================================================================================================*/
void RDSC_MakeEventData::setTrgEventID( const uchar* puc_EventID )
{
	if( NULL == puc_EventID ){
		EPN_DBG_MESSAGE((uchar*)"", (uint32_t)0, (uchar*)"RDSC_MakeEventData::setTrgEventID() error puc_EventID is NULL");
		return;
	}

	// EventIDを設定.
	snprintf( (char*)uc_TrgEventID, sizeof( uc_TrgEventID ), "%s", puc_EventID );
}

/*==================================================================================================*/
// トリガー時間のEventIDを取得する.
/*==================================================================================================*/
void RDSC_MakeEventData::getTrgEventID( uchar* puc_EventID )
{
	if( NULL == puc_EventID ){
		EPN_DBG_MESSAGE((uchar*)"", (uint32_t)0, (uchar*)"RDSC_MakeEventData::getTrgEventID() error puc_EventID is NULL");
		return;
	}

	// EventIDを設定.
	snprintf( (char*)puc_EventID, sizeof( uc_TrgEventID ), "%s", uc_TrgEventID );
}
// 2016/03/23 ktakahashi [ZeusS_ZX0]CQ526327_E-mail自動初期発信ありでCSRC設定のインポート・エクスポート後にボタンクリック不可 End.


