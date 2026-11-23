//=============================================================================.
//	インクルード・ファイル.
//=============================================================================.
#include "plibh_PD.h"
#include <panel/plibh/plibh_FUM2/plibh_FUM2.h>
#include <panel/plibh/plibhg/plibh2.h>
#include <panel/pdp/pdpg/pdpdef.h>
#include <panel/pcgbox/pcgbox_CLCD/pipbox_CLCD.h>
#include <panel/plibh/plibh_CLCD2/PNLC_BoxDataLib.h>
#include <panel/pcgcom_ext/pcgcom_extg/PCGC_ScanOapCom.h>
#include <panel/pcgscanfax/pcgscanfax_CLCD/pipscanfax_CLCD2.h>
#include <panel/plibh/plibhg/plibh.h>
#include <panel/ppcg/ppcg_CLCD/ppcginclude_CLCD.h>
#include <panel/plibh/plibh_CLCD/plibhspcparts_CLCD.h>
#include <panel/pddtext/pddtext_CLCD/pstr_CLCD.h>
#include <panel/pdp/pdp_CLCD/pdpdef_CLCD.h>
#include <panel/pdp/pdpg/pdpdefenl.h>
#include <panel/pcgcom/pcgcomg/pcgcom.h>
#include <panel/pcgutil/pcgutil_CLCD/pcgutility_CLCD.h>
#include <panel/plibh/plibh_CLCD/plibh_CLCD.h>
#include <panel/plibh/plibhg/plibh3.h>
#include <panel/plibh/plibh_PD/plibh_PD.h>
#include <panel/plibh/plibhg/pdpcomb.h>
#include <panel/pddgrap/pddgrap_CLCD/pddgrapinclude_CLCD.h>
#include <panel/plibh/plibh_CLCD2/plibh_CLCD2.h>
#include <panel/pcgadm/pcgadmg/pcgadmin1.h>
//comment [IT5_4.2][HCl-suribabu][RQ-686][WP914] added header file here.
#include <panel/plibh/plibh_CLCD2/PNLC_NetworkLan.h>
//================================================================================================.
//	定数・型の宣言.
//================================================================================================.
// ローテーショングラフィック用のパレット（とりあえず５０個分）
uchar PaletteTableCommon[] = {
	PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, 
	PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, 
	PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, 
	PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, 
	PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, PNLD_PaletteCommon, 
};
uchar PaletteTableCopy[] = {
	PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, 
	PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, 
	PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, 
	PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, 
	PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, PNLD_PaletteCopy, 
};
uchar PaletteTableFaxScan[] = {
	PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, 
	PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, 
	PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, 
	PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, 
	PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, PNLD_PaletteFaxScan, 
};
uchar PaletteTableBox[] = {
	PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, 
	PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, 
	PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, 
	PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, 
	PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, PNLD_PaletteBox, 
};
uchar PaletteTableAdmin[] = {
	PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, 
	PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, 
	PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, 
	PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, 
	PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, PNLD_PaletteAdmin, 
};
// サイクリックプログレスアイコン.
static PSinglePartsGraphic Gra_ProgressivePoint1_1( m30_064_000_1_X88_Y88, 0, 0, PNLD_PaletteCommon, TYPE_LayoutCentering, True);		// サイクリックプログレスアイコン1.
static PSinglePartsGraphic Gra_ProgressivePoint1_2( m30_064_000_2_X88_Y88, 0, 0, PNLD_PaletteCommon, TYPE_LayoutCentering, True);		// サイクリックプログレスアイコン2.
static PSinglePartsGraphic Gra_ProgressivePoint1_3( m30_064_000_3_X88_Y88, 0, 0, PNLD_PaletteCommon, TYPE_LayoutCentering, True);		// サイクリックプログレスアイコン3.
static PSinglePartsGraphic Gra_ProgressivePoint1_4( m30_064_000_4_X88_Y88, 0, 0, PNLD_PaletteCommon, TYPE_LayoutCentering, True);		// サイクリックプログレスアイコン4.
static PSinglePartsGraphic Gra_ProgressivePoint1_5( m30_064_000_5_X88_Y88, 0, 0, PNLD_PaletteCommon, TYPE_LayoutCentering, True);		// サイクリックプログレスアイコン5.
static PSinglePartsGraphic Gra_ProgressivePoint1_6( m30_064_000_6_X88_Y88, 0, 0, PNLD_PaletteCommon, TYPE_LayoutCentering, True);		// サイクリックプログレスアイコン6.
static PSinglePartsGraphic Gra_ProgressivePoint1_7( m30_064_000_7_X88_Y88, 0, 0, PNLD_PaletteCommon, TYPE_LayoutCentering, True);		// サイクリックプログレスアイコン7.
static PSinglePartsGraphic Gra_ProgressivePoint1_8( m30_064_000_8_X88_Y88, 0, 0, PNLD_PaletteCommon, TYPE_LayoutCentering, True);		// サイクリックプログレスアイコン8.
PDisplayParts* tbl_ProgressivePoint1[] = {
	&Gra_ProgressivePoint1_1,
	&Gra_ProgressivePoint1_2,
	&Gra_ProgressivePoint1_3,
	&Gra_ProgressivePoint1_4,
	&Gra_ProgressivePoint1_5,
	&Gra_ProgressivePoint1_6,
	&Gra_ProgressivePoint1_7,
	&Gra_ProgressivePoint1_8,
};
// comment 2016/08/08 MSE寺田 [IT5_3.3][RQ-440][WBS37][WBS39] 小さいプログレッシブアイコンを追加.
// サイクリックプログレスアイコン.
static PSinglePartsGraphic Gra_ProgressivePoint2_1( m30_064_001_1_X20_Y20, 0, 0, PNLD_PaletteCommon, TYPE_LayoutCentering, True);		// サイクリックプログレスアイコン1(小).
static PSinglePartsGraphic Gra_ProgressivePoint2_2( m30_064_001_2_X20_Y20, 0, 0, PNLD_PaletteCommon, TYPE_LayoutCentering, True);		// サイクリックプログレスアイコン2(小).
static PSinglePartsGraphic Gra_ProgressivePoint2_3( m30_064_001_3_X20_Y20, 0, 0, PNLD_PaletteCommon, TYPE_LayoutCentering, True);		// サイクリックプログレスアイコン3(小).
static PSinglePartsGraphic Gra_ProgressivePoint2_4( m30_064_001_4_X20_Y20, 0, 0, PNLD_PaletteCommon, TYPE_LayoutCentering, True);		// サイクリックプログレスアイコン4(小).
static PSinglePartsGraphic Gra_ProgressivePoint2_5( m30_064_001_5_X20_Y20, 0, 0, PNLD_PaletteCommon, TYPE_LayoutCentering, True);		// サイクリックプログレスアイコン5(小).
static PSinglePartsGraphic Gra_ProgressivePoint2_6( m30_064_001_6_X20_Y20, 0, 0, PNLD_PaletteCommon, TYPE_LayoutCentering, True);		// サイクリックプログレスアイコン6(小).
static PSinglePartsGraphic Gra_ProgressivePoint2_7( m30_064_001_7_X20_Y20, 0, 0, PNLD_PaletteCommon, TYPE_LayoutCentering, True);		// サイクリックプログレスアイコン7(小).
static PSinglePartsGraphic Gra_ProgressivePoint2_8( m30_064_001_8_X20_Y20, 0, 0, PNLD_PaletteCommon, TYPE_LayoutCentering, True);		// サイクリックプログレスアイコン8(小).
PDisplayParts* tbl_ProgressivePoint2[] = {
	&Gra_ProgressivePoint2_1,
	&Gra_ProgressivePoint2_2,
	&Gra_ProgressivePoint2_3,
	&Gra_ProgressivePoint2_4,
	&Gra_ProgressivePoint2_5,
	&Gra_ProgressivePoint2_6,
	&Gra_ProgressivePoint2_7,
	&Gra_ProgressivePoint2_8,
};

//	[クラス]	PNLC_CommonStampLib : スタンプ設定処理共通化ライブラリ.
// クラスインスタンスの取得.
PNLC_CommonStampLib* PNLC_CommonStampLib::newInstance( PNLE_StampPatternType e_StampType ) {
	switch( e_StampType ){
		case PNLE_StampPatternType_Watermark:	{
			static PNLC_CommonStampLib c_WatermarkLib( e_StampType );
			return &c_WatermarkLib ;
		}
		break;
		case PNLE_StampPatternType_Jimon:	{
			static PNLC_CommonStampLib c_StampLib( e_StampType );
			return &c_StampLib ;
		}
		break;
		default:
		break;
	}
	return NULL;
}

