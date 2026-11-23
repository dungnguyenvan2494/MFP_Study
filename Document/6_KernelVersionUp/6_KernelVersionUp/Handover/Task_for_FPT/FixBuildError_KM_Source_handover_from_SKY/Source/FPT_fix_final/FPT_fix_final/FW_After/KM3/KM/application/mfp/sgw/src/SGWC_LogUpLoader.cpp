/***********************************************************************/
/** @file  SGWC_LogUpLoader.cpp
 *  @brief ログア�?プロー�?
 *  
 *  @author 
 *  @date 
 */
/***********************************************************************/
#include "SGWC_LogUpLoader.h"
#include "SGWC_SaaSGWSetting.h"
#include "SGWC_SoapAnalysis.h"
#include "SGWC_SaaSGWMain.h"
#include "SGWC_Zip.h"
// #include "SFLG_StorageFileLibraryIf.h"
#include "SGWC_Timer.h"
#include "SGWC_File.h"
#include "SGWC_Encrypt.h"
#include "SGWC_Status.h"
#include "SGWC_Log.h"

#define SGWD_LOGFILEPATH_SIZE 128

// CQ.503553 Start.
extern INT8 gOwnPrivateKey[SGWD_STRMAX_PRIVATEKEY+1];
// CQ.503553 End.

/***********************************************************************/
/** @brief コンストラクタ.
 * 
 *  @author 
 *  @date 
 */
/***********************************************************************/
SGWC_FileInfo::SGWC_FileInfo()
: m_FileData(NULL), m_FileSize(0)
 , m_MimeFileData(NULL), m_MimeFileSize(0)
{
	::memset(m_FileName, 0, sizeof(m_FileName));
}

/***********************************************************************/
/** @brief �?ストラクタ.
 * 
 *  @author 
 *  @date 
 */
/***********************************************************************/
SGWC_FileInfo::~SGWC_FileInfo()
{
	if (NULL != m_FileData)
	{
		::free(m_FileData);
	}

	if (NULL != m_MimeFileData)
	{
		::free(m_MimeFileData);
	}
}

/***********************************************************************/
/** @brief ファイル�?報設�?.
 * 
 *  @param[in] FilePath	?��ファイルパス.
 *  @retval STATUS	?���?��?結果.
 *  @author 
 *  @date 
 */
/***********************************************************************/
STATUS SGWC_FileInfo::setInfo(char* FilePath)
{
	STATUS Ret = OK;

	::memset(m_FileName, 0, sizeof(m_FileName));
	m_FileSize = 0;
	if (NULL != m_FileData)
	{
		::free(m_FileData);
	}

	if (NULL != FilePath)
	{
// Warning対�?.
//		char* FileName = ::strrchr(reinterpret_cast<const char*>(FilePath), '/');
		char* FileName = const_cast<char*>(::strrchr(reinterpret_cast<const char*>(FilePath), '/'));
		if (NULL != FileName)
		{
			++FileName;
		}
		else
		{
			FileName = FilePath;
		}
		::strncpy(m_FileName, reinterpret_cast<const char*>(FileName), sizeof(m_FileName) - 1);

		struct stat StatBuff;
		if (-1 != SFLG_Stat(FilePath, &StatBuff))
		{
			m_FileSize = static_cast<UINT32>(StatBuff.st_size);
			m_FileData = reinterpret_cast<char*>(::malloc(m_FileSize));
			if (NULL != m_FileData)
			{
				FILE* FP = SFLG_Fopen(reinterpret_cast<const char*>(FilePath), "rb");
				if (NULL != FP)
				{
					// MOD [ST] by H.Sawada 2016.04.07 for remove warning
					//if ((UINT32)m_FileSize != SFLG_Fread(m_FileData, 1u, m_FileSize, FP))
					int rc = SFLG_Fread(m_FileData, 1u, m_FileSize, FP);
					if (0 > rc || m_FileSize != (UINT32)rc)
					// MOD [ED] by H.Sawada 2016.04.07 for remove warning
					{
						SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_ERROR, "%s[%d] Error", __PRETTY_FUNCTION__, __LINE__);
						Ret = EIO;
					}

					SFLG_Fclose(FP);
				}
				else
				{
					SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_ERROR, "%s[%d] Error", __PRETTY_FUNCTION__, __LINE__);
					Ret = ERROR;
				}

				if (OK != Ret)
				{
					::free(m_FileData);
					m_FileData = NULL;
				}
			}
			else
			{
				SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_ERROR, "%s[%d] Error", __PRETTY_FUNCTION__, __LINE__);
				Ret = ENOMEM;
			}

			if (OK != Ret)
			{
				m_FileSize = 0;
			}
		}
		else
		{
			SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_ERROR, "%s[%d] Error", __PRETTY_FUNCTION__, __LINE__);
			Ret = ERROR;
		}

		if (OK == Ret)
		{
			UINT32 MimeHeaderSize	= (UINT32)::strlen(SGWD_LOGUPLOAD_MIME_HEADER);
			UINT32 MimeFooterSize	= (UINT32)::strlen(SGWD_LOGUPLOAD_MIME_FOOTER);
			UINT32 BodySize			= getSize();
			m_MimeFileSize			= MimeHeaderSize + MimeFooterSize + BodySize;
			m_MimeFileData = reinterpret_cast<char*>(::malloc(m_MimeFileSize));
			if (NULL != m_MimeFileData)
			{
				UINT32 WroteSize = 0;

				::memcpy(&m_MimeFileData[WroteSize], SGWD_LOGUPLOAD_MIME_HEADER, MimeHeaderSize);
				WroteSize += MimeHeaderSize;
				::memcpy(&m_MimeFileData[WroteSize], getData(), BodySize);
				WroteSize += BodySize;
				::memcpy(&m_MimeFileData[WroteSize], SGWD_LOGUPLOAD_MIME_FOOTER, MimeFooterSize);
				WroteSize += BodySize;
			}
			else
			{
				SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_ERROR, "%s[%d] Error", __PRETTY_FUNCTION__, __LINE__);
				m_MimeFileSize = 0;
				Ret = ENOMEM;
			}
		}
	}

	return Ret;
}

