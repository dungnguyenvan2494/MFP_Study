#ifdef QT_PANEL
#include <QWidget>
#include <QInputEvent>
#include <QtGui>
#include "qtpanel.h"
#include <QPointer>
#include <QTimer>
#include "panel/plibl/plibl_CLCD/qtaccessibility.h"

#include <panel/pnlmngr/pnlmngrg/pnlmanager.h>
#include <panel/plibl/plibl_CLCD/tskpnldraw_CLCD.h>
#include <system/thr/PanelHW/THRC_PanelHardCom.h>

#include <system/sys/OsWrapper/SYSC_OSWrapper.h>
#include <panel/plibh/plibhg/plibh.h>
#include <panel/pdp/pdpg/pdpi.h>			// 電源管理.

#include <system/nvd/APIC_Setting.h>
#include <system/nvd/APIC_BrowserInfo.h>
#include "system/sys/APIC_TabletIF.h"

#include <panel/plibl/plibl_CLCD/qtevent.h>

#include "mtuiagentmanager.h"
#include "mtuiwidgetmanager.h"
#include "mtuistoragemanager.h"
#include "mtuigraphicmanager.h"
#include "mtuisoundmanager.h"
#include "mtuiagent.h"
#include <LAPC_WND_Notify_define.h>
#include "LAPC_BFW_EventHandler.h"

// comment 2015/01/08 OPS1-Sky_鐘江 [Poseidon][CQ508416].
#include "GFWC_SoundManager.h"
#include "GFWC_SoundManagerObserver.h"

// comment 2018/03/05 SCD2 Y.Yamamoto [Eagle][制御構成変更No.92]振動パネル対応.
#include "GFWC_VibrationManager.h"
#include "GFWC_VibrationManagerObserver.h"
#include <panel/pcgcom/pcgcom_CLCD/pcgcom_CLCD.h>
//comment 2018/4/9 SCD2-Sky肥後 [Eagle][制御構成変更No.92-1]振動パネル対応.
#include <panel/plibh/plibh_CLCD2/plibh_CLCD2.h>
// comment 2020/09/17 Sky東島 [IT6_2.0MR3][OP_BTS-27689].
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
#include <panel/plibh/plibh_CLCD2/PNLC_SH3PaneMenuControlLib.h>
#endif

#if DEF_PANEL_DEBUG
#define DEBUG_PANEL_PRINT printf
#else
#define DEBUG_PANEL_PRINT(...)
#endif

//モジュール内定義値.
#define DEF_QtPanelWakeupTime		25			// 25msで定期起床.
#define DEF_MouseSendStartTime		30			// 750msで押し続け開始時間.
#define DEF_MouseSendIntervalTime	4			// 100msで押し続け間隔時間.
#define DEF_KeyUnknown				0xFFFF		// 無効なKeyCode.
#define DEF_KeyRelease				0xFF		// 開放されたキーコード.
#define DEF_QtPanelEventSendTime 1000		// 1000msでイベント送信.


#define DEF_KeyRepeatStartTime			(750)								// キーリピート開始時間のデフォルト値.
#define DEF_KeyRepeatIntervalTime		(100)								// キーリピート感覚時間のデフォルト値.
#define DEF_UsbKbdKeySendStartTime		(DEF_KeyRepeatStartTime/DEF_QtPanelWakeupTime)		// キーリピート開始時間のデフォルト値(750ms)を定期起床回数に換算した値.
#define DEF_UsbKbdKeySendIntervalTime	(DEF_KeyRepeatIntervalTime/DEF_QtPanelWakeupTime)	// キーリピート感覚時間のデフォルト値(100ms)を定期起床回数に換算した値.
uchar QtPanel::uc_cmpPacket[TYPD_PACKET_MAXSIZE];							// データ有無チェック用バッファ.


ushort KmApplication::us_LedOnOffState = 0;	// Led状態.
ushort KmApplication::us_LedBlinkState = 0;	// Led点滅状態.
ushort KmApplication::us_LedBlinkTime = 0;	// Led点滅時間.

//-----------------------------------------------------------------------------.
// クラス NoOpeTimer_EventHandler.
//-----------------------------------------------------------------------------.
class NoOpeTimer_EventHandler : public LAPC_BFW_EventHandler
{

public:
	// コンストラクタ.
	NoOpeTimer_EventHandler()
	{
		// 処理なし.
	}

	// デストラクタ.
	virtual ~NoOpeTimer_EventHandler()
	{
		// 処理なし.
	}

	// イベントハンドラ処理.
	void eventHandler( const char* rcv_data, size_t rcv_size )
	{
		// 処理なし.
	}

	// タイマーハンドラ処理.
	void timerHandler()
	{
		// Event送信用timer処理呼び出し.
		QtPanel::newInstance()->recv_qwsProcessEventTimer();
	}

	// acceptハンドラ処理.
	void acceptHandler()
	{
		// 処理なし.
	}

	// イベントハンドラ処理.
	void eventProc( int fd )
	{
		// 処理なし.
	}
};

//-----------------------------------------------------------------------------.
// クラス RepeatTimer_EventHandler.
//-----------------------------------------------------------------------------.
class RepeatTimer_EventHandler : public LAPC_BFW_EventHandler {
public:
	// コンストラクタ.
	RepeatTimer_EventHandler()
	{
		// 処理なし.
	}

	// デストラクタ.
	virtual ~RepeatTimer_EventHandler()
	{
		// 処理なし.
	}

	// イベントハンドラ処理.
	void eventHandler( const char* rcv_data, size_t rcv_size )
	{
		// 処理なし.
	}

	// タイマーハンドラ処理.
	void timerHandler()
	{
		// 定期起床処理呼び出し.
		QtPanel::newInstance()->cyclicLoop();
	}

	// acceptハンドラ処理.
	void acceptHandler()
	{
		// 処理なし.
	}

	// イベントハンドラ処理.
	void eventProc( int fd )
	{
		// 処理なし.
	}
};

//------------------------------------------------------------------------------
/**
 *	@brief	キーイベント受信オブザーバクラス
 */
//------------------------------------------------------------------------------
class Panel_KeyObserver : public GFWC_KeyObserver
{

public:
	/**
	 *	@brief	コンストラクタ
	 */
	Panel_KeyObserver( void ) {}

	/**
	 *	@brief	デストラクタ
	 */
	virtual ~Panel_KeyObserver( void ) {}

	/**
	 *	@brief	キー押下イベントを通知する
	 *	@param	c_Point		キーイベント情報
	 */
	virtual void keyPressed( GFWS_KeyEvent& s_KeyEvent )
	{
		QtPanel::newInstance()->keyPressEvent( &s_KeyEvent );
	}

	/**
	 *	@brief	キー解放イベントを通知する
	 *	@param	c_Point		キーイベント情報
	 */
	virtual void keyReleased( GFWS_KeyEvent& s_KeyEvent )
	{
		QtPanel::newInstance()->keyReleaseEvent( &s_KeyEvent );
	}
};


// comment 2015/01/08 OPS1-Sky_鐘江 [Poseidon][CQ508416].
//------------------------------------------------------------------------------
/**
 *	@brief	Buzzer通知オブザーバクラス 
 */
//------------------------------------------------------------------------------
class Panel_SoundObserver : public GFWC_SoundManagerObserver
{
public:
	/**
	 *	@brief	コンストラクタ 
	 */
    Panel_SoundObserver( void ) {}
	
	/**
	 *	@brief	デストラクタ 
	 */
	virtual ~Panel_SoundObserver( void ) {}
	

	/**
	 *	@brief	Buzzer音通知.
	 *	@param[in]	e_SoundID		入力音.
	 */
	virtual void notifySound( GFWE_SoundId e_SoundID ) {
	    // パネルマネージャにメッセージを送信する.
	    int32_t type = (int32_t)MT_DontCare;                  // メッセージタイプ.
	    int32_t message = (int32_t)MS_QtBuzzer;               // メッセージ.
	    int32_t parameter = (int32_t)((type << 24) + (message << 16) + (int32_t)e_SoundID);
	    PMGC_PanelManager::newInstance()->QtTouchPanelKeyNotice(parameter);
	};
};

// comment 2018/03/05 SCD2 Y.Yamamoto [Eagle][制御構成変更No.92]振動パネル対応.
//------------------------------------------------------------------------------
/**
 *	@brief	バイブレーション通知オブザーバクラス 
 */
//------------------------------------------------------------------------------
class Panel_VibrationObserver : public GFWC_VibrationManagerObserver
{
public:
	/**
	 *	@brief	コンストラクタ 
	 */
    Panel_VibrationObserver( void ) {}
	
	/**
	 *	@brief	デストラクタ 
	 */
	virtual ~Panel_VibrationObserver( void ) {}
	

	/**
	 *	@brief	バイブレーション通知.
	 *	@param[in]	e_VibrationOperation		操作種別.
	 *	@param[in]	option						オプション値.
	 */
//	virtual void notifyVibration( GFWE_VibrationOperation e_VibrationOperation , GFWE_VibrationOption option ) {
	virtual void notifyVibration( GFWE_VibrationType e_VibrationType , GFWE_VibrationAmount e_VibrationAmount ) {
//comment 2018/4/9 SCD2-Sky肥後 [Eagle][制御構成変更No.92-1]振動パネル対応.
//		reqVibration( (PCGE_VibrationOperation)e_VibrationOperation , (PCGE_VibrationOption)option );
		reqVibration( (PCGE_VibrationType)e_VibrationType , (PCGE_VibrationAmount)e_VibrationAmount );
	};

//comment 2018/4/9 SCD2-Sky肥後 [Eagle][制御構成変更No.92-1]振動パネル対応.
	/**
	 *	@brief	振動状態を設定する.
	 */
	virtual void notifySetVibration( GFWD_BOOL bl_Vibration ) {
		setPanelSideVibration( bl_Vibration );
	};
};


//-----------------------------------------------------------------------------.
// コンストラクタ.
//-----------------------------------------------------------------------------.
QtPanel::QtPanel()
{
	// クラスの初期化.
	InitClass();

	// 画面サイズ/ポジションの設定
	setPosition( GFWC_Point( 0, 0 ) );

	// タッチイベントオブザーバ登録.
	m_pc_BridgeTouch = new PPnlMgrBridgeTouch();
	attachTouchObserver( m_pc_BridgeTouch );

	// キーイベントオブザーバ登録.
	m_pc_PanelKey = new Panel_KeyObserver();
	attachKeyObserver( m_pc_PanelKey );

// comment 2015/01/08 OPS1-Sky_鐘江 [Poseidon][CQ508416].
	// Buzzer通知オブザーバ登録.
	m_pc_SoundObserver = new Panel_SoundObserver;
	GFWC_SoundManager::attachObserver(m_pc_SoundObserver);

// comment 2018/03/05 SCD2 Y.Yamamoto [Eagle][制御構成変更No.92]振動パネル対応.
	// Vibration通知オブザーバ登録.
	m_pc_VibrationObserver = new Panel_VibrationObserver;
	GFWC_VibrationManager::attachObserver(m_pc_VibrationObserver);

	// 無操作タイマーイベントハンドラ生成.
	m_pc_noopehandler = new NoOpeTimer_EventHandler();

	// キーリピートタイマーイベントハンドラ生成.
	m_pc_RepeatTimerHandler	= new RepeatTimer_EventHandler();
	
	m_IsSetRepeatTimerEvent = false;

	bPowerSave = false;
	bLowPowerGamen = false;

	bKeyEventState = false;
	bMouseEventState = false;	// タッチパネル押下フラグ.

//[Qt4.7->Qt5.3]KMカスタマイズ start Que形式RingBuf変更 CQ.512406------------------------.    
    for(int i = TYPD_PKEY_QMAX - 1 ; i >= 0 ; i--){
	    keyData[i] = DEF_KeyUnknown;
    }

    keyDataFront = 0;
    keyDataRear  = 0;
//[Qt4.7->Qt5.3]KMカスタマイズ end   Que形式RingBuf変更 CQ.512406------------------------.

	mouseTimeState = 0;
	sendMouseData.data = 0;
	m_mouseData.data = 0;
	DirectKeyData = 0;
	initUsbKbd();							//データ初期化.

	b_EventSend = False;



	b_qtscvFlag = false;	// スクリーンセーバ判定フラグ.

	// 管理クラス生成～初期化.
	QWidget*	widget	= static_cast<QWidget*>( getLocalInstance() );
	PMTC_AgentManager::instance()->initialize( widget );
	PMTC_AgentManager::instance()->registerScreenWidget( widget );
	PMTC_WidgetManager::instance()->initialize( widget );

	PMTC_ModelManager::instance()->initialize();
	PMTC_StorageManager::instance()->initialize();
	PMTC_GraphicManager::instance()->initialize();
	PMTC_SoundManager::instance()->initialize();
}

