//==================================================================================================.
//[ファイル名]NETC_TwainCommon.cpp.
//[内容]TCPSocket内で使用する共通処理等を記載する.
//[コメント]コールバック等の特殊な関数に関してはこちらへは記載しない.
//==================================================================================================.
//インクルードファイル.
//==================================================================================================.
#include "NETC_TwainService.h"
#include "NETC_TwainSender.h"
#include "NETC_TwainReceiver.h"
#include "NETC_TwainStatus.h"
#include "NETC_TwainSupport.h"

#include "system/doc/docg/DOCC_CompositeDoc.h"
#include "system/nvd/APIC_Setting_FaxGfpSetting.h"
#include "system/nvd/APIC_Setting_FaxJobSetting.h"
#include "system/uac/APIC_AuthenticationServerSetting.h"

#include "LIBC_LogLib.h"		// 2010/07/01 amiya 【Minerva】<UV-00794> デバッグログ収集システム対応.

#include "lib/KM_md5.h"			// MinervaBK2nd CQ380261 OPS1八代 2013.05.30.
// 2014/09/03 [ZeusS][RQ-232]OPS1前田 Start.
#include "system/sys/APIC_AndroidPanel.h"
// 2014/09/03 [ZeusS][RQ-232]OPS1前田 End.

//==================================================================================================.
//変数･型の宣言.
int  NETC_TwainDebug_siSystemColor = -1; //0=Mono, 1=Color.
int  NETC_TwainDebug_siCenterStapleInFolds = -1; //0=NotInFolds,1=InFolds
//==================================================================================================.

// >> デバッグ関数 ---------------------------------------------------------------------------------.
// 2010/07/01 amiya 【Minerva】<UV-00794> デバッグログ収集システム対応 START.
#define NETD_TCPSocketLogMaxBuffSize		200										// .
//20130603 OPS1前田(Sky) [CQ380263]Heliosメモリ削減対応.
//#define NETD_TCPSocketLogMaxCount			2000									// 履歴保存行数.
#define NETD_TCPSocketLogMaxCount			1000									// 履歴保存行数.
static char sc_TCPSocketLogBuff[ NETD_TCPSocketLogMaxCount ][ NETD_TCPSocketLogMaxBuffSize ];
static uint32_t ul_TCPSocket_LogCnt = 0;										// 現在の有効行数.

void _debugTCPSocketLogWrite( char* a0, intptr_t a1, intptr_t a2, intptr_t a3, intptr_t a4, intptr_t a5, intptr_t a6 );	// TCPSocket_Log書き込み.
void _debugTCPSocketLogPrint();																// TCPSocket_Log出力.

//===================================================================================================.
// TCPSocket_Log書き込み.
//===================================================================================================.
void _debugTCPSocketLogWrite( char* a0, intptr_t a1, intptr_t a2, intptr_t a3, intptr_t a4, intptr_t a5, intptr_t a6 )
{
	uint32_t ul_Count = 0;
	uint32_t ul_Tick = SYSF_tickGet();
	char sc_DebugTCPSocketLogTemp[ NETD_TCPSocketLogMaxBuffSize ] = { 0, };
	char sc_DebugTCPSocketLog[ NETD_TCPSocketLogMaxBuffSize ] = { 0, };
	
	if ( ul_TCPSocket_LogCnt < NETD_TCPSocketLogMaxCount) {
		ul_Count =  ul_TCPSocket_LogCnt;
	} else {
		ul_Count = 0;
		ul_TCPSocket_LogCnt = 0;
	}
	// TCPSocketのログをsc_DebugTCPSocketLogTempに出す.
	sprintf( sc_DebugTCPSocketLogTemp, a0, a1, a2, a3, a4, a5, a6 );
	snprintf( sc_DebugTCPSocketLog, NETD_TCPSocketLogMaxBuffSize, "(%010u)%s", ul_Tick, sc_DebugTCPSocketLogTemp );

	// .
	memset( sc_TCPSocketLogBuff[ ul_Count ], 0, sizeof( sc_TCPSocketLogBuff[ ul_Count ] ) );
	strncpy( sc_TCPSocketLogBuff[ ul_Count ], sc_DebugTCPSocketLog, ( sizeof( sc_TCPSocketLogBuff[ ul_Count ] ) ) -1 );

	// ログシステム書き込み処理.
	LIBS_TCPSocketLog s_TCPSocketLog;									// ログ書き込み用構造体.
	memset( &s_TCPSocketLog, 0x00, sizeof( s_TCPSocketLog ) );			// 初期化.
	strncpy( s_TCPSocketLog.sc_TCPSocket_LogBuff, sc_DebugTCPSocketLog, sizeof( s_TCPSocketLog.sc_TCPSocket_LogBuff ) );		// ログ書き込み用構造体にデータ格納.// ncpyにする

	// 書き込み用カテゴリー指定.
	short ss_LogCategory = LIBD_LogCategoryNet;							// ネットワーク/スキャナー機能.

	// インデックス指定.
	short ss_Index = (-1);		// 次回インデックス位置指定.

	// LogLibＡＰＩをコールし、共有メモリへ書き込み.
	LIBF_WriteSystemLog( LIBD_LogID_TCPSocketLog, LIBD_LogOutPutLevel2, ss_LogCategory, &s_TCPSocketLog, &ss_Index );

	ul_TCPSocket_LogCnt++;
}
//===================================================================================================.
// TCPSocket_Log出力.これいるのか？.
//===================================================================================================.
void _debugTCPSocketLogPrint()
{
	uint32_t ul_Count;
SYSF_logMsg("[LogTop = %04d]\n", ( ul_TCPSocket_LogCnt - 1 ),0,0,0,0,0);
	for (ul_Count = 0; ul_Count < NETD_TCPSocketLogMaxCount; ul_Count++) {
		if ( strlen( sc_TCPSocketLogBuff[ ul_Count ] ) != 0 ) {
SYSF_logMsg("[No.%04d] %s\n", (int)ul_Count, reinterpret_cast<intptr_t>(sc_TCPSocketLogBuff[ ul_Count ]), 0,0,0,0 );
		} else {
			break;
		}
	}
}

// ▼ START ▼ CQ376665 S12-r_mori 2012/12/21 SysLog強化対応 
extern "C" {
	extern void printTCPSocketDebug();
}

void printTCPSocketDebug()
{
	extern void _debugWSDScanLogPrint();
	
	_debugTCPSocketLogPrint();
	
	_debugWSDScanLogPrint();

}
// ▲  END  ▲ CQ376665 S12-r_mori 2012/12/21 SysLog強化対応 

// 2010/07/01 amiya 【Minerva】<UV-00794> デバッグログ収集システム対応 END.

#ifndef LINUX
#include	"logLib.h"
#endif // LINUX
Bool NetTwainLogFlag = False;
Bool NetTwainLogFlag2 = False;
int NetTwainLogFlagCL = 0;
Bool NetTwainDebugEnableSSLFlag = False; // 2021/12/28 [IT6_2.2MR2] OP_BTS-37226 HIENG.
Bool NetTwainDebugEnableUserInfoFlag = False; //2022/2/7 [IT6_2.2MR2] OP_BTS-37530 HIENG.
#define NETD_TDEBUGLOG_MAX 128
#define NETD_TDEBUGSTRING_MAX 100
static struct {
	char message[ NETD_TDEBUGSTRING_MAX ];
} NetTwainLog[ NETD_TDEBUGLOG_MAX ];

void NetTwainSetTrace( Bool flag )
{
	NetTwainLogFlag = flag;
}
void NetTwainSetTrace2( Bool flag )
{
	NetTwainLogFlag2 = flag;
}
void NetTwainSetTraceCL( int flag )
{
	NetTwainLogFlagCL = flag;
}
// 2021/12/28 [IT6_2.2MR2] OP_BTS-37226 HIENG start.
void NetTwainDebugEnableSSL( Bool flag )
{
	NetTwainDebugEnableSSLFlag = flag; 
}
// 2021/12/28 [IT6_2.2MR2] OP_BTS-37226 HIENG end.
// 2022/2/7 [IT6_2.2MR2] OP_BTS-37530 HIENG start.
void NetTwainDebugEnableUserInfo( Bool flag )
{
	NetTwainDebugEnableUserInfoFlag = flag;
}
//ユーザー情報用ログ出力（個人情報はSysLogを含めデフォルト非表示とする）
void NetTwainDebugTraceUserInfo( char* a0, intptr_t a1, intptr_t a2, intptr_t a3, intptr_t a4, intptr_t a5, intptr_t a6 ){
	if( NetTwainDebugEnableUserInfoFlag == True )
	{
		NetTwainDebugTrace(a0,a1,a2,a3,a4,a5,a6 );
	}
}
// 2022/2/7 [IT6_2.2MR2] OP_BTS-37530 HIENG end.
void NetTwainDebugTrace( char* a0, intptr_t a1, intptr_t a2, intptr_t a3, intptr_t a4, intptr_t a5, intptr_t a6 ){
	_debugTCPSocketLogWrite(a0,a1,a2,a3,a4,a5,a6 );				// 2010/07/01 amiya 【Minerva】<UV-00794> デバッグログ収集システム対応.
	if( NetTwainLogFlag == True )
		SYSF_logMsg(a0,a1,a2,a3,a4,a5,a6 );
}
void NetTwainDebugTrace2( char* a0, int a1, int a2, int a3, int a4, int a5, int a6 ){
	if( NetTwainLogFlag2 == True )
		SYSF_logMsg(a0,a1,a2,a3,a4,a5,a6 );
}
void NetTwainDebugTraceSI( char* a0, int a1, int a2, int a3, int a4, int a5, int a6 ){
	if( NetTwainLogFlagCL & 0x01 )
		SYSF_logMsg(a0,a1,a2,a3,a4,a5,a6 );
}
void NetTwainDebugTraceCL( char* a0, int a1, int a2, int a3, int a4, int a5, int a6 ){
	if( NetTwainLogFlagCL & 0x02 )
		SYSF_logMsg(a0,a1,a2,a3,a4,a5,a6 );
}
void registNetTwainLog( char *pc_FunctionName ){
	static unsigned char i=0;
	static uint32_t j=0;

	sprintf( NetTwainLog[i/2].message,"No[%lu]::%s", j, pc_FunctionName );
	i= (unsigned char)(i+2);		j++;
}
void NetTwainHddStatus()
{
	APIC_HddInfo* pc_HddInfo;
	pc_HddInfo = APIC_HddInfo::newInstance();
	TYPE_HddStatus type_HddStatus;
	pc_HddInfo->lock();
	type_HddStatus = pc_HddInfo->getHddStatus();
	pc_HddInfo->unlock();
	NetTwainDebugTrace("HddStatus is [%d]\n",(int)type_HddStatus,0,0,0,0,0 );
}
void NetTwainSleepWait( char data )
{
	if ( 1 <= data && data <= 5 ) {
		NetTwainSleepWaitTime = data;
	} else {
		NetTwainSleepWaitTime = 3;
	}
}
void NetTwainPowerMode()
{
	APIC_Power* pc_Power;
	pc_Power = APIC_Power::newInstance();
	pc_Power->lock();
	Bool isPreHeat = pc_Power->isPreHeatMode();
	Bool isSleep = pc_Power->isSleepMode();
	Bool isSubPowerOff = pc_Power->isSubPowerOff();
	pc_Power->unlock();
	// 【IT5_4.1】【RQ-643:Start-up time shortening】SCD2深谷 2016/11/28 START.
	TYPE_SuperWarpStatus e_SuperWarp = APIC_Power::newInstance()->getSuperWarpStatus();		// SuperWarp状態取得.
	NetTwainDebugTrace("PowerMode is isPreHeat = [%d], isSleep = [%d], isSubPowerOff = [%d]\n",(int)isPreHeat,(int)isSleep,(int)isSubPowerOff,(int)e_SuperWarp,0,0 );
	// 【IT5_4.1】【RQ-643:Start-up time shortening】SCD2深谷 2016/11/28 END.
}
void NetTwainBoxInfo()
{
	netc_BoxInfoMan.printBoxInfo();
}
void NetTwainNotExistBoxInfo()
{
	netc_BoxInfoMan.printNotExistBoxInfo();
}

void dbgsetBackupMediaID( unsigned short _MediaID )
{
	netc_TwainClientMan.setBackupMediaID( (char)_MediaID );
}
void dbgsetBackupMediaFileNum( unsigned short _MediaFileNum )
{
	netc_TwainClientMan.setBackupMediaFileNum( _MediaFileNum );
}
void dbgsetBackupCheckMediaFileNum( unsigned short _MediaFileNum )
{
	netc_TwainClientMan.setBackupCheckMediaFileNum( _MediaFileNum );
}
void dbgsetBackupDataNum( uint32_t _DataNum )
{
	netc_TwainClientMan.setBackupDataNum( _DataNum );
}
void dbgsetBackupFileNumStatus( Bool _Status )
{
	netc_TwainClientMan.setBackupFileNumStatus( _Status );
}
void dbgsetRestoreMediaID( unsigned short _MediaID )
{
	netc_TwainClientMan.setRestoreMediaID( (char)_MediaID );
}
void dbgsetRestoreMediaFileNum( unsigned short _MediaFileNum )
{
	netc_TwainClientMan.setRestoreMediaFileNum( _MediaFileNum );
}
void dbgsetRestoreCheckMediaFileNum( unsigned short _MediaFileNum )
{
	netc_TwainClientMan.setRestoreCheckMediaFileNum( _MediaFileNum );
}
void dbgsetRestoreDataNum( uint32_t _DataNum )
{
	netc_TwainClientMan.setRestoreDataNum( _DataNum );
}
// << デバッグ関数 ---------------------------------------------------------------------------------.

