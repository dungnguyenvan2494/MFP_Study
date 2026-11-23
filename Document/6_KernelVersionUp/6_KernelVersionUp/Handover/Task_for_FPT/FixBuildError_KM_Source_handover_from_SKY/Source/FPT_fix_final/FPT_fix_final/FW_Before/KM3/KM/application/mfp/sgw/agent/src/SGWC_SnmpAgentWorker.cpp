/***********************************************************************/
/** @file  SGWC_SnmpAgentWorker.cpp
 *  @brief SNMPエージェント子スレッド関連
 *  
 *  @author 
 *  @date 
 */
/***********************************************************************/

#include <vector>
#include <sstream>
#include "SGWC_SnmpAgentWorker.h"
#include "SGWC_SaaSGWSetting.h"
#include "SGWC_DBDeviceList.h"
#include "SGWC_SnmpClient.h"
#include "SGWC_SnmpVariable.h"
#include "SGWC_Timer.h"
#include "SGWC_Log.h"

#define SGWD_SNMPAGENT_TEST 1

#if 0  // DEL ST 制御No.75 MES削除対応
/* MOD-ST 2016.08.25 IT5_4.0 制御構成変更No.43 NICFUM修正 T.Matsushita */
#if 0
#include "hdw_mod.h"
#include "r_fips140.h"
#else
#include "r_prov_fips140.h"
#endif
/* MOD-ED 2016.08.25 IT5_4.0 制御構成変更No.43 NICFUM修正 T.Matsushita */

#include "MESWRAP_Init.h"
#include "r_base64.h"
#endif　// DEL ED 制御No.75 MES削除対応

// ---------- request ----------

/***********************************************************************/
/** @class SGWC_StringUtil
 *  @brief ストリングユーティリティー関連
 *  
 *  @author 
 *  @date 
 */
/***********************************************************************/
class SGWC_StringUtil
{
public:
	/***********************************************************************/
	/** @brief ストリング分割
	 * 
	 *  @param[in] str						: 分割対象
	 *  @param[in] delim					: 区切り文字列
	 *  @param[in] count					: 分割回数
	 *  @retval std::vector< std::string >	: 分割結果
	 */
	/***********************************************************************/
	static std::vector< std::string > split(std::string& str, std::string& delim, INT32 count)
	{
		std::vector< std::string > ret;
		std::string::size_type pos1;
		std::string::size_type pos2;
		INT32 i;

//		SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_StringUtil::split ENTER>> str[%s], delim[%s], count[%d]", str.c_str(), delim.c_str(), count);

		for (i = 0, pos1 = 0;
			((-1 == count) || (count > i)) && (std::string::npos != (pos2 = str.find(delim, pos1)));
			++i, pos1 = pos2 + delim.size())
		{
			ret.push_back(str.substr(pos1, pos2 - pos1));
		}

		if (0 < (str.size() - pos1))
		{
			ret.push_back(str.substr(pos1, str.size() - pos1));
		}

//		SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_StringUtil::split LEAVE<<");
//		for (std::vector< std::string >::iterator it = ret.begin() ; ret.end() != it; ++it)
//		{
//			SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "Token:[%s]", (*it).c_str());
//		}

		return ret;
	}
};

/***********************************************************************/
/** @class SGWC_SnmpAgentReqData
 *  @brief SNMPエージェントリクエストデータ関連
 *  
 *  @attention 設定コマンドは非サポート
 *  @author 
 *  @date 
 */
