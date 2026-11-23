//=============================================================================.
//	インクルード・ファイル.
//=============================================================================.
#include "PNLC_StampLib.h"

//================================================================================================.
//	定数・型の宣言.
//================================================================================================.

//================================================================================================.
//	[クラス]	PNLC_StampLib : スタンプ設定ライブラリ.
//================================================================================================.

//================================================================================================.
/*================================================================================================
［機能］	種類追加処理
--------------------------------------------------------------------------------------------------
［引数］
		 PNLS_StampAddTypeInfo		pc_AddTypeInfo 	: スタンプ追加情報.
		 補足：必要パラメータ.
		 	pc_Common							// ジョブスペックポインタ.
		 	e_StampKind							// スタンプ種類.
		 	ss_AddIndex							// 追加対象のインデックス.
		 	e_AddJimonPattern					// 追加パターン種類（コピープロテクト時）.
		 	e_WatermarkPattern					// 追加パターン種類（繰り返しスタンプ時）.
		 	ss_AddLanguage						// 追加パターン言語.
		 	ss_AddPatternIndex					// 追加パターンインデックス.
		 	ss_AddPatternUsedArea				// 追加パターン使用領域.
		 	ss_CopyProtectMaxUsedArea			// 最大使用領域(０度：８, ＋４５度：４, －４５度：４).
		 	pss_CopyProtectNowUsedArea			// 使用済み領域ポインタ.
［戻値］	無し
================================================================================================*/
void PNLC_StampLib::addType( PNLS_StampAddTypeInfo* pc_AddTypeInfo )
{
	if( PNLE_StampKindWatermark == pc_AddTypeInfo->e_StampKind ) {
		addWatermarkType( pc_AddTypeInfo );
	} else {
		addCopyProtectType( pc_AddTypeInfo );
	}
}

/*================================================================================================
［機能］	コピープロテクト種類追加処理
--------------------------------------------------------------------------------------------------
［引数］
		 PNLS_StampAddTypeInfo		pc_AddTypeInfo 	: スタンプ追加情報.
［戻値］	無し
================================================================================================*/
void PNLC_StampLib::addCopyProtectType(	PNLS_StampAddTypeInfo* pc_AddTypeInfo )
{
	short ss_ShiftIndex = 0;	// シフト用インデックス.
	short ss_SearchIndex = 0;	// 検索用インデックス.
	// シフト用変数.
	short ss_ShiftLanguage;
	short ss_ShiftPatternIndex;
	TYPE_JimonPattern e_ShiftJimonPattern;
	// 追加パターンの領域確保 (追加対象のインデックスから追加パターンの使用領域分だけ後ろへシフトさせる).
	ss_ShiftIndex = pc_AddTypeInfo->ss_CopyProtectMaxUsedArea;
	for( ; ss_ShiftIndex >= ( pc_AddTypeInfo->ss_AddIndex + pc_AddTypeInfo->ss_AddPatternUsedArea ); ss_ShiftIndex-- ) {
		ss_SearchIndex = (short)( ss_ShiftIndex - pc_AddTypeInfo->ss_AddPatternUsedArea );
		// シフト対象のパターンを取得.
		e_ShiftJimonPattern = pc_AddTypeInfo->pc_Common->c_JimonPattern.getJimonPattern( ss_SearchIndex, ss_ShiftLanguage, ss_ShiftPatternIndex );
		//取得したパターンをシフト先に入れる.
		pc_AddTypeInfo->pc_Common->c_JimonPattern.setJimonPattern( ss_ShiftIndex, e_ShiftJimonPattern, ss_ShiftLanguage, ss_ShiftPatternIndex );
	}
	// パターン追加.
	for( ss_SearchIndex = pc_AddTypeInfo->ss_AddIndex; ss_SearchIndex < ( short )( pc_AddTypeInfo->ss_AddIndex+pc_AddTypeInfo->ss_AddPatternUsedArea ); ss_SearchIndex++ ) {
		pc_AddTypeInfo->pc_Common->c_JimonPattern.setJimonPattern( ss_SearchIndex, pc_AddTypeInfo->e_AddJimonPattern, pc_AddTypeInfo->ss_AddLanguage, pc_AddTypeInfo->ss_AddPatternIndex );
		*( pc_AddTypeInfo->pss_CopyProtectNowUsedArea ) = (short)( *( pc_AddTypeInfo->pss_CopyProtectNowUsedArea ) + 1 );		// 使用済み領域の計算.
	}
}

