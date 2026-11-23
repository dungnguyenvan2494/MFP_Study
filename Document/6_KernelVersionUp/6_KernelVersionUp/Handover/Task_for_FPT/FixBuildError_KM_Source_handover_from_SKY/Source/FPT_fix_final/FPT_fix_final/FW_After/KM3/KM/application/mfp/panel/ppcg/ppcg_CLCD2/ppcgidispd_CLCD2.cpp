//================================================================================================
//	インクルード・ファイル
//================================================================================================
#include <panel/ppcg/ppcg_CLCD2/pidispd_CLCD2.h>
#include <panel/ppcg/ppcg_CLCD/pidispd_CLCD.h>
#include <panel/pddfont/pddfontg/txt2cd.h>
#include "ppcgsinglegraph_CLCD2.h"
#include <panel/plibh/plibhg/plibh.h>

#define USE_UFST 1
#include <panel/plibl/plibl_CLCD/RasterizeSize.h>
//================================================================================================
//	定数・型の宣言
//================================================================================================

//================================================================================================
//	マクロ関数の定義
//================================================================================================

//================================================================================================
//	モジュール内変数の宣言
//================================================================================================

//================================================================================================
//	モジュール内関数のプロトタイプ宣言
//================================================================================================

/*================================================================================================
［機能］	ダブルクリック間隔時間取得
--------------------------------------------------------------------------------------------------
［引数］	void
［戻値］	ダブルクリック間隔(ms)
================================================================================================*/
short getDoubleClickInterval( void )
{
//	// 将来任意の間隔に設定可能となった場合の拡張用I/F
	APIC_OperationSetting* pc_OperationSetting = PnlComGetOperationSetting();

	short ss_IntervalMode = pc_OperationSetting->getDoubleTapIntervalMode();
	ushort us_IntervalTime = PNLD_ClickIntervalNormal;

	switch( ss_IntervalMode ) {
		case TYPE_DoubleTapIntervalModeSlow:
			us_IntervalTime = (ushort)( us_IntervalTime + ( PNLD_ClickIntervalSlow * 2 ) );
			break;
		case TYPE_DoubleTapIntervalModeLittleSlow:
			us_IntervalTime = (ushort)( us_IntervalTime + ( PNLD_ClickIntervalSlow ) );
			break;
		case TYPE_DoubleTapIntervalModeNormal:
			break;
		case TYPE_DoubleTapIntervalModeLittleFast:
			us_IntervalTime =(ushort)( us_IntervalTime - ( PNLD_ClickIntervalFast ) );
			break;
		case TYPE_DoubleTapIntervalModeFast:
			us_IntervalTime = (ushort)( us_IntervalTime - ( PNLD_ClickIntervalFast * 2 ) );
			break;
		default:
			break;
	}

	return us_IntervalTime;
}

