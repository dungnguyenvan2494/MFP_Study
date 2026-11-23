//
//ファイル名：pcgadmin5_CLCD.cpp.
//
//概要:管理者機能.
//
#include "system/nvd/nvdg/NVDC_Setting.h"	// 音声対応
#include "panel/plibh/plibh_CLCD2/PNLC_UICustomDataManager.h"
#include "panel/plibh/plibhg/plibh.h"
#include "panel/plibh/plibhg/plibh2.h"
#include "pigadmin5_CLCD.h"
#include "pigadmin3_CLCD.h"
#include "pigadmin1_CLCD.h"
#include "pcgadmin5_CLCD.h"
#include "pipadmin5_CLCD.h"
#include "pipadmin1_CLCD.h"
#include "pipadmin3_CLCD.h"
#include "pcgadmin9_CLCD.h"
#include "pigadmin9_CLCD.h"
#include <panel/pcgutil/pcgutil_CLCD/piputility_CLCD.h>
#include <panel/pcgbox/pcgbox_CLCD/pipbox_CLCD.h>
#include <panel/pcgutil/pcgutil_CLCD/pigutility_CLCD.h>
#include <panel/pdp/pdp_CLCD/pdpdef_CLCD.h>
// comment 2015/10/30 HCL_SunilNP [ZeusS_ZX0][制御構成変更_No.76-1] Adding error code.
#include <panel/pdp/pdpg/pdp_DetailCode.h>
#include <panel/plibh/plibhg/pdpcomb.h>
#include <panel/pdp/pdpg/pdpdef.h>
#include <panel/pcgwarn/pcgwarng/pctwarng.h>
#include <panel/pddtext/pddtext_CLCD/pstr_CLCD.h>
#include <panel/pddtext/pddtext_CLCD/psstr_CLCD.h>
#include <panel/pddtext/pddtext_CLCD/pfstr_CLCD.h>
#include <panel/pcgwarn/pcgwarn_CLCD/pigwarn1_CLCD.h>
// comment 2015/03/17 HCLT-Ezhil [HeliosL][KRQ-101] Authentication function code Enable setting screen.
#include <panel/pcgutil/pcgutil_CLCD/pigutility3_CLCD.h>
#include <panel/pcgscanfax/pcgscanfax_CLCD/pigscanfax_CLCD.h>
#include "system/nvd/APIC_BoxSetting.h"
// 2016/01/08 SecV2-6 【管理者による特定機能制御技術】 Sky 新道.
#include "system/doc/APIC_Box.h"
#include <system/doc/docg/APIC_BoxRegistryImp.h>
#include "system/nvd/APIC_Setting.h"
#include <panel/pdp/pdpg/pdptblg.h>
#include <panel/plibh/plibhg/plibhCommonKeybord.h>		// キーボード処理共通化.
// comment 2015/08/28 HCLT_SunilNP [ZeuS_ZX0][RQ-163] Addition of USB接続許可設定.
#include <panel/plibh/plibh_CLCD2/PNLC_QWSConnector.h>
#include <panel/pcgutil/pcgutil_CLCD/piputility3_CLCD.h>
#include <system/prv/APIC_PreviewProcessFileAccess.h>
#include <system/saa/APIC_UsbMemDeviceInfo.h>			// 【UV-00969】USBインポート/エクスポート対応 酒井健.
#include <panel/plibh/plibhg/plibh3.h>	//DonauLastFum【ネットワーク通信文字拡張】【キーボードバッファ対応】【13原】2009/07/13 add.
#include <system/sys/APIC_FuncPatternSetting.h>		// ColorUIFUMv5 Sky北野 UV-00705 パターンによるパネルカスタマイズ.
#include <system/opl/APIC_OPL.h>																	// 送信操作ログ管理I/F.
#include <panel/plibh/plibh_CLCD2/PNLC_OperationLog.h>												// 操作ログ収集クラスヘッダ.
#include "system/sys/APIC_SystemServiceFunction.h"
#include <system/saa/APIC_SaaUsbMemService.h>
#include <panel/plibh/plibh_CLCD2/PNLC_BoxDocJobSpecControlLib.h>
#include <panel/pcgscanoap/pcgscanoapg/pccscanoap.h>			// [UV-00796]トップメニュー設定追加.
#include <panel/plibh/plibh_CLCD2/plibh_CLCD2.h>
//Comment 2017/09/04 [HCL-Swathi] [IT5 4.2][RQ-686] [WP732]Multiple Network support.
#include <panel/plibh/plibh_CLCD2/PNLC_NetworkLan.h>
#include <panel/pcgcom_ext/pcgcom_extg/PCGC_AplCommonSet.h>
#include "system/nvd/APIC_Setting.h"
#include "LAPC_APP_ActiveAppManager.h"
#include "LAPC_BFW_System.h"
#include "LAPC_APP_RegistAppDataList.h"
#include "LAPC_APP_RegistAppData.h"
#include <panel/pcgadm/pcgadm_CLCD/pipadmin2_CLCD.h>
#include <panel/plibh/plibhg/plibh.h>
extern void setExtDebugLogPanel( const char* fmt, ... );

#include "system/sys/APIC_OptionSetting.h"  // 2013/08/26 岡部(日立JTE) 搭載メモリ量変化時の応用機能オプション化自動切り替え対応.
#include <panel/plibh/plibh_CLCD2/PNLC_SlideMenuControlLib.h>//comment 2015/01/08 OPS1-Sky村井 [Poseidon][CQ508519] 設定変更更新フラグ追加漏れ対応.

// 2014/11/12 [MAv3][RQ-245] バックアップ機能強化（eMMC→HDD） eMMC内の管理データをHDDにバックアップする 日立 村上(幸).
#include "system/sys/APIC_BackupRestore.h"
// 2015/03/05 [MAv3][RQ-243]XMPPによるリアルタイム指示 日立 村上(幸).
#include "system/nvd/APIC_RoimWscDataIF.h"

// comment 2015/08/03 OPS1 Sky稲葉 [ZeusS_ZX0][RQ-383] 標準トップメニュー割付けメニューにMarket Placeを追加.
#include <panel/plibh/plibh_CLCD2/PNLC_ActionDataLib.h>

// comment 2016/01/29 OPS1-Sky松下 [ユビキタスFUM][Version4][RQ-501] 台数拡張対応.
#include <panel/plibh/plibh_CLCD2/PNLC_UbqDocListLib.h>
// comment 2016/01/11 HCLT-Suresh [ZeusS_ZX0][CQ521813] Including header for partsGetSizeGraphic.
#include <panel/ppcg/ppcg_CLCD/pidispd_CLCD.h>

// Comment KronosS 2017/01/18 SCD2_3G永田 管理No.135 LXKログ設定追加.
#ifdef DEF_KRONOSS_Cus
#include "system/nvd/APIC_LXKlog.h"
#include "net/tcpsocket/NETC_StarbucksLog.h"
#endif // DEF_KRONOSS_Cus.
// Comment 2018/02/01 【HCLT-Venkata】【AR1033484】 Adding admin user details to Network Type.
#include "system/jlg/APIC_JLGDefine.h"

extern PCGC_AplCommonSet AplCommonSet;
#define	DEF_ListNumMax				128		// ＴＳＩ受信振分け先キーの最大数
#if defined( DEF_MinervaBKCus ) || defined( DEF_MinervaBKEmu )
static const short ss_choiceNormalSetting		= 0;			// 薄紙モード定着性優先チョイス(通常).
static const short ss_choiceThinPaperSetting	= 1;			// 薄紙モード定着性優先チョイス(薄紙定着性優先).
static const uchar uc_rtbSelIndexNo				= 0;			// ローテーションボタンインデックス(しない).
static const uchar uc_rtbSelIndexYes			= 1;			// ローテーションボタンンデックス(する).
#endif	// defined( DEF_MinervaBKCus ) || defined( DEF_MinervaBKEmu ).
struct TYPS_PnlFtpAddress {
	char    sc_HostAddress[PCGD_FTP_HOSTNAME_MAX + 1];			// ホスト名(63byte).
	char    sc_Dir[PCGD_FTP_FOLDER_MAX*PCGD_BuffSize_UTF8 + 1];					// ファイルパス(127*3byte).
	char    sc_LoginName[PCGD_FTP_USERNAME_MAX*PCGD_BuffSize_UTF8 + 1 ];		// ユーザー名(63*3byte).
	char    sc_Password[PCGD_FTP_USERPASSWORD_MAX*PCGD_BuffSize_UTF8 + 1 ];		// パスワード(63*3byte).
	Bool    b_AnonymousOn;
	Bool    b_PassiveModeOn;
	Bool    b_ProxyOn;
	ushort  us_PortNo;
	TYPS_PnlFtpAddress() : b_AnonymousOn( False ), b_PassiveModeOn( False ), b_ProxyOn( False ), us_PortNo( 21 ) {
		strcpy( sc_HostAddress, "" );
		strcpy( sc_Dir, "" );
		strcpy( sc_LoginName, "" );
		strcpy( sc_Password, "" );
	}
};
struct TYPS_PnlSmbAddress {
	char    sc_HostAddress[PCGD_SMB_HOSTNAME_MAX + 1];			// ホスト名(255byte).
	char    sc_Dir[PCGD_SMB_FOLDER_MAX*PCGD_BuffSize_UTF8 + 1];					// ファイルパス(255byte).
	char    sc_LoginName[PCGD_SMB_LOGINNAME_MAX*PCGD_BuffSize_UTF8 + 1];		// ユーザー名(127byte).
	char    sc_Password[PCGD_SMB_LOGINPASSWORD_MAX*PCGD_BuffSize_UTF8 + 1];		// パスワード(127byte).

public:
	TYPS_PnlSmbAddress() {
		strcpy( sc_HostAddress, "" );
		strcpy( sc_Dir, "" );
		strcpy( sc_LoginName, "" );
		strcpy( sc_Password, "" );
	}
};
struct TYPS_PnlWebDavAddress {
	char	sc_HostAddress[PCGD_MaxStrWebDavHostAddress + 1];	// ホスト名(63byte).
	char	sc_Dir[PCGD_MaxStrWebDavDir*PCGD_BuffSize_UTF8 + 1];					// ファイルパス(142*3byte).
	char	sc_LoginName[PCGD_MaxStrWebDavLoginName*PCGD_BuffSize_UTF8 + 1];		// ユーザー名(63*3byte).
	char	sc_Password[PCGD_MaxStrWebDavPassword*PCGD_BuffSize_UTF8 + 1];			// パスワード(63*3byte).
	Bool	b_ProxyOn;
	ushort	us_PortNo;
	Bool	b_SslOn;
public:
	TYPS_PnlWebDavAddress() : b_ProxyOn( False ), us_PortNo( 8080 ), b_SslOn( False ) {
		strcpy( sc_HostAddress, "" );
		strcpy( sc_Dir, "" );
		strcpy( sc_LoginName, "" );
		strcpy( sc_Password, "" );
	}
};
// comment 2016/02/16 OPS1-Sky後藤 [ZeusSBK][RQ-496] 画質調整設定画面追加.
static const struct PNLS_setupImageAdjustColorBalance {
	PSinglePartsGraphic*			cp_Graph;		// ポインタ.
	short							ss_start_x;		// 表示開始位置.
	short							ss_start_y;		// 表示開始位置.
	short							ss_gain_x;		// 移動倍率.
	short							ss_gain_y;		// 移動倍率.
} tbl_setupImageAdjustColorBalance[PNLE_PrinterColorType_Max] = {
	{	&Gra_AdminPrinterColorBalance_YellowPoint,	PNLD_START_POINT_YELLOW_X,	PNLD_START_POINT_YELLOW_Y,	PNLD_GAIN_POINT_YELLOW_X,	PNLD_GAIN_POINT_YELLOW_Y	},	// TYPE_TonerColorYellow.
	{	&Gra_AdminPrinterColorBalance_MagentaPoint,	PNLD_START_POINT_MAGENTA_X,	PNLD_START_POINT_MAGENTA_Y,	PNLD_GAIN_POINT_MAGENTA_X,	PNLD_GAIN_POINT_MAGENTA_Y	},	// TYPE_TonerColorMagenta.
	{	&Gra_AdminPrinterColorBalance_CyanPoint,	PNLD_START_POINT_CYAN_X,	PNLD_START_POINT_CYAN_Y,	PNLD_GAIN_POINT_CYAN_X,		PNLD_GAIN_POINT_CYAN_Y		},	// TYPE_TonerColorCyan.
	{	&Gra_AdminPrinterColorBalance_BlackPoint,	PNLD_START_POINT_BLACK_X,	PNLD_START_POINT_BLACK_Y,	PNLD_GAIN_POINT_BLACK_X,	PNLD_GAIN_POINT_BLACK_Y		},	// TYPE_TonerColorBlack.
};
char* 					gl_psc_ColorImageBefore;			// 変更前画像情報.
// カラーバランスレベルゲージタイトル変更用.
static const struct PNLS_setupImageAdjustColorBalanceChangeTitle {
	PNLE_PrinterColorKind e_ColorKind;
	PNLE_PrinterColorType e_ColorType;
	ushort 				  us_Symbol;
	
} tbl_setupImageAdjustColorBalanceChangeTitle[] = {
	{ PNLE_PrinterColorKind_YMCK,	PNLE_PrinterColorType_YellowBlue, 	s_pITMYellowAdjustment_W376H016I 	}, // イエロー.
	{ PNLE_PrinterColorKind_YMCK,	PNLE_PrinterColorType_MagentaGreen, s_pITMMagentaAdjustment_W376H016I	}, // マゼンタ.
	{ PNLE_PrinterColorKind_YMCK,	PNLE_PrinterColorType_CyanRed, 		s_pITMCyanAdjustment_W376H016I		}, // シアン.
	{ PNLE_PrinterColorKind_YMCK,	PNLE_PrinterColorType_Black, 		s_pITMBlackAdjustment_W376H016I		}, // ブラック.
	{ PNLE_PrinterColorKind_RGB,	PNLE_PrinterColorType_YellowBlue, 	s_pITMBlueAdjustment_W376H016I		}, // レッド.
	{ PNLE_PrinterColorKind_RGB,	PNLE_PrinterColorType_MagentaGreen, s_pITMGreenAdjustment_W376H016I		}, // グリーン.
	{ PNLE_PrinterColorKind_RGB,	PNLE_PrinterColorType_CyanRed, 		s_pITMRedAdjustment_W376H016I		}, // ブルー.
	{ PNLE_PrinterColorKind_RGB,	PNLE_PrinterColorType_Black, 		s_pITMBlackAdjustment_W376H016I		}, // ブラック.
};
#define 				VALUETOINDEX( sc_value ) ( (uchar)( sc_value + PNLD_MOVE_GAUGE_STANDARD ) )
#define 				INDEXTOVALUE( uc_value ) ( (char)( uc_value - PNLD_MOVE_GAUGE_STANDARD ) )

extern Bool isConvertLengthCheckUTF8( char*, ushort, EP_CharSet );	// UTF8最大サイズチェック関数.
TYPS_PnlFtpAddress		s_ImageLogFTPAddressWork;					// 画像ログ設定用.
TYPS_PnlWebDavAddress	st_ImageLogWebDAVAddressWork;				// 画像ログ設定用(WebDAV).
TYPS_PnlSmbAddress		st_ImageLogSMBAddressWork;					// 画像ログ設定用(SMB).
TYPS_PnlFtpAddress		st_ImageLogFTPAddressFaxReceptionWork;		// 画像ログ設定用(FAX受信)(FTP).
TYPS_PnlWebDavAddress	st_ImageLogWebDAVAddressFaxReceptionWork;	// 画像ログ設定用(FAX受信)(WebDAV).
TYPS_PnlSmbAddress		st_ImageLogSMBAddressFaxReceptionWork;		// 画像ログ設定用(FAX受信)(SMB).
extern PCGC_SmbBrowsingManager c_SmbBrowsingMgr;					// SMBブラウジング管理クラス.
char				sc_StrURLSet[ PCGD_MaxKeybordInfoBuffer ];		// 文字列入力用.

static const char sc_FunctionSetting		= 0;	// 機能設定.
static const char sc_CopyFunctionSetting 	= 1;	// コピー機能設定.
static const char sc_ScanFaxFunctionSetting = 2;	// スキャン/ファクス機能設定.
static const char sc_SystemBoxSetting		= 3;	// システムボックス機能設定.
static const char sc_CopyMemorySetting		= 4;	// コピー設定メモリ設定.
static const char sc_ScanFaxmemorySetting	= 5;	// スキャン/ファクス設定メモリ設定.
static const char sc_AppShortSetting		= 6;	// OpenAPIアプリショートカット設定.
static const char sc_GroupKeySetting		= 7;	// 登録アプリケーショングループ設定.
static const char sc_soundSetting			= 8;	// 音設定.
static const char sc_EcoSetting				= 9;	// Eco設定.
static const char sc_printMusicSetting		= 10;	// プリント楽譜.
static const char sc_GadgetSetting			= 11;	// ウィジェット設定.
static const char sc_SimpleSecuritySetup 	= 12;	// 簡単セキュリティー設定.
static const char PNLD_ShortCutKeySelectQRCode 	= 13;	// QRコード表示.
static const char sc_ControlMenu 			= 14;	// コントローラメニュー.
// comment 2015/03/04 OPS1-Sky榎本 [MinervaSBk][UV-01457]トップメニューへアドレス帳キー割付.
static const char PNLD_ShortCutKeySelectAddressBook		= 8;	// アドレス帳.

#define PNLD_TopMenuSettingFuncSelectDispMax 10		// １ページに表示される最大ボタン数.
#define PNLD_TopMenuSettingFuncSelectMAXPage 2		// 最大ページ数.
#define PNLD_TopMenuThameFileListTimeoutCnt ( 20 )	// Topメニュー背景カスタマイズ ファイルリストタイムアウト時間(単位 提起起床コール回数 ).
// 2015/07/10 SecV1-5 【管理者によるサービスモード制御技術】 Sky 新道.
// 管理者パスワード解除キー確認画面への遷移元画面情報.
#define PNLD_AdminPasswordReleaseKeyHyphenTotal		5		// 管理者パスワード解除キー区切り文字の合計数.
#define PNLD_AdminPasswordReleaseKeyDisplayBlock	5		// 管理者パスワード解除キー１ブロック表示文字数.
#define PNLD_AdminPasswordReleaseKeyPausePosition	(PNLD_AdminPasswordReleaseKeyDisplayBlock + 1)		// 管理者パスワード解除キー区切り含みの１ブロック表示文字数.
// 2015/09/29 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
#define PNLD_AdminMainStorageEncryptionSetChange	0	// 変更.
#define PNLD_AdminMainStorageEncryptionSetRelease	1	// 解除.
// 2016/01/08 SecV2-6 【管理者による特定機能制御技術】 Sky 新道.
#define PNLD_BoxOperationType_Public				1	// 共有ボックス削除.
#define PNLD_BoxOperationType_GroupDelete			2	// グループボックス削除.
#define PNLD_BoxOperationType_GroupChange			3	// グループボックス変更.
#define PNLD_BoxOperationType_SecureDoc				4	// セキュリティー文書ボックス削除.
#define PNLD_BoxOperationType_BulletinBoardDelete	5	// 掲示板ボックス削除.
#define PNLD_BoxOperationType_BulletinBoardChange	6	// 掲示板ボックス変更.
#define PNLD_BoxOperationType_PollingSend			7	// ポーリング送信ボックス削除.
#define PNLD_BoxOperationType_Relay					8	// 中継ボックス削除.
#define PNLD_BoxOperationType_Filing				9	// ファイリングナンバーボックス削除.

static const TYPE_EnableBoxSettingKind e_BoxType[PNLE_PAdminSecurityBoxLimitIndexMAX] = {	// ボックス種類.
	TYPE_EnableBoxSettingKindPublicBox,					// 共有ボックス.
	TYPE_EnableBoxSettingKindGroupBox,					// グループボックス.
	TYPE_EnableBoxSettingKindSecureDocBox,				// セキュリティー文書ボックス.
	TYPE_EnableBoxSettingKindBulletinBoardBox,			// 掲示板ボックス.
	TYPE_EnableBoxSettingKindPolingSendBox,				// ポーリング送信ボックス.
	TYPE_EnableBoxSettingKindRelayBox,					// 中継ボックス.
	TYPE_EnableBoxSettingKindFilingBox					// ファイリングナンバーボックス.
};
// 2016/02/11 OPS1-motohashi [SecV2-4] ストレージ暗号化ワード長選択画面追加.
// ストレージ暗号化ワードのキー選択状態.
#define PNLD_AdminStorageEncStrLenSel_Ascii20Char	(0)
#define PNLD_AdminStorageEncStrLenSel_Ascii64Char	(1)
// 2016/06/01 SCD2-Sky Shindo [SecV2] [CQ530312] 20文字選択時、セキュリティー強化モードを解除する.
//// ストレージ暗号化ワードの文字数設定値.
//#define PNLD_AdminStorageEncStrLenSel_MaxLen_Ascii20Char	(20)
//#define PNLD_AdminStorageEncStrLenSel_MaxLen_Ascii64Char	(64)

// 20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
#define PNLD_AdminStorageEncStrLenSel_ShiftCancelKey		(104)
void ans_comHDDDelete();

//================================================================================================
// 概要     : 外部関数：Fax画像ログ転送用FTPサーバー設定領域の取得.
//================================================================================================
TYPS_PnlFtpAddress* getImageLogFtpAddressFax( Bool b_Send )
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	if( False == pc_SoftDipSw->getSoftDipSwBit( 16, 3 ) && False == pc_SoftDipSw->getSoftDipSwBit( 4, 7 ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return NULL;
	}
	TYPS_PnlFtpAddress* pst_FtpAddress = ( ( False == b_Send ) ? &st_ImageLogFTPAddressFaxReceptionWork : &s_ImageLogFTPAddressWork );
	return pst_FtpAddress;
}
//================================================================================================
// 概要     : 外部関数：Fax画像ログ転送用WebDavサーバー設定領域の取得.
//================================================================================================
TYPS_PnlWebDavAddress* getImageLogWebDavAddressFax( Bool b_Send )
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	if( False == pc_SoftDipSw->getSoftDipSwBit( 16, 3 ) && False == pc_SoftDipSw->getSoftDipSwBit( 4, 7 ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return NULL;
	}
	TYPS_PnlWebDavAddress* pst_WebDavAddress = ( ( False == b_Send ) ? &st_ImageLogWebDAVAddressFaxReceptionWork : &st_ImageLogWebDAVAddressWork );
	return pst_WebDavAddress;
}
//================================================================================================
// 概要     : 外部関数：Fax画像ログ転送用SMBサーバー設定領域の取得.
//================================================================================================
TYPS_PnlSmbAddress* getImageLogSmbAddressFax( Bool b_Send )
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	if( False == pc_SoftDipSw->getSoftDipSwBit( 16, 3 ) && False == pc_SoftDipSw->getSoftDipSwBit( 4, 7 ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return NULL;
	}
	TYPS_PnlSmbAddress* pst_SmbAddress = ( ( False == b_Send ) ? &st_ImageLogSMBAddressFaxReceptionWork : &st_ImageLogSMBAddressWork );
	return pst_SmbAddress;
}