/***********************************************************************/
/** @brief ファイルパス取�?.
 * 
 *  @param[in,out] な�?.
 *  @retval char*	?��ファイルパス.
 *  @author 
 *  @date 
 */
/***********************************************************************/
char* SGWC_FileInfo::getName()
{
	return m_FileName;
}

/***********************************************************************/
/** @brief 書き込み�?ータ取�?.
 * 
 *  @param[in,out] な�?.
 *  @retval char*	?��書き込み�?ータ.
 *  @author 
 *  @date 
 */
/***********************************************************************/
char* SGWC_FileInfo::getData()
{
	return m_FileData;
}

/***********************************************************************/
/** @brief ファイルサイズ取�?
 * 
 *  @param[in,out] な�?.
 *  @retval UINT32	?��ファイルサイズ.
 *  @author 
 *  @date 
 */
/***********************************************************************/
UINT32 SGWC_FileInfo::getSize()
{
	return m_FileSize;
}

/***********************************************************************/
/** @brief MIME�?報取�?.
 * 
 *  @param[in,out] な�?.
 *  @retval char*	?��MIME�?報
 *  @author 
 *  @date 
 */
/***********************************************************************/
char* SGWC_FileInfo::getMimeData()
{
	return m_MimeFileData;
}

/***********************************************************************/
/** @brief MIME�?報サイズ取�?.
 * 
 *  @param[in,out] な�?.
 *  @retval UINT32	?��サイズ.
 *  @author 
 *  @date 
 */
/***********************************************************************/
UINT32 SGWC_FileInfo::getMimeSize()
{
	return m_MimeFileSize;
}

/***********************************************************************/
/** @brief コンストラクタ.
 * 
 *  @author 
 *  @date 
 */
/***********************************************************************/
SGWC_LogUpLoader::SGWC_LogUpLoader()
:	SGWC_Thread("SGW_LogUploader", 160, VX_FP_TASK, 1024*50), m_UploadSem(SEMTYPE_MUTEX)
{
	m_SysLogUpLoaderMsg = new SGWC_SysMsg();
	if (NULL == m_SysLogUpLoaderMsg)
	{
	}

	start();
}

/***********************************************************************/
/** @brief �?ストラクタ.
 * 
 *  @author 
 *  @date 
 */
/***********************************************************************/
SGWC_LogUpLoader::~SGWC_LogUpLoader()
{
	stop();
	join();

	delete m_SysLogUpLoaderMsg;
}

