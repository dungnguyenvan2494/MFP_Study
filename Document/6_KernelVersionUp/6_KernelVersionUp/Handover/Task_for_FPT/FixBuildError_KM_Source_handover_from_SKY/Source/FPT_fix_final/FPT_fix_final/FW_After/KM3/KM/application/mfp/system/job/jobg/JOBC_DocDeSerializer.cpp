//
// ファイル名：JOBC_DocDeSerializer.cpp
//
// 概要：deSerializeクラスのメンバ関数のソース
//

// インクルードファイル
#include "JOBC_DocDeSerializer.h"
// 2015/8/31 CQ516467 E002詳細コード対応 start.
#include "system/sys/APIC_FatalSignalDetailCode.h"
#include "../../../../api/jobspec/inc/JobSpec_define.h"
// 2015/8/31 CQ516467 E002詳細コード対応 end.

static const int32_t JOBD_OVERLAY_RESTORE_MAX =	3;

// 静的メンバ変数
static char sc_TraceLevel = 2;
static char TRACELEVEL_PRINT = 1;

JOBC_DocDeSerializer* JOBC_DocDeSerializer::pc_DocDeSerializer = NULL;

//-----------------------------------------------------------------------------
// public関数群
//-----------------------------------------------------------------------------

// 概要　　：コンストラクタ
// 戻り値　：
// 備考　　：
// 作成者　：
// レビュー：
// 変更履歴：変更日、変更者、変更内容（変更理由）を記述
JOBC_DocDeSerializer::JOBC_DocDeSerializer()
{
	pJob = NULL;
	pf_Ans_DeSerialize = NULL;
}

// 概要　　：デストラクタ
// 戻り値　：
// 備考　　：
// 作成者　：
// レビュー：
// 変更履歴：変更日、変更者、変更内容（変更理由）を記述
JOBC_DocDeSerializer::~JOBC_DocDeSerializer()
{
}

// 概要　　：インスタンス取得
// 戻り値　：JOBC_DocDeSerializer*
// 備考　　：
// 作成者　：
// レビュー：
// 変更履歴：変更日、変更者、変更内容（変更理由）を記述
JOBC_DocDeSerializer*
JOBC_DocDeSerializer::newInstance()
{
	if(pc_DocDeSerializer == NULL)
	{
		// システム上にJOBC_FaxSndStateTableクラスのインスタンスが
		// 存在しなければコンストラクタをコール
		pc_DocDeSerializer = new JOBC_DocDeSerializer();
		if(pc_DocDeSerializer == NULL)
		{
			// 領域確保失敗
			APIC_setFatalDetailCodeE002(CE002_DetailCode0x545E);
			APIC_ThrowSystemError(
				SYSE_SysErrTypeSystemExhausted,
				LOGF_DEBUG_EVENT_KIND_JOB,
				const_cast<const char *>("JOBC_DocDeSerializer::newInstance JOBC_DocDeSerializer領域確保失敗"));
		}
	}
	
	return pc_DocDeSerializer;
}

// 概要　　：初期化
// 戻り値　：なし
// 備考　　：
// 作成者　：
// レビュー：
// 変更履歴：変更日、変更者、変更内容（変更理由）を記述
void
JOBC_DocDeSerializer::initClass(JOBC_Job* pc_Job, void (*pAns_DeSerialize)(int32_t p1, void *p2))
{
	pJob = pc_Job;
	pf_Ans_DeSerialize = pAns_DeSerialize;
	
	pJob->clearRestoringJobInfo();
}

// 概要　　：インスタンス削除
// 戻り値　：なし
// 備考　　：
// 作成者　：
// レビュー：
// 変更履歴：変更日、変更者、変更内容（変更理由）を記述
void 
JOBC_DocDeSerializer::deleteInstance(int32_t sl_result)
{
	// DBGLOG
	JobTraceF6("JOBC_DocDeSerializer::deleteInstance:sl_result[%d]",
		sl_result,0,0,0,0,0,pJob
	);

	Bool b_isRecoverable = False;

	switch(sl_result){
	case DOCD_RetHddFull:				// ディスク容量不足
	case DOCD_RetBoxNotExist:			// ボックスが存在しない
	case DOCD_RetDocNotExist:			// DocFileが存在しない
	case DOCD_RetDocOpened:				// DocFileがオープンされている
	case JOBD_DESERIALIZE_CANCEL:		// シナリオキャンセル
		b_isRecoverable = True;
		break;
	default:
		break;
	}

	if( True == b_isRecoverable ){
		// オーバーレイのCompositeDoc解放＆初期化 
		// =>オーバーレイのCompositeDocを先に削除しなければisBindJob()が全てTrueとなるので注意 
		clearOverLayDoc();

		// ジョブ破棄
		// closeDoc & Doc::deleteInstance()
		if( True == isBindJob() ){
			// BindJobは親DocのcloseDocが不要。
			unsigned char num=1;
			while(NULL != pJob->getBoxToPrintDoc(num)){
				pJob->getBoxToPrintDoc(num)->closeDoc();		// CloseDoc
				pJob->getBoxToPrintDoc(num)->deleteInstance();	// Doc::deleteInstance()
				num++;
			}
		}
		else{
			// CloseDoc
			if( NULL != pJob->getBoxToPrintDoc(0) ){
				pJob->getBoxToPrintDoc(0)->closeDoc();
			}
		}
		if( NULL != pJob->getBoxToPrintDoc(0) ){
			// Doc::deleteInstance()
			pJob->getBoxToPrintDoc(0)->deleteInstance();
		}
	}
	else{
		// システムエラー発生
		APIC_setNewDetailCode(CE002_DetailCode0x545D, makeNewDetailCodeJob01(static_cast<int>(sl_result)));
		APIC_ThrowSystemError(SYSE_SysErrTypeSystemExhausted,LOGF_DEBUG_EVENT_KIND_JOB,
		const_cast<const char *>("JOBC_DocDeSerializer::deleteInstance ERR"));
	}
}