//////////////////////////////////////////////////////////////
// 概要 : イニシャライズ(50900600) : 画面開始処理.
//////////////////////////////////////////////////////////////
void PAdminInitializeChoiceData::startGamen()
{
	PAdminToSecuritySetcom::startGamen(); // 親画面のスタート処理.
	if( b_Confirm == False ) {	// イニシャライズ確認からの遷移.
		b_ChoiceJobLog			= False;	// ジョブ履歴フラグ.
		b_ChoiceCopyProgram		= False;	// コピープログラムフラグ.
		b_ChoiceNetworkSetting	= False;	// ネットワーク設定フラグ.
		b_ChoiceAddressReg		= False;	// 宛先登録フラグ.
		if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
			b_ChoiceDataCache		= False;	// キャッシュ/蓄積データフラグ.
		}
	}
	b_Confirm = False;	// イニシャライズ確認への遷移フラグ.
	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );
}
//////////////////////////////////////////////////////////////
// 概要 : イニシャライズ(50900600) : ボタン（押された）.
//////////////////////////////////////////////////////////////
void PAdminInitializeChoiceData::procButtonOn( uchar option )
{
	switch( option ) {
		case 0:	// ジョブ履歴キー.
			b_ChoiceJobLog = ( b_ChoiceJobLog == True ? False : True );
			break;
		case 1:	// コピープログラムキー.
			b_ChoiceCopyProgram = ( b_ChoiceCopyProgram == True ? False : True );
			break;
		case 2:	// ネットワーク設定キー.
			b_ChoiceNetworkSetting = ( b_ChoiceNetworkSetting == True ? False : True );
			break;
		case 3:	// 宛先登録キー.
			b_ChoiceAddressReg = ( b_ChoiceAddressReg == True ? False : True );
			break;
		// キャッシュ/蓄積データキー押下時の処理追加.
		case 4:	// キャッシュ/蓄積データキー.
			if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
				b_ChoiceDataCache = ( b_ChoiceDataCache == True ? False : True );
			}
			break;
		default:	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
	}
	setupGamen();
}
//////////////////////////////////////////////////////////////
// 概要 : イニシャライズ(50900600) : OKキー.
//////////////////////////////////////////////////////////////
void PAdminInitializeChoiceData::procOk( uchar )
{
// comment 2022/08/30 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
	ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
//	if( b_ChoiceJobLog == True			||
//	    b_ChoiceCopyProgram == True		||
//	    b_ChoiceNetworkSetting == True	||
//	    // イニシャライズ確認画面遷移条件に「キャッシュ/蓄積データ」を追加.
//	    // ただし、ZeusBK/Zeus2.1（機能拡張2）以降の時に有効.
//	    b_ChoiceAddressReg == True		||
//	    ( True == b_ChoiceDataCache && True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) )
//	{
//		// ネットワーク設定選択かつセキュリティー設定有効.
//		if( ( True == b_ChoiceNetworkSetting ) && ( True == PnlComGetSystemSetting()->getSecurityLevelUpOn() ) ) {
//			// 画面情報セット.
//			setDispInfo( PNLE_AdminInitializeChoiceData, this );
//			b_Confirm = True;								// 上階層からの遷移か判定フラグ.
//			nextGamen = &gp_AdminSecurityReinforceConfirm;	// セキュリティモード解除確認画面.
//		} else {
//			nextGamen = &gp_AdminInitializeChoiceDataClearConfirm;	// イニシャライズ確認.
//		}
//	} else {
//		if( True == PLBC_FuncVersionCheck::isOK_Type2() ) {
//			nextGamen = &gp_AdminSecurityScrtyDeleilP3;	// 管理者設定・セキュリティ設定・セキュリティ詳細3.
//		} else {
//			nextGamen = &gp_AdminSecurityScrtyDeleilP2;	// 管理者設定・セキュリティ設定・セキュリティ詳細2.
//		}
//	}
}
//////////////////////////////////////////////////////////////
// 概要 : イニシャライズ(50900600) : 表示画面のセットアップ.
//////////////////////////////////////////////////////////////
void PAdminInitializeChoiceData::setupGamen( short option )
{
	PAdminToSecuritySetcom::setupGamen(); // 親画面のセットアップ処理.

	if( True == PLBC_FuncVersionCheck::isOK_MiZe_Type2() ) {
		Key_AdminInitializeChoiceJobLog_MiZe.select( b_ChoiceJobLog );					// ジョブ履歴キー.
		Key_AdminInitializeChoiceCopyProgram_MiZe.select( b_ChoiceCopyProgram );		// コピープログラムキー.
		Key_AdminInitializeChoiceNetworkSetting_MiZe.select( b_ChoiceNetworkSetting );	// ネットワーク設定キー.
		Key_AdminInitializeChoiceAddressReg_MiZe.select( b_ChoiceAddressReg );			// 宛先登録キー.
		Key_AdminInitializeChoiceDataCache.select( b_ChoiceDataCache );					// キャッシュ/蓄積データキー.

		Key_AdminInitializeChoiceJobLog.showEnable( False );
		Key_AdminInitializeChoiceCopyProgram.showEnable( False );
		Key_AdminInitializeChoiceNetworkSetting.showEnable( False );
		Key_AdminInitializeChoiceAddressReg.showEnable( False );
	}
	else {
		Key_AdminInitializeChoiceJobLog.select( b_ChoiceJobLog );					// ジョブ履歴キー.
		Key_AdminInitializeChoiceCopyProgram.select( b_ChoiceCopyProgram );			// コピープログラムキー.
		Key_AdminInitializeChoiceNetworkSetting.select( b_ChoiceNetworkSetting );	// ネットワーク設定キー.
		Key_AdminInitializeChoiceAddressReg.select( b_ChoiceAddressReg );			// 宛先登録キー.

		Key_AdminInitializeChoiceJobLog_MiZe.showEnable( False );
		Key_AdminInitializeChoiceCopyProgram_MiZe.showEnable( False );
		Key_AdminInitializeChoiceNetworkSetting_MiZe.showEnable( False );
		Key_AdminInitializeChoiceAddressReg_MiZe.showEnable( False );
		Key_AdminInitializeChoiceDataCache.showEnable( False );
	}

}
//////////////////////////////////////////////////////////////
// 概要 : イニシャライズ(50900600) : ジョブ履歴フラグ取得.
//////////////////////////////////////////////////////////////
Bool PAdminInitializeChoiceData::getChoiceJobLog()
{
	return	b_ChoiceJobLog;			// ジョブ履歴フラグ.
}
//////////////////////////////////////////////////////////////
// 概要 : イニシャライズ(50900600) : コピープログラムフラグ取得.
//////////////////////////////////////////////////////////////
Bool PAdminInitializeChoiceData::getChoiceCopyProgram()
{
	return	b_ChoiceCopyProgram;	// コピープログラムフラグ.
}
//////////////////////////////////////////////////////////////
// 概要 : イニシャライズ(50900600) : ネットワーク設定フラグ取得.
//////////////////////////////////////////////////////////////
Bool PAdminInitializeChoiceData::getChoiceNetworkSetting()
{
	return	b_ChoiceNetworkSetting;	// ネットワーク設定フラグ.
}
//////////////////////////////////////////////////////////////
// 概要 : イニシャライズ(50900600) : 宛先登録フラグ取得.
//////////////////////////////////////////////////////////////
Bool PAdminInitializeChoiceData::getChoiceAddressReg()
{
	return	b_ChoiceAddressReg;		// 宛先登録フラグ.
}
//////////////////////////////////////////////////////////////
// 概要 : イニシャライズ(50900600) : キャッシュ/蓄積データフラグ取得.
//////////////////////////////////////////////////////////////
Bool PAdminInitializeChoiceData::getChoiceDataCache()
{
	return	b_ChoiceDataCache;		// キャッシュ/蓄積データフラグ.
}
//////////////////////////////////////////////////////////////
// 概要 : イニシャライズ(50900600) : 宛先登録フラグ取得.
//////////////////////////////////////////////////////////////
void PAdminInitializeChoiceData::setConfirmflag( Bool option )
{
	b_Confirm = option;		// 宛先登録フラグ.
}

//////////////////////////////////////////////////////////////
// 概要 : イニシャライズ電源OFF／ON催促(509806C0) : 画面開始処理.
//////////////////////////////////////////////////////////////
void PAdminInitializeChoiceDataPowerOffOn::startGamen()
{
	onlyPanel.bl_PowerOffOnMesage = True;	// 主電源OFF/ON画面から遷移させないためのフラグ.
	turnGamen = this;
	PAdminToSecuritySetcom::startGamen();
	pc_ActingNotification1.initialSPC( s_MSGAdminHDDOnceDataWriteDelPowerOffOn, 336, 212, 288, 216, True );
	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );
}
//////////////////////////////////////////////////////////////
// 概要 : イニシャライズ電源OFF／ON催促(509806C0) : 表示画面のセットアップ.
//////////////////////////////////////////////////////////////
void PAdminInitializeChoiceDataPowerOffOn::setupGamen( short option )
{
	PAdminToSecuritySetcom::setupGamen(); // 親画面のセットアップ処理.
	pc_ActingNotification1.SPCsetupGamen();
}
//////////////////////////////////////////////////////////////
// 概要 イニシャライズ電源OFF／ON催促(509806C0) :しおりの遷移可能不可能状態.
//////////////////////////////////////////////////////////////
Bool PAdminInitializeChoiceDataPowerOffOn::isEnableNextGamenBookmark()
{
	return False;
}

//================================================================================================
// 概要		:109806B0 暗号化／上書き優先設定変更確認 : 確定.
// 戻り値	:なし
// 備考		:
//================================================================================================
void PAdminHDDOnceDataWriteDeleteConfirmation::procOk( uchar option )
{
	if( b_Select == 1 ) {	// はいキー
		// 設定値の保存
		Bool _bl_isEraseOn = True;
		if( gp_AdminSecurityHDDManageOnceDataWriteDelete.uc_ModeSelect == 0 ) {	// 削除方式 OFF.
			_bl_isEraseOn = False;
		} else if( gp_AdminSecurityHDDManageOnceDataWriteDelete.uc_ModeSelect == 1 ) {	// 削除方式 モード1.
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	Start.
			PnlComGetSystemSetting()->setTempDataOverWriteErase( TYPE_TempDataOverWriteErase1, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	End.
		} else if( gp_AdminSecurityHDDManageOnceDataWriteDelete.uc_ModeSelect == 2 ) {	// 削除方式 モード2.
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	Start.
			PnlComGetSystemSetting()->setTempDataOverWriteErase( TYPE_TempDataOverWriteErase2, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	End.
		}
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	Start.
		PnlComGetSystemSetting()->setTempDataOverWriteEraseOn( _bl_isEraseOn , PnlComGetUser(), JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False, PnlComGetAuthorityID() );
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	End.
		// CommonAPIから優先設定を保存.
		if( gp_AdminSecurityHDDManageOnceDataWriteDelete.uc_PrioritySelect == 0 ) {
			PnlComGetSystemSetting()->setEncryptionOrOverWrite( True ) ;	//	暗号化優先.
		} else {
			PnlComGetSystemSetting()->setEncryptionOrOverWrite( False );	//	上書き優先.
		}
		nextGamen = &gp_AdminHDDCodingWordSettingPowerOffOn;	// 管理者設定・セキュリティ設定・HDD管理設定・一時データ上書き消去設定・設定確認・電源offon画面
	} else {	// いいえキー
		nextGamen = &gp_AdminSecurityHDDManageOnceDataWriteDelete;	// 管理者設定・セキュリティ設定・HDD管理設定・一時データ上書き消去.
	}
}
//================================================================================================
// 概要		: 109806B0 暗号化／上書き優先設定変更確認 : ボタン押下.
// 戻り値	:.
// 備考		:.
//================================================================================================
void PAdminHDDOnceDataWriteDeleteConfirmation::procButtonOn( uchar option )
{
	switch( option ) {
		case 0:		b_Select = 0;		break;		// いいえ.
		case 1:		b_Select = 1;		break;		// はい.
		default:						break;
	}
	setupGamen();
}
//================================================================================================
// 概要		:109806B0 暗号化／上書き優先設定変更確認 : 画面開始処理.
// 戻り値	:.
// 備考		:.
//================================================================================================
void PAdminHDDOnceDataWriteDeleteConfirmation::startGamen()
{
	PAdminSecurityHDDManageOnceDataWriteDelete::startGamen();
	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );
}
//================================================================================================
// 概要		:109806B0 暗号化／上書き優先設定変更確認 : 表示画面のセットアップ.
// 戻り値	:なし
// 備考		:
//================================================================================================
void PAdminHDDOnceDataWriteDeleteConfirmation::setupGamen( short option )
{
	Rbt_UtilityCheckYesNo.select( b_Select );	// 認証方式のキー表示.
	PAdminSecurityHDDManageOnceDataWriteDelete::setupGamen( option );
}
//================================================================================================
// 概要		: 10980810 HDDフォーマット: ボタン押下.
// 戻り値	:なし
// 備考		:
//================================================================================================
void PAdminSecurityHDDManageHDDFormat::procButtonOn( uchar option )
{
	switch( option ) {
		case 1:	b_Select = False;	break;	// はい.
		case 2:	b_Select = True;	break;	// いいえ.
		default:					break;
	}
	setupGamen();
}
//================================================================================================
// 概要		: 10980810 HDDフォーマット: 画面移動.
// 戻り値	:なし
// 備考		:
//================================================================================================
void PAdminSecurityHDDManageHDDFormat::procOk( uchar option )
{
	if( b_Select == False ) {	// はい を選択状態.
		nextGamen = &gp_AdminSecurityHDDManageDataDeleteNowAction;		// 管理者設定・セキュリティ設定・HDD管理設定・消去中共通.
		onlyPanel.bl_PowerOffOnMesage = True;	// 主電源OFF/ON画面から遷移させないためのフラグ
		turnGamen = NULL;
// comment 2019/05/08 SCD4 Sky西村 [Eagle/Sparrow 1.1MR 欧州リリース向けROM作成対応].
//		if( bl_HddEncryptionKeyNGAdminMove == False ) {
			// 通常は音声・動画インストール領域(ata04)はフォーマットしない.
			PnlComGetHddInfo()->part_renewHdd( ans_comHDDDelete );
// comment 2019/05/08 SCD4 Sky西村 [Eagle/Sparrow 1.1MR 欧州リリース向けROM作成対応].
//		}
//		else {
//			// 暗号化ﾜｰﾄﾞ不一致発生時は、全領域フォーマット.
//			PnlComGetHddInfo()->renewHdd( ans_comHDDDelete );
//		}
	} else {	// いいえ を選択状態.
		if( bl_HddEncryptionKeyNGAdminMove == False ) {
			PAdminToSecurityHDDManageSetcom::procCancel( option );
		} else {	// 暗号化ﾜｰﾄﾞ不一致発生時は解除方法画面へ.
			nextGamen = &gp_AdminFailureHDDCodingWord;				// 管理者モード・セキュリティ設定・HDD管理設定・暗号化ﾜｰﾄﾞ不一致設定.
		}
	}
}
//================================================================================================
// 概要		: 10980810 HDDフォーマット: 画面移動.
// 戻り値	:なし
// 備考		:
//================================================================================================
void PAdminSecurityHDDManageHDDFormat::startGamen()
{
	b_Select = True;	//最初は「いいえ」を選択.
	PAdminToSecurityHDDManageSetcom::startGamen();
	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );
}
//================================================================================================
// 概要		: 10980810 HDDフォーマット : 表示画面のセットアップ
// 戻り値	:なし
//================================================================================================
void PAdminSecurityHDDManageHDDFormat::setupGamen( short option )
{
// comment 2018/08/10 SCD2 K.Nishiki [RQ-698-41].
// 設定メニューのメニュー項目変更により、下記の処理は削除する.
//	//画面タイトルしおり表示切り替え.
//	if( bl_HddEncryptionKeyNGAdminMove == False ) {
//		// HDD管理設定画面から遷移してきた場合.
//		BookMark_UtilityAdminCom.changeStringData( 2, s_BKMSecuritySetting );							// セキュリティー\設定.
//		BookMark_UtilityAdminCom.changeStringData( 3, s_BKMStorageSetting );							// Storage Management Settings.s_BKMStorageSetting
//		BookMark_UtilityAdminCom.changeStringData( 4, s_BKMStorageFormat );							// Storage Format.s_BKMStorageFormat
//	} else {
//		//HDD暗号化ワード認証失敗時解除方法選択画面から遷移してきた場合.
//// comment 2015/01/06 【HCLT-Abhinav】 【CQ506826】 Display Storage Management Settings when HDD unmounted.
////			BookMark_UtilityAdminCom.changeStringData( 2, s_BKMHDDCodingSet );								// HDD暗号化\設定.
//			BookMark_UtilityAdminCom.changeStringData( 2, s_BKMStorageSetting );							// ストレージ管理設定.
//// comment 2014/07/23 OPS1 Abhinav 【Poseidon/HeliosL】 【制御構成変更_No.39】 Change message and button text to storage if HDD un-mounted.
//// comment 2014/09/12 HCLT-Vikash [Poseidon][CQ 502758] Deleting compiler switch as it is applicable for all IT5 model.
////#if defined(DEF_Zeus5BkCus) || defined(DEF_HeliosCus)		
//		BookMark_UtilityAdminCom.changeStringData( 3, s_BKMStorageFormat );							// Storage Format.s_BKMStorageFormat
//	}
	Rbt_AdminSecurityHDDManageFormatKey.select( b_Select );	// 認証方式のキー表示.
	PAdminToSecurityHDDManageSetcom::setupGamen( option );
//	//画面タイトルしおり表示切り替え.
//	if( bl_HddEncryptionKeyNGAdminMove == True ) {
//		Key_BookMarkUtilityAndAdmin[3].select( True );							// しおり(ユーティリティ/管理者)_04キー.
//		Gra_BookMarkArrowUtilityAndAdmin[3].showEnable( False );				// しおり(ユーティリティ/管理者)_05の上に配置する下矢印.
//		Key_BookMarkUtilityAndAdmin[4].showEnable( False );						// しおり(ユーティリティ/管理者)_05キー.
//	}
}
//================================================================================================
// 概要		:HDDフォーマット : しおりデータ取得処理
// 戻り値	:なし
// 備考		:
//================================================================================================
void PAdminSecurityHDDManageHDDFormat::procBookMark( uchar option )
{
	if( bl_HddEncryptionKeyNGAdminMove == True ) {	// 暗号化不一致エラー発生時.
		if( option == 0 ) {	// 設定メニューを押されたときは遷移.
			PAdminToSecurityHDDManageSetcom::procBookMark( option );
		} else {	// それ以外は利かせない.
			setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
		}
	} else {	// 通常時はしおりの制限なし.
		PAdminToSecurityHDDManageSetcom::procBookMark( option );
	}
}
//================================================================================================
// 概要		:HDDフォーマット : 定期起床
// 戻り値	:なし
// 備考		:
//================================================================================================
void PAdminSecurityHDDManageHDDFormat::procTimerWakeUp( uchar option )
{
	PAdminToSecurityHDDManageSetcom::procTimerWakeUp( option );
	if( nextGamen == NULL ) {	// 次画面がセットされていない場合.
		PanelLog( PNLD_LogKind_Admin, "getDocDeleteStatus: ", 0, 0, 0, 0, 0, 0 );
//		if( 0 != cp_DocList->getDocDeleteStatus() ) {	// ドキュメント削除中状態が「削除中」の場合、HDDフォーマット画面から抜ける.
		if( 0 != cp_DocList->getDocDeleteStatus()	// ドキュメント削除中状態が「削除中」の場合、HDDフォーマット画面から抜ける.
		 || False == APIC_VirusScan::newInstance()->canFinalize() ) {
			if( bl_HddEncryptionKeyNGAdminMove == True ) {	// 暗号化不一致エラー発生時.
				nextGamen = &gp_AdminFailureHDDCodingWord;		// 管理者モード・セキュリティ設定・HDD管理設定・暗号化ﾜｰﾄﾞ不一致設定.
			} else {
				nextGamen = &gp_AdminSecurityHDDManageSet;		// 管理者設定・環境設定・HDD管理設定メニュー.
			}
		}
	}
}
/////////////////////////////////////////////////////////////