/*================================================================================================
［機能］	実文字列のサイズを取得.
--------------------------------------------------------------------------------------------------
［引数］	ushort 				us_TextID			: テキストID.
			short& 				ss_XSize			: サイズX.
			short& 				ss_YSize			: サイズY.
			PNLE_SELECT_FONT 	e_SelectFont		: フォント指定.
			short 				ss_Option			: オプション指定.
［戻値］	無し.
［備考］	文字溢れ時の改行はなく、文字列そのままのサイズ.
================================================================================================*/
void partsGetSizePropText( ushort us_TextID, short& ss_XSize, short& ss_YSize, PNLE_SELECT_FONT e_SelectFont, short ss_Option )
{
	// ローカル変数定義.
	Bool					b_FontData = False;										// 描画する文字がある.
	uchar*					puc_ptr = ( uchar* )getTextPartsPointer( us_TextID );		// 文字列の取得.
	short 					ss_FontPosX = 0;										// 描画位置(X).
	short 					ss_FontPosY = 0;										// 描画位置(Y).
	short 					ss_xSize = 1;											// 今の文字のXサイズ.
	short 					ss_ySize = 16;											// 今の文字のYサイズ.
	EP_FontSize 			ep_FontSize = EP_FontSize_Half;							// 今の文字種類のサイズ(全角、半角、1/4角).
	uchar 					uc_FontType = FontType_ASCII;							// 文字コード種別.
	uchar* 					puc_ptrBak = NULL;										// (プロポーショナルの場合)文字コードバックアップ用.
	PLBC_RasterizeFontSize	c_RasterizeFontSize;

	// ローカル変数定義(プロポーショナル対応).
	short 					ss_xSizeP = 0;											// 次の文字までのオフセット.
	short 					ss_StrWidth = 0; 										// 文字列幅(X).
	short 					ss_StrHeight = 0; 										// 文字列幅(Y).
	uchar 					uc_FontSize = 16; 										// フォントサイズ.
	EP_CharSet 				uc_CharSet = EP_CharSet_ISO8859_1; 						// フォントセット.
	TYPE_OutlineFontType 	e_OutlineFont = TYPE_OutlineFont_Main; 					// フォントタイプ.
	Bool 					b_RasterizeFlag = False; 								// ラスタライズ実行フラグ.
	Bool					b_KerningCheck = False;									// カーニングチェック実施フラグ.
	short					ss_StringCount = 0;										// 同一コードの文字列数.
	uchar					uc_LinePitch = 0;										// 行送り(px).

	// 1/4角文字列かどうか判定.
	Bool bl_Quarter = isQuaterText( ( const uchar* )puc_ptr );
	// 1行の高さを取得.
	ss_StrHeight = getFontSizeY( e_SelectFont, bl_Quarter );
	// 行送りを取得.
	uc_LinePitch = (uchar)getLinePitch( e_SelectFont, ss_Option, bl_Quarter );

	// 文字列の終了コードでない間、ループ
	while( *puc_ptr != EndOfString ) {
		// 文字コードバックアップ.
		if( True == g_CharJudge( puc_ptr ) ) {
			puc_ptrBak = puc_ptr;
		} else {
			puc_ptrBak = NULL;
		}
		register uchar data = *puc_ptr++;				// 現在の文字をピックアップ.

		// 1.フォント制御コードの時.
		if( data >= FontControlStart && data < FontCursorStart ) {
			uc_FontType = data;							// 文字コードを保存する.
			short	ss_adjustment = 0;
			convSelectFont( e_SelectFont, uc_FontType, uc_FontSize, uc_CharSet, e_OutlineFont, ss_adjustment );
			ss_StringCount = 0;							// 同一コードの文字列数.
			if( FontType_SJISKatakana == uc_FontType || FontType_Kanji == uc_FontType ) {
				b_KerningCheck = True;					// 日本語の場合カーニングチェックを行う.
			} else {
				b_KerningCheck = False;					// 日本語以外はカーニングチェックを行わない.
			}
		} else if( data >= FontCursorStart && data < FontBunkStart ) {
			// 2.カーソル移動の時.
			if( CursorCRLF == data || CursorSPCRLF == data ) {
				uchar uc_Pitch = (uchar)( ( CursorCRLF == data ) ? uc_LinePitch : getFontSizeY( e_SelectFont, bl_Quarter ) );	// 特殊改行文字の場合は1行のサイズ分しか移動しない.
				ss_FontPosX     = 0;					// 描画位置(X)を左端へ移動.
				ss_FontPosY    = (short)( ss_FontPosY + uc_Pitch );				// 描画位置(Y)を1行下へ.
				ss_StrHeight   = (short)( ss_StrHeight + uc_Pitch );			// 全文字列の高さに、行送りを加算.
				ss_StringCount  = 0;
			}
		} else {
			// 3.文字コードの時.
			uchar uc_ExactFontByte = 0;
			b_FontData = False;
			register ushort i;
			for( i = 0; i < NumOfFontAttribute; i++ ) {
				// 3-1. 文字コードの検索.
				if( sp_FontAttributePluralSize[i].uc_FontType == uc_FontType ) {
					ep_FontSize = sp_FontAttributePluralSize[i].ep_FontSize;						// 文字コードのサイズ(全角、半角、1/4角)を保存.
					// 3-1-1. SJIS準拠の場合.
					if( sp_FontAttributePluralSize[i].ep_FontCodeType == EP_FontCodeType_SJIS ) {
						// 1byteコードの時.
						if( sp_FontAttributePluralSize[i].uc_NumofByte == 1 ) {
							b_FontData = True;
						} else if( sp_FontAttributePluralSize[i].uc_NumofByte == 2 ) {
							// 2byteコードの時.
							// 漢字で1バイト目が0xE0以上のときは、0x9Fの次に0xE0のフォントを置くために0x40を引いておく。
							if( sp_FontAttributePluralSize[i].uc_FontType == FontType_Kanji && data >= 0xE0 ) {
								data = (uchar)( data - 0x40 );
							}
							b_FontData = True;
							data = *puc_ptr++;	// 2byte目のデータを取得.
						}
					} else if( sp_FontAttributePluralSize[i].ep_FontCodeType == EP_FontCodeType_Bunk ) {
						// 3-1-2. バンク指定の場合
						if( data >= FontBunkStart && data < FontDataStart ) {	// フォントバンクの時
							b_FontData = True;
							data = *puc_ptr++;	// 2byte目のデータを取得.
						}
					} else if( sp_FontAttributePluralSize[i].ep_FontCodeType == EP_FontCodeType_2Byte ) {
						// 3-1-3. 2Byte指定の場合.
						if( data < sp_FontAttributePluralSize[i].uc_Offset1 ) {
							b_FontData = False;
						} else {
							b_FontData = True;
						}
						data = *puc_ptr++;				// 2byte目のフォントデータを取得
						if( sp_FontAttributePluralSize[i].uc_FontType == FontType_Chinese1 && data >= 0xA0 ) {
							data = (uchar)( data - 0x20 );				// 台湾は XX7F の次は XXA0 なので、0x20 引く.
						}
					} else if( EP_FontCodeType_MltByte == sp_FontAttributePluralSize[i].ep_FontCodeType ) {
						// マルチバイト言語.
						if( data < sp_FontAttributePluralSize[i].uc_Offset1 ) {
							b_FontData = False;
						} else {
							b_FontData = True;
						}

						if( FontType_Chinese2 == sp_FontAttributePluralSize[i].uc_FontType ) {
							// puc_ptrは2byte目を指しているため、puc_ptr[-1]が1byte目となる.
							uc_ExactFontByte	= checkGB18030( &puc_ptr[-1] );

							// フォントデータにはアクセスしないので、data値の正確な計算は行わない.
							data = *puc_ptr++;			// 2byte目のフォントデータを取得.

							switch( uc_ExactFontByte ) {
								// GB18030 の4byteコードは補正必要なし.
								// 次のフォントデータを指すため、puc_ptrを2byte先に進める.
								case PNLD_FontByte_4:	puc_ptr += 2; break;

								case PNLD_FontByte_2: break;

								default: b_FontData = False; uc_ExactFontByte = 0; break;
							}
						} else {
							b_FontData = False;
						}
					}
					ss_StringCount++;
					break;
				}
			}
			// 3-2. 1文字の幅取得.
			if( True == b_FontData ) {
				// 1/4角文字と他の文字は混在できない.
				Bool bl_QuarterChk = ( EP_FontSize_Quater == ep_FontSize ) ? True : False;
				if( bl_Quarter != bl_QuarterChk ) {
					return;
				}
				if( ss_Option & PNLD_TEXTOPTION_MONOSPACE ) {						// 固定幅文字列の場合(ラスタライズはしない).
					if( NULL != puc_ptrBak ) {										// スケーラブルフォントの場合.
						ss_xSizeP = getFixedFontXSize( data, e_SelectFont, uc_FontType, ss_Option );
					} else {
						// 3-2-2 ラスタライズ失敗時(bmpフォントで1文字描画).
						getBmpFontSize( e_SelectFont, uc_FontType, ep_FontSize, ss_xSize, ss_ySize );
						ss_xSizeP = ss_xSize;								// 次の文字までのオフセット.
					}
				} else {
					// ラスタライズ実行に必要な変数.
					static PLBS_AnsRasterize s_AnsRasterize;	// 変換情報取得用.
					static PLBS_ReqRasterize s_ReqRasterize;	// 変換情報要求用.
					memset( &s_AnsRasterize, 0, sizeof( s_AnsRasterize ) );
					memset( &s_ReqRasterize, 0, sizeof( s_ReqRasterize ) );
					s_AnsRasterize.b_ret = False;				// 変換結果を失敗にしておく.
					s_ReqRasterize.puc_font = puc_ptrBak;
					s_ReqRasterize.uc_fontByte = sp_FontAttributePluralSize[i].uc_NumofByte;
					if( 0 != uc_ExactFontByte ) {
						s_ReqRasterize.uc_fontByte = uc_ExactFontByte;
					}
					s_ReqRasterize.e_CharSet = uc_CharSet;
					s_ReqRasterize.e_FontType = e_OutlineFont;
					s_ReqRasterize.e_Format = TYPE_RasterizeFormat_Graymap;
					s_ReqRasterize.ss_xSize = uc_FontSize;
					// ラスタライズ実行.
					if( NULL != puc_ptrBak ) {
						s_AnsRasterize = c_RasterizeFontSize.ReqRasterizeFont( s_ReqRasterize );
						b_RasterizeFlag = True;													// 実行フラグを立てる.
					}
					// 3-2-1.ラスタライズ成功時(プロポーショナルフォントで1文字描画).
					if( True == s_AnsRasterize.b_ret ) {
						ss_xSizeP = s_AnsRasterize.us_xSize;											// 描画する文字のサイズ(X).
						if( True == b_KerningCheck && 1<ss_StringCount ) { ss_FontPosX = (short)( ss_FontPosX + PnlComGetKerningSize() ); }
					} else {
						// 3-2-2 ラスタライズ失敗時(bmpフォントで1文字描画).
						getBmpFontSize( e_SelectFont, uc_FontType, ep_FontSize, ss_xSize, ss_ySize );
						ss_xSizeP = ss_xSize;								// 次の文字までのオフセット.
					}
					// ラスタライズ実行後の終了処理.
					if( True == b_RasterizeFlag ) {
						c_RasterizeFontSize.freeChar();
					}

				}
				ss_FontPosX = (short)( ss_FontPosX + ss_xSizeP );																// 次の文字の位置(X)まで移動.
				ss_StrWidth = ( ss_StrWidth < ss_FontPosX ) ? ss_FontPosX : ss_StrWidth; 				// 文字列のサイズをセット.
			}
		}
	}
	// ラスタライズ実行後の終了処理.
	if( True == b_RasterizeFlag ) {
		c_RasterizeFontSize.RasterizeExit();
		b_RasterizeFlag = False;
	}

#if defined(SIMULATOR)
	// 文字描画位置のアウトプット(デバッグ用).
	if( DEF_PANEL_FONTPOS_SHOW == PnlComGetFontDrawInfo() ) { TRACE( "\n  <OriginalTxtSize>\n  W:%3d, H:%3d\n  ----------------------------\n", ss_StrWidth, ss_StrHeight ); }
#endif

	// 戻り値をセット.
	ss_XSize = ss_StrWidth;
	ss_YSize = ss_StrHeight;

}
/*================================================================================================
［機能］	実文字列の位置・サイズを取得(プロポーショナルフォント).
--------------------------------------------------------------------------------------------------
［引数］		ushort 				us_TextID			: テキストID.
				short				ss_XStart			: パーツの位置X.
				short				ss_YStart			: パーツの位置Y.
				short				ss_Width			: パーツのサイズX.
				short				ss_Height			: パーツのサイズY.
				TYPE_Layout			e_LayoutX			: X方向レイアウト.
				TYPE_VLayout		e_LayoutY			: Y方向レイアウト.
				PNLE_SELECT_FONT 	e_SelectFont		: フォント指定.
				short 				ss_Option			: オプション指定.
［戻値］		表示文字列情報.
================================================================================================*/
PNLS_TextInfo partsGetPosInfo( ushort us_TextID, short ss_XStart, short ss_YStart, short ss_Width, short ss_Height, TYPE_Layout e_LayoutX, TYPE_VLayout e_LayoutY, PNLE_SELECT_FONT e_SelectFont, short ss_Option )
{
	// ローカル変数定義.
	Bool					b_FontData = False;										// 描画する文字がある.
	uchar*					puc_ptr = ( uchar* )getTextPartsPointer( us_TextID );		// 文字列の取得.
	short 					ss_FontPosX = 0;										// 描画位置(X).
	short 					ss_FontPosY = 0;										// 描画位置(Y).
	short 					ss_xSize = 1;											// 今の文字のXサイズ.
	short 					ss_ySize = 16;											// 今の文字のYサイズ.
	EP_FontSize 			ep_FontSize = EP_FontSize_Half;							// 今の文字種類のサイズ(全角、半角、1/4角).
	uchar 					uc_FontType = FontType_ASCII;							// 文字コード種別.
	uchar* 					puc_ptrBak = NULL;										// (プロポーショナルの場合)文字コードバックアップ用.
	PLBC_RasterizeFontSize	c_RasterizeFontSize;

	// ローカル変数定義(プロポーショナル対応).
	short 					ss_xSizeP = 0;											// 次の文字までのオフセット.
	short 					ss_StrWidth = 0; 										// 文字列幅(X).
	short 					ss_StrHeight = 0; 										// 文字列幅(Y).
	uchar 					uc_FontSize = 16; 										// フォントサイズ.
	EP_CharSet 				uc_CharSet = EP_CharSet_ISO8859_1; 						// フォントセット.
	TYPE_OutlineFontType 	e_OutlineFont = TYPE_OutlineFont_Main; 					// フォントタイプ.
	Bool 					b_RasterizeFlag = False; 								// ラスタライズ実行フラグ.
	Bool					b_KerningCheck = False;									// カーニングチェック実施フラグ.
	short					ss_StringCount = 0;										// 同一コードの文字列数.
	uchar					uc_LinePitch = 0;										// 行送り(px).
	PNLE_DISPADJUST			uc_DispAdjust = PNLE_DISPADJUST_ALL;					// 文字列の状態(全ての文字表示).
	short					ss_Line;												// 行数.
	short					ss_StringTotalCount = 0 ;								// 文字数.

	// 1/4角文字列かどうか判定.
	Bool bl_Quarter = isQuaterText( ( const uchar* )puc_ptr );
	// 1行の高さを取得.
	ss_StrHeight = getFontSizeY( e_SelectFont, bl_Quarter );
	// 行送りを取得.
	uc_LinePitch = (uchar)getLinePitch( e_SelectFont, ss_Option, bl_Quarter );
	ss_Line = 1;
	PNLS_TextInfo s_TextInfo;	// 表示文字列構造体.
	// 行毎の先頭文字数情報初期化処理.
	for( uchar cnt = 0; cnt < 80; cnt++ ) { s_TextInfo.ss_HeadOfLineString[cnt] = 0; }


	// 文字列の終了コードでない間、ループ
	while( *puc_ptr != EndOfString ) {
		// 文字コードバックアップ.
		if( True == g_CharJudge( puc_ptr ) ) {
			puc_ptrBak = puc_ptr;
		} else {
			puc_ptrBak = NULL;
		}
		register uchar data = *puc_ptr++;				// 現在の文字をピックアップ.

		// 1.フォント制御コードの時.
		if( data >= FontControlStart && data < FontCursorStart ) {
			uc_FontType = data;							// 文字コードを保存する.
			short	ss_adjustment = 0;
			convSelectFont( e_SelectFont, uc_FontType, uc_FontSize, uc_CharSet, e_OutlineFont, ss_adjustment );
			ss_StringCount = 0;							// 同一コードの文字列数.
			if( FontType_SJISKatakana == uc_FontType || FontType_Kanji == uc_FontType ) {
				b_KerningCheck = True;					// 日本語の場合カーニングチェックを行う.
			} else {
				b_KerningCheck = False;					// 日本語以外はカーニングチェックを行わない.
			}
		} else if( data >= FontCursorStart && data < FontBunkStart ) {
			// 2.カーソル移動の時.
			if( CursorCRLF == data || CursorSPCRLF == data ) {
				uchar uc_Pitch = (uchar)( ( CursorCRLF == data ) ? uc_LinePitch : getFontSizeY( e_SelectFont, bl_Quarter ) );	// 特殊改行文字の場合は1行のサイズ分しか移動しない.
				if( ss_Height < ( ss_StrHeight + uc_Pitch ) ) {			// 改行すると行溢れになる場合.
					uc_DispAdjust = PNLE_DISPADJUST_PART;
					break;
				} else {
					ss_FontPosX     = 0;					// 描画位置(X)を左端へ移動.
					ss_FontPosY     = (short)( ss_FontPosY + uc_Pitch );				// 描画位置(Y)を1行下へ.
					ss_StrHeight    = (short)( ss_StrHeight + uc_Pitch );				// 全文字列の高さに、行送りを加算.
					ss_StringTotalCount++;
					ss_Line++;
					ss_StringCount  = 0;
					if( 2 <= ss_Line && 41 >= ss_Line && 0 == ss_FontPosX ) {
						s_TextInfo.ss_HeadOfLineString[ ss_Line - 2 ] = (short)( ss_StringTotalCount + 1 );
					}
				}
			}
		} else {
			// 3.文字コードの時.
			uchar uc_ExactFontByte = 0;
			b_FontData = False;
			register ushort i;
			for( i = 0; i < NumOfFontAttribute; i++ ) {
				// 3-1. 文字コードの検索.
				if( sp_FontAttributePluralSize[i].uc_FontType == uc_FontType ) {
					ep_FontSize = sp_FontAttributePluralSize[i].ep_FontSize;						// 文字コードのサイズ(全角、半角、1/4角)を保存.
					// 3-1-1. SJIS準拠の場合.
					if( sp_FontAttributePluralSize[i].ep_FontCodeType == EP_FontCodeType_SJIS ) {
						// 1byteコードの時.
						if( sp_FontAttributePluralSize[i].uc_NumofByte == 1 ) {
							b_FontData = True;
						} else if( sp_FontAttributePluralSize[i].uc_NumofByte == 2 ) {
							// 2byteコードの時.
							// 漢字で1バイト目が0xE0以上のときは、0x9Fの次に0xE0のフォントを置くために0x40を引いておく。
							if( sp_FontAttributePluralSize[i].uc_FontType == FontType_Kanji && data >= 0xE0 ) {
								data = (uchar)( data - 0x40 );
							}
							b_FontData = True;
							data = *puc_ptr++;	// 2byte目のデータを取得.
						}
					} else if( sp_FontAttributePluralSize[i].ep_FontCodeType == EP_FontCodeType_Bunk ) {
						// 3-1-2. バンク指定の場合
						if( data >= FontBunkStart && data < FontDataStart ) {	// フォントバンクの時
							b_FontData = True;
							data = *puc_ptr++;	// 2byte目のデータを取得.
						}
					} else if( sp_FontAttributePluralSize[i].ep_FontCodeType == EP_FontCodeType_2Byte ) {
						// 3-1-3. 2Byte指定の場合.
						if( data < sp_FontAttributePluralSize[i].uc_Offset1 ) {
							b_FontData = False;
						} else {
							b_FontData = True;
						}
						data = *puc_ptr++;				// 2byte目のフォントデータを取得
						if( sp_FontAttributePluralSize[i].uc_FontType == FontType_Chinese1 && data >= 0xA0 ) {
							data = (uchar)( data - 0x20 );				// 台湾は XX7F の次は XXA0 なので、0x20 引く.
						}
					} else if( EP_FontCodeType_MltByte == sp_FontAttributePluralSize[i].ep_FontCodeType ) {
						// マルチバイト言語.
						if( data < sp_FontAttributePluralSize[i].uc_Offset1 ) {
							b_FontData = False;
						} else {
							b_FontData = True;
						}

						if( sp_FontAttributePluralSize[i].uc_FontType == FontType_Chinese2 ) {
							// puc_ptrは2byte目を指しているため、puc_ptr[-1]が1byte目となる.
							uc_ExactFontByte	= checkGB18030( &puc_ptr[-1] );

							// フォントデータにはアクセスしないので、data値の正確な計算は行わない.
							data = *puc_ptr++;			// 2byte目のフォントデータを取得.

							switch( uc_ExactFontByte ) {
								// GB18030 の4byteコードは補正必要なし.
								// 次のフォントデータを指すため、puc_ptrを2byte先に進める.
								case PNLD_FontByte_4:	puc_ptr += 2; break;

								case PNLD_FontByte_2: break;

								default: b_FontData = False; uc_ExactFontByte = 0; break;
							}
						} else {
							b_FontData = False;
						}
					}
					ss_StringCount++;
					ss_StringTotalCount++;
					break;
				}
			}
			// 3-2. 1文字の幅取得.
			if( True == b_FontData ) {
				// 1/4角文字と他の文字は混在できない.
				Bool bl_QuarterChk = ( EP_FontSize_Quater == ep_FontSize ) ? True : False;
				if( bl_Quarter != bl_QuarterChk ) {
					memset( &s_TextInfo, 0, sizeof( s_TextInfo ) );
					return s_TextInfo;
				}
				if( ss_Option & PNLD_TEXTOPTION_MONOSPACE ) {						// 固定幅文字列の場合(ラスタライズはしない).
					if( NULL != puc_ptrBak ) {										// スケーラブルフォントの場合.
						ss_xSizeP = getFixedFontXSize( data, e_SelectFont, uc_FontType, ss_Option );
						// X方向に文字溢れになる場合は改行する.
						if( ss_Width < ( ss_FontPosX + ss_xSizeP ) ) {
							// 改行してもX・Y方向に文字溢れになる場合は描画しない
							if( ( ss_Width < ss_xSizeP ) || ( ss_Height < ( ss_StrHeight + uc_LinePitch ) ) ) {
								uc_DispAdjust = PNLE_DISPADJUST_PART;			// 一部の文字のみ表示.
								break;
							} else {
								uc_DispAdjust = PNLE_DISPADJUST_RETURN;			// 折り返して全ての文字を表示.
								ss_FontPosX     = 0;
								ss_FontPosY     = (short)( ss_FontPosY + uc_LinePitch );
								ss_StrHeight    = (short)( ss_StrHeight + uc_LinePitch );
								ss_Line++;
								ss_StringCount  = 0;
							}
						}
					} else {
						// 3-2-2 ラスタライズ失敗時(bmpフォントで1文字描画).
						getBmpFontSize( e_SelectFont, uc_FontType, ep_FontSize, ss_xSize, ss_ySize );
						ss_xSizeP = ss_xSize;								// 次の文字までのオフセット.

						// X方向に文字溢れになる場合は改行する.
						if( ss_Width < ( ss_FontPosX + ss_xSizeP ) ) {
							// 改行してもX・Y方向に文字溢れになる場合は描画しない
							if( ( ss_Width < ss_xSizeP ) || ( ss_Height < ( ss_StrHeight + uc_LinePitch ) ) ) {
								uc_DispAdjust = PNLE_DISPADJUST_PART;			// 一部の文字のみ表示.
								break;
							} else {
								uc_DispAdjust = PNLE_DISPADJUST_RETURN;			// 折り返して全ての文字を表示.
								ss_FontPosX     = 0;
								ss_FontPosY     = (short)( ss_FontPosY + uc_LinePitch );
								ss_StrHeight    = (short)( ss_StrHeight + uc_LinePitch );
								ss_Line++;
								ss_StringCount  = 0;
							}
						}
					}
				} else {																			// プロポーショナル文字幅の場合.
					// ラスタライズ実行に必要な変数.
					static PLBS_AnsRasterize s_AnsRasterize;	// 変換情報取得用.
					static PLBS_ReqRasterize s_ReqRasterize;	// 変換情報要求用.
					memset( &s_AnsRasterize, 0, sizeof( s_AnsRasterize ) );
					memset( &s_ReqRasterize, 0, sizeof( s_ReqRasterize ) );
					s_AnsRasterize.b_ret = False;				// 変換結果を失敗にしておく.
					s_ReqRasterize.puc_font = puc_ptrBak;
					s_ReqRasterize.uc_fontByte = sp_FontAttributePluralSize[i].uc_NumofByte;
					if( 0 != uc_ExactFontByte ) {
						s_ReqRasterize.uc_fontByte = uc_ExactFontByte;
					}
					s_ReqRasterize.e_CharSet = uc_CharSet;
					s_ReqRasterize.e_FontType = e_OutlineFont;
					s_ReqRasterize.e_Format = TYPE_RasterizeFormat_Graymap;
					s_ReqRasterize.ss_xSize = uc_FontSize;
//						// EU特殊文字で枠に入らない大文字はフォントを3/4にする.
					// ラスタライズ実行.
					if( NULL != puc_ptrBak ) {
						s_AnsRasterize = c_RasterizeFontSize.ReqRasterizeFont( s_ReqRasterize );
						b_RasterizeFlag = True;													// 実行フラグを立てる.
					}
					// 3-2-1.ラスタライズ成功時(プロポーショナルフォントで1文字描画).
					if( True == s_AnsRasterize.b_ret ) {
						ss_xSizeP = s_AnsRasterize.us_xSize;
						if( True == b_KerningCheck && 1<ss_StringCount ) { ss_FontPosX = (short)( ss_FontPosX + PnlComGetKerningSize() ); }
						// X方向に文字溢れになる場合は改行する.
						if( ss_Width < ( ss_FontPosX + ss_xSizeP ) ) {
							// 改行してもX・Y方向に文字溢れになる場合は描画しない
							if( ( ss_Width < ss_xSizeP ) || ( ss_Height < ( ss_StrHeight + uc_LinePitch ) ) ) {
								uc_DispAdjust = PNLE_DISPADJUST_PART;			// 一部の文字のみ表示.
								break;
							} else {
								uc_DispAdjust = PNLE_DISPADJUST_RETURN;			// 折り返して全ての文字を表示.
								ss_FontPosX     = 0;
								ss_FontPosY    = (short)( ss_FontPosY + uc_LinePitch );
								ss_StrHeight   = (short)( ss_StrHeight + uc_LinePitch );
								ss_Line++;
								ss_StringCount  = 0;
							}
						}
					} else {
						// 3-2-2 ラスタライズ失敗時(bmpフォントで1文字描画).
						getBmpFontSize( e_SelectFont, uc_FontType, ep_FontSize, ss_xSize, ss_ySize );
						ss_xSizeP = ss_xSize;								// 次の文字までのオフセット.
						// X方向に文字溢れになる場合は改行する.
						if( ss_Width < ( ss_FontPosX + ss_xSizeP ) ) {
							// 改行してもX・Y方向に文字溢れになる場合は描画しない
							if( ( ss_Width < ss_xSizeP ) || ( ss_Height < ( ss_StrHeight + uc_LinePitch ) ) ) {
								uc_DispAdjust = PNLE_DISPADJUST_PART;		// 一部の文字のみ表示.
								break;
							} else {
								uc_DispAdjust = PNLE_DISPADJUST_RETURN;		// 折り返して全ての文字を表示.
								ss_FontPosX     = 0;
								ss_FontPosY    = (short)( ss_FontPosY + uc_LinePitch );
								ss_StrHeight   = (short)( ss_StrHeight + uc_LinePitch );
								ss_Line++;
								ss_StringCount  = 0;
							}
						}
					}
					// ラスタライズ実行後の終了処理.
					if( True == b_RasterizeFlag ) {
						c_RasterizeFontSize.freeChar();
					}
				}

				// 行頭の1文字目が文字列の何番目かを保存(2行目以降のみ)
				if( 2 <= ss_Line && 41 >= ss_Line && 0 == ss_FontPosX ) {
					s_TextInfo.ss_HeadOfLineString[ ss_Line - 2 ] = ss_StringTotalCount;
				}
				ss_FontPosX = (short)( ss_FontPosX + ss_xSizeP );										// 次の文字の位置(X)まで移動.
				ss_StrWidth = ( ss_StrWidth < ss_FontPosX ) ? ss_FontPosX : ss_StrWidth; 				// 文字列のサイズをセット.
			}
		}
	}
	// ラスタライズ実行後の終了処理.
	if( True == b_RasterizeFlag ) {
		c_RasterizeFontSize.RasterizeExit();
		b_RasterizeFlag = False;
	}
	short ss_AdjustPosX = 0, ss_AdjustPosY = 0;

	// X方向のレイアウト.
	switch( e_LayoutX ) {
		case TYPE_LayoutCentering:		ss_AdjustPosX = (short)( ss_XStart + ( short )( ( ss_Width - ss_StrWidth ) / 2 ) );	break;		// センタリング.
		case TYPE_LayoutRightAdjust:	ss_AdjustPosX = (short)( ss_XStart + ( ss_Width - ss_StrWidth ) );					break;		// 右寄せ.
		case TYPE_LayoutLeftAdjust:
		case TYPE_LayoutFree:			ss_AdjustPosX = ss_XStart;												break;		// 左寄せ, フリー.
		default:				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
	}
	// Y方向のレイアウト.
	switch( e_LayoutY ) {
		case TYPE_VLayoutMiddle:		ss_AdjustPosY = (short)( ss_YStart + ( short )( ( ss_Height - ss_StrHeight ) / 2 ) );	break;		// センタリング.
		case TYPE_VLayoutBottom:		ss_AdjustPosY = (short)( ss_YStart + ( ss_Height - ss_StrHeight ) );					break;		// 下寄せ.
		case TYPE_VLayoutTop:
		case TYPE_VLayoutFree:			ss_AdjustPosY = ss_YStart;												break;		// 上寄せ, フリー.
		default:				LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );	break;
	}


	// 戻り値をセット.
	s_TextInfo.ss_PosX   = ss_AdjustPosX;
	s_TextInfo.ss_PosY   = ss_AdjustPosY;
	s_TextInfo.ss_Width  = ss_StrWidth;
	s_TextInfo.ss_Height = ss_StrHeight;
	s_TextInfo.uc_Adjust = uc_DispAdjust;
	s_TextInfo.ss_Line = ss_Line;

	if( DEF_PANEL_FONTPOS_SHOW == PnlComGetFontDrawInfo() ) {
		TRACE( "  <TextInfo>\n  X:%3d, Y:%3d, W:%3d, H:%3d, F:%d\n  ----------------------------\n\n", s_TextInfo.ss_PosX, s_TextInfo.ss_PosY, s_TextInfo.ss_Width, s_TextInfo.ss_Height, s_TextInfo.uc_Adjust );
	}
	return s_TextInfo;

}

