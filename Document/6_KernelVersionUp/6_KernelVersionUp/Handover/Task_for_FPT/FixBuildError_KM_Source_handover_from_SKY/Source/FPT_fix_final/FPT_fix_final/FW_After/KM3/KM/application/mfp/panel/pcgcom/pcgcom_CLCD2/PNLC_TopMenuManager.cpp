// ===============================================================================================
/**
 *	@file	PNLC_TopMenuManager.cpp
 *	@brief	パネルトップメニュー管理クラス.
 */
// ===============================================================================================

/*================================================================================================
	インクルード・ファイル.
================================================================================================*/
#include "PNLC_TopMenuManager.h"
#include <panel/plibh/plibh_CLCD2/plibh_CLCD2.h>		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
#include <panel/plibh/plibh_FUM2/plibh_FUM2.h>
#include <panel/plibh/plibh_CLCD2/PNLC_FunctionBadgeLib.h>

/*================================================================================================
	グローバル変数の宣言.
================================================================================================*/
const PNLS_CorrespondCopyGrpID	sc_CorrespondCopyGrpIDTbl[] = {
	{TYPE_PNLGRP1_COPY_Color,				TYPE_PNLMENU_COPY_Color},
	{TYPE_PNLGRP1_COPY_Density,				TYPE_PNLMENU_COPY_Density},
	{TYPE_PNLGRP1_COPY_OrgType,				TYPE_PNLMENU_COPY_OrgType},
	{TYPE_PNLGRP1_COPY_Paper,				TYPE_PNLMENU_COPY_Paper},
	{TYPE_PNLGRP1_COPY_Zoom,				TYPE_PNLMENU_COPY_Zoom},
	{TYPE_PNLGRP1_COPY_DuplexCombine,		TYPE_PNLMENU_COPY_DuplexCombine},
	{TYPE_PNLGRP1_COPY_Finishing,			TYPE_PNLMENU_COPY_Finishing},
	{TYPE_PNLGRP1_COPY_OrgSet,				TYPE_PNLMENU_COPY_OrgSet},
	{TYPE_PNLGRP1_COPY_OrgBook,				TYPE_PNLMENU_COPY_OrgBook},
	{TYPE_PNLGRP1_COPY_SeparateScan,		TYPE_PNLMENU_COPY_SeparateScan},
	{TYPE_PNLGRP1_COPY_AutoRotateOff,		TYPE_PNLMENU_COPY_AutoRotateOff},
	{TYPE_PNLGRP1_COPY_BackgroundRemoval,	TYPE_PNLMENU_COPY_BackgroundRemoval},
	{TYPE_PNLGRP1_COPY_TextEnhancement,		TYPE_PNLMENU_COPY_TextEnhancement},
	{TYPE_PNLGRP1_COPY_Glossy,				TYPE_PNLMENU_COPY_Glossy},
	{TYPE_PNLGRP1_COPY_NegPosReverse,		TYPE_PNLMENU_COPY_NegPosReverse},
	{TYPE_PNLGRP1_COPY_BackgroundColor,		TYPE_PNLMENU_COPY_BackgroundColor},
	{TYPE_PNLGRP1_COPY_ColorAdjust,			TYPE_PNLMENU_COPY_ColorAdjust},
	{TYPE_PNLGRP1_COPY_Booklet,				TYPE_PNLMENU_COPY_Booklet},
	{TYPE_PNLGRP1_COPY_PageMargin,			TYPE_PNLMENU_COPY_PageMargin},
	{TYPE_PNLGRP1_COPY_ImageShift,			TYPE_PNLMENU_COPY_ImageShift},
	{TYPE_PNLGRP1_COPY_MirrorImage,			TYPE_PNLMENU_COPY_MirrorImage},
	{TYPE_PNLGRP1_COPY_ImageRepeat,			TYPE_PNLMENU_COPY_ImageRepeat},
	{TYPE_PNLGRP1_COPY_PosterMode,			TYPE_PNLMENU_COPY_PosterMode},
	{TYPE_PNLGRP1_COPY_ImageAdjust,			TYPE_PNLMENU_COPY_ImageAdjust},
	{TYPE_PNLGRP1_COPY_OHPInterleave,		TYPE_PNLMENU_COPY_OHPInterleave},
	{TYPE_PNLGRP1_COPY_CoverSheet,			TYPE_PNLMENU_COPY_CoverSheet},
	{TYPE_PNLGRP1_COPY_InsertSheet,			TYPE_PNLMENU_COPY_InsertSheet},
	{TYPE_PNLGRP1_COPY_InsertImage,			TYPE_PNLMENU_COPY_InsertImage},
	{TYPE_PNLGRP1_COPY_Chapter,				TYPE_PNLMENU_COPY_Chapter},
	{TYPE_PNLGRP1_COPY_DateTime,			TYPE_PNLMENU_COPY_DateTime},
	{TYPE_PNLGRP1_COPY_PageNumber,			TYPE_PNLMENU_COPY_PageNumber},
	{TYPE_PNLGRP1_COPY_Stamp,				TYPE_PNLMENU_COPY_Stamp},
	{TYPE_PNLGRP1_COPY_StampRepeat,			TYPE_PNLMENU_COPY_StampRepeat},
	{TYPE_PNLGRP1_COPY_HeaderFooter,		TYPE_PNLMENU_COPY_HeaderFooter},
	{TYPE_PNLGRP1_COPY_Watermark,			TYPE_PNLMENU_COPY_Watermark},
	{TYPE_PNLGRP1_COPY_Overlay,				TYPE_PNLMENU_COPY_Overlay},
	{TYPE_PNLGRP1_COPY_RegisteredOverlay,	TYPE_PNLMENU_COPY_RegisteredOverlay},
	{TYPE_PNLGRP1_COPY_CopyProtect,			TYPE_PNLMENU_COPY_CopyProtect},
	{TYPE_PNLGRP1_COPY_CopyGuard,			TYPE_PNLMENU_COPY_CopyGuard},
	{TYPE_PNLGRP1_COPY_PasswordCopy,		TYPE_PNLMENU_COPY_PasswordCopy},
	{TYPE_PNLGRP1_COPY_FrameErase,			TYPE_PNLMENU_COPY_FrameErase},
	{TYPE_PNLGRP1_COPY_NonImageAreaErase,	TYPE_PNLMENU_COPY_NonImageAreaErase},
	{TYPE_PNLGRP1_COPY_SaveBox,				TYPE_PNLMENU_COPY_SaveBox},
	{TYPE_PNLGRP1_COPY_ProgramJob,			TYPE_PNLMENU_COPY_ProgramJob},
	{TYPE_PNLGRP1_COPY_CardCopy,			TYPE_PNLMENU_COPY_CardCopy},
};

