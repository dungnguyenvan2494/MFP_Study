// ================================================================================================
//! \file THRC_ThreadSeqCom.cpp スレッド シーケンス層の基本クラス定義 実装 
// ================================================================================================

#include "THRC_ThreadSeqCom.h"
#include "system/thr/thrInclude/THRC_ThreadInstance.h"
#include "system/thr/thrcom/THRC_ResourceCommon.h"

// ================================================================================================
//! <!--機能--> コンストラクタ 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし 
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2005/05/20 橋本昌 	新規作成 
// 2007/05/16 Fujii(Sky) サブシーケンスID追加対応 
// ================================================================================================
THRC_ThreadSeqCom::THRC_ThreadSeqCom()
//	→ 静的解析対応(CIM23820) 2011/05/23 saka.
	: uc_MySeqId(0),uc_MySubSeqId(0),e_DivTransMode( THRE_DivTransModeNon ),ps_CancelReqPara(NULL)
//	← 静的解析対応.
{
}


// ================================================================================================
//! <!--機能--> デストラクタ 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし 
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2005/05/20 橋本昌 	新規作成 
// ================================================================================================
THRC_ThreadSeqCom::~THRC_ThreadSeqCom()
{
}

// ================================================================================================
//! <!--機能--> シーケンス処理を開始する  
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param const ST_IJCIFMsgBox& ... 
//! \param EV_TaskID ............... 
//! \param THRE_ThrID .............. 
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n
//! <!--履歴-->
//  05/07/25 松永	 新規作成
// ================================================================================================
void THRC_ThreadSeqCom::start( const ST_IJCIFMsgBox& , EV_TaskID, THRE_ThrID )
{
	THRL_ThAssert( False, "THRC_ThreadSeqCom::start(): function is not defined.", 0, 0, 0, 0, CFA14_DetailCode0x0585 );
}

// ================================================================================================
//! <!--機能--> シーケンス処理を中止する   
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param const ST_IJCIFMsgBox& ... 
//! \param EV_TaskID ............... 
//! \param THRE_ThrID .............. 
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n
//! <!--履歴-->
//  05/07/25 松永	 新規作成
// ================================================================================================
void THRC_ThreadSeqCom::cancel( const ST_IJCIFMsgBox& , EV_TaskID, THRE_ThrID )
{
	THRL_ThAssert( False, "THRC_ThreadSeqCom::cancel(): function is not defined.", 0, 0, 0, 0, CFA14_DetailCode0x0586 );
}

// ================================================================================================
//! <!--機能--> シーケンスID(スレッド識別情報)の設定 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param uchar uc_SeqId ... 
//! <!--戻値-->
//! \return 
//! <!--詳細-->
//! \n
//! <!--履歴-->
//  05/07/08 yokoi	 新規作成
// ================================================================================================
void THRC_ThreadSeqCom::setSeqId( uchar uc_SeqId )
{
	uc_MySeqId = uc_SeqId;
}

// ================================================================================================
//! <!--機能--> シーケンスID(スレッド識別情報)の取得 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし
//! <!--戻値-->
//! \return 
//! <!--詳細-->
//! \n
//! <!--履歴-->
//  05/07/08 yokoi	 新規作成
// ================================================================================================
uchar THRC_ThreadSeqCom::getSeqId()
{
	return ( uc_MySeqId );
}

// ================================================================================================
//! <!--機能--> サブシーケンスID(スレッド識別情報)の設定 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param uchar uc_SeqId ... サブシーケンスID
//! <!--戻値-->
//! \return 
//! <!--詳細-->
//! \n
//! <!--履歴-->
//  07/05/14 Fujii(Sky)	 新規作成
// ================================================================================================
void THRC_ThreadSeqCom::setSubSeqId( uchar uc_SeqSubId )
{
	uc_MySubSeqId = uc_SeqSubId;
}

// ================================================================================================
//! <!--機能--> サブシーケンスID(スレッド識別情報)の取得 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし
//! <!--戻値-->
//! サブシーケンスID \return 
//! <!--詳細-->
//! \n
//! <!--履歴-->
//  07/05/14 Fujii(Sky)	 新規作成
// ================================================================================================
uchar THRC_ThreadSeqCom::getSubSeqId() const
{
	return ( uc_MySubSeqId );
}

// ================================================================================================
//! <!--機能--> シーケンス層の基本クラス  パラメータチェック 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param const ST_IJCIFMsgBox& rs_IJCIFMsgBox ............ 
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2005/06/17 橋本昌	新規作成 
// ================================================================================================
void THRC_ThreadSeqCom::checkSeqParameter( const ST_IJCIFMsgBox& rs_IJCIFMsgBox )	//!< パラメータチェック処理(シーケンス共通) 
{
	
}


// ================================================================================================
//! <!--機能-->
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param const ST_IJCIFMsgBox& rs_IJCIFMsgBox ........
//! \param EV_TaskID ev_ReqThID ........................
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2005/06/16 橋本昌	新規作成 
// 2013/03/18 Sky Emperorプリント1200Dpi対応.
// ================================================================================================
THRS_BufSeqPara THRC_ThreadSeqCom::getSeqPara( const ST_IJCIFMsgBox& rs_IJCIFMsgBox, THRE_ThrID e_ThrID )
{
	THRS_BufSeqPara st_BufSeqPara;
	// 画像転送開始メッセージパラメータを取得 
	st_BufSeqPara.pst_ThreadComPara = (ST_ThreadComPara*)rs_IJCIFMsgBox.pt_Parameter;
	// スレッドIDから画像転送管理情報を取得 
	st_BufSeqPara.pcc_OneImgTransInfo = CC_ImgTransInfoSingletonIf::newInstance().getOneImgTransInfoFromTh( e_ThrID, *st_BufSeqPara.pst_ThreadComPara );
	// 入力ページ情報を取得.
	if( THRE_DivTransModeNon != e_DivTransMode ){
		// 変換分割モードが「０」以外の場合はローカルLeafを使用する.
		st_BufSeqPara.pcc_OneInputPage = CC_ImgTransInfoSingletonIf::newInstance().getLocalLeaf( st_BufSeqPara.pst_ThreadComPara->ss_TransIndexNum, e_DivTransMode );
	} else {
		if( CC_ImgTransInfoSingletonIf::newInstance().isNeedLocalLeaf( e_ThrID, *st_BufSeqPara.pst_ThreadComPara ) == True ){
			st_BufSeqPara.pcc_OneInputPage = CC_ImgTransInfoSingletonIf::newInstance().getLocalLeaf( e_ThrID, *st_BufSeqPara.pst_ThreadComPara );
		} else {
			st_BufSeqPara.pcc_OneInputPage = st_BufSeqPara.pst_ThreadComPara->c_ThreadIF.pc_Leaf;
		}
	}
	
	return st_BufSeqPara;
}

// ================================================================================================
//! <!--機能--> シーケンス層の基本クラス  
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param const ST_IJCIFMsgBox& rs_IJCIFMsgBox ........... 
//! \param EV_TaskID ev_ReqThID ........................... 
//! \param THRS_BufImgInfo& rs_BufImgInfo ................. 
//! \param Bool b_1200dpiImgSizeMode  .................1200dpi相当の画像サイズで転送するモード(現状, InInputSeqのみ使用する).
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2005/06/16 橋本昌	新規作成 
// 2006/02/20 Koshino(Sky) 白紙印字対応 
// 2010/12/15 tadamasa Fiery1200dpi対応.
// 2013/05/10 fukuhara 静的解析対応.
// ================================================================================================
THRS_BufSeqPara THRC_ThreadSeqCom::getSeqParaSetImgComInfo( const ST_IJCIFMsgBox& rs_IJCIFMsgBox, THRE_ThrID e_ThrID, THRS_BufImgInfo& rs_BufImgInfo, Bool b_1200dpiImgSizeMode )
{
	THRS_BufSeqPara st_Para = getSeqPara( rs_IJCIFMsgBox, e_ThrID );
	if(  ( st_Para.pcc_OneInputPage == NULL || st_Para.pcc_OneImgTransInfo == NULL ) 
	  && (  ( st_Para.pst_ThreadComPara->pcc_OneImageOutputMode != NULL )
		 && ( st_Para.pst_ThreadComPara->pcc_OneImageOutputMode->pcc_OneExpandMode->isNonPrint() == False) ) ){
//		noticeFatal( EC_BufSeq_InputExImg );
		THRL_ThAssert( False , "THRC_ThreadSeqCom::getSeqParaSetImgComInfo : Pameter Error !!" , 0, 0, 0, 0, CFA14_DetailCode0x0587 );
	}
	THRL_ThAssert( ( st_Para.pcc_OneImgTransInfo != NULL ), "THRC_ThreadSeqCom::getSeqParaSetImgComInfo(): pcc_OneImgTransInfo NULL error." , 0, 0, 0, 0, CFA14_DetailCode0x0588 );
	if( st_Para.pcc_OneImgTransInfo->bl_PageTrans == True ) {
		rs_BufImgInfo.uc_TopBandNo = 1;	// ページ単位の転送時は１
	} else {
		rs_BufImgInfo.uc_TopBandNo = (uchar)st_Para.pst_ThreadComPara->ss_BandID;	// バンド単位の先頭バンド番号設定する　
	}
	
	if( st_Para.pst_ThreadComPara->pcc_OneImageOutputMode == NULL ) {
		THRL_ThAssert( (st_Para.pcc_OneInputPage != NULL) , "THRC_ThreadSeqCom::getSeqParaSetImgComInfo : pcc_OneInputPage NULL error." , 0, 0, 0, 0, CFA14_DetailCode0x0BAD );
		// 出力ではないときの設定
		setBufTransImgInfoIn( rs_BufImgInfo, st_Para.pcc_OneInputPage, st_Para.pcc_OneImgTransInfo, st_Para.pst_ThreadComPara->ss_BandID, b_1200dpiImgSizeMode );
	} else {
		// 出力時の設定　
		if( e_ThrID == THRE_ThrID_BufToFile ){
			// 出力シーケンスのJBIG圧縮転送 
			DOCC_Leaf* pcc_LocalLeaf = CC_ImgTransInfoSingletonIf::newInstance().getLocalLeaf( e_ThrID, *st_Para.pst_ThreadComPara );
			setBufTransImgInfoIn( rs_BufImgInfo, pcc_LocalLeaf, st_Para.pcc_OneImgTransInfo, st_Para.pst_ThreadComPara->ss_BandID );
		} else {
			if( isImageConvert( *st_Para.pst_ThreadComPara, e_ThrID ) == False){
				// 画像変換を行わないスレッド転送 
				setBufTransImgInfoOut( rs_BufImgInfo, *st_Para.pst_ThreadComPara, e_ThrID, st_Para.pst_ThreadComPara->ss_BandID );
			} else {
				// 画像変換を行うスレッド転送 
				setBufTransImgInfoOutForImageConvert( rs_BufImgInfo, *st_Para.pst_ThreadComPara, e_ThrID, st_Para.pst_ThreadComPara->ss_BandID );
			}
		}
	}

	return( st_Para );
}