QtPanel::~QtPanel()
{
//	if( NULL != m_pc_BridgeTouch ) {
//		delete m_pc_BridgeTouch;
//		m_pc_BridgeTouch = NULL;
//	}
//	if( NULL != m_pc_PanelKey ) {
//		delete m_pc_PanelKey;
//		m_pc_PanelKey = NULL;
//	}
//	if( NULL != m_pc_noopehandler ) {
//		delete m_pc_noopehandler;
//		m_pc_noopehandler = NULL;
//	}
//	if( NULL != m_pc_RepeatTimerHandler ) {
//		delete m_pc_RepeatTimerHandler;
//		m_pc_RepeatTimerHandler = NULL;
//	}

}
void QtPanel::startTimerEvent()
{
	//キーリピートタイマー設定(インターバルタイマー).
	if( false == m_IsSetRepeatTimerEvent )
	{
		API_RESULT	result	= 0;
		LAPC_BFW_System*	p_sys	= LAPC_BFW_System::newInstance();
		result	= p_sys->setRepeatTimerEvent( DEF_QtPanelWakeupTime, m_pc_RepeatTimerHandler, &m_sl_RepeatTimer_id );
		
		if( result == LAPE_ResultOK ) {
			m_IsSetRepeatTimerEvent = true;
		}
	}
}

void QtPanel::initializedGraphic(void)
{
	static bool init_flg = false;
	if(init_flg == false){
		QWidget* w = static_cast<QWidget*>( getLocalInstance() );
	
		if((w != NULL)&&(w->isVisible() == false)){
			QTimer::singleShot(0, w, SLOT(show()));
			init_flg = true;
		}
	}
}

// 修飾キー管理テーブル定義.
struct st_isKeyCodeModifier {
	uchar	uc_KeyCodeLinux;	// Linuxキーコード.
	bool	b_ModifierKeys;		// そのキーコードがModifierKey(修飾キー)かどうか.
};
const st_isKeyCodeModifier st_isKeyCodeModifierTbl[ ] = {
	{	GFWD_Key_USBKBD_Esc,			false	},	// esc.
	{	GFWD_Key_USBKBD_1,				false	},	// 1.
	{	GFWD_Key_USBKBD_2,				false	},	// 2.
	{	GFWD_Key_USBKBD_3,				false	},	// 3.
	{	GFWD_Key_USBKBD_4,				false	},	// 4.
	{	GFWD_Key_USBKBD_5,				false	},	// 5.
	{	GFWD_Key_USBKBD_6,				false	},	// 6.
	{	GFWD_Key_USBKBD_7,				false	},	// 7.
	{	GFWD_Key_USBKBD_8,				false	},	// 8.
	{	GFWD_Key_USBKBD_9,				false	},	// 9.
	{	GFWD_Key_USBKBD_0,				false	},	// 0.
	{	GFWD_Key_USBKBD_Hyphen,			false	},	// -.
	{	GFWD_Key_USBKBD_Tilde,			false	},	// ^.
	{	GFWD_Key_USBKBD_BS,				false	},	// BS.
	{	GFWD_Key_USBKBD_Tab,			false	},	// tab.
	{	GFWD_Key_USBKBD_Q,				false	},	// Q.
	{	GFWD_Key_USBKBD_W,				false	},	// W.
	{	GFWD_Key_USBKBD_E,				false	},	// E.
	{	GFWD_Key_USBKBD_R,				false	},	// R.
	{	GFWD_Key_USBKBD_T,				false	},	// T.
	{	GFWD_Key_USBKBD_Y,				false	},	// Y.
	{	GFWD_Key_USBKBD_U,				false	},	// U.
	{	GFWD_Key_USBKBD_I,				false	},	// I.
	{	GFWD_Key_USBKBD_O,				false	},	// O.
	{	GFWD_Key_USBKBD_P,				false	},	// P.
	{	GFWD_Key_USBKBD_At,				false	},	// @.
	{	GFWD_Key_USBKBD_BracketLeft,	false	},	// [.
	{	GFWD_Key_USBKBD_Enter,			false	},	// enter.
	{	GFWD_Key_USBKBD_CtrlLeft,		true	},	// ctrl(左).
	{	GFWD_Key_USBKBD_A,				false	},	// A.
	{	GFWD_Key_USBKBD_S,				false	},	// S.
	{	GFWD_Key_USBKBD_D,				false	},	// D.
	{	GFWD_Key_USBKBD_F,				false	},	// F.
	{	GFWD_Key_USBKBD_G,				false	},	// G.
	{	GFWD_Key_USBKBD_H,				false	},	// H.
	{	GFWD_Key_USBKBD_J,				false	},	// J.
	{	GFWD_Key_USBKBD_K,				false	},	// K.
	{	GFWD_Key_USBKBD_L,				false	},	// L.
	{	GFWD_Key_USBKBD_Semicolon,		false	},	// ;.
	{	GFWD_Key_USBKBD_Colon,			false	},	// :.
	{	GFWD_Key_USBKBD_Hankaku,		false	},	// 半角/全角.
	{	GFWD_Key_USBKBD_ShftLeft,		true	},	// shift(左).
	{	GFWD_Key_USBKBD_BracketRight,	false	},	// ].
	{	GFWD_Key_USBKBD_Z,				false	},	// Z.
	{	GFWD_Key_USBKBD_X,				false	},	// X.
	{	GFWD_Key_USBKBD_C,				false	},	// C.
	{	GFWD_Key_USBKBD_V,				false	},	// V.
	{	GFWD_Key_USBKBD_B,				false	},	// B.
	{	GFWD_Key_USBKBD_N,				false	},	// N.
	{	GFWD_Key_USBKBD_M,				false	},	// M.
	{	GFWD_Key_USBKBD_Comma,			false	},	// ,.
	{	GFWD_Key_USBKBD_Dot,			false	},	// ..
	{	GFWD_Key_USBKBD_Slash,			false	},	// /.
	{	GFWD_Key_USBKBD_ShiftRight,		true	},	// shift(右).
	{	GFWD_Key_USBKBD_Asterisk,		false	},	// *.
	{	GFWD_Key_USBKBD_AltLeft,		true	},	// alt(左).
	{	GFWD_Key_USBKBD_Space,			false	},	// スペース.
	{	GFWD_Key_USBKBD_CapsLock,		false	},	// CapsLock.
	{	GFWD_Key_USBKBD_F1,				false	},	// F1.
	{	GFWD_Key_USBKBD_F2,				false	},	// F2.
	{	GFWD_Key_USBKBD_F3,				false	},	// F3.
	{	GFWD_Key_USBKBD_F4,				false	},	// F4.
	{	GFWD_Key_USBKBD_F5,				false	},	// F5.
	{	GFWD_Key_USBKBD_F6,				false	},	// F6.
	{	GFWD_Key_USBKBD_F7,				false	},	// F7.
	{	GFWD_Key_USBKBD_F8,				false	},	// F8.
	{	GFWD_Key_USBKBD_F9,				false	},	// F9.
	{	GFWD_Key_USBKBD_F10,			false	},	// F10.
	{	GFWD_Key_USBKBD_NumLock,		false	},	// NumLock.
	{	GFWD_Key_USBKBD_ScrollLock,		false	},	// ScrollLock.
	{	GFWD_Key_USBKBD_10Key_7,		false	},	// 7.
	{	GFWD_Key_USBKBD_10Key_8,		false	},	// 8.
	{	GFWD_Key_USBKBD_10Key_9,		false	},	// 9.
	{	GFWD_Key_USBKBD_10Key_Minus,	false	},	// -.
	{	GFWD_Key_USBKBD_10Key_4,		false	},	// 4.
	{	GFWD_Key_USBKBD_10Key_5,		false	},	// 5.
	{	GFWD_Key_USBKBD_10Key_6,		false	},	// 6.
	{	GFWD_Key_USBKBD_10Key_Plus,		false	},	// +.
	{	GFWD_Key_USBKBD_10Key_1,		false	},	// 1.
	{	GFWD_Key_USBKBD_10Key_2,		false	},	// 2.
	{	GFWD_Key_USBKBD_10Key_3,		false	},	// 3.
	{	GFWD_Key_USBKBD_10Key_0,		false	},	// 0.
	{	GFWD_Key_USBKBD_10Key_Dot,		false	},	// ..
	{	GFWD_Key_USBKBD_AngleBracketRight,	false},	// >.
	{	GFWD_Key_USBKBD_F11,			false	},	// F11.
	{	GFWD_Key_USBKBD_F12,			false	},	// F12.
	{	GFWD_Key_USBKBD_BckSlashDown,	false	},	// \(下).
	{	GFWD_Key_USBKBD_Conv,			false	},	// 変換.
	{	GFWD_Key_USBKBD_KANA,			false	},	// カタカナ.
	{	GFWD_Key_USBKBD_NonConv,		false	},	// 無変換.
	{	GFWD_Key_USBKBD_Enter,			false	},	// enter.
	{	GFWD_Key_USBKBD_CtrlRight,		true	},	// ctrl(右).
	{	GFWD_Key_USBKBD_10Key_Slash,	false	},	// /.
	{	GFWD_Key_USBKBD_PrintScrn,		false	},	// PrintScrn.
	{	GFWD_Key_USBKBD_AltRight,		true	},	// alt(右).
	{	GFWD_Key_USBKBD_Home,			false	},	// home.
	{	GFWD_Key_USBKBD_UP,				false	},	// ↑.
	{	GFWD_Key_USBKBD_PageUp,			false	},	// PageUp.
	{	GFWD_Key_USBKBD_Left,			false	},	// ←.
	{	GFWD_Key_USBKBD_Right,			false	},	// →.
	{	GFWD_Key_USBKBD_End,			false	},	// End.
	{	GFWD_Key_USBKBD_Down,			false	},	// ↓.
	{	GFWD_Key_USBKBD_PageDown,		false	},	// PageDown.
	{	GFWD_Key_USBKBD_Insert,			false	},	// insert.
	{	GFWD_Key_USBKBD_Del,			false	},	// Del.
	{	GFWD_Key_USBKBD_Pause,			false	},	// Pause.
	{	GFWD_Key_USBKBD_BckSlashUP,		false	},	// \(上).
	{	GFWD_Key_USBKBD_WinLeft,		true	},	// win(左).
	{	GFWD_Key_USBKBD_WinRight,		true	},	// win(右).
	{	GFWD_Key_USBKBD_PopRight,		false	},	// pop(右).
};


