// ================================================================================================
//! \file THRC_ResourceCommon.cpp リソース管理クラス
// ================================================================================================

#include "lib/define.h"
#include "system/thr/thrInclude/THRC_ThreadInstance.h"
#include "system/thr/thrcom/THRC_Threaddef.h"
#include "system/thr/thrcom/THRC_ResourceCommon.h"
#include "system/thr/thrcom/THRC_ImageTransInfo.h"
#include "system/thr/thrcom/ThrCommonDrvrIf.h"	// LynxHW対応.

#include "system/thr/thrcom/THRC_BSPif.h"
#include "system/img/imgg/IMGC_ImageInfo.h"	// これはインクルードを許可する.
#include "system/thr/thrInclude/THRC_Threadif.h"
#include "system/img/imgc/IMGC_ImageMemoryPoolImpColorFileMemoryCom.h"
#include "system/sys/APIC_SystemInfo.h"
#include "system/sys/APIC_OptionSetting.h"

#include "system/thr/thrcom/ThrSysFWrapper.h"

#include "system/thr/thrInclude/THRC_TroubleCounter.h"  //アンチパターン対応.
#include "system/prt/APIC_Printer.h"                         //アンチパターン対応.

#include "system/thr/thrInclude/THRC_Asicdef.h"

#ifdef TORNADO
#ifdef DEF_AmurCus
#include "memoryPartition/memoryPartitionAmur.h"
#else
#include <Linux/Platform/InitMemory.h>
#endif
#endif	// #ifdef TORNADO.

// 100726 soota Add.
#include "system/thr/thrcom/THRC_ThreadBiosif.h"

#if defined(COSIM)
#include	"system/thr/COSIM/THRC_CosimMain.h"
#endif

// FWリファクタリング APIC_Setting.hの分割.
#include	"system/sys/APIC_Enabler.h"

// 2019/02/28 SCD2 伊藤史. EagleL MR1 制御構成変更No.1 BOX画像(KM形式)インポート対応（Eagle世代越リストア).
#include "system/img/IMGG_ThreadIf.h"

#if 0		// 伸張シフト処理変更により未使用.
// !!!暫定!!!.
#define	DEF_TILEMAP_ONEALINE		256
#endif

// OP/PP共通化.
#ifdef DEF_BTC_COMPRESS_4per8
#define BTC_PLANE_NUM1 16
#define BTC_PLANE_NUM2 16
#else
#define BTC_PLANE_NUM1 13
#define BTC_PLANE_NUM2 14
#endif
int btc48_buffdump_enable = 0 ;

// 2020/01/16 Sky合屋 【EagleZ+】OP_BTS-19356 ラインカード起動の判定処理追加.
extern uchar uc_BootMode;

/*================================================================================================
	スレッドライン数ログ デバッグ用 13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppより移植.
================================================================================================*/
ushort us_debug_showLineNo = 0;

extern "C"
{
	void showLineNo( int32_t para );
}
void showLineNo( int32_t para )
{
	us_debug_showLineNo = (ushort)para;
	printf( "showLineNo Mode [%d]\n",us_debug_showLineNo );
}

// ================================================================================================ 
//! <!--機能-->コンストラクタ 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  なし 
//! <!--戻値--> 
//! \return なし 
//! <!--詳細--> 
//! \n 
//! <!--履歴--> 
// 2005/05/12 yokoi		新規作成 
// ================================================================================================ 
THRC_ResourceCommon::THRC_ResourceCommon()
:ul_resComManId(0)
{
	init();
}

// ================================================================================================ 
//! <!--機能-->デストラクタ 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  なし 
//! <!--戻値--> 
//! \return なし 
//! <!--詳細--> 
//! \n 
//! <!--履歴--> 
// 2005/05/12 yokoi		新規作成 
// ================================================================================================ 
THRC_ResourceCommon::~THRC_ResourceCommon( void )
{
}

// ================================================================================================ 
//! <!--機能-->リソースの確保 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  pc_OneImgTransInfo リソースを確保する画像転送管理情報へのポインタ 
//! <!--戻値--> 
//! \return 確保できればTrue,できなければFalse 
//! <!--詳細--> 
//! リソースを確保する\n 
//! <!--履歴--> 
// 2005/05/12 yokoi		新規作成 
// 2009/03/18 仲川 		Linux新規実装 
// ================================================================================================ 
Bool THRC_ResourceCommon::alloc( CC_OneImgTransInfo* pc_OneImgTransInfo )
{
	THRC_IntLock::lock();

	Bool b_Ans = True;

	THRL_ThAssert( pc_OneImgTransInfo != NULL, "ResourceAllocError1" , 0, 0, 0, 0, CFA14_DetailCode0x0544 );

	//! 確保可能かどうかチェック
	if( check( pc_OneImgTransInfo ) == THRE_ResourceCheck_EnableAllocation ){
        s_AllocInfo.ul_VAddrThis = (uintptr_t)pc_OneImgTransInfo;
        for( int32_t i = 0; i < THRD_MAX_BUF_USE; i++ ){
            s_AllocInfo.s_UsedBuf[i].e_UsedBuf = pc_OneImgTransInfo->s_usedBuf[i].e_usedBuf;
            s_AllocInfo.s_UsedBuf[i].b_IsWrite = pc_OneImgTransInfo->s_usedBuf[i].b_isWrite;
            s_AllocInfo.s_UsedBuf[i].ul_NeedLine = pc_OneImgTransInfo->s_usedBuf[i].ul_NeedLine;
            s_AllocInfo.s_UsedBuf[i].ul_StartLine = pc_OneImgTransInfo->s_usedBuf[i].ul_StartLine;
            s_AllocInfo.s_UsedBuf[i].ul_EndLine = pc_OneImgTransInfo->s_usedBuf[i].ul_EndLine;
            s_AllocInfo.s_UsedBuf[i].e_BufMode = pc_OneImgTransInfo->s_usedBuf[i].e_BufMode;
            if( pc_OneImgTransInfo->s_usedBuf[i].pc_BufResMan ){
                s_AllocInfo.s_UsedBuf[i].ul_BufResManId = pc_OneImgTransInfo->s_usedBuf[i].pc_BufResMan->getMyId();
            }else{
                s_AllocInfo.s_UsedBuf[i].ul_BufResManId = 0;
            }
        }
        //! ドライバへ登録する
        THRL_Drvrif_Rc_SetOwner( getMyId(), (uintptr_t)&s_AllocInfo );

		b_ResourceAlloc = True;
		//! 予約していた場合は、その情報削除

		if( pc_ReserveInfo[0] == (CC_OneImgTransInfo*)s_AllocInfo.ul_VAddrThis ){

			uc_ReserveNum--;
			THRL_ThAssert( (uc_ReserveNum < THRD_MaxReserveNum), "ResourceAllocError3 %d", uc_ReserveNum , 0, 0, 0, CFA14_DetailCode0x0545 );
			if( uc_ReserveNum > 0 ){
				memcpy( &pc_ReserveInfo[0], &pc_ReserveInfo[1], uc_ReserveNum*sizeof(pc_ReserveInfo[0]) );
			}
			pc_ReserveInfo[uc_ReserveNum] = NULL;
		} else if( pc_ReserveInfo[0] != NULL ){
			//! 予約している人とAllocした人が異なる！先にcheck()しているのでここには来ないハズ
			THRL_ThAssert( False, "BufResourceAllocError2 0x%"PRIxPTR" != 0x%"PRIxPTR"", (uintptr_t)s_AllocInfo.ul_VAddrThis, (uintptr_t)pc_ReserveInfo[0] , 0, 0, CFA14_DetailCode0x0546 );
		}
		b_Ans = True;
	} else {
		b_Ans = False;
	}

	THRC_IntLock::unlock();
	return (b_Ans);
}

// ================================================================================================ 
//! <!--機能-->リソースの予約 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  pc_OneImgTransInfo リソースを予約する画像転送管理情報へのポインタ 
//! <!--戻値--> 
//! \return 予約できればTrue,できなければFalse 
//! <!--詳細--> 
//! リソースを予約する\n 
//! <!--履歴--> 
// 2005/05/12 yokoi		新規作成 
// ================================================================================================ 
Bool THRC_ResourceCommon::reserve( CC_OneImgTransInfo* pc_OneImgTransInfo )
{
	THRC_IntLock::lock();

	Bool b_Ans = True;

	THRL_ThAssert( pc_OneImgTransInfo != NULL, "ResourceReserveError1" , 0, 0, 0, 0, CFA14_DetailCode0x0547 );

	//! 予約可能かどうかチェック
	if( check( pc_OneImgTransInfo ) != THRE_ResourceCheck_Disable ){
		THRL_ThAssert( (uc_ReserveNum < THRD_MaxReserveNum), "ResourceReserveError %d", uc_ReserveNum , 0, 0, 0, CFA14_DetailCode0x0548 );
		pc_ReserveInfo[uc_ReserveNum] = pc_OneImgTransInfo;
		uc_ReserveNum++;
		b_Ans = True;
	} else {
		b_Ans = False;
	}

	THRC_IntLock::unlock();
	return (b_Ans);
}

// ================================================================================================ 
//! <!--機能-->リソースの解放 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  pc_OneImgTransInfo リソースを開放する画像転送管理情報へのポインタ 
//! <!--戻値--> 
//! \return 開放できればTrue,できなければFalse 
//! <!--詳細--> 
//! リソースを開放する\n 
//! <!--履歴--> 
// 2005/05/12 yokoi		新規作成 
// 2008/04/28 takamoto	CQ302491:開放するリソースのオーナーと退避情報が同一の場合は退避情報をクリアする.
// 2009/03/18 仲川 		Linux新規実装 
// 2009/12/16 大本 		CQ323165:スレッドワーニング除去対応.
// ================================================================================================ 
Bool THRC_ResourceCommon::release( CC_OneImgTransInfo* pc_OneImgTransInfo )
{
	THRC_IntLock::lock();

	Bool b_Ans = True;

	THRL_ThAssert( pc_OneImgTransInfo != NULL, "ResourceReleaseError1" , 0, 0, 0, 0, CFA14_DetailCode0x0549 );

	//! 確保中かどうかチェック
	if( b_ResourceAlloc == True ){
		THRL_ThAssert( s_AllocInfo.ul_VAddrThis != 0, "ResourceReleaseError" , 0, 0, 0, 0, CFA14_DetailCode0x054A );

		//! 現状、確保している人かどうかチェック
		if( pc_OneImgTransInfo == (CC_OneImgTransInfo*)s_AllocInfo.ul_VAddrThis ){
			memset( &s_AllocInfo, 0, sizeof(s_AllocInfo) );
            //! ドライバへ登録する
            THRL_Drvrif_Rc_SetOwner( getMyId(), (uintptr_t)&s_AllocInfo );
			b_ResourceAlloc = False;
			b_Ans = True;
		} else {
			b_Ans = False;
		}
	} else {
		THRL_ThAssert( s_AllocInfo.ul_VAddrThis == 0, "ResourceReleaseError" , 0, 0, 0, 0, CFA14_DetailCode0x054B );
		b_Ans = False;
	}

// 2013/07/19 ADS吉田 CQ.381171(Act_66426) SC-SFM-14 サブメモリのファイルメモリ化.
#ifdef DEF_Soc1200

	THRS_ResourceEvacuation* p_ResEvacuation = c_ResEvacuationList.findFirst();

	while( p_ResEvacuation != NULL ){

		if( p_ResEvacuation->pc_PushInfo != NULL &&
			p_ResEvacuation->pc_PushInfo == pc_OneImgTransInfo ){
			c_ResEvacuationList.del( p_ResEvacuation );
			delete p_ResEvacuation;
			break;
		}
		p_ResEvacuation = c_ResEvacuationList.findNext();
	}
#else
	//! CQ302491:開放リソースと退避リソースのチェック.
	if( pc_OneImgTransInfo == pc_PushInfo ){
		//! 開放リソースが退避中のリソースの場合.
		pc_PushInfo = NULL;
	}
#endif

	THRC_IntLock::unlock();
	return (b_Ans);
}

// ================================================================================================ 
//! <!--機能-->リソースの初期化 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  なし 
//! <!--戻値--> 
//! \return なし 
//! <!--詳細--> 
//! リソースを初期化する\n 
//! <!--履歴--> 
// 2005/05/12 yokoi		新規作成 
// 2008/04/28 takamoto	CQ302491:リソース退避管理情報追加.
// 2009/03/18 仲川 		Linux新規実装 
// ================================================================================================ 
void THRC_ResourceCommon::init( void )
{
	THRC_IntLock::lock();

	//! 初期化
    memset( &s_AllocInfo, 0, sizeof(s_AllocInfo ) );
    if( getMyId() ){
        THRL_Drvrif_Rc_Init( getMyId() );
    }
	b_ResourceAlloc = False;
	pc_ReserveInfo[0] = NULL;
	uc_ReserveNum = 0;
// 2013/07/18 ADS吉田 CQ.381171(Act_66426) SC-SFM-14 サブメモリのファイルメモリ化.
#ifdef DEF_Soc1200
	THRS_ResourceEvacuation* p_ResEvacuation = c_ResEvacuationList.findFirst();

	while( p_ResEvacuation != NULL ){
		delete p_ResEvacuation;
		p_ResEvacuation = c_ResEvacuationList.findNext();
	}
	c_ResEvacuationList.delAll();
#else
	pc_PushInfo = NULL;						//! CQ302491:リソース退避管理情報.
#endif
	THRC_IntLock::unlock();
}

// ================================================================================================ 
//! <!--機能-->リソース予約の解放 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  pc_OneImgTransInfo リソース予約を開放する画像転送管理情報へのポインタ 
//! <!--戻値--> 
//! \return 開放できればTrue,できなければFalse 
//! <!--詳細--> 
//! リソースを開放する。基本的にキャンセル時のみ利用。\n 
//! <!--履歴--> 
// 2005/05/12 yokoi		新規作成 .
// 2012/08/31 Gonsho	静的解析対応 memcpyする時の条件追加(最後のリソースキャンセル時はコピーしない).
// ================================================================================================ 
Bool THRC_ResourceCommon::releaseReserve( CC_OneImgTransInfo* pc_OneImgTransInfo )
{
	THRC_IntLock::lock();

	Bool b_Ans = False;

	THRL_ThAssert( pc_OneImgTransInfo != NULL, "ReserveReleaseError1" , 0, 0, 0, 0, CFA14_DetailCode0x054C );

	//! 予約中かどうかチェック
	if( uc_ReserveNum > 0 ){
		//! 現状、予約している人かどうかチェック
		for( uchar i = 0; (i < THRD_MaxReserveNum) && (b_Ans == False); i++ ){
			if( pc_OneImgTransInfo == pc_ReserveInfo[i] ){
				uc_ReserveNum--;
				THRL_ThAssert( (uc_ReserveNum < THRD_MaxReserveNum), "ReleaseReserveError %d", uc_ReserveNum , 0, 0, 0, CFA14_DetailCode0x054D );
				if( ( uc_ReserveNum > 0 ) && ( i+1 < THRD_MaxReserveNum ) ){
					memcpy( &pc_ReserveInfo[i], &pc_ReserveInfo[i+1], (uc_ReserveNum-i)*sizeof(pc_ReserveInfo[0]) );
				}
				pc_ReserveInfo[uc_ReserveNum] = NULL;
				b_Ans = True;
			}
		}
	} else {
		THRL_ThAssert( pc_ReserveInfo[0] == NULL, "ReserveReleaseError" , 0, 0, 0, 0, CFA14_DetailCode0x054E );
		b_Ans = False;
	}

	THRC_IntLock::unlock();
	return (b_Ans);
}

// ================================================================================================ 
//! <!--機能-->リソースの状態チェック 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  pc_OneImgTransInfo リソース状態をチェックする画像転送管理情報へのポインタ 
//! <!--戻値--> 
//! \return 指定の画像転送管理情報が、本リソースを確保できる/予約できる/何もできず、の状態を返す 
//! <!--詳細--> 
//! リソースの状態を取得する\n 
//! <!--履歴--> 
// 2005/05/12 yokoi		新規作成 
// 2006/04/27 sugimoto	DMA03バッファアンダーラン対応.
// 2009/01/23 s.ohba 	CQ314567対応(不要なAssert削除).
// 2009/03/18 仲川		Linux新規実装 
// ================================================================================================ 
THRE_ResourceCheckResult THRC_ResourceCommon::check( CC_OneImgTransInfo* pc_OneImgTransInfo )
{
	THRC_IntLock::lock();

	THRE_ResourceCheckResult e_Ans = THRE_ResourceCheck_EnableAllocation;
	CC_OneImgTransInfo* pc_AllocInfo = (CC_OneImgTransInfo*)s_AllocInfo.ul_VAddrThis;

	THRL_ThAssert( pc_OneImgTransInfo != NULL, "ResourceCheckError1" , 0, 0, 0, 0, CFA14_DetailCode0x054F );

	//! 確保中かどうかチェック
	if( b_ResourceAlloc == True ){
		THRL_ThAssert( pc_AllocInfo != NULL, "AllocInfo == NULL!" , 0, 0, 0, 0, CFA14_DetailCode0x0550 );
		//! 予約に空きがあるかどうかチェック 
		if( uc_ReserveNum >= THRD_MaxReserveNum ){
			//! リソースが現在使用されていて、かつ予約済み
			e_Ans = THRE_ResourceCheck_Disable;
		} else {
			//! DMA03バッファアンダーラン対応 START 
			//! ジョブID、AppID,入出力が一致すれば予約できるようにする 
			if( (pc_AllocInfo->ul_JobID == pc_OneImgTransInfo->ul_JobID)
					&& ( pc_AllocInfo->st_TransModePara.en_AppIDSelThread == pc_OneImgTransInfo->st_TransModePara.en_AppIDSelThread )
					&& ( pc_AllocInfo->uc_InOut == pc_OneImgTransInfo->uc_InOut ) ){
			//! DMA03バッファアンダーラン対応 END 
				e_Ans = THRE_ResourceCheck_EnableReservation;
			} else {
				e_Ans = THRE_ResourceCheck_Disable;
			}
		}
	} else {
		THRL_ThAssert( pc_AllocInfo == NULL, "AllocInfo == NULL!" , 0, 0, 0, 0, CFA14_DetailCode0x0551 );
		//! 予約中かどうかチェック
		if( uc_ReserveNum > 0 ){
			THRL_ThAssert( pc_ReserveInfo[0] != NULL, "ReserveInfo == NULL!" , 0, 0, 0, 0, CFA14_DetailCode0x0552 );
			//! 予約していた人の先頭と一致するかどうかチェック
			if( pc_ReserveInfo[0] == pc_OneImgTransInfo ){
				//! 予約者自身なので確保ＯＫ！
				e_Ans = THRE_ResourceCheck_EnableAllocation;
			} else {
				//! 予約に空きがあるかどうかチェック 
				if( uc_ReserveNum < THRD_MaxReserveNum ){
					//! DMA03バッファアンダーラン対応 START 
					//! ジョブID、AppID,入出力が一致すれば予約できるようにする 
					//! 確保者がいないので、予約者の先頭と比較する 
					if( pc_ReserveInfo[0]->ul_JobID == pc_OneImgTransInfo->ul_JobID
							&& ( pc_ReserveInfo[0]->st_TransModePara.en_AppIDSelThread == pc_OneImgTransInfo->st_TransModePara.en_AppIDSelThread )
							&& ( pc_ReserveInfo[0]->uc_InOut == pc_OneImgTransInfo->uc_InOut )){
					//! DMA03バッファアンダーラン対応 END 
						e_Ans = THRE_ResourceCheck_EnableReservation;
					} else {
						e_Ans = THRE_ResourceCheck_Disable;
					}
				} else {
					//! 他の人に予約されている。。。
					e_Ans = THRE_ResourceCheck_Disable;
				}
			}
		} else {
			//! 誰も確保/予約をしていないので、確保ＯＫ！
			e_Ans = THRE_ResourceCheck_EnableAllocation;
		}
	}

	THRC_IntLock::unlock();
	return (e_Ans);
}

// ================================================================================================ 
//! <!--機能-->リソースのオーナー取得 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  なし 
//! <!--戻値--> 
//! \return 本リソースを確保している画像転送管理情報へのポインタを返す。確保していなければNULLを返す。 
//! <!--詳細--> 
//! リソースのオーナーを取得する\n 
//! <!--履歴--> 
// 2005/05/12 yokoi		新規作成 
// 2009/03/18 仲川 		Linux新規実装 
// ================================================================================================ 
CC_OneImgTransInfo* THRC_ResourceCommon::getOwner( void )
{
    return (CC_OneImgTransInfo*)s_AllocInfo.ul_VAddrThis;
}

// ================================================================================================ 
//! <!--機能-->リソース退避情報設定. 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  CC_OneImgTransInfo*      画像転送管理情報(退避用). 
//! <!--戻値--> 
//! \return True：設定成功、False：設定失敗. 
//! <!--詳細--> 
//! リソースの退避情報を設定する.\n 
//! 1つのリソースに対して複数の退避は考慮しない(動いていないはず).\n 
//! <!--履歴--> 
// 2008/04/28 takamoto	新規作成 CQ302491対応.
// 2013/07/19 ADS吉田	CQ.381171(Act_66426) SC-SFM-14 サブメモリのファイルメモリ化.
// ================================================================================================ 
#ifdef DEF_Soc1200
Bool THRC_ResourceCommon::setPushResouce( THRS_ResourceEvacuation &c_ResEvaInfo )
{
	THRS_ResourceEvacuation* pc_ResEvaInfo = new THRS_ResourceEvacuation;
	THRL_ThAssert( pc_ResEvaInfo != NULL, "pc_ResEvaInfo == NULL!" , 0, 0, 0, 0, CFA14_DetailCode0x0BF2 );

	*pc_ResEvaInfo = c_ResEvaInfo;
	// 退避リストに追加.
	c_ResEvacuationList.add( pc_ResEvaInfo );

	return( True );
}
#else
Bool THRC_ResourceCommon::setPushResouce( CC_OneImgTransInfo* pc_OneImgTransInfo )
{
	Bool b_Ans = True;

	if( pc_PushInfo == NULL ){
		pc_PushInfo = pc_OneImgTransInfo;
	}else{
		b_Ans = False;
	}
	return( b_Ans );
}
#endif

// ================================================================================================ 
//! <!--機能-->リソース退避情報取得. 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  なし 
//! <!--戻値--> 
//! \return CC_OneImgTransInfo*      画像転送管理情報. 
//! <!--詳細--> 
//! リソースの退避情報を取得する.\n 
//! 画像転送管理情報：退避情報がない場合はNULL.\n 
//! 退避情報設定後は退避情報をクリアする.\n 
//! <!--履歴--> 
// 2008/04/28 takamoto	新規作成 CQ302491対応.
// 2013/07/19 ADS吉田	CQ.381171(Act_66426) SC-SFM-14 サブメモリのファイルメモリ化.
// ================================================================================================ 
#ifdef DEF_Soc1200
Bool THRC_ResourceCommon::getPushResouce( THRS_ResourceEvacuation &c_ResEvaInfo  )
{
	Bool b_Ret = False;

	THRS_ResourceEvacuation* pc_ResEvaInfo = c_ResEvacuationList.findFirst();
	if( pc_ResEvaInfo != NULL ){
		c_ResEvaInfo = *pc_ResEvaInfo;
		c_ResEvacuationList.del(pc_ResEvaInfo);
		delete pc_ResEvaInfo;
		b_Ret = True;
	}

	return b_Ret;
}
#else
CC_OneImgTransInfo* THRC_ResourceCommon::getPushResouce( void  )
{
	CC_OneImgTransInfo* pc_Ans = NULL;
	pc_Ans = pc_PushInfo;
	pc_PushInfo = NULL;
	return( pc_Ans );
}
#endif

#ifdef DEF_Soc1200
// ================================================================================================.
//! <!--機能-->リソースの退避情報の確認.
//! <!--------------------------------------------------------------------------------------------->. 
//! <!--引数-->.
//! \param  THRS_ResourceEvacuation &  リソース退避管理情報構造体.
//! <!--戻値-->.
//! \return True：退避情報有り、False：退避情報無し.
//! <!--詳細-->.
//! リソースの退避情報を確認する.
//! 退避情報がある場合はTrueを返却し、引数で指定された構造体に退避情報をコピーする. 
//! 退避情報がない場合はFalseを返却する. 
//! <!--履歴-->.
//! 2013/09/18 ADS坪井	新規作成 CQ.382887(Act_67852) SC-SFM-14 サブメモリのファイルメモリ化.
// ================================================================================================.
Bool THRC_ResourceCommon::peekPushResouce( THRS_ResourceEvacuation &c_ResEvaInfo  )
{
	Bool b_Ret = False;

	THRS_ResourceEvacuation* pc_ResEvaInfo = c_ResEvacuationList.findFirst();
	if( pc_ResEvaInfo != NULL ){
		c_ResEvaInfo = *pc_ResEvaInfo;
		b_Ret = True;
	}

	return b_Ret;
}
#endif

// ================================================================================================ 
//! <!--機能-->リソース退避情報の個数を取得. 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  なし 
//! <!--戻値--> 
//! \return short      リソース退避情報の個数. 
//! <!--詳細--> 
//! リソースの退避情報の個数を取得する.\n 
//! <!--履歴--> 
//! 2013/07/19 ADS吉田	新規作成 CQ.381171(Act_66426) SC-SFM-14 サブメモリのファイルメモリ化.
// ================================================================================================ 
#ifdef DEF_Soc1200
short THRC_ResourceCommon::getResEvacuationCount( void  )
{
	return c_ResEvacuationList.getCount();
}
#endif

// ================================================================================================ 
//! <!--機能-->リソース退避情報の個数を取得. 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  なし 
//! <!--戻値--> 
//! \return short      リソース退避情報の個数. 
//! <!--詳細--> 
//! リソースの退避情報の個数を取得する.\n 
//! <!--履歴--> 
//! 2013/11/1 伊藤史. 新規作成.
// ================================================================================================ 
void THRC_ResourceCommon::showResource( void  )
{
	ushort usIndex = 0;
	ushort us_max = uc_ReserveNum;
	if( THRD_MaxReserveNum <= us_max ){
		us_max = THRD_MaxReserveNum - 1;
	}
	printf("b_ResourceAlloc=%d, uc_ReserveNum=%d\n",b_ResourceAlloc,uc_ReserveNum);

	CC_OneImgTransInfo	*ownerOneImgTransInfo = getOwner();
	if( NULL != ownerOneImgTransInfo ){
		printf("  Owner OneImgTransInfo JobID=%"PRIu32", TaskID=%d, ThreadID=%d\n",
			ownerOneImgTransInfo->ul_JobID,ownerOneImgTransInfo->en_TaskID,ownerOneImgTransInfo->en_ThreadID);
	}

	for( usIndex = 0; usIndex<us_max; usIndex++ ){
		printf("  Reserve OneImgTransInfo=0x%08p, JobID=%"PRIu32", TaskID=%d, ThreadID=%d\n",pc_ReserveInfo[usIndex],
		pc_ReserveInfo[usIndex]->ul_JobID,pc_ReserveInfo[usIndex]->en_TaskID,pc_ReserveInfo[usIndex]->en_ThreadID);
	}
}

// ================================================================================================ 
//! <!--機能-->コンストラクタ 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  なし 
//! <!--戻値--> 
//! \return なし 
//! <!--詳細--> 
//! \n 
//! <!--履歴--> 
// 2007/03/27 takamoto	新規作成 
// 2013/02/28 Sky 機種共通化対応 THRC_BufferResourceSoC::THRC_BufferResourceSoC( )より処理を移植.
// 2015/10/26 Sky システム統合SoC(S800)対応.
// ================================================================================================ 
THRC_BufferResource::THRC_BufferResource( ) :
	us_PlaneNum(0), ul_BufferWidth(0), ul_BandOffset(0), ul_BufferLine(0), ul_StartBufferAddr(0), ul_BufferOffset(0)
{
	e_MyBufID = THRE_BufId_None;
	ul_bufResManId = 0;

	e_MyBufID = THRE_BufId_None;

	s_BufData.e_BufMode                = THRE_BufMode_UnKnown;
	s_BufData.ul_JobID                 = 0;
	s_BufData.b_clearExec              = False;
	s_BufData.uc_ThrIdx                = THRD_MaxThreadBusy;
	s_BufData.us_BandNum               = 0;
	s_BufData.ul_MemorySize            = 0;
	s_BufData.ul_BandLine              = 0;
	s_BufData.us_WritePage             = 0;		// 2014/12/04 伊藤史. CQ506718 ライン数リセット異常問題対策.
	s_BufData.us_ReadPage              = 0;		// 2014/12/04 伊藤史. CQ506718 ライン数リセット異常問題対策.
	memset( &s_ClearMsginfo, 0, sizeof(THRS_MsgMemoryClearInfo) );

}

// ================================================================================================ 
//! <!--機能-->デストラクタ 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  なし 
//! <!--戻値--> 
//! \return なし 
//! <!--詳細--> 
//! \n 
//! <!--履歴--> 
// 2005/05/12 yokoi		新規作成 
// ================================================================================================ 
THRC_BufferResource::~THRC_BufferResource( void )
{
}

// ================================================================================================.
//! <!--機能-->バッファリソースの確保.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  pc_OneImgTransInfo	：バッファリソースを確保する画像転送管理情報へのポインタ.
//! \param  b_isWrite			：書込み指定(True：書込み、False：読込み).
//! <!--戻値-->.
//! \return Bool				：処理結果(True：確保成功、False：確保失敗).
//! <!--詳細-->.
//! バッファリソースの確保を行う\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//  14/12/04 伊藤史. CQ506718 ライン数リセット異常問題対策. 使用ページを記憶.
// ================================================================================================.
Bool THRC_BufferResource::alloc( CC_OneImgTransInfo* pc_OneImgTransInfo, Bool b_isWrite )
{
	THRC_IntLock::lock();

	Bool b_Ans = False;

	THRL_ThAssert( pc_OneImgTransInfo != NULL, "BufResourceAllocError1" , 0, 0, 0, 0, CFA14_DetailCode0x089A );

	//! バッファリソースとしてのチェックを先に行う.
	if( check( pc_OneImgTransInfo, b_isWrite ) == THRE_ResourceCheck_EnableAllocation ){
		//! 対応するバッファIDを利用するものであれば.
		uchar uc_BufIdx = 0;
		if( searchBufIdx( pc_OneImgTransInfo, &uc_BufIdx, b_isWrite ) == True ){

			//! alloc処理.
			if( b_isWrite == True ){
				//! 書き込み側のalloc.
				//! 2006/04/27 sugimoto DMA03バッファアンダーラン対応 START.
				s_BufData.ul_JobID = pc_OneImgTransInfo->ul_JobID;
				//! 2006/04/27 sugimoto DMA03バッファアンダーラン対応 END.
				s_BufData.uc_ThrIdx = pc_OneImgTransInfo->uc_Busy;
				s_BufData.us_WritePage = pc_OneImgTransInfo->ss_InPageID;	// 2014/12/04 伊藤史. CQ506718 ライン数リセット異常問題対策.
				b_Ans = s_BufData.c_WriteResource.alloc( pc_OneImgTransInfo );
			} else {
				//! 読み込み側のalloc.
				s_BufData.us_ReadPage = pc_OneImgTransInfo->ss_InPageID;	// 2014/12/04 伊藤史. CQ506718 ライン数リセット異常問題対策.
				b_Ans = s_BufData.c_ReadResource.alloc( pc_OneImgTransInfo );
			}

			//! ライン調整.
			if( b_isWrite == True ){
				//! 書き込み側のalloc.
				pc_OneImgTransInfo->s_usedBuf[uc_BufIdx].ul_StartLine =
				pc_OneImgTransInfo->s_usedBuf[uc_BufIdx].ul_EndLine = drvrif_io_G_WriteTopLineNo( e_MyBufID );
			} else {
				//! 読み込み側のalloc.
				pc_OneImgTransInfo->s_usedBuf[uc_BufIdx].ul_StartLine =
				pc_OneImgTransInfo->s_usedBuf[uc_BufIdx].ul_EndLine = drvrif_io_G_ReadTopLineNo( e_MyBufID );
			}
		}
        setCancelFlg( False );
	}

	THRC_IntLock::unlock();
	return (b_Ans);
}


// ================================================================================================.
//! <!--機能-->バッファリソースの予約.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  pc_OneImgTransInfo	：バッファリソースを確保する画像転送管理情報へのポインタ.
//! \param  b_isWrite			：書込み指定(True：書込み、False：読込み).
//! <!--戻値-->.
//! \return Bool				：処理結果(True：確保成功、False：確保失敗).
//! <!--詳細-->.
//! バッファリソースの予約分の確保を行う\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
Bool THRC_BufferResource::reserve( CC_OneImgTransInfo* pc_OneImgTransInfo, Bool b_isWrite )
{
	THRC_IntLock::lock();

	uchar uc_BufIdx = 0;
	Bool b_Ans = False;

	THRL_ThAssert( pc_OneImgTransInfo != NULL, "BufResourceReserveError1" , 0, 0, 0, 0, CFA14_DetailCode0x089B );

	//! 対応するバッファIDを利用するものであれば.
	if( searchBufIdx( pc_OneImgTransInfo, &uc_BufIdx, b_isWrite ) == True ){
		if( b_isWrite == True ){
			//! 書き込み側のreserve.
			b_Ans = s_BufData.c_WriteResource.reserve( pc_OneImgTransInfo );
		} else {
			//! 読み込み側のreserve.
			b_Ans = s_BufData.c_ReadResource.reserve( pc_OneImgTransInfo );
		}
	}

	THRC_IntLock::unlock();
	return (b_Ans);
}


// ================================================================================================.
//! <!--機能-->バッファリソースの開放.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  pc_OneImgTransInfo	：バッファリソースを開放する画像転送管理情報へのポインタ.
//! \param  b_isWrite			：書込み指定(True：書込み、False：読込み).
//! <!--戻値-->.
//! \return Bool				：処理結果(True：開放成功、False：開放失敗).
//! <!--詳細-->.
//! バッファリソースの開放を行う\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//  14/10/27 伊藤史. CQ505475 同幅混載LeagalでFA16問題対策. 
//                   スキャナ入力でExtToBufが遅れて完了した場合のバッファ解放もれ対策.
//  14/12/04 伊藤史. CQ506718 ライン数リセット異常問題対策.  CQ505475でデグレを発生.
//                   意図しないタイミングでライン数リセットが働き FA14になった.
//                   ページ数が書込側＝読込側のみ、書込側のreleaseを行う.
//  15/04/13 tadamasa CQ511869 分割読み込み時のBufとFile転送が1ページ分完了したかどうかのフラグを立てる.
//  2016/01/29 simogaki CQ524099 ソリューション1200dpi出力で異常画像（画像がずれる）問題対応.
// ================================================================================================.
Bool THRC_BufferResource::release( CC_OneImgTransInfo* pc_OneImgTransInfo, Bool b_isWrite )
{
	THRC_IntLock::lock();

	uchar uc_BufIdx = 0;
	Bool b_Ans = False;

	THRL_ThAssert( pc_OneImgTransInfo != NULL, "BufResourceReserveError1" , 0, 0, 0, 0, CFA14_DetailCode0x089C );

	//! 対応するバッファIDを利用するものであれば.
	if( searchBufIdx( pc_OneImgTransInfo, &uc_BufIdx, b_isWrite ) == True ){
		if( b_isWrite == True ){
			//! 書き込み側のrelease.
			pc_OneImgTransInfo->s_usedBuf[uc_BufIdx].ul_EndLine = drvrif_io_G_WriteTopLineNo( e_MyBufID );
			b_Ans = s_BufData.c_WriteResource.release( pc_OneImgTransInfo );

			// 2014/10/27 伊藤史. BufToFileが先に完了している場合は、jobIdはクリアし、ライン数もリセットする.
			// 2014/12/04 伊藤史. CQ506718 ライン数リセット異常問題対策.  CQ505475でデグレを発生.
			//                   ページ数が書込側＝読込側のみ、書込側のreleaseを行う.
			if( (THRE_BufId_ExtIn == e_MyBufID) && (True == pc_OneImgTransInfo->b_ChkBufToFilePageEnd) &&
				(s_BufData.us_WritePage == pc_OneImgTransInfo->ss_InPageID) && (s_BufData.us_ReadPage == pc_OneImgTransInfo->ss_InPageID) ){
				THRL_SetThLog( 565, THRE_DebugLogType_MTCtrLog, 100, True, (uintptr_t)e_MyBufID, s_BufData.ul_JobID, pc_OneImgTransInfo->ss_InPageID, 0, 0 );

				s_BufData.us_WritePage = 0;
				s_BufData.us_ReadPage  = 0;
				s_BufData.ul_JobID = 0;
				s_BufData.uc_ThrIdx = THRD_MaxThreadBusy;	
				drvrif_io_S_WriteTopLineNo( e_MyBufID, 0 );
				drvrif_io_S_ReadTopLineNo( e_MyBufID, 0 );
				setBufLogToThLog( 473 );
				pc_OneImgTransInfo->b_ChkBufToFilePageEnd = False;
			}
		} else {
			//! 読み込み側のrelease.
			pc_OneImgTransInfo->s_usedBuf[uc_BufIdx].ul_StartLine = drvrif_io_G_ReadTopLineNo( e_MyBufID );
			b_Ans = s_BufData.c_ReadResource.release( pc_OneImgTransInfo );
			if( (getUsedLineNum() == 0)
				&& (getOwner(True) == NULL) ){
				// 対象バッファがアプリ入力バッファでラインクリアフラグがクリアしない場合はラインのクリアはしない.
				if ( ( e_MyBufID == THRE_BufId_AppIn ) && ( pc_OneImgTransInfo->m_b_LineClr_1200dpi == False ) ) {
						// ラインクリアはしない.
				} else {
					//! 完全に読み出しが終わり、書き込みも行っていなければ、jobIdはクリアし、ライン数もリセットする.
					s_BufData.us_WritePage = 0;
					s_BufData.us_ReadPage  = 0;
					s_BufData.ul_JobID = 0;	
					s_BufData.uc_ThrIdx = THRD_MaxThreadBusy;	
					drvrif_io_S_WriteTopLineNo( e_MyBufID, 0 );
					drvrif_io_S_ReadTopLineNo( e_MyBufID, 0 );
					setBufLogToThLog( 473 );
				}
			}
		}
	}
	if( s_BufData.e_BufMode != THRE_BufMode_Normal ){		//! モードが異なる場合は一旦ノーマルに戻しておく) バッファモード初期化追加.
		// 【課題】ここからTHRC_BufMemoryをコールするのはどうかと思うので検討が必要.
		THRC_BufMemorySingletonIf::newInstance().setBufMode( e_MyBufID, THRE_BufMode_Normal ) ;
	}

	THRC_IntLock::unlock();
	return (b_Ans);
}


