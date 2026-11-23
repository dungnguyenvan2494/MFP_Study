#if !defined(__THRC_REMOTE_IMG_CONTROL_H__)
#define __THRC_REMOTE_IMG_CONTROL_H__

/*=======================================================.
  ヘッダファイルのインクルード.
=======================================================*/
#include "system/include/typs.h"
#include "system/sys/sysg/SYSC_System.h"
#include "system/cnv/cmp/cmpcom/CNVC_Common.h"
//#include "system/sys/Timer/SYSC_Timer.h".

#include "system/fil/FIL_FsWrapperC.h"
#include "THRG_RemoteImgConvInitModule.h"
#include "THRC_RmtImgCnvIFTxCtrl.h"

/*=======================================================.
  デバッグ用定義.
=======================================================*/
#if !defined(TORNADO) || defined(SIMULATOR)
// シミュレータ専用定義	////////////////////////////////////////////////////////.
// シミュレータではコンパイルが通ればよしとする（動作は保証しない）.

	#define	FILF_Fopen		fopen
	#define	FILF_Fclose		fclose
	#define	FILF_Fprint		fprintf
	#define	FILF_Fread		fread
	#define	FILF_Ftell		ftell

	#ifdef TRACE
		#define	_TRACE_FUNC()
		#define	_TRACE_LOG		printf
	#endif

	#define	__FUNCTION__	__FILE__

	#define	RmtImgCnvIF_GetResultString(_result)	"Error_Dummy"

#else	////////////////////////////////////////////////////////////////.

	#ifdef TRACE
		#include <pthread.h>
		#define	_TID()			{printf("0x%08lx ", (uintptr_t)pthread_self());}
		#define	_TRACE_FUNC()	{_TID(); printf("[**EVENT**]%s(L%d)...\n", __FUNCTION__, __LINE__); fflush(stdout);}
		//#define	_TRACE_LOG(...)	{_TID(); printf("[**EVENT**]"__VA_ARGS__); fflush(stdout);}
		#define	_TRACE_LOG(...)	{_TID(); printf("[**EVENT**]" __VA_ARGS__); fflush(stdout);}	//  24/03/22 FPT		Fix build error GccVersionUp.
	#else
		#define	_TRACE_FUNC()
		#define	_TRACE_LOG(...)
	#endif

	char*	RmtImgCnvIF_GetResultString(THRCE_RmtImgCnvIF_ErrType result);

#endif	//!defined(TORNADO) || defined(SIMULATOR).

#ifndef	PSEUDO_ENVIRONMENT
	#define milli_sleep(_msec)	{ \
		_TRACE_LOG("wait %d msec. ...\n", _msec); \
		SYSF_taskDelay((int32_t)((float)_msec * (float)SYSF_sysClkRateGet() / (float)1000)); \
	}
#else
	#include <unistd.h>
	#define milli_sleep(_msec)	{ \
		_TRACE_LOG("wait %d msec. ...\n", _msec); \
		usleep( _msec * 1000 ); \
	}
#endif

/*=======================================================.
  DEFINE定義.
=======================================================*/
#define THRD_REMOTE_IMG_CONV_MAX_MSGS	50

#ifdef TORNADO
	#ifndef PSEUDO_ENVIRONMENT
		#define	THRD_REMOTE_IMG_CONV_XMLDATA_DIR	"/ata01/Capture/"
		#define	THRD_REMOTE_IMG_CONV_IMGDATA_DIR	"/ata01/Capture/"
	#else
		#define	THRD_REMOTE_IMG_CONV_XMLDATA_DIR	"./XMLData/"
		#define	THRD_REMOTE_IMG_CONV_IMGDATA_DIR	"./ImgData/"
	#endif
#else
	#define	THRD_REMOTE_IMG_CONV_XMLDATA_DIR	"./XMLData/"
	#define	THRD_REMOTE_IMG_CONV_IMGDATA_DIR	"./ImgData/"
#endif

//! ファイル拡張子.
#define THRD_REMOTE_IMG_CONV_FILE_EXTENSION_RAW	".raw"
#define THRD_REMOTE_IMG_CONV_FILE_EXTENSION_JPG	".jpg"
#define THRD_REMOTE_IMG_CONV_FILE_EXTENSION_PDF	".pdf"
#define THRD_REMOTE_IMG_CONV_FILE_EXTENSION_BIN	".bin"
#define THRD_REMOTE_IMG_CONV_FILE_EXTENSION_TXT	".txt"

// 画像変換へのコールバック引数.
#define THRD_REMOTE_CONVIF_STATE_SUCCESS					CNVD_RetOk			// 正常終了.
#define THRD_REMOTE_CONVIF_STATE_ERROR						CNVD_RetNg			// 異常終了.
																		// 　現バージョンでは、
																		// 　通信制御 API で発生する全てのエラーでこのステータスを使用している.
																		// 　エラー種別に応じた区別は実装していない.

