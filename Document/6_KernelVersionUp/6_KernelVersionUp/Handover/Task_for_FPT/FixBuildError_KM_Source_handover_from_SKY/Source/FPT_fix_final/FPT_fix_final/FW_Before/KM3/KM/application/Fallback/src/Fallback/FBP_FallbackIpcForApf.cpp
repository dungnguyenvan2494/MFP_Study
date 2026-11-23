///////////////////////////////////////////////////////////.
//  FBP_FallbackIpcForApf.cpp.
//  FallbackモードIPC通信クラス(対APFプロセス).
///////////////////////////////////////////////////////////.
#include <sys/types.h> 
#include <sys/wait.h>
#include "FBP_Define.h"
#include "FBP_Wrapper.h"
#include "FBP_FallbackIpcForApf.h"
#include "FBP_FallbackIFDefForApf.h"
#include "FBP_FallbackManager.h"
#include "FBP_Fallback_WebKit.h"
#include "FBP_DebugPrint.h"
#include <errno.h> 

//------------------------------------------------------------------------------.
// メッセージ関連定義値.
//------------------------------------------------------------------------------.
static const int32_t FBPD_FALLBACK_STACKSIZE	= (1024*30);


//--------------------------------------------------------------------------------------------.
///	\brief	機能：コンストラクタ.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
FBPC_FallbackIpcForApf::FBPC_FallbackIpcForApf()
	: m_b_Init(False)
	 ,m_msg_Q_ID_Receive(-1)
{
	//なにもしない
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：初期化.
///	\par	備考：.
///	\param	なし.
///	\result	処理結果.
//--------------------------------------------------------------------------------------------.
short FBPC_FallbackIpcForApf::initClass()
{
	FBPD_TRACE_SCOPE();

	// 未初期化時のみ.
	if ( False == m_b_Init ) {
		m_b_Init = True;
		/*----------------------------------------------------------------------*/
		/*		メッセージキューオープン										*/
		/*----------------------------------------------------------------------*/
		// (受信用) MFP監視プロセス→ブラウザ   メッセージ キューオープン.
		m_msg_Q_ID_Receive = FBPG_mqOpen( (char*)TEXT(FBPD_STSM_IPC_MSGQ_NAME_APF_BROWSER_REQ), FBPD_EVENT_MSG_MAX_APF_FBP_REQ, sizeof(FBPS_STSM_notice) );

		if( FBPD_ERROR == m_msg_Q_ID_Receive ) {
			FBPG_DebugPrint( 3, "FBPC_FallbackIpcForApf::initClass  MsgQ CreateError!  errno[%d]", errno);
			FBPG_DebugPrint( 3, "  m_msg_Q_ID_Receive = [%d]\n", m_msg_Q_ID_Receive );
			return FBPD_ERROR;
		}

		/*----------------------------------------------------------------------*/
		/*		タスク生成														*/
		/*----------------------------------------------------------------------*/
		int32_t ret = FBPG_threadCreateWithStackSize( EventReceive, FBPD_FALLBACK_STACKSIZE, FBPD_TaskName_ReceiverForApf );	// [MFPソリューション分離step1] IT不具合対応.
		if ( FBPD_ERROR == ret ) {
			FBPG_DebugPrint( 3, "FBPC_FallbackIpcForApf::initClass  Thread Create Err! errno[%d]\n", errno );
			return FBPD_ERROR;
		}
	}
	return FBPD_OK;
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：Apf → Fallbackタスク IFコール用 IPC.
///	\par	備考：.
///				イベントループ.
///	\param	なし.
///	\result	処理結果.
///	\par	適用：.
//--------------------------------------------------------------------------------------------.
void* FBPC_FallbackIpcForApf::EventReceive( void * )
{
	FBP_START_ROUTINE;
	FBPD_TRACE_SCOPE();

	int32_t si_res = 0;
	static FBPS_STSM_notice st_MsgData = {};

	FBPC_FallbackIpcForApf* pc_FallbackIpcForApf = FBPC_FallbackIpcForApf::newInstance();
	MSG_ID msg_Q_ID = pc_FallbackIpcForApf->getMsgQIdReceive();

	while (1) {
		// メッセージ受信待ち.
		si_res = FBPG_mqRecive( msg_Q_ID, (char *)(&st_MsgData), sizeof(FBPS_STSM_notice) );
		if ( FBPD_ERROR != si_res ) {
			FBPG_DebugPrint( 3, "FBPC_FallbackIpcForApf::EventReceive  [Apf --> FallbackIPC]  msgRecieve\n" );
			pc_FallbackIpcForApf->DirectKeyNotice( &st_MsgData );
		}
	}
	FBP_pthread_cleanup_pop(1);
	return FBPD_OK;
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：インスタンス取得.
///	\par	備考：.
///				インスタンス取得.
///	\param	なし.
///	\result	FBPC_FallbackIpcForApf	インスタンス.
//--------------------------------------------------------------------------------------------.
FBPC_FallbackIpcForApf* FBPC_FallbackIpcForApf::newInstance()
{
	static FBPC_FallbackIpcForApf c_Ins;
	return &c_Ins;
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：外キー通知.
///	\par	備考：.
///	\param	FBPS_STSM_notice* 外キーパラメータ.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void FBPC_FallbackIpcForApf::DirectKeyNotice( FBPS_STSM_notice* pst_MsgData )
{
	FBPD_TRACE_SCOPE();

	if( NULL == pst_MsgData ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackIpcForApf::DirectKeyNotice arg Null Error!!\n" );
		return;
	}

	BRWC_BrowserMsg st_Msg = {};

	if( True == DirectKeyConvert( &st_Msg, pst_MsgData ) ) {
		if( MS_Reset == st_Msg.s_Param.message ) {
			st_Msg.ss_type = BR_Msg_DirectKeyNotice_MS_Reset;
		} else if( MS_10Key == st_Msg.s_Param.message ) {
			st_Msg.ss_type = BR_Msg_DirectKeyNotice_MS_10Key;
		} else if( MS_ExtendKey == st_Msg.s_Param.message ) {
			st_Msg.ss_type = BR_Msg_DirectKeyNotice_MS_Clear;
		} else {
			return;
		}

		FBPC_FallbackManager::newInstance()->SendMsgFallbackManager( &st_Msg );
	}
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：外キーパラメータコンバート.
///	\par	備考：.
///	\param	BRWC_BrowserMsg*  pDst [O] ： 変換先.
///	\param	FBPS_STSM_notice* pSrc [O] ： 変換元.
///	\result	Bool 通知必要性.
///				True	:必要.
///				False	:不必要.
//--------------------------------------------------------------------------------------------.
Bool FBPC_FallbackIpcForApf::DirectKeyConvert( BRWC_BrowserMsg* pDst, FBPS_STSM_notice* pSrc )
{
	FBPD_TRACE_SCOPE();

	if( ( NULL == pDst ) || ( NULL == pSrc ) ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackIpcForApf::DirectKeyConvert arg Null Error!!\n" );
		return False;
	}

	Bool res = True;

	// 外キー動作のコンバート
	switch( pSrc->is_Press ) { 
	case CLICK_TOUCH_EVENT_UP:
		pDst->s_Param.type = MT_Up;
		break;
	case CLICK_TOUCH_EVENT_DOWN:
		pDst->s_Param.type = MT_On;
		break;
	default:
		res = False;
		break;
	}

	// 外キーボタンのコンバート
	switch( pSrc->event_id ) { 
	case STSM_Key_Hard_0:
		pDst->s_Param.message = MS_10Key;
		pDst->s_Param.option = BRWD_10Key_0;
		break;
	case STSM_Key_Hard_1:
		pDst->s_Param.message = MS_10Key;
		pDst->s_Param.option = BRWD_10Key_1;
		break;
	case STSM_Key_Hard_2:
		pDst->s_Param.message = MS_10Key;
		pDst->s_Param.option = BRWD_10Key_2;
		break;
	case STSM_Key_Hard_3:
		pDst->s_Param.message = MS_10Key;
		pDst->s_Param.option = BRWD_10Key_3;
		break;
	case STSM_Key_Hard_4:
		pDst->s_Param.message = MS_10Key;
		pDst->s_Param.option = BRWD_10Key_4;
		break;
	case STSM_Key_Hard_5:
		pDst->s_Param.message = MS_10Key;
		pDst->s_Param.option = BRWD_10Key_5;
		break;
	case STSM_Key_Hard_6:
		pDst->s_Param.message = MS_10Key;
		pDst->s_Param.option = BRWD_10Key_6;
		break;
	case STSM_Key_Hard_7:
		pDst->s_Param.message = MS_10Key;
		pDst->s_Param.option = BRWD_10Key_7;
		break;
	case STSM_Key_Hard_8:
		pDst->s_Param.message = MS_10Key;
		pDst->s_Param.option = BRWD_10Key_8;
		break;
	case STSM_Key_Hard_9:
		pDst->s_Param.message = MS_10Key;
		pDst->s_Param.option = BRWD_10Key_9;
		break;
	case STSM_Key_Hard_Clr:
		pDst->s_Param.message = MS_ExtendKey;
		pDst->s_Param.option = BRWD_ExtendKey_Clear;
		break;
	case STSM_Key_Hard_Reset:
		pDst->s_Param.message = MS_Reset;
		break;
	case STSM_Key_Hard_Sharp:
		pDst->s_Param.message = MS_10Key;
		pDst->s_Param.option = BRWD_10Key_Sharp;
		break;
	case STSM_Key_Hard_Asterisk:
		pDst->s_Param.message = MS_10Key;
		pDst->s_Param.option = BRWD_10Key_Aster;
		break;
	default:
		res = False;
		break;
	}

	return res;
}