// ================================================================================================
//! <!--機能--> シーケンス層の基本クラス  入力時の入力ページ情報から画像転送管理情報から高速タスクへの転送画像情報の設定  
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRS_BufImgInfo& st_BufImgInfo ..................
//! \param DOCC_Leaf* pcc_OneInputPage ...................
//! \param CC_OneImgTransInfo* pcc_OneImgTransInfo  ......
//! \param short ss_BandID ...............................
//! \param Bool b_1200dpiImgSizeMode .....................
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2005/06/17 橋本昌	新規作成 
// 2006/03/31 松永		入力時の色はorg,出力時の色はresultを使用するよう変更 
// 2009/07/16 yashiro Emperorプリント1200dpi対応.
// 2009/11/12 takamoto CQ322175 getImage関数のコール数削減対応.
// 2010/12/15 tadamasa Fiery1200dpi対応.
// 2013/04/08 Sky Fieryプリント1200Dpi対応.
// ================================================================================================
void THRC_ThreadSeqCom::setBufTransImgInfoIn( THRS_BufImgInfo& rs_BufImgInfo, DOCC_Leaf* pcc_OneInputPage, CC_OneImgTransInfo* pcc_OneImgTransInfo, short ss_BandID, Bool b_1200dpiImgSizeMode )
{
	ushort us_alignWidth = pcc_OneImgTransInfo->us_BitWidthUnit;

	// 画像情報の設定 
	rs_BufImgInfo.uc_MaxColorNum = pcc_OneImgTransInfo->uc_AllColorNum;	// カラー数 
	rs_BufImgInfo.uc_MaxAttBitNum = pcc_OneImgTransInfo->uc_AllAttNum;		// 全属性プレーン数 
	rs_BufImgInfo.uc_AllPlaneNum = pcc_OneImgTransInfo->uc_AllPlaneNum;	// 全プレーン数 
	
	IMGC_Image* pc_Image = pcc_OneInputPage->getImage();
	THRL_ThAssert( ( pc_Image != NULL ), "THRC_ThreadSeqCom::setBufTransImgInfoIn(): getImage NULL error." , 0, 0, 0, 0, CFA14_DetailCode0x0589 );
	// 06/03/31 松永 入力時の色はorg,出力時の色はresultを使用するようにする 
	if( pcc_OneImgTransInfo->getThreadInOut() == DEF_InputThread ){
		rs_BufImgInfo.e_ImageColor = pc_Image->c_ImageInfo.ec_ImageColorOrg;			// 画像カラー情報(Lab,YMCK,RGB...)
	}else{
		rs_BufImgInfo.e_ImageColor = pc_Image->c_ImageInfo.ec_ImageColorResult;			// 画像カラー情報(Lab,YMCK,RGB...)
	}
	rs_BufImgInfo.e_ImageTone = pc_Image->c_ImageInfo.ec_ImageTone;					// 画像階調情報(1bit-4bit,8bit)
	rs_BufImgInfo.e_ImageCompress = pc_Image->c_ImageInfo.ec_ImageCompressResult;		// 画像圧縮情報(BTC333,BTC+JBIG...)
	rs_BufImgInfo.us_alignWidth = pcc_OneImgTransInfo->us_BitWidthUnit;										// 画像横幅設定単位(dot単位) 

	if( b_1200dpiImgSizeMode == False ){
		// 転送元の画像横幅 
		//! 解像度が1200dpiの場合、600dpi相当に変換する(1200dpiでなければ、変換しない).
		rs_BufImgInfo.ul_SrcImageWidth = 
				_roundup( 
					(uint32_t)(theModeInfoLib.ReCalcResolution_Dot( pc_Image->c_ImageInfo.ct_TransSize.x, pc_Image->c_ImageInfo.c_Resolution.getDpi().x ) ), 
					(uint32_t)us_alignWidth ) * us_alignWidth;
		// 転送先の画像横幅 
		//! 解像度が1200dpiの場合、600dpi相当に変換する(1200dpiでなければ、変換しない).
		rs_BufImgInfo.ul_DstImageWidth = 
				_roundup( 
					(uint32_t)(theModeInfoLib.ReCalcResolution_Dot( pc_Image->c_ImageInfo.ct_TransSize.x, pc_Image->c_ImageInfo.c_Resolution.getDpi().x ) ),
					(uint32_t)us_alignWidth ) * us_alignWidth;	
		uint32_t ul_ImgHeight = getBufTransImgInfoInOutHeight( pcc_OneInputPage, NULL, pcc_OneImgTransInfo, ss_BandID, True );
		rs_BufImgInfo.ul_SrcImageHeight = ul_ImgHeight;		// 転送元の画像ライン数
		rs_BufImgInfo.ul_DstImageHeight = ul_ImgHeight;		// 転送先の画像ライン数
	}else{
		//b_1200dpiImgSizeModeがTrueになるのは、InInputSeqから呼ばれた時だけ.
		//InInputSeqから呼ばれた時には1200dpiのまま転送する必要がある.
		//その為、ReCalcResolution_Dot()の解像度の引数を600固定で設定している.
		// 転送元の画像横幅 
		//! 解像度が1200dpiの場合、600dpi相当に変換する(1200dpiでなければ、変換しない).
		rs_BufImgInfo.ul_SrcImageWidth = 
				_roundup( 
					(uint32_t)(theModeInfoLib.ReCalcResolution_Dot( pc_Image->c_ImageInfo.ct_TransSize.x, 600 ) ), 
					(uint32_t)us_alignWidth ) * us_alignWidth;
		// 転送先の画像横幅 
		//! 解像度が1200dpiの場合、600dpi相当に変換する(1200dpiでなければ、変換しない).
		rs_BufImgInfo.ul_DstImageWidth = 
				_roundup( 
					(uint32_t)(theModeInfoLib.ReCalcResolution_Dot( pc_Image->c_ImageInfo.ct_TransSize.x,600 ) ),
					(uint32_t)us_alignWidth ) * us_alignWidth;	
		uint32_t ul_ImgHeight = getBufTransImgInfoInOutHeight( pcc_OneInputPage, NULL, pcc_OneImgTransInfo, ss_BandID, True ,b_1200dpiImgSizeMode);
		rs_BufImgInfo.ul_SrcImageHeight = ul_ImgHeight;		// 転送元の画像ライン数
		rs_BufImgInfo.ul_DstImageHeight = ul_ImgHeight;		// 転送先の画像ライン数
		setFieryThLog( 468, THRE_DebugLogType_ThCtrLog, 3, True,1
						,rs_BufImgInfo.ul_SrcImageWidth,rs_BufImgInfo.ul_DstImageWidth,rs_BufImgInfo.ul_SrcImageHeight,rs_BufImgInfo.ul_DstImageHeight);
	}
}

// ================================================================================================
//! <!--機能--> シーケンス層の基本クラス  バッファからの入出力時のバンド単位の転送を考慮した転送画像ライン数の設定 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param DOCC_Leaf* pcc_OneInputPage .............................
//! \param CC_OneImageOutputMode* pcc_OneImageOutputMode ...........
//! \param CC_OneImgTransInfo* pcc_OneImgTransInfo .................
//! \param short ss_BandID .........................................
//! \param Bool b_1200dpiImgSizeMode ...............................
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2005/06/17 橋本昌	新規作成 
// 2009/07/16 yashiro Emperorプリント1200dpi対応.
// 2009/11/12 takamoto CQ322175 getImage関数のコール数削減対応.
// ================================================================================================
uint32_t THRC_ThreadSeqCom::getBufTransImgInfoInOutHeight( DOCC_Leaf* pcc_OneInputPage, CC_OneImageOutputMode* pcc_OneImageOutputMode, CC_OneImgTransInfo* pcc_OneImgTransInfo, short ss_BandID, Bool bl_InputSeq,Bool b_1200dpiImgSizeMode )
{
	uint32_t ul_ImgHeight;
	// 入力と出力とで画像サイズが異なるので入力、出力で分けて設定 
	if( bl_InputSeq ){	// 入力シーケンス 
	//! 解像度が1200dpiの場合、600dpi相当に変換する(1200dpiでなければ、変換しない).
		IMGC_Image* pc_Image = pcc_OneInputPage->getImage();
		THRL_ThAssert( ( pc_Image != NULL ), "THRC_ThreadSeqCom::getBufTransImgInfoInOutHeight(): getImage NULL error." , 0, 0, 0, 0, CFA14_DetailCode0x058A );
		ul_ImgHeight = theModeInfoLib.ReCalcResolution_Dot( pc_Image->c_ImageInfo.ct_TransSize.y, pc_Image->c_ImageInfo.c_Resolution.getDpi().y );
		if(b_1200dpiImgSizeMode){
			//1200の最終バンドの判断を正しく行う為に修正.
			ul_ImgHeight = ul_ImgHeight*2;
			ss_BandID = (short)(ss_BandID - 1);
		}
	} else {	// 出力シーケンス 
	//! 解像度が1200dpiの場合、600dpi相当に変換する(1200dpiでなければ、変換しない).
		ul_ImgHeight = theModeInfoLib.ReCalcResolution_Dot( pcc_OneImageOutputMode->cc_ToApporExtMode.ct_OutputDotSize.y, pcc_OneImageOutputMode->cc_ToApporExtMode.c_Resolution.getDpi().y );
	}
	if( pcc_OneImgTransInfo->bl_PageTrans == False ) {		
		// バンド単位の転送の時の計算を行う
		if( ss_BandID + pcc_OneImgTransInfo->uc_UnitBandTrans > pcc_OneImgTransInfo->uc_TransBandNum ) {
			// 最終バンドの転送の時
			ul_ImgHeight = ul_ImgHeight % ( pcc_OneImgTransInfo->uc_UnitBandTrans * DEF_BasicTransLine );
			if( ul_ImgHeight == 0 ){
				ul_ImgHeight = pcc_OneImgTransInfo->uc_UnitBandTrans * DEF_BasicTransLine;
			}
		} else {
			ul_ImgHeight = DEF_BasicTransLine * pcc_OneImgTransInfo->uc_UnitBandTrans;
		}
	}
	return( ul_ImgHeight );
}


