
#include "pdpc2_CLCD.h"
#include "panel/plibl/plibl_CLCD/Rasterize.h"

#include "system/sys/APIC_System.h"

#include <panel/plibh/plibh_CLCD/plibh_CLCD1.h>

#ifdef QT_PANEL
#include "../../../shlib/qtconnecter/includeApp/QCN_AppApi.h"
#endif	// #ifdef QT_PANEL.


// フォント種類.
#define	PNLE_FONTTYPE_GOTHIC	(0)	// ゴシック体.
#define	PNLE_FONTTYPE_MINCHO	(1)	// 明朝体.
#define	PNLE_FONTTYPE_MAIN		(2)	// 通常文字(主文字、副文字、特大、中大、大).
#define	PNLE_FONTTYPE_SUB		(3)	// 補助文字(補助情報).

#define PNLE_FONTTYPE_COUNT		(4)	// Fontの種類.

// 1Fontの情報.
struct PNL_OUTLINE_FONTBINARY {
	uint32_t	ul_fontOffset;	// オフセット.
	uint32_t	ul_fontSize;	// サイズ.
};

// アウトラインフォント管理用構造体.
struct PNL_OUTLINE_ONEFONT_DETAIL {
	char						c_langCode[8];					// 言語コード.
	PNL_OUTLINE_FONTBINARY		compressFont;					// 圧縮フォント情報.
	PNL_OUTLINE_FONTBINARY		decordFont[PNLE_FONTTYPE_COUNT];// 展開フォント情報.
};

struct PNL_OUTLINE_FONTHEADER {
	char						sc_version[32];					// バージョン情報.
	uchar						uc_fontLangCnt;					// 言語数.
	uchar						uc_buffer[3];					// 予約分(3バイト).
	PNL_OUTLINE_ONEFONT_DETAIL	fontInfo[TYPE_MAX_LANGUAGES];	// 言語別フォント情報(最大9言語).
};

static PNL_OUTLINE_FONTHEADER	g_PNL_OutLineFontHeader;			// アウトラインフォント管理情報.
static PNL_OUTLINE_FONTINFO		g_PNL_UseOutLineFontInfo;			// 使用するアウトラインフォント管理情報
static TYPE_Language			g_e_UseLang[TYPE_MAX_LANGUAGES];	// 使用する言語.

// フォントバッファ.
static uchar*	g_puc_fontBuffer = NULL;

// 圧縮Font情報取得.
// 言語コード取得
static PNL_OUTLINE_ONEFONT_DETAIL* PNL_GetOneLangInfo( TYPE_Language e_langType );

// 圧縮フォント取得

// 言語コードより言語種別取得
static Bool PNL_GetLangType( char* pc_LangCode, TYPE_Language* pLangType );
// 言語コード取得
static char* PNL_GetLangCode( TYPE_Language e_langType );

// 読み込む言語か否か.
static Bool PNL_isLang( char* pc_LangCode, TYPE_Language* pLangType = NULL );

// 選択可能な2バイト言語の取得.
static Bool	PNL_getUse2ByteLang( TYPE_Language* pe_lang );

// 使用可能なFont情報取得
static PNL_OUTLINE_ONEFONT* PNL_getUseFontInfo( TYPE_Language e_Lang );

#ifdef SIMULATOR
// SimWin用 エンディアン変換.
static uint32_t changeEndian( uint32_t );
// アウトラインフォントメモリ展開(ファイルは非圧縮に限る).
static Bool	loadOutlineFont( );
#endif
// キーボード言語の表示順を表示言語と同様に並び替える.
static	void	sortKeyBoardLanguage( PDPS_KeyMap* s_KeyMap, TYPE_Language* e_Language );
/**********************************************************************************************/
/* Font情報取得.																			  */
/**********************************************************************************************/
Bool	PNL_getOutLineFontInfo( uchar*					puc_fontBuf,		// (IN) Font情報展開用バッファ.
                                PNL_OUTLINE_FONTINFO*	p_fontInfo			// (OUT)フォント情報.
                           )
{
	// フォント情報初期化.
	memset( &g_PNL_OutLineFontHeader,	0, sizeof( g_PNL_OutLineFontHeader ) );
	memset( p_fontInfo,					0, sizeof( PNL_OUTLINE_FONTINFO ) );
	memset( g_e_UseLang,				0, sizeof( g_e_UseLang ) );
	memset( &g_PNL_UseOutLineFontInfo,	0, sizeof( g_PNL_UseOutLineFontInfo ) );

	g_puc_fontBuffer = puc_fontBuf;

	if( puc_fontBuf == NULL ) {
		// Font展開用バッファがNULLの場合は、Falseを返す.
		return False;
	}

	// ヘッダー情報取得.
	APIC_OutlineFont* pOutlineFontInfo;
	pOutlineFontInfo = 	APIC_OutlineFont::newInstance();
	if( pOutlineFontInfo->getFont3Data( 0, sizeof( g_PNL_OutLineFontHeader ), ( uchar* )&g_PNL_OutLineFontHeader ) == False ) {
		// ヘッダー情報の取得が出来ない場合はNGを返す.
		return False;
	}

	uint i,j;

	uchar langIndex = 0;

	uint32_t ui32_setOffset = 0;
	for( i=0; i<g_PNL_OutLineFontHeader.uc_fontLangCnt; i++ ) {
		uint32_t ui32_totalOffset	   = 0xFFFFFFFF;
		uint32_t ui32_totalSize		   = 0;

#ifdef SIMULATOR
		// ヘッダー情報はLittleEndianなので、SimWinで使う場合はBigEndianにする必要あり.
		g_PNL_OutLineFontHeader.fontInfo[i].compressFont.ul_fontOffset	= changeEndian( g_PNL_OutLineFontHeader.fontInfo[i].compressFont.ul_fontOffset );
		g_PNL_OutLineFontHeader.fontInfo[i].compressFont.ul_fontSize		= changeEndian( g_PNL_OutLineFontHeader.fontInfo[i].compressFont.ul_fontSize );
		for( j=0; j<sizeof( g_PNL_OutLineFontHeader.fontInfo[i].decordFont ) / sizeof( PNL_OUTLINE_FONTBINARY ); j++ ) {
			g_PNL_OutLineFontHeader.fontInfo[i].decordFont[j].ul_fontOffset = changeEndian( g_PNL_OutLineFontHeader.fontInfo[i].decordFont[j].ul_fontOffset );
			g_PNL_OutLineFontHeader.fontInfo[i].decordFont[j].ul_fontSize			  = changeEndian( g_PNL_OutLineFontHeader.fontInfo[i].decordFont[j].ul_fontSize );
		}
#endif

		if( PNL_isLang( g_PNL_OutLineFontHeader.fontInfo[i].c_langCode, &g_e_UseLang[langIndex] ) == False ) {
			continue;
		}

		p_fontInfo->fontInfo[langIndex].ul_compressFontOffset = g_PNL_OutLineFontHeader.fontInfo[i].compressFont.ul_fontOffset;
		p_fontInfo->fontInfo[langIndex].ul_compressFontSize   = g_PNL_OutLineFontHeader.fontInfo[i].compressFont.ul_fontSize;

		for( j=0; j<sizeof( g_PNL_OutLineFontHeader.fontInfo[i].decordFont ) / sizeof( PNL_OUTLINE_FONTBINARY ); j++ ) {
			uint32_t ui32_size	= g_PNL_OutLineFontHeader.fontInfo[i].decordFont[j].ul_fontSize;
			uint32_t ui32_offset	= g_PNL_OutLineFontHeader.fontInfo[i].decordFont[j].ul_fontOffset;

			if( ui32_size != 0 ) {
				ui32_totalSize += ui32_size;
				if( ui32_totalOffset > ui32_offset ) {  ui32_totalOffset = ui32_offset; }
			}
		}
		p_fontInfo->fontInfo[langIndex].ul_fontOffset = ui32_totalOffset + ui32_setOffset;
		p_fontInfo->fontInfo[langIndex].ul_fontSize   = ui32_totalSize;
		ui32_setOffset += ( ui32_totalOffset + ui32_totalSize );
		langIndex++;
	}

	p_fontInfo->uc_langFontCnt = langIndex;
	memcpy( &g_PNL_UseOutLineFontInfo, p_fontInfo, sizeof( g_PNL_UseOutLineFontInfo ) );

#ifdef SIMULATOR
	// SIMWINなら、ここでメモリ展開.
	return loadOutlineFont( );
#endif

	return True;
}


