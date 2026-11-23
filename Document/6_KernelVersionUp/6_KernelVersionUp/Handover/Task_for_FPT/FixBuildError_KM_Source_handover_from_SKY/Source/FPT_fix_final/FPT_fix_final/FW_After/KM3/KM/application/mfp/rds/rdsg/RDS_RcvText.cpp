//
//ファイル名：RDS_RcvText.cpp
//
//概要：ホストからの要求データ受信処理
//
//備考：
//・ホストからの要求データ受信処理は、Email/FAX/Telephonyシステムに依存すべきではない。
//　（Telephony固有のユーザデータの終端CRLFを期待してはいけない）
//・DXコマンド以外は"１コマンドの１パラメータを複数に分割する事は無い"という前提で設計を行った。
//　しかし、DXコマンド(部門管理データ用)は、この前提が成立しない（仕様が決まるのが遅かった）。
//　よって、DXコマンドの処理を 特殊扱いとしている。
//
//前提事項
//・「応答電文１のMoreが付く電文は最大長（固定長）」という制限は無い。(Telephony)
//・Email/FAXシステムは 送受信バッファサイズは、Mbytes単位で確保できる可能性がある。
//　よって、バッファサイズを保持する型はint32_tとした。
//
//Inquiry応答電文がMoreになる条件(Telephony)：
//・次のコマンドが書けない可能性がある。
//　－＞残り書込サイズが（最大データ長＋６）未満
//　（空白＋コマンド２文字＋パラメータ２文字＋最大データ長＋カンマ）
//・次のパラメータ＋データが書けない可能性がある。
//　－＞残り書込サイズが（最大データ長＋３）未満
//　（パラメータ２文字＋最大データ長＋カンマ）
//※パラメータが１つの場合でも カンマを考慮して判定する。

//From: Tetsuo Kimoto <tetsuo.kimoto@konicaminolta.jp>
//Date: Fri, 31 Oct 2003 17:32:50 +0900
//Subject: Re: Laplata1/Rhine1データ（パラメータ）が不連続（途中がない）の場合
//
//データ（パラメータ）が不連続（途中がない）の場合、固定長・可変長ともに
//複写機からの応答電文のユーザデータは、
//	l1000426-60-9269,01,020426-60-9270
//　　　　　　　　　　↑
//のようにデータがない場合でも、パラメータだけは付加してください。
//
//ただし、コマンド表でパラメータ数が256個まで規定されていても、
//データが128個しかない場合、129以降のパラメータ付加は不要です。
//
//現状、ホストソフト上の制約となっています。
//メール版のデータも同様です。　→浦崎さん（川村さん）

#include	<stdio.h>
#include	<string.h>
#include	"system/include/typs.h"
#include	"RDS_RcvText.h"
#include	"RDS_TskEvent.h"
#include	"RDS_Instance.h"	// add by tomiyasu 2008/10/22
#include	"system/sys/APIC_SystemError.h"
#include	"../api/jobspec/inc/JobSpec_define.h"		// 2015/09/15 mtoyoda 制御構成変更_No.76_E002詳細コード追加.
#include	"rds/rdsc/RDS_ReadCommand.h"	// 2016/05/12 H.Tokuno ZeusSBK 制御構成変更_No.57 カーネルログ取得による市場問題検証対応_Step2.
// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 start.
//#include	"../../../../shlib/LogLib/LIBC_LogLibDefine.h"		// 2017/03/27 日立 森本郁 [MFP管理技術_デバイス連携][KRQ-XXX]CSRA-DBMS連携.
// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 end.

#include "system/fil/FIL_FsWrapperC.h"
#include <stdlib.h>
#include "system/dbg/APIC_SystemLogging.h"

// 2018/11/01 HCLT-Jerin 【IT6_1.0】【Sparrow】 AR1058458 Added Start.
#include	"system/pwr/APIC_Power.h"
#include	"system/job/JOBC_JobIf.h"
#include	"system/prt/APIC_Printer.h"
// 2018/11/01 HCLT-Jerin 【IT6_1.0】【Sparrow】 AR1058458 Added End.
// 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Start.
#include	"RDS_Util.h"
#include	"system/thr_it6/thrcom/THRC_Threadif.h"
// 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 End.
// ログ出力
#define	RDSF_Log(msg)	APIC_ThrowSystemError(SYSE_SysErrTaskLog, LOGF_DEBUG_EVENT_KIND_RDS, msg)

// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 start.
// 2017/03/16 日立 森本郁 [MFP管理技術_デバイス連携]KRQ-XXX CSRA-DBMS連携 Start.
//static const RDSS_DMBSCommandInfo g_st_DBMSCommandInfo[] = {						// CSRA - DBMSテーブル種別対応管理用テーブル.
//	{ " NA",	DBAE_EnginLog },									// エンジンデータ.
//};
// 2017/03/16 日立 森本郁 [MFP管理技術_デバイス連携]KRQ-XXX CSRA-DBMS連携 End.
// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 end.

//
// RDSC_CommandMatrix
//

RDSC_CommandMatrix*	RDSC_CommandMatrix::mpCommandMatrixForEmail = 0;		// Email用コマンドMatrixのアドレス
RDSC_CommandMatrix*	RDSC_CommandMatrix::mpCommandMatrixForTelephony = 0;	// Telephony用コマンドMatrixのアドレス
RDSC_CommandMatrix*	RDSC_CommandMatrix::mpCommandMatrixForFax = 0;			// Fax用コマンドMatrixのアドレス

//概要：Email用コマンドMatrixのアドレスを取得
//戻り値：Email用コマンドMatrixのアドレス
//備考：
//作成者：宮本
//レビュー：2003/11/26、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成
	const RDSC_CommandMatrix*
RDSC_CommandMatrix::GetCommandMatrixForEmail()
{
	if (mpCommandMatrixForEmail == 0)
		mpCommandMatrixForEmail = new RDSC_CommandMatrix(SYSTEM_EMAIL);
	return mpCommandMatrixForEmail;
}

//概要：Telephony用コマンドMatrixのアドレスを取得
//戻り値：Telephony用コマンドMatrixのアドレス
//備考：
//作成者：宮本
//レビュー：2004/03/22、宮本
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2004/03/19、宮本、新規作成
	const RDSC_CommandMatrix*
RDSC_CommandMatrix::GetCommandMatrixForTelephony()
{
	if (mpCommandMatrixForTelephony == 0)
		mpCommandMatrixForTelephony = new RDSC_CommandMatrix(SYSTEM_TELEPHONY);
	return mpCommandMatrixForTelephony;
}

//概要：Fax用コマンドMatrixのアドレスを取得
//戻り値：Fax用コマンドMatrixのアドレス
//備考：
//作成者：宮本
//レビュー：2004/03/22、宮本
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2004/03/19、宮本、新規作成
	const RDSC_CommandMatrix*
RDSC_CommandMatrix::GetCommandMatrixForFax()
{
	if (mpCommandMatrixForFax == 0)
		mpCommandMatrixForFax = new RDSC_CommandMatrix(SYSTEM_FAX);
	return mpCommandMatrixForFax;
}

//デストラクタ
//作成者：宮本
//レビュー：2003/11/26、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成
RDSC_CommandMatrix::~RDSC_CommandMatrix()
{}

//概要：コマンドに対応する構造体のアドレスを取得する
//戻り値：コマンドに対応する構造体のアドレス
//備考：結果が失敗した時は０を返す
//作成者：宮本
//レビュー：2003/11/27、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成
	const RDSS_CommandInfo*
RDSC_CommandMatrix::GetCommandInfo(
		const unsigned char* pCommand,			// コマンド文字列
		Bool* pbIsGood							//結果
		) const
{
	int	nIndex0 = *pCommand++;
	if (('A' <= nIndex0) &&  nIndex0 <= 'Z')
		nIndex0 -= 'A';
	else
	{
		*pbIsGood = False;
		EPN_DBG_MESSAGE( (uchar*)"GetCommandInfo", __LINE__, (uchar*)"nIndex0 Error", nIndex0, 0, 0 );
		return 0;
	}

	int	nIndex1 = *pCommand++;
	if (('0' <= nIndex1) &&  nIndex1 <= '9')
		nIndex1 -= '0';
	else if (('A' <= nIndex1) &&  nIndex1 <= 'Z')
		nIndex1 -= ('A'-10);
	else
	{
		*pbIsGood = False;
		EPN_DBG_MESSAGE( (uchar*)"GetCommandInfo", __LINE__, (uchar*)"nIndex1 Error", nIndex0, nIndex1, 0 );
		return 0;
	}
	
	if (mCommandInfo[nIndex0][nIndex1].mDataSize != 0)
	{
		*pbIsGood = True;
		return &mCommandInfo[nIndex0][nIndex1];
	}
	else
	{
		//データサイズが０の時、無効なコマンドとする。
		*pbIsGood = False;
		EPN_DBG_MESSAGE( (uchar*)"GetCommandInfo", __LINE__, (uchar*)"mDataSize is 0", nIndex0, nIndex1, 0 );
		return 0;
	}
}

//概要：コマンドに対応する構造体のアドレスを取得する
//戻り値：コマンド情報
//備考：コンストラクタで初回しか呼ばれない事を想定。よって、ここでの異常検出は例外とした。
//作成者：宮本
//レビュー：2003/11/27、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成 
//          2006/11/01、浦崎、関数名の先頭に「_」を付けてもう一方の同名関数と明示的に区別するようにした。【CommandMatrix 初期化タイミング変更】  
//			2011/02/07、樋口、静的解析対応 CID_014692 (ソース上、問題ないが修正).
	RDSS_CommandInfo*
RDSC_CommandMatrix::_GetCommandInfo(
		int nIndex0,			// コマンドの最初
		int nIndex1				// コマンドの２番目
		)
{
	if (('A' <= nIndex0) &&  nIndex0 <= 'Z')
		nIndex0 -= 'A';
	else
	{									// 2011/02/07 higuchi Minerva/Zeus 【静的解析 影響度高 CID_014692】.
		//throw;
		APIC_ThrowSystemErrorDetailCode(							// 2015/09/15 mtoyoda 制御構成変更_No.76_E002詳細コード追加.
					SYSE_SysErrTypeFatalLogicalError,               // 内部論理矛盾(復旧不可能なもの)
					LOGF_DEBUG_EVENT_KIND_RDS, 
					"(E)CommandMatrix::CommandMatrix() 不正コマンド",
					CE002_DetailCode0x585C							// 2015/09/15 mtoyoda 制御構成変更_No.76_E002詳細コード追加.
					);
		return &mCommandInfo[0][0];		// 2011/02/07 higuchi Minerva/Zeus 【静的解析 影響度高 CID_014692】.
	}									// 2011/02/07 higuchi Minerva/Zeus 【静的解析 影響度高 CID_014692】.

	if (('0' <= nIndex1) &&  nIndex1 <= '9')
		nIndex1 -= '0';
	else if (('A' <= nIndex1) &&  nIndex1 <= 'Z')
		nIndex1 -= ('A'-10);
	else
	{									// 2011/02/07 higuchi Minerva/Zeus 【静的解析 影響度高 CID_014692】.
		//throw;
		APIC_ThrowSystemErrorDetailCode(							// 2015/09/15 mtoyoda 制御構成変更_No.76_E002詳細コード追加.
					SYSE_SysErrTypeFatalLogicalError,               // 内部論理矛盾(復旧不可能なもの)
					LOGF_DEBUG_EVENT_KIND_RDS, 
					"(E)CommandMatrix::CommandMatrix() 不正コマンド",
					CE002_DetailCode0x585D							// 2015/09/15 mtoyoda 制御構成変更_No.76_E002詳細コード追加.
					);
		return &mCommandInfo[0][0];		// 2011/02/07 higuchi Minerva/Zeus 【静的解析 影響度高 CID_014692】.
	}									// 2011/02/07 higuchi Minerva/Zeus 【静的解析 影響度高 CID_014692】.

	return &mCommandInfo[nIndex0][nIndex1];
}

//予約文字
const unsigned char	CodeOfSpace	= ' ';	//空白の文字コード
const unsigned char	CodeOfComma	= ',';	//カンマの文字コード
const unsigned char	CodeOfCR = '\x0d';	//CRの文字コード
const unsigned char	CodeOfLF = '\x0a';	//LFの文字コード

//規定文字
const unsigned char	CodeOfRegulatedSpace	= '\x1d';	//空白の文字コード
const unsigned char	CodeOfRegulatedComma	= '\x1c';	//カンマの文字コード
const unsigned char	CodeOfRegulatedCR		= '\x1e';	//CRの文字コード
const unsigned char	CodeOfRegulatedLF		= '\x1f';	//LFの文字コード

//概要：16進２文字を数値に変換
//戻り値：変換した数値
//備考：不正な文字だった場合は、引数の結果がFalseで０を返す
//作成者：宮本
//レビュー：2003/11/27、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成
	int
RDSC_CommandMatrix::Hex2Toi(
		const unsigned char* p,			// 文字列
		Bool* pbIsGood					// 結果
		)
{
	int	nValue = 0;

	for (int i=0; i<2; i++)
	{
		int	n = *p++;
		nValue *= 16;
		if (('0' <= n) &&  n <= '9')
			n -= '0';
		else if (('A' <= n) &&	n <= 'Z')
			n -= ('A'-10);
		else
		{
			*pbIsGood = False;
			return 0;
		}
		nValue += n;
	}
	*pbIsGood = True;
	return nValue;
}
//概要：予約文字を 規定文字に変換
//戻り値：無し
//備考：文字列は NULL終端でなくても良い
//作成者：宮本
//レビュー：2003/11/27、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成
//			2004/04/05、宮本、予約語変換にCRLFを追加
	void
RDSC_CommandMatrix::ConvertReservedToRegulated(
		unsigned char* p,				// 文字列の先頭アドレス
		int size						// 文字列のサイズ
		)
{
	for (unsigned char* pEnd = p + size; p < pEnd; p++)
	{
		switch (*p)
		{
		case CodeOfSpace:
			*p = CodeOfRegulatedSpace;
			break;
		case CodeOfComma:
			*p = CodeOfRegulatedComma;
			break;
		case CodeOfCR:
			*p = CodeOfRegulatedCR;
			break;
		case CodeOfLF:
			*p = CodeOfRegulatedLF;
			break;
		default:
			break;
		}
	}
}

// 2023/02/08 FPT-TinhNT16 【IT6_2.3】【RQ-941】【OP_BTS-41860】 Start.
//--------------------------------------------------------------------------------------------------.
// Overview			: Convert standard characters to reserved characters for ZO command.
// Creator			: Fpt.
// Review			:.
// Change history	: 2023/02/08 FPT-TinhNT16 【IT6_2.3】【RQ-941】【OP_BTS-41860】 Added.
//----------------------------------------------------------------------------------------------------.
	void
RDSC_CommandMatrix::ConvertRegulatedToReservedForZO(
		unsigned char* p,
		int size
		)
{
	for (unsigned char* pEnd = p + size; p < pEnd; p++)
	{
		switch (*p)
		{
		case CodeOfSpace:
			*p = CodeOfRegulatedSpace;
			break;
		case CodeOfCR:
			*p = CodeOfRegulatedCR;
			break;
		case CodeOfLF:
			*p = CodeOfRegulatedLF;
			break;
		default:
			break;
		}
	}
}
// 2023/02/08 FPT-TinhNT16 【IT6_2.3】【RQ-941】【OP_BTS-41860】 End.

//概要：規定文字を 予約文字に変換
//戻り値：無し
//備考：文字列は NULL終端でなくても良い
//作成者：宮本
//レビュー：2003/11/27、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成
//			2004/04/05、宮本、予約語変換にCRLFを追加
	void
RDSC_CommandMatrix::ConvertRegulatedToReserved(
		unsigned char* p,				// 文字列の先頭アドレス
		int size						// 文字列のサイズ
		)
{
	for (unsigned char* pEnd = p + size; p < pEnd; p++)
	{
		switch (*p)
		{
		case CodeOfRegulatedSpace:
			*p = CodeOfSpace;
			break;
		case CodeOfRegulatedComma:
			*p = CodeOfComma;
			break;
		case CodeOfRegulatedCR:
			*p = CodeOfCR;
			break;
		case CodeOfRegulatedLF:
			*p = CodeOfLF;
			break;
		default:
			break;
		}
	}
}

// 2016/05/30 H.Tokuno ZeusSBK CQ530311_Dmesg制御文字変換対応 Start.

// 取扱不可制御文字最後尾
const unsigned char	CodeOfCtrlCharLast  = 0x1B;	//ESCの文字コード.

//概要：制御文字0x00～0x1Bについて" "半角スペースへと変換する.
//      (予約文字として規定された0x0A(LF)、0x0D(CR)を除く).
//      引数のポインタがNULLである場合、何もせず終了する.
//前提：予約文字を規定文字に変換する処理(ConvertReservedToRegulated)が先行して行われていること.
//戻り値：無し.
//備考：文字列は NULL終端でなくても良い.
	void
RDSC_CommandMatrix::ConvertControlCharacterToRegulatedSpace(
		unsigned char* p,				// 文字列の先頭アドレス.
		int size						// 文字列のサイズ.
		)
{
    if ( p == NULL )
    {
		EPN_DBG_MESSAGE( (uchar*)"ConvertControlCharacterToRegulatedSpace", (uint32_t)0, (uchar*)"pointer is null" );
        return;
    }

	for (int i = 0; i < size; i++)
	{
		if ( *p <= CodeOfCtrlCharLast )
		{
			// CRLFについては対象外とする.
			if ( (*p != CodeOfCR) && (*p != CodeOfLF) )
			{
				*p = CodeOfRegulatedSpace;
				
			} 
		}
		p++;
	}
}
// 2016/05/30 H.Tokuno ZeusSBK CQ530311_Dmesg制御文字変換対応 End.


//
// RDSC_Inquiry
//

//概要：１コマンドのチェック
//戻り値：コマンドチェックの結果
//備考：コマンドにはパラメータがある場合と無い場合がある。
//　　　コマンド文字列の先頭は空白。
//作成者：宮本
//レビュー：2003/11/27、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成
	RDSE_SYNTAX_RESULT
RDSC_Inquiry::CheckCommand(
		const unsigned char* const pCommand,	// [IN]  コマンド文字列
		int32_t commandSize,						// [IN]  コマンド文字列のサイズ
		const RDSS_CommandInfo*& rpCommandInfo,	// [OUT] コマンド情報のアドレスの参照(異常時は０が格納される)
		int32_t* pCheckedCount,					// [OUT] チェックした文字数(異常時は０)
												//		 空白が続く場合は チェック済文字数に空白は含まなれない
		short* pTheNumberOfParam				// [OUT] 指定されたパラメータ数を格納するアドレス
		) const
{
	Bool	bIsGood = False;
	RDSE_SYNTAX_RESULT	result = RDSE_SYNTAX_RESULT_NO_DELIMITER;		//結果
	const unsigned char*	p = pCommand;
	const unsigned char*	pCommandEnd = p + commandSize;	// コマンド文字列の終端アドレス

	*pCheckedCount = 0;
	*pTheNumberOfParam = 0;

	//空白を期待する。
	if ((*p++ != CodeOfSpace) || (commandSize < (1+2))) return RDSE_SYNTAX_RESULT_NO_DELIMITER;
	//コマンドを期待する
	rpCommandInfo = mrCommandMatrix.GetCommandInfo(p, &bIsGood);
	//不正コマンドなら、終了
	if (!bIsGood) return RDSE_SYNTAX_RESULT_SYNTAX_ERROR;
// 2012/10/16 CQ.373522 Sky山村 モデム通信 DXエラー対応 start.
	// ★TODO:モデム通信の場合、DXは、MFPの条件を判断する.
	RDSE_TaskID taskID = theRdsApl.isRdsSystem();
	if( taskID == RDSE_TSK_ID_TELL )
	{
		if ( IsGoodDX(pCommand, commandSize) )
		{
			if( False == isEnableStatus_ReadDX() )
			{
				// 受付条件を確認を満たさない場合、エラー.
				return RDSE_SYNTAX_RESULT_SYNTAX_ERROR;
			}
		}
	}
// 2012/10/16 CQ.373522 Sky山村 モデム通信 DXエラー対応 end.

// 2018/07/03 HCLT-Vinay 【IT6_1.0】【制御構成変更_No.110】 Start.
	if( taskID == RDSE_TSK_ID_TELL )
	{
		if ( IsGoodZP(pCommand, commandSize) )
		{
			if( False == isEnableStatus_ReadZP() )
			{
				// 受付条件を確認を満たさない場合、エラー.
				return RDSE_SYNTAX_RESULT_SYNTAX_ERROR;
			}
		}
	}
// 2018/07/03 HCLT-Vinay 【IT6_1.0】【制御構成変更_No.110】 End.

// 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Start.
	if( taskID == RDSE_TSK_ID_TELL )
	{
		if ( IsGoodZV(pCommand, commandSize) )		//2022/10/26 FPT-TinhNT16 [IT6_2.2MR3][OP_BTS-40118] Modified.
		{
			// 受付条件を確認を満たさない場合、エラー.
			return RDSE_SYNTAX_RESULT_SYNTAX_ERROR;
		}
	}
// 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 End.

// 2017/02/06 H.Tokuno IT5_4.1 機器構成変更_No.49 CTLログの容量拡張 Start.
	// HDDアクセスを必要とするコマンドか否かを判定する.
	checkCommandNeedHDDAccess(pCommand);
// 2017/02/06 H.Tokuno IT5_4.1 機器構成変更_No.49 CTLログの容量拡張 End.
	
	p += 2;

	if (*p == CodeOfSpace)	//空白
	{
		//パラメータ無しのコマンドだった（正常（空白終端を検出））
		result = RDSE_SYNTAX_RESULT_OK_SPACE;
	}
	else if (p == pCommandEnd)	//終端
	{
		//パラメータ無しのコマンドだった（ユーザデータ文字列終端を検出）
		result = RDSE_SYNTAX_RESULT_OK_END;
	}
	else	//パラメータ
	{
		//パラメータがあるコマンド
		Bool	bInLoop = True;

		while (bInLoop)
		{
			//パラメータを期待
			//パラメータは２文字で、'0'～'9'、'A'～'F'
			int	nValue = RDSC_CommandMatrix::Hex2Toi(p, &bIsGood);
			//パラメータ値が不正
			if (!bIsGood) return RDSE_SYNTAX_RESULT_SYNTAX_ERROR;
			//最大値を越えている
			if (nValue > rpCommandInfo->mParamMax) return RDSE_SYNTAX_RESULT_SYNTAX_ERROR;
			p += 2;
			(*pTheNumberOfParam)++;	//パラメータ数を増加
			//' 'または','または"終端"を期待
			if (*p == CodeOfSpace)	//空白
			{
				//正常
				result = RDSE_SYNTAX_RESULT_OK_SPACE;
				bInLoop = False;
			}
			else if (*p == CodeOfComma)	//カンマ
			{
				p++;
				continue;
			}
			else if (p == pCommandEnd)
			{
				//正常（ユーザデータ文字列終端を検出）
				result = RDSE_SYNTAX_RESULT_OK_END;
				bInLoop = False;
			}
			else if (p > pCommandEnd)
			{
				result = RDSE_SYNTAX_RESULT_SYNTAX_ERROR;
				bInLoop = False;
			}
			else	//予期しない文字
			{
				result = RDSE_SYNTAX_RESULT_NO_DELIMITER;
				bInLoop = False;
			}
		}//end of while (bInLoop)
	}
	
	//エリアを越えて読んでいないか？
	if (p > pCommandEnd)
		result = RDSE_SYNTAX_RESULT_SYNTAX_ERROR;
	else
		*pCheckedCount = (int32_t)(p - pCommand);	// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
	return result;
}

//コンストラクタ
//作成者：宮本
//レビュー：2003/11/27、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成
//			2004/04/05、宮本、DXコマンド対応
RDSC_Inquiry::RDSC_Inquiry(
		const RDSC_CommandMatrix* pCommandMatrix):	// コマンドMatrixのアドレス	
		mStatus(STATUS_NORMAL_END),
		mpCommandTop(0),
		mpCommandEnd(0),
		mbIsHalfOfCommand(False),
		mbIsExistParam(True),
		mNextParamValue(0),
#if !defined(CSRC_INVADIDATE_DX)
		mOffsetOfXmlFile(0),
#endif	//end of #if !defined(CSRC_INVADIDATE_DX)
		mrCommandMatrix(*pCommandMatrix),
// 2016/05/12 H.Tokuno ZeusSBK 制御構成変更_No.57 カーネルログ取得による市場問題検証対応_Step2 Start.
		mOffsetOfDmsgFile(0),
		mDmsgFd(0),
		mDmsgFileSize(0),
		mAlreadyReadZHCommand(False),
// 2016/05/12 H.Tokuno ZeusSBK 制御構成変更_No.57 カーネルログ取得による市場問題検証対応_Step2 End.
// 2017/08/07 MAv5 エラーチェック機構 start.
		m_sc_NormalParamCnt(0),											//[AR : 1029580] 【 Eagle 】【Build error fix】 2018/03/19 [HCL_Raghu]
// 2017/08/07 MAv5 エラーチェック機構 end.
// 2017/02/06 H.Tokuno IT5_4.1 機器構成変更_No.49 CTLログの容量拡張 Start.
		mlOffsetOfEngLogFile(0),
		miEngLogFd(0),
		miEngLogFileSize(0),
		mbAlreadyReadZMCommand(False),
		mbExistNeedHDDAccessCommand(False)
// 2017/02/06 H.Tokuno IT5_4.1 機器構成変更_No.49 CTLログの容量拡張 End.

{
// 2010/03/01 yito UV-00866 機能カウンタ対応 start.
	meCreateXMLType = RDSE_CREATE_XML_MAX;
	memset( &mstXmlData[0], 0, sizeof(mstXmlData) );
// 2010/03/01 yito UV-00866 機能カウンタ対応 end  .
	memset( mCommand, 0, sizeof(mCommand) );		// 2011/02/07 higuchi Minerva/Zeus 【静的解析 影響度高 CID_023759】.
// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 start.
//	m_DbmsData_fd = 0;													// 2017/03/16 日立 森本郁 [MFP管理技術_デバイス連携]KRQ-XXX CSRA-DBMS連携.
//	m_ul_DbmsFileSize = 0;												// 2017/03/16 日立 森本郁 [MFP管理技術_デバイス連携]KRQ-XXX CSRA-DBMS連携.
//	memset( m_sc_DbmsFilePath, 0x00, sizeof(m_sc_DbmsFilePath) );		// 2017/03/16 日立 森本郁 [MFP管理技術_デバイス連携]KRQ-XXX CSRA-DBMS連携.
// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 end.
}
//CommandMatrixの初期化 
//作成者：浦崎 
//レビュー：
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2006/11/01、浦崎、新規作成
void RDSC_Inquiry::initCommandMatrix()
{
	// 【CommandMatrix 初期化タイミング変更】 06/11/01 浦崎 
	RDSC_CommandMatrix& Temp = const_cast<RDSC_CommandMatrix&>(mrCommandMatrix);
	Temp.initCommandMatrix();
}

#if !defined(CSRC_INVADIDATE_DX)
//概要：ＤＸコマンドのチェック
//戻り値：正しいＤＸコマンドか？
//備考：コマンドにはパラメータがある場合と無い場合がある。
//　　　コマンド文字列の先頭は空白。
//作成者：宮本
//レビュー：2004/05/08、宮本
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2004/04/05、宮本、DXコマンド対応
	Bool
RDSC_Inquiry::IsGoodDX(
		const unsigned char* const pCommand,	// [IN]  コマンド文字列
		int32_t commandSize						// [IN]  コマンド文字列のサイズ
		) const
{
	const char* const	p = reinterpret_cast<const char* const>(pCommand);
	//if (((commandSize == 3) && (strncmp(p, " DX", 3) == 0))
	//	|| ((commandSize == 5) && (strncmp(p, " DX00", 5) == 0)))
	if (( strncmp(p, " DX", 3) == 0)
		|| (strncmp(p, " DX00", 5) == 0))
	{
		return True;
	}
	else
	{
		return False;
	}
}
#endif	//end of #if !defined(CSRC_INVADIDATE_DX)

// 2018/07/03 HCLT-Vinay St.
	Bool
RDSC_Inquiry::IsGoodZP(
		const unsigned char* const pCommand,	// [IN]  コマンド文字列
		int32_t commandSize						// [IN]  コマンド文字列のサイズ
		) const
{
	const char* const	p = reinterpret_cast<const char* const>(pCommand);
	//if (((commandSize == 3) && (strncmp(p, " ZP", 3) == 0))
	//	|| ((commandSize == 5) && (strncmp(p, " ZP00", 5) == 0)))
	if (( strncmp(p, " ZP", 3) == 0)
		|| (strncmp(p, " ZP00", 5) == 0))
	{
		return True;
	}
	else
	{
		return False;
	}
}
// 2018/07/03 HCLT-Vinay End.

// 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Start.
//--------------------------------------------------------------------------------------------------.
// Overview			: Check syntax of ZV command.
// Creator			: Fpt.
// Review			:.
// Change history	: 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Add.
//----------------------------------------------------------------------------------------------------.
	Bool
RDSC_Inquiry::IsGoodZV(
		const unsigned char* const pCommand,	// [IN]  コマンド文字列
		int32_t commandSize						// [IN]  コマンド文字列のサイズ
		) const
{
	const char* const p = reinterpret_cast<const char* const>(pCommand);
	if (( strncmp(p, " ZV", 3) == 0)
		|| (strncmp(p, " ZV00", 5) == 0)) 
	{
		return True;
	}
	else {
		return False;
	}
}
// 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 End.

// 2016/05/12 H.Tokuno ZeusSBK 制御構成変更_No.57 カーネルログ取得による市場問題検証対応_Step2 Start.
//概要：ZHコマンドのチェック.
//戻り値：正しいZHコマンドか？.
//備考：コマンドにはパラメータがある場合と無い場合がある.
//      コマンド文字列の先頭は空白.
//      最初の3文字が" ZH"であることを確認する.
//作成者：H.Tokuno.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//			2016/05/12 新規作成.
	Bool
RDSC_Inquiry::IsGoodZH(
		const unsigned char* const pCommand	// [IN]  コマンド文字列.
		) const
{
	const char* const	p = reinterpret_cast<const char* const>(pCommand);
	if (( strncmp(p, " ZH", 3) == 0))
	{
		return True;
	}
	else
	{
		return False;
	}
}
// 2016/05/12 H.Tokuno ZeusSBK 制御構成変更_No.57 カーネルログ取得による市場問題検証対応_Step2 End.

// 2017/02/06 H.Tokuno IT5_4.1 機器構成変更_No.49 CTLログの容量拡張 Start.
//概要：ZMコマンドのチェック.
//戻り値：正しいZMコマンドか？.
//備考：コマンドにはパラメータがある場合と無い場合がある.
//      コマンド文字列の先頭は空白.
//      最初の3文字が" ZM"であることを確認する.
//作成者：H.Tokuno.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//			2017/02/06 新規作成.
	Bool
RDSC_Inquiry::IsGoodZM(
		const unsigned char* const pCommand	// [IN]  コマンド文字列.
		) const
{
	const char* const	p = reinterpret_cast<const char* const>(pCommand);
	if (( strncmp(p, " ZM", 3) == 0))
	{
		return True;
	}
	else
	{
		return False;
	}
}
//概要：ZNコマンドのチェック.
//戻り値：正しいZNコマンドか？.
//備考：コマンドにはパラメータがある場合と無い場合がある.
//      コマンド文字列の先頭は空白.
//      最初の3文字が" ZN"であることを確認する.
//作成者：H.Tokuno.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//			2017/02/06 新規作成.
	Bool
RDSC_Inquiry::IsGoodZN(
		const unsigned char* const pCommand	// [IN]  コマンド文字列.
		) const
{
	const char* const	p = reinterpret_cast<const char* const>(pCommand);
	if (( strncmp(p, " ZN", 3) == 0))
	{
		return True;
	}
	else
	{
		return False;
	}
}

