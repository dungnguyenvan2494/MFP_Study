//  $Id: BML_MIOA_TimerObserver.cpp,v 1.3 2007/07/31 08:19:57 terazono Exp $
// ****************************************************************************
// ファイル名: BML_MIOA_TimerObserver.cpp
// 概要: 
// $Author: terazono $
// $Date: 2007/07/31 08:19:57 $
// $Revision: 1.3 $
// ****************************************************************************

#include "BML_MIOA_TimerObserver.h"
#include "BML_CMMN_Messages.h"
#include "BML_UTIL_InlineFunctions.h"

// *****************************************************************************
// BMLC_MIOA_TimerObserver::BMLC_MIOA_TimerObserver
// *****************************************************************************
// 概要     : 
// 戻り値   : const 
// 備考     : 
// 作成者   : terazono
// レビュー : 
// 変更履歴 : 2007-07-06 初版.
// *****************************************************************************
BMLC_MIOA_TimerObserver::BMLC_MIOA_TimerObserver(const BMLS_SWAP_MSG_Q_ID  & rmsgqId)
    : _rmsgqId( rmsgqId )
    , _timId( 0 )
{
}

// *****************************************************************************
// BMLC_MIOA_TimerObserver::~BMLC_MIOA_TimerObserver
// *****************************************************************************
// 概要     : 
// 備考     : 
// 作成者   : terazono
// レビュー : 
// 変更履歴 : 2007-07-06 初版.
// *****************************************************************************
BMLC_MIOA_TimerObserver::~BMLC_MIOA_TimerObserver()
{
    detach();
}

// *****************************************************************************
// BMLC_MIOA_TimerObserver::expire
// *****************************************************************************
// 概要     : 
// 戻り値   : void
// 備考     : 
// 作成者   : terazono
// レビュー : 
// 変更履歴 : 2007-07-06 初版.
// *****************************************************************************
void BMLC_MIOA_TimerObserver::expire(BMLD_SWAP_TIM_ID timId)
{
    BMLS_CMMN_MsgQParam sQParam;

    sQParam.eMsgId     = BMLE_CMMN_MsgIdExpiryTimer;
    sQParam.pvShare    = reinterpret_cast< void* >( timId );
    sQParam.pvMsgParam = NULL;
    sQParam.pvReserve  = NULL;

    // 待ちなしで投げる.
    BMLF_UTIL_SendMsgToBMLinkSTask
        ( _rmsgqId, sQParam, BMLD_SWAP_NO_WAIT, BMLD_SWAP_MSG_PRI_NORMAL );
}

// *****************************************************************************
// BMLC_MIOA_TimerObserver::attach
// *****************************************************************************
// 概要     : 
// 戻り値   : void
// 備考     : 
// 作成者   : terazono
// レビュー : 
// 変更履歴 : 2007-07-06 初版.
// *****************************************************************************
void BMLC_MIOA_TimerObserver::attach()
{
    BMLC_SWAP_Timer * pcTimer = BMLC_SWAP_Timer::newInstance();
    if ( 0 == this->_timId )
    {
        this->_timId = pcTimer->attachObserver( this );
        // printf( "attach_Timer_Observer timId=%d.\n", this->_timId );
    }
}

// *****************************************************************************
// BMLC_MIOA_TimerObserver::detach
// *****************************************************************************
// 概要     : 
// 戻り値   : void
// 備考     : 
// 作成者   : terazono
// レビュー : 
// 変更履歴 : 2007-07-06 初版.
// *****************************************************************************
void BMLC_MIOA_TimerObserver::detach()
{
    if ( 0 < this->_timId )
    {
        BMLC_SWAP_Timer * pcTimer = BMLC_SWAP_Timer::newInstance();
        // printf( "detach_Timer_Observer timId=%d.\n", this->_timId );
        pcTimer->detachObserver( this->_timId );
        this->_timId = 0;
    }
}

// *****************************************************************************
// BMLC_MIOA_TimerObserver::start
// *****************************************************************************
// 概要     : 
// 戻り値   : void
// 備考     : 
// 作成者   : terazono
// レビュー : 
// 変更履歴 : 2007-07-31 初版.
// *****************************************************************************
void BMLC_MIOA_TimerObserver::start(uint32_t ulTimout)
{
    if ( 0 < this->_timId )
    {
        // タイマ作成(起動).
        // printf( "req_notifyWhen timId=%d, ulTimout=%d.\n", this->_timId, ulTimout );
        BMLC_SWAP_Timer * pcTimer = BMLC_SWAP_Timer::newInstance();
        BMLS_SWAP_ReqTime sReqTime;    
        sReqTime.eTimerType = BMLE_SWAP_CYCLE;
        sReqTime.RelTime    = ulTimout;
        pcTimer->req_notifyWhen( &sReqTime, this->_timId );
    }
}

// *****************************************************************************
// BMLC_MIOA_TimerObserver::cancel
// *****************************************************************************
// 概要     : 
// 戻り値   : void
// 備考     : 
// 作成者   : terazono
// レビュー : 
// 変更履歴 : 2007-07-06 初版.
// *****************************************************************************
void BMLC_MIOA_TimerObserver::cancel()
{
    if ( 0 < this->_timId )
    {
        BMLC_SWAP_Timer * pcTimer = BMLC_SWAP_Timer::newInstance();
        pcTimer->cancel( this->_timId );
    }
}

/******************************************************************************
    $Log: BML_MIOA_TimerObserver.cpp,v $
    Revision 1.3  2007/07/31 08:19:57  terazono
    修正:タイマ登録と,タイマ開始のメソッドを分けた.

    Revision 1.2  2007/07/17 00:58:25  terazono
    修正:NICFUM5.15対応.7/17までの修正分.

    Revision 1.1  2007/07/09 03:39:58  terazono
    新規追加:MIOAdapterで使用するタイマオブザーバ.
   
 ******************************************************************************/