//==================================================================================================.
//[機能] デバック用のJobSpecで表示する関数.
//==================================================================================================.
void NETC_TwainDebug_ShowJobSpec(const char* pc_Msg,TYPC_JobSpec* pc_JobSpec)
{
	TYPC_Common* pc_Common = pc_JobSpec->pc_Common;
	TYPC_Print*  pc_Print  = pc_JobSpec->pc_Print;
	TYPC_Scan*   pc_Scan   = pc_JobSpec->pc_Scan ;
	TYPC_Repeat*      pc_Repeat      = &(pc_Common->c_Repeat     );
	TYPC_Orientation* pc_Orientation = &(pc_Common->c_Orientation);
	TYPC_Finishing* pc_Finishing = &(pc_Print->c_Finishing);
	TYPC_PaperSize* pc_PaperSize = &(pc_Print->c_PaperSize);
	TYPC_PaperType* pc_PaperType = &(pc_Print->c_PaperType);
	TYPC_Nin1*      pc_Nin1      = &(pc_Print->c_Nin1     );
	TYPC_FeedTray*  pc_FeedTray  = &(pc_Print->c_PaperTray);
	TYPC_PageShot*  pc_PageShot  = &(pc_Scan->c_PageShot);

	NetTwainDebugTrace((char*)"### TicketSettings at %s ###\n",reinterpret_cast<intptr_t>(pc_Msg),0,0,0,0,0);
	NetTwainDebugTrace((char*)" JobType          = [%d]\n",pc_Common->c_JobType.getJobType(),0,0,0,0,0);
	NetTwainDebugTrace((char*)" sl_NumOfCopies   = [%d]\n",pc_Print->getNumOfCopies(),0,0,0,0,0);
	NetTwainDebugTrace((char*)" e_DuplexMode     = [%d]\n",pc_Print->getDuplexMode(),0,0,0,0,0);
	NetTwainDebugTrace((char*)" e_BindDirection  = [%d]\n",pc_Print->c_BindDirection.getBindDirection(),0,0,0,0,0);
	NetTwainDebugTrace((char*)" e_SortMode       = [%d]\n",pc_Print->c_SortMode.getSortMode(),0,0,0,0,0);
	NetTwainDebugTrace((char*)" e_OutputTray     = [%d]\n",pc_Print->c_OutputTray.getOutputTray(),0,0,0,0,0);
	NetTwainDebugTrace((char*)" bl_OffsetOn      = [%d]\n",pc_Finishing->getOffsetOn(),0,0,0,0,0);
	NetTwainDebugTrace((char*)" e_StapleMode     = [%d]\n",pc_Finishing->getStapleMode(),0,0,0,0,0);
	NetTwainDebugTrace((char*)" e_StaplePosition = [%d]\n",pc_Finishing->getStaplePosition(),0,0,0,0,0);
	NetTwainDebugTrace((char*)" e_FoldMode       = [%d]\n",pc_Finishing->getFoldMode(),0,0,0,0,0);
	NetTwainDebugTrace((char*)" e_FaceMode       = [%d]\n",pc_Finishing->getFaceMode(),0,0,0,0,0);
	NetTwainDebugTrace((char*)" e_PunchMode      = [%d]\n",pc_Finishing->getPunchMode(),0,0,0,0,0);
	NetTwainDebugTrace((char*)" e_PunchPosition  = [%d]\n",pc_Finishing->getPunchPosition(),0,0,0,0,0);
	NetTwainDebugTrace((char*)" e_PageShot       = [%d]\n",pc_PageShot->getPageShot(),0,0,0,0,0);
	NetTwainDebugTrace((char*)" c_PaperTray  :APS=[%d] Enable=[%d] Tray=[%d] Override=[%d]\n",
						pc_FeedTray->getApsOn(),
						pc_FeedTray->getFeedTrayEnable(),
						pc_FeedTray->getFeedTray(),
						pc_FeedTray->getOverridePaper(),
						0,0);
	NetTwainDebugTrace((char*)" c_Nin1       :Nin1=[%d] BookletOn=[%d]\n",
						pc_Nin1->getNin1(),
						pc_Nin1->getBookletOn(),
						0,0,0,0);
	NetTwainDebugTrace((char*)" c_Repeat     :On=[%d] Times=[%d] Wx=[%d] Wy=[%d] Full=[%d] Space=[%d]\n",
						pc_Repeat->getOn(),
						pc_Repeat->getRepeatTimes(),
						(int)(pc_Repeat->getRepeatWidth().x+0.5f),
						(int)(pc_Repeat->getRepeatWidth().y+0.5f),
						pc_Repeat->getFullPaperSize(),
						(int)pc_Repeat->getImageSpace());
	NetTwainDebugTrace((char*)" Wide         :On=[%d] ImagePosition=[%d]\n",
						pc_PaperSize->b_WideOn,
						pc_PaperType->getWidePaperImagePosition(),
						0,0,0,0);
	NetTwainDebugTrace((char*)" c_Orientation:Orientation=[%d] SetDirection=[%d]\n",
						pc_Orientation->getOrientation(),
						pc_Orientation->getSetDirection(),
						0,0,0,0);
	
	NetTwainDebugTrace((char*)" PerPageSet:On=[%d] Count=[%d]\n",
						pc_Finishing->getPerPageSetOn(),
						pc_Finishing->getPerPageSetCount(),
						0,0,0,0);
	for (int i=0; i<TYPD_MaxPerPageSet; ++i){
		NetTwainDebugTrace((char*)"   [%d/%d]\n",i+1,TYPD_MaxPerPageSet,0,0,0,0);
		TYPC_PerPageSet c_PerPageSet = pc_Finishing->getPerPageSet((unsigned char)(i+1));
		TYPC_FeedTray* pc_FeedTray = &(c_PerPageSet.c_FeedTray);
		NetTwainDebugTrace((char*)"     FeedTray:APS=[%d] Enable=[%d] Tray=[%d] Override=[%d]\n",
							pc_FeedTray->getApsOn(),
							pc_FeedTray->getFeedTrayEnable(),
							pc_FeedTray->getFeedTray(),
							pc_FeedTray->getOverridePaper(),
							0,0);
		NetTwainDebugTrace((char*)"     c_PerPageSet.e_DuplexMode = [%d]\n",c_PerPageSet.e_DuplexMode,0,0,0,0,0);		// iesato add 
	}
}
//==================================================================================================.
//[機能] デバック用のLeafの情報を表示する関数.
//==================================================================================================.
void NETC_TwainDebug_ShowLeaf(const char* pc_Msg,DOCC_Leaf* pc_Leaf)
{
	NetTwainDebugTrace((char*)"### LeafData at %s ###\n",reinterpret_cast<intptr_t>(pc_Msg),0,0,0,0,0);
	TYPC_PaperType c_PaperType = pc_Leaf->getPaperType();
	TYPC_PaperSize c_PaperSize = pc_Leaf->getPaperSize();
	TYPC_FeedTray  c_FeedTray  = pc_Leaf->getFeedTray();

	NetTwainDebugTrace((char*)" c_PaperType: Type=[%d] Color=[%d] Feed2nd=[%d] ImagePos=[%d]\n",
						c_PaperType.getPaperType(),
						c_PaperType.getPaperColor(),
						c_PaperType.getFeed2ndSidedPaper(),
						c_PaperType.getWidePaperImagePosition(),
						0,0);
	NetTwainDebugTrace((char*)" c_PaperSize: Code=[%d] x=[%d]x0.1 y=[%d]x0.1 WideOn=[%d]\n",
						c_PaperSize.e_SizeCode,
						(int)(c_PaperSize.c_SizeMM.x * 10.0f + 0.5f),
						(int)(c_PaperSize.c_SizeMM.y * 10.0f + 0.5f),
						c_PaperSize.b_WideOn,
						0,0);
	NetTwainDebugTrace((char*)" c_FeedTray: APS=[%d] Tray=[%d] Enable=[%d] Override=[%d]\n",
						c_FeedTray.getApsOn(),
						c_FeedTray.getFeedTray(),
						c_FeedTray.getFeedTrayEnable(),
						c_FeedTray.getOverridePaper(),
						0,0);

	IMGC_ImageInfo*    pc_ImageInfo    = &(pc_Leaf->getImage()->c_ImageInfo);
	TYPC_PaperSize*    pc_DocSize      = &(pc_ImageInfo->ct_DocSize);
	TYPC_PaperSize*    pc_ImageSize    = &(pc_ImageInfo->ct_ImageSize); //mm
	//>>Moselからユニバーサル対応のため、画像サイズ（dot）の取得方法が変更になったので、対応 06/06/22 yamahata 
	TYPC_Point<short>  pc_ImageDotSize = pc_ImageInfo->getImageSizeDot(); //dot
	//<<Moselからユニバーサル対応のため、画像サイズ（dot）の取得方法が変更になったので、対応 06/06/22 yamahata 
	TYPC_Resolution*   pc_Resolution   = &(pc_ImageInfo->c_Resolution);
	TYPC_Point<float>  c_Dpi = pc_Resolution->getDpi();

	NetTwainDebugTrace((char*)"   c_ImageInfo: ImageDir=[%d] DocLP=[%d] Color=[%d]\n",
						(int)(pc_ImageInfo->e_ImageDir),
						(int)(pc_ImageInfo->e_DocLP),
						(int)(pc_ImageInfo->ec_ColorMode),
						0,0,0);
	NetTwainDebugTrace((char*)"     ct_DocSize: X=[%d]x0.1 Y=[%d]x0.1 WideOn=[%d]\n",
						(int)(pc_DocSize->c_SizeMM.x * 10.0f + 0.5f),
						(int)(pc_DocSize->c_SizeMM.y * 10.0f + 0.5f),
						pc_DocSize->b_WideOn,
						0,0,0);
	NetTwainDebugTrace((char*)"     ct_ImageSize: X=[%d]x0.1 Y=[%d]x0.1 WideOn=[%d]\n",
						(int)(pc_ImageSize->c_SizeMM.x * 10.0f + 0.5f),
						(int)(pc_ImageSize->c_SizeMM.y * 10.0f + 0.5f),
						pc_ImageSize->b_WideOn,
						0,0,0);
	NetTwainDebugTrace((char*)"     getImageSizeDot(): X=[%d]x0.1 Y=[%d]x0.1\n",
						(int)((float)pc_ImageDotSize.x * 10.0f + 0.5f),
						(int)((float)pc_ImageDotSize.y * 10.0f + 0.5f),
						0,0,0,0);
	NetTwainDebugTrace((char*)"     c_Dpi: X=[%d] Y=[%d]\n",
						(int)(c_Dpi.x + 0.5f),
						(int)(c_Dpi.y + 0.5f),
						0,0,0,0);
}

//==================================================================================================
//[機能] JobList通知状態の設定 
//[履歴]2004.02.10 Matsumoto
//==================================================================================================
int NETC_TwainDecideActJobListObserverTypeBit(TYPE_JobType e_JobType,TYPE_SendMode e_SendMode,TYPC_DirectBoxAddress* pc_DirectBoxAddress, APIC_Job* cJob )



