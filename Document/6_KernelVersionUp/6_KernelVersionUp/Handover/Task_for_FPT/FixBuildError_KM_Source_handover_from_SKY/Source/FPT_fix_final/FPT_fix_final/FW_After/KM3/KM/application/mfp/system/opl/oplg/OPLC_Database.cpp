////////////////////////////////////////////////////////////////////////////////////////////////////.
//.
//    \file    ファイル名：OPLC_Database.cpp.
//.
//    \brief    概要：操作ログ データベースクラス.
//.
////////////////////////////////////////////////////////////////////////////////////////////////////.

//--------------------------------------------------------------------------------------------------.
//	インクルードファイル.
//--------------------------------------------------------------------------------------------------.
#include	<stdio.h>
#include	<string.h>
#include "OPLC_Database.h"
#include "system/sys/sysg/sys_common.h"
#include <stdarg.h>
#include "system/fil/FILC_FlatFileManagerIf.h"
#include "../../../../divlib/Common/panel/pddtext/pddtext_CLCD/psstr_CLCD.h"
#include "../../../../divlib/Common/panel/pddtext/pddtext_CLCD/pstrs_CLCD.h"
#include "../../../../divlib/Common/panel/pddtext/pddtext_CLCD/pstr_CLCD.h"
#include	<lib/LIBC_CharCodeConvert.h>
#include "OPLC_OPL.h"

extern "C" {
extern void DBGF_PrintNoOperationPath(char* psc_FileName, int32_t sl_LineNo); 
}

// 2019.10.29 OP_BTS-13736 S
//static void debugLog_SendOpeLogOut( LIBC_List<OPLC_SendOpeLogOut>* pc_SendOpeLogOutList, short ss_Count );
static void debugLog_SendOpeLogOut( std::list<OPLC_SendOpeLogOut>& pc_SendOpeLogOutList, short ss_Count );
// 2019.10.29 OP_BTS-13736 E
static short _strncpyClearCtrCode( uchar* puc_Dst, uchar* puc_Src, short ss_Size );

extern TYPE_CharSet getCharSetForCAPI();
extern const uchar* getTextPartsPointer( const ushort us_TextID );

//--------------------------------------------------------------------------------------------------.
//	debug.
//--------------------------------------------------------------------------------------------------.
//--------------------------------------------------------------------------------------------------.
//	debug.
//--------------------------------------------------------------------------------------------------.

//--------------------------------------------------------------------------------------------------.
//	OPLC_Databaseクラスの変数定義.
//--------------------------------------------------------------------------------------------------.
// データベースアドレス
static SYSC_Database* _pPushKeyLogDB;						// キー押下ログDB.
static SYSC_Database* _pPushKeyNameLogDB;					// キー押下ログ（キー名称）ログDB.
static SYSC_Database* _pAdrInfoLogDB;						// 宛先情報ログDB.
static SYSC_Database* _pMngOfSendOpeLogDB;					// 送信操作ログ管理DB.
static SYSC_Database* _pSendOpeLogDB[OPLE_SendOpeLogDBMax];	// 送信操作ログDB.

static OPLE_SetUpDB b_KeyPushLogSetUp;						// データベースセットアップ完了フラグ.
static OPLE_SetUpDB b_PushKeyNameLogSetUp;					// データベースセットアップ完了フラグ.
static OPLE_SetUpDB b_AdrInfoLogSetUp;						// データベースセットアップ完了フラグ.
static OPLE_SetUpDB b_MngOfSendOpeLogSetUp;					// データベースセットアップ完了フラグ.
static OPLE_SetUpDB b_SendOpeLogSetUp;						// データベースセットアップ完了フラグ.

SEM_ID OPLC_Database::semID = 0;
uchar  OPLC_Database::uc_SendOpeLogDBWriteIndex  = 0;		// 送信操作ログDBの現在書き込んできるDBのインデックス.
int32_t   OPLC_Database::us_SendOpeLogDBWriteRecord = 0;		// 送信操作ログDBの現在書き込んできるDBのレコード数.
SEM_ID OPLC_Database::OpeLogIdSemId = 0;						// 送信操作ログDBインデックス用セマフォID.

// 2019.06.25 SSD_1TB_SP-53 S
extern int32_t OPLD_SendOpeLogRecMax;		// 送信操作ログDBレコードMAX.
int32_t OPLE_SendOpeLogRecordMax = 0;
// 2019.06.25 SSD_1TB_SP-53 E

// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 Start.
char dbgOplCheckPoint=0;
int dbgOplErr=0;
// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 End.

//--------------------------------------------------------------------------------------------------.
//	デバッグ用処理定義.
//--------------------------------------------------------------------------------------------------.
extern Bool b_OplDebugLog;
extern Bool b_OplSaveLogNG;
extern "C" {
extern void OPLDebug_LogPrint(const char* fmt, uintptr_t ul_para1, uintptr_t ul_para2, uintptr_t ul_para3, uintptr_t ul_para4, uintptr_t ul_para5, uintptr_t ul_para6, bool b_on = False);
extern void OPLDebug_Wait(const char* str);
extern OPLE_ResultStatus OPLDebug_SaveLogNG(OPLE_ResultStatus result);
}

//--------------------------------------------------------------------------------------------------.
//	OPLC_Databaseクラス実装.
//--------------------------------------------------------------------------------------------------.
//関数----------------------------------------------------------------------------------------------.
//概要    ：コンストラクタ.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLC_Database::OPLC_Database()
{
	return;
}
OPLC_Database::~OPLC_Database()
{
}
//関数----------------------------------------------------------------------------------------------.
//概要    ：インスタンス取得.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLC_Database* OPLC_Database::newInstance()
{
	static OPLC_Database s_database;

	return &s_database;
}

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//■■■　　初期処理
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

//関数----------------------------------------------------------------------------------------------.
//概要    ：初期設定.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
void OPLC_Database::initClass()
{
	_pPushKeyLogDB = SYSC_Database::getInstance();
	_pPushKeyNameLogDB = SYSC_Database::getInstance();
	_pAdrInfoLogDB = SYSC_Database::getInstance();
	_pMngOfSendOpeLogDB = SYSC_Database::getInstance();
	for (int i = 0; i < OPLE_SendOpeLogDBMax; i++) {			// 送信操作ログDBのインスタンス取得.
		_pSendOpeLogDB[i] = SYSC_Database::getInstance();
	}
	uc_SendOpeLogDBWriteIndex = _getSendOpeLogDBWriteIndex();	// 送信操作ログDB書き込みインデックス取得.
	us_SendOpeLogDBWriteRecord = 0;

	// セットアップフラグの初期化.
	b_KeyPushLogSetUp		= OPLE_SetUpDBNG;
	b_PushKeyNameLogSetUp	= OPLE_SetUpDBNG;
	b_AdrInfoLogSetUp		= OPLE_SetUpDBNG;
	b_MngOfSendOpeLogSetUp	= OPLE_SetUpDBNG;
	b_SendOpeLogSetUp		= OPLE_SetUpDBNG;

	//セマフォの作成
	if( semID == 0 ) {
		semID = SYSF_semBCreate(SEM_Q_FIFO, SEM_FULL);
	}

	//セマフォの作成.
	if (0 == OpeLogIdSemId)
	{
		OpeLogIdSemId = SYSF_semBCreate(SEM_Q_FIFO, SEM_FULL);
	}
// 2019.06.25 SSD_1TB_SP-53 S
	// OPLC_OPL::initClass()でsetSendOpeLogRecMax()はコール済み.
	OPLE_SendOpeLogRecordMax = OPLD_SendOpeLogRecMax/OPLE_SendOpeLogDBMax;
// 2019.06.25 SSD_1TB_SP-53 E

	return ;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：ＤＢセットアップ処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_setupLogNVDB()
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;

	if ( ( b_KeyPushLogSetUp		== OPLE_SetUpDBNG ) ||
		 ( b_PushKeyNameLogSetUp	== OPLE_SetUpDBNG ) ||
		 ( b_AdrInfoLogSetUp		== OPLE_SetUpDBNG ) ||
		 ( b_MngOfSendOpeLogSetUp	== OPLE_SetUpDBNG ) )
	{
		// DBをセットアップする.
		e_result = _setupKeyPushLogDB();
		if ( e_result == OPLE_RsltStatError ) {
			return e_result;
		}
		b_KeyPushLogSetUp = OPLE_SetUpDBOK;			// セットアップ済みとする.

		// DBをセットアップする.
		e_result = _setupPushKeyNameLogDB();
		if ( e_result == OPLE_RsltStatError ) {
			return e_result;
		}
		b_PushKeyNameLogSetUp = OPLE_SetUpDBOK;	// セットアップ済みとする.

		// DBをセットアップする.
		e_result = _setupAdrInfoLogDB();
		if ( e_result == OPLE_RsltStatError ) {
			return e_result;
		}
		b_AdrInfoLogSetUp = OPLE_SetUpDBOK;			// セットアップ済みとする.

		// DBをセットアップする.
		e_result = _setupMngOfSendOpeLogDB();
		if ( e_result == OPLE_RsltStatError ) {
			return e_result;
		}
		b_MngOfSendOpeLogSetUp = OPLE_SetUpDBOK;	// セットアップ済みとする.
	}
	return e_result;
}


//関数----------------------------------------------------------------------------------------------.
//概要    ：ＤＢセットアップ処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_setupLogHDDDB()
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;

	if ( b_SendOpeLogSetUp == OPLE_SetUpDBNG )
	{
		// DBをセットアップする.
		e_result = _setupSendOpeLogDB();
		if ( e_result != OPLE_RsltStatComplete ) {
			return e_result;
		}
		b_SendOpeLogSetUp = OPLE_SetUpDBOK;	// セットアップ済みとする.
	}
	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：DBをセットアップする.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_setupKeyPushLogDB()
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;							// 本処理結果.

	if ( b_KeyPushLogSetUp == OPLE_SetUpDBOK ) {
		return e_result;
	}

	SYSS_Config s_config = {};
	memset( &s_config, 0x00, sizeof( s_config ) );

	sprintf(s_config.FilePath, "%s/KeyPush", RAM_DISK_DEV_NAME);
	s_config.UseFile					= True;
	s_config.DbNum						= 0;
	s_config.NumOfCache					= 0;
	s_config.RecordBytes				= sizeof( OPLS_KeyPushLog );
	s_config.MaxKey						= OPLD_KeyPushLogRecMax;
	s_config.KeySorting					= True;
	s_config.ExclusiveKey				= FALSE;
	s_config.NvRamConf.UseNvRam			= False;

	// DBをセットアップする.
	e_result = _setupDB( _pPushKeyLogDB, &s_config );
	if ( OPLE_RsltStatComplete == e_result ) {
		b_KeyPushLogSetUp = OPLE_SetUpDBOK;			// セットアップ済みとする.
	}

	OPLDEBUG_LOGPRINT(":%s\n", reinterpret_cast<uintptr_t>(s_config.FilePath), 0, 0, 0, 0, 0);

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：DBをセットアップする.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_setupPushKeyNameLogDB()
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;							// 本処理結果.

	if ( b_PushKeyNameLogSetUp == OPLE_SetUpDBOK ) {
		return e_result;
	}

	SYSS_Config s_config = {};
	memset( &s_config, 0x00, sizeof( s_config ) );


	sprintf(s_config.FilePath, "%s/PushKeyName", RAM_DISK_DEV_NAME);
	s_config.UseFile					= True;
	s_config.DbNum						= 0;
	s_config.NumOfCache					= 0;
	s_config.RecordBytes				= sizeof( OPLS_PushKeyNameLog );
	s_config.MaxKey						= OPLD_PushKeyNameLogRecMax;
	s_config.KeySorting					= True;
	s_config.ExclusiveKey				= FALSE;
	s_config.NvRamConf.UseNvRam			= False;

	// DBをセットアップする.
	e_result = _setupDB( _pPushKeyNameLogDB, &s_config );
	if ( OPLE_RsltStatComplete == e_result ) {
		b_PushKeyNameLogSetUp = OPLE_SetUpDBOK;			// セットアップ済みとする.
	}

	OPLDEBUG_LOGPRINT(":%s\n", reinterpret_cast<uintptr_t>(s_config.FilePath), 0, 0, 0, 0, 0);

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：DBをセットアップする.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_setupAdrInfoLogDB()
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;							// 本処理結果.

	if ( b_AdrInfoLogSetUp == OPLE_SetUpDBOK ) {
		return e_result;
	}

	SYSS_Config s_config = {};
	memset( &s_config, 0x00, sizeof( s_config ) );


	sprintf(s_config.FilePath, "%s/AdrInfo", RAM_DISK_DEV_NAME);
	s_config.UseFile					= True;
	s_config.DbNum						= 0;
	s_config.NumOfCache					= 0;
	s_config.RecordBytes				= sizeof( OPLS_AdrInfoLog );
	s_config.MaxKey						= OPLD_AdrInfoLogRecMax;
	s_config.KeySorting					= True;
	s_config.ExclusiveKey				= FALSE;
	s_config.NvRamConf.UseNvRam			= True;
	s_config.NvRamConf.PackedHeader		= True;

	s_config.NvRamConf.UseNvRam			= False;

	// DBをセットアップする.
	e_result = _setupDB( _pAdrInfoLogDB, &s_config );
	if ( OPLE_RsltStatComplete == e_result ) {
		b_AdrInfoLogSetUp = OPLE_SetUpDBOK;			// セットアップ済みとする.
	}

	OPLDEBUG_LOGPRINT(":%s\n", reinterpret_cast<uintptr_t>(s_config.FilePath), 0, 0, 0, 0, 0);

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：DBをセットアップする.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_setupMngOfSendOpeLogDB()
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;							// 本処理結果.

	if ( b_MngOfSendOpeLogSetUp	== OPLE_SetUpDBOK ) {
		return OPLE_RsltStatComplete;
	}
	
	SYSS_Config s_config = {};
	memset( &s_config, 0x00, sizeof( s_config ) );

	sprintf(s_config.FilePath, "%s/MngOfSendOpe", RAM_DISK_DEV_NAME);
	s_config.UseFile					= True;
	s_config.DbNum						= 0;
	s_config.NumOfCache					= 0;
	s_config.RecordBytes				= sizeof( OPLS_MngOfSendOpeLog );
	s_config.MaxKey						= OPLD_MngOfSendOpeLogRecMax;
	s_config.KeySorting					= True;
	s_config.ExclusiveKey				= TRUE;
	s_config.NvRamConf.UseNvRam			= False;

	// DBをセットアップする.
	e_result = _setupDB( _pMngOfSendOpeLogDB, &s_config );
	if ( OPLE_RsltStatComplete == e_result ) {
		b_MngOfSendOpeLogSetUp = OPLE_SetUpDBOK;			// セットアップ済みとする.
	}

	OPLDEBUG_LOGPRINT(":%s\n", reinterpret_cast<uintptr_t>(s_config.FilePath), 0, 0, 0, 0, 0);

	return e_result;
}