// UIFUM5→MinervaZeusマージ↓.
#define PNLD_MaxHDDLogEncryptionPassword	(  20 )				//デバッグ情報暗号化設定パスワード最大文字数.
#define PNLD_AdminCompareResultSame			(   0 )				//デバッグ情報暗号化設定パスワードの比較結果が同じ.
#define PNLD_ChangeEnlKey					( 100 )				//標準/拡大キー押下時の値.
//------------------------------------------------------------------------------
/**
* @brief 80980920 デバッグ情報暗号化設定登録 : クリアー押下処理.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingRegist::procClear( uchar option )
{
	//ログファイル削除中は、押下無効.
	if( isRunning() ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	//パスワード再入力のクリアー処理をコール.
	PAdminKeyBoardCom::procClearPasswordReInput( option );
	setupGamen( option );
}

//------------------------------------------------------------------------------
/**
* @brief 80980920 デバッグ情報暗号化設定登録 : 戻るキー押下処理.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingRegist::procCancel( uchar option )
{
	//ログファイル削除中は、押下無効.
	if( isRunning() ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	//HDD管理設定画面に遷移.
	nextGamen = &gp_AdminSecurityHDDManageSet;
}

//------------------------------------------------------------------------------
/**
* @brief 80980920 デバッグ情報暗号化設定登録 : OKキー押下処理.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingRegist::procOk( uchar option )
{
	//ログファイル削除中は、押下無効.
	if( isRunning() ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	Bool b_isProhibit = False;//「禁止」設定フラグ.
	Bool b_isDifferent = False;//「不一致」設定フラグ.

	//パスワードキーのフラグをチェック(T:暗号化ワードキー/F:暗号化ワード確認入力キー).
	if( True == b_PasswordSelect ) {
		//暗号化ワードキーが選択されている時.
		strncpy( sc_StrKeyboard2, sc_StrKeyboard, sizeof( sc_StrKeyboard2 ) - 1 );
		sc_StrKeyboard2[sizeof( sc_StrKeyboard2 ) - 1] = '\0';
	} else {
		//暗号化ワード確認入力キーが選択されている時.
// // comment  2018/02/16 HCL_Neeraj [IT5_4.2] Klockwork Changes ID: 361977 [SV.STRBO.BOUND_COPY.UNTERM].
//		strncpy( sc_StrKeyboard2Check, sc_StrKeyboard, sizeof( sc_StrKeyboard ) );
		strncpy( sc_StrKeyboard2Check, sc_StrKeyboard, sizeof( sc_StrKeyboard2Check ) - 1 );
	}

	//現在設定されているパスワードを取得.
	char sc_nowPassword[ PNLD_MaxHDDLogEncryptionPassword + 1 ];
	memset( sc_nowPassword, 0x00, sizeof( sc_nowPassword ) );
	APIC_DebugLogSystemSetting* pc_DebugLogSystemSetting = APIC_DebugLogSystemSetting::newInstance();
	pc_DebugLogSystemSetting->getHDDLogEncryptionPassword( sc_nowPassword );

	//入力した暗号化ワードが現在の暗号化ワードと同じかをチェック.
	char sc_CompareResult = ( char )strcmp( sc_nowPassword, sc_StrKeyboard2 );
	if( sc_CompareResult == PNLD_AdminCompareResultSame ) {
		//一致していれば禁止フラグを立てる.
		b_isProhibit = True;
	}

	//20桁未満ではないかチェック.
	if( strlen( sc_StrKeyboard2 ) < PNLD_MaxHDDLogEncryptionPassword ) {
		//20桁未満であれば禁止フラグを立てる.
		b_isProhibit = True;
	}

	//同一文字のみのパスワードではないかチェック.
	Bool b_SameCheck = True;
	for( int cnt = 0; cnt < PNLD_MaxHDDLogEncryptionPassword; cnt++ ) {
		//一つ目の文字と違う文字が現れたらフラグを落とす.
		if( sc_StrKeyboard2[ cnt ] != sc_StrKeyboard2[ 0 ] ) {
			b_SameCheck = False;
		}
	}

	//禁止フラグに結果を設定.
	b_isProhibit = ( char )( b_isProhibit | b_SameCheck );

	//暗号化ワードと暗号化ワード確認入力が同じかチェック.
	sc_CompareResult = ( char )strcmp( sc_StrKeyboard2, sc_StrKeyboard2Check );
	if( sc_CompareResult != PNLD_AdminCompareResultSame ) {
		//一致していなければ不一致フラグを立てる.
		b_isDifferent = True;
	}

	//禁止フラグまたは不一致フラグが立っているかチェック.
	if( b_isDifferent || b_isProhibit ) {
		//報知音設定.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		//不一致フラグが立っているかチェック.
		if( b_isDifferent ) {
			//パスワードエラータイプに「不一致」を設定.
			e_PasswordError = PNLE_PasswordErrorType_CompareError;
		} else {
			//パスワードエラータイプに「禁止」を設定.
			e_PasswordError = PNLE_PasswordErrorType_ProhibitError;
		}
		//  クリアー処理.
		procClear( option );

		//  現在のキーボード状態(標準/拡大)を取得.
		if( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
			//拡大であれば、エラーメッセージを表示する為に標準へ変更.
			procButtonOn( PNLD_ChangeEnlKey );
		} else {
			setupGamen( option );
		}
	} else {
		//フラグが立っていない時、ログファイルの削除要求.
		reqSystemLogDelete();
	}
}

//------------------------------------------------------------------------------
/**
* @brief 80980920 デバッグ情報暗号化設定登録 : ボタン押下処理.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingRegist::procButtonOn( uchar option )
{
	//ログファイル削除中は、押下無効.
	if( isRunning() ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	//パスワード再入力のボタン押下処理をコール.
	PAdminKeyBoardCom::procButtonOnPasswordReInput( option );
	setupGamen( option );
}

//------------------------------------------------------------------------------
/**
* @brief 80980920 デバッグ情報暗号化設定登録 : リセットキー.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingRegist::procReset( uchar option )
{
	//ログファイル削除中は、押下無効.
	if( isRunning() ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PAdminKeyBoardCom::procReset( option );
}

//------------------------------------------------------------------------------
/**
* @brief 80980920 デバッグ情報暗号化設定登録 : 1秒タイマ(Sleepモードに移行させない為、1秒毎にキー操作通知を行う).
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingRegist::procSecond( uchar option )
{
	//その他キー操作を通知.
	ppc_Power->changePowerMode( TYPE_PowerModeEtsKeyOperation );
	PAdminKeyBoardCom::procSecond( option );
}

//------------------------------------------------------------------------------
/**
* @brief 80980920 デバッグ情報暗号化設定登録 : 定期起床処理.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingRegist::procTimerWakeUp( uchar option )
{
	PAdminKeyBoardCom::procTimerWakeUp( option );
	//ログ削除状態が正常終了かチェック.
	if( PNLE_SystemLogDeleteStatus_OK == m_e_SystemLogDeleteStatus ) {
		//ログ削除状態を未実行に変更.
		m_e_SystemLogDeleteStatus = PNLE_SystemLogDeleteStatus_Non;
		//暗号化ワードを保存.
		APIC_DebugLogSystemSetting* pc_DebugLogSystemSetting = APIC_DebugLogSystemSetting::newInstance();
		if( NULL != pc_DebugLogSystemSetting ) {
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	Start.
			pc_DebugLogSystemSetting->setHDDLogEncryptionPassword( sc_StrKeyboard2, JLGE_InspectLogOperateIDL_Admin, JLGE_InspectLogIfOperationPanel, False );
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	End.
		} else {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		}
		//HDD管理設定画面に遷移する.
		nextGamen = &gp_AdminSecurityHDDManageSet;
	}
}

//------------------------------------------------------------------------------.
/**
* @brief 80980920 デバッグ情報暗号化設定登録 : ユーザ補助キー押下処理.
*/
//------------------------------------------------------------------------------.
void PAdminLogSystemEncryptionSettingRegist::procAccessibility( uchar option )
{
	// キー押下受付判定.
	if( isRunning() ) {										// ※保存中、出力中はキー押下無効.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PAdminKeyBoardCom::procAccessibility( option );
}


//------------------------------------------------------------------------------
/**
* @brief 80980920 デバッグ情報暗号化設定登録 : デフォルト背景表示部品の設定.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingRegist::setDefaultBackParts()
{
	//キーボード切換え状態が拡大かつキーボード初期状態判定フラグがTrueかチェック.
	if( ( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) && ( True == onlyPanel.b_InitKeybord ) ) {
		//拡大用デフォルト背景を設定.
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminLogSystemEncryptionSettingRegistDefaultWideEnl, ss_AdminLogSystemEncryptionSettingRegistDefaultWideEnl );
	} else {
		//標準用デフォルト背景を設定.
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminLogSystemEncryptionSettingRegistDefaultWide, ss_AdminLogSystemEncryptionSettingRegistDefaultWide );
	}
}

//------------------------------------------------------------------------------
/**
* @brief 80980920 デバッグ情報暗号化設定登録 : 画面開始処理.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingRegist::startGamen()
{
	//パスワード再入力の画面開始処理をコール.
	PAdminKeyBoardCom::startGamenPasswordReInput();
	//ワイドキーボード初期化処理.(ワイドキーボードタイプ、画面のタイトルパーツを指定、特殊設定).
	PLBC_WideKeyboard::getInstance()->startWideKeyboard( PNLE_WideKeyboard_Normal, &Txt_AdminLogSystemEncryptionSettingTitle, PNLE_WideKeyboardExTwoEnterPassword );

	//初期表示の時のみ初期化.
	if( PLBD_Enl_KBD_Initial == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
//	2023/09/11 FPT-DuongDV11 【OP_BTS-44872】 fix strncpy-param-overlap.
		// memset( sc_StrKeyboard, 0x00, sizeof( sc_StrKeyboard ) );
		char sc_Password[PNLD_MaxHDDLogEncryptionPassword+1];
		memset( sc_Password, 0x00, sizeof( sc_Password ) );
		//キーボードにデータをセット(キーボードの初期文字列、キーボードの最大入力文字数、キーボードの幅(char単位)).
		// setKeyboardData( sc_StrKeyboard, PNLD_MaxHDDLogEncryptionPassword, TYPD_AdminPasswordReInputKeybordMax );
		setKeyboardData( sc_Password, PNLD_MaxHDDLogEncryptionPassword, TYPD_AdminPasswordReInputKeybordMax );
	}

	PAdminKeyBoardCom::startGamen();
	PLBC_WideKeyboard::getInstance()->initInputArea( &Kbd_UtilityInputKeybordConv, &c_SlideTextMgr, sc_StrKeyboard, ( char )Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MaxInputNum, pc_ActivatableField, getCharSetForPanel() ); //(キーボード、スライド文字列管理、キーボード文字列、キーボードの(標準/拡大)のMAX領域、最大入力文字数入力表示エリア、入力表示エリア、文字コード).

	//ユーティリティ専用キーボード共通開始処理.
	startGamenKeybordUtility9E();
	//キーボード初期状態判定フラグをFalseとする.
	onlyPanel.b_InitKeybord = False;
	//暗号化ワードキーのテキストを変更.
	Key_PasswordReInput.changeText( s_KEYCodingWord );
	//暗号化ワード確認入力キーのテキストを変更.
	Key_PasswordReInputCheck.changeText( s_KEYCodingWordConfirm );
	//ログ削除状態を未実行に初期化.
	m_e_SystemLogDeleteStatus = PNLE_SystemLogDeleteStatus_Non;
}

//------------------------------------------------------------------------------
/**
* @brief 80980920 デバッグ情報暗号化設定登録 : 画面描画.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingRegist::setupGamen( short option )
{
	//キーボード拡大でないかチェック.
	if( PLBD_Enl_KBD_Enlarge != Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
		PAdminKeyBoardCom::setupGamen( option );

		//メッセージの表示処理.
		switch( e_PasswordError ) {
			//パスワードが不一致時.
			case PNLE_PasswordErrorType_CompareError:
				Txt_MessageAdminLogSystemEncryptionSettingRegist.changeText( s_MSGMessageAdminSecurityPasswordNotAgreeError );
				break;

			//パスワードが禁止時.
			case PNLE_PasswordErrorType_ProhibitError:
				Txt_MessageAdminLogSystemEncryptionSettingRegist.changeText( s_MSGMessageAdminSecurityPasswordNotUseError );
				break;

			//その他.
			default:
				Txt_MessageAdminLogSystemEncryptionSettingRegist.changeText( s_MSGDebugLogEncryptionSecondInput );
				break;

		}
	} else {
		//標準キーボードを隠す.
		Grp_KeybordTotalPartsO6.showEnable( False );
		//メッセージを隠す.
		Txt_MessageAdminLogSystemEncryptionSettingRegist.showEnable( False );
		PAdminKeyBoardCom::setupGamen( option );
	}
	//入力できないキーを非表示にする.
	Kbd_UtilityInputKeybordConv.setDisableKeyCharCode( KC_DisableKeyPattern6 );// パターン６("()>?[;:],\).
	//パスワード再入力のセットアップ画面処理をコール.
	PAdminKeyBoardCom::setupGamenPasswordReInput( option );
}

//------------------------------------------------------------------------------
/**
* @brief 80980920 デバッグ情報暗号化設定登録 : 画面終了処理.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingRegist::endGamen()
{
	//スライド文字列管理クラスの初期化.
	PLBC_WideKeyboard::getInstance()->initInputArea( &Kbd_UtilityInputKeybordConv, &c_SlideTextMgr, sc_StrKeyboard, ( char )Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MaxInputNum, &Btx_PasswordReInput, getCharSetForPanel() ); //(キーボード、スライド文字列管理、キーボード文字列、キーボードの(標準/拡大)のMAX領域、最大入力文字数入力表示エリア、入力表示エリア、文字コード).
	//ワイドキーボード終了処理.
	PLBC_WideKeyboard::getInstance()->endWideKeyboard();
	//暗号化ワードキーのテキストをデフォルトにを変更.
	Key_PasswordReInput.changeText( s_KEYPassword );
	//暗号化ワード確認入力キーのテキストをデフォルトに変更.
	Key_PasswordReInputCheck.changeText( s_KEYPasswordConfirm );
	//パスワード再入力の画面終了処理をコール.
	PAdminKeyBoardCom::endGamenPasswordReInput();
}

//------------------------------------------------------------------------------
/**
* @brief 80980920 デバッグ情報暗号化設定登録 : ログ削除完了時の処理.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingRegist::notifyLogSystemDeleteLogFileResult( DBGE_DebugLogSystemResult e_DebugLogSystemResult )
{
	//ログ削除状態が実行中かチェック.
	if( PNLE_SystemLogDeleteStatus_Exec == m_e_SystemLogDeleteStatus ) {
		//結果コードが正常終了であるかチェック.
		if( DBGE_DebugLogSystemResultOK == e_DebugLogSystemResult ) {
			//ログ削除状態を正常終了に設定.
			m_e_SystemLogDeleteStatus = PNLE_SystemLogDeleteStatus_OK;
		} else {
			//ログ削除状態を異常終了に設定.
			m_e_SystemLogDeleteStatus = PNLE_SystemLogDeleteStatus_NG;
			setBuzzerType( PCGE_BuzzerType_InputNg );
		}
	}
}

//------------------------------------------------------------------------------
/**
* @brief 80980920 デバッグ情報暗号化設定登録 : ログ削除要求処理.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingRegist::reqSystemLogDelete()
{
	//ログ削除状態を実行中に設定.
	m_e_SystemLogDeleteStatus = PNLE_SystemLogDeleteStatus_Exec;
	APIC_DebugLogSystem* pc_DebugLogSystem = APIC_DebugLogSystem::newInstance();
	// ログ削除要求.
	if( pc_DebugLogSystem ) {
		DBGE_FuncResult e_FuncResult = pc_DebugLogSystem->req_DeleteLogFile();
		if( DBGE_FuncResultErr == e_FuncResult ) {
			//ログ削除状態を異常終了に設定.
			m_e_SystemLogDeleteStatus = PNLE_SystemLogDeleteStatus_NG;
		} else if( DBGE_FuncResultNotEnable == e_FuncResult ) {
			//ログファイルない時は、ログ削除状態を正常終了に設定.
			m_e_SystemLogDeleteStatus = PNLE_SystemLogDeleteStatus_OK;
		}
	} else {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		//ログ削除状態を異常終了に設定.
		m_e_SystemLogDeleteStatus = PNLE_SystemLogDeleteStatus_NG;
	}
	if( PNLE_SystemLogDeleteStatus_NG == m_e_SystemLogDeleteStatus ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}

//------------------------------------------------------------------------------
/**
* @brief 80980920 デバッグ情報暗号化設定登録 : ログ削除実行中かどうか.
*/
//------------------------------------------------------------------------------
Bool PAdminLogSystemEncryptionSettingRegist::isRunning()
{
	Bool b_Result = False;
	if( ( PNLE_SystemLogDeleteStatus_Exec == m_e_SystemLogDeleteStatus ) || ( PNLE_SystemLogDeleteStatus_OK == m_e_SystemLogDeleteStatus ) ) {
		b_Result = True;
	}
	return b_Result;
}

//------------------------------------------------------------------------------
/**
* @brief 80980910 デバッグ情報暗号化設定変更 : クリアー押下処理.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingEdit::procClear( uchar option )
{
	PAdminKeyBoardCom::procClear( option );
	//スライド文字列管理クラスの初期化（ワイドキーボード用）.
	PLBC_WideKeyboard::getInstance()->initInputArea_Clear();
	setupGamen();
}

//------------------------------------------------------------------------------
/**
* @brief 80980910 デバッグ情報暗号化設定変更 : 戻るキー押下処理.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingEdit::procCancel( uchar option )
{
	//HDD管理設定画面に遷移.
	nextGamen = &gp_AdminSecurityHDDManageSet;
}

//------------------------------------------------------------------------------
/**
* @brief 80980910 デバッグ情報暗号化設定変更 : OKキー押下処理.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingEdit::procOk( uchar option )
{
	char sc_nowPassword[ PNLD_MaxHDDLogEncryptionPassword + 1 ];//現在設定されているパスワード.
	memset( sc_nowPassword, 0x00, sizeof( sc_nowPassword ) );
	//現在設定されているパスワードを取得.
	APIC_DebugLogSystemSetting* pc_DebugLogSystemSetting = APIC_DebugLogSystemSetting::newInstance();
// Comment 2015/11/02 【ZeusS_ZX0】【HCL_Vasu】【制御構成変更_No.76-1】Adding Error Code macro.
//	if( NULL == pc_DebugLogSystemSetting ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	if( NULL == pc_DebugLogSystemSetting ) { 
		LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
		if (NULL !=pc_BFW_System){
			pc_BFW_System->registDetailCode( PNLD_DetailCode_Admin_0x1900030010B24D01 );
		}
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return; 
	}
	pc_DebugLogSystemSetting->getHDDLogEncryptionPassword( sc_nowPassword );

	char sc_InputPassword[ PNLD_MaxHDDLogEncryptionPassword + 1 ];//キーボードの文字列.
	//キーボードの文字列を取得.
	getKeyboardData( sc_InputPassword );

	//現在のパスワードと一致しているかチェック.
	char sc_CompareResult = ( char )strcmp( sc_nowPassword, sc_InputPassword );
	if( sc_CompareResult == PNLD_AdminCompareResultSame ) {
		//デバッグ情報暗号化設定登録に遷移.
		nextGamen = &gp_AdminLogSystemEncryptionSettingRegist;
	} else {
		//パスワード不一致メッセージを表示させるフラグを立てる.
		m_b_ErrorFlg = True;
		PAdminKeyBoardCom::procClear( option );
		//報知音設定.
		setBuzzerType( PCGE_BuzzerType_InputNg );
		//現在のキーボード状態(標準/拡大)を取得.
		if( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
			//拡大であれば、エラーメッセージを表示する為に標準へ変更.
			PAdminKeyBoardCom::procButtonOn( PNLD_ChangeEnlKey );
		} else {
			setupGamen();
		}
	}
}

//------------------------------------------------------------------------------
/**
* @brief 80980910 デバッグ情報暗号化設定変更 : 1秒タイマ(Sleepモードに移行させない為、1秒毎にキー操作通知を行う).
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingEdit::procSecond( uchar option )
{
	//その他キー操作を通知.
	ppc_Power->changePowerMode( TYPE_PowerModeEtsKeyOperation );
	PAdminKeyBoardCom::procSecond( option );
}

//------------------------------------------------------------------------------
/**
* @brief 80980910 デバッグ情報暗号化設定変更 : デフォルト背景表示部品の設定.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingEdit::setDefaultBackParts()
{
	//キーボード切換え状態が拡大かつキーボード初期状態判定フラグがTrueかチェック.
	if( ( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) && ( True == onlyPanel.b_InitKeybord ) ) {
		//拡大用デフォルト背景を設定.
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminLogSystemEncryptionSettingEditDefaultWideEnl, ss_AdminLogSystemEncryptionSettingEditDefaultWideEnl );
	} else {
		//標準用デフォルト背景を設定.
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminLogSystemEncryptionSettingEditDefaultWide, ss_AdminLogSystemEncryptionSettingEditDefaultWide );
	}
}

//------------------------------------------------------------------------------
/**
* @brief 80980910 デバッグ情報暗号化設定変更 : 画面開始処理.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingEdit::startGamen()
{
// comment 2016/03/28 OPS1-Sky岡田 [Icarus][キーボード改善].
	onlyPanel.setKeyboard1Line( False );

	//ワイドキーボード初期化処理(ワイドキーボードタイプ、画面のタイトルパーツを指定、特殊設定).
	PLBC_WideKeyboard::getInstance()->startWideKeyboard( PNLE_WideKeyboard_Normal, &Txt_AdminLogSystemEncryptionSettingTitle, PNLE_WideKeyboardExNone );

	//パスワード表示フラグをTrueに変更.
	b_Password = True;
	//入力表示エリアをセット.
	pc_ActivatableField = &Btx_InputDspAreaWide_2;
	//キーボードクラスへカーソルのアドレスを渡す.
	Kbd_UtilityInputKeybordConv.GetCursor( pc_ActivatableField );

	//キーボード初期状態をチェック.
	if( False == onlyPanel.b_InitKeybord ) {
		//キーボード表示.
		Kbd_UtilityInputKeybordConv = PLBC_ConvertKeybord( PLBD_KBT_ASCII_S, 192, 184 );
		//キーボード切換え状態を初期状態とする.
		Kbd_UtilityInputKeybordConv.setChangeEnlKeybord( PLBD_Enl_KBD_Initial );
	}

	//拡大表示かチェック.
	if( True == Kbd_UtilityInputKeybordConv.setDispFieldEnl( &Btx_EnlChangeKeybord ) ) {
		//拡大表示であれば入力表示エリアを変更.
		pc_ActivatableField = &Btx_EnlChangeKeybord;
	}

	//初期表示の時のみ初期化.
	if( PLBD_Enl_KBD_Initial == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
		char sc_Password[ PNLD_MaxHDDLogEncryptionPassword + 1 ];
		memset( sc_Password, 0, sizeof( sc_Password ) );
		//キーボードにデータをセット(キーボードの初期文字列、キーボードの最大入力文字数、キーボードの幅(char単位)).
		setKeyboardData( sc_Password, PNLD_MaxHDDLogEncryptionPassword, TYPD_AdminKeybordMax );
		//エラーメッセージ表示フラグの初期化.
		m_b_ErrorFlg = False;
	}

	PAdminKeyBoardCom::startGamen();
	//ユーティリティ専用キーボード共通開始処理.
	startGamenKeybordUtility9E();
	//キーボード初期状態判定フラグをFalseとする.
	onlyPanel.b_InitKeybord = False;
	//スライド文字列管理クラスの初期化.
	PLBC_WideKeyboard::getInstance()->initInputArea( &Kbd_UtilityInputKeybordConv, &c_SlideTextMgr, sc_StrKeyboard, ( char )Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MaxInputNum, &Btx_InputDspAreaWide_2, getCharSetForPanel() ); //(キーボード、スライド文字列管理、キーボード文字列、キーボードの(標準/拡大)のMAX領域、最大入力文字数入力表示エリア、入力表示エリア、文字コード).
}

//------------------------------------------------------------------------------
/**
* @brief 80980910 デバッグ情報暗号化設定変更 : 画面描画.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingEdit::setupGamen( short option )
{
	//キーボード拡大状態かチェック.
	if( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
		//標準キーボードを隠す.
		Grp_KeybordTotalPartsO6.showEnable( False );
		//標準入力表示領域を隠す.
		Btx_InputDspAreaWide_2.showEnable( False );
		//エラーメッセージを隠す.
		Txt_AdminLogSystemEncryptionSettingEditErrorMessage.showEnable( False );
		//メニューメッセージを隠す.
		Txt_AdminLogSystemEncryptionSettingEditMenu.showEnable( False );
	}

	PAdminKeyBoardCom::setupGamen( option );

	//キーボード拡大状態ではないかチェック.
	if( PLBD_Enl_KBD_Enlarge != Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
		//メッセージを切替える.
		if( m_b_ErrorFlg == False ) {
			//通常メッセージ表示.
			Txt_AdminLogSystemEncryptionSettingEditErrorMessage.showEnable( False );
			Txt_AdminLogSystemEncryptionSettingEditMenu.showEnable( True );
		} else {
			//エラーメッセージ表示.
			Txt_AdminLogSystemEncryptionSettingEditMenu.showEnable( False );
			Txt_AdminLogSystemEncryptionSettingEditErrorMessage.showEnable( True );
		}
	}

	//入力できないキーを非表示にする.
	Kbd_UtilityInputKeybordConv.setDisableKeyCharCode( KC_DisableKeyPattern6 );// パターン６("()>?[;:],\).
}

//------------------------------------------------------------------------------
/**
* @brief 80980910 デバッグ情報暗号化設定変更 : 画面終了処理.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingEdit::endGamen()
{
	//ワイドキーボード終了処理.
	PLBC_WideKeyboard::getInstance()->endWideKeyboard();
	PAdminKeyBoardCom::endGamen();
}

//------------------------------------------------------------------------------
/**
* @brief 80980900 デバッグ情報暗号化設定確認 : キー押下処理.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingCheck::procButtonOn( uchar option )
{
	//押下キー(「はい」(0)/「いいえ」(1))をチェック.
	switch( option ) {
		case PNLE_EncryptionSettingSelectYesButton://はい」キー選択時.
			//キーの選択状態を「はい」に変更.
			m_s_Select = PNLE_EncryptionSettingSelectYesButton;
			break;

		case PNLE_EncryptionSettingSelectNoButton://「いいえ」キー選択時.
			//キーの選択状態を「いいえ」に変更.
			m_s_Select = PNLE_EncryptionSettingSelectNoButton;
			break;

		default:
			break;
	}
	setupGamen( option );
}

//------------------------------------------------------------------------------
/**
* @brief 80980900 デバッグ情報暗号化設定確認 : 画面移動.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingCheck::procOk( uchar option )
{
	//選択キー(「はい」(0)/「いいえ」(1))をチェック.
	if( m_s_Select == PNLE_EncryptionSettingSelectYesButton ) {
		//「はい」キー選択時、デバッグ情報暗号化設定変更に遷移.
		nextGamen = &gp_AdminLogSystemEncryptionSettingEdit;
	} else {
		//「いいえ」キー選択時、HDD管理設定画面に遷移.
		nextGamen = &gp_AdminSecurityHDDManageSet;
	}
}

//------------------------------------------------------------------------------
/**
* @brief 80980900 デバッグ情報暗号化設定確認 : 画面開始処理.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingCheck::startGamen()
{
	//キー選択状態を「いいえ」(1)に設定.
	m_s_Select = PNLE_EncryptionSettingSelectNoButton;
	PAdminToSecurityHDDManageSetcom::startGamen();
	//左エリア種類の有効設定（しおり表示）startGamen()の最後でコール.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark );
}

//------------------------------------------------------------------------------
/**
* @brief 80980900 デバッグ情報暗号化設定確認 : 画面描画.
*/
//------------------------------------------------------------------------------
void PAdminLogSystemEncryptionSettingCheck::setupGamen( short option )
{
	//キー(「はい」(0)/「いいえ」(1))の選択表示.
	Rbt_AdminLogSystemEncryptionSettingCheckKey.select( m_s_Select );
	PAdminToSecurityHDDManageSetcom::setupGamen( option );
}

/**
* @brief  : 管理者設定・セキュリティ設定・HDD管理設定・消去終了共通 : 画面開始処理.
* @note   : None.
* @param  : None.
* @return : なし.
*/
void ans_startListPrint( ushort us_Result );
/**
* @brief  : 管理者設定・セキュリティ設定・HDD管理設定・消去終了共通 : 画面開始処理.
* @note   : None.
* @param  : None.
* @return : なし.
*/
void PAdminSecurityHDDManageDataDeleteEndActionWarningDetection::startGamen()
{
	PAdminSecurityHDDManageDataDeleteEndAction::startGamen();
	// 主電源OFF/ON画面から遷移させないためのフラグ.
	onlyPanel.bl_PowerOffOnMesage = False;
	// リスト印字要求.
// Comment 2017/10/25【HCLT-Narsimhmaswamy】【IT5 4.2】【RQ686】【WP1215】Multiple Network support.
//	APIC_ListPrint::newInstance()->req_startListPrint( TYPE_ListPrintCompletionHDDFormat, TYPE_FeedTrayManual, ans_startListPrint, -1, 0, NULL, 0, TYPE_JobEntryIF_Panel );
// comment 2019/1/30 Sky鷲見 [OFP041].
	// IU/Drum/DevUnit ライフストップ設定.
	PnlComGetPrinterServiceSetting()->setPrinterAdjust( TYPE_PrinterAdjustIULifeStop, False );	// 値を確定させる.

// Comment 2018/09/10 【IT6_Eagle】【HCL_Satheesh】 AR 1047260 Overwrite operation Design changes.
	if ( gp_AdminSecurityHDDManageAllDataWriteDeleteAction.uc_KeySelect == BTNID_KEY_OVERWRITE_DELETE_AND_PRINT ) {
		APIC_ListPrint::newInstance()->req_startListPrint( TYPE_ListPrintCompletionHDDFormat, TYPE_FeedTrayManual, ans_startListPrint, -1, 0, NULL, 0, TYPE_JobEntryIF_Panel, 0, PNLC_NetworkLan::getInstance() ->getAdminNWLine( ) );
	}
	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );
}
/**
* @brief  : Warning Check function.
* @note   : None.
* @param  : None.
* @return : なし.
*/
void PAdminSecurityHDDManageDataDeleteEndActionWarningDetection::procWarningGamenExist( uchar option )
{
	gp_AdminSecurityHDDManageAllDataWriteDeleteAction.setOverwriteAndPrintOption( False );
	onlyPanel.setPanelWarning( PNL_ERROR_HDDReportPrint );
	// 上記以外の場合、状態遷移を行う.
	PMessageProcCom::procWarningGamenExist( option );
}


// ===============================================================================================.
//	［クラス］	PAdminUbqJoinGroupExecuting : L097600970 ユビキタスグループ設定データ処理中.
//	概要	：	画面開始処理.
//	戻り値	：	void.
//	備考	：	.
// ===============================================================================================.
void PAdminUbqJoinGroupExecuting::startGamen()
{
	// ローカル変数の初期化.
	char sc_GroupNameUTF8[TYPD_UbiquitousGroupNameLength + 1];
	memset( sc_GroupNameUTF8, '\0', sizeof(sc_GroupNameUTF8) );
	char sc_ConnectAddressUTF8[TYPD_RegisterNameMax + 1];
	memset( sc_ConnectAddressUTF8, '\0', sizeof(sc_ConnectAddressUTF8) );

	// 親クラスの画面開始処理をコール.
	PToAdminSetMenucom::startGamen();

	// 本画面でオートリセットがかからないようタイマーを止める.
	if( True == PnlComGetOperationSetting()->getAutoResetSystem().b_IsOn ) {
		c_autoRestTimer.c_AutoResetAll.stopTimer();				// システムオートリセットタイマー.
	} else {
		c_autoRestTimer.c_autoResetOffTimer.stopTimer();		// システムオートリセットOFF時の1分タイマー.
	}

	// NVDからグループ名と接続先アドレスを取得.
	APIC_UbiquitousSetting* pc_UbiquitousSetting = PnlComGetUbiquitousSetting();
	pc_UbiquitousSetting->getBelongingGroupName( sc_GroupNameUTF8 );
	pc_UbiquitousSetting->getTopologyManager( sc_ConnectAddressUTF8 );

	// ユビキタスグループ接続を要求.
// comment 2016/07/20 ユビキタス [RQ-553][1002238]【IT評価】PSWCの参加グループ設定失敗時、Attention警告が発生する.
//	Bool bl_result = PnlComGetTopologyManager()->req_AssociationRequest( sc_GroupNameUTF8, sc_ConnectAddressUTF8 );
	Bool bl_result = PnlComGetTopologyManager()->req_AssociationRequest( sc_GroupNameUTF8, sc_ConnectAddressUTF8, TYPE_TopologyAssociationRequestTypeDetail_Panel );
	if (False == bl_result) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); 
		return;
	}

	turnGamen = NULL;
	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key );
}