// 内部で使用する戻り値定義.
#define THRD_REMOTE_IMG_CONV_SUCCESS						0			// 正常終了.
#define THRD_REMOTE_IMG_CONV_INVALID_ARG					(-1)		// 引数異常.
#define THRD_REMOTE_IMG_CONV_MEMORY_ALLOCATE_ERROR			(-2)		// メモリ確保不可.
#define THRD_REMOTE_IMG_CONV_FILE_OPEN_ERROR				(-3)		// ファイルオープンエラー.
#define THRD_REMOTE_IMG_CONV_FILE_FORMAT_ERROR				(-4)		// XMLファイルフォーマットエラー.
#define THRD_REMOTE_IMG_CONV_UNKNOWN_ERROR					(-5)		// その他のエラー.
#define THRD_REMOTE_IMG_CONV_OPERATION_CANCELED				(-6)		// キャンセル要求による終了.


#define THRD_REMOTE_IMG_CONV_MSG_PERIODIC_LOOP				0			// 定期起床.
#define THRD_REMOTE_IMG_CONV_MSG_CONNECT_START				1			// 接続開始要求.
#define THRD_REMOTE_IMG_CONV_MSG_TRANS_START				2			// 転送開始要求.
#define THRD_REMOTE_IMG_CONV_MSG_TRANS_CANCEL				3			// 転送中止要求.
#define THRD_REMOTE_IMG_CONV_MSG_FILE_RECEIVE				4			// ファイル受信要求.
#define THRD_REMOTE_IMG_CONV_MSG_CONV_COMPLETE				5			// 画像変換完了通知.
#define THRD_REMOTE_IMG_CONV_MSG_CANCELED					6			// 転送中止完了通知.

#define THRD_REMOTE_IMG_CONV_STATUS_IDLE					0
#define THRD_REMOTE_IMG_CONV_STATUS_REQ_CONNECT_START		1
#define THRD_REMOTE_IMG_CONV_STATUS_WAIT_CONNECT_START_ANS	2
#define THRD_REMOTE_IMG_CONV_STATUS_REQ_TRANS_START			3
#define THRD_REMOTE_IMG_CONV_STATUS_WAIT_TRANS_START_ANS	4
#define THRD_REMOTE_IMG_CONV_STATUS_REQ_IMAGE_TRANS			5
#define THRD_REMOTE_IMG_CONV_STATUS_WAIT_IMAGE_TRANS_ANS	6
#define THRD_REMOTE_IMG_CONV_STATUS_WAIT_TRANS_COMP			7
#define THRD_REMOTE_IMG_CONV_STATUS_WAIT_CONVERT_COMP		8
#define THRD_REMOTE_IMG_CONV_STATUS_WAIT_CONVERT_DATA		9
#define THRD_REMOTE_IMG_CONV_STATUS_WAIT_MEMORY_FREE		10
#define THRD_REMOTE_IMG_CONV_STATUS_WAIT_TRANS_CANCEL_ANS	11


/*=======================================================.
  画像変換IFメッセージ格納用構造体.
=======================================================*/
typedef struct THRS_RemoteImgConvMsg
{
	char sc_MsgKind;			// メッセージ種別.
	void (*p_Callback)(void);	// 画像変換モジュールのコールバック関数.


}THRS_RemoteImgConvMsg;

/*=======================================================.
  グローバル関数/変数の定義.
=======================================================*/

// タスクのスタート関数コール用.
void tsk_RemoteImgConv( void );



/*=======================================================.
  画像変換パラメータ格納用構造体.
=======================================================*/
typedef struct THRS_RemoteImgConvFileInfo
{
	int32_t	sl_ImgFileID;			//!< 転送データファイルID.
	char	sc_FileName[256];		//!< 転送データファイル名.
	char	sc_FileKind;			//!< ファイル種別.
	char	sc_ColorMode;			//!< カラー情報.
	char	sc_AttrNum;				//!< 属性数.
	uint32_t	ul_PointX;				//!< 画像サイズ(X).
	uint32_t	ul_PointY;				//!< 画像サイズ(Y).
	uint32_t	ul_ResolutionX;			//!< 解像度(X).
	uint32_t	ul_ResolutionY;			//!< 解像度(Y).

}THRS_RemoteImgConvFileInfo;

typedef struct THRS_RemoteImgConvParam
{
	// 以下、画像変換IFパラメータ.
	char			sc_Detail[256];			// 結果詳細.
	char			sc_Application[256];	// 使用アプリケーション.
	char			sc_ConvKind[256];		// 変換種別.
	THRS_RemoteImgConvFileInfo* ps_RemoteImgConvFileInfo;
	Bool			b_OK;					// 変換処理結果.
	char			sc_DeviceKind;			// 転送先端末.
	char			sc_ProtocolKind;		// 転送プロトコル.
	ushort			us_FileCnt;				// ファイル数.
}THRS_RemoteImgConvParam;


