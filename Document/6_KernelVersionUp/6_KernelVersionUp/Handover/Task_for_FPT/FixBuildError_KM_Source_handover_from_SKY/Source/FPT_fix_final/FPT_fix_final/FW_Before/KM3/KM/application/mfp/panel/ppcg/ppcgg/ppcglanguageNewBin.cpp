//
//【名前】 Language.cpp
//
//【概要】 言語データ取扱関数インプリメントファイル
//
//【履歴】 [新規]  Mon Aug 16 15:51:31 2004 : 津ヶ谷 (ACTY SYSTEM)
//
////////////////////////////////////////////////////////////////////////////////

/*================================================================================================
	インクルード・ファイル
================================================================================================*/
// 外部フォントを使用するときに定義する.
#if !defined(SIMULATOR)
#include "panel/plibh/plibh_CLCD/plibh_Mtui.h"
#endif

#define EXT_FONT

#include <stdio.h>
#include <string.h>

#ifndef DIV_MFP_LIB
#ifndef LINUX
#include "sysFlashrom.h"
#endif	// #ifndef LINUX 
#include <system/thr/thrInclude/THRC_Threadif.h>
#include <system/thr/thrInclude/THRC_BSPif.h>
IMPORT st_FlashManager	st_FRMT[];		/* FlashROM */

#include "ppcglanguage.h"
#include "system/sys/APIC_SystemInfo.h"

#include <panel/pddfont/pddfontg/font.h>

#if defined(SIMULATOR)
#ifndef LINUX	/* VxWorks固有ファイルインクルード対応 */
#include <windows.h>
#endif	/* LINUX */

#include <assert.h>
#endif

#ifdef _DEBUG
#undef THIS_FILE
static char THIS_FILE[]=__FILE__;

#endif


#endif // DIV_MFP_LIB
#include <zlib.h>

#ifndef DIV_MFP_LIB
#ifdef LINUX
#else
#endif	// #ifdef LINUX.
#include <panel/ppcg/ppcgg/ppcglanguageNewBin.h>

#endif // DIV_MFP_LIB
#include <panel/plibh/plibhg/plibh.h>
#ifndef DIV_MFP_LIB
#include <panel/plibh/plibh_CLCD/plibh_CLCD1.h>
#include <panel/pdp/pdp_CLCD/pdpc2_CLCD.h>
#include <system/thr/thrInclude/THRC_MEMFunc.h>		// Linux暫定対応.
#include <panel/plibh/plibh_FUM2/plibh_FUM2.h>

#include "../../../../divlib/Common/blkapi/blkshm.h"
#if defined(DEF_Soc1200) && !defined(FUM_LINUX)
#include "../../../../divlib/client/Proxy_Common/RequestBeforeWarp.h"
#endif //defined(DEF_Soc1200) && !defined(FUM_LINUX).

#include "lib/filelib/SFLG_StorageFileLibraryIf.h"
#include <panel/plibh/plibh_CLCD2/plibh_CLCD2.h>

#include "../../../../divlib/Common/panel/pddtext/pddtext_Interface.h"

#include "../../../../shlib/GUI/gfw/include/GFWC_System.h"

// comment 2015/03/17 OPS1-Sky佐藤 [ZeusS][CQ511108] 言語情報の取得／設定処理見直し.
#include "panel/plibh/plibhg/plibhGuiWrapper.h"
// comment 2016/03/15 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
#include <panel/pnlmngr/pnlmngrg/PNLC_PartsAccessDebugLib.h>

// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更].
#include <panel/plibh/plibh_CLCD2/PNLC_SH3PaneMenuControlLib.h>
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.

/*================================================================================================
	定数・型の宣言
================================================================================================*/
#define PNL_RESULT_ERROR				-1		// エラー.
#define PNL_LANG_USER_ADDRESS			0		// ユーザ言語データ(アドレス)の位置に対応.	
#define PNL_LANG_USER_DATA				1		// ユーザ言語データ(データ長)の位置に対応.	
#define PNL_LANG_SER1_ADDRESS			2		// サービス1言語データ(アドレス)の位置に対応.	
#define PNL_LANG_SER1_USER_DATA			3		// サービス1言語データ(データ長)の位置に対応.	
#define PNL_LANG_SER2_USER_ADDRESS		4		// サービス2言語データ(アドレス)の位置に対応.	
#define PNL_LANG_SER2_USER_DATA			5		// サービス2言語データ(データ長)の位置に対応.	
#define PNL_LANG_BROW_USER_ADDRESS		6		// ブラウザ言語データ(アドレス)の位置に対応.　	
#define PNL_LANG_BROW_DATA				7		// ブラウザ言語データ(データ長)の位置に対応.	
#define PNLD_SELECTLANGFILEPATH_LEN		(64)	// 選択言語ファイルパス長.現状"/res/SelectLangData"だが64Byte分確保する.

// バッファサイズ.
#define PNLD_Size_FontBuf_KS			(114*1024)		// KS font data. 114KB.
#define PNLD_Size_FontBuf_Big5			(482*1024)		// BIG5 font data. 482KB.
#define PNLD_Size_FontBuf_GB18030		(1044*1024)		// BG18030 font data. 1044KB.

// PNLD_Size_FontBuf_Maxの値を変更した場合、下記パスのバッファサイズも修正する事.
// application\divlib\Common\blkapi\blkshm.h	.
// e_allDataFontRAM	のサイズ定義.
#define PNLD_Size_FontBuf_Max			PNLD_Size_FontBuf_GB18030	// 中韓台用バッファの最大サイズに合わせる.

// フォント用領域のID.
#define PNLD_BufId_Big5					(0)
#define PNLD_BufId_GB18030				(1)
#define PNLD_BufId_KS					(2)

#define PNLD_ByteOf1FontImg				(32)	// 1フォントビットマップのデータサイズ.

/*================================================================================================
	列挙型変数のタグ宣言
================================================================================================*/

/*================================================================================================
	マクロ関数の定義
================================================================================================*/
//Comment 2018/05/14 【HCLT-Devipriya】 【IT6_Eagle】 【KRQ-314-4】Adding 4 bytes to the language version offset due to JSON support in Message binary.
//#define OFFSET_LANGVERSION		(8)	// バージョンが格納されているオフセット
#define OFFSET_LANGVERSION		(12)	// バージョンが格納されているオフセット

/*================================================================================================
	構造体のタグ宣言
================================================================================================*/
struct STPanelLanguageTable {
	char* p_Code;			// 言語コード(ISO639-1code)
	TYPE_Language ec_Type;	// 表示言語(APIC_Setting.h に定義)
};

struct STPanelNeedInflateCheckTable {
	TYPE_MachineType e_MachineType;		// 製品タイプ.
	Bool	bl_MessageDeflated;			// メッセージが圧縮されているか.(True:圧縮されている、False:非圧縮)
};

// キーMAP用オフセット情報.
struct PNLS_KeyMapOffsetTable {
	int32_t i32_Addr;
	int32_t i32_Size;
};
struct PNLS_SelectLangData {				// 選択言語情報.
	char sc_LangCode[SIZE_LANGCODE];	// 言語コード.
	uchar uc_MSGBINIndex;				// メッセージバイナリ内の順番.
};
/*================================================================================================.
	ローカルクラス宣言.
================================================================================================*/
class PNLC_SelectLangInfo {	// 選択言語情報管理クラス.
private:
	PNLS_SelectLangData m_st_SelectLangData[PNL_MAX_LANGUAGES];	// 選択言語情報.
	uchar m_uc_SelectLangNum;									// 選択言語数.
	int32_t m_i32_MSGBINLangNum;								// メッセージバイナリ内の言語数.
	void clearData();											// データ初期化.

public:
	PNLC_SelectLangInfo();										// コンストラクタ.
	void createSelectLangInfo();								// 選択言語データ構築.
	uchar getMSGBINIndex( uchar uc_index );						// メッセージバイナリ内の順番取得.
	char* getLangCode( uchar uc_index );							// 言語コード取得.
	uchar getSelectLangNum();									// 選択言語数取得.
	int32_t getMSGBINLangNum();									// メッセージバイナリ内の言語数取得.
};
/*================================================================================================
	モジュール内変数の宣言
================================================================================================*/
static const STPanelLanguageTable stPanelLanguageTable[] = {
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
// comment 2015/06/15 OPS1-Sky入江 [ZeusS_ZX0][RQ-405] パネルUIのヘブライ語対応.
	{"he",  	TYPE_LanguageHebrew},		// ヘブライ語.
// comment 2018/12/14 Sky毛穴 [EagleHigh][RQ-258] タイ語とベトナム語の対応.
	{"vi",  	TYPE_LanguageVietnam},		// ベトナム語.
// comment 2022/11/9 Sky奥田 [RQ-938] バルト三国対応.
	{"et",  	TYPE_LanguageEstonian},		// エストニア語. 
	{"lv",  	TYPE_LanguageLatvian},		// ラトビア語. 
	{"lt",  	TYPE_LanguageLithuanian},	// リトアニア語. 
// comment 2023/2/7 Sky小熊 [IT6_2.3][RQ-970]Welsh as standard language for IT.
	{"cy",  	TYPE_LanguageWelsh},		// ウェールズ語. 
};

struct STPanelLanguageTableOcr {
	char* p_Code;							// 言語コード(ISO639-1code).
	TYPE_LanguageOcr ec_Type;				// 表示言語(typs_cpp.h に定義).
};

static const STPanelLanguageTableOcr stPanelLanguageOcrTable[] = {
	{"ja",  	TYPE_LanguageOcrJapanese		},		// 日本語.
	{"en",  	TYPE_LanguageOcrEnglish			},		// 米語.
	{"de",  	TYPE_LanguageOcrGerman			},		// ドイツ語.
	{"fr",  	TYPE_LanguageOcrFrench			},		// フランス語.
	{"it",  	TYPE_LanguageOcrItalian			},		// イタリア語.
	{"da",  	TYPE_LanguageOcrDanish			},		// デンマーク語.
	{"nl",  	TYPE_LanguageOcrDutch			},		// オランダ゛語.
	{"pt",  	TYPE_LanguageOcrPortuguese		},		// ポルトガル語.
	{"sv",  	TYPE_LanguageOcrSwedish			},		// スウェーデン語.
	{"es",  	TYPE_LanguageOcrSpanish			},		// スペイン語.
	{"no",  	TYPE_LanguageOcrNorwegian		},		// ノルウェー語.
	{"fi",  	TYPE_LanguageOcrFinnish			},		// フィンランド語.
	{"cs",  	TYPE_LanguageOcrCzech			},		// チェコ語.
	{"hu",  	TYPE_LanguageOcrHungarian		},		// ハンガリー語.
	{"el",  	TYPE_LanguageOcrGreek			},		// ギリシア語.
	{"tr",  	TYPE_LanguageOcrTurkish			},		// トルコ語.
	{"zh-TW",	TYPE_LanguageOcrChinese2		},		// 台湾語(TW は ISO3166 で指定された国名).
	{"zh-CN",	TYPE_LanguageOcrChinese1		},		// 中国語(CN は ISO3166 で指定された国名).
	{"ko",  	TYPE_LanguageOcrKorean			},		// ハングル語.
	{"ru",  	TYPE_LanguageOcrRussian			},		// ロシア語.
	{"hr",  	TYPE_LanguageOcrCroatian		},		// クロアチア語.
	{"uk",  	TYPE_LanguageOcrUkrainian		},		// ウクライナ語.
	{"ro",  	TYPE_LanguageOcrRomanian		},		// ルーマニア語.
	{"sk",  	TYPE_LanguageOcrSlovak			},		// スロバキア語.
	{"pl",  	TYPE_LanguageOcrPolish			},		// ポーランド語.
	{"lt",  	TYPE_LanguageOcrLithuanian		},		// リトアニア語.
	{"et",  	TYPE_LanguageOcrEstonian		},		// エストニア語.
	{"lv",  	TYPE_LanguageOcrLatvian			},		// ラトビア語.
	{"sl",  	TYPE_LanguageOcrSlovenia		},		// スロベニア語.
	{"ca",  	TYPE_LanguageOcrCatalan			},		// カタロニア語.
	{"bg",  	TYPE_LanguageOcrBulgarian		},		// ブルガリア語.
};

#if defined(DEF_Soc1200) && !defined(FUM_LINUX)
// 全言語データロードをWARP前に実施.
static Bool bl_InitLanguage_pre = False;
static Bool bl_InitLanguage_post = False;

static TYPE_LanguageOcr e_LanguageOcr[9];
#else //defined(DEF_Soc1200) && !defined(FUM_LINUX).
static Bool bl_InitLanguage = False;
#endif //defined(DEF_Soc1200) && !defined(FUM_LINUX).

static const STPanelNeedInflateCheckTable stPanelNeedInflateCheckTable[] = {
	{ TYPE_MachineTypeRhein1,		False },
	{ TYPE_MachineTypeRhein3,		False },
	{ TYPE_MachineTypeRhein1_5,		False },
	{ TYPE_MachineTypeRhein2,		False },
	{ TYPE_MachineTypeLaPlata1,		False },
	{ TYPE_MachineTypeLaPlata2,		False },
	{ TYPE_MachineTypeGanges1,		False },
	{ TYPE_MachineTypeGanges2,		False },
	{ TYPE_MachineTypeMirage,		False },
	{ TYPE_MachineTypeRhein2_5,		False },
	{ TYPE_MachineTypeRhein3_2,		False },
// 新しい機種のたびに、ここに追加するのは、忘れる可能性が高い.
// よって、今後はほぼ無いと思われる非圧縮で対応が必要な機種のみ、テーブルへ追加するようにする.

};

#if defined(SIMULATOR)
static Bool bl_SimPinterVersion = False; // SIM_PUI が起動しているかどうかのフラグ.
#endif
static PNLC_SelectLangInfo c_SelectLangInfo;		// 選択言語情報管理クラスオブジェクト.
//----------------------------------------------------------------------------
// ◇ 静的バッファ
//----------------------------------------------------------------------------

//--- ROM 言語データイメージ ---//
#if defined(SIMULATOR)
static unsigned char	guc_allDataROM[SIZE_FILEDATA_ALL];		// SIMWIN 用.
#else
#endif

//--- 選択言語用バッファ (RAM) ---//
#if defined(DEF_Soc1200) && !defined(FUM_LINUX)
// 全言語データロードをWARP前に実施.
static Bool b_isMsgDataLoaded_pre = False;								// メッセージデータのロード完了フラグ.
static Bool b_isMsgDataLoaded_post = False;								// メッセージデータのロード完了フラグ.
#else //defined(DEF_Soc1200) && !defined(FUM_LINUX).
static Bool b_isMsgDataLoaded = False;
#endif //defined(DEF_Soc1200) && !defined(FUM_LINUX).
#ifdef SIMULATOR
// 9言語バイナリ対応.
static unsigned char	guc_allDataRAM_SIM[SIZE_FILEDATA_SELECT* PNL_MAX_LANGUAGES];		// メッセージデータ(全件).
#endif // #ifdef SIMULATOR
static unsigned char*	guc_allDataRAM = NULL;								// メッセージデータ(全件).
static unsigned char*	guc_selDataRAM = NULL;								// メッセージデータ(選択).

#ifdef SIMULATOR
static unsigned char	guc_selDataRAM_Single_SIM[SIZE_FILEDATA_SELECT];	// 1言語分データ領域.
#endif

Bool isAllocSuccess = False;										// メモリ確保成功のフラグ.

static unsigned char*	guc_extFontData = NULL;						// フォントデータ.
static unsigned char*	gpuc_extFontData[3];

static unsigned char*	g_puc_PanelLangAreaPtr = NULL;			// パネル言語展開領域ポインタ格納用.
static unsigned char*	g_puc_OapMyPanelLangAreaPtr = NULL;	// OAPMyパネル言語展開領域ポインタ格納用.



static Bool b_isKeyMapDataLoaded = False;					// キーMAPデータのロード完了フラグ.

// NULLで文字列を初期化する.
static char sc_MessgeBinPath[64] = "";

static uint32_t ui32_FontSwitch[PNL_MAX_LANGUAGES];	//フォントスイッチデータ格納.

/*================================================================================================
	モジュール内関数のプロトタイプ宣言
================================================================================================*/
//----------------------------------------------------------------------------
// ◇ 外部非公開関数
//----------------------------------------------------------------------------

// 初期化関数.
#if defined(DEF_Soc1200) && !defined(FUM_LINUX)
// 全言語データロードをWARP前に実施.
static void	initLanguage_pre();
static void	initLanguage_post();
#else //defined(DEF_Soc1200) && !defined(FUM_LINUX).
static void	initLanguage();
#endif //defined(DEF_Soc1200) && !defined(FUM_LINUX).

// 言語データからメモリへの転送.
static inline int loadLangData2Ram( void* pRam, int32_t i32_src, size_t size );

// 言語データチェックサム取得関数.
static unsigned short getLanguageDataChecksum( void );

#if defined(SIMULATOR)
// チェックサム確認関数.
static Bool	IsValidCheckSum( unsigned char* puc_buffer, uint32_t ui32_bufSize );
#endif	// SIMULATOR

// 固定データサイズ取得関数.
static int32_t	GetFixedDataSize();

// 日本語言語コード判定関数.
static Bool	IsLanguageCodeJp( Bool* pbl_firstJp, Bool* pbl_selectJp, int si_LangNo );

// 言語コード獲得関数.
static TYPE_LanguageOcr GetLanguageCodeOcr( int si_LangNo );

// オフセット取得関数.
static Bool	GetLangDataOffset( int32_t* pi32_select, int32_t* pi32_service, Bool bl_1stJp, Bool bl_jp, int si_LangNo );

// 固定データおよび全言語コード取得設定関数.
static int32_t	SetFixedDataAndLangCode( int32_t i32_LangCount );

// 言語データ取得設定関数.
static int32_t	SetLangData( int32_t i32_dataOffset, int32_t i32_serviceOffset );

// フォントデータ取得設定関数
static int32_t	SetLangFontData( int si_LangNo, int32_t i32_storedSize );
// No307966対応.
// 選択フォントデータ再構築関数.
static int loadSelectFont( int LangNo );



// 終端アドレス格納関数
static void	SetTerminalAddress( int32_t i32_storedSize );


Bool isNeedInflate();

#endif // DIV_MFP_LIB
Bool do_decompress( unsigned short* pSrcBuf, uint32_t ui32_SrcBufSize,
                    unsigned char* pOutBuf, uint32_t ui32_OutBufSize, uint32_t* ui32_OutSize );

#ifndef DIV_MFP_LIB

// MFP Version可否判定関数.
static Bool IsMFPVersion();

#if defined(DEF_Soc1200) && !defined(FUM_LINUX)
// 治具パネルモード可否判定関数.
static Bool IsJigPanelMode();

// スキャナー有無判定関数.
static Bool IsScanerExist();
#endif //defined(DEF_Soc1200) && !defined(FUM_LINUX).

// Printer用パネル言語データへのオフセット取得関数.
static int32_t GetPrinterOffsetAddress();

// 固定データをRAMに書き込む関数.
static void WriteToRAMFixedData();

// MFP/Printerを判別してオフセットを取得する.
static int32_t GetOffsetIfPrinter();


// OapMyパネル用フォントデータ取得設定関数.
static int32_t SetLangData_OapMyPanel( int32_t i32_dataOffset, int32_t i32_serviceOffset );
static void loadKeyMapData();											// KeyMapデータのロード（ROM→RAM）.
static Bool loadSoftKeyMapData( int32_t,int32_t,PDPS_SoftKeyMap* );				// ソフトキーボードKeyMapデータのロード（ROM→RAM）.
static Bool loadUSBKeyMapData( int32_t ,int32_t ,PDPS_USBKeyMap* );				// 外付けキーボードKeyMapデータのロード（ROM→RAM）.
static Bool makeOffsetTable( const uchar*,uchar ,PNLS_KeyMapOffsetTable* );	// オフセットテーブル作成.
static Bool makeSoftCharCodeTable( const uchar*,uchar,PLBS_GeneralKeyInfo* );// ソフトキーボード用文字コードテーブル作成..
static Bool makeUSBCharCodeTable( const uchar*,uchar,PDPS_USBCharCode* );	// 外付けキーボード用文字コードテーブル作成..
static Bool getViewLanguage( int32_t,TYPE_Language* );							// 表示言語情報取得.
static	TYPE_Language	convertTYPELanguage( TYPE_Language e_Language );

static void setExtLangFontData( TYPE_Language ec_Type );
static int32_t	GetFixedDataSize_FromMSGBIN();			// メッセージバイナリ内固定データサイズ取得関数.
static int loadSelectLangKeymap( uchar* uc_RAMBuffer, uchar uc_RAMBufferSize, int32_t i32_OffsetTableKey, uchar uc_DataSize );		//全選択言語のオフセットアドレス処理.

GFWE_LCD_Size changeFontSwitchVal( int si_LangNo );	//フォントスイッチデータ変換処理.
GFWE_LCD_Size getDefaultFontSwitchVal();	//初期値のフォントスイッチデータ取得.

//////////////////////////////////////////////////////////////////////////////
//
// ◇ マクロ定義
//
//////////////////////////////////////////////////////////////////////////////
//----------------------------------------------------------------------------
//【名  前】 U32_GETVALUE
//
//【内  容】 指定バッファから 符号なし 4 バイト整数値を取り出す
//
//【引  数】 [i] p : バッファの先頭アドレス
//           [i] n : 先頭からのオフセット[B]
//
//【戻り値】 符号なし4 バイト整数
//
//----------------------------------------------------------------------------
#define U32_GETVALUE( p, n )		(*((uint32_t*)&(((unsigned char*)p)[n])))

//----------------------------------------------------------------------------
//【名  前】 S32_GETVALUE
//
//【内  容】 指定バッファから 符号付き 4 バイト整数値を取り出す
//
//【引  数】 [i] p : バッファの先頭アドレス
//           [i] n : 先頭からのオフセット[B]
//
//【戻り値】 符号付き 4 バイト整数
//
//----------------------------------------------------------------------------
#define S32_GETVALUE( p, n )		(*((int32_t*)&(((unsigned char*)p)[n])))

//----------------------------------------------------------------------------
//【名  前】 S32_LONG_ROM
//
//【内  容】 ROM 全体から 符号付き 4 バイト整数値を取り出す
//
//【引  数】 [i] n : 先頭からのオフセット [B]
//
//【戻り値】 符号付き 4 バイト整数
//
//----------------------------------------------------------------------------
#define S32_LONG_ROM( n )			S32_GETVALUE(guc_allDataROM, n)

//----------------------------------------------------------------------------
//【名  前】 S32_LONG_RAM
//
//【内  容】 選択言語 RAM から 符号付き 4 バイト整数値を取り出す
//
//【引  数】 [i] n : 先頭からのオフセット [B]
//
//【戻り値】 符号付き 4 バイト整数
//
//----------------------------------------------------------------------------
#define S32_LONG_RAM( n )		S32_GETVALUE(guc_selDataRAM, n)
#define S32_LONG_RAM_READ( n )		((guc_selDataRAM != NULL) ? S32_GETVALUE(guc_selDataRAM, n) : -1)

//----------------------------------------------------------------------------
//【名  前】 S32_WORD_SIZE
//
//【内  容】 指定のサイズ値を2byte(WORD)単位のサイズ値に変換する.
//           具体的には、奇数サイズだったら1byte大きな値を返す.
//
//【引  数】 [i] n : サイズ値.
//
//【戻り値】 符号付き 4 バイト整数
//
//----------------------------------------------------------------------------
#define S32_WORD_SIZE( n )		 ( n + ( n % 2 ))

#endif // DIV_MFP_LIB
// ログ表示用.
#define DISPLOG(a,b,c,d,e,f,g) SYSF_logMsg(a,b,c,d,e,f,g)
#ifndef DIV_MFP_LIB


//////////////////////////////////////////////////////////////////////////////
//
// ◇ 関数
//
//////////////////////////////////////////////////////////////////////////////

//================================================================================================
// 概要		:言語データの指定位置から指定サイズメモリに読込む.
// 戻り値	:int 正常に終了した場合は 0, それ以外は -1
// 備考		:
// 作成者	:川端.
// レビュー	:
//================================================================================================
// 言語データからメモリへの転送.
static inline int loadLangData2Ram( void* pRam, int32_t i32_src, size_t size )
{
	memset( pRam, 0, size );	// メモリクリア.
	if( i32_src < 0 ) { return -1; }

	getLanguageDataForUINewBin( ( unsigned short* )pRam, (uint32_t)( i32_src ), (uint32_t)size );

	return 0;
}