/***********************************************************************/
class SGWC_SnmpAgentReqData
{
public:
	/***********************************************************************/
	/** @brief SNMPエージェントリクエスト解析(一行)
	 * 
	 *  @param[in] line			: 解析対象
	 *  @param[in] data_index	: インデックス
	 *  @retval STATUS			: 処理結果
	 *  @author 
	 *  @date 
	 */
	/***********************************************************************/
	STATUS parse(std::string& line, INT32 data_index)
	{
		STATUS Ret = OK;
		std::string delim("=");
		std::vector< std::string > token = SGWC_StringUtil::split(line, delim, 1);
		std::stringstream key;

		key << "RequestData" << "-" << data_index;
		if (false == key.fail())
		{
//			SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentReqData::parse key=%s", (key.str()).c_str());
			if (key.str() == token[0])
			{
				std::string val_delim(",");
				std::vector< std::string > values = SGWC_StringUtil::split(token[1], val_delim, -1);
				for (INT32 val_index = 0; (static_cast<INT32>(values.size()) > val_index) && (OK == Ret); ++val_index)
				{
					switch (val_index)
					{
					case 0:
						// 要求種別
						m_ReqType = values[val_index];
						// Setは非サポート(NICFUMが非対応)
						if ("3" == m_ReqType)
						{
							Ret = ENOTSUP;
						}
						break;
					case 1:
						// 要求OID
						m_ReqOID = values[val_index];
						break;
					case 2:
						// 設定変数形式
						m_ReqSyntax = values[val_index];
						break;
					case 3:
						// 設定変数値
						m_ReqVal = values[val_index];
						break;
					default:
						Ret = ERROR;
						break;
					}
				}
			}
			else
			{
				Ret = ERROR;
			}
		}
		else
		{
			Ret = ERROR;
		}

		return Ret;
	}

	/***********************************************************************/
	/** @brief リクエストタイプ取得
	 * 
	 *  @retval std::string&	: リクエストタイプ
	 *  @author 
	 *  @date 
	 */
	/***********************************************************************/
	std::string& getReqType()
	{
		return m_ReqType;
	}

	/***********************************************************************/
	/** @brief リクエストOID取得
	 * 
	 *  @retval std::string&	: リクエストOID 
	 *  @author 
	 *  @date 
	 */
	/***********************************************************************/
	std::string& getReqOID()
	{
		return m_ReqOID;
	}

	/***********************************************************************/
	/** @brief SNMPクライアント取得
	 * 
	 *  @retval SGWC_SnmpClient&	: SNMPクライアント
	 *  @author 
	 *  @date 
	 */
	/***********************************************************************/
	SGWC_SnmpClient& getSnmpClient()
	{
		return m_SnmpClient;
	}

private:
	std::string			m_ReqType;
	std::string			m_ReqOID;
	std::string			m_ReqSyntax;
	std::string			m_ReqVal;
	SGWC_SnmpClient		m_SnmpClient;
};

/***********************************************************************/
/** @class SGWC_SnmpAgentReq
 *  @brief SNMPエージェントリクエスト関連
 *  
 *  @author 
 *  @date 
 */
/***********************************************************************/
class SGWC_SnmpAgentReq
{
public:
	/***********************************************************************/
	/** @brief コンストラクタ
	 *  
	 *  @author 
	 *  @date 
	 */
	/***********************************************************************/
	SGWC_SnmpAgentReq()
	{
//		SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentReq::SGWC_SnmpAgentReq()");
	}
	/***********************************************************************/
	/** @brief デストラクタ
	 *  
	 *  @author 
	 *  @date 
	 */
	/***********************************************************************/
	~SGWC_SnmpAgentReq()
	{
//		SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentReq::~SGWC_SnmpAgentReq()");
		for (std::vector< SGWC_SnmpAgentReqData* >::iterator it = m_ReqDataList.begin(); it != m_ReqDataList.end(); it++)
		{
			delete *it;
		}
	}