/***********************************************************************/
/** @brief インスタンス取�?.
 * 
 *  @param[in,out] な�?.
 *  @retval SGWC_LogUpLoader*	?��ログア�?プロードクラスのアドレス.
 *  @author 
 *  @date 
 */
/***********************************************************************/
SGWC_LogUpLoader* SGWC_LogUpLoader::getInstance()
{
	static SGWC_LogUpLoader Ins;

	return &Ins;
}

/***********************************************************************/
/** @brief ログア�?プロード�?��?(ログファイル圧縮⇒KMCore送信).
 * 
 *  @param[in,out] な�?.
 *  @retval STATUS	?���?��?結果.
 *  @author 
 *  @date 
 */
/***********************************************************************/
STATUS SGWC_LogUpLoader::upload()
{
	STATUS Ret = OK;

	if(SGWC_Status::newInstance()->getStatus() == TYPE_SGW_STATUS_ERROR) {
		// ファイルを作り続けてクラウドに送れな�?とHDDが枯�?するため�?
		// ネットワークエラーが発生して�?るとき�?�送らな�?.
		return Ret;
	}
	// Ac.1012769 Start.
	if(SGWF_isStorageLock()) {
		// HDDまた�?�SSDにアクセスできな�?と長時間の処�?�?ちが発生してパネルロ�?クエラーの原因になる可能性があるため�?
		// HDDまた�?�SSDがロ�?ク状態�?�とき�?�送らな�?.
		return ERROR;
	}
	// Ac.1012769 End.
	SGWF_StartHDDAccess(True);
	SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "%s[%d] ENTER(1)>>", __PRETTY_FUNCTION__, __LINE__);
	m_UploadSem.take();
	SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "%s[%d] ENTER(2)>>", __PRETTY_FUNCTION__, __LINE__);

	char LogFilePath[PATH_MAX_KM];
	char ZipFilePath[PATH_MAX_KM];

	// ログファイル出�?.
	struct timeval tv = {0};
	SGWC_Timer* timer = SGWC_Timer::newInstance();
	if (NULL != timer)
	{
		timer->getSyncedTime(&tv);
	}

	struct tm tim;
	gmtime_r( &(tv.tv_sec), &tim ); // KMCoreが持って�?る時刻は「UTC」�?�ため、localtime_rではなくgmtime_rを利用する.

	snprintf(LogFilePath, sizeof(LogFilePath)-1, "%s/GWLog_%04d%02d%02d%02d%02d%02d.log", FILC_BaseDirIf::newInstance(FILE_DirTypeSgw)->getPath(), tim.tm_year + 1900, tim.tm_mon + 1, tim.tm_mday, tim.tm_hour, tim.tm_min, tim.tm_sec);	/* MOD 2018.4.13 T.Tokitou [AccuRev_1026431] Klocwork */
	snprintf(ZipFilePath, sizeof(ZipFilePath)-1, "%s/GWLog_%04d%02d%02d%02d%02d%02d.zip", FILC_BaseDirIf::newInstance(FILE_DirTypeSgw)->getPath(), tim.tm_year + 1900, tim.tm_mon + 1, tim.tm_mday, tim.tm_hour, tim.tm_min, tim.tm_sec);	/* MOD 2018.4.13 T.Tokitou [AccuRev_1026431] Klocwork */

	SGWC_Log::newInstance()->outputDebugLog(LogFilePath);

	// ファイル圧縮.
	Ret = SGWC_Zip::getInstance()->zip(LogFilePath, ZipFilePath);
	if (OK == Ret)
	{
		// ファイルをKMCoreへ送信.
		SGWC_FileInfo FileInfo;

		Ret = FileInfo.setInfo(ZipFilePath);
		if (OK == Ret)
		{
			Ret = sendLog(&FileInfo);
		}
	}

	// 不要ファイル削除.
	SFLG_Unlink(LogFilePath);
	SFLG_Unlink(ZipFilePath);

	m_UploadSem.give();
	SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "%s[%d] LEAVE<<", __PRETTY_FUNCTION__, __LINE__);
	SGWF_EndHDDAccess();

	return Ret;
}

/***********************************************************************/
/** @brief オプションヘッダー�?報設�?.
 * 
 *  @param[in,out] SoapCliReqHeader	?��Soapヘッダー�?報設�?.
 *  @retval STATUS	?���?��?結果.
 *  @author 
 *  @date 
 */