#if defined(SIMULATOR)
//----------------------------------------------------------------------------
//【関数名】 loadAllLanguage
//
//【内  容】 言語データファイル読み込み関数
//
//【引  数】 [i] psc_filename : 言語データが格納されているファイル名
//
//【戻り値】 正常に終了した場合は 0, それ以外は -1
//
//----------------------------------------------------------------------------
int loadAllLanguage( const char* psc_filename )
{
	FILE*	fp = NULL;
	int32_t		i32_readSize = -1;

	if( psc_filename == NULL ) {
		return -1;
	}

	// 言語データ ROM 用 メモリの初期化
	memset( guc_allDataROM, 0, sizeof( guc_allDataROM ) );


	// バイナリモードでファイルを開く
	if( ( fp = SYSF_fopen( psc_filename, "rb" ) ) == NULL ) {
		return -1;
	}

	// ファイル読み込み
	i32_readSize = fread( guc_allDataROM, sizeof( unsigned char ), SIZE_FILEDATA_ALL, fp );
	if( i32_readSize <= 0 ) {
		SYSF_fclose( fp );
		return -1;
	}

	int iseof = feof( fp );

	// 全て読み出すことができたか確認
	if( iseof == 0 ) {
		// 終端ではない → バッファが足りない → SIZE_FILEDATA_ALL の変更必要
		SYSF_fclose( fp );
		return -1;
	}

	// ファイルを閉じる
	if( SYSF_fclose( fp ) != 0 ) {
		return -1;
	}

	// チェックサム確認
	if( !IsValidCheckSum( ( unsigned char* )guc_allDataROM, ( uint32_t )i32_readSize ) ) {
		// 言語データROM用 メモリの初期化
		memset( guc_allDataROM, 0, sizeof( guc_allDataROM ) );
		return -1;
	}

	return 0;
}
#endif

//================================================================================================
// 概要		:言語データバージョン取得関数.
// 戻り値	:int 正常に終了した場合は 0, それ以外は -1
// 備考		:
// 作成者	:川端.
// レビュー	:
// 変更履歴	:
//================================================================================================
// 言語データバージョン取得関数.
int		getLanguageDataVersion( char* buf, int buf_size )
{
	memset( buf, 0, buf_size );
	// CFが刺さっているか検出する.
	//Mosel 関数で取得するように変更する必要がある。井川
#ifdef LINUX
	if( False == isAbleMsgBin() )
#else
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	if( True == pc_SystemInfo->isMediaBootType() )
#endif	// #ifdef LINUX.
	{
		// 刺さっている場合.
		strncpy( buf, "Dummy Language", ( size_t )buf_size );
		return 0;
	}
	strncpy( buf, ( const char* )guc_selDataRAM + OFFSET_LANGVERSION, ( size_t )buf_size );
	return 0;

}

//----------------------------------------------------------------------------
//【関数名】 loadSelectLanguage
//
//【内  容】 選択言語データ再構築関数
//
//【引  数】 [i] LangNo  : 言語番号-1 (0～言語数-1)
//
//【戻り値】 正常に終了した場合は 0, それ以外は -1
//
//
//【備  考】
//
//    ◇ Rhein1 の場合のサービス言語位置
//
//     +-----------+------------------------+------------------------+
//     | 第1言語   | 日本語サービス言語位置 | 英語サービス言語位置   |
//     +===========+========================+========================+
//     | 日本      | 第1言語のサービス      | 第2言語のサービス      |
//     +-----------+------------------------+------------------------+
//     | 日本以外  | 日本語と同じ場所       | 第1言語のサービス      |
//     +-----------+------------------------+------------------------+
//
//    ◇ 拡張メモリがない場合、loadSelectLanguage_pre()はWARP前にコールされない。
//       そのため、WARP前に使用できないBlack APIをコールできる。
//         ・PnlComGetMyPanelType()
//
//----------------------------------------------------------------------------
#if defined(DEF_Soc1200) && !defined(FUM_LINUX)
int loadSelectLanguage_pre( int LangNo )
{

	// 初期化.
	if( bl_InitLanguage_pre == False ) {
		initLanguage_pre();
	}

	// 変数宣言
	int32_t	i32_allDataOffset  = 0;			// 選択言語データ用オフセット
	int32_t	i32_serviceOffset  = 0;			// サービス言語データ用オフセット
	int32_t	i32_storedDataSize = 0;			// 格納したデータサイズ
	Bool	bl_firstJp	= False;				// 第 1 言語 = 日本語 判定フラグ
	Bool	bl_jp = False;						// 選択言語  = 日本語 判定フラグ
	int32_t	i32_langCount = getLangCount();	// 設定可能言語数
	int32_t	i32_size;

	// 引数のエラーチェック
	if( LangNo < 0 || LangNo >= i32_langCount ) {
		return -1;
	}

	// 言語データが日本語であるかなどをチェックする
	IsLanguageCodeJp( &bl_firstJp, &bl_jp, LangNo );

	// 拡張メモリ無し時.
	if( False == APIC_Enabler::newInstance()->isExtendedMemory() ) {
		logMsg( "[loadSelectLanguage]guc_selDataRAM: %p\n", ( intptr_t )guc_selDataRAM,2,3,4,5,6 );
		if( NULL != g_puc_PanelLangAreaPtr && NULL != g_puc_OapMyPanelLangAreaPtr ) {
			// MyパネルタイプがOAPMｙパネルの場合.
			if( PCGE_OapMyPanel == PnlComGetMyPanelType() ) {
				guc_selDataRAM = g_puc_OapMyPanelLangAreaPtr;		// OAPMyパネル言語展開領域ポインタ格納用.
				logMsg( "[loadSelectLanguage]ChangeAreaPtr guc_selDataRAM ⇒ g_puc_OapMyPanelLangAreaPtr( %p ) \n", ( intptr_t )guc_selDataRAM,2,3,4,5,6 );
			} else {
				guc_selDataRAM = g_puc_PanelLangAreaPtr;			// パネル言語展開領域ポインタ格納用.
				logMsg( "[loadSelectLanguage]ChangeAreaPtr guc_selDataRAM ⇒ g_puc_PanelLangAreaPtr( %p ) \n", ( intptr_t )guc_selDataRAM,2,3,4,5,6 );
			}
		}
	}


	// メモリの初期化
	memset( guc_selDataRAM, 0, SIZE_FILEDATA_SELECT );

	// 先頭から(バージョン情報) から言語コードまでを格納
	i32_storedDataSize = SetFixedDataAndLangCode( i32_langCount );

	// データ格納オフセットを取得
	GetLangDataOffset( &i32_allDataOffset, &i32_serviceOffset, bl_firstJp, bl_jp, LangNo );

	if( NULL != g_puc_PanelLangAreaPtr && NULL != g_puc_OapMyPanelLangAreaPtr ) {
		// MyパネルタイプがOAPMｙパネルかつ拡張メモリ無しの場合.
		if( PCGE_OapMyPanel == PnlComGetMyPanelType() && ( False == APIC_Enabler::newInstance()->isExtendedMemory() ) ) {
			// OapMyパネル選択言語データ格納.
			i32_size = SetLangData_OapMyPanel( i32_allDataOffset, i32_serviceOffset );
		} else {
			// 選択言語データ格納.
			i32_size = SetLangData( i32_allDataOffset, i32_serviceOffset );
		}
	} else {
		// 選択言語データ格納.
		i32_size = SetLangData( i32_allDataOffset, i32_serviceOffset );
	}
	// 選択言語データ格納.
	if( i32_size <= 0 ) {
		return -1;
	}
	i32_storedDataSize += i32_size;

	// 選択言語フォントデータ格納.
	i32_storedDataSize += SetLangFontData( LangNo, i32_storedDataSize );



	// Panel言語終端アドレスを格納
	SetTerminalAddress( i32_storedDataSize );


	return 0;
}

int loadSelectLanguage_post()
{
	if( ( False == APIC_Enabler::newInstance()->isExtendedMemory() ) 	// 拡張メモリなしの場合は毎回ロードする.
	    || ( False == b_isKeyMapDataLoaded ) ) {
		loadKeyMapData();
		b_isKeyMapDataLoaded = True;
	}

	return 0;
}

int loadSelectLanguage( int LangNo )
{
	int i_ret = loadSelectLanguage_pre( LangNo );
	if( i_ret == 0 ) {
		return loadSelectLanguage_post();
	} else {
		return i_ret;
	}
}
#else //defined(DEF_Soc1200) && !defined(FUM_LINUX).
int loadSelectLanguage( int LangNo )
{

	// 初期化.
	if( bl_InitLanguage == False ) {
		initLanguage();
	}

	// 変数宣言
	int32_t	i32_allDataOffset  = 0;			// 選択言語データ用オフセット
	int32_t	i32_serviceOffset  = 0;			// サービス言語データ用オフセット
	int32_t	i32_storedDataSize = 0;			// 格納したデータサイズ
	Bool	bl_firstJp	= False;				// 第 1 言語 = 日本語 判定フラグ
	Bool	bl_jp = False;						// 選択言語  = 日本語 判定フラグ
	int32_t	i32_langCount = getLangCount();	// 設定可能言語数
	int32_t	i32_size;

	// 引数のエラーチェック
	if( LangNo < 0 || LangNo >= i32_langCount ) {
		return -1;
	}

	// 言語データが日本語であるかなどをチェックする
	IsLanguageCodeJp( &bl_firstJp, &bl_jp, LangNo );

	// 拡張メモリ無し時.
	if( False == APIC_Enabler::newInstance()->isExtendedMemory() ) {
		logMsg( "[loadSelectLanguage]guc_selDataRAM: %p\n", ( intptr_t )guc_selDataRAM,2,3,4,5,6 );
		// MyパネルタイプがOAPMｙパネルの場合.
		if( PCGE_OapMyPanel == PnlComGetMyPanelType() ) {
			guc_selDataRAM = g_puc_OapMyPanelLangAreaPtr;		// OAPMyパネル言語展開領域ポインタ格納用.
			logMsg( "[loadSelectLanguage]ChangeAreaPtr guc_selDataRAM ⇒ g_puc_OapMyPanelLangAreaPtr( %p ) \n", ( intptr_t )guc_selDataRAM,2,3,4,5,6 );
		} else {
			guc_selDataRAM = g_puc_PanelLangAreaPtr;			// パネル言語展開領域ポインタ格納用.
			logMsg( "[loadSelectLanguage]ChangeAreaPtr guc_selDataRAM ⇒ g_puc_PanelLangAreaPtr( %p ) \n", ( intptr_t )guc_selDataRAM,2,3,4,5,6 );
		}
	}

	// メモリの初期化
	memset( guc_selDataRAM, 0, SIZE_FILEDATA_SELECT );

	// 先頭から(バージョン情報) から言語コードまでを格納
	i32_storedDataSize = SetFixedDataAndLangCode( i32_langCount );

	// データ格納オフセットを取得
	GetLangDataOffset( &i32_allDataOffset, &i32_serviceOffset, bl_firstJp, bl_jp, LangNo );

	// MyパネルタイプがOAPMｙパネルかつ拡張メモリ無しの場合.
	if( PCGE_OapMyPanel == PnlComGetMyPanelType() && ( False == APIC_Enabler::newInstance()->isExtendedMemory() ) ) {
		// OapMyパネル選択言語データ格納.
		i32_size = SetLangData_OapMyPanel( i32_allDataOffset, i32_serviceOffset );
	} else {
		// 選択言語データ格納.
		i32_size = SetLangData( i32_allDataOffset, i32_serviceOffset );
	}
	// 選択言語データ格納.
	if( i32_size <= 0 ) {
		return -1;
	}
	i32_storedDataSize += i32_size;

	// 選択言語フォントデータ格納.
	i32_storedDataSize += SetLangFontData( LangNo, i32_storedDataSize );



	// Panel言語終端アドレスを格納
	SetTerminalAddress( i32_storedDataSize );

	if( ( False == PnlComGetEnabler()->isExtendedMemory() ) 	// 拡張メモリなしの場合は毎回ロードする.
	    || ( False == b_isKeyMapDataLoaded ) ) {
		loadKeyMapData();
		b_isKeyMapDataLoaded = True;
	}

	return 0;
}
#endif //defined(DEF_Soc1200) && !defined(FUM_LINUX).

//================================================================================================
// 概要		:選択言語データ再構築関数.
// 引  数   :TYPE_Language   : 言語タイプ
// 戻り値	:int 正常に終了した場合は 0, それ以外は -1
// 備考		:
// 作成者	:川端.
// レビュー	:
//================================================================================================
// 言語データバージョン取得関数.
int		loadSelectLangType( TYPE_Language ec_Type )
{
	// 初期化.
#if defined(DEF_Soc1200) && !defined(FUM_LINUX)
	if( bl_InitLanguage_pre == False ) {
		initLanguage_pre();
	}
	if( bl_InitLanguage_post == False ) {
		initLanguage_post();
	}
#else //defined(DEF_Soc1200) && !defined(FUM_LINUX).
	if( bl_InitLanguage == False ) {
		initLanguage();
	}
#endif //defined(DEF_Soc1200) && !defined(FUM_LINUX).

	int32_t i32_LangCount = getLangCount();		// 設定可能言語数
	int i;
	char szLangCode[SIZE_LANGCODE] = "en";

	// 指定された言語タイプに一致する言語コードを検索する.
	for( i = 0; i < ( int )( sizeof( stPanelLanguageTable ) / sizeof( stPanelLanguageTable[0] ) ); i++ ) {
		if( stPanelLanguageTable[i].ec_Type == ec_Type ) {
			strncpy( szLangCode, stPanelLanguageTable[i].p_Code, sizeof( szLangCode )-1 );
			break;
		}
	}

	// 見つかった言語コードが何番目か検索し、切換える.
	int i_en = 0;			// 英語が何言語目か(見つからないときは0のまま).
	for( i = 0; i < i32_LangCount; i++ ) {
		if( strcmp( szLangCode, ( const char* )&guc_selDataRAM[OFFSET_1ST_LANGCODE + ( SIZE_LANGCODE * i )] ) == 0 ) {
			// 見つかったので言語を切換える.
			setExtLangFontData( ec_Type );
			return loadSelectLanguage( i );
		}
		if( strcmp( "en", ( const char* )&guc_selDataRAM[OFFSET_1ST_LANGCODE + ( SIZE_LANGCODE * i )] ) == 0 ) {
			// 英語発見.
			i_en = i;
		}
	}
	// 見つからなかった.
	setExtLangFontData( TYPE_LanguageAmerican );
	return loadSelectLanguage( i_en );
}

//概要　　：言語データ切り替え処理(i-option)　：i-optionでは全言語データがロード済みであるため参照アドレスの切り替えを行う(初回呼び出し時に限り全言語のロードを行う).
#if defined(DEF_Soc1200) && !defined(FUM_LINUX)
int		loadSelectLangType_IOP_pre( TYPE_Language ec_Type )
{
	// まだ、言語データをロードしていない場合は、ロードを行う.
	if( False == b_isMsgDataLoaded_pre )
	{
		setLangBuf();
	}

	// 言語データのロードをパーティションへ行っていない場合は、従来の展開を行う.
	if( False == isAllocSuccess )
	{
		b_isMsgDataLoaded_pre = True;
		return 0;
	}

	// 初期化.
	if( bl_InitLanguage_pre == False ) {
		initLanguage_pre();
	}

	int32_t i32_LangCount = getLangCount();		// 設定可能言語数.
	int siCounter = 0;

	// 言語データのロードがされていない場合は全言語データをロードする.
	if( False == b_isMsgDataLoaded_pre )
	{
		if( True == PLBC_FuncVersionCheck::isOK_Type1() ) {
			for( ushort i=0; i<9; i++ ) {
				e_LanguageOcr[i] = TYPE_LanguageOcrNone;
			}
		}

		for( siCounter = 0; siCounter < i32_LangCount; siCounter++ )
		{
			guc_selDataRAM = guc_allDataRAM + ( SIZE_FILEDATA_SELECT * siCounter );

			// 各言語用の配列を初期化する.
			// この処理を行わないとメッセージデータが展開されない.
			// 先頭の配列はinitLanguage()内で初期化処理が行われているためここでは処理しない.
			if( siCounter != 0 )
			{
				WriteToRAMFixedData();			// 固定データをRAMに書き込む関数.
			}
			loadSelectLanguage_pre( siCounter );	// メッセージデータをロードする.

			if( True == PLBC_FuncVersionCheck::isOK_Type1() ) {
				e_LanguageOcr[siCounter] = GetLanguageCodeOcr( siCounter );	// 言語コードの獲得.
			}

		}
		// 言語データのロードが完了したことをあらわすフラグを立てる.
		b_isMsgDataLoaded_pre = True;
	}

	return 0;
}

int		loadSelectLangType_IOP( TYPE_Language ec_Type )
{
// comment 2015/03/17 OPS1-Sky佐藤 [ZeusS][CQ511108] 言語情報の取得／設定処理見直し.
	PnlGuiWrapperSetGuiLanguage( ec_Type );

	GFWE_LCD_Size FontSize = GFWE_LCD_Size_7inch;
	GFWC_System* pc_GuiSystem	= GFWC_System::getInstance();
	if( NULL == pc_GuiSystem ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return 0;
	}

	// 治具パネルモードの場合、言語データを再ロード.
	if( True == IsJigPanelMode() ) {
		bl_InitLanguage_pre = False;
		b_isMsgDataLoaded_pre = False;
		isAllocSuccess = False;

		loadSelectLangType_IOP_pre( ec_Type );
	}
#ifndef SIMULATOR
	// 仕向け情報をQt側へ通知.
	PnlComMtuiIf_MarketAreaNotify::instance()->setEnableChinese( PnlComIsChineseDestination() );
#endif	// SIMULATOR.

	PDTG_SetExtendMemoryInfo( APIC_Enabler::newInstance()->isExtendedMemory() );

	// 言語データのロードをパーティションへ行っていない場合は、従来の展開を行う.
	if( False == isAllocSuccess )
	{
		int i_ret = loadSelectLangType( ec_Type );
		b_isMsgDataLoaded_post = True;
		PDTG_SetLangData( guc_selDataRAM );

		return i_ret;
	}

	// 初期化.
	if( bl_InitLanguage_post == False ) {
		initLanguage_post();
	}

	int32_t i32_LangCount = getLangCount();		// 設定可能言語数.
	int siCounter = 0;
	char szLangCode[SIZE_LANGCODE] = "en";

	// 言語データのロードがされていない場合は全言語データをロードする.
	if( False == b_isMsgDataLoaded_post ) {
		for( siCounter = 0; siCounter < i32_LangCount; siCounter++ )
		{
			guc_selDataRAM = guc_allDataRAM + ( SIZE_FILEDATA_SELECT * siCounter );

			loadSelectLanguage_post();	// メッセージデータをロードする.
		}

		if( True == PLBC_FuncVersionCheck::isOK_Type1() ) {
			PnlComGetSearchablePDFSetting()->setDownloadCompLangList( e_LanguageOcr );

			PnlComGetSearchablePDFSetting()->setDownloadCompLangListFlg( True );
		}

		// 言語データのロードが完了したことをあらわすフラグを立てる.
		b_isMsgDataLoaded_post = True;
	}

	// 指定された言語タイプに一致する言語コードを検索する.
	for( siCounter = 0; siCounter < ( int )( sizeof( stPanelLanguageTable ) / sizeof( stPanelLanguageTable[0] ) ); siCounter++ ) {
		if( stPanelLanguageTable[siCounter].ec_Type == ec_Type ) {
			strncpy( szLangCode, stPanelLanguageTable[siCounter].p_Code, sizeof( szLangCode )-1 );
			break;
		}
	}

	// 見つかった言語コードが何番目か検索し、切換える.
	int i_en = 0;			// 英語が何言語目か(見つからないときは0のまま).
	for( siCounter = 0; siCounter < i32_LangCount; siCounter++ ) {
		if( strcmp( szLangCode, ( const char* )guc_allDataRAM + ( ( SIZE_FILEDATA_SELECT * siCounter ) + ( OFFSET_1ST_LANGCODE + ( SIZE_LANGCODE * siCounter ) ) ) ) == 0 ) {
			// 見つかったので言語を切換える.
			guc_selDataRAM = guc_allDataRAM + ( SIZE_FILEDATA_SELECT * siCounter );
			int iret;
			SHM_TYPE_DATA* p = NULL;

			// 共有メモリクラス取得
			DivCommon_Shm* p_DivCommon_Shm = DivCommon_Shm::newInstance();

			// アタッチ
			iret = p_DivCommon_Shm->Shm_attach( ( void* )&p );

			// アタッチ成功
			if( iret == 0 )
			{
				p->e_allDataRAM_size = (uint32_t)(  SIZE_FILEDATA_SELECT * siCounter );
				// デタッチ
				iret = p_DivCommon_Shm->Shm_detattch( p );
			}

			// No307966対応.
			// 選択フォントをロードする.
			loadSelectFont( siCounter );

			PDTG_SetLangData( guc_selDataRAM );
			//フォントスイッチに適したデータを設定.
			FontSize = changeFontSwitchVal( siCounter );
			pc_GuiSystem->changeFontConvTbl( FontSize );
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更].
			// Legacy画面のキーラベルを更新する為に、GUIに通知する.
			PNLC_SH3PaneMenuControlLib::newInstance()->notifyChangeLanguage();
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.

			return 0;
		}
		if( strcmp( "en", ( const char* )guc_allDataRAM + ( SIZE_FILEDATA_SELECT * siCounter ) + ( OFFSET_1ST_LANGCODE + ( SIZE_LANGCODE * siCounter ) ) ) == 0 ) {
			// 英語発見.
			i_en = siCounter;
		}
	}
	// 見つからなかった.
	// 強制的に英語表示.
	guc_selDataRAM = guc_allDataRAM + ( SIZE_FILEDATA_SELECT * i_en );
	int iret;
	SHM_TYPE_DATA* p = NULL;

	// 共有メモリクラス取得
	DivCommon_Shm* p_DivCommon_Shm = DivCommon_Shm::newInstance();

	// アタッチ
	iret = p_DivCommon_Shm->Shm_attach( ( void* )&p );

	// アタッチ成功
	if( iret == 0 )
	{
		p->e_allDataRAM_size = (uint32_t)( SIZE_FILEDATA_SELECT * i_en );
		// デタッチ
		iret = p_DivCommon_Shm->Shm_detattch( p );
	}

	PDTG_SetLangData( guc_selDataRAM );
	//フォントスイッチに適したデータを設定.
	FontSize = changeFontSwitchVal( i_en );
	pc_GuiSystem->changeFontConvTbl( FontSize );
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/02/18 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更].
	// Legacy画面のキーラベルを更新する為に、GUIに通知する.
	PNLC_SH3PaneMenuControlLib::newInstance()->notifyChangeLanguage();
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.

	return 0;
}
#else //defined(DEF_Soc1200) && !defined(FUM_LINUX).
int		loadSelectLangType_IOP( TYPE_Language ec_Type )
{
// comment 2015/03/17 OPS1-Sky佐藤 [ZeusS][CQ511108] 言語情報の取得／設定処理見直し.
	PnlGuiWrapperSetGuiLanguage( ec_Type );

	GFWE_LCD_Size FontSize = GFWE_LCD_Size_7inch;
	GFWC_System* pc_GuiSystem	= GFWC_System::getInstance();
	if( NULL == pc_GuiSystem ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return 0;
	}

	// まだ、言語データをロードしていない場合は、ロードを行う.
	if( False == b_isMsgDataLoaded )
	{
		setLangBuf();
	}

#ifndef SIMULATOR
	// 仕向け情報をQt側へ通知.
	PnlComMtuiIf_MarketAreaNotify::instance()->setEnableChinese( PnlComIsChineseDestination() );
#endif	// SIMULATOR.

	PDTG_SetExtendMemoryInfo( APIC_Enabler::newInstance()->isExtendedMemory() );

	// 言語データのロードをパーティションへ行っていない場合は、従来の展開を行う.
	if( False == isAllocSuccess )
	{
		b_isMsgDataLoaded = True;

		PDTG_SetLangData( guc_selDataRAM );

		return( loadSelectLangType( ec_Type ) );
	}

	// 初期化.
	if( bl_InitLanguage == False ) {
		initLanguage();
	}

	int32_t i32_LangCount = getLangCount();		// 設定可能言語数.
	int siCounter = 0;
	char szLangCode[SIZE_LANGCODE] = "en";

	// 言語データのロードがされていない場合は全言語データをロードする.
	if( False == b_isMsgDataLoaded )
	{

		TYPE_LanguageOcr e_LanguageOcr[9];

		if( True == PLBC_FuncVersionCheck::isOK_Type1() ) {
			for( ushort i=0; i<9; i++ ) {
				e_LanguageOcr[i] = TYPE_LanguageOcrNone;
			}
		}

		for( siCounter = 0; siCounter < i32_LangCount; siCounter++ )
		{
			guc_selDataRAM = guc_allDataRAM + ( SIZE_FILEDATA_SELECT * siCounter );

			// 各言語用の配列を初期化する.
			// この処理を行わないとメッセージデータが展開されない.
			// 先頭の配列はinitLanguage()内で初期化処理が行われているためここでは処理しない.
			if( siCounter != 0 )
			{
				WriteToRAMFixedData();			// 固定データをRAMに書き込む関数.
			}
			loadSelectLanguage( siCounter );	// メッセージデータをロードする.

			if( True == PLBC_FuncVersionCheck::isOK_Type1() ) {
				e_LanguageOcr[siCounter] = GetLanguageCodeOcr( siCounter );	// 言語コードの獲得.
			}

		}

		if( True == PLBC_FuncVersionCheck::isOK_Type1() ) {
			PnlComGetSearchablePDFSetting()->setDownloadCompLangList( e_LanguageOcr );

			PnlComGetSearchablePDFSetting()->setDownloadCompLangListFlg( True );
		}

		// 言語データのロードが完了したことをあらわすフラグを立てる.
		b_isMsgDataLoaded = True;
	}

	// 指定された言語タイプに一致する言語コードを検索する.
	for( siCounter = 0; siCounter < ( int )( sizeof( stPanelLanguageTable ) / sizeof( stPanelLanguageTable[0] ) ); siCounter++ ) {
		if( stPanelLanguageTable[siCounter].ec_Type == ec_Type ) {
			strcpy( szLangCode, stPanelLanguageTable[siCounter].p_Code );
			break;
		}
	}

	// 見つかった言語コードが何番目か検索し、切換える.
	int i_en = 0;			// 英語が何言語目か(見つからないときは0のまま).
	for( siCounter = 0; siCounter < i32_LangCount; siCounter++ ) {
		if( strcmp( szLangCode, ( const char* )guc_allDataRAM + ( ( SIZE_FILEDATA_SELECT * siCounter ) + ( OFFSET_1ST_LANGCODE + ( SIZE_LANGCODE * siCounter ) ) ) ) == 0 ) {
			// 見つかったので言語を切換える.
			guc_selDataRAM = guc_allDataRAM + ( SIZE_FILEDATA_SELECT * siCounter );
			int iret;
			SHM_TYPE_DATA* p = NULL;

			// 共有メモリクラス取得
			DivCommon_Shm* p_DivCommon_Shm = DivCommon_Shm::newInstance();

			// アタッチ
			iret = p_DivCommon_Shm->Shm_attach( ( void* )&p );

			// アタッチ成功
			if( iret == 0 )
			{
				p->e_allDataRAM_size = SIZE_FILEDATA_SELECT * siCounter;
				// デタッチ
				iret = p_DivCommon_Shm->Shm_detattch( p );
			}

			// 選択フォントをロードする.
			loadSelectFont( siCounter );

			PDTG_SetLangData( guc_selDataRAM );

			//フォントスイッチに適したデータを設定.
			FontSize = changeFontSwitchVal( siCounter );
			pc_GuiSystem->changeFontConvTbl( FontSize );
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/05/01 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更].
			// Legacy画面のキーラベルを更新する為に、GUIに通知する.
			PNLC_SH3PaneMenuControlLib::newInstance()->notifyChangeLanguage();
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
			return 0;
		}
		if( strcmp( "en", ( const char* )guc_allDataRAM + ( SIZE_FILEDATA_SELECT * siCounter ) + ( OFFSET_1ST_LANGCODE + ( SIZE_LANGCODE * siCounter ) ) ) == 0 ) {
			// 英語発見.
			i_en = siCounter;
		}
	}
	// 見つからなかった.
	// 強制的に英語表示.
	guc_selDataRAM = guc_allDataRAM + ( SIZE_FILEDATA_SELECT * i_en );
	int iret;
	SHM_TYPE_DATA* p = NULL;

	// 共有メモリクラス取得
	DivCommon_Shm* p_DivCommon_Shm = DivCommon_Shm::newInstance();

	// アタッチ
	iret = p_DivCommon_Shm->Shm_attach( ( void* )&p );

	// アタッチ成功
	if( iret == 0 )
	{
		p->e_allDataRAM_size = SIZE_FILEDATA_SELECT * i_en;
		// デタッチ
		iret = p_DivCommon_Shm->Shm_detattch( p );
	}

	PDTG_SetLangData( guc_selDataRAM );

	//フォントスイッチに適したデータを設定.
	FontSize = changeFontSwitchVal( i_en );
	pc_GuiSystem->changeFontConvTbl( FontSize );