	/***********************************************************************/
	/** @brief SNMPエージェントリクエスト解析(複数行)
	 * 
	 *  @param[out] lines	: 解析対象
	 *  @param[out] index	: インッデクス
	 *  @retval STATUS		: 処理結果
	 *  @author 
	 *  @date 
	 */
	/***********************************************************************/
	STATUS parse(std::vector< std::string >& lines, INT32 index)
	{
		STATUS Ret = OK;
		std::string key;

		switch (index)
		{
		case 0:
			// Length
			key = "Length";
			Ret = setVal(lines[index], key, m_Length);
			break;
		case 1:
			// DeviceID
			key = "DeviceID";
			Ret = setVal(lines[index], key, m_DevID);
			break;
		case 2:
			// TotalTimeout
			key = "TotalTimeout";
			Ret = setVal(lines[index], key, m_TotalTimeout);
			break;
		case 3:
			// RequestTimeout
			key = "RequestTimeout";
			Ret = setVal(lines[index], key, m_ReqTimeout);
			break;
		case 4:
			// Retry
			key = "Retry";
			Ret = setVal(lines[index], key, m_Retry);
			break;
		case 5:
			// Requests
			key = "Requests";
			Ret = setVal(lines[index], key, m_ReqNum);
			break;
		default:
			if (6 <= index)
			{
				// RequestData-X
				INT32 data_index = index - 6;

//				SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentReq::parse - parse and push back [ST] data_index=%d", data_index);

				SGWC_SnmpAgentReqData* ReqData = new SGWC_SnmpAgentReqData();
				if (NULL != ReqData)
				{
					Ret = ReqData->parse(lines[index], data_index);
					if (OK == Ret)
					{
						m_ReqDataList.push_back(ReqData);
					}
					else
					{
						delete ReqData;
					}
				}
				else
				{
					Ret = ENOMEM;
				}

//				SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentReq::parse - parse and push back [ED] Ret=%d", Ret);
			}
			else
			{
				Ret = ERROR;
			}
			break;
		}

		return Ret;
	}

	/***********************************************************************/
	/** @brief リクエスト長取得
	 * 
	 *  @retval std::string&	: リクエスト長
	 *  @author 
	 *  @date 
	 */
	/***********************************************************************/
	std::string& getLength()
	{
		return m_Length;
	}

	/***********************************************************************/
	/** @brief デバイスID取得
	 * 
	 *  @retval std::string&	: デバイスID
	 *  @author 
	 *  @date 
	 */
	/***********************************************************************/
	std::string& getDevID()
	{
		return m_DevID;
	}

	/***********************************************************************/
	/** @brief 通信処理全体のタイムタウト時間取得
	 * 
	 *  @retval std::string&	: 通信処理全体のタイムタウト時間
	 *  @author 
	 *  @date 
	 */
	/***********************************************************************/
	std::string& getTotalTimeout()
	{
		return m_TotalTimeout;
	}

	/***********************************************************************/
	/** @brief リクエストタイムアウト時間取得
	 * 
	 *  @retval std::string&	: リクエストタイムアウト時間
	 *  @author 
	 *  @date 
	 */
	/***********************************************************************/
	std::string& getReqTimeout()
	{
		return m_ReqTimeout;
	}

	/***********************************************************************/
	/** @brief リクエストリトライ回数取得
	 * 
	 *  @retval std::string&	: リクエストリトライ回数
	 *  @author 
	 *  @date 
	 */
	/***********************************************************************/
	std::string& getRetry()
	{
		return m_Retry;
	}

	/***********************************************************************/
	/** @brief リクエスト数取得
	 * 
	 *  @retval std::string&	: リクエスト数
	 *  @author 
	 *  @date 
	 */
	/***********************************************************************/
	std::string& getReqNum()
	{
		return m_ReqNum;
	}