/******************************************************************/
/* 選択可能な2バイト言語の取得.									  */
/******************************************************************/
Bool PNL_getUse2ByteLang( TYPE_Language* pe_lang )
{
	Bool bRet = False;
	APIC_MachineSetting* pc_APIC_MachineSetting = APIC_MachineSetting::newInstance();

	TYPE_SystemType	   eSystemType	  = pc_APIC_MachineSetting->getSystemType();
	TYPE_SubSystemType eSubSystemType = pc_APIC_MachineSetting->getSubSystemType();

	switch( eSystemType ) {
		case TYPE_SystemType_JP:			// 日本仕向.
			*pe_lang = TYPE_LanguageJapanese;			// 日本語.
			bRet = True;
			break;
		case TYPE_SystemType_US:			// 北米仕向.
			*pe_lang = TYPE_LanguageSize;				// 選択可能な2バイト言語なし.
			break;
		case TYPE_SystemType_EU:
			switch( eSubSystemType ) {
				case TYPE_SubSystemType1:			// 欧州仕向.
				case TYPE_SubSystemType2:				// サウジＢ/ブラジル仕向.
					*pe_lang = TYPE_LanguageSize;		// 選択可能な2バイト言語なし.
					break;
				case TYPE_SubSystemType6:			// BAU仕向(ハングル).
					*pe_lang = TYPE_LanguageHangul;		// ハングル.
					bRet = True;
					break;
				case TYPE_SubSystemType4:			// 中国仕向.
					*pe_lang = TYPE_LanguageChinese2;	// 簡体字中国語.
					bRet = True;
					break;
				case TYPE_SubSystemType3:			// BAU仕向(台湾).
				case TYPE_SubSystemType5:			// 台湾仕向.
					*pe_lang = TYPE_LanguageChinese1;	// 繁体字中国語.
					bRet = True;
					break;
			}
			break;
	}

	return bRet;
}

/******************************************************************/
/* 読み込む言語か否か.											  */
/******************************************************************/
Bool PNL_isLang( char* pc_LangCode, TYPE_Language* pe_LangType )
{
	TYPE_Language eLang;
	if( PNL_GetLangType( pc_LangCode, &eLang ) == False ) {
		return False;
	}

	if( pe_LangType != NULL ) {
		*pe_LangType = eLang;
	}

	if( eLang == TYPE_LanguageAmerican ) {
		// EFIGSなら読み込む
		return True;
	}

#ifdef SIMULATOR
	return True;
#endif


	// 選択可能な2Byte言語を取得.
	static TYPE_Language okLang;
	static Bool bInitFlg = False;
	if( bInitFlg == False ) {
		if( PNL_getUse2ByteLang( &okLang ) == False ) {
			return False;
		}
		bInitFlg = True;
	}

	return( okLang == eLang );
}

/******************************************************************/
/* アウトラインフォント取得.									  */
/******************************************************************/
uchar*	PNL_getOutlineFont( TYPE_Language e_Lang, uchar uc_fontType )
{
	// フォントバッファが取得できていないときは当然アウトラインフォントも取得不可.
	if( g_puc_fontBuffer == NULL ) { return NULL; }

#ifdef SIMULATOR
	DEBUG_APIC_OutlineFont* pOutlineFont = DEBUG_APIC_OutlineFont::newInstance();
	uchar* pTestFont =  pOutlineFont->loadFontFile();
	if( pTestFont != NULL ) { return pTestFont; }
#else
	APIC_OutlineFont* pOutlineFont = APIC_OutlineFont::newInstance();
#endif

	if( pOutlineFont->checkFont3Data() == TYPE_Font3DataNG ) {
		// Fontの展開に失敗しているときは、アウトラインフォント取得不可.
		return NULL;
	}

	TYPE_OutlineFontType e_fontType = uc_fontType;

	PNL_OUTLINE_ONEFONT_DETAIL* pFontInfo = PNL_GetOneLangInfo( e_Lang );
	if( pFontInfo == NULL ) {
		return NULL;
	}

	// 実際にFontがある先頭アドレスを検索.
	PNL_OUTLINE_ONEFONT* pOneFont = PNL_getUseFontInfo( e_Lang );
	if( pOneFont == NULL ) {
		// RAMに展開していない場合はNULL.
		return NULL;
	}

	uint32_t ui32_fontOffset = 0;
	if( e_fontType == TYPE_OutlineFont_GOTHIC ) {
		if( pFontInfo->decordFont[PNLE_FONTTYPE_GOTHIC].ul_fontSize != 0 ) {
			ui32_fontOffset = pFontInfo->decordFont[PNLE_FONTTYPE_GOTHIC].ul_fontOffset;
		} else if( pFontInfo->decordFont[PNLE_FONTTYPE_MINCHO].ul_fontSize != 0 ) {
			ui32_fontOffset = pFontInfo->decordFont[PNLE_FONTTYPE_MINCHO].ul_fontOffset;
		} else {
			return NULL;
		}
	} else if( e_fontType == TYPE_OutlineFont_MINCHO ) {
		if( pFontInfo->decordFont[PNLE_FONTTYPE_MINCHO].ul_fontSize != 0 ) {
			ui32_fontOffset = pFontInfo->decordFont[PNLE_FONTTYPE_MINCHO].ul_fontOffset;
		} else  if( pFontInfo->decordFont[PNLE_FONTTYPE_GOTHIC].ul_fontSize != 0 ) {
			ui32_fontOffset = pFontInfo->decordFont[PNLE_FONTTYPE_GOTHIC].ul_fontOffset;
		} else {
			return NULL;
		}
	} else if( e_fontType == TYPE_OutlineFont_Main ) {
		if( pFontInfo->decordFont[PNLE_FONTTYPE_MAIN].ul_fontSize != 0 ) {
			ui32_fontOffset = pFontInfo->decordFont[PNLE_FONTTYPE_MAIN].ul_fontOffset;
		} else  if( pFontInfo->decordFont[PNLE_FONTTYPE_GOTHIC].ul_fontSize != 0 ) {
			ui32_fontOffset = pFontInfo->decordFont[PNLE_FONTTYPE_GOTHIC].ul_fontOffset;
		} else {
			return NULL;
		}
	} else if( e_fontType == TYPE_OutlineFont_Sub ) {
		if( pFontInfo->decordFont[PNLE_FONTTYPE_SUB].ul_fontSize != 0 ) {
			ui32_fontOffset = pFontInfo->decordFont[PNLE_FONTTYPE_SUB].ul_fontOffset;
		} else  if( pFontInfo->decordFont[PNLE_FONTTYPE_GOTHIC].ul_fontSize != 0 ) {
			ui32_fontOffset = pFontInfo->decordFont[PNLE_FONTTYPE_GOTHIC].ul_fontOffset;
		} else {
			return NULL;
		}
	} else {
		// フォントの種類が増えたら、処理を追加すること.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return NULL;
	}

	return &g_puc_fontBuffer[ui32_fontOffset + pOneFont->ul_fontOffset];
}

/******************************************************************/
/* 使用可能なFont情報取得										  */
/******************************************************************/
PNL_OUTLINE_ONEFONT* PNL_getUseFontInfo( TYPE_Language e_Lang )
{
	static const TYPE_Language EFIGS_LANG[] = {
		TYPE_LanguageAmerican,
		TYPE_LanguageGerman,
		TYPE_LanguageFrench,
		TYPE_LanguageItalian,
		TYPE_LanguageSpanish,
	};

	uint i;
	for( i=0; i<sizeof( EFIGS_LANG ) / sizeof( TYPE_Language ); i++ ) {
		if( EFIGS_LANG[i] == e_Lang ) {
			// EFIGS言語なら、一旦言語をアメリカにする
			e_Lang = TYPE_LanguageAmerican;
			break;
		}
	}

	for( i=0; i<g_PNL_UseOutLineFontInfo.uc_langFontCnt; i++ ) {
		if( g_e_UseLang[i] == e_Lang ) {
			return &( g_PNL_UseOutLineFontInfo.fontInfo[i] );
		}
	}
	return NULL;
}