//概要：コマンド1件に対して、HDDアクセスを必要とするか否かをチェックする.
//戻り値：HDDアクセスを必要とするか否か.
//備考：ZM、ZN以外にも必要とするコマンドが現れた場合、ここに追加.
//      mbExistNeedHDDAccessCommandがTrueになるのはここだけ.
	void
RDSC_Inquiry::checkCommandNeedHDDAccess(
		const unsigned char* const pCommand	// [IN]  コマンド文字列.
		) const
{
	APIC_HddInfo*	p_hddinfo	= APIC_HddInfo::newInstance();
	if ( (p_hddinfo != NULL) && (p_hddinfo->getHddExist() == True) )
	{
		if ( IsGoodZM(pCommand) || IsGoodZN(pCommand) )
		{
			EPN_DBG_MESSAGE((uchar*)"CSRC", (uint32_t)0, (uchar*)"HDD Access command contains");
			mbExistNeedHDDAccessCommand = True;
		}
	}
}


//概要：HDD復帰処理が必要なコマンドが含まれていたかを示すフラグの取得関数.
//戻り値：HDD復帰処理が必要なコマンドが含まれていたか否か.
//備考：RDSC_Inquiry::CheckSyntax()コール後に本関数をコールすること.
//作成者：H.Tokuno.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//			2017/02/06 新規作成.
	Bool
RDSC_Inquiry::IsNeedHDDAccess()
{
	return mbExistNeedHDDAccessCommand;
}
// 2017/02/06 H.Tokuno IT5_4.1 機器構成変更_No.49 CTLログの容量拡張 End.

// 2010/03/01 yito UV-00866 機能カウンタ対応 start.
//------------------------------------------------------------------------------
//概要		：	機能カウンタコマンドのチェック
//戻り値	：	True 機能カウンタコマンドである.
//				False 機能カウンタコマンドではない.
//備考		：	コマンドにはパラメータがある場合と無い場合がある。
//　　　		コマンド文字列の先頭は空白。
//作成者	：	伊藤.
//変更履歴	：	変更日、変更者、変更内容（変更理由）を記述.
//				2010/03/01、伊藤　新規作成.
//------------------------------------------------------------------------------
	Bool
RDSC_Inquiry::IsGoodFuncCounterCommand(
		const uchar* const pCommand,	// [IN]  コマンド文字列
		int32_t commandSize						// [IN]  コマンド文字列のサイズ
		)
{
	const char* const	p = reinterpret_cast<const char* const>(pCommand);
	if ( ( strncmp(p, " BX", 3) == 0) || (strncmp(p, " BX00", 5) == 0) )
	{
		meCreateXMLType = RDSE_CREATE_XML_FUNCTION_COUNTERS;
		return True;
	}
// 2015/10/28 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(AQコマンド追加) START.
	else if( (strncmp(p, " AQ", 3) == 0) || (strncmp(p, " AQ00", 5) == 0) )
	{
		meCreateXMLType = RDSE_CREATE_XML_APPUSEFREQ_COUNTERS;
		return True;
	}
// 2015/10/28 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(AQコマンド追加) END.
	else if( (strncmp(p, " AR", 3) == 0) || (strncmp(p, " AR00", 5) == 0) )
	{
		meCreateXMLType = RDSE_CREATE_XML_UNIVERSAL_SETTINGS;
		return True;
	}
	else if( (strncmp(p, " AS", 3) == 0) || (strncmp(p, " AS00", 5) == 0) )
	{
		meCreateXMLType = RDSE_CREATE_XML_USER_SETTINGS;
		return True;
	}
	else if( (strncmp(p, " AT", 3) == 0) || (strncmp(p, " AT00", 5) == 0) )
	{
		meCreateXMLType = RDSE_CREATE_XML_ADMINISTRATOR_SETTINGS;
		return True;
	}
	else if( (strncmp(p, " AU", 3) == 0) || (strncmp(p, " AU00", 5) == 0) )
	{
		meCreateXMLType = RDSE_CREATE_XML_SERVICE_SETTINGS;
		return True;
	}
// 2015/11/05 ktakahashi ZeusS_ZX0 制御構成変更_No.76-2 ネットワークエラーカウンタ対応 Start.
	else if( (strncmp(p, " EM", 3) == 0) || (strncmp(p, " EM00", 5) == 0) )
	{
		meCreateXMLType = RDSE_CREATE_XML_NETWORKERR_COUNTERS;
		return True;
	}
// 2015/11/05 ktakahashi ZeusS_ZX0 制御構成変更_No.76-2 ネットワークエラーカウンタ対応 End.
	else
	{
		meCreateXMLType = RDSE_CREATE_XML_MAX;		// 例外値として最大値をセット.
		return False;
	}
}
// 2010/03/01 yito UV-00866 機能カウンタ対応 end  .

// 2017/03/16 日立 森本郁 [MFP管理技術_デバイス連携]KRQ-XXX CSRA-DBMS連携 Start.
//------------------------------------------------------------------------------.
//概要		：	DBMS取得コマンドのチェック.
//戻り値	：	True：	DBMS取得コマンドである.
//				False：	DBMS取得コマンドではない.
//備考		：	コマンドにはパラメータがある場合と無い場合がある.
//　　　		コマンド文字列の先頭は空白.
//作成者	：	日立 森本郁.
//変更履歴	：	変更日、変更者、変更内容（変更理由）を記述.
//------------------------------------------------------------------------------.
Bool RDSC_Inquiry::IsGoodDBMSCommand(
		const uchar* const	puc_Command,		// [IN]		コマンド文字列.
// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 start.
//		DBAE_DataType*		pe_DBMSDataKind		// [OUT]	DBMSテーブル種別.
		RDSE_CREATE_XML*	pe_CreateXmlKind	// [OUT]	生成するXMLデータ種別.
// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 end.
		)
{
	ushort				us_roopCnt		= 0;
// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 start.
//	ushort				us_roopCntMax	= sizeof( g_st_DBMSCommandInfo ) / sizeof( RDSS_DMBSCommandInfo );
// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 end.
	const char* const	psc_compStr = reinterpret_cast<const char* const>(puc_Command);
	
	// パラメータチェック.
	if( ( NULL == puc_Command ) ||
// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 start.
//		( NULL == pe_DBMSDataKind ) ) {
		( NULL == pe_CreateXmlKind ) ) {
// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 end.
		return False;
	}
	
// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 start.
//	for( us_roopCnt = 0; us_roopCnt < us_roopCntMax; us_roopCnt++ ) {
// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 end.
	for( us_roopCnt = 0; us_roopCnt < g_us_DbmsCommandInfoTableSize; us_roopCnt++ ) {
		if( 0 == strncmp( psc_compStr, g_st_DBMSCommandInfo[us_roopCnt].m_sc_CommandName, RDSD_CSRA_CHECK_COMMAND_NAME_LENGTH ) ) {
// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 start.
//			*pe_DBMSDataKind = g_st_DBMSCommandInfo[us_roopCnt].m_e_DbmsDataKind;
			*pe_CreateXmlKind = g_st_DBMSCommandInfo[us_roopCnt].m_e_CreateXMLKind;
			// 該当するXML種別を保持する.
			meCreateXMLType		= g_st_DBMSCommandInfo[us_roopCnt].m_e_CreateXMLKind;
// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 end.
			return True;
		}
	}
	
	return False;
}
// 2017/03/16 日立 森本郁 [MFP管理技術_デバイス連携]KRQ-XXX CSRA-DBMS連携 End.

//概要：Inquiryユーザデータのチェック
//戻り値：コマンドチェックの結果
//備考：コマンドにはパラメータがある場合と無い場合がある。複数コマンド。
//作成者：宮本
//レビュー：2003/11/26、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成
//			2004/04/05、宮本、DXコマンド対応
//			2007/06/22、佐久間 DXコマンド同報対応(DXコマンドのチェックはisGoodDX()ではなくCheckCommand()で行なう) 
	RDSE_SYNTAX_RESULT
RDSC_Inquiry::CheckSyntax(
		const unsigned char* pCommand,	// [IN]  ホストから要求されたコマンド文字列
		int32_t commandSize,				// [IN]  ホストから要求されたコマンド文字列のサイズ[単位:byte]
		int32_t* pTheNumberOfCommand,		// [OUT] 含まれていたコマンド数を格納するアドレス
		Bool* pbIsIncludedReadOnly,		// [OUT] ReadOnlyコマンドを含んでいたか？を格納するアドレス
// 2017/6/27 MAv5 CSRA接続 start.
		std::string* p_c_CommandSkip	// [OUT] 非対応コマンドを除いたコマンド文字列.
// 2017/6/27 MAv5 CSRA接続 end.
		) const
{
	RDSE_SYNTAX_RESULT	result = RDSE_SYNTAX_RESULT_NO_DELIMITER;		//結果
	Bool	bInLoop = True;
	const unsigned char* p = pCommand;			// 次のユーザデータ文字列アドレス
	int32_t	leftCommandSize = commandSize;		// 残りユーザデータ文字列のサイズ
	const RDSS_CommandInfo*	pCommandInfo = 0;	// コマンド情報
	int32_t	lCheckedCount = 0;					// チェックした文字数(異常時は０)
	short	theNumberOfParam = 0;				// 指定されたパラメータ数
	*pTheNumberOfCommand = 0;					// コマンド数をクリア
	*pbIsIncludedReadOnly = False;				// ReadOnlyコマンドは見つけていない
	
// 2017/02/06 H.Tokuno IT5_4.1 機器構成変更_No.49 CTLログの容量拡張 Start.
	mbExistNeedHDDAccessCommand = False;			// HDD復帰処理が必要がコマンドが見つかったかどうか。
// 2017/02/06 H.Tokuno IT5_4.1 機器構成変更_No.49 CTLログの容量拡張 End.

// 07/06/22 佐久間 DXコマンド同報対応(DXコマンドのチェックはisGoodDX()ではなくCheckCommand()で行なう) 
	while (bInLoop)
	{
		//概要：１コマンドのチェック
		result = 
		CheckCommand(
				p,								// [IN]  コマンド文字列
				leftCommandSize,				// [IN]  コマンド文字列のサイズ
				pCommandInfo,					// [OUT] コマンド情報のアドレスの参照(異常時は０が格納される)
				&lCheckedCount,					// [OUT] チェックした文字数(異常時は０)
												//		 空白が続く場合は チェック済文字数に空白は含まなれない
				&theNumberOfParam);				// [OUT] 指定されたパラメータ数を格納するアドレス
// 2017/6/27 MAv5 CSRA接続 start.
		//********************************************************************
		// Skip有効.
		//********************************************************************
		if( NULL != p_c_CommandSkip ) {
			if ((result == RDSE_SYNTAX_RESULT_OK_SPACE)		// 正常（空白終端を検出）.
				|| (result == 	RDSE_SYNTAX_RESULT_OK_END))	// 正常（ユーザデータ文字列終端を検出）.
			{
				if( 0 < lCheckedCount ) {
					p_c_CommandSkip->append( (const char*)p, (size_t)lCheckedCount );
				}

				leftCommandSize -= lCheckedCount;	// 残りサイズを 更新.
				p += lCheckedCount;					// ポインタを進める.
				(*pTheNumberOfCommand)++;			// コマンド数を インクリメント.
				if (pCommandInfo->mbIsReadOnly)		// ReadOnlyコマンドを見つけたら.
					*pbIsIncludedReadOnly = True;
			}
			else if( RDSE_SYNTAX_RESULT_SYNTAX_ERROR == result ){

				RDSD_DBG_MESSAGE_STR( "Skip ??", leftCommandSize, 0, 0 );

				// ログ.
				if( 3 <= leftCommandSize ) {
					unsigned char uc_log_command[3+1] = {};
					memcpy( uc_log_command, p, 3 );
					RDSD_DBG_MESSAGE_STR( uc_log_command, 0, 0, 0 );
				}

				// 終端の場合.
				if( leftCommandSize <= 3 ) {
					RDSD_DBG_MESSAGE_STR( "Skip OK!! last data", 0, 0, 0 );
					result = RDSE_SYNTAX_RESULT_OK_END;

				// 続きがある場合.
				} else {
					// 次のコマンドの区切り文字までサーチ. 
					p += 3;
					if( ( CodeOfSpace == *p ) || ( CodeOfComma == *p ) ){
						// 継続.
						RDSD_DBG_MESSAGE_STR( "Skip OK!! continue", 0, 0, 0 );
						leftCommandSize -= 3;
						result = RDSE_SYNTAX_RESULT_OK_SPACE;
					} else {
						RDSD_DBG_MESSAGE_STR( "SYNTAX ERROR", 0, 0, 0 );
					}
				}
			} else {
				// NOP.
			}
		//********************************************************************
		// Skip無効.
		//********************************************************************
		} else {

			if ((result == RDSE_SYNTAX_RESULT_OK_SPACE)		// 正常（空白終端を検出）.
				|| (result == 	RDSE_SYNTAX_RESULT_OK_END))	// 正常（ユーザデータ文字列終端を検出）.
			{
				leftCommandSize -= lCheckedCount;	// 残りサイズを 更新.
				p += lCheckedCount;					// ポインタを進める.
				(*pTheNumberOfCommand)++;			// コマンド数を インクリメント.
				if (pCommandInfo->mbIsReadOnly)		// ReadOnlyコマンドを見つけたら.
					*pbIsIncludedReadOnly = True;
			}
		}
// 2017/6/27 MAv5 CSRA接続 end.

		if (result != RDSE_SYNTAX_RESULT_OK_SPACE)	// 空白終端でない場合
		{
			bInLoop = False;
		}
	}//end of while (bInLoop)
	return result;
}


//概要：初期化
//前提：初回の応答のユーザデータを作成前にInit関数を呼び出す。
//戻り値：無し
//備考：連送では、送信確認MSG受信タイミングで、次の送信電文を作成する。よって、コンストラクタの初期化は期待できない。
//作成者：宮本
//レビュー：2003/11/27、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成
//			2004/04/05、宮本、DXコマンド対応
	void
RDSC_Inquiry::Init(
		const unsigned char* pCommand,	// ホストから要求されたコマンド文字列
		int32_t commandSize				// ホストから要求されたコマンド文字列のサイズ[単位:byte]
		)
{
	// 静的解析 CID15052 commandSizeがNULLの場合の処理を追加 Iwata 2011/07/27 Start.
	if (commandSize == 0)
	{
		mStatus = STATUS_NORMAL_END;
	}
	else
	{
		mStatus = STATUS_NEXT_COMMAND;	// 初期状態は、「次のコマンドから処理再開［未終了］」
	}	
// 静的解析 CID15052 commandSizeがNULLの場合の処理を追加 Iwata 2011/07/27 End.
	mpCommandTop = pCommand;
	mpCommandEnd = mpCommandTop + commandSize;
	mbIsHalfOfCommand = False;
	mbIsExistParam = False;
#if !defined(CSRC_INVADIDATE_DX)
	mOffsetOfXmlFile = 0;
#endif	//end of #if !defined(CSRC_INVADIDATE_DX)
	mNextParamValue = 0;

// 2017/08/07 MAv5 エラーチェック機構 start.
	m_c_CommandCheck.clear();
	m_sc_NormalParamCnt = 0;
// 2017/08/07 MAv5 エラーチェック機構 end.
	
	initMemberForZH();		// 2016/05/12 H.Tokuno ZeusSBK 制御構成変更_No.57 カーネルログ取得による市場問題検証対応_Step2.
// 2017/02/06 H.Tokuno IT5_4.1 機器構成変更_No.49 CTLログの容量拡張 Start.
	initMemberForZM();
	// HDDアクセスの必要なコマンドが含まれているかというフラグはここでFalseに初期化.
	mbExistNeedHDDAccessCommand = False;
// 2017/02/06 H.Tokuno IT5_4.1 機器構成変更_No.49 CTLログの容量拡張 End.
}


// 2016/05/12 H.Tokuno ZeusSBK 制御構成変更_No.57 カーネルログ取得による市場問題検証対応_Step2 Start.
//概要：ZHコマンド処理に関連する変数を初期化する.
//戻り値：無し.
//備考：mOffsetOfDmsgFile、mDmsgFd、mDmsgFileSize、mAlreadyReadZHCommandを初期化する.
//作成者：H.Tokuno.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//			2016/05/18、H.Tokuno、新規作成.
	void
RDSC_Inquiry::initMemberForZH(Bool readZHCommand)
{
	mOffsetOfDmsgFile = 0;
	mDmsgFd = 0;
	mDmsgFileSize = 0;
	mAlreadyReadZHCommand = readZHCommand;
}
// 2016/05/12 H.Tokuno ZeusSBK 制御構成変更_No.57 カーネルログ取得による市場問題検証対応_Step2 End.

// 2017/02/06 H.Tokuno IT5_4.1 機器構成変更_No.49 CTLログの容量拡張 Start.
//概要：ZMコマンド処理に関連する変数を初期化する.
//戻り値：無し.
//備考：mlOffsetOfEngLogFile、miEngLogFd、miEngLogFileSize、mbAlreadyReadZMCommandを初期化する.
//作成者：H.Tokuno.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//			2017/02/06、H.Tokuno、新規作成.
	void
RDSC_Inquiry::initMemberForZM(Bool readZMCommand)
{
	mlOffsetOfEngLogFile = 0;
	miEngLogFd = 0;
	miEngLogFileSize = 0;
	mbAlreadyReadZMCommand = readZMCommand;
}
// 2017/02/06 H.Tokuno IT5_4.1 機器構成変更_No.49 CTLログの容量拡張 End.

//概要：残りデータはあるか？
//戻り値：True:残りデータ有、False:残りデータ無
//備考：
//作成者：宮本
//レビュー：2003/11/27、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成
	Bool
RDSC_Inquiry::IsExistLeftData() const
{
	return (char)((mStatus == STATUS_NORMAL_END) ? False : True);	// 2010/07/17 sone Minerva warning 対応 charにキャスト.
}

//概要：コマンドの先頭から１コマンドを処理する
//		リードポインタから、１コマンドを書ける所まで書く。
//前提：バッファには 空白とコマンド文字が 書いてない。
//戻り値：不揮発アクセスの結果
//備考：コマンドにはパラメータがある場合と無い場合がある。
//　　　コマンド文字列の先頭は空白。
//作成者：宮本
//レビュー：2003/11/27、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成
	RDSE_NONVOLATILE_RESULT
RDSC_Inquiry::ProcessCommandAtTop(
		Bool* pbIsAbleToWrite,					// [OUT] 書き込み継続可能？
		unsigned char*& rpUserDataBuffer,		// [OUT] 書き込みバッファ先頭アドレス／関数終了時、呼び出し元のアドレスが進んでいる
// 2017/08/07 MAv5 エラーチェック機構 start.
		Bool*	pb_ResultParamCheck,			// [OUT] パラメータチェック結果.
// 2017/08/07 MAv5 エラーチェック機構 end.
		unsigned char* const pUserDataBufferEnd	// [IN]  書き込みバッファの終端アドレス
		)
{
	Bool	bIsGood = False;
	RDSE_NONVOLATILE_RESULT	result = RDSE_NONVOLATILE_RESULT_SYNTAXERR;		//結果
	
	*pbIsAbleToWrite = True;

	//終端はＯＫ（文法チェック済なので）
	if (mpCommandTop == mpCommandEnd)
	{
		mStatus = STATUS_NORMAL_END;
		return RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;
	}

	//空白を期待する。
	if ((*mpCommandTop != CodeOfSpace) || ((mpCommandEnd - mpCommandTop) < (2+1)))
	{
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR1" );
		return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
	}
	
	//コマンドを期待する
	const RDSS_CommandInfo*	pCommandInfo = mrCommandMatrix.GetCommandInfo(&mpCommandTop[1], &bIsGood);
	//不正コマンドなら、終了
	if (!bIsGood)
	{
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR2" );
		return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
	}
	
	//2008/03/12 許、Donauセキュリティ強化モードで遠隔データ書き換え/取得の抑制対応
	if(true == APIC_SystemSetting::newInstance()->getSecurityLevelUpOn()){	
		/* ※ Donauセキュリティ強化モードで、IISWに関連する情報のread要求はエラーとする。※ */
		switch( mpCommandTop[1] ){
		case 'T':				// 
			if(( 'I' == mpCommandTop[2] )||( 'H' == mpCommandTop[2] )){
// 2015/11/06 tatsumi ZeusS_ZX0 制御構成変更_No.76-2_ネットワークエラーカウンタ追加 START.
				RDS_incNetworkErrorCount( RDSE_NETWORK_ERROR_SECURITY_LEVELUP_IISW_RESTRICTED );
// 2015/11/06 tatsumi ZeusS_ZX0 制御構成変更_No.76-2_ネットワークエラーカウンタ追加 END.
				return RDSE_SYNTAX_RESULT_HIGHER_SECURITY_MODE;					//  
			}	
			break;
			
		default :
			break;
		}
	}

	//コマンドが書けるか確認。
	//・次のコマンドが書けない可能性がある。
	//　－＞残り書込サイズが（最大データ長＋６）未満
	//　（空白＋コマンド２文字＋パラメータ２文字＋最大データ長＋カンマ）
	if ((pUserDataBufferEnd - rpUserDataBuffer) < (pCommandInfo->mDataSize + 6))
	{
		*pbIsAbleToWrite = False;
		mStatus = STATUS_NEXT_COMMAND;
		return RDSE_NONVOLATILE_RESULT_OK_NEXT;
	}
	//コマンド文字を保存
	memcpy(mCommand, mpCommandTop+1, 2);
	mpCommandTop += 3;			// 読み込みアドレスを増やす
	//書き込みバッファに コマンドを書く
	*rpUserDataBuffer++ = ' ';
	memcpy(rpUserDataBuffer, mCommand, 2);
	rpUserDataBuffer += 2;

	if ((*mpCommandTop == CodeOfSpace) || (mpCommandTop == mpCommandEnd))	//空白 または 終端
	{
		//パラメータ無しのコマンドだった（空白終端を検出、または、ユーザデータ文字列終端を検出）

		mbIsExistParam = False;		// コマンドにパラメータは存在したか？
		mNextParamValue = 0;		// 次のパラメータ値（コマンドにパラメータが無い時のみ有効）
		mStatus = STATUS_HALFCOMMAND_NOPARAM;

		//概要：コマンドの途中から１コマンドを処理する（パラメータ指定無）
// 2017/08/07 MAv5 エラーチェック機構 start.
//		result = ProcessCommandAtHalfWithoutParam(pbIsAbleToWrite, rpUserDataBuffer, pUserDataBufferEnd);
		result = ProcessCommandAtHalfWithoutParam(pbIsAbleToWrite, rpUserDataBuffer, pb_ResultParamCheck, pUserDataBufferEnd);
// 2017/08/07 MAv5 エラーチェック機構 end.
	}
	else	//パラメータ
	{
		mbIsExistParam = True;		// コマンドにパラメータは存在したか？
		mStatus = STATUS_HALFCOMMAND_PARAM;

		//概要：コマンドの途中から１コマンドを処理する（パラメータ指定有）
		result = 
		ProcessCommandAtHalfWithParam(pbIsAbleToWrite, rpUserDataBuffer, pUserDataBufferEnd);
	}
	
	return result;
}

//概要：コマンドの途中から１コマンドを処理する（パラメータ指定有）
//		メンバ変数のコマンドと、リードポインタから、１コマンドを書ける所まで書く。
//前提：バッファには 空白とコマンド文字が 書いてある。
//戻り値：不揮発アクセスの結果
//備考：問い合わせデータの中に予約語が含まれる場合、変換する。
//作成者：宮本
//レビュー：2003/11/27、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成
	RDSE_NONVOLATILE_RESULT
RDSC_Inquiry::ProcessCommandAtHalfWithParam(
		Bool* pbIsAbleToWrite,					// [OUT] 書き込み継続可能？
		unsigned char*& rpUserDataBuffer,		// [OUT] 書き込みバッファ先頭アドレス／関数終了時、呼び出し元のアドレスが進んでいる
		unsigned char* const pUserDataBufferEnd	// [IN]  書き込みバッファの終端アドレス
		)
{
	//パラメータがあるコマンド
	Bool	bInLoop = True;
	RDSE_NONVOLATILE_RESULT	result = RDSE_NONVOLATILE_RESULT_SYNTAXERR;		//結果
	Bool	bIsGood = False;
	
// 2017/08/07 MAv5 エラーチェック機構 start.
	unsigned char uc_log_command[2+1] = {};
	memcpy( uc_log_command, mCommand, sizeof(mCommand) );
	RDSD_DBG_MESSAGE_CMD_STR( uc_log_command, 0, 0, 0 );
// 2017/08/07 MAv5 エラーチェック機構 end.
// 2018/11/01 HCLT-Jerin 【IT6_1.0】【Sparrow】 AR1058458 Modified Start.
	if(strncmp((char *)mCommand, "Z1", 2) == 0)		// 2018/11/11FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
	{
		TYPE_PowerSaveStatus	e_PowerStatus		= APIC_Power::newInstance()->getPowerSaveStatus();	// マシン電源状態を取得.
		JOBC_ActiveJobListIf*	pc_ActiveJobList	= JOBC_ActiveJobListIf::newInstance();
		APIC_PrinterWarning*	pc_PrintWarn		= APIC_PrinterWarning::newInstance();
		if ( ( e_PowerStatus == TYPE_PowerSaveStatusIdle  ) &&							//電源状態が待機状態.
			 ( True  == pc_ActiveJobList->isJobIdle()     ) &&							//JOBが存在しない.
			 ( False == pc_PrintWarn->isPrinterDoorOpen( TYPE_PrinterDoorFront ) ) )	//前ドアオープン状態でない.
		{
			// エンジンデータ取得実行.
			EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)"エンジンデータ取得実行。" , __LINE__ );
			comENG.getCcEngDataRequest().requestPMCounter();	// エンジンデータ取得要求 
			comENG.getCcEngDataRequest().getPMCounter();		// ＰＭカウンタ要求コマンド送信 
			comENG.getCcEngDataRequest().endPMCounter();		// ＰＭカウンタ要求完了処理
		}else{
			EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)"エンジンデータ取得要求発行が許可されませんでした。" , __LINE__ );
		}
	}
// 2018/11/01 HCLT-Jerin 【IT6_1.0】【Sparrow】 AR1058458 Modified End.
	//コマンドを期待する
	const RDSS_CommandInfo*	pCommandInfo = mrCommandMatrix.GetCommandInfo(mCommand, &bIsGood);
	//不正コマンドなら、終了
	if (!bIsGood)
	{
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR3" );
		return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
	}
	
	mStatus = STATUS_HALFCOMMAND_PARAM;

	while (bInLoop)
	{
		//パラメータが書けるか確認
		//・次のパラメータ＋データが書けない可能性がある。
		//　－＞残り書込サイズが（最大データ長＋３）未満
		//　（パラメータ２文字＋最大データ長＋カンマ）
		if ((pUserDataBufferEnd - rpUserDataBuffer) < (pCommandInfo->mDataSize + 3))
		{
			*pbIsAbleToWrite = False;
			return RDSE_NONVOLATILE_RESULT_OK_NEXT;
		}

		//パラメータを期待
		//パラメータは２文字で、'0'～'9'、'A'～'F'
		int	nValue = RDSC_CommandMatrix::Hex2Toi(mpCommandTop, &bIsGood);
		//パラメータ値が不正
		if (!bIsGood)
		{
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR4" );
			return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
		}
		//最大値を越えている
		if (nValue > pCommandInfo->mParamMax)
		{
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR5" );
			return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
		}
		
		unsigned char command[4];		// コマンド文字列＋パラメータ文字列
		memcpy(command, mCommand, 2);
		memcpy(&command[2], mpCommandTop, 2);
		short	writtenSize = 0; 	// バッファに書き込んだサイズ[単位:byte]
		
		//書き込みバッファに パラメータを書く
		//無効なパラメータは、パラメータ番号自体送らない仕様とするため、
		//ここでの書き込みは行わず、ReadCommand関数にて書き込むこととする。
//		memcpy(rpUserDataBuffer, mpCommandTop, 2);
//		rpUserDataBuffer += 2;

		RDSE_CMD_RESULT	ret =
		//ReadCommand呼び出し
		tskRdsEvent.ReadCommand(
			command,					// コマンド文字列＋パラメータ文字列（パラメータ 必ず書き込む）
			rpUserDataBuffer,			// ユーザデータバッファの先頭アドレス
			(short)(pCommandInfo->mDataSize + 2),	// データサイズ[単位:byte]（可変長の場合は 最大値）	// 2010/07/17 sone Minerva warning 対応 shortにキャスト.
			&writtenSize); 				// バッファに書き込んだサイズ[単位:byte]
		
		switch (ret)
		{
			case RDSE_CMD_RESULT_Normal:		// 正常
			case RDSE_CMD_RESULT_ErrNotRead:	// エラーで読めない（メカコン系データ）
			case RDSE_CMD_RESULT_ParamNotApply:
				//予約文字を 規定文字に変換
				RDSC_CommandMatrix::ConvertReservedToRegulated(rpUserDataBuffer, writtenSize);
				//書き込みアドレスを更新
				rpUserDataBuffer += writtenSize;
				break;
// 2010/07/29 yito UV-00886 Exp/Imp start.
			case RDSE_CMD_RESULT_NormalWriteData:		// WRで取得したデータをそのまま返却.
				// 予約文字の変換は行わない。
				rpUserDataBuffer += writtenSize;
				break;
// 2010/07/29 yito UV-00886 Exp/Imp end  .
			default:
//			case RDSE_CMD_RESULT_ErrNoData:		// データが無い（コマンド不正、パラメータ不正）
				DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_INVALID_ACCESS1" );
				return RDSE_NONVOLATILE_RESULT_INVALID_ACCESS;
			case RDSE_CMD_RESULT_ErrReadOnly:	// ReadOnlyに対する書き込みを行おうとした
				DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_WRITE_ERR_TO_RONLY1" );
				return RDSE_NONVOLATILE_RESULT_WRITE_ERR_TO_RONLY;
			// 2013.12.13 H.Wada [ZeusCD][UV-01385] WP61,81,88,308.
// 2018/05/03 HCLT-Vinay 【IT6_1.0 Eagle】制御構成変更_No.97：【ストレージ構成変更】 Mod Start.
			case RDSE_CMD_RESULT_ErrNoSSD:		// SSD非装着
				EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)"Err is RDSE_CMD_RESULT_ErrNoSSD" , __LINE__ );
				return RDSE_NONVOLATILE_RESULT_NO_SSD;
// 2018/05/03 HCLT-Vinay 【IT6_1.0 Eagle】制御構成変更_No.97：【ストレージ構成変更】 Mod End.
			// 2013.12.13 H.Wada [ZeusCD][UV-01385] WP61,81,88,308.
		}

		//読み込みポインタを進める
		mpCommandTop += 2;

		//' 'または','または"終端"を期待
		if (*mpCommandTop == CodeOfSpace)	//空白
		{
			//正常
			mStatus = STATUS_NEXT_COMMAND;
			result = RDSE_NONVOLATILE_RESULT_OK_NEXT;
			bInLoop = False;
		}
		else if (*mpCommandTop == CodeOfComma)	//カンマ
		{
			mpCommandTop++;

			if (RDSE_CMD_RESULT_ParamNotApply != ret)
			{
				//書き込みバッファに カンマを書く
				*rpUserDataBuffer++ = CodeOfComma;
			}
			
			continue;
		}
		else if (mpCommandTop == mpCommandEnd)	//終端
		{
			//正常（ユーザデータ文字列終端を検出）
			mStatus = STATUS_NORMAL_END;
			result = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;
			bInLoop = False;
			
			//最後のパラメータまで処理した時、最後にカンマが入っていればそれを消しておく.
			if (*(rpUserDataBuffer -1) == CodeOfComma)
			{
				*rpUserDataBuffer-- = '\0';
			}
			
		}
		else if (mpCommandTop > mpCommandEnd)
		{
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR6" );
			result = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			bInLoop = False;
		}
		else	//予期しない文字
		{
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR7" );
			result = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			bInLoop = False;
		}
	}//end of while (bInLoop)
	return result;
}