const PNLS_CorrespondFaxScanGrpID	sc_CorrespondFaxScanGrpIDTbl[] = {
	{TYPE_PNLGRP1_FAXSCAN_AddressBook,				TYPE_PNLMENU_FAXSCAN_AddressBook},
	{TYPE_PNLGRP1_FAXSCAN_DirectInput,				TYPE_PNLMENU_FAXSCAN_DirectInput},
	{TYPE_PNLGRP1_FAXSCAN_JobHistory,				TYPE_PNLMENU_FAXSCAN_JobHistory},
	{TYPE_PNLGRP1_FAXSCAN_EmailRx,					TYPE_PNLMENU_FAXSCAN_EmailRx},
	{TYPE_PNLGRP1_FAXSCAN_OffHook,					TYPE_PNLMENU_FAXSCAN_OffHook},
	{TYPE_PNLGRP1_FAXSCAN_DuplexSet,				TYPE_PNLMENU_FAXSCAN_DuplexSet},
	{TYPE_PNLGRP1_FAXSCAN_Resolution,				TYPE_PNLMENU_FAXSCAN_Resolution},
	{TYPE_PNLGRP1_FAXSCAN_Color,					TYPE_PNLMENU_FAXSCAN_Color},
	{TYPE_PNLGRP1_FAXSCAN_FileType,					TYPE_PNLMENU_FAXSCAN_FileType},
	{TYPE_PNLGRP1_FAXSCAN_ScanSize,					TYPE_PNLMENU_FAXSCAN_ScanSize},
	{TYPE_PNLGRP1_FAXSCAN_EmailSetting,				TYPE_PNLMENU_FAXSCAN_EmailSetting},
	{TYPE_PNLGRP1_FAXSCAN_OrgSet,					TYPE_PNLMENU_FAXSCAN_OrgSet},
	{TYPE_PNLGRP1_FAXSCAN_OrgBook,					TYPE_PNLMENU_FAXSCAN_OrgBook},
	{TYPE_PNLGRP1_FAXSCAN_SeparateScan,				TYPE_PNLMENU_FAXSCAN_SeparateScan},
	{TYPE_PNLGRP1_FAXSCAN_OrgType,					TYPE_PNLMENU_FAXSCAN_OrgType},
	{TYPE_PNLGRP1_FAXSCAN_Density,					TYPE_PNLMENU_FAXSCAN_Density},
	{TYPE_PNLGRP1_FAXSCAN_BackgroundRemoval,		TYPE_PNLMENU_FAXSCAN_BackgroundRemoval},
	{TYPE_PNLGRP1_FAXSCAN_FrameErase,				TYPE_PNLMENU_FAXSCAN_FrameErase},
	{TYPE_PNLGRP1_FAXSCAN_Sharpness,				TYPE_PNLMENU_FAXSCAN_Sharpness},
	{TYPE_PNLGRP1_FAXSCAN_Annotation,				TYPE_PNLMENU_FAXSCAN_Annotation},
	{TYPE_PNLGRP1_FAXSCAN_SendAndPrint,				TYPE_PNLMENU_FAXSCAN_SendAndPrint},
	{TYPE_PNLGRP1_FAXSCAN_FinishStamp,				TYPE_PNLMENU_FAXSCAN_FinishStamp},
	{TYPE_PNLGRP1_FAXSCAN_URLNotificationSetting,	TYPE_PNLMENU_FAXSCAN_URLNotificationSetting},
	{TYPE_PNLGRP1_FAXSCAN_EmailEncryption,			TYPE_PNLMENU_FAXSCAN_EmailEncryption},
	{TYPE_PNLGRP1_FAXSCAN_SmimeSignature,			TYPE_PNLMENU_FAXSCAN_SmimeSignature},
	{TYPE_PNLGRP1_FAXSCAN_FaxHeaderSetting,			TYPE_PNLMENU_FAXSCAN_FaxHeaderSetting},
	{TYPE_PNLGRP1_FAXSCAN_QuickMemoryTx,			TYPE_PNLMENU_FAXSCAN_QuickMemoryTx},
	{TYPE_PNLGRP1_FAXSCAN_LineSetting,				TYPE_PNLMENU_FAXSCAN_LineSetting},
	{TYPE_PNLGRP1_FAXSCAN_PollingTx,				TYPE_PNLMENU_FAXSCAN_PollingTx},
	{TYPE_PNLGRP1_FAXSCAN_PollingRx,				TYPE_PNLMENU_FAXSCAN_PollingRx},
	{TYPE_PNLGRP1_FAXSCAN_TimerTx,					TYPE_PNLMENU_FAXSCAN_TimerTx},
	{TYPE_PNLGRP1_FAXSCAN_PasswordTx,				TYPE_PNLMENU_FAXSCAN_PasswordTx},
	{TYPE_PNLGRP1_FAXSCAN_FCodeTx,					TYPE_PNLMENU_FAXSCAN_FCodeTx},
};

