//ファイル名：pcgacce_CLCD.cpp
//概要:ユーザー補助機能.
//////////////////////////////////////////////////////////////
#include <panel/plibh/plibh_CLCD/plibh_Mtui.h>
#include "panel/pddtext/pddtext_CLCD/pstr_CLCD.h"
#include "panel/pcgwarn/pcgwarn_CLCD/pipwarnenl_CLCD.h"	// [遠隔操作Androidパネル]管理No71_カスタム音設定エラー画面対応.
#include "pigacce_CLCD.h"
#include "pipacce_CLCD.h"
#include "pcgacce_CLCD.h"

#include "pidacce_CLCD.h"

#include "piputility_CLCD.h"
#include "../pcgutilg/pcgacce.h"
#include <panel/pcgutil/pcgutil_CLCD/pipaccedef_CLCD.h>
// 音声ガイド機能 追加.
#include <panel/plibh/plibhg/plibhvoice.h>
#include <panel/plibh/plibhg/PNL_VoiceCursorControl.h>
#include <panel/plibh/plibhg/PNL_VoiceRequestBuffer.h>
#include <panel/pddvoice/pddvoiceg/pvoice.h>

// 中止の場合は、TopMenu画面に戻る.
#include "panel/pcgact/pcgact_CLCD2/pigact_pA072002.h"
#include "panel/pcgact/pcgact_CLCD/pigactenl_CLCD.h"

#include "system/sys/APIC_TabletIF.h"
#include "panel/plibh/plibh_CLCD/plibh_CLCD.h"

#include <system/sys/sysg/appInit.h>
#include <panel/plibh/plibh_CLCD2/plibh_CLCD2.h>	// comment 2011/04/25 OSW1-Sky桐島 [MinervaZeus][管理No941-SSD削減対応(1GB⇒512MB)].
#include <panel/plibh/plibh_CLCD2/PNLC_QWSConnector.h>	// comment 2015/12/01 OPS1-Sky稲葉 [ZeusS_ZX0][制御No82] 欧州アクセシビリティ対応 ダブルタップ無効.
//comment [2016/08/01] [HCLT_Thangachamy] [Icarus] [CQ533250] Adding include.
#include <panel/plibh/plibh_CLCD2/PNLC_ScanFaxDataLib.h>
#include <panel/pcgact/pcgact_enl_CLCD2/pigactenl_p80M01.h>
// comment 2017/01/17 SCD2-Sky浜野 [IT5_3.4][1007985] 端末音リスト表示対応.
#include "GFWC_Cmd_SetTabletSoundList.h" 
// comment 2020/11/30 DWS1-Sky延原 【TAS000000003634】リモートパネル使用時にパスワード入力のキー操作がハイライト表示されてしまう.
#include "../../shlib/RecoveryLib/LIBC_RecoveryLib_IF.h"

//================================================================================================
//	定数・型の宣言 .
//================================================================================================
#define PNLD_MusicListLineMax						(uchar)4								// 音声ファイルリスト最大行数.
#define PNLD_ReqMusicFileTimeout					( 3000 * SYSF_sysClkRateGet() / 1000 )	// 3秒のtick変換.
#define PNLD_NotifyMusicFileTimeout					( 1000 * SYSF_sysClkRateGet() / 1000 )	// 1秒のtick変換.
#define PNLD_FileUnmatch							-1										// ファイルの不一致.
#define PNLD_MusicFileStartId						0										// ファイルIDの始まり
// comment 2015/12/01 OPS1-Sky稲葉 [ZeusS_ZX0][制御No82] 欧州アクセシビリティ対応 「2度打ち防止優先設定」キーを追加.
#define PNLD_ExchangeTwiceIntervalNVDToQT			100										// 2度打ち防止間隔変換（NVD→QT）.
#define PNLD_ExchangeTwiceIntervalUI				0.1									// 2度打ち防止間隔変換（UI）.
#define PNLD_TwiceTapStepValue						1										// 2度打ち防止間隔Step値）.
#define PNLD_TwiceTapMaxValue						10										// 2度打ち防止間隔最大値.
#define PNLD_TwiceTapMinValue						1										// 2度打ち防止間隔最小値.


TEXT_ID	uc_AccessibilityEnlMessage = NULL;			// メッセージ欄にて表示しようとしているメッセージ .

// 概要		:ユーザー補助・音設定画面テーブル.
static const TYPS_SoundSettingTable s_SoundSettingTable[] = {
//		音設定画面のタイプ						ブザー音種類						各音設定画面のメッセージ				各音設定画面のタイトル			戻り画面.
	{ PNLE_SoundSettingInputOk,				TYPE_BuzzerKindInputOk,				s_MSGInputConfirmSoundMessage,			s_TTLInputConfirmSound,			&gp_AccessibilityOperationConfirmSound    },	// 入力確認音(108A4011).
	{ PNLE_SoundSettingInputNg,				TYPE_BuzzerKindInputNg,				s_MSGInputCancelSoundMessage,			s_TTLInputCancelSound,			&gp_AccessibilityOperationConfirmSound    },	// 入力無効音(108A4012).
	{ PNLE_SoundSettingCardinalPoint,		TYPE_BuzzerKindCardinalPoint,		s_MSGBasicSoundMessage,					s_TTLBasicSound,				&gp_AccessibilityOperationConfirmSound    },	// 基点音(108A4013).
	{ PNLE_SoundSettingOperationEnd,		TYPE_BuzzerKindOperationEnd,		s_MSGOperationCompleteSoundMessage,		s_TTLOperationCompleteSound,	&gp_AccessibilityNormalCompleteSound      },	// 操作終了音(108A4021).
	{ PNLE_SoundSettingCommunicationEnd,	TYPE_BuzzerKindCommunicationEnd,	s_MSGConnectCompleteSoundMessage,		s_TTLConnectCompleteSound,		&gp_AccessibilityNormalCompleteSound      },	// 通信終了音(108A4022).
	{ PNLE_SoundSettingPreparationEnd,		TYPE_BuzzerKindPreparationEnd,		s_MSGReadyCompleteSoundMessage,			s_TTLReadyCompleteSound,		&gp_AccessibilitySoundSetKeyReceiveDenial },	// 準備完了音(108A4030).
	{ PNLE_SoundSettingWarning1,			TYPE_BuzzerKindWarning1,			s_MSGAttentionSoundSoftLevel1Message,	s_TTLAttentionSoundSoftLevel1,	&gp_AccessibilityAttentionSound           },	// 弱注意音(Level1)(108A4041).
	{ PNLE_SoundSettingWarning2,			TYPE_BuzzerKindWarning2,			s_MSGAttentionSoundSoftLevel2Message,	s_TTLAttentionSoundSoftLevel2,	&gp_AccessibilityAttentionSound           },	// 弱注意音(Level2)(108A4042).
	{ PNLE_SoundSettingWarning3,			TYPE_BuzzerKindWarning3,			s_MSGAttentionSoundSoftLevel3Message,	s_TTLAttentionSoundSoftLevel3,	&gp_AccessibilityAttentionSound           },	// 弱注意音(Level3)(108A4043).
	{ PNLE_SoundSettingError,				TYPE_BuzzerKindError,				s_MSGAttentionSoundStrongMessage,		s_TTLAttentionSoundStrong,		&gp_AccessibilityAttentionSound           },	// 強注意音(108A4044).
};
const short ss_SizeSoundSettingTbl = ( sizeof( s_SoundSettingTable ) / sizeof( TYPS_SoundSettingTable ) );
//================================================================================================
// 概要    :ユーザー補助・音出力設定(408A4100) ：ＯＫ.
//================================================================================================
void PAccessibilitySoundOutputSet::procOk( uchar option )
{
	uchar	uc_SoundOutput = 0;

	if( bl_BatchSoundOutputValue == True ) {						// 一括出力"する"の場合.
		uc_SoundOutput |= TYPE_BuzzerSettingTypeAll;
		if( bl_OperationConfirmSoundOutputValue == True ) {		// 操作確認音出力"する"の場合.
			uc_SoundOutput |= TYPE_BuzzerSettingTypeOpeConfirm;
		}
		if( bl_NormalCompleteSoundOutputValue == True ) {		// 正常終了音出力"する"の場合.
			uc_SoundOutput |= TYPE_BuzzerSettingTypeNormalEnd;
		}
		if( bl_ReadyCompleteSoundOutputValue == True ) {		// 準備完了音出力"する"の場合.
			uc_SoundOutput |= TYPE_BuzzerSettingTypePrepEnd;
		}
		if( bl_AttentionSoundOutputValue == True ) {			// 注意音出力"する"の場合.
			uc_SoundOutput |= TYPE_BuzzerSettingTypeWarning;
		}
	}
	PnlComGetOperationSetting()->setBuzzerTypeSetting( uc_SoundOutput );

	nextGamen = &gp_AccessibilitySoundSetKeyReceiveDenial;		// 108A4000   音設定.
}
//================================================================================================
// 概要    :ユーザー補助・音出力設定(408A4100) ：テンキー押下.
//================================================================================================
void PAccessibilitySoundOutputSet::proc10Key( uchar option )
{
	setBuzzerType( PCGE_BuzzerType_InputNg );
}
//================================================================================================
// 概要    :ユーザー補助・音出力設定(408A4100) ：ボタン押下.
//================================================================================================
void PAccessibilitySoundOutputSet::procButtonOn( uchar option )
{
	switch( option ) {
		case BTNID_AccessibilityBatchSoundOutputKeyOperationOn:		// 一括出力する.
			bl_BatchSoundOutputValue = True;
			bl_OperationConfirmSoundOutputValue = True;
			bl_NormalCompleteSoundOutputValue = True;
			bl_ReadyCompleteSoundOutputValue = True;
			bl_AttentionSoundOutputValue = True;
			break;
		case BTNID_AccessibilityBatchSoundOutputKeyOperationOff:	// 一括出力しない.
			bl_BatchSoundOutputValue = False;
			bl_OperationConfirmSoundOutputValue = False;
			bl_NormalCompleteSoundOutputValue = False;
			bl_ReadyCompleteSoundOutputValue = False;
			bl_AttentionSoundOutputValue = False;
			break;
		case BTNID_AccessibilityOperationConfirmSoundOutputKeyOperationOn:	bl_OperationConfirmSoundOutputValue = True;	break;		// 操作確認音出力する.
		case BTNID_AccessibilityOperationConfirmSoundOutputKeyOperationOff:	bl_OperationConfirmSoundOutputValue = False;	break;		// 操作確認音出力しない.
		case BTNID_AccessibilityNormalCompleteSoundOutputKeyOperationOn:	bl_NormalCompleteSoundOutputValue = True;	break;		// 正常終了音出力する.
		case BTNID_AccessibilityNormalCompleteSoundOutputKeyOperationOff:	bl_NormalCompleteSoundOutputValue = False;	break;		// 正常終了音出力しない.
		case BTNID_AccessibilityReadyCompleteSoundOutputKeyOperationOn:		bl_ReadyCompleteSoundOutputValue = True;		break;		// 準備完了音出力する.
		case BTNID_AccessibilityReadyCompleteSoundOutputKeyOperationOff: 	bl_ReadyCompleteSoundOutputValue = False;	break;		// 準備完了音出力しない.
		case BTNID_AccessibilityAttentionSoundOutputKeyOperationOn:			bl_AttentionSoundOutputValue = True;			break;		// 注意音出力する.
		case BTNID_AccessibilityAttentionSoundOutputKeyOperationOff:		bl_AttentionSoundOutputValue = False;		break;		// 注意音出力しない.
		default:	setBuzzerType( PCGE_BuzzerType_InputNg );		break;
	}
	setupGamen();
}
//================================================================================================
// 概要    :ユーザー補助・音出力設定(408A4100) ：表示画面のスタート.
//================================================================================================
void PAccessibilitySoundOutputSet::startGamen()
{
	uchar	uc_SoundOutput = 0;
	Bool bl_isContinue = bl_isUnder;	// comment 2012/08/07 OPS1-Sky浜野 [MinervaBK][CQ368917] メッセージ拡大画面からの戻り時に設定値を変更しない対応.

	PAccessibilitySoundSetKeyReceiveDenial::startGamen();

	// 下層からの遷移以外.
	if( True != bl_isContinue ) {
		bl_BatchSoundOutputValue = False;					// 一括出力しない.
		bl_OperationConfirmSoundOutputValue = False;		// 操作確認音出力しない.
		bl_NormalCompleteSoundOutputValue = False;			// 正常終了音出力しない.
		bl_ReadyCompleteSoundOutputValue = False;			// 準備完了音出力しない.
		bl_AttentionSoundOutputValue = False;				// 注意音出力しない.

		uc_SoundOutput = PnlComGetOperationSetting()->getBuzzerTypeSetting();

		if( TYPE_BuzzerSettingTypeAll == ( uc_SoundOutput & TYPE_BuzzerSettingTypeAll ) ) {					// 一括出力"する"の場合.
			bl_BatchSoundOutputValue = True;
		}
		if( TYPE_BuzzerSettingTypeOpeConfirm == ( uc_SoundOutput & TYPE_BuzzerSettingTypeOpeConfirm ) ) {		// 操作確認音出力"する"の場合.
			bl_OperationConfirmSoundOutputValue = True;
		}
		if( TYPE_BuzzerSettingTypeNormalEnd == ( uc_SoundOutput & TYPE_BuzzerSettingTypeNormalEnd ) ) {		// 正常終了音出力"する"の場合.
			bl_NormalCompleteSoundOutputValue = True;
		}
		if( TYPE_BuzzerSettingTypePrepEnd == ( uc_SoundOutput & TYPE_BuzzerSettingTypePrepEnd ) ) {			// 準備完了音出力"する"の場合.
			bl_ReadyCompleteSoundOutputValue = True;
		}
		if( TYPE_BuzzerSettingTypeWarning == ( uc_SoundOutput & TYPE_BuzzerSettingTypeWarning ) ) {			// 注意音出力"する"の場合.
			bl_AttentionSoundOutputValue = True;
		}
	}

}
//================================================================================================
// 概要    :ユーザー補助・音出力設定(408A4100) ：表示画面のスタート
//================================================================================================
void PAccessibilitySoundOutputSet::setupGamen( short option )
{
	PAccessibilitySoundSetCom::setupGamen( option );

	// メッセージ欄のセットアップ.
	setupMessageAreaAccessibility( s_MSGENLPerformSoundOutputSetup );

	// 選択されている(一括出力キー)キーの反転/非反転表示.
	Rbt_AccessibilityBatchSoundOutputKey.select( bl_BatchSoundOutputValue == True ? 0 : 1 );

	if( bl_BatchSoundOutputValue == True ) {	// 一括出力をする設定場合.
		Rbt_AccessibilityOperationConfirmSoundOutputKey.activate( True );	// 操作確認音出力 する/しないキー表示.
		Rbt_AccessibilityNormalCompleteSoundOutputKey.activate( True );		// 正常終了音出力 する/しないキー表示.
		Rbt_AccessibilityReadyCompleteSoundOutpuKey.activate( True );		// 準備完了音出力 する/しないキー表示.
		Rbt_AccessibilityAttentionSoundOutputKey.activate( True );			// 注意音出力 する/しないキー表示.
		// 選択されているキーの反転/非反転表示.
		Rbt_AccessibilityOperationConfirmSoundOutputKey.select( bl_OperationConfirmSoundOutputValue == True ? 0 : 1 );
		Rbt_AccessibilityNormalCompleteSoundOutputKey.select( bl_NormalCompleteSoundOutputValue == True ? 0 : 1 );
		Rbt_AccessibilityReadyCompleteSoundOutpuKey.select( bl_ReadyCompleteSoundOutputValue == True ? 0 : 1 );
		Rbt_AccessibilityAttentionSoundOutputKey.select( bl_AttentionSoundOutputValue == True ? 0 : 1 );

	} else {	// 一括出力をしない設定場合.
		Rbt_AccessibilityOperationConfirmSoundOutputKey.activate( False );	// 操作確認音出力 する/しないキー非表示.
		Rbt_AccessibilityNormalCompleteSoundOutputKey.activate( False );	// 正常終了音出力 する/しないキー表示.
		Rbt_AccessibilityReadyCompleteSoundOutpuKey.activate( False );		// 準備完了音出力 する/しないキー表示.
		Rbt_AccessibilityAttentionSoundOutputKey.activate( False );		// 注意音出力 する/しないキー表示.
	}
}
// 概要		:ユーザー補助・操作確認音(108A4010) : ＯＫ.
void PAccessibilityOperationConfirmSound::procOk( uchar option )
{
	nextGamen = &gp_AccessibilitySoundSetKeyReceiveDenial;		// 10894000   音設定.
}
// 概要		:ユーザー補助・操作確認音(108A4010) : 画面移動.
void PAccessibilityOperationConfirmSound::procGoToPage( uchar option )
{
	switch( option ) {
		// 入力確認音キー.
		case 1:
			setSoundSettingType( PNLE_SoundSettingInputOk );
			nextGamen = &gp_AccessibilitySoundVolumeSetting;
			break;
		// 入力無効音キー.
		case 2:
			setSoundSettingType( PNLE_SoundSettingInputNg );
			nextGamen = &gp_AccessibilitySoundVolumeSetting;
			break;
		// 基点音キー.
		case 3:
			setSoundSettingType( PNLE_SoundSettingCardinalPoint );
			nextGamen = &gp_AccessibilitySoundVolumeSetting;
			break;
		default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); break;
	}
}
// 概要		:ユーザー補助・操作確認音(108A4010) : 表示部品のセットアップ.
void PAccessibilityOperationConfirmSound::setupGamen( short option )
{
	PAccessibilitySoundSetCom::setupGamen( option );
	// メッセージ欄のセットアップ.
	setupMessageAreaAccessibility( s_MSGAcceChangeSettingValue );
	// 入力確認音の設定値表示.
	displaySoundSetStatus( &Txt_AccessibilitySoundSetting1, PnlComGetOperationSetting()->getBuzzerVolume( TYPE_BuzzerKindInputOk ) );
	// 入力無効音の設定値表示.
	displaySoundSetStatus( &Txt_AccessibilitySoundSetting2, PnlComGetOperationSetting()->getBuzzerVolume( TYPE_BuzzerKindInputNg ) );
	// 基点音の設定値表示.
	displaySoundSetStatus( &Txt_AccessibilitySoundSetting3, PnlComGetOperationSetting()->getBuzzerVolume( TYPE_BuzzerKindCardinalPoint ) );
}
// 概要		:ユーザー補助・正常終了音(108A4020) : ＯＫ.
void PAccessibilityNormalCompleteSound::procOk( uchar option )
{
	nextGamen = &gp_AccessibilitySoundSetKeyReceiveDenial;		// 10894000   音設定.
}
// 概要		:ユーザー補助・正常終了音(108A4020) : 画面移動.
void PAccessibilityNormalCompleteSound::procGoToPage( uchar option )
{
	switch( option ) {
		// 操作終了音キー.
		case 1:
			setSoundSettingType( PNLE_SoundSettingOperationEnd );
			nextGamen = &gp_AccessibilitySoundVolumeSetting;
			break;
		// 通信終了音キー.
		case 2:
			setSoundSettingType( PNLE_SoundSettingCommunicationEnd );
			nextGamen = &gp_AccessibilitySoundVolumeSetting;
			break;
		default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); break;
	}
}
// 概要		:ユーザー補助・正常終了音(108A4020) : 表示部品のセットアップ.
void PAccessibilityNormalCompleteSound::setupGamen( short option )
{
	PAccessibilitySoundSetCom::setupGamen( option );
	// メッセージ欄のセットアップ.
	setupMessageAreaAccessibility( s_MSGAcceChangeSettingValue );
	// 操作終了音の設定値表示.
	displaySoundSetStatus( &Txt_AccessibilitySoundSetting1, PnlComGetOperationSetting()->getBuzzerVolume( TYPE_BuzzerKindOperationEnd ) );
	// 通信終了音の設定値表示.
	displaySoundSetStatus( &Txt_AccessibilitySoundSetting2, PnlComGetOperationSetting()->getBuzzerVolume( TYPE_BuzzerKindCommunicationEnd ) );
}
// 概要		:ユーザー補助・注意音(108A4040) : ＯＫ.
void PAccessibilityAttentionSound::procOk( uchar option )
{
	nextGamen = &gp_AccessibilitySoundSetKeyReceiveDenial;		// 10894000   音設定.
}
// 概要		:ユーザー補助・注意音(108A4040) : 画面移動.
void PAccessibilityAttentionSound::procGoToPage( uchar option )
{
	switch( option ) {
		// 弱注意音(Level1)キー.
		case 1:
			setSoundSettingType( PNLE_SoundSettingWarning1 );
			nextGamen = &gp_AccessibilitySoundVolumeSetting;
			break;
		// 弱注意音(Level2)キー.
		case 2:
			setSoundSettingType( PNLE_SoundSettingWarning2 );
			nextGamen = &gp_AccessibilitySoundVolumeSetting;
			break;
		// 弱注意音(Level3)キー.
		case 3:
			setSoundSettingType( PNLE_SoundSettingWarning3 );
			nextGamen = &gp_AccessibilitySoundVolumeSetting;
			break;
		// 強注意音キー.
		case 4:
			setSoundSettingType( PNLE_SoundSettingError );
			nextGamen = &gp_AccessibilitySoundVolumeSetting;
			break;
		default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); break;
	}
}
// 概要		:ユーザー補助・注意音(108A4040) : 表示部品のセットアップ.
void PAccessibilityAttentionSound::setupGamen( short option )
{
	PAccessibilitySoundSetCom::setupGamen( option );
	// メッセージ欄のセットアップ.
	setupMessageAreaAccessibility( s_MSGAcceChangeSettingValue );
	// 弱注意音(Level1)の設定値表示.
	displaySoundSetStatus( &Txt_AccessibilitySoundSetting1, PnlComGetOperationSetting()->getBuzzerVolume( TYPE_BuzzerKindWarning1 ) );
	// 弱注意音(Level2)の設定値表示.
	displaySoundSetStatus( &Txt_AccessibilitySoundSetting2, PnlComGetOperationSetting()->getBuzzerVolume( TYPE_BuzzerKindWarning2 ) );
	// 弱注意音(Level3)の設定値表示.
	displaySoundSetStatus( &Txt_AccessibilitySoundSetting3, PnlComGetOperationSetting()->getBuzzerVolume( TYPE_BuzzerKindWarning3 ) );
	// 強注意音の設定値表示.
	displaySoundSetStatus( &Txt_AccessibilitySoundSetting4, PnlComGetOperationSetting()->getBuzzerVolume( TYPE_BuzzerKindError ) );
}
// 概要		:ユーザー補助・音設定共通画面 : ＯＫ.
void PAccessibilitySoundVolumeSetting::procOk( uchar option )
{
	// ＣｏｍｍｏｎＡＰＩへ設定する.
	TYPE_BuzzerVolume e_BuzzerVolume = TYPE_BuzzerVolumeOff;
	switch( ss_SoundVolumeSetting ) {
		case 0: e_BuzzerVolume = TYPE_BuzzerVolumeOff; break;		// しない.
		case 1: e_BuzzerVolume = TYPE_BuzzerVolumeLevel1; break;		// 小.
		case 2: e_BuzzerVolume = TYPE_BuzzerVolumeLevel2; break;		// 中.
		case 3: e_BuzzerVolume = TYPE_BuzzerVolumeLevel3; break;		// 大.
		default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); break;
	}
	PnlComGetOperationSetting()->setBuzzerVolume( s_SoundSettingNowGamen->e_BuzzerKind, e_BuzzerVolume );
	// 画面遷移.
	nextGamen = s_SoundSettingNowGamen->c_TrunSoundSetGamen;
}
// 概要		:ユーザー補助・音設定共通画面 : ボタン（押された）.
void PAccessibilitySoundVolumeSetting::procButtonOn( uchar option )
{
	switch( option ) {
		case 1: ss_SoundVolumeSetting = 1; break;		// 小キー.
		case 2: ss_SoundVolumeSetting = 2; break;		// 中キー.
		case 3: ss_SoundVolumeSetting = 3; break;		// 大キー.
		case 10: ss_SoundVolumeSetting = 2; break;		// するキー.			// ブザー音量「中」に設定する.
		case 11: ss_SoundVolumeSetting = 0; break;		// しないキー.
		default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); break;
	}
	setupGamen();
}
// 概要		:ユーザー補助・音設定共通画面 : 表示部品のスタート.
void PAccessibilitySoundVolumeSetting::startGamen()
{
	Bool bl_isContinue = bl_isUnder;	// comment 2012/08/07 OPS1-Sky浜野 [MinervaBK][CQ368917] メッセージ拡大画面からの戻り時に設定値を変更しない対応.

	PAccessibilitySoundSetCom::startGamen();
	// 現在の設定画面を取得する.
	PNLE_SoundSettingType e_SoundSetNowGamenType = getSoundSettingType();
	for( register ushort i = 0; i < ss_SizeSoundSettingTbl; i++ ) {
		if( e_SoundSetNowGamenType == s_SoundSettingTable[ i ].e_SoundSettingType ) {
			s_SoundSettingNowGamen = &s_SoundSettingTable[ i ];
			break;
		}
	}
	// 下層からの遷移以外.
	if( True != bl_isContinue ) {
		// 現在設定されている音設定画面のブザー音量の取得.
		TYPE_BuzzerVolume e_BuzzerVolume = PnlComGetOperationSetting()->getBuzzerVolume( s_SoundSettingNowGamen->e_BuzzerKind );
		switch( e_BuzzerVolume ) {
			case 0: ss_SoundVolumeSetting = 0; break;		// しない.
			case 1: ss_SoundVolumeSetting = 1; break;		// 小.
			case 2: ss_SoundVolumeSetting = 2; break;		// 中.
			case 3: ss_SoundVolumeSetting = 3; break;		// 大.
			default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); break;
		}
	}
}
// 概要		:ユーザー補助・音設定共通画面 : 表示部品のセットアップ.
void PAccessibilitySoundVolumeSetting::setupGamen( short option )
{
	PAccessibilitySoundSetCom::setupGamen( option );
	// メッセージ欄のセットアップ.
	setupMessageAreaAccessibility( s_SoundSettingNowGamen->str_SoundSettingMessage );
	// タイトル欄の表示切替処理.
	Txt_AccessibilitySoundVolumeSetting.changeText( s_SoundSettingNowGamen->str_SoundSettingTitle );
	// ブザー音量キーの反転表示.
	Rtb_AccessibilitySoundVolumeSetting.select( static_cast<short>( ss_SoundVolumeSetting - 1 ) );
	// しないキーが設定されている時は、ブザー音量キーをグレーアウトにする.
	Rtb_AccessibilitySoundVolumeSetting.activate( ss_SoundVolumeSetting == 0 ? False : True );
	// する／しないキー反転表示.
	Rtb_AccessibilityDo_NotDo.select( ss_SoundVolumeSetting == 0 ? 1 : 0 );
}
// 概要		:拡大メッセージ画面(ユニバーサル設定用): タブ押下.
void PEnlargeMessageGamenAcce::procIndexButton( uchar option )
{
	// 前画面に戻る.
	nextGamen = turnGamenEnlMessage;
	bl_isUnder = True;
}
// 概要		:拡大メッセージ画面(ユニバーサル設定用): 表示画面のセットアップ.
void PEnlargeMessageGamenAcce::setupGamen( short option )
{
	PAccessibilitycom::setupGamen( option );
	Txt_EnlargeMessageAcce.changeText( uc_AccessibilityEnlMessage );
}
/*================================================================================================
	Mosel機能拡張2
=================================================================================================*/
//================================================================================================
// 概要		:トップメニュー : メニューテーブル作成.
//================================================================================================

//================================================================================================
// 概要		:ユーザー補助・トップメニューへの遷移共通 : ＯＫ .
//================================================================================================
void PAccessibilityTopMenuCom::procOk( uchar option )
{
	bl_isUnder = True;
	nextGamen = &gp_AccessibilityTopMenu;				// ユーザー補助・トップメニュー.
}
//================================================================================================
// 概要		:ユーザー補助・トップメニューへの遷移共通 : 画面開始.
//================================================================================================
void PAccessibilityTopMenuCom::startGamen()
{
	PAccessibilitycom::startGamen();	// 親クラスの処理を呼ぶ.

	bl_isUnder = False;
}

// comment 2015/12/01 OPS1-Sky稲葉 [ZeusS_ZX0][制御No82] 欧州アクセシビリティ対応 「K4NA300000_2度打ち防止優先設定」画面を追加.
//================================================================================================
// 概要		:ユーザー補助・トップメニューへの遷移共通 : ダブルタップ間隔時間取得.
//================================================================================================
short PAccessibilityTopMenuCom::getIntervalTime( short ss_Level )
{
	ushort us_IntervalTime = PNLD_ClickIntervalNormal;

	switch( ss_Level ) {
		case TYPE_DoubleTapIntervalModeSlow:		// おそい.
			us_IntervalTime = static_cast<ushort>( us_IntervalTime + ( PNLD_ClickIntervalSlow * 2 ) );
			break;
		case TYPE_DoubleTapIntervalModeLittleSlow:	// ややおそい.
			us_IntervalTime = static_cast<ushort>( us_IntervalTime + ( PNLD_ClickIntervalSlow ) );
			break;
		case TYPE_DoubleTapIntervalModeNormal:		// ふつう.
			break;
		case TYPE_DoubleTapIntervalModeLittleFast:	// ややはやい.
			us_IntervalTime = static_cast<ushort>( us_IntervalTime - ( PNLD_ClickIntervalFast ) );
			break;
		case TYPE_DoubleTapIntervalModeFast:		// はやい.
			us_IntervalTime = static_cast<ushort>( us_IntervalTime - ( PNLD_ClickIntervalFast * 2 ) );
			break;
		default:
			break;
	}

	return us_IntervalTime;
}
	
#include <panel/pcgcom/pcgcom_CLCD/pcgcom_CLCD.h>
//================================================================================================
//概要    :タッチパネル輝度調整：ボタン（押された）.
//================================================================================================
void PAccessibilityBackLightBrightness::procButtonOn( uchar option )
{
	Bool bl_isUp = ( BTNID_AccessibilityBackLightBrightnessDown == option ) ? False : True;
	// アイコンキーがあるため、キーがグレイアウトされている条件は受付け禁止.
	if( True == isBackLightBrightnessLimit( bl_isUp ) ) {
		setBackLightBrightness( bl_isUp );
	} else {
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
	setupGamen();
}
//================================================================================================
//概要    :タッチパネル輝度調整：表示画面のセットアップ.
//================================================================================================
void PAccessibilityBackLightBrightness::setupGamen( short option )
{
	PAccessibilityTopMenuCom::setupGamen();

	// メッセージ欄のセットアップ.
	setupMessageAreaAccessibility( s_MSGTouchPanelAdjustBrightness );

	Key_AccessibilityBackLightBrightnessDown.activate( isBackLightBrightnessLimit( False ) );
	Key_AccessibilityBackLightBrightnessUp.activate( isBackLightBrightnessLimit( True ) );
}
// 音声ガイド機能追加.
//================================================================================================
// 概要		:108A8000 音声ガイド設定 開始画面.
//================================================================================================
void PAccessibilityVoiceGuideSetting::startGamen()
{
	Bool bl_isContinue = bl_isUnder;	// comment 2012/08/07 OPS1-Sky浜野 [MinervaBK][CQ368917] メッセージ拡大画面からの戻り時に設定値を変更しない対応.

	PAccessibilityTopMenuCom::startGamen();//親クラスのStartGamenに移動.
	//する･しないの現在の情報を取得.
	b_VoiceGuideOnOff = PnlComGetOperationSetting()->getVoiceVocalize();
	// 下層からの遷移以外.
	if( True != bl_isContinue ) {
		b_VoiceGuideOnOff = PnlComGetOperationSetting()->getVoiceVocalize();
	}
}
//================================================================================================
// 概要		:108A8000 音声ガイド設定 表示画面のセットアップ.
//================================================================================================
void PAccessibilityVoiceGuideSetting::setupGamen( short option )
{
	short ss_GuideStatus;				// 音声ガイド情報取得.
	TEXT_ID puc_VolumeStatus = NULL;	// 音量の文字列取得.
	TEXT_ID puc_SpeedStatus = NULL;		// 音声速度の文字列取得.

	PAccessibilityTopMenuCom::setupGamen();	// 親クラスに移動.
	setupMessageAreaAccessibility( s_MSGVoiceGuide );	// メッセージ欄のセットアップ.


	//音量の現在の情報を取得.
	ss_GuideStatus = PnlComGetOperationSetting()->getVoiceVolume();
	switch( ss_GuideStatus ) {
		case TYPE_VoiceVolume1: puc_VolumeStatus = s_ITMVolumeLevel_1; break;	//１.
		case TYPE_VoiceVolume2: puc_VolumeStatus = s_ITMVolumeLevel_2; break;	//２.
		case TYPE_VoiceVolume3: puc_VolumeStatus = s_ITMVolumeLevel_3; break;	//３.
		case TYPE_VoiceVolume4: puc_VolumeStatus = s_ITMVolumeLevel_4; break;	//４.
		case TYPE_VoiceVolume5: puc_VolumeStatus = s_ITMVolumeLevel_5; break;	//５.
		case TYPE_VoiceVolume6: puc_VolumeStatus = s_ITMVolumeLevel_6; break;	//６.
		case TYPE_VoiceVolume7: puc_VolumeStatus = s_ITMVolumeLevel_7; break;	//７.
		case TYPE_VoiceVolume8: puc_VolumeStatus = s_ITMVolumeLevel_8; break;	//８.
		case TYPE_VoiceVolume9: puc_VolumeStatus = s_ITMVolumeLevel_9; break;	//９.
	}
	//音量の現在の情報を表示.
	Bak_EnlAccessibilityVolumeStatus.changeText( puc_VolumeStatus );

	//音声速度の現在の情報を取得.
	ss_GuideStatus = PnlComGetOperationSetting()->getVoiceSpeed();
	switch( ss_GuideStatus ) {
		case TYPE_VoiceSpeedSlow: puc_SpeedStatus = s_KEYVoiceSpeedSlow; break;		//遅い.
		case TYPE_VoiceSpeedUsual: puc_SpeedStatus = s_KEYVoiceSpeedNomal; break;	//普通.
		case TYPE_VoiceSpeedQuick: puc_SpeedStatus = s_KEYVoiceSpeedQuick; break;	//速い.
	}
	//音声速度の現在の情報を表示.
	Btx_EnlAccessibilityVoiceSpeedStatus.changeText( puc_SpeedStatus );

	//する/しないのキーを反転表示.
	Rtb_AccessibilityDo_NotDo.select( b_VoiceGuideOnOff == False ? 1 : 0 );
	//するの場合は、キー表示、しないの場合はグレーアウト処理を行う.
	Grp_AccessibilityVoiceGuideSettingStatus.activate( b_VoiceGuideOnOff == False ? False : True );

}
//================================================================================================
// 概要		:108A8000 音声ガイド設定 ボタン（押された）.
//================================================================================================
void PAccessibilityVoiceGuideSetting::procButtonOn( uchar option )
{
	switch( option ) {
		case 10: b_VoiceGuideOnOff = True;	break;	//する.
		case 11: b_VoiceGuideOnOff = False;	break;	//しない.
		default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); break;
	}
	setupGamen();
}
//================================================================================================
// 概要		:108A8000 音声ガイド設定 画面移動.
//================================================================================================
void PAccessibilityVoiceGuideSetting::procGoToPage( uchar option )
{
	switch( option ) {
		case 2: nextGamen = &gp_AccessibilityVolume;			break;	// 音量.
		case 3: nextGamen = &gp_AccessibilityVoiceSpeed;		break;	// 音声速度.
		default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); break;
	}
}
//================================================================================================
// 概要		:108A8000 音声ガイド設定 設定終了.
//================================================================================================
void PAccessibilityVoiceGuideSetting::procOk( uchar option )
{
	PnlComGetOperationSetting()->setVoiceVocalize( b_VoiceGuideOnOff );	//音声ガイドする／しない確定.
	PAccessibilityTopMenuCom::procOk( option );

}