//-----------------------------------------------------------------------------.
// 外キー操作のQt対応.
//-----------------------------------------------------------------------------.
void QtPanel::keyPressEvent( GFWS_KeyEvent* e )
{
	//HardkeyとUSBキーボードのキー入力処理を分岐.
	/*------------------------------*/
	/*		 HardKey 押下時 		*/
	/*------------------------------*/
	if( true == isHardKey( e->si_Key ) ) {
		if( bLowPowerGamen ) {
			if( bPowerSave ) {
				bPowerSave = false;
//[Qt4.7->Qt5.3]KMカスタマイズ start Que形式RingBuf変更 CQ.512406------------------------.
			    //keyData = GFWD_Key_Hard_PowerSave;                         // パワーセーブキーを設定
//[Qt4.7->Qt5.3]KMカスタマイズ end   Que形式RingBuf変更 CQ.512406------------------------.
			}
		}
		else {
			bKeyEventState = true;
//[Qt4.7->Qt5.3]KMカスタマイズ start Que形式RingBuf変更 CQ.512406------------------------.
		    //keyData = e->si_Key;
//[Qt4.7->Qt5.3]KMカスタマイズ end   Que形式RingBuf変更 CQ.512406------------------------.
		}
	}
	/*------------------------------*/
	/*		USB キーボード押下時 	*/
	/*------------------------------*/
	else {
		// スキャンコード（Linux）を取得.
		uchar uc_RecieveKeyCode = ( uchar )e->sl_NativeScanCode;
		if( ( isUSBKeyBoardKeyRepertOn() == True )
		    && ( getUSBKeyBoardLastOnKeyCode()            == uc_RecieveKeyCode
		         || ( getUSBKeyBoardLastOnKeyCode() - 256 )  == uc_RecieveKeyCode
		         || ( getUSBKeyBoardLastOnKeyCode() - 512 )  == uc_RecieveKeyCode
		         || ( getUSBKeyBoardLastOnKeyCode() - 1024 ) == uc_RecieveKeyCode ) )
		{
			return;
		}

		// キーボード未接続状態、もしくは キーボード入力禁止状態時は、キー入力を受け付けない.
		if( ( TYPE_UsbKbdStateInfo_One != APIC_MachineSetting::newInstance()->getUsbStatus() ) ||	//キーボード未接続状態.
		    ( False == APIC_MachineSetting::newInstance()->getEnableUsbKbd() ) ) {				//キーボード入力禁止状態.
			return;
		}

		/*------------------------------------------------------------------------------------------------------------------*/
		/*	「BSP通知のキーデータ形式」の作成  (前回の「BSP通知のキーデータ形式」と今回通知の「Qt定義のキーコード」から) 	*/
		/*------------------------------------------------------------------------------------------------------------------*/
		uchar uc_WorkBuf[TYPD_PACKET_MAXSIZE];		//「BSP通知のキーデータ形式」作成用の作業バッファ.

		memcpy( uc_WorkBuf, uc_PrePacket, sizeof( uc_WorkBuf ) );		// 前回の「BSP通知のキーデータ形式」をコピー.

		for( uint i = 0; i < sizeof( st_isKeyCodeModifierTbl ) / sizeof( st_isKeyCodeModifier ); ++i ) {
			if( uc_RecieveKeyCode == st_isKeyCodeModifierTbl[i].uc_KeyCodeLinux ) {
				/*--------------------------------------------------*/
				/*	押下したキーがModifierKey(修飾キー)の場合.		*/
				/*--------------------------------------------------*/
				if( true == st_isKeyCodeModifierTbl[i].b_ModifierKeys ) {
					//ModifierKey情報に該当するビットフラグを立てる.
					uc_WorkBuf[0] |= uc_RecieveKeyCode;
				}
				/*--------------------------------------------------*/
				/*	押下したキーが通常キーの場合					*/
				/*--------------------------------------------------*/
				else {
					//通知された通常キーの格納位置(前回格納した位置+1)を探し格納する.
					for( int loopCnt = 0, SerchIndex = 1; loopCnt < TYPD_USB_KEYCOUNT_MAX; loopCnt++, SerchIndex++ ) {
						// 通常キー格納領域の終端を検知.
						if( 0xff == uc_WorkBuf[SerchIndex] ) {
							uc_WorkBuf[SerchIndex] = uc_RecieveKeyCode;						// 終端を通知されたキーで上書き.
							uc_WorkBuf[SerchIndex + 1] = 0xff;								// 終端を新たに追加.
							break;
						}
					}
				}
				break;
			}
		}

		memcpy( uc_PrePacket, uc_WorkBuf, sizeof( uc_PrePacket ) );		// 前回の「BSP通知のキーデータ形式」を更新.
		setPacketUSB( uc_WorkBuf );									// 作成した「BSP通知のキーデータ形式」をバッファへ格納.

	}
}

void QtPanel::setPowerSave( bool bLow )
{
	bPowerSave = bLow;
}

void QtPanel::setLowPowerGamen( bool bLow )
{
	bLowPowerGamen = bLow;
	if( !bLow ) { bPowerSave = false; }
	update();
}

//-----------------------------------------------------------------------------------------------------------------
//概要　　：QtPanelインスタンス取得.
//-------------------------------------------------------------------------------------------------------------------
QtPanel* QtPanel::newInstance()
{
	QtPanel*	p_qtpanel;
	p_qtpanel = PLBC_PanelLcdDraw::newInstance()->getQtPanelInstance();
	return( p_qtpanel );
}

/*================================================================================================
［機能］	キーコードをQT用コードに変換する。
--------------------------------------------------------------------------------------------------
［引数］	code	キーコード
［戻値］	無し
 [履歴]     2007/10/29 FSI土居		外キー開放イベント処理追加
			2009/08/28 FSI土居		長押し判定対応追加
================================================================================================*/
void QtPanel::keyReleaseEvent( GFWS_KeyEvent* e )
{
	//HardkeyとUSBキーボードのキー入力処理を分岐.
	/*------------------------------*/
	/*		 HardKey 開放時 		*/
	/*------------------------------*/
	if( true == isHardKey( e->si_Key ) ) {
		// 1回でも送信していない場合無効。
		if( bKeyEventState == True ) {
//[Qt4.7->Qt5.3]KMカスタマイズ start Que形式RingBuf変更 CQ.512406------------------------.
		    //keyData = DEF_KeyRelease;	// 開放.
//[Qt4.7->Qt5.3]KMカスタマイズ end   Que形式RingBuf変更 CQ.512406------------------------.
			bKeyEventState = false;
		}
	}

	/*------------------------------*/
	/*		USB キーボード 開放時 	*/
	/*------------------------------*/
	else {
		uchar uc_WorkBuf[TYPD_PACKET_MAXSIZE];					//「BSP通知のキーデータ形式」作成用の作業バッファ.

		memcpy( uc_WorkBuf, uc_PrePacket, sizeof( uc_WorkBuf ) );	// 前回の「BSP通知のキーデータ形式」をコピー.

		uchar uc_RecieveKeyCode = ( uchar )e->sl_NativeScanCode;

		for( uint i = 0; i < sizeof( st_isKeyCodeModifierTbl ) / sizeof( st_isKeyCodeModifier ); ++i ) {
			if( uc_RecieveKeyCode == st_isKeyCodeModifierTbl[i].uc_KeyCodeLinux ) {
				/*--------------------------------------------------*/
				/*	開放したキーがModifierKey(修飾キー)の場合.		*/
				/*--------------------------------------------------*/
				if( true == st_isKeyCodeModifierTbl[i].b_ModifierKeys ) {
					//ModifierKey情報の該当するビットフラグを落とす.
					uc_WorkBuf[0] = (uchar)(uc_WorkBuf[0] & ~uc_RecieveKeyCode);
				}
				/*--------------------------------------------------*/
				/*	開放したキーが通常キーの場合					*/
				/*--------------------------------------------------*/
				else {
					//通知された通常キーの格納位置を探し削除する.
					for( int loopCnt = 0, SerchIndex = 1; loopCnt < TYPD_USB_KEYCOUNT_MAX; loopCnt++, SerchIndex++ ) {
						// 開放対象のキー格納の位置を検知.
						if( uc_RecieveKeyCode == uc_WorkBuf[SerchIndex] ) {
							//開放対象のキーを削除し、後列のデータを詰める.
							for( ; loopCnt < TYPD_USB_KEYCOUNT_MAX; loopCnt++, SerchIndex++ ) {
								uc_WorkBuf[SerchIndex] = uc_WorkBuf[SerchIndex + 1];
							}
						}
					}
				}
				break;
			}
		}

		memcpy( uc_PrePacket, uc_WorkBuf, sizeof( uc_PrePacket ) );		// 前回の「BSP通知のキーデータ形式」を更新.
		setPacketUSB( uc_WorkBuf );									// 作成した「BSP通知のキーデータ形式」をバッファへ格納.

	}
}

//-----------------------------------------------------------------------------.
// 外キー操作対応.
//-----------------------------------------------------------------------------.
bool QtPanel::ConvKeyCodeWndToQt( uint32_t ul_in, uint32_t* ul_out )
{
	struct st_KeyCodeConvertTable {
		uint32_t ul_keyCodeWnd;	//「WindowManager通知のキーイベントコード」.
		uint32_t si_keyCodeQt;		//「Qt定義のキーコード」.
	} st_KeyConvTbl[ ] = {
		// 外キー.
		{LAPD_WND_NOTICE_10Key_0,			GFWD_Key_Hard_0			},
		{LAPD_WND_NOTICE_10Key_1,			GFWD_Key_Hard_1			},
		{LAPD_WND_NOTICE_10Key_2,			GFWD_Key_Hard_2			},
		{LAPD_WND_NOTICE_10Key_3,			GFWD_Key_Hard_3			},
		{LAPD_WND_NOTICE_10Key_4,			GFWD_Key_Hard_4			},
		{LAPD_WND_NOTICE_10Key_5,			GFWD_Key_Hard_5			},
		{LAPD_WND_NOTICE_10Key_6,			GFWD_Key_Hard_6			},
		{LAPD_WND_NOTICE_10Key_7,			GFWD_Key_Hard_7			},
		{LAPD_WND_NOTICE_10Key_8,			GFWD_Key_Hard_8			},
		{LAPD_WND_NOTICE_10Key_9,			GFWD_Key_Hard_9			},
		{LAPD_WND_NOTICE_ClearKey,			GFWD_Key_Hard_Clr		},
		{LAPD_WND_NOTICE_StopKey,			GFWD_Key_Hard_Stop		},
		{LAPD_WND_NOTICE_StartKey,			GFWD_Key_Hard_Start		},
		{LAPD_WND_NOTICE_IDKey,				GFWD_Key_Hard_IDInput	},
		{LAPD_WND_NOTICE_PowerSaveKey,		GFWD_Key_Hard_PowerSave	},
		{LAPD_WND_NOTICE_InterruptKey,		GFWD_Key_Hard_IntMode	},
		{LAPD_WND_NOTICE_ResetKey,			GFWD_Key_Hard_Reset		},
		{LAPD_WND_NOTICE_MenuKey,			GFWD_Key_Hard_Utility	},
		{LAPD_WND_NOTICE_ProgramKey,		GFWD_Key_Hard_Menu		},
		{LAPD_WND_NOTICE_CopyKey,			GFWD_Key_Hard_Customize1},
		{LAPD_WND_NOTICE_EnlargeDispKey,	GFWD_Key_Hard_Universal	},
		{LAPD_WND_NOTICE_10Key_Sharp,		GFWD_Key_Hard_Sharp		},
		{LAPD_WND_NOTICE_10Key_Asterisk,	GFWD_Key_Hard_Asterisk	},
		{LAPD_WND_NOTICE_FaxScanKey,		GFWD_Key_Hard_Customize2},
		{LAPD_WND_NOTICE_BoxKey,			GFWD_Key_Hard_Customize3},
		{LAPD_WND_NOTICE_HelpKey,			GFWD_Key_Hard_Help		},
		{LAPD_WND_NOTICE_ConfirmCopyKey,	GFWD_Key_Hard_SampleCopy},
		{LAPD_WND_NOTICE_VoiceGuide,		GFWD_Key_Hard_VoiceGuide},
		{LAPD_WND_NOTICE_Customize4,		GFWD_Key_Hard_Customize4},
	};

	bool result = false;
	for( uint i = 0; i < sizeof( st_KeyConvTbl )/sizeof( st_KeyConvTbl[0] ); i++ ) {
		if( st_KeyConvTbl[i].ul_keyCodeWnd == ul_in ) {
			*ul_out = st_KeyConvTbl[i].si_keyCodeQt;
			result = true;
			break;
		}
	}
	return result;
}

void QtPanel::wnd_KeyPressNtf( uint32_t event )				// WNDMGRからのキー押下イベント通知.
{
	uint32_t e;
	if( ConvKeyCodeWndToQt( event, &e ) ) {
		keyPress( e );
	} else {
		printf( " unknown keycode = %08x \n", event );
	}
	return;
}

void QtPanel::wnd_KeyReleaseNtf( uint32_t event )				// WNDMGRからのキー解放イベント通知.
{
	uint32_t e;
	if( ConvKeyCodeWndToQt( event, &e ) ) {
		keyRelease( e );
	} else {
		printf( " unknown keycode = %08x \n", event );
	}
	return;
}

