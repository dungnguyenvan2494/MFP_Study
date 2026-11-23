//======================================================================
// 
// ファイル: WDAC_WebDAVdDeviceAppTool.cpp 
// 概要: WebDAVdデバイスデバッグツール
// 履歴: 2006/12/22 e.nakamura  新規作成
//======================================================================
// 2017/12/19 【Eagle】【Simlin】 hitachi  Phat-Dang Modify Start.
//#ifdef SIMULATOR
//#pragma warning( disable : 4101 )
//#endif
// 2017/12/19 【Eagle】【Simlin】 hitachi  Phat-Dang Modify End.
//======================================================================
// インクルード
//======================================================================

//(■■moselコンパイルで回避するための一時的な処置、Thames-iの正式なコンパイルスイッチが出来たら再調整■■)

#ifdef TORNADO			/* VXWORKSで有効 */
#ifndef LINUX
#include <vxWorks.h>
#include <taskLib.h>
#include <errnoLib.h>
#include <usrLib.h>
#include <msgQLib.h>
#include <ioLib.h>
#include <tickLib.h>
#endif	/* LINUX */
#endif

#include <stdio.h>
#include <stdarg.h>
#include <string.h>
#include <time.h>

#include "WDAC_WebDAVdDeviceApp.h"
#include "system/sys/sysg/SYSF_MemoryAllocFree.h"
/* 2010.07.12 加藤木 UV-00794 デバッグログ収集システム対応 Start */
#include "../../shlib/LogLib/LIBC_LogLib.h"
#include "../../shlib/SettingLib/LIBC_DebugLogSystemSetting.h"
/* 2010.07.12 加藤木 UV-00794 デバッグログ収集システム対応 End */

//======================================================================
// ログ関連 
//======================================================================

//======================================================================
// トレース関連処理 
//======================================================================

// インデックス・トレースログ領域・出力モード切替 
int        si_WDAS_TrcIdx;
WDAS_Log   WDAS_TrcBuf[WDAS_LOG_COUNT];
int        si_WDAS_TrcMode;

// トレース初期化処理 
void WDAS_TrcInit()
{
	int si_loop;
	si_WDAS_TrcIdx = 0;
	si_WDAS_TrcMode = 0;

	for (si_loop=0; si_loop<WDAS_LOG_COUNT; si_loop++)
	{
		WDAS_TrcBuf[si_loop].date[0] = '\0';
		WDAS_TrcBuf[si_loop].info[0] = '\0';
	}
}


// トレースログ処理 
void WDAS_TrcLog(const char *pc_format, ...)
{
	// トレース無し 
	if( si_WDAS_TrcMode == 0 )
	{
		return;
	}

	struct tm		TmVal;
	va_list			va;
	int				si_length;
	WDAS_Log		tmplog;
	char			tmpbuff[ 512 ];		// ここを変更するときはWDAS_LOG_LENGTHより大きいサイズであることに留意してください
/* 2010.07.12 加藤木 UV-00794 デバッグログ収集システム対応 Start */
	LIBS_WebDAVdDeviceAppTrcDebugLogTbl st_DbgLgTbl;
	memset(&st_DbgLgTbl, 0, sizeof(LIBS_WebDAVdDeviceAppTrcDebugLogTbl));
/* 2010.07.12 加藤木 UV-00794 デバッグログ収集システム対応 End */

	struct timespec	TmsVal;
	// 現在時刻取得 
	clock_gettime( CLOCK_REALTIME, &TmsVal );
	SYSF_localtime_r(&(TmsVal.tv_sec), &TmVal);			// 20110414 mutoh CQ335800 localtime_r -> SYSF_localtime_r.

	// 時刻フォーマット変換 
	strftime(tmplog.date, 16, "%m/%d %H:%M:%S  ", &TmVal);
//	tmplog.date[15] = ' ';		// Linux対応(CQ:322310のついで) strftimeの後に、末端に空白をセットしておく(ログ破壊防止のため).
	tmplog.date[15] = '\0';

	// 書式調整 
    va_start(va, pc_format);
	si_length = vsprintf(tmpbuff, pc_format, va);
	va_end(va);

	// 文字列長チェック 
	if( si_length >= WDAS_LOG_LENGTH )
	{
//#ifdef TORNADO
//		SYSF_logMsg("message too int32_t : ",0,0,0,0,0,0);
//		SYSF_logMsg("%s",(int)tmplog.info,0,0,0,0,0);
//#else
		printf("message too int32_t : ");
		printf(tmpbuff);
//#endif
		return;
	}

//2009.06.23 加藤木 静的解析対応 Start.
	// 一時バッファから格納 
//	strncpy( tmplog.info, tmpbuff, WDAS_LOG_LENGTH );
	memset(tmplog.info, 0, sizeof(tmplog.info));
	strncpy( tmplog.info, tmpbuff, (WDAS_LOG_LENGTH - 1) );
//2009.06.23 加藤木 静的解析対応 Start.

	// メモリ格納 
	if( si_WDAS_TrcMode == 1 ){
		strcpy(WDAS_TrcBuf[si_WDAS_TrcIdx].date,tmplog.date);
		si_WDAS_TrcIdx++;
		si_WDAS_TrcIdx %= WDAS_LOG_COUNT;
	}
	// 即時出力 
	else if( si_WDAS_TrcMode == 2 )
	{
//#ifdef TORNADO
//		SYSF_logMsg("Trc : %s", (int)tmplog.date,0,0,0,0,0);
//#else
		printf("Trc : %s", tmplog.date);
//#endif
	}
/* 2010.07.12 加藤木 UV-00794 デバッグログ収集システム対応 Start */
	memcpy(st_DbgLgTbl.date, tmplog.date, (LIBD_WDAS_DATE_LENGTH -1));
	memcpy(st_DbgLgTbl.info, tmplog.info, (LIBD_WDAS_LOG_LENGTH -1));
	/* レコード書き込み位置は、次レコード位置(-1)を指定する */
	short ss_LogIndex = -1;
	LIBF_WriteSystemLog(LIBD_LogID_WDASTrace, 
						LIBD_LogOutPutLevel2,
						LIBD_LogCategoryPrint,
						&st_DbgLgTbl,
						&ss_LogIndex);
/* 2010.07.12 加藤木 UV-00794 デバッグログ収集システム対応 End */
}


