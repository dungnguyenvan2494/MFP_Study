//[UV-01324 サーバーレスユビキタスプリントシステム リスト表示遅延課題]start.
////////////////////////////////////////////////////////////////////////////////.
///	@file	PDLC_HookerLog.cpp.
///	@brief	概要：ユビキタスHooker用ログ管理クラス.
/// @date	変更履歴：なし.
////////////////////////////////////////////////////////////////////////////////.
#include <stdio.h>
#include <string.h>
#include "../../system/sys/OsWrapper/SYSC_OSWrapper.h"
#include "PDLC_HookerLog.h"
#include "system/sys/APIC_Enabler.h"
#include "system/sys/APIC_OptionSetting.h"					//	[UV-01324 サーバーレスユビキタスプリントシステム] 制御構成変更No.28対応.
#include "pdl/pdlg/PDLC_PdiCommon.h"						//	新詳細コード対応.
#include "system/sys/APIC_FatalSignalDetailCode.h"			//	新詳細コード対応.

#ifdef LINUX
	#define PDLD_taskIdSelf()		SYSF_taskIdSelf()
#else
	// OSWrapperがSimWinのSYSF_taskIdSelf()に対応していないのでMFCのAPIを使う.
	#define PDLD_taskIdSelf()		AfxGetThread()->m_nThreadID
#endif

extern "C"
{
void PDLG_HookerLogPrint();
void PDLG_HookerLogExecPrint( PDLE_LogForcePrintType e_ForcePrintType );
}


////////////////////////////////////////////////////////////////////////////////.
///	@class	PDLC_LogPrivate
///	@brief	概要：ユビキタスボックス用ログ管理クラス(Private).
////////////////////////////////////////////////////////////////////////////////.
class PDLC_LogPrivate
{
public:
	PDLE_LogForcePrintType	m_e_ForcePrintType;					///< ログ強制出力.
	
	
	////////////////////////////////////////////////////////////////////////////.
	/// @brief	概要：コンストラクタ.
	/// @return	なし.
	/// @par	備考：なし.
	/// @date	変更履歴：なし.
	////////////////////////////////////////////////////////////////////////////.
	PDLC_LogPrivate()
			: m_e_ForcePrintType( PDLE_LogForcePrintType_None )
			, m_ul_LogBuffAddPos( 0 )
//[UV-01324 サーバーレスユビキタスプリントシステム ログ検討]start.
			, m_ul_LogBuffAddPosErr( 0 )
//[UV-01324 サーバーレスユビキタスプリントシステム ログ検討]end.
	{};
	////////////////////////////////////////////////////////////////////////////.
	/// @brief	概要：デストラクタ.
	/// @return	なし.
	/// @par	備考：なし.
	/// @date	変更履歴：なし.
	////////////////////////////////////////////////////////////////////////////.
	virtual ~PDLC_LogPrivate()
	{};