/***********************************************************************/
STATUS SGWC_LogUpLoader::setSoapOptHeader(SGWC_SoapCliReqHeader* SoapCliReqHeader)
{
	STATUS Ret = OK;

	if (NULL != SoapCliReqHeader)
	{
		SGWS_SoapSendDataInfo SendDataInfo;
		::memset(&SendDataInfo, 0, sizeof(SendDataInfo));
		SGWS_SGW_AuthenticationKey stAuth;
		::memset(&stAuth, 0, sizeof(SGWS_SGW_AuthenticationKey));
		SGWF_getAuthenticationData(&stAuth);
		Ret = SGWF_getAuthenticationKey(SendDataInfo.sc_plainDateStr, SendDataInfo.sc_encDateStr, stAuth.sc_OwnPrivateKey);
		if (OK == Ret)
		{
			strcpy(SendDataInfo.sc_gwUserId, stAuth.sc_XMPP_UserName);

			t_sHttpHeaderList* Top = NULL;
			t_sHttpHeaderList* Cur = NULL;

			t_sHttpHeaderList* ContentDisposition = NULL;
			Ret = SGWC_HttpHeaderList::alloc(&ContentDisposition);
			if (OK == Ret)
			{
				SGWC_HttpHeaderList::setName(ContentDisposition, "Content-Disposition");
				SGWC_HttpHeaderList::setVal(ContentDisposition, "form-data");
				Top = Cur = ContentDisposition;

				t_sHttpHeaderList* SaaSGwId = NULL;
				Ret = SGWC_HttpHeaderList::alloc(&SaaSGwId);
				if (OK == Ret)
				{
					SGWC_HttpHeaderList::setName(SaaSGwId, "X-KM-SaaSGwId");
					SGWC_HttpHeaderList::setVal(SaaSGwId, SendDataInfo.sc_gwUserId);
					SGWC_HttpHeaderList::setNext(Cur, SaaSGwId);
					Cur = SGWC_HttpHeaderList::getNext(Cur);

					t_sHttpHeaderList* Signature = NULL;
					Ret = SGWC_HttpHeaderList::alloc(&Signature);
					if (OK == Ret)
					{
						SGWC_HttpHeaderList::setName(Signature, "X-KM-Signature");
						SGWC_HttpHeaderList::setVal(Signature, SendDataInfo.sc_plainDateStr);
						SGWC_HttpHeaderList::setNext(Cur, Signature);
						Cur = SGWC_HttpHeaderList::getNext(Cur);

						t_sHttpHeaderList* EncryptedSignature = NULL;
						Ret = SGWC_HttpHeaderList::alloc(&EncryptedSignature);
						if (OK == Ret)
						{
							SGWC_HttpHeaderList::setName(EncryptedSignature, "X-KM-EncryptedSignature");
							SGWC_HttpHeaderList::setVal(EncryptedSignature, SendDataInfo.sc_encDateStr);
							SGWC_HttpHeaderList::setNext(Cur, EncryptedSignature);
						}
					}
				}
			}

			if (OK != Ret)
			{
				SGWC_HttpHeaderList::free(&Top);
			}

			SoapCliReqHeader->setOptHeader(Top);
		}
	}
	else
	{
		SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_ERROR, "%s[%d] Error", __PRETTY_FUNCTION__, __LINE__);
		Ret = ERROR;
	}

	return Ret;
}

/***********************************************************************/
/** @brief Soapヘッダー�?報設�?.
 * 
 *  @param[in,out] SoapCliReqHeader	?��Soapヘッダー�?報.
 *  @param[in,out] DbRec	?��DB�?報.
 *  @param[in,out] FileInfo	?��ファイル�?報.
 *  @retval STATUS	?���?��?結果.
 *  @author 
 *  @date 
 */
