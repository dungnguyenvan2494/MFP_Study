////////////////////////////////////////////////////////////////////////////////////////////////////.
///.
///		\file		FBP_FallbackIpcForBrw.cpp.
///		\breif		概要：FallbackモードIPC通信クラス(対WebKitBrowser).
///		\author		Sky.
/// 	\data		2018/01/22.
///.
////////////////////////////////////////////////////////////////////////////////////////////////////.

#include <errno.h> 
#include "FBP_Wrapper.h"
#include "FBP_Define.h"
#include "FBP_Fallback_WebKit.h"
#include "FBP_FallbackManager.h"
#include "FBP_FallbackIpcForBrw.h"
#include "FBP_DebugPrint.h"

//------------------------------------------------------------------------------.
// メッセージ関連定義値.
//------------------------------------------------------------------------------.
static const int32_t FBPD_FALLBACK_STACKSIZE			= (1024*30);


//--------------------------------------------------------------------------------------------.
///	\brief	機能：コンストラクタ.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
FBPC_FallbackIpcForBrw::FBPC_FallbackIpcForBrw()
	: m_b_Init(False)
	 ,m_msg_Q_ID_Receive_FuncReq(-1)
	 ,m_msg_Q_ID_Send_FuncRes(-1)
	 ,m_msg_Q_ID_Send_Message(-1)
	 ,m_msg_Q_ID_Send_Callback(-1)
	 ,m_msg_Q_ID_Receive_Message(-1)
	 