/*================================================================================================
［機能］	繰り返しスタンプ種類追加処理
--------------------------------------------------------------------------------------------------
［引数］
		 PNLS_StampAddTypeInfo		pc_AddTypeInfo 	: スタンプ追加情報.
［戻値］	無し
================================================================================================*/
void PNLC_StampLib::addWatermarkType( PNLS_StampAddTypeInfo* pc_AddTypeInfo )
{
	short ss_ShiftIndex = 0;	// シフト用インデックス.
	short ss_SearchIndex = 0;	// 検索用インデックス.
	short ss_ShiftLanguage;
	short ss_ShiftPatternIndex;
	TYPE_WatermarkPattern e_ShiftWatermarkPattern;
	// 追加パターンの領域確保 (追加対象のインデックスから追加パターンの使用領域分だけ後ろへシフトさせる).
	ss_ShiftIndex = pc_AddTypeInfo->ss_CopyProtectMaxUsedArea;
	for( ; ss_ShiftIndex >= ( pc_AddTypeInfo->ss_AddIndex + pc_AddTypeInfo->ss_AddPatternUsedArea ); ss_ShiftIndex-- ) {
		ss_SearchIndex = (short)( ss_ShiftIndex - pc_AddTypeInfo->ss_AddPatternUsedArea );
		// シフト対象のパターンを取得.
		e_ShiftWatermarkPattern = pc_AddTypeInfo->pc_Common->c_Watermark.getWatermarkPattern( ss_SearchIndex, ss_ShiftLanguage, ss_ShiftPatternIndex );
		//取得したパターンをシフト先に入れる.
		pc_AddTypeInfo->pc_Common->c_Watermark.setWatermarkPattern( ss_ShiftIndex, e_ShiftWatermarkPattern, ss_ShiftLanguage, ss_ShiftPatternIndex );
	}
	// パターン追加.
	for( ss_SearchIndex = pc_AddTypeInfo->ss_AddIndex; ss_SearchIndex < ( short )( pc_AddTypeInfo->ss_AddIndex+pc_AddTypeInfo->ss_AddPatternUsedArea ); ss_SearchIndex++ ) {
		pc_AddTypeInfo->pc_Common->c_Watermark.setWatermarkPattern( ss_SearchIndex, pc_AddTypeInfo->e_WatermarkPattern, pc_AddTypeInfo->ss_AddLanguage, pc_AddTypeInfo->ss_AddPatternIndex );
		*( pc_AddTypeInfo->pss_CopyProtectNowUsedArea ) = (short)( *( pc_AddTypeInfo->pss_CopyProtectNowUsedArea ) + 1 );		// 使用済み領域の計算.
	}
}

/*================================================================================================
［機能］	スタンプ削除処理
--------------------------------------------------------------------------------------------------
［引数］
		 PNLS_StampDeleteTypeInfo		pc_DeleteTypeInfo 	: スタンプ削除情報.
		 補足：必要パラメータ.
		 	pc_Common						// ジョブスペックポインタ.
		 	e_StampKind						// スタンプ種類.
		 	ss_DeleteIndex					// シフト用インデックス.
		 	bl_EraseSpace					// スペース削除フラグ？.
		 	ss_CopyProtectMaxUsedArea		// 最大使用領域(０度：８, ＋４５度：４, －４５度：４).
		 	pss_CopyProtectNowUsedArea		// 使用済み領域.
［戻値］	無し
================================================================================================*/
void PNLC_StampLib::deleteType( PNLS_StampDeleteTypeInfo* pc_DeleteTypeInfo )
{
	if( PNLE_StampKindWatermark == pc_DeleteTypeInfo->e_StampKind ) { 	// 繰り返しスタンプ.
		deleteWatermarkType( pc_DeleteTypeInfo );
	} else {										// その他.
		deleteCopyProtectType( pc_DeleteTypeInfo );
	}
}