{
	switch (e_JobType)
	{
	case TYPE_JobTypeCopy:				// コピー	
	case TYPE_JobTypeIntCopy:			// 割込コピー	
	case TYPE_JobTypePrint:				// プリント	
	case TYPE_JobTypeSecurePrint:		// 機密文書プリント	
	case TYPE_JobTypePdl:				// ＰＤＬ（ＲＩＰ処理前）
		if ((e_SendMode != TYPE_SendModeBox)){						// 配信先がBoxではない場合
NetTwainDebugTrace((char*)"#### NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_Print !!!!!\n", 0,0,0,0,0,0);
			return NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_Print;
		}
		if ( pc_DirectBoxAddress->getOn() == True ) {		// Ｂｏｘ宛先 
NetTwainDebugTrace((char*)"#### pc_DirectBoxAddress->getOn() == True !!!!!\n", 0,0,0,0,0,0);
			if ( pc_DirectBoxAddress->getPrintOn() == False ) {			// プリント出力をしない 





NetTwainDebugTrace((char*)"#### NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_HddSave !!!!!\n", 0,0,0,0,0,0);
				return NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_HddSave;
			} else {													// Box保存＋プリント出力する 
NetTwainDebugTrace((char*)"#### OBSVRTYPBIT_HddSave and OBSVRTYPBIT_Print !!!!!\n", 0,0,0,0,0,0);
				return ( NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_HddSave | NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_Print );
			}
		} else {											// Box保存以外の場合 
NetTwainDebugTrace((char*)"#### pc_DirectBoxAddress->getOn() == False!!!!!\n", 0,0,0,0,0,0);



NetTwainDebugTrace((char*)"#### NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_Print !!!!!\n", 0,0,0,0,0,0);
			return NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_Print;
		}
	case TYPE_JobTypeScan:				// スキャン	
	case TYPE_JobTypeTwain:				// Ｔｗａｉｎ	
		if ((e_SendMode == TYPE_SendModeBox)){
			return NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_HddSave;			// Box保存の時は NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_HddSave を返す 
		}
		return NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_Send;
	case TYPE_JobTypeFaxSendQuick:		// ＦＡＸ（クイックメモリ送信）
	case TYPE_JobTypeFaxSendMemory:		// ＦＡＸ（メモリ送信）
	case TYPE_JobTypeFaxSendTimer:		// ＦＡＸ(タイマ送信）
	case TYPE_JobTypeFaxReceivePolling:	// ＦＡＸ（ポーリング送信）
		if ( e_SendMode == TYPE_SendModeUsbMem )  {							// Scan To USBは保存ジョブ 
			return (NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_HddSave);
		}
		if (e_SendMode == TYPE_SendModeWebDav ) {							// Scan To WebDavは保存+送信 
			return (NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_HddSave + NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_Send);
		}
		// #45184 対応 
		if ( (e_SendMode == TYPE_SendModeEmail) || (e_SendMode == TYPE_SendModeFtp) || (e_SendMode == TYPE_SendModeSmb) )  {
			// FaxモードでE-mail or FTP or SMB の場合は、スキャナ送信のActive Job になる。 
			return (NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_HddSave + NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_Send);
		}
		// #41287対策（FAX転送やPC－FAXで送信後に印刷する場合があるため） 
		if ( cJob->getPrintAfterSend() ) {							// 送信後プリントする場合  



			return (NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_Send + NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_Print);
		}
		return (NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_Send + NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_HddSave);
	case TYPE_JobTypeFaxReceive:			// ＦＡＸ受信プリント
	case TYPE_JobTypeIfaxReceive:		// ＩＦａｘ受信
	case TYPE_JobTypeFaxReceiveTimer:	// ＦＡＸ（タイマーポーリング受信）
		if ((e_SendMode == TYPE_SendModeBox)){
			return NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_HddSave;			// Box保存の時は NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_HddSave を返す 
		}
		return (NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_FaxRecv + NETE_TWAIN_ACTJOBLSTOBSVRTYPBIT_Print);
	}
	//ジョブタイプが不正な場合は、アクティブジョブを要求している全クライアントに通知する。
	return 0xFF; //set all bit.
}

//==================================================================================================
//[機能]Send後のCallBackAnswer処理
void CallBackAnswer(int , int32_t )
{
}

//==================================================================================================
//[機能]リストプリント実行時のコールバック
//==================================================================================================.
void ans_cancel(APIC_Job* )
{
}

//==================================================================================================.
//[機能]msec⇒tick数への変換処理.
//==================================================================================================.
int GetTicksPerMs(int si_milisec)
{
	return (int)(((int32_t)SYSF_sysClkRateGet() * si_milisec) / 1000);
}

//==================================================================================================.
//[機能]Systemがカラー機か否かを判断する.
//[戻り値] True=カラー機, False=モノクロ機.
//[履歴] 20050222 arisawa - Added to fix BugNo.24192.
//       20090527 sone 管理No.61:モノクロ機の場合は、ColorSystemをtrueに設定する。.
//==================================================================================================.
Bool NETC_Twain_isColorSystem()
{
	if (NETC_TwainDebug_siSystemColor != -1){
		return (NETC_TwainDebug_siSystemColor != 0);
	}
// 2009/05/27 sone Taiga 管理No.61 制御構成変更23 PCプリントドライバモノクロ対応 str.
#ifdef DEF_MonoCus
	return True;
#else	// DEF_MonoCus
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	pc_SystemInfo->lock();
	Bool bl_ColorSystem = pc_SystemInfo->isColorOrMonoMachine();
	pc_SystemInfo->unlock();
	return bl_ColorSystem;
#endif	// DEF_MonoCus
// 2009/05/27 sone Taiga 管理No.61 制御構成変更23 PCプリントドライバモノクロ対応 end.
}

//==================================================================================================
//[機能]中綴じが紙折りメニューに実装されるか否かを判断する.
//[戻り値] True=紙折りメニューに実装される, False=紙折りメニューに実装されない.
//[履歴] 20050317 arisawa - Added to fix BugNo.34864.
//==================================================================================================.
Bool NETC_Twain_isCenterStapleInFolds()
{
	if (NETC_TwainDebug_siCenterStapleInFolds != -1){
		return (NETC_TwainDebug_siCenterStapleInFolds != 0);
	}
	return (NETC_Twain_isColorSystem() == False);
}

//==================================================================================================
//[機能]SystemにRepeat機能があるか否かを判断する.
//[戻り値] True=Repeat機能有り, False=Repeat機能無し.
//[コメント]JobTicket変更のサポート有無とは異なる.
//[履歴] 20050319 arisawa - Added to fix BugNo.35423.
//==================================================================================================.
Bool NETC_Twain_isRepeatSupportedSystem()
{
	return (NETC_Twain_isColorSystem() == False);
}

//==================================================================================================
//[機能]SystemにPrintPosition機能があるか否かを判断する.
//[戻り値] True=PrintPosition機能有り, False=PrintPosition機能無し.
//[コメント]JobTicket変更のサポート有無とは異なる.
//[履歴] 20050319 arisawa - Added to fix BugNo.35505.
//==================================================================================================.
Bool NETC_Twain_isPrintPositionSupportedSystem()
{
	return (NETC_Twain_isColorSystem() == False);
}

//==================================================================================================
//[機能]HDDの装着状況を確認する.
//[戻り値] TYPE_HddStatus.
//[履歴] 2004.01.07 Matsumoto 作成.
//==================================================================================================.
TYPE_HddStatus getHDDStatus()
{
	APIC_HddInfo* pc_HddInfo;
	pc_HddInfo = APIC_HddInfo::newInstance();
	TYPE_HddStatus type_HddStatus;
	pc_HddInfo->lock();
	type_HddStatus = pc_HddInfo->getHddStatus();
	pc_HddInfo->unlock();
	return type_HddStatus;
}

//==================================================================================================
//[機能]HDDの装着状況を確認する.
//[戻り値] Bool.
//[履歴] 2004.07.15 Matsumoto 作成.
//==================================================================================================.
Bool isHDDExist()
{
	APIC_HddInfo* pc_HddInfo;
	pc_HddInfo = APIC_HddInfo::newInstance();
	Bool isHDDExist = False;
	pc_HddInfo->lock();
	isHDDExist = pc_HddInfo->getHddExist();
	pc_HddInfo->unlock();
	return isHDDExist;
}

//==================================================================================================
//[機能]HDDの装着状況を確認する.
//[戻り値] Bool.
//[履歴] 2004.11.16 Matsumoto 作成 
//		 2009/04/22 hiratani Taiga 制御構成変更91対応.
//==================================================================================================.
Bool isHDDAccessCheck()
{

//>> 2009/04/22 hiratani Taiga 制御構成変更91対応.
#if defined(DEF_MonoCus) || defined(DEF_SupportSSD)				// 20101117 amiya 【Minerva】制御構成変更141対応.
	// Taiga且つHDDあり(isHDDExist() == True)の場合は、getHDDStatus()はHDDの状態を返却する.
	// Taiga且つHDDなし(isHDDExist() == False)の場合は、getHDDStatus()はSSDの状態を返却する.
	// ※TaigaではSDDが付いていない状態はありえないので、isHDDExist()をコールする必要がなくなった.

// CQ320371 2009/09/02 okabe Start.
	// Faxのありなしを調べる.
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	Bool b_IsFaxExist = True;
	pc_SystemInfo->lock();

	//2013/11/05 [Zeus5BK]OPS1前田 CQ383567 DipSW25 0x00対応.
	if( ( pc_SystemInfo->getFaxExistAll(TYPE_CommLine1) == False )
	 && ( pc_SystemInfo->getFaxExistAll(TYPE_CommLine2) == False )
	 && ( pc_SystemInfo->getFaxExistAll(TYPE_CommLine3) == False ) 
	 && ( pc_SystemInfo->getFaxExistAll(TYPE_CommLine4) == False ) )
	{
		b_IsFaxExist = False;
	}

	pc_SystemInfo->unlock();
// CQ320371 2009/09/02 okabe End.
	// HDD/SSDの状態取得.
	TYPE_HddStatus e_HddStatus = getHDDStatus();
	if ( e_HddStatus == TYPE_HddStatusNon					// ＨＤＤ無し.
	  || e_HddStatus == TYPE_HddStatusConnecting			// 接続中.
	  || e_HddStatus == TYPE_HddStatusNonFormat				// 未フォーマット.
	  || ( b_IsFaxExist == False && isHDDExist() == False )	// CQ320371 2009/09/02 okabe 追加. // CQNO320848 2009/09/18 okabe Fax未装着かつHDD非装着時に条件文を変更.
	) {
		return False;
	}
	
	return True;
	
#else	// DEF_MonoCus || DEF_SupportSSD.
	
	if ( isHDDExist() == False ) {	// HDD非装着状態 
		return False;
	} else {
		TYPE_HddStatus e_HddStatus = getHDDStatus();
		if ( e_HddStatus == TYPE_HddStatusNon			// ＨＤＤ無し 
		  || e_HddStatus == TYPE_HddStatusConnecting	// 接続中 
		  || e_HddStatus == TYPE_HddStatusNonFormat		// 未フォーマット 
		) {
			return False;
		}
	}
	return True;
	
#endif	// DEF_MonoCus || DEF_SupportSSD.
//<< 2009/04/22 hiratani Taiga 制御構成変更91対応.
}

//==================================================================================================.
//[機能] HDDの装着有無とSSDのサイズを確認してBox機能が使用できるか判断する.
//[戻り値] NETE_Function_Box.
//[履歴] 2011/03/31 amiya 【Minerva】管理No941 SSD削減対応（1GB⇒512MB）.
//       2018/04/23 H.Tokuno Eagle 制御構成変更_No.99 getSsdSize()の削除対応.
//==================================================================================================.
NETE_Function_Box isCheckAvailableBox()
{
	Bool bl_HDDExist = isHDDExist();						// SSDの装着.
	TYPE_HddStatus e_HddStatus = getHDDStatus();			// SSDの状態.
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	pc_SystemInfo->lock();
	TYPE_StorageConfig e_StrageConfig = pc_SystemInfo->isStorageConfiguration();		// 装着されたストレージの状況.
	NETE_Function_Box e_Function_Box = NETE_Function_Box_Disabled;			// 戻り値格納用変数.
	Bool b_IsFaxExist = True;
	
	if( e_StrageConfig != TYPE_StorageConfig_None ) {
		// ストレージ構成情報取得成功.
		if( bl_HDDExist == True ) {		// SSDあり.
			if( ( e_HddStatus == TYPE_HddStatusNon )			// SSD無し.
			 || ( e_HddStatus == TYPE_HddStatusConnecting )		// 接続中.
			 || ( e_HddStatus == TYPE_HddStatusNonFormat ) ) {	// 未フォーマット.
				e_Function_Box = NETE_Function_Box_Disabled;							// 不可.
			} else {
				e_Function_Box = NETE_Function_Box_Available;							// 可能.
			}
		} else {						// μSDあり.
			// Taiga/DonauBK同様の対応.
			// BOX所有者変更/文書の削除⇒可能.
			// BOX間 蓄積文書コピー&移動/文書のリネーム⇒HDD非装着時はUIによる.

			//2013/11/05 [Zeus5BK]OPS1前田 CQ383567 DipSW25 0x00対応.
			if( ( pc_SystemInfo->getFaxExistAll(TYPE_CommLine1) == False )
			 && ( pc_SystemInfo->getFaxExistAll(TYPE_CommLine2) == False )
			 && ( pc_SystemInfo->getFaxExistAll(TYPE_CommLine3) == False ) 
			 && ( pc_SystemInfo->getFaxExistAll(TYPE_CommLine4) == False ) )
			{
				b_IsFaxExist = False;
			} else {		// FAXが装着されている.
NetTwainDebugTrace((char*)"### Fax1=[%d]/Fax2=[%d]/Fax3=[%d]/Fax4=[%d] ###\n",pc_SystemInfo->getFaxExistAll(TYPE_CommLine1),pc_SystemInfo->getFaxExistAll(TYPE_CommLine2),pc_SystemInfo->getFaxExistAll(TYPE_CommLine3), pc_SystemInfo->getFaxExistAll(TYPE_CommLine4),0,0);
			}
			// FAXの有無チェック.
			if ( ( e_HddStatus == TYPE_HddStatusNon )			// μSD無し.
			  || ( e_HddStatus == TYPE_HddStatusConnecting )	// 接続中.
			  || ( e_HddStatus == TYPE_HddStatusNonFormat )		// 未フォーマット.
			  || ( b_IsFaxExist == False ) ) {					// Fax未装着.
				e_Function_Box = NETE_Function_Box_Disabled;
			} else {
				e_Function_Box = NETE_Function_Box_SomePossible;
			}
		}
	} else {
		// ストレージ構成情報取得エラー.
		e_Function_Box = NETE_Function_Box_Disabled;	// 不可.
	}
	pc_SystemInfo->unlock();

NetTwainDebugTrace((char*)"### isCheckAvailableBox() e_Function_Box = [%d] ###\n",e_Function_Box,0,0,0,0,0);
	return e_Function_Box;
}
//==================================================================================================.
//[機能] HDDの装着有無とSSDのサイズを確認してScan機能が使用できるか判断する.
//[戻り値] NETE_Function_Scan.
//[履歴] 2011/03/31 amiya 【Minerva】管理No941 SSD削減対応（1GB⇒512MB）.
//       2018/04/23 H.Tokuno Eagle 制御構成変更_No.99 getSsdSize()の削除対応.
//==================================================================================================.
NETE_Function_Scan isCheckAvailableScan()
{
	TYPE_HddStatus e_HddStatus = getHDDStatus();			// SSDの状態.
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	pc_SystemInfo->lock();
	TYPE_StorageConfig e_StrageConfig = pc_SystemInfo->isStorageConfiguration();		// 装着されたストレージの状況.
	pc_SystemInfo->unlock();
	NETE_Function_Scan e_Function_Scan = NETE_Function_Scan_Disabled;			// 戻り値格納用変数.

	if( e_StrageConfig != TYPE_StorageConfig_None ) {
		// ストレージ構成情報取得成功.
		// IT6ではSSD,μSD共に1Gを下回ることが無く、サイズでのチェックは不要.
		// それにより、SSDとμSDで判定処理の差が無くなった.
		if( ( e_HddStatus == TYPE_HddStatusNon )			// SSD/μSD無し.
		 || ( e_HddStatus == TYPE_HddStatusConnecting )		// 接続中.
		 || ( e_HddStatus == TYPE_HddStatusNonFormat ) ) {	// 未フォーマット.
			e_Function_Scan = NETE_Function_Scan_Disabled;							// 不可.
		} else {
			e_Function_Scan = NETE_Function_Scan_Available;							// 可能.
		}
	} else {
		// ストレージ構成情報取得エラー.
		e_Function_Scan = NETE_Function_Scan_Disabled;	// 不可.
	}

NetTwainDebugTrace((char*)"### isCheckAvailableScan() e_Function_Scan = [%d] ###\n",e_Function_Scan,0,0,0,0,0);
	return e_Function_Scan;
}