	void Add( PDLE_LogLv e_LogLv, const char* psc_Func, const char* psc_FormatStr, intptr_t sl_Para1, intptr_t sl_Para2, intptr_t sl_Para3, intptr_t sl_Para4, intptr_t sl_Para5, intptr_t sl_Para6 )
	{
		if(( PDLE_LogForcePrintType_Pjl != m_e_ForcePrintType ) &&
		   ( PDLE_LogLvAnalyzePjl == e_LogLv ))
		{
			// 強制出力なし、かつ保存ログではない場合.
			return;
		}
		PDLS_OneLog  s_OneLog;
		
		// ログ1件分データを作成.
		s_OneLog.ul_Tick			= SYSF_tickGet();
		s_OneLog.sl_ThreadID		= PDLD_taskIdSelf();
		s_OneLog.psc_Func			= psc_Func;
		s_OneLog.psc_FormatStr		= psc_FormatStr;
		s_OneLog.sl_Para1			= sl_Para1;
		s_OneLog.sl_Para2			= sl_Para2;
		s_OneLog.sl_Para3			= sl_Para3;
		s_OneLog.sl_Para4			= sl_Para4;
		s_OneLog.sl_Para5			= sl_Para5;
		s_OneLog.sl_Para6			= sl_Para6;
		
		if( PDLE_LogLvAnalyzePjl == e_LogLv )
		{
			// ログ出力のみ.
			printLog( s_OneLog );
		}
		else if( PDLE_LogLvSavePrint == e_LogLv )
		{
			// ログ保存と、強制出力フラグがONなら出力.
			saveLog( s_OneLog );
			
			if( PDLE_LogForcePrintType_None != m_e_ForcePrintType )
			{
				printLog( s_OneLog );
			}
//[UV-01324 サーバーレスユビキタスプリントシステム ログ検討]start.
		}
		else if( PDLE_LogLvError == e_LogLv )
		{
			// ログ保存.
			saveLog( s_OneLog );

			// エラーログの場合は別途保存と、強制出力フラグがONなら出力.
			saveLogErr( s_OneLog );
			
			if( PDLE_LogForcePrintType_None != m_e_ForcePrintType )
			{
				printLog( s_OneLog );
			}
//[UV-01324 サーバーレスユビキタスプリントシステム ログ検討]end.
		}
	}
	
	
	void printAllLog()
	{
//[UV-01324 サーバーレスユビキタスプリントシステム ログ検討]start.
		printf( "------------------------------------------------------------\n" );
		printf( "PDLC_HookerLog start\n" );
		printf( "------------------------------------------------------------\n" );
//[UV-01324 サーバーレスユビキタスプリントシステム ログ検討]end.
		uint32_t ul_tempLogBuffAddPos = m_ul_LogBuffAddPos;
		
		for( int si_loop = 0; si_loop < PDLD_LogNumMax; si_loop++)
		{
			if( NULL != m_s_LogBuf[ ul_tempLogBuffAddPos ].psc_Func )
			{
				// ログ出力.
				printLog( m_s_LogBuf[ ul_tempLogBuffAddPos ] );
			}
			
			ul_tempLogBuffAddPos = ( ul_tempLogBuffAddPos + 1 ) % (uint32_t)PDLD_LogNumMax;
		}
//[UV-01324 サーバーレスユビキタスプリントシステム ログ検討]start.
		printf( "------------------------------------------------------------\n" );
		printf( "PDLC_HookerLog end\n" );
		printf( "------------------------------------------------------------\n" );
		printf( "------------------------------------------------------------\n" );
		printf( "PDLC_HookerErrorLog start\n" );
		printf( "------------------------------------------------------------\n" );
		uint32_t ul_tempLogBuffAddPosErr = m_ul_LogBuffAddPosErr;
// ユビキタス[RQ-673][Multitasking of LK-114 storage function] start.
//		for( si_loop = 0; si_loop < PDLD_LogNumMax; si_loop++)
		for( si_loop = 0; si_loop < PDLD_LogNumMaxErr; si_loop++)
// ユビキタス[RQ-673][Multitasking of LK-114 storage function] end.
		{
			if( NULL != m_s_LogBufErr[ ul_tempLogBuffAddPosErr ].psc_Func )
			{
				// ログ出力.
				printLog( m_s_LogBufErr[ ul_tempLogBuffAddPosErr ] );
			}
			
// ユビキタス[RQ-673][Multitasking of LK-114 storage function] start.
//			ul_tempLogBuffAddPosErr = ( ul_tempLogBuffAddPosErr + 1 ) % (uint32_t)PDLD_LogNumMax;
			ul_tempLogBuffAddPosErr = ( ul_tempLogBuffAddPosErr + 1 ) % (uint32_t)PDLD_LogNumMaxErr;
// ユビキタス[RQ-673][Multitasking of LK-114 storage function] end.
		}
		printf( "------------------------------------------------------------\n" );
		printf( "PDLC_HookerErrorLog end\n" );
		printf( "------------------------------------------------------------\n" );
//[UV-01324 サーバーレスユビキタスプリントシステム ログ検討]end.
	}


private:
	