// Comment 2019/08/10 [HCLT-Catherine][IT6_HeliosMLK][Seigyu.10 ]Compilation switch activity.
//#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
// comment 2018/05/01 SCD2 K.Takei [Eagle/SparrowMFP][制御構成変更_No.91_ハードキーレス対応_画面レイアウト変更].
	// Legacy画面のキーラベルを更新する為に、GUIに通知する.
	PNLC_SH3PaneMenuControlLib::newInstance()->notifyChangeLanguage();
#endif // #if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) )	// Eagle/SparrowMFP 10.1inch.

	return 0;
}
#endif //defined(DEF_Soc1200) && !defined(FUM_LINUX).

//概要　　：言語データ格納メモリ切り替え処理(i-option)　：DLK2(マイパネル)が有効の場合はパーティションのメモリを使用.
#ifdef LINUX
extern PART_ID memID_PanelLang;
#endif
// comment 2018/10/11 SCD2-Sky工藤 [特ROM][FQ-ISD2G-1800062-OFP184][AR1063987]SafeQで認証ログイン後に一部の画面でボタン等のテキストが表示されない.
//#define MEM_SIZE_PanelLang					(2*9*1024*1024 )						/* 18MB 9言語分 MemoryPartition Optionメモリあり */
#define MEM_SIZE_PanelLang					(3*9*1024*1024 )						/* 27MB 9言語分 MemoryPartition Optionメモリあり */

#ifndef SIMULATOR
#endif
void setLangBuf()
{
	guc_allDataRAM = NULL;

	int iret;
	SHM_TYPE_DATA* p = NULL;


	// Enabler操作クラス取得.
	APIC_Enabler* pc_Enabler = APIC_Enabler::newInstance();

	// 共有メモリクラス取得
	DivCommon_Shm* p_DivCommon_Shm = DivCommon_Shm::newInstance();

	// アタッチ
	iret = p_DivCommon_Shm->Shm_attach( ( void* )&p );

	// アタッチ成功
	if( iret == 0 )
	{
		// 1ROM_9言語対応：常に全言語ロードする.
		// 共有メモリを使用.
		guc_allDataRAM = p->e_allDataRAM;
		guc_selDataRAM = guc_allDataRAM;
		guc_extFontData = p->e_allDataFontRAM;
		if( True == pc_Enabler->isExtendedMemory() ) {
			// isAllocSuccessのフラグは、拡張メモリ有りの場合のみ True にする.
			isAllocSuccess = True;
		} else {
			// 【212_Myパネル機能の拡張メモリ不要化】.
			g_puc_PanelLangAreaPtr = guc_selDataRAM;
			g_puc_OapMyPanelLangAreaPtr = guc_selDataRAM + SIZE_FILEDATA_SELECT;
		}

		// デタッチ
		iret = p_DivCommon_Shm->Shm_detattch( p );
	}
	else
	{
		// 1ROM_9言語対応：常に全言語ロードする.
#if !defined(FUM_LINUX)
		if( True == pc_Enabler->isExtendedMemory() ) {

#ifndef SIMULATOR	// 2011/01/31 LocalUI分離 リンクエラー対応
			guc_allDataRAM = ( unsigned char* ) malloc( MEM_SIZE_PanelLang );
#else
			guc_allDataRAM = ( unsigned char* )malloc( ( unsigned )( MEM_SIZE_PanelLang ) );
#endif  // #ifndef SIMULATOR	// 2011/01/31 LocalUI分離 リンクエラー対応
		}
#else
//Simの修正
		if( True == pc_Enabler->isExtendedMemory() ) {
			guc_allDataRAM = guc_allDataRAM_SIM;
		}
#endif // #if !defined(FUM_LINUX)

		if( NULL == guc_allDataRAM ) {
#if !defined(FUM_LINUX)
#ifndef SIMULATOR	// 2011/01/31 LocalUI分離 リンクエラー対応
			// パネル言語展開領域格納.
			g_puc_PanelLangAreaPtr = ( unsigned char* ) malloc( SIZE_FILEDATA_SELECT );
			SYSF_logMsg( "[setLangBuf]g_puc_PanelLangAreaPtr: %p\n", ( intptr_t )g_puc_PanelLangAreaPtr,2,3,4,5,6 );
			// OAPMyパネル言語展開領域格納.
			g_puc_OapMyPanelLangAreaPtr = ( uchar* )( malloc( SIZE_FILEDATA_SELECT ) );
			SYSF_logMsg( "[setLangBuf]g_puc_OapMyPanelLangAreaPtr: %p\n", ( intptr_t )g_puc_OapMyPanelLangAreaPtr,2,3,4,5,6 );
			// OAPMyパネルフォントデータ展開領域格納.
			// 初期値をパネル言語展開領域のアドレスとする。
			guc_selDataRAM = g_puc_PanelLangAreaPtr;
			SYSF_logMsg( "[setLangBuf]guc_selDataRAM: %p\n", ( intptr_t )guc_selDataRAM,2,3,4,5,6 );
#else
			// パネル言語展開領域格納.
			g_puc_PanelLangAreaPtr = ( unsigned char* ) malloc( ( unsigned )SIZE_FILEDATA_SELECT );
			SYSF_logMsg( "[setLangBuf]g_puc_PanelLangAreaPtr: %p\n", ( intptr_t )g_puc_PanelLangAreaPtr,2,3,4,5,6 );
			// OAPMyパネル言語展開領域格納.
			g_puc_OapMyPanelLangAreaPtr = ( uchar* )( malloc( ( unsigned )SIZE_FILEDATA_SELECT ) ) );
			SYSF_logMsg( "[setLangBuf]g_puc_OapMyPanelLangAreaPtr: %p\n", ( intptr_t )g_puc_OapMyPanelLangAreaPtr,2,3,4,5,6 );
			// OAPMyパネルフォントデータ展開領域格納.
			// 初期値をパネル言語展開領域のアドレスとする。
			guc_selDataRAM = g_puc_PanelLangAreaPtr;
			SYSF_logMsg( "[setLangBuf]guc_selDataRAM: %p\n", ( intptr_t )guc_selDataRAM,2,3,4,5,6 );
#endif  // #ifndef SIMULATOR	// 2011/01/31 LocalUI分離 リンクエラー対応
#else
			guc_selDataRAM = guc_selDataRAM_Single_SIM;
#endif
		}
		else
		{
			// メモリパーティションを使用.
			guc_selDataRAM = guc_allDataRAM;
			isAllocSuccess = True;
		}
	}
}

//----------------------------------------------------------------------------
//【関数名】 getMessageLangUser
//
//【内  容】 ユーザー言語データメッセージ文字列取得関数
//
//【引  数】 [i] nNumber : メッセージ番号(0～)
//
//【戻り値】 取得したメッセージ文字列へのポインタ
//
//----------------------------------------------------------------------------
unsigned char*	getMessageLangUser( int Number )
{
	int32_t	i32_langDataOffset = 0;				// 選択言語データオフセット
	int32_t	i32_usrDataOffset = 0;				// ユーザー言語データ開始オフセット
	int32_t	i32_msgStrOffset	  = 0;				// メッセージ文字列先頭オフセット
	int32_t	i32_fixedSize = GetFixedDataSize();	// 固定データサイズの取得
	static unsigned char uc_Null[2] = {0, 0};

	// 選択言語データ開始位置を取得.
	i32_langDataOffset = S32_LONG_RAM_READ( i32_fixedSize );

	if( 0 < i32_langDataOffset && i32_langDataOffset < SIZE_FILEDATA_SELECT ) {
		// ユーザー言語データ開始位置を取得
		i32_usrDataOffset = i32_langDataOffset + S32_LONG_RAM_READ( i32_langDataOffset );

		if( 0 < i32_usrDataOffset + sizeof( int32_t ) * Number && i32_usrDataOffset + sizeof( int32_t ) * Number < SIZE_FILEDATA_SELECT ) {
			// メッセージ文字列へのオフセットアドレスを取得
			i32_msgStrOffset = i32_usrDataOffset
			                  + S32_LONG_RAM_READ( i32_usrDataOffset + sizeof( int32_t ) * Number );

			if( 0 < i32_msgStrOffset && i32_msgStrOffset < SIZE_FILEDATA_SELECT ) {
				return &guc_selDataRAM[ i32_msgStrOffset ];
			}
		}
	}
	return uc_Null;
}

//----------------------------------------------------------------------------
//【関数名】 getMessageLangService1
//
//【内  容】 サービス 1 言語データメッセージ文字列取得関数
//
//【引  数】 [i] nNumber : メッセージ番号(0～)
//
//【戻り値】 取得したメッセージ文字列へのポインタ
//
//----------------------------------------------------------------------------
unsigned char*	getMessageLangService1( int Number )
{
	int32_t	i32_langDataOffset = 0;			// 選択言語データオフセット
	int32_t	i32_service1Offset = 0;			// ユーザー言語データ開始オフセット
	int32_t	i32_msgStrOffset	  = 0;			// メッセージ文字列先頭オフセット
	int32_t	i32_fixedSize = GetFixedDataSize();	// 固定データサイズの取得
	static unsigned char uc_Null[2] = {0, 0};

	// 選択言語データ開始位置を取得
	i32_langDataOffset = S32_LONG_RAM_READ( i32_fixedSize );

	if( 0 < i32_langDataOffset + sizeof( int32_t ) * 2 && i32_langDataOffset + sizeof( int32_t ) * 2 < SIZE_FILEDATA_SELECT ) {
		// サービス１言語データ開始位置を取得
		i32_service1Offset = i32_langDataOffset
		                    + S32_LONG_RAM_READ( i32_langDataOffset + sizeof( int32_t ) * 2 );

		if( 0 < i32_service1Offset + sizeof( int32_t ) * Number && i32_service1Offset + sizeof( int32_t ) * Number < SIZE_FILEDATA_SELECT ) {
			// メッセージ文字列へのオフセットアドレスを取得
			i32_msgStrOffset = i32_service1Offset
			                  + S32_LONG_RAM_READ( i32_service1Offset + sizeof( int32_t ) * Number );

			if( 0 < i32_msgStrOffset && i32_msgStrOffset < SIZE_FILEDATA_SELECT ) {
				return &guc_selDataRAM[ i32_msgStrOffset ];
			}
		}
	}
	return uc_Null;
}

//----------------------------------------------------------------------------
//【関数名】 getMessageLangService2
//
//【内  容】 サービス 2 言語データメッセージ文字列取得関数
//
//【引  数】 [i] nNumber : メッセージ番号(0～)
//
//【戻り値】 取得したメッセージ文字列へのポインタ
//
//----------------------------------------------------------------------------
unsigned char*	getMessageLangService2( int Number )
{
	int32_t	i32_langDataOffset = 0;		// 選択言語データオフセット
	int32_t	i32_service2Offset = 0;		// ユーザー言語データ開始オフセット
	int32_t	i32_msgStrOffset   = 0;		// メッセージ文字列先頭オフセット
	int32_t	i32_fixedSize = GetFixedDataSize();	// 固定データサイズの取得
	static unsigned char uc_Null[2] = {0, 0};

	// 選択言語データ開始位置を取得
	i32_langDataOffset = S32_LONG_RAM_READ( i32_fixedSize );

	if( 0 < i32_langDataOffset + sizeof( int32_t ) * 4 && i32_langDataOffset + sizeof( int32_t ) * 4 < SIZE_FILEDATA_SELECT ) {
		// サービス２言語データ開始位置を取得
		i32_service2Offset = i32_langDataOffset
		                    + S32_LONG_RAM_READ( i32_langDataOffset + sizeof( int32_t ) * 4 );

		if( 0 < i32_service2Offset + sizeof( int32_t ) * Number && i32_service2Offset + sizeof( int32_t ) * Number < SIZE_FILEDATA_SELECT ) {
			// メッセージ文字列へのオフセットアドレスを取得
			i32_msgStrOffset = i32_service2Offset
			                  + S32_LONG_RAM_READ( i32_service2Offset + sizeof( int32_t ) * Number );

			if( 0 < i32_msgStrOffset && i32_msgStrOffset < SIZE_FILEDATA_SELECT ) {
				return &guc_selDataRAM[ i32_msgStrOffset ];
			}
		}
	}
	return uc_Null;
}

//----------------------------------------------------------------------------
//【関数名】 getMessageLangBrowser.
//
//【内  容】 ブラウザ言語データメッセージ文字列取得関数.
//
//【引  数】 [o] pi32_Size : データサイズ.
//
//【戻り値】 取得したメッセージ文字列へのポインタ.
//
//----------------------------------------------------------------------------
unsigned char*	getMessageLangBrowser( int32_t* pi32_Size )
{
	int32_t	i32_langDataOffset = 0;			// 選択言語データオフセット.
	int32_t	i32_browserOffset = 0;				// ブラウザ言語データ開始オフセット.
	int32_t	i32_fixedSize = GetFixedDataSize();	// 固定データサイズの取得.
	int32_t	i32_browserSize = 0;					// ブラウザ言語データサイズ.

	static unsigned char uc_Null[2] = {0, 0};

	// 選択言語データ開始位置を取得.
	i32_langDataOffset = S32_LONG_RAM_READ( i32_fixedSize );

	if( 0 < i32_langDataOffset + sizeof( int32_t ) * 6 && i32_langDataOffset + sizeof( int32_t ) * 6 < SIZE_FILEDATA_SELECT )
	{
		// ブラウザデータのサイズ.
		i32_browserSize = S32_LONG_RAM_READ( i32_langDataOffset + sizeof( int32_t ) * 7 );
		*pi32_Size = i32_browserSize;

		// サイズが０の場合.
		if( i32_browserSize == 0 )
		{
			return uc_Null;
		}

		// ブラウザデータのオフセット.
		i32_browserOffset = i32_langDataOffset + S32_LONG_RAM_READ( i32_langDataOffset + sizeof( int32_t ) * 6 );

		return &guc_selDataRAM[ i32_browserOffset ];
	}

	*pi32_Size = i32_browserSize;
	return uc_Null;
}

//----------------------------------------------------------------------------
//【関数名】 getLangCount
//
//【内  容】 言語データ数取得関数
//
//【引  数】 なし
//
//【戻り値】 言語データ数
//
//----------------------------------------------------------------------------
int32_t getLangCount()
{
	// CFが刺さっているか検出する.
	//Mosel 関数で取得するように変更する必要がある。井川
#ifdef LINUX
	if( False == isAbleMsgBin() )
#else
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();  /* 区間7 honda*** */
	if( True == pc_SystemInfo->isMediaBootType() )
#endif // #ifdef LINUX.
	{
		// 刺さっている場合.
		return 0;
	}
	int32_t i32_num = S32_LONG_RAM_READ( OFFSET_LANGCOUNT );

	if( i32_num > PNL_MAX_LANGUAGES * 2 ) {
		// エンディアンが逆など明らかにおかしいデータ.
		return 0;
	}
	if( i32_num > PNL_MAX_LANGUAGES ) {
		// 最大限語数に丸める.
		i32_num = PNL_MAX_LANGUAGES;
	}
	return i32_num;
}