//==================================================================================================
//[機能]ユーザー認証モードを確認する.
//[戻り値] TYPE_UserAuthentication.
//[履歴] 2004.01.07 Matsumoto 作成.
//==================================================================================================.
TYPE_UserAuthentication getUserAuthenticationMode()
{
	// ユーザー認証インスタンスを取得する 
	APIC_UserAuthentication* pc_UserAuthentication;
	pc_UserAuthentication = APIC_UserAuthentication::newInstance();
	TYPE_UserAuthentication type_UserAuthentication;
	pc_UserAuthentication->lock();
	// ユーザー認証設定状態を取得する 
	type_UserAuthentication = pc_UserAuthentication->getUserAuthentication();
	pc_UserAuthentication->unlock();
	return type_UserAuthentication;
}

//==================================================================================================
//[機能]部門認証モードを確認する 
//[戻り値] type_TrackMode
//[履歴] 2004.01.27 Iesato 作成 
//==================================================================================================.
TYPE_TrackMode getTrackMode()
{
	// 部門管理インスタンスを取得する 
	APIC_TrackManager* pc_TrackManager;
	pc_TrackManager = APIC_TrackManager::newInstance();
	TYPE_TrackMode type_TrackMode;						// 部門管理モード 
	pc_TrackManager->lock();							// セマフォ取得 
	// 部門認証モードを取得する 
	type_TrackMode = pc_TrackManager->getTrackMode();	//部門管理モードを取得 
	pc_TrackManager->unlock();						// セマフォ解放 
	return type_TrackMode;
}

//==================================================================================================
//[機能]ログイン状態を確認する 
//[戻り値] Bool True:ログインが済んでいる／False:ログインが済んでいない 
//[履歴] 2006.03.13 Matsumoto 作成 
//[履歴] 2006.07.22 Iesato #65061対策 Box管理者の状態確認処理を削除 
//==================================================================================================
Bool getLoginStatusCheck(
	Bool*			bl_UserLogined,
	APIC_User**		pc_User,
	Bool*			bl_TrackLogined,
	APIC_Track**	pc_Track,
	int				SocketFd
)
{
	Bool bl_Ret = False;		// ログイン済み状態 

	// 2014/08/25 [ZeusS][制御構成変更No.50]OPS1前田 Start.
	// 認証パターンA：(ユーザー認証：OFF ／ 部門認証：OFF) 
	if ( ( (getUserAuthenticationMode() == TYPE_UserAuthenticationOff) && (getTrackMode() == TYPE_TrackModeOff) )
	   ||( True == isFieryCalibrationCheck( SocketFd ) ) )
	// 2014/08/25 [ZeusS][制御構成変更No.50]OPS1前田 End.
	{
NetTwainDebugTrace((char*)"### getLoginStatusCheck_Authentication Pattern A ###\n",0,0,0,0,0,0);
		bl_Ret = True;
		*bl_UserLogined = True;
		*pc_User = netc_TwainClientMan.getApic_User( SocketFd );
		*bl_TrackLogined = False;
		*pc_Track = NULL;
	}
	// 認証パターンB：(ユーザー認証：ON  ／ 部門認証：OFF) 
// ▼ START ▼ CQ376645 S12-r_mori 2012/12/27 管理No68_本体装置認証（部門管理あり）+拡張サーバ認証管理する場合はＰＣ側でRTM Twain実施すると受信できない、エラーメッセージが表示される 
	else if ( (getUserAuthenticationMode() != TYPE_UserAuthenticationOff)
			&& ((getTrackMode() == TYPE_TrackModeOff) || ( NULL != netc_TwainClientMan.getResExtParameter( SocketFd ))) )
// ▲  END  ▲ CQ376645 S12-r_mori 2012/12/27 管理No68_本体装置認証（部門管理あり）+拡張サーバ認証管理する場合はＰＣ側でRTM Twain実施すると受信できない、エラーメッセージが表示される 
	{
NetTwainDebugTrace((char*)"### getLoginStatusCheck_Authentication Pattern B ###\n",0,0,0,0,0,0);
		if ( netc_TwainClientMan.getUserLogined( SocketFd ) == True )
		{
			bl_Ret = True;
			*bl_UserLogined = True;
			*pc_User = netc_TwainClientMan.getApic_User( SocketFd );
			*bl_TrackLogined = False;
			*pc_Track = NULL;
		}
	}
	// 認証パターンC：(ユーザー認証：OFF ／ 部門認証：ON ) 
	else if ( (getUserAuthenticationMode() == TYPE_UserAuthenticationOff) && (getTrackMode() != TYPE_TrackModeOff) )
	{
		// 2008.08.26 Donau問題点対応 CC 307769 平谷 str.
		if ( netc_TwainClientMan.getAccountLogined( SocketFd ) == True )
		{
NetTwainDebugTrace((char*)"### getLoginStatusCheck_Authentication Pattern C(general) ###\n",0,0,0,0,0,0);
			bl_Ret = True;
			*bl_UserLogined = False;
			*pc_User = netc_TwainClientMan.getApic_User( SocketFd );
			*bl_TrackLogined = True;
			*pc_Track = netc_TwainClientMan.getApic_Track( SocketFd );
		}
		// 管理者ログインの場合.
		// 管理者の場合はユーザーのインスタンスも返す必要がある為.
		if ( ( NULL != netc_TwainClientMan.getApic_Admin( SocketFd ) )
		  && ( True == netc_TwainClientMan.getUserLogined( SocketFd ) ) )
		{
NetTwainDebugTrace((char*)"### getLoginStatusCheck_Authentication Pattern C(Admin) ###\n",0,0,0,0,0,0);
			bl_Ret = True;
			*bl_UserLogined = True;
			*pc_User = netc_TwainClientMan.getApic_Admin( SocketFd );
			*bl_TrackLogined = True;
			*pc_Track = netc_TwainClientMan.getApic_Track( SocketFd );
		}
		// 2008.08.26 Donau問題点対応 CC 307769 平谷 end.
	}
	// 認証パターンD：(ユーザー認証：ON  ／ 部門認証：ON ) 
	else if ( (getUserAuthenticationMode() != TYPE_UserAuthenticationOff) && (getTrackMode() != TYPE_TrackModeOff) )
	{
		if ( (netc_TwainClientMan.getUserLogined( SocketFd ) == True) && (netc_TwainClientMan.getAccountLogined( SocketFd ) == True) )
		{
NetTwainDebugTrace((char*)"### getLoginStatusCheck_Authentication Pattern D(general) ###\n",0,0,0,0,0,0);
			bl_Ret = True;
			*bl_UserLogined = True;
			*pc_User = netc_TwainClientMan.getApic_User( SocketFd );
			*bl_TrackLogined = True;
			*pc_Track = netc_TwainClientMan.getApic_Track( SocketFd );
		}
	}
	return bl_Ret;
}

//==================================================================================================
//[機能] 監査ログ設定 ユーザ認証エラー 
//[履歴] 2006.12.14 Harada 作成 
//		 2008.04.18 Tana   [Donau] No.86 監査ログ→ジョブログに変更.
//==================================================================================================.
void setUserLoginErrLog( 
	APIC_User* pc_User,					// ユーザインスタンス 
	char* psc_UserName,					// ユーザ名 
	TYPE_UserResult sc_UserResult		// ユーザ結果 
)
{
	APIC_SystemSetting* pc_SystemSetting = NULL;
	pc_SystemSetting = APIC_SystemSetting::newInstance();

	// ジョブログ出力の可／不可.
	JLGE_InspectLog e_OperateCode;							// 操作コード.
	// 管理者モード.
	// TCPSocketでは、ユーザー名：Adminは管理者モードの扱いのみ.
	if ( strcmp( psc_UserName, TYPD_UserName_Admin ) == 0 )
	{
		e_OperateCode = JLGE_InspectLogAdminAuthentication;
	}
	// ユーザーモード.
	else
	{
		e_OperateCode = JLGE_InspectLogUserAuthentication;
	}
	pc_SystemSetting->lock();
	Bool bl_isEnableInspectLog = pc_SystemSetting->getJobLogInspectLogOn();
	pc_SystemSetting->unlock();

	if ( bl_isEnableInspectLog == True )
	{
		JLGS_Time s_OperateTime = {0};							// 操作日時.
		ushort us_ResultCode = JLGE_InspectLogResultOK;		// 結果コード.
		uint32_t ul_UserCode = JLGE_InspectLogOperateIDL_Non;	// 操作者コード.
		char sc_UserName[JLGD_DataLenUserName];				// 操作者名.
		memset(sc_UserName, 0, sizeof(sc_UserName));
		char sc_ExtServerName[JLGD_DataLenExtSvName];		// 外部サーバー名.
		memset(sc_ExtServerName, 0, sizeof(sc_ExtServerName));
		uint32_t ul_TrackCode = JLGE_InspectLogTrackIDL_Non;	// 部門コード.	// 2008.06.09 Harada Donau対応 定義変更.
		char sc_TrackName[JLGD_DataLenTrackName];			// 部門名.
		memset(sc_TrackName, 0, sizeof(sc_TrackName));
		uchar uc_CardId[JLGD_DataLenCardID];				// カードID.
		memset(uc_CardId, 0, sizeof(uc_CardId));
		char sc_ClientInfo[JLGD_DataLenClientInfo];			// 操作クライアント情報.
		memset(sc_ClientInfo, 0, sizeof(sc_ClientInfo));

		// 操作日時.
		pc_SystemSetting->lock();
		TYPC_Time c_Time;
		c_Time = pc_SystemSetting->getSystemTime();
		s_OperateTime.ss_Year   = c_Time.ss_Year;
		s_OperateTime.sc_Month  = c_Time.sc_Month;
		s_OperateTime.sc_Day    = c_Time.sc_Day;
		s_OperateTime.sc_Hour   = c_Time.sc_Hour;
		s_OperateTime.sc_Minute = c_Time.sc_Minute;
		s_OperateTime.sc_Second = c_Time.sc_Second;
		pc_SystemSetting->unlock();

		// 結果コード.
		switch( sc_UserResult ) {
		case TYPE_UserResultPenalty:		// ペナルティロック.
			us_ResultCode = JLGE_InspectLogResultPenaltylockAuthenticationNG;
			break;
		default:							// その他.
			us_ResultCode = JLGE_InspectLogResultAuthenticationNG;
			break;
		}

		if ( pc_User )
		{
			if ( e_OperateCode == JLGE_InspectLogUserAuthentication )
			{
				pc_User->lock();
				// 操作者コード.
				int32_t sl_UserNo = pc_User->getUserNo();	//2013/10/11 【スケーラブルFUM】[CQ383174] 20万人対応. 
				pc_User->unlock();
				// 一般ユーザー.
				if ( sl_UserNo <= TYPD_MidAuthServerUserIdMax )
				{
					ul_UserCode = JLGE_InspectLogOperateIDL_User | sl_UserNo;
				}
				// Box管理者.
				else if ( sl_UserNo == TYPD_IndexUserBoxAdmin )
				{
					ul_UserCode = JLGE_InspectLogOperateIDL_BoxAdmin;
				}
				// Publicユーザー.
				else if ( sl_UserNo == TYPD_IndexUserPublic )
				{
					ul_UserCode = JLGE_InspectLogOperateIDL_PublicUser;
				}
				// ログ管理者.
				else if ( sl_UserNo == TYPD_IndexUserLMU )
				{
					ul_UserCode = JLGE_InspectLogOperateIDL_LogAdmin;
				}
			}
		}

		// ジョブログ登録.
		JLGS_OpeLogCom s_OpeLogCom;							// ジョブログ登録(操作系)(共通).
		s_OpeLogCom.ul_Code = e_OperateCode;
		s_OpeLogCom.s_Time = s_OperateTime;
		s_OpeLogCom.us_Result = us_ResultCode;
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	Start.
		s_OpeLogCom.us_EventType = getEventTypeByResultCode( s_OpeLogCom.us_Result );
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	End.
		s_OpeLogCom.ul_OpeCode = ul_UserCode;
		strcpy(s_OpeLogCom.sc_Name, sc_UserName);
		strcpy(s_OpeLogCom.sc_ExtServerName, sc_ExtServerName);
		s_OpeLogCom.ul_TrackCode = ul_TrackCode;
		strcpy(s_OpeLogCom.sc_TrackName, sc_TrackName);
		memcpy(s_OpeLogCom.uc_CardID, uc_CardId, sizeof(uc_CardId));
		s_OpeLogCom.uc_IF = JLGE_InspectLogIfTCPSocket;
		strcpy(s_OpeLogCom.sc_ClientInfo, sc_ClientInfo);

		JLGS_OpeLogUserAuth s_OpeLogUserAuth;				// ジョブログ登録(操作系)(対象ユーザー情報).
		s_OpeLogUserAuth.ul_UserID = ul_UserCode;
		strcpy(s_OpeLogUserAuth.sc_UserName, sc_UserName);
		strcpy(s_OpeLogUserAuth.sc_RelationExtServerName, sc_ExtServerName);

		APIC_JLG* pc_JobLog = NULL;
		pc_JobLog = APIC_JLG::newInstance();
		pc_JobLog->addOpeLogUserAuthentication(&s_OpeLogCom, &s_OpeLogUserAuth, NULL);
	}
}