/*================================================================================================
［機能］	キー押下イベント.
--------------------------------------------------------------------------------------------------
［引数］	キーコード
［戻値］	無し
================================================================================================*/
void QtPanel::keyPress( uint32_t e )
{
	//HardkeyとUSBキーボードのキー入力処理を分岐.
	/*------------------------------*/
	/*		 HardKey 押下時 		*/
	/*------------------------------*/
	if( true == isHardKey( e ) ) {
		if( bLowPowerGamen ) {
			if( bPowerSave ) {
				bPowerSave = false;
//[Qt4.7->Qt5.3]KMカスタマイズ start Que形式RingBuf変更 CQ.512406------------------------.
    		    keyData[keyDataRear] = GFWD_Key_Hard_PowerSave;	// パワーセーブキーを設定.
    		    keyDataRear++;
    		    keyDataRear = keyDataRear > (TYPD_PKEY_QMAX - 1) ? 0 : keyDataRear;
//[Qt4.7->Qt5.3]KMカスタマイズ end   Que形式RingBuf変更 CQ.512406------------------------.
			}
		} else {
			bKeyEventState = true;
//[Qt4.7->Qt5.3]KMカスタマイズ start Que形式RingBuf変更 CQ.512406------------------------.
    	    keyData[keyDataRear] = e;
    	    keyDataRear++;
    	    keyDataRear = keyDataRear > (TYPD_PKEY_QMAX - 1) ? 0 : keyDataRear;
//[Qt4.7->Qt5.3]KMカスタマイズ end   Que形式RingBuf変更 CQ.512406------------------------.
		}
	}
	/*------------------------------*/
	/*		USB キーボード押下時 	*/
	/*------------------------------*/
	else {
		return;
	}
}

/*================================================================================================
［機能］	キー解放イベント.
--------------------------------------------------------------------------------------------------
［引数］	キーコード
［戻値］	無し
================================================================================================*/
void QtPanel::keyRelease( uint32_t e )
{
	//HardkeyとUSBキーボードのキー入力処理を分岐.
	/*------------------------------*/
	/*		 HardKey 開放時 		*/
	/*------------------------------*/
	if( true == isHardKey( e ) ) {
		// 1回でも送信していない場合無効。
		if( bKeyEventState == True ) {
//[Qt4.7->Qt5.3]KMカスタマイズ start Que形式RingBuf変更 CQ.512406------------------------.
    	    keyData[keyDataRear] = DEF_KeyRelease;	// 開放.
    	    keyDataRear++;
    	    keyDataRear = keyDataRear > (TYPD_PKEY_QMAX - 1) ? 0 : keyDataRear;
//[Qt4.7->Qt5.3]KMカスタマイズ end   Que形式RingBuf変更 CQ.512406------------------------.
			bKeyEventState = false;
		}
	}
	/*------------------------------*/
	/*		USB キーボード 開放時 	*/
	/*------------------------------*/
	else {
		return;
	}
}

/*---------------------------------------------------------------------------------------------------------------
［機能	TouchPanelRepeat処理
		25ms周期で関数が起動する為、以下のキーリピート間隔でパネル押下状態をパネルメインに送信する。
		キーリピート開始間隔はデフォルト750ms
		キーリピート間隔はデフォルト100ms
--------------------------------------------------------------------------------------------------
［引数］無し
［戻値］無し
 [履歴]	09/08/26 FSI土居 2度押し対応
 		13/06/20 ADS長谷川 ARMだとLITTLEエンディアンになる関係で本関数内でXとYが逆転してしまう。
 				 しかし、関連する別関数(mouseMoveEvent)では構造が異なる逆転しない。
 				 その結果、異なる座標のボタンが押されたり、反応が鈍くなる問題が発生した。
 				 根本原因は、新SIMと実機でXとYを逆転させていた経緯があり、
 				 これをもとから正す(X上位16bit、Y下位16bit)方法で修正する。
================================================================================================*/
void QtPanel::mouseSendData()
{
	ushort keyRepeatStart;
	ushort keyRepeatInterval;
	if( onlyPanel.bl_isEnlargeDisplayMode == True )
	{
		// 拡大表示選択中は、キーリピート開始間隔チョイスに応じてウェイト間隔を決める
		keyRepeatStart 	  = ( ushort )( PnlComGetOperationSetting()->getKeyRepeatStartTimeSec() * 40 );		// キーリピート開始間隔・スタート用
		keyRepeatInterval = ( short )( PnlComGetOperationSetting()->getKeyRepeatIntervalTimeSec() * 40 );	// キーリピート開始間隔・間隔時間用
	}
	else
	{
		// 拡大表示選択以外の時は、キーリピート開始間隔・・スタート時間は、７５０ｍｓｅｃとする
		keyRepeatStart = DEF_MouseSendStartTime;		// キーリピート開始間隔・スタート用（30 :750ms）
		keyRepeatInterval = DEF_MouseSendIntervalTime;		// キーリピート開始間隔・間隔時間用（ 4 :100ms）
	}
	DEBUG_PANEL_PRINT( "::: QtPanel::mouseSendData keyRepeatStart[%d] keyRepeatInterval[%d]::: \n",keyRepeatStart,keyRepeatInterval );
	// 前回押下状態ではない。
	if( mouseTimeState == 0 ) {
		if( sendMouseData.data == 0 && sendMouseData.data != m_mouseData.data ) {
			++mouseTimeState;
			sendMouseData.data = m_mouseData.data;
			DEBUG_PANEL_PRINT( "::: QtPanel::mouseSendData panel on : x[%d] y[%d] :::\n",sendMouseData.pos.x,sendMouseData.pos.y );
//@@ ite2コードを暫定的に有効.
			PMGC_PanelManager::newInstance()->TouchPanelKeyNotice( sendMouseData.data );
			bMouseEventState = true;	// タッチパネル押下フラグON.
		}
	}
	else {
		// 解放処理
		if( m_mouseData.data == 0 ) {
			mouseTimeState = 0;
			sendMouseData.data = m_mouseData.data;
			DEBUG_PANEL_PRINT( "::: QtPanel::mouseSendData panel off :::\n" );
//@@ ite2コードを暫定的に有効.
			PMGC_PanelManager::newInstance()->TouchPanelKeyNotice( sendMouseData.data );
			bMouseEventState = false;	// タッチパネル押下フラグOFF.
		}
		else {
			// 押し続け処理
			// オーバーフロー回避
			if( mouseTimeState >= 20000 ) {
				mouseTimeState -= 10000;
			}
			// 押し続け開始時のキー取得データが（Ｘ：±１０以内、Ｙ：±１５以内）の位置で押下されているとき
			if( ( ( ( sendMouseData.pos.x-10 ) <= m_mouseData.pos.x ) && ( ( sendMouseData.pos.x+10 ) >= m_mouseData.pos.x ) )
			    && ( ( ( sendMouseData.pos.y-15 ) <= m_mouseData.pos.y ) && ( ( sendMouseData.pos.y+15 ) >= m_mouseData.pos.y ) ) ) {
				// キーリピート処理開始
				if( mouseTimeState >= keyRepeatStart ) {
					if( ( mouseTimeState - keyRepeatStart ) % keyRepeatInterval ==  0 ) {
						DEBUG_PANEL_PRINT( "::: QtPanel::mouseSendData panel hold :::\n" );
//@@ ite2コードを暫定的に有効.
						PMGC_PanelManager::newInstance()->TouchPanelKeyNotice( sendMouseData.data );
					}
				}
				++mouseTimeState;
			}
			else {
				++mouseTimeState;
			}
		}
	}
}
//-----------------------------------------------------------------------------------------------------------------
//概要　　：DirectKeyRepeat処理
//			25ms周期で関数が起動する為、最後に押された外キー情報をパネルメインに送信する。
//引数　　：無し
//戻り値　：無し
//-------------------------------------------------------------------------------------------------------------------
void QtPanel::keySendData()
{
	static short stillOnCountDirectKey = 0;				// ダイレクトキー押し続け時のカウント用変数.
	static uchar uc_holdDirectKeyGetData = 0xff;		// 長押し用のキーデータ.
	static Bool isKeyHold = False;						// 長押し検出判定.

//[Qt4.7->Qt5.3]KMカスタマイズ start Que形式RingBuf変更 CQ.512406------------------------.
    int32_t tmpkeyData = DirectKeyData;
//[Qt4.7->Qt5.3]KMカスタマイズ end   Que形式RingBuf変更 CQ.512406------------------------.
    
	// ダイレクトキー情報
	static struct holdDirectKeyInfo {
		int32_t	scanData;		// スキャンデータ.
		int32_t	holdData;		// 長押し時データ.
		short	holdCount;		// 長押し判定カウント数.
	} holdkeyInfo[] = {
		{ GFWD_Key_Hard_VoiceGuide, 	GFWD_Key_Hard_Help, 	80 },	// ヘルプ. 80 x 25ms = 2秒.
	};

//[Qt4.7->Qt5.3]KMカスタマイズ start Que形式RingBuf変更 CQ.512406------------------------.
    if(keyDataFront != keyDataRear){
        tmpkeyData =  keyData[keyDataFront];
        keyDataFront++;
        keyDataFront = keyDataFront > (TYPD_PKEY_QMAX - 1) ? 0 : keyDataFront;
    }

	if( stillOnCountDirectKey == 0 ) {
		if(DirectKeyData != tmpkeyData) {
			DirectKeyData = tmpkeyData;
			uchar cnt;
			for( cnt = 0 ; cnt < sizeof( holdkeyInfo )/sizeof( holdkeyInfo[0] ); cnt++ ) {
				if( holdkeyInfo[cnt].scanData == DirectKeyData ) {
					isKeyHold = True;
					stillOnCountDirectKey++;
					uc_holdDirectKeyGetData = cnt;
					break;
				}
			}
			if( isKeyHold == False ) {
				PMGC_PanelManager::newInstance()->DirectKeyNotice( DirectKeyData );
			}
		}
	}
	else {
		// 前回押下状態
		if( isKeyHold == True )
		{ stillOnCountDirectKey++; }
		if( stillOnCountDirectKey >= holdkeyInfo[uc_holdDirectKeyGetData].holdCount && isKeyHold == True ) {
			PMGC_PanelManager::newInstance()->DirectKeyNotice( holdkeyInfo[uc_holdDirectKeyGetData].holdData );
			isKeyHold = False;
// comment 2017/03/23 【HCL_Prem】【IT5_4.0】【AR_1010106】Resetting the value of Key Long press counter.
			stillOnCountDirectKey = 0;
		}
		else if(DirectKeyData != tmpkeyData){
			PMGC_PanelManager::newInstance()->DirectKeyNotice( holdkeyInfo[uc_holdDirectKeyGetData].scanData );
			isKeyHold = False;
			stillOnCountDirectKey = 0;
// comment 2017/03/23 【HCL_Prem】【IT5_4.0】【AR_1010106】Sending Hard key release event to message queue after pressing voice guide key.
			DirectKeyData = tmpkeyData;
			PMGC_PanelManager::newInstance()->DirectKeyNotice( DirectKeyData );
		}
	}
//[Qt4.7->Qt5.3]KMカスタマイズ end   Que形式RingBuf変更 CQ.512406------------------------.
}
//-----------------------------------------------------------------------------------------------------------------
//概要　　：定期起床処理25ms周期で起動する。
//
//-------------------------------------------------------------------------------------------------------------------
void QtPanel::cyclicLoop()
{
	// USBキーボード キー送信.
	usbKbdKeySendData();

	// 外キー位置送信
	keySendData();

	// タッチパネル位置送信
	mouseSendData();
}

//-----------------------------------------------------------------------------------------------------------------
//概要　　：reset用qwsEvent処理.
//履歴　　：091125 スクリーンセーバ処理追加.
//-------------------------------------------------------------------------------------------------------------------
#if !defined(SIMULATOR) || defined(LINUX)
void QtPanel::recv_qwsProcessEvent()
{
	if( !b_EventSend ) {  // イベント送信要時、PanelManagerへイベント送信.
		// 送信先で値を使用していないため0を設定する.
		PMGC_PanelManager::newInstance()->QwsEventNotice( int32_t( 0 ) );
		b_EventSend = True;

		//無操作タイマー設定(ワンショットタイマ).
		LAPC_BFW_System* p_timer = LAPC_BFW_System::newInstance();
		p_timer->setTimerEvent( DEF_QtPanelEventSendTime, m_pc_noopehandler, &m_noopetimer_id );
	}
}
#endif	// !defined(SIMULATOR) || defined(LINUX).