	/***********************************************************************/
	/** @brief SNMPエージェントリクエストリスト取得
	 * 
	 *  @retval std::vector< SGWC_SnmpAgentReqData >&	: SNMPエージェントリクエストリスト
	 *  @author 
	 *  @date 
	 */
	/***********************************************************************/
	std::vector< SGWC_SnmpAgentReqData* >& getReqDataList()
	{
		return m_ReqDataList;
	}

private:
	/***********************************************************************/
	/** @brief SNMPエージェントリクエスト解析(一行)
	 * 
	 *  @param[in] line	: 解析対象
	 *  @param[in] key	: キー
	 *  @param[out] val	: 値
	 *  @retval STATUS	: 処理結果
	 *  @author 
	 *  @date 
	 */
	/***********************************************************************/
	STATUS setVal(std::string& line, std::string& key, std::string& val)
	{
		STATUS Ret = OK;
		std::string delim("=");
		std::vector< std::string > token = SGWC_StringUtil::split(line, delim, 1);

//		SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentReq::setVal ENTER>> line=%s, key=%s", line.c_str(), key.c_str());

		if (key == token[0])
		{
			val = token[1];
		}
		else
		{
			Ret = ERROR;
		}

//		SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentReq::setVal LEAVE<< Ret=%d, val=%s", Ret, val.c_str());

		return Ret;
	}

	std::string								m_Length;
	std::string								m_DevID;
	std::string								m_TotalTimeout;
	std::string								m_ReqTimeout;
	std::string								m_Retry;
	std::string								m_ReqNum;
	std::vector< SGWC_SnmpAgentReqData* >	m_ReqDataList;
};

// ====================================================================================================

/***********************************************************************/
/** @brief コンストラクタ
 *  
 *  @author 
 *  @date 
 */
/***********************************************************************/
SGWC_SnmpAgentWorker::SGWC_SnmpAgentWorker()
: SGWC_Thread("SGW_SnmpAgentWorker", 160, VX_FP_TASK, 1024*60),  m_Finished(FALSE), m_Socket(NULL), m_SharedMsg(NULL)
{
}

/***********************************************************************/
/** @brief デストラクタ
 *  
 *  @author 
 *  @date 
 */
/***********************************************************************/
SGWC_SnmpAgentWorker::~SGWC_SnmpAgentWorker()
{
}

/***********************************************************************/
/** @brief ソケット設定
 * 
 *  @param[in] Socket	: ソケット
 *  @author 
 *  @date 
 */
/***********************************************************************/
void SGWC_SnmpAgentWorker::setSocket(SGWC_Socket* Socket)
{
	m_Socket = Socket;
}

/***********************************************************************/
/** @brief 共有メッセージ設定
 * 
 *  @param[in] SharedMsg	: 共有メッセージ
 *  @author 
 *  @date 
 */
/***********************************************************************/
void SGWC_SnmpAgentWorker::setSharedMsg(SGWC_SharedMsg* SharedMsg)
{
	m_SharedMsg = SharedMsg;
}

/***********************************************************************/
/** @brief SNMPエージェント子スレッド終了フラグ
 * 
 *  @retval BOOL	: SNMPエージェント子スレッド終了フラグ
 *  @author 
 *  @date 
*/
/***********************************************************************/
BOOL SGWC_SnmpAgentWorker::getFinished()
{
	return m_Finished;
}