// 概要　　：子Docのインスタンス生成処理
// 戻り値　：True:生成成功、False:生成失敗
// 備考　　：
// 作成者　：
// レビュー：
// 変更履歴：変更日、変更者、変更内容（変更理由）を記述
Bool
JOBC_DocDeSerializer::addListCompositeDoc()
{
	int32_t DocRtn = DOCD_RetOk;
	Bool rtn = True;

	int32_t sl_DocID[TYPD_MaxProgramJob];

	int32_t sl_DocCnt = 0;
	DOCC_BoxIf* pBox=NULL;
	DOCC_CompositeDoc* pDoc=NULL;

	for(int i = 0 ; i < TYPD_MaxProgramJob ; i++ ){
		sl_DocID[i] = TYPD_NotSetting;
	}

	int32_t l_No = 0;

	if (True == pJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn()) {
		// プログラムジョブ時は、JobStatusよりDocIDを取得する.
		pJob->getJobStatus()->getProgJobDocumentNo(sl_DocID);

		l_No = TYPD_MaxProgramJob;

		// BoxNoはプログラムジョブBox固定.
		pBox = DOCC_BoxIf::newInstance(DOCD_ProgramJobBox);
	}
	else {
		// プログラムジョブ以外であればJobSpecよりDocIDを取得
		pJob->getJobSpec()->pc_Common->c_JobType.getDocumentNo(sl_DocID);

		l_No = TYPD_MaxBindJob;

		pBox = DOCC_BoxIf::newInstance(pJob->getJobSpec()->pc_Common->c_JobType.getBoxNo());
	}

	// Doc数算出
	for (int32_t num=0; num<l_No; num++) {
		if (0 < sl_DocID[static_cast<int>(num)]) {
			sl_DocCnt++;
		}
	}

	do
	{
		if( NULL == pBox ){
			JobTrace("JOBC_DocDeSerializer::addListCompositeDoc pBox=NULL!!",pJob);
			DocRtn = DOCD_RetParameterErr;
			break;
		}

		if( 1 != sl_DocCnt ) { // BindJob
			for(int32_t num=0;num<sl_DocCnt;num++)
			{
				// Doc::newInstance()
				pDoc = DOCC_CompositeDoc::newInstance();
				if( NULL == pDoc ){
					JobTrace("JOBC_DocDeSerializer::addListCompositeDoc Bind-pDoc=NULL!!",pJob);
					DocRtn = DOCD_RetDocNotExist;
					break;
				}
				// Set_DocPtr
				pJob->setBoxToPrintDoc(pDoc, num+1);
				// 連結pBox->getDoc()
				DocRtn = pBox->getDoc(pJob->getBoxToPrintDoc(num+1),sl_DocID[static_cast<int>(num)]);
				if( DOCD_RetOk != DocRtn ){
					JobTrace("JOBC_DocDeSerializer::addListCompositeDoc Bind-getDoc-NG!!",pJob);
					break;
				}
				// OpenDoc
				DocRtn = pJob->getBoxToPrintDoc(num+1)->openDoc();
				if( DOCD_RetOk != DocRtn ){
					JobTrace("JOBC_DocDeSerializer::addListCompositeDoc Bind-openDoc-NG!!",pJob);
					break;
				}
			}
			if( DOCD_RetOk != DocRtn ) break; // do while から抜ける
		}

		// ParentDoc
		// Doc::newInstance()
		pDoc = DOCC_CompositeDoc::newInstance();
		if( NULL == pDoc ){
			JobTrace("JOBC_DocDeSerializer::addListCompositeDoc pDoc=NULL!!",pJob);
			DocRtn = DOCD_RetDocNotExist;
			break;
		}
		// Set_DocPtr
		pJob->setBoxToPrintDoc(pDoc, 0);
		// BindJobかどうか
		if( False == isBindJob() ){
			// 連結pBox->getDoc()
			DocRtn = pBox->getDoc(pJob->getBoxToPrintDoc(0), sl_DocID[0]);
			if( DOCD_RetOk != DocRtn ){
				JobTrace("JOBC_DocDeSerializer::addListCompositeDoc getDoc-NG!!",pJob);
				break;
			}
			// OpenDoc
			DocRtn = pJob->getBoxToPrintDoc(0)->openDoc();
			// 既にファイルオープンされている場合は許容するものとする。
			if ( DOCD_RetOk != DocRtn )
			{
				// 文書保持設定が「しない」の場合は、Docが削除されるケースがあるため、
				// ファイルの２重オープンを許可しない。
				// 許可した場合、同じDOCを使用している後続ジョブが動けなくなってしまう。
				APIC_SystemSetting* _pc_SysSetting = APIC_SystemSetting::newInstance();
				Bool	b_AllDoc=False;
				if(_pc_SysSetting!=NULL)
				{
					b_AllDoc = _pc_SysSetting->getDeleteEnableInPickingDoc();
				}
				if (((b_AllDoc == False) && ( APID_DOCD_RetDocOpened == DocRtn)) || //文書保持設定「する」&&既にファイルを開いている．
					(pJob->getJobSpec()->pc_Common->c_JobType.getBoxNo() == DOCD_ImageDealerBox))	// イメージディラーBox である.
				{
					//イメージディーラーBoxの場合は、すでに他ジョブがファイルオープンしていていも、他ジョブがファイルを削除することがないので、NGではない。
					DocRtn = DOCD_RetOk;
				}else{
					JobTrace("JOBC_DocDeSerializer::addListCompositeDoc openDoc-NG!!",pJob);
					break;
				}
			}
		}
		pBox->deleteInstance();
	}while(False);

	if( DOCD_RetOk != DocRtn ){
		// ジョブ破棄・システムエラー発生
		if( NULL != pBox) pBox->deleteInstance();
		deleteInstance(DocRtn);
		rtn = False;
	}
	pJob->setNumOfDocNeeded(static_cast<uchar>(sl_DocCnt));
	return rtn;
}

