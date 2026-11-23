// =========================================================================== .
// ファイル名：OAP_TRN_deviceservice.cpp.
// 概要		 ：サービストランザクション.
// 変更履歴　：2012/02/10 T.Shima 		：New for ZeusMLK【MA】UV-01223.
// =========================================================================== .
#ifndef LINUX
#pragma warning( disable : 4786 )
#endif
// =========================================================================== .
//  インクルードファイル .
// =========================================================================== .
#include "OAP_COM_error.h"
#include "OAP_COM_dbgtrace.h"
#include "oap/src/client_service/OAP_SVC_transact.h"
#include "oap/src/client_service/OAP_SVC_templates.h"
#include "oap/src/client_service/OAP_SVC_library.h"
#include "oap/src/client_service/OAP_TRN_macro.h"
#include "OAP_COM_InternalApi.h"
#include "OAP_TRN_utility.h"
#include "../OAP_CNV_structconv.h" //IT5_4.1 1015066 Sahana 2017/04/11 Added.
#include "../OAP_CNV_fixedtextconv.h"	// [IT6_2.2MR2] 制御構成変更_No.34  2022/01/18 FPT-QuanPH5 Added.

// FWリファクタリング APIC_Setting.hの分割.
#include "system/nvd/APIC_Setting_MachineSetting.h"
#include "system/nvd/APIC_Setting_SystemSetting.h"
#include "system/nvd/APIC_Setting_OperationSetting.h"
#include "system/nvd/APIC_Setting_CopyScanJobSetting.h"
#include "system/nvd/APIC_Setting_PrintJobSetting.h"
#include "system/nvd/APIC_Setting_FaxJobSetting.h"
#include "system/nvd/APIC_Setting_Other.h"
// IT5_3.3 制御構成変更_No.16 HCL-Sahana 2016/08/02 Added Start.
#include "system/nvd/APIC_Setting_Network.h"
#include "system/nvd/APIC_Setting_TcpIpSetting.h"
#include "system/nvd/APIC_Setting_MailSendSetting.h"
#include "system/nvd/APIC_Setting_PswcMibSetting.h"
#include "system/nvd/APIC_Setting_UbiquitousSetting.h"
#include "system/nvd/APIC_Setting_IPv6Setting.h"
#include "system/nvd/APIC_Setting_SoftDipSw.h"
#include "system/thr/thrInclude/THRC_Threadif.h"	// [IT6_2.2MR2] 制御構成変更_No.34  2022/01/18 FPT-QuanPH5 Added.
// IT5_3.3 制御構成変更_No.16 HCL-Sahana 2016/08/02 Added End.
//2017/10/17 _XML基盤技術開発v2 pactera add Start.
#include "../OAP_CNV_structconv.h"
//2017/10/17 _XML基盤技術開発v2 pactera add End.
// [IT6_2.2MR2] 制御構成変更_No.34  2022/01/18 FPT-QuanPH5 Add Start.
#include "../../../../../divlib/Common/blkapi/blkshm.h"
//#include "../../../../../divlib/client/Proxy/system/sys/APIC_Enabler.h"
#include "system/sys/APIC_Enabler.h"
#include "lib/filelib/SFLG_StorageFileLibraryIf.h"
#include "system/sys/APIC_NetworkDevice.h"		//2022/11/22 [IT6_2.3][制御構成変更_No.52]FPT-HoaPH Added.
#include "system/fax/APIC_FaxDevice.h"	// FPT-VanPN2【IT6_2.3】制御構成変更_No.52 2022/11/10 ADD.
#include "system/sys/SystemInfo/SYSC_SystemInfo.h"	// FPT-VanPN2【IT6_2.3】制御構成変更_No.52 2022/11/10 ADD.
#include "system/sys/APIC_WebDAVTx.h"	// FPT-VanPN2【IT6_2.3】制御構成変更_No.52 2022/11/10 ADD.
using namespace std;

#define OAPD_SIZE_LANGCODE			(8)	
#define OAP_MAX_LANGUAGES	9
#define OAPD_KeyNumMax				49
#define OAPD_SIZE_FILEDATA_SELECT	(3*1024*1024)	

#define OAPD_S32_WORD_SIZE( n )		 ( n + ( n % 2 ))

// NULLで文字列を初期化する.
static char sc_OAP_MessgeBinPath[64] = "";
#define OAPD_OFFSET_LANGCOUNT		(44)
// 2022/11/16 RQ-938 バルト三国対応 Sky Add start.
// comment 2023/2/22 Sky小熊 [IT6_2.3][RQ-970]Welsh as standard language for IT.
//#define OAPD_MAX_LANGTBL		38
//#define OAPD_MAX_LANGTBL		35
#define OAPD_MAX_LANGTBL		39
static unsigned char*	guc_OAP_selDataRAM = NULL;
#define OAPD_S32_GETVALUE( p, n )		(*((int32_t*)&(((unsigned char*)p)[n])))
#define OAPD_S32_LONG_RAM_READ( n )		((guc_OAP_selDataRAM != NULL) ? OAPD_S32_GETVALUE(guc_OAP_selDataRAM, n) : -1)
#ifdef SIMULATOR
static unsigned char	guc_OAP_selDataRAM_Single_SIM[OAPD_SIZE_FILEDATA_SELECT];	// 1言語分データ領域.
#endif

#define OAPD_OFFSET_1ST_LANGCODE		(48)

#define DEF_LanguageSelectMax	9

#define OAP_MAX_USBKEYBOARDLANGUAGES	9

#define OAP_MAX_USBKEYBOARDLANGUAGES	9
#define OAP_MAX_SOFTKEYBOARDLANGUAGES	9
#define MEM_SIZE_OAPILang					(3*9*1024*1024 )
#define SIZE_KEYBOARDDATA_SELECT	(100*1024)
#define OAPD_FONTSWITCH_SIZE	4

#define SIZE_FIXED_DATA			(48)
#define OAPD_SoftKeyMode_Size					16
#define OAPD_USBCharCodeNum						112
#define OAPE_USBKeyKind_Num						2

// 2023/03/11 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1579]FPT-TungDH9 Added Start
#define DEF_TICKTOSECOND	200
uint32_t ui32_StartConnectTick = 0;
// 2023/03/11 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1579]FPT-TungDH9 Added End

Bool b_OAP_USBLoadOK = False;
Bool b_OAP_SoftLoadOK  = False;
TYPE_Language e_OAP_LoadLanguage[9];

TYPE_Language e_OAP_USBLoadLanguage[OAP_MAX_LANGUAGES];
TYPE_Language e_OAP_SoftLoadLanguage[OAP_MAX_LANGUAGES];
static unsigned char*	guc_OAPallDataRAM = NULL;	
static unsigned char*	guc_OAPextFontData = NULL;	
static unsigned char*	g_puc_OAPILangAreaPtr = NULL;		
static unsigned char*	g_puc_OapMyOAPILangAreaPtr = NULL;
#ifdef SIMULATOR
static unsigned char	guc_OAPallDataRAM_SIM[OAPD_SIZE_FILEDATA_SELECT* OAP_MAX_LANGUAGES];
#endif // #ifdef SIMULATOR
	
Bool isAllocSuccess = False;


struct OAPS_GeneralKeyInfo {
	char	sc_KeyTopChar[3];	// 表示する文字のコード
	char	sc_CharCode[3];		// キーコード
};

typedef uchar EP_CharSet;


struct OAPS_GeneralKeyTbl {
	EP_CharSet	ep_charSet;						// フォントセット（キャラクタセット）
	struct OAPS_GeneralKeyInfo keyInfo[OAPD_KeyNumMax];	// 49個分のキー情報
};
struct OAPS_SoftKeyMap {
	Bool valid;												// 有効フラグ（True：有効）.
	TYPE_Language e_Language; 								// キーボードレイアウト種別（国種別）情報.
	char sc_Encode;											// 文字コードエンコード種別（文字集合種別）情報.
	OAPS_GeneralKeyTbl st_CodeData[OAPD_SoftKeyMode_Size];	// 文字コードデータ.
};

typedef struct PMGS_OAPMgrChangeCharCode
{
	short	ss_KeyCode;						// キーコード用

	char	sc_CharCode[4];					// 文字コード用
	// [0]:文字コード1byte目　or　文字ではなければ 0.
	// [1]:文字コード2byte目  or  文字コード2byte目が無い場合 0.　
	// [2]: \0 or 文字コードでは無く、制御キー(スペース・BackSpace等)の場合 Define値
	// [3]:ウムラウト用
	char	sc_CharCodeShift[4];			// 文字コード用
	char	sc_CharCodeAlt[4];				// 文字コード用
	char	sc_CharCodeAltShift[4];			// 文字コード用

} PMGS_OAPMgrChangeCharCode;

struct OAPS_USBCharCode {
	int32_t sl_Num;												// 有効データ数.
	PMGS_OAPMgrChangeCharCode st_CharCode[OAPD_USBCharCodeNum];	// 文字コードデータ.
};

struct OAPS_USBKeyMap {
	Bool valid;												// 有効フラグ（True：有効）.
	TYPE_Language e_Language; 								// キーボードレイアウト種別（国種別）情報.
	char sc_Encode;											// 文字コードエンコード種別（文字集合種別）情報.
	OAPS_USBCharCode st_CodeData[OAPE_USBKeyKind_Num];		// 文字コードデータ.
};

struct OAPS_KeyMapOffsetTable {
	int32_t i32_Addr;
	int32_t i32_Size;
};
struct STOAPILanguageTable {
	char* p_Code;			// 言語コード(ISO639-1code)
	TYPE_Language ec_Type;	// 表示言語(APIC_Setting.h に定義)
};
static const STOAPILanguageTable stOAPILanguageTable[] = {
	{"ja",  	TYPE_LanguageJapanese},		// 日本語.
	{"en",  	TYPE_LanguageAmerican},		// 米語.
	{"de",  	TYPE_LanguageGerman},		// ドイツ語.
	{"fr",  	TYPE_LanguageFrench},		// フランス語.
	{"it",  	TYPE_LanguageItalian},		// イタリア語.
	{"da",  	TYPE_LanguageDanish},		// デンマーク語.
	{"nl",  	TYPE_LanguageDutch},		// オランダ゛語.
	{"pt",  	TYPE_LanguagePortuguese},	// ポルトガル語.
	{"sv",  	TYPE_LanguageSwedish},		// スウェーデン語.
	{"es",  	TYPE_LanguageSpanish},		// スペイン語.
	{"no",  	TYPE_LanguageNorwegian},	// ノルウェー語.
	{"fi",  	TYPE_LanguageFinnish},		// フィンランド語.
	{"cs",  	TYPE_LanguageCzech},		// チェコ語.
	{"hu",  	TYPE_LanguageHangarian},	// ハンガリー語.
	{"el",  	TYPE_LanguageGreek},		// ギリシア語.
	{"tr",  	TYPE_LanguageTurkish},		// トルコ語.
	{"zh-TW",	TYPE_LanguageChinese1},		// 台湾語(TW は ISO3166 で指定された国名).
	{"zh-CN",	TYPE_LanguageChinese2},		// 中国語(CN は ISO3166 で指定された国名).
	{"ko",  	TYPE_LanguageHangul},		// ハングル語.
	{"ru",  	TYPE_LanguageRussian},		// ロシア語.
	{"hr",  	TYPE_LanguageCroatian},		// クロアチア語.
	{"uk",  	TYPE_LanguageUkrainian},	// ウクライナ語.
	{"ro",  	TYPE_LanguageRomanian},		// ルーマニア語.
	{"sk",  	TYPE_LanguageSlovak},		// スロバキア語.
	{"ar",  	TYPE_LanguageArabic},		// アラビア語.
	{"th",  	TYPE_LanguageThai},			// タイ語.
	{"ms",  	TYPE_LanguageMalaysia},		// マレーシア語.
	{"pl",  	TYPE_LanguagePolish},		// ポーランド語.
	{"en-US",  	TYPE_LanguageAmerican},		// 米語(未使用).
	{"en-GB",  	TYPE_LanguageAmerican},		// イギリス英語(未使用).

	{"ca",  	TYPE_LanguageCatalan},		// カタロニア語.
	{"sl",  	TYPE_LanguageSlovene},		// スロベニア語.
	{"bg",		TYPE_LanguageBulgarian},	// ブルガリア語.
	{"eu",		TYPE_LanguageBasque},		// バスク語.
	{"he",  	TYPE_LanguageHebrew},		// ヘブライ語.
	{"vi",  	TYPE_LanguageVietnam},		// ベトナム語.
// 2022/11/16 RQ-938 バルト三国対応 Sky Add start.
	{"lt",  	TYPE_LanguageLithuanian},	// リトアニア語.
	{"lv",  	TYPE_LanguageLatvian},		// ラトビア語.
	{"et",  	TYPE_LanguageEstonian},		// エストニア語.    
// 2022/11/16 RQ-938 バルト三国対応 Sky Add end.
// comment 2023/2/22 Sky小熊 [IT6_2.3][RQ-970]Welsh as standard language for IT.
	{"cy",  	TYPE_LanguageWelsh},		// ウェールズ語.
};

TYPE_Language* m_pstOAP_langTable[OAP_MAX_LANGUAGES];

TYPE_Language* m_pstOAP_LangTable[OAP_MAX_LANGUAGES];

TYPE_Language st_OAP_LangTbl[OAPD_MAX_LANGTBL] = {
	TYPE_LanguageJapanese,	// 日本語.
	TYPE_LanguageAmerican,	// 英語．
	TYPE_LanguageGerman,		// ドイツ語.
	TYPE_LanguageFrench,		// フランス語.
	TYPE_LanguageItalian,		// イタリア語.
	TYPE_LanguageDanish,		// デンマーク語.
	TYPE_LanguageDutch,		// オランダ゛語.
	TYPE_LanguagePortuguese,	// ポルトガル語.
	TYPE_LanguageSwedish,		// スウェーデン語.
	TYPE_LanguageSpanish,		// スペイン語.
	TYPE_LanguageNorwegian,	// ノルウェー語.
	TYPE_LanguageFinnish,		// フィンランド語.
	TYPE_LanguageCzech,		// チェコ語.
	TYPE_LanguageHangarian,	// ハンガリー語.
	TYPE_LanguageGreek,		// ギリシア語.
	TYPE_LanguageTurkish,		// トルコ語.
	TYPE_LanguageChinese1,	// 台湾語.
	TYPE_LanguageChinese2,	// 中国語.
	TYPE_LanguageHangul,		// 韓国語.
	TYPE_LanguageRussian,		// ロシア語.
	TYPE_LanguageCroatian,	// クロアチア語.
	TYPE_LanguageUkrainian,	// ウクライナ語.
	TYPE_LanguageRomanian,	// ルーマニア語.
	TYPE_LanguageSlovak,		// スロベキア語.
	TYPE_LanguageArabic,		// アラビア語.
	TYPE_LanguageThai,			// タイ語.
	TYPE_LanguageMalaysia,	// マレーシア語.
	TYPE_LanguagePolish,		// ポーランド語.
	TYPE_LanguageCatalan,		// カタロニア語.
	TYPE_LanguageSlovene,		// スロベニア語.
	TYPE_LanguageBulgarian,	// ブルガリア語.
	TYPE_LanguageBasque,		// バスク語.
	TYPE_LanguageHebrew,		// ヘブライ語.
	TYPE_LanguageVietnam,		// ベトナム語.
// 2022/11/16 RQ-938 バルト三国対応 Sky Add start.
	TYPE_LanguageLithuanian,	// リトアニア語.
	TYPE_LanguageLatvian,		// ラトビア語.
	TYPE_LanguageEstonian,		// エストニア語.
// 2022/11/16 RQ-938 バルト三国対応 Sky Add end.
// comment 2023/2/22 Sky小熊 [IT6_2.3][RQ-970]Welsh as standard language for IT.
	TYPE_LanguageWelsh,	// ウェールズ語.
	TYPE_LanguageSize,      // [終端].
};

typedef struct {
	TYPE_ExtLangcode e_extLangcode; // External language code.
	TYPE_Language    e_Language;    // Language.
} OAPS_ExtLangCodeChange;

struct OAPS_KeyMap {
	OAPS_SoftKeyMap st_SoftKey[OAP_MAX_SOFTKEYBOARDLANGUAGES]; 	// ソフトキーボード.
	OAPS_USBKeyMap st_USBKey[OAP_MAX_USBKEYBOARDLANGUAGES];		// 外付けキーボード.

	// 表示言語データ.
	short ss_ViewLangNum;									// 表示言語数.
	TYPE_Language e_ViewLanguage[OAP_MAX_LANGUAGES];		// 表示言語情報.
};
// 変換テーブル用構造体.
struct stConvertCode {
	TYPE_Language	e_DBCode;		// IWSのコード.
	TYPE_Language	e_ChangeCode;	// 変換するコード.
};

// 変換テーブル.
const stConvertCode tbl_OAPF_convertTYPELanguage[] = {
	{ 0,	TYPE_LanguageJapanese	},		// 日本語.
	{ 1,	TYPE_LanguageAmerican	},		// 英語.
	{ 2,	TYPE_LanguageGerman		},		// ドイツ語.
	{ 3,	TYPE_LanguageFrench		},		// フランス語.
	{ 4,	TYPE_LanguageItalian	},		// イタリア語.
	{ 5,	TYPE_LanguageDutch		},		// オランダ語.
	{ 6,	TYPE_LanguageDanish		},		// デンマーク語.
	{ 7,	TYPE_LanguageSpanish	},		// スペイン語.
	{ 8,	TYPE_LanguageCzech		},		// チェコ語.
	{ 9,	TYPE_LanguageSwedish	},		// スウェーデン語.
	{ 10,	TYPE_LanguagePortuguese	},		// ポルトガル語.
	{ 11,	TYPE_LanguageFinnish	},		// フィンランド語.
	{ 12,	TYPE_LanguagePolish		},		// ポーランド語.
	{ 13,	TYPE_LanguageHangarian	},		// ハンガリー語.
	{ 14,	TYPE_LanguageRussian	},		// ロシア語.
	{ 15,	TYPE_LanguageNorwegian	},		// ノルウェー語.
	{ 16,	TYPE_LanguageChinese1	},		// 中国繁体字（台湾）.
	{ 17,	TYPE_LanguageChinese2	},		// 中国簡体字（中国）.
	{ 18,	TYPE_LanguageHangul		},		// ハングル.
	{ 19,	TYPE_LanguageArabic		},		// アラビア語.
	{ 20,	TYPE_LanguageAmerican	},		// ヘブライ語(定義値なし(英語を設定)).
	{ 21,	TYPE_LanguageThai		},		// タイ語.
	{ 22,	TYPE_LanguageTurkish	},		// トルコ語.
	{ 23,	TYPE_LanguageGreek		},		// ギリシア語.
	{ 24,	TYPE_LanguageRomanian	},		// ルーマニア語.
	{ 25,	TYPE_LanguageSlovak		},		// スロバキア語.
	{ 26,	TYPE_LanguageEstonian	},		// エストニア語.	// 2022/11/16 RQ-938 バルト三国対応 Sky Add start.
	{ 27,	TYPE_LanguageAmerican	},		// ペルシア語(定義値なし(英語を設定)).
	{ 28,	TYPE_LanguageCroatian	},		// クロアチア語.
	{ 29,	TYPE_LanguageAmerican	},		// インドネシア語(定義値なし(英語を設定)).
	{ 30,	TYPE_LanguageLithuanian	},		// リトアニア語.	// 2022/11/16 RQ-938 バルト三国対応 Sky Add start.
	{ 31,	TYPE_LanguageCatalan	},		// カタロニア語.
	{ 32,	TYPE_LanguageSlovene	},		// スロベニア語.
	{ 33,	TYPE_LanguageUkrainian	},		// ウクライナ語.
	{ 34,	TYPE_LanguageLatvian	},		// ラトビア語	.// 2022/11/16 RQ-938 バルト三国対応 Sky Add start.
	{ 35,	TYPE_LanguageBasque		},		// バスク語.
	{ 36,	TYPE_LanguageBulgarian	},		// ブルガリア語.
	{ 37,	TYPE_LanguageHebrew		},		// ヘブライ語.
	{ 38,	TYPE_LanguageVietnam	},		// ベトナム語.
// comment 2023/2/22 Sky小熊 [IT6_2.3][RQ-970]Welsh as standard language for IT.
	{ 39,	TYPE_LanguageWelsh		},		// ウェールズ語	.
	{ 0x7F,	0x7F					},		// Terminator.
};

OAPS_KeyMap* pst_KeyMap = 0;

static Bool OAPF_makeOffsetTable( const uchar*,uchar ,OAPS_KeyMapOffsetTable* );	// オフセットテーブル作成.

Bool OAPF_isAbleMsgBin()
{
	static Bool b_initAbleMsgBin = False;
	static Bool b_AbleMsgBin = False;
	FILE*	fp = NULL;
	if( False == b_initAbleMsgBin ) {	// 最初の一回のみ言語データ有無確認を行う.
// MessagePathを取得していない場合、取得する.
		if( '\0' == sc_OAP_MessgeBinPath[0] ) {
			memset( sc_OAP_MessgeBinPath, 0, sizeof( sc_OAP_MessgeBinPath ) );
			THRF_getMsgBinName( sc_OAP_MessgeBinPath, sizeof( sc_OAP_MessgeBinPath ) );
		}

		if( NULL != ( fp = fopen( sc_OAP_MessgeBinPath,"r" ) ) ) {	// 言語データ有り.
			b_AbleMsgBin = True;
//			DISPLOG( "OAPF_isAbleMsgBin open ok LINUX_MSG_BIN \n", 0,0,0,0,0,0 );
			fclose( fp );
		} else {	// 言語データ無し.
			b_AbleMsgBin = False;
//			DISPLOG( "OAPF_isAbleMsgBin open ng LINUX_MSG_BIN \n", 0,0,0,0,0,0 );
		}
		b_initAbleMsgBin = True;	// 確認完了.
	}
	return( b_AbleMsgBin );
}

int32_t OAPF_getLangCount()
{
	// CFが刺さっているか検出する.
	//Mosel 関数で取得するように変更する必要がある。井川
#ifdef LINUX
	if( False == OAPF_isAbleMsgBin() )
#else
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();  /* 区間7 honda*** */
	if( True == pc_SystemInfo->isMediaBootType() )
#endif // #ifdef LINUX.
	{
		// 刺さっている場合.
		return 0;
	}
	int32_t i32_num = OAPD_S32_LONG_RAM_READ( OAPD_OFFSET_LANGCOUNT );

	if( i32_num > OAP_MAX_LANGUAGES * 2 ) {
		// エンディアンが逆など明らかにおかしいデータ.
		return 0;
	}
	if( i32_num > OAP_MAX_LANGUAGES ) {
		// 最大限語数に丸める.
		i32_num = OAP_MAX_LANGUAGES;
	}
	return i32_num;
}

TYPE_Language OAPF_getLanguageType( int LangNo )
{
	char szLangCode[OAPD_SIZE_LANGCODE];
	if( LangNo >= OAPF_getLangCount() ) {
		return TYPE_LanguageAmerican;
	}
	// CFが刺さっているか検出する.
	//Mosel 関数で取得するように変更する必要がある。井川
#ifdef LINUX
	if( False == OAPF_isAbleMsgBin() )
#else
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();    /* 区間7 honda*** */
	if( True == pc_SystemInfo->isMediaBootType() )
#endif // #ifdef LINUX.
	{
		// 刺さっている場合.
		return TYPE_LanguageAmerican;
	}
	memcpy( szLangCode,
	        &guc_OAP_selDataRAM[OAPD_OFFSET_1ST_LANGCODE + ( OAPD_SIZE_LANGCODE * LangNo )], OAPD_SIZE_LANGCODE );

	int i;
	for( i = 0; i < ( int )( sizeof( stOAPILanguageTable ) / sizeof( stOAPILanguageTable[0] ) ); i++ ) {
		if( strcmp( szLangCode, stOAPILanguageTable[i].p_Code ) == 0 ) {
			return stOAPILanguageTable[i].ec_Type;
		}
	}
	return TYPE_LanguageAmerican;
}

Bool OAPF_isStandardMSGBinary()
{
	static const TYPE_Language tbl_StdLangs[OAP_MAX_LANGUAGES] = {
		TYPE_LanguageJapanese,	// 日本語.
		TYPE_LanguageAmerican,	// 英語.
		TYPE_LanguageGerman,	// ドイツ語.
		TYPE_LanguageFrench,	// フランス語.
		TYPE_LanguageItalian,	// イタリア語.
		TYPE_LanguageSpanish,	// スペイン語.
		TYPE_LanguageChinese1,	// 台湾語.
		TYPE_LanguageChinese2,	// 中国語.
		TYPE_LanguageHangul,	// 韓国語.
	};
	ushort i, j;
	TYPE_Language ec_Lang;
	ushort us_LangNum = (ushort)( OAPF_getLangCount() );
#if defined(SIMULATOR)
	if( 0 >= us_LangNum ) { return True; }	// SimWin でMSGバイナリがない場合は、標準ROMとしておく.
#endif	// defined(SIMULATOR).
	if( OAP_MAX_LANGUAGES != us_LangNum ) { return False; }	// 搭載言語数が9言語でない場合は、標準MSGバイナリではない.
	for( i = 0; i < OAP_MAX_LANGUAGES; i++ ) {
		ec_Lang = OAPF_getLanguageType( i );	// i 番目の言語データの言語タイプを取得する.
		for( j = 0; j < OAP_MAX_LANGUAGES; j++ ) {
			if( tbl_StdLangs[j] == ec_Lang ) {
				break;
			}
		}
		if( OAP_MAX_LANGUAGES == j ) {
			// 見つからなかった.
			return False;
		}
	}
	return True;
}

void OAPF_setExternalLanguage(int32_t m_sl_LangCount )
{
	APIC_MyServerSetting* pc_MyServerSetting = APIC_MyServerSetting::newInstance( TYPE_MyServerTypeMyPanel );
	if( ( NULL != pc_MyServerSetting ) && ( False == APIC_Enabler::newInstance()->isExtendedMemory() ) ) { // Myパネルが有効でかつ、メモリ拡張されていない（ありえない…）.
		TYPS_MyServerRegistration st_MyServerRegistration = pc_MyServerSetting->getApplication();
		if( True == st_MyServerRegistration.b_EnhancingSetting ) { // 拡張設定On.
			// 言語変換テーブル.
			const OAPS_ExtLangCodeChange st_2ByteLangList[] = {
				{ TYPE_ExtLangcodeJa,   TYPE_LanguageJapanese }, // 日本語.
				{ TYPE_ExtLangcodeZhTw, TYPE_LanguageChinese1 }, // 広東.
				{ TYPE_ExtLangcodeZhCn, TYPE_LanguageChinese2 }, // 中国.
				{ TYPE_ExtLangcodeKo,   TYPE_LanguageHangul   }, // 韓国.
			};

			unsigned short ss_langCount         = 0;
			unsigned short ss_2ByteLangCount    = 0;
			unsigned short ss_2ByteLangPosition = (unsigned short)( m_sl_LangCount );

			for( ss_langCount= 0; ss_langCount < ( unsigned short )m_sl_LangCount; ss_langCount++ ) {
				for( ss_2ByteLangCount = 0; ss_2ByteLangCount < ( unsigned short )( sizeof( st_2ByteLangList ) / sizeof( OAPS_ExtLangCodeChange ) );
				     ss_2ByteLangCount++ ) {
					if( *m_pstOAP_LangTable[ss_langCount] == st_2ByteLangList[ss_2ByteLangCount].e_Language ) {
						ss_2ByteLangPosition = ss_langCount; // ２バイト言語の位置を保存.
						break;
					}
				}
				if( ss_2ByteLangPosition != m_sl_LangCount ) { break; }
			}

			// 拡張言語のチェック.
			TYPE_ExtLangcode st_ExtLang[2];
			st_ExtLang[0] =  st_MyServerRegistration.e_LangageChoice1;
			st_ExtLang[1] =  st_MyServerRegistration.e_LangageChoice2;
			TYPE_ExtLangcode st_CheckExtLang[2];
			unsigned short ss_CheckExtLangCount = 0;
			memset( st_CheckExtLang, 0, sizeof( st_CheckExtLang ) );

			unsigned short ss_extLangCount = 0;
			for( ss_extLangCount = 0; ss_extLangCount < 2; ss_extLangCount++ ) {
				for( ss_2ByteLangCount = 0; ss_2ByteLangCount < ( unsigned short )( sizeof( st_2ByteLangList ) / sizeof( OAPS_ExtLangCodeChange ) );
				     ss_2ByteLangCount++ ) {
					if( st_ExtLang[ss_extLangCount] == st_2ByteLangList[ss_2ByteLangCount].e_extLangcode ) {    // ２バイト言語か.
						st_CheckExtLang[ss_CheckExtLangCount] = st_2ByteLangList[ss_2ByteLangCount].e_Language; // TYPE_ExtLangcodeをTYPE_Languageを変換.
						ss_CheckExtLangCount++;
						break;
					}
				}
			}

			if( ss_2ByteLangPosition == m_sl_LangCount ) {
				// ２バイト言語が含まれていない.
				for( ss_langCount = 0; ss_langCount < ss_CheckExtLangCount; ss_langCount++ ) {
					for( ss_extLangCount = 0; ss_extLangCount < ( unsigned short )( sizeof( st_OAP_LangTbl ) / sizeof( st_OAP_LangTbl[0] ) );
					     ss_extLangCount++ ) {
						if( st_OAP_LangTbl[ss_extLangCount] == st_CheckExtLang[ss_langCount] ) {
							if( DEF_LanguageSelectMax > m_sl_LangCount ) {
								m_pstOAP_LangTable[m_sl_LangCount] = &st_OAP_LangTbl[ss_extLangCount];
								m_sl_LangCount++;
							}
							break;
						}
					}
				}
			} else {
				// ２バイト言語が含まれていた.
				for( ss_langCount = 0; ss_langCount < ss_CheckExtLangCount; ss_langCount++ ) {
					for( ss_extLangCount = 0; ss_extLangCount < ( unsigned short )( sizeof( st_OAP_LangTbl ) / sizeof( st_OAP_LangTbl[0] ) );
					     ss_extLangCount++ ) {
						if( st_OAP_LangTbl[ss_extLangCount] == st_CheckExtLang[ss_langCount] ) {
							if( 0 == ss_langCount ) {
								m_pstOAP_LangTable[ss_2ByteLangPosition] = &st_OAP_LangTbl[ss_extLangCount];
							} else {
								if( DEF_LanguageSelectMax > m_sl_LangCount ) {
									m_pstOAP_LangTable[m_sl_LangCount] = &st_OAP_LangTbl[ss_extLangCount];
									m_sl_LangCount++;
								}
							}
							break;
						}
					}
				}
			}
		}
	}
}
/////////////////////////Start Selection Language//////////////////////////////

Bool OAPF_LanguageStartGamen()
{
	int32_t m_sl_LangCount = OAPF_getLangCount();

	if( 0 >= m_sl_LangCount ) {
		// 言語データを搭載していないときは、日英を選択可能とする(切換え可能かどうかは別にして).
#if defined(SIMULATOR)
		m_pstOAP_LangTable[0] = &st_OAP_LangTbl[1];  // 英語.
		m_pstOAP_LangTable[1] = &st_OAP_LangTbl[2];  // ドイツ語.
		m_pstOAP_LangTable[2] = &st_OAP_LangTbl[3];  // フランス語.
		m_pstOAP_LangTable[3] = &st_OAP_LangTbl[4];  // イタリア語.
		m_pstOAP_LangTable[4] = &st_OAP_LangTbl[9];  // スペイン語.
		m_pstOAP_LangTable[5] = &st_OAP_LangTbl[0];  // 日本語.
		m_pstOAP_LangTable[6] = &st_OAP_LangTbl[16]; // 台湾語.
		m_pstOAP_LangTable[7] = &st_OAP_LangTbl[17]; // 中国語.
		m_pstOAP_LangTable[8] = &st_OAP_LangTbl[18]; // 韓国語.
		m_sl_LangCount = OAP_MAX_LANGUAGES;
#else // defined(SIMULATOR)
		m_pstOAP_LangTable[0] = &st_OAP_LangTbl[1];  // 英語.
		m_pstOAP_LangTable[1] = &st_OAP_LangTbl[0];  // 日本語.
		m_sl_LangCount = 2;
#endif // defined(SIMULATOR)
	} else {
		// 言語データを搭載しているとき.
		m_sl_LangCount = ( m_sl_LangCount > OAP_MAX_LANGUAGES ) ? OAP_MAX_LANGUAGES : m_sl_LangCount; // 言語数は最大OAP_MAX_LANGUAGES(cf.ppcglanguage.h).
		ushort i = 0;
		ushort j = 0;
		TYPE_Language e_Language = TYPE_LanguageJapanese;
		for( i = 0; i < m_sl_LangCount; i++ ) { // 搭載言語数分、m_pstOAP_LangTable[]にデータをセットする.
			m_pstOAP_LangTable[i] = NULL;
			e_Language = OAPF_getLanguageType( i );   // i 番目の言語データの言語タイプを取得する.
			for( j = 0; j < sizeof( st_OAP_LangTbl ) / sizeof( st_OAP_LangTbl[0] ); j++ ) {
				if( TYPE_LanguageSize == st_OAP_LangTbl[j] ) {
					// データが見つからなかった.
					m_sl_LangCount = i;        // 搭載言語数ここまでにする.
					break;
				}
				if( st_OAP_LangTbl[j] == e_Language ) { // 一致するテーブルのポインタをセットする.
					m_pstOAP_LangTable[i] = &st_OAP_LangTbl[j];
					break;
				}
			}
		}
	}
	if( OAP_MAX_LANGUAGES == m_sl_LangCount ) {
		Bool bl_std = OAPF_isStandardMSGBinary();
		if( True == bl_std ) {
			// 標準9言語ROM.
			m_pstOAP_LangTable[0] = &st_OAP_LangTbl[1];  // 英語.
			m_pstOAP_LangTable[1] = &st_OAP_LangTbl[3];  // フランス語.
			m_pstOAP_LangTable[2] = &st_OAP_LangTbl[4];  // イタリア語.
			m_pstOAP_LangTable[3] = &st_OAP_LangTbl[2];  // ドイツ語.
			m_pstOAP_LangTable[4] = &st_OAP_LangTbl[9];  // スペイン語.
			m_pstOAP_LangTable[5] = &st_OAP_LangTbl[0];  // 日本語.
			m_pstOAP_LangTable[6] = &st_OAP_LangTbl[17]; // 中国語.
			m_pstOAP_LangTable[7] = &st_OAP_LangTbl[16]; // 台湾語.
			m_pstOAP_LangTable[8] = &st_OAP_LangTbl[18]; // 韓国語.
			m_sl_LangCount = 9;
		}
	}

	Bool bl_test_flg2 = False;
#if defined(SIMULATOR)
	FILE* fp;
	fp = SYSF_fopen( "panel_test_lang.ini", "r" );
	if( NULL != fp ) {
		bl_test_flg2 = True;
	}
#endif
	if( True == bl_test_flg2 ) {
		m_pstOAP_LangTable[0] = &st_OAP_LangTbl[5];
		m_pstOAP_LangTable[1] = &st_OAP_LangTbl[6];
		m_pstOAP_LangTable[2] = &st_OAP_LangTbl[7];
		m_pstOAP_LangTable[3] = &st_OAP_LangTbl[8];
		m_pstOAP_LangTable[4] = &st_OAP_LangTbl[10];
		m_pstOAP_LangTable[5] = &st_OAP_LangTbl[12];
	}
#if defined(SIMULATOR)
	if( NULL != fp ) {
		SYSF_fclose( fp );
		bl_test_flg2 = False;
	}
#endif
	// 画面表示は9言語までのため、10言語以上であれば、丸める.
	m_sl_LangCount = ( DEF_LanguageSelectMax < m_sl_LangCount ) ? DEF_LanguageSelectMax : m_sl_LangCount;
	OAPF_setExternalLanguage(m_sl_LangCount);
	return True;
}
/////////////////////////End Selection Language//////////////////////////////


/////////////////////////////////USB KEY BOARD////////////////////////////////////////////
void OAPF_addUSBLoadLanguage( TYPE_Language e_Language )
{
	for( uchar i = 0; i < ( uchar )( sizeof( e_OAP_USBLoadLanguage )/sizeof( TYPE_Language ) ); i++ ) {
		if( e_Language == e_OAP_USBLoadLanguage[i] ) {
			// 既に存在するので追加しない.
			return;
		}

		if( -1 == e_OAP_USBLoadLanguage[i] ) {
			// 空き領域まで来たので追加.
			e_OAP_USBLoadLanguage[i] = e_Language;
			return;
		}
	}
}
Bool OAPF_getLanguageDataForUINewBin( unsigned short* pss_Buf, uint32_t ui32_Offset, uint32_t ui32_Size )
{
#if !defined(COSIM)
#ifdef LINUX
#define MSG_DATA_CACHE	512				// 言語データキャッシュサイズ用定義.
	static Bool bl_Init = False;			// 言語データを読み込んだかどうか.
	static Bool bl_ExistLangData = False;	// 言語データの有無フラグ.
	static Bool bl_ExistCacheData = False;	// 言語データ（キャッシュ分）の有無フラグ.
	static unsigned short us_FirstSector[MSG_DATA_CACHE / sizeof( unsigned short )];	// 言語データ最初の512Byteデータ.
	FILE* fp;								// 言語データ用ファイルポインタ.
	Bool bl_ret = False;					// 戻り値.
	memset( pss_Buf, 0, ui32_Size );			// 転送先クリア.

	if( False == bl_Init ) {	// 最初の一回のみ言語データ有無確認を行う.
// MessagePathを取得していない場合、取得する.
		if( '\0' == sc_OAP_MessgeBinPath[0] ) {
			memset( sc_OAP_MessgeBinPath, 0, sizeof( sc_OAP_MessgeBinPath ) );
			THRF_getMsgBinName( sc_OAP_MessgeBinPath, sizeof( sc_OAP_MessgeBinPath ) );
		}
		if( NULL != ( fp = SFLG_Fopen( sc_OAP_MessgeBinPath, "rb" ) ) ) {
			bl_ExistLangData = True;	// 言語データ有り.
			/* 先頭512Byteのデータを保持しておく */
			size_t s_cashdata = SFLG_Fread( us_FirstSector, 1, MSG_DATA_CACHE, fp );
			if( s_cashdata != ( unsigned int )ERROR )
			{
				bl_ExistCacheData = True;
			}
			SFLG_Fclose( fp );
		}
		bl_Init = True;	// 確認完了.
	}
	if( True == bl_ExistLangData ) {	// 言語データが存在する場合はファイル読込みを行う.
		if( bl_ExistCacheData == True )
		{	/* キャッシュデータがある場合はそちらから読み出す */
			if( ui32_Offset + ui32_Size <= MSG_DATA_CACHE )
			{	// 512Byte以内かどうか.
				// キャッシュしたデータ内に要求されたデータがある.
				memcpy( pss_Buf, us_FirstSector + ui32_Offset / sizeof( unsigned short ), ui32_Size );
				return True;
			}
		}
// MessagePathを取得していない場合、取得する.
		if( '\0' == sc_OAP_MessgeBinPath[0] ) {
			memset( sc_OAP_MessgeBinPath, 0, sizeof( sc_OAP_MessgeBinPath ) );
			THRF_getMsgBinName( sc_OAP_MessgeBinPath, sizeof( sc_OAP_MessgeBinPath ) );
		}
		fp = SFLG_Fopen( sc_OAP_MessgeBinPath, "rb" );

		if( NULL != fp ) {
			SFLG_Fseek( fp, ui32_Offset, SEEK_SET );
			size_t s_ret = SFLG_Fread( pss_Buf, 1, ui32_Size, fp );
			if( s_ret != ui32_Size ) {
				memset( pss_Buf, 0, ui32_Size );	// 全部読込めなかったら全クリアする.
			} else {
				bl_ret = True;	// 読込み完了.
			}
			SFLG_Fclose( fp );
		}
	}
	return bl_ret;
#else   // #ifdef LINUX.
	static Bool bl_Init = False;														// 最初のセクタを読み込んだかどうか.
	static Bool bl_ExistLangData = False;												// 言語データの有無フラグ.
#define MSG_DATA_CASH	512
#define	FLASH_BYTE_PER_SECTOR			( 256 * 1024 )
	static ushort us_FirstSector[MSG_DATA_CASH / sizeof( unsigned short )];				// 言語データ最初の512Byteデータ.
	static uint32_t ui32_SectorNum = 0;

	// 転送先クリア.
	memset( pss_Buf, 0, ui32_Size );
	// パラメータチェック.
	if( ( ( uint32_t )pss_Buf & 0x01 ) == 1 ) {
		// ポインタが2バイト境界にない.
		return False;
	}
	if( ( ui32_Offset & 0x01 ) == 1 ) {
		// オフセットが奇数.
		return False;
	}
	if( ( ui32_Size & 0x01 ) == 1 ) {
		// サイズが奇数.
		return False;
	}
	// 言語データの有無チェック.
	if( bl_Init ) {
		if( bl_ExistLangData == False ) {
			// 言語データなし.
			return False;
		}
	} else {
		bl_ExistLangData = True;
		bl_Init = True;
	}

	// 領域内かチェック.
	// 1セクタ256KB単位によるMSGバイナリ領域内チェック.
	if( ( ui32_Offset + ui32_Size + FLASH_BYTE_PER_SECTOR - 1 ) / FLASH_BYTE_PER_SECTOR > ui32_SectorNum ) {
		// 領域外アクセス.
		return False;
	}
	static ushort us_Buf[FLASH_BYTE_PER_SECTOR / sizeof( unsigned short )];	// 1セクタ256KB領域をstatic定義.
	uint32_t ui32_SecNum = ui32_Offset / FLASH_BYTE_PER_SECTOR;					// オフセットのセクタ換算(256KB単位).
	uint32_t ui32_CpySize = 0;

	// 要求されたデータがキャッシュしたデータ内にあるかチェック.
	if( ui32_Offset + ui32_Size <= MSG_DATA_CASH ) {	// 512Byte以内かどうか.
		// キャッシュしたデータ内に要求されたデータがある.
		memcpy( pss_Buf, us_FirstSector + ui32_Offset / sizeof( unsigned short ), ui32_Size );
		return True;
	}

	// オフセットがセクタの単位で始まっているかチェック.
	if( ui32_Offset % FLASH_BYTE_PER_SECTOR != 0 ) {	// 256KB単位かどうか.
		// セクタ内の途中のデータが必要.
		ui32_CpySize = ui32_Size;
		if( ui32_Offset % FLASH_BYTE_PER_SECTOR + ui32_Size <= FLASH_BYTE_PER_SECTOR ) {
			// 指定されたデータが読み込んだセクタ内にある.
			// セクタの途中から指定サイズをコピーして終了.
			memcpy( pss_Buf, us_Buf + ( ui32_Offset % FLASH_BYTE_PER_SECTOR ) / sizeof( unsigned short ), ui32_Size );
			return True;
		} else {
			// セクタの途中からセクタの最後までコピー. 続きあり.
			ui32_CpySize = FLASH_BYTE_PER_SECTOR - ui32_Offset % FLASH_BYTE_PER_SECTOR;
			memcpy( pss_Buf, us_Buf + ( ui32_Offset % FLASH_BYTE_PER_SECTOR ) / sizeof( unsigned short ), ui32_CpySize );
			pss_Buf += ui32_CpySize / sizeof( unsigned short );
			ui32_Offset += ui32_CpySize;
			ui32_Size -= ui32_CpySize;
		}
	}

	// セクタの先頭からセクタの最後まで必要なデータをコピーする.
	ui32_SecNum = ui32_Offset / FLASH_BYTE_PER_SECTOR;				// セクタ開始位置(256KB単位).
	uint32_t ui32_SecCnt = ui32_Size / FLASH_BYTE_PER_SECTOR;			// セクタ数(256KB単位).
	ui32_CpySize = ui32_SecCnt * FLASH_BYTE_PER_SECTOR;				// 読み込みバイト数.
	pss_Buf += ui32_CpySize / sizeof( unsigned short );
	ui32_Offset += ui32_CpySize;
	ui32_Size -= ui32_CpySize;

	// 最後の端数があれば、その部分をコピーする.
	if( ui32_Size > 0 ) {
		memcpy( pss_Buf, us_Buf, ui32_Size );
	}
	ui32_CpySize = ui32_Size;
	return True;
#endif	// #ifdef LINUX.
#else	//#if !defined(COSIM)
	static Bool bl_Init = False;														// 最初のセクタを読み込んだかどうか.
	static Bool bl_ExistLangData = False;												// 言語データの有無フラグ.
#define MSG_DATA_CASH	512
#define	FLASH_BYTE_PER_SECTOR			( 256 * 1024 )
	static ushort us_FirstSector[MSG_DATA_CASH / sizeof( unsigned short )];				// 言語データ最初の512Byteデータ.
	static uint32_t ui32_SectorNum = 0;

	// 転送先クリア.
	memset( pss_Buf, 0, ui32_Size );
	// パラメータチェック.
	if( ( ( uint32_t )pss_Buf & 0x01 ) == 1 ) {
		// ポインタが2バイト境界にない.
		return False;
	}
	if( ( ui32_Offset & 0x01 ) == 1 ) {
		// オフセットが奇数.
		return False;
	}
	if( ( ui32_Size & 0x01 ) == 1 ) {
		// サイズが奇数.
		return False;
	}
	// 言語データの有無チェック.
	if( bl_Init ) {
		if( bl_ExistLangData == False ) {
			// 言語データなし.
			return False;
		}
	} else {
		bl_ExistLangData = True;
		bl_Init = True;
	}
	// 領域内かチェック.
	// 1セクタ256KB単位によるMSGバイナリ領域内チェック.
	if( ( ui32_Offset + ui32_Size + FLASH_BYTE_PER_SECTOR - 1 ) / FLASH_BYTE_PER_SECTOR > ui32_SectorNum ) {
		// 領域外アクセス.
		return False;
	}

	static ushort us_Buf[FLASH_BYTE_PER_SECTOR / sizeof( unsigned short )];	// 1セクタ256KB領域をstatic定義.
	uint32_t ui32_SecNum = ui32_Offset / FLASH_BYTE_PER_SECTOR;					// オフセットのセクタ換算(256KB単位).
	uint32_t ui32_CpySize = 0;

	// 要求されたデータがキャッシュしたデータ内にあるかチェック.
	if( ui32_Offset + ui32_Size <= MSG_DATA_CASH ) {	// 512Byte以内かどうか.
		// キャッシュしたデータ内に要求されたデータがある.
		memcpy( pss_Buf, us_FirstSector + ui32_Offset / sizeof( unsigned short ), ui32_Size );
		return True;
	}
	// オフセットがセクタの単位で始まっているかチェック.
	if( ui32_Offset % FLASH_BYTE_PER_SECTOR != 0 ) {	// 256KB単位かどうか.
		// セクタ内の途中のデータが必要.
		ui32_CpySize = ui32_Size;
		if( ui32_Offset % FLASH_BYTE_PER_SECTOR + ui32_Size <= FLASH_BYTE_PER_SECTOR ) {
			// 指定されたデータが読み込んだセクタ内にある.
			// セクタの途中から指定サイズをコピーして終了.
			memcpy( pss_Buf, us_Buf + ( ui32_Offset % FLASH_BYTE_PER_SECTOR ) / sizeof( unsigned short ), ui32_Size );
			return True;
		} else {
			// セクタの途中からセクタの最後までコピー. 続きあり.
			ui32_CpySize = FLASH_BYTE_PER_SECTOR - ui32_Offset % FLASH_BYTE_PER_SECTOR;
			memcpy( pss_Buf, us_Buf + ( ui32_Offset % FLASH_BYTE_PER_SECTOR ) / sizeof( unsigned short ), ui32_CpySize );
			pss_Buf += ui32_CpySize / sizeof( unsigned short );
			ui32_Offset += ui32_CpySize;
			ui32_Size -= ui32_CpySize;
		}
	}
	// セクタの先頭からセクタの最後まで必要なデータをコピーする.
	ui32_SecNum = ui32_Offset / FLASH_BYTE_PER_SECTOR;				// セクタ開始位置(256KB単位).
	uint32_t ui32_SecCnt = ui32_Size / FLASH_BYTE_PER_SECTOR;			// セクタ数(256KB単位).
	ui32_CpySize = ui32_SecCnt * FLASH_BYTE_PER_SECTOR;				// 読み込みバイト数.
	pss_Buf += ui32_CpySize / sizeof( unsigned short );
	ui32_Offset += ui32_CpySize;
	ui32_Size -= ui32_CpySize;

	// 最後の端数があれば、その部分をコピーする.
	if( ui32_Size > 0 ) {
		memcpy( pss_Buf, us_Buf, ui32_Size );
	}
	ui32_CpySize = ui32_Size;

	return True;
#endif	// #if !defined(COSIM)
}

static inline int OAPF_loadLangData2Ram( void* pRam, int32_t i32_src, size_t size )
{
	memset( pRam, 0, size );	// メモリクリア.
	if( i32_src < 0 ) { return -1; }

	OAPF_getLanguageDataForUINewBin( ( unsigned short* )pRam, (uint32_t)( i32_src ), (uint32_t)size );

	return 0;
}

struct OAPS_SelectLangData {				// 選択言語情報.
	char sc_LangCode[OAPD_SIZE_LANGCODE];	// 言語コード.
	uchar uc_MSGBINIndex;				// メッセージバイナリ内の順番.
};


class OAPC_SelectLangInfo {	// 選択言語情報管理クラス.
private:
	OAPS_SelectLangData m_st_SelectLangData[OAP_MAX_LANGUAGES];	// 選択言語情報.
	uchar m_uc_SelectLangNum;									// 選択言語数.
	int32_t m_i32_MSGBINLangNum;								// メッセージバイナリ内の言語数.
	void clearData();											// データ初期化.

public:
	OAPC_SelectLangInfo();										// コンストラクタ.
	void createSelectLangInfo();								// 選択言語データ構築.
	uchar getMSGBINIndex( uchar uc_index );						// メッセージバイナリ内の順番取得.
	char* getLangCode( uchar uc_index );							// 言語コード取得.
	uchar getSelectLangNum();									// 選択言語数取得.
	int32_t getMSGBINLangNum();									// メッセージバイナリ内の言語数取得.
};


void OAPC_SelectLangInfo::clearData() {	
	for( uchar i=0; i<OAP_MAX_LANGUAGES; i++ ) {
		memset( m_st_SelectLangData[i].sc_LangCode, '\0', sizeof( m_st_SelectLangData[i].sc_LangCode ) );
		m_st_SelectLangData[i].uc_MSGBINIndex = 255;
	}
	m_uc_SelectLangNum = 0;
	m_i32_MSGBINLangNum = 0;
}

OAPC_SelectLangInfo::OAPC_SelectLangInfo() {
	clearData();								// データ初期化.
}

uchar OAPC_SelectLangInfo::getMSGBINIndex( uchar uc_index ) {
	if( m_uc_SelectLangNum <= uc_index ) {		// 選択言語数以上の場合はエラー.
//		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return 255;
	}
	return m_st_SelectLangData[uc_index].uc_MSGBINIndex;
}


char* OAPC_SelectLangInfo::getLangCode( uchar uc_index ) {
	if( m_uc_SelectLangNum <= uc_index ) {		// 選択言語数以上の場合はエラー.
//		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return NULL;
	}
	if( ( uchar )( sizeof( m_st_SelectLangData ) / sizeof( m_st_SelectLangData[0] ) ) > uc_index ) {
		return m_st_SelectLangData[uc_index].sc_LangCode;
	} else {
		return NULL;
	}
}

uchar OAPC_SelectLangInfo::getSelectLangNum() {
	return m_uc_SelectLangNum;
}

int32_t OAPC_SelectLangInfo::getMSGBINLangNum() {
	return m_i32_MSGBINLangNum;
}
#define OAPD_SELECTLANGFILEPATH_LEN		(64)

void OAPC_SelectLangInfo::createSelectLangInfo() {
	clearData();								// データ初期化.

// メッセージバイナリから言語数を取得.
	int32_t i32_LangNum = 0;						// メッセージバイナリ内の搭載言語数.
	OAPF_loadLangData2Ram( &i32_LangNum, OAPD_OFFSET_LANGCOUNT, sizeof( i32_LangNum ) );
	if( 0 < i32_LangNum ) {
		m_i32_MSGBINLangNum = i32_LangNum;
	} else {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}

// メッセージバイナリから言語コードを取得.
	char sc_MSGBINLangCode[m_i32_MSGBINLangNum*OAPD_SIZE_LANGCODE];		// 言語コードのサイズは言語数×8バイト.
	memset( sc_MSGBINLangCode, '\0', sizeof( sc_MSGBINLangCode ) );
	OAPF_loadLangData2Ram( &sc_MSGBINLangCode, OAPD_OFFSET_1ST_LANGCODE, sizeof( sc_MSGBINLangCode ) );

// 選択言語ファイルより言語コードを取得する.
	char sc_SelectLangCode[OAP_MAX_LANGUAGES*OAPD_SIZE_LANGCODE];	// 言語コードのサイズは言語数(9)×8バイト.
	char sc_SelectLangPath[OAPD_SELECTLANGFILEPATH_LEN];		// 選択言語ファイルのファイルパス.
	uchar uc_SelectLangNum = 0;									// 選択言語ファイル内の言語数.
	memset( sc_SelectLangCode, '\0', sizeof( sc_SelectLangCode ) );
	memset( sc_SelectLangPath, '\0', sizeof( sc_SelectLangPath ) );
	// 選択言語ファイルのファイルパスをスレッドより取得する.
	THRF_getSelectLangDataFilePath( sc_SelectLangPath, ( uchar )( sizeof( sc_SelectLangPath ) ) );

	FILE* fp = SFLG_Fopen( sc_SelectLangPath, "rb" );				// 言語データ用ファイルポインタ.
	if( NULL != fp ) {
		SFLG_Fread( sc_SelectLangCode, 1, OAP_MAX_LANGUAGES*OAPD_SIZE_LANGCODE, fp );
		SFLG_Fclose( fp );	// ファイルのクローズ.

		// 先頭文字が"\0"でなければ有効言語コードと判断し,言語コード数+1する.
		for( uchar uc_index = 0; OAP_MAX_LANGUAGES > uc_index; uc_index++ ) {
			if( '\0' != *( sc_SelectLangCode + OAPD_SIZE_LANGCODE * uc_index ) ) {
				uc_SelectLangNum++;
			} else {	// 先頭"\0"以降の言語コードも"\0"なのでループを抜ける.
				break;
			}
		}
	} else {		// ファイルオープン失敗時(ファイルが存在しない)
		memcpy( sc_SelectLangCode, sc_MSGBINLangCode, sizeof( sc_SelectLangCode ) );	// 先頭から9言語分のデータを取得.

		// 言語数が9以上の場合は最大言語数に補正する.
		if( OAP_MAX_LANGUAGES < m_i32_MSGBINLangNum ) {
			uc_SelectLangNum = OAP_MAX_LANGUAGES;
		} else {
			uc_SelectLangNum = ( uchar )m_i32_MSGBINLangNum;
		}
	}

	// アドレス移動させるので先頭アドレスを変数へコピーする.
	char* psc_MSGBINLangCode = sc_MSGBINLangCode;
	char* psc_SelectLangCode = sc_SelectLangCode;

	for( uchar i=0; i < ( uchar )m_i32_MSGBINLangNum; i++ ) {
		if( m_uc_SelectLangNum >= uc_SelectLangNum ) {		// 選択言語数分 処理が完了している.
			break;
		}
		for( uchar j=0; j < uc_SelectLangNum; j++ ) {
			if( 0 == strncmp( psc_MSGBINLangCode,psc_SelectLangCode,OAPD_SIZE_LANGCODE ) ) {
				memcpy( m_st_SelectLangData[m_uc_SelectLangNum].sc_LangCode, psc_MSGBINLangCode, OAPD_SIZE_LANGCODE );
				m_st_SelectLangData[m_uc_SelectLangNum].uc_MSGBINIndex = i;
				m_uc_SelectLangNum++;
				break;
			}
			psc_SelectLangCode += OAPD_SIZE_LANGCODE;		// 次の言語コードへ(8バイト分進める).
		}
		psc_MSGBINLangCode += OAPD_SIZE_LANGCODE;		// 次の言語コードへ(8バイト分進める).
		psc_SelectLangCode = sc_SelectLangCode;		// 選択言語は先頭アドレスに戻す.
	}
}

static OAPC_SelectLangInfo c_SelectLangInfo;

static int OAPF_loadSelectLangKeymap( uchar* uc_RAMBuffer, uchar uc_RAMBufferSize, int32_t i32_OffsetTableKey, uchar uc_DataSize ) {

	int si_ret = -1;
	if( NULL == uc_RAMBuffer || 0 == uc_RAMBufferSize
	    || 0 == uc_DataSize|| 0 >= i32_OffsetTableKey ) {	// 引数チェック.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return si_ret;
	}

	uchar* puc_RAMBuffer = uc_RAMBuffer;		// 格納先バッファのアドレス.
	int32_t i32_BuffSize = 0;						// ローカルバッファのサイズ.

	c_SelectLangInfo.createSelectLangInfo();

	i32_BuffSize = ( int32_t )uc_DataSize * c_SelectLangInfo.getMSGBINLangNum();	// 全言語数分のバッファサイズを算出.

	if( 0 >= i32_BuffSize ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return si_ret;
	}

	uchar uc_TmpBuf[i32_BuffSize];							// 一時用バッファ.
	memset( uc_TmpBuf, 0, sizeof( uc_TmpBuf ) );
	si_ret = OAPF_loadLangData2Ram( uc_TmpBuf, i32_OffsetTableKey, sizeof( uc_TmpBuf ) );		// バイナリからデータを取得.
	if( -1 == si_ret ) {		//エラー発生時.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return si_ret;
	}

	for( uchar i = 0; i <  c_SelectLangInfo.getSelectLangNum() && i < OAP_MAX_LANGUAGES ; i++ ) {	// 選択言語数回ループする.
		if( ( int32_t )( sizeof( uc_TmpBuf ) / sizeof( uc_TmpBuf[0] ) ) > ( int32_t )( uc_DataSize * c_SelectLangInfo.getMSGBINIndex( i ) ) ) {
			// 選択言語のオフセットアドレス+サイズを格納先バッファ(計8バイト)をコピーする.
			memcpy( puc_RAMBuffer, ( uc_TmpBuf +  uc_DataSize * c_SelectLangInfo.getMSGBINIndex( i ) ),
			        uc_DataSize );
			puc_RAMBuffer += uc_DataSize;	// 格納先バッファのアドレスを次の領域(8バイト先)に進める
		}
	}
	return si_ret;
}

static	TYPE_Language	OAPF_convertTYPELanguage( TYPE_Language e_Language )
{
	TYPE_Language	e_RetCode=TYPE_LanguageAmerican;
	uchar	ucLoop=0;

	while( 0x7F != tbl_OAPF_convertTYPELanguage[ucLoop].e_DBCode ) {
		if( tbl_OAPF_convertTYPELanguage[ucLoop].e_DBCode == e_Language ) {
			e_RetCode = tbl_OAPF_convertTYPELanguage[ucLoop].e_ChangeCode;
			break;
		}
		ucLoop++;
	}
	return	e_RetCode;
}

Bool OAPF_makeSoftCharCodeTable( const uchar* puc_Start,uchar sc_Num,OAPS_GeneralKeyInfo* st_Table )
{
	if( ( NULL == puc_Start ) || ( NULL == st_Table ) ) {
		return False;
	}
	const uchar* puc_Current = puc_Start;

	// 指定された読込個数が、キーの最大個数を超えた場合は.
	// 読込個数をキーの最大個数(=49)に丸め込む.
	// plibh2.h.
	if( OAPD_KeyNumMax < sc_Num ) { sc_Num = OAPD_KeyNumMax; }
	for( uchar i = 0; i < sc_Num; i++ ) {
		// キートップ表示用文字コード.
		memcpy( st_Table[i].sc_KeyTopChar,puc_Current,sizeof( st_Table[i].sc_KeyTopChar ) );
		puc_Current += sizeof( st_Table[i].sc_KeyTopChar );

		// 内部制御用文字コード.
		memcpy( st_Table[i].sc_CharCode,puc_Current,sizeof( st_Table[i].sc_CharCode ) );
		puc_Current += sizeof( st_Table[i].sc_CharCode );
	}
	return True;
}

Bool OAPF_loadSoftKeyMapData( int32_t i32_Addr,int32_t i32_Size,OAPS_SoftKeyMap* st_SoftKeyMap )
{
	if( NULL == st_SoftKeyMap ) {
		printf( "[KEYMAP]★NULL pointer  st_SoftKeyMap:[%p]\n",st_SoftKeyMap );
		return False;
	}
	st_SoftKeyMap->valid = False;		// データを取得するまで無効.

	if( -1 == i32_Addr ) { return True; }	// データなし.

	uchar uc_RAMBuffer[5000];
	memset( uc_RAMBuffer,0,sizeof( uc_RAMBuffer ) );
	if( ( int32_t )sizeof( uc_RAMBuffer ) < i32_Size ) {
		printf( "[KEYMAP]★size error(SoftKeyMap)\n" );
		return False;
	}
	// Flashからロード.
	if( -1 == OAPF_loadLangData2Ram( uc_RAMBuffer,i32_Addr,i32_Size ) ) {
		printf( "[KEYMAP]★data load error(SoftKeyMap)\n" );
		return False;
	}
	uchar* puc_CurrentPoint = uc_RAMBuffer;

	// キーボードレイアウト種別（国種別）情報.
	short ss_LanguageTmp = 0;
	memcpy( &( ss_LanguageTmp ),puc_CurrentPoint,sizeof( short ) );
	printf( "■ss_LanguageTmp:%d\n",ss_LanguageTmp );
	TYPE_Language e_LanguageTmp = ( TYPE_Language )ss_LanguageTmp;
	e_LanguageTmp = OAPF_convertTYPELanguage( e_LanguageTmp );	// MSGバイナリの定義値(言語コード)をTYPE_Languageの定義値に変換.
	st_SoftKeyMap->e_Language = e_LanguageTmp;

	puc_CurrentPoint += sizeof( short );

	// 文字コードエンコード種別（文字集合種別）情報.
	memcpy( &( st_SoftKeyMap->sc_Encode ),
	        puc_CurrentPoint,
	        sizeof( st_SoftKeyMap->sc_Encode ) );

	puc_CurrentPoint += sizeof( st_SoftKeyMap->sc_Encode );

	// 文字コードオフセットアドレスの取得.
	OAPS_KeyMapOffsetTable st_CharCodeTableTmp[OAPD_SoftKeyMode_Size];
	memset( st_CharCodeTableTmp, 0x00, sizeof( st_CharCodeTableTmp ) );
	OAPF_makeOffsetTable( puc_CurrentPoint,sizeof( st_CharCodeTableTmp )/sizeof( OAPS_KeyMapOffsetTable ),st_CharCodeTableTmp );

	// 文字コードデータ取得.
	for( uchar i = 0; i < ( uchar )( sizeof( st_CharCodeTableTmp )/sizeof( OAPS_KeyMapOffsetTable ) ); i++ ) {

		// エンコード種別は言語単位で固定.
		st_SoftKeyMap->st_CodeData[i].ep_charSet = st_SoftKeyMap->sc_Encode;

		// 格納文字コード数.
		uchar uc_TableNum = (uchar)( st_CharCodeTableTmp[i].i32_Size / ( 6 * sizeof( char ) ) );

		// 文字コードテーブル作成.
		OAPF_makeSoftCharCodeTable( uc_RAMBuffer + st_CharCodeTableTmp[i].i32_Addr,uc_TableNum,st_SoftKeyMap->st_CodeData[i].keyInfo );
	}
	st_SoftKeyMap->valid = True;

	return True;
}
Bool OAPF_makeUSBCharCodeTable( const uchar* puc_Start,uchar sc_Num,OAPS_USBCharCode* st_Table )
{
	if( ( NULL == puc_Start ) || ( NULL == st_Table ) ) {
		printf( "[KEYMAP]★NULL pointer  puc_Start:[%p],st_Table:[%p]\n",puc_Start,st_Table );
		return False;
	}
	const uchar* puc_Current = puc_Start;

	// 指定された読込個数が、キーの最大個数を超えた場合は.
	// 読込個数をキーの最大個数(=112)に丸め込む.
	// pdpc2_CLCD.h.
	if( OAPD_USBCharCodeNum < sc_Num ) { sc_Num = OAPD_USBCharCodeNum; }

	st_Table->sl_Num = sc_Num;

	for( uchar i = 0; i < sc_Num; i++ ) {
		// キーコード.
		memcpy( &( st_Table->st_CharCode[i].ss_KeyCode ),puc_Current,sizeof( st_Table->st_CharCode[i].ss_KeyCode ) );
		puc_Current += sizeof( st_Table->st_CharCode[i].ss_KeyCode );

		// 通常入力文字コード.
		memcpy( &( st_Table->st_CharCode[i].sc_CharCode ),puc_Current,sizeof( st_Table->st_CharCode[i].sc_CharCode ) );
		puc_Current += sizeof( st_Table->st_CharCode[i].sc_CharCode );

		// Shift同時押し時文字コード.
		memcpy( &( st_Table->st_CharCode[i].sc_CharCodeShift ),puc_Current,sizeof( st_Table->st_CharCode[i].sc_CharCodeShift ) );
		puc_Current += sizeof( st_Table->st_CharCode[i].sc_CharCodeShift );

		// Alt同時押し時文字コード.
		memcpy( &( st_Table->st_CharCode[i].sc_CharCodeAlt ),puc_Current,sizeof( st_Table->st_CharCode[i].sc_CharCodeAlt ) );
		puc_Current += sizeof( st_Table->st_CharCode[i].sc_CharCodeAlt );

		// Shift+Alt同時押し時文字コード.
		memcpy( &( st_Table->st_CharCode[i].sc_CharCodeAltShift ),puc_Current,sizeof( st_Table->st_CharCode[i].sc_CharCodeAltShift ) );
		puc_Current += sizeof( st_Table->st_CharCode[i].sc_CharCodeAltShift );
	}

	return True;
}

Bool OAPF_getViewLanguage( int32_t i32_LangNo,TYPE_Language* pe_Language )
{
	if( ( i32_LangNo < 0 )
	    || ( i32_LangNo >= OAP_MAX_LANGUAGES )
	    || ( NULL == pe_Language )
	    || ( NULL == guc_OAP_selDataRAM ) ) {
		printf( "[KEYMAP]★Data Error  i32_LangNo:[%d],pe_Language:[%p],guc_OAP_selDataRAM:[%p]\n",i32_LangNo,pe_Language,guc_OAP_selDataRAM );
		return False;
	}

	char	sc_LangCode[OAPD_SIZE_LANGCODE];
	memset( sc_LangCode, 0, sizeof( sc_LangCode ) );

	memcpy( sc_LangCode,&guc_OAP_selDataRAM[OAPD_OFFSET_1ST_LANGCODE + ( OAPD_SIZE_LANGCODE * i32_LangNo )], OAPD_SIZE_LANGCODE );

	for( uchar i = 0; i < ( uchar )( sizeof( stOAPILanguageTable ) / sizeof( STOAPILanguageTable ) ); i++ ) {
		if( strncmp( sc_LangCode,stOAPILanguageTable[i].p_Code,sizeof( sc_LangCode ) ) == 0 ) {
			*pe_Language = stOAPILanguageTable[i].ec_Type;
			return True;
		}
	}
	return False;
}

Bool OAPF_loadUSBKeyMapData( int32_t i32_Addr,int32_t i32_Size,OAPS_USBKeyMap* st_USBKeyMap )
{
	if( NULL == st_USBKeyMap ) {
		printf( "[KEYMAP]★NULL pointer  st_USBKeyMap:[%p]\n",st_USBKeyMap );
		return False;
	}
	st_USBKeyMap->valid = False;		// データを取得するまで無効.

	if( -1 == i32_Addr )	{ return True; }	// データなし.

	uchar uc_RAMBuffer[5000];
	memset( uc_RAMBuffer,0,sizeof( uc_RAMBuffer ) );
	if( ( int32_t )sizeof( uc_RAMBuffer ) < i32_Size ) {
		printf( "[KEYMAP]★size error(st_USBKeyMap)\n" );
		return False;
	}
	// Flashからロード.
	if( -1 == OAPF_loadLangData2Ram( uc_RAMBuffer,i32_Addr,i32_Size ) ) {
		printf( "[KEYMAP]★data load error(st_USBKeyMap)\n" );
		return False;
	}
	uchar* puc_CurrentPoint = uc_RAMBuffer;

	// キーボードレイアウト種別（国種別）情報.
	short ss_LanguageTmp = 0;
	memcpy( &( ss_LanguageTmp ),puc_CurrentPoint,sizeof( short ) );
	TYPE_Language e_LanguageTmp = ( TYPE_Language )ss_LanguageTmp;
	e_LanguageTmp = OAPF_convertTYPELanguage( e_LanguageTmp );	// MSGバイナリのコードがIWSのコードなのでTYPE_Languageの定義値に変換.
	st_USBKeyMap->e_Language = e_LanguageTmp;

	puc_CurrentPoint += sizeof( short );

	// 文字コードエンコード種別（文字集合種別）情報.
	memcpy( &( st_USBKeyMap->sc_Encode ),
	        puc_CurrentPoint,
	        sizeof( st_USBKeyMap->sc_Encode ) );

	puc_CurrentPoint += sizeof( st_USBKeyMap->sc_Encode );

	// 文字コードオフセットアドレスの取得.
	OAPS_KeyMapOffsetTable st_CharCodeTableTmp[OAPE_USBKeyKind_Num];
	memset( st_CharCodeTableTmp, 0x00, sizeof( st_CharCodeTableTmp ) );
	OAPF_makeOffsetTable( puc_CurrentPoint,sizeof( st_CharCodeTableTmp )/sizeof( OAPS_KeyMapOffsetTable ),st_CharCodeTableTmp );

	// 文字コードデータ取得.
	for( uchar i = 0; i < ( uchar )( sizeof( st_CharCodeTableTmp )/sizeof( OAPS_KeyMapOffsetTable ) ); i++ ) {

		// 格納文字コード数.
		uchar uc_TableNum = (uchar)( st_CharCodeTableTmp[i].i32_Size / ( 18*sizeof( char ) ) );

		// 文字コードテーブル作成.
		OAPF_makeUSBCharCodeTable( uc_RAMBuffer + st_CharCodeTableTmp[i].i32_Addr,
		                      uc_TableNum,
		                      &( st_USBKeyMap->st_CodeData[i] ) );
	}
	st_USBKeyMap->valid = True;
	return True;
}

void OAPF_setKeyMapData( OAPS_KeyMap* s_KeyMap )
{
	pst_KeyMap = s_KeyMap;
}

void OAPF_loadKeyMapData()
{

//--------------------- Start setLangBuf.----------------------
	int iret;
	SHM_TYPE_DATA* p = NULL;
	uchar* puc_KeyMapStart = NULL;
	int32_t i32_RemainSize = 0;
	APIC_Enabler* pc_Enabler = APIC_Enabler::newInstance();

	// 共有メモリクラス取得
	DivCommon_Shm* p_DivCommon_Shm = DivCommon_Shm::newInstance();

	// アタッチ
	iret = p_DivCommon_Shm->Shm_attach( ( void* )&p );



	if( iret == 0 )
	{
		// 1ROM_9言語対応：常に全言語ロードする.
		// 共有メモリを使用.
		guc_OAPallDataRAM = p->e_allDataRAM;
		guc_OAP_selDataRAM = guc_OAPallDataRAM;
		guc_OAPextFontData = p->e_allDataFontRAM;
		if( True == pc_Enabler->isExtendedMemory() ) {
			// isAllocSuccessのフラグは、拡張メモリ有りの場合のみ True にする.
			isAllocSuccess = True;
		} else {
			// 【212_Myパネル機能の拡張メモリ不要化】.
			g_puc_OAPILangAreaPtr = guc_OAP_selDataRAM;
			g_puc_OapMyOAPILangAreaPtr = guc_OAP_selDataRAM + OAPD_SIZE_FILEDATA_SELECT;
		}

		// デタッチ
		iret = p_DivCommon_Shm->Shm_detattch( p );
	}
	else
	{
		// 1ROM_9言語対応：常に全言語ロードする.
#if !defined(FUM_LINUX)
		if( True == pc_Enabler->isExtendedMemory() ) {

#ifndef SIMULATOR
			guc_OAPallDataRAM = ( unsigned char* ) malloc( MEM_SIZE_OAPILang );
#else
			guc_OAPallDataRAM = ( unsigned char* )malloc( ( unsigned )( MEM_SIZE_OAPILang ) );
#endif 	
		}
#else
//Simの修正
		if( True == pc_Enabler->isExtendedMemory() ) {
			guc_OAPallDataRAM = guc_OAPallDataRAM_SIM;
		}
#endif // #if !defined(FUM_LINUX)

		if( NULL == guc_OAPallDataRAM ) {
#if !defined(FUM_LINUX)
#ifndef SIMULATOR
			// パネル言語展開領域格納.
			g_puc_OAPILangAreaPtr = ( unsigned char* ) malloc( OAPD_SIZE_FILEDATA_SELECT );
			SYSF_logMsg( "[setLangBuf]g_puc_OAPILangAreaPtr: %p\n", ( intptr_t )g_puc_OAPILangAreaPtr,2,3,4,5,6 );
			// OAPMyパネル言語展開領域格納.
			g_puc_OapMyOAPILangAreaPtr = ( uchar* )( malloc( OAPD_SIZE_FILEDATA_SELECT ) );
			SYSF_logMsg( "[setLangBuf]g_puc_OapMyOAPILangAreaPtr: %p\n", ( intptr_t )g_puc_OapMyOAPILangAreaPtr,2,3,4,5,6 );
			// OAPMyパネルフォントデータ展開領域格納.
			// 初期値をパネル言語展開領域のアドレスとする。
			guc_OAP_selDataRAM = g_puc_OAPILangAreaPtr;
			SYSF_logMsg( "[setLangBuf]guc_OAP_selDataRAM: %p\n", ( intptr_t )guc_OAP_selDataRAM,2,3,4,5,6 );
#else
			// パネル言語展開領域格納.
			g_puc_OAPILangAreaPtr = ( unsigned char* ) malloc( ( unsigned )OAPD_SIZE_FILEDATA_SELECT );
			SYSF_logMsg( "[setLangBuf]g_puc_OAPILangAreaPtr: %p\n", ( intptr_t )g_puc_OAPILangAreaPtr,2,3,4,5,6 );
			// OAPMyパネル言語展開領域格納.
			g_puc_OapMyOAPILangAreaPtr = ( uchar* )( malloc( ( unsigned )OAPD_SIZE_FILEDATA_SELECT ) ) );
			SYSF_logMsg( "[setLangBuf]g_puc_OapMyOAPILangAreaPtr: %p\n", ( intptr_t )g_puc_OapMyOAPILangAreaPtr,2,3,4,5,6 );
			// OAPMyパネルフォントデータ展開領域格納.
			// 初期値をパネル言語展開領域のアドレスとする。
			guc_OAP_selDataRAM = g_puc_OAPILangAreaPtr;
			SYSF_logMsg( "[setLangBuf]guc_OAP_selDataRAM: %p\n", ( intptr_t )guc_OAP_selDataRAM,2,3,4,5,6 );
#endif  // #ifndef SIMULATOR	// 2011/01/31 LocalUI分離 リンクエラー対応
#else
			guc_OAP_selDataRAM = guc_OAP_selDataRAM_Single_SIM;
#endif
		}
		else
		{
			// メモリパーティションを使用.
			guc_OAP_selDataRAM = guc_OAPallDataRAM;
			isAllocSuccess = True;
		}
	}
//--------------------- End setLangBuf.----------------------

	if( True == isAllocSuccess ) {
		// 9言語分のメモリ確保済み.
		// キーMAPデータ格納開始位置.
		// 拡張メモリありの場合、第1言語の終端に格納.
		puc_KeyMapStart = guc_OAPallDataRAM + ( OAPD_SIZE_FILEDATA_SELECT - SIZE_KEYBOARDDATA_SELECT );

		// メモリ残容量.
		i32_RemainSize = SIZE_KEYBOARDDATA_SELECT;

	} else {
		// 拡張メモリなしの場合、選択言語の終端に格納.
		puc_KeyMapStart = guc_OAP_selDataRAM + ( OAPD_SIZE_FILEDATA_SELECT - SIZE_KEYBOARDDATA_SELECT );
		// メモリ残容量.
		i32_RemainSize = SIZE_KEYBOARDDATA_SELECT;
	}

	if( ( NULL == guc_OAP_selDataRAM )
	    && ( NULL == guc_OAPallDataRAM ) ) {
		printf( "[KEYMAP]★NULL pointer  guc_OAP_selDataRAM:[%p],guc_OAPallDataRAM:[%p]\n",guc_OAP_selDataRAM,guc_OAPallDataRAM );
		return;
	}

	// 登録言語数取得.
	int32_t i32_LangCount = OAPF_getLangCount();
	int32_t i32_LangCountMSGBIN = c_SelectLangInfo.getMSGBINLangNum();				// メッセージバイナリ内の言語数.

	if( 0 >= i32_LangCount ) {
		printf( "[KEYMAP]★i32_LangCount error:[%d]\n",i32_LangCount );
		return;
	}

	int32_t	i32_langCount	 = c_SelectLangInfo.getMSGBINLangNum();
	// ソフトキーボードオフセットアドレステーブル先頭.
	int32_t	i32_fixedDataSize = 0;
	i32_fixedDataSize  = sizeof( unsigned char ) * SIZE_FIXED_DATA;
	i32_fixedDataSize = (int32_t)( i32_fixedDataSize + ( sizeof( unsigned char ) * OAPD_SIZE_LANGCODE * i32_langCount ) );

	const int32_t i32_OffsetTableSoftKey = (int32_t)( i32_fixedDataSize		// 固定エリア＋言語コード.
	                                   + ( OAPD_FONTSWITCH_SIZE * i32_LangCountMSGBIN )	 //フォントスイッチデータ.
	                                   + ( 2 * sizeof( int32_t ) * i32_LangCountMSGBIN )	// 言語データオフセットテーブル.
	                                   + ( 2 * sizeof( int32_t ) * i32_LangCountMSGBIN )	// フォントデータオフセットテーブル.
	                                   + ( 2 * sizeof( int32_t ) * i32_LangCountMSGBIN )	// 辞書データオフセットテーブル.
	                                   + ( 3 * 2 * sizeof( int32_t ) * i32_LangCountMSGBIN ) ); 	// Pattern BMPデータテーブル(パターン数は３固定).

	// 外付けキーボードオフセットアドレステーブル先頭.
	const int32_t i32_OffsetTableUSBKey = (int32_t)( i32_OffsetTableSoftKey + ( 2 * sizeof( int32_t ) * i32_LangCountMSGBIN ) );

	// -----------------------------------------------------------------.
	// ソフトキーボードオフセットアドレスの取得.
	// -----------------------------------------------------------------.
	OAPS_KeyMapOffsetTable st_SoftKeyTableTmp[OAP_MAX_SOFTKEYBOARDLANGUAGES];
	// 初期化.
	for( uchar i = 0; i < ( uchar )( sizeof( st_SoftKeyTableTmp )/sizeof( OAPS_KeyMapOffsetTable ) ); i++ ) {
		st_SoftKeyTableTmp[i].i32_Addr = -1;
		st_SoftKeyTableTmp[i].i32_Size = 0;
	}
	uchar uc_RAMBufferSoft[2 * OAP_MAX_SOFTKEYBOARDLANGUAGES * sizeof( int32_t )];
	memset( uc_RAMBufferSoft,0,sizeof( uc_RAMBufferSoft ) );

	// Flashからロード.
	if( 0 == OAPF_loadSelectLangKeymap( uc_RAMBufferSoft, ( uchar )( sizeof( uc_RAMBufferSoft ) ), i32_OffsetTableSoftKey, 2 * sizeof( int32_t ) ) ) {
		// オフセットテーブルへ設定.
		OAPF_makeOffsetTable( uc_RAMBufferSoft,(uchar)i32_LangCount,st_SoftKeyTableTmp );
	}
	// -----------------------------------------------------------------.

	// -----------------------------------------------------------------.
	// 外付けキーボードオフセットアドレスの取得.
	// -----------------------------------------------------------------.
	OAPS_KeyMapOffsetTable st_USBKeyTableTmp[OAP_MAX_USBKEYBOARDLANGUAGES];
	// 初期化.
	for( i = 0; i < ( uchar )( sizeof( st_USBKeyTableTmp )/sizeof( OAPS_KeyMapOffsetTable ) ); i++ ) {
		st_USBKeyTableTmp[i].i32_Addr = -1;
		st_USBKeyTableTmp[i].i32_Size = 0;
	}
	uchar uc_RAMBufferUSB[2 * OAP_MAX_USBKEYBOARDLANGUAGES * sizeof( int32_t )];
	memset( uc_RAMBufferUSB,0,sizeof( uc_RAMBufferUSB ) );

	// Flashからロード.
	if( 0 == OAPF_loadSelectLangKeymap( uc_RAMBufferUSB, ( uchar )( sizeof( uc_RAMBufferUSB ) ), i32_OffsetTableUSBKey, 2 * sizeof( int32_t ) ) ) {
		// オフセットテーブルへ設定.
		OAPF_makeOffsetTable( uc_RAMBufferUSB,(uchar)i32_LangCount,st_USBKeyTableTmp );
	}
	// -----------------------------------------------------------------.


	OAPS_KeyMap* st_KeyMapTmp = ( OAPS_KeyMap* )puc_KeyMapStart;

	memset( st_KeyMapTmp,0,sizeof( OAPS_KeyMap ) );

	// ソフトキーボードKeyMapデータのロード.
	for( i = 0; i < ( uchar )( sizeof( st_SoftKeyTableTmp )/sizeof( OAPS_KeyMapOffsetTable ) ); i++ ) {
		if( False == OAPF_loadSoftKeyMapData( st_SoftKeyTableTmp[i].i32_Addr,
		                                 st_SoftKeyTableTmp[i].i32_Size,
		                                 &( st_KeyMapTmp->st_SoftKey[i] ) ) ) {
			return;
		}
	}

	// 外付けキーボードKeyMapデータのロード.
	for( i = 0; i < ( uchar )( sizeof( st_USBKeyTableTmp )/sizeof( OAPS_KeyMapOffsetTable ) ); i++ ) {
		if( False == OAPF_loadUSBKeyMapData( st_USBKeyTableTmp[i].i32_Addr,
		                                st_USBKeyTableTmp[i].i32_Size,
		                                &( st_KeyMapTmp->st_USBKey[i] ) ) ) {
			return;
		}
	}

	// 表示言語情報取得.
	st_KeyMapTmp->ss_ViewLangNum = 0;
	for( i = 0; i < ( uchar )i32_LangCount; i++ ) {
		TYPE_Language e_VieLanguageTmp;
		if( True == OAPF_getViewLanguage( i,&e_VieLanguageTmp ) ) {
			st_KeyMapTmp->e_ViewLanguage[ st_KeyMapTmp->ss_ViewLangNum ] = e_VieLanguageTmp;
			st_KeyMapTmp->ss_ViewLangNum++;
		}
	}

	// キーMAPデータを設定.
	OAPF_setKeyMapData( st_KeyMapTmp );

}

const TYPE_Language tbl_OAP_USBKeyMapOKLanguage[] = {
	TYPE_LanguageAmerican,			// 米語.
	TYPE_LanguageFrench,			// フランス語.
	TYPE_LanguageItalian,			// イタリア語.
	TYPE_LanguageGerman,			// ドイツ語.
	TYPE_LanguageSpanish,			// スペイン語.
};

// 既存のソフトキーボード用キーMAP使用可能言語テーブル.
const TYPE_Language tbl_OAP_SoftKeyMapOKLanguage[] = {
	TYPE_LanguageJapanese,			// 日本語.
	TYPE_LanguageAmerican,			// 米語.
	TYPE_LanguageFrench,			// フランス語.
	TYPE_LanguageItalian,			// イタリア語.
	TYPE_LanguageGerman,			// ドイツ語.
	TYPE_LanguageSpanish,			// スペイン語.
	TYPE_LanguageChinese1,			// 広東語.
	TYPE_LanguageChinese2,			// 中国語.
	TYPE_LanguageHangul,			// ハングル語.
};

Bool OAPF_isUSBKeyMapOKLanguage( TYPE_Language e_Language )
{
	for( uchar i = 0; i < ( uchar )( sizeof( tbl_OAP_USBKeyMapOKLanguage )/sizeof( TYPE_Language ) ); i++ ) {
		if( e_Language == tbl_OAP_USBKeyMapOKLanguage[i] ) {
			return True;
		}
	}
	return False;
}

static	void	OAPF_sortKeyBoardLanguage( OAPS_KeyMap* s_KeyMap, TYPE_Language* e_Language )
{
	TYPE_Language	e_tmpLanguage[OAP_MAX_LANGUAGES];				// テンポラリ領域.
	TYPE_Language*	eViewLanguage;									// 表示言語格納用ポインタ.
	TYPE_Language	e_StandardViewLanguage[OAP_MAX_LANGUAGES] = {	// 標準９言語並び替え用.
		TYPE_LanguageAmerican,
		TYPE_LanguageFrench,
		TYPE_LanguageItalian,
		TYPE_LanguageGerman,
		TYPE_LanguageSpanish,
		TYPE_LanguageJapanese,
		TYPE_LanguageChinese2,
		TYPE_LanguageChinese1,
		TYPE_LanguageHangul,
	};
	uchar	ucIdx=0;			// テンポラリ領域のインデックス.

	// テンポラリ領域を"-1"で初期化.
	for( uchar i = 0; i < OAP_MAX_LANGUAGES; i++ ) {
		e_tmpLanguage[i] = -1;
	}
	// 並び替える前に登録されたデータに問題がないかチェック（並び替えで表示されなくなる）.
	uchar	ucLoop = ( uchar )( sizeof( e_Language )/sizeof( TYPE_Language ) );
	for( i = 0; i < ucLoop; i++ ) {
		for( uchar j = 0; j < ( uchar )( s_KeyMap->ss_ViewLangNum ); j++ ) {
			if( s_KeyMap->e_ViewLanguage[j] == e_Language[i] ) { break; }
		}
		if( ( j>=( uchar )( s_KeyMap->ss_ViewLangNum ) )
		    && ( e_Language[i] != -1 ) ) {
			printf( "★Illegal KeyBoard KeyMap Lang:%d\n",e_Language[i] );
		}
	}
	// 表示言語データを設定.
	if( False == OAPF_isStandardMSGBinary() ) {
		eViewLanguage = s_KeyMap->e_ViewLanguage;		// 標準９言語以外は、MSGバイナリの格納順に並び替える.
	} else {
		eViewLanguage = e_StandardViewLanguage;			// 標準９言語の場合、固定の順番に並び替える.
	}
	// 表示言語の順番通りにテンポラリ領域に格納する.
	for( i = 0; i < s_KeyMap->ss_ViewLangNum; i++ ) {
		for( uchar j = 0; j < OAP_MAX_LANGUAGES; j++ ) {
			if( -1 == e_Language[j] ) { break; }
			if( eViewLanguage[i] == e_Language[j] ) {
				e_tmpLanguage[ucIdx] = e_Language[j];
				ucIdx++;
				break;
			}
		}
	}
	// ソート結果を格納する.
	memcpy( e_Language, e_tmpLanguage, sizeof( e_tmpLanguage ) );
}

void OAPF_getUSBKeyBoardLoadLanguage( TYPE_Language* e_Language )
{
	if( NULL == e_Language ) { return; }

	if( True == b_OAP_USBLoadOK ) {
		// 生成済みのデータを使用する.
		memcpy( e_Language,e_OAP_USBLoadLanguage,sizeof( e_OAP_USBLoadLanguage ) );
		return;
	}
	// キーマップなしの場合.
	if( NULL == pst_KeyMap ) {
		// 英語のみを暫定的に救済する(USB起動対応).
		OAPF_addUSBLoadLanguage( TYPE_LanguageAmerican );
	} else {
		// キーMAPに登録されている言語を追加.
		for( uchar i = 0; i < ( uchar )( sizeof( pst_KeyMap->st_USBKey )/sizeof( OAPS_USBKeyMap ) ); i++ ) {
			if( False == pst_KeyMap->st_USBKey[i].valid ) { continue; }
			if( TYPE_LanguageJapanese == pst_KeyMap->st_USBKey[i].e_Language && NULL == pst_KeyMap->st_USBKey[i].sc_Encode )
			{
				continue;
			}

			OAPF_addUSBLoadLanguage( pst_KeyMap->st_USBKey[i].e_Language );
		}

		// パネル表示言語に登録されている言語を追加.
		for( i = 0; i < ( uchar )( pst_KeyMap->ss_ViewLangNum ); i++ ) {
			if( True == OAPF_isUSBKeyMapOKLanguage( pst_KeyMap->e_ViewLanguage[i] ) ) {
				OAPF_addUSBLoadLanguage( pst_KeyMap->e_ViewLanguage[i] );
			}
		}
		OAPF_sortKeyBoardLanguage( pst_KeyMap, e_OAP_USBLoadLanguage );		// 表示順を表示言語の順番にソートする.
	}
	// 追加結果を返す.
	memcpy( e_Language,e_OAP_USBLoadLanguage,sizeof( e_OAP_USBLoadLanguage ) );

	b_OAP_USBLoadOK = True;
}

////----------------------------end        USB-------------------------

void OAPF_addSoftLoadLanguage( TYPE_Language e_Language )
{
	for( uchar i = 0; i < ( uchar )( sizeof( e_OAP_SoftLoadLanguage )/sizeof( TYPE_Language ) ); i++ ) {
		if( e_Language == e_OAP_SoftLoadLanguage[i] ) {
			// 既に存在するので追加しない.
			return;
		}

		if( -1 == e_OAP_SoftLoadLanguage[i] ) {
			// 空き領域まで来たので追加.
			e_OAP_SoftLoadLanguage[i] = e_Language;
			return;
		}
	}
}

Bool OAPF_isSoftKeyMapOKLanguage( TYPE_Language e_Language )
{
	for( uchar i = 0; i < ( uchar )( sizeof( tbl_OAP_SoftKeyMapOKLanguage )/sizeof( TYPE_Language ) ); i++ ) {
		if( e_Language == tbl_OAP_SoftKeyMapOKLanguage[i] ) {
			return True;
		}
	}
	return False;
}
void OAPF_getSoftKeyBoardLoadLanguage( TYPE_Language* e_Language )
{
	if( NULL == e_Language ) { return; }

	if( True == b_OAP_SoftLoadOK ) {
		// 生成済みのデータを使用する.
		memcpy( e_Language,e_OAP_SoftLoadLanguage,sizeof( e_OAP_SoftLoadLanguage ) );
		return;
	}
	// キーマップなしの場合.
	if( NULL == pst_KeyMap ) {
		// 日本語/英語のみを暫定的に救済する(USB起動対応).
		OAPF_addSoftLoadLanguage( TYPE_LanguageJapanese );
		OAPF_addSoftLoadLanguage( TYPE_LanguageAmerican );
	} else {
		// キーMAPに登録されている言語を追加.
		for( uchar i = 0; i < ( uchar )( sizeof( pst_KeyMap->st_SoftKey )/sizeof( OAPS_SoftKeyMap ) ); i++ ) {
			if( False == pst_KeyMap->st_SoftKey[i].valid ) { continue; }

			OAPF_addSoftLoadLanguage( pst_KeyMap->st_SoftKey[i].e_Language );
		}

		// パネル表示言語に登録されている言語を追加.
		for( i = 0; i < ( uchar )( pst_KeyMap->ss_ViewLangNum ); i++ ) {
			if( True == OAPF_isSoftKeyMapOKLanguage( pst_KeyMap->e_ViewLanguage[i] ) ) {
				OAPF_addSoftLoadLanguage( pst_KeyMap->e_ViewLanguage[i] );
			}
		}
		OAPF_sortKeyBoardLanguage( pst_KeyMap, e_OAP_SoftLoadLanguage );		// 表示順を表示言語の順番にソートする.
	}
	// 追加結果を返す.
	memcpy( e_Language,e_OAP_SoftLoadLanguage,sizeof( e_OAP_SoftLoadLanguage ) );

	b_OAP_SoftLoadOK = True;
}
////--------------------------end key board---------------------------------------

void OAPF_KeyBoardLanguageSetComstartGamen()
{
	OAPF_LanguageStartGamen();
	
	int32_t i32_LanguageCount =	OAPF_getLangCount();
	TYPE_Language** pst_lang_table  = m_pstOAP_LangTable; 
	if( NULL == pst_lang_table ) {
		return;
	}
	
		for(int i = 0; i < OAP_MAX_LANGUAGES; i++ ) {
		if( -1 == e_OAP_LoadLanguage[i] ) {
			break;
		}
		for(int j = 0; j < i32_LanguageCount ; j++ ) {
			if ( NULL == pst_lang_table[j] )
			{
				printf("[%d:%s] QuanPH5 [debugNo.34] \n", __LINE__, __FUNCTION__);
				continue;
			}
			if( e_OAP_LoadLanguage[i] == *pst_lang_table[j] ) {
				m_pstOAP_langTable[i] = pst_lang_table[j];
////				sl_LangNum++;		///??????
				break;
			}
		}
	}
}

void OAPF_USBKeyBoardLanguageSetStartGamen()
{
	int i;

	// 配列を初期化.
	for( i = 0; i < OAP_MAX_USBKEYBOARDLANGUAGES; i++ ) {
		e_OAP_LoadLanguage[ i ] = -1;
		if ( True != b_OAP_USBLoadOK)
		{
			e_OAP_USBLoadLanguage[ i ] = -1;
		}
	}

	
	OAPF_getUSBKeyBoardLoadLanguage(e_OAP_LoadLanguage);
	OAPF_KeyBoardLanguageSetComstartGamen();
}


Bool OAPF_KeyBoardOAPF_LanguageStartGamen()
{
	int i;

	// 配列を初期化.
	for( i = 0; i < OAP_MAX_USBKEYBOARDLANGUAGES; i++ ) {
		e_OAP_LoadLanguage[ i ] = -1;
		if( True != b_OAP_SoftLoadOK)
		{
			e_OAP_SoftLoadLanguage[ i ] = -1;
		}
	}

	// 選択中言語を取得.
	OAPF_getSoftKeyBoardLoadLanguage( e_OAP_LoadLanguage );

	OAPF_KeyBoardLanguageSetComstartGamen();
	return true;
}

Bool OAPF_makeOffsetTable( const uchar* puc_Start,uchar sc_Num,OAPS_KeyMapOffsetTable* st_OffsetTable )
{
	if( ( NULL == puc_Start ) || ( NULL == st_OffsetTable ) ) {
		printf( "[KEYMAP]★NULL pointer  puc_Start:[%p],st_OffsetTable:[%p]\n",puc_Start,st_OffsetTable );
		return False;
	}
	const uchar* puc_Current = puc_Start;

	for( uchar i = 0; i < sc_Num; i++ ) {
		// アドレス.
		memcpy( &( st_OffsetTable[i].i32_Addr ),puc_Current,sizeof( st_OffsetTable[i].i32_Addr ) );
		puc_Current += sizeof( st_OffsetTable[i].i32_Addr );

		// サイズ.
		memcpy( &( st_OffsetTable[i].i32_Size ),puc_Current,sizeof( st_OffsetTable[i].i32_Size ) );
		st_OffsetTable[i].i32_Size = OAPD_S32_WORD_SIZE( st_OffsetTable[i].i32_Size );	// サイズデータを格納する際には奇数バイトのデータを偶数に丸める.
		puc_Current += sizeof( st_OffsetTable[i].i32_Size );
	}

	return True;
}
// [IT6_2.2MR2] 制御構成変更_No.34  2022/01/18 FPT-QuanPH5 Add End.

// =========================================================================== .
// .
// 実現部.
// .
// =========================================================================== .

void OAPC_UtilityItem::activate()
{
	// NOP.
}

// ---------------------------------------------------------------------------.
// ItemGenerator.
// ---------------------------------------------------------------------------.
template<OAPE_UtilitySetting_RequestItem T>
struct ItemGenerator
{
	static OAPC_UtilityItem* generate(OAPE_UtilitySetting_RequestItem e_item)
	{
		return (e_item==T) ? new OAPC_UtilityItemImp<T>() : 0;
	}
};

// ---------------------------------------------------------------------------.
// ItemManager.
// ---------------------------------------------------------------------------.
class __OAPC_UtilityItemManager : public OAPC_NonThreadSafe
{
private:
	typedef	OAPC_UtilityItem* (*GENE)(OAPE_UtilitySetting_RequestItem e_item);

	static GENE tblRequestGenerator[];

public:
	__OAPC_UtilityItemManager()
	{}
	~__OAPC_UtilityItemManager()
	{}

	OAPC_UtilityItem* _getItem(OAPE_UtilitySetting_RequestItem e_item)		// アイテム取得.
	{
		for (uint32_t i=0; tblRequestGenerator[i] != 0; i++) {
			OAPC_UtilityItem* ret = (*tblRequestGenerator[i])(e_item);
			if (ret) return ret;
		}
		return 0;
	}
};
__OAPC_UtilityItemManager :: GENE __OAPC_UtilityItemManager :: tblRequestGenerator[] =
{
	ItemGenerator<OAPE_UtilitySetting_RequestItem_Environment>::generate,
// MinervaZeus3rd UV-01309 Muratani 2013/03/18 Start.
	ItemGenerator<OAPE_UtilitySetting_RequestItem_Copy>::generate,
	ItemGenerator<OAPE_UtilitySetting_RequestItem_FaxScan>::generate,
	ItemGenerator<OAPE_UtilitySetting_RequestItem_Fax>::generate,		// ZeusS UV-01439 HCL-Pradeep 2014/08/18 Added.
// MinervaZeus3rd UV-01309 Muratani 2013/03/18 End.
//2017/03/15 スキャン設定_XML基盤技術開発v2 pactera add start.
	ItemGenerator<OAPE_UtilitySetting_RequestItem_Scan>::generate,
//2017/03/15 スキャン設定_XML基盤技術開発v2 pactera add end.
	ItemGenerator<OAPE_UtilitySetting_RequestItem_Network>::generate,	// ZeusS_ZX0 RQ-320対応 2015/7/15 1OPS Ohno Added. 
	ItemGenerator<OAPE_UtilitySetting_RequestItem_NetworkTuning>::generate,	//  IT5_3.3 制御構成変更_No.16 HCL-Sahana 2016/08/02 Added.
	0,
};

OAPC_UtilityItemManager::OAPC_UtilityItemManager() :
_pc_Rep(new __OAPC_UtilityItemManager())
{}

OAPD_COPY_OPERATE(OAPC_UtilityItemManager)

OAPC_UtilityItem* OAPC_UtilityItemManager::getItem(OAPE_UtilitySetting_RequestItem req) const
{
	if (_pc_Rep) { return _pc_Rep->_getItem(req); }
	return 0;
}

OAPC_UtilityItemManager& OAPC_UtilityItemManager::getInstance()
{
	if (!_pc_theInstance) _pc_theInstance = new OAPC_UtilityItemManager();
	return *_pc_theInstance;
}

OAPC_UtilityItemManager* OAPC_UtilityItemManager::_pc_theInstance = 0;

// ---------------------------------------------------------------------------.
// TransactImp <AppReqSetUtilitySetting)>.
// ---------------------------------------------------------------------------.
template<> class __OAPC_TransactImp<OAPS_AppReqSetUtilitySetting> :
	public OAPC_TransactImpBaseInquiry< OAPS_AppReqSetUtilitySetting , OAPE_Result_FaultRequest_AppReqSetUtilitySetting > 
{
friend class OAPC_TransactImp<OAPS_AppReqSetUtilitySetting>;
private:
	OAPC_AuthManager _authmanager;

	__OAPC_TransactImp() {}
	~__OAPC_TransactImp() {}
	
	inline OAPE_Response _response() const;
	inline bool _pre_check(const OAPC_Memory& msg, OAPC_Error& e);
	inline bool _authorize(const OAPC_Memory& msg, OAPC_Error& e);
	inline bool _execute(const OAPC_Memory& msg, OAPC_Memory& retres, OAPC_Error& e);
};

// ---------------------------------------------------------------------------.
// TransactImp <AppReqGetUtilitySetting>.
// ---------------------------------------------------------------------------.
template<> class __OAPC_TransactImp<OAPS_AppReqGetUtilitySetting> :
	public OAPC_TransactImpBaseInquiry< OAPS_AppReqGetUtilitySetting , OAPE_Result_FaultRequest_AppReqGetUtilitySetting > 
{
friend class OAPC_TransactImp<OAPS_AppReqGetUtilitySetting>;
private:
	OAPC_AuthManager _authmanager;

	__OAPC_TransactImp() {}
	~__OAPC_TransactImp() {}
	
	inline OAPE_Response _response() const;
	inline bool _pre_check(const OAPC_Memory& msg, OAPC_Error& e);
	inline bool _authorize(const OAPC_Memory& msg, OAPC_Error& e);
	inline bool _execute(const OAPC_Memory& msg, OAPC_Memory& retres, OAPC_Error& e);
	inline bool _preparation(const OAPC_Memory& msg, OAPC_Memory& retres, int iRequestItem, OAPC_Error& e);
	inline bool _post_process(const OAPC_Memory& msg, OAPC_Memory& retres, int iRequestItem, OAPC_Error& e);
};


// =========================================================================== .
// .
// トランザクション(マクロ) <AppReqSetUtilitySetting>.
// .
// =========================================================================== .
OAPD_TRANSACT_HANDLING(AppReqSetUtilitySetting)

OAPD_BEGIN_TRANSACT_PRECHECK(AppReqSetUtilitySetting)

	OAPD_TRACE_SCOPE("OAPD_BEGIN_TRANSACT_PRECHECK(AppReqSetUtilitySetting)")

	// OperatorInfo.
	if(p_req->s_OperatorInfo.empty())
	{
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralTagMissing, "OperatorInfo");
		return false;
	}

	// RequestItem.
	if(p_req->e_RequestItem.empty())
	{
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralTagMissing, "RequestItem");
		return false;
	}
	
	//2017/03/15 スキャン設定_XML基盤技術開発v2 pactera add start.
	// LockKey 
	if ( ( DEVICE_LOCKKEY_PASS_CHECK( p_req, _slock, e, true ) == false )
		//2017/09/01 LockKey変更_XML基盤技術開発v2  Pactera Added Start.
		&& OAPE_RequestDeviceType_PSWC != OAPC_AccountMan::getInstance()->findDeviceType(p_req->s_OperatorInfo.sc_AuthKey.get()) )
		//2017/09/01 LockKey変更_XML基盤技術開発v2  Pactera Added End.
	{
		return false;
	}
	//2017/03/15 スキャン設定_XML基盤技術開発v2 pactera add end.

// MinervaZeus3rd UV-01309 Muratani 2013/03/18 Start.
	OAPC_UtilityItemManager& resmanager = OAPC_UtilityItemManager::getInstance();
	::auto_ptr<OAPC_UtilityItem> resproc(resmanager.getItem(p_req->e_RequestItem));

	if (!resproc.get()) {
		e.setKind(OAPE_Fault_faultcode_Client);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull, "no memory fatal");
		return false;
	}
	// ZeusCD CQ383966 Gayathri 2013/08/29 Added Start. 
	// ExportImport時は個別プレチェックを行わない. 
	if( True == checkExpImpAccountCheck( p_req->s_OperatorInfo.sc_AuthKey.get() ) ){
		return true;
	}
	// ZeusCD CQ383966 Gayathri 2013/08/29 Added End. 
	if(!resproc->pre_check(p_req, e))
	{
		return false;
	}
// MinervaZeus3rd UV-01309 Muratani 2013/03/18 End.

OAPD_END_TRANSACT_PRECHECK

OAPD_BEGIN_TRANSACT_AUTHORIZE(AppReqSetUtilitySetting)

	OAPD_TRACE_SCOPE("OAPD_BEGIN_TRANSACT_AUTHORIZE(AppReqSetUtilitySetting)")

	// 外部に出しているメッセージではないためExp-Imp特殊ユーザーのみ受け付ける.
	OAPS_AccountUser accountuser;
	if( OAPC_AccountMan::getInstance()->findAccount( p_req->s_OperatorInfo.sc_AuthKey.get(), accountuser ) )
	{
		// WebUIからのSingleSignOn時はチェックしない.
		if(False == OAPC_AccountMan::getInstance()->getIsWebUIFromPanel(p_req->s_OperatorInfo.sc_AuthKey.get())){
// HeliosFUM(MAv2) UV-01378 SMBでのデータExportImportの改善 日立 芳賀 2013/10/21 修正Start .
			// Exp-Imp特殊ユーザーでない、かつ、CEでない場合 .
			if ((True != accountuser.b_ExpImp) && (OAPE_UserType_CE != accountuser.e_UserType) && (OAPE_UserType_Admin != accountuser.e_UserType)) // ZeusS_ZX0 RQ-298 2015/07/31 HCL-Sahana Modified.
// HeliosFUM(MAv2) UV-01378 SMBでのデータExportImportの改善 日立 芳賀 2013/10/21 修正End .
			{
				e.setCode(OAPE_Result_ErrorDetails_AuthNotAllowedAuthSetting);
				e.setDescription("you cannot operate by your authority");
				return false;
			}
		}
	}
	else
	{
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull, "AccountMan");
		return false;
	}

OAPD_END_TRANSACT_AUTHORIZE

OAPD_BEGIN_TRANSACT_EXECUTE(AppReqSetUtilitySetting, AppResSetUtilitySetting)

	OAPD_TRACE_SCOPE("OAPD_BEGIN_TRANSACT_EXECUTE(AppReqSetUtilitySetting, AppResSetUtilitySetting)")

	OAPC_UtilityItemManager& resmanager = OAPC_UtilityItemManager::getInstance();
	::auto_ptr<OAPC_UtilityItem> resproc(resmanager.getItem(p_req->e_RequestItem));

	//[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified Start.
	OAPC_ApiWrapper<APIC_SystemInfo> pc_SystemInfo(APIC_SystemInfo::newInstanceAdmin() );
	if( !pc_SystemInfo ) {
		 e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Cannot Get Instance" );
		return false;
	}
	//[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified End.
	if(!resproc.get())
	{
		e.setKind(OAPE_Fault_faultcode_Client);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull, "no memory fatal");
		return false;
	}

	//[IT6_SPARROW]WP_No.206-1 2018/03/16 Hcl-Neeraj Modified Start.
	if ((OAPE_UtilitySetting_RequestItem_FaxScan == p_req->e_RequestItem) || (OAPE_UtilitySetting_RequestItem_Fax == p_req->e_RequestItem))
	{
		//TYPE_MachineType e_MachineType = pc_SystemInfo->getMachineType();
		if(!(OAPI_CheckIsSparrowSFPModel())) //IT6_SparrowBK 制御構成変更_No.36 HCL-Palani 2019/08/06 Modified.
		{
			p_res->e_RequestItem = p_req->e_RequestItem;
		}
		else{
			if( False == checkExpImpAccountCheck( p_req->s_OperatorInfo.sc_AuthKey.get() ) ){
				e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralIllegalValue, "RequestItem");
				return false;
			}
		}
	}
	else{
		p_res->e_RequestItem = p_req->e_RequestItem;
	}
	//[IT6_SPARROW]WP_No.206-1 2018/03/16 Hcl-Neeraj Modified End.

	// 結果通知共通構造体作成.
	p_res->s_Result.set();

	if(!resproc->set(p_req, _slock, e))
	{
		return false;
	}

OAPD_END_TRANSACT_EXECUTE


// =========================================================================== .
// .
// トランザクション(マクロ) <AppReqGetUtilitySetting>.
// .
// =========================================================================== .
OAPD_TRANSACT_HANDLING_CUSTOMDATA(AppReqGetUtilitySetting)

// パラメータ設定.
OAPD_BEGIN_TRANSACT_PREPARATION(AppReqGetUtilitySetting)
	switch(iRequestItem) {
		case OAPE_CustomData_RequestItem_Environment:
			req->e_RequestItem = OAPE_UtilitySetting_RequestItem_Environment;
			break;

		case OAPE_CustomData_RequestItem_Copy:
			req->e_RequestItem = OAPE_UtilitySetting_RequestItem_Copy;
			break;

		case OAPE_CustomData_RequestItem_FaxScan:
			req->e_RequestItem = OAPE_UtilitySetting_RequestItem_FaxScan;
			break;

		case OAPE_CustomData_RequestItem_Fax:
			req->e_RequestItem = OAPE_UtilitySetting_RequestItem_Fax;
			break;

		case OAPE_CustomData_RequestItem_Network:
			req->e_RequestItem = OAPE_UtilitySetting_RequestItem_Network;
			break;

		case OAPE_CustomData_RequestItem_NetworkTuning:
			req->e_RequestItem = OAPE_UtilitySetting_RequestItem_NetworkTuning;
			break;
		case OAPE_CustomData_RequestItem_Scan:
			req->e_RequestItem = OAPE_UtilitySetting_RequestItem_Scan;
			break;
		default:
			return false;
	}

	if( !p_req->s_OperatorInfo.empty()) {
		req->s_OperatorInfo = p_req->s_OperatorInfo;
	}

	// IT6_Eagle機種開発_RQ-698_2portLAN対応 Pactera add start 2018/06/28
	switch( iRequestItem ){
		case OAPE_CustomData_RequestItem_NetworkTuning:
			if( !p_req->e_NetworkType.empty() ){
				req->e_NetworkType = p_req->e_NetworkType;
			}
			break;

		default:
			break;
	}
	// IT6_Eagle機種開発_RQ-698_2portLAN対応 Pactera add end 2018/06/28

	retres = p_param;

OAPD_END_TRANSACT_PREPARATION


OAPD_BEGIN_TRANSACT_PRECHECK(AppReqGetUtilitySetting)

	OAPD_TRACE_SCOPE("OAPD_BEGIN_TRANSACT_PRECHECK(AppReqGetUtilitySetting)")

	// OperatorInfo.
	if(p_req->s_OperatorInfo.empty())
	{
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralTagMissing, "OperatorInfo");
		return false;
	}

	// RequestItem.
	if(p_req->e_RequestItem.empty())
	{
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralTagMissing, "RequestItem");
		return false;
	}

OAPD_END_TRANSACT_PRECHECK

OAPD_BEGIN_TRANSACT_AUTHORIZE(AppReqGetUtilitySetting)

	OAPD_TRACE_SCOPE("OAPD_BEGIN_TRANSACT_AUTHORIZE(AppReqGetUtilitySetting)")

	// 外部に出しているメッセージではないためExp-Imp特殊ユーザーのみ受け付ける.
	OAPS_AccountUser accountuser;
	if( OAPC_AccountMan::getInstance()->findAccount( p_req->s_OperatorInfo.sc_AuthKey.get(), accountuser ) )
	{
// HeliosFUM(MAv2) UV-01378 SMBでのデータExportImportの改善 日立 芳賀 2013/10/21 修正Start .
		// Exp-Imp特殊ユーザーでない、かつ、CEでない場合 .
		if ( !OAPD_VERSION_CHECK(p_req->getVersion(), OAPD_OVER_V999) ) {	//2017/07/06 COMMONAPI_XML基盤技術開発v2 pactera Moded.
			if ((True != accountuser.b_ExpImp) && (OAPE_UserType_CE != accountuser.e_UserType) && (OAPE_UserType_Admin != accountuser.e_UserType)) // ZeusS_ZX0 RQ-298 2015/07/31 HCL-Sahana Modified.
// HeliosFUM(MAv2) UV-01378 SMBでのデータExportImportの改善 日立 芳賀 2013/10/21 修正End .
			{
				e.setCode(OAPE_Result_ErrorDetails_AuthNotAllowedAuthSetting);
				e.setDescription("you cannot operate by your authority");
				return false;
			}
		}
	}
	else
	{
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull, "AccountMan");
		return false;
	}

OAPD_END_TRANSACT_AUTHORIZE

// MAv4 CQ529919 2016/06/10 mod start.
OAPD_BEGIN_TRANSACT_EXECUTE_JOB_RUNNING(AppReqGetUtilitySetting, AppResGetUtilitySetting)

	OAPD_TRACE_SCOPE("OAPD_BEGIN_TRANSACT_EXECUTE_JOB_RUNNING(AppReqGetUtilitySetting, AppResGetUtilitySetting)")
// MAv4 CQ529919 2016/06/10 mod end.

	OAPC_UtilityItemManager& resmanager = OAPC_UtilityItemManager::getInstance();
	::auto_ptr<OAPC_UtilityItem> resproc(resmanager.getItem(p_req->e_RequestItem));

	//[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified Start.
	OAPC_ApiWrapper<APIC_SystemInfo> pc_SystemInfo(APIC_SystemInfo::newInstanceAdmin() );
	if( !pc_SystemInfo ) {
		 e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Cannot Get Instance" );
		return false;
	}
	//[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified End.
	if(!resproc.get())
	{
		e.setKind(OAPE_Fault_faultcode_Client);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull, "no memory fatal");
		return false;
	}

	//[IT6_SPARROW]WP_No.206-1 2018/03/16 Hcl-Neeraj Modified Start.
	if ((OAPE_UtilitySetting_RequestItem_FaxScan == p_req->e_RequestItem.get()) || (OAPE_UtilitySetting_RequestItem_Fax == p_req->e_RequestItem.get()))
	{
		//TYPE_MachineType e_MachineType = pc_SystemInfo->getMachineType();
		if(!(OAPI_CheckIsSparrowSFPModel())) //IT6_SparrowBK 制御構成変更_No.36 HCL-Palani 2019/08/06 Modified.
		{
			p_res->e_RequestItem = p_req->e_RequestItem;
		}
		else{
			if( False == checkExpImpAccountCheck( p_req->s_OperatorInfo.sc_AuthKey.get() ) ){
				e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralIllegalValue, "RequestItem");
				return false;
			}
		}
	}
	else{
			p_res->e_RequestItem = p_req->e_RequestItem;
	}
	//[IT6_SPARROW]WP_No.206-1 2018/03/16 Hcl-Neeraj Modified End.

	// 結果通知共通構造体作成.
	p_res->s_Result.set();

	if(!resproc->get(p_req, p_res, _slock, e))
	{
		return false;
	}

OAPD_END_TRANSACT_EXECUTE

// 応答データ設定.
OAPD_BEGIN_TRANSACT_POSTPROCESS(AppReqGetUtilitySetting, AppResGetUtilitySetting)

	switch(iRequestItem) {
		case OAPE_CustomData_RequestItem_Environment:
			p_res->s_CustomData.s_Environment = res->s_Environment;
			break;

		case OAPE_CustomData_RequestItem_Copy:
			p_res->s_CustomData.s_Copy2 = res->s_Copy;
			break;

		case OAPE_CustomData_RequestItem_FaxScan:
			p_res->s_CustomData.s_FaxScan = res->s_FaxScan;
			break;

		case OAPE_CustomData_RequestItem_Fax:
			p_res->s_CustomData.s_Fax3 = res->s_Fax;
			break;

		case OAPE_CustomData_RequestItem_Network:
			p_res->s_CustomData.s_Network2 = res->s_Network;
			break;

		case OAPE_CustomData_RequestItem_NetworkTuning:
			p_res->s_CustomData.s_NetworkTuning = res->s_NetworkTuning;
			break;
		
		case OAPE_CustomData_RequestItem_Scan:
			p_res->s_CustomData.s_Scan2 = res->s_Scan;
			break;
		default:
			return false;
	}

OAPD_END_TRANSACT_POSTPROCESS

// =========================================================================== .
// .
// トランザクション(実処理部).
// .
// =========================================================================== .
// =========================================================================== .
// Static Function / Inner Class Define.
// =========================================================================== .

// MinervaZeusInt CQ379190 h-kinoshita 2013/06/17 Start.
// =========================================================================== .
// ユーティリティ設定(RequestItem=Copy) 登録倍率設定/取得関数ポインタ.
// =========================================================================== .
typedef void (APIC_OperationSetting::*SetMagMemoryFunc)( const TYPC_Point<float> );
typedef TYPC_Point<float> (APIC_OperationSetting::*GetMagMemoryFunc)();

static const SetMagMemoryFunc tblSetMagMemoryFunc[OAPD_MAGMEMORYLIST_MAXLIST] = {
	&APIC_OperationSetting::setMagMemory01,
	&APIC_OperationSetting::setMagMemory02,
	&APIC_OperationSetting::setMagMemory03,
};

static const GetMagMemoryFunc tblGetMagMemoryFunc[OAPD_MAGMEMORYLIST_MAXLIST] ={
	&APIC_OperationSetting::getMagMemory01,
	&APIC_OperationSetting::getMagMemory02,
	&APIC_OperationSetting::getMagMemory03,
};
// MinervaZeusInt CQ379190 h-kinoshita 2013/06/17 End.


// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=Environment).
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Environment>::OAPC_UtilityItemImp()
{
	activate();
}
template<> OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Environment>::~OAPC_UtilityItemImp()
{}

// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=Environment) pre_check.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Environment>::pre_check(const OAPS_AppReqSetUtilitySetting* p_req, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=Environment)::pre_check()" );

	e.setMessage(OAPE_Result_FaultRequest_AppReqSetUtilitySetting);
// 2020/02/05 【Eagle Z+】OP_BTS-11760 HIENG Modified Start.
// 2020/02/19 【Eagle Z+】OP_BTS-20715(OP_BTS-11760) HIENG Modified Start.
	if( OAPD_VERSION_CHECK(p_req->getVersion(), OAPD_OVER_V77) ) {
// 2020/02/19 【Eagle Z+】OP_BTS-20715(OP_BTS-11760) HIENG Modified End.
// 2019/12/18 【Eagle Z+】OP_BTS-11760 HIENG Start.
// 2019/11/21 【HeliosMLK】 MachineNo.11 HIENG Start.
// #ifndef DEF_HeliosMLKCus
#if !defined (DEF_DenebMLKCus) && !defined (DEF_SparrowCus) && !defined (DEF_HeliosMLKCus)
	// 給紙トレイ設定(F1:上段).
	const OAPS_FreeSizePaper& s_FreeSizePaper01 = p_req->s_Environment.s_PaperTraySetting.s_PostInserter.s_UpperTray.s_PaperSize.s_FreeSizePaper01;
		if (!s_FreeSizePaper01.f_X.empty() && !s_FreeSizePaper01.f_X.range(139.7, 297.0))	// 2018/02/13 Pactera 修正.
		{
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralRangeIllegal, "FreeSizePaper01::X");
			return false;
		}
		if (!s_FreeSizePaper01.f_Y.empty() && !s_FreeSizePaper01.f_Y.range(182.0, 320.0))	// 2018/02/13 Pactera 修正.
		{
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralRangeIllegal, "FreeSizePaper01::Y");
			return false;
		}

	const OAPS_FreeSizePaper& s_FreeSizePaper02 = p_req->s_Environment.s_PaperTraySetting.s_PostInserter.s_UpperTray.s_PaperSize.s_FreeSizePaper02;
		if (!s_FreeSizePaper02.f_X.empty() && !s_FreeSizePaper02.f_X.range(139.7, 297.0))	// 2018/02/13 Pactera 修正.
		{
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralRangeIllegal, "FreeSizePaper02::X");
			return false;
		}
		if (!s_FreeSizePaper02.f_Y.empty() && !s_FreeSizePaper02.f_Y.range(182.0, 320.0))	// 2018/02/13 Pactera 修正.
		{
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralRangeIllegal, "FreeSizePaper02::Y");
			return false;
		}

	// 給紙トレイ設定(F2:下段).
	const OAPS_FreeSizePaper& s_FreeSizePaper1 = p_req->s_Environment.s_PaperTraySetting.s_PostInserter.s_LowerTray.s_PaperSize.s_FreeSizePaper01;
		if (!s_FreeSizePaper1.f_X.empty() && !s_FreeSizePaper1.f_X.range(139.7, 457.2))		// 2018/02/13 Pactera 修正.
		{
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralRangeIllegal, "FreeSizePaper01::X");
			return false;
		}
		if (!s_FreeSizePaper1.f_Y.empty() && !s_FreeSizePaper1.f_Y.range(182.0, 320.0))		// 2018/02/13 Pactera 修正.
		{
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralRangeIllegal, "FreeSizePaper01::Y");
			return false;
		}

	const OAPS_FreeSizePaper& s_FreeSizePaper2 = p_req->s_Environment.s_PaperTraySetting.s_PostInserter.s_LowerTray.s_PaperSize.s_FreeSizePaper02;
		if (!s_FreeSizePaper2.f_X.empty() && !s_FreeSizePaper2.f_X.range(139.7, 457.2))		// 2018/02/13 Pactera 修正.
		{
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralRangeIllegal, "FreeSizePaper02::X");
			return false;
		}
		if (!s_FreeSizePaper2.f_Y.empty() && !s_FreeSizePaper2.f_Y.range(182.0, 320.0))		// 2018/02/13 Pactera 修正.
		{
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralRangeIllegal, "FreeSizePaper02::Y");
			return false;
		}
#endif
// 2019/11/21 【HeliosMLK】 MachineNo.11 HIENG End.
// 2019/12/18 【Eagle Z+】OP_BTS-11760 HIENG End.
	} else {
	// 給紙トレイ設定(F1:上段).
	const OAPS_FreeSizePaper& s_FreeSizePaper01 = p_req->s_Environment.s_PaperTraySetting.s_PostInserter.s_UpperTray.s_PaperSize.s_FreeSizePaper01;
		if (!s_FreeSizePaper01.f_X.empty() && !s_FreeSizePaper01.f_X.range(139.7, 297.0))	// 2018/02/13 Pactera 修正.
		{
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralRangeIllegal, "FreeSizePaper01::X");
			return false;
		}
		if (!s_FreeSizePaper01.f_Y.empty() && !s_FreeSizePaper01.f_Y.range(182.0, 320.0))	// 2018/02/13 Pactera 修正.
		{
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralRangeIllegal, "FreeSizePaper01::Y");
			return false;
		}

	const OAPS_FreeSizePaper& s_FreeSizePaper02 = p_req->s_Environment.s_PaperTraySetting.s_PostInserter.s_UpperTray.s_PaperSize.s_FreeSizePaper02;
		if (!s_FreeSizePaper02.f_X.empty() && !s_FreeSizePaper02.f_X.range(139.7, 297.0))	// 2018/02/13 Pactera 修正.
		{
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralRangeIllegal, "FreeSizePaper02::X");
			return false;
		}
		if (!s_FreeSizePaper02.f_Y.empty() && !s_FreeSizePaper02.f_Y.range(182.0, 320.0))	// 2018/02/13 Pactera 修正.
		{
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralRangeIllegal, "FreeSizePaper02::Y");
			return false;
		}

	// 給紙トレイ設定(F2:下段).
	const OAPS_FreeSizePaper& s_FreeSizePaper1 = p_req->s_Environment.s_PaperTraySetting.s_PostInserter.s_LowerTray.s_PaperSize.s_FreeSizePaper01;
		if (!s_FreeSizePaper1.f_X.empty() && !s_FreeSizePaper1.f_X.range(139.7, 457.2))		// 2018/02/13 Pactera 修正.
		{
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralRangeIllegal, "FreeSizePaper01::X");
			return false;
		}
		if (!s_FreeSizePaper1.f_Y.empty() && !s_FreeSizePaper1.f_Y.range(182.0, 320.0))		// 2018/02/13 Pactera 修正.
		{
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralRangeIllegal, "FreeSizePaper01::Y");
			return false;
		}

	const OAPS_FreeSizePaper& s_FreeSizePaper2 = p_req->s_Environment.s_PaperTraySetting.s_PostInserter.s_LowerTray.s_PaperSize.s_FreeSizePaper02;
		if (!s_FreeSizePaper2.f_X.empty() && !s_FreeSizePaper2.f_X.range(139.7, 457.2))		// 2018/02/13 Pactera 修正.
		{
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralRangeIllegal, "FreeSizePaper02::X");
			return false;
		}
		if (!s_FreeSizePaper2.f_Y.empty() && !s_FreeSizePaper2.f_Y.range(182.0, 320.0))		// 2018/02/13 Pactera 修正.
		{
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralRangeIllegal, "FreeSizePaper02::Y");
			return false;
		}
	}
// 2019/12/18 【Eagle Z+】OP_BTS-11760 HIENG Start.
// 2020/02/05 【Eagle Z+】OP_BTS-11760 HIENG Modified End.
	return true;
}
// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem = Environment) set.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Environment>::set(const OAPS_AppReqSetUtilitySetting* p_req, OAPC_SyncLock& _slock, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=Environment)::set()" );

	e.setMessage( OAPE_Result_FaultRequest_AppReqSetUtilitySetting );

	//////////////////////////////////// .
	// I/F Lock .
	OAPC_ApiLock<APIC_SystemSetting> pc_SystemSetting(APIC_SystemSetting::newInstanceAdmin(), &_slock);
	OAPC_ApiLock<APIC_MachineSetting> pc_MachineSetting(APIC_MachineSetting::newInstanceAdmin() , &_slock);
	OAPC_ApiLock<APIC_OperationSetting> pc_OperationSetting(APIC_OperationSetting::newInstanceAdmin() , &_slock);
	OAPC_ApiLock<APIC_CopyJobSetting> pc_CopyJobSetting(APIC_CopyJobSetting::newInstanceAdmin(), &_slock);
	OAPC_ApiLock<APIC_PrintJobSetting> pc_PrintJobSetting(APIC_PrintJobSetting::newInstanceAdmin(), &_slock);
	OAPC_ApiLock<APIC_FeedTray> pc_FeedTray(APIC_FeedTray::newInstanceAdmin(), &_slock);
	OAPC_ApiLock<APIC_FaxJobSetting> pc_FaxJobSetting(APIC_FaxJobSetting::newInstanceAdmin(), &_slock);
	OAPC_ApiLock<APIC_ScanJobSetting> pc_ScanJobSetting(APIC_ScanJobSetting::newInstanceAdmin(), &_slock);
	OAPC_ApiLock<APIC_ScanerSetting> pc_ScanerSetting(APIC_ScanerSetting::newInstanceAdmin(), &_slock);
//ZeusMLK CQ369728 東野史明 2012/08/20 Start.
	OAPC_ApiLock<APIC_ScannerServiceSetting>   pc_ScannerServiceSetting(APIC_ScannerServiceSetting::newInstanceAdmin(), &_slock);
//ZeusMLK CQ369728 東野史明 2012/08/20 End.
	OAPC_ApiLock<APIC_SystemInfo> 		pc_SystemInfo( APIC_SystemInfo::newInstanceAdmin() , &_slock ); // MinervaZeus3rd UV制御構成変更_No.1202 Bairavi 2013/05/21 Added. 
	OAPC_ApiLock<APIC_PrinterServiceSetting>	pc_PrinterServiceSetting(APIC_PrinterServiceSetting::newInstanceAdmin(), &_slock);//【EagleH】RQ-720対応 2018/12/04 HCL-Garima Added.

	if(( !pc_MachineSetting 	) ||
	   ( !pc_OperationSetting 	) ||
	   ( !pc_CopyJobSetting 	) ||
	   ( !pc_PrintJobSetting 	) ||
	   ( !pc_FeedTray 			) ||
	   ( !pc_FaxJobSetting 		) ||
	   ( !pc_SystemSetting 		) ||
	   ( !pc_ScanJobSetting 	) ||
	   ( !pc_ScanerSetting 		) ||
//ZeusMLK CQ369728 東野史明 2012/08/20 Start.
	   ( !pc_ScannerServiceSetting 	) ||
//ZeusMLK CQ369728 東野史明 2012/08/20 End.
		( !pc_SystemInfo 	)|| 	// MinervaZeus3rd UV制御構成変更_No.1202 Bairavi 2013/05/21 Added. 
		( !pc_PrinterServiceSetting ))//【EagleH】RQ-720対応 2018/12/04 HCL-Garima Added.
	{
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance");
		return false;
	}
	
	// ※便宜上.
	const OAPS_UtilitySetting_Environment& s_Environment = p_req->s_Environment;

	//////////////////////////////////// .
	//	LanguageSelection.
	// ※便宜上.
	const OAPS_LanguageSelection& s_LanguageSelection = s_Environment.s_LanguageSelection;

	// 		Language.
	//ZeusS_ZX0 CQ522661 HCL-Vijay 2015/12/02 Modified Start.
	bool b_IsLanguageSupported = false;
	TYPE_Language e_Language = pc_MachineSetting->getLanguage();
	TYPE_Language e_SoftwareKeyboardLanguage = pc_OperationSetting->getKeyboardLanguage();
	TYPE_Language e_ExtKeyboardLanguage = pc_MachineSetting->getExtKeyboardLanguage();
	
	getCommAPIEnum(s_LanguageSelection.e_Language, e_Language );
	getCommAPIEnum(s_LanguageSelection.s_SoftwareKeyboardLanguage.e_Language, e_SoftwareKeyboardLanguage );
	getCommAPIEnum(s_LanguageSelection.s_ExtKeyboardLanguage.e_Language, e_ExtKeyboardLanguage );

	b_IsLanguageSupported = pc_MachineSetting->judgeEnableLanguage( e_Language );
	if(b_IsLanguageSupported) {
		pc_MachineSetting.set_enum_const_skip_check( s_LanguageSelection.e_Language, &APIC_MachineSetting::setLanguage );
	}
	// 		SoftwareKeyboardLanguage.
	// 			WithPanelLanguage,Language.
	pc_MachineSetting.set_enum_skip_check( s_LanguageSelection.s_SoftwareKeyboardLanguage.e_WithPanelLanguage, 	&APIC_MachineSetting::setSoftwareKeyboardWithPanalLanguage );
	b_IsLanguageSupported = pc_MachineSetting->judgeEnableLanguage( e_SoftwareKeyboardLanguage );
	if(b_IsLanguageSupported) {
		pc_OperationSetting.set_enum_skip_check( s_LanguageSelection.s_SoftwareKeyboardLanguage.e_Language, 		&APIC_OperationSetting::setKeyboardLanguage );
	}
	// 		ExtKeyboardLanguage.
	// 			WithPanelLanguage,Language.
	pc_MachineSetting.set_enum_skip_check( s_LanguageSelection.s_ExtKeyboardLanguage.e_WithPanelLanguage, 		&APIC_MachineSetting::setExtKeyboardWithPanalLanguage );
	b_IsLanguageSupported = pc_MachineSetting->judgeEnableLanguage( e_ExtKeyboardLanguage );
	if(b_IsLanguageSupported) {
		pc_MachineSetting.set_enum_skip_check( s_LanguageSelection.s_ExtKeyboardLanguage.e_Language, 				&APIC_MachineSetting::setExtKeyboardLanguage );
	}
	//ZeusS_ZX0 CQ522661 HCL-Vijay 2015/12/02 Modified End. 
	//////////////////////////////////// .
	// SelectKeyboard.
	pc_OperationSetting.set_enum_skip_check( s_Environment.e_SelectKeyboard, &APIC_OperationSetting::setKeyboardType );

	//////////////////////////////////// .
	//Keyboard Type.	
    // IT5_4.2 RQ_663 WP No.08 Kailash 2017/08/28 Added Start.
    pc_OperationSetting.set_enum_skip_check( s_Environment.e_KeyboardType, &APIC_OperationSetting::setKeyboardUISetting );
    // IT5_4.2 RQ_663 WP No.08 Kailash 2017/08/28 Added End.	
	// UnitSetting.
	pc_OperationSetting.set_enum_skip_check( s_Environment.e_UnitSetting, &APIC_OperationSetting::setUnitSystem );

	//////////////////////////////////// .
	//	PaperTraySetting.
	// ※便宜上.
	const OAPS_PaperTraySetting& s_PaperTraySetting = s_Environment.s_PaperTraySetting;

	// ※便宜上.
	const OAPS_AutoTraySelect& s_AutoTraySelect = s_PaperTraySetting.s_AutoTraySelect;

	//		AutoTraySelect
	//			Enable.
	pc_CopyJobSetting.set_enum_skip_check( s_AutoTraySelect.e_Enable, &APIC_CopyJobSetting::setAtsEnableAtUserSelectTray );

	// ※便宜上.
	const OAPS_EachTray& s_EachTray = s_AutoTraySelect.s_EachTray;
	//			EachTray.
	//				Tray1.
	pc_CopyJobSetting.set_enum_const_skip_check( s_EachTray.e_Tray1, 	&APIC_CopyJobSetting::setAtsApsEnable_Tray_1 );
	//				Tray2.
	pc_CopyJobSetting.set_enum_const_skip_check( s_EachTray.e_Tray2, 	&APIC_CopyJobSetting::setAtsApsEnable_Tray_2 );
	//				Tray3.
	pc_CopyJobSetting.set_enum_const_skip_check( s_EachTray.e_Tray3, 	&APIC_CopyJobSetting::setAtsApsEnable_Tray_3 );
	//				Tray4.
	pc_CopyJobSetting.set_enum_const_skip_check( s_EachTray.e_Tray4, 	&APIC_CopyJobSetting::setAtsApsEnable_Tray_4 );
	//				Tray5(LCT).
	pc_CopyJobSetting.set_enum_const_skip_check( s_EachTray.e_Lct, 		&APIC_CopyJobSetting::setAtsApsEnable_Tray_5 );
	//				Hand(Manual).
	pc_CopyJobSetting.set_enum_const_skip_check( s_EachTray.e_Hand, 	&APIC_CopyJobSetting::setAtsApsEnable_TrayManual );

	//		AutoTraySelect
	//			PriorityTrayList
	//				ArraySize.
	// 				PriorityTray.
	// 					Index.
	// 					TrayID.

	//.
	// ↓↓↓(以下、OAP_TRN_devicesetting_permanent.cpp <...CommonPaperHandling>::set() よりそのまま流用).
	//.
	if(!s_AutoTraySelect.s_PriorityTrayList.empty()) {
		Bool b_ATS_APS_1      = pc_CopyJobSetting->getAtsApsEnable_Tray_1();
		Bool b_ATS_APS_2      = pc_CopyJobSetting->getAtsApsEnable_Tray_2();
		Bool b_ATS_APS_3      = pc_CopyJobSetting->getAtsApsEnable_Tray_3();
		Bool b_ATS_APS_4      = pc_CopyJobSetting->getAtsApsEnable_Tray_4();
		Bool b_ATS_APS_5      = pc_CopyJobSetting->getAtsApsEnable_Tray_5();
		Bool b_ATS_APS_Manual = pc_CopyJobSetting->getAtsApsEnable_TrayManual();
		
		// TraySearchPriority にセットしたかを判断する為のフラグ 
		bool b_set_1 = false;
		bool b_set_2 = false;
		bool b_set_3 = false;
		bool b_set_4 = false;
		bool b_set_5 = false;
		bool b_set_Manual = false;

		TYPS_TraySearchPriority s_TraySearchPriority;		// CommonAPI用構造体 

		memset( &s_TraySearchPriority, TYPE_FeedTrayUnselect, sizeof(TYPS_TraySearchPriority) );

		short k = 0;										// CommonAPI構造体のトレイの優先順位のindex最大は６ 
		const short CommonAPI_MaxList = 6; 					// CommonAPIの設定数の最大値を超えないのチェック 機種依存 

		for( int i = 0; i < s_AutoTraySelect.s_PriorityTrayList.ss_ArraySize; i++ ){
			TYPE_FeedTray e_Com_Tmp;
			if ( getCommAPIEnum( s_AutoTraySelect.s_PriorityTrayList.s_PriorityTray[i].e_TrayID , e_Com_Tmp ) && // Enum変換が成功した場合 
				 ( k < CommonAPI_MaxList ) 
				)	
			{
				switch(e_Com_Tmp)
				{
				case TYPE_FeedTray1:		b_set_1      = true;	break;
				case TYPE_FeedTray2:		b_set_2      = true;	break;
				case TYPE_FeedTray3:		b_set_3      = true;	break;
				case TYPE_FeedTray4:		b_set_4      = true;	break;
				case TYPE_FeedTray5:		b_set_5      = true;	break;
				case TYPE_FeedTrayManual:	b_set_Manual = true;	break;
				default:	break;
				}

				s_TraySearchPriority.e_FeedTray[k] = e_Com_Tmp;		// 設定する 
				k++;
			}
		}
		// ATS/APS許可 でtrue が設定されているのにTaskPriorityで選択されていないものをセットする 
		if(b_ATS_APS_1 && !b_set_1 )         {	s_TraySearchPriority.e_FeedTray[k] = TYPE_FeedTray1;		k++;	}
		if(b_ATS_APS_2 && !b_set_2 )         {	s_TraySearchPriority.e_FeedTray[k] = TYPE_FeedTray2;		k++;	}
		if(b_ATS_APS_3 && !b_set_3 )         {	s_TraySearchPriority.e_FeedTray[k] = TYPE_FeedTray3;		k++;	}
		if(b_ATS_APS_4 && !b_set_4 )         {	s_TraySearchPriority.e_FeedTray[k] = TYPE_FeedTray4;		k++;	}
		if(b_ATS_APS_5 && !b_set_5 )         {	s_TraySearchPriority.e_FeedTray[k] = TYPE_FeedTray5;		k++;	}
		if(b_ATS_APS_Manual && !b_set_Manual){	s_TraySearchPriority.e_FeedTray[k] = TYPE_FeedTrayManual;	k++;	}
		
		pc_CopyJobSetting->setTraySearchPriority( s_TraySearchPriority );
	}
	//.
	// ↑↑↑(以下、OAP_TRN_devicesetting_permanent.cpp <...CommonPaperHandling>::set() よりそのまま流用).
	//.

	//		AutoPaperSelect.
	// ※便宜上.
	const OAPS_AutoPaperSelect& s_AutoPaperSelect = s_PaperTraySetting.s_AutoPaperSelect;

	// (一旦,現在の設定値をCommonより取得) APS-用紙設定.
	TYPS_PaperTypeOnAPS* pc_PaperTypeOnAPS = pc_CopyJobSetting->getPaperTypeOnAPS();
	//			UserPaper1.
	if( !s_AutoPaperSelect.e_UserPaper1.empty() ){
		getCommAPIEnum( s_AutoPaperSelect.e_UserPaper1, pc_PaperTypeOnAPS->b_TypeUser1 );
	}
//2018/02/15 HCL-Kailash 【Eagle】 RQ-722対応 Added Start.
#if ( !defined (DEF_EagleCus)  &&  !defined (DEF_SparrowCus) ) || defined (DEF_HeliosMLKCus)	// 2019/11/08 【HeliosMLK】 MachineNo.11 HIENG added.
	//			UserPaper2.
	if( !s_AutoPaperSelect.e_UserPaper2.empty() ){
		getCommAPIEnum( s_AutoPaperSelect.e_UserPaper2, pc_PaperTypeOnAPS->b_TypeUser2 );
	}
#endif
//2018/02/15 HCL-Kailash 【Eagle】 RQ-722対応 Added End.
	//ZeusS CQ505523 Palani 2014/11/05 Added Start.
	//	ThinPaper.
#if !( defined DEF_Zeus5BkCus || defined DEF_HeliosCus || defined DEF_SparrowCus || defined DEF_HeliosMLKCus )			//HeliosL 機器構成変更_No.20 Palani 2014/11/24 Added.		//2018/02/15 HCL-Kailash 【Eagle】 RQ-722対応 Modified.	// 2019/11/08 【HeliosMLK】 MachineNo.11 HIENG added.
	if( !s_AutoPaperSelect.e_ThinPaper.empty() ){
		getCommAPIEnum( s_AutoPaperSelect.e_ThinPaper, pc_PaperTypeOnAPS->b_TypeThin );
	}
#endif
	//ZeusS CQ505523 Palani 2014/11/05 Added End.
	// (Commonに設定).
	pc_CopyJobSetting->setPaperTypeOnAPS( pc_PaperTypeOnAPS );
	
	//			PriorityPaperTypeList.
	//				ArraySize.
	//				PriorityPaperType.

	// (一旦,現在の設定値をCommonより取得) APS-優先用紙種類設定.
	// ※優先度：(高)配列[0]←→配列[2](低) で順番に設定されている.
	TYPS_PaperTypeOnAPSPriority s_PaperTypeOnAPSPriority;
	::memset( &s_PaperTypeOnAPSPriority, 0, sizeof(s_PaperTypeOnAPSPriority) );
	pc_CopyJobSetting->getPaperTypeOnAPSPriority( &s_PaperTypeOnAPSPriority );
	
	// ※便宜上.
	const OAPS_PriorityPaperTypeList& s_PriorityPaperTypeList = s_PaperTraySetting.s_AutoPaperSelect.s_PriorityPaperTypeList;
	if( !s_PriorityPaperTypeList.ss_ArraySize.empty() ){
		short ss_ArraySize = (short)s_PriorityPaperTypeList.ss_ArraySize.get();
		for( short i = 0; ( i < ss_ArraySize ) && ( i < OAPD_PRIORITYPAPERTYPELIST_MAXLIST ); i++ ){
			// 優先順位(Index)=[1-3].
			if( !s_PriorityPaperTypeList.s_PriorityPaperType[i].ss_Index.empty() ){
				uchar uc_Index = (uchar)s_PriorityPaperTypeList.s_PriorityPaperType[i].ss_Index.get();
				if(( OAPD_PRIORITYPAPERTYPE_INDEX_LOW <= uc_Index ) && ( uc_Index <= OAPD_PRIORITYPAPERTYPE_INDEX_HIGH )){
					// 用紙種類.
					// ※優先順位(Index)=[1-3], 配列要素は[0-2]なので(-1).
					//Zeus5BK UV機器構成変更_No.61 Sundar 2014/05/09 Modified Start.
//2018/02/15 HCL-Kailash 【Eagle】 RQ-722対応 Added Start.
// 2019/11/11 【HeliosMLK】 MachineNo.11 HIENG Start.
#if defined (DEF_HeliosMLKCus)
				if((s_PriorityPaperTypeList.s_PriorityPaperType[i].e_PaperType != OAPE_PriorityPaperType_Thin) ){
// 2019/11/11 【HeliosMLK】 MachineNo.11 HIENG End.
#elif defined (DEF_EagleCus)
				if((s_PriorityPaperTypeList.s_PriorityPaperType[i].e_PaperType != OAPE_PriorityPaperType_User2) ){
#elif defined (DEF_SparrowCus)
				if( (s_PriorityPaperTypeList.s_PriorityPaperType[i].e_PaperType != OAPE_PriorityPaperType_User2)
					&&(s_PriorityPaperTypeList.s_PriorityPaperType[i].e_PaperType != OAPE_PriorityPaperType_Thin) ){
#else
				if(1){
#endif
//2018/02/15 HCL-Kailash 【Eagle】 RQ-722対応 Added End.
					if( !getCommAPIEnum(s_PriorityPaperTypeList.s_PriorityPaperType[i].e_PaperType, s_PaperTypeOnAPSPriority.e_PaperTypeOnAPSPriority[ uc_Index - 1 ])) {
						s_PaperTypeOnAPSPriority.e_PaperTypeOnAPSPriority[ uc_Index - 1 ] = TYPE_PaperTypeOnAPSPriorityUnselect;
					}
					//Zeus5BK UV機器構成変更_No.61 Sundar 2014/05/09 Modified End.
				}		//2018/02/15 HCL-Kailash 【Eagle】 RQ-722対応 Added.	
				}
			}
		}
		// (Commonに設定).
		pc_CopyJobSetting->setPaperTypeOnAPSPriority( &s_PaperTypeOnAPSPriority );
	}

	//		TrayUnmatch.
	pc_CopyJobSetting.set_enum_skip_check( s_PaperTraySetting.e_TrayUnmatch, &APIC_CopyJobSetting::setPaperSizeMismach );

	//		ListPrint.
	pc_PrintJobSetting->setApsAtListPrint( false );
	pc_PrintJobSetting.set_enum_skip_check( s_PaperTraySetting.s_ListPrint.e_Tray, 		&APIC_PrintJobSetting::setFeedTrayAtListPrint );
	pc_PrintJobSetting.set_enum_skip_check( s_PaperTraySetting.s_ListPrint.e_Duplex, 	&APIC_PrintJobSetting::setDuplexPrintAtListPrint );

// 2020/02/05 【Eagle Z+】OP_BTS-11760 HIENG Modified Start.
// 2020/02/19 【Eagle Z+】OP_BTS-20715(OP_BTS-11760) HIENG Modified Start.
	if( OAPD_VERSION_CHECK(p_req->getVersion(), OAPD_OVER_V77) ) {
// 2020/02/19 【Eagle Z+】OP_BTS-20715(OP_BTS-11760) HIENG Modified End.
// 2019/12/18 【Eagle Z+】OP_BTS-11760 HIENG Start.
// 2019/11/21 【HeliosMLK】 MachineNo.11 HIENG Start.
//#ifndef DEF_HeliosMLKCus
#if !defined (DEF_DenebMLKCus) && !defined (DEF_SparrowCus) && !defined (DEF_HeliosMLKCus)
	//		PostInserter.
	//			UpperTray.
	// ※便宜上.
	const OAPS_PostInserter_Tray& s_UpperTray = s_PaperTraySetting.s_PostInserter.s_UpperTray;

	// (一旦,現在の設定値をCommonより取得).
	TYPC_PaperType c_PaperType = pc_FeedTray->getPaperType( TYPE_FeedTrayPI1 );

	//				PaperType.
	if( !s_UpperTray.e_PaperType.empty() ){
		TYPE_PaperType e_ComPaperType = TYPE_PaperTypeNormal;		// (初期化).

		getCommAPIEnum( s_UpperTray.e_PaperType, e_ComPaperType );
		c_PaperType.setPaperType( e_ComPaperType );

		// (Commonに設定).
		pc_FeedTray->setPaperType( TYPE_FeedTrayPI1, c_PaperType );
	}

	//				OriginalSize.
	//					Auto.
	if( !s_UpperTray.s_PaperSize.empty() ){
		if( !s_UpperTray.s_PaperSize.e_Auto.empty() ){
			Bool b_Auto = False;
			getCommAPIEnum( s_UpperTray.s_PaperSize.e_Auto, b_Auto );
			// (Commonに設定).
			pc_FeedTray->setPaperSizeAutoDetectOn( TYPE_FeedTrayPI1, b_Auto );
		}
	
	//					SizeCode.
		// (一旦,現在の設定値をCommonより取得).
		TYPC_PaperSize c_PaperSize = pc_FeedTray->getPaperSize( TYPE_FeedTrayPI1 );
			//2017/10/17_XML基盤技術開発v2 pactera modify start.
		//if( !s_UpperTray.s_PaperSize.e_SizeCode.empty() ){
		//	getCommAPIEnum( s_UpperTray.s_PaperSize.e_SizeCode, c_PaperSize.e_SizeCode );
		//}
		if( !s_UpperTray.s_PaperSize.empty() ){			
			getCommAPIStruct(s_UpperTray.s_PaperSize,c_PaperSize );		
			//2017/10/17  _XML基盤技術開発v2 pactera modify end.		
		}
//IT5_4.1 1015066 Sahana 2017/04/11 Commented Start.		
//		if( !s_UpperTray.s_PaperSize.e_SizeCode.empty() ){
//			getCommAPIEnum( s_UpperTray.s_PaperSize.e_SizeCode, c_PaperSize.e_SizeCode );
//		}
//IT5_4.1 1015066 Sahana 2017/04/11 Commented Start.
//					Dimension.
		if( !s_UpperTray.s_PaperSize.s_Dimension2.f_X.empty() ){
			c_PaperSize.c_SizeMM.x = s_UpperTray.s_PaperSize.s_Dimension2.f_X.get();
		}
		if( !s_UpperTray.s_PaperSize.s_Dimension2.f_Y.empty() ){
			c_PaperSize.c_SizeMM.y = s_UpperTray.s_PaperSize.s_Dimension2.f_Y.get();
		}
// 2020/02/20 EagleZ+ OP_BTS-20656 日立 江上 DEL Start.
//		// (Commonに設定).
//		c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse ();
//		printf(" Bairavi Added Upper Tray X value  %d\n",  c_PaperSize.c_SizeMM.x);
//		printf(" Bairavi Added Upper Tray y value %d\n",  c_PaperSize.c_SizeMM.y);
//		
//		//Market Issue TAS000000002230 Hcl-Garima 12/12/2019 End.
// 2020/02/20 EagleZ+ OP_BTS-20656 日立 江上 DEL End.
//IT6_2.0MR2 OP_BTS-22193 [Merge of TAS000000002230] Hcl-Mangai 15/04/2019 Start.
		if(( !s_UpperTray.s_PaperSize.s_Dimension2.f_X.empty() ) && (!s_UpperTray.s_PaperSize.s_Dimension2.f_Y.empty())){
			c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse ();
		}
		
//IT6_2.0MR2 OP_BTS-22193 [Merge of TAS000000002230] Hcl-Mangai 15/04/2019 End.
		// (Commonに設定).
		pc_FeedTray->setPaperSize( TYPE_FeedTrayPI1, c_PaperSize );

		//2017/03/13 カバーフィーダー_XML基盤技術開発v2 pactera add start.
		if ( !s_UpperTray.s_PaperSize.s_FreeSizePaper01.empty() ) {
			pc_OperationSetting->setPaperSizeUnfix01NameUTF8( (char*)s_UpperTray.s_PaperSize.s_FreeSizePaper01.sc_RegisterName.get(), TYPE_TrayTypePI, TYPE_CharSetUTF8, TYPE_FeedTrayPI1 );  //Jenkins_Warning 04/01/18 HCL_Prashant
			TYPC_Point<float> c_Point1( 0.0f, 0.0f );
			c_Point1.x =  s_UpperTray.s_PaperSize.s_FreeSizePaper01.f_X.get();
			c_Point1.y =  s_UpperTray.s_PaperSize.s_FreeSizePaper01.f_Y.get();
			//c_PaperSize_01.c_SizeMM = c_PaperSize_01.c_SizeMM.xyReverse();
			pc_OperationSetting->setPaperSizeUnfix01( c_Point1, TYPE_TrayTypePI, TYPE_FeedTrayPI1 );
			// pc_FeedTray->setPaperSize( TYPE_FeedTrayPI1, c_PaperSize );
			
		}
		if ( !s_UpperTray.s_PaperSize.s_FreeSizePaper02.empty() ) {
			pc_OperationSetting->setPaperSizeUnfix02NameUTF8( (char*)s_UpperTray.s_PaperSize.s_FreeSizePaper02.sc_RegisterName.get(), TYPE_TrayTypePI, TYPE_CharSetUTF8, TYPE_FeedTrayPI1 );  //Jenkins_Warning 04/01/18 HCL_Prashant
			TYPC_Point<float> c_Point2( 0.0f, 0.0f );
			c_Point2.x =  s_UpperTray.s_PaperSize.s_FreeSizePaper02.f_X.get();
			c_Point2.y =  s_UpperTray.s_PaperSize.s_FreeSizePaper02.f_Y.get();
			//c_PaperSize_02.c_SizeMM = c_PaperSize_02.c_SizeMM.xyReverse();
			pc_OperationSetting->setPaperSizeUnfix02( c_Point2, TYPE_TrayTypePI, TYPE_FeedTrayPI1 );
		}
		//2017/03/13 カバーフィーダー_XML基盤技術開発v2 pactera add end.
	}

	//		PostInserter.
	//			LowerTray.
	// ※便宜上.
	const OAPS_PostInserter_Tray& s_LowerTray = s_PaperTraySetting.s_PostInserter.s_LowerTray;

	// (一旦,現在の設定値をCommonより取得).
	::memset( &c_PaperType, 0, sizeof(c_PaperType) );
	c_PaperType = pc_FeedTray->getPaperType( TYPE_FeedTrayPI2 );

	//				PaperType.
	if( !s_LowerTray.e_PaperType.empty() ){
		TYPE_PaperType e_ComPaperType = TYPE_PaperTypeNormal;		// (初期化).

		getCommAPIEnum( s_LowerTray.e_PaperType, e_ComPaperType );
		c_PaperType.setPaperType( e_ComPaperType );

		// (Commonに設定).
		pc_FeedTray->setPaperType( TYPE_FeedTrayPI2, c_PaperType );
	}

	//				OriginalSize.
	//					Auto.
	if( !s_LowerTray.s_PaperSize.empty() ){
		if( !s_LowerTray.s_PaperSize.e_Auto.empty() ){
			Bool b_Auto = False;
			getCommAPIEnum( s_LowerTray.s_PaperSize.e_Auto, b_Auto );
			// (Commonに設定).
			pc_FeedTray->setPaperSizeAutoDetectOn( TYPE_FeedTrayPI2, b_Auto );
		}
	
	//					SizeCode.
		// (一旦,現在の設定値をCommonより取得).
		TYPC_PaperSize c_PaperSize = pc_FeedTray->getPaperSize( TYPE_FeedTrayPI2 );
		//2017/10/17_XML基盤技術開発v2 pactera modify start.
		if( !s_LowerTray.s_PaperSize.empty() ){	
			//getCommAPIEnum( s_LowerTray.s_PaperSize.e_SizeCode, c_PaperSize.e_SizeCode );			
			getCommAPIStruct(s_LowerTray.s_PaperSize,c_PaperSize );		
			//2017/10/17  _XML基盤技術開発v2 pactera modify end.
			
		}
//IT5_4.1 1015066 Sahana 2017/04/11 Commented Start.
//		if( !s_LowerTray.s_PaperSize.e_SizeCode.empty() ){
//			getCommAPIEnum( s_LowerTray.s_PaperSize.e_SizeCode, c_PaperSize.e_SizeCode );
//		}
//IT5_4.1 1015066 Sahana 2017/04/11 Commented End
//				Dimension.
		if( !s_LowerTray.s_PaperSize.s_Dimension2.f_X.empty() ){
			c_PaperSize.c_SizeMM.x = s_LowerTray.s_PaperSize.s_Dimension2.f_X.get();
		}
		if( !s_LowerTray.s_PaperSize.s_Dimension2.f_Y.empty() ){
			c_PaperSize.c_SizeMM.y = s_LowerTray.s_PaperSize.s_Dimension2.f_Y.get();
		}
// 2020/02/20 EagleZ+ OP_BTS-20656 日立 江上 DEL Start.
//		// (Commonに設定).
//		c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse ();
//		printf(" Bairavi Added Lower Tray X value  %d\n",  c_PaperSize.c_SizeMM.x);
//		printf(" Bairavi Added Lower Tray y value %d\n",  c_PaperSize.c_SizeMM.y);
//		
//		//Market Issue TAS000000002230 Hcl-Garima 12/12/2019 End.
// 2020/02/20 EagleZ+ OP_BTS-20656 日立 江上 DEL End.

//IT6_2.0MR2 OP_BTS-22193 [Merge of TAS000000002230] Hcl-Mangai 15/04/2019 Start.
		if(( !s_LowerTray.s_PaperSize.s_Dimension2.f_X.empty() ) && (!s_LowerTray.s_PaperSize.s_Dimension2.f_Y.empty())){
			c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse ();
		}
		
//IT6_2.0MR2 OP_BTS-22193 [Merge of TAS000000002230] Hcl-Mangai 15/04/2019 End.
		// (Commonに設定).
		pc_FeedTray->setPaperSize( TYPE_FeedTrayPI2, c_PaperSize );

		//2017/03/13 カバーフィーダー_XML基盤技術開発v2 pactera add start.
		if ( !s_LowerTray.s_PaperSize.s_FreeSizePaper01.empty() ) {
			pc_OperationSetting->setPaperSizeUnfix01NameUTF8( (char*)s_LowerTray.s_PaperSize.s_FreeSizePaper01.sc_RegisterName.get(), TYPE_TrayTypePI, TYPE_CharSetUTF8, TYPE_FeedTrayPI2 ); //Jenkins_Warning 04/01/18 HCL_Prashant
			TYPC_Point<float> c_Point1( 0.0f, 0.0f );
			c_Point1.x  =  s_LowerTray.s_PaperSize.s_FreeSizePaper01.f_X.get();
			c_Point1.y =  s_LowerTray.s_PaperSize.s_FreeSizePaper01.f_Y.get();
			//c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse();
			pc_OperationSetting->setPaperSizeUnfix01( c_Point1, TYPE_TrayTypePI, TYPE_FeedTrayPI2 );
		}
		if ( !s_LowerTray.s_PaperSize.s_FreeSizePaper02.empty() ) {
			pc_OperationSetting->setPaperSizeUnfix02NameUTF8((char*) s_LowerTray.s_PaperSize.s_FreeSizePaper02.sc_RegisterName.get(), TYPE_TrayTypePI, TYPE_CharSetUTF8, TYPE_FeedTrayPI2 ); //Jenkins_Warning 04/01/18 HCL_Prashant
			TYPC_Point<float> c_Point2( 0.0f, 0.0f );
			c_Point2.x =  s_LowerTray.s_PaperSize.s_FreeSizePaper02.f_X.get();
			c_Point2.y =  s_LowerTray.s_PaperSize.s_FreeSizePaper02.f_Y.get();
			//c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse();
			pc_OperationSetting->setPaperSizeUnfix02( c_Point2, TYPE_TrayTypePI, TYPE_FeedTrayPI2 );
		}
		//2017/03/13 カバーフィーダー_XML基盤技術開発v2 pactera add end.
	}
#endif
// 2019/11/21 【HeliosMLK】 MachineNo.11 HIENG End.
// 2019/12/18 【Eagle Z+】OP_BTS-11760 HIENG End.
	} else {
	//		PostInserter.
	//			UpperTray.
	// ※便宜上.
	const OAPS_PostInserter_Tray& s_UpperTray = s_PaperTraySetting.s_PostInserter.s_UpperTray;

	// (一旦,現在の設定値をCommonより取得).
	TYPC_PaperType c_PaperType = pc_FeedTray->getPaperType( TYPE_FeedTrayPI1 );

	//				PaperType.
	if( !s_UpperTray.e_PaperType.empty() ){
		TYPE_PaperType e_ComPaperType = TYPE_PaperTypeNormal;		// (初期化).

		getCommAPIEnum( s_UpperTray.e_PaperType, e_ComPaperType );
		c_PaperType.setPaperType( e_ComPaperType );

		// (Commonに設定).
		pc_FeedTray->setPaperType( TYPE_FeedTrayPI1, c_PaperType );
	}

	//				OriginalSize.
	//					Auto.
	if( !s_UpperTray.s_PaperSize.empty() ){
		if( !s_UpperTray.s_PaperSize.e_Auto.empty() ){
			Bool b_Auto = False;
			getCommAPIEnum( s_UpperTray.s_PaperSize.e_Auto, b_Auto );
			// (Commonに設定).
			pc_FeedTray->setPaperSizeAutoDetectOn( TYPE_FeedTrayPI1, b_Auto );
		}
	
	//					SizeCode.
		// (一旦,現在の設定値をCommonより取得).
		TYPC_PaperSize c_PaperSize = pc_FeedTray->getPaperSize( TYPE_FeedTrayPI1 );
			//2017/10/17_XML基盤技術開発v2 pactera modify start.
		//if( !s_UpperTray.s_PaperSize.e_SizeCode.empty() ){
		//	getCommAPIEnum( s_UpperTray.s_PaperSize.e_SizeCode, c_PaperSize.e_SizeCode );
		//}
		if( !s_UpperTray.s_PaperSize.empty() ){			
			getCommAPIStruct(s_UpperTray.s_PaperSize,c_PaperSize );		
			//2017/10/17  _XML基盤技術開発v2 pactera modify end.		
		}
//IT5_4.1 1015066 Sahana 2017/04/11 Commented Start.		
//		if( !s_UpperTray.s_PaperSize.e_SizeCode.empty() ){
//			getCommAPIEnum( s_UpperTray.s_PaperSize.e_SizeCode, c_PaperSize.e_SizeCode );
//		}
//IT5_4.1 1015066 Sahana 2017/04/11 Commented Start.
//					Dimension.
		if( !s_UpperTray.s_PaperSize.s_Dimension2.f_X.empty() ){
			c_PaperSize.c_SizeMM.x = s_UpperTray.s_PaperSize.s_Dimension2.f_X.get();
		}
		if( !s_UpperTray.s_PaperSize.s_Dimension2.f_Y.empty() ){
			c_PaperSize.c_SizeMM.y = s_UpperTray.s_PaperSize.s_Dimension2.f_Y.get();
		}
		// (Commonに設定).
		pc_FeedTray->setPaperSize( TYPE_FeedTrayPI1, c_PaperSize );

		//2017/03/13 カバーフィーダー_XML基盤技術開発v2 pactera add start.
		if ( !s_UpperTray.s_PaperSize.s_FreeSizePaper01.empty() ) {
			pc_OperationSetting->setPaperSizeUnfix01NameUTF8( (char*)s_UpperTray.s_PaperSize.s_FreeSizePaper01.sc_RegisterName.get(), TYPE_TrayTypePI, TYPE_CharSetUTF8, TYPE_FeedTrayPI1 );  //Jenkins_Warning 04/01/18 HCL_Prashant
			TYPC_Point<float> c_Point1( 0.0f, 0.0f );
			c_Point1.x =  s_UpperTray.s_PaperSize.s_FreeSizePaper01.f_X.get();
			c_Point1.y =  s_UpperTray.s_PaperSize.s_FreeSizePaper01.f_Y.get();
			//c_PaperSize_01.c_SizeMM = c_PaperSize_01.c_SizeMM.xyReverse();
			pc_OperationSetting->setPaperSizeUnfix01( c_Point1, TYPE_TrayTypePI, TYPE_FeedTrayPI1 );
			// pc_FeedTray->setPaperSize( TYPE_FeedTrayPI1, c_PaperSize );
			
		}
		if ( !s_UpperTray.s_PaperSize.s_FreeSizePaper02.empty() ) {
			pc_OperationSetting->setPaperSizeUnfix02NameUTF8( (char*)s_UpperTray.s_PaperSize.s_FreeSizePaper02.sc_RegisterName.get(), TYPE_TrayTypePI, TYPE_CharSetUTF8, TYPE_FeedTrayPI1 );  //Jenkins_Warning 04/01/18 HCL_Prashant
			TYPC_Point<float> c_Point2( 0.0f, 0.0f );
			c_Point2.x =  s_UpperTray.s_PaperSize.s_FreeSizePaper02.f_X.get();
			c_Point2.y =  s_UpperTray.s_PaperSize.s_FreeSizePaper02.f_Y.get();
			//c_PaperSize_02.c_SizeMM = c_PaperSize_02.c_SizeMM.xyReverse();
			pc_OperationSetting->setPaperSizeUnfix02( c_Point2, TYPE_TrayTypePI, TYPE_FeedTrayPI1 );
		}
		//2017/03/13 カバーフィーダー_XML基盤技術開発v2 pactera add end.
	}

	//		PostInserter.
	//			LowerTray.
	// ※便宜上.
	const OAPS_PostInserter_Tray& s_LowerTray = s_PaperTraySetting.s_PostInserter.s_LowerTray;

	// (一旦,現在の設定値をCommonより取得).
	::memset( &c_PaperType, 0, sizeof(c_PaperType) );
	c_PaperType = pc_FeedTray->getPaperType( TYPE_FeedTrayPI2 );

	//				PaperType.
	if( !s_LowerTray.e_PaperType.empty() ){
		TYPE_PaperType e_ComPaperType = TYPE_PaperTypeNormal;		// (初期化).

		getCommAPIEnum( s_LowerTray.e_PaperType, e_ComPaperType );
		c_PaperType.setPaperType( e_ComPaperType );

		// (Commonに設定).
		pc_FeedTray->setPaperType( TYPE_FeedTrayPI2, c_PaperType );
	}

	//				OriginalSize.
	//					Auto.
	if( !s_LowerTray.s_PaperSize.empty() ){
		if( !s_LowerTray.s_PaperSize.e_Auto.empty() ){
			Bool b_Auto = False;
			getCommAPIEnum( s_LowerTray.s_PaperSize.e_Auto, b_Auto );
			// (Commonに設定).
			pc_FeedTray->setPaperSizeAutoDetectOn( TYPE_FeedTrayPI2, b_Auto );
		}
	
	//					SizeCode.
		// (一旦,現在の設定値をCommonより取得).
		TYPC_PaperSize c_PaperSize = pc_FeedTray->getPaperSize( TYPE_FeedTrayPI2 );
		//2017/10/17_XML基盤技術開発v2 pactera modify start.
		if( !s_LowerTray.s_PaperSize.empty() ){	
			//getCommAPIEnum( s_LowerTray.s_PaperSize.e_SizeCode, c_PaperSize.e_SizeCode );			
			getCommAPIStruct(s_LowerTray.s_PaperSize,c_PaperSize );		
			//2017/10/17  _XML基盤技術開発v2 pactera modify end.
			
		}
//IT5_4.1 1015066 Sahana 2017/04/11 Commented Start.
//		if( !s_LowerTray.s_PaperSize.e_SizeCode.empty() ){
//			getCommAPIEnum( s_LowerTray.s_PaperSize.e_SizeCode, c_PaperSize.e_SizeCode );
//		}
//IT5_4.1 1015066 Sahana 2017/04/11 Commented End
//				Dimension.
		if( !s_LowerTray.s_PaperSize.s_Dimension2.f_X.empty() ){
			c_PaperSize.c_SizeMM.x = s_LowerTray.s_PaperSize.s_Dimension2.f_X.get();
		}
		if( !s_LowerTray.s_PaperSize.s_Dimension2.f_Y.empty() ){
			c_PaperSize.c_SizeMM.y = s_LowerTray.s_PaperSize.s_Dimension2.f_Y.get();
		}
		// (Commonに設定).
		pc_FeedTray->setPaperSize( TYPE_FeedTrayPI2, c_PaperSize );

		//2017/03/13 カバーフィーダー_XML基盤技術開発v2 pactera add start.
		if ( !s_LowerTray.s_PaperSize.s_FreeSizePaper01.empty() ) {
			pc_OperationSetting->setPaperSizeUnfix01NameUTF8( (char*)s_LowerTray.s_PaperSize.s_FreeSizePaper01.sc_RegisterName.get(), TYPE_TrayTypePI, TYPE_CharSetUTF8, TYPE_FeedTrayPI2 ); //Jenkins_Warning 04/01/18 HCL_Prashant
			TYPC_Point<float> c_Point1( 0.0f, 0.0f );
			c_Point1.x  =  s_LowerTray.s_PaperSize.s_FreeSizePaper01.f_X.get();
			c_Point1.y =  s_LowerTray.s_PaperSize.s_FreeSizePaper01.f_Y.get();
			//c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse();
			pc_OperationSetting->setPaperSizeUnfix01( c_Point1, TYPE_TrayTypePI, TYPE_FeedTrayPI2 );
		}
		if ( !s_LowerTray.s_PaperSize.s_FreeSizePaper02.empty() ) {
			pc_OperationSetting->setPaperSizeUnfix02NameUTF8((char*) s_LowerTray.s_PaperSize.s_FreeSizePaper02.sc_RegisterName.get(), TYPE_TrayTypePI, TYPE_CharSetUTF8, TYPE_FeedTrayPI2 ); //Jenkins_Warning 04/01/18 HCL_Prashant
			TYPC_Point<float> c_Point2( 0.0f, 0.0f );
			c_Point2.x =  s_LowerTray.s_PaperSize.s_FreeSizePaper02.f_X.get();
			c_Point2.y =  s_LowerTray.s_PaperSize.s_FreeSizePaper02.f_Y.get();
			//c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse();
			pc_OperationSetting->setPaperSizeUnfix02( c_Point2, TYPE_TrayTypePI, TYPE_FeedTrayPI2 );
		}
		//2017/03/13 カバーフィーダー_XML基盤技術開発v2 pactera add end.
	}
	}
// 2020/02/05 【Eagle Z+】OP_BTS-11760 HIENG Modified End.
	//////////////////////////////////// .
	// AutoColorLevel.
//#ifndef DEF_DenebMLKCus  //【IT6_EagleBK】制御構成変更_No.30 対応 2019/06/03 HCL-Mangai Commented. // DenebMLK 制御構成変更_No.72 Chitra Added 2018/05/31 Start.
//#if !( defined  DEF_DenebMLKCus  ||  defined DEF_MonoCus )  //【EagleZPlus_BKZPlus】OP_BTS-11902対応 2019/09/11 HCL-Palani Commented.//【IT6_EagleBK】制御構成変更_No.30 対応 2019/06/03 HCL-Mangai Added.
	pc_CopyJobSetting.set_enum_const_skip_check( s_Environment.e_AutoColorLevel, &APIC_CopyJobSetting::setACSLevel );
//#endif //【EagleZPlus_BKZPlus】OP_BTS-11902対応 2019/09/11 HCL-Palani Commented.// DenebMLK 制御構成変更_No.72 Chitra Added 2018/05/31 End.
	//////////////////////////////////// .
	//	PowerSaveSetting.
	// ※便宜上.
	const OAPS_PowerSaveSetting& s_PowerSaveSetting = s_Environment.s_PowerSaveSetting;

	//		LowPowerMode.
	//			Value.
	// (一旦,現在の設定値をCommonより取得).
	TYPC_TimerOnOff c_TimerOnOff = pc_SystemSetting->getLowPowerMin();
	if( !s_PowerSaveSetting.s_LowPower.ss_Value.empty() ){
		c_TimerOnOff.ss_Timer = (short)s_PowerSaveSetting.s_LowPower.ss_Value.get();
		// (Commonに設定).
		pc_SystemSetting->setLowPowerMin( c_TimerOnOff );
	}

	//		SleepMode.
	// (一旦,現在の設定値をCommonより取得).
	::memset( &c_TimerOnOff, 0, sizeof(c_TimerOnOff) );
	c_TimerOnOff = pc_SystemSetting->getSleepMin();
Bool b_ManualStaplePriority = False;//【IT6_EagleH】RQ-771対応 HCL-Garima 2018/11/16 Mod.
	if( !s_PowerSaveSetting.s_SleepMode.empty() ){
	//			Enable.
		if( !s_PowerSaveSetting.s_SleepMode.e_Enable.empty() ){
//【IT6_EagleH】RQ-771対応 HCL-Garima 2018/11/16 Start.
#ifdef DEF_EagleCus	
	if( OAPD_VERSION_CHECK(p_req->getVersion(), OAPD_OVER_V74) == true ){  //IT6_EagleH UV VersionUp 25/02/2019 HCL-Mangai Modified.
		b_ManualStaplePriority = pc_SystemSetting->getManualStaplePriority();		
	}
#endif// DEF_EagleCus
//【IT6_EagleH】RQ-771対応 HCL-Garima 2018/11/16 End.
			if( (b_ManualStaplePriority == False )
			 || ((b_ManualStaplePriority == True )&& (OAPE_Bool_false == s_PowerSaveSetting.s_SleepMode.e_Enable))) {
				getCommAPIEnum( s_PowerSaveSetting.s_SleepMode.e_Enable, c_TimerOnOff.b_IsOn );
			 }
		}
	//			Value.
		if( !s_PowerSaveSetting.s_SleepMode.ss_Value.empty() ){
//【IT6_EagleH】RQ-771対応 HCL-Garima 2018/11/16 Start.
			if(b_ManualStaplePriority == False ) {
				c_TimerOnOff.ss_Timer = (short)s_PowerSaveSetting.s_SleepMode.ss_Value.get();
			}
//【IT6_EagleH】RQ-771対応 HCL-Garima 2018/11/16 End.
		}
		// (Commonに設定).
		pc_SystemSetting->setSleepMin( c_TimerOnOff );
	}
	//////////////////////////////////// .
	//	OutputSetting.
	// ※便宜上.
	const OAPS_OutputSetting& s_OutputSetting = s_Environment.s_OutputSetting;

	//		ReceivePrint.
	//			PcPrint.
	// 			> True：一括, False：受信同時.
	if( !s_OutputSetting.s_ReceivePrint.e_PcPrint.empty() ){
		if( OAPE_ReceivePrint_Switch_ReceiveEnd == s_OutputSetting.s_ReceivePrint.e_PcPrint.get() ){
			pc_CopyJobSetting->setRxFileLumpSumPrintOnAtPrint( True );
		}else{
			pc_CopyJobSetting->setRxFileLumpSumPrintOnAtPrint( False );
		}
	}
	// 			FaxPrint
	// 			> Off：一括, On：受信同時.
	if( !s_OutputSetting.s_ReceivePrint.e_FaxPrint.empty() ){
		if( OAPE_ReceivePrint_Switch_ReceiveEnd == s_OutputSetting.s_ReceivePrint.e_FaxPrint.get() ){
			// 一括(ジョブ受信).
			pc_FaxJobSetting->setFaxRcvPage( TYPE_FaxRcvPage_Off );
		}else{
			// 同時(ページ受信).
			pc_FaxJobSetting->setFaxRcvPage( TYPE_FaxRcvPage_On );
		}
	}

	//		OutputTrayPriority.
	//			Copy,Print,Fax,PrintList.
	setOutputTrayPriority( pc_CopyJobSetting, s_OutputSetting.s_OutputTrayPriority );

	//////////////////////////////////// .
	// AutoExpoLevel.
	pc_CopyJobSetting.set_enum_const_skip_check( s_Environment.e_AutoExpoLevel, &APIC_CopyJobSetting::setAutoExpoLevel );
//ZeusMLK CQ369728 東野史明 2012/08/20 Start.
	if( !s_Environment.e_AutoExpoLevel.empty() ){
		TYPE_AutoExpoLevel	e_AutoExpoLevel = TYPE_AutoExpoLevelN;	// ZeusMLK静的解析 2012/11/05 Sathiyapriyan CID:454917 Added. 
		getCommAPIEnum( s_Environment.e_AutoExpoLevel, e_AutoExpoLevel );
		pc_ScannerServiceSetting->setScannerAdjust( TYPE_ScannerAdjustAEReference, (short)e_AutoExpoLevel );
	}
//ZeusMLK CQ369728 東野史明 2012/08/20 End.

	//////////////////////////////////// .
	// SmallSizeOriginal.
	pc_CopyJobSetting.set_enum_skip_check( s_Environment.e_SmallSizeOriginal, &APIC_CopyJobSetting::setSmallSizeOriginal );

	//////////////////////////////////// .
	// BlankPagePrint.
	pc_CopyJobSetting.set_enum_skip_check( s_Environment.e_BlankPagePrint, &APIC_CopyJobSetting::setStampPrintAtBlankPage );

	//////////////////////////////////// .
	// PagePrintPosition.
	pc_OperationSetting.set_enum_skip_check( s_Environment.e_PagePrintPosition, &APIC_OperationSetting::setPagePrintPosition );

	//////////////////////////////////// .
	// PlatenSeparateScan.
	//[IT6_SPARROW]WP_No.206-1 2018/03/16 Hcl-Neeraj Modified Start.
	//TYPE_MachineType e_MachineType = pc_SystemInfo->getMachineType();
	if(!(OAPI_CheckIsSparrowSFPModel())) //IT6_SparrowBK 制御構成変更_No.36 HCL-Palani 2019/08/06 Modified.
	{
		pc_ScanJobSetting.set_enum_skip_check( s_Environment.e_PlatenSeparateScan, &APIC_ScanJobSetting::setPlatenSeparateScan );
	}
	//[IT6_SPARROW]WP_No.206-1 2018/03/16 Hcl-Neeraj Modified End.

	//////////////////////////////////// .
	// BlankDetectLevel.
	if( !s_Environment.sc_BlankDetectLevel.empty() ){
		pc_ScanerSetting->setBlankDetectLevel( (TYPE_BlankDetectLevel)s_Environment.sc_BlankDetectLevel.get() );		// (TYPE_BlankDetectLevel is char).
	}
	// MinervaZeus3rd UV制御構成変更_No.1202 Bairavi 2013/05/21 Added Start. 
	if( OAPD_VERSION_CHECK(p_req->getVersion(), OAPD_OVER_V53) ) {
		pc_SystemInfo.set_enum_skip_check( s_Environment.e_LeftAreaDefaultDisplay, &APIC_SystemInfo::setLeftAreaDefaultDisplay );
	}
	// MinervaZeus3rd UV制御構成変更_No.1202 Bairavi 2013/05/21 Added End.
	// MinervaSBK KRQ-94 05/05/2015 Sahana Added Start.
	// 重送検知機能使用設定.
	if( !s_Environment.e_MultiFeedDetection.empty()){
#ifndef DEF_HeliosMLKCus // 2019/10/14 【IT6_HeliosMLK】Machine No.17-1 HIENG Added.
		TYPE_Setting b_MultiFeedDetectSetting = (s_Environment.e_MultiFeedDetection == OAPE_OnOff_On ? TYPE_Setting_ON : TYPE_Setting_OFF);
		pc_SystemInfo->setMultiFeedDetectSetting (b_MultiFeedDetectSetting);
#endif // 2019/10/14 【IT6_HeliosMLK】Machine No.17-1 HIENG Added.
	}
	// MinervaSBK KRQ-94 05/05/2015 Sahana Added End

//IT5_3.4 制御構成変更_No.37 2016/12/12 add start.
	// DefaultBoxSetting.
	const OAPS_DefaultBoxSetting& s_DefaultBoxSetting = s_Environment.s_DefaultBoxSetting;
	if(!s_DefaultBoxSetting.empty())
	{
		StackGuard_Job pc_Job(TYPE_JobTypeFaxSendMemory);
		if(!pc_Job)
		{
			e.setKind(OAPE_Fault_faultcode_Server);
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance(Job)");
			return false;
		}
	
		TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
		if(!pc_JobSpec
		|| !pc_JobSpec->pc_Common
		|| !pc_JobSpec->pc_Scan
		|| !pc_JobSpec->pc_Send)
		{
			e.setKind(OAPE_Fault_faultcode_Server);
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance(JobSpec)");
			return false;
		}
		// JobSpec直下.
		OAPC_ApiWrapper<TYPC_Common> pc_Common(pc_JobSpec->pc_Common); 
		OAPC_ApiWrapper<TYPC_Scan>	 pc_Scan(pc_JobSpec->pc_Scan); 
		OAPC_ApiWrapper<TYPC_Send>	 pc_Send(pc_JobSpec->pc_Send); 
		// TYPC_Common.
		OAPC_ApiWrapper<TYPC_Resolution>		pc_Resolution(&pc_Common->c_Resolution); 
		OAPC_ApiWrapper<TYPC_Resolution>		pc_FaxResolution(&pc_Common->c_FaxResolution); 
		// TYPC_Send.
		OAPC_ApiWrapper<TYPC_ComSend>			pc_ComSend(&pc_Send->c_ComSend); 
		
		// Color.
		if(!s_DefaultBoxSetting.e_Color.empty())
		{
			Bool b_AcsOn = False;
			TYPE_ColorMode e_ColorMode = TYPE_ColorModeUnselect;
			Bool b_Binary = False;
			bool flag = true;
			switch(s_DefaultBoxSetting.e_Color.get())
			{
			  case OAPE_Acs_Auto:
				b_AcsOn     = True;
				e_ColorMode = TYPE_ColorModeUnselect;
				b_Binary    = False;
				break;
			  case OAPE_Acs_FullColor:
				b_AcsOn     = False;
				e_ColorMode = TYPE_ColorModeFullColor;
				b_Binary    = False;
				break;
			  case OAPE_Acs_Grayscale:
				b_AcsOn     = False;
				e_ColorMode = TYPE_ColorModeBlack;
				b_Binary    = False;
				break;
			  case OAPE_Acs_Monochrome:
				b_AcsOn     = False;
				e_ColorMode = TYPE_ColorModeUnselect;
				b_Binary    = True;
				break;
			  default:
				flag = false;
				break;
			}
			if(flag)
			{
				pc_Scan->setAcsOn(b_AcsOn);
				pc_Common->setColorMode(e_ColorMode);
				pc_Common->setBinary(b_Binary);
			}
		}

		// FileFormat.
		pc_ComSend.set_enum_skip_check(s_DefaultBoxSetting.e_FileFormat, &TYPC_ComSend::setFileFormat);
		if(!s_DefaultBoxSetting.e_FileFormat.empty()
		&& OAPE_FileFormat_Jpeg == s_DefaultBoxSetting.e_FileFormat)
		{ // Jpegの時は、ページ分割設定を1ページに固定(パネルと同じ処理).
			pc_ComSend->setPageSetting(TYPE_PageSettingSpecifiedPage);
			pc_ComSend->setSpecifiedPage(1);	// 1ページ固定.
		}

		// Resolution.
		pc_Resolution.set_enum_skip_check(s_DefaultBoxSetting.e_Resolution, &TYPC_Resolution::setFaxResolution);

		// FaxResolution.
		pc_FaxResolution.set_enum_skip_check(s_DefaultBoxSetting.e_FaxResolution, &TYPC_Resolution::setFaxResolution);

		pc_OperationSetting.set_enum_skip_check(s_DefaultBoxSetting.e_DefaultSetting, &APIC_OperationSetting::setInitModeScanToMediaOn);
		pc_OperationSetting->setInitModeScanToMedia(pc_JobSpec);
	}
//IT5_3.4 制御構成変更_No.37 2016/12/12 add end.
//【Eagle】RQ-720紙種自動検知 2017/11/07 HCL-Garima Start.
	//const OAPS_AutoPaperTypeDetectSetting& s_AutoPaperTypeDetectSetting = s_Environment.s_AutoPaperTypeDetectSetting;	//Eagle 1029580 64bitコンパイルエラー対応依頼 HCL-Palani 2018/03/05 Modified.
//【EagleH】RQ-720対応 2018/12/04 HCL-Garima Start.

#ifdef DEF_EagleCus	
	if(!s_Environment.s_AutoPaperTypeDetectSetting.empty())
	{
		TYPE_KeyMode e_KeyMode = pc_MachineSetting->getKeyMode();
		if(OAPD_VERSION_CHECK(p_req->getVersion(), OAPD_OVER_V74) == true){  //IT6_EagleH UV VersionUp 25/02/2019 HCL-Mangai Modified. 
//【EagleH】RQ-720対応 OP_BTS-1739 2018/12/25 HCL-Garima Start.
		 	if( ( (!s_Environment.s_AutoPaperTypeDetectSetting.e_ModeStatus.empty() &&  s_Environment.s_AutoPaperTypeDetectSetting.e_ModeStatus == OAPE_Bool_true )
			 || ( True == pc_SystemSetting->getAutoDetectedPaperType(TYPE_AutoDetectedPaperTypeModeStatus))	)
				&& (e_KeyMode  != TYPE_KeyModeKeyCounter 
					&& e_KeyMode != TYPE_KeyModeVendorKM
					&& e_KeyMode != TYPE_KeyModeAdminMachineKM))
		//【EagleH】RQ-720対応 OP_BTS-1739 2018/12/25 HCL-Garima End.
			{	

					if( !s_Environment.s_AutoPaperTypeDetectSetting.e_Auto.empty() ){
						Bool b_Auto = False;
						getCommAPIEnum( s_Environment.s_AutoPaperTypeDetectSetting.e_Auto, b_Auto );
						pc_SystemSetting->setAutoDetectedPaperType( TYPE_AutoDetectedPaperTypeMode ,b_Auto );
					}

					if(pc_SystemSetting->getAutoDetectedPaperType(TYPE_AutoDetectedPaperTypeMode) == True)//【Eagle】RQ-720紙種自動検知 2017/11/07 HCL-Garima Mod.
					{
						if( !s_Environment.s_AutoPaperTypeDetectSetting.e_Tray1.empty() ){
							Bool b_Tray1 = False;
							getCommAPIEnum( s_Environment.s_AutoPaperTypeDetectSetting.e_Tray1, b_Tray1 );
							pc_SystemSetting->setAutoDetectedPaperType( TYPE_AutoDetectedPaperTypeTray1Mode ,b_Tray1 );
						}
							if( !s_Environment.s_AutoPaperTypeDetectSetting.e_Tray2.empty() ){
							Bool b_Tray2 = False;
							getCommAPIEnum( s_Environment.s_AutoPaperTypeDetectSetting.e_Tray2, b_Tray2 );
							pc_SystemSetting->setAutoDetectedPaperType( TYPE_AutoDetectedPaperTypeTray2Mode ,b_Tray2 );
						}
						if( !s_Environment.s_AutoPaperTypeDetectSetting.e_Tray3.empty() ){
							Bool b_Tray3 = False;
							getCommAPIEnum( s_Environment.s_AutoPaperTypeDetectSetting.e_Tray3, b_Tray3 );
							pc_SystemSetting->setAutoDetectedPaperType( TYPE_AutoDetectedPaperTypeTray3Mode ,b_Tray3 );
						}
						if( !s_Environment.s_AutoPaperTypeDetectSetting.e_Tray4.empty() ){
							Bool b_Tray4 = False;
							getCommAPIEnum( s_Environment.s_AutoPaperTypeDetectSetting.e_Tray4, b_Tray4 );
							pc_SystemSetting->setAutoDetectedPaperType( TYPE_AutoDetectedPaperTypeTray4Mode ,b_Tray4 );
						}
						// AR1039470 HCL-Kailash Mod-Start 2018/03/22.
						if( !s_Environment.s_AutoPaperTypeDetectSetting.e_LCT.empty() ){
							Bool b_Lct = False;
							getCommAPIEnum( s_Environment.s_AutoPaperTypeDetectSetting.e_LCT, b_Lct );
							pc_SystemSetting->setAutoDetectedPaperType( TYPE_AutoDetectedPaperTypeLCTMode ,b_Lct ); // AR 1063586 HCL-Kailash 2018/10/12 Mod.
						}
						if( !s_Environment.s_AutoPaperTypeDetectSetting.e_ManualFeedTray.empty() ){
							Bool b_Manual = False;
							getCommAPIEnum( s_Environment.s_AutoPaperTypeDetectSetting.e_ManualFeedTray, b_Manual );
							pc_SystemSetting->setAutoDetectedPaperType( TYPE_AutoDetectedPaperTypeManualFeedMode ,b_Manual );
						}
						// AR1039470 HCL-Kailash Mod-End 2018/03/22.
						if( !s_Environment.s_AutoPaperTypeDetectSetting.e_ModeStatus.empty() ){
							Bool b_ModeStatus = False;
							getCommAPIEnum( s_Environment.s_AutoPaperTypeDetectSetting.e_ModeStatus, b_ModeStatus );
							pc_SystemSetting->setAutoDetectedPaperType( TYPE_AutoDetectedPaperTypeModeStatus ,b_ModeStatus );
						}
					}
			}
		}else{ //【EagleH】RQ-720対応 2018/12/04 HCL-Garima End.
		 if( !s_Environment.s_AutoPaperTypeDetectSetting.e_Auto.empty() ){
				Bool b_Auto = False;
				getCommAPIEnum( s_Environment.s_AutoPaperTypeDetectSetting.e_Auto, b_Auto );
				pc_SystemSetting->setAutoDetectedPaperType( TYPE_AutoDetectedPaperTypeMode ,b_Auto );
			}
			if( !s_Environment.s_AutoPaperTypeDetectSetting.e_Tray1.empty() ){
				Bool b_Tray1 = False;
				getCommAPIEnum( s_Environment.s_AutoPaperTypeDetectSetting.e_Tray1, b_Tray1 );
				pc_SystemSetting->setAutoDetectedPaperType( TYPE_AutoDetectedPaperTypeTray1Mode ,b_Tray1 );
			}
				if( !s_Environment.s_AutoPaperTypeDetectSetting.e_Tray2.empty() ){
				Bool b_Tray2 = False;
				getCommAPIEnum( s_Environment.s_AutoPaperTypeDetectSetting.e_Tray2, b_Tray2 );
				pc_SystemSetting->setAutoDetectedPaperType( TYPE_AutoDetectedPaperTypeTray2Mode ,b_Tray2 );
			}
			if( !s_Environment.s_AutoPaperTypeDetectSetting.e_Tray3.empty() ){
				Bool b_Tray3 = False;
				getCommAPIEnum( s_Environment.s_AutoPaperTypeDetectSetting.e_Tray3, b_Tray3 );
				pc_SystemSetting->setAutoDetectedPaperType( TYPE_AutoDetectedPaperTypeTray3Mode ,b_Tray3 );
			}
			if( !s_Environment.s_AutoPaperTypeDetectSetting.e_Tray4.empty() ){
				Bool b_Tray4 = False;
				getCommAPIEnum( s_Environment.s_AutoPaperTypeDetectSetting.e_Tray4, b_Tray4 );
				pc_SystemSetting->setAutoDetectedPaperType( TYPE_AutoDetectedPaperTypeTray4Mode ,b_Tray4 );
			}
			// AR1039470 HCL-Kailash Mod-Start 2018/03/22.
			if( !s_Environment.s_AutoPaperTypeDetectSetting.e_LCT.empty() ){
				Bool b_Lct = False;
				getCommAPIEnum( s_Environment.s_AutoPaperTypeDetectSetting.e_LCT, b_Lct );
				pc_SystemSetting->setAutoDetectedPaperType( TYPE_AutoDetectedPaperTypeLCTMode ,b_Lct ); // AR 1063586 HCL-Kailash 2018/10/12 Mod.
			}
			if( !s_Environment.s_AutoPaperTypeDetectSetting.e_ManualFeedTray.empty() ){
				Bool b_Manual = False;
				getCommAPIEnum( s_Environment.s_AutoPaperTypeDetectSetting.e_ManualFeedTray, b_Manual );
				pc_SystemSetting->setAutoDetectedPaperType( TYPE_AutoDetectedPaperTypeManualFeedMode ,b_Manual );
			}
			// AR1039470 HCL-Kailash Mod-End 2018/03/22.
			if( !s_Environment.s_AutoPaperTypeDetectSetting.e_ModeStatus.empty() ){
				Bool b_ModeStatus = False;
				getCommAPIEnum( s_Environment.s_AutoPaperTypeDetectSetting.e_ModeStatus, b_ModeStatus );
				pc_SystemSetting->setAutoDetectedPaperType( TYPE_AutoDetectedPaperTypeModeStatus ,b_ModeStatus );
			}
		}
	}
#endif
//【Eagle】RQ-720紙種自動検知 2017/11/07 HCL-Garima end.
//【EagleH】RQ-720対応 2018/12/04 HCL-Garima Start.
#ifdef DEF_EagleCus	
		//IT6_EagleH UV VersionUp 25/02/2019 HCL-Mangai Modified.
		if( OAPD_VERSION_CHECK(p_req->getVersion(), OAPD_OVER_V74) == true  
			&& !s_Environment.e_EnvelopeDetectSetting.empty() ) {
			if( True == pc_SystemSetting->getAutoDetectedPaperType(TYPE_AutoDetectedPaperTypeModeStatus) )
			{
				switch(s_Environment.e_EnvelopeDetectSetting.get())
				{
					case OAPE_EnvelopeDetectSetting_Detect:
						pc_PrinterServiceSetting->setPrinterAdjust(TYPE_PrinterAdjustEnvelopeDetect, 0);
						break;
					case OAPE_EnvelopeDetectSetting_DonotDetect:
						pc_PrinterServiceSetting->setPrinterAdjust(TYPE_PrinterAdjustEnvelopeDetect, 1);
						break;
					default:
						break;
				}

			}
		}
#endif
//【EagleH】RQ-720対応 2018/12/04 HCL-Garima End.

	// FPT-VanPN2【IT6_2.3】制御構成変更_No.52 2022/11/10 ADD Start.
	if( OAPD_VERSION_CHECK(p_req->getVersion(), OAPD_OVER_V734) == true  
			&& !s_Environment.s_WizardSetting.empty() ) 
	{
		const OAPS_WizardSetting& s_WizardSetting = s_Environment.s_WizardSetting;
		if(!s_WizardSetting.e_Country.empty())
		{
			TYPE_CountryType e_CountryCode = TYPE_CountryType_default;
			getCommAPIEnum(s_WizardSetting.e_Country, e_CountryCode);
			pc_SystemSetting->setCountryType(e_CountryCode);
		}
		if(!s_WizardSetting.e_NetworkCountrySupport.empty())
		{
			TYPE_CountryType e_NetworkCode = TYPE_CountryType_default;
			getCommAPIEnum(s_WizardSetting.e_NetworkCountrySupport, e_NetworkCode);
			pc_MachineSetting->setWirelessLANDestinationSetting(e_NetworkCode);
#if defined(DEF_ZeusMLKBoard)
			OAPC_ApiWrapper<SYSC_SystemInfo> 	pc_SyscSystemInfo(SYSC_SystemInfo::newInstance());
			if(pc_SyscSystemInfo)
			{
				pc_SyscSystemInfo->SetDeviceInfoToMio(TYPE_MioSetDeviceSelect_WLAN_INITIALIZE);
			}
#endif
		}
		if(!s_WizardSetting.e_FaxCountrySupport.empty())
		{
			TYPE_CountryType e_FaxCode = TYPE_CountryType_default;
			TYPE_CountryType e_FaxCodeCurrent = pc_MachineSetting->getCountryCode();
			getCommAPIEnum(s_WizardSetting.e_FaxCountrySupport, e_FaxCode);
			if(e_FaxCode != e_FaxCodeCurrent)
			{
				pc_MachineSetting->setCountryCode( e_FaxCode );
				APIC_FaxDevice* p_faxdev = APIC_FaxDevice::newInstanceAdmin();
				if(p_faxdev)
				{
					OAPC_ApiLock<APIC_FaxPanelSetting>			pc_FaxPanelSetting(APIC_FaxPanelSetting::newInstanceAdmin(), 			0);
					OAPC_ApiLock<APIC_FaxGfpSetting>			pc_FaxGfpSetting(APIC_FaxGfpSetting::newInstanceAdmin(), 				0);
					OAPC_ApiLock<APIC_FaxGhpSetting>			pc_FaxGhpSetting(APIC_FaxGhpSetting::newInstance(), 					0);
					OAPC_ApiLock<APIC_FaxGswSetting>			pc_FaxGswSetting(APIC_FaxGswSetting::newInstance(), 					0);
					OAPC_ApiLock<APIC_FaxJobSetting>			pc_FaxJobSetting(APIC_FaxJobSetting::newInstanceAdmin(), 				0);
					OAPC_ApiLock<APIC_FaxListReportSetting>		pc_FaxListReportSetting(APIC_FaxListReportSetting::newInstanceAdmin(), 	0);
					OAPC_ApiLock<APIC_FaxVoipSetting>			pc_FaxVoipSetting(APIC_FaxVoipSetting::newInstanceAdmin(), 				0);
					// 仕向を取得
					TYPE_SystemType	e_WorkSystemType = pc_MachineSetting->getSystemType();
					// ファクス機能設定を指定された仕向けの工場出荷値で設定
					pc_FaxPanelSetting&&pc_FaxPanelSetting->FactorySetup(e_WorkSystemType);
					// Ｇｆｐを指定された仕向けの工場出荷値で設定
					pc_FaxGfpSetting&&pc_FaxGfpSetting->FactorySetup(e_WorkSystemType);
					// Ｇｈｐを指定された仕向けの工場出荷値で設定
					pc_FaxGhpSetting&&pc_FaxGhpSetting->FactorySetup(e_WorkSystemType);
					// Ｇｓｗを指定された仕向けの工場出荷値で設定
					pc_FaxGswSetting&&pc_FaxGswSetting->FactorySetup(e_WorkSystemType);
					// Ｊｏｂを指定された仕向けの工場出荷値で設定
					pc_FaxJobSetting&&pc_FaxJobSetting->FactorySetup(e_WorkSystemType);
					// ファクスリストレポートを指定された仕向けの工場出荷値で設定.
					pc_FaxListReportSetting&&pc_FaxListReportSetting->FactorySetup(e_WorkSystemType);
					OAPC_ApiLock<APIC_SoftDipSw>	pc_SoftDipSw(APIC_SoftDipSw::newInstanceAdmin(), 0);
					if(pc_SoftDipSw && True == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( 237, 5 ))
					{
						pc_FaxVoipSetting&&pc_FaxVoipSetting->FactorySetup( e_WorkSystemType );
					}
					TYPE_CommLine e_LineNo = TYPE_CommLine1;
					char c_LineStatus = p_faxdev->getLineStatus( e_LineNo ) ;
					if( ( FAXE_FaxLineState )c_LineStatus == FAXE_LineStateIdle ) 
					{
						APIC_Fax* pFaxApi = APIC_Fax::instance( e_LineNo ) ;
						if(pFaxApi)
						{
							pFaxApi->dataChanged();
						}
					}
				}
			}
		}
		// FPT-VanPN2【IT6_2.3】制御構成変更_No.52 2022/12/05 ADD Start.
		if(!s_WizardSetting.e_TimeZoneCity.empty())
		{
			TYPE_TimezoneType e_TimezoneType = TYPE_TimezoneType_default;
			if(getCommAPIEnum(s_WizardSetting.e_TimeZoneCity, e_TimezoneType))
			{
				pc_SystemSetting->setTimezoneType(e_TimezoneType);
			}
		}
		// FPT-VanPN2【IT6_2.3】制御構成変更_No.52 2022/12/05 ADD End.
	}
	// FPT-VanPN2【IT6_2.3】制御構成変更_No.52 2022/11/10 ADD End.
	return true;
}

// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=Environment) Get.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Environment>::get(const OAPS_AppReqGetUtilitySetting* p_req, OAPS_AppResGetUtilitySetting* p_res, OAPC_SyncLock& _slock, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqGetUtilitySetting(RequestItem=Environment)::get()" );

	e.setMessage( OAPE_Result_FaultRequest_AppReqGetUtilitySetting );

	//////////////////////////////////// .
	// I/F Lock .
	OAPC_ApiLock<APIC_MachineSetting> pc_MachineSetting(APIC_MachineSetting::newInstanceAdmin() , &_slock);
	OAPC_ApiLock<APIC_OperationSetting> pc_OperationSetting(APIC_OperationSetting::newInstanceAdmin() , &_slock);
	OAPC_ApiLock<APIC_CopyJobSetting> pc_CopyJobSetting(APIC_CopyJobSetting::newInstanceAdmin(), &_slock);
	OAPC_ApiLock<APIC_PrintJobSetting> pc_PrintJobSetting(APIC_PrintJobSetting::newInstanceAdmin(), &_slock);
	OAPC_ApiLock<APIC_FeedTray> pc_FeedTray(APIC_FeedTray::newInstanceAdmin(), &_slock);
	OAPC_ApiLock<APIC_FaxJobSetting> pc_FaxJobSetting(APIC_FaxJobSetting::newInstanceAdmin(), &_slock);
	OAPC_ApiLock<APIC_SystemSetting> pc_SystemSetting(APIC_SystemSetting::newInstanceAdmin(), &_slock);
	OAPC_ApiLock<APIC_ScanJobSetting> pc_ScanJobSetting(APIC_ScanJobSetting::newInstanceAdmin(), &_slock);
	OAPC_ApiLock<APIC_ScanerSetting> pc_ScanerSetting(APIC_ScanerSetting::newInstanceAdmin(), &_slock);
	OAPC_ApiLock<APIC_PrinterSystem>   pc_PrinterSystem(APIC_PrinterSystem::newInstanceAdmin(), &_slock);
//ZeusMLK CQ369728 東野史明 2012/08/20 Start.
	OAPC_ApiLock<APIC_ScannerServiceSetting>   pc_ScannerServiceSetting(APIC_ScannerServiceSetting::newInstanceAdmin(), &_slock);
//ZeusMLK CQ369728 東野史明 2012/08/20 End.
	OAPC_ApiLock<APIC_SystemInfo> 		pc_SystemInfo( APIC_SystemInfo::newInstanceAdmin() , &_slock ); // MinervaZeus3rd UV制御構成変更_No.1202 Bairavi 2013/05/21 Added. 
	OAPC_ApiLock<APIC_PrinterServiceSetting>	pc_PrinterServiceSetting(APIC_PrinterServiceSetting::newInstanceAdmin(), &_slock);//【EagleH】RQ-720対応 2018/12/04 HCL-Garima Added.


	if(( !pc_MachineSetting 	) ||
	   ( !pc_OperationSetting 	) ||
	   ( !pc_CopyJobSetting 	) ||
	   ( !pc_PrintJobSetting 	) ||
	   ( !pc_FeedTray 			) ||
	   ( !pc_FaxJobSetting 		) ||
	   ( !pc_SystemSetting 		) ||
	   ( !pc_ScanJobSetting 	) ||
	   ( !pc_ScanerSetting 		) ||
	   ( !pc_PrinterSystem 		) ||
//ZeusMLK CQ369728 東野史明 2012/08/20 Start.
	   ( !pc_ScannerServiceSetting 	) ||
//ZeusMLK CQ369728 東野史明 2012/08/20 End.
       ( !pc_SystemInfo 	) ||  // MinervaZeus3rd UV制御構成変更_No.1202 Bairavi 2013/05/21 Added.  
	   ( !pc_PrinterServiceSetting ))//【EagleH】RQ-720対応 2018/12/04 HCL-Garima Added.
	{
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance");
		return false;
	}
	
	// ※便宜上.
	OAPS_UtilitySetting_Environment& s_Environment = p_res->s_Environment;

	//////////////////////////////////// .
	//	LanguageSelection.
	// ※便宜上.
	OAPS_LanguageSelection& s_LanguageSelection = s_Environment.s_LanguageSelection;

	// 		Language.
	getOpenAPIEnum( pc_MachineSetting->getLanguage(), s_LanguageSelection.e_Language );
// [IT6_2.2MR2] 制御構成変更_No.34  2022/01/18 FPT-QuanPH5 Add Start.
	OAPF_loadKeyMapData();
	//List Display
	////////////////selection/////////////////
	OAPF_LanguageStartGamen();
	short ss_count = 0;
	for(int i_index = 0;i_index <OAP_MAX_LANGUAGES; i_index++)
	{
		if(NULL != m_pstOAP_LangTable[i_index])
		{
			getOpenAPIEnum(*m_pstOAP_LangTable[i_index], s_LanguageSelection.s_SupportLanguageListDisplay.e_Language[i_index]);
			char sc_LanguageName[OAPD_LANGUAGENAME_MAXSTRING] = {0, };
			pc_MachineSetting->getLanguageName( *m_pstOAP_LangTable[i_index], sc_LanguageName );
			setStringData(s_LanguageSelection.s_SupportLanguageNameListDisplay.sc_LanguageName[i_index], sc_LanguageName);
			ss_count++;
		}
	}
	s_LanguageSelection.s_SupportLanguageListDisplay.ss_ArraySize = ss_count;
	s_LanguageSelection.s_SupportLanguageNameListDisplay.ss_ArraySize = ss_count;

	//////////////Keyboard///////////////////
	c_SelectLangInfo.createSelectLangInfo();
	
	TYPE_Language e_teamp_OAPS_LangTbl = st_OAP_LangTbl[33];
	e_teamp_OAPS_LangTbl = -1;
	for(int i_indexArr1 = 0; i_indexArr1 < OAP_MAX_LANGUAGES; i_indexArr1++)
	{
		if( NULL != m_pstOAP_langTable )
		{
			m_pstOAP_langTable[i_indexArr1] = & e_teamp_OAPS_LangTbl;
			if( NULL != m_pstOAP_langTable[i_indexArr1])
			{
				*m_pstOAP_langTable[i_indexArr1] = -1;
			}
		}
	}

	OAPF_KeyBoardOAPF_LanguageStartGamen();
	short ss_count2 = 0;
	for(int i_index2 = 0;i_index2 <OAP_MAX_LANGUAGES; i_index2++)
	{
		if( (NULL != m_pstOAP_langTable[i_index2]) && (-1 != *m_pstOAP_langTable[i_index2]) )
		{
			getOpenAPIEnum(*m_pstOAP_langTable[i_index2], s_LanguageSelection.s_SoftwareKeyboardLanguage.s_SupportLanguageListDisplay.e_Language[i_index2]);
			char sc_LanguageName[OAPD_LANGUAGENAME_MAXSTRING] = {0, };
			pc_MachineSetting->getLanguageName( *m_pstOAP_langTable[i_index2], sc_LanguageName );
			setStringData(s_LanguageSelection.s_SoftwareKeyboardLanguage.s_SupportLanguageNameListDisplay.sc_LanguageName[i_index2], sc_LanguageName);
			ss_count2++;
		}
	}
	s_LanguageSelection.s_SoftwareKeyboardLanguage.s_SupportLanguageListDisplay.ss_ArraySize = ss_count2;
	s_LanguageSelection.s_SoftwareKeyboardLanguage.s_SupportLanguageNameListDisplay.ss_ArraySize = ss_count2;
	
	/////////////ExternalKeyBoard////////////

	short ss_count3 = 0;
	e_teamp_OAPS_LangTbl = -1;
	for(int i_indexArr2 = 0; i_indexArr2 < OAP_MAX_LANGUAGES; i_indexArr2++)
	{
		if( NULL != m_pstOAP_langTable )
		{
			m_pstOAP_langTable[i_indexArr2] = & e_teamp_OAPS_LangTbl;
			if( NULL != m_pstOAP_langTable[i_indexArr2])
			{
				*m_pstOAP_langTable[i_indexArr2] = -1;
			}
		}
	}
	OAPF_USBKeyBoardLanguageSetStartGamen();
	for(int i_index3 = 0;i_index3 <OAP_MAX_LANGUAGES; i_index3++)
	{
		if( ( NULL != m_pstOAP_langTable[i_index3] )&& (-1 != *m_pstOAP_langTable[i_index3]))
		{
			getOpenAPIEnum(*m_pstOAP_langTable[i_index3], s_LanguageSelection.s_ExtKeyboardLanguage.s_SupportLanguageListDisplay.e_Language[i_index3]);
			char sc_LanguageName[OAPD_LANGUAGENAME_MAXSTRING] = {0, };
			pc_MachineSetting->getLanguageName( *m_pstOAP_langTable[i_index3], sc_LanguageName );
			setStringData(s_LanguageSelection.s_ExtKeyboardLanguage.s_SupportLanguageNameListDisplay.sc_LanguageName[i_index3], sc_LanguageName);
			ss_count3++;
		}
	}
	s_LanguageSelection.s_ExtKeyboardLanguage.s_SupportLanguageListDisplay.ss_ArraySize = ss_count3;
	s_LanguageSelection.s_ExtKeyboardLanguage.s_SupportLanguageNameListDisplay.ss_ArraySize = ss_count3;
// [IT6_2.2MR2] 制御構成変更_No.34  2022/01/18 FPT-QuanPH5 Add End.

	// 		SoftwareKeyboardLanguage.
	// 			WithPanelLanguage,Language.
	getOpenAPIEnum( pc_MachineSetting->getSoftwareKeyboardWithPanalLanguage(), 	s_LanguageSelection.s_SoftwareKeyboardLanguage.e_WithPanelLanguage );
	getOpenAPIEnum( pc_OperationSetting->getKeyboardLanguage(), 				s_LanguageSelection.s_SoftwareKeyboardLanguage.e_Language );

	// 		ExtKeyboardLanguage.
	// 			WithPanelLanguage,Language.
	getOpenAPIEnum( pc_MachineSetting->getExtKeyboardWithPanalLanguage(), 		s_LanguageSelection.s_ExtKeyboardLanguage.e_WithPanelLanguage );
	getOpenAPIEnum( pc_MachineSetting->getExtKeyboardLanguage(), 				s_LanguageSelection.s_ExtKeyboardLanguage.e_Language );

	//////////////////////////////////// .
	// SelectKeyboard.
	getOpenAPIEnum( pc_OperationSetting->getKeyboardType(), s_Environment.e_SelectKeyboard );

	//////////////////////////////////// .
	// Keyboard Type.
    // IT5_4.2 RQ_633 WP No.08 Kailash 2017/08/28 Added Start.
 	getOpenAPIEnum( pc_OperationSetting->getKeyboardUISetting(), s_Environment.e_KeyboardType );
    // IT5_4.2 RQ_633 WP No.08 Kailash 2017/08/28 Added End.	
	// UnitSetting.
	getOpenAPIEnum( pc_OperationSetting->getUnitSystem(), s_Environment.e_UnitSetting );

	//////////////////////////////////// .
	//	PaperTraySetting.
	// ※便宜上.
	OAPS_PaperTraySetting& s_PaperTraySetting = s_Environment.s_PaperTraySetting;

	//		AutoTraySelect.

	// ※便宜上.
	OAPS_AutoTraySelect& s_AutoTraySelect = s_PaperTraySetting.s_AutoTraySelect;

	Bool b_All            = pc_CopyJobSetting->getAtsEnableAtUserSelectTray();
	Bool b_ATS_APS_1      = pc_CopyJobSetting->getAtsApsEnable_Tray_1();
	Bool b_ATS_APS_2      = pc_CopyJobSetting->getAtsApsEnable_Tray_2();
	Bool b_ATS_APS_3      = pc_CopyJobSetting->getAtsApsEnable_Tray_3();
	Bool b_ATS_APS_4      = pc_CopyJobSetting->getAtsApsEnable_Tray_4();
	Bool b_ATS_APS_5      = pc_CopyJobSetting->getAtsApsEnable_Tray_5();
	Bool b_ATS_APS_Manual = pc_CopyJobSetting->getAtsApsEnable_TrayManual();

	//			Enable.
	getOpenAPIEnum( b_All, 				s_AutoTraySelect.e_Enable );
	//			EachTray.
	//				Tray1.
	getOpenAPIEnum( b_ATS_APS_1, 		s_AutoTraySelect.s_EachTray.e_Tray1 );
	//				Tray2.
	getOpenAPIEnum( b_ATS_APS_2, 		s_AutoTraySelect.s_EachTray.e_Tray2 );
	//				Tray3.
	getOpenAPIEnum( b_ATS_APS_3, 		s_AutoTraySelect.s_EachTray.e_Tray3 );
	//				Tray4.
	getOpenAPIEnum( b_ATS_APS_4, 		s_AutoTraySelect.s_EachTray.e_Tray4 );
	//				Tray5(LCT).
	getOpenAPIEnum( b_ATS_APS_5, 		s_AutoTraySelect.s_EachTray.e_Lct );
	//				Hand(Manual).
	getOpenAPIEnum( b_ATS_APS_Manual, 	s_AutoTraySelect.s_EachTray.e_Hand );

	//		AutoTraySelect
	//			PriorityTrayList
	//				ArraySize.
	// 				PriorityTray.
	// 					Index.
	// 					TrayID.

	//.
	// ↓↓↓(以下、OAP_TRN_devicesetting_permanent.cpp <...CommonPaperHandling>::get() よりそのまま流用).
	//.
	TYPS_TraySearchPriority s_TraySearchPriority = pc_CopyJobSetting->getTraySearchPriority();
	short k = 0;
	s_AutoTraySelect.s_PriorityTrayList.ss_ArraySize = 0;	//配列サイズの初期化.
	// TYPS_TraySearchPriority で保持しているTYPE_FeedTrayの配列数が６の為、ループ回数を６としている.
	// TYPS_TraySearchPriority では直値で６と記述されている.
	for( short i = 0; i < (OAPD_PRIORITYTRAYLIST_PRIORITYTRAY_MAXLIST-1); i++ )
	{
		MsgEnum<OAPE_TrayID> e_Open_Tmp(VERALL, VERALL, NONREM);
		TYPE_FeedTray e_Com_Tmp = TYPE_FeedTray1;			// (初期化).
		
		// CommonAPIの値を取得.
		e_Com_Tmp = s_TraySearchPriority.e_FeedTray[i];
		
		if( TYPE_FeedTrayUnselect != e_Com_Tmp )
		{
			if( TYPE_TrayStatusNon != pc_FeedTray->getTrayStatus( e_Com_Tmp ) )
			{
				Bool Select = False;
				switch( e_Com_Tmp )
				{
					case TYPE_FeedTray1:		Select = b_ATS_APS_1;		break;
					case TYPE_FeedTray2:		Select = b_ATS_APS_2;		break;
					case TYPE_FeedTray3:		Select = b_ATS_APS_3;		break;
					case TYPE_FeedTray4:		Select = b_ATS_APS_4;		break;
					case TYPE_FeedTray5:		Select = b_ATS_APS_5;		break;
					case TYPE_FeedTrayManual:	Select = b_ATS_APS_Manual;	break;
					default:	break;
				}
				if( Select )
				{
					if ( getOpenAPIEnum( e_Com_Tmp, e_Open_Tmp ) )
					{
						if( k < OAPD_PRIORITYTRAYLIST_PRIORITYTRAY_MAXLIST )
						{
							s_AutoTraySelect.s_PriorityTrayList.s_PriorityTray[k].ss_Index = k+1;
							s_AutoTraySelect.s_PriorityTrayList.s_PriorityTray[k].e_TrayID = e_Open_Tmp;
							k++;
						}
					}
				}
			}
		}
	}
	s_AutoTraySelect.s_PriorityTrayList.ss_ArraySize = k;
	//.
	// ↑↑↑(以下、OAP_TRN_devicesetting_permanent.cpp <...CommonPaperHandling>::get() よりそのまま流用).
	//.

	//		AutoPaperSelect.
	// ※便宜上.
	OAPS_AutoPaperSelect& s_AutoPaperSelect = s_PaperTraySetting.s_AutoPaperSelect;

	// (Commonより取得) APS-用紙設定.
	TYPS_PaperTypeOnAPS* pc_PaperTypeOnAPS = pc_CopyJobSetting->getPaperTypeOnAPS();
	//			UserPaper1.
	getOpenAPIEnum( pc_PaperTypeOnAPS->b_TypeUser1, s_AutoPaperSelect.e_UserPaper1 );
//2018/02/15 HCL-Kailash 【Eagle】 RQ-722対応 Added Start.
#if ( !defined (DEF_EagleCus)  &&  !defined (DEF_SparrowCus) ) || defined (DEF_HeliosMLKCus)	// 2019/11/08 【HeliosMLK】 MachineNo.11 HIENG added.
	//			UserPaper2.
	getOpenAPIEnum( pc_PaperTypeOnAPS->b_TypeUser2, s_AutoPaperSelect.e_UserPaper2 );
#endif
//2018/02/15 HCL-Kailash 【Eagle】 RQ-722対応 Added End.
	//ZeusS CQ505523 Palani 2014/11/05 Added Start.
	//ThinPaper.
#if !( defined DEF_Zeus5BkCus || defined DEF_HeliosCus || defined DEF_SparrowCus || defined DEF_HeliosMLKCus )			//HeliosL 機器構成変更_No.20 Palani 2014/11/24 Added.	//2018/02/15 HCL-Kailash 【Eagle】 RQ-722対応 Modified. // 2019/11/08 【HeliosMLK】 MachineNo.11 HIENG added.
	getOpenAPIEnum( pc_PaperTypeOnAPS->b_TypeThin,  s_AutoPaperSelect.e_ThinPaper );
#endif
	//ZeusS CQ505523 Palani 2014/11/05 Added End.
	
	//			PriorityPaperTypeList.
	//				ArraySize.
	//				PriorityPaperType.
	// (Commonより取得) APS-優先用紙種類設定.
	TYPS_PaperTypeOnAPSPriority s_PaperTypeOnAPSPriority;
	::memset( &s_PaperTypeOnAPSPriority, 0, sizeof(s_PaperTypeOnAPSPriority) );
	pc_CopyJobSetting->getPaperTypeOnAPSPriority( &s_PaperTypeOnAPSPriority );
	// ※優先度：(高)配列[0]←→配列[2](低) で順番に設定されている.
	
	// ※便宜上.
	OAPS_PriorityPaperTypeList& s_PriorityPaperTypeList = s_PaperTraySetting.s_AutoPaperSelect.s_PriorityPaperTypeList;
	// IT6 Eagle/Sparrow 1057139 HCL-Jerin 2018/09/14 Modified .
	//for( int j = 0; j < OAPD_PRIORITYPAPERTYPELIST_MAXLIST; j++ ){					// (ここのArraySizeは固定なので,LoopのMaxも固定でOK).
	for( int j = 0; j < OAPD_PRIORITYPAPERTYPE_INDEX_HIGH; j++ ){					// (ここのArraySizeは固定なので,LoopのMaxも固定でOK).
		// 優先順位.
		// ※優先順位(Index)=[1-3]なので(+1).
		s_PriorityPaperTypeList.s_PriorityPaperType[j].ss_Index = (j + 1);
		// 用紙種類.
		getOpenAPIEnum( s_PaperTypeOnAPSPriority.e_PaperTypeOnAPSPriority[j], s_PriorityPaperTypeList.s_PriorityPaperType[j].e_PaperType );
	}
	// IT6 Eagle/Sparrow 1057139 HCL-Jerin 2018/09/14 Modified.
	//			ArraySize.
	//s_PriorityPaperTypeList.ss_ArraySize = OAPD_PRIORITYPAPERTYPELIST_MAXLIST;		// (固定).
	s_PriorityPaperTypeList.ss_ArraySize = OAPD_PRIORITYPAPERTYPE_INDEX_HIGH;		// (固定).

	//		TrayUnmatch.
	getOpenAPIEnum( pc_CopyJobSetting->getPaperSizeMismach(), s_PaperTraySetting.e_TrayUnmatch );

	//		ListPrint.
	getOpenAPIEnum( pc_PrintJobSetting->getFeedTrayAtListPrint(), 	 s_PaperTraySetting.s_ListPrint.e_Tray );
	getOpenAPIEnum( pc_PrintJobSetting->getDuplexPrintAtListPrint(), s_PaperTraySetting.s_ListPrint.e_Duplex );

// 2020/02/05 【Eagle Z+】OP_BTS-11760 HIENG Modified Start.
// 2020/02/19 【Eagle Z+】OP_BTS-20715(OP_BTS-11760) HIENG Modified Start.
	if( OAPD_VERSION_CHECK(p_req->getVersion(), OAPD_OVER_V77) ) {
// 2020/02/19 【Eagle Z+】OP_BTS-20715(OP_BTS-11760) HIENG Modified End.
// 2019/12/18 【Eagle Z+】OP_BTS-11760 HIENG Start.
// 2019/11/21 【HeliosMLK】 MachineNo.11 HIENG Start.
//#ifndef DEF_HeliosMLKCus
#if !defined (DEF_DenebMLKCus) && !defined (DEF_SparrowCus) && !defined (DEF_HeliosMLKCus)
	//		PostInserter.
	//			UpperTray.
	// ※便宜上.
	OAPS_PostInserter_Tray& s_UpperTray = s_PaperTraySetting.s_PostInserter.s_UpperTray;

	// (Commonより取得).
	TYPC_PaperType c_PaperType = pc_FeedTray->getPaperType( TYPE_FeedTrayPI1 );

	//				PaperType.
	getOpenAPIEnum( c_PaperType.getPaperType(), s_UpperTray.e_PaperType );

	//				OriginalSize.
	//					Auto.
	getOpenAPIEnum( pc_FeedTray->getPaperSizeAutoDetectOn( TYPE_FeedTrayPI1 ), s_UpperTray.s_PaperSize.e_Auto );
	
	//				OriginalSize.
	//					SizeCode.
	// (Commonより取得).
	TYPC_PaperSize c_PaperSize = pc_FeedTray->getPaperSize( TYPE_FeedTrayPI1 );
	//IT5_4.1 1015066 Sahana 2017/04/11 Added Start
	if(TYPE_SizeCodeUnknown != c_PaperSize.e_SizeCode)
    	getOpenAPIStruct(c_PaperSize, s_UpperTray.s_PaperSize);
	//IT5_4.1 1015066 Sahana 2017/04/11 Added End

//	getOpenAPIEnum( c_PaperSize.e_SizeCode, s_UpperTray.s_PaperSize.e_SizeCode );	//IT5_4.1 1015066 Sahana 2017/04/11 Commented.
	
	//				OriginalSize.
	//					Dimension.
	c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse();				// 2017/10/25 Pactera Added.
	s_UpperTray.s_PaperSize.s_Dimension2.f_X = c_PaperSize.c_SizeMM.x;
	s_UpperTray.s_PaperSize.s_Dimension2.f_Y = c_PaperSize.c_SizeMM.y;

	//2017/03/13 カバーフィーダー_XML基盤技術開発v2 pactera add start.
	if ( OAPD_VERSION_CHECK(p_req->getVersion(), OAPD_OVER_V999) == true ) {
		{
			char sc_RegisterName[OAPD_FREESIZEPAPER_REGISTERNAME];
			memset( sc_RegisterName, 0, OAPD_FREESIZEPAPER_REGISTERNAME );
			pc_OperationSetting->getPaperSizeUnfix01NameUTF8(sc_RegisterName,TYPE_TrayTypePI, TYPE_CharSetUTF8, TYPE_FeedTrayPI1);
			s_UpperTray.s_PaperSize.s_FreeSizePaper01.sc_RegisterName = sc_RegisterName;
			// c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse();
			TYPC_Point<float> c_Point1( 0.0f, 0.0f );
			c_Point1 = pc_OperationSetting->getPaperSizeUnfix01( TYPE_TrayTypePI, TYPE_FeedTrayPI1 );
			s_UpperTray.s_PaperSize.s_FreeSizePaper01.f_X = c_Point1.x;
			s_UpperTray.s_PaperSize.s_FreeSizePaper01.f_Y = c_Point1.y;
		}
		{
			char sc_RegisterName[OAPD_FREESIZEPAPER_REGISTERNAME];
			memset( sc_RegisterName, 0, OAPD_FREESIZEPAPER_REGISTERNAME );
			pc_OperationSetting->getPaperSizeUnfix02NameUTF8(sc_RegisterName,TYPE_TrayTypePI, TYPE_CharSetUTF8, TYPE_FeedTrayPI1);
			s_UpperTray.s_PaperSize.s_FreeSizePaper02.sc_RegisterName = sc_RegisterName;
			TYPC_Point<float> c_Point2( 0.0f, 0.0f );
			c_Point2 = pc_OperationSetting->getPaperSizeUnfix02( TYPE_TrayTypePI, TYPE_FeedTrayPI1 );
			s_UpperTray.s_PaperSize.s_FreeSizePaper02.f_X = c_Point2.x;
			s_UpperTray.s_PaperSize.s_FreeSizePaper02.f_Y = c_Point2.y;
		}
	}
	//2017/03/13 カバーフィーダー_XML基盤技術開発v2 pactera add end.

	//		PostInserter.
	//			LowerTray.
	// ※便宜上.
	OAPS_PostInserter_Tray& s_LowerTray = s_PaperTraySetting.s_PostInserter.s_LowerTray;

	// (Commonより取得).
	::memset( &c_PaperType, 0, sizeof(c_PaperType) );
	c_PaperType = pc_FeedTray->getPaperType( TYPE_FeedTrayPI2 );

	//				PaperType.
	getOpenAPIEnum( c_PaperType.getPaperType(), s_LowerTray.e_PaperType );

	//				OriginalSize.
	//					Auto.
	getOpenAPIEnum( pc_FeedTray->getPaperSizeAutoDetectOn( TYPE_FeedTrayPI2 ), s_LowerTray.s_PaperSize.e_Auto );
	
	//				OriginalSize.
	//					SizeCode.
	// (Commonより取得).
	c_PaperSize = pc_FeedTray->getPaperSize( TYPE_FeedTrayPI2 );
	//IT5_4.1 1015066 Sahana 2017/04/11 Added Start
	if(TYPE_SizeCodeUnknown != c_PaperSize.e_SizeCode)
	getOpenAPIStruct(c_PaperSize, s_LowerTray.s_PaperSize); 
	//IT5_4.1 1015066 Sahana 2017/04/11 Added End
//	getOpenAPIEnum( c_PaperSize.e_SizeCode, s_LowerTray.s_PaperSize.e_SizeCode );	//IT5_4.1 1015066 Sahana 2017/04/11 Commented.
	
	//				OriginalSize.
	//					Dimension.
	c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse();				// 2017/10/25 Pactera Added.
	s_LowerTray.s_PaperSize.s_Dimension2.f_X = c_PaperSize.c_SizeMM.x;
	s_LowerTray.s_PaperSize.s_Dimension2.f_Y = c_PaperSize.c_SizeMM.y;

	//2017/03/13 カバーフィーダー_XML基盤技術開発v2 pactera add start.
	if ( OAPD_VERSION_CHECK(p_req->getVersion(), OAPD_OVER_V999) == true ) {
		{
			char sc_RegisterName[OAPD_FREESIZEPAPER_REGISTERNAME];
			memset( sc_RegisterName, 0, OAPD_FREESIZEPAPER_REGISTERNAME );
			pc_OperationSetting->getPaperSizeUnfix01NameUTF8(sc_RegisterName,TYPE_TrayTypePI, TYPE_CharSetUTF8, TYPE_FeedTrayPI2);
			s_LowerTray.s_PaperSize.s_FreeSizePaper01.sc_RegisterName = sc_RegisterName;
			c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse();
			TYPC_Point<float> c_Point1( 0.0f, 0.0f );
			c_Point1 = pc_OperationSetting->getPaperSizeUnfix01( TYPE_TrayTypePI, TYPE_FeedTrayPI2 );
			s_LowerTray.s_PaperSize.s_FreeSizePaper01.f_X = c_Point1.x;
			s_LowerTray.s_PaperSize.s_FreeSizePaper01.f_Y = c_Point1.y;
		}
		{
			char sc_RegisterName[OAPD_FREESIZEPAPER_REGISTERNAME];
			memset( sc_RegisterName, 0, OAPD_FREESIZEPAPER_REGISTERNAME );
			pc_OperationSetting->getPaperSizeUnfix02NameUTF8(sc_RegisterName,TYPE_TrayTypePI, TYPE_CharSetUTF8, TYPE_FeedTrayPI2);
			s_LowerTray.s_PaperSize.s_FreeSizePaper02.sc_RegisterName = sc_RegisterName;
			TYPC_Point<float> c_Point2( 0.0f, 0.0f );
			c_Point2 = pc_OperationSetting->getPaperSizeUnfix02( TYPE_TrayTypePI, TYPE_FeedTrayPI2 );
			s_LowerTray.s_PaperSize.s_FreeSizePaper02.f_X = c_Point2.x;
			s_LowerTray.s_PaperSize.s_FreeSizePaper02.f_Y = c_Point2.y;
		}
	}
	//2017/03/13 カバーフィーダー_XML基盤技術開発v2 pactera add end.
#endif
// 2019/11/21 【HeliosMLK】 MachineNo.11 HIENG End.
// 2019/12/18 【Eagle Z+】OP_BTS-11760 HIENG End.
	} else {
	//		PostInserter.
	//			UpperTray.
	// ※便宜上.
	OAPS_PostInserter_Tray& s_UpperTray = s_PaperTraySetting.s_PostInserter.s_UpperTray;

	// (Commonより取得).
	TYPC_PaperType c_PaperType = pc_FeedTray->getPaperType( TYPE_FeedTrayPI1 );

	//				PaperType.
	getOpenAPIEnum( c_PaperType.getPaperType(), s_UpperTray.e_PaperType );

	//				OriginalSize.
	//					Auto.
	getOpenAPIEnum( pc_FeedTray->getPaperSizeAutoDetectOn( TYPE_FeedTrayPI1 ), s_UpperTray.s_PaperSize.e_Auto );
	
	//				OriginalSize.
	//					SizeCode.
	// (Commonより取得).
	TYPC_PaperSize c_PaperSize = pc_FeedTray->getPaperSize( TYPE_FeedTrayPI1 );
	//IT5_4.1 1015066 Sahana 2017/04/11 Added Start
	if(TYPE_SizeCodeUnknown != c_PaperSize.e_SizeCode)
    	getOpenAPIStruct(c_PaperSize, s_UpperTray.s_PaperSize);
	//IT5_4.1 1015066 Sahana 2017/04/11 Added End

//	getOpenAPIEnum( c_PaperSize.e_SizeCode, s_UpperTray.s_PaperSize.e_SizeCode );	//IT5_4.1 1015066 Sahana 2017/04/11 Commented.
	
	//				OriginalSize.
	//					Dimension.
	c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse();				// 2017/10/25 Pactera Added.
	s_UpperTray.s_PaperSize.s_Dimension2.f_X = c_PaperSize.c_SizeMM.x;
	s_UpperTray.s_PaperSize.s_Dimension2.f_Y = c_PaperSize.c_SizeMM.y;

	//2017/03/13 カバーフィーダー_XML基盤技術開発v2 pactera add start.
	if ( OAPD_VERSION_CHECK(p_req->getVersion(), OAPD_OVER_V999) == true ) {
		{
			char sc_RegisterName[OAPD_FREESIZEPAPER_REGISTERNAME];
			memset( sc_RegisterName, 0, OAPD_FREESIZEPAPER_REGISTERNAME );
			pc_OperationSetting->getPaperSizeUnfix01NameUTF8(sc_RegisterName,TYPE_TrayTypePI, TYPE_CharSetUTF8, TYPE_FeedTrayPI1);
			s_UpperTray.s_PaperSize.s_FreeSizePaper01.sc_RegisterName = sc_RegisterName;
			// c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse();
			TYPC_Point<float> c_Point1( 0.0f, 0.0f );
			c_Point1 = pc_OperationSetting->getPaperSizeUnfix01( TYPE_TrayTypePI, TYPE_FeedTrayPI1 );
			s_UpperTray.s_PaperSize.s_FreeSizePaper01.f_X = c_Point1.x;
			s_UpperTray.s_PaperSize.s_FreeSizePaper01.f_Y = c_Point1.y;
		}
		{
			char sc_RegisterName[OAPD_FREESIZEPAPER_REGISTERNAME];
			memset( sc_RegisterName, 0, OAPD_FREESIZEPAPER_REGISTERNAME );
			pc_OperationSetting->getPaperSizeUnfix02NameUTF8(sc_RegisterName,TYPE_TrayTypePI, TYPE_CharSetUTF8, TYPE_FeedTrayPI1);
			s_UpperTray.s_PaperSize.s_FreeSizePaper02.sc_RegisterName = sc_RegisterName;
			TYPC_Point<float> c_Point2( 0.0f, 0.0f );
			c_Point2 = pc_OperationSetting->getPaperSizeUnfix02( TYPE_TrayTypePI, TYPE_FeedTrayPI1 );
			s_UpperTray.s_PaperSize.s_FreeSizePaper02.f_X = c_Point2.x;
			s_UpperTray.s_PaperSize.s_FreeSizePaper02.f_Y = c_Point2.y;
		}
	}
	//2017/03/13 カバーフィーダー_XML基盤技術開発v2 pactera add end.

	//		PostInserter.
	//			LowerTray.
	// ※便宜上.
	OAPS_PostInserter_Tray& s_LowerTray = s_PaperTraySetting.s_PostInserter.s_LowerTray;

	// (Commonより取得).
	::memset( &c_PaperType, 0, sizeof(c_PaperType) );
	c_PaperType = pc_FeedTray->getPaperType( TYPE_FeedTrayPI2 );

	//				PaperType.
	getOpenAPIEnum( c_PaperType.getPaperType(), s_LowerTray.e_PaperType );

	//				OriginalSize.
	//					Auto.
	getOpenAPIEnum( pc_FeedTray->getPaperSizeAutoDetectOn( TYPE_FeedTrayPI2 ), s_LowerTray.s_PaperSize.e_Auto );
	
	//				OriginalSize.
	//					SizeCode.
	// (Commonより取得).
	c_PaperSize = pc_FeedTray->getPaperSize( TYPE_FeedTrayPI2 );
	//IT5_4.1 1015066 Sahana 2017/04/11 Added Start
	if(TYPE_SizeCodeUnknown != c_PaperSize.e_SizeCode)
	getOpenAPIStruct(c_PaperSize, s_LowerTray.s_PaperSize); 
	//IT5_4.1 1015066 Sahana 2017/04/11 Added End
//	getOpenAPIEnum( c_PaperSize.e_SizeCode, s_LowerTray.s_PaperSize.e_SizeCode );	//IT5_4.1 1015066 Sahana 2017/04/11 Commented.
	
	//				OriginalSize.
	//					Dimension.
	c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse();				// 2017/10/25 Pactera Added.
	s_LowerTray.s_PaperSize.s_Dimension2.f_X = c_PaperSize.c_SizeMM.x;
	s_LowerTray.s_PaperSize.s_Dimension2.f_Y = c_PaperSize.c_SizeMM.y;

	//2017/03/13 カバーフィーダー_XML基盤技術開発v2 pactera add start.
	if ( OAPD_VERSION_CHECK(p_req->getVersion(), OAPD_OVER_V999) == true ) {
		{
			char sc_RegisterName[OAPD_FREESIZEPAPER_REGISTERNAME];
			memset( sc_RegisterName, 0, OAPD_FREESIZEPAPER_REGISTERNAME );
			pc_OperationSetting->getPaperSizeUnfix01NameUTF8(sc_RegisterName,TYPE_TrayTypePI, TYPE_CharSetUTF8, TYPE_FeedTrayPI2);
			s_LowerTray.s_PaperSize.s_FreeSizePaper01.sc_RegisterName = sc_RegisterName;
			c_PaperSize.c_SizeMM = c_PaperSize.c_SizeMM.xyReverse();
			TYPC_Point<float> c_Point1( 0.0f, 0.0f );
			c_Point1 = pc_OperationSetting->getPaperSizeUnfix01( TYPE_TrayTypePI, TYPE_FeedTrayPI2 );
			s_LowerTray.s_PaperSize.s_FreeSizePaper01.f_X = c_Point1.x;
			s_LowerTray.s_PaperSize.s_FreeSizePaper01.f_Y = c_Point1.y;
		}
		{
			char sc_RegisterName[OAPD_FREESIZEPAPER_REGISTERNAME];
			memset( sc_RegisterName, 0, OAPD_FREESIZEPAPER_REGISTERNAME );
			pc_OperationSetting->getPaperSizeUnfix02NameUTF8(sc_RegisterName,TYPE_TrayTypePI, TYPE_CharSetUTF8, TYPE_FeedTrayPI2);
			s_LowerTray.s_PaperSize.s_FreeSizePaper02.sc_RegisterName = sc_RegisterName;
			TYPC_Point<float> c_Point2( 0.0f, 0.0f );
			c_Point2 = pc_OperationSetting->getPaperSizeUnfix02( TYPE_TrayTypePI, TYPE_FeedTrayPI2 );
			s_LowerTray.s_PaperSize.s_FreeSizePaper02.f_X = c_Point2.x;
			s_LowerTray.s_PaperSize.s_FreeSizePaper02.f_Y = c_Point2.y;
		}
	}
	//2017/03/13 カバーフィーダー_XML基盤技術開発v2 pactera add end.
	}
// 2020/02/05 【Eagle Z+】OP_BTS-11760 HIENG Modified End.

	//////////////////////////////////// .
//HeliosMLK OP_BTS-17036 HCL-Garima 21/02/2020 start.
		s_PaperTraySetting.ss_ManualTrayPaperSizeCD = pc_FeedTray->getManualTrayPaperSizeCD();
//HeliosMLK OP_BTS-17036 HCL-Garima 21/02/2020 end.	
	// AutoColorLevel.
//#ifndef DEF_DenebMLKCus //【IT6_EagleBK】制御構成変更_No.30 対応 2019/06/03 HCL-Mangai Commented. // DenebMLK 制御構成変更_No.72 Chitra Added 2018/05/31 Start.
//#if !( defined  DEF_DenebMLKCus  ||  defined DEF_MonoCus )   //【EagleZPlus_BKZPlus】OP_BTS-11902対応 2019/09/11 HCL-Palani Commented.//【IT6_EagleBK】制御構成変更_No.30 対応 2019/06/03 HCL-Mangai Added.
	getOpenAPIEnum( pc_CopyJobSetting->getACSLevel(), s_Environment.e_AutoColorLevel );
//#endif //【EagleZPlus_BKZPlus】OP_BTS-11902対応 2019/09/11 HCL-Palani Commented.// DenebMLK 制御構成変更_No.72 Chitra Added 2018/05/31 End.

	//////////////////////////////////// .
	//	PowerSaveSetting.
	// ※便宜上.
	OAPS_PowerSaveSetting& s_PowerSaveSetting = s_Environment.s_PowerSaveSetting;

	//		LowPowerMode.
	// (Commonより取得).
	TYPC_TimerOnOff c_TimerOnOff = pc_SystemSetting->getLowPowerMin();
	//			Value.
	s_PowerSaveSetting.s_LowPower.ss_Value = c_TimerOnOff.ss_Timer;

	//		SleepMode.
	// (Commonより取得).
	::memset( &c_TimerOnOff, 0, sizeof(c_TimerOnOff) );
	c_TimerOnOff = pc_SystemSetting->getSleepMin();
	//			Enable.
	getOpenAPIEnum( c_TimerOnOff.b_IsOn, s_PowerSaveSetting.s_SleepMode.e_Enable );
	//			Value.
	s_PowerSaveSetting.s_SleepMode.ss_Value = c_TimerOnOff.ss_Timer;

	//////////////////////////////////// .
	//	OutputSetting.
	// ※便宜上.
	OAPS_OutputSetting& s_OutputSetting = s_Environment.s_OutputSetting;

	//		ReceivePrint.
	//			PcPrint.
	// 			> True：一括, False：受信同時.
	if( pc_CopyJobSetting->getRxFileLumpSumPrintOnAtPrint() ) {
		s_OutputSetting.s_ReceivePrint.e_PcPrint = OAPE_ReceivePrint_Switch_ReceiveEnd;
	} else {
		s_OutputSetting.s_ReceivePrint.e_PcPrint = OAPE_ReceivePrint_Switch_Receive;
	}
	//			PcFax.
	// 			> Off：一括, On：受信同時.
	if( TYPE_FaxRcvPage_Off == pc_FaxJobSetting->getFaxRcvPage() ) {
		s_OutputSetting.s_ReceivePrint.e_FaxPrint = OAPE_ReceivePrint_Switch_ReceiveEnd;
	} else {
		s_OutputSetting.s_ReceivePrint.e_FaxPrint = OAPE_ReceivePrint_Switch_Receive;
	}

	//		OutputTrayPriority.
	//			Copy,Print,Fax,PrintList.
	if( pc_PrinterSystem->isExistFinisher() ){
		OAPE_PaperOutput e_tray = OAPE_PaperOutput_MainTray;
		getOutputTrayPriority( pc_CopyJobSetting, e_tray, s_OutputSetting.s_OutputTrayPriority );
	}

	//////////////////////////////////// .
	// AutoExpoLevel.
//ZeusMLK CQ369728 東野史明 2012/08/20 Start.
	TYPE_AutoExpoLevel	e_AutoExpoLevel = pc_ScannerServiceSetting->getScannerAdjust(TYPE_ScannerAdjustAEReference);
	getOpenAPIEnum( e_AutoExpoLevel, s_Environment.e_AutoExpoLevel );
//ZeusMLK CQ369728 東野史明 2012/08/20 End.

	//////////////////////////////////// .
	// SmallSizeOriginal.
	getOpenAPIEnum( pc_CopyJobSetting->getSmallSizeOriginal(), s_Environment.e_SmallSizeOriginal );

	//////////////////////////////////// .
	// BlankPagePrint.
	getOpenAPIEnum( pc_CopyJobSetting->getStampPrintAtBlankPage(), s_Environment.e_BlankPagePrint );

	//////////////////////////////////// .
	// PagePrintPosition.
	getOpenAPIEnum( pc_OperationSetting->getPagePrintPosition(), s_Environment.e_PagePrintPosition );

	//////////////////////////////////// .
	// PlatenSeparateScan.
	//[IT6_SPARROW]WP_No.206-1 2018/03/16 Hcl-Neeraj Modified Start.
	//TYPE_MachineType e_MachineType = pc_SystemInfo->getMachineType();
	if(!(OAPI_CheckIsSparrowSFPModel())) //IT6_SparrowBK 制御構成変更_No.36 HCL-Palani 2019/08/06 Modified.
	{
		getOpenAPIEnum( pc_ScanJobSetting->getPlatenSeparateScan(), s_Environment.e_PlatenSeparateScan );
	}
	//[IT6_SPARROW]WP_No.206-1 2018/03/16 Hcl-Neeraj Modified End.
	// MinervaZeus3rd UV制御構成変更_No.1202 Bairavi 2013/05/21 Added Start. 
	getOpenAPIEnum( pc_SystemInfo->getLeftAreaDefaultDisplay(), s_Environment.e_LeftAreaDefaultDisplay );
	// MinervaZeus3rd UV制御構成変更_No.1202 Bairavi 2013/05/21 Added End. 

	// MinervaSBK KRQ-94 05/05/2015 Sahana Added Start.
	// 重送検知機能使用設定.
#ifndef DEF_HeliosMLKCus // 2019/10/14 【IT6_HeliosMLK】Machine No.17-1 HIENG Added.
	s_Environment.e_MultiFeedDetection = (pc_SystemInfo->getMultiFeedDetectSetting() == TYPE_Setting_ON ? OAPE_OnOff_On : OAPE_OnOff_Off);
#endif // 2019/10/14 【IT6_HeliosMLK】Machine No.17-1 HIENG Added.
	// MinervaSBK KRQ-94 05/05/2015 Sahana Added End.
	//////////////////////////////////// .
	// BlankDetectLevel.
	s_Environment.sc_BlankDetectLevel = pc_ScanerSetting->getBlankDetectLevel();

//IT5_3.4 制御構成変更_No.37 2016/12/12 add start.
	// DefaultBoxSetting.
	OAPS_DefaultBoxSetting& s_DefaultBoxSetting = s_Environment.s_DefaultBoxSetting;

	StackGuard_Job pc_Job(TYPE_JobTypeFaxSendMemory);
	if(!pc_Job)
	{
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance(Job)");
		return false;
	}
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if(!pc_JobSpec
	|| !pc_JobSpec->pc_Common
	|| !pc_JobSpec->pc_Scan
	|| !pc_JobSpec->pc_Send)
	{
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance(JobSpec)");
		return false;
	}
	TYPC_Common* pc_Common = pc_JobSpec->pc_Common;
	TYPC_Scan*   pc_Scan   = pc_JobSpec->pc_Scan;
	TYPC_Send*   pc_Send   = pc_JobSpec->pc_Send;

	getOpenAPIEnum(pc_OperationSetting->getInitModeScanToMediaOn(), s_DefaultBoxSetting.e_DefaultSetting);
	
	// IT6_Sparrow 1056790 2018/09/18 HCL-Naveen Added Start.
	if( True == pc_OperationSetting->getInitModeScanToMediaOn()) {
		pc_OperationSetting->getInitModeScanToMedia(pc_JobSpec);
	}
	// IT6_Sparrow 1056790 2018/09/18 HCL-Naveen Added End.

	// Color.
	if(True == pc_Scan->getAcsOn())
	{
		s_DefaultBoxSetting.e_Color = OAPE_Acs_Auto;				// オートカラー.
	} else {
		if(True == pc_Common->getBinary())
		{
			s_DefaultBoxSetting.e_Color = OAPE_Acs_Monochrome;		// 白黒2値.
		} else {
			if(TYPE_ColorModeBlack == pc_Common->getColorMode())
			{
				s_DefaultBoxSetting.e_Color = OAPE_Acs_Grayscale;	// グレースケール.
			} else {
				s_DefaultBoxSetting.e_Color = OAPE_Acs_FullColor;	// FullColor.
			}
		}
	}

	// FileFormat.
	getOpenAPIEnum(pc_Send->c_ComSend.getFileFormat(), s_DefaultBoxSetting.e_FileFormat);

	// Resolution.
	getOpenAPIEnum(pc_Common->c_Resolution.getFaxResolution(), s_DefaultBoxSetting.e_Resolution);

	// FaxResolution.
	getOpenAPIEnum(pc_Common->c_FaxResolution.getFaxResolution(), s_DefaultBoxSetting.e_FaxResolution);
//IT5_3.4 制御構成変更_No.37 2016/12/12 add end.
	//IT5_4.2 RQ-686 AR1034233 Hcl-Chitra 2018/01/20 Added Starts.
	if(p_req->e_NetworkType.get() == OAPE_NetworkType_NW2)
	{
		s_OutputSetting.s_ReceivePrint.e_FaxPrint.clean();
	}
	//IT5_4.2 RQ-686 AR1034233 Hcl-Chitra 2018/01/20 Added Ends.

	// CommonAPIのOAPサポート化対応 XML基盤技術開発v2 2017/03/23 Pactera Added Start.
	for( short ss_Index = 0; ss_Index < (OAPD_PRIORITYTRAYLIST_PRIORITYTRAY_MAXLIST-1); ss_Index++ ) {
		TYPE_FeedTray e_Tmp = TYPE_FeedTray1;			// (初期化).
		
		// CommonAPIの値を取得.
		e_Tmp = s_TraySearchPriority.e_FeedTray[ss_Index];
		
		if( TYPE_FeedTrayUnselect != e_Tmp ) {
			if( TYPE_TrayStatusNon != pc_FeedTray->getTrayStatus( e_Tmp ) )
			{
				getOpenAPIEnum( pc_FeedTray->getPaperStatus( e_Tmp ), s_Environment.e_PaperStatus );
			}
		}
	}
	// CommonAPIのOAPサポート化対応 XML基盤技術開発v2 2017/03/23 Pactera Added End.

	
//【Eagle】RQ-720紙種自動検知 2017/11/07 HCL-Garima Start.
#ifdef DEF_EagleCus	
	OAPS_AutoPaperTypeDetectSetting& s_AutoPaperTypeDetectSetting = s_Environment.s_AutoPaperTypeDetectSetting;
	Bool b_Auto        = pc_SystemSetting->getAutoDetectedPaperType(TYPE_AutoDetectedPaperTypeMode);
	Bool b_Tray_1      = pc_SystemSetting->getAutoDetectedPaperType(TYPE_AutoDetectedPaperTypeTray1Mode);
	Bool b_Tray_2      = pc_SystemSetting->getAutoDetectedPaperType(TYPE_AutoDetectedPaperTypeTray2Mode);
	Bool b_Tray_3      = pc_SystemSetting->getAutoDetectedPaperType(TYPE_AutoDetectedPaperTypeTray3Mode);
	Bool b_Tray_4      = pc_SystemSetting->getAutoDetectedPaperType(TYPE_AutoDetectedPaperTypeTray4Mode);
	Bool b_Tray_Lct    = pc_SystemSetting->getAutoDetectedPaperType(TYPE_AutoDetectedPaperTypeLCTMode);
	Bool b_Tray_Manual = pc_SystemSetting->getAutoDetectedPaperType(TYPE_AutoDetectedPaperTypeManualFeedMode);
	Bool b_ModeStatus    = pc_SystemSetting->getAutoDetectedPaperType(TYPE_AutoDetectedPaperTypeModeStatus);

		TYPE_KeyMode e_KeyMode = pc_MachineSetting->getKeyMode();
		if(OAPD_VERSION_CHECK(p_req->getVersion(), OAPD_OVER_V74) == true){  //IT6_EagleH UV VersionUp 25/02/2019 HCL-Mangai Modified.
		 	if(( True == pc_SystemSetting->getAutoDetectedPaperType(TYPE_AutoDetectedPaperTypeModeStatus))			
		//【EagleH】RQ-720対応 OP_BTS-1739 2018/12/25 HCL-Garima Start.
				&& (e_KeyMode  != TYPE_KeyModeKeyCounter 
					&& e_KeyMode != TYPE_KeyModeVendorKM
					&& e_KeyMode != TYPE_KeyModeAdminMachineKM))
			//【EagleH】RQ-720対応 OP_BTS-1739 2018/12/25 HCL-Garima End.
			{
				//Enable.
				getOpenAPIEnum( b_Auto, 		 s_AutoPaperTypeDetectSetting.e_Auto );
				//Tray1.
				getOpenAPIEnum( b_Tray_1, 		 s_AutoPaperTypeDetectSetting.e_Tray1 );
				//Tray2.
				getOpenAPIEnum( b_Tray_2, 		 s_AutoPaperTypeDetectSetting.e_Tray2 );
				//Tray3.
				getOpenAPIEnum( b_Tray_3, 		 s_AutoPaperTypeDetectSetting.e_Tray3 );
				//Tray4.
				getOpenAPIEnum( b_Tray_4, 		 s_AutoPaperTypeDetectSetting.e_Tray4 );
				//Tray5(LCT).
				getOpenAPIEnum( b_Tray_Lct, 	 s_AutoPaperTypeDetectSetting.e_LCT ); // AR1039470 HCL-Kailash Mod 2018/03/22.
				//Hand(Manual).
				getOpenAPIEnum( b_Tray_Manual, 	 s_AutoPaperTypeDetectSetting.e_ManualFeedTray ); // AR1039470 HCL-Kailash Mod 2018/03/22.
				//Mode Status.
				getOpenAPIEnum( b_ModeStatus, 	 s_AutoPaperTypeDetectSetting.e_ModeStatus );
			}
		}else{
			//Enable.
				getOpenAPIEnum( b_Auto, 		 s_AutoPaperTypeDetectSetting.e_Auto );
				//Tray1.
				getOpenAPIEnum( b_Tray_1, 		 s_AutoPaperTypeDetectSetting.e_Tray1 );
				//Tray2.
				getOpenAPIEnum( b_Tray_2, 		 s_AutoPaperTypeDetectSetting.e_Tray2 );
				//Tray3.
				getOpenAPIEnum( b_Tray_3, 		 s_AutoPaperTypeDetectSetting.e_Tray3 );
				//Tray4.
				getOpenAPIEnum( b_Tray_4, 		 s_AutoPaperTypeDetectSetting.e_Tray4 );
				//Tray5(LCT).
				getOpenAPIEnum( b_Tray_Lct, 	 s_AutoPaperTypeDetectSetting.e_LCT ); // AR1039470 HCL-Kailash Mod 2018/03/22.
				//Hand(Manual).
				getOpenAPIEnum( b_Tray_Manual, 	 s_AutoPaperTypeDetectSetting.e_ManualFeedTray ); // AR1039470 HCL-Kailash Mod 2018/03/22.
				//Mode Status.
				getOpenAPIEnum( b_ModeStatus, 	 s_AutoPaperTypeDetectSetting.e_ModeStatus );
			}
#endif	
//【Eagle】RQ-720紙種自動検知 2017/11/07 HCL-Garima End.
//【EagleH】RQ-720対応 2018/12/04 HCL-Garima Start.
#ifdef DEF_EagleCus	
	if( OAPD_VERSION_CHECK(p_req->getVersion(), OAPD_OVER_V74) == true) {  //IT6_EagleH UV VersionUp 25/02/2019 HCL-Mangai Modified.

		if( True == pc_SystemSetting->getAutoDetectedPaperType(TYPE_AutoDetectedPaperTypeModeStatus))
		{		
			switch(pc_PrinterServiceSetting->getPrinterAdjust( TYPE_PrinterAdjustEnvelopeDetect ))
			{
				case 0:
					s_Environment.e_EnvelopeDetectSetting = OAPE_EnvelopeDetectSetting_Detect;
					break;
				case 1:
					s_Environment.e_EnvelopeDetectSetting = OAPE_EnvelopeDetectSetting_DonotDetect;
					break;
				default:
					break;
			}
		}
	}
#endif
//【EagleH】RQ-720対応 2018/12/04 HCL-Garima End.
	// FPT-VanPN2【IT6_2.3】制御構成変更_No.52 2022/11/10 ADD Start.
	if( OAPD_VERSION_CHECK(p_req->getVersion(), OAPD_OVER_V734) == true) 
	{
		OAPS_WizardSetting& s_WizardSetting = s_Environment.s_WizardSetting;
		TYPE_CountryType e_CountryType = pc_SystemSetting->getCountryType();
		getOpenAPIEnum(e_CountryType, s_WizardSetting.e_Country);

		TYPE_CountryCode e_NetworkCountrySupport = pc_MachineSetting->getWirelessLANDestinationSetting();
		getOpenAPIEnum(e_NetworkCountrySupport, s_WizardSetting.e_NetworkCountrySupport);

		TYPE_CountryCode e_FaxCountrySupport = pc_MachineSetting->getCountryCode();
		getOpenAPIEnum(e_FaxCountrySupport, s_WizardSetting.e_FaxCountrySupport);
		// FPT-VanPN2【IT6_2.3】制御構成変更_No.52 2022/12/05 ADD Start.
		TYPE_TimezoneType e_TimezoneType = pc_SystemSetting->getTimezoneType();
		s_WizardSetting.e_TimeZoneCity = OAPE_TimeZoneCity_Unidentified;
		getOpenAPIEnum(e_TimezoneType, s_WizardSetting.e_TimeZoneCity);
		// FPT-VanPN2【IT6_2.3】制御構成変更_No.52 2022/12/05 ADD End.
	}
	// FPT-VanPN2【IT6_2.3】制御構成変更_No.52 2022/11/10 ADD End.
	return true;
}

// MinervaZeus3rd UV-01309 Muratani 2013/03/18 Start.
// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=Copy).
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Copy>::OAPC_UtilityItemImp()
{
	activate();
}
template<> OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Copy>::~OAPC_UtilityItemImp()
{}

// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=Copy) pre_check.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Copy>::pre_check(const OAPS_AppReqSetUtilitySetting* p_req, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=Copy)::pre_check()" );

	e.setMessage(OAPE_Result_FaultRequest_AppReqSetUtilitySetting);
	
	// コピー設定.
	if( p_req->s_Copy.empty() ){
		e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralTagMissing, "Copy" );
		return false;
	}
		
	// コピー初期設定.
	if( !p_req->s_Copy.s_DefaultCopySettingList.empty() ) {
		// ArraySizeチェック.
		if(p_req->s_Copy.s_DefaultCopySettingList.ss_ArraySize.empty()) {
			e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralTagMissing, "DefaultCopySettingList.ArraySize" );
			return false;
		}
		
		if( !p_req->s_Copy.s_DefaultCopySettingList.ss_ArraySize.range() ){
			e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralRangeIllegal, "DefaultCopySettingList.ArraySize" );
			return false;
		}
		
		// DefaultCopySetting.
		for( int i = 0; i < p_req->s_Copy.s_DefaultCopySettingList.ss_ArraySize.get(); i++ ){
			const OAPS_DefaultCopySetting& s_DefaultCopySetting = p_req->s_Copy.s_DefaultCopySettingList.s_DefaultCopySetting[i];
			
			// DisplayType.
			if( s_DefaultCopySetting.e_DisplayType.empty() ){
				e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralTagMissing, "DefaultCopySetting.DisplayType" );
				return false;
			}
#ifndef DEF_MonoCus
			// Color.
			if( !s_DefaultCopySetting.s_Color.empty() ){
				// Mode.
				if( !s_DefaultCopySetting.s_Color.e_Auto.empty() && OAPE_Bool_false == s_DefaultCopySetting.s_Color.e_Auto.get() ){
					// Mode.
					if( s_DefaultCopySetting.s_Color.e_Mode.empty() ){
						e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralTagMissing, "DefaultCopySetting.Color.Mode" );
						return false;
					}
				}
			}
#endif // DEF_MonoCus.
			// MinervaZeusInt CQ379190 h-kinoshita 2013/06/17 Start.
			// MagMemoryList.
			if( !s_DefaultCopySetting.s_MagMemoryList.empty() ){
				// ArraySizeチェック.
				if( !s_DefaultCopySetting.s_MagMemoryList.ss_ArraySize.range() ){
					e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralRangeIllegal, "DefaultCopySetting.MagMemoryList.ArraySize" );
				}
			}
			// MinervaZeusInt CQ379190 h-kinoshita 2013/06/17 End.
		}
	}

	return true;
}
// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem = Copy) set.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Copy>::set(const OAPS_AppReqSetUtilitySetting* p_req, OAPC_SyncLock& _slock, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=Copy)::set()" );

	e.setMessage( OAPE_Result_FaultRequest_AppReqSetUtilitySetting );
	
	//////////////////////////////////// .
	// I/F Lock .
	OAPC_ApiLock<APIC_OperationSetting> pc_OperationSetting(APIC_OperationSetting::newInstanceAdmin() , &_slock);

	if( !pc_OperationSetting )
	{
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance");
		return false;
	}

	// ※便宜上.
	const OAPS_UtilitySetting_Copy& s_Copy = p_req->s_Copy;

	// DefaultCopySetting.
	if( !s_Copy.s_DefaultCopySettingList.empty() ) {
		for( int i = 0; i < s_Copy.s_DefaultCopySettingList.ss_ArraySize.get(); i++ ) {
			const OAPS_DefaultCopySetting& s_DefaultCopySetting = s_Copy.s_DefaultCopySettingList.s_DefaultCopySetting[i];
			
			StackGuard_Job pc_Job( TYPE_JobTypeCopy );
			if( !pc_Job )
			{
				e.setKind(OAPE_Fault_faultcode_Server);
				e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance(Job)");
				return false;
			}
			
			TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
			if( (!pc_JobSpec)				||
				(!pc_JobSpec->pc_Common)		||
				(!pc_JobSpec->pc_Scan)		||
				(!pc_JobSpec->pc_Print)		)
			{
				e.setKind(OAPE_Fault_faultcode_Server);
				e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance(JobSpec)");
				return false;
			}

			// JobSpec直下.
			OAPC_ApiWrapper<TYPC_Common> pc_Common( pc_JobSpec->pc_Common ); 
			OAPC_ApiWrapper<TYPC_Scan>	 pc_Scan( pc_JobSpec->pc_Scan ); 
			OAPC_ApiWrapper<TYPC_Print>	 pc_Print( pc_JobSpec->pc_Print ); 

			// TYPC_Common.
			OAPC_ApiWrapper<TYPC_2ColorMode>		pc_2ColorMode( &pc_Common->c_2ColorMode ); 
			OAPC_ApiWrapper<TYPC_MonoColor>			pc_MonoColor( &pc_Common->c_MonoColor ); 
			OAPC_ApiWrapper<TYPC_Mag>				pc_Mag( &pc_Common->c_Mag ); 
			// TYPC_Scan.
			OAPC_ApiWrapper<TYPC_BindDirection>	 	pc_BindDirection( &pc_Scan->c_BindDirection ); 
			
			// TYPC_Print.
			OAPC_ApiWrapper<TYPC_BindDirection>	 	pc_OpenDirection( &pc_Print->c_BindDirection ); 
			OAPC_ApiWrapper<TYPC_Nin1>	 			pc_Nin1( &pc_Print->c_Nin1 ); 
			
#ifndef DEF_MonoCus
			//////////////.
			// Color.
			//////////////.
			// ※便宜上.
			const OAPS_ColorInfo& s_Color = s_DefaultCopySetting.s_Color;
			if( !s_Color.empty() ) {
				// Autoがなければ設定を行わない.
				if( !s_Color.e_Auto.empty() ){
					Bool b_2ColorMode = False;
					Bool b_MonoColorOn = False;

					// 2色カラー、単色カラーのOn/Off設定.
					if( OAPE_Bool_false == s_Color.e_Auto.get() ) {
						if( OAPE_ColorInfoMode_BiColor == s_Color.e_Mode ) {
							// 2色カラーOn.
							b_2ColorMode = True;
						} else if( OAPE_ColorInfoMode_MonoColor == s_Color.e_Mode ) {
							// 単色カラーOn.
							b_MonoColorOn = True;				
						} else {
							// 何もしない.
						}
					}

					pc_Scan.set_enum_skip_check( s_Color.e_Auto, &TYPC_Scan::setAcsOn );
					pc_Common.set_enum_skip_check( s_Color.e_Mode, &TYPC_Common::setColorMode );
					pc_MonoColor->setMonoColorOn(b_MonoColorOn);
					pc_MonoColor.set_enum_skip_check( s_Color.s_MonoColor.e_Color, &TYPC_MonoColor::setMonoColor );
					pc_MonoColor.set_enum_skip_check( s_Color.s_MonoColor.e_Mode, &TYPC_MonoColor::setMonoColorMode );
					pc_2ColorMode->set2ColorOn(b_2ColorMode);
					pc_2ColorMode.set_enum_skip_check( s_Color.e_BiColor, &TYPC_2ColorMode::setMonoColor );
				}
			}
#endif // DEF_MonoCus.

			//////////////.
			// 片面/両面.
			//////////////.
			// ※便宜上.
			const OAPS_DefaultCopySetting_DuplexMode& s_DuplexMode = s_DefaultCopySetting.s_DuplexMode;

			pc_Scan.set_enum_skip_check( s_DuplexMode.e_Scan, &TYPC_Scan::setDuplex );

			pc_Print.set_enum_skip_check( s_DuplexMode.e_Print, &TYPC_Print::setDuplexMode );
			
			//////////////.
			// ページ集約.
			//////////////.
			// ※便宜上.
			const OAPS_Nup& s_Nup = s_DefaultCopySetting.s_Nup;
			
			pc_Nin1.set_enum_skip_check( s_Nup.e_Mode, &TYPC_Nin1::setNin1 );
			pc_Nin1.set_enum_skip_check( s_Nup.e_NupModeOrder, &TYPC_Nin1::setVerticalOrHorizon );
			
			//////////////.
			// 倍率.
			//////////////.
			// ※便宜上.
			const OAPS_Mag& s_Mag = s_DefaultCopySetting.s_Mag;
			
			// MinervaZeusInt CQ379190 h-kinoshita 2013/06/17 Start.
			if( !s_Mag.e_Auto.empty() ){
				// 自動倍率設定が省略されていない場合のみ倍率を設定する.
				
				pc_Mag.set_enum_skip_check( s_Mag.e_Auto, &TYPC_Mag::setAmsOn );
				
				pc_Mag.set_enum_skip_check( s_Mag.e_Type, &TYPC_Mag::setMagType );

				pc_Mag.set_enum_skip_check( s_Mag.e_Minimal, &TYPC_Mag::setMinimal );
				
				TYPC_Point<float> c_Point;
				bool b_set = false;
				if( !s_Mag.f_X.empty() ){
					c_Point.x = s_Mag.f_X.get();
					b_set = true;
				}
				if( !s_Mag.f_Y.empty() ){
					c_Point.y = s_Mag.f_Y.get();
					b_set = true;
				}
				if( b_set ){
					pc_Mag->setMag( c_Point );
				}
			}
			
			//////////////.
			// 登録倍率.
			//////////////.
			// ※便宜上.
			const OAPS_MagMemoryList& s_MagMemoryList = s_DefaultCopySetting.s_MagMemoryList;
			
			if( !s_MagMemoryList.empty() ){
				if( !s_MagMemoryList.ss_ArraySize.empty() ){
				
					for( int i = 0; i < s_MagMemoryList.ss_ArraySize.get(); i++ ){
						TYPC_Point<float> c_MagMemoryPoint;

						if( s_MagMemoryList.s_MagMemory[i].f_X.empty() ){
							// 登録倍率値はX/Y同値の為、X値を設定する.
							continue;
						}

						c_MagMemoryPoint.x = s_MagMemoryList.s_MagMemory[i].f_X;
						c_MagMemoryPoint.y = s_MagMemoryList.s_MagMemory[i].f_X;

						// 変更した登録倍率設定を設定.
						( pc_OperationSetting->*(tblSetMagMemoryFunc[i]) )( c_MagMemoryPoint );
					}
				}
			}
			// MinervaZeusInt CQ379190 h-kinoshita 2013/06/17 End.
			
			if( OAPE_PanelDisplayType_Normal == s_DefaultCopySetting.e_DisplayType ) {
				pc_OperationSetting.set_enum_skip_check( s_DefaultCopySetting.e_DefaultSetting, &APIC_OperationSetting::setInitModeCopyOn );
				pc_OperationSetting->setInitModeCopy( pc_JobSpec );
				
			} else {
				pc_OperationSetting.set_enum_skip_check( s_DefaultCopySetting.e_DefaultSetting, &APIC_OperationSetting::setInitModeCopyOnForLargeDisp );
				pc_OperationSetting->setInitModeCopyForLargeDisp( pc_JobSpec );
			}
		}
	}
	
	return true;
}

// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=Copy) get.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Copy>::get(const OAPS_AppReqGetUtilitySetting* p_req, OAPS_AppResGetUtilitySetting* p_res, OAPC_SyncLock& _slock, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=Copy)::get()" );

	e.setMessage( OAPE_Result_FaultRequest_AppReqGetUtilitySetting );
	
	
	//////////////////////////////////// .
	// I/F Lock .
	OAPC_ApiLock<APIC_OperationSetting> pc_OperationSetting(APIC_OperationSetting::newInstanceAdmin() , &_slock);

	if( !pc_OperationSetting )
	{
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance");
		return false;
	}

	// ※便宜上.
	OAPS_UtilitySetting_Copy& s_Copy = p_res->s_Copy;
	
	// DefaultCopySetting.
	s_Copy.s_DefaultCopySettingList.ss_ArraySize = OAPD_DEFAUTSETTINGLIST_MAXLIST;
	for( int i = 0; i < s_Copy.s_DefaultCopySettingList.ss_ArraySize.get(); i++ ) {
		// ※便宜上.
		OAPS_DefaultCopySetting& s_DefaultCopySetting = s_Copy.s_DefaultCopySettingList.s_DefaultCopySetting[i];
			
		StackGuard_Job pc_Job( TYPE_JobTypeCopy );
		if( !pc_Job )
		{
			e.setKind(OAPE_Fault_faultcode_Server);
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance(Job)");
			return false;
		}
		
		TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
		if( (!pc_JobSpec)				||
			(!pc_JobSpec->pc_Common)	||
			(!pc_JobSpec->pc_Scan)		||
			(!pc_JobSpec->pc_Print)		)
		{
			e.setKind(OAPE_Fault_faultcode_Server);
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance(JobSpec)");
			return false;
		}
		TYPC_Common* pc_Common = pc_JobSpec->pc_Common;
		TYPC_Scan*   pc_Scan   = pc_JobSpec->pc_Scan;
		TYPC_Print*  pc_Print  = pc_JobSpec->pc_Print;
		
		if( OAPE_PanelDisplayType_Normal == i){
			s_DefaultCopySetting.e_DisplayType = OAPE_PanelDisplayType_Normal;
			getOpenAPIEnum( pc_OperationSetting->getInitModeCopyOn(), s_DefaultCopySetting.e_DefaultSetting );
			// IT6_Eagle 1068382 2018/12/12 HCL-Naveen Added Start.
			if( True == pc_OperationSetting->getInitModeCopyOn() ) {
				pc_OperationSetting->getInitModeCopy( pc_JobSpec );
			}
			// IT6_Eagle 1068382 2018/12/12 HCL-Naveen Added End.
		} else {
			s_DefaultCopySetting.e_DisplayType = OAPE_PanelDisplayType_Enlarge;
			getOpenAPIEnum( pc_OperationSetting->getInitModeCopyOnForLargeDisp(), s_DefaultCopySetting.e_DefaultSetting );
			
			// IT6_Eagle 1068382 2018/12/12 HCL-Naveen Added Start.
			if( True == pc_OperationSetting->getInitModeCopyOnForLargeDisp() ) {
				pc_OperationSetting->getInitModeCopyForLargeDisp( pc_JobSpec );
			}
			// IT6_Eagle 1068382 2018/12/12 HCL-Naveen Added End.
		}

#ifndef DEF_MonoCus
		//////////////.
		// Color.
		//////////////.
		//////////////.
		// ※便宜上.
		OAPS_ColorInfo& s_Color = s_DefaultCopySetting.s_Color;
		
		getOpenAPIEnum( pc_Scan->getAcsOn(), s_Color.e_Auto);
		
		getOpenAPIEnum( pc_Common->getColorMode(), s_Color.e_Mode);
		
		if( True == pc_Common->c_2ColorMode.get2ColorOn() ){
			s_Color.e_Mode = OAPE_ColorInfoMode_BiColor;
			
		} else if( True == pc_Common->c_MonoColor.getMonoColorOn() ){
			s_Color.e_Mode = OAPE_ColorInfoMode_MonoColor;
		}
		getOpenAPIEnum( pc_Common->c_MonoColor.getMonoColor(),  s_Color.s_MonoColor.e_Color);
		getOpenAPIEnum( pc_Common->c_MonoColor.getMonoColorMode(),  s_Color.s_MonoColor.e_Mode);
		
		getOpenAPIEnum( pc_Common->c_2ColorMode.getMonoColor(), s_Color.e_BiColor);
#endif // DEF_MonoCus.
	
		//////////////.
		// 片面/両面.
		//////////////.
		// ※便宜上.
		OAPS_DefaultCopySetting_DuplexMode& s_DuplexMode = s_DefaultCopySetting.s_DuplexMode;
		
		getOpenAPIEnum( pc_Scan->getDuplex(), s_DuplexMode.e_Scan );
	
		getOpenAPIEnum( pc_Print->getDuplexMode(), s_DuplexMode.e_Print );
		getOpenAPIEnum( pc_Print->c_BindDirection.getOpenDirection(), s_DuplexMode.e_OpenDirection );	// [WebUIv2]コピー＞両面／ページ集約＞開き方向 2017.12.8 add.
		
		//////////////.
		// ページ集約.
		//////////////.
		// ※便宜上.
		OAPS_Nup& s_Nup = s_DefaultCopySetting.s_Nup;
		
		getOpenAPIEnum( pc_Print->c_Nin1.getNin1(), s_Nup.e_Mode);	
		getOpenAPIEnum( pc_Print->c_Nin1.getVerticalOrHorizon(), s_Nup.e_NupModeOrder);	

		//////////////.
		// 原稿セット方向.
		//////////////.
		getOpenAPIEnum( pc_Common->c_Orientation.getSetDirection(),  s_DefaultCopySetting.e_SetDirection);	// [WebUIv2]コピー＞両面／ページ集約＞原稿セット方向 2017.12.8 add.

		//////////////.
		// 倍率.
		//////////////.
		// ※便宜上.
		OAPS_Mag& s_Mag = s_DefaultCopySetting.s_Mag;

		// MinervaZeusInt CQ379190 h-kinoshita 2013/06/17 Start.
		getOpenAPIEnum( pc_Common->c_Mag.getAmsOn(), s_Mag.e_Auto );
		getOpenAPIEnum( pc_Common->c_Mag.getMagType(), s_Mag.e_Type );
		getOpenAPIEnum( pc_Common->c_Mag.getMinimal(), s_Mag.e_Minimal );
		// MinervaZeusInt CQ379190 h-kinoshita 2013/06/17 End.

		TYPC_Point<float> c_Point = pc_Common->c_Mag.getMag();
		s_Mag.f_X = c_Point.x;
		s_Mag.f_Y = c_Point.y;

		// MinervaZeusInt CQ379190 h-kinoshita 2013/06/17 Start.
		//////////////.
		// 登録倍率.
		//////////////.
		// ※便宜上.
		OAPS_MagMemoryList&	s_MagMemoryList = s_DefaultCopySetting.s_MagMemoryList;

		s_MagMemoryList.ss_ArraySize = OAPD_MAGMEMORYLIST_MAXLIST;
		for( int i = 0; i < OAPD_MAGMEMORYLIST_MAXLIST; i++ ){
			// 登録倍率設定を取得.
			const TYPC_Point<float> c_MagMemoryPoint( ( pc_OperationSetting->*(tblGetMagMemoryFunc[i]) )() );

			// 登録倍率のX/Yは同値の為、f_YへXの倍率値を設定する.
			s_MagMemoryList.s_MagMemory[i].f_X = c_MagMemoryPoint.x;
			s_MagMemoryList.s_MagMemory[i].f_Y = c_MagMemoryPoint.y;
		}
		// MinervaZeusInt CQ379190 h-kinoshita 2013/06/17 End.

		// [WebUIv2]コピー＞濃度 2017.12.4 start.
		////////////////
		// 濃度.
		////////////////
		// ※便宜上.
		int32_t value = 0;
		switch(pc_Common->c_ImageAdjust.getDensity().getImageAdjust())
		{
			case TYPE_ImageAdjustP09:	value = -9;	break;
			case TYPE_ImageAdjustP08:	value = -8;	break;
			case TYPE_ImageAdjustP07:	value = -7;	break;
			case TYPE_ImageAdjustP06:	value = -6;	break;
			case TYPE_ImageAdjustP05:	value = -5;	break;
			case TYPE_ImageAdjustP04:	value = -4;	break;
			case TYPE_ImageAdjustP03:	value = -3;	break;
			case TYPE_ImageAdjustP02:	value = -2;	break;
			case TYPE_ImageAdjustP01:	value = -1;	break;
			case TYPE_ImageAdjustStd:	value = 0;	break;
			case TYPE_ImageAdjustM01:	value = 1;	break;
			case TYPE_ImageAdjustM02:	value = 2;	break;
			case TYPE_ImageAdjustM03:	value = 3;	break;
			case TYPE_ImageAdjustM04:	value = 4;	break;
			case TYPE_ImageAdjustM05:	value = 5;	break;
			case TYPE_ImageAdjustM06:	value = 6;	break;
			case TYPE_ImageAdjustM07:	value = 7;	break;
			case TYPE_ImageAdjustM08:	value = 8;	break;
			case TYPE_ImageAdjustM09:	value = 9;	break;
			default:								break;
		}
		s_DefaultCopySetting.sc_Density = value;
		// [WebUIv2]コピー＞濃度 2017.12.4 end.
		
		getOpenAPIEnum(pc_Common->c_OriginalImageMode.getOriginalImageMode(), s_DefaultCopySetting.e_OriginalImage);	// [WebUIv2]コピー＞原稿画質 2017.12.6 add.
		
		// [WebUIv2]コピー＞給紙指定 2017.12.11 start.
		// ※便宜上.
		OAPS_PaperSource& s_PaperSource = s_DefaultCopySetting.s_PaperSource;
		
		getOpenAPIEnum(pc_Print->c_PaperTray.getApsOn(), s_PaperSource.e_AutoPaperSelect);
		getOpenAPIEnum(pc_Print->c_PaperTray.getFeedTray(), s_PaperSource.e_InputTray);
		// [WebUIv2]コピー＞給紙指定 2017.12.11 End.
		
		getOpenAPIEnum(pc_Scan->c_MemoryIn.getMemoryInOn(), s_DefaultCopySetting.e_SeparateScan);	// [WebUIv2]コピー＞連続読込み 2017.12.8 add.
		
		// [WebUIv2]コピー＞原稿設定 2017.12.12 start.
		// 混載原稿.
		if( TYPE_OriginalSizeModeUnMix == pc_Scan->c_OriginalSize.getOriginalSizeMode() ){		// 混載しない.
			s_DefaultCopySetting.s_OriginalSettings.s_OriginalSetting.e_MixedOriginal = OAPE_MixedOriginal_Off;
		} else if( TYPE_OriginalSizeModeMix == pc_Scan->c_OriginalSize.getOriginalSizeMode() ){	// 混載する.
			if(True == pc_Scan->c_OriginalSize.getMixedWidthEnable())	// 異幅混載.
			{
				s_DefaultCopySetting.s_OriginalSettings.s_OriginalSetting.e_MixedOriginal = OAPE_MixedOriginal_DifferentWidth;
			} else {													// 同幅混載.
				s_DefaultCopySetting.s_OriginalSettings.s_OriginalSetting.e_MixedOriginal = OAPE_MixedOriginal_SameWidth;
			}
		} else {																				// 原稿指定が未定義.
			s_DefaultCopySetting.s_OriginalSettings.s_OriginalSetting.e_MixedOriginal = OAPE_MixedOriginal_Unidentified;
		}
		
		// 原稿のとじしろ.
		getOpenAPIEnum( pc_Scan->c_BindDirection.getBindDirection(), s_DefaultCopySetting.s_OriginalSettings.e_BindingPosition );
			
		// 原稿サイズ.
		if( True == pc_Scan->c_OriginalSize.getAutoSize() ){
			s_DefaultCopySetting.s_OriginalSettings.s_OriginalSize.e_Auto = OAPE_Enable_On;
			s_DefaultCopySetting.s_OriginalSettings.s_OriginalSize.e_SizeCode = OAPE_SizeCode_EdgeFeed_Unidentified;
		} else {
			s_DefaultCopySetting.s_OriginalSettings.s_OriginalSize.e_Auto = OAPE_Enable_Off;
			getOpenAPIEnum(  pc_Scan->c_OriginalSize.getPaperSize().e_SizeCode, s_DefaultCopySetting.s_OriginalSettings.s_OriginalSize.e_SizeCode);
		}
		
		// [WebUIv2]コピー＞原稿設定 2017.12.12 end.

		getOpenAPIEnum(pc_Print->c_ImageRotate.getImageRotateEnable(), s_DefaultCopySetting.e_AutoRotate);	// [WebUIv2]コピー＞自動画面回転 2017.12.14 add.

		////////////////
		// 部数.
		////////////////
		s_DefaultCopySetting.sl_NoOfSets = pc_Print->getNumOfCopies();	// [WebUIv2]コピー＞部数 2017.12.20 add.
		
		// [WebUIv2]コピー＞仕上げ 2017.12.19 start.
		// グループ/ソート.
		if( True == pc_Print->c_SortMode.getEnableSortChange() ){
			s_DefaultCopySetting.s_Finishing.e_GroupSort = OAPE_SortType_Auto;
		} else {
			getOpenAPIEnum( pc_Print->c_SortMode.getSortMode(), s_DefaultCopySetting.s_Finishing.e_GroupSort );
		}
		
		// ステープル種別.
		getOpenAPIEnum( pc_Print->c_Finishing.getStapleMode(), s_DefaultCopySetting.s_Finishing.e_StapleMode );
		
		// パンチ種別.
		getOpenAPIEnum( pc_Print->c_Finishing.getPunchMode(), s_DefaultCopySetting.s_Finishing.e_PunchMode );
		
		// ステープル位置.
		if( True == pc_Print->c_Finishing.getStaplePositionAuto() ){
			s_DefaultCopySetting.s_Finishing.e_StaplePosition = OAPE_OriginalBasicMode_BindDirection_Auto;
		} else {
			getOpenAPIEnum( pc_Print->c_Finishing.getStaplePosition(), s_DefaultCopySetting.s_Finishing.e_StaplePosition );
		}
		
		// パンチ位置.
		if( True == pc_Print->c_Finishing.getPunchPositionAuto() ){
			s_DefaultCopySetting.s_Finishing.e_PunchPosition = OAPE_OriginalBasicMode_BindDirection_Auto;
		} else {
			getOpenAPIEnum( pc_Print->c_Finishing.getPunchPosition(), s_DefaultCopySetting.s_Finishing.e_PunchPosition );
		}
		
		// ステープル角度.
		getOpenAPIEnum( pc_Print->c_Finishing.getStapleAngle(), s_DefaultCopySetting.s_Finishing.e_StapleAngle );	// [WebUIv2]コピー＞仕上り＞ステープル位置指定＞角度指定 2018.1.9 add.
		// [WebUIv2]コピー＞仕上げ 2017.12.19 end.
	}
	
	return true;
}


// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=FaxScan).
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_FaxScan>::OAPC_UtilityItemImp()
{
	activate();
}
template<> OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_FaxScan>::~OAPC_UtilityItemImp()
{}

// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=FaxScan) pre_check.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_FaxScan>::pre_check(const OAPS_AppReqSetUtilitySetting* p_req, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=FaxScan)::pre_check()" );
	//[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified Start.
	OAPC_ApiWrapper<APIC_SystemInfo> pc_SystemInfo(APIC_SystemInfo::newInstanceAdmin() );
	if( !pc_SystemInfo ) {
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Cannot Get Instance" );
		return false;
	}
	
	
	//TYPE_MachineType e_MachineType = pc_SystemInfo->getMachineType();
	if(OAPI_CheckIsSparrowSFPModel())//IT6_SparrowBK 制御構成変更_No.36 HCL-Palani 2019/08/06 Modified.
	{
		if( False == checkExpImpAccountCheck( p_req->s_OperatorInfo.sc_AuthKey.get() ) ){
			e.setKind(OAPE_Fault_faultcode_Server);
			e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralIllegalValue , "RequestItem:FaxScan" );
			return false;
		}
		else
		{
			return true;
		}
	}
	//[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified End.
	
	e.setMessage(OAPE_Result_FaultRequest_AppReqSetUtilitySetting);

	// ファクススキャン設定.
	if( p_req->s_FaxScan.empty() ){
		e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralTagMissing, "FaxScan" );
		return false;
	}
	
	// ファクススキャン初期設定.
	if( !p_req->s_FaxScan.s_DefaultFaxScanSettingList.empty() ) {
		// ArraySizeチェック.
		if(p_req->s_FaxScan.s_DefaultFaxScanSettingList.ss_ArraySize.empty()) {
			e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralTagMissing, "DefaultFaxScanSettingList.ArraySize" );
			return false;
		}
		
		if( !p_req->s_FaxScan.s_DefaultFaxScanSettingList.ss_ArraySize.range() ){
			e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralRangeIllegal, "DefaultFaxScanSettingList.ArraySize" );
			return false;
		}
		
		// DefaultFaxScanSetting.
		for( int i = 0; i < p_req->s_FaxScan.s_DefaultFaxScanSettingList.ss_ArraySize.get(); i++ ){
			const OAPS_DefaultFaxScanSetting& s_DefaultFaxScanSetting = p_req->s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i];
			
			// DisplayType.
			if( s_DefaultFaxScanSetting.e_DisplayType.empty() ){
				e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralTagMissing, "DefaultFaxScanSetting.DisplayType" );
				return false;
			}
		}
	}

	return true;
}
// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem = FaxScan) set.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_FaxScan>::set(const OAPS_AppReqSetUtilitySetting* p_req, OAPC_SyncLock& _slock, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=FaxScan)::set()" );

	e.setMessage( OAPE_Result_FaultRequest_AppReqSetUtilitySetting );
	
	//////////////////////////////////// .
	// I/F Lock .
	OAPC_ApiLock<APIC_OperationSetting> pc_OperationSetting(APIC_OperationSetting::newInstanceAdmin() , &_slock);
	OAPC_ApiWrapper<APIC_SystemInfo> pc_SystemInfo(APIC_SystemInfo::newInstanceAdmin() ); //[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified.

	if( !pc_OperationSetting || !pc_SystemInfo ) //[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified.
	{
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance");
		return false;
	}
	
	//[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified Start.
	//TYPE_MachineType e_MachineType = pc_SystemInfo->getMachineType();
	if(OAPI_CheckIsSparrowSFPModel())//IT6_SparrowBK 制御構成変更_No.36 HCL-Palani 2019/08/06 Modified.
	{
		if( True == checkExpImpAccountCheck( p_req->s_OperatorInfo.sc_AuthKey.get() ) ){
			return true;
		}
	}
	//[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified End.
	// ※便宜上.
	const OAPS_UtilitySetting_FaxScan& s_FaxScan = p_req->s_FaxScan;

	// DefaultFaxScanySetting.
	if( !s_FaxScan.s_DefaultFaxScanSettingList.empty() ) {
		for( int i = 0; i < s_FaxScan.s_DefaultFaxScanSettingList.ss_ArraySize.get(); i++ ) {
			const OAPS_DefaultFaxScanSetting& s_DefaultFaxScanSetting = s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i];

			StackGuard_Job pc_Job( TYPE_JobTypeFaxSendMemory );
			if( !pc_Job )
			{
				e.setKind(OAPE_Fault_faultcode_Server);
				e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance(Job)");
				return false;
			}
		
			TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
			if( (!pc_JobSpec)				||
				(!pc_JobSpec->pc_Common)	||
				(!pc_JobSpec->pc_Scan)		||
				(!pc_JobSpec->pc_Send)		)
			{
				e.setKind(OAPE_Fault_faultcode_Server);
				e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance(JobSpec)");
				return false;
			}
		
			// JobSpec直下.
			OAPC_ApiWrapper<TYPC_Common> pc_Common( pc_JobSpec->pc_Common ); 
			OAPC_ApiWrapper<TYPC_Scan>	 pc_Scan( pc_JobSpec->pc_Scan ); 
			OAPC_ApiWrapper<TYPC_Send>	 pc_Send( pc_JobSpec->pc_Send ); 

			// TYPC_Common.
			OAPC_ApiWrapper<TYPC_Resolution>		pc_Resolution( &pc_Common->c_Resolution ); 
			OAPC_ApiWrapper<TYPC_Resolution>		pc_FaxResolution( &pc_Common->c_FaxResolution ); 
			
			// TYPC_Send.
			OAPC_ApiWrapper<TYPC_ComSend>			pc_ComSend( &pc_Send->c_ComSend ); 
			
// comment [特ROM対応][MD-1(IT5-155マージ)]IT5-109対応のIT5 4.2定期特ROMへ機種展開要望 ->.
			OAPC_ApiLock<APIC_SoftDipSw> 			pc_SoftDipSw( APIC_SoftDipSw::newInstanceAdmin(), &_slock ); 
			if( !pc_SoftDipSw ){
				e.setKind(OAPE_Fault_faultcode_Server);
				e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance(DipSw)");
				return false;
			}

			// DipSwNo238 Bit0がOn 且つ、画面タイプが「通常画面」のとき(拡大画面は対象外).
			if( ( True                         == pc_SoftDipSw->getSoftDipSwBit( 132, 4 ) )
			 && ( OAPE_PanelDisplayType_Normal == s_DefaultFaxScanSetting.e_DisplayType ) ){

				// 各設定毎に、OAP_SRV_library.cpp/h内の関数を呼ぶ.
				// 組禁チェックもその中にある.

				// 以下は常に設定(ファクス発信元設定).
				APIC_FaxGfpSetting* pc_FaxGfpSetting = APIC_FaxGfpSetting::newInstanceAdmin();
				if( NULL == pc_FaxGfpSetting )
				{
					e.setKind(OAPE_Fault_faultcode_Server);
					e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance");
					return false;
				}
				TYPE_HeaderAddTx		e_HeaderAddTx;
				TYPE_Tti				e_TtiJobSpec;
				// 管理者モードで設定したTTI設定を取得する
				e_HeaderAddTx = pc_FaxGfpSetting->getHeaderAddTx();
				switch( e_HeaderAddTx ) {
					case TYPE_HeaderAddTx_In:		e_TtiJobSpec = TYPE_TtiInside;		break;
					case TYPE_HeaderAddTx_Out:		e_TtiJobSpec = TYPE_TtiOutside;		break;
					case TYPE_HeaderAddTx_Off:		e_TtiJobSpec = TYPE_TtiOff;			break;
					default: 						e_TtiJobSpec = TYPE_TtiOff;			break;
				}
				pc_JobSpec->pc_Fax->c_FaxCommon.setTti( e_TtiJobSpec );

				// カラーモード設定 保存.
				if( False == 
				  setFaxScanDefSet_Color( pc_Common, pc_Scan, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
					return false;
				}

				// 読込みサイズ設定 保存.
				if( False == 
				  setFaxScanDefSet_OriginalSize( pc_Scan, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
					return false;
				}

				// 解像度設定 保存.
				if( False == 
				  setFaxScanDefSet_Resolution( pc_Common, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
					return false;
				}

				// ファイル形式詳細設定 保存.
				if( False == 
				  setFaxScanDefSet_FileFormatSetting( pc_Common, pc_Send, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
					return false;
				}

				// 読込み面設定.
				if( False == 
				  setFaxScanDefSet_DuplexMode( pc_Scan, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
					return false;
				}

				// 件名設定.
				if( False == 
				  setFaxScanDefSet_Subject( pc_Send, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
					return false;
				}

				// 連続読込み設定.
				if( False == 
				  setFaxScanDefSet_SeparateScan( pc_Scan, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
					return false;
				}

				// 原稿指定設定.
				if( False == 
				  setFaxScanDefSet_OriginalSetting( pc_Common, pc_Scan, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
					return false;
				}

				// ブック原稿設定.
				if( False == 
				  setFaxScanDefSet_BookletOriginalSetting( pc_Common, pc_Scan, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
					return false;
				}

				// 原稿画質設定.
				if( False == 
				  setFaxScanDefSet_ImageQuality( pc_Common, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
					return false;
				}

				// 濃度設定.
				if( False == 
				  setFaxScanDefSet_DensityLevel( pc_Common, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
					return false;
				}

				// 下地調整設定.
				if( False == 
				  setFaxScanDefSet_GroundAdjustment( pc_Common, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
					return false;
				}

				// 日付/時刻スタンプ設定.
				if( False == 
				  setFaxScanDefSet_DateStamp( pc_Common, (const_cast<OAPS_DefaultFaxScanSetting*>(&s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i])), e ) ){
					return false;
				}

				// ページスタンプ設定.
				if( False == 
				  setFaxScanDefSet_PageStamp( pc_Common, (const_cast<OAPS_DefaultFaxScanSetting*>(&s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i])), e ) ){
					return false;
				}

				// スタンプ設定.
				if( False == 
				  setFaxScanDefSet_Stamp( pc_Common, (const_cast<OAPS_DefaultFaxScanSetting*>(&s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i])), e ) ){
					return false;
				}

				// ヘッダー/フッタースタンプ設定.
				if( False == 
				  setFaxScanDefSet_HeaderFooterStamp( pc_Common, (const_cast<OAPS_DefaultFaxScanSetting*>(&s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i])), e ) ){
					return false;
				}

				// 枠消し設定.
				if( False == 
				  setFaxScanDefSet_FrameEraseSetting( pc_Scan, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
					return false;
				}

				// シャープネス設定.
				if( False == 
				  setFaxScanDefSet_Sharpness( pc_Common, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
					return false;
				}

				// URL通知設定.
				if( False == 
				  setFaxScanDefSet_NotifyEMailAddress( pc_Send, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
					return false;
				}

			}else{

				//////////////.
				// Color.
				//////////////.
				// ※便宜上.
				if(!s_DefaultFaxScanSetting.e_Color.empty()) {
					Bool b_AcsOn = False;
					TYPE_ColorMode e_ColorMode = TYPE_ColorModeUnselect;
					Bool b_Binary = False;
					Bool b_Set = True;
					switch ( s_DefaultFaxScanSetting.e_Color.get() ) {
						case OAPE_Acs_Auto:
							b_AcsOn     = True;
							e_ColorMode = TYPE_ColorModeUnselect;
							b_Binary    = False;
							break;
						
						case OAPE_Acs_FullColor:
							b_AcsOn     = False;
							e_ColorMode = TYPE_ColorModeFullColor;
							b_Binary    = False;
							break;
						
						case OAPE_Acs_Grayscale:
							b_AcsOn     = False;
							e_ColorMode = TYPE_ColorModeBlack;
							b_Binary    = False;
							break;
						
						case OAPE_Acs_Monochrome:
							b_AcsOn     = False;
							e_ColorMode = TYPE_ColorModeUnselect;
							b_Binary    = True;
							break;
						default:
							b_Set = False;
							break;
					}
					if( True == b_Set ){
						pc_Scan->setAcsOn(b_AcsOn);
						pc_Common->setColorMode(e_ColorMode);
						pc_Common->setBinary(b_Binary);
					}
				}
				
				
				//////////////.
				// ファイル形式.
				//////////////.
				pc_ComSend.set_enum_skip_check( s_DefaultFaxScanSetting.e_FileFormat, &TYPC_ComSend::setFileFormat );
				
				if( !s_DefaultFaxScanSetting.e_FileFormat.empty() && OAPE_FileFormat_Jpeg == s_DefaultFaxScanSetting.e_FileFormat ){
					// Jpegの時は、ページ分割設定を1ページに固定(パネルと同じ処理).
					pc_ComSend->setPageSetting(TYPE_PageSettingSpecifiedPage);
					pc_ComSend->setSpecifiedPage( 1 );	// 1ページ固定.
				}
				
				
				//////////////.
				// 解像度.
				//////////////.
				pc_Resolution.set_enum_skip_check( s_DefaultFaxScanSetting.s_Resolution.e_Type, &TYPC_Resolution::setFaxResolution );
	
				pc_FaxResolution.set_enum_skip_check( s_DefaultFaxScanSetting.s_FaxResolution.e_Type, &TYPC_Resolution::setFaxResolution );
			}
// comment [特ROM対応][MD-1(IT5-155マージ)]IT5-109対応のIT5 4.2定期特ROMへ機種展開要望 <-.
			
			
			if( OAPE_PanelDisplayType_Normal == s_DefaultFaxScanSetting.e_DisplayType ) {
				pc_OperationSetting.set_enum_skip_check( s_DefaultFaxScanSetting.e_DefaultSetting, &APIC_OperationSetting::setInitModeFaxOn );
				pc_OperationSetting->setInitModeFax( pc_JobSpec );
				
			} else {
				pc_OperationSetting.set_enum_skip_check( s_DefaultFaxScanSetting.e_DefaultSetting, &APIC_OperationSetting::setInitModeFaxOnForLargeDisp );
				pc_OperationSetting->setInitModeFaxForLargeDisp( pc_JobSpec );
			}
		}
	}

	return true;
}

// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=FaxScan) get.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_FaxScan>::get(const OAPS_AppReqGetUtilitySetting* p_req, OAPS_AppResGetUtilitySetting* p_res, OAPC_SyncLock& _slock, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=FaxScan)::get()" );

	e.setMessage( OAPE_Result_FaultRequest_AppReqGetUtilitySetting );
	
	//////////////////////////////////// .
	// I/F Lock .
	OAPC_ApiLock<APIC_OperationSetting> pc_OperationSetting(APIC_OperationSetting::newInstanceAdmin() , &_slock);
	OAPC_ApiWrapper<APIC_SystemInfo> pc_SystemInfo(APIC_SystemInfo::newInstanceAdmin() );//[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified.

	if( !pc_OperationSetting || !pc_SystemInfo ) //[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified.
	{
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance");
		return false;
	}
	
	//[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified Start.
	
	//TYPE_MachineType e_MachineType = pc_SystemInfo->getMachineType();
	if(OAPI_CheckIsSparrowSFPModel())//IT6_SparrowBK 制御構成変更_No.36 HCL-Palani 2019/08/06 Modified.
	{
		if( True == checkExpImpAccountCheck( p_req->s_OperatorInfo.sc_AuthKey.get() ) ){
			return true;
		}
	}
	//[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified End.

	// ※便宜上.
	OAPS_UtilitySetting_FaxScan& s_FaxScan = p_res->s_FaxScan;

	// DefaultFaxScanSetting.
	s_FaxScan.s_DefaultFaxScanSettingList.ss_ArraySize = OAPD_DEFAUTSETTINGLIST_MAXLIST;
	for( int i = 0; i < s_FaxScan.s_DefaultFaxScanSettingList.ss_ArraySize.get(); i++ ) {
		OAPS_DefaultFaxScanSetting& s_DefaultFaxScanSetting = s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i];

		StackGuard_Job pc_Job( TYPE_JobTypeFaxSendMemory );
		if( !pc_Job )
		{
			e.setKind(OAPE_Fault_faultcode_Server);
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance(Job)");
			return false;
		}
		
		TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
		if( (!pc_JobSpec)				||
			(!pc_JobSpec->pc_Common)	||
			(!pc_JobSpec->pc_Scan)		||
			(!pc_JobSpec->pc_Send)		)
		{
			e.setKind(OAPE_Fault_faultcode_Server);
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance(JobSpec)");
			return false;
		}
		TYPC_Common* pc_Common = pc_JobSpec->pc_Common;
		TYPC_Scan*   pc_Scan   = pc_JobSpec->pc_Scan;
		TYPC_Send*   pc_Send   = pc_JobSpec->pc_Send;

		if( OAPE_PanelDisplayType_Normal == i){
			s_DefaultFaxScanSetting.e_DisplayType = OAPE_PanelDisplayType_Normal;
			getOpenAPIEnum( pc_OperationSetting->getInitModeFaxOn(), s_DefaultFaxScanSetting.e_DefaultSetting );
			
			// IT6_Eagle 1068382 2018/12/12 HCL-Naveen Added Start.
			if( True == pc_OperationSetting->getInitModeFaxOn() ) {
				pc_OperationSetting->getInitModeFax( pc_JobSpec );
			}
			// IT6_Eagle 1068382 2018/12/12 HCL-Naveen Added End.
		} else {
			s_DefaultFaxScanSetting.e_DisplayType = OAPE_PanelDisplayType_Enlarge;
			getOpenAPIEnum( pc_OperationSetting->getInitModeFaxOnForLargeDisp(), s_DefaultFaxScanSetting.e_DefaultSetting );
			
			// IT6_Eagle 1068382 2018/12/12 HCL-Naveen Added Start.
			if( True == pc_OperationSetting->getInitModeFaxOnForLargeDisp()) {
				pc_OperationSetting->getInitModeFaxForLargeDisp( pc_JobSpec );
			}
			// IT6_Eagle 1068382 2018/12/12 HCL-Naveen Added End.
		}

		//////////////.
		// Color.
		//////////////.
		// ※便宜上.
		Bool b_AcsOn = pc_Scan->getAcsOn();
		TYPE_ColorMode e_ColorMode = pc_Common->getColorMode();
		Bool b_Binary = pc_Common->getBinary();
		if( True == b_AcsOn ){
			s_DefaultFaxScanSetting.e_Color = OAPE_Acs_Auto;				// オートカラー.
		} else {
			if( True == b_Binary ) {
				s_DefaultFaxScanSetting.e_Color = OAPE_Acs_Monochrome;		// 白黒2値.
			} else {
				if( TYPE_ColorModeBlack == e_ColorMode ){
					s_DefaultFaxScanSetting.e_Color = OAPE_Acs_Grayscale;	// グレースケール.
				} else {
					s_DefaultFaxScanSetting.e_Color = OAPE_Acs_FullColor;	// FullColor.
				}
			}
		}
			
		//////////////.
		// ファイル形式.
		//////////////.
		getOpenAPIEnum( pc_Send->c_ComSend.getFileFormat(), s_DefaultFaxScanSetting.e_FileFormat );
			
			
		//////////////.
		// 解像度.
		//////////////.
		getOpenAPIEnum( pc_Common->c_Resolution.getFaxResolution(), s_DefaultFaxScanSetting.s_Resolution.e_Type );

		getOpenAPIEnum( pc_Common->c_FaxResolution.getFaxResolution(), s_DefaultFaxScanSetting.s_FaxResolution.e_Type );
		

// comment [特ROM対応][MD-1(IT5-155マージ)]IT5-109対応のIT5 4.2定期特ROMへ機種展開要望 ->.
		OAPC_ApiLock<APIC_SoftDipSw> 			pc_SoftDipSw( APIC_SoftDipSw::newInstanceAdmin(), &_slock ); 
		if( !pc_SoftDipSw ){
			e.setKind(OAPE_Fault_faultcode_Server);
			e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance(DipSw)");
			return false;
		}

		// DipSwNo238 Bit0がOn 且つ、画面タイプが「通常画面」のとき(拡大画面は対象外).
		if( ( True                         == pc_SoftDipSw->getSoftDipSwBit( 132, 4 ) )
		 && ( OAPE_PanelDisplayType_Normal == s_DefaultFaxScanSetting.e_DisplayType ) ){

			// 各設定毎に、OAP_SRV_library.cpp/h内の関数を呼ぶ.
			// 読込みサイズ設定 取得.
			if( False == 
			  getFaxScanDefSet_OriginalSize( pc_Scan, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
				return false;
			}

			// ファイル形式詳細設定.
			if( False == 
			  getFaxScanDefSet_FileFormatSetting( pc_Common, pc_Send, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
				return false;
			}

			// 読込み面設定.
			if( False == 
			  getFaxScanDefSet_DuplexMode( pc_Scan, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
				return false;
			}

			// 件名設定.
			if( False == 
			  getFaxScanDefSet_Subject( pc_Send, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
				return false;
			}

			// 連続読込み設定.
			if( False == 
			  getFaxScanDefSet_SeparateScan( pc_Scan, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
				return false;
			}

			// 原稿指定設定.
			if( False == 
			  getFaxScanDefSet_OriginalSetting( pc_Common, pc_Scan, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
				return false;
			}

			// ブック原稿設定.
			if( False == 
			  getFaxScanDefSet_BookletOriginalSetting( pc_Scan, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
				return false;
			}

			// 原稿画質設定.
			if( False == 
			  getFaxScanDefSet_ImageQuality( pc_Common, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
				return false;
			}

			// 濃度設定.
			if( False == 
			  getFaxScanDefSet_DensityLevel( pc_Common, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
				return false;
			}

			// 下地調整設定.
			if( False == 
			  getFaxScanDefSet_GroundAdjustment( pc_Common, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
				return false;
			}

			// 日付/時刻スタンプ設定.
			if( False == 
			  getFaxScanDefSet_DateStamp( pc_Common, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
				return false;
			}

			// ページスタンプ設定.
			if( False == 
			  getFaxScanDefSet_PageStamp( pc_Common, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
				return false;
			}

			// スタンプ設定.
			if( False == 
			  getFaxScanDefSet_Stamp( pc_Common, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
				return false;
			}

			// ヘッダー/フッタースタンプ設定.
			if( False == 
			  getFaxScanDefSet_HeaderFooterStamp( pc_Common, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
				return false;
			}

			// 枠消し設定.
			if( False == 
			  getFaxScanDefSet_FrameEraseSetting( pc_Scan, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
				return false;
			}

			// シャープネス設定.
			if( False == 
			  getFaxScanDefSet_Sharpness( pc_Common, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
				return false;
			}

			// URL通知設定.
			if( False == 
			  getFaxScanDefSet_NotifyEMailAddress( pc_Send, &s_FaxScan.s_DefaultFaxScanSettingList.s_DefaultFaxScanSetting[i], e ) ){
				return false;
			}
		}
// comment [特ROM対応][MD-1(IT5-155マージ)]IT5-109対応のIT5 4.2定期特ROMへ機種展開要望 <-.


	}
	return true;
}
// MinervaZeus3rd UV-01309 Muratani 2013/03/18 End.
// ZeusS UV-01439 HCL-Pradeep 2014/08/18 Added Start.
// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=Fax).
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Fax>::OAPC_UtilityItemImp()
{
	activate();
}
template<> OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Fax>::~OAPC_UtilityItemImp()
{}

// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=Fax) pre_check.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Fax>::pre_check(const OAPS_AppReqSetUtilitySetting* p_req, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=Fax)::pre_check()" );
	//[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified Start.
	OAPC_ApiWrapper<APIC_SystemInfo> pc_SystemInfo(APIC_SystemInfo::newInstanceAdmin() );
	if( !pc_SystemInfo ) {
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Cannot Get Instance" );
		return false;
	}
	
	
	//TYPE_MachineType e_MachineType = pc_SystemInfo->getMachineType();
	if(OAPI_CheckIsSparrowSFPModel())//IT6_SparrowBK 制御構成変更_No.36 HCL-Palani 2019/08/06 Modified.
	{
		if( False == checkExpImpAccountCheck( p_req->s_OperatorInfo.sc_AuthKey.get() ) ){
			e.setKind(OAPE_Fault_faultcode_Server);
			e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralIllegalValue , "RequestItem:Fax" );
			return false;
		}
	}
	//[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified End.
	
	return true;
}
// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem = Fax) set.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Fax>::set(const OAPS_AppReqSetUtilitySetting* p_req, OAPC_SyncLock& _slock, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=Fax)::set()" );

	e.setMessage( OAPE_Result_FaultRequest_AppReqSetUtilitySetting );
	
	// I/F Lock .
	OAPC_ApiLock<APIC_OperationSetting> pc_OperationSetting(APIC_OperationSetting::newInstanceAdmin() , &_slock);
	OAPC_ApiWrapper<APIC_SystemInfo> pc_SystemInfo(APIC_SystemInfo::newInstanceAdmin() ); //[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified.

	if( !pc_OperationSetting || !pc_SystemInfo ) //[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified.
	{
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance");
		return false;
	}
	
	//[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified Start.
	//TYPE_MachineType e_MachineType = pc_SystemInfo->getMachineType();
	if(OAPI_CheckIsSparrowSFPModel())//IT6_SparrowBK 制御構成変更_No.36 HCL-Palani 2019/08/06 Modified.
	{
		if( True == checkExpImpAccountCheck( p_req->s_OperatorInfo.sc_AuthKey.get() ) ){
			return true;
		}
	}
//[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified End.
	//ZeusS_ZX0 CQ525109 HCL-Vijay 2016/02/16 Added Start.	
	StackGuard_Job pc_Job( TYPE_JobTypeFaxSendMemory );
	if( !pc_Job )
	{
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance(Job)");
		return false;
	}
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if(!pc_JobSpec){
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance(JobSpec)");
		return false;
	}
	// DefaultFaxSetting.	
	pc_OperationSetting.set_enum_skip_check( p_req->s_Fax.e_DefaultSetting, &APIC_OperationSetting::setInitModeFaxAppOn );
	pc_OperationSetting->setInitModeFaxApp( pc_JobSpec );	
	//ZeusS_ZX0 CQ525109 HCL-Vijay 2016/02/16 Added End.
	return true;
}

// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=Fax) get.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Fax>::get(const OAPS_AppReqGetUtilitySetting* p_req, OAPS_AppResGetUtilitySetting* p_res, OAPC_SyncLock& _slock, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=Fax)::get()" );

	e.setMessage( OAPE_Result_FaultRequest_AppReqGetUtilitySetting );
	
	// I/F Lock .
	OAPC_ApiLock<APIC_OperationSetting> pc_OperationSetting(APIC_OperationSetting::newInstanceAdmin() , &_slock);
	OAPC_ApiWrapper<APIC_SystemInfo> pc_SystemInfo(APIC_SystemInfo::newInstanceAdmin() ); //[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified.

	if( !pc_OperationSetting || !pc_SystemInfo) //[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified.
	{
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance");
		return false;
	}
	//[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified Start.
	//TYPE_MachineType e_MachineType = pc_SystemInfo->getMachineType();
	if(OAPI_CheckIsSparrowSFPModel())//IT6_SparrowBK 制御構成変更_No.36 HCL-Palani 2019/08/06 Modified.
	{
		if( True == checkExpImpAccountCheck( p_req->s_OperatorInfo.sc_AuthKey.get() ) ){
			return true;
		}
	}
//[IT6_SPARROW]WP_No.206-1 2018/03/08 Hcl-Neeraj Modified End.
	//ZeusS_ZX0 CQ525109 HCL-Vijay 2016/02/16 Added Start.	
	StackGuard_Job pc_Job( TYPE_JobTypeFaxSendMemory );
	if( !pc_Job )
	{
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance(Job)");
		return false;
	}	
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if (!pc_JobSpec){
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance(JobSpec)");
		return false;
	}
	// DefaultFaxSetting.
	getOpenAPIEnum( pc_OperationSetting->getInitModeFaxAppOn(), p_res->s_Fax.e_DefaultSetting );

	// IT6_Eagle 1068382 2018/12/12 HCL-Naveen Added Start.
	if( True == pc_OperationSetting->getInitModeFaxAppOn()) {
		pc_OperationSetting->getInitModeFaxApp( pc_JobSpec );
	}
	// IT6_Eagle 1068382 2018/12/12 HCL-Naveen Added End.
	//ZeusS_ZX0 CQ525109 HCL-Vijay 2016/02/16 Added End.
		
	return true;
}
// ZeusS UV-01439 HCL-Pradeep 2014/08/18 Added End.

//2017/03/15 スキャン設定_XML基盤技術開発v2 pactera add start.
// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=Scan).
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Scan>::OAPC_UtilityItemImp()
{
	activate();
}
template<> OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Scan>::~OAPC_UtilityItemImp()
{}

// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=Scan) pre_check.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Scan>::pre_check(const OAPS_AppReqSetUtilitySetting* p_req, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=Scan)::pre_check()" );
	
	return true;
}

// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem = Scan) set.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Scan>::set(const OAPS_AppReqSetUtilitySetting* p_req, OAPC_SyncLock& _slock, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=Scan)::set()" );

	e.setMessage( OAPE_Result_FaultRequest_AppReqSetUtilitySetting );
	
	OAPC_ApiLock<APIC_ScanJobSetting> pc_ScanJobSetting(APIC_ScanJobSetting::newInstanceAdmin() , &_slock);

	if( !pc_ScanJobSetting )
	{
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance");
		return false;
	}
	
	const OAPS_UtilitySetting_Scan& s_Scan = p_req->s_Scan;
#ifndef DEF_DenebMLKCus  //【IT6_EagleBK】制御構成変更_No.30 対応 2019/06/03 HCL-Mangai Commented. // DenebMLK 制御構成変更_No.72 Chitra Added 2018/05/31 Start.
//#if !( defined  DEF_DenebMLKCus  ||  defined DEF_MonoCus )  //【IT6_EagleBK】制御構成変更_No.30 対応 2019/06/03 HCL-Mangai Added.//OP_BTS-11478 HCL-Garima 2019/08/29 commented.
	pc_ScanJobSetting.set_enum_skip_check( s_Scan.e_ColorTIFFFormat, &APIC_ScanJobSetting::setColorTIFFFormat );
#endif // DenebMLK 制御構成変更_No.72 Chitra Added 2018/05/31 End.
	pc_ScanJobSetting.set_enum_skip_check( s_Scan.e_FileNameAutoExtension, &APIC_ScanJobSetting::setFileNameAutoExtension );
	pc_ScanJobSetting.set_enum_skip_check( s_Scan.e_FileFormatPdf, &APIC_ScanJobSetting::setDefaultPDF );
	pc_ScanJobSetting.set_enum_skip_check( s_Scan.e_FileFormatXps, &APIC_ScanJobSetting::setDefaultXPS );
	
	return true;
}

template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Scan>::get(const OAPS_AppReqGetUtilitySetting* p_req, OAPS_AppResGetUtilitySetting* p_res, OAPC_SyncLock& _slock, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=Scan)::get()" );

	e.setMessage( OAPE_Result_FaultRequest_AppReqGetUtilitySetting );
	
	OAPC_ApiLock<APIC_ScanJobSetting> pc_ScanJobSetting(APIC_ScanJobSetting::newInstanceAdmin() , &_slock);

	if( !pc_ScanJobSetting )
	{
		e.setKind(OAPE_Fault_faultcode_Server);
		e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralAllocateBufferFull , "Failed to get instance");
		return false;
	}
#ifndef DEF_DenebMLKCus //【IT6_EagleBK】制御構成変更_No.30 対応 2019/06/03 HCL-Mangai Commented. // DenebMLK 制御構成変更_No.72 Chitra Added 2018/05/31 Start.
//#if !( defined  DEF_DenebMLKCus  ||  defined DEF_MonoCus )  //【IT6_EagleBK】制御構成変更_No.30 対応 2019/06/03 HCL-Mangai Added.//OP_BTS-11478 HCL-Garima 2019/08/29 commented.
	getOpenAPIEnum( pc_ScanJobSetting->getColorTIFFFormat(), p_res->s_Scan.e_ColorTIFFFormat );
#endif // DenebMLK 制御構成変更_No.72 Chitra Added 2018/05/31 End.	
	getOpenAPIEnum( pc_ScanJobSetting->getFileNameAutoExtension(), p_res->s_Scan.e_FileNameAutoExtension );
	
	getOpenAPIEnum( pc_ScanJobSetting->getDefaultPDF(), p_res->s_Scan.e_FileFormatPdf );
	
	getOpenAPIEnum( pc_ScanJobSetting->getDefaultXPS(), p_res->s_Scan.e_FileFormatXps );
	
	return true;
}
//2017/03/15 スキャン設定_XML基盤技術開発v2 pactera add end.

// ZeusS_ZX0 RQ-320対応 2015/7/15 1OPS Ohno Start. 
// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=Network).
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Network>::OAPC_UtilityItemImp()
{
	activate();
}
template<> OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Network>::~OAPC_UtilityItemImp()
{}

// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=Network) pre_check.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Network>::pre_check(const OAPS_AppReqSetUtilitySetting* p_req, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=Network)::pre_check()" );
	
	return true;
}
//2022/11/24 [IT6_2.3][制御構成変更_No.52]FPT-HoaPH Add Started.
static void ans_CallbackNetworkReset(Bool b_result)
{
}
//2022/11/24 [IT6_2.3][制御構成変更_No.52]FPT-HoaPH Add Ended.
// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem = Network) set.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Network>::set(const OAPS_AppReqSetUtilitySetting* p_req, OAPC_SyncLock& _slock, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=Network)::set()" );

	e.setMessage( OAPE_Result_FaultRequest_AppReqSetUtilitySetting );
	
	OAPC_ApiLock<APIC_IWSSetting> pc_IWSSetting( APIC_IWSSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiWrapper<APIC_SystemInfo>    pc_SystemInfo( APIC_SystemInfo::newInstanceAdmin()); //2022/11/24 [IT6_2.3][制御構成変更_No.52]FPT-HoaPH Added.
	if ( !pc_IWSSetting 
		||!pc_SystemInfo	//2022/11/24 [IT6_2.3][制御構成変更_No.52]FPT-HoaPH Added.
	)
	{
		e.setKind( OAPE_Fault_faultcode_Server );
		e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralAllocateBufferFull, "Failed to get instance" );
		return false;
	}

	// Exp-Imp特殊ユーザーのみ受付. 
	if( True == checkExpImpAccountCheck( p_req->s_OperatorInfo.sc_AuthKey.get() ) )
	{
		// 管理者パスワード変更制限設定. 
		pc_IWSSetting.set_enum_skip_check( p_req->s_Network.s_Iws.e_AdminPasswordChangeLimit, &APIC_IWSSetting::setAdminPasswordChangeLimit );
	}
	//2022/11/24 [IT6_2.3][制御構成変更_No.52]FPT-HoaPH Add Started.
	if(!p_req->s_Network.e_ResetNetwork.empty())
	{
		if(p_req->s_Network.e_ResetNetwork.get()==OAPE_Bool_true)
		{
			pc_SystemInfo->req_mioReset(ans_CallbackNetworkReset);
		}else{
			//NOP
		}
	}
	//2022/11/24 [IT6_2.3][制御構成変更_No.52]FPT-HoaPH Add Ended.
	return true;
}

//2023/03/08 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1576]FPT-VanPN2 Add Start.
#define STRING_DEFAULT_LMS_SERVER_URL "https://licensemanage.com:443/"
#define STRING_DEFAULT_DAV_SERVER_URL "https://remote-service-pf.com:443/"
#define STRING_DEFAULT_URL "0.0.0.0"
//2023/03/08 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1576]FPT-VanPN2 Add End.

// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=Network) get.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_Network>::get(const OAPS_AppReqGetUtilitySetting* p_req, OAPS_AppResGetUtilitySetting* p_res, OAPC_SyncLock& _slock, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=Network)::get()" );

	e.setMessage( OAPE_Result_FaultRequest_AppReqGetUtilitySetting );
	
	OAPC_ApiLock<APIC_IWSSetting> pc_IWSSetting( APIC_IWSSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_SslSetting>    pc_SslSetting(APIC_SslSetting::newInstanceAdmin(), &_slock);  		// OP_BTS-5966
	OAPC_ApiLock<APIC_TcpIpSetting> pc_TcpIpSetting(APIC_TcpIpSetting::newInstanceAdmin(), &_slock); 	// OP_BTS-5966
	OAPC_ApiLock<APIC_NetworkDevice> pc_NetworkDevice(APIC_NetworkDevice::newInstanceAdmin(), &_slock); //2022/11/22 [IT6_2.3][制御構成変更_No.52]FPT-HoaPH Added.
	OAPC_ApiWrapper<APIC_ExNetworkSetting> pc_ExNetworkSetting( APIC_ExNetworkSetting::newInstanceAdmin() );//2022/11/22 [IT6_2.3][制御構成変更_No.52]FPT-HoaPH Added.
	OAPC_ApiWrapper<APIC_RoimWscDataIF> pc_RoimWscDataIF(APIC_RoimWscDataIF::newInstanceAdmin());				//2023/03/08 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1576]FPT-VanPN2 Add
	OAPC_ApiLock<APIC_WebDavSetting> pc_WebDavSetting(APIC_WebDavSetting::newInstanceAdmin(), &_slock);			//2023/03/08 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1576]FPT-VanPN2 Add
	OAPC_ApiLock<APIC_WebDavCSRCSetting> pc_WebDavCSRCSetting(APIC_WebDavCSRCSetting::newInstanceAdmin(), &_slock);		//2023/03/08 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1576]FPT-VanPN2 Add
	OAPC_ApiLock<APIC_RdsSetting> pc_RdsSetting(APIC_RdsSetting::newInstance(), &_slock);		//2023/03/08 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1576]FPT-VanPN2 Add
	
	if ( !pc_IWSSetting 
	|| !pc_SslSetting 		// OP_BTS-5966
	|| !pc_TcpIpSetting		// OP_BTS-5966
	|| !pc_NetworkDevice	//2022/11/22 [IT6_2.3][制御構成変更_No.52]FPT-HoaPH Added.
	|| !pc_ExNetworkSetting	//2022/11/22 [IT6_2.3][制御構成変更_No.52]FPT-HoaPH Added.
	|| !pc_RoimWscDataIF	//2023/03/08 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1576]FPT-VanPN2 Add
	|| !pc_WebDavSetting	//2023/03/08 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1576]FPT-VanPN2 Add
	|| !pc_WebDavCSRCSetting//2023/03/08 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1576]FPT-VanPN2 Add
	|| !pc_RdsSetting		//2023/03/08 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1576]FPT-VanPN2 Add
	)
	{
		e.setKind( OAPE_Fault_faultcode_Server );
		e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralAllocateBufferFull, "Failed to get instance" );
		return false;
	}

	// Exp-Imp特殊ユーザーのみ受付. 
	if( True == checkExpImpAccountCheck( p_req->s_OperatorInfo.sc_AuthKey.get() ) )
	{
		// 管理者パスワード変更制限設定. 
		getOpenAPIEnum( pc_IWSSetting->getAdminPasswordChangeLimit(), p_res->s_Network.s_Iws.e_AdminPasswordChangeLimit );
	}		

		// OP_BTS-5966 start.
	
	// デバイス証明書リストの取得.
	TYPC_SslCertList	c_SslCertList[OAPD_SSLCERTLIST_SSLCERT_MAXLIST];
	TYPE_SslStatus		 e_sslStatus;
	uint32_t				ulCertNum = OAPD_SSLCERTLIST_SSLCERT_MAXLIST;
	e_sslStatus = pc_SslSetting->getSslCertList(0,TYPD_CertType_Device,&ulCertNum,&c_SslCertList[0]);

	// デバイス証明書リストの取得：証明書要求中.
	TYPC_SslCertList	c_SslCertListReq[OAPD_SSLCERTLIST_SSLCERT_MAXLIST];
	TYPE_SslStatus 		e_sslStatus_Req;
	uint32_t				ulCertReqNum = OAPD_SSLCERTLIST_SSLCERT_MAXLIST;
	e_sslStatus_Req = pc_SslSetting->getSslCertList(0,TYPD_CertType_DeviceReq,&ulCertReqNum,&c_SslCertListReq[0]);

	//ループカウンタ.
	uint32_t				ulCertCnt = 0;
	uint32_t				ulCertReqCnt = 0;
	// ホスト名取得.
	char c_HostName[HOST_NAME_MAX_LEN] = {0,};
	pc_TcpIpSetting->getHostName(c_HostName);
	// ドメイン名取得.
	char c_DomainName[DOMAIN_NAME_MAX_LEN] = {0,};
	pc_TcpIpSetting->getDomainName(c_DomainName);

	if ( e_sslStatus == TYPE_SslStatus_SslSuccess ) {
		if ( ulCertNum > 0 ) {
			for ( ulCertCnt = 0; ulCertCnt < ulCertNum; ulCertCnt++ ) {
				if ( c_SslCertList[ulCertCnt].ul_CertID == 0 ) {
					break;
				}
				p_res->s_Network.s_SslCertList.s_SslCert[ulCertCnt].ul_CertID = c_SslCertList[ulCertCnt].ul_CertID;
				p_res->s_Network.s_SslCertList.s_SslCert[ulCertCnt].sc_Issuer = c_SslCertList[ulCertCnt].uc_Issuer;
				p_res->s_Network.s_SslCertList.s_SslCert[ulCertCnt].sc_Subject = c_SslCertList[ulCertCnt].uc_Subject;
				getOpenAPIEnum(c_SslCertList[ulCertCnt].ul_CertUsage, p_res->s_Network.s_SslCertList.s_SslCert[ulCertCnt].e_CertUsage);
				getOpenAPIEnum(c_SslCertList[ulCertCnt].ul_CertUsageApp, p_res->s_Network.s_SslCertList.s_SslCert[ulCertCnt].e_CertUsageApp);
				getOpenAPIEnum(pc_SslSetting->checkSSLCertReCreate(c_HostName, c_DomainName, c_SslCertList[ulCertCnt].ul_CertID), p_res->s_Network.s_SslCertList.s_SslCert[ulCertCnt].e_SslCertReCreate);
			}
		}
	}
	if ( e_sslStatus_Req == TYPE_SslStatus_SslSuccess ) {
		if( ulCertReqNum > 0 ) {
			for( ulCertReqCnt = 0; ulCertReqCnt < ulCertReqNum; ulCertReqCnt++ ) {
				if ( c_SslCertListReq[ulCertReqCnt].ul_CertID == 0 ) {
					break;
				}
				p_res->s_Network.s_SslCertList.s_SslCert[ulCertCnt + ulCertReqCnt].ul_CertID = c_SslCertListReq[ulCertReqCnt].ul_CertID;
				p_res->s_Network.s_SslCertList.s_SslCert[ulCertCnt + ulCertReqCnt].sc_Issuer = c_SslCertListReq[ulCertReqCnt].uc_Issuer;
				p_res->s_Network.s_SslCertList.s_SslCert[ulCertCnt + ulCertReqCnt].sc_Subject = c_SslCertListReq[ulCertReqCnt].uc_Subject;
				getOpenAPIEnum(c_SslCertListReq[ulCertReqCnt].ul_CertUsage, p_res->s_Network.s_SslCertList.s_SslCert[ulCertCnt + ulCertReqCnt].e_CertUsage);
				getOpenAPIEnum(c_SslCertListReq[ulCertReqCnt].ul_CertUsageApp, p_res->s_Network.s_SslCertList.s_SslCert[ulCertCnt + ulCertReqCnt].e_CertUsageApp);
				getOpenAPIEnum(pc_SslSetting->checkSSLCertReCreate(c_HostName, c_DomainName, c_SslCertList[ulCertCnt].ul_CertID), p_res->s_Network.s_SslCertList.s_SslCert[ulCertCnt + ulCertReqCnt].e_SslCertReCreate);
			}
		}
	}
	p_res->s_Network.s_SslCertList.ss_ArraySize = ulCertNum + ulCertReqCnt;
	if ( (ulCertNum + ulCertReqCnt) > 0 ) {
		if ( e_sslStatus == TYPE_SslStatus_SslSuccess ) {
			getOpenAPIEnum(e_sslStatus, p_res->s_Network.s_SslCertList.e_SslStatus);
		} else if (e_sslStatus_Req == TYPE_SslStatus_SslSuccess ) {
			getOpenAPIEnum(e_sslStatus_Req, p_res->s_Network.s_SslCertList.e_SslStatus);
		} else {
			getOpenAPIEnum(e_sslStatus, p_res->s_Network.s_SslCertList.e_SslStatus);
		}
	}
//2022/11/22 [IT6_2.3][制御構成変更_No.52]FPT-HoaPH Add Started.
	if(!p_req->e_NetworkTypeMode.empty())
	{
		switch(p_req->e_NetworkTypeMode.get())
		{
			case OAPE_NetworkTypeMode_Intranet:
				//2023/06/05 [IT6_2.3] [OP_BTS-43758] FPT-ChienND7 Mod Started.
				// getOpenAPIEnum(pc_NetworkDevice->getChannelStatus(TYPE_ChannelTypeNetwork),p_res->s_Network.s_NetworkConfirm.s_Intranet.e_Status);	//2023/06/05 [IT6_2.3] [OP_BTS-43758] FPT-ChienND7 Comment Out.
				getOpenAPIEnum(pc_NetworkDevice->getChannelStatus(TYPE_ChannelTypeHttpClient),p_res->s_Network.s_NetworkConfirm.s_Intranet.e_Status);
				//2023/06/05 [IT6_2.3] [OP_BTS-43758] FPT-ChienND7 Mod Started.
				break;
			case OAPE_NetworkTypeMode_AccessPoint:
			//2023/03/07 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1579]FPT-VanPN2 Mod Start.
			{
				// 2023/03/07 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1579]FPT-TungDH9 Added Start
				uint32_t ui32_CurrentTick = SYSF_tickGet();
				bool bl_ConnectionTimeout = false;
				if (ui32_StartConnectTick == 0)
				{
					ui32_StartConnectTick = ui32_CurrentTick;
				}
				else
				{
					if (ui32_CurrentTick - ui32_StartConnectTick  > 30 * DEF_TICKTOSECOND) // 30 second
					{
						bl_ConnectionTimeout = true;
					}
				}
				// 2023/03/07 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1579]FPT-TungDH9 Added End

				TYPS_WlanApConnectStatusInfo s_StatusInfo;
				// getOpenAPIEnum(pc_ExNetworkSetting->getConnectStatus(&s_StatusInfo),p_res->s_Network.s_NetworkConfirm.s_AccessPoint.e_Result); // VanPN2 Comment out
				if(pc_ExNetworkSetting->getConnectStatus(&s_StatusInfo) == TYPE_WlanGetConnectStatus_Result_Ok)
				{
					getOpenAPIEnum(s_StatusInfo.e_WlanApStatus,p_res->s_Network.s_NetworkConfirm.s_AccessPoint.e_Result);
				}
				else
				{
					p_res->s_Network.s_NetworkConfirm.s_AccessPoint.e_Result = OAPE_AccessPoint_Result_NG;
				}
				// if response is NG in session, change to ConnectionWaiting
				if(!bl_ConnectionTimeout && p_res->s_Network.s_NetworkConfirm.s_AccessPoint.e_Result == OAPE_AccessPoint_Result_NG)
				{
					p_res->s_Network.s_NetworkConfirm.s_AccessPoint.e_Result = OAPE_AccessPoint_Result_ConnectionWaiting;
				}
				// Reset session check  status connection of Access Point and MFP
				if(	p_res->s_Network.s_NetworkConfirm.s_AccessPoint.e_Result == OAPE_AccessPoint_Result_OK
					|| p_res->s_Network.s_NetworkConfirm.s_AccessPoint.e_Result == OAPE_AccessPoint_Result_NG)
				{
					ui32_StartConnectTick = 0;
				}
				break;
			}
			//2023/03/07 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1579]FPT-VanPN2 Mod End.
			case OAPE_NetworkTypeMode_Internet:
				//2023/03/08 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1576]FPT-VanPN2 Add Start.
				{
					t_sMioSendWebDAV  s_MioSendWebDAV;
					memset(&s_MioSendWebDAV, 0x00, sizeof(t_sMioSendWebDAV));
					t_sMioProxyInfo s_sMioProxyInfoWebDAV;
					memset(&s_sMioProxyInfoWebDAV, 0x00, sizeof(t_sMioProxyInfo));

					unsigned char uc_LMS_URL[MAX_WEBDAV_REQURI] = {'\0'};
					pc_RdsSetting->getCsrcWebServerUrlAddressForProductCert(uc_LMS_URL);
					if(strcmp(reinterpret_cast<char*>(uc_LMS_URL), "") == 0 || strcmp(reinterpret_cast<char*>(uc_LMS_URL), STRING_DEFAULT_URL) == 0)
					{
						memcpy(reinterpret_cast<char*>(s_MioSendWebDAV.ucSendURL), STRING_DEFAULT_LMS_SERVER_URL, sizeof(STRING_DEFAULT_LMS_SERVER_URL));
					}
					else
					{
						memcpy(s_MioSendWebDAV.ucSendURL, uc_LMS_URL, MAX_WEBDAV_REQURI);
						unsigned short us_lastIndex = (unsigned short)strlen(reinterpret_cast<char*>(s_MioSendWebDAV.ucSendURL)) - 1;
						// Must have character '/' at the end of URL
						if(reinterpret_cast<char*>(s_MioSendWebDAV.ucSendURL)[us_lastIndex] != '/')
						{
							strcat(reinterpret_cast<char*>(s_MioSendWebDAV.ucSendURL), "/");
						}
					}

					Bool bl_RemoteCareProxyEnable = False;
					bl_RemoteCareProxyEnable = pc_WebDavCSRCSetting->getEnableWebDavCsrc();
					if(bl_RemoteCareProxyEnable == True) // 経由するプロキシサーバー
					{
						s_MioSendWebDAV.bEnableProxy = TRUE;
						s_MioSendWebDAV.pProxyInfo = &s_sMioProxyInfoWebDAV;   // Allocate memory for Proxy
						
						Bool bl_ProxyReferToWebDav = False;
						bl_ProxyReferToWebDav = pc_WebDavCSRCSetting->getExtractWebDAVClientSetting();
						// Set info for Proxy start
						unsigned char uc_ProxyServerURL[TYPD_RoimWscDataIF_ProxyServer_URLMaxLen] = {0,};
						unsigned char uc_ProxyServerUserName[TYPD_RoimWscDataIF_ProxyServer_UserNameMaxLen] = {0,};
						unsigned char uc_ProxyServerPassword[TYPD_RoimWscDataIF_ProxyServer_PasswordMaxLen] = {0,};
						unsigned short us_ProxyPortNum = 0;

						if(bl_ProxyReferToWebDav == False) // WebDAVクライアント設定と連動：ON(連動する)の場合
						{
							pc_WebDavSetting->getProxyAddress(uc_ProxyServerURL);
							us_ProxyPortNum = pc_WebDavSetting->getProxyPortNum();
							pc_WebDavSetting->getProxyUserName(uc_ProxyServerUserName);
							pc_WebDavSetting->getProxyPassword(uc_ProxyServerPassword);
						}
						else // WebDAVクライアント設定と連動：OFF(連動しない)の場合
						{
							pc_WebDavCSRCSetting->getProxyAddress(uc_ProxyServerURL);
							us_ProxyPortNum = pc_WebDavCSRCSetting->getProxyPortNum();
							pc_WebDavCSRCSetting->getProxyUserName(uc_ProxyServerUserName);
							pc_WebDavCSRCSetting->getProxyPassword(uc_ProxyServerPassword);
						}
						//2023/04/12 FPT-VietDQ11 OP_BTS-42640 Add Start.
						if((strncmp(reinterpret_cast<const char*>(uc_ProxyServerURL), STRING_DEFAULT_URL, (size_t)TYPD_RoimWscDataIF_ProxyServer_URLMaxLen) != 0) && (strncmp(reinterpret_cast<const char*>(uc_ProxyServerURL), "",(size_t)TYPD_RoimWscDataIF_ProxyServer_URLMaxLen) != 0))
						{
							memcpy(s_MioSendWebDAV.pProxyInfo->ucProxyAddr, uc_ProxyServerURL, TYPD_RoimWscDataIF_ProxyServer_URLMaxLen);
							s_MioSendWebDAV.pProxyInfo->usProxyPortNum = us_ProxyPortNum;
							memcpy(s_MioSendWebDAV.pProxyInfo->ucProxyUser, uc_ProxyServerUserName, TYPD_RoimWscDataIF_ProxyServer_UserNameMaxLen);
							memcpy(s_MioSendWebDAV.pProxyInfo->ucProxyPass, uc_ProxyServerPassword, TYPD_RoimWscDataIF_ProxyServer_PasswordMaxLen);
						}
						else
						{
							s_MioSendWebDAV.bEnableProxy = FALSE;
							s_MioSendWebDAV.pProxyInfo = NULL;
						}
						//2023/04/12 FPT-VietDQ11 OP_BTS-42640 Add End.
						// Set info for Proxy end
					}
					MioFileDesc fd = mioOpen("/mio/sw/14", O_WRONLY);
					// STATUS ret_error = mioCheckConnection(fd, SEND_TYPE_WEBDAV, &s_MioSendWebDAV);
					STATUS ret_error = APIC_WebDAVTx::newInstance()->checkConnection(fd, &s_MioSendWebDAV);
					mioClose(fd);

					if(ret_error == OK)
					{
						p_res->s_Network.s_NetworkConfirm.s_Internet.e_Status = OAPE_NetworkStatus_OK;
					}
					else
					{
						p_res->s_Network.s_NetworkConfirm.s_Internet.e_Status = OAPE_NetworkStatus_NG;
						if(ret_error == 408)
						{
							p_res->s_Network.s_NetworkConfirm.s_Internet.e_ErrorCode = OAPE_NetworkErrorCode_Connection_timeout;
						}
						else
						{
							p_res->s_Network.s_NetworkConfirm.s_Internet.e_ErrorCode = OAPE_NetworkErrorCode_Connection_refuse;
						}
						
					}
				}
				//2023/03/08 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1576]FPT-VanPN2 Add End.
				break;
			case OAPE_NetworkTypeMode_WWRSPFserver:
				//2023/03/08 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1576]FPT-VanPN2 Add Start.
				{
					
					Bool bl_RemoteServiceEnable = False;
					pc_RoimWscDataIF->getWWRemoteServicePermit(&bl_RemoteServiceEnable);
					TYPS_RoimWscDataIF_CertInstallationStatus s_CertInstallationStatus;
					memset(&s_CertInstallationStatus, 0x00, sizeof(TYPS_RoimWscDataIF_CertInstallationStatus));
					pc_RoimWscDataIF->getCertInstallationStatus( &s_CertInstallationStatus );
					// 管理者設定＞リモートサービス設定：ONの場合、WWRSPFサーバとの接続確認を行う。
					// Cert: 未登録状態であればNGとする。
					if(bl_RemoteServiceEnable == True && s_CertInstallationStatus.m_e_Result == TYPE_RoimWscDataIF_CertInstallationResult_OK) 
					{
						TYPS_RoimWscDataIF_ServerInfo s_ServerInfo;
						memset(&s_ServerInfo, 0x00, sizeof(TYPS_RoimWscDataIF_ServerInfo));
						pc_RoimWscDataIF->getServerInfo( &s_ServerInfo ); // Get current setting of server

						t_sMioSendWebDAV  s_MioSendWebDAV;
						memset(&s_MioSendWebDAV, 0x00, sizeof(t_sMioSendWebDAV));
						t_sMioProxyInfo s_sMioProxyInfoWebDAV;
						memset(&s_sMioProxyInfoWebDAV, 0x00, sizeof(t_sMioProxyInfo));

						// Set URL of WebDAV
						if(strcmp(s_ServerInfo.m_sc_URL, STRING_DEFAULT_URL) == 0 || strcmp(s_ServerInfo.m_sc_URL, "") == 0)
						{
							memcpy(reinterpret_cast<char*>(s_MioSendWebDAV.ucSendURL), STRING_DEFAULT_DAV_SERVER_URL, sizeof(STRING_DEFAULT_DAV_SERVER_URL));
						}
						else
						{
							memcpy(reinterpret_cast<char*>(s_MioSendWebDAV.ucSendURL), s_ServerInfo.m_sc_URL, sizeof(s_ServerInfo.m_sc_URL));
							unsigned short us_lastIndex = (unsigned short)strlen(reinterpret_cast<char*>(s_MioSendWebDAV.ucSendURL)) - 1;
							// Must have character '/' at the end of URL
							if(s_ServerInfo.m_sc_URL[us_lastIndex] != '/')
							{
								strcat(reinterpret_cast<char*>(s_MioSendWebDAV.ucSendURL), "/");
							}
						}

						Bool bl_ProxySVR_Enable = False;
						pc_RoimWscDataIF->getProxySVR_Enable(&bl_ProxySVR_Enable);
						// Set info of Proxy server
						if(bl_ProxySVR_Enable == True)	//サービス設定＞リモートサービス設定＞Proxyサーバー設定＞使用設定：ON
						{
							s_MioSendWebDAV.bEnableProxy = TRUE;
							s_MioSendWebDAV.pProxyInfo = &s_sMioProxyInfoWebDAV;
							Bool ProxySVR_LinkWebDAVClient = False;
							pc_RoimWscDataIF->getProxySVR_LinkWebDAVClient(&ProxySVR_LinkWebDAVClient);
							
							// 「WebDAV連動」の設定がONの場合は、管理者管理者設定＞ネットワーク＞WebDAV設定＞WebDAVクライアント設定の配下のプロキシサーバーの設定値からアドレス、ポート番号を取得してサーバーを指定する
							if(ProxySVR_LinkWebDAVClient == True)
							{
								unsigned char uc_ProxyServerURL[TYPD_RoimWscDataIF_ProxyServer_URLMaxLen] = {0,};
								unsigned char uc_ProxyServerUserName[TYPD_RoimWscDataIF_ProxyServer_UserNameMaxLen] = {0,};
								unsigned char uc_ProxyServerPassword[TYPD_RoimWscDataIF_ProxyServer_PasswordMaxLen] = {0,};
								unsigned short us_ProxyPortNum = 0;

								pc_WebDavSetting->getProxyAddress(uc_ProxyServerURL);
								us_ProxyPortNum = pc_WebDavSetting->getProxyPortNum();
								pc_WebDavSetting->getProxyUserName(uc_ProxyServerUserName);
								pc_WebDavSetting->getProxyPassword(uc_ProxyServerPassword);

								memcpy(s_MioSendWebDAV.pProxyInfo->ucProxyAddr, uc_ProxyServerURL, TYPD_RoimWscDataIF_ProxyServer_URLMaxLen);
								s_MioSendWebDAV.pProxyInfo->usProxyPortNum = us_ProxyPortNum;
								memcpy(s_MioSendWebDAV.pProxyInfo->ucProxyUser, uc_ProxyServerUserName, TYPD_RoimWscDataIF_ProxyServer_UserNameMaxLen);
								memcpy(s_MioSendWebDAV.pProxyInfo->ucProxyPass, uc_ProxyServerPassword, TYPD_RoimWscDataIF_ProxyServer_PasswordMaxLen);
							}
							else // 「WebDAV連動」の設定がOFFの場合は、サービスの該当設定の配下の設定値からアドレス、ポート番号を取得してサーバーを指定する
							{
								char sc_ProxyServerURL[TYPD_RoimWscDataIF_ProxyServer_URLMaxLen] = {0,};
								char sc_ProxyServerUserName[TYPD_RoimWscDataIF_ProxyServer_UserNameMaxLen] = {0,};
								char sc_ProxyServerPassword[TYPD_RoimWscDataIF_ProxyServer_PasswordMaxLen] = {0,};
								unsigned short us_ProxyPortNum = 0;

								pc_RoimWscDataIF->getProxySVR_URL( sc_ProxyServerURL );
								pc_RoimWscDataIF->getProxySVR_PortNumber(&us_ProxyPortNum);
								pc_RoimWscDataIF->getProxySVR_UserName( sc_ProxyServerUserName );
								pc_RoimWscDataIF->getProxySVR_Password( sc_ProxyServerPassword );

								// Set URL for proxy server
							//2023/04/12 FPT-VietDQ11 OP_BTS-42640 Add Start.
							if((strncmp(sc_ProxyServerURL, STRING_DEFAULT_URL, (size_t)TYPD_RoimWscDataIF_ProxyServer_URLMaxLen) != 0) && (strncmp((sc_ProxyServerURL), "",(size_t)TYPD_RoimWscDataIF_ProxyServer_URLMaxLen) != 0))
							{
								memcpy(reinterpret_cast<char*>(s_MioSendWebDAV.pProxyInfo->ucProxyAddr), sc_ProxyServerURL, TYPD_RoimWscDataIF_ProxyServer_URLMaxLen);
								// Set port for Proxy
								s_MioSendWebDAV.pProxyInfo->usProxyPortNum = us_ProxyPortNum;
								// Set username for proxy server
								memcpy(reinterpret_cast<char*>(s_MioSendWebDAV.pProxyInfo->ucProxyUser), sc_ProxyServerUserName, TYPD_RoimWscDataIF_ProxyServer_UserNameMaxLen);
								// Set password for proxy server
								memcpy(reinterpret_cast<char*>(s_MioSendWebDAV.pProxyInfo->ucProxyPass), sc_ProxyServerPassword, TYPD_RoimWscDataIF_ProxyServer_PasswordMaxLen);
							}
							else
							{
								s_MioSendWebDAV.bEnableProxy = FALSE;
								s_MioSendWebDAV.pProxyInfo = NULL;
							}
							//2023/04/12 FPT-VietDQ11 OP_BTS-42640 Add End.
							}
						}
						MioFileDesc fd = mioOpen("/mio/sw/14", O_WRONLY);
						// STATUS ret_error = mioCheckConnection(fd, SEND_TYPE_WEBDAV, &s_MioSendWebDAV);
						STATUS ret_error = APIC_WebDAVTx::newInstance()->checkConnection(fd, &s_MioSendWebDAV);
						mioClose(fd);
						if(ret_error == OK)
						{
							p_res->s_Network.s_NetworkConfirm.s_WWRSPFserver.e_Status = OAPE_NetworkStatus_OK;
						}
						else
						{
							p_res->s_Network.s_NetworkConfirm.s_WWRSPFserver.e_Status = OAPE_NetworkStatus_NG;
							if(ret_error == 408)
							{
								p_res->s_Network.s_NetworkConfirm.s_WWRSPFserver.e_ErrorCode = OAPE_NetworkErrorCode_Connection_timeout;
							}
							else
							{
								p_res->s_Network.s_NetworkConfirm.s_WWRSPFserver.e_ErrorCode = OAPE_NetworkErrorCode_Connection_refuse;
							}
							
						}
					}
					else
					{
						p_res->s_Network.s_NetworkConfirm.s_WWRSPFserver.e_Status = OAPE_NetworkStatus_NG;
						p_res->s_Network.s_NetworkConfirm.s_WWRSPFserver.e_ErrorCode = OAPE_NetworkErrorCode_Connection_refuse;
					}
				}
				//2023/03/08 [IT6_2.3][制御構成変更_No.52-1][IT6_23-1576]FPT-VanPN2 Add End.
				break;
			// [IT6_2.3][DEVALL_OP_3753] FPT Add Start.
			case OAPE_NetworkTypeMode_XMPPserver:
				{
					TYPE_RoimWscDataIF_XMPPSessionStatus e_XMPPSessionStatus = TYPE_RoimWscDataIF_XMPPSessionStatus_Disconnect;		// 接続状態.
					char sc_ErrorCode[TYPD_RoimWscDataIF_XMPPSessionDetailMaxLen];													// 接続結果(ｴﾗｰｺｰﾄﾞ).
					memset( sc_ErrorCode, 0x00, sizeof( sc_ErrorCode ) );

					pc_RoimWscDataIF->getSessionStatus( &e_XMPPSessionStatus, sc_ErrorCode);

					getOpenAPIEnum(e_XMPPSessionStatus,p_res->s_Network.s_NetworkConfirm.s_XMPPserver.e_Status);
					p_res->s_Network.s_NetworkConfirm.s_XMPPserver.sc_ErrorCode = sc_ErrorCode;
				}
				break;
			// [IT6_2.3][DEVALL_OP_3753] FPT Add End.
			default:
				break;
		}
	}
//2022/11/22 [IT6_2.3][制御構成変更_No.52]FPT-HoaPH Add Ended.
	// OP_BTS-5966 end.
//2022/11/24 [IT6_2.3][制御構成変更_No.52]FPT-HoaPH Add Started.
	if(True==pc_NetworkDevice->isNetworkInitialing())
	{
		p_res->s_Network.e_ResetNetworkStatus=OAPE_ResetNetworkStatus_Initializing;
	}else
	{
		p_res->s_Network.e_ResetNetworkStatus=(True==pc_NetworkDevice->getNetworkInitializationStatus())?OAPE_ResetNetworkStatus_Success:OAPE_ResetNetworkStatus_Failure;
	}
//2022/11/24 [IT6_2.3][制御構成変更_No.52]FPT-HoaPH Add Ended.
	
	
	return true;
}
// ZeusS_ZX0 RQ-320対応 2015/7/15 1OPS Ohno End. 
// IT5_3.3 制御構成変更_No.16 HCL-Sahana 2016/08/02 Added Start.
// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=NetworkTuning).
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_NetworkTuning>::OAPC_UtilityItemImp()
{
	activate();
}
template<> OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_NetworkTuning>::~OAPC_UtilityItemImp()
{}

// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=NetworkTuning) pre_check.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：.
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_NetworkTuning>::pre_check(const OAPS_AppReqSetUtilitySetting* p_req, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=NetworkTuning)::pre_check()" );
	
	return true;
}

// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem = NetworkTuning) set.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：Sahana
// レビュー：.
// 変更履歴：.
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_NetworkTuning>::set(const OAPS_AppReqSetUtilitySetting* p_req, OAPC_SyncLock& _slock, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=Network)::set()" );

	e.setMessage( OAPE_Result_FaultRequest_AppReqSetUtilitySetting );
	
	OAPC_ApiLock<APIC_TcpIpSetting> pc_TcpIpSetting( APIC_TcpIpSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_IppSetting> pc_IppSetting( APIC_IppSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_IPv6Setting> pc_IPv6Setting( APIC_IPv6Setting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_PrinterPortSetting> pc_PrinterPortSetting( APIC_PrinterPortSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_SmbPrintSetting> pc_SmbPrintSetting( APIC_SmbPrintSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_SmbBrowsing> pc_SmbBrowsing( APIC_SmbBrowsing::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_WebDavSetting> pc_WebDavSetting( APIC_WebDavSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_PswcMibSetting> pc_PswcMibSetting( APIC_PswcMibSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_UserAuthenticationSetting> pc_UserAuthenticationSetting( APIC_UserAuthenticationSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_MailSendSetting> pc_MailSendSetting( APIC_MailSendSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_UbiquitousSetting> pc_UbiquitousSetting( APIC_UbiquitousSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_SoftDipSw> 			pc_SoftDipSw( APIC_SoftDipSw::newInstanceAdmin(), &_slock ); 
	
	if ( !pc_TcpIpSetting 
		|| !pc_IppSetting
		|| !pc_IPv6Setting
		|| !pc_PrinterPortSetting
		|| !pc_WebDavSetting
		|| !pc_PswcMibSetting
		|| !pc_UserAuthenticationSetting
		|| !pc_MailSendSetting
		|| !pc_UbiquitousSetting
		|| !pc_SoftDipSw)
	{
		e.setKind( OAPE_Fault_faultcode_Server );
		e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralAllocateBufferFull, "Failed to get instance" );
		return false;
	}
	// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Added Start.
	TYPE_NetworkSpecific eNetworkSpecific = TYPE_NetworkSpecific_NW1;
		if((!p_req->e_NetworkType.empty()) && (OAPE_NetworkType_NW2 == p_req->e_NetworkType.get())){
			if(true == checkVLANSetting(e)){
				getCommAPIEnum( p_req->e_NetworkType, eNetworkSpecific );
				}
			else{
				e.setKind(OAPE_Fault_faultcode_Client);
				e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralIllegalValue, "VLAN setting is disabled");
				return false;
			}
		}
	// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Added End.
	// Exp-Imp特殊ユーザーのみ受付. 
	if( True == checkExpImpAccountCheck( p_req->s_OperatorInfo.sc_AuthKey.get() ) )
	{
		// DdnsUpdateTimer
		if(!p_req->s_NetworkTuning.ul_DdnsUpdateTimer.empty()) {
			uint32_t ul_DdnsUpdateTimer1 = p_req->s_NetworkTuning.ul_DdnsUpdateTimer.get();
			pc_TcpIpSetting->setDDNS_UpdateTimer( ul_DdnsUpdateTimer1, eNetworkSpecific );// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		}
		// DhcpRetryTime
		if(( True == pc_SoftDipSw->getSoftDipSwBit(88, 6) ) && (!p_req->s_NetworkTuning.ul_DhcpRetryTime.empty())) {
				uint32_t ul_DhcpRetryTime1 = p_req->s_NetworkTuning.ul_DhcpRetryTime.get();
				pc_TcpIpSetting->setDHCP_RetryTime( ul_DhcpRetryTime1, eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		}
		//IpAddress - ArpRetransTimeWired
		if(!p_req->s_NetworkTuning.s_IpAddress.ul_ArpRetransTimeWired.empty()) {
			uint32_t ul_ArpRetransTimeWired1 = p_req->s_NetworkTuning.s_IpAddress.ul_ArpRetransTimeWired.get();
			pc_TcpIpSetting->setARP_RetransTime_wired( ul_ArpRetransTimeWired1 );
		}
		//IpAddress - ArpRetransTimeWireless
		if(!p_req->s_NetworkTuning.s_IpAddress.ul_ArpRetransTimeWireless.empty()) {
			uint32_t ul_ArpRetransTimeWireless1 = p_req->s_NetworkTuning.s_IpAddress.ul_ArpRetransTimeWireless.get();
			pc_TcpIpSetting->setARP_RetransTime_wireless( ul_ArpRetransTimeWireless1 );
		}
		//IpAddress - ArpRetransTimeLo
		if(!p_req->s_NetworkTuning.s_IpAddress.ul_ArpRetransTimeLo.empty()) {
			uint32_t ul_ArpRetransTimeLo1 = p_req->s_NetworkTuning.s_IpAddress.ul_ArpRetransTimeLo.get();
			pc_TcpIpSetting->setARP_RetransTime_lo( ul_ArpRetransTimeLo1 );
		}
		//IppPrint
		if(!p_req->s_NetworkTuning.ul_IppPrint.empty()) {
			uint32_t ul_IppPrint1 = p_req->s_NetworkTuning.ul_IppPrint.get();
			pc_IppSetting->setIPP_KeepAlive( ul_IppPrint1 );
		}
		//Ipv6Settings - Ipv6DadTransmitsWired
		if(!p_req->s_NetworkTuning.s_Ipv6Settings.ul_Ipv6DadTransmitsWired.empty()) {
			uint32_t ul_Ipv6DadTransmitsWired1 = p_req->s_NetworkTuning.s_Ipv6Settings.ul_Ipv6DadTransmitsWired.get();
			pc_IPv6Setting->setIPV6_DADTransmits_wired( ul_Ipv6DadTransmitsWired1 );
		}
		//Ipv6Settings - Ipv6DadTransmitsWireless
		if(!p_req->s_NetworkTuning.s_Ipv6Settings.ul_Ipv6DadTransmitsWireless.empty()) {
			uint32_t ul_Ipv6DadTransmitsWireless1 = p_req->s_NetworkTuning.s_Ipv6Settings.ul_Ipv6DadTransmitsWireless.get();
			pc_IPv6Setting->setIPV6_DADTransmits_wireless( ul_Ipv6DadTransmitsWireless1 );
		}
		//Ipv6Settings - Ipv6DadTransmitsLo
		if(!p_req->s_NetworkTuning.s_Ipv6Settings.ul_Ipv6DadTransmitsLo.empty()) {
			uint32_t ul_Ipv6DadTransmitsLo1 = p_req->s_NetworkTuning.s_Ipv6Settings.ul_Ipv6DadTransmitsLo.get();
			pc_IPv6Setting->setIPV6_DADTransmits_lo( ul_Ipv6DadTransmitsLo1 );
		}
		//LpdBacklog
		if(!p_req->s_NetworkTuning.uc_LpdBacklog.empty()) {
			uchar uc_LpdBacklog1 = p_req->s_NetworkTuning.uc_LpdBacklog.get();
			pc_PrinterPortSetting->setLPD_Backlog( uc_LpdBacklog1, eNetworkSpecific );// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		}
		//MtuDiscovery
		if(!p_req->s_NetworkTuning.ul_MtuDiscovery.empty()) {
			uint32_t ul_MtuDiscovery1 = p_req->s_NetworkTuning.ul_MtuDiscovery.get();
			pc_TcpIpSetting->setIP_PMTU_Min( ul_MtuDiscovery1 );
		}
		//PMtuDiscovery
		if(!p_req->s_NetworkTuning.ul_PMtuDiscovery.empty()) {
			uint32_t ul_PMtuDiscovery1 = p_req->s_NetworkTuning.ul_PMtuDiscovery.get();
			pc_TcpIpSetting->setIP_PMTUDiscover( ul_PMtuDiscovery1 );
		}
		//【Eagle】RQ-475 MTUネットワーク接続の対応 HCL-Garima 2017/11/15 Start.
		//FMtuDiscovery
		if(!p_req->s_NetworkTuning.ul_FMtuDiscovery.empty()) {
		 uint32_t ul_FMtuDiscovery1 = p_req->s_NetworkTuning.ul_FMtuDiscovery.get();
		if(True == pc_IPv6Setting->getIPv6Address() && (ul_FMtuDiscovery1 < 1280 ))//【Eagle】AR.1047045 HCL-Garima 2018/06/01 Mod Start.
		{
			//【Eagle】AR.1047045 HCL-Garima 2018/06/01 Start.
			// No SET operation
           // IPｖ6有効時に。MTU値を1280未満に変更不可。
		}else {
		      pc_TcpIpSetting->setIP_PMTU_Fixed( ul_FMtuDiscovery1 );
           }
        }
		//【Eagle】AR.1047045 HCL-Garima 2018/06/01 Mod End.
		//【Eagle】RQ-475 MTUネットワーク接続の対応 HCL-Garima 2017/11/15 End.
		//Raw1Backlog
		if(!p_req->s_NetworkTuning.s_RawPrint.uc_Raw1Backlog.empty()) {
			uchar uc_Raw1Backlog1 = p_req->s_NetworkTuning.s_RawPrint.uc_Raw1Backlog.get();
			pc_PrinterPortSetting->setRaw1_Backlog( uc_Raw1Backlog1 ,eNetworkSpecific );// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		}
		//Raw2Backlog
		if(!p_req->s_NetworkTuning.s_RawPrint.uc_Raw2Backlog.empty()) {
			uchar uc_Raw2Backlog2 = p_req->s_NetworkTuning.s_RawPrint.uc_Raw2Backlog.get();
			pc_PrinterPortSetting->setRaw2_Backlog( uc_Raw2Backlog2 );
		}
		//Raw3Backlog
		if(!p_req->s_NetworkTuning.s_RawPrint.uc_Raw3Backlog.empty()) {
			uchar uc_Raw3Backlog3 = p_req->s_NetworkTuning.s_RawPrint.uc_Raw3Backlog.get();
			pc_PrinterPortSetting->setRaw3_Backlog( uc_Raw3Backlog3 );
		}		
		//Raw4Backlog
		if(!p_req->s_NetworkTuning.s_RawPrint.uc_Raw4Backlog.empty()) {
			uchar uc_Raw4Backlog4 = p_req->s_NetworkTuning.s_RawPrint.uc_Raw4Backlog.get();
			pc_PrinterPortSetting->setRaw4_Backlog( uc_Raw4Backlog4 );
		}
		//Raw5Backlog
		if(!p_req->s_NetworkTuning.s_RawPrint.uc_Raw5Backlog.empty()) {
			uchar uc_Raw5Backlog5 = p_req->s_NetworkTuning.s_RawPrint.uc_Raw5Backlog.get();
			pc_PrinterPortSetting->setRaw5_Backlog( uc_Raw5Backlog5 );
		}
		//Raw6Backlog
		if(!p_req->s_NetworkTuning.s_RawPrint.uc_Raw6Backlog.empty()) {
			uchar uc_Raw6Backlog6 = p_req->s_NetworkTuning.s_RawPrint.uc_Raw6Backlog.get();
			pc_PrinterPortSetting->setRaw6_Backlog( uc_Raw6Backlog6 );
		}
		//SmbTransmission-SmbFindFolderTimeout
		if(!p_req->s_NetworkTuning.s_SmbTransmission.us_SmbFindFolderTimeout.empty()) {
			ushort us_SmbFindFolderTimeout1 = p_req->s_NetworkTuning.s_SmbTransmission.us_SmbFindFolderTimeout.get();
			pc_SmbBrowsing->setSmbBrowwsingFindFlderTimeout( us_SmbFindFolderTimeout1,eNetworkSpecific );// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		}
		//SmbTransmission-SmbSharedRsclistTimeout
		if(!p_req->s_NetworkTuning.s_SmbTransmission.us_SmbSharedRsclistTimeout.empty()) {
			ushort us_SmbSharedRsclistTimeout1 = p_req->s_NetworkTuning.s_SmbTransmission.us_SmbSharedRsclistTimeout.get();
			pc_SmbBrowsing->setSmbBrowwsingSharedrsclistTimeout( us_SmbSharedRsclistTimeout1,eNetworkSpecific );// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		}
		//SmbTransmission-us_SmbcCreateTimeout
		if(!p_req->s_NetworkTuning.s_SmbTransmission.us_SmbcCreateTimeout.empty()) {
			ushort us_SmbcCreateTimeout1 = p_req->s_NetworkTuning.s_SmbTransmission.us_SmbcCreateTimeout.get();
			pc_SmbPrintSetting->setSmbcCreateTimeout( us_SmbcCreateTimeout1 ,eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		}
		//SmbTransmission-us_SmbcWriteTimeout
		if(!p_req->s_NetworkTuning.s_SmbTransmission.us_SmbcWriteTimeout.empty()) {
			ushort us_SmbcWriteTimeout1 = p_req->s_NetworkTuning.s_SmbTransmission.us_SmbcWriteTimeout.get();
			pc_SmbPrintSetting->setSmbcWriteTimeout( us_SmbcWriteTimeout1 ,eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		}
		//SmbTransmission-us_SmbcDeleteTimeout
		if(!p_req->s_NetworkTuning.s_SmbTransmission.us_SmbcDeleteTimeout.empty()) {
			ushort us_SmbcDeleteTimeout1 = p_req->s_NetworkTuning.s_SmbTransmission.us_SmbcDeleteTimeout.get();
			pc_SmbPrintSetting->setSmbcDeleteTimeout( us_SmbcDeleteTimeout1 ,eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		}
		//SmbTransmission-us_SmbcConnectTimeout
		if(!p_req->s_NetworkTuning.s_SmbTransmission.us_SmbcConnectTimeout.empty()) {
			ushort us_SmbcConnectTimeout1 = p_req->s_NetworkTuning.s_SmbTransmission.us_SmbcConnectTimeout.get();
			pc_SmbPrintSetting->setSmbcConnectTimeout( us_SmbcConnectTimeout1 ,eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		}
		//SmbTransmission-us_LmnrRespTimeout
		if(!p_req->s_NetworkTuning.s_SmbTransmission.us_LmnrRespTimeout.empty()) {
			ushort us_LmnrRespTimeout1 = p_req->s_NetworkTuning.s_SmbTransmission.us_LmnrRespTimeout.get();
			pc_SmbPrintSetting->setLMNR_RespTimeout( us_LmnrRespTimeout1 );
		}
		//SmbTransmission-us_NbtNamelookupTimeout
		if(!p_req->s_NetworkTuning.s_SmbTransmission.us_NbtNamelookupTimeout.empty()) {
			ushort us_NbtNamelookupTimeout1 = p_req->s_NetworkTuning.s_SmbTransmission.us_NbtNamelookupTimeout.get();
			pc_SmbPrintSetting->setNBT_NamelookupTimeout( us_NbtNamelookupTimeout1 );
		}
		//SmbTransmission-us_NbtNamelookupRetryCnt
		if(!p_req->s_NetworkTuning.s_SmbTransmission.us_NbtNamelookupRetryCnt.empty()) {
			ushort us_NbtNamelookupRetryCnt1 = p_req->s_NetworkTuning.s_SmbTransmission.us_NbtNamelookupRetryCnt.get();
			pc_SmbPrintSetting->setNBT_NamelookupRetryCnt( us_NbtNamelookupRetryCnt1 );
		}
		//SmbTransmission-us_WinsNamelookupTimeout
		if(!p_req->s_NetworkTuning.s_SmbTransmission.us_WinsNamelookupTimeout.empty()) {
			ushort us_WinsNamelookupTimeout1 = p_req->s_NetworkTuning.s_SmbTransmission.us_WinsNamelookupTimeout.get();
			pc_SmbPrintSetting->setWINS_NamelookupTimeout( us_WinsNamelookupTimeout1 );
		}
		//TCPSynCookieOnOff
		Bool b_TCP_SYNcookie_OnOff = True;
		b_TCP_SYNcookie_OnOff = (p_req->s_NetworkTuning.e_TCPSynCookieOnOff == OAPE_Bool_true ? True : False);
		pc_TcpIpSetting->setTCP_SYNcookie_OnOff( b_TCP_SYNcookie_OnOff );
		//TcpSetting - TcpRMemBufSize
		if(!p_req->s_NetworkTuning.s_TcpSetting.s_TcpRMemBufSize.empty()) {
			TYPS_TCP_Rmem_BufSize ps_TCP_Rmem_BufSize;
			::memset( &ps_TCP_Rmem_BufSize, 0, sizeof(ps_TCP_Rmem_BufSize) );

			if(!p_req->s_NetworkTuning.s_TcpSetting.s_TcpRMemBufSize.ul_TcpRmemBufSizeMin.empty()) {
				ps_TCP_Rmem_BufSize.ul_TCP_Rmem_Min = p_req->s_NetworkTuning.s_TcpSetting.s_TcpRMemBufSize.ul_TcpRmemBufSizeMin;
			}
			if(!p_req->s_NetworkTuning.s_TcpSetting.s_TcpRMemBufSize.ul_TcpRmemBufSizeDef.empty()) {
				ps_TCP_Rmem_BufSize.ul_TCP_Rmem_Default = p_req->s_NetworkTuning.s_TcpSetting.s_TcpRMemBufSize.ul_TcpRmemBufSizeDef;
			}
			if(!p_req->s_NetworkTuning.s_TcpSetting.s_TcpRMemBufSize.ul_TcpRmemBufSizeMax.empty()) {
				ps_TCP_Rmem_BufSize.ul_TCP_Rmem_Max = p_req->s_NetworkTuning.s_TcpSetting.s_TcpRMemBufSize.ul_TcpRmemBufSizeMax;
			}
			pc_TcpIpSetting->setTCP_Rmem( &ps_TCP_Rmem_BufSize );
		}
		//TcpSetting - ul_TcpRMemDefault
		if(!p_req->s_NetworkTuning.s_TcpSetting.ul_TcpRMemDefault.empty()) {
			uint32_t ul_TcpRMemDefault1 = p_req->s_NetworkTuning.s_TcpSetting.ul_TcpRMemDefault.get();
			pc_TcpIpSetting->setSOCK_Rmem_Default( ul_TcpRMemDefault1 );
		}
		//TcpSetting - TcpRMemMax
		if(!p_req->s_NetworkTuning.s_TcpSetting.ul_TcpRMemMax.empty()) {
			uint32_t ul_TcpRMemMax1 = p_req->s_NetworkTuning.s_TcpSetting.ul_TcpRMemMax.get();
			pc_TcpIpSetting->setSOCK_Rmem_Max( ul_TcpRMemMax1 );
		}
		//TcpSetting - e_TcpSack

		Bool b_TCP_SACK = True;
		b_TCP_SACK = (p_req->s_NetworkTuning.s_TcpSetting.e_TcpSack == OAPE_Bool_true ? True : False);
		pc_TcpIpSetting->setTCP_SACK( b_TCP_SACK );
		//TcpSetting - TcpWMemSize
		if(!p_req->s_NetworkTuning.s_TcpSetting.s_TcpWMemSize.empty()) {
			TYPS_TCP_Wmem_BufSize ps_TCP_Wmem_BufSize;
			::memset( &ps_TCP_Wmem_BufSize, 0, sizeof(ps_TCP_Wmem_BufSize) );
			
			if(!p_req->s_NetworkTuning.s_TcpSetting.s_TcpWMemSize.ul_TcpWMemSizeMin.empty()) {
				ps_TCP_Wmem_BufSize.ul_TCP_Wmem_Min = p_req->s_NetworkTuning.s_TcpSetting.s_TcpWMemSize.ul_TcpWMemSizeMin;
			}
			if(!p_req->s_NetworkTuning.s_TcpSetting.s_TcpWMemSize.ul_TcpWMemSizeDef.empty()) {
				ps_TCP_Wmem_BufSize.ul_TCP_Wmem_Default = p_req->s_NetworkTuning.s_TcpSetting.s_TcpWMemSize.ul_TcpWMemSizeDef;
			}
			if(!p_req->s_NetworkTuning.s_TcpSetting.s_TcpWMemSize.ul_TcpWMemSizeMax.empty()) {
				ps_TCP_Wmem_BufSize.ul_TCP_Wmem_Max = p_req->s_NetworkTuning.s_TcpSetting.s_TcpWMemSize.ul_TcpWMemSizeMax;
			}
			pc_TcpIpSetting->setTCP_Wmem( &ps_TCP_Wmem_BufSize );
		}
		//TcpSetting - TcpWMemDefault
		if(!p_req->s_NetworkTuning.s_TcpSetting.ul_TcpWMemDefault.empty()) {
			uint32_t ul_TcpWMemDefault1 = p_req->s_NetworkTuning.s_TcpSetting.ul_TcpWMemDefault.get();
			pc_TcpIpSetting->setSOCK_Wmem_Default( ul_TcpWMemDefault1 );
		}
		//TcpSetting - ul_TcpWMemMax
		if(!p_req->s_NetworkTuning.s_TcpSetting.ul_TcpWMemMax.empty()) {
			uint32_t ul_TcpWMemMax1 = p_req->s_NetworkTuning.s_TcpSetting.ul_TcpWMemMax.get();
			pc_TcpIpSetting->setSOCK_Wmem_Max( ul_TcpWMemMax1 );
		}
		//TcpSetting - WebDavd
		if(!p_req->s_NetworkTuning.us_WebDavd.empty()) {
			ushort us_WebDavd1 = p_req->s_NetworkTuning.us_WebDavd.get();
			pc_WebDavSetting->setWevDavd_MSGTimeout( us_WebDavd1 );
		}
		//s_DnsNameResolution-uc_DnsResolvTimeout
		if(!p_req->s_NetworkTuning.s_DnsNameResolution.uc_DnsResolvTimeout.empty()) {
			uchar uc_DnsResolvTimeout1 = p_req->s_NetworkTuning.s_DnsNameResolution.uc_DnsResolvTimeout.get();
			pc_TcpIpSetting->setDNS_ResolvTimeout( uc_DnsResolvTimeout1 ,eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		}		
		//s_DnsNameResolution-uc_DnsResolvTimeout
		if(!p_req->s_NetworkTuning.s_DnsNameResolution.uc_DnsResolvRetryCnt.empty()) {
			uchar uc_DnsResolvRetryCnt1 = p_req->s_NetworkTuning.s_DnsNameResolution.uc_DnsResolvRetryCnt.get();
			pc_TcpIpSetting->setDNS_ResolvRetryCnt( uc_DnsResolvRetryCnt1 ,eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		}
		//s_DirectPrint-ul_HttpdRecBufferSize
		if(!p_req->s_NetworkTuning.s_DirectPrint.ul_HttpdRecBufferSize.empty()) {
			uint32_t ul_HttpdRecBufferSize1 = p_req->s_NetworkTuning.s_DirectPrint.ul_HttpdRecBufferSize.get();
			pc_PswcMibSetting->setHTTPd_RecBufferSize( ul_HttpdRecBufferSize1 );
		}		
		//s_DirectPrint-uc_HttpRecExpandCnt
		if(!p_req->s_NetworkTuning.s_DirectPrint.uc_HttpRecExpandCnt.empty()) {
			uchar uc_HttpRecExpandCnt1 = p_req->s_NetworkTuning.s_DirectPrint.uc_HttpRecExpandCnt.get();
			pc_PswcMibSetting->setHTTPd_RecexpandCount( uc_HttpRecExpandCnt1 );
		}
		//s_KerberosAuth-us_Kbr5Timeout
		if(!p_req->s_NetworkTuning.s_KerberosAuth.us_Kbr5Timeout.empty()) {
			ushort us_Kbr5Timeout1 = p_req->s_NetworkTuning.s_KerberosAuth.us_Kbr5Timeout.get();
			pc_UserAuthenticationSetting->setUserAuth_KBR5Timeout( us_Kbr5Timeout1 ,eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		}
		//s_KerberosAuth-us_PopConRetryCnt
		if(!p_req->s_NetworkTuning.s_KerberosAuth.us_PopConRetryCnt.empty()) {
			ushort us_PopConRetryCnt1 = p_req->s_NetworkTuning.s_KerberosAuth.us_PopConRetryCnt.get();
			pc_MailSendSetting->setPOP_ConRetryCnt( us_PopConRetryCnt1 );
		}		
		//s_KerberosAuth-us_PopConInterval
		if(!p_req->s_NetworkTuning.s_KerberosAuth.us_PopConInterval.empty()) {
			ushort us_PopConInterval1 = p_req->s_NetworkTuning.s_KerberosAuth.us_PopConInterval.get();
			pc_MailSendSetting->setPOP_ConInterval( us_PopConInterval1 );
		}		
		//s_KerberosAuth-ul_SmtpSendSize
		if(!p_req->s_NetworkTuning.s_KerberosAuth.ul_SmtpSendSize.empty()) {
			uint32_t ul_SmtpSendSize1 = p_req->s_NetworkTuning.s_KerberosAuth.ul_SmtpSendSize.get();
			pc_MailSendSetting->setSMTP_SendSize( ul_SmtpSendSize1 );
		}
		//s_Ubiquitous-ul_UbiqKeepAliveTimeout
		if(!p_req->s_NetworkTuning.s_Ubiquitous.ul_UbiqKeepAliveTimeout.empty()) {
			uint32_t ul_UbiqKeepAliveTimeout1 = p_req->s_NetworkTuning.s_Ubiquitous.ul_UbiqKeepAliveTimeout.get();
			pc_UbiquitousSetting->setMFPIF_KeepALiveTime( ul_UbiqKeepAliveTimeout1 );
		}		
		//s_Ubiquitous-us_UbiqServerTimeout
		if(!p_req->s_NetworkTuning.s_Ubiquitous.us_UbiqServerTimeout.empty()) {
			ushort us_UbiqServerTimeout1 = p_req->s_NetworkTuning.s_Ubiquitous.us_UbiqServerTimeout.get();
			pc_UbiquitousSetting->setMFPIF_ServerTimeout( us_UbiqServerTimeout1 );
		}		
		//s_Ubiquitous-us_UbiqMsgTimeout
		if(!p_req->s_NetworkTuning.s_Ubiquitous.us_UbiqMsgTimeout.empty()) {
			ushort us_UbiqMsgTimeout1 = p_req->s_NetworkTuning.s_Ubiquitous.us_UbiqMsgTimeout.get();
			pc_UbiquitousSetting->setMFPIF_MSGTimeout( us_UbiqMsgTimeout1 );
		}		
		//s_Ubiquitous-us_UbiqPrintSendTimeout
		if(!p_req->s_NetworkTuning.s_Ubiquitous.us_UbiqPrintSendTimeout.empty()) {
			ushort us_UbiqPrintSendTimeout1 = p_req->s_NetworkTuning.s_Ubiquitous.us_UbiqPrintSendTimeout.get();
			pc_UbiquitousSetting->setPrintSendTimeout( us_UbiqPrintSendTimeout1 );
		}		
		//s_Ubiquitous-us_UbiqPrintSentRetry
		if(!p_req->s_NetworkTuning.s_Ubiquitous.us_UbiqPrintSentRetry.empty()) {
			ushort us_UbiqPrintSentRetry1 = p_req->s_NetworkTuning.s_Ubiquitous.us_UbiqPrintSentRetry.get();
			pc_UbiquitousSetting->setPrintSendRetry( us_UbiqPrintSentRetry1 );
		}		
		
		//IT5_4.2 【FQ-OPSD-1700369-OFP190】AR1037808 2018/02/15 HCL-Jerin Added Start.
		//DhcpDelayTime
		if(!p_req->s_NetworkTuning.us_DhcpDelayTime.empty()) {
			ushort us_DhcpDelayTime1 = p_req->s_NetworkTuning.us_DhcpDelayTime.get();
			pc_TcpIpSetting->setDelayForDHCP( (uint32_t) us_DhcpDelayTime1 );
		}
		
		//NetworkIFLinkupTimeOut
		Bool b_NetworkIFLinkupTimeOut = True;
		b_NetworkIFLinkupTimeOut = (p_req->s_NetworkTuning.e_NetworkIFLinkupTimeOut == OAPE_Bool_true ? True : False);
		pc_TcpIpSetting->setTimeoutOfNetworkIFLinkup( b_NetworkIFLinkupTimeOut );
		//IT5_4.2 【FQ-OPSD-1700369-OFP190】AR1037808 2018/02/15 HCL-Jerin Added End.
	}

	return true;
}

// =========================================================================== .
// 概要　　：ユーティリティ設定(RequestItem=NetworkTuning) get.
// 戻り値　：bool.
// 備考　　：.
// 作成者　：HCL-Sahana
// レビュー：
// 変更履歴：
// =========================================================================== .
template<> bool OAPC_UtilityItemImp<OAPE_UtilitySetting_RequestItem_NetworkTuning>::get(const OAPS_AppReqGetUtilitySetting* p_req, OAPS_AppResGetUtilitySetting* p_res, OAPC_SyncLock& _slock, OAPC_Error& e)
{
	OAPD_TRACE_SCOPE( "AppReqSetUtilitySetting(RequestItem=NetworkTuning)::get()" );

	e.setMessage( OAPE_Result_FaultRequest_AppReqGetUtilitySetting );
	
	OAPC_ApiLock<APIC_TcpIpSetting> pc_TcpIpSetting( APIC_TcpIpSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_IppSetting> pc_IppSetting( APIC_IppSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_IPv6Setting> pc_IPv6Setting( APIC_IPv6Setting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_PrinterPortSetting> pc_PrinterPortSetting( APIC_PrinterPortSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_SmbPrintSetting> pc_SmbPrintSetting( APIC_SmbPrintSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_SmbBrowsing> pc_SmbBrowsing( APIC_SmbBrowsing::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_WebDavSetting> pc_WebDavSetting( APIC_WebDavSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_PswcMibSetting> pc_PswcMibSetting( APIC_PswcMibSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_UserAuthenticationSetting> pc_UserAuthenticationSetting( APIC_UserAuthenticationSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_MailSendSetting> pc_MailSendSetting( APIC_MailSendSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_UbiquitousSetting> pc_UbiquitousSetting( APIC_UbiquitousSetting::newInstanceAdmin(), &_slock );
	OAPC_ApiLock<APIC_SoftDipSw> 			pc_SoftDipSw( APIC_SoftDipSw::newInstanceAdmin(), &_slock ); 
	
	if ( !pc_TcpIpSetting 
		|| !pc_IppSetting
		|| !pc_IPv6Setting
		|| !pc_PrinterPortSetting
		|| !pc_WebDavSetting
		|| !pc_PswcMibSetting
		|| !pc_UserAuthenticationSetting
		|| !pc_MailSendSetting
		|| !pc_UbiquitousSetting
		|| !pc_SoftDipSw)
	{
		e.setKind( OAPE_Fault_faultcode_Server );
		e.setCodeDescription( OAPE_Result_ErrorDetails_GeneralAllocateBufferFull, "Failed to get instance" );
		return false;
	}

	// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Added Start.
	TYPE_NetworkSpecific eNetworkSpecific = TYPE_NetworkSpecific_NW1;
		if((!p_req->e_NetworkType.empty()) && (OAPE_NetworkType_NW2 == p_req->e_NetworkType.get())){
			if(true == checkVLANSetting(e)){
				getCommAPIEnum( p_req->e_NetworkType, eNetworkSpecific );
			}
			else{
				e.setKind(OAPE_Fault_faultcode_Client);
				e.setCodeDescription(OAPE_Result_ErrorDetails_GeneralIllegalValue, "VLAN setting is disabled");
				return false;
			}
		}
	// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Added End.
	// Exp-Imp特殊ユーザーのみ受付. 
	if( True == checkExpImpAccountCheck( p_req->s_OperatorInfo.sc_AuthKey.get() ) 
	|| ((OAPE_UserType_Admin == p_req->s_OperatorInfo.e_UserType) && (OAPD_VERSION_CHECK(p_req->getVersion(), OAPD_OVER_RMMv1)) ) )
	{
		// DdnsUpdateTimer
		p_res->s_NetworkTuning.ul_DdnsUpdateTimer = pc_TcpIpSetting->getDDNS_UpdateTimer(eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		// DhcpRetryTime
		if ( True == pc_SoftDipSw->getSoftDipSwBit(88, 6) ) { 
			p_res->s_NetworkTuning.ul_DhcpRetryTime = pc_TcpIpSetting->getDHCP_RetryTime(eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		}
		//IpAddress - ArpRetransTimeWired
		p_res->s_NetworkTuning.s_IpAddress.ul_ArpRetransTimeWired = pc_TcpIpSetting->getARP_RetransTime_wired(eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		//IpAddress - ArpRetransTimeWireless
		p_res->s_NetworkTuning.s_IpAddress.ul_ArpRetransTimeWireless = pc_TcpIpSetting->getARP_RetransTime_wireless();
		//IpAddress - ArpRetransTimeLo
		p_res->s_NetworkTuning.s_IpAddress.ul_ArpRetransTimeLo = pc_TcpIpSetting->getARP_RetransTime_lo();
		//IppPrint
		p_res->s_NetworkTuning.ul_IppPrint = pc_IppSetting->getIPP_KeepAlive();
		//Ipv6Settings - Ipv6DadTransmitsWired
		p_res->s_NetworkTuning.s_Ipv6Settings.ul_Ipv6DadTransmitsWired = pc_IPv6Setting->getIPV6_DADTransmits_wired();
		//Ipv6Settings - Ipv6DadTransmitsWireless
		p_res->s_NetworkTuning.s_Ipv6Settings.ul_Ipv6DadTransmitsWireless = pc_IPv6Setting->getIPV6_DADTransmits_wireless();
		//Ipv6Settings - Ipv6DadTransmitsWireless
		p_res->s_NetworkTuning.s_Ipv6Settings.ul_Ipv6DadTransmitsLo = pc_IPv6Setting->getIPV6_DADTransmits_lo();
		//LpdBacklog
		p_res->s_NetworkTuning.uc_LpdBacklog = pc_PrinterPortSetting->getLPD_Backlog(eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		//MtuDiscovery
		p_res->s_NetworkTuning.ul_MtuDiscovery = pc_TcpIpSetting->getIP_PMTU_Min();
		//PMtuDiscovery
		p_res->s_NetworkTuning.ul_PMtuDiscovery = pc_TcpIpSetting->getIP_PMTUDiscover();
    	//FMtuDiscovery
		//【Eagle】RQ-475 MTUネットワーク接続の対応 HCL-Garima 2017/11/15 Start.
		//HCL-Chitra Modified AR1029580 06/07/2018 Start.
		// uint32_t ul_FMtuDiscovery1 = p_res->s_NetworkTuning.ul_FMtuDiscovery.get();
		//HCL-Chitra Modified AR1029580 06/07/2018 End.
		 //【Eagle】AR.1047045 HCL-Garima 2018/06/01 Mod Start.
	//	if(True == pc_IPv6Setting->getIPv6Address() && (ul_FMtuDiscovery1 >= 1280 ))	//【Eagle】AR.1047045 HCL-Garima 2018/06/01 Start.
	//		{
				p_res->s_NetworkTuning.ul_FMtuDiscovery = pc_TcpIpSetting->getIP_PMTU_Fixed();
	//		}	//【Eagle】AR.1047045 HCL-Garima 2018/06/01 Mod End.
    	//【Eagle】RQ-475 MTUネットワーク接続の対応 HCL-Garima 2017/11/15 End.
		//Raw1Backlog
		p_res->s_NetworkTuning.s_RawPrint.uc_Raw1Backlog = pc_PrinterPortSetting->getRaw1_Backlog(eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		//Raw2Backlog
		p_res->s_NetworkTuning.s_RawPrint.uc_Raw2Backlog = pc_PrinterPortSetting->getRaw2_Backlog();
		//Raw3Backlog
		p_res->s_NetworkTuning.s_RawPrint.uc_Raw3Backlog = pc_PrinterPortSetting->getRaw3_Backlog();
		//Raw4Backlog
		p_res->s_NetworkTuning.s_RawPrint.uc_Raw4Backlog = pc_PrinterPortSetting->getRaw4_Backlog();
		//Raw5Backlog
		p_res->s_NetworkTuning.s_RawPrint.uc_Raw5Backlog = pc_PrinterPortSetting->getRaw5_Backlog();
		//Raw6Backlog
		p_res->s_NetworkTuning.s_RawPrint.uc_Raw6Backlog = pc_PrinterPortSetting->getRaw6_Backlog();
		//SmbTransmission-SmbFindFolderTimeout
		p_res->s_NetworkTuning.s_SmbTransmission.us_SmbFindFolderTimeout = pc_SmbBrowsing->getSmbBrowwsingFindFlderTimeout(eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		//SmbTransmission-SmbSharedRsclistTimeout
		p_res->s_NetworkTuning.s_SmbTransmission.us_SmbSharedRsclistTimeout = pc_SmbBrowsing->getSmbBrowwsingSharedrsclistTimeout(eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		//SmbTransmission-us_SmbcCreateTimeout
		p_res->s_NetworkTuning.s_SmbTransmission.us_SmbcCreateTimeout = pc_SmbPrintSetting->getSmbcCreateTimeout(eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		//SmbTransmission-us_SmbcWriteTimeout
		p_res->s_NetworkTuning.s_SmbTransmission.us_SmbcWriteTimeout = pc_SmbPrintSetting->getSmbcWriteTimeout(eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		//SmbTransmission-us_SmbcDeleteTimeout
		p_res->s_NetworkTuning.s_SmbTransmission.us_SmbcDeleteTimeout = pc_SmbPrintSetting->getSmbcDeleteTimeout(eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		//SmbTransmission-us_SmbcConnectTimeout
		p_res->s_NetworkTuning.s_SmbTransmission.us_SmbcConnectTimeout = pc_SmbPrintSetting->getSmbcConnectTimeout(eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		//SmbTransmission-us_LmnrRespTimeout
		p_res->s_NetworkTuning.s_SmbTransmission.us_LmnrRespTimeout = pc_SmbPrintSetting->getLMNR_RespTimeout();
		//SmbTransmission-us_NbtNamelookupTimeout
		p_res->s_NetworkTuning.s_SmbTransmission.us_NbtNamelookupTimeout = pc_SmbPrintSetting->getNBT_NamelookupTimeout();
		//SmbTransmission-us_NbtNamelookupRetryCnt
		p_res->s_NetworkTuning.s_SmbTransmission.us_NbtNamelookupRetryCnt = pc_SmbPrintSetting->getNBT_NamelookupRetryCnt();
		//SmbTransmission-us_WinsNamelookupTimeout
		p_res->s_NetworkTuning.s_SmbTransmission.us_WinsNamelookupTimeout = pc_SmbPrintSetting->getWINS_NamelookupTimeout();
		//TCPSynCookieOnOff
		p_res->s_NetworkTuning.e_TCPSynCookieOnOff = (pc_TcpIpSetting->getTCP_SYNcookie_OnOff() == True ? OAPE_Bool_true : OAPE_Bool_false);
		//TcpSetting - TcpRMemBufSize
		TYPS_TCP_Rmem_BufSize ps_TCP_Rmem_BufSize;
		::memset( &ps_TCP_Rmem_BufSize, 0, sizeof(ps_TCP_Rmem_BufSize) );
			
		pc_TcpIpSetting->getTCP_Rmem( &ps_TCP_Rmem_BufSize );
		p_res->s_NetworkTuning.s_TcpSetting.s_TcpRMemBufSize.ul_TcpRmemBufSizeMin = ps_TCP_Rmem_BufSize.ul_TCP_Rmem_Min;
		p_res->s_NetworkTuning.s_TcpSetting.s_TcpRMemBufSize.ul_TcpRmemBufSizeDef = ps_TCP_Rmem_BufSize.ul_TCP_Rmem_Default;
		p_res->s_NetworkTuning.s_TcpSetting.s_TcpRMemBufSize.ul_TcpRmemBufSizeMax = ps_TCP_Rmem_BufSize.ul_TCP_Rmem_Max;
		//ul_TcpRMemDefault
		p_res->s_NetworkTuning.s_TcpSetting.ul_TcpRMemDefault = pc_TcpIpSetting->getSOCK_Rmem_Default();
		//TcpRMemMax
		p_res->s_NetworkTuning.s_TcpSetting.ul_TcpRMemMax = pc_TcpIpSetting->getSOCK_Rmem_Max();
		//e_TcpSack
		p_res->s_NetworkTuning.s_TcpSetting.e_TcpSack = (pc_TcpIpSetting->getTCP_SACK() == True ? OAPE_Bool_true : OAPE_Bool_false);
		//TcpSetting - TcpWMemSize
		TYPS_TCP_Wmem_BufSize ps_TCP_Wmem_BufSize;
		::memset( &ps_TCP_Wmem_BufSize, 0, sizeof(ps_TCP_Wmem_BufSize) );
			
		pc_TcpIpSetting->getTCP_Wmem( &ps_TCP_Wmem_BufSize );
		p_res->s_NetworkTuning.s_TcpSetting.s_TcpWMemSize.ul_TcpWMemSizeMin = ps_TCP_Wmem_BufSize.ul_TCP_Wmem_Min;
		p_res->s_NetworkTuning.s_TcpSetting.s_TcpWMemSize.ul_TcpWMemSizeDef = ps_TCP_Wmem_BufSize.ul_TCP_Wmem_Default;
		p_res->s_NetworkTuning.s_TcpSetting.s_TcpWMemSize.ul_TcpWMemSizeMax = ps_TCP_Wmem_BufSize.ul_TCP_Wmem_Max;
		//TcpWMemDefault
		p_res->s_NetworkTuning.s_TcpSetting.ul_TcpWMemDefault = pc_TcpIpSetting->getSOCK_Wmem_Default();
		//ul_TcpWMemMax
		p_res->s_NetworkTuning.s_TcpSetting.ul_TcpWMemMax = pc_TcpIpSetting->getSOCK_Wmem_Max();
		//WebDavd
		p_res->s_NetworkTuning.us_WebDavd = pc_WebDavSetting->getWevDavd_MSGTimeout();
		//s_DnsNameResolution-uc_DnsResolvTimeout
		p_res->s_NetworkTuning.s_DnsNameResolution.uc_DnsResolvTimeout = pc_TcpIpSetting->getDNS_ResolvTimeout(eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		//s_DnsNameResolution-uc_DnsResolvTimeout
		p_res->s_NetworkTuning.s_DnsNameResolution.uc_DnsResolvRetryCnt = pc_TcpIpSetting->getDNS_ResolvRetryCnt(eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		//s_DirectPrint-ul_HttpdRecBufferSize
		p_res->s_NetworkTuning.s_DirectPrint.ul_HttpdRecBufferSize = pc_PswcMibSetting->getHTTPd_RecBufferSize();
		//s_DirectPrint-uc_HttpRecExpandCnt
		p_res->s_NetworkTuning.s_DirectPrint.uc_HttpRecExpandCnt = pc_PswcMibSetting->getHTTPd_RecexpandCount();
		//s_KerberosAuth-us_Kbr5Timeout
		p_res->s_NetworkTuning.s_KerberosAuth.us_Kbr5Timeout = pc_UserAuthenticationSetting->getUserAuth_KBR5Timeout(eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		//s_KerberosAuth-us_PopConRetryCnt
		p_res->s_NetworkTuning.s_KerberosAuth.us_PopConRetryCnt = pc_MailSendSetting->getPOP_ConRetryCnt();
		//s_KerberosAuth-us_PopConInterval
		p_res->s_NetworkTuning.s_KerberosAuth.us_PopConInterval = pc_MailSendSetting->getPOP_ConInterval();
		//s_KerberosAuth-ul_SmtpSendSize
		p_res->s_NetworkTuning.s_KerberosAuth.ul_SmtpSendSize = pc_MailSendSetting->getSMTP_SendSize(eNetworkSpecific);// RQ-686対応 IT_3_WP.NO.910 2017/10/09 HCL-Kailash Mod.
		//s_Ubiquitous-ul_UbiqKeepAliveTimeout
		p_res->s_NetworkTuning.s_Ubiquitous.ul_UbiqKeepAliveTimeout = pc_UbiquitousSetting->getMFPIF_KeepALiveTime();
		//s_Ubiquitous-us_UbiqServerTimeout
		p_res->s_NetworkTuning.s_Ubiquitous.us_UbiqServerTimeout = pc_UbiquitousSetting->getMFPIF_ServerTimeout();
		//s_Ubiquitous-us_UbiqMsgTimeout
		p_res->s_NetworkTuning.s_Ubiquitous.us_UbiqMsgTimeout = pc_UbiquitousSetting->getMFPIF_MSGTimeout();
		//s_Ubiquitous-us_UbiqPrintSendTimeout
		p_res->s_NetworkTuning.s_Ubiquitous.us_UbiqPrintSendTimeout = pc_UbiquitousSetting->getPrintSendTimeout();
		//s_Ubiquitous-us_UbiqPrintSentRetry
		p_res->s_NetworkTuning.s_Ubiquitous.us_UbiqPrintSentRetry = pc_UbiquitousSetting->getPrintSendRetry();
		
		//IT5_4.2 【FQ-OPSD-1700369-OFP190】AR1037808 2018/02/15 HCL-Jerin Added Start.
		//DhcpDelayTime
		if(OAPD_VERSION_CHECK(p_req->getVersion(), OAPD_OVER_V613) == true){
			p_res->s_NetworkTuning.us_DhcpDelayTime = pc_TcpIpSetting->getDelayForDHCP();
		}
		
		//NetworkIFLinkupTimeOut
		if(OAPD_VERSION_CHECK(p_req->getVersion(), OAPD_OVER_V613) == true){
			p_res->s_NetworkTuning.e_NetworkIFLinkupTimeOut = (pc_TcpIpSetting->getTimeoutOfNetworkIFLinkup() == True ? OAPE_Bool_true : OAPE_Bool_false);
		}
		//IT5_4.2 【FQ-OPSD-1700369-OFP190】AR1037808 2018/02/15 HCL-Jerin Added End.
		
	}		

	return true;
}