/***********************************************************************/
/** @brief SNMPエージェント子スレッド処理
 * 
 *  @retval STATUS	: 処理結果
 *  @author 
 *  @date 
*/
/***********************************************************************/
STATUS SGWC_SnmpAgentWorker::execute()
{
	STATUS Ret = OK;

	SGWC_SnmpAgentReq AgentReq;

	INT32 request_size = static_cast<INT32>(sizeof(m_RecvBuf)) - 1;
	INT32 remain_size  = static_cast<INT32>(sizeof(m_RecvBuf)) - 1;
	INT32 total_size = 0;
	INT32 recved_size = 0;
	std::string eol("\r\n");
	std::vector< std::string > lines;

	m_ErrCode = "ResultCode=OK000000\r\n";

//	SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::recv - [ST]");

	// リクエストを全て受信する
	::memset(m_RecvBuf, 0, sizeof(m_RecvBuf));
	while ((0 < remain_size) && (OK == Ret))
	{
		// block
//		SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::main - recv: recved_size=%d, total_size=%d, remain_size=%d", recved_size, total_size, remain_size);
		recved_size = m_Socket->recv(&(m_RecvBuf[total_size]), static_cast<UINT32>(remain_size), 0);
//		SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::main - recv: recved_size=%d", recved_size);
//		SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::main - recv: recved_size=%d, total_size=%d, remain_size=%d", recved_size, total_size, remain_size);
		if (0 < recved_size)
		{
			total_size += recved_size;

			if (0 == lines.size())
			{
				m_RecvBuf[total_size] = '\0';
//				SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::recved total_size=%d, m_RecvBuf=%s", total_size, m_RecvBuf);
				m_RecvStr = reinterpret_cast<const INT8*>(m_RecvBuf);
				lines = SGWC_StringUtil::split(m_RecvStr, eol, 1);
				if (0 < lines.size())
				{
					Ret = AgentReq.parse(lines, 0);
					if (OK == Ret)
					{
						std::stringstream conv_length(AgentReq.getLength());
						conv_length >> request_size;
						if (false == conv_length.fail())
						{
							request_size += static_cast<INT32>(lines[0].size() + eol.size());
							if ((static_cast<INT32>(sizeof(m_RecvBuf)) - 1) < request_size)
							{
								Ret = EOVERFLOW;
							}
						}
						else
						{
							Ret = ERROR;
						}
					}
				}
			}

			remain_size = request_size - total_size;
		}
		else
		{
			if (0 > recved_size)
			{
				SGWC_NetworkErrorCnt::newInstance()->setNetworkCnt_App(SGWE_MIOSOCKRECV);
				SGWC_NetworkErrorCnt::newInstance()->setNetworkErrorCnt_App(SGWE_MIOSOCKRECV, (short)Ret);	/* Mod Y.Yamada 2017/11/06 AccuRev_1026431 *//* Merged T.Matsushita 2018/01/31 AccuRev_1035029 */
				Ret = EIO;
			}

			remain_size = 0;
		}
	}

//	SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::recv - [ED]");

	if (0 < total_size)
	{
		m_RecvBuf[total_size] = '\0';
		m_RecvStr = reinterpret_cast<const INT8*>(m_RecvBuf);

		SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::recved total_size=%d, m_RecvBuf=%s", total_size, m_RecvBuf);

		// リクエストを全て解析する
		if (OK == Ret)
		{
//			SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::main - parse all lines");

			lines = SGWC_StringUtil::split(m_RecvStr, eol, -1);

//			SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::execute - [ST] parse(size=%zu)", lines.size());

			for (INT32 i = 1; (static_cast<INT32>(lines.size()) > i) && (OK == Ret); ++i)
			{
				Ret = AgentReq.parse(lines, i);
			}

//			SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::execute - [ED] parse(size=%zu)", lines.size());
		}

		/*** SNMP使用 ***/

		if (OK != Ret)
		{
			m_ErrCode   = "ResultCode=GWE100002\r\n";
			m_ErrDetail = "ErrorDetail=<Bad Request> " + m_RecvStr + "\r\n";
		}

		// SNMPクライアント実行
		if (OK == Ret)
		{
			Ret = proxySnmpClient(AgentReq);
		}

		// レスポンスを送信する
		std::string response;

		Ret = getResponsData(AgentReq, response);
		if (OK == Ret)
		{
			INT8* send_buff = (INT8*)(response.c_str());
			INT32 response_size = static_cast<INT32>(response.size());

			remain_size = response_size;
			total_size = 0;
			INT32 sended_size = 0;

			while ((0 < remain_size) && (OK == Ret))
			{
				sended_size = m_Socket->send(&(send_buff[total_size]), static_cast<UINT32>(remain_size), 0);
				SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::sended sended_size=%d, send_buff=%s", sended_size, send_buff);
				if (0 <= sended_size)
				{
					total_size += sended_size;
				}
				else
				{
					SGWC_NetworkErrorCnt::newInstance()->setNetworkCnt_App(SGWE_MIOSOCKSEND);
					SGWC_NetworkErrorCnt::newInstance()->setNetworkErrorCnt_App(SGWE_MIOSOCKSEND, (short)Ret);	/* Mod Y.Yamada 2017/11/06 AccuRev_1026431 *//* Merged T.Matsushita 2018/01/31 AccuRev_1035029 */
					Ret = EIO;
				}

				remain_size = response_size - total_size;
			}
		}
	}
	else
	{
		m_Stop = TRUE;
	}

	return Ret;
}