// ================================================================================================
//! <!--機能--> シーケンス層の基本クラス  
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRS_BufAdrInfo& st_BufAdrInfo ..............【出力】
//! \param ST_ThreadComPara* pst_ThreadComPara .........
//! \param CC_OneImgTransInfo* pcc_OneImgTransInfo .....
//! \param Bool bl_ArrayAdr ............................
//! \param DOCC_Leaf* pcc_OneInputPage .................
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2005/06/16 橋本昌	新規作成 
// 2011/05/03 mutoh 	CQ342693対応	画像退避時(pcc_OneInputPage==NULL)にも静的解析でpcc_OneInputPageにアクセスするため修正.
// 2011/11/28 上田		属性データの取り出し対応.
// 2012/06/21 佐々木	1200dpiパフォーマンス改善(getImage関数のコール数削減).
// 2012/08/29 Gonsho	静的解析対応 pc_Image のアドレス取得タイミング変更。 正常ケースでpcc_OneInputPageがNULLの場合があるため.
// ================================================================================================
void THRC_ThreadSeqCom::setBufAdrInfo( THRS_BufAdrInfo& st_BufAdrInfo, ST_ThreadComPara* pst_ThreadComPara, CC_OneImgTransInfo* pcc_OneImgTransInfo, Bool bl_ArrayAdr, DOCC_Leaf* pcc_OneInputPage )
{
	register uchar uc_ColorNo;
	uchar uc_StartColorNo = 0;	// 設定先頭カラー番号 
	uchar uc_SetColorNum = 0;		// 設定カラー数 
	uchar uc_StartBandNo = 0;	// 設定先頭バンド番号 
	uchar uc_SetBandNum[ THRD_MAXBUF_COLORNUM ];		// 設定バンド数 (転送基準ライン数単位)
	uchar uc_SetPlaneNum[ THRD_MAXBUF_COLORNUM ];	// 設定プレーン数 
	IMGC_Image* pc_Image = NULL;	//Imageインスタンスアドレス.
	
	// 先頭カラー番号、カラー数 
	if( pcc_OneImgTransInfo->bl_AllColor == False ){	// １色単位の転送のとき 
		uc_StartColorNo = (uchar)pst_ThreadComPara->en_ColorElementNumber;	// 先頭カラー番号 
		uc_SetColorNum = 1;
	} else {
		uc_StartColorNo = 0;
		uc_SetColorNum = pcc_OneImgTransInfo->uc_AllColorNum;	// 全色数 
	}

	for( uc_ColorNo = 0; uc_ColorNo < uc_SetColorNum; uc_ColorNo++){	
		// 先頭バンド番号、バンド数 
		if( pcc_OneImgTransInfo->bl_PageTrans == False ){	
			// バンド単位の転送のとき(画像転送管理情報のページ単位の転送か否かで判断する）　 
			uc_StartBandNo = (uchar)( pst_ThreadComPara->ss_BandID - 1 );	// 先頭バンド番号 1 
			uc_SetBandNum[ uc_ColorNo ] = pcc_OneImgTransInfo->uc_UnitBandTrans;
			if( uc_StartBandNo + pcc_OneImgTransInfo->uc_UnitBandTrans > pcc_OneImgTransInfo->uc_TransBandNum ) {
				// バンド番号-1+転送基準ラインの倍数指定が転送バンド数より大きい時、最終バンドの補正を行う
				uc_SetBandNum[ uc_ColorNo ] = (uchar)( uc_StartBandNo + pcc_OneImgTransInfo->uc_UnitBandTrans - pcc_OneImgTransInfo->uc_TransBandNum );
			}
		} else {
			uc_StartBandNo = 0;
			uc_SetBandNum[ uc_ColorNo ] = pcc_OneImgTransInfo->uc_AllocBandNum[ uc_StartColorNo + uc_ColorNo ];
		}
		// 設定プレーン数（色毎に異なるので色数分設定）　 
		uc_SetPlaneNum[ uc_ColorNo ] = pcc_OneImgTransInfo->uc_PlaneNum[ uc_StartColorNo + uc_ColorNo ];
		if( bl_ArrayAdr == False ) {	//【Mosel】ここから
			// アプリアドレスのとき 
			register uchar uc_ImgDataNum = pcc_OneImgTransInfo->uc_AllColorNum;
			if( pcc_OneImgTransInfo->uc_AllAttNum>0) {
				uc_ImgDataNum--;	// 属性色数分を減算 
			}
			if( uc_ColorNo < uc_ImgDataNum ){
				uc_SetPlaneNum[ uc_ColorNo ] = 1;	//アプリアドレスは非圧縮データなので1プレーン 
			} else if( NULL != pcc_OneInputPage ) {		// 20110503 mutoh add CQ342693:正常でもNULLを設定してくることがあり、その場合の静的解析の回避 .
				pc_Image = pcc_OneInputPage->getImage();
				THRL_ThAssert( ( pc_Image != NULL ), "THRC_ThreadSeqCom::setBufAdrInfo(): getImage NULL error." , 0, 0, 0, 0, CFA14_DetailCode0x058B );
				if( pc_Image->c_ImageInfo.bl_TransPixelAtr == True ) {
					uc_SetPlaneNum[ uc_ColorNo ] = 1;	//アプリアドレスは非圧縮データなので1プレーン 				
				}
			}
		}	//【Mosel】ここまでMoselで新規に追加した場所なのでバグが潜んでいる可能性あり 橋本
		// 転送先バッファのプレーン数 
		st_BufAdrInfo.uc_BufPlaneNum[ uc_ColorNo ] = uc_SetPlaneNum[ uc_ColorNo ];	 
		// 転送先バッファの確保済みバンド数 
		st_BufAdrInfo.uc_BufBandNum[ uc_ColorNo ] = uc_SetBandNum[ uc_ColorNo ];	
	}
	setBufInfoOnlyAttTrans( &st_BufAdrInfo, pcc_OneImgTransInfo, pst_ThreadComPara );	// 属性データのみ転送時はバッファ情報を変更.
	st_BufAdrInfo.b_RingBufferMode = pcc_OneImgTransInfo->bl_RingBufferMode;	// リングバッファモードが有効か否か 
	
	if( bl_ArrayAdr == True ) {
		// バッファアレイ情報の設定（バッファメモリ→外部の属性を除く） 
		if(pcc_OneImgTransInfo->en_BufStatContDisCont != EN_BufContBusy){	// バッファメモリ不連続領域確保時 
			setBufAdrArrayInfo( st_BufAdrInfo, pcc_OneImgTransInfo, uc_StartBandNo, uc_StartColorNo, uc_SetColorNum, False );
		}else{		// バッファメモリ連続領域確保時 
			setContBufAdrArrayInfo( st_BufAdrInfo, pcc_OneImgTransInfo, uc_StartBandNo, uc_StartColorNo, uc_SetColorNum, False, pcc_OneInputPage, pst_ThreadComPara );
		}
	} else {
		// アプリアドレスの設定
		setBufAdrAppInfo( st_BufAdrInfo, pst_ThreadComPara, pcc_OneImgTransInfo, uc_StartBandNo, uc_StartColorNo );
	}
}