//概要：コマンドの途中から１コマンドを処理する（パラメータ指定無）
//		メンバ変数のコマンドと、次のパラメータ値から、１コマンドを書ける所まで書く。
//前提：バッファには 空白とコマンド文字が 書いてある。
//戻り値：不揮発アクセスの結果
//備考：問い合わせデータの中に予約語が含まれる場合、変換する。
//作成者：宮本
//レビュー：2003/11/27、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成
	RDSE_NONVOLATILE_RESULT
RDSC_Inquiry::ProcessCommandAtHalfWithoutParam(
		Bool* pbIsAbleToWrite,					// [OUT] 書き込み継続可能？
		unsigned char*& rpUserDataBuffer,		// [OUT] 書き込みバッファ先頭アドレス／関数終了時、呼び出し元のアドレスが進んでいる
// 2017/08/07 MAv5 エラーチェック機構 start.
		Bool*	pb_ResultParamCheck,			// [OUT] パラメータチェック結果.
// 2017/08/07 MAv5 エラーチェック機構 end.
		unsigned char* const pUserDataBufferEnd	// [IN]  書き込みバッファの終端アドレス
		)
{
	//パラメータがないコマンド
	Bool	bInLoop = True;
	Bool	xmlflag;							// add by tomiyasu 2007/04/16
	RDSE_NONVOLATILE_RESULT	result = RDSE_NONVOLATILE_RESULT_SYNTAXERR;		//結果
	Bool	bIsGood = False;

// 2017/08/07 MAv5 エラーチェック機構 start.
	unsigned char uc_command[2+1] = {};
	memcpy( uc_command, mCommand, sizeof(mCommand) );

// 2018/11/01 HCLT-Jerin 【IT6_1.0】【Sparrow】 AR1058458 Modified Start.
	if(strncmp((char *)mCommand, "Z1", 2) == 0)	// 2018/11/11FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
	{
		TYPE_PowerSaveStatus	e_PowerStatus		= APIC_Power::newInstance()->getPowerSaveStatus();	// マシン電源状態を取得.
		JOBC_ActiveJobListIf*	pc_ActiveJobList	= JOBC_ActiveJobListIf::newInstance();
		APIC_PrinterWarning*	pc_PrintWarn		= APIC_PrinterWarning::newInstance();
		if ( ( e_PowerStatus == TYPE_PowerSaveStatusIdle  ) &&							//電源状態が待機状態.
			 ( True  == pc_ActiveJobList->isJobIdle()     ) &&							//JOBが存在しない.
			 ( False == pc_PrintWarn->isPrinterDoorOpen( TYPE_PrinterDoorFront ) ) )	//前ドアオープン状態でない.
		{
			// エンジンデータ取得実行.
			EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)"エンジンデータ取得実行。" , __LINE__ );
			comENG.getCcEngDataRequest().requestPMCounter();	// エンジンデータ取得要求 
			comENG.getCcEngDataRequest().getPMCounter();		// ＰＭカウンタ要求コマンド送信 
			comENG.getCcEngDataRequest().endPMCounter();		// ＰＭカウンタ要求完了処理
		}else{
			EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)"エンジンデータ取得要求発行が許可されませんでした。" , __LINE__ );
		}
	}
// 2018/11/01 HCLT-Jerin 【IT6_1.0】【Sparrow】 AR1058458 Modified End.

	// 初期値.
	// 1コマンドでも何回かに分けて処置する可能性があるため継続してカウントする.
	if( 0 != m_c_CommandCheck.compare( (const char*)uc_command ) ) {
		m_c_CommandCheck.assign( (const char*)uc_command );
		m_sc_NormalParamCnt = 0;
	}

	if( NULL != pb_ResultParamCheck ) {
		*pb_ResultParamCheck = True;
	}

	RDSD_DBG_MESSAGE_CMD_STR( uc_command, 0, 0, 0 );
	RDSD_DBG_MESSAGE_CMD_STR( m_c_CommandCheck.c_str(), m_sc_NormalParamCnt, 0, 0 );
// 2017/08/07 MAv5 エラーチェック機構 end.
	
	//コマンドを期待する
	const RDSS_CommandInfo*	pCommandInfo = mrCommandMatrix.GetCommandInfo(mCommand, &bIsGood);
	//不正コマンドなら、終了
	if (!bIsGood)
	{
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR8" );
		return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
	}

	mStatus = STATUS_HALFCOMMAND_NOPARAM;

	while (bInLoop)
	{
		//パラメータが書けるか確認
		//・次のパラメータ＋データが書けない可能性がある。
		//　－＞残り書込サイズが（最大データ長＋３）未満
		//　（パラメータ２文字＋最大データ長＋カンマ）
		if ((pUserDataBufferEnd - rpUserDataBuffer) < (pCommandInfo->mDataSize + 3))
		{
			RDSD_DBG_MESSAGE_STR( "Buffer Over ", m_sc_NormalParamCnt, 0, 0 );
			*pbIsAbleToWrite = False;
			return RDSE_NONVOLATILE_RESULT_OK_NEXT;
		}

		char command[5];		// コマンド文字列＋パラメータ文字列
		memcpy(command, mCommand, 2);

//***************************  add start by tomiyasu 2007/04/16  **********************************//

		// 2014.05.19 Wada 制御構成変更_No.175_Ecoモード s.
		// ※Zeus4thの手動マージの際V7コマンドの条件が追記されます.
		//   V6を消してしまわないように注意.
// 2014/01/07 ktakahashi [ZeusCD][RQ-143] S.
		if( ( *mCommand == 'V' ) && ( ( *(mCommand+1)== '4' ) || ( *(mCommand+1)== '6' ) || ( *(mCommand+1)== '7' ) ) )
// 2014/01/07 ktakahashi [ZeusCD][RQ-143] E.
		// 2014.05.19 Wada 制御構成変更_No.175_Ecoモード e.
			xmlflag = True;
		else
			xmlflag = False;

//***************************  add end by tomiyasu 2007/04/16  ************************************//


		//パラメータを作成
		sprintf(&command[2], "%02X", 0xff & mNextParamValue++);
		short	writtenSize = 0; 	// バッファに書き込んだサイズ[単位:byte]
		
		if( xmlflag == False || ( xmlflag == True && mNextParamValue == 1 ))	//  add by tomiyasu 2007/04/16
		{
			//書き込みバッファに パラメータを書く
			//無効なパラメータは、パラメータ番号自体送らない仕様とするため、
			//ここでの書き込みは行わず、ReadCommand関数にて書き込むこととする。
//			memcpy(rpUserDataBuffer, &command[2], 2);
//			rpUserDataBuffer += 2;
		}
		
		RDSE_CMD_RESULT	ret =
		//ReadCommand呼び出し
		tskRdsEvent.ReadCommand(
			reinterpret_cast<unsigned char *>(command),	// コマンド文字列＋パラメータ文字列（パラメータ 必ず書き込む）
			rpUserDataBuffer,			// ユーザデータバッファの先頭アドレス
			(short)(pCommandInfo->mDataSize + 2),	// データサイズ[単位:byte]（可変長の場合は 最大値）	// 2010/07/17 sone Minerva warning 対応 shortにキャスト.
			&writtenSize); 				// バッファに書き込んだサイズ[単位:byte]

		switch (ret)
		{
			case RDSE_CMD_RESULT_Normal:		// 正常
// 2017/08/07 MAv5 エラーチェック機構 start.
				m_sc_NormalParamCnt++;
			//	break;           // 2018/02/16 【IT6_1.0】【AR1039363】 HCLT_Jahnavi Modified.
// 2017/08/07 MAv5 エラーチェック機構 end.
			case RDSE_CMD_RESULT_ErrNotRead:	// エラーで読めない（メカコン系データ）
			case RDSE_CMD_RESULT_ParamNotApply:
				//予約文字を 規定文字に変換
				RDSC_CommandMatrix::ConvertReservedToRegulated(rpUserDataBuffer, writtenSize);
				//書き込みアドレスを更新
				rpUserDataBuffer += writtenSize;
				break;
			case RDSE_CMD_RESULT_ErrReadOnly:	// ReadOnlyに対する書き込みを行おうとした
				DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_WRITE_ERR_TO_RONLY2" );
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"ReadOnlyエラー");
				return RDSE_NONVOLATILE_RESULT_WRITE_ERR_TO_RONLY;
			// 2013.12.13 H.Wada [ZeusCD][UV-01385] WP61,81,88,308.
// 2018/05/03 HCLT-Vinay 【IT6_1.0 Eagle】制御構成変更_No.97：【ストレージ構成変更】 Mod Start.
			case RDSE_CMD_RESULT_ErrNoSSD:		//SSD非装着
				EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)"Err is RDSE_CMD_RESULT_ErrNoSSD" , __LINE__ );
				return RDSE_NONVOLATILE_RESULT_NO_SSD;
// 2018/05/03 HCLT-Vinay 【IT6_1.0 Eagle】制御構成変更_No.97：【ストレージ構成変更】 Mod End.
			// 2013.12.13 H.Wada [ZeusCD][UV-01385] WP61,81,88,308.
			default:
//			case RDSE_CMD_RESULT_ErrNoData:		// データが無い（コマンド不正、パラメータ不正）
				DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_INVALID_ACCESS2" );
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"No data");
				return RDSE_NONVOLATILE_RESULT_INVALID_ACCESS;
		}

		if (mNextParamValue > pCommandInfo->mParamMax)
		{
			//最後のパラメータまで処理した
			mStatus = STATUS_NEXT_COMMAND;
			bInLoop = False;
			
			//最後のパラメータまで処理した時、最後にカンマが入っていればそれを消しておく.
			if (*(rpUserDataBuffer -1) == CodeOfComma)
			{
				*rpUserDataBuffer-- = '\0';
			}
			
			//' 'または"終端"を期待
			if (*mpCommandTop == CodeOfSpace)	//空白
			{
				result = RDSE_NONVOLATILE_RESULT_OK_NEXT;
			}
			else if (mpCommandTop == mpCommandEnd)	//終端
			{
				result = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;
			}
			else								//文法エラー
			{
				DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR9" );
				result = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			}
		}
		else	//カンマ
		{
			if (RDSE_CMD_RESULT_ParamNotApply != ret)
			{
				//書き込みバッファに カンマを書く
				if( xmlflag == False )
				{
					*rpUserDataBuffer++ = CodeOfComma;
				}
			}
		}
	}//end of while (bInLoop)
	
// 2017/08/07 MAv5 エラーチェック機構 start.
	if( m_sc_NormalParamCnt != pCommandInfo->mEnableParamCnt ) {

		// パラメータチェックが有効な場合のみ.
		if( RDSD_PARAMCHECK_DISABLE != pCommandInfo->mEnableParamCnt ) {
			
			RDSD_DBG_MESSAGE_STR( "Param Error!! m_sc_NormalParamCnt,mEnableParamCnt", m_sc_NormalParamCnt, pCommandInfo->mEnableParamCnt, 0 );
			
			if( NULL != pb_ResultParamCheck ) {
				*pb_ResultParamCheck = False;
			}
		}
	}
// 2017/08/07 MAv5 エラーチェック機構 end.
	
	return result;
}

// 2018/12/07 FPT-DongND4 IT6_2.0 【Sparrow】 【AR1070670】 added Start.
void putLine(
	uchar*						pc_Buffer,
	int64_t*					pl_BufferWriteSize,
	char* 						psc_param1,
	char* 						psc_param2
)
{
	if (NULL == pc_Buffer) {
		RDSD_DBG_MESSAGE_STR("Parameter == Null", 0, 0, 0);
		return;
	}

	if (NULL == pl_BufferWriteSize) {
		RDSD_DBG_MESSAGE_STR("Parameter == Null", 0, 0, 0);
		return;
	}

	// psc_param1は必須.
	if (NULL == psc_param1) {
		RDSD_DBG_MESSAGE_STR("Parameter == Null", 0, 0, 0);
		return;
	}

	string c_StrTemp("");

	c_StrTemp.append((const char*)psc_param1);

	// psc_param2は任意.
	if (NULL != psc_param2) {
		c_StrTemp.append((const char*)psc_param2);
	}

	// 書き込み済みサイズをオフセットにバッファコピー.
	strncpy((char*)(pc_Buffer + *pl_BufferWriteSize), c_StrTemp.c_str(), c_StrTemp.length());

	// 書き込み済みサイズを加算する.
	// ヌル文字は含めない.
	*pl_BufferWriteSize += c_StrTemp.length();
}

void encodeBase64(
	const uchar*				pc_BufferInput, 			// [IN]     エンコード前データ.
	const int64_t				l_BufferInputSize, 		// [IN]     エンコード前データサイズ.
	uchar*						pc_BufferOutput, 		// [OUT]    エンコード後データ       ※エンコード後は4/3倍になるため注意.
	int64_t*					pl_BufferOutputSize		// [OUT]    エンコード後データサイズ.
)
{

	if (NULL == pc_BufferInput) {
		return;
	}

	if (NULL == pc_BufferOutput) {
		return;
	}

	if (NULL == pl_BufferOutputSize) {
		return;
	}

	int64_t	l_TmpSize = l_BufferInputSize;
	uchar*	puc_Tmpbuf = (uchar*)pc_BufferInput;
	int64_t	l_BufferEncodeWriteSize = 0;

	convBase64 c_convBase64;

	while (0 < l_TmpSize) {

		char sc_Temp[RDSD_CmdDataCreaterEncodeMax * RDSD_CmdDataCreaterEncodeScale + 1] = {};
		int32_t i_EncodeSize = (l_TmpSize > RDSD_CmdDataCreaterEncodeMax) ? RDSD_CmdDataCreaterEncodeMax : l_TmpSize;
		c_convBase64.encode((char*)puc_Tmpbuf, sc_Temp, i_EncodeSize);
		puc_Tmpbuf += i_EncodeSize;
		l_TmpSize -= i_EncodeSize;

		// エンコード用にバッファに詰める.
		putLine(pc_BufferOutput, &l_BufferEncodeWriteSize, sc_Temp, (char*)"");
	}

	*pl_BufferOutputSize = l_BufferEncodeWriteSize;

	RDSD_DBG_MESSAGE_CMD_STR("encodeBase64 Size=", l_BufferInputSize, l_BufferEncodeWriteSize, 0);
}

// 2019/01/05 FPT-DongND4 IT6_2.0 【Sparrow】 【AR1070670】 added Start.
Bool isExistAnalysisLogFile()
{
	for (uint32_t mgmtTable = 1; mgmtTable <= 10; mgmtTable++)
	{
		char sc_AnalysisResultFileName[255] = { };
		if (getAnalysisResultFileName(mgmtTable, sc_AnalysisResultFileName))
		{
			printf("isExistAnalysisLogFile() is True\n");
			return True;
		}
	}

	printf("isExistAnalysisLogFile() is False\n");
	return False;
}
// 2019/01/05 FPT-DongND4 IT6_2.0 【Sparrow】 【AR1070670】 added End.
// 2018/12/07 FPT-DongND4 IT6_2.0 【Sparrow】 【AR1070670】 added End.
// 2018/07/03 HCLT-Vinay 【IT6_1.0】【制御構成変更_No.110】 Start.
int CreateZpDataXML( char * xmlfileName )
{		
	const char tempXMLHeader[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";
			
	char varDebugLogTimeStamp[67];
	char DbglogFileName[255];
	char tempLogData[250];
	
	char varDeviceId[31];
	char tempDeviceId[31];
	
	string temp;

	size_t readResult = 0;
	size_t writeResult = 0;	
	
    FILE *pFileDest = NULL;
	// 2019/01/05 FPT-DongND4 IT6_2.0 【Sparrow】 【AR1070670】 Modification Start.
	if( !isExistAnalysisLogFile() && !getHWCheckResultExist()) 				//2020/06/23 FPT-TinhNT16 【IT6_2.0_MR3】 【制御構成変更_No.38】.
	{
		if ( FILD_ErrNotExist != FILF_CheckFileExist( xmlfileName ) )
		{
			printf("isExistAnalysisLogFile() is false and DataFileZp.xml\n");

			return 0;
		}
	}
	// 2019/01/05 FPT-DongND4 IT6_2.0 【Sparrow】 【AR1070670】 Modification End.

	remove( xmlfileName );
	pFileDest = fopen(xmlfileName, "wb");
	
	if ( pFileDest == NULL )
	{
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "CreateZpDataXML Destination File Open Error1" );

		return -1;
	}
	
	// writing device ID.
	memset(varDeviceId,'\0',sizeof(varDeviceId));
	memset(tempDeviceId,'\0',sizeof(tempDeviceId));
	
	if( pFileDest != NULL){
		// 2018/12/07 FPT-DongND4 IT6_2.0 【Sparrow】 【AR1070670】 Modification Start.
		//writeResult = fwrite (tempXMLHeader , 1, sizeof(tempXMLHeader), pFileDest);
		writeResult = fwrite (tempXMLHeader , 1, sizeof(tempXMLHeader) -1, pFileDest);
		// 2018/12/07 FPT-DongND4 IT6_2.0 【Sparrow】 【AR1070670】 Modification End.
	}else{
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "CreateZpDataXML Destination File Open Error2" );
		return -1;
	}
	
	APIC_RdsSetting* pc_RdsSetting	= APIC_RdsSetting::newInstance();
	pc_RdsSetting->lock();
	pc_RdsSetting->getMachineID( (uchar *)varDeviceId );
	pc_RdsSetting->unlock();

	snprintf(tempDeviceId, sizeof(tempDeviceId), "<device id=\"%s\">",varDeviceId);		/* 2018/11/01 Eagle 静的解析対応: ID1000935,1000940 snprintf関数に変換 */
	if( pFileDest != NULL){
		// 2018/12/07 FPT-DongND4 IT6_2.0 【Sparrow】 【AR1070670】 Modification Start.
		//writeResult = fwrite (tempDeviceId , 1, sizeof(tempDeviceId), pFileDest);
		writeResult = fwrite (tempDeviceId , 1, strlen(tempDeviceId), pFileDest);
		// 2018/12/07 FPT-DongND4 IT6_2.0 【Sparrow】 【AR1070670】 Modification End.
	}else{
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "CreateZpDataXML Destination File Write Error1" );
		return -1;
	}
	
	printf("CreateZpDataXML ...\n");
	string DebugLogOpen = "<debugLog>";
	writeResult = fwrite (DebugLogOpen.c_str() , 1, DebugLogOpen.size(), pFileDest);
	
	//Bool flag = False;
	char sc_AnalysisResultFileName[255];
	memset(sc_AnalysisResultFileName,'\0',sizeof(sc_AnalysisResultFileName));
	Bool isLogExist = False;
	uint32_t cntFileLog = 0;			//2020/06/23 FPT-TinhNT16 【IT6_2.0_MR3】 【制御構成変更_No.38】.
	
//2020/07/22 FPT-TinhNT16 【制御構成変更_No.38】 【OP_BTS-25716】 【OP_BTS-25709】 Start.
	APIC_MachineSetting* pc_MachineSetting = APIC_MachineSetting::newInstance();
	pc_MachineSetting->lock();
	TYPE_SystemType e_SystemType = pc_MachineSetting->getSystemType();
	pc_MachineSetting->unlock();
//2020/07/22 FPT-TinhNT16 【制御構成変更_No.38】 【OP_BTS-25716】 【OP_BTS-25709】 End.

	for( uint32_t mgmtTable = 1 ; mgmtTable <= 10 ; mgmtTable++)
	{	
		memset(tempLogData,'\0',sizeof(tempLogData));
		
		memset(varDebugLogTimeStamp,'\0',sizeof(varDebugLogTimeStamp));
		memset(DbglogFileName,'\0',sizeof(DbglogFileName));
		
		isLogExist = getAnalysisResultFileName(mgmtTable,sc_AnalysisResultFileName);
		
		if( isLogExist == False )
		{
			
		} else {
			cntFileLog = mgmtTable;			//2020/07/22 FPT-TinhNT16 【制御構成変更_No.38】 【OP_BTS-25716】 【OP_BTS-25709】 Add.
			string BufferWithLog;
			
			string filePath;
			filePath.append(sc_AnalysisResultFileName);
			
			//string fileName = filePath.substr( filePath.size() - 66 , 66);
			string fileName = filePath.substr(filePath.find_last_of("/")+1);
			
			string timeStamp = fileName.substr(14,15);
			
			sprintf(tempLogData, "<data id=\"%d\" ",mgmtTable);
			BufferWithLog.append(tempLogData);
			//sprintf(tempLogData, "timestamp= \"%s",timeStamp);
			//2020/07/22 FPT-TinhNT16 【制御構成変更_No.38】 【OP_BTS-25716】 【OP_BTS-25709】 Start.
			switch( e_SystemType )
			{
				case TYPE_SystemType_JP:	// Japan.
					// Format: YYMMDDhhmmss
					snprintf(tempLogData, sizeof(tempLogData), "timestamp=\"20%s-%s-%sT%s:%s:%s\" ",timeStamp.substr(0,2).c_str(),timeStamp.substr(2,2).c_str(),timeStamp.substr(4,2).c_str(),timeStamp.substr(6,2).c_str(),timeStamp.substr(8,2).c_str(),timeStamp.substr(10,2).c_str());
					break;
				case TYPE_SystemType_US:	// US.
					// Format: MMDDYYhhmmss
					snprintf(tempLogData, sizeof(tempLogData), "timestamp=\"20%s-%s-%sT%s:%s:%s\" ",timeStamp.substr(4,2).c_str(),timeStamp.substr(0,2).c_str(),timeStamp.substr(2,2).c_str(),timeStamp.substr(6,2).c_str(),timeStamp.substr(8,2).c_str(),timeStamp.substr(10,2).c_str());
					break;
				case TYPE_SystemType_EU:	// EU.
					// Format: DDMMYYhhmmss
					snprintf(tempLogData, sizeof(tempLogData), "timestamp=\"20%s-%s-%sT%s:%s:%s\" ",timeStamp.substr(4,2).c_str(),timeStamp.substr(2,2).c_str(),timeStamp.substr(0,2).c_str(),timeStamp.substr(6,2).c_str(),timeStamp.substr(8,2).c_str(),timeStamp.substr(10,2).c_str());
					break;
				default:
					snprintf(tempLogData, sizeof(tempLogData), "timestamp=\"20%s-%s-%sT%s:%s:%s\" ",timeStamp.substr(0,2).c_str(),timeStamp.substr(2,2).c_str(),timeStamp.substr(4,2).c_str(),timeStamp.substr(6,2).c_str(),timeStamp.substr(8,2).c_str(),timeStamp.substr(10,2).c_str());
					break;
			}
			//2020/07/22 FPT-TinhNT16 【制御構成変更_No.38】 【OP_BTS-25716】 【OP_BTS-25709】 End.
			BufferWithLog.append(tempLogData);
			//BufferWithLog.append("\"");
			
			snprintf(tempLogData, sizeof(tempLogData), "filename=\"%s\">",fileName.c_str());	/* 2018/11/01 Eagle 静的解析対応: ID1000941 snprintf関数に変換 */
			BufferWithLog.append(tempLogData);
			
			string BufferWithLogClose = "</data>";
			
			FILE *pFileSrc = NULL;
			char *buffer = NULL;
			long lSize;

			pFileSrc = fopen(sc_AnalysisResultFileName, "rb");
			if(pFileSrc != NULL)
			{
	            fseek (pFileSrc , 0 , SEEK_END);
	            lSize = ftell (pFileSrc);
	            rewind (pFileSrc);
									
				buffer = (char*) malloc (sizeof(char)*lSize);
				if( buffer != NULL ){	/* 2018/11/01 Eagle 静的解析対応: ID1000938,1000939 NULLチェック開始位置変更 */
					// 2018/12/07 FPT-DongND4 IT6_2.0 【Sparrow】 【AR1070670】 Modification Start.
					//memset(buffer,'\0',sizeof(buffer));
					memset(buffer,'\0', sizeof(char)*lSize );
					// 2018/12/07 FPT-DongND4 IT6_2.0 【Sparrow】 【AR1070670】 Modification End.

					readResult = fread (buffer,1,lSize,pFileSrc);

				
					if ( readResult != (size_t)lSize ){			// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
						DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "CreateZpDataXML Source File Read Error" );
					}else{
						
					}
				
					if ( pFileDest != NULL ){
						writeResult = fwrite (BufferWithLog.c_str() , 1, BufferWithLog.size(), pFileDest);

						// 2018/12/07 FPT-DongND4 IT6_2.0 【Sparrow】 【AR1070670】 Modification Start.
						//writeResult = fwrite (buffer , 1, lSize, pFileDest);
						char *convBase64Buffer = (char*)malloc(sizeof(char) * lSize * 2);
						if( convBase64Buffer != NULL )
						{
							memset(convBase64Buffer, '\0', sizeof(char) * lSize * 2);
							int64_t pl_BufferOutputSize = 0;
							encodeBase64((uchar*)buffer, sizeof(char)*lSize, (uchar*)convBase64Buffer, &pl_BufferOutputSize);			// 2019/01/09 FPT-DucDH4 IT6_1.0 【DenebMLK】 Fix Warning modification
							writeResult = fwrite ( convBase64Buffer, 1, pl_BufferOutputSize, pFileDest);

							writeResult = fwrite (BufferWithLogClose.c_str() , 1, BufferWithLogClose.size(), pFileDest);

							free(convBase64Buffer);
						}
						// 2018/12/07 FPT-DongND4 IT6_2.0 【Sparrow】 【AR1070670】 Modification End.
					}

					//if( buffer != NULL )	// 2018/12/07 FPT-DongND4 IT6_2.0 【Sparrow】 【AR1070670】 Modification.
					free(buffer);
				}
				
				fclose (pFileSrc);
			} else {
				DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "CreateZpDataXML Source File Open Error" );
				if ( pFileDest != NULL ){
					writeResult = fwrite (BufferWithLog.c_str() , 1, BufferWithLog.size(), pFileDest);
					writeResult = fwrite (BufferWithLogClose.c_str() , 1, BufferWithLogClose.size(), pFileDest);
				}
				//writeResult = fwrite (tempLogData , 1, sizeof(tempLogData), pFileDest);
			}
			//2020/07/22 FPT-TinhNT16 【制御構成変更_No.38】 【OP_BTS-25716】 【OP_BTS-25709】 Start.
			//cntFileLog++;				//2020/06/23 FPT-TinhNT16 【IT6_2.0_MR3】 【制御構成変更_No.38】.
			//2020/07/22 FPT-TinhNT16 【制御構成変更_No.38】 【OP_BTS-25716】 【OP_BTS-25709】 End.
		}
	}
	
//2020/06/23 FPT-TinhNT16 【IT6_2.0_MR3】 【制御構成変更_No.38】 Start.
	char varDebugHWLogTimeStamp[67];
	char DbgHWlogFileName[255];
	char tempHWLogData[250];
	Bool b_HWCheckResultExist = False;

	//check the existence of HWlog file.
	b_HWCheckResultExist = getHWCheckResultExist();
	
	if (True == b_HWCheckResultExist){
		char sc_HWResultFileName[255];
		memset(sc_HWResultFileName,'\0',sizeof(sc_HWResultFileName));
		Bool b_HWLogExist = False;

		memset(tempHWLogData,'\0',sizeof(tempHWLogData));
		memset(varDebugHWLogTimeStamp,'\0',sizeof(varDebugHWLogTimeStamp));
		memset(DbgHWlogFileName,'\0',sizeof(DbgHWlogFileName));
		
		//Get filename of HWLog.
		b_HWLogExist = req_HWCheckFileCreate(sc_HWResultFileName);

		if ( False == b_HWLogExist ){
		
		}
		else {
			string BufferWithHWLog;
			string filePathHWLog;
			filePathHWLog.append(sc_HWResultFileName);

			string fileNameHWLog = filePathHWLog.substr(filePathHWLog.find_last_of("/")+1);
			string timeStampHWLog = fileNameHWLog.substr(14,12);

			snprintf(tempHWLogData, sizeof(tempHWLogData), "<data id=\"%d\" ",cntFileLog+1);		//2020/09/04 FPT-HaDV8 [IT6_2.0MR3] [OP_BTS-27017] Modified.
			BufferWithHWLog.append(tempHWLogData);
			
			//2020/07/22 FPT-TinhNT16 【制御構成変更_No.38】 【OP_BTS-25716】 【OP_BTS-25709】 Start.
			switch( e_SystemType )
			{
				case TYPE_SystemType_JP:	// Japan.
					// Format: YYMMDDhhmmss
					snprintf(tempHWLogData, sizeof(tempHWLogData), "timestamp=\"20%s-%s-%sT%s:%s:%s\" ",timeStampHWLog.substr(0,2).c_str(),timeStampHWLog.substr(2,2).c_str(),timeStampHWLog.substr(4,2).c_str(),timeStampHWLog.substr(6,2).c_str(),timeStampHWLog.substr(8,2).c_str(),timeStampHWLog.substr(10,2).c_str());
					break;
				case TYPE_SystemType_US:	// US.
					// Format: MMDDYYhhmmss
					snprintf(tempHWLogData, sizeof(tempHWLogData), "timestamp=\"20%s-%s-%sT%s:%s:%s\" ",timeStampHWLog.substr(4,2).c_str(),timeStampHWLog.substr(0,2).c_str(),timeStampHWLog.substr(2,2).c_str(),timeStampHWLog.substr(6,2).c_str(),timeStampHWLog.substr(8,2).c_str(),timeStampHWLog.substr(10,2).c_str());
					break;
				case TYPE_SystemType_EU:	// EU.
					// Format: DDMMYYhhmmss
					snprintf(tempHWLogData, sizeof(tempHWLogData), "timestamp=\"20%s-%s-%sT%s:%s:%s\" ",timeStampHWLog.substr(4,2).c_str(),timeStampHWLog.substr(2,2).c_str(),timeStampHWLog.substr(0,2).c_str(),timeStampHWLog.substr(6,2).c_str(),timeStampHWLog.substr(8,2).c_str(),timeStampHWLog.substr(10,2).c_str());
					break;
				default:
					snprintf(tempHWLogData, sizeof(tempHWLogData), "timestamp=\"20%s-%s-%sT%s:%s:%s\" ",timeStampHWLog.substr(0,2).c_str(),timeStampHWLog.substr(2,2).c_str(),timeStampHWLog.substr(4,2).c_str(),timeStampHWLog.substr(6,2).c_str(),timeStampHWLog.substr(8,2).c_str(),timeStampHWLog.substr(10,2).c_str());
					break;
			}
			//2020/07/22 FPT-TinhNT16 【制御構成変更_No.38】 【OP_BTS-25716】 【OP_BTS-25709】 End.
			BufferWithHWLog.append(tempHWLogData);

			snprintf(tempHWLogData, sizeof(tempHWLogData), "fileName=\"%s\">",fileNameHWLog.c_str());
			BufferWithHWLog.append(tempHWLogData);

			string BufferWithHWLogClose = "</data>";

			FILE *pFileSrcHWLog = NULL;
			char *bufferHWLog = NULL;
			long lSizeHWLog;

			pFileSrcHWLog = fopen(sc_HWResultFileName, "rb");
			if ( pFileSrcHWLog != NULL ){
				fseek (pFileSrcHWLog , 0 , SEEK_END);
				lSizeHWLog = ftell (pFileSrcHWLog);
				rewind (pFileSrcHWLog);

				bufferHWLog = (char*) malloc (sizeof(char)*lSizeHWLog);
				if ( bufferHWLog != NULL ){
					memset(bufferHWLog,'\0', sizeof(char)*lSizeHWLog );
					readResult = fread(bufferHWLog,1,lSizeHWLog,pFileSrcHWLog);

					if ( readResult != (size_t)lSizeHWLog ){
						DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "CreateZpDataXML Source File Read Error" );
					}
					else {

					}

					if ( pFileDest != NULL ){
						writeResult = fwrite (BufferWithHWLog.c_str() , 1, BufferWithHWLog.size(), pFileDest);
						char *convBase64Buffer = (char*)malloc(sizeof(char) * lSizeHWLog * 2);
						if( convBase64Buffer != NULL ){
							memset(convBase64Buffer, '\0', sizeof(char) * lSizeHWLog * 2);
							int64_t pl_BufferOutputSize = 0;
							encodeBase64((uchar*)bufferHWLog, sizeof(char)*lSizeHWLog, (uchar*)convBase64Buffer, &pl_BufferOutputSize);
							writeResult = fwrite ( convBase64Buffer, 1, pl_BufferOutputSize, pFileDest);
							writeResult = fwrite (BufferWithHWLogClose.c_str() , 1, BufferWithHWLogClose.size(), pFileDest);
							free(convBase64Buffer);
						}
					}
					free(bufferHWLog);
				}
				fclose (pFileSrcHWLog);
			}
			else {
				DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "CreateZpDataXML Source File Open Error" );
				if ( pFileDest != NULL ){
					writeResult = fwrite (BufferWithHWLog.c_str() , 1, BufferWithHWLog.size(), pFileDest);
					writeResult = fwrite (BufferWithHWLogClose.c_str() , 1, BufferWithHWLogClose.size(), pFileDest);
				}
			}
		}
	}
	else {

	}