// ================================================================================================.
//! <!--機能-->バッファリソース予約の解放.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  pc_OneImgTransInfo	：バッファリソースを開放する画像転送管理情報へのポインタ.
//! \param  b_isWrite			：書込み指定(True：書込み、False：読込み).
//! <!--戻値-->.
//! \return Bool				：処理結果(True：開放成功、False：開放失敗).
//! <!--詳細-->.
//! バッファリソースの予約分の開放を行う\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
Bool THRC_BufferResource::releaseReserve( CC_OneImgTransInfo* pc_OneImgTransInfo, Bool b_isWrite )
{
	THRC_IntLock::lock();

	uchar uc_BufIdx = 0;
	Bool b_Ans = False;

	THRL_ThAssert( pc_OneImgTransInfo != NULL, "BufResourceReserveError1" , 0, 0, 0, 0, CFA14_DetailCode0x089D );

	//! 対応するバッファIDを利用するものであれば.
	if( searchBufIdx( pc_OneImgTransInfo, &uc_BufIdx, b_isWrite ) == True ){
		if( b_isWrite == True ){
			//! 書き込み側の予約release.
			b_Ans = s_BufData.c_WriteResource.releaseReserve( pc_OneImgTransInfo );
		} else {
			//! 読み込み側の予約release.
			b_Ans = s_BufData.c_ReadResource.releaseReserve( pc_OneImgTransInfo );
		}
	}

	THRC_IntLock::unlock();
	return (b_Ans);
}


// ================================================================================================.
//! <!--機能-->バッファリソースのクリア判定処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  pc_OneImgTransInfoWrite	：バッファリソース書き込み側をクリアする画像転送管理情報へのポインタ.
//! \param  pc_OneImgTransInfoRead	：バッファリソース読み込み側をクリアする画像転送管理情報へのポインタ.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! バッファID単位でクリアを行う為のクリア実行判定を行う\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//  15/11/10 simogaki CQ520513 先端部が青が濃く印刷される不具合対応.
// ================================================================================================.

THRS_MsgMemoryClearInfo* THRC_BufferResource::clear( CC_OneImgTransInfo* pc_OneImgTransInfoWrite, CC_OneImgTransInfo* pc_OneImgTransInfoRead )
{
	THRC_IntLock::lock();

	THRS_MsgMemoryClearInfo* ps_Ans = NULL;										//! メッセージポインタの初期化.
	uchar	uc_BufIdx = 0;
	BOOL b_DoClear = False;	//	クリア実行フラグ 初期値は"実行しない".

	THRL_ThAssert( pc_OneImgTransInfoWrite != NULL, "BufResourceClearErrorW!" , 0, 0, 0, 0, CFA14_DetailCode0x089E );
	THRL_ThAssert( pc_OneImgTransInfoRead != NULL, "BufResourceClearErrorR!" , 0, 0, 0, 0, CFA14_DetailCode0x089F );

	//! Write側クリア処理.
	if( getOwner( True ) == pc_OneImgTransInfoWrite ){
		//! Write側は動作中の場合.
		release( pc_OneImgTransInfoWrite, True );
		b_DoClear = True;		//! 自スレッド使用中のバッファリソースはクリアする.
	} else {
		//! Write側は待機中の場合.
		if( searchBufIdx( pc_OneImgTransInfoWrite, &uc_BufIdx, True ) != True ){
			THRL_ThAssert( False, "clearErrWrite" , 0, 0, 0, 0, CFA14_DetailCode0x08A0 );
		}
	}

	//! Read側クリア処理.
	if( getOwner( False ) == pc_OneImgTransInfoRead ){
		//! Read側は動作中の場合.
		release( pc_OneImgTransInfoRead, False );
		b_DoClear = True;		//! 自スレッド使用中のバッファリソースはクリアする.
	} else {
		//! Read側は待機中の場合.
		//! まず、Read側として動作したかどうかチェック.
		if( searchBufIdx( pc_OneImgTransInfoRead, &uc_BufIdx, False ) == True ){	//! BufIdx取得.
			if((pc_OneImgTransInfoRead->c_TransLineMan.getSrcLine() == 0)
				|| (pc_OneImgTransInfoRead->s_usedBuf[uc_BufIdx].ul_StartLine != drvrif_io_G_ReadTopLineNo( e_MyBufID ))){
				if( searchBufIdx( pc_OneImgTransInfoWrite, &uc_BufIdx, True ) != True ){
					THRL_ThAssert( False, "clearErrRead InfoWrite" , 0, 0, 0, 0, CFA14_DetailCode0x08A1 );
				}
			}
		} else {
			THRL_ThAssert( False, "clearErrRead InfoRead" , 0, 0, 0, 0, CFA14_DetailCode0x08A2 );
		}
	}
	
	if(( getOwner( True ) == NULL )&&( getOwner( False ) == NULL )){
		//! 他のジョブが使用中か確認.
		if( (getUsedLineNum() == 0) 
			|| (pc_OneImgTransInfoWrite->c_TransLineMan.getDstLine() 
				!= pc_OneImgTransInfoRead->c_TransLineMan.getSrcLine()) ){
			b_DoClear = True;		//! 使用済み状態のバッファリソースはクリアする.
		}
	}
	
	//! クリアメッセージ生成.
	if(( s_BufData.b_clearExec == False )										//! クリア処理以外.
	&& ( b_DoClear == True )){
	    setCancelFlg( False );													//! キャンセルフラグのクリア.

		//! クリアメッセージのパラメータ設定.
		s_ClearMsginfo.ev_ReqThID = THRE_ThrID_None;								//! 要求元ID設定.
		s_ClearMsginfo.e_CtrID    = pc_OneImgTransInfoWrite->e_CtrID;				//! 画像制御部(高速タスク)ID設定.
		THRL_SetThLog( 651, THRE_DebugLogType_ThCtrLog, 0, False, (uintptr_t)getTaskName(pc_OneImgTransInfoWrite->en_ThreadID), e_MyBufID, s_ClearMsginfo.e_CtrID );
		s_ClearMsginfo.e_DMAID    = THRE_DmaId_MemClr;								//! Asicの1/2/3枚目は意識してない。CtrIDで振り分ける.
		s_ClearMsginfo.s_ClrBufInfo.pc_BufResMan = this;							//! バッファリソース管理を設定.

		setBufLogToThLog( 474 );
		s_ClearMsginfo.s_ClrBufInfo.ul_StartLine = 0;									//! 消去先端ライン.
		s_ClearMsginfo.s_ClrBufInfo.ul_EndLine   = getBufLineNum();						//! 消去終端ライン.
		//! クリア要求.
		setClearReq();
		ps_Ans = &s_ClearMsginfo;
		//! クリア中とする.
		s_BufData.b_clearExec = True;

		//! ログの追加.
		THRL_SetThLog( 40, THRE_DebugLogType_MTCtrLog, 100, True
			, (uintptr_t)THRL_GetBufIdMsg( e_MyBufID )
			, s_ClearMsginfo.s_ClrBufInfo.b_WriteRelease
			, s_ClearMsginfo.s_ClrBufInfo.b_ReadRelease
			, s_ClearMsginfo.s_ClrBufInfo.ul_StartLine
			, (ushort)s_ClearMsginfo.s_ClrBufInfo.ul_EndLine
			);
	}

	THRC_IntLock::unlock();

	return (ps_Ans);
}


// ================================================================================================.
//! <!--機能-->クリア要求設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! クリア要求の詳細設定を行う\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//	09/10/14 岡田	バッファIDの整理. 
//  10/03/23 世古	1200dpi対応にてエンジン出力用バッファのクリアすべき最大プレーン数が16となった.
//                  そのため、プレーン毎のクリアデータ設定処理に16プレーン時の設定を追加(CQ:324531).
//  12/09/03 小霜	UV-00877 Fax3or4回線対応.
//  12/11/21 ADS坪井 CQ.377856  MS-LCL_3 THRE_BufId_Main_SynthesisOut 定義削除に伴う対応.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//  13/03/13 Sky ZeusS対応.
//  15/10/06 Sky システム統合SoC(S800)対応.
//	16/03/18 Sky バーコード属性対応.
//  17/04/07 Sky ZeusSMLKPF対応.
//  17/04/28 日立 OP/PP共通化.
//  17/06/29 Sky OP/PP共通化.エンジンバッファID追加の対応.
// ================================================================================================.
void THRC_BufferResource::setClearReq()
{
	THRC_IntLock::lock();

	//! 共通設定.
	if( THRE_BufType_Main == getBufType(e_MyBufID) ) {
		s_ClearMsginfo.b_OutputSdram = False;
		s_ClearMsginfo.b_OutputPci = True;
	}
	else{
		s_ClearMsginfo.b_OutputSdram = True;
		s_ClearMsginfo.b_OutputPci = False;
	}
	s_ClearMsginfo.b_1200Mode = False;
	//! クリアデータの設定.
	s_ClearMsginfo.s_BufImgInfo.uc_MaxColorNum = 1;
	s_ClearMsginfo.s_BufImgInfo.uc_MaxAttBitNum = 0;
	s_ClearMsginfo.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_Density;
	s_ClearMsginfo.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_1bit;
	// OP/PP共通化. 
	s_ClearMsginfo.s_BufImgInfo.e_ImageCompress = IMGE_ImageCompress_BTCxxxAndJBIG;
	s_ClearMsginfo.s_BufImgInfo.ul_DstImageHeight = 
		(s_ClearMsginfo.s_ClrBufInfo.ul_EndLine - s_ClearMsginfo.s_ClrBufInfo.ul_StartLine + getBufLineNum()) % getBufLineNum();
	if( s_ClearMsginfo.s_BufImgInfo.ul_DstImageHeight == 0 ){
		//! 全部分をクリア.
		s_ClearMsginfo.s_BufImgInfo.ul_DstImageHeight = getBufLineNum();
	}
	s_ClearMsginfo.s_BufImgInfo.ul_SrcImageWidth = 0;								//! 転送元の画像横幅 (未使用なので0).
	s_ClearMsginfo.s_BufImgInfo.ul_SrcImageHeight = 0;								//! 転送元の画像ライン数(未使用なので0).
	s_ClearMsginfo.s_BufImgInfo.uc_TopBandNo = 0;									//! 先頭バンド番号 (ページ単位の転送のとき 1) (未使用なので0).
	s_ClearMsginfo.s_BufImgInfo.us_alignWidth = 0;									//! 画像横幅設定単位(dot単位)(未使用なので0).
	s_ClearMsginfo.s_BufImgInfo.uc_AllPlaneNum   = (uchar)getPlaneNum();
	s_ClearMsginfo.s_BufImgInfo.ul_DstImageWidth = getBufWidth();

	//! バッファサイズ取得.
	s_ClearMsginfo.ul_BandSize = (s_ClearMsginfo.s_BufImgInfo.ul_DstImageWidth/8)*s_ClearMsginfo.s_BufImgInfo.ul_DstImageHeight*s_ClearMsginfo.s_BufImgInfo.uc_AllPlaneNum;

	//! バッファ依存の設定.
	switch( e_MyBufID ){
		case THRE_BufId_Main:
		case THRE_BufId_ExtIn:
		case THRE_BufId_ExtIfIn:
		case THRE_BufId_RotPrint:
		case THRE_BufId_AppIn:
		case THRE_BufId_ConvCmp:
		case THRE_BufId_EngOut:
// OP/PP共通化.
#ifdef DEF_PP_Cus		
		case THRE_BufId_EngOut1:
		case THRE_BufId_EngOut2:
		case THRE_BufId_EngOut3:
#endif //DEF_PP_Cus.		
		case THRE_BufId_CombineEng:
		case THRE_BufId_Fax0Tx:
		case THRE_BufId_Fax1Tx:
		case THRE_BufId_Fax2Tx:
		case THRE_BufId_Fax3Tx:
		case THRE_BufId_Fax4Tx:
		case THRE_BufId_Fax0Rx:
		case THRE_BufId_Fax1Rx:
		case THRE_BufId_Fax2Rx:
		case THRE_BufId_Fax3Rx:
		case THRE_BufId_Fax4Rx:
		case THRE_BufId_ScnOut:
		case THRE_BufId_ConvExp:
		case THRE_BufId_CombineApp:
		case THRE_BufId_Main_AppIn:
		case THRE_BufId_Main_ConvCmp:
			//! バッファの設定.
			s_ClearMsginfo.s_SdramAdrInfo.ul_RingBufferAddr        = getAddr();											//! バッファ先頭アドレス（１０２４バイト単位）.
			s_ClearMsginfo.s_SdramAdrInfo.ul_RingBufferOffAddr     = _roundup( getBufOffset(), 1024 ) * 1024;			//! バッファ先頭アドレスオフセット（１０２４バイト単位）.
			s_ClearMsginfo.s_SdramAdrInfo.us_RingBufferMaxPlaneNum = (ushort)getPlaneNum();								//! バッファ最大プレーン数.
			s_ClearMsginfo.s_SdramAdrInfo.us_RingBufferWidth       = ushort( _roundup( (short)getBufWidth(), 512 ) * 512 );	//! バッファ横幅（５１２ビット単位）.
			s_ClearMsginfo.s_SdramAdrInfo.us_RingBufferHeight      = ushort( _roundup( getBufLineNum(), 16 ) * 16 );		//! バッファライン数（１ライン単位）.
			s_ClearMsginfo.s_SdramAdrInfo.us_DataWidth             = (ushort)s_ClearMsginfo.s_BufImgInfo.ul_DstImageWidth;	//! データ幅(16ビット単位).
			s_ClearMsginfo.s_SdramAdrInfo.us_DataHeight            = (ushort)s_ClearMsginfo.s_BufImgInfo.ul_DstImageHeight;	//! データライン数（実ライン数）（１ライン単位）.
			s_ClearMsginfo.s_SdramAdrInfo.us_RingImageTopLineNo    = (ushort)s_ClearMsginfo.s_ClrBufInfo.ul_StartLine;		//! 画像データ先頭ライン数( バッファ先頭 0line基準 ).
			s_ClearMsginfo.s_SdramAdrInfo.ul_RingImageTopAddr      = getAddr() 
				+ ( s_ClearMsginfo.s_SdramAdrInfo.us_RingBufferWidth/8 * s_ClearMsginfo.s_ClrBufInfo.ul_StartLine );		//! 画像データ先頭アドレス.
			s_ClearMsginfo.s_SdramAdrInfo.e_BufType                = getBufType();										// バッファ種別.
			memset( &s_ClearMsginfo.s_PciAdrInfo,   0, sizeof(s_ClearMsginfo.s_PciAdrInfo)   );	//!< 転送先PCI上アドレス情報は削除しておく.
			break;

        case THRE_BufId_Main_Combine:
        case THRE_BufId_Main_CombineJimon:
        case THRE_BufId_Main_Rot:
        case THRE_BufId_Main_Preview:
        case THRE_BufId_Main_SynthesisForm:
        case THRE_BufId_Main_SynthesisIn:
#ifdef DEF_A3A4COMM
		case THRE_BufId_Main_TmpEncode:
		case THRE_BufId_Main_TmpDecode:
#endif	// #ifdef DEF_A3A4COMM.
		case THRE_BufId_ScnInRaw:
		case THRE_BufId_ScnInRaw_Dup:
		case THRE_BufId_SkewCorr:
		case THRE_BufId_RGBDelay:
		case THRE_BufId_Rotate:
		// OP/PP共通化 工程スキャン対応.
		case THRE_BufId_RawImageScan:
		case THRE_BufId_RawImageScan_Dup:
			memset( &s_ClearMsginfo.s_SdramAdrInfo, 0, sizeof(s_ClearMsginfo.s_SdramAdrInfo) );	//!< 転送先SDRAMアドレス情報は削除しておく.
            break;

		default:
			THRL_ThAssert( False, "setClearReq e_MyBufID error! %d", e_MyBufID , 0, 0, 0, CFA14_DetailCode0x08A3 );
			break;
	}
	//プレーンごとのクリアデータ設定.
	switch(s_ClearMsginfo.s_BufImgInfo.uc_AllPlaneNum){
	//１プレーン.
	case 1:
		s_ClearMsginfo.s_BufImgInfo.uc_MaxColorNum = 1;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxAttBitNum = 0;
		s_ClearMsginfo.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_Density;
		s_ClearMsginfo.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_1bit;
		break;
	//２プレーン.
	case 2:
		s_ClearMsginfo.s_BufImgInfo.uc_MaxColorNum = 2;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxAttBitNum = 1;
		s_ClearMsginfo.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_Density;
		s_ClearMsginfo.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_1bit;
		break;
	//３プレーン.
	case 3:
		s_ClearMsginfo.s_BufImgInfo.uc_MaxColorNum = 1;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxAttBitNum = 0;
		s_ClearMsginfo.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_Light;
		s_ClearMsginfo.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_8bit;
		break;
	//４プレーン.
	case 4:
		s_ClearMsginfo.s_BufImgInfo.uc_MaxColorNum = 4;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxAttBitNum = 1;
		s_ClearMsginfo.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_RGB_ATT1;
		s_ClearMsginfo.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_1bit;
		break;
	//５プレーン.
	case 5:
		s_ClearMsginfo.s_BufImgInfo.uc_MaxColorNum = 5;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxAttBitNum = 1;
		s_ClearMsginfo.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_YMCK_Att1;
		s_ClearMsginfo.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_1bit;
		break;
	//６プレーン.
	case 6:
		s_ClearMsginfo.s_BufImgInfo.uc_MaxColorNum = 5;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxAttBitNum = 2;
		s_ClearMsginfo.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_YMCK_Att2;
		s_ClearMsginfo.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_1bit;
		break;
	//７プレーン.
	case 7:
		s_ClearMsginfo.s_BufImgInfo.uc_MaxColorNum = 4;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxAttBitNum = 4;
		s_ClearMsginfo.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_RGB_ATT4;
		s_ClearMsginfo.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_1bit;
		break;
	//８プレーン.
	case 8:
		s_ClearMsginfo.s_BufImgInfo.uc_MaxColorNum = 4;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxAttBitNum = 5;
		s_ClearMsginfo.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_RGB_ColorCode5;
		s_ClearMsginfo.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_1bit;
		break;
	//９プレーン.
	case 9: 
		s_ClearMsginfo.s_BufImgInfo.uc_MaxColorNum = 3;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxAttBitNum = 0;
		s_ClearMsginfo.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_RGB;
		s_ClearMsginfo.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_8bit;
		break;
	//１０プレーン.
	case 10:
		s_ClearMsginfo.s_BufImgInfo.uc_MaxColorNum = 4;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxAttBitNum = 1;
		s_ClearMsginfo.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_RGB_ATT1;
		s_ClearMsginfo.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_8bit;
		break;
	//１１プレーン.
	case 11:
		s_ClearMsginfo.s_BufImgInfo.uc_MaxColorNum = 4;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxAttBitNum = 2;
		s_ClearMsginfo.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_RGB_ATT2;
		s_ClearMsginfo.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_8bit;
		break;
	//１２プレーン.
	case 12:
		s_ClearMsginfo.s_BufImgInfo.uc_MaxColorNum = 4;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxAttBitNum = 0;
		s_ClearMsginfo.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_YMCK;
		s_ClearMsginfo.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_8bit;
		break;
	//１３プレーン.
	case 13:
		s_ClearMsginfo.s_BufImgInfo.uc_MaxColorNum = 5;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxAttBitNum = 1;
		s_ClearMsginfo.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_YMCK_Att1;
		s_ClearMsginfo.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_8bit;
		break;
	//１４プレーン.
	case 14:
		s_ClearMsginfo.s_BufImgInfo.uc_MaxColorNum = 5;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxAttBitNum = 2;
		s_ClearMsginfo.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_YMCK_Att2;
		s_ClearMsginfo.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_8bit;
		break;
	//１６プレーン.
	case 16:
		s_ClearMsginfo.b_1200Mode = True;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxColorNum = 4;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxAttBitNum = 0;
		s_ClearMsginfo.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_YMCK;
		s_ClearMsginfo.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_1bit;
		break;
	//２０プレーン 13/03/13 Sky ZeusS対応.
	case 20:
		s_ClearMsginfo.b_1200Mode = True;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxColorNum = 5;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxAttBitNum = 1;
		s_ClearMsginfo.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_YMCK_Att1;
		s_ClearMsginfo.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_1bit;
		break;
	//２１プレーン.
	// 通常の指定では２１プレーンが指定できないため、.
	// ２０プレーンと同等の設定で、色指定をYMCK_Att2にしておく.
	case 21:
		s_ClearMsginfo.b_1200Mode = True;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxColorNum = 5;
		s_ClearMsginfo.s_BufImgInfo.uc_MaxAttBitNum = 1;
		s_ClearMsginfo.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_YMCK_Att2;
		s_ClearMsginfo.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_1bit;
		break;
	case 0:
		//! ０プレーンはありえない.
		THRL_ThAssert( False, "setClearReq uc_AllPlaneNum error! %d", s_ClearMsginfo.s_BufImgInfo.uc_AllPlaneNum , 0, 0, 0, CFA14_DetailCode0x08A4 );
		break;
	default:
		//! 多値合成バッファは15プレーン以上あるので、クリア設定不能.
		break;
	}

	THRC_IntLock::unlock();
}


// ================================================================================================.
//! <!--機能-->バッファリソースのクリア完了.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  ps_MsgClearSequence	：クリアメッセージのポインタ.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! クリア完了時のバッファ初期化を行う\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
void THRC_BufferResource::clearEnd( THRS_MsgMemoryClearInfo* ps_MsgClearSequence )
{
	THRC_IntLock::lock();

	THRL_ThAssert( ps_MsgClearSequence != NULL, "BufResourceClearEndError!" , 0, 0, 0, 0, CFA14_DetailCode0x08A5 );

	//! バッファの初期化.
	drvrif_io_S_WriteTopLineNo( e_MyBufID, 0 );
	drvrif_io_S_ReadTopLineNo( e_MyBufID, 0 );
	s_BufData.ul_JobID = 0;	
	s_BufData.uc_ThrIdx = THRD_MaxThreadBusy;	
	//! クリアしたので、バッファフル状態は必ず解除.
	drvrif_io_S_BufFull( e_MyBufID, False );
	//! クリア中状態を解除.
	s_BufData.b_clearExec = False;
	setBufLogToThLog( 475 );

	THRC_IntLock::unlock();
}


// ================================================================================================.
//! <!--機能-->バッファリソースの確保可能確認.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  pc_OneImgTransInfo			：バッファリソースを確保する画像転送管理情報へのポインタ.
//! \param  b_isWrite					：書込み指定(True：書込み、False：読込み).
//! <!--戻値-->.
//! \return THRE_ResourceCheckResult	：判定結果(0：確保可能状態、1：予約可能状態、2：使用不可状態).
//! <!--詳細-->.
//! バッファリソース確保時の確保可否判定を行う\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
THRE_ResourceCheckResult THRC_BufferResource::check( CC_OneImgTransInfo* pc_OneImgTransInfo, Bool b_isWrite )
{
	THRC_IntLock::lock();

	uchar uc_BufIdx = 0;
	THRE_ResourceCheckResult e_Ans = THRE_ResourceCheck_EnableAllocation;

	THRL_ThAssert( pc_OneImgTransInfo != NULL, "BufResourcecheckError1" , 0, 0, 0, 0, CFA14_DetailCode0x08A6 );

	//! 対応するバッファIDを利用するものであれば.
	if( searchBufIdx( pc_OneImgTransInfo, &uc_BufIdx, b_isWrite ) == True ){
		if( b_isWrite == True ){
			//! 書き込み側のcheck.
			e_Ans = s_BufData.c_WriteResource.check( pc_OneImgTransInfo );
		} else {
			//! 読み込み側のcheck.
			e_Ans = s_BufData.c_ReadResource.check( pc_OneImgTransInfo );
		}
	}

	//! バッファリソース専用のチェック.
	if( e_Ans == THRE_ResourceCheck_EnableAllocation ){
		//! 書き込んだ分を読み出しきっていなく、書き込んだ人とcheckする人が異なる時は.
		if( (getUsedLineNum() > 0) 
			&& (s_BufData.ul_JobID != pc_OneImgTransInfo->ul_JobID) ){
			//! 異なるジョブでは先行して書き込ませない/読み込ませない.
			e_Ans = THRE_ResourceCheck_Disable;
		}

		//! 別のスレッドが使用中の時は書き込みを許可しない.
		if( (b_isWrite == True) 
			&& (getUsedLineNum() > 0) 
			&& (s_BufData.uc_ThrIdx != pc_OneImgTransInfo->uc_Busy) ){
			e_Ans = THRE_ResourceCheck_Disable;
		}

		//! 書き込みの確保時(ページの先頭)は、圧縮ライン単位(DEF_BasicCompExpLine)で切り上げるので、.
		//! その分スペースが空いていることを確認する.
		//! 取得バッファが1024よりも小さい時の処理も追加する.
		//! 圧縮ライン単位あいている事を確認するのは、書き込み確保時と同じ条件でよいので、.
		//! 先頭バンドのみとする。.
		//! そうしないと、バンド転送において2バンド目以降に無駄な空きができてしまう.
		else if( (b_isWrite == True) 
			&& (getFreeLineNum() < DEF_BasicCompExpLine) 
			&& ( (pc_OneImgTransInfo->bl_PageTrans == True) 
				|| ((pc_OneImgTransInfo->bl_PageTrans == False) && (pc_OneImgTransInfo->us_StartTransNum == 0)) )
			&& ( getBufLineNum() >= DEF_BasicCompExpLine ) ){
			//! 切り上げた時に、Readを追い越す可能性があるので書き込ませない.
			e_Ans = THRE_ResourceCheck_Disable;
		}

		//! クリア中はアクセス禁止！.
		if( s_BufData.b_clearExec == True ){
			e_Ans = THRE_ResourceCheck_Disable;
		}
	}

	THRC_IntLock::unlock();
	return (e_Ans);
}


// ================================================================================================.
//! <!--機能-->バッファリソースのオーナー取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  b_isWrite			：書込み指定(True：書込み、False：読込み).
//! <!--戻値-->.
//! \return CC_OneImgTransInfo*	：バッファリソースを確保している画像転送管理情報のポインタ.
//! <!--詳細-->.
//! バッファリソースを確保している画像転送管理情報のポインタを返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
CC_OneImgTransInfo* THRC_BufferResource::getOwner( Bool b_isWrite )
{
	THRC_IntLock::lock();

	CC_OneImgTransInfo* pc_Ans = (CC_OneImgTransInfo*)THRL_Drvrif_Br_GetOwner( getMyId(), b_isWrite );

	THRC_IntLock::unlock();
	return (pc_Ans);
}


// ================================================================================================.
//! <!--機能-->書き込み先頭ライン数取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return uint32_t		：書込み先頭ライン数.
//! <!--詳細-->.
//! 書込み先頭ライン数を返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
uint32_t THRC_BufferResource::getWriteLineNum( void ) const
{
	THRC_IntLock::lock();

	uint32_t ul_WriteTopLineNo = drvrif_io_G_WriteTopLineNo( e_MyBufID );

	THRC_IntLock::unlock();
	return (ul_WriteTopLineNo);
}


// ================================================================================================.
//! <!--機能-->書き込み先頭ライン数設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  ul_writeLine	：書込みライン数.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 書込み先頭ライン数を設定する\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
void THRC_BufferResource::addWriteLineNum( uint32_t ul_writeLine )
{
	THRC_IntLock::lock();

	THRL_Drvrif_Br_AddWriteLineNum( getMyId(), ul_writeLine );

	THRC_IntLock::unlock();
}


// ================================================================================================.
//! <!--機能-->読み込み先頭ライン数取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return uint32_t		：読込み先頭ライン数.
//! <!--詳細-->.
//! 読込み先頭ライン数を返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
uint32_t THRC_BufferResource::getReadLineNum( void ) const
{
	THRC_IntLock::lock();

	uint32_t ul_ReadTopLineNo = THRL_Drvrif_Br_GetReadLineNum( getMyId() );

	THRC_IntLock::unlock();
	return (ul_ReadTopLineNo);
}


// ================================================================================================.
//! <!--機能-->読み込み先頭ライン数設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  ul_readLine		：読込みライン数.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 読込み先頭ライン数を設定する\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
void THRC_BufferResource::addReadLineNum( uint32_t ul_readLine )
{
	THRC_IntLock::lock();

	THRL_Drvrif_Br_AddReadLineNum( getMyId(), ul_readLine );

	THRC_IntLock::unlock();
}


// ================================================================================================.
//! <!--機能-->フリーライン数取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return uint32_t		：フリーライン数.
//! <!--詳細-->.
//! フリーライン数を返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
uint32_t THRC_BufferResource::getFreeLineNum( void ) const
{
	THRC_IntLock::lock();

	uint32_t ul_freeLine = THRL_Drvrif_Br_GetFreeLineNum( getMyId() );

	THRC_IntLock::unlock();
	return ( ul_freeLine );
}


// ================================================================================================.
//! <!--機能-->利用ライン数取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return uint32_t		：利用ライン数.
//! <!--詳細-->.
//! 使用しているライン数を返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
uint32_t THRC_BufferResource::getUsedLineNum( void ) const
{
	THRC_IntLock::lock();

	uint32_t ul_Line = THRL_Drvrif_Br_GetUsedLineNum( getMyId() );

	THRC_IntLock::unlock();
	return ( ul_Line );
}


// ================================================================================================.
//! <!--機能-->書き込み先頭ライン数リセット.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 書き込み先頭ライン数をリセットする\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
void THRC_BufferResource::resetWriteLineNum()
{
	THRC_IntLock::lock();

	// バッファリソース確保している画像転送管理情報を取得.
	CC_OneImgTransInfo* pcc_OneImgTransInfo = getOwner(True);
	ushort us_Index = 0;

	THRL_ThAssert( (getOwner(True) != NULL), "BufResMan:resetWrite error Id %d", e_MyBufID , 0, 0, 0, CFA14_DetailCode0x08A7 );

	// 自分自身の画像転送管理情報を検索.
	for( us_Index = 0; us_Index < THRD_MAX_BUF_USE; us_Index++ ){
		if( e_MyBufID == pcc_OneImgTransInfo->s_usedBuf[us_Index].e_usedBuf ){
			// alloc()で設定しているul_StartLineをセット.
			drvrif_io_S_WriteTopLineNo( e_MyBufID, pcc_OneImgTransInfo->s_usedBuf[us_Index].ul_StartLine );
			break;
		}
	}

	setBufLogToThLog( 476 );

	//! フル状態の解除.
	drvrif_io_S_BufFull( e_MyBufID, False );
	THRC_IntLock::unlock();
}

// ================================================================================================.
//! <!--機能-->読み込み先頭ライン数リセット.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! この関数を設けた経緯を下記に示す。.
//! 画像合成生成時にキャンセルが発生しても、画像の生成は止めないので1ページ分の絵を作成するが、.
//! 画像の取り出しはそこでストップしてしまう。.
//! よって、複数ページで合成を行った場合、前のページの残りもあるので合成画像が1ページ分よりも.
//! 多くたまるためバッファが足りなくなることがある。その状態を回避するために、キャンセル時に.
//! この関数をコールして、読み込みライン数を更新して前のページを全部読み終わったことにする。.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
void THRC_BufferResource::resetReadLineNum( CC_OneImgTransInfo* pc_OneImgTransInfo )
{
	THRC_IntLock::lock();

	if( (getOwner(True) != NULL ) && (getOwner(True) != pc_OneImgTransInfo) ){
		uchar uc_BufIdx = 0;
		if( searchBufIdx( pc_OneImgTransInfo, &uc_BufIdx, True ) == True ){
			drvrif_io_S_ReadTopLineNo( e_MyBufID, pc_OneImgTransInfo->s_usedBuf[uc_BufIdx].ul_EndLine );
		}

		setBufLogToThLog( 477 );
	}

	THRC_IntLock::unlock();
}

// ================================================================================================.
//! <!--機能-->読み込み先頭ライン数リセット.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  u32_TopLine		：読込み先頭ライン数.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! <!--履歴-->.
//  16/01/14 Sky プレスキャン対応.
// ================================================================================================.
void THRC_BufferResource::initReadLineNum( uint32_t u32_TopLine )
{
	THRC_IntLock::lock();

	drvrif_io_S_ReadTopLineNo( e_MyBufID, u32_TopLine );

	THRC_IntLock::unlock();
}

// ================================================================================================.
//! <!--機能-->書き込み先頭ライン数リセット.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! <!--履歴-->.
//  16/01/14 Sky プレスキャン対応.
// ================================================================================================.
void THRC_BufferResource::initWriteLineNum()
{
	THRC_IntLock::lock();

	drvrif_io_S_WriteTopLineNo( e_MyBufID, 0 );

	drvrif_io_S_BufFull( e_MyBufID, False );

	THRC_IntLock::unlock();

}

// ================================================================================================.
//! <!--機能-->プレーンの先頭アドレス取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  uc_PlaneNo	：プレーン番号.
//! <!--戻値-->.
//! \return uintptr_t		：バッファの先頭アドレス.
//! <return uintptr_t!--詳細-->.
//! 指定されたプレーンから先頭アドレス取得し返す\n.
//! <!--履歴-->.
//  09/08/07	越野	 	Donau環境から分離して作成.
//  09/09/29	T.Shima		AsicサブメモリHW仕様変更対応.
//  09/10/20	大石 		プレーンの先頭アドレスを返す処理をドライバioctl()アクセスから.
// 							クラス変数(ul_BufferOffset、ul_StartBufferAddr)を参照するように修正.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
uintptr_t THRC_BufferResource::getAddr( uchar uc_PlaneNo ) const
{
// 2016/12/07 KM(Morikawa) S800 LinuxSim.	
	register uintptr_t ul_OffsetAddress = 0;									//! アドレスの初期化.
//	register uint32_t ul_OffsetAddress = 0;									//! アドレスの初期化.
	ul_OffsetAddress = uc_PlaneNo * ul_BufferOffset;						//! 指定プレーンまでのオフセットサイズ取得.
	if( isTileMapBuf(e_MyBufID) == True ){									//! タイルマップ.
		//! タイルマップアドレス境界へ.
		ul_OffsetAddress = toTileMap( ul_StartBufferAddr + ul_OffsetAddress );
	}else{
		ul_OffsetAddress = _roundup( (ul_StartBufferAddr + ul_OffsetAddress), 1024 ) * 1024;
	}

	return( ul_OffsetAddress );
}


// ================================================================================================.
//! <!--機能-->バンド数取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return ushort		：バンド数.
//! <!--詳細-->.
//! バンド数を返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
uint32_t THRC_BufferResource::getBandNum( ) const
{
	return( (uint32_t)s_BufData.us_BandNum );
}


// ================================================================================================.
//! <!--機能-->1バンドのオフセット取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return uint32_t		：バンドオフセット.
//! <!--詳細-->.
//! バンドオフセットを返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 2009/10/20 大石 		1バンドのオフセットを返す処理をドライバioctl()アクセスから.
// 						クラス変数(ul_BandOffset)を参照するように修正.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
uint32_t THRC_BufferResource::getBandOffset( ) const
{
	return (ul_BandOffset);

}


// ================================================================================================.
//! <!--機能-->指定バンドまでのオフセット取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return uint32_t		：指定バンドまでのオフセット.
//! <!--詳細-->.
//! 指定バンドまでのオフセットを取得し返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 2009/10/20 大石 		指定バンドまでのオフセットを返す処理をドライバioctl()アクセスから.
// 						クラス変数(ul_BandOffset)を参照するように修正.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
uint32_t THRC_BufferResource::getBandOffset( uchar uc_BandNo ) const
{
	//! リングバッファを考慮しない、プレーン前詰め制御.
	register uint32_t ul_OffsetSize = (uint32_t)(uc_BandNo % s_BufData.us_BandNum) * ul_BandOffset;
																		//! １バンド分の大きさ＊バンド数.
	return( ul_OffsetSize );											//! 入力バッファ先頭アドレス.
}


// ================================================================================================.
//! <!--機能-->バッファのオフセット取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return uint32_t		：バッファオフセット.
//! <!--詳細-->.
//! バッファオフセットを返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 2009/10/20 大石 		バッファのオフセットを返す処理をドライバioctl()アクセスから.
// 						クラス変数(ul_BufferOffset)を参照するように修正.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
uint32_t THRC_BufferResource::getBufOffset( ) const
{
	return( ul_BufferOffset );
}


// ================================================================================================.
//! <!--機能-->プレーン数取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return uint32_t		：プレーン数.
//! <!--詳細-->.
//! プレーン数を返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 2009/10/20 大石 		プレーン数を返す処理をドライバioctl()アクセスから.
// 						クラス変数(us_PlaneNum)を参照するように修正.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
uint32_t THRC_BufferResource::getPlaneNum( void ) const
{
	return( (uint32_t)us_PlaneNum );
}

// ================================================================================================.
//! <!--機能-->バッファ種別取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return THRE_BufType	：バッファ種別.
//! <!--詳細-->.
//! バッファ種別を返す\n.
//! <!--履歴-->.
//  17/04/07 Sky ZeusSMLKPF対応.
// ================================================================================================.
THRE_BufType THRC_BufferResource::getBufType( void ) const
{
	return( e_BufType );
}


// ================================================================================================.
//! <!--機能-->バッファのライン数取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return uint32_t		：バッファのライン.
//! <!--詳細-->.
//! バッファのラインを返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 2009/10/20 大石 		バッファのライン数を返す処理をドライバioctl()アクセスから.
// 						クラス変数(ul_BufferLine)を参照するように修正.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
uint32_t THRC_BufferResource::getBufLineNum( void ) const
{
	return (ul_BufferLine);
}