//-----------------------------------------------------------------------------------------------------------------
//概要　　：qwsProcessEventタイマー通知取得.
//-------------------------------------------------------------------------------------------------------------------
void QtPanel::recv_qwsProcessEventTimer()
{
	b_EventSend = False;
}

//-----------------------------------------------------------------------------------------------------------------.
//概要　　：USBキーボード対応関連の初期化.
//
//-----------------------------------------------------------------------------------------------------------------.
void QtPanel::initUsbKbd()
{
	memset( uc_cmpPacket, 0x00, sizeof( uc_cmpPacket ) );

	writePointer = 0;
	readPointer  = 0;
	memset( Packet, 0x00, sizeof( Packet ) );
	memset( USBKeyGetData, 0x00, sizeof( USBKeyGetData ) );
	b_USBKeyBoardKeyRepertOn = false;
	us_USBKeyBoardLastOnKeyCode = 0;

	// キー開放状態で初期化.
	memset( uc_PrePacket, 0xff, sizeof( uc_PrePacket ) );				// 一旦0xff(終端)で初期化.
	memset( uc_PrePacketUSBKey, 0xff, sizeof( uc_PrePacketUSBKey ) );
	uc_PrePacket[0] = 0;											// 修飾キー情報は0(未押下)で設定.
	uc_PrePacketUSBKey[0] = 0;

}

//-----------------------------------------------------------------------------------------------------------------.
//概要　　：USBキーボード対応関連の初期化.
//
//-----------------------------------------------------------------------------------------------------------------.
bool QtPanel::isHardKey( int si_keyCode )
{
	bool b_ret = false;			// 判定結果を「ハードキー以外」で初期化.

	const static int keyInfo[] =
	{
		GFWD_Key_Hard_0,
		GFWD_Key_Hard_1,
		GFWD_Key_Hard_2,
		GFWD_Key_Hard_3,
		GFWD_Key_Hard_4,
		GFWD_Key_Hard_5,
		GFWD_Key_Hard_6,
		GFWD_Key_Hard_7,
		GFWD_Key_Hard_8,
		GFWD_Key_Hard_9,
		GFWD_Key_Hard_Clr,
		GFWD_Key_Hard_Stop,
		GFWD_Key_Hard_Start,
		GFWD_Key_Hard_IDInput,
		GFWD_Key_Hard_PowerSave,
		GFWD_Key_Hard_IntMode,
		GFWD_Key_Hard_Reset,
		GFWD_Key_Hard_Utility,
		GFWD_Key_Hard_Program,
		GFWD_Key_Hard_Copy,
		GFWD_Key_Hard_Enlarge,
		GFWD_Key_Hard_Sharp,
		GFWD_Key_Hard_Asterisk,
		GFWD_Key_Hard_JobMemory,
		GFWD_Key_Hard_Fax,
		GFWD_Key_Hard_Box,
		GFWD_Key_Hard_Help,
		GFWD_Key_Hard_Accessibility,
		GFWD_Key_Hard_SampleCopy,
		GFWD_Key_Hard_Release,
		GFWD_Key_Hard_Menu,
		GFWD_Key_Hard_Universal,
		GFWD_Key_Hard_Customize1,
		GFWD_Key_Hard_Customize2,
		GFWD_Key_Hard_Customize3,
		GFWD_Key_Hard_VoiceGuide,
		GFWD_Key_Hard_Customize4,
	};
	// ハードキーテーブルから検索.
	for( uint i = 0; i < sizeof( keyInfo )/sizeof( keyInfo[0] ); i++ ) {
		// ハードキーテーブルから検出した場合.
		if( keyInfo[i] == si_keyCode ) {
			b_ret = true;
			break;
		}
	}

	return b_ret;
}

//-----------------------------------------------------------------------------------------------------------------.
//概要　　：パケットデータへのデータセット.(USB用).
//
//-----------------------------------------------------------------------------------------------------------------.
uchar QtPanel::setPacketUSB( uchar* setData )
{
	uchar data_exist = 0;
	if( memcmp( Packet[writePointer], uc_cmpPacket, TYPD_PACKET_MAXSIZE ) == 0 ) {
		data_exist = 1;
		memcpy( Packet[writePointer], setData, TYPD_PACKET_MAXSIZE );		// パケットデータへデータ書込みを行う.
		memset( setData,0x00,TYPD_PACKET_MAXSIZE );							// 書き込み元データ全クリア.
		if( ++writePointer >= TYPD_PACKETNUM ) { writePointer = 0; }
	} else {
		DEBUG_PANEL_PRINT( "QtPanel::setPacketUSB() : Pre_DataExist!!\n" );
	}
	return data_exist;
}

//-----------------------------------------------------------------------------------------------------------------.
//概要　　：1パケット取得.(USB用)
//
//-----------------------------------------------------------------------------------------------------------------.
uchar QtPanel::getPacketUSB( uchar* getPacket )
{
	uchar data_exist = 0;
	if( memcmp( Packet[readPointer], uc_cmpPacket, TYPD_PACKET_MAXSIZE ) != 0 ) {
		data_exist = 1;
		memcpy( getPacket, Packet[readPointer], TYPD_PACKET_MAXSIZE );
		memset( Packet[readPointer],0x00,TYPD_PACKET_MAXSIZE );		// 読み込み元データ全クリア.
		if( ++readPointer >= TYPD_PACKETNUM ) { readPointer = 0; }
	}
	return data_exist;
}

//-----------------------------------------------------------------------------------------------------------------.
//概要　　：USBキーボード キーデータ処理関数.
//
//-----------------------------------------------------------------------------------------------------------------.
void QtPanel::usbKbdKeySendData( void )
{

	ushort* p_USBKeyGetData;				// USBKeyGetDataのアドレス.
	register ushort keyRepeatStart = 0;		// キーリピート開始間隔・スタート用.
	register short keyRepeatInterval = 0;	// キーリピート間隔調整用.
	/*----------------------------------------------*/
	/*	 キーリピートの開始、間隔時間の設定			*/
	/*----------------------------------------------*/
	if( onlyPanel.bl_isEnlargeDisplayMode == True ) {
		// 拡大表示選択中は、キーリピート開始間隔チョイスに応じてウェイト間隔を決める .
		keyRepeatStart 	  = ( ushort )( PnlComGetOperationSetting()->getKeyRepeatStartTimeSec() * 40 );		// キーリピート開始間隔・スタート用 .
		keyRepeatInterval = ( ushort )( PnlComGetOperationSetting()->getKeyRepeatIntervalTimeSec() * 40 );	// キーリピート開始間隔・間隔時間用 .
	} else {
		// 拡大表示選択以外の時は、キーリピート開始間隔・・スタート時間は、７５０ｍｓｅｃとする .
		keyRepeatStart 		= DEF_UsbKbdKeySendStartTime;			// キーリピート開始間隔・スタート用（30 :７５０ｍｓｅｃ）.
		keyRepeatInterval 	= DEF_UsbKbdKeySendIntervalTime;		// キーリピート開始間隔・間隔時間用（ 4 :１００ｍｓｅｃ）.
	}

	/*----------------------------------------------*/
	/*		受信データの解析						*/
	/*----------------------------------------------*/
	USBKeycommandAnalyze();											// パケット解釈を行う.


	/*----------------------------------------------*/
	/*		受信キーコードをPanelManagerへ通知		*/
	/*----------------------------------------------*/
	p_USBKeyGetData = getUSBKeyInfo();								// USBキー押下時キーコード格納領域の先頭アドレス取得.
	if( 0 != *p_USBKeyGetData )										// キーコードが在る場合.
	{
		for( int i = 0; i < TYPD_QtUSBKeyCodeMax; ++i )
		{
			if( 0 == *( p_USBKeyGetData+i ) )							// 最後まで読み込んだ場合.
			{
				USBKeyInfoClear();									// USBキーコード格納領域をクリアする.
				break;
			}

			// メッセージ「パネルUSBキーボードキー受付通知」
			PMGC_PanelManager::newInstance()->USBKeyNotice( *( p_USBKeyGetData+i ), False );
		}
	}

	static short stillOnCountUSBKey = -1;			// USBキーボードキーリピート時のカウント用変数(インクリメントしてから開始する為 -1 で初期化).
	static bool b_USBKeyRepertStart = false;		// キーリピート開始フラグ.

	/*----------------------------------------------*/
	/*		キーリピートをPanelManagerへ通知		*/
	/*----------------------------------------------*/
	if( true == isUSBKeyBoardKeyRepertOn() )	{	// キーリピートON状態.
		++stillOnCountUSBKey;						// ２５ｍｓｅｃ周期で１カウント.

		if( keyRepeatStart <= stillOnCountUSBKey && false == b_USBKeyRepertStart ) {			// キーリピート開始時間に達した.
			// メッセージ「パネルUSBキーボードキー受付通知」.
			PMGC_PanelManager::newInstance()->USBKeyNotice( getUSBKeyBoardLastOnKeyCode(), True );

			b_USBKeyRepertStart = true;			// キーリピート開始.
			stillOnCountUSBKey = 0;				// リピートタイミングカウンタを初期化(すでに開始している為 0 で初期化).
		}
		if( keyRepeatInterval <= stillOnCountUSBKey && true == b_USBKeyRepertStart ) {	// リピート中の間隔時間に達した.
			// メッセージ「パネルUSBキーボードキー受付通知」.
			PMGC_PanelManager::newInstance()->USBKeyNotice( getUSBKeyBoardLastOnKeyCode(), True );

			stillOnCountUSBKey = 0;		// リピートタイミングカウンタを初期化(すでに開始している為 0 で初期化).
		}
	} else {	// キーリピートOFF状態.
		stillOnCountUSBKey = -1;								// リピートタイミングカウンタを初期化(インクリメントしてから開始する為 -1 で初期化).
		b_USBKeyRepertStart = false;							// キーリピート終了.
	}

}


//-----------------------------------------------------------------------------------------------------------------.
//概要　　：USBキーボード押下時キーコード取得.
//
//-----------------------------------------------------------------------------------------------------------------.
ushort* QtPanel::getUSBKeyInfo( void )
{
	return USBKeyGetData;

}



//-----------------------------------------------------------------------------------------------------------------.
//概要　　：USBキーボード押下時キーコード取得.
//
//-----------------------------------------------------------------------------------------------------------------.
void QtPanel::USBKeyInfoClear( void )
{
	memset( USBKeyGetData, 0, sizeof( USBKeyGetData ) );

}



//-----------------------------------------------------------------------------------------------------------------.
//概要　　：USBキーボードキーリピートモードか否かを取得する.
//
//-----------------------------------------------------------------------------------------------------------------.
bool  QtPanel::isUSBKeyBoardKeyRepertOn( void )
{
	return b_USBKeyBoardKeyRepertOn;

}


//-----------------------------------------------------------------------------------------------------------------.
//概要　　：USBキーボードキーリピートモードをON_OFF定する.
//
//-----------------------------------------------------------------------------------------------------------------.
void  QtPanel::setUSBKeyBoardKeyRepertOn( bool b_Set )
{
	b_USBKeyBoardKeyRepertOn = b_Set;

}



//-----------------------------------------------------------------------------------------------------------------.
//概要　　：前回押下したキーコードを取得する.
//
//-----------------------------------------------------------------------------------------------------------------.
ushort  QtPanel::getUSBKeyBoardLastOnKeyCode( void )
{
	return us_USBKeyBoardLastOnKeyCode;

}



//-----------------------------------------------------------------------------------------------------------------.
//概要　　：前回押下したキーコードを設定する.
//
//-----------------------------------------------------------------------------------------------------------------.
void  QtPanel::setUSBKeyBoardLastOnKeyCode( ushort us_Set )
{
	us_USBKeyBoardLastOnKeyCode = us_Set;

}