//2020/06/23 FPT-TinhNT16 【IT6_2.0_MR3】 【制御構成変更_No.38】 End.
	string DebugLogClose = "</debugLog>";
	writeResult = fwrite (DebugLogClose.c_str() , 1, DebugLogClose.size(), pFileDest);
	
	string DevIdClose = "</device>";				// 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Modified.
	writeResult = fwrite (DevIdClose.c_str() , 1, DevIdClose.size(), pFileDest);
	DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "writeResult", (int)writeResult);	// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
	fclose (pFileDest);
	return 0;
}
// 2018/07/03 HCLT-Vinay 【IT6_1.0】【制御構成変更_No.110】 End.

// 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Start.
//--------------------------------------------------------------------------------------------------.
// Overview			: Get time stamp for data tag of ZV command..
// Creator			: Fpt.
// Review			:.
// Change history	: 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Add.
//----------------------------------------------------------------------------------------------------.
void getTimeStamp( char* psc_TimeStamp)
{
	EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)" Start.");
	// 引数がNULLの場合終了.
	if(psc_TimeStamp == NULL){
		EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)" psc_TimeStamp is NULL");
		return;
	}

	// 日付部.
	time_t 		t_Time 		= 0;
	struct tm 	t_TmFile	= {};

	// Get current time.
	t_Time = RDSC_CsraUtil::getCurrentTime();

	//Convert current time (time_t -> tm).
	if( False == RDSC_TimeUtil::convertLocalTime((const time_t)t_Time, &t_TmFile)){
		RDSD_DBG_MESSAGE_STR( "convertLocalTime() NG", 0, 0, 0 );
		return;
	}

	// 補正.
	t_TmFile.tm_year	= t_TmFile.tm_year + 1900;
	t_TmFile.tm_mon		= t_TmFile.tm_mon + 1;

	// YYYYMMDDhhmmss.
	snprintf( psc_TimeStamp, 15, "%04d%02d%02d%02d%02d%02d", t_TmFile.tm_year, t_TmFile.tm_mon, t_TmFile.tm_mday, t_TmFile.tm_hour, t_TmFile.tm_min, t_TmFile.tm_sec );
}

//--------------------------------------------------------------------------------------------------.
// Overview			: Check exits file for usage frequency of WebUI and IWS app job.
// Creator			: Fpt.
// Review			:.
// Change history	: 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Add.
//----------------------------------------------------------------------------------------------------.
Bool isExistMarketUsageWebAPIFile()
{
	string c_Fname;
	c_Fname.append(CSRA_MARKET_USAGE_INFO_ROOT_PATH);
	c_Fname.append("/");
	c_Fname.append(CSRA_WEBAPI_CSV_FUNCCOUNTER_FILE);
	if ( FILD_ErrNotExist == FILF_CheckFileExist((char*)c_Fname.c_str()) )
	{
		EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)" The File Does Not Exist.");
		return False;
	}
	EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)" Market Usage File True.");
	return True;
}

//--------------------------------------------------------------------------------------------------.
// Overview			: Check exits folder for memory usage.
// Creator			: Fpt.
// Review			: .
// Change history	: 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Add.
//----------------------------------------------------------------------------------------------------.
Bool isExitsMemoryUsageFile()
{
	string c_Fname;
	c_Fname.append(CSRA_MEMORY_USAGE_PATH);

	if ( FILD_ErrNotExist == FILF_CheckFileExist((char*)c_Fname.c_str()) )
	{
		EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)" The Folder Does Not Exist.");
		return False;
	}
	EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)" Memory Folder True.");
	return True;
}

//--------------------------------------------------------------------------------------------------.
// Overview			: Get final result file name.
// Creator			: Fpt.
// Review			: .
// Change history	: 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Add.
//----------------------------------------------------------------------------------------------------.
Bool getMarketUsageInformationResultExist()
{
	EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)"Start.");

	if( (True == isExistMarketUsageWebAPIFile())				// Check exits file for usage frequency of WebUI and IWS app job.
	 || (True == isExitsMemoryUsageFile()) ) {					// Check exits file for memory usage.
		return True;
	}

	return False;
}

//--------------------------------------------------------------------------------------------------.
// Overview			: Check Exist Folder /ata0a/MarketUsageInformation.
// Creator			: Fpt.
// Review			: .
// Change history	: 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Add.
//----------------------------------------------------------------------------------------------------.
int checkExistFolderMarketUsageInformation() {
	EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)" Start.");
	int si_Ret						= OK;
	int b_reqCreateZVRootPath		= CSRA_STATUS_NONE;

	b_reqCreateZVRootPath = RDSC_CsraUtil::createMarketUsageInfomationRootFolder();
	EPN_DBG_MESSAGE((uchar*)"ZV: b_reqCreateZVRootPath ", (uint32_t)b_reqCreateZVRootPath, (uchar*)" !!!");
	if( (CSRA_STATUS_OK == b_reqCreateZVRootPath)
	 || (CSRA_STATUS_EXISTED == b_reqCreateZVRootPath) )
	{
		Bool b_ReqMemInfo = THRF_createMeminfoLogFile();		// Call func to collect memory file.
		if( True == b_ReqMemInfo ) {
			// NOP.
		}
		else {
			// NOP.
		}
	}
	else {
		si_Ret	= ERROR;
	}
	return si_Ret;
}

//--------------------------------------------------------------------------------------------------.
// Overview			: Compress Folder /ata0a/MarketUsageInformation/xxxxxxxxxxxxx__MarketUsageInformation.
// Creator			: Fpt.
// Review			: .
// Change history	: 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Add.
//----------------------------------------------------------------------------------------------------.
int compress_MarketUsageInformation(char* psc_FileName)
{
	EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)" Start.");
	// 引数がNULLの場合終了.
	if(psc_FileName == NULL){
		EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)" psc_FileName is NULL");
		return ERROR;
	}

//2023/02/02 FPT-LucNH2 OP_BTS-38822 Start.
	char* m_cp_CMD = NULL;								///< 圧縮コマンドライン.
//2023/02/02 FPT-LucNH2 OP_BTS-38822 End.
	int32_t si_Result = 0;						// コマンドの戻り値.
	int si_Ret = 0;

	char sc_SerialNumber[14];
	memset(sc_SerialNumber,'\0',sizeof(sc_SerialNumber));

	m_cp_CMD = (char*)malloc( _POSIX_ARG_MAX );
	memset(m_cp_CMD, 0x00, _POSIX_ARG_MAX );

	//Create filename.
	string c_Fname;
	APIC_RdsSetting* pc_RdsSetting = APIC_RdsSetting::newInstance();
	pc_RdsSetting->lock();
	pc_RdsSetting->getMachineID( (uchar *)sc_SerialNumber );
	pc_RdsSetting->unlock();

	c_Fname.append(sc_SerialNumber);
	c_Fname.append(CSRA_MARKET_USAGE_INFO_ZIP_FILENAME);

	// Create command compress.
	snprintf(m_cp_CMD, _POSIX_ARG_MAX, "cd %s;tar czvf %s *", CSRA_MARKET_USAGE_INFO_ROOT_PATH, c_Fname.c_str() );

	// モジュール詳細解析結果ファイルを圧縮.
	si_Result = system( m_cp_CMD );
// 2022/08/09 FPT-TinhNT16 【IT6_2.2MR3】【OP_BTS-38844】 Start.
	if ( -1 == si_Result ) {			// Executed command fails.
		si_Ret = -1;
	}
// 2022/08/09 FPT-TinhNT16 【IT6_2.2MR3】【OP_BTS-38844】 End.
	free( m_cp_CMD );
	m_cp_CMD = NULL;

	// 出力引数にファイル名文字列を格納.
	snprintf(psc_FileName, CSRA_DEF_FULLPATH_MAX, "%s/%s", CSRA_MARKET_USAGE_INFO_ROOT_PATH, c_Fname.c_str());
	EPN_DBG_MESSAGE((uchar*)"compress_MarketUsageInformation ", (uint32_t)si_Ret, (uchar*)" Finished.");

	return si_Ret;
}

//--------------------------------------------------------------------------------------------------.
// Overview			: Request create and compress file.
// Creator			: Fpt.
// Review			: .
// Change history	: 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Add.
//----------------------------------------------------------------------------------------------------.
Bool req_MarketUsageFileCreateAndCompress( char* psc_FileName )
{
	EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)" Start.");
	// 引数がNULLの場合終了.
	if(psc_FileName == NULL){
		EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)" psc_FileName is NULL.");
		return False;
	}

	int si_Ret = checkExistFolderMarketUsageInformation();
	Bool b_Ret = getMarketUsageInformationResultExist();

	//Coding compress file in folder /ata0a/MarketUsageInformation/.
	if ( 0 == si_Ret )
	{
		if ( b_Ret )
		{
			int si_comp = compress_MarketUsageInformation(psc_FileName);
			if ( si_comp != 0 ) {
				return False;
			}
		}
	}

	return True;
}

//--------------------------------------------------------------------------------------------------.
// Overview			: Delete zip file.
// Creator			: Fpt.
// Review			: .
// Change history	: 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Add.
//----------------------------------------------------------------------------------------------------.
void req_DeleteZipFolder()
{
	EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)" Start.");
//2023/02/02 FPT-LucNH2 OP_BTS-38822 Start.
	char* m_cp_CMD = NULL;								///< 圧縮コマンドライン.
//2023/02/02 FPT-LucNH2 OP_BTS-38822 End.
	int32_t si_Result = 0;
	m_cp_CMD = (char*)malloc( _POSIX_ARG_MAX  );
	memset(m_cp_CMD, 0x00, _POSIX_ARG_MAX );

	snprintf(m_cp_CMD, _POSIX_ARG_MAX, "cd %s;rm -rf *.tar.gz", CSRA_MARKET_USAGE_INFO_ROOT_PATH);

	si_Result = system( m_cp_CMD );
	free( m_cp_CMD );
	m_cp_CMD = NULL;

	return;
}

// 2022/08/09 FPT-TinhNT16 【IT6_2.2MR3】【OP_BTS-38844】 Start.
//--------------------------------------------------------------------------------------------------.
// Overview			: Delete data in folder /ata0a/MarketUsageInformation/.
// Creator			: Fpt.
// Review			: .
// Change history	: 2022/08/09 FPT-TinhNT16 【IT6_2.2MR3】【OP_BTS-38844】 Add.
//----------------------------------------------------------------------------------------------------.
void req_DeleteAnalysisDataMarketUsage()
{
	EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)" Start.");
//2023/02/02 FPT-LucNH2 OP_BTS-38822 Start.
	char* m_cp_CMD = NULL;								// 圧縮コマンドライン.
//2023/02/02 FPT-LucNH2 OP_BTS-38822 End.
	int32_t si_Result = 0;
	m_cp_CMD = (char*)malloc( _POSIX_ARG_MAX );
	memset(m_cp_CMD, 0x00, _POSIX_ARG_MAX );

	snprintf(m_cp_CMD, _POSIX_ARG_MAX, "cd %s;rm -rf *", CSRA_MARKET_USAGE_INFO_ROOT_PATH);

	si_Result = system( m_cp_CMD );				// Execute delete.
	free( m_cp_CMD );
	m_cp_CMD = NULL;

	return;
}
// 2022/08/09 FPT-TinhNT16 【IT6_2.2MR3】【OP_BTS-38844】 End.

//--------------------------------------------------------------------------------------------------.
// Overview			: Notification of transmission completion of market usage information result file.
// Creator			: Fpt.
// Review			: .
// Change history	: 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Add.
//----------------------------------------------------------------------------------------------------.
void completeSendMarketUsageResultFile()
{
	EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)" Start.");
	// Delete zip file.
	req_DeleteZipFolder();
	return ;
}

//--------------------------------------------------------------------------------------------------.
// Overview			: Create Data XML for ZV Command.
// Creator			: Fpt.
// Review			: .
// Change history	: 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Add.
//----------------------------------------------------------------------------------------------------.
int CreateZVDataXML( char * xmlFileName )
{
	EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)" Start.");
	const char tempXMLHeader[] = "<?xml version=\"1.0\" encoding=\"UTF-8\"?>\n";

	char sc_VarMarketUsageTimeStamp[67];
	char sc_MarketUsageFileName[255];
	char sc_TempMarketUsageData[255];

	char sc_VarDeviceId[31];
	char sc_TempDeviceId[31];

	Bool isExistFile = False;
	string c_Temp;

	size_t ui_ReadResult = 0;
	size_t ui_WriteResult = 0;

	FILE *pFileDest = NULL;

	if ( FILD_ErrNotExist != FILF_CheckFileExist( xmlFileName ) )
	{
		EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)"The Data File Of ZV Command Does Not Exist.");
		return 0;
	}

	remove( xmlFileName );
	pFileDest = fopen(xmlFileName, "wb");

	if ( pFileDest == NULL )
	{
		// Can't access and open file.
		DBG_CsrcErrorSequence( __FILE__, __LINE__, "CreateZVDataXML Destination File Open Error1 !!!" );
		return -1;
	}

	// Writing Device ID.
	memset(sc_VarDeviceId,'\0',sizeof(sc_VarDeviceId));
	memset(sc_TempDeviceId,'\0',sizeof(sc_TempDeviceId));

	if( pFileDest != NULL) {
		ui_WriteResult = fwrite (tempXMLHeader, 1, sizeof(tempXMLHeader) -1, pFileDest);
	} else {
		DBG_CsrcErrorSequence( __FILE__, __LINE__, "CreateZVDataXML Destination File Open Error2 !!!" );
		return -1;
	}

	APIC_RdsSetting* pc_RdsSetting = APIC_RdsSetting::newInstance();
	pc_RdsSetting->lock();
	pc_RdsSetting->getMachineID( (uchar *)sc_VarDeviceId );
	pc_RdsSetting->unlock();

	snprintf(sc_TempDeviceId, sizeof(sc_TempDeviceId), "<device id=\"%s\">",sc_VarDeviceId);		// <device> tag, id is serial number of machine.

	if( pFileDest != NULL) {
		ui_WriteResult = fwrite (sc_TempDeviceId, 1, strlen(sc_TempDeviceId), pFileDest);
	} else {
		DBG_CsrcErrorSequence( __FILE__, __LINE__, "CreateZVDataXML Destination File Write Error !!!" );
		return -1;
	}

	printf("Start Create Data XML...\n");
	if( OK == checkExistFolderMarketUsageInformation() ) {
		string c_MarketUsageInformationOpen = "<MarketUsageInformation>";		// <MarketUsageInformation> tag.
		ui_WriteResult = fwrite (c_MarketUsageInformationOpen.c_str(), 1, c_MarketUsageInformationOpen.size(), pFileDest);

		isExistFile = getMarketUsageInformationResultExist();

		if ( True == isExistFile )
		{
			memset(sc_VarMarketUsageTimeStamp,'\0',sizeof(sc_VarMarketUsageTimeStamp));
			memset(sc_MarketUsageFileName,'\0',sizeof(sc_MarketUsageFileName));
			memset(sc_TempMarketUsageData,'\0',sizeof(sc_TempMarketUsageData));

			Bool b_CreateFile = False;
			b_CreateFile = req_MarketUsageFileCreateAndCompress( sc_MarketUsageFileName );

			if( True == b_CreateFile )
			{
				string c_BufferData;
				string c_FilePath;
				string c_FileName;

				c_FilePath.append(sc_MarketUsageFileName);
				c_FileName = c_FilePath.substr(c_FilePath.find_last_of("/")+1);

				snprintf(sc_TempMarketUsageData, sizeof(sc_TempMarketUsageData), "<data id=1 ");
				c_BufferData.append(sc_TempMarketUsageData);

				// Call func get time stamp.
				getTimeStamp(sc_VarMarketUsageTimeStamp);
				string c_VarMarketUsageTimeStamp = (string)sc_VarMarketUsageTimeStamp;
				snprintf(sc_TempMarketUsageData, sizeof(sc_TempMarketUsageData), "timestamp=\"%s-%s-%sT%s:%s:%s\" ",c_VarMarketUsageTimeStamp.substr(0,4).c_str(),c_VarMarketUsageTimeStamp.substr(4,2).c_str(),c_VarMarketUsageTimeStamp.substr(6,2).c_str(),c_VarMarketUsageTimeStamp.substr(8,2).c_str(),c_VarMarketUsageTimeStamp.substr(10,2).c_str(),c_VarMarketUsageTimeStamp.substr(12,2).c_str());

				c_BufferData.append(sc_TempMarketUsageData);

				// Add file name.
				snprintf(sc_TempMarketUsageData, sizeof(sc_TempMarketUsageData), "fileName=\"%s\">",c_FileName.c_str());
				c_BufferData.append(sc_TempMarketUsageData);

				string c_BufferWithDataClose = "</data>";

				FILE *pFileSrcMarketUsageLog = NULL;
				char *bufferMarketUsageLog = NULL;
				long lSizeMarketUsageLog;

				pFileSrcMarketUsageLog = fopen(sc_MarketUsageFileName, "rb");

				if ( pFileSrcMarketUsageLog != NULL )
				{
					// Convert data to Base64.
					fseek (pFileSrcMarketUsageLog , 0 , SEEK_END);
					lSizeMarketUsageLog = ftell (pFileSrcMarketUsageLog);
					rewind (pFileSrcMarketUsageLog);

					bufferMarketUsageLog = (char*) malloc (sizeof(char)*lSizeMarketUsageLog);
					if ( bufferMarketUsageLog != NULL )
					{
						memset(bufferMarketUsageLog,'\0', sizeof(char)*lSizeMarketUsageLog );
						ui_ReadResult = fread(bufferMarketUsageLog,1,lSizeMarketUsageLog, pFileSrcMarketUsageLog);

						if ( ui_ReadResult != (size_t)lSizeMarketUsageLog )
						{
							DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "CreateZVDataXML Source File Read Error" );
						}
						else {

						}

						if ( pFileDest != NULL ) {
							ui_WriteResult = fwrite (c_BufferData.c_str() , 1, c_BufferData.size(), pFileDest);
							char *convBase64Buffer = (char*)malloc(sizeof(char) * lSizeMarketUsageLog * 2);
							if( convBase64Buffer != NULL ){
								memset(convBase64Buffer, '\0', sizeof(char) * lSizeMarketUsageLog * 2);
								int64_t pl_BufferOutputSize = 0;
								encodeBase64((uchar*)bufferMarketUsageLog, sizeof(char)*lSizeMarketUsageLog, (uchar*)convBase64Buffer, &pl_BufferOutputSize);
								ui_WriteResult = fwrite ( convBase64Buffer, 1, pl_BufferOutputSize, pFileDest);
								ui_WriteResult = fwrite (c_BufferWithDataClose.c_str() , 1, c_BufferWithDataClose.size(), pFileDest);
								free(convBase64Buffer);
								// 2022/08/09 FPT-TinhNT16 【IT6_2.2MR3】【OP_BTS-38844】 Start.
								if ( RDSD_CmdDataCreaterBuffSize <= pl_BufferOutputSize ) {			// Data size of ZV cmd after convert to Base64 over Max data size (2MB).
									EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)pl_BufferOutputSize, (uchar*)" Data size of ZV command is over!!!.");
									req_DeleteAnalysisDataMarketUsage();							// Delete data in folder /ata0a/MarketUsageInformation/.
								}
								// 2022/08/09 FPT-TinhNT16 【IT6_2.2MR3】【OP_BTS-38844】 End.
							}
						}
						free(bufferMarketUsageLog);
					}
					fclose (pFileSrcMarketUsageLog);
				}
				else {
					DBG_CsrcErrorSequence( __FILE__, __LINE__, "CreateZVDataXML Source File Open Error" );
					if ( pFileDest != NULL ){
						ui_WriteResult = fwrite (c_BufferData.c_str() , 1, c_BufferData.size(), pFileDest);
						ui_WriteResult = fwrite (c_BufferWithDataClose.c_str() , 1, c_BufferWithDataClose.size(), pFileDest);
					}
				}
			}
			else {
				//b_CreateFile == False.
				EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)" Compress File Data Of Market Usage Information Does Not Success.");
				req_DeleteZipFolder();			// 2022/08/09 FPT-TinhNT16 【IT6_2.2MR3】【OP_BTS-38844】 Added.
			}
		}
		else {
			//isExistFile == False.
			EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)" File Data Of Market Usage Information Does Not Exist.");
		}
		string c_MarketUsageInformationClose = "</MarketUsageInformation>";
		ui_WriteResult = fwrite (c_MarketUsageInformationClose.c_str() , 1, c_MarketUsageInformationClose.size(), pFileDest);
	}

	string c_DevIdClose = "</device>";
	ui_WriteResult = fwrite (c_DevIdClose.c_str() , 1, c_DevIdClose.size(), pFileDest);
	DBG_CsrcErrorSequence( __FILE__, __LINE__, "ui_WriteResult", (int)ui_WriteResult);
	fclose (pFileDest);

	return 0;
}
// 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 End.

#if !defined(CSRC_INVADIDATE_DX)

#define	USE_MAKE_FILE_PATH

#include	"system/fil/filg/FILC_BaseDir.h"
#if defined(USE_MAKE_FILE_PATH)
#include	"../ftp/RCD_Account.h"
#endif
//概要：DXコマンド用の不揮発の問い合わせ
//戻り値：不揮発アクセスの結果
//前提：ホストから要求されたコマンド文字列の文法チェックは正常終了している。
//備考：問い合わせデータの中に予約語が含まれる場合、変換する。
//　　　このコマンドのみXMLファイルへのアクセスが必要となるため、ここで処理を記述する。
//作成者：宮本
//レビュー：2004/05/08、宮本
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2004/04/08、宮本、DXコマンド対応
//			2007/06/22、佐久間 DXコマンド同報対応(DXコマンドの処理が完了したら読み込みポインタを進める) 
//			2012/08/20、曽根 MinervaBKのマージ対応（静的解析(CID433421)を反映 ”カンマ”の++処理を削除）.

	RDSE_NONVOLATILE_RESULT
RDSC_Inquiry::InquiryNonVolatileForDX(
		unsigned char* pUserDataBuffer, // [OUT] 応答のユーザデータバッファ先頭アドレス
		int32_t userDataBufferSize,		// [IN]  応答のユーザデータ最大サイズ[単位:byte]
		int32_t* pWrittenSize				// [OUT] 応答のユーザデータバッファに書き込んだサイズ[単位:byte]
		)
{
	RDSE_NONVOLATILE_RESULT	ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;

	//ファイル名の用意
	FILC_BaseDirIf*	pBaseDir = FILC_BaseDir::newInstance(FILE_DirTypeAccount);
	// 静的解析 CID146691 NULLチェックを追加 sone 2010/12/20 START.
	if( pBaseDir == NULL )
	{
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR1" );
		RDSF_Log("error pBaseDir NULL!!");
		return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR; 
	}
	// 静的解析 CID146691 NULLチェックを追加 sone 2010/12/20 END.
	
	#if defined(WIN32)
	TRACE("BaseDir:%s", pBaseDir->getPath());	// "/ram0/Account"を指しているようだ
	#endif
	const char* const	pEkcFilenameForCSRC = "ekc4csrc.xml";	// CSRC用 部門管理ＸＭＬファイル名
	static const int	MaxPath = 32;	// ファイル名のパスは、絶対パスで 16文字以下を想定している
#if defined(USE_MAKE_FILE_PATH)
	// 作成処理の名前作成に合わせる＋警告を出さないため＋スタックを節約するため
// Mosel start 2005.03.22 ikawa
#if !defined(COSIM)
// Mosel end 2005.03.22 ikawa
	RCDC_AccountDir	accountPath;
	const char*	p = 0;
	char	orgPath[MaxPath];
	p = accountPath.makeFilePath(pBaseDir->getPath(), USERCNT_FILENAME);		// 部門管理ファイル名
	// 静的解析 CID146692 NULLチェックを追加 sone 2010/12/20 START.
	if( p == NULL )
	{
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR2" );
		RDSF_Log("error USERCNT_FILENAME NULL!!");
		return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR; 
	}
	// 静的解析 CID146692 NULLチェックを追加 sone 2010/12/20 END.
	
	strcpy(orgPath, p);
	
	char	pathForCSRC[MaxPath];
	p = accountPath.makeFilePath(pBaseDir->getPath(), pEkcFilenameForCSRC);
	// 静的解析 CID146692 NULLチェックを追加 sone 2010/12/20 START.
	if( p == NULL )
	{
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR3" );
		RDSF_Log("error pEkcFilenameForCSRC NULL!!");
		return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR; 
	}
	// 静的解析 CID146692 NULLチェックを追加 sone 2010/12/20 END.
	
	strcpy(pathForCSRC, p);
// Mosel start 2005.03.22 ikawa
#else
	char	orgPath[MaxPath];
	sprintf(orgPath, "%s" "/" "%s", pBaseDir->getPath(), "usercnt.xml");	// 部門管理ファイル名
	char	pathForCSRC[MaxPath];
	sprintf(pathForCSRC, "%s" "/" "%s", pBaseDir->getPath(), pEkcFilenameForCSRC);
#endif
// Mosel end 2005.03.22 ikawa
#else
	char	orgPath[MaxPath];
	sprintf(orgPath, "%s" "/" "%s", pBaseDir->getPath(), "usercnt.xml");	// 部門管理ファイル名
	char	pathForCSRC[MaxPath];
	sprintf(pathForCSRC, "%s" "/" "%s", pBaseDir->getPath(), pEkcFilenameForCSRC);
#endif
	int	nRet = 0;

	//処理の最初で、ＸＭＬファイルを遠隔用にコピーする。ユーザデータバッファ先頭に コマンドとパラメータ情報を書き込む。
	if (mOffsetOfXmlFile == 0)
	{
		//ＸＭＬファイルのコピー
		nRet =
		FILF_Copy(orgPath, pathForCSRC);
		if (nRet == ERROR)
		{
			// コピーに失敗しても 前回のファイルがあれば 処理を継続可能であるが、前回のファイルが無い（初回）の可能性もある
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR4" );
			RDSF_Log("error at FILF_Copy");
			return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
		}

		if (userDataBufferSize < 6) { return RDSE_NONVOLATILE_RESULT_OK_NEXT; }	//他処理の仕様に合わせた

		// ユーザデータバッファ先頭に コマンドとパラメータ情報を書き込む
		strcpy(reinterpret_cast<char*>(pUserDataBuffer), " DX00");
		// ユーザデータバッファに書いた分だけ アドレスを進めて 書き込みサイズは減じる
		*pWrittenSize = 5;
		pUserDataBuffer += (*pWrittenSize);
		userDataBufferSize -= (*pWrittenSize);
		mStatus = STATUS_MIDDLE_OF_DX00;		// DX00の処理中［未終了］
	}
	// Gaia先取り Long案件ITNG CQNo327194 sone 2010/05/25 START.
	// mOffsetOfXmlFileが0でない場合は、書き込んだサイズを0クリアする.
	else
	{
			*pWrittenSize = 0;
	}
	// Gaia先取り Long案件ITNG CQNo327194 sone 2010/05/25 END.

	//ＸＭＬファイルのオープン
	int	nFd = FILF_Open(pathForCSRC, O_RDONLY, 0);
	if (nFd == ERROR)
	{
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR5" );
		RDSF_Log("error at FILF_Open");
		return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
	}

	//ＸＭＬファイルのサイズ確認
	struct stat		statusInfo;
	nRet =
	FILF_Stat(pathForCSRC, &statusInfo);
	if (nRet == ERROR)
	{
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR6" );
		RDSF_Log("error at FILF_Stat");
		FILF_Close(nFd);		//ＸＭＬファイルのクローズ
		return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
	}

	//ＸＭＬファイルのシーク
	nRet =
	static_cast<int32_t>(FILF_Lseek(nFd, (off_t)mOffsetOfXmlFile, SEEK_SET));
	if (nRet == ERROR)
	{
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR7" );
		RDSF_Log("error at FILF_Lseek");
		FILF_Close(nFd);		//ＸＭＬファイルのクローズ
		return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
	}

	//ＸＭＬファイルのリード
	int	nReadCount = 
	(int)FILF_Read(nFd, reinterpret_cast<char*>(pUserDataBuffer), userDataBufferSize);	// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
	if (nReadCount == ERROR)
	{
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR8" );
		RDSF_Log("error at FILF_Read");
		FILF_Close(nFd);		//ＸＭＬファイルのクローズ
		return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
	}

	//ＸＭＬファイルのクローズ
	FILF_Close(nFd);

	//予約文字を 規定文字に変換
	RDSC_CommandMatrix::ConvertReservedToRegulated(pUserDataBuffer, nReadCount);

	//書き込みサイズを更新
	*pWrittenSize += nReadCount;

	//読み込み位置を更新
	mOffsetOfXmlFile += nReadCount;

	if (mOffsetOfXmlFile < statusInfo.st_size)
	{
		ret = RDSE_NONVOLATILE_RESULT_OK_NEXT;			// 正常（残りデータ有）
	}
	else
	{
		ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;			// 正常（残りデータ無）
		mOffsetOfXmlFile = 0;
		//mStatus = STATUS_NORMAL_END;			// 終了

// 07/06/22 佐久間 DXコマンド同報対応(DXコマンドの処理が完了したら読み込みポインタを進める) 
#if 1
		//読み込みポインタを進める
		mpCommandTop += 3;

		//' 'または','または"終端"を期待
		if (*mpCommandTop == CodeOfSpace)	//空白
		{
			//正常
			mStatus = STATUS_NEXT_COMMAND;
			ret = RDSE_NONVOLATILE_RESULT_OK_NEXT;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForDX", *mpCommandTop, (uchar*)" 空白" );
		}
		else if (*mpCommandTop == CodeOfComma)	//カンマ
		{
			ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForDX", *mpCommandTop, (uchar*)" カンマ" );
//			continue;
		}
		else if (mpCommandTop == mpCommandEnd)	//終端
		{
			//正常（ユーザデータ文字列終端を検出）
			mStatus = STATUS_NORMAL_END;
			ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForDX", *mpCommandTop, (uchar*)" 終端" );
		}
		else if (mpCommandTop > mpCommandEnd)
		{
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR10" );
			ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForDX", *mpCommandTop, (uchar*)" 文法エラー(終端越え)" );
		}
		else	//予期しない文字
		{
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR11" );
			ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForDX", *mpCommandTop, (uchar*)" 文法エラー(不明文字)" );
		}
#endif
	}
	return ret;
}
#endif	//end of #if !defined(CSRC_INVADIDATE_DX)

Bool RDSC_Inquiry::zpFileStatus = False;
//2019/05/16 FPT-DongND4 [IT6_1.0MR2] [1070670]【Eagle】 added Start.
char RDSC_Inquiry::pathForCSRC_ZP[255] = "";
//2019/05/16 FPT-DongND4 [IT6_1.0MR2] [1070670]【Eagle】 added End.