/*================================================================================================
	static メンバ変数の宣言.
================================================================================================*/
TYPE_PNLGRP1_COPY     PNLC_TopMenuManager::s_mmbCopyBasicFunction[ TYPD_TopMenuCopyMax ];
short                 PNLC_TopMenuManager::ss_mmbCopyBasicFunctionNum = 0;
TYPE_PNLGRP1_COPY     PNLC_TopMenuManager::s_mmbCopyApplicationFunction[ TYPE_PNLGRP1_COPY_CardCopy + 1 ];
short                 PNLC_TopMenuManager::ss_mmbCopyApplicationFunctionNum = 0;
TYPE_PNLGRP1_FAXSCAN  PNLC_TopMenuManager::s_mmbFaxScanBasicFunction[ TYPD_TopMenuScanFaxMax ];
short                 PNLC_TopMenuManager::ss_mmbFaxScanBasicFunctionNum = 0;
TYPE_PNLGRP1_FAXSCAN  PNLC_TopMenuManager::s_mmbFaxScanApplicationFunction[ TYPE_PNLGRP1_FAXSCAN_FCodeTx + 1 ];
short                 PNLC_TopMenuManager::ss_mmbFaxScanApplicationFunctionNum = 0;

// ===============================================================================================
/**
 *	@brief		コンストラクタ.
 */