//==================================================================================================
//[機能] 監査ログ設定 部門認証エラー 
//[履歴] 2006.12.14 Harada 作成 
//		 2008.04.18 Tana   [Donau] No.86 監査ログ→ジョブログに変更.
//==================================================================================================
void setTrackLoginErrLog( 
	APIC_Track* pc_Track,					// 部門インスタンス 
	char* psc_TrackName,					// 部門名 
	TYPE_TrackResult sc_TrackResult			// 部門結果 
)
{
	APIC_SystemSetting* pc_SystemSetting = NULL;
	pc_SystemSetting = APIC_SystemSetting::newInstance();

	// ジョブログ出力の可／不可.
	JLGE_InspectLog e_OperateCode;							// 操作コード.
	e_OperateCode = JLGE_InspectLogTrackAuthentication;
	pc_SystemSetting->lock();
	Bool bl_isEnableInspectLog = pc_SystemSetting->getJobLogInspectLogOn();
	pc_SystemSetting->unlock();

	if ( bl_isEnableInspectLog == True )
	{
		JLGS_Time s_OperateTime = {0};							// 操作日時.
		ushort us_ResultCode = JLGE_InspectLogResultOK;		// 結果コード.
		uint32_t ul_UserCode = JLGE_InspectLogOperateIDL_Non;	// 操作者コード.
		char sc_UserName[JLGD_DataLenUserName];				// 操作者名.
		memset(sc_UserName, 0, sizeof(sc_UserName));
		char sc_ExtServerName[JLGD_DataLenExtSvName];		// 外部サーバー名.
		memset(sc_ExtServerName, 0, sizeof(sc_ExtServerName));
		uint32_t ul_TrackCode = JLGE_InspectLogTrackIDL_Non;	// 部門コード.	// 2008.06.09 Harada Donau対応 定義変更.
		char sc_TrackName[JLGD_DataLenTrackName];			// 部門名.
		memset(sc_TrackName, 0, sizeof(sc_TrackName));
		uchar uc_CardId[JLGD_DataLenCardID];				// カードID.
		memset(uc_CardId, 0, sizeof(uc_CardId));
		char sc_ClientInfo[JLGD_DataLenClientInfo];			// 操作クライアント情報.
		memset(sc_ClientInfo, 0, sizeof(sc_ClientInfo));

		// 操作日時.
		pc_SystemSetting->lock();
		TYPC_Time c_Time;
		c_Time = pc_SystemSetting->getSystemTime();
		s_OperateTime.ss_Year   = c_Time.ss_Year;
		s_OperateTime.sc_Month  = c_Time.sc_Month;
		s_OperateTime.sc_Day    = c_Time.sc_Day;
		s_OperateTime.sc_Hour   = c_Time.sc_Hour;
		s_OperateTime.sc_Minute = c_Time.sc_Minute;
		s_OperateTime.sc_Second = c_Time.sc_Second;
		pc_SystemSetting->unlock();

		// 結果コード.
		switch( sc_TrackResult ) {
		case TYPE_TrackResultPenalty:			// ペナルティロック.
			us_ResultCode = JLGE_InspectLogResultPenaltylockAuthenticationNG;
			break;
		default:								// その他.
			us_ResultCode = JLGE_InspectLogResultAuthenticationNG;
			break;
		}

		if ( pc_Track )
		{
			pc_Track->lock();
			// 部門コード.
			int32_t sl_TrackNo = pc_Track->getTrackNo();	//2013/10/11 【スケーラブルFUM】[CQ383174] 20万人対応. 
			pc_Track->unlock();
			// 一般ユーザー.
			if ( sl_TrackNo <= TYPD_MidAuthServerUserIdMax )
			{
				ul_TrackCode = JLGE_InspectLogTrackIDL_Track | sl_TrackNo;	// 2008.06.09 Harada Donau対応 定義変更.
			}
			// Box管理者.
			else if ( sl_TrackNo == TYPD_IndexTrackBoxAdmin )
			{
				ul_TrackCode = JLGE_InspectLogTrackIDL_BoxAdmin;	// 2008.06.09 Harada Donau対応 定義変更.
			}
			// Publicユーザー.
			else if ( sl_TrackNo == TYPD_IndexTrackPublic )
			{
				ul_TrackCode = JLGE_InspectLogTrackIDL_PublicUser;	// 2008.06.09 Harada Donau対応 定義変更.
			}
		}

		// ジョブログ登録.
		JLGS_OpeLogCom s_OpeLogCom;							// ジョブログ登録(操作系)(共通).
		s_OpeLogCom.ul_Code = e_OperateCode;
		s_OpeLogCom.s_Time = s_OperateTime;
		s_OpeLogCom.us_Result = us_ResultCode;
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	Start.
		s_OpeLogCom.us_EventType = getEventTypeByResultCode( s_OpeLogCom.us_Result );
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	End.
		s_OpeLogCom.ul_OpeCode = ul_UserCode;
		strcpy(s_OpeLogCom.sc_Name, sc_UserName);
		strcpy(s_OpeLogCom.sc_ExtServerName, sc_ExtServerName);
		s_OpeLogCom.ul_TrackCode = ul_TrackCode;
		strcpy(s_OpeLogCom.sc_TrackName, sc_TrackName);
		memcpy(s_OpeLogCom.uc_CardID, uc_CardId, sizeof(uc_CardId));
		s_OpeLogCom.uc_IF = JLGE_InspectLogIfTCPSocket;
		strcpy(s_OpeLogCom.sc_ClientInfo, sc_ClientInfo);

		JLGS_OpeLogTrackAuth s_OpeLogTrackAuth;				// ジョブログ登録(操作系)(対象部門情報).
		s_OpeLogTrackAuth.ul_TrackCode = ul_TrackCode;
		strcpy(s_OpeLogTrackAuth.sc_TrackName, sc_TrackName);

		APIC_JLG* pc_JobLog = NULL;
		pc_JobLog = APIC_JLG::newInstance();
		pc_JobLog->addOpeLogTrackAuthentication(&s_OpeLogCom, &s_OpeLogTrackAuth);
	}
}

//==================================================================================================
//[機能]Backup可能状態か確認する.
//[コメント]Taigaでは、HDDがついていない場合バックアップ不可能.
//[コメント]将来性を考えてMonoCusで区切ってるが、第4世代以降修正が必要な可能性あり.
//[履歴] 2009.06.04 okabe.
//		 2009/06/15 hiratani Taiga 制御構成変更11対応.
//==================================================================================================
Bool isBackupEnable()
{
#if defined(DEF_MonoCus) || defined(DEF_SupportSSD)				// 20101117 amiya 【Minerva】制御構成変更141対応.
	if ( isHDDExist() == True ) {	// HDD非装着状態.
		TYPE_HddStatus e_HddStatus = getHDDStatus();
		if ( e_HddStatus == TYPE_HddStatusNon			// ＨＤＤ無し.
		  || e_HddStatus == TYPE_HddStatusConnecting	// 接続中.
		  || e_HddStatus == TYPE_HddStatusNonFormat		// 未フォーマット.
		) {
			return False;
		} else {
			return True;
		}
	} else {
		return False;
	}
#else	// DEF_MonoCus || DEF_SupportSSD.
	return True;
#endif	// DEF_MonoCus || DEF_SupportSSD.
}

//==================================================================================================
//[機能]Restore可能状態か確認する.
//[戻り値] Bool
//[コメント]Taigaでは、HDDがついていない場合リストア不可能.
//[コメント]将来性を考えてMonoCusで区切ってるが、第4世代以降修正が必要な可能性あり.
//[履歴] 2009.06.04 okabe.
//		 2009/06/15 hiratani Taiga 制御構成変更11対応.
//==================================================================================================
Bool isRestoreEnable()
{
#if defined(DEF_MonoCus) || defined(DEF_SupportSSD)				// 20101117 amiya 【Minerva】制御構成変更141対応.
	if ( isHDDExist() == True ) {	// HDD非装着状態.
		TYPE_HddStatus e_HddStatus = getHDDStatus();
		if ( e_HddStatus == TYPE_HddStatusNon			// ＨＤＤ無し.
		  || e_HddStatus == TYPE_HddStatusConnecting	// 接続中.
		  || e_HddStatus == TYPE_HddStatusNonFormat		// 未フォーマット.
		) {
			return False;
		} else {
			return True;
		}
	} else {
		return False;
	}
#else	// DEF_MonoCus || DEF_SupportSSD.
	return True;
#endif	// DEF_MonoCus || DEF_SupportSSD.
}

//==================================================================================================
//[機能]強制メモリ受信ボックスへログイン中かつFAX受信ボックスの出力ユーザ履歴を残す機能が有効になっているか調べる.
//[履歴]2010/03/23 okabe Gaia <UV-00970> FAX受信ボックスの出力ユーザ履歴.
//==================================================================================================
Bool isForceMemoryReceiveBoxJournal( int SocketFd )
{
	Bool bl_SW = False;
	// 強制メモリ受信ボックスへログインしているか確認.
	if( 0 == netc_TwainClientMan.getLoginedBoxNo( SocketFd ) ){
		APIC_SoftDipSw* pc_SoftDipSw = APIC_SoftDipSw::newInstance();
		if ( pc_SoftDipSw != NULL ){
			pc_SoftDipSw->lock();
			bl_SW = pc_SoftDipSw->getSoftDipSwBit( 38, 8 );					// SoftSW No77.0x04の値を取得.
			pc_SoftDipSw->unlock();
		}
	}
	return bl_SW;
}