//-----------------------------------------------------------------------------------------------------------------.
//概要　　：1パケットずつデータを取得し、キーコードを求める.
//
//-----------------------------------------------------------------------------------------------------------------.
void  QtPanel::USBKeycommandAnalyze( void )
{
	uchar uc_TmpPacketUSBKey[TYPD_PACKET_MAXSIZE + 1 ];
	bool bl_PreDataSame = false;
	uchar sc_Count;
	uchar sc_PreDataCount;
	uchar sc_USBKeyGetCtr = 0;
	bool bl_EffectivePacketData = false;
	bool bl_GetKeyCode = false;

	memset( uc_TmpPacketUSBKey, 0, sizeof( uc_TmpPacketUSBKey ) );

	// 1パケットずつテンポラリ領域に受け取る.
	// 読込み可能でない（有効データが無い）場合は何もせずに抜ける.
	while( getPacketUSB( uc_TmpPacketUSBKey ) != 0 )
	{
#ifdef SIMULATOR
		char	c_DebufBuff[512];
		memset( c_DebufBuff,0x00,sizeof( c_DebufBuff ) );
		sprintf( ( char* )c_DebufBuff,"[USBKeycommandAnalyze]パケット受信 %02x %02x %02x %02x %02x %02x %02x %02x\n",
		         uc_TmpPacketUSBKey[0],
		         uc_TmpPacketUSBKey[1],
		         uc_TmpPacketUSBKey[2],
		         uc_TmpPacketUSBKey[3],
		         uc_TmpPacketUSBKey[4],
		         uc_TmpPacketUSBKey[5],
		         uc_TmpPacketUSBKey[6],
		         uc_TmpPacketUSBKey[7] );
		OutputDebugString( ( const char* )c_DebufBuff );
#endif

		// Altキーがはなされた場合(ウムラウト対応).
		if( ( ( 0x38 == ( 0xff & uc_PrePacketUSBKey[0] ) ) && ( 0x38 != ( 0xff & uc_TmpPacketUSBKey[0] ) ) )
		    || ( ( 0x64 == ( 0xff & uc_PrePacketUSBKey[0] ) ) && ( 0x64 != ( 0xff & uc_TmpPacketUSBKey[0] ) ) ) )
		{
			USBKeyGetData[sc_USBKeyGetCtr] = 0xffff;// Altはなしたコード.
			sc_USBKeyGetCtr++;
		}
		sc_Count = 1;// 要素[0]はヘッダーの為とばす.
		bl_GetKeyCode = false;

		// データを一つずつ見ていく.
		while( 255 != uc_TmpPacketUSBKey[sc_Count] )
		{
			sc_PreDataCount = 1; // 要素[0]はヘッダーの為とばす.
			// 長押しキー判定.
			while( 255 != uc_PrePacketUSBKey[sc_PreDataCount] )
			{
				if( uc_TmpPacketUSBKey[sc_Count] == uc_PrePacketUSBKey[sc_PreDataCount] ) {
					bl_PreDataSame = true;
					break;
				}
				sc_PreDataCount++;
			}
			if( true == bl_PreDataSame ) {	// 長押しキーならはじく.
				bl_PreDataSame = false;
			} else {
				// 同時押し対応.
				switch( 0xff & uc_TmpPacketUSBKey[0] ) {	// ヘッダーを参照する.
					case 0:// 同時押しなし.
					case 0x1d:	// LeftCtrl.
					case 0x61:	// RightCtrl.
					case 0x7d:	// LeftWindows.
					case 0x7e:	// RightWindows.
						USBKeyGetData[sc_USBKeyGetCtr] = uc_TmpPacketUSBKey[sc_Count];
						sc_USBKeyGetCtr++;
						bl_GetKeyCode = true;
						break;
					case 0x2a:	// LeftShift.
					case 0x36:	// RightShift.
					case 0x3e:	// Right + Left Shift（0x2aと0x36の論理和）.
						USBKeyGetData[sc_USBKeyGetCtr] = (ushort)(uc_TmpPacketUSBKey[sc_Count] + 256);
						sc_USBKeyGetCtr++;
						bl_GetKeyCode = true;
						break;
					case 0x38:	// LeftAlt.
					case 0x64:	// RightAlt.
						USBKeyGetData[sc_USBKeyGetCtr] = (ushort)(uc_TmpPacketUSBKey[sc_Count] + 512);
						sc_USBKeyGetCtr++;
						bl_GetKeyCode = true;
						break;
					case 0x6e:	// RightAlt + LeftShift（0x64と0x2aの論理和）.
					case 0x76:	// RightAlt + RightShift（0x64と0x36の論理和）.
						USBKeyGetData[sc_USBKeyGetCtr] = (ushort)(uc_TmpPacketUSBKey[sc_Count] + 1024);
						sc_USBKeyGetCtr++;
						bl_GetKeyCode = true;
					default: break;
				}
			}
			sc_Count++;
		}
		// パケットが「キーを離した通知パケット」の場合で且つキーリピート中の場合.
		// 前回最後に押下しっぱなしのキーコードか離されたか確認.
		if( false == bl_GetKeyCode && true == isUSBKeyBoardKeyRepertOn() ) {
			ushort us_LastOnKeyCode = 0;
			us_LastOnKeyCode = getUSBKeyBoardLastOnKeyCode();
			uchar i;
			for( i = 0; TYPD_PACKET_MAXSIZE >= i ; i++ ) {
				if( ( uc_TmpPacketUSBKey[i] == us_LastOnKeyCode ) || ( uc_TmpPacketUSBKey[i] == us_LastOnKeyCode - 256 )
				    || ( uc_TmpPacketUSBKey[i] == us_LastOnKeyCode - 512 ) || ( uc_TmpPacketUSBKey[i] == us_LastOnKeyCode - 1024 ) ) {
					break;
				}
			}
			if( TYPD_PACKET_MAXSIZE < i ) {	// 無い場合はキーリピート解除.
				setUSBKeyBoardKeyRepertOn( false );
			}
		}
		// 今回のPacketを記憶する.
		memcpy( uc_PrePacketUSBKey, uc_TmpPacketUSBKey, sizeof( uc_TmpPacketUSBKey ) );
		bl_EffectivePacketData = true;	// 有効データを検知した.
	}
	// 有効データを検知しなかった（キー押下状態に変更が無い）場合はキーリピート判定.
	if( false == bl_EffectivePacketData ) {

		Bool b_KeyRepertOn = false;

		for( sc_Count = 1; TYPD_PACKET_MAXSIZE >= sc_Count; sc_Count++ ) {

			if( 0xff == uc_PrePacketUSBKey[1] ) {
				break;	// キー未押下状態の場合はキーリピートさせない.
			}

			if( ( 0xff == uc_PrePacketUSBKey[sc_Count] ) && ( 2 <= sc_Count ) ) {
				--sc_Count;	// 最後に押されたキーコードを取得.

				// Enterキーでキーボード入力を終了した場合、パケット（0x00 0xff ......）が飛んでこない為.
				// Enterキーに限り、ここでキーリピート処理をはじく.
				if( ( 0x1c == uc_PrePacketUSBKey[sc_Count] )		// Enterキーコード.
				    || ( 0x60 == uc_PrePacketUSBKey[sc_Count] ) ) {	// Fn + Enter キーコード.
					break;
				}

				switch( 0xff & uc_PrePacketUSBKey[0] ) {	// ヘッダーを参照する.
					case 0:// 同時押しなし.
					case 0x1d:	// LeftCtrl.
					case 0x61:	// RightCtrl.
					case 0x7d:	// LeftWindows.
					case 0x7e:	// RightWindows.
						setUSBKeyBoardLastOnKeyCode( uc_PrePacketUSBKey[sc_Count] );
						break;
					case 0x2a:	// LeftShift.
					case 0x36:	// RightShift.
						setUSBKeyBoardLastOnKeyCode( (ushort)(uc_PrePacketUSBKey[sc_Count] + 256) );
						break;
					case 0x38:	// LeftAlt.
					case 0x64:	// RightAlt.
						setUSBKeyBoardLastOnKeyCode( (ushort)(uc_PrePacketUSBKey[sc_Count] + 512) );
						break;
					case 0x6e:	// RightAlt + LeftShift（0x64と0x2aの論理和）.
					case 0x76:	// RightAlt + RightShift（0x64と0x36の論理和）.
						setUSBKeyBoardLastOnKeyCode( (ushort)(uc_PrePacketUSBKey[sc_Count] + 1024) );
						break;
					default:
						// 上記以外の修飾キーの組み合わせ(Ctrl+Alt、Ctrl+Shift、Shift+Alt等).
						// この組み合わせは通常、無反応(キー押下しても反応なし)だが、.
						// LastOnKeyCodeは更新しておかないと今回対応のリピート判定ができなくなる.
						setUSBKeyBoardLastOnKeyCode( uc_PrePacketUSBKey[sc_Count] );
						break;
				}
				b_KeyRepertOn = true;	// キーリピートON.
				break;
			}
		}
		setUSBKeyBoardKeyRepertOn( b_KeyRepertOn );
	}

}



//-----------------------------------------------------------------------------------------------------------------
//概要　　：QtPanelをアクティブにする(signal).
//-----------------------------------------------------------------------------------------------------------------
void  QtPanel::activateQtPanel( void )
{
	raise();
	activateWindow();
// comment 2020/09/17 Sky東島 [IT6_2.0MR3][OP_BTS-27689].
#if ( !defined(DEF_SparrowAIOSFPCus) && !defined(DEF_DenebMLKCus) && !(defined (DEF_HeliosMLKCus) && defined (DEF_LCD_7INCH) ) )
	GFWE_LCD_PANE e_LcdPane = GFWC_System::getInstance()->getLcdPane();
	if ( GFWE_LCD_PANE_3 == e_LcdPane ) {
		PNLC_SH3PaneMenuControlLib::newInstance()->show();
	} else if( GFWE_LCD_PANE_2 == e_LcdPane ){
		PNLC_SH3PaneMenuControlLib::newInstance()->show2Pane();
	}
#endif
}

//-----------------------------------------------------------------------------.
// ［機能］	KmApplication コンストラクタ.
//-----------------------------------------------------------------------------.
KmApplication::KmApplication( int& argc, char** argv )
	: QApplication( argc, argv )
{
	setStyle(QStyleFactory::create("cleanlooks")); // スタイル設定.
// 2015/09/08 Sky松尾【CQ516119】Start.
	installEventFilter( this );
// 2015/09/08 Sky松尾【CQ516119】End.
}