// 概要　　：インスタンス化実施(req_DeSerialize)
// 戻り値　：True:req_DeSerialize成功、False:req_DeSerialize失敗
// 備考　　：
// 作成者　：
// レビュー：
// 変更履歴：変更日、変更者、変更内容（変更理由）を記述
Bool 
JOBC_DocDeSerializer::req_DeSerialize()
{
	int32_t DocRtn = DOCD_RetOk;
	Bool rtn = True;
	int32_t sl_BoxNo = TYPD_NotSetting;

	checkDocumentPage();

	int32_t sl_DocID[TYPD_MaxProgramJob] = {};

	for( int i = 0 ; i < TYPD_MaxProgramJob ; i++ ){
		sl_DocID[i] = TYPD_NotSetting;
	}

	if (True == pJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn()) {
		uint32_t ul_docIndex = pJob->getNumOfDocRestored();

		// プログラムジョブ時は、JobStatusよりDocIDを取得する.
		pJob->getJobStatus()->getProgJobDocumentNo(sl_DocID);
		// BoxNoはプログラムジョブBox固定.
		sl_BoxNo = DOCD_ProgramJobBox;

		if (TYPD_NotSetting != sl_DocID[ul_docIndex]) {
			DocRtn = DOCC_CompositeDoc::req_getInstance(
						sl_DocID[ul_docIndex],
						sl_BoxNo,
						JOBC_Job::ans_getDocInstanceFromDocCustom,
						pJob,
						False);

			if ((DOCD_RetRamExist != DocRtn) &&
				(DOCD_RetFileExist != DocRtn))
			{
				DocRtn = DOCD_RetNg;
			} else {
				DocRtn = DOCD_RetOk;
			}
		}
		else {
			for (uint32_t ul_cnt = 0; ul_cnt < ul_docIndex; ul_cnt++) {
				pJob->getPrccUsesInfo()->AnsData.s_OutDocInfo[ul_cnt].pCompositeDoc->openDoc();
			}
		}
	}
	else {
		// プログラムジョブ以外であればJobSpecよりDocIDを取得
		pJob->getJobSpec()->pc_Common->c_JobType.getDocumentNo(sl_DocID);
		sl_BoxNo = pJob->getJobSpec()->pc_Common->c_JobType.getBoxNo();
		// インスタンス化実施(req_DeSerialize)
		if( False == isBindJob() ){
			// NoBind
			DocRtn = pJob->getBoxToPrintDoc(0)->req_DeSerialize(
						sl_DocID[0],
						sl_BoxNo,
						pf_Ans_DeSerialize,
						pJob);
		}
		else{
			// Bind
			DocRtn = pJob->getBoxToPrintDoc(pJob->getNumOfDocRestored()+1)->req_DeSerialize(
						sl_DocID[pJob->getNumOfDocRestored()],
						sl_BoxNo,
						pf_Ans_DeSerialize,
						pJob);
		}
	}

	if( DOCD_RetOk != DocRtn ){
		// ジョブ破棄・システムエラー発生
		deleteInstance(DocRtn);
		rtn = False;
	}
	return rtn;
}