/******************************************************************/
/* 使用可能な言語のFontか										  */
/******************************************************************/
Bool PNL_isUseFontLang( TYPE_Language e_Lang )
{
	static const TYPE_Language EFIGS_LANG[] = {
		TYPE_LanguageAmerican,
		TYPE_LanguageGerman,
		TYPE_LanguageFrench,
		TYPE_LanguageItalian,
		TYPE_LanguageSpanish,
	};

	uint i;
	for( i=0; i<sizeof( EFIGS_LANG ) / sizeof( TYPE_Language ); i++ ) {
		if( EFIGS_LANG[i] == e_Lang ) {
			// EFIGS言語なら、一旦言語をアメリカにする
			e_Lang = TYPE_LanguageAmerican;
			break;
		}
	}

	for( i=0; i<g_PNL_UseOutLineFontInfo.uc_langFontCnt; i++ ) {
		if( g_e_UseLang[i] == e_Lang ) {
			return True;
		}
	}
	return False;
}


/******************************************************************/
/* 言語コード取得.												  */
/******************************************************************/
struct OUTLINE_LANGINFO {
	TYPE_Language	e_lang;
	char*			c_langCode;
};
static const OUTLINE_LANGINFO g_langInfo[] = {
	{ TYPE_LanguageAmerican,	"EFIGS"	},
	{ TYPE_LanguageGerman,		"EFIGS"	},
	{ TYPE_LanguageFrench,		"EFIGS"	},
	{ TYPE_LanguageItalian,		"EFIGS"	},
	{ TYPE_LanguageSpanish,		"EFIGS"	},
	{ TYPE_LanguageJapanese,	"ja"	},
	{ TYPE_LanguageHangul,		"kr"	},
	{ TYPE_LanguageChinese1,	"zh-TW"	},
	{ TYPE_LanguageChinese2,	"zh-CN"	},
};

char* PNL_GetLangCode( TYPE_Language e_langType )
{
	char* pc_langCode = NULL;
	for( uint i=0; i<sizeof( g_langInfo )/sizeof( OUTLINE_LANGINFO ); i++ ) {
		if( g_langInfo[i].e_lang == e_langType ) {
			pc_langCode = g_langInfo[i].c_langCode;
			break;
		}
	}

	if( pc_langCode == NULL ) {
		// OUTLINEフォント未対応言語.
		// (他の言語が追加された場合は、langInfo に追加してください。)
		SYSF_logMsg( "PNL_GetOneLangInfo():この言語(%d)はアウトラインフォント対応していません。", ( int )e_langType, 0, 0, 0, 0, 0 );
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}
	return pc_langCode;
}

/******************************************************************/
/* 言語コードより　言語種別取得									  */
/******************************************************************/
Bool PNL_GetLangType( char* pc_LangCode, TYPE_Language* pLangType )
{
	for( uint i=0; i<sizeof( g_langInfo )/sizeof( OUTLINE_LANGINFO ); i++ ) {
		if( strcmp( pc_LangCode, g_langInfo[i].c_langCode ) == 0 ) {
			*pLangType = g_langInfo[i].e_lang;
			return True;
		}
	}
	return False;
}
/******************************************************************/
/* 言語コードより　EFIGS_LANGかどうかを判定						  */
/******************************************************************/
Bool PNL_isEFIGSLang( TYPE_Language e_Lang )
{
	static const TYPE_Language EFIGS_LANG[] = {
		TYPE_LanguageAmerican,
		TYPE_LanguageGerman,
		TYPE_LanguageFrench,
		TYPE_LanguageItalian,
		TYPE_LanguageSpanish,
	};

	uint i;
	for( i=0; i<sizeof( EFIGS_LANG ) / sizeof( TYPE_Language ); i++ ) {
		if( EFIGS_LANG[i] == e_Lang ) {
			// EFIGS言語なら、一旦言語をアメリカにする
			return True;
		}
	}
	return False;
}

/******************************************************************/
/* とある言語のFont情報取得.									  */
/******************************************************************/
PNL_OUTLINE_ONEFONT_DETAIL* PNL_GetOneLangInfo( TYPE_Language e_langType )
{
	char* pc_langCode = PNL_GetLangCode( e_langType );

	// アウトラインフォントヘッダーより、指定言語のFont情報を探す.
	PNL_OUTLINE_ONEFONT_DETAIL* pFontInfo = NULL;
	int i;
	for( i=0; i< g_PNL_OutLineFontHeader.uc_fontLangCnt; i++ ) {
		if( NULL != pc_langCode
		    && strcmp( g_PNL_OutLineFontHeader.fontInfo[i].c_langCode, pc_langCode ) == 0 ) {
			pFontInfo = &( g_PNL_OutLineFontHeader.fontInfo[i] );
			break;
		}
	}

	return pFontInfo;
}

/******************************************************************/
/* 圧縮Font情報取得.											  */
/******************************************************************/


/*
 * 以下SimWin用デバッグ関数.
 */
#ifdef SIMULATOR
// SimWin用 エンディアン変換.
uint32_t changeEndian( uint32_t data )
{
	union BIGLITTLE {
		uint32_t	ui32_Data;
		uchar		ucByte[sizeof( uint32_t )];
	};

	BIGLITTLE source;
	BIGLITTLE dest;

	source.ui32_Data = data;

	uint i,j;

	for( i=0, j=sizeof( uint32_t )-1; i<sizeof( uint32_t ); i++, j-- ) {
		dest.ucByte[j] = source.ucByte[i];
	}

	return dest.ui32_Data;
}

// SimWin用 アウトラインフォントメモリ展開.
// 注：非圧縮ファイルしか対応していない。
Bool loadOutlineFont( )
{
	static const char* fontFileName = "DONAUSimWin_FO3.bin";
	static const int fontSize = 40 * 1024*1024;

	FILE* fp = SYSF_fopen( fontFileName, "rb" );
	if( fp == NULL ) {
		return False;
	}

	uchar* puc_Buf = ( uchar* )malloc( fontSize );

	fread( puc_Buf, sizeof( char ), fontSize, fp );
	SYSF_fclose( fp );

	// Fontの先頭アドレスまで移動.
	uchar* puc_StartFont = &puc_Buf[sizeof( PNL_OUTLINE_FONTHEADER ) ];
	int i,j, k;

	uint32_t offset = 0;
	for( i=0; i<g_PNL_UseOutLineFontInfo.uc_langFontCnt; i++ ) {
		TYPE_Language e_lang = g_e_UseLang[i];
		char* c_langCode = PNL_GetLangCode( e_lang );
		PNL_OUTLINE_ONEFONT_DETAIL* pOneFont = NULL;

		// フォントヘッダー情報より、使用言語の情報取得.
		for( j=0; j<g_PNL_OutLineFontHeader.uc_fontLangCnt; j++ ) {
			if( strcmp( c_langCode, g_PNL_OutLineFontHeader.fontInfo[j].c_langCode ) == 0 ) {
				pOneFont = &( g_PNL_OutLineFontHeader.fontInfo[j] );
				break;
			}
		}
		if( pOneFont == NULL ) {
			ASSERT( 0 );
			free( puc_Buf );
			return False;
		}

		// コピーサイズの算出
		uint32_t ui32_copySize = 0;
		for( j=0; j<PNLE_FONTTYPE_COUNT; j++ ) {
			ui32_copySize += pOneFont->decordFont[j].ul_fontSize;
		}

		// コピー元のoffset算出
		uint32_t ui32_srcOffset = 0;
		for( j=0; j<g_PNL_OutLineFontHeader.uc_fontLangCnt; j++ ) {
			if( &( g_PNL_OutLineFontHeader.fontInfo[j] ) == pOneFont ) {
				break;
			}

			for( k=0; k<PNLE_FONTTYPE_COUNT; k++ ) {
				ui32_srcOffset += g_PNL_OutLineFontHeader.fontInfo[j].decordFont[k].ul_fontSize;
			}
		}
		memcpy( &g_puc_fontBuffer[offset], &puc_StartFont[ui32_srcOffset], ui32_copySize );
		offset += ui32_copySize;
	}

	free( puc_Buf );
	return True;
}