// スタンプ共通処理.
// 最大使用可能領域.
short PNLC_CommonStampLib::getMaxUsableArea( APIC_Job* pc_Job )
{
	short ss_Angle = 0;
	switch( m_e_StampPatternType ) {		// スタンプタイプにより使用IF変更.
		case PNLE_StampPatternType_Watermark:		ss_Angle = pc_Job->getJobSpec()->pc_Common->c_Watermark.getAngle();		break;	// ウォーターマーク.
		case PNLE_StampPatternType_Jimon:			ss_Angle = pc_Job->getJobSpec()->pc_Common->c_JimonPattern.getAngle();	break;	// 地紋.
		default:		break;
	}
	switch( ss_Angle ) {
		case   0: return 8;	break;	//   ０°  最大使用領域８.
		case  45: return 4;	break;	// ＋４５°最大使用領域４.
		case 315: return 4;	break;	// －４５°最大使用領域４.
		default:		break;
	}
	return	0;
}
// 使用済領域.
short PNLC_CommonStampLib::getNowUsedArea( APIC_Job* pc_Job )
{
	short ss_NowUsedArea = 0;	// 使用済領域 計算.
	for( uchar i = 1; i <= 8; i++ ) {
		short ss_Language = 0; short ss_PatternIndex = 0;
		switch( m_e_StampPatternType ) {		// スタンプタイプにより使用IF変更.
			case PNLE_StampPatternType_Watermark:		if( TYPE_WatermarkPatternOff == pc_Job->getJobSpec()->pc_Common->c_Watermark.getWatermarkPattern( i, ss_Language, ss_PatternIndex ) ) {	return	ss_NowUsedArea;	}	break;	// ウォーターマーク.
			case PNLE_StampPatternType_Jimon:			if( TYPE_JimonPatternOff == pc_Job->getJobSpec()->pc_Common->c_JimonPattern.getJimonPattern( i, ss_Language, ss_PatternIndex ) ) {	return	ss_NowUsedArea;	}	break;		// 地紋.
			default:		break;
		}
		ss_NowUsedArea++;
	}
	return	ss_NowUsedArea;
}
// スタンプOn/Off判定（戻り値：-1=OFF/1以上=ON+機能の行数）.
short PNLC_CommonStampLib::isOnStampPattern( APIC_Job* pc_Job, char c_Pattern )
{
	short ss_MaxUsedArea = getMaxUsableArea( pc_Job );	// 最大使用可能領域.
	for( short i = 1; i <= ss_MaxUsedArea; i++ ) {
		short ss_Language = 0; short ss_PatternIndex = 0;
		if( c_Pattern == getStampPattern( pc_Job, i, ss_Language, ss_PatternIndex ) ) {
			return	i;
		}
	}
	return	-1;
}
// スタンプパターンＯＮ.
Bool PNLC_CommonStampLib::setStampPatternOn( APIC_Job* pc_Job, char c_Pattern, short ss_PatternIndex, short ss_Count )
{
	short ss_MaxUsedArea = getMaxUsableArea( pc_Job );	short ss_NowUsedArea = getNowUsedArea( pc_Job );
	Bool b_isRegistTime = false;
	switch( m_e_StampPatternType ) {		// スタンプタイプにより使用IF変更.
		case PNLE_StampPatternType_Watermark:		b_isRegistTime = ( c_Pattern == static_cast<char>( TYPE_WatermarkPatternTime ) ) ? True : False;		break;	// ウォーターマーク.
		case PNLE_StampPatternType_Jimon:			b_isRegistTime = ( c_Pattern == static_cast<char>( TYPE_JimonPatternTime ) ) ? True : False;			break;	// 地紋.
		default:		break;
	}
	
	if( True == b_isRegistTime ) {	// ＯＮにするのが時刻パターンの時.
		if( isOnStampPattern( pc_Job, c_Pattern ) == -1 ) {	// 時刻パターンが登録されていない時.
			if( ss_NowUsedArea >= ss_MaxUsedArea ) {	// 使用可能領域がない場合は領域オーバー警告発生.
				return False;
			}
			char c_PatternData = 0;
			switch( m_e_StampPatternType ) {		// スタンプタイプにより使用IF変更.
				case PNLE_StampPatternType_Watermark:		c_PatternData = TYPE_WatermarkPatternDate;		break;	// ウォーターマーク.
				case PNLE_StampPatternType_Jimon:			c_PatternData = TYPE_JimonPatternDate;			break;	// 地紋.
				default:		break;
			}
			short ss_Date = isOnStampPattern( pc_Job, c_PatternData );	// 日付パターンが既に存在しているはずで、その次に時刻パターンを登録する.
			addStampPattern( ss_Date+1, pc_Job, c_Pattern, PnlComGetLanguage(), ss_PatternIndex, ss_Count );	// スタンプパターンの追加.
		}
		return	True;
	}
	// 時刻パターン以外は共通処理でＯＫ.
	for( short i = 1; i <= ss_MaxUsedArea; i++ ) {
		short ss_TmpLanguage = 0;	short ss_TmpPatternIndex = 0;	char c_PatternOff = 0;	char c_PatternTemp = getStampPattern( pc_Job, i, ss_TmpLanguage, ss_TmpPatternIndex );	
			switch( m_e_StampPatternType ) {		// スタンプタイプにより使用IF変更.
				case PNLE_StampPatternType_Watermark:		c_PatternOff = TYPE_WatermarkPatternOff;	break;	// ウォーターマーク.
				case PNLE_StampPatternType_Jimon:			c_PatternOff = TYPE_JimonPatternOff;		break;	// 地紋.
				default:		break;
			}
		if( c_PatternTemp == c_Pattern ) {	// 既に設定済み.
			short ss_NowUsedPatternArea = 1;					// 設定済みパターンのサイズ.
			for( short j = i + 1; j <= ss_MaxUsedArea; j++ ) {		// 次のインデックスが同じパターンが確認.
				char c_PatternTempNext = getStampPattern( pc_Job, j, ss_TmpLanguage, ss_TmpPatternIndex );
				if( c_PatternTemp != c_PatternTempNext ) {	break;	}
				++ss_NowUsedPatternArea;
			}
			if( ss_Count > ss_MaxUsedArea - ss_NowUsedArea + ss_NowUsedPatternArea ) {	// 領域オーバー警告発生.
				return	False;
			}
			deleteStampPattern( i, pc_Job );	// スタンプパターンの削除.
			addStampPattern( i, pc_Job, c_Pattern, PnlComGetLanguage(), ss_PatternIndex, ss_Count );	// スタンプパターンの追加.
			return	True;
		} else if( c_PatternTemp == c_PatternOff ) {
			if( i+ss_Count-1 >= ss_MaxUsedArea+1 ) {	// 領域オーバー警告発生.
				return	False;
			}
			addStampPattern( i, pc_Job, c_Pattern, PnlComGetLanguage(), ss_PatternIndex, ss_Count );	// スタンプパターンの追加.
			switch( m_e_StampPatternType ) {		// スタンプタイプにより使用IF変更.
				case PNLE_StampPatternType_Watermark:		pc_Job->getJobSpec()->pc_Common->c_Watermark.setOn( True );		break;	// ウォーターマーク.
				case PNLE_StampPatternType_Jimon:			pc_Job->getJobSpec()->pc_Common->c_JimonPattern.setOn( True );	break;	// 地紋.
				default:		break;
			}
			return	True;
		}
	}
	return	False;
}
// スタンプパターンＯＦＦ.
void PNLC_CommonStampLib::setStampPatternOff( APIC_Job* pc_Job, char c_Pattern )
{
	short ss_MaxUsedArea = getMaxUsableArea( pc_Job );	// 最大使用可能領域.
	for( short i = 1; i <= ss_MaxUsedArea; i++ ) {
		short ss_Language = 0;	short ss_PatternIndex = 0;
		char c_PatternTemp = getStampPattern( pc_Job, i, ss_Language, ss_PatternIndex );
		if( c_Pattern == c_PatternTemp ) {
			deleteStampPattern( i, pc_Job );	// スタンプパターンの削除.
			return;
		}
	}
}
// 前のスタンプターン.
short PNLC_CommonStampLib::beforeStampPattern( short ss_Index, APIC_Job* pc_Job )
{
	if( ss_Index == 1 ) { return -1; }
	if( ss_Index == 2 ) { return  1; }
	ss_Index--;	// １つ前.
	short ss_TmpLanguage; short ss_TmpPatternIndex;
	char c_PatternTime = 0;	char c_PatternSpace = 0;	char c_PatternTemp = getStampPattern( pc_Job, ss_Index, ss_TmpLanguage, ss_TmpPatternIndex  );
	switch( m_e_StampPatternType ) {		// スタンプタイプにより使用IF変更.
		case PNLE_StampPatternType_Watermark:	c_PatternTime = TYPE_WatermarkPatternTime;	c_PatternSpace = TYPE_WatermarkPatternSpace;		break;	// ウォーターマーク.
		case PNLE_StampPatternType_Jimon:		c_PatternTime = TYPE_JimonPatternTime;		c_PatternSpace = TYPE_JimonPatternSpace;			break;	// 地紋.
		default:		break;
	}
	if( c_PatternTemp == c_PatternTime ) {
		ss_Index--;	// １つ前が時刻の場合はもう１つ前の日付まで戻る.
		c_PatternTemp = getStampPattern( pc_Job, ss_Index, ss_TmpLanguage, ss_TmpPatternIndex  );
	} else if( c_PatternTemp == c_PatternSpace ) {
		return	ss_Index;	// １つ前がスペースの場合はそれを選択.
	}
	ss_Index--;
	for( ; ss_Index >= 1; ss_Index-- ) {
		char c_BeforePattern = getStampPattern( pc_Job, ss_Index, ss_TmpLanguage, ss_TmpPatternIndex  );
		if( c_PatternTemp != c_BeforePattern ) {
			return	ss_Index+1;
		}
	}
	return  1;
}
// 次のスタンプパターン.
short PNLC_CommonStampLib::nextStampPattern( short ss_Index, APIC_Job* pc_Job, char& c_NextPattern )
{
	short ss_TmpLanguage; short ss_TmpPatternIndex;
	char c_PatternOff = 0;	char c_PatternSpace = 0;	char c_PatternTemp = getStampPattern( pc_Job, ss_Index, ss_TmpLanguage, ss_TmpPatternIndex  );
	switch( m_e_StampPatternType ) {		// スタンプタイプにより使用IF変更.
		case PNLE_StampPatternType_Watermark:	c_PatternOff = TYPE_WatermarkPatternOff;	c_PatternSpace = TYPE_WatermarkPatternSpace;		break;	// ウォーターマーク.
		case PNLE_StampPatternType_Jimon:		c_PatternOff = TYPE_JimonPatternOff;		c_PatternSpace = TYPE_JimonPatternSpace;			break;	// 地紋.
		default:		break;
	}
	for( short i = ss_Index+1; i <= 8; i++ ) {
		char c_Pattern = getStampPattern( pc_Job, i, ss_TmpLanguage, ss_TmpPatternIndex  );
		if( c_Pattern == c_PatternSpace || c_Pattern == c_PatternOff || c_Pattern != c_PatternTemp ) {
			c_NextPattern = c_Pattern;
			return	i;	// 次がスペース・OFF・パターンが異なる場合それを選択する.
		}
	}
	return  -1;
}
// スタンプパターンの追加.
void PNLC_CommonStampLib::addStampPattern( short ss_Index, APIC_Job* pc_Job, char c_Pattern, short ss_Language, short ss_PatternIndex, short ss_Count )
{
	for( short j = 0; j < ss_Count; j++ ) {
		short i;
		for( i = 8; i >= ss_Index+1; i-- ) {
			// 追加パターンの領域確保 (追加対象のインデックスから追加パターンの使用領域分だけ後ろへシフトさせる).
			short ss_TmpLanguage; short ss_TmpPatternIndex;
			char c_PatternTemp = getStampPattern( pc_Job, i-1, ss_TmpLanguage, ss_TmpPatternIndex  );
			setStampPattern( pc_Job, i, c_PatternTemp, ss_TmpLanguage, ss_TmpPatternIndex  );
		}
		setStampPattern( pc_Job, i, c_Pattern, ss_Language, ss_PatternIndex  );
	}
}
// スタンプパターンの削除.
short PNLC_CommonStampLib::deleteStampPattern( short ss_Index, APIC_Job* pc_Job, Bool bl_DeteleSpace )
{
	short ss_Language = 0; short ss_PatternIndex = 0;
	short ss_TmpLanguage; short ss_TmpPatternIndex;
	char c_PatternOff = 0;	char c_PatternSpace = 0;	char c_PatternData = 0;	char c_PatternTime = 0;	char c_PatternDel = getStampPattern( pc_Job, ss_Index, ss_TmpLanguage, ss_TmpPatternIndex  );
	switch( m_e_StampPatternType ) {		// スタンプタイプにより使用IF変更.
		case PNLE_StampPatternType_Watermark:	c_PatternOff = TYPE_WatermarkPatternOff;	c_PatternSpace = TYPE_WatermarkPatternSpace;	c_PatternData = TYPE_WatermarkPatternDate;	c_PatternTime = TYPE_WatermarkPatternTime;	break;	// ウォーターマーク.
		case PNLE_StampPatternType_Jimon:		c_PatternOff = TYPE_JimonPatternOff;		c_PatternSpace = TYPE_JimonPatternSpace;		c_PatternData = TYPE_JimonPatternDate;		c_PatternTime = TYPE_JimonPatternTime;		break;	// 地紋.
		default:		break;
	}

	// TYPE_WatermarkPattern e_TmpWatermarkPattern = pc_Job->getJobSpec()->pc_Common->c_Watermark.getWatermarkPattern( ss_Index, ss_TmpLanguage, ss_TmpPatternIndex );	// 消去開始時のパターンを覚えておく.
	short ss_Count = 0;
	for(;;) {
		char c_PatternTemp = getStampPattern( pc_Job, ss_Index, ss_TmpLanguage, ss_TmpPatternIndex  );
		// TYPE_WatermarkPattern e_TmpWatermarkPattern2 = pc_Job->getJobSpec()->pc_Common->c_Watermark.getWatermarkPattern( ss_Index, ss_TmpLanguage, ss_TmpPatternIndex );
		if( c_PatternDel != c_PatternTemp ) {
			if( c_PatternDel != c_PatternData || c_PatternTemp != c_PatternTime ) {
				break;	// 異なるパターンとなったら終了.	// 但し、日付を消去する場合は時刻も消去する.
			}
		}
		if( c_PatternDel == c_PatternSpace && ss_Count == 1 ) {
			break;	// スペースの場合は１つのみを消去する.
		}
		for( short i = ss_Index; i < 8; i++ ) {
			char c_Pattern = getStampPattern( pc_Job, i + 1, ss_Language, ss_PatternIndex );
			setStampPattern( pc_Job, i, c_Pattern, ss_Language, ss_PatternIndex );
		}
		setStampPattern( pc_Job, 8, c_PatternOff, ss_Language, ss_PatternIndex );
		ss_Count++;
	}
	if( bl_DeteleSpace == True ) {
		Bool bl_onlySpace = True;	// 残りパターンがスペースのみか判定する.
		for( short i = 1; i <= 8; i++ ) {
			char c_PatternTemp = getStampPattern( pc_Job, i, ss_Language, ss_PatternIndex );
			if( c_PatternTemp != c_PatternOff && c_PatternTemp != c_PatternSpace ) {	bl_onlySpace = False;	}
		}
		if( bl_onlySpace == True ) {	// 残りがスペースだけになったとき、パターンを全てOFF.
			for( short i = 1; i <= 8; i++ ) {
				setStampPattern( pc_Job, i, c_PatternOff, 0, 0 );
			}
		}
		if( getStampPattern( pc_Job, 1, ss_Language, ss_PatternIndex ) == c_PatternOff ) {	// パターンが一つも入っていない時、地紋OFF.
			switch( m_e_StampPatternType ) {		// スタンプタイプにより使用IF変更.
				case PNLE_StampPatternType_Watermark:		pc_Job->getJobSpec()->pc_Common->c_Watermark.setOn( False );		break;	// ウォーターマーク.
				case PNLE_StampPatternType_Jimon:			pc_Job->getJobSpec()->pc_Common->c_JimonPattern.setOn( False );		break;	// 地紋.
				default:		break;
			}
		}
	}
	return	ss_Count;	// 消去数.
}
// スタンプパターンを上へ移動.
short PNLC_CommonStampLib::moveStampPatternUpper( short ss_Index, APIC_Job* pc_Job )
{
	short ss_Before = beforeStampPattern( ss_Index, pc_Job );	// 前のウォーターマークパターン.
	if( ss_Before != -1 ) {
		short ss_Language1; short ss_PatternIndex1;
		char c_Pattern1 = getStampPattern( pc_Job, ss_Index, ss_Language1, ss_PatternIndex1 );	char c_Pattern2 = 0;	char c_PatternData = 0;	char c_PatternTime = 0;
		switch( m_e_StampPatternType ) {		// スタンプタイプにより使用IF変更.
			case PNLE_StampPatternType_Watermark:	c_Pattern2 = TYPE_WatermarkPatternOff;		c_PatternData = TYPE_WatermarkPatternDate;	c_PatternTime = TYPE_WatermarkPatternTime;	break;	// ウォーターマーク.
			case PNLE_StampPatternType_Jimon:		c_Pattern2 = TYPE_JimonPatternOff;			c_PatternData = TYPE_JimonPatternDate;		c_PatternTime = TYPE_JimonPatternTime;		break;	// 地紋.
			default:		break;
		}
// Comment 2016/09/14 [HCLT_Archana] [1002990] 【IT5_3.3】Klocwork Changes ID: 79124, 79128 [UNINIT.STACK.MIGHT] Variable Initialization.
//		short ss_Language2; short ss_PatternIndex2;
		short ss_Language2 = 0; short ss_PatternIndex2 = 0;
		if( ss_Index < 8 ) {
			c_Pattern2 = getStampPattern( pc_Job, ss_Index+1, ss_Language2, ss_PatternIndex2 );
		}
		short ss_Count = deleteStampPattern( ss_Index, pc_Job, False );	// スタンプパターンの削除.
		if( c_Pattern1 == c_PatternData && c_Pattern2 == c_PatternTime ) {
			addStampPattern( ss_Before,   pc_Job, c_Pattern1, ss_Language1, ss_PatternIndex1, 1 );	// スタンプパターンの追加.

			addStampPattern( ss_Before+1, pc_Job, c_Pattern2, ss_Language2, ss_PatternIndex2, 1 );	// スタンプパターンの追加.
		} else {
			addStampPattern( ss_Before, pc_Job, c_Pattern1, ss_Language1, ss_PatternIndex1, ss_Count );	// スタンプパターンの追加.
		}
		return	ss_Before;
	}
	return	-1;
}
// スタンプパターンを下へ移動.
short PNLC_CommonStampLib::moveStampPatternLower( short ss_Index, APIC_Job* pc_Job )
{
	char c_NextPattern = 0;	char c_PatternOff = 0;
	switch( m_e_StampPatternType ) {		// スタンプタイプにより使用IF変更.
		case PNLE_StampPatternType_Watermark:	c_PatternOff = TYPE_WatermarkPatternOff;		break;	// ウォーターマーク.
		case PNLE_StampPatternType_Jimon:		c_PatternOff = TYPE_JimonPatternOff;			break;	// 地紋.
		default:		break;
	}
	short ss_Next = nextStampPattern( ss_Index, pc_Job, c_NextPattern );	// 次のスタンプパターン.
	if( ss_Next != -1 && c_NextPattern != c_PatternOff ) {
		short ss_Language1; short ss_PatternIndex1;
		char c_Pattern1 = getStampPattern( pc_Job, ss_Next, ss_Language1, ss_PatternIndex1 );	char c_Pattern2 = 0;	char c_PatternData = 0;	char c_PatternTime = 0;
		switch( m_e_StampPatternType ) {		// スタンプタイプにより使用IF変更.
			case PNLE_StampPatternType_Watermark:	c_Pattern2 = TYPE_WatermarkPatternOff;		c_PatternData = TYPE_WatermarkPatternDate;	c_PatternTime = TYPE_WatermarkPatternTime;	break;	// ウォーターマーク.
			case PNLE_StampPatternType_Jimon:		c_Pattern2 = TYPE_JimonPatternOff;			c_PatternData = TYPE_JimonPatternDate;		c_PatternTime = TYPE_JimonPatternTime;		break;	// 地紋.
			default:		break;
		}
// Comment 2016/09/14 [HCLT_Archana] [1002990] 【IT5_3.3】Klocwork Changes ID: 79125, 79129 [UNINIT.STACK.MIGHT] Variable Initialization.
//		short ss_Language2; short ss_PatternIndex2;
		short ss_Language2 = 0; short ss_PatternIndex2 = 0;
		if( ss_Next < 8 ) {
			c_Pattern2 = getStampPattern( pc_Job, ss_Next+1, ss_Language2, ss_PatternIndex2 );
		}
		short ss_Count = deleteStampPattern( ss_Next, pc_Job, False );	// スタンプパターンの削除.
		if( c_Pattern1 == c_PatternData && c_Pattern2 == c_PatternTime ) {
			addStampPattern( ss_Index,   pc_Job, c_Pattern1, ss_Language1, ss_PatternIndex1, 1 );	// スタンプパターンの追加.
			addStampPattern( ss_Index+1, pc_Job, c_Pattern2, ss_Language2, ss_PatternIndex2, 1 );	// スタンプパターンの追加.
		} else {
			addStampPattern( ss_Index, pc_Job, c_Pattern1, ss_Language1, ss_PatternIndex1, ss_Count );	// スタンプパターンの追加.
		}
		return	nextStampPattern( ss_Index, pc_Job, c_NextPattern );	// 次のスタンプパターン.
	}
	return	-1;
}
// スタンプインデックスの取得.
short PNLC_CommonStampLib::getStampPatternIndex( APIC_Job* pc_Job, char c_Pattern )
{
	short ss_MaxUsedArea = getMaxUsableArea( pc_Job );	// 最大使用可能領域.
	for( short i = 1; i <= ss_MaxUsedArea; i++ ) {
		short ss_Language = 0;	short ss_PatternIndex = 0;
		char c_PatternTemp = getStampPattern( pc_Job, i, ss_Language, ss_PatternIndex );
		if( c_Pattern == c_PatternTemp ) {
			return	ss_PatternIndex;
		}
	}
	return	0;
}

