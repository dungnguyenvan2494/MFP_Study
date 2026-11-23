/*=======================================================================================

	［ﾀｲﾄﾙ］	VIDデータの管理

	［説明］	

=======================================================================================*/

/*=======================================================================================
	インクルードファイル 
=======================================================================================*/
// ..System
#include	<stdlib.h>
#include	<string.h>
#include	<stdio.h>

// ..Common
#include "system/include/typs.h"

// ..Local
#include	"MIBG_VIDManager.h"

/*=======================================================================================
	定数定義 
=======================================================================================*/
// ユビキタス[KRQ-278][スプール分散恒久対応][step2] start.
#define MIBD_PortTblSize			8			// ポート負荷状態初期値テーブルのサイズ.
// ユビキタス[KRQ-278][スプール分散恒久対応][step2] end.

/*=======================================================================================
	マクロ関数定義 
=======================================================================================*/
// 全VIDテーブル(_m_pc_iAllVIDData)で、VIDデータが定義されているかのチェック 
#define	MIBD_IsVIDDataExist( data ) 	( ( data EQ NULL ) ? False : True )

/*=======================================================================================
	列挙型定義 
=======================================================================================*/

/*=======================================================================================
	構造体定義 
=======================================================================================*/

/*=======================================================================================
	外部参照変数定義 
=======================================================================================*/

/*=======================================================================================
	外部定義関数 
=======================================================================================*/

/*=======================================================================================
	モジュール内変数定義 
=======================================================================================*/
// ジョブ情報テーブル実体 
// ht_sekiguchi 20050224 S [メモリ確保失敗によるプリント停止問題対応] 
static MIBS_JobDataTbl s_JobDataTblActive[MIBD_Max_ActiveJobListAll];
static MIBS_JobDataTbl s_JobDataTblLog[MIBD_Max_JobLogListAll];
/* 2008.08.18 UV-00536 sw_miya add S */
MIBS_PrintJobDataTbl s_PrintJobDataTblActive[MIBD_Max_ActiveJobListAll];
MIBS_PrintJobDataTbl s_PrintJobDataTblLog[MIBD_Max_JobLogListAll];
/* 2008.08.18 UV-00536 sw_miya add E */
/* 2008.10.23 No.309942 sw_saku add S */
MIBS_PrintJobDataTbl *sp_PrintJobDataTblAll[MIBD_Max_kmJmJobIndex];

MIBE_AttributeType MIBE_AttributeTbl[] = {
	MIBE_AttributeNon,							//属性なし.
	MIBE_AttributeOther,						//other(1).
	MIBE_AttributeJobAccountName,				//jobAccountName(21).
	MIBE_AttributeJobName,						//jobName(23).
	MIBE_AttributeDeviceNameRequested,			//deviceNameRequested(30).
	MIBE_AttributeFileName,						//fileName(34).
	MIBE_AttributeDocumentName,					//documentName(35).
	MIBE_AttributeOutputBin,					//outputBin(54).
	MIBE_AttributeMediumRequested,				//mediumRequested(170).
	MIBE_AttributeMediumConsumed,				//mediumConsumed(171).
	MIBE_AttributeColorantRequested,			//colorantRequested(172).
	MIBE_AttributeColorantConsumed,				//colorantConsumed(173).
	MIBE_AttributeMediumTypeConsumed,			//mediumTypeConsumed(174).
	MIBE_AttributeMediumSizeConsumed,			//mediumSizeConsumed(175).
	MIBE_AttributeJobStartedProcessingTime,		//jobStartedProcessingTime(193).
	MIBE_EndofAttributeJob,						//番兵.
};	
/* 2008.10.23 No.309942 sw_saku add E */

// ht_sekiguchi 20050224 E [メモリ確保失敗によるプリント停止問題対応] 
//static MIBS_JobDataTbl s_JobDataTbl[110];

// ユビキタス[KRQ-278][スプール分散恒久対応][step2] start.
// ポート負荷状態初期値テーブル.
const MIBC_PortLoadStatusMapPair MIBS_PortConfigTbl[MIBD_PortTblSize] = {
	 { MIBE_PortLpr1, MIBE_PortLoadStatusUnknown }
	,{ MIBE_PortLpr2, MIBE_PortLoadStatusUnknown }
	,{ MIBE_PortRaw1, MIBE_PortLoadStatusLow }
	,{ MIBE_PortRaw2, MIBE_PortLoadStatusLow }
	,{ MIBE_PortRaw3, MIBE_PortLoadStatusLow }
	,{ MIBE_PortRaw4, MIBE_PortLoadStatusLow }
	,{ MIBE_PortRaw5, MIBE_PortLoadStatusLow }
	,{ MIBE_PortRaw6, MIBE_PortLoadStatusLow }
};
// ユビキタス[KRQ-278][スプール分散恒久対応][step2] end.

/*=======================================================================================
	モジュール内関数定義
=======================================================================================*/