// ================================================================================================.
//! <!--機能-->バッファ横幅取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return uint32_t		：バッファ横幅.
//! <!--詳細-->.
//! バッファ横幅を返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 2009/10/20 大石 		バッファ横幅を返す処理をドライバioctl()アクセスから.
// 						クラス変数(ul_BufferWidth)を参照するように修正.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
uint32_t THRC_BufferResource::getBufWidth( void ) const
{
	return (ul_BufferWidth);

}


// ================================================================================================.
//! <!--機能-->バッファメモリの総サイズ取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return uint32_t		：バッファメモリの総サイズ.
//! <!--詳細-->.
//! バッファメモリの総サイズを返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
uint32_t THRC_BufferResource::getSizeOfMemory( ) const
{
	return( s_BufData.ul_MemorySize );
}


// ================================================================================================.
//! <!--機能-->バッファ情報設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  e_BufMode	：バッファモード.
//! \param  ul_Width	：バッファ幅.
//! \param  us_Band		：バンド数.
//! \param  us_Plane	：プレーン数.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 指定パラメータからバッファ情報の設定を行う\n.
//! <!--履歴-->.
//  09/08/07 	越野	 	Donau環境から分離して作成.
//  09/09/29	T.Shima		AsicサブメモリHW仕様変更対応.
// 2009/10/20 大石 		ドライバioctl()呼び出し時、クラスのメンバ変数(us_PlaneNum、ul_BufferWidth.
// 						ul_BandOffset、ul_BufferOffset、ul_BufferLine)にもセットする処理を追加.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//  17/04/07 Sky ZeusSMLKPF対応.
// ================================================================================================.
void THRC_BufferResource::setBuffer( THRE_BufMode e_BufMode, uint32_t ul_Width, ushort us_Band, ushort us_Plane ) 
{
	THRC_IntLock::lock();

	s_BufData.e_BufMode         = e_BufMode;
	drvrif_io_S_WriteTopLineNo( e_MyBufID, 0 );
	drvrif_io_S_ReadTopLineNo( e_MyBufID, 0 );

	s_BufData.us_BandNum      = us_Band;
	us_PlaneNum = us_Plane;
	drvrif_io_S_PlaneNum( e_MyBufID, us_PlaneNum );
	ul_BufferWidth = _roundup( ul_Width, 512 ) * 512;												//! バッファ幅（dot)512dot(64byte)単位.
	drvrif_io_S_BufferWidth( e_MyBufID, ul_BufferWidth );

	if( isTileMapBuf(e_MyBufID) == True ){															//! タイルマップ.
#ifdef COSIM
		// COSIM時 バッファサイズが小さい場合に、1Tailに複数バンド配置可能で合っても
		// １バンド＝１Tailで利用されるため、メモリサイズオーバーが発生する現象の対応.
		THRS_TileMapSize	s_TileMapSize;			//!< タイルマップサイズ値取得用.
		memset( &s_TileMapSize, 0x00, sizeof(THRS_TileMapSize) );
		//! タイルマップサイズ値取得.
		getTileMapSize( &s_TileMapSize );
		ushort us_IncluedeBands = s_TileMapSize.us_Aline / (ul_BufferWidth * s_BufData.ul_BandLine / 8 );
		// ０の場合は１に補正.
		us_IncluedeBands = ( us_IncluedeBands == 0 ) ? 1 : us_IncluedeBands;
		
		// 1Tailに格納可能なBand数で割っておく(データ的に余りはでないので考慮しない).
		ul_BandOffset   = toTileMap( ul_BufferWidth * s_BufData.ul_BandLine / 8 ) / us_IncluedeBands;
#else
		ul_BandOffset   = toTileMap( ul_BufferWidth * s_BufData.ul_BandLine / 8);
#endif		
																									//! バッファバンドオフセット再計算.
		ul_BufferOffset = toTileMap( ul_BandOffset * s_BufData.us_BandNum );
																									//! 画像の置ける大きさ（リングバッファなのでこれより大きくても読み込める）.
		s_BufData.ul_MemorySize   = ul_BufferOffset * us_PlaneNum;									//! バッファが占めるメモリ容量（バイト）最大値 画像の置ける大きさ＊最大プレーン数＊最大カラー数.
	}
	else if( isRawDataBuf(e_MyBufID) == True ){										//! Rawデータ.
		if( IMGE_BlockSizeConvertStatePopImage == IMGG_getBlockSizeConvertRestoreDataState() ){
			// リストア時は、通常の BTCバッファとして使用する.
			ul_BandOffset   = ul_BufferWidth * s_BufData.ul_BandLine / 8;				//! バッファバンドオフセット再計算.
			ul_BufferOffset = ul_BandOffset * s_BufData.us_BandNum;
			s_BufData.ul_MemorySize = ul_BufferOffset * us_PlaneNum;					//! バッファが占めるメモリ容量（バイト）最大値 画像の置ける大きさ＊最大プレーン数＊最大カラー数.
		} else {
		// Rawデータの場合、1色8プレーンとしてデータ量を合わせている.
		// オフセットは8プレーン分となるため8で割らず、メモリサイズ計算時は8で割って調整.
		ul_BandOffset   = ul_BufferWidth * s_BufData.ul_BandLine;					//! バッファバンドオフセット再計算.
		ul_BufferOffset = ul_BandOffset * s_BufData.us_BandNum;
		s_BufData.ul_MemorySize = ul_BufferOffset * us_PlaneNum / 8;				//! バッファが占めるメモリ容量（バイト）最大値 画像の置ける大きさ＊最大プレーン数＊最大カラー数.
	}
	}
	else{
		ul_BandOffset   = ul_BufferWidth * s_BufData.ul_BandLine / 8;				//! バッファバンドオフセット再計算.
		ul_BufferOffset = ul_BandOffset * s_BufData.us_BandNum;
		s_BufData.ul_MemorySize = ul_BufferOffset * us_PlaneNum;					//! バッファが占めるメモリ容量（バイト）最大値 画像の置ける大きさ＊最大プレーン数＊最大カラー数.
	}
	drvrif_io_S_BandOffset( e_MyBufID, ul_BandOffset );
	drvrif_io_S_BufferOffset( e_MyBufID, ul_BufferOffset );

	ul_BufferLine = s_BufData.ul_BandLine * s_BufData.us_BandNum;
	drvrif_io_S_BufferLine( e_MyBufID, ul_BufferLine );

	e_BufType = getBufType( e_MyBufID );		// バッファ種別を取得（メモリ選択用）.

	setBufLogToThLog( 478 );

	THRC_IntLock::unlock();
}


// ================================================================================================.
//! <!--機能-->バッファモード取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return THRE_BufMode	：バッファモード.
//! <!--詳細-->.
//! バッファモードを返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
THRE_BufMode THRC_BufferResource::getBufMode() const
{
	return ( s_BufData.e_BufMode );
}


// ================================================================================================.
//! <!--機能-->タイルマップのバッファかどうか.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  THRE_BufId		：バッファID.
//! <!--戻値-->.
//! \return Bool			：True タイルマップのバッファ / False タイルマップのバッファでない.
//! <!--詳細-->.
//! タイルマップのバッファかどうかを返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//	09/10/14 岡田	バッファIDの整理. 
//  12/09/03 小霜	UV-00877 Fax3or4回線対応.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//  13/03/13 Sky ZeusS対応.
//  15/10/06 Sky システム統合SoC(S800)対応.
//  15/10/06 Sky システム統合SoC(S800)サブメモリ削除対応.
//  16/10/25 Sky合屋    [IT5_4.1]制御構成変更No.40 自動評価対応.
//  17/04/07 Sky ZeusSMLKPF対応.
// ================================================================================================.
Bool THRC_BufferResource::isTileMapBuf( THRE_BufId e_BufId ) const
{
	Bool	b_Ans = False;

	// Asic2014のみタイルあり.
	if( THRE_AsicKind_Asic2014 == THRL_Drvrif_GetAsicKind( THRE_AsicUID_Asic_3 ) ) {
		switch( e_BufId ){
			case THRE_BufId_ExtIfIn:				//!< 外部入力用圧縮前バッファ.
				b_Ans = True;
				break;
			default:
				break;
		}
	}

	return ( b_Ans );
}


// ================================================================================================.
//! <!--機能-->Rawデータのバッファかどうか.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  THRE_BufId		：バッファID.
//! <!--戻値-->.
//! \return Bool			：True Rawデータのバッファ / False BTCデータのバッファ.
//! <!--詳細-->.
//! Rawデータのバッファかどうかを返す\n.
//! <!--履歴-->.
//  16/06/15 Sky システム統合SoC(S800)対応.
//  17/04/28 日立 OP/PP共通化.
// ================================================================================================.
Bool THRC_BufferResource::isRawDataBuf( THRE_BufId e_BufId ) const
{
	Bool	b_Ans = False;

	switch( e_BufId ){
		case THRE_BufId_ScnInRaw:				//!< スキャナ入力RAWデータバッファ.
		case THRE_BufId_ScnInRaw_Dup:           //!< スキャナ入力RAWデータバッファ(裏面).
		case THRE_BufId_SkewCorr:               //!< スキュー補正データバッファ.
		case THRE_BufId_RGBDelay:               //!< RGBデータ遅延バッファ.
		// OP/PP共通化 工程スキャン.
		case THRE_BufId_RawImageScan:
		case THRE_BufId_RawImageScan_Dup:
			b_Ans = True;
			break;
		default:
			break;
	}
	return ( b_Ans );
}

// ================================================================================================.
//! <!--機能-->指定バッファのバッファ種別を返す.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  THRE_BufId		：バッファID.
//! <!--戻値-->.
//! \return THRE_BufType	：バッファ種別.
//! <!--詳細-->.
//! メインメモリ、SDRAM CH0、SDRAM CH1の種別を返す\n.
//! <!--履歴-->.
//  17/04/07 Sky ZeusSMLKPF対応.
//  17/06/29 Sky OP/PP共通化.エンジンバッファID追加の対応.
// ================================================================================================.
THRE_BufType THRC_BufferResource::getBufType( THRE_BufId e_BufId ) const
{
	THRE_BufType e_BufType = THRE_BufType_Main;

	if( True == THRF_IsA800IntegrationExist() ) {
		switch( e_BufId ) {
			// SDRAM CH0.
			case THRE_BufId_AppIn:				//!< アプリ入力用圧縮前バッファ.
			case THRE_BufId_ExtIfIn:			//!< 外部入力用圧縮前バッファ.
			case THRE_BufId_ConvCmp:			//!< 画像変換用圧縮前バッファ.
			case THRE_BufId_ScnOut:				//!< ススキャン出力用伸張後バッファ.
			case THRE_BufId_ConvExp:			//!< 画像変換用伸張後バッファ.
			case THRE_BufId_CombineApp:			//!< 画像合成用バッファ スキャン用.
			case THRE_BufId_Fax0Tx:				//!< Fax送信用バッファ(iFax).
			case THRE_BufId_Fax0Rx:				//!< Fax受信用バッファ(iFax).
			case THRE_BufId_Fax1Tx:				//!< Fax送信用バッファ(回線１).
			case THRE_BufId_Fax2Tx:				//!< Fax送信用バッファ(回線２).
			case THRE_BufId_Fax3Tx:				//!< Fax送信用バッファ(回線３).
			case THRE_BufId_Fax4Tx:				//!< Fax送信用バッファ(回線４).
// OP/PP共通化.	
#ifdef DEF_PP_Cus
			case THRE_BufId_EngOut3:				//!< エンジン出力用伸張後バッファ.
#endif //DEF_PP_Cus.
				e_BufType = THRE_BufType_SDRAM0;
				break;

			// SDRAM CH1.
			case THRE_BufId_RotPrint:			//!< 回転&拡大プリント用伸張後バッファ.
			case THRE_BufId_EngOut:			//!< エンジン出力用伸張後バッファ.
// OP/PP共通化.	
#ifdef DEF_PP_Cus
			case THRE_BufId_EngOut1:			//!< エンジン出力用伸張後バッファ.
			case THRE_BufId_EngOut2:			//!< エンジン出力用伸張後バッファ.
#endif //DEF_PP_Cus.			
			case THRE_BufId_CombineEng:			//!< 画像合成用バッファ エンジン用.
			case THRE_BufId_Rotate:				//!< 回転前バッファ.
				e_BufType = THRE_BufType_SDRAM1;
				break;

			// Main memory.
			default:
				e_BufType = THRE_BufType_Main;
				break;
		}
	}
	else {
		switch( e_BufId ) {
			// SDRAM CH0.
			case THRE_BufId_ExtIfIn:			//!< 外部入力用圧縮前バッファ.
				e_BufType = THRE_BufType_SDRAM0;
				break;

			// Main memory.
			default:
				e_BufType = THRE_BufType_Main;
				break;
		}
	}

	return ( e_BufType );
}

// ================================================================================================.
//! <!--機能-->バッファIndex検索.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  pc_OneImgTransInfo		：検索対象となる画像転送管理情報.
//! \param  uc_BufIdx				：見つかった時のバッファIndex.
//! \param  b_isWrite				：書込み指定(True：書込み、False：読込み).
//! <!--戻値-->.
//! \return Bool					：処理結果(True：検索成功、False：検索失敗).
//! <!--詳細-->.
//! バッファリソース管理のインデックスの検索を行う\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
Bool THRC_BufferResource::searchBufIdx( CC_OneImgTransInfo* pc_OneImgTransInfo, uchar* uc_BufIdx, Bool b_isWrite )
{
	THRC_IntLock::lock();

	Bool b_Ans = False;
	THRE_BufId	e_ShadowBufId = THRE_BufId_None;

	//! 対象のバッファIDのIndexを検索する.
	for( register uchar k = 0; k < THRD_MAX_BUF_USE; k++ ){
		e_ShadowBufId = THRC_BufMemorySingletonIf::newInstance().chgShadowBufId( pc_OneImgTransInfo->s_usedBuf[k].e_usedBuf );
		if( ( e_ShadowBufId == e_MyBufID)	//! 指定のバッファであれば.
			&& (pc_OneImgTransInfo->s_usedBuf[k].b_isWrite == b_isWrite) ){ 
			//! 同じバッファが２個以上あると駄目なので要注意！！.
			THRL_ThAssert( (b_Ans == False), "searchBufIdx BufId duplicated" , 0, 0, 0, 0, CFA14_DetailCode0x08A8 );
			*uc_BufIdx = k;
			b_Ans = True;
		}
	}

	THRC_IntLock::unlock();

	return (b_Ans);
}


// ================================================================================================.
//! <!--機能-->バッファモード変更判定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return Bool	：True：変更可能、False：変更不可.
//! <!--詳細-->.
//! バッファモードの変更可否をチェックし結果を返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 2009/10/20 大石 		バッファのライン数を取得する処理をドライバioctl()アクセスから.
// 						クラス変数(ul_BufferLine)を参照するように修正.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
Bool THRC_BufferResource::chkBufModeChg()
{
	THRC_IntLock::lock();

	Bool	b_Ans = False;

	if( (s_BufData.c_ReadResource.getOwner() == NULL)
		&& (s_BufData.c_WriteResource.getOwner() == NULL)
		&& (ul_BufferLine != 0)								//! バッファが初期化されていれば.
		&& (THRL_Drvrif_Br_GetUsedLineNum( getMyId() ) == 0)
		&& (s_BufData.b_clearExec == False)
	){
		b_Ans = True;
	}

	THRC_IntLock::unlock();
	
	return( b_Ans );
}


// ================================================================================================.
//! <!--機能-->キャンセル状態取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return Bool	：キャンセル状態.
//! <!--詳細-->.
//! キャンセル状態を返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
Bool THRC_BufferResource::isCanceled( void )
{
	THRC_IntLock::lock();

	Bool b_cancelFlg = THRL_Drvrif_Br_IsCanceled( getMyId() );

	THRC_IntLock::unlock();
	return (b_cancelFlg);

}

// ================================================================================================.
//! <!--機能-->キャンセル状態設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  b_cancel	：キャンセル状態.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! キャンセル状態を設定する\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
void THRC_BufferResource::setCancelFlg( Bool b_cancel )
{
	THRC_IntLock::lock();

	THRL_Drvrif_Br_SetCancelFlg( getMyId(), b_cancel );

	THRC_IntLock::unlock();
}

// ================================================================================================.
//! <!--機能-->クリアメッセージ取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  e_BufId						：バッファID.
//! <!--戻値-->.
//! \return THRS_MsgMemoryClearInfo*	：クリアメッセージポインタ.
//! <!--詳細-->.
//! 指定バッファがクリア実行中の場合クリアメッセージポインタを返す\n.
//! 他クラスでは共有状態を知らない為、指定バッファIDはクリアを行なうIDを指し、e_MyBufIDは使用しているバッファIDとなる.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
THRS_MsgMemoryClearInfo* THRC_BufferResource::getMemoryClearMsg( THRE_BufId e_BufId )
{
	THRS_MsgMemoryClearInfo* pc_Ans = NULL;

	if( (e_BufId == e_MyBufID) && (s_BufData.b_clearExec == True) ){
		//! 指定バッファIDが自分自身で、クリア実行可能な場合.
		pc_Ans = &s_ClearMsginfo;
	}
	return( pc_Ans );
}

// ================================================================================================.
//! <!--機能-->切り下げ値取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  sl_value	：切り下げ対象値.
//! \param  sl_div		：切り下げ基準値.
//! <!--戻値-->.
//! \return int32_t		：切り下げ結果.
//! <!--詳細-->.
//! 切り下げ処理を行い切り下げ結果を返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
int32_t THRC_BufferResource::kirisage( int32_t sl_value, int32_t sl_div )
{
	register int32_t sl_Ret = ( sl_value / sl_div - (((sl_value%sl_div)<0)?1:0) );	//! * sl_div;.
	return( sl_Ret );
}


#if 0		// 伸張シフト処理変更により未使用.
// ================================================================================================.
//! <!--機能-->伸張時のタイルマップ変換.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  x		：.
//! \param  y		：.
//! \param  z		：.
//! <!--戻値-->.
//! \return long	：.
//! <!--詳細-->.
//! 伸張時のタイルマップへ変換する\n.
//! <!--履歴-->.
//  09/08/07 越野	 	Donau環境から分離して作成.
//  09/09/29 T.Shima	AsicサブメモリHW仕様変更対応.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
long THRC_BufferResource::linearToExpandTile( long x, long y, long w )
{
	//! タイルマップサイズ値取得.
	THRS_TileMapSize	s_TileMapSize;
	memset( &s_TileMapSize, 0x00, sizeof(THRS_TileMapSize) );
	getTileMapSize( &s_TileMapSize );

	register int32_t sl_Offset =  kirisage( y, DEF_TILEMAP_ONEALINE ) * ( w / 8 ) * DEF_TILEMAP_ONEALINE;

	sl_Offset += kirisage( x, s_TileMapSize.us_Width ) * ( s_TileMapSize.us_ByteWidth * s_TileMapSize.us_Hight );
	sl_Offset += kirisage( x % s_TileMapSize.us_Width, DEF_TILEMAP_ONEALINE ) * 8 + (x<0?DEF_TILEMAP_ONEALINE:0);	//! リニアバイトアドレスからタイルマップへアドレス変換（オフセットのみ）.

	return sl_Offset;
}
#endif

// ================================================================================================.
//! <!--機能-->タイルマップ変換.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  x		：.
//! \param  y		：.
//! \param  z		：.
//! <!--戻値-->.
//! \return int32_t	：.
//! <!--詳細-->.
//! タイルマップへ変換する\n.
//! <!--履歴-->.
//  09/08/07 越野	 	Donau環境から分離して作成.
//  09/09/29 T.Shima	AsicサブメモリHW仕様変更対応.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
//! 元の値を超えない切り捨てを行う除算.
//! 10/3 = 3,  -10/3 = -4 (普通に/を使うと-3になる).
static int32_t thDiv( int32_t sl_value, int32_t sl_div )
{
	int32_t sl_res = sl_value / sl_div;
	if( sl_value < 0 && sl_value % sl_div != 0 ) {
		sl_res --;
	}
	return sl_res;
}
//! 除算の余りを求める(thDiv仕様).
static int32_t thMod( int32_t sl_value, int32_t sl_dev )
{
	return sl_value - sl_dev * thDiv( sl_value, sl_dev );
}

int32_t THRC_BufferResource::linearToTile( int32_t sl_x, int32_t sl_y, int32_t sl_w ) const
{
	//! タイルマップサイズ値取得. 
	THRS_TileMapSize		s_TileMapSize;
	memset( &s_TileMapSize, 0x00, sizeof(THRS_TileMapSize) );
	getTileMapSize( &s_TileMapSize );

	return( thDiv(sl_y, s_TileMapSize.us_Hight) * (sl_w / s_TileMapSize.us_Width * s_TileMapSize.us_Aline)
		 + thMod(sl_y, s_TileMapSize.us_Hight) * (s_TileMapSize.us_Width / 8)
		 + thDiv(sl_x, s_TileMapSize.us_Width) * s_TileMapSize.us_Aline
		 + thMod(sl_x, s_TileMapSize.us_Width) / 8 );
}

// ================================================================================================.
//! <!--機能-->リニアアドレス変換.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  sl_x	：伸張開始x.
//! \param  sl_y	：伸張開始y.
//! \param  sl_w	：バッファ横幅.
//! <!--戻値-->.
//! \return long	：アドレスオフセット.
//! <!--詳細-->.
//! リニアアドレスへ変換する\n.
//! <!--履歴-->.
//  16/02/03 Sky 	伸張シフト対応.
// ================================================================================================.
int32_t THRC_BufferResource::linearToAdrOffset( int32_t sl_x, int32_t sl_y, int32_t sl_w ) const
{
	return( sl_y * (sl_w / 8) + sl_x / 8 );
}

// ================================================================================================.
//! <!--機能-->ライン数のログを残す.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param   ushort us_DebugLogMsgNo	ログに残すメッセージのNo.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  12/06/20 仲川	スレッドログ改善対応.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
void THRC_BufferResource::setBufLogToThLog(  ushort us_DebugLogMsgNo )
{
	if( us_debug_showLineNo == 1 ) { //! ライン数ログ有効時.
		THRL_SetThLog( us_DebugLogMsgNo, THRE_DebugLogType_MTCtrLog, 30, True
			, (uintptr_t)THRL_GetBufIdMsg( e_MyBufID )
			, drvrif_io_G_WriteTopLineNo( e_MyBufID )
			, THRL_Drvrif_Br_GetReadLineNum( getMyId() )
			);
	}
}

// ================================================================================================.
//! <!--機能-->バッファID初期化(初期化時のみ使用).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  THRE_BufId			：バッファID.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! バッファIDの初期設定を行い、\n.
//! 設定されたIDからドライバ側のバッファリソースの確保を行う\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
void THRC_BufferResource::initBufId( THRE_BufId e_BufId )
{
	e_MyBufID = e_BufId;

    ul_bufResManId = THRL_Drvrif_Br_ReserveBufResManId( e_MyBufID,
                                                   &s_BufData.c_ReadResource.ul_resComManId,
                                                   &s_BufData.c_WriteResource.ul_resComManId );

}


// ================================================================================================.
//! <!--機能-->バッファ情報初期化(アドレス/バンドライン/回転バッファの設定).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  uintptr_t ul_StartAddr		: バッファ開始アドレス.
//! \param  uint32_t  ul_BandLine		: バンドライン数.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 初期化時にのみ設定するバッファメモリの情報設定を行う\n.
//! <!--履歴-->.
//  09/08/07 	越野	 	Donau環境から分離して作成.
//  09/09/29	T.Shima		AsicサブメモリHW仕様変更対応.
// 2009/10/20 大石 		ドライバioctl()呼び出し時、クラスのメンバ変数(ul_StartBufferAddr)にもセットする処理を追加.
//  12/02/03	戸田		ファイルメモリ分割対応 仮想⇒物理アドレス変換フラグ設定を追加.
//												   引数に仮想⇒物理アドレス変換フラグを追加.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//	15/10/26 Sky システム統合SoC(S800)対応.
//	19/04/16 日立伊藤史. AR1075848 DenebMLKでFA14. アドレス再設定できていなかった.
// ================================================================================================.
void THRC_BufferResource::initBufArea( uintptr_t ul_StartAddr, uint32_t ul_BandLine )
{
	if( isTileMapBuf(e_MyBufID) == True ){														//! タイルマップ.
		ul_StartBufferAddr = toTileMap( ul_StartAddr );
		drvrif_io_S_StartBufferAddr( e_MyBufID, ul_StartBufferAddr );							//! 開始アドレス(タイルマップ).
		s_BufData.ul_BandLine        = ul_BandLine;												//! 1bandのライン数.
	}else{
		ul_StartBufferAddr = _roundup( ul_StartAddr, 1024 ) * 1024;
		drvrif_io_S_StartBufferAddr( e_MyBufID, ul_StartBufferAddr );							//! 開始アドレス.
		s_BufData.ul_BandLine        = ul_BandLine;												//! 1bandのライン数.
	}
}

// ================================================================================================.
//! <!--機能--> タイルマップサイズ値取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param THRS_TileMapSize* ps_TileMapSize		：タイルマップサイズ値格納先ポインタ.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 自バッファIDから、対象のタイルマップサイズの各値(幅、高さなど)を取得 \n.
//! <!--履歴-->.
//  09/09/29	T.Shima		AsicサブメモリHW仕様変更対応：新規作成.
//	10/12/20	大石		Asic30対応.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//  13/03/13 Sky ZeusS対応 Asicごとのサイズ取得をやめる.
// ================================================================================================.
void THRC_BufferResource::getTileMapSize( THRS_TileMapSize* ps_TileMapSize) const
{
	//! タイルマップサイズテーブル.
	static THRS_TileMapSize s_SelTableTileMapSize = {
		THRD_TileMap_Width,			//!< タイルマップサイズ：幅.
		THRD_TileMap_ByteWidth,		//!< タイルマップサイズ：幅(バイト単位).
		THRD_TileMap_Hight,			//!< タイルマップサイズ：高さ.
		THRD_TileMap_Aline			//!< タイルマップサイズ：アライン.
	};

	if( NULL == ps_TileMapSize ){
		THRL_ThAssert( False, "getTileMapSize() >ps_TileMapSize is Null..." , 0, 0, 0, 0, CFA14_DetailCode0x08A9 );
	}

	memcpy( ps_TileMapSize, &s_SelTableTileMapSize, sizeof(THRS_TileMapSize) );
}

// ================================================================================================.
//! <!--機能--> 入出力バッファサイズ値取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param THRS_InOutBufSize* ps_InOutBufSize		：入出力バッファサイズ値格納先ポインタ.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 自バッファIDから、対象の入出力バッファサイズの各値(幅、高さなど)を取得 \n.
//! <!--履歴-->.
//  09/09/29	T.Shima		AsicサブメモリHW仕様変更対応：新規作成.
//	10/12/20	大石		Asic30対応.
//  11/03/25	世古		スレッド管理バッファ横幅の1200dpiJbig伸張での単位の定義を追加.
//							前述定義を、本来のAsicサブメモリHW対応の定義にあわせた場所に移動させたため.
// 2012/11/21 ADS坪井  CQ.377856  MS-LCL_3 THRS_InOutBufSize構造体コンストラクタ対応.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//  13/03/13 Sky ZeusS対応 Asicごとのサイズ取得をやめる.
// 2014/12/08 ADS森本	CQ.506800(Act_504830) AS-MSC_6 A3A4共通化(メモリスケーラビリティ拡張).
// 2019/05/09 Sky OP_BTS-5109 両面混載の相対倍率対応. 
// ================================================================================================.
void THRC_BufferResource::getInOutBufSize( THRS_InOutBufSize* ps_InOutBufSize ) const
{
	//! Asicごとのバッファサイズテーブル.
	//! 13/03/15 Sky ZeusS対応 値はAsic共通.
	//! 今後, 値が異なるAsicが出てきたらテーブルに追加.
	// 2014/12/08 ADS森本	CQ.506800(Act_504830) AS-MSC_6 A3A4共通化(メモリスケーラビリティ拡張).
	static THRS_InOutBufSize  s_SelTableInOutBufSize[] = {
		THRS_InOutBufSize(
			THRD_InBuf_Width_Min,				//!< 入力バッファ幅の最小値.
			THRD_InBuf_Width_Max,				//!< 入力バッファ幅の最大値.
			THRD_OutBuf_Width_Min,				//!< 出力バッファ幅の最小値.
			THRD_OutBuf_Width_Max,				//!< 出力バッファ幅の最大値.
			THRD_ThrBuf_Width_Normal,			//!< スレッド管理バッファ幅の通常値.
			THRD_ThrBuf_Width_Normal_1200dpi,	//!< スレッド管理バッファ横幅の1200dpi(多値非圧縮)での単位(dot).
			THRD_OutBuf_Width_Max_Other			//!< 出力バッファ幅の最大値.
		),
#ifdef DEF_A3A4COMM // 2015/02/16 ADS森本 CQ.510156(Act_507195) 【A3A4共通化】コンパイルスイッチ対応(機能/メモリスケーラビリティ拡張分).
		// A4機用バッファ定義.
		THRS_InOutBufSize(
			THRD_InBuf_Width_Min,				//!< 入力バッファ幅の最小値.
			THRD_InBuf_Width_A4_Max,			//!< 入力バッファ幅の最大値(A4機).
			THRD_OutBuf_Width_Min,				//!< 出力バッファ幅の最小値.
			THRD_OutBuf_Width_A4_Max,			//!< 出力バッファ幅の最大値(A4機).
			THRD_ThrBuf_Width_A4_Normal,		//!< スレッド管理バッファ幅の通常値(A4機).
			THRD_ThrBuf_Width_Normal_1200dpi	//!< スレッド管理バッファ横幅の1200dpi(多値非圧縮)での単位(dot).
		),
#endif // #ifdef DEF_A3A4COMM.
	};

	if( NULL == ps_InOutBufSize ){
		THRL_ThAssert( False, "getInOutBufSize() >ps_InOutBufSize is Null..." , 0, 0, 0, 0, CFA14_DetailCode0x08AB );
	}
#ifdef DEF_A3A4COMM // 2015/02/16 ADS森本 CQ.510156(Act_507195) 【A3A4共通化】コンパイルスイッチ対応(機能/メモリスケーラビリティ拡張分).
	// 2014/12/08 ADS森本	CQ.506800(Act_504830) AS-MSC_6 A3A4共通化(メモリスケーラビリティ拡張).
	uchar	Index = 0;	// A3機種.
	uchar	IndexMax = sizeof(s_SelTableInOutBufSize) / sizeof(THRS_InOutBufSize);	// 範囲数取得.
	if( TYPE_CapacityA4 == APIC_OptionSetting::newInstance()->getCapacity() ){	// 機種 A3A4判定.
		Index = 1;	// A4機種の場合.
	}
	// 範囲チェック.
	THRL_ThAssert( Index < IndexMax, "getInOutBufSize() >A3A4 Kind invalid...(e_MyBufID:%d)", e_MyBufID , 0, 0, 0, CFA14_DetailCode0x0C8D );
	/* 2016/06/23 Klocwork #58042 対応 */
	ps_InOutBufSize->copy(s_SelTableInOutBufSize[Index]);
	
/*
	memcpy( ps_InOutBufSize, &s_SelTableInOutBufSize[Index], sizeof(THRS_InOutBufSize) );
*/

#else // #ifdef DEF_A3A4COMM.
	memcpy( ps_InOutBufSize, &s_SelTableInOutBufSize[0], sizeof(THRS_InOutBufSize) );
#endif // #ifdef DEF_A3A4COMM.
}

// ================================================================================================.
//! <!--機能--> タイルマップアドレス変換.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param	uintptr_t	ul_Addr			：リニアアドレス値.
//! <!--戻値-->.
//! \return	uintptr_t					：タイルマップアドレス値.
//! <!--詳細-->.
//! リニアアドレス値からタイルマップアドレス値に変換 \n.
//! <!--履歴-->.
//  09/09/29	T.Shima		AsicサブメモリHW仕様変更対応：新規作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
uintptr_t THRC_BufferResource::toTileMap( uintptr_t ul_Addr ) const
{
	uintptr_t				ul_RetAddr = 0;
	THRS_TileMapSize	s_TileMapSize;			//!< タイルマップサイズ値取得用.

	memset( &s_TileMapSize, 0x00, sizeof(THRS_TileMapSize) );

	//! タイルマップサイズ値取得.
	getTileMapSize( &s_TileMapSize );

	//! リニアアドレス→タイルマップアドレス変換.
	ul_RetAddr = _roundup( ul_Addr, s_TileMapSize.us_Aline ) * s_TileMapSize.us_Aline;

	return( ul_RetAddr );
}
// ================================================================================================.
//! <!--機能--> メモリクリア状態の取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param	void					：なし.
//! <!--戻値-->.
//! \return	Bool					：メモリクリア状態.
//! <!--詳細-->.
//! メモリクリア状態を取得する \n.
//! <!--履歴-->.
//  17/07/22	simogaki		新規作成 リソース確保済みスレッドで実行開始確認(chkTransExec)がOKとなり.
//  							該当のサブメモリ領域をクリア中に画像転送が開始され、FA14発生してしまう不具合修正のため追加.
// ================================================================================================.
Bool THRC_BufferResource::getBufMemClrStatus( void ){
	return s_BufData.b_clearExec;
}

// ================================================================================================.
//! <!--機能--> 制御JAM時の管理情報クリア.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param	THRE_CtrID				：画像制御部(高速タスク)ID.
//! <!--戻値-->.
//! \return							：なし.
//! <!--詳細-->.
//! バッファの管理情報をクリアする \n.
//! <!--履歴-->.
// 2020/06/04 Sky		FA17発生時にエンジン制御JAMにする対応.
// ================================================================================================.
void THRC_BufferResource::clearBuf_forCtrlJAM( THRE_CtrID e_CtrID )
{
	s_BufData.us_WritePage = 0;
	s_BufData.us_ReadPage  = 0;
	s_BufData.ul_JobID = 0;
	s_BufData.uc_ThrIdx = THRD_MaxThreadBusy;	
	drvrif_io_S_WriteTopLineNo( e_MyBufID, 0 );
	drvrif_io_S_ReadTopLineNo( e_MyBufID, 0 );
	drvrif_io_S_BufFull( e_MyBufID, False );

	SYSF_logMsg("(%010u)clearBuf_forCtrlJAM e_MyBufID=%d, e_CtrID=%d\n", SYSF_tickGet(), e_MyBufID, e_CtrID);
	THRL_SetThLog( 743, THRE_DebugLogType_ThCtrLog, 0, True, (uintptr_t)e_MyBufID, (uintptr_t)e_CtrID, 0, 0, 0, 0 );

	// キャンセルクリアが動作していない.
	if( s_BufData.b_clearExec == False )
	{
		// バッファクリア要求.
		s_ClearMsginfo.ev_ReqThID = THRE_ThrID_None;								//! 要求元ID設定.
		s_ClearMsginfo.e_CtrID    = e_CtrID;										//! 画像制御部(高速タスク)ID設定.
		s_ClearMsginfo.e_DMAID    = THRE_DmaId_MemClr;								//! Asicの1/2/3枚目は意識してない。CtrIDで振り分ける.
		s_ClearMsginfo.s_ClrBufInfo.pc_BufResMan = this;							//! バッファリソース管理を設定.
		s_ClearMsginfo.s_ClrBufInfo.ul_StartLine = 0;								//! 消去先端ライン.
		s_ClearMsginfo.s_ClrBufInfo.ul_EndLine   = getBufLineNum();					//! 消去終端ライン.

		setClearReq();
		s_BufData.b_clearExec = True;
		comMultiThreadProfile.MemClrReq( &s_ClearMsginfo );
	}
}

// ================================================================================================.
//! <!--機能-->コンストラクタ.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! メンバーの初期化を行う\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  12/11/21 ADS坪井 CQ.377856  MS-LCL_3 THRC_BufferResourceインスタンス動的確保変更に伴う修正.
//  13/01/28 ADS川口 CQ.377863  MS-LCL_23 メモリクリア情報(SDRAM)の動的確保化.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//  2013/07/12 ADS滕 CQ.381171(Act_66426) SC-SFM-23 サブメモリのファイルメモリ化.
//! 15/10/19	Sky	システム統合SoC(S800)対応.
// ================================================================================================.
THRC_BufMemory::THRC_BufMemory()
{
	ul_PhysicalMemSize = 0;										//! サブメモリ 合計 物理メモリサイズ(Byte)初期化.
	uc_MemoryClrCnt = 0;										//! メモリ初期化回数カウンター初期化.
	uc_MemoryClrCntPci = 0;										//! メモリクリアカウンター初期化.
	b_MemoryClear = False;										//! メモリクリアフラグ初期化.
	//! サブメモリ 各Slotごとの物理メモリサイズ(Byte)初期化.
	uchar uc_posx = 0;
	uchar uc_posy = 0;
	for( uc_posx = 0; uc_posx < THRD_SubMem_AsicNum; uc_posx++ ){
		for( uc_posy = 0; uc_posy < THRD_SubMem_SlotNum; uc_posy++ ){
			ul_OneSubMemorySize[uc_posx][uc_posy] = 0;
		}
	}
#ifdef DEF_Soc1200
	ul_SubFileMemoryStartAddr = 0;								//! サブファイルメモリの開始アドレス初期化.
	ul_SubFileMemorySize = 0;									//! サブファイルメモリのサイズ初期化.
#endif
	//! バッファリソース情報テーブルの初期化.
	for( uc_posx = 0; uc_posx < THRE_BufId_Num; uc_posx++ ){
		pc_BufResMan[uc_posx] = NULL;
	}
	//! クリアメッセージデータ初期化.
	for( uc_posx = 0; uc_posx < THRD_MSGDATA_MAIN_NUM; uc_posx++ ){
		memset( &s_MsgClearInfoPci[uc_posx], 0, sizeof(THRS_MsgMemoryClearInfo) );
	}
	for( uc_posx = 0; uc_posx < THRD_SubMem_AsicNum; uc_posx++ ){
		for( uc_posy = 0; uc_posy < THRD_SubMem_SlotNum; uc_posy++ ){
			ps_MsgClearInfoSdram[uc_posx][uc_posy] = NULL;
		}
	}
}