// スタンプパターン設定.
void PNLC_CommonStampLib::setStampPattern( APIC_Job* pc_Job, short ss_Index, char c_Pattern, short ss_Language, short ss_PatternIndex )
{
	switch( m_e_StampPatternType ) {		// スタンプタイプにより使用IF変更.
		case PNLE_StampPatternType_Watermark:	pc_Job->getJobSpec()->pc_Common->c_Watermark.setWatermarkPattern( ss_Index, static_cast<TYPE_WatermarkPattern>( c_Pattern ), ss_Language, ss_PatternIndex );	break;	// ウォーターマーク.
		case PNLE_StampPatternType_Jimon:		pc_Job->getJobSpec()->pc_Common->c_JimonPattern.setJimonPattern( ss_Index, static_cast<TYPE_JimonPattern>( c_Pattern ), ss_Language, ss_PatternIndex );			break;	// 地紋.
		default:		break;
	}
}

// スタンプパターン取得.
char PNLC_CommonStampLib::getStampPattern( APIC_Job* pc_Job, short ss_Index, short& ss_Language, short& ss_PatternIndex )
{
	char c_Pattern = 0;
	switch( m_e_StampPatternType ) {		// スタンプタイプにより使用IF変更.
		case PNLE_StampPatternType_Watermark:		c_Pattern = static_cast<char>( pc_Job->getJobSpec()->pc_Common->c_Watermark.getWatermarkPattern( ss_Index, ss_Language, ss_PatternIndex ) );		break;	// ウォーターマーク.
		case PNLE_StampPatternType_Jimon:			c_Pattern = static_cast<char>( pc_Job->getJobSpec()->pc_Common->c_JimonPattern.getJimonPattern( ss_Index, ss_Language, ss_PatternIndex ) );			break;	// 地紋.
		default:		break;
	}
	return c_Pattern;
}

// クラスインスタンスの取得.
PNLC_CommonTrayStatusLib* PNLC_CommonTrayStatusLib::newInstance()
{
	static PNLC_CommonTrayStatusLib c_TrayStatusLib;
	return &c_TrayStatusLib;
}
// トレイ状態設定.
void PNLC_CommonTrayStatusLib::setupTrayStatus( TYPE_FeedTray e_FeedTray, TYPE_TemporaryRescueUnit e_TemporaryRescueTray, PFormSettingButton* pButton )
{
	TYPC_PaperType c_PaperType = PnlComGetFeedTray()->getPaperType( e_FeedTray );
	pButton->setCasetSize( PnlComGetFeedTray()->getPaperSize(e_FeedTray), c_PaperType.getPaperType(), c_PaperType.getFeed2ndSidedPaper(), PnlComGetFeedTray()->getPaperStatus(e_FeedTray), e_FeedTray, getRemainPaper(e_FeedTray));	// トレイ情報セット.
	Bool b_KeyDispOn = True;	// トレイ有り、無し判定.
	if( e_FeedTray == TYPE_FeedTrayMax	// トレイ無し.
	 || TYPE_TrayTypeNon == PnlComGetFeedTray()->getTrayType(e_FeedTray)		// トレイ無し.
	 || PnlComGetSystemStatus()->getTemporaryRescueUserStatus(e_TemporaryRescueTray) == TYPE_TemporaryRescueStatusSet ) {	// トレイ切り離し.
		b_KeyDispOn = False;
	}
	pButton->showEnable(b_KeyDispOn);
}

// メールアドレスにPrefixSuffixを結合する.
void combinePrefixSuffix( char* sc_MailAddress, TYPS_PreFix& s_PreFix )
{
	char sc_TextBuff[PCGD_MaxKeybordInfoBuffer] = {0};		// 一時テキスト格納用バッファ.	// Prefix/Suffixからの戻りの場合は結合する.
	strncpy( sc_TextBuff, s_PreFix.sc_Prefix, sizeof( sc_TextBuff ) - 1 );
	short ss_PrefixTotalSize = strlen(sc_MailAddress) + strlen(s_PreFix.sc_Prefix);	// Prefixの文字の数と本画面で設定されている文字の数を足す.
	if( PCGD_SCAN_EMAILADDRESS_MAX < ss_PrefixTotalSize ) {	// Prefixを結合すると文字列最大サイズを超えてしまう時.
		strncpy( ( sc_TextBuff + strlen(s_PreFix.sc_Prefix) ), sc_MailAddress, ( PCGD_SCAN_EMAILADDRESS_MAX - strlen(s_PreFix.sc_Prefix) ) );	// Prefixを結合すると文字列最大サイズを超えてしまう場合は切り詰める.
		sc_TextBuff[PCGD_SCAN_EMAILADDRESS_MAX] = '\0';	// 終端コードを配列の最後に入れる.
	} else {	// Prefixを結合すると文字列最大サイズを超えない時.
// Comment 2016/09/14 [HCLT_Archana] [1002990] 【IT5_3.3】Klocwork Changes ID: 79126 [SV.STRBO.UNBOUND_COPY] strncat is used.
//		strcat( sc_TextBuff, sc_MailAddress );	// 文字列最大サイズを超えない場合には単純に結合する.
		strncat( sc_TextBuff, sc_MailAddress, sizeof( sc_TextBuff ) - strlen(sc_TextBuff)- 1 );	// 文字列最大サイズを超えない場合には単純に結合する.
		short ss_SuffixTotalSize = strlen(sc_TextBuff) + strlen(s_PreFix.sc_Suffix);	// Prefixと本画面で設定されている文字を結合した配列の文字数を算出する.
		if( PCGD_SCAN_EMAILADDRESS_MAX < ss_SuffixTotalSize ) {	// Suffixを結合すると文字列最大サイズを超えてしまう時.
			strncpy( ( sc_TextBuff + strlen(sc_TextBuff) ), s_PreFix.sc_Suffix, ( PCGD_SCAN_EMAILADDRESS_MAX - strlen(sc_TextBuff) ) );	// Suffixを結合すると文字列最大サイズを超えてしまう場合は切り詰める.
			sc_TextBuff[PCGD_SCAN_EMAILADDRESS_MAX] = '\0';	// 終端コードを配列の最後に入れる.
		} else {	// Suffixを結合すると文字列最大サイズを超えない時.
// Comment 2016/09/14 [HCLT_Archana] [1002990] 【IT5_3.3】Klocwork Changes ID: 79126 [SV.STRBO.UNBOUND_COPY] strncat is used.
//			strcat( sc_TextBuff, s_PreFix.sc_Suffix );	// 文字列最大サイズを超えない場合には単純に結合する.
			strncat( sc_TextBuff, s_PreFix.sc_Suffix, sizeof( sc_TextBuff ) - strlen(sc_TextBuff)- 1 );	// 文字列最大サイズを超えない場合には単純に結合する.
		}
	}
	strcpy( sc_MailAddress, sc_TextBuff );
}

