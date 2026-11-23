////////////////////////////////////////////////////////////////////////////////////////////////////.
///.
///		\file		FBP_FallbackManager.cpp.
///		\breif		概要：Fallbackモード制御クラス.
///		\author		Sky.
/// 	\data		2018/01/22.
///.
////////////////////////////////////////////////////////////////////////////////////////////////////.
#include <QUrl>
#include "FBP_Define.h"
#include "FBP_FallbackIpcForBrw.h"
#include "FBP_FallbackManager.h"
#include "FBP_FallbackIpcForMfpWatch.h"
#include "FBP_DebugPrint.h"
#include <errno.h>

#include "system/sys/APIC_HddInfo.h"
#include "system/nvd/APIC_Setting_MachineSetting.h"
#include "system/nvd/APIC_Setting_SystemSetting.h"
#include "system/nvd/APIC_Setting_SoftDipSw.h"
#include "system/nvd/APIC_Setting_Network.h"
#include "system/nvd/APIC_Setting_Other.h"
#include "system/nvd/APIC_ExtSetting.h"
#include "system/nvd/APIC_Setting_AppInfoSetting.h"
#include "system/nvd/APIC_Setting_IPv6Setting.h"
#include "system/uac/APIC_UserAcount.h"
#include "system/pwr/pwrg/PWRC_Power.h"
#include "system/cnt/CNTC_CntIf.h"
#include "system/sys/APIC_IwsUserAccount.h"

#include "include/LAPC_BFW_System.h"
#include "GFWC_System.h" 

#ifdef LINUX
//mpc種別に従い、フォントファイル格納パスを返却する関数
extern const char* getFontPathType();
#endif

template<typename T>
void ZEROCLEAR( T& dest ) {
	memset( &dest, 0x00, sizeof( T ) );
}

static const int32_t FBPD_FALLBACK_STACKSIZE	= (1024*50);								// タスクのスタックサイズ.

BRWS_CertData*			FBPC_FallbackManager::m_ps_ShmCertData = NULL;			// 検証する証明書データ格納用共有メモリ(静的変数)の初期化.
Bool					FBPC_FallbackManager::m_b_AutoClearProcessed = False;	// オートクリアフラグ(静的変数)の初期化.

MSG_ID					FBPC_FallbackManager::m_msg_Snd_ID = -1;				// メッセージキューID(送信用).
FBPS_DirPath			FBPC_FallbackManager::m_st_DirPath;						// ディレクトリ種別毎のディレクトリパス文字列(静的変数)の参照宣言.
BRWS_InitialSettingInfo	FBPC_FallbackManager::m_s_InitialSettingInfo;			// 起動設定(静的変数)の参照宣言.