//================================================================================================
// 概要		:108A8010 声の種類 開始画面.
//================================================================================================
void PAccessibilityVoiceCharacter::startGamen()
{
	PAccessibilityTopMenuCom::startGamen();//親クラスのStartGamenに移動.
	ss_VoiceCharacter = PnlComGetOperationSetting()->getVoiceCharacter();
}
//================================================================================================
// 画面開始音声IDの格納.
//================================================================================================
void PAccessibilityVoiceCharacter::addReqStartVoiceID()
{
	Bool b_StartVoice = False;

	// 音声非対応画面の音声ID格納
	if( addReqNotVoiceGamen() ) { return; }
	// 画面説明音声IDを格納
	addReqGamenVoiceID();
	// メッセージエリア音声IDを格納
	addReqMessageAreaVoiceID();
	// メッセージエリア音声の重複を排除
	PNLC_VoiceRequestBuffer::newInstance()->adjustMsgAreaRequest();

	PNLC_VoiceRequestBuffer::newInstance()->sendRequest();// 再生要求を発行.

	// 現在のカレントパーツ音声IDを格納
	::reqVoicePartsVID( this, PNLE_VoicePartsKindKeyCur );

	if( ss_VoiceCharacter == TYPE_VoiceKindMale ) {
		b_StartVoice = True;
		PnlComGetOperationSetting()->setVoiceCharacter( TYPE_VoiceKindFemale );	// カーソル位置のデフォルトである『女性』をセットする.
	}

	PNLC_VoiceRequestBuffer::newInstance()->sendRequest();// 再生要求を発行.

	if( b_StartVoice == True ) {
		PnlComGetOperationSetting()->setVoiceCharacter( ss_VoiceCharacter );	// 元に戻す.
	}
}
//================================================================================================
// 概要		:108A8010 声の種類 表示画面のセットアップ.
//================================================================================================
void PAccessibilityVoiceCharacter::setupGamen( short option )
{
	PAccessibilityTopMenuCom::setupGamen();
	setupMessageAreaAccessibility( s_MSGVoiceCharcter );	// メッセージ欄のセットアップ.
	Rtb_AccessibilityVoiceCharacterSelect.select( ss_VoiceCharacter );// 男性／女性 キーを反転表示.
}
//================================================================================================
// 概要		:108A8010 声の種類 ボタン（押された）.
//================================================================================================
void PAccessibilityVoiceCharacter::procButtonOn( uchar option )
{
	switch( option ) {
		case TYPE_VoiceKindMale:	//男性.
		case TYPE_VoiceKindFemale:	//女性.
			ss_VoiceCharacter = ( TYPE_VoiceKind )option;
			break;
		default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); break;
	}
	setupGamen();
}
//================================================================================================
// 概要		:108A8010 声の種類 ボタン（押された）音声再生.
//================================================================================================
void PAccessibilityVoiceCharacter::procButtonOnVoice( uchar option )
{
	ss_VoiceCharacterBackup = PnlComGetOperationSetting()->getVoiceCharacter();//バックアップを取る.

	::setVoiceRequestFlag( True );	// 再生要求設定に現在の再生中止を設定.
	PNLE_VoicePartsKind eVpartsKind = ::getVoicePushedPartsKind();	// パーツの音声種別を取得.

	procButtonOn( option );//通常のボタンの動作へ移動.

	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Debug, "VoiceCharacter[%zd]",(intptr_t)ss_VoiceCharacter ); }
	PnlComGetOperationSetting()->setVoiceCharacter( ss_VoiceCharacter );	// CommonAPIの中を一時的に更新する.

	::showVoiceCursor();	// 音声再生する前にカーソルの描画を更新する.


	// カレント音声取得処理.
	::reqVoicePartsVID( this, eVpartsKind );
	// 画面が更新されているのでメッセージエリアを取得.
	::reqVoiceMsgAreaVID( this );

	PNLC_VoiceRequestBuffer::newInstance()->sendRequest();// 再生要求を発行.

	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Debug, "VoiceCharacter[%zd]", (intptr_t)ss_VoiceCharacterBackup ); }
	PnlComGetOperationSetting()->setVoiceCharacter( ss_VoiceCharacterBackup );	// CommonAPIの中を元に戻す.
}

//================================================================================================
// 概要		:108A8010 声の種類 設定終了.
//================================================================================================
void PAccessibilityVoiceCharacter::procOk( uchar option )
{
	PnlComGetOperationSetting()->setVoiceCharacter( ss_VoiceCharacter );	//声の種類確定.
	nextGamen = &gp_AccessibilityVoiceGuideSetting;
}
//================================================================================================
// 概要		: 声の種類 テンキー４の処理(音声専用) .
//================================================================================================
void PAccessibilityVoiceCharacter::vproc4Key()
{
	ss_VoiceCharacterBackup = PnlComGetOperationSetting()->getVoiceCharacter();//バックアップを取る.

	// カーソルを前へ移動.
	PNLC_VoiceCursorControl* pcVcc = PNLC_VoiceCursorControl::newInstance();
	short ssCurrent = pcVcc->getCurrent();
	if( ssCurrent < 0 ) { return; } // カーソル非表示.

	// カーソルを前に移動します.
	pcVcc->decCursor();

	// 現在位置の音声再生.
	if( pcVcc->getCurrent() == ssCurrent ) { return; }	// 位置の変化なしの場合は何もしない.

	switch( pcVcc->getCurrent() ) {
		case 0:		e_VoiceCursor = TYPE_VoiceKindFemale;		break;	// カーソルの位置が女性のとき.
		case 1:		e_VoiceCursor = TYPE_VoiceKindMale;			break;	// カーソルの位置が男性のとき.
		default:	e_VoiceCursor = ss_VoiceCharacterBackup;	break;	// カーソルの位置がOKのとき、現状の声の種類をセットする.
	}

	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Debug, "VoiceCharacter[%zd]", (intptr_t)e_VoiceCursor ); }
	PnlComGetOperationSetting()->setVoiceCharacter( e_VoiceCursor );	// CommonAPIの中を一時的に更新する.

	// 現在位置のキー選択の音声IDリストを再生要求バッファへ格納する.
	addReqPartsVoiceID( pcVcc->getCurrent(), PNLE_VoicePartsKindKeyCur );
	// 声の種類を元に戻す前に再生要求を投げる.
	PNLC_VoiceRequestBuffer::newInstance()->sendRequest();// 再生要求を発行

	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Debug, "VoiceCharacter[%zd]", (intptr_t)ss_VoiceCharacterBackup ); }
	PnlComGetOperationSetting()->setVoiceCharacter( ss_VoiceCharacterBackup );	// CommonAPIの中を元に戻す.
}
//================================================================================================
// 概要		: 声の種類 テンキー６の処理(音声専用) .
//================================================================================================
void PAccessibilityVoiceCharacter::vproc6Key()
{
	ss_VoiceCharacterBackup = PnlComGetOperationSetting()->getVoiceCharacter();//バックアップを取る.

	// カーソルを次へ移動.
	PNLC_VoiceCursorControl* pcVcc = PNLC_VoiceCursorControl::newInstance();
	short ssCurrent = pcVcc->getCurrent();
	if( ssCurrent < 0 ) { return; } // カーソル非表示.

	// カーソルを次へ移動します.
	pcVcc->incCursor();

	// 現在位置の音声再生.
	if( pcVcc->getCurrent() == ssCurrent ) { return; }	// 位置の変化なしの場合は何もしない.

	switch( pcVcc->getCurrent() ) {
		case 0:		e_VoiceCursor = TYPE_VoiceKindFemale;		break;	// カーソルの位置が女性のとき.
		case 1:		e_VoiceCursor = TYPE_VoiceKindMale;			break;	// カーソルの位置が男性のとき.
		default:	e_VoiceCursor = ss_VoiceCharacterBackup;	break;	// カーソルの位置がOKのとき、現状の声の種類をセットする.
	}

	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Debug, "VoiceCharacter[%zd]", (intptr_t)e_VoiceCursor ); }
	PnlComGetOperationSetting()->setVoiceCharacter( e_VoiceCursor );	// CommonAPIの中を一時的に更新する.

	// 現在位置のキー選択の音声IDリストを再生要求バッファへ格納する.
	addReqPartsVoiceID( pcVcc->getCurrent(), PNLE_VoicePartsKindKeyCur );
	// 声の種類を元に戻す前に再生要求を投げる.
	PNLC_VoiceRequestBuffer::newInstance()->sendRequest();// 再生要求を発行

	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Debug, "VoiceCharacter[%zd]", (intptr_t)ss_VoiceCharacterBackup ); }
	PnlComGetOperationSetting()->setVoiceCharacter( ss_VoiceCharacterBackup );	// CommonAPIの中を元に戻す.
}

//================================================================================================
// 概要		:108A8020 音量 開始画面.
//================================================================================================
void PAccessibilityVolume::startGamen()
{
	Bool bl_isContinue = bl_isUnder;	// comment 2012/08/07 OPS1-Sky浜野 [MinervaBK][CQ368917] メッセージ拡大画面からの戻り時に設定値を変更しない対応.

	PAccessibilityTopMenuCom::startGamen();	//親クラスのStartGamenに移動.
	// 下層からの遷移以外.
	if( True != bl_isContinue ) {
		ss_Volume = PnlComGetOperationSetting()->getVoiceVolume();
	}
	b_VolumeSetEnd = False;		//音量未確定.
	ss_VolumeBackup = PnlComGetOperationSetting()->getVoiceVolume();//バックアップを取る.
}
//================================================================================================
// 概要		:108A8020 音量 表示画面のセットアップ.
//================================================================================================
void PAccessibilityVolume::setupGamen( short option )
{
	PAccessibilityTopMenuCom::setupGamen();
	setupMessageAreaAccessibility( s_MSGVoiceVolume );	// メッセージ欄のセットアップ.
	Rot_EnlAccessibilityVolume.changeGraphic( ss_Volume ); //音量レベルを表示.
}
//================================================================================================
// 概要		:108A8020 音量 ボタン（押された）.
//================================================================================================
void PAccessibilityVolume::procButtonOn( uchar option )
{
	b_VolumeError = False;//エラーフラグ初期化.

	switch( option ) {
		case 0:	// ちいさく.
			if( ss_Volume > 1 ) {	// 音量レベルが１でなければ音量ダウン.
				ss_Volume = ( TYPE_VoiceVolume )( ss_Volume - 1 );
			} else {	// 音量レベルが１の時は報知音を鳴らす.
				setBuzzerType( PCGE_BuzzerType_InputNg );
				b_VolumeError = True;
			}
			break;

		case 1:	// 大きく.
			if( ss_Volume < 9 ) {	// 音量レベルが９でなきときは音量アップ.
				ss_Volume = ( TYPE_VoiceVolume )( ss_Volume + 1 );
			} else {	// 音量レベルが９の時は報知音を鳴らす.
				setBuzzerType( PCGE_BuzzerType_InputNg );
				b_VolumeError = True;
			}
			break;

		default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );  break;
	}
// Commonへ更新するタイミング修正.
	PnlComGetOperationSetting()->setVoiceVolume( ss_Volume );	// CommonAPIの中を一時的に更新する.
	setupGamen();
}
//================================================================================================
// 概要		:108A8020 音量 ボタン（押された）音声再生.
//================================================================================================
void PAccessibilityVolume::procButtonOnVoice( uchar option )
{

	// 再生要求設定に現在の再生中止を設定.
	::setVoiceRequestFlag( True );
	// パーツの音声種別を取得.
	PNLE_VoicePartsKind eVpartsKind = ::getVoicePushedPartsKind();

	procButtonOn( option );//通常のボタンの動作へ移動.

	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Debug, "Volume[%zd]", (intptr_t)ss_Volume ); }
// Commonへ更新するタイミング修正.

	::showVoiceCursor();	// 音声再生する前にカーソルの描画を更新する.

	// カレント音声取得処理.
	::reqVoicePartsVID( this, eVpartsKind );
	// 画面が更新されているのでメッセージエリアを取得.
	::reqVoiceMsgAreaVID( this );

	// エラーフラグの状況によって変える.
	if( b_VolumeError == True ) {
		PNLC_VoiceRequestBuffer::newInstance()->clearRequest();// 再生要求バッファのクリア.
	} else {
		PNLC_VoiceRequestBuffer::newInstance()->sendRequest();// 再生要求を発行.
	}
	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Debug, "Volume[%zd]", (intptr_t)ss_VolumeBackup ); }
}
//================================================================================================
// 概要		:108A8020 音量 ボタン（押し続け、速く変化）キーリピート.
//================================================================================================
void PAccessibilityVolume::procButtonStillOnFast( uchar option ) {
	switch( option ) {
		case 0:	// 小さく.
		case 1:	// 大きく.
			procButtonOn( option );
			::showVoiceCursor();	// 音声再生する前にカーソルの描画を更新する.
			break;
		default: break;
	}
}
//================================================================================================
// 概要		:108A8020 音量 ダウン.
//================================================================================================
void PAccessibilityVolume::vprocAster( )
{

	// 再生要求設定に現在の再生中止を設定.
	::setVoiceRequestFlag( True );

	procButtonOn( 0 );//通常のボタンの動作へ移動.

	PnlComGetOperationSetting()->setVoiceVolume( ss_Volume );	// CommonAPIの中を一時的に更新する.

	// エラーフラグの状況によって変える.
	if( b_VolumeError == True ) {
		PNLC_VoiceRequestBuffer::newInstance()->clearRequest();// 再生要求バッファのクリア.
	} else {
		ushort	usVID = 0;			// 音声読み上げ用.
		usVID = v_KEYToSmall_Sel;	// 音声再生（*キー押下時）.
		reqVoicePlay( &usVID, 1, True, NULL );
	}
}
//================================================================================================
// 概要		:108A8020 音量 アップ.
//================================================================================================
void PAccessibilityVolume::vprocShrp( )
{

	// 再生要求設定に現在の再生中止を設定.
	::setVoiceRequestFlag( True );

	procButtonOn( 1 );//通常のボタンの動作へ移動.

	PnlComGetOperationSetting()->setVoiceVolume( ss_Volume );	// CommonAPIの中を一時的に更新する.

	// エラーフラグの状況によって変える.
	if( b_VolumeError == True ) {
		PNLC_VoiceRequestBuffer::newInstance()->clearRequest();	// 再生要求バッファのクリア.
	} else {
		ushort	usVID = 0;			// 音声読み上げ用.
		usVID = v_KEYToBig_Sel;	// 音声再生（#キー押下時）.
		reqVoicePlay( &usVID, 1, True, NULL );
	}
}
//================================================================================================
// 概要		:108A8020 音量 設定終了.
//================================================================================================
void PAccessibilityVolume::procOk( uchar option )
{
	b_VolumeSetEnd = True;	//音量確定.
	nextGamen = &gp_AccessibilityVoiceGuideSetting;
}
//================================================================================================
// 概要		:108A8020 音量 画面終了処理.
//================================================================================================
void PAccessibilityVolume::endGamen( )
{
	PAccessibilityTopMenuCom::endGamen();
	if( !b_VolumeSetEnd ) { PnlComGetOperationSetting()->setVoiceVolume( ss_VolumeBackup ); }	// CommonAPIの中を元に戻す.
}
//================================================================================================
// 概要		:108A8030 音声速度 開始画面.
//================================================================================================
void PAccessibilityVoiceSpeed::startGamen()
{
	Bool bl_isContinue = bl_isUnder;	// comment 2012/08/07 OPS1-Sky浜野 [MinervaBK][CQ368917] メッセージ拡大画面からの戻り時に設定値を変更しない対応.

	PAccessibilityTopMenuCom::startGamen();//親クラスのStartGamenに移動.
	// 下層からの遷移以外.
	if( True != bl_isContinue ) {
		ss_VoiceSpeed = PnlComGetOperationSetting()->getVoiceSpeed();
	}
}
//================================================================================================
// 画面開始音声IDの格納.
//================================================================================================
void PAccessibilityVoiceSpeed::addReqStartVoiceID()
{
	Bool b_StartVoice = False;

	// 音声非対応画面の音声ID格納
	if( addReqNotVoiceGamen() ) { return; }
	// 画面説明音声IDを格納
	addReqGamenVoiceID();
	// メッセージエリア音声IDを格納
	addReqMessageAreaVoiceID();
	// メッセージエリア音声の重複を排除
	PNLC_VoiceRequestBuffer::newInstance()->adjustMsgAreaRequest();

	PNLC_VoiceRequestBuffer::newInstance()->sendRequest();// 再生要求を発行.

	// 現在のカレントパーツ音声IDを格納
	::reqVoicePartsVID( this, PNLE_VoicePartsKindKeyCur );

	TYPE_VoiceSpeed ss_VoiceSpeedBack = PnlComGetOperationSetting()->getVoiceSpeed();

	if( ss_VoiceSpeedBack != TYPE_VoiceSpeedSlow ) {
		b_StartVoice = True;
		PnlComGetOperationSetting()->setVoiceSpeed( TYPE_VoiceSpeedSlow );	// カーソルのデフォルト位置である『遅い』をセットする.
	}

	PNLC_VoiceRequestBuffer::newInstance()->sendRequest();// 再生要求を発行.

	if( b_StartVoice == True ) {
		PnlComGetOperationSetting()->setVoiceSpeed( ss_VoiceSpeedBack );	// 元の速度に戻す.
	}
}
//================================================================================================
// 概要		:108A8030 音声速度 表示画面のセットアップ.
//================================================================================================
void PAccessibilityVoiceSpeed::setupGamen( short option )
{
	PAccessibilityTopMenuCom::setupGamen();
	setupMessageAreaAccessibility( s_MSGVoiceSpeed );	// メッセージ欄のセットアップ.
	Rtb_AccessibilityVoiceSpeedSelect.select( ss_VoiceSpeed );	//音声速度キー反転.
}
//================================================================================================
// 概要		:108A8030 音声速度 ボタン（押された）.
//================================================================================================
void PAccessibilityVoiceSpeed::procButtonOn( uchar option )
{
	switch( option ) {
		case TYPE_VoiceSpeedSlow:	//遅い.
		case TYPE_VoiceSpeedUsual:	//普通.
		case TYPE_VoiceSpeedQuick:	//速い.
			ss_VoiceSpeed = ( TYPE_VoiceSpeed )option;
			break;
		default: LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); break;
	}
	setupGamen();
}
//================================================================================================
// 概要		:108A8030 音声速度 ボタン（押された）音声再生.
//================================================================================================
void PAccessibilityVoiceSpeed::procButtonOnVoice( uchar option )
{
	ss_VoiceSpeedBackup = PnlComGetOperationSetting()->getVoiceSpeed();//バックアップを取る.

	// 再生要求設定に現在の再生中止を設定.
	::setVoiceRequestFlag( True );
	// パーツの音声種別を取得.
	PNLE_VoicePartsKind eVpartsKind = ::getVoicePushedPartsKind();

	procButtonOn( option );	//通常のボタンの動作へ移動.

	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Debug, "VoiceSpeed[%zd]", (intptr_t)ss_VoiceSpeed ); }
	PnlComGetOperationSetting()->setVoiceSpeed( ss_VoiceSpeed );	// CommonAPIの中を一時的に更新する.

	::showVoiceCursor();	// 音声再生する前にカーソルの描画を更新する.

	// カレント音声取得処理.
	::reqVoicePartsVID( this, eVpartsKind );
	// 画面が更新されているのでメッセージエリアを取得.
	::reqVoiceMsgAreaVID( this );

	PNLC_VoiceRequestBuffer::newInstance()->sendRequest();// 再生要求を発行.

	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Debug, "VoiceSpeed[%zd]", (intptr_t)ss_VoiceSpeedBackup ); }
	PnlComGetOperationSetting()->setVoiceSpeed( ss_VoiceSpeedBackup );	// CommonAPIの中を元に戻す.
}
//================================================================================================
// 概要		:108A8030 音声速度 設定終了.
//================================================================================================
void PAccessibilityVoiceSpeed::procOk( uchar option )
{
	PnlComGetOperationSetting()->setVoiceSpeed( ss_VoiceSpeed );	//音声速度確定.
	nextGamen = &gp_AccessibilityVoiceGuideSetting;
}
//================================================================================================
// 概要		: 音声速度 テンキー４の処理(音声専用) .
//================================================================================================
void PAccessibilityVoiceSpeed::vproc4Key()
{
	ss_VoiceSpeedBackup = PnlComGetOperationSetting()->getVoiceSpeed();	//バックアップを取る.

	// カーソルを前へ移動.
	PNLC_VoiceCursorControl* pcVcc = PNLC_VoiceCursorControl::newInstance();
	short ssCurrent = pcVcc->getCurrent();
	if( ssCurrent < 0 ) { return; } // カーソル非表示.

	// カーソルを前に移動します.
	pcVcc->decCursor();

	// 現在位置の音声再生.
	if( pcVcc->getCurrent() == ssCurrent ) { return; }	// 位置の変化なしの場合は何もしない.

	switch( pcVcc->getCurrent() ) {
		case 0:		e_VoiceCursor = TYPE_VoiceSpeedSlow;	break;	// カーソルの位置が遅いのとき.
		case 1:		e_VoiceCursor = TYPE_VoiceSpeedUsual;	break;	// カーソルの位置が普通のとき.
		case 2:		e_VoiceCursor = TYPE_VoiceSpeedQuick;	break;	// カーソルの位置が速いのとき.
		default:	e_VoiceCursor = ss_VoiceSpeedBackup;	break;	// カーソルの位置がOKのとき、現状の速度をセットする.
	}

	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Debug, "VoiceSpeed[%zd]", (intptr_t)e_VoiceCursor ); }
	PnlComGetOperationSetting()->setVoiceSpeed( e_VoiceCursor );	// CommonAPIの中を一時的に更新する.

	// 現在位置のキー選択の音声IDリストを再生要求バッファへ格納する.
	addReqPartsVoiceID( pcVcc->getCurrent(), PNLE_VoicePartsKindKeyCur );
	// 音声速度を元に戻す前に再生要求を投げる.
	PNLC_VoiceRequestBuffer::newInstance()->sendRequest();// 再生要求を発行

	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Debug, "VoiceSpeed[%zd]", (intptr_t)ss_VoiceSpeedBackup ); }
	PnlComGetOperationSetting()->setVoiceSpeed( ss_VoiceSpeedBackup );	// CommonAPIの中を元に戻す.
}
//================================================================================================
// 概要		: 音声速度 テンキー６の処理(音声専用) .
//================================================================================================
void PAccessibilityVoiceSpeed::vproc6Key()
{
	ss_VoiceSpeedBackup = PnlComGetOperationSetting()->getVoiceSpeed();//バックアップを取る.

	// カーソルを次へ移動.
	PNLC_VoiceCursorControl* pcVcc = PNLC_VoiceCursorControl::newInstance();
	short ssCurrent = pcVcc->getCurrent();
	if( ssCurrent < 0 ) { return; } // カーソル非表示.

	// カーソルを次へ移動します.
	pcVcc->incCursor();

	// 現在位置の音声再生.
	if( pcVcc->getCurrent() == ssCurrent ) { return; } // 位置の変化なしの場合は何もしない.

	switch( pcVcc->getCurrent() ) {
		case 0:		e_VoiceCursor = TYPE_VoiceSpeedSlow;	break;	// カーソルの位置が遅いのとき.
		case 1:		e_VoiceCursor = TYPE_VoiceSpeedUsual;	break;	// カーソルの位置が普通のとき.
		case 2:		e_VoiceCursor = TYPE_VoiceSpeedQuick;	break;	// カーソルの位置が速いのとき.
		default:	e_VoiceCursor = ss_VoiceSpeedBackup;	break;	// カーソルの位置がOKのとき、現状の速度をセットする.
	}

	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Debug, "VoiceSpeed[%zd]", (intptr_t)e_VoiceCursor ); }
	PnlComGetOperationSetting()->setVoiceSpeed( e_VoiceCursor );	// CommonAPIの中を一時的に更新する.

	// 現在位置のキー選択の音声IDリストを再生要求バッファへ格納する.
	addReqPartsVoiceID( pcVcc->getCurrent(), PNLE_VoicePartsKindKeyCur );
	// 音声速度を元に戻す前に再生要求を投げる.
	PNLC_VoiceRequestBuffer::newInstance()->sendRequest();// 再生要求を発行

	if( ::isVoiceLog() ) { ::outputVoiceLog( TYPE_VoiceLog_Debug, "VoiceSpeed[%zd]", (intptr_t)ss_VoiceSpeedBackup ); }
	PnlComGetOperationSetting()->setVoiceSpeed( ss_VoiceSpeedBackup );	// CommonAPIの中を元に戻す.
}
//================================================================================================
// 概要		:ユーザー補助・拡大表示初期設定共通. : 開始画面.
//================================================================================================
void PAccessibilityEnlargeDispInitSetCom::startGamen()
{
	PnlComGetSystemStatus()->setUserChoiceChanging( True );		// ユーザチョイス変更設定中.

	// 下層からの遷移以外.
	if( True != bl_isUnder ) {
		switch( uc_AccsTransGamenFlag ) {
			case PCGE_UtilityInitsetdispSelectCopy:
				// 初期モード設定・コピーを取得.
				// 初期設定共通にする場合、通常表示の初期設定を設定する.
				if( False == PnlComGetOperationSetting()->getNormalAndLargeCommonSetting() ) {
					b_EnlargeDispInitCopyOn	= PnlComGetOperationSetting()->getInitModeCopyOnForLargeDisp();
				} else {
					b_EnlargeDispInitCopyOn = PnlComGetOperationSetting()->getInitModeCopyOn();
				}
				break;
			case PCGE_UtilityInitsetdispSelectFax:
				// 初期モード設定・FAXを取得.
				// 初期設定共通にする場合、通常表示の初期設定を設定する.
				if( False == PnlComGetOperationSetting()->getNormalAndLargeCommonSetting() ) {
					b_EnlargeDispInitFaxOn = PnlComGetOperationSetting()->getInitModeFaxOnForLargeDisp();
				} else {
					b_EnlargeDispInitFaxOn = PnlComGetOperationSetting()->getInitModeFaxOn();
				}
				break;
			case PCGE_UtilityInitsetdispSelectFaxOnly:
				// 初期モード設定・FAXを取得.
				// 初期設定共通にする場合、通常表示の初期設定を設定する.
				if( NULL == PnlComGetOperationSetting() ) {
					LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
					return;
				}
				if( True == PnlComGetOperationSetting()->getNormalAndLargeCommonSetting() ) {
					m_bl_EnlargeDispInitFaxAppOn = PnlComGetOperationSetting()->getInitModeFaxAppOn();
				}
				break;
			case PCGE_UtilityInitsetdispSelectNone:
			default:
				break;
		}
	}


	PAccessibilityTopMenuCom::startGamen();
}
//================================================================================================
// 概要		:ユーザー補助・拡大表示初期設定共通. : ボタン（押された）.
//================================================================================================
void PAccessibilityEnlargeDispInitSetCom::procButtonOn( uchar option )
{
	switch( option ) {
		case  0: b_EnlargeDispInitCopyOn = True;	break;	// コピー・現在の設定値キー .
		case  1: b_EnlargeDispInitCopyOn = False; break;	// コピー・出荷時の設定値キー .
//		case  2: b_EnlargeDispInitFaxOn	= True;	break;	// ファクス・現在の設定値キー .
//		case  3: b_EnlargeDispInitFaxOn	= False;break;	// ファクス・出荷時の設定値キー .
		case  2:
			switch( uc_AccsTransGamenFlag ) {
				case PCGE_UtilityInitsetdispSelectFax:
					b_EnlargeDispInitFaxOn	= True;	// ファクス・現在の設定値キー .
					break;
				case PCGE_UtilityInitsetdispSelectFaxOnly:
					m_bl_EnlargeDispInitFaxAppOn = True;	// ファクス・現在の設定値キー .
					break;
				default: break;
			}
			break;
		case  3:
			switch( uc_AccsTransGamenFlag ) {
				case PCGE_UtilityInitsetdispSelectFax:
					b_EnlargeDispInitFaxOn	= False;	// ファクス・出荷時の設定値キー .
					break;
				case PCGE_UtilityInitsetdispSelectFaxOnly:
					m_bl_EnlargeDispInitFaxAppOn = False;	// ファクス・出荷時の設定値キー .
					break;
				default: break;
			}
			break;
		default: break;
	}
}
//================================================================================================
// 概要		:ユーザー補助・拡大表示初期設定共通. : 設定終了.
//================================================================================================
void PAccessibilityEnlargeDispInitSetCom::procOk( uchar option )
{

	Bool	bl_ProcResult = True;

	switch( uc_AccsTransGamenFlag ) {
		case PCGE_UtilityInitsetdispSelectCopy:
			// 初期モード設定・コピーを設定 .
			// 初期設定共通にする場合、通常表示の初期設定を設定する.
			if( False == PnlComGetOperationSetting()->getNormalAndLargeCommonSetting() ) {
				PnlComGetOperationSetting()->setInitModeCopyOnForLargeDisp( b_EnlargeDispInitCopyOn );
				if( b_EnlargeDispInitCopyOn == False ) {
					PnlComGetOperationSetting()->setInitModeCopyForLargeDisp( &c_JobSpecCopyDefult );
				} else {
					// 初期ジョブデータ設定・コピーに現在の設定値を設定 .
					APIC_Job* pc_Job = PnlComGetJobCopy();
					if( NULL == pc_Job ) {
						bl_ProcResult = False;
					}
					else {
						TYPC_JobSpec* pc_CopyJobSpec = pc_Job->getJobSpec();
						if( NULL == pc_CopyJobSpec ) {
							bl_ProcResult = False;
						} else {
							PnlComGetOperationSetting()->setInitModeCopyForLargeDisp( pc_CopyJobSpec );
						}
					}
				}
			} else {
				PnlComGetOperationSetting()->setInitModeCopyOn( b_EnlargeDispInitCopyOn );
				if( False == b_EnlargeDispInitCopyOn ) {
					PnlComGetOperationSetting()->setInitModeCopy( &c_JobSpecCopyDefult );
				} else {
					// 初期ジョブデータ設定・コピーに現在の設定値を設定 .
					APIC_Job* pc_Job = PnlComGetJobCopy();
					if( NULL == pc_Job ) {
						bl_ProcResult = False;
					}
					else {
						TYPC_JobSpec* pc_CopyJobSpec = pc_Job->getJobSpec();
						if( NULL == pc_CopyJobSpec ) {
							bl_ProcResult = False;
						} else {
							PnlComGetOperationSetting()->setInitModeCopy( pc_CopyJobSpec );
						}
					}
				}
			}
			break;
		case PCGE_UtilityInitsetdispSelectFax:
			// 初期モード設定・FAXを設定 .
			// 初期設定共通にする場合、通常表示の初期設定を設定する.
			if( False == PnlComGetOperationSetting()->getNormalAndLargeCommonSetting() ) {
				PnlComGetOperationSetting()->setInitModeFaxOnForLargeDisp( b_EnlargeDispInitFaxOn );
				if( b_EnlargeDispInitFaxOn == False ) {
					// 初期ジョブデータ設定・FAXに出荷時の設定値を設定 .
					PnlComGetOperationSetting()->setInitModeFaxForLargeDisp( &c_JobSpecFaxDefult );
				} else {
					// 初期ジョブデータ設定・FAXに現在の設定値を設定 .
//					APIC_Job* pc_Job = PnlComGetJobFax();
					APIC_Job* pc_Job = PnlComGetJobScanFax( PCGE_JobPutFaxScan );
					if( NULL == pc_Job ) {
						bl_ProcResult = False;
					}
					else {
						TYPC_JobSpec* pc_FaxJobSpec = pc_Job->getJobSpec();
						if( NULL == pc_FaxJobSpec ) {
							bl_ProcResult = False;
						} else {
// comment 2019/5/17 SCD2-Sky工藤 [特ROM][FQ-OPSD-1900172-OFP101][1076979]プリントデータを受信した際にC-E301(0xB3BB)が発生する場合がある.
							if( TYPE_TxRxModeAutoTx != pc_FaxJobSpec->pc_Fax->c_FaxAct.getTxRxMode() ) {
								pc_FaxJobSpec->pc_Fax->c_FaxAct.setTxRxMode( TYPE_TxRxModeNon );
							}
							PnlComGetOperationSetting()->setInitModeFaxForLargeDisp( pc_FaxJobSpec );
						}
					}
				}
			} else {
				PnlComGetOperationSetting()->setInitModeFaxOn( b_EnlargeDispInitFaxOn );
				if( False == b_EnlargeDispInitFaxOn ) {
					// 初期ジョブデータ設定・FAXに出荷時の設定値を設定 .
					PnlComGetOperationSetting()->setInitModeFax( &c_JobSpecFaxDefult );
				} else {
					// 初期ジョブデータ設定・FAXに現在の設定値を設定 .
//					APIC_Job* pc_Job = PnlComGetJobFax();
					APIC_Job* pc_Job = PnlComGetJobScanFax( PCGE_JobPutFaxScan );
					if( NULL == pc_Job ) {
						bl_ProcResult = False;
					}
					else {
						TYPC_JobSpec* pc_FaxJobSpec = pc_Job->getJobSpec();
						if( NULL == pc_FaxJobSpec ) {
							bl_ProcResult = False;
						} else {
// comment 2019/5/17 SCD2-Sky工藤 [特ROM][FQ-OPSD-1900172-OFP101][1076979]プリントデータを受信した際にC-E301(0xB3BB)が発生する場合がある.
							if( TYPE_TxRxModeAutoTx != pc_FaxJobSpec->pc_Fax->c_FaxAct.getTxRxMode() ) {
								pc_FaxJobSpec->pc_Fax->c_FaxAct.setTxRxMode( TYPE_TxRxModeNon );
							}  
							PnlComGetOperationSetting()->setInitModeFax( pc_FaxJobSpec );
						}
					}
				}
			}
			break;
		case PCGE_UtilityInitsetdispSelectFaxOnly:
			if( NULL == PnlComGetOperationSetting() ) {
				bl_ProcResult = False;
			} else {
				// 初期モード設定・FAXを設定 .
				// 初期設定共通にする場合、通常表示の初期設定を設定する.
				if( True == PnlComGetOperationSetting()->getNormalAndLargeCommonSetting() ) {
					PnlComGetOperationSetting()->setInitModeFaxAppOn( m_bl_EnlargeDispInitFaxAppOn );
					if( False == m_bl_EnlargeDispInitFaxAppOn ) {
						// 初期ジョブデータ設定・FAXに出荷時の設定値を設定 .
						PnlComGetOperationSetting()->setInitModeFaxApp( &c_JobSpecFaxDefult );
					} else {
						// 初期ジョブデータ設定・FAXに現在の設定値を設定 .
						APIC_Job* pc_Job = PnlComGetJobScanFax( PCGE_JobPutFaxOnly );
						if( NULL == pc_Job ) {
							bl_ProcResult = False;
						}
						else {
							TYPC_JobSpec* pc_FaxAppJobSpec = pc_Job->getJobSpec();
							if( NULL == pc_FaxAppJobSpec ) {
								bl_ProcResult = False;
							} else {
// comment 2019/5/17 SCD2-Sky工藤 [特ROM][FQ-OPSD-1900172-OFP101][1076979]プリントデータを受信した際にC-E301(0xB3BB)が発生する場合がある.
								if( TYPE_TxRxModeAutoTx != pc_FaxAppJobSpec->pc_Fax->c_FaxAct.getTxRxMode() ) {
									pc_FaxAppJobSpec->pc_Fax->c_FaxAct.setTxRxMode( TYPE_TxRxModeNon );
								} 
								PnlComGetOperationSetting()->setInitModeFaxApp( pc_FaxAppJobSpec );
							}
						}
					}
				}
			}
			break;

		case PCGE_UtilityInitsetdispSelectNone:
		default:
			break;
	}

	// エラー処理.
	if( True == bl_ProcResult ) {
		// 正常.
		nextGamen = &gp_AccessibilityEnlargeDispInitSet;			// ユーザー補助・拡大表示初期設定.
	} else {
		// 異常.
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
	}

}
//================================================================================================
// 概要		:ユーザー補助・拡大表示初期設定(208A1000). : 開始画面.
//================================================================================================
void PAccessibilityEnlargeDispInitSet::startGamen()
{
	// 親クラスの画面開始処理を呼ぶ.
	PAccessibilityTopMenuCom::startGamen();
}
//================================================================================================
// 概要		:ユーザー補助・拡大表示初期設定(208A1000). ：画面移動.
//================================================================================================
void PAccessibilityEnlargeDispInitSet::procGoToPage( uchar option )
{
	switch( option ) {
		case 1: nextGamen = &gp_AccessibilityEnlargeDispInitSetCopy; break;	// ユーザー補助・拡大表示初期設定・拡大表示コピー初期設定.		// 押されたキー判定.
		case 2:	nextGamen = &gp_AccessibilityEnlargeDispInitSetScanFax; break;	// ユーザー補助・拡大表示初期設定・拡大表示ファクス/スキャン初期設定. // ファクス/スキャンキー  // ボックスモード時ではない(*2)、かつ、拡大表示中 .
			break;
		case 3:	// ユーザー補助・拡大表示初期設定・拡大表示ファクス初期設定.
			nextGamen = &gp_AccessibilityEnlargeDispInitSetFax;
			break;
		default: break;	// 他.
	}
}
//================================================================================================
// 概要		:ユーザー補助・拡大表示初期設定(208A1000). ：テンキー.
//================================================================================================
void PAccessibilityEnlargeDispInitSet::proc10Key( uchar option )
{
	switch( option ) {
		case 1:		// コピーキー.
			if( True ==  PnlComGetUserTrackOn() ) {		// 認証状態.
				procGoToPage( option );
			} else {
				setBuzzerType( PCGE_BuzzerType_InputNg );
			}
			break;
		case 2:		// ファクス/スキャンキー.
//			// 元がファクス/スキャン画面の場合のみ遷移.
			if( ( True ==  PnlComGetUserTrackOn() ) && ( True == PnlComIsUseSsdScanSend() ) ) {		// 認証状態、かつ、HDD装着/SSD1GB.
				procGoToPage( option );
			} else {
				setBuzzerType( PCGE_BuzzerType_InputNg );
			}
		case 3:		// ファクスキー.
			if( ( True == Key_EnlAccessibilityEDInitSetFaxOnly.isNowShown() ) &&		// キー表示中.
			    ( True == Key_EnlAccessibilityEDInitSetFaxOnly.isActivating() ) ) {	// キー活性中.
				procGoToPage( option );
			} else {
				setBuzzerType( PCGE_BuzzerType_InputNg );
			}
			break;
		default: setBuzzerType( PCGE_BuzzerType_InputNg ); break;
	}
}
//================================================================================================
// 概要		:ユーザー補助・拡大表示初期設定(208A1000). ：設定終了(閉じる) .
//================================================================================================
void PAccessibilityEnlargeDispInitSet::procOk( uchar option )
{
	if( ppc_SystemStatus->getUserChoiceChanging() == True ) {
		// ユーザチョイス変更設定中がONなら、OFFにする .
		ppc_SystemStatus->setUserChoiceChanging( False );
	}
	PAccessibilityTopMenuCom::procOk( option );
}
//================================================================================================
// 概要		:ユーザー補助・拡大表示初期設定(208A1000). ：表示画面のセットアップ.
//================================================================================================
void PAccessibilityEnlargeDispInitSet::setupGamen( short option )
{
	PAccessibilityTopMenuCom::setupGamen( option );
	// 初期設定共通にする場合、「拡大表示初期設定」→「初期設定」に変更.
	Txt_TitleAccessibilityEnlargeDispInitSet.changeText( ( True == PnlComGetOperationSetting()->getNormalAndLargeCommonSetting() ) ? s_TTLDefaultSettings : s_ENLTTLEnlargeDispCopySetting );
	TYPE_KeyMode e_KeyMode = APIC_KeyDevice::getKeyMode();				//	キーモード.
	if(
	    // 認証装置装着時で未認証のとき.
	    ( e_KeyMode == TYPE_KeyModePageAcses
	      && ( onlyPanel.isWarningOn( comPanelJob, PERROR_CheckPageACSESPowerSupply )	 // 認証装置(PageACSES)電源確認.
	           || onlyPanel.isWarningOn( comPanelJob, PERROR_MagCardUnset )				 // 管理カード（磁気カード）未挿入(プリント時).
	           || onlyPanel.isWarningOn( comPanelJob, PERROR_MagCardUnset_S ) 			 // 管理カード（磁気カード）未挿入(設定時).
	           || onlyPanel.isWarningOn( comPanelJob, PERROR_PageACSESSetInaccurateCard ) // 認証カード認証エラー.
	           || onlyPanel.isWarningOn( comPanelJob, PERROR_PageACSESSetAnotherCard )	 // 認証カード不一致.
	         )
	    )
	    // ベンダー（従来・統合タイプ）＋キーカウンタ装着時.
	    ||( ( pc_MachineSetting->getKeyMode() == TYPE_KeyModeVendor || pc_MachineSetting->getKeyMode() == TYPE_KeyModeVendorKM )
	        && e_KeyMode == TYPE_KeyModeKeyCounter
	      )
	    // ベンダー（従来・統合タイプ）＋(お金不足orベンダ電源警告).
	    || ( ( pc_MachineSetting->getKeyMode() == TYPE_KeyModeVendor || pc_MachineSetting->getKeyMode() == TYPE_KeyModeVendorKM )
	         && ( onlyPanel.isWarningOn( comPanelJob, PERROR_CoinUnset )
	              || onlyPanel.isWarningOn( comPanelJob, PERROR_CheckVendorPowerSupply ) )
	       )
	)
	{
		// ボックスモード時、または、拡大表示ではない.
		Key_EnlAccessibilityEDInitSetCopy.showEnable( False );			// コピーキーを無効にする.
		Key_EnlAccessibilityEDInitSetFaxScan.showEnable( False );		// ファクス/スキャンキーを無効にする.
		Key_EnlAccessibilityEDInitSetFaxOnly.showEnable( False );		// ファクスを無効にする.

		Btx_AccessibilityMenuNo1.showEnable( False );					// メニュー番号１を無効にする.
		Btx_AccessibilityMenuNo2.showEnable( False );					// メニュー番号２を無効にする.
		Btx_AccessibilityMenuNo3.showEnable( False );					// メニュー番号３を無効にする.
		ss_CopyFaxScanKeyDisp = 0;										// 拡大表示の選択はなし.
	} else {
		Bool bl_EnableHDDExistFunc = PnlComIsUseSsdScanSend();				// // HDD/SSD使用状態取得(True：可、False：不可).
		Key_EnlAccessibilityEDInitSetCopy.showEnable( True );			// コピーキーを有効にする.
		Key_EnlAccessibilityEDInitSetFaxScan.showEnable( bl_EnableHDDExistFunc );		// ファクス/スキャンキーを有効にする.
		if( ( False == bl_EnableHDDExistFunc ) ||	// HDD/SSD使用不可の場合はファクスキーは非表示.
		    ( False == onlyPanel.isFaxExist() ) ||	// ファクスボード非装着の場合.
		    ( False == PnlComGetOperationSetting()->getNormalAndLargeCommonSetting() ) ||	// 通常/拡大表示共通設定が「共通にしない」の時非表示.
		    ( False == PLBC_FuncVersionCheck::isOK_IT5_Type1() ) ) {
			Key_EnlAccessibilityEDInitSetFaxOnly.showEnable( False );		// ファクスを無効にする.
			Btx_AccessibilityMenuNo3.showEnable( False );					// メニュー番号３を無効にする.
		} else {
			Key_EnlAccessibilityEDInitSetFaxOnly.showEnable( True );		// ファクスを有効にする.
			Btx_AccessibilityMenuNo3.showEnable( True );					// メニュー番号３を有効にする.
		}

		Btx_AccessibilityMenuNo1.showEnable( True );					// メニュー番号１を有効にする.
		Btx_AccessibilityMenuNo2.showEnable( bl_EnableHDDExistFunc );					// メニュー番号２を有効にする.
		// ボックスモードのチェックは意味がなく、拡大表示の条件は、この画面に入る前に行っているので、未認証のみにする.
		if( False ==  PnlComGetUserTrackOn() ) {
			Key_EnlAccessibilityEDInitSetCopy.activate( False );			// コピーキーをグレーアウトにする.
			Key_EnlAccessibilityEDInitSetFaxScan.activate( False );			// ファクス/スキャンキーをグレーアウトにする.
		} else {
			Key_EnlAccessibilityEDInitSetCopy.activate( True );				// コピーキーを有効にする.
			Key_EnlAccessibilityEDInitSetFaxScan.activate( True );			// ファクス/スキャンキーを有効にする.
		}
	}
//comment [2016/07/13] [HCLT-Thangachamy] [CQ533250] Modified the "ファクス / スキャン予約" text based on Fax applicability.
	PNLC_ScanFaxDataLib* pc_ScanFaxDataLib = PNLC_ScanFaxDataLib::newInstance();
	if( ( NULL != pc_ScanFaxDataLib ) 
		&& ( False == pc_ScanFaxDataLib->IsFaxSettingEnable() ) ) {
		Key_EnlAccessibilityEDInitSetFaxScan.changeText( s_pENLTTLEnlargeDispScan_W768H032I );
	}else{
		Key_EnlAccessibilityEDInitSetFaxScan.changeText( s_ENLTTLEnlargeDispFaxScan );
	}
	// メッセージ欄のセットアップ.
	PAccessibilitycom::setupMessageAreaAccessibility( s_ENLMSGSelectItem );
	PAccessibilityTopMenuCom::setupGamen( option );
}
//================================================================================================
// 概要		:ユーザー設定・拡大表示コピー初期設定(208A1010) : ボタン（押された）.
//================================================================================================
void PAccessibilityEnlargeDispInitSetCopy::procButtonOn( uchar option )
{
	// 親クラスのボタン処理を呼ぶ.
	PAccessibilityEnlargeDispInitSetCom::procButtonOn( option );
	setupGamen( option );
}
//================================================================================================
// 概要		:ユーザー設定・拡大表示コピー初期設定(208A1010) :  開始画面.
//================================================================================================
void PAccessibilityEnlargeDispInitSetCopy::startGamen()
{
	PAccessibilityEnlargeDispInitSetCom::uc_AccsTransGamenFlag = PCGE_UtilityInitsetdispSelectCopy;	// 拡大表示選択状態[コピー].
	PAccessibilityEnlargeDispInitSetCom::startGamen();	// 親クラスの処理を呼ぶ.
}
//================================================================================================
// 概要		:ユーザー設定・拡大表示コピー初期設定(208A1010) : ＯＫキー.
//================================================================================================
void PAccessibilityEnlargeDispInitSetCopy::procOk( uchar option )
{
	PAccessibilityEnlargeDispInitSetCom::procOk( option );		// 親クラスの処理を呼ぶ.
}
//================================================================================================
// 概要		:ユーザー設定・拡大表示コピー初期設定(208A1010) : 表示画面のセットアップ.
//================================================================================================
void PAccessibilityEnlargeDispInitSetCopy::setupGamen( short option )
{
	PAccessibilityTopMenuCom::setupGamen( option );
	PAccessibilitycom::setupMessageAreaAccessibility( ( True == PnlComGetOperationSetting()->getNormalAndLargeCommonSetting() ) ?  s_MSGCopyDefaultModeSetting : s_ENLMSGlEnlargeDispCopySetting_c );
	Rbt_AccessibilityEnlargeDispInitSetCopy.select( AccessibilityGetEnlargeDispInitCopyOn() == True ? 0 : 1 );
}