// ===============================================================================================
PNLC_TopMenuManager::PNLC_TopMenuManager()
{
	memset( s_mmbCopyBasicFunction, 0x00, sizeof( s_mmbCopyBasicFunction ) );
	ss_mmbCopyBasicFunctionNum = 0;
	memset( s_mmbCopyApplicationFunction, 0x00, sizeof( s_mmbCopyApplicationFunction ) );
	ss_mmbCopyApplicationFunctionNum = 0;
	memset( s_mmbFaxScanBasicFunction, 0x00, sizeof( s_mmbFaxScanBasicFunction ) );
	ss_mmbFaxScanBasicFunctionNum = 0;
	memset( s_mmbFaxScanApplicationFunction, 0x00, sizeof( s_mmbFaxScanApplicationFunction ) );
	ss_mmbFaxScanApplicationFunctionNum = 0;
}

// ===============================================================================================
/**
 *	@brief		自クラスのインスタンス取得.
 *	@return		自クラスのインスタンスのポインタ.
 */
// ===============================================================================================
PNLC_TopMenuManager* PNLC_TopMenuManager::newInstance()
{
	static PNLC_TopMenuManager c_TopMenuManager;
	return &c_TopMenuManager;
}

// ===============================================================================================
/**
 *	@brief		トップメニューの表示位置をCommonから取得する.
 */
// ===============================================================================================
void PNLC_TopMenuManager::initTopMenuFunctionPos()
{
	PNLE_APPMODE_E e_AppMode = PnlComGetAppMode();	// アプリ種別を取得.
	if( PNLE_APPMODE_COPY == e_AppMode ) {
		// コピー用のバッファに検索結果を格納してもらう.
		initTopMenuFunctionPos( s_mmbCopyBasicFunction, TYPD_TopMenuCopyMax, &ss_mmbCopyBasicFunctionNum,
		                        s_mmbCopyApplicationFunction, TYPE_PNLGRP1_COPY_CardCopy, &ss_mmbCopyApplicationFunctionNum );
	} else if( PNLE_APPMODE_SCANFAX == e_AppMode ) {
		// Scan/Fax用のバッファに検索結果を格納してもらう.
		initTopMenuFunctionPos( s_mmbFaxScanBasicFunction, TYPD_TopMenuScanFaxMax, &ss_mmbFaxScanBasicFunctionNum,
		                        s_mmbFaxScanApplicationFunction, TYPE_PNLGRP1_FAXSCAN_FCodeTx, &ss_mmbFaxScanApplicationFunctionNum );
	}
}

// ===============================================================================================
/**
 *	@brief		トップメニューの表示位置をCommonから取得する（実処理部）.
 *	@param[in]	basic			基本機能配列.
 *	@param[in]	basicSize		基本機能配列サイズ.
 *	@param[in]	basicNum		基本機能数.
 *	@param[in]	application		応用機能配列.
 *	@param[in]	applicationSize	応用機能配列サイズ.
 *	@param[in]	applicationNum	応用機能数.
 */