//--------------------------------------------------------------------------------------------.
///	\brief	機能：インスタンス取得.
///	\par	備考：.
///	\param	なし.
///	\result	FBPC_FallbackKeepData* インスタンスポインタ.
//--------------------------------------------------------------------------------------------.
FBPC_FallbackKeepData* FBPC_FallbackKeepData::newInstance()
{
	static FBPC_FallbackKeepData c_Ins;
	return &c_Ins;
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：ブラウザ再起動パラメータ構造体の保存.
///	\par	備考：.
///	\param	BRWS_RebootSettingInfo* ps_info [I] ： ブラウザ再起動パラメータ構造体へのポインタ.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void FBPC_FallbackKeepData::setReqRebootInfo( const BRWS_RebootSettingInfo* ps_info )
{
	FBPD_TRACE_SCOPE();

	if( NULL == ps_info ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackKeepData::setReqRebootInfo arg Null Error!!\n" );
		return;
	}

	memcpy( &data.m_st_RebootSettingInfo, ps_info, sizeof(BRWS_RebootSettingInfo) );
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：通常ブラウザ起動回数設定.
///	\par	備考：.
///	\param	short ss_Freq [I] ： 回数.
///							FBPD_START_FIRST:1回目.
///							FBPD_START_MORE :2回目以上.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void FBPC_FallbackKeepData::setStartFrequency( short ss_Freq )
{
	FBPD_TRACE_SCOPE();

	data.m_ss_StartFrequency = ss_Freq;
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：ブラウザ音量の取得.
///	\par	備考：.
///	\param	なし.
///	\result	short ブラウザ音量.
//--------------------------------------------------------------------------------------------.
short FBPC_FallbackKeepData::getBrowserVolume()
{
	FBPD_TRACE_SCOPE();

	return data.m_ss_BrowserVolume;
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：通常ブラウザ起動回数取得.
///	\par	備考：.
///	\param	なし.
///	\result	short 回数.
///						FBPD_START_FIRST:1回目.
///						FBPD_START_MORE :2回目以上.
//--------------------------------------------------------------------------------------------.
short FBPC_FallbackKeepData::get_StartFrequency()
{
	FBPD_TRACE_SCOPE();

	return data.m_ss_StartFrequency;
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：ブラウザ再起動パラメータ構造体のの取得.
///	\par	備考：.
///	\param	なし.
///	\result	BRWS_RebootSettingInfo* ブラウザ再起動パラメータ構造体のへのポインタ.
//--------------------------------------------------------------------------------------------.
BRWS_RebootSettingInfo* FBPC_FallbackKeepData::getReqRebootInfo()
{
	FBPD_TRACE_SCOPE();

	return &data.m_st_RebootSettingInfo;
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：コンストラクタ.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
FBPC_FallbackKeepData::FBPC_FallbackKeepData()
{
	ZEROCLEAR(data);

	data.m_ss_BrowserVolume		= TYPE_VoiceVolume5;
	data.m_ss_StartFrequency	= FBPD_START_FIRST;

    data.m_st_RebootSettingInfo.e_BrowserBootMode 							= BRWE_BrowserBootMode_Default;
	data.m_st_RebootSettingInfo.ss_BrowserTimeOut							= 0;
	data.m_st_RebootSettingInfo.e_Cookie									= TYPE_ExtCookieTypeReceive;
	data.m_st_RebootSettingInfo.s_ToolBarSetting.e_ToolBarSettingType		= TYPE_ExtToolBarSettingTypeHide;
	data.m_st_RebootSettingInfo.s_StatusBarSetting.e_StatusBarSettingType	= TYPE_ExtStatusBarSettingTypeHide;
	data.m_st_RebootSettingInfo.sc_LauncherAplNo							= '\0';
	data.m_st_RebootSettingInfo.sl_TimeDifference							= 0;
	data.m_st_RebootSettingInfo.b_OpenApiProxyEnable						= False;

	memset( data.m_st_RebootSettingInfo.sc_BrowserURL, 		'\0', sizeof(data.m_st_RebootSettingInfo.sc_BrowserURL)		); 
	memset( data.m_st_RebootSettingInfo.sc_SessionID, 		'\0', sizeof(data.m_st_RebootSettingInfo.sc_SessionID)		); 
	memset( data.m_st_RebootSettingInfo.sc_NoHostAddress, 	'\0', sizeof(data.m_st_RebootSettingInfo.sc_NoHostAddress)	); 
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：クラスの初期化.
///	\par	備考：.
///	\param	なし.
///	\result	short 処理結果.
///				FBPD_OK		:正常.
///				FBPD_ERROR	:異常.
//--------------------------------------------------------------------------------------------.
short FBPC_FallbackManager::initClass()
{
	FBPD_TRACE_SCOPE();

	// 未初期化時のみ.
	if ( False == m_b_Init ) {
		m_b_Init = True;

		/*----------------------------------------------------------------------*/
		/*		メッセージキューオープン										*/
		/*----------------------------------------------------------------------*/
		// (受信用) Fallbackタスクへの要求メッセージ キューオープン.
		m_msg_Q_ID = FBPG_mqOpen( (char*)TEXT(FBPD_IPC_MSGQ_NAME_FALLBACK_REQ), FBPD_EVENT_MSG_MAX_FALLBACK_REQ, sizeof(BRWC_BrowserMsg) );
		if ( FBPD_ERROR == m_msg_Q_ID ) {
			FBPG_DebugPrint( 3, "FBPC_FallbackManager::initClass  MsgQ CreateError!  errno[%d]\n", errno );
			return FBPD_ERROR;
		}

		/*----------------------------------------------------------------------*/
		/*		タスク生成														*/
		/*----------------------------------------------------------------------*/
		int32_t ret = FBPG_threadCreateWithStackSize( ReceiveEvent, FBPD_FALLBACK_STACKSIZE, FBPD_TaskName_ReceiverForFbp );	// [MFPソリューション分離step1] IT不具合対応.
		if ( FBPD_ERROR == ret ) {
			FBPG_DebugPrint( 3, "FBPC_FallbackIpcForBrw::initClass  Thread Create Err! errno[%d]\n", errno );
			return FBPD_ERROR;
		}

		/*----------------------------------------------------------------------*/
		/*		検証する証明書データ格納用共有メモリアドレスの取得				*/
		/*----------------------------------------------------------------------*/
		static int32_t si_shmFd = FBPG_shmOpen( (char*)TEXT(FBPD_IPC_SHM_MFPS_BRW_CERTDATA), O_CREAT | O_RDWR,  S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, True );
		if (FBPD_ERROR == si_shmFd) {
			FBPG_DebugPrint( 3, "[%s]FBPG_shmOpen error errno[%d].\n", __PRETTY_FUNCTION__, errno);
			return FBPD_ERROR;
		}
		m_ps_ShmCertData = (BRWS_CertData*)FBPG_mmap(NULL, ( sizeof(BRWS_CertData) * FBPD_BrwCertChainMax ), PROT_READ | PROT_WRITE, MAP_SHARED, si_shmFd, 0);
		if ((BRWS_CertData*)FBPD_ERROR == m_ps_ShmCertData ) {
			FBPG_DebugPrint( 3, "[%s]FBPG_mmap error errno[%d].\n", __PRETTY_FUNCTION__, errno);
			return FBPD_ERROR;
		}
	}
	return FBPD_OK;
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：インスタンスの取得.
///	\par	備考：.
///	\param	なし.
///	\result	FBPC_FallbackManager* インスタンスポインタ.
//--------------------------------------------------------------------------------------------.
FBPC_FallbackManager* FBPC_FallbackManager::newInstance()
{
	static FBPC_FallbackManager c_Ins;
	return &c_Ins;
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：メッセージ送信（FallbackManager宛て）.
///	\par	備考：.
///	\param	BRWC_BrowserMsg* pst_Data [I] ： 送信メッセージ.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void FBPC_FallbackManager::SendMsgFallbackManager( BRWC_BrowserMsg* pst_Data )
{
	FBPD_TRACE_SCOPE();

	if( NULL == pst_Data ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackManager::SendMsgFallbackManager arg Null Error!!\n" );
		return;
	}

	//FBPC_FallbackManagerへメッセージ送信.
	FBPG_mqSend( m_msg_Q_ID, ( char* )pst_Data, sizeof( BRWC_BrowserMsg ), FBPD_MSQ_FIFO );
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：Fallbackモード要求.
///	\par	備考：.
///	\param	True:MFP異常 / False:MFP正常.
///	\result	short 処理結果.
///				FBPD_OK		:正常.
///				FBPD_ERROR	:異常.
//--------------------------------------------------------------------------------------------.
short FBPC_FallbackManager::setFallbackMode( Bool b_FallbackMode )
{
	FBPD_TRACE_SCOPE();

	int32_t si_shmFd = FBPG_shmOpen( ( char* )WKBD_IPC_SHM_BRW_MFPWATCH_FALLBACK, O_CREAT | O_RDWR, S_IRUSR | S_IWUSR | S_IRGRP | S_IWGRP | S_IROTH | S_IWOTH, True  );
	if( -1 == si_shmFd ){
		return FBPD_ERROR;
	}

	WKBS_FallbackInfo* ps_info = ( WKBS_FallbackInfo* )FBPG_mmap( NULL, sizeof( WKBS_FallbackInfo ), PROT_READ | PROT_WRITE, MAP_SHARED, si_shmFd, 0 );
	if( ( WKBS_FallbackInfo* )-1 == ps_info ) {
// [MFPソリューション分離step1] 静的解析対応 Start.
		close( si_shmFd );
// [MFPソリューション分離step1] 静的解析対応 End.
		return FBPD_ERROR;
	}
	ps_info->b_FallbackMode = b_FallbackMode;
	
// [MFPソリューション分離step1] 静的解析対応 Start.
	close( si_shmFd );
// [MFPソリューション分離step1] 静的解析対応 End.
	return FBPD_OK;
}

/*============================================================================================*/
/* WebKitBrowser -> FallBack イベントハンドラ                                                 */
/*============================================================================================*/
//--------------------------------------------------------------------------------------------.
///	\brief	機能：ブラウザの取扱うファイル拡張子の判定処理を行う.
///	\par	備考：.
///	\param	char* psc_FileName       [I] ： ファイル名.
///	\param	char* psc_FileExtension  [O] ： 拡張子名   ※確保するバッファサイズにはFBPD_EXTENTION_LENGTH_MAXを使用する事.
///	\result	Bool 判定結果
///				True		: 許可.
///				False		: 禁止.
//--------------------------------------------------------------------------------------------.
Bool FBPC_FallbackManager::CheckExtensionFunc( char* psc_FileName, char* psc_FileExtension )
{
	FBPD_TRACE_SCOPE();

	char* psc_Externt = NULL;									// 拡張子検出用.
	char sc_ExterntName[FBPD_EXTENTION_LENGTH_MAX] = {};		// 拡張子格納用.
	ushort us_FileType = 0;										// 拡張子番号格納.

	// パラメータチェック.
	if ( NULL == psc_FileName || NULL == psc_FileExtension ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackManager::CheckExtensionFunc arg Null Error!!\n" );
		return False;
	}

	// ファイル名から「.」までのポインタを求める.
	psc_Externt = strrchr( psc_FileName, '.' );

	// ファイル名に「.」がない場合.
	if ( NULL == psc_Externt ) {
		return False;
	}

	// 「.」の分を進める.
	psc_Externt++;

	// ファイル拡張子をコピー.
	strncpy( sc_ExterntName, psc_Externt, sizeof(sc_ExterntName)-1 );

	// 拡張子を小文字に変換.
	for ( psc_Externt = sc_ExterntName; *psc_Externt != '\0';  psc_Externt++ ) {
		*psc_Externt = (char)( tolower(*psc_Externt) );
	}

	// 拡張子リストのサイズ算出.
	ushort us_ListSize = 0;			// リストの個数.
	us_ListSize = sizeof( e_BrowserExtensionCheckList ) / sizeof( FBPS_EXTENTION_STR_CHECK );

	// ファームウェアバージョン取得.
	uchar uc_MZFWVersion = getMZFWVersion();

	// ブラウザの許可拡張子リストとの比較.
	for( int32_t i = 0; i < us_ListSize ; i++ ) {
		if ( e_BrowserExtensionCheckList[i].uc_MZFWVersion <= uc_MZFWVersion ) {
			// 適用ファームウェアバージョンが、現在のファームウェアバージョン以下の場合のみ、処理を実行する.
			int32_t res = 1;
			res = strcmp( sc_ExterntName, e_BrowserExtensionCheckList[i].sc_ExtentName );
			if ( 0 == res ) {
				switch( i ) {
				case 0:
				case 1:
					us_FileType = FBPD_FILE_TYPE_PDF;
					break;
				default:
					us_FileType = FBPD_FILE_TYPE_NONE;
					break;
				}
				if ( FBPD_FILE_TYPE_NONE != us_FileType ) {
					// 拡張子をピリオド込みでセット.
					snprintf( psc_FileExtension, FBPD_EXTENTION_LENGTH_MAX, ".%s", e_BrowserExtensionCheckList[i].sc_ExtentName );
				}
				return True;
			}
		}
	}
	return False;
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：PDFファイルリンク開始要求を行う.
///	\par	備考：.
///	\param	char*	psc_FileName	[I] ： PDFファイル名.
///	\result	Bool 開始許可フラグ.
///				True		: 開始可能.
///				False		: 開始不可.
//--------------------------------------------------------------------------------------------.
Bool FBPC_FallbackManager::FileLinkStartReq( char* psc_FileName )
{
	FBPD_TRACE_SCOPE();

	char sc_Extention[FBPD_EXTENTION_LENGTH_MAX] = {};	// 拡張子格納バッファ.

	// パラメータチェック.
	if ( psc_FileName == NULL ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackManager::FileLinkStartReq arg Null Error!!\n" );
		return False;
	}

	FBPG_DebugPrint( 3, "FBPC_FallbackManager::FileLinkStartReq [FileName = %s]!!\n" , psc_FileName );
	// ファイル拡張子の判定.
	if ( True != CheckExtensionFunc( psc_FileName, sc_Extention ) ) { FBPG_DebugPrint( 3, " File Link Extension Error !! \n" ); return False; }

	// 自タスクにメッセージ送信.
	BRWC_BrowserMsg st_Msg = {};
	st_Msg.ss_type = BR_Msg_FileLinkCallBackFunc;

	st_Msg.s_InParam.uc_UcParam[0]  = BRWE_FLINK_MODE_DISPLAY;
	SendMsgFallbackManager( &st_Msg );

	return True;
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：ブラウザ起動応答.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void FBPC_FallbackManager::BrowserInitializeNotice()
{
	FBPD_TRACE_SCOPE();

	BRWC_BrowserMsg st_Msg = {};
	st_Msg.ss_type = BR_Msg_BrowserInitializeNotice;

	SendMsgFallbackManager( &st_Msg );
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：ブラウザ終了応答.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void FBPC_FallbackManager::BrowserFinalizeNotice()
{
	FBPD_TRACE_SCOPE();

	BRWC_BrowserMsg st_Msg = {};
	st_Msg.ss_type = BR_Msg_BrowserFinalizeNotice;

	SendMsgFallbackManager( &st_Msg );
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：描画開始/停止応答.
///	\par	備考：.
///	\param	uchar uc_Kind [I] ： 応答種別.
///	\param	uchar uc_Resp [I] ： 開始/停止結果.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void FBPC_FallbackManager::DrawControlNotice( uchar uc_Kind , uchar uc_Resp )
{
	FBPD_TRACE_SCOPE();

	switch( uc_Kind ) {
	case BRWE_DACT_KIND_START:
		if ( uc_Resp == BRWE_DACT_RESP_OK ) {
			{
				BRWC_BrowserMsg st_Msg = {};
				st_Msg.ss_type = BR_Msg_DrawStartNotice;
				SendMsgFallbackManager( &st_Msg );
			}
			{
				BRWC_BrowserMsg st_Msg = {};
				st_Msg.ss_type = BR_Msg_SoundControlStartNotice;
				SendMsgFallbackManager( &st_Msg );
			}
		} else {
			// DrawStartのNGはありえない.
		}
		break;
	case BRWE_DACT_KIND_STOP:
		if ( uc_Resp == BRWE_DACT_RESP_OK ) {
			// DrawStopの場合はサウンドを停止させる.
			BRWC_BrowserMsg st_Msg = {};
			st_Msg.ss_type = BR_Msg_SoundControlStopNotice;
			SendMsgFallbackManager( &st_Msg );
		} else {
			// DrawStartのNGはありえない.
		}
		break;
	default:
		// DrawStartのNGはありえない.
		break;
	}
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：ファームウェアバージョン取得.
///	\par	備考：.
///	\param	なし.
///	\result	uchar ファームウェアバージョン.
//--------------------------------------------------------------------------------------------.
uchar FBPC_FallbackManager::getMZFWVersion()
{
	FBPD_TRACE_SCOPE();

	return BRWE_MZFWVer_002;
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：ブラウザで取扱うファイル拡張子リストの取得.
///	\par	備考：.
///	\param	BRWS_EXTENTION_STR* ps_ExterntList [O] ： ファイル拡張子リスト.
///	\result	ushort リストの要素数.0
//--------------------------------------------------------------------------------------------.
ushort FBPC_FallbackManager::GetExtensionList( BRWS_EXTENTION_STR* ps_ExterntList )
{
	FBPD_TRACE_SCOPE();

	// パラメータチェック.
	if ( ps_ExterntList == NULL ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackManager::GetExtensionList arg Null Error!!\n" );
		return 0;
	}

	ushort us_ListSize = sizeof( e_BrowserExtensionCheckList ) / sizeof( FBPS_EXTENTION_STR_CHECK );

	// ファームウェアバージョン取得.
	uchar uc_MZFWVersion = getMZFWVersion();

	for( int32_t i = 0; i < us_ListSize ; i++ ) {
		if ( e_BrowserExtensionCheckList[i].uc_MZFWVersion <= uc_MZFWVersion ) {
			// 適用ファームウェアバージョンが、現在のファームウェアバージョン以下の場合のみ、コピー処理を実行する.
			// 拡張子名長をコピー.
			ps_ExterntList[i].us_ExtentLen = e_BrowserExtensionCheckList[i].us_ExtentLen;
			// 拡張子名をコピー.
			memcpy( ps_ExterntList[i].sc_ExtentName , e_BrowserExtensionCheckList[i].sc_ExtentName , e_BrowserExtensionCheckList[i].us_ExtentLen );
		}
	}

	return us_ListSize;
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：ブラウザエンジン再起動.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void FBPC_FallbackManager::ReqRebootBrowserEngine()
{
	FBPD_TRACE_SCOPE();

	BRWC_BrowserMsg st_Msg = {};
	st_Msg.ss_type = BR_Msg_BrowserReboot;

	SendMsgFallbackManager( &st_Msg );
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：中間証明書のリスト登録.
///	\par	備考：.
///	\param	uchar*             puc_CertData   [I]   ： 証明書データ.
///	\param	uint32_t           ul_CertSize    [I]   ： 証明書データサイズ.
///	\param	LAPS_NET_CertData* ps_MioCertData [I/O] ： 証明書データリスト.
///	\result	LAPS_NET_CertData*	証明書データリストの先頭ポインタ.
//--------------------------------------------------------------------------------------------.
LAPS_NET_CertData* FBPC_FallbackManager::addCertChainList( uchar* puc_CertData, uint32_t ul_CertSize, LAPS_NET_CertData* ps_MioCertData )
{
	FBPD_TRACE_SCOPE();

	// ps_MioCertDataのNULLチェックはしないこと.
	if( NULL == puc_CertData ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackManager::addCertChainList arg Null Error!!\n" );
		return ps_MioCertData;
	}
	
	LAPS_NET_CertData* ps_Data = NULL;
	ps_Data = (LAPS_NET_CertData*)malloc( sizeof(LAPS_NET_CertData) );
	if ( ps_Data ) {
		// リストにデータ設定.
		ps_Data->uiSize = ul_CertSize;
		ps_Data->pCert = puc_CertData;
		// 今までの先頭ポインタを次ポインタに設定.
		ps_Data->pNext = ps_MioCertData;
		// 先頭ポインタを変更.
		ps_MioCertData = ps_Data;
	}
	
	return ps_MioCertData;
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：中間証明書のリスト解放.
///	\par	備考：.
///	\param	LAPS_NET_CertData* ps_MioCertData [I/O] ： 証明書データリスト.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void FBPC_FallbackManager::freeCertChainList( LAPS_NET_CertData* ps_MioCertData )
{
	FBPD_TRACE_SCOPE();

	if( NULL == ps_MioCertData ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackManager::freeCertChainList arg Null Error!!\n" );
		return;
	}

	LAPS_NET_CertData* ps_Data = NULL;
	while( ps_MioCertData ) {
		ps_Data = ps_MioCertData->pNext;
		free( ps_MioCertData );
		ps_MioCertData = ps_Data;
	}
}

//--------------------------------------------------------------------------------------------.
/// \brief	機能：ネットワークエラーコードをカウントエラーコードに変換する.
/// \par	備考：.
/// \param	uint32_t ul_errorCode [I] ： ネットワークエラーコード.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void FBPC_FallbackManager::setCountErrorCode( uint32_t ul_errorCode )
{
	FBPD_TRACE_SCOPE();

	uchar uc_ErrSubCode = DEFAULT_COUNT_SUB_ERROR_CODE;
	switch( ul_errorCode ) {
	case NETWORK_ERROR_CODE_CONNECTION_REFUSED:
		uc_ErrSubCode = COUNT_ERROR_CODE_CONNECTION_REFUSED;
		break;
	case NETWORK_ERROR_CODE_REMOTE_HOST_CLOSED:
		uc_ErrSubCode = COUNT_ERROR_CODE_REMOTE_HOST_CLOSED;
		break;
	case NETWORK_ERROR_CODE_HOST_NOT_FOUND:
		uc_ErrSubCode = COUNT_ERROR_CODE_HOST_NOT_FOUND;
		break;
	case NETWORK_ERROR_CODE_TIMEOUT_ERROR:
		uc_ErrSubCode = COUNT_ERROR_CODE_TIMEOUT_ERROR;
		break;
	case NETWORK_ERROR_CODE_OPERATION_CANCELLED:
		uc_ErrSubCode = COUNT_ERROR_CODE_OPERATION_CANCELLED;
		break;
	case NETWORK_ERROR_CODE_SSL_HANDSHAKE_FAILED:
		uc_ErrSubCode = COUNT_ERROR_CODE_SSL_HANDSHAKE_FAILED;
		break;
	case NETWORK_ERROR_CODE_TEMPORARY_NETWORK_FAILURE:
		uc_ErrSubCode = COUNT_ERROR_CODE_TEMPORARY_NETWORK_FAILURE;
		break;
	case NETWORK_ERROR_CODE_PROXY_CONNECTION_REFUSED:
		uc_ErrSubCode = COUNT_ERROR_CODE_ROXY_CONNECTION_REFUSED;
		break;
	case NETWORK_ERROR_CODE_PROXY_CONNECTION_CLOSED :
		uc_ErrSubCode = COUNT_ERROR_CODE_CONNECTION_CLOSED;
		break;
	case NETWORK_ERROR_CODE_PROXY_NOT_FOUND :
		uc_ErrSubCode = COUNT_ERROR_CODE_PROXY_NOT_FOUND;
		break;
	case NETWORK_ERROR_CODE_PROXY_TIMEOUT_ERROR :
		uc_ErrSubCode = COUNT_ERROR_CODE_PROXY_TIMEOUT_ERROR;
		break;
	case NETWORK_ERROR_CODE_PROXY_AUTHENTICATION_REQUIRED :
		uc_ErrSubCode = COUNT_ERROR_CODE_PROXY_AUTHENTICATION_REQUIRED;
		break;
	case NETWORK_ERROR_CODE_CONETNT_ACCESS_DENIED :
		uc_ErrSubCode = COUNT_ERROR_CODE_CONETNT_ACCESS_DENIED;
		break;
	case NETWORK_ERROR_CODE_CONTENT_OPERATION_NOT_PERMITTED :
		uc_ErrSubCode = COUNT_ERROR_CODE_CONTENT_OPERATION_NOT_PERMITTED;
		break;
	case NETWORK_ERROR_CODE_CONTENT_NOT_FOUND:
		uc_ErrSubCode = COUNT_ERROR_CODE_CONTENT_NOT_FOUND;
		break;
	case NETWORK_ERROR_CODE_AUTHENTICATION_REQUIRED	:
		uc_ErrSubCode = COUNT_ERROR_CODE_AUTHENTICATION_REQUIRED;
		break;
	case NETWORK_ERROR_CODE_CONTENT_RESEND	:
		uc_ErrSubCode = COUNT_ERROR_CODE_CONTENT_RESEND;
		break;
	case NETWORK_ERROR_CODE_PROTOCOL_UNKNOWN:
		uc_ErrSubCode = COUNT_ERROR_CODE_PROTOCOL_UNKNOWN;
		break;
	case NETWORK_ERROR_CODE_PROTOCOL_INVALID_OPERATION:
		uc_ErrSubCode = COUNT_ERROR_CODE_PROTOCOL_INVALID_OPERATION;
		break;
	case NETWORK_ERROR_CODE_UNKNOWN_NETWORK	:
		uc_ErrSubCode = COUNT_ERROR_CODE_UNKNOWN_NETWORK;
		break;
	case NETWORK_ERROR_CODE_UNKNOWN_PROXY:
		uc_ErrSubCode = COUNT_ERROR_CODE_UNKNOWN_PROXY;
		break;
	case NETWORK_ERROR_CODE_UNKNOWN_CONTENT	:
		uc_ErrSubCode = COUNT_ERROR_CODE_UNKNOWN_CONTENT;
		break;
	case NETWORK_ERROR_CODE_PROTOCOL_FAILURE:
		uc_ErrSubCode = COUNT_ERROR_CODE_PROTOCOL_FAILURE;
		break;
	case ERROR_CODE_MIO_CERT_VALIDITY:
		uc_ErrSubCode = COUNT_ERROR_CODE_MIO_CERT_VALIDITY;
		break;
	case ERROR_CODE_MIO_CERT_CN:
		uc_ErrSubCode = COUNT_ERROR_CODE_MIO_CERT_CN;
		break;
	case ERROR_CODE_MIO_CERT_PURPOSE:
		uc_ErrSubCode = COUNT_ERROR_CODE_MIO_CERT_PURPOSE;
		break;
	case ERROR_CODE_MIO_CERT_CA	:
		uc_ErrSubCode = COUNT_ERROR_CODE_MIO_CERT_CA;
		break;
	case ERROR_CODE_MIO_CERT_REVOCATED:
		uc_ErrSubCode = COUNT_ERROR_CODE_MIO_CERT_REVOCATEDD;
		break;
	case ERROR_CODE_MIO_CERT_CONNECT_CA:
		uc_ErrSubCode = COUNT_ERROR_CODE_MIO_CERT_CONNECT_CA;
		break;
	case ERROR_CODE_MIO_CERT_REVOCATE_TIMEOUT:
		uc_ErrSubCode = COUNT_ERROR_CODE_MIO_CERT_REVOCATE_TIMEOUT;
		break;
	case ERROR_CODE_MIO_CERT_CRL_TOO_BIG:
		uc_ErrSubCode = COUNT_ERROR_CODE_MIO_CERT_CRL_TOO_BIG;
		break;
	case ERROR_CODE_MIO_CERT_FORMAT	:
		uc_ErrSubCode = COUNT_ERROR_CODE_MIO_CERT_FORMAT;
		break;
	case ERROR_CODE_MIO_CERT_DISABLE:
		uc_ErrSubCode = COUNT_ERROR_CODE_MIO_CERT_DISABLE;
		break;
	case ERROR_CODE_MIO_CERT_BUSY:
		uc_ErrSubCode = COUNT_ERROR_CODE_MIO_CERT_BUSY;
		break;
	case ERROR_CODE_MIO_CERT_INVALID_PARAM	:
		uc_ErrSubCode = COUNT_ERROR_CODE_MIO_CERT_INVALID_PARAM;
		break;
	case ERROR_CODE_MIO_CERT_INTERNAL:
		uc_ErrSubCode = COUNT_ERROR_CODE_MIO_CERT_INTERNAL;
		break;
	case NETWORK_ERROR_CODE_COMM_WEBSERVER_DOWNLOAD:
		uc_ErrSubCode = COUNT_ERROR_CODE_COMM_WEBSERVER_DOWNLOAD;
		break;
	case NETWORK_ERROR_CODE_COMM_WEBSERVER_UPLOAD:
		uc_ErrSubCode = COUNT_ERROR_CODE_COMM_WEBSERVER_UPLOAD;
		break;
	case NETWORK_ERROR_CODE_COMM_ACCESS_ERROR_UPLOAD:
		uc_ErrSubCode = COUNT_ERROR_CODE_COMM_ACCESS_ERROR_UPLOAD;
		break;
	case NETWORK_ERROR_CODE_COMM_TIMEOUT_ERROR_DOWNLOAD:
		uc_ErrSubCode = COUNT_ERROR_CODE_COMM_TIMEOUT_ERROR_DOWNLOAD;
		break;
	default:
		break;
	}
	CNTC_CounterIf::newInstance()->countNetworkError( COUNT_BROWSER_MAIN_CODE, uc_ErrSubCode );
}

//--------------------------------------------------------------------------------------------.
/// \brief	機能：詳細エラーコードを保存する.
/// \par	備考：なし.
/// \param	 uint32_t ul_errorCode [I] ： エラーコード.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void FBPC_FallbackManager::setDetailedErrorCode( uint32_t ul_errorCode, BRWE_ErrorType e_errorType )
{
	FBPD_TRACE_SCOPE();

	if ( e_errorType == BRWE_NetworkErrorCode || e_errorType == BRWE_CertificateErrorCode ) {
		setCountErrorCode( ul_errorCode );
	}
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：証明書関連エラーコードを保存する.
///	\par	備考：.
///	\param	 uint32_t ul_errorCode [I] ： エラーコード.
///	\result	void.
//--------------------------------------------------------------------------------------------.
void FBPC_FallbackManager::certificateErrorCode( uint32_t ul_errorCode )
{
	FBPD_TRACE_SCOPE();

	switch( ul_errorCode ) {
	case MIO_CERT_VALIDITY_ERR:
		setDetailedErrorCode( ERROR_CODE_MIO_CERT_VALIDITY,			BRWE_CertificateErrorCode );
		break;
	case MIO_CERT_CN_ERR:
		setDetailedErrorCode( ERROR_CODE_MIO_CERT_CN,				BRWE_CertificateErrorCode );
		break;
	case MIO_CERT_PURPOSE_ERR:
		setDetailedErrorCode( ERROR_CODE_MIO_CERT_PURPOSE,			BRWE_CertificateErrorCode );
		break;
	case MIO_CERT_CA_ERR:
		setDetailedErrorCode( ERROR_CODE_MIO_CERT_CA,				BRWE_CertificateErrorCode );
		break;
	case MIO_CERT_REVOCATED_ERR:
		setDetailedErrorCode( ERROR_CODE_MIO_CERT_REVOCATED,		BRWE_CertificateErrorCode );
		break;
	case MIO_CERT_CONNECT_CA_ERR:
		setDetailedErrorCode( ERROR_CODE_MIO_CERT_CONNECT_CA,		BRWE_CertificateErrorCode );
		break;
	case MIO_CERT_REVOCATE_TIMEOUT_ERR:
		setDetailedErrorCode( ERROR_CODE_MIO_CERT_REVOCATE_TIMEOUT,	BRWE_CertificateErrorCode );
		break;
	case MIO_CERT_CRL_TOO_BIG_ERR:
		setDetailedErrorCode( ERROR_CODE_MIO_CERT_CRL_TOO_BIG,		BRWE_CertificateErrorCode );
		break;
	case MIO_CERT_FORMAT_ERR:
		setDetailedErrorCode( ERROR_CODE_MIO_CERT_FORMAT,			BRWE_CertificateErrorCode );
		break;
	case MIO_CERT_DISABLE_ERR:
		setDetailedErrorCode( ERROR_CODE_MIO_CERT_DISABLE,			BRWE_CertificateErrorCode );
		break;
	case MIO_CERT_BUSY_ERR:
		setDetailedErrorCode( ERROR_CODE_MIO_CERT_BUSY,				BRWE_CertificateErrorCode );
		break;
	case MIO_CERT_INVALID_PARAM_ERR:
		setDetailedErrorCode( ERROR_CODE_MIO_CERT_INVALID_PARAM,	BRWE_CertificateErrorCode );
		break;
	case MIO_CERT_INTERNAL_ERR:
		setDetailedErrorCode( ERROR_CODE_MIO_CERT_INTERNAL,			BRWE_CertificateErrorCode );
		break;
	default:
		break;
	}
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：サーバー証明書の検証.
///	\par	備考：.
///	\param	uint32_t  ul_CertNum [I] ： 証明書件数.
///	\param	uchar* puc_SslUrl [I] ： SSLサイトのURL.
///	\result	uint32_t  検証結果.
///				FBPE_CERT_OK				:証明書検証OK.
///				FBPE_CERT_VALIDITY_ERR		:証明書期限エラー.
///				FBPE_CERT_CN_ERR			:証明書のCNフィールドが不一致.
///				FBPE_CERT_PURPOSE_ERR		:証明書は期待された用法を持っていない.
///				FBPE_CERT_CA_ERR			:証明書は信頼できません.
///				FBPE_CERT_INTERNALERR		:内部エラー.
//--------------------------------------------------------------------------------------------.
uint32_t FBPC_FallbackManager::certverify( uint32_t ul_CertNum, uchar* puc_SslUrl )
{
	FBPD_TRACE_SCOPE();

	if( NULL == puc_SslUrl ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackManager::certverify arg Null Error!!\n" );
		return FBPE_CERT_INTERNALERR;
	}

	LAPS_NET_CertVerify	s_MioCertVerify = {};		// 証明書検証パラメータ構造体.
	uint32_t			ul_Ret = FBPE_CERT_OK;		// 本関数の戻り値.
	uint32_t			ul_MioRet = 0;				// NICFUM関数の戻り値.
	LAPS_NET_CertData*	ps_MioCertData = NULL;		// 中間証明書データ.

	if ( ( !m_ps_ShmCertData ) || ( ul_CertNum <= 0 ) ) {
		return FBPE_CERT_INTERNALERR;
	}
	if ( ul_CertNum > FBPD_BrwCertChainMax ) {
		ul_CertNum = FBPD_BrwCertChainMax;
	}
	// 証明書検証パラメータ設定.
	s_MioCertVerify.uiSessionID = 0;
	// サーバー証明書設定
	s_MioCertVerify.pCert = m_ps_ShmCertData[0].uc_CertData;
	s_MioCertVerify.uiSize = m_ps_ShmCertData[0].ui32_CertSize;
	// 中間証明書設定(順不同).
	for( uint32_t ul_cnt = 1; ul_cnt < ul_CertNum; ul_cnt++ ) {
		ps_MioCertData = addCertChainList( m_ps_ShmCertData[ul_cnt].uc_CertData, m_ps_ShmCertData[ul_cnt].ui32_CertSize, ps_MioCertData );
	}
	s_MioCertVerify.pstIntermediate = ps_MioCertData;
	s_MioCertVerify.usCertVerify = MIO_CERT_VALIDITY|MIO_CERT_VERIFY_CNW|MIO_CERT_VERIFY_CA_EX;
	memcpy( s_MioCertVerify.cServerAddr, puc_SslUrl, sizeof( s_MioCertVerify.cServerAddr ) );
	s_MioCertVerify.eUsage = LAPE_NET_CERT_USE_FOR_SSL_SERVER;
	s_MioCertVerify.uiUseType = MIO_USE_CERT_NONE;

	// 証明書検証.
	ul_MioRet = LAPF_NET_SSLCertVerifyWithTimeout( &s_MioCertVerify, FB_CERTVERIFYTIMEOUT );
	certificateErrorCode(ul_MioRet);
	if ( ul_MioRet >= MIO_CERT_REVOCATED_ERR ) {
		ul_Ret = FBPE_CERT_INTERNALERR;
	} else {
		ul_Ret = ul_MioRet;
	}

	freeCertChainList( ps_MioCertData );
	return( ul_Ret );
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：ディレクトリパスの取得.
///	\par	備考：.
///	\param	FLE_DirType e_DirType [I] ： ディレクトリ種別.
///	\result	char* ディレクトリパス.
//--------------------------------------------------------------------------------------------.
char*  FBPC_FallbackManager::getRootPath( SFLE_DirType e_DirType )
{
	FBPD_TRACE_SCOPE();

	char sc_RootPath[SFLD_PATH_MAX + 1] = {};	// ディレクトリパス取得関数のパス格納先．
	int32_t si_ret = SFLD_Ng;					// ディレクトリパス取得関数の戻り値．
	char* psc_Path = NULL;						// ディレクトリパスポインタ変数．

	// ディレクトリパスポインタ変数に、ディレクトリ種別毎のディレクトリパス文字列の先頭アドレスを設定.
	switch( e_DirType ) {
	case SFLE_DirTypeBrowser:						// ブラウザ用.
		psc_Path = m_st_DirPath.sc_DirTypeBrowser;
		break;
	case SFLE_DirTypeBrowserCache:					// ブラウザ キャッシュ用.
		psc_Path = m_st_DirPath.sc_DirTypeBrowserCache;
		break;
	case SFLE_DirTypeBrowserCert:					// ブラウザ 証明書用.
		psc_Path = m_st_DirPath.sc_DirTypeBrowserCert;
		break;
	case SFLE_DirTypeBrowserFont:					// ブラウザ フォント用.
		psc_Path = m_st_DirPath.sc_DirTypeBrowserFont;
		break;
	case SFLE_DirTypeBrowserSetting:				// ブラウザ 設定用.
		psc_Path = m_st_DirPath.sc_DirTypeBrowserSetting;
		break;
	case SFLE_DirTypeBrowserPublic:					// ブラウザ パブリック用.
		psc_Path = m_st_DirPath.sc_DirTypeBrowserPublic;
		break;
	case SFLE_DirTypeBrowserPublicTuhm:				// ブラウザ パブリックサムネイル用.
		psc_Path = m_st_DirPath.sc_DirTypeBrowserPublicTuhm;
		break;
	case SFLE_DirTypeBrowserUser:					// ブラウザ ユーザー設定用.
		psc_Path = m_st_DirPath.sc_DirTypeBrowserUser;
		break;
	case SFLE_DirTypeBrowserUserThum:				// ブラウザ ユーザーサムネイル用.
		psc_Path = m_st_DirPath.sc_DirTypeBrowserUserThum;
		break;
	case SFLE_DirTypeBrowserTemp:					// ブラウザ 一時保存用.
		psc_Path = m_st_DirPath.sc_DirTypeBrowserTemp;
		break;
	case SFLE_DirTypeBrowserLocalContents:			// ブラウザ ローカルコンテンツ用.
		psc_Path = m_st_DirPath.sc_DirTypeBrowserLocalContents;
		break;
	case SFLE_DirTypeBrowserTrash:					// ブラウザ用ごみ箱.
		psc_Path = m_st_DirPath.sc_DirTypeBrowserTrash;
		break;
	case SFLE_DirTypeBrowserAccessLog:				// ブラウザ アクセスログ用.
		psc_Path = m_st_DirPath.sc_DirTypeBrowserAccessLog;
		break;
	case SFLE_DirTypeBrowserLanguage:				// ブラウザ 言語用.
		psc_Path = m_st_DirPath.sc_DirTypeBrowserLanguage;
		break;
	case SFLE_DirTypeGeneralDoc:					// ブラウザ GeneralDoc用.
		psc_Path = m_st_DirPath.sc_DirTypeBrowserGeneralDoc;
		break;
	default:										// ディレクトリ種別範囲外.
		// ディレクトリ種別が範囲外の場合.
		FBPG_DebugPrint( 3, "[%s]e_DirType=%d NG.\n", __PRETTY_FUNCTION__, e_DirType);
		psc_Path = m_st_DirPath.sc_DirTypeBrowserError;	// ブラウザ エラー用.
		// ディレクトリ種別が範囲外の場合、ディレクトリパスに空文字を返し処理を終了．
		return ( psc_Path );
	}

	// ディレクトリパスがNULLの場合、ディレクトリパス取得関数を呼び出す.
	if ( '\0' == psc_Path[0] ) {
		// HDDが装着されているか.
		APIC_HddInfo::newInstance()->getHddExist();
		// ディレクトリパス取得関数.
		si_ret = SFLG_getPath( SFLE_HddStateUse, e_DirType, sc_RootPath );

		if ( si_ret == SFLD_Ok ) {
			// 戻り値のディレクトリパスに、ディレクトリパス取得関数で取得したパスを設定.
			strncpy( psc_Path , sc_RootPath, sizeof( sc_RootPath ) );
		} else {
			// ディレクトリパス取得関数がエラー終了の場合、ディレクトリパスは空文字のまま.
			FBPG_DebugPrint( 3, "[%s]SFLG_getPath() returns null.\n", __PRETTY_FUNCTION__);
		}
	}

	return ( psc_Path );
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：クライアント証明書一覧の取得.
///	\par	備考：.
///	\param	なし.
///	\result	Bool 結果.
///				True		:正常.
///				False		:異常.
//--------------------------------------------------------------------------------------------.
Bool FBPC_FallbackManager::createClientCertList()
{
	FBPD_TRACE_SCOPE();

	Bool b_Ret = True;												// 本関数の戻り値.
	int32_t si_MioRet = LAPE_NET_SSL_SUCCESS;						// NICFUM関数の戻り値.
	FILE* p_fp = NULL;												// クライアント証明書一覧ファイルポインタ.
	LAPS_NET_CertList* ps_CertList = NULL;							// 証明書リスト.
	LAPS_NET_SslCertInfo s_MioCertInfo = {};						// 証明書サブジェクト情報.
	uint32_t ul_CertNum = 0;										// 証明書リスト数.
	uint32_t ul_CertCnt = 0;										// 証明書リストカウンタ.
	char* psc_CertData = NULL;										// 証明書データ.
	FBPS_BrowserClientCertInfo s_FBPS_BrowserClientCertInfo = {};	// 証明書データ構造体.
	static uint32_t ui_randSeed =  time( NULL );					// 乱数用シード.

	char sc_CerListFileName[FBPD_PATH_MAX_NF] = {};
	snprintf( sc_CerListFileName, ( sizeof( sc_CerListFileName ) ), "%s%s%s", getRootPath( SFLE_DirTypeBrowserCert ), "/", BRWD_CERTLIST_FILENAME );

	// 証明書リスト取得.
	si_MioRet = LAPF_NET_SSLGetCertList( 0, MIO_CERT_TYPE_DEVICE_BROWSER, ( uint32_t* )&ul_CertNum, ps_CertList );
	if ( ul_CertNum > 0 ) {
		ps_CertList = ( LAPS_NET_CertList* )malloc( ul_CertNum*sizeof( LAPS_NET_CertList ) );
		if ( ps_CertList ) {
			si_MioRet = LAPF_NET_SSLGetCertList( 0, MIO_CERT_TYPE_DEVICE_BROWSER, ( uint32_t* )&ul_CertNum, ps_CertList );
		} else {
			b_Ret = False;
		}
	}
	if ( si_MioRet != LAPE_NET_SSL_SUCCESS ) {
		b_Ret = False;
	}

	if ( b_Ret == True ) {
		psc_CertData = ( char* )malloc( FBPD_BrwCertVerifyMax );
		if ( !psc_CertData ) {
			b_Ret = False;
		}
	}

	if ( b_Ret == True ) {
		p_fp = SYSF_fopen( sc_CerListFileName, "wb" );
		if ( p_fp == NULL ) {
			b_Ret = False;
		} else {
			int32_t ret = chmod( sc_CerListFileName, S_IRUSR | S_IWUSR | S_IROTH | S_IWOTH );
			if( ret == -1 ){
				FBPG_DebugPrint( 3, "[%s:%d] (%s) fchmod is failed.(%d)\n", __PRETTY_FUNCTION__, __LINE__, sc_CerListFileName, errno );
				SYSF_fclose( p_fp );
				if ( psc_CertData ) free( psc_CertData );
				if ( ps_CertList ) free( ps_CertList );
				return False;
			}
			// クライアント証明書一覧ファイル作成.
			fwrite( &ul_CertNum, 1, sizeof( ul_CertNum ), p_fp );
			for( ul_CertCnt = 0; ul_CertCnt < ul_CertNum; ul_CertCnt++ ) {
				memset( &s_FBPS_BrowserClientCertInfo, 0, sizeof( s_FBPS_BrowserClientCertInfo ) );
				s_FBPS_BrowserClientCertInfo.si_CertSize = FBPD_BrwCertMax;
				snprintf( s_FBPS_BrowserClientCertInfo.sc_PassWord, sizeof( s_FBPS_BrowserClientCertInfo.sc_PassWord ),
				          "%04d_%04d_%04d",rand_r( &ui_randSeed )%10000,rand_r( &ui_randSeed )%10000,rand_r( &ui_randSeed )%10000 );
				si_MioRet = LAPF_NET_SSLGetPKCS12( ps_CertList[ul_CertCnt].uiCertID,
				                                   reinterpret_cast<const char*>( s_FBPS_BrowserClientCertInfo.sc_PassWord ),
				                                   &( s_FBPS_BrowserClientCertInfo.si_CertSize ), psc_CertData );

				s_FBPS_BrowserClientCertInfo.ul_CertID = ps_CertList[ul_CertCnt].uiCertID;
				memset( &s_MioCertInfo, 0x00, sizeof( s_MioCertInfo ) );
				si_MioRet = LAPF_NET_SSLGetCertInfo( &s_MioCertInfo, ps_CertList[ul_CertCnt].uiCertID );
				if ( si_MioRet == LAPE_NET_SSL_SUCCESS ) {
					memcpy( s_FBPS_BrowserClientCertInfo.sc_CommonName, s_MioCertInfo.szCN,
					        sizeof( s_MioCertInfo.szCN ) );
					fwrite( &( s_FBPS_BrowserClientCertInfo ), 1, sizeof( s_FBPS_BrowserClientCertInfo ), p_fp );
					fwrite( psc_CertData, 1, s_FBPS_BrowserClientCertInfo.si_CertSize, p_fp );
				} else {
					b_Ret = False;
				}
			}
			SYSF_fclose( p_fp );
		}
	}

	if ( psc_CertData ) {
		free( psc_CertData );
	}
	if ( ps_CertList ) {
		free( ps_CertList );
	}
	return( b_Ret );
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：クライアント証明書の取得.
///	\par	備考：.
///	\param	uint32_t  ul_CertID    [I] ： 取得する証明書のID.
///	\param	uchar* puc_Cert        [O] ： クライアント証明書格納領域.
///	\param	char*  psc_PassWord    [I] ： 復号化パスワード.
///	\result	int32_t クライアント証明書のサイズ.
//--------------------------------------------------------------------------------------------.
int32_t FBPC_FallbackManager::getClientCert( uint32_t ul_CertID, uchar* puc_Cert, char* psc_PassWord )
{
	FBPD_TRACE_SCOPE();

	if( ( NULL == puc_Cert ) || ( NULL == psc_PassWord ) ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackManager::getClientCert arg Null Error!!\n" );
		return 0;
	}
	
	int32_t si_CertDataSize = FBPD_BrwCertMax;	// 証明書データサイズ.
	int32_t	si_MioRet = LAPE_NET_SSL_SUCCESS;	// NICFUM関数の戻り値.

	si_MioRet = LAPF_NET_SSLGetPKCS12( ul_CertID, reinterpret_cast<const char*>( psc_PassWord ), &si_CertDataSize, ( char* )puc_Cert );
	if ( si_MioRet != LAPE_NET_SSL_SUCCESS ) {
		si_CertDataSize = 0;
	}
	return( si_CertDataSize );
}

//------------------------------------------------------------------------------.
///	\brief	機能：スクリーンセーバー表示中のパネル座標キー受付通知(ここではオートリセットタイマのため).
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::TouchPanelKeyNoticeScrSvr()
{
	FBPD_TRACE_SCOPE();

	BRWC_BrowserMsg st_Msg = {};
	st_Msg.ss_type = BR_Msg_PanelKeyRequest;

	SendMsgFallbackManager( &st_Msg );
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：キー押下時のタイマーリセット.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void FBPC_FallbackManager::procSomethingKeyOn()
{
	FBPD_TRACE_SCOPE();

	BRWC_BrowserMsg st_Msg = {};
	st_Msg.ss_type = BR_Msg_procSomethingKeyOn;

	SendMsgFallbackManager( &st_Msg );
}

//------------------------------------------------------------------------------.
///	\brief	機能：JavaScript疑似キー押下.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::JavaScriptSomethingKeyOn()
{
	FBPD_TRACE_SCOPE();

	procSomethingKeyOn();
}

//------------------------------------------------------------------------------.
///	\brief	機能：Beep音を鳴らす.
///	\par	備考：.
///	\param	uchar uc_soundtype [I] ： 音声タイプ.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::BeepPlayReq( uchar uc_soundtype )
{
	FBPD_TRACE_SCOPE();

	FBPC_FallbackIpcForMfpWatch::newInstance()->BeepPlayReq( uc_soundtype );
}

//------------------------------------------------------------------------------.
///	\brief	機能：USB外付けキーボード利用可否.
///	\par	備考：.
///	\param	なし.
///	\result	Bool 結果.
///				True		:可.
///				False		:否.
//------------------------------------------------------------------------------.
Bool FBPC_FallbackManager::isUSBKeyBoardConnect()
{
	FBPD_TRACE_SCOPE();

	if ( TYPE_UsbKbdStateInfo_One == APIC_MachineSetting::newInstance()->getUsbStatus() ) {
		return True;
	}
	return False;
}

//------------------------------------------------------------------------------.
///	\brief	機能：エラーコード通知.
///	\par	備考：.
///	\param	uint32_t       ul_errorCode [I] ： エラーコード（ブラウザ内部）.
///	\param	BRWE_ErrorType e_errorType  [I] ： エラーコード種別.
///												BRWE_BrowserErrorCode				:
///												BRWE_NetworkErrorCode				:
///												BRWE_CertificateErrorCode			:
///												BRWE_WebKitBrowserCrashErrorCode	:
///												BRWE_ChromiumBrowserCrashErrorCode	:
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::BrowserErrorCode( uint32_t ul_errorCode, BRWE_ErrorType e_errorType )
{
	FBPD_TRACE_SCOPE();

	setDetailedErrorCode( ul_errorCode, e_errorType );
}

//------------------------------------------------------------------------------.
///	\brief	機能：証明書タイプを変換する(ブラウザ -> NUCFUM).
///	\par	備考：.
///	\param	TYPE_BrwCertType e_CertType [I] ： 証明書タイプ.
///	\result	uint32_t MIO_CERT_TYPE_XXXXX.
//------------------------------------------------------------------------------.
uint32_t FBPC_FallbackManager::convCertType( TYPE_BrwCertType e_CertType )
{
	FBPD_TRACE_SCOPE();

	uint32_t ul_CertType = 0;	// 証明書種別(NICFUM用).

	if ( e_CertType == TYPE_BrwCertTypeServerRoot ) {
		ul_CertType = MIO_CERT_TYPE_ROOT;
	} else if ( e_CertType == TYPE_BrwCertTypeServerCA ) {
		ul_CertType = MIO_CERT_TYPE_INTERMEDIATE;
	} else if ( e_CertType == TYPE_BrwCertTypeServerTrust ) {
		ul_CertType = MIO_CERT_TYPE_ENTITY;
	} else if ( e_CertType == TYPE_BrwCertTypeServerUnTrust ) {
		ul_CertType = MIO_CERT_TYPE_UNTRUSTED;
	} else if ( e_CertType == TYPE_BrwCertTypeClient ) {
		ul_CertType = MIO_CERT_TYPE_DEVICE_BROWSER;
	}

	return( ul_CertType );
}

//------------------------------------------------------------------------------.
///	\brief	機能：指定種別の証明書リストを取得する.
///	\par	備考：.
///	\param	TYPE_BrwCertType      e_CertType  [I] ： 証明書種別.
///	\param	TYPS_BrowserCertList* ps_CertList [O] ： 証明書リスト.
///	\result	Bool 結果.
///				True		:処理正常.
///				False		:処理異常.
//------------------------------------------------------------------------------.
Bool FBPC_FallbackManager::getCertList( TYPE_BrwCertType e_CertType, TYPS_BrowserCertList* ps_CertList )
{
	FBPD_TRACE_SCOPE();

	LAPS_NET_CertList* ps_MioCertList = NULL;			// 証明書リスト.
	LAPS_NET_SslCertInfo s_MioCertInfo = {};			// 証明書発行先情報.
	LAPS_NET_SslCertInfo s_IssureCertInfo = {};			// 証明書発行元情報.
	TYPS_BrowserCertInfo s_CertInfo = {};
	uint32_t ui_CertListNum = 0;						// 証明書リストに含まれる証明書数.
	uint32_t ul_CertType = 0;							// 証明書タイプ.
	int32_t si_MioRet = LAPE_NET_SSL_SUCCESS;			// NICFUM関数の戻り値.
	Bool b_ret = True;									// 本関数の戻り値.
	uint32_t ui_CertCnt = 0;
	FILE* p_fpPreInstallCert = NULL;
	char sc_CharRead[FBPD_MAXCHARID] = {};
	std::list<int32_t> si_PreInstallCertlist;
	int32_t si_CertID = 0;
	Bool b_Is_PreInstallFileExists = False;

	if ( ps_CertList == NULL ) {
		return( False );
	}

	char sc_PreInstallCertFileName[FBPD_PATH_MAX_NF] = {};
	snprintf( sc_PreInstallCertFileName, (sizeof(sc_PreInstallCertFileName)), "%s%s%s", getRootPath(SFLE_DirTypeBrowserCert), "/", DEF_PreInstallCert_Repostry );
	p_fpPreInstallCert = SYSF_fopen( sc_PreInstallCertFileName, "rb" );
	if ( p_fpPreInstallCert == NULL ) {
		b_Is_PreInstallFileExists = False;
	} else 	{
		b_Is_PreInstallFileExists = True;
		while ( fgets(sc_CharRead, FBPD_MAXCHARID, p_fpPreInstallCert) ) {
			if ( *sc_CharRead != '\n' ) {
				si_CertID = atoi( sc_CharRead );
				si_PreInstallCertlist.push_back( si_CertID );
			}
		}
	}

	ps_CertList->e_CertType = e_CertType;
	ps_CertList->us_ListNum = 0;
	if ( !ps_CertList->s_CertList.empty() ) {
		ps_CertList->s_CertList.clear();
	}

	// 該当する証明書の件数を取得.
	ul_CertType = convCertType( e_CertType );
	si_MioRet = LAPF_NET_SSLGetCertList( 0, ul_CertType, &ui_CertListNum, ps_MioCertList );
	if ( si_MioRet != LAPE_NET_SSL_SUCCESS ) {
		b_ret = False;
	}

	if ( ui_CertListNum > 0 && b_ret == True ) {
		ps_MioCertList = ( LAPS_NET_CertList* )malloc( sizeof( LAPS_NET_CertList )*ui_CertListNum );
		if ( ps_MioCertList != NULL ) {
			// 該当する証明書の一覧を取得する.
			si_MioRet = LAPF_NET_SSLGetCertList( 0, ul_CertType, &ui_CertListNum, ps_MioCertList );
			if ( si_MioRet == LAPE_NET_SSL_SUCCESS ) {
				ps_CertList->us_ListNum = (ushort)ui_CertListNum;
				for( ui_CertCnt = 0; ui_CertCnt < ui_CertListNum; ui_CertCnt++ ) {
					memset( &s_CertInfo, 0x00, sizeof( s_CertInfo ) );
					// 発行先情報取得.
					si_MioRet = LAPF_NET_SSLGetCertInfo( &s_MioCertInfo, ps_MioCertList[ui_CertCnt].uiCertID );
					if ( si_MioRet == LAPE_NET_SSL_SUCCESS ) {
						// 発行元情報取得.
						si_MioRet = LAPF_NET_SSLGetCertIssuerInfo( &s_IssureCertInfo, ps_MioCertList[ui_CertCnt].uiCertID );
					}
					if ( si_MioRet == LAPE_NET_SSL_SUCCESS ) {
						// 証明書ID.
						s_CertInfo.ul_CertID = ps_MioCertList[ui_CertCnt].uiCertID;
						// 有効/無効(常に有効).
						s_CertInfo.e_CertEnable = TYPE_BrowserCertEnableOn;
						// 発行先.
						// 一般名.
						strncpy( s_CertInfo.s_SubjectDN.sc_CommonName, ( char* )s_MioCertInfo.szCN,
						         sizeof(s_CertInfo.s_SubjectDN.sc_CommonName) );			//[11/06/18][Static Analysis][NULL][288889] add.
						// 部門名.
						strncpy( s_CertInfo.s_SubjectDN.sc_OrganizationalUnit, ( char* )s_MioCertInfo.szOU,
						         sizeof(s_CertInfo.s_SubjectDN.sc_OrganizationalUnit) );	//[11/06/18][Static Analysis][NULL][288888] add.
						// 組織名.
						strncpy( s_CertInfo.s_SubjectDN.sc_Organization, ( char* )s_MioCertInfo.szO,
						         sizeof(s_CertInfo.s_SubjectDN.sc_Organization) );			//[11/06/18][Static Analysis][NULL][288907] add.
						// 国名.
						strncpy( s_CertInfo.s_SubjectDN.sc_Country,( char* )s_MioCertInfo.szC,
						         sizeof(s_CertInfo.s_SubjectDN.sc_Country) );			//[11/06/18][Static Analysis][NULL][288915] add.
						// 発行元.
						// 一般名.
						strncpy( s_CertInfo.s_IssureDN.sc_CommonName, ( char* )s_IssureCertInfo.szCN,
						          sizeof(s_CertInfo.s_IssureDN.sc_CommonName) );		//[11/06/18][Static Analysis][NULL][288905] add.
						// 部門名.
						strncpy( s_CertInfo.s_IssureDN.sc_OrganizationalUnit, ( char* )s_IssureCertInfo.szOU,
						         sizeof(s_CertInfo.s_IssureDN.sc_OrganizationalUnit) );	//[11/06/18][Static Analysis][NULL][288903] add.
						// 組織名.
						strncpy( s_CertInfo.s_IssureDN.sc_Organization, ( char* )s_IssureCertInfo.szO,
						         sizeof(s_CertInfo.s_IssureDN.sc_Organization) );		//[11/06/18][Static Analysis][NULL][288891] add.
						// 国名.
						strncpy( s_CertInfo.s_IssureDN.sc_Country, ( char* )s_IssureCertInfo.szC,
						         sizeof(s_CertInfo.s_IssureDN.sc_Country) );			//[11/06/18][Static Analysis][NULL][288885]add.
						// シリアル番号.
						strncpy( s_CertInfo.sc_SerialNumber, ( char* )s_MioCertInfo.szSerial,
						          sizeof(s_CertInfo.sc_SerialNumber) );					//[11/06/18][Static Analysis][NULL][288900] add.
						// 有効期限.
						s_CertInfo.s_Validity.ss_Year = ( short )( s_MioCertInfo.sValidEnd.tm_year + FBPD_TMYEAR_OFFSET );
						s_CertInfo.s_Validity.sc_Month = ( char )( s_MioCertInfo.sValidEnd.tm_mon + FBPD_TMMON_OFFSET );
						s_CertInfo.s_Validity.sc_Day = ( char )s_MioCertInfo.sValidEnd.tm_mday;
						s_CertInfo.s_Validity.sc_Hour = ( char )s_MioCertInfo.sValidEnd.tm_hour;
						s_CertInfo.s_Validity.sc_Minute = ( char )s_MioCertInfo.sValidEnd.tm_min;
						s_CertInfo.s_Validity.sc_Second = ( char )s_MioCertInfo.sValidEnd.tm_sec;

						//Save validity period start date.
						s_CertInfo.s_Validity_StartDate.ss_Year = (short)(s_MioCertInfo.sValidStart.tm_year + FBPD_TMYEAR_OFFSET);
						s_CertInfo.s_Validity_StartDate.sc_Month = (char)(s_MioCertInfo.sValidStart.tm_mon + FBPD_TMMON_OFFSET);
						s_CertInfo.s_Validity_StartDate.sc_Day = (char)s_MioCertInfo.sValidStart.tm_mday;
						s_CertInfo.s_Validity_StartDate.sc_Hour = (char)s_MioCertInfo.sValidStart.tm_hour;
						s_CertInfo.s_Validity_StartDate.sc_Minute = (char)s_MioCertInfo.sValidStart.tm_min;
						s_CertInfo.s_Validity_StartDate.sc_Second = (char)s_MioCertInfo.sValidStart.tm_sec;
						s_CertInfo.sl_SigTypeNID = s_MioCertInfo.nSigTypeNID;
						if (b_Is_PreInstallFileExists == True){
							if (std::find(si_PreInstallCertlist.begin(), si_PreInstallCertlist.end(), s_CertInfo.ul_CertID) != si_PreInstallCertlist.end()){
								s_CertInfo.b_PreInstallCert=True;
							} else {
								s_CertInfo.b_PreInstallCert=False;
							}
						} else {
							s_CertInfo.b_PreInstallCert=False;
						}
						// 証明書リストに格納.
						ps_CertList->s_CertList.push_back( s_CertInfo );
					} else {
						while( !ps_CertList->s_CertList.empty() ) {
							ps_CertList->s_CertList.pop_front();
						}
						ps_CertList->us_ListNum = 0;
						b_ret = False;
						break;
					}
				}
			} else {
				b_ret = False;
			}
		} else {
			b_ret = False;
		}
	}

	if ( ps_MioCertList != NULL ) {
		free( ps_MioCertList );
	}
	if ( p_fpPreInstallCert ){
		SYSF_fclose(p_fpPreInstallCert);
	}

	return( b_ret );
}

//------------------------------------------------------------------------------.
///	\brief	機能：証明書のCNチェック.
///	\par	備考：.
///	\param	char* sc_CertCommonName [I] 証明書のCN(CommonName).
///	\result	Bool 結果.
///				True		:正常.
///				False		:異常.
//------------------------------------------------------------------------------.
Bool FBPC_FallbackManager::checkForTrustedCert( char* sc_CertCommonName )
{
	FBPD_TRACE_SCOPE();

	TYPS_BrowserCertList	s_CertList = {};
	TYPS_BrowserCertInfo 	s_CertInfo = {};
	Bool b_ret = False;

	if ( False == getCertList( TYPE_BrwCertTypeServerTrust, &s_CertList) ) {
		return b_ret;
	}

	while( !s_CertList.s_CertList.empty() ) {
		s_CertInfo = s_CertList.s_CertList.front();
		s_CertList.s_CertList.pop_front();
		if ( strcmp( s_CertInfo.s_SubjectDN.sc_CommonName , sc_CertCommonName ) == 0 ) {
			b_ret = True;
			break;
		}
	}

	return b_ret;
}
/*===============================================================================================================*/


//------------------------------------------------------------------------------.
///	\brief	機能：コンストラクタ.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//------------------------------------------------------------------------------.
FBPC_FallbackManager::FBPC_FallbackManager()
	: m_b_Init(False)
	 ,m_msg_Q_ID(-1)
	 ,m_uc_Rebooting(0)
	 ,m_uc_AppChgNotice(0)
{
	ZEROCLEAR(m_st_DirPath);
	ZEROCLEAR(m_s_InitialSettingInfo);
}

//------------------------------------------------------------------------------.
///	\brief	機能：Finalize要求.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::FinalizeRequest()
{
	FBPD_TRACE_SCOPE();

	static BRWC_BrowserMsg st_Msg = {};

	/*--------------------------------------------------*/
	/*	メッセージ種別セット							*/
	/*--------------------------------------------------*/
	st_Msg.ss_type = BR_Msg_BrowserFinalizeRequest;

// [次世代パネルv2：マルチウィンドウ対応] Start.
	st_Msg.s_Param.u_FinalizeRequestParam1.ui32_Param = BRWD_FINALIZE_REQUEST_CLOSE_TAB;				// 次世代ブラウザ【OP_BTS-2756対応】.
// 次世代パネル 【AR課題1065737】start.
	st_Msg.s_Param.u_FinalizeRequestParam2.ui32_Param = BRWD_FINALIZE_REQUEST_DELETE_TRANS_HISTORY;		// 次世代ブラウザ【OP_BTS-2756対応】.
// 次世代パネル 【AR課題1065737】end.
// [次世代パネルv2：マルチウィンドウ対応] End.

	/*--------------------------------------------------*/
	/*	ブラウザ設定ファイルパスの作成					*/
	/*--------------------------------------------------*/
	// まずはpublicユーザー用作成＆セット.
	char sc_PathPublic[BRWD_INITIALUSERDIRECTORY] = {};
	snprintf( sc_PathPublic, (sizeof(sc_PathPublic)), "%s%s", getRootPath(SFLE_DirTypeBrowserPublic), "/" );

	uchar	uc_UserDirectory[BRWD_INITIALUSERDIRECTORY] = {};
	memcpy( uc_UserDirectory, ( uchar* )sc_PathPublic, sizeof(sc_PathPublic) );

	strncpy( (char*)st_Msg.s_OptionParam.s_BrowInitialSettingInfo.uc_UserDirectory, (char*)uc_UserDirectory, sizeof(st_Msg.s_OptionParam.s_BrowInitialSettingInfo.uc_UserDirectory) - 1 );

	/*--------------------------------------------------*/
	/*	ブラウザエンジンへメッセージ送信.				*/
	/*--------------------------------------------------*/
	FBPC_FallbackIpcForBrw::newInstance()->SendMsgBrwEngine( &st_Msg );
}

//------------------------------------------------------------------------------.
///	\brief	機能：ブラウザ再起動要求イベントハンドラ.
///	\par	備考：.
///	\param  BRWC_BrowserMsg* pst_BrowserMsg [I] ： 要求パラメータ.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::evt_BrowserReboot( BRWC_BrowserMsg* pst_BrowserMsg )
{
	FBPD_TRACE_SCOPE();

	if( NULL == pst_BrowserMsg ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackManager::evt_BrowserReboot arg Null Error!!\n" );
		return;
	}

	// 再起動中の数を増やす.
	m_uc_Rebooting++;

	//RebootInfoをKeepDataへ退避.
	BRWS_RebootSettingInfo st_RebootSettingInfo = {};

	memcpy( &st_RebootSettingInfo, &(pst_BrowserMsg->s_Param.s_ExtParam), sizeof(st_RebootSettingInfo) );
	FBPC_FallbackKeepData::newInstance()->setReqRebootInfo( &st_RebootSettingInfo );

	FinalizeRequest();
}

//------------------------------------------------------------------------------.
///	\brief	機能：Screen情報の取得.
///	\par	備考：.
///	\param	なし.
///	\result	BRWS_ScreenInfo Screen情報.
//------------------------------------------------------------------------------.
BRWS_ScreenInfo FBPC_FallbackManager::getScreenInfo()
{
	FBPD_TRACE_SCOPE();

	BRWS_ScreenInfo st_ScreenInfo = {};

	TYPS_PanelHardResolution st_PanelResolution = APIC_SystemInfo::newInstance()->getPanelHardResolution();

	// 拡張領域有効/無効の設定.
	BRWE_EXT_AREA extAreaExistFlag = BRWE_EXT_AREA_DISABLE;

	st_ScreenInfo.us_DisplayWidth = st_PanelResolution.us_ResolutionX;
	st_ScreenInfo.us_DisplayHeight = st_PanelResolution.us_ResolutionY;
	st_ScreenInfo.f_PanelSize = st_PanelResolution.sf_PanelSize;
	st_ScreenInfo.us_ResolutionX = st_PanelResolution.us_ResolutionX;
	st_ScreenInfo.us_ResolutionY = st_PanelResolution.us_ResolutionY;
	st_ScreenInfo.e_ExtArea = extAreaExistFlag;
	return st_ScreenInfo;
}

//------------------------------------------------------------------------------.
///	\brief	機能：応答通知メッセージ送信関数.
///	\par	備考：.
///	\param	BRWS_IpcMsgCallBack* st_callInfo [I] ： 応答通知情報.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::SendResponceMsg( const BRWS_IpcMsgCallBack* st_callInfo )
{
	FBPD_TRACE_SCOPE();

	//メッセージキュー作成 (関数初回コール時のみ）.
	if ( m_msg_Snd_ID == -1 ) {
		m_msg_Snd_ID = FBPG_mqOpen( (char*)TEXT(FBPD_IPC_MSGQ_NAME_FALLBACK_FBP_CALLBACK), 100, sizeof( BRWS_IpcMsgCallBack ), True );
		if ( m_msg_Snd_ID == FBPD_ERROR ) {
			FBPG_DebugPrint( 3, "FBPC_FallbackManager::SendResponceMsg FBPG_mqOpen Error!!\n" );
			return;
		}
	}

	if ( NULL == st_callInfo ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackManager::SendResponceMsg arg Null Error!!\n" );
		return;
	}
	//メッセージ送信.
	FBPG_mqSend( m_msg_Snd_ID, ( char* )st_callInfo, sizeof( BRWS_IpcMsgCallBack ), MSG_PRI_NORMAL );
}

//------------------------------------------------------------------------------.
///	\brief	機能：ファイルリンク応答通知.
///	\par	備考：.
///	\param  uchar uc_Resp [I] ： 処理結果 .
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::FileLinkStartResponce( uchar uc_Resp )
{
	FBPD_TRACE_SCOPE();

	BRWS_IpcMsgCallBack  msg = {};

	msg.ss_type = BR_Msg_FileLinkStartResp;		//メッセージ種別.
	msg.u_arg1.uc_arg = uc_Resp;				//処理結果.

	FBPC_FallbackManager::SendResponceMsg( &msg );
}

//------------------------------------------------------------------------------.
///	\brief	機能：PDFファイルリンク要求イベントハンドラ.
///	\par	備考：.
///	\param  BRWC_BrowserMsg* pst_BrowserMsg [I] ： 要求パラメータ.
///	\result	なし.
//------------------------------------------------------------------------------.
void  FBPC_FallbackManager::evt_FileLinkCallBackFunc( BRWC_BrowserMsg*  pst_BrowserMsg )
{
	FBPD_TRACE_SCOPE();

	if( NULL == pst_BrowserMsg ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackManager::evt_FileLinkCallBackFunc arg Null Error!!\n" );
		return;
	}

	FBPC_FallbackManager::FileLinkStartResponce( pst_BrowserMsg->s_InParam.uc_UcParam[0] );
}

//------------------------------------------------------------------------------.
///	\brief	機能：アプリ切替要求(表示)イベントハンドラ.
///	\par	備考：.
///	\param  なし.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::evt_AppChgNoticeStart()
{
	FBPD_TRACE_SCOPE();

	// 再起動中はフラグを保持するのみ.
	if( 0 != m_uc_Rebooting ){
		m_uc_AppChgNotice = FBPD_APP_CHG_NOTICE_START;
		FBPG_DebugPrint( 3, "FBPC_FallbackManager::evt_AppChgNoticeStart not send message!!\n" );
	} else {
		// メッセージ送信.
		BRWC_BrowserMsg st_Msg = {};
		st_Msg.ss_type = BR_Msg_DrawStartRequest;

		// 画面情報を更新する.
		st_Msg.s_OptionParam.s_BrowExInitialSettingInfo.s_InitialSettingInfo.s_ScreenInfo = getScreenInfo();

		FBPC_FallbackIpcForBrw::newInstance()->SendMsgBrwEngine(&st_Msg);

		m_uc_AppChgNotice = FBPD_APP_CHG_NOTICE_NONE;
	}
}

//------------------------------------------------------------------------------.
///	\brief	機能：アプリ切替要求(非表示)イベントハンドラ.
///	\par	備考：.
///	\param  なし.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::evt_AppChgNoticeEnd()
{
	FBPD_TRACE_SCOPE();

	// 再起動中はフラグを保持するのみ.
	if( 0 != m_uc_Rebooting ){
		m_uc_AppChgNotice = FBPD_APP_CHG_NOTICE_END;
		FBPG_DebugPrint( 3, "FBPC_FallbackManager::evt_AppChgNoticeEnd not send message!!\n" );
	} else {
		// メッセージ送信.
		BRWC_BrowserMsg st_Msg = {};
		st_Msg.ss_type = BR_Msg_DrawStopRequest;

		// 画面情報を更新する.
		st_Msg.s_OptionParam.s_BrowExInitialSettingInfo.s_InitialSettingInfo.s_ScreenInfo = getScreenInfo();

		FBPC_FallbackIpcForBrw::newInstance()->SendMsgBrwEngine(&st_Msg);

		m_uc_AppChgNotice = FBPD_APP_CHG_NOTICE_NONE;
	}

}

//------------------------------------------------------------------------------.
///	\brief	機能：通常ブラウザ起動回数情報設定.
///	\par	備考：.
///	\param  FBPD_START_INFO e_StartInfo [I] ： ブラウザ起動回数.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::setStartFrequency( FBPD_START_INFO e_StartInfo )
{
	FBPD_TRACE_SCOPE();

	FBPC_FallbackKeepData::newInstance()->setStartFrequency(e_StartInfo);
}

//------------------------------------------------------------------------------.
///	\brief	機能：スピーカー接続情報.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::SpkrNoticeToEngine()
{
	FBPD_TRACE_SCOPE();

	BRWE_SpkrInfo		e_SpkrInfo = BRWE_SpkrNonConnect;
	// スピーカー接続チェックはエンジンで独自で行っており、.
	// 本関数で設定するe_SpkrInfoの情報は参照していないので固定値を設定しておく.
	
	// メッセージ送信.
	BRWC_BrowserMsg		st_Msg = {};				// メッセージ.
	st_Msg.ss_type = BR_Msg_SpkrNotice;

	// スピーカーアイコン通知.
	st_Msg.s_OptionParam.s_BrowSpkrIconInfo.e_SpkrInfo = e_SpkrInfo;
	st_Msg.s_OptionParam.s_BrowSpkrIconInfo.bl_Mute = False;
	st_Msg.s_OptionParam.s_BrowSpkrIconInfo.uc_vol = FBPC_FallbackKeepData::newInstance()->getBrowserVolume();

	FBPC_FallbackIpcForBrw::newInstance()->SendMsgBrwEngine(&st_Msg);

}

//------------------------------------------------------------------------------.
///	\brief	機能：イニシャライズ完了通知処理.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::exeCmn_BrowserInitializeNotice()
{
	FBPD_TRACE_SCOPE();

	// 再起動中の数を減らす.
	if( 0 != m_uc_Rebooting ) {
		m_uc_Rebooting--;
	}

	// 再起動中にアプリ切替要求があれば実行する.
	if( FBPD_APP_CHG_NOTICE_START == m_uc_AppChgNotice ) {
		evt_AppChgNoticeStart();
	} else if( FBPD_APP_CHG_NOTICE_END == m_uc_AppChgNotice ) {
		evt_AppChgNoticeEnd();
	} else {
		// 何もしない.
	}

	//エンジンへスピーカー接続情報通知.
	SpkrNoticeToEngine();

	//サウンド開始要求.
	BRWC_BrowserMsg st_Msg = {};
	st_Msg.ss_type = BR_Msg_SoundControlStartNotice;
	SendMsgFallbackManager(&st_Msg);
}

//------------------------------------------------------------------------------.
///	\brief	機能：ブラウザイニシャライズ完了通知イベントハンドラ.
///	\par	備考：.
///	\param  なし.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::evt_BrowserInitializeNotice()
{
	FBPD_TRACE_SCOPE();

	setStartFrequency(FBPD_START_FIRST);

	exeCmn_BrowserInitializeNotice();
}

//------------------------------------------------------------------------------.
///	\brief	機能：エンジンに返す言語データへの変換処理.
///	\par	備考：.
///	\param	TYPE_Language e_Language [I] ： MFPの設定言語.
///	\result	uchar 言語タイプ.
//------------------------------------------------------------------------------.
uchar FBPC_FallbackManager::changeLangugeDataToEngine( TYPE_Language e_Language )
{
	FBPD_TRACE_SCOPE();

	uchar uc_LangData = BRWE_LANGUAGE_ENGLISH;		// デフォルトは英語としておく.

	struct LangugeData {
		TYPE_Language	e_Language;		// MFPの言語データ.
		uchar			uc_LangData;	// エンジンへ通知する言語データ.
	} st_LangData[] = {
		{ TYPE_LanguageJapanese 		, BRWE_LANGUAGE_JAPANESE			},	// 日本語.
		{ TYPE_LanguageAmerican 		, BRWE_LANGUAGE_ENGLISH				},	// 英語.
		{ TYPE_LanguageGerman 			, BRWE_LANGUAGE_GERMAN				},	// ドイツ語.
		{ TYPE_LanguageFrench 			, BRWE_LANGUAGE_FRENCH				},	// フランス語.
		{ TYPE_LanguageItalian 			, BRWE_LANGUAGE_ITALIAN				},	// イタリア語.
		{ TYPE_LanguageDanish 			, BRWE_LANGUAGE_DANISH				},	// デンマーク語.
		{ TYPE_LanguageDutch 			, BRWE_LANGUAGE_DUTCH				},	// オランダ語.
		{ TYPE_LanguagePortuguese 		, BRWE_LANGUAGE_PORTUGUESE			},	// ポルトガル語.
		{ TYPE_LanguageSwedish 			, BRWE_LANGUAGE_SWEDISH				},	// スウェーデン語.
		{ TYPE_LanguageSpanish 			, BRWE_LANGUAGE_SPANISH				},	// スペイン語.
		{ TYPE_LanguageNorwegian 		, BRWE_LANGUAGE_NORWEGIAN			},	// ノルウェー語.
		{ TYPE_LanguageFinnish 			, BRWE_LANGUAGE_FINNISH				},	// フィンランド語.
		{ TYPE_LanguageCzech 			, BRWE_LANGUAGE_CZECH				},	// チェコ語.
		{ TYPE_LanguageHangarian 		, BRWE_LANGUAGE_HUNGARIAN			},	// ハンガリー語.
		{ TYPE_LanguageGreek 			, BRWE_LANGUAGE_GREEK				},	// ギリシア語.
		{ TYPE_LanguageTurkish 			, BRWE_LANGUAGE_TURKISH				},	// トルコ語.
		{ TYPE_LanguageChinese1 		, BRWE_LANGUAGE_CHINESE_TRADITIONAL	},	// 広東語 繁体字.
		{ TYPE_LanguageChinese2 		, BRWE_LANGUAGE_CHINESE_SIMPLIFIED	},	// 中国語 簡体字.
		{ TYPE_LanguageHangul 			, BRWE_LANGUAGE_KOREAN				},	// 韓国語.
		{ TYPE_LanguageRussian 			, BRWE_LANGUAGE_RUSSIAN				},	// ロシア語.
		{ TYPE_LanguageCroatian 		, BRWE_LANGUAGE_CROATIAN			},	// クロアチア語.
		{ TYPE_LanguageUkrainian 		, BRWE_LANGUAGE_UKRAINIAN			},	// ウクライナ語.
		{ TYPE_LanguageRomanian 		, BRWE_LANGUAGE_ROMANIAN			},	// ルーマニア語.
		{ TYPE_LanguageSlovak 			, BRWE_LANGUAGE_SLOVAK				},	// スロバキア語(スロベキア語).
		{ TYPE_LanguagePolish 			, BRWE_LANGUAGE_POLISH				},	// ポーランド語.
		{ TYPE_LanguageCatalan 			, BRWE_LANGUAGE_CATALAN				},	// カタロニア語.
		{ TYPE_LanguageSlovene 			, BRWE_LANGUAGE_SLOVENIAN			},	// スロベニア語.
		{ TYPE_LanguageBulgarian		, BRWE_LANGUAGE_BULGARIAN			},	// ブルガリア語.
		{ TYPE_LanguageBasque			, BRWE_LANGUAGE_BASQUE				},	// バスク語.
	};

	ushort us_LangDataSize = sizeof( st_LangData )/sizeof( LangugeData );
	for( ushort i = 0 ; i < us_LangDataSize ; i++ ) {
		// 言語データが見つかったとき.
		if ( st_LangData[i].e_Language == e_Language ) {
			// エンジンに返す定義の設定.
			uc_LangData = st_LangData[i].uc_LangData;
		}
	}
	return uc_LangData;
}

//------------------------------------------------------------------------------.
///	\brief	機能：通常ブラウザ起動回数情報取得.
///	\par	備考：.
///	\param	なし.
///	\result	short 通常ブラウザ起動回数.
//------------------------------------------------------------------------------.
short FBPC_FallbackManager::get_StartFrequency()
{
	FBPD_TRACE_SCOPE();

	return FBPC_FallbackKeepData::newInstance()->get_StartFrequency();
}

//------------------------------------------------------------------------------.
///	\brief	機能：gmtimeの補正時間情報設定.
///	\par	備考：.
///	\param	なし.
///	\result	int32_t 補正時間情報設定.
//------------------------------------------------------------------------------.
int32_t FBPC_FallbackManager::getSystemGMTTime()
{
	FBPD_TRACE_SCOPE();

	int32_t sl_tdiff( 0 );
	APIC_SystemSetting* pc_systemSetting =  APIC_SystemSetting::newInstance();

	// サマータイム補正.
	TYPC_WeeklyTimer pc_WeeklyTimer = pc_systemSetting->getWeeklyTimer();
	if ( pc_WeeklyTimer.b_SummerTimeOn ) {
		// サマータイム設定がONの場合.
		sl_tdiff -= pc_WeeklyTimer.us_SummerTimeMin*60;
	}
	// タイムゾーン補正.
	TYPC_HourMinute pc_HourMinute = APIC_MailSendSetting::newInstance()->getTimeZone();
	sl_tdiff -= ( ( pc_HourMinute.sc_Hour*60*60 ) + ( pc_HourMinute.sc_Minute*60 ) );

	return sl_tdiff;
}

//------------------------------------------------------------------------------.
///	\brief	機能：WebKitBrowserバージョンの取得.
///	\par	備考：.
///	\param	char*    psc_WebKitBrowserVerStr [O] ： 取得先バッファ.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::getWebKitBrowserVerStr( char* psc_WebKitBrowserVerStr )
{
	FBPD_TRACE_SCOPE();

	if ( psc_WebKitBrowserVerStr == NULL ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackManager::getWebKitBrowserVerStr arg Null Error!!\n" );
		return;
	}
	
	// 機種バージョンを取得.
	uchar uc_MZFWVersion = getMZFWVersion();
	
	// WebKitBrowserのバージョンの文字列を格納する.
	if ( BRWE_MZFWVer_002 <= uc_MZFWVersion ) {
		switch (APIC_SystemInfo::newInstance()->getFuncVer2()) {
		case TYPE_FuncVer_1st:				// Poseidon(基本).
			snprintf(psc_WebKitBrowserVerStr, BRWD_UA_WebKitBrowserVerStrCnt, FBPD_UA_WebkitBrowserVer);
			break;
		case TYPE_FuncVer_Expansion1:		// Poseidon(機能拡張1)、ZeusS(基本).
			snprintf(psc_WebKitBrowserVerStr, BRWD_UA_WebKitBrowserVerStrCnt, FBPD_UA_WebkitBrowserVer_v2_1);  // ZeusS WebKitBrowser version 2.1.
			break;
		case TYPE_FuncVer_Expansion2:		// Poseidon(機能拡張2)、ZeusS(機能拡張1)、Heliios(基本).
		case TYPE_FuncVer_Expansion3:		// Poseidon(機能拡張3)、ZeusS(機能拡張2)、Heliios(機能拡張1)、MinervaSBK(基本).
			snprintf(psc_WebKitBrowserVerStr, BRWD_UA_WebKitBrowserVerStrCnt, FBPD_UA_WebkitBrowserVer_v2_2);  // HeliosL WebKitBrowser version 2.2.
			break;
		case TYPE_FuncVer_Expansion4:		// Poseidon(機能拡張4)、ZeusS(機能拡張3)、Heliios(機能拡張2)、MinervaSBK(機能拡張1)、ZeusS ZX0(基本).
		case TYPE_FuncVer_Expansion5:		// Icarus（基本）.
		case TYPE_FuncVer_Expansion6:		// Poseidon（機能拡張5）、ZeusS（機能拡張4）、Helios（機能拡張3）、MinervaSBK（機能拡張2）、ZeusS Z/X/0（機能拡張1）、ZeusSBK_1st（基本）、VenusMLK(基本）.
			snprintf(psc_WebKitBrowserVerStr, BRWD_UA_WebKitBrowserVerStrCnt, FBPD_UA_WebkitBrowserVer_v3_0);  // ZeusZX0 WebKitBrowser version 3.0.
			break;
		case TYPE_FuncVer_Expansion9:
		case TYPE_FuncVer_Expansion10:
			snprintf(psc_WebKitBrowserVerStr, BRWD_UA_WebKitBrowserVerStrCnt, FBPD_UA_WebkitBrowserVer_v3_1);  // ZeusS IT5_4.0 WebKitBrowser version 3.1.
			break;
		default:
			// 下記のログが出力された場合は、機能バージョン対応漏れが発生している為、対応を行うこと。.
			FBPG_DebugPrint( 3, "FBPC_FallbackManager::getWebKitBrowserVerStr  FuncVersion( Unknown ) getFuncVer2( %d )\n", ( int32_t )APIC_SystemInfo::newInstance()->getFuncVer2() );
			snprintf(psc_WebKitBrowserVerStr, BRWD_UA_WebKitBrowserVerStrCnt, FBPD_UA_WebkitBrowserVer);
			break;
		}
	} else {
		snprintf(psc_WebKitBrowserVerStr, BRWD_UA_WebKitBrowserVerStrCnt, FBPD_UA_WebkitBrowserVer_v1_1);
	}
}

//------------------------------------------------------------------------------.
///	\brief	機能：Gesture有効無効設定取得.
///	\par	備考：.
///	\param	なし.
///	\result	Bool 結果.
///				True		:有効.
///				False		:無効.
//------------------------------------------------------------------------------.
Bool FBPC_FallbackManager::getGestureEnable()
{
	FBPD_TRACE_SCOPE();

	Bool b_ret = False;
	
	// ソフトウェアDipSwitch FBPD_SOFTDIPSW_MTUIGESTURE 0：有効 0以外：無効.
	if ( False == APIC_SoftDipSw::newInstance()->getSoftDipSwBit( 96, 1 ) ) {
		b_ret = True;
	}
	
	return b_ret;
}

//------------------------------------------------------------------------------.
///	\brief	機能：25ピクセル対応有効無効に応じたフレームサイズを返す.
///	\par	備考：.
///	\param	FBPE_FrameMerginInfo e_Info [I] .
///										FBPE_FrameMerginDisable:無効.
///										FBPE_FrameMerginEnable :有効.
///	\result	BRWS_FrameMergin マージン情報.
//------------------------------------------------------------------------------.
BRWS_FrameMergin FBPC_FallbackManager::createFrameMergin( FBPE_FrameMerginInfo e_Info )
{
	FBPD_TRACE_SCOPE();

	BRWS_FrameMergin s_FrameMerginDisable = {
		FBPE_FrameMerginDisable_X,
		FBPE_FrameMerginDisable_Y,
		FBPE_FrameMerginDisable_Zoom
	};
	BRWS_FrameMergin s_FrameMerginEnable = {
		FBPE_FrameMerginEnable_X,
		FBPE_FrameMerginEnable_Y,
		FBPE_FrameMerginEnable_Zoom
	};

	BRWS_FrameMergin s_FremeMergin = s_FrameMerginDisable;

	switch( e_Info ) {
	case FBPE_FrameMerginDisable:
		s_FremeMergin = s_FrameMerginDisable;
		break;
	case FBPE_FrameMerginEnable:
		s_FremeMergin = s_FrameMerginEnable;
		break;
	default:
		break;
	}

	FBPG_DebugPrint( 3, "[%s] arg( %d )  us_X=%hu, us_Y=%hu, ui32_Zoom=%u \n", __PRETTY_FUNCTION__, e_Info, s_FremeMergin.us_X, s_FremeMergin.us_Y,  s_FremeMergin.ui32_Zoom);

	return s_FremeMergin;
}

//------------------------------------------------------------------------------.
///	\brief	機能：10.1インチ判定.
///	\par	備考：.
///	\param	なし.
///	\result	Bool 結果.
///				True		:有効.
///				False		:無効.
//------------------------------------------------------------------------------.
Bool FBPC_FallbackManager::get10pt1InchScreenMode()
{
	FBPD_TRACE_SCOPE();

	Bool b_OnOff = False;
	TYPS_PanelHardResolution s_panelResolution = APIC_SystemInfo::newInstance()->getPanelHardResolution();
	if ( 10.1f == s_panelResolution.sf_PanelSize ) {
		b_OnOff =True;
	}
	return b_OnOff;
}

//------------------------------------------------------------------------------.
///	\brief	機能：起動パラメータ デフォルト設定.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::setBrowserBootupBasic()
{
	FBPD_TRACE_SCOPE();

	char sc_PathPublic[BRWD_INITIALUSERDIRECTORY] = {};
	static const char sc_PublicUser[] = "Public";

	snprintf( sc_PathPublic, ( sizeof( sc_PathPublic ) ), "%s%s",  getRootPath( SFLE_DirTypeBrowserPublic ), "/" );
	// ブラウザ設定情報の共通部分の設定.
	m_s_InitialSettingInfo.ul_FitIntoPane 			= BRWD_FITINTOPANE_DEFAULT;		// 画面幅に表示を合わせるかどうかの設定.
	m_s_InitialSettingInfo.e_ImageDisables 			= BRWE_IST_DEFAULT;				// 画像の有効/無効設定.
	m_s_InitialSettingInfo.e_DisplayTable 			= BRWE_IST_DEFAULT;				// コンテンツに記述されたtable 要素を表形式で表示するかどうかの設定.
	m_s_InitialSettingInfo.e_CSSDisables 			= BRWE_IST_DEFAULT;				// CSSの設定.
	m_s_InitialSettingInfo.e_Kinsoku 				= BRWE_IST_DEFAULT;				// 禁則処理の設定.
	m_s_InitialSettingInfo.e_PopupWindow 			= BRWE_IST_DEFAULT;				// ポップアップウィンドウの設定.
	m_s_InitialSettingInfo.e_AnimateImage 			= BRWE_IST_DEFAULT;				// アニメーション表示するかどうかの設定.
	m_s_InitialSettingInfo.e_RunJavaScript 			= BRWE_IST_DEFAULT;				// JavaScript を動作させるかどうかの設定.
	m_s_InitialSettingInfo.e_WordWrap 				= BRWE_IST_DEFAULT;				// ワードラップの設定.
	m_s_InitialSettingInfo.e_RapidRender 			= BRWE_IST_DEFAULT;				// RapidRenderの設定.
	m_s_InitialSettingInfo.ul_SelectColor 			= BRWD_SELECTCOLOR_DEFAULT;		// 選択色の設定.
	m_s_InitialSettingInfo.e_SSLNotifyBefore 		= BRWE_IST_DEFAULT;				// 保護あり/なしのページ間の移動通知の設定.
	m_s_InitialSettingInfo.e_SSLVersion2 			= BRWE_IST_DEFAULT;				// SSL v2の設定.
	m_s_InitialSettingInfo.e_SSLVersion3 			= BRWE_IST_DEFAULT;				// SSL v3の設定.
	m_s_InitialSettingInfo.e_TLSVersion1 			= BRWE_IST_DEFAULT;				// TLS v1の設定.
	m_s_InitialSettingInfo.e_ProxyAuth				= BRWE_IST_DEFAULT;				// プロキシ認証を行なうかどうかの設定.
	m_s_InitialSettingInfo.uc_ProxyUserID[0]		= BRWD_BLANKVALUE;				// プロキシ認証ユーザーID.
	m_s_InitialSettingInfo.uc_ProxyPass[0]			= BRWD_BLANKVALUE;				// プロキシ認証パスワード.
	m_s_InitialSettingInfo.e_UseHTTP11OverProxy 	= BRWE_IST_DEFAULT;				// プロキシ経由で接続するときにHTTP/1.1 を使用するかどうかの設定.
	m_s_InitialSettingInfo.uc_ProxyHost[0]			= BRWD_BLANKVALUE;				// プロキシのホスト名.
	m_s_InitialSettingInfo.uc_ProxyHTTPPort[0]		= BRWD_BLANKVALUE;				// HTTPのプロキシのポート番号.
	m_s_InitialSettingInfo.uc_ProxyHTTPSPort[0]		= BRWD_BLANKVALUE;				// HTTPSのプロキシのポート番号.
	m_s_InitialSettingInfo.uc_NoProxyHosts[0]		= BRWD_BLANKVALUE;				// プロキシサーバ経由で接続する必要のないホストの設定.
	m_s_InitialSettingInfo.sc_WebkitBrowserVer[0]	= BRWD_BLANKVALUE;
	memcpy( m_s_InitialSettingInfo.uc_UserDirectory, ( uchar* )sc_PathPublic, sizeof( sc_PathPublic ) );
	m_s_InitialSettingInfo.us_UserDirectorySize		= strlen( sc_PathPublic );
	m_s_InitialSettingInfo.sl_UserNo				= TYPD_IndexUserPublic;
	memcpy( m_s_InitialSettingInfo.sc_UserName, sc_PublicUser, sizeof( sc_PublicUser ) );
	m_s_InitialSettingInfo.ul_UserType				= BRWD_USERTYPE_PUBLIC;
	m_s_InitialSettingInfo.bl_RecognizeExitTag 		= False; 						//Enable Fiery Exit status flag.
	m_s_InitialSettingInfo.bl_RecognizeLastPageSeting = False; 						//Enable Browser LastPage setting.2015.04.17 [HCL SHIKHA] [CQ511226]

	m_s_InitialSettingInfo.us_LangType				= changeLangugeDataToEngine( APIC_MachineSetting::newInstance()->getLanguage() );		//言語タイプ.

#ifdef LINUX
	const char* psc_FontFilePath = getFontPathType();
	if ( psc_FontFilePath == NULL ) {
		FBPG_DebugPrint( 3, "[%s]getFontPathType() return NULL.\n", __PRETTY_FUNCTION__);
	} else {
		strncpy( m_s_InitialSettingInfo.sc_FontFilePath, psc_FontFilePath, ( BRWD_INITIALFONTFILEPATH - 1 ) );
	}
#else
	const char* psc_FontFilePath = FBPD_FontFilePath;
	strncpy( m_s_InitialSettingInfo.sc_FontFilePath, psc_FontFilePath, ( BRWD_INITIALFONTFILEPATH - 1 ) );
#endif

	m_s_InitialSettingInfo.ss_StartFrequency	= get_StartFrequency();

	//仕向け情報を取得.
	APIC_MachineSetting* pc_MachineSetting = APIC_MachineSetting::newInstance();
	if ( NULL == pc_MachineSetting ) {
		FBPG_DebugPrint( 3, "[%s]APIC_MachineSetting::newInstance() return NULL.\n", __PRETTY_FUNCTION__);

	} else {
		m_s_InitialSettingInfo.e_SystemType = pc_MachineSetting->getSystemType();
	}
	
	m_s_InitialSettingInfo.bl_UserInput         = True;                                    // ユーザー入力可能.

	// gmtimeの補正時間情報設定.
	m_s_InitialSettingInfo.sl_TimeDifference = getSystemGMTTime();

	//WebKitBrowserバージョンの設定.
	getWebKitBrowserVerStr( m_s_InitialSettingInfo.sc_WebkitBrowserVer );
	m_s_InitialSettingInfo.b_GestureEnable = getGestureEnable();
	m_s_InitialSettingInfo.uc_MZFWVersion = getMZFWVersion();
	m_s_InitialSettingInfo.s_FrameMergin  = createFrameMergin( FBPE_FrameMerginDisable );
	m_s_InitialSettingInfo.b_InitPageUnload = False;

#ifdef DEF_LCD_7INCH
	m_s_InitialSettingInfo.bl_7inch	= True;	
#else	//#ifdef DEF_LCD_7INCH.
	m_s_InitialSettingInfo.bl_7inch	= False;
#endif	//#ifdef DEF_LCD_7INCH.

	// 拡張JavaScriptツールバー/ステータスバー一時利用モード使用不可に設定.
	m_s_InitialSettingInfo.bl_JavaScript_Toolbar = False;

	m_s_InitialSettingInfo.b_SlideMenuSetting = False;

	if (TYPE_FIPSMode_Enable == APIC_FIPSSetting::newInstance()->getFIPSMode()) {
		m_s_InitialSettingInfo.b_FipsModeEnable = True;
	} else {
		m_s_InitialSettingInfo.b_FipsModeEnable = False;
	}
	
	m_s_InitialSettingInfo.bl_Mute = False;							// ミュート設定 スピーカーボリューム設定値.

	m_s_InitialSettingInfo.e_TLSVersion1_1	= BRWE_IST_DEFAULT;		// TLS v1_1の設定.
	m_s_InitialSettingInfo.e_TLSVersion1_2	= BRWE_IST_DEFAULT;		// TLS v1_2の設定.

	m_s_InitialSettingInfo.bl_SSL30_DefOFF = True;

	if (get10pt1InchScreenMode()== True){ 
		m_s_InitialSettingInfo.bl_10pt1Inch = True;
	}else {
		m_s_InitialSettingInfo.bl_10pt1Inch = False;
	}

	m_s_InitialSettingInfo.bl_10pt1InchExtendedMode = False;
	//現状、bl_10pt1InchExtendedModeはTrueになる事ないので、s_SetEntryKeyInfo/uc_LedStateの設定は不要.

	m_s_InitialSettingInfo.e_ShaEnabledStatus = BRWE_ShaEnabled;

	m_s_InitialSettingInfo.bl_FaxEnable = False;
	// fallbackモード時はFax使えないのでFalse固定.

	//[EagleH][TLS1.3対応] Start.
	m_s_InitialSettingInfo.e_TLSVersion1_3	= BRWE_IST_DEFAULT;		// TLS v1_3の設定.
	//[EagleH][TLS1.3対応] End.
// [EagleZ+][リモートインスペクタ有効化改善対応] Start.
	m_s_InitialSettingInfo.bl_RemoteInspector = False;				// リモートインスペクタの有効／無効.
// [EagleZ+][リモートインスペクタ有効化改善対応] End.

	// [SecV6][RQ-922] SHA1_TLS1.0_1.1削除.
	m_s_InitialSettingInfo.sc_CipherSuiteList[0] = BRWD_BLANKVALUE;	//暗号スイート一覧.
}

//------------------------------------------------------------------------------.
///	\brief	機能：ChromiumBrowserバージョンの取得.
///	\par	備考：.
///	\param	char*    psc_ChromiumBrowserVerStr [O] ： 取得先バッファ.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::getChromiumBrowserVerStr( char* psc_ChromiumBrowserVerStr )
{
	FBPD_TRACE_SCOPE();

	if( NULL == psc_ChromiumBrowserVerStr ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackManager::getChromiumBrowserVerStr arg Null Error!!\n" );
		return;
	}
	
	uchar uc_MZFWVersion = getMZFWVersion();
	
	if( uc_MZFWVersion >= BRWE_MZFWVer_002 ) {
		switch( APIC_SystemInfo::newInstance()->getFuncVer2() ) {
		case TYPE_FuncVer_1st:
			snprintf( psc_ChromiumBrowserVerStr, BRWD_UA_WebKitBrowserVerStrCnt, FBPD_UA_WebkitBrowserVer_Chrome );
			break;
		case TYPE_FuncVer_Expansion1:
			snprintf( psc_ChromiumBrowserVerStr, BRWD_UA_WebKitBrowserVerStrCnt, FBPD_UA_WebkitBrowserVer_v2_1 );
			break;
		case TYPE_FuncVer_Expansion2:
		case TYPE_FuncVer_Expansion3:
			snprintf( psc_ChromiumBrowserVerStr, BRWD_UA_WebKitBrowserVerStrCnt, FBPD_UA_WebkitBrowserVer_v2_2 );
			break;
		case TYPE_FuncVer_Expansion4:
		case TYPE_FuncVer_Expansion5:
		case TYPE_FuncVer_Expansion6:
			snprintf( psc_ChromiumBrowserVerStr, BRWD_UA_WebKitBrowserVerStrCnt, FBPD_UA_WebkitBrowserVer_v3_0 );
			break;
		default:
			printf( "[getChromiumBrowserVerStr] FuncVersion( Unknown ) getFuncVer2( %d )\n", ( int )APIC_SystemInfo::newInstance()->getFuncVer2() );
			snprintf( psc_ChromiumBrowserVerStr, BRWD_UA_WebKitBrowserVerStrCnt, FBPD_UA_WebkitBrowserVer_Chrome );
			break;
		}
	} else {
		snprintf( psc_ChromiumBrowserVerStr, BRWD_UA_WebKitBrowserVerStrCnt, FBPD_UA_WebkitBrowserVer_v1_1 );
	}
}

//------------------------------------------------------------------------------.
///	\brief	機能：UserAgent取得.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::getUserAgentStr_Chromium()
{
	FBPD_TRACE_SCOPE();

	char sc_ChromiumBrowserVer[ BRWD_UA_WebKitBrowserVerStrCnt ] = {};
	getChromiumBrowserVerStr( sc_ChromiumBrowserVer );

	uchar uc_MZFWVersion = getMZFWVersion();

	char sc_SolutionVer[ FBPD_UA_StrCnt ] = {};
	char sc_OptionVer[ FBPD_UA_StrCnt ] = {};

	if( uc_MZFWVersion >= BRWE_MZFWVer_002 ) {
		snprintf( sc_SolutionVer, sizeof( sc_SolutionVer ), "%s%s%s%s",
		          FBPD_UA_Solution_1, FBPD_UA_Solution_1, FBPD_UA_Solution_0, FBPD_UA_Solution_1 );
		snprintf( sc_OptionVer, sizeof( sc_OptionVer ), "%s%s%s%s%s",
		          FBPD_UA_Option_0, FBPD_UA_Option_1, FBPD_UA_Option_0, FBPD_UA_Option_0, FBPD_UA_Option_1 );
	} else {
		snprintf( sc_SolutionVer, sizeof( sc_SolutionVer ), "%s%s%s",
		          FBPD_UA_Solution_1, FBPD_UA_Solution_1, FBPD_UA_Solution_0 );
		snprintf( sc_OptionVer, sizeof( sc_OptionVer ), "%s%s%s%s%s",
		          FBPD_UA_Option_0, FBPD_UA_Option_1, FBPD_UA_Option_1, FBPD_UA_Option_0, FBPD_UA_Option_1 );
	}

	char sc_CustomMajorVer[ FBPD_UA_CustomStrCnt ] = {};
	char sc_CustomMinorVer[ FBPD_UA_CustomStrCnt ] = {};
	
	if( uc_MZFWVersion >= BRWE_MZFWVer_002 ) {
		switch( APIC_SystemInfo::newInstance()->getFuncVer2() ) {
		case TYPE_FuncVer_1st:
			snprintf( sc_CustomMajorVer, FBPD_UA_CustomStrCnt, FBPD_UA_MajorVer );
			snprintf( sc_CustomMinorVer, FBPD_UA_CustomStrCnt, FBPD_UA_MinorVer );
			 break;
		case TYPE_FuncVer_Expansion1:
			snprintf( sc_CustomMajorVer, FBPD_UA_CustomStrCnt, FBPD_UA_MajorVer );
			snprintf( sc_CustomMinorVer, FBPD_UA_CustomStrCnt, FBPD_UA_MinorVer_01 );
			break;
		case TYPE_FuncVer_Expansion2:
		case TYPE_FuncVer_Expansion3:
			snprintf( sc_CustomMajorVer, FBPD_UA_CustomStrCnt, FBPD_UA_MajorVer );
			snprintf( sc_CustomMinorVer, FBPD_UA_CustomStrCnt, FBPD_UA_MinorVer_02 );
			break;
		case TYPE_FuncVer_Expansion4:
			snprintf( sc_CustomMajorVer, FBPD_UA_CustomStrCnt, FBPD_UA_MajorVer_Chrome );
			snprintf( sc_CustomMinorVer, FBPD_UA_CustomStrCnt, FBPD_UA_MinorVer );
			break;
		default:
			snprintf( sc_CustomMajorVer, FBPD_UA_CustomStrCnt, FBPD_UA_MajorVer_Chrome );
			snprintf( sc_CustomMinorVer, FBPD_UA_CustomStrCnt, FBPD_UA_MinorVer );
			break;	// [MFPソリューション分離step1] 静的解析対応.
		}
	} else {
		snprintf( sc_CustomMajorVer, FBPD_UA_CustomStrCnt, FBPD_UA_MajorVer );    
		snprintf( sc_CustomMinorVer, FBPD_UA_CustomStrCnt, FBPD_UA_MinorVer ); 
	}

	if( uc_MZFWVersion >= BRWE_MZFWVer_002 ) {
	//Creation of complete User Agent String.
		snprintf( ( char* )m_s_InitialSettingInfo.uc_UserAgent, FBPD_UA_StrCnt, FBPD_BrowserUserAgent_Chrome,
																				FBPD_UA_MozillaVer,
																				FBPD_UA_AppleWebkitVer_Chrome,
																				sc_ChromiumBrowserVer,
																				FBPD_UA_SafariVer_Chrome,
																				sc_CustomMajorVer,
																				sc_CustomMinorVer,
																				sc_SolutionVer,
																				sc_OptionVer,
																				FBPD_UA_VendorVer1 );
	} else {
		snprintf( ( char* )m_s_InitialSettingInfo.uc_UserAgent, FBPD_UA_StrCnt, FBPD_BrowserUserAgent_Chrome,
																				FBPD_UA_MozillaVer,
																				FBPD_UA_AppleWebkitVer_Chrome,
																				sc_ChromiumBrowserVer,
																				FBPD_UA_SafariVer_Chrome,
																				sc_CustomMajorVer,
																				sc_CustomMinorVer,
																				sc_SolutionVer,
																				sc_OptionVer,
																				FBPD_UA_VendorVer1 );
	}
}

//------------------------------------------------------------------------------.
///	\brief	機能：フレーム有効無効を返す.
///	\par	備考：.
///	\param	なし.
///	\result	FBPE_FrameMerginInfo マージン情報.
//------------------------------------------------------------------------------.
FBPE_FrameMerginInfo FBPC_FallbackManager::getFrameMerginInfo()
{
	FBPD_TRACE_SCOPE();

	APIC_SoftDipSw* p_Sw = APIC_SoftDipSw::newInstance();
	if ( NULL == p_Sw ) {
		FBPG_DebugPrint( 3, "[%s]APIC_SoftDipSw* == NULL -> return FBPE_FrameMerginEnable \n", __PRETTY_FUNCTION__);
		return FBPE_FrameMerginEnable;
	}

	// DipSW No.143(bit 0,1)：NVRAM 64-6(bit 0),64-7(bit 1).
	// タッチパネルの不感領域の設定切替え(bit 0,1).
	//  0x00  不感領域：26dot（デフォルト）  枠あり（100%）.
	//  0x01  不感領域：16dot                枠無し.
	//  0x02  不感領域： 9dot                枠無し.
	uchar uc_DipSw = 0x00;
	if ( True == p_Sw->getSoftDipSwBit( 64, 6 ) ) {	uc_DipSw |= 0x01;	}
	if ( True == p_Sw->getSoftDipSwBit( 64, 7 ) ) {	uc_DipSw |= 0x02;	}

	FBPE_FrameMerginInfo e_Info = FBPE_FrameMerginEnable;

	switch( uc_DipSw ) {
	case 0x00:
		e_Info = FBPE_FrameMerginEnable;
		break;
	default:
		e_Info = FBPE_FrameMerginDisable;
		break;
	}
	return e_Info;
}

//------------------------------------------------------------------------------.
///	\brief	機能：起動パラメータ カスタム設定(BRWE_BrowserBootMode_Default).
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::setBrowserBootupCustom_Default()
{
	FBPD_TRACE_SCOPE();

	const BRWS_RebootSettingInfo* s_Info = NULL;
	s_Info = FBPC_FallbackKeepData::newInstance()->getReqRebootInfo();

	m_s_InitialSettingInfo.bl_ToolBar 			= True;								// ツールバーの表示/非表示.
	m_s_InitialSettingInfo.bl_tab 				= True;								// タブの表示/非表示.
	m_s_InitialSettingInfo.bl_StatusBar			= True;								// ステータスバー表示/非表示.
	m_s_InitialSettingInfo.bl_ErrorDialog 		= True;								// エラーダイアログの表示/非表示.
	m_s_InitialSettingInfo.bl_BrwSpecificAdminSetting = True;

	// ブラウザ起動時に表示するコンテンツのURL.
	if ( 0x00 == s_Info->sc_BrowserURL[0] ) {
		FBPG_DebugPrint( 3, "[%s] url = 0x00 \n", __PRETTY_FUNCTION__);
		m_s_InitialSettingInfo.uc_StartPage[0] = BRWD_BLANKVALUE;						// ブラウザ起動時に表示するコンテンツのURL.
	} else {
		FBPG_DebugPrint( 3, "[%s] url = %s \n", __PRETTY_FUNCTION__, s_Info->sc_BrowserURL);
		memcpy( m_s_InitialSettingInfo.uc_StartPage, ( uchar* )s_Info->sc_BrowserURL, sizeof( s_Info->sc_BrowserURL ) );
	}

	m_s_InitialSettingInfo.ul_StartPageSetting 	= BRWD_STARTPAGE_DEFAULT;					// スタートページ設定.

	m_s_InitialSettingInfo.e_CookieDeleteTiming = FBPE_CookieNoDelete;
	m_s_InitialSettingInfo.e_CertificationDeleteTiming = FBPE_CertificationDeleteNoDelete;

	m_s_InitialSettingInfo.ul_CookieMode			= BRWD_COOKIE_DEFAULT;					// COOKIE設定.
	m_s_InitialSettingInfo.bl_WebdataEnableMode		= True;
	m_s_InitialSettingInfo.e_CacheMode 				= BRWE_IST_DEFAULT;						// キャッシュモードの設定.

	if ( 0 == s_Info->ss_BrowserTimeOut ) {
		m_s_InitialSettingInfo.sl_HTTPConnectTimeout = 20000;								// TCP接続タイムアウト設定.
	} else {
		m_s_InitialSettingInfo.sl_HTTPConnectTimeout = s_Info->ss_BrowserTimeOut * 1000;	// TCP接続タイムアウト設定.
	}

	m_s_InitialSettingInfo.sl_HTTPReqTimeout = s_Info->ss_BrowserTimeOut * 1000;			// リクエスト送信タイムアウト設定.
	m_s_InitialSettingInfo.sl_HTTPRspTimeout = s_Info->ss_BrowserTimeOut * 1000;			// レスポンス受信タイムアウト設定.

	// UserAgentを設定する.
	getUserAgentStr_Chromium();

	m_s_InitialSettingInfo.bl_Historie 			= True;										// 遷移履歴の設定.
	m_s_InitialSettingInfo.bl_Visit 			= True;										// 表示履歴（既訪問リンク）URLの設定.
	m_s_InitialSettingInfo.e_UseProxy 			= BRWE_IST_DEFAULT;							//【307783】【ブラウザ対応】【13山口】merge.

	m_s_InitialSettingInfo.bl_isPdfXpsEnable = False;

	m_s_InitialSettingInfo.bl_DefaultBrwSetting = False;

	m_s_InitialSettingInfo.s_FrameMergin = createFrameMergin( getFrameMerginInfo() );
	// PDFのデフォルト表示を通常ツールバーに設定.
	m_s_InitialSettingInfo.e_PdfToolBarDefaultType = WKBE_PdfToolBarType_Show;
	m_s_InitialSettingInfo.bl_RecognizeLastPageSeting = True;	//Enable Browser LastPage setting.
	
// [次世代パネルv2：マルチウィンドウ対応] Start.
	m_s_InitialSettingInfo.bl_Stay					=	s_Info->bl_Stay;
	m_s_InitialSettingInfo.us_BrowserAppID			=	s_Info->us_BrowserAppID;
	m_s_InitialSettingInfo.bl_LoadURL				=	s_Info->bl_LoadURL;
	m_s_InitialSettingInfo.bl_ShowMode				=	s_Info->bl_ShowMode;
	m_s_InitialSettingInfo.bl_BrowserAppRecreate	=	s_Info->bl_BrowserAppRecreate;
// [次世代パネルv2：マルチウィンドウ対応] End.

}

//------------------------------------------------------------------------------.
///	\brief	機能：起動パラメータ カスタム設定(BRWE_BrowserBootMode_Launcher).
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::setBrowserBootupCustom_Launcher()
{
	FBPD_TRACE_SCOPE();

	BRWS_RebootSettingInfo* s_Info = NULL;
	s_Info = FBPC_FallbackKeepData::newInstance()->getReqRebootInfo();

	// ツールバーの表示設定.
	if ( TYPE_ExtToolBarSettingTypeShow == s_Info->s_ToolBarSetting.e_ToolBarSettingType ) {
		//IWSでツールバー表示はないので流用対象外とする.とする.
	} else {
		m_s_InitialSettingInfo.bl_HomeControl = True;
		m_s_InitialSettingInfo.bl_ToolBar = False;											// ツールバーの非表示.

		if ( TYPE_ExtStatusBarSettingTypeShow == s_Info->s_StatusBarSetting.e_StatusBarSettingType ) {
			m_s_InitialSettingInfo.bl_tab = True;											// タブの表示/非表示.
			m_s_InitialSettingInfo.bl_StatusBar = True;										// ステータスバー表示/非表示.
		} else {
			m_s_InitialSettingInfo.bl_tab = False;											// タブの非表示.
			m_s_InitialSettingInfo.bl_StatusBar = False;									// ステータスバー表示/非表示.
		}
		m_s_InitialSettingInfo.bl_ErrorDialog = True;											// エラーダイアログの表示/非表示.

		// ツールバー非表示のブラウザランチャーでURLが無い場合は、IT3に合わせて空白ページとする.
		if ( '\0' == s_Info->sc_BrowserURL[0] ) {
			strncpy( ( char* )m_s_InitialSettingInfo.uc_StartPage, FBPD_InitialURL, sizeof( m_s_InitialSettingInfo.uc_StartPage ) );
		} else {
			strncpy( ( char* )m_s_InitialSettingInfo.uc_StartPage, s_Info->sc_BrowserURL, sizeof( s_Info->sc_BrowserURL ) );
		}
		m_s_InitialSettingInfo.ul_StartPageSetting = BRWD_STARTPAGE_DEFAULT;				// スタートページ設定.
		m_s_InitialSettingInfo.ul_CookieMode = BRWD_COOKIE_DEFAULT;							// COOKIE設定.
		m_s_InitialSettingInfo.bl_WebdataEnableMode		= True;									//[FQ-ISD2G-1700005-OFP112] [06/01/2017] [HCL Rajat] Fix.

		if ( TYPE_ExtCacheFunctionUsageSettingDisable == s_Info->e_CacheMode ) {
			m_s_InitialSettingInfo.e_CacheMode 	= BRWE_IST_FALSE;
		} else {
			m_s_InitialSettingInfo.e_CacheMode  = BRWE_IST_DEFAULT;
		}
		
		if ( 0 == s_Info->ss_BrowserTimeOut ) {
			s_Info->ss_BrowserTimeOut = 20;
		}

		m_s_InitialSettingInfo.sl_HTTPConnectTimeout	= s_Info->ss_BrowserTimeOut * 1000;	// TCP接続タイムアウト設定.
		m_s_InitialSettingInfo.sl_HTTPReqTimeout		= s_Info->ss_BrowserTimeOut * 1000;	// リクエスト送信タイムアウト設定.
		m_s_InitialSettingInfo.sl_HTTPRspTimeout		= s_Info->ss_BrowserTimeOut * 1000;	// レスポンス受信タイムアウト設定.

		getUserAgentStr_Chromium();															// HTTPヘッダのUser-Agent設定.

		// PDFViewerからの×ボタンでの戻り先は遷移履歴を参照して戻るため有効にする.
		m_s_InitialSettingInfo.bl_Historie = True;											// 遷移履歴の設定.

		m_s_InitialSettingInfo.bl_Visit = False;											// 表示履歴（既訪問リンク）URLの設定.
		m_s_InitialSettingInfo.e_UseProxy = BRWE_IST_DEFAULT;								// プロキシを使用するかどうかの設定.
		m_s_InitialSettingInfo.bl_isPdfXpsEnable = False;

		// 指定アプリ初期起動 未設定時固定の処理を行う.
		m_s_InitialSettingInfo.bl_JavaScript_Toolbar = True;

		if ( True == s_Info->bl_ExtJSUse ){
			m_s_InitialSettingInfo.ul_ExtJsUse = FBPD_ExtJavaScript_Enable_Job;
		}else {
			m_s_InitialSettingInfo.ul_ExtJsUse = FBPD_ExtJavaScript_Disable;
		}

// [次世代パネルv2：マルチウィンドウ対応] Start.
		m_s_InitialSettingInfo.bl_Stay					=	s_Info->bl_Stay;
		m_s_InitialSettingInfo.us_BrowserAppID			=	s_Info->us_BrowserAppID;
		m_s_InitialSettingInfo.bl_LoadURL				=	s_Info->bl_LoadURL;
		m_s_InitialSettingInfo.bl_ShowMode				=	s_Info->bl_ShowMode;
		m_s_InitialSettingInfo.bl_BrowserAppRecreate	=	s_Info->bl_BrowserAppRecreate;
// [次世代パネルv2：マルチウィンドウ対応] End.

	}
}

//------------------------------------------------------------------------------.
///	\brief	機能：セッションID取得.
///	\par	備考：.
///	\param	uint32_t* sessionId [O] セッションID.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::getCurrentSessionId( uint32_t* sessionId )
{
	FBPD_TRACE_SCOPE();

	LAPC_BFW_System::newInstance()->getCurrentSessionId( (uint32_t*)sessionId );
}

//------------------------------------------------------------------------------.
///	\brief	機能：倍率情報取得.
///	\par	備考：.
///	\param	なし.
///	\result	float 倍率情報.
//------------------------------------------------------------------------------.
float FBPC_FallbackManager::getDefaultZoomRatio()
{
	FBPD_TRACE_SCOPE();

	float f_ZoomRatio = 1.0;
	float f_ZoomRatio_width = 0;
	float f_ZoomRatio_Height = 0;

	BRWE_BrowserBootMode e_BrowserBootMode = BRWE_BrowserBootMode_Default;
	e_BrowserBootMode = FBPC_FallbackKeepData::newInstance()->getReqRebootInfo()->e_BrowserBootMode;

	if ( True == m_s_InitialSettingInfo.bl_10pt1Inch ) {
		if ( BRWE_BrowserBootMode_Default != e_BrowserBootMode ) {

			APIC_SoftDipSw* pc_Sw = APIC_SoftDipSw::newInstance();
			if ( pc_Sw ) {
				if ( False == pc_Sw->getSoftDipSwBit( 106, 2 ) ) {
					f_ZoomRatio_Height = (float)FBPD_XLNoExtnModeLargeWindowHeight/FBPD_LargeWindowHeight;
					if ( True == m_s_InitialSettingInfo.bl_tab && False == m_s_InitialSettingInfo.bl_ToolBar ) {
						f_ZoomRatio_Height = (float)( FBPD_XLNoExtnModeLargeWindowHeight-FBPD_TabStsBarHeight)/(FBPD_LargeWindowHeight-FBPD_TabStsBarHeight);
					}
					if ( False == m_s_InitialSettingInfo.bl_tab && True == m_s_InitialSettingInfo.bl_ToolBar ) {
						f_ZoomRatio_Height = (float)( FBPD_XLNoExtnModeLargeWindowHeight - FBPD_MainToolAreaHeight)/(FBPD_LargeWindowHeight - FBPD_MainToolAreaHeight);
					}
					if ( True == m_s_InitialSettingInfo.bl_tab && True == m_s_InitialSettingInfo.bl_ToolBar) {
						f_ZoomRatio_Height = (float)( FBPD_XLNoExtnModeLargeWindowHeight - FBPD_MainToolAreaHeight - FBPD_TabStsBarHeight)/(FBPD_LargeWindowHeight - FBPD_MainToolAreaHeight - FBPD_TabStsBarHeight);
					}
					f_ZoomRatio_width = (float)FBPD_XLModeLargeWindowWidth/FBPD_LargeWindowWidth;
					f_ZoomRatio = (f_ZoomRatio_width>f_ZoomRatio_Height) ?f_ZoomRatio_Height:f_ZoomRatio_width;
				}
			}
		}
	}
	return f_ZoomRatio;
}

//------------------------------------------------------------------------------.
///	\brief	機能：倍率マージン情報取得.
///	\par	備考：.
///	\param	なし.
///	\result	float 倍率マージン情報.
//------------------------------------------------------------------------------.
float FBPC_FallbackManager::getDefaultZoomMargin()
{
	FBPD_TRACE_SCOPE();

	float f_ZoomMarginWidth  = 0;

	if ( True == m_s_InitialSettingInfo.bl_10pt1Inch ) {
		if ( 1 != m_s_InitialSettingInfo.f_ZoomRatio ) {
			if ( FBPD_XLModeLargeWindowWidth > ( FBPD_LargeWindowWidth * m_s_InitialSettingInfo.f_ZoomRatio ) )
				f_ZoomMarginWidth = (float)( (FBPD_XLModeLargeWindowWidth - (m_s_InitialSettingInfo.f_ZoomRatio * FBPD_LargeWindowWidth))/2 );
		}
	}
	return f_ZoomMarginWidth;
}

//------------------------------------------------------------------------------.
///	\brief	機能：起動パラメータ カスタム設定.
///	\par	備考：.
///	\param	BRWE_BrowserBootMode e_BrowserBootMode [I] ： 起動モード.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::setBrowserBootupCustom( BRWE_BrowserBootMode e_BrowserBootMode )
{
	FBPD_TRACE_SCOPE();

	// ホームページ制御の初期化.
	m_s_InitialSettingInfo.bl_HomeControl = False;

	// PDFのデフォルト表示を拡張ツールバーに設定.
	m_s_InitialSettingInfo.e_PdfToolBarDefaultType = WKBE_PdfToolBarType_ExtShow;

//[21/06/2018] [HCL Amit] Add Start.
	m_s_InitialSettingInfo.b_IsContentsSlideModeForSWKeyboard = True;
	m_s_InitialSettingInfo.bl_TLS10_DefOFF = True;
	m_s_InitialSettingInfo.e_SSLCertificateVerSetting = BRWE_SSLCertificateVerificationDisplayOnlyOnce;
	m_s_InitialSettingInfo.s_painNum = FBPD_3PaneMode;
	m_s_InitialSettingInfo.ss_AuthorityID = 0;
	m_s_InitialSettingInfo.bl_WebUIMode = False;
//[21/06/2018] [HCL Amit] Add End.

	// IWS非搭載を固定設定.
	m_s_InitialSettingInfo.bl_IsIWSModel = False;

	// ブラウザ設定情報の起動モード毎の設定.
	switch( e_BrowserBootMode ) {
	case BRWE_BrowserBootMode_Default:  	// デフォルトモード.
		setBrowserBootupCustom_Default();	
		break;
	case BRWE_BrowserBootMode_Launcher:  	// ブラウザーランチャーモード.
	case BRWE_BrowserBootMode_SolutionTop:	// ソリューションTopモード.
		setBrowserBootupCustom_Launcher();
		break;
	default:
		FBPG_DebugPrint( 3, "[%s]case Error BootMode[%d].\n", __PRETTY_FUNCTION__, e_BrowserBootMode);
		break;
	}

	// セッションIDを取得.
	uint32_t sessionId = 0;
	getCurrentSessionId( &sessionId );
	m_s_InitialSettingInfo.ul_SessionId = sessionId;

	// IPv6 アドレス使用有無.
	m_s_InitialSettingInfo.bl_IPv6AddressEnable = false;
	// IPv6 グローバルアドレス.
		m_s_InitialSettingInfo.sc_IPv6GrobalIPAddress[0] = BRWD_BLANKVALUE;

	// IPv4 IPアドレス.
		m_s_InitialSettingInfo.sc_IPAddress[0] = BRWD_BLANKVALUE;

	//OpenAPI連携は未サポートなので、0固定.
	m_s_InitialSettingInfo.ul_CertID = 0;
	//Fallbackモード時は印刷は出来ないので、Web印刷の権限はDISABLE固定.
	m_s_InitialSettingInfo.sc_PrintStatus = BRWE_PRINTLIMIT_DISABEL;

	m_s_InitialSettingInfo.s_ScreenInfo = getScreenInfo();
	m_s_InitialSettingInfo.f_ZoomRatio  = getDefaultZoomRatio();
	m_s_InitialSettingInfo.f_FrameMarginWidth = getDefaultZoomMargin();

}

//------------------------------------------------------------------------------.
///	\brief	機能：起動パラメータ ユーザー情報.
///	\par	備考：.
///	\param  BRWS_InitialSettingInfo* ps_InitSettingInfo [O] ： 初期化情報構造体.
///	\result	なし.
//------------------------------------------------------------------------------.
void FBPC_FallbackManager::setInitialSettingInfoUserParam( BRWS_InitialSettingInfo* ps_InitSettingInfo )
{
	FBPD_TRACE_SCOPE();

	char sc_PathPublic[BRWD_INITIALUSERDIRECTORY] = {};
	static const char sc_PublicUser[] = "Public";

	snprintf( sc_PathPublic, ( sizeof( sc_PathPublic ) ), "%s%s", getRootPath( SFLE_DirTypeBrowserPublic ), "/" );

	ps_InitSettingInfo->us_UserDirectorySize	= strlen( sc_PathPublic );
	memcpy( ps_InitSettingInfo->uc_UserDirectory, ( uchar* )sc_PathPublic, sizeof( sc_PathPublic ) );

	ps_InitSettingInfo->sl_UserNo				= TYPD_IndexUserPublic;
	ps_InitSettingInfo->ul_UserType				= BRWD_USERTYPE_PUBLIC;
	memcpy( ps_InitSettingInfo->sc_UserName, sc_PublicUser, sizeof( sc_PublicUser ) );
	m_s_InitialSettingInfo.b_AdminRights		= False;
}

//------------------------------------------------------------------------------.
///	\brief	機能：ブラウザファイナライズ完了通知イベントハンドラ.
///	\par	備考：.
///	\param  なし.
///	\result	なし.
//------------------------------------------------------------------------------.
void  FBPC_FallbackManager::evt_BrowserFinalizeNotice()
{
	FBPD_TRACE_SCOPE();

	// ブラウザ起動要求.
	BRWC_BrowserMsg	 st_Msg = {};
	st_Msg.ss_type = BR_Msg_BrowserInitializeRequest;

	//起動パラメータ デフォルト設定.
	setBrowserBootupBasic();

	//起動パラメータ カスタム設定.
	setBrowserBootupCustom( FBPC_FallbackKeepData::newInstance()->getReqRebootInfo()->e_BrowserBootMode );

	setInitialSettingInfoUserParam( &m_s_InitialSettingInfo );

	st_Msg.s_OptionParam.s_BrowExInitialSettingInfo.s_InitialSettingInfo = m_s_InitialSettingInfo;

	FBPC_FallbackIpcForBrw::newInstance()->SendMsgBrwEngine( &st_Msg );

}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：サウンド開始要求通知イベントハンドラ.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void  FBPC_FallbackManager::evt_SoundControlStartNotice()
{
	FBPD_TRACE_SCOPE();

	BRWC_BrowserMsg st_Msg = {};
	st_Msg.ss_type = BR_Msg_SoundControlStartRequest;

	FBPC_FallbackIpcForBrw::newInstance()->SendMsgBrwEngine( &st_Msg );
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：サウンド停止要求通知イベントハンドラ.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void  FBPC_FallbackManager::evt_SoundControlStopNotice()
{
	FBPD_TRACE_SCOPE();

	BRWC_BrowserMsg st_Msg = {};
	st_Msg.ss_type = BR_Msg_SoundControlStopRequest;

	FBPC_FallbackIpcForBrw::newInstance()->SendMsgBrwEngine( &st_Msg );
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：ブラウザエンジン描画開始完了応答イベントハンドラ.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void  FBPC_FallbackManager::evt_DrawStartNotice()
{
	FBPD_TRACE_SCOPE();

	// 特に何もしない。
	// 監視プロセスが描画完了通知が必要であれば、ここに追加する.
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：パネル座標キー要求.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void  FBPC_FallbackManager::evt_PanelKeyRequest()
{
	FBPD_TRACE_SCOPE();

	FBPC_FallbackIpcForMfpWatch::newInstance()->exeCmn_procSomethingKeyOn();
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：キー押下時のタイマーリセット通知イベントハンドラ.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void  FBPC_FallbackManager::evt_procSomethingKeyOn()
{
	FBPD_TRACE_SCOPE();

	FBPC_FallbackIpcForMfpWatch::newInstance()->exeCmn_procSomethingKeyOn();
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：システムオートリセット受付通知イベントハンドラ.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void  FBPC_FallbackManager::evt_AutoClearForBrowser()
{
	FBPD_TRACE_SCOPE();

	BRWC_BrowserMsg st_Msg = {};
	st_Msg.ss_type = BR_Msg_AutoClearForBrowser;

	FBPC_FallbackIpcForBrw::newInstance()->SendMsgBrwEngine( &st_Msg );
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：システムオートリセット受付通知（実処理）.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void FBPC_FallbackManager::exeCmn_AutoClearNoticeSet()
{
	FBPD_TRACE_SCOPE();

	// オートリセットを受けたときはHOMEに設定.
	BRWC_BrowserMsg st_Msg = {};
	st_Msg.ss_type = BR_Msg_DirectKeyNotice;

	st_Msg.s_Param.message = MS_Reset;
	st_Msg.s_Param.option = BRWD_RESETREQ_HOMEDIALOGTAB;

	st_Msg.s_Param.type = MT_On;
	FBPC_FallbackIpcForBrw::newInstance()->SendMsgBrwEngine( &st_Msg );

	// ブラウザリセットの通知.
	APIC_IwsAuthScreen* pc_IwsAuthScreen = APIC_IwsAuthScreen::newInstance();
	if ( pc_IwsAuthScreen ) {
		pc_IwsAuthScreen->browserReset();
	}

	st_Msg.s_Param.type = MT_Up;
	FBPC_FallbackIpcForBrw::newInstance()->SendMsgBrwEngine( &st_Msg );
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：システムオートリセット受付通知イベントハンドラ.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void  FBPC_FallbackManager::evt_AutoClearNoticeSet()
{
	FBPD_TRACE_SCOPE();

	exeCmn_AutoClearNoticeSet();
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：Browserオートリセット受付通知イベントハンドラ.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void  FBPC_FallbackManager::evt_AutoClearNoticeBrowser()
{
	FBPD_TRACE_SCOPE();

	if ( True == m_b_AutoClearProcessed ) {
		return;
	}
	m_b_AutoClearProcessed = True;	// オートクリアフラグを済に更新.

	// オートリセットを受けたときはHOMEに設定.
	BRWC_BrowserMsg st_Msg = {};
	st_Msg.ss_type = BR_Msg_DirectKeyNotice;

	st_Msg.s_Param.message = MS_Reset;
	st_Msg.s_Param.option = BRWD_RESETREQ_HOMEDIALOGTAB;

	st_Msg.s_Param.type = MT_On;
	FBPC_FallbackIpcForBrw::newInstance()->SendMsgBrwEngine( &st_Msg );

	// ブラウザリセットの通知.
	APIC_IwsAuthScreen* pc_IwsAuthScreen = APIC_IwsAuthScreen::newInstance();
	if ( pc_IwsAuthScreen ) {
		pc_IwsAuthScreen->browserReset();
	}

	st_Msg.s_Param.type = MT_Up;
	FBPC_FallbackIpcForBrw::newInstance()->SendMsgBrwEngine( &st_Msg );
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：外キー通知イベントハンドラ.
///	\par	備考：.
///	\param  BRWC_BrowserMsg* pst_BrowserMsg [I] ： 要求パラメータ.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void FBPC_FallbackManager::evt_DirectKeyNotice( BRWC_BrowserMsg* pst_BrowserMsg )
{
	FBPD_TRACE_SCOPE();

	if( NULL == pst_BrowserMsg ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackManager::evt_DirectKeyNotice arg Null Error!!\n" );
		return;
	}
	
	BRWC_BrowserMsg st_Msg = {};
	Bool bl_DirectKeyNoticeSendFlag = True;
	
	// メッセージタイプがキー押下以外かつリセットキーの場合.
	if( ( MT_On != pst_BrowserMsg->s_Param.type )
		 && ( MS_Reset == pst_BrowserMsg->s_Param.message ) ) {
		bl_DirectKeyNoticeSendFlag = False;
	}
	
	if( ( MT_On != pst_BrowserMsg->s_Param.type ) && ( MT_Up != pst_BrowserMsg->s_Param.type ) ) {
		bl_DirectKeyNoticeSendFlag = False;
	}
	
	if( bl_DirectKeyNoticeSendFlag ) {
		st_Msg.ss_type = BR_Msg_DirectKeyNotice;

		st_Msg.s_Param.message = pst_BrowserMsg->s_Param.message;
		st_Msg.s_Param.type = pst_BrowserMsg->s_Param.type;
		
		if( MS_Reset == pst_BrowserMsg->s_Param.message ) {
			st_Msg.s_Param.option = BRWD_RESETREQ_HOMEDIALOGTAB;
			// リセットキーの場合のみ音を鳴らす契機を作る.
			BeepPlayReq( FBPD_INPUTCONF_SOUND );
		} else {
			st_Msg.s_Param.option = pst_BrowserMsg->s_Param.option;
		}
		
		FBPC_FallbackIpcForBrw::newInstance()->SendMsgBrwEngine( &st_Msg );
	}
}

//--------------------------------------------------------------------------------------------.
///	\brief	機能：MFPプロセス状態通知イベントハンドラ.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//--------------------------------------------------------------------------------------------.
void  FBPC_FallbackManager::evt_FallbackModeNotice( BRWC_BrowserMsg* pst_BrowserMsg )
{
	FBPD_TRACE_SCOPE();

	if( NULL == pst_BrowserMsg ) {
		FBPG_DebugPrint( 3, "FBPC_FallbackManager::evt_FallbackModeNotice arg Null Error!!\n" );
		return;
	}

	BRWC_BrowserMsg st_Msg = {};
	st_Msg.ss_type = BR_Msg_FallbackModeNotice;
	st_Msg.s_Param.option = pst_BrowserMsg->s_Param.option;
	if( st_Msg.s_Param.option ) {
		st_Msg.s_Param.u_Param1.ui32_Param = 3;
	} else {
		st_Msg.s_Param.u_Param1.ui32_Param = 0;
	}

	FBPC_FallbackIpcForBrw::newInstance()->SendMsgBrwEngine( &st_Msg );
}

//------------------------------------------------------------------------------.
///	\brief	機能：イベントループ.
///	\par	備考：.
///	\param	なし.
///	\result	なし.
//------------------------------------------------------------------------------.
void* FBPC_FallbackManager::ReceiveEvent( void* )
{
	FBP_START_ROUTINE;
	FBPD_TRACE_SCOPE();

	int32_t  si_res = 0;
	static BRWC_BrowserMsg st_Msg = {};

	FBPC_FallbackManager* pc_FallbackManager = FBPC_FallbackManager::newInstance();
	MSG_ID msgId = pc_FallbackManager->getMsgQId();

	UINT8	AppID = 0;																	// アプリID.
	LAPD_NET_MSG_Q_ID toApp = NULL;														// 送信用Queue ID.
	LAPD_NET_MSG_Q_ID toMio = NULL;														// 受信用Queue ID.

	// ネットワークI/Fを利用するためアプリを登録する.
	LAPF_NET_RegistApplication( (const UINT8*)"FallbackProc", &AppID, &toApp, &toMio);

	// MFPプロセスの状態を設定する.
	LAPF_NET_SetMfpProcessStatus( LAPF_NET_MFP_PROCESS_STATUS_NG );

	while( 1 ) {
		// メッセージ受信待ち.
		si_res = FBPG_mqRecive( msgId, (char *)(&st_Msg), sizeof(BRWC_BrowserMsg) );
		if ( FBPD_ERROR != si_res ) {
			//=====================================.
			//		関数タイプ毎に処理振り分け.
			//=====================================.
			switch ( st_Msg.ss_type ) {
			case BR_Msg_BrowserReboot:										// ブラウザ再起動.
				pc_FallbackManager->evt_BrowserReboot( &st_Msg );
				break;
			case BR_Msg_BrowserInitializeNotice:							// ブラウザエンジン起動要求応答.
				pc_FallbackManager->evt_BrowserInitializeNotice();
				break;
			case BR_Msg_BrowserFinalizeNotice:								// ブラウザエンジン終了要求応答.
				pc_FallbackManager->evt_BrowserFinalizeNotice();
				break;
			case BR_Msg_DrawStartNotice:									// ブラウザエンジン描画開始完了応答.
				pc_FallbackManager->evt_DrawStartNotice();
				break;
			case BR_Msg_FileLinkCallBackFunc:								// PDFファイルリンク操作内容コールバック通知.
				pc_FallbackManager->evt_FileLinkCallBackFunc( &st_Msg );
				break;
			case BR_Msg_AppChgNoticeStart:									// アプリケーション切替要求通知（開始）.
				pc_FallbackManager->evt_AppChgNoticeStart();
				break;
			case BR_Msg_AppChgNoticeEnd:									// アプリケーション切替要求通知（終了）.
				pc_FallbackManager->evt_AppChgNoticeEnd();
				break;
			case BR_Msg_SoundControlStartNotice:							// サウンド開始通知.
				pc_FallbackManager->evt_SoundControlStartNotice();
				break;
			case BR_Msg_SoundControlStopNotice:								// サウンド停止通知.
				pc_FallbackManager->evt_SoundControlStopNotice();
				break;
			case BR_Msg_PanelKeyRequest:									// パネル座標キー要求.
				pc_FallbackManager->evt_PanelKeyRequest();
				break;
			case BR_Msg_procSomethingKeyOn:									// キー押下時のタイマーリセット通知.
				pc_FallbackManager->evt_procSomethingKeyOn();
				break;
			case BR_Msg_AutoClearNoticeSet:									// システムオートリセット受付通知.
				pc_FallbackManager->evt_AutoClearForBrowser();
				pc_FallbackManager->evt_AutoClearNoticeSet();
				break;
			case BR_Msg_AutoClearNoticeBrowser:								// Browserオートリセット受付通知.
				pc_FallbackManager->evt_AutoClearNoticeBrowser();
				break;
			case BR_Msg_DirectKeyNotice_MS_Reset:							// 外キー(Reset)通知.
			case BR_Msg_DirectKeyNotice_MS_10Key:							// 外キー(10Key)通知.
			case BR_Msg_DirectKeyNotice_MS_Clear:							// 外キー(Clear)通知.
				pc_FallbackManager->evt_DirectKeyNotice( &st_Msg );
				break;
			case BR_Msg_FallbackModeNotice:									// MFPプロセス状態通知.
				pc_FallbackManager->evt_FallbackModeNotice( &st_Msg );
				break;
			default:
				FBPG_DebugPrint( 3, "FBPC_FallbackManager::ReceiveEvent  case error! [%d]\n", st_Msg.ss_type );
				break;
			}
		} else {
			FBPG_DebugPrint( 3, "FBPC_FallbackManager::ReceiveEvent  FBPG_mqRecive error! [%d]\n", errno );
		}
	}

	// 登録されたアプリを削除する.
	if ( AppID ) {
		LAPF_NET_UnregistApplication( AppID );
	}
	FBP_pthread_cleanup_pop(1);
	return FBPD_OK;
}