// ================================================================================================.
//! <!--機能-->デストラクタ.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  12/11/21 ADS坪井 CQ.377856  MS-LCL_3 THRC_BufferResourceインスタンス動的確保変更に伴う修正.
//  13/01/28 ADS川口 CQ.377863  MS-LCL_23 メモリクリア情報(SDRAM)の動的確保化.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
THRC_BufMemory::~THRC_BufMemory()
{
	// メモリクリア情報(SDRAM用)の領域解放.
	for( uchar uc_posx = 0; uc_posx < THRD_SubMem_AsicNum; uc_posx++ ){
		for( uchar uc_posy = 0; uc_posy < THRD_SubMem_SlotNum; uc_posy++ ){
			if( ps_MsgClearInfoSdram[uc_posx][uc_posy] != NULL )
			{
				delete ps_MsgClearInfoSdram[uc_posx][uc_posy];
				ps_MsgClearInfoSdram[uc_posx][uc_posy] = NULL;
			}
		}
	}

	//! バッファリソース情報の解放.
	for( uc_posx = 0; uc_posx < THRE_BufId_Num; uc_posx++ ){
		if(pc_BufResMan[uc_posx] != NULL){ delete pc_BufResMan[uc_posx]; }
		pc_BufResMan[uc_posx] = NULL;
	}
}


// ================================================================================================.
//! <!--機能-->初期化処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! SDRAMのサイズチェック及びバッファ情報の設定を行う\n.
//! <!--履歴-->.
//  09/08/07 越野	 	Donau環境から分離して作成.
//  08/08/11 R.Suzuki	CQ.309145 1Pass対応 3枚目の正式対応.
//	09/09/29 T.Shima	AsicサブメモリHW仕様変更対応.
//	2009/10/02 岡田		Asicサブメモリリソース対応.
//	10/12/20 大石		Asic30対応.
//	13/01/28 ADS川口	CQ.377863  MS-LCL_23 メモリクリア情報(SDRAM用)の領域確保を追加.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//  13/03/13 Sky ZeusS対応.
//  15/10/06 Sky		システム統合SoC(S800)対応.
//  15/10/22 大岡		[ZeusSZX0]制御構成変更No.76-1 新詳細コードデータ収集機能.
//  16/10/25 Sky合屋    [IT5_4.1]制御構成変更No.40 自動評価対応.
//  17/04/07 Sky		ZeusSMLKPF対応.
//  17/06/13 Sky OP/PP共通化.
// ================================================================================================.
void THRC_BufMemory::init()
{
	//! サブメモリのメモリサイズチェック.
	Bool	b_TypeOK = False;
	uint32_t	ul_Size0 = 0;
	uint32_t	ul_Size1 = 0;
	uint32_t	ul_Size2 = 0;
	uint32_t	ul_Size3 = 0;
	uint32_t	ul_Size4 = 0;
	uint32_t	ul_Size5 = 0;
	uint32_t	ul_Size6 = 0;	// AsicPMS追加 13/03/13 Sky ZeusS対応.
	uint32_t	ul_Size7 = 0;	// AsicPMS追加 13/03/13 Sky ZeusS対応.
	uint32_t	ul_Size8 = 0;	// Asic30(Fiery 2個目)追加 13/03/13 Sky ZeusS対応.
	uint32_t	ul_Size9 = 0;	// Asic30(Fiery 2個目)追加 13/03/13 Sky ZeusS対応.
	//! ASIC搭載数チェック.
	Bool	b_isAsic_0Exist = THRF_IsAsic_0Exist();
	Bool	b_isAsic_1Exist = THRF_IsAsic_1Exist();
	Bool	b_isAsic_2Exist = THRF_IsAsic_2Exist();
	Bool	b_isAsic_3Exist = THRF_IsAsic_3Exist();		// AsicPMS追加 13/03/13 Sky ZeusS対応.
	Bool	b_isAsic_4Exist = THRF_IsAsic_4Exist();		// Asic30(Fiery 2個目)追加 13/03/13 Sky ZeusS対応.

	if( b_isAsic_0Exist == False ){
		//! ASICなしでも初期起動はする。.
		return;
	}

	if( b_isAsic_0Exist == True ){
		//! SDRAM0のサイズチェック.
		ul_Size0 = THRF_GetDIMMSizeBySPD( THRE_SPD_DEVICE_0, b_TypeOK );
		//! SDRAM1のサイズチェック.
		ul_Size1 = THRF_GetDIMMSizeBySPD( THRE_SPD_DEVICE_1, b_TypeOK );
	}

	if( b_isAsic_1Exist == True ){
		//! Asic 2個目搭載時.
		//! SDRAM2のサイズチェック.
		ul_Size2 = THRF_GetDIMMSizeBySPD( THRE_SPD_DEVICE_2, b_TypeOK );
		//! SDRAM3のサイズチェック.
		ul_Size3 = THRF_GetDIMMSizeBySPD( THRE_SPD_DEVICE_3, b_TypeOK );
	}

	if( b_isAsic_2Exist == True ){
		//! Asic 3個目搭載時.
		//! SDRAM4のサイズチェック.
		ul_Size4 = THRF_GetDIMMSizeBySPD( THRE_SPD_DEVICE_4, b_TypeOK );
		//! SDRAM5のサイズチェック.
		ul_Size5 = THRF_GetDIMMSizeBySPD( THRE_SPD_DEVICE_5, b_TypeOK );
	}
	
	if( b_isAsic_3Exist == True ){		// Asic 4枚目追加 13/03/13 Sky ZeusS対応.
		//! Asic 4個目搭載時.
		//! SDRAM6のサイズチェック.
		ul_Size6 = THRF_GetDIMMSizeBySPD( THRE_SPD_DEVICE_6, b_TypeOK );
		//! SDRAM7のサイズチェック.
		ul_Size7 = THRF_GetDIMMSizeBySPD( THRE_SPD_DEVICE_7, b_TypeOK );
	}

	if( b_isAsic_4Exist == True ){		// Asic 5枚目追加 13/03/13 Sky ZeusS対応.
		//! Asic 5個目搭載時.
		//! SDRAM8のサイズチェック.
		ul_Size8 = THRF_GetDIMMSizeBySPD( THRE_SPD_DEVICE_8, b_TypeOK );
	}

	//メンバ変数へサブメモリ 各Slotごとの物理メモリサイズ(Byte) 設定.
	ul_OneSubMemorySize[ 0 ][ 0 ] = ul_Size0 * 1024 * 1024;
	ul_OneSubMemorySize[ 0 ][ 1 ] = ul_Size1 * 1024 * 1024;
	ul_OneSubMemorySize[ 1 ][ 0 ] = ul_Size2 * 1024 * 1024;
	ul_OneSubMemorySize[ 1 ][ 1 ] = ul_Size3 * 1024 * 1024;
	ul_OneSubMemorySize[ 2 ][ 0 ] = ul_Size4 * 1024 * 1024;
	ul_OneSubMemorySize[ 2 ][ 1 ] = ul_Size5 * 1024 * 1024;
	ul_OneSubMemorySize[ 3 ][ 0 ] = ul_Size6 * 1024 * 1024;
	ul_OneSubMemorySize[ 3 ][ 1 ] = ul_Size7 * 1024 * 1024;
	ul_OneSubMemorySize[ 4 ][ 0 ] = ul_Size8 * 1024 * 1024;
	ul_OneSubMemorySize[ 4 ][ 1 ] = ul_Size9 * 1024 * 1024;
	//! サブメモリ 合計 物理メモリサイズ(Byte).
	ul_PhysicalMemSize = (ul_Size0 + ul_Size1 + ul_Size2 + ul_Size3 + ul_Size4 + ul_Size5 + ul_Size6 + ul_Size7 + ul_Size8 + ul_Size9 ) * 1024 * 1024;

#if defined(COSIM) && defined(THR_SIMWIN)	// 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.
#else
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();	//! システム設定（ＡＰＩ）・インスタンス取得.
	if( (pc_SystemInfo->getExternalIFMode() == TYPE_ExternalIFMode1) && (THRF_IsFieryOption() == False) ) {
		// 新詳細コード保存.
		uint64_t		ull_NewDetailData = THRD_NewDetail_ModuleId_Thread;					// 1,2桁：スレッド.
		ull_NewDetailData |= (uint64_t)( TYPD_TroubleCode_FATAL_FIERY_NONE ) << THRD_NewDetail_TroubleCode_Shift;	// 3-6桁：トラブルコード.
		ull_NewDetailData |= (uint64_t)( pc_SystemInfo->getExternalIFMode() ) << 32;		// 7,8桁：外部I/F接続設定.
		ull_NewDetailData |= (uint64_t)( THRF_IsFieryOption() ) << 24;					// 9,10桁：Fiery用ASIC接続状態.
		THRL_ThAssertLog2( 0, ull_NewDetailData );
		//! 外部コントローラ接続されているのにFiery基板が装着されていないとき.
		DebugForever( TYPD_TroubleCode_FATAL_FIERY_NONE );
	}
#endif // #if defined(COSIM) && defined(THR_SIMWIN) // 【COSIM+ｽﾚｯﾄﾞSIMWIN対応】.

	//! サブメモリ上のバッファ初期化処理.
#if 0 // Asic1枚目(S800)はSDRAMサブメモリを搭載しないため初期化しない.
	if( b_isAsic_0Exist == True ){
		init_AsicSdram_1();
	}
#endif
	if( b_isAsic_1Exist == True ){
		init_AsicSdram_2();
	}
	if( b_isAsic_2Exist == True ){
		init_AsicSdram_3();
	}
	if( b_isAsic_3Exist == True ){		// Asic 4枚目追加 13/03/13 Sky ZeusS対応.
		init_AsicSdram_4();
	}
	if( b_isAsic_4Exist == True ){		// Asic 5枚目追加 13/03/13 Sky ZeusS対応.
		init_AsicSdram_5();
	}

	//! スレッド使用分のPCIメモリ上のメモリの初期化.
	init_ThreadUsePciMode();

#ifndef DEF_PP_Cus
	// OP機既存処理.
	// メモリクリア情報(SDRAM用)の領域確保.
	for( uchar uc_posx = 0; uc_posx < THRD_SubMem_AsicNum; uc_posx++ ){
		for( uchar uc_posy = 0; uc_posy < THRD_SubMem_SlotNum; uc_posy++ ){
			if( (uc_posx == 1 && b_isAsic_1Exist == True) ||
				(uc_posx == 2 && b_isAsic_2Exist == True) || 
				(uc_posx == 3 && b_isAsic_3Exist == True) ||
				(uc_posx == 4 && b_isAsic_4Exist == True) ){
				ps_MsgClearInfoSdram[uc_posx][uc_posy] = new THRS_MsgMemoryClearInfo;
				THRL_ThAssert( (ps_MsgClearInfoSdram[uc_posx][uc_posy] != NULL), "can't allocate ps_MsgClearInfoSdram[%d][%d]!!", uc_posx, uc_posy, 0, 0, CFA14_DetailCode0x0B83);
				memset( ps_MsgClearInfoSdram[uc_posx][uc_posy], 0, sizeof(THRS_MsgMemoryClearInfo) );
			}
		}
	}
	// DipSW(自動評価)の値を取得して設定.
	Bool bl_IsAutoTestMode = APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( 195, 0 );
	if( True == bl_IsAutoTestMode ) {
		THRL_SetAutoTestMode( True );
	}
#else // DEF_PP_Cus.
	// OP/PP共通化.
	// メモリクリア情報(SDRAM用)の領域確保.
	// PP機でSDRAM(n)のサイズチェック結果を追加条件として利用する。サブメモリ上スレッドの利用する領域はない場合、実処理をSKIPする.
	for( uchar uc_posx = 0; uc_posx < THRD_SubMem_AsicNum; uc_posx++ ){
		for( uchar uc_posy = 0; uc_posy < THRD_SubMem_SlotNum; uc_posy++ ){
			if( (uc_posx == 1 && b_isAsic_1Exist == True && (ul_Size2 > 0 || ul_Size3 > 0) ) ||
				(uc_posx == 2 && b_isAsic_2Exist == True && (ul_Size4 > 0 || ul_Size5 > 0) ) || 
				(uc_posx == 3 && b_isAsic_3Exist == True && (ul_Size6 > 0 || ul_Size7 > 0) ) ||
				(uc_posx == 4 && b_isAsic_4Exist == True && ul_Size8 > 0  ) ){
				ps_MsgClearInfoSdram[uc_posx][uc_posy] = new THRS_MsgMemoryClearInfo;
				THRL_ThAssert( (ps_MsgClearInfoSdram[uc_posx][uc_posy] != NULL), "can't allocate ps_MsgClearInfoSdram[%d][%d]!!", uc_posx, uc_posy, 0, 0, CFA14_DetailCode0x0B83);
				memset( ps_MsgClearInfoSdram[uc_posx][uc_posy], 0, sizeof(THRS_MsgMemoryClearInfo) );
			}
		}
	}
#endif // DEF_PP_Cus.
	
}


// ================================================================================================.
//! <!--機能-->スレッド管理分のメモリ全クリア処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! スレッド管理のPCI及びSDRAMの全クリアを行う\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  10/09/24 世古	 FPGA時もクリア処理を行う(CQ:xxxxxx).
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//  2013/03/15 【MP-BIM-01】日立JTE 古川 文字印字、スタンプオプション化対応.オプション化設定が無効の場合クリアしたことにする.
//  2013/04/08 酒井(日立JTE)   CQ.378970 文字印字・スタンプオプション化の機能マスクをSettingからStatusに変更.
//  2013/04/11 CQ379160 伊藤史. BufImageを文字スタンプ機能OFFでも確保する対応.メインバッファクリア要求のみマスク.
//  2019/09/17 Sky井柳  [SparrowBK][OP_BTS-13657] スタンプオプション指定のログ出力追加.
//  2019/09/27 Sky合屋 【SparrowBK】IT6機種は、スタンプオプション化設定にかかわらず、Subメモリクリアを実施するように修正する.
// ================================================================================================.
void THRC_BufMemory::AllClearMemory()
{
#if	defined(COSIM)
	extern uint32_t getCosimTestNo( void );
	uint32_t ul_CosimTestNo = getCosimTestNo();
	if( b_memClrFlg == True ){
		if( THRF_IsAsic_0Exist() == True ) {
		//! 07/07/04 morikaw ASICが存在する時のみメモリクリヤを実行する.
			//! Subメモリクリア.
			if( ul_CosimTestNo != 80000 ) {
				THRC_BufMemorySingletonIf::newInstance().AllClearSDRAM();
				comMultiThreadProfile.setPciBufClrState( EN_PciBufClrComplete );//! PCI上のバッファメモリ初期化クリア完了とする.
			}else{
				THRC_BufMemorySingletonIf::newInstance().AllClearPCI();
				uc_MemoryClrCnt++;
				clearComplete();
			}
		}
	}else{
		//! COSIMではクリアしたことにする。.
		comMultiThreadProfile.setPciBufClrState( EN_PciBufClrComplete );//! PCI上のバッファメモリ初期化クリア完了とする.
		uc_MemoryClrCnt++;
		clearComplete();
	}
#else
//! 基板立ち上げチェック.
//	if( THRF_IsAsic_0Exist() == True ) {
	//! ASICが存在する時のみメモリクリヤを実行する.
		//! Subメモリクリア.
		THRC_BufMemorySingletonIf::newInstance().AllClearSDRAM();
		//2013/03/15 【MP-BIM-01】日立JTE 古川 文字印字、スタンプオプション化設定が無効の場合もクリアしたことにする.
		//2013/04/11 CQ379160 伊藤史. メインバッファクリア要求のみマスク.
		//IT6機種は、スタンプオプション化設定にかかわらず、Subメモリクリアを実施する.
		//! メインバッファクリア要求.
		THRC_BufMemorySingletonIf::newInstance().AllClearPCI();
		uc_MemoryClrCnt++;
//	} else {
//		//! COSIMでない時もASICがない時はクリアしたことにする。.
//		comMultiThreadProfile.setPciBufClrState( EN_PciBufClrComplete );//! PCI上のバッファメモリ初期化クリア完了とする.
//		//! 終了シーケンスを通すために、回数をインクリメント.
//		++uc_MemoryClrCnt;
//		//! 定期起床を起動するためには、メモリクリヤ完了メッセージが必要.
//		sendMessage( TID_ThreadManager, TM_ImageClearEndNotice, 0 );
//	}
#endif
}

// ================================================================================================.
//! <!--機能-->スレッド管理のPCI全クリア処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! スレッド管理のPCI全クリアを行う\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 2010/12/13 tadamasa	Fiery1200dpi対応.
//   13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//   13/03/13 Sky ZeusS対応.
//   14/02/03 Sky ASIC777DS対応.
//   14/09/30 Sky Asic2014対応.
// 2014/12/08 ADS森本	CQ.506800(Act_504830) AS-MSC_6 A3A4共通化(メモリスケーラビリティ拡張).
// 2015/10/06 Sky システム統合SoC(S800)対応.
// 2017/04/07 Sky ZeusSMLKPF対応.
// 2017/05/01 日立 OP/PP共通化.
// ================================================================================================.
void THRC_BufMemory::AllClearPCI()
{
#if 0
//#ifdef DEF_A3A4COMM // 2015/02/16 ADS森本 CQ.510156(Act_507195) 【A3A4共通化】コンパイルスイッチ対応(機能/メモリスケーラビリティ拡張分).
	// 2014/12/08 ADS森本	CQ.506800(Act_504830) AS-MSC_6 A3A4共通化(メモリスケーラビリティ拡張).
	// 機能オプション対応にて切り替え.
	if(True == SYSF_getAppOptionMemStatus(TYPE_AppOptionStamp)){	//文字印字スタンプオプション化設定が有効.

#ifndef COSIM
		//! メインバッファクリア要求.
		ReqClearPCIBuf( THRE_ThrID_None, THRE_BufId_Main, THRE_CtrID_0);
#endif // COSIM.
		//! 合成画像用領域クリア要求.
		ReqClearPCIBuf( THRE_ThrID_None, THRE_BufId_Main_Combine, THRE_CtrID_0);
#ifndef COSIM
		//! 合成画像用領域（地紋、ウォーターマーク用）クリア要求.
		ReqClearPCIBuf( THRE_ThrID_None, THRE_BufId_Main_CombineJimon, THRE_CtrID_0);

		//! 合成画像用領域（回転用）クリア要求.
		ReqClearPCIBuf( THRE_ThrID_None, THRE_BufId_Main_Rot, THRE_CtrID_0);

		//! プレビュー用領域（回転用）クリア要求.
		ReqClearPCIBuf( THRE_ThrID_None, THRE_BufId_Main_Preview, THRE_CtrID_0);
#endif // COSIM.

	// 2012/04/10 Gonsho DS基板JPEGコーダ対応 Start.
		//! Android JPEG圧縮用バッファ クリア要求.
		ReqClearPCIBuf( THRE_ThrID_None, THRE_BufId_Main_AndroidJpegCompIn, THRE_CtrID_0);	

		//! Android JPEG圧縮用バッファ クリア要求.
		ReqClearPCIBuf( THRE_ThrID_None, THRE_BufId_Main_AndroidJpegCompOut, THRE_CtrID_0);
	// 2012/04/10 Gonsho DS基板JPEGコーダ対応 End.
	}
	else{	//文字印字スタンプオプション化設定が無効.
#ifndef COSIM
		//! メインバッファクリア要求.
		ReqClearPCIBuf( THRE_ThrID_None, THRE_BufId_Main, THRE_CtrID_0);
#endif
		//! Android JPEG圧縮用バッファ クリア要求.
		ReqClearPCIBuf( THRE_ThrID_None, THRE_BufId_Main_AndroidJpegCompIn, THRE_CtrID_0);

		//! Android JPEG圧縮用バッファ クリア要求.
		ReqClearPCIBuf( THRE_ThrID_None, THRE_BufId_Main_AndroidJpegCompOut, THRE_CtrID_0);
	}

	if( True == SYSF_getAppOptionStatus(TYPE_AppOptionSubFileMem) ) {	// 一時バッファが有効時.
		ReqClearPCIBuf( THRE_ThrID_None, THRE_BufId_Main_TmpEncode, THRE_CtrID_0);
		ReqClearPCIBuf( THRE_ThrID_None, THRE_BufId_Main_TmpDecode, THRE_CtrID_0);
	}

#else // #ifdef DEF_A3A4COMM.

	// COSIMのバッファクリア
	// S800ではBufImage全てをバッファクリアするため、不要なバッファを除く場合は修正が必要.

	THRS_MsgMemoryClearInfo& s_ClrMsg = s_MsgClearInfoPci[uc_MemoryClrCntPci];
	uc_MemoryClrCntPci++;

	if(uc_MemoryClrCntPci >= THRD_MSGDATA_MAIN_NUM){
		uc_MemoryClrCntPci = 0;
	}
	memset( &s_ClrMsg.s_PciAdrInfo, 0, sizeof(THRS_BufAdrInfo) );	//! 使用しない転送先情報を初期化.
	s_ClrMsg.s_SdramAdrInfo.ul_RingBufferAddr = (uintptr_t)THRF_GetThreadBufferTopAdrOnMainMemory();	//! BufImageの先頭アドレス取得.
	s_ClrMsg.ul_BandSize = GetMemPartSizeFromPartID( memID_BufImage) - THRD_MARGIN_PART;		//! BufImageサイズ管理情報のマージンを考慮.

	s_ClrMsg.e_CtrID = THRE_CtrID_0;
	s_ClrMsg.e_DMAID = THRE_DmaId_MemClr;
	s_ClrMsg.s_ClrBufInfo.pc_BufResMan = NULL;						//! リソース管理クラスを経由しない.

#if defined(DEF_S800)
	if( thePowerManagerTask.isJpgHddAsic1200TRInitBusy() == True ){	// スリープからの復帰.
		s_ClrMsg.ev_ReqThID = TID_PowerManeger;
	} else {
		s_ClrMsg.ev_ReqThID = TID_ThreadManager;
	}
#else /* defined(DEF_S800) */
	s_ClrMsg.ev_ReqThID = TID_ThreadManager;
#endif /* defined(DEF_S800) */

	s_ClrMsg.b_OutputSdram = False;
	s_ClrMsg.b_OutputPci = True;
	s_ClrMsg.b_1200Mode = False;
	s_ClrMsg.s_BufImgInfo.uc_MaxColorNum = 1;
	s_ClrMsg.s_BufImgInfo.uc_AllPlaneNum = 1;
	s_ClrMsg.s_BufImgInfo.uc_MaxAttBitNum = 0;
	s_ClrMsg.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_Density;
	s_ClrMsg.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_1bit;
	// OP/PP共通化. 
	s_ClrMsg.s_BufImgInfo.e_ImageCompress = IMGE_ImageCompress_BTCxxxAndJBIG;

	s_ClrMsg.s_BufImgInfo.ul_DstImageWidth  = DEF_IMAGEWIDTH_MAX;	// S800はレジスタ仕様上、横幅16384まで指定可能.
	s_ClrMsg.s_BufImgInfo.ul_DstImageHeight = s_ClrMsg.ul_BandSize / (s_ClrMsg.s_BufImgInfo.ul_DstImageWidth / 8);	// ul_BandSizeを8倍するとオーバーフローするので計算方法変更.
	
	if( s_ClrMsg.s_BufImgInfo.ul_DstImageHeight > 0xffff ){
		//! 桁あふれ時の対応.
		while( s_ClrMsg.s_BufImgInfo.ul_DstImageHeight > 0xffff ){
			//! ショートを超えるとまずいので、プレーン数を増やしてライン数を減らす.
			if( s_ClrMsg.s_BufImgInfo.uc_MaxColorNum < 4 ){
				//! 4色まではカラーで頑張る.
				s_ClrMsg.s_BufImgInfo.uc_MaxColorNum = (uchar)(s_ClrMsg.s_BufImgInfo.uc_MaxColorNum * 2);
			} else {
				//! カラー一杯なら、属性ビットを利用する.
				THRL_ThAssert( (s_ClrMsg.s_BufImgInfo.uc_MaxAttBitNum == 0), "ClearSizeOver!" , 0, 0, 0, 0, CFA14_DetailCode0x0FE3 );
				s_ClrMsg.s_BufImgInfo.uc_MaxAttBitNum = s_ClrMsg.s_BufImgInfo.uc_MaxColorNum;
				s_ClrMsg.s_BufImgInfo.uc_MaxColorNum++;
			}
			s_ClrMsg.s_BufImgInfo.uc_AllPlaneNum = s_ClrMsg.s_BufImgInfo.uc_MaxColorNum;
			if( s_ClrMsg.s_BufImgInfo.uc_MaxAttBitNum > 0 ){
				s_ClrMsg.s_BufImgInfo.uc_AllPlaneNum = (uchar)(s_ClrMsg.s_BufImgInfo.uc_AllPlaneNum + (s_ClrMsg.s_BufImgInfo.uc_MaxAttBitNum - 1));
			}
			s_ClrMsg.s_BufImgInfo.ul_DstImageHeight = s_ClrMsg.s_BufImgInfo.ul_DstImageHeight / 2;
		}
	}

	s_ClrMsg.s_BufImgInfo.ul_SrcImageWidth = 0;
	s_ClrMsg.s_BufImgInfo.ul_SrcImageHeight = 0;
	s_ClrMsg.s_BufImgInfo.uc_TopBandNo = 0;
	s_ClrMsg.s_BufImgInfo.us_alignWidth = 0;

	s_ClrMsg.s_SdramAdrInfo.ul_RingBufferOffAddr = s_ClrMsg.ul_BandSize / s_ClrMsg.s_BufImgInfo.uc_AllPlaneNum;
	s_ClrMsg.s_SdramAdrInfo.us_RingBufferMaxPlaneNum = 1;
	s_ClrMsg.s_SdramAdrInfo.us_RingBufferWidth = (ushort)s_ClrMsg.s_BufImgInfo.ul_DstImageWidth;
	s_ClrMsg.s_SdramAdrInfo.us_RingBufferHeight = (ushort)s_ClrMsg.s_BufImgInfo.ul_DstImageHeight;
	s_ClrMsg.s_SdramAdrInfo.us_DataWidth = (ushort)s_ClrMsg.s_BufImgInfo.ul_DstImageWidth;
	s_ClrMsg.s_SdramAdrInfo.us_DataHeight = (ushort)s_ClrMsg.s_BufImgInfo.ul_DstImageHeight;
	s_ClrMsg.s_SdramAdrInfo.us_RingImageTopLineNo = 0;
	s_ClrMsg.s_SdramAdrInfo.ul_RingImageTopAddr = s_ClrMsg.s_SdramAdrInfo.ul_RingBufferAddr;
	s_ClrMsg.s_SdramAdrInfo.e_BufType = THRE_BufType_Main;

	comMultiThreadProfile.setPciBufClrState( EN_PciBufClrBusy );	//! PCI上のバッファメモリクリア中とする.

	comMultiThreadProfile.MemClrReq( &s_ClrMsg );

#endif // #ifdef DEF_A3A4COMM.
}


// ================================================================================================.
//! <!--機能-->クリアメッセージを設定して要求を行なう.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  THRE_ThrID			：スレッドID.
//! \param  THRE_BufId			：バッファID.
//! \param  THRE_CtrID			：高速タスクID.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! クリアメッセージを設定して要求を行なう\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  10/01/12 karasaki    CQ323312 クリアする画像ライン数の計算時にプレーン数を考慮するよう修正.
// 2010/12/13 tadamasa	Fiery1200dpi対応.
// 2012/02/03 戸田		ファイルメモリ分割対応.
// 2012/11/21 ADS坪井 CQ.377856  MS-LCL_3 THRC_BufferResourceインスタンス動的確保変更に伴う修正.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//  13/03/13 Sky ZeusS対応 不要バッファに対する処理を削除.
//  17/04/28 日立 OP/PP共通化.
// ================================================================================================.
void THRC_BufMemory::ReqClearPCIBuf( THRE_ThrID e_ThrId, THRE_BufId e_BufId, THRE_CtrID e_CtrId )
{
	//! メインメモリバッファ数分確保(ヘッダーで確保).
	THRS_MsgMemoryClearInfo& s_ClrMsg = s_MsgClearInfoPci[uc_MemoryClrCntPci];
	uc_MemoryClrCntPci++;

	if(uc_MemoryClrCntPci >= THRD_MAX_MAINMEM){
		uc_MemoryClrCntPci = 0;
	}

	THRL_ThAssert( pc_BufResMan[e_BufId] != NULL, "THRC_BufMemory::ReqClearPCIBuf invalid e_BufId=%d" , e_BufId, 0, 0, 0, CFA14_DetailCode0x0B72 );

	//! Asic 1枚目でクリア.
	s_ClrMsg.e_CtrID = e_CtrId;
	s_ClrMsg.e_DMAID = THRE_DmaId_MemClr;
	s_ClrMsg.s_ClrBufInfo.pc_BufResMan = NULL;						//! リソース管理クラスを経由しない.
#if defined(DEF_S800)
	if( thePowerManagerTask.isJpgHddAsic1200TRInitBusy() == True ){	// スリープからの復帰.
		s_ClrMsg.ev_ReqThID = TID_PowerManeger;
	} else {
		s_ClrMsg.ev_ReqThID = TID_ThreadManager;
	}
#else /* defined(DEF_S800) */
	s_ClrMsg.ev_ReqThID = e_ThrId;
#endif /* defined(DEF_S800) */
	s_ClrMsg.b_OutputSdram = False;
	s_ClrMsg.b_OutputPci = True;
	s_ClrMsg.b_1200Mode = False;
	s_ClrMsg.s_BufImgInfo.uc_MaxColorNum = 1;
	s_ClrMsg.s_BufImgInfo.uc_AllPlaneNum = 1;
	s_ClrMsg.s_BufImgInfo.uc_MaxAttBitNum = 0;
	s_ClrMsg.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_Density;
	s_ClrMsg.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_1bit;
	// OP/PP共通化. 
	s_ClrMsg.s_BufImgInfo.e_ImageCompress = IMGE_ImageCompress_BTCxxxAndJBIG;
	s_ClrMsg.s_BufImgInfo.ul_DstImageWidth = pc_BufResMan[e_BufId]->getBufWidth();
	s_ClrMsg.s_BufImgInfo.ul_DstImageHeight = pc_BufResMan[e_BufId]->getBufLineNum() * pc_BufResMan[e_BufId]->getPlaneNum();
	s_ClrMsg.s_BufImgInfo.ul_SrcImageWidth = 0;
	s_ClrMsg.s_BufImgInfo.ul_SrcImageHeight = 0;
	s_ClrMsg.s_BufImgInfo.uc_TopBandNo = 0;
	s_ClrMsg.s_BufImgInfo.us_alignWidth = 0;
	s_ClrMsg.s_PciAdrInfo.b_RingBufferMode = False;
	s_ClrMsg.s_PciAdrInfo.s_BufAddress[0].s_Color[0].ul_PlaneAdd[0] = pc_BufResMan[e_BufId]->getAddr();
	s_ClrMsg.s_PciAdrInfo.uc_BufBandNum[0] = 1;
	s_ClrMsg.s_PciAdrInfo.uc_BufPlaneNum[0] = 1;
	s_ClrMsg.s_PciAdrInfo.ul_BufferWidth = s_ClrMsg.s_BufImgInfo.ul_DstImageWidth;
	comMultiThreadProfile.setPciBufClrState( EN_PciBufClrBusy );	//! PCI上のバッファメモリクリア中とする.

	comMultiThreadProfile.MemClrReq( &s_ClrMsg );
}