static Bool g_useOKFlg = False;

/*
 * フォントバイナリ　ヘッダー取得.
 */
Bool DEBUG_APIC_OutlineFont::getFont3Data( uint32_t ul_offset, uint32_t ul_size, uchar* puc_Buf )
{
	static const char* pcFontBinaryFile = "DONAUSimWin_FO3.bin";

	FILE* fp = SYSF_fopen( pcFontBinaryFile, "rb" );
	if( fp == NULL ) {
		return False;
	}

	uint32_t readSize = fread( &puc_Buf[ul_offset], sizeof( char ), ul_size, fp );
	SYSF_fclose( fp );

	if( readSize < ul_size ) {
		return False;
	}

	g_useOKFlg = True;

	return True;
}

// アウトラインフォントデータのメモリ設定確認処理.
Bool DEBUG_APIC_OutlineFont::checkFont3Data( void )
{
	return g_useOKFlg;
}

uchar* DEBUG_APIC_OutlineFont::loadFontFile()
{
	static uchar* pcFontBinary = NULL;


	FILE* fp = SYSF_fopen( "c:\\font.txt", "r" );
	if( fp == NULL ) { return NULL; }

	if( pcFontBinary == NULL ) {
		pcFontBinary = ( uchar* )malloc( 15 * 1024 * 1024 );
	}

	char line[1024];
	fgets( line, 1023, fp );
	SYSF_fclose( fp );

	for( int i=strlen( line )-1; i>=0; i-- ) {
		if( line[i] <= 0x20 ) { line[i] = '\0';} else { break; }
	}



	fp = SYSF_fopen( line, "rb" );
	if( fp == NULL ) { return NULL; }
	fread( pcFontBinary, 1, 15 * 1024 * 1024,  fp );

	SYSF_fclose( fp );

	return pcFontBinary;
}

#endif

