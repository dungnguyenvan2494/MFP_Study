
/*================================================================================================
	インクルード・ファイル
================================================================================================*/
#include "PNLC_SelectPaperTray_PD.h"
#include <panel/plibh/plibhg/pdpcomb.h>
#include <panel/pcgcom/pcgcomg/pcgcom1.h>
#include <panel/pcgcopy/pcgcopyg/pcgcopy.h>
#include <panel/pcgcopy/pcgcopyg/pcgcopy2.h>
#include <panel/plibh/plibh_CLCD2/PNLC_SelectPaperTrayCom_DataLib.h>
#include <panel/pcgcom/pcgcom_CLCD2/PNLC_PopupListControl.h>
#include <panel/pcgact/pcgact_CLCD2/pigact_pA070202.h>
#include <panel/pcgact/pcgact_PD/pcgact_A070202000.h>
extern PA070202000 gp_A070202000;
/*================================================================================================
	定数・型の宣言
================================================================================================*/
//トレイタイプテーブル.
static const TYPE_FeedTray TrayTypeTbl[PNLD_SelectPaperTrayKeyMax] = {
	TYPE_FeedTray1,
	TYPE_FeedTray2,
	TYPE_FeedTray3,
	TYPE_FeedTray4,
	TYPE_FeedTrayManual,
	TYPE_FeedTray5,
};
//トレイタイプテーブルPI装着時.
static const TYPE_FeedTray TrayTypePIAddTbl[PNLD_SelectPaperTrayKeyAddPIMax] = {
	TYPE_FeedTray1,
	TYPE_FeedTray2,
	TYPE_FeedTray3,
	TYPE_FeedTray4,
	TYPE_FeedTrayManual,
	TYPE_FeedTray5,
	TYPE_FeedTrayPI1,
	TYPE_FeedTrayPI2,
};
static const struct TYPS_TypeUserConversion {
	TYPE_PrinterAdjust			e_PrinterAdjust;
	TYPE_PaperType				e_PaperType;
} st_typeUserConversion_com[] = {
	{ TYPE_PrinterAdjustBasicWeightUser1,	TYPE_PaperTypeUser1	},
	{ TYPE_PrinterAdjustBasicWeightUser2,	TYPE_PaperTypeUser2	},
	{ TYPE_PrinterAdjustBasicWeightUser3,	TYPE_PaperTypeUser3	},
	{ TYPE_PrinterAdjustBasicWeightUser4,	TYPE_PaperTypeUser4	},
	{ TYPE_PrinterAdjustBasicWeightUser5,	TYPE_PaperTypeUser5	},
	{ TYPE_PrinterAdjustBasicWeightUser6,	TYPE_PaperTypeUser6	},
// comment 2021/02/24 Sky東島 [MinervaSSBK][制御No10].
//#if defined( DEF_EagleZpCus ) || defined( DEF_EagleZBKpCus )	 			//	2019/08/27 SCD2 一ノ瀬【機器No.7】EagleZ+/Z+BK ユーザー紙7対応.
#if defined( DEF_EagleZpCus ) || defined( DEF_EagleZBKpCus ) || defined( DEF_MinervaSSBKCus )
	{ TYPE_PrinterAdjustBasicWeightUser7,	TYPE_PaperTypeUser7	},
#endif // defined( DEF_EagleZpCus ) || defined( DEF_EagleZBKpCus ).
};
static const uchar uc_typeUserConversion_comSize = ( sizeof( st_typeUserConversion_com ) / sizeof( st_typeUserConversion_com[0] ) );
//キーID→トレイタイプ変換テーブル.
const TYPS_SelectedTray s_SelectPaperIdToTray[] = {
	{ PNLD_ID_SELECTPAPERTRAY1,			TYPE_FeedTray1      },
	{ PNLD_ID_SELECTPAPERTRAY2,			TYPE_FeedTray2      },
	{ PNLD_ID_SELECTPAPERTRAY3,			TYPE_FeedTray3      },
	{ PNLD_ID_SELECTPAPERTRAY4,			TYPE_FeedTray4      },
	{ PNLD_ID_SELECTPAPERTRAYMANUAL,	TYPE_FeedTrayManual },
	{ PNLD_ID_SELECTPAPERTRAYLCT,		TYPE_FeedTray5      },
	{ PNLD_ID_SELECTPAPERTRAY_PI1,		TYPE_FeedTrayPI1    },
	{ PNLD_ID_SELECTPAPERTRAY_PI2,		TYPE_FeedTrayPI2    },
};
const uchar uc_SelectPaperIdToTray = static_cast<uchar>( sizeof( s_SelectPaperIdToTray ) / sizeof( TYPS_SelectedTray ) );
// コンストラクタ.
PNLC_SelectPaperTrayCommon_PD::PNLC_SelectPaperTrayCommon_PD(PNLE_FeedTrayPattern e_FeedTrayPattern)
{
// Comment 2016/09/01 [HCLT_SunilNP] [1002990] 【IT5_3.3】Klocwork Changes ID: 46325, 46340, 46341, 46342, 46343, 46344 [UNINIT.CTOR.MUST] initialized the missing member variables.
	m_pc_NowJob			= NULL;
	m_pc_FeedTray		= NULL;
	m_e_FeedTray 		= TYPE_FeedTrayUnselect;
	m_e_FeedTrayPattern = e_FeedTrayPattern;
	m_p_SelectPaperTrayParts	= NULL;
	m_p_AutoKey = NULL;
	m_puc_TrayPartsTbl	= NULL;
	m_ss_TrayMax		= 0;
	memset( &m_s_MediaSourceList, 0x00, sizeof( m_s_MediaSourceList ) );
	m_b_isActiveGeneralApplication	= False;
}
// インスタンス生成.
PNLC_SelectPaperTrayCommon_PD* PNLC_SelectPaperTrayCommon_PD::newInstance(void)
{
	static PNLC_SelectPaperTrayCommon_PD c_PNLC_SelectPaperTrayCommon_PD;
	return &c_PNLC_SelectPaperTrayCommon_PD;
}
// トレイ情報表示処理.
void PNLC_SelectPaperTrayCommon_PD::setupPaperTrayInfo(PSinglePartsTextEx* p_PITitleParts)
{
	Bool bl_ExistPIParts = True;
	if( PNLE_FeedTrayPatternPaperSeeting == m_e_FeedTrayPattern ) {													// 表示対象のトレイ選択画面は「用紙選択画面」である.
		for( uchar i = 0; PNLD_SelectPaperTrayKeyAddPIMax > i; ++i ) {
			if( ( TYPE_FeedTrayPI1 == TrayTypePIAddTbl[i] && NULL == m_p_SelectPaperTrayParts->c_SelectPaperTrayKey.getPartsAddress( i ) ) || ( TYPE_FeedTrayPI2 == TrayTypePIAddTbl[i] && NULL == m_p_SelectPaperTrayParts->c_SelectPaperTrayKey.getPartsAddress( i ) )) {	// 表示対象のトレイ選択画面に「PI1パーツ」か「PI2パーツ」が存在しない.
				bl_ExistPIParts = False;
				break;
			}
		}
	}
	if( PNLE_FeedTrayPatternCoverSheetSeeting == m_e_FeedTrayPattern || PNLE_FeedTrayPatternInterSheetSeeting == m_e_FeedTrayPattern || ( PNLE_FeedTrayPatternPaperSeeting == m_e_FeedTrayPattern && bl_ExistPIParts )) {	// 表示対象のトレイ選択画面は「カバーシート用紙選択画面」か「インターシート用紙選択画面」か「(PIパーツが存在する)用紙選択画面」である.
		Bool bl_PITray = ( PnlComGetPrinterSystem()->isCoverSheetFeeder() == True ? True : False );		// ＰＩ装着状態.
		Bool bl_LCTTray = True;		// ＬＣＴトレイ表示.
		short ss_Select = setupPaperCasset2( m_e_FeedTray, False, False, bl_PITray, bl_LCTTray );		// トレイの情報表示.
		m_p_SelectPaperTrayParts->c_SelectPaperTrayKey.select( ss_Select );
		if( NULL != p_PITitleParts )  p_PITitleParts->showEnable( bl_PITray );			//ポストインサータータイトル.
	} else {
		short ss_Select = setupPaperCasset( m_e_FeedTray, False, False );		// トレイの情報表示.
		m_p_SelectPaperTrayParts->c_SelectPaperTrayKey.select( ss_Select );
	}
}
// 指定された給紙口の表示条件を変更する.
void PNLC_SelectPaperTrayCommon_PD::changeSelectPaperTrayKeyStatus( TYPE_FeedTray e_FeedTray, PNLE_PARTSSTATUS e_PartsStatus )
{
	Bool bl_ExistPIParts = True;
	short ss_TrayNum = -1;
	if( PNLE_FeedTrayPatternPaperSeeting == m_e_FeedTrayPattern ) {													// 表示対象のトレイ選択画面は「用紙選択画面」である.
		for( uchar i = 0; PNLD_SelectPaperTrayKeyAddPIMax > i; ++i ) {
			if( ( TYPE_FeedTrayPI1 == TrayTypePIAddTbl[i] && NULL == m_p_SelectPaperTrayParts->c_SelectPaperTrayKey.getPartsAddress( i ) ) || ( TYPE_FeedTrayPI2 == TrayTypePIAddTbl[i] && NULL == m_p_SelectPaperTrayParts->c_SelectPaperTrayKey.getPartsAddress( i ) )) {	// 表示対象のトレイ選択画面に「PI1パーツ」か「PI2パーツ」が存在しない.
				bl_ExistPIParts = False;
				break;
			}
		}
	}
	if( PNLE_FeedTrayPatternCoverSheetSeeting == m_e_FeedTrayPattern || PNLE_FeedTrayPatternInterSheetSeeting == m_e_FeedTrayPattern || ( PNLE_FeedTrayPatternPaperSeeting == m_e_FeedTrayPattern && bl_ExistPIParts )) {	// 表示対象のトレイ選択画面は「カバーシート用紙選択画面」か「インターシート用紙選択画面」か「(PIパーツが存在する)用紙選択画面」である.
		for( uchar i = 0; i < PNLD_SelectPaperTrayKeyAddPIMax; i++ ) {
			if( TrayTypePIAddTbl[i] == e_FeedTray ) {
				ss_TrayNum = getPartsNum(i);
				if (-1 != ss_TrayNum) {
					PFormSettingButton* pc_PartsAddress = dynamic_cast<PFormSettingButton*>( m_p_SelectPaperTrayParts->c_SelectPaperTrayKey.getPartsAddress( static_cast<uchar>( ss_TrayNum ) ) );
					if( NULL == pc_PartsAddress ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
					switch( e_PartsStatus ) {
						case PNLE_PARTSSTATUS_NOTEXIST:
						case PNLE_PARTSSTATUS_HIDE:			pc_PartsAddress->showEnable( False );	break;
						case PNLE_PARTSSTATUS_ENABLE:		pc_PartsAddress->showEnable( True );	break;
						case PNLE_PARTSSTATUS_ACTIVATE:		pc_PartsAddress->activate( True );		break;
						case PNLE_PARTSSTATUS_INACTIVATE:	pc_PartsAddress->activate( False );		break;
					}
				}
				break;
			}
		}
	} else {
		for( uchar i = 0; PNLD_SelectPaperTrayKeyMax > i; i++ ) {
			if( TrayTypeTbl[i] == e_FeedTray ) {
				ss_TrayNum = getPartsNum(i);
				if (-1 != ss_TrayNum) {
					PFormSettingButton* pc_PartsAddress = dynamic_cast<PFormSettingButton*>( m_p_SelectPaperTrayParts->c_SelectPaperTrayKey.getPartsAddress( static_cast<uchar>( ss_TrayNum ) ) );
					if( NULL == pc_PartsAddress ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
					switch( e_PartsStatus ) {
						case PNLE_PARTSSTATUS_NOTEXIST:
						case PNLE_PARTSSTATUS_HIDE:			pc_PartsAddress->showEnable( False );	break;
						case PNLE_PARTSSTATUS_ENABLE:		pc_PartsAddress->showEnable( True );	break;
						case PNLE_PARTSSTATUS_ACTIVATE:		pc_PartsAddress->activate( True );		break;
						case PNLE_PARTSSTATUS_INACTIVATE:	pc_PartsAddress->activate( False );		break;
					}
				}
				break;
			}
		}
	}
}
// 画面表示処理.
void PNLC_SelectPaperTrayCommon_PD::setupSelectPaperTray( TYPE_FeedTray e_FeedTray, Bool bl_ApsShowEnable, Bool bl_ApsOn, PSinglePartsTextEx* p_PITitleParts )
{
	m_e_FeedTray = e_FeedTray;
	setupPaperTrayInfo(p_PITitleParts);	// 給紙口情報表示・マシン図表示.
	setupPaperMachineInfo();			// マシン図表示.
	if (NULL != m_p_AutoKey) {
		if (True == PNLC_CopyDataLib::newInstance()->getPIToggleStatus()) { bl_ApsShowEnable = False; }	//ポストインサーター表示時は自動キーを非表示にする.// comment 2015/11/17 OPS1-sky南波 [CQ521645][MinrervaSBK] PI非装着時のトレイ表示.
		m_p_AutoKey->showEnable( bl_ApsShowEnable );
		if( bl_ApsShowEnable ) {
			if( bl_ApsOn )	m_p_AutoKey->chkOn();	//自動キー選択.
			else			m_p_AutoKey->chkOff();	//自動キー非選択.
		}
	}
}
// 給紙口カセットキー表示処理.
short PNLC_SelectPaperTrayCommon_PD::setupPaperCasset( const TYPE_FeedTray e_SelectFeedTray, const Bool b_HandTray, const Bool b_MPTray )
{
	short ss_Select = -1;	// 給紙口キーの選択.
	short ss_TrayNum = -1;
	Bool b_KeyDispOn;
	const TYPE_TemporaryRescueUnit e_TemporaryRescueTray[] = {
		TYPE_TemporaryRescueUnitTray1,				// トレイ1.
		TYPE_TemporaryRescueUnitTray2,				// トレイ2.
		TYPE_TemporaryRescueUnitTray3,				// トレイ3.
		TYPE_TemporaryRescueUnitTray4,				// トレイ4.
		TYPE_TemporaryRescueUnitTrayManual,			// 手差しトレイ.
// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#ifdef DEF_DenebMLKCus
		TYPE_TemporaryRescueUnitTray5,				// トレイ5.
#else
		TYPE_TemporaryRescueUnitTrayLCT,			// LCT.
// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#endif	// DEF_DenebMLKCus.
		TYPE_TemporaryRescueUnitPI,					// PI（カバーシート装置）.
		TYPE_TemporaryRescueUnitPI,					// PI（カバーシート装置）.
	};
	for( uchar i = 0; PNLD_SelectPaperTrayKeyMax > i; i++ ) {	// ボタン表示、非表示処理.
		ss_TrayNum = getPartsNum(i);
		if ( -1 != ss_TrayNum ) {
			// トレイ有り、無し判定.
// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#ifdef DEF_DenebMLKCus
			if( ( TYPE_TrayTypeNon == m_pc_FeedTray->getTrayType( TrayTypeTbl[i] ) ) || ( TYPE_TrayTypeLCT == m_pc_FeedTray->getTrayType( TrayTypeTbl[i] ) ) ) b_KeyDispOn = False;
#else
			if( ( TYPE_FeedTray5 == TrayTypeTbl[i] ) || ( TYPE_TrayTypeNon == m_pc_FeedTray->getTrayType( TrayTypeTbl[i] ) ) || ( TYPE_TrayTypeLCT == m_pc_FeedTray->getTrayType( TrayTypeTbl[i] ) ) ) b_KeyDispOn = False;
// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#endif	// DEF_DenebMLKCus.
			else if( ( True == b_MPTray ) && ( TYPE_TrayTypeMPC == m_pc_FeedTray->getTrayType( TrayTypeTbl[i] ) ) ) b_KeyDispOn = False;	// マルチトレイでマルチトレイ非表示の判定.
			else b_KeyDispOn = True;
			if( ( True == b_KeyDispOn ) && ( TYPE_FeedTrayManual == TrayTypeTbl[i] ) ) {
				if( True == b_HandTray ) b_KeyDispOn = False;	// 手差しOFFの時.
			}
			PFormSettingButton* pc_PartsAddress = dynamic_cast<PFormSettingButton*>( m_p_SelectPaperTrayParts->c_SelectPaperTrayKey.getPartsAddress( static_cast<uchar>( ss_TrayNum ) ) );
			if( NULL == pc_PartsAddress ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return ss_Select; }
			pc_PartsAddress->setCasetSize( m_pc_FeedTray->getPaperSize( TrayTypeTbl[i] ), m_pc_FeedTray->getPaperType( TrayTypeTbl[i] ).getPaperType(), m_pc_FeedTray->getPaperType( TrayTypeTbl[i] ).getFeed2ndSidedPaper(), m_pc_FeedTray->getPaperStatus( TrayTypeTbl[i] ), TrayTypeTbl[i], getRemainPaper( TrayTypeTbl[i] ) );	// トレイ情報セット.
			if( PnlComGetSystemStatus()->getTemporaryRescueUserStatus( e_TemporaryRescueTray[i] ) == TYPE_TemporaryRescueStatusSet ) b_KeyDispOn = False;	// LCC未装着時.
			if( m_b_isActiveGeneralApplication ) {	// OAPアプリが起動中か.
				if( checkOapApplicationPrintSetting( TrayTypeTbl[i] ) == False ) b_KeyDispOn = False;	//アプリの設定でtrayが非表示に設定されていた.
			}
			pc_PartsAddress->showEnable( b_KeyDispOn );
		}
	}

// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#ifndef DEF_DenebMLKCus
	ss_TrayNum = getPartsNum( PNLD_ID_SELECTPAPERTRAYLCT );
	if ( -1 != ss_TrayNum ) {
		PFormSettingButton* pc_PartsAddress = dynamic_cast<PFormSettingButton*>( m_p_SelectPaperTrayParts->c_SelectPaperTrayKey.getPartsAddress( static_cast<uchar>( ss_TrayNum ) ) );
		if( NULL == pc_PartsAddress ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return ss_Select; }
		if( TYPE_FeedTrayMax == checkLCTCasetDisp() ) pc_PartsAddress->showEnable( False );
		else {
			pc_PartsAddress->setCasetSize( m_pc_FeedTray->getPaperSize( checkLCTCasetDisp() ), m_pc_FeedTray->getPaperType( checkLCTCasetDisp() ).getPaperType(), m_pc_FeedTray->getPaperType( checkLCTCasetDisp() ).getFeed2ndSidedPaper(), m_pc_FeedTray->getPaperStatus( checkLCTCasetDisp() ), checkLCTCasetDisp(), getRemainPaper( checkLCTCasetDisp() ) );	// トレイ情報セット.
			Bool b_LCTOn = True;
			if( PnlComGetSystemStatus()->getTemporaryRescueUserStatus( TYPE_TemporaryRescueUnitTrayLCT ) == TYPE_TemporaryRescueStatusSet ) b_LCTOn = False;
			pc_PartsAddress->showEnable( b_LCTOn );
		}
	}
// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#endif	// DEF_DenebMLKCus.

	if( TYPE_FeedTrayMax == e_SelectFeedTray ) ss_Select = -1;
	else if( TYPE_TrayTypeLCT == m_pc_FeedTray->getTrayType( e_SelectFeedTray ) ) ss_Select = 5;
	else {
		switch( e_SelectFeedTray ) {
			case TYPE_FeedTray1: ss_Select = PNLD_ID_SELECTPAPERTRAY1; break;
			case TYPE_FeedTray2: ss_Select = PNLD_ID_SELECTPAPERTRAY2; break;
			case TYPE_FeedTray3: ss_Select = PNLD_ID_SELECTPAPERTRAY3; break;
			case TYPE_FeedTray4: ss_Select = PNLD_ID_SELECTPAPERTRAY4; break;
			case TYPE_FeedTrayManual:	ss_Select = PNLD_ID_SELECTPAPERTRAYMANUAL;	break;
			case TYPE_FeedTray5:		ss_Select = PNLD_ID_SELECTPAPERTRAYLCT;	break;
			case TYPE_FeedTrayPI1:
			case TYPE_FeedTrayPI2:
			case TYPE_FeedTrayMax:		ss_Select = -1;	break;
		}
	}
	if (-1 != ss_Select) ss_Select = getPartsNum(ss_Select);
	return ss_Select;
}
// 給紙口カセットキー表示処理.
short PNLC_SelectPaperTrayCommon_PD::setupPaperCasset2( const TYPE_FeedTray e_SelectFeedTray, const Bool b_HandTray, const Bool b_MPTray, const Bool b_PITray, const Bool b_LCTTray )
{
	short ss_Select = -1;
	short ss_TrayNum = -1;
	Bool b_KeyDispOn;
	const TYPE_TemporaryRescueUnit e_TemporaryRescueTray[] = {
		TYPE_TemporaryRescueUnitTray1,				// トレイ1.
		TYPE_TemporaryRescueUnitTray2,				// トレイ2.
		TYPE_TemporaryRescueUnitTray3,				// トレイ3.
		TYPE_TemporaryRescueUnitTray4,				// トレイ4.
		TYPE_TemporaryRescueUnitTrayManual,			// 手差しトレイ.
// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#ifdef DEF_DenebMLKCus
		TYPE_TemporaryRescueUnitTray5,				// トレイ5.
#else
		TYPE_TemporaryRescueUnitTrayLCT,			// LCT.
// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#endif	// DEF_DenebMLKCus.
		TYPE_TemporaryRescueUnitPI,					// PI（カバーシート装置）.
		TYPE_TemporaryRescueUnitPI,					// PI（カバーシート装置）.
	};
	Bool b_PIToggleStatus = PNLC_CopyDataLib::newInstance()->getPIToggleStatus();// comment 2015/11/17 OPS1-sky南波 [CQ521645][MinrervaSBK] PI非装着時のトレイ表示.
	for( uchar i = 0; PNLD_SelectPaperTrayKeyAddPIMax > i; i++ ) {	// ボタン表示、非表示処理.
		ss_TrayNum = getPartsNum(i);
		if ( -1 != ss_TrayNum ) {
			// トレイ有り、無し判定.
// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#ifdef DEF_DenebMLKCus
			if( ( TYPE_TrayTypeNon == m_pc_FeedTray->getTrayType( TrayTypePIAddTbl[i] ) ) || ( TYPE_TrayTypeLCT == m_pc_FeedTray->getTrayType( TrayTypePIAddTbl[i] ) ) ) b_KeyDispOn = False;
#else
			if( ( TYPE_FeedTray5 == TrayTypePIAddTbl[i] ) || ( TYPE_TrayTypeNon == m_pc_FeedTray->getTrayType( TrayTypePIAddTbl[i] ) ) || ( TYPE_TrayTypeLCT == m_pc_FeedTray->getTrayType( TrayTypePIAddTbl[i] ) ) ) b_KeyDispOn = False;
// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#endif	// DEF_DenebMLKCus.
			else if( ( True == b_MPTray ) && ( TYPE_TrayTypeMPC == m_pc_FeedTray->getTrayType( TrayTypePIAddTbl[i] ) ) ) b_KeyDispOn = False;	// マルチトレイでマルチトレイ非表示の判定.
			else b_KeyDispOn = True;
			if( ( True == b_KeyDispOn ) && ( TYPE_FeedTrayManual == TrayTypePIAddTbl[i] ) ) {	// 手差しOFFの時.
				if( True == b_HandTray ) b_KeyDispOn = False;
			}
			if( ( False == b_PITray ) && ( ( TYPE_FeedTrayPI1 == TrayTypePIAddTbl[i] ) || ( TYPE_FeedTrayPI2 == TrayTypePIAddTbl[i] ) ) ) b_KeyDispOn = False;
// comment 2015/11/17 OPS1-sky南波 [CQ521645][MinrervaSBK] PI非装着時のトレイ表示.
			if(PNLE_FeedTrayPatternPaperSeeting == m_e_FeedTrayPattern){
				if(( True == b_PIToggleStatus ) && ((TYPE_FeedTrayPI1 != TrayTypePIAddTbl[i]) && (TYPE_FeedTrayPI2 != TrayTypePIAddTbl[i]))) {
					b_KeyDispOn = False;	// PIトグル有効時はトレイ1～4を非表示.
				} else if ((False == b_PIToggleStatus) && ((TYPE_FeedTrayPI1 == TrayTypePIAddTbl[i]) || (TYPE_FeedTrayPI2 == TrayTypePIAddTbl[i]))){
					b_KeyDispOn = False;	// PIトグル無効時はPI1,PI2を非表示.
				} else { }					//何もなし.
			}
			PFormSettingButton* pc_PartsAddress = dynamic_cast<PFormSettingButton*>( m_p_SelectPaperTrayParts->c_SelectPaperTrayKey.getPartsAddress( static_cast<uchar>( ss_TrayNum ) ) );
			if( NULL == pc_PartsAddress ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return ss_Select; }
//			dynamic_cast<PFormSettingButton*>( m_p_SelectPaperTrayParts->c_SelectPaperTrayKey.getPartsAddress( ss_TrayNum ) )->setCasetSize( m_pc_FeedTray->getPaperSize( TrayTypePIAddTbl[i] ), m_pc_FeedTray->getPaperType( TrayTypePIAddTbl[i] ).getPaperType(), m_pc_FeedTray->getPaperType( TrayTypePIAddTbl[i] ).getFeed2ndSidedPaper(), m_pc_FeedTray->getPaperStatus( TrayTypePIAddTbl[i] ), TrayTypePIAddTbl[i], getRemainPaper( TrayTypePIAddTbl[i] ) );	// トレイ情報セット.
			if( PnlComGetSystemStatus()->getTemporaryRescueUserStatus( e_TemporaryRescueTray[i] ) == TYPE_TemporaryRescueStatusSet ) b_KeyDispOn = False;	// 暫定救済中はキーを非表示.
			if( m_b_isActiveGeneralApplication ) {	// OAPアプリが起動中か.
				if( checkOapApplicationPrintSetting( TrayTypePIAddTbl[i] ) == False ) b_KeyDispOn = False;	//アプリの設定でtrayが非表示に設定されていた.
			}
			if( True == b_KeyDispOn ) {
				pc_PartsAddress->setCasetSize( m_pc_FeedTray->getPaperSize( TrayTypePIAddTbl[i] ), m_pc_FeedTray->getPaperType( TrayTypePIAddTbl[i] ).getPaperType(), m_pc_FeedTray->getPaperType( TrayTypePIAddTbl[i] ).getFeed2ndSidedPaper(), m_pc_FeedTray->getPaperStatus( TrayTypePIAddTbl[i] ), TrayTypePIAddTbl[i], getRemainPaper( TrayTypePIAddTbl[i] ) );	// トレイ情報セット.
			}
			pc_PartsAddress->showEnable( b_KeyDispOn );
		}
	}

// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#ifndef DEF_DenebMLKCus
	ss_TrayNum = getPartsNum( PNLD_ID_SELECTPAPERTRAYLCT );
	if ( -1 != ss_TrayNum ) {
		PFormSettingButton* pc_PartsAddress = dynamic_cast<PFormSettingButton*>( m_p_SelectPaperTrayParts->c_SelectPaperTrayKey.getPartsAddress( static_cast<uchar>( ss_TrayNum ) ) );
		if( NULL == pc_PartsAddress ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return ss_Select; }
// comment 2015/11/17 OPS1-sky南波 [CQ521645][MinrervaSBK] PI非装着時のトレイ表示.
//		if( ( False == b_LCTTray ) || ( TYPE_FeedTrayMax == checkLCTCasetDisp() ) ) dynamic_cast<PFormSettingButton*>( m_p_SelectPaperTrayParts->c_SelectPaperTrayKey.getPartsAddress( ss_TrayNum ) )->showEnable( False );
//		else {
//			dynamic_cast<PFormSettingButton*>( m_p_SelectPaperTrayParts->c_SelectPaperTrayKey.getPartsAddress( ss_TrayNum ) )->setCasetSize( m_pc_FeedTray->getPaperSize( checkLCTCasetDisp() ), m_pc_FeedTray->getPaperType( checkLCTCasetDisp() ).getPaperType(), m_pc_FeedTray->getPaperType( checkLCTCasetDisp() ).getFeed2ndSidedPaper(), m_pc_FeedTray->getPaperStatus( checkLCTCasetDisp() ), checkLCTCasetDisp(), getRemainPaper( checkLCTCasetDisp() ) );	// トレイ情報セット.
//			Bool b_LCTOn = True;
//			if( ppc_SystemStatus->getTemporaryRescueUserStatus( TYPE_TemporaryRescueUnitTrayLCT ) == TYPE_TemporaryRescueStatusSet ) b_LCTOn = False;	// 暫定救済中はキーを非表示.
//			dynamic_cast<PFormSettingButton*>( m_p_SelectPaperTrayParts->c_SelectPaperTrayKey.getPartsAddress( ss_TrayNum ) )->showEnable( b_LCTOn );
//		}
		// LCT表示処理.
		Bool b_LCTOn = True;	TYPE_FeedTray e_FeedTrayLCT = checkLCTCasetDisp();
		if( ( False == b_LCTTray ) || ( TYPE_FeedTrayMax == e_FeedTrayLCT ) ) {	b_LCTOn = False; }
		else {
			pc_PartsAddress->setCasetSize( m_pc_FeedTray->getPaperSize( e_FeedTrayLCT ), m_pc_FeedTray->getPaperType( e_FeedTrayLCT ).getPaperType(), m_pc_FeedTray->getPaperType( e_FeedTrayLCT ).getFeed2ndSidedPaper(), m_pc_FeedTray->getPaperStatus( e_FeedTrayLCT ), e_FeedTrayLCT, getRemainPaper( e_FeedTrayLCT ) );	// トレイ情報セット.
			if( ppc_SystemStatus->getTemporaryRescueUserStatus( TYPE_TemporaryRescueUnitTrayLCT ) == TYPE_TemporaryRescueStatusSet ) { b_LCTOn = False; }// 暫定救済中はキーを非表示.
		}
// comment 2015/11/17 OPS1-sky南波 [CQ521645][MinrervaSBK] PI非装着時のトレイ表示.
		if(PNLE_FeedTrayPatternPaperSeeting == m_e_FeedTrayPattern){
			if(True == b_PIToggleStatus) { b_LCTOn = False; }	// PIトグル有効時はLCTを非表示.
		}
		pc_PartsAddress->showEnable( b_LCTOn );
	}
// comment 2018/05/17 SCD2-Sky中野 [DenebMLK][機器構成変更No.7] 5段増設プリント機能追加対応.
#endif

	if( TYPE_FeedTrayMax == e_SelectFeedTray ) ss_Select = -1;
	else if( TYPE_TrayTypeLCT == m_pc_FeedTray->getTrayType( e_SelectFeedTray ) ) ss_Select = PNLD_ID_SELECTPAPERTRAYLCT;
	else {
		switch( e_SelectFeedTray ) {
			case TYPE_FeedTray1: ss_Select = PNLD_ID_SELECTPAPERTRAY1; break;
			case TYPE_FeedTray2: ss_Select = PNLD_ID_SELECTPAPERTRAY2; break;
			case TYPE_FeedTray3: ss_Select = PNLD_ID_SELECTPAPERTRAY3; break;
			case TYPE_FeedTray4: ss_Select = PNLD_ID_SELECTPAPERTRAY4; break;
			case TYPE_FeedTrayManual: ss_Select = PNLD_ID_SELECTPAPERTRAYMANUAL; break;
			case TYPE_FeedTray5: ss_Select = PNLD_ID_SELECTPAPERTRAYLCT; break;
			case TYPE_FeedTrayPI1: ss_Select = PNLD_ID_SELECTPAPERTRAY_PI1; break;
			case TYPE_FeedTrayPI2: ss_Select = PNLD_ID_SELECTPAPERTRAY_PI2; break;
		}
	}
	if (-1 != ss_Select) ss_Select = getPartsNum(ss_Select);
	return ss_Select;
}
// 用紙のセット方向表示処理.
void PNLC_SelectPaperTrayCommon_PD::setupPaperDirection(PGroupParts& c_PaperDirectionIcon)
{
	Bool bl_DirIcon = False;	// 用紙セット方向アイコン非表示.
	if( TYPE_FeedTrayManual == m_e_FeedTray ) bl_DirIcon = True;	// 手差しトレイが現在選択中の場合、用紙セット方向アイコンを表示.
	c_PaperDirectionIcon.showEnable( bl_DirIcon );	// 用紙セット方向アイコン表示処理.
}
// Oapアプリ(から取得した)メディアソースリスト判定関数.
Bool PNLC_SelectPaperTrayCommon_PD::checkOapApplicationPrintSetting( TYPE_FeedTray e_DisplayFeedTray )
{
	for( short i = 0; i < m_s_MediaSourceList.sc_Count; i++ ) {
		switch( m_s_MediaSourceList.e_MediaSource[i] ) {
			case TYPE_ExtMediaSourceAutoSelect: ///< オート.
				break;
			case TYPE_ExtMediaSourceManualFeedTray:///< 手差し
				if( TYPE_FeedTrayManual == e_DisplayFeedTray )
				{ return( True ); }	//表示設定.
				break;
			case TYPE_ExtMediaSourceTray1:		///< トレイ1
				if( TYPE_FeedTray1 == e_DisplayFeedTray )
				{ return( True ); }	//表示設定.
				break;
			case TYPE_ExtMediaSourceTray2:		///< トレイ2
				if( TYPE_FeedTray2 == e_DisplayFeedTray )
				{ return( True ); }	//表示設定.
				break;
			case TYPE_ExtMediaSourceTray3:		///< トレイ3
				if( TYPE_FeedTray3 == e_DisplayFeedTray )
				{ return( True ); }	//表示設定.
				break;
			case TYPE_ExtMediaSourceTray4:		///< トレイ4
				if( TYPE_FeedTray4 == e_DisplayFeedTray )
				{ return( True ); }	//表示設定.
				break;
			case TYPE_ExtMediaSourceTray5:		///< トレイ5
				if( TYPE_FeedTray5 == e_DisplayFeedTray )
				{ return( True ); }	//表示設定.
				break;
			case TYPE_ExtMediaSourceTray6:		///< トレイ6
				break;
		}
	}
	return False; //チェックにひっかかなければ非表示設定.
}
// ユーザー紙名表示処理.
void PNLC_SelectPaperTrayCommon_PD::setupUserPaperTypeAndPaperKindString( const TYPE_FeedTray e_FeedTray, PNLS_SelectPaperTrayCmnPaperInfoParts* p_PSelectPaperTrayCmnPaperInfoParts )
{
	if( TYPE_FeedTrayMax == e_FeedTray ) {
		p_PSelectPaperTrayCmnPaperInfoParts->c_UserPaper.showEnable( False );
		p_PSelectPaperTrayCmnPaperInfoParts->c_PaperKind.showEnable( False );
		p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.showEnable( False );
		p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionType.showEnable( False );		//坪量非表示.
		p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionTypeuser.showEnable( False );	//坪量非表示.
		p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionTypeGPA.showEnable( False );	//坪量非表示.
		p_PSelectPaperTrayCmnPaperInfoParts->c_CustomPaper.showEnable( False );
	} else {
		TEXT_ID puc_StrUserPaper 	= 0;
		TEXT_ID puc_StrPaperKind 	= NULL;
		for( uchar i = 0; PNLC_SelectPaperTrayCom_DataLib::newInstance()->getUserPaperStringCount() > i; ++i ) {	//ユーザー紙名取得.
			if( m_pc_FeedTray->getPaperType( e_FeedTray ).getPaperType() == PNLC_SelectPaperTrayCom_DataLib::newInstance()->getUserPaperString( i ).e_PaperType ) {	// 用紙種類が一致した場合.
				if( ( TYPE_CategoryState_ENABLE == PnlComGetEnablerSetting()->getFEK_CotegoryState( TYPE_FEKType19 ) ) && ( TYPE_PaperTypeUser6 == m_pc_FeedTray->getPaperType( e_FeedTray ).getPaperType() ) ) {	// 文字列の取得.
#ifdef DEF_LCD_7INCH
					puc_StrUserPaper = s_pITMLaminate_W152H018A;	// 「ユーザー紙6」→「ラミネート」.
#else	//DEF_LCD_7INCH.
					puc_StrUserPaper = s_pITMLaminate_W152H016A;	// 「ユーザー紙6」→「ラミネート」.
#endif  //DEF_LCD_7INCH.
				} else puc_StrUserPaper = PNLC_SelectPaperTrayCom_DataLib::newInstance()->getUserPaperString( i ).puc_StrPaperType;
				break;
			}
		}
		uchar uc_Buffer[ TYPE_CustomPaperNameBuffSize+1 ] = {0};	// 変換用テキストバッファ.
		PNLC_SelectPaperTrayCom_DataLib::newInstance()->getCustomPaperNameFromCustomPaperType( m_pc_FeedTray->getPaperType( e_FeedTray ).getCustomPaperType(), uc_Buffer, sizeof( uc_Buffer ) );	// カスタム用紙属性に対応するカスタム用紙名称（SJIS）を取得する.
		if( TYPE_CustomPaperType_UnSelect != m_pc_FeedTray->getPaperType( e_FeedTray ).getCustomPaperType() ) {					// カスタム用紙属性がカスタムペーパー未選択でない場合.
			p_PSelectPaperTrayCmnPaperInfoParts->c_UserPaper.showEnable( False );
			if( TYPE_CustomPaperType20 == m_pc_FeedTray->getPaperType( e_FeedTray ).getCustomPaperType() ) p_PSelectPaperTrayCmnPaperInfoParts->c_CustomPaper.changeText( s_pITMRecycledPaper_W152H016A );
			else p_PSelectPaperTrayCmnPaperInfoParts->c_CustomPaper.changeExternalText( uc_Buffer, sizeof( uc_Buffer ), getCharSetForPanel() );
			p_PSelectPaperTrayCmnPaperInfoParts->c_CustomPaper.showEnable( True );
// Comment KronosS -> 2017/03/01 SCD2 3G Sky瀬戸 [AccuRev.1012111]ユーザー紙坪量問題.
//comment 2018/12/14 HCLT-Basker [DenebMLK][制御構成変更_No.45 ] Remove the unneccessary PaperTypes from DenebMLK models.
//#ifdef DEF_KRONOSS_Cus
#if defined (DEF_KRONOSS_Cus) || defined(DEF_DenebMLKCus)
			// KronosSではユーザー紙も坪量固定となるため共通処理にする.
			p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionTypeuser.showEnable( False );	//坪量(ユーザー紙)非表示.
			p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionTypeGPA.showEnable( False );	//坪量(ユーザー紙)非表示.
			p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionType.changeText( PNLC_SelectPaperTrayCom_DataLib::newInstance()->getUserDirectionTypeString( m_pc_FeedTray->getPaperType( e_FeedTray ).getPaperType() ) );
			p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionType.showEnable( True );		//坪量表示.
#else // DEF_KRONOSS_Cus.
			if( 0 != puc_StrUserPaper ) {	// カスタム用紙にユーザー紙が設定されている場合.
				p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionType.showEnable( False );		//坪量非表示.
				short ss_DispNum = 0;
				char sc_AryDisp[6] = {0};
				for( uchar uc_Count = 0; uc_typeUserConversion_comSize > uc_Count ; uc_Count++ ) {
					if( st_typeUserConversion_com[uc_Count].e_PaperType == m_pc_FeedTray->getPaperType( e_FeedTray ).getPaperType() ) {
						ss_DispNum = PnlComGetPrinterServiceSetting()->getPrinterAdjust( st_typeUserConversion_com[uc_Count].e_PrinterAdjust );
						break;
					}
				}
				snprintf( sc_AryDisp, sizeof( sc_AryDisp ), "%d", ss_DispNum );
				p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionTypeuser.changeASCIIText( reinterpret_cast<uchar*>( sc_AryDisp ) );
				p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionTypeuser.showEnable( True );	//坪量(ユーザー紙)表示.
				p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionTypeGPA.showEnable( True );		//坪量(ユーザー紙)表示.
			} else {	// カスタム用紙にユーザー紙以外の場合.
				p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionTypeuser.showEnable( False );	//坪量(ユーザー紙)非表示.
				p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionTypeGPA.showEnable( False );	//坪量(ユーザー紙)非表示.
				p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionType.changeText( PNLC_SelectPaperTrayCom_DataLib::newInstance()->getUserDirectionTypeString( m_pc_FeedTray->getPaperType( e_FeedTray ).getPaperType() ) );
				p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionType.showEnable( True );		//坪量表示.
			}
#endif // DEF_KRONOSS_Cus.
// Comment KronosS <- 2017/03/01 SCD2 3G Sky瀬戸 [AccuRev.1012111]ユーザー紙坪量問題.
		} else {
			p_PSelectPaperTrayCmnPaperInfoParts->c_CustomPaper.showEnable( False );
			p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionType.showEnable( False );		//坪量非表示.
			if( puc_StrUserPaper ) {	//ユーザー紙名表示.
				p_PSelectPaperTrayCmnPaperInfoParts->c_UserPaper.changeText( puc_StrUserPaper );	// 用紙種類の切り替え.
				p_PSelectPaperTrayCmnPaperInfoParts->c_UserPaper.showEnable( True );
// Comment KronosS -> 2017/03/01 SCD2 3G Sky瀬戸 [AccuRev.1012111]ユーザー紙坪量問題.
//comment 2018/12/14 HCLT-Basker [DenebMLK][制御構成変更_No.45 ] Remove the unneccessary PaperTypes from DenebMLK models.
//#ifdef DEF_KRONOSS_Cus
#if defined (DEF_KRONOSS_Cus) || defined(DEF_DenebMLKCus)
				// KronosSではユーザー紙も坪量固定となるためその他の用紙と同処理にする.
				p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionTypeuser.showEnable( False );	//坪量(ユーザー紙)非表示.
				p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionTypeGPA.showEnable( False );	//坪量(ユーザー紙)非表示.
				p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionType.changeText( PNLC_SelectPaperTrayCom_DataLib::newInstance()->getUserDirectionTypeString( m_pc_FeedTray->getPaperType( e_FeedTray ).getPaperType() ) );
				p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionType.showEnable( True );		//坪量表示.
#else // DEF_KRONOSS_Cus.
				short ss_DispNum = 0;
				char	sc_AryDisp[6] = {0};
				for( uchar uc_Count = 0; uc_typeUserConversion_comSize > uc_Count ; uc_Count++ ) {
					if( st_typeUserConversion_com[uc_Count].e_PaperType == m_pc_FeedTray->getPaperType( e_FeedTray ).getPaperType() ) {
						ss_DispNum = PnlComGetPrinterServiceSetting()->getPrinterAdjust( st_typeUserConversion_com[uc_Count].e_PrinterAdjust );
						break;
					}
				}
				snprintf( sc_AryDisp, sizeof( sc_AryDisp ), "%d", ss_DispNum );
				p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionTypeuser.changeASCIIText( reinterpret_cast<uchar*>( sc_AryDisp ) );
				p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionTypeuser.showEnable( True );	//坪量(ユーザー紙)表示.
				p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionTypeGPA.showEnable( True );		//坪量(ユーザー紙)表示.
#endif // DEF_KRONOSS_Cus.
// Comment KronosS <- 2017/03/01 SCD2 3G Sky瀬戸 [AccuRev.1012111]ユーザー紙坪量問題.
			} else {
				p_PSelectPaperTrayCmnPaperInfoParts->c_UserPaper.showEnable( False );
				p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionTypeuser.showEnable( False );	//坪量(ユーザー紙)非表示.
				p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionTypeGPA.showEnable( False );	//坪量(ユーザー紙)非表示.
				p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionType.changeText( PNLC_SelectPaperTrayCom_DataLib::newInstance()->getUserDirectionTypeString( m_pc_FeedTray->getPaperType( e_FeedTray ).getPaperType() ) );
				p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionType.showEnable( True );	//坪量非表示.
			}
		}
		for( uchar i = 0; PNLC_SelectPaperTrayCom_DataLib::newInstance()->getPaperKindStringCount() > i; ++i ) {	//用紙種類取得.
			if( m_pc_FeedTray->getPaperType( e_FeedTray ).getPaperType() == PNLC_SelectPaperTrayCom_DataLib::newInstance()->getPaperKindString( i ).e_PaperType ) {	// 用紙種類が一致した場合.
				puc_StrPaperKind = PNLC_SelectPaperTrayCom_DataLib::newInstance()->getPaperKindString( i ).puc_StrPaperType;	// 文字列の取得.
				break;
			}
		}
		if( puc_StrPaperKind ) {	//用紙種類表示.
			p_PSelectPaperTrayCmnPaperInfoParts->c_PaperKind.changeText( puc_StrPaperKind );	// 用紙種類の切り替え.
			p_PSelectPaperTrayCmnPaperInfoParts->c_PaperKind.showEnable( True );
		} else p_PSelectPaperTrayCmnPaperInfoParts->c_PaperKind.showEnable( False );
		if( True == PnlComGetPrintJobSetting()->getPaperTypeNormalEx() ){		// 普通紙Ex有効.
// comment 2015/10/13 OPS1-Sky稲葉 [ZeusS_ZX0][CQ.519941] 厚紙3にEcoが表示される不具合のフェールセーフ対応.
//			Bool	bl_show = False;
//			if ( ( True == m_pc_FeedTray->getPaperType( e_FeedTray ).getFeed2ndSidedPaper() ) && ( False == m_pc_FeedTray->getPaperType( e_FeedTray ).getPaperTypeNormalEx() ) ) {	// 両面2面目.
//				bl_show = True;
//				p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.changeText( s_pITM2ndSide_W088H016A );
//			} else if ( ( False == m_pc_FeedTray->getPaperType( e_FeedTray ).getFeed2ndSidedPaper() ) && ( True == m_pc_FeedTray->getPaperType( e_FeedTray ).getPaperTypeNormalEx() ) ) {	// Eco/厚さ変更.
//				bl_show = True;
//				if ( TYPE_SystemType_JP == PnlComGetDestination() ) p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.changeText( s_pITMEcoPaper_W088H016A );
//				else p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.changeText( s_pITMExPaper_W088H016A );
//			} else if ( ( True == m_pc_FeedTray->getPaperType( e_FeedTray ).getFeed2ndSidedPaper() ) && ( True == m_pc_FeedTray->getPaperType( e_FeedTray ).getPaperTypeNormalEx() ) ) {	// 両面2面目+Eco/厚さ変更.
//				bl_show = True;
//				if ( TYPE_SystemType_JP == PnlComGetDestination() ) p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.changeText( s_pITMNormalEcoPaper_W088H016A );
//				else p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.changeText( s_pITMNormalExPaper_W088H016A );
//			} else bl_show = False;		// 非表示にする.
//			p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.showEnable( bl_show );
//			if ( True == m_pc_FeedTray->getPaperType( e_FeedTray ).getPaperTypeNormalEx() ) p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionType.changeText( s_pNOT60to70gm2_W064H028B );
//			else p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionType.changeText( PNLC_SelectPaperTrayCom_DataLib::newInstance()->getUserDirectionTypeString( m_pc_FeedTray->getPaperType( e_FeedTray ).getPaperType() ) );
			Bool	bl_2ndSide	= m_pc_FeedTray->getPaperType( e_FeedTray ).getFeed2ndSidedPaper();
			Bool	bl_Ex		= ( TYPE_PaperTypeNormal != m_pc_FeedTray->getPaperType( e_FeedTray ).getPaperType() ) ? False : m_pc_FeedTray->getPaperType( e_FeedTray ).getPaperTypeNormalEx();	// 普通紙でない場合はEco/厚さ変更を解除する.// comment 2015/10/13 OPS1-Sky稲葉 [ZeusS_ZX0][CQ.519941] 厚紙3にEcoが表示される不具合のフェールセーフ対応.
			Bool	bl_show = False;
			if ( ( True == bl_2ndSide ) && ( False == bl_Ex ) ) {	// 両面2面目.
				bl_show = True;
//comment 2016/09/16 HCL-Utsav[1002027] Changes made for conversion to half font.
//				p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.changeText( s_pITM2ndSide_W088H016A );
#ifdef DEF_LCD_7INCH
				p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.changeText( s_pITM2ndSide_W128H018A );
#else	//DEF_LCD_7INCH.				
				p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.changeText( s_pITM2ndSide_W116H016A );
#endif  //DEF_LCD_7INCH.
			} else if ( ( False == bl_2ndSide ) && ( True == bl_Ex ) ) {	// Eco/厚さ変更.
				bl_show = True;
				if ( TYPE_SystemType_JP == PnlComGetDestination() ) p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.changeText( s_pITMEcoPaper_W088H016A );
				else p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.changeText( s_pITMExPaper_W088H016A );
			} else if ( ( True == bl_2ndSide ) && ( True == bl_Ex ) ) {	// 両面2面目+Eco/厚さ変更.
				bl_show = True;
// comment 2017/01/18 [HCL_Priya C] [IT5_4.0][1006281] Quater font activity changes.
//				if ( TYPE_SystemType_JP == PnlComGetDestination() ) p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.changeText( s_pITMNormalEcoPaper_W088H016A );
				if ( TYPE_SystemType_JP == PnlComGetDestination() ) p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.changeText( s_pITMNormalEcoPaper_W138H016A );
				else p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.changeText( s_pITMNormalExPaper_W088H016A );
			} else bl_show = False;		// 非表示にする.
			p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.showEnable( bl_show );
			if ( True == bl_Ex ) p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionType.changeText( s_pNOT60to70gm2_W064H028B );
			else p_PSelectPaperTrayCmnPaperInfoParts->c_DirectionType.changeText( PNLC_SelectPaperTrayCom_DataLib::newInstance()->getUserDirectionTypeString( m_pc_FeedTray->getPaperType( e_FeedTray ).getPaperType() ) );
		} else {
//comment 2016/09/16 HCL-Utsav[1002027] Changes made for conversion to half font.
//			p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.changeText( s_pITM2ndSide_W088H016A );
#ifdef DEF_LCD_7INCH
			p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.changeText( s_pITM2ndSide_W128H018A );
#else	//DEF_LCD_7INCH.				
			p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.changeText( s_pITM2ndSide_W116H016A );
#endif  //DEF_LCD_7INCH.
			p_PSelectPaperTrayCmnPaperInfoParts->c_2ndSide.showEnable( m_pc_FeedTray->getPaperType( e_FeedTray ).getFeed2ndSidedPaper() ? True : False );
		}
	}
}
// 初期化関数.
void PNLC_SelectPaperTrayCommon_PD::Initialize( PNLS_SelectPaperTrayCmnMndParts* p_SelectPaperTrayParts, uchar* puc_TrayPartsTbl, short ss_TrayMax, APIC_Job* pc_NowJob, APIC_FeedTray* pc_FeedTray, short ss_StartPositionX, short ss_StartPositionY, PCheckButton* p_AutoKey )
{
	m_p_AutoKey = p_AutoKey;
	m_p_SelectPaperTrayParts = p_SelectPaperTrayParts;
	m_puc_TrayPartsTbl = puc_TrayPartsTbl;
	m_ss_TrayMax	= ss_TrayMax;
	m_pc_NowJob 	= pc_NowJob;
	m_pc_FeedTray 	= pc_FeedTray;
	m_p_SelectPaperTrayParts->c_SMachinePreview.setStartPosition( ss_StartPositionX, ss_StartPositionY );
}
// 指定トレイのローテーションボタン内の登録番号取得.
short PNLC_SelectPaperTrayCommon_PD::getPartsNum( short ss_Number )
{
	short ss_TrayNum = -1;
	for(short ss_Cnt = 0; ss_Cnt < m_ss_TrayMax; ss_Cnt++) {
		if (m_puc_TrayPartsTbl[ss_Cnt] == ss_Number) {
			ss_TrayNum = ss_Cnt;
			break;
		}
	}
	return ss_TrayNum;
}
// 指定トレイのローテーションボタン内の登録番号取得.
short PNLC_SelectPaperTrayCommon_PD::getTrayPartsIdFromTrayPartsTbl( short ss_Number )
{
	short ss_TrayPartsId = -1;
	if (ss_Number < m_ss_TrayMax && ss_Number >= 0) ss_TrayPartsId = m_puc_TrayPartsTbl[ss_Number];
	return ss_TrayPartsId;
}
// インスタンス生成.
PNLC_SelectPaperTrayPaperSet_PD* PNLC_SelectPaperTrayPaperSet_PD::newInstance(void)
{
	static PNLC_SelectPaperTrayPaperSet_PD c_PNLC_SelectPaperTrayPaperSet_PD;
	return &c_PNLC_SelectPaperTrayPaperSet_PD;
}
// Initilize PI toggle status to False. 
void PNLC_SelectPaperTrayPaperSet_PD::Initialize(PNLS_SelectPaperTrayCmnMndParts* p_SelectPaperTrayCmnMndParts, uchar* puc_TrayPartsTbl, short ss_TrayMax, APIC_Job* pc_NowJob, APIC_FeedTray* pc_FeedTray, short ss_StartPositionX, short ss_StartPositionY, PCheckButton* p_AutoKey )
{
	PNLC_CopyDataLib::newInstance()->setPIToggleStatus(False);
	PNLC_SelectPaperTrayCommon_PD::Initialize(p_SelectPaperTrayCmnMndParts, puc_TrayPartsTbl, ss_TrayMax, pc_NowJob, pc_FeedTray, ss_StartPositionX, ss_StartPositionY, p_AutoKey);
}
// 描画更新処理.
void PNLC_SelectPaperTrayPaperSet_PD::update( PCheckButton& c_AutoKey, PSinglePartsTextEx* p_PITitleParts )
{
	TYPE_FeedTray e_FeedTray;
//	short ss_TrayNum = -1;
	if (True == PNLC_CopyDataLib::newInstance()->getPIToggleStatus()) e_FeedTray = PNLC_CopyDataLib::newInstance()->getSelectedFeedTray();
	else {
		e_FeedTray = m_pc_NowJob->getJobSpec()->pc_Print->c_PaperTray.getFeedTray();
		if ((True == m_pc_NowJob->getJobSpec()->pc_Print->c_PaperTray.getApsOn()) && (False == m_pc_NowJob->getJobSpec()->pc_Print->c_PaperTray.getFeedTrayEnable())) e_FeedTray = TYPE_FeedTrayMax;	//APS設定ONで給紙トレイ有効でない時.
	}
	Bool bl_show = True;
// comment 2018/08/28 【IT6_Sparrow】【HCLT_Sowjanya】【AR1052534】hiding auto key when vendor2 is set for Sparrow models.
#ifndef DEF_SparrowCus
	if (TYPE_KeyModeVendor == APIC_KeyDevice::getKeyMode()) bl_show = APIC_SoftDipSw::newInstance()->getSoftDipSwBit( 28, 4 );	// 従来ベンダー、キーカウンタI/Fベンダー時はDisSWに従って許可、不許可を切り替える。
#else
	if (TYPE_KeyModeVendorKM == APIC_KeyDevice::getKeyMode())
		bl_show = False;
#endif //DEF_SparrowCus
// comment [2015/06/01] [HCLT-Karthi][MinervaSBK][機器構成変更_No.23] Display/Hide Paper Tray based on Trya type .
// comment 2015/11/17 OPS1-sky南波 [CQ521645][MinrervaSBK] PI非装着時のトレイ表示.
//	if( PnlComGetPrinterSystem()->isCoverSheetFeeder() == True ) {		
//		if (True == PNLC_CopyDataLib::newInstance()->getPIToggleStatus()) {	
//			changeSelectPaperTrayKeyStatus( TYPE_FeedTray1, PNLE_PARTSSTATUS_HIDE );
//			changeSelectPaperTrayKeyStatus( TYPE_FeedTray2, PNLE_PARTSSTATUS_HIDE );
//			changeSelectPaperTrayKeyStatus( TYPE_FeedTray3, PNLE_PARTSSTATUS_HIDE );
//			changeSelectPaperTrayKeyStatus( TYPE_FeedTray4, PNLE_PARTSSTATUS_HIDE );
//			changeSelectPaperTrayKeyStatus( TYPE_FeedTray5, PNLE_PARTSSTATUS_HIDE );
//			changeSelectPaperTrayKeyStatus( TYPE_FeedTrayManual, PNLE_PARTSSTATUS_HIDE );
//			changeSelectPaperTrayKeyStatus( TYPE_FeedTrayPI1, PNLE_PARTSSTATUS_ENABLE );
//			changeSelectPaperTrayKeyStatus( TYPE_FeedTrayPI2, PNLE_PARTSSTATUS_ENABLE );
//			c_AutoKey.showEnable(False);
//			m_e_FeedTray = e_FeedTray;
//			for (uchar i = TYPE_FeedTrayPI1; PNLD_SelectPaperTrayKeyAddPIMax > i; i++) {
//				ss_TrayNum = getPartsNum(i);
//				if ( -1 != ss_TrayNum ) dynamic_cast<PFormSettingButton*>(m_p_SelectPaperTrayParts->c_SelectPaperTrayKey.getPartsAddress( ss_TrayNum ))->setCasetSize(m_pc_FeedTray->getPaperSize(TrayTypePIAddTbl[i]), m_pc_FeedTray->getPaperType(TrayTypePIAddTbl[i]).getPaperType(), m_pc_FeedTray->getPaperType(TrayTypePIAddTbl[i]).getFeed2ndSidedPaper(), m_pc_FeedTray->getPaperStatus(TrayTypePIAddTbl[i]), TrayTypePIAddTbl[i], getRemainPaper(TrayTypePIAddTbl[i]));
//			}
//			short ss_TrayNum = -1;
//			for (i = 0; i < uc_SelectPaperIdToTray; i++) {
//				if (s_SelectPaperIdToTray[i].e_FeedTray == e_FeedTray) {
//					ss_TrayNum = getPartsNum(s_SelectPaperIdToTray[i].ucButtonId);
//					break;
//				}
//			}
//			if (-1 != ss_TrayNum) {
//				m_p_SelectPaperTrayParts->c_SelectPaperTrayKey.select( e_FeedTray );
//			}
//			setupPaperMachineInfo();
//		} else {	//選択トレイ描画更新.
//			setupSelectPaperTray( e_FeedTray, bl_show, m_pc_NowJob->getJobSpec()->pc_Print->c_PaperTray.getApsOn(), p_PITitleParts );
// comment MinervaZeus 特ROM <- Sky桑原 2011/10/28 [DA-15] キーカウンタ・ベンダ接続時のACS・APS_Donau.
//			changeSelectPaperTrayKeyStatus( TYPE_FeedTray1, PNLE_PARTSSTATUS_ENABLE );
//			changeSelectPaperTrayKeyStatus( TYPE_FeedTray2, PNLE_PARTSSTATUS_ENABLE );
//			changeSelectPaperTrayKeyStatus( TYPE_FeedTray3, PNLE_PARTSSTATUS_ENABLE);
//			changeSelectPaperTrayKeyStatus( TYPE_FeedTray4, PNLE_PARTSSTATUS_ENABLE );
//			changeSelectPaperTrayKeyStatus( TYPE_FeedTray5, PNLE_PARTSSTATUS_ENABLE );
//			changeSelectPaperTrayKeyStatus( TYPE_FeedTrayManual, PNLE_PARTSSTATUS_ENABLE);
//			changeSelectPaperTrayKeyStatus( TYPE_FeedTrayPI1, PNLE_PARTSSTATUS_HIDE );
//			changeSelectPaperTrayKeyStatus( TYPE_FeedTrayPI2, PNLE_PARTSSTATUS_HIDE);
//		}	
//	} else setupSelectPaperTray( e_FeedTray, bl_show, m_pc_NowJob->getJobSpec()->pc_Print->c_PaperTray.getApsOn(), p_PITitleParts );
	setupSelectPaperTray( e_FeedTray, bl_show, m_pc_NowJob->getJobSpec()->pc_Print->c_PaperTray.getApsOn(), p_PITitleParts );//選択トレイ描画更新.
}
// 用紙トレイの設定処理.
void PNLC_SelectPaperTrayPaperSet_PD::setSelectedTray( uchar option )
{
	short ss_TrayPartsId = getTrayPartsIdFromTrayPartsTbl(option);
	if (-1 != ss_TrayPartsId) {
		uchar uc_TrayPartsId = static_cast<uchar>( ss_TrayPartsId );
		if( True == isOn( CNK_OHP, m_pc_NowJob ) ) {	// OHP合紙が設定されている場合（この時点でAPSではない）.
			if( ( procPaperCassetButtonOn( uc_TrayPartsId, &( s_SelectPaperIdToTray[0] ), uc_SelectPaperIdToTray ) == m_pc_NowJob->getJobSpec()->pc_Print->c_OhpInsertion.getFeedTray() ) && ( ( TYPE_TrayTypeMPC == m_pc_FeedTray->getTrayType( procPaperCassetButtonOn( uc_TrayPartsId, &( s_SelectPaperIdToTray[0] ), uc_SelectPaperIdToTray ) ) ) || ( TYPE_TrayTypeMultiMan == m_pc_FeedTray->getTrayType( procPaperCassetButtonOn( uc_TrayPartsId, &( s_SelectPaperIdToTray[0] ), uc_SelectPaperIdToTray ) ) ) ) ) {	// 選択するトレイがOHP合紙用の給紙トレイで、MPCまたはマルチ手差しの場合.
				PBasicMessageProc::setBuzzerType( PCGE_BuzzerType_InputNg );	// 選択できない給紙トレイ時は受付音を鳴らさない.
				return;
			}
		}
		if(PNLD_ID_SELECTPAPERTRAY_PI1== uc_TrayPartsId || PNLD_ID_SELECTPAPERTRAY_PI2== uc_TrayPartsId) PNLC_CopyDataLib::newInstance()->setSelectedFeedTray(procPaperCassetButtonOn( uc_TrayPartsId, &( s_SelectPaperIdToTray[0] ), uc_SelectPaperIdToTray ));
		else {
			if( False == combinationCheckPaperFeed( procPaperCassetButtonOn( uc_TrayPartsId, &( s_SelectPaperIdToTray[0] ), uc_SelectPaperIdToTray ), m_pc_NowJob ) ) {	// 給紙口選択時の禁止組み合わせチェック.
				if( False == PLBC_JudgeCenterOriAndThickPaperCombination( PLBC_ConvertPaperTypeToCombination( m_pc_FeedTray->getPaperType( procPaperCassetButtonOn( uc_TrayPartsId, &( s_SelectPaperIdToTray[0] ), uc_SelectPaperIdToTray ) ).getPaperType(), m_pc_FeedTray->getPaperType( procPaperCassetButtonOn( uc_TrayPartsId, &( s_SelectPaperIdToTray[0] ), uc_SelectPaperIdToTray ) ).getFeed2ndSidedPaper() ), m_pc_NowJob ) )	{ return; }
			}
			onlyPanel.setFeedMode( m_pc_NowJob, procPaperCassetButtonOn( uc_TrayPartsId, &( s_SelectPaperIdToTray[0] ), uc_SelectPaperIdToTray ), False );	// 給紙口指定の設定＆ＡＰＳ解除.
			onlyPanel.setAMSModeUtility( m_pc_NowJob );					// ユーティリティの設定によってAMSを設定.
			selectAutoPaperMag();										// APS/AMS処理関数.
		}
	}
}
// APSの設定変更処理.
void PNLC_SelectPaperTrayPaperSet_PD::changeApsSetting()
{
	if( m_pc_NowJob->getJobSpec()->pc_Print->c_PaperTray.getApsOn() ) {
// comment 2020/02/12 【HCLT-Thangachamy】 【OP_BTS-16026】 Combination check is added to prohibit Half fold + Thick3 setting.
		if( m_pc_NowJob->getJobSpec()->pc_Print->c_PaperType.getPaperType() == TYPE_PaperTypeThick3 
			&& PnlComGetPrinterSystem()->getFinisherType() == TYPE_FinisherTypeFS539 ) {	//If FS539 installed, If the paper type of the selected tray is thick paper 4, cancel the half fold.
			if( ( isOn( CNK_Centerori, m_pc_NowJob ) == True )
				|| ( isOn( CNK_Centerori_FS534, m_pc_NowJob ) == True )
				|| ( isOn( CNK_Centerori_Collate, m_pc_NowJob ) == True )
				|| ( isOn( CNK_Centerori_Collate_FS534, m_pc_NowJob ) == True )
				|| ( isOn( CNK_Centerori_Page, m_pc_NowJob ) == True )
				|| ( isOn( CNK_Centerori_Page_FS534, m_pc_NowJob ) == True ) ){
				combinationCheck( CNK_SpecialPaperCard3, m_pc_NowJob, True, True );
				return;
			}
		}
		cancel( CNK_APS, CNK_Size, m_pc_NowJob );	// APS解除.
		onlyPanel.setFeedTrayAPSCancel( m_pc_NowJob );	// ユーティリティの設定によってAPS解除時初期設定トレイを選択する.
		static const struct CheckPaperTypeTable {
			TYPE_PaperType	e_PaperType;				// 用紙種類.
			EP_Combination	e_Combination;				// 機能種類.
		} checkPaperTypeTable[] = {
			{ TYPE_PaperTypeNotDuplex1, CNK_S3Paper			},			// 両面不可紙.
			{ TYPE_PaperTypeOHP,		CNK_SpecialPaperOHP	},			// OHP.
			{ TYPE_PaperTypeSpecial1,	CNK_SpecialPaper	},			// 特殊紙.
			{ TYPE_PaperTypeNormal,		CNK_NomalPaper		},			// 普通紙.
			{ TYPE_PaperTypeThin1,		CNK_ThinPaper		},			// 薄紙.
			{ TYPE_PaperTypeThick1,		CNK_SpecialPaperCard1	},		// 厚紙1.
			{ TYPE_PaperTypeThick1Plus,	CNK_SpecialPaperCard1P	},		// 厚紙1+.
			{ TYPE_PaperTypeThick2,		CNK_SpecialPaperCard2	},		// 厚紙2.
			{ TYPE_PaperTypeThick3,		CNK_SpecialPaperCard3	},		// 厚紙3.
			{ TYPE_PaperTypeThick4,		CNK_SpecialPaperCard4	},		// 厚紙4.
			{ TYPE_PaperTypeNormal,		CNK_NomalPaperRemove	},		// 普通紙両面2面目.
			{ TYPE_PaperTypeThin1,		CNK_ThinPaperRemove	},			// 薄紙両面2面目.
			{ TYPE_PaperTypeThick1,		CNK_SpecialPaperCard1Remove	},	// 厚紙１両面2面目.
			{ TYPE_PaperTypeThick1Plus,	CNK_SpecialPaperCard1PRemove},	// 厚紙1+両面2面目.
			{ TYPE_PaperTypeThick2,		CNK_SpecialPaperCard2Remove	},	// 厚紙２両面2面目.
			{ TYPE_PaperTypeThick3,		CNK_SpecialPaperCard3Remove	},	// 厚紙３両面2面目.
			{ TYPE_PaperTypeThick4,		CNK_SpecialPaperCard4Remove	},	// 厚紙4 両面2面目.
			{ TYPE_PaperTypeLetterHead,	CNK_LetterHeadPaper	},			// レターヘッド紙.
			{ TYPE_PaperTypeColor,		CNK_ColorPaper		},			// 色紙.
			{ TYPE_PaperTypeEnvelope,	CNK_SpecialEnvelope	},			// 封筒.
			{ TYPE_PaperTypeIndex,		CNK_IndexPaper	},				// インデックス紙..
#ifdef DEF_A4_Cus		// VenusMLK先行 st_ysato 2015.05.12 Str.
			{ TYPE_PaperTypeGloss1,		CNK_GlossPaper1		},			// 光沢紙1.
			{ TYPE_PaperTypeGloss1Plus,	CNK_GlossPaper1Plus	},			// 光沢紙1+.
			{ TYPE_PaperTypeGloss2,		CNK_GlossPaper2		},			// 光沢紙2.
			{ TYPE_PaperPostCard,		CNK_PostCard		},			// はがき.
			{ TYPE_PaperTypeLabel,		CNK_LabelPaper		},			// ラベル紙.
#endif	// DEF_A4_Cus.	// VenusMLK先行 st_ysato 2015.05.12 End.
		};
		for( uchar i = 0; static_cast<uchar>( sizeof( checkPaperTypeTable ) / sizeof( checkPaperTypeTable[0] ) ) > i; ++i ) {	// 選択されたトレイの用紙種類の組合せ禁止後優先チェックを行う.
			if( m_pc_NowJob->getJobSpec()->pc_Print->c_PaperType.getPaperType() == checkPaperTypeTable[i].e_PaperType ) combinationCheck( checkPaperTypeTable[i].e_Combination, m_pc_NowJob, True, False );
		}
		if( m_pc_NowJob->getJobSpec()->pc_Print->c_PaperType.getPaperType() == TYPE_PaperTypeThick4 && PnlComGetPrinterSystem()->getFinisherType() == TYPE_FinisherTypeCB ) {	// CBフィニッシャー装着で、選択されたトレイの用紙種類が厚紙4の場合は、仕分けを解除する.
			if( isOn( CNK_Offset, m_pc_NowJob ) == True ) cancel( CNK_Offset, CNK_Size, m_pc_NowJob );
		}
	} else if( combinationCheck( CNK_APS, m_pc_NowJob, False ) ) {
		if( m_pc_NowJob->getJobSpec()->pc_Common->c_Mag.getAmsOn() ) {	// オート倍率がOnの場合.
			combinationCheck( CNK_APS );	// APSキーと組み合わせ禁止チェック.
			onlyPanel.setFeedMode( m_pc_NowJob, m_pc_NowJob->getJobSpec()->pc_Print->c_PaperTray.getFeedTray(), True );	// APSセット.
			if( ( TYPE_Nin1_1in1 != m_pc_NowJob->getJobSpec()->pc_Print->c_Nin1.getNin1() ) && ( PnlComGetOperationSetting()->getEnableNin1Mag() ) ) PnlComSetNin1DefaultMag( m_pc_NowJob->getJobSpec()->pc_Print, m_pc_NowJob->getJobSpec()->pc_Common );	// Nin1優先倍率設定が有効な場合 お好み倍率設定.
			else {
				m_pc_NowJob->getJobSpec()->pc_Common->c_Mag.setMagType( TYPE_MagTypeFull );	// 倍率種別を等倍に設定.
				TYPC_Point<float> c_Mag;
				c_Mag.x = PCGD_MagTypeFull; c_Mag.y = PCGD_MagTypeFull;
				m_pc_NowJob->getJobSpec()->pc_Common->c_Mag.setMag( c_Mag );	// 倍率値を等倍に設定.
			}
		} else {
			combinationCheck( CNK_APS );	// APSキーと組み合わせ禁止チェック.
			onlyPanel.setFeedMode( m_pc_NowJob, m_pc_NowJob->getJobSpec()->pc_Print->c_PaperTray.getFeedTray(), True );	// APSセット.
		}
	}
	selectAutoPaperMag();	// APS/AMS処理関数.
}
// インスタンス生成.
PNLC_SelectPaperTrayOHPInsPaperSet_PD* PNLC_SelectPaperTrayOHPInsPaperSet_PD::newInstance(void)
{
	static PNLC_SelectPaperTrayOHPInsPaperSet_PD c_PNLC_SelectPaperTrayOHPInsPaperSet_PD;
	return &c_PNLC_SelectPaperTrayOHPInsPaperSet_PD;
}
// 初期化関数.
void PNLC_SelectPaperTrayOHPInsPaperSet_PD::Initialize( PNLS_SelectPaperTrayCmnMndParts& c_SelectPaperTrayParts, uchar* puc_TrayPartsTbl, short ss_TrayMax, APIC_Job* pc_NowJob, APIC_FeedTray* pc_FeedTray, Bool b_act, short ss_StartPositionX, short ss_StartPositionY )
{
	m_p_AutoKey = NULL;
	m_p_SelectPaperTrayParts = &c_SelectPaperTrayParts;
	m_puc_TrayPartsTbl = puc_TrayPartsTbl;
	m_ss_TrayMax	= ss_TrayMax;
	m_pc_NowJob 	= pc_NowJob;
	m_pc_FeedTray 	= pc_FeedTray;
	m_p_SelectPaperTrayParts->c_SMachinePreview.setStartPosition( ss_StartPositionX, ss_StartPositionY );
}
// 描画更新処理.
void PNLC_SelectPaperTrayOHPInsPaperSet_PD::update( short option )
{
	setupSelectPaperTray( m_pc_NowJob->getJobSpec()->pc_Print->c_OhpInsertion.getFeedTray(), False, False );
	if( TYPE_PaperInsertionOff != m_pc_NowJob->getJobSpec()->pc_Print->c_OhpInsertion.getPaperInsertion() ) changeSelectPaperTrayKeyStatus( m_pc_NowJob->getJobSpec()->pc_Print->c_PaperTray.getFeedTray(), PNLE_PARTSSTATUS_HIDE );	// OHP用紙のトレイまたは、手差しの場合ボタンを非表示.
	changeSelectPaperTrayKeyStatus( TYPE_FeedTrayManual, PNLE_PARTSSTATUS_HIDE );
}
// 用紙トレイの設定処理.
void PNLC_SelectPaperTrayOHPInsPaperSet_PD::setSelectedTray( uchar option )
{
	short ss_TrayPartsId = getTrayPartsIdFromTrayPartsTbl(option);
	if (-1 != ss_TrayPartsId) {
		uchar uc_TrayPartsId = static_cast<uchar>( ss_TrayPartsId );
		// 給紙口選択時の禁止組み合わせチェック.
		if( True == combinationCheckPaperType( m_pc_NowJob, procPaperCassetButtonOn( uc_TrayPartsId, &( s_SelectPaperIdToTray[0] ), uc_SelectPaperIdToTray ), False, CNK_OHP ) ) {
			combinationCheckPaperType( m_pc_NowJob, procPaperCassetButtonOn( uc_TrayPartsId, &( s_SelectPaperIdToTray[0] ), uc_SelectPaperIdToTray ), True, CNK_OHP );				// 選択可能な給紙口しか表示しないので、用紙種類の組合せ禁止チェックのみ行う.
			m_pc_NowJob->getJobSpec()->pc_Print->c_OhpInsertion.setFeedTray( procPaperCassetButtonOn( uc_TrayPartsId, &( s_SelectPaperIdToTray[0] ), uc_SelectPaperIdToTray ) );	// OHP合紙の選択トレイを設定.
		}
		selectAutoPaperMag();	// APS/AMS処理関数.
	}
}
// インスタンス生成.
PNLC_SelectPaperTrayCoverPaperSet_PD* PNLC_SelectPaperTrayCoverPaperSet_PD::newInstance(void)
{
	static PNLC_SelectPaperTrayCoverPaperSet_PD c_PNLC_SelectPaperTrayCoverPaperSet_PD;
	return &c_PNLC_SelectPaperTrayCoverPaperSet_PD;
}
// 初期化関数.
void PNLC_SelectPaperTrayCoverPaperSet_PD::Initialize( PNLS_SelectPaperTrayCmnMndParts& c_SelectPaperTrayParts, uchar* puc_TrayPartsTbl, short ss_TrayMax, APIC_Job* pc_NowJob, APIC_FeedTray* pc_FeedTray, Bool b_CoversheetFront, short ss_StartPositionX, short ss_StartPositionY )
{
	m_p_AutoKey = NULL;
	m_p_SelectPaperTrayParts = &c_SelectPaperTrayParts;
	m_puc_TrayPartsTbl = puc_TrayPartsTbl;
	m_ss_TrayMax	= ss_TrayMax;
	m_pc_NowJob 	= pc_NowJob;
	m_pc_FeedTray 	= pc_FeedTray;
	m_p_SelectPaperTrayParts->c_SMachinePreview.setStartPosition( ss_StartPositionX, ss_StartPositionY );
	m_b_CoversheetFront = b_CoversheetFront;	//表カバーフラグ設定.
}
// カバー用紙 描画更新処理.
void PNLC_SelectPaperTrayCoverPaperSet_PD::update( short option )
{
	setupSelectPaperTray( getSelectedFeedTray(), False, False );	// パーツの描画更新.
	if( PnlComGetPrinterSystem()->isCoverSheetFeeder() == True ) {	// ＰＩトレイが装着の場合.
		if( ( m_pc_NowJob->getJobSpec()->pc_Print->c_Intersheet.getFrontMode().getPaperInsertion() == TYPE_PaperInsertionWithImage && m_b_CoversheetFront == True ) || ( m_pc_NowJob->getJobSpec()->pc_Print->c_Intersheet.getBackMode().getPaperInsertion() == TYPE_PaperInsertionWithImage && m_b_CoversheetFront == False ) ) {	// カバーシート画面で表コピー／裏コピー選択時は、「上段トレイキー」と「下段トレイキー」をグレーアウトにする.
			changeSelectPaperTrayKeyStatus( TYPE_FeedTrayPI1, PNLE_PARTSSTATUS_INACTIVATE );
			changeSelectPaperTrayKeyStatus( TYPE_FeedTrayPI2, PNLE_PARTSSTATUS_INACTIVATE );
		} else {
			changeSelectPaperTrayKeyStatus( TYPE_FeedTrayPI1, PNLE_PARTSSTATUS_ACTIVATE );
			changeSelectPaperTrayKeyStatus( TYPE_FeedTrayPI2, PNLE_PARTSSTATUS_ACTIVATE );
		}
	}
	if( True == isOn( CNK_ProgramJob, m_pc_NowJob ) ) {
		if( PNLC_PopupListControl::newInstance()->getNowPopup() == &gp_A070202000 ) {
			// プログラムジョブON時かつ実行中の場合は、PIトレイ以外を非表示にする.
			changeSelectPaperTrayKeyStatus( TYPE_FeedTray1, PNLE_PARTSSTATUS_HIDE );
			changeSelectPaperTrayKeyStatus( TYPE_FeedTray2, PNLE_PARTSSTATUS_HIDE );
			changeSelectPaperTrayKeyStatus( TYPE_FeedTray3, PNLE_PARTSSTATUS_HIDE );
			changeSelectPaperTrayKeyStatus( TYPE_FeedTray4, PNLE_PARTSSTATUS_HIDE );
			changeSelectPaperTrayKeyStatus( TYPE_FeedTray5, PNLE_PARTSSTATUS_HIDE );
			changeSelectPaperTrayKeyStatus( TYPE_FeedTrayManual, PNLE_PARTSSTATUS_HIDE );
		} else {
			// プログラムジョブON時は、PIトレイ以外を非活性にする.
			changeSelectPaperTrayKeyStatus( TYPE_FeedTray1, PNLE_PARTSSTATUS_INACTIVATE );
			changeSelectPaperTrayKeyStatus( TYPE_FeedTray2, PNLE_PARTSSTATUS_INACTIVATE );
			changeSelectPaperTrayKeyStatus( TYPE_FeedTray3, PNLE_PARTSSTATUS_INACTIVATE );
			changeSelectPaperTrayKeyStatus( TYPE_FeedTray4, PNLE_PARTSSTATUS_INACTIVATE );
			changeSelectPaperTrayKeyStatus( TYPE_FeedTray5, PNLE_PARTSSTATUS_INACTIVATE );
			changeSelectPaperTrayKeyStatus( TYPE_FeedTrayManual, PNLE_PARTSSTATUS_INACTIVATE );
		}
	}
}
// カバー用紙  用紙トレイの設定処理.
void PNLC_SelectPaperTrayCoverPaperSet_PD::setSelectedTray( uchar option )
{
	short ss_TrayPartsId = getTrayPartsIdFromTrayPartsTbl(option);
	if (-1 != ss_TrayPartsId) {
		const TYPE_FeedTray e_FeedTray = procPaperCassetButtonOn( static_cast<uchar>( ss_TrayPartsId ), &( s_SelectPaperIdToTray[0] ), uc_SelectPaperIdToTray );	// 選択されたトレイの取得.
		if( True == combinationCheckPaperFeed( e_FeedTray, m_pc_NowJob, ( ( True == m_b_CoversheetFront ) ? CNK_CoverModeFront : CNK_CoverModeBack ), True ) ) {	// 給紙口選択時の禁止組み合わせチェック.
			if( True == m_b_CoversheetFront ) {
				TYPC_IntersheetMode c_CoversheetFront = m_pc_NowJob->getJobSpec()->pc_Print->c_Intersheet.getFrontMode();
				c_CoversheetFront.setFeedTray( e_FeedTray );	// オモテ表紙の選択トレイを変更.
				if( ( ( TYPE_FeedTrayPI1 == e_FeedTray ) || ( TYPE_FeedTrayPI2 == e_FeedTray ) ) && ( TYPE_PaperInsertionWithImage == c_CoversheetFront.getPaperInsertion() ) ) c_CoversheetFront.setPaperInsertion( TYPE_PaperInsertionWithoutImage );	// PIトレイ選択時に表紙にコピーする場合.
				m_pc_NowJob->getJobSpec()->pc_Print->c_Intersheet.setFrontMode( c_CoversheetFront );	// 変更されたオモテ表紙の選択トレイを設定.
			} else {
				TYPC_IntersheetMode c_CoversheetBack  = m_pc_NowJob->getJobSpec()->pc_Print->c_Intersheet.getBackMode();
				c_CoversheetBack.setFeedTray( e_FeedTray );		// ウラ表紙の選択トレイを変更.
				if( ( ( TYPE_FeedTrayPI1 == e_FeedTray ) || ( TYPE_FeedTrayPI2 == e_FeedTray ) ) && ( TYPE_PaperInsertionWithImage == c_CoversheetBack.getPaperInsertion() ) ) c_CoversheetBack.setPaperInsertion( TYPE_PaperInsertionWithoutImage );	// PIトレイ選択時に表紙にコピーする場合.
				m_pc_NowJob->getJobSpec()->pc_Print->c_Intersheet.setBackMode( c_CoversheetBack );	// 変更されたウラ表紙の選択トレイを設定.
			}
		}
	}
}
// 設定トレイ情報取得処理.
TYPE_FeedTray PNLC_SelectPaperTrayCoverPaperSet_PD::getSelectedFeedTray()
{
	TYPE_FeedTray e_FeedTray = TYPE_FeedTrayMax;
	if( True == m_b_CoversheetFront ) e_FeedTray = m_pc_NowJob->getJobSpec()->pc_Print->c_Intersheet.getFrontMode().getFeedTray();	// オモテ表紙の選択トレイを取得.
	else e_FeedTray = m_pc_NowJob->getJobSpec()->pc_Print->c_Intersheet.getBackMode().getFeedTray();	// ウラ表紙の選択トレイを取得.
	return e_FeedTray;
}
// インスタンス生成.
PNLC_SelectPaperTrayPageInsertPaperSet_PD* PNLC_SelectPaperTrayPageInsertPaperSet_PD::newInstance(void)
{
	static PNLC_SelectPaperTrayPageInsertPaperSet_PD c_PNLC_SelectPaperTrayPageInsertPaperSet_PD;
	return &c_PNLC_SelectPaperTrayPageInsertPaperSet_PD;
}
// 描画更新処理.
void PNLC_SelectPaperTrayPageInsertPaperSet_PD::update( short option )
{
	setupSelectPaperTray( m_pc_NowJob->getJobSpec()->pc_Print->c_Intersheet.getFeedTrayMiddle( 0 ), False, False );	// パーツの描画更新.
	if( PnlComGetPrinterSystem()->isCoverSheetFeeder() == True ) {	// ＰＩトレイが装着の場合.
		if( m_pc_NowJob->getJobSpec()->pc_Print->c_Intersheet.getMiddleMode().getPaperInsertion() == TYPE_PaperInsertionWithImage ) {	// 挿入方法が「コピーする」の場合は、ＰＩトレイキーをグレーアウトにする.
			changeSelectPaperTrayKeyStatus( TYPE_FeedTrayPI1, PNLE_PARTSSTATUS_INACTIVATE );
			changeSelectPaperTrayKeyStatus( TYPE_FeedTrayPI2, PNLE_PARTSSTATUS_INACTIVATE );
		} else {
			changeSelectPaperTrayKeyStatus( TYPE_FeedTrayPI1, PNLE_PARTSSTATUS_ACTIVATE );
			changeSelectPaperTrayKeyStatus( TYPE_FeedTrayPI2, PNLE_PARTSSTATUS_ACTIVATE );
		}
	}
}
// 用紙トレイの設定処理.
void PNLC_SelectPaperTrayPageInsertPaperSet_PD::setSelectedTray( uchar option )
{
	short ss_TrayPartsId = getTrayPartsIdFromTrayPartsTbl(option);
	if (-1 != ss_TrayPartsId) {
		const TYPE_FeedTray e_FeedTray = procPaperCassetButtonOn( static_cast<uchar>( ss_TrayPartsId ), &( s_SelectPaperIdToTray[0] ), uc_SelectPaperIdToTray );							// 選択されたトレイの取得.
		if( True == combinationCheckPaperFeed( e_FeedTray, m_pc_NowJob, CNK_MiddleMode, True ) ) {
			for( uchar i = 0; PCGD_PAGE_NUMBER_CNT_MAX > i; ++i ) 	m_pc_NowJob->getJobSpec()->pc_Print->c_Intersheet.setFeedTrayMiddle( i, e_FeedTray );	// 挿入紙の選択トレイを設定.
			TYPC_IntersheetMode c_IntersheetMode = m_pc_NowJob->getJobSpec()->pc_Print->c_Intersheet.getMiddleMode();										// 挿入紙の選択トレイを設定.
			c_IntersheetMode.setFeedTray( e_FeedTray );
			if( e_FeedTray == TYPE_FeedTrayPI1 || e_FeedTray == TYPE_FeedTrayPI2 ) c_IntersheetMode.setPaperInsertion( TYPE_PaperInsertionWithoutImage );	// トレイがフィニッシャーのPIトレイならば挿入方法は「コピーしない」.
			m_pc_NowJob->getJobSpec()->pc_Print->c_Intersheet.setMiddleMode( c_IntersheetMode );
		}
	}
}
// インスタンス生成.
PNLC_SelectPaperTrayChapterPaperSet_PD* PNLC_SelectPaperTrayChapterPaperSet_PD::newInstance(void)
{
	static PNLC_SelectPaperTrayChapterPaperSet_PD c_PNLC_SelectPaperTrayChapterPaperSet_PD;
	return &c_PNLC_SelectPaperTrayChapterPaperSet_PD;
}
// 用紙トレイの設定処理.
void PNLC_SelectPaperTrayChapterPaperSet_PD::setSelectedTray( uchar option )
{
	short ss_TrayPartsId = getTrayPartsIdFromTrayPartsTbl(option);
	if (-1 != ss_TrayPartsId) {
		const TYPE_FeedTray e_FeedTray = procPaperCassetButtonOn( static_cast<uchar>( ss_TrayPartsId ), &( s_SelectPaperIdToTray[0] ), uc_SelectPaperIdToTray );	// 選択されたトレイの取得.
		if( True == combinationCheckPaperFeed( e_FeedTray, m_pc_NowJob, CNK_Chapter, True ) ) m_pc_NowJob->getJobSpec()->pc_Print->c_Chapter.setFeedTray( e_FeedTray );	// OHP合紙が設定されている場合（この時点でAPSではない）.
	}
}
// インスタンス生成.
PNLC_SelectPaperTrayPaperSetBox_PD* PNLC_SelectPaperTrayPaperSetBox_PD::newInstance(void)
{
	static PNLC_SelectPaperTrayPaperSetBox_PD c_PNLC_SelectPaperTrayPaperSetBox_PD;
	return &c_PNLC_SelectPaperTrayPaperSetBox_PD;
}
// 用紙トレイの設定処理.
void PNLC_SelectPaperTrayPaperSetBox_PD::setSelectedTray( uchar option )
{
	short ss_TrayPartsId = getTrayPartsIdFromTrayPartsTbl(option);
	if (-1 != ss_TrayPartsId) {
		const TYPE_FeedTray e_FeedTray = procPaperCassetButtonOn( static_cast<uchar>( ss_TrayPartsId ), &( s_SelectPaperIdToTray[0] ), uc_SelectPaperIdToTray );	// 選択されたトレイの取得.
		if( True == isOn( CNK_OHP, m_pc_NowJob ) ) {	// OHP合紙が設定されている場合（この時点でAPSではない）.
			if( ( e_FeedTray == m_pc_NowJob->getJobSpec()->pc_Print->c_OhpInsertion.getFeedTray() ) && ( ( TYPE_TrayTypeMPC == m_pc_FeedTray->getTrayType( e_FeedTray ) ) || ( TYPE_TrayTypeMultiMan == m_pc_FeedTray->getTrayType( e_FeedTray ) ) ) ) {	// 選択するトレイがOHP合紙用の給紙トレイで、MPCまたはマルチ手差しの場合.
				PBasicMessageProc::setBuzzerType( PCGE_BuzzerType_InputNg );	// 選択できない給紙トレイ時は受付音を鳴らさない.
				return;
			}
		}
		if( False == combinationCheckPaperFeed( e_FeedTray, m_pc_NowJob ) ) {	// 給紙口選択時の禁止組み合わせチェック.
			EP_Combination e_combi = PLBC_ConvertPaperTypeToCombination( m_pc_FeedTray->getPaperType( e_FeedTray ).getPaperType(), m_pc_FeedTray->getPaperType( e_FeedTray ).getFeed2ndSidedPaper() );
			if( False == PLBC_JudgeCenterOriAndThickPaperCombination( e_combi, m_pc_NowJob ) )	{ return; }
		}
		onlyPanel.setFeedMode( m_pc_NowJob, e_FeedTray, False );	// 給紙口指定の設定＆ＡＰＳ解除.
		onlyPanel.setAMSModeUtility( m_pc_NowJob );	// ユーティリティの設定によってAMSを設定.
		selectAutoPaperMag();	// APS/AMS処理関数.
		if( True == ErrorCheck()) {	// 警告検出.
			onlyPanel.setFeedMode( m_pc_NowJob, m_pc_NowJob->getJobSpec()->pc_Print->c_PaperTray.getFeedTray(), m_pc_NowJob->getJobSpec()->pc_Print->c_PaperTray.getApsOn() );	// 設定した値を元に戻す.
			onlyPanel.setAMSModeUtility( m_pc_NowJob );								// ユーティリティの設定によってAMSを設定.
			selectAutoPaperMag();													// APS/AMS処理関数.
			onlyPanel.setPanelWarning( PNL_ERROR_BoxDocChangeImp );					// 警告表示.
		}
	}
}
// APSの設定変更処理.
void PNLC_SelectPaperTrayPaperSetBox_PD::changeApsSetting()
{
	if( m_pc_NowJob->getJobSpec()->pc_Print->c_PaperTray.getApsOn() ) {	// APSがON→OFF時.
		cancel( CNK_APS, CNK_Size, m_pc_NowJob );			// APS解除.
		onlyPanel.setFeedTrayAPSCancel( m_pc_NowJob );		// ユーティリティの設定によってAPS解除時初期設定トレイを選択する.
		selectAutoPaperMag();								// APS/AMS処理関数.
		static const struct CheckPaperTypeTable {
			TYPE_PaperType	e_PaperType;				// 用紙種類.
			EP_Combination	e_Combination;				// 機能種類.
		} checkPaperTypeTable[] = {
			{ TYPE_PaperTypeNotDuplex1, CNK_S3Paper			},			// 両面不可紙.
			{ TYPE_PaperTypeOHP,		CNK_SpecialPaperOHP	},			// OHP.
			{ TYPE_PaperTypeSpecial1,	CNK_SpecialPaper	},			// 特殊紙.
			{ TYPE_PaperTypeNormal,		CNK_NomalPaper		},			// 普通紙.
			{ TYPE_PaperTypeThin1,		CNK_ThinPaper		},			// 薄紙.// comment MinervaZeus  Sky小尾 2011/05/16 [UV-01038][機器1001].
			{ TYPE_PaperTypeThick1,		CNK_SpecialPaperCard1	},		// 厚紙1.
			{ TYPE_PaperTypeThick1Plus,	CNK_SpecialPaperCard1P	},		// 厚紙1+.
			{ TYPE_PaperTypeThick2,		CNK_SpecialPaperCard2	},		// 厚紙2.
			{ TYPE_PaperTypeThick3,		CNK_SpecialPaperCard3	},		// 厚紙3.
			{ TYPE_PaperTypeThick4,		CNK_SpecialPaperCard4	},		// 厚紙4.
			{ TYPE_PaperTypeNormal,		CNK_NomalPaperRemove	},		// 普通紙両面2面目.
			{ TYPE_PaperTypeThin1,		CNK_ThinPaperRemove		},		// 薄紙両面2面目.// comment MinervaZeus Sky小尾 2011/05/16 [UV-01038][機器1001].
			{ TYPE_PaperTypeThick1,		CNK_SpecialPaperCard1Remove	},	// 厚紙１両面2面目.
			{ TYPE_PaperTypeThick1Plus,	CNK_SpecialPaperCard1PRemove},	// 厚紙1+両面2面目.
			{ TYPE_PaperTypeThick2,		CNK_SpecialPaperCard2Remove	},	// 厚紙２両面2面目.
			{ TYPE_PaperTypeThick3,		CNK_SpecialPaperCard3Remove	},	// 厚紙３両面2面目.
			{ TYPE_PaperTypeThick4,		CNK_SpecialPaperCard4Remove	},	// 厚紙4 両面2面目.
			{ TYPE_PaperTypeLetterHead,	CNK_LetterHeadPaper	},			// レターヘッド紙.
			{ TYPE_PaperTypeColor,		CNK_ColorPaper		},			// 色紙.
			{ TYPE_PaperTypeEnvelope,	CNK_SpecialEnvelope	},			// 封筒.
			{ TYPE_PaperTypeIndex,		CNK_IndexPaper	},				// インデックス紙.// comment MinervaZeus Sky小尾 2011/05/16 [UV-01038][機器1001].
#ifdef DEF_A4_Cus		// VenusMLK先行 st_ysato 2015.05.12 Str.
			{ TYPE_PaperTypeGloss1,		CNK_GlossPaper1		},			// 光沢紙1.
			{ TYPE_PaperTypeGloss1Plus,	CNK_GlossPaper1Plus	},			// 光沢紙1+.
			{ TYPE_PaperTypeGloss2,		CNK_GlossPaper2		},			// 光沢紙2.
			{ TYPE_PaperPostCard,		CNK_PostCard		},			// はがき.
			{ TYPE_PaperTypeLabel,		CNK_LabelPaper		},			// ラベル紙.
#endif	// DEF_A4_Cus.	// VenusMLK先行 st_ysato 2015.05.12 End.
		};
		for( uchar i = 0; static_cast<uchar>( sizeof( checkPaperTypeTable ) / sizeof( checkPaperTypeTable[0] ) ) > i; ++i ) {	// 選択されたトレイの用紙種類の組合せ禁止後優先チェックを行う.
			if( m_pc_NowJob->getJobSpec()->pc_Print->c_PaperType.getPaperType() == checkPaperTypeTable[i].e_PaperType ) {
				if( False == combinationCheck( checkPaperTypeTable[i].e_Combination, m_pc_NowJob ) ) {	// 組禁チェック.
					changeApsOn();		// 警告発生時はAPSに戻す.
					return;
				}
			}
		}
		if( True == ErrorCheck() ) {	// 警告検出.
			changeApsOn();		// 警告発生時はAPSに戻す.
			onlyPanel.setPanelWarning( PNL_ERROR_BoxDocChangeImp );		// 警告表示.
		}
	} else changeApsOn();	// APSがOFF→ON時, APS設定処理.
}
// 警告検知処理.
Bool PNLC_SelectPaperTrayPaperSetBox_PD::ErrorCheck()
{
	Bool b_Error = False;
	const TYPE_WarningName e_UsbBoxNgSpec[] = { PCAUTION_OffsetPaperSizeError_S };	// 用紙種類に関しては、combinationCheck()で検知しているため、ここではチェックしない. // オフセット用紙サイズ不適合(Caution).
	for( ushort i = 0; i < sizeof( e_UsbBoxNgSpec ) / sizeof( e_UsbBoxNgSpec[0] ); i++ ) {	// 警告チェック.
		if( True == onlyPanel.isWarningOn( m_pc_NowJob, e_UsbBoxNgSpec[i] ) ) b_Error = True;	// 警告が発生しているかチェック.
	}
	return b_Error;
}
// APS解除処理.
void PNLC_SelectPaperTrayPaperSetBox_PD::changeApsOn()
{
	if( combinationCheck( CNK_APS, m_pc_NowJob, False ) ) {
		if( m_pc_NowJob->getJobSpec()->pc_Common->c_Mag.getAmsOn() ) {	// オート倍率がOnの場合.
			combinationCheck( CNK_APS );									// APSキーと組み合わせ禁止チェック.
			onlyPanel.setFeedMode( m_pc_NowJob, m_pc_NowJob->getJobSpec()->pc_Print->c_PaperTray.getFeedTray(), True );			// APSセット.
			m_pc_NowJob->getJobSpec()->pc_Print->c_PaperType.setPaperType( TYPE_PaperTypeNormal );		// セットされたジョブの用紙種類を強制的に普通紙へ変更.
			m_pc_NowJob->getJobSpec()->pc_Print->c_PaperType.setFeed2ndSidedPaper( False );			// 両面2面目を解除.
			m_pc_NowJob->getJobSpec()->pc_Print->c_PaperSize.e_SizeCode = TYPE_SizeCodeUnknown;		// 用紙サイズをUnknownにｾｯﾄ.
			if( TYPE_Nin1_1in1 != m_pc_NowJob->getJobSpec()->pc_Print->c_Nin1.getNin1() && PnlComGetOperationSetting()->getEnableNin1Mag() ) PnlComSetNin1DefaultMag( m_pc_NowJob->getJobSpec()->pc_Print, m_pc_NowJob->getJobSpec()->pc_Common );	// Nin1優先倍率設定が有効な場合,  お好み倍率設定.
			else {
				m_pc_NowJob->getJobSpec()->pc_Common->c_Mag.setMagType( TYPE_MagTypeFull );			// 倍率種別を等倍に設定.
				TYPC_Point<float> c_Mag;
				c_Mag.x = PCGD_MagTypeFull;
				c_Mag.y = PCGD_MagTypeFull;
				m_pc_NowJob->getJobSpec()->pc_Common->c_Mag.setMag( c_Mag );	// 倍率値を等倍に設定.
			}
		} else {
			combinationCheck( CNK_APS );									// APSキーと組み合わせ禁止チェック.
			onlyPanel.setFeedMode( m_pc_NowJob, m_pc_NowJob->getJobSpec()->pc_Print->c_PaperTray.getFeedTray(), True );			// APSセット.
			m_pc_NowJob->getJobSpec()->pc_Print->c_PaperType.setPaperType( TYPE_PaperTypeNormal );		// セットされたジョブの用紙種類を強制的に普通紙へ変更.
			m_pc_NowJob->getJobSpec()->pc_Print->c_PaperType.setFeed2ndSidedPaper( False );			// 両面2面目を解除.
			m_pc_NowJob->getJobSpec()->pc_Print->c_PaperSize.e_SizeCode = TYPE_SizeCodeUnknown;		// 用紙サイズをUnknownにｾｯﾄ.
		}
	}
	selectAutoPaperMag();	// APS/AMS処理関数.
}