/*=======================================================.
  THRG_RemoteImgConvIFクラスの宣言.
=======================================================*/
class THRG_RemoteImgConvIF
{
/*=====================================.
  関数の定義.
=====================================*/
public :
	
	// コンストラクタ.
	THRG_RemoteImgConvIF();
	
	// クラスのインスタンスへのポインタを返却する.
	static THRG_RemoteImgConvIF* newInstance();
	
	// クラスの初期値設定を行う.
	void initClass();

	// 画像変換IFメインループ用関数.
	void taskRemoteImgConvIF();
	
	// 画像変換、および画像変換IF通信モジュールへ公開する外部API.
	short THRG_RemoteImgConvSend(void (*ans_startConvIF)(int32_t sl_state));
	short THRG_RemoteImgConvCancel(void (*ans_cancelConvIF)());
	
	// 画像変換パラメータ用メモリ確保関数.
	Bool THRG_AllocImgCnvParam(uchar uc_FileCnt);
	
	// 画像変換パラメータ用メモリ確保関数.
	void THRG_FreeImgCnvParam(THRS_RemoteImgConvParam* ps_RemoteImgConvParam);
	
	// タスクから呼び出される、通信用関数.
	short	reqPollingProc();
	short	reqConnectStart();
	short	reqSendTransStartXml();
	short	reqSendImageData();
	short	reqSendTransCancelXml(void (*callback)(void));
	short	rcvTransCancelAns();

	// ファイル有無チェック処理.
	static short	checkExistFile( char * sc_FileName );
	static inline void delay( int32_t _msec ) {
		SYSF_taskDelay((int32_t)((float)_msec * (float)SYSF_sysClkRateGet() / (float)1000)); 
	}

	
	static void	rcvConnectStartAns(THRCS_RmtImgCnvIF_CBParamType s_CbParam);
	static void	rcvSendTransStartXmlAns(THRCS_RmtImgCnvIF_CBParamType s_CbParam);
	static void	rcvSendImageDataAns(THRCS_RmtImgCnvIF_CBParamType s_CbParam);
	static void	rcvConvResultXml(THRCS_RmtImgCnvIF_CBParamType s_CbParam);
	static void	rcvConvertedImage(THRCS_RmtImgCnvIF_CBParamType s_CbParam);
	static void	FUM_rcvchkrptXml(THRCS_RmtImgCnvIF_CBParamType s_CbParam);
	static void	FUM_chkchkrptXml(THRCS_RmtImgCnvIF_CBParamType s_CbParam);
	static void	FUM_rcvtxrptXml(THRCS_RmtImgCnvIF_CBParamType s_CbParam);
	static void	FUM_chktxrptXml(THRCS_RmtImgCnvIF_CBParamType s_CbParam);
	static void	FUM_chkconvertXml(THRCS_RmtImgCnvIF_CBParamType s_CbParam);
	static void	FUM_chkconvertedFile(THRCS_RmtImgCnvIF_CBParamType s_CbParam);

	static void	reqSendCancelXml(THRCS_RmtImgCnvIF_CBParamType s_CbParam);
	static void	rcvSendCancelXmlAns(THRCS_RmtImgCnvIF_CBParamType s_CbParam);
	static void	FUM_chkCancelXml(THRCS_RmtImgCnvIF_CBParamType s_CbParam);
	static void	FUM_rcvCancelXml(THRCS_RmtImgCnvIF_CBParamType s_CbParam);

	Bool isCanceled(THRCS_RmtImgCnvIF_CBParamType s_CbParam);

	void CallBackFunkCall(int32_t sl_state);
	
	void (*ImgCnvCallBack)(int32_t sl_state);
	void (*ImgCnvCancelCB)(void);
	
private :
	//! 転送データファイル名設定.
	void setTransFileName();

/*=====================================.
  変数の定義.
=====================================*/
public :

	// 画像変換パラメータ格納用構造体.
	THRS_RemoteImgConvParam s_RemoteImgConvParam;
	
	// 次に期待するファイル名.
	char	sc_NextFileName[256];
	
	// 画像変換へのコールバック.

	// 画像変換IFのステータス.
	ushort	us_ConvIfStatus;
	
	
private :
	
	// 画像変換IFタスクのメッセージID.
	MSG_Q_ID msg_Q_ID;

};

//// コンパイルエラー対策.
//void ans_startConvIF(long sl_state, THRS_RemoteImgConvParam* ps_RmoteImagConvParam).
//{.
//	return;.
//}.
//void ans_cancelConvIF().
//{.
//	return;.
//}.



#endif