//関数----------------------------------------------------------------------------------------------.
//概要    ：DBをセットアップする.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_setupSendOpeLogDB()
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;							// 本処理結果.

	if ( b_SendOpeLogSetUp	== OPLE_SetUpDBOK ) {
		return OPLE_RsltStatComplete;
	}
	
	SYSS_Config s_config = {};
	memset( &s_config, 0x00, sizeof( s_config ) );

	const char* pfilePath = FILC_BaseDirIf::newInstance( FILE_DirTypeSendOperationLog )->getPath();

	for (int i = 0; i < OPLE_SendOpeLogDBMax; i++) {
		//フルパス指定用
		sprintf(s_config.FilePath, "%s/Opl%03d", pfilePath, i);
		OPLDEBUG_LOGPRINT("DB%03d :%s\n", i, reinterpret_cast<uintptr_t>(s_config.FilePath), 0, 0, 0, 0);

		s_config.UseFile					= TRUE;
		s_config.DbNum						= 0x00;
		s_config.NumOfCache					= 0;
		s_config.RecordBytes				= sizeof( OPLS_SendOpeLog );
		s_config.MaxKey						= OPLD_SendOpeLogRecMax;
		s_config.KeySorting					= True;
		s_config.ExclusiveKey				= FALSE;
		s_config.NvRamConf.UseNvRam			= False;

		// DBをセットアップする.
		e_result = _setupDB( _pSendOpeLogDB[i], &s_config );
		if ( OPLE_RsltStatComplete != e_result ) {
			b_SendOpeLogSetUp = OPLE_SetUpDBNG;				// セットアップ未とする.
			return e_result;
		}
	}
	b_SendOpeLogSetUp = OPLE_SetUpDBOK;						// セットアップ済みとする.

	if (OPLE_SendOpeLogDBIndexDelete == uc_SendOpeLogDBWriteIndex) {
		// 送信操作ログ全削除中に電源OFFされたので、全削除する.
		_deleteSendOpeLogDB();
	}

	int32_t sl_cnt = 0;
	_getAllRecordCountAtDB(_pSendOpeLogDB[uc_SendOpeLogDBWriteIndex], &sl_cnt);
	if (OPLE_SendOpeLogRecordMax <= sl_cnt) {
		// 送信操作ログDB書き込みインデックスを書き込む前に、電源OFFされたので、インデックスを更新する.
		_updateSendOpeLogDBWriteIndex();
	}

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：送信操作ログ管理DB復旧（ステータス更新）.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_initMngOfSendOpeLogNextStatus( OPLS_MngOfSendOpeLog* p_Data )
{
	if( NULL == p_Data ) {
		return OPLE_RsltStatError;
	}

	OPLS_MngOfSendOpeLog s_mngData = {};

	s_mngData.ul_SendOpeLogID			= p_Data->ul_SendOpeLogID;				// 操作ID.
	s_mngData.e_MngOfSendOpeLogStatus	= p_Data->e_MngOfSendOpeLogStatus;		// 送信操作ログ管理ステータスコード.
	s_mngData.us_KeyPushLogRecCount		= p_Data->us_KeyPushLogRecCount;		// キー押下ログレコード数.
	s_mngData.us_AdrInfoLogRecCount		= p_Data->us_AdrInfoLogRecCount;		// 宛先情報ログレコード数.

	switch( p_Data->e_MngOfSendOpeLogStatus ) {
	case	OPLE_MngStatSaveKeyPush :												// キー押下ログ保存中.
	case	OPLE_MngStatSaveAdrInfo :												// 宛先情報ログ保存中（宛先情報ログがある場合のキー押下ログ保存完了）.
	case	OPLE_MngStatSaveSendOpeLog :											// 送信操作ログ保存中.
	case	OPLE_MngStatErrDelKeyAndAdr :											// 宛先情報ログ保存エラー時のキー押下ログ削除中.
	case	OPLE_MngStatErrDelKeyPush :												// キー押下ログ保存エラー時のキー押下ログ削除中.
	case	OPLE_MngStatDelKeyAndAdr :												// 送信操作ログ保存完了によるキー押下ログ削除中（宛先情報あり）.
	case	OPLE_MngStatDelKeyPush :												// 送信操作ログ保存完了によるキー押下ログ削除中（宛先情報なし）.
	case	OPLE_MngStatDelKeyPushEnd :												// 送信操作ログ保存完了によるキー押下ログ削除完了（宛先情報あり）.
	case	OPLE_MngStatErrDelAdrInfo :												// 宛先情報ログ保存エラー時の宛先情報ログ削除中.
	case	OPLE_MngStatDelAdrInfo :												// 送信操作ログ保存完了による宛先情報ログ削除中.
	case	OPLE_MngStatDelSendOpeLog :												// 送信操作ログ削除中.
		s_mngData.e_MngOfSendOpeLogStatus = _getMngOfSendOpeLogNextStatus( p_Data->e_MngOfSendOpeLogStatus, False, True );
		break;
	default :	return OPLE_RsltStatComplete;	// 上記以外は何もしない.
	}

	OPLE_ResultStatus e_result = _updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：送信操作ログ管理DB復旧（レコード復旧）.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_initMngOfSendOpeLogRecord( OPLS_MngOfSendOpeLog* p_Data )
{
	if( NULL == p_Data ) {
		return OPLE_RsltStatError;
	}

	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;

	switch( p_Data->e_MngOfSendOpeLogStatus ) {
	case	OPLE_MngStatIDSet :						// 操作ID採番.
	case	OPLE_MngStatErrDelKeyPushEnd :			// 宛先情報ログ保存エラー時のキー押下ログ削除完了.
	case	OPLE_MngStatErrDelAdrInfoEnd :			// 宛先情報ログ保存エラー時の宛先情報ログ削除完了.
	case	OPLE_MngStatDelAdrInfoEnd :				// 送信操作ログ保存完了による宛先情報ログ削除完了.
		// 管理レコード削除.
		e_result = _deleteRecordByKeyAtDB(_pMngOfSendOpeLogDB, p_Data->ul_SendOpeLogID);
		break;
	case	OPLE_MngStatSaveKeyPush :				// キー押下ログ保存中.
	case	OPLE_MngStatSaveAdrInfo :				// 宛先情報ログ保存中（宛先情報ログがある場合のキー押下ログ保存完了）.
	case	OPLE_MngStatSaveSendOpeLog :			// 送信操作ログ保存中.
	case	OPLE_MngStatErrDelKeyAndAdr :			// 宛先情報ログ保存エラー時のキー押下ログ削除中.
	case	OPLE_MngStatErrDelKeyPush :				// キー押下ログ保存エラー時のキー押下ログ削除中.
	case	OPLE_MngStatDelKeyAndAdr :				// 送信操作ログ保存完了によるキー押下ログ削除中（宛先情報あり）.
	case	OPLE_MngStatDelKeyPush :				// 送信操作ログ保存完了によるキー押下ログ削除中（宛先情報なし）.
	case	OPLE_MngStatDelKeyPushEnd :				// 送信操作ログ保存完了によるキー押下ログ削除完了（宛先情報あり）.
	case	OPLE_MngStatErrDelAdrInfo :				// 宛先情報ログ保存エラー時の宛先情報ログ削除中.
	case	OPLE_MngStatDelAdrInfo :				// 送信操作ログ保存完了による宛先情報ログ削除中.
	case	OPLE_MngStatDelSendOpeLog :				// 送信操作ログ削除中.
		// ステータス更新.
		
		e_result = _initMngOfSendOpeLogNextStatus(p_Data);
		break;
	default :
		// 何もしない.
		break;
	}
	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：送信操作ログ管理DB初期化処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_initMngOfSendOpeLogDB()
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	int32_t sl_cnt = 0;
	LIST c_list = {};
	SYSS_RecordListHeader* p_recordList = NULL;
	SYSS_RecordListHeader* p_tmp = NULL;
	OPLS_MngOfSendOpeLog* p_data = NULL;

	SYSF_lstInit( &c_list );													// レコード取得リストの初期化.
	
	_getAllRecordCountAtDB( _pMngOfSendOpeLogDB, &sl_cnt );						// 送信操作ログ管理DBレコード件数取得.

	if(sl_cnt > 0) {															// 送信操作ログ管理DBにレコードがある場合.

		_getListRecordsAtDB( _pMngOfSendOpeLogDB, &c_list, OPLD_MngOfSendOpeLogRecMax, 0 );		// 送信操作ログ管理DBから全レコードを取得する.

		p_recordList = reinterpret_cast<SYSS_RecordListHeader*>(SYSF_lstFirst( &c_list ));		// リストの1番目のデータ取得.
		while( NULL!=p_recordList ){													// リストからでーたがなくなるまで処理を繰り返す.
			p_tmp = p_recordList;												// リストデータのポインタをヘッダデータポインタに移す.
			p_data = reinterpret_cast<OPLS_MngOfSendOpeLog*>(++p_tmp);							// ヘッダサイズ分ポインタを移動（データ部の先頭にポインタを移す）.
			if ( OPLE_RsltStatError == _initMngOfSendOpeLogRecord( p_data ) ) {	// 送信操作ログ管理DBレコード復旧処理.
				e_result = OPLE_RsltStatError;									// 異常の場合は処理終了.
				break;
			}
			p_recordList = reinterpret_cast<SYSS_RecordListHeader *>(SYSF_lstNext( &p_recordList->Node ));	// 次のリストデータを取得する.
		}
		_deleteListRecordsAtDB( _pMngOfSendOpeLogDB, &c_list );					// 取得したリストを開放する.
	}
	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：キー押下ログDB初期化処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_initPushKeyLogDB()
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	int32_t sl_cnt = 0;
	LIST c_list = {};
	SYSS_RecordListHeader* p_recordList = NULL;
	SYSS_RecordListHeader* p_tmp = NULL;
	OPLS_KeyPushLog* p_data = NULL;

	SYSF_lstInit( &c_list );													// レコード取得リストの初期化.

	_getAllRecordCountAtDB( _pPushKeyLogDB, &sl_cnt );							// キー押下ログDBレコード件数取得.

	if(sl_cnt > 0) {															// キー押下ログDBにレコードがある場合.

		_getListRecordsAtDB( _pPushKeyLogDB, &c_list, OPLD_KeyPushLogRecMax, 0 );		// キー押下ログDBから全レコードを取得する.

	    p_recordList = reinterpret_cast<SYSS_RecordListHeader*>(SYSF_lstFirst( &c_list ));		// リストの1番目のデータ取得.
		while( NULL!=p_recordList ) {													// リストからでーたがなくなるまで処理を繰り返す.
			p_tmp = p_recordList;												// リストデータのポインタをヘッダデータポインタに移す.
			p_data = reinterpret_cast<OPLS_KeyPushLog*>(++p_tmp);									// ヘッダサイズ分ポインタを移動（データ部の先頭にポインタを移す）.

			e_result = _existRecordByKeyAtDB( _pMngOfSendOpeLogDB, p_data->ul_SendOpeLogID );		// 取得したキー押下ログDBレコードのキーと同じキーのレコードが送信操作ログ管理DBに存在するかチェックする.

			if( e_result != OPLE_RsltStatComplete ) {							// 送信操作ログ管理DBにレコードがない場合、不要データの為、各Dbのレコードを削除する.
				if ( OPLE_RsltStatError == _deleteRecordByKeyAtDB( _pAdrInfoLogDB, p_data->ul_SendOpeLogID ) ) {		// 宛先情報ログDBのレコードを削除する.
					e_result = OPLE_RsltStatError;									// 異常の場合は処理終了.
					break;
				}
				if ( OPLE_RsltStatError == _deleteRecordByKeyAtDB( _pPushKeyNameLogDB, p_data->ul_SendOpeLogID ) ) {	// キー押下ログ（キー名称）DBのレコードを削除する.
					e_result = OPLE_RsltStatError;									// 異常の場合は処理終了.
					break;
				}
				// キー押下ログ（キー名称）DBのレコード管理はキー押下ログDBで行っている為、キー押下ログ（キー名称）DBレコードはキー押下ログDBレコード前に削除すること.
				if ( OPLE_RsltStatError == _deleteRecordByKeyAtDB( _pPushKeyLogDB, p_data->ul_SendOpeLogID ) ) {		// キー押下ログDBのレコードを削除する.
					e_result = OPLE_RsltStatError;									// 異常の場合は処理終了.
					break;
				}
				e_result = OPLE_RsltStatComplete;
			}
			p_recordList = reinterpret_cast<SYSS_RecordListHeader *>(SYSF_lstNext( &p_recordList->Node ));	// 次のリストデータを取得する.
		}
		_deleteListRecordsAtDB( _pPushKeyLogDB, &c_list );						// 取得したリストを開放する.
	}
	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：宛先情報ログDB初期化処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_initAdrInfoLogDB()
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	int32_t sl_cnt = 0;
	LIST c_list = {};
	SYSS_RecordListHeader* p_recordList = NULL;
	SYSS_RecordListHeader* p_tmp = NULL;
	OPLS_AdrInfoLog* p_data = NULL;

	SYSF_lstInit( &c_list );													// レコード取得リストの初期化.

	_getAllRecordCountAtDB( _pAdrInfoLogDB, &sl_cnt );							// 宛先情報ログDBレコード件数取得.

	if(sl_cnt > 0) {															// 宛先情報ログDBにレコードがある場合.

		_getListRecordsAtDB( _pAdrInfoLogDB, &c_list, OPLD_AdrInfoLogRecMax, 0 );		// 宛先情報ログDBから全レコードを取得する.

	    p_recordList = reinterpret_cast<SYSS_RecordListHeader*>(SYSF_lstFirst( &c_list ));		// リストの1番目のデータ取得.
		while( NULL!=p_recordList ) {													// リストからでーたがなくなるまで処理を繰り返す.
			p_tmp = p_recordList;												// リストデータのポインタをヘッダデータポインタに移す.
			p_data = reinterpret_cast<OPLS_AdrInfoLog*>(++p_tmp);									// ヘッダサイズ分ポインタを移動（データ部の先頭にポインタを移す）.

			e_result = _existRecordByKeyAtDB( _pMngOfSendOpeLogDB, p_data->ul_SendOpeLogID );		// 取得した宛先情報ログDBレコードのキーと同じキーのレコードが送信操作ログ管理DBに存在するかチェックする.

			if( e_result != OPLE_RsltStatComplete ) {							// 送信操作ログ管理DBにレコードがない場合、不要データの為、各Dbのレコードを削除する.
				if ( OPLE_RsltStatError == _deleteRecordByKeyAtDB( _pAdrInfoLogDB, p_recordList->Key ) ) {
					e_result = OPLE_RsltStatError;
					break;
				}
				e_result = OPLE_RsltStatComplete;
			}
			p_recordList = reinterpret_cast<SYSS_RecordListHeader *>(SYSF_lstNext( &p_recordList->Node ));
		}
		_deleteListRecordsAtDB( _pAdrInfoLogDB, &c_list );
	}
	return e_result;
}

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■.
//■■■　　削除処理.
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■.
//関数----------------------------------------------------------------------------------------------.
//概要    ：不要レコード削除処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
void OPLC_Database::_deleteLogDB()
{
	int32_t sl_cnt = 0;
	LIST c_List = {};
	SYSS_RecordListHeader* p_RecordList = NULL;
	SYSS_RecordListHeader* p_tmp = NULL;
	OPLS_MngOfSendOpeLog* p_data = NULL;
	
	SYSF_lstInit(&c_List);
	_getAllRecordCountAtDB( _pMngOfSendOpeLogDB, &sl_cnt );
	
	if(sl_cnt > 0) {
		_getListRecordsAtDB( _pMngOfSendOpeLogDB, &c_List, OPLD_MngOfSendOpeLogRecMax, 0 );	// 送信操作ログ管理DB全レコードをリストで取得.
		p_RecordList = reinterpret_cast<SYSS_RecordListHeader*>(SYSF_lstFirst( &c_List ));					// リストの先頭データポインタを取得.
		
		while( NULL!=p_RecordList ){
			p_tmp = p_RecordList;
			p_data = reinterpret_cast<OPLS_MngOfSendOpeLog*>(++p_tmp);
			if( OPLE_RsltStatComplete == _deleteSendOpeLogIDRecord(p_data) ) {
				break;	// ループを抜ける.
			}
			p_RecordList = reinterpret_cast<SYSS_RecordListHeader *>(SYSF_lstNext( &p_RecordList->Node ));	// 次のリストデータポインタを取得する.
		}
		_deleteListRecordsAtDB(_pMngOfSendOpeLogDB,&c_List);	// 使用したリストを開放する.
	}
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：不要レコード削除処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_deleteSendOpeLogIDRecord( OPLS_MngOfSendOpeLog* p_data )
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;

	switch( p_data->e_MngOfSendOpeLogStatus ) {
		case	OPLE_MngStatSaveKeyPushErr :			// キー押下ログ保存エラー.
		case	OPLE_MngStatErrDelKeyPushStart :		// キー押下ログ保存エラー時のキー押下ログ削除.
		case	OPLE_MngStatDelKeyPushStart :			// 送信操作ログ保存完了によるキー押下ログ削除.
			e_result = _deleteSendOpeLogIDRecord_KeyDB( p_data );
			break;
			
		case	OPLE_MngStatSaveAdrInfoErr :			// 宛先情報ログ保存エラー.
		case	OPLE_MngStatDelKeyAndAdrStart :			// 送信操作ログ保存完了によるキー押下ログ、宛先情報ログ削除.
		case	OPLE_MngStatErrDelKeyAndAdrStart :		// 宛先情報ログ保存エラー時のキー押下ログ、宛先情報ログ削除.
			e_result = _deleteSendOpeLogIDRecord_KeyDBAndAdrDB( p_data );
			break;
			
		case	OPLE_MngStatErrDelAdrInfoStart :		// 宛先情報ログ保存エラー時の宛先情報ログ削除.
		case	OPLE_MngStatDelAdrInfoStart :			// 送信操作ログ保存完了による宛先情報ログ削除.
			e_result = _deleteSendOpeLogIDRecord_AdrDB( p_data );
			break;
			
		case	OPLE_MngStatSaveSendOpeLogErr :			// 送信操作ログ保存エラー.
		case	OPLE_MngStatDelSendOpeLogStart :		// 送信操作ログ削除.
		case	OPLE_MngStatSaveSendOpeLog:				// 送信操作ログ保存中.
			e_result = _deleteSendOpeLogIDRecord_LogDB( p_data );
			if (OPLE_RsltStatComplete == e_result) {
				if( 0 < p_data->us_AdrInfoLogRecCount ) {
					e_result = _deleteSendOpeLogIDRecord_KeyDBAndAdrDB( p_data );
				} else {
					e_result = _deleteSendOpeLogIDRecord_KeyDB( p_data );
				}
			}
			break;
			
		case	OPLE_MngStatSaveSendOpeLogEnd :			// 送信操作ログ保存完了.
			if( 0 < p_data->us_AdrInfoLogRecCount ) {
				e_result = _deleteSendOpeLogIDRecord_KeyDBAndAdrDB( p_data );
			} else {
				e_result = _deleteSendOpeLogIDRecord_KeyDB( p_data );
			}
			break;
			
		case	OPLE_MngStatErrDelAdrInfoEnd :			// 宛先情報ログ保存エラー時の宛先情報ログ削除完了.
		case	OPLE_MngStatDelAdrInfoEnd :				// 送信操作ログ保存完了による宛先情報ログ削除完了.
		case	OPLE_MngStatAllComplete :				// 送信操作ログ管理DB削除（送信操作ログ処理完了）.
			e_result = _deleteSendOpeLogIDRecord_MngDB( p_data );
			break;
		default :	break;
	}
	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：不要レコード削除処理（キー押下ログDB）.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_deleteSendOpeLogIDRecord_KeyDB( OPLS_MngOfSendOpeLog* p_data )
{
	OPLDEBUG_LOGPRINT(" ID=%d\n", p_data->ul_SendOpeLogID,0,0,0,0,0);

	OPLS_MngOfSendOpeLog s_mngData = {};
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;

	memset( &s_mngData, 0x00, sizeof(OPLS_MngOfSendOpeLog) );
	s_mngData.ul_SendOpeLogID			= p_data->ul_SendOpeLogID;				// 操作ID.
	s_mngData.e_MngOfSendOpeLogStatus	= OPLE_MngStatErrDelKeyPush;			// 送信操作ログ管理ステータスコード.
	s_mngData.us_KeyPushLogRecCount		= p_data->us_KeyPushLogRecCount;		// キー押下ログレコード数.
	s_mngData.us_AdrInfoLogRecCount		= p_data->us_AdrInfoLogRecCount;		// 宛先情報ログレコード数.

	// 送信操作ログ管理DBを更新する.
	e_result = _updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );
	if(OPLE_RsltStatComplete != e_result){
		return e_result;
	}
	

	if (False != b_OplDebugLog) {
		int32_t sl_cnt = 0;
		_getAllRecordCountAtDB( _pPushKeyNameLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]_deleteSendOpeLogIDRecord_KeyDB START %d\n", sl_cnt,0,0,0,0,0);
	}

	// 該当のキーを持つキー押下ログ（キー名称）DBを削除する.
	e_result = _deleteRecordByKeyAtDB( _pPushKeyNameLogDB, s_mngData.ul_SendOpeLogID );
	if( OPLE_RsltStatComplete != e_result ) {
		s_mngData.e_MngOfSendOpeLogStatus = _getMngOfSendOpeLogNextStatus( s_mngData.e_MngOfSendOpeLogStatus, False, True );
		_updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );
		return e_result;
	}
	
	if (False != b_OplDebugLog) {
		int32_t sl_cnt = 0;
		_getAllRecordCountAtDB( _pPushKeyNameLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(キー名称DB)終了 %d\n", sl_cnt,0,0,0,0,0);

		_getAllRecordCountAtDB( _pPushKeyLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(キーDB)開始 %d\n", sl_cnt,0,0,0,0,0);
	}

	// 該当のキーを持つキー押下ログDBを削除する.
	e_result = _deleteRecordByKeyAtDB( _pPushKeyLogDB, s_mngData.ul_SendOpeLogID );
	if( OPLE_RsltStatComplete != e_result  ){
		s_mngData.e_MngOfSendOpeLogStatus = _getMngOfSendOpeLogNextStatus( s_mngData.e_MngOfSendOpeLogStatus, False, True );
		_updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );
		return e_result;
	}
	
	if (False != b_OplDebugLog) {
		int32_t sl_cnt = 0;
		_getAllRecordCountAtDB( _pPushKeyLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(キーDB)終了 %d\n", sl_cnt,0,0,0,0,0);

		_getAllRecordCountAtDB( _pMngOfSendOpeLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(管理DB)開始 %d\n", sl_cnt,0,0,0,0,0);
	}

	// 該当のキーを持つ送信操作ログ管理DBを削除する.
	e_result = _deleteRecordByKeyAtDB( _pMngOfSendOpeLogDB, s_mngData.ul_SendOpeLogID );

	if (False != b_OplDebugLog) {
		int32_t sl_cnt = 0;
		_getAllRecordCountAtDB( _pMngOfSendOpeLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(管理DB)終了 %d\n", sl_cnt,0,0,0,0,0);
	}

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：不要レコード削除処理（キー押下ログDB＋宛先情報ログDB）.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_deleteSendOpeLogIDRecord_KeyDBAndAdrDB( OPLS_MngOfSendOpeLog* p_data )
{
	OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(キー押下ログDB＋宛先情報ログDB) ID=%d\n", p_data->ul_SendOpeLogID,0,0,0,0,0);

	OPLS_MngOfSendOpeLog s_mngData = {};
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;

	memset( &s_mngData, 0x00, sizeof(OPLS_MngOfSendOpeLog) );
	s_mngData.ul_SendOpeLogID			= p_data->ul_SendOpeLogID;				// 操作ID.
	s_mngData.e_MngOfSendOpeLogStatus	= p_data->e_MngOfSendOpeLogStatus;		// 送信操作ログ管理ステータスコード.
	s_mngData.us_KeyPushLogRecCount		= p_data->us_KeyPushLogRecCount;		// キー押下ログレコード数.
	s_mngData.us_AdrInfoLogRecCount		= p_data->us_AdrInfoLogRecCount;		// 宛先情報ログレコード数.

	// 送信操作ログ管理次ステータス取得処理で次のステータスを取得し、送信操作ログ管理DB構造体のステータスにセットする.
	s_mngData.e_MngOfSendOpeLogStatus = static_cast<OPLE_MngOfSendOpeLogStatus>(OPLE_MngStatErrDelKeyAndAdr);

	// 送信操作ログ管理DBを更新する.
	e_result = _updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );
	if(OPLE_RsltStatComplete != e_result){	// 更新失敗.
		return e_result;
	}

	if (False != b_OplDebugLog) {
		int32_t sl_cnt = 0;
		_getAllRecordCountAtDB( _pPushKeyNameLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(キー名称DB)開始 %d\n", sl_cnt,0,0,0,0,0);
	}

	// 該当のキーを持つキー押下ログ（キー名称）DBを削除する.
	e_result = _deleteRecordByKeyAtDB( _pPushKeyNameLogDB, s_mngData.ul_SendOpeLogID );
	if( OPLE_RsltStatComplete != e_result ) {	// 削除失敗.
		s_mngData.e_MngOfSendOpeLogStatus = static_cast<OPLE_MngOfSendOpeLogStatus>(_getMngOfSendOpeLogNextStatus( s_mngData.e_MngOfSendOpeLogStatus, True, True ));
		_updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );
		return e_result;
	}
	

	if (False != b_OplDebugLog) {
		int32_t sl_cnt = 0;
		_getAllRecordCountAtDB( _pPushKeyNameLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(キー名称DB)終了 %d\n", sl_cnt,0,0,0,0,0);

		_getAllRecordCountAtDB( _pPushKeyLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(キーDB)開始 %d\n", sl_cnt,0,0,0,0,0);
	}

	// 該当のキーを持つキー押下ログDBを削除する.
	e_result = _deleteRecordByKeyAtDB( _pPushKeyLogDB, s_mngData.ul_SendOpeLogID );
	if( OPLE_RsltStatComplete != e_result ) {	// 削除失敗.
		s_mngData.e_MngOfSendOpeLogStatus = static_cast<OPLE_MngOfSendOpeLogStatus>(_getMngOfSendOpeLogNextStatus( s_mngData.e_MngOfSendOpeLogStatus, True, True ));
		_updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );
		return e_result;
	}
	
	if (False != b_OplDebugLog) {
		int32_t sl_cnt = 0;
		_getAllRecordCountAtDB( _pPushKeyLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(キーDB)終了 %d\n", sl_cnt,0,0,0,0,0);
	}

	// 送信操作ログ管理次ステータス取得処理で次のステータスを取得し、送信操作ログ管理DB構造体のステータスにセットする.
	s_mngData.e_MngOfSendOpeLogStatus = static_cast<OPLE_MngOfSendOpeLogStatus>(_getMngOfSendOpeLogNextStatus( s_mngData.e_MngOfSendOpeLogStatus, True, False ));
	s_mngData.us_KeyPushLogRecCount = 0;	// 送信操作ログ管理DB構造体のキー押下ログレコード件数に０をセットする.
	
	// 送信操作ログ管理DBを更新する.
	e_result = _updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );
	if(OPLE_RsltStatComplete != e_result){	// 更新失敗.
		return e_result;
	}

	if (False != b_OplDebugLog) {
		int32_t sl_cnt = 0;
		_getAllRecordCountAtDB( _pAdrInfoLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(宛先DB)開始 %d\n", sl_cnt,0,0,0,0,0);
	}

	// 該当のキーを持つ宛先情報ログDBを削除する.
	e_result = _deleteRecordByKeyAtDB( _pAdrInfoLogDB, s_mngData.ul_SendOpeLogID );
	if( OPLE_RsltStatComplete != e_result ){	// 削除失敗.
		s_mngData.e_MngOfSendOpeLogStatus = static_cast<OPLE_MngOfSendOpeLogStatus>(_getMngOfSendOpeLogNextStatus( s_mngData.e_MngOfSendOpeLogStatus, False, True ));
		_updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );
		return e_result;
	}

	if (False != b_OplDebugLog) {
		int32_t sl_cnt = 0;
		_getAllRecordCountAtDB( _pAdrInfoLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(宛先DB)終了 %d\n", sl_cnt,0,0,0,0,0);

		_getAllRecordCountAtDB( _pMngOfSendOpeLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(管理DB)開始 %d\n", sl_cnt,0,0,0,0,0);
	}
	
	// 該当のキーを持つ送信操作ログ管理DBを削除する.
	e_result = _deleteRecordByKeyAtDB( _pMngOfSendOpeLogDB, s_mngData.ul_SendOpeLogID );

	if (False != b_OplDebugLog) {
		int32_t sl_cnt = 0;
		_getAllRecordCountAtDB( _pMngOfSendOpeLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(管理DB)終了 %d\n", sl_cnt,0,0,0,0,0);
	}

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：不要レコード削除処理（宛先情報ログDB）.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_deleteSendOpeLogIDRecord_AdrDB( OPLS_MngOfSendOpeLog* p_data )
{
	OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(宛先情報ログDB) ID=%d\n", p_data->ul_SendOpeLogID,0,0,0,0,0);

	OPLS_MngOfSendOpeLog s_mngData = {};
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;

	memset( &s_mngData, 0x00, sizeof(OPLS_MngOfSendOpeLog) );
	s_mngData.ul_SendOpeLogID			= p_data->ul_SendOpeLogID;				// 操作ID.
	s_mngData.e_MngOfSendOpeLogStatus	= p_data->e_MngOfSendOpeLogStatus;		// 送信操作ログ管理ステータスコード.
	s_mngData.us_KeyPushLogRecCount		= p_data->us_KeyPushLogRecCount;		// キー押下ログレコード数.
	s_mngData.us_AdrInfoLogRecCount		= p_data->us_AdrInfoLogRecCount;		// 宛先情報ログレコード数.

	// 送信操作ログ管理次ステータス取得処理で次のステータスを取得し、送信操作ログ管理DB構造体のステータスにセットする.
	s_mngData.e_MngOfSendOpeLogStatus = static_cast<OPLE_MngOfSendOpeLogStatus>(_getMngOfSendOpeLogNextStatus( s_mngData.e_MngOfSendOpeLogStatus, True, False ));

	// 送信操作ログ管理DBを更新する.
	e_result = _updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );
	if( OPLE_RsltStatComplete != e_result ) {	// 更新失敗.
		return e_result;
	}
	
	if (False != b_OplDebugLog) {
		int32_t sl_cnt = 0;
		_getAllRecordCountAtDB( _pAdrInfoLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(宛先DB)開始 %d\n", sl_cnt,0,0,0,0,0);
	}

	// 該当のキーを持つ宛先情報ログDBを削除する.
	e_result = _deleteRecordByKeyAtDB( _pAdrInfoLogDB, s_mngData.ul_SendOpeLogID );
	if( OPLE_RsltStatComplete != e_result ) {	// 削除失敗.
		s_mngData.e_MngOfSendOpeLogStatus = static_cast<OPLE_MngOfSendOpeLogStatus>(_getMngOfSendOpeLogNextStatus( s_mngData.e_MngOfSendOpeLogStatus, False, True ));
		_updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );
		return e_result;
	}
	
	if (False != b_OplDebugLog) {
		int32_t sl_cnt = 0;
		_getAllRecordCountAtDB( _pAdrInfoLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(宛先DB)終了 %d\n", sl_cnt,0,0,0,0,0);

		_getAllRecordCountAtDB( _pMngOfSendOpeLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(管理DB)開始 %d\n", sl_cnt,0,0,0,0,0);
	}

	// 該当のキーを持つ送信操作ログ管理DBを削除する.
	e_result = _deleteRecordByKeyAtDB( _pMngOfSendOpeLogDB, s_mngData.ul_SendOpeLogID );
	
	if (False != b_OplDebugLog) {
		int32_t sl_cnt = 0;
		_getAllRecordCountAtDB( _pMngOfSendOpeLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(管理DB)終了 %d\n", sl_cnt,0,0,0,0,0);
	}
	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：不要レコード削除処理（送信操作ログDB）.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_deleteSendOpeLogIDRecord_LogDB( OPLS_MngOfSendOpeLog* p_data )
{
	OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(送信操作ログDB) ID=%d\n", p_data->ul_SendOpeLogID,0,0,0,0,0);

	OPLS_MngOfSendOpeLog s_mngData = {};
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	Bool b_Error = False;

	memset( &s_mngData, 0x00, sizeof(OPLS_MngOfSendOpeLog) );
	s_mngData.ul_SendOpeLogID			= p_data->ul_SendOpeLogID;				// 操作ID.
	s_mngData.e_MngOfSendOpeLogStatus	= p_data->e_MngOfSendOpeLogStatus;		// 送信操作ログ管理ステータスコード.
	s_mngData.us_KeyPushLogRecCount		= p_data->us_KeyPushLogRecCount;		// キー押下ログレコード数.
	s_mngData.us_AdrInfoLogRecCount		= p_data->us_AdrInfoLogRecCount;		// 宛先情報ログレコード数.

	// 送信操作ログ管理次ステータス取得処理で次のステータスを取得し、送信操作ログ管理DB構造体のステータスにセットする.
	s_mngData.e_MngOfSendOpeLogStatus = static_cast<OPLE_MngOfSendOpeLogStatus>(_getMngOfSendOpeLogNextStatus( s_mngData.e_MngOfSendOpeLogStatus, True, False ));

	// 送信操作ログ管理DBを更新する.
	e_result = _updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );
	if( OPLE_RsltStatComplete != e_result ) {	// 更新失敗.
		return e_result;
	}

	// 該当のキーを持つ送信操作ログDBを削除する.
	uint32_t ul_index = uc_SendOpeLogDBWriteIndex;
	uint32_t ul_delete_cnt = 0;
	int32_t sl_cnt = 0;
	for ( ; ; ) {

		_getAllRecordCountAtDB( _pSendOpeLogDB[ul_index], &sl_cnt );
		if (0 != sl_cnt) {	// 書き込みインデックスが示すDBのレコード数が0の時は、前のDBで書き込み終了しているので、除外する
			if (False != b_OplDebugLog) {
				OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(送信操作ログDB%02d)開始 %d\n", ul_index, sl_cnt,0,0,0,0);
			}

			e_result = _deleteRecordByKeyAtDB(_pSendOpeLogDB[ul_index], s_mngData.ul_SendOpeLogID, &ul_delete_cnt);
			if (OPLE_RsltStatComplete != e_result) {
				b_Error = True;
			}

			if (False != b_OplDebugLog) {
				int32_t sl_cnt2 = 0;
				_getAllRecordCountAtDB( _pSendOpeLogDB[ul_index], &sl_cnt2 );
				OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(送信操作ログDB%02d)終了 %d\n", ul_index, sl_cnt2,0,0,0,0);
			}

			if ((0 == ul_delete_cnt) || (True == b_Error)) {
				if (uc_SendOpeLogDBWriteIndex != ul_index) {
					ul_index = _IncrementSendOpeLogDBIndex(ul_index);
					_setSendOpeLogDBWriteIndex(ul_index);
				}
				break;
			}
		}
		ul_index = _DecrementSendOpeLogDBIndex(ul_index);
	}
	
	// 送信操作ログ管理次ステータス取得処理で次のステータスを取得し、送信操作ログ管理DB構造体のステータスにセットする.
	s_mngData.e_MngOfSendOpeLogStatus = static_cast<OPLE_MngOfSendOpeLogStatus>(_getMngOfSendOpeLogNextStatus( s_mngData.e_MngOfSendOpeLogStatus, False, b_Error ));
	
	// 送信操作ログ管理DBを更新する.
	e_result = _updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );
	
	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：不要レコード削除処理（宛先情報ログDB）.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_deleteSendOpeLogIDRecord_MngDB( OPLS_MngOfSendOpeLog* p_data )
{
	OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(管理DB) ID=%d\n", p_data->ul_SendOpeLogID,0,0,0,0,0);

	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;

	if (False != b_OplDebugLog) {
		int32_t sl_cnt = 0;
		_getAllRecordCountAtDB( _pMngOfSendOpeLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(管理DB)開始 %d\n", sl_cnt,0,0,0,0,0);
	}

	// 該当のキーを持つ送信操作ログ管理DBを削除する.
	e_result = _deleteRecordByKeyAtDB( _pMngOfSendOpeLogDB, p_data->ul_SendOpeLogID );

	if (False != b_OplDebugLog) {
		int32_t sl_cnt = 0;
		_getAllRecordCountAtDB( _pMngOfSendOpeLogDB, &sl_cnt );
		OPLDEBUG_LOGPRINT("[OPL]不要レコード削除処理(管理DB)終了 %d\n", sl_cnt,0,0,0,0,0);
	}

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：不要レコード削除処理（宛先情報ログDB）.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_deleteAllLogDB()
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	
	OPLDEBUG_LOGPRINT("[OPL]全削除 開始\n", 0,0,0,0,0,0);

	// 送信操作ログ管理DBをクリアする.
	if ( OPLE_RsltStatComplete != _allDataClearAtDB( _pMngOfSendOpeLogDB ) ) {
		e_result = OPLE_RsltStatError;	// 異常を返す.
	}

	// 送信操作ログDBをクリアする.
	if ( OPLE_RsltStatComplete != _deleteSendOpeLogDB() ) {
		e_result = OPLE_RsltStatError;	// 異常を返す.
	}

	// キー押下ログ（キー名称）DBをクリアする.
	if ( OPLE_RsltStatComplete != _allDataClearAtDB( _pPushKeyNameLogDB ) ) {
		e_result = OPLE_RsltStatError;	// 異常を返す.
	}
	
	// キー押下ログDBをクリアする.
	if ( OPLE_RsltStatComplete != _allDataClearAtDB( _pPushKeyLogDB ) ) {
		e_result = OPLE_RsltStatError;	// 異常を返す.
	}
	
	// 宛先情報ログDBをクリアする.
	if ( OPLE_RsltStatComplete != _allDataClearAtDB( _pAdrInfoLogDB ) ) {
		e_result = OPLE_RsltStatError;	// 異常を返す.
	}
	
	OPLDEBUG_LOGPRINT("[OPL]全削除 終了\n", 0,0,0,0,0,0);
	return e_result;
}
//関数----------------------------------------------------------------------------------------------.
//概要    ：不要レコード全削除処理（送信操作ログDB）.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_deleteSendOpeLogDB()
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;

	_setSendOpeLogDBWriteIndex(OPLE_SendOpeLogDBIndexDelete);
	for (int i = 0; i < OPLE_SendOpeLogDBMax; i++) {
		if ( OPLE_RsltStatComplete != _allDataClearAtDB( _pSendOpeLogDB[i] ) ) {
			e_result = OPLE_RsltStatError;	// 異常を返す.
			break;
		}
	}
	OPLDEBUG_WAIT("送信操作ログ全削除完了前");
	_setSendOpeLogDBWriteIndex(OPLE_SendOpeLogDBIndexInit);

	return e_result;
}

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■.
//■■■　　保存処理.
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■.
//関数----------------------------------------------------------------------------------------------.
//概要    ：ログ一時保存処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_saveLogToNVDB( OPLS_KeyPushLogIF*  ps_KeyLogIF, ushort  us_KeyCount, OPLS_AdrInfoLogIF*  ps_AdrLogIF, ushort  us_AdrCount, uint32_t *ul_SendOpeLogID )
{
	OPLS_MngOfSendOpeLog s_mngData = {};
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	ushort us_number = 0;

	Bool b_adrInfo = ( 0 < us_AdrCount )? True : False;

	if( False == _isEnableSaveLogToNVDB( us_KeyCount, us_AdrCount) ) {	// ログ保存エリアが空いていない.
		e_result = OPLE_RsltStatError;
		return e_result;
	}
	OPLDEBUG_LOGPRINT("[OPL]_saveLogToNVDB\n", 0,0,0,0,0,0);

	memset( &s_mngData, 0x00, sizeof(OPLS_MngOfSendOpeLog) );
	s_mngData.ul_SendOpeLogID			= 0;									// 操作ID.
	s_mngData.e_MngOfSendOpeLogStatus	= OPLE_MngStatNoStatus;					// 送信操作ログ管理ステータスコード.
	s_mngData.us_KeyPushLogRecCount		= us_KeyCount;							// キー押下ログレコード数.
	s_mngData.us_AdrInfoLogRecCount		= us_AdrCount;							// 宛先情報ログレコード数.

	// 送信操作ログ管理DBに登録する.
	e_result = _putMngOfSendOpeLogRecord( &s_mngData );
	if( OPLE_RsltStatComplete != e_result ) {	// 登録失敗.
		return e_result;
	}

	// 次の送信操作ログ管理ステータスを取得する.
	s_mngData.e_MngOfSendOpeLogStatus = static_cast<OPLE_MngOfSendOpeLogStatus>(_getMngOfSendOpeLogNextStatus( s_mngData.e_MngOfSendOpeLogStatus, b_adrInfo, False ));
	_updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );	// 送信操作ログ管理DBを更新する.

	// キー押下ログ情報を登録する.
	us_number = 1;
	e_result = _makeKeyPushLogDB( ps_KeyLogIF, us_KeyCount, s_mngData.ul_SendOpeLogID, us_number );
	if( OPLE_RsltStatComplete != e_result ) {	// キー押下ログ情報、登録失敗.
		// エラー発生時の次の送信操作ログ管理ステータスを取得する.
		s_mngData.e_MngOfSendOpeLogStatus = static_cast<OPLE_MngOfSendOpeLogStatus>(_getMngOfSendOpeLogNextStatus( s_mngData.e_MngOfSendOpeLogStatus, b_adrInfo, True ));
		_updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );	// 送信操作ログ管理DBを更新する.
		return e_result;
	}

	if( 0 < us_AdrCount ) {	// 宛先情報ログ1件以上あり.
		// 次の送信操作ログ管理ステータスを取得する.
		s_mngData.e_MngOfSendOpeLogStatus = static_cast<OPLE_MngOfSendOpeLogStatus>(_getMngOfSendOpeLogNextStatus( s_mngData.e_MngOfSendOpeLogStatus, b_adrInfo, False ));
		_updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );
		
		us_number = static_cast<ushort>(us_KeyCount + 1);
		e_result = _makeAdrInfoLogDB( ps_AdrLogIF, us_AdrCount, s_mngData.ul_SendOpeLogID, us_number );	// 宛先情報ログ情報を登録する.
		if( OPLE_RsltStatComplete != e_result ) {	// 宛先情報ログ、保存失敗.
			// エラー発生時の次の送信操作ログ管理ステータスを取得する.
			s_mngData.e_MngOfSendOpeLogStatus = static_cast<OPLE_MngOfSendOpeLogStatus>(_getMngOfSendOpeLogNextStatus( s_mngData.e_MngOfSendOpeLogStatus, b_adrInfo, True ));
			_updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );	// 送信操作ログ管理DBを更新する.
			return e_result;
		}
	}

	// 次の送信操作ログ管理ステータスを取得する.
	s_mngData.e_MngOfSendOpeLogStatus = static_cast<OPLE_MngOfSendOpeLogStatus>(_getMngOfSendOpeLogNextStatus( s_mngData.e_MngOfSendOpeLogStatus, b_adrInfo, False ));
	_updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );

	if (NULL != ul_SendOpeLogID) {
		*ul_SendOpeLogID = s_mngData.ul_SendOpeLogID;
	}
	return e_result;
}


//関数----------------------------------------------------------------------------------------------.
//概要    ：ログ一時保存可能判断.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
Bool OPLC_Database::_isEnableSaveLogToNVDB( ushort us_KeyCount, ushort us_AdrCount )
{
	int32_t sl_keyCnt = 0;
	int32_t sl_adrCnt = 0;
	
	// キー押下ログDBレコード件数を取得.
	_getAllRecordCountAtDB( _pPushKeyLogDB, &sl_keyCnt );
	if ( ( OPLD_KeyPushLogRecMax - sl_keyCnt )  < static_cast<int32_t>(us_KeyCount) ) {
		return False;
	}
	
	// 宛先情報ログDBレコード件数を取得.
	_getAllRecordCountAtDB( _pAdrInfoLogDB, &sl_adrCnt );
	if ( ( OPLD_AdrInfoLogRecMax - sl_adrCnt )  < static_cast<int32_t>(us_AdrCount) ) {
		return False;
	}
	
	return True;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：送信操作ログ管理レコード登録.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_putMngOfSendOpeLogRecord( OPLS_MngOfSendOpeLog* ps_Data )
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	
	ps_Data->ul_SendOpeLogID = _numberingOperationID();		// 操作ID.
	ps_Data->e_MngOfSendOpeLogStatus = OPLE_MngStatIDSet;	// 送信操作ログ管理ステータスコード.
	
	// 送信操作ログ管理DBにレコード追加する.
	e_result = _addRecordAtDB( _pMngOfSendOpeLogDB, ps_Data, ps_Data->ul_SendOpeLogID );
	
	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：キー押下ログDB登録.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_makeKeyPushLogDB( OPLS_KeyPushLogIF* ps_Data, ushort us_count, uint32_t ul_Key, ushort us_Num )
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	OPLS_KeyPushLogIF* ps_keyIf = NULL;
	ushort us_cnt = 0;

	ps_keyIf = ps_Data;
	us_cnt = us_Num;
	for ( int i = 0; i < us_count; i++ ) {
		e_result = _putKeyPushLogDB( ps_keyIf, ul_Key, static_cast<ushort>(us_cnt+i) );	// キー押下を宛先情報ログDB登録に渡す.
		if( OPLE_RsltStatComplete != e_result ){	// 登録失敗.
			return e_result;
		}
		ps_keyIf++;
	}

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：キー押下ログレコードDB登録.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_putKeyPushLogDB( OPLS_KeyPushLogIF* ps_Data, uint32_t ul_Key, ushort us_Num )
{
	OPLS_KeyPushLog s_keyPushLog = {};
	OPLS_PushKeyNameLog s_pushKeyNameLog = {};
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;

	if( NULL == ps_Data ){
		return OPLE_RsltStatError;
	}

	memset( &s_keyPushLog, 0x00, sizeof(OPLS_KeyPushLog) );
	memset( &s_pushKeyNameLog, 0x00, sizeof(OPLS_PushKeyNameLog) );

	s_keyPushLog.ul_SendOpeLogID = ul_Key;															// 操作ID.
	s_keyPushLog.us_LogNo = us_Num;																	// 操作ID内の連番.
	memcpy( s_keyPushLog.sc_EventTimeStr, ps_Data->sc_EventTimeStr, OPLD_TimeStrMax );				// イベント発生時刻（YYYYMMDDHHDDSSSSS）.
	s_keyPushLog.e_EventStatusCode = ps_Data->e_EventStatusCode;									// イベントステータスコード.
	s_keyPushLog.e_EventDataKind = ps_Data->e_EventDataKind;										// イベントデータ種別コード.
	s_keyPushLog.us_EventDataID = ps_Data->us_EventDataID;											// イベント名の文字列ID/状態変化内容コード.
	s_keyPushLog.us_EventSubDataID = ps_Data->us_EventSubDataID;									// 削除文字の文字ID/補足情報コード.

	if( OPLE_EventDataKindStrIdStrChar == ps_Data->e_EventDataKind ) {	// または文字列ID＋文字列の場合.
		memcpy( s_keyPushLog.sc_EventSubDataStr, ps_Data->sc_EventDataStr, OPLD_SubInfoStrMax );
		s_keyPushLog.us_EventSubDataID = 0;
	}
	if ( OPLE_SubInfoKeyPosition == ps_Data->us_EventSubDataID ) {		// キー座標の場合.
		memcpy( s_keyPushLog.sc_EventSubDataStr, ps_Data->sc_EventSubDataStr, OPLD_SubInfoStrMax );
	}

	// キー押下ログDBにレコード追加する.
	e_result = _addRecordAtDB( _pPushKeyLogDB, &s_keyPushLog, ul_Key );
	if( OPLE_RsltStatComplete != e_result ){	// 登録失敗.
		return e_result;
	}

	if( OPLE_EventDataKindStrChar == ps_Data->e_EventDataKind ) {		// イベントデータ種別コードが文字列.

		s_pushKeyNameLog.ul_SendOpeLogID = ul_Key;															// 操作ID.
		s_pushKeyNameLog.us_LogNo = us_Num;																	// 操作ID内の連番.
		memcpy( s_pushKeyNameLog.sc_EventDataStr, ps_Data->sc_EventDataStr, OPLD_KeyNameStrMax );			// 文字列IDを持たないキー名称文字列.

		e_result = _addRecordAtDB( _pPushKeyNameLogDB, &s_pushKeyNameLog, ul_Key );	// キー押下ログ（キー名称）DBにレコード追加する.
	}

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：宛先情報ログDB登録.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_makeAdrInfoLogDB( OPLS_AdrInfoLogIF* ps_Data, ushort us_count, uint32_t ul_Key, ushort us_Num )
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	OPLS_AdrInfoLogIF* ps_adrIf = NULL;
	ushort us_cnt = 0;

	ps_adrIf = ps_Data;
	us_cnt = us_Num;
	for ( int i = 0; i < us_count; i++ ) {
		e_result = _putAdrInfoLogDB( ps_adrIf, ul_Key, static_cast<ushort>(us_cnt+i) );	//宛先情報を宛先情報ログDB登録に渡す.
		
		if( OPLE_RsltStatComplete != e_result ){	// 登録失敗.
			return e_result;
		}
		ps_adrIf++;
	}

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：宛先情報ログDBレコード作成.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_putAdrInfoLogDB( OPLS_AdrInfoLogIF* ps_Data, uint32_t ul_Key, ushort us_Num )
{
	OPLS_AdrInfoLog s_adrInfoLog = {};
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	
	memset( &s_adrInfoLog, 0x00, sizeof(OPLS_AdrInfoLog) );
	
	s_adrInfoLog.ul_SendOpeLogID = ul_Key;															// 操作ID.
	s_adrInfoLog.us_LogNo = us_Num;																	// 操作ID内の連番.
	memcpy( s_adrInfoLog.sc_EventTimeStr, ps_Data->sc_EventTimeStr, OPLD_TimeStrMax );				// イベント発生時刻（YYYYMMDDHHDDSSSSS）.
	s_adrInfoLog.e_AdrKind = ps_Data->e_AdrKind;													// 直接宛先・短縮宛先の区分.
	s_adrInfoLog.e_SendMode = ps_Data->e_SendMode;													// 宛先種別.
	s_adrInfoLog.ss_ShorteningAddressNo = ps_Data->ss_ShorteningAddressNo;							// 登録番号.
	memcpy( s_adrInfoLog.sc_AddressName, ps_Data->sc_AddressName, TYPD_MaxStrAddressName );			// 登録名.
	memcpy( s_adrInfoLog.sc_Address, ps_Data->sc_Address, OPLD_EventDataStrMax );					// アドレス/ボックス番号.
	memcpy( s_adrInfoLog.sc_Directory, ps_Data->sc_Directory, TYPD_MaxStrSmbDir );					// ディレクトリ名.
	
	e_result = _addRecordAtDB( _pAdrInfoLogDB, &s_adrInfoLog, ul_Key );	// 宛先情報ログDBにレコード追加する.
	
	return e_result;
}

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■.
//■■■　　保存処理（正式保存）.
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■.

//関数----------------------------------------------------------------------------------------------.
//概要    ：ログ正式保存処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_saveLogDB(uint32_t *ul_SendOpeLogID)
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	
	int  i_next = 0;
	int32_t sl_cnt = 0;
	LIST c_List = {};
	SYSS_RecordListHeader* p_RecordList = NULL;
	SYSS_RecordListHeader* p_tmp = NULL;
	OPLS_MngOfSendOpeLog* p_data = NULL;
	
	SYSF_lstInit(&c_List);
	_getAllRecordCountAtDB( _pMngOfSendOpeLogDB, &sl_cnt );	// 送信操作ログ管理DBのレコード数を取得する.
	
	OPLDEBUG_LOGPRINT("[OPL]_saveLogDB cnt=%ld\n", sl_cnt,0,0,0,0,0);
	if(sl_cnt > 0) {	// 送信操作ログ管理DBにレコードが存在する場合.
		_getListRecordsAtDB( _pMngOfSendOpeLogDB, &c_List, OPLD_MngOfSendOpeLogRecMax, 0 );	// 送信操作ログ管理DB全レコードをリストで取得する.
		
		p_RecordList = reinterpret_cast<SYSS_RecordListHeader*>(SYSF_lstFirst( &c_List ));	// リストの先頭データポインタを取得する.
		while( NULL!=p_RecordList ){
			p_tmp = p_RecordList;
			p_data = reinterpret_cast<OPLS_MngOfSendOpeLog*>(++p_tmp);
			
			e_result = _distributeSendOpeLogRecord(p_data, &i_next);

			if (NULL != ul_SendOpeLogID) {
				*ul_SendOpeLogID = p_data->ul_SendOpeLogID;
			}

			if( OPLE_RsltStatComplete != e_result) {	// 異常終了.
				break;	// ループを抜ける.
			}

			if( 0 == i_next) {	// 振り分け完了.
				break;	// ループを抜ける.
			}
			
			p_RecordList = reinterpret_cast<SYSS_RecordListHeader *>(SYSF_lstNext( &p_RecordList->Node ));	// 次のリストデータポインタを取得する.
		}
		
		_deleteListRecordsAtDB(_pMngOfSendOpeLogDB,&c_List);	// 使用したリストを開放する.
	}

	return e_result;

}

//関数----------------------------------------------------------------------------------------------.
//概要    ：正式保存対象レコードキー振り分け処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_distributeSendOpeLogRecord( OPLS_MngOfSendOpeLog* p_data, int* i_NextFlag )
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;

	*i_NextFlag = 0;									// 次データ無をセットする.

	switch( p_data->e_MngOfSendOpeLogStatus ) {
		case	OPLE_MngStatSaveKeyPushEnd :			// 宛先情報がない場合のキー押下ログ保存完了.
		case	OPLE_MngStatSaveAdrInfoEnd :			// 宛先情報ログ保存完了.
		case	OPLE_MngStatDelSendOpeLogErr :			// 送信操作ログ削除完了.
			e_result = _procSendOpeLogRecord(p_data);	// 送信操作ログDBレコード保存処理を呼ぶ.
			*i_NextFlag = 0;							// 次データ無をセットする.
			break;

		case	OPLE_MngStatSaveSendOpeLogEnd :			// 送信操作ログ保存完了.
		case	OPLE_MngStatDelKeyAndAdrStart :			// 送信操作ログ保存完了によるキー押下ログ、宛先情報ログ削除.
		case	OPLE_MngStatDelKeyPushStart :			// 送信操作ログ保存完了によるキー押下ログ削除.
		case	OPLE_MngStatDelKeyAndAdr :				// 送信操作ログ保存完了によるキー押下ログ削除中（宛先情報あり）.
		case	OPLE_MngStatDelKeyPush :				// 送信操作ログ保存完了によるキー押下ログ削除中（宛先情報なし）.
		case	OPLE_MngStatDelKeyPushEnd :				// 送信操作ログ保存完了によるキー押下ログ削除完了（宛先情報あり）.
		case	OPLE_MngStatErrDelAdrInfoStart :		// 宛先情報ログ保存エラー時の宛先情報ログ削除.
		case	OPLE_MngStatDelAdrInfoStart :			// 送信操作ログ保存完了による宛先情報ログ削除.
		case	OPLE_MngStatErrDelAdrInfo :				// 宛先情報ログ保存エラー時の宛先情報ログ削除中.
		case	OPLE_MngStatDelAdrInfo :				// 送信操作ログ保存完了による宛先情報ログ削除中.
		case	OPLE_MngStatErrDelAdrInfoEnd :			// 宛先情報ログ保存エラー時の宛先情報ログ削除完了.
		case	OPLE_MngStatDelAdrInfoEnd :				// 送信操作ログ保存完了による宛先情報ログ削除完了.
		case	OPLE_MngStatDelSendOpeLogStart:			// 送信操作ログ削除.
			*i_NextFlag = 0;							// 次データ無をセットする.
			break;
		default :
			*i_NextFlag = 1;							// 次データ有をセットする.
			break;
	}
	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：送信操作ログDBレコード保存処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_procSendOpeLogRecord( OPLS_MngOfSendOpeLog* p_data )
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	int32_t sl_retryCnt = 0;

	for ( ; ; ) {
		OPLDEBUG_LOGPRINT("[OPL]送信操作ログ保存    [ID:%d, %d回目]\n",p_data->ul_SendOpeLogID, sl_retryCnt+1,0,0,0,0);
		e_result = _saveSendOpeLogRecord(p_data);				// 送信操作ログ保存処理.
		if (OPLE_RsltStatComplete == e_result) {				// 保存成功した場合.
			OPLDEBUG_LOGPRINT("[OPL]送信操作ログ保存成功[ID:%d, %d回目]\n",p_data->ul_SendOpeLogID, sl_retryCnt+1,0,0,0,0);
			break;
		}

		OPLDEBUG_LOGPRINT("[OPL]送信操作ログ保存失敗[ID:%d, %d回目]\n",p_data->ul_SendOpeLogID, sl_retryCnt+1,0,0,0,0);
		sl_retryCnt++;											// リトライ回数カウントアップ.
		if (OPLE_SendOpeLogSaveRetryMax <= sl_retryCnt) {		// リトライ回数が最大数に達した場合.
			e_result = OPLE_RsltStatError;						// 保存失敗で終了する.
			break;
		}

		_deleteSendOpeLogIDRecord_LogDB(p_data);				// 途中まで保存した送信操作ログを削除する.
	}
	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：送信操作ログDBレコード保存処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_saveSendOpeLogRecord( OPLS_MngOfSendOpeLog* p_data )
{
	OPLS_MngOfSendOpeLog s_mngData = {};
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	int32_t sl_recCnt = 0;
	ushort us_localNo = 0;
	Bool b_error = False;
	Bool b_adrInfo = ( 0 < p_data->us_AdrInfoLogRecCount )? True : False;


	// 送信操作ログ管理DBデータをセット.
	memset( &s_mngData, 0x00, sizeof(OPLS_MngOfSendOpeLog) );
	s_mngData.ul_SendOpeLogID			= p_data->ul_SendOpeLogID;				// 操作ID.
	s_mngData.e_MngOfSendOpeLogStatus	= OPLE_MngStatSaveSendOpeLog;			// 送信操作ログ保存中.
	s_mngData.us_KeyPushLogRecCount		= p_data->us_KeyPushLogRecCount;		// キー押下ログレコード数.
	s_mngData.us_AdrInfoLogRecCount		= p_data->us_AdrInfoLogRecCount;		// 宛先情報ログレコード数.

	e_result = _updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );	// 送信操作ログ管理DBを更新.
	if( OPLE_RsltStatComplete != e_result ){	// 更新失敗.
		return e_result;
	}

	if (False != b_OplDebugLog) {
		int32_t sl_cnt = 0;
		sl_cnt = _getSendOpeLogDataCount();
		OPLDEBUG_LOGPRINT("[OPL]送信操作ログDBレコード数   %d\n", sl_cnt,0,0,0,0,0);
		OPLDEBUG_LOGPRINT("[OPL]保存するレコード数         %d,%d\n", s_mngData.us_KeyPushLogRecCount,s_mngData.us_AdrInfoLogRecCount,0,0,0,0);
	}

	sl_recCnt = s_mngData.us_KeyPushLogRecCount + (s_mngData.us_AdrInfoLogRecCount * 5);	// キー押下ログ件数＋宛先情報ログ件数*5で送信操作ログ登録レコード件数を出す.
	e_result = _checkBlankRecAtSendOpeLogDB( sl_recCnt );	// 送信操作ログ保存可能エリア確保処理で送信操作ログ登録レコード件数分の空きエリアを空ける.
	if( OPLE_RsltStatComplete != e_result ){	// 処理失敗.
		return e_result;
	}

	_getAllRecordCountAtDB( _pSendOpeLogDB[uc_SendOpeLogDBWriteIndex], &us_SendOpeLogDBWriteRecord );	// 現在書き込み中の送信操作ログDBのレコード数を取得する.
	OPLDEBUG_LOGPRINT("[OPL]★★★★★★★★送信操作ログ保存開始[ID:%d,Index:%d, Records:%d]★★★★★★★★\n", p_data->ul_SendOpeLogID, uc_SendOpeLogDBWriteIndex, us_SendOpeLogDBWriteRecord,0,0,0);

	us_localNo = 0;
	e_result = _convertKeyPushLogToSendOpeLog( p_data->ul_SendOpeLogID, &us_localNo );	// キー押下ログ編集.
	if( False == b_adrInfo ) {	// 宛先情報無.
		e_result = OPLDEBUG_SAVELOGNG(e_result);
	}
	if( OPLE_RsltStatComplete != e_result ) {	// 処理失敗.
		b_error = True;
		s_mngData.e_MngOfSendOpeLogStatus = static_cast<OPLE_MngOfSendOpeLogStatus>(_getMngOfSendOpeLogNextStatus( s_mngData.e_MngOfSendOpeLogStatus, b_adrInfo, b_error ));	// 次の送信操作ログ管理ステータスを取得.
		_updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );	// 送信操作ログ管理DBを更新する.
		return e_result;
	}

	if( True == b_adrInfo ) {	// 宛先情報有.
		e_result = _convertAdrInfoLogToSendOpeLog( p_data->ul_SendOpeLogID, &us_localNo );	// 宛先情報ログ編集.
		e_result = OPLDEBUG_SAVELOGNG(e_result);
		if( OPLE_RsltStatComplete != e_result ) {
			b_error = True;
			s_mngData.e_MngOfSendOpeLogStatus = static_cast<OPLE_MngOfSendOpeLogStatus>(_getMngOfSendOpeLogNextStatus( s_mngData.e_MngOfSendOpeLogStatus, b_adrInfo, b_error ));	// 次の送信操作ログ管理ステータスを取得.
			_updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );	// 送信操作ログ管理DBを更新.
			return e_result;
		}
	}

	s_mngData.e_MngOfSendOpeLogStatus = static_cast<OPLE_MngOfSendOpeLogStatus>(_getMngOfSendOpeLogNextStatus( s_mngData.e_MngOfSendOpeLogStatus, b_adrInfo, b_error ));	// 次の送信操作ログ管理ステータスを取得.
	e_result = _updateRecordByKeyAtDB( _pMngOfSendOpeLogDB, &s_mngData, s_mngData.ul_SendOpeLogID );	// 送信操作ログ管理DBを更新.

	OPLDEBUG_LOGPRINT("[OPL]★★★★★★★★送信操作ログ保存終了[ID:%d,Index:%d, Records:%d]★★★★★★★★\n", p_data->ul_SendOpeLogID, uc_SendOpeLogDBWriteIndex, us_SendOpeLogDBWriteRecord,0,0,0);

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：送信操作ログDB保存可能エリアをチェックする.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_checkBlankRecAtSendOpeLogDB( int32_t sl_size )
{
	int32_t sl_cnt = 0;
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	int32_t sl_remain_record = 0;
	uint32_t ul_index = uc_SendOpeLogDBWriteIndex;
	uint32_t ul_delete_cnt = 0;
	LIST c_List = {};
	SYSS_RecordListHeader* p_RecordList = NULL;
	SYSS_RecordListHeader* p_tmp = NULL;
	OPLS_SendOpeLog* p_data = NULL;
	uint32_t ul_SendOpeLogID = 0;

		OPLDEBUG_LOGPRINT("[OPL]送信操作ログDB必要残レコード数 %d\n", sl_size,0,0,0,0,0);

	_getAllRecordCountAtDB( _pSendOpeLogDB[ul_index], &sl_cnt );					// 現在書き込み中の送信操作ログDBの全レコード数を取得.
	sl_remain_record = OPLE_SendOpeLogRecordMax - sl_cnt;							// 現在書き込み中の送信操作ログDBの残レコード数を算出.

		OPLDEBUG_LOGPRINT("[OPL]送信操作ログDB%02d残レコード数  %d\n", ul_index, sl_remain_record,0,0,0,0);

	for ( ; ; ) {
		OPLDEBUG_LOGPRINT("[OPL]送信操作ログDB合計残レコード数 %d\n", sl_remain_record,0,0,0,0,0);
		if (sl_remain_record > sl_size) {											// 保存するレコード数だけの空きがある.
			break;
		}
		ul_index = _IncrementSendOpeLogDBIndex(ul_index);
		_getAllRecordCountAtDB( _pSendOpeLogDB[ul_index], &sl_cnt );
		if (0 == sl_cnt) {
			OPLDEBUG_LOGPRINT("[OPL]送信操作ログDB%02d空[%d]\n", ul_index,sl_cnt,0,0,0,0);
			sl_remain_record += OPLE_SendOpeLogRecordMax;							// 空DBなので、何もしないで残レコード数を加算する.
		} else {
			OPLDEBUG_LOGPRINT("[OPL]送信操作ログDB%02d使用済 全削除開始[%d]\n", ul_index,sl_cnt,0,0,0,0);

			// 削除する送信操作ログDBの最後のレコードの操作IDを取得する.
			SYSF_lstInit(&c_List);													// リスト初期化.
			_getListRecordsAtDB( _pSendOpeLogDB[ul_index], &c_List, 1, sl_cnt-1 );	// 最終のレコード取得.
			p_RecordList = reinterpret_cast<SYSS_RecordListHeader*>(SYSF_lstFirst( &c_List ));		// リストの先頭データポインタを取得する.
			p_tmp = p_RecordList;
			p_data = reinterpret_cast<OPLS_SendOpeLog*>(++p_tmp);
			ul_SendOpeLogID = p_data->ul_SendOpeLogID;								// 操作IDを確保しておく.
			_deleteListRecordsAtDB(_pSendOpeLogDB[ul_index], &c_List);				// 使用したリストを開放する.

			if ( OPLE_RsltStatComplete != _allDataClearAtDB( _pSendOpeLogDB[ul_index] ) ) {	// 送信操作ログのレコード全削除.
				e_result = OPLE_RsltStatError;
				break;
			}
			sl_remain_record += OPLE_SendOpeLogRecordMax;
			OPLDEBUG_LOGPRINT("[OPL]送信操作ログDB%02d使用済 全削除終了 最後レコード操作ID:%d\n", ul_index,ul_SendOpeLogID,0,0,0,0);
		}
	}

	if (0 != ul_SendOpeLogID) {
		// 古い送信操作ログDBのレコードを削除しているので、最後に削除したDBの最後のレコードの操作IDの残りのレコードを削除する.
		OPLDEBUG_LOGPRINT("[OPL]最後に削除したDBの最後レコード操作ID %d\n", ul_SendOpeLogID,0,0,0,0,0);
		for ( ; ; ) {
			ul_index = _IncrementSendOpeLogDBIndex(ul_index);

			if ( OPLE_RsltStatError == _deleteRecordByKeyAtDB(_pSendOpeLogDB[ul_index], ul_SendOpeLogID, &ul_delete_cnt)) {
				e_result = OPLE_RsltStatError;
				break;
			}
			OPLDEBUG_LOGPRINT("[OPL]送信操作ログDB%02d 操作ID:%d レコード削除数:%d\n", ul_index,ul_SendOpeLogID,ul_delete_cnt,0,0,0);
			if (0 == ul_delete_cnt) {
				break;
			}
		}
	}

	return e_result;
}


//関数----------------------------------------------------------------------------------------------.
//概要    ：キー押下ログを送信操作ログにする.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_convertKeyPushLogToSendOpeLog( uint32_t ul_SendOpeLogID, ushort* us_StartNum )
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	LIST c_List = {};
	SYSS_RecordListHeader* p_RecordList = NULL;
	SYSS_RecordListHeader* p_tmp = NULL;
	OPLS_KeyPushLog* p_data = NULL;
	ushort us_localNo = 0;

	SYSF_lstInit(&c_List);
	if( NULL != us_StartNum ) {
		us_localNo = *us_StartNum;
	}

	e_result = _getRecordListByKeyAtDB(_pPushKeyLogDB, &c_List, ul_SendOpeLogID );	// キー押下ログDBレコードを取得.
	if( OPLE_RsltStatComplete != e_result ){	// 取得失敗.
		return e_result;
	}
	
	p_RecordList = reinterpret_cast<SYSS_RecordListHeader*>(SYSF_lstFirst( &c_List ));	//リストの先頭データポインタを取得する.
	while( NULL!=p_RecordList ){
		p_tmp = p_RecordList;
		p_data = reinterpret_cast<OPLS_KeyPushLog*>(++p_tmp);
		
		us_localNo = static_cast<ushort>(us_localNo + 1);
		OPLDEBUG_LOGPRINT("[OPL]送信操作ログ保存(キー押下ログ)[%d-%d]\n", ul_SendOpeLogID, us_localNo,0,0,0,0);
		e_result = _putKeyPushLogToSendOpeLog( us_localNo, p_data );	// キー押下ログを送信操作ログDBに登録.
		if( OPLE_RsltStatComplete != e_result ){	// 登録失敗.
			return e_result;
		}
		p_RecordList = reinterpret_cast<SYSS_RecordListHeader *>(SYSF_lstNext( &p_RecordList->Node ));	// 次のリストデータポインタを取得.
	}
	_deleteListRecordsAtDB(_pPushKeyLogDB,&c_List);	// 使用したリストを開放する.

	*us_StartNum = us_localNo;

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：キー押下ログレコード保存処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_putKeyPushLogToSendOpeLog( ushort us_StartNum, OPLS_KeyPushLog* ps_data )
{
	OPLS_SendOpeLog s_SendOpeLog = {};
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	
	char sc_KeyNameStr[OPLD_KeyNameStrMax + 1];
	memset(sc_KeyNameStr, 0x00, sizeof(sc_KeyNameStr));

	memset( &s_SendOpeLog, 0x00, sizeof(OPLS_SendOpeLog) );
	s_SendOpeLog.ul_SendOpeLogID = ps_data->ul_SendOpeLogID;
	s_SendOpeLog.us_LogNo = us_StartNum;
	memcpy( s_SendOpeLog.sc_EventTimeStr, ps_data->sc_EventTimeStr, OPLD_TimeStrMax );
	s_SendOpeLog.e_EventStatusCode = ps_data->e_EventStatusCode;
	s_SendOpeLog.us_EventSubDataID = ps_data->us_EventSubDataID;

	if( (OPLE_EventDataKindStrId == ps_data->e_EventDataKind)				// イベント種別が文字列ID.
	|| (OPLE_EventDataKindStrIdStrChar == ps_data->e_EventDataKind) ) {		// または文字列ID＋文字列.
		s_SendOpeLog.us_EventDataID = ps_data->us_EventDataID;
	}
	
	if( OPLE_EventDataKindStrChar == ps_data->e_EventDataKind ) {				// イベント種別が文字列.
		
		e_result = _getPushKeyName( s_SendOpeLog.ul_SendOpeLogID, s_SendOpeLog.us_LogNo, sc_KeyNameStr, sizeof(sc_KeyNameStr));	// キー押下ログ（キー名称）DBからキー名称を取得.
		if( OPLE_RsltStatComplete != e_result ){	// 取得失敗.
			return e_result;
		}
		
		memcpy(s_SendOpeLog.sc_EventDataStr, sc_KeyNameStr,sizeof(sc_KeyNameStr));	// キー名称を送信操作ログDB構造体にセット.
	}

	if( (OPLE_EventDataKindStrIdStrChar == ps_data->e_EventDataKind) ||
	 	(OPLE_SubInfoKeyPosition == ps_data->us_EventSubDataID) ) {
		memcpy(s_SendOpeLog.sc_EventSubDataStr, ps_data->sc_EventSubDataStr,sizeof(s_SendOpeLog.sc_EventSubDataStr));
	}

	e_result = _addRecordAtSendOpeLogDB( &s_SendOpeLog, ps_data->ul_SendOpeLogID );			// 送信操作ログDBに登録.
	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：キー名称取得処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_getPushKeyName( uint32_t ul_Key, ushort us_LogNo, char* pc_KeyName, int i_size )
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	LIST c_List = {};
	SYSS_RecordListHeader* p_RecordList = NULL;
	SYSS_RecordListHeader* p_tmp = NULL;
	OPLS_PushKeyNameLog* p_data = NULL;
	int i_listCnt = 0;

	SYSF_lstInit(&c_List);
	_getPatternRecordAtDB(_pPushKeyNameLogDB, ul_Key, us_LogNo, &c_List, &i_listCnt );	// キー押下ログ（キー名称）DBから検索.

	if( 0 >= i_listCnt ) {	// 該当レコードなし.
		e_result = OPLE_RsltStatError;
		return e_result;
	}

	p_RecordList = reinterpret_cast<SYSS_RecordListHeader*>(SYSF_lstFirst( &c_List ));	// リストの先頭データポインタを取得.
	if( p_RecordList != NULL ){
		p_tmp = p_RecordList;
		p_data = reinterpret_cast<OPLS_PushKeyNameLog*>(++p_tmp);
		memcpy( pc_KeyName,  p_data->sc_EventDataStr, i_size );	// キー名称をコピー.
	}

	_deleteListRecordsAtDB(_pPushKeyNameLogDB,&c_List);	// 使用したリストを開放する.

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：宛先情報ログレコード変換処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_convertAdrInfoLogToSendOpeLog( uint32_t ul_SendOpeLogID, ushort* us_StartNum )
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	LIST c_List = {};
	SYSS_RecordListHeader* p_RecordList = NULL;
	SYSS_RecordListHeader* p_tmp = NULL;
	OPLS_AdrInfoLog* p_data = NULL;
	ushort us_localNo = *us_StartNum;
	TYPE_NetworkSpecific e_NetworkSpecific = TYPE_NetworkSpecific_NW1; // 2017/10/10 RQ-686 複合機の複数ネットワーク対応.
	
	SYSF_lstInit(&c_List);
	
	e_result = _getRecordListByKeyAtDB(_pAdrInfoLogDB, &c_List, ul_SendOpeLogID );	// 宛先情報ログDBレコードを取得.
	if( OPLE_RsltStatComplete != e_result ){	// 取得失敗.
		return e_result;
	}

	p_RecordList = reinterpret_cast<SYSS_RecordListHeader*>(SYSF_lstFirst( &c_List ));	// リストの先頭データポインタを取得.
	while( NULL!=p_RecordList ){
		p_tmp = p_RecordList;
		p_data = reinterpret_cast<OPLS_AdrInfoLog*>(++p_tmp);

// 2017/10/10 RQ-686 複合機の複数ネットワーク対応 Start.
		// ネットワーク判定.
		switch(p_data->e_AdrKind){
			case OPLE_AdrInfoDirectAddress:
			case OPLE_AdrInfoShorteningAddress:
				e_NetworkSpecific = TYPE_NetworkSpecific_NW1;
				break;
			case OPLE_AdrInfoDirectAddressNW2:
			case OPLE_AdrInfoShorteningAddressNW2:
				e_NetworkSpecific = TYPE_NetworkSpecific_NW2;
				break;
			default:
				break;
		}
// 2017/10/10 RQ-686 複合機の複数ネットワーク対応 End.

		us_localNo = static_cast<ushort>(us_localNo + 1);
		OPLDEBUG_LOGPRINT("[OPL]送信操作ログ保存(宛先種別)[%d-%d]\n", ul_SendOpeLogID, us_localNo,0,0,0,0);
		e_result = _putAdrInfoLog_SendMode( us_localNo, p_data,e_NetworkSpecific );	// 宛先種別を送信操作ログDBに登録.
		if( OPLE_RsltStatComplete != e_result ){	// 登録失敗.
			break;
		}

		if( OPLE_AdrInfoShorteningAddress == p_data->e_AdrKind || // 短縮宛先の場合.
			OPLE_AdrInfoShorteningAddressNW2 == p_data->e_AdrKind) { // 2017/10/10 RQ-686 複合機の複数ネットワーク対応.
			if( 0 < p_data->ss_ShorteningAddressNo ) {
				us_localNo = static_cast<ushort>(us_localNo + 1);
				OPLDEBUG_LOGPRINT("[OPL]送信操作ログ保存(短縮宛先登録番号)[%d-%d]\n", ul_SendOpeLogID, us_localNo,0,0,0,0);
				e_result = _putAdrInfoLog_AddressNo( us_localNo, p_data,e_NetworkSpecific );	// 登録番号を送信操作ログDBに登録.
				if( OPLE_RsltStatComplete != e_result ){	// 登録失敗.
				break;
				}
			}
		}

		if( 0 != strlen(p_data->sc_AddressName) ) {
			us_localNo = static_cast<ushort>(us_localNo + 1);
			OPLDEBUG_LOGPRINT("[OPL]送信操作ログ保存(登録名)[%d-%d]\n", ul_SendOpeLogID, us_localNo,0,0,0,0);
			e_result = _putAdrInfoLog_AddressName( us_localNo, p_data,e_NetworkSpecific );	// 登録名を送信操作ログDBに登録.
			if( OPLE_RsltStatComplete != e_result ){	// 登録失敗.
				break;
			}
		}

		if( 0 != strlen(p_data->sc_Address) ) {
			us_localNo = static_cast<ushort>(us_localNo + 1);
			OPLDEBUG_LOGPRINT("[OPL]送信操作ログ保存(アドレス)[%d-%d]\n", ul_SendOpeLogID, us_localNo,0,0,0,0);
			e_result = _putAdrInfoLog_Address( us_localNo, p_data,e_NetworkSpecific );	// アドレスを送信操作ログDBに登録.
			if( OPLE_RsltStatComplete != e_result ){	// 登録失敗.
				break;
			}
		}

		if( 0 != strlen(p_data->sc_Directory) ) {
			us_localNo = static_cast<ushort>(us_localNo + 1);
			OPLDEBUG_LOGPRINT("[OPL]送信操作ログ保存(ディレクトリ)[%d-%d]\n", ul_SendOpeLogID, us_localNo,0,0,0,0);
			e_result = _putAdrInfoLog_Directory( us_localNo, p_data,e_NetworkSpecific );	// ディレクトリを送信操作ログDBに登録する.
			if( OPLE_RsltStatComplete != e_result ){	// 登録失敗.
				break;
			}
		}

		p_RecordList = reinterpret_cast<SYSS_RecordListHeader *>(SYSF_lstNext( &p_RecordList->Node ));	// 次のリストデータポインタを取得.
	}

	if( ( *us_StartNum - 1 ) < us_localNo ) {
		*us_StartNum = us_localNo;
	}
	
	_deleteListRecordsAtDB(_pAdrInfoLogDB,&c_List);	// 使用したリストを開放する.
	
	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：宛先種別保存処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_putAdrInfoLog_SendMode( ushort us_StartNum, OPLS_AdrInfoLog* ps_data,TYPE_NetworkSpecific e_NetworkSpecific )
{
	OPLS_SendOpeLog s_SendOpeLog = {};
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;

	memset( &s_SendOpeLog, 0x00, sizeof(OPLS_SendOpeLog) );
	
	// 送信操作ログDB構造体にセット.
	s_SendOpeLog.ul_SendOpeLogID = ps_data->ul_SendOpeLogID;
	s_SendOpeLog.us_LogNo = us_StartNum;
	memcpy( s_SendOpeLog.sc_EventTimeStr, ps_data->sc_EventTimeStr, OPLD_TimeStrMax );
// 2017/10/10 RQ-686 複合機の複数ネットワーク対応 Start.
//	s_SendOpeLog.e_EventStatusCode = OPLE_EvtStatAdrInfo;
	if(e_NetworkSpecific == TYPE_NetworkSpecific_NW1){
		s_SendOpeLog.e_EventStatusCode = OPLE_EvtStatAdrInfo;
	}else{
		s_SendOpeLog.e_EventStatusCode = OPLE_EvtStatAdrInfoNW2;
	}
// 2017/10/10 RQ-686 複合機の複数ネットワーク対応 End.
	s_SendOpeLog.us_EventDataID = _convertSendModeToStringID( ps_data->e_SendMode );	// 宛先種別IDを文字列IDに変換.
	
	e_result = _addRecordAtSendOpeLogDB( &s_SendOpeLog, ps_data->ul_SendOpeLogID );			// 送信操作ログDBに登録.
	
	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：宛先情報ログレコード変換処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
ushort OPLC_Database::_convertSendModeToStringID( TYPE_SendMode e_SendMode )
{
	TEXT_ID textID = 0;
	
	OPLDebug_LogPrint("_convertSendModeToStringID() e_SendMode =%d\n", e_SendMode,0,0,0,0,0);
	switch( e_SendMode ) {
		case TYPE_SendModeFax:			textID = s_Fax;							break;	// ファクス.
		case TYPE_SendModeIfax:			textID = s_REPNoteIFaxX5Y1;				break;	// I-Fax.
		case TYPE_SendModeEmail:		textID = s_UtilityAddressRegScanEmail;	break;	// E-mail.
		case TYPE_SendModeFtp:			textID = s_AddressResistFTP;			break;	// FTP.
		case TYPE_SendModeSmb:			textID = s_ITMSmb;						break;	// SMB.
		case TYPE_SendModeBox:			textID = s_UtilityAutoResetSystemBox;	break;	// ボックス.
		case TYPE_SendModeIPAddressFax:	textID = s_ITMIPAddressFaxX21Y1;		break;	// IPアドレスFAX.
		case TYPE_SendModeWebDav:		textID = s_KEYListPrintWebDAVFileTX;	break;	// WebDAV.
		case TYPE_SendModeUsbMem:		textID = s_ITMUsbMemorySave;			break;	// ＵＳＢメモリ.
// 2019.02.01 RQ-272(EagleH) S.
		case TYPE_SendModeIPFax:		textID = s_pITMIPFaxT38_W074H016A;		break;	// IP-FaxT.38.
// 2019.02.01 RQ-272(EagleH) E.
		default:	break;
	}
	return textID;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：状態情報ログレコード変換処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
ushort OPLC_Database::_convertStatToStringID( OPLE_ChangeStatus e_ChangeStatus )
{
	TEXT_ID textID = 0;

	switch( e_ChangeStatus ){
		case OPLE_ChngStatPowerOn:			textID = s_REPMainPowerOn;			break;	// 起動.
		case OPLE_ChngStatLogin:			textID = s_REPLogin;				break;	// ログイン.
		case OPLE_ChngStatLogout:			textID = s_REPLogOut;				break;	// ログアウト.
		case OPLE_ChngStatAdfSet:			textID = s_REPADFOriginalSetting;	break;	// ADF原稿セット.
		case OPLE_ChngStatAutoResetScanfax:	textID = s_REPFaxScanAutoReset;		break;	// (スキャン/ファックス)オートリセット.
		case OPLE_ChngStatAutoResetBox:		textID = s_REPCopyAutoReset;		break;	// (ボックス)オートリセット.
		case OPLE_ChngStatLowPower:			textID = s_REPLowPower;				break;	// 低電力.
		case OPLE_ChngStatSleep:			textID = s_REPPowerSave;			break;	// 省電力.
		default:	break;
	}
	return textID;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：登録番号保存処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_putAdrInfoLog_AddressNo( ushort us_StartNum, OPLS_AdrInfoLog* ps_data,TYPE_NetworkSpecific e_NetworkSpecific )
{
	OPLS_SendOpeLog s_SendOpeLog = {};
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	
	memset( &s_SendOpeLog, 0x00, sizeof(OPLS_SendOpeLog) );
	
	// 送信操作ログDB構造体にセット.
	s_SendOpeLog.ul_SendOpeLogID = ps_data->ul_SendOpeLogID;
	s_SendOpeLog.us_LogNo = us_StartNum;
	memcpy( s_SendOpeLog.sc_EventTimeStr, ps_data->sc_EventTimeStr, OPLD_TimeStrMax );
// 2017/10/10 RQ-686 複合機の複数ネットワーク対応 Start.
//	s_SendOpeLog.e_EventStatusCode = OPLE_EvtStatAdrInfo;
	if(e_NetworkSpecific == TYPE_NetworkSpecific_NW1){
		s_SendOpeLog.e_EventStatusCode = OPLE_EvtStatAdrInfo;
	}else{
		s_SendOpeLog.e_EventStatusCode = OPLE_EvtStatAdrInfoNW2;
	}
// 2017/10/10 RQ-686 複合機の複数ネットワーク対応 End.
	sprintf( s_SendOpeLog.sc_EventDataStr, "%d", ps_data->ss_ShorteningAddressNo );
	
	e_result = _addRecordAtSendOpeLogDB( &s_SendOpeLog, ps_data->ul_SendOpeLogID );			// 送信操作ログDBに登録.
	
	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：登録名保存処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_putAdrInfoLog_AddressName( ushort us_StartNum, OPLS_AdrInfoLog* ps_data,TYPE_NetworkSpecific e_NetworkSpecific )
{
	OPLS_SendOpeLog s_SendOpeLog = {};
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	
	memset( &s_SendOpeLog, 0x00, sizeof(OPLS_SendOpeLog) );
	
	// 送信操作ログDB構造体にセット.
	s_SendOpeLog.ul_SendOpeLogID = ps_data->ul_SendOpeLogID;
	s_SendOpeLog.us_LogNo = us_StartNum;
	memcpy( s_SendOpeLog.sc_EventTimeStr, ps_data->sc_EventTimeStr, OPLD_TimeStrMax );
// 2017/10/10 RQ-686 複合機の複数ネットワーク対応 Start.
//	s_SendOpeLog.e_EventStatusCode = OPLE_EvtStatAdrInfo;
	if(e_NetworkSpecific == TYPE_NetworkSpecific_NW1){
		s_SendOpeLog.e_EventStatusCode = OPLE_EvtStatAdrInfo;
	}else{
		s_SendOpeLog.e_EventStatusCode = OPLE_EvtStatAdrInfoNW2;
	}
// 2017/10/10 RQ-686 複合機の複数ネットワーク対応 End.
	sprintf( s_SendOpeLog.sc_EventDataStr, "%s", ps_data->sc_AddressName );
	
	e_result = _addRecordAtSendOpeLogDB( &s_SendOpeLog, ps_data->ul_SendOpeLogID );			// 送信操作ログDBに登録.
	
	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：アドレス保存処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_putAdrInfoLog_Address( ushort us_StartNum, OPLS_AdrInfoLog* ps_data,TYPE_NetworkSpecific e_NetworkSpecific )
{
	OPLS_SendOpeLog s_SendOpeLog = {};
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	
	memset( &s_SendOpeLog, 0x00, sizeof(OPLS_SendOpeLog) );
	
	// 送信操作ログDB構造体にセット.
	s_SendOpeLog.ul_SendOpeLogID = ps_data->ul_SendOpeLogID;
	s_SendOpeLog.us_LogNo = us_StartNum;
	memcpy( s_SendOpeLog.sc_EventTimeStr, ps_data->sc_EventTimeStr, OPLD_TimeStrMax );
// 2017/10/10 RQ-686 複合機の複数ネットワーク対応 Start.
//	s_SendOpeLog.e_EventStatusCode = OPLE_EvtStatAdrInfo;
	if(e_NetworkSpecific == TYPE_NetworkSpecific_NW1){
		s_SendOpeLog.e_EventStatusCode = OPLE_EvtStatAdrInfo;
	}else{
		s_SendOpeLog.e_EventStatusCode = OPLE_EvtStatAdrInfoNW2;
	}
// 2017/10/10 RQ-686 複合機の複数ネットワーク対応 End.
	sprintf( s_SendOpeLog.sc_EventDataStr, "%s", ps_data->sc_Address );
	
	e_result = _addRecordAtSendOpeLogDB( &s_SendOpeLog, ps_data->ul_SendOpeLogID );			// 送信操作ログDBに登録.
	
	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：ディレクトリ保存処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_putAdrInfoLog_Directory( ushort us_StartNum, OPLS_AdrInfoLog* ps_data,TYPE_NetworkSpecific e_NetworkSpecific )
{
	OPLS_SendOpeLog s_SendOpeLog = {};
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	
	memset( &s_SendOpeLog, 0x00, sizeof(OPLS_SendOpeLog) );
	
	// 送信操作ログDB構造体にセット.
	s_SendOpeLog.ul_SendOpeLogID = ps_data->ul_SendOpeLogID;
	s_SendOpeLog.us_LogNo = us_StartNum;
	memcpy( s_SendOpeLog.sc_EventTimeStr, ps_data->sc_EventTimeStr, OPLD_TimeStrMax );
// 2017/10/10 RQ-686 複合機の複数ネットワーク対応 Start.
//	s_SendOpeLog.e_EventStatusCode = OPLE_EvtStatAdrInfo;
	if(e_NetworkSpecific == TYPE_NetworkSpecific_NW1){
		s_SendOpeLog.e_EventStatusCode = OPLE_EvtStatAdrInfo;
	}else{
		s_SendOpeLog.e_EventStatusCode = OPLE_EvtStatAdrInfoNW2;
	}
// 2017/10/10 RQ-686 複合機の複数ネットワーク対応 End.
	sprintf( s_SendOpeLog.sc_EventDataStr, "%s", ps_data->sc_Directory );
	
	e_result = _addRecordAtSendOpeLogDB( &s_SendOpeLog, ps_data->ul_SendOpeLogID );			// 送信操作ログDBに登録.
	
	return e_result;
}

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■.
//■■■　　出力処理.
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■.

OPLE_ResultStatus OPLC_Database::_creatLog( TYPC_Time cl_StartTime, TYPC_Time cl_EndTime )
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	uint32_t ul_firstKey = 0;
	uint32_t ul_startKey = 0;
	uint32_t ul_endKey = 0;
	uint32_t ul_firstIndex  = 0;
	uint32_t ul_firstOffset = 0;
	uint32_t ul_startIndex  = 0;
	uint32_t ul_startOffset = 0;
	TYPC_Time c_firstTime;
	TYPC_Time c_startTime;

	OPLDEBUG_LOGPRINT("[OPL]★★★★★★★★★★★★★★★★★★★★★★★★★★★\n", 0,0,0,0,0,0);
	OPLDEBUG_LOGPRINT("[OPL]★★★★★★★★送信操作ログ出力  開始★★★★★★★★\n", 0,0,0,0,0,0);
	OPLDEBUG_LOGPRINT("[OPL]★★★★★★★★★★★★★★★★★★★★★★★★★★★\n", 0,0,0,0,0,0);
	OPLDEBUG_LOGPRINT("[OPL]送信操作ログ出力 開始時間[%04d/%02d/%02d %02d]\n", cl_StartTime.ss_Year,cl_StartTime.sc_Month,cl_StartTime.sc_Day,cl_StartTime.sc_Hour,0,0);
	OPLDEBUG_LOGPRINT("[OPL]送信操作ログ出力 終了時間[%04d/%02d/%02d %02d]\n", cl_EndTime.ss_Year,cl_EndTime.sc_Month,cl_EndTime.sc_Day,cl_EndTime.sc_Hour,0,0);

	_deleteLog();	// ログ保存領域をクリアする.

	e_result = _getOldestKeyAtDB( &ul_firstKey, &c_firstTime, &ul_firstIndex, &ul_firstOffset);	// 最も古いキーを取得する.
	if( OPLE_RsltStatComplete != e_result ){											// 取得失敗.
		return e_result;
	}
	if( 0 == ul_firstKey ) {															// データがない場合.
		return e_result;																// 正常終了.
	}

	OPLDEBUG_LOGPRINT("[OPL]送信操作ログ出力 最古の送信操作ID[%d-%d-%d]\n", ul_firstKey,ul_firstIndex,ul_firstOffset,0,0,0);

	if( 0 >= cl_StartTime.ss_Year
	 || 0 >= cl_StartTime.sc_Month
	 || 0 >= cl_StartTime.sc_Day ) {													// 開始年月日のいずれかが０の場合.
		ul_startKey    = ul_firstKey;													// 開始キーを最も古いキーとする.
		c_startTime    = c_firstTime;
		ul_startIndex  = ul_firstIndex;
		ul_startOffset = ul_firstOffset;
	} else {
		e_result = _searchStartRecord( ul_firstKey, c_firstTime, ul_firstIndex, ul_firstOffset, cl_StartTime, cl_EndTime, &ul_startKey, &c_startTime, &ul_startIndex, &ul_startOffset);	// 開始年月日のいずれも０でない場合、開始キーを検索.
		if( OPLE_RsltStatComplete != e_result ){										// 検索失敗.
			return e_result;
		}
		if( 0 == ul_startKey ) {														// 取得範囲内に開始キーがない場合.
			OPLDEBUG_LOGPRINT("[OPL]送信操作ログ出力 該当レコード無\n", 0,0,0,0,0,0);
			return e_result;															// レコード０で処理を終了する.
		}
	}

	OPLDEBUG_LOGPRINT("[OPL]送信操作ログ出力 開始送信操作ID[%d-%d-%d]\n", ul_startKey,ul_startIndex,ul_startOffset,0,0,0);

	if( 0 >= cl_EndTime.ss_Year
	 || 0 >= cl_EndTime.sc_Month
	 || 0 >= cl_EndTime.sc_Day ) {
		ul_endKey = 0;
	} else {
		e_result = _searchEndRecord( ul_startKey, c_startTime, ul_startIndex, ul_startOffset, cl_EndTime, &ul_endKey );
		if( OPLE_RsltStatComplete != e_result ){										// 取得失敗.
			return e_result;
		}
		if( 0 == ul_endKey ) {															// 取得範囲内に終了キーがない場合.
			OPLDEBUG_LOGPRINT("[OPL]送信操作ログ出力 該当レコード無\n", 0,0,0,0,0,0);
			return e_result;															// レコード０で処理を終了する.
		}
	}

	OPLDEBUG_LOGPRINT("[OPL]送信操作ログ出力 終了送信操作ID[%d]\n", ul_endKey,0,0,0,0,0);

	e_result = _getOutRecord( ul_startKey, ul_startIndex, ul_startOffset, ul_endKey, cl_StartTime, cl_EndTime );	// 出力データを取得.

	OPLDEBUG_LOGPRINT("[OPL]★★★★★★★★★★★★★★★★★★★★★★★★★★★\n", 0,0,0,0,0,0);
	OPLDEBUG_LOGPRINT("[OPL]★★★★★★★★送信操作ログ出力  終了★★★★★★★★\n", 0,0,0,0,0,0);
	OPLDEBUG_LOGPRINT("[OPL]★★★★★★★★★★★★★★★★★★★★★★★★★★★\n", 0,0,0,0,0,0);

	return e_result;
}

OPLE_ResultStatus OPLC_Database::_searchStartRecord(uint32_t _ul_PreKey, TYPC_Time c_PreTime, uint32_t _ul_PreIndex, uint32_t _ul_PreOffset, TYPC_Time _cl_StartTime, TYPC_Time _cl_EndTime, uint32_t* _pul_StartKey, TYPC_Time* _pc_StartTime, uint32_t* _pul_StartIndex, uint32_t* _pul_StartOffset)
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	uint32_t ul_nextKey = _ul_PreKey;
	TYPC_Time c_Time = c_PreTime;
	uint32_t ul_index   = _ul_PreIndex;
	uint32_t ul_offset  = _ul_PreOffset;

	if( 0 >= _ul_PreKey ) {
		e_result = OPLE_RsltStatError;
		return e_result;
	}
	if( OPLE_SendOpeLogDBMax <= _ul_PreIndex ) {
		e_result = OPLE_RsltStatError;
		return e_result;
	}
	if( OPLE_SendOpeLogRecordMax <= _ul_PreOffset ) {
		e_result = OPLE_RsltStatError;
		return e_result;
	}

	if( 0 >= _cl_StartTime.ss_Year
	 || 0 >= _cl_StartTime.sc_Month
	 || 0 >= _cl_StartTime.sc_Day ) {
		e_result = OPLE_RsltStatError;
		return e_result;
	}

	if( NULL == _pul_StartKey ) {
		e_result = OPLE_RsltStatError;
		return e_result;
	}
	if( NULL == _pc_StartTime ) {
		e_result = OPLE_RsltStatError;
		return e_result;
	}
	if( NULL == _pul_StartIndex ) {
		e_result = OPLE_RsltStatError;
		return e_result;
	}
	if( NULL == _pul_StartOffset ) {
		e_result = OPLE_RsltStatError;
		return e_result;
	}

	OPLDEBUG_LOGPRINT("[OPL]開始操作ID検索 開始\n", 0,0,0,0,0,0);

	for ( ; ; ) {
		OPLDEBUG_LOGPRINT("[OPL]操作ID      [%d]\n", ul_nextKey,0,0,0,0,0);
		OPLDEBUG_LOGPRINT("[OPL]イベント時間[%04d/%02d/%02d %02d:%02d:%02d]\n",  c_Time.ss_Year,c_Time.sc_Month,c_Time.sc_Day,c_Time.sc_Hour,c_Time.sc_Minute,c_Time.sc_Second);
		if ( 0 == _checkTimeInKeyRecord(c_Time, _cl_StartTime, _cl_EndTime) ) {	// 取得範囲内の場合、前のキーを開始キーとする（最古キーの場合は、最古キーを開始キーとする）.
			*_pul_StartKey    = _ul_PreKey;
			*_pc_StartTime    = c_Time;
			*_pul_StartIndex  = ul_index;
			*_pul_StartOffset = ul_offset;
			break;
		}

		_ul_PreKey = ul_nextKey;
		ul_nextKey = 0;
		if( OPLE_RsltStatError == ( _getNextKeyAtDB( _ul_PreKey, ul_index, ul_offset, &ul_nextKey, &c_Time, &ul_index, &ul_offset) ) ) {	// 次キーの取得.
			e_result = OPLE_RsltStatError;
			*_pul_StartKey = 0;
			break;
		}
		if( 0 == ul_nextKey ) {			// 次キーがない場合.
			*_pul_StartKey = 0;
			break;
		}

	}

	OPLDEBUG_LOGPRINT("[OPL]開始操作ID検索 終了\n", 0,0,0,0,0,0);

	return e_result;
}

OPLE_ResultStatus OPLC_Database::_searchEndRecord( uint32_t _ul_PreKey, TYPC_Time c_PreTime, uint32_t _ul_PreIndex, uint32_t _ul_PreOffset, TYPC_Time _cl_EndTime, uint32_t* _pul_EndKey )
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	uint32_t ul_nextKey = _ul_PreKey;
	TYPC_Time c_Time = c_PreTime;
	uint32_t ul_index = _ul_PreIndex;
	uint32_t ul_offset = _ul_PreOffset;

	if( 0 >= _ul_PreKey ) {
		e_result = OPLE_RsltStatError;
		return e_result;
	}
	if( OPLE_SendOpeLogDBMax <= _ul_PreIndex ) {
		e_result = OPLE_RsltStatError;
		return e_result;
	}
	if( OPLE_SendOpeLogRecordMax <= _ul_PreOffset ) {
		e_result = OPLE_RsltStatError;
		return e_result;
	}

	if( 0 >= _cl_EndTime.ss_Year
	 || 0 >= _cl_EndTime.sc_Month
	 || 0 >= _cl_EndTime.sc_Day ) {
		e_result = OPLE_RsltStatError;
		return e_result;
	}

	if( NULL == _pul_EndKey ) {
		e_result = OPLE_RsltStatError;
		return e_result;
	}

	OPLDEBUG_LOGPRINT("[OPL]終了操作ID検索 開始\n", 0,0,0,0,0,0);

	*_pul_EndKey = 0;
	for ( ; ; ) {
		OPLDEBUG_LOGPRINT("[OPL]操作ID      [%d]\n", ul_nextKey,0,0,0,0,0);
		OPLDEBUG_LOGPRINT("[OPL]イベント時間[%04d/%02d/%02d %02d:%02d:%02d]\n",  c_Time.ss_Year,c_Time.sc_Month,c_Time.sc_Day,c_Time.sc_Hour,c_Time.sc_Minute,c_Time.sc_Second);
		if ( 0 >= _compareTime(c_Time, _cl_EndTime) ) {				// 終了年月日時と同じ、または、小さい（古い）場合、キーを保存する.
			*_pul_EndKey = ul_nextKey;
		} else {													// 終了年月日時を越えたので終了.
			break;
		}

		_ul_PreKey = ul_nextKey;
		ul_nextKey = 0;
		if( OPLE_RsltStatError == ( _getNextKeyAtDB( _ul_PreKey, ul_index, ul_offset, &ul_nextKey, &c_Time, &ul_index, &ul_offset) ) ) {	// 次キーの取得.
			e_result = OPLE_RsltStatError;
			*_pul_EndKey = 0;
			break;
		}
		if( 0 == ul_nextKey ) {										// 次キーがない場合.
			break;
		}
	}

	OPLDEBUG_LOGPRINT("[OPL]終了操作ID検索 終了\n", 0,0,0,0,0,0);

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：時刻比較処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
int OPLC_Database::_checkTimeInKeyRecord( TYPC_Time c_Time, TYPC_Time cl_StartTime, TYPC_Time cl_EndTime )
{

	Bool b_checkStartTime = True;
	Bool b_checkEndTime = True;
	int i_checkStartTimeResult = 0;
	int i_checkEndTimeResult = 0;


	if( 0 >= cl_StartTime.ss_Year
	 || 0 >= cl_StartTime.sc_Month
	 || 0 >= cl_StartTime.sc_Day ) {
		b_checkStartTime = False;
	}

	if( 0 >= cl_EndTime.ss_Year
	 || 0 >= cl_EndTime.sc_Month
	 || 0 >= cl_EndTime.sc_Day ) {
		b_checkEndTime = False;
	}

	if( True == b_checkStartTime ) {
		i_checkStartTimeResult = _compareTime(c_Time, cl_StartTime);
	}

	if( True == b_checkEndTime ) {
		i_checkEndTimeResult = _compareTime(c_Time, cl_EndTime);
	}

	if( 0 > i_checkStartTimeResult ) {				// 開始年月日時より小さい（古い）場合.
		return -1;									// 取得範囲より古い.
	} else if ( 0 < i_checkEndTimeResult ) {		// 終了年月日時より大きい（新しい）場合.
		return 1;									// 取得範囲より新しい.
	} else {										// 開始年月日時～終了年月日時の間の場合.
		return 0;									// 取得範囲内.
	}

	return 0;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：時刻比較処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
int OPLC_Database::_compareTime( TYPC_Time c_Time, TYPC_Time cl_BaseTime )
{
	int i_result = 0;
	TYPC_Time c_dataTime;
	TYPC_Time c_inputTime;

	// 送信操作ログの時刻データを分、秒、ミリ秒を０にしてセット.
	c_dataTime.ss_Year   = c_Time.ss_Year;
	c_dataTime.sc_Month  = c_Time.sc_Month;
	c_dataTime.sc_Day    = c_Time.sc_Day;
	c_dataTime.sc_Hour   = c_Time.sc_Hour;
	c_dataTime.sc_Minute = 0;
	c_dataTime.sc_Second = 0;
	
	// 引数の時刻を分、秒、ミリ秒を０にしてセット.
	c_inputTime.ss_Year   = cl_BaseTime.ss_Year;
	c_inputTime.sc_Month  = cl_BaseTime.sc_Month;
	c_inputTime.sc_Day    = cl_BaseTime.sc_Day;
	c_inputTime.sc_Hour   = cl_BaseTime.sc_Hour;
	c_inputTime.sc_Minute = 0;
	c_inputTime.sc_Second = 0;

	if (c_dataTime > c_inputTime ) {	// 送信操作ログの時刻データが大きい場合.
		i_result = 1;
	} else if (c_dataTime < c_inputTime ) {	// 送信操作ログの時刻データが小さい場合.
		i_result = -1;
	}
	else {
		// no operation.
		DBGF_PrintNoOperationPath(__FILE__, __LINE__);
	}

	return i_result;
}

int OPLC_Database::_compareTime( OPLS_SendOpeLog* ps_Data, TYPC_Time cl_Time )
{
	int i_result = 0;
	TYPC_Time c_dataTime;
	TYPC_Time c_inputTime;
	char sc_Year[5];			// 年.
	char sc_Month[3];		// 月.
	char sc_Day[3];			// 日.
	char sc_hour[3];			// 時.

	memset(sc_Year, 0x00, sizeof(sc_Year));
	memcpy(sc_Year,  &ps_Data->sc_EventTimeStr[0], 4);
	memset(sc_Month, 0x00, sizeof(sc_Month));
	memcpy(sc_Month, &ps_Data->sc_EventTimeStr[4], 2);
	memset(sc_Day, 0x00, sizeof(sc_Day));
	memcpy(sc_Day,   &ps_Data->sc_EventTimeStr[6], 2);
	memset(sc_hour, 0x00, sizeof(sc_hour));
	memcpy(sc_hour,  &ps_Data->sc_EventTimeStr[8], 2);

	// 送信操作ログの時刻データを分、秒、ミリ秒を０にしてセット.
	c_dataTime.ss_Year   = static_cast<short>(atoi(sc_Year));
	c_dataTime.sc_Month  = static_cast<char>(atoi(sc_Month));
	c_dataTime.sc_Day    = static_cast<char>(atoi(sc_Day));
	c_dataTime.sc_Hour   = static_cast<char>(atoi(sc_hour));
	c_dataTime.sc_Minute = 0;
	c_dataTime.sc_Second = 0;
	
	// 引数の時刻を分、秒、ミリ秒を０にしてセット.
	c_inputTime.ss_Year   = cl_Time.ss_Year;
	c_inputTime.sc_Month  = cl_Time.sc_Month;
	c_inputTime.sc_Day    = cl_Time.sc_Day;
	c_inputTime.sc_Hour   = cl_Time.sc_Hour;
	c_inputTime.sc_Minute = 0;
	c_inputTime.sc_Second = 0;

	if ( c_dataTime > c_inputTime ) {	// 送信操作ログの時刻データが大きい場合.
		i_result = 1;
	} else if ( c_dataTime < c_inputTime ) {	// 送信操作ログの時刻データが小さい場合.
		i_result = -1;
	}
	else {
		// no operation.
		DBGF_PrintNoOperationPath(__FILE__, __LINE__);
	}

	return i_result;
}

OPLE_ResultStatus OPLC_Database::_getOutRecord( uint32_t ul_StartKey, uint32_t ul_StartIndex, uint32_t _ul_StartOffset, uint32_t ul_EndKey, TYPC_Time cl_StartTime, TYPC_Time cl_EndTime )
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;
	LIST c_List = {};
	SYSS_RecordListHeader* p_RecordList = NULL;
	SYSS_RecordListHeader* p_tmp = NULL;
	OPLS_SendOpeLog* p_data = NULL;
	uint32_t ul_key = ul_StartKey;
	uint32_t ul_index = ul_StartIndex;
	uint32_t ul_offset = _ul_StartOffset;
	uint32_t ul_next_index = 0;
	uint32_t ul_temp_key = 0;
	bool b_end = false;
	TYPC_Time c_Time;

	Bool b_findFirstData = False;
	Bool b_findEndData = False;
// 2016/11/14 IT5_4.1 制御構成変更_No.41 Boxフル状態再現対応 Start.
#ifdef FUM_LINUX
	int32_t si32_Count01 = 0;
	int32_t si32_Count02 = 0;
	int32_t si32_Count03 = 0;
#endif // FUM_LINUX
// 2016/11/14 IT5_4.1 制御構成変更_No.41 Boxフル状態再現対応 End.

	if( 0 >= ul_StartKey ) {
		e_result = OPLE_RsltStatError;
		return e_result;
	}
	if( OPLE_SendOpeLogDBMax <= ul_StartIndex ) {
		e_result = OPLE_RsltStatError;
		return e_result;
	}
	if( OPLE_SendOpeLogRecordMax <= _ul_StartOffset ) {
		e_result = OPLE_RsltStatError;
		return e_result;
	}

	OPLDEBUG_LOGPRINT("[OPL]出力対象レコード取得 開始\n", 0,0,0,0,0,0);
// 2016/11/14 IT5_4.1 制御構成変更_No.41 Boxフル状態再現対応 Start.
#ifdef FUM_LINUX
	si32_Count01 = 0;
#endif // FUM_LINUX
// 2016/11/14 IT5_4.1 制御構成変更_No.41 Boxフル状態再現対応 End.
	for ( ; ; ) {																					

		if( (0 != ul_EndKey) 																		// 引数の終了キーが０でなく.
		&& (ul_key == ul_EndKey) ) {																// かつ、現在のキーと同じ場合.
			b_findEndData = True;																	// 終了レコード検索対象フラグをセット.
		}
// 2016/11/14 IT5_4.1 制御構成変更_No.41 Boxフル状態再現対応 Start.
#ifdef FUM_LINUX
		si32_Count02 = 0;
#endif // FUM_LINUX
// 2016/11/14 IT5_4.1 制御構成変更_No.41 Boxフル状態再現対応 End.
		for ( ; ; ) {																				// 操作ID単位のループ処理.

			SYSF_lstInit(&c_List);
			e_result = _getRecordListByKeyAtDB(_pSendOpeLogDB[ul_index], &c_List, ul_key );			// 開始キーのレコードリストを取得.

			p_RecordList = reinterpret_cast<SYSS_RecordListHeader*>(SYSF_lstFirst( &c_List ));						// リストの先頭データポインタを取得.
// 2016/11/14 IT5_4.1 制御構成変更_No.41 Boxフル状態再現対応 Start.
#ifdef FUM_LINUX
			si32_Count03 = 0;
#endif // FUM_LINUX
// 2016/11/14 IT5_4.1 制御構成変更_No.41 Boxフル状態再現対応 End.
			while( NULL!=p_RecordList ){																	// 送信操作ログDB単位の処理ループ.

				p_tmp = p_RecordList;
				p_data = reinterpret_cast<OPLS_SendOpeLog*>(++p_tmp);

				if ( False == b_findFirstData ) {													// 先頭レコード検索中.
					if( OPLE_EvtStatAdrInfo == p_data->e_EventStatusCode ||							// 宛先情報の場合、次のレコードをチェックする.
						OPLE_EvtStatAdrInfoNW2 == p_data->e_EventStatusCode) { // 2017/10/10 RQ-686 複合機の複数ネットワーク対応.
						p_RecordList = reinterpret_cast<SYSS_RecordListHeader *>(SYSF_lstNext( &p_RecordList->Node ));// 次のリストデータポインタを取得.
						continue;
					}
					if ( 0 > _compareTime(p_data, cl_StartTime) ) {									// 開始年月日時より小さい（古い）場合、次のレコードをチェックする.
						p_RecordList = reinterpret_cast<SYSS_RecordListHeader *>(SYSF_lstNext( &p_RecordList->Node ));// 次のリストデータポインタを取得.
						continue;
					}
					b_findFirstData = True;
				}

				if ( True == b_findEndData ) {														// 最終レコード検索中.
					if( OPLE_EvtStatAdrInfo != p_data->e_EventStatusCode &&							// 宛先情報でない場合.
						OPLE_EvtStatAdrInfoNW2 != p_data->e_EventStatusCode) { // 2017/10/10 RQ-686 複合機の複数ネットワーク対応.
						if ( 0 < _compareTime(p_data, cl_EndTime) ) {								// 終了年月日時より大きい場合、処理を抜ける.
							b_end = True;
							break;
						}
					}
				}

				OPLDEBUG_LOGPRINT("[OPL]出力対象レコード ID:%d-%d EventCode:%d, EventTime:%s\n", p_data->ul_SendOpeLogID,p_data->us_LogNo,p_data->e_EventStatusCode,reinterpret_cast<uintptr_t>(p_data->sc_EventTimeStr),0,0);
				_setSendLogOutData(p_data);															// 取得データを出力用リストにセット.

				p_RecordList = reinterpret_cast<SYSS_RecordListHeader *>(SYSF_lstNext( &p_RecordList->Node ));		// 次のリストデータポインタを取得.
// 2016/11/14 IT5_4.1 制御構成変更_No.41 Boxフル状態再現対応 Start.
#ifdef FUM_LINUX
				si32_Count03++;
				if (0 == si32_Count03%100)
				{
					SYSF_taskDelay((SYSF_sysClkRateGet() * 100) / 1000);
				}
#endif // FUM_LINUX
// 2016/11/14 IT5_4.1 制御構成変更_No.41 Boxフル状態再現対応 End.
			}

			_deleteListRecordsAtDB( _pSendOpeLogDB[ul_index], &c_List );							// 使用したリストを開放.

			if (False != b_end) {
				// 最終の操作IDの最終レコードを処理した為、抜ける.
				break;
			}

			ul_next_index = _IncrementSendOpeLogDBIndex(ul_index);									// 次の送信操作ログDBのインデックス取得.

			// 次の送信操作ログDBの最初のレコードにul_keyのレコードがあるか確認.
			SYSF_lstInit(&c_List);																	// リスト初期化.
			_getListRecordsAtDB( _pSendOpeLogDB[ul_next_index], &c_List, 1, 0 );					// 最初のレコード取得.

			if (0 == c_List.count) {																// レコードがないとき.
				// 同じ操作IDで、次のDBに存在せずに、その次のDBに存在することはないので、レコードがない場合は、次の操作IDを検索する為、抜ける.
				break;
			}

			p_RecordList = reinterpret_cast<SYSS_RecordListHeader*>(SYSF_lstFirst( &c_List ));						// リストの先頭データポインタを取得する.
			p_tmp = p_RecordList;
			p_data = reinterpret_cast<OPLS_SendOpeLog*>(++p_tmp);
			ul_temp_key = p_data->ul_SendOpeLogID;
			_deleteListRecordsAtDB(_pSendOpeLogDB[ul_next_index], &c_List);							// 使用したリストを開放する.

			if (ul_temp_key != ul_key) {
				// 次のDBに同じ操作IDはないので、次の操作IDを検索する為、抜ける.
				break;
			}

			ul_index  = ul_next_index;
			ul_offset = 0;
// 2016/11/14 IT5_4.1 制御構成変更_No.41 Boxフル状態再現対応 Start.
#ifdef FUM_LINUX
			si32_Count02++;
			if (0 == si32_Count02%100)
			{
				SYSF_taskDelay((SYSF_sysClkRateGet() * 100) / 1000);
			}
#endif // FUM_LINUX
// 2016/11/14 IT5_4.1 制御構成変更_No.41 Boxフル状態再現対応 End.
		}

		if (False != b_end) {
			// 最終の操作IDの最終レコードを処理した為、抜ける.
			break;
		}

		if ( True == b_findEndData ) {
			// 最終の操作IDを処理した為、抜ける.
			break;
		}

		if( OPLE_RsltStatError == ( _getNextKeyAtDB( ul_key, ul_index, ul_offset, &ul_key, &c_Time, &ul_index, &ul_offset) ) ) {	// 次の操作IDの取得.
			e_result = OPLE_RsltStatError;
			break;
		}
		if( 0 == ul_key ) {																			// 次の操作IDがない場合.
			break;
		}
// 2016/11/14 IT5_4.1 制御構成変更_No.41 Boxフル状態再現対応 Start.
#ifdef FUM_LINUX
		si32_Count01++;
		if (0 == si32_Count01%100)
		{
			SYSF_taskDelay((SYSF_sysClkRateGet() * 100) / 1000);
		}
#endif // FUM_LINUX
// 2016/11/14 IT5_4.1 制御構成変更_No.41 Boxフル状態再現対応 End.
	}

	OPLDEBUG_LOGPRINT("[OPL]出力対象レコード取得 終了\n", 0,0,0,0,0,0);

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：出力対象データ作成.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
void OPLC_Database::_setSendLogOutData( OPLS_SendOpeLog* ps_data )
{
	uchar uc_dataStrBuf[OPLD_EventDataStrMax + 1];													// イベントデータ文字列コード変換用バッファ.
	uchar uc_subDataStrBuf[OPLD_SubInfoStrMax + 1];													// 補足情報文字列コード変換用バッファ.

	memset( uc_dataStrBuf, 0x00, sizeof(uc_dataStrBuf) );											// イベントデータ文字列コード変換用バッファをクリアする.
	memset( uc_subDataStrBuf, 0x00, sizeof(uc_subDataStrBuf) );										// 補足情報文字列コード変換用バッファをクリアする.

	OPLC_SendOpeLogOut* pc_logData = new OPLC_SendOpeLogOut;
	OPLS_SendOpeLogOutData* ps_Log = &pc_logData->s_OutData;

	TEXT_ID textID = 0;

	memcpy( ps_Log->sc_EventTimeStr, ps_data->sc_EventTimeStr, sizeof(ps_data->sc_EventTimeStr) );	// イベント発生時刻をセット.
	ps_Log->e_EventStatusCode = ps_data->e_EventStatusCode;	// イベントステータスコードをセット.
	
	if( 0 != ps_data->us_EventDataID ) {	// イベントデータがIDの場合.
		ps_Log->us_EventDataID = ps_data->us_EventDataID;											// イベントデータIDにセット.
		
		switch( ps_data->e_EventStatusCode ){
			case OPLE_EvtStatChngStat:			// 状態変化.
			case OPLE_EvtStatChngStatNW2:		// 状態変化. // 2017/10/10 RQ-686 複合機の複数ネットワーク対応.
				textID = _convertStatToStringID( ps_data->us_EventDataID );
				break;
			case OPLE_EvtStatKeyPush:			// キー押下.
			case OPLE_EvtStatViewPanel:			// 画面表示.
			case OPLE_EvtStatAdrInfo:			// 宛先情報.
			case OPLE_EvtStatAdrInfoClearAll:	// 全宛先解除.
// 2017/10/10 RQ-686 複合機の複数ネットワーク対応 Start.
			case OPLE_EvtStatKeyPushNW2:			// キー押下.
			case OPLE_EvtStatViewPanelNW2:			// 画面表示.
			case OPLE_EvtStatAdrInfoNW2:			// 宛先情報.
			case OPLE_EvtStatAdrInfoClearAllNW2:	// 全宛先解除.
// 2017/10/10 RQ-686 複合機の複数ネットワーク対応 End.
// 2018/03/14 AR.1032676 Start.
			case OPLE_EvtStatAdrInfoClearBox:		// 宛先解除(BOX).
			case OPLE_EvtStatAdrInfoClearScan:		// 宛先解除(SCAN).
			case OPLE_EvtStatAdrInfoClearFax:		// 宛先解除(FAX).
			case OPLE_EvtStatAdrInfoClearBoxNW2:	// 宛先解除(BOX)(NW2向け).
			case OPLE_EvtStatAdrInfoClearScanNW2:	// 宛先解除(SCAN)(NW2向け).
			case OPLE_EvtStatAdrInfoClearFaxNW2:	// 宛先解除(FAX)(NW2向け).
// 2018/03/14 AR.1032676 End.
			default:
				textID = ps_data->us_EventDataID;
				break;
		}
		strncpy( ps_Log->sc_EventDataStr, const_cast<char*>(reinterpret_cast<const char*>(getTextPartsPointer( textID ))), OPLD_EventDataStrMax );
	} else {	// イベントデータが文字列の場合.
		ps_Log->us_EventDataID = 0;																	// イベントデータIDに０をセット.
		memcpy( ps_Log->sc_EventDataStr, ps_data->sc_EventDataStr, sizeof(ps_data->sc_EventDataStr) );	// イベントデータ文字列にセット.
	}
// 2019.06.26 EagleH OP_BTS-7883 S
//	setupTextToNonNextLineString( reinterpret_cast<uchar*>(ps_Log->sc_EventDataStr), reinterpret_cast<uchar*>(ps_Log->sc_EventDataStr) );
	setupTextToNonNextLineString(ps_Log->sc_EventDataStr);
// 2019.06.26 EagleH OP_BTS-7883 E

	if ( OPLE_SubInfoKeyPosition == ps_data->us_EventSubDataID ) {									// 補足情報が"キー座標"の場合.
		ps_Log->us_EventSubDataID = ps_data->us_EventSubDataID;										// 補足情報IDにセット.
		LIBF_ConvertToUTF8( static_cast<LIBE_CharSet>(getCharSetForCAPI()), reinterpret_cast<char *>(ps_data->sc_EventSubDataStr), reinterpret_cast<char *>(uc_subDataStrBuf), OPLD_SubInfoStrMax );	// 補足情報文字列をUTF8に変換する.
		memcpy( ps_Log->sc_EventSubDataStr, uc_subDataStrBuf, OPLD_SubInfoStrMax );					// 補足情報文字列にセット.
	} else if ( OPLE_SubInfoAdrSelect == ps_data->us_EventSubDataID ) {								// 補足情報が"宛先選択"の場合.
		ps_Log->us_EventSubDataID = ps_data->us_EventSubDataID;										// 補足情報IDにセット.
		strncpy( ps_Log->sc_EventSubDataStr, const_cast<char*>(reinterpret_cast<const char*>(getTextPartsPointer( s_REPAddressSelect ))), OPLD_SubInfoStrMax );
	} else if ( OPLE_SubInfoAdrRelease == ps_data->us_EventSubDataID ) {							// 補足情報が"宛先解除"の場合.
		ps_Log->us_EventSubDataID = ps_data->us_EventSubDataID;										// 補足情報IDにセット.
		strncpy( ps_Log->sc_EventSubDataStr, const_cast<char*>(reinterpret_cast<const char*>(getTextPartsPointer( s_REPAddressClear ))), OPLD_SubInfoStrMax );
	} else if( 0 != ps_data->us_EventSubDataID ) {	// 補足情報がIDの場合.
		ps_Log->us_EventSubDataID = ps_data->us_EventSubDataID;										// 補足情報IDにセット.
		memset( ps_Log->sc_EventSubDataStr, 0x00, sizeof(ps_data->sc_EventSubDataStr) );			// 補足情報文字列をクリア.
	} else {	// 補足情報が文字列の場合.
		ps_Log->us_EventSubDataID = 0;																// 補足情報IDに０をセット.
		LIBF_ConvertToUTF8( static_cast<LIBE_CharSet>(getCharSetForCAPI()), reinterpret_cast<char *>(ps_data->sc_EventSubDataStr), reinterpret_cast<char *>(uc_subDataStrBuf), OPLD_SubInfoStrMax );	// 補足情報文字列をUTF8に変換する.
		memcpy( ps_Log->sc_EventSubDataStr, uc_subDataStrBuf, OPLD_SubInfoStrMax );					// 補足情報文字列にセット.
	}
	
// 2019.10.29 OP_BTS-13736 S
//	c_SendOpeLogOutList.add(pc_logData);
	c_SendOpeLogOutList.push_back(*pc_logData);
// 2019.10.29 OP_BTS-13736 E
// 2019.12.10 OP_BTS-17172 S
	// Listにはコピーコンストラクタされているのでここで開放する.
	delete pc_logData;
// 2019.12.10 OP_BTS-17172 E
	
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：シンボルTextから改行文字列を省いた文字列を取得.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
// 2019.06.26 EagleH OP_BTS-7883 S
//void OPLC_Database::setupTextToNonNextLineString( const uchar* puc_InputText, uchar* puc_OutputText )
//{
//	uchar* sc_StrBuf = NULL;
//	char sc_NextLine[ 2 ] = { 0x1f, 0x00 };
//
//	if( NULL==puc_InputText || NULL==puc_OutputText ){
//		return;
//	}
//
//	strcpy( reinterpret_cast<char*>(puc_OutputText), const_cast<char*>(reinterpret_cast<const char*>(puc_InputText)) );
//	sc_StrBuf = reinterpret_cast<uchar*>(strstr(reinterpret_cast<char*>(puc_OutputText), sc_NextLine));
//	while (NULL != sc_StrBuf)
//	{
//		strcpy( reinterpret_cast<char*>(sc_StrBuf), reinterpret_cast<char*>(sc_StrBuf + 1));
//		sc_StrBuf = reinterpret_cast<uchar*>(strstr(reinterpret_cast<char*>(puc_OutputText), sc_NextLine));
//	}
//}
void OPLC_Database::setupTextToNonNextLineString(char* puc_Text)
{
	int32_t cnt = 0;

	char* cp_StrPos = NULL;
	char sc_tmpOutput[OPLD_EventDataStrMax + 1] = {0};

	if(puc_Text == NULL){
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::setupTextToNonNextLineString() puc_Text is NULL \n",0,0,0,0,0,0 );
		return;
	}

	// ポジション退避.
	cp_StrPos = puc_Text;

	while ('\0' != *puc_Text){
		if(*puc_Text != 0x1f){
			sc_tmpOutput[cnt] = *puc_Text;
			if(cnt >= OPLD_EventDataStrMax){
				OPLD_LogMsg(0x03, "OPLC_Database::setupTextToNonNextLineString() cnt max\n",0,0,0,0,0,0 );
				break;
			}
			cnt++;
		}
		puc_Text++;
	}

	strncpy(cp_StrPos,sc_tmpOutput,OPLD_EventDataStrMax + 1);

	return;
}
// 2019.06.26 EagleH OP_BTS-7883 E

//関数----------------------------------------------------------------------------------------------.
//概要    ：送信操作ログ出力データ取得要求.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
// 2019.10.29 OP_BTS-13736 S
//short OPLC_Database::_getSendOpeLogData( LIBC_List<OPLC_SendOpeLogOut>* pc_SendOpeLogOutList, short ss_Start, short ss_Num )
int32_t OPLC_Database::_getSendOpeLogData( std::list<OPLC_SendOpeLogOut>& pc_SendOpeLogOutList, int32_t ss_Start, int32_t ss_Num )
// 2019.10.29 OP_BTS-13736 E
{

// 2019.10.29 OP_BTS-13736 S
//	short ss_listCount = 0;
//	short ss_recCount = 0;
//	OPLC_SendOpeLogOut* pc_SendOpeLogOut = NULL;
	int32_t ss_listCount = 0;
	int32_t ss_recCount = 0;
	std::list<OPLC_SendOpeLogOut>::iterator pc_SendOpeLogOut;
// 2019.10.29 OP_BTS-13736 E

	OPLD_LogMsg(0x03, "OPLC_Database::_getSendOpeLogData() start \n",0,0,0,0,0,0 );

// 2019.10.29 OP_BTS-13736 S
//	if( NULL == pc_SendOpeLogOutList ) {
//		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getSendOpeLogData() pc_SendOpeLogOutList NULL \n",0,0,0,0,0,0 );
//		return 0;
//	}
// 2019.10.29 OP_BTS-13736 E

// 2019.10.29 OP_BTS-13736 S
//	pc_SendOpeLogOutList->Lock(); 
//	while (pc_SendOpeLogOutList->getCount() > 0) {	// データが残っていた場合.
//		pc_SendOpeLogOutList->del(pc_SendOpeLogOutList->findFirst());	// データをクリア.
//	}
//	pc_SendOpeLogOutList->UnLock();
	// 全データクリア.
	pc_SendOpeLogOutList.clear();
// 2019.10.29 OP_BTS-13736 E
	if( 0 > ss_Start							// 開始位置が０未満.
	|| OPLD_SendOpeLogRecMax <= ss_Start ) {	// または、開始位置が送信操作ログMAX件数以上の場合.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getSendOpeLogData() Para error1 \n",0,0,0,0,0,0 );
		return 0;
	}
	
	if( 0 >= ss_Num							// 取得個数が０以下の場合.
	|| OPLD_SendOpeLogRecMax < ss_Num ) {	// または、取得個数が送信操作ログMAX件数より大きい場合.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getSendOpeLogData() Para error2 \n",0,0,0,0,0,0 );
		return 0;
	}

// 2019.10.29 OP_BTS-13736 S
//	c_SendOpeLogOutList.Lock();
//	ss_listCount = c_SendOpeLogOutList.getCount();			// 送信操作ログ出力用データ件数取得.
	ss_listCount = static_cast<int32_t>(c_SendOpeLogOutList.size());				// 送信操作ログ出力用データ件数取得.
	if( 0 >= ss_listCount ) {								// 取得個数が０以下の場合.
//		c_SendOpeLogOutList.UnLock(); // 2019.10.29 OP_BTS-13736.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getSendOpeLogData() size err ss_listCount = %d \n",ss_listCount,0,0,0,0,0 );
		return 0;
	} else if ( OPLD_SendOpeLogRecMax < ss_listCount ) {	// 取得個数が送信操作ログMAX件数以上の場合.
		ss_listCount = OPLD_SendOpeLogRecMax;
	}
	else {
		// no operation.
		DBGF_PrintNoOperationPath(__FILE__, __LINE__);
	}
// 2019.10.29 OP_BTS-13736 S
//	pc_SendOpeLogOut = c_SendOpeLogOutList.findFirst();	// 最初のノードを取得.
//	for (int i=0; i<ss_listCount; i++) {
//
//		if( 0 != i ) {
//			pc_SendOpeLogOut = c_SendOpeLogOutList.findNext();	// 次のノードを取得.
//			if( NULL == pc_SendOpeLogOut ) {
//				break;
//			}
//		}
//
//		if( ss_Start > i ) {	// 開始位置未満.
//			continue;
//		}
//		if( (i - (ss_Start-1)) > ss_Num ) {	// 取得個数より大きい.
//			break;
//		}
//
//		pc_SendOpeLogOutList->add(pc_SendOpeLogOut);	// リストに追加する.
//// 2016/11/14 IT5_4.1 制御構成変更_No.41 Boxフル状態再現対応 Start.
//#ifdef FUM_LINUX
//		if (0 == (i+1)%50)
//		{
//			SYSF_taskDelay((SYSF_sysClkRateGet() * 100) / 1000);
//		}
//#endif // FUM_LINUX
// 2016/11/14 IT5_4.1 制御構成変更_No.41 Boxフル状態再現対応 End.
//		ss_recCount = static_cast<short>(ss_recCount + 1);
//	}
//	c_SendOpeLogOutList.UnLock(); // 2019.10.29 OP_BTS-13736.
	for(pc_SendOpeLogOut = c_SendOpeLogOutList.begin();pc_SendOpeLogOut != c_SendOpeLogOutList.end();++pc_SendOpeLogOut){
		if( ss_Start > ss_recCount ) {	// 開始位置未満.
			ss_recCount++;
			continue;
		}
		if( (ss_recCount - (ss_Start-1)) > ss_Num ) {	// 取得個数より大きい.
			OPLD_LogMsg(0x03, "OPLC_Database::_getSendOpeLogData() Num over ss_Start[%d] ss_Num[%d] ss_recCount[%d]\n",ss_Start,ss_Num,ss_recCount,0,0,0 );
			break;
		}
		pc_SendOpeLogOutList.push_back(*pc_SendOpeLogOut);	// リストに追加する.
		ss_recCount++;
	}
// 2019.10.29 OP_BTS-13736 E

	OPLD_LogMsg(0x03, "OPLC_Database::_getSendOpeLogData() end [%d]\n",ss_recCount,0,0,0,0,0 );

	debugLog_SendOpeLogOut( pc_SendOpeLogOutList, ss_recCount ) ;

	return ss_recCount;

}

//関数----------------------------------------------------------------------------------------------.
//概要    ：送信操作ログ出力データ開放.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
void OPLC_Database::_deleteLog()
{
// 2019.10.29 OP_BTS-13736 S.
//	OPLC_SendOpeLogOut* pc_SendOpeLogOut = NULL;
//
//	c_SendOpeLogOutList.Lock();
//	while (c_SendOpeLogOutList.getCount() > 0) {
//		pc_SendOpeLogOut = c_SendOpeLogOutList.findFirst();	// 最初のノードを取得.
//		c_SendOpeLogOutList.del(pc_SendOpeLogOut);			// ノードを削除.
//		delete pc_SendOpeLogOut;
//	}
//	c_SendOpeLogOutList.UnLock();
// 2019.12.10 OP_BTS-17172 S
//	std::list<OPLC_SendOpeLogOut>::iterator pc_SendOpeLogOut;
//	for(pc_SendOpeLogOut = c_SendOpeLogOutList.begin();pc_SendOpeLogOut != c_SendOpeLogOutList.end();++pc_SendOpeLogOut){
//		c_SendOpeLogOutList.erase(pc_SendOpeLogOut);	// リストに追加する.
//		delete &(*pc_SendOpeLogOut);
//	}
	// 全削除を行う.
	c_SendOpeLogOutList.clear();
// 2019.12.10 OP_BTS-17172 E
// 2019.10.29 OP_BTS-13736 E

}

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//■■■　　その他
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

//関数----------------------------------------------------------------------------------------------.
//概要    ：送信操作ログDBにデータがあるかチェックする.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
Bool OPLC_Database::_chkSendOpeLogExist()
{
	Bool b_result = False;														// チェック結果（デフォルト：レコード無）.

	if( 0 < _getSendOpeLogDataCount() ) {										// 送信操作ログDBレコード件数が１以上の場合.
		b_result = True;														// チェック結果をレコード有とする.
	}

	return b_result;															// チェック結果を返す.
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：送信操作ログDBのレコード数を取得する.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
int32_t OPLC_Database::_getSendOpeLogDataCount()
{
	int32_t sl_cnt = 0;															// レコード数.
	int32_t sl_all_cnt = 0;														// 全レコード数.

	// 送信操作ログDBレコード数を取得する.
	for (int i = 0; i < OPLE_SendOpeLogDBMax; i++) {
		if( OPLE_RsltStatComplete != _getAllRecordCountAtDB( _pSendOpeLogDB[i], &sl_cnt ) ) {	// 取得失敗の場合.
			sl_cnt = 0;															// レコード数に０をセットする.
		}
		sl_all_cnt += sl_cnt;
	}
	return sl_all_cnt;															// 全レコード数を返す.
}

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■
//■■■　　共通処理
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■

//関数----------------------------------------------------------------------------------------------.
//概要    ：送信操作ログ管理次ステータス取得処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_MngOfSendOpeLogStatus OPLC_Database::_getMngOfSendOpeLogNextStatus( OPLE_MngOfSendOpeLogStatus e_Status, Bool b_IsAdrinfo, Bool b_IsError )
{
	OPLE_MngOfSendOpeLogStatus e_nextStatus = e_Status;
	
	switch( e_Status ) {
	case	OPLE_MngStatIDSet :													// 操作ID採番.
		e_nextStatus = OPLE_MngStatSaveKeyPush;									// →　キー押下ログ保存中.
		break;
	case	OPLE_MngStatSaveKeyPush :											// キー押下ログ保存中.
		if( True == b_IsError ) {												// エラー有の場合.
			e_nextStatus = OPLE_MngStatSaveKeyPushErr;							// →　キー押下ログ保存エラー.
		} else if ( True == b_IsAdrinfo ) {										// エラー無で宛先情報有の場合.
			e_nextStatus = OPLE_MngStatSaveAdrInfo;								// →　宛先情報ログ保存中（宛先情報ログがある場合のキー押下ログ保存完了）.
		} else {																// エラー無、宛先情報無の場合.
			e_nextStatus = OPLE_MngStatSaveKeyPushEnd;							// →　宛先情報がない場合のキー押下ログ保存完了.
		}
		break;
	case	OPLE_MngStatSaveKeyPushErr :										// キー押下ログ保存エラー.
		e_nextStatus = OPLE_MngStatSaveKeyPush;									// →　キー押下ログ保存中.
		break;
	case	OPLE_MngStatSaveKeyPushEnd :										// 宛先情報がない場合のキー押下ログ保存完了.
	case	OPLE_MngStatSaveAdrInfoEnd :										// 宛先情報ログ保存完了.
		e_nextStatus = OPLE_MngStatSaveSendOpeLog;								// →　送信操作ログ保存中.
		break;
	case	OPLE_MngStatSaveAdrInfo :											// 宛先情報ログ保存中（宛先情報ログがある場合のキー押下ログ保存完了）.
		if( True == b_IsError ) {												// エラー有の場合.
			e_nextStatus = OPLE_MngStatSaveAdrInfoErr;							// →　宛先情報ログ保存エラー.
		} else {																// エラー無の場合.
			e_nextStatus = OPLE_MngStatSaveAdrInfoEnd;							// →　宛先情報ログ保存完了.
		}
		break;
	case	OPLE_MngStatSaveAdrInfoErr :										// 宛先情報ログ保存エラー.
		e_nextStatus = OPLE_MngStatErrDelKeyAndAdrStart;						// →　宛先情報ログ保存エラー時のキー押下ログ、宛先情報ログ削除.
		break;
	case	OPLE_MngStatSaveSendOpeLog :										// 送信操作ログ保存中.
		if( True == b_IsError ) {												// エラー有の場合.
			e_nextStatus = OPLE_MngStatSaveSendOpeLogErr;						// →　送信操作ログ保存エラー.
		} else {																// エラー無の場合.
			e_nextStatus = OPLE_MngStatSaveSendOpeLogEnd;						// →　送信操作ログ保存完了.
		}
		break;
	case	OPLE_MngStatSaveSendOpeLogErr :										// 送信操作ログ保存エラー.
		e_nextStatus = OPLE_MngStatDelSendOpeLog;								// →　送信操作ログ削除中.
		break;
	case	OPLE_MngStatSaveSendOpeLogEnd :										// 送信操作ログ保存完了.
		if ( True == b_IsAdrinfo ) {											// 宛先情報有の場合.
			e_nextStatus = OPLE_MngStatDelKeyAndAdrStart;						// →　送信操作ログ保存完了によるキー押下ログ、宛先情報ログ削除.
		} else {																// 宛先情報無の場合.
			e_nextStatus = OPLE_MngStatDelKeyPushStart;							// →　送信操作ログ保存完了によるキー押下ログ削除.
		}
		break;
	case	OPLE_MngStatErrDelKeyAndAdrStart :									// 宛先情報ログ保存エラー時のキー押下ログ、宛先情報ログ削除.
		e_nextStatus = OPLE_MngStatErrDelKeyAndAdr;								// →　宛先情報ログ保存エラー時のキー押下ログ削除中.
		break;
	case	OPLE_MngStatErrDelKeyPushStart :									// キー押下ログ保存エラー時のキー押下ログ削除.
		e_nextStatus = OPLE_MngStatErrDelKeyPush;								// →　キー押下ログ保存エラー時のキー押下ログ削除中.
		break;
	case	OPLE_MngStatDelKeyAndAdrStart :										// 送信操作ログ保存完了によるキー押下ログ、宛先情報ログ削除.
		e_nextStatus = OPLE_MngStatDelKeyAndAdr;								// →　送信操作ログ保存完了によるキー押下ログ削除中（宛先情報あり）.
		break;
	case	OPLE_MngStatDelKeyPushStart :										// 送信操作ログ保存完了によるキー押下ログ削除.
		e_nextStatus = OPLE_MngStatDelKeyPush;									// →　送信操作ログ保存完了によるキー押下ログ削除中（宛先情報なし）.
		break;
	case	OPLE_MngStatErrDelKeyAndAdr :										// 宛先情報ログ保存エラー時のキー押下ログ削除中.
		if( True == b_IsError ) {												// エラー有の場合.
			e_nextStatus = OPLE_MngStatErrDelKeyAndAdrStart;					// →　宛先情報ログ保存エラー時のキー押下ログ、宛先情報ログ削除.
		} else {																// エラー無の場合.
			e_nextStatus = OPLE_MngStatErrDelAdrInfo;							// →　宛先情報ログ保存エラー時の宛先情報ログ削除中.
		}
		break;
	case	OPLE_MngStatErrDelKeyPush :											// キー押下ログ保存エラー時のキー押下ログ削除中.
		if( True == b_IsError ) {												// エラー有の場合.
			e_nextStatus = OPLE_MngStatErrDelKeyPushStart;						// →　キー押下ログ保存エラー時のキー押下ログ削除.
		} else {																// エラー無の場合.
			e_nextStatus = OPLE_MngStatAllComplete;								// →　送信操作ログ管理DB削除（送信操作ログ処理完了）.
		}
		break;
	case	OPLE_MngStatDelKeyAndAdr :											// 送信操作ログ保存完了によるキー押下ログ削除中（宛先情報あり）.
		if( True == b_IsError ) {												// エラー有の場合.
			e_nextStatus = OPLE_MngStatDelKeyAndAdrStart;						// →　送信操作ログ保存完了によるキー押下ログ、宛先情報ログ削除.
		} else {																// エラー無の場合.
			e_nextStatus = OPLE_MngStatDelAdrInfo;								// →　送信操作ログ保存完了による宛先情報ログ削除中.
		}
		break;
	case	OPLE_MngStatDelKeyPush :											// 送信操作ログ保存完了によるキー押下ログ削除中（宛先情報なし）.
		if( True == b_IsError ) {												// エラー有の場合.
			e_nextStatus = OPLE_MngStatDelKeyPushStart;							// →　送信操作ログ保存完了によるキー押下ログ削除.
		} else {																// エラー無の場合.
			e_nextStatus = OPLE_MngStatAllComplete;								// →　送信操作ログ管理DB削除（送信操作ログ処理完了）.
		}
		break;
	case	OPLE_MngStatErrDelKeyPushEnd :										// 宛先情報ログ保存エラー時のキー押下ログ削除完了.
		e_nextStatus = OPLE_MngStatErrDelAdrInfo;								// →　宛先情報ログ保存エラー時の宛先情報ログ削除中.
		break;
	case	OPLE_MngStatDelKeyPushEnd :											// 送信操作ログ保存完了によるキー押下ログ削除完了（宛先情報あり）.
		e_nextStatus = OPLE_MngStatDelAdrInfo;									// →　送信操作ログ保存完了による宛先情報ログ削除中.
		break;
	case	OPLE_MngStatErrDelAdrInfoStart :									// 宛先情報ログ保存エラー時の宛先情報ログ削除.
		e_nextStatus = OPLE_MngStatErrDelAdrInfo;								// →　宛先情報ログ保存エラー時の宛先情報ログ削除中.
		break;
	case	OPLE_MngStatDelAdrInfoStart :										// 送信操作ログ保存完了による宛先情報ログ削除.
		e_nextStatus = OPLE_MngStatDelAdrInfo;									// →　送信操作ログ保存完了による宛先情報ログ削除中.
		break;
	case	OPLE_MngStatErrDelAdrInfo :											// 宛先情報ログ保存エラー時の宛先情報ログ削除中.
		if( True == b_IsError ) {												// エラー有の場合.
			e_nextStatus = OPLE_MngStatErrDelAdrInfoStart;						// →　宛先情報ログ保存エラー時の宛先情報ログ削除.
		} else {																// エラー無の場合.
			e_nextStatus = OPLE_MngStatAllComplete;								// →　送信操作ログ管理DB削除（送信操作ログ処理完了）.
		}
		break;
	case	OPLE_MngStatDelAdrInfo :											// 送信操作ログ保存完了による宛先情報ログ削除中.
		if( True == b_IsError ) {												// エラー有の場合.
			e_nextStatus = OPLE_MngStatDelAdrInfoStart;							// →　送信操作ログ保存完了による宛先情報ログ削除.
		} else {																// エラー無の場合.
			e_nextStatus = OPLE_MngStatAllComplete;								// →　送信操作ログ管理DB削除（送信操作ログ処理完了）.
		}
		break;
	case	OPLE_MngStatErrDelAdrInfoEnd :										// 宛先情報ログ保存エラー時の宛先情報ログ削除完了.
	case	OPLE_MngStatDelAdrInfoEnd :											// 送信操作ログ保存完了による宛先情報ログ削除完了.
		e_nextStatus = OPLE_MngStatAllComplete;									// →　送信操作ログ管理DB削除（送信操作ログ処理完了）.
		break;
	case	OPLE_MngStatDelSendOpeLogStart :									// 送信操作ログ削除.
		e_nextStatus = OPLE_MngStatDelSendOpeLog;								// →　送信操作ログ削除中.
		break;
	case	OPLE_MngStatDelSendOpeLog :											// 送信操作ログ削除中.
		if( True == b_IsError ) {												// エラー有の場合.
			e_nextStatus = OPLE_MngStatDelSendOpeLogStart;						// →　送信操作ログ削除.
		} else {																// エラー無の場合.
			e_nextStatus = OPLE_MngStatDelSendOpeLogErr;						// →　送信操作ログ削除完了.
		}
		break;
	case	OPLE_MngStatDelSendOpeLogErr :										// 送信操作ログ削除完了.
		if ( True == b_IsAdrinfo ) {											// 宛先情報有の場合.
			e_nextStatus = OPLE_MngStatSaveAdrInfoEnd;							// →　宛先情報ログ保存完了.
		} else {																// 宛先情報無の場合.
			e_nextStatus = OPLE_MngStatSaveKeyPushEnd;							// →　宛先情報がない場合のキー押下ログ保存完了.
		}
		break;
	default :
		break;
	}
	
	return e_nextStatus;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：最も古いキーを取得する.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_getOldestKeyAtDB(uint32_t* _pul_SendOpeLogID, TYPC_Time* pc_EventTime, uint32_t* _pul_SendOpeLogDBIndex, uint32_t* _pul_SendOpeLogDBOffset)
{
	OPLE_ResultStatus e_result = OPLE_RsltStatError;							// 本処理結果.
	uint32_t ul_index = uc_SendOpeLogDBWriteIndex;
	int32_t sl_cnt = 0;
	LIST c_List = {};
	SYSS_RecordListHeader* p_RecordList = NULL;
	SYSS_RecordListHeader* p_tmp = NULL;
	OPLS_SendOpeLog* p_data = NULL;

	if ( NULL == _pul_SendOpeLogID ) {											// 格納先がNULLの場合.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getOldestKeyAtDB() Parameter(_pul_SendOpeLogID) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}
	if ( NULL == pc_EventTime ) {												// 格納先がNULLの場合.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getOldestKeyAtDB() Parameter(pc_EventTime) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}
	if ( NULL == _pul_SendOpeLogDBIndex ) {										// 格納先がNULLの場合.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getOldestKeyAtDB() Parameter(_pul_SendOpeLogDBIndex) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}
	if ( NULL == _pul_SendOpeLogDBOffset ) {									// 格納先がNULLの場合.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getOldestKeyAtDB() Parameter(_pul_SendOpeLogDBOffset) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	uint32_t sl_IndexBackup = ul_index;

	for ( ; ; ) {
		ul_index = _IncrementSendOpeLogDBIndex(ul_index);
		_getAllRecordCountAtDB( _pSendOpeLogDB[ul_index], &sl_cnt );
		if (0 != sl_cnt) {
			SYSF_lstInit(&c_List);												// リスト初期化.
			_getListRecordsAtDB( _pSendOpeLogDB[ul_index], &c_List, 1, 0 );		// 最初のレコード取得.
			p_RecordList = reinterpret_cast<SYSS_RecordListHeader*>(SYSF_lstFirst( &c_List ));	// リストの先頭データポインタを取得する.
			p_tmp = p_RecordList;
			p_data = reinterpret_cast<OPLS_SendOpeLog*>(++p_tmp);
			*_pul_SendOpeLogID = p_data->ul_SendOpeLogID;						// 最古の操作ID.
			_covertTime(p_data->sc_EventTimeStr, pc_EventTime);					// 文字列の時間からTYPEC_Timeへ変換.
			*_pul_SendOpeLogDBIndex  = ul_index;								// 送信操作ログDBのインデックス.
			*_pul_SendOpeLogDBOffset = 0;										// 送信操作ログDBのどの位置のレコードか.
			_deleteListRecordsAtDB(_pSendOpeLogDB[ul_index], &c_List);			// 使用したリストを開放する.
			break;
		}
		// 用意されているDBを一回りしたため、データ無しとして終了.
		if (sl_IndexBackup == ul_index)
		{
			SYSF_lstInit(&c_List);		// リスト初期化.
			*_pul_SendOpeLogID = 0;		// 操作ID初期化.
			break;
		}
	}

	return OPLE_RsltStatComplete;
}

OPLE_ResultStatus OPLC_Database::_getNextKeyAtDB(uint32_t ul_NowSendOpeLogID, uint32_t _ul_NowSendOpeLogDBIndex, uint32_t _ul_NowSendOpeLogDBOffset, uint32_t* _pul_NextSendOpeLogID, TYPC_Time* pc_NextEventTime, uint32_t* _pul_NextSendOpeLogDBIndex, uint32_t* _pul_NextSendOpeLogDBOffset)
{
	OPLE_ResultStatus e_result = OPLE_RsltStatError;							// 本処理結果.
	uint32_t ul_index = _ul_NowSendOpeLogDBIndex;
	uint32_t ul_offset = _ul_NowSendOpeLogDBOffset;
	Bool b_end = False;
	LIST c_List = {};
	SYSS_RecordListHeader* p_RecordList = NULL;
	SYSS_RecordListHeader* p_tmp = NULL;
	OPLS_SendOpeLog* p_data = NULL;

	if ( NULL == _pul_NextSendOpeLogID ) {										// 格納先がNULLの場合.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getNextKeyAtDB() Parameter(_pul_NextSendOpeLogID) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}
	if ( NULL == _pul_NextSendOpeLogDBIndex ) {									// 格納先がNULLの場合.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getNextKeyAtDB() Parameter(_pul_NextSendOpeLogDBIndex) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}
	if ( NULL == _pul_NextSendOpeLogDBOffset ) {								// 格納先がNULLの場合.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getNextKeyAtDB() Parameter(_pul_NextSendOpeLogDBOffset) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	for ( ; ; ) {
		SYSF_lstInit(&c_List);													// リスト初期化.
		_getListRecordsAtDB( _pSendOpeLogDB[ul_index], &c_List, OPLE_SendOpeLogRecordMax, ul_offset );	// オフセットから最終までのレコード取得.

		if (0 != c_List.count) {
			p_RecordList = reinterpret_cast<SYSS_RecordListHeader*>(SYSF_lstFirst( &c_List ));	// リストの先頭データポインタを取得.
			while( NULL!=p_RecordList ){
				p_tmp = p_RecordList;
				p_data = reinterpret_cast<OPLS_SendOpeLog*>(++p_tmp);
				if (p_data->ul_SendOpeLogID != ul_NowSendOpeLogID) {			// 次の操作IDがみつかった.
					e_result = OPLE_RsltStatComplete;
					*_pul_NextSendOpeLogID       = p_data->ul_SendOpeLogID;
					_covertTime(p_data->sc_EventTimeStr, pc_NextEventTime);		// 文字列の時間からTYPEC_Timeへ変換.
					*_pul_NextSendOpeLogDBIndex  = ul_index;
					*_pul_NextSendOpeLogDBOffset = ul_offset;
					OPLDEBUG_LOGPRINT("[OPL]次操作ID    [%d-%d-%d]\n", *_pul_NextSendOpeLogID,*_pul_NextSendOpeLogDBIndex,*_pul_NextSendOpeLogDBOffset,0,0,0);
					b_end = True;
					break;
				}
				p_RecordList = reinterpret_cast<SYSS_RecordListHeader *>(SYSF_lstNext( &p_RecordList->Node ));	// 次のキーからデータを取得.
				ul_offset++;
			}
			_deleteListRecordsAtDB(_pSendOpeLogDB[ul_index], &c_List);			// 使用したリストを開放する.
		}

		if (False != b_end) {
			break;																// 次の操作IDが見つかったので、処理終了.
		}

		if (uc_SendOpeLogDBWriteIndex == ul_index) {
			*_pul_NextSendOpeLogID = 0;
			e_result = OPLE_RsltStatComplete;
			break;																// 次の操作IDが見つからなかった(ul_NowSendOpeLogIDが最新の操作ID)ので、処理終了
		}

		ul_index = _IncrementSendOpeLogDBIndex(ul_index);
		ul_offset = 0;

	}

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：DBのレコード数を取得する.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_getAllRecordCountAtDB( SYSC_Database*	p_Database, int32_t* pl_Cnt)
{
	int32_t	sl_result = 0;														// SYSC_Database処理結果.
	OPLE_ResultStatus e_result = OPLE_RsltStatError;							// 本処理結果.

	if ( NULL == p_Database ) {													// DBインスタンスアドレスがNULLの場合.
		dbgOplCheckPoint=1;		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getAllRecordCountAtDB() Parameter(p_Database) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	if ( NULL == pl_Cnt ) {														// レコード件数格納先がNULLの場合.
		dbgOplCheckPoint=2;		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getAllRecordCountAtDB() Parameter(pl_Cnt) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	sl_result = p_Database->countRecords();										// DBレコード数を取得する.

	if ( sl_result >= 0 ) {														// レコード数が０以上の場合.
		*pl_Cnt = sl_result;													// 取得結果を引数のレコード数にセットする.
		e_result = OPLE_RsltStatComplete;										// 本処理結果に処理完了をセットする.
	} else {
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getAllRecordCountAtDB() p_Database->countRecords() Error(0x%x)%p\n",sl_result,p_Database,0,0,0,0 );
		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 Start.
		dbgOplCheckPoint=3;
		dbgOplErr = SYSF_errnoGet();
		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 End.
	}

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：指定された開始位置から指定された件数分レコードを取得する.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
int32_t OPLC_Database::_getListRecordsAtDB( SYSC_Database* p_Database, LIST* p_List, int32_t sl_Size, int32_t sl_Start )
{
	int32_t	sl_cnt = -1;														// 取得レコード件数.

	if ( NULL == p_Database ) {													// DBインスタンスアドレスがNULLの場合.
		dbgOplCheckPoint=4;		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getListRecordsAtDB() Parameter(p_Database) NULL\n",0,0,0,0,0,0 );
		return sl_cnt;															// 取得レコードなしで処理を終了する.
	}

	if ( NULL == p_List ) {														// レコード格納先がNULLの場合.
		dbgOplCheckPoint=5;		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getListRecordsAtDB() Parameter(p_List) NULL\n",0,0,0,0,0,0 );
		return sl_cnt;															// 取得レコードなしで処理を終了する.
	}

	if ( 0 >= sl_Size ) {														// 取得件数上限値が０以下の場合.
		dbgOplCheckPoint=6;		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getListRecordsAtDB() Parameter(sl_Size) NG(%d)\n",sl_Size,0,0,0,0,0 );
		return sl_cnt;															// 取得レコードなしで処理を終了する.
	}

	if ( 0 > sl_Start ) {														// 開始位置が０未満の場合.
		dbgOplCheckPoint=7;		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getListRecordsAtDB() Parameter(sl_Start) NG(%d)\n",sl_Start,0,0,0,0,0 );
		return sl_cnt;															// 取得レコードなしで処理を終了する.
	}

	sl_cnt = p_Database->listRecords( sl_Start, sl_Size, p_List );				// 開始位置から指定件数レコードを取得する.

	if ( ( sl_cnt < 0 ) || ( sl_cnt != p_List->count ) ) {						// 取得件数が０未満、または、取得レコードリストに取得件数分ない　場合.
		sl_cnt = -1;															// 取得件数を -1 （エラー）とする.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getListRecordsAtDB() p_Database->listRecords() Error sl_cnt(%d)\n",sl_cnt,0,0,0,0,0 );
		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 Start.
		dbgOplCheckPoint=8;
		dbgOplErr = SYSF_errnoGet();
		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 End.
	}
	
	return sl_cnt;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：DBのレコードリストを開放する.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_deleteListRecordsAtDB( SYSC_Database* p_Database, LIST* p_List )
{
	OPLE_ResultStatus e_result = OPLE_RsltStatError;							// 本処理結果.

	if ( NULL == p_Database ) {													// DBインスタンスアドレスがNULLの場合.
		dbgOplCheckPoint=9;		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_deleteListRecordsAtDB() Parameter(p_Database) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	if ( NULL == p_List || 0 >= SYSF_lstCount( p_List ) ) {						// 開放対象データがNULLの場合.
		e_result = OPLE_RsltStatComplete;										// 本処理結果に処理完了をセットする.
		return e_result;														// 処理を終了する.
	}

	p_Database->deleteList(p_List);												// DBからリスト取得したデータを開放する.

	e_result = OPLE_RsltStatComplete;											// 本処理結果に処理完了をセットする.

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：DBの全レコードを削除する.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_allDataClearAtDB( SYSC_Database* p_Database )
{
	int		i_result = 0;														// SYSC_Database処理結果.
	OPLE_ResultStatus e_result = OPLE_RsltStatError;							// 本処理結果.
	int32_t	sl_cnt = 0;

	if ( NULL == p_Database ) {													// DBインスタンスアドレスがNULLの場合.
		dbgOplCheckPoint=10;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_allDataClearAtDB() Parameter(p_Database) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	if( OPLE_RsltStatComplete != _getAllRecordCountAtDB( p_Database, &sl_cnt ) ) {	// DB内にレコードがあるかチェックする.
		dbgOplCheckPoint=11;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_allDataClearAtDB() _getAllRecordCountAtDB() Error\n",0,0,0,0,0,0 );
		return e_result;														// 取得失敗の場合、エラーで処理を終了する.
	}

	if( 0 == sl_cnt ) {															// 
		e_result = OPLE_RsltStatComplete;										// 本処理結果に処理完了をセットする.
		return e_result;
	}
	
	_lock();																	// セマフォロック（※削除中にDBアクセスさせない）.

	i_result = p_Database->deleteAllRecord();									// DBの全レコードを削除する.

	_unlock();																	// セマフォアンロック.

	if ( OK == i_result ) {														// 処理結果が追加成功の場合.
		e_result = OPLE_RsltStatComplete;										// 本処理結果に処理完了をセットする.
	} else {
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_allDataClearAtDB() p_Database->deleteAllRecord() Error\n",0,0,0,0,0,0 );
		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 Start.
		dbgOplCheckPoint=12;
		dbgOplErr = SYSF_errnoGet();
		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 End.
	}

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：DBに指定キーでレコードを追加する.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_setupDB( SYSC_Database* p_Database, SYSS_Config* p_Config )
{
	int i_result = 0;															// SYSC_Database処理結果.
	OPLE_ResultStatus e_result = OPLE_RsltStatError;							// 本処理結果.

	if ( NULL == p_Database ) {													// DBインスタンスアドレスがNULLの場合.
		dbgOplCheckPoint=13;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_setupDB() Parameter(p_Database) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	if ( NULL == p_Config ) {													// DB設定がNULLの場合.
		dbgOplCheckPoint=14;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_setupDB() Parameter(p_Config) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}


	i_result = p_Database->setup( p_Config );									// DBをセットアップする.

	if( ( OK == i_result ) || ( SYSD_EINIT == i_result ) ) {					// セットアップ完了の場合.
		e_result = OPLE_RsltStatComplete;											// 本処理結果に処理完了をセットする.
	} else {
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_setupDB() p_Database->setup() Error\n",0,0,0,0,0,0 );
		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 Start.
		dbgOplCheckPoint=15;
		dbgOplErr = SYSF_errnoGet();
		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 End.
	}

	return e_result;
}


//関数----------------------------------------------------------------------------------------------.
//概要    ：DBに指定キーでレコードを追加する.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_addRecordAtDB( SYSC_Database* p_Database, void* p_Data, uint32_t ul_SendOpeLogID )
{
	int32_t	sl_result = 0;														// SYSC_Database処理結果.
	OPLE_ResultStatus e_result = OPLE_RsltStatError;							// 本処理結果.
	uint32_t	ul_Key = 0UL;

	if ( NULL == p_Database ) {													// DBインスタンスアドレスがNULLの場合.
		dbgOplCheckPoint=16;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_addRecordAtDB() Parameter(p_Database) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	if ( NULL == p_Data ) {														// 追加データがNULLの場合.
		dbgOplCheckPoint=17;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_addRecordAtDB() Parameter(p_Data) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	if ( 0 == ul_SendOpeLogID ) {												// 追加キーが０の場合.
		dbgOplCheckPoint=18;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_addRecordAtDB() Parameter(ul_Key) 0\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	sl_result = p_Database->addRecord( p_Data, &ul_Key );						// DBに指定キーでデータを追加する.

	if ( FILD_OK == sl_result ) {												// 処理結果が追加成功の場合.
		e_result = OPLE_RsltStatComplete;										// 本処理結果に処理完了をセットする.
	} else {
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_addRecordAtDB() p_Database->addRecord() Error\n",0,0,0,0,0,0 );
		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 Start.
		dbgOplCheckPoint=19;
		dbgOplErr = SYSF_errnoGet();
		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 End.
	}

	return e_result;
}


//関数----------------------------------------------------------------------------------------------.
//概要    ：DBの指定キーのレコードを更新する.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_updateRecordByKeyAtDB( SYSC_Database* p_Database, void* p_Data, uint32_t ul_SendOpeLogID )
{
	int32_t	sl_result = 0;														// SYSC_Database処理結果.
	OPLE_ResultStatus e_result = OPLE_RsltStatError;							// 本処理結果.

	if ( NULL == p_Database ) {													// DBインスタンスアドレスがNULLの場合.
		dbgOplCheckPoint=20;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_updateRecordByKeyAtDB() Parameter(p_Database) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	if ( NULL == p_Data ) {														// 更新データがNULLの場合.
		dbgOplCheckPoint=21;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_updateRecordByKeyAtDB() Parameter(p_Data) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	if ( 0 == ul_SendOpeLogID ) {													// 更新キーが０の場合.
		dbgOplCheckPoint=22;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_updateRecordByKeyAtDB() Parameter(ul_Key) 0\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	if (_pMngOfSendOpeLogDB != p_Database) {
		dbgOplCheckPoint=23;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		// 操作IDだけでレコードが確定できるのは、_pMngOfSendOpeLogDBのみなので、終了する.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_updateRecordByKeyAtDB() Parameter(p_Database) _Not pMngOfSendOpeLogDB\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}
	OPLS_MngOfSendOpeLog s_Data = {};
	uint32_t ul_key = 0;
	STATUS status = p_Database->get1stKey( &ul_key );							// 先頭のレコードのKeyを取り出す.
	while( OK == status ) {
		sl_result = p_Database->retrieveRecord( ul_key, reinterpret_cast<void *>(&s_Data) );	// 指定キーでレコード取得.

		if(FILD_OK == sl_result) {												// 指定Keyのレコード取得が成功した時.
			if (ul_SendOpeLogID == s_Data.ul_SendOpeLogID) {					// 指定した操作IDと取得したレコードの操作IDが一致した時.
				sl_result = p_Database->updateRecord( ul_key, p_Data, True );	// DBの指定キーのデータを更新する.
				break;
			}
		} else {
			// レコード取得失敗.
			OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_updateRecordByKeyAtDB() p_Database->retrieveRecord() Error\n",0,0,0,0,0,0 );
			// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 Start.
			dbgOplCheckPoint=24;
			dbgOplErr = SYSF_errnoGet();
			// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 End.
			return e_result;													// 処理を終了する.
		}
		status = p_Database->getNextKey( ul_key , &ul_key );					//次のレコードのkeyを取り出す.
	}

	if ( FILD_OK == sl_result ) {												// 処理結果が更新成功の場合.
		e_result = OPLE_RsltStatComplete;										// 本処理結果に処理完了をセットする.
	} else {
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_updateRecordByKeyAtDB() p_Database->updateRecord() Error\n",0,0,0,0,0,0 );
		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 Start.
		dbgOplCheckPoint=25;
		dbgOplErr = SYSF_errnoGet();
		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 End.
	}

	return e_result;
}


//関数----------------------------------------------------------------------------------------------.
//概要    ：DBの指定キーのレコードをリストで取得する.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_getRecordListByKeyAtDB( SYSC_Database* p_Database, LIST* p_Data, uint32_t ul_SendOpeLogID )
{
	OPLE_ResultStatus e_result = OPLE_RsltStatError;							// 本処理結果.

	if ( NULL == p_Database ) {													// DBインスタンスアドレスがNULLの場合.
		dbgOplCheckPoint=26;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getRecordListByKeyAtDB() Parameter(p_Database) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	if ( NULL == p_Data ) {														// 取得データ格納先がNULLの場合.
		dbgOplCheckPoint=27;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getRecordListByKeyAtDB() Parameter(p_Data) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	if ( 0 == ul_SendOpeLogID ) {												// 取得キーが０の場合.
		dbgOplCheckPoint=28;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getRecordListByKeyAtDB() Parameter(ul_Key) 0\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	int i_offset    = 0;														// レコード内の検索先頭オフセット.
	int i_size      = 4;														// 比較するバイト数.
	int i_listCount = 0;														// 検索レコード件数.

	i_listCount = p_Database->listPatternRecords( i_offset, i_size, &ul_SendOpeLogID, p_Data );	// 指定操作IDを持つレコードを検索する.
	if (  0 > i_listCount ) {													// 取得件数が０未満の場合.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getRecordListByKeyAtDB() p_Database->listPatternRecords() Error\n",0,0,0,0,0,0 );
		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 Start.
		dbgOplCheckPoint=29;
		dbgOplErr = SYSF_errnoGet();
		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 End.
	} else {
		e_result = OPLE_RsltStatComplete;										// 本処理結果に処理完了をセットする.
	}
	
	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：DBの指定キーのレコードを削除する.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_deleteRecordByKeyAtDB( SYSC_Database* p_Database, uint32_t _ul_SendOpeLogID, uint32_t *_ul_delete_cnt)
{
	int32_t	sl_result = 0;														// SYSC_Database処理結果.
	OPLE_ResultStatus e_result = OPLE_RsltStatError;							// 本処理結果.

	if ( NULL == p_Database ) {													// DBインスタンスアドレスがNULLの場合.
		dbgOplCheckPoint=30;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_deleteRecordByKeyAtDB() Parameter(p_Database) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	if ( 0 == _ul_SendOpeLogID ) {												// 削除キーが０の場合.
		dbgOplCheckPoint=31;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_deleteRecordByKeyAtDB() Parameter(ul_Key) 0\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	LIST c_List = {};
	SYSS_RecordListHeader* p_RecordList = NULL;

	SYSF_lstInit(&c_List);

	e_result = _getRecordListByKeyAtDB(p_Database, &c_List, _ul_SendOpeLogID);				// 削除したい操作IDのレコードをリストで取得.
	if( OPLE_RsltStatComplete != e_result ){							// 取得失敗.
		return OPLE_RsltStatError;
	}
	if (NULL != _ul_delete_cnt) {
		*_ul_delete_cnt = c_List.count;
	}
	if (0 == c_List.count) {											// リストが0の時.
		return OPLE_RsltStatComplete;
	}

	p_RecordList = reinterpret_cast<SYSS_RecordListHeader*>(SYSF_lstFirst( &c_List ));			// リストの先頭データポインタを取得.
	while( NULL!=p_RecordList ){

		sl_result = p_Database->deleteRecord( p_RecordList->Key );				// DBの指定キーのデータを削除する.
		if ((FILD_OK != sl_result) && (FILD_ErrNotExist != sl_result)) {
			OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_deleteRecordByKeyAtDB() p_Database->deleteRecord() Error\n",0,0,0,0,0,0 );
			e_result = OPLE_RsltStatError;								// 処理を終了する.
			// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 Start.
			dbgOplCheckPoint=32;
			dbgOplErr = SYSF_errnoGet();
			// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 End.
			break;
		}
		p_RecordList = reinterpret_cast<SYSS_RecordListHeader *>(SYSF_lstNext( &p_RecordList->Node ));	// 次のキーからデータを取得.
	}

	_deleteListRecordsAtDB( p_Database, &c_List );								// 使用したリストを開放.

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：DBの指定操作IDと内部連番のレコードを取得する.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_getPatternRecordAtDB( SYSC_Database* p_Database, uint32_t ul_LogID, ushort us_LogNo, LIST* p_List, int* pi_ListCnt )
{
	OPLE_ResultStatus e_result = OPLE_RsltStatError;							// 本処理結果.
	int i_offset = 0;															// レコード内の検索先頭オフセット.
	int i_size = 6;																// 比較するバイト数(ul_SendOpeLogIDの4+us_LogNoの2).
	int i_listCount = 0;														// 検索レコード件数.

	if ( NULL == p_Database ) {													// DBインスタンスアドレスがNULLの場合.
		dbgOplCheckPoint=33;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getPatternRecordAtDB() Parameter(p_Database) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	if ( NULL == p_List ) {														// 取得データ格納先がNULLの場合.
		dbgOplCheckPoint=34;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getPatternRecordAtDB() Parameter(p_List) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	if ( NULL == pi_ListCnt ) {													// 取得データ件数格納先がNULLの場合.
		dbgOplCheckPoint=35;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getPatternRecordAtDB() Parameter(pi_ListCnt) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	if ( 0 == ul_LogID ) {														// 操作IDが０の場合.
		dbgOplCheckPoint=36;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getPatternRecordAtDB() Parameter(ul_LogID) 0\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	if ( 0 == us_LogNo ) {														// 操作ID内連番が０の場合.
		dbgOplCheckPoint=37;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getPatternRecordAtDB() Parameter(us_LogNo) 0\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}
	OPLS_SendOpeLogID s_ptrn = {};												// 検索するパターン.
	s_ptrn.ul_SendOpeLogID = ul_LogID;											// 検索パターンデータに操作IDをセットする.
	s_ptrn.us_LogNo = us_LogNo;													// 検索パターンデータに操作ID内連番をセットする.

	i_listCount = p_Database->listPatternRecords( i_offset, i_size, &s_ptrn, p_List );	// 指定操作IDと内部連番を持つレコードを検索する.

	if ( ( 0 > i_listCount ) || ( i_listCount != p_List->count ) ) {			// 取得件数が０未満、または、取得件数と取得レコードリストの数が一致しない　場合.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_getPatternRecordAtDB() p_Database->listPatternRecords() Error\n",0,0,0,0,0,0 );
		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 Start.
		dbgOplCheckPoint=38;
		dbgOplErr = SYSF_errnoGet();
		// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 End.
	} else {
		*pi_ListCnt = i_listCount;												// 取得レコード件数を引数にセットする.
		e_result = OPLE_RsltStatComplete;										// 本処理結果に処理完了をセットする.
	}

	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：DBの指定キーのレコードが存在するかチェックする.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_existRecordByKeyAtDB( SYSC_Database* p_Database, uint32_t ul_SendOpeLogID )
{
	OPLE_ResultStatus e_result = OPLE_RsltStatError;							// 本処理結果.

	if ( NULL == p_Database ) {													// DBインスタンスアドレスがNULLの場合.
		dbgOplCheckPoint=39;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_existRecordByKeyAtDB() Parameter(p_Database) NULL\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	if ( 0 == ul_SendOpeLogID ) {												// キーが０の場合.
		dbgOplCheckPoint=40;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_existRecordByKeyAtDB() Parameter(ul_Key) 0\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}

	if (_pMngOfSendOpeLogDB != p_Database) {
		dbgOplCheckPoint=41;	// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加.
		// 操作IDだけでレコードが確定できるのは、_pMngOfSendOpeLogDBのみなので、終了する.
		OPLD_LogMsg(0x03, "OPLC_Database::_existRecordByKeyAtDB() Parameter(p_Database) _Not pMngOfSendOpeLogDB\n",0,0,0,0,0,0 );
		return e_result;														// 処理を終了する.
	}
	int32_t	sl_result = 0;														// SYSC_Database処理結果.
	OPLS_MngOfSendOpeLog s_Data = {};
	uint32_t ul_key = 0;
	STATUS status = p_Database->get1stKey( &ul_key );							// 先頭のレコードのKeyを取り出す.
	while( OK == status ) {
		sl_result = p_Database->retrieveRecord( ul_key, reinterpret_cast<void *>(&s_Data) );	// 指定キーでレコード取得.

		if(FILD_OK == sl_result) {												// 指定Keyのレコード取得が成功した時.
			if (ul_SendOpeLogID == s_Data.ul_SendOpeLogID) {					// 指定した操作IDと取得したレコードの操作IDが一致した時.
				e_result = OPLE_RsltStatComplete;								// 本処理結果に処理完了をセットする.
				break;
			}
		} else {
			// レコード取得失敗.
			OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_existRecordByKeyAtDB() p_Database->retrieveRecord() Error\n",0,0,0,0,0,0 );
			// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 Start.
			dbgOplCheckPoint=42;
			dbgOplErr = SYSF_errnoGet();
			// 2020/11/11 OP_BTS-28976【IT6多発トラブル】E201対応 ログ追加 End.
			break;
		}
		status = p_Database->getNextKey( ul_key , &ul_key );					//次のレコードのkeyを取り出す.
	}


	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：文字列IDを文字列に変換する.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
void OPLC_Database::_convertIDToStringAtDB( const ushort us_TextID, uchar* puc_String, int i_Size )
{

	if ( 0 == us_TextID ) {														// 文字列IDが０の場合.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_existRecordByKeyAtDB() Parameter(us_TextID) 0\n",0,0,0,0,0,0 );
		return ;																// 処理を終了する.
	}

	if ( NULL == puc_String ) {													// 文字列格納先がNULLの場合.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_existRecordByKeyAtDB() Parameter(puc_String) NULL\n",0,0,0,0,0,0 );
		return ;																// 処理を終了する.
	}

	if ( 0 >= i_Size ) {														// 文字列サイズが０以下の場合.
		OPLD_ErrMsg(0xFF,0x03, "OPLC_Database::_existRecordByKeyAtDB() Parameter(i_Size) NG(%d)\n",i_Size,0,0,0,0,0 );
		return ;																// 処理を終了する.
	}

		LIBF_ConvertToUTF8( static_cast<LIBE_CharSet>(getCharSetForCAPI()), const_cast<char *>(reinterpret_cast<const char *>(getTextPartsPointer( us_TextID ))), reinterpret_cast<char *>(puc_String), i_Size );	// 文字列IDを文字列に変換する.
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：制御コードを除いた文字列をコピーする.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
short OPLC_Database::_strncpyClearCtrCode( uchar* puc_Dst, uchar* puc_Src, short ss_Size )
{
	short ss_cnt = 0;
	uchar* puc_DstWork = puc_Dst;
	uchar* puc_SrcWork = puc_Src;
	
	// 引数チェック.
	if( NULL == puc_Dst || NULL == puc_Src ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );	
		return 0;
	}

	// 終端文字、または用意されたバッファサイズ分コピー処理する.
	// ただしコントロール文字の場合はコピーしない.
	for(;;) {
		if( '\0' == *puc_SrcWork || ss_cnt >= ss_Size ) {
			break;
		} else if( *puc_SrcWork < 0x20 ) {
			puc_SrcWork++;
		} else {
			*puc_DstWork++ = *puc_SrcWork++;
			ss_cnt++;
		} 
	}
	*puc_DstWork = '\0';
	return ss_cnt;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：操作ID採番.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
uint32_t OPLC_Database::_numberingOperationID()
{
	uint32_t ul_logID = 0;															// 操作ID.
    
	APIC_SystemSetting* pc_SystemSetting = APIC_SystemSetting::newInstance();	// システム設定インスタンス取得.
	if( NULL == pc_SystemSetting ) {											// システム設定インスタンスがNULLの場合.
		return ul_logID;														// 操作ID０で処理を終了する.
	}

	_OpeLogIdSemLock();

	ul_logID = pc_SystemSetting->getSendOpeLogID();								// 操作ID取得.
	if ( OPLD_SendOpeLogIDMax == ul_logID ) {									// 操作IDが最大値の場合.
		ul_logID = 1;															// 操作IDに１をセットする.
	} else {																	// 操作IDが最大値でない場合.
		ul_logID = ul_logID + 1;												// 操作IDをインクリメントする.
	}
	pc_SystemSetting->setSendOpeLogID( ul_logID );								// 操作IDをセットする.

	_OpeLogIdSemUnlock();

	OPLDEBUG_LOGPRINT("[OPL][%d]\n", ul_logID,0,0,0,0,0);
	return ul_logID;															// 操作IDを返す.
}
//関数----------------------------------------------------------------------------------------------.
//概要    ：DBに指定キーでレコードを追加する(送信操作ログDB専用).
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
OPLE_ResultStatus OPLC_Database::_addRecordAtSendOpeLogDB(OPLS_SendOpeLog* p_Data, uint32_t ul_SendOpeLogID)
{
	OPLE_ResultStatus e_result = OPLE_RsltStatComplete;

	e_result = _addRecordAtDB(_pSendOpeLogDB[uc_SendOpeLogDBWriteIndex], p_Data, ul_SendOpeLogID);	// 送信操作ログ追加
	if (OPLE_RsltStatComplete == e_result) {
		us_SendOpeLogDBWriteRecord++;																// レコード数加算.
		OPLDEBUG_LOGPRINT("[OPL][Index:%d, Records:%d]\n", uc_SendOpeLogDBWriteIndex, us_SendOpeLogDBWriteRecord,0,0,0,0);
		OPLDEBUG_WAIT("1 record wait");
		if (OPLE_SendOpeLogRecordMax <= us_SendOpeLogDBWriteRecord) {								// 最大レコード数に達した場合.
			OPLDEBUG_WAIT("write index");
			_updateSendOpeLogDBWriteIndex();														// 書き込みインデックス更新.
			us_SendOpeLogDBWriteRecord = 0;
		}
	}
	return e_result;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：送信操作ログDB書き込みインデックス取得.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
uchar OPLC_Database::_getSendOpeLogDBWriteIndex()
{
	uchar uc_index = 0;

	APIC_SystemSetting* pc_SystemSetting = APIC_SystemSetting::newInstance();	// システム設定インスタンス取得.
	if( NULL == pc_SystemSetting ) {											// システム設定インスタンスがNULLの場合.
		return uc_index;
	}

	uc_index = pc_SystemSetting->getSendOpeLogDBIndex();

	OPLDEBUG_LOGPRINT("[OPL]index[%d]\n", uc_index,0,0,0,0,0);
	return uc_index;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：送信操作ログDB書き込みインデックス設定.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
void OPLC_Database::_setSendOpeLogDBWriteIndex(uchar _uc_SendOpeLogDBWriteIndex)
{
	APIC_SystemSetting* pc_SystemSetting = APIC_SystemSetting::newInstance();	// システム設定インスタンス取得.
	if( NULL == pc_SystemSetting ) {											// システム設定インスタンスがNULLの場合.
		return;
	}

	uc_SendOpeLogDBWriteIndex = _uc_SendOpeLogDBWriteIndex;
	pc_SystemSetting->setSendOpeLogDBIndex(uc_SendOpeLogDBWriteIndex);
	OPLDEBUG_LOGPRINT("[OPL]index[%d]\n", uc_SendOpeLogDBWriteIndex,0,0,0,0,0);
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：送信操作ログDB書き込みインデックス更新.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
void OPLC_Database::_updateSendOpeLogDBWriteIndex()
{
	uc_SendOpeLogDBWriteIndex = _IncrementSendOpeLogDBIndex(uc_SendOpeLogDBWriteIndex);
	_setSendOpeLogDBWriteIndex(uc_SendOpeLogDBWriteIndex);
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：送信操作ログDBのインデックス加算.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
uint32_t OPLC_Database::_IncrementSendOpeLogDBIndex(uint32_t _ul_index )
{
	_ul_index++;
	if (OPLE_SendOpeLogDBMax <= _ul_index) {
		_ul_index = 0;
	}
	return _ul_index;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：送信操作ログDBのインデックス減算.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
uint32_t OPLC_Database::_DecrementSendOpeLogDBIndex(uint32_t _ul_index )
{
	if (0 < _ul_index) {
		_ul_index--;
	} else {
		_ul_index = OPLE_SendOpeLogDBMax - 1;
	}
	return _ul_index;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：文字列時間(YYYYMMDDHHDDSS)からTYPC_Time形への変換処理.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
void OPLC_Database::_covertTime(char *sc_TimeStr, TYPC_Time *c_Time)
{
	char sc_Year[5];											// 年.
	char sc_Month[3];											// 月.
	char sc_Day[3];												// 日.
	char sc_Hour[3];											// 時.
	char sc_Minute[3];											// 分.
	char sc_Second[3];											// 秒.

	memset(sc_Year,    0x00, sizeof(sc_Year));
	memcpy(sc_Year,    &sc_TimeStr[0], 4);

	memset(sc_Month,   0x00, sizeof(sc_Month));
	memcpy(sc_Month,   &sc_TimeStr[4], 2);

	memset(sc_Day,     0x00, sizeof(sc_Day));
	memcpy(sc_Day,     &sc_TimeStr[6], 2);

	memset(sc_Hour,    0x00, sizeof(sc_Hour));
	memcpy(sc_Hour,    &sc_TimeStr[8], 2);

	memset(sc_Minute,  0x00, sizeof(sc_Minute));
	memcpy(sc_Minute,  &sc_TimeStr[10], 2);

	memset(sc_Second,  0x00, sizeof(sc_Second));
	memcpy(sc_Second,  &sc_TimeStr[12], 2);

	c_Time->ss_Year   = static_cast<short>(atoi(sc_Year));
	c_Time->sc_Month  = static_cast<char>(atoi(sc_Month));
	c_Time->sc_Day    = static_cast<char>(atoi(sc_Day));
	c_Time->sc_Hour   = static_cast<char>(atoi(sc_Hour));
	c_Time->sc_Minute = static_cast<char>(atoi(sc_Minute));;
	c_Time->sc_Second = static_cast<char>(atoi(sc_Second));;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：ローカルセマフォのロック.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
void OPLC_Database::_lock()
{
	SYSF_semTake( semID, WAIT_FOREVER );										// セマフォロック.
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：ローカルセマフォのアンロック.
//作成者  ：.
//レビュー：未実施（レビュー実施日と名前を記述）.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//--------------------------------------------------------------------------------------------------.
void OPLC_Database::_unlock()
{
	SYSF_semGive( semID );														// セマフォアンロック.
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：操作ID用セマフォのロック.
//作成者  ：日立 芳賀.
//レビュー：2015/01/19.
//変更履歴：2015/01/19 新規作成.
//--------------------------------------------------------------------------------------------------.
void OPLC_Database::_OpeLogIdSemLock()
{
	if (ERROR == SYSF_semTake(OpeLogIdSemId, WAIT_FOREVER))						// セマフォロック.
	{
		OPLD_ErrMsg(0xFF,0x03,"_IndexSemLock SYSF_semTake is Error. SemId=%p\n", reinterpret_cast<uintptr_t>(OpeLogIdSemId));
	}
	return;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：操作ID用セマフォのアンロック.
//作成者  ：日立 芳賀.
//レビュー：2015/01/19.
//変更履歴：2015/01/19 新規作成.
//--------------------------------------------------------------------------------------------------.
void OPLC_Database::_OpeLogIdSemUnlock()
{
	if (ERROR == SYSF_semGive(OpeLogIdSemId))										// セマフォアンロック.
	{
		OPLD_ErrMsg(0xFF,0x03,"_IndexSemUnlock SYSF_semGive is Error. SemId=%p\n", reinterpret_cast<uintptr_t>(OpeLogIdSemId));
	}
	return;
}

//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■.
//■■■　　デバッグ用処理. 操作性改善のデバッグ処理をそのままコピペ.
//■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■■.
// 2019.10.29 OP_BTS-13736 S
//static void debugLog_SendOpeLogOut( LIBC_List<OPLC_SendOpeLogOut>* pc_SendOpeLogOutList, short ss_Count )
static void debugLog_SendOpeLogOut( std::list<OPLC_SendOpeLogOut>& pc_SendOpeLogOutList, short ss_Count )
// 2019.10.29 OP_BTS-13736 E
{
	FILE* fp = NULL;
//	OPLC_SendOpeLogOut* pc_SendOpeLogOut = NULL;
	char outBuf[1024];
	char stringBuf[OPLD_EventDataStrMax + 1];
	char fileName[256];
	uchar uc_StringWork[1024];													// 文字列変換用バッファ.
	char eventDataStrBuf[OPLD_EventDataStrMax + 1];
	char eventSubDataStr[OPLD_SubInfoStrMax + 1];
	const char sc_EventStatusCode[][24] = { "キー押下", "画面表\示", "状態変化", "宛先情報", "全宛先解除" };
	const char sc_EventDataID[][70] = { "起動", "ログイン", "オートリセット", "副電源OFF", "ログアウト", "ADF原稿セット", "（コピー）オートリセット", "（スキャン／ファックス）オートリセット", "（ボックス）オートリセット", "低電力", "省電力"};
	const char sc_EventAddDataID[][24] = { "宛先選択", "宛先解除", "全宛先解除" };
	char sc_EventStatusStrBuf_SJIS[24];
	char sc_EventStatusStrBuf_UTF8[24];
	char sc_subDataStrBuf_SJIS[24];
	char sc_subDataStrBuf_UTF8[24];
	Bool b_file = False;

	if (False == b_OplDebugLog) {
		return;
	}

	memset(fileName, 0x00, sizeof(fileName));
	const char* pfilePath = FILC_BaseDirIf::newInstance( OPLE_DatabaseIDSendOpeLog )->getPath();
	sprintf(fileName, "%s/Opl/SendOpeLogOut.txt", pfilePath);

	OPLF_Printf("========== sys:OPL ==========\n");
	OPLF_Printf("SendOperationLog Out Data\n");
	OPLF_Printf("========== sys:OPL ==========\n");

	if(( fp = SYSF_fopen( fileName, "a+" )) != NULL )	b_file = True;

	memset(outBuf, 0x00, sizeof(outBuf));
	sprintf(outBuf, "========== sys:OPL ==========\n");
	if( True == b_file )	fwrite( outBuf, strlen( outBuf ), 1, fp );
	memset(outBuf, 0x00, sizeof(outBuf));
	sprintf(outBuf, "[OPL]    SendOperationLog Out Data\n");
	if( True == b_file )	fwrite( outBuf, strlen( outBuf ), 1, fp );
	memset(outBuf, 0x00, sizeof(outBuf));
	sprintf(outBuf, "========== sys:OPL ==========\n");
	if( True == b_file )	fwrite( outBuf, strlen( outBuf ), 1, fp );

// 2019.10.29 OP_BTS-13736 S
//	pc_SendOpeLogOut = pc_SendOpeLogOutList->findFirst();	// 最初のノードを取得.
//	for (int i=0; i<ss_Count; i++) {
//
//		if( 0 != i ) {
//			pc_SendOpeLogOut = pc_SendOpeLogOutList->findNext();	// 次のノードを取得.
//		}
//		if( NULL == pc_SendOpeLogOut ) {
//			break;
//		}
	std::list<OPLC_SendOpeLogOut>::iterator pc_SendOpeLogOut;
	for(pc_SendOpeLogOut = pc_SendOpeLogOutList.begin();pc_SendOpeLogOut != pc_SendOpeLogOutList.end();++pc_SendOpeLogOut){
// 2019.10.29 OP_BTS-13736 E
		memset(outBuf, 0x00, sizeof(outBuf));
		memset(stringBuf, 0x00, sizeof(stringBuf));
		memset(eventDataStrBuf, 0x00, sizeof(eventDataStrBuf));
		memset(eventSubDataStr, 0x00, sizeof(eventSubDataStr));
		memset( uc_StringWork, 0x00, sizeof( uc_StringWork ) );
		memset( sc_EventStatusStrBuf_SJIS, 0x00, sizeof( sc_EventStatusStrBuf_SJIS ) );
		memset( sc_EventStatusStrBuf_UTF8, 0x00, sizeof( sc_EventStatusStrBuf_UTF8 ) );
		memset( sc_subDataStrBuf_SJIS, 0x00, sizeof( sc_subDataStrBuf_SJIS ) );
		memset( sc_subDataStrBuf_UTF8, 0x00, sizeof( sc_subDataStrBuf_UTF8 ) );
		if( OPLE_EvtStatChngStat != pc_SendOpeLogOut->s_OutData.e_EventStatusCode ) {
			if( 0 != pc_SendOpeLogOut->s_OutData.us_EventDataID ) {
				_strncpyClearCtrCode(uc_StringWork, const_cast<uchar *>(getTextPartsPointer( pc_SendOpeLogOut->s_OutData.us_EventDataID )), OPLD_EventDataStrMax );	// 文字列IDを文字列に変換する.
				LIBF_ConvertToUTF8( static_cast<LIBE_CharSet>(getCharSetForCAPI()), reinterpret_cast<char *>(uc_StringWork), static_cast<char *>(stringBuf), OPLD_EventDataStrMax );
			}
		} else {
			if( 1 <= pc_SendOpeLogOut->s_OutData.us_EventDataID && 11 >= pc_SendOpeLogOut->s_OutData.us_EventDataID ) {
				memcpy(uc_StringWork, sc_EventDataID[pc_SendOpeLogOut->s_OutData.us_EventDataID-1], 70);
				LIBF_ConvertToUTF8( static_cast<LIBE_CharSet>(getCharSetForCAPI()), reinterpret_cast<char *>(uc_StringWork), static_cast<char *>(stringBuf), 70 );
			}
		}
		if( 0 != strlen(pc_SendOpeLogOut->s_OutData.sc_EventDataStr) ) {
			LIBF_ConvertFromUTF8( static_cast<char *>(pc_SendOpeLogOut->s_OutData.sc_EventDataStr), static_cast<LIBE_CharSet>(getCharSetForCAPI()), static_cast<char *>(eventDataStrBuf), OPLD_EventDataStrMax );
		}
		if( 0 != strlen(pc_SendOpeLogOut->s_OutData.sc_EventSubDataStr) ) {
			LIBF_ConvertFromUTF8( static_cast<char *>(pc_SendOpeLogOut->s_OutData.sc_EventSubDataStr), static_cast<LIBE_CharSet>(getCharSetForCAPI()), static_cast<char *>(eventSubDataStr), OPLD_SubInfoStrMax );
		}
		if( 1 <= pc_SendOpeLogOut->s_OutData.e_EventStatusCode && 5 >= pc_SendOpeLogOut->s_OutData.e_EventStatusCode ) {
			memcpy(sc_EventStatusStrBuf_SJIS, sc_EventStatusCode[pc_SendOpeLogOut->s_OutData.e_EventStatusCode-1], 24);
			LIBF_ConvertToUTF8( static_cast<LIBE_CharSet>(getCharSetForCAPI()), static_cast<char *>(sc_EventStatusStrBuf_SJIS), static_cast<char *>(sc_EventStatusStrBuf_UTF8), 24 );
		}
		if( 1 <= pc_SendOpeLogOut->s_OutData.us_EventSubDataID && 3 >= pc_SendOpeLogOut->s_OutData.us_EventSubDataID ) {
			memcpy(sc_subDataStrBuf_SJIS, sc_EventAddDataID[pc_SendOpeLogOut->s_OutData.us_EventSubDataID-1], 24);
			LIBF_ConvertToUTF8( static_cast<LIBE_CharSet>(getCharSetForCAPI()), static_cast<char *>(sc_subDataStrBuf_SJIS), static_cast<char *>(sc_subDataStrBuf_UTF8), 24 );
		}
		sprintf(outBuf, "%s,%d(%s),%6d(%s),%s,%d(%s),%s\n",
			pc_SendOpeLogOut->s_OutData.sc_EventTimeStr,
			pc_SendOpeLogOut->s_OutData.e_EventStatusCode,
			sc_EventStatusStrBuf_UTF8,
			pc_SendOpeLogOut->s_OutData.us_EventDataID,
			stringBuf,
			pc_SendOpeLogOut->s_OutData.sc_EventDataStr,
			pc_SendOpeLogOut->s_OutData.us_EventSubDataID,
			sc_subDataStrBuf_UTF8,
			pc_SendOpeLogOut->s_OutData.sc_EventSubDataStr);

		if( True == b_file )	fwrite( outBuf, strlen( outBuf ), 1, fp );

		OPLF_Printf("[OPL] %s,%d(%s),%6d(%s),%s,%d(%s),%s\n",
			pc_SendOpeLogOut->s_OutData.sc_EventTimeStr,
			pc_SendOpeLogOut->s_OutData.e_EventStatusCode,
			sc_EventStatusStrBuf_UTF8,
			pc_SendOpeLogOut->s_OutData.us_EventDataID,
			stringBuf,
			pc_SendOpeLogOut->s_OutData.sc_EventDataStr,
			pc_SendOpeLogOut->s_OutData.us_EventSubDataID,
			sc_subDataStrBuf_UTF8,
			pc_SendOpeLogOut->s_OutData.sc_EventSubDataStr);

	}

	if( True == b_file )	SYSF_fclose( fp );
	OPLF_Printf("[OPL] --------------------------------------------------------------------------\n");
}

static short _strncpyClearCtrCode( uchar* puc_Dst, uchar* puc_Src, short ss_Size )
{
	short ss_cnt = 0;
	uchar* puc_DstWork = puc_Dst;
	uchar* puc_SrcWork = puc_Src;
	
	// 引数チェック.
	if( NULL == puc_Dst || NULL == puc_Src ) {
		LOGF_debug_PutEventLog( LOGF_DEBUG_EVENT_KIND_ALL, 0, LOGD_DEBUG_LEVEL_FATAL, "Assert Failed!" );	
		return 0;
	}

	// 終端文字、または用意されたバッファサイズ分コピー処理する.
	// ただしコントロール文字の場合はコピーしない.
	for(;;) {
		if( '\0' == *puc_SrcWork || ss_cnt >= ss_Size ) {
			break;
		} else if( *puc_SrcWork < 0x20 ) {
			puc_SrcWork++;
		} else {
			*puc_DstWork++ = *puc_SrcWork++;
			ss_cnt++;
		} 
	}
	*puc_DstWork = '\0';
	return ss_cnt;
}

//関数----------------------------------------------------------------------------------------------.
//概要    ：デバッグ機能：セマフォ情報をターミナルに出力する.
//作成者  ：日立 芳賀.
//レビュー：2015/01/19 日立 太田、芳賀.
//変更履歴：2015/01/19 新規作成.
//--------------------------------------------------------------------------------------------------.
void OPLC_Database::debugPrintSemInfo()
{
	OPLF_Printf("========== sys:OPL ==========\n");
	OPLF_Printf(" semID\n");
	OPLF_Printf("========== sys:OPL ==========\n");
	if (0 != semID)
	{
		OPLF_Printf("### semShow(semID, 0)\n");
		SYSF_semShow(semID, 0);
		OPLF_Printf("### semShow(semID, 1)\n");
		SYSF_semShow(semID, 1);
		OPLF_Printf("\n");
	}
	OPLF_Printf("========== sys:OPL ==========\n");
	OPLF_Printf(" OpeLogIdSemId\n");
	OPLF_Printf("========== sys:OPL ==========\n");
	if (0 != OpeLogIdSemId)
	{
		OPLF_Printf("### semShow(OpeLogIdSemId, 0)\n");
		SYSF_semShow(OpeLogIdSemId, 0);
		OPLF_Printf("### semShow(OpeLogIdSemId, 1)\n");
		SYSF_semShow(OpeLogIdSemId, 1);
		OPLF_Printf("\n");
	}
	return;
}

// 2016/11/14 IT5_4.1 制御構成変更_No.41 Boxフル状態再現対応 Start.
//関数----------------------------------------------------------------------------------------------.
//概要    ：デバッグ機能：指定数分の送信操作ログを追加.
//作成者  ：日立 芳賀.
//レビュー：2016/11/14 日立 太田、芳賀.
//変更履歴：2016/11/14 新規作成.
//--------------------------------------------------------------------------------------------------.
uint32_t OPLC_Database::debugAddOpeLogSpecifiedCount(uint32_t ui32_Count)
{
	OPLS_SendOpeLog		s_SendOpeLog	= {};
	uint32_t			ui32_Result		= OPLE_RsltStatError;
	ushort				us_LogNo		= 0;
	TYPC_Time			c_Time;
	TYPC_Time			c_PreviousTime;
	int32_t				si32_msecCnt	= 0;
	TEXT_ID				textID			= 0;
	uint32_t			ui32_totalCnt = 0;

	// まずは全削除.
	_deleteSendOpeLogDB();

	for (us_LogNo=1; us_LogNo<=ui32_Count; us_LogNo++)
	{
		memset(&s_SendOpeLog, 0x00, sizeof(OPLS_SendOpeLog));

		s_SendOpeLog.ul_SendOpeLogID = _numberingOperationID();
		s_SendOpeLog.us_LogNo = us_LogNo;

		c_Time.setNowTime();
		if ((c_PreviousTime.ss_Year == c_Time.ss_Year)&&
			(c_PreviousTime.sc_Month == c_Time.sc_Month)&&
			(c_PreviousTime.sc_Day == c_Time.sc_Day)&&
			(c_PreviousTime.sc_Hour == c_Time.sc_Hour)&&
			(c_PreviousTime.sc_Minute == c_Time.sc_Minute)&&
			(c_PreviousTime.sc_Second == c_Time.sc_Second))
		{
			si32_msecCnt++;
		}
		else
		{
			si32_msecCnt = 0;
		}
		snprintf(	s_SendOpeLog.sc_EventTimeStr,
					sizeof(s_SendOpeLog.sc_EventTimeStr)-1,
					"%04d%02d%02d%02d%02d%02d%02d0",
					c_Time.ss_Year, c_Time.sc_Month, c_Time.sc_Day, c_Time.sc_Hour, c_Time.sc_Minute, c_Time.sc_Second, si32_msecCnt);
		c_PreviousTime = c_Time;

		s_SendOpeLog.e_EventStatusCode = OPLE_EvtStatChngStat;

		s_SendOpeLog.us_EventDataID = OPLE_ChngStatAutoResetBox;

		textID = _convertStatToStringID(s_SendOpeLog.us_EventDataID);
		strncpy(s_SendOpeLog.sc_EventDataStr, reinterpret_cast<const char*>(getTextPartsPointer(textID)), OPLD_EventDataStrMax);

		ui32_Result = _addRecordAtSendOpeLogDB(&s_SendOpeLog, s_SendOpeLog.ul_SendOpeLogID);
		if (OPLE_RsltStatComplete != ui32_Result)
		{
			OPLD_ErrMsg(0xFF,0x03,"OPLC_Database::debugAddOpeLogSpecifiedCount _addRecordAtSendOpeLogDB(INDEX=%d) Err[%d]\n", us_LogNo, ui32_Result);
			break;
		}
		// 50Record挿入したら100msのディレイ.
		if (0 == (us_LogNo % 50))
		{
			SYSF_taskDelay((SYSF_sysClkRateGet() * 100) / 1000);
		}

		// トータルカウンタがループ数と一致したら終了.
		if((ui32_totalCnt + us_LogNo) == ui32_Count){
			printf("debugAddOpeLogSpecifiedCount loop end ui32_totalCnt = %d us_LogNo = %d\n",ui32_totalCnt,us_LogNo);
			break;
		}

		// 1000件毎にLogNoをリセット(1度に登録できるのは1000押下分という仕様に沿う).
		if(us_LogNo == OPLD_KeyPushLogRecMax){
			ui32_totalCnt += us_LogNo;
			printf("debugAddOpeLogSpecifiedCount CountUp ui32_totalCnt = %d\n",ui32_totalCnt);
			us_LogNo = 1;
		}

	}
	return ui32_Result;
}
// 2016/11/14 IT5_4.1 制御構成変更_No.41 Boxフル状態再現対応 End.

//----------------------------------------------------------------------------------------------
// システム共通デバッグ出力関数 (Opl用にカスタマイズ)
//----------------------------------------------------------------------------------------------
//概要　　：垂れ流しログ出力用ラッパー.
//戻り値　：なし.
//引数　　：なし.
//備考　　：文字列長が行の最大(200byte)に達したら改行して切る.
//        ：出力内容に改行コードが含まれていても考慮せずに200byteで改行するので注意.
//        ：ファイルコードについては以下の値をコール元で直に設定する.
//        ：(Docでは変換表を用いているがOplは量が少ないので直値とする)
//          APIC_OPL.cpp              0x01
//          OPLG_InitModuleIf.cpp     0x02
//          OPLC_Database.cpp         0x03
//          OPLC_OPL.cpp              0x04
//.
int32_t OPLF_LogMsg(int32_t sl_fileCode, int32_t sl_Line,const char* fmt, ...){
	char sc_fmtEdit[OPLD_LogMsgLen +2] = {}; // 末尾に改行+NULL文字分を用意.
	char sc_line[OPLD_LogMsgLen +2]    = {};
	
	va_list arg;
	va_start( arg, fmt );
	snprintf( sc_fmtEdit, sizeof(sc_fmtEdit), "(%010u) (sys:OPL) (%02x)(%05d)%s",SYSF_tickGet(),sl_fileCode,sl_Line, fmt );
	vsnprintf( sc_line, sizeof(sc_line), sc_fmtEdit, arg );
	va_end( arg );
	
	// 最大長まで達した場合は末尾で改行して切る.
	if( OPLD_LogMsgLen <= strlen(sc_line) ){
		sc_line[ OPLD_LogMsgLen   ] = '\n';
		sc_line[ OPLD_LogMsgLen+1 ] = '\0';
	}
	return SYSF_logMsg( sc_line, 0,0,0,0,0,0 );
}

//概要　　：垂れ流しエラーログ出力用ラッパー.
//戻り値　：なし.
//引数　　：なし.
//備考　　：文字列長が行の最大(200byte)に達したら改行して切る.
//        ：出力内容に改行コードが含まれていても考慮せずに200byteで改行するので注意.
//        ：ファイルコードについては以下の値をコール元で直に設定する.
//        ：(Docでは変換表を用いているがOplは量が少ないので直値とする)
//          APIC_OPL.cpp              0x01
//          OPLG_InitModuleIf.cpp     0x02
//          OPLC_Database.cpp         0x03
//          OPLC_OPL.cpp              0x04
//.
int32_t OPLF_ErrMsg(int32_t sl_ErrCode,int32_t sl_fileCode, int32_t sl_Line,const char* fmt, ...){
	char sc_fmtEdit[OPLD_LogMsgLen +2] = {}; // 末尾に改行+NULL文字分を用意.
	char sc_line[OPLD_LogMsgLen +2]    = {};
	
	va_list arg;
	va_start( arg, fmt );
	snprintf( sc_fmtEdit, sizeof(sc_fmtEdit), "(%010u) (sys:OPL) (%02x)(%05d)(%02x)Opl_Alert %s",SYSF_tickGet(),sl_fileCode,sl_Line,sl_ErrCode, fmt );
	vsnprintf( sc_line, sizeof(sc_line), sc_fmtEdit, arg );
	va_end( arg );
	
	// 最大長まで達した場合は末尾で改行して切る.
	if( OPLD_LogMsgLen <= strlen(sc_line) ){
		sc_line[ OPLD_LogMsgLen   ] = '\n';
		sc_line[ OPLD_LogMsgLen+1 ] = '\0';
	}
	return SYSF_logMsg( sc_line, 0,0,0,0,0,0 );
}

//.
//概要　　：ログ関数出力用ラッパー.
//戻り値　：なし.
//引数　　：なし.
//備考　　：文字列長が行の最大(200byte)に達したら改行して切る.
//        ：出力内容に改行コードが含まれていても考慮せずに200byteで改行するので注意.
//.
int32_t OPLF_Printf(const char* fmt, ...){
	char sc_line[OPLD_LogMsgLen +2] = {};
	
	va_list arg;
	va_start( arg, fmt );
	vsnprintf( sc_line, sizeof(sc_line), fmt, arg );
	va_end( arg );
	
	// 最大長まで達した場合は末尾で改行して切る.
	if( OPLD_LogMsgLen <= strlen(sc_line) ){
		sc_line[ OPLD_LogMsgLen   ] = '\n';
		sc_line[ OPLD_LogMsgLen+1 ] = '\0';
	}
	return SYSF_logMsg( sc_line, 0,0,0,0,0,0 );
}