// 概要　　：DocumentPage判定
// 戻り値　：True:3000ページ以内、False:3000ページ超
// 備考　　：
// 作成者　：
// レビュー：
// 変更履歴：変更日、変更者、変更内容（変更理由）を記述
Bool 
JOBC_DocDeSerializer::checkDocumentPage()
{
	int32_t sl_DocID[TYPD_MaxProgramJob];
	int32_t sl_BoxNo = TYPD_NotSetting;

	for(int i = 0 ; i < TYPD_MaxProgramJob ; i++ ){
		sl_DocID[i] = TYPD_NotSetting;
	}

	if (True == pJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn()) {
		// プログラムジョブ時は、JobStatusよりDocIDを取得する.
		pJob->getJobStatus()->getProgJobDocumentNo(sl_DocID);
		// BoxNoはプログラムジョブBox固定.
		sl_BoxNo = DOCD_ProgramJobBox;
	}
	else {
		// プログラムジョブ以外であればJobSpecよりDocIDを取得
		pJob->getJobSpec()->pc_Common->c_JobType.getDocumentNo(sl_DocID);
		sl_BoxNo = pJob->getJobSpec()->pc_Common->c_JobType.getBoxNo();
	}

	// 展開page
	uint32_t ul_tenkaiPage =
		DOCC_DocFileAccessIf::getLeafCnt(
			sl_BoxNo,
			sl_DocID[pJob->getNumOfDocRestored()]);
	// 空きpage
	uint32_t ul_akiPage = DOCD_MaxPage - DOCC_Leaf::getUsedLeafInstCnt();
	return ( ul_tenkaiPage <= ul_akiPage ) ? True : False;
}