// 2018/07/03 HCLT-Vinay 【IT6_1.0】【制御構成変更_No.110】 Start.
	RDSE_NONVOLATILE_RESULT
RDSC_Inquiry::InquiryNonVolatileForZP(
		unsigned char* pUserDataBuffer, // [OUT] 応答のユーザデータバッファ先頭アドレス
		int32_t userDataBufferSize,		// [IN]  応答のユーザデータ最大サイズ[単位:byte]
		int32_t* pWrittenSize				// [OUT] 応答のユーザデータバッファに書き込んだサイズ[単位:byte]
		)
{
	RDSE_NONVOLATILE_RESULT	ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;

	int			nRet = 0;
	int 		xmlCreateStatus = 0;
//	char		pathForCSRC_ZP[255] = "";//2019/05/16 FPT-DongND4 [IT6_1.0MR2] [1070670]【Eagle】 modification Start.
	memset(pathForCSRC_ZP,'\0',sizeof(pathForCSRC_ZP));
	strncpy(pathForCSRC_ZP, "/ata0a/DataFileZp.xml" ,22);
	
	if( zpFileStatus == False ){
		xmlCreateStatus = CreateZpDataXML(pathForCSRC_ZP);
		zpFileStatus = True;
	}

	//処理の最初で、ＸＭＬファイルを遠隔用にコピーする。ユーザデータバッファ先頭に コマンドとパラメータ情報を書き込む。
	if (mOffsetOfXmlFile == 0)
	{
		if (userDataBufferSize < 6) { return RDSE_NONVOLATILE_RESULT_OK_NEXT; }	//他処理の仕様に合わせた

		// ユーザデータバッファ先頭に コマンドとパラメータ情報を書き込む
		strcpy(reinterpret_cast<char*>(pUserDataBuffer), " ZP00");
		// ユーザデータバッファに書いた分だけ アドレスを進めて 書き込みサイズは減じる
		*pWrittenSize = 5;
		pUserDataBuffer += (*pWrittenSize);
		userDataBufferSize -= (*pWrittenSize);
		mStatus = STATUS_MIDDLE_OF_ZP00;		// ZP00の処理中［未終了］
	}
	else
	{
			*pWrittenSize = 0;
	}
	
//2019/06/13 FPT-HaiVV3 【Eagle】 [CWH] 文法エラー(ER50)について 【Eagle】 added Start.
	struct stat		statusInfo;
	statusInfo.st_size = 0;
	// 管理者設定>セキュリティ設定>セキュリティ詳細>デバッグ情報の取出し 
	APIC_DebugLogSystemSetting* pc_DebugLogSystemSetting = APIC_DebugLogSystemSetting::newInstance();
	Bool b_DebugInfoTake = pc_DebugLogSystemSetting->getTakeLogPermissionSetting();
	if (b_DebugInfoTake) //Check if Take Log Permission Setting is enable to add xml data to ZP00.
	{
		//ＸＭＬファイルのオープン
		int	nFd = FILF_Open(pathForCSRC_ZP, O_RDONLY, 0);
		if (nFd == ERROR)
		{
			DBG_CsrcErrorSequence(__FILE__, __LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR5");
			RDSF_Log("error at FILF_Open");
			zpFileStatus = False;
			return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
		}

		//ＸＭＬファイルのサイズ確認
		nRet =
			FILF_Stat(pathForCSRC_ZP, &statusInfo);
		if (nRet == ERROR)
		{
			DBG_CsrcErrorSequence(__FILE__, __LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR6");
			RDSF_Log("error at FILF_Stat");
			FILF_Close(nFd);		//ＸＭＬファイルのクローズ
			zpFileStatus = False;
			return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
		}

		//ＸＭＬファイルのシーク
		nRet =
			static_cast<int32_t>(FILF_Lseek(nFd, (off_t)mOffsetOfXmlFile, SEEK_SET));
		if (nRet == ERROR)
		{
			DBG_CsrcErrorSequence(__FILE__, __LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR7");
			RDSF_Log("error at FILF_Lseek");
			FILF_Close(nFd);		//ＸＭＬファイルのクローズ
			zpFileStatus = False;
			return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
		}

		//ＸＭＬファイルのリード
		int	nReadCount =
			(int)FILF_Read(nFd, reinterpret_cast<char*>(pUserDataBuffer), userDataBufferSize);	// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
		if (nReadCount == ERROR)
		{
			DBG_CsrcErrorSequence(__FILE__, __LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR8");
			RDSF_Log("error at FILF_Read");
			FILF_Close(nFd);		//ＸＭＬファイルのクローズ
			zpFileStatus = False;
			return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
		}

		//ＸＭＬファイルのクローズ
		FILF_Close(nFd);

		//予約文字を 規定文字に変換
		RDSC_CommandMatrix::ConvertReservedToRegulated(pUserDataBuffer, nReadCount);

		//書き込みサイズを更新
		*pWrittenSize += nReadCount;

		//読み込み位置を更新
		mOffsetOfXmlFile += nReadCount;
	}
//2019/06/13 FPT-HaiVV3 【Eagle】 [CWH] 文法エラー(ER50)について 【Eagle】 added End.
	if (mOffsetOfXmlFile < statusInfo.st_size)
	{
		ret = RDSE_NONVOLATILE_RESULT_OK_NEXT;			// 正常（残りデータ有）
	}
	else
	{
		ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;			// 正常（残りデータ無）
		mOffsetOfXmlFile = 0;
		//mStatus = STATUS_NORMAL_END;			// 終了
		
		zpFileStatus = False;
		char sc_AnalysisResultFileName[255];
		for( uint32_t mgmtTable = 1 ; mgmtTable <= 10 ; mgmtTable++){
			if(True == getAnalysisResultFileName(mgmtTable,sc_AnalysisResultFileName)){
				printf("CSRC ZP Analysis file : %d \n",mgmtTable);
				completeSendAnalysisResultFile(mgmtTable);
			}
		}
//2020/06/23 FPT-TinhNT16 【IT6_2.0_MR3】 【制御構成変更_No.38】 Start.
		char sc_HWResultFileName[255];
		if(True == req_HWCheckFileCreate(sc_HWResultFileName)){
			req_HWCheckFileSendComplete();
		}
//2020/06/23 FPT-TinhNT16 【IT6_2.0_MR3】 【制御構成変更_No.38】 End.

// 2018/07/03 佐久間 ZPコマンド同報対応(ZPコマンドの処理が完了したら読み込みポインタを進める) 
		//読み込みポインタを進める
		mpCommandTop += 3;

		//' 'または','または"終端"を期待
		if (*mpCommandTop == CodeOfSpace)	//空白
		{
			//正常
			mStatus = STATUS_NEXT_COMMAND;
			ret = RDSE_NONVOLATILE_RESULT_OK_NEXT;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZP", *mpCommandTop, (uchar*)" 空白" );
		}
		else if (*mpCommandTop == CodeOfComma)	//カンマ
		{
			ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZP", *mpCommandTop, (uchar*)" カンマ" );
//			continue;
		}
		else if (mpCommandTop == mpCommandEnd)	//終端
		{
			//正常（ユーザデータ文字列終端を検出）
			mStatus = STATUS_NORMAL_END;
			ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZP", *mpCommandTop, (uchar*)" 終端" );
			zpFileStatus = False;
		}
		else if (mpCommandTop > mpCommandEnd)
		{
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR10" );
			ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZP", *mpCommandTop, (uchar*)" 文法エラー(終端越え)" );
		}
		else	//予期しない文字
		{
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR11" );
			ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZP", *mpCommandTop, (uchar*)" 文法エラー(不明文字)" );
		}
	}
	DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "xmlCreateStatus:", xmlCreateStatus); // 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
	return ret;
}

// 2018/07/03 HCLT-Vinay 【IT6_1.0】【制御構成変更_No.110】 End.

// 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Start.
Bool RDSC_Inquiry::zvFileStatus = False;
char RDSC_Inquiry::pathForCSRC_ZV[255] = "";

RDSE_NONVOLATILE_RESULT RDSC_Inquiry::InquiryNonVolatileForZV(
	unsigned char* pUserDataBuffer,			// [OUT] 応答のユーザデータバッファ先頭アドレス.
	int32_t userDataBufferSize,				// [IN]  応答のユーザデータ最大サイズ[単位:byte].
	int32_t* pWrittenSize					// [OUT] 応答のユーザデータバッファに書き込んだサイズ[単位:byte].
	)
{
	EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZV ", (uint32_t)0, (uchar*)" Start.");

	RDSE_NONVOLATILE_RESULT ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;

	int nRet = 0;
	int xmlCreateStatus = 0;
	memset(pathForCSRC_ZV,'\0',sizeof(pathForCSRC_ZV));
	strncpy(pathForCSRC_ZV, "/ata0a/DataFileZV.xml", 22);
	
	if( zvFileStatus == False ){
		xmlCreateStatus = CreateZVDataXML(pathForCSRC_ZV);
		zvFileStatus = True;
	}

	// 処理の最初で、ＸＭＬファイルを遠隔用にコピーする。ユーザデータバッファ先頭に コマンドとパラメータ情報を書き込む.
	if (mOffsetOfXmlFile == 0)
	{
		if (userDataBufferSize < 6) {
			return RDSE_NONVOLATILE_RESULT_OK_NEXT;			// 他処理の仕様に合わせた.
		}

		// ユーザデータバッファ先頭に コマンドとパラメータ情報を書き込む.
		strncpy(reinterpret_cast<char*>(pUserDataBuffer), " ZV00", 6);
		// ユーザデータバッファに書いた分だけ アドレスを進めて 書き込みサイズは減じる.
		*pWrittenSize = 5;
		pUserDataBuffer += (*pWrittenSize);
		userDataBufferSize -= (*pWrittenSize);
		mStatus = STATUS_MIDDLE_OF_ZV00;		// ZV00の処理中［未終了］.
	}
	else {
		*pWrittenSize = 0;
	}

	struct stat 		statusInfo;
	statusInfo.st_size = 0;

	//ＸＭＬファイルのオープン.
	int nFd = FILF_Open(pathForCSRC_ZV, O_RDONLY, 0);
	if (nFd == ERROR)
	{
		DBG_CsrcErrorSequence(__FILE__, __LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR5");
		RDSF_Log("error at FILF_Open");
		zvFileStatus = False;
		return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
	}

	//ＸＭＬファイルのサイズ確認.
	nRet = FILF_Stat(pathForCSRC_ZV, &statusInfo);
	if (nRet == ERROR)
	{
		DBG_CsrcErrorSequence(__FILE__, __LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR6");
		RDSF_Log("error at FILF_Stat");
		FILF_Close(nFd);					//ＸＭＬファイルのクローズ.
		zvFileStatus = False;
		return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
	}

	//ＸＭＬファイルのシーク.
	nRet = static_cast<int32_t>(FILF_Lseek(nFd, (off_t)mOffsetOfXmlFile, SEEK_SET));
	if (nRet == ERROR)
	{
		DBG_CsrcErrorSequence(__FILE__, __LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR7");
		RDSF_Log("error at FILF_Lseek");
		FILF_Close(nFd);					//ＸＭＬファイルのクローズ.
		zvFileStatus = False;
		return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
	}

	//ＸＭＬファイルのリード.
	int nReadCount = (int)FILF_Read(nFd, reinterpret_cast<char*>(pUserDataBuffer), userDataBufferSize);
	if (nReadCount == ERROR)
	{
		DBG_CsrcErrorSequence(__FILE__, __LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR8");
		RDSF_Log("error at FILF_Read");
		FILF_Close(nFd);					//ＸＭＬファイルのクローズ.
		zvFileStatus = False;
		return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
	}

	//ＸＭＬファイルのクローズ.
	FILF_Close(nFd);

	//予約文字を 規定文字に変換.
	RDSC_CommandMatrix::ConvertReservedToRegulated(pUserDataBuffer, nReadCount);

	//書き込みサイズを更新.
	*pWrittenSize += nReadCount;

	//読み込み位置を更新.
	mOffsetOfXmlFile += nReadCount;

	if (mOffsetOfXmlFile < statusInfo.st_size)
	{
		ret = RDSE_NONVOLATILE_RESULT_OK_NEXT;				// 正常（残りデータ有）.
	}
	else {
		ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;			// 正常（残りデータ無）.
		mOffsetOfXmlFile = 0;
		zvFileStatus = False;
	//	char sc_MarketUsageFileName[255];

		if(True == getMarketUsageInformationResultExist()) {
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZV",(uint32_t)0, (uchar*)" start call completeSendMarketUsageResultFile().");
			completeSendMarketUsageResultFile();
		}

		//読み込みポインタを進める.
		mpCommandTop += 3;

		//' 'または','または"終端"を期待.
		if (*mpCommandTop == CodeOfSpace)			//空白.
		{
			//正常 .
			mStatus = STATUS_NEXT_COMMAND;
			ret = RDSE_NONVOLATILE_RESULT_OK_NEXT;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZV", *mpCommandTop, (uchar*)" 空白" );
		}
		else if (*mpCommandTop == CodeOfComma)		//カンマ.
		{
			ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZV", *mpCommandTop, (uchar*)" カンマ" );

		}
		else if (mpCommandTop == mpCommandEnd)		//終端.
		{
			//正常（ユーザデータ文字列終端を検出）.
			mStatus = STATUS_NORMAL_END;
			ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZV", *mpCommandTop, (uchar*)" 終端" );
			zvFileStatus = False;
		}
		else if (mpCommandTop > mpCommandEnd)
		{
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR10" );
			ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZV", *mpCommandTop, (uchar*)" 文法エラー(終端越え)" );
		}
		else	//予期しない文字.
		{
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR11" );
			ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZV", *mpCommandTop, (uchar*)" 文法エラー(不明文字)" );
		}
	}
	DBG_CsrcErrorSequence( __FILE__, __LINE__, "xmlCreateStatus: ", xmlCreateStatus);
	return ret;
}
// 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 End.

// 2022/08/09 FPT-TinhNT16 【IT6_2.2MR3】【OP_BTS-38844】 Start.
//--------------------------------------------------------------------------------------------------.
// Overview			: Update status for Zv file when reading ZV command ends abnormally.
// Creator			: Fpt.
// Review			: .
// Change history	: 2022/08/09 FPT-TinhNT16 【IT6_2.2MR3】【OP_BTS-38844】 Add.
//----------------------------------------------------------------------------------------------------.
void RDSC_Inquiry::UpdateStatusEndZvFile() {
	if ( STATUS_MIDDLE_OF_ZV00 == mStatus ){
		zvFileStatus = False;
	}
	EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)zvFileStatus, (uchar*)" Updated status for Zv file.");
}
// 2022/08/09 FPT-TinhNT16 【IT6_2.2MR3】【OP_BTS-38844】 End.

// 2016/05/12 H.Tokuno ZeusSBK 制御構成変更_No.57 カーネルログ取得による市場問題検証対応_Step2 Start.
//概要：ZHコマンド用の不揮発の問い合わせ.
//戻り値：不揮発アクセスの結果.
//前提：ホストから要求されたコマンド文字列の文法チェックは正常終了している.
//備考：dmsgを取得し、送信する.
//作成者：H.Tokuno.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//			2016/05/12、H.Tokuno、ZHコマンド対応.

	RDSE_NONVOLATILE_RESULT
RDSC_Inquiry::InquiryNonVolatileForZH(
		unsigned char* pUserDataBuffer, // [OUT] 応答のユーザデータバッファ先頭アドレス.
		int32_t userDataBufferSize,		// [IN]  応答のユーザデータ最大サイズ[単位:byte].
		int32_t* pWrittenSize,				// [OUT] 応答のユーザデータバッファに書き込んだサイズ[単位:byte].
		int param						// [IN]  ZHコマンドに付与されていたパラメタ.
		)
{
	// 戻り値のための変数.
	RDSE_NONVOLATILE_RESULT	ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;

	// 読込みが必要な状態かを表すフラグ.
	Bool	needReadFlag = True;

	// ファイル処理の結果格納変数.
	int		nRet = 0;

	// paramの妥当性チェック
	if ( (RDSD_ZH_HAS_NO_PARAM > param) || (param > RDSD_READ_PARAMAX_ZH) )
	{
		EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZH", (uint32_t)0, (uchar*)"argment:param is invalid" );
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR1" );
		initMemberForZH();
		return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
	}

	// ファイルディスクリプタを確認し、ファイルオープンの必要がある場合はオープンを行う.
	if ( mDmsgFd == 0 )
	{
		// 既にZHコマンドを読込み済みか？.
		if ( True == mAlreadyReadZHCommand )
		{
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZH", (uint32_t)0, (uchar*)"2 ZH commands are detected." );
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR2" );
			initMemberForZH();
			return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
		}

		// ZH+パラメタの文字列を書き込む際に使用する文字列.
		char	tempStr[6] = {0};

		// ファイル名のパスは、FILから絶対パスで 256文字が要求されている.
		char	pathForCSRC[PATH_MAX_KM+1] = {0};
		ushort  us_getParam = 0x00;
		
		// FILのAPI使用箇所.
		// paramが負の場合、パラメタがついていなかったとみなす.
		// そうでない場合は、正常なパラメタが付随していたとして、us_getParamに代入する.
		if ( RDSD_ZH_HAS_NO_PARAM != param )
		{

			us_getParam =static_cast<ushort>( param);
		}
		RDSE_GET_DMSGPATH_RESULT s_Result = getDmsgFilePath(us_getParam, pathForCSRC);

		// ファイル名が取得できなかった場合、設定パラメタを返して終わり.
		// パラメタなしの場合は00となる.
		if( RDSE_GET_DMSGPATH_RESULT_ERR_FILE_NOTFOUND == s_Result )
		{
			// バッファの残量チェック.
			if ( RDSD_ZH_DATA_MAXSIZE < (mOffsetOfDmsgFile + 5) )
			{
				EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZH", (uint32_t)0, (uchar*)"buffer has no free area." );
				DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_OTHER3" );
				initMemberForZH();
				return RDSE_NONVOLATILE_RESULT_OTHER;
			}

			snprintf( tempStr, sizeof(tempStr), " ZH%02X", us_getParam);
			strncpy(reinterpret_cast<char*>(pUserDataBuffer), tempStr, sizeof(tempStr));
			// ユーザデータバッファに書いた分だけ アドレスを進めて 書き込みサイズは減じる.
			*pWrittenSize = 5;
			pUserDataBuffer += (*pWrittenSize);
			userDataBufferSize -= (*pWrittenSize);

			// 読込み処理不要としておく.
			needReadFlag = False;
		}
		// その他のエラーで終了した場合、エラー応答として終了する.
		else if ( RDSE_GET_DMSGPATH_RESULT_ERR_OHTER == s_Result )
		{
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZH", (uint32_t)0, (uchar*)"getDmsgFilePath returned another error." );
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR4" );
			initMemberForZH();
			return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
		}
		// ファイル名が取得できた場合、ファイルオープン処理.
		else
		{
			if (userDataBufferSize < 6) { return RDSE_NONVOLATILE_RESULT_OK_NEXT; }	//他処理の仕様に合わせた.

			// ユーザデータバッファ先頭に コマンドとパラメータ情報を書き込む.
			snprintf( tempStr, sizeof(tempStr), " ZH%02X", us_getParam);
			strncpy(reinterpret_cast<char*>(pUserDataBuffer), tempStr, sizeof(tempStr));
			// ユーザデータバッファに書いた分だけ アドレスを進めて 書き込みサイズは減じる.
			*pWrittenSize = 5;
			pUserDataBuffer += (*pWrittenSize);
			userDataBufferSize -= (*pWrittenSize);

			//ファイルのオープン.
			mDmsgFd = FILF_Open(pathForCSRC, O_RDONLY, 0);
			if (mDmsgFd == ERROR)
			{
				EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZH", (uint32_t)0, (uchar*)"file open error" );
				DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR5" );
				initMemberForZH();
				return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
			}

			//ファイルのサイズ確認.
			struct stat		statusInfo = {0};
			nRet = FILF_Stat(pathForCSRC, &statusInfo);
			if (nRet == ERROR)
			{
				EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZH", (uint32_t)0, (uchar*)"status get error" );
				DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR6" );
				FILF_Close(mDmsgFd);
				initMemberForZH();
				return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
			}

			// ファイルサイズをメンバ変数へ代入.

			mDmsgFileSize = static_cast<int32_t> (statusInfo.st_size);
		}
	}

	// ファイル読込みは必要か？.
	if ( True == needReadFlag )
	{
		// mOffsetOfDmsgFileが0でない場合は、書き込んだサイズを0クリアする.
		if (mOffsetOfDmsgFile != 0)
		{
			*pWrittenSize = 0;
		}

		// ファイルのシーク.

		nRet = static_cast<int32_t>(FILF_Lseek(mDmsgFd, mOffsetOfDmsgFile, SEEK_SET));
		if (nRet == ERROR)
		{
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZH", (uint32_t)0, (uchar*)"file seek error" );
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR7" );
			FILF_Close(mDmsgFd);
			initMemberForZH();
			return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
		}

		// 読み込もうとしているサイズ"userDataBufferSize"でファイルを読み込んだ際、.
		// 512kbyteを超えてしまう場合、読込みサイズが512kbyteに丸められるように調整する.
		int32_t readSize = userDataBufferSize;
		if ( RDSD_ZH_DATA_MAXSIZE < (mOffsetOfDmsgFile + userDataBufferSize) )
		{
			readSize = RDSD_ZH_DATA_MAXSIZE - mOffsetOfDmsgFile;
		}

		// ファイルのリード.
		int nReadCount = (int)FILF_Read(mDmsgFd, reinterpret_cast<char*>(pUserDataBuffer), readSize);	// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
		if (nReadCount == ERROR)
		{
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZH", (uint32_t)0, (uchar*)"file read error" );
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR8" );
			FILF_Close(mDmsgFd);
			initMemberForZH();
			return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
		}

		// 予約文字を 規定文字に変換。制御文字の処理に先行して実施が必要.
		RDSC_CommandMatrix::ConvertReservedToRegulated(pUserDataBuffer, nReadCount);

		// 制御文字を、半角スペースに置き換え.
		RDSC_CommandMatrix::ConvertControlCharacterToRegulatedSpace(pUserDataBuffer, nReadCount); // 2016/05/30 H.Tokuno ZeusSBK CQ530311_Dmesg制御文字変換対応.

		// 書き込みサイズを更新.
		*pWrittenSize += nReadCount;

		// 読み込み位置を更新.
		mOffsetOfDmsgFile += nReadCount;
		
		// オフセットが512kbyteに到達した際、もしくは.
		// オフセットがデータサイズに到達した際に、読込みフラグを下げる.
		if ( (mOffsetOfDmsgFile >= mDmsgFileSize) || (mOffsetOfDmsgFile >= RDSD_ZH_DATA_MAXSIZE) )
		{
			needReadFlag = False;
		}
	}

	// ループ判定.
	if ( True == needReadFlag )
	{
		ret = RDSE_NONVOLATILE_RESULT_OK_NEXT;			// 正常（残りデータ有）.
	}
	else
	{
		ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;			// 正常（残りデータ無）.

		// ZHコマンド同報対応(ZHコマンドの処理が完了したら読み込みポインタを進める) .
		// 読み込みポインタを進める.
		// パラメタが存在する場合はパラメタ含めて5進める.
		if ( RDSD_ZH_HAS_NO_PARAM != param )
		{
			mpCommandTop += 5;
		}
		// パラメタが無い場合はコマンド+空白の3進める.
		else
		{
			mpCommandTop += 3;
		}

		// ZHコマンドを読み込み、次のコマンドを確認する場合.
		Bool readZHCommand = False;
		
		
		//' 'または','または"終端"を期待.
		if (*mpCommandTop == CodeOfSpace)	//空白.
		{
			//正常
			mStatus = STATUS_NEXT_COMMAND;
			ret = RDSE_NONVOLATILE_RESULT_OK_NEXT;
			readZHCommand = True;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZH", *mpCommandTop, (uchar*)" 空白" );
		}
		else if (*mpCommandTop == CodeOfComma)	//カンマ.
		{
			ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZH", *mpCommandTop, (uchar*)" カンマ" );
//			continue;
		}
		else if (mpCommandTop == mpCommandEnd)	//終端.
		{
			//正常（ユーザデータ文字列終端を検出）.
			mStatus = STATUS_NORMAL_END;
			ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZH", *mpCommandTop, (uchar*)" 終端" );
		}
		else if (mpCommandTop > mpCommandEnd)
		{
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR10" );
			ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZH", *mpCommandTop, (uchar*)" 文法エラー(終端越え)" );
		}
		else	//予期しない文字.
		{
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR11" );
			ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZH", *mpCommandTop, (uchar*)" 文法エラー(不明文字)" );
		}
		
		//ファイルのクローズ.
		if ( mDmsgFd != 0 )
		{
    		FILF_Close(mDmsgFd);
		}
		initMemberForZH(readZHCommand);
	}
	return ret;
}



// 2017/02/06 H.Tokuno IT5_4.1 機器構成変更_No.49 CTLログの容量拡張 Start.
//概要：ZMコマンド用の不揮発の問い合わせ.
//戻り値：不揮発アクセスの結果.
//前提：ホストから要求されたコマンド文字列の文法チェックは正常終了している.
//備考：エンジンログを取得し、送信する.
//作成者：H.Tokuno.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//			2017/02/06、H.Tokuno、ZMコマンド対応.

	RDSE_NONVOLATILE_RESULT
RDSC_Inquiry::InquiryNonVolatileForZM(
		unsigned char* pUserDataBuffer, // [OUT] 応答のユーザデータバッファ先頭アドレス.
		int32_t userDataBufferSize,		// [IN]  応答のユーザデータ最大サイズ[単位:byte].
		int32_t* pWrittenSize,				// [OUT] 応答のユーザデータバッファに書き込んだサイズ[単位:byte].
		int param						// [IN]  ZMコマンドに付与されていたパラメタ.
		)
{
	// 戻り値のための変数.
	RDSE_NONVOLATILE_RESULT	ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;

	// 読込みが必要な状態かを表すフラグ.
	Bool	needReadFlag = True;

	// ファイル処理の結果格納変数.
	off_t		nRet = 0;	// 2017/09/11 kobayashi Eagle 静的解析 No.89022.

	// paramの妥当性チェック.
	if ( (RDSD_ZM_HAS_NO_PARAM > param) || (param > RDSD_READ_PARAMAX_ZM) )
	{
		EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZM", (uint32_t)0, (uchar*)"argment:param is invalid" );
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR1" );
		initMemberForZM();
		return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
	}

	// ファイルディスクリプタを確認し、ファイルオープンの必要がある場合はオープンを行う.
	if ( miEngLogFd == 0 )
	{
		// 既にZMコマンドを読込み済みか？.
		if ( True == mbAlreadyReadZMCommand )
		{
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZM", (uint32_t)0, (uchar*)"2 ZM commands are detected." );
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR2" );
			initMemberForZM();
			return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
		}

		// ZM+パラメタの文字列を書き込む際に使用する文字列.
		char	tempStr[6] = {0};

		// ファイル名のパスは、FILから絶対パスで 256文字が要求されている.
		char	pathForCSRC[PATH_MAX_KM+1] = {0};
		ushort  us_getParam = 0x00;
		
		// FILのAPI使用箇所.
		// paramが負の場合、パラメタがついていなかったとみなす.
		// そうでない場合は、正常なパラメタが付随していたとして、us_getParamに代入する.
		if ( RDSD_ZM_HAS_NO_PARAM != param )
		{
			us_getParam = (ushort)param;		// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
		}
		RDSE_GET_ENGLOGPATH_RESULT s_Result = getEngLogFilePath(us_getParam, pathForCSRC);

		// ファイル名が取得できなかった場合、設定パラメタを返して終わり.
		// パラメタなしの場合は00となる.
		if( RDSE_GET_ENGLOGPATH_RESULT_ERR_FILE_NOTFOUND == s_Result )
		{
			// バッファの残量チェック.
			if ( RDSD_ZM_DATA_MAXSIZE < (mlOffsetOfEngLogFile + 5) )
			{
				EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZM", (uint32_t)0, (uchar*)"buffer has no free area." );
				DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_OTHER3" );
				initMemberForZM();
				return RDSE_NONVOLATILE_RESULT_OTHER;
			}

			snprintf( tempStr, sizeof(tempStr), " ZM%02X", us_getParam);
			strncpy(reinterpret_cast<char*>(pUserDataBuffer), tempStr, sizeof(tempStr));
			// ユーザデータバッファに書いた分だけ アドレスを進めて 書き込みサイズは減じる.
			*pWrittenSize = 5;
			pUserDataBuffer += (*pWrittenSize);
			userDataBufferSize -= (*pWrittenSize);

			// 読込み処理不要としておく.
			needReadFlag = False;
		}
		// その他のエラーで終了した場合、エラー応答として終了する.
		else if ( RDSE_GET_ENGLOGPATH_RESULT_ERR_OHTER == s_Result )
		{
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZM", (uint32_t)0, (uchar*)"getEngLogFilePath returned another error." );
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR4" );
			initMemberForZM();
			return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
		}
		// ファイル名が取得できた場合、ファイルオープン処理.
		else
		{
			if (userDataBufferSize < 6) { return RDSE_NONVOLATILE_RESULT_OK_NEXT; }	//他処理の仕様に合わせた.

			// ユーザデータバッファ先頭に コマンドとパラメータ情報を書き込む.
			snprintf( tempStr, sizeof(tempStr), " ZM%02X", us_getParam);
			strncpy(reinterpret_cast<char*>(pUserDataBuffer), tempStr, sizeof(tempStr));
			// ユーザデータバッファに書いた分だけ アドレスを進めて 書き込みサイズは減じる.
			*pWrittenSize = 5;
			pUserDataBuffer += (*pWrittenSize);
			userDataBufferSize -= (*pWrittenSize);

			//ファイルのオープン.
			miEngLogFd = FILF_Open(pathForCSRC, O_RDONLY, 0);
			if (miEngLogFd == ERROR)
			{
				EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZM", (uint32_t)0, (uchar*)"file open error" );
				DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR5" );
				initMemberForZM();
				return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
			}

			//ファイルのサイズ確認.
			struct stat		statusInfo = {0};
			nRet = FILF_Stat(pathForCSRC, &statusInfo);
			if (nRet == ERROR)
			{
				EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZM", (uint32_t)0, (uchar*)"status get error" );
				DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR6" );
				FILF_Close(miEngLogFd);
				initMemberForZM();
				return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
			}

			// ファイルサイズをメンバ変数へ代入.
			miEngLogFileSize = statusInfo.st_size;
		}
	}

	// ファイル読込みは必要か？.
	if ( True == needReadFlag )
	{
		// mOffsetOfDmsgFileが0でない場合は、書き込んだサイズを0クリアする.
		if (mlOffsetOfEngLogFile != 0)
		{
			*pWrittenSize = 0;
		}

		// ファイルのシーク.
		nRet = FILF_Lseek(miEngLogFd, mlOffsetOfEngLogFile, SEEK_SET);
		if (nRet == ERROR)
		{
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZM", (uint32_t)0, (uchar*)"file seek error" );
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR7" );
			FILF_Close(miEngLogFd);
			initMemberForZM();
			return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
		}

		// 読み込もうとしているサイズ"userDataBufferSize"でファイルを読み込んだ際、.
		// 214400bytesを超えてしまう場合、読込みサイズが214400byteに丸められるように調整する.
		int32_t readSize = userDataBufferSize;
		if ( RDSD_ZM_DATA_MAXSIZE < (mlOffsetOfEngLogFile + userDataBufferSize) )
		{
			readSize = static_cast<int32_t>(RDSD_ZM_DATA_MAXSIZE - mlOffsetOfEngLogFile);    //[AR : 1026431] 【 Eagle 】【静的解析対応用課題】 2017/11/28 [HCL_Raghu] 
		}

		// ファイルのリード.
		int nReadCount = (int)FILF_Read(miEngLogFd, reinterpret_cast<char*>(pUserDataBuffer), readSize);		// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
		if (nReadCount == ERROR)
		{
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZM", (uint32_t)0, (uchar*)"file read error" );
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR8" );
			FILF_Close(miEngLogFd);
			initMemberForZM();
			return RDSE_NONVOLATILE_RESULT_FILE_ACCESS_ERR;
		}

		// 予約文字を 規定文字に変換。制御文字の処理に先行して実施が必要.
		RDSC_CommandMatrix::ConvertReservedToRegulated(pUserDataBuffer, nReadCount);

		// 制御文字を、半角スペースに置き換え.
		RDSC_CommandMatrix::ConvertControlCharacterToRegulatedSpace(pUserDataBuffer, nReadCount); // 2016/05/30 H.Tokuno ZeusSBK CQ530311_Dmesg制御文字変換対応.

		// 書き込みサイズを更新.
		*pWrittenSize += nReadCount;

		// 読み込み位置を更新.
		mlOffsetOfEngLogFile += nReadCount;
		
		// オフセットが214400bytesに到達した際、もしくは.
		// オフセットがデータサイズに到達した際に、読込みフラグを下げる.
		if ( (mlOffsetOfEngLogFile >= miEngLogFileSize) || (mlOffsetOfEngLogFile >= RDSD_ZM_DATA_MAXSIZE) )
		{
			needReadFlag = False;
		}
	}

	// ループ判定.
	if ( True == needReadFlag )
	{
		ret = RDSE_NONVOLATILE_RESULT_OK_NEXT;			// 正常（残りデータ有）.
	}
	else
	{
		ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;			// 正常（残りデータ無）.

		// ZMコマンド同報対応(ZMコマンドの処理が完了したら読み込みポインタを進める) .
		// 読み込みポインタを進める.
		// パラメタが存在する場合はパラメタ含めて5進める.
		if ( RDSD_ZM_HAS_NO_PARAM != param )
		{
			mpCommandTop += 5;
		}
		// パラメタが無い場合はコマンド+空白の3進める.
		else
		{
			mpCommandTop += 3;
		}

		// ZMコマンドを読み込み、次のコマンドを確認する場合.
		Bool readZMCommand = False;
		
		
		//' 'または','または"終端"を期待.
		if (*mpCommandTop == CodeOfSpace)	//空白.
		{
			//正常.
			mStatus = STATUS_NEXT_COMMAND;
			ret = RDSE_NONVOLATILE_RESULT_OK_NEXT;
			readZMCommand = True;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZM", *mpCommandTop, (uchar*)" 空白" );
		}
		else if (*mpCommandTop == CodeOfComma)	//カンマ.
		{
			ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZM", *mpCommandTop, (uchar*)" カンマ" );
		}
		else if (mpCommandTop == mpCommandEnd)	//終端.
		{
			//正常（ユーザデータ文字列終端を検出）.
			mStatus = STATUS_NORMAL_END;
			ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZM", *mpCommandTop, (uchar*)" 終端" );
		}
		else if (mpCommandTop > mpCommandEnd)
		{
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR10" );
			ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZM", *mpCommandTop, (uchar*)" 文法エラー(終端越え)" );
		}
		else	//予期しない文字.
		{
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR11" );
			ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForZM", *mpCommandTop, (uchar*)" 文法エラー(不明文字)" );
		}
		
		//ファイルのクローズ.
		if ( miEngLogFd != 0 )
		{
    		FILF_Close(miEngLogFd);
		}
		initMemberForZM(readZMCommand);
	}
	return ret;
}
// 2017/02/06 H.Tokuno IT5_4.1 機器構成変更_No.49 CTLログの容量拡張 End.

// 2010/03/01 yito UV-00866 機能カウンタ対応 start.
//概要：機能カウンタコマンド用の不揮発の問い合わせ.
//戻り値：不揮発アクセスの結果.
//前提：ホストから要求されたコマンド文字列の文法チェックは正常終了している。.
//作成者：伊藤.
//レビュー：未.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//			2010/03/01、伊藤 新規作成.
	RDSE_NONVOLATILE_RESULT
RDSC_Inquiry::InquiryNonVolatileForFuncCount(
		uchar* pUserDataBuffer, // [OUT] 応答のユーザデータバッファ先頭アドレス.
		int32_t userDataBufferSize,		// [IN]  応答のユーザデータ最大サイズ[単位:byte].
		int32_t* pWrittenSize				// [OUT] 応答のユーザデータバッファに書き込んだサイズ[単位:byte].
		)
{
	RDSE_NONVOLATILE_RESULT	ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;

	EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForFC", (int32_t)meCreateXMLType, (uchar*)" start" );

	//処理の最初で、ＸＭＬファイルを生成する。ユーザデータバッファ先頭に コマンドとパラメータ情報を書き込む.
	if (mOffsetOfXmlFile == 0)
	{
		// 対象の構造体を初期化.
		memset( &mstXmlData[meCreateXMLType], 0, sizeof(mstXmlData[0]) );
		
		// XML生成を行う.
		if( RDSG_GetXML( &mstXmlData[meCreateXMLType], meCreateXMLType ) )
		{	// 成功.
			EPN_DBG_MESSAGE( (uchar*)"RDSG_GetXML", (int32_t)meCreateXMLType, (uchar*)" OK" );	
			EPN_DBG_MESSAGE( (uchar*)"mstXmlData[meCreateXMLType] :", (int32_t)mstXmlData[meCreateXMLType].l_DataSize, (uchar*)" " );	
		}
		else
		{	// 失敗.
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_OTHER1" );
			EPN_DBG_MESSAGE( (uchar*)"RDSG_GetXML", (int32_t)meCreateXMLType, (uchar*)" NG" );	
			return RDSE_NONVOLATILE_RESULT_OTHER;	// エラーを返却（XML用領域確保に失敗）.
		}

		if(userDataBufferSize < 6)			// 他処理の仕様に合わせた.
		{
			return RDSE_NONVOLATILE_RESULT_OK_NEXT;
		}

		// ユーザデータバッファ先頭に コマンドとパラメータ情報を書き込む.
		switch(meCreateXMLType)
		{
			case RDSE_CREATE_XML_FUNCTION_COUNTERS:
				strcpy(reinterpret_cast<char*>(pUserDataBuffer), " BX00");
				break;
// 2015/10/28 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(AQコマンド追加) START.
			case RDSE_CREATE_XML_APPUSEFREQ_COUNTERS:
				strcpy(reinterpret_cast<char*>(pUserDataBuffer), " AQ00");
				break;
// 2015/10/28 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(AQコマンド追加) END.
			case RDSE_CREATE_XML_UNIVERSAL_SETTINGS:
				strcpy(reinterpret_cast<char*>(pUserDataBuffer), " AR00");
				break;
			case RDSE_CREATE_XML_USER_SETTINGS:
				strcpy(reinterpret_cast<char*>(pUserDataBuffer), " AS00");
				break;
			case RDSE_CREATE_XML_ADMINISTRATOR_SETTINGS:
				strcpy(reinterpret_cast<char*>(pUserDataBuffer), " AT00");
				break;
			case RDSE_CREATE_XML_SERVICE_SETTINGS:
				strcpy(reinterpret_cast<char*>(pUserDataBuffer), " AU00");
				break;
// 2015/11/05 ktakahashi ZeusS_ZX0 制御構成変更_No.76-2 ネットワークエラーカウンタ対応 Start.
			case RDSE_CREATE_XML_NETWORKERR_COUNTERS:
				strcpy(reinterpret_cast<char*>(pUserDataBuffer), " EM00");
				break;
// 2015/11/05 ktakahashi ZeusS_ZX0 制御構成変更_No.76-2 ネットワークエラーカウンタ対応 End.
			default:
				// ありえない.
				break;
		}
		// ユーザデータバッファに書いた分だけ アドレスを進めて 書き込みサイズは減じる
		*pWrittenSize = 5;
		pUserDataBuffer += (*pWrittenSize);
		userDataBufferSize -= (*pWrittenSize);
	}
	// Gaia先取り Long案件ITNG CQNo327194 sone 2010/05/25 START.
	// mOffsetOfXmlFileが0でない場合は、書き込んだサイズを0クリアする.
	else
	{
			*pWrittenSize = 0;
	}
	// Gaia先取り Long案件ITNG CQNo327194 sone 2010/05/25 END.

	
	// データコピー.
	int32_t	sl_ReadCount = 0;
	if( userDataBufferSize < (int32_t)(mstXmlData[meCreateXMLType].l_DataSize - mOffsetOfXmlFile))		// 2010/11/05 higuchi Minerva warning 対応.
	{	// バッファサイズ分コピー.
		sl_ReadCount = userDataBufferSize;
	}
	else
	{	// 全てコピー.
		sl_ReadCount = mstXmlData[meCreateXMLType].l_DataSize - mOffsetOfXmlFile;
	}
	memcpy( pUserDataBuffer, mstXmlData[meCreateXMLType].ptr + mOffsetOfXmlFile, sl_ReadCount );
	
	// 予約文字を 規定文字に変換.
	RDSC_CommandMatrix::ConvertReservedToRegulated(pUserDataBuffer, sl_ReadCount);

	// 書き込みサイズを更新.
	*pWrittenSize += sl_ReadCount;
	EPN_DBG_MESSAGE( (uchar*)"mOffsetOfXmlFile :", (int32_t)mOffsetOfXmlFile, (uchar*)" " );
	EPN_DBG_MESSAGE( (uchar*)"sl_ReadCount     :", (int32_t)sl_ReadCount, (uchar*)" " );

	// 読み込み位置を更新.
	mOffsetOfXmlFile += sl_ReadCount;
	if ( (int32_t)mOffsetOfXmlFile < mstXmlData[meCreateXMLType].l_DataSize )							// 2010/11/05 higuchi Minerva warning 対応.
	{
		ret = RDSE_NONVOLATILE_RESULT_OK_NEXT;			// 正常（残りデータ有）.
	}
	else
	{
		ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;		// 正常（残りデータ無）.
		mOffsetOfXmlFile = 0;

		// XMLデータの開放＆初期化を行う.
		free( mstXmlData[meCreateXMLType].ptr);
		memset( &mstXmlData[meCreateXMLType], 0, sizeof(mstXmlData[0]) );

		// 読み込みポインタを進める.
		mpCommandTop += 3;

		// ' 'または','または"終端"を期待.
		if (*mpCommandTop == CodeOfSpace)	// 空白.
		{
			// 正常.
			mStatus = STATUS_NEXT_COMMAND;
			ret = RDSE_NONVOLATILE_RESULT_OK_NEXT;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForFC", *mpCommandTop, (uchar*)" 空白" );
		}
		else if (*mpCommandTop == CodeOfComma)	// カンマ.
		{
			mpCommandTop++;
			// 書き込みバッファに カンマを書く.
			*pUserDataBuffer++ = CodeOfComma;
			ret = RDSE_NONVOLATILE_RESULT_OK_NEXT;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForFC", *mpCommandTop, (uchar*)" カンマ" );
		}
		else if (mpCommandTop == mpCommandEnd)	//終端.
		{
			// 正常（ユーザデータ文字列終端を検出）.
			mStatus = STATUS_NORMAL_END;
			ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForFC", *mpCommandTop, (uchar*)" 終端" );
		}
		else if (mpCommandTop > mpCommandEnd)
		{
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR12" );
			ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForFC", *mpCommandTop, (uchar*)" 文法エラー(終端越え)" );
		}
		else	// 予期しない文字.
		{
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR13" );
			ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForFC", *mpCommandTop, (uchar*)" 文法エラー(不明文字)" );
		}
	}
	return ret;
}
// 2010/03/01 yito UV-00866 機能カウンタ対応 end  .

// 2017/03/16 日立 森本郁 [MFP管理技術_デバイス連携]KRQ-XXX CSRA-DBMS連携 Start.
//概要：DBMSコマンド用の不揮発の問い合わせ.
//戻り値：不揮発アクセスの結果.
//前提：ホストから要求されたコマンド文字列の文法チェックは正常終了している。.
//作成者：日立 森本郁.
//レビュー：.
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//			2017/03/16	日立 森本郁 新規作成.
RDSE_NONVOLATILE_RESULT RDSC_Inquiry::InquiryNonVolatileForDBMS(
		uchar*			puc_UserDataBuffer,		// [OUT]	応答のユーザデータバッファ先頭アドレス.
		int32_t			sl_userDataBufferSize,	// [IN]		応答のユーザーデータバッファ最大サイズ[単位:byte].
		int32_t*		psl_WrittenSize,		// [OUT]	応答のユーザデータバッファに書き込んだサイズ[単位:byte].
// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 start.
//		DBAE_DataType	e_DBMSDataKind			// [IN]		DBMSテーブル種別.
		RDSE_CREATE_XML	e_CreateXmlKind			// [IN]		生成するXMLデータ種別.
// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 end.
		)
{
	// 戻り値の為の変数.
	RDSE_NONVOLATILE_RESULT	e_ret	= RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;
	
	int32_t		sl_WriteSize		= 0;										// 今回の書込み予定データサイズ.
// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 start.
//	ssize_t		ul_ReadDataSize		= 0;										// 読み込みファイルサイズ.
//	Bool		b_Ret				= False;									// DBMSデータ書込み結果.
//	STATUS		result				= ERROR;									// ファイル操作結果.
//	struct stat	st_stat = {};													// ファイル状態.
//	char sc_DBMSFilePath[RDSD_DBMS_RECORD_FILE_MAX_FILE_PATH] = {0};
	
//	EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForDBMS", (int32_t)e_DBMSDataKind, (uchar*)" start" );
	EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForDBMS", (int32_t)e_CreateXmlKind, (uchar*)" start" );
// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 end.
	
	// パラメータチェック.
	if( ( NULL == puc_UserDataBuffer ) ||
		( NULL == psl_WrittenSize ) ) {
		EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForDBMS", *mpCommandTop, (uchar*)"Invalid Parameter." );
		return RDSE_NONVOLATILE_RESULT_OTHER;								// エラー返却(パラメータ異常).
	}
	
	// DBMSへのレコード出力要求は初回のみ実施.
	if( 0 == mOffsetOfXmlFile ) {
		// コマンド名＋パラメータが書き込めるバッファサイズが残っているか？.
		if( 6 > sl_userDataBufferSize ) {			// 他処理の仕様に合わせた.
			return RDSE_NONVOLATILE_RESULT_OK_NEXT;
		}

// 2017/11/15 [Eagle-Low][RQ-710][WP-xxx]PDMデータフォーマット変更対応 日立 森本郁 start.
//		RDSC_DBMSCommand	c_DBMSCommand;									// DBMSレコードデータ取得クラス.
//		
//		memset( sc_DBMSFilePath, 0x00, sizeof( sc_DBMSFilePath ) );
//		
//		// DBMSからDBレコード情報を取得.
//		b_Ret = c_DBMSCommand.getDBMSData( e_DBMSDataKind, sc_DBMSFilePath );
//		
//		memcpy( m_sc_DbmsFilePath, sc_DBMSFilePath, sizeof( m_sc_DbmsFilePath ) );
//		
//		if( False == b_Ret ) {
//			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForDBMS", *mpCommandTop, (uchar*)"DBMS File is not Exist." );
//			return RDSE_NONVOLATILE_RESULT_OTHER;
//		}
//		m_DbmsData_fd = open( m_sc_DbmsFilePath, O_RDONLY, LIBD_SHMMODE );
//		
//		if( ERROR == m_DbmsData_fd ) {
//			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForDBMS", *mpCommandTop, (uchar*)"DBMS File Open Failure." );
//			return RDSE_NONVOLATILE_RESULT_OTHER;
//		}
//		
//		// DBMSレコードファイルサイズの取得.
//		result = fstat( m_DbmsData_fd, &st_stat );
//		
//		if( ERROR != result ) {
//			m_ul_DbmsFileSize = st_stat.st_size;
//		} else {
//			// 不正なファイルサイズの為、ファイル削除する.
//			close( m_DbmsData_fd );
//			remove( m_sc_DbmsFilePath );
//			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForDBMS", *mpCommandTop, (uchar*)"Invalid File Size." );
//			return RDSE_NONVOLATILE_RESULT_OTHER;
//		}
// 2017/11/15 [Eagle-Low][RQ-710][WP-279]PDMデータフォーマット変更対応 日立 森本郁 end.
		
		ushort us_roopCnt = 0;
// 2017/11/15 [Eagle-Low][RQ-710][WP-279]PDMデータフォーマット変更対応 日立 森本郁 start.
//		ushort	us_roopCntMax	= sizeof( g_st_DBMSCommandInfo ) / sizeof( RDSS_DMBSCommandInfo );
		
//		for( us_roopCnt = 0; us_roopCnt < us_roopCntMax; us_roopCnt++ ){
		for( us_roopCnt = 0; us_roopCnt < g_us_DbmsCommandInfoTableSize; us_roopCnt++ ){
			// バッファに書き込むためのコマンド文字列が格納されている要素数を検索する.
//			if( e_DBMSDataKind == g_st_DBMSCommandInfo[us_roopCnt].m_e_DbmsDataKind ){
			if( e_CreateXmlKind == g_st_DBMSCommandInfo[us_roopCnt].m_e_CreateXMLKind ){
				break;
			}
		}
		
		// 対象の構造体を初期化.
		memset( &mstXmlData[meCreateXMLType], 0, sizeof(mstXmlData[0]) );
		
		// XML生成を行う.
		if( RDSG_GetXML( &mstXmlData[meCreateXMLType], meCreateXMLType ) )
		{	// 成功.
			EPN_DBG_MESSAGE( (uchar*)"RDSG_GetXML", (int32_t)meCreateXMLType, (uchar*)" OK" );	
			EPN_DBG_MESSAGE( (uchar*)"mstXmlData[meCreateXMLType] :", (int32_t)mstXmlData[meCreateXMLType].l_DataSize, (uchar*)" " );	
		}
		else
		{	// 失敗.
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_OTHER1" );
			EPN_DBG_MESSAGE( (uchar*)"RDSG_GetXML", (int32_t)meCreateXMLType, (uchar*)" NG" );	
			return RDSE_NONVOLATILE_RESULT_OTHER;	// エラーを返却（XML用領域確保に失敗）.
		}
// 2017/11/15 [Eagle-Low][RQ-710][WP-279]PDMデータフォーマット変更対応 日立 森本郁 end.
		
		char sc_CommandHeader[RDSD_CSRA_COMMAND_HEADER_LENGTH + 1];
		
		memset( sc_CommandHeader, 0x00, sizeof( sc_CommandHeader ) );
		snprintf( sc_CommandHeader, RDSD_CSRA_COMMAND_HEADER_LENGTH + 1, "%s00", g_st_DBMSCommandInfo[us_roopCnt].m_sc_CommandName );
		
		// コマンド名書込み処理.
		strncpy( reinterpret_cast<char*>(puc_UserDataBuffer), sc_CommandHeader, RDSD_CSRA_COMMAND_HEADER_LENGTH + 1	 );
		
		// ユーザーデータバッファ書込み開始アドレスと、書込み可能バッファサイズの値を更新.
		*psl_WrittenSize = 5;
		puc_UserDataBuffer += (*psl_WrittenSize);
		sl_userDataBufferSize -= (*psl_WrittenSize);
	}
	
// 2017/11/15 [Eagle-Low][RQ-710][WP-279]PDMデータフォーマット変更対応 日立 森本郁 start.
//	// 書込みデータサイズの算出.
//	if( sl_userDataBufferSize < ( m_ul_DbmsFileSize - mOffsetOfXmlFile ) ){		// 応答ユーザーデータ最大バッファサイズ分書き込む.
//		sl_WriteSize = sl_userDataBufferSize;
//	} else {																	// 残データ全て書き込む.
//		sl_WriteSize = m_ul_DbmsFileSize - mOffsetOfXmlFile;
//	}
//	
//	// 書込みデータサイズ分ファイル読込み.
//	ul_ReadDataSize = read( m_DbmsData_fd, reinterpret_cast<char*>(puc_UserDataBuffer), sl_WriteSize );
//	
//	if( ERROR == ul_ReadDataSize ) {
//		// ファイル読込みに失敗したため、ファイル削除し、DBMS関連データの初期化を行う.
//		m_ul_DbmsFileSize	= 0;
//		close( m_DbmsData_fd );
//		remove( m_sc_DbmsFilePath );
//		EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForDBMS", *mpCommandTop, (uchar*)"DBMS File Read Failure." );
//		return RDSE_NONVOLATILE_RESULT_OTHER;
//	}
//	
//	// 書込みサイズ更新.
//	*psl_WrittenSize += ul_ReadDataSize;
//	
//	// 読み込み位置更新.
//	mOffsetOfXmlFile += ul_ReadDataSize;
	// データコピー.
	if( sl_userDataBufferSize < (int32_t)(mstXmlData[meCreateXMLType].l_DataSize - mOffsetOfXmlFile))		// 2010/11/05 higuchi Minerva warning 対応.
	{	// バッファサイズ分コピー.
		sl_WriteSize = sl_userDataBufferSize;
	}
	else
	{	// 全てコピー.
		sl_WriteSize = mstXmlData[meCreateXMLType].l_DataSize - mOffsetOfXmlFile;
	}
	memcpy( puc_UserDataBuffer, mstXmlData[meCreateXMLType].ptr + mOffsetOfXmlFile, sl_WriteSize );
	
	// 予約文字を 規定文字に変換.
// 2023/02/08 FPT-TinhNT16 【IT6_2.3】【RQ-941】【OP_BTS-41860】 Start.
//	RDSC_CommandMatrix::ConvertReservedToRegulated( puc_UserDataBuffer, sl_WriteSize );
	RDSC_CommandMatrix::ConvertRegulatedToReservedForZO( puc_UserDataBuffer, sl_WriteSize );
// 2023/02/08 FPT-TinhNT16 【IT6_2.3】【RQ-941】【OP_BTS-41860】 End.
	
	// 書込みサイズ更新.
	*psl_WrittenSize = *psl_WrittenSize + sl_WriteSize;
	
	// 読み込み位置更新.
	mOffsetOfXmlFile = mOffsetOfXmlFile + sl_WriteSize;
	
	// DBMSコマンド残データ判定処理.
//	if( mOffsetOfXmlFile < m_ul_DbmsFileSize ){								// DBMSコマンド残データ有(次回続きのバッファから書込み実施).
	if( (int32_t)mOffsetOfXmlFile < mstXmlData[meCreateXMLType].l_DataSize ){	// DBMSコマンド残データ有(次回続きのバッファから書込み実施).	
// 2017/11/15 [Eagle-Low][RQ-710][WP-279]PDMデータフォーマット変更対応 日立 森本郁 end.
		// 正常（残りデータ）.
		e_ret = RDSE_NONVOLATILE_RESULT_OK_NEXT;
	} else {																	// DBMSコマンド算データ無(CSRC未処理コマンド有無判定へ).
		// XMLデータの初期化、ファイル削除.
		mOffsetOfXmlFile	= 0;
// 2017/11/15 [Eagle-Low][RQ-710][WP-279]PDMデータフォーマット変更対応 日立 森本郁 start.
//		m_ul_DbmsFileSize	= 0;
//		close( m_DbmsData_fd );
//		remove( m_sc_DbmsFilePath );
		// XMLデータの開放＆初期化を行う.
		free( mstXmlData[meCreateXMLType].ptr);
		memset( &mstXmlData[meCreateXMLType], 0, sizeof(mstXmlData[0]) );
// 2017/11/15 [Eagle-Low][RQ-710][WP-279]PDMデータフォーマット変更対応 日立 森本郁 end.
		
		// コマンド読込み位置を更新(パラメータはないものと想定).
		mpCommandTop += 3;
		
		// ' [半角スペース]'または"終端"を期待.
		if (*mpCommandTop == CodeOfSpace){									// ' [半角スペース]'.
			// 正常.
			mStatus = STATUS_NEXT_COMMAND;
			e_ret = RDSE_NONVOLATILE_RESULT_OK_NEXT;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForDBMS", *mpCommandTop, (uchar*)" 空白" );
		} else if (*mpCommandTop == CodeOfComma) {							// ',[カンマ]'.
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR14" );
			e_ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForDBMS", *mpCommandTop, (uchar*)" カンマ" );
		} else if(mpCommandTop == mpCommandEnd) {							// 終端.
			// 正常（ユーザデータ文字列終端を検出）.
			mStatus = STATUS_NORMAL_END;
			e_ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForDBMS", *mpCommandTop, (uchar*)" 終端" );
		} else if (mpCommandTop > mpCommandEnd) {							// 終端越え.
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR15" );
			e_ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForDBMS", *mpCommandTop, (uchar*)" 文法エラー(終端越え)" );
		} else {															// 予期しない文字.
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR16" );
			e_ret = RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatileForDBMS", *mpCommandTop, (uchar*)" 文法エラー(不明文字)" );
		}
	}
	
	return e_ret;
}
// 2017/03/16 日立 森本郁 [MFP管理技術_デバイス連携]KRQ-XXX CSRA-DBMS連携 End.

//概要：不揮発の問い合わせ
//　　　要求されたユーザデータに従って、不揮発を読み出し 応答のユーザデータを作成する。
//戻り値：不揮発アクセスの結果
//前提：初回の応答のユーザデータを作成前にInit関数を呼び出す。
//　　　ホストから要求されたコマンド文字列の文法チェックは正常終了している。
//備考：問い合わせデータの中に予約語が含まれる場合、変換する。
//　　　Telephonyの場合
//　　　応答のユーザデータの最大サイズに入らなかったデータは、連送する。CR/LFは書き込まない。
//　　　Inquiry／ModifyReadで共通して使用する。
//　　　応答電文１のユーザデータを作成する。
//作成者：宮本
//レビュー：2003/11/27、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成
//			2004/04/05、宮本、DXコマンド対応
//			2007/09/20、佐久間、コマンド途中から1コマンドを処理する必要がある場合はDXコマンドの処理に移行しない 
//　　　　　　　　　　　　　　　（問題点No.84901対応） 
//			2008/03/12 許、Donauセキュリティ強化モードで遠隔データ書き換え/取得の抑制対応

	RDSE_NONVOLATILE_RESULT
RDSC_Inquiry::InquiryNonVolatile(
		unsigned char* pUserDataBuffer, // [OUT] 応答のユーザデータバッファ先頭アドレス
		int32_t userDataBufferSize,		// [IN]  応答のユーザデータ最大サイズ[単位:byte]
		int32_t* pWrittenSize,				// [OUT] 応答のユーザデータバッファに書き込んだサイズ[単位:byte]
// 2017/6/27 MAv5 CSRA接続 start.
		std::string* p_c_CommandParamCheckError	// [OUT] パラメータチェックでエラーとなったコマンド文字列（区切り文字なし 例：「A0A1」） .
// 2017/6/27 MAv5 CSRA接続 end.
		)
{
		EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatile", (uint32_t)0, (uchar*)"start" );

#if !defined(CSRC_INVADIDATE_DX)
	//注意：userDataBufferSizeを減じる前に 部門管理のチェックを行う必要がある
	//部門管理データ用コマンド専用チェック
	if (IsGoodDX(mpCommandTop, (int32_t)(mpCommandEnd - mpCommandTop)))		// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning	
	{
		// 2011/06/30 yito CQ349199 期日指定発信でのDX取得 start.
		// デバイスカウンタの取得条件の確認(UV-0884対応).
		if( False == isEnableStatus_ReadDX() )
		{
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatile", (uint32_t)0, (uchar*)"DX:RDSE_SYNTAX_RESULT_UNMATCH" );
			return RDSE_SYNTAX_RESULT_UNMATCH;
		}
		// 2011/06/30 yito CQ349199 期日指定発信でのDX取得 end.

		// 07/09/20 佐久間 コマンド途中から1コマンドを処理する必要がある場合はDXコマンドの処理に移行しない 
		if(  ( mStatus != STATUS_HALFCOMMAND_NOPARAM ) && ( mStatus != STATUS_HALFCOMMAND_PARAM ) ){
		//DXコマンド用の不揮発の問い合わせ
		return InquiryNonVolatileForDX(
			pUserDataBuffer,		// [OUT] 応答のユーザデータバッファ先頭アドレス
			userDataBufferSize,		// [IN]  応答のユーザデータ最大サイズ[単位:byte]
			pWrittenSize);			// [OUT] 応答のユーザデータバッファに書き込んだサイズ[単位:byte]
		}
	}
#endif	//end of #if !defined(CSRC_INVADIDATE_DX)
// 2018/07/03 HCLT-Vinay St.
	if (IsGoodZP(mpCommandTop, (int32_t)(mpCommandEnd - mpCommandTop)))	// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning	
	{
		// 2011/06/30 yito CQ349199 期日指定発信でのDX取得 start.
		// デバイスカウンタの取得条件の確認(UV-0884対応).
		if( False == isEnableStatus_ReadZP() )
		{
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatile", (uint32_t)0, (uchar*)"ZP:RDSE_SYNTAX_RESULT_UNMATCH" );
			return RDSE_SYNTAX_RESULT_UNMATCH;
		}
		// 2011/06/30 yito CQ349199 期日指定発信でのDX取得 end.

		// 07/09/20 佐久間 コマンド途中から1コマンドを処理する必要がある場合はDXコマンドの処理に移行しない 
		if(  ( mStatus != STATUS_HALFCOMMAND_NOPARAM ) && ( mStatus != STATUS_HALFCOMMAND_PARAM ) ){
		//DXコマンド用の不揮発の問い合わせ
		return InquiryNonVolatileForZP(
			pUserDataBuffer,		// [OUT] 応答のユーザデータバッファ先頭アドレス
			userDataBufferSize,		// [IN]  応答のユーザデータ最大サイズ[単位:byte]
			pWrittenSize);			// [OUT] 応答のユーザデータバッファに書き込んだサイズ[単位:byte]
		}
	}
// 2018/07/03 HCLT-Vinay Ed.

// 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Start.
	if (IsGoodZV(mpCommandTop, (int32_t)(mpCommandEnd - mpCommandTop))) {
		// コマンド途中から1コマンドを処理する必要がある場合はZVコマンドの処理に移行しない.
		if(( mStatus != STATUS_HALFCOMMAND_NOPARAM ) && ( mStatus != STATUS_HALFCOMMAND_PARAM )) {
		// ZVコマンド用の不揮発の問い合わせ.
			return InquiryNonVolatileForZV(
				pUserDataBuffer,				// [OUT] 応答のユーザデータバッファ先頭アドレス.
				userDataBufferSize,				// [IN]  応答のユーザデータ最大サイズ[単位:byte].
				pWrittenSize);					// [OUT] 応答のユーザデータバッファに書き込んだサイズ[単位:byte].
		}
	}
// 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 End.

// 2016/05/12 H.Tokuno ZeusSBK 制御構成変更_No.57 カーネルログ取得による市場問題検証対応_Step2 Start.
	// ZHコマンドの場合はこちらへ.
	if (IsGoodZH(mpCommandTop))
	{
		// ZHパラメータを取得.
		// パラメータが付与されていない場合は00が付与されているとみなす.
		int nValue = RDSD_ZH_HAS_NO_PARAM;

		Bool	bIsGood = False;
		const	unsigned char*	p = mpCommandTop;
		// " ZHXX"という形で来たものとみなす.
		p += 3;
		int temp = RDSC_CommandMatrix::Hex2Toi(p, &bIsGood);
		if( True == bIsGood )
		{
			nValue = temp;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatile", (uint32_t)nValue, (uchar*)"ZH has a param." );
		}

		// コマンド途中から1コマンドを処理する必要がある場合はZHコマンドの処理に移行しない.
		if(  ( mStatus != STATUS_HALFCOMMAND_NOPARAM ) && ( mStatus != STATUS_HALFCOMMAND_PARAM ) ){
		// ZHコマンド用の不揮発の問い合わせ.
		return InquiryNonVolatileForZH(
			pUserDataBuffer,		// [OUT] 応答のユーザデータバッファ先頭アドレス.
			userDataBufferSize,		// [IN]  応答のユーザデータ最大サイズ[単位:byte].
			pWrittenSize,			// [OUT] 応答のユーザデータバッファに書き込んだサイズ[単位:byte].
			nValue);				// [IN]  ZHコマンドに付与されていたパラメタ.
		}
	}
// 2016/05/12 H.Tokuno ZeusSBK 制御構成変更_No.57 カーネルログ取得による市場問題検証対応_Step2 End.

// 2017/02/06 H.Tokuno IT5_4.1 機器構成変更_No.49 CTLログの容量拡張 Start.
	// ZMコマンドの場合はこちらへ.
	if (IsGoodZM(mpCommandTop))
	{
		// ZMパラメータを取得.
		// パラメータが付与されていない場合は00が付与されているとみなす.
		int nValue = RDSD_ZM_HAS_NO_PARAM;

		Bool	bIsGood = False;
		const	unsigned char*	p = mpCommandTop;
		// " ZMXX"という形で来たものとみなす.
		p += 3;
		int temp = RDSC_CommandMatrix::Hex2Toi(p, &bIsGood);
		if( True == bIsGood )
		{
			nValue = temp;
			EPN_DBG_MESSAGE( (uchar*)"InquiryNonVolatile", (uint32_t)nValue, (uchar*)"ZM has a param." );
		}

		// コマンド途中から1コマンドを処理する必要がある場合はZMコマンドの処理に移行しない.
		if(  ( mStatus != STATUS_HALFCOMMAND_NOPARAM ) && ( mStatus != STATUS_HALFCOMMAND_PARAM ) ){
		// ZMコマンド用のHDDの問い合わせ.
		return InquiryNonVolatileForZM(
			pUserDataBuffer,		// [OUT] 応答のユーザデータバッファ先頭アドレス.
			userDataBufferSize,		// [IN]  応答のユーザデータ最大サイズ[単位:byte].
			pWrittenSize,			// [OUT] 応答のユーザデータバッファに書き込んだサイズ[単位:byte].
			nValue);				// [IN]  ZMコマンドに付与されていたパラメタ.
		}
	}
// 2017/02/06 H.Tokuno IT5_4.1 機器構成変更_No.49 CTLログの容量拡張 End.

// 2010/03/01 yito UV-00866 機能カウンタ対応 start.
	if( True == IsGoodFuncCounterCommand(mpCommandTop, (int32_t)(mpCommandEnd - mpCommandTop)))		// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning	
	{
		// コマンド途中から1コマンドを処理する必要がある場合は機能カウンタコマンドの処理に移行しない.
		if(  ( mStatus != STATUS_HALFCOMMAND_NOPARAM ) && ( mStatus != STATUS_HALFCOMMAND_PARAM ) )
		{
			return InquiryNonVolatileForFuncCount(
				pUserDataBuffer,		// [OUT] 応答のユーザデータバッファ先頭アドレス.
				userDataBufferSize,		// [IN]  応答のユーザデータ最大サイズ[単位:byte].
				pWrittenSize			// [OUT] 応答のユーザデータバッファに書き込んだサイズ[単位:byte].
				);
		}
	}
// 2010/03/01 yito UV-00866 機能カウンタ対応 end  .

// 2017/11/15 [Eagle-Low][RQ-710][WP-279]PDMデータフォーマット変更対応 日立 森本郁 start.
// 2017/03/16 日立 森本郁 [MFP管理技術_デバイス連携]KRQ-XXX CSRA-DBMS連携 Start.
//	DBAE_DataType	e_DBMSDataKind = DBAE_OtherType;
//	
//	// DBMS系コマンドチェック処理.
//	if( True == IsGoodDBMSCommand( mpCommandTop, &e_DBMSDataKind ) ) {
//		// コマンド途中から1コマンドを処理する必要がある場合はDBMSコマンドの処理に移行しない.
//		if(  ( mStatus != STATUS_HALFCOMMAND_NOPARAM ) && ( mStatus != STATUS_HALFCOMMAND_PARAM ) ) {
//			return InquiryNonVolatileForDBMS(
//				pUserDataBuffer,							// [OUT] 応答のユーザデータバッファ先頭アドレス.
//				static_cast<int32_t>(userDataBufferSize),	// [IN]  応答のユーザデータ最大サイズ[単位:byte].
//				reinterpret_cast<int32_t*>(pWrittenSize),	// [OUT] 応答のユーザデータバッファに書き込んだサイズ[単位:byte].
//				e_DBMSDataKind								// [IN]  DBMS取得対象レコード種別.
//				);
//		}
//	}
// 2017/03/16 日立 森本郁 [MFP管理技術_デバイス連携]KRQ-XXX CSRA-DBMS連携 End.
	RDSE_CREATE_XML	e_CreateXmlKind = RDSE_CREATE_XML_MAX;
	
	// DBMS系コマンドチェック処理.
	if( True == IsGoodDBMSCommand( mpCommandTop, &e_CreateXmlKind ) ) {
		// コマンド途中から1コマンドを処理する必要がある場合はDBMSコマンドの処理に移行しない.
		if(  ( mStatus != STATUS_HALFCOMMAND_NOPARAM ) && ( mStatus != STATUS_HALFCOMMAND_PARAM ) ) {
			return InquiryNonVolatileForDBMS(
				pUserDataBuffer,							// [OUT] 応答のユーザデータバッファ先頭アドレス.
				userDataBufferSize,							// [IN]  応答のユーザデータ最大サイズ[単位:byte].
				pWrittenSize,								// [OUT] 応答のユーザデータバッファに書き込んだサイズ[単位:byte].
				e_CreateXmlKind								// [IN]  生成するXMLデータ種別.
				);
		}
	}
// 2017/11/15 [Eagle-Low][RQ-710][WP-279]PDMデータフォーマット変更対応 日立 森本郁 end.

	if (userDataBufferSize != 0) userDataBufferSize--;	//低位関数が文字列終端'\0'を書き込むのでサイズを減じる。

	Bool	bIsAbleToWrite = True;			// 書き込み継続可能？
	Bool	bInLoop = True;
	unsigned char* p = pUserDataBuffer;		// １関数が処理するバッファの先頭アドレス
	unsigned char* const	pUserDataBufferEnd = pUserDataBuffer + userDataBufferSize;		// 書き込みバッファの終端アドレス
	RDSE_NONVOLATILE_RESULT	ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;

	while (bInLoop)
	{
// 2017/08/07 MAv5 エラーチェック機構 start.
		Bool b_ResultParamCheck = True;
// 2017/08/07 MAv5 エラーチェック機構 end.

		RDSD_DBG_MESSAGE_CMD_STR(" status =", mStatus, 0, 0 );

		//内部状態に合わせて １コマンドを処理する
		switch (mStatus)
		{
		default:
//		case STATUS_NORMAL_END:			// 終了
			bInLoop = False;
			ret = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;
			break;
		case STATUS_NEXT_COMMAND:		// 次のコマンドから処理再開［未終了］
			//概要：コマンドの先頭から１コマンドを処理する
			//		リードポインタから、１コマンドを書ける所まで書く。
			//		バッファにはコマンド文字は書いてない。
			//		パラメータ有無確認後、どちらかのサブ関数を呼び出す。
// 2017/08/07 MAv5 エラーチェック機構 start.
//			ret = 	ProcessCommandAtTop(&bIsAbleToWrite, p, pUserDataBufferEnd);
			ret = 	ProcessCommandAtTop(&bIsAbleToWrite, p, &b_ResultParamCheck, pUserDataBufferEnd);
// 2017/08/07 MAv5 エラーチェック機構 end.
			break;
		case STATUS_HALFCOMMAND_PARAM:	// コマンドの途中から処理再開（パラメータ指定有）［未終了］
			//概要：コマンドの途中から１コマンドを処理する（パラメータ指定有）
			//		メンバ変数のコマンドと、リードポインタから、１コマンドを書ける所まで書く。
			//		バッファにはコマンド文字は書いてある。
			ret = 
			ProcessCommandAtHalfWithParam(&bIsAbleToWrite, p, pUserDataBufferEnd);
			break;
		case STATUS_HALFCOMMAND_NOPARAM:	// コマンドの途中から処理再開（パラメータ指定無）［未終了］
			//概要：コマンドの途中から１コマンドを処理する（パラメータ指定無）
			//		メンバ変数のコマンドと、次のパラメータ値から、１コマンドを書ける所まで書く。
			//		バッファにはコマンド文字は書いてある。
			//戻り値：不揮発アクセスの結果
			//備考：
// 2017/08/07 MAv5 エラーチェック機構 start.
//			ret = ProcessCommandAtHalfWithoutParam(&bIsAbleToWrite, p, pUserDataBufferEnd);
			ret = ProcessCommandAtHalfWithoutParam(&bIsAbleToWrite, p, &b_ResultParamCheck, pUserDataBufferEnd);
// 2017/08/07 MAv5 エラーチェック機構 end.
			break;
		}

		RDSD_DBG_MESSAGE_CMD_STR(" result =", ret, bIsAbleToWrite, mStatus );

		switch (ret)
		{
		case RDSE_NONVOLATILE_RESULT_OK_NO_NEXT:		// 正常（残りデータ無）
		case RDSE_NONVOLATILE_RESULT_OK_NEXT:			// 正常（残りデータ有）
			if (!bIsAbleToWrite)
			{
				bInLoop = False;
			}
			break;
		default:
		//以下はコマンドRead/Write関数の検出エラー
		//case RDSE_NONVOLATILE_RESULT_SYNTAXERR:			// 文法エラー（内部矛盾）
		//case RDSE_NONVOLATILE_RESULT_READ_ERR:			// エラーで読めない（メカコン系データ）
		//case RDSE_NONVOLATILE_RESULT_INVALID_ACCESS:	// データが無い（コマンド不正、パラメータ不正）
		//case RDSE_NONVOLATILE_RESULT_WRITE_ERR_TO_RONLY:// ReadOnlyに対する書き込みを行おうとした
		//case RDSE_SYNTAX_RESULT_HIGHER_SECURITY_MODE セキュリティ強化時にIISWに関する情報(TI/THコマンド)のデータ要求を行った.
			bInLoop = False;
			APIC_ThrowSystemError(
						SYSE_SysErrTypeRecoverableLogicalError,         // 内部論理矛盾(復旧可能なもの)
						LOGF_DEBUG_EVENT_KIND_RDS, 
						"(E)コマンドRead/Write関数の検出エラー");
			break;
		}

// 2017/08/07 MAv5 エラーチェック機構 start.
		if( False == b_ResultParamCheck ) {
			// mCommandはNULL終端の文字列でないので注意.
			if( NULL != p_c_CommandParamCheckError ) {
				p_c_CommandParamCheckError->append( (const char*)mCommand, (size_t)sizeof(mCommand) );
			}
		}
// 2017/08/07 MAv5 エラーチェック機構 end.

	}//end of while (bInLoop)

	//書き込んだサイズの設定
	switch (ret)
	{
	case RDSE_NONVOLATILE_RESULT_OK_NO_NEXT:		// 正常（残りデータ無）
	case RDSE_NONVOLATILE_RESULT_OK_NEXT:			// 正常（残りデータ有）
		*pWrittenSize = (int32_t)(p - pUserDataBuffer);		// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
		break;
	default:
		//異常検出の場合、書き込みサイズは０にする
		*pWrittenSize = 0;
		break;
	}
	return ret;
}



// 2016/05/12 H.Tokuno ZeusSBK 制御構成変更_No.57 カーネルログ取得による市場問題検証対応_Step2 Start.

RDSC_Inquiry::RDS_FileSort RDSC_Inquiry::mDmsgFileSortArray[RDSD_ZH_DMSG_FILE_MAX_NUM] = {{{0}}};

//概要  ：/ata01/KernelLog配下にあるファイルでN番目に新しいファイルを取得する. //20180328  【IT6_1.0】【Eagle】【制御構成変更_No.99】HCLT-Rajeshwari Modified.
//戻り値：short 関数終了時の原因.
//        RESULT_DMSG_FILE_NOT_FOUND	-1	// ファイルが存在しなかった.
//        RESULT_ANOTHER_ERROR			 0	// ファイルが存在しない場合以外のエラー.
//        RESULT_NORMAL_FINISH			 1	// 正常終了.
//前提  ：ZHコマンドでしか使用しない.
//引数  ：uchar			us_fileIndex      : 取得ファイルのIndex.
//        char *		pc_OutputFilePath : 該当ファイルのパス、コールする側で255+1の領域を確保していること.
//作成者：H.Tokuno.
//レビュー：
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//			2016/05/18、H.Tokuno、新規作成.
	RDSE_GET_DMSGPATH_RESULT
RDSC_Inquiry::getDmsgFilePath( ushort us_fileIndex, char *pc_OutputFilePath )
{
	DIR 			*pDir = NULL;
	ushort			us_fileCount = 0;
	struct dirent	*pDirent = NULL;
	struct stat 	status = {0};
	int 			len = 0;
	char			sc_FullPath[PATH_MAX_KM+1] = {0};
	FILC_BaseDirIf	*pBaseDir = NULL;
	char 			*pDirPath = NULL;
	RDS_FileSort	st_FileSortTmp = {{0}};
	unsigned char	uc_SortCount = 0;


	if(pc_OutputFilePath == NULL) {
		EPN_DBG_MESSAGE( (uchar*)"RDS_getFilePath", (short)0, (uchar*)"pc_OutputFilePath is NULL" );
		return RDSE_GET_DMSGPATH_RESULT_ERR_OHTER;
	}

	// ディレクトリパス取得.
	pBaseDir = FILC_BaseDirIf::newInstance(FILE_DirTypeDebugLog);	//2018/06/25 【IT6_1.0】【EAGLE】【制御構成変更_No.110】HCLT-Jahnavi Modified.
	if(pBaseDir != NULL) {
		pDirPath = (char*)pBaseDir->getPath();
		if(pDirPath == NULL) {
			EPN_DBG_MESSAGE( (uchar*)"RDS_getFilePath", (short)0, (uchar*)"Can't get Directory path." );
			return RDSE_GET_DMSGPATH_RESULT_ERR_OHTER;
		}
		
		EPN_DBG_MESSAGE( (uchar*)"RDS_getFilePath:pDirPath", (short)0, (uchar*)pDirPath );
	}
	else{
		EPN_DBG_MESSAGE( (uchar*)"RDS_getFilePath", (short)0, (uchar*)"Can't create FILC_BaseDirIf instance." );
		return RDSE_GET_DMSGPATH_RESULT_ERR_OHTER;
	}

	// ディレクトリに存在するファイル数と要求Indexを比較.
	// 存在ファイル数以上を指定した場合はエラーとする.
 	us_fileCount = FILF_getFileCount(pDirPath);
	EPN_DBG_MESSAGE( (uchar*)"RDS_getFilePath", (short)us_fileCount, (uchar*)"file count." );
	EPN_DBG_MESSAGE( (uchar*)"RDS_getFilePath", (short)us_fileIndex, (uchar*)"file index." );
	if ( us_fileCount == 0 )					// ファイルが1つも存在していない.
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_getFilePath", (short)0, (uchar*)"No dmsg file exists." );
		return RDSE_GET_DMSGPATH_RESULT_ERR_FILE_NOTFOUND;
	}
 	else if ( us_fileCount < us_fileIndex + 1 )		// 要求Indexの方が大きい?.
	{
 		EPN_DBG_MESSAGE( (uchar*)"RDS_getFilePath", (short)0, (uchar*)"The file selected with this index isn't found." );
		return RDSE_GET_DMSGPATH_RESULT_ERR_FILE_NOTFOUND;
	}
	else
	{
		if(( pDir = FILF_Opendir(pDirPath)) == NULL )
		{
			EPN_DBG_MESSAGE( (uchar*)"RDS_getFilePath", (short)0, (uchar*)"Directory open has been failed." );
			return RDSE_GET_DMSGPATH_RESULT_ERR_OHTER;
		}
		else
		{
			// 配列の初期化.
			memset(mDmsgFileSortArray, 0x00, sizeof(mDmsgFileSortArray));

			strncpy(sc_FullPath, pDirPath, PATH_MAX_KM);
			len = (int)strlen(sc_FullPath) ;	// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
			if( sc_FullPath[len-1] != '/' )
			{
				sc_FullPath[len] = '/';
				sc_FullPath[len+1] = 0;
				len++;
			}

			//  ディレクトリエントリのリード.
			while( (pDirent = FILF_Readdir(pDir)) )
			{
				// カレントディレクトリと親ディレクトリは対象外.
				if( (strncmp( pDirent->d_name, ".", 1 ) == 0 && strlen( pDirent->d_name ) == 1)	// カレントディレクトリ.
				 || strncmp( pDirent->d_name, "..", 2 ) == 0 ) {								// 親ディレクトリ.
					// 次のエントリリード.
					continue;
				}

				// ファイルパス長がPATH_MAX_KMを超えてしまう場合は対象外とする.
				if (len+strlen(pDirent->d_name) > PATH_MAX_KM){
					continue;
				}
				// 2020/2/26 y.hotta OP_BTS-21363 SIMでZH投入されるとトラブルE304が発生することがある Modify Start.
				//strncpy( &sc_FullPath[len], pDirent->d_name, sizeof(pDirent->d_name) ) ;
				strncpy( &sc_FullPath[len], pDirent->d_name, sizeof(sc_FullPath) - len ) ;	// 書込み開始位置をずらした分、書込みサイズを減らす.
				// 2020/2/26 y.hotta OP_BTS-21363 SIMでZH投入されるとトラブルE304が発生することがある Modify End.

				/* ステータスの取得 */
				if( FILF_Stat( sc_FullPath, &status ) != -1 )
				{
					if( S_ISREG( status.st_mode ) )			// ファイル?.
					{
						// 配列に格納.
						strncpy(mDmsgFileSortArray[uc_SortCount].sc_SortFilePath, sc_FullPath, sizeof(sc_FullPath)-1);   //[AR : 1026431] 【 Eagle 】【静的解析対応用課題】 2017/11/28 [HCL_Raghu] 
						mDmsgFileSortArray[uc_SortCount].sl_time = (int32_t)status.st_mtime;	// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
						uc_SortCount++;
					}
				}
				
				// 念のため、配列格納数が30に達したら、whileループを抜けるようにしておく.
				if( RDSD_ZH_DMSG_FILE_MAX_NUM <= uc_SortCount )
				{
					break;
				}
			}
			FILF_Closedir( pDir ) ;
			
			// 配列をファイルが新しい順にソート.
			for(uchar us_Count1 = 0; us_Count1 < (uc_SortCount - 1); us_Count1++){				// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
				for (uchar us_Count2 = us_Count1 + 1; us_Count2 < uc_SortCount; us_Count2++){		// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
					if (mDmsgFileSortArray[us_Count1].sl_time < mDmsgFileSortArray[us_Count2].sl_time){
						st_FileSortTmp = mDmsgFileSortArray[us_Count1];
						mDmsgFileSortArray[us_Count1] = mDmsgFileSortArray[us_Count2];
						mDmsgFileSortArray[us_Count2] = st_FileSortTmp;
					}
				}
			}

			// 指定番目のファイルパスを返却.
			memcpy(pc_OutputFilePath, mDmsgFileSortArray[us_fileIndex].sc_SortFilePath, sizeof(mDmsgFileSortArray[us_fileIndex].sc_SortFilePath));
			EPN_DBG_MESSAGE( (uchar*)"RDS_getFilePath : pc_OutputFilePath", (short)0, (uchar*)pc_OutputFilePath);
		}
	}
	return RDSE_GET_DMSGPATH_RESULT_OK;
}
// 2016/05/12 H.Tokuno ZeusSBK 制御構成変更_No.57 カーネルログ取得による市場問題検証対応_Step2 End.



// 2017/02/06 H.Tokuno IT5_4.1 機器構成変更_No.49 CTLログの容量拡張 Start.
//概要  ：汎用ログディレクトリ配下にあるエンジンログのファイルパスを取得する.
//戻り値：short 関数終了時の原因.
//        RDSE_GET_ENGLOGPATH_RESULT_ERR_FILE_NOTFOUND	-1	// ファイルが存在しなかった.
//        RDSE_GET_ENGLOGPATH_RESULT_ERR_OHTER			 0	// ファイルが存在しない場合以外のエラー.
//        RDSE_GET_ENGLOGPATH_RESULT_OK					 1	// 正常終了.
//前提  ：ZMコマンドでしか使用しない.
//引数  ：ushort			us_Param      : 指定パラメタ.
//        char *		pc_OutputFilePath : 該当ファイルのパス、コールする側で255+1の領域を確保していること.
//作成者：H.Tokuno.
//レビュー：
//変更履歴：変更日、変更者、変更内容（変更理由）を記述.
//			2017/02/06、H.Tokuno、新規作成.
	RDSE_GET_ENGLOGPATH_RESULT
RDSC_Inquiry::getEngLogFilePath( ushort us_Param, char *pc_OutputFilePath )
{
	// FILのディレクトリインターフェイス.
	FILC_BaseDirIf	*pBaseDir		= NULL;
	// ディレクトリパス文字列.
	char 			*pDirPath		= NULL;
	// ファイル数.
	ushort			us_fileCount	= 0;
	// ファイル名の文字列.
	char			sc_FileName[RDSD_ZM_ENGLOG_FILE_NAME_MAX_LEN]		= {0};
	// ファイルステータス.
	struct stat 	status			= {0};
	// フルパス用文字列.
	char			sc_FullPath[PATH_MAX_KM+1] = {0};

	if(pc_OutputFilePath == NULL) {
		EPN_DBG_MESSAGE( (uchar*)"getEngLogFilePath", (short)0, (uchar*)"pc_OutputFilePath is NULL" );
		return RDSE_GET_ENGLOGPATH_RESULT_ERR_OHTER;
	}

	// HDD復帰開始処理が正常に行われていない場合、エラーを返す.
	APIC_HddInfo*	p_hddinfo	= APIC_HddInfo::newInstance();
	if( (p_hddinfo != NULL) && (p_hddinfo->getHddExist() == True) )
	{
		if( tskRdsEvent.isHDDAccessStart() == False )
		{
			EPN_DBG_MESSAGE( (uchar*)"getEngLogFilePath", (short)0, (uchar*)"HDD Access not started" );
			return RDSE_GET_ENGLOGPATH_RESULT_ERR_OHTER;
		}
	}

	// ディレクトリパス取得.
	pBaseDir = FILC_BaseDirIf::newInstance(FILE_DirTypeLog);
	if(pBaseDir != NULL) {
		pDirPath = (char*)pBaseDir->getPath();
		if(pDirPath == NULL) {
			EPN_DBG_MESSAGE( (uchar*)"getEngLogFilePath", (short)0, (uchar*)"Can't get Directory path." );
			return RDSE_GET_ENGLOGPATH_RESULT_ERR_OHTER;
		}
		
		EPN_DBG_MESSAGE( (uchar*)"getEngLogFilePath:pDirPath", (short)0, (uchar*)pDirPath );
	}
	else{
		EPN_DBG_MESSAGE( (uchar*)"getEngLogFilePath", (short)0, (uchar*)"Can't create FILC_BaseDirIf instance." );
		return RDSE_GET_ENGLOGPATH_RESULT_ERR_OHTER;
	}

	// ファイルが0個である場合は処理を行わない.
 	us_fileCount = FILF_getFileCount(pDirPath);
	if ( us_fileCount > 0 )
	{
		// ファイル名の作成.
		// ファイル名の通し番号は001～005.
		snprintf( sc_FileName, RDSD_ZM_ENGLOG_FILE_NAME_MAX_LEN, PRTD_ENGINE_FILE_NAME_TEMPLATE, (us_Param+1) );

		// 取得したディレクトリパスを、sc_FullPathにコピー.
		strncpy( sc_FullPath, pDirPath, PATH_MAX_KM );
		
		// 終端が'/'で無い場合、'/'を付与する.
		short len = (short)strlen(sc_FullPath) ;		// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
		if( len > 0 )
		{
			if( sc_FullPath[len-1] != '/' )
			{
				sc_FullPath[len] = '/';
				sc_FullPath[len+1] = '\0';
				len++;
			}
		}
		else
		{
			// フルパス用文字列の文字列長が0となっている場合.
			EPN_DBG_MESSAGE( (uchar*)"getEngLogFilePath", (short)0, (uchar*)"Directory path length is 0." );
			return RDSE_GET_ENGLOGPATH_RESULT_ERR_OHTER;
		}

		// ファイルパス長がPATH_MAX_KMを超えてしまう場合はエラーとする.
		if ( len + strlen(sc_FileName) > PATH_MAX_KM )
		{
			EPN_DBG_MESSAGE( (uchar*)"getEngLogFilePath", (short)0, (uchar*)"File path length is over the limit." );
			return RDSE_GET_ENGLOGPATH_RESULT_ERR_OHTER;
		}
		// フルパスの作成.
		// lenはディレクトリパス+'/'の長さ、そこからファイル名を書き込む.
		strncpy( &sc_FullPath[len], sc_FileName, sizeof(sc_FileName) ) ;

		// ファイルのフルパスからステータスの取得.
		if( FILF_Stat( sc_FullPath, &status ) != -1 )
		{
			// ファイルか否かを判定.
			if( S_ISREG( status.st_mode ) )			// ファイル?.
			{
				// 指定されたインデックスのファイルが存在する場合、フルパスの文字列を格納する.
				snprintf(pc_OutputFilePath, PATH_MAX_KM, "%s", sc_FullPath);
				EPN_DBG_MESSAGE( (uchar*)"getEngLogFilePath : pc_OutputFilePath", (short)0, (uchar*)pc_OutputFilePath);
				return RDSE_GET_ENGLOGPATH_RESULT_OK;
			}
		}
	}
	// ファイルが見つからなかった.
	EPN_DBG_MESSAGE( (uchar*)"getEngLogFilePath", (short)0, (uchar*)"File is not found." );
	return RDSE_GET_ENGLOGPATH_RESULT_ERR_FILE_NOTFOUND;
}

// 2017/02/06 H.Tokuno IT5_4.1 機器構成変更_No.49 CTLログの容量拡張 End.







//
// RDSC_Modify
//

//コンストラクタ
//作成者：宮本
//レビュー：2003/11/27、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成
RDSC_Modify::RDSC_Modify(
		const RDSC_CommandMatrix* pCommandMatrix):	// コマンドMatrixのアドレス	
		mrCommandMatrix(*pCommandMatrix)
{}

//CommandMatrixの初期化 
//作成者：浦崎 
//レビュー：
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2006/11/01、浦崎、新規作成
void RDSC_Modify::initCommandMatrix()
{
	// 【CommandMatrix 初期化タイミング変更】 06/11/01 浦崎 
	RDSC_CommandMatrix& Temp = const_cast<RDSC_CommandMatrix&>(mrCommandMatrix);
	Temp.initCommandMatrix();
}


//概要：１コマンドのチェック
//戻り値：コマンドチェックの結果
//備考：１コマンド・１パラメータ・１データ
//作成者：宮本
//レビュー：2003/11/27、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成
	RDSE_SYNTAX_RESULT
RDSC_Modify::CheckCommand(
		const unsigned char* const pCommand,	// [IN]  コマンド文字列
		int32_t commandSize,						// [IN]  コマンド文字列のサイズ
		const RDSS_CommandInfo*& rpCommandInfo,	// [OUT] コマンド情報のアドレスの参照(異常時は０が格納される)
		int32_t* pCheckedCount						// [OUT] チェックした文字数(異常時は０)
												//		 空白が続く場合は チェック済文字数に空白は含まなれない
		) const
{
	Bool	bIsGood = False;
	RDSE_SYNTAX_RESULT	result = RDSE_SYNTAX_RESULT_NO_DELIMITER;		//結果
	const unsigned char*	p = pCommand;
	const unsigned char*	pCommandEnd = p + commandSize;	// コマンド文字列の終端アドレス

	*pCheckedCount = 0;

	//空白を期待する。
	if ((*p++ != CodeOfSpace) || (commandSize < (1+2))) return RDSE_SYNTAX_RESULT_NO_DELIMITER;
	//コマンドを期待する
	rpCommandInfo = mrCommandMatrix.GetCommandInfo(p, &bIsGood);
	//不正コマンドなら、終了
	if (!bIsGood) return RDSE_SYNTAX_RESULT_SYNTAX_ERROR;
// 2012/11/02 CQ.374548 Sky忠政 start.
	RDSE_TaskID taskID = theRdsApl.isRdsSystem();
	// モデム通信の場合.
	if( taskID == RDSE_TSK_ID_TELL )
	{
		// セキュリティ強化モードの取得.
		// ONは取得NG.
		APIC_SystemSetting *pc_SystemSetting = APIC_SystemSetting::newInstance();
		pc_SystemSetting->lock();
		Bool b_Security = pc_SystemSetting->getSecurityLevelUpOn();
		pc_SystemSetting->unlock();
		if( True == b_Security )
		{
				// 受付条件を確認を満たさない場合、エラー.
				return RDSE_SYNTAX_RESULT_SYNTAX_ERROR;
		}
	}
// 2012/11/02 CQ.374548 Sky忠政 end.
	p += 2;

	//パラメータを期待
	//パラメータは２文字で、'0'～'9'、'A'～'F'
	int	nValue = RDSC_CommandMatrix::Hex2Toi(p, &bIsGood);
	//パラメータ値が不正
	if (!bIsGood) return RDSE_SYNTAX_RESULT_SYNTAX_ERROR;
	//最大値を越えている
	if (nValue > rpCommandInfo->mParamMax) return RDSE_SYNTAX_RESULT_SYNTAX_ERROR;
	p += 2;

	//ReadOnlyに対する書き込みは、エラー
	if (rpCommandInfo->mbIsReadOnly) return RDSE_SYNTAX_RESULT_UNCHANGEABLE_ITEM;

	//１つだけのデータを期待
	//' 'または"終端"まで アドレスを進める
	const unsigned char*	pDataTop = p;	//データ部の先頭アドレス
	Bool bInLoop = True;
	while (bInLoop)
	{
		if (*p == CodeOfSpace)	//空白
		{
			//正常
			result = RDSE_SYNTAX_RESULT_OK_SPACE;
			bInLoop = False;
		}
		else if (p == pCommandEnd)
		{
			//正常（ユーザデータ文字列終端を検出）
			result = RDSE_SYNTAX_RESULT_OK_END;
			bInLoop = False;
		}
// 2010/07/29 yito UV-00886 Exp/Imp start.
		// カンマかつWriteで複数パラメータ指定が可能の場合、SYNTAXエラーとしない.
		else if ( (*p == CodeOfComma)
			&& !(rpCommandInfo->mWriteInfo & RDSE_COMMANDINFO_WRITEINFO_MULTI_PARAM) )
		{
			result = RDSE_SYNTAX_RESULT_SYNTAX_ERROR;
			bInLoop = False;
		}
//		else if ((*p == CodeOfComma) || (p > pCommandEnd))	//カンマ、終端越えは エラー
		else if (p > pCommandEnd)	//終端越えは エラー
// 2010/07/29 yito UV-00886 Exp/Imp end  .
		{
			result = RDSE_SYNTAX_RESULT_SYNTAX_ERROR;
			bInLoop = False;
		}
		else
			p++;
	}

// 2010/07/29 yito UV-00886 Exp/Imp start.
	// 最大データ長を越えている.
	// ただし、複数パラメータ指定が可能の時は例外とする。
	if ((p - pDataTop) > rpCommandInfo->mDataSize 
		&& !(rpCommandInfo->mWriteInfo & RDSE_COMMANDINFO_WRITEINFO_MULTI_PARAM))
	{
		return RDSE_SYNTAX_RESULT_SYNTAX_ERROR;
	}
// 2010/07/29 yito UV-00886 Exp/Imp end  .
	else
		*pCheckedCount = (int32_t)(p - pCommand);		// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
	return result;
}

//概要：ModifyRewriteユーザデータのチェック
//　　　基本的に E-mailのWRを想定した文法チェックを行う。
//　　　この関数では、A7をコマンドとして見なさない。
//戻り値：コマンドチェックの結果
//備考：１コマンド・１パラメータ・１データ
//作成者：宮本
//レビュー：2003/11/26、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成
	RDSE_SYNTAX_RESULT
RDSC_Modify::CheckSyntax(
		const unsigned char* pCommand,	// [IN]  ホストから要求されたコマンド文字列
		int32_t commandSize				// [IN]  ホストから要求されたコマンド文字列のサイズ[単位:byte]
		) const
{
	RDSE_SYNTAX_RESULT	result = RDSE_SYNTAX_RESULT_NO_DELIMITER;		//結果
	const unsigned char* p = pCommand;			// 次のユーザデータ文字列アドレス
	int32_t	leftCommandSize = commandSize;		// 残りユーザデータ文字列のサイズ
	const RDSS_CommandInfo*	pCommandInfo = 0;	// コマンド情報
	int32_t	lCheckedCount = 0;					// チェックした文字数(異常時は０)

	//概要：１コマンドのチェック
	result = 
	CheckCommand(
			p,								// [IN]  コマンド文字列
			leftCommandSize,				// [IN]  コマンド文字列のサイズ
			pCommandInfo,					// [OUT] コマンド情報のアドレスの参照(異常時は０が格納される)
			&lCheckedCount);				// [OUT] チェックした文字数(異常時は０)
											//		 空白が続く場合は チェック済文字数に空白は含まなれない

	if (result == RDSE_SYNTAX_RESULT_OK_SPACE)	// 正常（空白終端を検出）
	{
		leftCommandSize -= lCheckedCount;	// 残りサイズを 更新
		p += lCheckedCount;					// ポインタを進める

		if (strncmp(reinterpret_cast<const char*>(p), " A7", 3) != 0) return RDSE_SYNTAX_RESULT_SYNTAX_ERROR;	//A7でない複数コマンドは エラーとする
		//概要：１コマンドのチェック
		result = 
		CheckCommand(
				p,								// [IN]  コマンド文字列
				leftCommandSize,				// [IN]  コマンド文字列のサイズ
				pCommandInfo,					// [OUT] コマンド情報のアドレスの参照(異常時は０が格納される)
				&lCheckedCount);				// [OUT] チェックした文字数(異常時は０)
												//		 空白が続く場合は チェック済文字数に空白は含まなれない
		//空白終端の場合は、複数コマンドとして エラーとする
		if (result == RDSE_SYNTAX_RESULT_OK_SPACE)
			return RDSE_SYNTAX_RESULT_SYNTAX_ERROR;
		//空白終端でない場合は、戻り値をそのまま返す
	}
	//空白終端でない場合は、戻り値をそのまま返す

	return result;
}

//概要：不揮発の変更
//　　　要求されたユーザデータに従って、不揮発に書き込む。
//　　　この関数では、A7をコマンドとして見なさない。
//戻り値：不揮発アクセスの結果
//前提：ホストから要求されたコマンド文字列の文法チェックは正常終了している。
//備考：ホストから要求されたコマンド文字列中に 予約語が含まれる場合、変換する。
//　　　Telephonyの場合
//　　　ModifyRewriteで使用する。
//作成者：宮本
//レビュー：2003/11/27、堤
//変更履歴：変更日、変更者、変更内容（変更理由）を記述
//			2003/11/15、宮本、新規作成
//			2007/02/26　佐久間　問題点No.77287対応。受信バッファを144→280に変更 
	RDSE_NONVOLATILE_RESULT
RDSC_Modify::ModifyNonVolatile(
		const unsigned char* pCommand,	// [IN]  ホストから要求されたコマンド文字列
		int32_t commandSize				// [IN]  ホストから要求されたコマンド文字列のサイズ[単位:byte]
		) const
{
	//文法チェック済なので、最初のコマンドのみを処理する
	Bool	bIsGood = False;
	RDSE_NONVOLATILE_RESULT	result = RDSE_NONVOLATILE_RESULT_SYNTAXERR;		//結果

// 2016/12/22 H.Tokuno RQ-643_Start-up time shortening Start.
	// センターから設定書き換え要求を受信した場合、処理の成否に関わらず、SuperWarp起動を不可とする.
	// CSRCはサービスモード/管理者モードに入らず直接CommonAPIをコールして設定を行っている.
	// SuperWarp起動不可の判定はCommonAPI側で行っているが、サービスログインされないためその判定が出来ない.
	// 再起動が必要な設定の書き換えについても同様で、CSRCは再起動を促さず、判定が出来ない.
	// しかし、項目毎に個別の判断を行うと抜け漏れのリスクが高いことから、CSRCからの書き換え要求時は.
	// 一律でSuperWarp不可を設定するすることとなった.

	tskRdsEvent.setSuperWarpDisableCSRC();

// 2016/12/22 H.Tokuno RQ-643_Start-up time shortening End.

//******************  add start by tomiyasu 2008/10/22 *************************************//

	RDSE_TaskID				taskID;
	taskID = theRdsApl.isRdsSystem();

	if( taskID == RDSE_TSK_ID_TELL || taskID == RDSE_TSK_ID_FAX )
	{
		if(true == APIC_SystemSetting::newInstance()->getSecurityLevelUpOn())	
		{
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_OTHER2" );
			return RDSE_NONVOLATILE_RESULT_OTHER;
		}
	}	

//******************  add end by tomiyasu 2008/10/22 *************************************//

	
	const unsigned char*	p = pCommand;
	const unsigned char*	pCommandEnd = p + commandSize;	// コマンド文字列の終端アドレス

	//空白を期待する。
	if ((*p++ != CodeOfSpace) || (commandSize < (1+2))) return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
	//コマンドを期待する
	const RDSS_CommandInfo*	pCommandInfo = mrCommandMatrix.GetCommandInfo(p, &bIsGood);
	//不正コマンドなら、終了
	if (!bIsGood){
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR14" );
		return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
	}
	const unsigned char*	pCommandTop = p;
	p += 2;

// 2010/07/29 yito UV-00886 Exp/Imp start.
	RDSE_CMD_RESULT	ret = RDSE_CMD_RESULT_Normal;
	// 複数パラメータ可能コマンド処理.
	if ( pCommandInfo->mWriteInfo & RDSE_COMMANDINFO_WRITEINFO_MULTI_PARAM )
	{
		EPN_DBG_MESSAGE( (uchar*)"ModifyNonVolatile", (uint32_t)0, (uchar*)"multi param:start" );
		//ReadOnlyに対する書き込みは、エラー.
		if (pCommandInfo->mbIsReadOnly){
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_WRITE_ERR_TO_RONLY3" );
			return RDSE_NONVOLATILE_RESULT_WRITE_ERR_TO_RONLY;
		}
	
		// Writeコマンドの情報を格納する領域。
		RDSC_WriteCommandBackup c_Backup;
		if(c_Backup.Initialize() == False)
		{
			// メモリ確保できない.
			EPN_DBG_MESSAGE( (uchar*)"ModifyNonVolatile", (uint32_t)0, (uchar*)"err:Initialize" );
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_OTHER3" );
			return RDSE_NONVOLATILE_RESULT_OTHER;
		}
		char* pDataTop = NULL;
		char* pDataEnd = NULL;
//		RDSE_NONVOLATILE_RESULT result = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;				// 2010/11/17 higuchi Minerva warning 対応.

		// パラメータとデータ部を抽出
		Bool b_InLoopParam = True;	// パラメータ部のループ継続フラグ
		Bool b_InLoopData = True;	// データ部のループ継続フラグ　
		while(b_InLoopParam)
		{
			int	nValue = RDSC_CommandMatrix::Hex2Toi(p, &bIsGood);
			//パラメータ値が不正/パラメータ値が最大値を越えている.
			if (!bIsGood || nValue > pCommandInfo->mParamMax )
			{	
				c_Backup.Finalize();
				DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR15" );
				return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			}
			p += 2;
			
			// パラメータのデータ先頭位置の保存.
			pDataTop = (char*)p;
			b_InLoopData = True;
			while (b_InLoopData)
			{
				// カンマ時はb_InLoopParamはつづける。
				if (*p == CodeOfComma)
				{
					b_InLoopData = False;
					pDataEnd = (char*)p;
					p++;
				}
				//空白かユーザデータ文字列終端を検出時は、b_InLoopParamから出る。
				else if ((*p == CodeOfSpace) || (p == pCommandEnd))
				{
					b_InLoopData = False;
					b_InLoopParam = False;
					pDataEnd = (char*)p;
				}
				else if ((p > pCommandEnd))	// 終端越えは エラー。
				{
					c_Backup.Finalize();
					DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR16" );
					return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
				}
				else
				{
					p++;
				}
			}// end:while(b_InLoopData).
			
			if(pDataTop != pDataEnd)
			{
				// 1パラメータ内の文字数がRDSD_PARAM_MAX_DATASIZE以上のとき、エラーとする.
				intptr_t sl_len = (intptr_t)((intptr_t)pDataEnd - (intptr_t)pDataTop);
				if( sl_len > RDSD_PARAM_MAX_DATASIZE )
				{
					EPN_DBG_MESSAGE( (uchar*)"ModifyNonVolatile", (uint32_t)0, (uchar*)"err:sl_len" );
					c_Backup.Finalize();
					DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR17" );
					return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
				}
				char sc_Data[RDSD_PARAM_MAX_DATASIZE+1] = {0};
				memcpy(sc_Data, pDataTop, sl_len);

				//概要：規定文字を 予約文字に変換。
				RDSC_CommandMatrix::ConvertRegulatedToReserved((uchar*)&sc_Data[0], (int)sl_len);		// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
				
				c_Backup.setParamAndData( (uchar)nValue, sc_Data );
			}
		}	// end:while(b_InLoopParam).
		
		// コマンドのパラメータ文字から終端までをバックアップ.
		intptr_t sl_CommandLen = (intptr_t)((intptr_t)pDataEnd - (intptr_t)(&pCommandTop[2]) );
		EPN_DBG_MESSAGE( (uchar*)"ModifyNonVolatile", (uint32_t)sl_CommandLen, (uchar*)"sl_CommandLen" );
		if(RDSD_WRITE_COMMAND_DATASIZE <  sl_CommandLen)
		{
			EPN_DBG_MESSAGE( (uchar*)"ModifyNonVolatile", (uint32_t)0, (uchar*)"err:sl_len2" );
		}
		tskRdsEvent.setWriteString( (char*)&pCommandTop[2] , (int32_t)sl_CommandLen);		// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
		
		c_Backup.setCommandInfo(pCommandInfo);
		c_Backup.setCommand((char*)pCommandTop);
		
// 静的解析 IDNo288162 NULLチェックを追加 kita 2010/12/20 Start.
		RDSS_WRITE_COMMAND_BACKUP* pCommandBackup = NULL;
		pCommandBackup = c_Backup.getPointer();
		if( NULL == pCommandBackup )
		{
			EPN_DBG_MESSAGE( (uchar*)"ModifyNonVolatile", (uint32_t)sl_CommandLen, (uchar*)"pCommandBackup is null" );
			// 文法エラー.
			c_Backup.Finalize();
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR18" );
			return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
		}
		
		//WriteCommand呼び出し
		ret = tskRdsEvent.WriteCommand(
				const_cast<unsigned char*>(pCommandTop),	// コマンド文字列＋パラメータ文字列（パラメータ 必ず書き込む）
				NULL,										// データ文字列
				0,											// データサイズ[単位:byte]
				pCommandBackup );
// 静的解析 IDNo288162 NULLチェックを追加 kita 2010/12/20 End.

		// c_Backup領域の解放.
//		c_Backup.Finalize();	// 解放は不要だった。
		EPN_DBG_MESSAGE( (uchar*)"ModifyNonVolatile", (uint32_t)0, (uchar*)"multi param:end" );
	}
	// 1パラメータ専用コマンド処理.
	else
	{
		//パラメータを期待
		//パラメータは２文字で、'0'～'9'、'A'～'F'
		int	nValue = RDSC_CommandMatrix::Hex2Toi(p, &bIsGood);
		//パラメータ値が不正
		if (!bIsGood){
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR19" );
			return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
		}
		//最大値を越えている
		if (nValue > pCommandInfo->mParamMax){
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR20" );
			return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
		}
		p += 2;

		//ReadOnlyに対する書き込みは、エラー
		if (pCommandInfo->mbIsReadOnly){
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_WRITE_ERR_TO_RONLY4" );
			return RDSE_NONVOLATILE_RESULT_WRITE_ERR_TO_RONLY;
		}

		//１つだけのデータを期待
		//' 'または"終端"まで アドレスを進める
		const unsigned char*	pDataTop = p;	//データ部の先頭アドレス
		Bool bInLoop = True;
		while (bInLoop)
		{
			if ((*p == CodeOfSpace) || (p == pCommandEnd))
			{
				//空白か ユーザデータ文字列終端を検出
				bInLoop = False;
			}
			else if ((*p == CodeOfComma) || (p > pCommandEnd))	//カンマ、終端越えは エラー
			{
				DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR21" );
				return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
			}
			else
				p++;
		}
		
		//最大データ長を越えている
		short	dataLength = static_cast<short>(p - pDataTop);
		if (dataLength > pCommandInfo->mDataSize){
			DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_SYNTAXERR22" );
			return RDSE_NONVOLATILE_RESULT_SYNTAXERR;
		}

		//受信バッファを書替えない前提があるので、バッファが必要
		// 070226　佐久間　問題点No.77287対応。受信バッファを144→280に変更 
		//unsigned char	buf[256+24];	//"TI00"の最大サイズ255をカバーできるバッファ([+]はマージン) 
		unsigned char	buf[255*3+24];	//2013/06/19 MinervaZeus3rd 制御1209 FW強制更新対応　最大255*3のサイズに対応する.
		memset(buf, 0x00, sizeof(buf));		// バッファの初期化 071016　佐久間　問題点No.90121対応 
		memcpy(buf, pDataTop, dataLength);
		//概要：規定文字を 予約文字に変換
		RDSC_CommandMatrix::ConvertRegulatedToReserved(buf, dataLength);
		
		//WriteCommand呼び出し
		ret = tskRdsEvent.WriteCommand(
				const_cast<unsigned char*>(pCommandTop),	// コマンド文字列＋パラメータ文字列（パラメータ 必ず書き込む）
				buf,				// データ文字列
				dataLength);		// データサイズ[単位:byte]

	}
// 2010/07/29 yito UV-00886 Exp/Imp end  .

	switch (ret)
	{
	case RDSE_CMD_RESULT_Normal:		// 正常
		result = RDSE_NONVOLATILE_RESULT_OK_NO_NEXT;
		break;
	default:
//	case RDSE_CMD_RESULT_ErrNotRead:	// エラーで読めない（メカコン系データ）
//	case RDSE_CMD_RESULT_ErrNoData:		// データが無い（コマンド不正、パラメータ不正）
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_INVALID_ACCESS/[%d]", (int)ret );
		result = RDSE_NONVOLATILE_RESULT_INVALID_ACCESS;
		break;
	case RDSE_CMD_RESULT_ErrReadOnly:	// ReadOnlyに対する書き込みを行おうとした
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_WRITE_ERR_TO_RONLY5" );
		result = RDSE_NONVOLATILE_RESULT_WRITE_ERR_TO_RONLY;
		break;
	case RDSE_CMD_RESULT_ErrCenterClock_A5:	// センターの時計がおかしい。（※ A5 コマンドでのみ使用可）
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_INVALID_TIME" );
		result = RDSE_NONVOLATILE_RESULT_INVALID_TIME;		// 期日指定発信(A5)の設定時間が不正
		break;
	case RDSE_CMD_RESULT_ErrScope:		// データ範囲外
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_INCORRECT" );
		result = RDSE_NONVOLATILE_RESULT_INCORRECT;			// 不正な値を書き込もうとした［センタには不可項目を返す］
		break;
	case RDSE_CMD_RESULT_ErrLengthError:		// データレングス異常 04/03/23 浦崎 
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_INVALID_WRITE" );
		result = RDSE_NONVOLATILE_INVALID_WRITE;				// データ長が不正［センタには文法エラーを返す］
		break;
	case RDSE_CMD_RESULT_ErrCopyerBusy:		// コピー中による書換え不可 04/05/24 浦崎 
	case RDSE_CMD_RESULT_ErrMaintenance:		// メンテナンス中による書換え不可 04/05/24 浦崎 
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_UNCHANGEABLE_COPY/[%d]", (int)ret );
		result = RDSE_NONVOLATILE_UNCHANGEABLE_COPY;			// コピー中による書換え不可［センタにはコピー中（変更不可）を返す］
		break;
	case RDSE_CMD_RESULT_ErrOther:		// (その他のエラー)
		DBG_CsrcErrorSequence( __FILE__ ,__LINE__, "RDSE_NONVOLATILE_RESULT_OTHER4" );
		result = RDSE_NONVOLATILE_RESULT_OTHER;				// その他のエラー
		break;
	// 2013.12.13 H.Wada [ZeusCD][UV-01385] WP61,81,88,308.
// 2018/05/03 HCLT-Vinay 【IT6_1.0 Eagle】制御構成変更_No.97：【ストレージ構成変更】 Mod Start.
	case RDSE_CMD_RESULT_ErrNoSSD:		// SSD非装着
		EPN_DBG_MESSAGE( (uchar*)__FUNCTION__, (uint32_t)0, (uchar*)"Err is RDSE_CMD_RESULT_ErrNoSSD" , __LINE__ );
		result = RDSE_NONVOLATILE_RESULT_NO_SSD;			// SSD非装着
// 2018/05/03 HCLT-Vinay 【IT6_1.0 Eagle】制御構成変更_No.97：【ストレージ構成変更】 Mod End.
		break;
	// 2013.12.13 H.Wada [ZeusCD][UV-01385] WP61,81,88,308.
	}

	return result;
}


// 2010/07/29 yito UV-00886 Exp/Imp start.
/*******************************************************************************
	RDSC_WriteCommandBackupの処理部											.
*******************************************************************************/
// コンストラクタ（引数なし）.
RDSC_WriteCommandBackup::RDSC_WriteCommandBackup()
{
	mps_backup = NULL;
	uc_ParamCountCurrent = 0;
}
// コンストラクタ（引数あり）.
RDSC_WriteCommandBackup::RDSC_WriteCommandBackup(RDSS_WRITE_COMMAND_BACKUP *ps_backup)
{
	mps_backup = ps_backup;
	uc_ParamCountCurrent = 0;
}
// デストラクタ.
RDSC_WriteCommandBackup::~RDSC_WriteCommandBackup()
{
}

// 格納領域の取得＆初期化.
Bool RDSC_WriteCommandBackup::Initialize()
{
	// 先にデータがある場合、領域確保しない。
	if(mps_backup == NULL)
	{
		mps_backup = (RDSS_WRITE_COMMAND_BACKUP*)malloc(sizeof(RDSS_WRITE_COMMAND_BACKUP));
	}
	// 領域が確保できていない場合、なにもしない.
	if(mps_backup == NULL)
	{
		// 領域の確保失敗.
		return False;
	}
	
	// 初期値を設定.
	memset(mps_backup->sc_Command, 0, sizeof(mps_backup->sc_Command) );
	mps_backup->us_ParamCount = 0;
	memset(mps_backup->ps_ParamData ,0, sizeof(RDSS_PARAM_DATA*) * RDSD_PARAM_MAX_NUMBER);

	return True;
}

// 格納領域の解放.
void RDSC_WriteCommandBackup::Finalize()
{
	if(NULL == mps_backup)
	{
		// なにもしない.
		return;
	}
	// すべての領域を開放する.
	for(int i = 0; i < RDSD_PARAM_MAX_NUMBER; i++)
	{
		if(NULL == mps_backup->ps_ParamData[i])
		{
			continue;	// なにもせず次へ.
		}
		// 領域解放.
		free(mps_backup->ps_ParamData[i]);
		mps_backup->ps_ParamData[i] = NULL;
	}
	
	free(mps_backup);
	mps_backup = NULL;
}

// コマンド名の保存.
void RDSC_WriteCommandBackup::setCommand(char *psc_Command)
{
	if(NULL == psc_Command || NULL == mps_backup)
	{
		// なにもしない.
		return;
	}

	memcpy(mps_backup->sc_Command, psc_Command, RDSD_PARAM_COMMAND_DATASIZE);
}

// コマンド名の取得.
char* RDSC_WriteCommandBackup::getCommand()
{
	return (mps_backup->sc_Command);
}

// コマンドInfo構造体の保存.
void RDSC_WriteCommandBackup::setCommandInfo(const RDSS_CommandInfo *ps_CommandInfo)
{
	if(NULL == ps_CommandInfo)
	{
		return;
	}
	memcpy( &mps_backup->s_CommandInfo, ps_CommandInfo, sizeof(RDSS_CommandInfo));
}

// コマンドInfo構造体の取得.
RDSS_CommandInfo* RDSC_WriteCommandBackup::getCommandInfo()
{
	return &(mps_backup->s_CommandInfo);
}

// パラメータとデータの設定.
void RDSC_WriteCommandBackup::setParamAndData(uchar ucParam, char* pscData)
{
	if(NULL == mps_backup)
	{
		// なにもしない.
		return;
	}
	if(NULL == pscData)
	{
		// ありえない.
		return;
	}
	int32_t sl_len = (int32_t)strlen(pscData);		// 2018/11/11 FPT-DatNT30 IT6_1.0 【AR1029580】 fix type cast warning
	if(RDSD_PARAM_MAX_DATASIZE < sl_len)
	{
		// データ長が長すぎる.
		return;
	}
	
	Bool b_isData = False;
	int i;
	for(i = 0; i < RDSD_PARAM_MAX_NUMBER; i++)
	{
		if(NULL == mps_backup->ps_ParamData[i])
		{
			b_isData = True;
			break;
		}
	}
	if(False == b_isData)
	{
		// すべて埋まっている.
		return;
	}

	mps_backup->ps_ParamData[i] = (RDSS_PARAM_DATA*)malloc(sizeof(RDSS_PARAM_DATA));
	if(NULL == mps_backup->ps_ParamData[i])
	{
		// 確保失敗.
		return;
	}
	// データを設定.
	mps_backup->ps_ParamData[i]->uc_Param = ucParam;
	strcpy(mps_backup->ps_ParamData[i]->sc_Data,pscData);
	// パラメータ数をインクリメント.
	mps_backup->us_ParamCount++;				// 2010/11/17 higuchi Minerva warning 対応.
}
void RDSC_WriteCommandBackup::setParamAndData(RDSS_PARAM_DATA* pstParamData)
{
	if(NULL == pstParamData)
	{
		return;
	}
	setParamAndData(pstParamData->uc_Param, pstParamData->sc_Data);
}
	
// パラメータとデータの取得.
RDSS_PARAM_DATA* RDSC_WriteCommandBackup::getParamAndData(uchar ucParam)
{
	if(NULL == mps_backup)
	{
		return NULL;
	}
	for(int i = 0; i < RDSD_PARAM_MAX_NUMBER; i++)
	{
		if(NULL == mps_backup->ps_ParamData[i])
		{
			continue;
		}
		if(ucParam == mps_backup->ps_ParamData[i]->uc_Param)
		{
			return mps_backup->ps_ParamData[i];
		}
	}
	// 見つからない場合、NULL返却.
	return NULL;
}

// パラメータとデータの取得(先頭から順に取得する).
RDSS_PARAM_DATA* RDSC_WriteCommandBackup::getParamAndDataNext(Bool b_isTop)
{
	// 先頭のものを取得する。
	if(b_isTop == True)
	{
		uc_ParamCountCurrent = 0; 
	}
	if(NULL == mps_backup)
	{
		return NULL;
	}
	uchar uc_Current = uc_ParamCountCurrent;
	if(NULL == mps_backup->ps_ParamData[uc_Current])
	{
		// 見つからない場合、NULL返却.
		return NULL;
	}
	uc_ParamCountCurrent++;	// 次回取得用にインクリメント.
	return mps_backup->ps_ParamData[uc_Current];
}

// パラメータ数の取得.
ushort RDSC_WriteCommandBackup::getParamCount()
{
	if(NULL == mps_backup)
	{
		return 0;	// 領域未確保の場合、0.
	}
	return (mps_backup->us_ParamCount);
}
	
// Writeバックアップ情報を別EVENT等に渡す用関数.
// この関数を使用したら、メモリの解放は「ポインタを受け取った側」が行う。
RDSS_WRITE_COMMAND_BACKUP* RDSC_WriteCommandBackup::getPointer()
{
	return (mps_backup);
}

// 2010/07/29 yito UV-00886 Exp/Imp end  .
// 2011/06/30 yito CQ349199 期日指定発信でのDX取得 start.
// デバイスカウンタ(DX)を取得してよいかをチェックする関数.
// 戻り値 True:取得OK, False:取得NG.
Bool isEnableStatus_ReadDX()
{
	EPN_DBG_MESSAGE( (uchar*)"DX", (uint32_t)0, (uchar*)"isEnableStatus_ReadDX:start" );

	// デバイスカウンタが取得できる条件.
	//		1.セキュリティ強化モードがOFFに設定されている.
	//		2.遠隔カウンター管理設定が許可に設定されている。
	//		3.本体装置認証、もしくは、外部サーバー認証がONもしくは、部門管理設定にて管理するに設定されている.

	// 1.セキュリティ強化モードの取得.
	//	 ONは取得NG.
	APIC_SystemSetting *pc_SystemSetting = APIC_SystemSetting::newInstance();
	pc_SystemSetting->lock();
	Bool b_Security = pc_SystemSetting->getSecurityLevelUpOn();
	pc_SystemSetting->unlock();
	if( True == b_Security )
	{
		EPN_DBG_MESSAGE( (uchar*)"DX", (uint32_t)0, (uchar*)"b_Security:True" );
// 2015/11/06 tatsumi ZeusS_ZX0 制御構成変更_No.76-2_ネットワークエラーカウンタ追加 START.
		RDS_incNetworkErrorCount( RDSE_NETWORK_ERROR_DEVICE_COUNTER_SECURITY_LEVELUP_ON );
// 2015/11/06 tatsumi ZeusS_ZX0 制御構成変更_No.76-2_ネットワークエラーカウンタ追加 END.
		return False;
	}
	
	// 2.遠隔カウンター管理設定の取得.
	APIC_OperationSetting *pc_OperationSetting = APIC_OperationSetting::newInstance();
	pc_OperationSetting->lock();
	Bool b_CntSetting = pc_OperationSetting->getCsrcUserTrackCntSetting();
	pc_OperationSetting->unlock();
	if( False == b_CntSetting )
	{
		EPN_DBG_MESSAGE( (uchar*)"DX", (uint32_t)0, (uchar*)"b_CntSetting:False" );
// 2015/11/06 tatsumi ZeusS_ZX0 制御構成変更_No.76-2_ネットワークエラーカウンタ追加 START.
		RDS_incNetworkErrorCount( RDSE_NETWORK_ERROR_DEVICE_COUNTER_DISABLE );
// 2015/11/06 tatsumi ZeusS_ZX0 制御構成変更_No.76-2_ネットワークエラーカウンタ追加 END.
		return False;
	}
	
	// 3.本体装置認証、もしくは、外部サーバー認証がONもしくは、部門管理設定にて管理するに設定されている.
	// 認証方式の取得.
	APIC_UserAuthentication *pc_UserAuthentication = APIC_UserAuthentication::newInstance();
	pc_UserAuthentication->lock();
	TYPE_UserAuthentication e_Authentication = pc_UserAuthentication->getUserAuthentication();
	pc_UserAuthentication->unlock();
	// 部門管理の取得.
	APIC_TrackManager *pc_TrackManager = APIC_TrackManager::newInstance();
	pc_TrackManager->lock();
	TYPE_TrackMode e_TrackMode = pc_TrackManager->getTrackMode();
	pc_TrackManager->unlock();
	
	if(!(	( e_Authentication == TYPE_UserAuthenticationDevice) || 	// 本体認証.
			( e_Authentication == TYPE_UserAuthenticationServer ) ||	// 外部サーバ認証:.
			( e_Authentication == TYPE_UserAuthenticationDeviceAndServer ) ||	// 本体装置認証+外部サーバー認証:.
			( e_Authentication == TYPE_UserAuthenticationDeviceAndMidAuth ) ||	// 本体認証+拡張サーバ:.
			( e_TrackMode != TYPE_TrackModeOff )						// 部門管理設定:管理する.
		) )
	{
		EPN_DBG_MESSAGE( (uchar*)"DX", (uint32_t)0, (uchar*)"UserAuth or Track:False" );
// 2015/11/06 tatsumi ZeusS_ZX0 制御構成変更_No.76-2_ネットワークエラーカウンタ追加 START.
		RDS_incNetworkErrorCount( RDSE_NETWORK_ERROR_DEVICE_COUNTER_AUTH_ERROR );
// 2015/11/06 tatsumi ZeusS_ZX0 制御構成変更_No.76-2_ネットワークエラーカウンタ追加 END.
		return False;
	}
	
	// すべてOKなら,True.
	EPN_DBG_MESSAGE( (uchar*)"DX", (uint32_t)0, (uchar*)"isEnableStatus_ReadDX:end(OK)" );
	return True;
}
// 2011/06/30 yito CQ349199 期日指定発信でのDX取得 end  .
// 2018/07/03 HCLT-Vinay 【IT6_1.0】【制御構成変更_No.110】 Start.
Bool isEnableStatus_ReadZP()
{
	EPN_DBG_MESSAGE( (uchar*)"ZP", (uint32_t)0, (uchar*)"isEnableStatus_ReadZP:start" );

	// 1.セキュリティ強化モードの取得.
	//	 ONは取得NG.
	APIC_SystemSetting *pc_SystemSetting = APIC_SystemSetting::newInstance();
	pc_SystemSetting->lock();
	Bool b_Security = pc_SystemSetting->getSecurityLevelUpOn();
	pc_SystemSetting->unlock();
	if( True == b_Security )
	{
		EPN_DBG_MESSAGE( (uchar*)"ZP", (uint32_t)0, (uchar*)"b_Security:True" );
		RDS_incNetworkErrorCount( RDSE_NETWORK_ERROR_DEVICE_COUNTER_SECURITY_LEVELUP_ON );
		return False;
	}
	
//2019/06/13 FPT-HaiVV3 【Eagle】 [CWH] 文法エラー(ER50)について 【Eagle】 remove Start.
//	// 2.管理者設定>セキュリティ設定>セキュリティ詳細>デバッグ情報の取出し 
//	APIC_DebugLogSystemSetting* pc_DebugLogSystemSetting = APIC_DebugLogSystemSetting::newInstance();
//	Bool b_DebugInfoTake = pc_DebugLogSystemSetting->getTakeLogPermissionSetting();
//	if( False == b_DebugInfoTake )
//	{
//		EPN_DBG_MESSAGE( (uchar*)"ZP", (uint32_t)0, (uchar*)"b_DebugInfoTake:False" );
//		RDS_incNetworkErrorCount( RDSE_NETWORK_ERROR_SECURITY_DEBUGLOG_PERMISSION_PROHIBIT );
//		return False;
//	}
//2019/06/13 FPT-HaiVV3 【Eagle】 [CWH] 文法エラー(ER50)について 【Eagle】 remove End.
	//3. Log Analysis In Progress
	if ( True == checkLogAnalysisStatus() )
	{
		EPN_DBG_MESSAGE( (uchar*)"ZP", (uint32_t)0, (uchar*)"LogAnalysis:True" );
		RDS_incNetworkErrorCount( RDSE_NETWORK_ERROR_DURING_LOG_ANALAYSIS );
		return False;
	}
	
	// すべてOKなら,True.
	EPN_DBG_MESSAGE( (uchar*)"ZP", (uint32_t)0, (uchar*)"isEnableStatus_ReadZP:end(OK)" );
	return True;
}
// 2018/07/03 HCLT-Vinay 【IT6_1.0】【制御構成変更_No.110】 End.