/***********************************************************************/
/** @brief SNMPエージェント子スレッドメイン
 * 
 *  @retval STATUS	: 処理結果
 *  @author 
 *  @date 
 */
/***********************************************************************/
STATUS SGWC_SnmpAgentWorker::main()
{
	STATUS Ret = OK;

//	SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::main ENTER>> m_Socket=%p, m_SharedMsg=%p", m_Socket, m_SharedMsg);

	if ((NULL != m_Socket) && (NULL != m_SharedMsg))
	{
		while ((OK == Ret) && (FALSE == m_Stop))
		{
			Ret = execute();
		}
		if(Ret != OK) {
			SGWC_NetworkErrorCnt::newInstance()->setNetworkErrorCnt_App(SGWE_MIO_SGWDFUNC, SGWE_NETWORK_FUNC_SNMP);
		}

		m_Socket->shutdown(SGWD_SHUT_RDWR);
		delete m_Socket;
		m_Socket = NULL;
	}
	else
	{
		// 20160817 静的解析指摘対応. #53782
		if (NULL != m_Socket)
		{
			delete m_Socket;
			m_Socket = NULL;
		}
		
		Ret = EINVAL;
	}

	m_Finished = TRUE;

//	SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::main LEAVE>> Ret=%d",  Ret);

	return Ret;
}

/***********************************************************************/
/** @brief レスポンスストリング取得
 * 
 *  @param[in] AgentReq			: SNMPエージェントリクエスト
 *  @param[out] strReturnData	: レスポンスストリング
 *  @retval STATUS				: 処理結果
 *  @author 
 *  @date 
 */