//================================================================================================
// 概要		:ユーザー設定・拡大表示コピー初期設定(208A1010) : 画面終了処理.
//================================================================================================
void PAccessibilityEnlargeDispInitSetCopy::endGamen( void )
{

	PAccessibilityEnlargeDispInitSetCom::uc_AccsTransGamenFlag = PCGE_UtilityInitsetdispSelectNone;	// 拡大表示選択状態[初期化].

	// 親クラスの画面終了処理を呼ぶ.
	PAccessibilitycom::endGamen();
}

//================================================================================================
// 概要		:ユーザー設定・拡大表示ファクス/スキャン初期設定(208A1020) : ボタン（押された）.
//================================================================================================
void PAccessibilityEnlargeDispInitSetScanFax::procButtonOn( uchar option )
{
	// 親クラスのボタン処理を呼ぶ.
	PAccessibilityEnlargeDispInitSetCom::procButtonOn( option );
	setupGamen( option );
}
//================================================================================================
// 概要		:ユーザー設定・拡大表示ファクス/スキャン初期設定(208A1020) :  開始画面.
//================================================================================================
void PAccessibilityEnlargeDispInitSetScanFax::startGamen()
{
	PAccessibilityEnlargeDispInitSetCom::uc_AccsTransGamenFlag = PCGE_UtilityInitsetdispSelectFax;	// 拡大表示選択状態[コピー].
	PAccessibilityEnlargeDispInitSetCom::startGamen();	// 親クラスの処理を呼ぶ.
}
//================================================================================================
// 概要		:ユーザー設定・拡大表示ファクス/スキャン初期設定(208A1020) : ＯＫキー.
//================================================================================================
void PAccessibilityEnlargeDispInitSetScanFax::procOk( uchar option )
{
	PAccessibilityEnlargeDispInitSetCom::procOk( option );		// 親クラスの処理を呼ぶ.
}
//================================================================================================
// 概要		:ユーザー設定・拡大表示ファクス/スキャン初期設定(208A1020) : 表示画面のセットアップ.
//================================================================================================
void PAccessibilityEnlargeDispInitSetScanFax::setupGamen( short option )
{
	PAccessibilityTopMenuCom::setupGamen( option );
//comment [2016/07/13] [HCLT-Thangachamy] [CQ533250] Modified the "ファクス / スキャン予約" text based on Fax applicability.
//	PAccessibilitycom::setupMessageAreaAccessibility( ( True == PnlComGetOperationSetting()->getNormalAndLargeCommonSetting() ) ?  s_MSGScannerFaxDefaultModeSetting : s_ENLMSGEnlargeDisppScanSetting_c );
	PNLC_ScanFaxDataLib* pc_ScanFaxDataLib = PNLC_ScanFaxDataLib::newInstance();
	if( True == PnlComGetOperationSetting()->getNormalAndLargeCommonSetting() ){
		if( ( NULL != pc_ScanFaxDataLib ) 
			&& ( False == pc_ScanFaxDataLib->IsFaxSettingEnable() ) ) {
			PAccessibilitycom::setupMessageAreaAccessibility( s_pMSGScannerDefaultModeSetting_W624H016I );
			Txt_TitleAccessibilityEnlargeDispInitSetScanFax.changeText( s_pENLTTLEnlargeDispScan_W768H032I );
		} else {
			PAccessibilitycom::setupMessageAreaAccessibility( s_MSGScannerFaxDefaultModeSetting );
			Txt_TitleAccessibilityEnlargeDispInitSetScanFax.changeText( s_ENLTTLEnlargeDispFaxScan );	// 「ファクス/スキャン」.
		}
	} else {		
		if( ( NULL != pc_ScanFaxDataLib ) 
			&& ( False == pc_ScanFaxDataLib->IsFaxSettingEnable() ) ) {
			PAccessibilitycom::setupMessageAreaAccessibility( s_pENLMSGEnlargeDisppScanSetting_c_W560H033I );
			Txt_TitleAccessibilityEnlargeDispInitSetScanFax.changeText( s_pENLTTLEnlargeDispScan_W768H032I );
		} else {
			PAccessibilitycom::setupMessageAreaAccessibility( s_ENLMSGEnlargeDisppScanSetting_c );
			Txt_TitleAccessibilityEnlargeDispInitSetScanFax.changeText( s_ENLTTLEnlargeDispFaxScan );	// 「ファクス/スキャン」.
		}
	}
	Rbt_AccessibilityEnlargeDispInitSetScanFax.select( AccessibilityGetEnlargeDispInitFaxOn() == True ? 0 : 1 );
}
//================================================================================================
// 概要		:ユーザー設定・拡大表示ファクス/スキャン初期設定(208A1020): 画面終了処理.
//================================================================================================
void PAccessibilityEnlargeDispInitSetScanFax::endGamen( void )
{

	PAccessibilityEnlargeDispInitSetCom::uc_AccsTransGamenFlag = PCGE_UtilityInitsetdispSelectNone;	// 拡大表示選択状態[初期化].

	// 親クラスの画面終了処理を呼ぶ.
	PAccessibilitycom::endGamen();
}

//================================================================================================
// 概要		:ユーザー設定・拡大表示ファクス初期設定(H0NA103000) : ボタン（押された）.
//================================================================================================
void PAccessibilityEnlargeDispInitSetFax::procButtonOn( uchar option )
{
	// 親クラスのボタン処理を呼ぶ.
	PAccessibilityEnlargeDispInitSetCom::procButtonOn( option );
	setupGamen( option );
}
//================================================================================================
// 概要		:ユーザー設定・拡大表示ファクス初期設定(H0NA103000) :  開始画面.
//================================================================================================
void PAccessibilityEnlargeDispInitSetFax::startGamen()
{
	PAccessibilityEnlargeDispInitSetCom::uc_AccsTransGamenFlag = PCGE_UtilityInitsetdispSelectFaxOnly;	// 拡大表示選択状態[ファクス].
	PAccessibilityEnlargeDispInitSetCom::startGamen();	// 親クラスの処理を呼ぶ.

	PAccessibilitycom::setupMessageAreaAccessibility( s_MSGFaxDefaultModeSetting );
	Txt_TitleAccessibilityEnlargeDispInitSetScanFax.changeText( s_TTLENLFax );	// 「ファクス」.
}
//================================================================================================
// 概要		:ユーザー設定・拡大表示ファクス初期設定(H0NA103000) : 表示画面のセットアップ.
//================================================================================================
void PAccessibilityEnlargeDispInitSetFax::setupGamen( short option )
{
	PAccessibilityTopMenuCom::setupGamen( option );
	Rbt_AccessibilityEnlargeDispInitSetScanFax.select( True == AccessibilityGetEnlargeDispInitFaxAppOn() ? 0 : 1 );
}

//================================================================================================
// 概要		:ユーザー設定・拡大表示ファクス初期設定(H0NA103000): 画面終了処理.
//================================================================================================
void PAccessibilityEnlargeDispInitSetFax::endGamen( void )
{
	PAccessibilityEnlargeDispInitSetCom::uc_AccsTransGamenFlag = PCGE_UtilityInitsetdispSelectNone;	// 拡大表示選択状態[初期化].

	// 親クラスの画面終了処理を呼ぶ.
	PAccessibilitycom::endGamen();
}

//================================================================================================
// 概要		:ユーザー補助・ダブルタップ間隔設定(808A1000) :  開始画面.
//================================================================================================
void PAccessibilityDoubleTapIntervalSetting::startGamen()
{
// comment 2015/12/01 OPS1-Sky稲葉 [ZeusS_ZX0][制御No82] 欧州アクセシビリティ対応 「2度打ち防止優先設定」キーを追加.
//	Bool bl_isContinue = bl_isUnder;

	APIC_OperationSetting* pc_OperationSetting = PnlComGetOperationSetting();
	PAccessibilityTopMenuCom::startGamen();	// 親クラスをコール.
	setupMessageAreaAccessibility( s_ENLMSGDoubleTapIntervalSetting );

	PnlComMtuiIf_DoubleTapInterval::instance()->setComponent( &Cmp_DoubleTapInterval );
	Cmp_DoubleTapInterval.setDoubleTapInterval( getIntervalTime( sc_TapLevel ) );

// comment 2015/12/01 OPS1-Sky稲葉 [ZeusS_ZX0][制御No82] 欧州アクセシビリティ対応 「2度打ち防止優先設定」キーを追加.
//	// 下層からの遷移以外.
//	if( True != bl_isContinue ) {
//		ss_TapLevel = pc_OperationSetting->getDoubleTapIntervalMode();   	// 調整レベル取得.
//	} else {
//		// 下層からの戻りの場合は、ダブルタップ時間を再設定する。
//		PnlComQtSetDoubleTapInterval( getIntervalTime( ss_TapLevel ) );
//	}
		sc_TapLevel = pc_OperationSetting->getDoubleTapIntervalMode();   	// 調整レベル取得.

	PNLC_QWSConnector* pc_QWSConnector = PNLC_QWSConnector::newInstance();
	if( NULL != pc_QWSConnector ) {
		// Qtに2度打ち防止設定を「無効」で通知する.
		pc_QWSConnector->setTwiceTapInterval( false, 0 );
	}

	PnlComMtuiIf_DoubleTapInterval::instance()->show();
}
//================================================================================================
// 概要		:ユーザー補助・ダブルタップ間隔設定(808A1000) : 表示画面のセットアップ.
//================================================================================================
void PAccessibilityDoubleTapIntervalSetting::setupGamen( short option )
{
	PAccessibilityTopMenuCom::setupGamen( option );	// 親クラスをコール.
	setLevelGuage( ( uchar )sc_TapLevel );	// 調整レベルグラフィック.
}

//================================================================================================
// 概要		:ユーザー補助・ダブルタップ間隔設定(808A1000) : 表示画面の画面終了処理.
//================================================================================================
void PAccessibilityDoubleTapIntervalSetting::endGamen()
{
	PAccessibilityTopMenuCom::endGamen();	// 親クラスをコール.

// comment 2015/12/01 OPS1-Sky稲葉 [ZeusS_ZX0][制御No82] 欧州アクセシビリティ対応 「2度打ち防止優先設定」キーを追加.
//	PnlComQtSetDoubleTapInterval( getDoubleClickInterval() );
	// 画面開始時に、Qt/QtConnecterに対して値を設定しているため、画面終了時に元に戻す.
	// NVDから優先設定、2度打ち防止間隔を取得する.
	Bool bl_TwiceTap = False;
	uchar uc_TwiceTapInterval = 0;
	APIC_OperationSetting* pc_OperationSetting = PnlComGetOperationSetting();
	if ( NULL != pc_OperationSetting ) {
		bl_TwiceTap = ( TYPE_PriorityTapSetting_DoubleTap == pc_OperationSetting->getPriorityTapSetting() ) ? False : True;
		uc_TwiceTapInterval = pc_OperationSetting->getTwiceStrikeBlockInterval();
	}

	// Qtに2度打ち防止設定をNVD値で通知する.
	PNLC_QWSConnector* pc_QWSConnector = PNLC_QWSConnector::newInstance();
	if( NULL != pc_QWSConnector ) {
		pc_QWSConnector->setTwiceTapInterval( bl_TwiceTap, uc_TwiceTapInterval * PNLD_ExchangeTwiceIntervalNVDToQT );
	}

	PnlComQtSetDoubleTapInterval( bl_TwiceTap ? 0 : getDoubleClickInterval() );

	PnlComMtuiIf_DoubleTapInterval::instance()->hide();

	PnlComMtuiIf_DoubleTapInterval::instance()->setComponent( 0 );
}

//================================================================================================
// 概要		:ユーザー補助・ダブルタップ間隔設定(808A1000) : ＯＫキー.
//================================================================================================
void PAccessibilityDoubleTapIntervalSetting::procOk( uchar option )
{
	APIC_OperationSetting* pc_OperationSetting = PnlComGetOperationSetting();
	PAccessibilityTopMenuCom::procOk( option );					// 親クラスをコール.
	pc_OperationSetting->setDoubleTapIntervalMode( sc_TapLevel );	// ダブルタップ間隔調整値設定.
// comment 2015/12/01 OPS1-Sky稲葉 [ZeusS_ZX0][制御No82] 欧州アクセシビリティ対応 「2度打ち防止優先設定」キーを追加.
	nextGamen = &gp_AccessibilityTwicePressPreventionPrioritySetting;
}
//================================================================================================
// 概要		:ユーザー補助・ダブルタップ間隔設定(808A1000) : ボタン（押された）.
//================================================================================================
void PAccessibilityDoubleTapIntervalSetting::procButtonOn( uchar option )
{
	PAccessibilityTopMenuCom::procButtonOn( option );	// 親クラスをコール.
	switch( option ) {
		case BTNID_KEY_AccessibilityDoubleTapIntervalSlow:	// レベル・おそく.
			if( TYPE_DoubleTapIntervalModeSlow < sc_TapLevel ) {
				sc_TapLevel--;
			}
			break;
		case BTNID_KEY_AccessibilityDoubleTapIntervalFarst:	// レベル・はやく.
			if( TYPE_DoubleTapIntervalModeFast > sc_TapLevel ) {
				sc_TapLevel++;
			}
			break;
		case BTNID_KEY_AccessibilityDoubleTapIntervalNormal:	sc_TapLevel = TYPE_DoubleTapIntervalModeNormal;		break;	// レベル・ふつう.
		default:	break;
	}
	PnlComQtSetDoubleTapInterval( getIntervalTime( sc_TapLevel ) );
	Cmp_DoubleTapInterval.setDoubleTapInterval( getIntervalTime( sc_TapLevel ) );
	setupGamen( option );
}
// comment 2015/12/01 OPS1-Sky稲葉 [ZeusS_ZX0][制御No82] 欧州アクセシビリティ対応 「K4NA300000_2度打ち防止優先設定」画面を追加.
//================================================================================================
// 概要		:ユーザー補助・ダブルタップ間隔設定(808A1000) :  （押し続け、速く変化）キーリピート.
//================================================================================================
void PAccessibilityDoubleTapIntervalSetting::procButtonStillOnFast( uchar option ) {
	switch( option ) {
		case BTNID_KEY_AccessibilityDoubleTapIntervalSlow:	// レベル・おそく.
		case BTNID_KEY_AccessibilityDoubleTapIntervalFarst:	// レベル・はやく.
			procButtonOn( option );
			break;
		default: break;
	}
}
//================================================================================================
// 概要		:ユーザー補助・ダブルタップ間隔設定(808A1000) : ダブルタップ.
//================================================================================================
void PAccessibilityDoubleTapIntervalSetting::procDubleTap( uchar option )
{
}
//================================================================================================
// 概要		:ユーザー補助・ダブルタップ間隔設定(808A1000) : 1秒タイマー.
//================================================================================================
void PAccessibilityDoubleTapIntervalSetting::procSecond( uchar option )
{
	// 親クラスの処理.
	PAccessibilityTopMenuCom::procSecond( option );
}
//================================================================================================
// 概要		:ユーザー補助・ダブルタップ間隔設定(808A1000) : レベルゲージの表示設定.
//================================================================================================
void PAccessibilityDoubleTapIntervalSetting::setLevelGuage( uchar taplevel )
{
	for( int i = 0; i<5 ; i++ ) {
		Gra_AccessibilityDoubleTapIntervalLevelCursor[i].showEnable( False );
	}
	Gra_AccessibilityDoubleTapIntervalLevelCursor[taplevel].showEnable( True );
}

//================================================================================================.
// 概要		: ユーザー補助・ダブルタップ間隔設定(808A1000) : アクティブ通知処理 .
//================================================================================================.
void PAccessibilityDoubleTapIntervalSetting::procActivate()
{
	PAccessibilityTopMenuCom::procActivate();	// 親クラスの処理を呼ぶ.
	PnlComMtuiIf_DoubleTapInterval::instance()->show();
}

//================================================================================================.
// 概要		: ユーザー補助・ダブルタップ間隔設定(808A1000)  : 非アクティブ通知処理 .
//================================================================================================.
void PAccessibilityDoubleTapIntervalSetting::procInactivate()
{
	PAccessibilityTopMenuCom::procInactivate();	// 親クラスの処理を呼ぶ.
	PnlComMtuiIf_DoubleTapInterval::instance()->hide();
}

// comment 2015/12/01 OPS1-Sky稲葉 [ZeusS_ZX0][制御No82] 欧州アクセシビリティ対応 「K4NA300000_2度打ち防止優先設定」画面を追加.
//================================================================================================
// 概要		:ユーザー補助・ダブルタップ間隔設定(808A1000) : ダブルタップ間隔時間取得.
//================================================================================================
//short PAccessibilityDoubleTapIntervalSetting::getIntervalTime( short ss_Level )
//{
//	ushort us_IntervalTime = PNLD_ClickIntervalNormal;
//
//	switch( ss_Level ) {
//		case TYPE_DoubleTapIntervalModeSlow:		// おそい.
//			us_IntervalTime += PNLD_ClickIntervalSlow * 2;
//			break;
//		case TYPE_DoubleTapIntervalModeLittleSlow:	// ややおそい.
//			us_IntervalTime += PNLD_ClickIntervalSlow;
//			break;
//		case TYPE_DoubleTapIntervalModeNormal:		// ふつう.
//			break;
//		case TYPE_DoubleTapIntervalModeLittleFast:	// ややはやい.
//			us_IntervalTime -= PNLD_ClickIntervalFast;
//			break;
//		case TYPE_DoubleTapIntervalModeFast:		// はやい.
//			us_IntervalTime -= PNLD_ClickIntervalFast * 2;
//			break;
//		default:
//			break;
//	}
//
//	return us_IntervalTime;
//}