// 概要　　：もう一度deSerializeするか判定
// 戻り値　：True:req_deSerialize実行、False:req_deSerialize不要
// 備考　　：
// 作成者　：
// レビュー：
// 変更履歴：変更日、変更者、変更内容（変更理由）を記述
Bool
JOBC_DocDeSerializer::checkReDeSerializeDoc(int32_t sl_result)
{
	if(sc_TraceLevel>=TRACELEVEL_PRINT) JobTrace("JOBC_DocDeSerializer::checkReDeSerializeDoc start!!",pJob);

	// 展開済みドキュメント数を更新する前で全ドキュメント数と展開済みのドキュメント数が一致した場合、 
	// オーバーレイの要求のAnsが戻ってきたとみなす 
	if(pJob->getNumOfDocNeeded() <= pJob->getNumOfDocRestored())
	{
		// オーバーレイのシリアライズが異常の場合 
		if( DOCD_RetOk != sl_result )
		{
			JobTraceF6("JOBC_DocDeSerializer::checkReDeSerializeDoc (Overlay)::OverLay Serialize Err!!(Ret=%d)",
				sl_result,0,0,0,0,0,pJob);
			// オーバーレイを解除する(面倒な為、Timerにはいれない) 
			clearOverLayDoc();
		}

		// DOC数更新
		pJob->incrementNumOfDocRestored();

		if(sc_TraceLevel>=TRACELEVEL_PRINT){
			JobTraceF6("JOBC_DocDeSerializer::checkReDeSerializeDoc (Overlay)::OverLay_1 NumOfDocNeeded()=[%d],NumOfDocRestored()=[%d]",
				pJob->getNumOfDocNeeded(),pJob->getNumOfDocRestored(),0,0,0,0,pJob);
		}

		// バインドあり時、リストアカウンター(sl_Cnt)は、バインド数＋オーバーレイ画像表面、裏面の2とする。
		// バインドなし時、リストアカウンター(sl_Cnt)は、1文書＋オーバーレイ画像表面、裏面の2とする。
		int32_t	sl_Cnt = 0;
		
		if( True == isBindJob() ){
			sl_Cnt = pJob->getNumOfDocNeeded()+2;
		}
		else{
			sl_Cnt = JOBD_OVERLAY_RESTORE_MAX;
		}

		// オーバーレイ表面＋裏面情報設定で、リストア未完了の判定
		if(pJob->getNumOfDocRestored() < sl_Cnt)
		{
			// オーバーレイ表面情報設定なしの場合、裏面のデシリアライズ処理を行う。情報設定なしの場合、何もしない。
			if(req_OverLayDeSerializeBack() == True)
			{
				if(sc_TraceLevel>=TRACELEVEL_PRINT){
					JobTraceF6("JOBC_DocDeSerializer::checkReDeSerializeDoc (Overlay)::OverLay_2 NumOfDocNeeded()=[%d],NumOfDocRestored()=[%d]",
						pJob->getNumOfDocNeeded(),pJob->getNumOfDocRestored(),0,0,0,0,pJob);
				}
				return True;
			}
		}

		if(sc_TraceLevel>=TRACELEVEL_PRINT){
			JobTraceF6("JOBC_DocDeSerializer::checkReDeSerializeDoc (Overlay)::OverLay_3 NumOfDocNeeded()=[%d],NumOfDocRestored()=[%d] sl_Cnt=[%d] ",
				pJob->getNumOfDocNeeded(),pJob->getNumOfDocRestored(),sl_Cnt,0,0,0,pJob);
		}
		if(sc_TraceLevel>=TRACELEVEL_PRINT) JobTrace("JOBC_DocDeSerializer::checkReDeSerializeDoc End!!",pJob);

		return False;
	}

	if( DOCD_RetOk != sl_result ){
	}
	else{
		// DOC数更新
		pJob->incrementNumOfDocRestored();

		if(sc_TraceLevel>=TRACELEVEL_PRINT){
			JobTraceF6("JOBC_DocDeSerializer::checkReDeSerializeDoc (Overlay)::OverLay_4 NumOfDocNeeded()=[%d],NumOfDocRestored()=[%d]",
				pJob->getNumOfDocNeeded(),pJob->getNumOfDocRestored(),0,0,0,0,pJob);
		}

		if( pJob->getNumOfDocNeeded() == pJob->getNumOfDocRestored() ){
			// 展開終了

			if ((True == isBindJob()) &&
				(False == pJob->getJobSpec()->pc_Common->c_JobType.getProgramJobOn()))
			{
				// Bind-連結
				unsigned char num=1;
				while(NULL != pJob->getBoxToPrintDoc(num)){
					int32_t DocRtn = pJob->getBoxToPrintDoc(0)->addByPtr(pJob->getBoxToPrintDoc(num));
					if( DOCD_RetOk != DocRtn ){
						JobTrace("JOBC_DocDeSerializer::checkReDeSerializeDoc addByPtr-NG!!",pJob);
						// ジョブ破棄・システムエラー発生
						deleteInstance(DocRtn);
						return False;
					}
					num++;
				}
			}
			
			// BoxToJobの仕上がりプレビューにてオーバーレイの設定がされている場合はもう一回シリアライズする 
			if(req_OverLayDeSerialize() == True)
			{
				if(sc_TraceLevel>=TRACELEVEL_PRINT){
					JobTraceF6("JOBC_DocDeSerializer::checkReDeSerializeDoc (Overlay)::OverLay_5 NumOfDocNeeded()=[%d],NumOfDocRestored()=[%d]",
						pJob->getNumOfDocNeeded(),pJob->getNumOfDocRestored(),0,0,0,0,pJob);
				}
				return True;
			}
			// オーバーレイ表面情報設定なしの場合、裏面のデシリアライズ処理を行う。情報設定なしの場合、何もしない。
			if(req_OverLayDeSerializeBack() == True)
			{
				if(sc_TraceLevel>=TRACELEVEL_PRINT){
					JobTraceF6("JOBC_DocDeSerializer::checkReDeSerializeDoc (OverlayBack)::OverLay_6 NumOfDocNeeded()=[%d],NumOfDocRestored()=[%d]",
						pJob->getNumOfDocNeeded(),pJob->getNumOfDocRestored(),0,0,0,0,pJob);
				}
				return True;
			}

			// DBGLOG
			if(sc_TraceLevel>=TRACELEVEL_PRINT) JobTrace("JOBC_DocDeSerializer::checkReDeSerializeDoc End!!",pJob);
		}
		else{
			// 展開未終了
			if( True == checkDocumentPage()){
				return True;
			}
		}
	}
	return False;
}
//概要    ：オーバーレイのCompositeDoc解放＆初期 
//作成者  ：13開発 榎本(SKY) 
//レビュー：未実施（レビュー実施日と名前を記述） 
//変更履歴：変更日、変更者、変更内容（変更理由）を記述 
void 
JOBC_DocDeSerializer::clearOverLayDoc()
{
	int32_t slDocRtn = DOCD_RetOk;	// DOC戻り値 

	// 仕上がりプレビューのボックス状態ではない場合、何もしない 
	switch(pJob->getBoxJobState())
	{
	case JOBE_BoxJobState_Preview_Requesting	:	// メモリ展開要求中の場合 
	case JOBE_BoxJobState_Preview_Restoring		:	// メモリ展開中の場合 
	case JOBE_BoxJobState_Preview_Completed		:	// メモリ展開完了の場合 
	case JOBE_BoxJobState_Preview_CancelReq		:	// メモリ破棄要求中の場合 
		break;
	default										:	// その他の場合 
		return;
	}
	// CompositeDoc数ループ 
	for(int32_t slCnt=0; slCnt<JOBD_JOBCCATCHCOMPOSITEDOC_MAX; slCnt++)
	{
		// CompositeDoc取得異常の場合、全てのドキュメントチェック完了とみなし終了 
		if(pJob->getBoxToPrintDoc(slCnt) == NULL)
		{
			break;
		}
		// 指定されたCompositeDocがオーバーレイBoxではない場合、何もしない 
		if(pJob->getBoxToPrintDoc(slCnt)->getBoxID() != DOCD_OverLayBox)
		{
			continue;
		}
		// オーバーレイクローズ 
		slDocRtn = pJob->getBoxToPrintDoc(slCnt)->closeDoc();
		if(slDocRtn != DOCD_RetOk)
		{
			JobTraceF6("JOBC_DocDeSerializer::clearOverLayDoc closeDoc NG!!(Rtn=%d)",
				slDocRtn,0,0,0,0,0,pJob
			);
		}
		// インスタンス削除 
		pJob->getBoxToPrintDoc(slCnt)->deleteInstance();
		// Compositeポインタクリア 
		pJob->setBoxToPrintDoc(NULL, slCnt);
	}
	return;
}