// ===============================================================================================.
//	［クラス］	PAdminUbqJoinGroupExecuting : L097600970 ユビキタスグループ設定データ処理中.
//	概要	：	画面表示処理.
//	戻り値	：	void.
//	備考	：	.
// ===============================================================================================.
void PAdminUbqJoinGroupExecuting::setupGamen( short option )
{
	// 親クラスの画面セットアップをコール.
	PToAdminSetMenucom::setupGamen( option );

	Prg_AdminUbqJoinGroupExecutingIcon.animateParts();		// プログレスバーのアニメーション.
}


// ===============================================================================================.
//	［クラス］	PAdminUbqJoinGroupExecuting : L097600970 ユビキタスグループ設定データ処理中.
//	概要	：	画面終了処理.
//	戻り値	：	void.
//	備考	：	.
// ===============================================================================================.
void PAdminUbqJoinGroupExecuting::endGamen()
{
	PToAdminSetMenucom::endGamen();									// 親クラスの画面終了処理.

	// タイマーを開始する.
	if( True == PnlComGetOperationSetting()->getAutoResetSystem().b_IsOn ) {
		c_autoRestTimer.c_AutoResetAll.resetTimer( True );			// システムオートリセットタイマー.
	} else {
		c_autoRestTimer.c_autoResetOffTimer.resetTimer( True );		// システムオートリセットOFF時の1分タイマー.
	}
}


// ===============================================================================================.
//	［クラス］	PAdminUbqJoinGroupExecuting : L097600970 ユビキタスグループ設定データ処理中.
//	概要	：	オブザーバー通知処理.
//	戻り値	：	void.
//	備考	：	.
// ===============================================================================================.
void PAdminUbqJoinGroupExecuting::procObserver( TYPE_PNL_Observer e_PNL_Observer )
{
// comment 2022/08/18 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
	ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
//	if (TYPE_PNL_Observer_AssociationRequest == e_PNL_Observer) {	// ユビキタスグループ接続通知の場合.
//		// 接続結果をオブザーバクラスから取得.
//		TYPE_TopologyAssociationResult e_AssociationResult = c_TopologyAppObserver.getAssociationResult();
//
//		// 接続結果毎に処理を分岐.
//		switch( e_AssociationResult ) {
//			case TYPE_TopologyAssociationResult_Success:
//			case TYPE_TopologyAssociationResult_Registered:
//				nextGamen = &gp_AdminUbqJoinGroupMenu;					// 「L097600900_参加グループ設定」画面へ遷移.
//				break;
//			case TYPE_TopologyAssociationResult_NoMactchGroupName:
//			case TYPE_TopologyAssociationResult_Full:
//			case TYPE_TopologyAssociationResult_TimeOut:
//			case TYPE_TopologyAssociationResult_Failed:
//				gp_AdminUbqJoinGroupSetError.setAssociationResult( e_AssociationResult );
//				nextGamen = &gp_AdminUbqJoinGroupSetError;				// 「L097600950_接続先通信エラー」画面へ遷移.
//				break;
//			default:
//				break;
//		}
//	} else {														// それ以外の通知の場合.
//		PToAdminSetMenucom::procObserver( e_PNL_Observer );				// 親クラスの処理をコール.
//	}
}


// comment 2016/14/03 HCLT-Krishna [ZeusSBK][CQ525643] Commenting the structure.
////comment 2015/03/09 HCLT_Archana [Helios][KRQ-101] Authentication Function Enabled list I291000400.
//// Tables for choosing Symbol Text to be displayed.
//typedef struct _PNLS_AdminAuthFunctionEnabledFunctionListTbl {
//	uchar uc_LicenseVersion;		// License version.
//	TEXT_ID text_id_admin; 			// Text of the license.
//}PNLS_AdminAuthFunctionEnabledFunctionListTbl;
// comment 2016/14/03 HCLT-Krishna [ZeusSBK][CQ525643] Adding Maximum size for Application name and it's version.
#define PNLD_ApplicationName_License_Max	(55 * PCGD_BuffSize_UTF8 + PCGD_EndCode)				// Maximum length which can be displayed in text field.
// comment 2016/14/03 HCLT-Krishna [ZeusSBK][CQ525643] Macros defined for the Function code.
typedef uchar PNLE_APPLICATION_ID;				// Application ID.
	#define PNLE_APPLICATION_AIRPRINT	(1)		// Airprint.
	#define PNLE_APPLICATION_MOPRIA		(2)		// Mopria.
	#define PNLE_APPLICATION_WIFIDIRECT	(3)		// Wi-Fi Direct.
// comment 2016/14/03 HCLT-Krishna [ZeusSBK][CQ525643] Table for choosing symbol text to be displayed.
struct PNLS_AdminAuthFunctionEnabledFunctionListTbl {
	PNLE_APPLICATION_ID	 e_ApplicationId;															// Application ID.
	Bool 				 bl_Installed;																// Flag to store application is installed or not.
	TEXT_ID				 text_id_admin;																// Text of the license.
	char				 sc_ApplicationNameAndLicenseVer[PNLD_ApplicationName_License_Max];			// String to hold Application Name and License Version.
};	
// comment 2019/05/20 HCLT-Shalini [EagleH][OP_BTS-5062] Message ID Modification for display of Mopria.
// comment 2019/02/26 HCLT-Shalini [EagleH][制御構成変更_No.64_Mopria1.3対応] Changing of MessageID s_ITMMopriaTitle to s_pNOTITMMopria_Print_W112H016I .
//// comment 2016/03/14 HCLT_Krishna [ZeusSBK][CQ525643] Creation of table.	
PNLS_AdminAuthFunctionEnabledFunctionListTbl st_AdminauthFunctionEnabledfunctionCodeListTbl[PNLD_AUTHENTICATIONFUNCTIONENABLEDLIST] ={
	{ PNLE_APPLICATION_AIRPRINT, False , s_ITMAirPrintTitle , "" },
//	{ PNLE_APPLICATION_MOPRIA, False , s_ITMMopriaTitle , "" },
//	{ PNLE_APPLICATION_MOPRIA, False , s_pNOTITMMopria_Print_W112H016I , "" },
	{ PNLE_APPLICATION_MOPRIA, False , s_pITMMopriaPrint_W112H016I , "" },
	{ PNLE_APPLICATION_WIFIDIRECT, False , s_ITMAuthFunctionWifiDirect , "" },
};	
// comment 2016/01/27 HCLT_Krishna [ZeusSBK][制御構成変更_No.11] Remove the Airprint , Mopria and WiFi definition.
//PNLS_AdminAuthFunctionEnabledFunctionListTbl	st_AdminauthFunctionEnabledAirPrintfunctionCodeListTbl[] = {	//	Table for Airprint license.
//	{TYPE_AirPrintLicence_Ver1_4, s_NOTITMAuthFunctionAirPrinit14},							//	Airprint 1.4.
//	{TYPE_AirPrintLicence_Ver1_5, s_NOTITMAuthFunctionAirPrint15},							//	Airprint 1.5.
//};
	
//PNLS_AdminAuthFunctionEnabledFunctionListTbl	st_AdminauthFunctionEnabledMopriafunctionCodeListTbl[] = {		// Table for Mopria license.
//	{TYPE_MopriaLicence_Ver1_2,  s_NOTITMAuthFunctionMopria12},
//// Comment 2015/09/22 HCLT-Deeparaj [MinervaSBK] [制御構成変更_No.71] Adding new entry for Mopria1.3 license.
//	{TYPE_MopriaLicence_Ver1_3,  s_NOTITMAuthFunctionMopria13},								//	Mopria 1.3.
//};

//PNLS_AdminAuthFunctionEnabledFunctionListTbl	st_AdminauthFunctionEnabledWfdfunctionCodeListTbl[] = {			//	Table for wifi direct license.
//	{TYPE_WfdLicence_Enable, s_ITMAuthFunctionWifiDirect},									//	WiFi Direct.
//};
//====================================================================================================================.
/**
* @brief  : To Handle Screen Start processing.
* @note   : PAdminAuthenticationFunctionEnableList : I291000400_認証機能有効化設定／有効機能一覧.
* @param  : None.
* @return : None.
* @author :	2015/03/09 HCLT_Archana [HeliosL][KRQ-101].
*/
//====================================================================================================================.
void PAdminAuthenticationFunctionEnableList::startGamen()
{
	PToAdminSetMenucom::startGamen();														//	Base Class Call.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key);					//	Left Area.
	// comment 2016/03/14 HCLT-Krishna [ZeusSBK][CQ525643] Getting the values from API.
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	if ( NULL == pc_SystemInfo ) {
		LAPC_BFW_System *pc_BFW_System = LAPC_BFW_System::newInstance();
		if (NULL !=pc_BFW_System) {
			pc_BFW_System ->registDetailCode( PNLD_DetailCode_Admin_0x1900660010D98501 );
		}
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	ushort k = 0;
	for ( k = 0; k < PNLD_AUTHENTICATIONFUNCTIONENABLEDLIST; k++ )	{
		switch ( st_AdminauthFunctionEnabledfunctionCodeListTbl[k].e_ApplicationId )	{
			case PNLE_APPLICATION_AIRPRINT:
				{	
					char sc_AirprintLicenceVersion[TYPE_AirPrintLicenceVer_Size];														//Buffer to store Airprint License.
					memset( sc_AirprintLicenceVersion, 0 , sizeof( sc_AirprintLicenceVersion ) );					
					TYPE_AirPrintLicence e_AirPrintLicence = pc_SystemInfo->getAirPrintLicenceVersion( sc_AirprintLicenceVersion );		// To get the Airprint enable status.
					
					if ( TYPE_AirPrintLicence_None != e_AirPrintLicence ) {
						char sc_ApplicationName_Airprint[PNLD_ApplicationName_License_Max];
						memset( sc_ApplicationName_Airprint , 0 , sizeof( sc_ApplicationName_Airprint) );
						memset( st_AdminauthFunctionEnabledfunctionCodeListTbl[k].sc_ApplicationNameAndLicenseVer ,
							0 , sizeof( st_AdminauthFunctionEnabledfunctionCodeListTbl[k].sc_ApplicationNameAndLicenseVer ) );
						
						st_AdminauthFunctionEnabledfunctionCodeListTbl[k].bl_Installed = True;
						LIBF_ConvertToUTF8( (LIBE_CharSet)getCharSetForPanel(),					
							(char*)getTextPartsPointer( st_AdminauthFunctionEnabledfunctionCodeListTbl[k].text_id_admin ),		
							 sc_ApplicationName_Airprint, sizeof( sc_ApplicationName_Airprint ));							// Convert Application Name to UTF8 format.
						snprintf( st_AdminauthFunctionEnabledfunctionCodeListTbl[k].sc_ApplicationNameAndLicenseVer ,		
							sizeof( st_AdminauthFunctionEnabledfunctionCodeListTbl[k].sc_ApplicationNameAndLicenseVer ) ,
							"%s %s" , sc_ApplicationName_Airprint , sc_AirprintLicenceVersion );							// Appending Application Name and license.
					} else {
						st_AdminauthFunctionEnabledfunctionCodeListTbl[k].bl_Installed = False;
					}
				}	
				break;
			case PNLE_APPLICATION_MOPRIA:
				{			
					char sc_MopriaLicenceVersion[TYPE_MopriaLicenceVer_Size];													//Buffer to store Mopria License.
					memset( sc_MopriaLicenceVersion, 0 , sizeof( sc_MopriaLicenceVersion ) );					
// comment 2019/02/26 HCLT-Shalini [EagleH][制御構成変更_No.64_Mopria1.3対応] Addition of API info. for Mopria Scan Version .
					char sc_MopriaLicenceVersion_Scan[TYPE_MopriaLicenceVer_Size];													//Buffer to store Mopria License.
					memset( sc_MopriaLicenceVersion_Scan, 0 , sizeof( sc_MopriaLicenceVersion_Scan ) );					
					TYPE_MopriaLicence e_MopriaLicence =  pc_SystemInfo->getMopriaLicenceVersion( sc_MopriaLicenceVersion);	// To get the Mopria enable status.
					TYPE_MopriaScanLicence e_MopriaScanLicence =  pc_SystemInfo-> getMopriaScanLicenceVersion( sc_MopriaLicenceVersion_Scan );	// To get the Mopria enable status.
					if ( TYPE_MopriaLicence_None != e_MopriaLicence && TYPE_MopriaScanLicence_None != e_MopriaScanLicence) {
						char sc_ApplicationName_Mopria[PNLD_ApplicationName_License_Max];
						memset( sc_ApplicationName_Mopria , 0 , sizeof( sc_ApplicationName_Mopria ) );
						char sc_ApplicationName_MopriaScan[PNLD_ApplicationName_License_Max];
						memset( sc_ApplicationName_MopriaScan , 0 , sizeof( sc_ApplicationName_MopriaScan ) );
						char sc_ApplicationName_MopriaClose[PNLD_ApplicationName_License_Max];
						memset( sc_ApplicationName_MopriaClose , 0 , sizeof( sc_ApplicationName_MopriaClose ) );
						memset( st_AdminauthFunctionEnabledfunctionCodeListTbl[k].sc_ApplicationNameAndLicenseVer ,
							0 , sizeof( st_AdminauthFunctionEnabledfunctionCodeListTbl[k].sc_ApplicationNameAndLicenseVer ) );
						
						st_AdminauthFunctionEnabledfunctionCodeListTbl[k].bl_Installed = True;
// comment 2019/05/23 HCLT-Shalini [EagleH][OP_BTS-5062] Changing the getCharSetForPanel() to getCharSetForCAPI() .
//						LIBF_ConvertToUTF8( (LIBE_CharSet)getCharSetForPanel(),
						LIBF_ConvertToUTF8( (LIBE_CharSet)getCharSetForCAPI(),
							(char*)getTextPartsPointer( st_AdminauthFunctionEnabledfunctionCodeListTbl[k].text_id_admin ),
							 sc_ApplicationName_Mopria, sizeof( sc_ApplicationName_Mopria ));									// Convert Application Name to UTF8 format.
// comment 2019/02/26 HCLT-Shalini [EagleH][制御構成変更_No.64_Mopria1.3対応] Addition of API info. for Mopria Scan Version .
//						LIBF_ConvertToUTF8( (LIBE_CharSet)getCharSetForPanel(),
//							(char*)getTextPartsPointer( s_pNOTITMDotScan_W056H016I ),
						LIBF_ConvertToUTF8( (LIBE_CharSet)getCharSetForCAPI(),
							(char*)getTextPartsPointer( s_pITMDotScan_W056H016I ),
							 sc_ApplicationName_MopriaScan, sizeof( sc_ApplicationName_MopriaScan  ));									// Convert Application Name to UTF8 format.
//						LIBF_ConvertToUTF8( (LIBE_CharSet)getCharSetForPanel(),
						LIBF_ConvertToUTF8( (LIBE_CharSet)getCharSetForCAPI(),
							(char*)getTextPartsPointer( s_pNOTITMRightbracket_W016H016I ),
							 sc_ApplicationName_MopriaClose, sizeof( sc_ApplicationName_MopriaClose ));									// Convert Application Name to UTF8 format.
//						snprintf( st_AdminauthFunctionEnabledfunctionCodeListTbl[k].sc_ApplicationNameAndLicenseVer ,
//							sizeof( st_AdminauthFunctionEnabledfunctionCodeListTbl[k].sc_ApplicationNameAndLicenseVer ) ,		
//							"%s %s" , sc_ApplicationName_Mopria , sc_MopriaLicenceVersion );									// Appending Application Name and license.
// comment 2019/03/14 HCLT-Shalini [EagleH][OP_BTS-4093] Changing the Display version of Mopria.
						if ( strncmp( sc_MopriaLicenceVersion, TYPE_MopriaLicenceVer2_0_1_Str, TYPE_MopriaLicenceVer_Size ) == 0 ) {
							snprintf( ( char* )sc_MopriaLicenceVersion, sizeof( sc_MopriaLicenceVersion ),TYPE_MopriaLicenceVer2_0_Str);
						}						
						snprintf( st_AdminauthFunctionEnabledfunctionCodeListTbl[k].sc_ApplicationNameAndLicenseVer ,
							sizeof( st_AdminauthFunctionEnabledfunctionCodeListTbl[k].sc_ApplicationNameAndLicenseVer ) ,		
							"%s%s %s%s%s" , sc_ApplicationName_Mopria , sc_MopriaLicenceVersion , sc_ApplicationName_MopriaScan, sc_MopriaLicenceVersion_Scan ,sc_ApplicationName_MopriaClose );									// Appending Application Name and license.
					} else {
						st_AdminauthFunctionEnabledfunctionCodeListTbl[k].bl_Installed = False;
					}
				}	
				break;
			case PNLE_APPLICATION_WIFIDIRECT:
				{					
					TYPE_WfdLicence e_WfdLicence = PnlComGetSystemInfo()->getWfdLicenceStatus();							// To get the Wi-fi Direct enable status.	
					
					if ( TYPE_WfdLicence_Disable != e_WfdLicence )	{
						char sc_ApplicationName_wifi[PNLD_ApplicationName_License_Max];
						memset( sc_ApplicationName_wifi , 0 , sizeof( sc_ApplicationName_wifi) );
						memset( st_AdminauthFunctionEnabledfunctionCodeListTbl[k].sc_ApplicationNameAndLicenseVer ,
							0 , sizeof( st_AdminauthFunctionEnabledfunctionCodeListTbl[k].sc_ApplicationNameAndLicenseVer ) );
						
						st_AdminauthFunctionEnabledfunctionCodeListTbl[k].bl_Installed = True;
						LIBF_ConvertToUTF8( (LIBE_CharSet)getCharSetForPanel(),
							(char*)getTextPartsPointer( st_AdminauthFunctionEnabledfunctionCodeListTbl[k].text_id_admin ),
							 sc_ApplicationName_wifi, sizeof( sc_ApplicationName_wifi ));									   // Convert Application Name to UTF8 format.						
						strncpy( st_AdminauthFunctionEnabledfunctionCodeListTbl[k].sc_ApplicationNameAndLicenseVer , sc_ApplicationName_wifi,
							sizeof( st_AdminauthFunctionEnabledfunctionCodeListTbl[k].sc_ApplicationNameAndLicenseVer ) -1 );	// Appending Application Name.e
					} else {
						st_AdminauthFunctionEnabledfunctionCodeListTbl[k].bl_Installed = False;
					}
				}	
				break;
			default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, ( char* )"Assert Failed!" );	break;
		}
	}		
}
// comment 2016/03/14 HCLT-Krishna [ZeusSBK][CQ525643] Commenting the setupGamen().
////====================================================================================================================.
///**
//* @brief  : To Handle Screen Display processing.
//* @note   : PAdminAuthenticationFunctionEnableList : I291000400_認証機能有効化設定／有効機能一覧.
//* @param  : short.
//* @return : None.
//* @author :	2015/03/09 HCLT_Archana [HeliosL][KRQ-101].
//*/
////====================================================================================================================.
//void PAdminAuthenticationFunctionEnableList::setupGamen( short option )
//{
//	PToAdminSetMenucom::setupGamen( option );						//	Base Class Call.
	// comment 2016/01/27 HCLT_Krishna[ZeusSBK][制御構成変更_No.11] Declare the variable for Airprint, Mopria  and WiFi Licence.
	//TYPE_AirPrintLicence 	e_AirPrintLicence 		= TYPE_AirPrintLicence_None;	// Airprint License.
	//TYPE_MopriaLicence 		e_MopriaLicence 		= TYPE_MopriaLicence_None;		// Mopria License.
	//TYPE_WfdLicence 		e_WfdLicence			= TYPE_WfdLicence_Disable;		// Wi-Fi Direct License.
	//char sc_AirprintLicenceVersion[TYPE_AirPrintLicenceVer_Size];																//Airprint License.
	//char sc_MopriaLicenceVersion[TYPE_MopriaLicenceVer_Size];																	//Mopria License.
	//APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	//if (NULL == pc_SystemInfo) {
	//	LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
	//	if (NULL !=pc_BFW_System){
	//		pc_BFW_System ->registDetailCode(PNLD_DetailCode_Admin_0x1900660010D98501);
	//	}
	//	LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	//	return;
	//}

	//// Get the values from the APIs.
	//// comment 2016/01/27 HCLT_Krishna [ZeusSBK][制御構成変更_No.11] Getting the Airprint and Mopria License version using the API functions.
	//// TYPE_AirPrintLicence e_AirPrintLicence = PnlComGetSystemInfo()->getAirPrintLicenceStatus();			// To get the Airprint enable status.
	//// TYPE_MopriaLicence e_MopriaLicence = PnlComGetSystemInfo()->getMopriaLicenceStatus();				// To get the Mopria enable status.
	//memset (sc_AirprintLicenceVersion, '\0', sizeof(sc_AirprintLicenceVersion));
	//memset (sc_MopriaLicenceVersion, '\0', sizeof(sc_MopriaLicenceVersion));
    //e_AirPrintLicence = pc_SystemInfo->getAirPrintLicenceVersion(sc_AirprintLicenceVersion );			// To get the Airprint enable status.
    //e_MopriaLicence = pc_SystemInfo->getMopriaLicenceVersion(sc_MopriaLicenceVersion );					// To get the Mopria enable status.
	//e_WfdLicence = PnlComGetSystemInfo()->getWfdLicenceStatus();
// if ( TYPE_AirPrintLicence_None == e_AirPrintLicence) {
//		Txt_AdminAuthenticationFunctionEnabledAirPrint.showEnable(False);								//Hiding the Airprint text.
//      Num_AdminAuthenticationFunctionEnabledAirPrint.showEnable(False);								//Hiding the Airprint Version.
//	} else{
//	     Txt_AdminAuthenticationFunctionEnabledAirPrint.showEnable(True);								//Enabling the Airprint text.
//		 Num_AdminAuthenticationFunctionEnabledAirPrint.showEnable(True);								//Enabling the Airprint Version.
//		 Num_AdminAuthenticationFunctionEnabledAirPrint.changeTextString(sc_AirprintLicenceVersion);    //Displaying the Version number.
//    }
//	if ( TYPE_MopriaLicence_None == e_MopriaLicence) {
//		 Txt_AdminAuthenticationFunctionEnabledMopria.showEnable(False);								//Hiding the Mopria text.
//         Txt_AdminAuthenticationFunctionEnabledMopria.showEnable(False);								//Hiding the Mopria Version.
//	} else{
//	     Txt_AdminAuthenticationFunctionEnabledMopria.showEnable(True);									//Enabling the Mopria text.
//		 Num_AdminAuthenticationFunctionEnabledMopria.showEnable(True);									//Enabling the Mopria version.
//		 Num_AdminAuthenticationFunctionEnabledMopria.changeTextString(sc_MopriaLicenceVersion);		//Displaying the Version number.
//    }
//	if ( TYPE_WfdLicence_Disable == e_WfdLicence) {
//		Txt_AdminAuthenticationFunctionEnabledWiFi.showEnable(False);									//Hiding the WiFi text.
//	} else{
//		Txt_AdminAuthenticationFunctionEnabledWiFi.showEnable(True);									//Enabling the WiFi text.
//	}										// To get the Wifi direct enable status.


// comment 2015/06/01 HCLT-Chandrasekar [Helios][CQ510921] Changing short to ushort.
	//short k=0;
	//short i=0;
