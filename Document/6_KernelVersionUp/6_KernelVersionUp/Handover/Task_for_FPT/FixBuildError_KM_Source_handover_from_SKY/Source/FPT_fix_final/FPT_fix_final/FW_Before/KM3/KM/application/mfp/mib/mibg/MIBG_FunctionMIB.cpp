/*=======================================================================================

	［タイトル］	MIB機能を処理するクラス 

	［  説明  ］	

=======================================================================================*/

/*=======================================================================================
	インクルードファイル 
=======================================================================================*/
// ..System

// ..Common
#include 	"system/include/typs.h"

// ..Local
#include	"MIBG_FunctionMIB.h"
#include	"MIBG_MIBInstance.h"
#include    "NETC_FixedValueDefine.h"

// 【IT5_4.1】【制御構成変更_No.1】HCL-Kaushik 2016/11/14 START.
// 【MinervaSSBK】【制御構成変更_No.10】HT-Kurihara 2021/01/29 START.
#if defined( DEF_MinervaSSBKCus )
#include	"mib/mibColor2/MIBC_VIDColor2.h"	// MinervaSSBK.
#elif ( defined DEF_MINERVAS_CUS || defined DEF_MINERVASBK_CUS || defined DEF_MINERVAMLK_CUS  )
// 【MinervaSSBK】【制御構成変更_No.10】HT-Kurihara 2021/01/29 END.
// 【IT5_4.1】【制御構成変更_No.1】HCL-Kaushik 2016/11/14 END.
#include	"mib/mibColor1/MIBC_VIDColor1.h"	// MinervaS BK, MinervaMLK.
// HeliosL 制御構成変更 No.1 sysObjectID対応 OPS1深谷 2014/11/07 START.
#elif ( defined DEF_Zeus5BkCus || defined DEF_HeliosCus || defined DEF_DenebMLKCus ) // 【IT6_DenebMLK】【制御構成変更_No.1】 2017/04/27 HCL-RamKisson Modified 
// #if defined( DEF_Zeus5BkCus ).
// HeliosL 制御構成変更 No.1 sysObjectID対応 OPS1深谷 2014/11/07 END.
#include	"mib/mibColor3/MIBC_VIDColor3.h"	// Zeus5BK HeLiosL.
// 2015/06/05 HCL A3A4共通 MIB Modification Start.
#elif defined( DEF_A4_Cus )
#include	"mib/mibA4Color1/MIBC_VIDA4Color1.h"	// VenusMLK
// 2015/06/05 HCL A3A4共通 MIB Modification End.

#else
#include	"mib/mibColor2/MIBC_VIDColor2.h"	// ZeusS, ZeusSBK, ZeusBK_2nd,Eagle_1/2/2.5, Eagle_Z/X/0.
#endif

// ユビキタス[KRQ-278][スプール分散恒久対応][step2] start.
#include	"system/nvd/nvdg/NVDC_Setting_UbiquitousSetting.h"
// ユビキタス[KRQ-278][スプール分散恒久対応][step2] end.

#include 	"system/sys/APIC_FatalSignalDetailCode.h" //IT5_4.2 新詳細コード追加対応 1043534 HCL_sundar 2018/4/6 ADDED.
 //2021/03/04 [IT6_2.2]  RQ-817  IT6_2.2-399 HIENG Start.
#include	"system/sys/APIC_VirusScan.h"
#include	"system/nvd/APIC_Setting_MachineSetting.h"
//2021/03/04 [IT6_2.2]  RQ-817  IT6_2.2-399 HIENG End.

/*=======================================================================================
	定数定義 
=======================================================================================*/
#define MIBD_VirusMsgBuf 127
#define MIBD_VirusMsgTimeBuf 31
#define MIBD_VirusMsgJobNumBuf 15

/*=======================================================================================
	マクロ関数定義 
=======================================================================================*/

/*=======================================================================================
	列挙型定義 
=======================================================================================*/

/*=======================================================================================
	構造体定義 
=======================================================================================*/

/*=======================================================================================
	外部参照変数定義 
=======================================================================================*/
#if defined( ROM_DIV_3 )
extern char *pt_mioSendSNMPTrap;
#endif
/*=======================================================================================
	外部定義関数
=======================================================================================*/

/*=======================================================================================
	モジュール内変数定義 
=======================================================================================*/
static t_sWSDPrinterJobs spWSDJobDataTblActive[MIBD_Max_ActiveJobListAll];
static short ssWSDActiveJobNumOld = 0;
static INT32 i32WSDJobActiveJobIDOld[MIBD_Max_ActiveJobListAll];
static short ssWSDActiveJobNumNew = 0;
static INT32 i32WSDJobActiveJobIDNew[MIBD_Max_ActiveJobListAll];
static t_sWSDPrinterJobs spWSDJobDataTblLog[MIBD_Max_JobLogListAll];
static t_sWSDPrinterStatusSummary spWSDPrinterStatusSummaryOld[1];
static t_sWSDPrinterStatusSummary spWSDPrinterStatusSummaryNew[1];
static short ssWSDTransferJobCnt = 0;
static MIBS_JobDataTblTransferStock s_TransferJobTbl[MIBD_Max_ActiveJobListAll];

// 【AR1037899】FQ-OPSD-1700411-OFP212 市場要望対応　HCL-Ramkisson 2018/01/30 Added Start.
INT32  i32p_nJobIdNetRxStatusSummary[MIBD_Max_RXJOBCount]; 
INT32  i32p_nJobIdActiveObserverStatusSummary[MIBD_Max_RXJOBCount];
Bool b_MibNicfumSentCheck = False;
// 【AR1037899】FQ-OPSD-1700411-OFP212 市場要望対応　HCL-Ramkisson 2018/01/30 Added End.
/*=======================================================================================
	モジュール内関数定義 
=======================================================================================*/
//IT5_4.2 新詳細コード追加対応 1043534 HCL_sundar 2018/4/6 START.
extern Bool b_NVDG_InitModule;
extern Bool  SetMibDetailCode(int Loc, uint64_t MIBCode);
extern unsigned int  ui_CrashDbgFlag;
//IT5_4.2 新詳細コード追加対応 1043534 HCL_sundar 2018/4/6 END.
/*=======================================================================================
// CLASS	: MIBC_FuncManMIB 
// ABSTRACT	: MIB機能を処理するクラス 
=======================================================================================*/



// 【AR1037899】FQ-OPSD-1700411-OFP212 市場要望対応　HCL-Ramkisson 2018/01/30 Added Start.