/***********************************************************************/
STATUS SGWC_LogUpLoader::setSoapCliReqHeader(SGWC_SoapCliReqHeader* SoapCliReqHeader, SGWS_SGW_HTTPHeaderData* DbRec, SGWC_FileInfo* FileInfo)
{
	STATUS Ret = OK;

	if ((NULL != SoapCliReqHeader) && (NULL != DbRec) && (NULL != FileInfo))
	{
// Warning対�?.
//		char* gwif = strcasestr((const char*)(DbRec->sc_URL_KmCoreUrl), "/GwIf");
		char* gwif = const_cast<char*>(strcasestr((const char*)(DbRec->sc_URL_KmCoreUrl), "/GwIf"));
		if (NULL != gwif)
		{
			*gwif = '\0';
			strncat(DbRec->sc_URL_KmCoreUrl, "/logupload", TYPD_SGW_MAX_URL - strlen((const char*)(DbRec->sc_URL_KmCoreUrl)) - 1);
			Ret = SoapCliReqHeader->setRequestURI((const char*)(DbRec->sc_URL_KmCoreUrl));
			if (OK == Ret)
			{
				SoapCliReqHeader->setMethod((INT32)MIO_HTTP_REQUEST_POST);
				SoapCliReqHeader->setContentLength(FileInfo->getMimeSize());
				SoapCliReqHeader->setContentType("multipart/form-data; boundary=mime_boundary_logupload");
				setSoapOptHeader(SoapCliReqHeader);
			}
		}
	}
	else
	{
		SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_ERROR, "%s[%d] Error", __PRETTY_FUNCTION__, __LINE__);
		Ret = EINVAL;
	}

	return Ret;
}

/***********************************************************************/
/** @brief Soapボディー�?報設�?.
 * 
 *  @param[in,out] SoapCliReqBody	?��Soapボディー�?報.
 *  @param[in,out] SesID	?��セ�?ションID.
 *  @param[in,out] FileInfo	?��ファイル�?報.
 *  @retval STATUS	?���?��?結果.
 *  @author 
 *  @date 
 */
/***********************************************************************/
STATUS SGWC_LogUpLoader::setSoapCliReqBody(SGWC_SoapCliReqBody* SoapCliReqBody, INT32 SesID, SGWC_FileInfo* FileInfo)
{
	STATUS Ret = OK;

	if ((NULL != SoapCliReqBody) && (NULL != FileInfo))
	{
		SoapCliReqBody->setSesID(SesID);
		SoapCliReqBody->setDataLen(FileInfo->getMimeSize());
		SoapCliReqBody->setData(FileInfo->getMimeData());
		SoapCliReqBody->setEndFlag(MIO_SOAP_EOB);
	}
	else
	{
		SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_ERROR, "%s[%d] Error", __PRETTY_FUNCTION__, __LINE__);
		Ret = EINVAL;
	}

	return Ret;
}

/***********************************************************************/
/** @brief ログア�?プロード�?��?(KMCore送信).
 * 
 *  @param[in,out] FileInfo	?��ファイル�?報.
 *  @retval STATUS	?���?��?結果.
 *  @author 
 *  @date 
 */
/***********************************************************************/
STATUS SGWC_LogUpLoader::sendLog(SGWC_FileInfo* FileInfo)
{
	STATUS Ret = OK;

	uchar HTTP_RetryCount = SGWC_SaaSGWSetting::newInstance()->getHTTP_RetryCount();
	if(OK == Ret)
	{
		for (INT16 RetryCnt = (INT16)(HTTP_RetryCount); 0 <= RetryCnt; --RetryCnt)
		{
			Ret = sendLogCore(FileInfo);
			if (OK == Ret)
			{
				RetryCnt = 0;
			}
		}
	}

	return Ret;
}

/***********************************************************************/
/** @brief ログア�?プロード�?��?(KMCore送信コア処�?).
 * 
 *  @param[in,out] FileInfo	?��ファイル�?報.
 *  @retval STATUS	?���?��?結果.
 *  @author 
 *  @date 
 */