// キーモードに対応するキーMAP取得情報テーブル.
const PDPS_USBKeyModeToMapInfo PDPC_KeyMapManager::tbl_USBKeyModeToMapInfo[] = {
	{	TYPE_USBKeyBoardMode_EnglishHalfBig,			TYPE_LanguageAmerican,		PDPE_USBKeyKind_CapsLock	},	//英数記(半)大文字.
	{	TYPE_USBKeyBoardMode_EnglishHalfSmall,			TYPE_LanguageAmerican,		PDPE_USBKeyKind_Normal		},	//英数記(半)小文字.
	{	TYPE_USBKeyBoardMode_EnglishAllBig,				TYPE_LanguageAmerican,		PDPE_USBKeyKind_CapsLock	},	//英数記(全)大文字.
	{	TYPE_USBKeyBoardMode_EnglishAllSmall,			TYPE_LanguageAmerican,		PDPE_USBKeyKind_Normal		},	//英数記(全)小文字.
	{	TYPE_USBKeyBoardMode_FrenchBig,					TYPE_LanguageFrench,		PDPE_USBKeyKind_CapsLock	},	//フランス大文字.
	{	TYPE_USBKeyBoardMode_FrenchSmall,				TYPE_LanguageFrench,		PDPE_USBKeyKind_Normal		},	//フランス小文字.
	{	TYPE_USBKeyBoardMode_ItalianBig,				TYPE_LanguageItalian,		PDPE_USBKeyKind_CapsLock	},	//イタリア大文字.
	{	TYPE_USBKeyBoardMode_ItalianSmall,				TYPE_LanguageItalian,		PDPE_USBKeyKind_Normal		},	//イタリア小文字.
	{	TYPE_USBKeyBoardMode_GermanBig,					TYPE_LanguageGerman,		PDPE_USBKeyKind_CapsLock	},	//ドイツ大文字.
	{	TYPE_USBKeyBoardMode_GermanSmall,				TYPE_LanguageGerman,		PDPE_USBKeyKind_Normal		},	//ドイツ小文字.
	{	TYPE_USBKeyBoardMode_SpanishBig,				TYPE_LanguageSpanish,		PDPE_USBKeyKind_CapsLock	},	//スペイン大文字.
	{	TYPE_USBKeyBoardMode_SpanishSmall,				TYPE_LanguageSpanish,		PDPE_USBKeyKind_Normal		},	//スペイン小文字.
	{	TYPE_USBKeyBoardMode_EnglishUKBig,				TYPE_LanguageAmerican,		PDPE_USBKeyKind_CapsLock	},	//イギリス英語大文字.
	{	TYPE_USBKeyBoardMode_EnglishUKSmall,			TYPE_LanguageAmerican,		PDPE_USBKeyKind_Normal		},	//イギリス英語小文字.
	{	TYPE_USBKeyBoardMode_FrenchBig_OnlyASCII,		TYPE_LanguageFrench,		PDPE_USBKeyKind_CapsLock	},	//フランス大文字_英数記のみ.
	{	TYPE_USBKeyBoardMode_FrenchSmall_OnlyASCII,		TYPE_LanguageFrench,		PDPE_USBKeyKind_Normal		},	//フランス小文字_英数記のみ.
	{	TYPE_USBKeyBoardMode_ItalianBig_OnlyASCII,		TYPE_LanguageItalian,		PDPE_USBKeyKind_CapsLock	},	//イタリア大文字_英数記のみ.
	{	TYPE_USBKeyBoardMode_ItalianSmall_OnlyASCII,	TYPE_LanguageItalian,		PDPE_USBKeyKind_Normal		},	//イタリア小文字_英数記のみ.
	{	TYPE_USBKeyBoardMode_GermanBig_OnlyASCII,		TYPE_LanguageGerman,		PDPE_USBKeyKind_CapsLock	},	//ドイツ大文字_英数記のみ.
	{	TYPE_USBKeyBoardMode_GermanSmall_OnlyASCII,		TYPE_LanguageGerman,		PDPE_USBKeyKind_Normal		},	//ドイツ小文字_英数記のみ.
	{	TYPE_USBKeyBoardMode_SpanishBig_OnlyASCII,		TYPE_LanguageSpanish,		PDPE_USBKeyKind_CapsLock	},	//スペイン大文字_英数記のみ.
	{	TYPE_USBKeyBoardMode_SpanishSmall_OnlyASCII,	TYPE_LanguageSpanish,		PDPE_USBKeyKind_Normal		},	//スペイン小文字_英数記のみ.
	{	TYPE_USBKeyBoardMode_EnglishUKBig_OnlyASCII,	TYPE_LanguageAmerican,		PDPE_USBKeyKind_CapsLock	},	//イギリス英語大文字_英数記のみ.
	{	TYPE_USBKeyBoardMode_EnglishUKSmall_OnlyASCII,	TYPE_LanguageAmerican,		PDPE_USBKeyKind_Normal		},	//イギリス英語小文字_英数記のみ.
	{	TYPE_USBKeyBoardMode_DanishSmall,				TYPE_LanguageDanish,		PDPE_USBKeyKind_Normal		},	// デンマーク小文字.
	{	TYPE_USBKeyBoardMode_DanishBig,					TYPE_LanguageDanish,		PDPE_USBKeyKind_CapsLock	},	// デンマーク大文字.
	{	TYPE_USBKeyBoardMode_DutchSmall,				TYPE_LanguageDutch,			PDPE_USBKeyKind_Normal		},	// オランダ小文字.
	{	TYPE_USBKeyBoardMode_DutchBig,					TYPE_LanguageDutch,			PDPE_USBKeyKind_CapsLock	},	// オランダ大文字.
	{	TYPE_USBKeyBoardMode_PortugueseSmall,			TYPE_LanguagePortuguese,	PDPE_USBKeyKind_Normal		},	// ポルトガル小文字.
	{	TYPE_USBKeyBoardMode_PortugueseBig,				TYPE_LanguagePortuguese,	PDPE_USBKeyKind_CapsLock	},	// ポルトガル大文字.
	{	TYPE_USBKeyBoardMode_SwedishSmall,				TYPE_LanguageSwedish,		PDPE_USBKeyKind_Normal		},	// スウェーデン小文字.
	{	TYPE_USBKeyBoardMode_SwedishBig,				TYPE_LanguageSwedish,		PDPE_USBKeyKind_CapsLock	},	// スウェーデン大文字.
	{	TYPE_USBKeyBoardMode_NorwegianSmall,			TYPE_LanguageNorwegian,		PDPE_USBKeyKind_Normal		},	// ノルウェー小文字.
	{	TYPE_USBKeyBoardMode_NorwegianBig,				TYPE_LanguageNorwegian,		PDPE_USBKeyKind_CapsLock	},	// ノルウェー大文字.
	{	TYPE_USBKeyBoardMode_FinnishSmall,				TYPE_LanguageFinnish,		PDPE_USBKeyKind_Normal		},	// フィンランド小文字.
	{	TYPE_USBKeyBoardMode_FinnishBig,				TYPE_LanguageFinnish,		PDPE_USBKeyKind_CapsLock	},	// フィンランド大文字.
	{	TYPE_USBKeyBoardMode_CzechSmall,				TYPE_LanguageCzech,			PDPE_USBKeyKind_Normal		},	// チェコ小文字.
	{	TYPE_USBKeyBoardMode_CzechBig,					TYPE_LanguageCzech,			PDPE_USBKeyKind_CapsLock	},	// チェコ大文字.
	{	TYPE_USBKeyBoardMode_HangarianSmall,			TYPE_LanguageHangarian,		PDPE_USBKeyKind_Normal		},	// ハンガリー小文字.
	{	TYPE_USBKeyBoardMode_HangarianBig,				TYPE_LanguageHangarian,		PDPE_USBKeyKind_CapsLock	},	// ハンガリー大文字.
	{	TYPE_USBKeyBoardMode_GreekSmall,				TYPE_LanguageGreek,			PDPE_USBKeyKind_Normal		},	// ギリシア小文字.
	{	TYPE_USBKeyBoardMode_GreekBig,					TYPE_LanguageGreek,			PDPE_USBKeyKind_CapsLock	},	// ギリシア大文字.
	{	TYPE_USBKeyBoardMode_TurkishSmall,				TYPE_LanguageTurkish,		PDPE_USBKeyKind_Normal		},	// トルコ小文字.
	{	TYPE_USBKeyBoardMode_TurkishBig,				TYPE_LanguageTurkish,		PDPE_USBKeyKind_CapsLock	},	// トルコ大文字.
	{	TYPE_USBKeyBoardMode_RussianSmall,				TYPE_LanguageRussian,		PDPE_USBKeyKind_Normal		},	// ロシア小文字.
	{	TYPE_USBKeyBoardMode_RussianBig,				TYPE_LanguageRussian,		PDPE_USBKeyKind_CapsLock	},	// ロシア大文字.
	{	TYPE_USBKeyBoardMode_CroatianSmall,				TYPE_LanguageCroatian,		PDPE_USBKeyKind_Normal		},	// クロアチア小文字.
	{	TYPE_USBKeyBoardMode_CroatianBig,				TYPE_LanguageCroatian,		PDPE_USBKeyKind_CapsLock	},	// クロアチア大文字.
	{	TYPE_USBKeyBoardMode_UkrainianSmall,			TYPE_LanguageUkrainian,		PDPE_USBKeyKind_Normal		},	// ウクライナ小文字.
	{	TYPE_USBKeyBoardMode_UkrainianBig,				TYPE_LanguageUkrainian,		PDPE_USBKeyKind_CapsLock	},	// ウクライナ大文字.
	{	TYPE_USBKeyBoardMode_RomanianSmall,				TYPE_LanguageRomanian,		PDPE_USBKeyKind_Normal		},	// ルーマニア小文字.
	{	TYPE_USBKeyBoardMode_RomanianBig,				TYPE_LanguageRomanian,		PDPE_USBKeyKind_CapsLock	},	// ルーマニア大文字.
	{	TYPE_USBKeyBoardMode_SlovakSmall,				TYPE_LanguageSlovak,		PDPE_USBKeyKind_Normal		},	// スロベキア小文字.
	{	TYPE_USBKeyBoardMode_SlovakBig,					TYPE_LanguageSlovak,		PDPE_USBKeyKind_CapsLock	},	// スロベキア大文字.
	{	TYPE_USBKeyBoardMode_PolishSmall,				TYPE_LanguagePolish,		PDPE_USBKeyKind_Normal		},	// ポーランド小文字.
	{	TYPE_USBKeyBoardMode_PolishBig,					TYPE_LanguagePolish,		PDPE_USBKeyKind_CapsLock	},	// ポーランド大文字.
	{	TYPE_USBKeyBoardMode_DanishSmall_OnlyASCII,		TYPE_LanguageDanish,		PDPE_USBKeyKind_Normal		},	// デンマーク小文字.
	{	TYPE_USBKeyBoardMode_DanishBig_OnlyASCII,		TYPE_LanguageDanish,		PDPE_USBKeyKind_CapsLock	},	// デンマーク大文字.
	{	TYPE_USBKeyBoardMode_DutchSmall_OnlyASCII,		TYPE_LanguageDutch,			PDPE_USBKeyKind_Normal		},	// オランダ小文字.
	{	TYPE_USBKeyBoardMode_DutchBig_OnlyASCII,		TYPE_LanguageDutch,			PDPE_USBKeyKind_CapsLock	},	// オランダ大文字.
	{	TYPE_USBKeyBoardMode_PortugueseSmall_OnlyASCII,	TYPE_LanguagePortuguese,	PDPE_USBKeyKind_Normal		},	// ポルトガル小文字.
	{	TYPE_USBKeyBoardMode_PortugueseBig_OnlyASCII,	TYPE_LanguagePortuguese,	PDPE_USBKeyKind_CapsLock	},	// ポルトガル大文字.
	{	TYPE_USBKeyBoardMode_SwedishSmall_OnlyASCII,	TYPE_LanguageSwedish,		PDPE_USBKeyKind_Normal		},	// スウェーデン小文字.
	{	TYPE_USBKeyBoardMode_SwedishBig_OnlyASCII,		TYPE_LanguageSwedish,		PDPE_USBKeyKind_CapsLock	},	// スウェーデン大文字.
	{	TYPE_USBKeyBoardMode_NorwegianSmall_OnlyASCII,	TYPE_LanguageNorwegian,		PDPE_USBKeyKind_Normal		},	// ノルウェー小文字.
	{	TYPE_USBKeyBoardMode_NorwegianBig_OnlyASCII,	TYPE_LanguageNorwegian,		PDPE_USBKeyKind_CapsLock	},	// ノルウェー大文字.
	{	TYPE_USBKeyBoardMode_FinnishSmall_OnlyASCII,	TYPE_LanguageFinnish,		PDPE_USBKeyKind_Normal		},	// フィンランド小文字.
	{	TYPE_USBKeyBoardMode_FinnishBig_OnlyASCII,		TYPE_LanguageFinnish,		PDPE_USBKeyKind_CapsLock	},	// フィンランド大文字.
	{	TYPE_USBKeyBoardMode_CzechSmall_OnlyASCII,		TYPE_LanguageCzech,			PDPE_USBKeyKind_Normal		},	// チェコ小文字.
	{	TYPE_USBKeyBoardMode_CzechBig_OnlyASCII,		TYPE_LanguageCzech,			PDPE_USBKeyKind_CapsLock	},	// チェコ大文字.
	{	TYPE_USBKeyBoardMode_HangarianSmall_OnlyASCII,	TYPE_LanguageHangarian,		PDPE_USBKeyKind_Normal		},	// ハンガリー小文字.
	{	TYPE_USBKeyBoardMode_HangarianBig_OnlyASCII,	TYPE_LanguageHangarian,		PDPE_USBKeyKind_CapsLock	},	// ハンガリー大文字.
	{	TYPE_USBKeyBoardMode_GreekSmall_OnlyASCII,		TYPE_LanguageGreek,			PDPE_USBKeyKind_Normal		},	// ギリシア小文字.
	{	TYPE_USBKeyBoardMode_GreekBig_OnlyASCII,		TYPE_LanguageGreek,			PDPE_USBKeyKind_CapsLock	},	// ギリシア大文字.
	{	TYPE_USBKeyBoardMode_TurkishSmall_OnlyASCII,	TYPE_LanguageTurkish,		PDPE_USBKeyKind_Normal		},	// トルコ小文字.
	{	TYPE_USBKeyBoardMode_TurkishBig_OnlyASCII,		TYPE_LanguageTurkish,		PDPE_USBKeyKind_CapsLock	},	// トルコ大文字.
	{	TYPE_USBKeyBoardMode_RussianSmall_OnlyASCII,	TYPE_LanguageRussian,		PDPE_USBKeyKind_Normal		},	// ロシア小文字.
	{	TYPE_USBKeyBoardMode_RussianBig_OnlyASCII,		TYPE_LanguageRussian,		PDPE_USBKeyKind_CapsLock	},	// ロシア大文字.
	{	TYPE_USBKeyBoardMode_CroatianSmall_OnlyASCII,	TYPE_LanguageCroatian,		PDPE_USBKeyKind_Normal		},	// クロアチア小文字.
	{	TYPE_USBKeyBoardMode_CroatianBig_OnlyASCII,		TYPE_LanguageCroatian,		PDPE_USBKeyKind_CapsLock	},	// クロアチア大文字.
	{	TYPE_USBKeyBoardMode_UkrainianSmall_OnlyASCII,	TYPE_LanguageUkrainian,		PDPE_USBKeyKind_Normal		},	// ウクライナ小文字.
	{	TYPE_USBKeyBoardMode_UkrainianBig_OnlyASCII,	TYPE_LanguageUkrainian,		PDPE_USBKeyKind_CapsLock	},	// ウクライナ大文字.
	{	TYPE_USBKeyBoardMode_RomanianSmall_OnlyASCII,	TYPE_LanguageRomanian,		PDPE_USBKeyKind_Normal		},	// ルーマニア小文字.
	{	TYPE_USBKeyBoardMode_RomanianBig_OnlyASCII,		TYPE_LanguageRomanian,		PDPE_USBKeyKind_CapsLock	},	// ルーマニア大文字.
	{	TYPE_USBKeyBoardMode_SlovakSmall_OnlyASCII,		TYPE_LanguageSlovak,		PDPE_USBKeyKind_Normal		},	// スロベキア小文字.
	{	TYPE_USBKeyBoardMode_SlovakBig_OnlyASCII,		TYPE_LanguageSlovak,		PDPE_USBKeyKind_CapsLock	},	// スロベキア大文字.
	{	TYPE_USBKeyBoardMode_PolishSmall_OnlyASCII,		TYPE_LanguagePolish,		PDPE_USBKeyKind_Normal		},	// ポーランド小文字.
	{	TYPE_USBKeyBoardMode_PolishBig_OnlyASCII,		TYPE_LanguagePolish,		PDPE_USBKeyKind_CapsLock	},	// ポーランド大文字.
	{	TYPE_USBKeyBoardMode_CatalanSmall,				TYPE_LanguageCatalan,		PDPE_USBKeyKind_Normal		},	// カタロニア小文字.
	{	TYPE_USBKeyBoardMode_CatalanBig,				TYPE_LanguageCatalan,		PDPE_USBKeyKind_CapsLock	},	// カタロニア大文字.
	{	TYPE_USBKeyBoardMode_SloveneSmall,				TYPE_LanguageSlovene,		PDPE_USBKeyKind_Normal		},	// スロベニア小文字.
	{	TYPE_USBKeyBoardMode_SloveneBig,				TYPE_LanguageSlovene,		PDPE_USBKeyKind_CapsLock	},	// スロベニア大文字.
	{	TYPE_USBKeyBoardMode_CatalanSmall_OnlyASCII,	TYPE_LanguageCatalan,		PDPE_USBKeyKind_Normal		},	// カタロニア小文字_英数記のみ.
	{	TYPE_USBKeyBoardMode_CatalanBig_OnlyASCII,		TYPE_LanguageCatalan,		PDPE_USBKeyKind_CapsLock	},	// カタロニア大文字_英数記のみ.
	{	TYPE_USBKeyBoardMode_SloveneSmall_OnlyASCII,	TYPE_LanguageSlovene,		PDPE_USBKeyKind_Normal		},	// スロベニア小文字_英数記のみ.
	{	TYPE_USBKeyBoardMode_SloveneBig_OnlyASCII,		TYPE_LanguageSlovene,		PDPE_USBKeyKind_CapsLock	},	// スロベニア大文字_英数記のみ.
	{	TYPE_USBKeyBoardMode_BulgarianSmall,			TYPE_LanguageBulgarian,		PDPE_USBKeyKind_Normal		},	// ブルガリア小文字.
	{	TYPE_USBKeyBoardMode_BulgarianBig,				TYPE_LanguageBulgarian,		PDPE_USBKeyKind_CapsLock	},	// ブルガリア大文字.
	{	TYPE_USBKeyBoardMode_BasqueSmall,				TYPE_LanguageBasque,		PDPE_USBKeyKind_Normal		},	// バスク小文字.
	{	TYPE_USBKeyBoardMode_BasqueBig,					TYPE_LanguageBasque,		PDPE_USBKeyKind_CapsLock	},	// バスク大文字.
	{	TYPE_USBKeyBoardMode_BulgarianSmall_OnlyASCII,	TYPE_LanguageBulgarian,		PDPE_USBKeyKind_Normal		},	// ブルガリア小文字_英数記のみ.
	{	TYPE_USBKeyBoardMode_BulgarianBig_OnlyASCII,	TYPE_LanguageBulgarian,		PDPE_USBKeyKind_CapsLock	},	// ブルガリア大文字_英数記のみ.
	{	TYPE_USBKeyBoardMode_BasqueSmall_OnlyASCII,		TYPE_LanguageBasque,		PDPE_USBKeyKind_Normal		},	// バスク小文字_英数記のみ.
	{	TYPE_USBKeyBoardMode_BasqueBig_OnlyASCII,		TYPE_LanguageBasque,		PDPE_USBKeyKind_CapsLock	},	// バスク大文字_英数記のみ.
};