// 2012/09/19 amiya CQ371727 UV-01212：ボックス登録/変更でパスワード規約違反で失敗した時のログがない START.
//==================================================================================================
//[機能] BOX生成・変更時、パスワード規約となったときにエラーログを残す.
//[履歴] 2012/09/19 amiya CQ371727 UV-01212：ボックス登録/変更でパスワード規約違反で失敗した時のログがない.
//==================================================================================================
void isSetPasswordNGErrorLog( int si_SocketFd, NETE_Box_Operation e_Box_Operation, int32_t sl_BoxID )
{
NetTwainDebugTrace((char*)"### isSetPasswordNGErrorLog() START !!###\n", 0,0,0,0,0,0);
NetTwainDebugTrace((char*)"<<isSetPasswordNGErrorLog()>> e_Box_Operation = [%d]\n", e_Box_Operation,0,0,0,0,0);
NetTwainDebugTrace((char*)"<<isSetPasswordNGErrorLog()>> sl_BoxID = [%d]\n", (int)sl_BoxID,0,0,0,0,0);
	
	// ジョブログ登録.
	JLGS_OpeLogCom s_OpeLogCom;							// ジョブログ登録(操作系)(共通).
	JLGS_OpeLogBox s_OpeLogBox;							// ジョブログ 操作系【ボックス操作情報】.
	
	// ★★①共通部分をセットする★★.
	// 操作コード.
	if( e_Box_Operation == NETE_Box_Operation_Create )
	{
		if( NULL != netc_TwainClientMan.getApic_Admin( si_SocketFd ) )	// 管理者でログイン中の場合.
		{
			s_OpeLogCom.ul_Code = JLGE_InspectLogAdminRegistBox;	// 管理者モードでの、ボックス登録.
		} else {
			s_OpeLogCom.ul_Code = JLGE_InspectLogUserRegistBox;		// ユーザによるボックス登録.
		}
	} else {
		if( NULL != netc_TwainClientMan.getApic_Admin( si_SocketFd ) )	// 管理者でログイン中の場合.
		{
			s_OpeLogCom.ul_Code = JLGE_InspectLogAdminChangeBoxPassword;	// 管理者モードでの、ボックスパスワード変更.
		} else {
			s_OpeLogCom.ul_Code = JLGE_InspectLogUserChangeBoxPassword;		// ユーザによるボックスパスワード変更.
		}
	}
	
	// 操作日時.
	JLGS_Time s_OperateTime = {0};
	APIC_SystemSetting* pc_SystemSetting = NULL;
	pc_SystemSetting = APIC_SystemSetting::newInstance();
	pc_SystemSetting->lock();
	TYPC_Time c_Time;
	c_Time = pc_SystemSetting->getSystemTime();
	s_OperateTime.ss_Year   = c_Time.ss_Year;
	s_OperateTime.sc_Month  = c_Time.sc_Month;
	s_OperateTime.sc_Day    = c_Time.sc_Day;
	s_OperateTime.sc_Hour   = c_Time.sc_Hour;
	s_OperateTime.sc_Minute = c_Time.sc_Minute;
	s_OperateTime.sc_Second = c_Time.sc_Second;
	pc_SystemSetting->unlock();
	s_OpeLogCom.s_Time = s_OperateTime;

	// 結果コード.
	s_OpeLogCom.us_Result = JLGE_InspectLogResultNG;
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	Start.
	s_OpeLogCom.us_EventType = getEventTypeByResultCode( s_OpeLogCom.us_Result );
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	End.

	// IF番号.
	s_OpeLogCom.uc_IF = JLGE_InspectLogIfTCPSocket;
	
	// 操作者コード.
	// 操作者名.
	APIC_User* pc_User = netc_TwainClientMan.getApic_User( si_SocketFd );
	APIC_Track* pc_Track = netc_TwainClientMan.getApic_Track( si_SocketFd );
	
	s_OpeLogCom.ul_OpeCode = JLGE_InspectLogOperateIDL_Non;				// 操作者コードの初期値[ユーザ/部門不定(特定できない)].
	memset( s_OpeLogCom.sc_Name, 0, JLGD_DataLenUserName + 1 );			// 操作者名の初期値[なし].
	s_OpeLogCom.ul_TrackCode = JLGE_InspectLogTrackIDL_Non;				// 部門コードの初期値[部門不定(特定できない)]..
	memset( s_OpeLogCom.sc_TrackName, 0, JLGD_DataLenTrackName + 1 );	// 部門名の初期値[なし]..

	if( NULL != netc_TwainClientMan.getApic_Admin( si_SocketFd ) ) {	// ★管理者の場合.
		// 操作者コード.
		s_OpeLogCom.ul_OpeCode = JLGE_InspectLogOperateIDL_Admin;		// 管理者.
		// 操作者名.
		// 管理者でログイン時は、操作者名はなしでいい.
		// 部門コード.
		s_OpeLogCom.ul_TrackCode = JLGE_InspectLogTrackIDL_Admin;		// 管理者.
		// 部門名.
		// 管理者でログイン時は、部門名はなしでいい.
	} else {															// ★管理者以外の場合.
		// ユーザー関連の値をセットする.
		if( NULL != pc_User ) {
			int32_t sl_UserNo = pc_User->getUserNo();	//2013/10/11 【スケーラブルFUM】[CQ383174] 20万人対応.
NetTwainDebugTraceUserInfo((char*)"<<isSetPasswordNGErrorLog()>> sl_UserNo = [%ld]\n", sl_UserNo,0,0,0,0,0);		// 2022/2/7 [IT6_2.2MR2] OP_BTS-37530 HIENG.
			if( sl_UserNo == TYPD_IndexUserBoxAdmin )
			{
				// 操作者コード.
				s_OpeLogCom.ul_OpeCode = JLGE_InspectLogOperateIDL_BoxAdmin;		// ボックス管理者.
				// 操作者名.
				// ボックス管理者でログイン時は、操作者名はなしでいい.
			}
			else if ( sl_UserNo == TYPD_IndexUserPublic )
			{
				// 操作者コード.
				s_OpeLogCom.ul_OpeCode = JLGE_InspectLogOperateIDL_PublicUser;		// Publicユーザ.
				// 操作者名.
				// Publicユーザでログイン時は、操作者名はなしでいい.
			}
			else
			{
				// 操作者コード.
				s_OpeLogCom.ul_OpeCode = JLGE_InspectLogOperateIDL_User + sl_UserNo;			// ユーザ認証.
				// 操作者名.
				// 操作者はTCPSocketで保持しているログインユーザー名を入れる.
				if( netc_TwainClientMan.getLoginedUserName( si_SocketFd ) != NULL ) {
NetTwainDebugTraceUserInfo((char*)"<<isSetPasswordNGErrorLog()>> [Normal] UserName = [%s]\n", reinterpret_cast<intptr_t>(netc_TwainClientMan.getLoginedUserName( si_SocketFd )),0,0,0,0,0);			// 2022/2/7 [IT6_2.2MR2] OP_BTS-37530 HIENG.
					strncpy( s_OpeLogCom.sc_Name, netc_TwainClientMan.getLoginedUserName( si_SocketFd ), (sizeof(netc_TwainClientMan.getLoginedUserName( si_SocketFd )) + 1) );
				} else {
					// 操作者名が入っていない場合は、そのまま.
NetTwainDebugTrace((char*)"<<isSetPasswordNGErrorLog()>> LoginedUserName NULL\n", 0,0,0,0,0,0);
				}
			}
			
			// 外部サーバー名称.
//			APID_MaxLengthOfServerName(32) < JLGD_DataLenExtSvName(96)なのでjlgの型を採用する.
			memset( s_OpeLogCom.sc_ExtServerName, 0, JLGD_DataLenExtSvName + 1 );	// 外部サーバ名(初期化).
			// サーバー名称を取得 
			char sc_ServerName[APID_MaxLengthOfServerName + 1] = {0,};
			pc_User->getLoginServerName( sc_ServerName );
			if( sc_ServerName != NULL ) {
				memcpy( s_OpeLogCom.sc_ExtServerName, sc_ServerName, APID_MaxLengthOfServerName );
			} else {
				// NULLの場合は、空っぽでいい.
			}
		}
		
		// 部門関連の値をセットする.
		if( NULL != pc_Track ) {
			int32_t sl_TrackNo = pc_Track->getTrackNo();	//2013/10/11 【スケーラブルFUM】[CQ383174] 20万人対応. 
NetTwainDebugTrace((char*)"<<isSetPasswordNGErrorLog()>> sl_TrackNo = [%ld]\n", sl_TrackNo,0,0,0,0,0);
			if( sl_TrackNo == TYPD_IndexTrackBoxAdmin )
			{
				// 操作者コード.
				s_OpeLogCom.ul_TrackCode = JLGE_InspectLogTrackIDL_BoxAdmin;	// ボックス管理者.
				// 操作者名.
				// ボックス管理者でログイン時は、操作者名はなしでいい.
			}
			else if ( sl_TrackNo == TYPD_IndexTrackPublic )
			{
				// 操作者コード.
				s_OpeLogCom.ul_TrackCode = JLGE_InspectLogTrackIDL_Public;		// Publicユーザ.
				// 操作者名.
				// Publicユーザでログイン時は、操作者名はなしでいい.
			}
			else
			{
				// 操作者コード.
				s_OpeLogCom.ul_TrackCode = JLGE_InspectLogTrackIDL_Track + sl_TrackNo;		// 部門.
				// 部門名.
				// TCPSocketでは、部門IDは保持しているが、部門名は保持していないのでNVDから取得する.
				char tmpTrackName[ JLGD_DataLenTrackName + 1 ] = {0,};		// 2012/10/31 amiya 静的解析 [CID454415/CID454751].
				pc_Track->getTrackName( &tmpTrackName[0] );				// 部門名取得 
				strncpy( s_OpeLogCom.sc_TrackName, &tmpTrackName[0], sizeof(tmpTrackName) );
			}
		}
	}
	
	// 他の値は設定不要のため初期値をセットする.
	memset( s_OpeLogCom.uc_CardID, 0, JLGD_DataLenCardIDOpeLog + 1 );		// カードID.
	memset( s_OpeLogCom.sc_ClientInfo, 0, JLGD_DataLenClientInfo + 1 );		// 操作クライアント情報.
	memset( s_OpeLogCom.uc_EmployerID, 0, JLGD_DataLenEmployerID + 1);		// 社員番号.
	memset( s_OpeLogCom.uc_UserName, 0, JLGD_DataLenCardUserName + 1 );		// ユーザー名.
	memset( s_OpeLogCom.uc_DivisionName, 0, JLGD_DataLenDivisionName + 1 );	// 部署名.
	
	
	// ★★②ボックス部分をセットする★★.
	s_OpeLogBox.sl_TargetBoxNo = 0;			// 操作対象ボックス番号に初期値をセット.
	s_OpeLogBox.sl_TargetBoxNo = sl_BoxID;			// 操作対象ボックス番号.
	
	s_OpeLogBox.sl_MoveBoxNo = 2147483647;										// 移動先ボックス番号.
	s_OpeLogBox.sl_CopyBoxNo = 2147483647;										// コピー先ボックス番号.
	memset( s_OpeLogBox.sc_TargetDocumentName, 0, JLGD_DataLenDocName + 1 );	// 操作対象文書名.
	memset( s_OpeLogBox.sc_AfterDocumentName, 0, JLGD_DataLenDocName + 1 );		// 変更後文書名.
	memset( s_OpeLogBox.sc_SecretID, 0, JLGD_DataLenSecretID + 1 );				// 機密文書ID.
	
	
	
NetTwainDebugTrace((char*)"<<isSetPasswordNGErrorLog()>> [Normal] s_OpeLogCom.ul_Code = [%d]\n", (int)s_OpeLogCom.ul_Code,0,0,0,0,0);
NetTwainDebugTrace((char*)"<<isSetPasswordNGErrorLog()>> [Normal] s_OpeLogCom.ul_OpeCode = [%d]\n", (int)s_OpeLogCom.ul_OpeCode,0,0,0,0,0);
NetTwainDebugTraceUserInfo((char*)"<<isSetPasswordNGErrorLog()>> [Normal] s_OpeLogCom.sc_Name = [%s]\n", reinterpret_cast<intptr_t>(s_OpeLogCom.sc_Name),0,0,0,0,0);		// 2022/2/7 [IT6_2.2MR2] OP_BTS-37530 HIENG.
NetTwainDebugTrace((char*)"<<isSetPasswordNGErrorLog()>> [Normal] s_OpeLogCom.ul_TrackCode = [%d]\n", (int)s_OpeLogCom.ul_TrackCode,0,0,0,0,0);
NetTwainDebugTrace((char*)"<<isSetPasswordNGErrorLog()>> [Normal] s_OpeLogCom.sc_TrackName = [%s]\n", reinterpret_cast<intptr_t>(s_OpeLogCom.sc_TrackName),0,0,0,0,0);
NetTwainDebugTrace((char*)"<<isSetPasswordNGErrorLog()>> [Normal] s_OpeLogCom.sc_ExtServerName = [%s]\n", reinterpret_cast<intptr_t>(s_OpeLogCom.sc_ExtServerName),0,0,0,0,0);
NetTwainDebugTrace((char*)"<<isSetPasswordNGErrorLog()>> [Box] s_OpeLogBox.sl_TargetBoxNo = [%d]\n", (int)s_OpeLogBox.sl_TargetBoxNo,0,0,0,0,0);
	
	
	
	APIC_JLG* pc_JobLog = NULL;
	pc_JobLog = APIC_JLG::newInstance();
	pc_JobLog->addOpeLogBox( &s_OpeLogCom, &s_OpeLogBox );		// ジョブログ登録(操作系)(共通).
}
// 2012/09/19 amiya CQ371727 UV-01212：ボックス登録/変更でパスワード規約違反で失敗した時のログがない END.

