//==============================================================================.
// [ファイル名] SCL_COM_message.cpp.
// [内      容] eSCL Job ControlとのI/F.
//==============================================================================.
//==============================================================================.
// インクルードファイル.
#include "SCL_COM_message.h"
#include "SCL_COM_dbgtrace.h"
#include "SCL_COM_InternalApi_external.h"
#include "SCL_COM_define.h"
#include "../mio_cnv/SCL_MIO_define.h"

SCLC_IOMessage SCLC_IOMessage::_c_IOMessage;	// クラスインスタンス.
//==============================================================================.
// コンストラクタ.
SCLC_IOMessage::SCLC_IOMessage() 
// CQ508697対応 ITS1G 林 start.
:m_msgQID(NULL)			// キューID.
// CQ508697対応 ITS1G 林 end.
{
	for(int i = 0 ; i < SOAP_RECVBUF_NUM ; i++ ){
		appresobs[i] = NULL;
	}
}
//==============================================================================.
// インスタンス取得.
// 戻り値：クラスインスタンス.
SCLC_IOMessage& SCLC_IOMessage::getInstance(){
	return _c_IOMessage;
}

//==============================================================================.
// eSCLパーサー部からメッセージを受信するためのキューを作成する.
// 戻り値：true：作成成功 / false：作成失敗.
bool SCLC_IOMessage::SCLC_CreateMsgQueue(){
	bool	b_ret		= false;	// 実行結果.
	int		i_QueNum	= SOAP_RECVBUF_NUM;					// キュー内のメッセージ数.
	int		i_QueSize	= SCLC_MioDefine::MAX_SENDBUF_SIZE;	// メッセージキューの最大メッセージサイズ.
	// MessageQue作成.
	m_msgQID = SYSF_msgQCreate( i_QueNum, i_QueSize, MSG_Q_FIFO );
	// 作成失敗.
	if( NULL == m_msgQID ){
		SCLD_TRACE_FATAL( "SCLC_IOMessage::SCLC_CreateMsgQueue(): queue create error.");
	// 作成成功.
	}else{
		SCLD_TRACE( "SCLC_IOMessage::SCLC_CreateMsgQueue(): queue create success." );
		b_ret = true;
	}
	return b_ret;
}

//==============================================================================.
// SCLC_CreateMsgQueueで作成したキューから、メッセージを受信する.
// 戻り値：bufferに格納されたデータサイズ.
// 引数　：SCLS_ReqHeader* pHeader：[OUT]ヘッダ情報.
// 　　　　void* buffer 　　　　　：[OUT]パースされた受信データが格納された構造体の先頭アドレス.
int SCLC_IOMessage::SCLC_MsgQReceive( SCLS_ReqHeader** pHeader, void** buffer ){
	int		i_ret = ERROR;		// 実行結果.
	int		i_buffersize = 0;	// バッファサイズ.
	SCLS_ReqMessage	s_ReqMessage;
	memset( &s_ReqMessage, 0, sizeof(s_ReqMessage) );
	// メッセージ受信.
	i_ret = SYSF_msgQReceive( m_msgQID, (char*)&s_ReqMessage, sizeof( s_ReqMessage ), WAIT_FOREVER );
	// 受信失敗.
	if( ERROR == i_ret ){
		SCLD_TRACE_FATAL( "SCLC_IOMessage::SCLC_MsgQReceive(): queue receive error.");
	// 受信成功.
	}else{
		SCLD_TRACE( "SCLC_IOMessage::SCLC_MsgQReceive(): queue receive success." );
		// ヘッダ情報設定 ////////.
		*pHeader = s_ReqMessage.ps_ReqHeader;
		// データ設定 ////////.
		i_buffersize = getReqBufferSize( s_ReqMessage.ps_ReqHeader->e_Message );
		if( 0 != i_buffersize ){
			if( NULL == s_ReqMessage.p_ReqBuffer ){
				SCLD_TRACE_FATAL( "SCLC_IOMessage::SCLC_MsgQReceive(): s_ReqMessage.p_ReqBuffer is NULL.");
			}else{
				// バッファの設定.
				*buffer = s_ReqMessage.p_ReqBuffer;
				i_ret = i_buffersize;
			}
		}else{
			buffer = NULL;
			i_ret = 0;
		}
	}
	return i_ret;
}