//======================================================================
// エラーログ関連処理 
//======================================================================

// インデックス・エラーログ領域・出力モード切替 
int         si_WDAS_ErrIdx;
WDAS_Log    WDAS_ErrBuf[WDAS_LOG_COUNT];
int         si_WDAS_ErrMode;

// エラーログ領域初期化処理 
void WDAS_ErrInit()
{
	int si_loop;
	si_WDAS_ErrIdx = 0;
	si_WDAS_ErrMode = 1;

	for (si_loop=0; si_loop<WDAS_LOG_COUNT; si_loop++)
	{
		WDAS_ErrBuf[si_loop].date[0] = '\0';
		WDAS_ErrBuf[si_loop].info[0] = '\0';
	}
}

// エラーログ処理 
void WDAS_ErrLog(const char *pc_format, ...)
{
	// トレース無し 
	if( si_WDAS_ErrMode == 0 )
	{
		return;
	}

	struct tm		TmVal;
	va_list			va;
	int				si_length;
	WDAS_Log		tmplog;
	char			tmpbuff[ 512 ];		// ここを変更するときはWDAS_LOG_LENGTHより大きいサイズであることに留意してください
/* 2010.07.12 加藤木 UV-00794 デバッグログ収集システム対応 Start */
	LIBS_WebDAVdDeviceAppErrDebugLogTbl st_DbgLgTbl;
	memset(&st_DbgLgTbl, 0, sizeof(LIBS_WebDAVdDeviceAppErrDebugLogTbl));
/* 2010.07.12 加藤木 UV-00794 デバッグログ収集システム対応 End */

	struct timespec	TmsVal;
	// 現在時刻取得 
	clock_gettime( CLOCK_REALTIME, &TmsVal );
	SYSF_localtime_r(&(TmsVal.tv_sec), &TmVal);			// 20110414 mutoh CQ335800 localtime_r -> SYSF_localtime_r.

	// 時刻フォーマット変換 
	strftime(tmplog.date, 16, "%m/%d %H:%M:%S  ", &TmVal);
//	tmplog.date[15] = ' ';		// Linux対応(CQ:322310のついで) strftimeの後に、末端に空白をセットしておく(ログ破壊防止のため).
	tmplog.date[15] = '\0';

	// 書式調整 
    va_start(va, pc_format);
	si_length = vsprintf(tmpbuff, pc_format, va);
	va_end(va);

	// 文字列長チェック 
	if( si_length >= WDAS_LOG_LENGTH )
	{
//#ifdef TORNADO
//		SYSF_logMsg("message too int32_t : ",0,0,0,0,0,0);
//		SYSF_logMsg("%s",(int)tmplog.info,0,0,0,0,0);
//#else
		printf("message too int32_t : ");
		printf(tmpbuff);
//#endif
		return;
	}

//2009.06.23 加藤木 静的解析対応 Start.
	// 一時バッファから格納 
//	strncpy( tmplog.info, tmpbuff, WDAS_LOG_LENGTH );
	memset(tmplog.info, 0, sizeof(tmplog.info));
	strncpy( tmplog.info, tmpbuff, (WDAS_LOG_LENGTH - 1) );
//2009.06.23 加藤木 静的解析対応 End.

	// メモリ格納 
	if( si_WDAS_ErrMode == 1 ){
		strcpy(WDAS_ErrBuf[si_WDAS_ErrIdx].date,tmplog.date);
		si_WDAS_ErrIdx++;
		si_WDAS_ErrIdx %= WDAS_LOG_COUNT;
	}
	// 即時出力 
	else if( si_WDAS_ErrMode == 2 )
	{
//#ifdef TORNADO
//		SYSF_logMsg("Err : %s", (int)tmplog.date,0,0,0,0,0);
//#else
		printf("Err : %s", tmplog.date);
//#endif
	}
/* 2010.07.12 加藤木 UV-00794 デバッグログ収集システム対応 Start */
	memcpy(st_DbgLgTbl.date, tmplog.date, (LIBD_WDAS_DATE_LENGTH -1));
	memcpy(st_DbgLgTbl.info, tmplog.info, (LIBD_WDAS_LOG_LENGTH -1));
	/* レコード書き込み位置は、次レコード位置(-1)を指定する */
	short ss_LogIndex = -1;
	LIBF_WriteSystemLog(LIBD_LogID_WDASErr, 
						LIBD_LogOutPutLevel2,
						LIBD_LogCategoryPrint,
						&st_DbgLgTbl,
						&ss_LogIndex);
/* 2010.07.12 加藤木 UV-00794 デバッグログ収集システム対応 End */
}