// TextField に表示する文字列を整形する.
char* SetupInputString(
	short ss_lines,				// 表示する行数.
	short ss_length,			// 1行の文字数.
	char* sc_String,			// 元の文字列.
	short ss_offsetValue,		// 取り出す文字列の開始点.
	EP_CharSet ep_CharSet,		// フォントセット.
	Bool bDependsOnCharSet )	// True:キャラセットに依存する False:UTF-8文字列を扱う.
{
	static char sc_Buffer[1000];	// 大きめを確保する.
	setupInputString( ss_lines, ss_length, sc_String, sc_Buffer, ss_offsetValue, ep_CharSet, bDependsOnCharSet );
	return	sc_Buffer;
}
// オーバーレイ登録日表示関数.
void PnlComSetOverLayRegistDayAdd( PActivatableAdd* p_Btx_OverLayRegistDay, uchar uc_Index )
{
	char sc_RegistrationDay[JBLD_DAYDISP_MAX + 2] = {0};	// 登録日表示用変数.
	TYPC_Overlay c_Overlay;									// オーバーレイ情報取得.
	PnlComGetOperationSetting()->getOverlay( uc_Index, &c_Overlay, True );
	APIC_Doc* pc_Doc = cp_OneDoc;	// 画像登録日の取得.
	APIC_DocInfo pc_DocInfo;
	pc_Doc->setDocId( c_Overlay.sl_DocID );
// Commment 2017/09/22 HCL_Venkataramesh [IT5_4.2][RQ-686][WP689] setBoxId support for Network2 user.
//	pc_Doc->setBoxId( DOCD_OverLayBox );
	PNLC_NetworkLan::getInstance()->setBoxIdPNL( pc_Doc, DOCD_OverLayBox );
	int32_t i32_ret = APID_DocapiMethodOk;
	i32_ret = pc_Doc->getDocInfo( APID_DocapiNull, NULL, &pc_DocInfo, LIBE_CharSetDefault );
	if( APID_DocapiMethodOk == i32_ret ) {
		TYPC_Time c_Time = pc_DocInfo.t_DocTime;
		if( c_Time.ss_Year >= 1990 && c_Time.ss_Year <= 2089 && c_Time.sc_Month >= 1 && c_Time.sc_Month <= 12 && c_Time.sc_Day >= 1 && c_Time.sc_Day <= 31 ) {
			switch( PnlComGetMachineSetting()->getSystemType() ) {		// 仕向けにより表示文字列を変更する.
				case TYPE_SystemType_US: snprintf( sc_RegistrationDay, sizeof( sc_RegistrationDay ), "%02d/%02d/%04d", c_Time.sc_Month, c_Time.sc_Day, c_Time.ss_Year );	break;	// ＵＳ仕向け.
				case TYPE_SystemType_JP: snprintf( sc_RegistrationDay, sizeof( sc_RegistrationDay ),"%04d/%02d/%02d", c_Time.ss_Year, c_Time.sc_Month, c_Time.sc_Day );		break;	// ＪＰ仕向け.
				case TYPE_SystemType_EU: snprintf( sc_RegistrationDay, sizeof( sc_RegistrationDay ),"%02d/%02d/%04d", c_Time.sc_Day, c_Time.sc_Month, c_Time.ss_Year );		break;	// ＥＵ仕向け.
			}
		}
	}
	p_Btx_OverLayRegistDay->changeTextString( reinterpret_cast<uchar*>( sc_RegistrationDay ) );
}
// オーバレイ共通関数
void exeOvelayCallBack_FromStatus( PNLD_OverlayOverWriteStatus e_Status )
{

	TYPE_PNL_CallBack e_Option = 0;

	switch( e_Status ) {
		case PNLD_OverlayOverWriteStatus_NewWait:
			e_Option = TYPE_PNL_CallBack_OverlayOverWriteStatus_NewWait;	break;

		case PNLD_OverlayOverWriteStatus_NewExe:
			e_Option = TYPE_PNL_CallBack_OverlayOverWriteStatus_NewExe;		break;

		case PNLD_OverlayOverWriteStatus_OverWait:
			e_Option = TYPE_PNL_CallBack_OverlayOverWriteStatus_OverWait;	break;

		case PNLD_OverlayOverWriteStatus_OverExe:
			e_Option = TYPE_PNL_CallBack_OverlayOverWriteStatus_OverExe;	break;

		case PNLD_OverlayOverWriteStatus_NewEnd:
		case PNLD_OverlayOverWriteStatus_OverEnd:
		case PNLD_OverlayOverWriteStatus_OverCancel:	// 伊丹Sky(藤田健司)　2011/07/20 CQ351506
		case PNLD_OverlayOverWriteStatus_DelEnd:
			e_Option = TYPE_PNL_CallBack_OverlayOverWriteStatus_End;		break;
	}

	if( 0 != e_Option ) {
		CallBackIndi( e_Option );
	}
}
// 濃度設定.
Bool setDensity( APIC_Job* pc_Job, short n, EP_Combination  e_Combination )
{
// Comment 2016/08/30 [HCLT_Premkumar] [1002990] 【IT5_3.3】Klocwork Changes ID: 45966[NPD.FUNC.CALL.MUST] Adding NULL check for pc_Job.
//	if( combinationCheck( e_Combination, pc_Job ) == False ) {	// 組み合わせ禁止チェック.
//		return	False;
//	}
//	TYPC_OneImageAdjust c_OneImageAdjust = pc_Job->getJobSpec()->pc_Common->c_ImageAdjust.getDensity();
	if ( NULL == pc_Job ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if ( NULL == pc_JobSpec ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	TYPC_Common* pc_Common = pc_JobSpec->pc_Common;
	if ( NULL == pc_Common ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	if ( combinationCheck( e_Combination, pc_Job ) == False ) {	// 組み合わせ禁止チェック.
		return	False;
	}
	TYPC_OneImageAdjust c_OneImageAdjust = pc_Common->c_ImageAdjust.getDensity();
	switch( n ) {
		case 0:	c_OneImageAdjust.setImageAdjust( TYPE_ImageAdjustM09 );	break;		// 「うすい」.
		case 1:	c_OneImageAdjust.setImageAdjust( TYPE_ImageAdjustM06 );	break;
		case 2:	c_OneImageAdjust.setImageAdjust( TYPE_ImageAdjustM04 );	break;
		case 3:	c_OneImageAdjust.setImageAdjust( TYPE_ImageAdjustM02 );	break;
		case 4:	c_OneImageAdjust.setImageAdjust( TYPE_ImageAdjustStd );	break;
		case 5:	c_OneImageAdjust.setImageAdjust( TYPE_ImageAdjustP02 );	break;
		case 6:	c_OneImageAdjust.setImageAdjust( TYPE_ImageAdjustP04 );	break;
		case 7:	c_OneImageAdjust.setImageAdjust( TYPE_ImageAdjustP06 );	break;
		case 8:	c_OneImageAdjust.setImageAdjust( TYPE_ImageAdjustP09 );	break;		// 「こい」.
	}
// Comment 2016/08/30 [HCLT_Premkumar] [1002990] 【IT5_3.3】Klocwork Changes ID: 45966[NPD.FUNC.CALL.MUST] Adding NULL check for pc_Job.
//	pc_Job->getJobSpec()->pc_Common->c_ImageAdjust.setDensity(c_OneImageAdjust);
	pc_Common->c_ImageAdjust.setDensity(c_OneImageAdjust);
	return	True;
}
// 濃度取得.
short getDensity( APIC_Job* pc_Job )
{
// Comment 2016/09/15 [HCLT_Sri charan] [1002990] 【IT5_3.3】Klocwork Changes ID: 68330[NPD.FUNC.MUST] Added Null check.
//	switch( pc_Job->getJobSpec()->pc_Common->c_ImageAdjust.getDensity().getImageAdjust() ) {
	if ( NULL == pc_Job ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return	-1;
	}
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if ( NULL == pc_JobSpec ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return	-1;
	}
	TYPC_Common* pc_Common = pc_JobSpec->pc_Common;
	if ( NULL == pc_Common ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return	-1;
	}
	switch ( pc_Common->c_ImageAdjust.getDensity().getImageAdjust() ) {
		case TYPE_ImageAdjustM09:	return 0;	// 「うすい」.
		case TYPE_ImageAdjustM06:	return 1;
		case TYPE_ImageAdjustM04:	return 2;
		case TYPE_ImageAdjustM02:	return 3;
		case TYPE_ImageAdjustStd:	return 4;
		case TYPE_ImageAdjustP02:	return 5;
		case TYPE_ImageAdjustP04:	return 6;
		case TYPE_ImageAdjustP06:	return 7;
		case TYPE_ImageAdjustP09:	return 8;	// 「こい」.
	}
	return	-1;
}
// 下地調整レベル設定.
Bool setBackDensity( APIC_Job* pc_Job, short n )
{
// Comment 2016/08/30 [HCLT_Premkumar] [1002990] 【IT5_3.3】Klocwork Changes ID: 45968[NPD.FUNC.CALL.MUST] Adding NULL check for pc_Job.
//	if( 9 == n ) {			// 自動キーの場合.
//		if( combinationCheck( CNK_BackDensityAuto, pc_Job ) == False ) { return False; }
//		pc_Job->getJobSpec()->pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityUnselect );
//		pc_Job->getJobSpec()->pc_Common->c_ImageAdjust.setAutoExpo( True );
//	} else {
//		if( combinationCheck( CNK_BackDensityManu, pc_Job ) == False ) { return False; }
//		pc_Job->getJobSpec()->pc_Common->c_ImageAdjust.setAutoExpo( False );
//		switch( n ) {
//			case 0:	pc_Job->getJobSpec()->pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityLevelL6 );	break;	// 「-6」.
//			case 1:	pc_Job->getJobSpec()->pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityLevelL5 );	break;
//			case 2:	pc_Job->getJobSpec()->pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityLevelL4 );	break;
//			case 3:	pc_Job->getJobSpec()->pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityLevelL3 );	break;
//			case 4:	pc_Job->getJobSpec()->pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityLevelL2 );	break;
//			case 5:	pc_Job->getJobSpec()->pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityLevelL1 );	break;
//			case 6:	pc_Job->getJobSpec()->pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityLevelN  );	break;	// 「 0」.
//			case 7:	pc_Job->getJobSpec()->pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityLevelD1 );	break;
//			case 8:	pc_Job->getJobSpec()->pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityLevelD2 );	break;	// 「+2」.
//		}
//	}
	if ( NULL == pc_Job ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	TYPC_JobSpec* pc_JobSpec = pc_Job->getJobSpec();
	if ( NULL == pc_JobSpec ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	TYPC_Common* pc_Common = pc_JobSpec->pc_Common;
	if ( NULL == pc_Common ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return False;
	}
	if ( 9 == n ) {			// 自動キーの場合.
		if ( combinationCheck( CNK_BackDensityAuto, pc_Job ) == False ) {
			return False;
		}
		pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityUnselect );
		pc_Common->c_ImageAdjust.setAutoExpo( True );
	} else {
		if ( combinationCheck( CNK_BackDensityManu, pc_Job ) == False ) {
			return False;
		}
		pc_Common->c_ImageAdjust.setAutoExpo( False );
		switch ( n ) {
			case 0:	pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityLevelL6 );	break;	// 「-6」.
			case 1:	pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityLevelL5 );	break;
			case 2:	pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityLevelL4 );	break;
			case 3:	pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityLevelL3 );	break;
			case 4:	pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityLevelL2 );	break;
			case 5:	pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityLevelL1 );	break;
			case 6:	pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityLevelN  );	break;	// 「 0」.
			case 7:	pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityLevelD1 );	break;
			case 8:	pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityLevelD2 );	break;	// 「+2」.
		}
	}
	return	True;
}
// 下地調整レベル取得.
short getBackDensity( APIC_Job* pc_Job )
{
	if( True == ( pc_Job->getJobSpec()->pc_Common->c_ImageAdjust.getAutoExpo() ) ) { return 9; }	// 自動.
	switch( pc_Job->getJobSpec()->pc_Common->c_ImageAdjust.getBackDensity() ) {
		case TYPE_BackDensityLevelL6:	return 0;	// 「-6」.
		case TYPE_BackDensityLevelL5:	return 1;
		case TYPE_BackDensityLevelL4:	return 2;
		case TYPE_BackDensityLevelL3:	return 3;
		case TYPE_BackDensityLevelL2:	return 4;
		case TYPE_BackDensityLevelL1:	return 5;
		case TYPE_BackDensityLevelN:	return 6;	// 「 0」.
		case TYPE_BackDensityLevelD1:	return 7;
		case TYPE_BackDensityLevelD2:	return 8;	// 「+2」.
	}
	return	-1;
}
// 画質調整 : レベル表示.
void displayIconThisMode( TYPE_ImageAdjust _ec_ImageAdjust, PSinglePartsResizableGraphic* epGraph )
{
	PNLS_Parts* ps_parts;
	switch( _ec_ImageAdjust ) {
		case TYPE_ImageAdjustP09:	// 画質調整レベル・微調＋９・祖調＋３
		case TYPE_ImageAdjustP08:
			ps_parts = tbl_i00_703_000_X112_Y16_s7;		// 画質調整・調整レベル表示７
			break;
		case TYPE_ImageAdjustP07:	// 画質調整レベル・微調＋７
		case TYPE_ImageAdjustP06:	// 画質調整レベル・微調＋６・祖調＋２
		case TYPE_ImageAdjustP05:
			ps_parts = tbl_i00_703_000_X112_Y16_s6;		// 画質調整・調整レベル表示６
			break;
		case TYPE_ImageAdjustP04:	// 画質調整レベル・微調＋４
		case TYPE_ImageAdjustP03:	// 画質調整レベル・微調＋３・祖調＋１
		case TYPE_ImageAdjustP02:
			ps_parts = tbl_i00_703_000_X112_Y16_s5;		// 画質調整・調整レベル表示５
			break;
		case TYPE_ImageAdjustP01:	// 画質調整レベル・微調＋１
		case TYPE_ImageAdjustStd:	// 画質調整レベル・標準
		case TYPE_ImageAdjustM01:
			ps_parts = tbl_i00_703_000_X112_Y16_s4;		// 画質調整・調整レベル表示４
			break;
		case TYPE_ImageAdjustM02:	// 画質調整レベル・微調－２
		case TYPE_ImageAdjustM03:	// 画質調整レベル・微調－３・祖調－１
		case TYPE_ImageAdjustM04:
			ps_parts = tbl_i00_703_000_X112_Y16_s3;		// 画質調整・調整レベル表示３
			break;
		case TYPE_ImageAdjustM05:	// 画質調整レベル・微調－５
		case TYPE_ImageAdjustM06:	// 画質調整レベル・微調－６・祖調－２
		case TYPE_ImageAdjustM07:
			ps_parts = tbl_i00_703_000_X112_Y16_s2;		// 画質調整・調整レベル表示２
			break;
		case TYPE_ImageAdjustM08:	// 画質調整レベル・微調－８
		case TYPE_ImageAdjustM09:
			ps_parts = tbl_i00_703_000_X112_Y16_s1;		// 画質調整・調整レベル表示１
			break;
		default:	return;
	}
	epGraph->changeParts( ps_parts );
}
struct PCGS_ImageAdjustCopyDensity_PD {
	TYPE_ImageAdjust e_ImageAdjust;
	uchar			 uc_Index;
} s_AryImageAdjustCopyDensity_PD[] = {
	{ TYPE_ImageAdjustP09, 0 },
	{ TYPE_ImageAdjustP08, 0 },
	{ TYPE_ImageAdjustP07, 0 },
	{ TYPE_ImageAdjustP06, 1 },
	{ TYPE_ImageAdjustP05, 1 },
	{ TYPE_ImageAdjustP04, 2 },
	{ TYPE_ImageAdjustP03, 2 },
	{ TYPE_ImageAdjustP02, 3 },
	{ TYPE_ImageAdjustP01, 3 },
	{ TYPE_ImageAdjustStd, 4 },
	{ TYPE_ImageAdjustM01, 5 },
	{ TYPE_ImageAdjustM02, 5 },
	{ TYPE_ImageAdjustM03, 6 },
	{ TYPE_ImageAdjustM04, 6 },
	{ TYPE_ImageAdjustM05, 7 },
	{ TYPE_ImageAdjustM06, 7 },
	{ TYPE_ImageAdjustM07, 8 },
	{ TYPE_ImageAdjustM08, 8 },
	{ TYPE_ImageAdjustM09, 8 },
};
const uchar uc_SizeOfAryImageAdjustCopyDensity_PD = sizeof( s_AryImageAdjustCopyDensity_PD ) / sizeof( PCGS_ImageAdjustCopyDensity_PD );
static const TYPE_ImageAdjust e_AryImageAdjustLevelEx[] = {
	TYPE_ImageAdjustM09,	TYPE_ImageAdjustM08,	TYPE_ImageAdjustM07,
	TYPE_ImageAdjustM06,	TYPE_ImageAdjustM05,	TYPE_ImageAdjustM04,
	TYPE_ImageAdjustM03,	TYPE_ImageAdjustM02,	TYPE_ImageAdjustM01,
	TYPE_ImageAdjustStd,
	TYPE_ImageAdjustP01,	TYPE_ImageAdjustP02,	TYPE_ImageAdjustP03,
	TYPE_ImageAdjustP04,	TYPE_ImageAdjustP05,	TYPE_ImageAdjustP06,
	TYPE_ImageAdjustP07,	TYPE_ImageAdjustP08,	TYPE_ImageAdjustP09
};
static const short	ss_ImageAdjustLevelSize = static_cast<short>( sizeof( e_AryImageAdjustLevelEx ) / sizeof( TYPE_ImageAdjust ) );
static const TYPE_ImageAdjust e_AryImageAdjustLevelEx_Sharpness[] = {
	TYPE_ImageAdjustM09,	TYPE_ImageAdjustM06,	TYPE_ImageAdjustM03,
	TYPE_ImageAdjustStd,
	TYPE_ImageAdjustP03,	TYPE_ImageAdjustP06,	TYPE_ImageAdjustP09
};
static const short	ss_ImageAdjustLevelSize_Sharpness = static_cast<short>( sizeof( e_AryImageAdjustLevelEx_Sharpness ) / sizeof( TYPE_ImageAdjust ) );