// 概要　　：BoxToJobの仕上がりプレビュー用オーバーレイのシリアライズ処理 
// 戻り値　：True:req_deSerialize実行、False:req_deSerialize不要
// 備考　　：
// 作成者　：
// レビュー：
// 変更履歴：変更日、変更者、変更内容（変更理由）を記述
Bool
JOBC_DocDeSerializer::req_OverLayDeSerialize()
{
	int32_t slDocRtn = DOCD_RetOk;		// Doc戻り値 
	int32_t	slCnt = 0;					// カウンタ 
	DOCC_BoxIf * pBox = NULL;			// ボックスポインタ 
	DOCC_CompositeDoc * pDoc = NULL;	// ドキュメントポインタ 
	TYPS_Overlay sOverlay;				// オーバーレイ情報 
	TYPC_Overlay cOverlay;				// 登録オーバーレイ情報 

	// 仕上がりプレビューのボックス状態ではない場合、何もしない 
	switch(pJob->getBoxJobState())
	{
	case JOBE_BoxJobState_Preview_Requesting	:	// メモリ展開要求中の場合 
	case JOBE_BoxJobState_Preview_Restoring		:	// メモリ展開中の場合 
	case JOBE_BoxJobState_Preview_Completed		:	// メモリ展開完了の場合 
	case JOBE_BoxJobState_Preview_CancelReq		:	// メモリ破棄要求中の場合 
		break;
	default										:	// その他の場合 
		return False;
	}
	// オーバーレイ情報取得 
	sOverlay = pJob->getJobSpec()->pc_Common->getOverlay();
	// オーバーレイが指定されていない場合、何もしない 
	if(sOverlay.e_Overlay != TYPE_OverlayOn_Load)
	{
		return False;
	}
	// 不揮発から登録オーバーレイ情報を取得 
	// 呼び出しオーバーレイのIDが範囲内(1～101)かどうかをチェック.
	if ((0 < sOverlay.uc_ID) && (NVDD_MAX_OperationSettingOverlay >= sOverlay.uc_ID))
	{
		// 登録オーバーレイ情報を取得.
		APIC_OperationSetting::newInstance()->getOverlay(sOverlay.uc_ID,&cOverlay);
	}

	// オーバーレイのCompositeDoc格納場所を探す 
	for(slCnt=0; slCnt<JOBD_JOBCCATCHCOMPOSITEDOC_MAX; slCnt++)
	{
		// 空きCompositeDocがあった場合 
		if(pJob->getBoxToPrintDoc(slCnt) == NULL)
		{
			break;
		}
	}
	// 空きCompositeDocを取得異常の場合、何もしない 
	if( (slCnt <= 0)								||	// コンポジットDocが何も設定されていない場合 
		(slCnt >= JOBD_JOBCCATCHCOMPOSITEDOC_MAX))		// 空きコンポジットDocがない場合 
	{
		JobTraceF6("JOBC_DocDeSerializer::req_OverLayDeSerialize Getting BlankCompositeDoc Error!!(Cnt=%d)",
			static_cast<int>(slCnt),0,0,0,0,0,pJob);
		return False;
	}
	// 空きページ数チェック(オーバーレイの1ページをメモリ上に展開可能かチェック) 
	if(DOCD_MaxPage - DOCC_Leaf::getUsedLeafInstCnt() <= 0)
	{
		JobTraceF6("JOBC_DocDeSerializer::req_OverLayDeSerialize No BlankPage!!",
			0,0,0,0,0,0,pJob);
		return False;
	}

	// オーバーレイボックスのインスタンスを生成 
	pBox = DOCC_BoxIf::newInstance(DOCD_OverLayBox);
	if( NULL == pBox ){
		JobTraceF6("JOBC_DocDeSerializer::req_OverLayDeSerialize OverLay Box Create Instance Error!!",
			0,0,0,0,0,0,pJob);
		return False;
	}
	// ドキュメントのインスタンス生成 
	pDoc = DOCC_CompositeDoc::newInstance();
	if( NULL == pDoc )
	{
		JobTraceF6("JOBC_DocDeSerializer::req_OverLayDeSerialize OverLay Doc Create Instance Error!!",
			0,0,0,0,0,0,pJob);
		pBox->deleteInstance();											// オーバーレイボックスのインスタンス削除 
		return False;
	}
	// コンポジットドック設定 
	pJob->setBoxToPrintDoc(pDoc, slCnt);
	// 連結pBox->getDoc() 
	slDocRtn = pBox->getDoc(pJob->getBoxToPrintDoc(slCnt), cOverlay.sl_DocID);
	if( DOCD_RetOk != slDocRtn )
	{
		JobTraceF6("JOBC_DocDeSerializer::req_OverLayDeSerialize OverLay Doc getDocError!!(Ret=%d)",
			static_cast<int>(slDocRtn),0,0,0,0,0,pJob);
		pDoc->deleteInstance();						// ドキュメントのインスタンス削除 
		pJob->setBoxToPrintDoc(NULL, slCnt);		// ドキュメント情報削除 
		pBox->deleteInstance();						// オーバーレイボックスのインスタンス削除 
		return False;
	}
	// OpenDoc 
	slDocRtn = pJob->getBoxToPrintDoc(slCnt)->openDoc();
	if((DOCD_RetOk != slDocRtn) && (APID_DOCD_RetDocOpened != slDocRtn))
	{
		JobTraceF6("JOBC_DocDeSerializer::req_OverLayDeSerialize OverLay Doc Open Error!!(Ret=%d)",
			static_cast<int>(slDocRtn),0,0,0,0,0,pJob);
		pDoc->deleteInstance();						// ドキュメントのインスタンス削除 
		pJob->setBoxToPrintDoc(NULL, slCnt);		// ドキュメント情報削除 
		pBox->deleteInstance();						// オーバーレイボックスのインスタンス削除 
		return False;
	}
	// オーバーレイボックスのインスタンス削除 
	pBox->deleteInstance();
	// シリアライズ要求 
	slDocRtn = pJob->getBoxToPrintDoc(slCnt)->req_DeSerialize(
		cOverlay.sl_DocID, DOCD_OverLayBox, pf_Ans_DeSerialize, pJob);
	if(DOCD_RetOk != slDocRtn)
	{
		JobTraceF6("JOBC_DocDeSerializer::req_OverLayDeSerialize OverLay Doc Serialize Error!!(Ret=%d)",
			static_cast<int>(slDocRtn),0,0,0,0,0,pJob);
		// Docクローズ(クローズ異常の場合、メッセージのみ出力) 
		slDocRtn = pDoc->closeDoc();
		if(DOCD_RetOk != slDocRtn)
		{
			JobTraceF6("JOBC_DocDeSerializer::req_OverLayDeSerialize OverLay Doc !! Close Error(Ret=%d)",
				static_cast<int>(slDocRtn),0,0,0,0,0,pJob);
		}
		pDoc->deleteInstance();						// ドキュメントのインスタンス削除 
		pJob->setBoxToPrintDoc(NULL, slCnt);		// ドキュメント情報削除 
		return False;
	}
	return True;
}