// ================================================================================================
//! <!--機能-->
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRS_BufAdrInfo& st_BufAdrInfo .............................【出力】
//! \param CC_OneImgTransInfo* pcc_OneImgTransInfo ....................
//! \param uchar uc_StartBandNo .......................................
//! \param uchar uc_StartColorNo ......................................
//! \param uchar uc_SetColorNum .......................................
//! \param Bool bl_BufToExtAttMode ....................................
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2005/06/28 橋本昌	新規作成 
// 2007/04/02 鈴村		C6035→C6014に変更 
// ================================================================================================
void THRC_ThreadSeqCom::setBufAdrArrayInfo( THRS_BufAdrInfo& st_BufAdrInfo, CC_OneImgTransInfo* pcc_OneImgTransInfo, uchar uc_StartBandNo, uchar uc_StartColorNo, uchar uc_SetColorNum, Bool bl_BufToExtAttMode )
{
	// 転送先バッファアドレスの設定 
	CC_OneBufArray* pcc_OneBufArray;	// アレイアドレス計算用
	uchar uc_BufSetColor;				// 高速タスクへのメッセージの色開始番号　
	for( uchar uc_ColorNo = 0; uc_ColorNo < uc_SetColorNum; uc_ColorNo++ ){
		if( bl_BufToExtAttMode == True ) {
			uc_BufSetColor = 1;	// バッファメモリ→外部画像転送時の属性は２色目に設定する　
		} else {
			uc_BufSetColor = uc_ColorNo;	// 先頭色から前詰めで順番に設定する　
		} 
		for( uchar uc_BandNo = 0; uc_BandNo < st_BufAdrInfo.uc_BufBandNum[ uc_BufSetColor ]; uc_BandNo++ ){
			// １色の先頭プレーンのアレイアドレス設定
			pcc_OneBufArray = pcc_OneImgTransInfo->cc_BufBand[ uc_StartBandNo + uc_BandNo ].cc_BufColorBlock[ uc_StartColorNo + uc_ColorNo ].cc_ArrayInfo;
			if( pcc_OneBufArray == NULL ) { DebugForeverFA14( CFA14_DebugFatal, CFA14_DetailCode0x08C6 ); }

			for( uchar uc_PlaneNo = 0; uc_PlaneNo < st_BufAdrInfo.uc_BufPlaneNum[ uc_BufSetColor ]; uc_PlaneNo++ ){
				st_BufAdrInfo.s_BufAddress[ uc_BandNo ].s_Color[ uc_BufSetColor ].ul_PlaneAdd[ uc_PlaneNo ] 
					= pcc_OneBufArray->getArrayAddr();
				if( pcc_OneBufArray == NULL ) { DebugForeverFA14( CFA14_DebugFatal, CFA14_DetailCode0x08C7 ); }
				pcc_OneBufArray = pcc_OneBufArray->next();	// 次のアレイアドレスへ	
			}
		}
	}
}

// ================================================================================================
//! <!--機能-->
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRS_BufAdrInfo& st_BufAdrInfo ..........................【出力】
//! \param CC_OneImgTransInfo* pcc_OneImgTransInfo .................
//! \param uchar uc_StartBandNo ....................................
//! \param uchar uc_StartColorNo ...................................
//! \param uchar uc_SetColorNum ....................................
//! \param Bool bl_BufToExtAttMode .................................
//! \param DOCC_Leaf* pcc_OneInputPage .............................
//! \param ST_ThreadComPara* pst_ThreadComPara .....................
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2005/06/28 橋本昌	新規作成 
// ================================================================================================
void THRC_ThreadSeqCom::setContBufAdrArrayInfo( THRS_BufAdrInfo& st_BufAdrInfo, CC_OneImgTransInfo* pcc_OneImgTransInfo, uchar uc_StartBandNo, uchar uc_StartColorNo, uchar uc_SetColorNum, Bool bl_BufToExtAttMode, DOCC_Leaf* pcc_OneInputPage, ST_ThreadComPara* pst_ThreadComPara )
{
	// Moselでは連続確保はないので後回し 
	// 引数に無駄がおおい 修正するときに洗いなおす 
	THRL_ThAssert( False , "THRC_ThreadSeqCom::setContBufAdrArrayInfo : ContBufAdr Error !!" , 0, 0, 0, 0, CFA14_DetailCode0x058C );
}

// ================================================================================================
//! <!--機能-->
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRS_BufAdrInfo& st_BufAdrInfo ..........................【出力】.
//! \param ST_ThreadComPara* pst_ThreadComPara .....................
//! \param CC_OneImgTransInfo* pcc_OneImgTransInfo .................
//! \param uchar uc_StartBandNo ....................................
//! \param uchar uc_StartColorNo ...................................
//! <!--戻値-->
//! \return なし.
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2005/06/28 橋本昌	新規作成.
// 2006/05/20 忠政		サムネイル表示時に異常画像になる問題対応.
// 2006/09/25 Fujii(Sky)	長尺プリントで無画像バンド対応.
// 2007/09/11 takamoto	白紙プレビュー時のカラー情報取得対応.
// 2009/11/12 takamoto CQ322175 getImage関数のコール数削減対応.
// 2010/02/12 岡田		CQ323987 白紙+文字合成で、BufToApp転送時Access Violationが発生する問題点対応.
// 2011/11/28 上田		属性データのみ転送時は1色目と3色目を入れ替えないように対応.
// 2011/12/20 堀下		CQ359297(FAX受信でFA14)対応.
// 2020/07/09 日立		OP_BTS-18777 無画像バンドフラグ問題.
// ================================================================================================
void THRC_ThreadSeqCom::setBufAdrAppInfo( THRS_BufAdrInfo& st_BufAdrInfo, ST_ThreadComPara* pst_ThreadComPara, CC_OneImgTransInfo* pcc_OneImgTransInfo, uchar uc_StartBandNo, uchar uc_StartColorNo )
{
	THRL_ThAssert( (pst_ThreadComPara != NULL), "THRC_ThreadSeqCom::setBufAdrAppInfo pst_ThreadComParaErr" , 0, 0, 0, 0, CFA14_DetailCode0x058D );
	THRL_ThAssert( (pcc_OneImgTransInfo != NULL), "THRC_ThreadSeqCom::setBufAdrAppInfo pcc_OneImgTransInfoErr" , 0, 0, 0, 0, CFA14_DetailCode0x058E );

	THRE_ThrID e_ThrID = pcc_OneImgTransInfo->en_ThreadID;
	uchar uc_SwapColor = 0;	// 1色目と3色目入れ替えない.

	if( e_ThrID == THRE_ThrID_AppToBuf ){
		DOCC_Leaf* pcc_OneInputPage = pst_ThreadComPara->c_ThreadIF.pc_Leaf;
		THRL_ThAssert( ( pcc_OneInputPage != NULL ), "THRC_ThreadSeqCom::setBufAdrAppInfo pcc_OneInputPageErr" , 0, 0, 0, 0, CFA14_DetailCode0x058F );
		IMGC_Image* pc_Image = pcc_OneInputPage->getImage();
		THRL_ThAssert( ( pc_Image != NULL ), "THRC_ThreadSeqCom::setBufAdrAppInfo(): getImage NULL error." , 0, 0, 0, 0, CFA14_DetailCode0x0590 );
		if(( pc_Image->c_ImageInfo.ec_ImageColorOrg == IMGE_ImageColor_RGB		)		// ＲＧＢ.
		|| ( pc_Image->c_ImageInfo.ec_ImageColorOrg == IMGE_ImageColor_RGB_ATT1	)		// ＲＧＢ(属性1bit付加).
		|| ( pc_Image->c_ImageInfo.ec_ImageColorOrg == IMGE_ImageColor_RGB_ATT2	)){		// ＲＧＢ(属性2bit付加).
			uc_SwapColor = 1;	// 1色目と3色目入れ替る BGRの順番を RGBにする.
			// アプリ入力でRGBのとき スレッドの内部の画像はBGRで扱うため.
		}
	}
	else if( e_ThrID == THRE_ThrID_BufToApp ){
		// 2011/12/20 堀下 CQ359297対応.
		THRL_ThAssert( (pst_ThreadComPara->pcc_OneImageOutputMode != NULL), "THRC_ThreadSeqCom::setBufAdrAppInfo pcc_OneImageOutputModeErr", 0, 0, 0, 0, CFA14_DetailCode0x0A23 );

		if( ( pst_ThreadComPara->pcc_OneImageOutputMode->cc_ToApporExtMode.b_OnlyAttTrans == False )
		&& (( theColorInformation.getNin1ImageColor( *pst_ThreadComPara, e_ThrID, pst_ThreadComPara->pcc_OneImageOutputMode->pcc_OneExpandMode ) == IMGE_ImageColor_RGB				)
		 || ( theColorInformation.getNin1ImageColor( *pst_ThreadComPara, e_ThrID, pst_ThreadComPara->pcc_OneImageOutputMode->pcc_OneExpandMode ) == IMGE_ImageColor_RGB_ATT4		)
		 || ( theColorInformation.getNin1ImageColor( *pst_ThreadComPara, e_ThrID, pst_ThreadComPara->pcc_OneImageOutputMode->pcc_OneExpandMode ) == IMGE_ImageColor_RGB_ColorCode5	)
		 || ( theColorInformation.getNin1ImageColor( *pst_ThreadComPara, e_ThrID, pst_ThreadComPara->pcc_OneImageOutputMode->pcc_OneExpandMode ) == IMGE_ImageColor_RGB_ColorCode6	))){
			uc_SwapColor = 1;	// 1色目と3色目入れ替る BGRの順番を RGBにする.
			// アプリ出力でRGBの時 スレッドの画像は BGRの順番を考慮.
		}
	}

//Str【OP_BTS-18777 : 2020.07.09】.
	Bool b_NoImage = True;
//End【OP_BTS-18777 : 2020.07.09】.
	for( uchar uc_ColorNo = 0; uc_ColorNo < THRD_MAXBUF_COLORNUM; uc_ColorNo++ ){
		for( uchar uc_BandNo = 0; uc_BandNo < 2; uc_BandNo++ ){
			for(  uchar uc_PlaneNo = 0; uc_PlaneNo < THRD_MAXBUF_PLANENUM; uc_PlaneNo++ ){
				uchar uc_SetColorNo = uc_ColorNo;
				if( uc_SwapColor == 1 ){
					// 1色目と3色目のアドレスを入れ替る処理.
					switch( uc_ColorNo ){
						case 0:
							uc_SetColorNo = 2;	// B.
							break;
						case 1:
							uc_SetColorNo = 1;	// G.
							break;
						case 2:
							uc_SetColorNo = 0;	// R.
							break;
					}
				}

//Str【OP_BTS-18777 : 2020.07.09】.おためし４バンド以下は無画像とする.
//				if( pst_ThreadComPara->ss_BandID > 4 ) {
					st_BufAdrInfo.s_BufAddress[ uc_BandNo ].s_Color[ uc_ColorNo ].ul_PlaneAdd[ uc_PlaneNo ]
						= pst_ThreadComPara->ul_AppAdr[uc_SetColorNo][uc_PlaneNo][uc_BandNo];
//				}
				if( st_BufAdrInfo.s_BufAddress[ uc_BandNo ].s_Color[ uc_ColorNo ].ul_PlaneAdd[ uc_PlaneNo ] != NULL ) {
					b_NoImage = False;	// 画像情報あり判定.
				}
//				if( e_ThrID == THRE_ThrID_AppToBuf && pst_ThreadComPara->pcc_OneImageOutputMode == NULL ) {
//					printf(">>>N=%d/B=%d/C=%d/P=%d/Add=%X\n",pst_ThreadComPara->b_NoImgBand[uc_BandNo],uc_BandNo,uc_ColorNo,uc_PlaneNo, st_BufAdrInfo.s_BufAddress[ uc_BandNo ].s_Color[ uc_ColorNo ].ul_PlaneAdd[ uc_PlaneNo ] );
//				}
//End【OP_BTS-18777 : 2020.07.09】.

// OP_BTS-21249 FAX異常画像問題ログ仕込み start.
				// FAX送信、FAX受信時にアプリアドレスをログに残す.
				if( pcc_OneImgTransInfo->st_TransModePara.en_AppIDSelThread == THRE_AppID_FaxSend ){
					// FAX送信.
					if( st_BufAdrInfo.s_BufAddress[ uc_BandNo ].s_Color[ uc_ColorNo ].ul_PlaneAdd[ uc_PlaneNo ] != NULL ) {
						THRL_SetThLog( 736, THRE_DebugLogType_ThCtrLog, 0, True, uc_BandNo, uc_ColorNo, uc_PlaneNo, 
							st_BufAdrInfo.s_BufAddress[ uc_BandNo ].s_Color[ uc_ColorNo ].ul_PlaneAdd[ uc_PlaneNo ], 0, 0 );
					}
					if( pst_ThreadComPara->ul_AppAdr[uc_SetColorNo][uc_PlaneNo][uc_BandNo] != NULL ) {
						THRL_SetThLog( 738, THRE_DebugLogType_ThCtrLog, 0, True, uc_SetColorNo, uc_PlaneNo, uc_BandNo, 
							pst_ThreadComPara->ul_AppAdr[uc_SetColorNo][uc_PlaneNo][uc_BandNo], 0, 0 );
					}
				}else if ( pcc_OneImgTransInfo->st_TransModePara.en_AppIDSelThread == THRE_AppID_FaxRecv ) {  
					// FAX受信.
					if( st_BufAdrInfo.s_BufAddress[ uc_BandNo ].s_Color[ uc_ColorNo ].ul_PlaneAdd[ uc_PlaneNo ] != NULL ) {
						THRL_SetThLog( 737, THRE_DebugLogType_ThCtrLog, 0, True, uc_BandNo, uc_ColorNo, uc_PlaneNo, 
							st_BufAdrInfo.s_BufAddress[ uc_BandNo ].s_Color[ uc_ColorNo ].ul_PlaneAdd[ uc_PlaneNo ], 0, 0 );
					}
					if( pst_ThreadComPara->ul_AppAdr[uc_SetColorNo][uc_PlaneNo][uc_BandNo] != NULL ) {
						THRL_SetThLog( 739, THRE_DebugLogType_ThCtrLog, 0, True, uc_SetColorNo, uc_PlaneNo, uc_BandNo, 
							pst_ThreadComPara->ul_AppAdr[uc_SetColorNo][uc_PlaneNo][uc_BandNo], 0, 0 );
					}
				}
				
// OP_BTS-21249 FAX異常画像問題ログ仕込み end.
				
				
				//2006/05/22 忠政.
				//サムネイルの表示が異常画像になる問題対応。.
				//出力の時にはダミーアドレスをセットする必要がないのに、ダミーアドレスをセットしていた為、.
				//異常画像となってしまっていた。入力時のみ制限する。.
				if( (pst_ThreadComPara->pcc_OneImageOutputMode == NULL ) && ( pst_ThreadComPara->b_NoImgBand[uc_BandNo] == True)){	// 無画像.
					st_BufAdrInfo.s_BufAddress[ uc_BandNo ].s_Color[ uc_ColorNo ].ul_PlaneAdd[ uc_PlaneNo ] = comMultiThreadProfile.getClearDummyArrayAdr();	// Read専用ダミーのアドレス.
//Str【OP_BTS-18777 : 2020.07.09】.
					b_NoImage = False;	// 画像情報あり判定.
//End【OP_BTS-18777 : 2020.07.09】.
				}
			}
		}
	}
//Str【OP_BTS-18777 : 2020.07.09】.
	if((b_NoImage == True) && (e_ThrID == THRE_ThrID_AppToBuf) && (pst_ThreadComPara->pcc_OneImageOutputMode == NULL)){
		printf(">>>b_NoImgBand From False To True\n");				// 画像情報なし判定.
		for( uchar uc_ColorNo1 = 0; uc_ColorNo1 < THRD_MAXBUF_COLORNUM; uc_ColorNo1++ ){
			for( uchar uc_BandNo1 = 0; uc_BandNo1 < 2; uc_BandNo1++ ){
				pst_ThreadComPara->b_NoImgBand[uc_BandNo1] = True;	// 画像情報なし判定より無画像バンド情報に補正.
				for( uchar uc_PlaneNo1 = 0; uc_PlaneNo1 < THRD_MAXBUF_PLANENUM; uc_PlaneNo1++ ){
					st_BufAdrInfo.s_BufAddress[ uc_BandNo1 ].s_Color[ uc_ColorNo1 ].ul_PlaneAdd[ uc_PlaneNo1 ] = comMultiThreadProfile.getClearDummyArrayAdr();	// Read専用ダミーのアドレス.
				}
			}
		}
	}
//End【OP_BTS-18777 : 2020.07.09】.
}