// MinervaBK2nd CQ380261 OPS1八代 2013.05.30 START.
//==================================================================================================
//[機能]   MD5のメッセージダイジェストを比較する(強制メモリ受信ボックス専用).
//[条件]   コマンド指定のBoxIDが強制メモリ受信ボックス(=0)のときのみ、コールすること.
//[戻り値] void.
//[引数]   [第一引数] [IN ] :ソケットファイルディスクリプション
//         [第二引数] [IN ] :メッセージダイジェスト化されたボックスワード(コマンドに付加されたモノ).
//         [第三引数] [OUT] :DocのAPIに渡すため、ボックスパスワードを返す.
//						    :認証OKの場合は、不揮発の強制メモリ受信パスワードを返す.
//						    :認証NGの場合は、必ず失敗するパスワードを返す.
//[履歴] 2013/05/30 OPS1八代 新規作成.
//==================================================================================================.
void CompMD5_FaxMemRcvPswd( int SocketFd, char* psc_BoxPswdFromCommond, char* psc_FixPswd )
{
NetTwainDebugTrace((char*)"CompMD5_FaxMemRcvPswd() START\n", 0,0,0,0,0,0);
	
	int i_PassLen = 0;
	uchar uc_CmpPassWord[TYPD_MaxCmpPassWord + 1];
	memset( uc_CmpPassWord, 0x00, sizeof(uc_CmpPassWord) );

	char buf[256];
	memset( buf, 0, sizeof(buf) );

	char sc_md5[256];
	memset( sc_md5, 0, sizeof(sc_md5) );

	uchar	uc_ChallengeValue[16];
	memset( uc_ChallengeValue, 0, sizeof(uc_ChallengeValue) );

	// Outputの引数(パスワード)に入力できない文字列を仮代入.
	// パスワードが一致しなかった場合、この文字列を呼び元へ返し、DocのAPIへ渡す.
	memset( psc_FixPswd, 0x01, 1 );

	// MinervaBK2nd CQ379634(静的解析) ID32955 2013.06.07 OPS1八代 START.
	if(netc_TwainClientMan.getChallengeValue( SocketFd ) == NULL){
		errorLog_TwainCommon("CompMD5_FaxMemRcvPswd() getChallengeValue NULL!!!\n",0,0,0,0,0,0);
		return;
	}
	// MinervaBK2nd CQ379634(静的解析) ID32955 2013.06.07 OPS1八代 END.

	memcpy( uc_ChallengeValue, netc_TwainClientMan.getChallengeValue( SocketFd ), sizeof(uc_ChallengeValue) );

	// FAX基盤数だけループさせる.
	for( uchar i = 0; i < TYPD_CommLineMax; i++ ) {	
		TYPE_CommLine e_Line = (TYPE_CommLine)(i+1);

		// FAX基盤装着時にチェックすればよいが、インターネットFAXは基盤無しでも使用できるため、FAX回線1は必ずチェックする.
		if( ( TYPE_CommLine1 == e_Line ) || ( True ==  APIC_SystemInfo::newInstance()->getFaxExistAll( e_Line ) ) )  {
			
			// 強制メモリ受信する/しないの不揮発値を確認する.
			// これを見れば、FAX基盤装着/非装着は見なくてもいいような感じもするが.
			// 強制メモリ受信する->FAX基盤非装着->再起動した場合、
			// 強制メモリ受信の不揮発値がクリアされるかわからないので、念のため、FAX基盤装着/非装着も見る.
			if( TYPE_RcvFunc_CompulsionRxOn == APIC_FaxGfpSetting::newInstance()->getRcvFunc_CompulsionRxAll( e_Line ) ) {

				// 回線に対応する強制メモリ受信ボックスパスワードを取得.
				APIC_FaxGfpSetting::newInstance()->getRcvFunc_CompulsionRxPasswordAll( e_Line, uc_CmpPassWord );

				i_PassLen = (int)strlen( (const char*)uc_CmpPassWord );
				
NetTwainDebugTraceUserInfo((char*)"CompMD5_FaxMemRcvPswd() Line[%d] Pswd=[%s] Len=[%d]\n",  (int)e_Line, reinterpret_cast<intptr_t>(uc_CmpPassWord),(int)i_PassLen,0,0,0);		// 2022/2/7 [IT6_2.2MR2] OP_BTS-37530 HIENG.
				
				// MD5で強制メモリ受信ボックスパスワードをメッセージダイジェスト化.
				memcpy( &buf[0],  uc_ChallengeValue, 16 );
				memcpy( &buf[16], uc_CmpPassWord, i_PassLen );

				KM_MD5( (unsigned char*)&buf[0], ( 16 + i_PassLen ), (unsigned char*)&sc_md5[0] );

				// メッセージダイジェス化したパスワード同士を比較する.
				if ( memcmp( psc_BoxPswdFromCommond, sc_md5, 16 ) == 0 ) {
					// パスワードが一致した.パスワードを代入し、呼び元へ返し、DocのAPIへ渡す.
					memcpy( psc_FixPswd, uc_CmpPassWord, ( i_PassLen + 1 ) );
					break;

				} else {
					// パスワードが一致しなかった.MD作成用のバッファ領域を初期化.
					i_PassLen = 0;
					memset( buf,    0, sizeof(buf) );
					memset( sc_md5, 0, sizeof(sc_md5) );
				}
			}
		}
	}
NetTwainDebugTraceUserInfo((char*)"CompMD5_FaxMemRcvPswd() END FixPswd=[%s]\n", reinterpret_cast<intptr_t>(psc_FixPswd),0,0,0,0,0);		// 2022/2/7 [IT6_2.2MR2] OP_BTS-37530 HIENG.
}
// MinervaBK2nd CQ380261 OPS1八代 2013.05.30 END.

// 2014/08/25 [ZeusS][制御構成変更No.50]OPS1前田 Start.
//==================================================================================================
//[機能]   Fieryキャリブレーション固有処理かを判定.
//[条件]   .
//[戻り値] Bool(True:Fieryキャリブレーション, False:Fieryキャリブレーションではない.
//[引数]   [第一引数] [IN ] :ソケットファイルディスクリプション
//[履歴] 2014/08/25 OPS1前田 新規作成.
//==================================================================================================.
Bool isFieryCalibrationCheck( int SocketFd )
{
	Bool bl_FieryCalibration = False;											// Fieryキャリブレーション判定フラグ.

	if(  netc_TwainClientMan.getAppID(SocketFd) == 0x11 )        // Fieryキャリブレーション.
	{
		bl_FieryCalibration = True;
	}
NetTwainDebugTrace((char*)"### isFieryCalibrationCheck bl_FieryCalibration = %d ###\n", bl_FieryCalibration ,0,0,0,0,0);
	return bl_FieryCalibration;
}
// 2014/08/25 [ZeusS][制御構成変更No.50]OPS1前田 End.

// 2014/09/03 [ZeusS][RQ-232][WP955]OPS1前田 Start.
//==================================================================================================
//[機能]   bizhubRAアプリからの接続かを判定.
//[条件]   .
//[戻り値] Bool(True:bizhubRAアプリからの接続, False:bizhubRAアプリからの接続ではない.
//[引数]   [第一引数] [IN ] :ソケットファイルディスクリプション
//[履歴] 2014/09/03 OPS1前田 新規作成.
//==================================================================================================.
Bool isbizhubRACheck( int SocketFd )
{
	Bool bl_bizhubRA = False;

	if( netc_TwainClientMan.getAppID(SocketFd) == 0x12 )        // bizhubRA.
	{
		bl_bizhubRA = True;
	}
	return bl_bizhubRA;
}
// 2014/09/03 [ZeusS][RQ-232][WP955]OPS1前田 End.

// 2014/09/08 [ZeusS][RQ-232][WP957]OPS1前田 Start.
//==================================================================================================
//[機能]   bizhubRAからの要求の場合、CommonAPIへ認証結果を設定する.
//[条件]   .
//[戻り値] なし.
//[引数]   [第一引数] [IN ] :認証結果 (ログアウトフラグがFalseの場合のみ使用).
//									  (ログアウトフラグがTrueの場合はFalseを指定する).
//   	   [第二引数] [IN ] :ログアウトフラグ (True :ログアウト、Lan切断時 / False :ログイン).
//   	   [第三引数] [IN ] :ソケットファイルディスクリプション.
//   	   [第四引数] [IN ] :タイムアウトフラグ.
//[履歴] 2014/09/08 OPS1前田 新規作成.
//==================================================================================================.
void setAuthResult(
	Bool bl_AuthResult,			//認証結果.
	Bool bl_LogoutFlg,			//ログアウトフラグ.
	int SocketFd,				//ソケットファイルディスクリプション.
	Bool bl_TimeOutFlg			//タイムアウトフラグ.
)
{
NetTwainDebugTrace((char*)"#### setAuthResult() Start! ####\n",0,0,0,0,0,0);
NetTwainDebugTrace((char*)"#### setAuthResult() CommandID = %d ####\n", netc_TwainClientMan.getCommandID( SocketFd ) ,0,0,0,0,0);
	APIC_AndroidPanel* pc_AndroidPanel = APIC_AndroidPanel::newInstance();

	if( False == bl_LogoutFlg ) {														//ログイン要求の場合ログイン結果とログインしたユーザーのカラー制限を設定する.
		//RAユーザの事前認証：認証結果に応じて成/否を変更.
		pc_AndroidPanel->setRAUserPreAuth( bl_AuthResult );
NetTwainDebugTrace((char*)"#### setAuthResult() Login ####\n",0,0,0,0,0,0);

		if( True == bl_AuthResult ) {													// 認証が成功した場合は、ログインしたユーザーのスキャン制限情報を設定.
NetTwainDebugTrace((char*)"#### setAuthResult() bl_AuthResult == True ####\n",0,0,0,0,0,0);
			if( ( netc_TwainClientMan.getCommandID( SocketFd ) == SCL_USER_LOGIN ) 
			  ||( netc_TwainClientMan.getCommandID( SocketFd ) == SCL_CUSTOMIZED_LOGIN )
			) {
NetTwainDebugTrace((char*)"#### setAuthResult() CommandID == SCL_USER_LOGIN || SCL_CUSTOMIZED_LOGIN ####\n",0,0,0,0,0,0);
				//RAユーザのスキャン制限情報：ユーザの制限設定に合わせてカラー/モノクロのみ/不可を変更.
				// ユーザーインスタンスを取得する.
				APIC_User* pc_User = netc_TwainClientMan.getApic_User( SocketFd );
				if ( pc_User != NULL )
				{
					pc_User->lock();
					TYPE_FuncLimit e_EnableScan_User = pc_User->getFunctionLimitation().e_EnableScan;								// スキャン.
					pc_User->unlock();

					// 部門認証と連動していない場合、または、「部門とユーザー認証がユーザー設定を使用」かつ「ユーザーがユーザー認証と連動していないを選択」の場合.
					if( ( ( getTrackMode() != TYPE_TrackModeOnWithUserAuthentication ) )
					 || ( !( ( getTrackMode() == TYPE_TrackModeUserSelect ) && ( pc_User != NULL ) && ( pc_User->getUserTrackMode() == TYPE_UserTrackMode_Link  ) ) ) )
					{
NetTwainDebugTrace((char*)"#### setAuthResult() Login TYPE_TrackAuthentication_UnLink ####\n",0,0,0,0,0,0);
						// スキャン機能制限.
						if( e_EnableScan_User == TYPE_FuncLimit_Disable ) {					// 禁止.
							pc_AndroidPanel->setRAScanLimitColorInfo( APIE_RAScanLimitColorInfoDisable );
						} else if( e_EnableScan_User == TYPE_FuncLimit_OnlyBlack ) {		// ブラックのみ許可.
							pc_AndroidPanel->setRAScanLimitColorInfo( APIE_RAScanLimitColorInfoMonoOnly );
						} else {															// 許可.
							pc_AndroidPanel->setRAScanLimitColorInfo( APIE_RAScanLimitColorInfoColor );
						}
					// 部門認証と連動している場合、「部門とユーザー認証がユーザー設定を使用」かつ「ユーザーがユーザー認証と連動するを選択」の場合、部門側の設定とユーザー側の設定と厳しいほうの設定に倒して応答する.
					} else {
NetTwainDebugTrace((char*)"#### setAuthResult() Login TYPE_UserTrackMode_Link ####\n",0,0,0,0,0,0);
						APIC_Track* pc_Track = netc_TwainClientMan.getApic_Track( SocketFd );
						if ( pc_Track != NULL ) {
							pc_Track->lock();
							// 部門の機能制限情報の取得.
							TYPE_FuncLimit e_EnableScan_Track = pc_Track->getFunctionLimitation().e_EnableScan;
							pc_Track->unlock();
		
							// スキャン機能制限.
							if( ( e_EnableScan_User == TYPE_FuncLimit_Disable ) || ( e_EnableScan_Track == TYPE_FuncLimit_Disable ) ) {			// どっちか禁止⇒禁止.
								pc_AndroidPanel->setRAScanLimitColorInfo( APIE_RAScanLimitColorInfoDisable );
							} else if( ( e_EnableScan_User == TYPE_FuncLimit_Enable ) && ( e_EnableScan_Track == TYPE_FuncLimit_Enable ) ) {	// 両方許可⇒許可.
								pc_AndroidPanel->setRAScanLimitColorInfo( APIE_RAScanLimitColorInfoColor );
							} else {																											// ブラックのみ許可.
								pc_AndroidPanel->setRAScanLimitColorInfo( APIE_RAScanLimitColorInfoMonoOnly );
							}
						}
					}
				}
			} else if( netc_TwainClientMan.getCommandID( SocketFd ) == SCL_ACCOUNT_LOGIN ) {
NetTwainDebugTrace((char*)"#### setAuthResult() CommandID == SCL_ACCOUNT_LOGIN ####\n",0,0,0,0,0,0);
				// 管理者インスタンスを取得する.
				APIC_Track* pc_Track = netc_TwainClientMan.getApic_Track( SocketFd );
				if ( pc_Track != NULL )
				{
					// 部門の機能制限情報の取得.
					pc_Track->lock();
					TYPE_FuncLimit e_EnableScan_Track = pc_Track->getFunctionLimitation().e_EnableScan;
					pc_Track->unlock();
					
					// スキャン機能制限.
					if( e_EnableScan_Track == TYPE_FuncLimit_Disable ) {
						pc_AndroidPanel->setRAScanLimitColorInfo( APIE_RAScanLimitColorInfoDisable );
					} else if( e_EnableScan_Track == TYPE_FuncLimit_OnlyBlack ) {
						pc_AndroidPanel->setRAScanLimitColorInfo( APIE_RAScanLimitColorInfoMonoOnly );
					} else {
						pc_AndroidPanel->setRAScanLimitColorInfo( APIE_RAScanLimitColorInfoColor );
					}
				}
			} else if( netc_TwainClientMan.getCommandID( SocketFd ) == SCL_VENDOR_LOGIN ) {
NetTwainDebugTrace((char*)"#### setAuthResult() CommandID == SCL_VENDOR_LOGIN ####\n",0,0,0,0,0,0);
				pc_AndroidPanel->setRAScanLimitColorInfo( APIE_RAScanLimitColorInfoColor );
			} else {
NetTwainDebugTrace((char*)"#### setAuthResult() CommandID = %d Excluded ####\n", netc_TwainClientMan.getCommandID( SocketFd ) ,0,0,0,0,0);
			}
		} else {																		// 認証が失敗した場合は、CommonAPIを初期化する.
NetTwainDebugTrace((char*)"#### setAuthResult() bl_AuthResult == False ####\n",0,0,0,0,0,0);
		//RAユーザのスキャン制限情報：不可へ変更.
			pc_AndroidPanel->setRAScanLimitColorInfo( APIE_RAScanLimitColorInfoDisable );
		}
	} else {																			//ログイン要求以外(ログアウト、Lan切断時)の場合は、CommonAPIを初期化する.
		//2015/04/21 [ZeusS][CQ512658]OPS1前田 Start.
		if( True == bl_TimeOutFlg ) {
NetTwainDebugTrace((char*)"#### setAuthResult() TimeOut ####\n",0,0,0,0,0,0);
			Bool bl_RAConnection = False;		//タイムアウトが発生したセッション以外でRAの接続がある場合はTrueとする.
			
			for ( int index = 0; index < SUPPORTED_CLIENT_NUM; index++ ) {
				//タイムアウトが発生したセッション以外でRAの接続がある場合、フラグを立てる.
				if( ( SocketFd != netc_TwainClientMan.getSocketFd( index ) )
				  &&( 0x12     == netc_TwainClientMan.getAppID_ALL( index ) ) 
				) {
NetTwainDebugTrace((char*)"#### setAuthResult() bizhubRA in connection index %d ####\n",index,0,0,0,0,0);
					bl_RAConnection = True;
					break;
				}
			}

			if( False == bl_RAConnection ) {
NetTwainDebugTrace((char*)"#### setAuthResult() not bizhubRA in connection index %d ####\n",index,0,0,0,0,0);
				//タイムアウトが発生したセッション以外でRAの接続がないためCommonのフラグを書き換える.
				//RAユーザの事前認証：否へ変更.
				pc_AndroidPanel->setRAUserPreAuth( False );

				//RAユーザのスキャン制限情報：不可へ変更.
				pc_AndroidPanel->setRAScanLimitColorInfo( APIE_RAScanLimitColorInfoDisable );
			}

		} else {
NetTwainDebugTrace((char*)"#### setAuthResult() Logout ####\n",0,0,0,0,0,0);
		//RAユーザの事前認証：否へ変更.
		pc_AndroidPanel->setRAUserPreAuth( False );

		//RAユーザのスキャン制限情報：不可へ変更.
		pc_AndroidPanel->setRAScanLimitColorInfo( APIE_RAScanLimitColorInfoDisable );
		}
		//2015/04/21 [ZeusS][CQ512658]OPS1前田 End.
	}
NetTwainDebugTrace((char*)"#### setAuthResult() End! ####\n",0,0,0,0,0,0);

}
// 2014/09/08 [ZeusS][RQ-232][WP957]OPS1前田 End.

