//クラス-----------------------------------------------------------------------------------------------------------------.
// ファイル : CHOP_A056300100-1_機密文書ID／パスワード入力.
// Version  : PD ver.019. 
//-----------------------------------------------------------------------------------------------------------------------.
#define SIZE( table ) ( sizeof( table ) / sizeof( PDisplayParts* ) )
#include <panel/plibh/plibh_CLCD2/PNLC_TabletConnectRestrictLib.h>
#include <panel/plibh/plibh_CLCD2/PNLC_BoxDocInfoControlLib.h>
#include <panel/ppcg/ppcg_PD/ppcg_PD.h>
// Comment 2017/09/04 [HCL-TANU] [IT5 4.2][RQ-686] [WP755]Multiple Network support.
#include <panel/plibh/plibh_CLCD2/PNLC_NetworkLan.h>
#include <panel/pcgbox/pcgbox_CLCD2/pigbox_base.h>				// PNLRFN_V2_ITEM_0001.
#define gp_A056300200_1 gp_DocSecretDataBeforePassPrint_box		// PNLRFN_V2_ITEM_0001.
#define gp_A056300200 gp_DocSecretDataBeforePassPrint_box		// PNLRFN_V2_ITEM_0001.
#include "pcgbox_A054100010.h"
extern PA054100010 gp_A054100010;
#include "pcgbox_A056300100.h"
// Comment 2019/10/09 【HCLT-Krishna】【IT6_HeliosMLK】【制御構成変更_No.43】 SSDレス時の機密文書プリント対応
//// Comment 2018/11/19 【HCLT-Prashannaa】【IT6_DenebMLK】【制御構成変更_No.83】 
//#ifndef DEF_DenebMLKCus
#if !defined  (DEF_DenebMLKCus) && !defined ( DEF_HeliosMLKCus )
#ifdef DEF_LCD_7INCH
#include "PD/A056300100_1_7inch.cpp"
#else
#include "PD/A056300100_1.cpp"
#endif // DEF_LCD_7INCH.
#else
#include "PD/A056300100_1_DeM.cpp"
#endif
// Comment 2019/10/09 【HCLT-Krishna】【IT6_HeliosMLK】【制御構成変更_No.43】 SSDレス時の機密文書プリント対応
//// Comment 2018/11/19 【HCLT-Vandana】【IT6_DenebMLK】【制御構成変更_No.83】
//#ifndef DEF_DenebMLKCus
#if defined  (DEF_DenebMLKCus) || defined ( DEF_HeliosMLKCus )
#include <panel/pcgact/pcgact_CLCD2/pigact_pA072002.h> 
#endif
#define PNLD_SECRETPASSWORD_MAX 64
static PDisplayParts* const tbl_Mode1[] = { &Bak_BackKeyboardLine, &Bak_2ndLevelBackKeyboardLine, &Grp_KeybordInputChange };
static PDisplayParts* const tbl_Mode2[] = { &A056300100_1_Txt0001, &Bak_BackKeyboard, &Bak_2ndLevelBackKeyboard };
//PanelDesigner対応------------------------------------------------------------------------------------------------------.
// 画面開始処理.
void PA056300100_1::startGamen()
{
	if( PnlComGetSystemSetting()->getSecurePrintCertification() == TYPE_SecurePrintCertification1 ) {	// 機密文書パスワード画面へ.
		PNLC_TabletSecurityControlLib::newInstance()->securityAreaEntered();							// 認証操作禁止モードが"モード1"の場合は、セキュリティエリアとみなす.
// comment 2022/12/13 Sky東島 [IT6_2.3][OP_BTS-39836].
		// ボックスパスワード入力中状態を設定.
		onlyPanel.setEnteringBoxPassword( PNLE_EnteringBoxPassword_Typing );
		memset( A056300100_1_Gra0012.sc_Input,'\0', sizeof( A056300100_1_Gra0012.sc_Input ) );
		memset( sc_MmbTextPassword,'\0', sizeof( sc_MmbTextPassword ) );
		A056300100_1_Gra0012.setup( PNLD_KBTC_5Country, PNLD_BOXKEYBOARD_MAX16CHAR, KC_DisableKeyPatternNon, PCGD_TrackPasswordUTF8 );
		A056300100_1_Gra0012.setKeybordLayoutType( PNLD_KeybordLayout_Type2 );
		A056300100_1_Gra0012.setTxGy( 8, 12 );
		PNLS_InputNumKeyboard s_inp = { PNLD_KBTC_5Country, s_pKEYDocID_W136H028A, s_pKEYPassword_W136H028A, sc_MmbTextPassword, PNLD_SECRETPASSWORD_MAX, True };
		A056300100_1_Gra0012.setInputNum( PNLE_INPUTNUMKEYBOARD_TWO, s_inp );
		setOffset( 0, -8 );
	} else {
		memset( A056300100_1_Gra0012.sc_Input,'\0',sizeof( A056300100_1_Gra0012.sc_Input ) );
		A056300100_1_Gra0012.setup( PNLD_KBTC_5Country, PNLD_BOXKEYBOARD_MAX16CHAR, KC_DisableKeyPatternNon, PCGD_TrackPasswordUTF8 );
		A056300100_1_Gra0012.setKeybordLayoutType( PNLD_KeybordLayout_Type1 );
		A056300100_1_Gra0012.setTxGy( 8, 16 );
	}
	bl_PanelDesigner = True;				// PanelDesigner対応.
	PPopupAnimationCom::startGamen();		// 親クラス呼出し(これを行わないとPopupWindowが表示しない).
// コールバック関数の登録.
	ATTACH_OBSERVER( PA056300100_1, gp_A056300100_1, PA056300100_1::prOk,     A056300100_1_Btn0008, Bool, Bool ); // OK.
	ATTACH_OBSERVER( PA056300100_1, gp_A056300100_1, PA056300100_1::prCancel, A056300100_1_Btn0006, Bool, Bool ); // キャンセル.
	setOkButtonForFunctionBadge( &A056300100_1_Btn0008 ); // 機能バッチボタン押下時に代替え処理を行うＯＫキーを登録.
// 変数等の初期化処理を記載する.
	b_mSecurePrintMode = ( PnlComGetSystemSetting()->getSecurePrintCertification() == TYPE_SecurePrintCertification1 ) ? True : False;
// ここに構成による表示条件を記載する.
	if( b_mSecurePrintMode ) {	// 機密文書パスワード画面へ.
		A056300100_1_Gra0000.changeSize( 784, 440 );
		A056300100_1_Btn0006.shiftPosition( 0, 16 );
		A056300100_1_Btn0008.shiftPosition( 0, 16 );
		A056300100_1_Btn0010.shiftPosition( 0, 16 );
		for( uchar i=0; i< sizeof(tbl_Mode2)/sizeof(PDisplayParts*); i++ ) { tbl_Mode2[i]->showEnable( False ); }
		for(       i=0; i< sizeof(tbl_Mode1)/sizeof(PDisplayParts*); i++ ) { tbl_Mode1[i]->showEnable( True  ); }
	} else {
		for( uchar i=0; i< sizeof(tbl_Mode1)/sizeof(PDisplayParts*); i++ ) { tbl_Mode1[i]->showEnable( False ); }
		for(       i=0; i< sizeof(tbl_Mode2)/sizeof(PDisplayParts*); i++ ) { tbl_Mode2[i]->showEnable( True  ); }
	}
// ここに画面の初期化処理を記載する.
	A056300100_1_Btn0010.showEnable( False );											// ドキュメント削除ボタン非表示.
	if( ( ( PnlComGetUser() != NULL ) && ( PnlComGetUser()->getUserType() == TYPE_UserTypeBoxAdmin ) ) || ( ( PnlComGetTrack() != NULL ) && ( PnlComGetTrack()->getTrackType() == TYPE_TrackTypeBoxAdmin ) ) ) {
		A056300100_1_Btn0010.showEnable( True );
		int32_t i32_DocNum = PJobListControl::GetDocListSelectNum();
// Commment 2017/09/22 HCL_Venkataramesh [IT5_4.2][RQ-686][WP689] setBoxId support for Network2 user.
//		cp_DocList->setBoxId( DOCD_SecretDocumentsBox );								// アクセスボックスIDを設定.
		PNLC_NetworkLan::getInstance()->setBoxIdPNL( cp_DocList, DOCD_SecretDocumentsBox );
		cp_DocList->getMaxDocCnt( NULL, s_DocSelectBoxInfo.sc_UserPass, &i32_DocNum );	// 機密文書ボックス内のドキュメント最大数を取得.
		A056300100_1_Btn0010.activate( ( i32_DocNum != 0 ) ? True : False );
	}
	setupGamenAtButtonOn();	// ボタンＯＮ時の表示部品のセットアップ.
}
// ボタンＯＮ時の表示部品のセットアップ.
void PA056300100_1::setupGamenAtButtonOn()
{
	A056300100_1_Gra0012.setInsertCharLenCheck( TYPE_InsertCharLenStrUTF8 );			// 文字数でのカウント指定.
	A056300100_1_Gra0012.setProhibitionBitType( KC_DisableKeyPatternNon );
	A056300100_1_Btn0008.activate( ( static_cast<ushort>( strlen( A056300100_1_Gra0012.sc_Input ) ) > 0 ) ? True : False );
}
// OK.
Bool PA056300100_1::prOk( Bool )
{
	Bool b_iswarning = False;
	memset ( s_SecretDocInfo.sc_secretDocId, '\0', sizeof( s_SecretDocInfo.sc_secretDocId ) );
	strncpy( s_SecretDocInfo.sc_secretDocId, A056300100_1_Gra0012.sc_Input, sizeof( s_SecretDocInfo.sc_secretDocId )-1 );
	s_SecretDocInfo.sc_secretDocId[ sizeof( s_SecretDocInfo.sc_secretDocId )-1 ] = '\0';
	PNLC_BoxDataLib::newInstance()->setSecretDocInfo( s_SecretDocInfo );
	int32_t i32_docCnt = 0;
	if( True == APIC_SoftDipSw::newInstance()->getSoftDipSwBitForPanelSpecNo( 53, 2 ) ) {			// DipSW 53 bit2.
// Commment 2017/09/22 HCL_Venkataramesh [IT5_4.2][RQ-686][WP670] support for Network2 user.
//		cp_DocList->getMaxDocCntSecret( APIE_SecretDoc, PNLC_BoxDataLib::newInstance()->getSecretDocInfo().sc_secretDocId, APID_DocapiNull, &i32_docCnt );
		TYPE_NetworkSpecific e_NetworkSpecific = PNLC_NetworkLan::getInstance()->getLine();
		cp_DocList->getMaxDocCntSecret( APIE_SecretDoc, PNLC_BoxDataLib::newInstance()->getSecretDocInfo().sc_secretDocId, APID_DocapiNull, &i32_docCnt , e_NetworkSpecific );
		if( 0 == i32_docCnt ) { onlyPanel.setPanelWarning( PNL_PERROR_SecurityBoxDocNon ); return False; }
	}
	if( b_mSecurePrintMode ) {
		PNLC_BoxDocInfoControlLib* pc_BoxDocInfoControlLib = PNLC_BoxDocInfoControlLib::newInstance();
		if( NULL == PnlComGetUser() ) { return False; }
		uint32_t ui32_OpeCode   = JLGE_InspectLogOperateIDL_Non;   // 操作者コード.
		uint32_t ui32_TrackCode = JLGE_InspectLogTrackIDL_Non;     // 部門コード.
		JLGS_OpeLogCom s_OpeLogCom;
		JLGS_OpeLogBox s_OpeLogBox;
		memset( &s_OpeLogCom, 0x00, sizeof( s_OpeLogCom ) );
		memset( &s_OpeLogBox, 0x00, sizeof( s_OpeLogBox ) );
		s_OpeLogBox.sl_MoveBoxNo = INT_MAX;
		s_OpeLogBox.sl_CopyBoxNo = INT_MAX;
		if( APIC_SystemSetting::newInstance()->getJobLogInspectLogOn( ) && APIC_SystemSetting::newInstance()->getLogOutputOn( ) ) {	// 監査ログ設定可能チェック.
			if( PnlComGetUser()->getUserType() != TYPE_UserTypeAdmin && PnlComGetUser()->getUserType() != TYPE_UserTypeBoxAdmin && PnlComGetUser()->getUserType() != TYPE_UserTypePublicUser ) {	//ユーザータイプは【管理者、Ｂｏｘ管理者、ＰＵＢＬＩＣユーザー】じゃない場合.
				ui32_OpeCode = JLGE_InspectLogOperateIDL_User + PnlComGetUser()->getUserNo();											// 通常ユーザー、部門の場合.
				if( NULL != PnlComGetTrack() ) { ui32_TrackCode = JLGE_InspectLogTrackIDL_Track + PnlComGetTrack()->getTrackNo(); }	// 部門コード.
			} else if( TYPE_UserTypePublicUser == PnlComGetUser()->getUserType() ) {					// ユーザータイプはＰＵＢＬＩＣユーザーの場合.
				if( ( PnlComGetUser()->getUserType() == TYPE_UserTypePublicUser ) && ( ( PnlComGetUserAuthentication()->getUserAuthentication( PNLC_NetworkLan::getInstance() ->getLine( ) ) == TYPE_UserAuthenticationOff ) 
					&& ( TYPE_TrackModeOff != PnlComGetTrackManager()->getTrackMode( PNLC_NetworkLan::getInstance()->getLine() ) ) ) ) {	//部門はＰＵＢＬＩＣユーザーの場合.
					if( NULL != PnlComGetTrack() ) {
						ui32_OpeCode = JLGE_InspectLogOperateIDL_PublicUser;								// パブリックユーザ.
						ui32_TrackCode = JLGE_InspectLogTrackIDL_Track + PnlComGetTrack()->getTrackNo();	// 部門コード.
					}
				} else {
					ui32_OpeCode = JLGE_InspectLogOperateIDL_PublicUser;									// パブリックユーザ.
				}
			} else if( TYPE_UserTypeAdmin == PnlComGetUser()->getUserType() ) {		// ユーザータイプは管理者の場合.
				ui32_OpeCode = JLGE_InspectLogOperateIDL_Admin;						// 対象は管理者.
				ui32_TrackCode = JLGE_InspectLogTrackIDL_Admin;						// 部門コード.
			} else {
				ui32_OpeCode = JLGE_InspectLogOperateIDL_BoxAdmin;					// 対象はボックス管理者.
				ui32_TrackCode = JLGE_InspectLogTrackIDL_BoxAdmin;					// 部門コード.
			}
			char sc_ExServerName[PCGD_DomainNameUTF8];								// 外部サーバ名の設定.
			PnlComGetUser()->getLoginServerName( sc_ExServerName );
			char  sc_UserName[PCGD_UserNameUTF8_193];								// ユーザ名の設定.
			PnlComGetUser()->getUserNameUTF8( sc_UserName );
			char  sc_TrackName[PCGD_TrackDispNameUTF8_61] = { '\0' };				// 部門名の設定.
			if( NULL != PnlComGetTrack() ) {
				switch( PnlComGetTrackManager()->getTrackMode( PNLC_NetworkLan::getInstance()->getLine() ) ) {					// 部門管理モード取得 .
					case TYPE_TrackModeOnWithUserAuthentication:	PnlComGetTrack()->getTrackName( sc_TrackName );			break;
					case TYPE_TrackModeNoAndPassword:				PnlComGetTrack()->getTrackName( sc_TrackName );			break;	// 部門名と部門パスワードを入力する. 部門名取得.
					case TYPE_TrackModeOnlyPassword:				PnlComGetTrack()->getTrackDispNameUTF8( sc_TrackName );	break;	// 部門パスワードのみを入力する. 部門の表示名(名称)取得.
				}
			}
			s_OpeLogCom.ul_Code			= JLGE_InspectLogSecretDocAuthenticationPass;     		// 操作コード.
			APIC_JLG::newInstance()->getNowTime( &s_OpeLogCom.s_Time );     					// 操作日時.
			s_OpeLogCom.us_Result		= JLGE_InspectLogResultOK;     							// 結果コード.
			s_OpeLogCom.ul_OpeCode		= ui32_OpeCode;     										// 操作者コード.
			strncpy( s_OpeLogCom.sc_Name, sc_UserName, sizeof( s_OpeLogCom.sc_Name) -1 );     				// 操作者名.
			s_OpeLogCom.ul_TrackCode	= ui32_TrackCode;     									// 操作者コード.
			strncpy( s_OpeLogCom.sc_TrackName,     sc_TrackName,    sizeof( s_OpeLogCom.sc_TrackName )-1 );	// 部門名.
			strncpy( s_OpeLogCom.sc_ExtServerName, sc_ExServerName, sizeof( s_OpeLogCom.sc_ExtServerName )-1 );	// 外部サーバ名.
			s_OpeLogCom.uc_IF			= JLGE_InspectLogIfOperationPanel;						// I/F番号.
			strncpy( s_OpeLogBox.sc_SecretID, PNLC_BoxDataLib::newInstance()->getSecretDocInfo().sc_secretDocId, sizeof( s_OpeLogBox.sc_SecretID )-1 );		//ジョブログ 操作系の機密文書IDにセット。.
			s_OpeLogBox.sl_TargetBoxNo	= s_DocSelectBoxInfo.sl_BoxId;							// BoxID.
		}
		BOXS_SecretDocumentInfo	s_TempDocInfo = PNLC_BoxDataLib::newInstance()->getSecretDocInfo();
		memset( &s_TempDocInfo.sc_secretPasswd, '\0', sizeof( s_TempDocInfo.sc_secretPasswd ) );
		strncpy( s_TempDocInfo.sc_secretPasswd, sc_MmbTextPassword, sizeof( s_TempDocInfo.sc_secretPasswd )-1 );
		PNLC_BoxDataLib::newInstance()->setSecretDocInfo( s_TempDocInfo );
		if( pc_BoxDocInfoControlLib->getSecretBoxMaxdocCnt( PNLC_BoxDataLib::newInstance()->getSecretDocInfo().sc_secretDocId , sc_MmbTextPassword ) == 0 ) {		//機密文書ＩＤとパスワードが一致するボックス内にドキュメントが存在しない場合は、ブランクにする .
			memset( sc_MmbTextPassword, 0, sizeof( sc_MmbTextPassword ) );			// 文字列の初期化.
			A056300100_1_Gra0012.procClearLine();
			A056300100_1_Gra0012.procKeyBoard( BTNID_KEY_INPUTCHANGELINEONE );
			onlyPanel.setPanelWarning( PNL_ERROR_SecurityDocumentCertificate );		// ５秒間の警告.
			b_iswarning = True;
			s_OpeLogCom.us_Result = JLGE_InspectLogResultAuthenticationNG;			// 結果コードに認証失敗を設定.
// comment 2017/10/06 【HCL-RamKishor】【IT5_4.2】【RQ686】【WP900】 Supporting API for Network 1.
			s_OpeLogCom.uc_NetworkTypeOperationInfo = PNLC_NetworkLan::getInstance()->getLine();
// comment 2018/03/21 【HCL-Gadiya Sai】【IT5_4.2】【AR1042110】 For USer with admin rights.
//			s_OpeLogCom.uc_NetworkTypeOperatorInfo= PNLC_NetworkLan::getInstance()->getLine();
			s_OpeLogCom.uc_NetworkTypeOperatorInfo = TYPE_NetworkSpecific_NW1;										// Setting default as NW1.
			APIC_User* pc_User = PnlComGetUser();
			if( ( NULL != pc_User ) && ( pc_User->getSpecialAuthority( TYPE_AuthorityMode_Admin ) ) ) {
				short ss_AuthorityID = PnlComGetAuthorityID();
				if( TYPD_NotSetting != ss_AuthorityID ) {
					TYPS_SpecialAuthorityLoginInfo s_SpecialAuthorityLoginInfo;
					if( pc_User->getSpecialAuthorityLoginInfo( ss_AuthorityID,&s_SpecialAuthorityLoginInfo) ) {		// Checking whether user is having Admin rights or not.
						s_OpeLogCom.uc_NetworkTypeOperatorInfo = s_SpecialAuthorityLoginInfo.e_NWType;				// If login user having admin rights setting the login network type to joblog.
					}
				}
			}			
			APIC_JLG::newInstance()->addOpeLogBox( &s_OpeLogCom, &s_OpeLogBox );	// 監査ログ登録.
		} else {
// comment 2022/12/13 Sky東島 [IT6_2.3][OP_BTS-39836].
			// ボックスパスワード入力完了を設定.
			onlyPanel.setEnteringBoxPassword( PNLE_EnteringBoxPassword_Completed );
			memset ( PNLC_BoxDataLib::newInstance()->getSecretDocInfo().sc_secretPasswd, '\0', sizeof( PNLC_BoxDataLib::newInstance()->getSecretDocInfo().sc_secretPasswd ) );
			strncpy( PNLC_BoxDataLib::newInstance()->getSecretDocInfo().sc_secretPasswd, sc_MmbTextPassword, ( sizeof( PNLC_BoxDataLib::newInstance()->getSecretDocInfo().sc_secretPasswd ) - 1 ) );
// comment 2017/10/06 【HCL-RamKishor】【IT5_4.2】【RQ686】【WP900】 Supporting API for Network 1.
			s_OpeLogCom.uc_NetworkTypeOperationInfo = PNLC_NetworkLan::getInstance()->getLine();
// comment 2018/03/21 【HCL-Gadiya Sai】【IT5_4.2】【AR1042110】 For USer with admin rights.
//			s_OpeLogCom.uc_NetworkTypeOperatorInfo = PNLC_NetworkLan::getInstance()->getLine();
			s_OpeLogCom.uc_NetworkTypeOperatorInfo = TYPE_NetworkSpecific_NW1;										// Setting default as NW1.
			APIC_User* pc_User = PnlComGetUser();
			if( ( NULL != pc_User ) && ( pc_User->getSpecialAuthority( TYPE_AuthorityMode_Admin ) ) ) {
				short ss_AuthorityID = PnlComGetAuthorityID();
				if( TYPD_NotSetting != ss_AuthorityID ) {
					TYPS_SpecialAuthorityLoginInfo s_SpecialAuthorityLoginInfo;
					if( pc_User->getSpecialAuthorityLoginInfo( ss_AuthorityID,&s_SpecialAuthorityLoginInfo) ) {		// Checking whether user is having Admin rights or not.
						s_OpeLogCom.uc_NetworkTypeOperatorInfo = s_SpecialAuthorityLoginInfo.e_NWType;				// If login user having admin rights setting the login network type to joblog.
					}
				}
			}
			APIC_JLG::newInstance()->addOpeLogBox( &s_OpeLogCom, &s_OpeLogBox );	// 監査ログ登録.
// Comment 2017/09/23 [HCL-Abinav] [IT5 4.2][RQ-686] [WP706]Multiple Network support.
			if(TYPE_NetworkSpecific_NW2 == PNLC_NetworkLan::getInstance()->getLine() ) {
				s_DocSelectBoxInfo.sl_BoxId = DOCD_SecretDocumentsBoxNW2;		
			} else {
				s_DocSelectBoxInfo.sl_BoxId = DOCD_SecretDocumentsBox;
			}
		}
	}
	return !b_iswarning;
}
// キャンセル.
Bool PA056300100_1::prCancel( Bool )
{
	PNLC_TabletSecurityControlLib::newInstance()->securityAreaExited();
// Comment 2019/10/09 【HCLT-Krishna】【IT6_HeliosMLK】【制御構成変更_No.43】 SSDレス時の機密文書プリント対応
//// Comment 2018/11/19 【HCLT-Vandana】【IT6_DenebMLK】【制御構成変更_No.83】
//#ifdef DEF_DenebMLKCus
#if defined  (DEF_DenebMLKCus) || defined ( DEF_HeliosMLKCus )
	if ( True == PnlComIsSecurityDoc() ) {
		PNLC_PopupListControl::newInstance()->addPopup( &gp_HomeTopMenu, NULL, NULL, 0, 0, 0, 0, True );
// comment 2022/08/08 FPT.ThanhLH11 [IT6_2.2MR3][DenebMLK, HeliosMLK][OP_BTS-38933]共有ボックスの文書を選択してから印刷画面が表示不正問題対策。
		onlyPanel.b_nowBoxMode = False;
		onlyPanel.setTopmenuMode( True );
	}else{
		PNLC_PopupListControl::newInstance()->closeTop( False );
	}
// comment 2022/08/08 FPT.ThanhLH11 [IT6_2.2MR3][DenebMLK, HeliosMLK][OP_BTS-38933]共有ボックスの文書を選択してから印刷画面が表示不正問題対策。
// //comment 2019/08/20 [HCL-Rajesh]  [IT6_EagleZPlus_BKZPlus][OP_BTS-7534] The boxmode flag as set to false if secure print is cancelled and set to hometopmenu true to transit to the home screen.
// 	onlyPanel.b_nowBoxMode = False;
// 	onlyPanel.setTopmenuMode( True );
#endif
	return True;
}
//従来処理(画面外処理)---------------------------------------------------------------------------------------------------.
// 表示部品のセットアップ.
void PA056300100_1::setupGamen( short option )
{
	PPopupAnimationCom::setupGamen( option );
	printLEDControl( False );
// comment 2017/06/13 SCD2-2G-Sky長間 [IT5_4.1][AR1023355] Keyboard Linkによる入力時のOKキー活性化対応.
	A056300100_1_Btn0008.activate( ( ( ushort )strlen( A056300100_1_Gra0012.sc_Input ) > 0 ) ? True : False );
}
// 画面終了処理.
void PA056300100_1::endGamen()
{
// comment 2022/12/13 Sky東島 [IT6_2.3][OP_BTS-39836].
	if( PNLE_EnteringBoxPassword_Typing == onlyPanel.getEnteringBoxPassword() ){
		// ボックスパスワード入力中状態の場合は未入力状態にする.
		onlyPanel.setEnteringBoxPassword( PNLE_EnteringBoxPassword_NotEntered );
	}
	PPopupAnimationCom::endGamen();
	if( b_mSecurePrintMode ) {
		A056300100_1_Gra0000.changeSize( 784, 424 );
		A056300100_1_Btn0006.shiftPosition( 0, -16 );
		A056300100_1_Btn0008.shiftPosition( 0, -16 );
		A056300100_1_Btn0010.shiftPosition( 0, -16 );
		setOffset( 0, 8 );
	}
}
//画面固有処理-----------------------------------------------------------------------------------------------------------.