// ================================================================================================.
//! <!--機能--> 属性データのみ転送時のバッファ情報変更.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param THRS_BufAdrInfo* ps_BufAdrInfo .......................... 【出力】転送元(SDRAM側)バッファ情報.
//! \param CC_OneImgTransInfo* pc_OneImgTransInfo .................. 【入力】画像転送管理情報.
//! \param ST_ThreadComPara* ps_ThreadComPara ...................... 【入力】画像転送開始メッセージパラメータ.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! 属性データのみ転送時は属性データを1色あたり1プレーンの画像データの設定に変更する\n.
//! <!--履歴-->.
// 2011/11/28 上田		新規作成.
// 24/03/22 FPT		Fix build error GccVersionUp.
// ================================================================================================.
void THRC_ThreadSeqCom::setBufInfoOnlyAttTrans( THRS_BufAdrInfo* ps_BufAdrInfo,
                                                CC_OneImgTransInfo* pc_OneImgTransInfo,
                                                ST_ThreadComPara* ps_ThreadComPara )
{
	uchar uc_ColorNo = 0;
	
	THRL_ThAssert( (pc_OneImgTransInfo != NULL),
	               "THRC_ThreadSeqCom::setBufInfoOnlyAttTrans pc_OneImgTransInfo", 0, 0, 0, 0, CFA14_DetailCode0x0A24 );
	THRL_ThAssert( (ps_ThreadComPara != NULL),
	               "THRC_ThreadSeqCom::setBufInfoOnlyAttTrans ps_ThreadComPara", 0, 0, 0, 0, CFA14_DetailCode0x0A25 );

	if(    ( pc_OneImgTransInfo->en_ThreadID == THRE_ThrID_BufToApp )
	    && ( ps_ThreadComPara->pcc_OneImageOutputMode->cc_ToApporExtMode.b_OnlyAttTrans == True ) ){
		// 格納可能な属性データ数上限確認.
		THRL_ThAssert( (pc_OneImgTransInfo->uc_AllAttNum <= THRD_MAXBUF_COLORNUM),
					   "THRC_ThreadSeqCom::setBufInfoOnlyAttTrans uc_AllAttNumErr %d",
					   pc_OneImgTransInfo->uc_AllAttNum, 0, 0, 0, CFA14_DetailCode0x0A26 );
		// 属性データを1色につき1プレーン設定.
		THRL_ThAssert( (ps_BufAdrInfo != NULL),
		               "THRC_ThreadSeqCom::setBufInfoOnlyAttTrans ps_BufAdrInfoErr", 0, 0, 0, 0, CFA14_DetailCode0x0A27 );
		uchar uc_BandNum = ps_BufAdrInfo->uc_BufBandNum[0];
		for( uc_ColorNo = 0; uc_ColorNo < pc_OneImgTransInfo->uc_AllAttNum; uc_ColorNo++ ){
			ps_BufAdrInfo->uc_BufPlaneNum[uc_ColorNo] = 1;
			ps_BufAdrInfo->uc_BufBandNum[uc_ColorNo] = uc_BandNum;
		}
		// 残りは0の設定にする.
		for( ; uc_ColorNo < THRD_MAXBUF_COLORNUM; uc_ColorNo++ ){
			ps_BufAdrInfo->uc_BufPlaneNum[uc_ColorNo] = 0;
			ps_BufAdrInfo->uc_BufBandNum[uc_ColorNo] = 0;
		}
	}
}