//================================================================================================
// 概要		:言語タイプを取得する.
// 戻り値	:TYPE_Language を返す(ない場合は TYPE_LanguageAmericanを返す)
// 備考		:
// 作成者	:川端.
// レビュー	:
// 変更履歴	:
//================================================================================================
TYPE_Language getLanguageType( int LangNo )
{
	char szLangCode[SIZE_LANGCODE];
	if( LangNo >= getLangCount() ) {
		return TYPE_LanguageAmerican;
	}
	// CFが刺さっているか検出する.
	//Mosel 関数で取得するように変更する必要がある。井川
#ifdef LINUX
	if( False == isAbleMsgBin() )
#else
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();    /* 区間7 honda*** */
	if( True == pc_SystemInfo->isMediaBootType() )
#endif // #ifdef LINUX.
	{
		// 刺さっている場合.
		return TYPE_LanguageAmerican;
	}
	memcpy( szLangCode,
	        &guc_selDataRAM[OFFSET_1ST_LANGCODE + ( SIZE_LANGCODE * LangNo )], SIZE_LANGCODE );

	int i;
	for( i = 0; i < ( int )( sizeof( stPanelLanguageTable ) / sizeof( stPanelLanguageTable[0] ) ); i++ ) {
		if( strcmp( szLangCode, stPanelLanguageTable[i].p_Code ) == 0 ) {
			return stPanelLanguageTable[i].ec_Type;
		}
	}
	return TYPE_LanguageAmerican;
}
// 標準MSGバイナリか否か.
Bool isStandardMSGBinary()
{
	static const TYPE_Language tbl_StdLangs[PNL_MAX_LANGUAGES] = {
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
	ushort us_LangNum = (ushort)( getLangCount() );
#if defined(SIMULATOR)
	if( 0 >= us_LangNum ) { return True; }	// SimWin でMSGバイナリがない場合は、標準ROMとしておく.
#endif	// defined(SIMULATOR).
	if( PNL_MAX_LANGUAGES != us_LangNum ) { return False; }	// 搭載言語数が9言語でない場合は、標準MSGバイナリではない.
	for( i = 0; i < PNL_MAX_LANGUAGES; i++ ) {
		ec_Lang = getLanguageType( i );	// i 番目の言語データの言語タイプを取得する.
		for( j = 0; j < PNL_MAX_LANGUAGES; j++ ) {
			if( tbl_StdLangs[j] == ec_Lang ) {
				break;
			}
		}
		if( PNL_MAX_LANGUAGES == j ) {
			// 見つからなかった.
			return False;
		}
	}
	return True;
}

//////////////////////////////////////////////////////////////////////////////
//
// ◇ 外部に公開しない関数
//
//////////////////////////////////////////////////////////////////////////////

//================================================================================================
// 概要		:初期化関数.
// 戻り値	:なし
// 備考		:
// 作成者	:川端.
// レビュー	:
//================================================================================================
#if defined(DEF_Soc1200) && !defined(FUM_LINUX)
void initLanguage_pre()
{
#ifdef SIMULATOR
	if( guc_selDataRAM == NULL ) {
		setLangBuf();
	}
#endif

	strcpy( ( char* )&guc_selDataRAM[OFFSET_LANGVERSION], "Dummy Language" );

	if( bl_InitLanguage_pre == False ) {
#ifndef LINUX
		APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
#endif
		// CFが刺さっているか検出する.
		//Mosel 関数で取得するように変更する必要がある。井川
#ifdef LINUX
		if( False == isAbleMsgBin() )
#else
		if( True == pc_SystemInfo->isMediaBootType() )
#endif // #ifdef LINUX.
		{
			// 刺さっている場合.
			return;
		}
		c_SelectLangInfo.createSelectLangInfo();			// 選択言語データ構築.
		// 言語データの先頭部分(言語コード(最大6言語)まで)をコピーする.
		WriteToRAMFixedData();		// 固定データをRAMに書き込む関数.
		bl_InitLanguage_pre = True;
	}
}

void initLanguage_post()
{
	if( bl_InitLanguage_post == False ) {
		APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();

		char romVersion[ 40 ];
		unsigned short us_CheckSum = 0xAAAA;
		memset( romVersion, 0, sizeof( romVersion ) );
		// バージョン番号・オペパネメッセージデータ.
		getLanguageDataVersion( romVersion, sizeof romVersion - 1 );		// 取得.
		pc_SystemInfo->setRomVersion( TYPE_RomMFPControllerPanelMessage, romVersion );
		// チェックサム・オペパネメッセージデータ.
		// チェックサム取得.
		us_CheckSum = getLanguageDataChecksum();
		us_CheckSum = (short)( us_CheckSum + pc_SystemInfo->getOutLineFontRomCheckSum() );		// 2009/02/09 古野 CQ:315257 中韓台 ダウンロード時のアウトラインフォントのチェックサム値計算方法の変更.
		pc_SystemInfo->setRomCheckSum( TYPE_RomMFPControllerPanelMessage, us_CheckSum );
		bl_InitLanguage_post = True;
	}
}
#else //defined(DEF_Soc1200) && !defined(FUM_LINUX).
void initLanguage()
{
#ifdef SIMULATOR
	if( guc_selDataRAM == NULL ) {
		setLangBuf();
	}
#endif

	strcpy( ( char* )&guc_selDataRAM[OFFSET_LANGVERSION], "Dummy Language" );

	if( bl_InitLanguage == False ) {

		APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
		// CFが刺さっているか検出する.
		//Mosel 関数で取得するように変更する必要がある。井川
#ifdef LINUX
		if( False == isAbleMsgBin() )
#else
		if( True == pc_SystemInfo->isMediaBootType() )
#endif // #ifdef LINUX.
		{
			// 刺さっている場合.
			return;
		}
		c_SelectLangInfo.createSelectLangInfo();			// 選択言語データ構築.
		// 言語データの先頭部分(言語コード(最大6言語)まで)をコピーする.
		WriteToRAMFixedData();		// 固定データをRAMに書き込む関数.


		char romVersion[ 40 ];
		unsigned short us_CheckSum = 0xAAAA;
		memset( romVersion, 0, sizeof( romVersion ) );
		// バージョン番号・オペパネメッセージデータ.
		getLanguageDataVersion( romVersion, sizeof romVersion - 1 );		// 取得.
		pc_SystemInfo->setRomVersion( TYPE_RomMFPControllerPanelMessage, romVersion );
		// チェックサム・オペパネメッセージデータ.
		// チェックサム取得.
		us_CheckSum = getLanguageDataChecksum();
		us_CheckSum += pc_SystemInfo->getOutLineFontRomCheckSum();		// 2009/02/09 古野 CQ:315257 中韓台 ダウンロード時のアウトラインフォントのチェックサム値計算方法の変更.
		pc_SystemInfo->setRomCheckSum( TYPE_RomMFPControllerPanelMessage, us_CheckSum );
		bl_InitLanguage = True;
	}
}
#endif //defined(DEF_Soc1200) && !defined(FUM_LINUX).

//================================================================================================
// 概要		:言語データチェックサム取得関数.
// 戻り値	:unsigned short	チェックサム
// 備考		:
// 作成者	:川端.
// レビュー	:
//================================================================================================
static unsigned short getLanguageDataChecksum( void )
{
	unsigned short us_Checksum = 0xFFFF;
	// CFが刺さっているか検出する.
	//Mosel 関数で取得するように変更する必要がある。井川
#ifdef LINUX
	if( False == isAbleMsgBin() )
#else
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();  /* 区間7 honda*** */
	if( True == pc_SystemInfo->isMediaBootType() )
#endif // #ifdef LINUX.
	{
		// 刺さっている場合.
		return us_Checksum;
	}

	getLanguageDataForUINewBin( &us_Checksum, SIZE_FILEDATA_ALL-2, 2 );


	return us_Checksum;
}

#if defined(SIMULATOR)
//----------------------------------------------------------------------------
//【関数名】 IsValidCheckSum
//
//【内  容】 チェックサムの確認
//
//【引  数】 [i] puc_buffer : データが格納されているバッファの先頭アドレス
//           [i] sl_bufSize : データのサイズ[B]
//
//【戻り値】 チェックサムが正しい場合は True, それ以外は False
//
//           は 0xAAAAAAAA となっているため比較は行なわない。
//
//           データの最後から -4バイトまでを 4バイト整数で積算する。積算した
//           結果と最後の 4バイトを比較する。
//
//----------------------------------------------------------------------------
Bool IsValidCheckSum( unsigned char* puc_buffer, uint32_t ui32_bufSize )
{
	uint32_t	ui32_checkSum = 0;	// 未定義？
	uint32_t	ui32_dataSum = 0;

	// 引数の確認
	if( puc_buffer == NULL ) {
		return False;
	}

	// データサイズの確認 (最低でも 4bytes x 2 はないといけない)
	if( ui32_bufSize < ( sizeof( uint32_t ) * 2 ) ) {
		return False;
	}

	// データサイズの確認 (4整数倍でなければいけない)
	if( ui32_bufSize % sizeof( int32_t ) != 0 ) {
		return False;
	}

	// 最後から4バイト手前までの合計を求める (4バイト符号なし整数で)
	for( uint32_t n = 0; n < ( ui32_bufSize - sizeof( uint32_t ) ); n += sizeof( uint32_t ) ) {
		ui32_dataSum += U32_GETVALUE( puc_buffer, n );
	}

	// チェックサムの値 (最後の4バイト)
	ui32_checkSum = U32_GETVALUE( puc_buffer, ui32_bufSize - sizeof( uint32_t ) );

//#pragma SOMSG(★ ファイルのチェックサムが正しくなったら比較を行う ★)
	// 確認
	//

	return True;
}
#endif

//----------------------------------------------------------------------------
//【関数名】 GetFixedDataSize
//
//【内  容】 固定データサイズ取得関数
//
//【引  数】 なし
//
//【戻り値】 固定データサイズ
//
//----------------------------------------------------------------------------
static int32_t GetFixedDataSize()
{
	int32_t	i32_langCount	 = getLangCount();	// 言語数を取得
	int32_t	i32_fixedDataSize = 0;

	// 固定データサイズを計算
	i32_fixedDataSize  = sizeof( unsigned char ) * SIZE_FIXED_DATA;
	i32_fixedDataSize = (int32_t)( i32_fixedDataSize + ( sizeof( unsigned char ) * SIZE_LANGCODE * i32_langCount ) );

	return i32_fixedDataSize;
}

//----------------------------------------------------------------------------
//【関数名】 IsLanguageCodeJp
//
//【内  容】 日本語言語コード判定関数
//
//【引  数】 [o] pFirstJp  : 第 1 言語 = 日本語 判定フラグ
//			 [o] pSelectJp : 選択言語  = 日本語 判定フラグ
//			 [i] si_LangNo : 選択言語番号
//
//【戻り値】 正常に終了した場合は True, それ以外は False
//
//----------------------------------------------------------------------------
static Bool IsLanguageCodeJp( Bool* pbl_firstJp, Bool* pbl_selectJp, int si_LangNo )
{
	if( pbl_firstJp == NULL || pbl_selectJp == NULL ) {
		return False;
	}

	char	sc_firstLangCode[SIZE_LANGCODE];
	char	sc_langCode[SIZE_LANGCODE];

	// 初期化
	memset( sc_firstLangCode, 0, sizeof( sc_firstLangCode ) );
	memset( sc_langCode, 0, sizeof( sc_langCode ) );
	*pbl_firstJp  = False;
	*pbl_selectJp = False;

	// 第１言語コードをコピー
	memcpy( sc_firstLangCode,
	        &guc_selDataRAM[OFFSET_1ST_LANGCODE], SIZE_LANGCODE );
	// 選択言語コードをコピー
	memcpy( sc_langCode,
	        &guc_selDataRAM[OFFSET_1ST_LANGCODE + ( SIZE_LANGCODE * si_LangNo )], SIZE_LANGCODE );

	// 比較
	if( strcmp( sc_firstLangCode, "ja" ) == 0 ) {
		*pbl_firstJp = True;				// 第１言語が日本語
	}
	if( strcmp( sc_langCode, "ja" ) == 0 ) {
		*pbl_selectJp = True;				// 選択言語が日本語
	}

	return True;
}

//----------------------------------------------------------------------------
//【関数名】 GetLangDataOffset
//
//【内  容】 オフセット取得関数
//
//【引  数】 [o] pi32_select  : コピーするユーザー言語が格納されている言語データのオフセット格納位置
//			 [o] pi32_service : コピーするサービス言語が格納されている言語データのオフセット格納位置
//			 [i] bl_1stJp    : 第 1 言語 = 日本語 判定フラグ
//			 [i] bl_jp	     : 選択言語  = 日本語 判定フラグ
//			 [i] si_LangNo   : 選択言語番号
//
//           ※ pi32_select と pi32_service は ROM全体オフセット
//
//【戻り値】 正常に終了した場合は True, それ以外は False
//
//
//      ◇ ユーザー言語
//         常に選択された言語データのユーザー言語を使用する
//
//      ◇ サービス言語(現仕様)
//         9言語ROM対応(一括RAM展開)により、日中韓台は各サービス言語を適用、それ以外は英語を適用する。
//
//      ◇ サービス言語(旧)
//         仕向が日本のときに日本語を選択し、そうでなければ英語を選択する。
//         サービスの言語：第1言語が日本語のとき、第1言語:日本語、第2言語:英語
//                         第1言語が日本語以外のとき、第1言語:英語、第N言語:日本語(ユーザ言語と同じ位置)
//
//      ◇ サービス言語(旧々)
//         第1言語データ内のサービス言語を使用する場合
//         ・選択言語が第1の場合
//         ・選択言語が第2以降で、選択言語が日本語でない、第1言語が日本語でない場合
//
//         第2言語データ内のサービス言語を使用する場合
//         ・選択言語が第2以降の場合で、選択言語が日本語でない、第1言語が日本語の場合
//
//         選択言語データ内のサービス言語を使用する場合
//         ・上記以外の場合
//
//----------------------------------------------------------------------------
static Bool GetLangDataOffset( int32_t* pi32_select, int32_t* pi32_service, Bool bl_1stJp, Bool bl_jp, int si_LangNo )
{
	if( pi32_select == NULL || pi32_service == NULL ) {
		return False;
	}
	// 初期化
	*pi32_select  = 0;
	*pi32_service = 0;

	// 固定データサイズの取得
	// メッセージバイナリ内の固定データを取得する.
	int32_t i32_fixedSize = GetFixedDataSize_FromMSGBIN();
	i32_fixedSize += PNLD_FONTSWITCH_SIZE * c_SelectLangInfo.getMSGBINLangNum(); //フォントスイッチデータ分追加.

	//-----------------------------------------------------------------------
	// 対象のユーザー言語が入っている言語データのオフセットを取得
	//-----------------------------------------------------------------------
	*pi32_select = (int32_t)( i32_fixedSize + ( ( sizeof( int32_t ) * 2 ) * c_SelectLangInfo.getMSGBINIndex( ( uchar )si_LangNo ) ) );

	//-----------------------------------------------------------------------
	// 対象のサービス言語が入っている言語データのオフセットを取得
	//-----------------------------------------------------------------------

	// 言語数を取得.
	int32_t i32_LangNum = getLangCount();
	int i = 0;
	char	sc_langCode[SIZE_LANGCODE];

	// 選択言語コードをコピー.
	memcpy( sc_langCode,
	        &guc_selDataRAM[OFFSET_1ST_LANGCODE + ( SIZE_LANGCODE * si_LangNo )], SIZE_LANGCODE );

	// サービス言語が存在する言語は、自言語のサービス言語を適用する(サービス言語の翻訳対象で無い言語は英語).
	if( strcmp( sc_langCode, "ja" ) == 0 ) {
		// 選択言語が日本語.
		for( i = 0; i < i32_LangNum; i++ ) {
			if( getLanguageType( i ) == TYPE_LanguageJapanese ) {
				// 日本語発見!.
				*pi32_service = (int32_t)( i32_fixedSize + ( sizeof( int32_t ) * 2 ) * c_SelectLangInfo.getMSGBINIndex( ( uchar )i ) );
				return True;
			}
		}
	}
	if( strcmp( sc_langCode, "zh-CN" ) == 0 ) {
		// 選択言語が中国語.
		for( i = 0; i < i32_LangNum; i++ ) {
			if( getLanguageType( i ) == TYPE_LanguageChinese2 ) {
				// 中国語発見!.
				*pi32_service = (int32_t)( i32_fixedSize + ( sizeof( int32_t ) * 2 ) * c_SelectLangInfo.getMSGBINIndex( ( uchar )i ) );
				return True;
			}
		}
	}
	if( strcmp( sc_langCode, "zh-TW" ) == 0 ) {
		// 選択言語が台湾語.
		for( i = 0; i < i32_LangNum; i++ ) {
			if( getLanguageType( i ) == TYPE_LanguageChinese1 ) {
				// 台湾語発見!.
				*pi32_service = (int32_t)( i32_fixedSize + ( sizeof( int32_t ) * 2 ) * c_SelectLangInfo.getMSGBINIndex( ( uchar )i ) );
				return True;
			}
		}
	}
	if( strcmp( sc_langCode, "ko" ) == 0 ) {
		// 選択言語が韓国語.
		for( i = 0; i < i32_LangNum; i++ ) {
			if( getLanguageType( i ) == TYPE_LanguageHangul ) {
				// 韓国語発見!.
				*pi32_service = (int32_t)( i32_fixedSize + ( sizeof( int32_t ) * 2 ) * c_SelectLangInfo.getMSGBINIndex( ( uchar )i ) );
				return True;
			}
		}
	}
	// 日中韓台で無ければ英語を表示する.
	char	sc_firstLangCode[SIZE_LANGCODE];

	// 第１言語コードをコピー
	memcpy( sc_firstLangCode, &guc_selDataRAM[OFFSET_1ST_LANGCODE], SIZE_LANGCODE );

	if(
	    ( strcmp( sc_firstLangCode, "ja" )		== 0 ) 		// 日本語.
	    ||( strcmp( sc_firstLangCode, "zh-CN" )	== 0 ) 		// 中国語.
	    ||( strcmp( sc_firstLangCode, "ko" )		== 0 ) 		// 韓国語.
	    ||( strcmp( sc_firstLangCode, "zh-TW" )	== 0 ) 		// 台湾語.
	)
	{
		// 第1言語が日中韓台のときは第2言語(英語)を選択する.
		*pi32_service = (int32_t)( i32_fixedSize + ( sizeof( int32_t ) * 2 ) * 1 );
		return True;
	}
	else
	{
		// 第1言語が日中韓台以外のときは第1言語(英語)を選択する.
		*pi32_service = i32_fixedSize + ( sizeof( int32_t ) * 2 ) * 0;
		return True;
	}



	return True;
}

//----------------------------------------------------------------------------
//【関数名】 SetFixedDataAndLangCode
//
//【内  容】 固定データ設定関数　(先頭から言語コードまでをコピーする)
//
//【引  数】 [i] i32_LangCount : 言語データ数
//
//【戻り値】 固定データサイズ
//
//----------------------------------------------------------------------------
static int32_t SetFixedDataAndLangCode( int32_t i32_LangCount )
{
	int32_t i32_fixedDataSize = 0;

	// 固定データサイズを計算
	i32_fixedDataSize  = sizeof( unsigned char ) * SIZE_FIXED_DATA;
	i32_fixedDataSize = (int32_t)( i32_fixedDataSize + ( sizeof( unsigned char ) * SIZE_LANGCODE * i32_LangCount ) );

	// 固定データを格納
	WriteToRAMFixedData();		// 固定データをRAMに書き込む関数.

	return i32_fixedDataSize;
}

//----------------------------------------------------------------------------
//【関数名】 SetLangData
//
//【内  容】 言語データ取得設定関数
//
//【引  数】 [i] i32_dataOffset    : 対象の言語データが格納されているオフセット
//			 [i] i32_serviceOffset : サービス言語が格納されている言語データオフセット
//
//【戻り値】 言語データサイズ
//
//----------------------------------------------------------------------------
static int32_t SetLangData( int32_t i32_dataOffset, int32_t i32_serviceOffset )
{
	int32_t	i32_selectDataOffset = 0;	// 選択言語メモリ(guc_selDataRAM)上のオフセット
	int32_t	i32_langAdrsOffset	= 0;	// 言語データ内アドレス&データ長格納用オフセット
	int32_t	i32_addressData = 0;		// オフセットアドレス
	int32_t	i32_addressServ = 0;		// オフセットアドレス
	int32_t	i32_returnSize = 0;		// 戻り値用総サイズ
	int32_t	i32_addressBrow = 0;		// オフセットアドレス.

	int32_t	i32_msgAddress = 0;		// 単一言語メッセージデータ内のアドレス
	int32_t	i32_msgSize = 0;			// 単一言語メッセージデータ内のサイズ

	int32_t	i32_dataAddress   = 0;		// データ先頭アドレス
	int32_t	i32_totalDataSize = 0;		// 格納した総言語データサイズ
	int32_t*	pi32_selectLangData = NULL;	// 単一言語メッセージデータ先頭アドレス (int32_t*)
	int32_t i32_allLangData[6];				// ROM上の選択言語データの先頭アドレス
	int32_t i32_allServData[6];				// ROM上のサービス用選択言語データの先頭アドレス
	int32_t i32_allBrowData[8];				// ROM上のブラウザ用選択言語データの先頭アドレス.

	ushort* pus_DeflatedData = NULL;	// 圧縮データの一次格納先(mallocで領域確保する).
	uint32_t ui32_TmpBufSize = 0;			// 一次格納先のサイズ.
	uint32_t ui32_OutSize= 0;				// 解凍されたサイズ.
	Bool bl_decompResult = True;		// 解凍成功したか.

	// 固定データサイズ取得.
	int32_t i32_fixedDataSize = GetFixedDataSize();

	// オフセット設定.
	i32_selectDataOffset = (int32_t)( i32_fixedDataSize + ( sizeof( int32_t ) * COUNT_SELDATA_ITEMS ) );
	i32_langAdrsOffset = i32_selectDataOffset;

	int32_t i32_tempOffset = 0;

	// MFPとPrinterを判別してオフセット値を計算する.
	i32_tempOffset = i32_dataOffset + GetOffsetIfPrinter();
	if( False == IsMFPVersion() )
	{
		// Printerの時は、i32_dataOffsetにPSWCとPrinterのオフセットアドレス分が含まれているため、その分を減算しておく.
		i32_tempOffset -= SIZE_FIXED_DATA_PSWC_AND_PTR;
	}
	// 言語データのオフセット取得.
	loadLangData2Ram( &i32_addressData, i32_tempOffset, sizeof( i32_addressData ) );
	if( i32_addressData == -1 ) {	return 0;	}
	i32_tempOffset = i32_addressData + GetOffsetIfPrinter();	// MFP/Printerを判別してオフセットを取得する.
	loadLangData2Ram( i32_allLangData, i32_tempOffset, sizeof( i32_allLangData ) );

	// MFPとPrinterを判別してオフセット値を計算する.
	i32_tempOffset = i32_serviceOffset + GetOffsetIfPrinter();
	if( False == IsMFPVersion() )
	{
		// Printerの時は、i32_dataOffsetにPSWCとPrinterのオフセットアドレス分が含まれているため、その分を原産しておく.
		i32_tempOffset -= SIZE_FIXED_DATA_PSWC_AND_PTR;
	}
	// 言語データ（サービス）のオフセット取得.
	loadLangData2Ram( &i32_addressServ, i32_tempOffset, sizeof( i32_addressServ ) );
	if( i32_addressServ == -1 ) {	return 0;	}
	i32_tempOffset = i32_addressServ + GetOffsetIfPrinter();	// MFP/Printerを判別してオフセットを取得する.
	loadLangData2Ram( i32_allServData, i32_tempOffset, sizeof( i32_allServData ) );


	// ブラウザの言語データはユーザ言語と連動するため言語データのオフセットを使用する.
	// MFPとPrinterを判別してオフセット値を計算する.
	i32_tempOffset = i32_dataOffset + GetOffsetIfPrinter();
	if( False == IsMFPVersion() )
	{
		// Printerの時は、i32_dataOffsetにPSWCとPrinterのオフセットアドレス分が含まれているため、その分を減算しておく.
		i32_tempOffset -= SIZE_FIXED_DATA_PSWC_AND_PTR;
	}
	// 言語データのオフセット取得.
	// i32_addressBrow[0-1]: ユーザ言語 i32_addressBrow[2-5]: サ－ビス i32_addressBrow[6-7]: ブラウザ.
	loadLangData2Ram( &i32_addressBrow, i32_tempOffset, sizeof( i32_addressBrow ) );
	if( i32_addressBrow == -1 ) {	return 0;}
	i32_tempOffset = i32_addressBrow + GetOffsetIfPrinter();	// MFP/Printerを判別してオフセットを取得する.
	loadLangData2Ram( i32_allBrowData, i32_tempOffset, sizeof( i32_allBrowData ) );

	if( isNeedInflate() == True ) {
		// ユーザデータ、サービスデータのうち、最もサイズの大きな圧縮サイズを動的確保の容量とする.
		// 一回の動的確保で、ユーザデータ、サービスデータを全て展開したいため.
		ui32_TmpBufSize = i32_allLangData[1]; // まずユーザデータの圧縮サイズを入れる.
		//                    i32_allServDataに-1が入ることがあるので、その対応.
		if( i32_allServData[2] > 0 && ( int32_t )ui32_TmpBufSize < i32_allServData[2] ) {
			ui32_TmpBufSize = i32_allServData[2]; // サービス１データのサイズのほうが大きければサービス１データの圧縮サイズにする.
		}
		if( i32_allServData[4] > 0 && ( int32_t )ui32_TmpBufSize < i32_allServData[4] ) {
			ui32_TmpBufSize = i32_allServData[4]; // サービス２データのサイズのほうが大きければサービス２データの圧縮サイズにする.
		}
		// getLanguageDataForUI()によるデータ取得時は偶数バイト単位で取得するため、奇数バイトだった場合は1byte多く取得する必要がある.
		// 圧縮サイズが奇数バイトの場合を考慮したサイズを確保する.
		ui32_TmpBufSize = S32_WORD_SIZE( ui32_TmpBufSize );
		pus_DeflatedData = ( ushort* )malloc( ui32_TmpBufSize );

		// 領域確保できなかった場合、ゼロリターン.
		if( pus_DeflatedData == NULL )
		{
			DISPLOG( " ★★ppcglanguage malloc([%d]) 失敗!!!\n", ui32_TmpBufSize,0,0,0,0,0 );
			return 0;
		}
	}

	// 単一言語メッセージデータの先頭アドレス
	pi32_selectLangData  = ( int32_t* ) &guc_selDataRAM[i32_langAdrsOffset];

	// ユーザー言語データのコピー先アドレス
	i32_selectDataOffset = (int32_t)( i32_selectDataOffset + ( sizeof( int32_t ) * COUNT_MSGDATA_ITEMS ) );

	// コピー総サイズ
	i32_totalDataSize = (int32_t)( i32_totalDataSize + ( sizeof( int32_t ) * COUNT_MSGDATA_ITEMS ) );


	int32_t i32_OffsetROM = 0;
	//--------------------------------------------------------------------
	// ユーザー言語データをコピー
	//--------------------------------------------------------------------
	if( i32_allLangData[0] != -1 ) {

		i32_dataAddress = i32_addressData + i32_allLangData[0];
		i32_msgSize	= i32_allLangData[1];
		i32_OffsetROM = i32_dataAddress + GetOffsetIfPrinter();

		// RAM上でのユーザー言語の格納位置(単一言語メッセージデータ内オフセット)
		i32_msgAddress = sizeof( int32_t ) * COUNT_MSGDATA_ITEMS;

		if( isNeedInflate() == False ) { // 解凍不要な機種の場合.
			// コピーする領域がRAMに収まる.
			if( i32_selectDataOffset + i32_msgSize < SIZE_FILEDATA_SELECT ) {
				// コピー
				loadLangData2Ram( ( void* )&guc_selDataRAM[i32_selectDataOffset],
				                  i32_OffsetROM, i32_msgSize );

				// [単一言語メッセージデータ]にアドレスとデータ長の格納
				pi32_selectLangData[0] = i32_msgAddress;
				pi32_selectLangData[1] = i32_msgSize;

			} else {
				// アドレスとデータ長の格納.
				pi32_selectLangData[0] = i32_msgAddress;
				pi32_selectLangData[1] = 0;
			}

			// オフセットの再設定
			i32_selectDataOffset += i32_msgSize;
			i32_msgAddress += i32_msgSize;

			// コピー済サイズの更新
			i32_totalDataSize += i32_msgSize;

		} else {
			if( NULL == pus_DeflatedData )
			{
				DISPLOG( " ★★ppcglanguage malloc([%d]) 失敗!!!\n", ui32_TmpBufSize,0,0,0,0,0 );
				return 0;
			}
			memset( pus_DeflatedData, 0, ui32_TmpBufSize ); // 圧縮データの取得用領域を初期化.

			// コピー
			loadLangData2Ram( ( void* )pus_DeflatedData,	i32_OffsetROM, S32_WORD_SIZE( i32_msgSize ) );

			// 解凍
			bl_decompResult = do_decompress( pus_DeflatedData, i32_msgSize, &guc_selDataRAM[i32_selectDataOffset], (uint32_t)( SIZE_FILEDATA_SELECT - i32_selectDataOffset ), &ui32_OutSize );

			// 解凍に成功.
			if( bl_decompResult == True ) {
				// [単一言語メッセージデータ]にアドレスとデータ長の格納
				pi32_selectLangData[0] = i32_msgAddress;	// [単一言語メッセージデータ]の先頭からユーザ言語データへのオフセットアドレス.
				pi32_selectLangData[1] = ui32_OutSize;		// ユーザ言語データのデータサイズ.
			} else {
				// アドレスとデータ長の格納.
				pi32_selectLangData[0] = i32_msgAddress;
				pi32_selectLangData[1] = 0;
			}


			// オフセットの再設定
			i32_selectDataOffset += ui32_OutSize;  // [guc_selDataRAM]上のオフセット.
			i32_msgAddress += ui32_OutSize;		// [単一言語メッセージデータ]内でのユーザ言語データ終端位置.

			// コピー済サイズの更新
			i32_totalDataSize += ui32_OutSize;
		}

	} else {
		// アドレスとデータ長の格納
		pi32_selectLangData[0] = i32_allLangData[0];
		pi32_selectLangData[1] = 0;
	}

	//--------------------------------------------------------------------
	// サービス 1 言語データをコピー
	//--------------------------------------------------------------------
	if( i32_allServData[2] != -1 ) {

		i32_dataAddress = i32_addressServ + i32_allServData[2];
		i32_msgSize   = i32_allServData[3];
		i32_OffsetROM = i32_dataAddress + GetOffsetIfPrinter();

		if( isNeedInflate() == False ) {
			// コピーする領域がRAMに収まる.
			if( i32_selectDataOffset + i32_msgSize < SIZE_FILEDATA_SELECT ) {
				// コピー
				loadLangData2Ram( ( void* )&guc_selDataRAM[i32_selectDataOffset],
				                  i32_OffsetROM, i32_msgSize );

				// アドレスとデータ長の格納.
				pi32_selectLangData[2] = i32_msgAddress;
				pi32_selectLangData[3] = i32_msgSize;
			} else {
				// アドレスとデータ長の格納.
				pi32_selectLangData[2] = i32_msgAddress;
				pi32_selectLangData[3] = 0;
			}

			// オフセットの再設定.
			i32_selectDataOffset += i32_msgSize;
			i32_msgAddress		+= i32_msgSize;

			// データサイズの格納.
			i32_totalDataSize += i32_msgSize;

		} else {
			if( NULL == pus_DeflatedData )
			{
				DISPLOG( " ★★ppcglanguage malloc([%d]) 失敗!!!\n", ui32_TmpBufSize,0,0,0,0,0 );
				return 0;
			}

			// 動的に確保したメモリの初期化.
			memset( pus_DeflatedData, 0, ui32_TmpBufSize );
			// コピー
			loadLangData2Ram( ( void* )pus_DeflatedData,	i32_OffsetROM, S32_WORD_SIZE( i32_msgSize ) );
			// 解凍
			bl_decompResult = do_decompress( pus_DeflatedData, i32_msgSize, &guc_selDataRAM[i32_selectDataOffset], (uint32_t)( SIZE_FILEDATA_SELECT - i32_selectDataOffset ), &ui32_OutSize );

			// 解凍に成功.
			if( bl_decompResult == True ) {
				// アドレスとデータ長の格納.
				pi32_selectLangData[2] = i32_msgAddress;  // [単一言語メッセージデータ]の先頭からサービス１言語データへのオフセットアドレス.
				pi32_selectLangData[3] = ui32_OutSize;		// サービス１言語データの解凍後データサイズ.
			} else {
				// アドレスとデータ長の格納.
				pi32_selectLangData[2] = i32_msgAddress;
				pi32_selectLangData[3] = 0;
			}

			// オフセットの再設定.
			i32_selectDataOffset += ui32_OutSize;
			i32_msgAddress		+= ui32_OutSize;

			// データサイズの格納.
			i32_totalDataSize += ui32_OutSize;

		}

	} else {
		// アドレスとデータ長の格納.
		pi32_selectLangData[2] = i32_allServData[2];
		pi32_selectLangData[3] = 0;
	}

	//--------------------------------------------------------------------
	// サービス ２言語データをコピー
	//--------------------------------------------------------------------
	if( i32_allServData[4] != -1 ) {

		i32_dataAddress = i32_addressServ + i32_allServData[4];
		i32_msgSize     = i32_allServData[5];
		i32_OffsetROM = i32_dataAddress + GetOffsetIfPrinter();

		if( isNeedInflate() == False ) {
			// コピーする領域がRAMに収まる.
			if( i32_selectDataOffset + i32_msgSize < SIZE_FILEDATA_SELECT ) {
				// コピー.
				loadLangData2Ram( ( void* )&guc_selDataRAM[i32_selectDataOffset],
				                  i32_OffsetROM, i32_msgSize );

				// アドレスとデータ長の格納.
				pi32_selectLangData[4] = i32_msgAddress;
				pi32_selectLangData[5] = i32_msgSize;
			} else {
				// アドレスとデータ長の格納.
				pi32_selectLangData[4] = i32_msgAddress;
				pi32_selectLangData[5] = 0;
			}

			// オフセットの再設定.
			i32_selectDataOffset += i32_msgSize;
			i32_msgAddress		+= i32_msgSize;

			// データサイズの格納.
			i32_totalDataSize += i32_msgSize;

		} else { // 2006/11/29 12加藤正 [メッセージバイナリ新形式]解凍が必要な機種の場合.
			if( NULL == pus_DeflatedData )
			{
				DISPLOG( " ★★ppcglanguage malloc([%d]) 失敗!!!\n", ui32_TmpBufSize,0,0,0,0,0 );
				return 0;
			}

			// 動的に確保したメモリの初期化.
			memset( pus_DeflatedData, 0, ui32_TmpBufSize );
			// コピー
			// getLanguageDataForUI()によるデータ取得時は偶数バイト単位で取得するため、圧縮データが奇数バイトの場合は1byte多く取得する必要がある.
			loadLangData2Ram( ( void* )pus_DeflatedData,	i32_OffsetROM, S32_WORD_SIZE( i32_msgSize ) );
			// 解凍
			bl_decompResult = do_decompress( pus_DeflatedData, i32_msgSize, &guc_selDataRAM[i32_selectDataOffset], (uint32_t)( SIZE_FILEDATA_SELECT - i32_selectDataOffset ), &ui32_OutSize );

			// 解凍に成功.
			if( bl_decompResult == True ) {
				// アドレスとデータ長の格納.
				pi32_selectLangData[4] = i32_msgAddress;  // [単一言語メッセージデータ]の先頭からサービス２言語データへのオフセットアドレス.
				pi32_selectLangData[5] = ui32_OutSize;		// サービス２言語データの解凍後データサイズ.
			} else {
				// アドレスとデータ長の格納.
				pi32_selectLangData[4] = i32_msgAddress;
				pi32_selectLangData[5] = 0;
			}

			// オフセットの再設定.
			i32_selectDataOffset += ui32_OutSize;
			i32_msgAddress		+= ui32_OutSize;

			// データサイズの格納.
			i32_totalDataSize += ui32_OutSize;
		}

	} else {
		// アドレスとデータ長の格納
		pi32_selectLangData[4] = i32_allServData[4];
		pi32_selectLangData[5] = 0;
	}




	//--------------------------------------------------------------------
	// ブラウザ言語データをコピー.
	//--------------------------------------------------------------------
	// i32_addressBrow[0-1]: ユーザ言語 i32_addressBrow[2-5]: サ－ビス i32_addressBrow[6-7]: ブラウザ.
	if( i32_allBrowData[6] != -1 )		// データサイズが0xFFFFFFFFのとき.
	{
		i32_dataAddress = i32_addressBrow + i32_allBrowData[6];
		i32_msgSize     = i32_allBrowData[7];
		i32_OffsetROM = i32_dataAddress + GetOffsetIfPrinter();

		if( isNeedInflate() == False ) {
			// コピーする領域がRAMに収まる.
			if( i32_selectDataOffset + i32_msgSize < SIZE_FILEDATA_SELECT )
			{
				// コピー.
				loadLangData2Ram( ( void* )&guc_selDataRAM[i32_selectDataOffset],i32_OffsetROM, i32_msgSize );
				// アドレスとデータ長の格納.
				pi32_selectLangData[6] = i32_msgAddress;
				pi32_selectLangData[7] = i32_msgSize;
			}
			else
			{
				// アドレスとデータ長の格納.
				pi32_selectLangData[6] = i32_msgAddress;
				pi32_selectLangData[7] = 0;
			}

			// オフセットの再設定.
			i32_selectDataOffset += i32_msgSize;
			i32_msgAddress		+= i32_msgSize;
			// データサイズの格納.
			i32_totalDataSize += i32_msgSize;
		} else { // 2006/11/29 12加藤正 [メッセージバイナリ新形式]解凍が必要な機種の場合.
			if( NULL == pus_DeflatedData )
			{
				DISPLOG( " ★★ppcglanguage malloc([%d]) 失敗!!!\n", ui32_TmpBufSize,0,0,0,0,0 );
				return 0;
			}

			// 動的に確保したメモリの初期化.
			memset( pus_DeflatedData, 0, ui32_TmpBufSize );
			// コピー
			// getLanguageDataForUI()によるデータ取得時は偶数バイト単位で取得するため、圧縮データが奇数バイトの場合は1byte多く取得する必要がある.
			loadLangData2Ram( ( void* )pus_DeflatedData,	i32_OffsetROM, S32_WORD_SIZE( i32_msgSize ) );
			// 解凍
			bl_decompResult = do_decompress( pus_DeflatedData, i32_msgSize, &guc_selDataRAM[i32_selectDataOffset], (uint32_t)( SIZE_FILEDATA_SELECT - i32_selectDataOffset ), &ui32_OutSize );

			// 解凍に成功.
			if( bl_decompResult == True ) {
				// アドレスとデータ長の格納.
				pi32_selectLangData[6] = i32_msgAddress;  // [単一言語メッセージデータ]の先頭からブラウザデータへのオフセットアドレス.
				pi32_selectLangData[7] = ui32_OutSize;		// ブラウザデータの解凍後データサイズ.
			} else {
				// アドレスとデータ長の格納.
				pi32_selectLangData[6] = i32_msgAddress;
				pi32_selectLangData[7] = 0;
			}

			// オフセットの再設定.
			i32_selectDataOffset += ui32_OutSize;
			i32_msgAddress		+= ui32_OutSize;

			// データサイズの格納.
			i32_totalDataSize += ui32_OutSize;
		}

	}
	else
	{
		// アドレスとデータ長の格納.
		pi32_selectLangData[6] = 0;
		pi32_selectLangData[7] = 0;
	}

	// 解凍に使用したバッファ領域を開放.
	free( pus_DeflatedData );

	//--------------------------------------------------------------------
	// 言語データのアドレスとデータ長の格納
	//--------------------------------------------------------------------
	S32_LONG_RAM( i32_fixedDataSize )	= (int32_t)( i32_fixedDataSize + ( sizeof( int32_t ) * COUNT_SELDATA_ITEMS ) );
	S32_LONG_RAM( i32_fixedDataSize + sizeof( int32_t ) ) = i32_totalDataSize;

	// 戻り値設定
	i32_returnSize = i32_totalDataSize;


	//		サービス 1 言語データと サービス 2 言語データが逆に
	//		入れてしまったため, アドレスとデータ長を入れ替えることで対応
	//
	int32_t	i32_addrTemp;
	int32_t	i32_sizeTemp;

	i32_addrTemp = pi32_selectLangData[2];
	i32_sizeTemp = pi32_selectLangData[3];
	pi32_selectLangData[2] = pi32_selectLangData[4];
	pi32_selectLangData[3] = pi32_selectLangData[5];
	pi32_selectLangData[4] = i32_addrTemp;
	pi32_selectLangData[5] = i32_sizeTemp;

	return i32_returnSize;
}

//----------------------------------------------------------------------------
//【関数名】 SetLangFontData
//
//【内  容】 フォントデータ取得設定関数
//
//【引  数】 [i] nLangNo	   : 選択言語番号
//			 [i] i32_storedSize : フォントデータまでに格納したデータサイズ
//
//【戻り値】 フォントデータサイズ
//
//----------------------------------------------------------------------------
static int32_t SetLangFontData( int si_LangNo, int32_t i32_storedSize )
{
	int32_t	i32_selectDataOffset = 0;				// 選択メモリ格納用オフセット
	int32_t	i32_langAdrsOffset	= 0;				// アドレス&データ長格納用オフセット
	int32_t	i32_fixedDataSize = GetFixedDataSize();	// 固定データサイズ取得

	int32_t	i32_fontOffset = 0;
	int32_t	i32_address	= 0;
	int32_t	i32_dataSize	  = 0;

	ushort* pus_DeflatedData = NULL;	// 圧縮データの一次格納先(mallocで領域確保する).
	uint32_t ui32_TmpBufSize = 0;		// 一次格納先のサイズ.
	uint32_t ui32_OutSize= 0;				// 解凍されたサイズ.
	Bool bl_decompResult = True;		// 解凍成功したか.

	Bool bExtendedMemory = False;		//DonauLastFum【ネットワーク通信文字拡張】【13東阪】2009/04/30 Add.
	unsigned char* puc_Font = NULL;		//DonauLastFum【ネットワーク通信文字拡張】【13東阪】2009/04/30 Add.


	// メッセージバイナリ内の固定データ、言語順を使用する.
	i32_fontOffset  = GetFixedDataSize_FromMSGBIN();														// 言語データ先頭
	i32_fontOffset += PNLD_FONTSWITCH_SIZE * c_SelectLangInfo.getMSGBINLangNum(); 						//フォントスイッチデータ.
	i32_fontOffset += (int32_t)( ( sizeof( int32_t ) * 2 ) * c_SelectLangInfo.getMSGBINLangNum() );						// フォントデータの先頭
	i32_fontOffset += (int32_t)( ( sizeof( int32_t ) * 2 ) * ( int32_t )( c_SelectLangInfo.getMSGBINIndex( ( uchar )si_LangNo ) ) );	// 指定言語のフォントデータ

	// 選択メモリ格納位置
	i32_selectDataOffset = (int32_t)( i32_storedSize + ( sizeof( int32_t ) * COUNT_SELDATA_ITEMS ) ) ;

	// アドレスの格納位置
	i32_langAdrsOffset = (int32_t)( i32_fixedDataSize + ( sizeof( int32_t ) * 2 ) );

	int32_t i32_OffsetROM = 0;

	// アドレスとデータサイズの取得
	i32_OffsetROM = i32_fontOffset + GetOffsetIfPrinter();
	if( False == IsMFPVersion() )
	{
		// Printerの時は、i32_dataOffsetにPSWCとPrinterのオフセットアドレス分が含まれているため、その分を原産しておく.
		i32_OffsetROM -= SIZE_FIXED_DATA_PSWC_AND_PTR;
	}
	loadLangData2Ram( &i32_address, i32_OffsetROM, sizeof( i32_address ) );
	loadLangData2Ram( &i32_dataSize, (int32_t)(i32_OffsetROM + sizeof( int32_t ) ), sizeof( i32_dataSize ) );

	if( i32_address == -1 ) {
		return 0;
	}

// comment 2014/12/08 【HCL-Nagaraj】【CQ-506924】  Klockwork Changes ID:30682 [SV.TAINTED.ALLOC_SIZE].
	if( 0 >= i32_dataSize ){
		return 0;
	}
//	memcpy( guc_selDataRAM + sl_selectDataOffset, 
//			guc_allDataROM + sl_address, sl_dataSize );
// comment 2014/02/04 OPS1-Sky大嶋 [ZeusCD][制御構成変更1332] 中国語ビットマップフォントデータ領域の拡大.
////CQ371786 s.
////	memset(&guc_extFontData[0], 0xFF, sizeof(guc_extFontData));	// フォントデータを 0xFF で埋め尽くす.
//	memset(guc_extFontData, 0xFF, (SIZE_FONTDATA_SELECT));	// フォントデータを 0xFF で埋め尽くす.
////CQ371786 e.
	memset(guc_extFontData, 0xFF, (PNLD_Size_FontBuf_Max));	// フォントデータを 0xFF で埋め尽くす.

	if( isNeedInflate() == False ) {
		loadLangData2Ram( guc_extFontData, i32_address, i32_dataSize );

		// アドレスとデータ長の格納
		S32_LONG_RAM( i32_langAdrsOffset ) = i32_selectDataOffset;
		S32_LONG_RAM( i32_langAdrsOffset + sizeof( int32_t ) ) = i32_dataSize;
	} else {

		// 動的確保の容量.
		// getLanguageDataForUI()によるデータ取得時は偶数バイト単位で取得するため、奇数バイトだった場合は1byte多く取得する必要がある.
		// 圧縮サイズが奇数バイトの場合を考慮し、4byte多めに確保する.(3byteは念のため)
		ui32_TmpBufSize = (uint32_t)( i32_dataSize + sizeof( int32_t ) );
		pus_DeflatedData = ( ushort* )malloc( ui32_TmpBufSize );

		// 領域確保できなかった場合、ゼロリターン.
		if( pus_DeflatedData == NULL )
		{
			DISPLOG( "language malloc([%d]) 失敗\n", ui32_TmpBufSize,0,0,0,0,0 );
			return 0;
		}

		memset( pus_DeflatedData, 0, ui32_TmpBufSize );


		TYPE_Language eLang = TYPE_LanguageAmerican;
		Bool bMemErr = False;


		// 増設メモリ有無を判定する.
// 中韓台のフォントを常時ロードさせておかないと.
// changeExternalText等で表示させる文字が化けてしまう為、拡張メモリ有り状態と同じメモリ確保&フォントロード処理を行う.
		bExtendedMemory = True;
		if( bExtendedMemory != False ) {

			// ターゲットの言語を判定する.
			uchar**	ppuc_ExtFontData	= NULL;
			uint32_t	ui32_ExtFontDataByte	= 0;
			eLang = getLanguageType( si_LangNo );

			switch( eLang ) {
				case TYPE_LanguageChinese1:
					ppuc_ExtFontData	= &gpuc_extFontData[PNLD_BufId_Big5];
					ui32_ExtFontDataByte	= PNLD_Size_FontBuf_Big5;
					break;

				case TYPE_LanguageChinese2:
					ppuc_ExtFontData	= &gpuc_extFontData[PNLD_BufId_GB18030];
					ui32_ExtFontDataByte	= PNLD_Size_FontBuf_GB18030;
					break;

				case TYPE_LanguageHangul:
					ppuc_ExtFontData	= &gpuc_extFontData[PNLD_BufId_KS];
					ui32_ExtFontDataByte	= PNLD_Size_FontBuf_KS;
					break;

				default: break;
			}

			if( NULL == ppuc_ExtFontData ) {
				bMemErr = True;
			} else if( NULL == *ppuc_ExtFontData ) {
				// メモリ確保は1回のみとする.
				*ppuc_ExtFontData = ( uchar* )malloc( ui32_ExtFontDataByte );
				if( NULL == *ppuc_ExtFontData ) {
					bMemErr = True;
				} else {
					// フォントデータを 0xFF で埋め尽くす.
					memset( *ppuc_ExtFontData, 0xFF, ui32_ExtFontDataByte );
				}
			}


			// メモリ確保が確保できたか確認する.
			if( bMemErr != False ) {
				DISPLOG( "language malloc([%d]) failure. \n", ui32_ExtFontDataByte,0,0,0,0,0 );
				free( pus_DeflatedData );
				return 0;
			}
		}



		// コピー
		// getLanguageDataForUI()によるデータ取得時は偶数バイト単位で取得するため、圧縮データが奇数バイトの場合は1byte多く取得する必要がある.
		loadLangData2Ram( ( void* )pus_DeflatedData, i32_address, S32_WORD_SIZE( i32_dataSize ) );
		// 解凍


		uint32_t ui32_ExtFontDataSize = 0;
		puc_Font = guc_extFontData;
		if( bExtendedMemory != False ) {
			if( eLang == TYPE_LanguageChinese1 ) {
				puc_Font = gpuc_extFontData[PNLD_BufId_Big5];
				ui32_ExtFontDataSize = PNLD_Size_FontBuf_Big5;
			} else if( eLang == TYPE_LanguageChinese2 ) {
				puc_Font = gpuc_extFontData[PNLD_BufId_GB18030];
				ui32_ExtFontDataSize = PNLD_Size_FontBuf_GB18030;
			} else if( eLang == TYPE_LanguageHangul ) {
				puc_Font = gpuc_extFontData[PNLD_BufId_KS];
				ui32_ExtFontDataSize = PNLD_Size_FontBuf_KS;
			}
		}
		bl_decompResult = do_decompress( pus_DeflatedData, i32_dataSize, puc_Font, ui32_ExtFontDataSize, &ui32_OutSize );
		if( puc_Font != guc_extFontData ) {
			memcpy( guc_extFontData, puc_Font, ui32_ExtFontDataSize );		// 台中韓のフォントデータを共有メモリに設定.
		}
		logMsg( "language Font Inflate 解凍サイズ=[%d]byte\n", ui32_OutSize,0,0,0,0,0 );

		// 解凍に失敗.
		if( bl_decompResult == False ) {
			DISPLOG( "language Font Inflate Error 解凍サイズ=[%d]byte\n", ui32_OutSize,0,0,0,0,0 );
		}
	}

	// 解凍に使用したバッファ領域を開放.
	free( pus_DeflatedData );

	const uchar** puc_fonttbl = NULL;
	int si_FontNum = 0;					// 1バンクあたりの文字数.
	switch( getLanguageType( si_LangNo ) ) {
		case TYPE_LanguageChinese1:	// 台湾.
			puc_fonttbl = fonthead0b;
			si_FontNum = 0xFF - 0x40 - 0x20;
			break;
		case TYPE_LanguageChinese2:	// 中国語.
			puc_fonttbl = fonthead0c;
			si_FontNum = 0xFF - 0x40 - 0x01;
			break;
		case TYPE_LanguageHangul:	// 韓国語.
			puc_fonttbl = fonthead11;
			si_FontNum = 0xFF - 0xA0;
			break;
		default:
			break;
	}
	int i;
	if( puc_fonttbl != NULL ) {
		if( puc_fonttbl == fonthead11 ) {
			if( False != bExtendedMemory ) {

				// 韓国語は1バイト目が A1-AC、B0-C8.
				for( i = 0; i < 0xAC - 0xA1 + 1; i++ ) {
					if( static_cast<UINT>( i * si_FontNum * PNLD_ByteOf1FontImg ) > PNLD_Size_FontBuf_KS ) { break; }
					puc_fonttbl[i] = &puc_Font[i * si_FontNum * PNLD_ByteOf1FontImg];
				}
				for( i = ( int )( 0xAC-0xA1+1 ); i < ( int )( 0xC8-0xA1+1 ); i++ ) {
					if( static_cast<UINT>( ( i - ( 0xB0- 0xAC-1 ) ) * si_FontNum * PNLD_ByteOf1FontImg ) > PNLD_Size_FontBuf_KS ) { break; }
					puc_fonttbl[i] = &puc_Font[( i - ( 0xB0- 0xAC-1 ) ) * si_FontNum * PNLD_ByteOf1FontImg];
				}
			} else {

				// 韓国語は1バイト目が A1-AC、B0-C8.
				for( i = 0; i < 0xAC - 0xA1 + 1; i++ ) {
					if( static_cast<UINT>( i * si_FontNum * PNLD_ByteOf1FontImg ) > PNLD_Size_FontBuf_Max ) { break; }
					puc_fonttbl[i] = guc_extFontData + ( i * si_FontNum * PNLD_ByteOf1FontImg );
				}
				for( i = ( int )( 0xAC-0xA1+1 ); i < ( int )( 0xC8-0xA1+1 ); i++ ) {	// No325349対応.
					if( i > FontHeadSize_CKT || static_cast<UINT>( ( i - ( 0xB0- 0xAC-1 ) ) * si_FontNum * PNLD_ByteOf1FontImg ) > PNLD_Size_FontBuf_Max ) {	// No325349対応.
						LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
						break;
					}
					puc_fonttbl[i] = guc_extFontData + ( ( i - ( 0xB0- 0xAC-1 ) ) * si_FontNum * PNLD_ByteOf1FontImg );
				}
			}			//DonauLastFum【ネットワーク通信文字拡張】【13東阪】2009/04/30 Add.
		} else if( puc_fonttbl == fonthead0c ) {
			// 中国.
			uchar*	puc_LastBlock	= NULL;
			uint32_t	ui32_BuffSize		= 0;
			uint32_t	ui32_UseBytes		= 0;

			if( False != bExtendedMemory ) {
				puc_LastBlock	= puc_Font;
				ui32_BuffSize		= PNLD_Size_FontBuf_GB18030;
			} else {
				puc_LastBlock	= guc_extFontData;
				ui32_BuffSize		= sizeof( guc_extFontData );
			}

			for( i = 0; i < FontHeadSize_CKT; i++ ) {
				if( ui32_UseBytes > ui32_BuffSize ) {
					break;
				}

				if( i <= ( PNLD_GB18030_2Byte_1e - PNLD_GB18030_2Byte_1s ) ) {
					// 配列前半データは、2byteコードを配置.
					si_FontNum = PNLD_GB18030_2ByteCnt_2;
				} else {
					// 配列後半データは、4byteコードを配置.
					si_FontNum = PNLD_GB18030_4ByteCnt_3 * PNLD_GB18030_4ByteCnt_4;
				}

				puc_fonttbl[i]	= puc_LastBlock;
				puc_LastBlock	+= ( si_FontNum * PNLD_ByteOf1FontImg );
				ui32_UseBytes		+= ( si_FontNum * PNLD_ByteOf1FontImg );
			}
		} else {
			// 台湾.
			if( False != bExtendedMemory ) {			// Donau拡張機能3、Amur機能拡張2以降.

				for( i = 0; i < FontHeadSize_CKT; i++ ) {
					if( static_cast<UINT>( i * si_FontNum * PNLD_ByteOf1FontImg ) > PNLD_Size_FontBuf_Big5 ) { break; }
					puc_fonttbl[i] = &puc_Font[i * si_FontNum * PNLD_ByteOf1FontImg];
				}
			} else {
				for( i = 0; i < FontHeadSize_CKT; i++ ) {
					if( static_cast<UINT>( i * si_FontNum * PNLD_ByteOf1FontImg ) > PNLD_Size_FontBuf_Max ) { break; }
					puc_fonttbl[i] = guc_extFontData + ( i * si_FontNum * PNLD_ByteOf1FontImg );
				}
			}	// //DonauLastFum【ネットワーク通信文字拡張】【13東阪】2009/04/30 Add.
		}
	}
	return i32_dataSize;
}
//----------------------------------------------------------------------------.
//【関数名】 loadSelectFont.
//.
//【内  容】 選択フォントデータ再構築関数.
//.
//【引  数】 [i] LangNo  : 言語番号-1 (0～言語数-1).
//.
//【戻り値】 正常に終了した場合は 0, それ以外は -1.
//.
//----------------------------------------------------------------------------.
int loadSelectFont( int LangNo )
{
	// 引数のエラーチェック.
	int32_t i32_langCount = getLangCount();	// 言語データ数取得.
	if( LangNo < 0 || LangNo >= i32_langCount ) {
		return -1;
	}

	int32_t i32_storedDataSize = 0;	// 格納したデータサイズ.

	// 先頭から(バージョン情報) から言語コードまでのサイズを格納.
	i32_storedDataSize = GetFixedDataSize();

	// 選択言語データサイズを格納.
	int32_t i32_size = S32_LONG_RAM_READ( i32_storedDataSize + sizeof( int32_t ) );
	if( i32_size <= 0 ) {
		return -1;
	}
	i32_storedDataSize += i32_size;

	// 選択言語フォントデータ格納.
	i32_storedDataSize += SetLangFontData( LangNo, i32_storedDataSize );

	// Panel言語終端アドレスを格納.
	SetTerminalAddress( i32_storedDataSize );

	return 0;
}


//----------------------------------------------------------------------------
//【関数名】 SetTerminalAddress
//
//【内  容】 終端アドレス格納関数
//
//【引  数】 [i] i32_storedSize : 終端までに格納したデータサイズ
//
//【戻り値】 なし
//
//----------------------------------------------------------------------------
void SetTerminalAddress( int32_t i32_storedSize )
{
	int32_t	i32_langAdrsOffset = 0;				// アドレス格納用オフセット
	int32_t	i32_totalDataSize  = 0;				// 格納したデータの総サイズ

	int32_t	i32_fixedDataSize  = GetFixedDataSize();	// 固定データサイズ取得

	// 総サイズの計算
	i32_totalDataSize = (int32_t)( i32_storedSize + ( sizeof( int32_t ) * COUNT_SELDATA_ITEMS ) );

	// アドレスの格納位置
	i32_langAdrsOffset = (int32_t)( i32_fixedDataSize + ( sizeof( int32_t ) * ( COUNT_SELDATA_ITEMS - 2 ) ) );	// 終端アドレスが8バイトになったため、-2とする.

	// アドレスの格納
	*((intptr_t*)&guc_selDataRAM[i32_langAdrsOffset]) = ( intptr_t )guc_selDataRAM + i32_totalDataSize;
}



//----------------------------------------------------------------------------
//【関数名】 isNeedInflate
//
//【内  容】 メッセージデータが圧縮されているかどうかの判断処理.
//           機種により、判定をする.
//
//【引  数】 なし
//
//【戻り値】 True ：圧縮されている.
//           False：圧縮されていない.
//
//----------------------------------------------------------------------------
Bool isNeedInflate()
{
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	TYPE_MachineType e_MachineType = pc_SystemInfo->getMachineType();

	// 製品タイプから、メッセージデータが圧縮されているかを返す.
	for( uint i = 0; i < sizeof( stPanelNeedInflateCheckTable )/sizeof( stPanelNeedInflateCheckTable[0] ); i++ ) {
		if( stPanelNeedInflateCheckTable[i].e_MachineType == e_MachineType ) {
			return stPanelNeedInflateCheckTable[i].bl_MessageDeflated;
		}
	}

	return True;
}

#endif // DIV_MFP_LIB
//----------------------------------------------------------------------------
//【関数名】 do_decompress
//
//【内  容】 圧縮されている言語データの解凍を行う.
//
//【引  数】
//           [i] pSrcBuf     : 圧縮データ.
//			 [i] ui32_SrcBufSize  : 圧縮データのサイズ
//           [i] pOutBuf     : 解凍データ格納先.
//           [i] ui32_OutBufSize : 格納先の容量.
//			 [o] ui32_OutSize  : 解凍したデータサイズ
//
//【戻り値】 True ：正常に解凍できた
//           False：解凍に失敗した.
//
//----------------------------------------------------------------------------
Bool do_decompress( unsigned short* pSrcBuf, uint32_t ui32_SrcBufSize,
                    unsigned char* pOutBuf, uint32_t ui32_OutBufSize, uint32_t* ui32_OutSize )
{
	Bool bl_result = True;

	z_stream	st_ZLIB; // zlibに渡す用.

	// 構造体の初期化を行う。
	st_ZLIB.zalloc	= Z_NULL;
	st_ZLIB.zfree	= Z_NULL;
	st_ZLIB.opaque	= Z_NULL;
	st_ZLIB.msg		= Z_NULL;
	// 解凍処理の前の初期化処理.
	if( inflateInit( &st_ZLIB ) != Z_OK ) {
		printf( "language inflateInit: %s\n", ( ( st_ZLIB.msg ) ? st_ZLIB.msg : "???" ) );
		return False;
	}

	st_ZLIB.next_in   = ( unsigned char* )pSrcBuf;	// 入力バッファポインタ.
	st_ZLIB.avail_in  = ui32_SrcBufSize;				// 入力データサイズ.
	st_ZLIB.next_out  = ( uchar* )pOutBuf;			// 出力バッファポインタ.
	st_ZLIB.avail_out = ui32_OutBufSize;				// 出力データサイズ.

	// inflate関数を呼び出して、解凍処理を行なう.
	int status = Z_OK;

	while( status != Z_STREAM_END ) {
		status = inflate( &st_ZLIB, Z_SYNC_FLUSH );

		if( status == Z_STREAM_END ) {
			break; /* 完了 */
		}

		if( status != Z_OK ) {  /* エラー */
			if( status == Z_STREAM_ERROR ) {
				DISPLOG( "Stream Error\n", 0,0,0,0,0,0 );
			} else if( status == Z_DATA_ERROR ) {
				DISPLOG( "Data Error\n", 0,0,0,0,0,0 );
			} else if( status == Z_MEM_ERROR ) {
				DISPLOG( "Memory Error\n", 0,0,0,0,0,0 );
			} else if( status == Z_BUF_ERROR ) {
				DISPLOG( "Buffer Error\n", 0,0,0,0,0,0 );
			} else {
				DISPLOG( "Other Error\n", 0,0,0,0,0,0 );
			}
			if( st_ZLIB.msg ) {
				DISPLOG( "language inflate: %s\n", ( intptr_t )( st_ZLIB.msg ),0,0,0,0,0 );
			} else {
				DISPLOG( "language inflate: Inflate Unknown Error\n", 0,0,0,0,0,0 );
			}
			bl_result = False;
			break;
		}
	}

	// 解凍したサイズを格納.
	*ui32_OutSize = st_ZLIB.total_out;

	if( st_ZLIB.avail_in != 0 ) { // 圧縮データが残っている場合.
		DISPLOG( "Can't Inflate All Data\n", 0,0,0,0,0,0 );
		bl_result = False;
	}

	// 後始末.
	if( inflateEnd( &st_ZLIB ) != Z_OK ) {
		if( st_ZLIB.msg ) {
			DISPLOG( "language inflate: %s\n", ( intptr_t )( st_ZLIB.msg ),0,0,0,0,0 );
		} else {
			DISPLOG( "language inflate: Inflate Unknown Error\n", 0,0,0,0,0,0 );
		}
	}

	return bl_result;
}
#ifndef DIV_MFP_LIB

//----------------------------------------------------------------------------
//【関数名】 IsMFPVersion
//
//【内  容】 MFP Version可否判定関数.
//
//【引  数】 なし.
//
//【戻り値】 MFP Versionの可否.
//           True : MFP Version.
//           False : Printer Version.
//
//----------------------------------------------------------------------------
static Bool IsMFPVersion()
{
	Bool	bl_IsMfp = False;

#if defined(DEF_Soc1200) && !defined(FUM_LINUX)
	if( True == IsScanerExist() )
	{
		bl_IsMfp = True; // MFPバージョン.
	}
	else
	{
		// WARP後のみ、治具パネルモードを判定する.
		if( TRUE == b_isMsgDataLoaded_pre ) {
			APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
			APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();	// DipSw（API）・インスタンス取得.

			// Printerバージョンの場合で冶具パネルモードの場合はMFPバージョンとして返す.
			// 冶具パネルモードは、「トラブルリセットモード」および「ソフトウェアＳＷ62番が１」のとき.
			if( True == pc_SystemInfo->isExtraMode()
			    && True == pc_SoftDipSw->getSoftDipSwBit( 4, 3 )
			  ) {
				bl_IsMfp = True; // MFPバージョン.
			}
			else
			{
				bl_IsMfp = False; // Pバージョン.
			}
		} else {
			bl_IsMfp = False; // Pバージョン.
		}
	}
#else //defined(DEF_Soc1200) && !defined(FUM_LINUX).
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();	// DipSw（API）・インスタンス取得.

	if( True == pc_SystemInfo->isScanerExist() )
	{
		bl_IsMfp = True; // MFPバージョン.
	}
	else
	{
		// Printerバージョンの場合で冶具パネルモードの場合はMFPバージョンとして返す.
		// 冶具パネルモードは、「トラブルリセットモード」および「ソフトウェアＳＷ62番が１」のとき.
		if( True == pc_SystemInfo->isExtraMode()
		    && True == pc_SoftDipSw->getSoftDipSwBit( 4, 3 )
		  ) {
			bl_IsMfp = True; // MFPバージョン.
		}
		else
		{
			bl_IsMfp = False; // Pバージョン.
		}
	}
#endif //defined(DEF_Soc1200) && !defined(FUM_LINUX).
//Comment 2018/03/26 [HCLT-Premkumar][IT6_Sparrow][AR1042298]Returning True for MFP and Printer models.
//As per message binary specification change, from IT3 onwards, printer portion is not used in message binary.
//Message binary structure is common for both MFP and Printer models. Code is changed from IT6 onwards. 
//IsMFPVersion( ) function will always return True irrespective of the model.
//	return bl_IsMfp;
	return True;
}

#if defined(DEF_Soc1200) && !defined(FUM_LINUX)
//----------------------------------------------------------------------------
//【関数名】 IsJigPanelMode
//
//【内  容】 治具パネルモード可否判定関数.
//
//【引  数】 なし.
//
//【戻り値】 治具パネルモードの可否.
//           True : 治具パネルモード.
//           False : 治具パネルモード以外.
//
//----------------------------------------------------------------------------
static Bool IsJigPanelMode()
{
	Bool	bl_IsJig = False;

	if( True == IsScanerExist() ) {
		bl_IsJig = False; // MFPバージョン.
	} else {
		APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
		APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();	// DipSw（API）・インスタンス取得.

		// 冶具パネルモードは、「トラブルリセットモード」および「ソフトウェアＳＷ62番が１」のとき.
		if( True == pc_SystemInfo->isExtraMode()
		    && True == pc_SoftDipSw->getSoftDipSwBit( 4, 3 )
		  ) {
			bl_IsJig = True; // 治具パネルモード.
		}
		else
		{
			bl_IsJig = False; // Printerバージョン.
		}
	}

	return bl_IsJig;
}

//----------------------------------------------------------------------------
//【関数名】 IsScanerExist
//
//【内  容】 スキャナー有無判定関数.
//
//【引  数】 なし.
//
//【戻り値】 スキャナーの有無.
//           True : スキャナー有.
//           False : スキャナー無.
//
//----------------------------------------------------------------------------
static Bool IsScanerExist()
{
	Bool	bl_IsScaner = False;

	if( TRUE == b_isMsgDataLoaded_pre ) {
		APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
		bl_IsScaner = pc_SystemInfo->isScanerExist();
	} else {
		bl_IsScaner = RequestBeforeWarp::isScanerExist();
	}

	return bl_IsScaner;
}
#endif //defined(DEF_Soc1200) && !defined(FUM_LINUX).

//----------------------------------------------------------------------------
//【関数名】 GetPrinterOffsetAddress
//
//【内  容】 Printer用パネル言語データへのオフセット取得関数.
//
//【引  数】 なし.
//
//【戻り値】 オフセットアドレス.
//
//----------------------------------------------------------------------------
static int32_t GetPrinterOffsetAddress()
{
	int32_t	i32_offsetAddress = 0;

	// Printerオフセットアドレスを取得.
	memcpy( &i32_offsetAddress, &guc_selDataRAM[SIZE_OFFSET_ADDRESS_PRT], sizeof( i32_offsetAddress ) );

	return i32_offsetAddress;
}
//----------------------------------------------------------------------------
//【関数名】 WriteToRAMFixedData
//
//【内  容】 固定データをRAMに書き込む関数.
//
//【引  数】 なし.
//
//【戻り値】 なし.
//
//----------------------------------------------------------------------------
static void WriteToRAMFixedData()
{


	// PSWCオフセット＋Printerデータオフセット＋バージョン情報＋仕向けコードをRAMに展開.
	loadLangData2Ram( guc_selDataRAM, 0, OFFSET_LANGCOUNT );

	// 言語数をコピー.
	int32_t i32_LangNum = 0;		// 言語数.
	i32_LangNum = ( int32_t )c_SelectLangInfo.getSelectLangNum();
	memcpy( ( guc_selDataRAM + OFFSET_LANGCOUNT ), &( i32_LangNum ), sizeof( i32_LangNum ) );
	// 選択言語の言語コードを順に加えていく.
	for( uchar i=0; i < c_SelectLangInfo.getSelectLangNum(); i++ ) {
		char* pc_LangCode = c_SelectLangInfo.getLangCode( i );
		if( NULL != pc_LangCode ) {
			memcpy( ( guc_selDataRAM + OFFSET_1ST_LANGCODE + SIZE_LANGCODE*i ),
		        pc_LangCode, SIZE_LANGCODE );
		}
	}
	//フォントスイッチの初期処理(初期値は0(FONTSWITCH_DEFAULT)).
	memset( ui32_FontSwitch,PNLD_FONTSWITCH_DEFAULT,sizeof( ui32_FontSwitch ) );
	//フォントスイッチの取得.
	loadLangData2Ram( &ui32_FontSwitch, SIZE_FIXED_DATA+( SIZE_LANGCODE*i32_LangNum ), PNLD_FONTSWITCH_SIZE*i32_LangNum );	//2番目の引数はフォントスイッチの先頭OFFSET位置,3番目の引数は読み込みサイズ.

}
//----------------------------------------------------------------------------
//【関数名】 GetOffsetIfPrinter
//
//【内  容】 MFP/Printerを判別してオフセットを取得する.
//
//【引  数】 なし.
//
//【戻り値】 オフセット値.
//
//----------------------------------------------------------------------------
static int32_t GetOffsetIfPrinter()
{
	int32_t i32_Offset = 0;

	if( IsMFPVersion() == False )
	{
		// Printer Ver.の時.
		i32_Offset = GetPrinterOffsetAddress();
	}
	return i32_Offset;
}


#if defined(SIMULATOR)
//----------------------------------------------------------------------------
//【関数名】 setPuiSimOnForLang
//
//【内  容】 SIM_PUI の起動時にフラグをＯＮにする.
//
//【引  数】 [i] bl_PuiSimOn : SIM_PUIが起動された.
//
//【戻り値】 なし.
//
//【備  考】
//
//----------------------------------------------------------------------------
void setPuiSimOnForLang( Bool bl_PuiSimOn )
{
	bl_SimPinterVersion = bl_PuiSimOn;
}

Bool isPuiSimOnForLang()
{
	return bl_SimPinterVersion;
}
#endif

//----------------------------------------------------------------------------.
//【関数名】 GetLanguageCodeOcr.
//
//【内  容】 言語コード獲得関数.
//
//【引  数】 [i] si_LangNo : 選択言語番号.
//
//【戻り値】 言語コード(TYPE_LanguageOcr).
//
//----------------------------------------------------------------------------.
static TYPE_LanguageOcr GetLanguageCodeOcr( int si_LangNo )
{
	char	sc_langCode[SIZE_LANGCODE];

	// 初期化.
	memset( sc_langCode, 0, sizeof( sc_langCode ) );

	// 選択言語コードをコピー.
	memcpy( sc_langCode,
	        &guc_selDataRAM[OFFSET_1ST_LANGCODE + ( SIZE_LANGCODE * si_LangNo )], SIZE_LANGCODE );

	// 比較.
	for( short i = 0; i < ( short )( sizeof( stPanelLanguageOcrTable ) / sizeof( stPanelLanguageOcrTable[0] ) ); i++ ) {
		if( 0 == strcmp( stPanelLanguageOcrTable[i].p_Code, sc_langCode ) ) {
			return stPanelLanguageOcrTable[i].ec_Type;
		}
	}

	// 該当なし.
	return TYPE_LanguageOcrNone;
}

#ifdef LINUX
//================================================================================================
// 概要		:メッセージバイナリ存在有無を返す.
// 戻り値	:True : 有　False : 無.
//================================================================================================
Bool isAbleMsgBin()
{
	static Bool b_initAbleMsgBin = False;
	static Bool b_AbleMsgBin = False;
	FILE*	fp = NULL;
	if( False == b_initAbleMsgBin ) {	// 最初の一回のみ言語データ有無確認を行う.
// MessagePathを取得していない場合、取得する.
		if( '\0' == sc_MessgeBinPath[0] ) {
			memset( sc_MessgeBinPath, 0, sizeof( sc_MessgeBinPath ) );
			THRF_getMsgBinName( sc_MessgeBinPath, sizeof( sc_MessgeBinPath ) );
		}

		if( NULL != ( fp = fopen( sc_MessgeBinPath,"r" ) ) ) {	// 言語データ有り.
			b_AbleMsgBin = True;
			DISPLOG( "isAbleMsgBin open ok LINUX_MSG_BIN \n", 0,0,0,0,0,0 );
			fclose( fp );
		} else {	// 言語データ無し.
			b_AbleMsgBin = False;
			DISPLOG( "isAbleMsgBin open ng LINUX_MSG_BIN \n", 0,0,0,0,0,0 );
		}
		b_initAbleMsgBin = True;	// 確認完了.
	}
	return( b_AbleMsgBin );
}
#endif	// #ifdef LINUX.
#endif // DIV_MFP_LIB

#ifdef DIV_MFP_LIB
int32_t getLangCount() {
	return 0;
}
TYPE_Language getLanguageType( int LangNo ) {
	return 0;
}
int loadSelectLangType_IOP( TYPE_Language ec_Type ) {
	return 0;
}
int loadSelectLangType( TYPE_Language ec_Type ) {
	return 0;
}
#endif // DIV_MFP_LIB
//----------------------------------------------------------------------------.
//【関数名】 loadKeyMapData.
//【内  容】 KeyMapデータのロード（ROM→RAM）.
//----------------------------------------------------------------------------.
void loadKeyMapData()
{
	if( ( NULL == guc_selDataRAM )
	    && ( NULL == guc_allDataRAM ) ) {
		printf( "[KEYMAP]★NULL pointer  guc_selDataRAM:[%p],guc_allDataRAM:[%p]\n",guc_selDataRAM,guc_allDataRAM );
		return;
	}

	// 登録言語数取得.
	int32_t i32_LangCount = getLangCount();
	int32_t i32_LangCountMSGBIN = c_SelectLangInfo.getMSGBINLangNum();				// メッセージバイナリ内の言語数.

	if( 0 >= i32_LangCount ) {
		printf( "[KEYMAP]★i32_LangCount error:[%d]\n",i32_LangCount );
		return;
	}
	uchar* puc_KeyMapStart = NULL;
	int32_t i32_RemainSize = 0;

	if( True == isAllocSuccess ) {
		// 9言語分のメモリ確保済み.
		// キーMAPデータ格納開始位置.
		// 拡張メモリありの場合、第1言語の終端に格納.
		puc_KeyMapStart = guc_allDataRAM + ( SIZE_FILEDATA_SELECT - SIZE_KEYBOARDDATA_SELECT );

		// メモリ残容量.
		i32_RemainSize = SIZE_KEYBOARDDATA_SELECT;

	} else {
		// 拡張メモリなしの場合、選択言語の終端に格納.
		puc_KeyMapStart = guc_selDataRAM + ( SIZE_FILEDATA_SELECT - SIZE_KEYBOARDDATA_SELECT );
		// メモリ残容量.
		i32_RemainSize = SIZE_KEYBOARDDATA_SELECT;
	}


	// ソフトキーボードオフセットアドレステーブル先頭.
	const int32_t i32_OffsetTableSoftKey = (int32_t)( GetFixedDataSize_FromMSGBIN()			// 固定エリア＋言語コード.
	                                   + ( PNLD_FONTSWITCH_SIZE * i32_LangCountMSGBIN )	 //フォントスイッチデータ.
	                                   + ( 2 * sizeof( int32_t ) * i32_LangCountMSGBIN )	// 言語データオフセットテーブル.
	                                   + ( 2 * sizeof( int32_t ) * i32_LangCountMSGBIN )	// フォントデータオフセットテーブル.
	                                   + ( 2 * sizeof( int32_t ) * i32_LangCountMSGBIN )	// 辞書データオフセットテーブル.
	                                   + ( 3 * 2 * sizeof( int32_t ) * i32_LangCountMSGBIN ) ); 	// Pattern BMPデータテーブル(パターン数は３固定).

	// 外付けキーボードオフセットアドレステーブル先頭.
	const int32_t i32_OffsetTableUSBKey = (int32_t)( i32_OffsetTableSoftKey + ( 2 * sizeof( int32_t ) * i32_LangCountMSGBIN ) );


	// -----------------------------------------------------------------.
	// ソフトキーボードオフセットアドレスの取得.
	// -----------------------------------------------------------------.
	PNLS_KeyMapOffsetTable st_SoftKeyTableTmp[PNL_MAX_SOFTKEYBOARDLANGUAGES];
	// 初期化.
	for( uchar i = 0; i < ( uchar )( sizeof( st_SoftKeyTableTmp )/sizeof( PNLS_KeyMapOffsetTable ) ); i++ ) {
		st_SoftKeyTableTmp[i].i32_Addr = -1;
		st_SoftKeyTableTmp[i].i32_Size = 0;
	}
	uchar uc_RAMBufferSoft[2 * PNL_MAX_SOFTKEYBOARDLANGUAGES * sizeof( int32_t )];
	memset( uc_RAMBufferSoft,0,sizeof( uc_RAMBufferSoft ) );

	// Flashからロード.
	if( 0 == loadSelectLangKeymap( uc_RAMBufferSoft, ( uchar )( sizeof( uc_RAMBufferSoft ) ), i32_OffsetTableSoftKey, 2 * sizeof( int32_t ) ) ) {
		// オフセットテーブルへ設定.
		makeOffsetTable( uc_RAMBufferSoft,(uchar)i32_LangCount,st_SoftKeyTableTmp );
	}
	// -----------------------------------------------------------------.

	// -----------------------------------------------------------------.
	// 外付けキーボードオフセットアドレスの取得.
	// -----------------------------------------------------------------.
	PNLS_KeyMapOffsetTable st_USBKeyTableTmp[PNL_MAX_USBKEYBOARDLANGUAGES];

	// 初期化.
	for( i = 0; i < ( uchar )( sizeof( st_USBKeyTableTmp )/sizeof( PNLS_KeyMapOffsetTable ) ); i++ ) {
		st_USBKeyTableTmp[i].i32_Addr = -1;
		st_USBKeyTableTmp[i].i32_Size = 0;
	}
	uchar uc_RAMBufferUSB[2 * PNL_MAX_USBKEYBOARDLANGUAGES * sizeof( int32_t )];
	memset( uc_RAMBufferUSB,0,sizeof( uc_RAMBufferUSB ) );

	// Flashからロード.
	if( 0 == loadSelectLangKeymap( uc_RAMBufferUSB, ( uchar )( sizeof( uc_RAMBufferUSB ) ), i32_OffsetTableUSBKey, 2 * sizeof( int32_t ) ) ) {
		// オフセットテーブルへ設定.
		makeOffsetTable( uc_RAMBufferUSB,(uchar)i32_LangCount,st_USBKeyTableTmp );
	}
	// -----------------------------------------------------------------.

	PDPS_KeyMap* st_KeyMapTmp = ( PDPS_KeyMap* )puc_KeyMapStart;

	memset( st_KeyMapTmp,0,sizeof( PDPS_KeyMap ) );

	// ソフトキーボードKeyMapデータのロード.
	for( i = 0; i < ( uchar )( sizeof( st_SoftKeyTableTmp )/sizeof( PNLS_KeyMapOffsetTable ) ); i++ ) {
		if( False == loadSoftKeyMapData( st_SoftKeyTableTmp[i].i32_Addr,
		                                 st_SoftKeyTableTmp[i].i32_Size,
		                                 &( st_KeyMapTmp->st_SoftKey[i] ) ) ) {
			return;
		}
	}

	// 外付けキーボードKeyMapデータのロード.
	for( i = 0; i < ( uchar )( sizeof( st_USBKeyTableTmp )/sizeof( PNLS_KeyMapOffsetTable ) ); i++ ) {
		if( False == loadUSBKeyMapData( st_USBKeyTableTmp[i].i32_Addr,
		                                st_USBKeyTableTmp[i].i32_Size,
		                                &( st_KeyMapTmp->st_USBKey[i] ) ) ) {
			return;
		}
	}

	// 表示言語情報取得.
	st_KeyMapTmp->ss_ViewLangNum = 0;
	for( i = 0; i < ( uchar )i32_LangCount; i++ ) {
		TYPE_Language e_VieLanguageTmp;
		if( True == getViewLanguage( i,&e_VieLanguageTmp ) ) {
			st_KeyMapTmp->e_ViewLanguage[ st_KeyMapTmp->ss_ViewLangNum ] = e_VieLanguageTmp;
			st_KeyMapTmp->ss_ViewLangNum++;
		}
	}

	// キーMAPデータを設定.
	PDPC_KeyMapManager::newInstance()->setKeyMapData( st_KeyMapTmp );

}

//----------------------------------------------------------------------------.
//【関数名】 loadSoftKeyMapData.
//【内  容】 ソフトキーボードKeyMapデータのロード（ROM→RAM）.
//----------------------------------------------------------------------------.
Bool loadSoftKeyMapData( int32_t i32_Addr,int32_t i32_Size,PDPS_SoftKeyMap* st_SoftKeyMap )
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
	if( -1 == loadLangData2Ram( uc_RAMBuffer,i32_Addr,i32_Size ) ) {
		printf( "[KEYMAP]★data load error(SoftKeyMap)\n" );
		return False;
	}
	uchar* puc_CurrentPoint = uc_RAMBuffer;

	// キーボードレイアウト種別（国種別）情報.
	short ss_LanguageTmp = 0;
	memcpy( &( ss_LanguageTmp ),puc_CurrentPoint,sizeof( short ) );
	printf( "■ss_LanguageTmp:%d\n",ss_LanguageTmp );
	TYPE_Language e_LanguageTmp = ( TYPE_Language )ss_LanguageTmp;
	e_LanguageTmp = convertTYPELanguage( e_LanguageTmp );	// MSGバイナリの定義値(言語コード)をTYPE_Languageの定義値に変換.
	st_SoftKeyMap->e_Language = e_LanguageTmp;

	puc_CurrentPoint += sizeof( short );

	// 文字コードエンコード種別（文字集合種別）情報.
	memcpy( &( st_SoftKeyMap->sc_Encode ),
	        puc_CurrentPoint,
	        sizeof( st_SoftKeyMap->sc_Encode ) );

	puc_CurrentPoint += sizeof( st_SoftKeyMap->sc_Encode );

	// 文字コードオフセットアドレスの取得.
	PNLS_KeyMapOffsetTable st_CharCodeTableTmp[PLBD_SoftKeyMode_Size];
	memset( st_CharCodeTableTmp, 0x00, sizeof( st_CharCodeTableTmp ) );
	makeOffsetTable( puc_CurrentPoint,sizeof( st_CharCodeTableTmp )/sizeof( PNLS_KeyMapOffsetTable ),st_CharCodeTableTmp );

	// 文字コードデータ取得.
	for( uchar i = 0; i < ( uchar )( sizeof( st_CharCodeTableTmp )/sizeof( PNLS_KeyMapOffsetTable ) ); i++ ) {

		// エンコード種別は言語単位で固定.
		st_SoftKeyMap->st_CodeData[i].ep_charSet = st_SoftKeyMap->sc_Encode;

		// 格納文字コード数.
		uchar uc_TableNum = (uchar)( st_CharCodeTableTmp[i].i32_Size / ( 6 * sizeof( char ) ) );

		// 文字コードテーブル作成.
		makeSoftCharCodeTable( uc_RAMBuffer + st_CharCodeTableTmp[i].i32_Addr,uc_TableNum,st_SoftKeyMap->st_CodeData[i].keyInfo );
	}
	st_SoftKeyMap->valid = True;

	return True;
}