/***********************************************************************/
STATUS SGWC_LogUpLoader::sendLogCore(SGWC_FileInfo* FileInfo)
{
	STATUS Ret = OK;

	if (NULL != FileInfo)
	{
		SGWC_SaaSGWMain* GWMain = SGWC_SaaSGWMain::newInstance();
		SGWC_SoapClient Client(GWMain->getSharedSoapMsg(), GWMain->getSoapParser());

		SGWC_SoapCliReqHeader	ReqHeader;
		SGWC_SoapCliReqBody		ReqBody;
		SGWC_SoapCliResHeader	ResHeader;
		SGWC_SoapCliResBody		ResBody;

		// ----- send -----

		// send header
		SGWS_SGW_HTTPHeaderData st_Data;
		memset(&st_Data, 0, sizeof(SGWS_SGW_HTTPHeaderData));
		ReqHeader.getHeaderData(&st_Data);
		Ret = ReqHeader.initialize(&st_Data);
		if (OK == Ret)
		{
			Ret = setSoapCliReqHeader(&ReqHeader, &st_Data, FileInfo);
			if (OK == Ret)
			{
				Ret = Client.sendReqHeader(&ReqHeader);
			}
		}

		// send body
		if (OK == Ret)
		{
			Ret = setSoapCliReqBody(&ReqBody, ReqHeader.getSesID(), FileInfo);
			if (OK == Ret)
			{
				Ret = Client.sendReqBody(&ReqBody);
			}
		}

		// ----- recv -----

		if (OK == Ret)
		{
			Ret = Client.recvResponse(ReqBody.getSesID(), &ResHeader, &ResBody);
		}

		if (OK == Ret)
		{
			t_sHttpHeaderList* OptHeader;
			for (OptHeader = ResHeader.getOptHeader(); NULL != OptHeader; OptHeader = SGWC_HttpHeaderList::getNext(OptHeader))
			{
				INT8* Name = SGWC_HttpHeaderList::getName(OptHeader);

				SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "%s[%d] OptHeader:Name=%s", __PRETTY_FUNCTION__, __LINE__, Name);
				if (0 == ::strcasecmp(reinterpret_cast<const char*>(Name), "X-KM-ResultCode"))
				{
					break;
				}
			}

			char* Found = NULL;
			if (NULL != OptHeader)
			{
				INT8* Val = SGWC_HttpHeaderList::getVal(OptHeader);

				SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "%s[%d] OptHeader:Val=%s", __PRETTY_FUNCTION__, __LINE__, Val);
// Warning対�?.
//				Found = ::strstr(reinterpret_cast<const char*>(Val), "OK");
				Found = const_cast<char*>(::strstr(reinterpret_cast<const char*>(Val), "OK"));
				
				if(NULL == Found){
					// エラーカウントア�?�?.
					SGWC_NetworkErrorCnt::newInstance()->setNetworkErrorCnt_Cloud(Val);
				} else {
					// 処�?成功なら�?�直近�?�ネットワークエラーカウントをクリア(TYPE_SGW_ERRCODE_SUCCESS)する.
					SGWC_NetworkErrorCnt::newInstance()->clearErrorCodeDetail();
				}
			}

			if (NULL == Found)
			{
				SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_ERROR, "%s[%d] Error", __PRETTY_FUNCTION__, __LINE__);
				Ret = ERROR;
			}
		}
	}
	// 品質改�? start.
	else {
		SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_ERROR, "%s[%d] FileInfo Error", __PRETTY_FUNCTION__, __LINE__);
		Ret = ERROR;
	}
	// 品質改�? end.
	return Ret;
}

/***********************************************************************/
/** @brief ログア�?プロードタスクメイン.
 * 
 *  @param[in,out] な�?.
 *  @retval STATUS	?���?��?結果.
 *  @author 
 *  @date 
 */
/***********************************************************************/
STATUS SGWC_LogUpLoader::main()
{
	STATUS Ret = OK;
	SGWS_MsgParam MsgParam;

	// main loop
	/* SGWD_WAIT_FOREVERなのでm_StopをTRUEにしてもスレ�?ドが終�?しな�?可能性があ�?. */
	while ((ERROR != Ret) && (FALSE == m_Stop))
	{
		Ret = m_SysLogUpLoaderMsg->recv(&MsgParam);
		if (ERROR != Ret)
		{
			if (!SGWC_Status::newInstance()->getDisable())
			{
				// CQ.503553 Start.
				// 暗号化キーがま�?準備できて�?な�?場合�?�送信しな�?.
				if(strlen(gOwnPrivateKey) == 0) {
					SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "%s[%d] LogSend NotReady", __PRETTY_FUNCTION__, __LINE__);
				} else {
					upload();
				}
				// CQ.503553 End.
			}
		}
	}
	SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_ERROR, "%s[%d] while-loop break Ret = %d, m_Stop = %d, TID = %d", __PRETTY_FUNCTION__, __LINE__, Ret, m_Stop, getThreadID());

	return Ret;
}

/***********************************************************************/
/** @brief ア�?プロード�?��?(同期処�?).
 * 
 *  @param[in,out] な�?.
 *  @retval STATUS	?���?��?結果.
 *  @author 
 *  @date 
 */