// ==================================================================
// PAccessibilityCommonKeyboard : ユーザー補助共通キーボード :  コンストラクタ.
// ==================================================================
PAccessibilityCommonKeyboard::PAccessibilityCommonKeyboard(
    PDisplayParts** partsTable,
    short partsNumber,
    Bool isNoWarningGamen )
	: PAccessibilityTopMenu( partsTable, partsNumber, isNoWarningGamen )
{
	memset( sc_InputString, 0, sizeof( sc_InputString ) );	// パスワード入力用
	us_MAXInputStringSize = 0;								// 最大入力文字サイズ
	pc_MmbDispField = NULL;
}
// ==================================================================
// PAccessibilityCommonKeyboard : ユーザー補助共通キーボード :  画面開始処理.
// ==================================================================
void PAccessibilityCommonKeyboard::startGamen()
{
// comment 2020/11/30 DWS1-Sky延原 【TAS000000003634】リモートパネル使用時にパスワード入力のキー操作がハイライト表示されてしまう.
	// リモートキーボードの場合、ハイライト表示を抑制する.
	if ( True == isRemotePanelConnecting() ) {
		LIBG_EnterHighlightDisplayHideSection();	//キー押下時のハイライト表示抑止.
	} else {
		LIBG_LeaveHighlightDisplayHideSection();	//キー押下時のハイライト表示抑止解除.
	}

	PAccessibilityTopMenu::startGamen();

	// 入力文字表示領域のアドレスを取得する.
	Kbd_UtilityInputKeybordConv.GetCursor( &Btx_UtilityAddressRegInputKeybord );
	// 入力文字表示領域を変更.
	pc_MmbDispField = &Btx_UtilityAddressRegInputKeybord;
	// キーボード初期状態の場合、キーボード切換え状態を初期状態とする.
	Kbd_UtilityInputKeybordConv.setInitKeybordStatus1();

	// スライド文字列管理クラスの初期化. 初期表示以外は文字列を初期化しない.
	if( PLBD_Enl_KBD_Initial == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
		// 文字列の初期化.
		memset( sc_InputString, '\0', sizeof( sc_InputString ) );
		// パス入力状態初期化.
		c_SlideTextMgr.initStatus( sc_InputString );
	}
	c_SlideTextMgr.init_CLCD( sc_InputString, ( char )Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), us_MAXInputStringSize, pc_MmbDispField, getCharSetForPanel() );
	startGamenKeybordUtility9E();
	Kbd_UtilityInputKeybordConv.registUSBKeyInvalidKey( 1, PLBD_KBD_AllCharUSBKeyShift );
	Kbd_UtilityInputKeybordConv.registUSBKeyInvalidKey( 34, PLBD_KBD_AllCharUSBKeyShift );
	// キーボード初期状態判定フラグをFalseとする.
	onlyPanel.b_InitKeybord = False;

	Kbd_UtilityInputKeybordConv.setChangeEnlKeybord( PLBD_Enl_KBD_Enlarge );
	KeybordType ec_KeybordType = Kbd_UtilityInputKeybordConv.changeKeyboadType();
	Kbd_UtilityInputKeybordConv.setKeybordType( ec_KeybordType );
	Kbd_UtilityInputKeybordConv.GetCursor( &Btx_EnlChangeKeybord );
	pc_MmbDispField = &Btx_EnlChangeKeybord;
	c_SlideTextMgr.init_CLCD( sc_InputString, ( char )Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(),
	                          us_MAXInputStringSize, pc_MmbDispField, getCharSetForPanel() );
}
// ==================================================================
// PAccessibilityCommonKeyboard : ユーザー補助共通キーボード :  表示画面のセットアップ.
// ==================================================================
void PAccessibilityCommonKeyboard::setupGamen( short option )
{
	Kbd_UtilityInputKeybordConv.showKeybord();		// 変換用キーボードの表示.
	setupGamenKeybordUtility();						// 標準／ワイド共通化.
	// 通常入力／パス入力の状態取得.
	char sc_AsterPass[ DEF_MAX_InputStringBufSize ];
	memset( sc_AsterPass, '\0',sizeof( sc_AsterPass ) );
	short ss_Strlen = static_cast<short>( strlen( sc_InputString ) );
	if( ss_Strlen > 0 ) {
		memset( sc_AsterPass, '*', ss_Strlen );
	}
	// キーボードの種類を取得する.
	KeybordType ec_KeybordType = Kbd_UtilityInputKeybordConv.getKeybordType();
	// 日本語入力の場合は未確定文字列表示用にスライド処理を行う.
	c_SlideTextMgr.PreStringSlideControl( sc_AsterPass, ec_KeybordType );
	c_SlideTextMgr.drawStr( sc_AsterPass, True );	//【 CQ362628 : ZeusMLK : Nagaoka : 2012.07.13 : 入力文字と表示文字の差分によるOffset時の不具合対応 】.
	printLEDControl( False );	// スタートキーＬＥＤの制御.
	if( PLBD_Enl_KBD_Enlarge == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
		PLBC_ConvertKeybord* pc_ConvKeyboard = PnlComGetConvertKeybord();
		if( ( NULL != pc_ConvKeyboard )
		    && ( True == pc_ConvKeyboard->isRemoteKeyboardMode() )
		    && ( True == pc_ConvKeyboard->isIMERunning() ) ) {
			// RemoteKeyboardでのIME起動中はクリアキーを非表示にする為何もしない.
		} else {
			Key_SoftEnl_Clear.showEnableShow( True );
		}
	}
}
// ==================================================================
// PAccessibilityCommonKeyboard : ユーザー補助共通キーボード :  ボタン（押された）.
// ==================================================================
void PAccessibilityCommonKeyboard::procButtonOn( uchar option )
{

	// キーボード共通ボタン押下処理
	procButtonOnKeybordUtility( option );
	c_SlideTextMgr.e_MmbCharSet = getCharSetForPanel();
	char	sc_keyCode[PLBD_KBD_BufMax];	// 確定文字列.
	if( True == Kbd_ScanKeybordConv.setKeybordDecideStr( option, sc_keyCode ) ) {
		c_SlideTextMgr.insertStr( sc_InputString, sc_keyCode );		// 無変換または、変換確定の場合、確定文字列を挿入.
	}
	setupGamen();
}
// ==================================================================
// PAccessibilityCommonKeyboard : ユーザー補助共通キーボード :  戻る.
// ==================================================================
void PAccessibilityCommonKeyboard::procCancel( uchar option )
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	if( NULL == pc_SoftDipSw ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();
	if( NULL == pc_PopupListControl ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }


	clearKeyBoard();		// キーボードデータクリア
	// DipSWNo.165 bit0がONの時、中止の場合は、TopMenu画面に戻る.
	if( True == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW165, 0 ) ) {
		setTurnGamenThis( NULL );								// 戻りをなしとする .
		if( onlyPanel.bl_isEnlargeDisplayMode == True ) {		// 拡大表示モード時 .
			nextGamen = &gp_EnlAct_HomeTopMenu;						// 拡大TopMenu .
		} else {
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//			nextGamen = NULL;									// 画面遷移はしない.
//			pc_PopupListControl->addPopup( &gp_HomeTopMenu, NULL, NULL, 0, 0, 0, 0, True );	// A072002000_トップメニュー（統合トップメニュー）.
			PnlComChangeScreenHome();
		}
		PAccessibilitycom::setAccessibilityGamenExit();			// ユーザー補助を抜ける時の処理 .
		bl_UnSetTurnGamen = True;								// 戻り画面をセットしない.
	} else {
		PAccessibilityTopMenu::procOk( option );
	}
}
// ==================================================================
// PAccessibilityCommonKeyboard : ユーザー補助共通キーボード :  画面終了処理.
// ==================================================================
void PAccessibilityCommonKeyboard::endGamen()
{
// comment 2020/11/30 DWS1-Sky延原 【TAS000000003634】リモートパネル使用時にパスワード入力のキー操作がハイライト表示されてしまう.
	// リモートキーボードの場合、ハイライト表示を抑制する.
	LIBG_LeaveHighlightDisplayHideSection();

	PAccessibilityTopMenu::endGamen();	// 親クラスの処理.
	Kbd_UtilityInputKeybordConv.setInitKeybordStatus1();	// キーボード初期状態の場合、キーボード切換え状態を初期状態とする.
	endGamenKeybordUtility();				// キーボード共通処理.
}
// ==================================================================
// PAccessibilityCommonKeyboard : ユーザー補助共通キーボード :  テンキー.
// ==================================================================
void PAccessibilityCommonKeyboard::proc10Key( uchar option )
{
	char	sc_Num[PCGD_NumberKeyCode];
	int32_t	i32_InputNum = 0;
	Kbd_UtilityInputKeybordConv.proc10Key( &option );
	if( 255 == option ) {
		setupGamen();
		return;
	}
	if( us_MAXInputStringSize <= ( ushort )strlen( sc_InputString ) ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;		// 最大入力長まで行っているならば入力しない
	}
	// 10キー入力値の取得
	i32_InputNum = proc10KeyInput( i32_InputNum, PCGD_MaxKyebordInput, option, False, EP_InputMaxLimitPattern_Refuse );
	// 入力されたのが0-9キーの時のみ処理を受け付ける
	if( ( 0 <= i32_InputNum ) && ( 9 >= i32_InputNum ) ) {
		snprintf( sc_Num, sizeof(sc_Num), "%d", i32_InputNum );				// 取得値を文字に変換する.
		c_SlideTextMgr.insertStr( sc_InputString, sc_Num );	// 文字を追加.
	}
	setupGamen();
}
// ==================================================================
// PAccessibilityCommonKeyboard : ユーザー補助共通キーボード :  クリア.
// ==================================================================
void PAccessibilityCommonKeyboard::procClear( uchar option )
{
	// 未確定文字列が存在する場合はクリアキーを受け付けない.
	if( '\0' != Kbd_UtilityInputKeybordConv.c_PreString[0] ) {
		// 報知音設定
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	clearKeyBoard();		// キーボードデータクリア
	setupGamen();
}
// ==================================================================
// PAccessibilityCommonKeyboard : ユーザー補助共通キーボード :  シャープキー.
// ==================================================================
void PAccessibilityCommonKeyboard::procShrp( uchar option )
{
	Kbd_UtilityInputKeybordConv.proc10Key( &option );
	if( 255 == option ) {
		setupGamen();
		return;
	}
	// アルファベットの時は受け付けない.
	if( False == Kbd_UtilityInputKeybordConv.b_KeybordAlphabetMode ) {
		char	sc_Num[PCGD_NumberKeyCode];
		// 最大入力長まで行っているならば入力しない.
		if( us_MAXInputStringSize <= ( ushort )strlen( sc_InputString ) ) {
			setBuzzerType( PCGE_BuzzerType_InputNg );
			return;
		}
		snprintf( sc_Num, sizeof(sc_Num), "#" );		// 入力された#の表示を行う.
		c_SlideTextMgr.insertStr( sc_InputString, sc_Num );	// 文字を追加.
	}
	// 画面セットアップ.
	setupGamen();
}
// ==================================================================
// PAccessibilityCommonKeyboard : ユーザー補助共通キーボード :  アスタリスクキー.
// ==================================================================
void PAccessibilityCommonKeyboard::procAster( uchar option )
{
	Kbd_UtilityInputKeybordConv.proc10Key( &option );
	if( 255 == option ) {
		setupGamen();
		return;
	}
	// アルファベットの時は受け付けない.
	if( Kbd_UtilityInputKeybordConv.b_KeybordAlphabetMode == False ) {
		char	sc_Num[ PCGD_NumberKeyCode ];
		// 最大入力長まで行っているならば入力しない.
		if( us_MAXInputStringSize <= ( ushort )strlen( sc_InputString ) ) {
			setBuzzerType( PCGE_BuzzerType_InputNg );
			return;
		}
		snprintf( sc_Num, sizeof(sc_Num), "*" );		// 入力された*の表示を行う.
		c_SlideTextMgr.insertStr( sc_InputString, sc_Num );	// 文字を追加.
	}
	// 画面セットアップ.
	setupGamen();
}
// ==================================================================
// PAccessibilityCommonKeyboard : ユーザー補助共通キーボード :  キーボード.
// ==================================================================
void PAccessibilityCommonKeyboard::procKeyBoard( uchar option )
{
	if( True == c_SlideTextMgr.getSelectProc() ) {
		if( True == c_SlideTextMgr.getStatus() ) {
			// 入力中状態に変更
			c_SlideTextMgr.setStatus( False );
			clearKeyBoard();		// キーボードデータクリア
		}
	}
	Kbd_UtilityInputKeybordConv.procCursorPosition( ( char* )&c_SlideTextMgr, sc_InputString, option, False );	// カーソル位置処理
	setupGamen();
}
// ==================================================================
// PAccessibilityCommonKeyboard : ユーザー補助共通キーボード :  キーボードデータクリア.
// ==================================================================
void PAccessibilityCommonKeyboard::clearKeyBoard()
{
	memset( sc_InputString, '\0', sizeof( sc_InputString ) );
	// スライド文字列管理クラスの初期化.
	Kbd_UtilityInputKeybordConv.GetCursor( pc_MmbDispField );		// 入力文字表示領域のアドレスを取得する.
	c_SlideTextMgr.init_CLCD( sc_InputString, ( char )Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), us_MAXInputStringSize, pc_MmbDispField, getCharSetForPanel() );
	if( True == PLBC_FuncVersionCheck::isOK_Type1() ) {
		PLBC_WideKeyboard::getInstance()->initInputArea_Clear();	// スライド文字列管理クラスの初期化 （Wideキーボード用）.
	}
}
// ==================================================================
// PAccessibilityCommonKeyboard : ユーザー補助共通キーボード :  最大入力文字数.
// ==================================================================
void PAccessibilityCommonKeyboard::setMAXInputStringSize( ushort ss_inputsize )
{
	us_MAXInputStringSize = ( DEF_MAX_InputStringBufSize < ss_inputsize ) ? DEF_MAX_InputStringBufSize : ss_inputsize;
}
// ==================================================================
// PAccessibilityCommonKeyboard : ユーザー補助共通キーボード :  キーボード文字列取得.
// ==================================================================
void PAccessibilityCommonKeyboard::getKeyBoardString( char* sc_strText )
{
	// 文字列をコピー.
	strcpy( sc_strText, sc_InputString );
}