// サンプルコピー用紙サイズ.
static const TYPE_SizeCode e_AryMonitorSizeCode_PD[] =
{
#ifndef DEF_A4_Cus		// Comment VenusMLK -> 2016/02/25 OPS2 Sky高田(庸) [CQ525381]サンプルコピー可能な用紙サイズ変更.
	TYPE_SizeCodeA4Lef,			// A4Y.
	TYPE_SizeCodeLetterLef,		// LetterY.
	TYPE_SizeCodeA3Sef,			// A3T.
	TYPE_SizeCodeLedgerSef		// 11x17T.
#else 					// Comment VenusMLK -- 2016/02/25 OPS2 Sky高田(庸) [CQ525381]サンプルコピー可能な用紙サイズ変更.
	TYPE_SizeCodeA4Sef,			// A4T.
	TYPE_SizeCodeLetterSef,		// LetterT.
#endif // DEF_A4_Cus	// Comment VenusMLK <- 2016/02/25 OPS2 Sky高田(庸) [CQ525381]サンプルコピー可能な用紙サイズ変更.

};
const uchar uc_SizeMonitorSizeCode_PD = static_cast<uchar>( sizeof( e_AryMonitorSizeCode_PD ) / sizeof( TYPE_SizeCode ) );
// サンプルコピー用紙種類.
static const TYPE_PaperType e_AryMonitorPaperType_PD[] = {
	TYPE_PaperTypeNormal,		// 普通紙.
	TYPE_PaperTypeNotDuplex1,	// 両面不可紙.
};
const uchar uc_SizeMonitorPaperType_PD = static_cast<uchar>( sizeof( e_AryMonitorPaperType_PD ) / sizeof( TYPE_PaperType ) );