/***********************************************************************/
STATUS SGWC_LogUpLoader::upload_sync()
{
	STATUS Ret = OK;

	if (!SGWC_Status::newInstance()->getDisable())
	{
		// CQ.503553 Start.
		// 暗号化キーがま�?準備できて�?な�?場合�?�送信しな�?.
		if(strlen(gOwnPrivateKey) == 0) {
			SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "%s[%d] LogSend NotReady2", __PRETTY_FUNCTION__, __LINE__);
			Ret = ERROR;
		} else {
			Ret = upload();
		}
		// CQ.503553 End.
	}
	else
	{
		Ret = ERROR;
	}

	return Ret;
}

/***********************************************************************/
/** @brief ア�?プロード�?��?(非同期�?��?).
 * 
 *  @param[in,out] な�?.
 *  @retval STATUS	?���?��?結果.
 *  @author 
 *  @date 
 */
/***********************************************************************/
STATUS SGWC_LogUpLoader::upload_async()
{
	STATUS Ret = OK;

	if (!SGWC_Status::newInstance()->getDisable())
	{
		SGWS_MsgParam MsgParam;

		Ret = m_SysLogUpLoaderMsg->send(&MsgParam);
	}
	else
	{
		Ret = ERROR;
	}

	return Ret;
}

/***********************************************************************/
/** @brief 全ファイルア�?プロー�?.
 * 
 *  @param[in,out] な�?.
 *  @retval STATUS	?���?��?結果.
 *  @author 
 *  @date 
 */
/***********************************************************************/
STATUS SGWC_LogUpLoader::uploadAll()
{
	STATUS Ret = OK;
	std::stringstream StrStream;
	std::string FilePath;
	std::string SavePath;
	DIR *Dir;
	struct dirent *DirEnt;
	struct stat FileStat;
	std::string LogExt = ".log";
	std::string::size_type LogPos = 0;
	std::string ZipExt = ".zip";
	std::string::size_type ZipPos = 0;

	SGWF_StartHDDAccess(True);
	m_UploadSem.take();

	Dir = SFLG_OpenDir((char*)(FILC_BaseDirIf::newInstance(FILE_DirTypeSgw)->getPath()));
	if (NULL != Dir)
	{
		while (NULL != (DirEnt = SFLG_ReadDir(Dir)))
		{
			StrStream.str("");
			StrStream.clear();
			StrStream << FILC_BaseDirIf::newInstance(FILE_DirTypeSgw)->getPath() << "/" << DirEnt->d_name;
			FilePath = StrStream.str();

			if (0 != SFLG_Stat((INT8*)FilePath.c_str(), &FileStat))
			{
				continue;
			}

			if (0 == S_ISREG(FileStat.st_mode))
			{
				continue;
			}

			if (std::string::npos != (LogPos = FilePath.rfind(LogExt)))
			{
				SavePath = FilePath;
				FilePath.replace(LogPos, ZipExt.size(), ZipExt);

				Ret = SGWC_Zip::getInstance()->zip((INT8*)SavePath.c_str(), (INT8*)FilePath.c_str());
				SFLG_Unlink(SavePath.c_str());
			}

			if (OK != Ret)
			{
				break;
			}

			if (std::string::npos != (ZipPos = FilePath.rfind(ZipExt)))
			{
				SGWC_FileInfo FileInfo;

				Ret = FileInfo.setInfo((INT8*)FilePath.c_str());
// 自動ログア�?プロード機�?�ドロ�?�?.
// 溜まって�?るファイルを削除するのみとする.
//				if (OK == Ret)
//				{
//					Ret = sendLog(&FileInfo);
//				}

				SFLG_Unlink(FilePath.c_str());
			}

			if (OK != Ret)
			{
				break;
			}
		}

		SFLG_CloseDir(Dir);
	}
	else
	{
		Ret = ERROR;
	}

	m_UploadSem.give();
	SGWF_EndHDDAccess();

	return Ret;
}

void SGWC_LogUpLoader::showMsgQ(){
	printf("#SGW_LogUpLoader msgQ:0x%x\n", m_SysLogUpLoaderMsg->getMsgQID());
	SYSF_msgQShow(m_SysLogUpLoaderMsg->getMsgQID(), 1);
}