/*================================================================================================
［機能］	コピープロテクト削除処理
--------------------------------------------------------------------------------------------------
［引数］
		 PNLS_StampDeleteTypeInfo		pc_DeleteTypeInfo 	: スタンプ追加情報.
［戻値］	無し
================================================================================================*/
void PNLC_StampLib::deleteCopyProtectType( PNLS_StampDeleteTypeInfo* pc_DeleteTypeInfo )
{
	short ss_ShiftIndex = (short)( pc_DeleteTypeInfo->ss_DeleteIndex+1 );	// シフト用インデックス.
	// 削除対象パターン検索用変数.
	short ss_NowLanguage = 0;
	short ss_NowPatternIndex = 0;
	TYPE_JimonPattern e_JimonPattern;
	// シフト用変数.
	short ss_ShiftLanguage1;
	short ss_ShiftLanguage2;
	short ss_ShiftPatternIndex1;
	short ss_ShiftPatternIndex2;
	TYPE_JimonPattern e_ShiftJimonPattern1;
	TYPE_JimonPattern e_ShiftJimonPattern2;
	// 削除対象パターン取得.
	TYPE_JimonPattern e_DeleteJimonPattern = pc_DeleteTypeInfo->pc_Common->c_JimonPattern.getJimonPattern( pc_DeleteTypeInfo->ss_DeleteIndex, ss_NowLanguage, ss_NowPatternIndex );
	// 削除対象パターンをOFFにする.
	pc_DeleteTypeInfo->pc_Common->c_JimonPattern.setJimonPattern( pc_DeleteTypeInfo->ss_DeleteIndex, TYPE_JimonPatternOff,0, 0 );
	*( pc_DeleteTypeInfo->pss_CopyProtectNowUsedArea ) = (short)( *( pc_DeleteTypeInfo->pss_CopyProtectNowUsedArea ) - 1 );		// 使用済み領域の計算.
	// 削除対象パターンが登録パターンの場合は複数行削除.
	if( e_DeleteJimonPattern == TYPE_JimonPatternArbitrary ) {
		for( ; ss_ShiftIndex < ( short )( pc_DeleteTypeInfo->ss_CopyProtectMaxUsedArea+1 ); ss_ShiftIndex++ ) {
			e_JimonPattern = pc_DeleteTypeInfo->pc_Common->c_JimonPattern.getJimonPattern( ss_ShiftIndex, ss_NowLanguage, ss_NowPatternIndex );
			if( e_JimonPattern != TYPE_JimonPatternArbitrary ) {
				// 登録パターン以外になったらぬける.
				break;
			}
			pc_DeleteTypeInfo->pc_Common->c_JimonPattern.setJimonPattern( ss_ShiftIndex, TYPE_JimonPatternOff,0, 0 );
			*( pc_DeleteTypeInfo->pss_CopyProtectNowUsedArea ) = (short)( *( pc_DeleteTypeInfo->pss_CopyProtectNowUsedArea ) - 1 );		// 使用済み領域の計算.
		}
	} else if( e_DeleteJimonPattern == TYPE_JimonPatternDate ) {	// 削除対象パターンが日付のとき.
		// 削除対象パターンの次が時刻のとき.
		if( pc_DeleteTypeInfo->ss_DeleteIndex != pc_DeleteTypeInfo->ss_CopyProtectMaxUsedArea
		    && pc_DeleteTypeInfo->pc_Common->c_JimonPattern.getJimonPattern( (short)( pc_DeleteTypeInfo->ss_DeleteIndex+1 ), ss_NowLanguage, ss_NowPatternIndex ) == TYPE_JimonPatternTime ) {
			pc_DeleteTypeInfo->pc_Common->c_JimonPattern.setJimonPattern( ss_ShiftIndex, TYPE_JimonPatternOff,0, 0 );
			*( pc_DeleteTypeInfo->pss_CopyProtectNowUsedArea ) = (short)( *( pc_DeleteTypeInfo->pss_CopyProtectNowUsedArea ) - 1 );		// 使用済み領域の計算.
			ss_ShiftIndex++;
		}
		if( pc_DeleteTypeInfo->bl_EraseSpace == True ) {
			pc_DeleteTypeInfo->pc_Common->c_JimonPattern.setDateKind( TYPE_DateKind1 );			// 日付の種類を初期値に戻す.
			pc_DeleteTypeInfo->pc_Common->c_JimonPattern.setTimeKind( TYPE_TimeKindOff );		// 時刻の種類を初期値に戻す.
		}
	}
	// OFFにした削除パターン以降のパターンを前に詰める.
	for( ; ss_ShiftIndex < ( short )( pc_DeleteTypeInfo->ss_CopyProtectMaxUsedArea+1 ); ss_ShiftIndex++ ) {
		e_ShiftJimonPattern1 = pc_DeleteTypeInfo->pc_Common->c_JimonPattern.getJimonPattern( pc_DeleteTypeInfo->ss_DeleteIndex, ss_ShiftLanguage1, ss_ShiftPatternIndex1 );
		e_ShiftJimonPattern2 = pc_DeleteTypeInfo->pc_Common->c_JimonPattern.getJimonPattern( ss_ShiftIndex, ss_ShiftLanguage2, ss_ShiftPatternIndex2 );
		pc_DeleteTypeInfo->pc_Common->c_JimonPattern.setJimonPattern( pc_DeleteTypeInfo->ss_DeleteIndex, e_ShiftJimonPattern2,ss_ShiftLanguage2, ss_ShiftPatternIndex2 );
		pc_DeleteTypeInfo->pc_Common->c_JimonPattern.setJimonPattern( ss_ShiftIndex,  e_ShiftJimonPattern1,ss_ShiftLanguage1, ss_ShiftPatternIndex1 );
		pc_DeleteTypeInfo->ss_DeleteIndex = (short)( pc_DeleteTypeInfo->ss_DeleteIndex + 1 );
	}
	// 残りパターンがスペースのみか判定する.
	Bool bl_onlySpace = True;
	for( uchar i = 1; i < 9; i++ ) {
		e_JimonPattern = pc_DeleteTypeInfo->pc_Common->c_JimonPattern.getJimonPattern( i, ss_NowLanguage, ss_NowPatternIndex );
		if( e_JimonPattern == TYPE_JimonPatternOff ) {			// パターンがOFFのときは判定終了.
			break;
		} else if( e_JimonPattern != TYPE_JimonPatternSpace ) {	// パターンがスペース以外のときは判定終了.
			bl_onlySpace = False;	// スペース以外のパターンあり.
			break;
		}
	}
	if( pc_DeleteTypeInfo->bl_EraseSpace == True ) {
		// 残りがスペースだけになったとき、パターンを全てOFF.
		if( bl_onlySpace == True ) {
			*( pc_DeleteTypeInfo->pss_CopyProtectNowUsedArea ) = 0;		// 使用済み領域の計算.
			for( uchar i = 1; i < 9; i++ ) {
				pc_DeleteTypeInfo->pc_Common->c_JimonPattern.setJimonPattern( i, TYPE_JimonPatternOff, 0, 0 );
			}
		}
	}
	// パターンが一つも入っていない時、地紋OFF.
	e_JimonPattern = pc_DeleteTypeInfo->pc_Common->c_JimonPattern.getJimonPattern( 1, ss_NowLanguage, ss_NowPatternIndex );
	if( e_JimonPattern == TYPE_JimonPatternOff ) {
		// 地紋OFF.
		if( PNLE_StampKindJimon == pc_DeleteTypeInfo->e_StampKind ) {			// コピープロテクト.
			pc_DeleteTypeInfo->pc_Common->c_JimonPattern.setOn( False );
		} else if( PNLE_StampKindCopyGuard == pc_DeleteTypeInfo->e_StampKind ) {	// コピーガード.
			pc_DeleteTypeInfo->pc_Common->getJimonPattern2()->setCopyGuardOn( False );
		}
		else if( PNLE_StampKindPassWordCopy == pc_DeleteTypeInfo->e_StampKind ) {	// パスワードコピー.
			pc_DeleteTypeInfo->pc_Common->getJimonPattern2()->setPasswordCopyOn( False );
		}
	}
}