/*=======================================================================================
// CLASS	: MIBC_VIDDataBasic
// ABSTRACT	: 個々のVIDデータを扱う基本クラス(INT型･インデックス無し･対応Hexパーツ無し) 
=======================================================================================*/

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_VIDDataBasic::MIBC_VIDDataBasic
// ABSTRACT	: コンストラクタ 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 自分自身 
// CREATE	: 01-08-03	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
MIBC_VIDDataBasic::MIBC_VIDDataBasic(
	const VID				_r_ceVID,							// VID
	const MIO_MSG_ID		_r_ceReqMIOMsgID,					// MIO Msg ID(要求) 
	const MIO_MSG_ID		_r_ceCmdMIOMsgID,					// MIO Msg ID(コマンド) 
	void* const				_r_vpcData							// データへのポインタ 
) : _m_ceVID( _r_ceVID ), _m_ceReqMIOMsgID( _r_ceReqMIOMsgID ),
	_m_ceCmdMIOMsgID( _r_ceCmdMIOMsgID ), _m_vpcData( _r_vpcData )
{
	// 引数のチェック 
	// ..VIDが未定義ではあってはいけない 
	DASSERT( VIDD_VID_Start < _r_ceVID AND _r_ceVID < VIDD_VID_End );
	// ..データへのポインタがNULLであってはいけない 
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_VIDDataBasic::ui32Get
// ABSTRACT	: データの取得
// FUNCTION	: 引数にデータをセットして返す
// NOTE		: 
// RETURN	: UINT32 _m_ui32CrntSize : 取得サイズ=現在のデータサイズ 
// CREATE	: 02-05-17	河辺
//			  新規作成
//			  02-06-10	河辺
//			  nmIJCIF.cppで全VIDデータの処理を行うように変更 
/////////////////////////////////////////////////////////////////////////////////////////
UINT32 MIBC_VIDDataBasic::ui32Get(
	const VID			_r_ceVID,					// VID
// 【IT5_4.2】【RQ686】HCL-Kaushik 2017/09/01 Modify START.
	void* const			_r_vpcGetData,				// 取得するデータへのポインタ
	MIO_NWIF_TYPE eNwIfIndex
// 【IT5_4.2】【RQ686】HCL-Kaushik 2017/09/01 Modify END.
)
{
	// 引数のチェック 
	// .. 取得するデータへのポインタがNULLであってはいけない 
	DASSERT( _r_vpcGetData NE NULL );

	// データの設定
	UINT32 ui32CrntSize = 0;
	UINT32 ui32MaxSize = ui32GetMaxSize();
// 【IT5_4.2】【RQ686】HCL-Kaushik 2017/09/01 Modify START.
	ui32CrntSize = in_ui32ProcGetAllVIDData( _r_ceVID, _r_vpcGetData, ui32MaxSize, eNwIfIndex);
// 【IT5_4.2】【RQ686】HCL-Kaushik 2017/09/01 Modify END.

	return ui32CrntSize;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_VIDDataBasic::ui32VIDGet
// ABSTRACT	: データの取得
// FUNCTION	: 引数にデータをセットして返す
// NOTE		: 
// RETURN	: UINT32 _m_ui32CrntSize : 取得サイズ=現在のデータサイズ 
// CREATE	: 02-04-06	河辺
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
UINT32 MIBC_VIDDataBasic::ui32VIDGet(
	const VID			_r_ceVID,					// VID
	void* const			_r_vpcGetData				// 取得するデータへのポインタ
)
{
	// 引数のチェック 
	// .. 取得するデータへのポインタがNULLであってはいけない 
	DASSERT( _r_vpcGetData NE NULL );

	// データの設定
	UINT32 ui32CrntSize = 0;

	return ui32CrntSize;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_VIDDataBasic::vSet 
// ABSTRACT	: データの設定 
// FUNCTION	: 引数のデータを内部データにセットする 
// NOTE		: INT型が対象なので、設定するデータのサイズは関係ない 
// RETURN	: 無し
// CREATE	: 01-08-03	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_VIDDataBasic::vSet(
	const void* const		_r_cvpcSetData,				// 設定するデータへのポインタ
	CUINT32					_r_cui32SetDataSize,		// 設定するデータのサイズ 
	const MIBE_CharacterSet	_r_ceCharacterSet			// 文字集合(データタイプがストリングの時のみ有効) 
)
{
	// 引数のチェック 
	// .. 設定するデータへのポインタがNULLであってはいけない 
	DASSERT( _r_cvpcSetData NE NULL );

	// データの設定
	UINT32 ui32SetSize = ui32GetSize();
	_vSetData( _m_vpcData, _r_cvpcSetData, ui32SetSize );
}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_VIDDataBasic::_vSetData 
// ABSTRACT	: データ設定 
// FUNCTION	: 
// NOTE		: 
//			  
// RETURN	: 無し 
// CREATE	: 01-08-03	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_VIDDataBasic::_vSetData(
	void* const			_r_vpcDstData,				// 設定されるデータへのポインタ 
	const void* const	_r_cvpcSrcData,				// 設定するデータへのポインタ 
	CUINT32				_r_cui32Size				// データサイズ 
)
{
	// 引数のチェック
	// ..設定するデータへのポインタがNULLであってはいけない
	DASSERT( _r_cvpcSrcData NE NULL );

	// データ設定
	memcpy( _r_vpcDstData, _r_cvpcSrcData, _r_cui32Size );
}
/*=======================================================================================
// CLASS	: MIBC_VIDDataWith1Index
// ABSTRACT	: 個々のVIDデータを扱う基本クラス(インデックス一つ) 
=======================================================================================*/

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_VIDDataWith1Index::MIBC_VIDDataWith1Index
// ABSTRACT	: コンストラクタ 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 自分自身 
// CREATE	: 03-01-16	松本 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
MIBC_VIDDataWith1Index::MIBC_VIDDataWith1Index(
	const VID				vid,							// VID
	const MIO_MSG_ID		req,					// MIO Msg ID(要求) 
	const MIO_MSG_ID		cmd,					// MIO Msg ID(コマンド) 
	void* const				data							// データへのポインタ 
) : MIBC_VIDDataBasic( vid, req, cmd, data ), _m_i32Index( 0 )		// Minerva/Zeus 静的解析 2011/02/21 y.ito .
{}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_VIDDataWith1Index::isMe
// ABSTRACT	: 自分のMIO Msg IDか？ 
// FUNCTION	: 
// NOTE		: Bool isMine : True : 自分のMIO Msg ID
// RETURN	: 無し 
// CREATE	: 01-08-03	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
Bool MIBC_VIDDataWith1Index::isMe(
	const MIO_MSG_ID	_r_ceMIOMsgID,				// MIO Msg ID 
	CINT32*				_r_ci32pIndex				// インデックス 
) const
{
	// MIO Msg ID及びIndexが等しいものが自分のもの 
	Bool isMine = False;
	Bool isIDEqual = MIBC_VIDDataBasic::isMe( _r_ceMIOMsgID );
	if ( isIDEqual ) {
		if ( _r_ci32pIndex[0] EQ _m_i32Index ) {
			isMine = True;
		}
	}
	return isMine;
}

/*=======================================================================================
// CLASS	: MIBC_VIDDataWith2Index
// ABSTRACT	: 個々のVIDデータを扱う基本クラス(インデックス二つ) 
=======================================================================================*/
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_VIDDataWith1Index::MIBC_VIDDataWith1Index
// ABSTRACT	: コンストラクタ 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 自分自身 
// CREATE	: 01-08-03	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
MIBC_VIDDataWith2Index::MIBC_VIDDataWith2Index(
	const VID				vid,							// VID
	const MIO_MSG_ID		req,					// MIO Msg ID(要求) 
	const MIO_MSG_ID		cmd,					// MIO Msg ID(コマンド) 
	void* const				data							// データへのポインタ 
) : MIBC_VIDDataBasic( vid, req, cmd, data ), 
	MIBC_VIDDataWith1Index( vid, req, cmd, data ),
	_m_i32Index( 0 )		// Minerva/Zeus 静的解析 2011/02/21 y.ito .
{}				// コンストラクタ

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_VIDDataWith2Index::isMe
// ABSTRACT	: 自分のMIO Msg IDか？ 
// FUNCTION	: 
// NOTE		: Bool isMine : True : 自分のMIO Msg ID
// RETURN	: 無し 
// CREATE	: 01-08-03	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
Bool MIBC_VIDDataWith2Index::isMe(
	const MIO_MSG_ID	_r_ceMIOMsgID,				// MIO Msg ID 
	CINT32*				_r_ci32pIndex				// インデックス 
) const
{
	// MIO Msg ID及びIndexが等しいものが自分のもの 
	Bool isMine = False;
	Bool isEqual = MIBC_VIDDataWith1Index::isMe( _r_ceMIOMsgID, _r_ci32pIndex );
	if ( isEqual ) {
		if ( _r_ci32pIndex[1] EQ _m_i32Index ) {
			isMine = True;
		}
	}
	return isMine;
}

/*=======================================================================================
// CLASS	: MIBC_VIDDataINT32
// ABSTRACT	: 個々のVIDデータを扱うクラス(INT型･インデックス無し･対応Hexパーツ無し) 
=======================================================================================*/
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_VIDDataWith1Index::MIBC_VIDDataWith1Index
// ABSTRACT	: コンストラクタ 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 自分自身 
// CREATE	: 01-08-03	廣田 	
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
MIBC_VIDDataINT32::MIBC_VIDDataINT32( const VID vid, const MIO_MSG_ID req, const MIO_MSG_ID cmd, void* const data ) : MIBC_VIDDataBasic( vid, req, cmd, data ) {}					// コンストラクタ 

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_VIDDataINT32::ui32GetValueStr
// ABSTRACT	: 値を文字として取得 
// FUNCTION	: 引数にデータをセットして返す。最後にNULLを付加して返す。
// NOTE		: 32bitなので、0-4,294,967,295の10桁となるので、取得するデータへの領域は、
//			  最後のNULLも含めてchar型で11個分の領域を用意しておくこと
// RETURN	: UINT32 ui32GetSize : 取得サイズ 
// CREATE	: 01-08-03	廣田 
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
UINT32 MIBC_VIDDataINT32::ui32GetValueStr(
	const VID			_r_ceVID,					// VID
	void* const			_r_vpcGetData				// 取得するデータへのポインタ
)
{
	#define	MIBD_Ascii_0	0x30
	// 引数のチェック 
	// .. 取得するデータへのポインタがNULLであってはいけない 
	DASSERT( _r_vpcGetData NE NULL );

	// 現在のデータの取得 
	INT32 i32Data = 0;
	ui32Get( _r_ceVID, &i32Data );
	UINT32 ui32GetSize = _ui32GetValueToStr( i32Data, (UINT8* const)_r_vpcGetData );
	return ui32GetSize;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_VIDDataWith1Index::MIBC_VIDDataWith1Index
// ABSTRACT	: コンストラクタ 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 自分自身 
// CREATE	: 01-08-03	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
MIBC_VIDDataINT32With1Index::MIBC_VIDDataINT32With1Index( const VID vid, const MIO_MSG_ID req, const MIO_MSG_ID cmd,
		void* const data, CINT16 index ) : MIBC_VIDDataBasic( vid, req, cmd, data ),
		MIBC_VIDDataINT32( vid, req, cmd, data ),
		MIBC_VIDDataWith1Index( vid, req, cmd, data )
		{	INT32 in32Index[MIBD_MaxIndex];
			in32Index[0] = index;
			MIBC_VIDDataWith1Index::vSetIndex( in32Index );	}			// コンストラクタ

MIBC_VIDDataINT32With2Index::MIBC_VIDDataINT32With2Index( const VID vid, const MIO_MSG_ID req, const MIO_MSG_ID cmd,
		void* const data, CINT16 index1, CINT16 index2 ) : MIBC_VIDDataBasic( vid, req, cmd, data ),
		MIBC_VIDDataWith1Index( vid, req, cmd, data ),
		MIBC_VIDDataINT32( vid, req, cmd, data ),
		MIBC_VIDDataWith2Index( vid, req, cmd, data )
		{	INT32 in32Index[MIBD_MaxIndex];
			in32Index[0] = index1; in32Index[1] = index2; 
			MIBC_VIDDataWith2Index::vSetIndex( in32Index );	}			// コンストラクタ

/*=======================================================================================
// CLASS	: MIBC_VIDDataSTR
// ABSTRACT	: 個々のVIDデータを扱うクラス(ストリング型･インデックス無し) 
=======================================================================================*/
// Minerva/Zeus 静的解析 2011/02/21 y.ito start.
MIBC_VIDDataSTR::MIBC_VIDDataSTR( const VID vid, const MIO_MSG_ID req, const MIO_MSG_ID cmd, void* const data, CUINT32 maxSize ) : 
		MIBC_VIDDataBasic( vid, req, cmd, data ), 
		_m_cui32MaxSize( maxSize ), 
		_m_ui32CrntSize( 0 ), 
		_m_eCharacterSet( MIBE_CharacterSet_Start ) 
{}
// Minerva/Zeus 静的解析 2011/02/21 y.ito end.

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_VIDDataSTR::vSet 
// ABSTRACT	: データの設定 
// FUNCTION	: 引数のデータを内部データにセットする 
// NOTE		: データサイズは終端文字を含まない事 
// RETURN	: 無し
// CREATE	: 01-08-03	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_VIDDataSTR::vSet(
	const void* const		_r_cvpcSetData,				// 設定するデータへのポインタ
	CUINT32					_r_cui32SetDataSize,		// 設定するデータのサイズ 
	const MIBE_CharacterSet	_r_ceCharacterSet			// 文字集合(データタイプがストリングの時のみ有効) 
)
{
	// 引数のチェック 
	// .. 設定するデータへのポインタがNULLであってはいけない 
	DASSERT( _r_cvpcSetData NE NULL );

	// データの設定
	// .. 設定するデータサイズが、最大サイズより大きい場合は、最大サイズできる 
	_m_eCharacterSet = _r_ceCharacterSet;
	if ( _m_cui32MaxSize < _r_cui32SetDataSize ) {
		_m_ui32CrntSize = _m_cui32MaxSize;
	} else {
		_m_ui32CrntSize = _r_cui32SetDataSize;
	}
	_vSetData( _m_vpcData, _r_cvpcSetData, _m_ui32CrntSize );
}

MIBC_VIDDataSTRWith1Index::MIBC_VIDDataSTRWith1Index( const VID vid, const MIO_MSG_ID req, const MIO_MSG_ID cmd, void* const data,
		CINT32 maxSize, CINT16 index ) : MIBC_VIDDataBasic( vid, req, cmd, data ),
		MIBC_VIDDataSTR( vid, req, cmd, data, maxSize ),
		MIBC_VIDDataWith1Index( vid, req, cmd, data )
		{	INT32 in32Index[MIBD_MaxIndex];
			in32Index[0] = index;
			MIBC_VIDDataWith1Index::vSetIndex( in32Index );	}			// コンストラクタ

MIBC_VIDDataSTRWith2Index::MIBC_VIDDataSTRWith2Index( const VID vid, const MIO_MSG_ID req, const MIO_MSG_ID cmd, void* const data,
		CINT32 maxSize, CINT16 index1, CINT16 index2 ) : MIBC_VIDDataBasic( vid, req, cmd, data ),
		MIBC_VIDDataWith1Index( vid, req, cmd, data ),
		MIBC_VIDDataSTR( vid, req, cmd, data, maxSize ),
		MIBC_VIDDataWith2Index( vid, req, cmd, data )
		{	INT32 in32Index[MIBD_MaxIndex];
			in32Index[0] = index1;	in32Index[1] = index2;
			MIBC_VIDDataWith2Index::vSetIndex( in32Index );	}			// コンストラクタ

/*=======================================================================================
	CLASS		: MIBC_AllVIDMan
	ABSTRACT	: 全VIDデータの管理
=======================================================================================*/

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::MIBC_AllVIDMan
// ABSTRACT	: ｺﾝｽﾄﾗｸﾀ
// FUNCTION	: ｺﾝｽﾄﾗｸﾀ
// NOTE		: 
// RETURN	: 自分自身
// CREATE	: 01-05-30	廣田
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
MIBC_AllVIDMan::MIBC_AllVIDMan(
	MIBC_VIDDataBasic* const* _r_pc_iAllVIDData,					// 全VIDﾃﾞｰﾀ 
	CINT32					_r_ci32AllVIDDataSize				// 全VIDのﾃﾞｰﾀ数 
) : _m_pc_iAllVIDData( _r_pc_iAllVIDData ), 
	_m_ci32AllVIDDataSize( _r_ci32AllVIDDataSize ),
	_m_spSemID( 0 )		// Minerva/Zeus 静的解析 2011/02/21 y.ito .
{
	// 引数のチェック
	// .. VIDデータが一つもないことはない 
	DASSERT( 0 < _r_ci32AllVIDDataSize );

// ユビキタス[KRQ-278][スプール分散恒久対応][step2] start.
	// LPR1～RAW6負荷状態Mapを追加.
	m_c_PortLoadStatusMap.clear();													// 負荷状態Mapをクリア.

	for( ushort i = 0; i < MIBD_PortTblSize; i++ ){
		m_c_PortLoadStatusMap.insert( MIBS_PortConfigTbl[i] );						// 対象ポートをMapに追加.
	}
// ユビキタス[KRQ-278][スプール分散恒久対応][step2] end.
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::vCreateSemphore
// ABSTRACT	: セマフォの生成
// FUNCTION	: セマフォの生成
// NOTE		: 
// RETURN	: 無し
// CREATE	: 01-06-05	廣田
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_AllVIDMan::vCreateSemphore()
{
// バイナリではなく、排他制御のセマフォを生成するように修正	2004.07.23	kawabe
	_m_spSemID = SYSF_semMCreate( SEM_Q_PRIORITY, False );
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::vSemphoreLock
// ABSTRACT	: セマフォのロック
// FUNCTION	: セマフォのロック
// NOTE		: 
// RETURN	: 無し
// CREATE	: 01-06-05	廣田
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_AllVIDMan::vSemphoreLock()
{
	// 永遠に待ちつづける
	SYSF_semTake( _m_spSemID, WAIT_FOREVER );
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::vSemphoreUnLock()
// ABSTRACT	: セマフォのロック解除
// FUNCTION	: セマフォのロック解除
// NOTE		: 
// RETURN	: 無し
// CREATE	: 01-06-05	廣田
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_AllVIDMan::vSemphoreUnLock()
{
	SYSF_semGive( _m_spSemID );
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::eGetVID
// ABSTRACT	: MIOMsgIDからVIDへの変換 
// FUNCTION	: 引数で指定されるMIOMsgIDに相当するVIDを取得する 
// NOTE		: 
// RETURN	: VID
// CREATE	: 01-08-06	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
VID MIBC_AllVIDMan::eGetVID(
	const MIO_MSG_ID	_r_ceMIOMsgID,				// MIO Msg ID 
	CINT32* const		_r_ci32pcIndiIndex			// 各Indexの指定値(左から)(Indexの数分データが有る) 
) const
{
	VID eVID = VIDD_VID_Undef;			// ???
	INT32 i32AllVIDDataIndex = DUndefIndex;		// 該当するデータ無し
	// ATODE 
	// サーチせずに行う。 
	for ( INT32 index = 0; index < _m_ci32AllVIDDataSize; index++ ) {
		if ( _m_pc_iAllVIDData[ index ] != NULL ) {
			if ( _m_pc_iAllVIDData[ index ]->isMe( _r_ceMIOMsgID, _r_ci32pcIndiIndex ) ) {
				i32AllVIDDataIndex = index;
				break;
			}
		}
	}
	if ( i32AllVIDDataIndex != DUndefIndex ) {
		eVID = _m_pc_iAllVIDData[ i32AllVIDDataIndex ]->ui32GetVID();
	}

	return eVID;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::i32Get
// ABSTRACT	: データの取得 
// FUNCTION	: 引数にデータをセットして返す 
// NOTE		: 
// RETURN	: UINT32 ui32GetSize : 取得サイズ 
// CREATE	: 02-05-17	河辺 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
UINT32 MIBC_AllVIDMan::ui32Get(
	const VID			_r_ceVID,					// VID
// 【IT5_4.2】【RQ686】HCL-Kaushik 2017/09/01 Modify START.
	void* const			_r_vpcGetData,				// 取得するデータへのポインタ
	MIO_NWIF_TYPE eNwIfIndex
// 【IT5_4.2】【RQ686】HCL-Kaushik 2017/09/01 Modify END.
)
{
	// 取得するデータへのポインタがNULLであってはいけない
	DASSERT( _r_vpcGetData NE NULL );

	// データの取得
	INT32 i32AllVIDDataIndex = _i32GetAllVIDDataIndex( _r_ceVID );
	UINT32 ui32GetSize = 0;
	if ( i32AllVIDDataIndex != DUndefIndex ) {
// 【IT5_4.2】【RQ686】HCL-Kaushik 2017/09/01 Modify START.
		ui32GetSize = _m_pc_iAllVIDData[ i32AllVIDDataIndex ]->ui32Get( _r_ceVID, _r_vpcGetData ,eNwIfIndex);
// 【IT5_4.2】【RQ686】HCL-Kaushik 2017/09/01 Modify END.
	}
	return ui32GetSize;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::ui32VIDGet
// ABSTRACT	: VIDデータの取得 
// FUNCTION	: 引数にデータをセットして返す 
// NOTE		: 
// RETURN	: UINT32 ui32GetSize : 取得サイズ 
// CREATE	: 02-06-04	河辺 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
UINT32 MIBC_AllVIDMan::ui32VIDGet(
 	const VID			_r_ceVID,					// VID
 	void* const			_r_vpcGetData				// 取得するデータへのポインタ
)
{
	// データの取得
	INT32 i32AllVIDDataIndex = _i32GetAllVIDDataIndex( _r_ceVID );
	UINT32 ui32GetSize = 0;
	if ( i32AllVIDDataIndex != DUndefIndex ) {
		ui32GetSize = _m_pc_iAllVIDData[ i32AllVIDDataIndex ]->ui32VIDGet( _r_ceVID, _r_vpcGetData );
	}
	return ui32GetSize;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::vSet
// ABSTRACT	: データの設定(Stringデータの設定時のみ使用すること)
// FUNCTION	: 引数のデータを内部データにセットする
// NOTE		: MIBC_VIDDataBasicのデータ設定は、内部データの大きさと設定するデータの大きさが
//			  等しいとして実装されている。そこをこの関数で吸収する
// RETURN	: 無し
// CREATE	: 01-08-06	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_AllVIDMan::vSet(
	const VID				_r_ceVID,					// VID 
	const void* const		_r_cvpcSetData,				// 設定するデータへのポインタ
	CUINT32					_r_cui32SetDataSize,		// 設定するデータのサイズ 
	const MIBE_CharacterSet	_r_ceCharacterSet			// 文字集合(データタイプがストリングの時のみ有効) 
)
{
	if ( VIDD_kmJmJobGroup_Start < _r_ceVID AND _r_ceVID < VIDD_kmJmJobGroup_End ) {
		// 設定するデータへのポインタがNULLの時は何もしない 
		if ( _r_cvpcSetData NE NULL ) {
			INT32 i32AllVIDDataIndex = _i32GetAllVIDDataIndex( _r_ceVID );
			if ( i32AllVIDDataIndex != DUndefIndex ) {
				_m_pc_iAllVIDData[ i32AllVIDDataIndex ]->vSet( _r_cvpcSetData, _r_cui32SetDataSize, _r_ceCharacterSet );
			}
		}
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::ui32GetMaxSize
// ABSTRACT	: 最大データサイズの取得 
// FUNCTION	: 
// NOTE		: UINT32 ui32MaxDataSize : 最大データサイズ
// RETURN	: 
// CREATE	: 01-08-06	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
UINT32 MIBC_AllVIDMan::ui32GetMaxSize(
	const VID	_r_ceVID						// VID
) const
{
	// データの取得
	INT32 i32AllVIDDataIndex = _i32GetAllVIDDataIndex( _r_ceVID );
	UINT32 ui32MaxDataSize = 0;
	if ( i32AllVIDDataIndex != DUndefIndex ) {
		ui32MaxDataSize = _m_pc_iAllVIDData[ i32AllVIDDataIndex ]->ui32GetMaxSize();
	}
	return ui32MaxDataSize;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::ui32GetSize
// ABSTRACT	: 現在のデータサイズの取得 
// FUNCTION	: 
// NOTE		: INT32 データサイズ
// RETURN	: 
// CREATE	: 01-06-06	廣田
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
UINT32 MIBC_AllVIDMan::ui32GetSize(
	const VID	_r_ceVID						// VID
) const
{
	// データの取得
	INT32 i32AllVIDDataIndex = _i32GetAllVIDDataIndex( _r_ceVID );
	UINT32 ui32DataSize = 0;
	if ( i32AllVIDDataIndex != DUndefIndex ) {
		ui32DataSize = _m_pc_iAllVIDData[ i32AllVIDDataIndex ]->ui32GetSize();
	}
	return ui32DataSize;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::eGetDataType
// ABSTRACT	: データタイプの取得 
// FUNCTION	: 引数で指定されるVIDに相当するMIOMsgIDを取得する 
// NOTE		: 
// RETURN	: MIO_MSG_ID 
// CREATE	: 01-06-06	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
MIBE_APIDataType MIBC_AllVIDMan::eGetDataType(
	const VID	_r_ceVID						// VID
) const
{
	// データの取得
	MIBE_APIDataType eDataType = MIBE_APIDataType_INT32;
	INT32 i32AllVIDDataIndex = _i32GetAllVIDDataIndex( _r_ceVID );
	if ( i32AllVIDDataIndex != DUndefIndex ) {
		eDataType = MIBE_APIDataType(*_m_pc_iAllVIDData[ i32AllVIDDataIndex ]);
	}
	return eDataType;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: NMIBC_AllVIDMan::eGetCharacterSet
// ABSTRACT	: 文字集合の取得 
// FUNCTION	: 引数で指定されるVIDに相当する文字集合を取得する 
// NOTE		: 
// RETURN	: MIBE_CharacterSet 
// CREATE	: 01-06-11	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
MIBE_CharacterSet MIBC_AllVIDMan::eGetCharacterSet(
	const VID	_r_ceVID						// VID
) const
{
	// データの取得
	MIBE_CharacterSet eCharacterSet = MIBE_CharacterSet_USASCII;
	INT32 i32AllVIDDataIndex = _i32GetAllVIDDataIndex( _r_ceVID );
	if ( i32AllVIDDataIndex != DUndefIndex ) {
		eCharacterSet = MIBE_CharacterSet(*_m_pc_iAllVIDData[ i32AllVIDDataIndex ]);
	}
	return eCharacterSet;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::ui8GetNumOfIndex
// ABSTRACT	: Indexの数の取得 
// FUNCTION	: 
// NOTE		: UINT8 ui8NumOfIndex : Indexの数の取得 
// RETURN	: 
// CREATE	: 01-08-07	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
UINT8 MIBC_AllVIDMan::ui8GetNumOfIndex(
	const VID	_r_ceVID						// VID
) const
{
	// データの取得
	UINT8 ui8NumOfIndex = 0;
	INT32 i32AllVIDDataIndex = _i32GetAllVIDDataIndex( _r_ceVID );
	if ( i32AllVIDDataIndex != DUndefIndex ) {
		ui8NumOfIndex = _m_pc_iAllVIDData[ i32AllVIDDataIndex ]->ui8GetNumOfIndex();
	}
	return ui8NumOfIndex;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::vSetIndex
// ABSTRACT	: Indexの設定 
// FUNCTION	: 引数のIndexをセットする 
// NOTE		: 
// RETURN	: 無し 
// CREATE	: 01-12-17	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_AllVIDMan::vSetIndex(
	const VID		_r_ceVID,						// VID
	INT32*			_r_i32pIndex					// Index 
)
{
	// データの取得
	INT32 i32AllVIDDataIndex = _i32GetAllVIDDataIndex( _r_ceVID );
	if ( i32AllVIDDataIndex != DUndefIndex ) {
		_m_pc_iAllVIDData[ i32AllVIDDataIndex ]->vSetIndex( _r_i32pIndex );
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::vGetIndex
// ABSTRACT	: Indexの取得 
// FUNCTION	: 引数にIndexをセットして返す 
// NOTE		: 
// RETURN	: 無し 
// CREATE	: 01-08-07	廣田 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_AllVIDMan::vGetIndex(
	const VID		_r_ceVID,						// VID
	INT32*			_r_i32pIndex					// Index 
) const
{
	// データの取得
	INT32 i32AllVIDDataIndex = _i32GetAllVIDDataIndex( _r_ceVID );
	if ( i32AllVIDDataIndex != DUndefIndex ) {
		_m_pc_iAllVIDData[ i32AllVIDDataIndex ]->vGetIndex( _r_i32pIndex );
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::eGetCmdMIOMsgID
// ABSTRACT	: VIDからMIOMsgIDへの変換
// FUNCTION	: 引数で指定されるVIDに相当するMIO Cmd Msg IDを取得する
// NOTE		: 
// RETURN	: MIO_MSG_ID
// CREATE	: 01-05-31	廣田
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
MIO_MSG_ID MIBC_AllVIDMan::eGetCmdMIOMsgID(
	const VID	_r_ceVID						// VID
) const
{
	// データの取得
	MIO_MSG_ID eMIOMsgID = MIBD_UndefMIOMsgID;			// ???
	INT32 i32AllVIDDataIndex = _i32GetAllVIDDataIndex( _r_ceVID );
	if ( i32AllVIDDataIndex != DUndefIndex ) {
		eMIOMsgID = _m_pc_iAllVIDData[ i32AllVIDDataIndex ]->eGetCmdMIOMsgID();
	}
	return eMIOMsgID;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::eGetReqMIOMsgID
// ABSTRACT	: VIDからMIOMsgIDへの変換
// FUNCTION	: 引数で指定されるVIDに相当するMIO Req Msg IDを取得する
// NOTE		: 
// RETURN	: MIO_MSG_ID
// CREATE	: 01-05-31	廣田
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
MIO_MSG_ID MIBC_AllVIDMan::eGetReqMIOMsgID(
	const VID	_r_ceVID						// VID
) const
{
	// データの取得
	MIO_MSG_ID eMIOMsgID = MIBD_UndefMIOMsgID;			// ???
	INT32 i32AllVIDDataIndex = _i32GetAllVIDDataIndex( _r_ceVID );
	if ( i32AllVIDDataIndex != DUndefIndex ) {
		eMIOMsgID = _m_pc_iAllVIDData[ i32AllVIDDataIndex ]->eGetReqMIOMsgID();
	}
	return eMIOMsgID;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::i32GetValueStr
// ABSTRACT	: 値を文字として取得 
// FUNCTION	: 引数にデータをセットして返す。
// NOTE		: 
// RETURN	: UINT32 ui32GetSize : 取得サイズ 
// CREATE	: 01-08-06	廣田 
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
UINT32 MIBC_AllVIDMan::ui32GetValueStr(
	const VID			_r_ceVID,					// VID
	void* const			_r_vpcGetData				// 取得するデータへのポインタ
)
{
	// 引数のチェック 
	// .. 取得するデータへのポインタがNULLであってはいけない 
	DASSERT( _r_vpcGetData NE NULL );

	UINT32 ui32GetSize = 0;
	INT32 i32AllVIDDataIndex = _i32GetAllVIDDataIndex( _r_ceVID );
	if ( i32AllVIDDataIndex != DUndefIndex ) {
		ui32GetSize = _m_pc_iAllVIDData[ i32AllVIDDataIndex ]->ui32GetValueStr( _r_ceVID, _r_vpcGetData );
	}
	return ui32GetSize;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::ui8GetVIDPartsCnvTblSize
// ABSTRACT	: VIDパーツ変換情報テーブルサイズの取得 
// FUNCTION	: 
// NOTE		: 
// RETURN	: UINT8 
// CREATE	: 01-08-06	廣田 
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
UINT8 MIBC_AllVIDMan::ui8GetVIDPartsCnvTblSize(
	const VID					_r_ceVID					// VID 
)
{
	UINT8 ui8VIDPartsCnvTblSize = 0;
	INT32 i32AllVIDDataIndex = _i32GetAllVIDDataIndex( _r_ceVID );
	if ( i32AllVIDDataIndex != DUndefIndex ) {
		ui8VIDPartsCnvTblSize = _m_pc_iAllVIDData[ i32AllVIDDataIndex ]->ui8GetVIDPartsCnvTblSize();
	}
	return ui8VIDPartsCnvTblSize;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::i32GetValueToStr
// ABSTRACT	: VIDパーツ変換情報テーブルの指定インデックスに対応する文字列の取得 
// FUNCTION	: 引数にデータをセットして返す。
// NOTE		: ストリング型以外の型で使用すること 
// RETURN	: UINT32 ui32GetSize : 取得サイズ 
// CREATE	: 01-08-06	廣田 
//			  新規作成
/////////////////////////////////////////////////////////////////////////////////////////
UINT32 MIBC_AllVIDMan::ui32GetValueToStr(
	const VID			_r_ceVID,					// VID
	CUINT8				_r_cui8TblIndex,			// VIDパーツ変換情報テーブルの指定インデックス 
	const MIBE_Language	_r_ceLanguage,				// 言語 
	UINT8* const		_r_vpcGetData,				// 値(INT32)に対する文字列 
	UINT8* const		_r_vpcValueStr,				// 値(INT32)を文字列化したもの 
	Bool&				_r_risSelect				// その値が現在選択されている値か 
)
{
	// 引数のチェック 
	// .. 取得するデータへのポインタがNULLであってはいけない 
	DASSERT( _r_vpcGetData NE NULL );

	INT32 i32AllVIDDataIndex = _i32GetAllVIDDataIndex( _r_ceVID );
	UINT32 ui32GetSize = 0;
	if ( i32AllVIDDataIndex != DUndefIndex ) {
		ui32GetSize = _m_pc_iAllVIDData[ i32AllVIDDataIndex ]->ui32GetValueToStr(
			_r_ceVID, _r_cui8TblIndex, _r_ceLanguage, _r_vpcGetData, _r_vpcValueStr, _r_risSelect );
	}
	return ui32GetSize;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::_i32GetAllVIDDataIndex
// ABSTRACT	: インデックスの取得
// FUNCTION	: 引数で指定されるVIDに相当する、全VIDデータ_m_pc_iAllVIDDataのインデックスを
//			  取得する
// NOTE		: チェック無しでインラインにでもしてください 
// RETURN	: INT32 DUndefIndex : 該当するデータ無し
//								  該当するデータのインデックス番号
// CREATE	: 01-05-30	廣田		新規作成
//			  02-01-17	廣田
//				高速化のため検索をやめ、VIDとVIDTblが１対１になるように変更
/////////////////////////////////////////////////////////////////////////////////////////
INT32 MIBC_AllVIDMan::_i32GetAllVIDDataIndex(
	const VID			_r_ceVID					// VID
) const
{
	// 引数のチェック
	// ..VIDが未定義ではあってはいけない
	DASSERT( VIDD_VID_Start < _r_ceVID AND _r_ceVID < VIDD_VID_End );

	// VIDとVIDTblのインデックスは１対１に対応している 
	INT32 i32AllVIDDataIndex = _r_ceVID;

	// 全VIDテーブル(_m_pc_iAllVIDData)で、VIDデータが定義されているかどうか 
	if ( !MIBD_IsVIDDataExist( _m_pc_iAllVIDData[ i32AllVIDDataIndex ] ) ) {
		i32AllVIDDataIndex = DUndefIndex;		// 該当するデータ無し
	}
	return	i32AllVIDDataIndex;
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::eIJCSet
// ABSTRACT	: データの設定
// FUNCTION	: 引数のデータをIJCデータにセットする
// NOTE		: 
// RETURN	: 無し
// CREATE	: 02-05-17	河辺
//			  新規作成 
//			  02-06-10	河辺
//			  nmIJCIF.cppで全VIDデータの処理を行うように変更 
/////////////////////////////////////////////////////////////////////////////////////////
MIBE_CmdResult MIBC_AllVIDMan::eIJCSet(
	const VID				_r_ceVID,					// VID
	const void* const		_r_cvpcSetData,				// 設定するデータへのポインタ
	CUINT32					_r_cui32SetDataSize,		// 設定するデータのサイズ 
	const MIBE_CharacterSet	_r_ceCharacterSet			// 文字集合(データタイプがストリングの時のみ有効) 
)
{
	if ( _r_ceVID NE VIDD_VID_Undef ) {
		return in_eProcSetAllVIDData( _r_ceVID, _r_cvpcSetData, _r_cui32SetDataSize );
	} else {
		return MIBE_CmdResult_NO_SUCH_NAME;
	}
}
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::isVIDDataExist
// ABSTRACT	: VIDデータが存在するかどうか
// FUNCTION	: 
// NOTE		: 
// RETURN	: Bool : True		VIDデータが存在する
//					 False			　　　 存しない
// CREATE	: 	02-05-17	河辺	
//				新規作成
/////////////////////////////////////////////////////////////////////////////////////////
Bool MIBC_AllVIDMan::isVIDDataExist(
	const VID			_r_ceVID					// VID
)
{
	return	(char)(MIBD_IsVIDDataExist( _m_pc_iAllVIDData[ _r_ceVID ] ));
}


/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::vGetJobData
// ABSTRACT	: NM内部のジョブ情報へのデータセット 
// FUNCTION	: 引数のデータをNM内部の情報にセットする
// NOTE		: 
// RETURN	: 無し
// CREATE	: 03-03-12	松本 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
INT32 MIBC_AllVIDMan::vGetJobData(
	CINT32 					_r_index,		// ゲットするテーブル番号 
	const VID				_r_ceVID,		// VID 
	const void* const		_r_cvpGetData	// 取得するデータへのポインタ 
)
{
	INT32 data = sizeof(INT32);
	if ( (0 > _r_index) OR (MIBD_Max_kmJmJobIndex < _r_index) ) {
		return 0;
	} else if ( _r_index < MIBD_Max_ActiveJobListAll ) {
		// アクティブジョブデータから取得 
		switch ( _r_ceVID ) {
			case VIDD_kmJmJobJobSetIndex_1:			// ジョブセットID 
				*((INT32*)_r_cvpGetData) = s_JobDataTblActive[_r_index].i8JmJobJobSetIndex;
				break;
			case VIDD_kmJmJobIndex_1:				// ジョブID 
				*((INT32*)_r_cvpGetData) = s_JobDataTblActive[_r_index].i32JmJobIndex;
				break;
			case VIDD_kmJmJobFunction_1:			// 機能分類 
				*((INT32*)_r_cvpGetData) = s_JobDataTblActive[_r_index].i8JmJobFunction;
				break;
			case VIDD_kmJmJobStatus_1:				// 状態 
				*((INT32*)_r_cvpGetData) = s_JobDataTblActive[_r_index].i8JmJobStatus;
				break;
			case VIDD_kmJmJobStatusDetails_1:		// 詳細状態 
				strcpy( (INT8*)_r_cvpGetData, s_JobDataTblActive[_r_index].i8JmJobStatusDetails );
				data = (INT32)strlen((INT8*)_r_cvpGetData);
				break;
			case VIDD_kmJmJobReceivedTime_1:		// 受付時間 
				memcpy( (INT8*)_r_cvpGetData, s_JobDataTblActive[_r_index].i8JmJobReceivedTime, VIDD_Max_kmJmJobReceivedTime );
				if ( s_JobDataTblActive[_r_index].i8JmJobReceivedTime[0] == '\0' ) {
					data = 0;
				} else {
 					data = VIDD_Max_kmJmJobReceivedTime;
 				}
				break;
			case VIDD_kmJmJobCompleteTime_1:		// 完了時間 
				memcpy( (INT8*)_r_cvpGetData, s_JobDataTblActive[_r_index].i8JmJobCompleteTime, VIDD_Max_kmJmJobCompleteTime );
				if ( s_JobDataTblActive[_r_index].i8JmJobCompleteTime[0] == '\0' ) {
					data = 0;
				} else {
					data = VIDD_Max_kmJmJobCompleteTime;
				}
				break;
			case VIDD_kmJmJobPriority_1:			// 優先順位 
				*((INT32*)_r_cvpGetData) = s_JobDataTblActive[_r_index].i8JmJobPriority;
				break;
			case VIDD_kmJmJobDivNumber_1:			// 部門番号 
				strcpy( (INT8*)_r_cvpGetData, s_JobDataTblActive[_r_index].i8JmJobDivNumber );
				data = (INT32)strlen((INT8*)_r_cvpGetData);
				break;
			case VIDD_kmJmJobOwner_1:				// オーナー 
				strcpy( (INT8*)_r_cvpGetData, s_JobDataTblActive[_r_index].i8JmJobOwner );
				data = (INT32)strlen((INT8*)_r_cvpGetData);
				break;
			case VIDD_kmJmJobType_1:				// 種類 
				*((INT32*)_r_cvpGetData) = s_JobDataTblActive[_r_index].i16JmJobType;
				break;
			case VIDD_kmJmJobName_1:				// オーナー 
				strcpy( (INT8*)_r_cvpGetData, s_JobDataTblActive[_r_index].i8JmJobName );
				data = (INT32)strlen((INT8*)_r_cvpGetData);
				break;
			case VIDD_kmJmJobDocPageNumbers_1:		// 枚数 
				*((INT32*)_r_cvpGetData) = s_JobDataTblActive[_r_index].i16JmJobDocPageNumbers;
				break;
			case VIDD_kmJmJobDocCopyNumbers_1:		// 部数 
				*((INT32*)_r_cvpGetData) = s_JobDataTblActive[_r_index].i32JmJobDocCopyNumbers;
				break;
			case VIDD_kmJmJobPageSize_1:			// 出力用紙サイズ、スキャン画像サイズ 
				*((INT32*)_r_cvpGetData) = s_JobDataTblActive[_r_index].i32JmJobPageSize;
				break;
			case VIDD_kmJmJobDestination_1:			// 排紙先/配信先 
				strcpy( (INT8*)_r_cvpGetData, s_JobDataTblActive[_r_index].i8JmJobDestination );
				data = (INT32)strlen((INT8*)_r_cvpGetData);
				break;
			case VIDD_kmJmJobDriverJobID_1: 		// Printer JobID 
				strcpy( (INT8*)_r_cvpGetData, s_JobDataTblActive[_r_index].i8JmJobDriverJobID );
				data = (INT32)strlen((INT8*)_r_cvpGetData);
				break;
//	【IT5_3.3】 【1002836】	HCL-Bairavi 2016/07/22 Modified Start.
// 【MinervaSBK】【EMU72コンパイルエラー対応】OPS1深谷 2015/05/27 START.
// 【特ROM対応依頼(MBK-25)】　印刷済み部数のMIB追加対応 Poseidon Vaibhav 2014/07/17 START.
			case VIDD_kmJmJobOutputDocCopyNumbers_1:	// 印刷済み部数 
				*((INT32*)_r_cvpGetData) = s_JobDataTblActive[_r_index].i32JmJobOutputDocCopyNumbers;
				break;
// 【特ROM対応依頼(MBK-25)】　印刷済み部数のMIB追加対応 Poseidon Vaibhav 2014/07/17 END.
// 【MinervaSBK】【EMU72コンパイルエラー対応】OPS1深谷 2015/05/27 END.
//	【IT5_3.3】 【1002836】	HCL-Bairavi 2016/07/22 Modified End.
			default:	break;
		}
	} else if( _r_index < ( MIBD_Max_ActiveJobListAll+MIBD_Max_JobLogListAll) ) { //			//2016/09/07 HCL-Sankar Static Analysis Fix 1002990.
		// 履歴ジョブデータから取得 
		switch ( _r_ceVID ) {
			case VIDD_kmJmJobJobSetIndex_1:			// ジョブセットID 
				*((INT32*)_r_cvpGetData) = s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i8JmJobJobSetIndex;
				break;
			case VIDD_kmJmJobIndex_1:				// ジョブID 
				*((INT32*)_r_cvpGetData) = s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i32JmJobIndex;
				break;
			case VIDD_kmJmJobFunction_1:			// 機能分類 
				*((INT32*)_r_cvpGetData) = s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i8JmJobFunction;
				break;
			case VIDD_kmJmJobStatus_1:				// 状態 
				*((INT32*)_r_cvpGetData) = s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i8JmJobStatus;
				break;
			case VIDD_kmJmJobStatusDetails_1:		// 詳細状態 
				strcpy( (INT8*)_r_cvpGetData, s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i8JmJobStatusDetails );
				data = (INT32)strlen((INT8*)_r_cvpGetData);
				break;
			case VIDD_kmJmJobReceivedTime_1:		// 受付時間 
				memcpy( (INT8*)_r_cvpGetData, s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i8JmJobReceivedTime, VIDD_Max_kmJmJobReceivedTime );
				if ( s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i8JmJobReceivedTime[0] == '\0' ) {
					data = 0;
				} else {
 					data = VIDD_Max_kmJmJobReceivedTime;
 				}
				break;
			case VIDD_kmJmJobCompleteTime_1:		// 完了時間 
				memcpy( (INT8*)_r_cvpGetData, s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i8JmJobCompleteTime, VIDD_Max_kmJmJobCompleteTime );
				if ( s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i8JmJobCompleteTime[0] == '\0' ) {
					data = 0;
				} else {
 					data = VIDD_Max_kmJmJobCompleteTime;
 				}
				break;
			case VIDD_kmJmJobPriority_1:			// 優先順位 
				*((INT32*)_r_cvpGetData) = s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i8JmJobPriority;
				break;
			case VIDD_kmJmJobDivNumber_1:			// 部門番号 
				strcpy( (INT8*)_r_cvpGetData, s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i8JmJobDivNumber );
				data = (INT32)strlen((INT8*)_r_cvpGetData);
				break;
			case VIDD_kmJmJobOwner_1:				// オーナー 
				strcpy( (INT8*)_r_cvpGetData, s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i8JmJobOwner );
				data = (INT32)strlen((INT8*)_r_cvpGetData);
				break;
			case VIDD_kmJmJobType_1:				// 種類 
				*((INT32*)_r_cvpGetData) = s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i16JmJobType;
				break;
			case VIDD_kmJmJobName_1:				// オーナー 
				strcpy( (INT8*)_r_cvpGetData, s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i8JmJobName );
				data = (INT32)strlen((INT8*)_r_cvpGetData);
				break;
			case VIDD_kmJmJobDocPageNumbers_1:		// 枚数 
				*((INT32*)_r_cvpGetData) = s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i16JmJobDocPageNumbers;
				break;
			case VIDD_kmJmJobDocCopyNumbers_1:		// 部数 
				*((INT32*)_r_cvpGetData) = s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i32JmJobDocCopyNumbers;
				break;
			case VIDD_kmJmJobPageSize_1:			// 出力用紙サイズ、スキャン画像サイズ 
				*((INT32*)_r_cvpGetData) = s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i32JmJobPageSize;
				break;
			case VIDD_kmJmJobDestination_1:			// 排紙先/配信先 
				strcpy( (INT8*)_r_cvpGetData, s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i8JmJobDestination );
				data = (INT32)strlen((INT8*)_r_cvpGetData);
				break;
			case VIDD_kmJmJobDriverJobID_1: 		// Printer JobID 
				strcpy( (INT8*)_r_cvpGetData, s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i8JmJobDriverJobID );
				data = (INT32)strlen((INT8*)_r_cvpGetData);
				break;
//	【IT5_3.3】 【1002836】	HCL-Bairavi 2016/07/22 Modified Start.
// 【MinervaSBK】【EMU72コンパイルエラー対応】OPS1深谷 2015/05/27 START.
// 【特ROM対応依頼(MBK-25)】　印刷済み部数のMIB追加対応 Poseidon Vaibhav 2014/07/17 START.
			case VIDD_kmJmJobOutputDocCopyNumbers_1:	// 印刷済み部数 
				*((INT32*)_r_cvpGetData) = s_JobDataTblLog[_r_index-MIBD_Max_ActiveJobListAll].i32JmJobOutputDocCopyNumbers;
				break;
// // 【特ROM対応依頼(MBK-25)】　印刷済み部数のMIB追加対応 Poseidon Vaibhav 2014/07/17 END.
// 【MinervaSBK】【EMU72コンパイルエラー対応】OPS1深谷 2015/05/27 END.
//	【IT5_3.3】 【1002836】	HCL-Bairavi 2016/07/22 Modified End.
			default:	break;
		}
	}

	return data;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::vGetSerchJobData
// ABSTRACT	: NM内部のジョブ情報のJobIDを保持するデータインデックスの取得 
// FUNCTION	: 指定されたJobIDを保持するテーブルインデックス値を返す 
// NOTE		: 
// RETURN	: テーブルインデックス
// CREATE	: 03-03-12	松本 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
INT32 MIBC_AllVIDMan::vGetSerchJobData(
	CINT32 			_r_index		// 取得するJobID 
)
{
	INT32 data = -1;
	for ( INT32 index = 0; index < MIBD_Max_ActiveJobListAll; index++ ) {
		if ( s_JobDataTblActive[index].i32JmJobIndex == _r_index ) {
			data = index;
			break;
		} else if ( s_JobDataTblActive[index].i32JmJobIndex == 0 ) {
			break;
		}
	}
	
	if ( data == -1 ) {
		for ( INT32 index = 0; index < MIBD_Max_JobLogListAll; index++ ) {
			if ( s_JobDataTblLog[index].i32JmJobIndex == _r_index ) {
				data = MIBD_Max_ActiveJobListAll + index;
				break;
			} else if ( s_JobDataTblLog[index].i32JmJobIndex == 0 ) {
				break;
			}
		}
	}
	
	return data;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::vInitJobTbl
// ABSTRACT	: NM内部のジョブ情報の初期化 
// FUNCTION	: NM内部のジョブ情報の初期化 
// NOTE		: 
// RETURN	: 
// CREATE	: 03-07-08	松本 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_AllVIDMan::vInitJobTbl()
{
	// アクティブジョブテーブル初期化 
	for ( INT32 index = 0; index < MIBD_Max_ActiveJobListAll; index++ ) {
		s_JobDataTblActive[index].i8JmJobJobSetIndex = -1;					// ジョブセットID 
		s_JobDataTblActive[index].i32JmJobIndex = MIBD_UndefJobID;			// ジョブID 
		s_JobDataTblActive[index].i8JmJobFunction = -1;						// 機能分類 
		s_JobDataTblActive[index].i8JmJobStatus = -1;						// 状態 
		s_JobDataTblActive[index].i8JmJobStatusDetails[0] = '\0';			// 詳細状態 
		s_JobDataTblActive[index].i8JmJobReceivedTime[0] = '\0';			// 受付時間 
		s_JobDataTblActive[index].i8JmJobCompleteTime[0] = '\0';			// 完了時間 
		s_JobDataTblActive[index].i8JmJobPriority = -1;						// 優先順位 
		s_JobDataTblActive[index].i8JmJobDivNumber[0] = '\0';				// 部門番号 
		s_JobDataTblActive[index].i8JmJobOwner[0] = '\0';					// オーナー 
		s_JobDataTblActive[index].i16JmJobType = -1;							// 種類 
		s_JobDataTblActive[index].i8JmJobName[0] = '\0';					// ジョブ名 
		s_JobDataTblActive[index].i16JmJobDocPageNumbers = -1;				// 枚数 
		s_JobDataTblActive[index].i32JmJobDocCopyNumbers = -1;				// 部数 
		s_JobDataTblActive[index].i32JmJobPageSize = -1;					// 出力用紙サイズ、スキャン画像サイズ 
		s_JobDataTblActive[index].i8JmJobDestination[0] = '\0';				// 排紙先/配信先 
		s_JobDataTblActive[index].i8JmJobDriverJobID[0] = '\0';				// JobDriverJobID 
// 【特ROM対応依頼(MBK-25)】　印刷済み部数のMIB追加対応 Poseidon Vaibhav 2014/07/17 START.
		s_JobDataTblActive[index].i32JmJobOutputDocCopyNumbers = -1;		// 印刷済み部数 
// 【特ROM対応依頼(MBK-25)】　印刷済み部数のMIB追加対応 Poseidon Vaibhav 2014/07/17 END.
	}

	// 履歴ジョブテーブル初期化 
	for ( INT32 index = 0; index < MIBD_Max_JobLogListAll; index++ ) {
		s_JobDataTblLog[index].i8JmJobJobSetIndex = -1;						// ジョブセットID 
		s_JobDataTblLog[index].i32JmJobIndex = MIBD_UndefJobID;				// ジョブID 
		s_JobDataTblLog[index].i8JmJobFunction = -1;						// 機能分類 
		s_JobDataTblLog[index].i8JmJobStatus = -1;							// 状態 
		s_JobDataTblLog[index].i8JmJobStatusDetails[0] = '\0';				// 詳細状態 
		s_JobDataTblLog[index].i8JmJobReceivedTime[0] = '\0';				// 受付時間 
		s_JobDataTblLog[index].i8JmJobCompleteTime[0] = '\0';				// 完了時間 
		s_JobDataTblLog[index].i8JmJobPriority = -1;						// 優先順位 
		s_JobDataTblLog[index].i8JmJobDivNumber[0] = '\0';					// 部門番号 
		s_JobDataTblLog[index].i8JmJobOwner[0] = '\0';						// オーナー 
		s_JobDataTblLog[index].i16JmJobType = -1;							// 種類 
		s_JobDataTblLog[index].i8JmJobName[0] = '\0';						// ジョブ名 
		s_JobDataTblLog[index].i16JmJobDocPageNumbers = -1;					// 枚数 
		s_JobDataTblLog[index].i32JmJobDocCopyNumbers = -1;					// 部数 
		s_JobDataTblLog[index].i32JmJobPageSize = -1;						// 出力用紙サイズ、スキャン画像サイズ 
		s_JobDataTblLog[index].i8JmJobDestination[0] = '\0';				// 排紙先/配信先 
		s_JobDataTblLog[index].i8JmJobDriverJobID[0] = '\0';				// JobDriverJobID 
// 【特ROM対応依頼(MBK-25)】　印刷済み部数のMIB追加対応 Poseidon Vaibhav 2014/07/17 START.
		s_JobDataTblLog[index].i32JmJobOutputDocCopyNumbers = -1;			// 印刷済み部数 
// 【特ROM対応依頼(MBK-25)】　印刷済み部数のMIB追加対応 Poseidon Vaibhav 2014/07/17 END.
	}
}
/* 2008.08.18 UV-00536 sw_miya add S */
/* 2008.10.23 No.310563 sw_saku chg S */
/////////////////////////////////////////////////////////////////////////////////////////.
// MODULE	: MIBC_AllVIDMan::vGetJobMonitoringData.
// ABSTRACT	: .
// FUNCTION	: JobMonitoringDataのジョブ情報へのデータセット.
// NOTE		: 引数のデータをvGetJobMonitoringData情報にセットする.
// RETURN	: データサイズ.
// CREATE	: .
//			  新規作成.
/////////////////////////////////////////////////////////////////////////////////////////.
INT32 MIBC_AllVIDMan::vGetJobMonitoringData(
	CINT32 					_r_index,				// ゲットするテーブル番号 
	const VID				_r_ceVID,				// VID 
	const void* const		_r_cvpGetData,			// 取得するデータへのポインタ 
	INT32					_r_AttributeTypeIndex	// 属性のタイプ番号
)
{
	INT32 data = 0;
	
	/* 2008.12.09 No.312803 sw_saku chg S */
	if ( (0 > _r_index) OR (MIBD_Max_PrintJobListALL <= _r_index) ) {
		return 0;
	}else{
	/* 2008.12.09 No.312803 sw_saku chg E */
		switch ( _r_ceVID ) {
			case VIDD_jmJobIDJobSetIndex_Index1:
				*((INT32*)_r_cvpGetData) = 1;
				data = sizeof(INT32);
				break;
			case VIDD_jmJobIDJobIndex_Index1:				//JobID.
				*((INT32*)_r_cvpGetData) = (INT32)sp_PrintJobDataTblAll[_r_index]->i32JmJobIndex;
				data = sizeof(INT32);
				break;
			case VIDD_jmJobState_Index1:					//JobStatus.
				*((INT32*)_r_cvpGetData) = (INT32)sp_PrintJobDataTblAll[_r_index]->i8JmJobStatus;
				data = sizeof(INT32);
				break;
			case VIDD_jmJobStateReasons1_Index1:			//Jobの詳細情報.
				*((INT32*)_r_cvpGetData) = (INT32)sp_PrintJobDataTblAll[_r_index]->i32JmJobStateReasons;
				data = sizeof(INT32);
				break;
			case VIDD_jmJobOwner_Index1:					//ユーザ名.
				strcpy((char*)_r_cvpGetData, &sp_PrintJobDataTblAll[_r_index]->i8JmJobOwner[0]);
				data = (INT32)strlen((char*)_r_cvpGetData);
				break;
			case VIDD_jmAttributeValueAsOctets_Index1:		//Job属性.
				switch(_r_AttributeTypeIndex){
					case MIBE_AttributeOther:									// other(1).
						strcpy((char*)_r_cvpGetData, &sp_PrintJobDataTblAll[_r_index]->i8jmAttributeValue1[0]);
						data = (INT32)strlen((char*)_r_cvpGetData);
						break;
					case MIBE_AttributeJobAccountName:							// jobAccountName(21).
						strcpy((char*)_r_cvpGetData, &sp_PrintJobDataTblAll[_r_index]->i8jmAttributeValue21[0]);
						data = (INT32)strlen((char*)_r_cvpGetData);
						break;
					case MIBE_AttributeJobName:									// jobName(23).
						strcpy((char*)_r_cvpGetData, &sp_PrintJobDataTblAll[_r_index]->i8jmAttributeValue23[0]);
						data = (INT32)strlen((char*)_r_cvpGetData);
						break;
					case MIBE_AttributeDeviceNameRequested:						// deviceNameRequested(30).
						strcpy((char*)_r_cvpGetData, &sp_PrintJobDataTblAll[_r_index]->i8jmAttributeValue30[0]);
						data = (INT32)strlen((char*)_r_cvpGetData);
						break;
					case MIBE_AttributeFileName:								// fileName(34).
						strcpy((char*)_r_cvpGetData, &sp_PrintJobDataTblAll[_r_index]->i8jmAttributeValue34[0]);
						data = (INT32)strlen((char*)_r_cvpGetData);
						break;
					case MIBE_AttributeDocumentName:							// documentName(35).
						strcpy((char*)_r_cvpGetData, &sp_PrintJobDataTblAll[_r_index]->i8jmAttributeValue35[0]);
						data = (INT32)strlen((char*)_r_cvpGetData);
						break;
					case MIBE_AttributeOutputBin:								// outputBin(54).
						strcpy((char*)_r_cvpGetData, &sp_PrintJobDataTblAll[_r_index]->i8jmAttributeValue54[0]);
						data = (INT32)strlen((char*)_r_cvpGetData);
						break;
					case MIBE_AttributeMediumRequested:							// mediumRequested(170).
						strcpy((char*)_r_cvpGetData, &sp_PrintJobDataTblAll[_r_index]->i8jmAttributeValue170[0]);
						data = (INT32)strlen((char*)_r_cvpGetData);
						break;
					case MIBE_AttributeMediumConsumed:							// mediumConsumed(171).
						strcpy((char*)_r_cvpGetData, &sp_PrintJobDataTblAll[_r_index]->i8jmAttributeValue171[0]);
						data = (INT32)strlen((char*)_r_cvpGetData);
						break;
					case MIBE_AttributeColorantRequested:						// colorantRequested(172).
						strcpy((char*)_r_cvpGetData, &sp_PrintJobDataTblAll[_r_index]->i8jmAttributeValue172[0]);
						data = (INT32)strlen((char*)_r_cvpGetData);
						break;
					case MIBE_AttributeColorantConsumed:						// colorantConsumed(173).
						strcpy((char*)_r_cvpGetData, &sp_PrintJobDataTblAll[_r_index]->i8jmAttributeValue173[0]);
						data = (INT32)strlen((char*)_r_cvpGetData);
						break;
					case MIBE_AttributeMediumTypeConsumed:						// mediumTypeConsumed(174).
						strcpy((char*)_r_cvpGetData, &sp_PrintJobDataTblAll[_r_index]->i8jmAttributeValue174[0]);
						data = (INT32)strlen((char*)_r_cvpGetData);
						break;
					case MIBE_AttributeMediumSizeConsumed:						// mediumSizeConsumed(175).
						strcpy((char*)_r_cvpGetData, &sp_PrintJobDataTblAll[_r_index]->i8jmAttributeValue175[0]);
						data = (INT32)strlen((char*)_r_cvpGetData);
						break;
					case MIBE_AttributeJobStartedProcessingTime:				// jobStartedProcessingTime(193).
						strcpy((char*)_r_cvpGetData, &sp_PrintJobDataTblAll[_r_index]->i8jmAttributeValue193[0]);
						data = (INT32)11;										//履歴の登録時間.
						break;
					default:
						break;
				}
				break;
			default:
				break;
		}
	}
	
	return data;
	/* 2008.10.23 No.310563 sw_saku chg E */
}

/////////////////////////////////////////////////////////////////////////////////////////.
// MODULE	: MIBC_AllVIDMan::vCheckJobMonitoringData.
// ABSTRACT	: ジョブ情報のJobIDを保持するデータインデックスの取得.
// FUNCTION	: 指定されたJobIDを保持するテーブルインデックス値を返す.
// NOTE		: 
// RETURN	: テーブルインデックス.
// CREATE	: 
//			  新規作成.
/////////////////////////////////////////////////////////////////////////////////////////.
INT32 MIBC_AllVIDMan::vCheckJobMonitoringData(
	CINT32 				_r_index					// 取得するJobID.
)
{
	INT32	i32Result = -1;						//返却用.
	INT32	i32SearchJmJobIndex = s_PrintJobDataTblActive[_r_index].i32JmJobIndex;
	Bool	b_flag = False;
	INT32 	index;
	
	for( index = 0; index < MIBD_Max_JobLogListAll; index++ ){
		if( s_PrintJobDataTblLog[index].i32JmJobIndex == i32SearchJmJobIndex ){
			 b_flag = True;
			break;
		}
	}
	
	//同じIndexがあった場合.
	if(b_flag == True ){
		i32Result = index;
	}
	
	return i32Result;
}

/////////////////////////////////////////////////////////////////////////////////////////.
// MODULE	: MIBC_AllVIDMan::vInitJobMonitoringTbl.
// ABSTRACT	: JobMonitoring関連情報の初期化.
// FUNCTION	: JobMonitoring関連情報の初期化.
// NOTE		: 
// RETURN	: 
// CREATE	: 
//			  新規作成.
/////////////////////////////////////////////////////////////////////////////////////////.
void MIBC_AllVIDMan::vInitJobMonitoringTbl()
{
	// アクティブジョブテーブル初期化.
	for ( INT32 _r_index = 0; _r_index < MIBD_Max_ActiveJobListAll; _r_index++ ) {
		s_PrintJobDataTblActive[_r_index].i32JmJobIndex = MIBD_UndefJobID;			//JobID.
		s_PrintJobDataTblActive[_r_index].i8JmJobStatus = -1;						//JobStatus.
		s_PrintJobDataTblActive[_r_index].i32JmJobStateReasons = -1;				//Jobの詳細情報.
		s_PrintJobDataTblActive[_r_index].i8JmJobOwner[0] = '\0';					//ユーザ名.
		s_PrintJobDataTblActive[_r_index].i8jmAttributeValue1[0] = '\0';			// other(1).
		s_PrintJobDataTblActive[_r_index].i8jmAttributeValue21[0] = '\0';			// jobAccountName(21).
		s_PrintJobDataTblActive[_r_index].i8jmAttributeValue23[0] = '\0';			// jobName(23).
		s_PrintJobDataTblActive[_r_index].i8jmAttributeValue30[0] = '\0';			// deviceNameRequested(30).
		s_PrintJobDataTblActive[_r_index].i8jmAttributeValue34[0] = '\0';			// fileName(34).
		s_PrintJobDataTblActive[_r_index].i8jmAttributeValue35[0] = '\0';			// documentName(35).
		s_PrintJobDataTblActive[_r_index].i8jmAttributeValue54[0] = '\0';			// outputBin(54).
		s_PrintJobDataTblActive[_r_index].i8jmAttributeValue170[0] = '\0';			// mediumRequested(170).
		s_PrintJobDataTblActive[_r_index].i8jmAttributeValue171[0] = '\0';			// mediumConsumed(171).
		s_PrintJobDataTblActive[_r_index].i8jmAttributeValue172[0] = '\0';			// colorantRequested(172).
		s_PrintJobDataTblActive[_r_index].i8jmAttributeValue173[0] = '\0';			// colorantConsumed(173).
		s_PrintJobDataTblActive[_r_index].i8jmAttributeValue174[0] = '\0';			// mediumTypeConsumed(174).
		s_PrintJobDataTblActive[_r_index].i8jmAttributeValue175[0] = '\0';			// mediumSizeConsumed(175).
		s_PrintJobDataTblActive[_r_index].i8jmAttributeValue193[0] = '\0';			// jobStartedProcessingTime(193).
	}
	
	// 履歴ジョブテーブル初期化.
	for ( INT32 _r_index = 0; _r_index < MIBD_Max_JobLogListAll; _r_index++ ) {
		s_PrintJobDataTblLog[_r_index].i32JmJobIndex = MIBD_UndefJobID;				//JobID.
		s_PrintJobDataTblLog[_r_index].i8JmJobStatus = -1;							//JobStatus.
		s_PrintJobDataTblLog[_r_index].i32JmJobStateReasons = -1;					//Jobの詳細情報.
		s_PrintJobDataTblLog[_r_index].i8JmJobOwner[0] = '\0';						//ユーザ名.
		s_PrintJobDataTblLog[_r_index].i8jmAttributeValue1[0] = '\0';				// other(1).
		s_PrintJobDataTblLog[_r_index].i8jmAttributeValue21[0] = '\0';				// jobAccountName(21).
		s_PrintJobDataTblLog[_r_index].i8jmAttributeValue23[0] = '\0';				// jobName(23).
		s_PrintJobDataTblLog[_r_index].i8jmAttributeValue30[0] = '\0';				// deviceNameRequested(30).
		s_PrintJobDataTblLog[_r_index].i8jmAttributeValue34[0] = '\0';				// fileName(34).
		s_PrintJobDataTblLog[_r_index].i8jmAttributeValue35[0] = '\0';				// documentName(35).
		s_PrintJobDataTblLog[_r_index].i8jmAttributeValue54[0] = '\0';				// outputBin(54).
		s_PrintJobDataTblLog[_r_index].i8jmAttributeValue170[0] = '\0';				// mediumRequested(170).
		s_PrintJobDataTblLog[_r_index].i8jmAttributeValue171[0] = '\0';				// mediumConsumed(171).
		s_PrintJobDataTblLog[_r_index].i8jmAttributeValue172[0] = '\0';				// colorantRequested(172).
		s_PrintJobDataTblLog[_r_index].i8jmAttributeValue173[0] = '\0';				// colorantConsumed(173).
		s_PrintJobDataTblLog[_r_index].i8jmAttributeValue174[0] = '\0';				// mediumTypeConsumed(174).
		s_PrintJobDataTblLog[_r_index].i8jmAttributeValue175[0] = '\0';				// mediumSizeConsumed(175).
		s_PrintJobDataTblLog[_r_index].i8jmAttributeValue193[0] = '\0';				// jobStartedProcessingTime(193).
	}
	
	return;
}


/* 2008.08.18 UV-00536 sw_miya add E */

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::vSetAllJobData
// ABSTRACT	: 内部のジョブ情報へ全ジョブデータをセット 
// FUNCTION	: 引数のジョブテーブルのデータを内部データに全てコピーする 
// NOTE		: 
// RETURN	: 
// CREATE	: 04-05-18	河辺 
//			  新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_AllVIDMan::vSetAllJobData(
	MIBE_JobList 			_r_eJobList,		// ジョブリスト種類 
	MIBS_JobDataTbl*		_r_spJobDataTbl		// ジョブデータテーブル 
)
{
	switch( _r_eJobList ) {
		case MIBE_JobList_Active:
			memcpy ( s_JobDataTblActive, _r_spJobDataTbl, sizeof(MIBS_JobDataTbl)*MIBD_Max_ActiveJobListAll );
			break;
		case MIBE_JobList_Log:
			memcpy ( s_JobDataTblLog, _r_spJobDataTbl, sizeof(MIBS_JobDataTbl)*MIBD_Max_JobLogListAll );
			break;
		default:	break;
	}
}

// ユビキタス[KRQ-278][スプール分散恒久対応][step2] start.
/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::updatePortLoadStatus 
// ABSTRACT	: ポート負荷状態の更新 
// FUNCTION	: PDLから取得した負荷状態をMIBの定義値に置換して保存する 
// NOTE		: 
// RETURN	: 
// CREATE	: 16-09-30	ユビキタスFUM 新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_AllVIDMan::updatePortLoadStatus()
{
	// PDLからポート情報を取得.
	PDLC_PortLoadStatusMap c_PortLoadStatusMap;
	APIC_PdlInterpreterCommonIF::newInstance()->getPortLoadStatus( c_PortLoadStatusMap );

	for( ushort i = 0; i < MIBD_PortTblSize; i++ ){

		MIBE_Port e_MIBPort = MIBS_PortConfigTbl[i].first;

		switch( e_MIBPort ) {
		case MIBE_PortLpr1:
		case MIBE_PortLpr2:
			setPortLoadStatus( e_MIBPort, MIBE_PortLoadStatusUnknown );							// 「不明」を設定.
			break;
		case MIBE_PortRaw1:
		case MIBE_PortRaw2:
		case MIBE_PortRaw3:
		case MIBE_PortRaw4:
		case MIBE_PortRaw5:
		case MIBE_PortRaw6:
		{
			TYPE_Port e_PdlPort = convertPortDefToPDL( e_MIBPort );								// MIBのポート定義値をPDLの定義値に変換.
			PDLC_PortLoadStatusMapItr c_Itr = c_PortLoadStatusMap.find( e_PdlPort );			// PDLのMapから負荷状態を取り出す.
			if( c_PortLoadStatusMap.end() == c_Itr ){
				break;
			}
			MIBE_PortLoadStatus e_MIBLoadStatus = convertPortLoadStatusFromPDL( c_Itr->second );// PDLの負荷状態定義値からMIBの定義値に変換.
			setPortLoadStatus( e_MIBPort, e_MIBLoadStatus );									// 負荷状態を設定.
		}
			break;
		default:
			break;
		}
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::vGetPortStatus 
// ABSTRACT	: VID毎のポート負荷状態を取得 
// FUNCTION	: 
// NOTE		: 
// RETURN	: ポート負荷状態 
// CREATE	: 16-09-30	ユビキタスFUM 新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
UINT32 MIBC_AllVIDMan::vGetPortStatus( const VID _r_ceVID, void* const _r_vpcGetData )
{
	switch( _r_ceVID ){
		case VIDD_kmPrtLocalPortStatus_Index1:		// LPR1 ユビキタス[KRQ-278][スプール分散恒久対応][step2] Accurev.1010564 .
			*((INT32*)_r_vpcGetData) = (INT32)getPortLoadStatus( MIBE_PortLpr1 );
			break;

		case VIDD_kmPrtLocalPortStatus_Index2:		// LPR2 ユビキタス[KRQ-278][スプール分散恒久対応][step2] Accurev.1010564 .
			*((INT32*)_r_vpcGetData) = (INT32)getPortLoadStatus( MIBE_PortLpr2 );
			break;

		case VIDD_kmPrtLocalPortStatus_Index3:		// RAW1 ユビキタス[KRQ-278][スプール分散恒久対応][step2] Accurev.1010564 .
			*((INT32*)_r_vpcGetData) = (INT32)getPortLoadStatus( MIBE_PortRaw1 );
			break;

		case VIDD_kmPrtLocalPortStatus_Index4:		// RAW2 ユビキタス[KRQ-278][スプール分散恒久対応][step2] Accurev.1010564 .
			*((INT32*)_r_vpcGetData) = (INT32)getPortLoadStatus( MIBE_PortRaw2 );
			break;

		case VIDD_kmPrtLocalPortStatus_Index5:		// RAW3 ユビキタス[KRQ-278][スプール分散恒久対応][step2] Accurev.1010564 .
			*((INT32*)_r_vpcGetData) = (INT32)getPortLoadStatus( MIBE_PortRaw3 );
			break;

		case VIDD_kmPrtLocalPortStatus_Index6:		// RAW4 ユビキタス[KRQ-278][スプール分散恒久対応][step2] Accurev.1010564 .
			*((INT32*)_r_vpcGetData) = (INT32)getPortLoadStatus( MIBE_PortRaw4 );
			break;

		case VIDD_kmPrtLocalPortStatus_Index7:		// RAW5 ユビキタス[KRQ-278][スプール分散恒久対応][step2] Accurev.1010564 .
			*((INT32*)_r_vpcGetData) = (INT32)getPortLoadStatus( MIBE_PortRaw5 );
			break;

		case VIDD_kmPrtLocalPortStatus_Index8:		// RAW6 ユビキタス[KRQ-278][スプール分散恒久対応][step2] Accurev.1010564 .
			*((INT32*)_r_vpcGetData) = (INT32)getPortLoadStatus( MIBE_PortRaw6 );
			break;

		default:
			break;
	}

	return sizeof(INT32);
}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::setPortLoadStatus 
// ABSTRACT	: Mapにポート負荷状態を保存 
// FUNCTION	: 
// NOTE		: 
// RETURN	: 
// CREATE	: 16-09-30	ユビキタスFUM 新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
void MIBC_AllVIDMan::setPortLoadStatus( const MIBE_Port e_Port, const MIBE_PortLoadStatus e_setLoadStatus )
{
	MIBC_PortLoadStatusMapItr c_Itr = m_c_PortLoadStatusMap.find( e_Port );			// 指定ポートをマップから検索.

	if ( m_c_PortLoadStatusMap.end() == c_Itr ) {
		m_c_PortLoadStatusMap.insert( MIBC_PortLoadStatusMapPair( e_Port, e_setLoadStatus ) );			// マップに存在しない場合追加(例外).
	}else{
		m_c_PortLoadStatusMap[e_Port] = e_setLoadStatus;							// 引数の状態に変更.
	}
}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::getPortLoadStatus 
// ABSTRACT	: Mapからポート負荷状態を取得 
// FUNCTION	: 
// NOTE		: 
// RETURN	: ポート負荷状態 
// CREATE	: 16-09-30	ユビキタスFUM 新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
MIBE_PortLoadStatus MIBC_AllVIDMan::getPortLoadStatus( const MIBE_Port e_Port )
{
	MIBE_PortLoadStatus e_retPortLoadStatus = MIBE_PortLoadStatusUnknown;

	MIBC_PortLoadStatusMapItr c_Itr = m_c_PortLoadStatusMap.find( e_Port );			// 指定ポートをマップから検索.
	if ( m_c_PortLoadStatusMap.end() != c_Itr ) {
		e_retPortLoadStatus = c_Itr->second;										// 指定ポートの負荷状態を取得.
	}

	return e_retPortLoadStatus;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::convertPortLoadStatusFromPDL 
// ABSTRACT	: PDLのポート負荷状態定義値をMIBのポート負荷常態定義値に変換 
// FUNCTION	: 
// NOTE		: 
// RETURN	: ポート負荷状態 
// CREATE	: 16-09-30	ユビキタスFUM 新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
MIBE_PortLoadStatus MIBC_AllVIDMan::convertPortLoadStatusFromPDL( const PDLE_PortLoadStatus uc_PDLPortLoadStatus )
{
	MIBE_PortLoadStatus e_MIBPortLoadStatus = MIBE_PortLoadStatusUnknown;

	switch( uc_PDLPortLoadStatus ){
		case PDLE_PortLoadStatusLow:			// 低負荷.
			e_MIBPortLoadStatus = MIBE_PortLoadStatusLow;
			break;

		case PDLE_PortLoadStatusHigh:			// 高負荷.
			e_MIBPortLoadStatus = MIBE_PortLoadStatusHigh;
			break;

		case PDLE_PortLoadStatusDisable:		// 使用不可.
			e_MIBPortLoadStatus = MIBE_PortLoadStatusDisable;
			break;

		case PDLE_PortLoadStatusUnknown:		// 未定.
			e_MIBPortLoadStatus = MIBE_PortLoadStatusUnknown;
			break;

		default:
			break;
	}
	
	return e_MIBPortLoadStatus;
}

/////////////////////////////////////////////////////////////////////////////////////////
// MODULE	: MIBC_AllVIDMan::convertPortDefToPDL 
// ABSTRACT	: MIBのポート定義値をPDLのポート定義値に変換 
// FUNCTION	: 
// NOTE		: 
// RETURN	: ポート(PDL側の定義値) 
// CREATE	: 16-09-30	ユビキタスFUM 新規作成 
/////////////////////////////////////////////////////////////////////////////////////////
TYPE_Port MIBC_AllVIDMan::convertPortDefToPDL( const MIBE_Port e_MIBPort )
{
	TYPE_Port e_PDLPort = TYPE_PortRaw1;

	switch( e_MIBPort ){
		case MIBE_PortLpr1:						// LPR1.
			e_PDLPort = TYPE_PortLpd1;
			break;
		case MIBE_PortLpr2:						// LPR2.
			e_PDLPort = TYPE_PortLpd2;
			break;
		case MIBE_PortRaw1:						// RAW1.
			e_PDLPort = TYPE_PortRaw1;
			break;
		case MIBE_PortRaw2:						// RAW2.
			e_PDLPort = TYPE_PortRaw2;
			break;
		case MIBE_PortRaw3:						// RAW3.
			e_PDLPort = TYPE_PortRaw3;
			break;
		case MIBE_PortRaw4:						// RAW4.
			e_PDLPort = TYPE_PortRaw4;
			break;
		case MIBE_PortRaw5:						// RAW5.
			e_PDLPort = TYPE_PortRaw5;
			break;
		case MIBE_PortRaw6:						// RAW6.
			e_PDLPort = TYPE_PortRaw6;
			break;
		default:
			break;
	}

	return e_PDLPort;
}
// ユビキタス[KRQ-278][スプール分散恒久対応][step2] end.