// comment 2016/01/27 HCLT_Krishna [ZeusSBK][制御構成変更_No.11] Commenting the usage of function code list table for Airprint License , Mopria License and Wi-Fi Direct License. 
//	ushort k = 0, i = 0;	
//	for (k=0; k<sizeof(st_AdminauthFunctionEnabledAirPrintfunctionCodeListTbl)/sizeof(st_AdminauthFunctionEnabledAirPrintfunctionCodeListTbl[0]);k++) {
//		if( e_AirPrintLicence == st_AdminauthFunctionEnabledAirPrintfunctionCodeListTbl[k].uc_LicenseVersion) {
//			Tfd_AdminAuthenticationFunctionEnabledFunctionCodeList[i].changeText(st_AdminauthFunctionEnabledAirPrintfunctionCodeListTbl[k].text_id_admin);
//			Tfd_AdminAuthenticationFunctionEnabledFunctionCodeList[i].showEnable(True);
//			i++;
//			break;
//		}
//	}
//// Mopria License.
//	for ( k = 0; k < sizeof(st_AdminauthFunctionEnabledMopriafunctionCodeListTbl)/sizeof(st_AdminauthFunctionEnabledMopriafunctionCodeListTbl[0]); k++) {
//		if( e_MopriaLicence == st_AdminauthFunctionEnabledMopriafunctionCodeListTbl[k].uc_LicenseVersion ) {
//			Tfd_AdminAuthenticationFunctionEnabledFunctionCodeList[i].changeText(st_AdminauthFunctionEnabledMopriafunctionCodeListTbl[k].text_id_admin);
//			Tfd_AdminAuthenticationFunctionEnabledFunctionCodeList[i].showEnable(True);
//			i++;
//			break;
//		}
//	}
//// Wi-Fi Direct License.
//	for ( k = 0; k < sizeof(st_AdminauthFunctionEnabledWfdfunctionCodeListTbl)/sizeof(st_AdminauthFunctionEnabledWfdfunctionCodeListTbl[0]); k++) {
//		if( e_WfdLicence == st_AdminauthFunctionEnabledWfdfunctionCodeListTbl[k].uc_LicenseVersion ) {
//			Tfd_AdminAuthenticationFunctionEnabledFunctionCodeList[i].changeText(st_AdminauthFunctionEnabledWfdfunctionCodeListTbl[k].text_id_admin);
//			Tfd_AdminAuthenticationFunctionEnabledFunctionCodeList[i].showEnable(True);
//			i++;
//			break;
//		}
//	}
//	
//	// Hide the Function List parts which are NOT enabled.
//	for ( k = (PNLD_AUTHENTICATIONFUNCTIONENABLEDLIST-1) ; k >= i; k--) {		
//		Tfd_AdminAuthenticationFunctionEnabledFunctionCodeList[k].showEnable(False);
////comment 2015/04/24 HCLT_Rengaraaj [HeliosL][CQ512570] Hiding Function name list row if relevant function name not enabled.
//		Btx_AdminAuthenticationFunctionEnabledFunctionCodeList[k].showEnable(False);
//// comment 2015/06/01 HCLT-Chandrasekar [Helios][CQ510921] Determine the minimum loop run.
//		if( 0 == k) {
//			break;
//		}
//	}
//}
//====================================================================================================================.
/**
* @brief  : To Handle Screen Display processing.
* @note   : PAdminAuthenticationFunctionEnableList : I291000400_認証機能有効化設定／有効機能一覧.
* @param  : short.
* @return : None.
* @author : 2016/03/14 HCLT_Krishna[ZeusSBK][CQ525643].
*/
//====================================================================================================================.
void PAdminAuthenticationFunctionEnableList::setupGamen( short option ) 
{
	PToAdminSetMenucom::setupGamen( option );						// Base Class Call.
	ushort us_ExistRow = 0;											// Number of Existing Licenses.
	short ss_Cnt = 0;
	for ( ss_Cnt = 0; ss_Cnt < PNLD_AUTHENTICATIONFUNCTIONENABLEDLIST; ss_Cnt++ ) {
		if ( True == st_AdminauthFunctionEnabledfunctionCodeListTbl[ss_Cnt].bl_Installed ) {
			Btx_AdminAuthenticationFunctionEnabledFunctionCodeList[us_ExistRow].changeExternalText( (uchar*)st_AdminauthFunctionEnabledfunctionCodeListTbl[ss_Cnt].sc_ApplicationNameAndLicenseVer ,
				sizeof( st_AdminauthFunctionEnabledfunctionCodeListTbl[ss_Cnt].sc_ApplicationNameAndLicenseVer ) , getCharSetForPanel() );
			Btx_AdminAuthenticationFunctionEnabledFunctionCodeList[us_ExistRow].showEnable( True );
			us_ExistRow++;
		}
	}
	for ( ss_Cnt = (PNLD_AUTHENTICATIONFUNCTIONENABLEDLIST-1) ; ss_Cnt >= us_ExistRow; ss_Cnt-- ) {
		Btx_AdminAuthenticationFunctionEnabledFunctionCodeList[ss_Cnt].showEnable( False );
	}
}
//====================================================================================================================.
/**
* @brief  : To Handle OK Key processing.
* @note   : PAdminAuthenticationFunctionEnableList : I291000400_認証機能有効化設定／有効機能一覧.
* @param  : uchar.
* @return : None.
* @author :	2015/03/09 HCLT_Archana [HeliosL][KRQ-101].
*/
//====================================================================================================================.
void PAdminAuthenticationFunctionEnableList::procOk( uchar option )
{
	if ( True == gp_UtilityMfpInfo.m_bl_FromUtilityMfpInfo ) {
		nextGamen = &gp_UtilityMfpInfo;					//	60801000_装置情報表示.
	} else {
// comment 2022/09/06 Sky西場 [IT6_2.3][OP_BTS-37620][スリム化].
		ExpPanelLog( PNLD_ExpLogKind_AlwaysLogOut, "[DeleteUnnecessaryProcessing] [%s]", __PRETTY_FUNCTION__, 0, 0, 0, 0, 0 );
//		nextGamen = &gp_AdminAuthenticationEnable;		//	I291000000_認証機能有効化設定.
	}
}

// comment 2016/1/25 OPS1 Sky飯田 [ZeusS_ZX0][CQ520659]セキュリティ強化モード時に設定変更しないにしても変更されている問題.
// ==================================================================.
// PAdminSecurityUSBConnectionPermissionDataLib : USB接続許可設定のデータクラス.
// ==================================================================.
/**
 *	@brief	不揮発への設定_外部メモリー(ユーザー).
 *	@param	無し.
 *	@retval	無し.
 */
void PAdminSecurityUSBConnectionPermissionDataLib::writeDataToNVD_ExtMemoryUser()
{
// comment 2017/09/04 【HCL-Ramkishor】【IT5_4.2】【RQ686】【WP975】【詳細制御仕様書_管理No.33】 The new API is used and to detect the current network type, new API arguments are passed.
	TYPE_NetworkSpecific e_NetworkSpecific = PNLC_NetworkLan::getInstance()->getAdminNWLine();
	// プロセス間通信の回数を減らす為、現在の設定と差異がある場合のみset関数をコールする.
	APIC_SystemSetting* pc_SystemSetting = PnlComGetSystemSetting();
	if( NULL != pc_SystemSetting ) {
// comment 2017/09/04 【HCL-Ramkishor】【IT5_4.2】【RQ686】【WP975】【詳細制御仕様書_管理No.33】 The new API is used and to detect the current network type, new API arguments are passed.
//		if(( pc_SystemSetting->getEnableUSBConnectAllUser() != m_bl_PermissionExtMemoryUserAll )
		if(( pc_SystemSetting->getEnableUSBConnectAllUser( e_NetworkSpecific ) != m_bl_PermissionExtMemoryUserAll )
		 &&( False == m_bl_PermissionExtMemoryUserAll )) {
			// 現在の設定から変更があり、かつ禁止に変更の場合.
// comment 2017/09/04 【HCL-Ramkishor】【IT5_4.2】【RQ686】【WP975】【詳細制御仕様書_管理No.33】 The new API is used and to detect the current network type, new API arguments are passed.
//			pc_SystemSetting->setEnableUSBConnectAllUser( m_bl_PermissionExtMemoryUserAll, JLGE_InspectLogIfOperationPanel );
//	comment 2017/10/02 【HCL-Suribabu】【IT5_4.2】【AR1031284】Modified the API values for setting to display the update the value. 
//			pc_SystemSetting->setEnableUSBConnectAllUser( m_bl_PermissionExtMemoryUserAll, JLGE_InspectLogIfOperationPanel, e_NetworkSpecific );
			pc_SystemSetting->setEnableUSBConnectAllUser( m_bl_PermissionExtMemoryUserAll, JLGE_InspectLogIfOperationPanel, False, NULL, TYPD_NotSetting, e_NetworkSpecific );
		} else if( True == m_bl_PermissionExtMemoryUserAll ) {
			// 個別設定の場合は、個々の項目を設定する為、ALLの設定に変更が無い場合も各項目をチェックする.
// comment 2017/09/04 【HCL-Ramkishor】【IT5_4.2】【RQ686】【WP975】【詳細制御仕様書_管理No.33】 The new API is used and to detect the current network type, new API arguments are passed.
//			if( pc_SystemSetting->getEnableUSBConnectAllUser() != m_bl_PermissionExtMemoryUserAll ) {
//				pc_SystemSetting->setEnableUSBConnectAllUser( m_bl_PermissionExtMemoryUserAll, JLGE_InspectLogIfOperationPanel );
			if( pc_SystemSetting->getEnableUSBConnectAllUser( e_NetworkSpecific ) != m_bl_PermissionExtMemoryUserAll ) {
//	comment 2017/10/02 【HCL-Suribabu】【IT5_4.2】【AR1031284】Modified the API values for setting to display the update the value. 
//				pc_SystemSetting->setEnableUSBConnectAllUser( m_bl_PermissionExtMemoryUserAll, JLGE_InspectLogIfOperationPanel, e_NetworkSpecific );
				pc_SystemSetting->setEnableUSBConnectAllUser( m_bl_PermissionExtMemoryUserAll, JLGE_InspectLogIfOperationPanel, False, NULL, TYPD_NotSetting, e_NetworkSpecific );
			}
			
			APIC_UsbMemSetting*	pc_UsbMemSetting = APIC_UsbMemSetting::newInstance();
			
			if( NULL != pc_UsbMemSetting ) {
				// 文書保存.
//Comment 2017/09/04 [HCL-Swathi] [IT5 4.2][RQ-686] [WP721]Multiple Network support.
//				if( pc_UsbMemSetting->getEnableScanToUsb() != m_bl_PermissionExtMemoryUserSaveDoc ) {
//					pc_UsbMemSetting->setEnableScanToUsb( m_bl_PermissionExtMemoryUserSaveDoc, JLGE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID() );
//				}
				
//				// 文書印刷.
//				if( pc_UsbMemSetting->getEnableUsbDirectPrint() != m_bl_PermissionExtMemoryUserPrint ) {
//					pc_UsbMemSetting->setEnableUsbDirectPrint( m_bl_PermissionExtMemoryUserPrint, JLGE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID() );
//				}
				
//				// 外部メモリー文書読込み.
//				if( pc_UsbMemSetting->getEnableUsbMemoryReading() != m_bl_PermissionExtMemoryUserReadDoc ) {
//					pc_UsbMemSetting->setEnableUsbMemoryReading( m_bl_PermissionExtMemoryUserReadDoc, JLGE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID() );
				TYPE_NetworkSpecific e_NetworkSpecific  = PNLC_NetworkLan::getInstance() ->getAdminNWLine();
				if( pc_UsbMemSetting->getEnableScanToUsb( e_NetworkSpecific ) != m_bl_PermissionExtMemoryUserSaveDoc ) {
					pc_UsbMemSetting->setEnableScanToUsb( m_bl_PermissionExtMemoryUserSaveDoc, JLGE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID(), e_NetworkSpecific );
				}
				
				// 文書印刷.
				if( pc_UsbMemSetting->getEnableUsbDirectPrint( e_NetworkSpecific ) != m_bl_PermissionExtMemoryUserPrint ) {
					pc_UsbMemSetting->setEnableUsbDirectPrint( m_bl_PermissionExtMemoryUserPrint, JLGE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID(), e_NetworkSpecific );
				}
				
				 //外部メモリー文書読込み.
				if( pc_UsbMemSetting->getEnableUsbMemoryReading( e_NetworkSpecific ) != m_bl_PermissionExtMemoryUserReadDoc ) {
					pc_UsbMemSetting->setEnableUsbMemoryReading( m_bl_PermissionExtMemoryUserReadDoc, JLGE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID(), e_NetworkSpecific );
				}
			}
		}
	}
}

/**
 *	@brief	不揮発への設定_外部メモリー(管理者).
 *	@param	無し.
 *	@retval	無し.
 */
void PAdminSecurityUSBConnectionPermissionDataLib::writeDataToNVD_ExtMemoryAdmin()
{
	// プロセス間通信の回数を減らす為、現在の設定と差異がある場合のみset関数をコールする.
	APIC_SystemSetting* pc_SystemSetting = PnlComGetSystemSetting();
	if( NULL != pc_SystemSetting ) {
		if(( pc_SystemSetting->getEnableUSBConnectAllAdmin() != m_bl_PermissionExtMemoryAdminAll )
		 &&( False == m_bl_PermissionExtMemoryAdminAll )) {
			// 現在の設定から変更があり、かつALLの設定を禁止に設定の場合.
			pc_SystemSetting->setEnableUSBConnectAllAdmin( m_bl_PermissionExtMemoryAdminAll, JLGE_InspectLogIfOperationPanel );
			
		} else if( True == m_bl_PermissionExtMemoryAdminAll ) {
			// 個別設定の場合は、個々の項目を設定する為、ALLの設定に変更が無い場合も各項目をチェックする.
			if( pc_SystemSetting->getEnableUSBConnectAllAdmin() != m_bl_PermissionExtMemoryAdminAll ) {
				pc_SystemSetting->setEnableUSBConnectAllAdmin( m_bl_PermissionExtMemoryAdminAll, JLGE_InspectLogIfOperationPanel );
			}
			
			APIC_FWUpdateSetting* pc_FWUpdateSetting = PnlComGetFWUpdateSetting();
			
			if( NULL != pc_FWUpdateSetting ) {
				// USBからの設定書込み.
				if( pc_FWUpdateSetting->getClientPeculiarSetting() != m_bl_PermissionExtMemoryAdminWriteConfigFromUSB ) {
					pc_FWUpdateSetting->setClientPeculiarSetting( m_bl_PermissionExtMemoryAdminWriteConfigFromUSB , JLGE_InspectLogIfOperationPanel, False, PnlComGetUser(), PnlComGetAuthorityID() );
				}
			}
		}
	}
}

/**
 *	@brief	不揮発への設定_外部メモリー(サービス).
 *	@param	無し.
 *	@retval	無し.
 */
void PAdminSecurityUSBConnectionPermissionDataLib::writeDataToNVD_ExtMemoryService()
{
	// プロセス間通信の回数を減らす為、現在の設定と差異がある場合のみset関数をコールする.
	APIC_SystemSetting* pc_SystemSetting = PnlComGetSystemSetting();
	if( NULL != pc_SystemSetting ) {
		if(( pc_SystemSetting->getEnableUSBConnectAllService() != m_bl_PermissionExtMemoryServAll )
		 &&( False == m_bl_PermissionExtMemoryServAll )) {
			// 現在の設定から変更があり、かつALLの設定を禁止に設定の場合.
			pc_SystemSetting->setEnableUSBConnectAllService( m_bl_PermissionExtMemoryServAll, JLGE_InspectLogIfOperationPanel );
			
			//サービス->デバッグ設定->その他 を「しない」に設定変更する.
			PNLC_QWSConnector* pc_QWSConnector = PNLC_QWSConnector::newInstance();
			pc_QWSConnector->setSupportType( PNLD_QSUPPORT_NONE );
			
		} else if( True == m_bl_PermissionExtMemoryServAll ) {
			// 個別設定の場合は、個々の項目を設定する為、ALLの設定に変更が無い場合も各項目をチェックする.
			if( pc_SystemSetting->getEnableUSBConnectAllService() != m_bl_PermissionExtMemoryServAll ) {
				pc_SystemSetting->setEnableUSBConnectAllService( m_bl_PermissionExtMemoryServAll, JLGE_InspectLogIfOperationPanel );
			}
			
			// ストレージデータバックアップ.
			if( pc_SystemSetting->getEnableHddDataBackup() != m_bl_PermissionExtMemoryServStorageDataBackUp ) {
				pc_SystemSetting->setEnableHddDataBackup( m_bl_PermissionExtMemoryServStorageDataBackUp, JLGE_InspectLogIfOperationPanel );
			}
			
			// FW更新.
			if( pc_SystemSetting->getEnableUSBConnectForFWDownload() != m_bl_PermissionExtMemoryServFWUpdate ) {
				pc_SystemSetting->setEnableUSBConnectForFWDownload( m_bl_PermissionExtMemoryServFWUpdate, JLGE_InspectLogIfOperationPanel );
			}
		}
	}
}

/**
 *	@brief	不揮発への設定_PC接続.
 *	@param	無し.
 *	@retval	無し.
 */
void PAdminSecurityUSBConnectionPermissionDataLib::writeDataToNVD_PCConnect()
{
	// プロセス間通信の回数を減らす為、現在の設定と差異がある場合のみset関数をコールする.
	APIC_SystemSetting* pc_SystemSetting = PnlComGetSystemSetting();
	if( NULL != pc_SystemSetting ) {
		if(( pc_SystemSetting->getEnableUSBConnectAllPC() != m_bl_PermissionPCConnectAll )
		 &&( False == m_bl_PermissionPCConnectAll )) {
			// 現在の設定から変更があり、かつALLの設定を禁止に設定の場合.
			pc_SystemSetting->setEnableUSBConnectAllPC( m_bl_PermissionPCConnectAll, JLGE_InspectLogIfOperationPanel );
			
		} else if( True == m_bl_PermissionPCConnectAll ) {
			// 個別設定の場合は、個々の項目を設定する為、ALLの設定に変更が無い場合も各項目をチェックする.
			if( pc_SystemSetting->getEnableUSBConnectAllPC() != m_bl_PermissionPCConnectAll ) {
				pc_SystemSetting->setEnableUSBConnectAllPC( m_bl_PermissionPCConnectAll, JLGE_InspectLogIfOperationPanel );
			}
			
			// PC印刷.
			if( pc_SystemSetting->getEnableUSBConnectForPC() != m_bl_PermissionPCConnectPCPrint ) {
				pc_SystemSetting->setEnableUSBConnectForPC( m_bl_PermissionPCConnectPCPrint, JLGE_InspectLogIfOperationPanel );
			}
			
			APIC_AirPrintSetting* pc_AirPrintSetting = PnlComGetAirPrintSetting();
			
			if( NULL != pc_AirPrintSetting ) {
				// AirPrint.
				if( pc_AirPrintSetting->getEnableUSB() != m_bl_PermissionPCConnectAirPrint ) {
					pc_AirPrintSetting->setEnableUSB( m_bl_PermissionPCConnectAirPrint, JLGE_InspectLogIfOperationPanel );
				}
			}
		}
	}
}

/**
 *	@brief	インスタンス取得処理.
 *	@param	無し.
 *	@retval	インスタンスのアドレス.
 */
PAdminSecurityUSBConnectionPermissionDataLib* PAdminSecurityUSBConnectionPermissionDataLib::getInstance()
{
	// インスタンスを生成.
	static PAdminSecurityUSBConnectionPermissionDataLib c_Instance;
	return &c_Instance;
}
/**
 *	@brief	不揮発への設定(データクラスのメンバ情報で不揮発へ設定する).
 *	@param	無し.
 *	@retval	無し.
 */
void PAdminSecurityUSBConnectionPermissionDataLib::writeDataToNVD()
{
	APIC_SystemSetting* pc_SystemSetting = PnlComGetSystemSetting();

	if( NULL != pc_SystemSetting ) {
		// USB接続許可設定が一括禁止か詳細設定か.
		if(( pc_SystemSetting->getEnableUSBConnectAll() != m_bl_PermissionSelect )
		 &&( False == m_bl_PermissionSelect )) {
			// 変更があり、かつ一括禁止設定の場合.
			pc_SystemSetting->setEnableUSBConnectAll( m_bl_PermissionSelect, JLGE_InspectLogIfOperationPanel );
			
		} else if( True == m_bl_PermissionSelect ) {
			// 変更が無くても詳細設定の場合は配下の項目を確認する.
			// プロセス間通信の回数を減らす為、現在の設定と差異がある場合のみset関数をコールする.
			if( pc_SystemSetting->getEnableUSBConnectAll() != m_bl_PermissionSelect ) {
				pc_SystemSetting->setEnableUSBConnectAll( m_bl_PermissionSelect, JLGE_InspectLogIfOperationPanel );
			}
			
			// 認証装置.
			if( pc_SystemSetting->getEnableUSBConnectForAuth() != m_bl_PermissionAuthDevice ) {
				pc_SystemSetting->setEnableUSBConnectForAuth( m_bl_PermissionAuthDevice, JLGE_InspectLogIfOperationPanel );
			}
			
			// 外部キーボード.
			if( pc_SystemSetting->getEnableUSBConnectForKeyboard() != m_bl_PermissionUSBKeyboard ) {
				pc_SystemSetting->setEnableUSBConnectForKeyboard( m_bl_PermissionUSBKeyboard, JLGE_InspectLogIfOperationPanel );
			}
			
			// 外部メモリー(ユーザー):ALL.
			writeDataToNVD_ExtMemoryUser();
			
			// 外部メモリー(管理者):ALL.
			writeDataToNVD_ExtMemoryAdmin();
			
			// 外部メモリー(サービス):ALL.
			writeDataToNVD_ExtMemoryService();
			
			// PC接続:ALL.
			writeDataToNVD_PCConnect();
		}
	}
}
/**
 *	@brief	不揮発からの取得(データクラスのメンバを更新する).
 *	@param	無し.
 *	@retval	無し.
 */
void PAdminSecurityUSBConnectionPermissionDataLib::getDataFromNVD()
{
	APIC_SystemSetting* pc_SystemSetting = PnlComGetSystemSetting();
	APIC_UsbMemSetting*	pc_UsbMemSetting = APIC_UsbMemSetting::newInstance();
	APIC_FWUpdateSetting* pc_FWUpdateSetting = PnlComGetFWUpdateSetting();
	APIC_AirPrintSetting* pc_AirPrintSetting = PnlComGetAirPrintSetting();
	
// comment 2017/09/04 【HCL-Ramkishor】【IT5_4.2】【RQ686】【WP975】【詳細制御仕様書_管理No.33】 The new API is used and to detect the current network type.
	TYPE_NetworkSpecific e_NetworkSpecific = PNLC_NetworkLan::getInstance()->getAdminNWLine();
	// 各メンバ変数の値を不揮発の値で更新する.
	if( NULL != pc_SystemSetting ) {
		m_bl_PermissionSelect								= pc_SystemSetting->getEnableUSBConnectAll();
		m_bl_PermissionAuthDevice							= pc_SystemSetting->getEnableUSBConnectForAuth();
		m_bl_PermissionUSBKeyboard							= pc_SystemSetting->getEnableUSBConnectForKeyboard();
// comment 2017/09/04 【HCL-Ramkishor】【IT5_4.2】【RQ686】【WP975】【詳細制御仕様書_管理No.33】 The new API is used and to detect the current network type, new API arguments are passed.
//		m_bl_PermissionExtMemoryUserAll						= pc_SystemSetting->getEnableUSBConnectAllUser();
		m_bl_PermissionExtMemoryUserAll						= pc_SystemSetting->getEnableUSBConnectAllUser( e_NetworkSpecific );
		m_bl_PermissionExtMemoryAdminAll					= pc_SystemSetting->getEnableUSBConnectAllAdmin();
		m_bl_PermissionExtMemoryServAll						= pc_SystemSetting->getEnableUSBConnectAllService();
		m_bl_PermissionExtMemoryServStorageDataBackUp		= pc_SystemSetting->getEnableHddDataBackup();
		m_bl_PermissionExtMemoryServFWUpdate				= pc_SystemSetting->getEnableUSBConnectForFWDownload();
		m_bl_PermissionPCConnectAll							= pc_SystemSetting->getEnableUSBConnectAllPC();
		m_bl_PermissionPCConnectPCPrint						= pc_SystemSetting->getEnableUSBConnectForPC();
	}
	if( NULL != pc_UsbMemSetting ) {
//Comment 2017/09/04 [HCL-Swathi] [IT5 4.2][RQ-686] [WP721]Multiple Network support.
//		m_bl_PermissionExtMemoryUserSaveDoc					= pc_UsbMemSetting->getEnableScanToUsb();
//		m_bl_PermissionExtMemoryUserPrint					= pc_UsbMemSetting->getEnableUsbDirectPrint();
//		m_bl_PermissionExtMemoryUserReadDoc					= pc_UsbMemSetting->getEnableUsbMemoryReading();
		TYPE_NetworkSpecific e_NetworkSpecific  = PNLC_NetworkLan::getInstance() ->getAdminNWLine();
		m_bl_PermissionExtMemoryUserSaveDoc					= pc_UsbMemSetting->getEnableScanToUsb( e_NetworkSpecific );
		m_bl_PermissionExtMemoryUserPrint					= pc_UsbMemSetting->getEnableUsbDirectPrint( e_NetworkSpecific );
		m_bl_PermissionExtMemoryUserReadDoc					= pc_UsbMemSetting->getEnableUsbMemoryReading( e_NetworkSpecific );
		
	}
	if( NULL != pc_FWUpdateSetting ) {
		m_bl_PermissionExtMemoryAdminWriteConfigFromUSB		= pc_FWUpdateSetting->getClientPeculiarSetting();
	}
	if( NULL != pc_AirPrintSetting ) {
		m_bl_PermissionPCConnectAirPrint					= pc_AirPrintSetting->getEnableUSB();
	}
}
/**
 *	@brief	設定内容が全て禁止かチェックする.
 *	@param	無し.
 *	@retval	Bool	True：全て禁止  False：全て禁止、ではない.
 */
Bool PAdminSecurityUSBConnectionPermissionDataLib::IsAllProhibit()
{
	Bool bl_ret = True;
	// メンバ変数をチェックどれか1つでもTrue(許可)の場合はFalseを返す.
	if(( m_bl_PermissionAuthDevice )
	 ||( m_bl_PermissionUSBKeyboard )
	 ||( m_bl_PermissionExtMemoryUserAll )
	 ||( m_bl_PermissionExtMemoryAdminAll )
	 ||( m_bl_PermissionExtMemoryServAll )
	 ||( m_bl_PermissionPCConnectAll )) {
		bl_ret = False;
	}
	
	return bl_ret;
}

// 2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
//	クラス	：	PAdminEncryptionSetting : K09801000_Sec_暗号化設定.
//	概要	：	コンストラクタ.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
PAdminEncryptionSetting::PAdminEncryptionSetting( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen )
	: PAdminToSecurityHDDManageSetcom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen )
// comment 2019/4/5 Sky黒田（達） [IT6_1.0MR][AR1075633] 暗号化設定画面でサブストレージ(HDD)キーが表示されている.
//	, m_b_SubStorageDisplay(True)
{
}
// ===============================================================================================.
//	クラス	：	PAdminEncryptionSetting : K09801000_Sec_暗号化設定.
//	概要	：	デストラクタ.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
PAdminEncryptionSetting::~PAdminEncryptionSetting()
{
}
// ===============================================================================================.
//	クラス	：	PAdminEncryptionSetting : K09801000_Sec_暗号化設定.
//	概要	：	画面開始処理.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminEncryptionSetting::startGamen()
{
	PAdminToSecurityHDDManageSetcom::startGamen();
	
	// メニュー画面データ.
	SP_UtilityAndAdminMenuData sp_UtilityAndAdminMenuData[] = {
		{ s_pKEYAdminMainStorageEncryption_W216H016I,	&gp_AdminMainStorageEncryptionSetConfirm },	// K09801100_Sec メインストレージ暗号化設定確認.
// comment 2019/4/5 Sky黒田（達） [IT6_1.0MR][AR1075633] 暗号化設定画面でサブストレージ(HDD)キーが表示されている.
//		{ s_pKEYAdminSubStorageEncryption_W216H016I,	&gp_AdminHDDCodingWordSetConfirmation },	// 10980720 HDD暗号化設定確認.
		{ NULL,			NULL },
		{ NULL,			NULL },
		{ NULL,			NULL },
		{ NULL,			NULL },
		{ NULL,			NULL },
		{ NULL,			NULL },
		{ NULL, 		NULL },
		{ NULL,			NULL },
		{ NULL,			NULL },
	};

// comment 2019/4/5 Sky黒田（達） [IT6_1.0MR][AR1075633] 暗号化設定画面でサブストレージ(HDD)キーが表示されている.
//	if( True == PnlComGetHddInfo()->getHddExist() )	{			// HDD装着時、HDD暗号化設定を表示.
//		m_b_SubStorageDisplay = True;
//	 } else {
//	 	m_b_SubStorageDisplay = False;	
//	 }

	pc_MenuUtilityAndAdmin.initialSPC( sp_UtilityAndAdminMenuData );	// メニュー画面データセット.
	// 左エリア種類の有効設定（しおり表示）startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key);
}
// ===============================================================================================.
//	クラス	：	PAdminEncryptionSetting : K09801000_Sec_暗号化設定.
//	概要	：	表示部品のセットアップ.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminEncryptionSetting::setupGamen( short ss_option )
{
	PAdminToSecurityHDDManageSetcom::setupGamen( ss_option );

// comment 2019/4/5 Sky黒田（達） [IT6_1.0MR][AR1075633] 暗号化設定画面でサブストレージ(HDD)キーが表示されている.
//	if( True == m_b_SubStorageDisplay){
//		pc_MenuUtilityAndAdmin.changeSPCStatus(2 , PNLE_PARTSSTATUS_ENABLE );	// サブストレージ(HDD)キー表示.
//	} else {
//		pc_MenuUtilityAndAdmin.changeSPCStatus(2 , PNLE_PARTSSTATUS_HIDE );		// サブストレージ(HDD)キー非表示.
//	}
	pc_MenuUtilityAndAdmin.SPCsetupGamen();
}
//================================================================================================.
//  クラス	：	PAdminEncryptionSetting : K09801000_Sec_暗号化設定.
//	概要	：	OKボタン押下.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
//================================================================================================.
void PAdminEncryptionSetting::procOk( uchar uc_option )
{
	nextGamen = &gp_AdminSecurityHDDManageSet;		// 管理者設定・環境設定・ストレージ管理設定.
}
//================================================================================================.
//  クラス	：	PAdminEncryptionSetting : K09801000_Sec_暗号化設定.
//	概要	：	画面遷移.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
//================================================================================================.
void PAdminEncryptionSetting::procGoToPage( uchar uc_option )
{
	nextGamen = pc_MenuUtilityAndAdmin.SPCprocGoToPage( uc_option );
}
//================================================================================================.
//  クラス	：	PAdminEncryptionSetting : K09801000_Sec_暗号化設定.
//	概要	：	テンキー.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
//================================================================================================.
void PAdminEncryptionSetting::proc10Key( uchar uc_option )
{
	nextGamen = pc_MenuUtilityAndAdmin.SPCproc10Key( uc_option );
}
//================================================================================================.
//  クラス	：	PAdminEncryptionSetting : K09801000_Sec_暗号化設定.
//	概要	：	デフォルト背景表示部品の設定処理.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
//================================================================================================.
void PAdminEncryptionSetting::setDefaultBackParts()
{
	setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminEncryptionSettingDefault, ss_AdminEncryptionSettingDefault );
}

// 2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetConfirm : K09801100_Sec_メインストレージ暗号化確認.
//	概要	：	コンストラクタ.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
PAdminMainStorageEncryptionSetConfirm::PAdminMainStorageEncryptionSetConfirm( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen )
	: PAdminToSecurityHDDManageSetcom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen )
	, m_b_KeySelect(False)
	, m_b_EnableEncryption(False)
{
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetConfirm : K09801100_Sec_メインストレージ暗号化確認.
//	概要	：	デストラクタ.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
PAdminMainStorageEncryptionSetConfirm::~PAdminMainStorageEncryptionSetConfirm()
{
}
//================================================================================================.
//  クラス	：	PAdminMainStorageEncryptionSetConfirm : K09801100_Sec_メインストレージ暗号化確認.
//	概要	：	OKボタン押下.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
//================================================================================================.
void PAdminMainStorageEncryptionSetConfirm::procButtonOn( uchar uc_option )
{
	switch ( uc_option ) {
		case DEF_BTNID_KEY_AdminMainStorageEncryptionSetConfirmYes:
			m_b_KeySelect = True;		// 「はい」キー選択.
			break;
		case DEF_BTNID_KEY_AdminMainStorageEncryptionSetConfirmNo:
			m_b_KeySelect = False;		// 「いいえ」キー選択.
			break;
		default:
			break;
	}
	setupGamen();
}
//================================================================================================.
//  クラス	：	PAdminMainStorageEncryptionSetConfirm : K09801100_Sec_メインストレージ暗号化確認.
//	概要	：	OKボタン押下.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
//			：	2016/02/11 OPS1-motohashi [SecV2-4] 暗号化ワード登録時は文字列長設定画面への遷移に変更.
//================================================================================================.
void PAdminMainStorageEncryptionSetConfirm::procOk( uchar uc_option )
{
	if( True == m_b_KeySelect){
		// 「はい」選択時.
		if ( True == m_b_EnableEncryption ){
			// 暗号化設定実施時.
			nextGamen = &gp_AdminMainStorageEncryptionSetChange;	// K09801120_Sec_メインストレージ暗号化設定変更／解除.
		} else {
			// 暗号化設定未実施時.
			PLBC_CheckFromMenu::getInstance()->setFromMenu( True );
			nextGamen = &gp_AdminMainStorageEncStrLenSelect;		// メインストレージ暗号化ワード文字列長選択.
		}
	} else {
		// 「いいえ」選択時.
		nextGamen = &gp_AdminEncryptionSetting;						// K09801000_Sec_暗号化設定.
	}		
}
//================================================================================================.
//  クラス	：	PAdminMainStorageEncryptionSetConfirm : K09801100_Sec_メインストレージ暗号化確認.
//	概要	：	画面開始処理.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
//================================================================================================.
void PAdminMainStorageEncryptionSetConfirm::startGamen()
{
	PAdminToSecurityHDDManageSetcom::startGamen();
	m_b_KeySelect = False;
	// 2016/06/01 SCD2-Sky Shindo [SecV2] [CQ530312] 20文字選択時、セキュリティー強化モードを解除する.
	gp_AdminMainStorageEncStrLenSelect.setFromUpperGamen( False );

	APIC_SystemSetting* pc_SystemSetting = APIC_SystemSetting::newInstance();
	if( NULL != pc_SystemSetting ){				// インスタンスNULLチェック.
		m_b_EnableEncryption = pc_SystemSetting->getEnableMainStorageEncryption();
	}
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
	else {
//		LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
//		if ( NULL != pc_BFW_System ) {
//	   		pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900650010B56001);
//		}
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, 3, "Assert Failed!" );
	}
	// 左エリア種類の有効設定（しおり表示）startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key);
}
//================================================================================================.
//  クラス	:	PAdminMainStorageEncryptionSetConfirm : K09801100_Sec_メインストレージ暗号化確認.
//	概要	：	表示部品のセットアップ.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
//================================================================================================.
void PAdminMainStorageEncryptionSetConfirm::setupGamen( short ss_option )
{
// comment 2020/09/23 Sky西村 [IT6-51] HDD暗号化設定画面の表示メッセージの切り替え.
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	if( NULL == pc_SoftDipSw ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	if( True == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW239, 6 ) ) {
		APIC_SystemSetting* pc_SystemSetting = APIC_SystemSetting::newInstance();
		if( NULL == pc_SystemSetting ) {
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return;
		}
		char sc_NowPassword[TYPD_AdminLockPassMax+1] = {0};
		pc_SystemSetting->getHddLockPassword( sc_NowPassword );
		Bool bl_StorageLockPassword	= ( '\0' != sc_NowPassword[0] ) ? True : False;
		Bool bl_StorageEncryption	= pc_SystemSetting->getEnableMainStorageEncryption();
		
		if( False == bl_StorageLockPassword ) {
			if( False == bl_StorageEncryption ) {
				// DipSW有効、ロック無効、暗号化無効.
				Txt_AdminMainStorageEncryptionSetConfirmMessage.changeText( s_pMSGAdminMainStorageEncryptionSetConfirmDisableEncryptionDisableLock_W480H144I );
			} else {
				// DipSW有効、ロック無効、暗号化有効.
				Txt_AdminMainStorageEncryptionSetConfirmMessage.changeText( s_pMSGAdminMainStorageEncryptionSetConfirmEnableEncryptionDisableLock_W480H144I );
			}
			Rbt_AdminMainStorageEncryptionSetConfirmKey.showEnable( True );
		} else {
			if( False == bl_StorageEncryption ) {
				// DipSW有効、ロック有効、暗号化無効.
				Txt_AdminMainStorageEncryptionSetConfirmMessage.changeText( s_pMSGAdminMainStorageEncryptionSetConfirmDisableEncryptionEnableLock_W480H144I );
			} else {
				// DipSW有効、ロック有効、暗号化有効.
				Txt_AdminMainStorageEncryptionSetConfirmMessage.changeText( s_pMSGAdminMainStorageEncryptionSetConfirmEnableEncryptionEnableLock_W480H144I );
			}
			Rbt_AdminMainStorageEncryptionSetConfirmKey.showEnable( False );
		}
	} else {
		Txt_AdminMainStorageEncryptionSetConfirmMessage.changeText( s_pMSGAdminMainStorageEncryptionSetConfirm_W480H048I );
		Rbt_AdminMainStorageEncryptionSetConfirmKey.showEnable( True );
	}
	
	// はい/いいえキーの選択表示.
	Rbt_AdminMainStorageEncryptionSetConfirmKey.select( True == m_b_KeySelect ? 0 : 1  );
	PAdminToSecurityHDDManageSetcom::setupGamen( ss_option );
}
//================================================================================================.
//  クラス	：	PAdminMainStorageEncryptionSetConfirm : K09801100_Sec_メインストレージ暗号化確認.
//	概要	：	デフォルト背景表示部品の設定処理.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
//================================================================================================.
void PAdminMainStorageEncryptionSetConfirm::setDefaultBackParts()
{
	setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminMainStorageEncryptionSetConfirmDefault, ss_AdminMainStorageEncryptionSetConfirmDefault );
}