// ===============================================================================================
template < class T1, class T2 >
void PNLC_TopMenuManager::initTopMenuFunctionPos(
    T1		basic,
    uchar	basicSize,
    short*	basicNum,
    T2		application,
    uchar	applicationSize,
    short*	applicationNum
) {
	// パラメータのチェック.
	if( NULL == basic || 0 == basicSize || NULL == application || 0 == applicationSize ) {
		return;
	}


	// アプリ種別を取得.
	PNLE_APPMODE_E e_AppMode = PnlComGetAppMode();

	// パターンカスタマイズクラスのインスタンス取得.
	TYPE_CustomPatternTarget e_PnlPtnSeting;
	if( PNLE_APPMODE_COPY == e_AppMode ) {
		e_PnlPtnSeting = TYPE_CustomPatternTargetCopy;
	} else if( PNLE_APPMODE_SCANFAX == e_AppMode ) {
		e_PnlPtnSeting = TYPE_CustomPatternTargetFaxScan;
	} else {
		return;
	}

	// メンバ変数の初期化.
	memset( basic, 0x00, basicSize );
	*basicNum = 0;
	memset( application, 0x00, applicationSize );
	*applicationNum = 0;

	uchar e_Setting = 0;							// 機能ID保持用バッファ.

	// 基本設定の並び順を取得.
	for( register uchar i=1; i<=basicSize; ++i ) {
		// アプリ種別に応じて、CommonAPIより並び順を取得.
		if( PNLE_APPMODE_COPY == e_AppMode ) {
			e_Setting = PnlComGetTopMenuCopySetting( ( uchar )( i - 1 ) );
			*basicNum = TYPD_TopMenuCopyMax;		// コピーの基本機能数.
		} else if( PNLE_APPMODE_SCANFAX == e_AppMode ) {
			e_Setting = PnlComGetTopMenuScanFaxSetting( ( uchar )( i - 1 ) );
			*basicNum = TYPD_TopMenuScanFaxMax;		// Scan/Faxの基本機能数.
		}
		// 表示・非表示を取得して表示時のみバッファに保持.
		if( True == PnlComGetFunctionShow( i , e_PnlPtnSeting ) ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
			basic[i-1] = e_Setting;
		}
	}

	Bool bl_AlreadyDisp = False;					// 基本設定で検出済みかどうかフラグ.
	// 応用設定の並び順を取得.
	for( register uchar i=1; i<=applicationSize; ++i ) {
		// 基本設定で既に検出済みかどうか.
		for( register uchar j=1; j<=basicSize; ++j ) {
			if( i == basic[j-1] ) {
				bl_AlreadyDisp = True;
			}
		}
		if( False == bl_AlreadyDisp ) {
			// 表示・非表示を取得して表示時のみバッファに保持.
			if( True == PnlComGetFunctionShow( i , e_PnlPtnSeting ) ) {		// MinervaSky Sky東島 UV-00743 ユーザー部門別パターン設定.
				application[i-1] = i;
				++( *applicationNum );
			}
		} else {
			bl_AlreadyDisp = False;					// フラグを戻す.
		}
	}
}

// ===============================================================================================
/**
 *	@brief		機能バッジ表示位置問い合わせ.
 *	@param[in]	uc_FunctionID		機能ID.
 *	@param[out]	*p_Rect				表示位置取得用.
 *	@param[out]	*p_FunctionType		ポップアップの表示種別（基本設定、応用設定、その他）.
 *	@retval		True				表示あり.
 *	@retval		False				表示なし.
 */