/*================================================================================================
［機能］	繰り返しスタンプ削除処理
--------------------------------------------------------------------------------------------------
［引数］
		 PNLS_StampDeleteTypeInfo		pc_DeleteTypeInfo 	: スタンプ追加情報.
［戻値］	無し
================================================================================================*/
void PNLC_StampLib::deleteWatermarkType( PNLS_StampDeleteTypeInfo* pc_DeleteTypeInfo )
{
	short ss_ShiftIndex = (short)( pc_DeleteTypeInfo->ss_DeleteIndex+1 );	// シフト用インデックス.
	// 削除対象パターン検索用変数.
	short ss_NowLanguage = 0;
	short ss_NowPatternIndex = 0;
	TYPE_WatermarkPattern e_WatermarkPattern;
	// シフト用変数.
	short ss_ShiftLanguage1;
	short ss_ShiftLanguage2;
	short ss_ShiftPatternIndex1;
	short ss_ShiftPatternIndex2;
	TYPE_WatermarkPattern e_ShiftWatermarkPattern1;
	TYPE_WatermarkPattern e_ShiftWatermarkPattern2;
	// 削除対象パターン取得.
	TYPE_WatermarkPattern e_DeleteWatermarkPattern = pc_DeleteTypeInfo->pc_Common->c_Watermark.getWatermarkPattern( pc_DeleteTypeInfo->ss_DeleteIndex, ss_NowLanguage, ss_NowPatternIndex );
	// 削除対象パターンをOFFにする.
	pc_DeleteTypeInfo->pc_Common->c_Watermark.setWatermarkPattern( pc_DeleteTypeInfo->ss_DeleteIndex, TYPE_WatermarkPatternOff,0, 0 );
	*( pc_DeleteTypeInfo->pss_CopyProtectNowUsedArea ) = (short)( *( pc_DeleteTypeInfo->pss_CopyProtectNowUsedArea ) - 1 );		// 使用済み領域の計算.
	// 削除対象パターンが登録パターンの場合は複数行削除.
	if( e_DeleteWatermarkPattern == TYPE_WatermarkPatternArbitrary ) {
		for( ; ss_ShiftIndex < ( short )( pc_DeleteTypeInfo->ss_CopyProtectMaxUsedArea+1 ); ss_ShiftIndex++ ) {
			e_WatermarkPattern = pc_DeleteTypeInfo->pc_Common->c_Watermark.getWatermarkPattern( ss_ShiftIndex, ss_NowLanguage, ss_NowPatternIndex );
			if( e_WatermarkPattern != TYPE_WatermarkPatternArbitrary ) {
				// 登録パターン以外になったらぬける.
				break;
			}
			pc_DeleteTypeInfo->pc_Common->c_Watermark.setWatermarkPattern( ss_ShiftIndex, TYPE_WatermarkPatternOff,0, 0 );
			*( pc_DeleteTypeInfo->pss_CopyProtectNowUsedArea ) = (short)( *( pc_DeleteTypeInfo->pss_CopyProtectNowUsedArea ) - 1 );		// 使用済み領域の計算.
		}
	} else if( e_DeleteWatermarkPattern == TYPE_WatermarkPatternDate ) {	// 削除対象パターンが日付のとき.
		// 削除対象パターンの次が時刻のとき.
		if( pc_DeleteTypeInfo->ss_DeleteIndex != pc_DeleteTypeInfo->ss_CopyProtectMaxUsedArea
		    && pc_DeleteTypeInfo->pc_Common->c_Watermark.getWatermarkPattern( (short)( pc_DeleteTypeInfo->ss_DeleteIndex+1 ), ss_NowLanguage, ss_NowPatternIndex ) == TYPE_WatermarkPatternTime ) {
			pc_DeleteTypeInfo->pc_Common->c_Watermark.setWatermarkPattern( ss_ShiftIndex, TYPE_WatermarkPatternOff,0, 0 );
			*( pc_DeleteTypeInfo->pss_CopyProtectNowUsedArea ) = (short)( *( pc_DeleteTypeInfo->pss_CopyProtectNowUsedArea ) - 1 );		// 使用済み領域の計算.
			ss_ShiftIndex++;
		}
		if( pc_DeleteTypeInfo->bl_EraseSpace == True ) {
			pc_DeleteTypeInfo->pc_Common->c_Watermark.setDateKind( TYPE_DateKind1 );			// 日付の種類を初期値に戻す.
			pc_DeleteTypeInfo->pc_Common->c_Watermark.setTimeKind( TYPE_TimeKindOff );		// 時刻の種類を初期値に戻す.
		}
	}
	// OFFにした削除パターン以降のパターンを前に詰める.
	for( ; ss_ShiftIndex < ( short )( pc_DeleteTypeInfo->ss_CopyProtectMaxUsedArea+1 ); ss_ShiftIndex++ ) {
		e_ShiftWatermarkPattern1 = pc_DeleteTypeInfo->pc_Common->c_Watermark.getWatermarkPattern( pc_DeleteTypeInfo->ss_DeleteIndex, ss_ShiftLanguage1, ss_ShiftPatternIndex1 );
		e_ShiftWatermarkPattern2 = pc_DeleteTypeInfo->pc_Common->c_Watermark.getWatermarkPattern( ss_ShiftIndex, ss_ShiftLanguage2, ss_ShiftPatternIndex2 );
		pc_DeleteTypeInfo->pc_Common->c_Watermark.setWatermarkPattern( pc_DeleteTypeInfo->ss_DeleteIndex, e_ShiftWatermarkPattern2,ss_ShiftLanguage2, ss_ShiftPatternIndex2 );
		pc_DeleteTypeInfo->pc_Common->c_Watermark.setWatermarkPattern( ss_ShiftIndex,  e_ShiftWatermarkPattern1,ss_ShiftLanguage1, ss_ShiftPatternIndex1 );
		pc_DeleteTypeInfo->ss_DeleteIndex = (short)( pc_DeleteTypeInfo->ss_DeleteIndex + 1 );
	}
	// 残りパターンがスペースのみか判定する.
	Bool bl_onlySpace = True;
	for( uchar i = 1; i < 9; i++ ) {
		e_WatermarkPattern = pc_DeleteTypeInfo->pc_Common->c_Watermark.getWatermarkPattern( i, ss_NowLanguage, ss_NowPatternIndex );
		if( e_WatermarkPattern == TYPE_WatermarkPatternOff ) {			// パターンがOFFのときは判定終了.
			break;
		} else if( e_WatermarkPattern != TYPE_WatermarkPatternSpace ) {	// パターンがスペース以外のときは判定終了.
			bl_onlySpace = False;	// スペース以外のパターンあり.
			break;
		}
	}
	if( pc_DeleteTypeInfo->bl_EraseSpace == True ) {
		// 残りがスペースだけになったとき、パターンを全てOFF.
		if( bl_onlySpace == True ) {
			*( pc_DeleteTypeInfo->pss_CopyProtectNowUsedArea ) = 0;		// 使用済み領域の計算.
			for( uchar i = 1; i < 9; i++ ) {
				pc_DeleteTypeInfo->pc_Common->c_Watermark.setWatermarkPattern( i, TYPE_WatermarkPatternOff, 0, 0 );
			}
		}
	}
	// パターンが一つも入っていない時、地紋OFF.
	e_WatermarkPattern = pc_DeleteTypeInfo->pc_Common->c_Watermark.getWatermarkPattern( 1, ss_NowLanguage, ss_NowPatternIndex );
	if( e_WatermarkPattern == TYPE_WatermarkPatternOff ) {
		// 地紋OFF.
		pc_DeleteTypeInfo->pc_Common->c_Watermark.setOn( False );
	}
}
//================================================================================================.
//	[機能]	クラスインスタンスの取得.
//------------------------------------------------------------------------------------------------.
//	[引数]	無し.
//	[戻値]	PNLC_StampLib* .
//================================================================================================.
PNLC_StampLib* PNLC_StampLib::newInstance() {
	static PNLC_StampLib c_StampLib;
	return &c_StampLib ;

}