// ================================================================================================.
//! <!--機能-->SDRAM全クリア処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! SDRAMの全クリアを行う\n.
//! <!--履歴-->.
//  09/08/07 越野	 	Donau環境から分離して作成.
//  08/08/11 R.Suzuki	CQ.309145 1Pass対応 3枚目の正式対応.
//  09/07/15 河村		CQ.319244 省電力復帰からの呼び出しに対応.
//	09/09/29 T.Shima	AsicサブメモリHW仕様変更対応.
// 2009/10/02 岡田		Asicサブメモリリソース対応.
// 2012/03/28 山下	　　CQ363358_No.405_Asic30DSのSDRAMクリアにて256MBを2回のクリアがされていない対応.　
// 2013/01/28 ADS川口	CQ.377863  MS-LCL_23 メモリクリア情報(SDRAM)の動的確保化.
//						未搭載のASICはTHRS_MsgMemoryClearInfoを確保しないためクリア処理を実施しない.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//  13/03/13 Sky ZeusS対応.
//  14/09/30 Sky Asic2014対応.
//  15/10/06 Sky システム統合SoC(S800)対応.
//  17/04/07 Sky ZeusSMLKPF対応.
//  17/04/28 日立 OP/PP共通化.
//  17/06/13 Sky OP/PP共通化.
//  17/07/11 Sky OP/PP共通化.Sim起動時エラー暫定対応.
//  18/11/21 Sky 【DenebMLK】[AR.1059875] 立ち上げ時の暫定対応(強制メモリクリア処理)の削除.
// ================================================================================================.
void THRC_BufMemory::AllClearSDRAM()
{
	setDebugLowSpeedSeq( 510, 0 );
	Bool b_TypeOK = False;

#ifdef DEF_PP_Cus
	printf( "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	printf( "@@@ THRC_BufMemory::AllClearSDRAM() if Rembrandt return! This measure is temporary(2017/07/12 release only)!!\n");
	printf( "@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@@\n");
	return ;
#endif // DEF_PP_Cus.

	b_MemoryClear = False;
	THRE_BIOS_AsicUID e_AsicUID[] = { THRE_AsicUID_Asic_1, THRE_AsicUID_Asic_2, THRE_AsicUID_Asic_3, THRE_AsicUID_Asic_4, THRE_AsicUID_Asic_5 };	// 新規Asic追加 13/03/13 Sky ZeusS対応.
	
	//! Asic5枚xSDRAM2枚クリア.
	for( uchar i = 0; i < (THRD_SubMem_AsicNum * THRD_SubMem_SlotNum); i++ ){
		// 未搭載のASICの場合クリア処理不要で、ps_MsgClearInfoSdramに領域割当てしていないため処理を飛ばす.
		if( ps_MsgClearInfoSdram[i/THRD_SubMem_SlotNum][i%THRD_SubMem_SlotNum] == NULL ){
			continue;
		}
		THRS_MsgMemoryClearInfo& rs_Msg = *ps_MsgClearInfoSdram[i/THRD_SubMem_SlotNum][i%THRD_SubMem_SlotNum];
		memset( &rs_Msg.s_PciAdrInfo, 0, sizeof(THRS_BufAdrInfo) );	//! 使用しない転送先情報を初期化.
		rs_Msg.ul_BandSize = THRF_GetDIMMSizeBySPD( i, b_TypeOK ) * 1024 * 1024;		//! メモリバンドサイズ.
		rs_Msg.s_SdramAdrInfo.ul_RingBufferAddr = 0x0;						//! SDRAMメモリクリア先頭アドレス.
		
		// yamashita Donauソースより移植 Sta.
		if( (i % THRD_SubMem_SlotNum) == 0 ){
			//! 入力バッファ? Asic内の1枚目の方.
			rs_Msg.s_SdramAdrInfo.e_BufType         = THRE_BufType_SDRAM0;		//! バッファ種別.
		} else {
			//! 出力バッファ? Asic内の2枚目の方.
			rs_Msg.s_SdramAdrInfo.e_BufType         = THRE_BufType_SDRAM1;		//! バッファ種別.
		}
		// yamashita Donauソースより移植 End.

		switch( i / THRD_SubMem_SlotNum ){
			case 0:																//! Asic1枚目.
				rs_Msg.e_CtrID = THRE_CtrID_0;
				rs_Msg.e_DMAID = THRE_DmaId_MemClr;
				if( THRF_IsAsic_0Exist() == False ){
					continue;
				}
				if( (i % THRD_SubMem_SlotNum) != 0 ){							//! SDRAM2枚目なしの為.
					continue;
				}
				break;
			case 1:																//! Asic2枚目.
				rs_Msg.e_CtrID = THRE_CtrID_1;
				rs_Msg.e_DMAID = THRE_DmaId_MemClr_2;
				if( THRF_IsAsic_1Exist() == False ){
					continue;
				}
				if( (i % THRD_SubMem_SlotNum) != 0 ){							//! A800Fieryの場合は、SDRAM2枚目クリアしない.
					if( False == THRF_IsA800IntegrationExist() ){
						continue;
					}
				}
				break;
			case 2:																//! Asic3枚目.
				rs_Msg.e_CtrID = THRE_CtrID_2;
				rs_Msg.e_DMAID = THRE_DmaId_MemClr_3;
				if( THRF_IsAsic_2Exist() == False ){
					continue;
				}
				break;
			case 3:																//! Asic4枚目 13/03/13 Sky ZeusS対応.
				rs_Msg.e_CtrID = THRE_CtrID_3;
				// DMAID削除に伴い、コメントアウト.
//				rs_Msg.e_DMAID = THRE_DmaId_MemClr_4;
				if( THRF_IsAsic_3Exist() == False ){
					continue;
				}
				if( (i % THRD_SubMem_SlotNum) != 0 ){							//! SDRAM2枚目なしの為(※2枚目はPIC用のためスレッドではクリアしない).
					continue;
				}
				break;
			case 4:																//! Asic5枚目 13/03/13 Sky ZeusS対応.
				rs_Msg.e_CtrID = THRE_CtrID_4;
				// DMAID削除に伴い、コメントアウト.
//				rs_Msg.e_DMAID = THRE_DmaId_MemClr_5;
#ifndef DEF_PP_Cus
				if( THRF_IsAsic_4Exist() == False ){
					continue;
				}
#else // DEF_PP_Cus.
				// OP/PP共通化.
				// PP機：A800の3枚目（Asic5枚目）については、スレッドでのバッファ初期化は行わないため、クリア処理を実行しない.
				continue;
#endif // DEF_PP_Cus.
				if( (i % THRD_SubMem_SlotNum) != 0 ){							//! SDRAM2枚目なしの為.
					continue;
				}
				break;
			default:
				THRL_ThAssert( False, "AllClearSDRAM Asic count error %d", (i/2) , 0, 0, 0, CFA14_DetailCode0x088F );
				break;
		}

		rs_Msg.s_ClrBufInfo.pc_BufResMan = NULL;								//! リソース管理クラスを経由しない.
		// 2009/07/15 河村 CQ319244 - スリープからの復帰はスレッド電源制御部へ通知.
		if( thePowerManagerTask.isJpgHddAsic1200TRInitBusy() == True ){	// スリープからの復帰.
			rs_Msg.ev_ReqThID = TID_PowerManeger;
		} else {
			rs_Msg.ev_ReqThID = TID_ThreadManager;
		}
		rs_Msg.b_OutputSdram = True;
		rs_Msg.b_OutputPci = False;
		rs_Msg.b_1200Mode = False;
		rs_Msg.s_BufImgInfo.uc_MaxColorNum = 1;
		rs_Msg.s_BufImgInfo.uc_AllPlaneNum = 1;
		rs_Msg.s_BufImgInfo.uc_MaxAttBitNum = 0;
		rs_Msg.s_BufImgInfo.e_ImageColor = IMGE_ImageColor_Density;
		rs_Msg.s_BufImgInfo.e_ImageTone = IMGE_ImageTone_1bit;
		// OP/PP共通化. 
		rs_Msg.s_BufImgInfo.e_ImageCompress = IMGE_ImageCompress_BTCxxxAndJBIG;
		THRE_AsicKind e_AsicKind = THRL_Drvrif_GetAsicKind( e_AsicUID[i / THRD_SubMem_SlotNum] );
		switch( e_AsicKind ) {
			case THRE_AsicKind_Asic2014:	// 新規Asic追加.
			case THRE_AsicKind_A800:		// 新規Asic追加.
			default:
				// S800/PMS/2014/A800はレジスタ仕様上、横幅16384まで指定可能.
				// 画像横幅はMax:16384に指定し、326MBをクリアする.
				rs_Msg.s_BufImgInfo.ul_DstImageWidth = DEF_IMAGEWIDTH_MAX;
				break;
		}
		
		rs_Msg.s_BufImgInfo.ul_DstImageHeight = rs_Msg.ul_BandSize / (rs_Msg.s_BufImgInfo.ul_DstImageWidth / 8);		// ul_BandSizeを8倍するとオーバーフローするので計算方法変更.
		//! 桁あふれ時の対応.
		while( rs_Msg.s_BufImgInfo.ul_DstImageHeight > 0x10000 ){
			//! ショートを超えるとまずいので、プレーン数を増やしてライン数を減らす.
			//! 0x10000ラインについてはレジスタ設定時に-1するため設定可能.
			//! ただし、s_SdramAdrInfo.us_RingBufferHeightにコピーすると桁落ちするため注意.
			//! 参照する場合は、s_BufImgInfo.ul_DstImageHeightを使用する.
			if( rs_Msg.s_BufImgInfo.uc_MaxColorNum < 4 ){
				//! 4色まではカラーで頑張る.
				rs_Msg.s_BufImgInfo.uc_MaxColorNum = (uchar)(rs_Msg.s_BufImgInfo.uc_MaxColorNum * 2);
			} else {
				//! カラー一杯なら、属性ビットを利用する.
				THRL_ThAssert( (rs_Msg.s_BufImgInfo.uc_MaxAttBitNum == 0), "ClearSizeOver!" , 0, 0, 0, 0, CFA14_DetailCode0x0890 );
				rs_Msg.s_BufImgInfo.uc_MaxAttBitNum = rs_Msg.s_BufImgInfo.uc_MaxColorNum;
				rs_Msg.s_BufImgInfo.uc_MaxColorNum++;
			}
			rs_Msg.s_BufImgInfo.uc_AllPlaneNum = rs_Msg.s_BufImgInfo.uc_MaxColorNum;
			if( rs_Msg.s_BufImgInfo.uc_MaxAttBitNum > 0 ){
				rs_Msg.s_BufImgInfo.uc_AllPlaneNum = (uchar)(rs_Msg.s_BufImgInfo.uc_AllPlaneNum + (rs_Msg.s_BufImgInfo.uc_MaxAttBitNum - 1));
			}
			THRL_ThAssert( (rs_Msg.s_BufImgInfo.ul_DstImageHeight % 2 == 0), "SDRAM AllClear Error" , 0, 0, 0, 0, CFA14_DetailCode0x0891 );
			rs_Msg.s_BufImgInfo.ul_DstImageHeight = rs_Msg.s_BufImgInfo.ul_DstImageHeight / 2;
		}
		rs_Msg.s_BufImgInfo.ul_SrcImageWidth = 0;
		rs_Msg.s_BufImgInfo.ul_SrcImageHeight = 0;
		rs_Msg.s_BufImgInfo.uc_TopBandNo = 0;
		rs_Msg.s_BufImgInfo.us_alignWidth = 0;
		rs_Msg.s_SdramAdrInfo.ul_RingBufferOffAddr = rs_Msg.ul_BandSize / rs_Msg.s_BufImgInfo.uc_AllPlaneNum;
		rs_Msg.s_SdramAdrInfo.us_RingBufferMaxPlaneNum = 1;
		rs_Msg.s_SdramAdrInfo.us_RingBufferWidth = (ushort)rs_Msg.s_BufImgInfo.ul_DstImageWidth;
		rs_Msg.s_SdramAdrInfo.us_RingBufferHeight = (ushort)rs_Msg.s_BufImgInfo.ul_DstImageHeight;
		rs_Msg.s_SdramAdrInfo.us_DataWidth = (ushort)rs_Msg.s_BufImgInfo.ul_DstImageWidth;
		rs_Msg.s_SdramAdrInfo.us_DataHeight = (ushort)rs_Msg.s_BufImgInfo.ul_DstImageHeight;
		rs_Msg.s_SdramAdrInfo.us_RingImageTopLineNo = 0;
		rs_Msg.s_SdramAdrInfo.ul_RingImageTopAddr = rs_Msg.s_SdramAdrInfo.ul_RingBufferAddr;
		THRL_SetThLog( 37, THRE_DebugLogType_MTCtrLog, 13, True,i,THRD_SubMem_SlotNum,rs_Msg.s_SdramAdrInfo.ul_RingBufferAddr);

#ifdef COSIM
		//エンジン出力(SDRAM0),スキャン出力(SDRAM1)用バッファまで初期化.
		//横幅 2048.
		rs_Msg.s_BufImgInfo.ul_DstImageWidth = 2048;							//! きれいに割れるようにしておく.
		rs_Msg.s_SdramAdrInfo.us_RingBufferWidth = 2048;
		rs_Msg.s_SdramAdrInfo.us_DataWidth = 2048;
		rs_Msg.s_BufImgInfo.ul_DstImageHeight = 2048;
		rs_Msg.s_SdramAdrInfo.us_RingBufferHeight = 2048;
		rs_Msg.s_SdramAdrInfo.us_DataHeight = 2048;
#endif // COSIM.

		comMultiThreadProfile.MemClrReq( &rs_Msg );
		uc_MemoryClrCnt++;
	}
}
// ================================================================================================.
//! <!--機能-->SDRAMクリア完了処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! SDRAMのクリア完了時に完了を判定し状態を設定する\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
void THRC_BufMemory::clearComplete()
{
	THRL_ThLogMsgDebug(( "THRC_BufMemory init SDRAM Clear Complete\n" ));
	THRL_ThAssert( (uc_MemoryClrCnt != 0), "clearComplete ClrCnt error" , 0, 0, 0, 0, CFA14_DetailCode0x0892 );

	uc_MemoryClrCnt--;
	if( uc_MemoryClrCnt == 0 ){
		IMGC_ImageMemoryPoolImpColorFileMemoryInstance& comMemoryManager = IMGC_ImageMemoryPoolImpColorFileMemorySingletonIf::newInstance();
		b_MemoryClear = True;
		comMemoryManager.setMemoryManagerReady();
	}
}


// ================================================================================================.
//! <!--機能-->SDRAMクリア完了チェック.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return Bool		：SDRAMクリア完了(True：完了、False：未完了).
//! <!--詳細-->.
//! SDRAMのクリア完了状態を返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
Bool THRC_BufMemory::isClearSDRAM()
{
	return ( b_MemoryClear );
}


// ================================================================================================.
//! <!--機能-->SDRAMの物理メモリサイズ取得(Byte).
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return uint32_t		：物理サイズ(Byte).
//! <!--詳細-->.
//! SDRAMの物理メモリサイズを返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
uint32_t THRC_BufMemory::GetPhysicalMemSize()
{
	return ( ul_PhysicalMemSize );
}


// ================================================================================================.
//! <!--機能-->SDRAMのバッファモード設定処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  e_BufId		：バッファID.
//! \param  e_BufMode	：バッファモード.
//! <!--戻値-->.
//! \return Bool		：処理結果(True：正常、False：異常).
//! <!--詳細-->.
//! SDRAMのバッファモードの設定を行う\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  12/11/21 ADS坪井 CQ.377856  MS-LCL_3 THRC_BufferResourceインスタンス動的確保変更に伴う修正.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//  13/04/19 ADS坪井 CQ.379397(Act_64292)  MS-EVP-69 <静的解析対応>e_chgBufId値の範囲チェック処理を追加.
// ================================================================================================.
Bool THRC_BufMemory::setBufMode( THRE_BufId e_BufId, THRE_BufMode e_BufMode )
{
	THRC_IntLock c_Lock;
	Bool b_Ans = False;

	THRE_BufId e_chgBufId = chgShadowBufId( e_BufId );							//! 共有バッファID取得.

	THRL_ThAssert( (e_chgBufId < THRE_BufId_Num) && (pc_BufResMan[e_chgBufId] != NULL), "THRC_BufMemory::setBufMode invalid e_chgBufId=%d" , e_chgBufId, 0, 0, 0, CFA14_DetailCode0x0B73 );

	if( (e_BufMode == pc_BufResMan[e_chgBufId]->getBufMode())					//! バッファモードチェック.
	 && (e_BufId == e_chgBufId)													//! 共有チェック.
	){
		//! 同一バッファで同一モードの為、変更不要.
		b_Ans = True;
	} else {
		//! 異なるバッファ又は異なるモードの為、変更.
		if( pc_BufResMan[e_chgBufId]->chkBufModeChg() == True ){					//! モード変更可否チェック.
			//! バッファモード変更可能.
			THRS_SubBufParaCom	s_Param;
			b_Ans = True;
			GetBufMemoryInfo( e_BufId, e_BufMode, s_Param );
			pc_BufResMan[e_chgBufId]->setBuffer( e_BufMode, s_Param.ul_BufWidth, s_Param.us_BandMax, s_Param.us_PlaneMax );
																				//! バッファ情報設定処理.
		} else {
			b_Ans = False;
		}
	}
	return ( b_Ans );
}


// ================================================================================================.
//! <!--機能-->指定バッファIDの共有先バッファID取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  e_BufId		：バッファID(共有元).
//! <!--戻値-->.
//! \return THRE_BufId	：バッファID(共有先).
//! <!--詳細-->.
//! 指定バッファIDから共有先のバッファIDを検索して返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  09/08/09 n.suzuki	静的解析対応3rd.ID:21745,21744.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//  13/12/03 戸田		CQ383954 Zeus5BK3rdマージ コピーでFA14の修正。THRE_BufId_Noneの場合はアサートしないように条件追加.
// ================================================================================================.
THRE_BufId THRC_BufMemory::chgShadowBufId( THRE_BufId e_BufId )
{
	struct THRS_SelShadowBufId {
		THRE_BufId			e_MyBufId;							//! 元のバッファID.
		THRE_BufId			e_ShadowBufId;						//! 共有先のバッファID.
	};
	static THRS_SelShadowBufId s_SelTbl[] = {
		//元のBufId						共有先のBufId.
//		{ THRE_BufId_ExtInDup,			THRE_BufId_ExtIn				},	//! 例:スキャン入力両面用⇒スキャン入力.
		{ THRE_BufId_None,				THRE_BufId_None					},
	};

	THRL_ThAssert( ( ( e_BufId < THRE_BufId_Num ) || ( e_BufId == THRE_BufId_None ) ),"chgShadowBufId e_BufId = %d", e_BufId , 0, 0, 0, CFA14_DetailCode0x0BFD ); 

	THRE_BufId e_ShadowBufId = e_BufId;
	uchar uc_IndexMax = sizeof(s_SelTbl)/sizeof(THRS_SelShadowBufId);
	//! 共有先バッファID検索.
	for( uchar uc_Index=0; uc_Index<uc_IndexMax; uc_Index++ ) {
		if( e_BufId == s_SelTbl[uc_Index].e_MyBufId ) {
			e_ShadowBufId = s_SelTbl[uc_Index].e_ShadowBufId;
			break;
		}
	}

	return( e_ShadowBufId );
}


// ================================================================================================.
//! <!--機能-->バッファリソース管理クラスのインスタンス取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  e_BufId					：バッファID.
//! <!--戻値-->.
//! \return THRC_BufferResource*	： バッファリソース管理のインスタンス.
//! <!--詳細-->.
//! 指定バッファIDからバッファリソース管理クラスのインスタンスを返す\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  12/11/21 ADS坪井 CQ.377856  MS-LCL_3 THRC_BufferResourceインスタンス動的確保変更に伴う修正.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//  13/04/19 ADS坪井 CQ.379397(Act_64292)  MS-EVP-69 <静的解析対応>e_BufferId値の範囲チェック処理を追加.
// ================================================================================================.
THRC_BufferResource* THRC_BufMemory::getBufferResourceInstance( THRE_BufId e_BufId )
{
	THRE_BufId e_BufferId = chgShadowBufId( e_BufId );
	if (e_BufferId < THRE_BufId_Num) {
		if (pc_BufResMan[e_BufferId] == NULL) { THRL_ThLogMsgDebug( "THRC_BufMemory::getBufferResourceInstance invalid e_BufferId=%d\n", e_BufferId ); }
		return( pc_BufResMan[e_BufferId] );
	} else {
		THRL_ThLogMsgDebug( "THRC_BufMemory::getBufferResourceInstance invalid e_BufferId=%d\n", e_BufferId );
		return( NULL );
	}
}

#ifdef DEF_Soc1200
// ================================================================================================.
//! <!--機能-->サブファイルメモリの開始アドレス取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return uintptr_t	： サブファイルメモリの開始アドレス.
//! <!--詳細-->.
//! サブファイルメモリの開始アドレスを返す\n.
//! <!--履歴-->.
//! 2013/07/10 ADS滕 CQ.381171(Act_66426) SC-SFM-23 サブメモリのファイルメモリ化 新規作成.
// ================================================================================================.
uintptr_t THRC_BufMemory::getSubFileMemoryStartAddr()
{
	return ul_SubFileMemoryStartAddr;
}

// ================================================================================================.
//! <!--機能-->サブファイルメモリのサイズ取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return uint32_t	： サブファイルメモリのサイズ.
//! <!--詳細-->.
//! サブファイルメモリのサイズを返す\n.
//! <!--履歴-->.
//! 2013/07/10 ADS滕 CQ.381171(Act_66426) SC-SFM-23 サブメモリのファイルメモリ化 新規作成.
// ================================================================================================.
uint32_t THRC_BufMemory::getSubFileMemorySize()
{
	return ul_SubFileMemorySize;
}

// ================================================================================================.
//! <!--機能-->サブファイルメモリのサイズ設定.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  ul_Size	：サブファイルメモリのサイズ.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! サブファイルメモリのサイズを設定する\n.
//! <!--履歴-->.
//! 2013/08/05 ADS坪井 CQ.381171(Act_66426) SC-SFM-23 サブメモリのファイルメモリ化 新規作成.
// ================================================================================================.
void THRC_BufMemory::setSubFileMemorySize( uint32_t ul_Size )
{
	ul_SubFileMemorySize = ul_Size;
}
#endif	// DEF_Soc1200.

// ================================================================================================.
//! <!--機能-->Asic5枚時の各SDRAMの初期化.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! Asic5枚目の各SDRAMのアドレス、サイズ等の情報を設定する\n.
//! <!--履歴-->.
//  13/03/13 Sky ZeusS対応 新規作成.
//  15/10/06 Sky システム統合SoC(S800)対応.
//  17/06/13 Sky OP/PP共通化.
// ================================================================================================.
void THRC_BufMemory::init_AsicSdram_5( )
{
	THRL_ThLogMsgDebug(( "[[[[[[ SDRAM buffer address setting Start [[[[[[\n" ));

#ifdef DEF_PP_Cus
	// OP/PP共通化.
	// A800_3(ASIC5)はスレッドでのバッファ利用は無いため、初期化処理を行う必要が無い。.
	if ( THRE_AsicKind_A800 != THRL_Drvrif_GetAsicKind( THRE_AsicUID_Asic_5 ) ) {
	// OP機既存処理.
#endif // DEF_PP_Cus.
	
	///////////////////////////////////////////////////.
	//////////////// Asic 5個目 SDRAM0 ////////////////.
	///////////////////////////////////////////////////.
	//! メモリ開始アドレスは０.
	register uintptr_t ul_StartAddr = 0;
	THRL_ThLogMsgDebug( "== Asic 5th SDRAM0 start address 0x%08"PRIXPTR"H \n", ul_StartAddr );
	
	//! メモリ領域をはみ出していないかチェック.
	register uint32_t ul_Asic_5_SDRAM0 = ul_OneSubMemorySize[ 4 ][ 0 ];	//【暫定:橋本】].
	THRL_ThAssert( ul_StartAddr < ul_Asic_5_SDRAM0, "BufAddress Error Asic_5 SDRAM0 MemSize %"PRIdPTR", BufSize %"PRId32"", ul_StartAddr, ul_Asic_5_SDRAM0 , 0, 0, CFA14_DetailCode0x0C2C );

#ifdef DEF_PP_Cus
	// OP/PP共通化.
	}else{
		// PP機対応.
		THRL_ThLogMsgDebug( "[[[[[[ SDRAM buffer address setting Skip [[[[[[\n" );
	}
#endif // DEF_PP_Cus.
	
	THRL_ThLogMsgDebug(( "[[[[[[ SDRAM buffer address setting End [[[[[[\n" ));
}

// ================================================================================================.
//! <!--機能-->Asic4枚時の各SDRAMの初期化.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! Asic4枚目の各SDRAMのアドレス、サイズ等の情報を設定する\n.
//! <!--履歴-->.
//  13/03/13 Sky ZeusS対応 新規作成.
//  17/06/13 Sky OP/PP共通化.
// ================================================================================================.
void THRC_BufMemory::init_AsicSdram_4( )
{
	THRL_ThLogMsgDebug( "[[[[[[ SDRAM buffer address setting Start [[[[[[\n" );

	register uint32_t ul_TmpBufSize = 0;	//! 仮のバッファサイズ格納用.
	
#ifdef DEF_PP_Cus
	// OP/PP共通化.
	// A800_2(ASIC4)はスレッドでのバッファ利用は無いため、初期化処理を行う必要が無い。.
	if( THRE_AsicKind_A800 != THRL_Drvrif_GetAsicKind( THRE_AsicUID_Asic_4 ) ) {
	// OP機既存処理.
#endif // DEF_PP_Cus.
	///////////////////////////////////////////////////.
	//////////////// Asic 4個目 SDRAM0 ////////////////.
	///////////////////////////////////////////////////.
	//! メモリ開始アドレスは０.
	register uintptr_t ul_StartAddr = 0;
	THRL_ThLogMsgDebug( "== Asic 4th SDRAM0 start address 0x%08"PRIXPTR"H \n", ul_StartAddr );

	//! エンジン出力用バッファ.
	ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_EngOut );				//! エンジン出力用バッファ初期化.
	THRL_SetDbgBufPlace( THRE_BufId_EngOut, THRE_DebugBufPlace_ASIC_3 );						//! デバッグ用.
	ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.

	//! エンジン画像合成用伸張バッファ.
	ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_CombineEng );			//! エンジン画像合成用伸張バッファ初期化.
	THRL_SetDbgBufPlace( THRE_BufId_CombineEng, THRE_DebugBufPlace_ASIC_3 );					//! デバッグ用追加.
	ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
	
	//! メモリ領域をはみ出していないかチェック.
	register uint32_t ul_Asic_4_SDRAM = ul_OneSubMemorySize[ 3 ][ 0 ];
	THRL_ThAssert( ul_StartAddr < ul_Asic_4_SDRAM, "BufAddress Error Asic_4 SDRAM MemSize %"PRIdPTR", BufSize %"PRId32"", ul_StartAddr, ul_Asic_4_SDRAM , 0, 0, CFA14_DetailCode0x0C2B );

#ifdef DEF_PP_Cus
	// OP/PP共通化.
	}else{
		// PP機対応.
		THRL_ThLogMsgDebug( "[[[[[[ SDRAM buffer address setting Skip [[[[[[\n" );
	}
#endif // DEF_PP_Cus.
	
	THRL_ThLogMsgDebug(( "[[[[[[ SDRAM buffer address setting End [[[[[[\n" ));
}

// ================================================================================================.
//! <!--機能-->Asic3枚目の各SDRAMの初期化.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! Asic3枚目の各SDRAMのアドレス、サイズ等の情報を設定する\n.
//! <!--履歴-->.
//  2008/08/11 R.Suzuki	CQ.309145 1Pass対応 新規作成.
//	2010/12/20 大石		Asic30対応.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//  13/03/13 Sky ZeusS対応.
//  14/02/03 Sky ASIC777DS対応.
//  14/09/30 Sky Asic2014対応.
//  15/10/06 Sky システム統合SoC(S800)対応.
//  16/10/25 Sky合屋    [IT5_4.1]制御構成変更No.40 自動評価対応.
//  16/12/26 Sky合屋	[IT5_4.1][1010034]ZeusSの自動評価 テストモード2で裏面に異常画像が印字される問題の対応.
//  17/04/07 Sky ZeusSMLKPF対応.
// ================================================================================================.
void THRC_BufMemory::init_AsicSdram_3( )
{
	// Asic 3個目 SDRAM0 128MByte.
	uint32_t ul_TmpBufSize = 0;	// 仮のバッファサイズ格納用.
	uint32_t ul_Asic_2_SDRAM0 = 0;		// サイズチェック用変数.
	uintptr_t ul_StartAddr0 = 0;
	Bool bl_IsAutoTestMode = APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( 195, 0 );

	//! SDRAM0の初期化.
	THRL_ThLogMsgDebug( "[[[[[[ SDRAM buffer address setting Start [[[[[[\n" );
	THRL_ThLogMsgDebug( "== Asic 3rd SDRAM0 start address 0x%08"PRIXPTR"H \n", ul_StartAddr0 );

	// ASIC2014DSでFieryを使用する かつ 自動評価無効 の 場合.
	if( False == bl_IsAutoTestMode ) { 
		//! Fiery入力用圧縮バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr0, THRE_BufId_ExtIfIn );							//! 外部入力用圧縮バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_ExtIfIn, THRE_DebugBufPlace_ASIC_2 );							//! デバッグ用追加.
		ul_StartAddr0 = _roundup( ( ul_StartAddr0 + ul_TmpBufSize ), ( 1 * 1024 ) ) * ( 1 * 1024 );		//! １KByte境界へ割り付け切り上げ.
	}

	// メモリ領域をはみ出していないかチェック.
	// Asic2014装着時は、開始アドレスからメモリサイズMAXまで使用するので <= で比較チェック.
	ul_Asic_2_SDRAM0 = ul_OneSubMemorySize[ 2 ][ 0 ];
	THRL_ThAssert( ul_StartAddr0 <= ul_Asic_2_SDRAM0, "BufAddress Error Asic_2 SDRAM0 MemSize %"PRIdPTR", BufSize %"PRId32"", ul_StartAddr0, ul_Asic_2_SDRAM0, 0, 0, CFA14_DetailCode0x0893 );
	//自動評価有効な場合 .
	if( True == bl_IsAutoTestMode ){
		// スキャナIF入力用圧縮バッファ裏面用(擬似バッファ) .
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr0, THRE_BufId_DummyExtInDup );					// スキャナIF入力用バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_DummyExtInDup, THRE_DebugBufPlace_ASIC_2 );						// デバッグ用追加.
		ul_StartAddr0 = _roundup( ( ul_StartAddr0 + ul_TmpBufSize ), ( 1 * 1024 ) ) * ( 1 * 1024 );		// 1KB協会へ割付切り上げ.
	}


	THRL_ThLogMsgDebug( "[[[[[[ SDRAM buffer address setting End [[[[[[\n" );

}