{
	;	//処理なし.
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：メッセージキュー作成.
///	\par	備考：.
///	\param	なし.
///	\result	short	- 作成結果.
//		FBPD_OK			:成功.
//		FBPD_ERROR		:失敗.
//--------------------------------------------------------------------------------------------.
short FBPC_FallbackIpcForBrw::createMsgQue()
{
	FBPD_TRACE_SCOPE();

	/*----------------------------------------------------------------------*/
	/*		メッセージキューオープン										*/
	/*----------------------------------------------------------------------*/
	// (受信用) WebKitBrowser→ FallbackIPC層  関数呼び出し要求メッセージ キューオープン.
	m_msg_Q_ID_Receive_FuncReq  = FBPG_mqOpen( (char*)TEXT(FBPD_IPC_MSGQ_NAME_BRW_FALLBACK_FUNCREQ), FBPD_EVENT_MSG_MAXBRW_FALLBACK_FUNCREQ, sizeof(BRWS_IpcBrwFuncReqParam), True );
	if( FBPD_ERROR == m_msg_Q_ID_Receive_FuncReq ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackIpcForBrw::CreateMsgQue  MsgQ CreateError!  errno[%d]", errno);
		FBPG_DebugPrint( 3, "Receive_funcReq[%d]", m_msg_Q_ID_Receive_FuncReq );
		return FBPD_ERROR;
	}

	// (受信用) FallbackIPC層→ WebKitBrowser  関数戻り値通知メッセージ   キューオープン.
	m_msg_Q_ID_Send_FuncRes = FBPG_mqOpen( (char*)TEXT(FBPD_IPC_MSGQ_NAME_BRW_FALLBACK_FUNCRES), FBPD_EVENT_MSG_MAXBRW_FALLBACK_FUNCRES, sizeof(BRWS_IpcBrwFuncResParam), True );
	if( FBPD_ERROR == m_msg_Q_ID_Send_FuncRes ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackIpcForBrw::CreateMsgQue  MsgQ CreateError!  errno[%d]", errno);
		FBPG_DebugPrint( 3, "Send_funcRes[%d]", m_msg_Q_ID_Send_FuncRes );
		mq_close( m_msg_Q_ID_Receive_FuncReq );
		m_msg_Q_ID_Receive_FuncReq = FBPD_ERROR;
		return FBPD_ERROR;
	}

	// (受信用) FallbackIPC層→ WebKitBrowser  メッセージ通知   キューオープン.
	m_msg_Q_ID_Receive_Message = FBPG_mqOpen( (char*)TEXT(FBPD_IPC_MSGQ_NAME_FALLBACK_BRW_MSG), FBPD_EVENT_MSG_MAX_FALLBACK_BRW_MSG, sizeof(BRWC_BrowserMsg), True );
	if( FBPD_ERROR == m_msg_Q_ID_Receive_Message ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackIpcForBrw::CreateMsgQue  MsgQ CreateError!  errno[%d]", errno);
		FBPG_DebugPrint( 3, "Receive_Msg[%d]", m_msg_Q_ID_Receive_Message );
		mq_close( m_msg_Q_ID_Receive_FuncReq );
		m_msg_Q_ID_Receive_FuncReq = FBPD_ERROR;
		mq_close( m_msg_Q_ID_Send_FuncRes );
		m_msg_Q_ID_Send_FuncRes = FBPD_ERROR;
		return FBPD_ERROR;
	}
	return FBPD_OK;
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：初期化.
///	\par	備考：.
///	\param	なし.
///	\result	short	- 処理結果.
//		FBPD_OK			:正常.
//		FBPD_ERROR		:異常.
//--------------------------------------------------------------------------------------------.
short FBPC_FallbackIpcForBrw::initClass()
{
	FBPD_TRACE_SCOPE();

	// 未初期化時のみ.
	if(False == m_b_Init) {
		m_b_Init = True;
		/*----------------------------------------------------------------------*/
		/*		タスク生成														*/
		/*----------------------------------------------------------------------*/
		int32_t ret = FBPG_threadCreateWithStackSize( ReceiveBrowserFunc, FBPD_FALLBACK_STACKSIZE, FBPD_TaskName_ReceiverForBrw );	// [MFPソリューション分離step1] IT不具合対応.
		if (FBPD_ERROR == ret) {
			FBPG_DebugPrint( 3, "FBPC_FallbackIpcForBrw::initClass  Thread Create Err! errno[%d]", errno);
			return FBPD_ERROR;
		}
	}
	return FBPD_OK;
}



//--------------------------------------------------------------------------------------------.
///	\brief	機能：WebKitBrowser → Fallbackタスク IFコール用 IPC.
///	\par	備考：イベントループ.
///	\param	なし.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void* FBPC_FallbackIpcForBrw::ReceiveBrowserFunc( void * )
{
	FBP_START_ROUTINE;
	FBPD_TRACE_SCOPE();

	int32_t si_res = 0;

	static BRWS_IpcBrwFuncReqParam st_rcvMsg = {};
	static BRWS_IpcBrwFuncResParam st_sedMsg = {};

	FBPC_FallbackManager* pc_FallbackManager = FBPC_FallbackManager::newInstance();
	MSG_ID msgIdReceive = FBPC_FallbackIpcForBrw::newInstance()->getMsgQIdReceiveFuncReq();
	MSG_ID msgIdSend    = FBPC_FallbackIpcForBrw::newInstance()->getMsgQIdSendFuncRes();


	while( 1 ) {
		// メッセージ受信待ち.
		si_res = FBPG_mqRecive(msgIdReceive, (char *)(&st_rcvMsg), sizeof(BRWS_IpcBrwFuncReqParam));

		if ( FBPD_ERROR != si_res ) {
			//送信用メッセージ構造体の初期化.
			memset( &st_sedMsg, 0x00, sizeof( st_sedMsg ) );
			st_sedMsg.e_funcType = st_rcvMsg.e_funcType;

			//=====================================.
			//		関数タイプ毎に処理振り分け.
			//=====================================.
			FBPG_DebugPrint( 3, "FBPC_FallbackIpcForBrw::ReceiveBrowserFunc  Brw --> FallbackIPC]  msgRecieve  [%d]\n", st_rcvMsg.e_funcType );
			switch( st_rcvMsg.e_funcType ) {
			case BRWE_BrwFuncTypeBrowserInitializeNotice:			// ブラウザ起動応答.
				pc_FallbackManager->BrowserInitializeNotice();
				break;
			case BRWE_BrwFuncTypeBrowserFinalizeNotice:				// ブラウザ終了応答.
				pc_FallbackManager->BrowserFinalizeNotice();
				break;
			case BRWE_BrwFuncTypeFileLinkStartReq:					// ファイルリンク開始要求.
				{
					char* psc_FileLink = NULL;
					if( !st_rcvMsg.st_pData1.b_Null ) {
						psc_FileLink = st_rcvMsg.st_pData1.u_entity.sc_FileLinkStartReq;
					}
					st_sedMsg.u_ret.b_ret = pc_FallbackManager->FileLinkStartReq( psc_FileLink );
				}
				break;
			case BRWE_BrwFuncTypeDrawControlNotice:					// 描画開始/停止応答.
				pc_FallbackManager->DrawControlNotice( st_rcvMsg.u_arg1.uc_arg, st_rcvMsg.u_arg2.uc_arg );
				break;
			case BRWE_BrwFuncTypeGetExtensionList:					// ブラウザで取扱うファイル拡張子リストの取得.
				{
					BRWS_EXTENTION_STR* pst_List = NULL;
					if( !st_rcvMsg.st_pData1.b_Null ) {
						pst_List = st_rcvMsg.st_pData1.u_entity.st_GetExtensionList;
					}
					st_sedMsg.u_ret.us_ret = pc_FallbackManager->GetExtensionList( pst_List );
					if( False == st_rcvMsg.st_pData1.b_Null ) {
						ushort us_ListSize = sizeof( st_rcvMsg.st_pData1.u_entity.st_GetExtensionList )/ sizeof( BRWS_EXTENTION_STR );
						for( uchar i = 0; i < us_ListSize; i++ ) {
							st_sedMsg.u_entity.st_GetExtensionList[i] = st_rcvMsg.st_pData1.u_entity.st_GetExtensionList[i];
						}
					}
				}
				break;
			case BRWE_BrwFuncTypeReqRebootBrowserEngine:			// ブラウザエンジン再起動.
				pc_FallbackManager->ReqRebootBrowserEngine();
				break;
			case BRWE_BrwFuncTypeCertverify:						// サーバー証明書の検証.
				st_sedMsg.u_ret.ul_ret = pc_FallbackManager->certverify( st_rcvMsg.u_arg2.ui32_arg, st_rcvMsg.u_arg1.s_Certverify_arg.uc_sslurl );
				break;
			case BRWE_BrwFuncTypeCreateClientCertList:				// クライアント証明書一覧の取得.
				st_sedMsg.u_ret.b_ret = pc_FallbackManager->createClientCertList();
				break;
			case BRWE_BrwFuncTypeGetClientCert:						// クライアント証明書の取得.
				st_sedMsg.u_ret.si_ret = pc_FallbackManager->getClientCert( st_rcvMsg.u_arg1.ui32_arg, st_sedMsg.u_entity.uc_CertData, st_rcvMsg.st_pData1.u_entity.sc_PassWord );
				break;
			case BRWE_BrwFuncTypeTouchPanelKeyNoticeScrSvr:			//スクリーンセーバー表示中のパネル座標キー受付通知(ここではオートリセットタイマのため).
				pc_FallbackManager->TouchPanelKeyNoticeScrSvr();
				break;
			case BRWE_BrwFuncTypeSomethingKeyOn:					// JavaScript疑似キー押下通知.
				pc_FallbackManager->JavaScriptSomethingKeyOn();
				break;
			case BRWE_BrwFuncTypeBeepPlayReq:						// Beep音を鳴らす.
				pc_FallbackManager->BeepPlayReq( st_rcvMsg.u_arg1.uc_arg );
				break;
			case BRWE_BrwFuncTypeGetUSBKeyboardStatus:				// USB外付けキーボード利用可否.
				st_sedMsg.u_ret.b_ret = pc_FallbackManager->isUSBKeyBoardConnect();
				break;
			case BRWE_BrwFuncTypeBrowserErrorCode:					// エラーコード通知.
				pc_FallbackManager->BrowserErrorCode( st_rcvMsg.u_arg1.ui32_arg, (BRWE_ErrorType)st_rcvMsg.u_arg2.us_arg );
				break;
			case BRWE_BrwFuncTypeCheckForTrustedCert:				// 証明書のCNチェック.
				st_sedMsg.u_ret.b_ret = pc_FallbackManager->checkForTrustedCert(st_rcvMsg.st_pData1.u_entity.sc_CertCommonName);
				break;
			default:												// default.
				FBPG_DebugPrint( 3, "FBPC_FallbackIpcForBrw::ReceiveBrowserFunc  case error! [%d]\n", st_rcvMsg.e_funcType);
				break;
			}
			// ブラウザエンジンへIF応答メッセージ送信.
			FBPG_mqSend( msgIdSend, ( char* )&st_sedMsg, sizeof( st_sedMsg ), FBPD_MSQ_FIFO );
		} else {
			FBPG_DebugPrint( 3, "FBPC_FallbackIpcForBrw::ReceiveBrowserFunc  FBPG_mqRecive error! [%d]", errno);
		}
	}
	FBP_pthread_cleanup_pop(1);
	return NULL;
}


//--------------------------------------------------------------------------------------------.
///	\brief	機能：インスタンス取得.
///	\par	備考：インスタンス取得.
///	\param	なし.
///	\result	FBPC_FallbackIpcForBrw	インスタンス.
//--------------------------------------------------------------------------------------------.
FBPC_FallbackIpcForBrw* FBPC_FallbackIpcForBrw::newInstance()
{
	static FBPC_FallbackIpcForBrw c_Ins;

	return &c_Ins;
}



//--------------------------------------------------------------------------------------------.
///	\brief	機能：Finalize要求.
///	\par	備考：Fallbackタスクからブラウザエンジンへ要求.
///	\param	なし.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void  FBPC_FallbackIpcForBrw::FinalizeRequest()
{
	FBPD_TRACE_SCOPE();

	static BRWC_BrowserMsg  st_Msg = {};
	
	/*--------------------------------------------------*/
	/*	メッセージ種別セット							*/
	/*--------------------------------------------------*/
	st_Msg.ss_type = BR_Msg_BrowserFinalizeRequest;

	/*--------------------------------------------------*/
	/*	ブラウザ設定ファイルパスの作成					*/
	/*--------------------------------------------------*/
	// まずはpublicユーザー用作成＆セット.
	char sc_PathPublic[BRWD_INITIALUSERDIRECTORY] = {};
	snprintf( sc_PathPublic, (sizeof(sc_PathPublic)), "%s%s", FBPC_FallbackManager::getRootPath(SFLE_DirTypeBrowserPublic), "/");

	uchar	uc_UserDirectory[BRWD_INITIALUSERDIRECTORY] = {};
	memcpy( uc_UserDirectory, ( uchar* )sc_PathPublic, sizeof(sc_PathPublic) );

	/*--------------------------------------------------*/
	/*	ブラウザエンジンへメッセージ送信.				*/
	/*--------------------------------------------------*/
	SendMsgBrwEngine(&st_Msg);
}



//--------------------------------------------------------------------------------------------.
///	\brief	機能：メッセージ送信.
///	\par	備考：Fallbackタスクからブラウザエンジンへ要求.
///	\param	BRWC_BrowserMsg* 送信データ.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void  FBPC_FallbackIpcForBrw::SendMsgBrwEngine(BRWC_BrowserMsg* pst_Data)
{
	FBPD_TRACE_SCOPE();

	if( NULL == pst_Data ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackIpcForBrw::SendMsgBrwEngine arg Null Error!!\n" );
		return;
	}
	MSG_ID msgQ = FBPC_FallbackIpcForBrw::newInstance()->getMsgQIdReceiveMessage();
	FBPG_mqSend( msgQ, ( char* )pst_Data, sizeof( BRWC_BrowserMsg ), FBPD_MSQ_FIFO );

}