extern "C" 
{
	void WDAS_SetTrcLogLevel(int si_TrcLogLevel);
	void WDAS_SetErrLogLevel(int si_ErrLogLevel);
#ifdef TORNADO
	void WDAS_TrcLogOutput();
	void WDAS_ErrLogOutput();
#endif
}

void WDAS_SetTrcLogLevel(int si_TrcLogLevel)
{
	si_WDAS_TrcMode = si_TrcLogLevel;
}

void WDAS_SetErrLogLevel(int si_ErrLogLevel)
{
	si_WDAS_ErrMode = si_ErrLogLevel;
}

#ifdef TORNADO
// ADD-S 2009/01/21 NAKAMAURA アンチパターン対応.
#include "system/jlg/APIC_JLG.h"
extern	struct	WDAS_URIUserInfo s_UriUserInfo[] ;
void	WDAS_TableOutput()
{
	ushort i ;
	char	sc_JobLogPath[256+1] ;
	memset(sc_JobLogPath ,0x00, 256) ;
	for (i=0 ; i<3; i++)
	{
		printf("%s\n",(char *)&s_UriUserInfo[i].pu_RequestURI) ;
	}
	APIC_JLG* pc_JLG = APIC_JLG::newInstance();
	if ( pc_JLG == NULL ) {
		printf("APIC_JLG::newInstance error\n") ;
		return ;
	}
	if ( pc_JLG->getWebDAVJobLogPath( sc_JobLogPath, JLGE_Connect_Default ) != 0 ) {
		printf("pc_JLG->getWebDAVJobLogPath error(%s)\n",sc_JobLogPath) ;
//		strcpy(sc_JobLogPath , "http://192.168.242.91:80/DAV/NewJobLog/" ) ;
		return ;
	}
	if ( strlen(sc_JobLogPath) <= 0 ) {
		printf("path get error\n") ;
	}
	printf("path get seikou\n") ;
	printf("--> %s(%d)\n",sc_JobLogPath, strlen(sc_JobLogPath)) ;
	for ( ushort icnt=0; icnt< 256; icnt++) {
		if ( strncmp((char *)&sc_JobLogPath[icnt], "/DAV", 4) == 0 ) {
			break ;
		}
	}
	printf("-> %s <-\n",(char *)&sc_JobLogPath[icnt]) ;
// 2010.08.04 竹内 warning除去対応 Start.
//	ushort copy_size = strlen(sc_JobLogPath) - icnt ;
	ushort copy_size = (ushort)(strlen(sc_JobLogPath) - icnt) ;
// 2010.08.04 竹内 warning除去対応 End.
	memset((char *)&s_UriUserInfo[icnt].pu_RequestURI, 0x00, 256) ;
	strncpy((char *)&s_UriUserInfo[2].pu_RequestURI, (char *)&sc_JobLogPath[icnt], copy_size) ;
	for (i=0 ; i<3; i++)
	{
		printf("%s\n",(char *)&s_UriUserInfo[i].pu_RequestURI) ;
	}
}
// ADD-E 2009/01/21 NAKAMAURA アンチパターン対応.
void WDAS_TrcLogOutput()
{
	printf("=====WDAC_WebDAVdOapDeviceApp Trace Log Start=====\n");
	ushort	us_Index=0;
	for( us_Index=0; us_Index<WDAS_LOG_COUNT; us_Index++ ){
		printf("%s", WDAS_TrcBuf[us_Index].date);
	}
	printf("=====WDAC_WebDAVdOapDeviceApp Trace Log End=====\n");
}

void WDAS_ErrLogOutput()
{
	printf("=====WDAC_WebDAVdOapDeviceApp Error Log Start=====\n");
	ushort	us_Index=0;
	for( us_Index=0; us_Index<WDAS_LOG_COUNT; us_Index++ ){
		printf("%s", WDAS_ErrBuf[us_Index].date);
	}
	printf("=====WDAC_WebDAVdOapDeviceApp Error Log End=====\n");
}
#endif // TORNADO 

//(■■moselコンパイルで回避するための一時的な処置、Thames-iの正式なコンパイルスイッチが出来たら再調整■■)