	struct PDLS_OneLog
	{
		uint32_t		ul_Tick;			///< 受付時のtick.
		THR_ID				sl_ThreadID;		///< 対象スレッドID.
		const char*			psc_Func;			///< 関数名.
		const char*			psc_FormatStr;		///< 書式付き文字列.
		intptr_t			sl_Para1;			///< 書式付き文字列のパラメータ1.
		intptr_t			sl_Para2;			///< 書式付き文字列のパラメータ2.
		intptr_t			sl_Para3;			///< 書式付き文字列のパラメータ3.
		intptr_t			sl_Para4;			///< 書式付き文字列のパラメータ4.
		intptr_t			sl_Para5;			///< 書式付き文字列のパラメータ5.
		intptr_t			sl_Para6;			///< 書式付き文字列のパラメータ6.
		////////////////////////////////////////////////////////////////////////////.
		/// @brief	概要：コンストラクタ.
		/// @return	なし.
		/// @par	備考：なし.
		/// @date	変更履歴：なし.
		////////////////////////////////////////////////////////////////////////////.
		PDLS_OneLog()
				: ul_Tick( 0 )
				, sl_ThreadID( 0 )
				, psc_Func( NULL )
				, psc_FormatStr( NULL )
				, sl_Para1( 0 )
				, sl_Para2( 0 )
				, sl_Para3( 0 )
		{};
	};

	PDLS_OneLog					m_s_LogBuf[ PDLD_LogNumMax ];		///< ログ格納バッファ.
	uint32_t						m_ul_LogBuffAddPos;					///< ログバッファスタート位置.
//[UV-01324 サーバーレスユビキタスプリントシステム ログ検討]start.
// ユビキタス[RQ-673][Multitasking of LK-114 storage function] start.
//	PDLS_OneLog					m_s_LogBufErr[ PDLD_LogNumMax ];	///< エラーログ格納バッファ.
	PDLS_OneLog					m_s_LogBufErr[ PDLD_LogNumMaxErr ];	///< エラーログ格納バッファ.
// ユビキタス[RQ-673][Multitasking of LK-114 storage function] end.
	uint32_t						m_ul_LogBuffAddPosErr;				///< エラーログバッファスタート位置.
//[UV-01324 サーバーレスユビキタスプリントシステム ログ検討]end.


	void saveLog( PDLS_OneLog s_OneLog )
	{
		// ログをバッファに追加する.
		m_s_LogBuf[ m_ul_LogBuffAddPos ] = s_OneLog;
		
		m_ul_LogBuffAddPos = (m_ul_LogBuffAddPos + 1) % (uint32_t)PDLD_LogNumMax;
	}

//[UV-01324 サーバーレスユビキタスプリントシステム ログ検討]start.
	void saveLogErr( PDLS_OneLog s_OneLog )
	{
		// ログをバッファに追加する.
		m_s_LogBufErr[ m_ul_LogBuffAddPosErr ] = s_OneLog;
// ユビキタス[RQ-673][Multitasking of LK-114 storage function] start.
//		m_ul_LogBuffAddPosErr = (m_ul_LogBuffAddPosErr + 1) % (uint32_t)PDLD_LogNumMax;
		m_ul_LogBuffAddPosErr = (m_ul_LogBuffAddPosErr + 1) % (uint32_t)PDLD_LogNumMaxErr;
// ユビキタス[RQ-673][Multitasking of LK-114 storage function] end.
	}
//[UV-01324 サーバーレスユビキタスプリントシステム ログ検討]end.