// ================================================================================================.
//! <!--機能-->Asic2枚目の各SDRAMの初期化.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! Asic2枚目の各SDRAMのアドレス、サイズ等の情報を設定する\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
// 2008/08/11 R.Suzuki	CQ.309145 1Pass対応 関数名変更と整理 init_DoubleAsicSdramMode → init_AsicSdram_2.
// 2009/10/02 岡田		Asicサブメモリリソース対応.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//  17/04/07 Sky ZeusSMLKPF対応.
//  17/06/29 Sky OP/PP共通化.エンジンバッファID追加の対応.
// ================================================================================================.
void THRC_BufMemory::init_AsicSdram_2( )
{
	THRL_ThLogMsgDebug(( "[[[[[[ SDRAM buffer address setting Start [[[[[[\n" ));

	register uint32_t ul_TmpBufSize = 0;	//! 仮のバッファサイズ格納用.
	
	///////////////////////////////////////////////////.
	//////////////// Asic 2個目 SDRAM0 ////////////////.
	///////////////////////////////////////////////////.
	//! メモリ開始アドレス.
	register uintptr_t ul_StartAddr0 = 0;
	register uintptr_t ul_StartAddr1 = 0;

	if( False == THRF_IsA800IntegrationExist() ) { 
		THRL_ThLogMsgDebug( "== Asic 2nd SDRAM0 start address 0x%08"PRIXPTR"H \n", ul_StartAddr0 );

		//! Fiery入力用圧縮バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr0, THRE_BufId_ExtIfIn );				//! 外部入力用圧縮バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_ExtIfIn, THRE_DebugBufPlace_ASIC_1 );				//! デバッグ用追加.
		ul_StartAddr0 = _roundup( (ul_StartAddr0+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
	
		//! メモリ領域をはみ出していないかチェック.
#ifdef COSIM
		register uint32_t ul_Asic_1_SDRAM0 = 128*1024*1024;	//【暫定:橋本】].
#else
		register uint32_t ul_Asic_1_SDRAM0 = ul_OneSubMemorySize[ 1 ][ 0 ];	//【暫定:橋本】].
#endif
		THRL_ThAssert( ul_StartAddr0 < ul_Asic_1_SDRAM0, "BufAddress Error Asic_1 SDRAM0 MemSize %"PRIdPTR", BufSize %"PRId32"", ul_StartAddr0, ul_Asic_1_SDRAM0 , 0, 0, CFA14_DetailCode0x0894 );
	}else{
#ifdef DEF_PP_Cus
		// OP/PP共通化.
		// メモリ開始アドレス変更.
		// PP機ではサブメモリのバッファ構成が変更になったため、修正.
		// OP機ではEmperorIP用バッファとスレッドバッファ間のReserve領域用意していたが、PP機ではReserve領域を取得しない形に変更.
		ul_StartAddr0 = 0x0C000000;
		ul_StartAddr1 = 0x0C000000;
#else
		// メモリ開始アドレス変更.
		ul_StartAddr0 = 0x20000000;
		ul_StartAddr1 = 0x10000000;
#endif

		// CH0 /////////////////////////////////////////////////////////////////////////////.
		THRL_ThLogMsgDebug( "== Asic 2nd SDRAM0 start address 0x%08"PRIXPTR"H \n", ul_StartAddr0 );

		//! Emperor入力用圧縮バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr0, THRE_BufId_AppIn );				//! Emperor入力用圧縮バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_AppIn, THRE_DebugBufPlace_ASIC_1 );					//! デバッグ用追加.
		ul_StartAddr0 = _roundup( (ul_StartAddr0+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.

		//! Fiery入力用圧縮バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr0, THRE_BufId_ExtIfIn );				//! 外部入力用圧縮バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_ExtIfIn, THRE_DebugBufPlace_ASIC_1 );				//! デバッグ用追加.
		ul_StartAddr0 = _roundup( (ul_StartAddr0+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.

		//! 画像変換後圧縮前バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr0, THRE_BufId_ConvCmp );				//! 画像変換後圧縮前バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_ConvCmp, THRE_DebugBufPlace_ASIC_1 );				//! デバッグ用.
		ul_StartAddr0 = _roundup( (ul_StartAddr0+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.

		//! スキャン出力用伸張バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr0, THRE_BufId_ScnOut );				//! スキャン出力用伸張バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_ScnOut, THRE_DebugBufPlace_ASIC_1 );				//! デバッグ用追加.
		ul_StartAddr0 = _roundup( (ul_StartAddr0+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.

		//! 画像変換前用伸張バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr0, THRE_BufId_ConvExp );				//! 画像変換前用伸張バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_ConvExp, THRE_DebugBufPlace_ASIC_1 );				//! デバッグ用追加.
		ul_StartAddr0 = _roundup( (ul_StartAddr0+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.

		//! アプリ出力画像合成伸張後バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr0, THRE_BufId_CombineApp );			//! アプリ出力画像合成伸張後バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_CombineApp, THRE_DebugBufPlace_ASIC_1 );			//! デバッグ用.
		ul_StartAddr0 = _roundup( (ul_StartAddr0+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.

		//! Fax送信(iFax)用伸張バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr0, THRE_BufId_Fax0Tx );				//! Fax送信(iFax)用伸張バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Fax0Tx, THRE_DebugBufPlace_ASIC_1 );				//! デバッグ用追加.
		ul_StartAddr0 = _roundup( (ul_StartAddr0+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
		
		//! Fax受信(iFax)用圧縮バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr0, THRE_BufId_Fax0Rx );				//! Fax受信(iFax)用圧縮バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Fax0Rx, THRE_DebugBufPlace_ASIC_1 );				//! デバッグ用追加.
		ul_StartAddr0 = _roundup( (ul_StartAddr0+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
		
		if( APIC_SystemInfo::newInstance()->getFax1Exist() == True ){
			//! Fax送信(回線1)用伸張バッファ.
			ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr0, THRE_BufId_Fax1Tx );			//! Fax送信(回線1)用伸張バッファ初期化.
			THRL_SetDbgBufPlace( THRE_BufId_Fax1Tx, THRE_DebugBufPlace_ASIC_1 );			//! デバッグ用追加.
			ul_StartAddr0 = _roundup( (ul_StartAddr0+ul_TmpBufSize), (1*1024) ) * (1*1024);	//! １KByte境界へ割り付け切り上げ.
		}
		
		if( APIC_SystemInfo::newInstance()->getFax2Exist() == True ){
			//! Fax送信(回線2)用伸張バッファ.
			ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr0, THRE_BufId_Fax2Tx );			//! Fax送信(回線2)用伸張バッファ初期化.
			THRL_SetDbgBufPlace( THRE_BufId_Fax2Tx, THRE_DebugBufPlace_ASIC_1 );			//! デバッグ用追加.
			ul_StartAddr0 = _roundup( (ul_StartAddr0+ul_TmpBufSize), (1*1024) ) * (1*1024);	//! １KByte境界へ割り付け切り上げ.
		}
		
		//! Fax送信(回線3)用伸張バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr0, THRE_BufId_Fax3Tx );				//! Fax送信(回線3)用伸張バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Fax3Tx, THRE_DebugBufPlace_ASIC_1 );				//! デバッグ用追加.
		ul_StartAddr0 = _roundup( (ul_StartAddr0+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
		
		//! Fax送信(回線4)用伸張バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr0, THRE_BufId_Fax4Tx );				//! Fax送信(回線4)用伸張バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Fax4Tx, THRE_DebugBufPlace_ASIC_1 );				//! デバッグ用追加.
		ul_StartAddr0 = _roundup( (ul_StartAddr0+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
// OP/PP共通化.
#ifdef DEF_PP_Cus		
		//! エンジン出力用伸張後バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr0, THRE_BufId_EngOut3 );				//! エンジン出力用伸張後バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_EngOut3, THRE_DebugBufPlace_ASIC_1 );				//! デバッグ用追加.
		ul_StartAddr0 = _roundup( (ul_StartAddr0+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
#endif //DEF_PP_Cus.		
		// CH1 /////////////////////////////////////////////////////////////////////////////.
		THRL_ThLogMsgDebug( "== Asic 2nd SDRAM1 start address 0x%08"PRIXPTR"H \n", ul_StartAddr1 );

		//! 回転&拡大プリント用伸張バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr1, THRE_BufId_RotPrint );			//! 回転&拡大プリント用伸張バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_RotPrint, THRE_DebugBufPlace_ASIC_1 );				//! デバッグ用追加.
		ul_StartAddr1 = _roundup( (ul_StartAddr1+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
		
		//! エンジン出力用バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr1, THRE_BufId_EngOut );				//! エンジン出力用バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_EngOut, THRE_DebugBufPlace_ASIC_1 );				//! デバッグ用追加.
		ul_StartAddr1 = _roundup( (ul_StartAddr1+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
// OP/PP共通化.
#ifdef DEF_PP_Cus	
		//! エンジン出力用バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr1, THRE_BufId_EngOut1 );				//! エンジン出力用バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_EngOut1, THRE_DebugBufPlace_ASIC_1 );				//! デバッグ用追加.
		ul_StartAddr1 = _roundup( (ul_StartAddr1+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.

		//! エンジン出力用バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr1, THRE_BufId_EngOut2 );				//! エンジン出力用バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_EngOut2, THRE_DebugBufPlace_ASIC_1 );				//! デバッグ用追加.
		ul_StartAddr1 = _roundup( (ul_StartAddr1+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
#endif //DEF_PP_Cus.		
		//! エンジン画像合成用伸張バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr1, THRE_BufId_CombineEng );			//! エンジン画像合成用伸張バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_CombineEng, THRE_DebugBufPlace_ASIC_1 );			//! デバッグ用追加.
		ul_StartAddr1 = _roundup( (ul_StartAddr1+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
		
		//! 回転前バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr1, THRE_BufId_Rotate );				//! 回転前バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Rotate, THRE_DebugBufPlace_ASIC_1 );				//! デバッグ用追加.
		ul_StartAddr1 = _roundup( (ul_StartAddr1+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.

		//! メモリ領域をはみ出していないかチェック.
		register uint32_t ul_Asic_1_SDRAM0 = ul_OneSubMemorySize[ 1 ][ 0 ];
		THRL_ThAssert( ul_StartAddr0 < ul_Asic_1_SDRAM0, "BufAddress Error Asic_1 SDRAM0 MemSize %"PRIdPTR", BufSize %"PRId32"", ul_StartAddr0, ul_Asic_1_SDRAM0 , 0, 0, CFA14_DetailCode0x10A1 );
		register uint32_t ul_Asic_1_SDRAM1 = ul_OneSubMemorySize[ 1 ][ 1 ];
		THRL_ThAssert( ul_StartAddr1 < ul_Asic_1_SDRAM1, "BufAddress Error Asic_1 SDRAM0 MemSize %"PRIdPTR", BufSize %"PRId32"", ul_StartAddr1, ul_Asic_1_SDRAM1 , 0, 0, CFA14_DetailCode0x10A2 );
	}
	
	THRL_ThLogMsgDebug(( "[[[[[[ SDRAM buffer address setting End [[[[[[\n" ));
}


// Function ------------------------------------------------------
// Overview : Asic1枚時の各SDRAMの初期化.
// History  : 2009/08/07 越野        Donau環境から分離して作成.
//            2008/08/11 R.Suzuki    CQ.309145 1Pass対応 関数名変更 init_SingleAsicSdramMode → init_AsicSdram_1.
//            2009/09/17 sugimoto    GaiaPF FAX用バッファをSDRAM1⇒0へ移動 モノクロ75ppm.
//            2009/09/29 T.Shima     AsicサブメモリHW仕様変更対応.
//            2009/10/02 岡田        Asicサブメモリリソース対応.
//            2012/09/03 小霜        UV-00877 Fax3or4回線対応.
//            2012/12/14 ADS坪井     CQ.377862  MS-LCL_22 Fax1回線/Fax2回線用のSDRAMの初期化は、該当するFaxオプションが有効の場合のみ実行するように変更.
//            2013/02/28 Sky         機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//            2013/03/13 Sky         ZeusS対応.
//            2013/07/10 ADS滕       CQ.381171(Act_66426) SC-SFM-23 サブメモリのファイルメモリ化.
//            2013/11/22 ADS岡村     CQ.381188(Act_69136) PS-SAC_36 シュリンク版統合ASIC対応.        ★TODO:PIC PC間遅延メモリの確保.
//            2014/04/10 ADS滕       CQ.384987(Act_72902) ソリューションモデル切替対応.
//            2014/04/25 ADS竹内     CQ.389967(Act_73361) モデル切替の判定条件を変更.
//            2014/12/05 ADS吉田     CQ.506801(Act_504828) AS-FSC_14 A3A4共通化(機能スケーラビリティ拡張).
//                                   ⇒機能オプション「TYPE_AppOptionSubMemSplitSave」を以下機能別に分離.
//                                     ・TYPE_AppOptionSubMemSplitSave ：サブメモリ可変化[1ページ確保動作]、スワップ分割保存・分割読出.
//                                     ・TYPE_AppOptionSubMemPartition ：サブメモリ割り当て切り替え対応.
//                                     ・TYPE_AppOptionSubFileMem      ：サブメモリのファイルメモリ化.
//                                     ・TYPE_AppOptionWorstSize       ：最悪圧縮率対応.
//            2014/12/17 ADS松本(考) CQ.506800(Act_504832) AS-MSC_12 A3/A4共通化FUM(メモリスケーラビリティ拡張).
//            2014/12/08 ADS森本	 CQ.506800(Act_504830) AS-MSC_6 A3A4共通化(メモリスケーラビリティ拡張).
//            2015/01/20 ADS森本     CQ.506800(Act_504831) AS-MSC_6 A3A4共通化(メモリスケーラビリティ拡張).
//                                   UT_CT_CIT不具合管理表No.56: A4機 PIC-PC間遅延メモリ動的対応.
//            2015/10/06 Sky		システム統合SoC(S800)対応.
//            2016/10/25 Sky合屋     [IT5_4.1]制御構成変更No.40 自動評価対応.
//
void THRC_BufMemory::init_AsicSdram_1( )
{
	THRL_ThLogMsgDebug( "[[[[[[ SDRAM buffer address setting Start [[[[[[\n" );

	register uint32_t ul_TmpBufSize = 0;	//! 仮のバッファサイズ格納用.
	
	///////////////////////////////////////////////////.
	//////////////// Asic 1個目 SDRAM0 ////////////////.
	///////////////////////////////////////////////////.
	//! メモリ開始アドレスは０.
	register uintptr_t ul_StartAddr = 0;
	THRL_ThLogMsgDebug( "== Asic 1st SDRAM0 start address 0x%08"PRIXPTR"H \n", ul_StartAddr );
	
	//! 画像変換後圧縮前バッファ.
	ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_ConvCmp );				//! 画像変換後圧縮前バッファ初期化.
	THRL_SetDbgBufPlace( THRE_BufId_ConvCmp, THRE_DebugBufPlace_ASIC_0 );					//! デバッグ用.
	ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
	
	//! アプリ出力画像合成伸張後バッファ.
	ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_CombineApp );			//! アプリ出力画像合成伸張後バッファ初期化.
	THRL_SetDbgBufPlace( THRE_BufId_CombineApp, THRE_DebugBufPlace_ASIC_0 );					//! デバッグ用.
	ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
	
	//! スキャナIF入力用圧縮バッファ.
	ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_ExtIn );				//! スキャナIF入力用バッファ初期化.
	THRL_SetDbgBufPlace( THRE_BufId_ExtIn, THRE_DebugBufPlace_ASIC_0 );						//! デバッグ用追加.
	ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
	
	//! Emperor入力用圧縮バッファ.
	ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_AppIn );				//! Emperor入力用圧縮バッファ初期化.
	THRL_SetDbgBufPlace( THRE_BufId_AppIn, THRE_DebugBufPlace_ASIC_0 );						//! デバッグ用追加.
	ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
	
	//! 回転&拡大プリント用伸張バッファ.
	ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_RotPrint );				//! 回転&拡大プリント用伸張バッファ初期化.
	THRL_SetDbgBufPlace( THRE_BufId_RotPrint, THRE_DebugBufPlace_ASIC_0 );					//! デバッグ用追加.
	ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
	
	//! スキャン出力用伸張バッファ.
	ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_ScnOut );				//! スキャン出力用伸張バッファ初期化.
	THRL_SetDbgBufPlace( THRE_BufId_ScnOut, THRE_DebugBufPlace_ASIC_0 );						//! デバッグ用追加.
	ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
	
	//! 画像変換前用伸張バッファ.
	ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_ConvExp );				//! 画像変換前用伸張バッファ初期化.
	THRL_SetDbgBufPlace( THRE_BufId_ConvExp, THRE_DebugBufPlace_ASIC_0 );					//! デバッグ用追加.
	ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
	
	//! Fax送信(iFax)用伸張バッファ.
	ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax0Tx );				//! Fax送信(iFax)用伸張バッファ初期化.
	THRL_SetDbgBufPlace( THRE_BufId_Fax0Tx, THRE_DebugBufPlace_ASIC_0 );						//! デバッグ用追加.
	ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
	
	//! Fax受信(iFax)用圧縮バッファ.
	ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax0Rx );				//! Fax受信(iFax)用圧縮バッファ初期化.
	THRL_SetDbgBufPlace( THRE_BufId_Fax0Rx, THRE_DebugBufPlace_ASIC_0 );						//! デバッグ用追加.
	ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
	
	Bool bl_IsAutoTestMode = APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( 195, 0 );
	// 自動評価有効な場合.
	if( True == bl_IsAutoTestMode ){
		//! スキャナIF入力用圧縮バッファ(擬似バッファ).
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_DummyExtIn );					// スキャナIF入力用バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_DummyExtIn, THRE_DebugBufPlace_ASIC_0 );					// デバッグ用追加.
		ul_StartAddr = _roundup( ( ul_StartAddr + ul_TmpBufSize ), (1 * 1024 ) ) * ( 1 * 1024 );	// 1KB協会へ割付切り上げ.
	}                                                                                               

#ifdef DEF_A3A4COMM // 2015/02/16 ADS森本 CQ.510156(Act_507195) 【A3A4共通化】コンパイルスイッチ対応(機能/メモリスケーラビリティ拡張分).
	if ( SYSF_getAppOptionStatus(TYPE_AppOptionFAX) == TYPE_AppOptSettingOn ){					// 2014/12/17 ADS松本(考) CQ.506800(Act_504832) AS-MSC_12 A3/A4共通化FUM(メモリスケーラビリティ拡張).
		if( APIC_SystemInfo::newInstance()->getFax1Exist() == True ){
			//! Fax送信(回線1)用伸張バッファ.
			ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax1Tx );				//! Fax送信(回線1)用伸張バッファ初期化.
			THRL_SetDbgBufPlace( THRE_BufId_Fax1Tx, THRE_DebugBufPlace_ASIC_0 );				//! デバッグ用追加.
			ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
			
			//! Fax受信(回線1)用圧縮バッファ.
			ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax1Rx );				//! Fax受信(回線1)用圧縮バッファ初期化.
			THRL_SetDbgBufPlace( THRE_BufId_Fax1Rx, THRE_DebugBufPlace_ASIC_0 );				//! デバッグ用追加.
			ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
		}
		
		if( APIC_SystemInfo::newInstance()->getFax2Exist() == True ){
			//! Fax送信(回線2)用伸張バッファ.
			ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax2Tx );				//! Fax送信(回線2)用伸張バッファ初期化.
			THRL_SetDbgBufPlace( THRE_BufId_Fax2Tx, THRE_DebugBufPlace_ASIC_0 );				//! デバッグ用追加.
			ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
			
			//! Fax受信(回線2)用圧縮バッファ.
			ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax2Rx );				//! Fax受信(回線2)用圧縮バッファ初期化.
			THRL_SetDbgBufPlace( THRE_BufId_Fax2Rx, THRE_DebugBufPlace_ASIC_0 );				//! デバッグ用追加.
			ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
		}
		
		if( APIC_SystemInfo::newInstance()->getFaxExistAll(TYPE_CommLine3) == True ){			// 2014/12/17 ADS松本(考) CQ.506800(Act_504832) AS-MSC_12 A3/A4共通化FUM(メモリスケーラビリティ拡張).
			//! Fax送信(回線3)用伸張バッファ.
			ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax3Tx );				//! Fax送信(回線3)用伸張バッファ初期化.
			THRL_SetDbgBufPlace( THRE_BufId_Fax3Tx, THRE_DebugBufPlace_ASIC_0 );				//! デバッグ用追加.
			ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
			
			//! Fax受信(回線3)用圧縮バッファ.
			ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax3Rx );				//! Fax受信(回線3)用圧縮バッファ初期化.
			THRL_SetDbgBufPlace( THRE_BufId_Fax3Rx, THRE_DebugBufPlace_ASIC_0 );				//! デバッグ用追加.
			ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
		}																						// 2014/12/17 ADS松本(考) CQ.506800(Act_504832) AS-MSC_12 A3/A4共通化FUM(メモリスケーラビリティ拡張).
		
		if( APIC_SystemInfo::newInstance()->getFaxExistAll(TYPE_CommLine4) == True ){			// 2014/12/17 ADS松本(考) CQ.506800(Act_504832) AS-MSC_12 A3/A4共通化FUM(メモリスケーラビリティ拡張).
			//! Fax送信(回線4)用伸張バッファ.
			ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax4Tx );				//! Fax送信(回線4)用伸張バッファ初期化.
			THRL_SetDbgBufPlace( THRE_BufId_Fax4Tx, THRE_DebugBufPlace_ASIC_0 );				//! デバッグ用追加.
			ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
			
			//! Fax受信(回線4)用圧縮バッファ.
			ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax4Rx );				//! Fax受信(回線4)用圧縮バッファ初期化.
			THRL_SetDbgBufPlace( THRE_BufId_Fax4Rx, THRE_DebugBufPlace_ASIC_0 );				//! デバッグ用追加.
			ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
		}																						// 2014/12/17 ADS松本(考) CQ.506800(Act_504832) AS-MSC_12 A3/A4共通化FUM(メモリスケーラビリティ拡張).
	}																							// 2014/12/17 ADS松本(考) CQ.506800(Act_504832) AS-MSC_12 A3/A4共通化FUM(メモリスケーラビリティ拡張).
#else // #ifdef DEF_A3A4COMM.
	if( APIC_SystemInfo::newInstance()->getFax1Exist() == True ){
		//! Fax送信(回線1)用伸張バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax1Tx );				//! Fax送信(回線1)用伸張バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Fax1Tx, THRE_DebugBufPlace_ASIC_0 );						//! デバッグ用追加.
		ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
		
		//! Fax受信(回線1)用圧縮バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax1Rx );				//! Fax受信(回線1)用圧縮バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Fax1Rx, THRE_DebugBufPlace_ASIC_0 );						//! デバッグ用追加.
		ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
	}
	
	if( APIC_SystemInfo::newInstance()->getFax2Exist() == True ){
		//! Fax送信(回線2)用伸張バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax2Tx );				//! Fax送信(回線2)用伸張バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Fax2Tx, THRE_DebugBufPlace_ASIC_0 );						//! デバッグ用追加.
		ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
		
		//! Fax受信(回線2)用圧縮バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax2Rx );				//! Fax受信(回線2)用圧縮バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Fax2Rx, THRE_DebugBufPlace_ASIC_0 );						//! デバッグ用追加.
		ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
	}
	
	//! Fax送信(回線3)用伸張バッファ.
	ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax3Tx );				//! Fax送信(回線3)用伸張バッファ初期化.
	THRL_SetDbgBufPlace( THRE_BufId_Fax3Tx, THRE_DebugBufPlace_ASIC_0 );						//! デバッグ用追加.
	ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
	
	//! Fax受信(回線3)用圧縮バッファ.
	ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax3Rx );				//! Fax受信(回線3)用圧縮バッファ初期化.
	THRL_SetDbgBufPlace( THRE_BufId_Fax3Rx, THRE_DebugBufPlace_ASIC_0 );						//! デバッグ用追加.
	ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
	
	//! Fax送信(回線4)用伸張バッファ.
	ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax4Tx );				//! Fax送信(回線4)用伸張バッファ初期化.
	THRL_SetDbgBufPlace( THRE_BufId_Fax4Tx, THRE_DebugBufPlace_ASIC_0 );						//! デバッグ用追加.
	ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.
	
	//! Fax受信(回線4)用圧縮バッファ.
	ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax4Rx );				//! Fax受信(回線4)用圧縮バッファ初期化.
	THRL_SetDbgBufPlace( THRE_BufId_Fax4Rx, THRE_DebugBufPlace_ASIC_0 );						//! デバッグ用追加.
	ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.

#endif // #ifdef DEF_A3A4COMM.

	//! エンジン出力用バッファ.
	ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_EngOut );				//! エンジン出力用バッファ初期化.
	THRL_SetDbgBufPlace( THRE_BufId_EngOut, THRE_DebugBufPlace_ASIC_0 );						//! デバッグ用.
	ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.

	//! エンジン画像合成用伸張バッファ.
	ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_CombineEng );			//! エンジン画像合成用伸張バッファ初期化.
	THRL_SetDbgBufPlace( THRE_BufId_CombineEng, THRE_DebugBufPlace_ASIC_0 );					//! デバッグ用追加.
	ul_StartAddr = _roundup( (ul_StartAddr+ul_TmpBufSize), (1*1024) ) * (1*1024);		//! １KByte境界へ割り付け切り上げ.

#ifdef DEF_Soc1200
#ifdef DEF_A3A4COMM // 2015/02/16 ADS森本 CQ.510156(Act_507195) 【A3A4共通化】コンパイルスイッチ対応(機能/メモリスケーラビリティ拡張分).
	// 2014/12/08 ADS森本	CQ.506800(Act_504830) AS-MSC_6 A3A4共通化(メモリスケーラビリティ拡張).
	// 機能オプション対応にて、サブメモリのファイルメモリ確保対応.
	register uint32_t ul_Asic_0_SDRAM = ul_OneSubMemorySize[ 0 ][ 0 ];// 2015/01/20 ADS森本 CQ.506800(Act_504831) AS-MSC_6 A3A4共通化(メモリスケーラビリティ拡張).
	if( True == SYSF_getAppOptionStatus(TYPE_AppOptionSubFileMem) ){		// 機能判定結果が有効の場合.
		if( TYPE_CapacityA3 == APIC_OptionSetting::newInstance()->getCapacity() ){	// 機種 A3A4判定.
			// A3機の場合.
			ul_SubFileMemoryStartAddr = ul_StartAddr;						// サブファイルメモリの開始アドレスを設定.
			ul_SubFileMemorySize = (uint32_t)THRD_SubFileMemorySize_S800;	// サブファイルメモリのサイズを設定.
		}
		else {
			// A4機の場合.
			ul_SubFileMemoryStartAddr = ul_StartAddr;						// サブファイルメモリの開始アドレスを設定.
			register uint32_t ul_SubFileMemorySizeMax = ul_Asic_0_SDRAM - ul_StartAddr;	// サブファイルメモリとして割り当て可能な最大サイズ. // 2015/01/20 ADS森本 CQ.506800(Act_504831) AS-MSC_6 A3A4共通化(メモリスケーラビリティ拡張).
			// サブメモリの余剰領域をサブファイルメモリに割り当てる.
			ul_SubFileMemorySize = ul_SubFileMemorySizeMax;					// サブファイルメモリのサイズを割り当て可能な最大サイズに設定.
		}
	}
	else {
		ul_SubFileMemorySize = 0;											//! サブファイルメモリを0に設定する.
	}
	ul_StartAddr = _roundup( (ul_StartAddr+ul_SubFileMemorySize), (1*1024) ) * (1*1024);	//! １KByte境界へ割り付け切り上げ.
	
	//! メモリ領域をはみ出していないかチェック.
	THRL_ThAssert( ul_StartAddr <= ul_Asic_0_SDRAM, "BufAddress Error Asic_0 SDRAM MemSize %d, BufSize %d", ul_StartAddr, ul_Asic_0_SDRAM , 0, 0, CFA14_DetailCode0x0895 );
#else // #ifdef DEF_A3A4COMM.
	if( True == SYSF_getAppOptionStatus(TYPE_AppOptionSubMemSplitSave) ){		//! 機能判定結果が有効の場合.
		ul_SubFileMemoryStartAddr = ul_StartAddr;										//! サブファイルメモリの開始アドレスを設定.
		ul_SubFileMemorySize = (uint32_t)THRD_SubFileMemorySize_S800;					//! サブファイルメモリのサイズを設定.
		ul_StartAddr = _roundup( (ul_StartAddr+ul_SubFileMemorySize), (1*1024) ) * (1*1024);	//! １KByte境界へ割り付け切り上げ.
	} else {
		ul_SubFileMemorySize = 0;	//! サブファイルメモリを0に設定する.
	}
	//! メモリ領域をはみ出していないかチェック.
	register uint32_t ul_Asic_0_SDRAM = ul_OneSubMemorySize[ 0 ][ 0 ];
	THRL_ThAssert( ul_StartAddr <= ul_Asic_0_SDRAM, "BufAddress Error Asic_0 SDRAM MemSize %"PRIdPTR", BufSize %"PRId32"", ul_StartAddr, ul_Asic_0_SDRAM , 0, 0, CFA14_DetailCode0x0895 );
#endif // #ifdef DEF_A3A4COMM.
#else	// DEF_Soc1200.
	//! メモリ領域をはみ出していないかチェック.
	register uint32_t ul_Asic_0_SDRAM = ul_OneSubMemorySize[ 0 ][ 0 ];
	THRL_ThAssert( ul_StartAddr < ul_Asic_0_SDRAM, "BufAddress Error Asic_0 SDRAM MemSize %"PRIdPTR", BufSize %"PRId32"", ul_StartAddr, ul_Asic_0_SDRAM , 0, 0, CFA14_DetailCode0x0895 );
#endif	// DEF_Soc1200.

	THRL_ThLogMsgDebug(( "[[[[[[ SDRAM buffer address setting End [[[[[[\n" ));
}


// ================================================================================================.
//! <!--機能-->スレッド使用分のPCIメモリ上のメモリの初期化.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! スレッド使用分のPCIメモリ上のメモリのアドレス、サイズ等の情報を設定する\n.
//! <!--履歴-->.
//   09/08/07 越野	  Donau環境から分離して作成.
// 2009/10/02 岡田	  Asicメインメモリリソース対応.
//   10/09/24 世古	  FPGA基板装着状態取得関数名変更(CQ:xxxxxx).
// 2010/12/13 tadamasa	Fiery1200dpi対応.
// 2012/02/03 戸田		ファイルメモリ分割対応.
//   13/02/28 Sky     機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//   13/03/13 Sky     ZeusS対応.
// 2014/01/09 JTE酒井 【DS-BIM_1】文字印字スタンプ有効時のみPCIメモリの初期化を実施するよう修正.
// 2014/04/10 ADS滕   CQ.384987(Act_72902) ソリューションモデル切替対応.
// 2014/04/25 ADS竹内 CQ.389967(Act_73361) モデル切替の判定条件を変更.
// 2014/07/08 tadamasa 制御構成変更No241 全モデルでbizhubRAの使用メモリを確保するように修正.
// 2014/09/03 Sky山田 CQ.503177 AndroidとRTIありのFax受信が同時に動くと異常画像になる対応(RTI合成用バッファメモリ確保).
// 2014/12/08 ADS森本 CQ.506800(Act_504830) AS-MSC_6 A3A4共通化(メモリスケーラビリティ拡張).
// 2015/10/06 Sky     システム統合SoC(S800)対応.
// 2017/04/07 Sky	  ZeusSMLKPF対応.
// 2017/05/01 日立 OP/PP共通化.
// 2018/11/20 Sky	  DenebMLK 合成画像用領域（地紋、ウォーターマーク用）有効化.
// 2019/10/03 Sky合屋 【SparrowBK】スタンプオプション化設定が無効の時に、合成画像用領域/合成画像用領域（地紋、ウォーターマーク用）/合成画像用領域（回転用）/プレビュー用領域 有効化.
// 2020/01/16 Sky合屋 【EagleZ+】OP_BTS-19356 ラインカード起動の判定処理追加.
// 2023/01/10 Sky	  Threadメモリ使用領域チェック対応.
// ================================================================================================.
void THRC_BufMemory::init_ThreadUsePciMode( )
{
	THRL_ThLogMsgDebug( "[[[[[[ PCI buffer address setting Start [[[[[[\n" );

	register uint32_t ul_TmpBufSize = 0;	//! 仮のバッファサイズ格納用.
	register uintptr_t ul_StartAddr = 0;
	
//! Simの時も、仮のアドレスを利用する(実機と同じアドレスでデバッグしやすいため).
//! ただし、連番データ等をSimでも書き込む場合がまずいので、連番を各部分を作る時には注意！.
#if defined (TORNADO) || defined (SIMULATOR)
	//! 実機設定.
	ul_StartAddr = (uintptr_t)THRF_GetThreadBufferTopAdrOnMainMemory();
#endif

	THRL_ThLogMsgDebug( "== PCI start address 0x%08"PRIXPTR"H \n", ul_StartAddr );
	
	//! メインバッファ(画像合成、Fax用圧縮データ展開用).
	ul_TmpBufSize = init_BufMemoryCom(ul_StartAddr, THRE_BufId_Main );					//! メインバッファ初期化.
	THRL_SetDbgBufPlace( THRE_BufId_Main, THRE_DebugBufPlace_Main );							//! デバッグ用.
	ul_StartAddr += ul_TmpBufSize;														//! アドレス更新.
		
	//! 合成画像用領域.
	ul_TmpBufSize = init_BufMemoryCom(ul_StartAddr, THRE_BufId_Main_Combine );			//! 合成画像用領域初期化.
	THRL_SetDbgBufPlace( THRE_BufId_Main_Combine, THRE_DebugBufPlace_Main );					//! デバッグ用.
	ul_StartAddr += ul_TmpBufSize;														//! アドレス更新.
		
	//! 合成画像用領域（地紋、ウォーターマーク用）.
	ul_TmpBufSize = init_BufMemoryCom(ul_StartAddr, THRE_BufId_Main_CombineJimon );		//! 合成画像用領域初期化.
	THRL_SetDbgBufPlace( THRE_BufId_Main_CombineJimon, THRE_DebugBufPlace_Main );			//! デバッグ用.
	ul_StartAddr += ul_TmpBufSize;														//! アドレス更新.
		
	//! 合成画像用領域（回転用）.
	ul_TmpBufSize = init_BufMemoryCom(ul_StartAddr, THRE_BufId_Main_Rot );				//! 合成画像用領域初期化.
	THRL_SetDbgBufPlace( THRE_BufId_Main_Rot, THRE_DebugBufPlace_Main );						//! デバッグ用.
	ul_StartAddr += ul_TmpBufSize;														//! アドレス更新.

// 2012/04/10 Gonsho DS基板JPEGコーダ対応 Start.
	ul_TmpBufSize = init_BufMemoryCom(ul_StartAddr, THRE_BufId_Main_AndroidJpegCompIn );			//! Android JPEG用.
	THRL_SetDbgBufPlace( THRE_BufId_Main_AndroidJpegCompIn, THRE_DebugBufPlace_Main );	//! デバッグ用
	ul_StartAddr += ul_TmpBufSize;

	ul_TmpBufSize = init_BufMemoryCom(ul_StartAddr, THRE_BufId_Main_AndroidJpegCompOut );			//! Android JPEG用.
	THRL_SetDbgBufPlace( THRE_BufId_Main_AndroidJpegCompOut, THRE_DebugBufPlace_Main );	//! デバッグ用
	ul_StartAddr += ul_TmpBufSize;
// 2012/04/10 Gonsho DS基板JPEGコーダ対応 End.
	//! プレビュー用領域.
	ul_TmpBufSize = init_BufMemoryCom(ul_StartAddr, THRE_BufId_Main_Preview );			//! プレビュー用領域初期化.
	THRL_SetDbgBufPlace( THRE_BufId_Main_Preview, THRE_DebugBufPlace_Main );					//! デバッグ用.
	ul_StartAddr += ul_TmpBufSize;														//! アドレス更新.

#if !defined(DEF_DenebMLKCus) // DenebMLK メモリ4G対応.
#ifndef COSIM		//! Cosimの時はHDDの接続が無いので、インスタンスの取得はできないが、多値合成領域の初期化は必要.
	APIC_HddInfo *pc_HddInfo = APIC_HddInfo::newInstance();
	if( APIC_Enabler::newInstance()->isExtendedMemory() == True && 						// 増設メモリ装着判定.
		pc_HddInfo != NULL && 
		pc_HddInfo->getHddExist() == True ) {
#endif
		//! 多値合成用領域(Form).
		ul_TmpBufSize = init_BufMemoryCom(ul_StartAddr, THRE_BufId_Main_SynthesisForm );	//! 多値合成用領域初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Main_Preview, THRE_DebugBufPlace_Main );					//! デバッグ用.
		ul_StartAddr += ul_TmpBufSize;														//! アドレス更新.
		//! 多値合成用領域(In).
		ul_TmpBufSize = init_BufMemoryCom(ul_StartAddr, THRE_BufId_Main_SynthesisIn );		//! 多値合成用領域初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Main_Preview, THRE_DebugBufPlace_Main );					//! デバッグ用.
		ul_StartAddr += ul_TmpBufSize;														//! アドレス更新.
#ifndef COSIM
	}
#endif
#endif	//#if !defined(DEF_DenebMLKCus).

	if( False == THRF_IsA800IntegrationExist() ) { 
		//! スキャナIF入力用圧縮バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_ExtIn );						//! スキャナIF入力用バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_ExtIn, THRE_DebugBufPlace_Main );							//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.

		//! Emperor入力用圧縮バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_AppIn );						//! Emperor入力用圧縮バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_AppIn, THRE_DebugBufPlace_Main );							//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.

		//! 画像変換後圧縮前バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_ConvCmp );						//! 画像変換後圧縮前バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_ConvCmp, THRE_DebugBufPlace_Main );							//! デバッグ用.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.

		//! 回転&拡大プリント用伸張バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_RotPrint );						//! 回転&拡大プリント用伸張バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_RotPrint, THRE_DebugBufPlace_Main );						//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.
		
		//! スキャン出力用伸張バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_ScnOut );						//! スキャン出力用伸張バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_ScnOut, THRE_DebugBufPlace_Main );							//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.
		
		//! 画像変換前用伸張バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_ConvExp );						//! 画像変換前用伸張バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_ConvExp, THRE_DebugBufPlace_Main );							//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.

		//! アプリ出力画像合成伸張後バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_CombineApp );					//! アプリ出力画像合成伸張後バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_CombineApp, THRE_DebugBufPlace_Main );						//! デバッグ用.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.

		//! Fax送信(iFax)用伸張バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax0Tx );						//! Fax送信(iFax)用伸張バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Fax0Tx, THRE_DebugBufPlace_Main );							//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.
		
		//! Fax受信(iFax)用圧縮バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax0Rx );						//! Fax受信(iFax)用圧縮バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Fax0Rx, THRE_DebugBufPlace_Main );							//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.

		if( APIC_SystemInfo::newInstance()->getFax1Exist() == True ){
			//! Fax送信(回線1)用伸張バッファ.
			ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax1Tx );					//! Fax送信(回線1)用伸張バッファ初期化.
			THRL_SetDbgBufPlace( THRE_BufId_Fax1Tx, THRE_DebugBufPlace_Main );						//! デバッグ用追加.
			ul_StartAddr += ul_TmpBufSize;															//! アドレス更新.
			
			//! Fax受信(回線1)用圧縮バッファ.
			ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax1Rx );					//! Fax受信(回線1)用圧縮バッファ初期化.
			THRL_SetDbgBufPlace( THRE_BufId_Fax1Rx, THRE_DebugBufPlace_Main );						//! デバッグ用追加.
			ul_StartAddr += ul_TmpBufSize;															//! アドレス更新.
		}
		
#if !defined(DEF_SparrowCus)
		if( APIC_SystemInfo::newInstance()->getFax2Exist() == True ){
			//! Fax送信(回線2)用伸張バッファ.
			ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax2Tx );					//! Fax送信(回線2)用伸張バッファ初期化.
			THRL_SetDbgBufPlace( THRE_BufId_Fax2Tx, THRE_DebugBufPlace_Main );						//! デバッグ用追加.
			ul_StartAddr += ul_TmpBufSize;															//! アドレス更新.
			
			//! Fax受信(回線2)用圧縮バッファ.
			ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax2Rx );					//! Fax受信(回線2)用圧縮バッファ初期化.
			THRL_SetDbgBufPlace( THRE_BufId_Fax2Rx, THRE_DebugBufPlace_Main );						//! デバッグ用追加.
			ul_StartAddr += ul_TmpBufSize;															//! アドレス更新.
		}
		
#if !defined(DEF_DenebMLKCus) // DenebMLK メモリ4G対応.
		//! Fax送信(回線3)用伸張バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax3Tx );						//! Fax送信(回線3)用伸張バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Fax3Tx, THRE_DebugBufPlace_Main );							//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.
		
		//! Fax受信(回線3)用圧縮バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax3Rx );						//! Fax受信(回線3)用圧縮バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Fax3Rx, THRE_DebugBufPlace_Main );							//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.
		
		//! Fax送信(回線4)用伸張バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax4Tx );						//! Fax送信(回線4)用伸張バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Fax4Tx, THRE_DebugBufPlace_Main );							//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.
		
		//! Fax受信(回線4)用圧縮バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax4Rx );						//! Fax受信(回線4)用圧縮バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Fax4Rx, THRE_DebugBufPlace_Main );							//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.
#endif	//#if !defined(DEF_DenebMLKCus).
#endif // #if !defined(DEF_SparrowCus).

		//! エンジン出力用バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_EngOut );						//! エンジン出力用バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_EngOut, THRE_DebugBufPlace_Main );							//! デバッグ用.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.

		//! エンジン画像合成用伸張バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_CombineEng );					//! エンジン画像合成用伸張バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_CombineEng, THRE_DebugBufPlace_Main );						//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.

		//! スキャナ入力RAWデータバッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_ScnInRaw );						//! スキャナ入力RAWデータバッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_ScnInRaw, THRE_DebugBufPlace_Main );						//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.
		
#if !defined(DEF_DenebMLKCus) // DenebMLK メモリ4G対応.
		//! スキャナ入力RAWデータバッファ(裏面).
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_ScnInRaw_Dup );					//! スキャナ入力RAWデータバッファ(裏面)初期化.
		THRL_SetDbgBufPlace( THRE_BufId_ScnInRaw_Dup, THRE_DebugBufPlace_Main );					//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.
#endif	//#if !defined(DEF_DenebMLKCus).
		
		//! スキュー補正データバッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_SkewCorr );						//! スキュー補正データバッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_SkewCorr, THRE_DebugBufPlace_Main );						//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.

		//! RGBデータ遅延バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_RGBDelay );						//! RGBデータ遅延バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_RGBDelay, THRE_DebugBufPlace_Main );						//! デバッグ用.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.

		//! 回転前バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Rotate );						//! 回転前バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Rotate, THRE_DebugBufPlace_Main );							//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.

	}else{
		//! スキャナ入力RAWデータバッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_ScnInRaw );						//! スキャナ入力RAWデータバッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_ScnInRaw, THRE_DebugBufPlace_Main );						//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.
		
		//! スキャナ入力RAWデータバッファ(裏面).
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_ScnInRaw_Dup );					//! スキャナ入力RAWデータバッファ(裏面)初期化.
		THRL_SetDbgBufPlace( THRE_BufId_ScnInRaw_Dup, THRE_DebugBufPlace_Main );					//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.
		
		//! スキュー補正データバッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_SkewCorr );						//! スキュー補正データバッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_SkewCorr, THRE_DebugBufPlace_Main );						//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.

		//! RGBデータ遅延バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_RGBDelay );						//! RGBデータ遅延バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_RGBDelay, THRE_DebugBufPlace_Main );						//! デバッグ用.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.

		//! スキャナIF入力用圧縮バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_ExtIn );						//! スキャナIF入力用バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_ExtIn, THRE_DebugBufPlace_Main );							//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.

		if( APIC_SystemInfo::newInstance()->getFax1Exist() == True ){
			//! Fax受信(回線1)用圧縮バッファ.
			ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax1Rx );					//! Fax受信(回線1)用圧縮バッファ初期化.
			THRL_SetDbgBufPlace( THRE_BufId_Fax1Rx, THRE_DebugBufPlace_Main );						//! デバッグ用追加.
			ul_StartAddr += ul_TmpBufSize;															//! アドレス更新.
		}
		
		if( APIC_SystemInfo::newInstance()->getFax2Exist() == True ){
			//! Fax受信(回線2)用圧縮バッファ.
			ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax2Rx );					//! Fax受信(回線2)用圧縮バッファ初期化.
			THRL_SetDbgBufPlace( THRE_BufId_Fax2Rx, THRE_DebugBufPlace_Main );						//! デバッグ用追加.
			ul_StartAddr += ul_TmpBufSize;															//! アドレス更新.
		}
		
		//! Fax受信(回線3)用圧縮バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax3Rx );						//! Fax受信(回線3)用圧縮バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Fax3Rx, THRE_DebugBufPlace_Main );							//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.
		
		//! Fax受信(回線4)用圧縮バッファ.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Fax4Rx );						//! Fax受信(回線4)用圧縮バッファ初期化.
		THRL_SetDbgBufPlace( THRE_BufId_Fax4Rx, THRE_DebugBufPlace_Main );							//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.

		//! アプリ入力用圧縮前バッファ（メイン用）.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Main_AppIn );					//! アプリ入力用圧縮前バッファ（メイン用）.
		THRL_SetDbgBufPlace( THRE_BufId_Main_AppIn, THRE_DebugBufPlace_Main );						//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.

		//! 画像変換用圧縮前バッファ（メイン用）.
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_Main_ConvCmp );					//! 画像変換用圧縮前バッファ（メイン用）.
		THRL_SetDbgBufPlace( THRE_BufId_Main_ConvCmp, THRE_DebugBufPlace_Main );					//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.

#ifdef DEF_PP_Cus
		//! 工程スキャン用出力バッファ (OP/PP共通化).
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_RawImageScan );					//! 工程スキャン用出力バッファ.
		THRL_SetDbgBufPlace( THRE_BufId_RawImageScan, THRE_DebugBufPlace_Main );					//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.

		//! 工程スキャン用出力バッファ(裏面) (OP/PP共通化).
		ul_TmpBufSize = init_BufMemoryCom( ul_StartAddr, THRE_BufId_RawImageScan_Dup );				//! 工程スキャン用出力バッファ(裏面).
		THRL_SetDbgBufPlace( THRE_BufId_RawImageScan_Dup, THRE_DebugBufPlace_Main );				//! デバッグ用追加.
		ul_StartAddr += ul_TmpBufSize;																//! アドレス更新.
#endif	// #ifdef DEF_PP_Cus.

	}

	ul_StartAddr -= (uintptr_t)THRF_GetThreadBufferTopAdrOnMainMemory();
	// Threadメモリ使用領域チェック対応.
	register uintptr_t ul_MemRangeStart = (uintptr_t)THRF_GetThreadBufferTopAdrOnMainMemory();
	register uintptr_t ul_MemRangeSize  = GetMemPartSizeFromPartID( memID_BufImage ) - THRD_MARGIN_PART;
	THRF_SetUsageMemRange(THRE_MemBufImage_Main, ul_MemRangeStart, (size_t)ul_MemRangeSize);

#ifdef DEF_A3A4COMM	
	if( True == SYSF_getAppOptionMemStatus( TYPE_AppOptionSubFileMem ) ){
		// サブアフィルメモリが有効.
		register uintptr_t ul_StartTmpBufAddr = 0;
		 // EncodeTmpBufferの先頭アドレスを追加.
		ul_StartTmpBufAddr = (uintptr_t)THRF_GetEncodeTmpBufferAddr();
		 
		ul_TmpBufSize = init_BufMemoryCom(ul_StartTmpBufAddr, THRE_BufId_Main_TmpEncode );			//! 暗号化用一時バッファ.
		THRL_SetDbgBufPlace( THRE_BufId_Main_TmpEncode, THRE_DebugBufPlace_Main );	//! デバッグ用
		ul_StartTmpBufAddr += ul_TmpBufSize;
		
		ul_TmpBufSize = init_BufMemoryCom(ul_StartTmpBufAddr, THRE_BufId_Main_TmpDecode );			//! 復号化用一時バッファ.
		THRL_SetDbgBufPlace( THRE_BufId_Main_TmpDecode, THRE_DebugBufPlace_Main );	//! デバッグ用
		ul_StartTmpBufAddr += ul_TmpBufSize;
	}
#endif	// #ifdef DEF_A3A4COMM.
	
	if( THRF_IsAsicFpga() == True ) {
		THRL_ThLogMsgErr("THRC_BufMemory::init_ThreadUsePciMode() No Assert check ul_StartAddr=0x%"PRIxPTR"\n",ul_StartAddr);
		return;
	}

#ifndef COSIM
// HCL Modified for SimLinux launch purpose starts.
#ifndef SIMULATOR
	if( 7 != uc_BootMode ){ //ラインカード起動でない.
		THRL_ThAssert( ul_StartAddr <= (GetMemPartSizeFromPartID( memID_BufImage )-THRD_MARGIN_PART),"BufAddress Error ul_StartAddr = %d", ul_StartAddr , 0, 0, 0, CFA14_DetailCode0x0896 );
	}
#endif
// HCL Modified for SimLinux launch purpose ends.
#endif

	THRL_ThLogMsgDebug(( "[[[[[[ PCI buffer address setting End [[[[[[\n" ));
}


// ================================================================================================.
//! <!--機能-->SDRAMバッファメモリの初期化 共通.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  ul_StartAddr	：開始アドレス.
//! \param  e_BufId			：バッファID.
//! <!--戻値-->.
//! \return ulong			：バッファサイズ.
//! <!--詳細-->.
//! SDRAMのバッファメモリのサイズ初期化を行う\n.
//! <!--履歴-->.
//  09/08/07 越野	 Donau環境から分離して作成.
//  12/02/03 戸田	 ファイルメモリ分割対応.
//  12/11/21 ADS坪井 CQ.377856  MS-LCL_3 THRC_BufferResourceインスタンス動的確保変更に伴う修正.
//  13/02/28 Sky 機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
// ================================================================================================.
uint32_t THRC_BufMemory::init_BufMemoryCom( uintptr_t ul_StartAddr, THRE_BufId e_BufId )
{
	THRS_SubBufParaCom	s_Param;

	register uint32_t ul_BufAllSize = 0;														//! 総バッファサイズ.

	GetBufMemoryInfo( e_BufId, THRE_BufMode_Normal, s_Param);								//! 各バッファのサイズ取得.

	if( NULL == pc_BufResMan[e_BufId] ){
	pc_BufResMan[e_BufId] = new THRC_BufferResource;
	THRL_ThAssert( pc_BufResMan[e_BufId] != NULL, "THRC_BufMemory::init_BufMemoryCom Alloc Error! e_BufId=%d" , e_BufId, 0, 0, 0, CFA14_DetailCode0x0B74 );
	pc_BufResMan[e_BufId]->initBufId( e_BufId );	//! バッファID初期化.
	}
	pc_BufResMan[e_BufId]->initBufArea( ul_StartAddr, s_Param.ul_BandLine );		//! バッファ情報初期化.
	pc_BufResMan[e_BufId]->setBuffer( THRE_BufMode_Normal, s_Param.ul_BufWidth, s_Param.us_BandMax, s_Param.us_PlaneMax );
	ul_BufAllSize = ( (s_Param.ul_BufWidth / 8) * s_Param.ul_BandLine * s_Param.us_BandMax * s_Param.us_PlaneMax );

	return( ul_BufAllSize );
}