//-----------------------------------------------------------------------------------------------------------------
//［機能］	接続タッチパネルタイプ取得処理.
//-----------------------------------------------------------------------------------------------------------------
void KmApplication::LoadPanelType()
{
// Comment 2019/11/27 [HCLT_Selvaraj] [IT6_EagleZplus] [OP_BTS-11276] Address Sanitizer correction for EagleZplus Models.
	uint64_t us_PanelType = PCGE_PanelType_SResistance;
#if !defined(DEF_MinervaPreEmu)
	if( -1 >= sysBspIoctlPanelSio( &us_PanelType, EXDB_PanelType_Get ) ) {		// 接続タッチパネルタイプを取得.
		// 取得NGの場合.
		us_PanelType = PCGE_PanelType_SResistance;		// 接続タッチパネルタイプをシングルタッチ抵抗膜とする.
	}
#endif
	onlyPanel.setPanelType( us_PanelType );		// 接続タッチパネルタイプをPC_Panelへ設定.
}
//-----------------------------------------------------------------------------------------------------------------
//［機能］	KmApplication Event処理.
//-----------------------------------------------------------------------------------------------------------------
bool KmApplication::event( QEvent* event )
{
	switch( event->type() ) {
		case QEvent::ApplicationActivate:
			LoadPanelType();		// 接続タッチパネルタイプ取得.
			break;
		case QEvent::ApplicationDeactivate:	// 非Active時、及びそれ意外は何もしない.
		default:
			break;
	}
	return QApplication::event( event );
}
//-----------------------------------------------------------------------------------------------------------------
//［機能］	KmApplication Event通知処理.
//-----------------------------------------------------------------------------------------------------------------
bool KmApplication::notify( QObject* pc_Object, QEvent* pc_Event )
{
// 2015/09/08 Sky松尾【CQ516119】Start.	
	// [VerupQt4.7.1→Qt5.3] KMカスタマイズ対応 2015.02.10 start.
//	Bool b_reset = True;	// reset可否判定Flag.

//	if ( QEvent::KeyPress == pc_Event->type() ) {	// EventがKeyの場合.
//		QKeyEvent* keyevent = static_cast<QKeyEvent*>( pc_Event );	// eventをKeyEventにcast.
//		if ( GFWD_Key_Hard_PowerSave == keyevent->key() ) {	// キーが"押下以外"かつ"パワーセブキー"の場合はresetしない.
//			b_reset = False;
//		}
//	} else if (	QEvent::MouseButtonDblClick == pc_Event->type() || QEvent::MouseButtonPress == pc_Event->type() ||
//				QEvent::MouseButtonRelease == pc_Event->type() || QEvent::MouseMove == pc_Event->type() ) {	// EventがMouseの場合.
		// タッチイベントの場合無操作タイマーリセットを行う.
//	} else {	// Eventがキーとマウス以外の場合.
//		b_reset = False;	// Reset処理を行わない.
//	}
//	if ( b_reset ){
//		QtPanel::newInstance()->recv_qwsProcessEvent();
//	}
	// [VerupQt4.7.1→Qt5.3] KMカスタマイズ対応 2015.02.10 end.
// 2015/09/08 Sky松尾【CQ516119】End.	
// comment 2019/12/05 Sky東島 [EagleZ+][OP_BTS-14918].
	static Bool bl_PressEventSend = False;							// キー押下時のイベント送信 True:送信した False:送信していない.
	static Bool bl_PressInSleep = False;		// キー押下イベント破棄後の押下解除通知 True：通知していない False:通知済み.
	
	if( ( pc_Event->type() == QEvent::MouseButtonPress ) || ( pc_Event->type() == QEvent::MouseButtonDblClick ) ||
	    ( pc_Event->type() == QEvent::MouseMove ) || ( pc_Event->type() == QEvent::MouseButtonRelease )
	  ) {

		//省電力中は、panelmanagerへイベント通知.
		TYPE_PowerSaveStatus e_PowerSaveStatus = ppc_Power->getPowerSaveStatus();
// comment 2015/09/03 MSE内藤 [MinervaSBK][CQ515177] 電源からの省電力復帰可能オブザーバ通知受信後は、電源状態によるpanelmanagerへイベント通知をしない.
// comment 2019/12/05 Sky東島 [EagleZ+][OP_BTS-14918].
//		if( False == onlyPanel.bl_PwrStatusSleepOutFlg ) {
		if( ( False == onlyPanel.bl_PwrStatusSleepOutFlg )
		 && ( False == bl_PressEventSend ) ){				// キー押下時にイベント送信している場合はイベントを破棄しない.
			if (  (e_PowerSaveStatus == TYPE_PowerSaveStatusLowPower ||
				   e_PowerSaveStatus == TYPE_PowerSaveStatusLowPowerOut) ||		// ローパワー中.
				 ((e_PowerSaveStatus == TYPE_PowerSaveStatusSleep				// スリープ.
				|| e_PowerSaveStatus == TYPE_PowerSaveStatusSleepDelay			// スリープ遅延.
				|| e_PowerSaveStatus == TYPE_PowerSaveStatusSleepOut			// スリープ解除中.
				|| e_PowerSaveStatus == TYPE_PowerSaveStatusSleepDelayIn		// スリープ遅延移行中.
				|| e_PowerSaveStatus == TYPE_PowerSaveStatusSleepDelayOut		// スリープ遅延解除中.
				|| e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOff			// 副電源OFF.
				|| e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOffDelay	// 副電源OFF遅延.
				|| e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOnProc		// 副電源ON処理中.
				|| e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOffDelayIn	// 副電源OFF遅延移行中.
				|| e_PowerSaveStatus == TYPE_PowerSaveStatusSubPowerOffDelayOut)// 副電源OFF遅延解除中.
				&& (False == c_PowerObserver.nowWeeklyTimerTempWakeUp()) ) )	// スリープで一時起動状態で無い.
			{
				QMouseEvent *me = dynamic_cast<QMouseEvent *>(pc_Event);
				if ( me != NULL ) {
					short	x	= (short)(me->x());
					short	y	= (short)(me->y());
					int32_t	msg	= (int32_t)((( y ) << 16 ) + x);
// comment 2019/12/05 Sky東島 [EagleZ+][OP_BTS-14918].
					if( pc_Event->type() == QEvent::MouseButtonRelease ){
						// キーリリース時は座標を0で通知し、LocalUIのキー押下状態を解除する.
						msg = 0;
						bl_PressInSleep = False;
					} else {
						bl_PressInSleep = True;
					}
					PMGC_PanelManager::newInstance()->TouchPanelKeyNotice(msg);
				}
			
				//イベントを送信しない.
				return(false);
			}
		}
	}
	
	// [VerupQt4.7.1→Qt5.3] CIT不具合No.39対応 2015.04.24 Start.
	// Windowへのイベント以外間引き処理を行う.
	if( false == pc_Object->isWindowType() ){
	if( trimMouseMoveEvent( pc_Object, pc_Event ) ) {
		return( true );
	} else if( trimTouchUpdateEvent( pc_Object, pc_Event ) ) {
		return( true );
	}
	}
	// [VerupQt4.7.1→Qt5.3] CIT不具合No.39対応 2015.04.24 End.
// comment 2020/02/14 Sky東島 [EagleZ+][OP_BTS-20207].
	// QApplication::notifyの引数は開放されることがあるため情報を先に取得しておく.
	bool bl_IsWindowType = pc_Object->isWindowType();
	QEvent::Type e_MouseEventType = pc_Event->type();
	bool result = QApplication::notify( pc_Object, pc_Event );
// comment 2019/12/05 Sky東島 [EagleZ+][OP_BTS-14918].
// comment 2020/02/14 Sky東島 [EagleZ+][OP_BTS-20207].
//	if( true == pc_Object->isWindowType() ){
//		if( ( pc_Event->type() == QEvent::MouseButtonPress )
//		 || ( pc_Event->type() == QEvent::MouseButtonDblClick )
//		 || ( pc_Event->type() == QEvent::MouseMove ) ){
//			bl_PressEventSend = True;
//		} else if( pc_Event->type() == QEvent::MouseButtonRelease ) {
	if( true == bl_IsWindowType ){
		if( ( e_MouseEventType == QEvent::MouseButtonPress )
		 || ( e_MouseEventType == QEvent::MouseButtonDblClick )
		 || ( e_MouseEventType == QEvent::MouseMove ) ){
			bl_PressEventSend = True;
		} else if( e_MouseEventType == QEvent::MouseButtonRelease ) {
			bl_PressEventSend = False;
			if( True == bl_PressInSleep ){
				// キー押下イベント破棄後に押下解除を通知していない場合は通知する.
				PMGC_PanelManager::newInstance()->TouchPanelKeyNotice(0);
				bl_PressInSleep = False;
			}
		}
	}
	return( result );
}

/*================================================================================================
［機能］	本メソッドにて、MouseMoveEventを間引きする.
				タブレット非接続時は、45msec間隔で配信する.
				タブレット接続時は、90msec間隔で配信する.
--------------------------------------------------------------------------------------------------
［引数］	o		イベント配信先のオブジェクトをあらわす.
			e		配信するイベントをあらわす.
［戻値］	なし.
［履歴］	2014/07/22	Sky酒井(直)		CQ391325対応.
				本メソッド内にてQApplication::notifyの呼び出しを削除する.
				*) タイマをトリガとしてインスタンスを削除することで、破棄されたインスタンスへの参照を防ぎます.
================================================================================================*/
bool KmApplication::trimMouseMoveEvent( QObject* o, QEvent* e ) {
	static bool bl_sendEnabled = false;					// マウスイベント(move)の送信抑制をあらわす(trueならば送信を許可する, falseならば送信を抑制する).
	static const int PNLD_EventInterval = 45;			// マウスイベント(move)の送信間隔をあらわすmsec(タブレット非接続時).
	static const int PNLD_EventInterval_Tablet = 90;	// マウスイベント(move)の送信間隔をあらわすmsec(タブレット接続時).
	static int si_TimerId = -1;							// イベント間引きタイマーのIDをあらわす.

	if( NULL == o || NULL == e ) {
		// 引数が正しくない場合、該当イベントは送信しない.
		return ( true );
	}

	// タブレット接続時は間引きの間隔を90msecとする.
	// タブレット接続中に過剰なイベント送信が発生すると、画像転送制御が間に合わなくなるため.
	int si_EventInterval = PNLD_EventInterval;
	if( True == onlyPanel.m_bl_TabletMultiTouchEnableState ) {
		si_EventInterval = PNLD_EventInterval_Tablet;
	}

	bool bl_Result = false;		// 戻り値をあらわす.trueの場合は該当イベントを対象widgetへ送信しない(つまりイベントを間引く).
	switch( e->type() ) {
		case QEvent::MouseButtonPress:
		case QEvent::MouseButtonDblClick:
			if( 0 <= si_TimerId ) {
				killTimer( si_TimerId );
				si_TimerId = -1;
			}
			bl_sendEnabled = true;
			break;
		case QEvent::MouseMove:
			// マウスイベント(move)の送信が抑制されている場合は、該当イベントを対象widgetへ送信しない(つまりイベントを間引く).
			if( true != bl_sendEnabled ) {
				bl_Result = true;
			}
			// イベント間引きタイマーが開始されていない場合は、イベント間引きタイマーを起動する.
			// 同時に、マウスイベント(move)の送信抑制を開始する.
			if( 0 > si_TimerId ) {
				si_TimerId = startTimer( si_EventInterval );
				bl_sendEnabled = false;
			}
			break;
		case QEvent::MouseButtonRelease:
			if( 0 <= si_TimerId ) {
				killTimer( si_TimerId );
				si_TimerId = -1;
			}
			bl_sendEnabled = false;
			break;
		case QEvent::Timer:
		{
			// イベント間引きタイマーのトリガーを受信した際は、マウスイベント(move)の送信抑制を解除する.
			QTimerEvent* pc_TimerEvent = dynamic_cast<QTimerEvent*>( e );
			if( this == o && NULL != pc_TimerEvent ) {
				if( pc_TimerEvent->timerId() == si_TimerId ) {
					killTimer( si_TimerId );
					si_TimerId = -1;
					bl_sendEnabled = true;
				}
			}
			break;
		}
		default: break;
	}
	return ( bl_Result );
}