	void printLog( PDLS_OneLog s_OneLog )
	{
		// ログを出力する.
		// 書式付文字列用バッファを作成する.
		static const char* psc_headerStr = "0x%10lu|%016p|%35s|";
		int32_t sl_BuffSize = (int32_t)strlen( psc_headerStr );
		
		if(( !s_OneLog.psc_Func ) || ( !s_OneLog.psc_FormatStr ))
		{
			// 設定文字列が無い場合は関数を終了する.
			return;
		}
		
		sl_BuffSize = (int32_t)( sl_BuffSize + strlen( s_OneLog.psc_FormatStr ) + strlen("\n") + 1 );
		char* psc_str = new char[ sl_BuffSize ];
		
		if( !psc_str )
		{
			return;
		}
		// 出力用文字列作成.
		sprintf( psc_str, "%s%s\n", psc_headerStr, s_OneLog.psc_FormatStr );
		psc_str[ sl_BuffSize - 1 ] = 0;
		
		// ログ出力.
		printf( psc_str, s_OneLog.ul_Tick, s_OneLog.sl_ThreadID, s_OneLog.psc_Func,
			 s_OneLog.sl_Para1, s_OneLog.sl_Para2, s_OneLog.sl_Para3, s_OneLog.sl_Para4, s_OneLog.sl_Para5, s_OneLog.sl_Para6 );
		
		// 後始末.
		delete [] psc_str;
	}


};


////////////////////////////////////////////////////////////////////////////.
// staticインスタンス.
////////////////////////////////////////////////////////////////////////////.
// 2014.02.25 m.umeno <START> ユビキタスプリント未使用時メモリ削減.
static PDLC_LogPrivate* pc_g_LogPrivate = NULL;		///< ログ管理クラスのポインタ.
// 2014.02.25 m.umeno <E N D> ユビキタスプリント未使用時メモリ削減.

//	新詳細コード ソースNo <->ファイル名 変換テーブル(PDLC_PdiCommon.cppの定義参照).
extern char s_ChangeNoToFileName[ PDLD_SRCNOMAX ][ PATH_MAX_KM + 1 ];
////////////////////////////////////////////////////////////////////////////.
/// @brief	概要：ログ登録.
/// @return	なし.
/// @param[in] psc_Func			関数名.
/// @param[in] psc_FormatStr	ログに出力する、書式付き文字列.
/// @param[in] sl_Para1			書式付き文字列のパラメータ1.
/// @param[in] sl_Para1			書式付き文字列のパラメータ2.
/// @param[in] sl_Para1			書式付き文字列のパラメータ3.
/// @param[in] sl_Para1			書式付き文字列のパラメータ4.
/// @param[in] sl_Para1			書式付き文字列のパラメータ5.
/// @param[in] sl_Para1			書式付き文字列のパラメータ6.
/// @par	備考：なし.
/// @date	変更履歴：20160513	CQ529265		新詳細コード対応.
////////////////////////////////////////////////////////////////////////////.
void PDLG_AddHookerLog( PDLE_LogLv e_LogLv, const char* psc_Func, char* psc_FileName, uint32_t ul_line, const char* psc_FormatStr, intptr_t sl_Para1, intptr_t sl_Para2, intptr_t sl_Para3, intptr_t sl_Para4, intptr_t sl_Para5, intptr_t sl_Para6 )
{
// 2014.02.25 m.umeno <START> ユビキタスプリント未使用時メモリ削減.
	if( NULL == pc_g_LogPrivate ){
		if( NULL == PDLG_HookerLogInit() ){
			return;
		}
	}
	// 新詳細コード登録.
	if( PDLE_LogLvError == e_LogLv ){
		uint64_t	ull_NewDetailCode	= 0;	// 新詳細コード 16進数、16桁(8byte).
		ushort		us_FileNo			= 0;	// ソースNo.
		ushort		us_FileIdx			= 0;	// 変換テーブルIndex.
		uint32_t	ul_NewDetailCodeU	= 0;	// 新詳細コード 上位8桁.
		uint32_t	ul_NewDetailCodeL	= 0;	// 新詳細コード 下位8桁.

		for ( us_FileIdx = 0; us_FileIdx < PDLD_SRCNOMAX; us_FileIdx++ )
		{
			if ( strstr( s_ChangeNoToFileName[us_FileIdx], psc_FileName ) != NULL )
			{	// ファイル名がヒット.
				us_FileNo = static_cast<ushort>(us_FileIdx + 1);
				break;
			}
		}
		ul_NewDetailCodeU	|= ( (uint32_t)PDLD_MODULE_NO << 3 * 8 );		// 新詳細コード 先頭1  ~ 2桁 (1byte): モジュールNo.
		ul_NewDetailCodeL	|= ( (uint32_t)us_FileNo << 2 * 8 );			// 新詳細コード 先頭11 ~ 12桁(1byte): ソースNo.
		ul_NewDetailCodeL	|= (ushort)ul_line;								// 新詳細コード 先頭13 ~ 16桁(2byte): ライン数.

		ull_NewDetailCode	= ((uint64_t)ul_NewDetailCodeU << 4 * 8);
		ull_NewDetailCode	|= (uint64_t)ul_NewDetailCodeL;

		// 新詳細コードをセット(旧詳細コード指定無しのため、0).
		APIC_setNewDetailCode( 0, ull_NewDetailCode );
	}
	// ログを追加する.
	pc_g_LogPrivate->Add( e_LogLv, psc_Func, psc_FormatStr, sl_Para1, sl_Para2, sl_Para3, sl_Para4, sl_Para5, sl_Para6 );
// 2014.02.25 m.umeno <E N D> ユビキタスプリント未使用時メモリ削減.
}