//----------------------------------------------------------------------------.
//【関数名】 loadUSBKeyMapData.
//【内  容】 外付けキーボードKeyMapデータのロード（ROM→RAM）.
//----------------------------------------------------------------------------.
Bool loadUSBKeyMapData( int32_t i32_Addr,int32_t i32_Size,PDPS_USBKeyMap* st_USBKeyMap )
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
	if( -1 == loadLangData2Ram( uc_RAMBuffer,i32_Addr,i32_Size ) ) {
		printf( "[KEYMAP]★data load error(st_USBKeyMap)\n" );
		return False;
	}
	uchar* puc_CurrentPoint = uc_RAMBuffer;

	// キーボードレイアウト種別（国種別）情報.
	short ss_LanguageTmp = 0;
	memcpy( &( ss_LanguageTmp ),puc_CurrentPoint,sizeof( short ) );
	TYPE_Language e_LanguageTmp = ( TYPE_Language )ss_LanguageTmp;
	e_LanguageTmp = convertTYPELanguage( e_LanguageTmp );	// MSGバイナリのコードがIWSのコードなのでTYPE_Languageの定義値に変換.
	st_USBKeyMap->e_Language = e_LanguageTmp;

	puc_CurrentPoint += sizeof( short );

	// 文字コードエンコード種別（文字集合種別）情報.
	memcpy( &( st_USBKeyMap->sc_Encode ),
	        puc_CurrentPoint,
	        sizeof( st_USBKeyMap->sc_Encode ) );

	puc_CurrentPoint += sizeof( st_USBKeyMap->sc_Encode );

	// 文字コードオフセットアドレスの取得.
	PNLS_KeyMapOffsetTable st_CharCodeTableTmp[PDPE_USBKeyKind_Num];
	memset( st_CharCodeTableTmp, 0x00, sizeof( st_CharCodeTableTmp ) );
	makeOffsetTable( puc_CurrentPoint,sizeof( st_CharCodeTableTmp )/sizeof( PNLS_KeyMapOffsetTable ),st_CharCodeTableTmp );

	// 文字コードデータ取得.
	for( uchar i = 0; i < ( uchar )( sizeof( st_CharCodeTableTmp )/sizeof( PNLS_KeyMapOffsetTable ) ); i++ ) {

		// 格納文字コード数.
		uchar uc_TableNum = (uchar)( st_CharCodeTableTmp[i].i32_Size / ( 18*sizeof( char ) ) );

		// 文字コードテーブル作成.
		makeUSBCharCodeTable( uc_RAMBuffer + st_CharCodeTableTmp[i].i32_Addr,
		                      uc_TableNum,
		                      &( st_USBKeyMap->st_CodeData[i] ) );
	}
	st_USBKeyMap->valid = True;

	// QtConnectorへ外付けキーボード用キーMapを登録する.
	PDPC_KeyMapManager::newInstance()->setUsbKeyMapToQt( st_USBKeyMap );

	return True;
}