// comment 2015/12/14 OPS1 Sky東島 [ZeusSZX0][CQ521810].
#define PNLD_PNG_OFFSET_EXTEND	9	// GFWD_PNG_OFFSET_EXTENDと同じ値にすること.
/*================================================================================================
［機能］	KM独自形式のグラフィックデータにバイリニア表示フラグを付与する.
--------------------------------------------------------------------------------------------------
［引数］		char* 				sc_Data			: 画像データ.
［戻値］		なし.
================================================================================================*/
void setBilinearFlag( char *sc_Data )
{
	if( NULL != sc_Data ){
		// アイコンフォーマットチェック.
		TYPE_ExtIconFormat e_IconFormat = PnlComGetStandardPanelSetting()->checkIconFormat( ( uchar* )sc_Data );
		if( TYPE_ExtIconFormat_OriginalPng == e_IconFormat ){
			// 画像データの9バイト目上位3bitでバイリニア表示判定を行っている.
			// 7bit目、5bit目を1 6bit目を0に設定することでバイリニア表示となる.
			sc_Data[PNLD_PNG_OFFSET_EXTEND] &= 0x1F;
			sc_Data[PNLD_PNG_OFFSET_EXTEND] = (char)( sc_Data[PNLD_PNG_OFFSET_EXTEND] | ( 0x01 << 7 ) );
			sc_Data[PNLD_PNG_OFFSET_EXTEND] |= 0x01 << 5;
		}
	}
}