////////////////////////////////////////////////////////////////////////////.
/// @brief	概要：ログ出力.
/// @return	なし.
/// @par	備考：なし.
/// @date	変更履歴：なし.
////////////////////////////////////////////////////////////////////////////.
void PDLG_HookerLogPrint()
{
// 2014.02.25 m.umeno <START> ユビキタスプリント未使用時メモリ削減.
	if( NULL == pc_g_LogPrivate ){
		if( NULL == PDLG_HookerLogInit() ){
			return;
		}
	}
	// 全ログを出力する.
	pc_g_LogPrivate->printAllLog();
// 2014.02.25 m.umeno <E N D> ユビキタスプリント未使用時メモリ削減.
};


////////////////////////////////////////////////////////////////////////////.
/// @brief	概要：ログを実行中ログとして出力するか.
/// @return	なし.
/// @param[in] e_ForcePrintType		PDLE_LogForcePrintType_None    ：実行中ログの強制表示無し\n.
/// @par	備考：なし.
/// @date	変更履歴：なし.
////////////////////////////////////////////////////////////////////////////.
void PDLG_HookerLogExecPrint( PDLE_LogForcePrintType e_ForcePrintType )
{
// 2014.02.25 m.umeno <START> ユビキタスプリント未使用時メモリ削減.
	if( NULL == pc_g_LogPrivate ){
		if( NULL == PDLG_HookerLogInit() ){
			return;
		}
	}
	// ログ出力タイプ更新.
	pc_g_LogPrivate->m_e_ForcePrintType = e_ForcePrintType;
// 2014.02.25 m.umeno <E N D> ユビキタスプリント未使用時メモリ削減.
};

// 2014.02.25 m.umeno <START> ユビキタスプリント未使用時メモリ削減.
////////////////////////////////////////////////////////////////////////////.
/// @brief	概要：ログ管理クラスの実体を準備する.
/// @return	なし.
/// @par	備考：なし.
/// @date	変更履歴：なし.
////////////////////////////////////////////////////////////////////////////.
PDLC_LogPrivate* PDLG_HookerLogInit()
{
//[UV-01324 サーバーレスユビキタスプリントシステム] 制御構成変更No.28対応 start.
//	// 増設メモリが付いていなければインスタンスを動的確保しない.
//	if( True == APIC_Enabler::newInstance()->isExtendedMemory() && NULL == pc_g_LogPrivate ){
	// ストレージ機能が有効でなければインスタンスを動的確保しない.
	if( TYPE_AppOptSettingOn == SYSF_getAppOptionSetting(TYPE_AppOptionUbiquitousStorage) && NULL == pc_g_LogPrivate ){
//[UV-01324 サーバーレスユビキタスプリントシステム] 制御構成変更No.28対応 end.
		pc_g_LogPrivate = new PDLC_LogPrivate;
	}
	return pc_g_LogPrivate;
}
// 2014.02.25 m.umeno <END> ユビキタスプリント未使用時メモリ削減.
//[ユビキタスプリントstep2 リスト表示遅延課題]end.