//-----------------------------------------------------------------------------------------------------------------
//［機能］TouchUpdateEvent間引き処理.
//-----------------------------------------------------------------------------------------------------------------
bool KmApplication::trimTouchUpdateEvent( QObject* o, QEvent* e ) {
	static const int TouchUpdateEventInterval = 45;
	static QPointer<QObject> touchHandlingObject = NULL;
	static QTouchEvent* keepingTouchUpdateEvent = NULL;
	static int touchUpdateTimerId = -1;
	static const int TouchUpdateEventTabletInterval = 90;// タブレット接続時用間引き時間.
	int i_EventInterval = TouchUpdateEventInterval;
	if( True == onlyPanel.m_bl_TabletMultiTouchEnableState ) {
		i_EventInterval = TouchUpdateEventTabletInterval;// タブレット接続時の間引き時間を変更する.
	}

	bool result = false;
	if( e->type() == QEvent::TouchBegin ) {
		touchHandlingObject = o;
		if( 0 <= touchUpdateTimerId ) {
			killTimer( touchUpdateTimerId );
			touchUpdateTimerId = -1;
		}
		if( keepingTouchUpdateEvent != NULL ) {
			delete keepingTouchUpdateEvent;
			keepingTouchUpdateEvent = NULL;
		}
	} else if( e->type() == QEvent::TouchUpdate ) {
		if( touchHandlingObject != NULL ) {
			touchHandlingObject = o;
			if( touchUpdateTimerId < 0 ) {
				touchUpdateTimerId = startTimer( i_EventInterval );
			} else {
				QTouchEvent* te = dynamic_cast<QTouchEvent*>( e );
				if( te == NULL ) {
					qWarning( "[MTUI]can't handle touch event. illegal event." );
					return( false );
				}
				QTouchEvent* keepEvent = new QTouchEvent( *te );
				if( keepingTouchUpdateEvent != NULL ) {
					QTouchEvent* pc_TmpTouchEvent = keepingTouchUpdateEvent;
					keepingTouchUpdateEvent = NULL;
					if( te->touchPoints().size() != pc_TmpTouchEvent->touchPoints().size() ) {
						// タッチ箇所の数が変わってる場合は捨てないで通知する.
						QApplication::notify( touchHandlingObject.data(), pc_TmpTouchEvent );
					} else {
						// タッチ箇所が変わってる場合は捨てるが、前回タッチ位置を補正する.
						QList<QTouchEvent::TouchPoint> adjustedPoints( te->touchPoints() );
						for( int no = 0; no < te->touchPoints().size(); no++ ) {
							adjustedPoints[no].setLastPos( pc_TmpTouchEvent->touchPoints().at( no ).lastPos() );
							adjustedPoints[no].setLastScenePos( pc_TmpTouchEvent->touchPoints().at( no ).lastScenePos() );
							adjustedPoints[no].setLastScreenPos( pc_TmpTouchEvent->touchPoints().at( no ).lastScreenPos() );
							adjustedPoints[no].setLastNormalizedPos( pc_TmpTouchEvent->touchPoints().at( no ).lastNormalizedPos() );
						}
						delete keepEvent;
// 2014/11/08 Sky太田【QT5.3VerUp】【QTAPI置き換え】Start.
//                        keepEvent = new QTouchEvent(te->type(), te->deviceType(), te->modifiers(), te->touchPointStates(), adjustedPoints);
                        keepEvent = new QTouchEvent(te->type(), te->device(), te->modifiers(), te->touchPointStates(), adjustedPoints);
// 2014/11/08 Sky太田【QT5.3VerUp】【QTAPI置き換え】End.
					}
					delete pc_TmpTouchEvent;
				}
				keepingTouchUpdateEvent = keepEvent;
				result = true;
			}
		}
	} else if( e->type() == QEvent::TouchEnd ) {
		if( 0 <= touchUpdateTimerId ) {
			killTimer( touchUpdateTimerId );
			touchUpdateTimerId = -1;
		}
		if( keepingTouchUpdateEvent != NULL ) {
			QTouchEvent* pc_TmpTouchEvent = keepingTouchUpdateEvent;
			keepingTouchUpdateEvent = NULL;
			QApplication::notify( o, pc_TmpTouchEvent );
			delete pc_TmpTouchEvent;
		}
		touchHandlingObject = NULL;
	} else if( e->type() == QEvent::Timer ) {
		if( o == this ) {
			QTimerEvent* te = dynamic_cast<QTimerEvent*>( e );
			if( te == NULL ) {
				qWarning( "[MTUI]can't handle timer event. illegal event." );
				return( false );
			}
			if( touchUpdateTimerId == te->timerId() ) {
				killTimer( touchUpdateTimerId );
				touchUpdateTimerId = -1;
				if( keepingTouchUpdateEvent != NULL ) {
					// QApplication::notify()コール後にタッチイベントのシーケンスでkeepingTouchUpdateEventの領域のdeleteを防止.
					QTouchEvent* pc_TmpTouchEvent = keepingTouchUpdateEvent;
					keepingTouchUpdateEvent = NULL;
					// QObjectが破棄されていない場合は通知する.
					if( !( touchHandlingObject.isNull() ) ) {
						QApplication::notify( touchHandlingObject.data(), pc_TmpTouchEvent );
					}
					delete pc_TmpTouchEvent;
					if( touchUpdateTimerId < 0 ) { // 直前のnotifyでタイマー発行していない場合.
						touchUpdateTimerId = startTimer( i_EventInterval );
					}
				}
			}
		}
	}
	return( result );
}

//-----------------------------------------------------------------------------------------------------------------
//［機能］	KmApplication Led状態保持.
//-----------------------------------------------------------------------------------------------------------------
void KmApplication::setLedState( LED_SETTING leddata, bool b_blink )
{
	if( false == b_blink ) {	// setLEDからの要求.
		if( DEF_BIT_ON == leddata.LedOnOff ) {	// 点灯オン.
			us_LedOnOffState = us_LedOnOffState | leddata.LedNo;
			us_LedBlinkState = (ushort)(us_LedBlinkState & ~leddata.LedNo);
		} else {		// 点灯オフ.
			us_LedOnOffState = (ushort)(us_LedOnOffState & ~leddata.LedNo);
			us_LedBlinkState = (ushort)(us_LedBlinkState & ~leddata.LedNo);
		}
	} else {	// setLEDBlinkからの要求.
		if( DEF_BIT_ON == leddata.LedBlink ) {// 点滅オン.
			us_LedOnOffState = (ushort)(us_LedOnOffState & ~leddata.LedNo);
			us_LedBlinkState = us_LedBlinkState |  leddata.LedNo;
			if( LEDBlinkTime02 == leddata.LedBlinkOnTime ) {
				us_LedBlinkTime= us_LedBlinkTime |  leddata.LedNo;
			} else {
				us_LedBlinkTime= (ushort)(us_LedBlinkTime & ~leddata.LedNo);
			}
		} else {	// 点滅オフ.
			us_LedOnOffState = (ushort)(us_LedOnOffState & ~leddata.LedNo);
			us_LedBlinkState = (ushort)(us_LedBlinkState & ~leddata.LedNo);
		}
	}
}
extern "C" { int sysBspIoctlPanelSio( FUNCPTR buffer, EXDB_PanelKind kind ); }
//-----------------------------------------------------------------------------------------------------------------
//［機能］	LED一括反映.
//-----------------------------------------------------------------------------------------------------------------
void KmApplication::loadLED()
{
	LED_SETTING leddata;
	ushort us_LedOnOffSetting = 0;		// Led点灯・消灯指定.
	ushort us_LedBlinkOffSetting = 0;	// Led点滅OFF指定.
	ushort us_LedBlinkOn01 = 0;			// Led点滅時間01.
	ushort us_LedBlinkOn02 = 0;			// Led点滅時間02.
	for( ushort us_LED = DEF_LED_CMD_LED01; us_LED <= DEF_LED_CMD_LED11; us_LED = (ushort)( us_LED << 1 ) ) {
		if( us_LedOnOffState & us_LED ) {
			us_LedOnOffSetting    |= us_LED;	// On.
			us_LedBlinkOffSetting |= us_LED;	// Blink Off.
		} else if( us_LedBlinkState & us_LED ) {
			if( us_LedBlinkTime & us_LED ) {
				us_LedBlinkOn02 |= us_LED;		// Blink On2.
			} else {
				us_LedBlinkOn01 |= us_LED;		// Blink On1.
			}
		} else {
			us_LedBlinkOffSetting |= us_LED;	// Blink Off.
		}
	}

	// 一括点滅OFF.
	leddata.LedNo = us_LedBlinkOffSetting | DEF_LED_CMD_MULTI;
	leddata.LedOnOff = DEF_BIT_OFF;
	leddata.LedBlink = DEF_BIT_OFF;
	sysBspIoctlPanelSio( &leddata, EXDB_PanelKindSetLedBlinkMulti );

	// 全点灯・消灯.
	leddata.LedNo = us_LedOnOffSetting | DEF_LED_CMD_MULTI;
	leddata.LedOnOff = DEF_BIT_ON;
	leddata.LedBlink = DEF_BIT_OFF;
	sysBspIoctlPanelSio( &leddata, EXDB_PanelKindSetLedAll );

	// 一括LED点滅ON 1.
	if( us_LedBlinkOn01 != 0 ) {
		leddata.LedNo = us_LedBlinkOn01 | DEF_LED_CMD_MULTI;
		leddata.LedOnOff = DEF_BIT_ON;
		leddata.LedBlink = DEF_BIT_ON;
		leddata.LedBlinkOnTime = LEDBlinkTime01;
		leddata.LedBlinkOffTime = LEDBlinkTime01;
		sysBspIoctlPanelSio( &leddata,EXDB_PanelKindSetLedBlinkMulti );
	}

	// 一括LED点滅ON 2.
	if( us_LedBlinkOn02 != 0 ) {
		leddata.LedNo = us_LedBlinkOn02 | DEF_LED_CMD_MULTI;
		leddata.LedOnOff = DEF_BIT_ON;
		leddata.LedBlink = DEF_BIT_ON;
		leddata.LedBlinkOnTime = LEDBlinkTime02;
		leddata.LedBlinkOffTime = LEDBlinkTime02;
		sysBspIoctlPanelSio( &leddata, EXDB_PanelKindSetLedBlinkMulti );
	}
}

//-----------------------------------------------------------------------------------------------------------------
//［機能］	QWSEvent処理.
//-----------------------------------------------------------------------------------------------------------------
//#if !defined(SIMULATOR) || defined(LINUX)
//bool KmApplication::qwsEventFilter(QWSEvent *event)
//{
//	Bool b_reset = True;	// reset可否判定Flag.
//
//	if ( QWSEvent::Key == event->type ) {	// EventがKeyの場合.
//		QWSKeyEvent* keyevent = static_cast<QWSKeyEvent*>( event );	// eventをKeyEventにcast.
//		if (( 1 != keyevent->simpleData.is_press ) || ( GFWD_Key_Hard_PowerSave == keyevent->simpleData.keycode ) ) {	// キーが"押下以外"かつ"パワーセブキー"の場合はresetしない.
//			b_reset = False;
//		}
//	} else if ( QWSEvent::Mouse == event->type ) {	// EventがMouseの場合.
//		// タッチイベントの場合無操作タイマーリセットを行う.
//	} else {	// Eventがキーとマウス以外の場合.
//		b_reset = False;	// Reset処理を行わない.
//	}
//	if ( b_reset ){
//		QtPanel::newInstance()->recv_qwsProcessEvent();
//	}
//    return false;
//}
//#endif // !defined(SIMULATOR) || defined(LINUX).

// 2015/09/08 Sky松尾【CQ516119】Start.
bool KmApplication::eventFilter( QObject *pc_Editor, QEvent *pc_Event )
{
	Bool b_reset = True;	// reset可否判定Flag.
	
	if( ( !pc_Editor ) || ( !pc_Event ) ){
		qDebug( "[KmApplication::eventFilter] Parameter Error\n" );
		return false;
	}

	if( QEvent::KeyPress == pc_Event->type() ){	// EventがKeyの場合.
		QKeyEvent* keyevent = static_cast<QKeyEvent*>( pc_Event );	// eventをKeyEventにcast.
		if( GFWD_Key_Hard_PowerSave == keyevent->key() ){	// キーが"押下以外"かつ"パワーセブキー"の場合はresetしない.
			b_reset = False;
		}
	} else if( QEvent::MouseMove == pc_Event->type() ){
		if( False == pc_Editor->isWindowType() ){
			b_reset = False;	// Reset処理を行わない.
		}
	} else if( ( QEvent::MouseButtonDblClick == pc_Event->type() )
	        || ( QEvent::MouseButtonPress == pc_Event->type() )
	        || ( QEvent::MouseButtonRelease == pc_Event->type() )
	){	// EventがMouseの場合.
		// タッチイベントの場合無操作タイマーリセットを行う.
	} else {	// Eventがキーとマウス以外の場合.
		b_reset = False;	// Reset処理を行わない.
	}
	
	if ( b_reset ){
		QtPanel::newInstance()->recv_qwsProcessEvent();
	}
	
    return false;
}
// 2015/09/08 Sky松尾【CQ516119】End.


//================================================================================================.
//［機能］	カスタムイベント受信処理.
//------------------------------------------------------------------------------------------------.
//［引数］	QEvent	 						event				イベントメッセージ.
//［戻値］	.
// [履歴].
//================================================================================================.
void KmApplication::customEvent( QEvent* pc_Event ) {
	if( PQTC_Event::isUserEventType( pc_Event->type() ) ) {
		PQTC_Event* pc_CastedEvent = ( PQTC_Event* )pc_Event;
		Bool bl_IsProcessed = pc_CastedEvent->receiver()->receiveEvent( pc_CastedEvent );
		PQTC_EventClient* pc_Client = pc_CastedEvent->getSynchronizedClient();
		if( NULL != pc_Client ) {
			pc_Client->complete( bl_IsProcessed );
		}
	}
	return;
}

#endif //QT_PANEL