/***********************************************************************/
STATUS SGWC_SnmpAgentWorker::getResponsData(SGWC_SnmpAgentReq& AgentReq, std::string& strReturnData)
{
	STATUS ret = OK;

	std::string strRspData;
	std::string strValData;

	SGWC_SnmpVariable stVal;

	short sNum = 0;
	short sRec = 0;
	std::ostringstream strNum;
	std::ostringstream strRec;
	std::ostringstream strRes;
	std::ostringstream strResDetail;

	t_sMioSNMPVariable* pNow = NULL;
	std::string strReqOID;
	std::string strResOID;

	std::string strVal;
	INT8*       pVal;

	// リストを全検索.
	std::vector< SGWC_SnmpAgentReqData* >& vResList = AgentReq.getReqDataList();
	for (std::vector< SGWC_SnmpAgentReqData* >::iterator it = vResList.begin(); it != vResList.end(); it++)
	{
		sRec = 0;
		strNum.str("");
		strNum.clear();
		strNum << sNum;

		strReqOID = (*it)->getReqOID();
		SGWC_SnmpClient& SnmpClient = (*it)->getSnmpClient();

		SGWC_SnmpClientRsp& rsp = SnmpClient.getRsp();
		t_eMioSNMPClientRspCode RspCode = (t_eMioSNMPClientRspCode)(rsp.getRspCode());

		SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SNMP rspCode:%d", RspCode);

		/*** 結果, エラー詳細, 応答データリストの個別結果に変換 ***/
		strRes.str("");
		strRes.clear();
		switch (RspCode)
		{
		case MIO_SNMPC_SUCCESS:
			// 正常終了.
			strRes << 0; // 成功.

			/*** 取得変数リストを生成 ***/
			strResDetail.str("");
			strResDetail.clear();
			strResDetail << rsp.getRspCode();

			// ResponseData と GetVariableBindings を別々に作成.
			pNow = rsp.getVariable();
//			SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::getResponsData %p = rsp.getVariable()", pNow);
			while (pNow != NULL)
			{
				strResOID = reinterpret_cast<const INT8*>(stVal.getOID(pNow));

//				SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::getResponsData strResOID.c_str()=%s, strReqOID.c_str()=%s", strResOID.c_str(), strReqOID.c_str());
				// 応答Valiableから検索する.
				if (NULL == ::strstr(strResOID.c_str(), strReqOID.c_str()))
				{
					// 違う場合は、次を検索する.
					pNow = pNow->pNext;
					continue;
				}

				strRec.str("");
				strRec.clear();
				strRec << sRec;

				strVal = "";
				pVal = stVal.getVal(pNow);
				if (NULL != pVal)
				{
					strVal = (const INT8*)pVal;
				}

				// VariableDataを作成.
				strValData += "GetVariableBindings-" + strNum.str() + "-" + strRec.str() + "=" + (const INT8*)(stVal.getOID(pNow)) + "," + strResDetail.str() + "," + (const INT8*)(stVal.getASN(pNow)) + "," + strVal + "\r\n";
				// リクエストを加算.
				sRec++;
				// 次のデータ.
				pNow = pNow->pNext;
			}
			break;
		case MIO_SNMPC_TIMEOUT:
			// SNMPリクエストリトライオーバー.
			m_ErrCode   = "ResultCode=GWE100005\r\n";
			m_ErrDetail = "ErrorDetail=<Retry Over> (excecuted request : " + AgentReq.getRetry() + "/" + AgentReq.getRetry() + ")\r\n";
			strRes << 1; // リトライ回数オーバー.
			break;
		default:
			// SNMPリクエスト送信失敗.
			m_ErrCode   = "ResultCode=GWE100003\r\n";
			m_ErrDetail = "ErrorDetail=<I/O error> (excecuted request : " + AgentReq.getRetry() + "/" + AgentReq.getRetry() + ")\r\n";
			strRes << 2; // I/Oエラー.
			break;
		}

		/*** 応答データリストを生成 ***/
		strRec.str("");
		strRec.clear();
		strRec << sRec;
		strRspData += "ResponseData-" + strNum.str() + "=" + strReqOID + "," + strRes.str() + "," + strRec.str() + "\r\n";
		sNum++; // レコード数をアップ.
	}

//	SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "strRspData:%s", strRspData.c_str());
//	SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "strValData:%s", strValData.c_str());

	// 最後に結合する.
	std::string strData;
	std::ostringstream strLength;

	strData = m_ErrCode + m_ErrDetail + strRspData + strValData;
	strLength << strData.size();
	strReturnData = "Length=" + strLength.str() + "\r\n" + strData;

	return ret;
}

/***********************************************************************/
/** @brief SNMPクライアント実行
 * 
 *  @param[in,out] AgentReq	: SNMPエージェントリクエスト
 *  @retval STATUS			: 処理結果
 *  @author 
 *  @date 
 */