// 概要　　：BoxToJobの仕上がりプレビュー用オーバーレイ(裏面）のシリアライズ処理
// 戻り値　：True:req_deSerialize実行、False:req_deSerialize不要
// 備考　　：
// 作成者　：
// レビュー：
// 変更履歴：変更日、変更者、変更内容（変更理由）を記述
Bool
JOBC_DocDeSerializer::req_OverLayDeSerializeBack()
{
	int32_t slDocRtn = DOCD_RetOk;		// Doc戻り値 
	int32_t	slCnt = 0;					// カウンタ 
	DOCC_BoxIf * pBox = NULL;			// ボックスポインタ 
	DOCC_CompositeDoc * pDoc = NULL;	// ドキュメントポインタ 
	TYPS_Overlay sOverlayBack;			// オーバーレイ情報 
	TYPC_Overlay cOverlayBack;			// 登録オーバーレイ情報 

	// 仕上がりプレビューのボックス状態ではない場合、何もしない 
	switch(pJob->getBoxJobState())
	{
	case JOBE_BoxJobState_Preview_Requesting	:	// メモリ展開要求中の場合 
	case JOBE_BoxJobState_Preview_Restoring		:	// メモリ展開中の場合 
	case JOBE_BoxJobState_Preview_Completed		:	// メモリ展開完了の場合 
	case JOBE_BoxJobState_Preview_CancelReq		:	// メモリ破棄要求中の場合 
		break;
	default										:	// その他の場合 
		return False;
	}

	// オーバーレイ情報取得 
	sOverlayBack = pJob->getJobSpec()->pc_Common->getOverlayBack();

	// オーバーレイが指定されていない場合、何もしない 
	if(sOverlayBack.e_Overlay != TYPE_OverlayOn_Load)
	{
		return False;
	}

	// 不揮発から登録オーバーレイ情報を取得 
	// 呼び出しオーバーレイのIDが範囲内(1～101)かどうかをチェック.
	if ((0 < sOverlayBack.uc_ID) && (NVDD_MAX_OperationSettingOverlay >= sOverlayBack.uc_ID))
	{
		// 登録オーバーレイ情報を取得.
		APIC_OperationSetting::newInstance()->getOverlay(sOverlayBack.uc_ID,&cOverlayBack);
	}

	// オーバーレイのCompositeDoc格納場所を探す 
	for(slCnt=0; slCnt<JOBD_JOBCCATCHCOMPOSITEDOC_MAX; slCnt++)
	{
		// 空きCompositeDocがあった場合 
		if(pJob->getBoxToPrintDoc(slCnt) == NULL)
		{
			break;
		}
	}

	// 空きCompositeDocを取得異常の場合、何もしない 
	if( (slCnt <= 0)								||	// コンポジットDocが何も設定されていない場合 
		(slCnt >= JOBD_JOBCCATCHCOMPOSITEDOC_MAX))		// 空きコンポジットDocがない場合 
	{
		JobTraceF6("JOBC_DocDeSerializer::req_OverlayDeSerializeBack Getting BlankCompositeDoc Error!!(Cnt=%d)",
			static_cast<int>(slCnt),0,0,0,0,0,pJob);
		return False;
	}
	// 空きページ数チェック(オーバーレイの1ページをメモリ上に展開可能かチェック) 
	if(DOCD_MaxPage - DOCC_Leaf::getUsedLeafInstCnt() <= 0)
	{
		JobTraceF6("JOBC_DocDeSerializer::req_OverlayDeSerializeBack No Blank Page!!",
			0,0,0,0,0,0,pJob);
		return False;
	}

	// オーバーレイボックスのインスタンスを生成 
	pBox = DOCC_BoxIf::newInstance(DOCD_OverLayBox);
	if( NULL == pBox ){
		JobTraceF6("JOBC_DocDeSerializer::req_OverlayDeSerializeBack OverLay Box Create Instance Error!!",
			0,0,0,0,0,0,pJob);
		return False;
	}
	// ドキュメントのインスタンス生成 
	pDoc = DOCC_CompositeDoc::newInstance();
	if( NULL == pDoc )
	{
		JobTraceF6("JOBC_DocDeSerializer::req_OverlayDeSerializeBack OverLay Doc Create Instance Error!!",
			0,0,0,0,0,0,pJob);
		pBox->deleteInstance();											// オーバーレイボックスのインスタンス削除 
		return False;
	}
	// コンポジットドック設定 
	pJob->setBoxToPrintDoc(pDoc, slCnt);

	// 登録オーバーレイ裏面情報のみ場合、Indexの2番目に格納する処理.
	if(pJob->getBoxToPrintDoc(1) == NULL) slCnt++;

	if( slCnt >= JOBD_JOBCCATCHCOMPOSITEDOC_MAX )		// 空きコンポジットDocに収まらない場合 
	{
		JobTraceF6("JOBC_DocDeSerializer::req_OverlayDeSerializeBack OverLay Doc Size Insufficient!!(Ret=%d)",
			static_cast<int>(slDocRtn),0,0,0,0,0,pJob);
		pDoc->deleteInstance();						// ドキュメントのインスタンス削除 
		pBox->deleteInstance();						// オーバーレイボックスのインスタンス削除 
		return False;
	}

	// 連結pBox->getDoc() 
	slDocRtn = pBox->getDoc(pJob->getBoxToPrintDoc(slCnt), cOverlayBack.sl_DocID);
	if( DOCD_RetOk != slDocRtn )
	{
		JobTraceF6("JOBC_DocDeSerializer::req_OverlayDeSerializeBack OverLay Doc getDocError!!(Ret=%d)",
			static_cast<int>(slDocRtn),0,0,0,0,0,pJob);
		pDoc->deleteInstance();						// ドキュメントのインスタンス削除 
		pJob->setBoxToPrintDoc(NULL, slCnt);		// ドキュメント情報削除 
		pBox->deleteInstance();						// オーバーレイボックスのインスタンス削除 
		return False;
	}
	// OpenDoc 
	slDocRtn = pJob->getBoxToPrintDoc(slCnt)->openDoc();
	if((DOCD_RetOk != slDocRtn) && (APID_DOCD_RetDocOpened != slDocRtn))
	{
		JobTraceF6("JOBC_DocDeSerializer::req_OverlayDeSerializeBack OverLay Doc Open Error!!(Ret=%d)",
			static_cast<int>(slDocRtn),0,0,0,0,0,pJob);
		pDoc->deleteInstance();						// ドキュメントのインスタンス削除 
		pJob->setBoxToPrintDoc(NULL, slCnt);		// ドキュメント情報削除 
		pBox->deleteInstance();						// オーバーレイボックスのインスタンス削除 
		return False;
	}
	// オーバーレイボックスのインスタンス削除 
	pBox->deleteInstance();
	// シリアライズ要求 
	slDocRtn = pJob->getBoxToPrintDoc(slCnt)->req_DeSerialize(
		cOverlayBack.sl_DocID, DOCD_OverLayBox, pf_Ans_DeSerialize, pJob);
	if(DOCD_RetOk != slDocRtn)
	{
		JobTraceF6("JOBC_DocDeSerializer::req_OverlayDeSerializeBack OverLay Doc Serialize Error!!(Ret=%d)",
			static_cast<int>(slDocRtn),0,0,0,0,0,pJob);
		// Docクローズ(クローズ異常の場合、メッセージのみ出力) 
		slDocRtn = pDoc->closeDoc();
		if(DOCD_RetOk != slDocRtn)
		{
			JobTraceF6("JOBC_DocDeSerializer::req_OverlayDeSerializeBack OverLay Doc Close Error!!(Ret=%d)",
				static_cast<int>(slDocRtn),0,0,0,0,0,pJob);
		}
		pDoc->deleteInstance();						// ドキュメントのインスタンス削除 
		pJob->setBoxToPrintDoc(NULL, slCnt);		// ドキュメント情報削除 
		return False;
	}
	return True;
}