//==============================================================================.
// パーサーにメッセージを送信する.
// 戻り値：true：送信成功 / false：送信失敗.
// 引数　：SCLS_ResHeader* pHeader：[IN]ヘッダ情報.
// 　　　　void* buffer 　　　　　：[IN]送信を要求する構造体データの先頭アドレス.
// 　　　　unsigned int length　　：[IN]bufferに格納したデータサイズ.
bool SCLC_IOMessage::SCLC_MsgQSend( SCLS_ResHeader* pHeader, void* buffer, unsigned int length ){
	for(int i=0;i<SOAP_RECVBUF_NUM;i++) {
		if( appresobs[i] != NULL && pHeader != NULL ){
			if( pHeader->ul_SessionID == appresobs[i]->getSessionID() ) {
// CQ508697対応 ITS1G 林 start.
				appresobs[i]->m_p_buffer	= buffer;
				appresobs[i]->m_ps_pHeader	= pHeader;
				appresobs[i]->m_ui_length	= length;
				SCLD_TRACE("SCLC_IOMessage::SCLC_MsgQSend(): m_ui_length="<< appresobs[i]->m_ui_length );
				SCLD_TRACE("SCLS_ResHeader e_Message:"			<< appresobs[i]->m_ps_pHeader->e_Message);
				SCLD_TRACE("SCLS_ResHeader ul_SessionID:"		<< appresobs[i]->m_ps_pHeader->ul_SessionID);
				SCLD_TRACE("SCLS_ResHeader us_RspCode:"			<< appresobs[i]->m_ps_pHeader->us_RspCode);
				SCLD_TRACE("SCLS_ResHeader uc_ContentType:"		<< (char*)appresobs[i]->m_ps_pHeader->uc_ContentType);
				SCLD_TRACE("SCLS_ResHeader uc_ContentLocation:"	<< (char*)appresobs[i]->m_ps_pHeader->uc_ContentLocation);
				SCLD_TRACE("SCLS_ResHeader uc_Location:"		<< (char*)appresobs[i]->m_ps_pHeader->uc_Location);
// CQ508697対応 ITS1G 林 end.
				appresobs[i]->notify();
				detachObserver(i);
			}
		}
	}
	return true;
}

//==============================================================================.
// eSCLパーサー部からメッセージを受信するためのキューを削除する.
// 戻り値：true：削除成功 / false：削除失敗.
bool SCLC_IOMessage::SCLC_DeleteMsgQueue(){
	bool b_ret = false;	// 実行結果.
	// 削除失敗.
	if( ERROR == SYSF_msgQDelete( m_msgQID ) ){
		SCLD_TRACE_FATAL( "SCLC_IOMessage::SCLC_DeleteMsgQueue(): queue delete error.");
	// 削除成功.
	}else{
		SCLD_TRACE( "SCLC_IOMessage::SCLC_DeleteMsgQueue(): queue delete success." );
		b_ret = true;
	}
	for ( int i = 0; i < SOAP_RECVBUF_NUM; i++){
		if( appresobs[i] != NULL ){
			appresobs[i]->unlock();
			detachObserver(i);
		}
	}
	return b_ret;
}

//==============================================================================.
// eSCL Job Controlへ通知するデータをセット.
// 戻り値：true：成功 / false：失敗.
bool SCLC_IOMessage::SCLC_Internal_MsgQsend( SCLS_ReqHeader* pHeader, void* buffer ){
	bool b_ret = false;	// 実行結果.
	SCLS_ReqMessage	s_ReqMessage;
	memset( &s_ReqMessage, 0, sizeof(s_ReqMessage) );
	// pHeaderのNULLチェック.
	if( NULL == pHeader ){
		SCLD_TRACE_FATAL( "SCLC_IOMessage::SCLC_Internal_MsgQsend(): pHeader is NULL.");
	}else{
		SCLD_TRACE( __PRETTY_FUNCTION__ );
		SCLD_TRACE("SCLS_ReqHeader e_Message:"			<< pHeader->e_Message);
		SCLD_TRACE("SCLS_ReqHeader ul_SessionID:"		<< pHeader->ul_SessionID);
		SCLD_TRACE("SCLS_ReqHeader e_Method:"			<< pHeader->e_Method);
		SCLD_TRACE("SCLS_ReqHeader uc_TargetPath:"		<< (char*)pHeader->uc_TargetPath);
		SCLD_TRACE("SCLS_ReqHeader uc_Host:"			<< (char*)pHeader->uc_Host);
		SCLD_TRACE("SCLS_ReqHeader uc_ContentType:"		<< (char*)pHeader->uc_ContentType);

		// ヘッダ情報設定 ////////.
		s_ReqMessage.ps_ReqHeader =  pHeader;
		// データ設定 ////////.
		s_ReqMessage.p_ReqBuffer = buffer;
		// eSCL Job Controlへ通知するデータをセット.
		if( ERROR == SYSF_msgQSend( m_msgQID, (char*)&s_ReqMessage, sizeof(SCLS_ReqMessage), WAIT_FOREVER, MSG_PRI_NORMAL ) ){
			SCLD_TRACE_FATAL( "SCLC_IOMessage::SCLC_Internal_MsgQsend(): queue send error.");
		}else{
			b_ret = true;
		}
	}
	return b_ret;
}

//==============================================================================.
// AppReqメッセージ構造体のサイズ取得.
// 戻り値：AppReqメッセージ構造体のサイズ.
// 引数　：SCLE_Message s_Message ：[IN]eSCLメッセージID.
int SCLC_IOMessage::getReqBufferSize( SCLE_Message s_Message ){
	int i_ret = 0;
	switch( s_Message ){
	case SCLE_AppReqScanBufferInfo:
	case SCLE_AppReqScanJobs:
		i_ret = int( sizeof( SCLS_scanScanSettings ) );
		break;
	default:
		break;
	}
	return i_ret;
}

bool SCLC_IOMessage::attachObserver( SCLC_ExternalAppResObserver* obs )
{
	int i;
	for( i = 0; i < SOAP_RECVBUF_NUM; i++ ){
		if( appresobs[i] == NULL ){
			appresobs[i] = obs;
			break;
		}
	}
	if( i == SOAP_RECVBUF_NUM ){
		return false;
	}
	return true;
}

void SCLC_IOMessage::detachObserver( unsigned int i )
{
	appresobs[i] = NULL;
}