// 既存の外付けキーボード用キーMAP使用可能言語テーブル.
const TYPE_Language PDPC_KeyMapManager::tbl_USBKeyMapOKLanguage[] = {
	TYPE_LanguageAmerican,			// 米語.
	TYPE_LanguageFrench,			// フランス語.
	TYPE_LanguageItalian,			// イタリア語.
	TYPE_LanguageGerman,			// ドイツ語.
	TYPE_LanguageSpanish,			// スペイン語.
};

// 既存のソフトキーボード用キーMAP使用可能言語テーブル.
const TYPE_Language PDPC_KeyMapManager::tbl_SoftKeyMapOKLanguage[] = {
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

// インスタンス取得.
PDPC_KeyMapManager* PDPC_KeyMapManager::newInstance()
{
	static PDPC_KeyMapManager c_KeyMapManager;
	return &c_KeyMapManager;
}

// デフォルトコンストラクタ.
PDPC_KeyMapManager::PDPC_KeyMapManager()
{
	pst_KeyMap = 0;
	b_USBLoadOK = False;
	b_SoftLoadOK = False;

	for( uchar i = 0; i < PNL_MAX_LANGUAGES; i++ ) {
		e_USBLoadLanguage[i] = -1;
		e_SoftLoadLanguage[i] = -1;
	}
}

// キーモードに対応するキーMAP情報取得.
const PDPS_USBKeyModeToMapInfo* PDPC_KeyMapManager::getUSBKeyMapInfo( TYPE_USBKeyBoardMode e_Mode )
{
	for( uchar i = 0; i < ( uchar )( sizeof( tbl_USBKeyModeToMapInfo )/sizeof( PDPS_USBKeyModeToMapInfo ) ); i++ ) {
		if( e_Mode == tbl_USBKeyModeToMapInfo[i].e_Mode ) {
			return 	&( tbl_USBKeyModeToMapInfo[i] );
		}
	}
	return NULL;
}

// 外付けキーボード用キーMAPが使用できる言語へ追加.
void PDPC_KeyMapManager::addUSBLoadLanguage( TYPE_Language e_Language )
{
	for( uchar i = 0; i < ( uchar )( sizeof( e_USBLoadLanguage )/sizeof( TYPE_Language ) ); i++ ) {
		if( e_Language == e_USBLoadLanguage[i] ) {
			// 既に存在するので追加しない.
			return;
		}

		if( -1 == e_USBLoadLanguage[i] ) {
			// 空き領域まで来たので追加.
			e_USBLoadLanguage[i] = e_Language;
			return;
		}
	}
}
// ソフトキーボード用キーMAPが使用できる言語へ追加.
void PDPC_KeyMapManager::addSoftLoadLanguage( TYPE_Language e_Language )
{
	for( uchar i = 0; i < ( uchar )( sizeof( e_SoftLoadLanguage )/sizeof( TYPE_Language ) ); i++ ) {
		if( e_Language == e_SoftLoadLanguage[i] ) {
			// 既に存在するので追加しない.
			return;
		}

		if( -1 == e_SoftLoadLanguage[i] ) {
			// 空き領域まで来たので追加.
			e_SoftLoadLanguage[i] = e_Language;
			return;
		}
	}
}

// 既存の外付けキーボード用キーMAPが使用可能となる言語判定.
Bool PDPC_KeyMapManager::isUSBKeyMapOKLanguage( TYPE_Language e_Language )
{
	for( uchar i = 0; i < ( uchar )( sizeof( tbl_USBKeyMapOKLanguage )/sizeof( TYPE_Language ) ); i++ ) {
		if( e_Language == tbl_USBKeyMapOKLanguage[i] ) {
			return True;
		}
	}
	return False;
}

// 既存のソフトキーボード用キーMAPが使用可能となる言語判定.
Bool PDPC_KeyMapManager::isSoftKeyMapOKLanguage( TYPE_Language e_Language )
{
	for( uchar i = 0; i < ( uchar )( sizeof( tbl_SoftKeyMapOKLanguage )/sizeof( TYPE_Language ) ); i++ ) {
		if( e_Language == tbl_SoftKeyMapOKLanguage[i] ) {
			return True;
		}
	}
	return False;
}

// 外付けキーボード用キーMAP取得.
Bool PDPC_KeyMapManager::getUSBKeyMapBinary( PMGS_PnlMgrChangeCharCode** tbl_ChangeCharCode, int32_t* sl_Size, TYPE_USBKeyBoardMode e_USBKeyBoardMode )
{
	if( ( NULL == pst_KeyMap )
	    || ( NULL == tbl_ChangeCharCode )
	    || ( NULL == sl_Size ) ) { return False; }

	// キーモードに対応するキーMAP情報取得.
	const PDPS_USBKeyModeToMapInfo* st_ModeToMapInfo = getUSBKeyMapInfo( e_USBKeyBoardMode );
	if( NULL == st_ModeToMapInfo ) { return False; }

#ifdef QT_PANEL
	PDPS_USBKeyMap*	pc_USBKeyMap	= NULL;		// キーMap取得用ポインタ.
	Bool			bl_UK			= False;	// キーMap取得時にUKかどうかを指定するためのフラグ.

	APIC_MachineSetting* pc_APIC_MachineSetting = APIC_MachineSetting::newInstance();
	if( NULL == pc_APIC_MachineSetting ) {
		return False;
	}

	if( TYPE_LanguageAmerican == st_ModeToMapInfo->e_Language ) {
		// 言語が英語、かつEU仕向けの場合は、UK英語用キーMapを取得する.
		if( TYPE_SystemType_EU == pc_APIC_MachineSetting->getSystemType() &&
		    TYPE_SubSystemType1 == pc_APIC_MachineSetting->getSubSystemType()
		  ) {
			bl_UK = True;
		}
	}

	// QtConnectorから外付けキーボードのキーMapデータを取得する.
	pc_USBKeyMap = ( PDPS_USBKeyMap* )QCNF_UsbKeyboard_getUsbKeyMap( st_ModeToMapInfo->e_Language, False, bl_UK );
	if( NULL == pc_USBKeyMap || False == pc_USBKeyMap->valid ) {
		return False;
	}

	// QtConnectorから取得したキーMapを、outputパラメータにセットする.
	*tbl_ChangeCharCode	= pc_USBKeyMap->st_CodeData[st_ModeToMapInfo->e_Kind].st_CharCode;
	*sl_Size			= pc_USBKeyMap->st_CodeData[st_ModeToMapInfo->e_Kind].sl_Num;

	return True;
#else
	// キーMAP情報を元にキーMAPを取得する.
	for( uchar i = 0; i < ( uchar )( sizeof( pst_KeyMap->st_USBKey )/sizeof( PDPS_USBKeyMap ) ); i++ ) {
		if( False == pst_KeyMap->st_USBKey[i].valid ) { continue; }

		if( pst_KeyMap->st_USBKey[i].e_Language == st_ModeToMapInfo->e_Language ) {
			// 対応するキーMAPを引数に設定.
			*tbl_ChangeCharCode = pst_KeyMap->st_USBKey[i].st_CodeData[st_ModeToMapInfo->e_Kind].st_CharCode;
			*sl_Size = pst_KeyMap->st_USBKey[i].st_CodeData[st_ModeToMapInfo->e_Kind].sl_Num;
			return True;
		}
	}
	return False;
#endif	// #ifdef QT_PANEL.
}

// ソフトウェアキーボード用キーMAP取得.
Bool PDPC_KeyMapManager::getSoftKeyMapBinary( PLBS_GeneralKeyTbl** tbl_GeneralKeyTbl, TYPE_Language e_Language, PLBD_SoftKeyMode e_SoftKeyMode )
{
	if( ( NULL == pst_KeyMap )
	    || ( NULL == tbl_GeneralKeyTbl )
	    || ( e_SoftKeyMode >= PLBD_SoftKeyMode_Size ) ) { return False; }

	// 引数情報を元にキーMAP情報を取得.
	for( uchar i = 0; i < ( uchar )( sizeof( pst_KeyMap->st_SoftKey )/sizeof( PDPS_SoftKeyMap ) ); i++ ) {
		if( False == pst_KeyMap->st_SoftKey[i].valid ) { continue; }

		if( pst_KeyMap->st_SoftKey[i].e_Language == e_Language ) {
			// 対応するキーMAPを引数に設定.
			*tbl_GeneralKeyTbl = &( pst_KeyMap->st_SoftKey[i].st_CodeData[e_SoftKeyMode] );
			return True;
		}
	}
	return False;
}

// 外付けキーボード用キーMAP言語取得.
void PDPC_KeyMapManager::getUSBKeyBoardLoadLanguage( TYPE_Language* e_Language )
{
	if( NULL == e_Language ) { return; }

	if( True == b_USBLoadOK ) {
		// 生成済みのデータを使用する.
		memcpy( e_Language,e_USBLoadLanguage,sizeof( e_USBLoadLanguage ) );
		return;
	}
	// キーマップなしの場合.
	if( NULL == pst_KeyMap ) {
		// 英語のみを暫定的に救済する(USB起動対応).
		addUSBLoadLanguage( TYPE_LanguageAmerican );
	} else {
		// キーMAPに登録されている言語を追加.
		for( uchar i = 0; i < ( uchar )( sizeof( pst_KeyMap->st_USBKey )/sizeof( PDPS_USBKeyMap ) ); i++ ) {
			if( False == pst_KeyMap->st_USBKey[i].valid ) { continue; }

			addUSBLoadLanguage( pst_KeyMap->st_USBKey[i].e_Language );
		}

		// パネル表示言語に登録されている言語を追加.
		for( i = 0; i < ( uchar )( pst_KeyMap->ss_ViewLangNum ); i++ ) {
			if( True == isUSBKeyMapOKLanguage( pst_KeyMap->e_ViewLanguage[i] ) ) {
				addUSBLoadLanguage( pst_KeyMap->e_ViewLanguage[i] );
			}
		}
		sortKeyBoardLanguage( pst_KeyMap, e_USBLoadLanguage );		// 表示順を表示言語の順番にソートする.
	}
	// 追加結果を返す.
	memcpy( e_Language,e_USBLoadLanguage,sizeof( e_USBLoadLanguage ) );

	b_USBLoadOK = True;
}

// ソフトウェアキーボード用キーMAP言語取得.
void PDPC_KeyMapManager::getSoftKeyBoardLoadLanguage( TYPE_Language* e_Language )
{
	if( NULL == e_Language ) { return; }

	if( True == b_SoftLoadOK ) {
		// 生成済みのデータを使用する.
		memcpy( e_Language,e_SoftLoadLanguage,sizeof( e_SoftLoadLanguage ) );
		return;
	}
	// キーマップなしの場合.
	if( NULL == pst_KeyMap ) {
		// 日本語/英語のみを暫定的に救済する(USB起動対応).
		addSoftLoadLanguage( TYPE_LanguageJapanese );
		addSoftLoadLanguage( TYPE_LanguageAmerican );
	} else {
		// キーMAPに登録されている言語を追加.
		for( uchar i = 0; i < ( uchar )( sizeof( pst_KeyMap->st_SoftKey )/sizeof( PDPS_SoftKeyMap ) ); i++ ) {
			if( False == pst_KeyMap->st_SoftKey[i].valid ) { continue; }

			addSoftLoadLanguage( pst_KeyMap->st_SoftKey[i].e_Language );
		}

		// パネル表示言語に登録されている言語を追加.
		for( i = 0; i < ( uchar )( pst_KeyMap->ss_ViewLangNum ); i++ ) {
			if( True == isSoftKeyMapOKLanguage( pst_KeyMap->e_ViewLanguage[i] ) ) {
				addSoftLoadLanguage( pst_KeyMap->e_ViewLanguage[i] );
			}
		}
		sortKeyBoardLanguage( pst_KeyMap, e_SoftLoadLanguage );		// 表示順を表示言語の順番にソートする.
	}
	// 追加結果を返す.
	memcpy( e_Language,e_SoftLoadLanguage,sizeof( e_SoftLoadLanguage ) );

	b_SoftLoadOK = True;
}

// 外付けキーボード用キーMAP判定.
Bool PDPC_KeyMapManager::isUSBKeyBoardKeyMapFromMSbin( TYPE_Language e_Language )
{
	if( NULL == pst_KeyMap ) { return False; }

	for( uchar i = 0; i < ( uchar )( sizeof( pst_KeyMap->st_USBKey )/sizeof( PDPS_USBKeyMap ) ); i++ ) {
		if( False == pst_KeyMap->st_USBKey[i].valid ) { continue; }
		if( pst_KeyMap->st_USBKey[i].e_Language == e_Language ) { return True; }
	}
	return False;
}

// ソフトウェアキーボード用キーMAP判定.
Bool PDPC_KeyMapManager::isSoftKeyBoardKeyMapFromMSbin( TYPE_Language e_Language )
{
	if( NULL == pst_KeyMap ) { return False; }

	for( uchar i = 0; i < ( uchar )( sizeof( pst_KeyMap->st_SoftKey )/sizeof( PDPS_SoftKeyMap ) ); i++ ) {
		if( False == pst_KeyMap->st_SoftKey[i].valid ) { continue; }
		if( pst_KeyMap->st_SoftKey[i].e_Language == e_Language ) { return True; }
	}
	return False;
}
// キーMAPデータアドレス設定.
void PDPC_KeyMapManager::setKeyMapData( PDPS_KeyMap* s_KeyMap )
{
	pst_KeyMap = s_KeyMap;
}

// デフォルトキーMapをQtへ登録.
void PDPC_KeyMapManager::setDefaultKeyMapToQt(
    PMGS_PnlMgrChangeCharCode*	p_CharCode,			// キーMapポインタ.
    uint32_t					ul_Size,			// キーMapサイズ.
    PMGS_PnlMgrChangeCharCode*	p_CharCode_Caps,	// キーMap（Caps）ポインタ.
    uint32_t					ul_Size_Caps,		// キーMap（Caps）サイズ.
    TYPE_Language				e_Language,			// 言語.
    Bool						bl_UK				// UK英語かどうかフラグ（True：UK、False：UK以外）.
) {
	if( NULL == p_CharCode || NULL == p_CharCode_Caps || 0 == ul_Size || 0 == ul_Size_Caps ) {
		return;
	}

	PDPS_USBKeyMap keyMap;

	// 渡されたキーMapデータを、QtConnectorへ登録する形へ整形する.
	keyMap.valid		= True;
	keyMap.e_Language	= e_Language;
	keyMap.sc_Encode	= 0;

	keyMap.st_CodeData[0].sl_Num = ul_Size;
	memcpy( keyMap.st_CodeData[0].st_CharCode, p_CharCode, ul_Size * sizeof( PMGS_PnlMgrChangeCharCode ) );

	keyMap.st_CodeData[1].sl_Num = ul_Size_Caps;
	memcpy( keyMap.st_CodeData[1].st_CharCode, p_CharCode_Caps, ul_Size_Caps * sizeof( PMGS_PnlMgrChangeCharCode ) );

#ifdef QT_PANEL
	// QtConnectorへ登録（デフォルトキーMap）.
	QCNF_UsbKeyboard_setUsbKeyMap( True, ( QCNS_UsbKeyMap* )&keyMap, bl_UK );
#endif	// #ifdef QT_PANEL.
}

// メッセージバイナリのキーMapをQtへ登録.
void PDPC_KeyMapManager::setUsbKeyMapToQt( PDPS_USBKeyMap* p_KeyMap )
{
	if( NULL == p_KeyMap ) {
		return;
	}

	APIC_MachineSetting* pc_APIC_MachineSetting = APIC_MachineSetting::newInstance();
	if( NULL == pc_APIC_MachineSetting ) {
		return;
	}

	if( TYPE_LanguageAmerican == p_KeyMap->e_Language ) {
		// 言語が英語、かつEU仕向けの場合は、UK英語フラグをTrueにして登録する.
		if( TYPE_SystemType_EU == pc_APIC_MachineSetting->getSystemType() &&
		    TYPE_SubSystemType1 == pc_APIC_MachineSetting->getSubSystemType() ) {
#ifdef QT_PANEL
			// QtConnectorへ登録（メッセージバイナリのMap）.
			QCNF_UsbKeyboard_setUsbKeyMap( False, ( QCNS_UsbKeyMap* )p_KeyMap, True );
#endif	// #ifdef QT_PANEL.
			return;
		}
	}

#ifdef QT_PANEL
	// QtConnectorへ登録（メッセージバイナリのMap）.
	QCNF_UsbKeyboard_setUsbKeyMap( False, ( QCNS_UsbKeyMap* )p_KeyMap, False );
#endif	// #ifdef QT_PANEL.
}

// キーボード言語の表示順を表示言語と同様に並び替える.
static	void	sortKeyBoardLanguage( PDPS_KeyMap* s_KeyMap, TYPE_Language* e_Language )
{
	TYPE_Language	e_tmpLanguage[PNL_MAX_LANGUAGES];				// テンポラリ領域.
	TYPE_Language*	eViewLanguage;									// 表示言語格納用ポインタ.
	TYPE_Language	e_StandardViewLanguage[PNL_MAX_LANGUAGES] = {	// 標準９言語並び替え用.
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
	for( uchar i = 0; i < PNL_MAX_LANGUAGES; i++ ) {
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
	if( False == isStandardMSGBinary() ) {
		eViewLanguage = s_KeyMap->e_ViewLanguage;		// 標準９言語以外は、MSGバイナリの格納順に並び替える.
	} else {
		eViewLanguage = e_StandardViewLanguage;			// 標準９言語の場合、固定の順番に並び替える.
	}

	// 表示言語の順番通りにテンポラリ領域に格納する.
	for( i = 0; i < s_KeyMap->ss_ViewLangNum; i++ ) {
		for( uchar j = 0; j < PNL_MAX_LANGUAGES; j++ ) {
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