// ================================================================================================
//! <!--機能--> シーケンス アドレス設定 PCI側専用.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRS_BufAdrInfo* ps_BufInfo ............... 設定するアドレスの格納場所.
//! \param const THRC_BufferResource* pc_BufResMan ... PCIバッファのリソースマネージャー.
//! \param const THRS_BufImgInfo rs_BufImgInfo ....... 画像情報.
//! \param const Bool b_isWrite ...................... 書き込み時かどうか.
//! \param const Bool b_isBTCComp .................... BTC圧縮かどうか（デフォルト値：false）.
//! <!--戻値-->
//! \return なし.
//! <!--詳細-->
//! \n
//! <!--履歴-->
//  07/06/15 yokoi	 新規作成.
//  07/07/10 yokoi	 2値にも対応.
//	07/08/07 yokoi	 Read時に正確なアドレスがとれない問題を修正.
//	07/08/20 yokoi	 バッファ横幅のパラメータ設定漏れを修正.
//	07/09/14 yokoi	 バンド数を、常にSrcからとってしまっていた問題を修正.
//  09/10/27 越野	 多値合成DMA機能変更対応.
//	09/11/27 小倉	 プレーン数分アドレスを設定するよう修正.
//	09/12/07 岡田	 BTC圧縮時に階調を1bitにするよう修正.
//					 1200dpiのPCプリント時は、rs_BufImgInfo.e_ImageToneの条件で動作する.
//  13/08/09 鈴木    静的解析対応3rd.ID:21890.
//  17/04/28 日立    OP/PP共通化.
// ================================================================================================
void THRC_ThreadSeqCom::setPCIBufAdrInfo(
    THRS_BufAdrInfo* ps_BufInfo,
    const THRC_BufferResource* pc_BufResMan,
    const THRS_BufImgInfo& rs_BufImgInfo,
    Bool b_isWrite,
    Bool b_isBTCComp,
    ushort us_BasicTransLine
)
{
	THRL_ThAssert( (ps_BufInfo != NULL), "THRC_ThreadSeqCom setPCIBufAdrInfo BufInfoErr" , 0, 0, 0, 0, CFA14_DetailCode0x0591 );
	THRL_ThAssert( (pc_BufResMan != NULL), "THRC_ThreadSeqCom setPCIBufAdrInfo BufResManErr" , 0, 0, 0, 0, CFA14_DetailCode0x0592 );
	uint32_t ul_BaseLine = 0;													// 転送開始位置.
	uint32_t ul_BufferWidth = pc_BufResMan->getBufWidth();						// バッファの横幅.
	uint32_t ul_BufferLineNum = pc_BufResMan->getBufLineNum();					// バッファのライン数.
	uchar uc_ColorNum = rs_BufImgInfo.uc_MaxColorNum;
	uint32_t ul_MaxBandNum = 0;
	if( b_isWrite == True ){
		ul_MaxBandNum = _roundup( rs_BufImgInfo.ul_DstImageHeight, us_BasicTransLine );	// １ページバンド数.
	} else {
		ul_MaxBandNum = _roundup( rs_BufImgInfo.ul_SrcImageHeight, us_BasicTransLine );	// １ページバンド数.
	}

	// 階調の設定.
	uchar uc_ToneBit = 8;
	if( rs_BufImgInfo.e_ImageTone == IMGE_ImageTone_1bit ){
		uc_ToneBit = 1;
	}else if( b_isBTCComp == True ){
		uc_ToneBit = 1;
	}

	// 転送元バッファのプレーン数の設定.
	uchar uc_PlaneNum = 8;
	if( rs_BufImgInfo.e_ImageTone == IMGE_ImageTone_1bit ){	
		uc_PlaneNum = 1;
	} else if ( b_isBTCComp == True ){
		// OP/PP共通化.
		if ( IMGE_ImageCompress_BTCxxxAndJBIG == IMGE_ImageCompress_BTC444AndJBIG ){
			uc_PlaneNum = 4;
		}else{
			uc_PlaneNum = 3;
		}		
	}

	// カラー数のチェックを行い、属性によるカラー数の補正を行う.
	THRL_ThAssert( (uc_ColorNum <= THRD_MAXBUF_COLORNUM), "THRC_ThreadSeqCom setPCIBufAdrInfo ColorNumErr %d", uc_ColorNum , 0, 0, 0, CFA14_DetailCode0x0593 );
	if( rs_BufImgInfo.uc_MaxAttBitNum > 0 ){
		uc_ColorNum--;
	}

	// 転送開始位置の設定.
	if( b_isWrite == True ){
		ul_BaseLine = pc_BufResMan->getWriteLineNum();
	} else {
		ul_BaseLine = pc_BufResMan->getReadLineNum();
	}

	// カラーの設定(PCI側なのでプレーン数は1).
	THRL_ThAssert( (ul_MaxBandNum <= THRD_MAX_BANDNUM), "THRC_ThreadSeqCom setPCIBufAdrInfo BandNumErr %"PRId32"", ul_MaxBandNum , 0, 0, 0, CFA14_DetailCode0x0594 );
	for( uchar uc_ColorNo = 0; uc_ColorNo < uc_ColorNum; uc_ColorNo++ ){
		// 生データの場合、多値の場合には1色8bitなので各カラーの先頭プレーンは8プレーン相当ごとになる. 
		// BTC圧縮データの場合、多値の場合には各カラーの先頭プレーンは3プレーンごとになる. 
		for( uchar uc_PlaneNo = 0; uc_PlaneNo < uc_PlaneNum; uc_PlaneNo++ ) {
			uintptr_t ul_BaseAddr = pc_BufResMan->getAddr( (uchar)(uc_ColorNo * uc_PlaneNum + uc_PlaneNo) );
			for( uchar ucBandNo = 0; ucBandNo < ul_MaxBandNum; ucBandNo++ ){
				if ( uc_PlaneNo <  THRD_MAXBUF_PLANENUM ) {
					uint32_t ul_OffsetLine = (ul_BaseLine + (ucBandNo * DEF_BasicTransLine)) % ul_BufferLineNum;
					ps_BufInfo->s_BufAddress[ ucBandNo ].s_Color[ uc_ColorNo ].ul_PlaneAdd[ uc_PlaneNo ] = ul_BaseAddr + (ul_OffsetLine * ul_BufferWidth / (8/uc_ToneBit));
				}
			}
		}
		ps_BufInfo->uc_BufBandNum[uc_ColorNo] = (uchar)ul_MaxBandNum;
		if( b_isBTCComp == False ){
			ps_BufInfo->uc_BufPlaneNum[uc_ColorNo] = 1;
		} else {
			ps_BufInfo->uc_BufPlaneNum[uc_ColorNo] = uc_PlaneNum;
		}
	}

	// 属性の設定 
	if( rs_BufImgInfo.uc_MaxAttBitNum > 0 ){
		THRL_ThAssert( (rs_BufImgInfo.uc_MaxAttBitNum <= THRD_MAXBUF_PLANENUM), "THRC_ThreadSeqCom setPCIBufAdrInfo PlaneNumErr %d", rs_BufImgInfo.uc_MaxAttBitNum , 0, 0, 0, CFA14_DetailCode0x0595 );
		for( uchar uc_AttPlaneNo = 0; uc_AttPlaneNo < rs_BufImgInfo.uc_MaxAttBitNum; uc_AttPlaneNo++ ){
			uintptr_t ul_BaseAddr = pc_BufResMan->getAddr( (uchar)((uc_ColorNum * uc_PlaneNum) + uc_AttPlaneNo) );
			for( uchar ucBandNo = 0; ucBandNo < ul_MaxBandNum; ucBandNo++ ){
				uint32_t ul_OffsetLine = (ul_BaseLine + (ucBandNo * DEF_BasicTransLine)) % ul_BufferLineNum;
				ps_BufInfo->s_BufAddress[ ucBandNo ].s_Color[ uc_ColorNum ].ul_PlaneAdd[ uc_AttPlaneNo ] = ul_BaseAddr + (ul_OffsetLine * ul_BufferWidth / 8);
			}
		}
		ps_BufInfo->uc_BufBandNum[uc_ColorNum] = (uchar)ul_MaxBandNum;
		ps_BufInfo->uc_BufPlaneNum[uc_ColorNum] = rs_BufImgInfo.uc_MaxAttBitNum;	
	}
	// バッファ横幅の設定 
	ps_BufInfo->ul_BufferWidth = ul_BufferWidth;
	// フルバッファもあるが、利用しない変数なので常にFalseとしておく 
	ps_BufInfo->b_RingBufferMode = False;
}

// ================================================================================================
//! <!--機能--> シーケンス層の基本クラス  バッファID 検索 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param CC_OneImgTransInfo* pcc_OneImgTransInfo ....................
//! \param THRE_BufId e_BufID .........................................
//! \param Bool b_Assert .............................................. デフォルト(True),  True: Assetする ,  False: 指定された BufIDが見つからなくても Assertしない 
//! <!--戻値-->
//! \return uchar uc_BufIndex  THRD_MAX_BUF_USE のとき見つからなかった 
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2005/07/09 橋本昌	新規作成 
// ================================================================================================
uchar THRC_ThreadSeqCom::getBufIndex( CC_OneImgTransInfo* pcc_OneImgTransInfo, THRE_BufId e_BufID, Bool b_Assert )
{
	register uchar uc_BufIndex = 0;
	while( uc_BufIndex < THRD_MAX_BUF_USE && pcc_OneImgTransInfo->s_usedBuf[ uc_BufIndex ].e_usedBuf != e_BufID ){
		uc_BufIndex++;
	}
	
	if( b_Assert == True && uc_BufIndex >= THRD_MAX_BUF_USE ){
		THRL_ThAssert( False, "getBufIndex :  not found ThreadID ( %d ), BufID ( %d )", pcc_OneImgTransInfo->en_ThreadID, e_BufID , 0, 0, CFA14_DetailCode0x0596 );
	}
	
	return uc_BufIndex;
}


// ================================================================================================
//! <!--機能-->  シーケンス層の基本クラス 画像主走査サイズ伸張情報から取得 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param CC_OneExpandMode CC_OneExpandMode .......
//! \param pc_OneExpandMode .......................
//! \param CC_OneImgTransInfo CC_OneImgTransInfo ...
//! \param pcc_OneImgTransInfo ....................
//! \param ushort us_alignWidth ....................
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2005/07/13 橋本昌	新規作成 
// 2009/07/16 yashiro Emperorプリント1200dpi対応.
// ================================================================================================
uint32_t THRC_ThreadSeqCom::getOutputImgInfoWidth( CC_OneExpandMode* pc_OneExpandMode, CC_OneImgTransInfo* pcc_OneImgTransInfo, ushort us_alignWidth )
{
	//! 解像度が1200dpiの場合、600dpi相当に変換する(1200dpiでなければ、変換しない).
	register uint32_t ul_Width = _roundup( MMtoDOT( pc_OneExpandMode->ct_OutputSize.x, theModeInfoLib.ReCalcResolution_DPI( pc_OneExpandMode->c_Resolution.getDpi().x ) ), us_alignWidth ) * us_alignWidth;
	return ul_Width;
}