// ================================================================================================.
//! <!--機能-->SDRAMバッファメモリパラメータ取得.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param  e_BufId			：バッファID.
//! \param  e_BufMode		：バッファモード.
//! \param  s_Param			：バッファパラメータ.
//! <!--戻値-->.
//! \return Bool			：処理結果(True：正常、False：異常).
//! <!--詳細-->.
//! SDRAMのバッファメモリ情報(幅、バンド数、プレーン数、バンドライン数)の検索を行い設定する\n.
//! Band数などは、THRC_ThreadifDonau.hの定義値を参照している。\n.
//! (定義名でテーブルに記載すると見づらくなるため) \n.
//! <!--履歴-->.
//  09/08/07 越野	 	Donau環境から分離して作成.
//	09/09/17 sugimoto		GaiaPF 6バンドを10バンドへ拡張 モノクロ75ppm対応.
//  09/09/29 T.Shima	AsicサブメモリHW仕様変更対応. ※※※ COSIM側未対応 ※※※.
//	09/10/02 岡田		Asicサブメモリリソース対応.
//	10/04/05 太田		SoC1200 多値合成_強制付加印字対応.THRE_BufId_Main_SynthesisInのPlane数を14→13に変更.
//						[理由]多値合成において、THRE_BufId_Main_SynthesisInへ転送するDMA02がYMCK_Att2のデ-タをBTC圧縮デ-タで.
//							  出力することができない為、DMA02を用いてSDRAMからTHRE_BufId_Main_SynthesisInへ.
//							  転送せず、JBIG伸張後のSDRAM(THRE_BufId_ConvExp)上のデ-タを直接DMA04の入力とする.
//							  従って、THRE_BufId_Main_SynthesisInの確保すべき領域サイズは従来の最大Plane数であった.
//							  YMCK_Att2(14プレ-ン)でなく、RGB_Att4(13プレ-ン)で考慮するようにする.
//	10/04/21 太田		SoC1200 多値合成_強制付加印字 CT不具合対応[SoC問題点リストNo.368].
//	10/07/26 太田		オプションAsic動的切り替え対応.
//	10/09/10 木村		ジョブ毎のバッファサイズテーブルにCosim 3rd用の定義追加.
//						Cosim 3rd(Cosim Large)ではサブメモリは実機共通のテーブルを使用し、メインメモリは実機と異なる定義を使用.
//						Cosim 3rd用とCosim 3rd以外のBand,Plane,Line,Widthデータはメモリ仕様通り修正.
// 2010/12/13 tadamasa Fiery1200dpi対応.
// 2010/12/20 大石		Asic30対応.
// 2011/09/10 山下		アクセラレータ Asicオプション化・Socオプション化対応見直し
//                      ・THRS_SubBufParaComに追加されていたAsic種別は不要であるため削除.
//                      ・アクセラレータで追加されたフラグは不要であるため削除.
//                      ・Socの実装にあわせて、接続Asicの状態を考慮し、THRE_BufId_ExtInDup/THRE_BufId_ExtIfIn.
//                      　のテーブルデータを決定する対応を追加.
// 2012/02/03 戸田		ファイルメモリ分割対応 仮想⇒物理アドレス変換フラグ追加.
//						仮想BufImageは仮想アドレスなので、変換フラグはTrue.
//						物理BufImageとサブメモリは物理アドレスなので、変換フラグはFalse.
// 2012/09/03 小霜		UV-00877 Fax3or4回線対応.
// 2012/12/03 北澤　　　静的解析対応.
//   13/02/28 Sky       機種共通化対応 THRC_ResourceCommonSoC.cppから移植.
//   13/03/13 Sky       ZeusS対応.
// 2013/11/22 ADS岡村	CQ.381188(Act_69136) PS-SAC_36 シュリンク版統合ASIC対応.
// 2013/12/27 ADS本田	CQ.381621(Act_69974) Emperor入力用圧縮バッファを半減(14MB→7MB).
// 2014/04/10 ADS滕		CQ.384987(Act_72902) ソリューションモデル切替対応.
// 2014/04/25 ADS竹内   CQ.389967(Act_73361) モデル切替の判定条件を変更.
// 2014/08/07 Sky小倉	CQ.501488 PC-FAX送信時、「T73」のエラーが発生した対応（Fax用バッファとエンジン合成用バッファを戻す）.
// 2014/09/03 Sky山田	CQ.503177 AndroidとRTIありのFax受信が同時に動くと異常画像になる対応(RTI合成用A3サイズのバッファメモリ確保).
// 2014/12/05 ADS吉田   CQ.506801(Act_504828) AS-FSC_14 A3A4共通化(機能スケーラビリティ拡張).
//                      ⇒機能オプション「TYPE_AppOptionSubMemSplitSave」を以下機能別に分離.
//                        ・TYPE_AppOptionSubMemSplitSave ：サブメモリ可変化[1ページ確保動作]、スワップ分割保存・分割読出.
//                        ・TYPE_AppOptionSubMemPartition ：サブメモリ割り当て切り替え対応.
//                        ・TYPE_AppOptionSubFileMem      ：サブメモリのファイルメモリ化.
//                        ・TYPE_AppOptionWorstSize       ：最悪圧縮率対応.
// 2014/12/08 ADS森本	CQ.506800(Act_504830) AS-MSC_6 A3A4共通化(メモリスケーラビリティ拡張).
// 2015/02/19 Sky井柳	CQ.510128対応(Fiery入力サブメモリサイズ変更：6バンド(24MB･6MB) → 62バンド(248M･62MB).
// 2015/02/23 Sky下垣	CQ.508058 ソリューション 1200dpi異常画像 THRE_BufId_AppInのバッファ拡張.
// 2015/03/10 tadamasa  CQ508058 ソリューション 1200dpi異常画像 THRE_BufId_AppInのバッファ拡張　正式対応.
// 2015/04/24 tadamasa  CQ512331 ソリューション THRE_BufId_Mainのバンド数を39→43バンドに変更する.
// 2015/10/06 Sky     	システム統合SoC(S800)対応.
//! 2015/11/30 Sky井柳	[ZeusS ZX0]CQ520979対応：10.1インチ対応によるbizhubRA JPEG圧縮用領域サイズ変更.
// 2016/10/17 Sky千葉	AR1006215【市場問題マージ対応】SRA3,両面印刷,1200dpiでIC416から印刷を実行すると、C-FA14 及びC-40C5が発生する.
//  16/10/25 Sky合屋    [IT5_4.1]制御構成変更No.40 自動評価対応.
//! 2017/04/07 Sky		ZeusSMLKPF対応.
//! 2017/04/28 日立	 OP/PP共通化.
//! 2017/06/29 Sky      OP/PP共通化.エンジンバッファID追加の対応.
// 2018/11/20 Sky	    DenebMLK 長尺でFAXできない不具合対応.
//! 2019/04/16 日立伊藤史 AR1075848 SparrowMFP リストアバッファサイズ誤り修正.
//! 2019/07/09 Sky井柳 [SparrowBK/HeliosMLK] 制御構成変更No.6 マシンタイプ対応.
//! 2019/09/09 日立畔柳[SparrowBK]OP_BTS-10868 【開発提起】Emu機のマシンタイプ設定不可.
//! 2019/09/09 日立築地[EagleZ+]OP_BTS-19053_Android_IOSからBizhubRAを接続して、端末側画面には金色横線が表示された(領域不足).
//! 2021/08/18 Sky		MinervaSSBKのA3 FCOT対応でA800のみTHRE_BufId_EngOutを拡張.
// ================================================================================================.
void THRC_BufMemory::GetBufMemoryInfo( THRE_BufId e_BufId, THRE_BufMode e_BufMode, THRS_SubBufParaCom &s_Param )
{
	struct THRS_SubBufPara {
		THRE_BufId			e_SearchBufId;			//! 検索対象バッファID.
#if defined(COSIM)
		THRS_SubBufParaCom	s_Param;
#else	// defined(COSIM).
		THRS_SubBufParaCom	s_Param[THRE_SubParam_Num];	//! バッファ情報. // 2013/06/25 ADS森本	CQ.381621(Act_66306) SP-SAP-19	サブメモリ可変化 サブメモリ構成設定(No.1)対応.
#endif	// defined(COSIM).
	};
	static THRS_SubBufPara s_ParamCode[] = {
#if defined(COSIM) && !defined(LARGE) // SoCCosim3rd LARGEビルド時は実機用のテーブルを参照する.
		{ THRE_BufId_Main,              { 112,  1,     16, DEF_THRBUF_WIDTH_NORMAL, False }},	//! THRE_BufId_Main.
		{ THRE_BufId_Main_Combine,      { 1,    1,     16, DEF_THRBUF_WIDTH_NORMAL, False }},	//! THRE_BufId_Main_Combine.
		{ THRE_BufId_Main_CombineJimon, { 1,    1,     16, DEF_THRBUF_WIDTH_NORMAL, False }},	//! THRE_BufId_Main_CombineJimon.
		{ THRE_BufId_Main_Rot,          { 1,    1,     16, DEF_THRBUF_WIDTH_NORMAL, False }},	//! THRE_BufId_Main_Rot.
		{ THRE_BufId_Main_Preview,      { 1,    1,    256, 256                    , False }},	//! THRE_BufId_Main_Preview.
		//! PCI側なので、1色8プレーンとして持つ。(1色1プレーンとすると、属性部分とメモリ横幅が合わなくなるので.
		{ THRE_BufId_Main_SynthesisForm,{ 3,   13,     16, DEF_THRBUF_WIDTH_NORMAL, False }},	//! THRE_BufId_Main_SynthesisForm.
		{ THRE_BufId_Main_SynthesisIn,  { 2,   13,     16, DEF_THRBUF_WIDTH_NORMAL, False }},	//! THRE_BufId_Main_SynthesisIn  YMCK+Att2→RGB_Att4(10/04/05 soota 関数履歴を参照).
		//! SDRAM.
		{ THRE_BufId_ExtIn,             { 8,   13,     16, DEF_THRBUF_WIDTH_NORMAL, False }},	//! THRE_BufId_ExtIn.
		{ THRE_BufId_RotPrint,          {64,   14,     16, DEF_THRBUF_WIDTH_NORMAL, False }},	//! THRE_BufId_RotPrint.
		{ THRE_BufId_EngOut,            {64,   16,     16, DEF_THRBUF_WIDTH_NORMAL, False }},	//! THRE_BufId_EngOut.
		{ THRE_BufId_CombineEng,        {64,    3,     16, DEF_THRBUF_WIDTH_NORMAL, False }},	//! THRE_BufId_CombineEng.
		{ THRE_BufId_Fax0Tx,            { 4,    1,     16, DEF_THRBUF_WIDTH_NORMAL, False }},	//! THRE_BufId_Fax0Tx.
																												//! THRE_BufId_Fax1Tx.
																												//! THRE_BufId_Fax2Tx.
																												//! THRE_BufId_Fax0Rx.
																												//! THRE_BufId_Fax1Rx.
																												//! THRE_BufId_Fax2Rx.
		{ THRE_BufId_AppIn,             { 8,   16,     16, DEF_THRBUF_WIDTH_NORMAL, False }},	//! THRE_BufId_AppIn.
		{ THRE_BufId_ExtIfIn,           { 8,   14,     16, DEF_THRBUF_WIDTH_NORMAL, False }},	//! THRE_BufId_ExtIfIn.
		{ THRE_BufId_ConvCmp,           {64,   14,     16, DEF_THRBUF_WIDTH_NORMAL, False }},	//! THRE_BufId_ConvCmp.
		{ THRE_BufId_ConvExp,           {64,   16,     16, DEF_THRBUF_WIDTH_NORMAL, False }},	//! THRE_BufId_ConvExp.
																												//! THRE_BufId_ConvCmpImgLog.
																												//! THRE_BufId_ConvExpImgLog.
		{ THRE_BufId_ScnOut,            {64,   13,     16, DEF_THRBUF_WIDTH_NORMAL, False }},	//! THRE_BufId_ScnOut.
																												//! THRE_BufId_ScnOutImgLog.
		{ THRE_BufId_CombineApp,        {64,    5,     16, DEF_THRBUF_WIDTH_NORMAL, False }}	//! THRE_BufId_CombineApp.
#else			// Cosim時.
 #if defined(COSIM) && defined(LARGE)
		{ THRE_BufId_Main,              { 1,    1,    256, THRD_ThrBuf_Width_Normal_Other, False }},		//! THRE_BufId_Main.
		{ THRE_BufId_Main_Combine,      { 1,    1,      1, THRD_ThrBuf_Width_Normal_Other, False }},		//! THRE_BufId_Main_Combine.
		{ THRE_BufId_Main_CombineJimon, { 1,    1,      1, THRD_ThrBuf_Width_Normal_Other, False }},		//! THRE_BufId_Main_CombineJimon.
		{ THRE_BufId_Main_Rot,          { 1,    1,      1, THRD_ThrBuf_Width_Normal_Other, False }},		//! THRE_BufId_Main_Rot.
		{ THRE_BufId_Main_Preview,      { 1,    1,   1024, 1024							 , False }},		//! THRE_BufId_Main_Preview.
		//! PCI側なので、1色8プレーンとして持つ。(1色1プレーンとすると、属性部分とメモリ横幅が合わなくなるので.
		//! 2010.04.21 soota 多値合成 原稿用、及びForm用バッファのメモリ横幅をサブメモリ側のメモリ横幅と同じにする.
		{ THRE_BufId_Main_SynthesisForm,{ 1,   13,    256, THRD_ThrBuf_Width_Normal, False }},	//! THRE_BufId_Main_SynthesisForm.
		{ THRE_BufId_Main_SynthesisIn,  { 1,   13,    256, THRD_ThrBuf_Width_Normal, False }},	//! THRE_BufId_Main_SynthesisIn YMCK+Att2→RGB_Att4(10/04/05 soota 関数履歴を参照).
 		{ THRE_BufId_ExtIn,             { 6,   13,    256, THRD_ThrBuf_Width_Normal, False }},	//! THRE_BufId_ExtIn.
 #else
		//! THRE_BufId_Main.
		{ THRE_BufId_Main,
#if !defined(DEF_DenebMLKCus) // DenebMLK メモリ4G対応.
			{{112,    1,    256, THRD_ThrBuf_Width_Normal_Other },			//! Soc構成 標準 Index 0.
#else	//#if !defined(DEF_DenebMLKCus).
			{{93,    1,    256, THRD_ThrBuf_Width_Normal_Other },			//! Soc構成 標準 Index 0.
#endif	//#if !defined(DEF_DenebMLKCus).
			{ 93,    1,    256, THRD_ThrBuf_Width_Normal_Other },			//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 { 93,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//! A4機構成 MFP Index 2.
			 { 93,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//!          AIO Index 3.	// ソリューション.
			 { 43,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//!          SFP Index 4.	// ソリューション.
#else	// #if defined(DEF_SparrowCus).
			 { 93,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//! A4機構成 標準 Index 2.
			 { 43,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
		//! THRE_BufId_Main_Combine.
		{ THRE_BufId_Main_Combine,
			{{  1,    1,    256, THRD_ThrBuf_Width_Normal_Other },			//! Soc構成 標準 Index 0.
			 {  0,    1,    256, THRD_ThrBuf_Width_Normal_Other },			//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  1,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//! A4機構成 MFP Index 2.
			 {  1,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//!          AIO Index 3.
			 {  1,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  1,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//! A4機構成 標準 Index 2.
			 {  0,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
		//! THRE_BufId_Main_CombineJimon.
		{ THRE_BufId_Main_CombineJimon,
			{{  1,    1,    256, THRD_ThrBuf_Width_Normal_Other },			//! Soc構成 標準 Index 0.
			 {  0,    1,    256, THRD_ThrBuf_Width_Normal_Other },			//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  1,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//! A4機構成 MFP Index 2.
			 {  1,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//!          AIO Index 3.
			 {  1,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  1,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//! A4機構成 標準 Index 2.
			 {  0,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
		//! THRE_BufId_Main_Rot.
		{ THRE_BufId_Main_Rot,
			{{  1,    1,    256, THRD_ThrBuf_Width_Normal_Other },			//! Soc構成 標準 Index 0.
			 {  0,    1,    256, THRD_ThrBuf_Width_Normal_Other },			//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  1,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//! A4機構成 MFP Index 2.
			 {  1,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//!          AIO Index 3.
			 {  1,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  1,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//! A4機構成 標準 Index 2.
			 {  0,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },		//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
		//! THRE_BufId_Main_Preview.
		{ THRE_BufId_Main_Preview,
			{{  1,    1,   1024, 1024	},									//! Soc構成 標準 Index 0.
			 {  0,    1,   1024, 1024	},									//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  1,    1,   1024, 1024	},									//! A4機構成 MFP Index 2.
			 {  1,    1,   1024, 1024	},									//!          AIO Index 3.
			 {  0,    1,   1024, 1024	},									//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  1,    1,   1024, 1024	},									//! A4機構成 標準 Index 2.
			 {  0,    1,   1024, 1024	},									//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
		//OP/PP共通化.
		//! THRE_BufId_Main_SynthesisForm.
		//! PCI側なので、1色8プレーンとして持つ。(1色1プレーンとすると、属性部分とメモリ横幅が合わなくなるので.
		//! 2010.04.21 soota 多値合成 原稿用、及びForm用バッファのメモリ横幅をサブメモリ側のメモリ横幅と同じにする.
		{ THRE_BufId_Main_SynthesisForm,
			{{ 43,   BTC_PLANE_NUM1,    256, THRD_OutBuf_Width_Max },		//! Soc構成 標準 Index 0.
			 {  0,   BTC_PLANE_NUM1,    256, THRD_OutBuf_Width_Max },		//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 { 43,   BTC_PLANE_NUM1,    256, THRD_OutBuf_Width_Max },		//! A4機構成 MFP Index 2.
			 {  0,   BTC_PLANE_NUM1,    256, THRD_OutBuf_Width_Max },		//!          AIO Index 3.
			 {  0,   BTC_PLANE_NUM1,    256, THRD_OutBuf_Width_Max },		//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 { 43,   BTC_PLANE_NUM1,    256, THRD_OutBuf_Width_Max },		//! A4機構成 標準 Index 2.
			 {  0,   BTC_PLANE_NUM1,    256, THRD_OutBuf_Width_Max },		//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
#ifdef DEF_A800
		//! THRE_BufId_Main_AppIn.
		{ THRE_BufId_Main_AppIn,
			{{  8,    1,    256, THRD_ThrBuf_Width_Normal_Other },			//! Soc構成 標準 Index 0.
			 {  4,    1,    256, THRD_ThrBuf_Width_Normal_Other }}},		//!         最小 Index 1.
		//OP/PP共通化.
		//! THRE_BufId_Main_ConvCmp.
		{ THRE_BufId_Main_ConvCmp,
			{{  8,   BTC_PLANE_NUM2,    256, THRD_ThrBuf_Width_Normal },				//! Soc構成 標準 Index 0.
			 {  4,   BTC_PLANE_NUM2,    256, THRD_ThrBuf_Width_Normal }}},				//!         最小 Index 1.
		//! THRE_BufId_Main_SynthesisIn YMCK+Att2→RGB_Att4(10/04/05 soota 関数履歴を参照).
		{ THRE_BufId_Main_SynthesisIn,
			{{ 12,   BTC_PLANE_NUM2,    256, THRD_OutBuf_Width_Max },		//! Soc構成 標準 Index 0.
			 {  0,   BTC_PLANE_NUM2,    256, THRD_OutBuf_Width_Max }}},		//!         最小 Index 1.
#else	// #ifdef DEF_A800.
		//OP/PP共通化.
		//! THRE_BufId_Main_SynthesisIn YMCK+Att2→RGB_Att4(10/04/05 soota 関数履歴を参照).
		{ THRE_BufId_Main_SynthesisIn,
			{{  3,   BTC_PLANE_NUM1,    256, THRD_OutBuf_Width_Max },		//! Soc構成 標準 Index 0.
			 {  0,   BTC_PLANE_NUM1,    256, THRD_OutBuf_Width_Max },		//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  3,   BTC_PLANE_NUM1,    256, THRD_OutBuf_Width_Max },		//! A4機構成 MFP Index 2.
			 {  0,   BTC_PLANE_NUM1,    256, THRD_OutBuf_Width_Max },		//!          AIO Index 3.
			 {  0,   BTC_PLANE_NUM1,    256, THRD_OutBuf_Width_Max },		//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  3,   BTC_PLANE_NUM1,    256, THRD_OutBuf_Width_Max },		//! A4機構成 標準 Index 2.
			 {  0,   BTC_PLANE_NUM1,    256, THRD_OutBuf_Width_Max },		//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
#endif	//#ifdef DEF_A800.
// 2012/04/10 Gonsho DS基板JPEGコーダ対応 Start.
		//! THRE_BufId_Main_AndroidJpegCompIn.
		{ THRE_BufId_Main_AndroidJpegCompIn,
#ifdef THR_RESTYPE_WSVGA	// WSVGA(1024×600).
			{{  1,    1,    600, THRD_OutBuf_Width_AndroidJpegCompIn_Other },		//! Soc構成 標準 Index 0.
			 {  1,    1,    600, THRD_OutBuf_Width_AndroidJpegCompIn_Other },		//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  1,    1,    600, THRD_OutBuf_Width_AndroidJpegCompIn_Other },		//! A4機構成 MFP Index 2.
			 {  1,    1,    600, THRD_OutBuf_Width_AndroidJpegCompIn_Other },		//!          AIO Index 3.
			 {  1,    1,    600, THRD_OutBuf_Width_AndroidJpegCompIn_Other },		//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  1,    1,    600, THRD_OutBuf_Width_AndroidJpegCompIn_Other },		//! A4機構成 標準 Index 2.
			 {  1,    1,    600, THRD_OutBuf_Width_AndroidJpegCompIn_Other },		//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
#else // THR_RESTYPE_WSVGA
#ifdef DEF_A800
			//TT上のコピー
			{{  1,    1,    600, THRD_OutBuf_Width_AndroidJpegCompIn_Other },		//! Soc構成 標準 Index 0.
			 {  1,    1,    600, THRD_OutBuf_Width_AndroidJpegCompIn_Other },		//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  1,    1,    600, THRD_OutBuf_Width_AndroidJpegCompIn_Other },		//! A4機構成 MFP Index 2.
			 {  1,    1,    600, THRD_OutBuf_Width_AndroidJpegCompIn_Other },		//!          AIO Index 3.
			 {  1,    1,    600, THRD_OutBuf_Width_AndroidJpegCompIn_Other },		//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  1,    1,    600, THRD_OutBuf_Width_AndroidJpegCompIn_Other },		//! A4機構成 標準 Index 2.
			 {  1,    1,    600, THRD_OutBuf_Width_AndroidJpegCompIn_Other },		//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
#else
			{{  1,    1,    480, THRD_OutBuf_Width_AndroidJpegCompIn_Other },									//! Soc構成 標準 Index 0.
			 {  1,    1,    480, THRD_OutBuf_Width_AndroidJpegCompIn_Other },									//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  1,    1,    480, THRD_OutBuf_Width_AndroidJpegCompIn_Other },		//! A4機構成 MFP Index 2.
			 {  1,    1,    480, THRD_OutBuf_Width_AndroidJpegCompIn_Other },		//!          AIO Index 3.
			 {  1,    1,    480, THRD_OutBuf_Width_AndroidJpegCompIn_Other },		//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  1,    1,    480, THRD_OutBuf_Width_AndroidJpegCompIn_Other },		//! A4機構成 標準 Index 2.
			 {  1,    1,    480, THRD_OutBuf_Width_AndroidJpegCompIn_Other },		//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
#endif
#endif // THR_RESTYPE_WSVGA
		//! THRE_BufId_Main_AndroidJpegCompOut.
		{ THRE_BufId_Main_AndroidJpegCompOut,
			{{  1,    1,      1, THRD_OutBuf_Width_AndroidJpegCompOut_Other },		//! Soc構成 標準 Index 0.
			 {  1,    1,      1, THRD_OutBuf_Width_AndroidJpegCompOut_Other },		//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  1,    1,      1, THRD_OutBuf_Width_AndroidJpegCompOut_Other },		//! A4機構成 MFP Index 2.
			 {  1,    1,      1, THRD_OutBuf_Width_AndroidJpegCompOut_Other },		//!          AIO Index 3.
			 {  1,    1,      1, THRD_OutBuf_Width_AndroidJpegCompOut_Other },		//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  1,    1,      1, THRD_OutBuf_Width_AndroidJpegCompOut_Other },		//! A4機構成 標準 Index 2.
			 {  1,    1,      1, THRD_OutBuf_Width_AndroidJpegCompOut_Other },		//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
// 2012/04/10 Gonsho DS基板JPEGコーダ対応 End.
		//! SDRAM.
		//! THRE_BufId_ExtIn スキャナIF入力用圧縮バッファ.
		//OP/PP共通化.
		{ THRE_BufId_ExtIn,
			{{  8,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_Normal_Other },		//! Soc構成 標準 Index 0.
			 {  8,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_Normal_Other },		//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  8,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//! A4機構成 MFP Index 2.
			 {  8,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//!          AIO Index 3.
			 {  0,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  8,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//! A4機構成 標準 Index 2.
			 { 40,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
 #endif
   #if defined(COSIM)
		{ THRE_BufId_RotPrint,          { 6,   13,    256, THRD_ThrBuf_Width_Normal, False }},		//! THRE_BufId_RotPrint 13/03/13 Sky ZeusS対応 Plane数 14→13に変更.
		{ THRE_BufId_EngOut,            {12,   20,    256, THRD_ThrBuf_Width_Normal, False }},		//! THRE_BufId_EngOut 13/03/13 Sky ZeusS対応 Plane数 16→20に変更.
		{ THRE_BufId_CombineEng,        {56,    2,    256, THRD_ThrBuf_Width_Normal, False }},		//! THRE_BufId_CombineEng.
		{ THRE_BufId_Fax0Tx,            { 4,    1,    256, THRD_ThrBuf_Width_Normal, False }},		//! THRE_BufId_Fax0Tx.
																									//! THRE_BufId_Fax1Tx.
																									//! THRE_BufId_Fax2Tx.
																									//! THRE_BufId_Fax3Tx.
																									//! THRE_BufId_Fax4Tx.
																									//! THRE_BufId_Fax0Rx.
																									//! THRE_BufId_Fax1Rx.
																									//! THRE_BufId_Fax2Rx.
																									//! THRE_BufId_Fax3Rx.
																									//! THRE_BufId_Fax4Rx.
		{ THRE_BufId_AppIn,             { 4,   20,    256, THRD_ThrBuf_Width_Normal, False }},		//! THRE_BufId_AppIn 13/03/13 Sky ZeusS対応 Plane数 16→20に変更.
		{ THRE_BufId_ExtIfIn,           { 6,   16,    256, THRD_ThrBuf_Width_Normal, False }},		//! THRE_BufId_ExtIfIn 13/03/13 Sky ZeusS対応 Plane数 14→16に変更.
		{ THRE_BufId_ConvCmp,           { 4,   14,    256, THRD_ThrBuf_Width_Normal, False }},		//! THRE_BufId_ConvCmp.
		{ THRE_BufId_ConvExp,           { 4,   16,    256, THRD_ThrBuf_Width_Normal, False }},		//! THRE_BufId_ConvExp.
		{ THRE_BufId_ScnOut,            { 4,   13,    256, THRD_ThrBuf_Width_Normal, False }},		//! THRE_BufId_ScnOut.
		{ THRE_BufId_CombineApp,        {44,    5,    256, THRD_ThrBuf_Width_Normal, False }}		//! THRE_BufId_CombineApp.
   #else	// #if defined(COSIM).
		//OP/PP共通化.
		//! THRE_BufId_RotPrint 回転&拡大プリント用伸張バッファ.
		{ THRE_BufId_RotPrint,
			{{  8,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_Normal_Other },		//! Soc構成 標準 Index 0.
			 {  8,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_Normal_Other },		//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  8,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//! A4機構成 MFP Index 2.
			 {  8,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//!          AIO Index 3.
			 {  8,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  8,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//! A4機構成 標準 Index 2.
			 {  8,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
			
// OP/PP共通化.
#ifdef DEF_PP_Cus
		//! THRE_BufId_EngOut エンジン出力用バッファ.
		{ THRE_BufId_EngOut,
			{{ 100,   21,    256, THRD_ThrBuf_Width_Normal_Other },		//! Soc構成 標準 Index 0.
			 {  8,   21,    256, THRD_ThrBuf_Width_Normal_Other }}},	//!         最小 Index 1. ソリューションモデル1200dpiプリント対応.
		//! THRE_BufId_EngOut1 エンジン出力用バッファ.
		{ THRE_BufId_EngOut1,
			{{ 100,   21,    256, THRD_ThrBuf_Width_Normal_Other },		//! Soc構成 標準 Index 0.
			 {  8,   21,    256, THRD_ThrBuf_Width_Normal_Other }}},	//!         最小 Index 1. ソリューションモデル1200dpiプリント対応.
		//! THRE_BufId_EngOut2 エンジン出力用バッファ.
		{ THRE_BufId_EngOut2,
			{{ 100,   21,    256, THRD_ThrBuf_Width_Normal_Other },		//! Soc構成 標準 Index 0.
			 {  8,   21,    256, THRD_ThrBuf_Width_Normal_Other }}},	//!         最小 Index 1. ソリューションモデル1200dpiプリント対応.
		//! THRE_BufId_EngOut3 エンジン出力用バッファ.
		{ THRE_BufId_EngOut3,
			{{ 100,   21,    256, THRD_ThrBuf_Width_Normal_Other },		//! Soc構成 標準 Index 0.
			 {  8,   21,    256, THRD_ThrBuf_Width_Normal_Other }}},	//!         最小 Index 1. ソリューションモデル1200dpiプリント対応.
#else //DEF_PP_Cus.
		//! THRE_BufId_EngOut エンジン出力用バッファ.
		{ THRE_BufId_EngOut,
#if !defined(DEF_DenebMLKCus) // DenebMLK メモリ4G対応.
#ifdef DEF_A800
			{{ 28,   21,    256, THRD_ThrBuf_Width_Normal_Other },		//! Soc構成 標準 Index 0.
#else
			{{ 12,   21,    256, THRD_ThrBuf_Width_Normal_Other },		//! Soc構成 標準 Index 0.
#endif
#else	//#if !defined(DEF_DenebMLKCus)
			{{ 40,   13,    256, THRD_ThrBuf_Width_Normal_Other },		//! Soc構成 標準 Index 0.
#endif	//#if !defined(DEF_DenebMLKCus)
			 {  8,   21,    256, THRD_ThrBuf_Width_Normal_Other },		//!         最小 Index 1. ソリューションモデル1200dpiプリント対応.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 { 12,   21,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//! A4機構成 MFP Index 2.
			 { 20,   21,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//!          AIO Index 3.
			 { 12,   21,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 { 12,   21,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//! A4機構成 標準 Index 2.
			 {  8,   21,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//!          最小 Index 3. ソリューションモデル1200dpiプリント対応.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
#endif //DEF_PP_Cus.
			
		//! THRE_BufId_CombineEng エンジン画像合成用伸張バッファ.
		{ THRE_BufId_CombineEng,
			{{ 56,    2,    256, THRD_ThrBuf_Width_Normal_Other },		//! Soc構成 標準 Index 0.
			 { 56,    2,    256, THRD_ThrBuf_Width_Normal_Other },		//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 { 33,    2,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//! A4機構成 MFP Index 2.
			 { 33,    2,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//!          AIO Index 3.
			 { 33,    2,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 { 33,    2,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//! A4機構成 標準 Index 2.
			 { 33,    2,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
		//! THRE_BufId_Fax0Tx.
		//! THRE_BufId_Fax1Tx.
		//! THRE_BufId_Fax2Tx.
		//! THRE_BufId_Fax3Tx.
		//! THRE_BufId_Fax4Tx.
		//! THRE_BufId_Fax0Rx.
		//! THRE_BufId_Fax1Rx.
		//! THRE_BufId_Fax2Rx.
		//! THRE_BufId_Fax3Rx.
		//! THRE_BufId_Fax4Rx.
		{ THRE_BufId_Fax0Tx,
			{{  8,    1,    256, THRD_ThrBuf_Width_Normal_Other },		//! Soc構成 標準 Index 0.
			 {  8,    1,    256, THRD_ThrBuf_Width_Normal_Other },		//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  8,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//! A4機構成 MFP Index 2.
			 {  8,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//!          AIO Index 3.
			 {  0,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  8,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//! A4機構成 標準 Index 2.
			 {  8,    1,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
		//! THRE_BufId_AppIn Emperor入力用圧縮バッファ.
		{ THRE_BufId_AppIn,
#if !defined(DEF_DenebMLKCus) // DenebMLK メモリ4G対応.
			{{  8,   21,    256, THRD_ThrBuf_Width_Normal_Other },		//! Soc構成 標準 Index 0.
#else	//#if !defined(DEF_DenebMLKCus)
			{{  4,   14,    256, THRD_ThrBuf_Width_Normal_Other },		//! Soc構成 標準 Index 0.
#endif	//#if !defined(DEF_DenebMLKCus)
			 {  4,   21,    256, THRD_ThrBuf_Width_Normal_Other },		//!         最小 Index 1.//ソリューションモデル1200dpiプリント対応.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  8,   21,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//! A4機構成 MFP Index 2.
			 {  8,   21,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//!          AIO Index 3.
			 {  8,   21,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  8,   21,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//! A4機構成 標準 Index 2.
			 {  4,   21,    256, THRD_ThrBuf_Width_A4_Normal_Other },	//!          最小 Index 3.//ソリューションモデル1200dpiプリント対応.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
		//! THRE_BufId_ExtIfIn.
		//! Fiery入力はテーブル選択後にs_SubBufParaExtInDupの情報を参照し、バッファ情報を決定する.
		{ THRE_BufId_ExtIfIn,
			{{ 40,   20,    256, THRD_ThrBuf_Width_Normal},			//! Soc構成 標準 Index 0.
			 {  8,   20,    256, THRD_ThrBuf_Width_Normal},			//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  0,   20,    256, THRD_ThrBuf_Width_A4_Normal },		//! A4機構成 MFP Index 2.
			 {  0,   20,    256, THRD_ThrBuf_Width_A4_Normal },		//!          AIO Index 3.
			 {  0,   20,    256, THRD_ThrBuf_Width_A4_Normal },		//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  0,   20,    256, THRD_ThrBuf_Width_A4_Normal },		//! A4機構成 標準 Index 2.
			 {  0,   20,    256, THRD_ThrBuf_Width_A4_Normal },		//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
		//OP/PP共通化.
		//! THRE_BufId_ConvCmp 画像変換後圧縮前バッファ.
		{ THRE_BufId_ConvCmp,
#if !defined(DEF_DenebMLKCus) // DenebMLK メモリ4G対応.
			{{  8,   BTC_PLANE_NUM2,    256, THRD_ThrBuf_Width_Normal },		//! Soc構成 標準 Index 0.
#else	//#if !defined(DEF_DenebMLKCus)
			{{  4,   BTC_PLANE_NUM2,    256, THRD_ThrBuf_Width_Normal },		//! Soc構成 標準 Index 0.
#endif	//#if !defined(DEF_DenebMLKCus)
			 {  4,   BTC_PLANE_NUM2,    256, THRD_ThrBuf_Width_Normal },		//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  8,   BTC_PLANE_NUM2,    256, THRD_ThrBuf_Width_Normal },		//! A4機構成 MFP Index 2.
			 {  4,   BTC_PLANE_NUM2,    256, THRD_ThrBuf_Width_Normal },		//!          AIO Index 3.	// ソリューション.
			 {  4,   BTC_PLANE_NUM2,    256, THRD_ThrBuf_Width_Normal },		//!          SFP Index 4.	// ソリューション.
#else	// #if defined(DEF_SparrowCus).
			 {  8,   BTC_PLANE_NUM2,    256, THRD_ThrBuf_Width_Normal },		//! A4機構成 標準 Index 2.
			 {  4,   BTC_PLANE_NUM2,    256, THRD_ThrBuf_Width_Normal },		//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
		//! THRE_BufId_ConvExp 画像変換前用伸張バッファ.
		{ THRE_BufId_ConvExp,
#if !defined(DEF_DenebMLKCus) // DenebMLK メモリ4G対応.
			{{  8,   16,                256, THRD_ThrBuf_Width_Normal },		//! Soc構成 標準 Index 0.
#else	//#if !defined(DEF_DenebMLKCus)
			{{  8,   14,                256, THRD_ThrBuf_Width_Normal },		//! Soc構成 標準 Index 0.
#endif	//#if !defined(DEF_DenebMLKCus)
			 {  8,   BTC_PLANE_NUM2,    256, THRD_ThrBuf_Width_Normal },		//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  8,               16,    256, THRD_ThrBuf_Width_Normal },		//! A4機構成 MFP Index 2.
			 {  8,   BTC_PLANE_NUM2,    256, THRD_ThrBuf_Width_Normal },		//!          AIO Index 3.
			 {  8,   BTC_PLANE_NUM2,    256, THRD_ThrBuf_Width_Normal },		//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  8,               16,    256, THRD_ThrBuf_Width_Normal },		//! A4機構成 標準 Index 2.
			 {  8,   BTC_PLANE_NUM2,    256, THRD_ThrBuf_Width_Normal },		//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
		//! THRE_BufId_ScnOut スキャン出力用伸張バッファ.
		{ THRE_BufId_ScnOut,
			{{  8,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_Normal_Other },	//! Soc構成 標準 Index 0.
			 {  8,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_Normal_Other },	//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  8,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_Normal_Other },	//! A4機構成 MFP Index 2.
			 {  8,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_Normal_Other },	//!          AIO Index 3.
			 {  0,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_Normal_Other },	//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  8,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_Normal_Other },	//! A4機構成 標準 Index 2.
			 {  8,   BTC_PLANE_NUM1,    256, THRD_ThrBuf_Width_Normal_Other },	//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
		//! THRE_BufId_CombineApp アプリ出力画像合成伸張後バッファ.
		{ THRE_BufId_CombineApp,
			{{ 44,    5,    256, THRD_ThrBuf_Width_Normal_Other },	//! Soc構成 標準 Index 0.
			 {  0,    5,    256, THRD_ThrBuf_Width_Normal_Other },	//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 { 33,    5,    256, THRD_ThrBuf_Width_A4_Normal_Other },//! A4機構成 MFP Index 2.
			 { 33,    5,    256, THRD_ThrBuf_Width_A4_Normal_Other },//!          AIO Index 3.
			 {  0,    5,    256, THRD_ThrBuf_Width_A4_Normal_Other },//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 { 33,    5,    256, THRD_ThrBuf_Width_A4_Normal_Other },//! A4機構成 標準 Index 2.
			 {  0,    5,    256, THRD_ThrBuf_Width_A4_Normal_Other },//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
		//! THRE_BufId_ScnInRaw スキャナ入力RAWデータバッファ.
		{ THRE_BufId_ScnInRaw,
			{{ 41,   24,    256, THRD_ThrBuf_Width_Normal_Other },	//! Soc構成 標準 Index 0.
			 { 41,   24,    256, THRD_ThrBuf_Width_Normal_Other },	//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 { 34,   24,    256, THRD_ThrBuf_Width_A4_Max_Skew },	//! A4機構成 MFP Index 2.
			 { 34,   24,    256, THRD_ThrBuf_Width_A4_Max_Skew },	//!          AIO Index 3.
			 {  0,   24,    256, THRD_ThrBuf_Width_A4_Max_Skew },	//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 { 34,   24,    256, THRD_ThrBuf_Width_A4_Max_Skew },	//! A4機構成 標準 Index 2.
			 { 34,   24,    256, THRD_ThrBuf_Width_A4_Max_Skew },	//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
		//! THRE_BufId_ScnInRaw_Dup スキャナ入力RAWデータバッファ(裏面).
		{ THRE_BufId_ScnInRaw_Dup,
			{{ 41,   24,    256, THRD_ThrBuf_Width_Normal_Other },	//! Soc構成 標準 Index 0.
			 {  0,   24,    256, THRD_ThrBuf_Width_Normal_Other },	//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 { 34,   24,    256, THRD_ThrBuf_Width_A4_Max_Skew },	//! A4機構成 MFP Index 2.
			 { 34,   24,    256, THRD_ThrBuf_Width_A4_Max_Skew },	//!          AIO Index 3.
			 {  0,   24,    256, THRD_ThrBuf_Width_A4_Max_Skew },	//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 { 34,   24,    256, THRD_ThrBuf_Width_A4_Max_Skew },	//! A4機構成 標準 Index 2.
			 {  0,   24,    256, THRD_ThrBuf_Width_A4_Max_Skew },	//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
		//! THRE_BufId_SkewCorr スキュー補正データバッファ.
		{ THRE_BufId_SkewCorr,
			{{  1,   24,    256, THRD_ThrBuf_Width_Normal_Other },	//! Soc構成 標準 Index 0.
			 {  1,   24,    256, THRD_ThrBuf_Width_Normal_Other },	//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  1,   24,    256, THRD_ThrBuf_Width_A4_Normal_Other },//! A4機構成 MFP Index 2.
			 {  1,   24,    256, THRD_ThrBuf_Width_A4_Normal_Other },//!          AIO Index 3.
			 {  0,   24,    256, THRD_ThrBuf_Width_A4_Normal_Other },//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  1,   24,    256, THRD_ThrBuf_Width_A4_Normal_Other },//! A4機構成 標準 Index 2.
			 {  1,   24,    256, THRD_ThrBuf_Width_A4_Normal_Other },//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
		//! THRE_BufId_RGBDelay RGBデータ遅延バッファ.
		{ THRE_BufId_RGBDelay,
			{{  1,   24,     40, THRD_ThrBuf_Width_Normal_Other },	//! Soc構成 標準 Index 0.
			 {  1,   24,     40, THRD_ThrBuf_Width_Normal_Other },	//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  1,   24,     40, THRD_ThrBuf_Width_A4_Normal_Other },//! A4機構成 MFP Index 2.
			 {  1,   24,     40, THRD_ThrBuf_Width_A4_Normal_Other },//!          AIO Index 3.
			 {  1,   24,     40, THRD_ThrBuf_Width_A4_Normal_Other },//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  1,   24,     40, THRD_ThrBuf_Width_A4_Normal_Other },//! A4機構成 標準 Index 2.
			 {  1,   24,     40, THRD_ThrBuf_Width_A4_Normal_Other },//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
#ifdef DEF_A800
		//! THRE_BufId_Rotate 回転前バッファ.
		{ THRE_BufId_Rotate,
			{{ 15,    2,   1024, THRD_ThrBuf_Width_Rotate_Other },	//! Soc構成 標準 Index 0.
			 { 12,    2,   1024, THRD_ThrBuf_Width_Rotate_Other }}},//!         最小 Index 1.
#else	// #ifdef DEF_A800.
		//! THRE_BufId_Rotate 回転前バッファ.
		{ THRE_BufId_Rotate,
			{{ 12,    2,   1024, THRD_ThrBuf_Width_Rotate_Other },	//! Soc構成 標準 Index 0.
			 { 12,    2,   1024, THRD_ThrBuf_Width_Rotate_Other },	//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 { 12,    2,   1024, THRD_ThrBuf_Width_Rotate_Other },	//! A4機構成 MFP Index 2.
			 { 12,    2,   1024, THRD_ThrBuf_Width_Rotate_Other },	//!          AIO Index 3.
			 { 12,    2,   1024, THRD_ThrBuf_Width_Rotate_Other },	//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 { 12,    2,   1024, THRD_ThrBuf_Width_Rotate_Other },	//! A4機構成 標準 Index 2.
			 { 12,    2,   1024, THRD_ThrBuf_Width_Rotate_Other },	//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
#endif	//#ifdef DEF_A800.
			
#ifdef DEF_PP_Cus
		// OP/PP共通化 工程スキャン.
		//! THRE_BufId_RawImageScan 工程スキャン用出力バッファ
		{ THRE_BufId_RawImageScan,
			{{ 41,   24,    256, THRD_ThrBuf_Width_Normal_Other },	//! Soc構成 標準 Index 0.
			 { 41,   24,    256, THRD_ThrBuf_Width_Normal_Other }}},//!         最小 Index 1.
		//! THRE_BufId_RawImageScan_Dup 工程スキャン用出力バッファ(裏面)
		{ THRE_BufId_RawImageScan_Dup,
			{{ 41,   24,    256, THRD_ThrBuf_Width_Normal_Other },	//! Soc構成 標準 Index 0.
			 { 41,   24,    256, THRD_ThrBuf_Width_Normal_Other }}},//!         最小 Index 1.
#endif	// DEF_PP_Cus.

		{ THRE_BufId_DummyExtIn,
			{{  2,   13,    256, THRD_ThrBuf_Width_Normal },        //! Soc構成 標準 Index 0.
			 {  2,   13,    256, THRD_ThrBuf_Width_Normal },        //!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  2,   13,    256, THRD_ThrBuf_Width_A4_Normal },		//! A4機構成 MFP Index 2.
			 {  2,   13,    256, THRD_ThrBuf_Width_A4_Normal },		//!          AIO Index 3.
			 {  2,   13,    256, THRD_ThrBuf_Width_A4_Normal },		//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  2,   13,    256, THRD_ThrBuf_Width_A4_Normal },		//! A4機構成 標準 Index 2.
			 {  2,   13,    256, THRD_ThrBuf_Width_A4_Normal },		//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}},
		{ THRE_BufId_DummyExtInDup,
			{{  2,   13,    256, THRD_ThrBuf_Width_Normal},         //! Soc構成 標準 Index 0.
			 {  2,   13,    256, THRD_ThrBuf_Width_Normal},         //!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  2,   13,    256, THRD_ThrBuf_Width_A4_Normal },		//! A4機構成 MFP Index 2.
			 {  2,   13,    256, THRD_ThrBuf_Width_A4_Normal },		//!          AIO Index 3.
			 {  2,   13,    256, THRD_ThrBuf_Width_A4_Normal },		//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  2,   13,    256, THRD_ThrBuf_Width_A4_Normal },		//! A4機構成 標準 Index 2.
			 {  2,   13,    256, THRD_ThrBuf_Width_A4_Normal },		//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}}
   #endif	// #if defined(COSIM).
#endif 			// Cosim時.
	};

	static THRS_SubBufPara s_ParamCode512 = {
		THRE_BufId_ScnInRaw,
			{{ 100,   16,    256, THRD_ThrBuf_Width_Normal_Other },	//! Soc構成 標準 Index 0.
			 { 100,   16,    256, THRD_ThrBuf_Width_Normal_Other },	//!         最小 Index 1.
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
#if defined(DEF_SparrowCus)
			 {  100,   16,    256, THRD_ThrBuf_Width_Normal_Other },	//! A4機構成 MFP Index 2.
			 {  0,   16,    256, THRD_ThrBuf_Width_Normal_Other },	//!          AIO Index 3.
			 {  0,   16,    256, THRD_ThrBuf_Width_Normal_Other },	//!          SFP Index 4.
#else	// #if defined(DEF_SparrowCus).
			 {  0,   16,    256, THRD_ThrBuf_Width_Normal_Other },	//! A4機構成 標準 Index 2.
			 {  0,   16,    256, THRD_ThrBuf_Width_Normal_Other },	//!          最小 Index 3.
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}};

	uchar uc_Idx = 0;
	uchar uc_IndexMax = sizeof(s_ParamCode)/sizeof(THRS_SubBufPara);

	//! 共通サイズのバッファIDに変換.
	THRE_BufId e_ChgBufId = e_BufId;
	switch ( e_BufId ){
		case THRE_BufId_Fax1Tx:									//! Fax送信(回線1).
		case THRE_BufId_Fax2Tx:									//! Fax送信(回線2).
		case THRE_BufId_Fax3Tx:									//! Fax送信(回線3).
		case THRE_BufId_Fax4Tx:									//! Fax送信(回線4).
		case THRE_BufId_Fax0Rx:									//! Fax受信(iFax).
		case THRE_BufId_Fax1Rx:									//! Fax受信(回線1).
		case THRE_BufId_Fax2Rx:									//! Fax受信(回線2).
		case THRE_BufId_Fax3Rx:									//! Fax受信(回線3).
		case THRE_BufId_Fax4Rx:									//! Fax受信(回線4).
			e_ChgBufId = THRE_BufId_Fax0Tx;
			break;
		default:
			break;
	}

	if( 0 < btc48_buffdump_enable ){
		printf("**** THRC_BufMemory::GetBufMemoryInfo() s_ParamCode dump start ****\n");
		for( int i=0; i<uc_IndexMax; i++ ){
			printf("  -- BufferId = %2d\n",s_ParamCode[i].e_SearchBufId);
			printf("     --[0]  %4d %4d %4d %4d \n",s_ParamCode[i].s_Param[0].us_BandMax,s_ParamCode[i].s_Param[0].us_PlaneMax,s_ParamCode[i].s_Param[0].ul_BandLine,s_ParamCode[i].s_Param[0].ul_BufWidth);
			printf("     --[1]  %4d %4d %4d %4d \n",s_ParamCode[i].s_Param[1].us_BandMax,s_ParamCode[i].s_Param[1].us_PlaneMax,s_ParamCode[i].s_Param[1].ul_BandLine,s_ParamCode[i].s_Param[1].ul_BufWidth);
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
			printf("     --[2]  %4d %4d %4d %4d \n",s_ParamCode[i].s_Param[2].us_BandMax,s_ParamCode[i].s_Param[2].us_PlaneMax,s_ParamCode[i].s_Param[2].ul_BandLine,s_ParamCode[i].s_Param[2].ul_BufWidth);
			printf("     --[3]  %4d %4d %4d %4d \n",s_ParamCode[i].s_Param[3].us_BandMax,s_ParamCode[i].s_Param[3].us_PlaneMax,s_ParamCode[i].s_Param[3].ul_BandLine,s_ParamCode[i].s_Param[3].ul_BufWidth);
#if defined(DEF_SparrowCus)
			printf("     --[4]  %4d %4d %4d %4d \n",s_ParamCode[i].s_Param[4].us_BandMax,s_ParamCode[i].s_Param[4].us_PlaneMax,s_ParamCode[i].s_Param[4].ul_BandLine,s_ParamCode[i].s_Param[4].ul_BufWidth);
#endif	// #if defined(DEF_SparrowCus).
#endif	// #ifdef DEF_A3A4COMM.
		}
		printf("**** THRC_BufMemory::GetBufMemoryInfo() s_ParamCode dump end ****\n");
	}			
			
// 2013/06/25 ADS森本	CQ.381621(Act_66306) SP-SAP-19	サブメモリ可変化 サブメモリ構成設定(No.1)対応.
#if !defined(COSIM)
#ifdef DEF_A3A4COMM		// IT6メモリパーティション A3A4COMM対応.
	//! ソリューションモデル切替判定処理.
	uchar uc_SubIdx = THRE_SubParam_Std;	// 標準構成のIndex指定(A3機).
#if defined(DEF_SparrowCus)
	if( TYPE_CapacityA3 == APIC_OptionSetting::newInstance()->getCapacity() ){ // 機種 A3A4判定.
		//! ソリューションモデル切替判定処理.
		if( True == SYSF_getAppOptionStatus(TYPE_AppOptionSubMemSplitSave) ){ // 機能判定.
			// 機能判定結果が有効の場合.
			// 最小構成のIndex指定.
			uc_SubIdx = THRE_SubParam_Min;
		}
	} else {
		switch( THRF_GetMachineTypeInfo() ) {
		case TYPE_MachineTypeSparrow1MFP:		// SparrowMFP1.
		case TYPE_MachineTypeSparrow2MFP:		// SparrowMFP2.
		case TYPE_MachineTypeSparrowBK0MFP:		// SparrowBK 0MFP.
		case TYPE_MachineTypeSparrowBK1MFP:		// SparrowBK 1MFP.
		case TYPE_MachineTypeSparrowBKEmu800:	// SparrowBK Emu800.
		case TYPE_MachineTypeSparrowBKMFPEmu1:	// SparrowBK 1MFP Emu.
		case TYPE_MachineTypeSparrowBKMFPEmu2:	// SparrowBK 2MFP Emu.
			/* 2018.08.22 es_ishida AR1046366 PC Print1200dpi + 登録オーバーレイ異常 */
			// Eagleのテーブルを使用するよう変更.
			uc_SubIdx = THRE_SubParam_Std;		// A4機構成 MFP Index 2.
			break;
		case TYPE_MachineTypeSparrow2AIO:		// SparrowAIO2.
			uc_SubIdx = THRE_SubParam_A4_AIO;	// A4機構成 AIO Index 3.
			break;
		case TYPE_MachineTypeSparrow1SFP:		// SparrowSFP1.
		case TYPE_MachineTypeSparrow2SFP:		// SparrowSFP2.
		case TYPE_MachineTypeSparrowBK0SFP:		// SparrowBK 0SFP.
		case TYPE_MachineTypeSparrowBKSFPEmu1:	// SparrowBK 1SFP.
		case TYPE_MachineTypeSparrowBKSFPEmu2:	// SparrowBK 2SFP.
			uc_SubIdx = THRE_SubParam_A4_SFP;	// A4機構成 SFP Index 4.
			break;
		}
	}
#else	// #if defined(DEF_SparrowCus).
	// 機能オプション対応(A4機モデル追加)を判定とする.
	if( ( True == SYSF_getAppOptionStatus(TYPE_AppOptionSubMemSplitSave) ) ||	// サブメモリ可変化[1ページ確保動作]、スワップ分割保存・分割読出.
		( True == SYSF_getAppOptionStatus(TYPE_AppOptionSubMemPartition) )){	// サブメモリ割り当て切り替え対応.
		// 機能判定結果が有効の場合.
		if( TYPE_CapacityA3 == APIC_OptionSetting::newInstance()->getCapacity() ){ // 機種 A3A4判定.
			uc_SubIdx = THRE_SubParam_Min;// 最小構成のIndex指定(A3機).
		}
		else{	// A4機.
#ifndef DEF_S800Emu
			uc_SubIdx = THRE_SubParam_A4_Min;// 最小構成のIndex指定(A4機).
#else
#ifdef DEF_SparrowAIOSFPCus
			uc_SubIdx = THRE_SubParam_A4_Min;// 最小構成のIndex指定(A4機).
#else
/* 暫定対応(メモリマップをA3:8G/HDDありに変更) : K.Shiranami */
			uc_SubIdx = THRE_SubParam_Min;// 最小構成のIndex指定(A3機).
#endif
#endif
		}
	}
	else{
		if( TYPE_CapacityA4 == APIC_OptionSetting::newInstance()->getCapacity() ){ // 機種 A3A4判定.
			// A4機.
#ifndef DEF_S800Emu
			uc_SubIdx = THRE_SubParam_A4_Std;// 標準構成のIndex指定(A4機).
#else
/* 暫定対応(メモリマップをA3:8G/HDDありに変更) : K.Shiranami */
#ifdef DEF_SparrowAIOSFPCus
			uc_SubIdx = THRE_SubParam_A4_Std;// 標準構成のIndex指定(A4機).
#else
			uc_SubIdx = THRE_SubParam_Std;	// 標準構成のIndex指定(A3機).
#endif
#endif
		}
	}
#endif	// #if defined(DEF_SparrowCus).
#else // #ifdef DEF_A3A4COMM.
	//! ソリューションモデル切替判定処理.
	uchar uc_SubIdx = THRE_SubParam_Std;// 標準構成のIndex指定.
	if( True == SYSF_getAppOptionStatus(TYPE_AppOptionSubMemSplitSave) ){ // 機能判定.
		// 機能判定結果が有効の場合.
		// 最小構成のIndex指定.
		uc_SubIdx = THRE_SubParam_Min;
	}
	
#endif // #ifdef DEF_A3A4COMM.
	THRL_ThAssert( uc_SubIdx < THRE_SubParam_Num, "GetBufMemoryInfo() Table exchange NG" , 0, 0, 0, 0, CFA14_DetailCode0x0B8C );
#endif	// !defined(COSIM).

	//! 補正IDによるテーブル位置検索.
	for( uc_Idx=0; uc_Idx<uc_IndexMax; uc_Idx++ ){
		if( s_ParamCode[uc_Idx].e_SearchBufId == e_ChgBufId ){
			break;
		}
	}
	THRL_ThAssert( uc_Idx < uc_IndexMax, "GetBufMemoryInfo() Table search NG" , 0, 0, 0, 0, CFA14_DetailCode0x0898 );

	THRS_SubBufParaCom s_Para;
#ifdef COSIM
	// 選択したバッファ情報を利用する.
	s_Para = s_ParamCode[uc_Idx].s_Param;
#else
	s_Para = s_ParamCode[uc_Idx].s_Param[uc_SubIdx];	// 2013/06/25 ADS森本	CQ.381621(Act_66306) SP-SAP-19	サブメモリ可変化 サブメモリ構成設定(No.1)対応.
#endif

	// 2019/2/28 SCD2 伊藤史. EagleL MR1 制御構成変更No.1 BOX画像(KM形式)インポート対応（Eagle世代越リストア).
	if( (e_BufId == THRE_BufId_ScnInRaw) && 
		(IMGE_BlockSizeConvertStatePopImage == IMGG_getBlockSizeConvertRestoreDataState()) ){
		s_Para = s_ParamCode512.s_Param[uc_SubIdx];
	}

	//! バッファ情報設定.
	s_Param.us_BandMax  = s_Para.us_BandMax;
	s_Param.us_PlaneMax = s_Para.us_PlaneMax;
	s_Param.ul_BandLine = s_Para.ul_BandLine;
	s_Param.ul_BufWidth = s_Para.ul_BufWidth;

	//! モードによるバッファ情報補正.
	if( e_BufMode == THRE_BufMode_A3Lef ){
		s_Param.ul_BufWidth = THRD_A3LefWidth;
		s_Param.us_BandMax = THRD_A3BufLine / DEF_BasicTransLine;
	} else if ( e_BufMode == THRE_BufMode_Long ){
		s_Param.us_BandMax = THRD_LongBufLine / DEF_BasicTransLine;
		s_Param.us_PlaneMax = THRD_LongPlaneNum;
	}
	THRL_ThAssert( (s_Param.ul_BufWidth * s_Param.us_BandMax * s_Param.us_PlaneMax) <= (s_Para.ul_BufWidth * s_Para.us_BandMax * s_Para.us_PlaneMax), "GetBufMemoryInfo() Size Over Mode=%d" , 0, 0, 0, 0, CFA14_DetailCode0x0899 );
}

// ================================================================================================ 
// THRC_DMAResource のログ実装関数.
// ================================================================================================ 
struct THRS_DMAResourceLog {
	CC_OneImgTransInfo* 			pc_OneImgTransInfo;
	THRS_ResourceEvacuation			c_ResEvacuation;
	uint32_t						ul_tick;
	uint32_t 						ul_JobID;		// pc_OneImgTransInfo->ul_JobID;
	Bool							b_Ans;
	THRE_DmaId						e_MyDmaID;
	EV_TaskID						e_TaskID;
	uchar							uc_OneThreadId;
	uchar							uc_type;
	uchar							uc_ThreadID;	// pc_OneImgTransInfo->en_ThreadID;
	uchar							uc_ThreadImgProcMode;	// pc_OneImgTransInfo->en_ThreadImgProcMode;
};
#define	DEF_DMArcLog_None		0
#define	DEF_DMArcLog_alloc		1
#define	DEF_DMArcLog_reserve	2
#define	DEF_DMArcLog_release	3
#define	DEF_DMArcLog_init		4
#define	DEF_DMArcLog_releaseReserve		5
#define	DEF_DMArcLog_setPushResouce		6
#define	DEF_DMArcLog_peekPushResouce	7
#define	DEF_DMArcLog_getPushResouce		8


#define	THRD_MaxDMAResourceLogNum	1024
static uint32_t ul_DMAResourceLogIdx = 0;
static Bool b_DMAResourceLogFlg = False;
static THRS_DMAResourceLog st_DMAResourceLog[THRD_MaxDMAResourceLogNum];

uint32_t	ul_MaxDMAResourceLogNum = THRD_MaxDMAResourceLogNum;
static THRS_DMAResourceLog* pst_DMAResourceLog = st_DMAResourceLog;

// ================================================================================================ 
// THRC_DMAResource のログ拡張.  debugF 33.
// ================================================================================================ 
void dbg_extendDMAResourceLog( uint32_t ul_LogNum )
{
	if( THRD_MaxDMAResourceLogNum != ul_MaxDMAResourceLogNum ){
		free((void *)pst_DMAResourceLog);
	}
	if( 1 >= ul_MaxDMAResourceLogNum ){
		ul_MaxDMAResourceLogNum = THRD_MaxDMAResourceLogNum;
		pst_DMAResourceLog = st_DMAResourceLog;
		ul_DMAResourceLogIdx = 0;
		b_DMAResourceLogFlg = False;
	} else {
		ul_MaxDMAResourceLogNum = ul_LogNum * 1024;
		pst_DMAResourceLog = (THRS_DMAResourceLog *)malloc( ul_MaxDMAResourceLogNum * sizeof(THRS_DMAResourceLog) );
		ul_DMAResourceLogIdx = 0;
		b_DMAResourceLogFlg = False;
	}
}

// ================================================================================================ 
// THRC_DMAResource のログ記録.
// ================================================================================================ 
void dbg_setDMAResourceLog( uchar uc_type,Bool b_Ans,
	THRE_DmaId 						e_MyDmaID,
	CC_OneImgTransInfo* 			pc_OneImgTransInfo,
	EV_TaskID						e_TaskID,
	uchar							uc_OneThreadId,
	THRS_ResourceEvacuation*		pc_ResEvacuation )
{
	uint32_t ul_Idx = ul_DMAResourceLogIdx;
	if( ++ul_DMAResourceLogIdx >= ul_MaxDMAResourceLogNum ){
		ul_DMAResourceLogIdx = 0;
		b_DMAResourceLogFlg = True;
	}
	pst_DMAResourceLog[ul_Idx].ul_tick = SYSF_tickGet();		// SYSF_対応.
	pst_DMAResourceLog[ul_Idx].uc_type = uc_type;
	pst_DMAResourceLog[ul_Idx].b_Ans = b_Ans;
	pst_DMAResourceLog[ul_Idx].e_MyDmaID = e_MyDmaID;

	pst_DMAResourceLog[ul_Idx].pc_OneImgTransInfo = pc_OneImgTransInfo;
	if( NULL != pc_OneImgTransInfo ){
		pst_DMAResourceLog[ul_Idx].ul_JobID = pc_OneImgTransInfo->ul_JobID;
		pst_DMAResourceLog[ul_Idx].uc_ThreadID = pc_OneImgTransInfo->en_ThreadID;
		pst_DMAResourceLog[ul_Idx].uc_ThreadImgProcMode = pc_OneImgTransInfo->en_ThreadImgProcMode;

	
	} else {
		pst_DMAResourceLog[ul_Idx].ul_JobID = 0;
		pst_DMAResourceLog[ul_Idx].uc_ThreadID = 0;
		pst_DMAResourceLog[ul_Idx].uc_ThreadImgProcMode = 0;
	}
	pst_DMAResourceLog[ul_Idx].e_TaskID = e_TaskID;
	pst_DMAResourceLog[ul_Idx].uc_OneThreadId = uc_OneThreadId;
	
	if( NULL == pc_ResEvacuation ){
		pst_DMAResourceLog[ul_Idx].c_ResEvacuation.pc_PushInfo = NULL;
		pst_DMAResourceLog[ul_Idx].c_ResEvacuation.e_sendTaskID = 0;
		pst_DMAResourceLog[ul_Idx].c_ResEvacuation.uc_OneThreadId = 0;
	} else {
		pst_DMAResourceLog[ul_Idx].c_ResEvacuation.pc_PushInfo = pc_ResEvacuation->pc_PushInfo;
		pst_DMAResourceLog[ul_Idx].c_ResEvacuation.e_sendTaskID = pc_ResEvacuation->e_sendTaskID;
		pst_DMAResourceLog[ul_Idx].c_ResEvacuation.uc_OneThreadId = pc_ResEvacuation->uc_OneThreadId;
	}
}

// ================================================================================================ 
// THRC_DMAResource のログ表示.
// ================================================================================================ 
void dsp_DMAResourceLog()
{
	uint32_t ul_LogNum = 0;
	uint32_t ul_Idx = 0;
	uint32_t ul_cnt;

	if( b_DMAResourceLogFlg == True ){
		ul_LogNum = ul_MaxDMAResourceLogNum;
		ul_Idx = ul_DMAResourceLogIdx;
	} else {
		ul_LogNum = ul_DMAResourceLogIdx;
		ul_Idx = 0;
	}

	printf("(    tick)[No]: DMA-ID,type(b_Ans),CC_OneImgTransInfo*(JobID,ThreadID,ImgProcMode),e_TaskID,uc_OneThreadId,c_ResEvacuation(CC_OneImgTransInfo*,e_sendTaskID,uc_OneThreadId)\n");
	for(ul_cnt = 0; ul_cnt < ul_LogNum; ul_cnt++ ){
		printf( "(%8"PRIu32")[%4"PRIu32"]: %02d,", pst_DMAResourceLog[ul_Idx].ul_tick, ul_Idx, pst_DMAResourceLog[ul_Idx].e_MyDmaID );
		switch( pst_DMAResourceLog[ul_Idx].uc_type ){
		case DEF_DMArcLog_alloc:
			printf("alloc          ");
			break;
		case DEF_DMArcLog_reserve:
			printf("reserve        ");
			break;
		case DEF_DMArcLog_release:
			printf("release        ");
			break;
		case DEF_DMArcLog_init:
			printf("init           ");
			break;
		case DEF_DMArcLog_releaseReserve:
			printf("releaseReserve ");
			break;
		case DEF_DMArcLog_setPushResouce:
			printf("setPushResouce ");
			break;
		case DEF_DMArcLog_peekPushResouce:
			printf("peekPushResouce");
			break;
		case DEF_DMArcLog_getPushResouce:
			printf("getPushResouce ");
			break;
		default:
			printf("unKnown(%2d)  ",pst_DMAResourceLog[ul_Idx].uc_type);
			break;
		}
		printf("(%d),",pst_DMAResourceLog[ul_Idx].b_Ans);
		if( NULL == pst_DMAResourceLog[ul_Idx].pc_OneImgTransInfo ){
			printf("   ----   (--,--,--),");
		} else {
			printf("0x%08"PRIxPTR"(%2"PRIu32",%2d,%2d),",(uintptr_t)pst_DMAResourceLog[ul_Idx].pc_OneImgTransInfo,	// 2014/09/19 日立 岡部(好) 静的解析対応.
				pst_DMAResourceLog[ul_Idx].ul_JobID,pst_DMAResourceLog[ul_Idx].uc_ThreadID,pst_DMAResourceLog[ul_Idx].uc_ThreadImgProcMode);
		}

		printf("%2d,%2d,",pst_DMAResourceLog[ul_Idx].e_TaskID,pst_DMAResourceLog[ul_Idx].uc_OneThreadId);

		if( NULL != pst_DMAResourceLog[ul_Idx].c_ResEvacuation.pc_PushInfo ){
			printf("(0x%"PRIxPTR"",(uintptr_t)pst_DMAResourceLog[ul_Idx].c_ResEvacuation.pc_PushInfo);
		} else {
			printf("(----");
		}
		printf(",%d,%d)\n",pst_DMAResourceLog[ul_Idx].c_ResEvacuation.e_sendTaskID,pst_DMAResourceLog[ul_Idx].c_ResEvacuation.uc_OneThreadId);
		ul_Idx++;
		if( ul_Idx >= ul_MaxDMAResourceLogNum ){
			ul_Idx = 0;
		}
	}
}

// ================================================================================================ 
//! <!--機能-->コンストラクタ. 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  pc_OneImgTransInfo リソースを確保する画像転送管理情報へのポインタ 
//! <!--戻値--> 
//! \return 確保できればTrue,できなければFalse 
//! <!--詳細--> 
//! リソースを確保する\n 
//! <!--履歴--> 
// 2014/06/01 伊藤史 新規作成. ログ追加. 
// ================================================================================================ 
THRC_DMAResource::THRC_DMAResource( THRE_DmaId e_DmaId )
{
	// THRC_ResourceCommonのコンストラクタではドライバと結び付けない。
	// THRC_BufferResourceとの兼ね合いからサブクラスがドライバと結びつける。
	ul_resComManId = THRL_Drvrif_Rc_ReserveResComManId();
	e_MyDmaID = e_DmaId;
	e_MyTaskID = TID_MAX;
	uc_MyOneThreadId = TID_MAX;		// None の定義がないので、TID_MAXで代用.
}

// ================================================================================================ 
//! <!--機能-->リソースの確保 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  pc_OneImgTransInfo リソースを確保する画像転送管理情報へのポインタ 
//! <!--戻値--> 
//! \return 確保できればTrue,できなければFalse 
//! <!--詳細--> 
//! リソースを確保する\n 
//! <!--履歴--> 
// 2014/06/01 伊藤史 新規作成. ログ追加. 
// ================================================================================================ 
Bool THRC_DMAResource::alloc( CC_OneImgTransInfo* pc_OneImgTransInfo, EV_TaskID e_TaskID, uchar uc_OneThreadId )
{
	Bool b_Ans = THRC_ResourceCommon::alloc( pc_OneImgTransInfo );
	if( True == b_Ans ){
		// 確保成功.
		e_MyTaskID = e_TaskID;
		uc_MyOneThreadId = uc_OneThreadId;
	}
	dbg_setDMAResourceLog( DEF_DMArcLog_alloc,b_Ans,e_MyDmaID,pc_OneImgTransInfo,e_TaskID,uc_OneThreadId,NULL);
	return (b_Ans);
}

// ================================================================================================ 
//! <!--機能-->リソースの予約 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  pc_OneImgTransInfo リソースを予約する画像転送管理情報へのポインタ 
//! <!--戻値--> 
//! \return 予約できればTrue,できなければFalse 
//! <!--詳細--> 
//! リソースを予約する\n 
//! <!--履歴--> 
// 2014/06/01 伊藤史 新規作成. ログ追加. 
// ================================================================================================ 
Bool THRC_DMAResource::reserve( CC_OneImgTransInfo* pc_OneImgTransInfo )
{
	Bool b_Ans = THRC_ResourceCommon::reserve( pc_OneImgTransInfo );
	dbg_setDMAResourceLog( DEF_DMArcLog_reserve,b_Ans,e_MyDmaID,pc_OneImgTransInfo,0,0,NULL);
	return (b_Ans);
}

// ================================================================================================ 
//! <!--機能-->リソースの解放 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  pc_OneImgTransInfo リソースを開放する画像転送管理情報へのポインタ 
//! <!--戻値--> 
//! \return 開放できればTrue,できなければFalse 
//! <!--詳細--> 
//! リソースを開放する\n 
//! <!--履歴--> 
// 2014/06/01 伊藤史 新規作成. ログ追加. 
// ================================================================================================ 
Bool THRC_DMAResource::release( CC_OneImgTransInfo* pc_OneImgTransInfo )
{
	Bool b_Ans = THRC_ResourceCommon::release( pc_OneImgTransInfo );
	dbg_setDMAResourceLog( DEF_DMArcLog_release,b_Ans,e_MyDmaID,pc_OneImgTransInfo,0,0,NULL);
	return (b_Ans);
}

// ================================================================================================ 
//! <!--機能-->リソースの初期化 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  なし 
//! <!--戻値--> 
//! \return なし 
//! <!--詳細--> 
//! リソースを初期化する\n 
//! <!--履歴--> 
// 2005/05/12 yokoi		新規作成 
// 2014/06/01 伊藤史 新規作成. ログ追加. 
// ================================================================================================ 
void THRC_DMAResource::init( void )
{
	dbg_setDMAResourceLog( DEF_DMArcLog_init,1,e_MyDmaID,NULL,0,0,NULL);
	THRC_ResourceCommon::init();
}

// ================================================================================================ 
//! <!--機能-->リソース予約の解放 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  pc_OneImgTransInfo リソース予約を開放する画像転送管理情報へのポインタ 
//! <!--戻値--> 
//! \return 開放できればTrue,できなければFalse 
//! <!--詳細--> 
//! リソースを開放する。基本的にキャンセル時のみ利用。\n 
//! <!--履歴--> 
// 2014/06/01 伊藤史 新規作成. ログ追加. 
// ================================================================================================ 
Bool THRC_DMAResource::releaseReserve( CC_OneImgTransInfo* pc_OneImgTransInfo )
{
	Bool b_Ans = THRC_ResourceCommon::releaseReserve( pc_OneImgTransInfo );
	dbg_setDMAResourceLog( DEF_DMArcLog_releaseReserve,b_Ans,e_MyDmaID,pc_OneImgTransInfo,0,0,NULL);
	return (b_Ans);
}

// ================================================================================================ 
//! <!--機能-->リソース退避情報設定. 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  CC_OneImgTransInfo*      画像転送管理情報(退避用). 
//! <!--戻値--> 
//! \return True：設定成功、False：設定失敗. 
//! <!--詳細--> 
//! リソースの退避情報を設定する.\n 
//! 1つのリソースに対して複数の退避は考慮しない(動いていないはず).\n 
//! <!--履歴--> 
// 2014/06/01 伊藤史 新規作成. ログ追加. 
// ================================================================================================ 
#ifdef DEF_Soc1200
Bool THRC_DMAResource::setPushResouce( THRS_ResourceEvacuation &c_ResEvaInfo )
{
	Bool b_Ans = THRC_ResourceCommon::setPushResouce( c_ResEvaInfo );
	dbg_setDMAResourceLog( DEF_DMArcLog_setPushResouce,b_Ans,e_MyDmaID,NULL,0,0,&c_ResEvaInfo);
	return (b_Ans);
}
#else
Bool THRC_DMAResource::setPushResouce( CC_OneImgTransInfo* pc_OneImgTransInfo )
{
	Bool b_Ans = THRC_ResourceCommon::setPushResouce( pc_OneImgTransInfo );
	return (b_Ans);
}
#endif

// ================================================================================================ 
//! <!--機能-->リソース退避情報取得. 
//! <!---------------------------------------------------------------------------------------------> 
//! <!--引数--> 
//! \param  なし 
//! <!--戻値--> 
//! \return CC_OneImgTransInfo*      画像転送管理情報. 
//! <!--詳細--> 
//! リソースの退避情報を取得する.\n 
//! 画像転送管理情報：退避情報がない場合はNULL.\n 
//! 退避情報設定後は退避情報をクリアする.\n 
//! <!--履歴--> 
// 2014/06/01 伊藤史 新規作成. ログ追加. 
// ================================================================================================ 
#ifdef DEF_Soc1200
Bool THRC_DMAResource::getPushResouce( THRS_ResourceEvacuation &c_ResEvaInfo  )
{
	Bool b_Ans = THRC_ResourceCommon::getPushResouce( c_ResEvaInfo );
	if( True == b_Ans ){
		e_MyTaskID = c_ResEvaInfo.e_sendTaskID;
		uc_MyOneThreadId = c_ResEvaInfo.uc_OneThreadId;
	}
	dbg_setDMAResourceLog( DEF_DMArcLog_getPushResouce,b_Ans,e_MyDmaID,NULL,0,0,&c_ResEvaInfo);
	return (b_Ans);
}
#else
CC_OneImgTransInfo* THRC_DMAResource::getPushResouce( void  )
{
	CC_OneImgTransInfo* pc_Ans = THRC_ResourceCommon::getPushResouce();
	return( pc_Ans );
}
#endif

#ifdef DEF_Soc1200
// ================================================================================================.
//! <!--機能-->リソースの退避情報の確認.
//! <!--------------------------------------------------------------------------------------------->. 
//! <!--引数-->.
//! \param  THRS_ResourceEvacuation &  リソース退避管理情報構造体.
//! <!--戻値-->.
//! \return True：退避情報有り、False：退避情報無し.
//! <!--詳細-->.
//! リソースの退避情報を確認する.
//! 退避情報がある場合はTrueを返却し、引数で指定された構造体に退避情報をコピーする. 
//! 退避情報がない場合はFalseを返却する. 
//! <!--履歴-->.
// 2014/06/01 伊藤史 新規作成. ログ追加. 
// ================================================================================================.
Bool THRC_DMAResource::peekPushResouce( THRS_ResourceEvacuation &c_ResEvaInfo  )
{
	Bool b_Ans = THRC_ResourceCommon::peekPushResouce( c_ResEvaInfo );
	dbg_setDMAResourceLog( DEF_DMArcLog_getPushResouce,b_Ans,e_MyDmaID,NULL,0,0,&c_ResEvaInfo);
	return (b_Ans);
}
#endif