//----------------------------------------------------------------------------.
//【関数名】 makeOffsetTable.
//【内  容】 オフセットテーブル作成.
//----------------------------------------------------------------------------.
Bool makeOffsetTable( const uchar* puc_Start,uchar sc_Num,PNLS_KeyMapOffsetTable* st_OffsetTable )
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
		st_OffsetTable[i].i32_Size = S32_WORD_SIZE( st_OffsetTable[i].i32_Size );	// サイズデータを格納する際には奇数バイトのデータを偶数に丸める.
		puc_Current += sizeof( st_OffsetTable[i].i32_Size );
	}

	return True;
}

//----------------------------------------------------------------------------.
//【関数名】 makeSoftCharCodeTable.
//【内  容】 ソフトキーボード用文字コードテーブル作成.
//----------------------------------------------------------------------------.
Bool makeSoftCharCodeTable( const uchar* puc_Start,uchar sc_Num,PLBS_GeneralKeyInfo* st_Table )
{
	if( ( NULL == puc_Start ) || ( NULL == st_Table ) ) {
		printf( "[KEYMAP]★NULL pointer  puc_Start:[%p],st_Table:[%p]\n",puc_Start,st_Table );
		return False;
	}
	const uchar* puc_Current = puc_Start;

	// 指定された読込個数が、キーの最大個数を超えた場合は.
	// 読込個数をキーの最大個数(=49)に丸め込む.
	// plibh2.h.
	if( PLBD_KeyNumMax < sc_Num ) { sc_Num = PLBD_KeyNumMax; }
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

//----------------------------------------------------------------------------.
//【関数名】 makeUSBCharCodeTable.
//【内  容】 外付けキーボード用文字コードテーブル作成.
//----------------------------------------------------------------------------.
Bool makeUSBCharCodeTable( const uchar* puc_Start,uchar sc_Num,PDPS_USBCharCode* st_Table )
{
	if( ( NULL == puc_Start ) || ( NULL == st_Table ) ) {
		printf( "[KEYMAP]★NULL pointer  puc_Start:[%p],st_Table:[%p]\n",puc_Start,st_Table );
		return False;
	}
	const uchar* puc_Current = puc_Start;

	// 指定された読込個数が、キーの最大個数を超えた場合は.
	// 読込個数をキーの最大個数(=112)に丸め込む.
	// pdpc2_CLCD.h.
	if( PDPD_USBCharCodeNum < sc_Num ) { sc_Num = PDPD_USBCharCodeNum; }

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
//----------------------------------------------------------------------------.
//【関数名】 getViewLanguage.
//【内  容】 表示言語情報取得.
//----------------------------------------------------------------------------.
Bool getViewLanguage( int32_t i32_LangNo,TYPE_Language* pe_Language )
{
	if( ( i32_LangNo < 0 )
	    || ( i32_LangNo >= PNL_MAX_LANGUAGES )
	    || ( NULL == pe_Language )
	    || ( NULL == guc_selDataRAM ) ) {
		printf( "[KEYMAP]★Data Error  i32_LangNo:[%d],pe_Language:[%p],guc_selDataRAM:[%p]\n",i32_LangNo,pe_Language,guc_selDataRAM );
		return False;
	}

	char	sc_LangCode[SIZE_LANGCODE];
	memset( sc_LangCode, 0, sizeof( sc_LangCode ) );

	memcpy( sc_LangCode,&guc_selDataRAM[OFFSET_1ST_LANGCODE + ( SIZE_LANGCODE * i32_LangNo )], SIZE_LANGCODE );

	for( uchar i = 0; i < ( uchar )( sizeof( stPanelLanguageTable ) / sizeof( STPanelLanguageTable ) ); i++ ) {
		if( strncmp( sc_LangCode,stPanelLanguageTable[i].p_Code,sizeof( sc_LangCode ) ) == 0 ) {
			*pe_Language = stPanelLanguageTable[i].ec_Type;
			return True;
		}
	}
	return False;
}


// 変換テーブル用構造体.
struct stConvertCode {
	TYPE_Language	e_DBCode;		// IWSのコード.
	TYPE_Language	e_ChangeCode;	// 変換するコード.
};

// 変換テーブル.
const stConvertCode tbl_convertTYPELanguage[] = {
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
	{ 26,	TYPE_LanguageAmerican	},		// エストニア語(定義値なし(英語を設定)).
	{ 27,	TYPE_LanguageAmerican	},		// ペルシア語(定義値なし(英語を設定)).
	{ 28,	TYPE_LanguageCroatian	},		// クロアチア語.
	{ 29,	TYPE_LanguageAmerican	},		// インドネシア語(定義値なし(英語を設定)).
	{ 30,	TYPE_LanguageAmerican	},		// リトアニア語(定義値なし(英語を設定)).
	{ 31,	TYPE_LanguageCatalan	},		// カタロニア語.
	{ 32,	TYPE_LanguageSlovene	},		// スロベニア語.
	{ 33,	TYPE_LanguageUkrainian	},		// ウクライナ語.
	{ 34,	TYPE_LanguageAmerican	},		// ラトビア語(定義値なし(英語を設定)).
	{ 35,	TYPE_LanguageBasque		},		// バスク語.
	{ 36,	TYPE_LanguageBulgarian	},		// ブルガリア語.
// comment 2015/06/15 OPS1-Sky入江 [ZeusS_ZX0][RQ-405] パネルUIのヘブライ語対応.
	{ 37,	TYPE_LanguageHebrew		},		// ヘブライ語.
// comment 2018/12/14 Sky毛穴 [EagleHigh][RQ-258] タイ語とベトナム語の対応.
	{ 38,	TYPE_LanguageVietnam	},		// ベトナム語.
// comment 2022/11/9 Sky奥田 [RQ-938] バルト三国対応.
	{ 39,  	TYPE_LanguageEstonian	},		// エストニア語. 
	{ 40,  	TYPE_LanguageLatvian	},		// ラトビア語. 
	{ 41,  	TYPE_LanguageLithuanian	},		// リトアニア語. 
// comment 2023/2/7 Sky小熊 [IT6_2.3][RQ-970]Welsh as standard language for IT.
	{ 42, 	TYPE_LanguageWelsh},			// ウェールズ語. 
	{ 0x7F,	0x7F					},		// Terminator.
};

//----------------------------------------------------------------------------.
//【関数名】 convertTYPELanguage.
//【内  容】 IWSの言語コードをTYPE_Languageの定義値に変換する.
//----------------------------------------------------------------------------.
static	TYPE_Language	convertTYPELanguage( TYPE_Language e_Language )
{
	TYPE_Language	e_RetCode=TYPE_LanguageAmerican;
	uchar	ucLoop=0;

	while( 0x7F != tbl_convertTYPELanguage[ucLoop].e_DBCode ) {
		if( tbl_convertTYPELanguage[ucLoop].e_DBCode == e_Language ) {
			e_RetCode = tbl_convertTYPELanguage[ucLoop].e_ChangeCode;
			break;
		}
		ucLoop++;
	}
	return	e_RetCode;
}

// MFPのTHRC_Threadif.cppのgetLanguageDataForUI()を元に新規作成
// ================================================================================================.
//! \param unsigned short *pss_Buf ............... 転送先のバッファへのポインタ(2バイト境界).
//! \param uint32_t ui32_Offset ............ 言語データ先頭からのオフセット(偶数).
//! \param uint32_t ui32_Size .............. 転送するデータサイズ(バイト単位)(偶数).
//! \return True:成功／False:失敗.
//! 回転は考慮無しで算出。\n.
//! \param なし.
//! \n.
// ================================================================================================.
Bool getLanguageDataForUINewBin( unsigned short* pss_Buf, uint32_t ui32_Offset, uint32_t ui32_Size )
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
		if( '\0' == sc_MessgeBinPath[0] ) {
			memset( sc_MessgeBinPath, 0, sizeof( sc_MessgeBinPath ) );
			THRF_getMsgBinName( sc_MessgeBinPath, sizeof( sc_MessgeBinPath ) );
		}

		if( NULL != ( fp = SFLG_Fopen( sc_MessgeBinPath, "rb" ) ) ) {
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
		if( '\0' == sc_MessgeBinPath[0] ) {
			memset( sc_MessgeBinPath, 0, sizeof( sc_MessgeBinPath ) );
			THRF_getMsgBinName( sc_MessgeBinPath, sizeof( sc_MessgeBinPath ) );
		}

		fp = SFLG_Fopen( sc_MessgeBinPath, "rb" );

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

//----------------------------------------------------------------------------.
//【関数名】 setExtLangFontData.
//【内  容】 拡張メモリ無し時に中韓台の言語のfontheadデータをロードする.
//【引  数】 TYPE_Language ec_Type : 現在のパネル言語( 多重でロードしないようにするため ).
//----------------------------------------------------------------------------.
static void setExtLangFontData( TYPE_Language ec_Type )
{
	// 拡張メモリ有りの場合は全言語ロードされる為何もせずリターン.
	if( True == APIC_Enabler::newInstance()->isExtendedMemory() ) {	return;	}

	int32_t i32_LangCount = getLangCount();		// 設定可能言語数.
	int i;	// ループカウンタ.
	// 台湾.
	if( TYPE_LanguageChinese1 != ec_Type ) {
		for( i = 0; i < i32_LangCount; i++ ) {
			if( 0 == strcmp( "zh-TW", ( const char* )&guc_selDataRAM[OFFSET_1ST_LANGCODE + ( SIZE_LANGCODE * i )] ) ) {	// 台湾語.
				SetLangFontData( i, 0 );	// Minerva/ZeusはROM情報が圧縮データ扱いで.
				// 第2引数の選択メモリ格納位置を決めるための格納データサイズは考慮する必要が無い為0にしている.
				break;
			}
		}
	}

	// 中国.
	if( TYPE_LanguageChinese2 != ec_Type ) {
		for( i = 0; i < i32_LangCount; i++ ) {
			if( 0 == strcmp( "zh-CN", ( const char* )&guc_selDataRAM[OFFSET_1ST_LANGCODE + ( SIZE_LANGCODE * i )] ) ) {	// 中国語.
				SetLangFontData( i, 0 );	// Minerva/ZeusはROM情報が圧縮データ扱いで.
				// 第2引数の選択メモリ格納位置を決めるための格納データサイズは考慮する必要が無い為0にしている.
				break;
			}
		}
	}

	// 韓国.
	if( TYPE_LanguageHangul != ec_Type ) {
		for( i = 0; i < i32_LangCount; i++ ) {
			if( 0 == strcmp( "ko", ( const char* )&guc_selDataRAM[OFFSET_1ST_LANGCODE + ( SIZE_LANGCODE * i )] ) ) {	// 韓国語.
				SetLangFontData( i, 0 );	// Minerva/ZeusはROM情報が圧縮データ扱いで.
				// 第2引数の選択メモリ格納位置を決めるための格納データサイズは考慮する必要が無い為0にしている.
				break;
			}
		}
	}
}
//----------------------------------------------------------------------------.
//【関数名】 SetLangData_OapMyPanel.
//
//【内  容】 言語データ取得設定関数.
//
//【引  数】 [i] i32_dataOffset    : 対象の言語データが格納されているオフセット.
//			 [i] i32_serviceOffset : サービス言語が格納されている言語データオフセット.
//
//【戻り値】 言語データサイズ.
//
//----------------------------------------------------------------------------
static int32_t SetLangData_OapMyPanel( int32_t i32_dataOffset, int32_t i32_serviceOffset )
{
	int32_t i32_selectDataOffset = 0;		// 選択言語メモリ(guc_selDataRAM)上のオフセット.
	int32_t i32_langAdrsOffset	= 0;		// 言語データ内アドレス&データ長格納用オフセット.
	int32_t i32_addressData = 0;			// オフセットアドレス.
	int32_t i32_addressServ = 0;			// オフセットアドレス.
	int32_t i32_returnSize = 0;				// 戻り値用総サイズ.
	int32_t i32_addressBrow = 0;			// オフセットアドレス.

	int32_t i32_msgAddress = 0;				// 単一言語メッセージデータ内のアドレス.
	int32_t i32_msgSize = 0;				// 単一言語メッセージデータ内のサイズ.

	int32_t i32_dataAddress   = 0;			// データ先頭アドレス.
	int32_t i32_totalDataSize = 0;			// 格納した総言語データサイズ.
	int32_t* pi32_selectLangData = NULL;	// 単一言語メッセージデータ先頭アドレス (int32_t*).
	int32_t i32_allLangData[6];				// ROM上の選択言語データの先頭アドレス.
	int32_t i32_allServData[6];				// ROM上のサービス用選択言語データの先頭アドレス.
	int32_t i32_allBrowData[8];				// ROM上のブラウザ用選択言語データの先頭アドレス.

// pus_DeflatedDataは使う前にNULLチェックすること.
	ushort* pus_DeflatedData = NULL;	// 圧縮データの一次格納先(mallocで領域確保する).
	uint32_t ui32_TmpBufSize = 0;			// 一次格納先のサイズ.
	uint32_t ui32_OutSize= 0;				// 解凍されたサイズ.
	Bool bl_decompResult = True;		// 解凍成功したか.

//#ifdef SIMULATOR
//	guc_selDataRAM = guc_selDataRAM_OapMyPanel_SIM;
//#endif
	//言語領域を初期化.
	WriteToRAMFixedData();

	// 固定データサイズ取得.
	int32_t i32_fixedDataSize = GetFixedDataSize();

	// オフセット設定.
	i32_selectDataOffset = (int32_t)( i32_fixedDataSize + ( sizeof( int32_t ) * COUNT_SELDATA_ITEMS ) );
	i32_langAdrsOffset = i32_selectDataOffset;

	int32_t i32_tempOffset = 0;

	// MFPとPrinterを判別してオフセット値を計算する.
	i32_tempOffset = i32_dataOffset + GetOffsetIfPrinter();
	if( False == IsMFPVersion() ) {
		// Printerの時は、i32_dataOffsetにPSWCとPrinterのオフセットアドレス分が含まれているため、その分を減算しておく.
		i32_tempOffset -= SIZE_FIXED_DATA_PSWC_AND_PTR;
	}
	// 言語データのオフセット取得.
	loadLangData2Ram( &i32_addressData, i32_tempOffset, sizeof( i32_addressData ) );
	if( PNL_RESULT_ERROR == i32_addressData ) {
		DISPLOG( " ★★PNL_RESULT_ERROR[言語データのオフセット取得]失敗!!!\n", 0,0,0,0,0,0 );
		return 0;
	}
	i32_tempOffset = i32_addressData + GetOffsetIfPrinter();	// MFP/Printerを判別してオフセットを取得する.
	loadLangData2Ram( i32_allLangData, i32_tempOffset, sizeof( i32_allLangData ) );

	// MFPとPrinterを判別してオフセット値を計算する.
	i32_tempOffset = i32_serviceOffset + GetOffsetIfPrinter();
	if( False == IsMFPVersion() ) {
		// Printerの時は、i32_dataOffsetにPSWCとPrinterのオフセットアドレス分が含まれているため、その分を原産しておく.
		i32_tempOffset -= SIZE_FIXED_DATA_PSWC_AND_PTR;
	}
	// 言語データ（サービス）のオフセット取得.
	loadLangData2Ram( &i32_addressServ, i32_tempOffset, sizeof( i32_addressServ ) );
	if( PNL_RESULT_ERROR == i32_addressServ ) {
		DISPLOG( " ★★PNL_RESULT_ERROR[言語データ（サービス）のオフセット取得]失敗!!!\n", 0,0,0,0,0,0 );
		return 0;
	}
	i32_tempOffset = i32_addressServ + GetOffsetIfPrinter();	// MFP/Printerを判別してオフセットを取得する.
	loadLangData2Ram( i32_allServData, i32_tempOffset, sizeof( i32_allServData ) );

	// ブラウザの言語データはユーザ言語と連動するため言語データのオフセットを使用する.
	// MFPとPrinterを判別してオフセット値を計算する.
	i32_tempOffset = i32_dataOffset + GetOffsetIfPrinter();
	if( False == IsMFPVersion() ) {
		// Printerの時は、i32_dataOffsetにPSWCとPrinterのオフセットアドレス分が含まれているため、その分を減算しておく.
		i32_tempOffset -= SIZE_FIXED_DATA_PSWC_AND_PTR;
	}
	// 言語データのオフセット取得.
	loadLangData2Ram( &i32_addressBrow, i32_tempOffset, sizeof( i32_addressBrow ) );
	if( PNL_RESULT_ERROR == i32_addressBrow ) {
		DISPLOG( " ★★PNL_RESULT_ERROR[ブラウザ言語データのオフセット取得]失敗!!!\n", 0,0,0,0,0,0 );
		return 0;
	}
	i32_tempOffset = i32_addressBrow + GetOffsetIfPrinter();	// MFP/Printerを判別してオフセットを取得する.
	loadLangData2Ram( i32_allBrowData, i32_tempOffset, sizeof( i32_allBrowData ) );

	if( True == isNeedInflate() ) {
		// ユーザデータ、サービスデータのうち、最もサイズの大きな圧縮サイズを動的確保の容量とする.
		// 一回の動的確保で、ユーザデータ、サービスデータを全て展開したいため.
		ui32_TmpBufSize = i32_allLangData[PNL_LANG_USER_DATA]; // まずユーザデータの圧縮サイズを入れる.
		if( i32_allServData[PNL_LANG_SER1_ADDRESS] > 0 && ( int32_t )ui32_TmpBufSize < i32_allServData[PNL_LANG_SER1_ADDRESS] ) {
			ui32_TmpBufSize = i32_allServData[PNL_LANG_SER1_ADDRESS]; // サービス１データのサイズのほうが大きければサービス１データの圧縮サイズにする.
		}
		if( i32_allServData[PNL_LANG_SER2_USER_ADDRESS] > 0 && ( int32_t )ui32_TmpBufSize < i32_allServData[PNL_LANG_SER2_USER_ADDRESS] ) {
			ui32_TmpBufSize = i32_allServData[PNL_LANG_SER2_USER_ADDRESS]; // サービス２データのサイズのほうが大きければサービス２データの圧縮サイズにする.
		}
		// 圧縮サイズが奇数バイトの場合を考慮したサイズを確保する.
		ui32_TmpBufSize = S32_WORD_SIZE( ui32_TmpBufSize );
		pus_DeflatedData = ( ushort* )malloc( ui32_TmpBufSize );

		// 領域確保できなかった場合、ゼロリターン.
		if( pus_DeflatedData == NULL ) {
			DISPLOG( " ★★ppcglanguage malloc([%d]) 失敗!!!\n", ui32_TmpBufSize,0,0,0,0,0 );
			return 0;
		}
	}

	// 単一言語メッセージデータの先頭アドレス.
	pi32_selectLangData  = ( int32_t* ) &guc_selDataRAM[i32_langAdrsOffset];

	// ユーザー言語データのコピー先アドレス.
	i32_selectDataOffset = (int32_t)( i32_selectDataOffset + ( sizeof( int32_t ) * COUNT_MSGDATA_ITEMS ) );

	// コピー総サイズ.
	i32_totalDataSize = (int32_t)( i32_totalDataSize + ( sizeof( int32_t ) * COUNT_MSGDATA_ITEMS ) );

	int32_t i32_OffsetROM = 0;
	//--------------------------------------------------------------------
	// ユーザー言語データをコピー.
	//--------------------------------------------------------------------
	if( PNL_RESULT_ERROR != i32_allLangData[PNL_LANG_USER_ADDRESS] ) {

		i32_dataAddress = i32_addressData + i32_allLangData[PNL_LANG_USER_ADDRESS];
		i32_msgSize	   = i32_allLangData[PNL_LANG_USER_DATA];
		i32_OffsetROM = i32_dataAddress + GetOffsetIfPrinter();

		// RAM上でのユーザー言語の格納位置(単一言語メッセージデータ内オフセット).
		i32_msgAddress = sizeof( int32_t ) * COUNT_MSGDATA_ITEMS;

		if( isNeedInflate() == False ) { // 解凍不要な機種の場合.
			// コピーする領域がRAMに収まる.
			if( i32_selectDataOffset + i32_msgSize < SIZE_FILEDATA_SELECT ) {
				// コピー.
				loadLangData2Ram( ( void* )&guc_selDataRAM[i32_selectDataOffset],	i32_OffsetROM, i32_msgSize );

				// [単一言語メッセージデータ]にアドレスとデータ長の格納.
				pi32_selectLangData[PNL_LANG_USER_ADDRESS] = i32_msgAddress;
				pi32_selectLangData[PNL_LANG_USER_DATA] = i32_msgSize;
			} else {
				// アドレスとデータ長の格納.
				pi32_selectLangData[PNL_LANG_USER_ADDRESS] = i32_msgAddress;
				pi32_selectLangData[PNL_LANG_USER_DATA] = 0;
			}

			// オフセットの再設定.
			i32_selectDataOffset += i32_msgSize;
			i32_msgAddress += i32_msgSize;

			// コピー済サイズの更新.
			i32_totalDataSize += i32_msgSize;

		} else {
// pus_DeflatedDataは使う前にNULLチェックすること.
			if( NULL == pus_DeflatedData ) {return 0;}
			memset( pus_DeflatedData, 0, ui32_TmpBufSize ); // 圧縮データの取得用領域を初期化.
			loadLangData2Ram( ( void* )pus_DeflatedData,	i32_OffsetROM, S32_WORD_SIZE( i32_msgSize ) );
			// 解凍.
			bl_decompResult = do_decompress( pus_DeflatedData,
			                                 i32_msgSize,
			                                 &guc_selDataRAM[i32_selectDataOffset],
			                                 (uint32_t)( SIZE_FILEDATA_SELECT - i32_selectDataOffset ),
			                                 &ui32_OutSize );
			// 解凍に成功.
			if( True == bl_decompResult ) {
				// [単一言語メッセージデータ]にアドレスとデータ長の格納.
				pi32_selectLangData[PNL_LANG_USER_ADDRESS] = i32_msgAddress;	// [単一言語メッセージデータ]の先頭からユーザ言語データへのオフセットアドレス.
				pi32_selectLangData[PNL_LANG_USER_DATA] = ui32_OutSize;		// ユーザ言語データのデータサイズ.
			} else {
				// アドレスとデータ長の格納.
				pi32_selectLangData[PNL_LANG_USER_ADDRESS] = i32_msgAddress;
				pi32_selectLangData[PNL_LANG_USER_DATA] = 0;
			}

			// オフセットの再設定.
			i32_selectDataOffset += ui32_OutSize;  // [guc_selDataRAM]上のオフセット.
			i32_msgAddress += ui32_OutSize;		// [単一言語メッセージデータ]内でのユーザ言語データ終端位置.

			// コピー済サイズの更新.
			i32_totalDataSize += ui32_OutSize;
		}
	} else {
		// アドレスとデータ長の格納.
		pi32_selectLangData[PNL_LANG_USER_ADDRESS] = i32_allLangData[PNL_LANG_USER_ADDRESS];
		pi32_selectLangData[PNL_LANG_USER_DATA] = 0;
	}

	//--------------------------------------------------------------------
	// サービス 1 言語データをコピー.
	//--------------------------------------------------------------------
	if( PNL_RESULT_ERROR != i32_allServData[PNL_LANG_SER1_ADDRESS] ) {

		i32_dataAddress = i32_addressServ + i32_allServData[PNL_LANG_SER1_ADDRESS];
		i32_msgSize     = i32_allServData[PNL_LANG_SER1_USER_DATA];
		i32_OffsetROM = i32_dataAddress + GetOffsetIfPrinter();
		if( False == isNeedInflate() ) {
			// コピーする領域がRAMに収まる.
			if( i32_selectDataOffset + i32_msgSize < SIZE_FILEDATA_SELECT ) {
				// コピー
				loadLangData2Ram( ( void* )&guc_selDataRAM[i32_selectDataOffset], i32_OffsetROM, i32_msgSize );
				// アドレスとデータ長の格納.
				pi32_selectLangData[PNL_LANG_SER1_ADDRESS] = i32_msgAddress;
				pi32_selectLangData[PNL_LANG_SER1_USER_DATA] = i32_msgSize;
			} else {
				// アドレスとデータ長の格納.
				pi32_selectLangData[PNL_LANG_SER1_ADDRESS] = i32_msgAddress;
				pi32_selectLangData[PNL_LANG_SER1_USER_DATA] = 0;
			}

			// オフセットの再設定.
			i32_selectDataOffset += i32_msgSize;
			i32_msgAddress		+= i32_msgSize;

			// データサイズの格納.
			i32_totalDataSize += i32_msgSize;
		} else {
// pus_DeflatedDataは使う前にNULLチェックすること.
			if( NULL == pus_DeflatedData ) {return 0;}
			// 動的に確保したメモリの初期化.
			memset( pus_DeflatedData, 0, ui32_TmpBufSize );
			loadLangData2Ram( ( void* )pus_DeflatedData,	i32_OffsetROM, S32_WORD_SIZE( i32_msgSize ) );
			// 解凍.
			bl_decompResult = do_decompress( pus_DeflatedData,
			                                 i32_msgSize,
			                                 &guc_selDataRAM[i32_selectDataOffset],
			                                 (uint32_t)( SIZE_FILEDATA_SELECT - i32_selectDataOffset ),
			                                 &ui32_OutSize );
			// 解凍に成功.
			if( True == bl_decompResult ) {
				// アドレスとデータ長の格納.
				pi32_selectLangData[PNL_LANG_SER1_ADDRESS] = i32_msgAddress;  // [単一言語メッセージデータ]の先頭からサービス１言語データへのオフセットアドレス.
				pi32_selectLangData[PNL_LANG_SER1_USER_DATA] = ui32_OutSize;		// サービス１言語データの解凍後データサイズ.
			} else {
				// アドレスとデータ長の格納.
				pi32_selectLangData[PNL_LANG_SER1_ADDRESS] = i32_msgAddress;
				pi32_selectLangData[PNL_LANG_SER1_USER_DATA] = 0;
			}

			// オフセットの再設定.
			i32_selectDataOffset += ui32_OutSize;
			i32_msgAddress += ui32_OutSize;

			// データサイズの格納.
			i32_totalDataSize += ui32_OutSize;
		}
	} else {
		// アドレスとデータ長の格納.
		pi32_selectLangData[PNL_LANG_SER1_ADDRESS] = i32_allServData[PNL_LANG_SER1_ADDRESS];
		pi32_selectLangData[PNL_LANG_SER1_USER_DATA] = 0;
	}

	//--------------------------------------------------------------------
	// サービス ２言語データをコピー.
	//--------------------------------------------------------------------
	if( PNL_RESULT_ERROR != i32_allServData[PNL_LANG_SER2_USER_ADDRESS] ) {

		i32_dataAddress = i32_addressServ + i32_allServData[PNL_LANG_SER2_USER_ADDRESS];
		i32_msgSize     = i32_allServData[PNL_LANG_SER2_USER_DATA];
		i32_OffsetROM = i32_dataAddress + GetOffsetIfPrinter();
		if( isNeedInflate() == False ) {
			// コピーする領域がRAMに収まる.
			if( i32_selectDataOffset + i32_msgSize < SIZE_FILEDATA_SELECT ) {
				loadLangData2Ram( ( void* )&guc_selDataRAM[i32_selectDataOffset], i32_OffsetROM, i32_msgSize );
				// アドレスとデータ長の格納.
				pi32_selectLangData[PNL_LANG_SER2_USER_ADDRESS] = i32_msgAddress;
				pi32_selectLangData[PNL_LANG_SER2_USER_DATA] = i32_msgSize;
			} else {
				// アドレスとデータ長の格納.
				pi32_selectLangData[PNL_LANG_SER2_USER_ADDRESS] = i32_msgAddress;
				pi32_selectLangData[PNL_LANG_SER2_USER_DATA] = 0;
			}

			// オフセットの再設定.
			i32_selectDataOffset += i32_msgSize;
			i32_msgAddress += i32_msgSize;

			// データサイズの格納.
			i32_totalDataSize += i32_msgSize;
		} else {
// pus_DeflatedDataは使う前にNULLチェックすること.
			if( NULL == pus_DeflatedData ) {return 0;}
			// 動的に確保したメモリの初期化.
			memset( pus_DeflatedData, 0, ui32_TmpBufSize );
			// コピー.
			loadLangData2Ram( ( void* )pus_DeflatedData,	i32_OffsetROM, S32_WORD_SIZE( i32_msgSize ) );
			// 解凍 .
			bl_decompResult = do_decompress( pus_DeflatedData,
			                                 i32_msgSize,
			                                 &guc_selDataRAM[i32_selectDataOffset],
			                                 (uint32_t)( SIZE_FILEDATA_SELECT - i32_selectDataOffset ),
			                                 &ui32_OutSize );
			// 解凍に成功.
			if( True == bl_decompResult ) {
				// アドレスとデータ長の格納.
				pi32_selectLangData[PNL_LANG_SER2_USER_ADDRESS] = i32_msgAddress;  // [単一言語メッセージデータ]の先頭からサービス２言語データへのオフセットアドレス.
				pi32_selectLangData[PNL_LANG_SER2_USER_DATA] = ui32_OutSize;		// サービス２言語データの解凍後データサイズ.
			} else {
				// アドレスとデータ長の格納.
				pi32_selectLangData[PNL_LANG_SER2_USER_ADDRESS] = i32_msgAddress;
				pi32_selectLangData[PNL_LANG_SER2_USER_DATA] = 0;
			}

			// オフセットの再設定.
			i32_selectDataOffset += ui32_OutSize;
			i32_msgAddress		+= ui32_OutSize;

			// データサイズの格納.
			i32_totalDataSize += ui32_OutSize;
		}
	} else {
		// アドレスとデータ長の格納.
		pi32_selectLangData[PNL_LANG_SER2_USER_ADDRESS] = i32_allServData[PNL_LANG_SER2_USER_ADDRESS];
		pi32_selectLangData[PNL_LANG_SER2_USER_DATA] = 0;
	}

	//--------------------------------------------------------------------
	// ブラウザ言語データをコピー.
	//--------------------------------------------------------------------
	if( PNL_RESULT_ERROR != i32_allBrowData[PNL_LANG_BROW_USER_ADDRESS] ) {		// データサイズが0xFFFFFFFFのとき.
		i32_dataAddress = i32_addressBrow + i32_allBrowData[PNL_LANG_BROW_USER_ADDRESS];
		i32_msgSize     = i32_allBrowData[PNL_LANG_BROW_DATA];
		i32_OffsetROM = i32_dataAddress + GetOffsetIfPrinter();

		if( False == isNeedInflate() ) {
			// コピーする領域がRAMに収まる.
			if( ( i32_selectDataOffset + i32_msgSize ) < SIZE_FILEDATA_SELECT ) {
				// コピー.
				loadLangData2Ram( ( void* )&guc_selDataRAM[i32_selectDataOffset],i32_OffsetROM, i32_msgSize );
				// アドレスとデータ長の格納.
				pi32_selectLangData[PNL_LANG_BROW_USER_ADDRESS] = i32_msgAddress;
				pi32_selectLangData[PNL_LANG_BROW_DATA] = i32_msgSize;
			} else {
				// アドレスとデータ長の格納.
				pi32_selectLangData[PNL_LANG_BROW_USER_ADDRESS] = i32_msgAddress;
				pi32_selectLangData[PNL_LANG_BROW_DATA] = 0;
			}

			// オフセットの再設定.
			i32_selectDataOffset += i32_msgSize;
			i32_msgAddress		+= i32_msgSize;
			// データサイズの格納.
			i32_totalDataSize += i32_msgSize;
		} else {
// pus_DeflatedDataは使う前にNULLチェックすること.
			if( NULL == pus_DeflatedData ) {return 0;}
			// 動的に確保したメモリの初期化.
			memset( pus_DeflatedData, 0, ui32_TmpBufSize );
			// コピー.
			// getLanguageDataForUI()によるデータ取得時は偶数バイト単位で取得するため、圧縮データが奇数バイトの場合は1byte多く取得する必要がある.
			loadLangData2Ram( ( void* )pus_DeflatedData,	i32_OffsetROM, S32_WORD_SIZE( i32_msgSize ) );
			// 解凍
			bl_decompResult = do_decompress( pus_DeflatedData,
			                                 i32_msgSize,
			                                 &guc_selDataRAM[i32_selectDataOffset],
			                                 (uint32_t)( SIZE_FILEDATA_SELECT - i32_selectDataOffset ),
			                                 &ui32_OutSize );
			// 解凍に成功.
			if( bl_decompResult == True ) {
				// アドレスとデータ長の格納.
				pi32_selectLangData[PNL_LANG_BROW_USER_ADDRESS] = i32_msgAddress;  // [単一言語メッセージデータ]の先頭からブラウザデータへのオフセットアドレス.
				pi32_selectLangData[PNL_LANG_BROW_DATA] = ui32_OutSize;		// ブラウザデータの解凍後データサイズ.
			} else {
				// アドレスとデータ長の格納.
				pi32_selectLangData[PNL_LANG_BROW_USER_ADDRESS] = i32_msgAddress;
				pi32_selectLangData[PNL_LANG_BROW_DATA] = 0;
			}

			// オフセットの再設定.
			i32_selectDataOffset += ui32_OutSize;
			i32_msgAddress += ui32_OutSize;

			// データサイズの格納.
			i32_totalDataSize += ui32_OutSize;
		}
	} else {
		// アドレスとデータ長の格納.
		pi32_selectLangData[PNL_LANG_BROW_USER_ADDRESS] = 0;
		pi32_selectLangData[PNL_LANG_BROW_DATA] = 0;
	}
	// 解凍に使用したバッファ領域を開放.
	if( pus_DeflatedData != NULL ) {
		free( pus_DeflatedData );
	}

	//--------------------------------------------------------------------
	// 言語データのアドレスとデータ長の格納.
	//--------------------------------------------------------------------
	S32_LONG_RAM( i32_fixedDataSize ) = (int32_t)( i32_fixedDataSize + ( sizeof( int32_t ) * COUNT_SELDATA_ITEMS ) );
	S32_LONG_RAM( i32_fixedDataSize + sizeof( int32_t ) ) = i32_totalDataSize;

	// 戻り値設定.
	i32_returnSize = i32_totalDataSize;

	int32_t i32_addrTemp = pi32_selectLangData[PNL_LANG_SER1_ADDRESS];
	int32_t i32_sizeTemp = pi32_selectLangData[PNL_LANG_SER1_USER_DATA];
	pi32_selectLangData[PNL_LANG_SER1_ADDRESS] = pi32_selectLangData[PNL_LANG_SER2_USER_ADDRESS];
	pi32_selectLangData[PNL_LANG_SER1_USER_DATA] = pi32_selectLangData[PNL_LANG_SER2_USER_DATA];
	pi32_selectLangData[PNL_LANG_SER2_USER_ADDRESS] = i32_addrTemp;
	pi32_selectLangData[PNL_LANG_SER2_USER_DATA] = i32_sizeTemp;

	return i32_returnSize;
}

//----------------------------------------------------------------------------.
//【関数名】 changeFontSwitchVal.
//
//【内  容】 フォントスイッチ変更処理.
//
//【引  数】 [i] i_LangNo    : 言語番号-1 (0～言語数-1).
//
//【戻り値】 GFWE用のフォントスイッチ(9/7インチ).
//
//           DEF_LCD_7INCHは今後も削除されることはないということなので、不定値はスイッチにあったデータを返す.
//----------------------------------------------------------------------------
GFWE_LCD_Size changeFontSwitchVal( int si_LangNo )
{
	GFWE_LCD_Size ret_val = GFWE_LCD_Size_9inch;

	//閾値チェック.
	if( ( 0 > si_LangNo ) || ( PNL_MAX_LANGUAGES <= si_LangNo ) ) {
		//引数がマイナス値、もしくは配列数以上の場合、強制的にスイッチのデフォルト値を返す.
		ret_val = getDefaultFontSwitchVal();
	} else {
		switch( ui32_FontSwitch[si_LangNo] ) {
			//7インチ.
			case PNLD_FONTSWITCH_7INCH:
				ret_val = GFWE_LCD_Size_7inch;
				break;
			//9インチ.
			case PNLD_FONTSWITCH_9INCH:
				ret_val = GFWE_LCD_Size_9inch;
				break;
			//その他.
			default:
				//スイッチのデフォルト値を返す.
				ret_val = getDefaultFontSwitchVal();
				break;
		}
	}
	return ret_val;
}
//----------------------------------------------------------------------------.
//【関数名】 getDefaultFontSwitchVal.
//
//【内  容】 初期のフォントスイッチデータ取得.
//
//【引  数】 なし.
//
//【戻り値】 GFWE用のフォントスイッチ(9/7インチ).
//
//----------------------------------------------------------------------------
GFWE_LCD_Size getDefaultFontSwitchVal()
{
#ifdef DEF_LCD_7INCH
	return( GFWE_LCD_Size_7inch );
#else
	return( GFWE_LCD_Size_9inch );
#endif
}

// データ初期化.
void PNLC_SelectLangInfo::clearData() {

	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	for( uchar i=0; i<PNL_MAX_LANGUAGES; i++ ) {
		memset( m_st_SelectLangData[i].sc_LangCode, '\0', sizeof( m_st_SelectLangData[i].sc_LangCode ) );
		m_st_SelectLangData[i].uc_MSGBINIndex = 255;
	}
	m_uc_SelectLangNum = 0;
	m_i32_MSGBINLangNum = 0;
}
// コンストラクタ.
PNLC_SelectLangInfo::PNLC_SelectLangInfo() {
	clearData();								// データ初期化.
}
// 選択言語データ構築.
void PNLC_SelectLangInfo::createSelectLangInfo() {
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	clearData();								// データ初期化.

// メッセージバイナリから言語数を取得.
	int32_t i32_LangNum = 0;						// メッセージバイナリ内の搭載言語数.
	loadLangData2Ram( &i32_LangNum, OFFSET_LANGCOUNT, sizeof( i32_LangNum ) );
	if( 0 < i32_LangNum ) {
		m_i32_MSGBINLangNum = i32_LangNum;
	} else {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}

// メッセージバイナリから言語コードを取得.
	char sc_MSGBINLangCode[m_i32_MSGBINLangNum*SIZE_LANGCODE];		// 言語コードのサイズは言語数×8バイト.
	memset( sc_MSGBINLangCode, '\0', sizeof( sc_MSGBINLangCode ) );
	loadLangData2Ram( &sc_MSGBINLangCode, OFFSET_1ST_LANGCODE, sizeof( sc_MSGBINLangCode ) );

// 選択言語ファイルより言語コードを取得する.
	char sc_SelectLangCode[PNL_MAX_LANGUAGES*SIZE_LANGCODE];	// 言語コードのサイズは言語数(9)×8バイト.
	char sc_SelectLangPath[PNLD_SELECTLANGFILEPATH_LEN];		// 選択言語ファイルのファイルパス.
	uchar uc_SelectLangNum = 0;									// 選択言語ファイル内の言語数.

	memset( sc_SelectLangCode, '\0', sizeof( sc_SelectLangCode ) );
	memset( sc_SelectLangPath, '\0', sizeof( sc_SelectLangPath ) );

	// 選択言語ファイルのファイルパスをスレッドより取得する.
	THRF_getSelectLangDataFilePath( sc_SelectLangPath, ( uchar )( sizeof( sc_SelectLangPath ) ) );

	FILE* fp = SFLG_Fopen( sc_SelectLangPath, "rb" );				// 言語データ用ファイルポインタ.
	if( NULL != fp ) {
		SFLG_Fread( sc_SelectLangCode, 1, PNL_MAX_LANGUAGES*SIZE_LANGCODE, fp );
		SFLG_Fclose( fp );	// ファイルのクローズ.

		// 先頭文字が"\0"でなければ有効言語コードと判断し,言語コード数+1する.
		for( uchar uc_index = 0; PNL_MAX_LANGUAGES > uc_index; uc_index++ ) {
			if( '\0' != *( sc_SelectLangCode + SIZE_LANGCODE * uc_index ) ) {
				uc_SelectLangNum++;
			} else {	// 先頭"\0"以降の言語コードも"\0"なのでループを抜ける.
				break;
			}
		}
	} else {		// ファイルオープン失敗時(ファイルが存在しない)
		memcpy( sc_SelectLangCode, sc_MSGBINLangCode, sizeof( sc_SelectLangCode ) );	// 先頭から9言語分のデータを取得.

		// 言語数が9以上の場合は最大言語数に補正する.
		if( PNL_MAX_LANGUAGES < m_i32_MSGBINLangNum ) {
			uc_SelectLangNum = PNL_MAX_LANGUAGES;
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
			if( 0 == strncmp( psc_MSGBINLangCode,psc_SelectLangCode,SIZE_LANGCODE ) ) {
				memcpy( m_st_SelectLangData[m_uc_SelectLangNum].sc_LangCode, psc_MSGBINLangCode, SIZE_LANGCODE );
				m_st_SelectLangData[m_uc_SelectLangNum].uc_MSGBINIndex = i;
				m_uc_SelectLangNum++;
				break;
			}
			psc_SelectLangCode += SIZE_LANGCODE;		// 次の言語コードへ(8バイト分進める).
		}
		psc_MSGBINLangCode += SIZE_LANGCODE;		// 次の言語コードへ(8バイト分進める).
		psc_SelectLangCode = sc_SelectLangCode;		// 選択言語は先頭アドレスに戻す.
	}
}
// メッセージバイナリ内の順番取得.
uchar PNLC_SelectLangInfo::getMSGBINIndex( uchar uc_index ) {
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	if( m_uc_SelectLangNum <= uc_index ) {		// 選択言語数以上の場合はエラー.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return 255;
	}
	return m_st_SelectLangData[uc_index].uc_MSGBINIndex;
}
// 言語コード取得.
char* PNLC_SelectLangInfo::getLangCode( uchar uc_index ) {
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	if( m_uc_SelectLangNum <= uc_index ) {		// 選択言語数以上の場合はエラー.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return NULL;
	}
	if( ( uchar )( sizeof( m_st_SelectLangData ) / sizeof( m_st_SelectLangData[0] ) ) > uc_index ) {
		return m_st_SelectLangData[uc_index].sc_LangCode;
	} else {
		return NULL;
	}
}
// 選択言語数取得.
uchar PNLC_SelectLangInfo::getSelectLangNum() {
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	return m_uc_SelectLangNum;
}
// メッセージバイナリ内の言語数取得.
int32_t PNLC_SelectLangInfo::getMSGBINLangNum() {
	DEF_UPDATEDRAWPARTSINFO;		// comment 2016/03/30 OPS1-Sky長尾芳人 [操作性改善][不要描画] 不要描画検出機構対応.
	
	return m_i32_MSGBINLangNum;
}
//全選択言語のオフセットアドレス処理.
static int loadSelectLangKeymap( uchar* uc_RAMBuffer, uchar uc_RAMBufferSize, int32_t i32_OffsetTableKey, uchar uc_DataSize ) {

	int si_ret = -1;
	if( NULL == uc_RAMBuffer || 0 == uc_RAMBufferSize
	    || 0 == uc_DataSize|| 0 >= i32_OffsetTableKey ) {	// 引数チェック.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return si_ret;
	}

	uchar* puc_RAMBuffer = uc_RAMBuffer;		// 格納先バッファのアドレス.
	int32_t i32_BuffSize = 0;						// ローカルバッファのサイズ.

	i32_BuffSize = ( int32_t )uc_DataSize * c_SelectLangInfo.getMSGBINLangNum();	// 全言語数分のバッファサイズを算出.

	if( 0 >= i32_BuffSize ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return si_ret;
	}

	uchar uc_TmpBuf[i32_BuffSize];							// 一時用バッファ.
	memset( uc_TmpBuf, 0, sizeof( uc_TmpBuf ) );
	si_ret = loadLangData2Ram( uc_TmpBuf, i32_OffsetTableKey, sizeof( uc_TmpBuf ) );		// バイナリからデータを取得.
	if( -1 == si_ret ) {		//エラー発生時.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return si_ret;
	}

	for( uchar i = 0; i <  c_SelectLangInfo.getSelectLangNum() && i < PNL_MAX_LANGUAGES ; i++ ) {	// 選択言語数回ループする.
		if( ( int32_t )( sizeof( uc_TmpBuf ) / sizeof( uc_TmpBuf[0] ) ) > ( int32_t )( uc_DataSize * c_SelectLangInfo.getMSGBINIndex( i ) ) ) {
			// 選択言語のオフセットアドレス+サイズを格納先バッファ(計8バイト)をコピーする.
			memcpy( puc_RAMBuffer, ( uc_TmpBuf +  uc_DataSize * c_SelectLangInfo.getMSGBINIndex( i ) ),
			        uc_DataSize );
			puc_RAMBuffer += uc_DataSize;	// 格納先バッファのアドレスを次の領域(8バイト先)に進める
		}
	}
	return si_ret;
}
// メッセージバイナリ内固定データサイズ取得関数.
static int32_t GetFixedDataSize_FromMSGBIN() {
	int32_t	i32_langCount	 = c_SelectLangInfo.getMSGBINLangNum();	// メッセージバイナリ内の言語数を取得
	int32_t	i32_fixedDataSize = 0;

	// 固定データサイズを計算
	i32_fixedDataSize  = sizeof( unsigned char ) * SIZE_FIXED_DATA;
	i32_fixedDataSize = (int32_t)( i32_fixedDataSize + ( sizeof( unsigned char ) * SIZE_LANGCODE * i32_langCount ) );

	return i32_fixedDataSize;
}