// ===============================================================================================
Bool PNLC_TopMenuManager::getFunctionPos( uchar uc_FunctionID, PNLS_Rect* p_Rect, PNLE_FunctionType* p_FunctionType )
{
	PNLE_APPMODE_E e_AppMode = PnlComGetAppMode();	// アプリ種別を取得.

	short ss_BasicSearchPos = -1;					// 基本設定の検索用変数.
	short ss_ApplicationSearchPos = -1;				// 応用設定の検索用変数.

	*p_FunctionType = PNLE_FunctionType_Other;		// 表示種別.

	if( PNLE_APPMODE_COPY == e_AppMode ) {
		// コピー基本設定テーブルを検索.
		for( register ushort i=1; i<=TYPD_TopMenuCopyMax; ++i ) {
			if( uc_FunctionID == s_mmbCopyBasicFunction[i-1] ) {
				ss_BasicSearchPos = i;									// ヒットしたインデックスを保持.
				*p_FunctionType = PNLE_FunctionType_Basic;				// 基本設定を返却.
				break;
			}
		}
		// 基本設定テーブルになかった場合は、応用設定テーブルを検索.
		if( ss_BasicSearchPos == -1 ) {
			for( register ushort i=1; i<=TYPE_PNLGRP1_COPY_CardCopy; ++i ) {
				if( uc_FunctionID == s_mmbCopyApplicationFunction[i-1] ) {
					ss_ApplicationSearchPos = i;						// ヒットしたインデックスを保持.
					*p_FunctionType = PNLE_FunctionType_Application;	// 応用設定を返却.
					break;
				}
			}
		}
	} else if( PNLE_APPMODE_SCANFAX == e_AppMode ) {
		// Scan/Fax基本設定テーブルを検索.
		for( register ushort j=1; j<=TYPD_TopMenuScanFaxMax; ++j ) {
			if( uc_FunctionID == s_mmbFaxScanBasicFunction[j-1] ) {
				ss_BasicSearchPos = j;									// ヒットしたインデックスを保持.
				*p_FunctionType = PNLE_FunctionType_Basic;				// 基本設定を返却.
				break;
			}
		}
		// 基本設定テーブルになかった場合は、応用設定テーブルを検索.
		if( ss_BasicSearchPos == -1 ) {
			for( register ushort j=1; j<=TYPE_PNLGRP1_FAXSCAN_FCodeTx; ++j ) {
				if( uc_FunctionID == s_mmbFaxScanApplicationFunction[j-1] ) {
					ss_ApplicationSearchPos = j;						// ヒットしたインデックスを保持.
					*p_FunctionType = PNLE_FunctionType_Application;	// 応用設定を返却.
					break;
				}
			}
		}
	}
	if( -1 != ss_BasicSearchPos ) {
		p_Rect->ss_PosX		= ( ( ss_BasicSearchPos % 2 ) - 1 ) * 96 + 8;
		p_Rect->ss_PosY		= 360;
		p_Rect->ss_Width	= 88;
		p_Rect->ss_Height	= 80;
	} else if( -1 != ss_ApplicationSearchPos ) {
		p_Rect->ss_PosX		= 16;
		p_Rect->ss_PosY		= 16;
		p_Rect->ss_Width	= 88;
		p_Rect->ss_Height	= 112;
	} else {
		return False;
	}

	return True;
}

// ===============================================================================================
/**
 *	@brief		機能IDが基本設定かどうかを問い合わせ.
 *	@param[in]	uc_FunctionID		機能ID.
 *	@retval		True				基本設定.
 *	@retval		False				それ以外（応用設定、もしくは該当なし）.
 */
// ===============================================================================================
Bool PNLC_TopMenuManager::isBasicFunction( uchar uc_FunctionID )
{
	PNLE_APPMODE_E e_AppMode = PnlComGetAppMode();	// アプリ種別を取得.

	if( PNLE_APPMODE_COPY == e_AppMode ) {
		// コピー基本設定テーブルに登録があるかどうかを検索.
		for( register ushort i=1; i<=TYPD_TopMenuCopyMax; ++i ) {
			if( uc_FunctionID == s_mmbCopyBasicFunction[i-1] ) {
				return True;
			}
		}
	} else if( PNLE_APPMODE_SCANFAX == e_AppMode ) {
		// Scan/Fax基本設定テーブルに登録があるかどうかを検索.
		for( register ushort j=1; j<=TYPD_TopMenuScanFaxMax; ++j ) {
			if( uc_FunctionID == s_mmbFaxScanBasicFunction[j-1] ) {
				return True;
			}
		}
	}
	return False;
}