INT32 getMatchingJOBID()
{
	INT32 nMatchingJobID = MIBD_UndefNetRXJobID;
	
	for(int i = 0; i <  MIBD_Max_RXJOBCount;i++)
	{
		for(int j = 0; j < MIBD_Max_RXJOBCount;j++)
		{
			if(i32p_nJobIdNetRxStatusSummary[i] == i32p_nJobIdActiveObserverStatusSummary[j])
			{
				if(i32p_nJobIdNetRxStatusSummary[i] != MIBD_UndefNetRXJobID)
				{
					nMatchingJobID = i32p_nJobIdNetRxStatusSummary[i];
					i32p_nJobIdNetRxStatusSummary[i] = MIBD_UndefNetRXJobID;
					i32p_nJobIdActiveObserverStatusSummary[j] = MIBD_UndefNetRXJobID;
					return nMatchingJobID;
				}
			}	
		}
	}	
	return nMatchingJobID;
}
Bool NETG_FixedValueDefine_getJobIdNotifyFromNetRx(INT32 i32p_nJobId )
{
	NETG_FixedValueDefine* pc_FixedValueDefine = NETG_FixedValueDefine::newInstance();
	
    Bool bJobIdNetPrintRx = pc_FixedValueDefine->getJobIdNotifyFromNetRx( i32p_nJobId );

    
	return bJobIdNetPrintRx;
}
// 【AR1037899】FQ-OPSD-1700411-OFP212 市場要望対応　HCL-Ramkisson 2018/01/30 Added End.
/////////////////////////////////////////////////////////////////////////////////////////
// MIOからの要求処理 

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::vMIOReqDispatcher 
// ABSTRACT	: MIO受信メッセージのDispatch 
// FUNCTION	: MIOからの受信メッセージを処理する。MIOに応答メッセージを返す必要がある場合は 
//			  引数 _r_srMIOTraMsg に応答メッセージをセットし、IJCに送信する必要があるなら 
//			  引数_r_srIJCTraMsg 
// RETURN	: 
//			
// CREATE	: 01-05-22	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_FuncManMIB::vMIOReqDispatcher(
	// 2018/11/28 FPT-Huy AR1063739【IT6_1.0】【Eagle】 Modification Start.
	//MsgMioToApp&		_r_srRcvMsg,		// MIOからの受信メッセージ 
	MsgMioToApp		_r_srRcvMsg,		// MIOからの受信メッセージ 
	// 2018/11/28 FPT-Huy AR1063739【IT6_1.0】【Eagle】 Modification End.
	MsgAppToMio&		_r_srMIOTraMsg		// MIOへの送信メッセージ 
)
{
	switch( _r_srRcvMsg.MessageID )
	{
		case MIOREQ_WSDMetadata :			_vProcWSDMetadata( _r_srRcvMsg, _r_srMIOTraMsg );			break;	// 
		case MIOREQ_WSDPrinterActiveJobs :	_vProcWSDPrinterActiveJobs( _r_srRcvMsg, _r_srMIOTraMsg );	break;	// 
		case MIOREQ_WSDPrinterJobStatus :	_vProcWSDPrinterJobStatus( _r_srRcvMsg, _r_srMIOTraMsg );	break;	// 
		default: 
			if( _isReqMsg( _r_srRcvMsg.MessageID ) )
			{
				// MIOからのメッセージが要求メッセージ 
				_vProcReqestMsg( _r_srRcvMsg, _r_srMIOTraMsg );
			}
			else if( _isCmdMsg( _r_srRcvMsg.MessageID ) )
			{
				// コマンドメッセージ 
				_vProcCommandMsg( _r_srRcvMsg );
			}
			else
			{
				// ここにはこないはず 
				DWARNING( "// MIB (fncmib) : Undefined Msg ID From MIO" );
			}
			break;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// MIOからの要求処理 
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_vProcReqestMsg 
// ABSTRACT	: 要求メッセージの処理 
// FUNCTION	:  
// NOTE		: 
// RETURN	: 無し 
// CREATE	: 01-06-06	廣田 
//			  新規作成 
//			  02-05-20	河辺
//			  NM構造見直しのため変更
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_FuncManMIB::_vProcReqestMsg(
	MsgMioToApp&		_r_srRcvMsg,		// MIOからの受信メッセージ 
	MsgAppToMio&		_r_srTraMsg			// MIOへの送信メッセージ 
)
{
	DTRACE( ("*** MIB Request Message ***\r\n") );
	DTRACE( ("[Rcv] SequenceNo    = %d\r\n",_r_srRcvMsg.SequenceNo) );
	DTRACE( ("[Rcv] ApplicationID = %d\r\n",_r_srRcvMsg.ApplicationID) );
	DTRACE( ("[Rcv] MessageID     = %d\r\n",_r_srRcvMsg.MessageID) );

	// 受信データの取得 
	register MibReqData* spMibReqData = (MibReqData*)_r_srRcvMsg.pParameter;

/* 2008.10.23 No.310563 sw_saku chg S */
	INT32 i32_dcount;
	DTRACE( ("[Rcv] pParameter->index_value\r\n") );
	for(i32_dcount = 0 ; i32_dcount < MAX_INDEX ; i32_dcount++){
		DTRACE( ("%3d,",i32_dcount) );
	}	
	DTRACE( ("\r\n"));	
	for(i32_dcount = 0 ; i32_dcount < MAX_INDEX ; i32_dcount++){
		DTRACE( ("%3d,",spMibReqData->index_value[i32_dcount]) );
	}	
	DTRACE( ("\r\n"));
	DTRACE( ("[Rcv] pParameter->index_count    = %d\r\n",spMibReqData->index_count) );
/* 2008.10.23 No.310563 sw_saku chg E */

	// .. データ領域の確保 
	MibRspCmdData* spMibRspCmdData = NULL;
	for ( ;; ) {
		spMibRspCmdData = (MibRspCmdData*)malloc(sizeof(MibRspCmdData));
		if ( spMibRspCmdData NE NULL ) {
			break;
		}
		register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;	// ｍｓ→ティック数に変換(1000ms)
		if( sl_Tick == 0L ) { sl_Tick = 1L; }
		++sl_Tick;
		SYSF_taskDelay( sl_Tick );
	}

	UINT32 ui32MaxSize;
	UINT32 ui32GetSize = 0;
	MIB_DATA_TYPE eDataType;
	// 2018/10/31 FPT-ChiNM AR1043768【IT5_4.2】【Eagle】 Delete Start.
	//void* vpData = NULL;
	// 2018/10/31 FPT-ChiNM AR1043768【IT5_4.2】【Eagle】 Delete End.

	//初期化.
	memset(&spMibRspCmdData->next_index_value[0], -1, sizeof(spMibRspCmdData->next_index_value));
	
	spMibRspCmdData->next_index_value[0] = -1;
	spMibRspCmdData->next_index_value[1] = -1;

	Bool isDataExist = True;
	
//	【IT5_3.3】 【1002836】	HCL-Bairavi 2016/07/22 Modified Start.
// 【特ROM対応依頼(MBK-25)】　印刷済み部数のMIB追加対応 Poseidon Vaibhav 2014/07/17 START.
//#if defined( DEF_Zeus5BkCus )
	if ( ( (MIOREQ_kmJmJobJobSetIndex <= _r_srRcvMsg.MessageID) AND (_r_srRcvMsg.MessageID <= MIOREQ_kmJmJobPaperAttribute) ) ||
		 ( (MIOREQ_kmJmJobOutputDocCopyNumbers == _r_srRcvMsg.MessageID) ) )
	{
//#else
//	if ( (MIOREQ_kmJmJobJobSetIndex <= _r_srRcvMsg.MessageID) AND (_r_srRcvMsg.MessageID <= MIOREQ_kmJmJobPaperAttribute) ) {
//#endif
// 【特ROM対応依頼(MBK-25)】　印刷済み部数のMIB追加対応 Poseidon Vaibhav 2014/07/17 END.
//	【IT5_3.3】 【1002836】	HCL-Bairavi 2016/07/22 Modified End.
		// ジョブ情報 
		// 応答データの設定 
		// .. MIOMsgIDからVIDへの変換 
		const VID ceVID = _eGetJobVID( _r_srRcvMsg.MessageID );

	DTRACE( ("[---] VID = %d\r\n",ceVID) );

		// .. データの設定 
		eDataType = (MIB_DATA_TYPE)_m_irAllVIDMan.eGetDataType( ceVID );

		ui32MaxSize = _m_irAllVIDMan.ui32GetMaxSize( ceVID );
		// 2018/10/31 FPT-ChiNM AR1043768【IT5_4.2】【Eagle】 Modification Start.
		for ( ;; ) {
			// DB No.36080	「部門名を最大文字数入れると、SNMPタイムアウトを起こす」対策	2005.04.25	kawabe
			// mallocする領域を+1増やす 
			spMibRspCmdData->pData = malloc( ui32MaxSize + 1 );
			if (spMibRspCmdData->pData NE NULL ) {
				break;
			}
			register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;	// ｍｓ→ティック数に変換(1000ms)
			if( sl_Tick == 0L ) { sl_Tick = 1L; }
			++sl_Tick;
			SYSF_taskDelay( sl_Tick );
		}
		memset( spMibRspCmdData->pData, 0, ui32MaxSize + 1 );
		INT32 i32SerchData = _m_irAllVIDMan.vGetSerchJobData( spMibReqData->index_value[1] );
		if ( i32SerchData EQ -1 ) {
			isDataExist = False;
		} else {
			ui32GetSize = _m_irAllVIDMan.vGetJobData( i32SerchData, ceVID, spMibRspCmdData->pData );
			// 2018/10/31 FPT-ChiNM AR1043768【IT5_4.2】【Eagle】 Modification End.
		}

	} else if ( (MIOREQ_prtAlertSeverityLevel <= _r_srRcvMsg.MessageID) AND (_r_srRcvMsg.MessageID <= MIOREQ_prtAlertTime) ) {

		// prtAlertTable
		// .. MIOMsgIDからVIDへの変換 
		const VID ceVID = _eGetAlertVID( _r_srRcvMsg.MessageID );
		// .. データの設定 
		eDataType = (MIB_DATA_TYPE)_m_irAllVIDMan.eGetDataType( ceVID );
		_m_irAllVIDMan.eIJCSet( VIDD_prtAlertSetIndex, &spMibReqData->index_value[0], sizeof(INT32), MIBE_CharacterSet_USASCII );

		ui32MaxSize = _m_irAllVIDMan.ui32GetMaxSize( ceVID );
		// 2018/10/31 FPT-ChiNM AR1043768【IT5_4.2】【Eagle】 Modification Start.
		for ( ;; ) {
			// mallocする領域を+1増やす 	2005.04.25	kawabe
			spMibRspCmdData->pData = malloc( ui32MaxSize + 1 );
			if ( spMibRspCmdData->pData NE NULL ) {
				break;
			}
			register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;	// ｍｓ→ティック数に変換(1000ms)
			if( sl_Tick == 0L ) { sl_Tick = 1L; }
			++sl_Tick;
			SYSF_taskDelay( sl_Tick );
		}
		memset( spMibRspCmdData->pData, 0, ui32MaxSize + 1 );
		ui32GetSize = _m_irAllVIDMan.ui32Get( ceVID, spMibRspCmdData->pData );
		if ( ui32GetSize == MIBD_NoDataExist ) {
			// メモリリーク対策		2004.08.10	kawabe
			if (spMibRspCmdData->pData != NULL ) { free( spMibRspCmdData->pData ); }
			spMibRspCmdData->pData = NULL;
			ui32GetSize = 0;
		}
		// 2018/10/31 FPT-ChiNM AR1043768【IT5_4.2】【Eagle】 Modification End.
		
		int32_t ul_index = 0;
		_m_irAllVIDMan.ui32Get( VIDD_prtAlertIndex, &ul_index );
		spMibRspCmdData->index_value[0] = ul_index;
		spMibRspCmdData->index_value[1] = 0;
		spMibRspCmdData->index_count = spMibReqData->index_count;
		
		int32_t ul_nextIndex = 0;
		_m_irAllVIDMan.ui32Get( VIDD_prtAlertSetIndex, &ul_nextIndex );
		spMibRspCmdData->next_index_value[0] = ul_nextIndex;
		spMibRspCmdData->next_index_value[1] = -1;

	} else if ( (MIOREQ_kmScnAlertIndex <= _r_srRcvMsg.MessageID) AND (_r_srRcvMsg.MessageID <= MIOREQ_kmScnAlertTime) ) {
		
		if ( _r_srRcvMsg.MessageID == MIOREQ_kmScnAlertDescription || _r_srRcvMsg.MessageID == MIOREQ_kmScnAlertTime ) {
			eDataType = (MIB_DATA_TYPE)MIBE_APIDataType_OCTET_STR;
		} else {
			eDataType = (MIB_DATA_TYPE)MIBE_APIDataType_INT32;
		}
		ui32GetSize = 0;
		// 2018/10/31 FPT-ChiNM AR1043768【IT5_4.2】【Eagle】 Modification Start.
		spMibRspCmdData->pData = NULL;
		// 2018/10/31 FPT-ChiNM AR1043768【IT5_4.2】【Eagle】 Modification End.
		spMibRspCmdData->next_index_value[0] = -1;
		spMibRspCmdData->next_index_value[1] = -1;
	/* 2008.08.18 UV-00536 sw_miya add S */
	} else if ( (MIOREQ_jmGeneralNumberOfActiveJobs <= _r_srRcvMsg.MessageID) AND (_r_srRcvMsg.MessageID <= MIOREQ_jmAttributeValueAsOctets) ) {
		
		//jmGeneral Group,jmJobID Group,jmJob Group,jmAttribute Group.
		// .. MIOMsgIDからVIDへの変換 
		const VID ceVID = _eGetJobMonitoringVID( _r_srRcvMsg.MessageID );
		
		// データタイプ取得.
		eDataType = (MIB_DATA_TYPE)_m_irAllVIDMan.eGetDataType( ceVID );
		
		ui32MaxSize = _m_irAllVIDMan.ui32GetMaxSize( ceVID );
		// 2018/10/31 FPT-ChiNM AR1043768【IT5_4.2】【Eagle】 Modification Start.
		for ( ;; ) {
			spMibRspCmdData->pData = malloc( ui32MaxSize + 1 );
			if ( spMibRspCmdData->pData NE NULL ) {
				break;
			}
			register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;	// ｍｓ→ティック数に変換(1000ms)
			if( sl_Tick == 0L ) { sl_Tick = 1L; }
			++sl_Tick;
			SYSF_taskDelay( sl_Tick );
		}
		
		//初期化.
		memset(spMibRspCmdData->pData, 0, ui32MaxSize+1);
		// 2018/10/31 FPT-ChiNM AR1043768【IT5_4.2】【Eagle】 Modification End.
		/* 2008.10.23 No.309942 sw_saku chg S */
		//ジョブ数分応答が必要か判定.
		if( _isJudgmentJobMonitoring(ceVID ) == true){
			//ジョブインデックス取得.
			/* 2008.12.09 No.312803 sw_saku chg S */
			//2016/09/07 HCL-Sankar Static Analysis Fix 1002990 Start.
			INT32 i32JobID_temp = 0;
			if( spMibReqData->index_count < MAX_INDEX ) {
				i32JobID_temp = _getSerchIndexJobMonitoring(ceVID,spMibReqData->index_value,spMibReqData->index_count);
			}
			const INT32 i32JobID = i32JobID_temp;
			//2016/09/07 HCL-Sankar Static Analysis Fix 1002990 End.
			const INT32 i32SearchIndex = _getIndexALLPrintJob(i32JobID);
			//サイズ,データを設定.
			/* 2008.11.17 No.312010 sw_saku add S */
			// 2018/10/31 FPT-ChiNM AR1043768【IT5_4.2】【Eagle】 Modification Start.
			if( i32SearchIndex >= 0 
				&& i32SearchIndex <= MIBD_Max_PrintJobListALL){
			/* 2008.11.17 No.312010 sw_saku add E */
				ui32GetSize = _m_irAllVIDMan.vGetJobMonitoringData( i32SearchIndex, ceVID, spMibRspCmdData->pData, spMibReqData->index_value[2]);
			}else{
				if ( spMibRspCmdData->pData != NULL ) { free( spMibRspCmdData->pData ); }
				spMibRspCmdData->pData = NULL;
				ui32GetSize = 0;
			}
			//next_indexの設定.
			const INT32 i32NextSearchIndex = _getNextIndexALLPrintJob(i32JobID);
			_getNextIndexJobMonitoring(ceVID,spMibRspCmdData,spMibReqData,i32SearchIndex,i32NextSearchIndex);
			/* 2008.12.09 No.312803 sw_saku chg E */
		}else{
			ui32GetSize = _m_irAllVIDMan.ui32Get( ceVID, spMibRspCmdData->pData );
			// 2018/10/31 FPT-ChiNM AR1043768【IT5_4.2】【Eagle】 Modification End.
		}
	/* 2008.10.23 No.309942 sw_saku chg E */
	/* 2008.08.18 UV-00536 sw_miya add E */	
// ユビキタス[KRQ-278][スプール分散恒久対応][step2] start.
	} else if( MIOREQ_kmPrtLocalPortStatus == _r_srRcvMsg.MessageID ) {		// MIOからのメッセージがポート負荷状態取得要求の場合. // ユビキタス[KRQ-278][スプール分散恒久対応][step2] Accurev.1010564 .

		// MIOMsgIDからVIDへの変換.
		const VID ceVID = _m_irAllVIDMan.eGetVID( _r_srRcvMsg.MessageID, (CINT32*)spMibReqData->index_value );
		// データタイプの取得.
		eDataType = (MIB_DATA_TYPE)_m_irAllVIDMan.eGetDataType( ceVID );
		// 最大サイズの取得.
		ui32MaxSize = _m_irAllVIDMan.ui32GetMaxSize( ceVID );

		// データ取得領域を確保.
		// 2018/10/31 FPT-ChiNM AR1043768【IT5_4.2】【Eagle】 Modification Start.
		for ( ;; ) {
			spMibRspCmdData->pData = malloc( ui32MaxSize + 1 );
			if ( spMibRspCmdData->pData NE NULL ) {
				break;
			}
			register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;
			if( sl_Tick == 0L ) { sl_Tick = 1L; }
			++sl_Tick;
			SYSF_taskDelay( sl_Tick );
		}
		memset( spMibRspCmdData->pData, 0, ui32MaxSize + 1 );
		
		ui32GetSize = _m_irAllVIDMan.vGetPortStatus( ceVID, spMibRspCmdData->pData );
		// 2018/10/31 FPT-ChiNM AR1043768【IT5_4.2】【Eagle】 Modification End.
// ユビキタス[KRQ-278][スプール分散恒久対応][step2] end.
	} else {
		// ジョブ・Alert情報以外 
		// 応答データの設定 
		// .. MIOMsgIDからVIDへの変換 
		const VID ceVID = _m_irAllVIDMan.eGetVID( _r_srRcvMsg.MessageID, (CINT32*)spMibReqData->index_value );

	DTRACE( ("[---] VID = %d\r\n",ceVID) );

		// .. データの設定 
		eDataType = (MIB_DATA_TYPE)_m_irAllVIDMan.eGetDataType( ceVID );

		ui32MaxSize = _m_irAllVIDMan.ui32GetMaxSize( ceVID );
		// 2018/10/31 FPT-ChiNM AR1043768【IT5_4.2】【Eagle】 Modification Start.
		for ( ;; ) {
			// mallocする領域を+1増やす 	2005.04.25	kawabe
			spMibRspCmdData->pData = malloc( ui32MaxSize + 1 );
			if ( spMibRspCmdData->pData NE NULL ) {
				break;
			}
			register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;	// ｍｓ→ティック数に変換(1000ms)
			if( sl_Tick == 0L ) { sl_Tick = 1L; }
			++sl_Tick;
			SYSF_taskDelay( sl_Tick );
		}
		memset( spMibRspCmdData->pData, 0, ui32MaxSize + 1 );
// 【IT5_4.2】【RQ686】HCL-Kaushik 2017/09/01 Modify START.
		ui32GetSize = _m_irAllVIDMan.ui32Get( ceVID, spMibRspCmdData->pData, spMibReqData->eNwIfIndex);
// 【IT5_4.2】【RQ686】HCL-Kaushik 2017/09/01 Modify END.
	}

	// 取得データ無し. 
	if( isDataExist == False ){
		// メモリ領域解放. 
		if (spMibRspCmdData->pData != NULL ) { free( spMibRspCmdData->pData ); }
		spMibRspCmdData->pData = NULL;
		ui32GetSize = 0;
	}
	// 2018/10/31 FPT-ChiNM AR1043768【IT5_4.2】【Eagle】 Modification End.

	for ( register short i = 0; i < MAX_INDEX; i++ ) {
		spMibRspCmdData->index_value[i] = spMibReqData->index_value[i];
	}
	spMibRspCmdData->index_count = spMibReqData->index_count;
	spMibRspCmdData->MibDataType = eDataType;
	spMibRspCmdData->data_length = ui32GetSize;
	// 2018/10/31 FPT-ChiNM AR1043768【IT5_4.2】【Eagle】 Delete Start.
	//spMibRspCmdData->pData = vpData;
	// 2018/10/31 FPT-ChiNM AR1043768【IT5_4.2】【Eagle】 Delete End.

	// .. 応答データの設定 
	_r_srTraMsg.pParameter = spMibRspCmdData;

	switch( spMibRspCmdData->MibDataType ) {
		case MIB_INT32:
			DTRACE( ("[Tra] pParameter->MibDataType    = INT32\r\n") );
			break;
		case MIB_UINT32:
			DTRACE( ("[Tra] pParameter->MibDataType    = UINT32\r\n") );
			break;
		case MIB_OCTET_STR:
			DTRACE( ("[Tra] pParameter->MibDataType    = OCTET_STR\r\n") );
			break;
		case MIB_STRUCT:
			DTRACE( ("[Tra] pParameter->MibDataType    = STRUCT\r\n") );
			break;
		default:
			DTRACE( ("[Tra] pParameter->MibDataType    = ...UNKNOWN...\r\n") );
			break;
	}
	DTRACE( ("[Tra] pParameter->data_length    = %d\r\n",spMibRspCmdData->data_length) );
	char strMessageTmp[512];
	memset( strMessageTmp, 0, sizeof(strMessageTmp) );
	if ( spMibRspCmdData->pData != NULL ) {
		switch( spMibRspCmdData->MibDataType ) {
			case MIB_INT32:
				DTRACE( ("[Tra] pParameter->pData          = %d\r\n",*(INT32*)(spMibRspCmdData->pData)) );
				break;
			case MIB_UINT32:
				DTRACE( ("[Tra] pParameter->pData          = %d\r\n",*(UINT32*)(spMibRspCmdData->pData)) );
				break;
			case MIB_OCTET_STR:
			case MIB_STRUCT:
				memcpy( (char*)strMessageTmp, (const char*)(spMibRspCmdData->pData), spMibRspCmdData->data_length );
				DTRACE( ("[Tra] pParameter->pData          = %s\r\n",strMessageTmp) );
				break;
			default:
				DTRACE( ("[Tra] pParameter->pData          = ...UNKNOWN...\r\n") );
				break;
		}
	}

	/* 2008.10.23 No.310563 sw_saku chg S */
	DTRACE( ("[Tra] pParameter->index_value\r\n") );
	for(i32_dcount = 0 ; i32_dcount < MAX_INDEX ; i32_dcount++){
		DTRACE( ("%3d,", i32_dcount) );
	}
	DTRACE( ("\r\n"));	
	for(i32_dcount = 0 ; i32_dcount < MAX_INDEX ; i32_dcount++){
		DTRACE( ("%3d,",spMibRspCmdData->index_value[i32_dcount] ) );
	}
	DTRACE( ("\r\n"));
	DTRACE( ("[Tra] pParameter->index_count         = %d\r\n", spMibRspCmdData->index_count) );
	DTRACE( ("[Tra] pParameter->next_index_value\r\n") );
	for(i32_dcount = 0 ; i32_dcount < MAX_INDEX ; i32_dcount++){
		DTRACE( ("%3d,", i32_dcount) );
	}	
	DTRACE( ("\r\n"));
	for(i32_dcount = 0 ; i32_dcount < MAX_INDEX ; i32_dcount++){
		DTRACE( ("%3d,",spMibRspCmdData->next_index_value[i32_dcount] ) );
	}
	DTRACE( ("\r\n") );
	/* 2008.10.23 No.310563 sw_saku chg E */

}

/* 2008.10.23 No.309942 sw_saku add S */
/////////////////////////////////////////////////////////////////////////////////////////.
// MODULE	: MIBC_FuncManMIB::_isCheckIndexValue.
// ABSTRACT	: .
// FUNCTION	: index_value[]が全て0でないかチェックする.
// NOTE		: .
// RETURN	: 全て0の場合：False、それ以外：True.
// CREATE	: .
/////////////////////////////////////////////////////////////////////////////////////////.
Bool MIBC_FuncManMIB::_isCheckIndexValue(const INT32* pi32IndexValue,const UINT8 ui8index_count)
{
	Bool b_result = False;

	for(INT32 s_cout=0; s_cout < (INT32)ui8index_count; s_cout++){
		
		if(*(pi32IndexValue + s_cout) != 0){	//0以外の場合.
			b_result = True;
			break;
		}
	}

	return b_result;
}
/* 2008.10.23 No.309942 sw_saku add E */

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_eGetJobVID 
// ABSTRACT	: MIOMsgIDからVIDへの変換（ジョブデータ用） 
// FUNCTION	: 
// NOTE		:  
// RETURN	: 無し 
// CREATE	: 
/////////////////////////////////////////////////////////////////////////////////////////
VID MIBC_FuncManMIB::_eGetJobVID(
	const MIO_MSG_ID	_r_ceMIOMsgID				// MIO Msg ID 
)
{
	VID eVID = VIDD_VID_Undef;
	
	switch ( _r_ceMIOMsgID ) {
		case MIOREQ_kmJmJobJobSetIndex :
			eVID = VIDD_kmJmJobJobSetIndex_1;	break;
		case MIOREQ_kmJmJobIndex :
			eVID = VIDD_kmJmJobIndex_1;			break;
		case MIOREQ_kmJmJobFunction :
			eVID = VIDD_kmJmJobFunction_1;		break;
		case MIOREQ_kmJmJobStatus :
			eVID = VIDD_kmJmJobStatus_1;			break;
		case MIOREQ_kmJmJobStatusDetails :
			eVID = VIDD_kmJmJobStatusDetails_1;	break;
		case MIOREQ_kmJmJobReceivedTime :
			eVID = VIDD_kmJmJobReceivedTime_1;	break;
		case MIOREQ_kmJmJobCompleteTime :
			eVID = VIDD_kmJmJobCompleteTime_1;	break;
		case MIOREQ_kmJmJobPriority :
			eVID = VIDD_kmJmJobPriority_1;		break;
		case MIOREQ_kmJmJobDivNumber :
			eVID = VIDD_kmJmJobDivNumber_1;		break;
		case MIOREQ_kmJmJobOwner :
			eVID = VIDD_kmJmJobOwner_1;			break;
		case MIOREQ_kmJmJobType :
			eVID = VIDD_kmJmJobType_1;			break;
		case MIOREQ_kmJmJobName :
			eVID = VIDD_kmJmJobName_1;			break;
		case MIOREQ_kmJmJobDocPageNumbers :
			eVID = VIDD_kmJmJobDocPageNumbers_1;	break;
		case MIOREQ_kmJmJobDocCopyNumbers :
			eVID = VIDD_kmJmJobDocCopyNumbers_1;	break;
		case MIOREQ_kmJmJobPageSize :
			eVID = VIDD_kmJmJobPageSize_1;		break;
		case MIOREQ_kmJmJobDestination :
			eVID = VIDD_kmJmJobDestination_1;	break;
		case MIOREQ_kmJmJobDriverJobID:
			eVID = VIDD_kmJmJobDriverJobID_1;
			break;
//	【IT5_3.3】 【1002836】	HCL-Bairavi 2016/07/22 Modified Start.
// 【MinervaSBK】【EMU72コンパイルエラー対応】OPS1深谷 2015/05/27 START.
// 【特ROM対応依頼(MBK-25)】　印刷済み部数のMIB追加対応 Poseidon Vaibhav 2014/07/17 START.
		case MIOREQ_kmJmJobOutputDocCopyNumbers:
			eVID = VIDD_kmJmJobOutputDocCopyNumbers_1;	break;
// 【特ROM対応依頼(MBK-25)】　印刷済み部数のMIB追加対応 Poseidon Vaibhav 2014/07/17 END.
// 【MinervaSBK】【EMU72コンパイルエラー対応】OPS1深谷 2015/05/27 START.
//	【IT5_3.3】 【1002836】	HCL-Bairavi 2016/07/22 Modified End.
		default :	break;
	}

	return eVID;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_isJobIndexData 
// ABSTRACT	: コマンドメッセージの処理 
// FUNCTION	: 
// NOTE		:  
// RETURN	: 無し 
// CREATE	: 
/////////////////////////////////////////////////////////////////////////////////////////
Bool MIBC_FuncManMIB::_isJobIndexData(
	const VID			_r_ceVID				// VID 
)
{
	Bool isJobIndexData = False;
	if ( _r_ceVID == VIDD_kmJmJobIndex_1 ) {
		isJobIndexData = True;
	}
	return isJobIndexData;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_eGetAlertVID 
// ABSTRACT	: MIOMsgIDからVIDへの変換（Alertデータ用） 
// FUNCTION	: 
// NOTE		:  
// RETURN	: 無し 
// CREATE	: 
/////////////////////////////////////////////////////////////////////////////////////////
VID MIBC_FuncManMIB::_eGetAlertVID(
	const MIO_MSG_ID	_r_ceMIOMsgID				// MIO Msg ID 
)
{
	VID eVID = VIDD_VID_Undef;

	switch ( _r_ceMIOMsgID ) {
		case MIOREQ_prtAlertSeverityLevel :
			eVID = VIDD_prtAlertSeverityLevel_1;	break;
		case MIOREQ_prtAlertTrainingLevel :
			eVID = VIDD_prtAlertTrainingLevel_1;	break;
		case MIOREQ_prtAlertGroup :
			eVID = VIDD_prtAlertGroup_1;			break;
		case MIOREQ_prtAlertGroupIndex :
			eVID = VIDD_prtAlertGroupIndex_1;		break;
		case MIOREQ_prtAlertLocation :
			eVID = VIDD_prtAlertLocation_1;			break;
		case MIOREQ_prtAlertCode :
			eVID = VIDD_prtAlertCode_1;				break;
		case MIOREQ_prtAlertDescription :
			eVID = VIDD_prtAlertDescription_1;		break;
		case MIOREQ_prtAlertTime :
			eVID = VIDD_prtAlertTime_1;				break;
		default:	break;
	}

	return eVID;
}
		
/* 2008.10.23 No.309942 sw_saku add S */
/////////////////////////////////////////////////////////////////////////////////////////.
// MODULE	: MIBC_FuncManMIB::_isJudgmentJobMonitoring.
// ABSTRACT	:  .
// FUNCTION	: ジョブ数分応答が必要か判定.
// NOTE		: ジョブ数に応じて応答数が変化するIDが追加された場合はswitch文に追加が必要.
// RETURN	: true	:ジョブ数に応じて応答数が変化する .
// 			: false	:ジョブ数に関係なく応答が一回 .
// CREATE	: .
/////////////////////////////////////////////////////////////////////////////////////////.
bool MIBC_FuncManMIB::_isJudgmentJobMonitoring(
	const VID	_r_ceVID						// VID
) const
{
	bool bRet = false;
	
	switch ( _r_ceVID ) {
		case VIDD_jmJobIDJobSetIndex_Index1 :
		case VIDD_jmJobIDJobIndex_Index1 :
		case VIDD_jmJobState_Index1 :
		case VIDD_jmJobStateReasons1_Index1 :
		case VIDD_jmJobOwner_Index1 :
		case VIDD_jmAttributeValueAsOctets_Index1 :
			bRet = true;
			break;
		default:
			break;
	}

	return bRet;
}

/* 2008.12.09 No.312803 sw_saku chg S */
/////////////////////////////////////////////////////////////////////////////////////////.
// MODULE	: MIBC_FuncManMIB::_getNextIndexJobMonitoring.
// ABSTRACT	: .
// FUNCTION	: 次のジョブモニタリングインデックスの設定（JobMonitoringデータ用）.
// NOTE		: .
// RETURN	: 無し .
// CREATE	: .
/////////////////////////////////////////////////////////////////////////////////////////.
void MIBC_FuncManMIB::_getNextIndexJobMonitoring(
	const VID	_r_ceVID,						// VID.
	MibRspCmdData *_r_spMibRspCmdData,			//応答データ.
	const MibReqData *_r_spMibReqData,			//受信データ.
	const INT32 _i32SearchIndex,				//受信データのインデックス.
	const INT32 _i32NextSearchIndex				//受信データの次のインデックス.
	
)
{
	INT32 i32Count;

	/* 2008.11.17 No.312010 sw_saku add S */
	//無効なジョブIDかつ次のジョブIDがない場合.
	if(_i32NextSearchIndex == -1 && _i32SearchIndex == -1){
		memset(&_r_spMibRspCmdData->next_index_value[0], -1, sizeof(_r_spMibRspCmdData->next_index_value));
		return;
	}
	//次のジョブIDがないかつ場合jmAttributeValueAsOctetsでない場合.
	if(_i32NextSearchIndex == -1 && _r_ceVID != VIDD_jmAttributeValueAsOctets_Index1){
		memset(&_r_spMibRspCmdData->next_index_value[0], -1, sizeof(_r_spMibRspCmdData->next_index_value));
		return;
	}
	//次のジョブIDがないかつ場合jmAttributeValueAsOctetsかつ次の属性がない場合.
	if(_i32NextSearchIndex == -1 && _r_ceVID == VIDD_jmAttributeValueAsOctets_Index1 && _r_spMibReqData->index_value[2] == MIBE_EndofAttributeJob){
		memset(&_r_spMibRspCmdData->next_index_value[0], -1, sizeof(_r_spMibRspCmdData->next_index_value));
		return;
	}
	// ZeusMLK CQ369077 kubota 2012/8/30 START.
	// 静的解析にて下記修正を行ったが、jmAttributeValueAsOctetsは同じVIDで複数のIndexデータを取得するため.
	// 次ジョブデータが存在しない(_i32NextSearchIndexは初期値)状態で、何回かループ処理が実行される.
	// その際、下記処理条件を満たすため、次に検索すべきjmAttributeValueAsOctetsのIndex情報が初期化されてしまい.
	// jmAttributeValueAsOctetsに所属する先頭データしか取れなくなり、シーケンスが崩れるため実装を削除する.
	// 尚、次に検索すべきのIDが無効かチェックする処理は、上記if文で検索条件を満たせているため.
	// 2012/8/30現在、これ以上処理を追加する必要はない.
	// Minerva/Zeus 静的解析 2011/02/21 y.ito start.
	// 次に検索すべきのIDが無効なため（配列のアクセス）.
//	if(_i32NextSearchIndex < 0 ){
//		memset(&_r_spMibRspCmdData->next_index_value[0], -1, sizeof(_r_spMibRspCmdData->next_index_value));
//		return;
//	}
	// Minerva/Zeus 静的解析 2011/02/21 y.ito end.
	// ZeusMLK CQ369077 kubota 2012/8/30 END.

	/* 2008.11.17 No.312010 sw_saku add E */

	//next_index設定.
	for(i32Count = 0; i32Count < MAX_INDEX; i32Count++){
		_r_spMibRspCmdData->next_index_value[i32Count] = _r_spMibReqData->index_value[i32Count];
	}
	
	switch ( _r_ceVID ) {
		case VIDD_jmJobIDJobSetIndex_Index1 :
		case VIDD_jmJobIDJobIndex_Index1 :
			_r_spMibRspCmdData->next_index_value[MAX_INDEX-2] = (sp_PrintJobDataTblAll[_i32NextSearchIndex]->i32JmJobIndex >> 8)& 0x000000FF;	//ジョブID上位8ビット.
			_r_spMibRspCmdData->next_index_value[MAX_INDEX-1] = sp_PrintJobDataTblAll[_i32NextSearchIndex]->i32JmJobIndex & 0x000000FF;			//ジョブID下位8ビット.
			break;

		case VIDD_jmJobState_Index1 :
		case VIDD_jmJobStateReasons1_Index1 :
		case VIDD_jmJobOwner_Index1 :
			_r_spMibRspCmdData->next_index_value[0] = 0x00000001;	//1固定.
			_r_spMibRspCmdData->next_index_value[1] = sp_PrintJobDataTblAll[_i32NextSearchIndex]->i32JmJobIndex;	//ジョブID.
			break;
		case VIDD_jmAttributeValueAsOctets_Index1 :
			_r_spMibRspCmdData->next_index_value[0] = 0x00000001;		//1固定.
			//無効なID.
			if(_i32SearchIndex == -1){
				_r_spMibRspCmdData->next_index_value[1] = sp_PrintJobDataTblAll[_i32NextSearchIndex]->i32JmJobIndex;
				_r_spMibRspCmdData->next_index_value[2] = MIBE_AttributeOther;
			}else{
				//属性検索.
				for(i32Count = 0 ; i32Count < MIBE_AttributeJobNum ;i32Count++){
					if(MIBE_AttributeTbl[i32Count] > _r_spMibReqData->index_value[2]){
						break;
					}
				}
				if(i32Count == MIBE_AttributeJobNum){ 	//最後の属性.
					_r_spMibRspCmdData->next_index_value[1] = sp_PrintJobDataTblAll[_i32NextSearchIndex]->i32JmJobIndex;
					_r_spMibRspCmdData->next_index_value[2] = MIBE_AttributeOther;
				}else{														//次の属性が存在.
					_r_spMibRspCmdData->next_index_value[1] = sp_PrintJobDataTblAll[_i32SearchIndex]->i32JmJobIndex; //受信時と同じindexを設定.
					_r_spMibRspCmdData->next_index_value[2] = MIBE_AttributeTbl[i32Count];
				}
			}
			_r_spMibRspCmdData->next_index_value[3] = 0x00000001;		//1固定.
			break;
		default:
			break;
	}
	return;
}
	
/////////////////////////////////////////////////////////////////////////////////////////.
// MODULE	: MIBC_FuncManMIB::_getSerchIndexJobMonitoring.
// ABSTRACT	: .
// FUNCTION	: ジョブモニタリングインデックスの取得（JobMonitoringデータ用）.
// NOTE		: 存在しないジョブIDを取得した場合は、0を返却する.
//			  存在するジョブIDの場合は、その値を返却する.
// RETURN	: 受信メッセージから取得したジョブID .
// CREATE	: .
/////////////////////////////////////////////////////////////////////////////////////////.
INT32 MIBC_FuncManMIB::_getSerchIndexJobMonitoring(
	const VID	_r_ceVID,						// VID.
	const INT32 *_r_index_value,				//index_value.
	const UINT8 ui8index_count					//index_count.
)
{
	INT32 i32jobID = (INT32)-1;
	//開始メッセージの場合.
	if(!_isCheckIndexValue(_r_index_value,ui8index_count)){
		i32jobID = (INT32)0;
		return i32jobID;
	}
	
	//ジョブID取得.
	switch ( _r_ceVID ) {
		case VIDD_jmJobIDJobSetIndex_Index1 :
		case VIDD_jmJobIDJobIndex_Index1 :
			
			i32jobID = (_r_index_value[MAX_INDEX-2] << 8) + _r_index_value[MAX_INDEX-1];
			break;
		case VIDD_jmJobState_Index1 :
		case VIDD_jmJobStateReasons1_Index1 :
		case VIDD_jmJobOwner_Index1 :
			i32jobID = _r_index_value[1];
			break;
		case VIDD_jmAttributeValueAsOctets_Index1 :
			i32jobID = _r_index_value[1];
			break;
		default:
			break;
	}

	return i32jobID;
}
/* 2008.10.23 No.309942 sw_saku add E */	
/* 2008.12.09 No.312803 sw_saku chg E */

/* 2008.12.09 No.312803 sw_saku add S */
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_getIndexALLPrintJob 
// ABSTRACT	: sp_PrintJobDataTblAll配列のIndexを返却する（JobMonitoringデータ用） 
// FUNCTION	: 
// NOTE		:  
// RETURN	: sp_PrintJobDataTblAll配列のIndex 
// CREATE	: 
/////////////////////////////////////////////////////////////////////////////////////////
INT32 MIBC_FuncManMIB::_getIndexALLPrintJob(
	const INT32	_i32jobID
){

	INT32 i32ret = (INT32)-1;
	
	//ジョブIDが存在するか判定する.
	//存在しない場合は、返却値に0を設定する.
	for(INT32 i = (INT32)0 ; i < MIBD_Max_PrintJobListALL ; i++)
	{
		if(sp_PrintJobDataTblAll[i] == NULL){
			return -1;
		}
		if((INT32)sp_PrintJobDataTblAll[i]->i32JmJobIndex == _i32jobID){
			i32ret = i;
			break;
		}
	}
	return i32ret;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_getNextIndexALLPrintJob 
// ABSTRACT	: sp_PrintJobDataTblAll配列のNextIndexを返却する（JobMonitoringデータ用） 
// FUNCTION	: 
// NOTE		:  
// RETURN	: 無し 
// CREATE	: 
/////////////////////////////////////////////////////////////////////////////////////////
INT32 MIBC_FuncManMIB::_getNextIndexALLPrintJob(
	const INT32	_i32jobID				
){

	INT32 i32ret = (INT32)-1;
	
	//次のジョブIDを返却する.
	//存在しないジョブIDの場合でも、次に大きなジョブIDを返却する.
	for(INT32 i = (INT32)0 ; i < MIBD_Max_PrintJobListALL ; i++)
	{
		if(sp_PrintJobDataTblAll[i] == NULL){
			return -1;
		}
		
		if((INT32)sp_PrintJobDataTblAll[i]->i32JmJobIndex > _i32jobID){
			i32ret = i;
			break;
		}
	}
	return i32ret;
}
/* 2008.12.09 No.312803 sw_saku add E */

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_eGetJobMonitoringVID 
// ABSTRACT	: MIOMsgIDからVIDへの変換（JobMonitoringデータ用） 
// FUNCTION	: 
// NOTE		:  
// RETURN	: 無し 
// CREATE	: 
/////////////////////////////////////////////////////////////////////////////////////////
VID MIBC_FuncManMIB::_eGetJobMonitoringVID(
	const MIO_MSG_ID	_r_ceMIOMsgID				// MIO Msg ID 
){
	VID eVID = VIDD_VID_Undef;

	switch ( _r_ceMIOMsgID ) {
		case MIOREQ_jmGeneralNumberOfActiveJobs :
			eVID = VIDD_jmGeneralNumberOfActiveJobs_Index1;
			break;
		case MIOREQ_jmGeneralOldestActiveJobIndex:
			eVID = VIDD_jmGeneralOldestActiveJobIndex_Index1;
			break;
		case MIOREQ_jmGeneralNewestActiveJobIndex:
			eVID = VIDD_jmGeneralNewestActiveJobIndex_Index1;
			break;
		case MIOREQ_jmGeneralJobSetName:
			eVID = VIDD_jmGeneralJobSetName_Index1;
			break;
		case MIOREQ_jmJobIDJobSetIndex:
			eVID = VIDD_jmJobIDJobSetIndex_Index1;
			break;
		case MIOREQ_jmJobIDJobIndex:
			eVID = VIDD_jmJobIDJobIndex_Index1;
			break;
		case MIOREQ_jmJobState:
			eVID = VIDD_jmJobState_Index1;
			break;
		case MIOREQ_jmJobStateReasons1:
			eVID = VIDD_jmJobStateReasons1_Index1;
			break;
		case MIOREQ_jmJobOwner:
			eVID = VIDD_jmJobOwner_Index1;
			break;
		case MIOREQ_jmAttributeValueAsOctets:
			eVID = VIDD_jmAttributeValueAsOctets_Index1;
			break;
		default:
			break;
	}

	return eVID;
}

/* 2008.08.18 UV-00536 sw_miya add E */	

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_vProcCommandMsg 
// ABSTRACT	: コマンドメッセージの処理 
// FUNCTION	: 
// NOTE		: コマンドメッセージは全てIJCの応答が返ってきてからVIDにセットする 
//			  キャラクターセットは全てUS-ASCIIとする(どのコードで設定されているか情報がない) 
// RETURN	: 無し 
// CREATE	: 01-06-06	廣田 
//			  新規作成 
//			  02-05-20	河辺
//			  NM構造見直しのため変更
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_FuncManMIB::_vProcCommandMsg(
	MsgMioToApp&		_r_srRcvMsg		// MIOからの受信メッセージ 
)
{
	DTRACE( ("*** MIB Command Message ***\r\n") );
	DTRACE( ("[Rcv] SequenceNo    = %d\r\n",_r_srRcvMsg.SequenceNo) );
	DTRACE( ("[Rcv] ApplicationID = %d\r\n",_r_srRcvMsg.ApplicationID) );
	DTRACE( ("[Rcv] MessageID     = %d\r\n",_r_srRcvMsg.MessageID) );

	// 受信データの取得 
	register MibRspCmdData* spMibRspCmdData = (MibRspCmdData*)_r_srRcvMsg.pParameter;

	DTRACE( ("[Rcv] pParameter->index_value[0] = %d\r\n",spMibRspCmdData->index_value[0]) );
	DTRACE( ("[Rcv] pParameter->index_value[1] = %d\r\n",spMibRspCmdData->index_value[1]) );
	DTRACE( ("[Rcv] pParameter->index_count    = %d\r\n",spMibRspCmdData->index_count) );
	switch( spMibRspCmdData->MibDataType ) {
		case MIB_INT32:
			DTRACE( ("[Rcv] pParameter->MibDataType    = INT32\r\n") );
			break;
		case MIB_UINT32:
			DTRACE( ("[Rcv] pParameter->MibDataType    = UINT32\r\n") );
			break;
		case MIB_OCTET_STR:
			DTRACE( ("[Rcv] pParameter->MibDataType    = OCTET_STR\r\n") );
			break;
		case MIB_STRUCT:
			DTRACE( ("[Rcv] pParameter->MibDataType    = STRUCT\r\n") );
			break;
		default:
			DTRACE( ("[Rcv] pParameter->MibDataType    = ...UNKNOWN...\r\n") );
			break;
	}
	DTRACE( ("[Rcv] pParameter->data_length    = %d\r\n",spMibRspCmdData->data_length) );
	char strMessageTmp[512];
	memset( strMessageTmp, 0, sizeof(strMessageTmp) );
	switch( spMibRspCmdData->MibDataType ) {
		case MIB_INT32:
			DTRACE( ("[Rcv] pParameter->pData          = %d\r\n",*(INT32*)(spMibRspCmdData->pData)) );
			break;
		case MIB_UINT32:
			DTRACE( ("[Rcv] pParameter->pData          = %d\r\n",*(UINT32*)(spMibRspCmdData->pData)) );
			break;
		case MIB_OCTET_STR:
		case MIB_STRUCT:
			memcpy( (char*)strMessageTmp, (const char*)(spMibRspCmdData->pData), spMibRspCmdData->data_length );
			DTRACE( ("[Rcv] pParameter->pData          = %s\r\n",strMessageTmp) );
			break;
		default:
			DTRACE( ("[Rcv] pParameter->pData          = ...UNKNOWN...\r\n") );
			break;
	}

	VID ceVID = _m_irAllVIDMan.eGetVID( _r_srRcvMsg.MessageID, (CINT32*)spMibRspCmdData->index_value );

	// kmJmJobStatusにcancelRequest(2)がセットされたら、pDataにジョブIDを指定してジョブ破棄を行う	2004.06.03 kawabe
	Bool bl_IJCSet = True;
	if ( _r_srRcvMsg.MessageID EQ MIOCMD_kmJmJobStatus ) {
		ceVID = VIDD_kmJmJobStatus_1;
		if ( *((INT32*)spMibRspCmdData->pData) EQ 2 )  {
			*((INT32*)spMibRspCmdData->pData) = spMibRspCmdData->index_value[1];
		} else {
			bl_IJCSet = False;
		}
	}
	
	// IJCにデータをセットする	
	// 修正	2004.06.03 kawabe
	MIBE_CmdResult eResult = MIBE_CmdResult_NO_ERR;
	if ( bl_IJCSet ) {
		eResult = _m_irAllVIDMan.eIJCSet( ceVID, spMibRspCmdData->pData, spMibRspCmdData->data_length, MIBE_CharacterSet_USASCII );
	} else {
		eResult = MIBE_CmdResult_BAD_VALUE;
	}

	DTRACE( ("[---] VID = %d\r\n",ceVID) );

	// MIOへの応答データの設定 
	// .. 領域確保 
	register MibRptData* spMibRptData = NULL;
	for ( ;; ) {
		spMibRptData = (MibRptData*)malloc( sizeof(MibRptData) );
		if ( spMibRptData NE NULL ) {
			break;
		}
		register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;	// ｍｓ→ティック数に変換(1000ms)
		if( sl_Tick == 0L ) { sl_Tick = 1L; }
		++sl_Tick;
		SYSF_taskDelay( sl_Tick );
	}

	// .. 結果 
	spMibRptData->MioCmdMib = (MIOCMD_MIB_RESULT)eResult;
	// .. インデックスの設定 
	for ( register short i = 0; i < MAX_INDEX; i++ ) {
		spMibRptData->MibRpt.index_value[i] = spMibRspCmdData->index_value[i];
	}
	spMibRptData->MibRpt.index_count = spMibRspCmdData->index_count;
	spMibRptData->MibRpt.data_length = spMibRspCmdData->data_length;
	spMibRptData->MibRpt.MibDataType = spMibRspCmdData->MibDataType;

	// .. 実データの設定 
	void* vpData = NULL;
	for ( ;; ) {
		vpData = malloc( spMibRptData->MibRpt.data_length );
		if ( vpData NE NULL ) {
			break;
		}
		register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;	// ｍｓ→ティック数に変換(1000ms)
		if( sl_Tick == 0L ) { sl_Tick = 1L; }
		++sl_Tick;
		SYSF_taskDelay( sl_Tick );
	}
	spMibRptData->MibRpt.pData = vpData;

	switch( spMibRptData->MioCmdMib ) {
		case MIOCMD_MIB_NO_ERR:
			DTRACE( ("[Tra] pParameter->MioCmdMib      = NO ERR\r\n") );
			break;
		case MIOCMD_MIB_NO_SUCH_NAME:
			DTRACE( ("[Tra] pParameter->MioCmdMib      = NO SUCH NAME\r\n") );
			break;
		case MIOCMD_MIB_BAD_VALUE:
			DTRACE( ("[Tra] pParameter->MioCmdMib      = BAD VALUE\r\n") );
			break;
		case MIOCMD_MIB_GENERAL_ERR:
			DTRACE( ("[Tra] pParameter->MioCmdMib      = GENERAL ERR\r\n") );
			break;
		default:
			DTRACE( ("[Tra] pParameter->MioCmdMib      = ...UNKNOWN...\r\n") );
			break;
	}
	DTRACE( ("\r\n") );

	// データ領域の開放 	2004.07.22	kawabe
	// 2018/11/10 FPT-Huy AR1063739【IT6_1.0】【Eagle】 Modification Start.
	//free( spMibRspCmdData->pData );
	if(spMibRspCmdData->pData != NULL)
	{
		free(spMibRspCmdData->pData);
		spMibRspCmdData->pData = NULL;
	}
	// 2018/11/10 FPT-Huy AR1063739【IT6_1.0】【Eagle】 Modification End.

	// MIOへの応答メッセージの設定 
	APP_MSG_ID eMsgID = (APP_MSG_ID)_m_irAllVIDMan.eGetCmdMIOMsgID( ceVID );
	_m_irMIOMsgMan.eMsgTrans( eMsgID, spMibRptData, _r_srRcvMsg.SequenceNo );
}

/////////////////////////////////////////////////////////////////////////////////////////
// MIOへの初期データの通知
////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::vProcRptInitData 
// ABSTRACT	: 初期データの通知 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 無し 
// CREATE	: 02-05-20	河辺
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_FuncManMIB::vProcRptInitData()
{
	static MioSupportOID* spMIOSupportOID; //初期通信データ領域を確保するためのポインター 
	spMIOSupportOID = _spProcRptInitDataMakeRegion();

	// 受信データの処理 
	for ( int eVID = (int)VIDD_VID_Start; eVID < (int)VIDD_VID_End; eVID++ ) {
		// MIO通知データの設定 
		_vProcRptInitDataMIO( spMIOSupportOID, (VID)eVID );
	}

	// MIOからの要求を処理するタスクおよび、定期的に起動されるタスクを生成/起動する
	// ここでいい？？
	n_vAtInitMsgFromIJC();

	// MIOへの応答メッセージの設定 
	_m_irMIOMsgMan.eMsgTrans( MIORPT_SupportOID, spMIOSupportOID );
}

////////////////////////////////////////////////////////////////////////////////////////
#if 1
void MIBC_FuncManMIB::vProcRptInitDataUpdating()
{

	static MioSupportOID* spMIOSupportOID_Entry = NULL; // 追加データ領域を確保するためのポインター.
	spMIOSupportOID_Entry = _spProcRptInitDataMakeRegion();
	spMIOSupportOID_Entry->SupportOID_Cnt = 0;


	static MioSupportOID* spMIOSupportOID_Remove = NULL; // 削除データ領域を確保するためのポインター.
	spMIOSupportOID_Remove = _spProcRptInitDataMakeRegion();
	spMIOSupportOID_Remove->SupportOID_Cnt = 0;

	// 受信データの処理 
	for ( int eVID = (int)VIDD_VID_Start; eVID < (int)VIDD_VID_End; eVID++ ) {
		// MIO通知データの設定 
		_vProcRptInitDataMIO( spMIOSupportOID_Entry, spMIOSupportOID_Remove, (VID)eVID );
	}

	// MIOへの応答メッセージの設定.
	if( 0 < spMIOSupportOID_Entry->SupportOID_Cnt ) {
		_m_irMIOMsgMan.eMsgTrans( MIORPT_DataIndexEntry, spMIOSupportOID_Entry ); // 追加.
	}
	else {
		if( spMIOSupportOID_Entry->pMioObjectID )
			free( spMIOSupportOID_Entry->pMioObjectID );
		if( spMIOSupportOID_Entry )
			free( spMIOSupportOID_Entry );
	}

	if( 0 < spMIOSupportOID_Remove->SupportOID_Cnt ) {
		_m_irMIOMsgMan.eMsgTrans( MIORPT_DataIndexRemove, spMIOSupportOID_Remove ); // 削除.
	}
	else {
		if( spMIOSupportOID_Remove->pMioObjectID )
			free( spMIOSupportOID_Remove->pMioObjectID );
		if( spMIOSupportOID_Remove )
			free( spMIOSupportOID_Remove );
	}
}


void MIBC_FuncManMIB::_vProcRptInitDataMIO(
	MioSupportOID*		_r_spMIOSupportOID_Entry,
	MioSupportOID*		_r_spMIOSupportOID_Remove,
	const VID			_r_ceVID				// VID 
)
{
	if ( !_m_irAllVIDManOld.isVIDDataExist( _r_ceVID ) && _m_irAllVIDMan.isVIDDataExist( _r_ceVID ) ) {
		// 無効 -> 有効の場合  MIOに通知する追加データ.
		if ( _isProcRptInitDataMIOSet( _r_ceVID ) ) {
			// MIO_MSG_IDのREQメッセージ.
			MIO_MSG_ID eReqMIOMsgID = _m_irAllVIDMan.eGetReqMIOMsgID( _r_ceVID );
			_vProcRptInitDataMIOSet( eReqMIOMsgID, _r_ceVID, _r_spMIOSupportOID_Entry );
			// MIO_MSG_IDのCMDメッセージ.
			MIO_MSG_ID eCmdMIOMsgID = _m_irAllVIDMan.eGetCmdMIOMsgID( _r_ceVID );
			if ( eCmdMIOMsgID != MIBD_UndefMIOMsgID ) {
				_vProcRptInitDataMIOSet( eCmdMIOMsgID, _r_ceVID, _r_spMIOSupportOID_Entry );
			}
		}
	}
	else if ( _m_irAllVIDManOld.isVIDDataExist( _r_ceVID ) && !_m_irAllVIDMan.isVIDDataExist( _r_ceVID ) ) {
		// 有効 -> 無効の場合  MIOに通知する削除データ.
		if ( _isProcRptInitDataMIOSet( _r_ceVID ) ) {
			// MIO_MSG_IDのREQメッセージ.
			MIO_MSG_ID eReqMIOMsgID = _m_irAllVIDManOld.eGetReqMIOMsgID( _r_ceVID );
			_vProcRptInitDataMIOSetOld( eReqMIOMsgID, _r_ceVID, _r_spMIOSupportOID_Remove );

			// MIO_MSG_IDのCMDメッセージ.
			MIO_MSG_ID eCmdMIOMsgID = _m_irAllVIDManOld.eGetCmdMIOMsgID( _r_ceVID );
			if ( eCmdMIOMsgID != MIBD_UndefMIOMsgID ) {
				_vProcRptInitDataMIOSetOld( eCmdMIOMsgID, _r_ceVID, _r_spMIOSupportOID_Remove );
			}
		}
	}
	else if ( _m_irAllVIDManOld.isVIDDataExist( _r_ceVID ) && _m_irAllVIDMan.isVIDDataExist( _r_ceVID ) ) {
		// 有効 -> 有効の場合.
		// non.
	}
	else {
		// 無効 -> 無効の場合.
		// non.
	}
}
#else
void MIBC_FuncManMIB::vProcRptInitDataUpdating()
{
	// 受信データの処理 
	for ( int eVID = (int)VIDD_VID_Start; eVID < (int)VIDD_VID_End; eVID++ ) {
		// MIO通知データの設定.
		_vProcRptInitDataMIOandTrans( (VID)eVID );
	}
}


void MIBC_FuncManMIB::_vProcRptInitDataMIOandTrans(
	const VID			_r_ceVID				// VID 
)
{
	if ( !_m_irAllVIDManOld.isVIDDataExist( _r_ceVID ) && _m_irAllVIDMan.isVIDDataExist( _r_ceVID ) ) {
		// 無効 -> 有効の場合  MIOに通知する追加データ
		static MioSupportOID* _r_spMIOSupportOID_Entry; // 追加データ領域を確保するためのポインター 
		_r_spMIOSupportOID_Entry = _spProcRptInitDataMakeRegion(); // 全確保する必要はない！！
		_r_spMIOSupportOID_Entry->SupportOID_Cnt = 0;

		if ( _isProcRptInitDataMIOSet( _r_ceVID ) ) {
			// MIO_MSG_IDのREQメッセージ 
			MIO_MSG_ID eReqMIOMsgID = _m_irAllVIDMan.eGetReqMIOMsgID( _r_ceVID );
			_vProcRptInitDataMIOSet( eReqMIOMsgID, _r_ceVID, _r_spMIOSupportOID_Entry );

			// MIO_MSG_IDのCMDメッセージ 
			MIO_MSG_ID eCmdMIOMsgID = _m_irAllVIDMan.eGetCmdMIOMsgID( _r_ceVID );
			if ( eCmdMIOMsgID != MIBD_UndefMIOMsgID ) {
				_vProcRptInitDataMIOSet( eCmdMIOMsgID, _r_ceVID, _r_spMIOSupportOID_Entry );
			}
		}
		// MIOへの応答メッセージの設定.
		_m_irMIOMsgMan.eMsgTrans( MIORPT_DataIndexEntry, _r_spMIOSupportOID_Entry ); // 追加.
	}
	else if ( _m_irAllVIDManOld.isVIDDataExist( _r_ceVID ) && !_m_irAllVIDMan.isVIDDataExist( _r_ceVID ) ) {
		// 有効 -> 無効の場合  MIOに通知する削除データ.
		static MioSupportOID* _r_spMIOSupportOID_Remove; // 削除データ領域を確保するためのポインター 
		_r_spMIOSupportOID_Remove = _spProcRptInitDataMakeRegion(); // 全確保する必要はない！！
		_r_spMIOSupportOID_Remove->SupportOID_Cnt = 0;

		if ( _isProcRptInitDataMIOSet( _r_ceVID ) ) {
			// MIO_MSG_IDのREQメッセージ 
			MIO_MSG_ID eReqMIOMsgID = _m_irAllVIDManOld.eGetReqMIOMsgID( _r_ceVID );
			_vProcRptInitDataMIOSetOld( eReqMIOMsgID, _r_ceVID, _r_spMIOSupportOID_Remove );

			// MIO_MSG_IDのCMDメッセージ 
			MIO_MSG_ID eCmdMIOMsgID = _m_irAllVIDManOld.eGetCmdMIOMsgID( _r_ceVID );
			if ( eCmdMIOMsgID != MIBD_UndefMIOMsgID ) {
				_vProcRptInitDataMIOSetOld( eCmdMIOMsgID, _r_ceVID, _r_spMIOSupportOID_Remove );
			}
		}
		// MIOへの応答メッセージの設定.
		_m_irMIOMsgMan.eMsgTrans( MIORPT_DataIndexRemove, _r_spMIOSupportOID_Remove ); // 削除.
	}
	else if ( _m_irAllVIDManOld.isVIDDataExist( _r_ceVID ) && _m_irAllVIDMan.isVIDDataExist( _r_ceVID )) {
		// 有効 -> 有効の場合.
		// non.
	}
	else {
		// 無効 -> 無効の場合.
		// non.
	}
}

#endif
//2021/06/30 [IT6_2.1_MR_CheckIn][TAS000000008827] Start.

////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::vProcJobDataInit 
// ABSTRACT	: ジョブ情報の初期化 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 無し 
// CREATE	: 03-07-08	松本 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_FuncManMIB::vProcJobDataInit()
{
	// NMジョブ情報の初期化処理 
	_m_irAllVIDMan.vInitJobTbl();

	// Minerva/Zeus 静的解析 2011/02/21 y.ito start.
	for( int i_loop = 0; i_loop < MIBD_Max_ActiveJobListAll; i_loop++ )
	{
		i32WSDJobActiveJobIDOld[i_loop] = MIBD_UndefJobID;
		i32WSDJobActiveJobIDNew[i_loop] = MIBD_UndefJobID;
	}
	// Minerva/Zeus 静的解析 2011/02/21 y.ito end.
	memset( spWSDPrinterStatusSummaryOld, 0, sizeof( t_sWSDPrinterStatusSummary ) );
	memset( spWSDPrinterStatusSummaryNew, 0, sizeof( t_sWSDPrinterStatusSummary ) );
	memset( s_TransferJobTbl, 0, sizeof( MIBS_JobDataTblTransferStock )*MIBD_Max_ActiveJobListAll );
	
	for( int i = 0; i < MIBD_Max_ActiveJobListAll; i++ )
	{
		s_TransferJobTbl[i].i32JmJobTransferID = MIBD_UndefJobID;
		s_TransferJobTbl[i].i8JmJobFlg = MIBD_TransferJobflgOff;
	}
	
	/* 2008.08.18 UV-00536 sw_miya add S */
	_m_irAllVIDMan.vInitJobTbl();
	
	/* 2008.08.18 UV-00536 sw_miya add E */
}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_spProcRptInitDataMakeRegion 
// ABSTRACT	: MIOへの初期通信データ領域の確保 
// FUNCTION	: 
// NOTE		: 
// RETURN	: MioSupportOID* spMIOSupportOID 
// CREATE	: 01-06-21	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
MioSupportOID* MIBC_FuncManMIB::_spProcRptInitDataMakeRegion()
{
	// MIOへの初期通知領域の確保(MioSupportOIDのMioObjectIDの領域)  
	// .. VIDでRWの時、MIO_MSG_IDはREQとCMDの二つになることがあるので 
	// .. 領域としてはMIOのMsgIDの数だけ取っておく 
	register MioSupportOID* spMIOSupportOID = NULL;
	for ( ;; ) {
		spMIOSupportOID = (MioSupportOID*)malloc(sizeof(MioSupportOID));
		if ( spMIOSupportOID NE NULL ) {
			break;
		}
		register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;	// ｍｓ→ティック数に変換(1000ms)
		if( sl_Tick == 0L ) { sl_Tick = 1L; }
		++sl_Tick;
		SYSF_taskDelay( sl_Tick );
	}

	register INT32 i32NumOfMIOMsgID = ( (MIOREQ_MIBEND - MIOREQ_MIBSTART) + (MIOCMD_MIBEND - MIOCMD_MIBSTART) );
	register MioObjectID* spMIOObjectID = NULL;
	for ( ;; ) {
		spMIOObjectID = (MioObjectID*)malloc(sizeof(MioObjectID)*i32NumOfMIOMsgID*20);
		if ( spMIOObjectID NE NULL ) {
			break;
		}
		register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;	// ｍｓ→ティック数に変換(1000ms)
		if( sl_Tick == 0L ) { sl_Tick = 1L; }
		++sl_Tick;
		SYSF_taskDelay( sl_Tick );
	}

	spMIOSupportOID->SupportOID_Cnt = 0;
	spMIOSupportOID->pMioObjectID = spMIOObjectID;
	return spMIOSupportOID;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_vProcRptInitDataMIO 
// ABSTRACT	: MIOへの初期通信データの設定  
// FUNCTION	: 
// NOTE		: 
// RETURN	: 無し 
// CREATE	: 01-06-07	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_FuncManMIB::_vProcRptInitDataMIO(
	MioSupportOID*		_r_spMIOSupportOID,
	const VID			_r_ceVID				// VID 
)
{
	if ( _m_irAllVIDMan.isVIDDataExist( _r_ceVID ) ) {
		// MIOに初期通知するデータの場合だけ、データを作成 
		if ( _isProcRptInitDataMIOSet( _r_ceVID ) ) {
			// MIO_MSG_IDのREQメッセージ 
			MIO_MSG_ID eReqMIOMsgID = _m_irAllVIDMan.eGetReqMIOMsgID( _r_ceVID );
			_vProcRptInitDataMIOSet( eReqMIOMsgID, _r_ceVID, _r_spMIOSupportOID );

			// MIO_MSG_IDのCMDメッセージ 
			MIO_MSG_ID eCmdMIOMsgID = _m_irAllVIDMan.eGetCmdMIOMsgID( _r_ceVID );
			if ( eCmdMIOMsgID != MIBD_UndefMIOMsgID ) {
				_vProcRptInitDataMIOSet( eCmdMIOMsgID, _r_ceVID, _r_spMIOSupportOID );
			}
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_isProcRptInitDataMIOSet 
// ABSTRACT	: MIOに初期通知するデータかどうかのチェック  
// FUNCTION	: 
// NOTE		: ジョブ情報は最初のデータ(インデックス1,1)のみ送付 
// RETURN	: Bool isSet : True : MIOに初期通知してよいデータ 
// CREATE	: 01-06-21	廣田 
//			  新規作成 
//			  01-12-14  山畑 
//			　JobIndexがVIDD_kmJmJobIndex_○に変更したのに対応 
/////////////////////////////////////////////////////////////////////////////////////////
Bool MIBC_FuncManMIB::_isProcRptInitDataMIOSet(
	const VID				_r_ceVID				// VID 
)
{
	register Bool isSet = True;
	return isSet;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_vProcRptInitDataMIOSet 
// ABSTRACT	: MIOへの初期通信データの設定 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 無し 
// CREATE	: 01-06-08	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_FuncManMIB::_vProcRptInitDataMIOSet(
	const MIO_MSG_ID		_r_ceMIOMsgID,			// MIO Msg ID 
	const VID				_r_ceVID,				// VID 
	MioSupportOID*			_r_spMIOSupportOID		//  
)
{
	if ( _r_ceMIOMsgID NE MIBD_UndefMIOMsgID ) {
		// データをセットするところまでポインタを進める 
		register MioObjectID* spMIOObjectID = _r_spMIOSupportOID->pMioObjectID;
		UINT32& ui32SupportOID_Cnt = _r_spMIOSupportOID->SupportOID_Cnt;

		// データの設定 
		spMIOObjectID[ ui32SupportOID_Cnt ].MessageID = (APP_MSG_ID)_r_ceMIOMsgID;
		// .. インデックスの取得 
		_m_irAllVIDMan.vGetIndex( _r_ceVID, (INT32*)spMIOObjectID[ ui32SupportOID_Cnt ].index_value );
		spMIOObjectID[ ui32SupportOID_Cnt ].index_count = _m_irAllVIDMan.ui8GetNumOfIndex( _r_ceVID );
		// サポートOIDの数を増やす 
		ui32SupportOID_Cnt++;
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
#if 1
void MIBC_FuncManMIB::_vProcRptInitDataMIOSetOld(
	const MIO_MSG_ID		_r_ceMIOMsgID,			// MIO Msg ID 
	const VID				_r_ceVID,				// VID 
	MioSupportOID*			_r_spMIOSupportOID		//  
)
{
	if ( _r_ceMIOMsgID NE MIBD_UndefMIOMsgID ) {
		// データをセットするところまでポインタを進める 
		register MioObjectID* spMIOObjectID = _r_spMIOSupportOID->pMioObjectID;
		UINT32& ui32SupportOID_Cnt = _r_spMIOSupportOID->SupportOID_Cnt;

		// データの設定 
		spMIOObjectID[ ui32SupportOID_Cnt ].MessageID = (APP_MSG_ID)_r_ceMIOMsgID;
		// .. インデックスの取得 
		_m_irAllVIDManOld.vGetIndex( _r_ceVID, (INT32*)spMIOObjectID[ ui32SupportOID_Cnt ].index_value );
		spMIOObjectID[ ui32SupportOID_Cnt ].index_count = _m_irAllVIDManOld.ui8GetNumOfIndex( _r_ceVID );

		// サポートOIDの数を増やす 
		ui32SupportOID_Cnt++;
	}
}
#endif
//2021/06/30 [IT6_2.1_MR_CheckIn][TAS000000008827] End.
/////////////////////////////////////////////////////////////////////////////////////////
// MIOへの変更データの通知（定期取得）
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::vGetIJCData
// ABSTRACT	: IJCデータ取得関数
// FUNCTION	: IJCのJOBデータ、Alertデータを取得する 
// NOTE		: 
// RETURN	: 無し
// CREATE	: 02-05-20	河辺
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_FuncManMIB::vGetIJCData()
{
	// Alertデータの取得 
	_vAlertDataGet();
	// JOBデータの取得 
	_vJobDataGet();
	
	/* 2008.08.18 UV-00536 sw_miya add S */
	
	_vJobMonitoringDataGet();
	/* 2008.08.18 UV-00536 sw_miya add E */

// ユビキタス[KRQ-278][スプール分散恒久対応][step2] start.
	// ポート負荷状態の取得.
	_vPortDataGet();
// ユビキタス[KRQ-278][スプール分散恒久対応][step2] end.
}
/* 2008.08.18 UV-00536 sw_miya add S */
/////////////////////////////////////////////////////////////////////////////////////////.
// MODULE	: MIBC_FuncManMIB::_vJobMonitoringDataGet.
// ABSTRACT	: JobMonitoringデータの取得.
// FUNCTION	: 
// NOTE		: 
// RETURN	: 
// CREATE	: 
//			  新規作成.
/////////////////////////////////////////////////////////////////////////////////////////.

void MIBC_FuncManMIB::_vJobMonitoringDataGet()
{
	try{
	// 確保した領域を初期化する 
	memset( s_PrintJobDataTblActive, 0, sizeof(MIBS_PrintJobDataTbl)*MIBD_Max_ActiveJobListAll );
	memset( s_PrintJobDataTblLog, 0, sizeof(MIBS_PrintJobDataTbl)*MIBD_Max_JobLogListAll );
	/* 2008.10.16 No.310563 sw_saku add S */
	memset( sp_PrintJobDataTblAll, 0, sizeof(MIBS_PrintJobDataTbl*)*MIBD_Max_kmJmJobIndex );
	/* 2008.10.16 No.310563 sw_saku add E */
	
	
	for ( short i = 0; i < MIBD_Max_ActiveJobListAll; i++ ) {
		s_PrintJobDataTblActive[i].i32JmJobIndex = MIBD_UndefJobID;
	}
	for ( i = 0; i < MIBD_Max_JobLogListAll; i++ ) {
		s_PrintJobDataTblLog[i].i32JmJobIndex = MIBD_UndefJobID;
	}
	
	// アクティブジョブ情報の取得 
	short ss_ActiveJobNum = getActivePrintJobData( s_PrintJobDataTblActive );
	
	// 履歴ジョブ情報の取得 
	short ss_LogJobNum = getDonePrintJobData( s_PrintJobDataTblLog );
	/* 2008.10.16 No.310563 sw_saku add S */
	// すべてのプリントジョブ情報配列の生成
	getAllPrintJobData( sp_PrintJobDataTblAll ,s_PrintJobDataTblActive ,s_PrintJobDataTblLog , ss_ActiveJobNum , ss_LogJobNum);
	/* 2008.10.16 No.310563 sw_saku add E */
	} catch (const std::bad_alloc& e) {
			b_NVDG_InitModule = SetMibDetailCode(11,MIBD_NewDetailCode_MemoryAccessFailureLoc11);

			if( False == b_NVDG_InitModule) {
				SYSF_logMsg("[MIB] Error Code recording Failed Loc11!!\n", 0,0,0,0,0,0);	
			}
    	}
	return;
}

/* 2008.08.18 UV-00536 sw_miya add E */
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_vAlertDataGet
// ABSTRACT	: Alertデータの取得 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 無し
// CREATE	: 02-05-22	河辺
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_FuncManMIB::_vAlertDataGet()
{
	getAlertEntry();
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_vJobDataGet
// ABSTRACT	: JOBデータの取得 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 無し
// CREATE	: 02-05-22	河辺
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
Bool b_MibStaticTestChecked = False;		/* 領域サイズ表示用 */
void MIBC_FuncManMIB::_vJobDataGet()
{

	// JobIDを取得するためのデータ領域の確保 
	static INT32 i32pCrntData[MIBD_Max_kmJmJobIndex];
	if( b_MibStaticTestChecked )
	{
		SYSF_logMsg("i32pCrntData Size = %d\n", sizeof(INT32)*MIBD_Max_kmJmJobIndex, 2, 3, 4, 5, 6 );
	}
	static INT32 i32pNewData[MIBD_Max_kmJmJobIndex];
	if( b_MibStaticTestChecked )
	{
		SYSF_logMsg("i32pNewData Size = %d\n", sizeof(INT32)*MIBD_Max_kmJmJobIndex, 2, 3, 4, 5, 6 );
	}

	// データクリア 
	_vProcRspCrntDataClear( i32pCrntData, i32pNewData, MIBD_Max_kmJmJobIndex );

	// 現在のデータの取得 
	_vProcRspCrntDataJobGetCrnt( i32pCrntData );

	// ジョブデータテーブルの領域確保 
	static MIBS_JobDataTbl spJobDataTblActive[MIBD_Max_ActiveJobListAll];
	if( b_MibStaticTestChecked )
	{
		SYSF_logMsg("spJobDataTblActive Size = %d\n", sizeof(MIBS_JobDataTbl)*MIBD_Max_ActiveJobListAll, 2, 3, 4, 5, 6 );
	}
	static MIBS_JobDataTbl spJobDataTblLog[MIBD_Max_JobLogListAll];
	if( b_MibStaticTestChecked )
	{
		SYSF_logMsg("spJobDataTblLog Size = %d\n", sizeof(MIBS_JobDataTbl)*MIBD_Max_JobLogListAll, 2, 3, 4, 5, 6 );
	}
    // Aborted、Cancelジョブのための一時情報格納領域確保 
	static MIBS_JobDataTblStock spJobDataTblStock[MIBD_Max_JobLogListAll];
	if( b_MibStaticTestChecked )
	{
		SYSF_logMsg("spJobDataTblStock Size = %d\n", sizeof(MIBS_JobDataTblStock)*MIBD_Max_JobLogListAll, 2, 3, 4, 5, 6 );
	}

	// 確保した領域を初期化する 
	memset( spJobDataTblActive, 0, sizeof(MIBS_JobDataTbl)*MIBD_Max_ActiveJobListAll );
	memset( spJobDataTblLog, 0, sizeof(MIBS_JobDataTbl)*MIBD_Max_JobLogListAll );
    memset( spJobDataTblStock, 0, sizeof(MIBS_JobDataTblStock)*MIBD_Max_JobLogListAll );
	for ( short i = 0; i < MIBD_Max_ActiveJobListAll; i++ ) {
		spJobDataTblActive[i].i32JmJobIndex = MIBD_UndefJobID;
	}
	for ( i = 0; i < MIBD_Max_JobLogListAll; i++ ) {
		spJobDataTblLog[i].i32JmJobIndex = MIBD_UndefJobID;
	}
    for ( i = 0; i < MIBD_Max_JobLogListAll; i++ ) {
        spJobDataTblStock[i].i32JmJobIndex = MIBD_UndefJobID;
    }

	// アクティブジョブ情報の取得 
	short ss_ActiveJobNum = getActiveJobData( spJobDataTblActive );
	_m_irAllVIDMan.vSetAllJobData( MIBE_JobList_Active, spJobDataTblActive );

	// 領域を初期化する 
	memset( spWSDJobDataTblActive, 0, sizeof( t_sWSDPrinterJobs )*MIBD_Max_ActiveJobListAll );
	// WSD Printer Job Status の取得.
	// getWSDPrinterJobDataのパラメータに確保しているジョブ領域の最大数を追加する.
	ssWSDActiveJobNumNew = getWSDPrinterJobData( spWSDJobDataTblActive, spJobDataTblActive, MIBD_Max_ActiveJobListAll );



	//// WSD Print ジョブ終了判断(MIORPT_PrintComplete通知処理) 
	// ジョブIDリスト(New)初期化 
	// Minerva/Zeus 静的解析 2011/02/21 y.ito start.
	for( i = 0; i < MIBD_Max_ActiveJobListAll; i++ )
	{
		i32WSDJobActiveJobIDNew[i] = MIBD_UndefJobID;
	}
	// Minerva/Zeus 静的解析 2011/02/21 y.ito end.
	
	// ジョブIDリスト(New)更新 
	for( i = 0; i < ssWSDActiveJobNumNew; i++ )
	{
		i32WSDJobActiveJobIDNew[i] = spWSDJobDataTblActive[i].sJobSummary.sStatus.nJobId;
	}
	// OldのジョブIDが、Newに存在しないこと(ジョブ終了)を検出 
	short ssOld = 0;
	short ssNew = 0;
	short ssWork = 0, ssTransferJob = 0, ssAddJob = 0;
	Bool bExistFlg = False;
	// 【AR1037899】FQ-OPSD-1700411-OFP212 市場要望対応 HCL-Ramkisson 2018/01/30 Start.
	Bool bJobIdNetPrintRx = False;	
	// 【AR1037899】FQ-OPSD-1700411-OFP212 市場要望対応 HCL-Ramkisson 2018/01/30 End.
	// 実行中Jobのパラメータを確認する.
	for( ssWork = 0; ssWork < ssWSDActiveJobNumNew; ssWork++ )
	{
		// 実行中Jobが転送FaxJob(Pc-FAX送信)か確認する.
		if( spJobDataTblActive[ssWork].i8JmJobTxMode == TYPE_TxModePcFax )
		{
			// 転送FaxJob比較.
			Bool EndProcessFlg = True;	// 新しく生成されたPC-FAXジョブか確認するためのフラグ.
			for( ssTransferJob = 0; ssTransferJob < ssWSDTransferJobCnt; ssTransferJob++ )
			{
				// 1回前に取得した実行中JobIDと転送FaxJobIDを比較する.
				if( spJobDataTblActive[ssWork].i32JmJobIndex == s_TransferJobTbl[ssTransferJob].i32JmJobTransferID )
				{
					// IDが一致していたら、フラグをたてfor文から抜ける.
					EndProcessFlg = False;
					break;
				}
			}
			// 新しい実行中Jobで転送FaxJobがあったら、リストに追加する.
			if( True == EndProcessFlg )
			{
				s_TransferJobTbl[ssTransferJob].i32JmJobTransferID = spJobDataTblActive[ssWork].i32JmJobIndex;
				ssWSDTransferJobCnt++;
			}
		}
		else
		{
			for( ssTransferJob = 0; ssTransferJob < ssWSDTransferJobCnt; ssTransferJob++ )
			{
				// PC-FAX送信だったジョブ状態が切り替わった場合、フラグを立てる.
				if( spJobDataTblActive[ssWork].i32JmJobIndex == s_TransferJobTbl[ssTransferJob].i32JmJobTransferID )
				{
					s_TransferJobTbl[ssTransferJob].i8JmJobFlg = MIBD_TransferJobflgOn;
				}
			}
		}
	}
	
	// 転送FaxJobリストからフラグがOnのものを初期化する.
	for( ssTransferJob = 0; ssTransferJob < ssWSDTransferJobCnt; ssTransferJob++ )
	{
		if( s_TransferJobTbl[ssTransferJob].i8JmJobFlg == MIBD_TransferJobflgOn )
		{
			// IDを削除する.
			s_TransferJobTbl[ssTransferJob].i32JmJobTransferID = MIBD_UndefJobID;
			s_TransferJobTbl[ssTransferJob].i8JmJobFlg = MIBD_TransferJobflgOff;
		}
	}
	
	// 転送FaxJobが1秒以内に処理されるか、キャンセル及び破棄された場合に.
	// 転送FaxJobリストの情報を初期化する.
	short ss_DoneJobNum = getDoneJobData( spJobDataTblLog );
	for( ssTransferJob = 0; ssTransferJob < ssWSDTransferJobCnt; ssTransferJob++ )
	{
		for( ssWork = 0; ssWork < ss_DoneJobNum; ssWork++ )
		{
			if( spJobDataTblLog[ssWork].i32JmJobIndex == s_TransferJobTbl[ssTransferJob].i32JmJobTransferID
			 && ( spJobDataTblLog[ssWork].i8JmJobStatus == 32		// 32: aborted.
				|| spJobDataTblLog[ssWork].i8JmJobStatus == 33		// 33: canceled.
				|| spJobDataTblLog[ssWork].i8JmJobStatus == 30 ) )	// 30: completed.
			{
				s_TransferJobTbl[ssTransferJob].i32JmJobTransferID = MIBD_UndefJobID;
				s_TransferJobTbl[ssTransferJob].i8JmJobFlg = MIBD_TransferJobflgOff;
			}
		}
	}
	
	// 初期化されたもの以外をローカルの転送FaxJobリストに抽出しコピーする.
	if( ssWSDTransferJobCnt > 0 )
	{
		// ローカルの転送FaxJobリストとカウンタを作成.
		MIBS_JobDataTblTransferStock s_TransferCovJobTbl[MIBD_Max_ActiveJobListAll];
		short ssCov = 0, ssWSDTransJobCntLocal = 0;
		
		// ローカルの転送FaxJobリスト領域を確保する.
		memset( s_TransferCovJobTbl, 0, sizeof( MIBS_JobDataTblTransferStock )*MIBD_Max_ActiveJobListAll );
		
		for( i = 0; i < MIBD_Max_ActiveJobListAll; i++ )
		{
			// 確保したローカルの転送FaxJobリスト領域を初期化する.
			s_TransferCovJobTbl[i].i32JmJobTransferID = MIBD_UndefJobID;
			s_TransferCovJobTbl[i].i8JmJobFlg = MIBD_TransferJobflgOff;
		}
		
		for( ssCov = 0; ssCov < ssWSDTransferJobCnt; ssCov++ )
		{
			// IDが未定義以外のものをローカルリストに格納し、カウントする.
			if( s_TransferJobTbl[ssCov].i32JmJobTransferID != MIBD_UndefJobID )
			{
				s_TransferCovJobTbl[ssWSDTransJobCntLocal].i32JmJobTransferID = s_TransferJobTbl[ssCov].i32JmJobTransferID;
				s_TransferCovJobTbl[ssWSDTransJobCntLocal].i8JmJobFlg = s_TransferJobTbl[ssCov].i8JmJobFlg;
				ssWSDTransJobCntLocal++;
			}
		}
		
		// ローカルの転送FaxJobリストに抽出したリストをメインの転送FaxJobリストにコピーする.
		ssWSDTransferJobCnt = ssWSDTransJobCntLocal;
		memcpy( s_TransferJobTbl, s_TransferCovJobTbl, sizeof( MIBS_JobDataTblTransferStock )*MIBD_Max_ActiveJobListAll );
		
	}

	for( ssOld = 0; ssOld < ssWSDActiveJobNumOld; ssOld++ )
	{
		bExistFlg = False;
		
		for( ssNew = 0; ssNew < ssWSDActiveJobNumNew; ssNew++ )
		{
			if( i32WSDJobActiveJobIDOld[ssOld] == i32WSDJobActiveJobIDNew[ssNew] )
			{
				bExistFlg = True;
				break;
			}
		}

		if(bExistFlg == False)
		{
			for( ssWork = 0; ssWork < ssWSDTransferJobCnt; ssWork++ )
			{
				// 一つ前の実行中Jobで転送FaxJobとIDが一致しているか確認.
				if( s_TransferJobTbl[ssWork].i32JmJobTransferID == i32WSDJobActiveJobIDOld[ssOld] )
				{
					// 転送ジョブは処理が続くため、Delete通知を流さない.
					bExistFlg = True;
					i32WSDJobActiveJobIDNew[ssWSDActiveJobNumNew + ssAddJob] = i32WSDJobActiveJobIDOld[ssOld];
					ssAddJob++;
					break;
				}
			}
		}

		// ジョブ終了の場合、MIORPT_PrintCompleteをMIOに通知 
		if( bExistFlg == False )
		{
			// データ領域の確保 
			register INT32* i32p_nJobId = NULL;
			for( ; ; )
			{
				i32p_nJobId = (INT32*)malloc( sizeof(INT32) );
				if( i32p_nJobId NE NULL ) { break; }
				register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;	// ｍｓ→ティック数に変換(1000ms)
				if( sl_Tick == 0L ) { sl_Tick = 1L; }
				++sl_Tick;
				SYSF_taskDelay( sl_Tick );
			}
			// 確保した領域にJobIDのパラメータを入れる 
			*i32p_nJobId = i32WSDJobActiveJobIDOld[ssOld];
			// MIOへの応答メッセージの設定 
			try{
			// 【AR1037899】FQ-OPSD-1700411-OFP212 市場要望対応　HCL-Ramkisson 2018/01/30 Modified Start.
			bJobIdNetPrintRx = NETG_FixedValueDefine_getJobIdNotifyFromNetRx( i32WSDJobActiveJobIDOld[ssOld]);
			if( True == bJobIdNetPrintRx)
			{	

				_m_irMIOMsgMan.eMsgTrans( MIORPT_PrintComplete, i32p_nJobId );
				if(b_MibNicfumSentCheck)
				{
				SYSF_logMsg("\n [FQ-OPSD-1700411-OFP212 Log] bJobIdNetPrintRx is True, First condition TRUE, Sending MIORPT_PrintComplete");
				}
			}
			//	[AR : 1026431] 【 Eagle 】【静的解析対応用課題】 2018/03/15 [HCL_Ramkisson] Mod Start.
			else
			{
			   if(i32p_nJobId != NULL)
			   {
			     free(i32p_nJobId);
				 i32p_nJobId = NULL;
			   }
			}
			//	[AR : 1026431] 【 Eagle 】【静的解析対応用課題】 2018/03/15 [HCL_Ramkisson] Mod End.
			} catch (const std::bad_alloc& e) {
			b_NVDG_InitModule = SetMibDetailCode(14,MIBD_NewDetailCode_MemoryAccessFailureLoc14);

			if( False == b_NVDG_InitModule) {
				SYSF_logMsg("[MIB] Error Code recording Failed Loc14!!\n", 0,0,0,0,0,0);	
			}
    	}
        try{
			register INT32* nMatchingJobID = NULL;
			for( ; ; )
			{
				nMatchingJobID = (INT32*)malloc( sizeof(INT32) );
				if( nMatchingJobID NE NULL ) { break; }
				register long sl_Tick = (long)SYSF_sysClkRateGet() * 1000 / 1000;	// ｍｓ→ティック数に変換(1000ms)
				if( sl_Tick == 0L ) { sl_Tick = 1L; }
				++sl_Tick;
				SYSF_taskDelay( sl_Tick );
			}
		    
			*nMatchingJobID = getMatchingJOBID();
			if( MIBD_UndefNetRXJobID != *nMatchingJobID )
			{
				_m_irMIOMsgMan.eMsgTrans( MIORPT_PrintComplete, nMatchingJobID );
				if(b_MibNicfumSentCheck)
				{
				SYSF_logMsg("\n [FQ-OPSD-1700411-OFP212 Log]  Second condition TRUE, Sending MIORPT_PrintComplete");
				}
			}
			//	[AR : 1026431] 【 Eagle 】【静的解析対応用課題】 2018/03/15 [HCL_Ramkisson] Mod Start.
			else
			{
				if(nMatchingJobID  != NULL)
			  	{
			  		free(nMatchingJobID);
					nMatchingJobID = NULL;
	        	}
			} 
			} catch (const std::bad_alloc& e) {
			b_NVDG_InitModule = SetMibDetailCode(15,MIBD_NewDetailCode_MemoryAccessFailureLoc15);

			if( False == b_NVDG_InitModule) {
				SYSF_logMsg("[MIB] Error Code recording Failed Loc15!!\n", 0,0,0,0,0,0);	
			}
		}	//	[AR : 1026431] 【 Eagle 】【静的解析対応用課題】 2018/03/15 [HCL_Ramkisson] Mod End.			
			
            // 【AR1037899】FQ-OPSD-1700411-OFP212 市場要望対応 HCL-Ramkisson 2018/01/30 End.
#if 0 /* for DEBUG */
logMsg("[MIB] MIORPT_PrintComplete, JobID=%d\n", *i32p_nJobId, 2, 3, 4, 5, 6);
#endif

		}
	}
	ssWSDActiveJobNumOld = (short)(ssWSDActiveJobNumNew + ssAddJob);
	// Old <- New 
	memcpy( i32WSDJobActiveJobIDOld, i32WSDJobActiveJobIDNew, sizeof( INT32 )*MIBD_Max_ActiveJobListAll );

	// 履歴ジョブ情報の取得 
	short ss_LogJobNum = getDoneJobData( spJobDataTblLog );

	// 領域を初期化する 
	memset( spWSDJobDataTblLog, 0, sizeof( t_sWSDPrinterJobs )*MIBD_Max_JobLogListAll );
	// WSD Printer Job Status の取得.
	// getWSDPrinterJobDataのパラメータに確保しているジョブ領域の最大数を追加する.
	getWSDPrinterJobData( spWSDJobDataTblLog, spJobDataTblLog, MIBD_Max_JobLogListAll );

	// 取得した履歴ジョブ情報から、kmJmJobStatusが"32(aborted)"、"33(canceled)"であるジョブを検索し、
	// 一致したジョブの一部情報を確保した領域に保存する 
    int k = 0;    // 一時格納情報個数 
    for ( i = 0; i < ss_LogJobNum; i++ ) {
        if ( spJobDataTblLog[i].i8JmJobStatus == 32          // 32: aborted 
            || spJobDataTblLog[i].i8JmJobStatus == 33 ) {    // 33: canceled 
            spJobDataTblStock[k].i32JmJobIndex = spJobDataTblLog[i].i32JmJobIndex;                           // kmJmJobIndexの保存 
            spJobDataTblStock[k].i8JmJobStatus = spJobDataTblLog[i].i8JmJobStatus;                           // kmJmJobStatusの保存 
            strcpy( spJobDataTblStock[k].i8JmJobStatusDetails, spJobDataTblLog[i].i8JmJobStatusDetails );    // kmJmJobStatusDetailsの保存 
            k++;
        }
    }
    // 一時情報格納領域で保存したkmJmJobIndexと履歴ジョブのkmJmJobIndexを比較し、
    // 一致した履歴ジョブ情報を一時情報格納領域の情報で上書き設定する 
    if ( k != 0 ) {    // 一時情報格納領域に情報が存在する 
        for ( i = 0; i < ss_LogJobNum; i++ ) {
            for ( int j = 0; j < k; j++ ) {
                if ( spJobDataTblStock[j].i32JmJobIndex == MIBD_UndefJobID ) {
                    continue;
                }
                if ( spJobDataTblStock[j].i32JmJobIndex == spJobDataTblLog[i].i32JmJobIndex ) {    // JobIndexが一致 
                    spJobDataTblStock[j].i32JmJobIndex = MIBD_UndefJobID;                                             // kmJmJobIndexの上書き設定 
                    spJobDataTblLog[i].i8JmJobStatus = spJobDataTblStock[j].i8JmJobStatus;                            // kmJmJobStatusの上書き設定 
                    strcpy( spJobDataTblLog[i].i8JmJobStatusDetails, spJobDataTblStock[j].i8JmJobStatusDetails );     // kmJmJobStatusDetailsの上書き設定 
                    break;
                }
            }
        }
    }
	_m_irAllVIDMan.vSetAllJobData( MIBE_JobList_Log, spJobDataTblLog );

	// JobIDを新しいデータ領域に設定 
	for ( i = 0; i < ss_ActiveJobNum; i++ ) {
		i32pNewData[i] = spJobDataTblActive[i].i32JmJobIndex;
	}
	for ( i = 0; i < ss_LogJobNum; i++ ) {
		// DonauBK CQ327929 kubota 2010/7/2 START.
		// コーディング上問題があるため、配列内の処理に()を追加.
		i32pNewData[(ss_ActiveJobNum+i)] = spJobDataTblLog[i].i32JmJobIndex;
		// DonauBK CQ327929 kubota 2010/7/2 END.
	}


	// データチェック 
	_vProcRspCrntDataComp( i32pCrntData, i32pNewData, MIBD_Max_kmJmJobIndex );

	// MIOへの通知 
	_vProcRspCrntDataJobMIO( MIORPT_JobIndexRemove, i32pCrntData, MIBD_Max_kmJmJobIndex );
	_vProcRspCrntDataJobMIO( MIORPT_JobIndexEntry,  i32pNewData, MIBD_Max_kmJmJobIndex );

	b_MibStaticTestChecked = False;

	// 領域を初期化する 
	memset( spWSDPrinterStatusSummaryNew, 0, sizeof( t_sWSDPrinterStatusSummary ) );
	// WSD Printer Status の取得 
	getWSDPrinterStatus( spWSDPrinterStatusSummaryNew );
	// WSD Printer Status の比較 
// [静的解析対応] 構造体の比較を"memcmp"で行うのは好ましくないため修正	2007.04.25	kawabe
	Bool isStatusChaneged = False;
	if ( (spWSDPrinterStatusSummaryOld[0].eState != spWSDPrinterStatusSummaryNew[0].eState )
		|| (spWSDPrinterStatusSummaryOld[0].ePrimaryStateReason != spWSDPrinterStatusSummaryNew[0].ePrimaryStateReason ) ) {
		isStatusChaneged = True;
	} else {
		for( int i = 0; i < MAX_IPP_PRINTER_STATE_REASONS; i ++ ) {
			if (spWSDPrinterStatusSummaryOld[0].eStateReasons[i] != spWSDPrinterStatusSummaryNew[0].eStateReasons[i] ) {
				isStatusChaneged = True;
				break;
			}
		}
	}
	if ( isStatusChaneged )
	{
		// 内容に違いがあった場合、MIORPT_WSDPrinterStatusChangedをMIOに通知 
		// データ領域の確保 
		register t_sWSDPrinterStatusSummary* spWSDPrinterStatusSummary = NULL;
		for( ; ; )
		{
			spWSDPrinterStatusSummary = (t_sWSDPrinterStatusSummary*)malloc( sizeof(t_sWSDPrinterStatusSummary) );
			if( spWSDPrinterStatusSummary NE NULL ) { break; }
			register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;	// ｍｓ→ティック数に変換(1000ms)
			if( sl_Tick == 0L ) { sl_Tick = 1L; }
			++sl_Tick;
			SYSF_taskDelay( sl_Tick );
		}
		// 確保した領域にWSD Printer Status (New) を入れる 
		memcpy( spWSDPrinterStatusSummary, spWSDPrinterStatusSummaryNew, sizeof( t_sWSDPrinterStatusSummary )  );
		// MIOへの応答メッセージの設定 
		_m_irMIOMsgMan.eMsgTrans( MIORPT_WSDPrinterStatusChanged, spWSDPrinterStatusSummary );



	}
	// Old <- New 
	memcpy( spWSDPrinterStatusSummaryOld, spWSDPrinterStatusSummaryNew, sizeof( t_sWSDPrinterStatusSummary )  );

}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_vProcRspCrntDataClear 
// ABSTRACT	: 動的に変わるジョブ情報のデータクリア 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 無し 
// CREATE	: 01-06-27	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_FuncManMIB::_vProcRspCrntDataClear(
	INT32*				_r_i32pCrntData,		// 現在のデータ 
	INT32*				_r_i32pNewData,			// 新しいデータ 
	CINT16				_r_ci16MaxDataSize		// 最大データサイズ 
)
{
	for ( register INT16 i = 0; i < _r_ci16MaxDataSize; i++ ) {
		_r_i32pCrntData[ i ] = _r_i32pNewData[ i ] = MIBD_UndefJobID;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_vProcRspCrntDataJobGetCrnt 
// ABSTRACT	: ジョブ情報の現在のデータの取得 
// FUNCTION	: 引数にジョブ情報の現在のデータを設定して返す 
// NOTE		: 
// RETURN	: 無し 
// CREATE	: 01-06-27	廣田 
//			  新規作成 
//			  01-12-14  山畑 
//			  JobIndexがVIDD_kmJmJobIndex_○に変更したのに対応 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_FuncManMIB::_vProcRspCrntDataJobGetCrnt(
	INT32*				_r_i32pCrntData			// 現在のデータ 
)
{
	INT32 i32JobID = MIBD_UndefJobID;
	INT16 i16Index = 0;
	for ( register INT16 i16Index2 = 0; i16Index2 < MIBD_Max_kmJmJobIndex; i16Index2++ ) {
		_m_irAllVIDMan.vGetJobData( (INT32)i16Index2, VIDD_kmJmJobIndex_1, &i32JobID );//JobStatus
		if ( i32JobID != MIBD_UndefJobID ) {
			_r_i32pCrntData[i16Index] = i32JobID;
			i16Index++;
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_vProcRspCrntDataComp 
// ABSTRACT	: 動的に変わるジョブ情報のデータ変化チェック 
// FUNCTION	: 新しいデータと現在のデータを比較し、一致したものにはMIBD_UndefJobIDを設定していく 
//			  その結果新しいデータに残っているものが、新規追加分 
//			  現在のデータにのこっているものが、削除分となる 
// NOTE		: 引数の値がMIBD_UndefJobIDとなるところまで、データは入っている 
// RETURN	: 無し 
// CREATE	: 01-06-27	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_FuncManMIB::_vProcRspCrntDataComp(
	INT32*				_r_i32pCrntData,		// 現在のデータ 
	INT32*				_r_i32pNewData,			// 新しいデータ 
	CINT16				_r_ci16MaxDataSize		// 最大データサイズ 
)
{
	for ( register INT16 i = 0; i < _r_ci16MaxDataSize; i++ ) {
		if ( _r_i32pCrntData[ i ] EQ MIBD_UndefJobID ) {
			break;
		}
		for ( register INT16 j = 0; j < _r_ci16MaxDataSize; j++ ) {
			if ( _r_i32pNewData[ j ] EQ MIBD_UndefJobID ) {
				continue;
			}
			if ( _r_i32pNewData[ j ] EQ _r_i32pCrntData[ i ] ) {
				_r_i32pCrntData[ i ] = _r_i32pNewData[ j ] = MIBD_UndefJobID;
				break;
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_vProcRspCrntDataJobMIO 
// ABSTRACT	: MIOへのジョブの情報の通知 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 無し 
// CREATE	: 01-06-26	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_FuncManMIB::_vProcRspCrntDataJobMIO(
	const APP_MSG_ID	_r_ceMsgID,				// MIOへの送信メッセージID 
	const INT32*		_r_ci32pData,			// データ 
	CINT16				_r_ci16MaxDataSize		// 最大データサイズ 
)
{
	for ( register INT16 i = 0; i < _r_ci16MaxDataSize; i++ ) {
		if ( _r_ci32pData[ i ] NE MIBD_UndefJobID ) {
			register INT32* i32pIndex = NULL;
			for ( ;; ) {
				i32pIndex = (INT32*)malloc( 2*sizeof(INT32) );
				if ( i32pIndex NE NULL ) {
					break;
				}
				register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;	// ｍｓ→ティック数に変換(1000ms)
				if( sl_Tick == 0L ) { sl_Tick = 1L; }
				++sl_Tick;
				SYSF_taskDelay( sl_Tick );

			}
			i32pIndex[0] = 1;					// index1は１固定 
			i32pIndex[1] = _r_ci32pData[ i ];

			// MIOへの応答メッセージの設定 
			_m_irMIOMsgMan.eMsgTrans( _r_ceMsgID, i32pIndex );
		}
	}
}
/* 2008.08.18 UV-00536 sw_tnaka add S */
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_vProcRspCrntDataJobMonitoringMIO 
// ABSTRACT	: MIOへのジョブの情報の通知 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 無し.
// CREATE	: 
//			  新規作成.
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_FuncManMIB::_vProcRspCrntDataJobMonitoringMIO(
	const APP_MSG_ID	_r_ceMsgID,				// MIOへの送信メッセージID 
	const INT32*		_r_ci32pData,			// データ 
	CINT16				_r_ci16MaxDataSize		// 最大データサイズ 
)
{
	for ( register INT16 i = 0; i < _r_ci16MaxDataSize; i++ ) {
		if ( _r_ci32pData[ i ] NE MIBD_UndefJobID ) {
			register INT32* i32pIndex = NULL;
			for ( ;; ) {
				i32pIndex = (INT32*)malloc( 2*sizeof(INT32) );
				if ( i32pIndex NE NULL ) {
					break;
				}
				register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;	// ｍｓ→ティック数に変換(1000ms)
				if( sl_Tick == 0L ) { sl_Tick = 1L; }
				++sl_Tick;
				SYSF_taskDelay( sl_Tick );

			}
			i32pIndex[0] = 1;					// index1は１固定 
			i32pIndex[1] = _r_ci32pData[ i ];
			
			// MIOへの応答メッセージの設定 
			_m_irMIOMsgMan.eMsgTrans( _r_ceMsgID, i32pIndex );
		}
	}
}
	
	
/* 2008.08.18 UV-00536 sw_tnaka add E */
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_isDataChanged
// ABSTRACT	: データ変更のチェック
// FUNCTION	: 
// NOTE		: 無し
// RETURN	: Bool _isDataChanged : True : データが変更
// CREATE	: 01-06-20	廣田
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
Bool MIBC_FuncManMIB::_isDataChanged(
	void*					_r_vpIJCData,			// IJCデータ 
	CUINT32					_r_cui32IJCDataSize,	// IJCデータのサイズ 
	void*					_r_vpVIDData,			// VIDデータ 
	CUINT32					_r_cui32VIDDataSize		// VIDデータのサイズ 
)
{
	Bool isDataChanged = False;
	if( _r_cui32VIDDataSize EQ _r_cui32IJCDataSize ) {
		UINT8* ui8pIJCData = (UINT8*)_r_vpIJCData;
		UINT8* ui8pVIDData = (UINT8*)_r_vpVIDData;
		for ( UINT32 i = 0; i < _r_cui32VIDDataSize; i++ ) {
			if( ui8pVIDData[i] NE ui8pIJCData[i] ) {
				isDataChanged = True;
				break;
			}
		}
	} else {
		isDataChanged = True;
	}
	return isDataChanged;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_ui32GetIJCData
// ABSTRACT	: IJCデータの取得 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 無し
// CREATE	: 02-05-20	河辺
//			  新規作成
//			  02-06-10	河辺
//			  nmIJCIF.cppで全VIDデータの処理を行うように変更 
/////////////////////////////////////////////////////////////////////////////////////////
UINT32 MIBC_FuncManMIB::_ui32GetIJCData(
	const VID			_r_ceVID,					// VID
	void* const			_r_vpcGetData,				// 取得するデータへのポインタ
	UINT32				_r_ui32MaxSize
)
{
	return in_ui32ProcGetAllVIDData( _r_ceVID, _r_vpcGetData, _r_ui32MaxSize );
}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::attachObserver 
// ABSTRACT	: オブザーバー登録処理 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 
// CREATE	: 05-09-27	今村 
//			  新規作成
//			　07-01-12	河辺 
//			　InterpreterObserverオブザーバー登録追加 
/////////////////////////////////////////////////////////////////////////////////////////
MIBC_PswcMibSettingObserver c_PswcMibSettingObserver;		// PswcMibSettingオブザーバー 
MIBC_SnmpTrapSettingObserver c_SnmpTrapSettingObserver;		// SnmpTrapSettingオブザーバー 
MIBC_InterpreterObserver	c_InterpreterObserver;			// InterpreterObserverオブザーバー 
MIBC_InitSuperWarpObserver	c_InitSuperWarpObserver;		// SuperWarpObserverオブザーバー 	// IT5_4.1 RQ-643/KRQ-290 HCL-Sankar 2016/12/17 Added.


// 【AR1037899】FQ-OPSD-1700411-OFP212 市場要望対応 HCL-Ramkisson 2018/01/30 Start.
MIBC_ActiveJobListObserver pc_ActiveJobListObserver;
// 【AR1037899】FQ-OPSD-1700411-OFP212 市場要望対応 HCL-Ramkisson 2018/01/30 End.

void MIBC_FuncManMIB::attachObserver()
{
	// PswcMibSettingオブザーバー登録 
	APIC_PswcMibSetting* pc_PswcMibSetting = APIC_PswcMibSetting::newInstance();
	pc_PswcMibSetting->lock();
	pc_PswcMibSetting->attachObserver(&c_PswcMibSettingObserver);
	pc_PswcMibSetting->unlock();

	// SnmpTrapSettingオブザーバー登録 
	APIC_SnmpTrapSetting* pc_SnmpTrapSetting = APIC_SnmpTrapSetting::newInstance();
	pc_SnmpTrapSetting->lock();
	pc_SnmpTrapSetting->attachObserver(&c_SnmpTrapSettingObserver);
	pc_SnmpTrapSetting->unlock();

	// InterpreterObserverオブザーバー登録 
	APIC_Interpreter* pc_Interpreter = APIC_Interpreter::newInstance();
	pc_Interpreter->lock();
	pc_Interpreter->attachObserver(&c_InterpreterObserver);
	pc_Interpreter->unlock();
	
	
	// IT5_4.1 RQ-643/KRQ-290 HCL-Sankar 2016/12/17 Added Start.
	// SuperWarpObserverオブザーバー 
	APIC_SuperWarpOperation* pc_sprwrpOp = APIC_SuperWarpOperation::newInstanceAdmin();
	if( NULL != pc_sprwrpOp ){
		pc_sprwrpOp->attachObserver( 12, &c_InitSuperWarpObserver );
	}
	// IT5_4.1 RQ-643/KRQ-290 HCL-Sankar 2016/12/17 Added End.
	
	// 【AR1037899】FQ-OPSD-1700411-OFP212 市場要望対応 HCL-Ramkisson 2018/01/30 Start.
	APIC_ActiveJobList* pc_activeJobList = APIC_ActiveJobList::newInstance();
	pc_activeJobList->lock();
	pc_activeJobList->attachObserver(&pc_ActiveJobListObserver);
	pc_activeJobList->unlock();
// 【AR1037899】FQ-OPSD-1700411-OFP212 市場要望対応 HCL-Ramkisson 2018/01/30 End.
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::notifySnmpFailCounterClear 
// ABSTRACT	: SNMP認証失敗カウンタクリア通知 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 
// CREATE	: 05-09-27	今村    新規作成
//            06-01-27  岩本裕  DB No.52126対策 MIO側にカウンタクリアのパラメータを渡す処理を修正する 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_FuncManMIB::notifySnmpFailCounterClear()
{
	// データ領域の確保 
	register t_eMioSNMPAuthUser* pe_MioSNMPAuthError = NULL;
	for ( ;; ) {
		pe_MioSNMPAuthError = (t_eMioSNMPAuthUser*)malloc(sizeof(t_eMioSNMPAuthUser));
		if ( pe_MioSNMPAuthError NE NULL ) {
			break;
		}
		register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;	// ｍｓ→ティック数に変換(1000ms)
		if( sl_Tick == 0L ) { sl_Tick = 1L; }
		++sl_Tick;
		SYSF_taskDelay( sl_Tick );
	}

	// 確保した領域にRead/Write権限のロックを解除するパラメータを入れる 
	*pe_MioSNMPAuthError = MIO_SNMP_AUTH_BOTH;

	// MIOへの応答メッセージの設定 
	_m_irMIOMsgMan.eMsgTrans( MIORPT_SNMPAuthUnLock, pe_MioSNMPAuthError );
}
// ユビキタス[KRQ-278][スプール分散恒久対応][step2] start.
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::notifySnmpFailCounterClear 
// ABSTRACT	: ポート負荷状態の更新 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 
// CREATE	: 16-09-30	ユビキタスFUM 新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_FuncManMIB::_vPortDataGet()
{
	try{
	if( TYPD_UbqDeviceNumberExpansionDisable == APIC_UbiquitousSetting::newInstance()->getDeviceNumberExpansion() ) {
		return;
	}

	_m_irAllVIDMan.updatePortLoadStatus();
	} catch (const std::bad_alloc& e) {
			b_NVDG_InitModule = SetMibDetailCode(12,MIBD_NewDetailCode_MemoryAccessFailureLoc12);

			if( False == b_NVDG_InitModule) {
				SYSF_logMsg("[MIB] Error Code recording Failed Loc12!!\n", 0,0,0,0,0,0);	
			}
    	}
}
// ユビキタス[KRQ-278][スプール分散恒久対応][step2] end.

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_PswcMibSettingObserver::dataChanged 
// ABSTRACT	: データ変化通知 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 
// CREATE	: 05-09-27	今村 
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_PswcMibSettingObserver::dataChanged()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_PswcMibSettingObserver::notifySnmpFailCounterClear 
// ABSTRACT	: SNMP認証失敗カウンタクリア通知 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 
// CREATE	: 05-09-27	今村 
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_PswcMibSettingObserver::notifySnmpFailCounterClear()
{
	// MIOへの応答メッセージの設定 
	c_iFuncManMIB.notifySnmpFailCounterClear();

}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_SnmpTrapSettingObserver::dataChanged 
// ABSTRACT	: データ変化通知 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 
// CREATE	: 05-09-27	今村 
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_SnmpTrapSettingObserver::dataChanged()
{
}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_SnmpTrapSettingObserver::notifySnmpTrap 
// ABSTRACT	: SNMP TRAP通知 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 
// CREATE	: 05-09-27	今村 
//				新規作成
//			  06-08-03	今村 
//				[BugNo.64399] SNMP TRAPが通知されない問題を修正 
/////////////////////////////////////////////////////////////////////////////////////////
unsigned int ui_SnmpTrapDbgFlg = 0;

extern "C"
{
	void mibSetTraceSnmpTrap( unsigned int ui_Level );
}

void mibSetTraceSnmpTrap( unsigned int ui_Level )
{
    ui_SnmpTrapDbgFlg = ui_Level;
}

// MinervaZeus CQ348375 kubota 2011/6/16 START.
// OIDDigitは、sysObjectIDに含まれる数値データの個数を定義している.
// Minerva用にDefine17が定義されていたが、sysObectIDの数値データの個数は.
// 15から変わっていないため、MinervaのDefineを削除する.
// MinervaZeus No913_SuppliesInformation Shalini 2011/04/21 START.
#define MIBD_Max_OIDDigit	15
// MinervaZeus No913_SuppliesInformation Shalini 2011/04/21 END.

static uint32_t sendOID[ MIBD_Max_OIDDigit ];	// sysObjectIDの数値データを保存する変数.
static Bool b_isGetOID = False;						// True:OID情報を取得済み, False:取得未.
// MinervaZeus CQ348375 kubota 2011/6/16 END.

static const struct SCnvTbl {
	TYPE_SnmpTrapEvent		e_SnmpTrapEvent;
	// MinervaZeus CQ349518 kubota 2011/6/28 START.
	short					ss_UnitColorType;
	// MinervaZeus CQ349518 kubota 2011/6/28 END.
	unsigned int			ui_SpecificType;
	char*					sc_MessageString;
} sCnvTbl[] = {
	// !!!仮設定!!! 
	// ■課題■ [2006/04/21 制御12 今村]
	//   Itemは統合仕様の定義だが、spcific-trapとvariable-bindingsは機種依存設計にしておいたほうが良いかも…? 
	//   NETG_FixedValueDefine.hに、機種毎に定義すれば良いような気がスル→制御11石井さん・関口さんと相談必要 
	{TYPE_SnmpTrapEventColdStart,					0,		0,		(char*)"Cold Start"},
	{TYPE_SnmpTrapEventAuthenticationFailure,		0,		1,		(char*)"Authentication Failure"},
	{TYPE_SnmpTrapEventServiceCall,					0,		2,		(char*)"Service Call"},
	{TYPE_SnmpTrapEventJam,							0,		3,		(char*)"Paper Jam"},
	{TYPE_SnmpTrapEventPaperEmpty,					0,		4,		(char*)"No Paper"},
	{TYPE_SnmpTrapEventTonerEmpty,					1,		5,		(char*)"Toner Empty(C)"},
	{TYPE_SnmpTrapEventTonerEmpty,					2,		5,		(char*)"Toner Empty(M)"},
	{TYPE_SnmpTrapEventTonerEmpty,					3,		5,		(char*)"Toner Empty(Y)"},
	{TYPE_SnmpTrapEventTonerEmpty,					4,		5,		(char*)"Toner Empty(K)"},
	{TYPE_SnmpTrapEventStapleEmpty,					0,		6,		(char*)"Staple Empty"},
	{TYPE_SnmpTrapEventFinisherTrayOver,			0,		7,		(char*)"Paper Remaining OutputTray"},
	{TYPE_SnmpTrapEventPunchDustFull,				0,		8,		(char*)"Punch Dust Full"},
	{TYPE_SnmpTrapEventPMCall,						0,		9,		(char*)"Maintenance Required"},
	{TYPE_SnmpTrapEventEndJob,						0,		10,		(char*)"Job End"},
	{TYPE_SnmpTrapEventSnmpAuthFail,				0,		11,		(char*)"SNMP Authentication Failure"},
	{TYPE_SnmpTrapEventWasteTonerBox,				0,		12,		(char*)"WasteToner Full"},
	{TYPE_SnmpTrapEventImagingUnitLife,				5,		13,		(char*)"Life Limit Imaging Unit(C)"},
	{TYPE_SnmpTrapEventImagingUnitLife,				6,		13,		(char*)"Life Limit Imaging Unit(M)"},
	{TYPE_SnmpTrapEventImagingUnitLife,				7,		13,		(char*)"Life Limit Imaging Unit(Y)"},
	// Poseidon CQ504079 Vaibhav 2014/10/08 START.
	{TYPE_SnmpTrapEventDrumUnitLifeStop,			8,		13,		(char*)"Life Limit Drum(C)"},
	{TYPE_SnmpTrapEventDrumUnitLifeStop,			9,		13,		(char*)"Life Limit Drum(M)"},
	{TYPE_SnmpTrapEventDrumUnitLifeStop,			10,		13,		(char*)"Life Limit Drum(Y)"},
	{TYPE_SnmpTrapEventDrumUnitLifeStop,			11,		13,		(char*)"Life Limit Drum(K)"},
	{TYPE_SnmpTrapEventDevelopUnitLifeStop,			12,		13,		(char*)"Life Limit Developer Unit(C)"},
	{TYPE_SnmpTrapEventDevelopUnitLifeStop,			13,		13,		(char*)"Life Limit Developer Unit(M)"},
	{TYPE_SnmpTrapEventDevelopUnitLifeStop,			14,		13,		(char*)"Life Limit Developer Unit(Y)"},
	{TYPE_SnmpTrapEventDevelopUnitLifeStop,			15,		13,		(char*)"Life Limit Developer Unit(K)"},
	//【VenusMLK】【CQ523804】HCL_Pawan 2016/02/03 Start.
	#ifdef DEF_A4_Cus
	{TYPE_SnmpTrapEventImagingUnitLife,				8,		13,		(char*)"Life Limit Imaging Unit(K)"},
	#else	
	{TYPE_SnmpTrapEventImagingUnitLife,				8,		13,		(char*)"Life Limit Drum(C)"},
	#endif
	//【VenusMLK】【CQ523804】HCL_Pawan 2016/02/03 End.
	{TYPE_SnmpTrapEventImagingUnitLife,				9,		13,		(char*)"Life Limit Drum(M)"},
	{TYPE_SnmpTrapEventImagingUnitLife,				10,		13,		(char*)"Life Limit Drum(Y)"},
	{TYPE_SnmpTrapEventImagingUnitLife,				11,		13,		(char*)"Life Limit Drum(K)"},
	{TYPE_SnmpTrapEventImagingUnitLife,				12,		13,		(char*)"Life Limit Developer Unit(C)"},
	{TYPE_SnmpTrapEventImagingUnitLife,				13,		13,		(char*)"Life Limit Developer Unit(M)"},
	{TYPE_SnmpTrapEventImagingUnitLife,				14,		13,		(char*)"Life Limit Developer Unit(Y)"},
	{TYPE_SnmpTrapEventImagingUnitLife,				15,		13,		(char*)"Life Limit Developer Unit(K)"},
	// Poseidon CQ504079 Vaibhav 2014/10/08 END.
	{TYPE_SnmpTrapEventDefineUnitLife,				0,		14,		(char*)"Life Limit Fuser Unit"},
	{TYPE_SnmpTrapEventCopyBeltUnitLife,			0,		15,		(char*)"Life Limit Transfer Belt"},
	{TYPE_SnmpTrapEventCopyRollerUnitLife,			0,		16,		(char*)"Life Limit Transfer Roller"},
	{TYPE_SnmpTrapEventCleanerOzonFilterLife,		0,		17,		(char*)"Life Limit Other Parts"},
// 2019/03/05 FPT-LongVD2 IT6_2.0 [EagleH] OP_BTS-1392 Modify Start.
	{TYPE_SnmpTrapEventStaplerJunkFull,				0,		22,		(char*)"Staple Dust Full"},
// 2019/03/05 FPT-LongVD2 IT6_2.0 [EagleH] OP_BTS-1392 Modify End.
	#ifdef DEF_A4_Cus	//VenusMLK HCL_Rupa WP 729 / WBS 297 20151222 Start.
	// 2014/09/10:sk_naka:[ZeusS]制御構成変更No.4 HDDミラーリングエラー_S.
	{TYPE_SnmpTrapEventHDDMirroringError,			0,		18,		(char*)"SSD Mirroring Error"}, // 【IT6】【機器構成変更No.100】HCL-Ramkisson 2018/04/12 Modified.
	// 2014/09/10:sk_naka:[ZeusS]制御構成変更No.4 HDDミラーリングエラー_E.
	{TYPE_SnmpTrapEventJobComplete,					0,		19,		(char*)"Job Complete"},
// 2019/01/28 FPT-LucNH2 IT6_2.0 [EagleH] OP_BTS-1392 Modify Start.
//	{TYPE_SnmpTrapEventStaplerJunkFull,				0,		22,		(char*)"Staple Dust Full"},			// 2019/03/05 FPT-LongVD2 IT6_2.0 [EagleH] OP_BTS-1392 Modify.
// 2019/01/28 FPT-LucNH2 IT6_2.0 [EagleH] OP_BTS-1392 Modify End.
//【KronosS】[WBS No.34] HCL_Pawan Modified 2016/09/29 Start.
#ifdef DEF_KRONOSS_Cus
	{TYPE_SnmpTrapEventJobError,					0,		20,		(char*)"Job Error"},
	{TYPE_SnmpTrapEventMaintenanceKitLife,			0,		21,		(char*)"Life Limit Maintenance Kit"}
#else
	{TYPE_SnmpTrapEventJobError,					0,		20,		(char*)"Job Error"},
#endif
//【KronosS】[WBS No.34] HCL_Pawan Modified 2016/09/29 End.
	#else
	// 2014/09/10:sk_naka:[ZeusS]制御構成変更No.4 HDDミラーリングエラー_S.
	{TYPE_SnmpTrapEventHDDMirroringError,			0,		18,		(char*)"SSD Mirroring Error"}, // 【IT6】【機器構成変更No.100】HCL-Ramkisson 2018/04/12 Modified.
	// 2014/09/10:sk_naka:[ZeusS]制御構成変更No.4 HDDミラーリングエラー_E.
	#endif	//VenusMLK HCL_Rupa WP 729 / WBS 297 20151222 End.
	{TYPE_SnmpTrapEventVirusScan,					0,		23,		(char*)"Virus detected"} //2021/03/04 [IT6_2.2]  RQ-817  IT6_2.2-399 HIENG Start.
	// MinervaZeus No913_SuppliesInformation Shalini 2011/04/18 END.
,	{TYPE_SnmpTrapEventPwdAtk,						0,		24,		(char*)"Detection Password Attack"}
,	{TYPE_SnmpTrapEventAuthAtk,						0,		25,		(char*)"Detection Authentication Attack"}
,	{TYPE_SnmpTrapEventPenaltyLock,					0,		26,		(char*)"Authentication Failed - Penalty Lock"}
,	{TYPE_SnmpTrapEventBannedOriginalCopy,			0,		27,		(char*)"Banned Original Copy"}
,	{TYPE_SnmpTrapEventDomainSendError,				0,		28,		(char*)"Domain Send Error"}
,	{TYPE_SnmpTrapEventExternalDevice,				0,		29,		(char*)"Detection External Device"}
,	{TYPE_SnmpTrapEventImport,						0,		30,		(char*)"Import"}
,	{TYPE_SnmpTrapEventExport,						0,		31,		(char*)"Export"}
,	{TYPE_SnmpTrapEventBoxOperation,				0,		32,		(char*)"Box Operation"}
,	{TYPE_SnmpTrapEventServiceLogin,				0,		33,		(char*)"Authentication - Service Login"}
,	{TYPE_SnmpTrapEventPatternFileWarning,			0,		34,		(char*)"Virus Scan Pattern File Update Failure"}
};
void MIBC_SnmpTrapSettingObserver::notifySnmpTrap( TYPE_SnmpTrapEvent e_SnmpTrapEvent, short ss_UnitColorType, const char* pc_displayInfo )
{
	/* DEBUG */
	if( ui_SnmpTrapDbgFlg )
	{
		SYSF_logMsg("\n\n\n+++MIBC_SnmpTrapSettingObserver::notifySnmpTrap CALLED!!+++\n\n\n",1,2,3,4,5,6);
	}

	// SNMP TRAP設定が有効か?
	APIC_SnmpTrapSetting* pc_SnmpTrapSetting = APIC_SnmpTrapSetting::newInstance();
	// 【IT5_4.2】【RQ686】HCL-Ramkisson 2017/10/13 Modify start.
	pc_SnmpTrapSetting->lock();		
	Bool b_EnableSnmpTrap_NW1 = pc_SnmpTrapSetting->getEnableSnmpTrap(NWIF_TYPE_MAIN_LAN);
	Bool b_EnableSnmpTrap_NW2 = pc_SnmpTrapSetting->getEnableSnmpTrap(NWIF_TYPE_EXTEND_LAN);   
	pc_SnmpTrapSetting->unlock();
	
	/* DEBUG */
	if( ui_SnmpTrapDbgFlg )
	{
		if( b_EnableSnmpTrap_NW1)
		{
			SYSF_logMsg("\n +++MIBC_SnmpTrapSettingObserver::notifySnmpTrap b_EnableSnmpTrap_NW1 == TRUE\n",1,2,3,4,5,6);
		}
		else
		{
			SYSF_logMsg("\n +++MIBC_SnmpTrapSettingObserver::notifySnmpTrap b_EnableSnmpTrap_NW1 == FALSE\n",1,2,3,4,5,6);
		}
		
		if( b_EnableSnmpTrap_NW2)
		{
			SYSF_logMsg("\n +++MIBC_SnmpTrapSettingObserver::notifySnmpTrap b_EnableSnmpTrap_NW2 == TRUE\n",1,2,3,4,5,6);
		}
		else
		{
			SYSF_logMsg("\n +++MIBC_SnmpTrapSettingObserver::notifySnmpTrap b_EnableSnmpTrap_NW2 == FALSE\n",1,2,3,4,5,6);
		}
	}
	
	// 【IT5_4.2】【RQ686】HCL-Ramkisson 2017/10/13 Modify END.

	// HDDがバックアップ中(・リストア中)か?
	APIC_SystemStatus* pc_SystemStatus = APIC_SystemStatus::newInstance();
	pc_SystemStatus->lock();
	Bool b_DoBackup = pc_SystemStatus->getDoBackup();
	pc_SystemStatus->unlock();

	/* DEBUG */
	if( ui_SnmpTrapDbgFlg )
	{
		if( b_DoBackup )
		{
			SYSF_logMsg("   b_DoBackup == TRUE\n",1,2,3,4,5,6);
		}
		else
		{
			SYSF_logMsg("   b_DoBackup == FALSE\n",1,2,3,4,5,6);
		}
	}

	// SNMP TRAP設定 = [有効]の場合 かつ 
	// HDDがバックアップ中(・リストア中)でない場合 
	if( (b_EnableSnmpTrap_NW1 || b_EnableSnmpTrap_NW2) && !b_DoBackup ) // 【IT5_4.2】【RQ686】HCL-Ramkisson 2017/10/13 Modify.
	{
		// pTrapGroup設定 
		t_sMioSnmpTrapGroup sendTarget;
		memset( &sendTarget, 0x00, sizeof(t_sMioSnmpTrapGroup) );
		UCHAR uc_IPAddress[254];			// Minerva/Zeus 静的解析 2011/02/21 y.ito 256->254(NVDD_Max_SnmpTrapIPAddress)にサイズ変更.
		UINT16 us_PortNo;
		UCHAR uc_Community[MAX_STRING16];	// 2007.3.1	[静的解析エラー] nicfum側ののサイズに合わせて修正	kawabe
		UCHAR uc_HostNetworkAddress[MAX_STRING9];
		UCHAR uc_HostNodeAddress[MAX_STRING13];
		UINT16 us_Loop;
		pc_SnmpTrapSetting->lock();
		// IP1 
		if( pc_SnmpTrapSetting->getSnmpTrapEvent1( e_SnmpTrapEvent ) )
		{
			pc_SnmpTrapSetting->getIPAddress1( (char*)uc_IPAddress );
			strcpy((char*)sendTarget.sTrapTargetIP[0].ucHostAddress, (const char*)uc_IPAddress );
			us_PortNo = (ushort)pc_SnmpTrapSetting->getPortNumber1();
			sendTarget.sTrapTargetIP[0].usPortNo = us_PortNo;	/* ★ */
			pc_SnmpTrapSetting->getCommunityName1( (char*)uc_Community );
			strcpy((char*)sendTarget.sTrapTargetIP[0].ucCommunity, (const char*)uc_Community );
			/* DEBUG */
			if( ui_SnmpTrapDbgFlg )
			{
				SYSF_logMsg("   IP1(Alert=%d) == ON\n",e_SnmpTrapEvent,2,3,4,5,6);
				SYSF_logMsg("   IP1(IPAddress) == %s\n",(intptr_t)sendTarget.sTrapTargetIP[0].ucHostAddress,2,3,4,5,6);
				SYSF_logMsg("   IP1(PortNumber) == %d\n",sendTarget.sTrapTargetIP[0].usPortNo,2,3,4,5,6);
				SYSF_logMsg("   IP1(Community) == %s\n",(intptr_t)sendTarget.sTrapTargetIP[0].ucCommunity,2,3,4,5,6);
			}
		}
		else
		{
			/* DEBUG */
			if( ui_SnmpTrapDbgFlg )
			{
				SYSF_logMsg("   IP1(Alert=%d) == OFF\n",e_SnmpTrapEvent,2,3,4,5,6);
			}
		}
		// IP2 
		if( pc_SnmpTrapSetting->getSnmpTrapEvent2( e_SnmpTrapEvent ) )
		{
			pc_SnmpTrapSetting->getIPAddress2( (char*)uc_IPAddress );
			strcpy((char*)sendTarget.sTrapTargetIP[1].ucHostAddress, (const char*)uc_IPAddress );
			us_PortNo = (ushort)pc_SnmpTrapSetting->getPortNumber2();
			sendTarget.sTrapTargetIP[1].usPortNo = us_PortNo;	/* ★ */
			pc_SnmpTrapSetting->getCommunityName2( (char*)uc_Community );
			strcpy((char*)sendTarget.sTrapTargetIP[1].ucCommunity, (const char*)uc_Community );
			/* DEBUG */
			if( ui_SnmpTrapDbgFlg )
			{
				SYSF_logMsg("   IP2(Alert=%d) == ON\n",e_SnmpTrapEvent,2,3,4,5,6);
				SYSF_logMsg("   IP2(IPAddress) == %s\n",(intptr_t)sendTarget.sTrapTargetIP[1].ucHostAddress,2,3,4,5,6);
				SYSF_logMsg("   IP2(PortNumber) == %d\n",sendTarget.sTrapTargetIP[1].usPortNo,2,3,4,5,6);
				SYSF_logMsg("   IP2(Community) == %s\n",(intptr_t)sendTarget.sTrapTargetIP[1].ucCommunity,2,3,4,5,6);
			}
		}
		else
		{
			/* DEBUG */
			if( ui_SnmpTrapDbgFlg )
			{
				SYSF_logMsg("   IP2(Alert=%d) == OFF\n",e_SnmpTrapEvent,2,3,4,5,6);
			}
		}
		// IP3 
		if( pc_SnmpTrapSetting->getSnmpTrapEvent3( e_SnmpTrapEvent ) )
		{
			pc_SnmpTrapSetting->getIPAddress3( (char*)uc_IPAddress );
			strcpy((char*)sendTarget.sTrapTargetIP[2].ucHostAddress, (const char*)uc_IPAddress );
			us_PortNo = (ushort)pc_SnmpTrapSetting->getPortNumber3();
			sendTarget.sTrapTargetIP[2].usPortNo = us_PortNo;	/* ★ */
			pc_SnmpTrapSetting->getCommunityName3( (char*)uc_Community );
			strcpy((char*)sendTarget.sTrapTargetIP[2].ucCommunity, (const char*)uc_Community );
			/* DEBUG */
			if( ui_SnmpTrapDbgFlg )
			{
				SYSF_logMsg("   IP3(Alert=%d) == ON\n",e_SnmpTrapEvent,2,3,4,5,6);
				SYSF_logMsg("   IP3(IPAddress) == %s\n",(intptr_t)sendTarget.sTrapTargetIP[2].ucHostAddress,2,3,4,5,6);
				SYSF_logMsg("   IP3(PortNumber) == %d\n",sendTarget.sTrapTargetIP[2].usPortNo,2,3,4,5,6);
				SYSF_logMsg("   IP3(Community) == %s\n",(intptr_t)sendTarget.sTrapTargetIP[2].ucCommunity,2,3,4,5,6);
			}
		}
		else
		{
			/* DEBUG */
			if( ui_SnmpTrapDbgFlg )
			{
				SYSF_logMsg("   IP3(Alert=%d) == OFF\n",e_SnmpTrapEvent,2,3,4,5,6);
			}
		}
		// IP4 
		if( pc_SnmpTrapSetting->getSnmpTrapEvent4( e_SnmpTrapEvent ) )
		{
			pc_SnmpTrapSetting->getIPAddress4( (char*)uc_IPAddress );
			strcpy((char*)sendTarget.sTrapTargetIP[3].ucHostAddress, (const char*)uc_IPAddress );
			us_PortNo = (ushort)pc_SnmpTrapSetting->getPortNumber4();
			sendTarget.sTrapTargetIP[3].usPortNo = us_PortNo;	/* ★ */
			pc_SnmpTrapSetting->getCommunityName4( (char*)uc_Community );
			strcpy((char*)sendTarget.sTrapTargetIP[3].ucCommunity, (const char*)uc_Community );
			/* DEBUG */
			if( ui_SnmpTrapDbgFlg )
			{
				SYSF_logMsg("   IP4(Alert=%d) == ON\n",e_SnmpTrapEvent,2,3,4,5,6);
				SYSF_logMsg("   IP4(IPAddress) == %s\n",(intptr_t)sendTarget.sTrapTargetIP[3].ucHostAddress,2,3,4,5,6);
				SYSF_logMsg("   IP4(PortNumber) == %d\n",sendTarget.sTrapTargetIP[3].usPortNo,2,3,4,5,6);
				SYSF_logMsg("   IP4(Community) == %s\n",(intptr_t)sendTarget.sTrapTargetIP[3].ucCommunity,2,3,4,5,6);
			}
		}
		else
		{
			/* DEBUG */
			if( ui_SnmpTrapDbgFlg )
			{
				SYSF_logMsg("   IP4(Alert=%d) == OFF\n",e_SnmpTrapEvent,2,3,4,5,6);
			}
		}
		// IP5 
		if( pc_SnmpTrapSetting->getSnmpTrapEvent5( e_SnmpTrapEvent ) )
		{
			pc_SnmpTrapSetting->getIPAddress5( (char*)uc_IPAddress );
			strcpy((char*)sendTarget.sTrapTargetIP[4].ucHostAddress, (const char*)uc_IPAddress );
			us_PortNo = (ushort)pc_SnmpTrapSetting->getPortNumber5();
			sendTarget.sTrapTargetIP[4].usPortNo = us_PortNo;	/* ★ */
			pc_SnmpTrapSetting->getCommunityName5( (char*)uc_Community );
			strcpy((char*)sendTarget.sTrapTargetIP[4].ucCommunity, (const char*)uc_Community );
			/* DEBUG */
			if( ui_SnmpTrapDbgFlg )
			{
				SYSF_logMsg("   IP5(Alert=%d) == ON\n",e_SnmpTrapEvent,2,3,4,5,6);
				SYSF_logMsg("   IP5(IPAddress) == %s\n",(intptr_t)sendTarget.sTrapTargetIP[4].ucHostAddress,2,3,4,5,6);
				SYSF_logMsg("   IP5(PortNumber) == %d\n",sendTarget.sTrapTargetIP[4].usPortNo,2,3,4,5,6);
				SYSF_logMsg("   IP5(Community) == %s\n",(intptr_t)sendTarget.sTrapTargetIP[4].ucCommunity,2,3,4,5,6);

			}
		}
		else
		{
			/* DEBUG */
			if( ui_SnmpTrapDbgFlg )
			{
				SYSF_logMsg("   IP5(Alert=%d) == OFF\n",e_SnmpTrapEvent,2,3,4,5,6);
			}
		}
		// IPX 
		if( pc_SnmpTrapSetting->getSnmpTrapEventIPX( e_SnmpTrapEvent ) )
		{
			pc_SnmpTrapSetting->getIPAddressIPX( (char*)uc_HostNetworkAddress );
			strcpy((char*)sendTarget.sTrapTargetIPX.ucHostNetworkAddress, (const char*)uc_HostNetworkAddress );
			pc_SnmpTrapSetting->getNodeAddressIPX( (char*)uc_HostNodeAddress );
			strcpy((char*)sendTarget.sTrapTargetIPX.ucHostNodeAddress, (const char*)uc_HostNodeAddress );
			pc_SnmpTrapSetting->getCommunityNameIPX( (char*)uc_Community );
			strcpy((char*)sendTarget.sTrapTargetIPX.ucCommunity, (const char*)uc_Community );
			/* DEBUG */
			if( ui_SnmpTrapDbgFlg )
			{
				SYSF_logMsg("   IPX(Alert=%d) == ON\n",e_SnmpTrapEvent,2,3,4,5,6);
				SYSF_logMsg("   IPX(HostNetworkAddress) == %s\n",(intptr_t)sendTarget.sTrapTargetIPX.ucHostNetworkAddress,2,3,4,5,6);
				SYSF_logMsg("   IPX(HostNodeAddress) == %s\n",(intptr_t)sendTarget.sTrapTargetIPX.ucHostNodeAddress,2,3,4,5,6);
				SYSF_logMsg("   IPX(Community) == %s\n",(intptr_t)sendTarget.sTrapTargetIPX.ucCommunity,2,3,4,5,6);
			}
		}
		else
		{
			/* DEBUG */
			if( ui_SnmpTrapDbgFlg )
			{
				SYSF_logMsg("   IPX(Alert=%d) == OFF\n",e_SnmpTrapEvent,2,3,4,5,6);
			}
		}
		pc_SnmpTrapSetting->unlock();

		// pSendParam設定 
		t_sMioSendTrapParam sendParam;
		memset( &sendParam, 0x00, sizeof(t_sMioSendTrapParam) );
		sendParam.nGenericType = MIO_TRAPTYP_ENTERPRISESSPECIFIC;
		for( us_Loop = 0; us_Loop < sizeof(sCnvTbl)/sizeof(SCnvTbl); us_Loop++ ) {
			if( sCnvTbl[us_Loop].e_SnmpTrapEvent == e_SnmpTrapEvent ) {
				sendParam.nSpecificType = sCnvTbl[us_Loop].ui_SpecificType; 
				/* DEBUG */
				if( ui_SnmpTrapDbgFlg )
				{
					SYSF_logMsg("   sendParam.nSpecificType == %d\n",sendParam.nSpecificType,2,3,4,5,6);
				}
				break;
			}
		}
		
		t_sMioTrapValue sendValue;
		memset( &sendValue, 0x00, sizeof(t_sMioTrapValue) );
		// MinervaZeus CQ348375 kubota 2011/6/16 START.
		// ObjectID情報が未取得かチェック.
		if( False == b_isGetOID )
		{
			// ObjectID情報を取得する.
			_getOID( sendOID );
			
			// 電源ON中はObjectIDは更新されないため、フラグを立てる.
			b_isGetOID = True;
		}
		// MinervaZeus CQ348375 kubota 2011/6/16 END.
		#if defined( __LP64__ ) 
			sendValue.pOid = sendOID;
		#else
			sendValue.pOid = reinterpret_cast<long unsigned int*>(sendOID);
		#endif
		
				/* DEBUG */
				if( ui_SnmpTrapDbgFlg )
				{
					for( int x = 0; x < MIBD_Max_OIDDigit; x++ )
					{
						SYSF_logMsg("   sendValue.pOid[%d] == %d\n",x,sendOID[x],3,4,5,6);
					}
				}
		sendValue.nOidLen = MIBD_Max_OIDDigit;	/* ★ */
		/* DEBUG */
		if( ui_SnmpTrapDbgFlg )
		{
			SYSF_logMsg("   sendValue.nOidLen == %d\n",sendValue.nOidLen,2,3,4,5,6);
		}
		sendValue.nASN1Type = MIO_TVTYP_DISPLAYSTRING;
		sendValue.pValue = (void*)"";
		sendValue.lValueLength = sizeof("");
		char c_dispInfo[FVDD_Max_SnmpDisplayString] = {0};
		for( us_Loop = 0; us_Loop < sizeof(sCnvTbl)/sizeof(SCnvTbl); us_Loop++ ) {
			// MinervaZeus CQ349518 kubota 2011/6/28 START.
			if( ( sCnvTbl[us_Loop].e_SnmpTrapEvent == e_SnmpTrapEvent )
			 && ( sCnvTbl[us_Loop].ss_UnitColorType == ss_UnitColorType ) ) {
			// MinervaZeus CQ349518 kubota 2011/6/28 END.
				// pc_displayInfoが非NULLの場合、MIO_TVTYP_DISPLAYSTRINGとして追加情報を結合した文字列を生成.
				if (pc_displayInfo) {
					snprintf(c_dispInfo, sizeof(c_dispInfo), "%s: %s", sCnvTbl[us_Loop].sc_MessageString, pc_displayInfo);
					sendValue.pValue = c_dispInfo;
				} else {
					sendValue.pValue = (void*)sCnvTbl[us_Loop].sc_MessageString;
				}
				//2021/03/04 [IT6_2.2]  RQ-817  IT6_2.2-399 HIENG End.
				/* DEBUG */
				if( ui_SnmpTrapDbgFlg )
				{
					SYSF_logMsg("   sendValue.pValue == %s\n",(intptr_t)sendValue.pValue,2,3,4,5,6);
				}
				sendValue.lValueLength = strlen((const char*)sendValue.pValue);	/* ★ */
				/* DEBUG */
				if( ui_SnmpTrapDbgFlg )
				{
					SYSF_logMsg("   sendValue.lValueLength == %d\n",sendValue.lValueLength,2,3,4,5,6);
				}
				break;
			}
		}
		sendParam.pTrapValueList = &sendValue;

		// 【IT5_4.2】【RQ686】HCL-Ramkisson 2017/10/13 Modify Start.
		// mioSendSNMPTrap コール 
		STATUS eStatus1 = ERROR; 
		STATUS eStatus2 = ERROR; 
#ifdef ROM_DIV_3
		if( pt_mioSendSNMPTrap != NULL )
		{
			/* DEBUG */
			if( ui_SnmpTrapDbgFlg )
			{
				SYSF_logMsg("   pt_mioSendSNMPTrap CALL !!!START!!!\n",1,2,3,4,5,6);
			}
			
			if(b_EnableSnmpTrap_NW1)
			{
				SYSF_logMsg("   \n pt_mioSendSNMPTrap CALL for b_EnableSnmpTrap_NW1 !!!START!!!\n",1,2,3,4,5,6);
			    sendTarget.eNwIfIndex = NWIF_TYPE_MAIN_LAN;
			    eStatus1 = ((STATUS(*)( t_sMioSnmpTrapGroup*, t_sMioSendTrapParam* ))pt_mioSendSNMPTrap)( &sendTarget, &sendParam );	/* ★ */
				SYSF_logMsg("   \n pt_mioSendSNMPTrap CALL !!!END!!! (Status1 = %d)\n",eStatus1,2,3,4,5,6);
			}	
		
			
			if(b_EnableSnmpTrap_NW2)
			{
				SYSF_logMsg("   \n pt_mioSendSNMPTrap CALL for b_EnableSnmpTrap_NW2 !!!START!!!\n",1,2,3,4,5,6);
			    sendTarget.eNwIfIndex = NWIF_TYPE_EXTEND_LAN;
			    eStatus2 = ((STATUS(*)( t_sMioSnmpTrapGroup*, t_sMioSendTrapParam* ))pt_mioSendSNMPTrap)( &sendTarget, &sendParam );	/* ★ */
				SYSF_logMsg("   \n pt_mioSendSNMPTrap CALL !!!END!!! (Status2 = %d)\n",eStatus2,2,3,4,5,6);
			}		
		}
		else
		{
			DWARNING( "■□[NICFUM ROMDIV]MIBC_SnmpTrapSettingObserver::notifySnmpTrap() pt_mioSendSNMPTrap NULL pointer.□■\n" );
		}
#else	// #ifdef ROM_DIV_3
        if(b_EnableSnmpTrap_NW1)
		{
			 sendTarget.eNwIfIndex = NWIF_TYPE_MAIN_LAN;
			 eStatus1 = mioSendSNMPTrap( &sendTarget, &sendParam ); 
		}
		if(b_EnableSnmpTrap_NW2)
		{
			 sendTarget.eNwIfIndex = NWIF_TYPE_EXTEND_LAN;
			 eStatus2 = mioSendSNMPTrap( &sendTarget, &sendParam ); 
		}
		
#endif	// #ifdef ROM_DIV_3
		// ZeusS_ZX0 制御構成変更_No.76-2 Hcl_Bairavi 2015/10/30 START.
		if(b_EnableSnmpTrap_NW1)
		{
			if ( eStatus1 != OK ) {
				DWARNING( "mioSendSNMPTrap Failure from NW1 \n" );
				APIC_NetworkErrorCnt* pc_NetworkErrorCnt = APIC_NetworkErrorCnt::newInstance();
	    		if(NULL != pc_NetworkErrorCnt) {
	        		pc_NetworkErrorCnt->lock();
			        pc_NetworkErrorCnt->inc(TYPE_NetworkErrorCnt_MIB_IPP, MIBD_NetErrorSubCode_MioSendSNMPTrapErr, 1);
	       		 	pc_NetworkErrorCnt->unlock();
				}
			}
		}
		if(b_EnableSnmpTrap_NW2)
		{
			if ( eStatus2 != OK ) {
				DWARNING( "mioSendSNMPTrap Failure from NW2 \n" );
				APIC_NetworkErrorCnt* pc_NetworkErrorCnt = APIC_NetworkErrorCnt::newInstance();
	    		if(NULL != pc_NetworkErrorCnt) {
	        		pc_NetworkErrorCnt->lock();
			        pc_NetworkErrorCnt->inc(TYPE_NetworkErrorCnt_MIB_IPP, MIBD_NetErrorSubCode_MioSendSNMPTrapErr, 1);
	       		 	pc_NetworkErrorCnt->unlock();
				}
			}
		}
		//2021/03/04 [IT6_2.2]  RQ-817  IT6_2.2-399 HIENG Start.
		if (TYPE_SnmpTrapEventVirusScan == e_SnmpTrapEvent) {
			if ((True == b_EnableSnmpTrap_NW1) || (True == b_EnableSnmpTrap_NW2)) {
				Bool status = True;
				if ((OK != eStatus1) && (OK != eStatus2)) {
					status = False;
				}
				APIC_VirusScan::newInstance()->setVirusSendInfoSnmp(status);
			}
		}
		//2021/03/04 [IT6_2.2]  RQ-817  IT6_2.2-399 HIENG End.
		// 【IT5_4.2】【RQ686】HCL-Ramkisson 2017/10/13 Modify End.
		// ZeusS_ZX0 制御構成変更_No.76-2 Hcl_Bairavi 2015/10/30 END.
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_SnmpTrapSettingObserver::_getOID 
// ABSTRACT	: OID取得  
// FUNCTION	: 
// NOTE		: 
// RETURN	: 
// CREATE	: 06-03-09	今村 
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
#define MIBD_Max_OIDString	64
#define MIBD_Max_DigitBuf	8
#define MIBD_Delimiter_Dot	0x2e
#define MIBD_Delimiter_NULL	0x00
void MIBC_SnmpTrapSettingObserver::_getOID( uint32_t* sendOID )
{
	if( sendOID == NULL ) { return; };			// ASSERTION FAILED

	// MinervaZeus CQ348375 kubota 2011/6/16 START.
	char sc_SysObjectID[MIBD_Max_OIDString + 1] = {0,};
	// MinervaZeus CQ348375 kubota 2011/6/16 END.
	NETG_FixedValueDefine_getSysObjectID( sc_SysObjectID );

	UINT16 us_Loop = 0;
	UINT16 us_OIDLoc = 0;
	for( us_Loop = 0; us_Loop < MIBD_Max_OIDDigit; us_Loop++ )
	{
		UINT16 us_DigitLoc = 0;
		char sc_DigitBuf[MIBD_Max_DigitBuf];
		memset( sc_DigitBuf, 0x00, sizeof(sc_DigitBuf) );
		while( sc_SysObjectID[us_OIDLoc] != MIBD_Delimiter_Dot && 
			   sc_SysObjectID[us_OIDLoc] != MIBD_Delimiter_NULL )
		{
			sc_DigitBuf[us_DigitLoc] = sc_SysObjectID[us_OIDLoc];
			us_DigitLoc++;
			us_OIDLoc++;
		}
		sendOID[us_Loop] = atoi(sc_DigitBuf);
		us_OIDLoc++;
	}
}

//【AR1037899】FQ-OPSD-1700411-OFP212 市場要望対応 HCL-Ramkisson 2018/01/30 End.
// 2019/04/02 FPT-SangTX [IT6_1.0MR2] AR1075308 Merge START.
// 2018/11/28 mt_iwa FQ-OPSD-1800450-OFP247 START.
// APIC_Jobのlock/unlock(セマフォ取得処理)にて、デットロックが発生するケースが存在するため処理を削除する.
void MIBC_ActiveJobListObserver::deleteJob(APIC_Job* pc_Job)
{
	if(pc_Job != NULL)
	{
		 if(True == pc_Job->getInvalidJob())
		 {
			// pc_Job->lock();
		 	for (short i = 0; i < MIBD_Max_RXJOBCount; i++)
		 	{
		   		if(i32p_nJobIdActiveObserverStatusSummary[i] == MIBD_UndefNetRXJobID) 
		   		{
					i32p_nJobIdActiveObserverStatusSummary[i] = pc_Job->getJobStatus()->getJobID();
					break;
		    	}			
		 	}
		 	// pc_Job->unlock();
		 }
	}
}
// 2018/11/28 mt_iwa FQ-OPSD-1800450-OFP247 END.
// 2019/04/02 FPT-SangTX [IT6_1.0MR2] AR1075308 Merge END.
// 【AR1037899】FQ-OPSD-1700411-OFP212 市場要望対応 HCL-Ramkisson 2018/01/30 End.

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_vProcWSDMetadata 
// ABSTRACT	: WSD META DATA関連情報の取得 
// FUNCTION	: WSD META DATA関連情報の取得 
// NOTE		: 受信メッセージのパラメータには何もセットされていない 
// RETURN	: 無し 
// CREATE	: 	[2006/11/27 制御12 今村] 
//					新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_FuncManMIB::_vProcWSDMetadata(
	MsgMioToApp&		_r_srRcvMsg,		// MIOからの受信メッセージ 
	MsgAppToMio&		_r_srTraMsg			// MIOへの送信メッセージ 
)
{
	// データ領域の確保 
	t_sWSDMetadata* ps_WSDMetadata = NULL;
	for( ; ; )
	{
		ps_WSDMetadata = (t_sWSDMetadata*)malloc( sizeof( t_sWSDMetadata ) );
		if( ps_WSDMetadata NE NULL )
		{
			break;
		}
		register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;
		if( sl_Tick == 0L ) { sl_Tick = 1L; }
		++sl_Tick;
		SYSF_taskDelay( sl_Tick );
	}

	// 領域クリア 
	memset( ps_WSDMetadata, 0, sizeof( t_sWSDMetadata ) );

	// WSD META DATA の取得 
	vGetWSDMetadata( ps_WSDMetadata );

	// 応答データの設定 
	_r_srTraMsg.pParameter = ps_WSDMetadata;

}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_vProcWSDPrinterActiveJobs 
// ABSTRACT	: WSD Printer Active Jobs関連情報の取得 
// FUNCTION	: WSD Printer Active Jobs関連情報の取得 
// NOTE		: 受信メッセージのパラメータには何もセットされていない 
// RETURN	: 無し 
// CREATE	: [2007/01/05 制御12 今村] 新規作成 
//			: DB No.81935対策 WSD Printer Active Jobsが0である場合、NULLポインタを返すように修正  07.06.07 mt_iwa 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_FuncManMIB::_vProcWSDPrinterActiveJobs(
	MsgMioToApp&		_r_srRcvMsg,		// MIOからの受信メッセージ 
	MsgAppToMio&		_r_srTraMsg			// MIOへの送信メッセージ 
)
{
    // WSD Printer Active Jobsが0の場合、NULLポインタを返す 
    if ( ssWSDActiveJobNumNew == 0 )
    {
        // 応答データの設定 
        _r_srTraMsg.pParameter = NULL;
    }
    else
    {
		// MinervaSBK CQ517225 2015.09.02 OPS1八代 START.
#if 1
		t_sWSDPrinterJobs* ps_Bef = NULL;	// psNextに設定するための一時変数.

		for( int i = 0; i < ssWSDActiveJobNumNew; i++ ) {

			// データ領域の確保 
			t_sWSDPrinterJobs* ps_WSDPrinterJobs = NULL;
			for( ; ; )
			{
				ps_WSDPrinterJobs = (t_sWSDPrinterJobs*)malloc( sizeof( t_sWSDPrinterJobs ) );
				if( ps_WSDPrinterJobs NE NULL )
				{
					break;
				}
				register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;
				if( sl_Tick == 0L ) { sl_Tick = 1L; }
				++sl_Tick;
				SYSF_taskDelay( sl_Tick );
			}

			// 領域クリア 
			memset( ps_WSDPrinterJobs, 0, sizeof( t_sWSDPrinterJobs ) );

			// WSD Printer Active Jobs の取得 
			memcpy( (t_sWSDPrinterJobs*)ps_WSDPrinterJobs, &spWSDJobDataTblActive[i], sizeof( t_sWSDPrinterJobs ) );

			// 次エントリへのポインタは不明なので、NULLにしておく.
			ps_WSDPrinterJobs->psNext = NULL;

			// 応答データには先頭アドレスを設定.
			if( 0 == i ) _r_srTraMsg.pParameter = ps_WSDPrinterJobs;

			// 前エントリへのポインタが確定したので、設定する.
			if( NULL != ps_Bef ) { 
				ps_Bef->psNext = ps_WSDPrinterJobs;
			}

			// 次週、エントリポインタを設定するために、アドレスを保持.
			ps_Bef = ps_WSDPrinterJobs;
		}
#else
		// データ領域の確保 
		t_sWSDPrinterJobs* ps_WSDPrinterJobs = NULL;
		for( ; ; )
		{
			ps_WSDPrinterJobs = (t_sWSDPrinterJobs*)malloc( sizeof( t_sWSDPrinterJobs ) );
			if( ps_WSDPrinterJobs NE NULL )
			{
				break;
			}
			register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;
			if( sl_Tick == 0L ) { sl_Tick = 1L; }
			++sl_Tick;
			SYSF_taskDelay( sl_Tick );
		}

		// 領域クリア 
		memset( ps_WSDPrinterJobs, 0, sizeof( t_sWSDPrinterJobs ) );

		// WSD Printer Active Jobs の取得 
		memcpy( (t_sWSDPrinterJobs*)ps_WSDPrinterJobs, (const t_sWSDPrinterJobs*)spWSDJobDataTblActive, sizeof( t_sWSDPrinterJobs ) );

		// 応答データの設定 
		_r_srTraMsg.pParameter = ps_WSDPrinterJobs;
#endif
		// MinervaSBK CQ517225 2015.09.02 OPS1八代 END.
	}

}
// ZeusMLK【CQ373819】八代晃佳 2012/11/01 Start.
#define MIBD_WSDWaitNum		   4		// MIBTaskManCyclic処理待ち回数.
#define MIBD_WSDWaitTick	   2000		// MIBTaskManCyclic処理待ちディレイ時間(msec).
// ZeusMLK【CQ373819】八代晃佳 2012/11/01 END
// ZeusS_ZX0 CQ524295 OPS1八代 2016.01.27 START.
Bool bl_Dbg_WSDV2Verify = False;
// ターミナルコール用.
extern "C"{
	void DbgMIB_WSDV2Verify();
}

void DbgMIB_WSDV2Verify( Bool bl_Input )
{
	bl_Dbg_WSDV2Verify = bl_Input;
	return;
}

// ZeusS_ZX0 CQ524295 OPS1八代 2016.01.27 END.

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_FuncManMIB::_vProcWSDPrinterJobStatus 
// ABSTRACT	: WSD Printer Job Status関連情報の取得 
// FUNCTION	: WSD Printer Job Status関連情報の取得 
// NOTE		: 受信メッセージのパラメータにはJobIDがセットされている 
// RETURN	: 無し 
// CREATE	: 	[2007/01/06 制御12 今村] 
//					新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_FuncManMIB::_vProcWSDPrinterJobStatus(
	MsgMioToApp&		_r_srRcvMsg,		// MIOからの受信メッセージ 
	MsgAppToMio&		_r_srTraMsg			// MIOへの送信メッセージ 
)
{
	// 受信データの取得 
	register INT32* i32pJobID = (INT32*)_r_srRcvMsg.pParameter;

	// データ領域の確保 
	t_sWSDPrinterJobSummary* ps_WSDPrinterJobSummary = NULL;
	for( ; ; )
	{
		ps_WSDPrinterJobSummary = (t_sWSDPrinterJobSummary*)malloc( sizeof( t_sWSDPrinterJobSummary ) );
		if( ps_WSDPrinterJobSummary NE NULL )
		{
			break;
		}
		register int32_t sl_Tick = (int32_t)SYSF_sysClkRateGet() * 1000 / 1000;
		if( sl_Tick == 0L ) { sl_Tick = 1L; }
		++sl_Tick;
		SYSF_taskDelay( sl_Tick );
	}

	// 領域クリア 
	memset( ps_WSDPrinterJobSummary, 0, sizeof( t_sWSDPrinterJobSummary ) );

	// WSD Printer Job Status の取得 
// ZeusMLK【CQ373819】八代晃佳 2012/11/02 Start.
	INT32 i32_IndexWSDWait = 0;
	INT32 i32_Index = 0;
	for( i32_IndexWSDWait = 0; i32_IndexWSDWait < MIBD_WSDWaitNum; i32_IndexWSDWait++ ) 
	{
		// 指定されたJobIDを検索する 
		for( i32_Index = 0; i32_Index < MIBD_Max_JobLogListAll; i32_Index++ )
		{
			if( *i32pJobID == spWSDJobDataTblLog[i32_Index].sJobSummary.sStatus.nJobId )
			{
				// 検索で見つかった場合のみ、コピーをする。
				memcpy( (t_sWSDPrinterJobSummary*)ps_WSDPrinterJobSummary, 
					(const t_sWSDPrinterJobSummary*)&spWSDJobDataTblLog[i32_Index].sJobSummary, 
					sizeof( t_sWSDPrinterJobSummary ) );
				break;
			}
		}

		// 指定されたJobIDの情報が見つからなかった場合.
		// ディレイを行い、MIBTaskManCyclicタスクの処理を待つ.
		if( 0 == ps_WSDPrinterJobSummary->sStatus.nJobId ) {

			// セマフォのロック解除 
			_m_irAllVIDMan.vSemphoreUnLock();

			// ディレイ.
			register int32_t sl_WSDWaitTick = (int32_t)SYSF_sysClkRateGet() * MIBD_WSDWaitTick / 1000;	// ｍｓ→ティック数に変換
			if( sl_WSDWaitTick == 0L ) { sl_WSDWaitTick = 1L; }
			SYSF_taskDelay( sl_WSDWaitTick );

			// セマフォのロック 
			_m_irAllVIDMan.vSemphoreLock(); 

		// 指定されたJobIDの情報が見つかった場合は抜ける.
		} else {
			break;
		}
	}
// ZeusMLK【CQ373819】八代晃佳 2012/11/02 End.

	// ZeusS_ZX0 CQ524295 OPS1八代 2016.01.27 START.
	if( True == bl_Dbg_WSDV2Verify ) {
		printf("[MIB] DebugMode WSDV2Verify ON \n");
		// 指定されたJobIDの情報が見つからなかった場合、ジョブIDを鸚鵡返しに返す.
		if( 0 == ps_WSDPrinterJobSummary->sStatus.nJobId ) {
			printf("[MIB] JobID is Not Found, Repeat = %d \n", *i32pJobID );
			ps_WSDPrinterJobSummary->sStatus.nJobId = *i32pJobID;
			ps_WSDPrinterJobSummary->sStatus.eJobState = JSTATE_COMPLETED;
		} else {
			printf("[MIB] JobID is Found = %d \n", ps_WSDPrinterJobSummary->sStatus.nJobId );
		}
	}
	// ZeusS_ZX0 CQ524295 OPS1八代 2016.01.27 END.

	// 応答データの設定 
	_r_srTraMsg.pParameter = ps_WSDPrinterJobSummary;

}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_InterpreterObserver::builtinDataChanged 
// ABSTRACT	: 内蔵データ状態変化通知  
// FUNCTION	: 
// NOTE		: 
// RETURN	: 
// CREATE	: 07-01-12	河辺 
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_InterpreterObserver::builtinDataChanged()
{
	// ICCプロファイル情報の取得 
	getICCProfileInfo_MIB();
}


// IT5_4.1 RQ-643/KRQ-290 HCL-Sankar 2016/12/17 Added Start.
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_InitSuperWarpObserver::initialize()
// ABSTRACT	: 内蔵データ状態変化通知  
// FUNCTION	: 
// NOTE		: 
// RETURN	: 
// CREATE	: 2016.12.17	河辺 
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////

void MIBC_InitSuperWarpObserver::initialize()
{
//2021/06/30 [IT6_2.1_MR_CheckIn][TAS000000008827] Start.
#if 1
	APIC_SuperWarpOperation* pc_sprwrpOp = APIC_SuperWarpOperation::newInstanceAdmin();
	//モジュール番号: 12
	//SuperWarp復帰 初期化状態: Start
	if( NULL != pc_sprwrpOp ){
		pc_sprwrpOp->setInitializeStatus( 12, TYPE_SuperWarpInitializeStatus_Start );
	}
	taskAPI_MIB_SuperWarp();
#else
	APIC_SuperWarpOperation* pc_sprwrpOp = APIC_SuperWarpOperation::newInstanceAdmin();
	//モジュール番号: 12
	//SuperWarp復帰 初期化状態: Start
	getInitTime();							// 初期時刻取得 
	set_prtMarkerPowerOnCount_init();
	if( NULL != pc_sprwrpOp ){
		pc_sprwrpOp->setInitializeStatus( 12, TYPE_SuperWarpInitializeStatus_Start );
		pc_sprwrpOp->setInitializeStatus( 12, TYPE_SuperWarpInitializeStatus_EndOk );
	}
#endif
//2021/06/30 [IT6_2.1_MR_CheckIn][TAS000000008827] End.
}
// IT5_4.1 RQ-643/KRQ-290 HCL-Sankar 2016/12/17 Added End.

// 【IT5_4.2】【RQ686】HCL-Kaushik 2017/09/01 Added start.
TYPE_NetworkSpecific getNetworkType(MIO_NWIF_TYPE eNwIfIndex)
{
	TYPE_NetworkSpecific e_NetworkType = TYPE_NetworkSpecific_NONE;
              if(eNwIfIndex == NWIF_TYPE_MAIN_LAN)
              {
              		e_NetworkType = TYPE_NetworkSpecific_NW1;
              } else if(eNwIfIndex == NWIF_TYPE_EXTEND_LAN) {
              		e_NetworkType = TYPE_NetworkSpecific_NW2;
              }
              else {
              		DTRACE( ("***Invalid Network Type***\r\n") );
			  }
	return e_NetworkType;
}
// 【IT5_4.2】【RQ686】HCL-Kaushik 2017/09/01 Added End.