// ==================================================================
// PAccessibilityCommonKeyboard : ユーザー補助共通キーボード :  戻る.
// ==================================================================
void PAccessibilityCommonKeyboard::procReset( uchar option )
{
	APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
	if( NULL == pc_SoftDipSw ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }
	PNLC_PopupListControl* pc_PopupListControl = PNLC_PopupListControl::newInstance();
	if( NULL == pc_PopupListControl ) { LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" ); return; }

	// DipSWNo.165 bit0がONの時、中止の場合は、TopMenu画面に戻る.
	if( True == pc_SoftDipSw->getSoftDipSwBitForPanelSpecNo( DEFSOFTWARESW165, 0 ) ) {
		clearKeyBoard();		// キーボードデータクリア
		setTurnGamenThis( NULL );								// 戻りをなしとする .
		if( onlyPanel.bl_isEnlargeDisplayMode == True ) {		// 拡大表示モード時 .
			nextGamen = &gp_EnlAct_HomeTopMenu;						// 拡大TopMenu画面 .
		} else {
// comment 2018/11/05 SCD2-Sky肥後 [EagleH] WebUIv3FUM対応.
//			nextGamen = NULL;									// 画面遷移はしない.
//			pc_PopupListControl->addPopup( &gp_HomeTopMenu, NULL, NULL, 0, 0, 0, 0, True );	// A072002000_トップメニュー（統合トップメニュー）.
			PnlComChangeScreenHome();
		}
		PAccessibilitycom::setAccessibilityGamenExit();			// ユーザー補助を抜ける時の処理 .
		bl_UnSetTurnGamen = True;								// 戻り画面をセットしない.
	} else {
		PAccessibilityTopMenu::procReset( option );
	}
}

// comment 2020/11/30 DWS1-Sky延原 【TAS000000003634】リモートパネル使用時にパスワード入力のキー操作がハイライト表示されてしまう.
// ==================================================================
// PAccessibilityCommonKeyboard : ユーザー補助共通キーボード :  リモートパネル接続通知イベント.
// ==================================================================
void PAccessibilityCommonKeyboard::procRemotePanelStart( uchar option )
{
	LIBG_EnterHighlightDisplayHideSection();	//キー押下時のハイライト表示抑止.

	PAccessibilityTopMenu::procRemotePanelStart( option );
}

// ==================================================================
// PAccessibilityCommonKeyboard : ユーザー補助共通キーボード :  リモートパネル切断通知イベント.
// ==================================================================
void PAccessibilityCommonKeyboard::procRemotePanelStop( uchar option )
{
	LIBG_LeaveHighlightDisplayHideSection();	//キー押下時のハイライト表示抑止解除.

	PAccessibilityTopMenu::procRemotePanelStop( option );
}

// ==================================================================
// PAccessibilityCommonKeyboard : ユーザー補助共通キーボード :  非アクティブ状態通知イベント.
// ==================================================================
void PAccessibilityCommonKeyboard::procInactivate()
{
	LIBG_LeaveHighlightDisplayHideSection();	//キー押下時のハイライト表示抑止解除.

	// 親クラスを呼び出し.
	PAccessibilityTopMenu::procInactivate();
}

// ==================================================================
// PAccessibilityCommonKeyboard : ユーザー補助共通キーボード :  リモートパネル接続中判定.
// ==================================================================
Bool PAccessibilityCommonKeyboard::isRemotePanelConnecting( void )
{
	Bool bl_RemotePanelConnecting = False;

	// リモートパネル接続状態を確認する.
	APIC_RemotePanelSetting* pc_RemotePanelSetting = APIC_RemotePanelSetting::newInstance();
	if( NULL != pc_RemotePanelSetting && TYPE_RemoteStatusNone != pc_RemotePanelSetting->getRemoteStatus() ) {
		bl_RemotePanelConnecting = True;
	}

	return bl_RemotePanelConnecting;
}

// ============================================================================================= //
//////////////////////////////////////////////////////////////
// 概要 : 管理者パスワード入力画面 : 画面開始処理.
//////////////////////////////////////////////////////////////
void PAccessibilityAdminPasswordInput::startGamen()
{
	if( PLBD_Enl_KBD_Initial == Kbd_UtilityInputKeybordConv.getChangeEnlKeybord() ) {
		bl_PasswordError = False;
	}
	// 親クラスの画面開始処理をコール.
	PAccessibilityCommonKeyboard::setMAXInputStringSize( DEF_MAX_AdminPasswordInput );	// 入力文字数 64文字制限.
	PAccessibilityCommonKeyboard::startGamen();
}
//////////////////////////////////////////////////////////////
// 概要 : 管理者パスワード入力画面 : 画面開始処理.
//////////////////////////////////////////////////////////////
void PAccessibilityAdminPasswordInput::setupGamen( short option )
{
	PAccessibilityCommonKeyboard::setupGamen( option );
	if( 0 == strlen( sc_InputString ) ) {
		if( False == bl_PasswordError ) {	// 通常メッセージ.
			if( True == PLBC_FuncVersionCheck::isOK_Type1() ) {
#ifdef DEF_LCD_7INCH
				Txt_MsgAccessibilityAdminPasswordInput.changeText( s_pMSGMessageAdminPassword2_W520H038A );
#else // #ifdef DEF_LCD_7INCH.
				Txt_MsgAccessibilityAdminPasswordInput.changeText( s_MSGMessageAdminPassword2 );
#endif // #ifdef DEF_LCD_7INCH.
			} else {
				Txt_MsgAccessibilityAdminPasswordInput.changeText( s_MSGAdministratorPassword );
			}
		} else {						// エラーメッセージ.
			Txt_MsgAccessibilityAdminPasswordInput.changeText( s_MSGPasswordNotMatch );			// パスワードが一致しません。入力し直してください.
		}
		Txt_MsgAccessibilityAdminPasswordInput.showEnable( True );
		Btx_EnlChangeKeybord.setCursorOption( UC_CURSOR_OPTION_OFF );
	} else {
		Txt_MsgAccessibilityAdminPasswordInput.showEnable( False );
		Btx_EnlChangeKeybord.setCursorOption( UC_CURSOR_OPTION_ON );
	}
}

//////////////////////////////////////////////////////////////
// 概要 : 管理者パスワード入力画面 : OKボタン（押された）.
//////////////////////////////////////////////////////////////
void PAccessibilityAdminPasswordInput::procOk( uchar option )
{
	// パスワードエラーペナルティ中はOK処理をしない.
	if( True == bl_PasswordError ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	// 比較用バッファ.
	char sc_InputPassword[ DEF_MAX_InputStringBufSize ];
	char sc_Password[ DEF_MAX_InputStringBufSize ];
	memset( sc_InputPassword, 0, sizeof( sc_InputPassword ) );
	memset( sc_Password, 0, sizeof( sc_Password ) );
	// ユーザーパスワード取得
	APIC_User* pc_UserAdmin = APIC_User::newInstance( TYPD_IndexUserAdmin );  // 管理者.
	if( NULL == pc_UserAdmin ) {
		return;
	}
	pc_UserAdmin->getUserPassword( sc_Password );
	// 入力文字列取得
	getKeyBoardString( sc_InputPassword );
	// パスワード比較.
	if( 0 == strncmp( sc_InputPassword, sc_Password, DEF_MAX_InputStringBufSize ) ) {	// 管理者パスワードが一致.
		bl_PasswordError = False;							// エラー状態初期化.
		clearKeyBoard();								// キーボードデータクリア
		nextGamen = &gp_AccessibilityTopMenu;			// ユーザー補助.
	} else {	// 管理者パスワードが不一致.
		clearKeyBoard();		// キーボードデータクリア.
		setBuzzerType( PCGE_BuzzerType_Caution1 );
		bl_PasswordError = True;					// 不一致エラー.
		setupGamen();
	}
}
//////////////////////////////////////////////////////////////
// 概要 : 管理者パスワード入力画面 : 1秒タイマ.
//////////////////////////////////////////////////////////////
void PAccessibilityAdminPasswordInput::procSecond( uchar option )
{
	static uchar uc_localPenaltyTimer = 0;
	// 親クラスの処理.
	PAccessibilityCommonKeyboard::procSecond( option );
	// ５秒で警告を解除する.
	if( True == bl_PasswordError ) {			// パスワードエラーでタイマカウント開始.
		if( 5 <= ++uc_localPenaltyTimer ) {
			bl_PasswordError = False;		// ユーザー認証エラーを解除する.
			uc_localPenaltyTimer = 0;	// タイマカウントクリア.
		}
	}
	setupGamen();
}
//////////////////////////////////////////////////////////////
// 概要 : 管理者パスワード入力画面 : テンキー（押された）.
//////////////////////////////////////////////////////////////
void PAccessibilityAdminPasswordInput::proc10Key( uchar option )
{
	// パスワードエラーペナルティ中は処理をしない.
	if( True == bl_PasswordError ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	PAccessibilityCommonKeyboard::proc10Key( option );
}
//////////////////////////////////////////////////////////////
// 概要 : 管理者パスワード入力画面 : クリアキー（押された）.
//////////////////////////////////////////////////////////////
void PAccessibilityAdminPasswordInput::procClear( uchar option )
{
	// パスワードエラーペナルティ中は処理をしない.
	if( True == bl_PasswordError ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	PAccessibilityCommonKeyboard::procClear( option );
}
//////////////////////////////////////////////////////////////
// 概要 : 管理者パスワード入力画面 : ボタン（押された）.
//////////////////////////////////////////////////////////////
void PAccessibilityAdminPasswordInput::procButtonOn( uchar option )
{
	// パスワードエラーペナルティ中は処理をしない.
	if( True == bl_PasswordError ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	PAccessibilityCommonKeyboard::procButtonOn( option );
}
//////////////////////////////////////////////////////////////
// 概要 : 管理者パスワード入力画面 : キーボードイベント.
//////////////////////////////////////////////////////////////
void PAccessibilityAdminPasswordInput::procKeyBoard( uchar option )
{
	// パスワードエラーペナルティ中は処理をしない.
	if( True == bl_PasswordError ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	PAccessibilityCommonKeyboard::procKeyBoard( option );
}
//////////////////////////////////////////////////////////////
// 概要 : 管理者パスワード入力画面 : 中止ボタン（押された）.
//////////////////////////////////////////////////////////////
void PAccessibilityAdminPasswordInput::procCancel( uchar option )
{
	// パスワードエラーペナルティ中は処理をしない.
	if( True == bl_PasswordError ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	PAccessibilityCommonKeyboard::procCancel( option );
}
//////////////////////////////////////////////////////////////
// 概要 : ユーティリティー共通キーボード入力 : シャープキー.
//////////////////////////////////////////////////////////////
void PAccessibilityAdminPasswordInput::procShrp( uchar option )
{
	// パスワードエラーペナルティ中は処理をしない.
	if( True == bl_PasswordError ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	PAccessibilityCommonKeyboard::procShrp( option );
}
//////////////////////////////////////////////////////////////
// 概要 : ユーティリティー共通キーボード入力 : アスタリスクキー.
//////////////////////////////////////////////////////////////
void PAccessibilityAdminPasswordInput::procAster( uchar option )
{
	// パスワードエラーペナルティ中は処理をしない.
	if( True == bl_PasswordError ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	PAccessibilityCommonKeyboard::procAster( option );
}
//////////////////////////////////////////////////////////////
// 概要 : ユーティリティー共通キーボード入力 : リセットキー.
//////////////////////////////////////////////////////////////
void PAccessibilityAdminPasswordInput::procReset( uchar option )
{
	// パスワードエラーペナルティ中は処理をしない.
	if( True == bl_PasswordError ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PAccessibilityCommonKeyboard::procReset( option );
}
//////////////////////////////////////////////////////////////
// 概要 : ユーティリティー共通キーボード入力 : ユーティリティキー.
//////////////////////////////////////////////////////////////
void PAccessibilityAdminPasswordInput::procUtility( uchar option )
{
	// パスワードエラーペナルティ中は処理をしない.
	if( True == bl_PasswordError ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PAccessibilityCommonKeyboard::procUtility( option );
}
//////////////////////////////////////////////////////////////
// 概要 : ユーティリティー共通キーボード入力 : ユーザー補助キー.
//////////////////////////////////////////////////////////////
void PAccessibilityAdminPasswordInput::procAccessibility( uchar option )
{
	// パスワードエラーペナルティ中は処理をしない.
	if( True == bl_PasswordError ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PAccessibilityCommonKeyboard::procAccessibility( option );
}
//////////////////////////////////////////////////////////////
// 概要 : ユーティリティー共通キーボード入力 : パワーセーブキー.
//////////////////////////////////////////////////////////////
void PAccessibilityAdminPasswordInput::procPowerSave( uchar option )
{
	// パスワードエラーペナルティ中は処理をしない.
	if( True == bl_PasswordError ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}

	PAccessibilityCommonKeyboard::procPowerSave( option );
}

//////////////////////////////////////////////////////////////
// 概要 : 管理者パスワード入力画面 : デフォルト背景.
//////////////////////////////////////////////////////////////
void PAccessibilityAdminPasswordInput::setDefaultBackParts()
{
	setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AccessibilityAdminPasswordInputEnlDefault, ss_AccessibilityAdminPasswordInputEnlDefault );	// 拡大用デフォルト背景.
}
// ============================================================================================= //
//====================================================================== .
// 概要 : 管理者パスワード入力画面(ワイド) : 画面開始処理.
//====================================================================== .
void PAccessibilityAdminPasswordInputWide::startGamen()
{
	PLBC_WideKeyboard::getInstance()->startWideKeyboard( PNLE_WideKeyboard_Normal, &Txt_UtilityAndAdminComTitle, PNLE_WideKeyboardExNone ) ;
	PAccessibilityAdminPasswordInput::startGamen();
	PLBC_WideKeyboard::getInstance()->initInputArea( &Kbd_UtilityInputKeybordConv,&c_SlideTextMgr,sc_InputString, (char)Kbd_UtilityInputKeybordConv.getKeyboardDispLenMax(), us_MAXInputStringSize, &Btx_InputDspAreaWide, getCharSetForPanel() ) ;	// スライド文字列管理クラスの初期化 .
}

//====================================================================== .
// 概要 : 管理者パスワード入力画面(ワイド) : 画面終了処理.
//====================================================================== .
void PAccessibilityAdminPasswordInputWide::endGamen()
{
	PLBC_WideKeyboard::getInstance()->endWideKeyboard() ;
	PAccessibilityAdminPasswordInput::endGamen() ;
}

//====================================================================== .
// 概要 : 管理者パスワード入力画面(ワイド) : クリア.
//====================================================================== .
void PAccessibilityAdminPasswordInputWide::procClear( uchar option )
{
	if( True == bl_PasswordError ) {
		setBuzzerType( PCGE_BuzzerType_InputNg );
		return;
	}
	PAccessibilityAdminPasswordInput::procClear( option );
	PLBC_WideKeyboard::getInstance()->initInputArea_Clear() ;
	setupGamen();
}

static const struct MusicKindCategoryTbl {
	uchar							uc_MenuNo;				// 反応音出力種類.
	APIE_MusicKind					e_MusicKind;			// 反応音選択種類.
} st_MusicKindCategoryTbl[] = {
	{ PNLD_AccessibilityMusicOutputSetMenu_OperationCheck,		APIE_MusicKindInputOk		},		// 入力確認音.
	{ PNLD_AccessibilityMusicOutputSetMenu_OperationCheck,		APIE_MusicKindInputNg		},		// 入力無効音.
	{ PNLD_AccessibilityMusicOutputSetMenu_OperationCheck,		APIE_MusicKindCardinalPoint	},		// 基点音.
	{ PNLD_AccessibilityMusicOutputSetMenu_OperationCheck,		APIE_MusicKindMotion1		},		// モーション１（ポップアップ）.
	{ PNLD_AccessibilityMusicOutputSetMenu_OperationCheck,		APIE_MusicKindMotion2		},		// モーション２（キーボード）.
	{ PNLD_AccessibilityMusicOutputSetMenu_NormalEnd,			APIE_MusicKindOperation		},		// 操作終了音.
	{ PNLD_AccessibilityMusicOutputSetMenu_NormalEnd,			APIE_MusicKindCommunication	},		// 通信終了音.
	{ PNLD_AccessibilityMusicOutputSetMenu_NormalEnd,			APIE_MusicKindPcPrint		},		// PCプリント完了音.
	{ PNLD_AccessibilityMusicOutputSetMenu_NormalEnd,			APIE_MusicKindFaxPrint		},		// FAXプリント完了音.
	{ PNLD_AccessibilityMusicOutputSetMenu_Preparation,			APIE_MusicKindPreparation	},		// 準備完了音.
	{ PNLD_AccessibilityMusicOutputSetMenu_Warning,				APIE_MusicKindWarning1		},		// 弱注意音<level-1>.
	{ PNLD_AccessibilityMusicOutputSetMenu_Warning,				APIE_MusicKindWarning2		},		// 弱注意音<level-2>.
	{ PNLD_AccessibilityMusicOutputSetMenu_Warning,				APIE_MusicKindWarning3		},		// 弱注意音<level-3>.
	{ PNLD_AccessibilityMusicOutputSetMenu_Warning,				APIE_MusicKindError			},		// 強注意音.
	{ PNLD_AccessibilityMusicOutputSetMenu_Delete,				APIE_MusicKindDelete		},		// 削除音.
	{ PNLD_AccessibilityMusicOutputSetMenu_Auth,				APIE_MusicKindAuthOk		},		// 認証成功.
	{ PNLD_AccessibilityMusicOutputSetMenu_Auth,				APIE_MusicKindAuthNg		},		// 認証失敗.
	{ PNLD_AccessibilityMusicOutputSetMenu_All,					APIE_MusicKindMax			},		// 入力確認音.
};

// ====================================================================================================================.
//	機能：コンストラクタ(反応音メニュー選択画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		pc_partsTable		画面パーツテーブル.
//		ss_partsNumber		画面パーツ数.
//		b_isNoWarningGamen	警告表示禁止指定.
//		ps_MenuTbl			メニューテーブル.
//		uc_MenuCnt			メニュー数.
//		pc_EndGamen			終了画面.
///	\par	履歴:.\n
// ====================================================================================================================.
PAccessibilityMusicMenuCom::PAccessibilityMusicMenuCom( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen,
        PNLS_AccessibilityMusicSetMenuTbl* ps_MenuTbl, uchar uc_MenuCnt, PBasicMessageProc* pc_EndGamen )
	: PAccessibilityTopMenuCom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen ),
	  m_ps_MenuTbl( ps_MenuTbl )
	  , m_uc_MenuCnt( uc_MenuCnt )
	  , m_uc_MenuPageNum( 0 )
	  , m_uc_SelectMenuNo( PNLD_AccessibilityMusicOutputSetMenuSelectNone )
	  , m_us_PageNum( 1 )
	  , m_pc_EndGamen( pc_EndGamen )
{
	m_uc_MenuPageNum = (uchar)(uc_MenuCnt / PNLD_AccessibilityMusicMenuViewCount
	                   + ( ( ( uc_MenuCnt % PNLD_AccessibilityMusicMenuViewCount ) > 0 ) ? 1 : 0 ));	// ページ数算出.
}

// ====================================================================================================================.
//	機能：デフォルト背景設定(反応音メニュー選択画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicMenuCom::setDefaultBackParts()
{
	setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AccessibilityMusicOutputSetDefault,
	                         ss_AccessibilityMusicOutputSetDefault );
}

// ====================================================================================================================.
//	機能：初期化処理(反応音メニュー選択画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicMenuCom::initialize()
{
	m_us_PageNum = 1;	// 表示ページを初期化する.
}

// ====================================================================================================================.
//	機能：画面セットアップ処理(反応音メニュー選択画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicMenuCom::startGamen()
{
	PAccessibilityTopMenuCom::startGamen();

	PNLC_FromBeforeGamen* pc_FromBeforeGamen = PNLC_FromBeforeGamen::getInstance();
	if( False == pc_FromBeforeGamen->isFromBeforeGamen() ) {			// 上位画面からの遷移ではない.
		m_us_PageNum = 1;											// ページ数の初期化.
	}
	pc_FromBeforeGamen->setFromBeforeGamen( False );				// 遷移後のOFF設定.
}

// ====================================================================================================================.
//	機能：画面セットアップ処理(反応音メニュー選択画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicMenuCom::setupGamen( short ss_option )
{
	PAccessibilityTopMenuCom::setupGamen( ss_option );

	setupMessageAreaAccessibility( s_MSGChangeSettingValueKeyboardInput2 );							// メッセージ欄の設定.

	// メニューの設定.
	uchar uc_MenuStartNo = (uchar)(( m_us_PageNum - 1 ) * PNLD_AccessibilityMusicMenuViewCount);				// メニュー開始番号.
	uchar uc_MenuRowNo = 0;																			// メニュー列番号.
	uchar uc_MenuNo = 0;																			// 対象メニュー番号.
	for( uc_MenuRowNo = 0; uc_MenuRowNo < PNLD_AccessibilityMusicMenuViewCount; uc_MenuRowNo++ ) {
		uc_MenuNo = static_cast<uchar>( uc_MenuStartNo + uc_MenuRowNo );
		if( uc_MenuNo < m_uc_MenuCnt ) {
			Key_AccessibilityMusicMenuBtn[ uc_MenuRowNo ].changeText(
			    m_ps_MenuTbl[ uc_MenuNo ].tid_MenuString );										// メニューテキスト設定.
			Key_AccessibilityMusicMenuBtn[ uc_MenuRowNo ].activate( True );							// 選択キー有効設定.
			Btx_AccessibilityMusicMenuNo[ uc_MenuRowNo ].showEnable( True );						// 選択キー表示.
			Key_AccessibilityMusicMenuBtn[ uc_MenuRowNo ].showEnable( True );						// メニューテキスト表示.
		} else {
			Btx_AccessibilityMusicMenuNo[ uc_MenuRowNo ].showEnable( False );						// 選択キー非表示.
			Key_AccessibilityMusicMenuBtn[ uc_MenuRowNo ].showEnable( False );						// メニューテキスト非表示.
		}
	}

	// ページ数の設定.
	if( m_uc_MenuPageNum > 1 ) {
		Bak_E70X616Y160W136H044.set( &Num_AccessibilityPageNowEnl, m_us_PageNum, True );			// 現在のページ数.
		Bak_E70X616Y160W136H044.set( &Num_AccessibilityPageSetEnl, m_uc_MenuPageNum, True );		// 総ページ数.
		Bak_E70X616Y160W136H044.showEnable( True );													// ページ数表示.
		Bak_E90X608Y152W176H152.showEnable( True );													// ページ数背景表示.
		Key_EnlAccessibilityPageUp.showEnable( True );												// アップキー表示.
		Key_AccessibilityMusicOutputSetPageDown.showEnable( True );									// ダウンキー表示.
	} else {
		Bak_E70X616Y160W136H044.showEnable( False );												// ページ数非表示.
		Bak_E90X608Y152W176H152.showEnable( False );												// ページ数背景表示.
		Key_EnlAccessibilityPageUp.showEnable( False );												// アップキー非表示.
		Key_EnlAccessibilityPageDown.showEnable( False );											// ダウンキー非表示.
	}
}

// ====================================================================================================================.
//	機能：ボタン押下処理(反応音メニュー選択画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		uc_option	オプション(ボタンオプション).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicMenuCom::procButtonOn( uchar uc_option )
{
	switch( uc_option ) {
		case 0:										// ページアップキー.
			m_us_PageNum--;
			if( m_us_PageNum < 1 ) {
				m_us_PageNum = m_uc_MenuPageNum;	// 最終ページ.
			}
			break;
		case 1:										// ページダウンキー.
			m_us_PageNum++;
			if( m_uc_MenuPageNum < m_us_PageNum ) {
				m_us_PageNum = 1;					// 先頭ページ.
			}
			break;
		default:
			break;
	}
	setVoiceGamenInfo();
	setupGamen();
}

// ====================================================================================================================.
//	機能：テンキー押下処理(反応音メニュー選択画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		uc_option	オプション(テンキーオプション).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicMenuCom::proc10Key( uchar uc_option )
{
	uchar uc_GoToPageOption = m_uc_MenuCnt;
	switch( uc_option ) {
		case 1:	// 1キー.
			uc_GoToPageOption = PNLD_AccessibilityMusicMenu_1;
			break;
		case 2:	// 2キー.
			uc_GoToPageOption = PNLD_AccessibilityMusicMenu_2;
			break;
		case 3:	// 3キー.
			uc_GoToPageOption = PNLD_AccessibilityMusicMenu_3;
			break;
		case 4:	// 4キー.
			uc_GoToPageOption = PNLD_AccessibilityMusicMenu_4;
			break;
		case 5:	// 5キー.
			uc_GoToPageOption = PNLD_AccessibilityMusicMenu_5;
			break;
		default:// 他のテンキー.
			setBuzzerType( PCGE_BuzzerType_InputNg );
			return;
	}
	procGoToPage( uc_GoToPageOption );
}

// ====================================================================================================================.
//	機能：画面移動処理(反応音メニュー選択画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		uc_option	オプション(ボタンオプション).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicMenuCom::procGoToPage( uchar uc_option )
{
	// メニュー開始番号.
	uchar uc_MenuStartNo = (uchar)(( m_us_PageNum - 1 ) * PNLD_AccessibilityMusicOutputSetMenuViewCount);
	PBasicMessageProc* pc_NextGamen = NULL;
	uchar uc_SelectMenuNo = PNLD_AccessibilityMusicMenuSelectNone;

	switch( uc_option ) {
		case PNLD_AccessibilityMusicMenu_1:					// メニュー：一列目キー.
			uc_SelectMenuNo = uc_MenuStartNo + 0;
			break;
		case PNLD_AccessibilityMusicMenu_2:					// メニュー：二列目キー.
			uc_SelectMenuNo = static_cast<uchar>( uc_MenuStartNo + 1 );
			break;
		case PNLD_AccessibilityMusicMenu_3:					// メニュー：三列目キー.
			uc_SelectMenuNo = static_cast<uchar>( uc_MenuStartNo + 2 );
			break;
		case PNLD_AccessibilityMusicMenu_4:					// メニュー：四列目キー.
			uc_SelectMenuNo = static_cast<uchar>( uc_MenuStartNo + 3 );
			break;
		case PNLD_AccessibilityMusicMenu_5:					// メニュー：五列目キー.
			uc_SelectMenuNo = static_cast<uchar>( uc_MenuStartNo + 4 );
			break;
		default:
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return;
	}

	if( PNLD_AccessibilityMusicMenuSelectNone != uc_SelectMenuNo
	    && uc_SelectMenuNo < m_uc_MenuCnt ) {
		pc_NextGamen = m_ps_MenuTbl[ uc_SelectMenuNo ].pc_NextGamen;
	} else {
		setBuzzerType( PCGE_BuzzerType_InputNg );		// 画面に無い番号の場合.
		return;
	}

	if( NULL != pc_NextGamen ) {
		PNLC_FromBeforeGamen* pc_FromBeforeGamen = PNLC_FromBeforeGamen::getInstance();
		pc_FromBeforeGamen->setFromBeforeGamen( True );	// 上位画面からの遷移設定.
		nextGamen = pc_NextGamen;						// 画面遷移.
	}
}
// ====================================================================================================================.
//	機能：設定終了処理(反応音メニュー選択画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		uc_option	オプション(ボタンオプション).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicMenuCom::procOk( uchar uc_option )
{
	if( NULL != m_pc_EndGamen ) {
		nextGamen = m_pc_EndGamen;						// 終了(OK)後の遷移先へ.
	} else {
		PAccessibilityTopMenuCom::procOk( uc_option );
	}
}


// ====================================================================================================================.
//	機能：メニュー有効無効判断処理(音設定画面).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		uc_MenuNo	メニュー番号.
//	引数：.
//		uc_MenuNo	メニュー番号.
//	戻値：.
//		Bool		有効無効(True：有効, False：無効).
///	\par	履歴:.\n
// ====================================================================================================================.
Bool PAccessibilityMusicTop::isMenuEnable( uchar uc_MenuNo )
{
	if( PNLD_AccessibilityMusicOutputSetMenu_All == uc_MenuNo ) {
		return True;											// 一括設定は常に有効.
	}

	APIC_Music* pc_Music = APIC_Music::newInstance();
	Bool b_AllEnable = pc_Music->isCategoryEnable( APIE_MusicCategoryAll );
	if( False == b_AllEnable ) {
		return False;											// 一括で無効設定の場合は、無効にする.
	}
	return b_AllEnable;

}

// ====================================================================================================================.
//	機能：画面セットアップ処理(90NA4000:音設定画面).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicTop::setupGamen( short ss_option )
{
	PAccessibilityMusicMenuCom::setupGamen( ss_option );

	setupMessageAreaAccessibility( s_ENLMSGSelectItem );												// メッセージ欄の設定.

	Bool b_Enable = False;
	uchar uc_MenuStartNo = (uchar)(( m_us_PageNum - 1 ) * PNLD_AccessibilityMusicMenuViewCount);				// メニュー開始番号.
	uchar uc_MenuRowNo = 0;																			// メニュー列番号.
	uchar uc_MenuNo = 0;																			// 対象メニュー番号.
	for( uc_MenuRowNo = 0; uc_MenuRowNo < PNLD_AccessibilityMusicMenuViewCount; uc_MenuRowNo++ ) {
		uc_MenuNo = static_cast<uchar>( uc_MenuStartNo + uc_MenuRowNo );
		b_Enable = isMenuEnable( uc_MenuNo );
		if( True == b_Enable ) {
			continue;																				// 有効の時は次へ.
		}
		if( uc_MenuNo < m_uc_MenuCnt ) {
			Key_AccessibilityMusicMenuBtn[ uc_MenuRowNo ].activate( False );						// 選択キー無効設定.
		}
	}
}

// ====================================================================================================================.
//	機能：10キー押下処理(90NA4000:音設定画面).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：uc_opton.
//	戻値    ：.
// ====================================================================================================================.
void PAccessibilityMusicTop::proc10Key( uchar uc_option )
{
	uchar uc_MenuStartNo = (uchar)(( m_us_PageNum - 1 ) * PNLD_AccessibilityMusicMenuViewCount);				// メニュー開始番号.
	uchar uc_MenuNo = 0;																			// 対象メニュー番号.
	uc_MenuNo = static_cast<uchar>( uc_MenuStartNo + ( uc_option - 1 ) );

	if( True == isMenuEnable( uc_MenuNo ) ) {					// キーが有効.
		PAccessibilityMusicMenuCom::proc10Key( uc_option );
	} else {													// キーが無効.
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}

// ====================================================================================================================.
//	機能：設定終了(90NA4000:音設定画面).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：uc_opton.
//	戻値    ：.
// ====================================================================================================================.
void PAccessibilityMusicTop::procOk( uchar uc_option )
{
	// 統合トップメニューフラグがTrueでなければ、ユニバーサルのトップメニューへ.
	if( False == onlyPanel.getReturnGamenTopMenu() ) {
		PAccessibilityMusicMenuCom::procOk( uc_option );
	} else {
		Bool bl_ModeBack = onlyPanel.isTopmenuMode();
		onlyPanel.setTopmenuMode( True );
		setTurnGamenThis( NULL );				// 戻りをなしとする .
		rebuildGamen( uc_option );
		PAccessibilitycom::setAccessibilityGamenExit();		// ユーザー補助を抜ける時の処理 .
		bl_UnSetTurnGamen = True;				// 戻り画面をセットしない.
		onlyPanel.setTopmenuMode( bl_ModeBack );
	}
}

//関数------------------------------------------------------------------------------------------------------------------.
//概要    ：画面の音声情報を設定(108A4000_音設定).
//備考    ：.
//戻値    ：.
//作成者  ：kuroda.
//レビュー：.
//変更履歴：.
//----------------------------------------------------------------------------------------------------------------------.
void PAccessibilityMusicTop::setVoiceGamenInfo()
{
	m_pstVoiceGamenInfo = &VGI_AccessibilitySoundSetKeyReceiveDenial_MiB_P1;
}

// ====================================================================================================================.
//	機能：コンストラクタ(反応音種類選択画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		pc_partsTable		画面パーツテーブル.
//		ss_partsNumber		画面パーツ数.
//		b_isNoWarningGamen	警告表示禁止指定.
//		ps_MenuTbl			メニューテーブル.
//		uc_MenuCnt			メニュー数.
//		pc_EndGamen			終了画面.
///	\par	履歴:.\n
// ====================================================================================================================.
PAccessibilityMusicOutputSetCom::PAccessibilityMusicOutputSetCom( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen,
        PNLS_AccessibilityMusicSetMenuTbl* ps_MenuTbl, uchar uc_MenuCnt, PBasicMessageProc* pc_EndGamen )
	: PAccessibilityTopMenuCom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen ),
	  m_ps_MenuTbl( ps_MenuTbl ), m_uc_MenuCnt( uc_MenuCnt ), m_uc_MenuPageNum( 0 ),
	  m_uc_SelectMenuNo( PNLD_AccessibilityMusicOutputSetMenuSelectNone ),
	  m_uc_PageNum( 1 ), m_pc_EndGamen( pc_EndGamen )
{
	m_uc_MenuPageNum = uc_MenuCnt / PNLD_AccessibilityMusicOutputSetMenuViewCount;	// ページ数算出.
	if( uc_MenuCnt % PNLD_AccessibilityMusicOutputSetMenuViewCount != 0 ) {			// 端数は切り上げ.
		m_uc_MenuPageNum++;
	}
	memset( m_b_MenuYesNo, 0x0, sizeof( m_b_MenuYesNo ) );
}

// ====================================================================================================================.
//	機能：デストラクタ(反応音種類選択画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
PAccessibilityMusicOutputSetCom::~PAccessibilityMusicOutputSetCom()
{
}

// ====================================================================================================================.
//	機能：デフォルト背景設定(反応音種類選択画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicOutputSetCom::setDefaultBackParts()
{
	setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AccessibilityMusicOutputSetDefault,
	                         ss_AccessibilityMusicOutputSetDefault );
}

// ====================================================================================================================.
//	機能：画面開始処理(反応音種類選択画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicOutputSetCom::startGamen()
{
	PAccessibilityTopMenuCom::startGamen();

	PNLC_FromBeforeGamen* pc_FromBeforeGamen = PNLC_FromBeforeGamen::getInstance();
	if( False == pc_FromBeforeGamen->isFromBeforeGamen() ) {			// 上位画面からの遷移ではない.
		if( PNLD_AccessibilityMusicOutputSetMenuSelectNone != m_uc_SelectMenuNo ) {
			// 子画面遷移時のメニュー状態の設定.
			m_b_MenuYesNo[ m_uc_SelectMenuNo ] = isMenuYesNo( m_uc_SelectMenuNo );
			m_uc_SelectMenuNo = PNLD_AccessibilityMusicOutputSetMenuSelectNone;
		}
		return ;
	}
	pc_FromBeforeGamen->setFromBeforeGamen( False );				// 遷移後のOFF設定.

	// 先頭ページ（初期ページ）に設定.
	m_uc_PageNum = 1;												// 表示しているページナンバー.

	setVoiceGamenInfo();
	uchar uc_MenuNo = 0;											// メニュー番号.
	for( uc_MenuNo = 0; uc_MenuNo < m_uc_MenuCnt; uc_MenuNo++ ) {	// メニュー状態の設定.
		m_b_MenuYesNo[ uc_MenuNo ] = isMenuYesNo( uc_MenuNo );
	}
}

// ====================================================================================================================.
//	機能：画面セットアップ処理(反応音種類選択画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicOutputSetCom::setupGamen( short ss_option )
{
	PAccessibilityTopMenuCom::setupGamen( ss_option );

	// メッセージ欄のセットアップ.
	setupMessageAreaAccessibility( s_MSGENLPerformSoundOutputSetup );

	// メニューの設定.
	uchar uc_MenuStartNo = ( uchar )( ( m_uc_PageNum - 1 ) * PNLD_AccessibilityMusicOutputSetMenuViewCount );	// メニュー開始番号.
	uchar uc_MenuRowNo = 0;																					// メニュー列番号.
	uchar uc_MenuNo = 0;																					// 対象メニュー番号.
	for( uc_MenuRowNo = 0; uc_MenuRowNo < PNLD_AccessibilityMusicOutputSetMenuViewCount; uc_MenuRowNo++ ) {
		uc_MenuNo = ( uchar )( uc_MenuStartNo + uc_MenuRowNo );
		if( uc_MenuNo < m_uc_MenuCnt ) {
			Btx_AccessibilityMusicOutputSetMenu[ uc_MenuRowNo ].changeText(
			    m_ps_MenuTbl[ uc_MenuNo ].tid_MenuString );												// メニューテキスト設定.
			Rbt_AccessibilityMusicOutputSetMenu[ uc_MenuRowNo ].select( ( uchar )( m_b_MenuYesNo[ uc_MenuNo ] ? 0:1 ) );	// 選択キー設定.
			Rbt_AccessibilityMusicOutputSetMenu[ uc_MenuRowNo ].activate( True );							// 選択キー有効設定.
			Btx_AccessibilityMusicOutputSetMenu[ uc_MenuRowNo ].showEnable( True );							// メニューテキスト表示.
			Rbt_AccessibilityMusicOutputSetMenu[ uc_MenuRowNo ].showEnable( True );							// 選択キー表示.
		} else {
			Btx_AccessibilityMusicOutputSetMenu[ uc_MenuRowNo ].showEnable( False );						// メニューテキスト非表示.
			Rbt_AccessibilityMusicOutputSetMenu[ uc_MenuRowNo ].showEnable( False );						// 選択キー非表示.
		}
	}

	// ページ数の設定.
	if( m_uc_MenuPageNum > 1 ) {
		Btx_AccessibilityMusicOutputSetPageNum.set( &Num_AccessibilityMusicOutputSetPageNowEnl, m_uc_PageNum, True );		// 現在のページ数.
		Btx_AccessibilityMusicOutputSetPageNum.set( &Num_AccessibilityMusicOutputSetPageSetEnl, m_uc_MenuPageNum, True );	// 総ページ数.
		Btx_AccessibilityMusicOutputSetPageNum.showEnable( True );		// ページ数表示.
		Key_AccessibilityMusicOutputSetPageUp.showEnable( True );		// アップキー表示.
		Key_AccessibilityMusicOutputSetPageDown.showEnable( True );		// ダウンキー表示.
	} else {
		Btx_AccessibilityMusicOutputSetPageNum.showEnable( False );		// ページ数非表示.
		Key_AccessibilityMusicOutputSetPageUp.showEnable( False );		// アップキー非表示.
		Key_AccessibilityMusicOutputSetPageDown.showEnable( False );	// ダウンキー非表示.
	}
}

// ====================================================================================================================.
//	機能：ボタン押下処理(反応音種類選択画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		uc_option	オプション(ボタンオプション).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicOutputSetCom::procButtonOn( uchar uc_option )
{
	uchar uc_MenuStartNo = ( uchar )( ( m_uc_PageNum - 1 ) * PNLD_AccessibilityMusicOutputSetMenuViewCount );	// メニュー開始番号.
	switch( uc_option ) {
		case PNLD_AccessibilityMusicOutputSetPageUp:	// ページアップキー.
			m_uc_PageNum--;
			if( m_uc_PageNum < 1 ) {
				m_uc_PageNum = m_uc_MenuPageNum;		// 最終ページ.
			}
			break;
		case PNLD_AccessibilityMusicOutputSetPageDown:	// ページダウンキー.
			m_uc_PageNum++;
			if( m_uc_MenuPageNum < m_uc_PageNum ) {
				m_uc_PageNum = 1;						// 先頭ページ.
			}
			break;
		case PNLD_AccessibilityMusicOutputSetMenuYes_1:	// メニュー：一列目するキー.
			m_b_MenuYesNo[ uc_MenuStartNo + 0 ] = True;
			break;
		case PNLD_AccessibilityMusicOutputSetMenuYes_2:	// メニュー：二列目するキー.
			m_b_MenuYesNo[ uc_MenuStartNo + 1 ] = True;
			break;
		case PNLD_AccessibilityMusicOutputSetMenuYes_3:	// メニュー：三列目するキー.
			m_b_MenuYesNo[ uc_MenuStartNo + 2 ] = True;
			break;
		case PNLD_AccessibilityMusicOutputSetMenuYes_4:	// メニュー：四列目するキー.
			m_b_MenuYesNo[ uc_MenuStartNo + 3 ] = True;
			break;
		case PNLD_AccessibilityMusicOutputSetMenuYes_5:	// メニュー：五列目するキー.
			m_b_MenuYesNo[ uc_MenuStartNo + 4 ] = True;
			break;
		case PNLD_AccessibilityMusicOutputSetMenuNo_1:	// メニュー：一列目しないキー.
			m_b_MenuYesNo[ uc_MenuStartNo + 0 ] = False;
			break;
		case PNLD_AccessibilityMusicOutputSetMenuNo_2:	// メニュー：二列目しないキー.
			m_b_MenuYesNo[ uc_MenuStartNo + 1 ] = False;
			break;
		case PNLD_AccessibilityMusicOutputSetMenuNo_3:	// メニュー：三列目しないキー.
			m_b_MenuYesNo[ uc_MenuStartNo + 2 ] = False;
			break;
		case PNLD_AccessibilityMusicOutputSetMenuNo_4:	// メニュー：四列目しないキー.
			m_b_MenuYesNo[ uc_MenuStartNo + 3 ] = False;
			break;
		case PNLD_AccessibilityMusicOutputSetMenuNo_5:	// メニュー：五列目しないキー.
			m_b_MenuYesNo[ uc_MenuStartNo + 4 ] = False;
			break;
		default:
			break;
	}
	setVoiceGamenInfo();
	setupGamen();
}

// ====================================================================================================================.
//	機能：設定終了処理(反応音種類選択画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		uc_option	オプション(ボタンオプション).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicOutputSetCom::procOk( uchar uc_option )
{
	uchar uc_MenuNo = 0;											// メニュー番号.

	for( uc_MenuNo = 0; uc_MenuNo < m_uc_MenuCnt; uc_MenuNo++ ) {
		setMenuYesNo( uc_MenuNo, m_b_MenuYesNo[ uc_MenuNo ] );		// 終了時の設定処理.
	}
	APIC_Music* pc_Music = APIC_Music::newInstance();
	if( NULL == pc_Music ) { return; }
	ushort i = 0;
	for( i = 0; i < sizeof( st_MusicKindCategoryTbl )/sizeof( MusicKindCategoryTbl ); i++ ) {
		if( PNLD_AccessibilityMusicOutputSetMenu_All == st_MusicKindCategoryTbl[i].uc_MenuNo ) {
			break;
		}
		pc_Music->setEnable( st_MusicKindCategoryTbl[i].e_MusicKind, m_b_MenuYesNo[ st_MusicKindCategoryTbl[i].uc_MenuNo ] );
	}

	nextGamen = m_pc_EndGamen;										// 終了(OK)後の遷移先へ.
}


// ====================================================================================================================.
//	機能：画面セットアップ処理(90NA4010:音出力設定画面面).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicOutputSet::setupGamen( short ss_option )
{
	PAccessibilityMusicOutputSetCom::setupGamen( ss_option );
	Bool b_AllEnable = m_b_MenuYesNo[ PNLD_AccessibilityMusicOutputSetMenu_All ];
	if( True == b_AllEnable ) {
		return ;																							// 一括設定が有効の時は、無効にする必要が無いため、処理しない.
	}
	uchar uc_MenuStartNo = (uchar)(( m_uc_PageNum - 1 ) * PNLD_AccessibilityMusicOutputSetMenuViewCount);			// メニュー開始番号.
	uchar uc_MenuRowNo = 0;																					// メニュー列番号.
	uchar uc_MenuNo = 0;																					// 対象メニュー番号.
	for( uc_MenuRowNo = 0; uc_MenuRowNo < PNLD_AccessibilityMusicOutputSetMenuViewCount; uc_MenuRowNo++ ) {
		uc_MenuNo = static_cast<uchar>( uc_MenuStartNo + uc_MenuRowNo );
		if( PNLD_AccessibilityMusicOutputSetMenu_All == uc_MenuNo ) {
			continue;																						// 一括設定のみ有効.
		}
		if( uc_MenuNo < m_uc_MenuCnt ) {
			Rbt_AccessibilityMusicOutputSetMenu[ uc_MenuRowNo ].activate( False );							// 選択キー無効設定.
		}
	}
}

// ====================================================================================================================.
//	機能：画面セットアップ処理(90NA4010:音出力設定画面面).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicOutputSet::procButtonOn( uchar uc_option )
{
	uchar uc_MenuNo = 0;

	switch( uc_option ) {
		case PNLD_AccessibilityMusicOutputSetMenuYes_1:
			if( 1 == m_uc_PageNum ) {								// 1ページ目の一括出力するの場合.
				// 全ての項目をするに設定.
				for( uc_MenuNo = PNLD_AccessibilityMusicOutputSetMenu_All; uc_MenuNo < m_uc_MenuCnt; uc_MenuNo++ ) {
					m_b_MenuYesNo[ uc_MenuNo ] = True;
				}
			}
			break;
		default:
			break;
	}
	PAccessibilityMusicOutputSetCom::procButtonOn( uc_option );
}

// ====================================================================================================================.
//	機能：出力ON/OFF判断処理(90NA4100:音出力設定画面).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		uc_MenuNo	メニュー番号.
//	戻値：.
//		Bool		出力ON/OFF判断(True：する, False：しない).
///	\par	履歴:.\n
// ====================================================================================================================.
Bool PAccessibilityMusicOutputSet::isMenuYesNo( uchar uc_MenuNo )
{
	Bool b_YesNo = False;
	APIE_MusicCategory e_Category = APIE_MusicCategorySettingMax;
	APIC_Music* pc_Music = APIC_Music::newInstance();

	switch( uc_MenuNo ) {
		case PNLD_AccessibilityMusicOutputSetMenu_All:				// 一括出力.
			e_Category = APIE_MusicCategoryAll;
			break;
		case PNLD_AccessibilityMusicOutputSetMenu_OperationCheck:	// 操作確認音出力.
			e_Category = APIE_MusicCategoryOpeConfirm;
			break;
		case PNLD_AccessibilityMusicOutputSetMenu_NormalEnd:		// 正常終了音出力.
			e_Category = APIE_MusicCategoryNormalEnd;
			break;
		case PNLD_AccessibilityMusicOutputSetMenu_Preparation:		// 準備完了音出力.
			e_Category = APIE_MusicCategoryPrepEnd;
			break;
		case PNLD_AccessibilityMusicOutputSetMenu_Warning:			// 注意音出力.
			e_Category = APIE_MusicCategoryWarning;
			break;
		case PNLD_AccessibilityMusicOutputSetMenu_Delete:			// 削除音出力.
			e_Category = APIE_MusicCategoryDelete;
			break;
		case PNLD_AccessibilityMusicOutputSetMenu_Auth:				// 認証音出力.
			e_Category = APIE_MusicCategoryAuth;
			break;
		default:
			break;
	}
	b_YesNo = pc_Music->isCategoryEnable( e_Category );			// 分類単位の有効無効取得.
	return b_YesNo;
}

// ====================================================================================================================.
//	機能：出力ON/OFF判断設定処理(90NA4100:音出力設定画面).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		uc_MenuNo	メニュー番号.
//		b_YesNo		メニュー番号に対応した出力ON/OFF状態.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicOutputSet::setMenuYesNo( uchar uc_MenuNo , Bool b_YesNo )
{
	APIE_MusicCategory e_Category = APIE_MusicCategorySettingMax;
	APIC_Music* pc_Music = APIC_Music::newInstance();

	switch( uc_MenuNo ) {
		case PNLD_AccessibilityMusicOutputSetMenu_All:				// 一括出力.
			e_Category = APIE_MusicCategoryAll;
			break;
		case PNLD_AccessibilityMusicOutputSetMenu_OperationCheck:	// 操作確認音出力.
			e_Category = APIE_MusicCategoryOpeConfirm;
			break;
		case PNLD_AccessibilityMusicOutputSetMenu_NormalEnd:		// 正常終了音出力.
			e_Category = APIE_MusicCategoryNormalEnd;
			break;
		case PNLD_AccessibilityMusicOutputSetMenu_Preparation:		// 準備完了音出力.
			e_Category = APIE_MusicCategoryPrepEnd;
			break;
		case PNLD_AccessibilityMusicOutputSetMenu_Warning:			// 注意音出力.
			e_Category = APIE_MusicCategoryWarning;
			break;
		case PNLD_AccessibilityMusicOutputSetMenu_Delete:			// 削除音出力.
			e_Category = APIE_MusicCategoryDelete;
			break;
		case PNLD_AccessibilityMusicOutputSetMenu_Auth:				// 認証音出力.
			e_Category = APIE_MusicCategoryAuth;
			break;
		default:
			break;
	}
	pc_Music->setCategoryEnable( e_Category, b_YesNo );			// 分類単位の有効無効設定.
}

// ====================================================================================================================.
//	機能：設定終了処理(反応音メニュー選択画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		uc_option	オプション(ボタンオプション).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityAuthMusicOutputSet::procOk( uchar uc_option )
{
	if( NULL != m_pc_EndGamen ) {
		PNLC_FromBeforeGamen* pc_FromBeforeGamen = PNLC_FromBeforeGamen::getInstance();
		pc_FromBeforeGamen->setFromBeforeGamen( True );		// 上位画面からの遷移設定.
	}
	PAccessibilityMusicMenuCom::procOk( uc_option );
}

//関数------------------------------------------------------------------------------------------------------------------.
//概要    ：画面の音声情報を設定(408A4100_音出力設定).
//備考    ：.
//戻値    ：.
//作成者  ：kuroda.
//レビュー：.
//変更履歴：.
//----------------------------------------------------------------------------------------------------------------------.
void PAccessibilityMusicOutputSet::setVoiceGamenInfo()
{
	switch( m_uc_PageNum ) {
		case 1:
			m_pstVoiceGamenInfo = &VGI_AccessibilitySoundOutputSet_MiB_P1;
			break;
		case 2:
			m_pstVoiceGamenInfo = &VGI_AccessibilitySoundOutputSet_MiB_P2;
			break;
		default:
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return;
	}
}

static const struct SetupMessageTbl {
	APIE_MusicKind					e_MusicKind;			// 反応音選択種類.
	TEXT_ID							tid_string;				// 文字列.
} st_SetupMessageTbl[] = {
	{ APIE_MusicKindInputOk,		s_ENLMSGMusicInputOkTypeVolumeSet	},		// 入力確認音.
	{ APIE_MusicKindInputNg,		s_ENLMSGMusicInputNgTypeVolumeSet2	},		// 入力無効音.
	{ APIE_MusicKindCardinalPoint,	s_ENLMSGMusicCardinalPointTypeVolumeSet2	},	// 基点音.
	{ APIE_MusicKindOperation,		s_ENLMSGMusicOperationTypeVolumeSet	},		// 操作終了音.
	{ APIE_MusicKindCommunication,	s_ENLMSGMusicCommunicationTypeVolumeSet	},	// 通信終了音.
	{ APIE_MusicKindPreparation,	s_ENLMSGMusicPreparationTypeVolumeSet	},	// 準備完了音.
	{ APIE_MusicKindWarning1,		s_ENLMSGMusicLevel1TypeVolumeSet	},		// 弱注意音<level-1>.
	{ APIE_MusicKindWarning2,		s_ENLMSGMusicLevel2TypeVolumeSet	},		// 弱注意音<level-2>.
	{ APIE_MusicKindWarning3,		s_ENLMSGMusicLevel3TypeVolumeSet	},		// 弱注意音<level-3>.
	{ APIE_MusicKindError,			s_ENLMSGMusicStrongTypeVolumeSet	},		// 強注意音.
	{ APIE_MusicKindDelete,			s_ENLMSGMusicDeleteTypeVolumeSet	},		// 削除音.
	{ APIE_MusicKindMotion1,		s_ENLMSGMusicPopupTypeVolumeSet3	},			// ポップアップ音.
	{ APIE_MusicKindMotion2,		s_ENLMSGMusicKeyboardTypeVolumeSet	},		// キーボード音.
	{ APIE_MusicKindPcPrint,		s_ENLMSGMusicPcPrintTypeVolumeSet2	},		// PCプリント完了音.
	{ APIE_MusicKindAuthOk,			s_ENLMSGMusicAuthOkTypeVolumeSet	},		// 認証成功音.
	{ APIE_MusicKindAuthNg,			s_ENLMSGMusicAuthNgTypeVolumeSet	},		// 認証失敗音.
	{ APIE_MusicKindFaxPrint,		s_ENLMSGMusicFaxPrintTypeVolumeSet3	},		// FAXプリント完了音.
	{ APIE_MusicKindMax,			s_KEYSoundSetting	},						// 種類最大数.
};

// ====================================================================================================================.
//	機能：コンストラクタ(反応音種類/音量設定画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		pc_partsTable		画面パーツテーブル.
//		ss_partsNumber		画面パーツ数.
//		b_isNoWarningGamen	警告表示禁止指定.
//		e_MusicKind			反応音小分類.
//		uc_SelectNum		選択種類:数値ボタン数.
//		pc_EndGamen			終了画面.
///	\par	履歴:.\n
// ====================================================================================================================.
PAccessibilityMusicTypeAndVolumeSetCom::PAccessibilityMusicTypeAndVolumeSetCom( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen,
        APIE_MusicKind e_MusicKind, uchar uc_SelectNum, PBasicMessageProc* pc_EndGamen )
	: PAccessibilityTopMenuCom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen ),
	  m_e_MusicKind( e_MusicKind )
	  , m_e_MusicVolume( APIE_MusicVolumeDefault )
	  , m_e_MusicSelect( APIE_MusicSelect1 )
	  , m_b_CustomEnable( False )
	  , m_b_TabletKeyView( False )
	  , m_uc_SelectNum( uc_SelectNum )
	  , m_pc_EndGamen( pc_EndGamen )					// 静的解析 2012/09/05 第一OPS 酒井(健).
	  , m_b_YesNoKey( False )								// 静的解析 2012/09/05 第一OPS 酒井(健).
	  , m_uc_MessageIndex( 0 )	// 静的解析 2012/09/05 第一OPS 酒井(健).
{
}

// ====================================================================================================================.
//	機能：デフォルト背景設定(反応音種類/音量設定画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicTypeAndVolumeSetCom::setDefaultBackParts()
{
	setDefaultBackPartsInfo( ( PDisplayParts** )tbl_AccessibilityMusicTypeAndVolumeSetDefault,
	                         ss_AccessibilityMusicTypeAndVolumeSetDefault );
}

// ====================================================================================================================.
//	機能：画面開始処理(反応音種類/音量設定画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicTypeAndVolumeSetCom::startGamen()
{
	Bool bl_isContinue = bl_isUnder;	// comment 2012/08/07 OPS1-Sky浜野 [MinervaBK][CQ368917] メッセージ拡大画面からの戻り時に設定値を変更しない対応.

	PAccessibilityTopMenuCom::startGamen();
	// 反応音選択種類からメッセージテキスト検索.
	for( m_uc_MessageIndex = 0; st_SetupMessageTbl[ m_uc_MessageIndex ].e_MusicKind != APIE_MusicKindMax; m_uc_MessageIndex++ ) {
		if( st_SetupMessageTbl[ m_uc_MessageIndex ].e_MusicKind == m_e_MusicKind ) {				// 反応音選択種類が一致.
			break;
		}
	}
	APIC_Music* pc_Music = APIC_Music::newInstance();
	PLBC_MusicKind* pc_MusicKind = PLBC_MusicKind::getInstance();
	PNLC_FromBeforeGamen* pc_FromBeforeGamen = PNLC_FromBeforeGamen::getInstance();
	// 下層からの遷移以外.
	if( True != bl_isContinue ) {
		m_e_MusicVolume = pc_Music->getVolume( m_e_MusicKind );				// 反応音ボリューム取得.
		m_e_MusicSelect = pc_Music->getMusicSelect( m_e_MusicKind );		// 反応音選択種類.
	}
	pc_MusicKind->setFromMusicListDisp( False );							// 端末音選択画面遷移状態の設定.
	pc_FromBeforeGamen->setFromBeforeGamen( False );						// 遷移後のOFF設定.

	if( m_uc_SelectNum < m_e_MusicSelect ) {									// 指定不可チェック.
		m_e_MusicSelect = APIE_MusicSelect1;								// ブザー音指定.
	}
	// 下層からの遷移以外.
	if( True != bl_isContinue ) {
		m_b_YesNoKey = pc_Music->isEnable( m_e_MusicKind );						// する/しないの取得.
	}
	APIC_MusicCustomize* pc_MusicCustomize = APIC_MusicCustomize::newInstance();
	m_b_CustomEnable = pc_MusicCustomize->getCustomEnable( m_e_MusicKind );	// カスタム有効/無効.

	APIC_TabletIF* pc_TabletIF = APIC_TabletIF::newInstance();
	// 端末音キー表示条件(音声カスタマイズ機能が有効の場合のみ、端末音キーを表示する).
	m_b_TabletKeyView = False;
	if( True == pc_TabletIF->getConnectState() ) {													// 遠隔操作端末が接続されていることを確認する.
		if( APIE_TabletOptionEnable == pc_TabletIF->getOptionEnable( APIE_TabletMusicCustomize ) ) {	// 音声カスタマイズ機能が有効であることを確認する.
			m_b_TabletKeyView = True;
		}
	}
	if( False == m_b_TabletKeyView ) {										// 未接続時の対応.
		m_b_CustomEnable = False;											// カスタム無効.
	}

	uchar uc_SelectNo;
	for( uc_SelectNo = 0; uc_SelectNo < PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Count; uc_SelectNo++ ) {	// 種類の音が重複するためボタンのブザー設定をOFF.
		Key_AccessibilityMusicTypeAndVolumeSetMusic[ uc_SelectNo ].setBuzzer( False );
	}
	Key_AccessibilityMusicTypeAndVolumeSetSmall.setBuzzer( False );			// 小キー押下時の音が重複するためボタンのブザー設定をOFF.
	Key_AccessibilityMusicTypeAndVolumeSetNormal.setBuzzer( False );		// 中キー押下時の音が重複するためボタンのブザー設定をOFF.
	Key_AccessibilityMusicTypeAndVolumeSetLoud.setBuzzer( False );			// 大キー押下時の音が重複するためボタンのブザー設定をOFF.
}

// ====================================================================================================================.
//	機能：画面セットアップ処理(反応音種類/音量設定画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicTypeAndVolumeSetCom::setupGamen( short ss_option )
{
	PAccessibilityTopMenuCom::setupGamen();
	setupMessageAreaAccessibility( st_SetupMessageTbl[ m_uc_MessageIndex ].tid_string );		// メッセージ欄のテキスト変更.

	uchar uc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Music1;			// 初期値を音1に設定.

	switch( m_e_MusicSelect ) {		// 種類選択.
		case APIE_MusicSelect1:			// 音1.
			uc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Music1;
			break;
		case APIE_MusicSelect2:			// 音2.
			uc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Music2;
			break;
		case APIE_MusicSelect3:			// 音3.
			uc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Music3;
			break;
		case APIE_MusicSelect4:			// 音4.
			uc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Music4;
			break;
		case APIE_MusicSelect5:			// 音5.
			uc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Music5;
			break;
		case APIE_MusicSelect6:			// 音6.
			uc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Music6;
			break;
		default:
			if( APIE_MusicKindFaxPrint != m_e_MusicKind ) {		// デフォルト音はFaxプリント音のみ音2、他は音1.
				uc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Music1;
			} else {
				uc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Music2;
			}
			break;
	}
	if( True == m_b_CustomEnable ) {	// カスタム有効優先.
		uc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Tablet;
	}
	Rbt_AccessibilityMusicTypeAndVolumeSetType.select( uc_SelectIndex );

	// 種類表示の有無指定.
	uchar uc_SelectNo;
	for( uc_SelectNo = 0; uc_SelectNo < PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Count; uc_SelectNo++ ) {
		if( uc_SelectNo < m_uc_SelectNum ) {
			Key_AccessibilityMusicTypeAndVolumeSetMusic[ uc_SelectNo ].showEnable( True );	// 表示.
		} else {
			Key_AccessibilityMusicTypeAndVolumeSetMusic[ uc_SelectNo ].showEnable( False );	// 非表示.
		}
	}

	if( True == m_b_TabletKeyView ) {										// 端末音キーの表示.
		Key_AccessibilityMusicTypeAndVolumeSetTablet.showEnable( True );	// 表示.
	} else {
		Key_AccessibilityMusicTypeAndVolumeSetTablet.showEnable( False );	// 非表示.
	}

	// 反応音ボリューム選択.
	uc_SelectIndex = 0;
	switch( m_e_MusicVolume ) {
		case APIE_MusicVolumeLevel1:	// ブザー音(小).
			uc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Small;
			break;
		case APIE_MusicVolumeLevel2:	// ブザー音(中).
			uc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Normal;
			break;
		case APIE_MusicVolumeLevel3:	// ブザー音(大).
			uc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Loud;
			break;
		default:
			break;
	}

	if( True == m_b_YesNoKey ) {													// する/しないキーの選択.
		Rbt_AccessibilityMusicTypeAndVolumeSetYesNo.select( 0 );				// するキー選択.
	} else {
		Rbt_AccessibilityMusicTypeAndVolumeSetYesNo.select( 1 );				// しないキー選択.
	}
	Key_AccessibilityMusicTypeAndVolumeSetTablet.activate( m_b_YesNoKey );		// タブレットキー表示切替.
	Key_AccessibilityMusicTypeAndVolumeSetSmall.activate( m_b_YesNoKey );		// 小キー表示切替.
	Key_AccessibilityMusicTypeAndVolumeSetNormal.activate( m_b_YesNoKey );		// 中キー表示切替.
	Key_AccessibilityMusicTypeAndVolumeSetLoud.activate( m_b_YesNoKey );		// 大キー表示切替.
	for( uc_SelectNo = 0; uc_SelectNo < PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Count; uc_SelectNo++ ) {
		Key_AccessibilityMusicTypeAndVolumeSetMusic[ uc_SelectNo ].activate( m_b_YesNoKey );	// 種類キー表示切替.
	}

	Rbt_AccessibilityMusicTypeAndVolumeSetVolume.select( uc_SelectIndex );
}

// ====================================================================================================================.
//	機能：ボタン押下処理(反応音種類/音量設定画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		uc_option	オプション(ボタンオプション).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicTypeAndVolumeSetCom::procButtonOn( uchar uc_option )
{
	APIC_Music* pc_Music = APIC_Music::newInstance();
	switch( uc_option ) {
		// 種類.
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Music1:	// 種類:音1キー.
			m_e_MusicSelect = APIE_MusicSelect1;
			m_b_CustomEnable = False;
			break;
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Music2:	// 種類:音2キー.
			m_e_MusicSelect = APIE_MusicSelect2;
			m_b_CustomEnable = False;
			break;
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Music3:	// 種類:音3キー.
			m_e_MusicSelect = APIE_MusicSelect3;
			m_b_CustomEnable = False;
			break;
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Music4:	// 種類:音4キー.
			m_e_MusicSelect = APIE_MusicSelect4;
			m_b_CustomEnable = False;
			break;
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Music5:	// 種類:音5キー.
			m_e_MusicSelect = APIE_MusicSelect5;
			m_b_CustomEnable = False;
			break;
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Music6:	// 種類:音6キー.
			m_e_MusicSelect = APIE_MusicSelect6;
			m_b_CustomEnable = False;
			break;

		// 音量.
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Small:		// 音量:小キー.
			m_e_MusicVolume = APIE_MusicVolumeLevel1;
			break;
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Normal:	// 音量:中キー.
			m_e_MusicVolume = APIE_MusicVolumeLevel2;
			break;
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Loud:		// 音量:大キー.
			m_e_MusicVolume = APIE_MusicVolumeLevel3;
			break;

		case PNLD_AccessibilityMusicTypeAndVolumeSet_Yes:		// するキー.
			m_b_YesNoKey = True;
			break;
		case PNLD_AccessibilityMusicTypeAndVolumeSet_No:		// しないキー.
			m_b_YesNoKey = False;
			break;
		default:
			break;
	}
	// 種類と音量が選択された場合、音を再生.
	if( ( PNLD_AccessibilityMusicTypeAndVolumeSet_Music1 <= uc_option
	      && uc_option <= PNLD_AccessibilityMusicTypeAndVolumeSet_Music6 )
	    || ( PNLD_AccessibilityMusicTypeAndVolumeSet_Small <= uc_option
	         && uc_option <= PNLD_AccessibilityMusicTypeAndVolumeSet_Loud ) ) {
		setBuzzerType( PCGE_BuzzerType_Off, True ); // 受付音を強制的に無効にする.
		pc_Music->reqMusicTest( m_e_MusicKind, m_e_MusicSelect, m_e_MusicVolume );
	}
	setupGamen();
}

// ====================================================================================================================.
//	機能：画面移動処理(反応音種類/音量設定画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		uc_option	オプション(ボタンオプション).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicTypeAndVolumeSetCom::procGoToPage( uchar uc_option )
{
	PLBC_MusicKind* pc_MusicKind = PLBC_MusicKind::getInstance();
	switch( uc_option ) {
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Tablet:	// 端末音キー.
			pc_MusicKind->setMusicKind( m_e_MusicKind );		// 反応音小分類の設定.
			pc_MusicKind->setEndGamen( getNowGamen() );			// 戻り先画面の設定.
			pc_MusicKind->setMusicVolume( m_e_MusicVolume );	// 反応音ボリューム設定(試聴用).
			pc_MusicKind->setFromMusicListDisp( True );			// 端末音選択画面遷移状態の設定.
			pc_MusicKind->setMusicSelect( m_e_MusicSelect );	// 反応音選択種類設定(試聴用).
			nextGamen = &gp_AccessibilityMusicListSet;			// 端末音設定画面へ遷移.
			break;
		default:
			break;
	}
}

// ====================================================================================================================.
//	機能：設定終了処理(反応音種類/音量設定画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		uc_option	オプション(ボタンオプション).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicTypeAndVolumeSetCom::procOk( uchar uc_option )
{
	setTypeAndVolume();										// 終了時の設定処理.

	if( NULL != m_pc_EndGamen ) {
		PNLC_FromBeforeGamen* pc_FromBeforeGamen = PNLC_FromBeforeGamen::getInstance();
		pc_FromBeforeGamen->setFromBeforeGamen( True );		// 上位画面からの遷移設定.
		nextGamen = m_pc_EndGamen;							// 終了(OK)後の遷移先へ.
	} else {
		PAccessibilityTopMenuCom::procOk( uc_option );
	}
}

// ====================================================================================================================.
//	機能：反応音種類/音量設定処理(反応音種類/音量設定画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityMusicTypeAndVolumeSetCom::setTypeAndVolume()
{
	APIC_Music* pc_Music = APIC_Music::newInstance();
	if( True == m_b_YesNoKey ) {
		pc_Music->setVolume( m_e_MusicKind, m_e_MusicVolume );		// 反応音ボリューム.
		pc_Music->setMusicSelect( m_e_MusicKind, m_e_MusicSelect );	// 反応音選択種類.

		if( True == m_b_TabletKeyView ) {							// 接続時(端末音キー表示時)のみ、カスタム有効/無効を設定する.
			APIC_MusicCustomize* pc_MusicCustomize = APIC_MusicCustomize::newInstance();
			pc_MusicCustomize->setCustomEnable( m_e_MusicKind, m_b_CustomEnable );
		}
	}
	pc_Music->setEnable( m_e_MusicKind, m_b_YesNoKey );				// 出力有無.

}

// ====================================================================================================================.
//	機能：カスタムID番号取得処理(反応音種類選択画面共通クラス).
//	-------------------------------------------------------------------------------------------------------------------.
//	戻値：.
//		short	カスタムID番号.
///	\par	履歴:.\n
// ====================================================================================================================.
short PAccessibilityMusicTypeAndVolumeSetCom::getCustomIdNo()
{
	short ss_IdNo = APID_MusicCustomizeIdNone;
	PLBC_MusicKind* pc_MusicKind = PLBC_MusicKind::getInstance();
	APIC_MusicCustomize* pc_MusicCustomize = APIC_MusicCustomize::newInstance();
	APIS_MusicCustomizeAdminInfo st_AdmInf;
	memset( &st_AdmInf, 0x0, sizeof( st_AdmInf ) );

	pc_MusicKind->getMusicCustomAdmInfo( &st_AdmInf );					// 管理情報取得.
	pc_MusicCustomize->getIdNo( &st_AdmInf, m_e_MusicKind, &ss_IdNo );	// カスタムID番号取得.
	return ss_IdNo;
}

//関数------------------------------------------------------------------------------------------------------------------.
//概要    ：画面の音声情報を設定(反応音種類/音量設定画面共通クラス).
//備考    ：.
//戻値    ：.
//作成者  ：kuroda.
//レビュー：.
//変更履歴：.
//----------------------------------------------------------------------------------------------------------------------.
void PAccessibilityMusicTypeAndVolumeSetCom::setVoiceGamenInfo()
{
	switch( m_e_MusicKind ) {
		case APIE_MusicKindInputOk:			// 入力確認音.
			m_pstVoiceGamenInfo = &VGI_AccessibilityInputConfirmSound_MiB;
			break;
		case APIE_MusicKindInputNg:			// 入力無効音.
			m_pstVoiceGamenInfo = &VGI_AccessibilityInputCancelSound_MiB;
			break;
		case APIE_MusicKindCardinalPoint:	// 基点音.
			m_pstVoiceGamenInfo = &VGI_AccessibilityBasicSound_MiB;
			break;
		case APIE_MusicKindOperation:		// 操作終了音.
			m_pstVoiceGamenInfo = &VGI_AccessibilityOperationCompleteSound_MiB;
			break;
		case APIE_MusicKindCommunication:	// 通信終了音.
			m_pstVoiceGamenInfo = &VGI_AccessibilityConnectCompleteSound_MiB;
			break;
		case APIE_MusicKindPreparation:		// 準備完了音.
			m_pstVoiceGamenInfo = &VGI_AccessibilityReadyCompleteSound_MiB;
			break;
		case APIE_MusicKindWarning1:		// 弱注意音(Level1).
			m_pstVoiceGamenInfo = &VGI_AccessibilityAttentionSoundSoftLevel1Sound_MiB;
			break;
		case APIE_MusicKindWarning2:		// 弱注意音(Level2).
			m_pstVoiceGamenInfo = &VGI_AccessibilityAttentionSoundSoftLevel2Sound_MiB;
			break;
		case APIE_MusicKindWarning3:		// 弱注意音(Level3).
			m_pstVoiceGamenInfo = &VGI_AccessibilityAttentionSoundSoftLevel3Sound_MiB;
			break;
		case APIE_MusicKindError:			// 強注意音.
			m_pstVoiceGamenInfo = &VGI_AccessibilityAttentionSoundStrongSound_MiB;
			break;
		case APIE_MusicKindDelete:			// 削除音.
			m_pstVoiceGamenInfo = &VGI_AccessibilityDeletionSound_MiB;
			break;
		case APIE_MusicKindMotion1:			// ウィンドウポップアップ音.
			m_pstVoiceGamenInfo = &VGI_AccessibilityWindowPopUpSound_MiB;
			break;
		case APIE_MusicKindMotion2:			// キーボード操作音.
			m_pstVoiceGamenInfo = &VGI_AccessibilityKeyboardOperationSound_MiB;
			break;
		case APIE_MusicKindPcPrint:			// プリンター印刷完了音.
			m_pstVoiceGamenInfo = &VGI_AccessibilityPrintJobCompleteSound_MiB;
			break;
		case APIE_MusicKindAuthOk:			// 認証成功.
			m_pstVoiceGamenInfo = &VGI_AccessibilityAuthenticationSuccessfulSound_MiB;
			break;
		case APIE_MusicKindAuthNg:			// 認証失敗.
			m_pstVoiceGamenInfo = &VGI_AccessibilityAuthenticationFailureSound_MiB;
			break;
		case APIE_MusicKindFaxPrint:		// ファクス受信印刷完了音.
			m_pstVoiceGamenInfo = &VGI_AccessibilityFaxRXPrintCompleteSound_MiB;
			break;
		default:
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return;
	}
}


// ====================================================================================================================.
//	機能：画面開始処理(90NA4200:音量／種類設定画面).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：.
//	戻値    ：.
// ====================================================================================================================.
void PAccessibilityMusicTypeAndVolumeBatchSet::startGamen()
{
	PAccessibilityTopMenuCom::startGamen();

	APIC_Music* pc_Music = APIC_Music::newInstance();
	// 反応音ボリューム取得.
	m_e_MusicVolumeValue[0] = pc_Music->getVolume( APIE_MusicKindInputOk );			// 入力確認音.
	m_e_MusicVolumeValue[1] = pc_Music->getVolume( APIE_MusicKindInputNg );			// 入力無効音.
	m_e_MusicVolumeValue[2] = pc_Music->getVolume( APIE_MusicKindCardinalPoint );	// 基点音.
	m_e_MusicVolumeValue[3] = pc_Music->getVolume( APIE_MusicKindOperation );		// 操作終了音.
	m_e_MusicVolumeValue[4] = pc_Music->getVolume( APIE_MusicKindCommunication );	// 通信終了音.
	m_e_MusicVolumeValue[5] = pc_Music->getVolume( APIE_MusicKindPreparation );		// 準備完了音.
	m_e_MusicVolumeValue[6] = pc_Music->getVolume( APIE_MusicKindWarning1 );		// 弱注意音<level-1>.
	m_e_MusicVolumeValue[7] = pc_Music->getVolume( APIE_MusicKindWarning2 );		// 弱注意音<level-2>.
	m_e_MusicVolumeValue[8] = pc_Music->getVolume( APIE_MusicKindWarning3 );		// 弱注意音<level-3>.
	m_e_MusicVolumeValue[9] = pc_Music->getVolume( APIE_MusicKindError );			// 強注意音.
	m_e_MusicVolumeValue[10] = pc_Music->getVolume( APIE_MusicKindDelete );			// 削除音.
	m_e_MusicVolumeValue[11] = pc_Music->getVolume( APIE_MusicKindMotion1 );		// モーション１（ポップアップ）.
	m_e_MusicVolumeValue[12] = pc_Music->getVolume( APIE_MusicKindMotion2 );		// モーション２（キーボード）.
	m_e_MusicVolumeValue[13] = pc_Music->getVolume( APIE_MusicKindPcPrint );		// PCプリント完了音.
	m_e_MusicVolumeValue[14] = pc_Music->getVolume( APIE_MusicKindAuthOk );			// 認証成功.
	m_e_MusicVolumeValue[15] = pc_Music->getVolume( APIE_MusicKindAuthNg );			// 認証失敗.
	m_e_MusicVolumeValue[16] = pc_Music->getVolume( APIE_MusicKindFaxPrint );		// FAXプリント完了音.
	// ボリュームの一致チェック.
	m_bl_MusicVolumeBatch = True;
	for( uchar i = 0; i < APIE_MusicKindMax; i++ ) {
		if( m_e_MusicVolumeValue[0] != m_e_MusicVolumeValue[i] ) {
			m_bl_MusicVolumeBatch = False;			// 一致していないをセット.
			break;
		}
	}
	m_e_MusicVolume = m_e_MusicVolumeValue[0];	// 先頭データをセット.

	// 反応音選択種類.
	m_e_MusicSelectValue[0] = pc_Music->getMusicSelect( APIE_MusicKindInputOk );		// 入力確認音.
	m_e_MusicSelectValue[1] = pc_Music->getMusicSelect( APIE_MusicKindInputNg );		// 入力無効音.
	m_e_MusicSelectValue[2] = pc_Music->getMusicSelect( APIE_MusicKindCardinalPoint );	// 基点音.
	m_e_MusicSelectValue[3] = pc_Music->getMusicSelect( APIE_MusicKindOperation );		// 操作終了音.
	m_e_MusicSelectValue[4] = pc_Music->getMusicSelect( APIE_MusicKindCommunication );	// 通信終了音.
	m_e_MusicSelectValue[5] = pc_Music->getMusicSelect( APIE_MusicKindPreparation );	// 準備完了音.
	m_e_MusicSelectValue[6] = pc_Music->getMusicSelect( APIE_MusicKindWarning1 );		// 弱注意音<level-1>.
	m_e_MusicSelectValue[7] = pc_Music->getMusicSelect( APIE_MusicKindWarning2 );		// 弱注意音<level-2>.
	m_e_MusicSelectValue[8] = pc_Music->getMusicSelect( APIE_MusicKindWarning3 );		// 弱注意音<level-3>.
	m_e_MusicSelectValue[9] = pc_Music->getMusicSelect( APIE_MusicKindError );			// 強注意音.
	m_e_MusicSelectValue[10] = pc_Music->getMusicSelect( APIE_MusicKindDelete );		// 削除音.
	m_e_MusicSelectValue[11] = pc_Music->getMusicSelect( APIE_MusicKindMotion1 );		// モーション１（ポップアップ）.
	m_e_MusicSelectValue[12] = pc_Music->getMusicSelect( APIE_MusicKindMotion2 );		// モーション２（キーボード）.
	m_e_MusicSelectValue[13] = pc_Music->getMusicSelect( APIE_MusicKindPcPrint );		// PCプリント完了音.
	m_e_MusicSelectValue[14] = pc_Music->getMusicSelect( APIE_MusicKindAuthOk );		// 認証成功.
	m_e_MusicSelectValue[15] = pc_Music->getMusicSelect( APIE_MusicKindAuthNg );		// 認証失敗.
	m_e_MusicSelectValue[16] = pc_Music->getMusicSelect( APIE_MusicKindFaxPrint );		// FAXプリント完了音.
	// 音種類の一致チェック.
	m_bl_MusicSelectBatch = True;
	for( uchar i = 0; i < APIE_MusicKindMax; i++ ) {
		if( m_e_MusicSelectValue[0] != m_e_MusicSelectValue[i] ) {
			m_bl_MusicSelectBatch = False;			// 一致していないをセット.
			break;
		}
	}
	m_e_MusicSelect = m_e_MusicSelectValue[0];	// 先頭データをセット.

	for( uchar uc_SelectNo = 0; uc_SelectNo < PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Count; uc_SelectNo++ ) {	// 種類の音が重複するためボタンのブザー設定をOFF.
		Key_AccessibilityMusicTypeAndVolumeSetMusic[ uc_SelectNo ].setBuzzer( False );
	}
	Key_AccessibilityMusicTypeAndVolumeSetSmall.setBuzzer( False );			// 小キー押下時の音が重複するためボタンのブザー設定をOFF.
	Key_AccessibilityMusicTypeAndVolumeSetNormal.setBuzzer( False );		// 中キー押下時の音が重複するためボタンのブザー設定をOFF.
	Key_AccessibilityMusicTypeAndVolumeSetLoud.setBuzzer( False );			// 大キー押下時の音が重複するためボタンのブザー設定をOFF.

	m_b_BatchOrIndividualKey = False;
	if( True == m_bl_MusicVolumeBatch && True == m_bl_MusicSelectBatch ) {	// 音の種類と大きさがともに全種類一致している.
		m_b_BatchOrIndividualKey = True;
	}
}

// ====================================================================================================================.
//	機能：画面セットアップ処理(90NA4200:音量／種類設定画面).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：ss_option.
//	戻値    ：.
// ====================================================================================================================.
void PAccessibilityMusicTypeAndVolumeBatchSet::setupGamen( short ss_option )
{
	PAccessibilityTopMenuCom::setupGamen();
	Key_AccessibilityMusicTypeAndVolumeSetTablet.showEnable( False );
	setupMessageAreaAccessibility( s_MSG_MusicBatchAndIndividualSet2 );	// メッセージ欄のセットアップ.

	char sc_SelectIndex = -1;		// 初期値を選択しないに設定.
	if( True == m_bl_MusicSelectBatch ) {					// 反応音種類が全種類一致している場合.
		switch( m_e_MusicSelect ) {		// 種類選択.
			case APIE_MusicSelect1:			// Light.
				sc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Music1;
				break;
			case APIE_MusicSelect2:			// Ceramic.
				sc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Music2;
				break;
			case APIE_MusicSelect3:			// Future.
				sc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Music3;
				break;
			case APIE_MusicSelect4:			// Wood.
				sc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Music4;
				break;
			case APIE_MusicSelect5:			// Solid.
				sc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Music5;
				break;
			case APIE_MusicSelect6:			// Signal.
				sc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Music6;
				break;
			default:
				break;
		}
	} else {
		sc_SelectIndex = -1;	// 反応音種類が全種類一致していない場合は非選択状態.
	}
	Rbt_AccessibilityMusicTypeAndVolumeSetType.select( sc_SelectIndex );	// キー選択.

	// 反応音ボリューム選択.
	sc_SelectIndex = -1;
	if( True == m_bl_MusicVolumeBatch ) {
		switch( m_e_MusicVolume ) {
			case APIE_MusicVolumeLevel1:	// ブザー音(小).
				sc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Small;
				break;
			case APIE_MusicVolumeLevel2:	// ブザー音(中).
				sc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Normal;
				break;
			case APIE_MusicVolumeLevel3:	// ブザー音(大).
				sc_SelectIndex = PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Loud;
				break;
			default:
				break;
		}
	} else {
		sc_SelectIndex = -1;	// 大きさが一致していない場合は非選択状態.
	}
	Rbt_AccessibilityMusicTypeAndVolumeSetVolume.select( sc_SelectIndex );	// キー選択.

	// 一括/個別キーの選択.
	if( True == m_b_BatchOrIndividualKey ) {									// 音の種類と大きさがともに全種類一致しているとき.
		Rbt_AccessibilityMusicTypeAndVolumeSetBatchIndividual.select( 0 );		// 一括キー選択.
		Key_AccessibilityMusicTypeAndVolumeChangeIndividualSetting.activate( False );		// 個別設定変更キーを非活性状態にする.
	} else {
		Rbt_AccessibilityMusicTypeAndVolumeSetBatchIndividual.select( 1 );		// 個別キー選択.
		Key_AccessibilityMusicTypeAndVolumeChangeIndividualSetting.activate( True );		// 個別設定変更キーを活性状態にする.
	}
	Key_AccessibilityMusicTypeAndVolumeSetSmall.activate( m_b_BatchOrIndividualKey );		// 小キー表示切替.
	Key_AccessibilityMusicTypeAndVolumeSetNormal.activate( m_b_BatchOrIndividualKey );		// 中キー表示切替.
	Key_AccessibilityMusicTypeAndVolumeSetLoud.activate( m_b_BatchOrIndividualKey );		// 大キー表示切替.

	for( uchar uc_SelectNo = 0; uc_SelectNo < PNLD_AccessibilityMusicTypeAndVolumeSetSelect_Count; uc_SelectNo++ ) {
		Key_AccessibilityMusicTypeAndVolumeSetMusic[ uc_SelectNo ].activate( m_b_BatchOrIndividualKey );	// 音の種類キー表示切替.
	}
}

// ====================================================================================================================.
//	機能：ボタン押下処理(90NA4200:音量／種類設定画面).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：uc_option.
//	戻値    ：.
// ====================================================================================================================.
void PAccessibilityMusicTypeAndVolumeBatchSet::procButtonOn( uchar uc_option )
{
	APIC_Music* pc_Music = APIC_Music::newInstance();
	Bool bl_KindOrVolumPush = False;						// 種類キーもしくは大きさキー押下時にフラグを立てる.
	switch( uc_option ) {
		// 種類.
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Music1:	// 種類:Lightキー.
			m_e_MusicSelect = APIE_MusicSelect1;
			m_bl_MusicSelectBatch = True;
			bl_KindOrVolumPush = True;
			break;
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Music2:	// 種類:Ceramicキー.
			m_e_MusicSelect = APIE_MusicSelect2;
			m_bl_MusicSelectBatch = True;
			bl_KindOrVolumPush = True;
			break;
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Music3:	// 種類:Futureキー.
			m_e_MusicSelect = APIE_MusicSelect3;
			m_bl_MusicSelectBatch = True;
			bl_KindOrVolumPush = True;
			break;
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Music4:	// 種類:Woodキー.
			m_e_MusicSelect = APIE_MusicSelect4;
			m_bl_MusicSelectBatch = True;
			bl_KindOrVolumPush = True;
			break;
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Music5:	// 種類:Solidキー.
			m_e_MusicSelect = APIE_MusicSelect5;
			m_bl_MusicSelectBatch = True;
			bl_KindOrVolumPush = True;
			break;
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Music6:	// 種類:Signalキー.
			m_e_MusicSelect = APIE_MusicSelect6;
			m_bl_MusicSelectBatch = True;
			bl_KindOrVolumPush = True;
			break;

		// 音量.
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Small:		// 音量:小キー.
			m_e_MusicVolume = APIE_MusicVolumeLevel1;
			m_bl_MusicVolumeBatch = True;
			bl_KindOrVolumPush = True;
			break;
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Normal:	// 音量:中キー.
			m_e_MusicVolume = APIE_MusicVolumeLevel2;
			m_bl_MusicVolumeBatch = True;
			bl_KindOrVolumPush = True;
			break;
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Loud:		// 音量:大キー.
			m_e_MusicVolume = APIE_MusicVolumeLevel3;
			m_bl_MusicVolumeBatch = True;
			bl_KindOrVolumPush = True;
			break;

		// 一括／個別.
		case PNLD_AccessibilityMusicTypeAndVolumeSet_Yes:		// 一括キー.
			m_b_BatchOrIndividualKey = True;
			break;
		case PNLD_AccessibilityMusicTypeAndVolumeSet_No:		// 個別キー.
			m_b_BatchOrIndividualKey = False;
			break;

		default:
			break;
	}

	// 種類・大きさが押下されたとき.
	if( True == bl_KindOrVolumPush ) {
		// 音の種類・音量が両方選択されたときのみ音を鳴らす.
		if( True == m_bl_MusicSelectBatch && True == m_bl_MusicVolumeBatch ) {
			setBuzzerType( PCGE_BuzzerType_Off, True );											// 受付音を強制的に無効にする.
			pc_Music->reqMusicTest( APIE_MusicKindInputOk, m_e_MusicSelect, m_e_MusicVolume );	// 入力確認音を鳴らす.
		}
	}
	setupGamen();
}

// ====================================================================================================================.
//	機能：画面移動処理(90NA4200:音量／種類設定画面).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：uc_option.
//	戻値    ：.
// ====================================================================================================================.
void PAccessibilityMusicTypeAndVolumeBatchSet::procGoToPage( uchar uc_option )
{
	gp_AccessibilityMusicIndividualMenu.initialize();		// ページ数を初期化.
	nextGamen = &gp_AccessibilityMusicIndividualMenu;		// 個別設定画面へ遷移.
}

// ====================================================================================================================.
//	機能：設定終了処理(90NA4200:音量／種類設定画面).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：uc_option.
//	戻値    ：.
// ====================================================================================================================.
void PAccessibilityMusicTypeAndVolumeBatchSet::procOk( uchar uc_option )
{
	setTypeAndVolume();										// 終了時の設定処理.
	nextGamen = &gp_AccessibilityMusicTop;					// 音設定画面へ遷移.
}

// ====================================================================================================================.
//	機能：反応音種類/音量設定処理(90NA4200:音量／種類設定画面).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：.
//	戻値    ：.
// ====================================================================================================================.
void PAccessibilityMusicTypeAndVolumeBatchSet::setTypeAndVolume()
{
	APIC_Music* pc_Music = APIC_Music::newInstance();

	// 種類、ボリュームキーが選択されているときに書込み.
	if( True == m_bl_MusicVolumeBatch ) {
		// 反応音ボリューム.
// comment 2016/05/23 SCD2-Sky浜野 [ZeusSBK][CQ528994] 音一括設定パフォーマンス改善.
//		pc_Music->setVolume( APIE_MusicKindInputOk, m_e_MusicVolume );				// 入力確認音.
//		pc_Music->setVolume( APIE_MusicKindInputNg, m_e_MusicVolume );				// 入力無効音.
//		pc_Music->setVolume( APIE_MusicKindCardinalPoint, m_e_MusicVolume );		// 基点音.
//		pc_Music->setVolume( APIE_MusicKindOperation, m_e_MusicVolume );			// 操作終了音.
//		pc_Music->setVolume( APIE_MusicKindCommunication, m_e_MusicVolume );		// 通信終了音.
//		pc_Music->setVolume( APIE_MusicKindPreparation, m_e_MusicVolume );			// 準備完了音.
//		pc_Music->setVolume( APIE_MusicKindWarning1, m_e_MusicVolume );				// 弱注意音<level-1>.
//		pc_Music->setVolume( APIE_MusicKindWarning2, m_e_MusicVolume );				// 弱注意音<level-2>.
//		pc_Music->setVolume( APIE_MusicKindWarning3, m_e_MusicVolume );				// 弱注意音<level-3>.
//		pc_Music->setVolume( APIE_MusicKindError, m_e_MusicVolume );				// 強注意音.
//		pc_Music->setVolume( APIE_MusicKindDelete, m_e_MusicVolume );				// 削除音.
//		pc_Music->setVolume( APIE_MusicKindMotion1, m_e_MusicVolume );				// ウィンドウポップアップ音.
//		pc_Music->setVolume( APIE_MusicKindMotion2, m_e_MusicVolume );				// キーボード操作音.
//		pc_Music->setVolume( APIE_MusicKindPcPrint, m_e_MusicVolume );				// プリンター印刷完了音.
//		pc_Music->setVolume( APIE_MusicKindAuthOk, m_e_MusicVolume );				// 認証成功音.
//		pc_Music->setVolume( APIE_MusicKindAuthNg, m_e_MusicVolume );				// 認証失敗音.
//		pc_Music->setVolume( APIE_MusicKindFaxPrint, m_e_MusicVolume );				// ファクス受信印刷完了音
		TYPS_MusicVolume st_MusicVolume[] = {
			{ APIE_MusicKindInputOk,		m_e_MusicVolume, False },
			{ APIE_MusicKindInputNg,		m_e_MusicVolume, False },
			{ APIE_MusicKindCardinalPoint,	m_e_MusicVolume, False },
			{ APIE_MusicKindOperation,		m_e_MusicVolume, False },
			{ APIE_MusicKindCommunication,	m_e_MusicVolume, False },
			{ APIE_MusicKindPreparation,	m_e_MusicVolume, False },
			{ APIE_MusicKindWarning1,		m_e_MusicVolume, False },
			{ APIE_MusicKindWarning2,		m_e_MusicVolume, False },
			{ APIE_MusicKindWarning3,		m_e_MusicVolume, False },
			{ APIE_MusicKindError,			m_e_MusicVolume, False },
			{ APIE_MusicKindDelete,			m_e_MusicVolume, False },
			{ APIE_MusicKindMotion1,		m_e_MusicVolume, False },
			{ APIE_MusicKindMotion2,		m_e_MusicVolume, False },
			{ APIE_MusicKindPcPrint,		m_e_MusicVolume, False },
			{ APIE_MusicKindAuthOk,			m_e_MusicVolume, False },
			{ APIE_MusicKindAuthNg,			m_e_MusicVolume, False },
			{ APIE_MusicKindFaxPrint,		m_e_MusicVolume, False },
		};
		pc_Music->setAllVolume( st_MusicVolume, (sizeof( st_MusicVolume ) / sizeof( TYPS_MusicVolume )) );
	}

	if( True == m_bl_MusicSelectBatch ) {
		// 反応音選択種類.
// comment 2016/05/23 SCD2-Sky浜野 [ZeusSBK][CQ528994] 音一括設定パフォーマンス改善.
//		pc_Music->setMusicSelect( APIE_MusicKindInputOk, m_e_MusicSelect );			// 入力確認音.
//		pc_Music->setMusicSelect( APIE_MusicKindInputNg, m_e_MusicSelect );			// 入力無効音.
//		pc_Music->setMusicSelect( APIE_MusicKindCardinalPoint, m_e_MusicSelect );	// 基点音.
//		pc_Music->setMusicSelect( APIE_MusicKindOperation, m_e_MusicSelect );		// 操作終了音.
//		pc_Music->setMusicSelect( APIE_MusicKindCommunication, m_e_MusicSelect );	// 通信終了音.
//		pc_Music->setMusicSelect( APIE_MusicKindPreparation, m_e_MusicSelect );		// 準備完了音.
//		pc_Music->setMusicSelect( APIE_MusicKindWarning1, m_e_MusicSelect );		// 弱注意音<level-1>.
//		pc_Music->setMusicSelect( APIE_MusicKindWarning2, m_e_MusicSelect );		// 弱注意音<level-2>.
//		pc_Music->setMusicSelect( APIE_MusicKindWarning3, m_e_MusicSelect );		// 弱注意音<level-3>.
//		pc_Music->setMusicSelect( APIE_MusicKindError, m_e_MusicSelect );			// 強注意音.
//		pc_Music->setMusicSelect( APIE_MusicKindDelete, m_e_MusicSelect );			// 削除音.
//		pc_Music->setMusicSelect( APIE_MusicKindMotion1, m_e_MusicSelect );			// ウィンドウポップアップ音.
//		pc_Music->setMusicSelect( APIE_MusicKindMotion2, m_e_MusicSelect );			// キーボード操作音.
//		pc_Music->setMusicSelect( APIE_MusicKindPcPrint, m_e_MusicSelect );			// プリンター印刷完了音.
//		pc_Music->setMusicSelect( APIE_MusicKindAuthOk, m_e_MusicSelect );			// 認証成功音.
//		pc_Music->setMusicSelect( APIE_MusicKindAuthNg, m_e_MusicSelect );			// 認証失敗音.
//		pc_Music->setMusicSelect( APIE_MusicKindFaxPrint, m_e_MusicSelect );		// ファクス受信印刷完了音.
		TYPS_MusicSelect st_MusicSelect[] = {
			{ APIE_MusicKindInputOk,		m_e_MusicSelect, False },
			{ APIE_MusicKindInputNg,		m_e_MusicSelect, False },
			{ APIE_MusicKindCardinalPoint,	m_e_MusicSelect, False },
			{ APIE_MusicKindOperation,		m_e_MusicSelect, False },
			{ APIE_MusicKindCommunication,	m_e_MusicSelect, False },
			{ APIE_MusicKindPreparation,	m_e_MusicSelect, False },
			{ APIE_MusicKindWarning1,		m_e_MusicSelect, False },
			{ APIE_MusicKindWarning2,		m_e_MusicSelect, False },
			{ APIE_MusicKindWarning3,		m_e_MusicSelect, False },
			{ APIE_MusicKindError,			m_e_MusicSelect, False },
			{ APIE_MusicKindDelete,			m_e_MusicSelect, False },
			{ APIE_MusicKindMotion1,		m_e_MusicSelect, False },
			{ APIE_MusicKindMotion2,		m_e_MusicSelect, False },
			{ APIE_MusicKindPcPrint,		m_e_MusicSelect, False },
			{ APIE_MusicKindAuthOk,			m_e_MusicSelect, False },
			{ APIE_MusicKindAuthNg,			m_e_MusicSelect, False },
			{ APIE_MusicKindFaxPrint,		m_e_MusicSelect, False },
		};
		pc_Music->setAllMusicSelect( st_MusicSelect, (sizeof( st_MusicSelect ) / sizeof( TYPS_MusicSelect )) );
	}
}

// ====================================================================================================================.
//	機能：画面の音声情報を設定(90NA4200:音量／種類設定画面).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：.
//	戻値    ：.
// ====================================================================================================================.
void PAccessibilityMusicTypeAndVolumeBatchSet::setVoiceGamenInfo()
{
	m_pstVoiceGamenInfo = &VGI_AccessibilityBatchSetSound;
}


// ====================================================================================================================.
//	機能：メニュー有効無効判断処理(90NA4300:個別設定画面).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：ss_option.
//	戻値    ：.
// ====================================================================================================================.
Bool PAccessibilityMusicIndividualMenu::isMenuEnable( uchar uc_MenuNo )
{
	APIC_Music* pc_Music = APIC_Music::newInstance();

	Bool b_Enable = False;
	APIE_MusicCategory e_Category = APIE_MusicCategorySettingMax;

	switch( uc_MenuNo ) {
		case PNLD_AccessibilityMusicOutputSetMenu_OperationCheck:	// 操作確認音出力.
			e_Category = APIE_MusicCategoryOpeConfirm;
			break;
		case PNLD_AccessibilityMusicOutputSetMenu_NormalEnd:		// 正常終了音出力.
			e_Category = APIE_MusicCategoryNormalEnd;
			break;
		case PNLD_AccessibilityMusicOutputSetMenu_Preparation:		// 準備完了音出力.
			e_Category = APIE_MusicCategoryPrepEnd;
			break;
		case PNLD_AccessibilityMusicOutputSetMenu_Warning:			// 注意音出力.
			e_Category = APIE_MusicCategoryWarning;
			break;
		case PNLD_AccessibilityMusicOutputSetMenu_Delete:			// 削除音出力.
			e_Category = APIE_MusicCategoryDelete;
			break;
		case PNLD_AccessibilityMusicOutputSetMenu_Auth:				// 認証音出力.
			e_Category = APIE_MusicCategoryAuth;
			break;
		default:
			break;
	}
	b_Enable = pc_Music->isCategoryEnable( e_Category );		// 分類個別設定の取得.

	return b_Enable;
}

// ====================================================================================================================.
//	機能：画面セットアップ処理(90NA4300:個別設定画面).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：ss_option.
//	戻値    ：.
// ====================================================================================================================.
void PAccessibilityMusicIndividualMenu::setupGamen( short ss_option )
{
	PAccessibilityMusicMenuCom::setupGamen( ss_option );

	setupMessageAreaAccessibility( s_ENLMSGSelectItem );												// メッセージ欄の設定.

	Bool b_Enable = False;
	uchar uc_MenuStartNo = (uchar)(( m_us_PageNum - 1 ) * PNLD_AccessibilityMusicMenuViewCount + 1);			// メニュー開始番号.
	uchar uc_MenuRowNo = 0;																			// メニュー列番号.
	uchar uc_MenuNo = 0;																			// 対象メニュー番号.
	for( uc_MenuRowNo = 0; uc_MenuRowNo < PNLD_AccessibilityMusicMenuViewCount; uc_MenuRowNo++ ) {
		uc_MenuNo = static_cast<uchar>( uc_MenuStartNo + uc_MenuRowNo );
		b_Enable = isMenuEnable( uc_MenuNo );
		if( True == b_Enable ) {
			continue;																				// 有効の時は次へ.
		}
		if( uc_MenuNo - 1 < m_uc_MenuCnt ) {
			Key_AccessibilityMusicMenuBtn[ uc_MenuRowNo ].activate( False );						// 選択キー無効設定.
		}
	}
}

// ====================================================================================================================.
//	機能：10キー押下処理(90NA4300:個別設定画面).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：uc_option.
//	戻値    ：.
// ====================================================================================================================.
void PAccessibilityMusicIndividualMenu::proc10Key( uchar uc_option )
{
	uchar uc_MenuStartNo = (uchar)(( m_us_PageNum - 1 ) * PNLD_AccessibilityMusicMenuViewCount);			// メニュー開始番号.
	uchar uc_MenuNo = 0;																		// 対象メニュー番号.
	uc_MenuNo = static_cast<uchar>( uc_MenuStartNo + uc_option );

	if( True == isMenuEnable( uc_MenuNo ) ) {					// キーが有効.
		PAccessibilityMusicMenuCom::proc10Key( uc_option );
	} else {													// キーが無効.
		setBuzzerType( PCGE_BuzzerType_InputNg );
	}
}

// ====================================================================================================================.
//	機能：画面の音声情報を設定(90NA4300:個別設定画面).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：.
//	戻値    ：.
// ====================================================================================================================.
void PAccessibilityMusicIndividualMenu::setVoiceGamenInfo()
{
	switch( m_us_PageNum ) {
		case 1:
			m_pstVoiceGamenInfo = &VGI_AccessibilityIndivisualSoundSetKeyReceiveDenial_P1;
			break;
		case 2:
			m_pstVoiceGamenInfo = &VGI_AccessibilityIndivisualSoundSetKeyReceiveDenial_P2;
			break;
		default:
			LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
			return;
	}
}


APIE_CustomizeSettingStatus	PAccessibilityMusicListSet::m_e_CustomizeSettingStatus = APIE_CustomizeSettingNG;		// 音声ファイル設定結果.
WDOG_ID						PAccessibilityMusicListSet::wdog_ListID = 0;											// ウォッチドッグＩＤ(ファイルリスト用).
WDOG_ID						PAccessibilityMusicListSet::wdog_NotifyID = 0;											// ウォッチドッグＩＤ(設定確定用).
PAccessibilityMusicListSet::PAccessibilityMusicListSet( PDisplayParts** partsTable, short partsNumber, Bool isNoWarningGamen )
	: PAccessibilityTopMenuCom( partsTable, partsNumber, isNoWarningGamen )
{
	m_ss_SelectMusicFileId   = -1;															// 端末音選択ファイル情報 ファイルID.
	memset( m_st_AllMusicFileInfo, 0x0, sizeof( m_st_AllMusicFileInfo ) );					// 端末音ファイル情報.
	m_ss_MusicListFileNum 		= 0;														// 端末音ファイル数.
	m_ss_TotalListPageNum 		= 1;														// リスト総ページ.
	m_ss_NowListPageNum 		= 1;														// リスト表示ページ.
	m_ss_SelectMusicListNum		= -1;														// 選択リスト№
	m_ss_SelectMusicListPage	= 0;														// 選択リストページ.
	m_uc_ShowListNum			= 0;														// 従来パーツ表示リスト数.
	m_bl_ReqMusicFileListTimer	= False;													// 音声ファイル情報取得要求タイマー.
	m_bl_NotifyMusicFileTimer	= False;													// 音声ファイル設定タイマー.
	m_e_MusicListKind = APIE_MusicKindMax;															// 反応音種類.
	m_st_MusicCustomAdminInfo.e_Kind = APIE_MusicCustomAdmKindNone;									// 音声カスタマイズ管理情報 管理種別
	memset( m_st_MusicCustomAdminInfo.sc_Key , 0x0, sizeof( m_st_MusicCustomAdminInfo.sc_Key ) );	// 音声カスタマイズ管理情報 識別キー
	m_pc_MusicCustomize = NULL;
}
void PAccessibilityMusicListSet::startGamen()
{
	PAccessibilityTopMenuCom::startGamen();													// 親クラスコール.
	m_ss_TotalListPageNum = 1;																// リスト総ページ.
	m_ss_NowListPageNum = 1;																// リスト表示ページ.
	m_uc_ShowListNum	= 0;																// 従来パーツ表示リスト数.
	m_e_MusicListKind = PLBC_MusicKind::getInstance()->getMusicKind();						// 反応音種類.
	m_pc_MusicCustomize = APIC_MusicCustomize::newInstance();								// APIC_MusicCustomizeインスタンスポインタ.
	PLBC_MusicKind::getInstance()->getMusicCustomAdmInfo( &m_st_MusicCustomAdminInfo );		// 音声カスタマイズ管理情報.
	m_pc_MusicCustomize->getIdNo( &m_st_MusicCustomAdminInfo,
	                              m_e_MusicListKind,
	                              &m_ss_SelectMusicFileId );								// 選択リスト№.
	m_bl_ReqMusicFileListTimer = False;														// 音声ファイル情報取得要求タイマー.
	m_bl_NotifyMusicFileTimer = False;														// 音声ファイル設定タイマー.

	uchar uc_MusicIndex;
	for( uc_MusicIndex = 0; uc_MusicIndex < PNLD_MusicListLineMax; uc_MusicIndex++ ) {		// 種類の音が重複するためボタンのブザー設定をOFF.
		Key_MusicList[ uc_MusicIndex ].setBuzzer( False );
	}

	if( PNLE_MTUIMODE_V0 <= PLBC_MultiTouchFunction::getMultiTouchMode() ) {
// comment 2017/01/17 SCD2-Sky浜野 [IT5_3.4][1007985] 端末音リスト表示対応.
//		PnlComMtuiIf_TabletSoundModel::instance()->alloc();
//		PnlComMtuiIf_TabletSoundModel::instance()->clear();
//		PnlComMtuiIf_TabletSoundSelectListView::instance()->selectAll( false );
//		PnlComMtuiIf_TabletSoundSelectListView::instance()->raise();
		PnlComMtuiIf_ModelCom::instance()->clear( &Cmp_AccessibilityMusicList ); 
		PnlComMtuiIf_ListViewCom::instance()->selectAll( &Cmp_AccessibilityMusicList, False ); 
	}
	reqMusicFileList();																		// 音声ファイル情報取得要求.
}
void PAccessibilityMusicListSet::setupGamen( short option )
{
	PAccessibilityTopMenuCom::setupGamen();													// 親クラスコール.
	setupMessageAreaAccessibility( s_MSGMusicList );										// メッセージ欄のセットアップ.
	if( PNLE_MTUIMODE_V0 <= PLBC_MultiTouchFunction::getMultiTouchMode() ) {
		Btx_MusicListPageReal.showEnable( False );											// ページ番号非表示.
		Grp_MusicListPageKey.showEnable( False );											// ↑↓キー非表示.
		Bak_MusicListPage.showEnable( False );												// ページ送り背景非表示.
		Rbt_MusicListKey.showEnable( False );												// リストキー非表示.
// comment 2017/01/17 SCD2-Sky浜野 [IT5_3.4][1007985] 端末音リスト表示対応.
		Cmp_AccessibilityMusicList.showEnable( True );
	} else {
// comment 2017/01/17 SCD2-Sky浜野 [IT5_3.4][1007985] 端末音リスト表示対応.
		Cmp_AccessibilityMusicList.showEnable( False );
		showMusicList( m_ss_NowListPageNum );
	}
	if( True == m_bl_ReqMusicFileListTimer || -1 == m_ss_SelectMusicListNum ) { 				// 端末音ファイル取得中、または選択されていない時はグレーアウト.
		Key_AccessibilityOK.activate( False );												// OKキー.
	} else {																				// 端末音ファイル取得では無く、選択されている時は活性化.
		Key_AccessibilityOK.activate( True );												// OKキー.
	}

}
void PAccessibilityMusicListSet::endGamen()
{
	PAccessibilityTopMenuCom::endGamen();
	m_pc_MusicCustomize->detachObserver( &m_c_MusicCustomizeObserver );						// 音声ファイル情報の取得通知用オブザーバ削除.
	m_pc_MusicCustomize->clearMusicCustomIdSetCallback();									// 音声ファイル設定コールバッククリア.
	if( PNLD_MusicFileStartId <= m_ss_SelectMusicFileId ) {
		m_pc_MusicCustomize->reqMusicPlay( False,
		                                   m_e_MusicListKind,
		                                   PLBC_MusicKind::getInstance()->getMusicSelect(),
		                                   PLBC_MusicKind::getInstance()->getMusicVolume(),
		                                   m_ss_SelectMusicFileId,
		                                   True );												// 音声再生停止.
	} else {
		APIC_Music::newInstance()->reqMusicStop( True );									// 音声再生停止.
	}
	if( PNLE_MTUIMODE_V0 <= PLBC_MultiTouchFunction::getMultiTouchMode() ) {
// comment 2017/01/17 SCD2-Sky浜野 [IT5_3.4][1007985] 端末音リスト表示対応.
//		PnlComMtuiIf_TabletSoundSelectListView::instance()->hide();
//		PnlComMtuiIf_TabletSoundModel::instance()->free();
		PnlComMtuiIf_ListViewCom::instance()->hide( &Cmp_AccessibilityMusicList ); 
	}
}
void PAccessibilityMusicListSet::procButtonOn( uchar option )
{
	short ss_index = 0;
	switch( option ) {
		case BTNID_KEY_AccessibilityMusicListPageUp:										// ↑矢印キー押下.
			if( 1 == m_ss_NowListPageNum ) {												// 現在先頭のページ.
				m_ss_NowListPageNum = m_ss_TotalListPageNum;								// 最終ページを現在のページへ.
			} else {
				m_ss_NowListPageNum--;
			}
			break;
		case BTNID_KEY_AccessibilityMusicListPageDown:										// ↓矢印キー押下?.
			if( m_ss_TotalListPageNum == m_ss_NowListPageNum ) {							// 現在最終のページ？.
				m_ss_NowListPageNum = 1;
			} else {
				m_ss_NowListPageNum++;
			}
			break;
		case BTNID_KEY_AccessibilityMusicList1:												// リストキー１押下.
		case BTNID_KEY_AccessibilityMusicList2:												// リストキー２押下.
		case BTNID_KEY_AccessibilityMusicList3:												// リストキー３押下.
		case BTNID_KEY_AccessibilityMusicList4:												// リストキー４押下.
			m_ss_SelectMusicListNum = ( short )( option - BTNID_KEY_AccessibilityMusicList1 ); // 選択した端末音リストの番号(リストパーツ配列の0から開始).
			m_ss_SelectMusicListPage = m_ss_NowListPageNum;									// 選択した端末音リストページ数.
			ss_index = ( short )( ( ( m_ss_NowListPageNum - 1 ) * PNLD_MusicListLineMax )
			                      + m_ss_SelectMusicListNum );						// ファイルIndex.
			if( 0 == ss_index ) {															// 本体設定と同じ.
				m_ss_SelectMusicFileId = -1;
			} else {
				m_ss_SelectMusicFileId = m_st_AllMusicFileInfo[ ss_index - 1 ].ss_Id;		// ファイルID.
			}
			break;
		default:
			break;
	}
	if( BTNID_KEY_AccessibilityMusicList1 <= option
	    && option <= BTNID_KEY_AccessibilityMusicList4 ) {
		short ss_FileId = m_ss_SelectMusicFileId;											// ファイルID.
		if( PNLD_MusicFileStartId > m_ss_SelectMusicFileId ) {								// 端末音と同じ音を再生する場合.
			ss_FileId = APID_MusicSelectNonCustom;
		}
		APIC_Music::newInstance()->reqMusicTest( m_e_MusicListKind,
		        PLBC_MusicKind::getInstance()->getMusicSelect(),
		        PLBC_MusicKind::getInstance()->getMusicVolume(),
		        ss_FileId );													// 本体音を再生.
	}
	setupGamen();
}
void PAccessibilityMusicListSet::procQtButtonOn( ushort option )
{
	if( 0 < option && option <= APID_MusicFileListNum ) {
		m_ss_SelectMusicFileId = m_st_AllMusicFileInfo[ option - 1 ].ss_Id;					// ファイルID.
	} else {
		m_ss_SelectMusicFileId = -1;
	}
	m_ss_SelectMusicListNum = ( short )( option % PNLD_MusicListLineMax );					// リストIndex.
// comment 2017/01/17 SCD2-Sky浜野 [IT5_3.4][1007985] 端末音リスト表示対応.
//	PnlComMtuiIf_TabletSoundSelectListView::instance()->selectAll( false );
//	PnlComMtuiIf_TabletSoundSelectListView::instance()->select( option, true );				// CQ371161対応 リスト選択.
	PnlComMtuiIf_ListViewCom::instance()->selectAll( &Cmp_AccessibilityMusicList, False ); 
	PnlComMtuiIf_ListViewCom::instance()->select( &Cmp_AccessibilityMusicList, option, True ); 
	short ss_FileId = m_ss_SelectMusicFileId;												// ファイルID.
	if( PNLD_MusicFileStartId > m_ss_SelectMusicFileId ) {									// 端末音と同じ音を再生する場合.
		ss_FileId = APID_MusicSelectNonCustom;
	}
	APIC_Music::newInstance()->reqMusicTest( m_e_MusicListKind,
	        PLBC_MusicKind::getInstance()->getMusicSelect(),
	        PLBC_MusicKind::getInstance()->getMusicVolume(),
	        ss_FileId );														// 本体音を再生.
	setupGamen();
}
void PAccessibilityMusicListSet::procOk( uchar option )
{
	if( PNLD_MusicFileStartId <= m_ss_SelectMusicFileId ) {
		reqNotifyMusicFile();														// 音声ファイル設定.
	} else {
		// 未指定データ（本体と同じ設定）の設定.
		m_pc_MusicCustomize->setIdNo( &m_st_MusicCustomAdminInfo,
		                              m_e_MusicListKind,
		                              m_ss_SelectMusicFileId );						// 選択リスト№.
		m_pc_MusicCustomize->setCustomEnable( m_e_MusicListKind, False );			// カスタム無効.
		// 設定処理をしない場合は直接遷移する.
		if( NULL != PLBC_MusicKind::getInstance()->getEndGamen() ) {
			nextGamen = PLBC_MusicKind::getInstance()->getEndGamen();				// 画面遷移先指定.
		} else {
			PAccessibilityTopMenuCom::procOk( option );
		}																			// 画面遷移する.
	}
}
void PAccessibilityMusicListSet::procCallBack( TYPE_PNL_CallBack option )
{
	switch( option ) {
		case TYPE_PNL_CallBack_NotifyMusicFile:												// 音声ファイル設定応答.
			m_bl_NotifyMusicFileTimer = False;												// 音声ファイル設定タイマー.
			SYSF_wdDelete( wdog_NotifyID );													// ウォッチドックタイマーの削除.
			if( APIE_CustomizeSettingOK == m_e_CustomizeSettingStatus ) {
				m_pc_MusicCustomize->setCustomEnable( m_e_MusicListKind, True );			// カスタム有効.
				if( NULL != PLBC_MusicKind::getInstance()->getEndGamen() ) {
					nextGamen = PLBC_MusicKind::getInstance()->getEndGamen();				// 画面遷移先指定.
				} else {
					PAccessibilityTopMenuCom::procOk( 0 );
				}																			// 画面遷移する.
			} else {
				nextGamen = &gp_AccessibilityMusicListSetErr;								// 警告画面へ遷移.
			}
			break;
		case TYPE_PNL_CallBack_ReqMusicFileTimeout:											// 音声ファイル情報取得要求応答.
			reqMusicFileListTimeout();
			m_ss_SelectMusicListPage = 1;													// 選択した端末音リストページ数.
			m_ss_SelectMusicFileId = -1;
			m_ss_SelectMusicListNum = 0;
			break;
		case TYPE_PNL_CallBack_NotifyMusicFileTimeout:										// 音声ファイル設定応答.
			reqNotifyMusicFileTimeout();
			break;
		default:
			break;
	}
	PAccessibilityTopMenuCom::procCallBack( option );										// 親クラスコール.
}
// 概要	: 90NA4190 端末音設定 オブザーバイベントハンドラ.
void PAccessibilityMusicListSet::procObserver( TYPE_PNL_Observer option )
{
	switch( option ) {
		case TYPE_PNL_Observer_ReqMusicFileList:											// 音声ファイル情報取得要求応答.
		{
			m_bl_ReqMusicFileListTimer = False;												// 音声ファイル情報取得要求タイマーOFF.
			SYSF_wdDelete( wdog_ListID );													// ウォッチドックタイマーの削除.
			m_pc_MusicCustomize->detachObserver( &m_c_MusicCustomizeObserver );				// 音声ファイル情報の取得通知用オブザーバ削除.
			m_pc_MusicCustomize->getMusicFileList( m_st_AllMusicFileInfo );					// 音声ファイル情報取得.
			short ss_Index = 0;
			for( ss_Index = 0; ss_Index < APID_MusicFileListNum; ss_Index++ ) {			// カスタム音ファイル数チェック.
				if( m_st_AllMusicFileInfo[ ss_Index ].sc_Name[ 0 ] == '\0' ) {
					break;
				}
			}
			m_ss_MusicListFileNum = ( short )( ss_Index + 1 );								// 音声ファイル情報設定数.
			if( PNLE_MTUIMODE_V0 <= PLBC_MultiTouchFunction::getMultiTouchMode() ) {
				// TextID(本体と同じ)から文字列を取得(UTF-8)して設定.
				char sc_Name[APID_MusicFileNameLength];
				memset( sc_Name, 0x0, sizeof( sc_Name ) );
				PnlComConvertTextIDToString( s_ITMMusicListMFPSettingUse, sc_Name, sizeof( sc_Name ) );
// comment 2017/01/17 SCD2-Sky浜野 [IT5_3.4][1007985] 端末音リスト表示対応.
//				PnlComMtuiIf_TabletSoundModel::instance()->addDataToSubmit( 0, ( uchar* )sc_Name );
//				// カスタム音リスト表示設定.
//				for( ss_Index = 0; ss_Index < APID_MusicFileListNum; ss_Index++ ) {
//					if( m_st_AllMusicFileInfo[ss_Index].sc_Name[0] == '\0' ) {
//						break;
//					}
//					PnlComMtuiIf_TabletSoundModel::instance()->addDataToSubmit( ss_Index + 1, ( uchar* )m_st_AllMusicFileInfo[ss_Index].sc_Name );
//				}
//				PnlComMtuiIf_TabletSoundModel::instance()->submitDatas();
//				PnlComMtuiIf_TabletSoundSelectListView::instance()->show();
				std::list<GFWS_TabletSoundListInfo*>	c_TabletSoundList;
				
				//GFWS_TabletSoundListInfo* pst_SoundListInfo = new GFWS_TabletSoundListInfo( 0, (uchar*)( "000" ), (uchar*)( sc_Name ), GFWD_TRUE );
				uchar uc_SrcNo[ 8 + 1 ] = {0}; snprintf( reinterpret_cast<char*>( uc_SrcNo ), sizeof( uc_SrcNo ), "000");
				GFWS_TabletSoundListInfo* pst_SoundListInfo = new GFWS_TabletSoundListInfo( 0, &uc_SrcNo[0], reinterpret_cast<uchar*>( sc_Name ), GFWD_TRUE );
				c_TabletSoundList.push_back( pst_SoundListInfo );
				
				for( ss_Index = 0; ss_Index < APID_MusicFileListNum; ss_Index++ ) {
					if( m_st_AllMusicFileInfo[ss_Index].sc_Name[0] == '\0' ) {
						break;
					}
					char sc_No[ 8 + 1 ];
					memset( sc_No, '\0', sizeof( sc_No ) );
					snprintf( sc_No, sizeof( sc_No )-1, "%03d", ss_Index+1 );
					GFWS_TabletSoundListInfo* pst_SoundListInfo = new GFWS_TabletSoundListInfo( ss_Index+1, reinterpret_cast<uchar*>( sc_No ), reinterpret_cast<uchar*>( m_st_AllMusicFileInfo[ss_Index].sc_Name ), GFWD_TRUE );
					c_TabletSoundList.push_back( pst_SoundListInfo );
				}

				GFWC_Cmd_SetTabletSoundList	c_Cmd( &c_TabletSoundList );
				GFWC_Com_Component* pc_GUIComponent = static_cast<GFWC_Com_Component*>( Cmp_AccessibilityMusicList.getGUIParts() );
				if( 0 != pc_GUIComponent )
				{
					// 送信履歴宛先リスト活性化状態設定コマンドを実行.
					pc_GUIComponent->doProcessing( c_Cmd );
					// リストの先頭から終端までのデータを取得し破棄する.
					for( std::list<GFWS_TabletSoundListInfo*>::iterator itr = c_TabletSoundList.begin(); itr != c_TabletSoundList.end(); ++itr )
					{
						// データ破棄
						delete *itr;
					}
				}
				PnlComMtuiIf_ListViewCom::instance()->show( &Cmp_AccessibilityMusicList );
			}
			m_ss_TotalListPageNum = ( short )( ( m_ss_MusicListFileNum + PNLD_MusicListLineMax - 1 ) / PNLD_MusicListLineMax );	// 音声ファイル数からリストページ数を算出.
			setSelectColumn();
			break;
		}
		default:
			break;
	}
	PAccessibilityTopMenuCom::procObserver( option );										// 親クラスコール.
}
uchar PAccessibilityMusicListSet::getShowListNum()
{
	if( m_bl_ReqMusicFileListTimer == True ) {													// タイマーカウント中は非表示.
		m_uc_ShowListNum = 0;
	} else {
		if( 0 == m_ss_MusicListFileNum ) {														// ファイルリストが無い場合は、1(「本体設定と同じ」のみ).
			m_uc_ShowListNum = 1;
		} else {
			if( ( m_ss_NowListPageNum * PNLD_MusicListLineMax ) <= m_ss_MusicListFileNum ) {
				m_uc_ShowListNum = PNLD_MusicListLineMax;										// 表示ページの最大がリスト数以下の場合は、MAX(4).
			} else {
				m_uc_ShowListNum = ( uchar )( m_ss_MusicListFileNum % PNLD_MusicListLineMax );	// 表示ページでリスト表示数に満たない場合は、有効数のみ.
			}
		}
	}
	return m_uc_ShowListNum;
}
void PAccessibilityMusicListSet::showMusicList( short ss_ListPageNum )
{
	uchar uc_Index = 0;
	if( False == m_bl_ReqMusicFileListTimer ) {
		for( uc_Index = 0; uc_Index < getShowListNum(); uc_Index++ ) {
			Key_MusicList[ uc_Index ].set( &Nfd_MusicListNo[ uc_Index ],
			                               ( float )( ( PNLD_MusicListLineMax * ( ss_ListPageNum - 1 ) ) + uc_Index + 1 ),
			                               True, 3 );																	// No.表示.
			if( 1 == ss_ListPageNum && 0 == uc_Index ) {																	// 1ページ目の№1のリストの場合.
				Key_MusicList[ uc_Index ].changeText( &Tfd_MusicListTx[ uc_Index ], s_ITMMusicListMFPSettingUse );		// 「本体設定と同じ」を表示する.
			} else {
				Key_MusicList[ uc_Index ].changeExternalText( &Tfd_MusicListTx[ uc_Index ],
				        ( uchar* )m_st_AllMusicFileInfo[( PNLD_MusicListLineMax * ( ss_ListPageNum - 1 ) ) + uc_Index - 1 ].sc_Name,
				        sizeof( m_st_AllMusicFileInfo[( PNLD_MusicListLineMax * ( ss_ListPageNum - 1 ) ) + uc_Index - 1 ].sc_Name ),
				        getCharSetForPanel() );														// 名称表示.
			}
		}
	}
	Bool bl_ShowList[ PNLD_MusicListLineMax ] = { False };									// リスト表示状態.
	for( uc_Index = 0; uc_Index < getShowListNum(); uc_Index++ ) {
		bl_ShowList[ uc_Index ] = True;
	}
	for( uc_Index = 0; uc_Index < PNLD_MusicListLineMax; uc_Index++ ) {
		Key_MusicList[ uc_Index ].showEnable( bl_ShowList[ uc_Index ] );					// リスト表示状態設定.
	}
	Btx_MusicListPageReal.set( &Num_MusicListCurrentPage, m_ss_NowListPageNum, True );		// 現ページ設定値.
	Btx_MusicListPageReal.set( &Num_MusicListTotalPage, m_ss_TotalListPageNum, True );		// 総ページ設定値.
	if( m_ss_MusicListFileNum > PNLD_MusicListLineMax ) {											// 端末音ファイル数が4件以上.
		Grp_MusicListPageKey.showEnable( True );											// 矢印キー表示.
	} else {																				// 端末音ファイル数が4件未満.
		Grp_MusicListPageKey.showEnable( False );											// 矢印キー非表示.
	}
	if( ss_ListPageNum == m_ss_SelectMusicListPage ) {
		Rbt_MusicListKey.select( m_ss_SelectMusicListNum );									// 選択ページの場合、リストを選択状態に.
	} else {
		Rbt_MusicListKey.select( -1 );														// 選択ページではない場合、全てのリストを非選択状態に.
	}
}
void PAccessibilityMusicListSet::setSelectColumn()
{
	short ss_Index = 0;
	if( PNLD_MusicFileStartId <= m_ss_SelectMusicFileId ) {			// ID不正(「本体設定と同じ」指定).
		short ss_FileIndex = 0;
		ss_FileIndex = checkMusicList();							// 端末音ファイルから選択IDと一致する物を確認.
		if( PNLD_FileUnmatch == ss_FileIndex ) {					// 選択する物がない、またはリストが更新されている場合.
			if( PNLE_MTUIMODE_V0 <= PLBC_MultiTouchFunction::getMultiTouchMode() ) {
// comment 2017/01/17 SCD2-Sky浜野 [IT5_3.4][1007985] 端末音リスト表示対応.
//				PnlComMtuiIf_TabletSoundSelectListView::instance()->selectAll( false );
				PnlComMtuiIf_ListViewCom::instance()->selectAll( &Cmp_AccessibilityMusicList, False ); 
			} else {
				m_ss_SelectMusicListNum = -1;
			}
			return;
		}
		ss_Index = ( short )( ss_FileIndex + 1 );						// 本体設定分のずらし.
	}
	if( PNLE_MTUIMODE_V0 <= PLBC_MultiTouchFunction::getMultiTouchMode() ) {
		procQtButtonOn( ss_Index );
	} else {
		m_ss_SelectMusicListNum = ss_Index % PNLD_MusicListLineMax;
		m_ss_SelectMusicListPage = ( short )( ( ss_Index / PNLD_MusicListLineMax ) + 1 );	// 選択した端末音リストページ数.
	}
	setupGamen();																			// リストキー非表示.
}
short PAccessibilityMusicListSet::checkMusicList()
{
	short ss_Idx = 0;
	short ss_FileIndex = PNLD_FileUnmatch;
	for( ss_Idx = 0; ss_Idx < APID_MusicFileListNum; ss_Idx++ ) {							// カスタム音リスト表示設定.
		if( m_st_AllMusicFileInfo[ ss_Idx ].sc_Name[ 0 ] == '\0'
		    || m_ss_SelectMusicFileId == m_st_AllMusicFileInfo[ ss_Idx ].ss_Id ) {
			break;
		}
	}
	if( ss_Idx < APID_MusicFileListNum ) {													// 見つかった場合.
		if( m_ss_SelectMusicFileId == m_st_AllMusicFileInfo[ ss_Idx ].ss_Id				// 最後に確定されたIDが存在する.
		    && True == m_st_AllMusicFileInfo[ ss_Idx ].b_RegistFlg ) {
			ss_FileIndex = ss_Idx;
		}
	}
	return ss_FileIndex;
}
void PAccessibilityMusicListSet::reqMusicFileList()
{
	wdog_ListID = SYSF_wdCreate();											// ウォッチドッグID.
	m_pc_MusicCustomize->attachObserver( &m_c_MusicCustomizeObserver );		// 音声ファイル情報の取得通知用オブザーバ登録.
	SYSF_wdStart( wdog_ListID, PNLD_ReqMusicFileTimeout,
	              ( FUNCPTR )ansReqMusicFileTimeout, 0 );						// タイマー起動.
	m_bl_ReqMusicFileListTimer = True;										// 音声ファイル情報取得要求タイマー.
	m_pc_MusicCustomize->reqMusicFileList();								// 音声ファイル情報取得要求.
}
void PAccessibilityMusicListSet::reqNotifyMusicFile()
{
	wdog_NotifyID = SYSF_wdCreate();														// ウォッチドッグID.
	SYSF_wdStart( wdog_NotifyID, PNLD_NotifyMusicFileTimeout,
	              ( FUNCPTR )ansNotifyMusicFileTimeout, 0 );									// タイマー起動.
	m_bl_NotifyMusicFileTimer = True;														// 音声ファイル設定タイマー.
	m_pc_MusicCustomize->reqMusicCustomIdSet( m_e_MusicListKind,
	        m_ss_SelectMusicFileId, ansNotifyMusicFile );					// 音声ファイル設定.
}
void PAccessibilityMusicListSet::reqMusicFileListTimeout()
{
	if( True == m_bl_ReqMusicFileListTimer ) {
		m_pc_MusicCustomize->detachObserver( &m_c_MusicCustomizeObserver );	// 音声ファイル情報の取得通知用オブザーバ削除.
		m_bl_ReqMusicFileListTimer = False;									// 音声ファイル情報取得要求タイマー.
	}
}
void PAccessibilityMusicListSet::reqNotifyMusicFileTimeout()
{
	if( True == m_bl_NotifyMusicFileTimer ) {
		m_pc_MusicCustomize->clearMusicCustomIdSetCallback();								// 音声ファイル設定コールバッククリア.
		m_bl_NotifyMusicFileTimer = False;													// 音声ファイル設定タイマー.
		nextGamen = &gp_AccessibilityMusicListSetErr;										// 警告画面へ遷移.
	}
}
void PAccessibilityMusicListSet::ansNotifyMusicFile( const APIE_CustomizeSettingStatus e_CustomizeSettingStatus )
{
	m_e_CustomizeSettingStatus = e_CustomizeSettingStatus;
	CallBackIndi( TYPE_PNL_CallBack_NotifyMusicFile );										// 音声ファイル設定応答.
}
void PAccessibilityMusicListSet::ansReqMusicFileTimeout()
{
	CallBackIndi( TYPE_PNL_CallBack_ReqMusicFileTimeout );									// 音声ファイル情報取得要求応答.
}
void PAccessibilityMusicListSet::ansNotifyMusicFileTimeout()
{
	CallBackIndi( TYPE_PNL_CallBack_NotifyMusicFileTimeout );								// 音声ファイル設定応答.
}
// 概要 : 10J80000 端末音設定エラー 表示画面のセットアップ.
void PAccessibilityMusicListSetErr::setupGamen( short option )
{
	PAccessibilityTopMenuCom::setupGamen( option );											// 親クラスコール.
	Txt_EnlSystemErrorMessageArea1.changeText( s_MSGRemoteCtrlTermReqErr );					// 拡大警告メッセージ共通.
}
// 概要 : 10J80000 端末音設定エラー OKキー.
void PAccessibilityMusicListSetErr::procOk( uchar option )
{
	if( NULL == PLBC_MusicKind::getInstance()->getEndGamen() ) {							// NULLの場合は無いはずだが、確認はしておく.
		PAccessibilityTopMenuCom::procOk( option );											// ユーザー補助Topに戻す.
	} else {
		nextGamen = PLBC_MusicKind::getInstance()->getEndGamen();							// 画面遷移先指定.
	}
}
//================================================================================================
//概要    :画面反転設定画面(93NA2000) : ＯＫキー押下.
//================================================================================================
void PAccessibilityDispColorReverse::procOk( uchar option )
{
	PAccessibilityTopMenuCom::procOk( option );
	APIC_OperationSetting* pc_OperationSetting = APIC_OperationSetting::newInstance();
	if( NULL == pc_OperationSetting ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	pc_OperationSetting->setDispColorReverse( m_bl_DispColorReverse );
}
//================================================================================================
//概要    :画面反転設定画面(93NA2000) : ボタン（押された）.
//================================================================================================
void PAccessibilityDispColorReverse::procButtonOn( uchar option )
{
	switch( option ) {
		case PNLD_AccessibilityDispColorReverseON: m_bl_DispColorReverse = True; break;			// 表示する.
		case PNLD_AccessibilityDispColorReverseOFF: m_bl_DispColorReverse = False; break;		// 表示しない.
		default: break;
	}
	setupGamen( option );
}
//================================================================================================
//概要    :画面反転設定画面(93NA2000) : 画面開始処理.
//================================================================================================
void PAccessibilityDispColorReverse::startGamen()
{
	PAccessibilityTopMenuCom::startGamen();
	APIC_OperationSetting* pc_OperationSetting = APIC_OperationSetting::newInstance();
	if( NULL == pc_OperationSetting ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_UNKNOWN, "Assert Failed!" );
		return;
	}
	m_bl_DispColorReverse = pc_OperationSetting->getDispColorReverse();
}
//================================================================================================
//概要    :画面反転設定画面(93NA2000) : 表示部品のセットアップ.
//================================================================================================
void PAccessibilityDispColorReverse::setupGamen( short option )
{
	Rtb_AccessibilityDispColorReverse.select( False == m_bl_DispColorReverse? 1 : 0 );

	// メッセージ欄のセットアップ.
	setupMessageAreaAccessibility( s_MSGUniversalDispColorReverseAll );

	PAccessibilityTopMenuCom::setupGamen( option );
}
// comment 2015/12/01 OPS1-Sky稲葉 [ZeusS_ZX0][制御No82] 欧州アクセシビリティ対応 「K4NA300000_2度打ち防止優先設定」画面を追加.
//================================================================================================
//概要    :K4NA300000_2度打ち防止優先設定 : コンストラクタ.
//================================================================================================
PAccessibilityTwicePressPreventionPrioritySetting::PAccessibilityTwicePressPreventionPrioritySetting(
    PDisplayParts** partsTable,
    short partsNumber,
    Bool isNoWarningGamen )
	: PAccessibilityTopMenuCom( partsTable, partsNumber, isNoWarningGamen )
{
	m_e_PriorityTapSetting = TYPE_PriorityTapSetting_DoubleTap;
}
//================================================================================================
//概要    :K4NA300000_2度打ち防止優先設定 : ＯＫキー押下.
//================================================================================================
void PAccessibilityTwicePressPreventionPrioritySetting::procOk( uchar option )
{
	PAccessibilityTopMenuCom::procOk( option );

	// NVDから優先設定、ダブルタップ間隔、2度打ち防止間隔を取得する.
	Bool bl_TwiceTap = False;
	uchar uc_TwiceTapInterval = 0;
	short ss_TapLevel =0;
	APIC_OperationSetting* pc_OperationSetting = PnlComGetOperationSetting();
	if ( NULL != pc_OperationSetting ) {
		pc_OperationSetting->setPriorityTapSetting( m_e_PriorityTapSetting );		
		bl_TwiceTap = ( TYPE_PriorityTapSetting_DoubleTap == m_e_PriorityTapSetting ) ? False : True;
		uc_TwiceTapInterval = pc_OperationSetting->getTwiceStrikeBlockInterval();
		ss_TapLevel = pc_OperationSetting->getDoubleTapIntervalMode();
	}

	// Qtに2度打ち防止設定をNVD値で通知する.
	PNLC_QWSConnector* pc_QWSConnector = PNLC_QWSConnector::newInstance();
	if( NULL != pc_QWSConnector ) {
		pc_QWSConnector->setTwiceTapInterval( bl_TwiceTap, uc_TwiceTapInterval * PNLD_ExchangeTwiceIntervalNVDToQT );
	}
	
	// Qtconnecterにダブルタップ間隔を通知する.
	if ( NULL != pc_OperationSetting ) {
		PnlComQtSetDoubleTapInterval( bl_TwiceTap ? 0 : getIntervalTime( ss_TapLevel ) );
	}
}
//================================================================================================
//概要    :K4NA300000_2度打ち防止優先設定 : ボタン（押された）.
//================================================================================================
void PAccessibilityTwicePressPreventionPrioritySetting::procGoToPage( uchar option )
{
	switch( option ) {
		case PNLD_AccessibilityDoubleTapPriority:				// ダブルタップ優先キー.
			m_e_PriorityTapSetting = TYPE_PriorityTapSetting_DoubleTap; 
			nextGamen = &gp_AccessibilityDoubleTapIntervalSetting;
			break;
		case PNLD_AccessibilityTwicePressPreventionPriority:	// 2度打ち防止優先キー.
			m_e_PriorityTapSetting = TYPE_PriorityTapSetting_TwiceStrikeBlock;
			nextGamen = &gp_AccessibilityTwicePressPreventionTimeSpan;
			break;
		default: break;
	}
}
//================================================================================================
//概要    :K4NA300000_2度打ち防止優先設定 : 画面開始処理.
//================================================================================================
void PAccessibilityTwicePressPreventionPrioritySetting::startGamen()
{
	// 下層からの遷移以外.
	if( True != bl_isUnder ) {
		APIC_OperationSetting* pc_OperationSetting = PnlComGetOperationSetting();
		if ( NULL != pc_OperationSetting ) {
			m_e_PriorityTapSetting = pc_OperationSetting->getPriorityTapSetting();
		}
	}
	PAccessibilityTopMenuCom::startGamen();

	// アクシビリティ画面表示フラグをON.
	onlyPanel.b_IsAccessibilityGamen = True;
}
//================================================================================================
//概要    :K4NA300000_2度打ち防止優先設定 : 表示部品のセットアップ.
//================================================================================================
void PAccessibilityTwicePressPreventionPrioritySetting::setupGamen( short option )
{
	PAccessibilityTopMenuCom::setupGamen( option );
	// メッセージ欄のセットアップ.
// comment 2018/08/10 SCD2 T.Hori [Eagle/SparrowMFP][UD評価課題対応 No.41] ユニバーサル設定メッセージ追加.
	setupMessageAreaAccessibility( s_pENLMSGTwicePressPreventionSelectClassicOnly_W696H056A );

	Rtb_AccessibilityTwicePressPreventionPrioritySetting.select( TYPE_PriorityTapSetting_DoubleTap == m_e_PriorityTapSetting ? 0 : 1 );
}

//================================================================================================
//概要    :K4NA310000_2度打ち防止優先 : コンストラクタ.
//================================================================================================
PAccessibilityTwicePressPreventionTimeSpan::PAccessibilityTwicePressPreventionTimeSpan(
    PDisplayParts** partsTable,
    short partsNumber,
    Bool isNoWarningGamen )
	: PAccessibilityTopMenuCom( partsTable, partsNumber, isNoWarningGamen )
{
	m_uc_Interval = 0x00;			// 間隔時間.
}
//================================================================================================
//概要    :K4NA310000_2度打ち防止優先 : ＯＫ.
//================================================================================================
void PAccessibilityTwicePressPreventionTimeSpan::procOk( uchar option )
{
	PAccessibilityTopMenuCom::procOk( option );
	nextGamen = &gp_AccessibilityTwicePressPreventionPrioritySetting;

	// NVDから優先設定、2度打ち防止間隔を取得する.
	Bool bl_TwiceTap = False;
	APIC_OperationSetting* pc_OperationSetting = PnlComGetOperationSetting();
	if ( NULL != pc_OperationSetting ) {
		 pc_OperationSetting->setTwiceStrikeBlockInterval( m_uc_Interval );
		 bl_TwiceTap = ( TYPE_PriorityTapSetting_DoubleTap == pc_OperationSetting->getPriorityTapSetting() ) ? False : True;
	}

	// Qtに2度打ち防止設定をNVD値で通知する.
	PNLC_QWSConnector* pc_QWSConnector = PNLC_QWSConnector::newInstance();
	if( NULL != pc_QWSConnector ) {
		pc_QWSConnector->setTwiceTapInterval( bl_TwiceTap, m_uc_Interval * PNLD_ExchangeTwiceIntervalNVDToQT );
	}
}
//================================================================================================
//概要    :K4NA310000_2度打ち防止優先 : ボタン(押された).
//================================================================================================
void PAccessibilityTwicePressPreventionTimeSpan::procButtonOn( uchar option )
{
	switch( option ) {
		case PNLD_AccessibilityTimeSpanMinus:
			if( m_uc_Interval > PNLD_TwiceTapMinValue ) {
				m_uc_Interval = static_cast<uchar>( m_uc_Interval - PNLD_TwiceTapStepValue );
			} else {
				setBuzzerType( PCGE_BuzzerType_InputNg );
			}
			break;
		case PNLD_AccessibilityTimeSpanPlus:
			if( m_uc_Interval < PNLD_TwiceTapMaxValue ) {
				m_uc_Interval = static_cast<uchar>( m_uc_Interval + PNLD_TwiceTapStepValue );
			} else {
				setBuzzerType( PCGE_BuzzerType_InputNg );
			}
			break;

		default:	break;
	}
	setupGamen( option );
}
//================================================================================================
//概要    :K4NA310000_2度打ち防止優先 : （押し続け、速く変化）キーリピート.
//================================================================================================
void PAccessibilityTwicePressPreventionTimeSpan::procButtonStillOnFast( uchar option ) {
	switch( option ) {
		case PNLD_AccessibilityTimeSpanMinus:	// 小さく.
		case PNLD_AccessibilityTimeSpanPlus:	// 大きく.
			procButtonOn( option );
			break;
		default: break;
	}
}
//================================================================================================
//概要    :K4NA310000_2度打ち防止優先 : 開始画面.
//================================================================================================
void PAccessibilityTwicePressPreventionTimeSpan::startGamen()
{
	PAccessibilityTopMenuCom::startGamen();	// 親クラスの処理を呼ぶ.

		APIC_OperationSetting* pc_OperationSetting = PnlComGetOperationSetting();
		if ( NULL != pc_OperationSetting ) {
			m_uc_Interval = pc_OperationSetting->getTwiceStrikeBlockInterval();
		}

	// アクシビリティ画面表示フラグをON.
	onlyPanel.b_IsAccessibilityGamen = True;
}
//================================================================================================
//概要    :K4NA310000_2度打ち防止優先 : 表示部品のセットアップ.
//================================================================================================
void PAccessibilityTwicePressPreventionTimeSpan::setupGamen( short option )
{
	float sf_Interval = ( float )( m_uc_Interval * PNLD_ExchangeTwiceIntervalUI );
	Btx_AccessibilityTimeSpanSec = sf_Interval;		// 間隔時間.

	PAccessibilityTopMenuCom::setupGamen( option );
	// メッセージ欄のセットアップ.
	setupMessageAreaAccessibility( s_pENLMSGTwicePressPrevention_W368H050A );
}

//comment 2018/4/9 SCD2-Sky肥後 [Eagle][制御構成変更No.92-1]振動パネル対応.
// ====================================================================================================================.
//	機能：P0N0110000_バイブ設定 : コンストラクタ.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		pc_partsTable		画面パーツテーブル.
//		ss_partsNumber		画面パーツ数.
//		b_isNoWarningGamen	警告表示禁止指定.
///	\par	履歴:.\n
// ====================================================================================================================.
PAccessibilityVibrationSetting::PAccessibilityVibrationSetting( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen )
	: PAccessibilityTopMenuCom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen )
	, m_e_VibrationAmount( TYPE_VibrationAmount_Medium )
	, m_e_VibrationPowerKey( TYPE_VibrationResponse_On )
	, m_e_VibrationPanelKey( TYPE_VibrationResponse_On )
	, m_e_VibrationLongTapKey( TYPE_VibrationResponse_On )
{
}

// ====================================================================================================================.
//	機能：P0N0110000_バイブ設定 : 画面開始処理.
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationSetting::startGamen()
{
	
	PAccessibilityTopMenuCom::startGamen();
	
	APIC_OperationSetting* pc_Vibration =  PnlComGetOperationSetting();
	if ( NULL == pc_Vibration ) {
		return;
	}
	m_e_VibrationAmount = pc_Vibration->getVibrationAmount();				// バイブの強さを取得する.
	m_e_VibrationPowerKey = pc_Vibration->getVibrationLongTapPowerKey();	// 電源キーのバイブ情報を取得する.
	m_e_VibrationPanelKey = pc_Vibration->getVibrationResponse();			// タップ時の振動が有効かどうかを取得する.
	m_e_VibrationLongTapKey = pc_Vibration->getVibrationLongTapTouchKey();	// ロングタップ時の振動が有効かどうかを取得する.
}

// ====================================================================================================================.
//	機能：P0N0110000_バイブ設定 : 画面セットアップ処理.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//	ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationSetting::setupGamen( short ss_option )
{
	PAccessibilityTopMenuCom::setupGamen( ss_option );
	setupMessageAreaAccessibility( s_MSGAcceChangeSettingValue );
	switch( m_e_VibrationAmount ) {
		case TYPE_VibrationAmount_Small:	// 弱.
			Bak_EnlAccessibilityVibrationAmountStatus.changeText( s_pENLITMVibrationLow_W224H032H );
			break;
		case TYPE_VibrationAmount_Medium:	// 中.
			Bak_EnlAccessibilityVibrationAmountStatus.changeText( s_pENLITMVibrationMiddle_W224H032H );
			break;
		case TYPE_VibrationAmount_Large:	// 強.
			Bak_EnlAccessibilityVibrationAmountStatus.changeText( s_pENLITMVibrationHigh_W224H032H );		
			break;
		default: break;
	}
	if ( ( TYPE_VibrationResponse_Off == m_e_VibrationPowerKey )
	&& ( TYPE_VibrationResponse_Off == m_e_VibrationPanelKey )
	&& ( TYPE_VibrationResponse_Off == m_e_VibrationLongTapKey ) ) {
	  Key_AccessibilityVibrationAmountKey.activate( False );
	  Bak_EnlAccessibilityVibrationAmountStatus.activate( False );
	}
}

// ====================================================================================================================.
//	機能：P0N0110000_バイブ設定 : 画面移動処理.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//	ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationSetting::procGoToPage( uchar option )
{
	switch( option ) {
		case PNLD_AccessibilityVibrationPanelKeySetting:				// タッチパネルのキー.
			nextGamen = &gp_AccessibilityVibrationPanelKeySetting;
			break;
		case PNLD_AccessibilityVibrationPowerKeySetting:				// 電源キー.
			nextGamen = &gp_AccessibilityVibrationPowerKeySetting;
			break;
		case PNLD_AccessibilityVibrationAmountSetting:				// バイブの強さキー.
			nextGamen = &gp_AccessibilityVibrationAmountSetting;
			break;
		default: break;
	}
}

// ====================================================================================================================.
//	機能：P0N0110000_バイブ設定 : 画面の音声情報を設定.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：.
//	戻値    ：.
// ====================================================================================================================.
void PAccessibilityVibrationSetting::setVoiceGamenInfo()
{
	m_pstVoiceGamenInfo = &VGI_AccessibilityVibrationSetting;
}

// ====================================================================================================================.
//	機能：P0N0111000_タッチパネルのキー設定 : コンストラクタ.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		pc_partsTable		画面パーツテーブル.
//		ss_partsNumber		画面パーツ数.
//		b_isNoWarningGamen	警告表示禁止指定.
///	\par	履歴:.\n
// ====================================================================================================================.
PAccessibilityVibrationPanelKeySetting::PAccessibilityVibrationPanelKeySetting( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen )
	: PAccessibilityTopMenuCom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen )
{
}

// ====================================================================================================================.
//	機能：P0N0111000_タッチパネルのキー設定：画面セットアップ処理.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//	ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationPanelKeySetting::setupGamen( short ss_option )
{
	PAccessibilityTopMenuCom::setupGamen( ss_option );
	setupMessageAreaAccessibility( s_MSGAcceChangeSettingValue );
}
// ====================================================================================================================.
//	機能：P0N0111000_タッチパネルのキー設定：OK.
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationPanelKeySetting::procOk( uchar option )
{
	nextGamen = &gp_AccessibilityVibrationSetting;
}

// ====================================================================================================================.
//	機能：P0N0111000_タッチパネルのキー設定：画面移動処理.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//	ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationPanelKeySetting::procGoToPage( uchar option )
{
	switch( option ) {
		case PNLD_AccessibilityVibrationSingleTapSetting:				// タップキー.
			nextGamen = &gp_AccessibilityVibrationSingleTap;
			break;
		case PNLD_AccessibilityVibrationLongTapSetting:				// ロングタップキー.
			nextGamen = &gp_AccessibilityVibrationLongTap;
			break;
		default: break;
	}
}

// ====================================================================================================================.
//	機能：P0N0111000_タッチパネルのキー設定：画面の音声情報を設定.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：.
//	戻値    ：.
// ====================================================================================================================.
void PAccessibilityVibrationPanelKeySetting::setVoiceGamenInfo()
{
	m_pstVoiceGamenInfo = &VGI_AccessibilityVibrationPanelKeySetting;
}

// ====================================================================================================================.
//	機能：P0N0112000_振動量設定：コンストラクタ.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		pc_partsTable		画面パーツテーブル.
//		ss_partsNumber		画面パーツ数.
//		b_isNoWarningGamen	警告表示禁止指定.
///	\par	履歴:.\n
// ====================================================================================================================.
PAccessibilityVibrationAmountSetting::PAccessibilityVibrationAmountSetting( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen )
	: PAccessibilityTopMenuCom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen )
	, m_e_VibrationAmount( TYPE_VibrationAmount_Medium )
{
}

// ====================================================================================================================.
//	機能：P0N0112000_振動量設定：画面開始処理.
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationAmountSetting::startGamen()
{
	Bool bl_isContinue = bl_isUnder;
	
	PAccessibilityTopMenuCom::startGamen();
	
	if( True != bl_isContinue ) {
		APIC_OperationSetting* pc_Vibration =	PnlComGetOperationSetting();
		if ( NULL != pc_Vibration ) {
			m_e_VibrationAmount = pc_Vibration->getVibrationAmount();		// バイブの強さを取得する.
		}
	}
}

// ====================================================================================================================.
//	機能：P0N0112000_振動量設定：画面セットアップ処理.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//	ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationAmountSetting::setupGamen( short ss_option )
{
	PAccessibilityTopMenuCom::setupGamen( ss_option );
#ifdef DEF_LCD_7INCH
	setupMessageAreaAccessibility( s_pENLMSGVibrationAmount_W560H056A );
#else
	setupMessageAreaAccessibility( s_pENLMSGVibrationAmount_W560H048A );
#endif
	short ss_SelectIndex_PanelKey = PNLD_AccessibilityVibrationMedium;
	switch( m_e_VibrationAmount ) {
		case TYPE_VibrationAmount_Small:	// 弱.
			ss_SelectIndex_PanelKey = PNLD_AccessibilityVibrationSmall;
			break;
		case TYPE_VibrationAmount_Medium:	// 中.
			ss_SelectIndex_PanelKey = PNLD_AccessibilityVibrationMedium;
			break;
		case TYPE_VibrationAmount_Large:	// 強.
			ss_SelectIndex_PanelKey = PNLD_AccessibilityVibrationLarge;
			break;
		default: break;
	}
	Rtb_AccessibilityVibrationAmountKey.select( ss_SelectIndex_PanelKey );
}

// ====================================================================================================================.
//	機能：P0N0112000_振動量設定：OK.
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationAmountSetting::procOk( uchar option )
{
	APIC_OperationSetting* pc_Vibration =  PnlComGetOperationSetting();
	if ( NULL == pc_Vibration ) {
		return;
	}
	pc_Vibration->setVibrationAmount( m_e_VibrationAmount );		// バイブパターンを設定する.
	reqVibration();
	nextGamen = &gp_AccessibilityVibrationSetting;
}

// ====================================================================================================================.
//	機能：P0N0112000_振動量設定：ボタン(押された).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//	ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationAmountSetting::procButtonOn( uchar option )
{
	switch( option ) {
		case PNLD_AccessibilityVibrationSmall:	// 弱.
			m_e_VibrationAmount = TYPE_VibrationAmount_Small;
			break;
		case PNLD_AccessibilityVibrationMedium:	// 中.
			m_e_VibrationAmount = TYPE_VibrationAmount_Medium;
			break;
		case PNLD_AccessibilityVibrationLarge:	// 強.
			m_e_VibrationAmount = TYPE_VibrationAmount_Large;
			break;
		default: break;
	}
	reqVibration(PCGE_VibrationType_Unspecified, m_e_VibrationAmount);
	setupGamen( option );
}

// ====================================================================================================================.
//	機能：P0N0112000_振動量設定：画面の音声情報を設定.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：.
//	戻値    ：.
// ====================================================================================================================.
void PAccessibilityVibrationAmountSetting::setVoiceGamenInfo()
{
	m_pstVoiceGamenInfo = &VGI_AccessibilityVibrationAmountSetting;
}

// ====================================================================================================================.
//	機能：P0N0113000_バイブパターン設定：コンストラクタ.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		pc_partsTable		画面パーツテーブル.
//		ss_partsNumber		画面パーツ数.
//		b_isNoWarningGamen	警告表示禁止指定.
///	\par	履歴:.\n
// ====================================================================================================================.
PAccessibilityVibrationPatternSetting::PAccessibilityVibrationPatternSetting( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen )
	: PAccessibilityTopMenuCom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen )
	, m_e_VibrationType( TYPE_VibrationType_TypeA )
{
}

// ====================================================================================================================.
//	機能：P0N0113000_バイブパターン設定：画面開始処理.
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationPatternSetting::startGamen()
{
	Bool bl_isContinue = bl_isUnder;
	
	PAccessibilityTopMenuCom::startGamen();
	
	if( True != bl_isContinue ) {
		APIC_OperationSetting* pc_Vibration =	PnlComGetOperationSetting();
		if ( NULL != pc_Vibration ) {
			m_e_VibrationType = pc_Vibration->getVibrationType();		// バイブパターンを取得する.
		}
	}
}

// ====================================================================================================================.
//	機能：P0N0113000_バイブパターン設定：画面セットアップ処理.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//	ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationPatternSetting::setupGamen( short ss_option )
{
	PAccessibilityTopMenuCom::setupGamen( ss_option );
#ifdef DEF_LCD_7INCH
	setupMessageAreaAccessibility( s_pENLMSGVibrationPattern_W560H056A );
#else
	setupMessageAreaAccessibility( s_pENLMSGVibrationPattern_W560H048A );
#endif
	short ss_SelectIndex_PanelKey = PNLD_AccessibilityVibrationPattern1;
	switch( m_e_VibrationType ) {
		case TYPE_VibrationType_TypeA:	// パターン1.
			ss_SelectIndex_PanelKey = PNLD_AccessibilityVibrationPattern1;
			break;
		case TYPE_VibrationType_TypeB:	// パターン2.
			ss_SelectIndex_PanelKey = PNLD_AccessibilityVibrationPattern2;
			break;
		case TYPE_VibrationType_TypeC:	// パターン3.
			ss_SelectIndex_PanelKey = PNLD_AccessibilityVibrationPattern3;
			break;
		default: break;
	}
	Rtb_AccessibilityVibrationpatternKey.select( ss_SelectIndex_PanelKey );
}

// ====================================================================================================================.
//	機能：P0N0113000_バイブパターン設定：OK.
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationPatternSetting::procOk( uchar option )
{
	APIC_OperationSetting* pc_Vibration =  PnlComGetOperationSetting();
	if ( NULL == pc_Vibration ) {
		return;
	}
	pc_Vibration->setVibrationType( m_e_VibrationType );		// バイブパターンを設定する.
	reqVibration();
	bl_isUnder = True;
	nextGamen = &gp_AccessibilityVibrationSingleTap;
}

// ====================================================================================================================.
//	機能：P0N0113000_バイブパターン設定：ボタン(押された).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//	ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationPatternSetting::procButtonOn( uchar option )
{
	switch( option ) {
		case PNLD_AccessibilityVibrationPattern1:	// パターン1.
			m_e_VibrationType = TYPE_VibrationType_TypeA;
			break;
		case PNLD_AccessibilityVibrationPattern2:	// パターン2.
			m_e_VibrationType = TYPE_VibrationType_TypeB;
			break;
		case PNLD_AccessibilityVibrationPattern3:	// パターン3.
			m_e_VibrationType = TYPE_VibrationType_TypeC;
			break;
		default: break;
	}
	reqVibration(m_e_VibrationType, PCGE_VibrationAmount_Unspecified);
	setupGamen( option );
}

// ====================================================================================================================.
//	機能：P0N0113000_バイブパターン設定：画面の音声情報を設定.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：.
//	戻値    ：.
// ====================================================================================================================.
void PAccessibilityVibrationPatternSetting::setVoiceGamenInfo()
{
	m_pstVoiceGamenInfo = &VGI_AccessibilityVibrationPatternSetting;
}

// ====================================================================================================================.
//	機能：P0N0114000_電源キー振動設定：コンストラクタ.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		pc_partsTable		画面パーツテーブル.
//		ss_partsNumber		画面パーツ数.
//		b_isNoWarningGamen	警告表示禁止指定.
///	\par	履歴:.\n
// ====================================================================================================================.
PAccessibilityVibrationPowerKeySetting::PAccessibilityVibrationPowerKeySetting( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen )
	: PAccessibilityTopMenuCom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen )
	, m_e_VibrationPowerKey( TYPE_VibrationResponse_On )
{
}

// ====================================================================================================================.
//	機能：P0N0114000_電源キー振動設定：画面開始処理.
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationPowerKeySetting::startGamen()
{
	Bool bl_isContinue = bl_isUnder;
	
	PAccessibilityTopMenuCom::startGamen();
	
	if( True != bl_isContinue ) {
		APIC_OperationSetting* pc_Vibration =	PnlComGetOperationSetting();
		if ( NULL != pc_Vibration ) {
			m_e_VibrationPowerKey = pc_Vibration->getVibrationLongTapPowerKey();		// 電源キーのバイブ情報を取得する.
		}
	}
}

// ====================================================================================================================.
//	機能：P0N0114000_電源キー振動設定：画面セットアップ処理.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//	ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationPowerKeySetting::setupGamen( short ss_option )
{
	PAccessibilityTopMenuCom::setupGamen( ss_option );
//comment 2018/4/9 SCD2-Sky肥後 [Eagle][制御構成変更No.92-1]振動パネル対応.
#ifdef DEF_LCD_7INCH
	setupMessageAreaAccessibility( s_pENLMSGVibrationLongTapPowerKey_W560H056A );
#else
	setupMessageAreaAccessibility( s_pENLMSGVibrationLongTapPowerKey_W560H048A );
#endif
	
	short ss_SelectIndex_PanelKey = PNLD_AccessibilityVibrationPowerKey_On;
	switch( m_e_VibrationPowerKey ) {
		case TYPE_VibrationResponse_On:	// する.
			ss_SelectIndex_PanelKey = PNLD_AccessibilityVibrationPowerKey_On;
			break;
		case TYPE_VibrationResponse_Off:	// しない.
			ss_SelectIndex_PanelKey = PNLD_AccessibilityVibrationPowerKey_Off;
			break;
		default: break;
	}
	Rbt_AccessibilityVibrationPowerKeyYesNo.select( ss_SelectIndex_PanelKey );
}

// ====================================================================================================================.
//	機能：P0N0114000_電源キー振動設定：OK.
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationPowerKeySetting::procOk( uchar option )
{
	APIC_OperationSetting* pc_Vibration =  PnlComGetOperationSetting();
	if ( NULL == pc_Vibration ) {
		return;
	}
	pc_Vibration->setVibrationLongTapPowerKey( m_e_VibrationPowerKey );		// 電源キーの振動状態を設定する.
	nextGamen = &gp_AccessibilityVibrationSetting;
}

// ====================================================================================================================.
//	機能：P0N0114000_電源キー振動設定：ボタン(押された).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//	ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationPowerKeySetting::procButtonOn( uchar option )
{
	switch( option ) {
		case PNLD_AccessibilityVibrationPowerKey_On:	// 振動する.
			m_e_VibrationPowerKey = TYPE_VibrationResponse_On;
			break;
		case PNLD_AccessibilityVibrationPowerKey_Off:	// 振動しない.
			m_e_VibrationPowerKey = TYPE_VibrationResponse_Off;
			break;
		default: break;
	}
	setupGamen( option );
}

// ====================================================================================================================.
//	機能：P0N0114000_電源キー振動設定：画面の音声情報を設定.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：.
//	戻値    ：.
// ====================================================================================================================.
void PAccessibilityVibrationPowerKeySetting::setVoiceGamenInfo()
{
	m_pstVoiceGamenInfo = &VGI_AccessibilityVibrationPowerKeySetting;
}

// comment 2018/02/28 SCD2 Y.Yamamoto [Eagle][制御構成変更No.92]振動パネル対応.
// ====================================================================================================================.
//	機能：P0N0111100_シングルタップ：コンストラクタ.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		pc_partsTable		画面パーツテーブル.
//		ss_partsNumber		画面パーツ数.
//		b_isNoWarningGamen	警告表示禁止指定.
///	\par	履歴:.\n
// ====================================================================================================================.
PAccessibilityVibrationSingleTap::PAccessibilityVibrationSingleTap( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen )
	: PAccessibilityTopMenuCom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen )
	, m_e_VibrationPanelKey( TYPE_VibrationResponse_On )
//comment 2018/4/9 SCD2-Sky肥後 [Eagle][制御構成変更No.92-1]振動パネル対応.
	, m_e_VibrationType( TYPE_VibrationType_TypeA )
{
}

// ====================================================================================================================.
//	機能：P0N0111100_シングルタップ：画面開始処理.
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationSingleTap::startGamen()
{
	Bool bl_isContinue = bl_isUnder;
	
	PAccessibilityTopMenuCom::startGamen();
//comment 2018/4/9 SCD2-Sky肥後 [Eagle][制御構成変更No.92-1]振動パネル対応.	
	APIC_OperationSetting* pc_Vibration =  PnlComGetOperationSetting();
	if ( NULL == pc_Vibration ) {
		return;
	}
	if( True != bl_isContinue ) {
//		APIC_OperationSetting* pc_Vibration =  APIC_OperationSetting::newInstance();
		m_e_VibrationPanelKey = pc_Vibration->getVibrationResponse();		// タップ時の振動が有効かどうかを取得する.
	}
//comment 2018/4/9 SCD2-Sky肥後 [Eagle][制御構成変更No.92-1]振動パネル対応.
	m_e_VibrationType = pc_Vibration->getVibrationType();		// バイブパターンを取得する.
}

// ====================================================================================================================.
//	機能：P0N0111100_シングルタップ：画面セットアップ処理.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//	ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationSingleTap::setupGamen( short ss_option )
{
//comment 2018/4/9 SCD2-Sky肥後 [Eagle][制御構成変更No.92-1]振動パネル対応.	
//	PAccessibilityTopMenuCom::setupGamen();
//	setupMessageAreaAccessibility( s_pENLMSGVibrationSingleTap_W368H050A );
	PAccessibilityTopMenuCom::setupGamen( ss_option );
	
#ifdef DEF_LCD_7INCH
	setupMessageAreaAccessibility( s_pENLMSGVibrationSingleTap_W560H056A );
#else
	setupMessageAreaAccessibility( s_pENLMSGVibrationSingleTap_W560H048A );
#endif	
//comment 2018/4/9 SCD2-Sky肥後 [Eagle][制御構成変更No.92-1]振動パネル対応.
//	uchar uc_SelectIndex_PanelKey = PNLD_AccessibilityVibrationSingleTapSet_PanelKey_On;
	short ss_SelectIndex_PanelKey = PNLD_AccessibilityVibrationSingleTapSet_PanelKey_On;
	Bool bl_VibrationPatternDisp = True;
	if ( TYPE_VibrationResponse_On == m_e_VibrationPanelKey ) {
//comment 2018/4/9 SCD2-Sky肥後 [Eagle][制御構成変更No.92-1]振動パネル対応.
//		uc_SelectIndex_PanelKey = PNLD_AccessibilityVibrationSingleTapSet_PanelKey_On;
		ss_SelectIndex_PanelKey = PNLD_AccessibilityVibrationSingleTapSet_PanelKey_On;
		bl_VibrationPatternDisp = True;
	} else {
//comment 2018/4/9 SCD2-Sky肥後 [Eagle][制御構成変更No.92-1]振動パネル対応.
//		uc_SelectIndex_PanelKey = PNLD_AccessibilityVibrationSingleTapSet_PanelKey_Off;
		ss_SelectIndex_PanelKey = PNLD_AccessibilityVibrationSingleTapSet_PanelKey_Off;
		bl_VibrationPatternDisp = False;
	}
//comment 2018/4/9 SCD2-Sky肥後 [Eagle][制御構成変更No.92-1]振動パネル対応.
//	Rbt_AccessibilityVibrationSingleTapPanelKeyYesNo.select( uc_SelectIndex_PanelKey );
	Rtb_AccessibilityDo_NotDo.select( ss_SelectIndex_PanelKey );
	Key_AccessibilityVibrationPatternKey.activate( bl_VibrationPatternDisp );
	Bak_EnlAccessibilityVibrationPatternStatus.activate( bl_VibrationPatternDisp );

//comment 2018/4/9 SCD2-Sky肥後 [Eagle][制御構成変更No.92-1]振動パネル対応.
	switch( m_e_VibrationType ) {
		case TYPE_VibrationType_TypeA:	// パターン1.
			Bak_EnlAccessibilityVibrationPatternStatus.changeText( s_pENLKEYVibrationPattern1_W224H032H );
			break;
		case TYPE_VibrationType_TypeB:	// パターン2.
			Bak_EnlAccessibilityVibrationPatternStatus.changeText( s_pENLKEYVibrationPattern2_W224H032H );
			break;
		case TYPE_VibrationType_TypeC:	// パターン3.
			Bak_EnlAccessibilityVibrationPatternStatus.changeText( s_pENLKEYVibrationPattern3_W224H032H );		
			break;
		default: break;
	}
}

// ====================================================================================================================.
//	機能：P0N0111100_シングルタップ：OK.
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationSingleTap::procOk( uchar option )
{
	APIC_OperationSetting* pc_Vibration =  APIC_OperationSetting::newInstance();
//comment 2018/4/9 SCD2-Sky肥後 [Eagle][制御構成変更No.92-1]振動パネル対応.
	if ( NULL == pc_Vibration ) {
		return;
	}
	pc_Vibration->setVibrationResponse( m_e_VibrationPanelKey );		// タップ時の振動が有効かどうかを設定する.
//comment 2018/4/9 SCD2-Sky肥後 [Eagle][制御構成変更No.92-1]振動パネル対応.
//	PAccessibilityTopMenuCom::procOk( option );
	nextGamen = &gp_AccessibilityVibrationPanelKeySetting;
}

// ====================================================================================================================.
//	機能：P0N0111100_シングルタップ：ボタン(押された).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//	ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationSingleTap::procButtonOn( uchar option )
{
	switch( option ) {
		case 10:				// パネルキー振動する.
			m_e_VibrationPanelKey = TYPE_VibrationResponse_On;
			break;
		case 11:				// パネルキー振動しない.
			m_e_VibrationPanelKey = TYPE_VibrationResponse_Off;
			break;
		default: break;
	}
	setupGamen( option );
}
//comment 2018/4/9 SCD2-Sky肥後 [Eagle][制御構成変更No.92-1]振動パネル対応.
// ====================================================================================================================.
//	機能：P0N0111100_シングルタップ：画面移動処理.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//	ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationSingleTap::procGoToPage( uchar option )
{
	switch( option ) {
		case PNLD_AccessibilityVibrationPatternSetting:				// バイブパターンキー.
			nextGamen = &gp_AccessibilityVibrationPatternSetting;
			break;
		default: break;
	}
}

// ====================================================================================================================.
//	機能：P0N0111100_シングルタップ：画面の音声情報を設定.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：.
//	戻値    ：.
// ====================================================================================================================.
void PAccessibilityVibrationSingleTap::setVoiceGamenInfo()
{
	m_pstVoiceGamenInfo = &VGI_AccessibilityVibrationSingleTap;
}

//comment 2018/4/9 SCD2-Sky肥後 [Eagle][制御構成変更No.92-1]振動パネル対応.
// ====================================================================================================================.
//	機能：P0N0111200_ロングタップ：コンストラクタ.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//		pc_partsTable		画面パーツテーブル.
//		ss_partsNumber		画面パーツ数.
//		b_isNoWarningGamen	警告表示禁止指定.
///	\par	履歴:.\n
// ====================================================================================================================.
PAccessibilityVibrationLongTap::PAccessibilityVibrationLongTap( PDisplayParts** pc_partsTable, short ss_partsNumber, Bool b_isNoWarningGamen )
	: PAccessibilityTopMenuCom( pc_partsTable, ss_partsNumber, b_isNoWarningGamen )
	, m_e_VibrationLongTapKey( TYPE_VibrationResponse_On )
{
}

// ====================================================================================================================.
//	機能：P0N0111200_ロングタップ：画面開始処理.
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationLongTap::startGamen()
{	
	Bool bl_isContinue = bl_isUnder;
	
	PAccessibilityTopMenuCom::startGamen();
	
	if( True != bl_isContinue ) {
		APIC_OperationSetting* pc_Vibration =  PnlComGetOperationSetting();
		if ( NULL != pc_Vibration ) {
			m_e_VibrationLongTapKey = pc_Vibration->getVibrationLongTapTouchKey();		// ロングタップ時の振動が有効かどうかを取得する.
		}
	}
}

// ====================================================================================================================.
//	機能：P0N0111200_ロングタップ：画面セットアップ処理.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//	ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationLongTap::setupGamen( short ss_option )
{
	PAccessibilityTopMenuCom::setupGamen( ss_option );
//comment 2018/4/9 SCD2-Sky肥後 [Eagle][制御構成変更No.92-1]振動パネル対応.
#ifdef DEF_LCD_7INCH
	setupMessageAreaAccessibility( s_pENLMSGVibrationLongTap_W560H056A );
#else
	setupMessageAreaAccessibility( s_pENLMSGVibrationLongTap_W560H048A );
#endif
	
	short ss_SelectIndex_PanelKey = PNLD_AccessibilityVibrationLongTapKey_On;
	if ( TYPE_VibrationResponse_On == m_e_VibrationLongTapKey ) {
		ss_SelectIndex_PanelKey = PNLD_AccessibilityVibrationLongTapKey_On;
	} else {
		ss_SelectIndex_PanelKey = PNLD_AccessibilityVibrationLongTapKey_Off;
	}
	Rbt_AccessibilityVibrationLongTapKeyYesNo.select( ss_SelectIndex_PanelKey );
}

// ====================================================================================================================.
//	機能：P0N0111200_ロングタップ：OK.
//	-------------------------------------------------------------------------------------------------------------------.
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationLongTap::procOk( uchar option )
{
	APIC_OperationSetting* pc_Vibration =  PnlComGetOperationSetting();
	if ( NULL == pc_Vibration ) {
		return;
	}
	pc_Vibration->setVibrationLongTapTouchKey( m_e_VibrationLongTapKey );		// ロングタップ時の振動が有効かどうかを設定する.
	nextGamen = &gp_AccessibilityVibrationPanelKeySetting;
}

// ====================================================================================================================.
//	機能：P0N0111200_ロングタップ：ボタン(押された).
//	-------------------------------------------------------------------------------------------------------------------.
//	引数：.
//	ss_option	オプション(デフォルト:0).
///	\par	履歴:.\n
// ====================================================================================================================.
void PAccessibilityVibrationLongTap::procButtonOn( uchar option )
{
	switch( option ) {
		case PNLD_AccessibilityVibrationLongTapKey_On:				// 振動する.
			m_e_VibrationLongTapKey = TYPE_VibrationResponse_On;
			break;
		case PNLD_AccessibilityVibrationLongTapKey_Off:				// 振動しない.
			m_e_VibrationLongTapKey = TYPE_VibrationResponse_Off;
			break;
		default: break;
	}
	setupGamen( option );
}

// ====================================================================================================================.
//	機能：P0N0111200_ロングタップ：画面の音声情報を設定.
//	-------------------------------------------------------------------------------------------------------------------.
//	引数	：.
//	戻値    ：.
// ====================================================================================================================.
void PAccessibilityVibrationLongTap::setVoiceGamenInfo()
{
	m_pstVoiceGamenInfo = &VGI_AccessibilityVibrationLongTap;
}