// ===============================================================================================
/**
 *	@brief		機能バッジ表示位置問い合わせ_コピーの基本設定.
 *	@param[in]	e_FunctionID		機能ID.
 *	@param[out]	*p_Rect				表示位置取得用.
 *	@retval		True				表示あり.
 *	@retval		False				表示なし.
 */
// ===============================================================================================
Bool PNLC_TopMenuManager::getFunctionPosCopyBasic( TYPE_PNLGRP1_COPY e_FunctionID, PNLS_Rect* p_Rect )
{
//	char sc_BasicSearchPos = -1;					// 基本設定の検索用変数.	// 2014.11.12 T.Kitade 未使用変数削除.
	Bool ret = False;

	*p_Rect = PNLC_FunctionBadgeLib::newInstance()->moveToScreenFunctionBadgeRectCopy( e_FunctionID );
	if( p_Rect->ss_PosX != -1 ) {
		ret = True;
	}

	return ret;
}

// ===============================================================================================
/**
 *	@brief		機能バッジ表示位置問い合わせ_スキャン/ファクスの基本設定.
 *	@param[in]	e_FunctionID		機能ID.
 *	@param[out]	*p_Rect				表示位置取得用.
 *	@retval		True				表示あり.
 *	@retval		False				表示なし.
 */
// ===============================================================================================
Bool PNLC_TopMenuManager::getFunctionPosScanFaxBasic( TYPE_PNLGRP1_FAXSCAN e_FunctionID, PNLS_Rect* p_Rect )
{
//	char sc_BasicSearchPos = -1;					// 基本設定の検索用変数.	// 2014.11.12 T.Kitade 未使用変数削除.
	Bool ret = False;

	*p_Rect = PNLC_FunctionBadgeLib::newInstance()->moveToScreenFunctionBadgeRectScanFax( e_FunctionID );
	if( p_Rect->ss_PosX != -1 ) {
		ret = True;
	}

	return ret;
}

// ===============================================================================================
/**
 *	@brief		機能IDが基本設定かどうかを問い合わせ_コピーの基本設定.
 *	@param[in]	e_FunctionID		機能ID.
 *	@retval		True				基本設定.
 *	@retval		False				それ以外（応用設定、もしくは該当なし）.
 */
// ===============================================================================================
Bool PNLC_TopMenuManager::isBasicFunctionCopy( TYPE_PNLGRP1_COPY e_FunctionID )
{
	Bool ret = False;

	// コピー基本設定テーブルに登録があるかどうかを検索.
	for( uchar uc_copy = 0; uc_copy < TYPD_TopMenuCopyMax; uc_copy++ ) {
		if( e_FunctionID == s_mmbCopyBasicFunction[uc_copy] ) {
			ret = True;
		}
	}
	return ret;
}

// ===============================================================================================
/**
 *	@brief		機能IDが基本設定かどうかを問い合わせ_スキャン/ファクスの基本設定.
 *	@param[in]	e_FunctionID		機能ID.
 *	@retval		True				基本設定.
 *	@retval		False				それ以外（応用設定、もしくは該当なし）.
 */
// ===============================================================================================
Bool PNLC_TopMenuManager::isBasicFunctionScanFax( TYPE_PNLGRP1_FAXSCAN e_FunctionID )
{
	Bool ret = False;

	// スキャン/ファクス基本設定テーブルに登録があるかどうかを検索.
	for( uchar uc_scanfax = 0; uc_scanfax < TYPD_TopMenuScanFaxMax; uc_scanfax++ ) {
		if( e_FunctionID == s_mmbFaxScanBasicFunction[uc_scanfax] ) {
			ret = True;
		}
	}
	return ret;
}