// MinervaSBK RQ-218 OPS1八代 2015.03.31 START.
//==================================================================================================
//[機能]   認証アプリ登録情報から、CostCenter利用設定を取得する.
//[条件]   APIC_ExtSettingクラスのlock～unlock期間中に、このAPIをコールしないこと！(デッドロックになる） .
//[戻り値] Bool(True:CostCenter利用する, False:CostCenter利用しない).
//[引数]   なし.
//[履歴] 2015/03/31 OPS1八代 新規作成.
//==================================================================================================.
Bool isCostCenterUsageSettingEnable()
{
NetTwainDebugTrace((char*)"#### isCostCenterUsageSettingEnable() Start! ####\n",0,0,0,0,0,0);
	Bool bl_Rtn = False;

	APIC_ExtSetting* pc_ExtSetting = NULL;
	TYPC_ExtAplDetail* pc_ExtAplDetail = NULL;
	TYPS_ExtDetailAuthentication* ps_ExtDetailAuth = NULL;

	TYPS_ExtRegistration s_ExtRegistration;
	memset( &s_ExtRegistration, 0, sizeof(TYPS_ExtRegistration) );

	// 認証アプリの登録状態を不揮発から検索する.
	for( int i = 1;  i <= TYPD_ExtMaxApplication; i++ )	{
		pc_ExtSetting = APIC_ExtSetting::newInstance( i );
		if ( pc_ExtSetting != NULL ) {
			pc_ExtSetting->lock();

			s_ExtRegistration = pc_ExtSetting->getApplication();

			if ( TYPE_ExtEntryOn == s_ExtRegistration.e_Entry ) {

				if( ( TYPE_ExtTypeAuthentication == s_ExtRegistration.s_Version.e_Type )
				 || ( TYPE_ExtTypeBrowserAuth == s_ExtRegistration.s_Version.e_Type ) ) {
					pc_ExtAplDetail = pc_ExtSetting->getAplDetail();

					if ( pc_ExtAplDetail != NULL ) {
						ps_ExtDetailAuth = pc_ExtAplDetail->getAuthentication();

						if ( ps_ExtDetailAuth != NULL ) {
							if ( TYPE_AuthenticationEnable == ps_ExtDetailAuth->s_Function.e_Authentication ) {
								if( TYPE_ExtCostCenterUsageSettingEnable == ps_ExtDetailAuth->s_Function.s_CostCenterSetting.e_CostCenterUsageSetting ) {
									bl_Rtn = True;
									pc_ExtSetting->unlock();
									break;
								}
							}
						}
					}
				}
			}

			pc_ExtSetting->unlock();
		}
		pc_ExtSetting = NULL;
		pc_ExtAplDetail = NULL;
		ps_ExtDetailAuth = NULL;
	}
NetTwainDebugTrace((char*)"#### bl_Rtn = %d####\n",bl_Rtn,0,0,0,0,0);
NetTwainDebugTrace((char*)"#### isCostCenterUsageSettingEnable() End! ####\n",0,0,0,0,0,0);
	return bl_Rtn;
}

//==================================================================================================
//[機能]   CostCenterIDを返却する.
//[条件]   戻り値がTrueのときのみ、CostCenterIDを利用すること.
//[戻り値] Bool(True:CostCenterIDがNULL以外, False:CostCenterIDがNULL).
//[引数]   [第一引数] [IN ] :ソケットファイルディスクリプション.
//   	   [第二引数] [OUT] :CostCenterID(文字列).
//[履歴] 2015/03/31 OPS1八代 新規作成.
//==================================================================================================.
Bool getCostCenterID( int SocketFd, char* psc_CostCenterID )
{
NetTwainDebugTrace((char*)"#### getCostCenterID() Start! ####\n",0,0,0,0,0,0);

	Bool bl_Rtn = False;

	if( True == isCostCenterUsageSettingEnable() ) {
NetTwainDebugTrace((char*)"### CostCenterUsageSettingEnable ###\n",0,0,0,0,0,0);

		TYPS_ResExtParameter* ps_ResExtParameter = netc_TwainClientMan.getResExtParameter( SocketFd );
		if( NULL != ps_ResExtParameter ) {
			strncpy( psc_CostCenterID, ps_ResExtParameter->ps_ExtCostCenter->sc_CostCenterID, TYPD_ExtMaxCostCenterID );
			bl_Rtn = True;
NetTwainDebugTrace((char*)"### sc_CostCenterID = %s ###\n",reinterpret_cast<intptr_t>(psc_CostCenterID),0,0,0,0,0);
			}
	}

NetTwainDebugTrace((char*)"#### bl_Rtn = %d####\n",bl_Rtn,0,0,0,0,0);
NetTwainDebugTrace((char*)"#### getCostCenterID() End! ####\n",0,0,0,0,0,0);

	return bl_Rtn;
}
// MinervaSBK RQ-218 OPS1八代 2015.03.31 END.
// MinervaSBK CQ515835 2015.08.26 OPS1八代 START.
//==================================================================================================
//[機能]   重送検知時原稿戻し警告の発生有無を判定する.
//[条件]   なし.
//[戻り値] Bool(True:発生している, False:発生していない).
//[引数]   [第一引数] なし.
//[履歴] 2015/08/26 OPS1八代 新規作成.
//==================================================================================================.
Bool isMultiFeedDetect()
{
	Bool bl_Rtn = False;

	APIC_ActiveJobList* pc_ActiveJobList = APIC_ActiveJobList::newInstance();

	if( NULL != pc_ActiveJobList ) {
		pc_ActiveJobList->lock();
		APIC_Job* pc_Job = pc_ActiveJobList->getCurrentScanJob();
		pc_ActiveJobList->unlock();

		if( NULL != pc_Job ) {
			pc_Job->lock();
			if( True ==  pc_Job->getJobSpec()->pc_Scan->getADFMultiFeedDetectOn() ) {
				if( True == pc_Job->isWarning( CAUTION_DocReturnNum1 ) 					// 原稿戻し枚数警告1(SimLin用処理).
				 || True == pc_Job->isWarning( ERROR_DocReturnNum2 ) 					// 原稿戻し枚数警告2(SimLin用処理).
				 || True == pc_Job->isWarning( CAUTION_DocReturn1_MultiFeedDetect ) 	// 重送検知時原稿戻し警告1.
				 || True == pc_Job->isWarning( ERROR_DocReturn2_MultiFeedDetect ) 		// 重送検知時原稿戻し警告2.
				) {
					bl_Rtn = True;
				}
			}
			pc_Job->unlock();
		}
	}
NetTwainDebugTrace((char*)"#### isMultiFeedDetect() bl_Rtn = %d####\n",bl_Rtn,0,0,0,0,0);

	return bl_Rtn;
}
// MinervaSBK CQ515835 2015.08.26 OPS1八代 END.

// 2015/11/09 GTaguchi ZeusS_ZX0 制御構成変更_No.76-2 START.
//==================================================================================================
//[機能]ネットワークエラーカウンターの指定サブコードに対応したカウンタを加算する.
//[戻り値]
//[コメント]
//[レビュー]
//[履歴] 2015.11.09 GTaguchi.
//==================================================================================================
void incNetworkErrorCnt(
	NETE_ErrCntSubCode e_SubCode
)
{
	APIC_NetworkErrorCnt* pc_NetworkErrorCnt = APIC_NetworkErrorCnt::newInstance();
	
	if( NULL != pc_NetworkErrorCnt ) {
		pc_NetworkErrorCnt->lock();
		pc_NetworkErrorCnt->inc( TYPE_NetworkErrorCnt_TCPSocket, e_SubCode );
		pc_NetworkErrorCnt->unlock();
	} else {
NetTwainDebugTrace("APIC_NetworkErrorCnt NULL ERROR\n",0,0,0,0,0,0);
	}
	
	return;

}
// 2015/11/09 GTaguchi ZeusS_ZX0 制御構成変更_No.76-2 END.