// 濃いレベルを取得.
TYPE_ImageAdjust PnlComGetLevelGaugeDeep( TYPE_ImageAdjust e_ImageAdjustNow, uchar uc_Offset )
{
	//	処理過程で値がマイナス値をとりうるので、「uchar」ではなく「char」とする.
	char sc_Counter = 0;
	for( ; uc_SizeOfAryImageAdjustCopyDensity_PD > sc_Counter ; ++sc_Counter ) {
		if( e_ImageAdjustNow == s_AryImageAdjustCopyDensity_PD[static_cast<uchar>( sc_Counter )].e_ImageAdjust ) {
			break;
		}
	}
#ifdef LINUX
	sc_Counter = std::max( sc_Counter - uc_Offset, 0 );
#else
	sc_Counter = max( sc_Counter - uc_Offset, 0 );
#endif
	return s_AryImageAdjustCopyDensity_PD[static_cast<uchar>( sc_Counter )].e_ImageAdjust;
}
// 薄いレベルを取得.
TYPE_ImageAdjust PnlComGetLevelGaugeLight( TYPE_ImageAdjust e_ImageAdjustNow, uchar uc_Offset )
{
	uchar uc_Counter = 0;
	for( ; uc_SizeOfAryImageAdjustCopyDensity_PD > uc_Counter ; ++uc_Counter ) {
		if( e_ImageAdjustNow == s_AryImageAdjustCopyDensity_PD[uc_Counter].e_ImageAdjust ) {
			break;
		}
	}
	//	元々Enumの最後の値が設定されていた場合を考慮し、最後の有効値と比較.
#ifdef LINUX
	uc_Counter = std::min( uc_Counter + uc_Offset, uc_SizeOfAryImageAdjustCopyDensity_PD - 1 );
#else
	uc_Counter = min( uc_Counter + uc_Offset, uc_SizeOfAryImageAdjustCopyDensity_PD - 1 );
#endif /*LINUX*/
	return s_AryImageAdjustCopyDensity_PD[uc_Counter].e_ImageAdjust;
}
// 数値(座標)への変換.
short convertLevelValue( const TYPE_ImageAdjust e_ImageAdjust )
{
	short ss_Result = 0;
	for( short ss_Cnt = 0 ; ss_ImageAdjustLevelSize > ss_Cnt ; ++ss_Cnt ) {
		if( e_AryImageAdjustLevelEx[ ss_Cnt ] == e_ImageAdjust ) {
			ss_Result = ss_Cnt;
			break;
		}
	}
	return ss_Result;
}
// 数値(座標)への変換(シャープネス用).
short convertLevelValue_Sharpness( const TYPE_ImageAdjust e_ImageAdjust )
{
	short ss_Result = 0;
	for( short ss_Cnt = 0 ; ss_ImageAdjustLevelSize_Sharpness > ss_Cnt ; ++ss_Cnt ) {
		if( e_AryImageAdjustLevelEx_Sharpness[ ss_Cnt ] == e_ImageAdjust ) {
			ss_Result = ss_Cnt;
			break;
		}
	}
	return ss_Result;
}
// サンプルコピー.
void prepareSampleCopy_PD()
{
	if( PnlComGetJobCopy()->getJobSpec()->pc_Common->c_MonoColor.getMonoColorOn() ) {
		PnlComGetJobCopy()->getJobSpec()->pc_Common->c_MonoColor.setMonoColorOn( False );
		PnlComGetJobCopy()->getJobSpec()->pc_Common->setColorMode( TYPE_ColorModeFullColor );
	} else if( PnlComGetJobCopy()->getJobSpec()->pc_Common->c_2ColorMode.get2ColorOn() ) {
		PnlComGetJobCopy()->getJobSpec()->pc_Common->c_2ColorMode.set2ColorOn( False );
		PnlComGetJobCopy()->getJobSpec()->pc_Common->setColorMode( TYPE_ColorModeFullColor );
	}
	const Bool b_AutoExpoOn = PnlComGetJobCopy()->getJobSpec()->pc_Common->c_ImageAdjust.getAutoExpo();// 下地除去の自動設定状態とレベルを保存.
	combinationCheck( CNK_SampleCopy );// 先設定モードを解除するための組み合わせ禁止処理.
	TYPE_FeedTray e_FeedTray = TYPE_FeedTray1;
	if( True == enableImageMonitor_PD( &e_FeedTray ) ) {
		onlyPanel.setFeedMode( PnlComGetJobCopy(), e_FeedTray, False );// 給紙口の設定.
	}
	// 倍率の設定.
	TYPC_Point<float> c_MagTemplate( PCGD_MagTypeFull, PCGD_MagTypeFull );
	PnlComGetJobCopy()->getJobSpec()->pc_Common->c_Mag.setMagType( TYPE_MagTypeFull );
	PnlComGetJobCopy()->getJobSpec()->pc_Common->c_Mag.setMag( c_MagTemplate );
	// 片面→片面の設定.
	PnlComGetJobCopy()->getJobSpec()->pc_Scan->setDuplex( False );
	PnlComGetJobCopy()->getJobSpec()->pc_Print->setDuplexMode( TYPE_DuplexModeOff );
	// 小冊子をOff.
	PnlComGetJobCopy()->getJobSpec()->pc_Print->c_Nin1.setBookletOn( False );
	PnlComGetJobCopy()->getJobSpec()->pc_Print->c_Nin1.setNin1( TYPE_Nin1_1in1 );
	PnlComGetJobCopy()->getJobSpec()->pc_Print->c_Finishing.setStapleMode( TYPE_StapleModeNone );
	PnlComGetJobCopy()->getJobSpec()->pc_Print->c_Finishing.setStaplePosition( TYPE_BindDirectionAuto );
	if( b_AutoExpoOn ) { // 保存した下地除去の自動設定状態から設定する下地除去レベルを判定.
		PnlComGetJobCopy()->getJobSpec()->pc_Common->c_ImageAdjust.setBackDensity( TYPE_BackDensityLevelN );
	}
	TYPC_DirectBoxAddress* pc_DirectBoxAddress = comPanelJob->getJobSpec()->pc_Send->c_ComSend.c_DirectAddressBook.pageBoxAddress( 1 );
	if( pc_DirectBoxAddress->getOn() ) {
		cancel( CNK_CopyToBox, CNK_Size, PnlComGetJobCopy() );
	}
	if( isOn( CNK_ProgramJob, PnlComGetJobCopy() ) == True ) {
		cancel( CNK_ProgramJob, CNK_Size, PnlComGetJobCopy() );// プログラムジョブの取消し.
	}
	PnlComGetJobCopy()->getJobSpec()->pc_Print->setNumOfCopies( 1 );// 置数を1に設定.
}
Bool enableImageMonitor_PD( TYPE_FeedTray* pe_FeedTray )
{
	TYPS_TraySearchPriority s_TrayPriority = PnlComGetCopyJobSetting()->getTraySearchPriority();// 給紙トレイの優先順位指定の取得.
	for( uchar uc_Type = 0; uc_SizeMonitorPaperType_PD > uc_Type; ++uc_Type ) { // 用紙種類.
		for( uchar uc_Size = 0; uc_SizeMonitorSizeCode_PD > uc_Size; ++uc_Size ) {// 用紙サイズ.
			for( uchar uc_Tray = 0; 6 > uc_Tray; ++uc_Tray ) {// 給紙口.
#ifdef DEF_A4_Cus		// Comment VenusMLK -> 2016/03/08 OPS2 Sky高田(庸) [CQ525628]サンプルコピー画面のトレイ検索が遅い問題の修正.
				if ((TYPE_FeedTrayUnselect != s_TrayPriority.e_FeedTray[uc_Tray]) && (TYPE_TrayStatusNon != PnlComGetFeedTray()->getTrayStatus(s_TrayPriority.e_FeedTray[uc_Tray]))) {
#endif // DEF_A4_Cus	// Comment VenusMLK <- 2016/03/08 OPS2 Sky高田(庸) [CQ525628]サンプルコピー画面のトレイ検索が遅い問題の修正.
				if( TYPE_PaperStatusEmpty != PnlComGetFeedTray()->getPaperStatus( s_TrayPriority.e_FeedTray[uc_Tray] ) ) {// 対象トレイの用紙の有無を判定.
					TYPC_PaperSize c_PaperSize = PnlComGetFeedTray()->getPaperSize( s_TrayPriority.e_FeedTray[uc_Tray] );// 用紙サイズ・用紙種類の取得.
					TYPC_PaperType c_PaperType = PnlComGetFeedTray()->getPaperType( s_TrayPriority.e_FeedTray[uc_Tray] );
					const TYPE_SizeCode e_SizeCode = c_PaperSize.e_SizeCode;
					const TYPE_PaperType e_PaperType = c_PaperType.getPaperType();
					if( ( e_SizeCode == e_AryMonitorSizeCode_PD[uc_Size] ) && ( c_PaperSize.b_WideOn == False )
					    && ( e_PaperType == e_AryMonitorPaperType_PD[uc_Type] ) ) { // サイズ(ワイド紙は優先しない)・種類の適合した用紙が存在する場合.
						*pe_FeedTray = s_TrayPriority.e_FeedTray[uc_Tray];
						return True;
					}
				}
#ifdef DEF_A4_Cus		// Comment VenusMLK -> 2016/03/08 OPS2 Sky高田(庸) [CQ525628]サンプルコピー画面のトレイ検索が遅い問題の修正.
				}
#endif // DEF_A4_Cus	// Comment VenusMLK <- 2016/03/08 OPS2 Sky高田(庸) [CQ525628]サンプルコピー画面のトレイ検索が遅い問題の修正.
			}
		}
	}
	*pe_FeedTray = TYPE_FeedTrayMax; // サンプルコピー用の用紙が存在しない.
	return False;
}
//	カラー	：同時プリント機能リセット.
void ResetSendWithPrint()
{
	PCGC_JobSpecScanFaxBackup* pc_JobBack = PnlComGetJobSpecScanFaxBack();
	if( NULL == pc_JobBack ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	APIC_Job* pc_JobScanFax = PnlComGetJobScanFax();
	if( NULL == pc_JobScanFax ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_JobSpec* pc_JobSpec = pc_JobScanFax->getJobSpec();
	if( NULL == pc_JobSpec ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_Print* pc_Print = pc_JobSpec->pc_Print;
	if( NULL == pc_Print ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	pc_JobBack->b_MmbSendWithPrint = False;
	pc_Print->setNumOfCopies( 1 );							// 部数1をセット.
	pc_Print->setDuplexMode( TYPE_DuplexModeOff );			// 片面をセットする.
	combinationCheck( CNK_APS );							// APSキーと組み合わせ禁止チェック.
	pc_Print->c_PaperTray.setFeedTrayEnable( False );		// APSセット.
	onlyPanel.setFeedMode( pc_JobScanFax, TYPE_FeedTrayMax, False );
	cancel( CNK_Staple, CNK_Size, pc_JobScanFax );			// ステイプル解除.
	pc_Print->c_SortMode.setSortMode( TYPE_SortModeGroup );	// 同時印刷をキャンセルする時なので念のためdefaultであるグループモードを設定.
}
//	カラー カラー設定用.
void changePageSetting()
{
	const Bool b_Hdd = ( ( TYPE_HddStatusFormated == APIC_HddInfo::newInstance()->getHddStatus() ) ? True : False ) ;
	APIC_Job* pc_JobScanFax = PnlComGetJobScanFax();
	if( NULL == pc_JobScanFax ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_JobSpec* pc_JobSpec = pc_JobScanFax->getJobSpec();
	if( NULL == pc_JobSpec ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	TYPC_Send* pc_Send = pc_JobSpec->pc_Send;
	if( NULL == pc_Send ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	if( ( TYPE_FileFormatPdf == pc_Send->c_ComSend.getFileFormat() ) && ( False == b_Hdd ) ) {	// ファイル形式・ページ設定がPDF選択・HDD無しの場合.// ページ設定値の設定.
		pc_Send->c_ComSend.setPageSetting( TYPE_PageSettingSpecifiedPage );						// ページ分割.
		pc_Send->c_ComSend.setSpecifiedPage( 1 );												// ページ分割数1固定.
	}
}
// 地紋背景パターン.
static char tbl_JimonBackgroundPatterns[] = {
	TYPE_JimonBackgroundPattern1,	// 地紋背景パターン：唐草の時.
	TYPE_JimonBackgroundPattern2,	// 地紋背景パターン：網代の時.
	TYPE_JimonBackgroundPattern3,	// 地紋背景パターン：麻の葉の時.
	TYPE_JimonBackgroundPattern4,	// 地紋背景パターン：亀甲花菱の時.
	TYPE_JimonBackgroundPattern5,	// 地紋背景パターン：七宝の時.
	TYPE_JimonBackgroundPattern6,	// 地紋背景パターン：うろこの時.
	TYPE_JimonBackgroundPattern7,	// 地紋背景パターン：毘沙門亀甲の時.
	TYPE_JimonBackgroundPattern8,	// 地紋背景パターン：紗綾形の時.
};

static size_t ss_JimonBackgroundPatternsMax = sizeof(tbl_JimonBackgroundPatterns) / sizeof(char);

// 現在の地紋背景パターン選択値を取得する.
short getCurrentJimonBackgroundPattern( APIC_Job* pc_Job )
{
// Comment 2016/09/29  【HCLT-Krishna】 【IT5_3.3】【Accurev ID:1004076】【Warning Removal Activity】 Changing the data type of variable 'i' from 'int' to 'uint'.
//	for (int i = 0; i < ss_JimonBackgroundPatternsMax; i++) {
	for (uint i = 0; i < ss_JimonBackgroundPatternsMax; i++) {
		if (pc_Job->getJobSpec()->pc_Common->c_JimonPattern.getJimonBackgroundPattern() == tbl_JimonBackgroundPatterns[i])
			return i;
	}
	return -1;
}
// 現在の選択インデックスから地紋背景パターンを設定する.
void setCurrentJimonBackgroundPattern( APIC_Job* pc_Job, short patidx )
{
	pc_Job->getJobSpec()->pc_Common->c_JimonPattern.setJimonBackgroundPattern(tbl_JimonBackgroundPatterns[patidx]);
}
// PStaplePunchPreviewComponent.cppからの移植.
static const struct PNLS_StaplePunchPreviewTypeTable_PD {
	TYPE_StapleMode		e_StapleMode;				// ステイプルモード.
	TYPE_BindDirection	e_StapleDirection;			// ステイプル位置.
	TYPE_PunchMode		e_PunchMode;				// パンチモード.
	TYPE_BindDirection	e_PunchDirection;			// パンチ位置.
	short				option;						// 反転するときのoption値.
	const uchar*		puc_FinishingIcon;			// 仕上げ位置アイコン.
} s_StaplePunchPreviewTypeTable_PD[] = {
	{ TYPE_StapleModeNone,		TYPE_BindDirectionAuto,	TYPE_PunchMode2,	TYPE_BindDirectionAuto,		-1,	i00_552_000_36_X96_Y96	},	// ２穴パンチ・オート.
	{ TYPE_StapleModeNone,		TYPE_BindDirectionAuto,	TYPE_PunchMode2,	TYPE_BindDirectionLeft,		0,	i00_552_000_02_X96_Y96	},	// ２穴パンチ・左.
	{ TYPE_StapleModeNone,		TYPE_BindDirectionAuto,	TYPE_PunchMode2,	TYPE_BindDirectionTop,		1,	i00_552_000_01_X96_Y96	},	// ２穴パンチ・上.
	{ TYPE_StapleModeNone,		TYPE_BindDirectionAuto,	TYPE_PunchMode2,	TYPE_BindDirectionRight,	2,	i00_552_000_03_X96_Y96	},	// ２穴パンチ・右.
	{ TYPE_StapleModeNone,		TYPE_BindDirectionAuto,	TYPE_PunchMode3,	TYPE_BindDirectionAuto,		-1,	i00_552_000_37_X96_Y96	},	// ３穴パンチ・オート.
	{ TYPE_StapleModeNone,		TYPE_BindDirectionAuto,	TYPE_PunchMode3,	TYPE_BindDirectionLeft,		0,	i00_552_000_05_X96_Y96	},	// ３穴パンチ・左.
	{ TYPE_StapleModeNone,		TYPE_BindDirectionAuto,	TYPE_PunchMode3,	TYPE_BindDirectionTop,		1,	i00_552_000_04_X96_Y96	},	// ３穴パンチ・上.
	{ TYPE_StapleModeNone,		TYPE_BindDirectionAuto,	TYPE_PunchMode3,	TYPE_BindDirectionRight,	2,	i00_552_000_06_X96_Y96	},	// ３穴パンチ・右.
	{ TYPE_StapleModeNone,		TYPE_BindDirectionAuto,	TYPE_PunchMode4,	TYPE_BindDirectionAuto,		-1,	i00_552_000_38_X96_Y96	},	// ４穴パンチ・オート.
	{ TYPE_StapleModeNone,		TYPE_BindDirectionAuto,	TYPE_PunchMode4,	TYPE_BindDirectionLeft,		0,	i00_552_000_27_X96_Y96	},	// ４穴パンチ・左.
	{ TYPE_StapleModeNone,		TYPE_BindDirectionAuto,	TYPE_PunchMode4,	TYPE_BindDirectionTop,		1,	i00_552_000_26_X96_Y96	},	// ４穴パンチ・上.
	{ TYPE_StapleModeNone,		TYPE_BindDirectionAuto,	TYPE_PunchMode4,	TYPE_BindDirectionRight,	2,	i00_552_000_28_X96_Y96	},	// ４穴パンチ・右.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionAuto,	TYPE_PunchModeNone,	TYPE_BindDirectionAuto,		-1,	i00_552_000_39_X96_Y96	},	// コーナーステープル・オート.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionLeft,	TYPE_PunchModeNone,	TYPE_BindDirectionAuto,		3,	i00_552_000_10_X96_Y96	},	// コーナーステープル・左.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionRight,TYPE_PunchModeNone,	TYPE_BindDirectionAuto,		4,	i00_552_000_11_X96_Y96	},	// コーナーステープル・右.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionTop,	TYPE_PunchModeNone,	TYPE_BindDirectionAuto,		3,	i00_552_000_10_X96_Y96	},	// コーナーステープル・上.「コーナーステープル・上」対応堀内.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionAuto,	TYPE_PunchMode2,	TYPE_BindDirectionAuto,		-1,	i00_552_000_40_X96_Y96	},	// コーナーステープル・オート／２穴パンチ・オート.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionAuto,	TYPE_PunchMode2,	TYPE_BindDirectionLeft,		5,	i00_552_000_20_X96_Y96	},	// コーナーステープル・オート／２穴パンチ・左.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionAuto,	TYPE_PunchMode2,	TYPE_BindDirectionTop,		6,	i00_552_000_18_X96_Y96	},	// コーナーステープル・オート／２穴パンチ・上.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionAuto,	TYPE_PunchMode2,	TYPE_BindDirectionRight,	8,	i00_552_000_24_X96_Y96	},	// コーナーステープル・オート／２穴パンチ・右.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionLeft,	TYPE_PunchMode2,	TYPE_BindDirectionLeft,		5,	i00_552_000_20_X96_Y96	},	// コーナーステープル・左／２穴パンチ・左.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionLeft,	TYPE_PunchMode2,	TYPE_BindDirectionTop,		6,	i00_552_000_18_X96_Y96	},	// コーナーステープル・左／２穴パンチ・上.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionRight,TYPE_PunchMode2,	TYPE_BindDirectionTop,		7,	i00_552_000_22_X96_Y96	},	// コーナーステープル・右／２穴パンチ・上.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionRight,TYPE_PunchMode2,	TYPE_BindDirectionRight,	8,	i00_552_000_24_X96_Y96	},	// コーナーステープル・右／２穴パンチ・右.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionTop,	TYPE_PunchMode2,	TYPE_BindDirectionLeft,		5,	i00_552_000_20_X96_Y96	},	// コーナーステープル・上／２穴パンチ・左.「コーナーステープル・上」対応堀内.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionTop,	TYPE_PunchMode2,	TYPE_BindDirectionTop,		6,	i00_552_000_18_X96_Y96	},	// コーナーステープル・上／２穴パンチ・上.「コーナーステープル・上」対応堀内.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionAuto,	TYPE_PunchMode3,	TYPE_BindDirectionAuto,		-1,	i00_552_000_41_X96_Y96	},	// コーナーステープル・オート／３穴パンチ・オート.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionAuto,	TYPE_PunchMode3,	TYPE_BindDirectionLeft,		5,	i00_552_000_21_X96_Y96	},	// コーナーステープル・オート／３穴パンチ・左.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionAuto,	TYPE_PunchMode3,	TYPE_BindDirectionTop,		6,	i00_552_000_19_X96_Y96	},	// コーナーステープル・オート／３穴パンチ・上.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionAuto,	TYPE_PunchMode3,	TYPE_BindDirectionRight,	8,	i00_552_000_25_X96_Y96	},	// コーナーステープル・オート／３穴パンチ・右.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionLeft,	TYPE_PunchMode3,	TYPE_BindDirectionLeft,		5,	i00_552_000_21_X96_Y96	},	// コーナーステープル・左／３穴パンチ・左.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionLeft,	TYPE_PunchMode3,	TYPE_BindDirectionTop,		6,	i00_552_000_19_X96_Y96	},	// コーナーステープル・左／３穴パンチ・上.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionRight,TYPE_PunchMode3,	TYPE_BindDirectionTop,		7,	i00_552_000_23_X96_Y96	},	// コーナーステープル・右／３穴パンチ・上.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionRight,TYPE_PunchMode3,	TYPE_BindDirectionRight,	8,	i00_552_000_25_X96_Y96	},	// コーナーステープル・右／３穴パンチ・右.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionTop,	TYPE_PunchMode3,	TYPE_BindDirectionLeft,		5,	i00_552_000_21_X96_Y96	},	// コーナーステープル・上／３穴パンチ・左.「コーナーステープル・上」対応堀内.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionTop,	TYPE_PunchMode3,	TYPE_BindDirectionTop,		6,	i00_552_000_19_X96_Y96	},	// コーナーステープル・上／３穴パンチ・上.「コーナーステープル・上」対応堀内.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionAuto,	TYPE_PunchMode4,	TYPE_BindDirectionAuto,		-1,	i00_552_000_42_X96_Y96	},	// コーナーステープル・オート／４穴パンチ・オート.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionAuto,	TYPE_PunchMode4,	TYPE_BindDirectionLeft,		5,	i00_552_000_33_X96_Y96	},	// コーナーステープル・オート／４穴パンチ・左.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionAuto,	TYPE_PunchMode4,	TYPE_BindDirectionTop,		6,	i00_552_000_32_X96_Y96	},	// コーナーステープル・オート／４穴パンチ・上.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionAuto,	TYPE_PunchMode4,	TYPE_BindDirectionRight,	8,	i00_552_000_35_X96_Y96	},	// コーナーステープル・オート／４穴パンチ・右.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionLeft,	TYPE_PunchMode4,	TYPE_BindDirectionLeft,		5,	i00_552_000_33_X96_Y96	},	// コーナーステープル・左／４穴パンチ・左.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionLeft,	TYPE_PunchMode4,	TYPE_BindDirectionTop,		6,	i00_552_000_32_X96_Y96	},	// コーナーステープル・左／４穴パンチ・上.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionRight,TYPE_PunchMode4,	TYPE_BindDirectionTop,		7,	i00_552_000_34_X96_Y96	},	// コーナーステープル・右／４穴パンチ・上.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionRight,TYPE_PunchMode4,	TYPE_BindDirectionRight,	8,	i00_552_000_35_X96_Y96	},	// コーナーステープル・右／４穴パンチ・右.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionTop,	TYPE_PunchMode4,	TYPE_BindDirectionLeft,		5,	i00_552_000_33_X96_Y96	},	// コーナーステープル上/４穴パンチ・左.「コーナーステープル上」対応堀内.
	{ TYPE_StapleModeCorner,	TYPE_BindDirectionTop,	TYPE_PunchMode4,	TYPE_BindDirectionTop,		6,	i00_552_000_32_X96_Y96	},	// コーナーステープル上/４穴パンチ・上.「コーナーステープル上」対応堀内.
	{ TYPE_StapleModeSide,		TYPE_BindDirectionAuto,	TYPE_PunchModeNone,	TYPE_BindDirectionAuto,		-1,	i00_552_000_43_X96_Y96	},	// サイドステープル・オート.
	{ TYPE_StapleModeSide,		TYPE_BindDirectionLeft,	TYPE_PunchModeNone,	TYPE_BindDirectionAuto,		0,	i00_552_000_08_X96_Y96	},	// サイドステープル・左.
	{ TYPE_StapleModeSide,		TYPE_BindDirectionTop,	TYPE_PunchModeNone,	TYPE_BindDirectionAuto,		1,	i00_552_000_07_X96_Y96	},	// サイドステープル・上.
	{ TYPE_StapleModeSide,		TYPE_BindDirectionRight,TYPE_PunchModeNone,	TYPE_BindDirectionAuto,		2,	i00_552_000_09_X96_Y96	},	// サイドステープル・右.
	{ TYPE_StapleModeSide,		TYPE_BindDirectionAuto,	TYPE_PunchMode2,	TYPE_BindDirectionAuto,		-1,	i00_552_000_44_X96_Y96	},	// サイドステープル・／２穴パンチ・オート.
	{ TYPE_StapleModeSide,		TYPE_BindDirectionLeft,	TYPE_PunchMode2,	TYPE_BindDirectionLeft,		0,	i00_552_000_14_X96_Y96	},	// サイドステープル・／２穴パンチ・左.
	{ TYPE_StapleModeSide,		TYPE_BindDirectionTop,	TYPE_PunchMode2,	TYPE_BindDirectionTop,		1,	i00_552_000_12_X96_Y96	},	// サイドステープル・／２穴パンチ・上.
	{ TYPE_StapleModeSide,		TYPE_BindDirectionRight,TYPE_PunchMode2,	TYPE_BindDirectionRight,	2,	i00_552_000_16_X96_Y96	},	// サイドステープル・／２穴パンチ・右.
	{ TYPE_StapleModeSide,		TYPE_BindDirectionAuto,	TYPE_PunchMode3,	TYPE_BindDirectionAuto,		-1,	i00_552_000_45_X96_Y96	},	// サイドステープル・／３穴パンチ・オート.
	{ TYPE_StapleModeSide,		TYPE_BindDirectionLeft,	TYPE_PunchMode3,	TYPE_BindDirectionLeft,		0,	i00_552_000_15_X96_Y96	},	// サイドステープル・／３穴パンチ・左.
	{ TYPE_StapleModeSide,		TYPE_BindDirectionTop,	TYPE_PunchMode3,	TYPE_BindDirectionTop,		1,	i00_552_000_13_X96_Y96	},	// サイドステープル・／３穴パンチ・上.
	{ TYPE_StapleModeSide,		TYPE_BindDirectionRight,TYPE_PunchMode3,	TYPE_BindDirectionRight,	2,	i00_552_000_17_X96_Y96	},	// サイドステープル・／３穴パンチ・右.
	{ TYPE_StapleModeSide,		TYPE_BindDirectionAuto,	TYPE_PunchMode4,	TYPE_BindDirectionAuto,		-1,	i00_552_000_46_X96_Y96	},	// サイドステープル・／４穴パンチ・オート.
	{ TYPE_StapleModeSide,		TYPE_BindDirectionLeft,	TYPE_PunchMode4,	TYPE_BindDirectionLeft,		0,	i00_552_000_30_X96_Y96	},	// サイドステープル・／４穴パンチ・左.
	{ TYPE_StapleModeSide,		TYPE_BindDirectionTop,	TYPE_PunchMode4,	TYPE_BindDirectionTop,		1,	i00_552_000_29_X96_Y96	},	// サイドステープル・／４穴パンチ・上.
	{ TYPE_StapleModeSide,		TYPE_BindDirectionRight,TYPE_PunchMode4,	TYPE_BindDirectionRight,	2,	i00_552_000_31_X96_Y96	},	// サイドステープル・／４穴パンチ・右.
};
uchar* ChangeFinishingModeImage_PD(
    TYPE_StapleMode e_StapleMode,
    TYPE_BindDirection e_StapleDirection,
    TYPE_PunchMode e_PunchMode,
    TYPE_BindDirection e_PunchDirection
)
{
	ushort imageIndexNum = 0;
	//引数の条件を元に表示する画像を決定.
	for( imageIndexNum=0; imageIndexNum<sizeof( s_StaplePunchPreviewTypeTable_PD )/sizeof( PNLS_StaplePunchPreviewTypeTable_PD ); imageIndexNum++ ) {
		if( ( s_StaplePunchPreviewTypeTable_PD[ imageIndexNum ].e_StapleMode==e_StapleMode ) && ( s_StaplePunchPreviewTypeTable_PD[ imageIndexNum ].e_StapleDirection == e_StapleDirection ) ) {
			if( ( s_StaplePunchPreviewTypeTable_PD[ imageIndexNum ].e_PunchMode==e_PunchMode ) && ( s_StaplePunchPreviewTypeTable_PD[ imageIndexNum ].e_PunchDirection == e_PunchDirection ) ) {
				return const_cast<uchar*>( s_StaplePunchPreviewTypeTable_PD[ imageIndexNum ].puc_FinishingIcon );
			}
		}
	}
	return NULL;
}
// オーバーレイ登録日表示関数.
void PnlComSetOverLayRegistDay_PD( PActivatableAdd* p_Btx_OverLayRegistDay, uchar uc_Index )
{
	TYPE_SystemType e_SystemType = PnlComGetMachineSetting()->getSystemType();	// 仕向け設定取得.
	char sc_RegistrationDay[JBLD_DAYDISP_MAX + 2] = { 0 };						// 登録日表示用変数.
	TYPC_Overlay c_Overlay;				// オーバーレイ情報取得.
	PnlComGetOperationSetting()->getOverlay( uc_Index, &c_Overlay, True );
	APIC_DocInfo pc_DocInfo;
	cp_OneDoc->setDocId( c_Overlay.sl_DocID );
// Commment 2017/09/22 HCL_Venkataramesh [IT5_4.2][RQ-686][WP689] setBoxId support for Network2 user.
//	cp_OneDoc->setBoxId( DOCD_OverLayBox );
	PNLC_NetworkLan::getInstance()->setBoxIdPNL( cp_OneDoc, DOCD_OverLayBox );
	if( APID_DocapiMethodOk == cp_OneDoc->getDocInfo( APID_DocapiNull, NULL, &pc_DocInfo, LIBE_CharSetDefault ) ) {
		TYPC_Time c_Time = pc_DocInfo.t_DocTime;
		if( ( c_Time.ss_Year >= 1990 ) && ( c_Time.ss_Year <= 2089 ) && ( c_Time.sc_Month >= 1 ) && ( c_Time.sc_Month <= 12 ) && ( c_Time.sc_Day >= 1 ) && ( c_Time.sc_Day <= 31 ) ) {
			switch( e_SystemType ) {	// 仕向けにより表示文字列を変更する.
				case TYPE_SystemType_US: snprintf( sc_RegistrationDay, sizeof( sc_RegistrationDay ), "%02d/%02d/%04d", c_Time.sc_Month, c_Time.sc_Day,   c_Time.ss_Year );	break;	// ＵＳ仕向け.
				case TYPE_SystemType_JP: snprintf( sc_RegistrationDay, sizeof( sc_RegistrationDay ), "%04d/%02d/%02d", c_Time.ss_Year,  c_Time.sc_Month, c_Time.sc_Day );	break;	// ＪＰ仕向け.
				case TYPE_SystemType_EU: snprintf( sc_RegistrationDay, sizeof( sc_RegistrationDay ), "%02d/%02d/%04d", c_Time.sc_Day,   c_Time.sc_Month, c_Time.ss_Year );	break;	// ＥＵ仕向け.
				default:	break;
			}
		}
	}
	p_Btx_OverLayRegistDay->changeTextString( reinterpret_cast<uchar*>( sc_RegistrationDay ) );
}
//	日にち／時間をナンバーフィールドに設定(pcgcom2.cppからの移植).
// Comment 2016/10/05 HCLT-Gomathy [IT5_3.3][1005057] Adding PActivatableAdd argument to display Date/Time in A056700100.
//void SetDayTimeDispCLCD_PD( CNumberFieldEx* pc_Num_Day, CNumberFieldEx* pc_Num_Time, time_t* c_Time, JBLE_TIME_DISP_TYPE e_TimeType, JBLE_YEAR_DISP_TYPE e_Year_Type )
void SetDayTimeDispCLCD_PD( PActivatableAdd* pc_Btx_DayTime, CNumberFieldEx* pc_Num_Day, CNumberFieldEx* pc_Num_Time, time_t* c_Time, JBLE_TIME_DISP_TYPE e_TimeType, JBLE_YEAR_DISP_TYPE e_Year_Type )
{
	struct tm st_Time = { 0 };
	if( ERROR == SYSF_localtime_r( c_Time, &st_Time ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}
	char sc_tmpDay[JBLD_DAYDISP_MAX + 2] = { 0 };				//日にち格納バッファ.
	char sc_tmpTime[JBLD_TIMEDISP_MAX + 2] = { 0 };				//時間格納バッファ.
	char sc_Year[ 5 ] = { 0 };									//年格納バッファ.
	char sc_tmpYear[ 3 ] = { 0 };
	char* sc_pYear = 0;
	char sc_QuaterS[2];
	st_Time.tm_year += 1900;									// 年を整形.
	snprintf( sc_Year, sizeof( sc_Year ), "%04d", st_Time.tm_year );
	sc_pYear = &sc_Year[ 0 ];
	sc_pYear+= 2;
	strncpy( sc_tmpYear, sc_pYear, 2 );
	memset( sc_QuaterS,0,sizeof( sc_QuaterS ) );				// 1/4角表示対応.
	sc_QuaterS[0] = FontType_ASCII_Quater;
	TYPE_SystemType e_SystemType = PnlComGetMachineSetting()->getSystemType();		// 現在の仕向け設定.
	if( e_TimeType == JBLE_TIME_DISP_TYPE_QUARTER ) {			//1/4角表示.
		if( e_Year_Type != JBLE_YEAR_DISP_TYPE_NORMAL ) {		//日にちをフォーマット
			switch( e_SystemType ) {							// 仕向けにより表示文字列を変更する.
				case TYPE_SystemType_US:	snprintf( sc_tmpDay, sizeof( sc_tmpDay ),"%s%02d/%02d/%s", sc_QuaterS, st_Time.tm_mon+1, st_Time.tm_mday, sc_tmpYear ); break;	// ＭＣ仕向け.
				case TYPE_SystemType_JP:	snprintf( sc_tmpDay, sizeof( sc_tmpDay ),"%s%s/%02d/%02d", sc_QuaterS, sc_tmpYear, st_Time.tm_mon+1, st_Time.tm_mday ); break;	// 日本仕向け.
				case TYPE_SystemType_EU:
				default: 					snprintf( sc_tmpDay, sizeof( sc_tmpDay ), "%s%02d/%02d/%s", sc_QuaterS, st_Time.tm_mday, st_Time.tm_mon+1, sc_tmpYear ); break;	// ＥＵ仕向け.
			}
		} else {
			switch( e_SystemType ) {							// 標準.// 仕向けにより表示文字列を変更する.
				case TYPE_SystemType_US:	snprintf( sc_tmpDay, sizeof( sc_tmpDay ), "%s%02d/%02d/%04d", sc_QuaterS, st_Time.tm_mon+1, st_Time.tm_mday, st_Time.tm_year ); break;	// ＭＣ仕向け.
				case TYPE_SystemType_JP:	snprintf( sc_tmpDay, sizeof( sc_tmpDay ), "%s%04d/%02d/%02d", sc_QuaterS, st_Time.tm_year, st_Time.tm_mon+1, st_Time.tm_mday ); break;	// 日本仕向け.
				case TYPE_SystemType_EU:
				default:					snprintf( sc_tmpDay, sizeof( sc_tmpDay ), "%s%02d/%02d/%04d", sc_QuaterS, st_Time.tm_mday, st_Time.tm_mon+1, st_Time.tm_year ); break;	// ＥＵ仕向け.
			}
		}
		snprintf( sc_tmpTime, sizeof( sc_tmpTime ), "%s%02d:%02d",sc_QuaterS,st_Time.tm_hour,st_Time.tm_min );		//時間をフォーマット.
	} else {
		if( e_Year_Type != JBLE_YEAR_DISP_TYPE_NORMAL ) {		//日にちをフォーマット
			switch( e_SystemType ) {							// 仕向けにより表示文字列を変更する.
				case TYPE_SystemType_US:	snprintf( sc_tmpDay, sizeof( sc_tmpDay ), "%02d/%02d/%s", st_Time.tm_mon+1, st_Time.tm_mday, sc_tmpYear ); break;	// ＭＣ仕向け.
				case TYPE_SystemType_JP:	snprintf( sc_tmpDay, sizeof( sc_tmpDay ), "%s/%02d/%02d", sc_tmpYear, st_Time.tm_mon+1, st_Time.tm_mday ); break;	// 日本仕向け.
				case TYPE_SystemType_EU:
				default:					snprintf( sc_tmpDay, sizeof( sc_tmpDay ), "%02d/%02d/%s", st_Time.tm_mday, st_Time.tm_mon+1, sc_tmpYear ); break;	// ＥＵ仕向け.
			}
		} else {
			switch( e_SystemType ) {							// 標準.// 仕向けにより表示文字列を変更する.
				case TYPE_SystemType_US:	snprintf( sc_tmpDay, sizeof( sc_tmpDay ), "%02d/%02d/%04d", st_Time.tm_mon+1, st_Time.tm_mday, st_Time.tm_year ); break;	// ＭＣ仕向け.
				case TYPE_SystemType_JP:	snprintf( sc_tmpDay, sizeof( sc_tmpDay ), "%04d/%02d/%02d", st_Time.tm_year, st_Time.tm_mon+1, st_Time.tm_mday ); break;	// 日本仕向け.
				case TYPE_SystemType_EU:
				default:					snprintf( sc_tmpDay, sizeof( sc_tmpDay ), "%02d/%02d/%04d", st_Time.tm_mday, st_Time.tm_mon+1, st_Time.tm_year ); break;	// ＥＵ仕向け.
			}
		}
		snprintf( sc_tmpTime, sizeof( sc_tmpTime ), "%02d:%02d" ,st_Time.tm_hour,st_Time.tm_min );		//時間をフォーマット.
	}
// Comment 2016/10/05 HCLT-Gomathy [IT5_3.3][1005057] pc_Btx_DayTime is used to display Date/Time.
//	pc_Num_Day->changeTextString( ( const uchar* )sc_tmpDay );											// ドキュメント登録日.
//	pc_Num_Time->changeTextString( ( const uchar* )sc_tmpTime );										// ドキュメント時間.
	pc_Btx_DayTime->changeTextString( pc_Num_Day, reinterpret_cast<const uchar*>( sc_tmpDay ) );		// ドキュメント登録日.
	pc_Btx_DayTime->changeTextString( pc_Num_Time, reinterpret_cast<const uchar*>( sc_tmpTime ) );		// ドキュメント時間.
}
//	ドキュメントの登録日を設定する(PNLC_BoxDocInfoControlLib.cppからの移植).
void setDayTimeDisplay_PD( PTextFieldEx* pc_DateTime, time_t* pc_Time, JBLE_TIME_DISP_TYPE e_TimeType, JBLE_YEAR_DISP_TYPE e_Year_Type )
{
	char		sc_tmpDay[ JBLD_DAYDISP_MAX + 2 + JBLD_TIMEDISP_MAX + 2 ] = { 0 };					// 日にち格納バッファ.
	char		sc_tmpTime[ JBLD_TIMEDISP_MAX + 2 ] = { 0 };										// 時間格納バッファ.
	char		sc_Year[ 5 ] = { 0 };																// 年格納バッファ.
	char		sc_tmpYear[ 3 ] = { 0 };
	char*		psc_Year = NULL;
	char		sc_QuaterS[ 2 ] = { 0 };
	char*		psc_DateTime;
	struct tm	st_Time = { 0 };
	if( ERROR == SYSF_localtime_r( pc_Time, &st_Time ) ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}
	st_Time.tm_year += 1900;																		// 年を整形.
	snprintf( sc_Year, sizeof( sc_Year ), "%04d", st_Time.tm_year );
	psc_Year = &sc_Year[ 0 ];
	psc_Year += 2;
	strncpy( sc_tmpYear, psc_Year, 2 );
	sc_QuaterS[ 0 ] = FontType_ASCII_Quater;														// 1/4角表示対応.
	TYPE_SystemType e_SystemType = PnlComGetMachineSetting()->getSystemType();						// 現在の仕向け設定.
	if( JBLE_TIME_DISP_TYPE_QUARTER == e_TimeType ) {												// 1/4角表示場合.
		if( JBLE_YEAR_DISP_TYPE_NORMAL != e_Year_Type ) {											// 日にちをフォーマット.
			switch( e_SystemType ) {																// 仕向けにより表示文字列を変更する.
				case TYPE_SystemType_US:	snprintf( sc_tmpDay, sizeof( sc_tmpDay ),"%s%02d/%02d/%s", sc_QuaterS, st_Time.tm_mon + 1, st_Time.tm_mday, sc_tmpYear );	break;			// ＭＣ仕向け.
				case TYPE_SystemType_JP:	snprintf( sc_tmpDay, sizeof( sc_tmpDay ),"%s%s/%02d/%02d", sc_QuaterS, sc_tmpYear, st_Time.tm_mon + 1, st_Time.tm_mday );	break;			// 日本仕向け.
				case TYPE_SystemType_EU:
				default:					snprintf( sc_tmpDay, sizeof( sc_tmpDay ),"%s%02d/%02d/%s", sc_QuaterS, st_Time.tm_mday, st_Time.tm_mon + 1, sc_tmpYear );	break;			//ＥＵ仕向け.
			}
		} else {
			switch( e_SystemType ) {																// 標準.// 仕向けにより表示文字列を変更する.
				case TYPE_SystemType_US:	snprintf( sc_tmpDay,sizeof( sc_tmpDay ),"%s%02d/%02d/%04d", sc_QuaterS, st_Time.tm_mon + 1, st_Time.tm_mday, st_Time.tm_year );		break;	// ＭＣ仕向け.
				case TYPE_SystemType_JP:	snprintf( sc_tmpDay,sizeof( sc_tmpDay ),"%s%04d/%02d/%02d", sc_QuaterS, st_Time.tm_year, st_Time.tm_mon + 1, st_Time.tm_mday );		break;	// 日本仕向け.
				case TYPE_SystemType_EU:
				default:					snprintf( sc_tmpDay, sizeof( sc_tmpDay ),"%s%02d/%02d/%04d", sc_QuaterS, st_Time.tm_mday, st_Time.tm_mon + 1, st_Time.tm_year );	break;	// ＥＵ仕向け.
			}
		}
		snprintf( sc_tmpTime, sizeof( sc_tmpTime ), "%s%02d:%02d", sc_QuaterS,st_Time.tm_hour, st_Time.tm_min );																		//時間をフォーマット.
	} else {
		if( JBLE_YEAR_DISP_TYPE_NORMAL != e_Year_Type ) {											// 日にちをフォーマット.
			switch( e_SystemType ) {																// 仕向けにより表示文字列を変更する.
				case TYPE_SystemType_US:	snprintf( sc_tmpDay, sizeof( sc_tmpDay ), "%02d/%02d/%s", st_Time.tm_mon + 1, st_Time.tm_mday, sc_tmpYear );	break;						// ＭＣ仕向け.
				case TYPE_SystemType_JP:	snprintf( sc_tmpDay, sizeof( sc_tmpDay ), "%s/%02d/%02d", sc_tmpYear, st_Time.tm_mon + 1, st_Time.tm_mday );	break;						// 日本仕向け.
				case TYPE_SystemType_EU:
				default:					snprintf( sc_tmpDay, sizeof( sc_tmpDay ), "%02d/%02d/%s", st_Time.tm_mon + 1, st_Time.tm_mday, sc_tmpYear );	break;						// ＥＵ仕向け.
			}
		} else {
			switch( e_SystemType ) {																// 標準.// 仕向けにより表示文字列を変更する.
				case TYPE_SystemType_US:	snprintf( sc_tmpDay, sizeof( sc_tmpDay ), "%02d/%02d/%04d", st_Time.tm_mon + 1, st_Time.tm_mday, st_Time.tm_year ); break;					// ＭＣ仕向け.
				case TYPE_SystemType_JP:	snprintf( sc_tmpDay, sizeof( sc_tmpDay ), "%04d/%02d/%02d", st_Time.tm_year, st_Time.tm_mon + 1, st_Time.tm_mday ); break;					// 日本仕向け.
				case TYPE_SystemType_EU:
				default:					snprintf( sc_tmpDay, sizeof( sc_tmpDay ), "%02d/%02d/%04d", st_Time.tm_mday, st_Time.tm_mon + 1, st_Time.tm_year ); break;					// ＥＵ仕向け.
			}
		}
		snprintf( sc_tmpTime, sizeof( sc_tmpTime ), "%02d:%02d", st_Time.tm_hour, st_Time.tm_min );																						// 時間をフォーマット.
	}
	psc_DateTime = sc_tmpDay;																		// ドキュメント登録日 + ドキュメント時間.
// Comment 2016/09/14 [HCLT_Archana] [1002990] 【IT5_3.3】Klocwork Changes ID: 79130 [ABV.GENERAL] strncat is used.
//	strcat( psc_DateTime, ( char* )" " );
	strncat( psc_DateTime, " " , sizeof( sc_tmpDay ) - strlen( sc_tmpDay ) -1);
	strncat( psc_DateTime, sc_tmpTime, sizeof( sc_tmpDay ) - strlen( sc_tmpDay ) -1 );
	pc_DateTime->changeASCIIText( reinterpret_cast<const uchar*>( psc_DateTime ) );									// 登録時間を表示.
}