// 2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetEntry : K09801110_Sec_メインストレージ暗号化設定新規登録.
//	概要	：	コンストラクタ.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
//			：	2016/02/11 OPS1-motohashi [SecV2-4] 暗号化ワードの文字列長設定用のメンバ変数初期化.
// ===============================================================================================.
PAdminMainStorageEncryptionSetEntry::PAdminMainStorageEncryptionSetEntry( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen )
	: PAdminKeyBoardCom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen )
	, m_uc_error(PNLE_PasswordErrorType_Non)
	, m_ss_MaxStrLen( 0 )
{
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetEntry : K09801110_Sec_メインストレージ暗号化設定新規登録.
//	概要	：	デストラクタ.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
PAdminMainStorageEncryptionSetEntry::~PAdminMainStorageEncryptionSetEntry()
{
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetEntry : K09801110_Sec_メインストレージ暗号化設定新規登録.
//	概要	：	画面開始処理.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
//			：	2016/02/11 OPS1-motohashi [SecV2-4] 暗号化ワードの入力可能文字列長設定の追加.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetEntry::startGamen()
{
	// パスワード再入力の画面開始処理をコール.
	PAdminKeyBoardCom::startGamenPasswordReInput();
// comment 2019/3/12 Sky黒田（達） [Eagle][制御構成変更_No.10] SSD装着時のストレージ暗号化機能対応.
//	PLBC_WideKeyboard::getInstance()->startWideKeyboard( PNLE_WideKeyboard_Normal, &Txt_UtilityAndAdminComTitle, PNLE_WideKeyboardExNone ) ;
	PLBC_WideKeyboard::getInstance()->startWideKeyboard( PNLE_WideKeyboard_Normal, &Txt_UtilityAndAdminComTitle, PNLE_WideKeyboardExTwoEnterPassword ) ;
	// 初期表示以外は文字列を初期化しない.
	if ( PLBD_Enl_KBD_Initial == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) 
	{
		m_uc_error = PNLE_PasswordErrorType_Non;
	//	2023/09/11 FPT-DuongDV11 【OP_BTS-44872】 fix strncpy-param-overlap.
	//	strncpy( sc_StrKeyboard , "", sizeof(sc_StrKeyboard));
		char sc_Password[m_ss_MaxStrLen+1];
		memset( sc_Password, 0x00, sizeof( sc_Password ) );
// comment 2019/04/13 HCLT-Taqiuddin [IT6_1.0MR2][AR1075864] Accommodating all the 64 characters in the Encryption passphrase screen.
// comment 2019/04/22 HCLT-Prashannaa [IT6_1.0MR2][AR1075864] reverting the fix.
	// 	setKeyboardData( sc_StrKeyboard, m_ss_MaxStrLen, TYPD_AdminKeybordMax );
	//	setKeyboardData( sc_StrKeyboard, m_ss_MaxStrLen, TYPD_AdminEncryptionKeybordMax );
		setKeyboardData( sc_Password, m_ss_MaxStrLen, TYPD_AdminKeybordMax );
	}

	PAdminKeyBoardCom::startGamen();
	PLBC_WideKeyboard::getInstance()->initInputArea(&Kbd_UtilityInputKeybordConv,&c_SlideTextMgr,sc_StrKeyboard, Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MaxInputNum,pc_ActivatableField,getCharSetForPanel() );	// スライド文字列管理クラスの初期化.
	// ユーティリティ専用キーボード共通開始処理.
	startGamenKeybordUtility9E();

// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 Start.
//	// キーボードを５ヶ国語キーボードとする.
//	Kbd_UtilityInputKeybordConv.setKeybord5Country( True );
	
	// キーボード初期状態判定フラグをFalseとする.
	onlyPanel.b_InitKeybord = False;


	// タイトルの表示.
	Txt_UtilityAndAdminComTitle.changeText( s_pTTLAdminMainStorageEncryption_W608H016I );

// 2016/03/15 CQ525871 文言修正：パスワード->暗号化ワード Sky大嶋.
	Key_PasswordReInput.changeText( s_KEYCodingWord );
	Key_PasswordReInputCheck.changeText( s_KEYCodingWordConfirm );

	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key);
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetEntry : K09801110_Sec_メインストレージ暗号化設定新規登録.
//	概要	：	画面終了処理.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetEntry::endGamen()
{
	PLBC_WideKeyboard::getInstance()->initInputArea(&Kbd_UtilityInputKeybordConv,&c_SlideTextMgr,sc_StrKeyboard, Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MaxInputNum,&Btx_InputDspAreaWide,getCharSetForPanel(), 1, -1, False );	// スライド文字列管理クラスの初期化 .
	PLBC_WideKeyboard::getInstance()->endWideKeyboard() ;
	Key_PasswordReInput.changeText( s_KEYPassword );
	Key_PasswordReInputCheck.changeText( s_KEYPasswordConfirm );

	// パスワード再入力の画面終了処理をコール.
	PAdminKeyBoardCom::endGamenPasswordReInput();
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetEntry : K09801110_Sec_メインストレージ暗号化設定新規登録.
//	概要	：	表示画面のセットアップ.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetEntry::setupGamen( short ss_option )
{
	if ( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
		// 標準のキーボード画面固有パーツを消す.
		Grp_KeybordTotalPartsO6.showEnable( False );
		Txt_MessageAdminNewHDDCodingWordEntryWide.showEnable( False );
	}	
	PAdminKeyBoardCom::setupGamen( ss_option );
	
	// メッセージ表示処理.
	switch ( m_uc_error )
	{
		case PNLE_PasswordErrorType_CompareError:		// パスワードは不一致時.
			Txt_MessageAdminNewHDDCodingWordEntryWide.changeText( s_MSGMessageAdminSecurityPasswordNotAgreeError );
			break;
		case PNLE_PasswordErrorType_ProhibitError:		// パスワードは禁止時.
			Txt_MessageAdminNewHDDCodingWordEntryWide.changeText( s_MSGMessageAdminSecurityPasswordNotUseError );
			break;
		default:
			Txt_MessageAdminNewHDDCodingWordEntryWide.changeText( s_MSGCodingWord2Input );
			break;
	}
	
	Kbd_UtilityInputKeybordConv.setDisableKeyCharCode( KC_DisableKeyPattern6 );	// パターン６("()>?[;:],\).

	// パスワード再入力のセットアップ画面処理をコール.
	PAdminKeyBoardCom::setupGamenPasswordReInput( ss_option );

// 20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
	char sc_StrInput[PCGD_MaxKeybordInfoBuffer] = {0};
	char sc_StrInputCheck[PCGD_MaxKeybordInfoBuffer] = {0};
	if ( True == b_PasswordSelect ) {
		memset( sc_StrInput ,  0x00, sizeof( sc_StrInput ) );
		strncpy( sc_StrInput, sc_StrKeyboard, ( sizeof( sc_StrInput ) - 1 ) );

		memset( sc_StrInputCheck , 0x00, sizeof( sc_StrInputCheck ) );
		strncpy( sc_StrInputCheck, sc_StrKeyboard2Check, sizeof( sc_StrInputCheck )-1 );
	} else {
		memset( sc_StrInput ,  0x00, sizeof( sc_StrInput ) );
		strncpy( sc_StrInput, sc_StrKeyboard2, ( sizeof( sc_StrInput ) - 1 ) );

		memset( sc_StrInputCheck , 0x00, sizeof( sc_StrInputCheck ) );
		strncpy( sc_StrInputCheck, sc_StrKeyboard, sizeof( sc_StrInputCheck )-1 );
	}

	if ( ( ( getNativeCodeByteCount( sc_StrInput ) == 0 ) && ( getNativeCodeByteCount( sc_StrInputCheck ) == 0 ) )
	  || ( ( getNativeCodeByteCount( sc_StrInput ) == m_ss_MaxStrLen ) && ( getNativeCodeByteCount( sc_StrInputCheck ) == m_ss_MaxStrLen ) ) )
	{
		if (  PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
			// 拡大キーボードの時.
			Key_EnlKeyboardOK.activate( True );		// OKキー(拡大).
		} else {
			// 標準キーボードの時.
			Key_UtilityOk.activate( True );			// OKキー(標準).
		}
	} else {
		if (  PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
			// 拡大キーボードの時.
			Key_EnlKeyboardOK.activate( False );	// OKキー(拡大).
		} else {
			// 標準キーボードの時.
			Key_UtilityOk.activate( False );		// OKキー(標準).
		}
	}
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetEntry : K09801110_Sec_メインストレージ暗号化設定新規登録.
//	概要	：	中止キー（標準）押下.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetEntry::procCancel( uchar uc_option )
{
// 20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
//	nextGamen = &gp_AdminEncryptionSetting;		// K09801000_Sec_暗号化設定.
	nextGamen = &gp_AdminMainStorageEncStrLenSelect;	// 暗号化ワード長選択.
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetEntry : K09801110_Sec_メインストレージ暗号化設定新規登録.
//	概要	：	中止キー（拡大）押下.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetEntry::procReturn( uchar uc_option )
{
// 20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
//	nextGamen = &gp_AdminEncryptionSetting;		// K09801000_Sec_暗号化設定.
	nextGamen = &gp_AdminMainStorageEncStrLenSelect;	// 暗号化ワード長選択.
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetEntry : K09801110_Sec_メインストレージ暗号化設定新規登録.
//	概要	：	クリア押下.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetEntry::procClear( uchar uc_option )
{    
	PAdminKeyBoardCom::procClearPasswordReInput( uc_option );
	setupGamen();
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetEntry : K09801110_Sec_メインストレージ暗号化設定新規登録.
//	概要	：	OKキー押下.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
//			：	2016/02/11 OPS1-motohashi [SecV2-4] 暗号化ワード長選択追加によりチェック時にも前回／現在の文字列長考慮を追加.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetEntry::procOk( uchar uc_option )
{
	Bool b_isProhibit = False;		// 禁止フラグ.
	Bool b_isDifferent = False;		// 不一致フラグ.

	if ( True == b_PasswordSelect )
	{
		strncpy( sc_StrKeyboard2, sc_StrKeyboard, ( sizeof( sc_StrKeyboard2 ) - 1 ) );
		sc_StrKeyboard2[ sizeof( sc_StrKeyboard2 ) - 1 ] = '\0';
	}
	else
	{
		strncpy( sc_StrKeyboard2Check, sc_StrKeyboard, sizeof( sc_StrKeyboard2Check )-1 );
		sc_StrKeyboard2Check[sizeof(sc_StrKeyboard2Check)- 1] = '\0';
	}

// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 Start.
	// 入力したﾜｰﾄﾞが前回の暗号化ﾜｰﾄﾞと一致なら、該当パスワードは使用禁止.
	if(PnlComGetSystemSetting()->chkMainStorageEncryptionPassword(sc_StrKeyboard2))
	{
		b_isProhibit = True;
	}

	// 入力文字制限数であることの確認 .
	if((ushort)m_ss_MaxStrLen != strlen(sc_StrKeyboard2))
	{
		b_isProhibit = True;
	}

	// 同一文字のみのパスワードでないことの確認 .
	Bool b_check = True;
	for( short i = 1; sc_StrKeyboard2[i] != '\0' && i < m_ss_MaxStrLen; i++ )
	{
		if ( sc_StrKeyboard2[0] != sc_StrKeyboard2[i] )
		{
			b_check = False;
			break;
		}
	}
	b_isProhibit |= b_check;

	// ユーロ記号は受け付けない.
	for( short i = 0 ; i < m_ss_MaxStrLen; i++ )
	{
		if( char(0x80) == sc_StrKeyboard2[i] )
		{
			b_isProhibit = True;
		}
	}

	// 入力したﾜｰﾄﾞが確認入力と一致していることの確認.
	if ( 0 != strncmp( sc_StrKeyboard2,  sc_StrKeyboard2Check, m_ss_MaxStrLen ))
	{
		b_isDifferent = True;
	}

	if ( b_isDifferent || b_isProhibit )
	{
		// 禁止または不一致の場合、エラーフラグを立てて画面を初期化する.
		setBuzzerType( PCGE_BuzzerType_InputNg );

		if ( b_isProhibit )
		{
			// パスワードエラー「禁止」を設定.
			m_uc_error = PNLE_PasswordErrorType_ProhibitError;
		}
		else if ( b_isDifferent )
		{
			// パスワードエラー「不一致」を設定.
			m_uc_error = PNLE_PasswordErrorType_CompareError;
		}

		// エラーの時、入力をクリア.
		strncpy( sc_StrKeyboard, "", sizeof( sc_StrKeyboard ) );
		strncpy( sc_StrKeyboard2, "", sizeof( sc_StrKeyboard2 ) );
		strncpy( sc_StrKeyboard2Check, "", sizeof( sc_StrKeyboard2Check) );

		// 入力表示を更新.
		pc_ActivatableField = &Btx_PasswordReInputCheck;
		Kbd_UtilityInputKeybordConv.GetCursor( pc_ActivatableField );
		c_SlideTextMgr.init_CLCD( sc_StrKeyboard, Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MaxInputNum, pc_ActivatableField, getCharSetForPanel() );

		pc_MmbDispField = &Btx_PasswordReInput;
		Kbd_UtilityInputKeybordConv.GetCursor( pc_ActivatableField );
		c_SlideTextMgr.init_CLCD( sc_StrKeyboard, Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MaxInputNum, pc_ActivatableField, getCharSetForPanel() ); 

		// 選択状態を初期化.
		b_PasswordSelect = True;
		
		procClear( uc_option );

		if ( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
		{
			// 拡大の時、「拡大OFFキー」と同じ処理をする.
			procButtonOn( 100 );
		}
		else
		{
			// 画面セットアップ.
			setupGamen();
		}

	}
	else
	{
		// 暗号化ﾜｰﾄﾞの保存.
		gp_AdminMainStorageEncryptionSetAct.setMainStorageEncriptionSetting( True, sc_StrKeyboard2 );

// 2016/06/01 SCD2-Sky Shindo [SecV2] [CQ530312] 20文字選択時、セキュリティー強化モードを解除する.
		APIC_SystemSetting* pc_SystemSetting = APIC_SystemSetting::newInstance();
		if ( NULL == pc_SystemSetting ) {
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//			LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
//			if ( NULL != pc_BFW_System ) {
//		   		pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900030010B56101);
//			}
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, 3, "Assert Failed!" );
			return;
		}
		if( True == pc_SystemSetting->getSecurityLevelUpOn()
		 && PNLD_AdminStorageEncStrLenSel_MaxLen_Ascii20Char == m_ss_MaxStrLen ){
			// セキュリティー強化設定をOFFに設定する.
			PnlComGetSystemSetting()->setSecurityLevelUpOn( False, PnlComGetUser(), JLGE_InspectLogIfOperationPanel);
		}
		
		// メインストレージ暗号化処理中画面へ. 
		nextGamen = &gp_AdminMainStorageEncryptionSetAct;
	}
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetEntry : K09801110_Sec_メインストレージ暗号化設定新規登録.
//	概要	：	OKキー押下.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetEntry::procButtonOn( uchar uc_option )
{
	// パスワード再入力の押されたボタン処理をコール.
	PAdminKeyBoardCom::procButtonOnPasswordReInput( uc_option );
	// 画面セットアップ.
	setupGamen();
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetEntry : K09801110_Sec_メインストレージ暗号化設定新規登録.
//	概要	：	.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetEntry::setDefaultBackParts()
{
	if (( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() )
		 && ( True == onlyPanel.b_InitKeybord )) {	// 拡大用デフォルト背景.
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminMainStorageEncryptionSetEntryDefaultEnl, ss_AdminMainStorageEncryptionSetEntryEnl );
	} else {	// 標準用デフォルト背景.
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminMainStorageEncryptionSetEntryDefault, ss_AdminMainStorageEncryptionSetEntry );
	}
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetEntry : K09801110_Sec_メインストレージ暗号化設定新規登録.
//	概要	：	.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2016/02/11 OPS1-motohashi [SecV2-4] 暗号化ワードの文字列長選択を行えるように設定メソッド追加.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetEntry::setMaxStrLen( short ss_MaxStrLen )
{
	m_ss_MaxStrLen = ss_MaxStrLen;
}

// 2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetChange : K09801120_Sec_メインストレージ暗号化設定変更／解除.
//	概要	：	コンストラクタ.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
PAdminMainStorageEncryptionSetChange::PAdminMainStorageEncryptionSetChange( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen )
	: PAdminKeyBoardCom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen )
	, m_uc_KeySelect(PNLD_AdminMainStorageEncryptionSetChange)
	, m_b_error(False)
{
// 2016/06/01 SCD2-Sky Shindo [SecV2] [CQ530312] 20文字選択時、セキュリティー強化モードを解除する.
//// comment 2016/05/11 SCD2-2G-Sky延原 [SecV2] [CQ528021] 20文字選択時、セキュリティー強化モードを解除する、付随修正.
//	bl_FromUpperGamen = False;				// 下位画面への遷移元画面通知フラグ初期化.
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetChange : K09801120_Sec_メインストレージ暗号化設定変更／解除.
//	概要	：	デストラクタ.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
PAdminMainStorageEncryptionSetChange::~PAdminMainStorageEncryptionSetChange()
{
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetChange : K09801120_Sec_メインストレージ暗号化設定変更／解除.
//	概要	：	画面開始処理.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetChange::startGamen()
{
// comment 2019/3/12 Sky黒田（達） [Eagle][制御構成変更_No.10] SSD装着時のストレージ暗号化機能対応.
//	PLBC_WideKeyboard::getInstance()->startWideKeyboard( PNLE_WideKeyboard_Normal, &Txt_UtilityAndAdminComTitle, PNLE_WideKeyboardExNone ) ;
	PLBC_WideKeyboard::getInstance()->startWideKeyboard( PNLE_WideKeyboard_Normal, &Txt_UtilityAndAdminComTitle, PNLE_WideKeyboardExTwoEnterPassword ) ;
	// パスワード表示フラグ.
	b_Password = True;
	// 表示エリア.
	pc_DispField = &Tfdc_AdminNetworkSetComInputKeybord;
	pc_ActivatableField = &Btx_AdminNetworkSetComInputKeybord2;
	//キーボードクラスへカーソルのアドレスを渡す.
	Kbd_UtilityInputKeybordConv.GetCursor( pc_ActivatableField );
	// キーボード初期状態の場合の処理.
	if ( False == onlyPanel.b_InitKeybord ) {
		Kbd_UtilityInputKeybordConv = PLBC_ConvertKeybord( PLBD_KBT_ASCII_S, 192, 184 );	// 位置をずらす.
		// キーボード切換え状態を初期状態とする.
		Kbd_UtilityInputKeybordConv.setChangeEnlKeybord( PLBD_Enl_KBD_Initial );
	}
	// 拡大表示の場合、入力文字表示領域を変更.
	if ( True == Kbd_UtilityInputKeybordConv.setDispFieldEnl(&Btx_EnlChangeKeybord) ) { pc_ActivatableField = &Btx_EnlChangeKeybord; }
	// キーボードにデータをセット.
	char _sc_Password[TYPD_MainStorageEncryptionPassword_MaxLen+1];
	// 初期表示以外は文字列を初期化しない.
	if ( PLBD_Enl_KBD_Initial == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
		memset( _sc_Password, 0, sizeof(_sc_Password) );
		setKeyboardData( _sc_Password, TYPD_MainStorageEncryptionPassword_MaxLen, TYPD_AdminKeybordMax );
		m_b_error = False;				// エラーメッセージ表示フラグ.
	}
	PAdminKeyBoardCom::startGamen();
	// ユーティリティ専用キーボード共通開始処理.
	startGamenKeybordUtility9E();

// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 Start.
//	// キーボードを５ヶ国語キーボードとする.
//	Kbd_UtilityInputKeybordConv.setKeybord5Country( True );
	
	// キーボード初期状態判定フラグをFalseとする.
	onlyPanel.b_InitKeybord = False;

	PLBC_WideKeyboard::getInstance()->initInputArea(&Kbd_UtilityInputKeybordConv,&c_SlideTextMgr,sc_StrKeyboard, Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), ss_MaxInputNum,&Btx_InputDspAreaWide_2,getCharSetForPanel()) ;	// スライド文字列管理クラスの初期化 .

	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key);

// 2016/06/01 SCD2-Sky Shindo [SecV2] [CQ530312] 20文字選択時、セキュリティー強化モードを解除する.
//// comment 2016/05/11 SCD2-2G-Sky延原 [SecV2] [CQ528021] 20文字選択時、セキュリティー強化モードを解除する、付随修正.
//	bl_FromUpperGamen = False;				// 下位画面への遷移元画面通知フラグ初期化.

}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetChange : K09801120_Sec_メインストレージ暗号化設定変更／解除.
//	概要	：	画面終了処理.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetChange::endGamen()
{
	PLBC_WideKeyboard::getInstance()->endWideKeyboard() ;
	PAdminKeyBoardCom::endGamen() ;
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetChange : K09801120_Sec_メインストレージ暗号化設定変更／解除.
//	概要	：	表示画面のセットアップ.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetChange::setupGamen( short ss_option )
{
	// キーボード標準状態のsetup処理.
	if ( PLBD_Enl_KBD_Enlarge != Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
		Rbt_AdminMainStorageEncryptionSetChangeKey.showEnable( True );		// 変更／解除キー.
	} else {
	// キーボード拡大状態のsetup処理.
		Grp_KeybordTotalPartsO6.showEnable( False );						// 標準のキーボード画面固有パーツを消す.
		Btx_AdminNetworkSetComInputKeybord2.showEnable( False );			// 標準のキーボード画面固有パーツを消す.
		Txt_MessageAdminSecurityPasswordNotAgreeError.showEnable( False );	// 暗号化ﾜｰﾄﾞが一致しません。入力し直してください.
		Txt_MessageAdminHDDCodingWordMenu.showEnable( False );				// 現在の暗号化ﾜｰﾄﾞを入力し、項目を選択してください.
		Rbt_AdminMainStorageEncryptionSetChangeKey.showEnable( False );		// 変更／解除キー.
	}
	PAdminKeyBoardCom::setupGamen( ss_option );

	if ( PLBD_Enl_KBD_Enlarge != Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
	// キーボード標準状態のメッセージ表示処理.
		if ( False == m_b_error ) {
			// 通常メッセージ.
			Txt_MessageAdminSecurityPasswordNotAgreeError.showEnable( False );	// 暗号化ﾜｰﾄﾞが一致しません。入力し直してください.
			Txt_MessageAdminHDDCodingWordMenu.showEnable( True );				// 現在の暗号化ﾜｰﾄﾞを入力し、項目を選択してください.
		} else {
			// エラーメッセージ. 
			Txt_MessageAdminHDDCodingWordMenu.showEnable( False );				// 現在の暗号化ﾜｰﾄﾞを入力し、項目を選択してください.
			Txt_MessageAdminSecurityPasswordNotAgreeError.showEnable( True );	// 暗号化ﾜｰﾄﾞが一致しません。入力し直してください.
		}
	}
	Kbd_UtilityInputKeybordConv.setDisableKeyCharCode( KC_DisableKeyPattern6 );	// パターン６("()>?[;:],\).
	Rbt_AdminMainStorageEncryptionSetChangeKey.select( m_uc_KeySelect );		// 変更／解除キー反転表示.
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetChange : K09801120_Sec_メインストレージ暗号化設定変更／解除.
//	概要	：	中止キー（標準）押下.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetChange::procCancel( uchar uc_option )
{
	nextGamen = &gp_AdminEncryptionSetting;		// K09801000_Sec_暗号化設定.
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetChange : K09801120_Sec_メインストレージ暗号化設定変更／解除.
//	概要	：	中止キー（拡大）押下.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetChange::procReturn( uchar uc_option )
{
	nextGamen = &gp_AdminEncryptionSetting;		// K09801000_Sec_暗号化設定.
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetChange : K09801120_Sec_メインストレージ暗号化設定変更／解除.
//	概要	：	OKキー押下.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetChange::procOk( uchar uc_option )
{
	procGoToPage( m_uc_KeySelect );
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetChange : K09801120_Sec_メインストレージ暗号化設定変更／解除.
//	概要	：	OKキー押下.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetChange::procButtonOn( uchar uc_option )
{
	switch( uc_option ) {
		case DEF_BTNID_KEY_MainStorageEncryptionSetChange:	// 変更.
			 m_uc_KeySelect = PNLD_AdminMainStorageEncryptionSetChange; setupGamen();
			 break;
		case DEF_BTNID_KEY_MainStorageEncryptionSetRelease:	// 解除.
			 m_uc_KeySelect = PNLD_AdminMainStorageEncryptionSetRelease; setupGamen();
			 break;
		default:											// 親を呼ぶ.
			 PAdminKeyBoardCom::procButtonOn( uc_option );
			 break;
	}
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetChange : K09801120_Sec_メインストレージ暗号化設定変更／解除.
//	概要	：	遷移ボタン.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
//			：	2016/02/11 OPS1-motohashi [SecV2-4] 暗号化ワード登録時は文字列長設定画面への遷移に変更.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetChange::procGoToPage( uchar uc_option )
{
	// キーボードから取得.
	char _sc_Password[TYPD_MainStorageEncryptionPassword_MaxLen+1];
	memset( _sc_Password, 0, sizeof(_sc_Password) );
	getKeyboardData( _sc_Password );
// 2016/04/11 CQ526818 ストレージ暗号化パスワードのハッシュ化対応 Sky大嶋 Start.
	switch ( uc_option ) {
		case PNLD_AdminMainStorageEncryptionSetChange:		// 変更キー.
			if ( PnlComGetSystemSetting()->chkMainStorageEncryptionPassword(_sc_Password) ) {			// 入力した暗号化ワードが合っているか確認.
// 2016/06/01 SCD2-Sky Shindo [SecV2] [CQ530312] 20文字選択時、セキュリティー強化モードを解除する.
//// comment 2016/05/11 SCD2-2G-Sky延原 [SecV2] [CQ528021] 20文字選択時、セキュリティー強化モードを解除する、付随修正.
//				bl_FromUpperGamen = True;				// 下位画面への遷移元画面通知フラグ.
				gp_AdminMainStorageEncStrLenSelect.setFromUpperGamen( True );
				// 現在の暗号化ワードと一致の場合.
				PLBC_CheckFromMenu::getInstance()->setFromMenu( True );
				nextGamen = &gp_AdminMainStorageEncStrLenSelect;		// メインストレージ暗号化ワード文字列長選択.
			} else {
				// 現在の暗号化ワードと不一致の場合.
				PAdminKeyBoardCom::procClear( uc_option );
				m_b_error = True;			// エラーメッセージ表示フラグＯＮ.
				setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定.
				if ( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {	// 文字を表示するため、拡大→標準へ変更.
					procButtonOn( 100 );	// 標準⇔拡大切換えキー押下.
				} else {	// procButtonOnを押した後にsetupGamenを呼ぶと初期化フラグが落ちてしまうため、↑では呼ばない.
					setupGamen();
				}
			}
			break;
		case PNLD_AdminMainStorageEncryptionSetRelease:		// 解除キー.
			if ( PnlComGetSystemSetting()->chkMainStorageEncryptionPassword(_sc_Password) ) {		// 入力したパスワードが合っているか確認.
// 2016/06/01 SCD2-Sky Shindo [SecV2] [CQ530312] 20文字選択時、セキュリティー強化モードを解除する.
//// comment 2016/05/11 SCD2-2G-Sky延原 [SecV2] [CQ528021] 20文字選択時、セキュリティー強化モードを解除する、付随修正.
//				bl_FromUpperGamen = True;				// 下位画面への遷移元画面通知フラグ.
				// 現在の暗号化ワードと一致の場合.
				gp_AdminMainStorageEncryptionSetAct.setMainStorageEncriptionSetting( False, NULL );	// メインストレージ暗号化処理中に解除を設定する.
// comment 2019/3/12 Sky黒田（達） [Eagle][制御構成変更_No.10] SSD装着時のストレージ暗号化機能対応.
//				if( True == PnlComGetSystemSetting()->getSecurityLevelUpOn() ) {
				APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();	// DipSw（API）・インスタンス取得.
				if ( pc_SoftDipSw != NULL ) {

					if ( ( True == PnlComGetSystemSetting()->getSecurityLevelUpOn() ) 
						&& ( False == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW164, 2 ) ) ) {	// DipSw164 bit2無効.
						// セキュリティ強化モード有効時に「解除」を選択している場合.
						setDispInfo( PNLE_AdminMainStorageEncryptionSetChange, this );	// セキュリティーモード解除確認画面で使用する為.
						nextGamen = &gp_AdminSecurityReinforceConfirm;					// セキュリティーモード解除確認.
					} else {
						// セキュリティ強化モード無効時に「解除」を選択している場合.
						nextGamen = &gp_AdminMainStorageEncryptionSetAct;				// メインストレージ暗号化処理中画面. 
					}
				}
			} else {
				// 現在の暗号化ワードと不一致の場合.
				PAdminKeyBoardCom::procClear( uc_option );
				m_b_error = True;			// エラーメッセージ表示フラグＯＮ.
				setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定.
				if ( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {	// 文字を表示するため、拡大→標準へ変更.
					procButtonOn( 100 );	// 標準⇔拡大切換えキー押下.
				} else {	// procButtonOnを押した後にsetupGamenを呼ぶと初期化フラグが落ちてしまうため、↑では呼ばない.
					setupGamen();
				}
			}
			break;
		default:
			setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定.
			break;
	}
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetChange : K09801120_Sec_メインストレージ暗号化設定変更／解除.
//	概要	：	.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetChange::setDefaultBackParts()
{
	if (( PLBD_Enl_KBD_Enlarge == Kbd_ScanKeybordConv.getChangeEnlKeybord() ) && ( True == onlyPanel.b_InitKeybord )) {
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminMainStorageEncryptionSetChangeDefaultEnl , ss_AdminMainStorageEncryptionSetChangeDefaultEnl );	// 拡大用デフォルト背景.
	} else {
		setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminMainStorageEncryptionSetChangeDefault, ss_AdminMainStorageEncryptionSetChangeDefault );	// 標準用デフォルト背景.
	}
}

// 2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetAct : K09801130_Sec_メインストレージ暗号化処理中.
//	概要	：	コンストラクタ.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
PAdminMainStorageEncryptionSetAct::PAdminMainStorageEncryptionSetAct( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen )
	: PAdminInvalidKeyGamencom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen )
	, m_b_MainStorageEncryption(False)
{
	memset( m_sc_MainStorageEncryptionPassword, 0, sizeof(m_sc_MainStorageEncryptionPassword) );
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetAct : K09801130_Sec_メインストレージ暗号化処理中.
//	概要	：	デストラクタ.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
PAdminMainStorageEncryptionSetAct::~PAdminMainStorageEncryptionSetAct()
{
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetAct : K09801130_Sec_メインストレージ暗号化処理中.
//	概要	：	画面開始処理.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetAct::startGamen()
{
	PAdminInvalidKeyGamencom::startGamen();
	APIC_Power* pc_Power = APIC_Power::newInstance();
	if( NULL == pc_Power ){			// NULLチェックを追加.
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//		LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
//		if ( NULL != pc_BFW_System ) {
//	   		pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900650010B56201);
//		}
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, 3, "Assert Failed!" );
		return;
	}
	pc_Power->setPowerSaveDelay(__FILE__, __LINE__);	// 節電モード移行不可を設定.
	PanelLog( PNLD_LogKind_Admin, "ans_EncryptMainStorageAction() P1[%d] P2[%s] P3[%d] P4[%s]", m_b_MainStorageEncryption, m_sc_MainStorageEncryptionPassword, sizeof(m_sc_MainStorageEncryptionPassword), "ans_EncryptMainStorageAction", 0, 0 );
// 2016/09/16 SCD2-Sky 宮下 req_encryptMainStorage関数の引数のパスワード長を正しいものにした.
//	if(True != PnlComGetSystemSetting()->req_encryptMainStorage( m_b_MainStorageEncryption, m_sc_MainStorageEncryptionPassword, sizeof(m_sc_MainStorageEncryptionPassword), ans_EncryptMainStorageAction ) ){
	if(True != PnlComGetSystemSetting()->req_encryptMainStorage( m_b_MainStorageEncryption, m_sc_MainStorageEncryptionPassword, TYPD_MainStorageEncryptionPassword_MaxLen, ans_EncryptMainStorageAction ) ){
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, 3, "Assert Failed!" );
		LOGF_debug_PutEventLog(LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!");
		nextGamen = &gp_AdminEncryptionSetting;			// 暗号化設定画面.
	}
	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key);
}

// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetAct : K09801130_Sec_メインストレージ暗号化処理中.
//	概要	：	画面描画更新処理.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetAct::setupGamen( short ss_Option )
{
	Prg_AdminMainStorageEcnryptSetActNowActionIcon.animateParts();		// プログレスバーのアニメーション.

	PAdminInvalidKeyGamencom::setupGamen( ss_Option );
}

// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetAct : K09801130_Sec_メインストレージ暗号化処理中.
//	概要	：	画面終了処理.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetAct::endGamen()
{
	APIC_Power* pc_Power = APIC_Power::newInstance();
	if( NULL == pc_Power ){			// NULLチェックを追加.
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//		LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
//		if ( NULL != pc_BFW_System ) {
//	   		pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900670010B56301);
//		}
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, 3, "Assert Failed!" );
		return;
	}
	pc_Power->resetPowerSaveDelay(__FILE__, __LINE__);	// 節電モード移行不可を解除.
	PAdminInvalidKeyGamencom::endGamen();
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetAct : K09801130_Sec_メインストレージ暗号化処理中.
//	概要	：	暗号化設定.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetAct::setMainStorageEncriptionSetting(Bool b_Encryption , const char* sc_EncryptionPassword)
{
	m_b_MainStorageEncryption = b_Encryption;					// 暗号化（する/しない）.
	memset( m_sc_MainStorageEncryptionPassword, 0, sizeof(m_sc_MainStorageEncryptionPassword));	
	if ( NULL != sc_EncryptionPassword){
// comment 2018/02/23【HCLT-Vishal】【IT6_Eagle】 Klockwork Changes ID:1674086【SV.STRBO.BOUND_COPY.UNTERM 】. 
//		strncpy( m_sc_MainStorageEncryptionPassword, sc_EncryptionPassword, TYPD_MainStorageEncryptionPassword_MaxLen+1);	// 暗号化パスワード.
		strncpy( m_sc_MainStorageEncryptionPassword, sc_EncryptionPassword, ( TYPD_MainStorageEncryptionPassword_MaxLen ));	// 暗号化パスワード.
	}
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetAct : K09801130_Sec_メインストレージ暗号化処理中.
//	概要	：	コールバックイベント処理.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetAct::ans_EncryptMainStorageAction(int32_t sl_option)
{
	PanelLog( PNLD_LogKind_Admin, "ans_EncryptMainStorageAction() P1[%d] ", sl_option, 0, 0, 0, 0, 0 );
	if( APID_DocapiMethodOk == sl_option ){
		CallBackIndi(TYPE_PNL_CallBack_EncryptMainStorageActionOk);
	}else {
		CallBackIndi(TYPE_PNL_CallBack_EncryptMainStorageActionNg);
	}
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetAct : K09801130_Sec_メインストレージ暗号化処理中.
//	概要	：	コールバックイベント処理.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetAct::procCallBack( TYPE_PNL_CallBack e_type )
{
	switch( e_type ) {
		case TYPE_PNL_CallBack_EncryptMainStorageActionOk:		// 暗号化処理成功.
			nextGamen = &gp_AdminMainStorageEncryptionSetCompletion;	// メインストレージ暗号化ﾜｰﾄﾞ設定完了画面.
			break;
		case TYPE_PNL_CallBack_EncryptMainStorageActionNg:		// 暗号化処理失敗.
			nextGamen = &gp_AdminEncryptionSetting;				// 暗号化設定画面.
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, 3, "Assert Failed!" );
			break;
	}
}
//================================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetAct : K09801130_Sec_メインストレージ暗号化処理中.
//	概要	：	デフォルト背景表示部品の設定処理.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/15 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
//================================================================================================.
void PAdminMainStorageEncryptionSetAct::setDefaultBackParts()
{
	setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminMainStorageEncryptionSetActDefault, ss_AdminMainStorageEncryptionSetActDefault );
}

// 2015/09/30 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetCompletion : K09801140_Sec_メインストレージ暗号化ワード設定完了.
//	概要	：	コンストラクタ.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/30 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
PAdminMainStorageEncryptionSetCompletion::PAdminMainStorageEncryptionSetCompletion( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen )
	: PAdminInvalidKeyGamencom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen )
{
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetCompletion : K09801140_Sec_メインストレージ暗号化ワード設定完了.
//	概要	：	デストラクタ.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/30 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
PAdminMainStorageEncryptionSetCompletion::~PAdminMainStorageEncryptionSetCompletion()
{
}
// ===============================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetCompletion : K09801140_Sec_メインストレージ暗号化ワード設定完了.
//	概要	：	画面開始処理.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/30 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
// ===============================================================================================.
void PAdminMainStorageEncryptionSetCompletion::startGamen()
{
	onlyPanel.bl_PowerOffOnMesage = True;	// 主電源OFF/ON画面から遷移させないためのフラグ.
	turnGamen = &gp_AdminMainStorageEncryptionSetCompletion;
	PAdminInvalidKeyGamencom::startGamen();
	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key);
}
//================================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetCompletion : K09801140_Sec_メインストレージ暗号化ワード設定完了.
//	概要	：	1秒タイマ.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/30 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
//================================================================================================.
void PAdminMainStorageEncryptionSetCompletion::procSecond( uchar option )
{
	ppc_Power->changePowerMode( TYPE_PowerModeEtsKeyOperation );		// その他キー操作を通知する.
}
//================================================================================================.
//	クラス	：	PAdminMainStorageEncryptionSetCompletion : K09801140_Sec_メインストレージ暗号化ワード設定完了.
//	概要	：	デフォルト背景表示部品の設定処理.
//	戻り値	：	.
//	備考	：	.
//	変更履歴：	2015/09/30 SecV1-4 【SSD/eMMCのデータ暗号化復号化技術】 Sky 新道.
//================================================================================================.
void PAdminMainStorageEncryptionSetCompletion::setDefaultBackParts()
{
	setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminMainStorageEncryptionSetCompletionDefault, ss_AdminMainStorageEncryptionSetCompletionDefault );
}

//================================================================================================.
// 概要		:	PAdminStorageEncStrLenSelectCom	:	暗号化ワード長選択共通クラス	：コンストラクタ.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
// 			:	2016/06/01 SCD2-Sky Shindo [SecV2] [CQ530312] 20文字選択時、セキュリティー強化モードを解除する.
//================================================================================================.
PAdminStorageEncStrLenSelectCom::PAdminStorageEncStrLenSelectCom( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen )
	: PAdminToSecuritySetcom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen )
	, m_b_FromUpperGamen( False )
{
}

//================================================================================================.
// 概要		:	PAdminStorageEncStrLenSelectCom	:	暗号化ワード長選択共通クラス	：デストラクタ.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
//================================================================================================.
PAdminStorageEncStrLenSelectCom::~PAdminStorageEncStrLenSelectCom()
{
}

//================================================================================================.
// 概要		:	PAdminStorageEncStrLenSelectCom	:	暗号化ワード長選択共通クラス	：画面開始処理.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
//================================================================================================.
void PAdminStorageEncStrLenSelectCom::startGamen()
{
	PAdminToSecuritySetcom::startGamen();

	Key_UtilityCancel.shiftPosition( PNLD_AdminStorageEncStrLenSel_ShiftCancelKey, 0 );
}

//================================================================================================.
// 概要		:	PAdminStorageEncStrLenSelectCom	:	暗号化ワード長選択共通クラス	：画面開始処理.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
//================================================================================================.
void PAdminStorageEncStrLenSelectCom::endGamen()
{
	Key_UtilityCancel.shiftPosition( -PNLD_AdminStorageEncStrLenSel_ShiftCancelKey, 0 );

	PAdminToSecuritySetcom::endGamen();
}
//================================================================================================.
// 概要		:	PAdminStorageEncStrLenSelectCom	:	暗号化ワード長選択共通クラス	：遷移元画面取得.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016/06/01 SCD2-Sky Shindo [SecV2] [CQ530312] 20文字選択時、セキュリティー強化モードを解除する.
//================================================================================================.
Bool PAdminStorageEncStrLenSelectCom::getFromUpperGamen()
{
	return m_b_FromUpperGamen;
}
//================================================================================================.
// 概要		:	PAdminStorageEncStrLenSelectCom	:	暗号化ワード長選択共通クラス	：遷移元画面設定.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016/06/01 SCD2-Sky Shindo [SecV2] [CQ530312] 20文字選択時、セキュリティー強化モードを解除する.
//================================================================================================.
void PAdminStorageEncStrLenSelectCom::setFromUpperGamen( Bool b_FromUpperGamen )
{
	m_b_FromUpperGamen = b_FromUpperGamen;
}

//================================================================================================.
// 概要		:	PAdminMainStorageEncStrLenSelect	:	メインストレージ暗号化ワード長選択	：コンストラクタ.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016/02/11 OPS1-motohashi [SecV2-4] ストレージ暗号化ワード長選択画面追加.
// 				20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
//================================================================================================.
PAdminMainStorageEncStrLenSelect::PAdminMainStorageEncStrLenSelect( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen )
  : PAdminStorageEncStrLenSelectCom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen )
	, m_ss_SelectKey( PNLD_AdminStorageEncStrLenSel_Ascii20Char )
	, m_bl_EnableEncrypt( False )
{
}

//================================================================================================.
// 概要		:	PAdminMainStorageEncStrLenSelect	:	メインストレージ暗号化ワード長選択	：デストラクタ.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016/02/11 OPS1-motohashi [SecV2-4] ストレージ暗号化ワード長選択画面追加.
//================================================================================================.
PAdminMainStorageEncStrLenSelect::~PAdminMainStorageEncStrLenSelect()
{
}

//================================================================================================.
// 概要		:	PAdminMainStorageEncStrLenSelect	:	メインストレージ暗号化ワード長選択	：画面開始処理.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016/02/11 OPS1-motohashi [SecV2-4] ストレージ暗号化ワード長選択画面追加.
// 			20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
//================================================================================================.
void PAdminMainStorageEncStrLenSelect::startGamen()
{
	PAdminStorageEncStrLenSelectCom::startGamen();

	if( True == PLBC_CheckFromMenu::getInstance()->getFromMenu() ) {
		m_ss_SelectKey = PNLD_AdminStorageEncStrLenSel_Ascii20Char;
		m_bl_EnableEncrypt = PnlComGetSystemSetting()->getEnableMainStorageEncryption();
		PLBC_CheckFromMenu::getInstance()->setFromMenu( False );
	}

	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key);
}

//================================================================================================.
// 概要		:	PAdminMainStorageEncStrLenSelect	:	メインストレージ暗号化ワード長選択	：画面更新処理.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016/02/11 OPS1-motohashi [SecV2-4] ストレージ暗号化ワード長選択画面追加.
/// 			20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
//================================================================================================.
void PAdminMainStorageEncStrLenSelect::setupGamen( short ss_Option )
{
	Rbt_AdminStorageEncStrLenSelect.select( m_ss_SelectKey );

	if( True == m_bl_EnableEncrypt ) {	// 暗号設定が有効の場合は変更時.
		Txt_AdminStorageEncStrLenSelect.changeText( s_pMSGAdminStorageEncPassLenSelectChange_W624H048I );
	} else {							// 暗号化設定が無効の場合は新規時.
		Txt_AdminStorageEncStrLenSelect.changeText( s_pMSGAdminStorageEncPassLenSelectNew_W624H048I );
	}

	PAdminStorageEncStrLenSelectCom::setupGamen( ss_Option );
}

//================================================================================================.
// 概要		:	PAdminMainStorageEncStrLenSelect	:	メインストレージ暗号化ワード長選択	：ボタン押下.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016/02/11 OPS1-motohashi [SecV2-4] ストレージ暗号化ワード長選択画面追加.
//				20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
//================================================================================================.
void PAdminMainStorageEncStrLenSelect::procButtonOn( uchar uc_Option )
{
	switch( uc_Option ) {
		case PNLD_BTNID_KEY_AdminStorageEncStrLenSelectAscii20Char:
			m_ss_SelectKey = PNLD_AdminStorageEncStrLenSel_Ascii20Char;
			procGoToPage( uc_Option );
			break;
		case PNLD_BTNID_KEY_AdminStorageEncStrLenSelectAscii64Char:
			m_ss_SelectKey = PNLD_AdminStorageEncStrLenSel_Ascii64Char;
			procGoToPage( uc_Option );
			break;
		default:
			// 何もしない.
			break;
	}

	setupGamen();
}

////================================================================================================.
//// 概要		:	PAdminMainStorageEncStrLenSelect	:	メインストレージ暗号化ワード長選択	：OKボタン押下.
//// 戻り値	:	なし.
//// 備考		:	.
//// 変更履歴	:	2016/02/11 OPS1-motohashi [SecV2-4] ストレージ暗号化ワード長選択画面追加.
////================================================================================================.
//void PAdminMainStorageEncStrLenSelect::procOk( uchar uc_Option )
//{
//	short	ss_MaxStrLen = 0;
//
//	// 基本的にキーはどちらかが選択されている状態としている為.
//	// defaultケースは初期値の20文字選択とみなす.
//	switch( m_uc_SelectKey ) {
//		case PNLD_AdminStorageEncStrLenSel_Ascii64Char:
//			ss_MaxStrLen = PNLD_AdminStorageEncStrLenSel_MaxLen_Ascii64Char;
//			break;
//
//		case PNLD_AdminStorageEncStrLenSel_Ascii20Char:
//		default:
//			ss_MaxStrLen = PNLD_AdminStorageEncStrLenSel_MaxLen_Ascii20Char;
//			break;
//	}
//
//	gp_AdminMainStorageEncryptionSetEntry.setMaxStrLen( ss_MaxStrLen );
//	nextGamen = &gp_AdminMainStorageEncryptionSetEntry;
//}
//================================================================================================.
// 概要		:	PAdminMainStorageEncStrLenSelect	:	メインストレージ暗号化ワード長選択	：遷移ボタン押下.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	 20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
//================================================================================================.
void PAdminMainStorageEncStrLenSelect::procGoToPage( uchar uc_Option )
{
	short	ss_MaxStrLen = 0;

// comment 2016/05/11 SCD2-2G-Sky延原 [SecV2] [CQ528021] 20文字選択時、セキュリティー強化モードを解除する.
	APIC_SystemSetting* pc_SystemSetting = APIC_SystemSetting::newInstance();
	if ( NULL == pc_SystemSetting ) {
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//		LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
//		if ( NULL != pc_BFW_System ) {
//	   		pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900020010B56401);
//		}
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, 3, "Assert Failed!" );
		return;
	}

	switch( uc_Option ) {
		case PNLD_BTNID_KEY_AdminStorageEncStrLenSelectAscii20Char:
			ss_MaxStrLen = PNLD_AdminStorageEncStrLenSel_MaxLen_Ascii20Char;
			break;
		case PNLD_BTNID_KEY_AdminStorageEncStrLenSelectAscii64Char:
			ss_MaxStrLen = PNLD_AdminStorageEncStrLenSel_MaxLen_Ascii64Char;
			break;
		default:
			// 何もしない.
			break;
	}

// comment 2016/05/11 SCD2-2G-Sky延原 [SecV2] [CQ528021] 20文字選択時、セキュリティー強化モードを解除する.
	// セキュリティ強化モードONで20文字キー選択時、セキュリティ強化モード解除確認画面に遷移する.
//	gp_AdminMainStorageEncryptionSetEntry.setMaxStrLen( ss_MaxStrLen );
//	nextGamen = &gp_AdminMainStorageEncryptionSetEntry;
	gp_AdminMainStorageEncryptionSetEntry.setMaxStrLen( ss_MaxStrLen );
// comment 2019/3/12 Sky黒田（達） [Eagle][制御構成変更_No.10] SSD装着時のストレージ暗号化機能対応.
//	if ( True == pc_SystemSetting->getSecurityLevelUpOn()
//		&& PNLD_BTNID_KEY_AdminStorageEncStrLenSelectAscii20Char == uc_Option) {
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();	// DipSw（API）・インスタンス取得.
	if ( pc_SoftDipSw != NULL ) {
		if ( ( True == pc_SystemSetting->getSecurityLevelUpOn() )
			&& ( False == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW164, 2 )	)		// DipSW164 2bit目無効時.
			&& ( PNLD_BTNID_KEY_AdminStorageEncStrLenSelectAscii20Char == uc_Option ) ) {
			setDispInfo( PNLE_AdminMainStorageEncStrLenSelect, this );		// セキュリティ強化モード解除確認画面で使用する為に画面情報をセット.
			nextGamen = &gp_AdminSecurityReinforceConfirm;					// セキュリティ強化モード解除確認.
		}
		else {
			nextGamen = &gp_AdminMainStorageEncryptionSetEntry;
		}
	}
}

//================================================================================================.
// 概要		:	PAdminMainStorageEncStrLenSelect	:	メインストレージ暗号化ワード長選択	：中止ボタン押下.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
//================================================================================================.
void PAdminMainStorageEncStrLenSelect::procCancel( uchar uc_Option )
{
// 2016/06/01 SCD2-Sky Shindo [SecV2] [CQ530312] 20文字選択時、セキュリティー強化モードを解除する.
//// comment 2016/05/11 SCD2-2G-Sky延原 [SecV2] [CQ528021] 20文字選択時、セキュリティー強化モードを解除する、付随修正.
	// K09801120_Sec_メインストレージ暗号化設定変更／解除からの遷移時対応追加.
//	nextGamen = &gp_AdminEncryptionSetting;
//	if (True == gp_AdminMainStorageEncryptionSetChange.bl_FromUpperGamen) {
	if ( True == PAdminStorageEncStrLenSelectCom::getFromUpperGamen() ) {
		nextGamen = &gp_AdminMainStorageEncryptionSetChange;		// K09801120_Sec_メインストレージ暗号化設定変更／解除.
	}
	else {
		nextGamen = &gp_AdminEncryptionSetting;						// K09801000_Sec_暗号化設定からの遷移時.
	}
	

}

//================================================================================================.
// 概要		:	PAdminMainStorageEncStrLenSelect	:	メインストレージ暗号化ワード長選択	：デフォルト背景表示部品の設定処理.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016/02/11 OPS1-motohashi [SecV2-4] ストレージ暗号化ワード長選択画面追加.
//================================================================================================.
void PAdminMainStorageEncStrLenSelect::setDefaultBackParts()
{
	setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminMainStorageEncStrLenSelectDefault, ss_AdminMainStorageEncStrLenSelectDefault );
}

//================================================================================================.
// 概要		:	PAdminSubStorageEncStrLenSelect	:	サブストレージ暗号化ワード長選択	：コンストラクタ.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016/02/11 OPS1-motohashi [SecV2-4] ストレージ暗号化ワード長選択画面追加.
/// 			20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
//================================================================================================.
PAdminSubStorageEncStrLenSelect::PAdminSubStorageEncStrLenSelect( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen )
  : PAdminStorageEncStrLenSelectCom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen )
	, m_ss_SelectKey( PNLD_AdminStorageEncStrLenSel_Ascii20Char )
	, m_bl_EnableEncrypt( False )
{
}

//================================================================================================.
// 概要		:	PAdminSubStorageEncStrLenSelect	:	サブストレージ暗号化ワード長選択	：デストラクタ.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016/02/11 OPS1-motohashi [SecV2-4] ストレージ暗号化ワード長選択画面追加.
//================================================================================================.
PAdminSubStorageEncStrLenSelect::~PAdminSubStorageEncStrLenSelect()
{
}

//================================================================================================.
// 概要		:	PAdminSubStorageEncStrLenSelect	:	サブストレージ暗号化ワード長選択	：画面開始処理.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016/02/11 OPS1-motohashi [SecV2-4] ストレージ暗号化ワード長選択画面追加.
/// 			20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
//================================================================================================.
void PAdminSubStorageEncStrLenSelect::startGamen()
{
	PAdminStorageEncStrLenSelectCom::startGamen();

	if( True == PLBC_CheckFromMenu::getInstance()->getFromMenu() ) {
		m_ss_SelectKey = PNLD_AdminStorageEncStrLenSel_Ascii20Char;
		m_bl_EnableEncrypt = PnlComGetSystemSetting()->getEnableEncryption();
		PLBC_CheckFromMenu::getInstance()->setFromMenu( False );
	}

	// 左エリア種類の有効設定（ジョブ表示（通常））startGamen()の最後でコールしてください.
	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key);
}

//================================================================================================.
// 概要		:	PAdminSubStorageEncStrLenSelect	:	サブストレージ暗号化ワード長選択	：画面更新処理.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016/02/11 OPS1-motohashi [SecV2-4] ストレージ暗号化ワード長選択画面追加.
/// 			20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
//================================================================================================.
void PAdminSubStorageEncStrLenSelect::setupGamen( short ss_Option )
{
	Rbt_AdminStorageEncStrLenSelect.select( m_ss_SelectKey );

	if( True == m_bl_EnableEncrypt ) {	// 暗号設定が有効の場合は変更時.
		Txt_AdminStorageEncStrLenSelect.changeText( s_pMSGAdminStorageEncPassLenSelectChange_W624H048I );
	} else {							// 暗号化設定が無効の場合は新規時.
		Txt_AdminStorageEncStrLenSelect.changeText( s_pMSGAdminStorageEncPassLenSelectNew_W624H048I );
	}

	PAdminToSecuritySetcom::setupGamen( ss_Option );
}

//================================================================================================.
// 概要		:	PAdminSubStorageEncStrLenSelect	:	サブストレージ暗号化ワード長選択	：ボタン押下.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016/02/11 OPS1-motohashi [SecV2-4] ストレージ暗号化ワード長選択画面追加.
// 				20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
//================================================================================================.
void PAdminSubStorageEncStrLenSelect::procButtonOn( uchar uc_Option )
{
	switch( uc_Option ) {
		case PNLD_BTNID_KEY_AdminStorageEncStrLenSelectAscii20Char:
			m_ss_SelectKey = PNLD_AdminStorageEncStrLenSel_Ascii20Char;
			procGoToPage( uc_Option );
			break;
		case PNLD_BTNID_KEY_AdminStorageEncStrLenSelectAscii64Char:
			m_ss_SelectKey = PNLD_AdminStorageEncStrLenSel_Ascii64Char;
			procGoToPage( uc_Option );
			break;
		default:
			// 何もしない.
			break;
	}

	setupGamen();
}

// 20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
////================================================================================================.
//// 概要		:	PAdminSubStorageEncStrLenSelect	:	サブストレージ暗号化ワード長選択	：OKボタン押下.
//// 戻り値	:	なし.
//// 備考		:	.
//// 変更履歴	:	2016/02/11 OPS1-motohashi [SecV2-4] ストレージ暗号化ワード長選択画面追加.
////================================================================================================.
//void PAdminSubStorageEncStrLenSelect::procOk( uchar uc_Option )
//{
//	short	ss_MaxStrLen = 0;
//
//	// 基本的にキーはどちらかが選択されている状態としている為.
//	// defaultケースは初期値の20文字選択とみなす.
//	switch( m_uc_SelectKey ) {
//		case PNLD_AdminStorageEncStrLenSel_Ascii64Char:
//			ss_MaxStrLen = PNLD_AdminStorageEncStrLenSel_MaxLen_Ascii64Char;
//			break;
//
//		case PNLD_AdminStorageEncStrLenSel_Ascii20Char:
//		default:
//			ss_MaxStrLen = PNLD_AdminStorageEncStrLenSel_MaxLen_Ascii20Char;
//			break;
//	}
//
//	gp_AdminHDDCodingEntry.setMaxStrLen( ss_MaxStrLen );
//	nextGamen = &gp_AdminHDDCodingEntry;
//}
//================================================================================================.
// 概要		:	PAdminSubStorageEncStrLenSelect	:	サブストレージ暗号化ワード長選択	：遷移ボタン押下.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
//================================================================================================.
void PAdminSubStorageEncStrLenSelect::procGoToPage( uchar uc_Option )
{
	short	ss_MaxStrLen = 0;

// 2016/06/01 SCD2-Sky Shindo [SecV2] [CQ530312] 20文字選択時、セキュリティー強化モードを解除する.
	APIC_SystemSetting* pc_SystemSetting = APIC_SystemSetting::newInstance();
	if ( NULL == pc_SystemSetting ) {
//Comment 2019/06/14 [HCLT_Monisha][EagleH][N Rank] Removed unwanted Detailed code.
//		LAPC_BFW_System *pc_BFW_System=LAPC_BFW_System::newInstance();
//		if ( NULL != pc_BFW_System ) {
//	   		pc_BFW_System->registDetailCode(PNLD_DetailCode_Admin_0x1900020010B56501);
//		}
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, 3, "Assert Failed!" );
		return;
	}

	switch( uc_Option ) {
		case PNLD_BTNID_KEY_AdminStorageEncStrLenSelectAscii20Char:
			ss_MaxStrLen = PNLD_AdminStorageEncStrLenSel_MaxLen_Ascii20Char;
			break;
		case PNLD_BTNID_KEY_AdminStorageEncStrLenSelectAscii64Char:
			ss_MaxStrLen = PNLD_AdminStorageEncStrLenSel_MaxLen_Ascii64Char;
			break;
		default:
			// 何もしない.
			break;
	}

	gp_AdminHDDCodingEntry.setMaxStrLen( ss_MaxStrLen );
// 2016/06/01 SCD2-Sky Shindo [SecV2] [CQ530312] 20文字選択時、セキュリティー強化モードを解除する.
//	nextGamen = &gp_AdminHDDCodingEntry;
	// セキュリティ強化モードONで20文字キー選択時、セキュリティ強化モード解除確認画面に遷移する.
	if ( True == pc_SystemSetting->getSecurityLevelUpOn()
		&& PNLD_BTNID_KEY_AdminStorageEncStrLenSelectAscii20Char == uc_Option) {
		setDispInfo( PNLE_AdminSubStorageEncStrLenSelect, this );		// セキュリティ強化モード解除確認画面で使用する為に画面情報をセット.
		nextGamen = &gp_AdminSecurityReinforceConfirm;					// セキュリティ強化モード解除確認.
	}
	else {
		nextGamen = &gp_AdminHDDCodingEntry;
	}
}

//================================================================================================.
// 概要		:	PAdminSubStorageEncStrLenSelect	:	サブストレージ暗号化ワード長選択	：中止ボタン押下.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	20160419 motohashi [CQ.527580] 暗号化ワード入力長選択の操作性改善.
//================================================================================================.
void PAdminSubStorageEncStrLenSelect::procCancel( uchar uc_Option )
{
// 2016/06/01 SCD2-Sky Shindo [SecV2] [CQ530312] 20文字選択時、セキュリティー強化モードを解除する.
//	nextGamen = &gp_AdminEncryptionSetting;
	if (True == PAdminStorageEncStrLenSelectCom::getFromUpperGamen()) {
		nextGamen = &gp_AdminHDDCodingWordSetting;					// 10980750_HDD暗号化設定変更／解除.
	}
	else {
		nextGamen = &gp_AdminEncryptionSetting;						// K09801000_Sec_暗号化設定からの遷移時.
	}
}

//================================================================================================.
// 概要		:	PAdminSubStorageEncStrLenSelect	:	サブストレージ暗号化ワード長選択	：デフォルト背景表示部品の設定処理.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016/02/11 OPS1-motohashi [SecV2-4] ストレージ暗号化ワード長選択画面追加.
//================================================================================================.
void PAdminSubStorageEncStrLenSelect::setDefaultBackParts()
{
	setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AdminSubStorageEncStrLenSelectDefault, ss_AdminSubStorageEncStrLenSelectDefault );

}