/***********************************************************************/
STATUS SGWC_SnmpAgentWorker::proxySnmpClient(SGWC_SnmpAgentReq& AgentReq)
{
	STATUS Ret = OK;

//	SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::proxySnmpClient - execute SNMP process");

	// 送信先IPアドレス取得
	SGWS_DeviceInfo DeviceInfo;

	memset(&DeviceInfo, 0, sizeof(DeviceInfo));
	strncpy(DeviceInfo.sc_ID, (AgentReq.getDevID()).c_str(), sizeof(DeviceInfo.sc_ID) - 1);
	Ret = SGWC_DBDeviceList::newInstance()->getList(&DeviceInfo);
	if (OK != Ret)
	{
		m_ErrCode   = "ResultCode=GWE100004\r\n";
		m_ErrDetail = "ErrorDetail=<Bad DeviceId> " + AgentReq.getDevID() + "\r\n";
	}

	// SNMP要求リスト生成
	if (OK == Ret)
	{
		std::vector< SGWC_SnmpAgentReqData* >& ReqDataList = AgentReq.getReqDataList();
		t_sMioSNMPVariable* Variable;

//		SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::proxySnmpClient - [ST] create(size=%zu)", ReqDataList.size());

		for (std::vector< SGWC_SnmpAgentReqData* >::iterator it = ReqDataList.begin() ; ReqDataList.end() != it; ++it)
		{
			SGWC_SnmpClient& SnmpClient = (*it)->getSnmpClient();
			SGWC_SnmpClientReq& SnmpClientReq = SnmpClient.getReq();

			SnmpClientReq.setSesID(SGWF_MioGetSessionID());
			SnmpClientReq.setCommand((INT8*)(((*it)->getReqType()).c_str()));
			SnmpClientReq.setPort("161");
			SnmpClientReq.setHostAddr(DeviceInfo.sc_IP);
			SnmpClientReq.setCommunity("public");
			SnmpClientReq.setTimeout((INT8*)(AgentReq.getReqTimeout().c_str()));
			SnmpClientReq.setRetry((INT8*)(AgentReq.getRetry().c_str()));

			Variable = (t_sMioSNMPVariable*)(malloc(sizeof(t_sMioSNMPVariable)));
			if (NULL != Variable)
			{
				::memset(Variable, 0, sizeof(t_sMioSNMPVariable));
				SGWC_SnmpVariable::setOID(Variable, (INT8*)(((*it)->getReqOID()).c_str()));
				SnmpClientReq.setVariable(Variable);
			}
			else
			{
				Ret = ENOMEM;
			}

			if (OK != Ret)
			{
				m_ErrCode   = "ResultCode=GWE999999\r\n";
				m_ErrDetail = "ErrorDetail=<unknown error> " + m_RecvStr + "\r\n";
				break;
			}
		}

//		SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::proxySnmpClient - [ED] create(size=%zu)", ReqDataList.size());
	}

	// SNMP要求リスト実行
	if (OK == Ret)
	{
		SGWC_Timer* Timer = SGWC_Timer::newInstance();
		if (NULL != Timer)
		{
			UINT32 SeqNo = 0;
			double EndTime = 0;
			double TotalTimeout = 0;
			std::stringstream ConvTotalTimeout(AgentReq.getTotalTimeout());

			ConvTotalTimeout >> TotalTimeout;
			EndTime = Timer->getMfpTime() + (TotalTimeout * 1e-3);

			std::vector< SGWC_SnmpAgentReqData* >& ReqDataList = AgentReq.getReqDataList();

//			SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::proxySnmpClient - [ST] request(size=%zu)", ReqDataList.size());

			for (std::vector< SGWC_SnmpAgentReqData* >::iterator it = ReqDataList.begin() ; ReqDataList.end() != it; ++it)
			{
				SGWC_SnmpClient& SnmpClient = (*it)->getSnmpClient();

				Ret = SnmpClient.request(m_SharedMsg, SeqNo++);
				if (OK != Ret)
				{
					m_ErrCode   = "ResultCode=GWE999999\r\n";
					m_ErrDetail = "ErrorDetail=<unknown error> " + m_RecvStr + "\r\n";
					break;
				}

				if (EndTime <= Timer->getMfpTime())
				{
					m_ErrCode   = "ResultCode=GWE100001\r\n";
					m_ErrDetail = "ErrorDetail=<timed out> (excecuted request : " + AgentReq.getRetry() + "/" + AgentReq.getRetry() + ")\r\n";
					Ret = ETIMEDOUT;
					break;
				}
			}

//			SGWC_Log::newInstance()->logmsg(SGWD_LogLevel_DEBUG, "SGWC_SnmpAgentWorker::proxySnmpClient - [ED] request(size=%zu)", ReqDataList.size());
		}
		else
		{
			Ret = ENOMEM;
		}
	}

	return Ret;
}