// ================================================================================================
//! <!--機能-->  シーケンス層の基本クラス 副主走査サイズ伸張情報から取得 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param CC_OneExpandMode* pc_OneExpandMode .......................
//! \param CC_OneImgTransInfo* pcc_OneImgTransInfo ..................
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2005/07/13 橋本昌	新規作成 
// 2009/07/16 yashiro Emperorプリント1200dpi対応.
// ================================================================================================
uint32_t THRC_ThreadSeqCom::getOutputImgInfoHeight( CC_OneExpandMode* pc_OneExpandMode, CC_OneImgTransInfo* pcc_OneImgTransInfo, short ss_BandID )
{
	//! 解像度が1200dpiの場合、600dpi相当に変換する(1200dpiでなければ、変換しない).
	register uint32_t ul_Height = MMtoDOT( pc_OneExpandMode->ct_OutputSize.y, theModeInfoLib.ReCalcResolution_DPI( pc_OneExpandMode->c_Resolution.getDpi().y ) );

	if( pcc_OneImgTransInfo->bl_PageTrans == False ) {		
		// バンド単位の転送の時の計算を行う
		if( ss_BandID + pcc_OneImgTransInfo->uc_UnitBandTrans > pcc_OneImgTransInfo->uc_TransBandNum ) {
			// 最終バンドの転送の時
			ul_Height = ul_Height % ( pcc_OneImgTransInfo->uc_UnitBandTrans * DEF_BasicTransLine );
			if( ul_Height == 0 ){
				ul_Height = pcc_OneImgTransInfo->uc_UnitBandTrans * DEF_BasicTransLine;
			}
		} else {
			ul_Height = DEF_BasicTransLine * pcc_OneImgTransInfo->uc_UnitBandTrans;
		}
	}
	
	return ul_Height;
}

// ================================================================================================
//! <!--機能--> シーケンス層の基本クラス 画像変換を行わないスレッド転送の転送画像情報の設定 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRS_BufImgInfo& rs_BufImgInfo .......................
//! \param ST_ThreadComPara& rs_ThreadComPara ...................
//! \param THRE_ThrID e_ThrID ...................................
//! \param short ss_BandID ......................................
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2005/07/13 橋本昌	新規作成 
// 2007/09/11 takamoto	白紙プレビュー時のカラー/階調情報取得対応 
// ================================================================================================
void THRC_ThreadSeqCom::setBufTransImgInfoOut( THRS_BufImgInfo& rs_BufImgInfo, ST_ThreadComPara& rs_ThreadComPara, THRE_ThrID e_ThrID, short ss_BandID )
{
	// データ変換なしのスレッド転送のとき 
	
	CC_OneImgTransInfo* pcc_OneImgTransInfo = CC_ImgTransInfoSingletonIf::newInstance().getOneImgTransInfoFromTh( e_ThrID, rs_ThreadComPara );
	THRL_ThAssert( ( pcc_OneImgTransInfo != NULL ), "THRC_ThreadSeqCom::setBufTransImgInfoOut(): getOneImgTransInfoFromTh NULL error." , 0, 0, 0, 0, CFA14_DetailCode0x0597 );
	CC_OneExpandMode* pc_LocalOneExpandMode = CC_ImgTransInfoSingletonIf::newInstance().getLocalOneExpandMode( e_ThrID, rs_ThreadComPara );
	
	// 画像情報の設定 
	rs_BufImgInfo.uc_MaxColorNum = pcc_OneImgTransInfo->uc_AllColorNum;	// カラー数 
	rs_BufImgInfo.uc_MaxAttBitNum = pcc_OneImgTransInfo->uc_AllAttNum;		// 全属性プレーン数 
	rs_BufImgInfo.uc_AllPlaneNum = pcc_OneImgTransInfo->uc_AllPlaneNum;	// 全プレーン数 
	
	rs_BufImgInfo.e_ImageColor = theColorInformation.getNin1ImageColor( rs_ThreadComPara, e_ThrID, pc_LocalOneExpandMode );	// 画像カラー情報(Lab,YMCK,RGB...)
	rs_BufImgInfo.e_ImageTone = theColorInformation.getNin1ImageTone( rs_ThreadComPara, e_ThrID, pc_LocalOneExpandMode );		// 画像階調情報(1bit-4bit,8bit)
	rs_BufImgInfo.e_ImageCompress = theColorInformation.getNin1ImageCompress( pc_LocalOneExpandMode );	// 画像圧縮情報(BTC333,BTC+JBIG...)
	rs_BufImgInfo.us_alignWidth = pcc_OneImgTransInfo->us_BitWidthUnit;										// 画像横幅設定単位(dot単位) 
	
	// 転送元の画像横幅 
	rs_BufImgInfo.ul_SrcImageWidth = getOutputImgInfoWidth( pc_LocalOneExpandMode, pcc_OneImgTransInfo, pcc_OneImgTransInfo->us_BitWidthUnit );
	
	// 転送元の画像ライン数	
	rs_BufImgInfo.ul_SrcImageHeight = getOutputImgInfoHeight( pc_LocalOneExpandMode, pcc_OneImgTransInfo, ss_BandID );
	
	// 転送先の画像横幅 
	rs_BufImgInfo.ul_DstImageWidth = rs_BufImgInfo.ul_SrcImageWidth;
	// 転送先の画像ライン数
	rs_BufImgInfo.ul_DstImageHeight = rs_BufImgInfo.ul_SrcImageHeight;
}

// ================================================================================================
//! <!--機能-->　シーケンス層の基本クラス 画像変換を行うスレッド転送の転送画像情報の設定 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param THRS_BufImgInfo& rs_BufImgInfo ......................
//! \param ST_ThreadComPara& rs_ThreadComPara ..................
//! \param THRE_ThrID e_ThrID ..................................
//! \param short ss_BandID .....................................
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2005/07/14 橋本昌	新規作成 
// 2007/09/11 takamoto	白紙プレビュー時のカラー/階調情報取得対応 
// ================================================================================================
void THRC_ThreadSeqCom::setBufTransImgInfoOutForImageConvert(  THRS_BufImgInfo& rs_BufImgInfo, ST_ThreadComPara& rs_ThreadComPara, THRE_ThrID e_ThrID, short ss_BandID )
{
	// データ変換(解像度変換、変倍、色変換、２値多値変換、画像合成)ありのスレッド転送のとき 
	
	CC_OneImgTransInfo* pcc_OneImgTransInfo = CC_ImgTransInfoSingletonIf::newInstance().getOneImgTransInfoFromTh( e_ThrID, rs_ThreadComPara );
	THRL_ThAssert( ( pcc_OneImgTransInfo != NULL ), "THRC_ThreadSeqCom::setBufTransImgInfoOutForImageConvert(): getOneImgTransInfoFromTh NULL error." , 0, 0, 0, 0, CFA14_DetailCode0x0598 );
	
	// 画像情報の設定 
	rs_BufImgInfo.uc_MaxColorNum = pcc_OneImgTransInfo->uc_AllColorNum;	// カラー数 
	rs_BufImgInfo.uc_MaxAttBitNum = pcc_OneImgTransInfo->uc_AllAttNum;		// 全属性プレーン数 
	rs_BufImgInfo.uc_AllPlaneNum = pcc_OneImgTransInfo->uc_AllPlaneNum;	// 全プレーン数 
	
//	// 色情報の取得 
//	DOCC_Leaf* pcc_DstOneInputPage = CC_ImgTransInfoSingletonIf::newInstance().getLocalLeaf( e_ThrID, rs_ThreadComPara );	// データ変換後のLeaf情報を取得 
//	rs_BufImgInfo.e_ImageColor = pcc_DstOneInputPage->getImage()->c_ImageInfo.ec_ImageColorResult;			// 画像カラー情報(Lab,YMCK,RGB...)
//	rs_BufImgInfo.e_ImageTone = pcc_DstOneInputPage->getImage()->c_ImageInfo.ec_ImageTone;					// 画像階調情報(1bit-4bit,8bit)
//	rs_BufImgInfo.e_ImageCompress = pcc_DstOneInputPage->getImage()->c_ImageInfo.ec_ImageCompressResult;		// 画像圧縮情報(BTC333,BTC+JBIG...)
	
	register ushort us_alignWidth = 1;	// 解像度変換前後のデータ横幅は 1dot 単位 
//	rs_BufImgInfo.us_alignWidth = pcc_OneImgTransInfo->us_BitWidthUnit;										// 画像横幅設定単位(dot単位) 
	
	///////////// データ変換前 ////////////
	CC_OneExpandMode* pc_SrcLocalOneExpandMode = CC_ImgTransInfoSingletonIf::newInstance().getLocalOneExpandMode( e_ThrID, rs_ThreadComPara );
	// 色情報の取得 
	rs_BufImgInfo.e_ImageColor = theColorInformation.getNin1ImageColor( rs_ThreadComPara, e_ThrID, pc_SrcLocalOneExpandMode );	// 画像カラー情報(Lab,YMCK,RGB...)
	rs_BufImgInfo.e_ImageTone = theColorInformation.getNin1ImageTone( rs_ThreadComPara, e_ThrID, pc_SrcLocalOneExpandMode );	// 画像階調情報(1bit-4bit,8bit)
	rs_BufImgInfo.e_ImageCompress = theColorInformation.getNin1ImageCompress( pc_SrcLocalOneExpandMode );	// 画像圧縮情報(BTC333,BTC+JBIG...)

	// 転送元の画像横幅 
	rs_BufImgInfo.ul_SrcImageWidth = getOutputImgInfoWidth( pc_SrcLocalOneExpandMode, pcc_OneImgTransInfo, us_alignWidth );
	
	// 転送元の画像ライン数	
	rs_BufImgInfo.ul_SrcImageHeight = getOutputImgInfoHeight( pc_SrcLocalOneExpandMode, pcc_OneImgTransInfo, ss_BandID );
	
	///////////// データ変換後 ////////////
	DOCC_Leaf* pcc_DstOneInputPage = CC_ImgTransInfoSingletonIf::newInstance().getLocalLeaf( e_ThrID, rs_ThreadComPara );	// データ変換後のLeaf情報を取得 
	// 転送先の画像横幅 
	THRL_ThAssert( ( pcc_DstOneInputPage->getImage() != NULL ), "THRC_ThreadSeqCom::setBufTransImgInfoOutForImageConvert(): getImage NULL error." , 0, 0, 0, 0, CFA14_DetailCode0x0599 );
	rs_BufImgInfo.ul_DstImageWidth = _roundup( (uint32_t)pcc_DstOneInputPage->getImage()->c_ImageInfo.ct_TransSize.x, (uint32_t)us_alignWidth ) * us_alignWidth;	
	// 転送先の画像ライン数
	rs_BufImgInfo.ul_DstImageHeight = getBufTransImgInfoInOutHeight( pcc_DstOneInputPage, NULL, pcc_OneImgTransInfo, ss_BandID, True );
}