//================================================================================================.
// 概要		:	PAdminHDDCodingWordStrLenSelect	:	再設定暗号化ワード長選択	：コンストラクタ.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016.04.28 OPS1-Sky西原 [セキュリティ技術v2][CQ527851].
//================================================================================================.
PAdminHDDCodingWordStrLenSelect::PAdminHDDCodingWordStrLenSelect( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen )
	: PAdminStorageEncStrLenSelectCom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen )
	, m_uc_SelectKey( PNLD_AdminStorageEncStrLenSel_Ascii20Char )
{
}

//================================================================================================.
// 概要		:	PAdminHDDCodingWordStrLenSelect	:	再設定暗号化ワード長選択	：画面開始処理.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016.04.28 OPS1-Sky西原 [セキュリティ技術v2][CQ527851].
//================================================================================================.
void PAdminHDDCodingWordStrLenSelect::startGamen()
{
	PAdminStorageEncStrLenSelectCom::startGamen();

	if( True == PLBC_CheckFromMenu::getInstance()->getFromMenu() ) {
		m_uc_SelectKey = PNLD_AdminStorageEncStrLenSel_Ascii20Char;
		PLBC_CheckFromMenu::getInstance()->setFromMenu( False );
	}

	setLeftAreaEnableType( TYPD_LeftAreaBookmark | TYPD_LeftAreaSoft10Key);
}

//================================================================================================.
// 概要		:	PAdminHDDCodingWordStrLenSelect	:	再設定暗号化ワード長選択	：画面更新処理.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016.04.28 OPS1-Sky西原 [セキュリティ技術v2][CQ527851].
//================================================================================================.
void PAdminHDDCodingWordStrLenSelect::setupGamen( short ss_Option )
{
	Rbt_AdminStorageEncStrLenSelect.select( m_uc_SelectKey );

	Txt_AdminStorageEncStrLenSelect.changeText( s_pMSGAdminStorageEncPassLenSelectChange_W624H048I );

	PAdminStorageEncStrLenSelectCom::setupGamen( ss_Option );
}
//================================================================================================.
// 概要		:	PAdminHDDCodingWordStrLenSelect	:	再設定暗号化ワード長選択	：ボタン押下.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016.04.28 OPS1-Sky西原 [セキュリティ技術v2][CQ527851].
//================================================================================================.
void PAdminHDDCodingWordStrLenSelect::procButtonOn( uchar uc_Option )
{
	switch( uc_Option ) {
		case PNLD_BTNID_KEY_AdminStorageEncStrLenSelectAscii20Char:
			m_uc_SelectKey = PNLD_AdminStorageEncStrLenSel_Ascii20Char;
			procGoToPage( uc_Option );
			break;
		case PNLD_BTNID_KEY_AdminStorageEncStrLenSelectAscii64Char:
			m_uc_SelectKey = PNLD_AdminStorageEncStrLenSel_Ascii64Char;
			procGoToPage( uc_Option );
			break;
		default:
			// 何もしない.
			break;
	}

	setupGamen();
}
//================================================================================================.
// 概要		:	PAdminHDDCodingWordStrLenSelect	:	再設定暗号化ワード長選択	：遷移ボタン押下.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016.04.28 OPS1-Sky西原 [セキュリティ技術v2][CQ527851].
//================================================================================================.
void PAdminHDDCodingWordStrLenSelect::procGoToPage( uchar uc_Option )
{
	short	ss_MaxStrLen = PNLD_AdminStorageEncStrLenSel_MaxLen_Ascii20Char;

	switch( m_uc_SelectKey ) {
		case PNLD_AdminStorageEncStrLenSel_Ascii20Char:
			ss_MaxStrLen = PNLD_AdminStorageEncStrLenSel_MaxLen_Ascii20Char;
			break;
		case PNLD_AdminStorageEncStrLenSel_Ascii64Char:
			ss_MaxStrLen = PNLD_AdminStorageEncStrLenSel_MaxLen_Ascii64Char;
			break;
		default:
			break;
	}

	gp_AdminHDDCodingWordResetting.setMaxStrLen( ss_MaxStrLen );
	nextGamen = &gp_AdminHDDCodingWordResetting;
}
//================================================================================================.
// 概要		:	PAdminHDDCodingWordStrLenSelect	:	再設定暗号化ワード長選択	：中止ボタン押下.
// 戻り値	:	なし.
// 備考		:	.
// 変更履歴	:	2016.04.28 OPS1-Sky西原 [セキュリティ技術v2][CQ527851].
//================================================================================================.
void PAdminHDDCodingWordStrLenSelect::procCancel( uchar uc_Option )
{
	nextGamen = &gp_AdminFailureHDDCodingWord;
}

// 2016/08/25 CQ533947 暗号化パスワード不一致時しおり無効化 Sky大嶋 Start.
//================================================================================================.
// 概要		:HDDフォーマット : しおりデータ取得処理 .
// 戻り値	:なし .
// 備考		:.
// 2016/08/25 OPS1-Sky Ohshima 暗号化不一致エラー時の「しおり」無効化.
//================================================================================================.
void PAdminHDDCodingWordStrLenSelect::procBookMark( uchar uc_option )
{
	if( bl_HddEncryptionKeyNGAdminMove ) {	// 暗号化不一致エラー発生時.
		if( 0 == uc_option ) {	// 設定メニューを押されたときは遷移.
			PAdminStorageEncStrLenSelectCom::procBookMark( uc_option );
		} else {	// それ以外は利かせない.
			setBuzzerType( PCGE_BuzzerType_InputNg );	// 報知音設定
		}
	} else {	// 通常時はしおりの制限なし.
		PAdminStorageEncStrLenSelectCom::procBookMark( uc_option );
	}
}
// 2016/08/25 CQ533947 暗号化パスワード不一致時しおり無効化 Sky大嶋 End.