// ================================================================================================
//! <!--機能--> シーケンス層の基本クラス 画像変換を行う転送か否か判断 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param ST_ThreadComPara ST_ThreadComPara ...
//! \param rs_ThreadComPara ...................
//! \param THRE_ThrID e_ThrID ..................
//! <!--戻値-->
//! \return なし
//! <!--詳細-->
//! \n
//! <!--履歴-->
// 2005/07/13 橋本昌	新規作成.
// 2013/03/13 Sky		ZeusS対応 未使用の画像処理モードを削除.
// ================================================================================================
Bool THRC_ThreadSeqCom::isImageConvert( ST_ThreadComPara& rs_ThreadComPara, THRE_ThrID e_ThrID )
{
	Bool bl_ImageConvert = False;	// 画像絵変換なしの転送 
	
	// バッファメモリ→バッファメモリ
	if( e_ThrID == THRE_ThrID_BufToBuf ){
		if( rs_ThreadComPara.en_ThreadImgProcMode == THRE_ImgProcMode_ChgResolution 	// 解像度変換
			|| rs_ThreadComPara.en_ThreadImgProcMode == THRE_ImgProcMode_ChgColor
			|| rs_ThreadComPara.en_ThreadImgProcMode == THRE_ImgProcMode_Tatika
			|| rs_ThreadComPara.en_ThreadImgProcMode == THRE_ImgProcMode_ImgConv ){
			bl_ImageConvert = True;	// 画像絵変換あり 
		}
	}
	
	return bl_ImageConvert;
}

// ================================================================================================
//! <!--機能--> シーケンス層の基本クラス 画像合成する際の合成用バッファ上でのプレーン番号取得 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param ST_ThreadComPara* ps_ThreadComPara ....................
//! \param THRE_CompositeMode e_CompositeMode ....................
//! <!--戻値-->
//! \return 各合成において使用するプレーンの番号を返す。
//! <!--詳細-->
//! \n	エンジン出力時の各合成での使用するプレーン番号を返す関数。
//! \n	Moselでは1bit(1プレーン)だが、Donauでは３bit(3プレーン分)あるので、 
//! \n	その対応の為に新規作成する。
//! <!--履歴-->
// 2007/05/08 tadamasa 新規作成.
// 2013/03/19 Sky ZeusS対応 オーバーレイ属性3bit→2bit対応.
// ================================================================================================
//tada3bit
uchar THRC_ThreadSeqCom::getAttPlaneNoForImageCompositeEng( THRE_CompositeMode e_CompositeMode )
{
	register uchar uc_PlaneNo = 0;
	switch( e_CompositeMode ){
		case THRE_CompositeMode_HeaderFooter:	// 文字合成(ヘッダ・フッダ)
		case THRE_CompositeMode_SampleCopy:		// 文字合成(サンプルコピー)
		case THRE_CompositeMode_FilingNo:		// 文字合成(ファイリングナンバー)
			uc_PlaneNo = 0;
			break;
		case THRE_CompositeMode_RTI:			// 文字合成(RTI)
		// 2008/04 sugimoto CITで見つかった問題対策. 
		case THRE_CompositeMode_SingleWaterMark:	// コピーウォーターマーク.
		case THRE_CompositeMode_OverLay:		// オーバーレイ.
			uc_PlaneNo = 1;
			break;

		default:
			THRL_ThAssert( False, "THRC_ThreadSeqCom::getAttPlaneNoForImageComposite CompositMode Error! :%d ", e_CompositeMode , 0, 0, 0, CFA14_DetailCode0x059A );
			break;
	}

#ifdef COSIM // COSIM 2nd.検証70002(合成　3bit合成パターン) 特殊環境用 対応.
	extern uint32_t getCosimTestNo( void );
	uint32_t ul_CosimTestNo = getCosimTestNo();

	if( ul_CosimTestNo == 70002 ) {
	//COSIM検証70002時は、プレーンNoを固定で設定する.
		switch( e_CompositeMode ){
			case THRE_CompositeMode_HeaderFooter:	// 文字合成(ヘッダ・フッダ)
				uc_PlaneNo = 0;
				break;
			case THRE_CompositeMode_SampleCopy:		// 文字合成(サンプルコピー)
				uc_PlaneNo = 1;
				break;
			case THRE_CompositeMode_FilingNo:		// 文字合成(ファイリングナンバー)
				uc_PlaneNo = 2;
				break;
			case THRE_CompositeMode_RTI:			// 文字合成(RTI)
				uc_PlaneNo = 1;
				break;
			case THRE_CompositeMode_OverLay:		// オーバーレイ   
				uc_PlaneNo = 2;
				break;

			default:
				THRL_ThAssert( False, "THRC_ThreadSeqCom::getAttPlaneNoForImageComposite CompositMode Error! :%d ", e_CompositeMode , 0, 0, 0, CFA14_DetailCode0x059B );
				break;
		}
	}
#endif		
	return uc_PlaneNo;
}

// ================================================================================================.
//! <!--機能--> 合成用属性データ転送元アドレス設定 PCI側.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param THRS_BufAdrInfo& rs_SrcBufInfo  ... 合成用属性データ転送元アドレス.
//! \param CC_OneImgTransInfo* pcc_OneImgTransInfo  ... 画像転送管理情報.
//! \param ulong ul_ImageWidth  ... 画像横幅.
//! \param ulong ul_ImageHeight  ... 画像高さ.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//! 13/03/18 Sky ZeusS対応 ImageConvSeqから移動 (MultiTransDataSeqでも使用するため).
// ================================================================================================.
void THRC_ThreadSeqCom::setPCIMainBufAdrInfo( THRS_BufAdrInfo& rs_SrcBufInfo, CC_OneImgTransInfo* pcc_OneImgTransInfo,
                                                          uint32_t ul_ImageWidth, uint32_t ul_ImageHeight )
{
	// アドレスはバッファ管理とInputMemoryから取得.
	short ss_BufIdx = pcc_OneImgTransInfo->getUsedBufIndex( THRE_BufId_Main, False );
	THRL_ThAssert( (ss_BufIdx >= 0), "setPCIBufAdrInfo Buf error!" , 0, 0, 0, 0, CFA14_DetailCode0x0C33 );
	THRC_BufferResource* pc_BufResorce = pcc_OneImgTransInfo->s_usedBuf[ ss_BufIdx ].pc_BufResMan;;
	register uintptr_t ul_ArrayStartAdr = pc_BufResorce->getAddr();
	register uint32_t ul_BandSize = ul_ImageWidth/8 * DEF_BasicTransLine;
	register uint32_t ul_MaxBandNum = _roundup( ul_ImageHeight, DEF_BasicTransLine );

	for( register uchar ucBandNo = 0; ucBandNo < ul_MaxBandNum; ucBandNo++ ){
		rs_SrcBufInfo.s_BufAddress[ ucBandNo ].s_Color[ 0 ].ul_PlaneAdd[ 0 ] = ul_ArrayStartAdr + ucBandNo * ul_BandSize;
	}

	rs_SrcBufInfo.uc_BufBandNum[ 0 ] = (uchar)ul_MaxBandNum;
	rs_SrcBufInfo.uc_BufPlaneNum[ 0 ] = 1;
	for( register uchar ucColorNo = 1; ucColorNo<THRD_MAXBUF_COLORNUM; ucColorNo++ ){
		// 念のため未使用のデータをクリア.
		rs_SrcBufInfo.uc_BufBandNum[ ucColorNo ] = 0;
		rs_SrcBufInfo.uc_BufPlaneNum[ ucColorNo ] = 0;
	}

	rs_SrcBufInfo.b_RingBufferMode = False;
}

// ================================================================================================
//! <!--機能--> 分割転送モードの設定 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param uchar uc_SeqId ... 
//! <!--戻値-->
//! \return 
//! <!--詳細-->
//! \n
//! <!--履歴-->
//  13/03/25 sky  新規作成
// ================================================================================================
void THRC_ThreadSeqCom::setDivTransMode( THRE_DivTransMode e_DivTransModePara )
{
	e_DivTransMode = e_DivTransModePara;
}

// ================================================================================================
//! <!--機能--> 分割転送モードの取得 
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param なし
//! <!--戻値-->
//! \return 
//! <!--詳細-->
//! \n
//! <!--履歴-->
//  13/03/25 sky  新規作成
// ================================================================================================
THRE_DivTransMode THRC_ThreadSeqCom::getDivTransMode()
{
	return ( e_DivTransMode );
}
