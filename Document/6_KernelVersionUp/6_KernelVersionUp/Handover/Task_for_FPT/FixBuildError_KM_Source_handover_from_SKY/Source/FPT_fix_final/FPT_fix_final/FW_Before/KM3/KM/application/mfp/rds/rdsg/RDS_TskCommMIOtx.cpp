
/*
 * ファイル名	：
 *		RDS_TskCommMIO.cpp
 * 機能			：
 *		KMRDSアプリ・タスク通信処理部（ＭＩＯとの送信処理）
 *
 *				メール送信処理は、KMRDSアプリからコールした MIO の API 関数中でメールサーバと
 *				ダイレクトにやり取りをする仕組みであるため、KMRDSアプリのタスクが長時間ブロック
 *				される事を前提に設計しなくてはいけない。
 *				通常のタスク間メッセージを処理するタスクがブロックされると、その間に処理出来ない
 *				メッセージが蓄積され、メッセージキューのオーバーフローが発生する可能性が考えられる為、
 *				送信処理のみ別タスクに分け、タスクがブロックされても問題の無い仕組みとする。
 *
 *
 * 履歴
 *		02/04/02 浦崎 [初版]
 */



/***************************************************************************************************
 *   インクルードファイル
 ***************************************************************************************************/
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>		//TSS1_KMRDS

/* KMRDS ヘッダーファイル	---------------------------------*/
#include "RDS_Instance.h"
#include "RDS_VRmsc.h"
#include "RDS_TskEvent.h"
#include "RDS_TskCommMIO.h"
#include "RDS_FwNWDL.h"
#include "RDS_WebDAVClient.h"			// HHTP通信　08/02/06 寺内 新規追加 
#include "RDS_WebDAVTask.h"			    // HHTP通信　08/02/06 寺内 新規追加 
// 2017/6/27 MAv5 CSRA接続 start.
#include "RDS_Util.h"
// 2017/6/27 MAv5 CSRA接続 end.
#include "../rdsc/RDS_ReadCommand.h"	//2018/12/24 【FPT-VuLH13】【IT6_2.0】【EAGLEH】 No.36_FAX通信管理レポートの改善 Added Start.

/* IJC ヘッダーファイル		---------------------------------*/
#ifdef DEF_MoselCom	// marumoto changed 
				// 機種固有 これは必要なコンパイルＳＷ
//#include "system/thr/thrcom/THRC_Vxworks.h"
#include "system/thr/thrInclude/THRC_Vxworks.h"
#else
#include "system/thr/thr/bcvxworks.h"
#endif
#include "system/sys/OsWrapper/SYSC_OSWrapper.h"

// FWリファクタリング APIC_Setting.hの分割.
#include "system/nvd/APIC_Setting_MailSendSetting.h"
// 2017/6/27 MAv5 CSRA接続 start.
#include "RDS_CsraConnectionManager.h"
// 2017/6/27 MAv5 CSRA接続 end.

#include "system/nvd/APIC_CSRASetting.h"		// 2020/09/25 RQ-776 CWH 1-way通信方式の追加 start.
// 管理システム統合化開発 RQ-811 要求仕様1 2020/07/30 Sky二口 START.
#include "RDS_CsrcConnectionManager.h"
#include "system/sys/Roim/SYSC_Roim.h"
// 管理システム統合化開発 RQ-811 要求仕様1 2020/07/30 Sky二口 END.
/* Tornado2.2 ビルド対応 13D Marumoto */
// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/8 Sky梶谷 START.
#include "RDS_RptDataManager.h"
#include "RDS_CsrcPreCheck.h"
#include "RDS_CsraPreCheck.h"
// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/8 Sky梶谷 END.
#ifdef TORV22
#undef NULL
#define NULL	(0)
#endif
/* Tornado2.2 ビルド対応 13D Marumoto */

#if defined( ROM_DIV_3 )	/* marumoto 2006.3.27 NICFUM ROM_DIV */
extern char *pt_mioOpen;
extern char *pt_mioSetSendInfo;
extern char *pt_mioClose;
extern char *pt_mioSendData;
#endif

/***************************************************************************************************
 *   定数の定義
 ***************************************************************************************************/
const int _S_Frame_and_LEN = 10;	// 12△123456△ の 10 byte 

// CSRC定期発信用発信項目文字列数 added by 佐久間 07/04/12.
// 07/04/26 佐久間 カバレッジdataBlockに「V4」を追加(問題点No.79912対応).
// 09/01/30  宮尾  カバレッジdataBlockにV2,V3を追加(CQ314910対応).
// // 品質総点検 CQNo327671 sone 以下のデータブロックのコマンド内容を機種毎で分ける 2010/05/24.
//「Life count data Block, Life cycle data Block」「Adjustment data Block」「カバレッジ data Block」
// 10/03/08 yito	UV-00866		機能カウンタ対応 CSRD-System data BlockにAA,ABを追加.
// 10/03/08 yito	UV-00866		機能カウンタ対応 Component Information data BlockにAR,AS,AT,AUを追加.
// 10/03/08 yito	UV-00866		機能カウンタ対応 Service count data にBXを追加.
// 11/01/27 Iwata	UV以外の案件	FA14対策 History data にZAを追加.
// 11/01/07 sone CQNo336727対応 CSRC機能仕様書に合わせて修正.
// 11/01/19 yito UV-00867 EcoIndex V6コマンドを追加.
// 11/09/06 yito CQ354690 AHコマンドを追加.
// 2012/01/17 曽根	UV-01143対応 ZeusMLK、MinervaZeus2ndで使用する配列を新たに用意.
// 12/04/20 忠政 CQ364188　V3コマンドの削除.
// 2012/09/11 amiya CQ371634 SAコマンドを追加.
// 2013/01/18 tadamasa CQ376895 FAX3,4回線対応 SendCommandStr_2_1追加.
// 2013/03/15 satohiro CQ378202 SendCommandStr_2_1にXHコマンドを追加.
// 2013/02/14 satohiro ZM3rd UV-01120 CSRC HTTP通信の改善 SendCommandStr_3rd追加 M9コマンドの追加.
// 2014/03/05 ktakahashi [Zeus5BK][機種立ち上げ].
// 2014/07/11 ktakahashi ZeusS 制御構成変更No.1(機種立ち上げ).
// 2015/02/18 日立 森本郁 【MAv3】RQ-250 機能カウンタ拡張対応(機能バージョン 1.0、2.0対応).
// 2015/07/14 M.Adachi [ZeusS_ZX0]RQ-250(MAv3マージ対応)
// 2015/10/28 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(AQコマンド追加).
// 2015/11/05 ktakahashi ZeusS_ZX0 制御構成変更_No.76-2 ネットワークエラーカウンタ対応(02にENを追加).
// 2015/12/19 HCLT_Pawan VenusMLK【 管理No.8 】【WP135】機能バージョン added new table as SendCommandStr_Exp_05
// 2016/03/14 HCLT_Pawan VenusMLK CQ525067 Added AQ command in service count data block
// 2017/10/24 HCLT-Vinay IT6_1.0 [Eagle] 制御構成変更No.1 [機種立ち上げ].
// 2018/07/12 HCLT-Vinay IT6_1.0 Added ZP Command 制御構成変更_No.110 [Eagle] Modified.
// IT6 機能バージョン1.0 コマンド文字列作成用テーブル.
// 2017/10/24 HCLT-Vinay IT6_1.0 制御構成変更_No.1 [Eagle] Start.
// 2018/09/05 FPT-TruongNV6 【IT6_1.0】【Sparrow】 AR1052454 AddNew Start.
const int _S_Command_MAX_LEN = 11;
static const char* SendCommandStr[_S_Command_MAX_LEN] =
// 2018/09/05 FPT-TruongNV6 【IT6_1.0】【Sparrow】 AR1052454 AddNew Start.
{
#ifdef DEF_MonoCus
	" B0 BG B9 BH BI F6 F3 WC P1 BJ PH PK", 					// 01:Sales count data.
#elif DEF_A4_Cus
	" B0 BG B9 BH BI F6 F3 WC P1 PH PI PJ PK",
#else
	" B0 BG B9 BH BI F6 F3 WC P1 BJ PH PI PJ PK", 				// 01:Sales count data.
#endif
#ifdef DEF_A4_Cus
	" E0 E1 E3 E4 J0 E5 EN", 										// 02:Error count data.//2018/05/23 【IT6_1.0】【AR-1046539】HCLT-Jahnavi modified.
#else
	" E0 E3 E4 J0 E5 EN", 											// 02:Error count data.//2018/05/23 【IT6_1.0】【AR-1046539】HCLT-Jahnavi modified.
#endif
	" F0 F1 BX AP SV AQ",										// 03:Service count data.
	" G2 Z1 H1 H2 HC", 											// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
// 2018/01/29 HCLT-Baba IT6_1.0  [Eagle] 制御構成変更_No.84  Start.
//2018/12/03【FPT-VuLH13】【IT6_2.0】【EAGLEH】 No.36_FAX通信管理レポートの改善 Added XR command Start.
// 2019/04/11 FPT-SangTX 【IT6_2.0】 OP_BTS-4908  Modification Start.
// 2019/03/12 FPT-LinhVT3 【IT6_2.0】【EagleH】 OP_BTS_3339 Modification Start.
//#ifdef DEF_EagleCus
//	" Z0 XD Z8 ZA ZC ZG ZD ZE ZR XR", 			// 06:History data.
//#else
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR XR", 		// 06:History data. //2019/02/12 FPT-HaiVV3 Q&A93057 Remove commands XI XJ XK XL XM XN XO XP ZP.
//#endif
// 2019/03/12 FPT-LinhVT3 【IT6_2.0】【EagleH】 OP_BTS_3339 Modification End.
// 2019/04/11 FPT-SangTX 【IT6_2.0】 OP_BTS-4908  Modification End.
//2018/12/03【FPT-VuLH13】【IT6_2.0】【EAGLEH】 No.36_FAX通信管理レポートの改善 Added XR command End.
// 2018/01/29 HCLT-Baba IT6_1.0  [Eagle] 制御構成変更_No.84  End.
	" DX", 														// 07:EKC data Block.
	#ifdef DEF_A4_Cus
	" X0 X1 X2 XB XE AX AY AZ I9",
	#else
	" X0 X1 X2 XB XC XE XF XG XH AX AY AZ I9", 					// 08:Adjustment data Block.
	#endif
#ifdef DEF_MonoCus
// 2018/12/13 FPT-LinhVT3 【IT6_1.0】【DenebMLK】 AR1068962 Modification Start.
#ifdef DEF_DenebMLKCus
	" V2 V4 V6 V7 VA VB VD VE",									// 09:カバレッジ data Block.
#else
	" V2 V4 V5 V6 V7 VA VB VD VE",								// 09:カバレッジ data Block.
#endif //DEF_DenebMLKCus
// 2018/12/13 FPT-LinhVT3 【IT6_1.0】【DenebMLK】 AR1068962 Modification End.
#else
	" V2 V3 V4 V5 V6 V7 VA VB VD VE",							// 09:カバレッジ data Block.
#endif
	" AR AS AT AU",												// 10:Component Information data Block.
// 2018/09/05 FPT-TruongNV6 【IT6_1.0】【Sparrow】 AR1052454 AddNew Start.
//2018/12/03【FPT-VuLH13】【IT6_2.0】【EAGLEH】 No.36_FAX通信管理レポートの改善 Added XR command Start.
#ifdef DEF_SparrowCus
// 2019/10/28 y.hotta EagleZ+ OP_BTS-14998 センター通知項目11を選択時、XRコマンドも取得される Modify Start.
//	" XI XM XR",													// 11:Protocol Trace Data Block.
	" XI XM",													// 11:Protocol Trace Data Block.
#elif defined(DEF_EagleCus)
//	" XI XJ XK XL XM XN XO XP XR",									// 11:Protocol Trace Data Block.
	" XI XJ XK XL XM XN XO XP",									// 11:Protocol Trace Data Block.
// 2019/10/28 y.hotta EagleZ+ OP_BTS-14998 センター通知項目11を選択時、XRコマンドも取得される Modify End.	
//2018/12/03【FPT-VuLH13】【IT6_2.0】【EAGLEH】 No.36_FAX通信管理レポートの改善 Added XR command End.
// 2018/11/26 y.hotta AR1053477 [DenebMLK] 片方向通信でX系のコマンドが送信されない Add Start.
#elif DEF_MonoCus
	" XI XM",													// 11:Protocol Trace Data Block.
// 2018/11/26 y.hotta AR1053477 [DenebMLK] 片方向通信でX系のコマンドが送信されない Add End.
#else
	"",
#endif
// 2018/09/05 FPT-TruongNV6 【IT6_1.0】【Sparrow】 AR1052454 AddNew Start.
};
// 2017/10/24 HCLT-Vinay IT6_1.0 制御構成変更_No.1 [Eagle] End.

// 2018/09/26 FPT-LucNH2 【 IT6_2.0 】【EagleH】【制御構成変更_No.4】 Start.
// IT6 機能バージョン2.0 コマンド文字列作成用テーブル.
// 2019/08/27 SparrowBK OP_BTS-12328 機能毎コマンド文字列作成用テーブル不備対応 日立 江上 MOD Start.
static const char* SendCommandStr_Exp_01[_S_Command_MAX_LEN] =
// 2019/08/27 SparrowBK OP_BTS-12328 機能毎コマンド文字列作成用テーブル不備対応 日立 江上 MOD End.
{
#ifdef DEF_MonoCus
// 2019/06/21 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 MOD Start.
	#if defined(DEF_SparrowBKCus)
	" B0 BG B9 BH BI F6 F3 WC P1 PH PK",						// 01:Sales count data.
	#else
	" B0 BG B9 BH BI F6 F3 WC P1 BJ PH PK", 					// 01:Sales count data.
	#endif
// 2019/06/21 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 MOD End.
#elif DEF_A4_Cus
	" B0 BG B9 BH BI F6 F3 WC P1 PH PI PJ PK",
#else
	" B0 BG B9 BH BI F6 F3 WC P1 BJ PH PI PJ PK", 				// 01:Sales count data.
#endif
#ifdef DEF_A4_Cus
	" E0 E1 E3 E4 J0 E5 EN", 										// 02:Error count data.//2018/05/23 【IT6_1.0】【AR-1046539】HCLT-Jahnavi modified.
#else
	" E0 E3 E4 J0 E5 EN", 											// 02:Error count data.//2018/05/23 【IT6_1.0】【AR-1046539】HCLT-Jahnavi modified.
#endif
	" F0 F1 BX AP SV AQ",										// 03:Service count data.
	" G2 Z1 H1 H2 HC", 											// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
// 2018/01/29 HCLT-Baba IT6_1.0  [Eagle] 制御構成変更_No.84  Start.
// 2018/07/27 FPT-LucNH2 IT6_1.0  [Eagle] 機器構成変更_No.92 (#3) Start.
//2018/12/03【FPT-VuLH13】【IT6_2.0】【EAGLEH】 No.36_FAX通信管理レポートの改善 Added XR command Start.
// 2019/03/12 FPT-LinhVT3 【IT6_2.0】【EagleH】 OP_BTS_3339 Modification Start.
// 2019/06/27 y.hotta EagleH OP_BTS-7025 SparrowMFPのFAX通信で全コマンド取得が正常終了しない Delete Start.
//#if defined(DEF_EagleCus)
//	" Z0 XD Z8 ZA ZC ZG ZD ZE ZR XR", 							// 06:History data.
//#else
// 2019/06/27 y.hotta EagleH OP_BTS-7025 SparrowMFPのFAX通信で全コマンド取得が正常終了しない Delete End.
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR XR", 						// 06:History data. //2019/03/12 FTP-HaiVV3 Remove commands XI XJ XK XL XM XN XO XP ZP T7 T8 T9 TA.
//#endif
// 2019/03/12 FPT-LinhVT3 【IT6_2.0】【EagleH】 OP_BTS_3339 Modification End.
//2018/12/03【FPT-VuLH13】【IT6_2.0】【EAGLEH】 No.36_FAX通信管理レポートの改善 Added XR command End.
// 2018/07/27 FPT-LucNH2 IT6_1.0  [Eagle] 機器構成変更_No.92 (#3) End.
// 2018/01/29 HCLT-Baba IT6_1.0  [Eagle] 制御構成変更_No.84  End.
	" DX", 														// 07:EKC data Block.
// 2019/12/25 FPT-LucNH2 【IT6_2.0】 OP_BTS-14653  Modification Start.
	#if defined(DEF_A4_Cus) || defined(DEF_HeliosMLKCus)
	" X0 X1 X2 XB XE AX AY AZ I9",
// 2019/12/25 FPT-LucNH2 【IT6_2.0】 OP_BTS-14653  Modification End.
	#else
	" X0 X1 X2 XB XC XE XF XG XH AX AY AZ I9", 					// 08:Adjustment data Block.
	#endif
#ifdef DEF_MonoCus
	" V2 V4 V5 V6 V7 VA VB VD VE",								// 09:カバレッジ data Block.
#else
	" V2 V3 V4 V5 V6 V7 VA VB VD VE",							// 09:カバレッジ data Block.
#endif
	" AR AS AT AU",												// 10:Component Information data Block.
// 2019/08/27 SparrowBK OP_BTS-12328 機能毎コマンド文字列作成用テーブル不備対応 日立 江上 ADD Start.
// 2019/12/25 FPT-LucNH2 【IT6_2.0】 OP_BTS-14653  Modification Start.
#if defined(DEF_SparrowCus) || defined(DEF_HeliosMLKCus)
	" XI XM",													// 11:Protocol Trace Data Block.
// 2019/12/25 FPT-LucNH2 【IT6_2.0】 OP_BTS-14653  Modification End.
#elif defined(DEF_EagleCus)
	" XI XJ XK XL XM XN XO XP",									// 11:Protocol Trace Data Block.
#elif DEF_MonoCus
	" XI XM",													// 11:Protocol Trace Data Block.
#else
	"",
#endif
// 2019/08/27 SparrowBK OP_BTS-12328 機能毎コマンド文字列作成用テーブル不備対応 日立 江上 ADD End.
};
// 2018/09/26 FPT-LucNH2 【 IT6_2.0 】【EagleH】【制御構成変更_No.4】 End.

//2020/07/01 FPT-TinhNT16 【 IT6_2.1 】 制御構成変更_No.4 start.
// IT6 機能バージョン2.1 コマンド文字列作成用テーブル.
static const char* SendCommandStr_Exp_02[_S_Command_MAX_LEN] =
{
#ifdef DEF_MonoCus
	#if defined(DEF_SparrowBKCus)
	" B0 BG B9 BH BI F6 F3 WC P1 PH PK",							// 01:Sales count data.
	#else
	" B0 BG B9 BH BI F6 F3 WC P1 BJ PH PK", 						// 01:Sales count data.
	#endif
#elif defined DEF_A4_Cus
	" B0 BG B9 BH BI F6 F3 WC P1 PH PI PJ PK",
#else
	" B0 BG B9 BH BI F6 F3 WC P1 BJ PH PI PJ PK", 					// 01:Sales count data.
#endif
#ifdef DEF_A4_Cus
	" E0 E1 E3 E4 J0 E5 EN", 										// 02:Error count data.
#else
	" E0 E3 E4 J0 E5 EN", 											// 02:Error count data.
#endif
	" F0 F1 BX AP SV AQ",											// 03:Service count data.
	" G2 Z1 H1 H2 HC", 												// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
//2020/11/24 FPT-TinhNT16 【OP_BTS-29462】 [CSRC][制御構成変更_No.81][IT評価] Start.
#ifdef DEF_EagleCus
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR XR ZT", 						// 06:History data.
#else
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR XR", 							// 06:History data.
#endif
//2020/11/24 FPT-TinhNT16 【OP_BTS-29462】 [CSRC][制御構成変更_No.81][IT評価] End.
	" DX", 															// 07:EKC data Block.
	#if defined(DEF_A4_Cus) || defined(DEF_HeliosMLKCus)
	" X0 X1 X2 XB XE AX AY AZ I9",
	#else
	" X0 X1 X2 XB XC XE XF XG XH AX AY AZ I9", 						// 08:Adjustment data Block.
	#endif
#ifdef DEF_MonoCus
	" V2 V4 V5 V6 V7 VA VB VD VE",									// 09:カバレッジ data Block.
#else
	" V2 V3 V4 V5 V6 V7 VA VB VD VE",								// 09:カバレッジ data Block.
#endif
	" AR AS AT AU",													// 10:Component Information data Block.
#if defined(DEF_SparrowCus) || defined(DEF_HeliosMLKCus)
	" XI XM",														// 11:Protocol Trace Data Block.
#elif defined(DEF_EagleCus)
	" XI XJ XK XL XM XN XO XP",										// 11:Protocol Trace Data Block.
#elif defined DEF_MonoCus
	" XI XM",														// 11:Protocol Trace Data Block.
#else
	"",
#endif
};
//2020/07/01 FPT-TinhNT16 【 IT6_2.1 】 制御構成変更_No.4 end.

//2020/12/22 FPT-HaDV8 【 IT6_2.2 】 制御構成変更_No.4 Start.
// IT6 機能バージョン2.2 コマンド文字列作成用テーブル.
static const char* SendCommandStr_Exp_03[_S_Command_MAX_LEN] =
{
#ifdef DEF_MonoCus
	#if defined(DEF_SparrowBKCus)
	" B0 BG B9 BH BI F6 F3 WC P1 PH PK",							// 01:Sales count data.
	#else
	" B0 BG B9 BH BI F6 F3 WC P1 BJ PH PK", 						// 01:Sales count data.
	#endif
#elif defined DEF_A4_Cus
	" B0 BG B9 BH BI F6 F3 WC P1 PH PI PJ PK",
#else
	" B0 BG B9 BH BI F6 F3 WC P1 BJ PH PI PJ PK", 					// 01:Sales count data.
#endif
#ifdef DEF_A4_Cus
	" E0 E1 E3 E4 J0 E5 EN", 										// 02:Error count data.
#else
	" E0 E3 E4 J0 E5 EN", 											// 02:Error count data.
#endif
	" F0 F1 BX AP SV AQ",											// 03:Service count data.
	" G2 Z1 H1 H2 HC", 												// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
#ifdef DEF_EagleCus
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR XR ZT", 						// 06:History data.
#else
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR XR", 							// 06:History data.
#endif
	" DX", 															// 07:EKC data Block.
	#if defined(DEF_A4_Cus) || defined(DEF_HeliosMLKCus)
	" X0 X1 X2 XB XE AX AY AZ I9",
	#else
	" X0 X1 X2 XB XC XE XF XG XH AX AY AZ I9", 						// 08:Adjustment data Block.
	#endif
#ifdef DEF_MonoCus
	" V2 V4 V5 V6 V7 VA VB VD VE",									// 09:カバレッジ data Block.
#else
	" V2 V3 V4 V5 V6 V7 VA VB VD VE",								// 09:カバレッジ data Block.
#endif
	" AR AS AT AU",													// 10:Component Information data Block.
#if defined(DEF_SparrowCus) || defined(DEF_HeliosMLKCus)
	" XI XM",														// 11:Protocol Trace Data Block.
#elif defined(DEF_EagleCus)
	" XI XJ XK XL XM XN XO XP",										// 11:Protocol Trace Data Block.
#elif defined DEF_MonoCus
	" XI XM",														// 11:Protocol Trace Data Block.
#else
	"",
#endif
};
//2020/12/22 FPT-HaDV8 【 IT6_2.2 】 制御構成変更_No.4 End.

//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 Start.
// IT6 機能バージョン2.3 コマンド文字列作成用テーブル.
static const char* SendCommandStr_Exp_04[_S_Command_MAX_LEN] =
{
#ifdef DEF_MonoCus
	#if defined(DEF_SparrowBKCus)
	" B0 BG B9 BH BI F6 F3 WC P1 PH PK",							// 01:Sales count data.
	#else
	" B0 BG B9 BH BI F6 F3 WC P1 BJ PH PK", 						// 01:Sales count data.
	#endif
#elif defined DEF_A4_Cus
	" B0 BG B9 BH BI F6 F3 WC P1 PH PI PJ PK",
#else
	" B0 BG B9 BH BI F6 F3 WC P1 BJ PH PI PJ PK", 					// 01:Sales count data.
#endif
#ifdef DEF_A4_Cus
	" E0 E1 E3 E4 J0 E5 EN", 										// 02:Error count data.
#else
	" E0 E3 E4 J0 E5 EN", 											// 02:Error count data.
#endif
	" F0 F1 BX AP SV AQ",											// 03:Service count data.
	" G2 Z1 H1 H2 HC", 												// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
#ifdef DEF_EagleCus
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR XR ZT", 						// 06:History data.
#else
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR XR", 							// 06:History data.
#endif
	" DX", 															// 07:EKC data Block.
	#if defined(DEF_A4_Cus) || defined(DEF_HeliosMLKCus)
	" X0 X1 X2 XB XE AX AY AZ I9",
	#else
	" X0 X1 X2 XB XC XE XF XG XH AX AY AZ I9", 						// 08:Adjustment data Block.
	#endif
#ifdef DEF_MonoCus
	" V2 V4 V5 V6 V7 VA VB VD VE",									// 09:カバレッジ data Block.
#else
	" V2 V3 V4 V5 V6 V7 VA VB VD VE",								// 09:カバレッジ data Block.
#endif
	" AR AS AT AU",													// 10:Component Information data Block.
#if defined(DEF_SparrowCus) || defined(DEF_HeliosMLKCus)
	" XI XM",														// 11:Protocol Trace Data Block.
#elif defined(DEF_EagleCus)
	" XI XJ XK XL XM XN XO XP",										// 11:Protocol Trace Data Block.
#elif defined DEF_MonoCus
	" XI XM",														// 11:Protocol Trace Data Block.
#else
	"",
#endif
};
//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 End.

// comment 2018/10/10 SCD2-Sky東野 [IT6] [AR1062320] 不具合対応 Start.
// 2019/12/13 日立 齋藤 【SparrowBK_SFP】【OP_BTS-17015】コマンド文字列作成用テーブル仕様書不一致修正対応 Start.
// SparrowSFPはこちらのテーブルを使用.
static const char* SendCommandStr_SparrowSFP[_S_Command_MAX_LEN] =
{
//2019/11/11 FPT-LucNH2 【SparrowSFP2.0】 調査依頼、片方向通信ZA途中までしか通知されない Start.
	" B0 BG B9 BH BI F6 F3 WC P1 PH PI PJ PK",						// 01:Sales count data.
	" E0 E1 E3 E4 J0 E5 EN", 										// 02:Error count data.
	" F0 F1 BX AP SV AQ",											// 03:Service count data.
	" G2 Z1 H1 H2 HC", 												// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR",								// 06:History data.
	" DX", 															// 07:EKC data Block.
	" X0 X1 X2 XB XE AX AY AZ I9",									// 08:Adjustment data Block.
	" V2 V3 V4 V5 V6 V7 VA VB VD VE",								// 09:カバレッジ data Block.
	" AR AS AT AU",													// 10:Component Information data Block.
	"",																// 11:Protocol Trace Data Block.
//2019/11/11 FPT-LucNH2 【SparrowSFP2.0】 調査依頼、片方向通信ZA途中までしか通知されない End.
};
// comment 2018/10/10 SCD2-Sky東野 [IT6] [AR1062320] 不具合対応 End.
//2019/11/11 FPT-LucNH2 【SparrowSFP2.0】 調査依頼、片方向通信ZA途中までしか通知されない Start.
static const char* SendCommandStr_SparrowSFP_Exp_01[_S_Command_MAX_LEN] =
{
	" B0 BG B9 BH BI F6 F3 WC P1 PH PI PJ PK",						// 01:Sales count data.
	" E0 E1 E3 E4 J0 E5 EN", 										// 02:Error count data.
	" F0 F1 BX AP SV AQ",											// 03:Service count data.
	" G2 Z1 H1 H2 HC", 												// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR",								// 06:History data.
	" DX", 															// 07:EKC data Block.
	" X0 X1 X2 XB XE AX AY AZ I9",									// 08:Adjustment data Block.
	" V2 V3 V4 V5 V6 V7 VA VB VD VE",								// 09:カバレッジ data Block.
	" AR AS AT AU",													// 10:Component Information data Block.
	"",																// 11:Protocol Trace Data Block.
};
//2019/11/11 FPT-LucNH2 【SparrowSFP2.0】 調査依頼、片方向通信ZA途中までしか通知されない End.

//2020/07/01 FPT-TinhNT16 【 IT6_2.1 】 制御構成変更_No.4 start.
static const char* SendCommandStr_SparrowSFP_Exp_02[_S_Command_MAX_LEN] =
{
	" B0 BG B9 BH BI F6 F3 WC P1 PH PI PJ PK",						// 01:Sales count data.
	" E0 E1 E3 E4 J0 E5 EN",										// 02:Error count data.
	" F0 F1 BX AP SV AQ",											// 03:Service count data.
	" G2 Z1 H1 H2 HC",												// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR",								// 06:History data.
	" DX",															// 07:EKC data Block.
	" X0 X1 X2 XB XE AX AY AZ I9",									// 08:Adjustment data Block.
	" V2 V3 V4 V5 V6 V7 VA VB VD VE",								// 09:カバレッジ data Block.
	" AR AS AT AU",													// 10:Component Information data Block.
	"",																// 11:Protocol Trace Data Block.
};
//2020/07/01 FPT-TinhNT16 【 IT6_2.1 】 制御構成変更_No.4 end.

//2020/12/22 FPT-HaDV8 【 IT6_2.2 】 制御構成変更_No.4 Start.
static const char* SendCommandStr_SparrowSFP_Exp_03[_S_Command_MAX_LEN] =
{
	" B0 BG B9 BH BI F6 F3 WC P1 PH PI PJ PK",						// 01:Sales count data.
	" E0 E1 E3 E4 J0 E5 EN",										// 02:Error count data.
	" F0 F1 BX AP SV AQ",											// 03:Service count data.
	" G2 Z1 H1 H2 HC",												// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR",								// 06:History data.
	" DX",															// 07:EKC data Block.
	" X0 X1 X2 XB XE AX AY AZ I9",									// 08:Adjustment data Block.
	" V2 V3 V4 V5 V6 V7 VA VB VD VE",								// 09:カバレッジ data Block.
	" AR AS AT AU",													// 10:Component Information data Block.
	"",																// 11:Protocol Trace Data Block.
};
//2020/12/22 FPT-HaDV8 【 IT6_2.2 】 制御構成変更_No.4 End.

//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 Start.
static const char* SendCommandStr_SparrowSFP_Exp_04[_S_Command_MAX_LEN] =
{
	" B0 BG B9 BH BI F6 F3 WC P1 PH PI PJ PK",						// 01:Sales count data.
	" E0 E1 E3 E4 J0 E5 EN",										// 02:Error count data.
	" F0 F1 BX AP SV AQ",											// 03:Service count data.
	" G2 Z1 H1 H2 HC",												// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR",								// 06:History data.
	" DX",															// 07:EKC data Block.
	" X0 X1 X2 XB XE AX AY AZ I9",									// 08:Adjustment data Block.
	" V2 V3 V4 V5 V6 V7 VA VB VD VE",								// 09:カバレッジ data Block.
	" AR AS AT AU",													// 10:Component Information data Block.
	"",																// 11:Protocol Trace Data Block.
};
//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 End.

// 2019/06/21 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 ADD Start.
static const char* SendCommandStr_SparrowBkSFP[_S_Command_MAX_LEN] =
{
	" B0 BG B9 BI WC P1 PH PK", 									// 01:Sales count data.
	" E0 E1 E3 E4 J0 EN", 											// 02:Error count data.
	" F0 F1 BX AP SV AQ",											// 03:Service count data.
	" G2 Z1 H1 H2 HC", 												// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR", 								// 06:History data.
	" DX", 															// 07:EKC data Block.
	" AX AY AZ I9", 												// 08:Adjustment data Block.
	" V2 V4 V5 V6 V7 VA VB VD VE",									// 09:カバレッジ data Block.
	" AR AS AT AU",													// 10:Component Information data Block.
	"",																// 11:Protocol Trace Data Block.
};
// 2019/06/21 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 ADD End.
// 2019/12/13 日立 齋藤 【SparrowBK_SFP】【OP_BTS-17015】コマンド文字列作成用テーブル仕様書不一致修正対応 End.

//2020/07/01 FPT-TinhNT16 【 IT6_2.1 】 制御構成変更_No.4 start.
static const char* SendCommandStr_SparrowBkSFP_Exp_02[_S_Command_MAX_LEN] =
{
	" B0 BG B9 BI WC P1 PH PK", 									// 01:Sales count data.
	" E0 E1 E3 E4 J0 EN", 											// 02:Error count data.
	" F0 F1 BX AP SV AQ",											// 03:Service count data.
	" G2 Z1 H1 H2 HC", 												// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR", 								// 06:History data.
	" DX", 															// 07:EKC data Block.
	" AX AY AZ I9", 												// 08:Adjustment data Block.
	" V2 V4 V5 V6 V7 VA VB VD VE",									// 09:カバレッジ data Block.
	" AR AS AT AU",													// 10:Component Information data Block.
	"",																// 11:Protocol Trace Data Block.
};
//2020/07/01 FPT-TinhNT16 【 IT6_2.1 】 制御構成変更_No.4 end.

//2020/12/22 FPT-HaDV8 【 IT6_2.2 】 制御構成変更_No.4 Start.
static const char* SendCommandStr_SparrowBkSFP_Exp_03[_S_Command_MAX_LEN] =
{
	" B0 BG B9 BI WC P1 PH PK", 									// 01:Sales count data.
	" E0 E1 E3 E4 J0 EN", 											// 02:Error count data.
	" F0 F1 BX AP SV AQ",											// 03:Service count data.
	" G2 Z1 H1 H2 HC", 												// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR", 								// 06:History data.
	" DX", 															// 07:EKC data Block.
	" AX AY AZ I9", 												// 08:Adjustment data Block.
	" V2 V4 V5 V6 V7 VA VB VD VE",									// 09:カバレッジ data Block.
	" AR AS AT AU",													// 10:Component Information data Block.
	"",																// 11:Protocol Trace Data Block.
};
//2020/12/22 FPT-HaDV8 【 IT6_2.2 】 制御構成変更_No.4 End.

//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 Start.
static const char* SendCommandStr_SparrowBkSFP_Exp_04[_S_Command_MAX_LEN] =
{
	" B0 BG B9 BI WC P1 PH PK", 									// 01:Sales count data.
	" E0 E1 E3 E4 J0 EN", 											// 02:Error count data.
	" F0 F1 BX AP SV AQ",											// 03:Service count data.
	" G2 Z1 H1 H2 HC", 												// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR", 								// 06:History data.
	" DX", 															// 07:EKC data Block.
	" AX AY AZ I9", 												// 08:Adjustment data Block.
	" V2 V4 V5 V6 V7 VA VB VD VE",									// 09:カバレッジ data Block.
	" AR AS AT AU",													// 10:Component Information data Block.
	"",																// 11:Protocol Trace Data Block.
};
//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 End.

// 2019/09/17 【HeliosMLK】【Control No.1】【HELIOSMLK-73】 HIENG Start.
static const char* SendCommandStr_HeliosMLK_CN[_S_Command_MAX_LEN] =
{
	" B0 BG B9 BH BI F6 WC P1 BJ PH PI PJ PK",                      // 01:Sales count data.
	" E0 E3 E4 J0 EN",												// 02:Error count data.
	" F0 F1 BX AP SV AQ",											// 03:Service count data.
	" G2 Z1 H1 H2 HC",												// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR",								// 06:History data.
	" DX",															// 07:EKC data Block.
	" AX AY AZ I9",													// 08:Adjustment data Block.
	" V2 V3 V4 V5 V6 V7 VA VB VD VE",								// 09:カバレッジ data Block.
	" AR AS AT AU",													// 10:Component Information data Block.
	"",																// 11:Protocol Trace Data Block.
};
// 2019/09/17 【HeliosMLK】【Control No.1】【HELIOSMLK-73】 HIENG End.

//2020/07/01 FPT-TinhNT16 【 IT6_2.1 】 制御構成変更_No.4 start.
static const char* SendCommandStr_HeliosMLK_CN_Exp_02[_S_Command_MAX_LEN] =
{
	" B0 BG B9 BH BI F6 WC P1 BJ PH PI PJ PK",						// 01:Sales count data.
	" E0 E3 E4 J0 EN",												// 02:Error count data.
	" F0 F1 BX AP SV AQ",											// 03:Service count data.
	" G2 Z1 H1 H2 HC",												// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR",								// 06:History data.
	" DX",															// 07:EKC data Block.
	" AX AY AZ I9",													// 08:Adjustment data Block.
	" V2 V3 V4 V5 V6 V7 VA VB VD VE",								// 09:カバレッジ data Block.
	" AR AS AT AU",													// 10:Component Information data Block.
	"",																// 11:Protocol Trace Data Block.
};
//2020/07/01 FPT-TinhNT16 【 IT6_2.1 】 制御構成変更_No.4 end.

//2020/12/22 FPT-HaDV8 【 IT6_2.2 】 制御構成変更_No.4 Start.
static const char* SendCommandStr_HeliosMLK_CN_Exp_03[_S_Command_MAX_LEN] =
{
	" B0 BG B9 BH BI F6 WC P1 BJ PH PI PJ PK",						// 01:Sales count data.
	" E0 E3 E4 J0 EN",												// 02:Error count data.
	" F0 F1 BX AP SV AQ",											// 03:Service count data.
	" G2 Z1 H1 H2 HC",												// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR",								// 06:History data.
	" DX",															// 07:EKC data Block.
	" AX AY AZ I9",													// 08:Adjustment data Block.
	" V2 V3 V4 V5 V6 V7 VA VB VD VE",								// 09:カバレッジ data Block.
	" AR AS AT AU",													// 10:Component Information data Block.
	"",																// 11:Protocol Trace Data Block.
};
//2020/12/22 FPT-HaDV8 【 IT6_2.2 】 制御構成変更_No.4 End.

//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 Start.
static const char* SendCommandStr_HeliosMLK_CN_Exp_04[_S_Command_MAX_LEN] =
{
	" B0 BG B9 BH BI F6 WC P1 BJ PH PI PJ PK",						// 01:Sales count data.
	" E0 E3 E4 J0 EN",												// 02:Error count data.
	" F0 F1 BX AP SV AQ",											// 03:Service count data.
	" G2 Z1 H1 H2 HC",												// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR",								// 06:History data.
	" DX",															// 07:EKC data Block.
	" AX AY AZ I9",													// 08:Adjustment data Block.
	" V2 V3 V4 V5 V6 V7 VA VB VD VE",								// 09:カバレッジ data Block.
	" AR AS AT AU",													// 10:Component Information data Block.
	"",																// 11:Protocol Trace Data Block.
};
//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 End.

//2021/03/09 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 制御構成変更_No.4 Start.
static const char* SendCommandStr_MinervaSSBK_Exp_03[_S_Command_MAX_LEN] =
{
	" B0 BG B9 BH BI F6 F3 WC P1 BJ PH PK",							// 01:Sales count data.
	" E0 E3 E4 J0 E5 EN",											// 02:Error count data.
	" F0 F1 BX AP SV AQ",											// 03:Service count data.
	" G2 Z1 H1 H2 HC",												// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR XR ZT",							// 06:History data.
	" DX",															// 07:EKC data Block.
	" X0 X1 X2 XB XC XE XF XG XH AX AY AZ I9",						// 08:Adjustment data Block.
	" V2 V4 V5 V6 V7 VA VB VD VE",									// 09:カバレッジ data Block.
	" AR AS AT AU",													// 10:Component Information data Block.
	" XI XJ XK XL XM XN XO XP",										// 11:Protocol Trace Data Block.
};
//2021/03/09 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 制御構成変更_No.4 End.

//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 Start.
static const char* SendCommandStr_MinervaSSBK_Exp_04[_S_Command_MAX_LEN] =
{
	" B0 BG B9 BH BI F6 F3 WC P1 BJ PH PK",							// 01:Sales count data.
	" E0 E3 E4 J0 E5 EN",											// 02:Error count data.
	" F0 F1 BX AP SV AQ",											// 03:Service count data.
	" G2 Z1 H1 H2 HC",												// 04:Life count data Block, Life cycle data Block.
	" A0 A1 A5 A9 AA AB AH AI AJ I0 M4 M6 M7 M8 M9 S7 S8 SA TC",	// 05:CSRD-System data Block, Device config data.
	" Z0 XD Z8 Z9 ZA ZC ZG ZD ZE ZR XR ZT",							// 06:History data.
	" DX",															// 07:EKC data Block.
	" X0 X1 X2 XB XC XE XF XG XH AX AY AZ I9",						// 08:Adjustment data Block.
	" V2 V4 V5 V6 V7 VA VB VD VE",									// 09:カバレッジ data Block.
	" AR AS AT AU",													// 10:Component Information data Block.
	" XI XJ XK XL XM XN XO XP",										// 11:Protocol Trace Data Block.
};
//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 End.
static const char SendCommandTotalFCCntStr[10] = " OC PC FC";		// トータルカラーカウンタを算出するコマンド一覧 

#define RDSD_SegInfo_eNowCommModeInit static_cast<RDSE_CommMioMsg>(0xFF)	// 2019.12.16 OP_BTS-16268 新詳細コード強化.

/***************************************************************************************************
 *   実体の定義
 ***************************************************************************************************/
extern const char _O_EPNET[] ;
extern const char testMail_Body[];						/* test mail */

/*---------------------------------------------------------------------*/
/* 送信バッファ：                                                      */
/*	_S_Frame_and_LEN  フレームNo.と LENフィールドのレングス            */
/*            ：8                 暗号化した場合の最大増量(byte)       */
/*            ：5                 保険                                 */
// 2018/12/05 FPT-ChiNM IT6_1.0 【機器構成変更_No.92】 Modification Start.
//#define _S_txDataBuffSize_Email	(2048+100)
#define _S_txDataBuffSize_Email	(1024*16+100)		// 2019/03/14 FPT-DucDH4 IT6_1.0 【1073924】 Modification .1024*8 -> 1024*16
// 2018/12/05 FPT-ChiNM IT6_1.0 【機器構成変更_No.92】 Modification End.
//static uchar uc_txData[ _S_txDataBuffSize_Email + _S_Frame_and_LEN +8+5 ];
static uchar uc_txData[ _S_txDataBuffSize_Email + _S_Frame_and_LEN +8+5 +16 ];

// 2014/07/08 wada CQ500942_CSRCでのバックトレースログ取得改善 s.
// ZAコマンドRead要求フラグ.
extern Bool b_ReadZA_ReadReqFlg;
// 送信済みフラグ保存許可.
extern Bool b_ReadZA_SendReqFlg;
// 2014/07/08 wada CQ500942_CSRCでのバックトレースログ取得改善 e.

// 2019.12.16 OP_BTS-16268 新詳細コード強化 start.
Bool b_RdsSegInfoIsCsraConnectionManagerSending = False;
RDSE_Email_CommMode e_RdsSegInfoNowCommMode = RDSD_SegInfo_eNowCommModeInit;
// 2019.12.16 OP_BTS-16268 新詳細コード強化 end.

/***************************************************************************************************
 *   プロトタイプ宣言
 ***************************************************************************************************/
extern void dbg_PrintMioSendEmail( MioSendEmail* pSrc );
extern void dbg_PrintMLog( void );


/***************************************************************************************************
 *   関数
 ***************************************************************************************************/
/***************************************************************************************************
 *   MIO との通信処理クラス
 ***************************************************************************************************/
/***************************************************************************************************
 *   ◆ 送信時のシーケンス
 ***************************************************************************************************/
/*==================================================================================================*/
//概要　：発信開始要求MSG
//備考　：
/*--------------------------------------------------------------------------------------------------*/
// 04/06/15 浦崎 問題点6879対策。
//               SMTPチャネルが MIO_STATUS_READY 以外の時は、送信シーケンスを開始しないようにした。
//               ⇒ SMTPチャネルがReady以外の時に送信処理を実行しようとしていたため、
//                  本来、送信シーケンスが始まる前に送信処理が行えない状態である事を検出し、
//                  シーケンスを行わないようにしていた処理が有効に働かず、送信シーケンスを
//                  実行してしまっていた。
//                  そのため、シーケンスがエラーとなってしまい、１時間後に再送する処理が
//                  走ってしまっていた。 
// 04/11/09 浦崎 InternetISW 対応のため、引数を、CSRC通信、テストメール通信、InternetISW通信と、
//               モードを指定出来るようにした。 
/*==================================================================================================*/
void RDSC_RdsCommMIO::reqTransStart( RDSE_Email_CommMode e_CommMIO_CommMode )
{
	RDSS_EventMsg msgBuf;
	msgBuf.ul_msg   = (int32_t)CommMioMSG_reqTransStart;
	msgBuf.ul_para1 = (int32_t)e_CommMIO_CommMode;				//( True == b_SendTestMailReq ? 1 : 0 );
	msgBuf.ul_para2 = (int32_t)0;
	msgBuf.ul_para3 = (int32_t)0;
	msgBuf.ul_para4 = (int32_t)0;
	msgBuf.ul_para4 = (int32_t)0;

	SYSF_msgQSend( msg_Q_ID_MainTask, (char*)&msgBuf, sizeof(RDSS_EventMsg), WAIT_FOREVER, MSG_PRI_NORMAL );
}
/**/
void RDSC_RdsCommMIO::msg_reqTransStart( uint32_t ul_CommMode )
{
	MIO_CHAN_STATUS		chStat;

	if(    (NOT_ACT == tskRdsEvent.c_ActPort.getActLevel())		/* 03/11/06 浦崎 嘘アクト(即ち NOT_ACT 以外)でも動くようにする。 */
		|| (True != theRdsApl.b_connectStat_Cntrl_LL)			/* 接続確認 */
		|| (True != isMIOActive())								/* 04/02/19 浦崎 */
		|| (RDSE_STS_Trans != theRdsApl.e_CommStatus)
		|| (MIO_Idring_State != transState)	)
	{
		ushort us_tmpErrCode = theRdsEmail.cnvEmailErrCode( MLog_Nothing, RDSE_ERR_MIO_NOT_READY_OTHER );			// パネルに表示するエラーコード作成。  
		tskRdsEvent.ansTransStart( RDSE_TSK_ID_MIO, False, RDSE_ERR_MIO_NOT_READY_OTHER, us_tmpErrCode );
		return;
	}

	chStat = getStatusSMTP();
	//if(    (MIO_STATUS_NOT_INSTALLED == chStat)
	//	|| (MIO_STATUS_INITIALIZING  == chStat)    )
	if( MIO_STATUS_READY !=	chStat )
	{
		EPN_DBG_MESSAGE( (uchar*)"RDS_MIOtx", (uint32_t)chStat, (uchar*)" (SMTPチャネル使用不可状態)" );
		ushort us_tmpErrCode = theRdsEmail.cnvEmailErrCode( MLog_Nothing, RDSE_ERR_MIO_NOT_READY_SMTP );			// パネルに表示するエラーコード作成。  
		tskRdsEvent.ansTransStart( RDSE_TSK_ID_MIO, False, RDSE_ERR_MIO_NOT_READY_SMTP, us_tmpErrCode );
		return;
	}


	// 以降、通常処理： 
	cancelTimer();																/* タイマーリセット */
	transState = MIO_Trans_MakeText_State;										/* ステートをテキスト作成完了待ちに。 */
	e_NowCommMode = (RDSE_Email_CommMode)ul_CommMode;

	// ansTransStart() 
	tskRdsEvent.ansTransStart( RDSE_TSK_ID_MIO, True, 0 );
	theVRcontroller.c_MailLog.indStartRecLog();									/* 通信ログ記録開始通知 */
	switch( e_NowCommMode )
	{
	case RDSE_Email_CommModeTestMail:		// テストメール通信 
		EPN_DBG_MESSAGE( (uchar*)"RDS_MIOtx", 0, (uchar*)" ＜TestMail 送信開始要求受信＞" );


		/* テストメール送信時は、イベントデータ作成処理は行わない。 */
		ansStartTransPhase( True );		// 送信開始要求をTrue固定でコール.	// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
		break;

	case RDSE_Email_CommModeISW:			// InternetISW通信 
		EPN_DBG_MESSAGE( (uchar*)"RDS_MIOtx", 0, (uchar*)" ＜InternetISW 送信開始要求受信＞" );

		// reqStartTransPhase()  
		tskRdsEvent.reqStartTransPhase( RDSE_TSK_ID_MIO, 0 );
		setTimer( 10*RDSD_Second );												/* テキスト作成完了待ちタイマーセット */
		break;

	case RDSE_Email_CommModeRds:			// CSRC通信 
	default:
		EPN_DBG_MESSAGE( (uchar*)"RDS_MIOtx", 0, (uchar*)" ＜KMRDS 送信開始要求受信＞" );

		// reqStartTransPhase()  
		tskRdsEvent.reqStartTransPhase( RDSE_TSK_ID_MIO, 0 );
		setTimer( 10*RDSD_Second );												/* テキスト作成完了待ちタイマーセット */
		break;
	}
}
/*==================================================================================================*/
//概要　：送信フェイズ応答MSG
//備考　：
/*==================================================================================================*/
void RDSC_RdsCommMIO::ansStartTransPhase( Bool result )
{
	if( True == result )
	{
		RDSS_EventMsg msgBuf;
		msgBuf.ul_msg   = (int32_t)CommMioMSG_indTransStart;
		msgBuf.ul_para1 = (int32_t)0;
		msgBuf.ul_para2 = (int32_t)0;
		msgBuf.ul_para3 = (int32_t)0;
		msgBuf.ul_para4 = (int32_t)0;
		msgBuf.ul_para4 = (int32_t)0;

		SYSF_msgQSend( msg_Q_ID_TxSqTask, (char*)&msgBuf, sizeof(RDSS_EventMsg), WAIT_FOREVER, MSG_PRI_NORMAL );
	}
	else
	{
		stopComm( MLog_Nothing, RDSE_ERR_MIO_NAK_ANS_StartTransPhase, RDSE_ERR_MIO_NAK_ANS_StartTransPhase );			/* シーケンス破棄（１分後再開） */
	}
}
/*==================================================================================================
//	MIO と送信処理を行うためのタスク
//--------------------------------------------------------------------------------------------------
// 02/04/02 浦崎 [初版]
==================================================================================================*/
void RDSC_RdsCommMIO::task_txMIO( void )
{
	uint32_t						ul_Msg;
	// 2015/04/09 mtoyoda Helios Warning除去対応.
	//uint32_t						ul_Option;
	RDSS_EventMsg				msg;



	// ２）メッセージループ
	while(1) {
		// メッセージ受信（シュミレータ上では受信できない場合、関数終了）  
		if( ERROR == SYSF_msgQReceive( msg_Q_ID_TxSqTask, (char*)&msg, sizeof(RDSS_EventMsg), WAIT_FOREVER ) )
		{
			return;
		}

		setDeepSleepDelay_CSRC();		// ディープスリープへの移行を抑止する.		// 2010/11/22 higuchi Minervs UV-00719 【スリープ時の省電力対応】.
		// CSRC Debug Yamamura Start.
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"###RDSC_RdsCommMIO::task_txMIO", (uint32_t)msg.ul_msg, (uint32_t)msg.ul_para1, (uint32_t)msg.ul_para2 );
		EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"msg", (uint32_t)msg.ul_para3, (uint32_t)msg.ul_para4, (uint32_t)msg.ul_para5 );
		// CSRC Debug Yamamura End.


		// ↓ 過去のコードが動くようにするためのだまし 
		ul_Msg		= msg.ul_msg;
		// 2015/04/09 mtoyoda Helios Warning除去対応.
		//ul_Option	= msg.ul_para1;

		// 通常のメッセージ処理
		if( True == isMIOActive() ) {
// 2017/6/27 MAv5 CSRA接続 start.
			switch( ul_Msg ) {
			case CommMioMSG_indTransStart:
// 2017/9/12 MAv5 AccuRev 1028998 start.
				if ( True == RDSC_CsraConnectionManager::newInstance()->isSending() ) {
// 2017/9/12 MAv5 AccuRev 1028998 end.
					b_RdsSegInfoIsCsraConnectionManagerSending = True;	// 2019.12.16 OP_BTS-16268 新詳細コード強化.
					tx_transCsraSq();
// 管理システム統合化開発 RQ-811 要求仕様1 2020/07/30 Sky二口 START.
				} else if ( True == RDSC_CsrcConnectionManager::newInstance( RDSE_CommunicationType_Immediate )->isSending() ) {
					// WebDAV送信失敗時はログにファイル名を残したいため、.
					// ファイル名は送信前のここで作成しておく.
					std::string c_FileName("");
					RDSC_CsrcUtil::updateFileName( &c_FileName );
					
					// SYSにファイル名を一時保存.
					if ( !c_FileName.empty() ) {
						APIC_RoimWscDataIF::newInstance()->setCSRCCommandInfoFileName( ( const char* )c_FileName.c_str() );
					} else {
						tskRdsEvent.procEndCreateResFile( TYPE_RoimWscDataIF_ResultInfo_Nack, TYPD_CSRCFORWW_RESULT_RDS_InternalError );
						return;
					}
					
					// コピーしておく.
					memset( e_WebDAVClient.filename, 0, sizeof(e_WebDAVClient.filename) );
					strncpy( e_WebDAVClient.filename, c_FileName.c_str(), sizeof(e_WebDAVClient.filename) - 1 );
					
// 2021/01/20 Sky二口 [OP_BTS-30824][RQ-811]即時要求用の通信履歴Logの追加 START.
					// 送信シーケンスに入る前に通信種別を設定しておく.TYPE_RdsTrgKindRes_WW
					theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindRes_WW );
// 2021/01/20 Sky二口 [OP_BTS-30824][RQ-811]即時要求用の通信履歴Logの追加 END.

					/* RQ-811ではファイル送信シーケンスに入る前に、応答ファイルの作成が完了しているため、
					   直接WebDAV送信シーケンスに入る。*/
					RDSD_DBG_MESSAGE_STR( "CSRCForWW : Start WebDAV Send Sequence!!", 0, 0, 0 );
					e_Webdavtask.reqNicMsgStart();		// HTTP通信処理部（自タスク）へ「送信開始」MSG送信.
// 管理システム統合化開発 RQ-811 要求仕様1 2020/07/30 Sky二口 END.
// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/22 Sky梶谷 START.
				} else if( (True == RDSC_RptDataManager::newInstance()->isAvailableSendReport())		// レポート発信が有効な場合.
						&& (True != tskRdsEvent.isNowTrans(TYPE_RdsTrgKindIni))							) {	// 初期発信は、通常CSRCで処理する.
// 管理システム統合化開発 RQ-893 [OP_BTS-32775] 2021/04/20 Sky大橋 START.
					//EPN_DBG_MESSAGE( (uchar*)"CSRC", (ushort)0, (uchar*)"CSRAレポート発信 通知ファイル生成" );
					EPN_DBG_MESSAGE( (uchar*)"CSRC", (ushort)0, (uchar*)"CSRAレポート発信 通知ファイル生成 / 送信" );
// 管理システム統合化開発 RQ-893 [OP_BTS-32775] 2021/04/20 Sky大橋 END.
					tx_transCsraReportSq();
// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/22 Sky梶谷 END.
				} else if( (RDSE_STS_Trans  == theRdsApl.e_CommStatus)
					   &&( (RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice)
					    || (RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice)
					    || (RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice) ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
				{
					EPN_DBG_MESSAGE( (uchar*)"CSRC", (ushort)0, (uchar*)" ＜新タスク：MIO と送信処理を開始します＞" );
					e_RdsSegInfoNowCommMode = static_cast<RDSE_CommMioMsg>(e_NowCommMode);	// 2019.12.16 OP_BTS-16268 新詳細コード強化.
					tx_transSq();							// 送信シーケンス開始
				} else {
					// NOP.
				}
				break;
			default:
				break;
			}
// 2017/6/27 MAv5 CSRA接続 end.
			EP_SIM_RETURN();								// 実機コンパイル以外であれば、リターンするマクロ
		}
		resetDeepSleepDelay_CSRC();		// ディープスリープへの移行を抑止解除する.		// 2010/11/22 higuchi Minervs UV-00719 【スリープ時の省電力対応】.
		
		// 2019.12.16 OP_BTS-16268 新詳細コード強化 start.
		b_RdsSegInfoIsCsraConnectionManagerSending = False;
		e_RdsSegInfoNowCommMode = RDSD_SegInfo_eNowCommModeInit;
		// 2019.12.16 OP_BTS-16268 新詳細コード強化 end.
	} //----- while(1) -----
}

extern ushort us_CsrcCenterSendSettingDebug;
/*==================================================================================================
//	MIO とのメール送信シーケンス（テキスト作成完了後）
//--------------------------------------------------------------------------------------------------
// 02/04/03 浦崎  接続タイムアウト時の処理をコーディング 
// 04/06/04 浦崎  発信種別にテストメールをセット。 
// 04/06/14 浦崎  応答発信バックアップ対象外対応。
//                ・センターからのコマンドを揮発バッファから読み出すようにした。 
// 05/10/27 浦崎  Nicfum4.x 対応：To、Cc、Bcc の初期化に対応。 
// 06/05/19 佐久間  AES暗号化対応(パディングによりサイズが増えるため一度に送信できる最大サイズを16Byte減) 
// 07/04/12 佐久間 E-mail片方向通信における定期発信対応(No.78385)  
// 07/09/13 佐久間 E-mail版初期発信時A1及びB0コマンドを送信するように修正(No.84544)
// 13/02/14 佐藤宏 HB定期発信の処理をFuncVerによって切り分ける ZM3rd UV-01120 CSRC HTTP通信の改善.
// 14/03/05 ktakahashi [Zeus5BK][機種立ち上げ].
// 14/07/11 ktakahashi ZeusS 制御構成変更No.1(機種立ち上げ).
// 14/10/20 ktakahashi HeliosL 制御構成変更No.1 No.3 機種立ち上げ.
// 16/02/01 Sky平田 ZeusS CQ521285 監査ログ HCD-PP対応.
==================================================================================================*/
// AES対応 06.05.19 佐久間 
//#define MIO_SEND_UNIT	(1024*2)		// 一度に送信できる最大サイズ 
//#define MIO_SEND_UNIT	(1024*2-16)		// 一度に送信できる最大サイズ 
// 2019/03/14 FPT-DucDH4 IT6_1.0 【1073924】 Modification MIO_SEND_UNIT.1024*8 -> 1024*16
#define MIO_SEND_UNIT	(1024*16-16)		// 一度に送信できる最大サイズ // 2018/12/23 FPT-ChiNM IT6_1.0 【機器構成変更_No.92】changed 
void RDSC_RdsCommMIO::tx_transSq( void )
{
	Bool				result;
	int					sendInfoAns = 0;
	int					fd = ERROR;
	MioSendEmail		sendinfo;
	MioEmailSet			emset;
	MioMIMEBodyHeader*	pBodyHeader;
    EmailAddrList  AddrTo = {"", "", NULL, NULL, 0};
    EmailAddrList  AddrCc = {"", "", NULL, NULL, 0};
    EmailAddrList  AddrBcc= {"", "", NULL, NULL, 0};
	uchar				uc_TmpHostCmd[2048];
	RDSC_RdsEmail::RDSS_EmailCmdResult	e_cmdResult;
	memset( &e_cmdResult, 0, sizeof(RDSC_RdsEmail::RDSS_EmailCmdResult) );		// 2012/08/28 sone ZeusMLK 静的解析 影響度高 CID_434985.
	uchar uc_ReadFWVersionCommand[2048] = {""};									// 2011/02/07 higuchi Minerva/Zeus 【静的解析 影響度高 CID_014697】.
	memset( uc_ReadFWVersionCommand, 0x00, sizeof(uc_ReadFWVersionCommand) );	// 2011/02/07 higuchi Minerva/Zeus 【静的解析 影響度高 CID_014697】.
	strcpy( (char*)uc_ReadFWVersionCommand, "82 000008 RD S7 S9");				// 2011/02/07 higuchi Minerva/Zeus 【静的解析 影響度高 CID_014697】.		// 2010/10/08 higuchi Minerva UV-00966 【機種コード取り込み】.

// 07/04/12 佐久間 E-mail片方向通信における定期発信対応(No.78385)  
	ushort				us_CsrcCenterSendSetting;
	const char			uc_CsrcCycletmpFormat[] = "82 000000 ";
	uchar*				puc_tmpCommand = &uc_TmpHostCmd[10];
	uint32_t				ul_Length = 0x000000;
	char				sc_Length[7];
	
	TYPE_RdsTrgKind		e_RdsTrgKind = 0;
	bool				b_Trgflg = false;	// トリガ発信検出用フラグ 

	if( MIO_Trans_MakeText_State != transState )
	{
		stopComm( MLog_Nothing, RDSE_ERR_MIO_ILLEGAL_STATE_ERR_1, RDSE_ERR_MIO_ILLEGAL_STATE_ERR_1 );	// シーケンス破棄（１分後再開）
		return;
	}

	cancelTimer();

	short	ss_NumOfCommandResFiles = 0;
	short	ss_NumOfEventDataFiles = 0;
	short	ss_NumOfAllFiles = 0;

	// HB排他制御の為に追加
	EPN_DBG_MESSAGE((uchar*)"HB", (uint32_t)e_Webdavtask.localTrg, (uchar*)"ローカルトリガ");
	
	
	if( c_HBdetect.b_TransHB != True )
	{
		switch( e_NowCommMode )
		{
		case RDSE_Email_CommModeTestMail:		// テストメール通信 -------------------------------------------------------------------------- 
			EPN_DBG_MESSAGE( (uchar*)"CSRC", (ushort)0, (uchar*)" ＜Test Mail＞" );
			theVRcontroller.c_MailLog.setKind( TYPE_RdsTrgKindTestMail );				// 発信種別にテストメールをセット 04/06/04 浦崎  
			break;

		case RDSE_Email_CommModeISW:			// InternetISW通信 -------------------------------------------------------------------------- 

			// 結果通知発信か？（今回の通信で S7コマンドの送信が必要であるか否か） 
			//if( True == c_FND.isTxResultMode() )
			if( c_FND.uc_transMap[1][1] )										/* 結果通知発信 */
			{
				uint32_t ul_ErrCode = pc_RdsSetting->getRdsISWMode_ResultErrCode();
				EPN_DBG_MESSAGE( (uchar*)"CSRC", (ushort)ul_ErrCode, (uchar*)" ＜InternetISW 結果通知発信＞" );
				if( ul_ErrCode == 0 )
				{
					theRdsEmail.analizeHostCommand(
						RDSE_Email_CommModeISW,
						RDSE_AnaMode_ReadWD,					// モード：InternetISW の S7 コマンドリード時に使う。 
						(const uchar*)uc_ReadFWVersionCommand,				// ホストコマンド文字列 
						&e_cmdResult,										// 処置結果 ★ISW の応答の場合、エラーはありえない。 
						&q_RD_Data,											// ReadCommand() にて読み込んだデータ 
						&ss_NumOfCommandResFiles,							// RD による応答ファイル数 
						&q_SendInfo											// 上記のファイルごとのファイルサイズ 
					);
				}
				else
				{
					// エラーコードがセットされている場合は、ここでは何もしない。 
					// イベントデータ作成処理で送信データが作成されている。 
				}
			}
			
			if( c_FND.uc_transMap[1][0] )										/* TG への応答発信 */
			{
				// TG に対する応答発信  
				// RD コマンドを処理する。 ----------------------------------------------------- 
				// バックアップしたセンターコマンドを解析して処理してみる。 
				// pc_RdsSetting->getHostCmdSave( uc_TmpHostCmd ); 04/06/14 浦崎 ホストからのコマンドは不揮発への保存対象でなくなったことへの対応。 
				theRdsEmail.getHostCmdData( uc_TmpHostCmd, (ushort)sizeof(uc_TmpHostCmd) );
	
	
				// ※ 戻り値で RD と、WR の応答による添付ファイル数を得る。 
				//e_cmdResult.ul_TypeCode = pc_RdsSetting->getEmailResultTypeCode();
				e_cmdResult.ul_ErrCode = pc_RdsSetting->getEmailResultErrCode();
				if( 0 == e_cmdResult.ul_ErrCode )
				{
					// 2013.12.19 H.Wada [ZeusCD][UV-01385] WP73.
					if( True == c_FND.getReceiveWriteT6() )
					{
						//T6
						e_cmdResult.ul_ErrCode = c_FND.getResT6_ErrCode();
					}
					else
					{
						//TG
						e_cmdResult.ul_ErrCode = c_FND.getResTG_ErrCode();
					}
					// 2013.12.19 H.Wada [ZeusCD][UV-01385] WP73.
				}
	
				EPN_DBG_MESSAGE( (uchar*)"CSRC", (ushort)e_cmdResult.ul_ErrCode, (uchar*)" ＜InternetISW 応答発信＞" );
				if( 0 == e_cmdResult.ul_ErrCode )						// エラーが発生していない場合  
				{
					theRdsEmail.analizeHostCommand(
						RDSE_Email_CommModeRds,
						RDSE_AnaMode_OnlyRD,					// モード：RDコマンドのみ処理する。 
						uc_TmpHostCmd,										// ホストコマンド文字列 
						&e_cmdResult,										// 処置結果 
						&q_RD_Data,											// ReadCommand() にて読み込んだデータ 
						&ss_NumOfCommandResFiles,							// RD による応答ファイル数 
						&q_SendInfo											// 上記のファイルごとのファイルサイズ 
					);
					// センターから受信したコマンドのチェックは、E-mail受信完了時に行われている。 
					// よって、下記処理は不要。 ⇒ 【下記処理は行ってはいけない。】 
					//pc_RdsSetting->setEmailResultTypeCode( e_cmdResult.ul_TypeCode );
					//pc_RdsSetting->setEmailResultErrCode( e_cmdResult.ul_ErrCode );
	
					EPN_DBG_MESSAGE( (uchar*)"tx_transSq()", (ushort)e_cmdResult.ul_ErrCode, (uchar*)" <<< e_cmdResult.ul_ErrCode" );
				}
			}
	
			ss_NumOfEventDataFiles  = c_EvevntDataBuf.isNum();							// イベントデータファイル数 
			ss_NumOfAllFiles = (short)(ss_NumOfCommandResFiles + ss_NumOfEventDataFiles);	// 2010/07/17 sone Minerva warning 対応 shortにキャスト.
			break;
	
		case RDSE_Email_CommModeRds:			// CSRC通信---------------------------------------------------------------------------------- 
		default:
			// RD コマンドを処理する。 ----------------------------------------------------- 
			// バックアップしたセンターコマンドを解析して処理してみる。 
			// pc_RdsSetting->getHostCmdSave( uc_TmpHostCmd ); 04/06/14 浦崎 ホストからのコマンドは不揮発への保存対象でなくなったことへの対応。 
			theRdsEmail.getHostCmdData( uc_TmpHostCmd, (ushort)sizeof(uc_TmpHostCmd) );
	
			// イベントデータ作成完了 
			//EPN_DBG_MESSAGE( (uchar*)"CSRC", (ushort)0, (uchar*)" ＜イベントデータ作成完了！＞" );
			//EPN_DBG_MESSAGE( (uchar*)"CSRC", (ushort)0, (uchar*)uc_TmpHostCmd); // 要求されたコマンド文字列.
			EPN_DBG_PRINTF( "[CommMIO::tx_transSq]＜イベントデータ作成完了！＞%s", uc_TmpHostCmd );
	
// CQ342723 FQ-CSOPS-1100130 便乗発信の修正 2011/03/10 y.ito start.
			// 応答発信[発信中]のときは、無視するようにする. 
			EPN_DBG_MESSAGE( (uchar*)"CSRC", (ushort)0, (uchar*)"★1:start");
			if( False == theTrgMap.isExistOnGoingTrg( TYPE_RdsTrgKindRes_RDS ) )
			{
				EPN_DBG_MESSAGE( (uchar*)"CSRC", (ushort)0, (uchar*)"★2");

				// Donau新機能対応.   
				// MFP側からの発信要求があった場合（0 < ss_NumOfEventDataFiles）は、.
				// 特定のデータリード用コマンドを MFP側から自発的に送信する。       .
				while( ( false == b_Trgflg ) &&							// トリガ発信検出用フラグ 
					   ( e_RdsTrgKind <  TYPE_RdsTrgKindNumberOfTrg ) )		
				{
					//下記3発信は、MFPからの自発的な発信ではないのでトータルカラーカウンタは付与しない.
					//期日指定発信は自発的な発信だが、この関数内の、期日指定発信専用処理にてトータルカラーカウンタの要求コマンドを付与しているため、不要。
					if ( (e_RdsTrgKind != TYPE_RdsTrgKindCenter)  &&
						 (e_RdsTrgKind != TYPE_RdsTrgKindRes_RDS) )
					{
						b_Trgflg = theTrgMap.isExistOnGoingTrg( e_RdsTrgKind );
					}
					
					e_RdsTrgKind++;
				}
			}
			EPN_DBG_MESSAGE( (uchar*)"CSRC", (ushort)b_Trgflg, (uchar*)"★1:end");
// CQ342723 FQ-CSOPS-1100130 便乗発信の修正 2011/03/10 y.ito end.
	
			if( b_Trgflg ) 
			{
				ul_Length = 0;
				memset( &uc_TmpHostCmd[0], 0, sizeof(uc_TmpHostCmd) );
				
				// "82 000000 "を付与.
				memcpy( &uc_TmpHostCmd[0], &uc_CsrcCycletmpFormat[0], strlen( uc_CsrcCycletmpFormat ) );
				
				// RDフォーマットを付与.
				memcpy( puc_tmpCommand, (uchar*)"RD", 2 );	// +"RD".
				puc_tmpCommand += 2;
				ul_Length += 2;
				
				// +" A1 B0" トータルカラーカウンタを算出するコマンド群を付与.
				memcpy( puc_tmpCommand, " A1 B0", 6 );
				puc_tmpCommand += 6;
				ul_Length += 6;

				// +" OC PC FC" トータルカラーカウンタを算出するコマンド群を付与.
				memcpy( puc_tmpCommand, SendCommandTotalFCCntStr, ( sizeof( SendCommandTotalFCCntStr ) - 1 ) );
				puc_tmpCommand += ( sizeof( SendCommandTotalFCCntStr ) - 1 );
				ul_Length += sizeof( SendCommandTotalFCCntStr ) - 1;
				
				// 統合第4世代 片方向通信時の時刻生成機能 2009/1/22 sone START.
				//E-mail/HTTP片方向、かつ初期発信またはメンテナンス終了発信または期日指定発信の場合.
				if( ((RDSE_TSK_ID_MIO  == theRdsApl.isRdsSystem() && True == theRdsApl.isSimplex()) ||
					 (RDSE_TSK_ID_HTTP == theRdsApl.isRdsSystem() && True != pc_RdsSetting->getCsrcCommunicationMethod())) && 
				      ((True == tskRdsEvent.isNowTrans( TYPE_RdsTrgKindMnteEnd )) ||
				      (True == tskRdsEvent.isNowTrans( TYPE_RdsTrgKindIni )) ||
				      (True == tskRdsEvent.isNowTrans( TYPE_RdsTrgKindTimeOut ))) )
				{
					memcpy( puc_tmpCommand, (uchar*)" A6", 3 );
					puc_tmpCommand += 3;
					ul_Length += 3;
				}
				// 統合第4世代 片方向通信時の時刻生成機能 2009/1/22 sone END.

				// 警告発信の場合.
				if( True == tskRdsEvent.isNowTrans( TYPE_RdsTrgKindWarnBody ) )
				{
					// JAM多発警告発信の場合 かつ DipSwがONの場合.
					//if( ( ( TYPE_RdsWarnStatNormal != pc_RdsSetting->getRdsMCBJStat() ) ||			// ( 用紙系JAM多発警告発生状態 or.
					//	  ( TYPE_RdsWarnStatNormal != pc_RdsSetting->getRdsMOBJStat() ) ) &&		//   原稿系JAM多発警告発生状態 ) かつ.
					//	( 0 != ( pc_RdsSetting->getDipSw21() & RDSD_BIT3 ) ) )						// 100JAM履歴付加フラグ(DipSW21のBIT3)がONの場合.
					if( c_mkText_ModeWarn.b_JAMFrequencyFlag )
					{
						memcpy( puc_tmpCommand, (uchar*)" Z0", 3 );	// +" Z0" 100JAM履歴情報付加.
						puc_tmpCommand += 3;
						ul_Length += 3;
					}
				}
			
				// 緊急発信の場合.
				if( True == tskRdsEvent.isNowTrans( TYPE_RdsTrgKindTrbl ) )
				{
					memcpy( puc_tmpCommand, (uchar*)" S7", 3 );	// +" S7" FWバージョン情報付加.
					puc_tmpCommand += 3;
					ul_Length += 3;
				}
				
				// E-mail/HTTP片方向通信時の期日指定発信コマンド付与.
				if( (RDSE_TSK_ID_MIO  == theRdsApl.isRdsSystem() && True == theRdsApl.isSimplex()) ||						// E-mail片方向通信の場合 
					(RDSE_TSK_ID_HTTP == theRdsApl.isRdsSystem() && True != pc_RdsSetting->getCsrcCommunicationMethod()) )	// もしくは、HTTP片方向通信の場合 
				{	
					// 期日指定発信のトリガがかかっている場合(現在の通信が期日指定発信でなくとも、トリガがかかっていればコマンドを付与する).
					if( ( True == theTrgMap.b_transMap[1][ TYPE_RdsTrgKindTimeOut ] ) )
					{			
						// 定期発信の送信項目取得 
						us_CsrcCenterSendSetting = pc_RdsSetting->getCsrcCenterSendSetting();
						us_CsrcCenterSendSetting = us_CsrcCenterSendSetting & 0xFFFF;
						us_CsrcCenterSendSettingDebug = us_CsrcCenterSendSetting;
						
						// 定期発信の送信項目がある場合.
						if( us_CsrcCenterSendSetting )
						{
// 2019/07/23 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 DEL Start.
//// 2014/07/11 ktakahashi ZeusS 制御構成変更No.1 マシンタイプ追加(WP65) Start.
//							// 機能バージョン取得.
//							//TYPE_FuncVer e_FuncVer = APIC_SystemInfo::newInstance()->getFuncVer2();	//[AR : 1029580] 【 Eagle 】【Build error fix】 2018/03/09 [HCL_Raghu]
//// 2018/09/05 FPT-TruongNV6 【IT6_1.0】【Sparrow】 AR1052454 Modification Start.
//							// コマンド文字列のポインタをテンポラリの配列にコピーする.
//							const char *cpcSendCommandStr[_S_Command_MAX_LEN] = { NULL };
//// comment 2018/10/10 SCD2-Sky東野 [IT6] [AR1062320] 不具合対応 Start.
//							APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
//							pc_SystemInfo->lock();
//							TYPE_MachineType e_Type = pc_SystemInfo->getMachineType();
//							pc_SystemInfo->unlock();
//
//							// 機能バージョン1.0.
//							for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
//							{
//								if(	(TYPE_MachineTypeSparrow1SFP == e_Type) ||
//									(TYPE_MachineTypeSparrow2SFP == e_Type) )
//								{
//									cpcSendCommandStr[ ss_i ] = SendCommandStr_SparrowSFP[ ss_i ];
//								}else{
//									cpcSendCommandStr[ ss_i ] = SendCommandStr[ ss_i ];
//								}
//							}
//// comment 2018/10/10 SCD2-Sky東野 [IT6] [AR1062320] 不具合対応 End.
//							// 機能バージョン1.0.
//// 2018/09/26 FPT-LucNH2 【 IT6_2.0 】【EagleH】【制御構成変更_No.4】 Start.
//							TYPE_FuncVer e_FuncVer = APIC_SystemInfo::newInstance()->getFuncVer2();
//							if( TYPE_FuncVer_Expansion1 <= e_FuncVer )		// 機能バージョン2.0
//							{
//								for( short ss_i = 0; ss_i < 10; ss_i ++ )
//								{
//									cpcSendCommandStr[ ss_i ] = SendCommandStr_Exp_01[ ss_i ];
//								}
//							}
//							else										// 機能バージョン1.0.
//							{
//								for( short ss_i = 0; ss_i < 10; ss_i ++ )
//								{
//									cpcSendCommandStr[ ss_i ] = SendCommandStr[ ss_i ];
//								}
//							}
//// 2018/09/26 FPT-LucNH2 【 IT6_2.0 】【EagleH】【制御構成変更_No.4】 End.
//// 2018/09/05 FPT-TruongNV6 【IT6_1.0】【Sparrow】 AR1052454 Modification End.
// 2019/06/21 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 DEL End.
// 2019/06/21 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 MOD Start.
							// コマンド文字列のポインタをテンポラリの配列にコピーする.
							const char *cpcSendCommandStr[_S_Command_MAX_LEN] = { NULL };
							// マシンタイプ、機能バージョン取得.
							APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
							pc_SystemInfo->lock();
							TYPE_MachineType e_Type = pc_SystemInfo->getMachineType();
							TYPE_FuncVer e_FuncVer = pc_SystemInfo->getFuncVer2();
							pc_SystemInfo->unlock();

//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 Start.
							if( TYPE_FuncVer_Expansion4 <= e_FuncVer )				// 機能バージョン2.3.
							{
								if( (TYPE_MachineTypeSparrow1SFP == e_Type) ||
									(TYPE_MachineTypeSparrow2SFP == e_Type) )
								{
									for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr_SparrowSFP_Exp_04[ ss_i ];
									}
								}
								else if( TYPE_MachineTypeSparrowBK0SFP == e_Type )
								{
									for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr_SparrowBkSFP_Exp_04[ ss_i ];
									}
								}
								else if( (TYPE_MachineTypeHeliosMLK3_CN == e_Type) ||
										 (TYPE_MachineTypeHeliosMLK2_5_CN == e_Type) ||
										 (TYPE_MachineTypeHeliosMLK2_5_CNDF == e_Type) ||
										 (TYPE_MachineTypeHeliosMLK2_CNDF == e_Type) )
								{
									for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr_HeliosMLK_CN_Exp_04[ ss_i ];
									}
								}
								else if( (TYPE_MachineTypeMinervaSSBKX == e_Type) ||
										 (TYPE_MachineTypeMinervaSSBK0_US == e_Type) ||
										 (TYPE_MachineTypeMinervaSSBK0_EU == e_Type) )
								{
									for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr_MinervaSSBK_Exp_04[ ss_i ];
									}
								}
								else
								{
									for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr_Exp_04[ ss_i ];
									}
								}
							}
//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 End.
//2020/12/22 FPT-HaDV8 【 IT6_2.2 】 制御構成変更_No.4 Start.
							else if( TYPE_FuncVer_Expansion3 <= e_FuncVer )				// 機能バージョン2.2.
							{
								if( (TYPE_MachineTypeSparrow1SFP == e_Type) ||
									(TYPE_MachineTypeSparrow2SFP == e_Type) )
								{
									for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr_SparrowSFP_Exp_03[ ss_i ];
									}
								}
								else if( TYPE_MachineTypeSparrowBK0SFP == e_Type )
								{
									for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr_SparrowBkSFP_Exp_03[ ss_i ];
									}
								}
								else if( (TYPE_MachineTypeHeliosMLK3_CN == e_Type) ||
										 (TYPE_MachineTypeHeliosMLK2_5_CN == e_Type) ||
										 (TYPE_MachineTypeHeliosMLK2_5_CNDF == e_Type) ||
										 (TYPE_MachineTypeHeliosMLK2_CNDF == e_Type) )
								{
									for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr_HeliosMLK_CN_Exp_03[ ss_i ];
									}
								}
//2021/03/09 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 制御構成変更_No.4 Start.
								else if( (TYPE_MachineTypeMinervaSSBKX == e_Type) ||
										 (TYPE_MachineTypeMinervaSSBK0_US == e_Type) ||
										 (TYPE_MachineTypeMinervaSSBK0_EU == e_Type) )
								{
									for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr_MinervaSSBK_Exp_03[ ss_i ];
									}
								}
//2021/03/09 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 制御構成変更_No.4 End.
								else
								{
									for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr_Exp_03[ ss_i ];
									}
								}
							}
//2020/12/22 FPT-HaDV8 【 IT6_2.2 】 制御構成変更_No.4 End.
//2020/07/01 FPT-TinhNT16 【 IT6_2.1 】 制御構成変更_No.4 start.
							else if( TYPE_FuncVer_Expansion2 <= e_FuncVer )				// 機能バージョン2.1.
							{
								if( (TYPE_MachineTypeSparrow1SFP == e_Type) ||
									(TYPE_MachineTypeSparrow2SFP == e_Type) )
								{
									for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr_SparrowSFP_Exp_02[ ss_i ];
									}
								}
								else if( TYPE_MachineTypeSparrowBK0SFP == e_Type )
								{
									for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr_SparrowBkSFP_Exp_02[ ss_i ];
									}
								}
								else if( (TYPE_MachineTypeHeliosMLK3_CN == e_Type) ||
										 (TYPE_MachineTypeHeliosMLK2_5_CN == e_Type) ||
										 (TYPE_MachineTypeHeliosMLK2_5_CNDF == e_Type) ||
										 (TYPE_MachineTypeHeliosMLK2_CNDF == e_Type) )
								{
									for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr_HeliosMLK_CN_Exp_02[ ss_i ];
									}
								}
								else
								{
									for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr_Exp_02[ ss_i ];
									}
								}
							}
//2020/07/01 FPT-TinhNT16 【 IT6_2.1 】 制御構成変更_No.4 end.
							else if( TYPE_FuncVer_Expansion1 <= e_FuncVer )				// 機能バージョン2.0.
							{
								if( (TYPE_MachineTypeSparrow1SFP == e_Type) ||
									(TYPE_MachineTypeSparrow2SFP == e_Type) )
								{
									for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr_SparrowSFP_Exp_01[ ss_i ];
									}
								}
								else if( TYPE_MachineTypeSparrowBK0SFP == e_Type )
								{
									for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr_SparrowBkSFP[ ss_i ];
									}
								}
// 2019/09/17 【HeliosMLK】【Control No.1】【HELIOSMLK-73】 HIENG Start.
								else if( (TYPE_MachineTypeHeliosMLK3_CN == e_Type) ||
										 (TYPE_MachineTypeHeliosMLK2_5_CN == e_Type) ||
// 2019/12/19 【HeliosMLK】【Control No.52】【HELIOSMLK-1204】 HIENG Start.
										 (TYPE_MachineTypeHeliosMLK2_5_CNDF == e_Type) ||
										 (TYPE_MachineTypeHeliosMLK2_CNDF == e_Type) )
// 2019/12/19 【HeliosMLK】【Control No.52】【HELIOSMLK-1204】 HIENG End.
								{
									for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr_HeliosMLK_CN[ ss_i ];
									}
								}
// 2019/09/17 【HeliosMLK】【Control No.1】【HELIOSMLK-73】 HIENG End.
								else
								{
// 2019/08/27 SparrowBK OP_BTS-12328 機能毎コマンド文字列作成用テーブル不備対応 日立 江上 MOD Start.
									for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
// 2019/08/27 SparrowBK OP_BTS-12328 機能毎コマンド文字列作成用テーブル不備対応 日立 江上 MOD End.
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr_Exp_01[ ss_i ];
									}
								}
							}
							else													// 機能バージョン1.0.
							{
								for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
								{
									if( (TYPE_MachineTypeSparrow1SFP == e_Type) ||
										(TYPE_MachineTypeSparrow2SFP == e_Type) )
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr_SparrowSFP[ ss_i ];
									}
									else
									{
										cpcSendCommandStr[ ss_i ] = SendCommandStr[ ss_i ];
									}
								}
							}
// 2019/07/23 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 MOD End.

							//項目【1】(Sales count data)が選択されていた場合.
							if( us_CsrcCenterSendSetting & 0x0001 )
							{
								memcpy( puc_tmpCommand, cpcSendCommandStr[0], strlen( cpcSendCommandStr[0] ) );
								puc_tmpCommand += strlen( cpcSendCommandStr[0] );
								ul_Length += strlen( cpcSendCommandStr[0] );
							}
							
							//項目【2】(Error count data)が選択されていた場合.
							if( us_CsrcCenterSendSetting & 0x0002 )
							{
								memcpy( puc_tmpCommand, cpcSendCommandStr[1], strlen( cpcSendCommandStr[1] ) );
								puc_tmpCommand += strlen( cpcSendCommandStr[1] );
								ul_Length += strlen( cpcSendCommandStr[1] );
							}
							//項目【3】(Service count datar)が選択されていた場合.
							if( us_CsrcCenterSendSetting & 0x0004 )
							{
								memcpy( puc_tmpCommand, cpcSendCommandStr[2], strlen( cpcSendCommandStr[2] ) );
								puc_tmpCommand += strlen( cpcSendCommandStr[2] );
								ul_Length += strlen( cpcSendCommandStr[2] );
							}
							
							//項目【4】(Life count data Block, Life cycle data Block)が選択されていた場合.
							if( us_CsrcCenterSendSetting & 0x0008 )
							{
								memcpy( puc_tmpCommand, cpcSendCommandStr[3], strlen( cpcSendCommandStr[3] ) );
								puc_tmpCommand += strlen( cpcSendCommandStr[3] );
								ul_Length += strlen( cpcSendCommandStr[3] );
							}
							
							//項目【5】(CSRD-System data Block, Device config data)が選択されていた場合.
							if( us_CsrcCenterSendSetting & 0x0010 )
							{
								memcpy( puc_tmpCommand, cpcSendCommandStr[4], strlen( cpcSendCommandStr[4] ) );
								puc_tmpCommand += strlen( cpcSendCommandStr[4] );
								ul_Length += strlen( cpcSendCommandStr[4] );
							}
							
							//項目【6】(History data)が選択されていた場合.
							if( us_CsrcCenterSendSetting & 0x0020 )
							{
								memcpy( puc_tmpCommand, cpcSendCommandStr[5], strlen( cpcSendCommandStr[5] ) );
								puc_tmpCommand += strlen( cpcSendCommandStr[5] );
								ul_Length += strlen( cpcSendCommandStr[5] );
							}
							
							//項目【7】(EKC data Block)が選択されていた場合.
							//セキュリティ強化モードON時、DXコマンドはセンターへ通知できないため、
							//セキュリティ強化モードON、かつセンターへの通知項目にDXコマンドが含まれていた場合、発信を中止する。
							if(us_CsrcCenterSendSetting & 0x0040) 
							{
								if( True == isEnableStatus_ReadDX() )	// 2011/06/30 yito CQ349199 期日指定発信でのDX取得.
								{
									memcpy( puc_tmpCommand, cpcSendCommandStr[6], strlen( cpcSendCommandStr[6] ) );
									puc_tmpCommand += strlen( cpcSendCommandStr[6] );
									ul_Length += strlen( cpcSendCommandStr[6] );
								}
								else
								{
									// 2011/06/30 yito CQ349199 期日指定発信でのDX取得 start.
									EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"DXコマンド送信条件に満たないため、DXコマンドの要求取り消し" );
									// 2011/06/30 yito CQ349199 期日指定発信でのDX取得 end  .
								}
							}

// 2019/10/21 y.hotta OP_BTS-14654 片方向通信時、通知項目に「7」を選択してないのに「DX」が取得される Del Start.
//// 2018/07/03 HCLT-Vinay St.
//							if(us_CsrcCenterSendSetting & 0x0020) // comment 2018/10/10 SCD2-Sky東野 [IT6] [AR1062320] 不具合対応 End.
//							{
//								if( True == isEnableStatus_ReadZP() )
//								{
//									memcpy( puc_tmpCommand, cpcSendCommandStr[6], strlen( cpcSendCommandStr[6] ) );
//									puc_tmpCommand += strlen( cpcSendCommandStr[6] );
//									ul_Length += strlen( cpcSendCommandStr[6] );
//								}
//								else
//								{
//									EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"ZPコマンド送信条件に満たないため、ZPコマンドの要求取り消し" );
//								}
//							}
//// 2018/07/03 HCLT-Vinay End.							
// 2019/10/21 y.hotta OP_BTS-14654 片方向通信時、通知項目に「7」を選択してないのに「DX」が取得される Del End.
							
							//項目【8】(Adjustment data Block)が選択されていた場合.
							if( us_CsrcCenterSendSetting & 0x0080 )
							{
								memcpy( puc_tmpCommand, cpcSendCommandStr[7], strlen( cpcSendCommandStr[7] ) );
								puc_tmpCommand += strlen( cpcSendCommandStr[7] );
								ul_Length += strlen( cpcSendCommandStr[7] );
							}
							
							//項目【9】(カバレッジ data Block)が選択されていた場合.
							if( us_CsrcCenterSendSetting & 0x0100 )
							{
								memcpy( puc_tmpCommand, cpcSendCommandStr[8], strlen( cpcSendCommandStr[8] ) );
								puc_tmpCommand += strlen( cpcSendCommandStr[8] );
								ul_Length += strlen( cpcSendCommandStr[8] );
							}
							//項目【10】(Component Information data Block)が選択されていた場合.
							if( us_CsrcCenterSendSetting & 0x0200 )
							{
								memcpy( puc_tmpCommand, cpcSendCommandStr[9], strlen( cpcSendCommandStr[9] ) );
								puc_tmpCommand += strlen( cpcSendCommandStr[9] );
								ul_Length += strlen( cpcSendCommandStr[9] );
							}
// 2018/09/05 FPT-TruongNV6 【IT6_1.0】【Sparrow】 AR1052454 AddNew Start.
							//項目【11】(Protocol Trace Data)が選択されていた場合.
							if (us_CsrcCenterSendSetting & 0x0400)
							{
								memcpy(puc_tmpCommand, cpcSendCommandStr[10], strlen(cpcSendCommandStr[10]));
								puc_tmpCommand += strlen(cpcSendCommandStr[10]);
								ul_Length += strlen(cpcSendCommandStr[10]);
							}
// 2018/09/05 FPT-TruongNV6 【IT6_1.0】【Sparrow】 AR1052454 AddNew Start.
// 2014/07/11 ktakahashi ZeusS 制御構成変更No.1 マシンタイプ追加(WP65) End.
						}
					}
				}

// 2010/10/08 higuchi Minerva UV-00966 【機種コード取り込み】 Start.
				// 初期発信(全通信)／期日指定発信(片方向通信用のみ)時、S9コマンド付加.
				if(	True == tskRdsEvent.isNowTrans( TYPE_RdsTrgKindIni )  ||																// 初期発信.
					(	True == theTrgMap.b_transMap[1][ TYPE_RdsTrgKindTimeOut ] &&														// 期日指定発信.
						(	( RDSE_TSK_ID_MIO  == theRdsApl.isRdsSystem() && True == theRdsApl.isSimplex() ) ||								// E-mail片方向通信.
							( RDSE_TSK_ID_HTTP == theRdsApl.isRdsSystem() && True != pc_RdsSetting->getCsrcCommunicationMethod() ) ) )		// HTTP片方向通信.
				  )
				{
					memcpy( puc_tmpCommand, (uchar*)" S9", 3 );	// +" S9" 機種コード情報付加.
					puc_tmpCommand += 3;
					ul_Length += 3;
				}
// 2010/10/08 higuchi Minerva UV-00966 【機種コード取り込み】 End.

				// コマンドの文字列長をセット.
				sprintf( &sc_Length[0], "%06X", ul_Length );
				memcpy( &uc_TmpHostCmd[3], &sc_Length[0], strlen( sc_Length ) );
				
				// エラーコード(エラーなし)、RD受信状態をセット.
				e_cmdResult.ul_ErrCode = 0;						// エラーなし 
				e_cmdResult.ul_TypeCode = RDSD_ResType_ZR;		// RDを受信した状態 
			}
			else
			{
				// ※ 戻り値で RD と、WR の応答による添付ファイル数を得る。 
				e_cmdResult.ul_ErrCode = pc_RdsSetting->getEmailResultErrCode();
				e_cmdResult.ul_TypeCode = pc_RdsSetting->getEmailResultTypeCode();
			}
			
			// コマンドの付与があった、もしくは応答発信の場合、analizeHostCommandを実行.
			if ( ( True == b_Trgflg ) ||
			     ( True == theTrgMap.isExistOnGoingTrg( TYPE_RdsTrgKindRes_RDS ) ) )
			{
				if(    (0 == e_cmdResult.ul_ErrCode)						// エラーが発生していない場合  
					&& ((e_cmdResult.ul_TypeCode & RDSD_ResType_ZR) || (e_cmdResult.ul_TypeCode & RDSD_ResType_ZW))   )		// RD、WR を受信していた場合 
				{
					//EPN_DBG_MESSAGE((uchar*)"DBG ホストコマンド文字列", (uint32_t)0, (uchar*)uc_TmpHostCmd);
					EPN_DBG_PRINTF_DIV( "[CommMIO::tx_transSq]DBG ホストコマンド文字列 %s", uc_TmpHostCmd );
					
					theRdsEmail.analizeHostCommand(
						RDSE_Email_CommModeRds,
						RDSE_AnaMode_OnlyRD,					// モード：RDコマンドのみ処理する。 
						uc_TmpHostCmd,										// ホストコマンド文字列 
						&e_cmdResult,										// 処置結果 
						&q_RD_Data,											// ReadCommand() にて読み込んだデータ 
						&ss_NumOfCommandResFiles,							// RD による応答ファイル数 
						&q_SendInfo											// 上記のファイルごとのファイルサイズ 
					);
					// センターから受信したコマンドのチェックは、E-mail受信完了時に行われている。 
					// よって、下記処理は不要。 ⇒ 【下記処理は行ってはいけない。】 
					//pc_RdsSetting->setEmailResultTypeCode( e_cmdResult.ul_TypeCode );
					//pc_RdsSetting->setEmailResultErrCode( e_cmdResult.ul_ErrCode );
		
					EPN_DBG_MESSAGE( (uchar*)"tx_transSq()", (ushort)e_cmdResult.ul_ErrCode, (uchar*)" <<< e_cmdResult.ul_ErrCode" );
				}
			}
			
			
			ss_NumOfEventDataFiles = c_EvevntDataBuf.isNum();							// イベントデータファイル数 
			ss_NumOfAllFiles = (short)(ss_NumOfCommandResFiles + ss_NumOfEventDataFiles);	// 2010/07/17 sone Minerva warning 対応 shortにキャスト.
			
			EPN_DBG_MESSAGE((uchar*)"イベントファイル数", (uint32_t)ss_NumOfEventDataFiles, (uchar*)"");
			EPN_DBG_MESSAGE((uchar*)"RDファイル数", (uint32_t)ss_NumOfCommandResFiles, (uchar*)"");
			
			break;
		}

		theRdsEmail.makeEmailData( e_NowCommMode );										// E-mailのヘッダーデータ作成、応答の確認で使用するデータ作成
	
		if( RDSE_Email_CommModeTestMail != e_NowCommMode )								// 通常のKMRDS通信なら
		{
			if( ss_NumOfAllFiles <= 0 )
			{
				stopComm( MLog_Nothing, RDSE_ERR_MIO_ILLEGAL_STATE_ERR_1, RDSE_ERR_MIO_ILLEGAL_STATE_ERR_1 );	// シーケンス破棄（１分後再開）
				return;
			}
		}
	}
		
	/*~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~~*/

	result = False;																	// 失敗で初期化 
	// （１）E-mail情報作成 
	if ( False != tx_makeMioSendInfoData
					(
						&sendinfo,
						&emset,
						&AddrTo,
						&AddrCc,
						&AddrBcc,
						&pBodyHeader,
						ss_NumOfEventDataFiles,
						ss_NumOfCommandResFiles,
						&q_SendInfo
					)
		)
	{
		if( ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )	// HTTP通信の場合 08/02/04 寺内 新規追加 
		 || ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ))	// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
		{
					
			// HB排他制御の為に追加
			if( c_HBdetect.b_TransHB != True )
			{
				// （４）mioSendData() テキスト送信 
				if( False != tx_transText( fd, ss_NumOfEventDataFiles, ss_NumOfCommandResFiles ) )
				{
					result = True;													// 送信終了 
				}
				else
				{
					EPN_DBG_MESSAGE( (uchar*)"CSRC", 0, (uchar*)"＜テキスト送信に失敗＞" );
					result = False;
				}
				
				/* E-mail版フォーマットファイル作成 */
				e_WebDAVClient.PutHeader( &emset );
			}
			else
			{
				result = True;													// HBは無条件で送信終了 
			}
				
		//	e_Webdavtask.reqSendStart();
// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化 START.
			if( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice ){
				// HTTP通信処理部（自タスク）へ「送信開始」MSG送信 
				e_Webdavtask.reqNicMsgStart();
			}
			else{
				RDSC_OAPTask::instance().indOapDevReq();
// 2017/07/20 Sky MAv5 1026004_DCA通信時に自動再発信が行われない Start.
		    	transState = MIO_Trans_WaitTransEndMSG_State;					// 送信終了通知待ち 
				return;															// OAP通信時はデータを作成するだけなのでここで処理を抜ける.
// 2017/07/20 Sky MAv5 1026004_DCA通信時に自動再発信が行われない End.
			}
// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化 END.
		}
		else				// E-mail 
		{
			// （２）mioOpen() 
			//fd = epMioOpen( (char*)"/mio/se/9", O_WRONLY | O_NDELAY );... 現状、O_NDELAY は未対応！ 
#ifdef ROM_DIV_3	/* marumoto 2006.3.27 NICFUM ROM_DIV */
			if( pt_mioOpen != NULL )
			{
				fd = ((int(*)(char *,int))pt_mioOpen)( (char*)"/mio/se/9", O_WRONLY );
			}
			else
			{
				fd = ERROR;
				EPN_DBG_MESSAGE( (uchar*)"CSRC", 0, (uchar*)" RDSC_RdsCommMIO::tx_transSq pt_mioOpen NULL pointer." );
			}
#else
			fd = epMioOpen( (char*)"/mio/se/9", O_WRONLY );
#endif
			if ( ERROR != fd )
			{
				EPN_DBG_MESSAGE( (uchar*)"CSRC", fd, (uchar*)" ＜mioOpen() ---> 送信処理 Ｇｏ！＞" );
				transState = MIO_Trans_txText_State;									// テキスト送信 

				// （３）mioSetSendInfo() see 「Apendix C」
				dbg_PrintMioSendEmail( &sendinfo );										// for Debug 
#ifdef ROM_DIV_3	/* marumoto 2006.3.27 NICFUM ROM_DIV */
				if( pt_mioSetSendInfo != NULL )
				{
					sendInfoAns = ((STATUS(*)(MioFileDesc,MIO_SEND_TYPE,void *))pt_mioSetSendInfo)((MioFileDesc)fd, SEND_TYPE_EMAIL, (MioSendEmail *)&sendinfo);
				}
				else
				{
					sendInfoAns = ERROR;
					EPN_DBG_MESSAGE( (uchar*)"CSRC", 0, (uchar*)" RDSC_RdsCommMIO::tx_transSq pt_mioSetSendInfo NULL pointer." );
				}
#else
				sendInfoAns = epMioSetSendInfo((MioFileDesc)fd, SEND_TYPE_EMAIL, (MioSendEmail *)&sendinfo);
#endif
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	Start.
				// 監査ログに保存.
				addOperationLogSession( sendInfoAns );
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	End.

				if ( OK == sendInfoAns )
				{
					// （４）mioSendData() テキスト送信 
					if( False != tx_transText( fd, ss_NumOfEventDataFiles, ss_NumOfCommandResFiles ) )
					{
						result = True;													// 送信終了 
					}
					else
					{
						EPN_DBG_MESSAGE( (uchar*)"CSRC", 0, (uchar*)"＜テキスト送信に失敗＞" );
						result = False;
					}
				}
				else
				{
					EPN_DBG_MESSAGE( (uchar*)"CSRC", sendInfoAns, (uchar*)"＜mioSetSendInfo() 失敗＞" );
					result = False;
				}
			}
			else
			{
				EPN_DBG_MESSAGE( (uchar*)"CSRC", fd, (uchar*)"＜mioOpen() 失敗＞" );
				result = False;
			}
		}
	}
	else
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", 0, (uchar*)"＜E-mail情報作成に失敗＞" );
		result = False;
	}


	// 送信終了処理 ----------------------------------------------------- 
	if( True == result )
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", fd, (uchar*)"＜MIO に送信終了->TransComplete 待ち＞" );
    //2012/03/08 sone CQNo362757対応 HTTP通信時は30分のタイマーをセットするStart.
		if( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
		{
			EPN_DBG_MESSAGE( (uchar*)"tx_transSq", 0, (uchar*)"SetTimer起動(HTTP用)" );			
	//2021/04/19 FPT-ThanhPC7 TAS000000003019 modify wait time 30 -> 15 minutes Start.
			setTimer( 15*RDSD_Minute );	// 30分待ってみる(HTTP通信の最大遅延は300秒だが、フェールセーフとして30分にする). 
	//2021/04/19 FPT-ThanhPC7 TAS000000003019 modify wait time 30 -> 15 minutes End.
		}
		else
		{
			EPN_DBG_MESSAGE( (uchar*)"tx_transSq", 0, (uchar*)"SetTimer起動(HTTP以外用)" );			
			setTimer( 2*RDSD_Minute );		// ２分待ってみる。 
		}
    //2012/03/08 sone CQNo362757対応 HTTP通信時は30分のタイマーをセットするEnd.
    	transState = MIO_Trans_WaitTransEndMSG_State;							// 送信終了通知待ち 
	}
	else
	{
		if( MIO_Trans_WaitAnswer_State == transState )
		{
			setTimer( 2*RDSD_Minute );											// ２分待ってみる 
		}
		else
		{
			// 02/04/03 浦崎  接続タイムアウト時の処理をコーディング 
			switch( sendInfoAns ){
#ifndef ENOTCONN
  #define ENOTCONN 57			/* 57  Socket is not connected（サーバ側のLANケーブルが抜けている）	*/
#endif
#ifndef ETIMEDOUT
  #define ETIMEDOUT 60			/* 60  Connection timed out（接続タイムアウト）						*/
#endif
#ifndef ENETDOWN
  #define ENETDOWN 62			/* 62  Network is down（複写機側のLANケーブルが抜けている）			*/
#endif 

				case ENOTCONN :			/* 57  Socket is not connected（サーバ側のLANケーブルが抜けている）	*/
				case ETIMEDOUT :		/* 60  Connection timed out（接続タイムアウト）						*/
				case ENETDOWN :			/* 62  Network is down（複写機側のLANケーブルが抜けている）			*/
					theVRcontroller.c_MailLog.setLog(							// 通信ログをセット 
						&s_tmpMioHeader.Adr.From[0],									// From 
						&theRdsEmail.s_TxEmailHeader.adr[0],							// To 
						&theRdsEmail.s_TxEmailHeader.date,								// Date 
						&theRdsEmail.s_TxEmailHeader.subject[0],						// Subject 
						NULL,															// Received 
						MLog_Tx_ConnectionTimeOut,										// （結果） 
						(ushort)sendInfoAns												/* エラーコード */
					);
					stopComm( MLog_Tx_ConnectionTimeOut, RDSE_ERR_MIO_ServerConnectErr, (ushort)sendInfoAns );		// シーケンス破棄（１時間後） 
// 2015/11/06 tatsumi ZeusS_ZX0 制御構成変更_No.76-2_ネットワークエラーカウンタ追加 START.
					RDS_incNetworkErrorCount( RDSE_NETWORK_ERROR_SEQUENCE_NETWORK );
// 2015/11/06 tatsumi ZeusS_ZX0 制御構成変更_No.76-2_ネットワークエラーカウンタ追加 END.
					break;
				default :
					stopComm( MLog_Nothing2, RDSE_ERR_MIO_TransCompleteERR, (ushort)sendInfoAns );					// シーケンス破棄（１分後再開） 
					break;
			}
		}
	}

	// mioClose() ----------------------------------------------------- 
	if ( ERROR != fd )
	{	// mioOpen() が成功していた場合、必ずクローズするようにする
#ifdef ROM_DIV_3	/* marumoto 2006.3.27 NICFUM ROM_DIV */
		if( pt_mioClose != NULL )
		{
			((int(*)(MioFileDesc))pt_mioClose)(fd);
		}
		else
		{
			EPN_DBG_MESSAGE( (uchar*)"CSRC", 0, (uchar*)" RDSC_RdsCommMIO::tx_transSq pt_mioClose NULL pointer." );
		}
#else
		epMioClose(fd);
#endif
	}

	// バッファ開放 ----------------------------------------------------- 
	freeMIMEBodyHeader( emset.pMIMEBodyHeader );	// pBodyHeader 
}
/*==================================================================================================
//	MioSendInfo() で送信するためのデータを作成する
//		この関数は、「theRdsEmail.makeEmailData()」をコールしてからコールする必要がある
//--------------------------------------------------------------------------------------------------
// 02/05/24 浦崎  NU 時は、暗号化しないようにした(Content-Type に「type=dat」をつけない)
// 02/05/27 浦崎  Fiery 版と処理を合わせるため、NU 時も暗号化する（よって、上記処理を削除）。
// 02/06/26 浦崎  暗号化キーが登録されている時のファイルサイズに関して、暗号化によるデータ増加分を
//				  考慮していなかったため、修正。
// 03/02/26 浦崎  MioSendEmail データ作成時に、AppID をセットしていなかった問題を修正。
// 05/10/06 浦崎 [CSRC nicfum 4.1 対応] Cc と Bcc に NULL をセットしないと mioSetSendInfo() で 
//               エラーになるようになったため、NULL をセットするようにした。 
// 06/06/29 佐久間　NICFUM5.0(Mosel用NICFUM)対応において、MioSendEmail構造体に
		　　SMTP認証のパラメータ(bEnableSMTPAuth)が追加された。そのため
		　　Mail送信データ作成時に認証の有無を設定するように対応した。
// 10/06/08 曽根　特ROM対応によりbEnableSMTPAuth(bool)がuiEnableSMTPAuth(UINT32)に変更されたことにより対応。
==================================================================================================*/
Bool RDSC_RdsCommMIO::tx_makeMioSendInfoData
(
	MioSendEmail*			pSendinfo,
	MioEmailSet*			pEmset,
	EmailAddrList*			pAddrTo,
	EmailAddrList*			pAddrCc,
	EmailAddrList*			pAddrBcc, 
	MioMIMEBodyHeader**		ppBodyHeader,
	short					ss_NumOfEventDataFiles,
	short					ss_NumOfCommandResFiles,
	RDSC_MIOSendInfoData*	q_SendInfo
)
{
	register int		indx;
	int					si_len;
	MioMIMEBodyHeader*	pLastHeader = NULL;
	RDSS_EventData*		pText;
	short				ss_NumOfAllFiles = (short)(ss_NumOfEventDataFiles + ss_NumOfCommandResFiles);	// 2010/07/17 sone Minerva warning 対応 shortにキャスト.

	// MioEmailSet データ作成 -----------------------------------------------------
	memset( pEmset, 0, sizeof(MioEmailSet) );

	/* SenderAddr */
	if( ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )	// HTTP通信の場合固定アドレスを使用する。 08/02/04 寺内　新規追加 
	||  ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		strcpy( (char*)pEmset->SenderAddr, "host@csrc.konicaminolta.com" );
	}
	else
	{
		strcpy( (char*)pEmset->SenderAddr, "" );
	}

	/* ContentType */
	strcpy( (char*)pEmset->ContentType, "Text/Plain" );

	/* FromAddr */
	if( ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )	// HTTP通信の場合固定アドレスを使用する。 08/02/04 寺内　新規追加
	||  ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		strcpy( (char*)pEmset->FromAddr, "device@csrc.konicaminolta.com" );
		strcpy( (char*)s_tmpMioHeader.Adr.From, "device@csrc.konicaminolta.com" );
	}
	else
	{
		pc_RdsSetting->getPOP3MailAdr( pEmset->FromAddr );
		pc_RdsSetting->getPOP3MailAdr( s_tmpMioHeader.Adr.From );
	}

	/* ToAddr */
	if( ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )	// HTTP通信の場合固定アドレスを使用する。 08/02/04 寺内　新規追加
	||  ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{
		strcpy( (char*)pAddrTo->EmailAddress, "host@csrc.konicaminolta.com" );
	}
	else
	{
		memcpy( (char*)pAddrTo->EmailAddress, &theRdsEmail.s_TxEmailHeader.adr[0], _S_Adr );
	}
	
	/* ReplyToAddr */
	strcpy( (char*)pEmset->ReplyToAddr, "" );

	/* SendDate */
	theRdsEmail.cnvDateToString( &theRdsEmail.s_TxEmailHeader.date, (uchar*)pEmset->SendDate );

	/* OriginalRecipient */
	strcpy( (char*)pEmset->OriginalRecipient, "" );

	/* Subject */
	memcpy( (char*)pEmset->MailSubject, theRdsEmail.s_TxEmailHeader.subject, _S_Subject );

	/* Mail Body */
	switch( e_NowCommMode )
	{
	case RDSE_Email_CommModeTestMail:		// テストメール通信 
		pEmset->pMailBody = (uchar*)testMail_Body;
		pEmset->MailBodyLen = strlen( (const char*)pEmset->pMailBody );
		break;

	case RDSE_Email_CommModeISW:			// InternetISW通信 
	case RDSE_Email_CommModeRds:			// CSRC通信 
	default:
		pEmset->pMailBody = NULL;
		pEmset->MailBodyLen = 0;
		break;
	}

	pEmset->MailCharSet = MAIL_US_ASCII;
	strcpy( (char*)pEmset->ContentDescription, "Remote Management System");
	pEmset->FileCount   = (ushort)ss_NumOfAllFiles;														// テキスト数 

	pAddrTo->pNextEmailAddr  = NULL;
	pEmset->pToAddr  = pAddrTo;

	/* pAddrCc */
    pEmset->pCcAddr  = 0;	// 05/10/06 浦崎 

    /* pAddrBcc */
    pEmset->pBccAddr = 0;	// 05/10/06 浦崎 

	//pEmset->pCcAddr  = pAddrCc;
	//pEmset->pBccAddr = pAddrBcc;

	if( ( RDSE_TSK_ID_HTTP != theRdsApl.e_CommDevice )	// HTTP通信の場合通さない。 08/02/04 寺内　新規追加 
	&&  ( RDSE_TSK_ID_OAP != theRdsApl.e_CommDevice ) )	// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	{


		// MioSendEmail データ作成 -----------------------------------------------------
		memset( pSendinfo, 0, sizeof(MioSendEmail) );
		pSendinfo->pEmailSet = pEmset;
		pSendinfo->EnableDSN = FALSE;
		pSendinfo->EnableMDN = FALSE;
		pSendinfo->JobID     = 0;
		pSendinfo->AppID     = APP_EPNET;		// 03/02/26 追加 浦崎 
	
		// 06/06/29 SMTP認証対応 佐久間 
	//#if defined(DEF_MoselCom)
	//#if defined(DEF_ThamesCus)
					// 機種固有 これは必要なコンパイルＳＷ
		if( (TYPE_SmtpCertificationSMTP == pc_RdsSetting->getSmtpCertification() )
			|| ( TYPE_SmtpCertificationBoth == pc_RdsSetting->getSmtpCertification()))
		{
			// MLK UV-00660対応 SMTP認証強度設定が管理者設定に追加されたため、参照先をDipSWから管理者設定に変更 sone 2012/1/6 START.
			// 以前対応していたDipSWの処理は不要なので、削除.
			APIC_MailSendSetting* pc_MailSendSetting		= APIC_MailSendSetting::newInstance();
			UINT32			ui_SmtpAuthSetting	= MIO_SMTP_AUTH_NONE;
			TYPE_SmtpCertificationMethod e_Method = TYPE_SmtpCertificationMethod_Kerberos;
			TYPE_SmtpCertificationMethodFunc e_Return = TYPE_SmtpCertificationMethodOn;
			
			// DIGEST-MD5認証使用する/しない.
			e_Method = TYPE_SmtpCertificationMethod_DIGESTMD5;
			pc_MailSendSetting->lock();
			e_Return = pc_MailSendSetting->getSmtpCertificationMethod( e_Method );
			pc_MailSendSetting->unlock();
			
			// 使用するの場合はNICFUMへ渡すパラメータに0x0008(MIO_SMTP_AUTH_DIGESTMD5)をORする。
			if (e_Return == TYPE_SmtpCertificationMethodOn)
			{
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"DIGEST-MD5 True.");
				ui_SmtpAuthSetting = ui_SmtpAuthSetting | MIO_SMTP_AUTH_DIGESTMD5;
			}
			
			// CRAM-MD5認証認証使用する/しない.
			e_Method = TYPE_SmtpCertificationMethod_CRAMMD5;
			pc_MailSendSetting->lock();
			e_Return = pc_MailSendSetting->getSmtpCertificationMethod( e_Method );
			pc_MailSendSetting->unlock();
			
			// 使用するの場合はNICFUMへ渡すパラメータに0x0004(MIO_SMTP_AUTH_CRAMMD5)をORする。
			if (e_Return == TYPE_SmtpCertificationMethodOn)
			{
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"CRAM-MD5 True.");
				ui_SmtpAuthSetting = ui_SmtpAuthSetting | MIO_SMTP_AUTH_CRAMMD5;
			}

			// LOGIN認証認証認証使用する/しない.
			e_Method = TYPE_SmtpCertificationMethod_LOGIN;
			pc_MailSendSetting->lock();
			e_Return = pc_MailSendSetting->getSmtpCertificationMethod( e_Method );
			pc_MailSendSetting->unlock();
			
			// 使用するの場合はNICFUMへ渡すパラメータに0x0002(MIO_SMTP_AUTH_LOGIN)をORする。
			if (e_Return == TYPE_SmtpCertificationMethodOn)
			{
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"LOGIN True.");
				ui_SmtpAuthSetting = ui_SmtpAuthSetting | MIO_SMTP_AUTH_LOGIN;
			}

			// PLAIN認証認証認証使用する/しない.
			e_Method = TYPE_SmtpCertificationMethod_PLAIN;
			pc_MailSendSetting->lock();
			e_Return = pc_MailSendSetting->getSmtpCertificationMethod( e_Method );
			pc_MailSendSetting->unlock();
			
			// 使用するの場合はNICFUMへ渡すパラメータに0x0001(MIO_SMTP_AUTH_PLAIN)をORする。
			if (e_Return == TYPE_SmtpCertificationMethodOn)
			{
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"PLAIN True.");
				ui_SmtpAuthSetting = ui_SmtpAuthSetting | MIO_SMTP_AUTH_PLAIN;
			}

			// Kerberos認証認証認証使用する/しない.
			e_Method = TYPE_SmtpCertificationMethod_Kerberos;
			pc_MailSendSetting->lock();
			e_Return = pc_MailSendSetting->getSmtpCertificationMethod( e_Method );
			pc_MailSendSetting->unlock();
			
			// 使用するの場合はNICFUMへ渡すパラメータに0x0020(MIO_SMTP_AUTH_GSSAPI)をORする。
			if (e_Return == TYPE_SmtpCertificationMethodOn)
			{
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"Kerberos True.");
				ui_SmtpAuthSetting = ui_SmtpAuthSetting | MIO_SMTP_AUTH_GSSAPI;
			}
			
			// NTLMv1認証認証認証使用する/しない.
			e_Method = TYPE_SmtpCertificationMethod_NTLMv1;
			pc_MailSendSetting->lock();
			e_Return = pc_MailSendSetting->getSmtpCertificationMethod( e_Method );
			pc_MailSendSetting->unlock();
			
			// 使用するの場合はNICFUMへ渡すパラメータに0x0010(MIO_SMTP_AUTH_NTLM)をORする。
			if (e_Return == TYPE_SmtpCertificationMethodOn)
			{
				EPN_DBG_MESSAGE((uchar*)"DBG", (uint32_t)0, (uchar*)"NTLMv1 True.");
				ui_SmtpAuthSetting = ui_SmtpAuthSetting | MIO_SMTP_AUTH_NTLM;
			}
			// MLK UV-00660対応 SMTP認証強度設定が管理者設定に追加されたため、参照先をDipSWから管理者設定に変更 sone 2012/1/6 END.
			
			char sc_DbgAuth[10];
			memset(sc_DbgAuth, 0x00, sizeof(sc_DbgAuth));
			sprintf(sc_DbgAuth, "%02x", ui_SmtpAuthSetting);
			
			EPN_DBG_MESSAGE((uchar*)"SmtpAuthSetting", (uint32_t)ui_SmtpAuthSetting, (uchar*)sc_DbgAuth);
			
			// FQ-CSOPS-0900312/0900420対策 ここまで.

			pSendinfo->uiEnableSMTPAuth = ui_SmtpAuthSetting;
			memset(pSendinfo->ucAccount, 0, sizeof(pSendinfo->ucAccount)	);	// NULL をセット 
			memset(pSendinfo->ucPassword, 0, sizeof(pSendinfo->ucPassword)	);	// NULL をセット 
			pSendinfo->pSvrAddr = NULL;
			pSendinfo->usSvrPort = NULL;
			pSendinfo->usTimeout = NULL;
			memset(pSendinfo->ucRealm, 0, sizeof(pSendinfo->ucRealm)	);	// NULL をセット 
		}
		else
		{
			pSendinfo->uiEnableSMTPAuth = MIO_SMTP_AUTH_NONE;
			// FQ-CSOPS-0900420のマージ対応 CQNo328012 sone 2010/06/08 END.
		}
	}
	// MioMIMEBodyHeader データ作成 -----------------------------------------------------
	*ppBodyHeader = NULL;
	pLastHeader = NULL;
	for( indx=0; indx<ss_NumOfAllFiles; indx++ ){
		(*ppBodyHeader) = (MioMIMEBodyHeader*)malloc( sizeof(MioMIMEBodyHeader) );
		if (NULL == (*ppBodyHeader)){
			return( False );
		}
		memset( *ppBodyHeader, 0, sizeof(MioMIMEBodyHeader) );
		strcpy( (char*)(*ppBodyHeader)->ContentType, "application/octet-stream" );


		if( ss_NumOfEventDataFiles )
		{
			// まずは、イベントデータバッファのデータから処理 
			pText = c_EvevntDataBuf.getRdPtr( indx );
			si_len = pText->us_Len;
			ss_NumOfEventDataFiles --;		// 04/03/10 浦崎　 
		}
		else
		{
			// RD コマンドを処理 
			uint32_t*	pul_RDFileNum;
			q_SendInfo->get( &pul_RDFileNum );
			si_len = 0;
			if( NULL != pul_RDFileNum )		// 念のため。 
			{
				si_len = *pul_RDFileNum;
				free( pul_RDFileNum );		// 04/06/23 浦崎 free 忘れがあった．．．ため、対策。 
			}
			ss_NumOfCommandResFiles--;

			// M500 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入らない。 
			uchar uc_DumyM5Buff[30];
			ushort us_Len = RDSC_MakeEventData::makeM500
				(
					uc_DumyM5Buff,			// ユーザーデータバッファ 
					(uchar*)"WW"			// 発信固有文字列 
				);

			si_len += us_Len;

		}
		// FrameNo と LEN のレングス分加算する。 
		si_len += _S_Frame_and_LEN;


// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/22 Sky梶谷 START.
//		//if( True == theVRcontroller.c_Cipher.isKeyExist() ){
//		if( ( ( True == theVRcontroller.c_Cipher.isKeyExist() ) && ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice ) ) ||					// E-mailでかつ暗号化ON. 
//			( ( True == pc_RdsSetting->getCsrcTextEncryptionOn() ) && ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice ) ) ||			// HTTPでかつ暗号化ON.
//			( ( True == pc_RdsSetting->getCsrcTextEncryptionOn() ) && ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) ) ){			// OAPでかつ暗号化ON.	// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
		Bool b_EncryptionOn = pc_RdsSetting->getCsrcTextEncryptionOn();					// CSRCの設定値に従う.
		// CSRAレポート発信が利用可で、「初期発信」中でなければ.
		if ( (True == RDSC_RptDataManager::newInstance()->isAvailableSendReport()) && (True != tskRdsEvent.isNowTrans(TYPE_RdsTrgKindIni)) ) {
			b_EncryptionOn = RDSC_CsraPreCheck::newInstance()->isCsraEncryptionOn();	// CSRAの設定値に従う.
		}
		if( ( ( True == theVRcontroller.c_Cipher.isKeyExist() ) && ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice ) ) ||		// E-mailでかつ暗号化ON. 
			( ( True == b_EncryptionOn ) && ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice ) ) ||								// HTTPでかつ暗号化ON.
			( ( True == b_EncryptionOn ) && ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) ) ){								// OAPでかつ暗号化ON.
// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/22 Sky梶谷 END.
			// Mosel AES暗号化対応 06/07/12 佐久間 
//#if defined(DEF_MoselCom)
				// 機種固有 これは必要なコンパイルＳＷ
			// 暗号キーがある場合、Content-Type の「type」に、「enc」を指定する
			sprintf( (char*)(*ppBodyHeader)->FileName, "text%d.dat; type=enc", (indx+1) );
			si_len = (si_len + 16) / 16 * 16;													// 16 の倍数化（元々 16 の倍数であった場合は、16バイト増える）	06/07/14 佐久間 
//#else
//			// 暗号キーがある場合、Content-Type の「type」に、「dat」を指定する
//			sprintf( (char*)(*ppBodyHeader)->FileName, "text%d.dat; type=dat", (indx+1) );
//			si_len = (si_len + 8) / 8 * 8;													// 8 の倍数化（元々 8 の倍数であった場合は、8バイト増える）	02/05/23 浦崎
//#endif
		EPN_DBG_MESSAGE( (uchar*)"AES", si_len, (uchar*)" 送信サイズ " );
		} else {
			sprintf( (char*)(*ppBodyHeader)->FileName, "text%d.dat", (indx+1) );
		}

		(*ppBodyHeader)->FileSize = si_len;
		(*ppBodyHeader)->pNextHeader = NULL;

		if ( pLastHeader == NULL )
			pEmset->pMIMEBodyHeader = (*ppBodyHeader);										// はじめの添付ファイルは、大元の尻にくっつける
		else pLastHeader->pNextHeader = (*ppBodyHeader);									// 2つ目以降は、順次、前の尻にくっつける

		pLastHeader = (*ppBodyHeader);
	}

	return( True );
}
/*==================================================================================================
//	MIO を使用し、テキストを送信する
//--------------------------------------------------------------------------------------------------
// 02/05/24 浦崎  ・暗号化するとテキスト長が増加するが、増加したサイズを MIO に渡していなかった為
//				    修正
//				  ・NU 時は、暗号化しないようにした。
// 02/05/27 浦崎  Fiery 版と処理を合わせるため、NU 時も暗号化する（よって、上記処理を削除）。
==================================================================================================*/
/**/
bool ___dbg_dispAFileFlag = true;
void epDispAfile( int i )			// デバッグ用：送信添付ファイルの内容をターミナルに出力するかしないかの切り替え。 
{
	___dbg_dispAFileFlag = ( i != 0 ? true : false );
}
/**/
inline void printDBG_LOGtxMIOData( uchar* pData, int si_len )
{
#define _S_DbgDispString  48		// 230; 


// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/22 Sky梶谷 START.
//	if( ( ( True == theVRcontroller.c_Cipher.isKeyExist() ) && ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice ) )					// E-mailでかつ暗号化ON
//	 || ( ( True == pc_RdsSetting->getCsrcTextEncryptionOn() ) && ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice ) )				// HTTPでかつ暗号化ON
//	 || ( ( True == pc_RdsSetting->getCsrcTextEncryptionOn() ) && ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) ) ){			// OAPでかつ暗号化ON.	// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
	Bool b_EncryptionOn = pc_RdsSetting->getCsrcTextEncryptionOn();					// CSRCの設定値に従う.
	// CSRAレポート発信が利用可で、「初期発信」中でなければ.
	if ( (True == RDSC_RptDataManager::newInstance()->isAvailableSendReport()) && (True != tskRdsEvent.isNowTrans(TYPE_RdsTrgKindIni)) ) {
		b_EncryptionOn = RDSC_CsraPreCheck::newInstance()->isCsraEncryptionOn();	// CSRAの設定値に従う.
	}
	if( ( ( True == theVRcontroller.c_Cipher.isKeyExist() ) && ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice ) )		// E-mailでかつ暗号化ON
	 || ( ( True == b_EncryptionOn ) && ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice ) )							// HTTPでかつ暗号化ON
	 || ( ( True == b_EncryptionOn ) && ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) ) ){							// OAPでかつ暗号化ON.
// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/22 Sky梶谷 END.
		EPN_DBG_MESSAGE( (uchar*)"CSRC", si_len, (uchar*)" (暗号化されている) " );
		return;
	}
#if 1
	if( true == ___dbg_dispAFileFlag ) {
		uchar uc_dbgDispString[250];
		if( si_len < _S_DbgDispString )
		{
			memcpy( uc_dbgDispString, pData, _S_DbgDispString );
			uc_dbgDispString[si_len] = NULL;
			EPN_DBG_MESSAGE( (uchar*)"----", si_len, uc_dbgDispString );
		}
		else
		{
			int dbg_Indx = 0;
			while( 0 < (si_len - (_S_DbgDispString*dbg_Indx)) )
			{
				memset( uc_dbgDispString, 0, sizeof(uc_dbgDispString) );
				memcpy( uc_dbgDispString, &pData[_S_DbgDispString*dbg_Indx], _S_DbgDispString );
				if( 0 > (si_len - (_S_DbgDispString*(dbg_Indx+1))) ){
					uc_dbgDispString[(si_len - (_S_DbgDispString*dbg_Indx))] = NULL;
				}
				EPN_DBG_MESSAGE( (uchar*)"----", si_len, uc_dbgDispString );
				dbg_Indx++;
			}
		}
	}
#endif
}
/**/
Bool RDSC_RdsCommMIO::tx_transText( int fd, short ss_NumOfEventDataFiles, short ss_NumOfCommandResFiles )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", 0, (uchar*)"＜mioSetSendInfo() tx_transText＞" );			//2018/10/18 FPT-DucDH4 Modification log debug
	EPN_DBG_MESSAGE( (uchar*)"CSRC", ss_NumOfEventDataFiles, (uchar*)" ss_NumOfEventDataFiles" );			//2018/10/18 FPT-DucDH4 Modification log debug
	EPN_DBG_MESSAGE( (uchar*)"CSRC", ss_NumOfCommandResFiles, (uchar*)" ss_NumOfCommandResFiles" );			//2018/10/18 FPT-DucDH4 Modification log debug


	// mioSendData() テキスト送信 -----------------------------------------------------
	// 変数初期化
	int				fileIndex = 0;
	short			ss_numOfAllFiles = (short)(ss_NumOfEventDataFiles + ss_NumOfCommandResFiles);	// 2010/07/17 sone Minerva warning 対応 shortにキャスト.
	Bool			b_SendEventData = True;
	uchar			uc_FrameNo = 1;			// 1 で初期化 
	register int	indx;
	e_WebDAVClient.total_size = 0;			// 送信トータルサイズを初期化. 
	memset(e_WebDAVClient.uc_TmpBuff, 0, sizeof(e_WebDAVClient.uc_TmpBuff));				// バッファクリア 
	memset(e_WebDAVClient.uc_EventTmpBuff, 0, sizeof(e_WebDAVClient.uc_EventTmpBuff));		// バッファクリア 
	memset(e_WebDAVClient.ui_EventTmpBuffSize, 0, sizeof(e_WebDAVClient.ui_EventTmpBuffSize));							//イベントバッファサイズの初期化.
	e_WebDAVClient.ui_TmpBuffSize = 0;																		//RDバッファサイズの初期化.
	uc_TmpFrameNo = 0x01;	// 初期化. 

	for( indx=1; indx<=ss_numOfAllFiles; indx++ )
	{
		if( 0 < ss_NumOfEventDataFiles )
		{
			// イベントデータをまず全部送信してしまう。 
			b_SendEventData = True;
			sendEventData( fd, ss_NumOfEventDataFiles, ss_NumOfCommandResFiles, fileIndex, uc_FrameNo );
			ss_NumOfEventDataFiles --;
			fileIndex++;						// 05/01/25 浦崎 イベントデータ送信ごとにインクリメントしないといけなかった。 
		}
		else
		{
			// イベントデータがなくなったら、RD コマンドの処理をする。 
			b_SendEventData = False;
			sendRDData( fd, ss_NumOfEventDataFiles, ss_NumOfCommandResFiles, fileIndex, uc_FrameNo );
			ss_NumOfCommandResFiles --;
			fileIndex++;						// 05/01/25 浦崎 イベントデータ送信ごとにインクリメントしないといけなかった。 
		}


		// １データ送信終了時の処理 
		if( True == b_SendEventData )
		{
			EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)e_MyID, (uchar*)" 1データの送信完了！" );
			
			// イベントデータ送信であれば、イベント処理部に通知。 
			if( ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
			||  ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
			{
				// e_MyIDはMIOクラスのインスタンスから生成される変数なので、
				// 通信方式がHTTPの場合は、通信方式を強制的にHTTPのDefineを引数として渡す。
				tskRdsEvent.indTransEventData( theRdsApl.e_CommDevice );		// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
			}
			else if( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice )
			{
				// e_MyIDはMIOクラスのインスタンスから生成される変数なので、
				// 通信方式がMIOの場合は、いじらなくてもそのまま使える。
				tskRdsEvent.indTransEventData( e_MyID );
			}
			else
			{
				EPN_DBG_MESSAGE( (uchar*)"tx_transText", (uint32_t)e_MyID, (uchar*)" この条件に入ることはありえない！" );
			}
			
			
			
		}
		else
		{
			// RD の応答の場合、特に何もする事は無い。 
		}

		uc_FrameNo++;

	} // ← for ループ（全テキストを送信するまで） 


	return( True );
}
/*==================================================================================================*/
// イベントデータ送信処理。 
/*--------------------------------------------------------------------------------------------------*/
// 06/05/19 佐久間  AES暗号化対応のため、sendAttachedFileの引数を追加 
/*==================================================================================================*/
Bool RDSC_RdsCommMIO::sendEventData( int fd, short ss_NumOfEventDataFiles, short ss_NumOfCommandResFiles, int fileIndex, uchar uc_FrameNo )
{
	uint32_t			ul_txDataLen;
	Bool			b_LastFrame = False;

	EPN_DBG_MESSAGE( (uchar*)"[CSRC]", (uint32_t)ss_NumOfEventDataFiles, (uchar*)" RDSC_RdsCommMIO::sendEventData() ss_NumOfEventDataFiles");			//2018/10/18 FPT-DucDH4 Add log debug
	EPN_DBG_MESSAGE((uchar*)"[CSRC]", (uint32_t)ss_NumOfCommandResFiles, (uchar*)" RDSC_RdsCommMIO::sendEventData() ss_NumOfEventDataFiles");			//2018/10/18 FPT-DucDH4 Add log debug
	if( 0 < ss_NumOfEventDataFiles )
	{
		// イベントデータをまず全部送信してしまう。 
		memset( &uc_txData[_S_Frame_and_LEN], 0x00, sizeof(uc_txData) - _S_Frame_and_LEN );									// 2011/02/07 higuchi Minerva/Zeus 【静的解析 影響度高 CID_014696】 Start.
		memcpy( &uc_txData[_S_Frame_and_LEN], &(c_EvevntDataBuf.rd->uc_Buff[0]), sizeof(c_EvevntDataBuf.rd->uc_Buff) );		// 2011/02/07 higuchi Minerva/Zeus 【静的解析 影響度高 CID_014696】 Start.
		ul_txDataLen = c_EvevntDataBuf.rd->us_Len;

		ss_NumOfEventDataFiles --;

		// FrameNo と LEN を作る。 
		if( 0 == (ss_NumOfEventDataFiles + ss_NumOfCommandResFiles) )
		{
			b_LastFrame = True;
		}
		RDSC_MakeEventData::makeFrameNo_and_Len( uc_txData, ul_txDataLen, uc_FrameNo, b_LastFrame );

		// FrameNo と LEN 作成後、レングスをその分加算する。 
		ul_txDataLen += _S_Frame_and_LEN;

//		sl_TotalSendSize = 0;		// トータル送信サイズ初期化 
//		e_WebDAVClient.total_size = 0;	// 使用していないので破棄. 
		
		/* MIO へ送信する。 */
//		sendAttachedFile( fd, ul_txDataLen, uc_txData, fileIndex );
		// AES暗号化対応。データサイズが必ず2032Byteよりも小さい値となるため、
		// 第３引数をNULL、第４引数をTrueとする。// 追加 06.05.17 佐久間 
		// AES暗号化対応。パディングあり/なしの設定はここで行なう必要があるため 
		// 引数を追加。 06/07/20 佐久間 
		//sendAttachedFile( fd, ul_txDataLen, uc_txData, fileIndex ,NULL ,True); 
		EPN_DBG_MESSAGE( (uchar*)"[CSRC]", (uint32_t)ul_txDataLen , (uchar*)" RDSC_RdsCommMIO::sendEventData() ul_txDataLen: ");			//2018/10/18 FPT-DucDH4 Add log debug
		sendAttachedFile( fd, ul_txDataLen, uc_txData, fileIndex ,NULL ,True, False); 
	}

	return( True );
}
/*==================================================================================================*/
// イベントデータがなくなったら、RD コマンドの処理をする。 
/*--------------------------------------------------------------------------------------------------*/
// 06/07/20 佐久間  AES暗号化対応におけるパディング対応のため、sendAttachedFileの引数を追加 
// 06/10/04 佐久間　問題点No.70203対応。AES暗号化の場合は16 の倍数化の考慮が必要。 
/*==================================================================================================*/
Bool RDSC_RdsCommMIO::sendRDData( int fd, short ss_NumOfEventDataFiles, short ss_NumOfCommandResFiles, int fileIndex, uchar uc_FrameNo )
{
	uint32_t			ul_txDataLen;
	Bool			b_LastFrame = False;


	// RD 処理結果をキューより抜き出す。 
	uchar*			puc_Mode;
	uint32_t*			pul_RDDataSize;
	uchar*			puc_HostCmdData;
	short*			pss_HostCmdSize;
	uchar*			puc_A700Data;

	uchar	uc_IvData[16];		// 暗号化用IVデータ(16Byte) // 追加 06.05.17 佐久間 
	bool	b_FirstData;		// 先頭データ判別フラグ(True:先頭データ,False:後続データ) // 追加 06.05.17 佐久間 

	q_RD_Data.get( &puc_Mode, &pul_RDDataSize, &puc_HostCmdData, &pss_HostCmdSize, &puc_A700Data );	// キューからデータゲット（バッファへのポインタ、サイズ、モード） 

	uint32_t ul_HeaderLen = 0;
	// puc_A700Data は NULL の時もある。 
	if(    NULL != pul_RDDataSize
		&& NULL != puc_Mode
		&& NULL != puc_HostCmdData
		&& NULL != pss_HostCmdSize
		&& NULL != puc_A700Data    )
	{
		ul_txDataLen = *pul_RDDataSize;													// RD データのサイズは事前に計算してあるため、それを読み出す。 
		uchar*	puc_CommandString;
		if( RDSD_ResType_ZR == *puc_Mode )
		{
			puc_CommandString = (uchar*)"ZR";
		}
		else if( RDSD_ResType_WD_TxResult  == *puc_Mode )
		{
			puc_CommandString = (uchar*)"FD";
		}
		else
		{
			puc_CommandString = (uchar*)"ZW";
		}

		ushort us_addM5Len = 0;
		if( RDSD_ResType_WD_TxResult  == *puc_Mode )
		{
			// M501 コマンド 「M501yymmddhhmmss ?? 」← 末尾にスペースが入る。 
			uchar uc_BcdEventID[6+1];
			c_FND.getTrgTime( TYPE_RdsTrgKindTxResultI_ISW, uc_BcdEventID );
			us_addM5Len = RDSC_MakeEventData::makeM501
				(
					&uc_txData[_S_Frame_and_LEN],		// ユーザーデータバッファ 
					uc_BcdEventID,						// イベントID 
					puc_CommandString					// 発信固有文字列 
				);
			us_addM5Len--;	// 前述の通り末尾にスペースが入っているため、削る。 
		}
		else
		{
			// M500 コマンド 「M500yymmddhhmmss ?? 」← 末尾にスペースが入らない。 
			us_addM5Len = RDSC_MakeEventData::makeM500(				// 05/01/31 浦崎 メールIDが付加されないデグレ修正。 
					&uc_txData[_S_Frame_and_LEN],		// ユーザーデータバッファ 
					puc_CommandString					// 発信固有文字列 
				);
		}

		ul_txDataLen += us_addM5Len;
		ul_HeaderLen = us_addM5Len + _S_Frame_and_LEN;
	}
	else
	{
		ul_txDataLen = 0;
		EPN_DBG_MESSAGE( (uchar*)"CSRC", ss_NumOfEventDataFiles, (uchar*)" ★★★ ここに来る事は考えられない！！！ ★★★" );
	}


	ss_NumOfCommandResFiles --;

	// FrameNo と LEN を作る。 
	if( 0 == (ss_NumOfEventDataFiles + ss_NumOfCommandResFiles) )
	{
		b_LastFrame = True;
	}
	RDSC_MakeEventData::makeFrameNo_and_Len( uc_txData, ul_txDataLen, uc_FrameNo, b_LastFrame );

	// FrameNo と LEN 作成後、レングスをその分加算する。 
	ul_txDataLen += _S_Frame_and_LEN;


	uc_TmpFrameNo |= 0x80;
	
	// 静的解析 CID12653 NULLチェックを追加 sone 2010/12/20 START.
	// 静的解析 CID15052 NULLチェックの追加と関数の引数にポインターを使用しないよう修正 Iwata 2011/07/27 Start.
	if( NULL == puc_HostCmdData)
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", 0, (uchar*)" ★★★ puc_HostCmdDataかpss_HostCmdSizeがNULL！！ ★★★" );
	}
	// 静的解析 CID12653 NULLチェックを追加 sone 2010/12/20 END.
	int32_t sl_HostCmdSize = 0;

	if(	NULL == pss_HostCmdSize)
	{
		sl_HostCmdSize = 0;
	}
	else
	{
		sl_HostCmdSize = (int32_t)*pss_HostCmdSize;
	}

	/*---------------------------------------------------------------------------------------------*/
	// 初期化 
	theRdsEmail.mInquiry.Init(
		puc_HostCmdData,								// ホストから要求されたコマンド文字列(受信バッファ先頭アドレス + オフセット, )  
		sl_HostCmdSize							// ホストから要求されたコマンド文字列のサイズ[単位:byte](受信電文長 - オフセット)  
	);
	// 静的解析 CID15052 NULLチェックの追加と関数の引数にポインターを使用しないよう修正 Iwata 2011/07/27 End.

	// 残りデータはあるか？ 
	Bool b_Loop = True;
	int32_t  sl_tmpSize = 0;
	uint32_t ul_RemainIndx = 0;
	
	b_FirstData = True;								// 先頭データフラグOn // 追加 06.05.17 佐久間 
	memset(&uc_IvData[0], '\0', sizeof(uc_IvData));		// 暗号化用IVをNULLで初期化 // 追加 06.05.17 佐久間 
	
	//sl_TotalSendSize = 0;		// トータル送信サイズ初期化 
	//e_WebDAVClient.total_size = 0;

// 2017/08/07 MAv5 エラーチェック機構 start.
	std::string c_ParamCheckError("");
// 2017/08/07 MAv5 エラーチェック機構 end.
	
	while ( theRdsEmail.mInquiry.IsExistLeftData() || 0 < ul_RemainIndx )
	{
		//概要：不揮発の問い合わせ 
		//　　　要求されたユーザデータに従って、不揮発を読み出し 応答のユーザデータを作成する。 
		sl_tmpSize = 0;
		RDSE_NONVOLATILE_RESULT	e_NonvolatileRet = theRdsEmail.mInquiry.InquiryNonVolatile(
			&uc_txData[ ul_HeaderLen + ul_RemainIndx ],						// [OUT] 応答のユーザデータバッファ先頭アドレス 
			(_S_txDataBuffSize_Email - ul_HeaderLen - ul_RemainIndx),		// [IN]  応答のユーザデータ最大サイズ[単位:byte] 
			&sl_tmpSize,													// [OUT] 応答のユーザデータバッファに書き込んだサイズ[単位:byte] 
// 2017/08/07 MAv5 エラーチェック機構 start.
			&c_ParamCheckError
// 2017/08/07 MAv5 エラーチェック機構 end.
		);

		EPN_DBG_MESSAGE( (uchar*)"ul_HeaderLen + ul_RemainIndx", ul_HeaderLen + ul_RemainIndx, (uchar*)" " );
		EPN_DBG_MESSAGE( (uchar*)"ul_RemainIndx               ", ul_RemainIndx,                (uchar*)" " );
		EPN_DBG_MESSAGE( (uchar*)"sl_tmpSize                  ", sl_tmpSize,                   (uchar*)" " );

		// 2014/07/08 wada CQ500942_CSRCでのバックトレースログ取得改善 s.
		/*
			正常の時しかTrueにならないので、ここでチェックする.
		*/
		if( True == b_ReadZA_ReadReqFlg )
		{
			b_ReadZA_SendReqFlg = True;
		}
		// 2014/07/08 wada CQ500942_CSRCでのバックトレースログ取得改善 e.

		Bool b_8_div = True;
		switch (e_NonvolatileRet)
		{
		case RDSE_NONVOLATILE_RESULT_OK_NO_NEXT :			// 正常（残りデータ無）
			b_8_div = False;
			break;
		case RDSE_NONVOLATILE_RESULT_OK_NEXT :				// 正常（残りデータ有）
			b_8_div = True;
			break;
		case RDSE_NONVOLATILE_RESULT_SYNTAXERR :			// 文法エラー（内部矛盾）
			EPN_DBG_MESSAGE( (uchar*)"anaCmd_TypeRD", 0, (uchar*)" 文法エラー（内部矛盾）" );
			b_Loop = False;
			break;

		// 以下はコマンドRead/Write関数の検出エラー 
		case RDSE_NONVOLATILE_RESULT_READ_ERR :				// エラーで読めない（メカコン系データ）
		case RDSE_NONVOLATILE_RESULT_INVALID_ACCESS :		// データが無い（コマンド不正、パラメータ不正）
		case RDSE_NONVOLATILE_RESULT_WRITE_ERR_TO_RONLY :	// ReadOnlyに対する書き込みを行おうとした 
		default :
			EPN_DBG_MESSAGE( (uchar*)"anaCmd_TypeRD", (uint32_t)e_NonvolatileRet, (uchar*)" なんらかのエラー" );
			b_Loop = False;
			break;
		}
		if( b_Loop == False )
		{
			break;
		}


		uint32_t ul_thisTotalSize = sl_tmpSize + ul_HeaderLen + ul_RemainIndx;
		int si_thisSendSize = 0;
		if( True == b_8_div )	// 残データありのため、8 の倍数化の考慮が必要。 
		{
//#if defined(DEF_MoselCom)
				// 機種固有 これは必要なコンパイルＳＷ
			// 06/10/04 佐久間 AES暗号化の場合は16 の倍数化の考慮が必要。 
			int mag = ul_thisTotalSize/16;
			si_thisSendSize = 16*mag;
//#else
//			int mag = ul_thisTotalSize/8;
//			si_thisSendSize = 8*mag;
//#endif
			ul_RemainIndx = ul_thisTotalSize - si_thisSendSize;
		}
		else					// RD で読み出せる残データ無し。 
		{
			if( NULL != puc_A700Data )		// 静的解析 2013/05/11 mtoyoda ID18604  puc_A700DataのNULLチェックを行う。
			{
				if( 0x00 != *puc_A700Data ){
					/* puc_A700Data の先頭にはスペースも入っている。 */
					memcpy( &uc_txData[ul_thisTotalSize], puc_A700Data, 15 );
					ul_thisTotalSize += 15;
				}
			}
			si_thisSendSize = ul_thisTotalSize;
			ul_RemainIndx = 0;
		}



		/* MIO へ送信する。 */
		// AES暗号化対応。パディングあり/なしの設定はここで行なう必要があるため 
		// 引数を追加。 06/07/20 佐久間 
		// AES暗号化対応 06.05.19 佐久間 
		sendAttachedFile( fd, si_thisSendSize, uc_txData, fileIndex, &uc_IvData[0], b_FirstData, b_8_div );

		b_FirstData = False;

		// 8 の倍数から漏れたものの補正。 
		if( 0 != ul_RemainIndx ) {
			memcpy( uc_txData, &uc_txData[ ul_thisTotalSize - ul_RemainIndx ] , ul_RemainIndx );
		}
		ul_HeaderLen = 0;
		// ul_RemainIndx ← は、0 クリアしてはいけない。 
	}

// 2017/08/07 MAv5 エラーチェック機構 start.
	// CSRCにRWコマンドを通知する.
	if( 0 != c_ParamCheckError.length() ) {

		RDSD_DBG_MESSAGE_STR( "ParamCheckError!!", 0, 0, 0 );
		std::string	c_Log = c_ParamCheckError.substr( 0, 30 );
		RDSD_DBG_MESSAGE_STR( c_Log.c_str(), 0, 0, 0 );

		uchar uc_Temp[300] = {};
		RDSC_Time c_rdsWebTime = theVRmsc.getNowTime();							// 現在時刻取得.
		c_rdsWebTime.cnvTime_2_String( uc_Temp, RDSE_T2S_yyyymmddhhmmss );		// RDS現在時刻(BCDyyyymmddhhmmss形式)取得.
		sprintf( (char*)uc_Temp, "%s.txt", uc_Temp );							// ".txt"格納.

		tskRdsEvent.indNotifyCmdGenError( RDSE_CmdGenError_Kind_CSRC, (const uchar*)uc_Temp, (const uchar*)c_ParamCheckError.c_str() );
	}
// 2017/08/07 MAv5 エラーチェック機構 end.
	
	// 04/06/23 浦崎 free 忘れがあった．．．ため、対策。 
	free( puc_Mode );
	free( pul_RDDataSize );
	free( puc_HostCmdData );
	free( pss_HostCmdSize );
	free( puc_A700Data );

	return( True );
}
/*==================================================================================================*/
// MIO にデータを送信する。 
/*--------------------------------------------------------------------------------------------------*/
// 06.05.19 佐久間 AES暗号化対応 
// 06.07.20 佐久間  AES暗号化対応におけるパディング対応のため、sendAttachedFileの引数を追加 
// 16/02/01 Sky平田 ZeusS CQ521285 監査ログ HCD-PP対応.
/*==================================================================================================*/
//Bool RDSC_RdsCommMIO::sendAttachedFile( int fd, int si_TotalSize, uchar* pFileData, int fileIndex )
//Bool RDSC_RdsCommMIO::sendAttachedFile( int fd, int si_TotalSize, uchar* pFileData, int fileIndex, uchar* puc_IVData, bool b_FirstData )
Bool RDSC_RdsCommMIO::sendAttachedFile( int fd, int si_TotalSize, uchar* pFileData, int fileIndex, uchar* puc_IVData, Bool b_FirstData, Bool b_LastData )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", (int32_t)si_TotalSize, (uchar*)"sendAttachedFile si_TotalSize ", (int32_t)si_TotalSize );			//2018/10/18 FPT-DucDH4 Add log debug
	// 2011/05/05 yito CQ343107 start.
	// AESのパディングの影響で、データサイズが0になっている可能性があるため、
	// データサイズが0の際は送信処理をしない。
	if( 0 == si_TotalSize )
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", 0, (uchar*)"sendAttachedFile" );
		return( True );
	}
	// 2011/05/05 yito CQ343107 end  .

	int				offset;
	int				remain, sendSize;

	signed char		sc_padding;	// パディングフラグ(0：パディング必要、1：パディング不要) // 追加 06.05.15 佐久間 
	int32_t sl_encDataSize;	// 暗号化後のデータサイズ // 追加 06.05.17 佐久間 
	Bool b_firstEnc;			// 先頭データフラグ(True:先頭データ, False:後続データ) // 追加 06.05.17 佐久間 
	/* MIO へ送信する。 */
	remain = si_TotalSize;
	offset = 0;
	b_firstEnc = b_FirstData;	// 先頭データフラグの待避 // 追加 06．05.17 佐久間 
	
	
	do
	{
		// AES暗号化対応 06.05.19 佐久間 
		//if (remain >= MIO_SEND_UNIT)	sendSize = MIO_SEND_UNIT;							// 送信サイズの上限チェック（らしい）
		//else							sendSize = remain;
		// 送信サイズの上限チェック（らしい）
		if (remain > MIO_SEND_UNIT){						// 続きのデータがある場合 
			sendSize = MIO_SEND_UNIT;
				sc_padding = 1;								// パディング不要 
		} else {											// 最終データの場合 
			sendSize = remain;
			// パディング対応 06.07.20 佐久間 
			if( b_LastData == True )						// 残データあり 
			{
				sc_padding = 1;								// パディング不要 
			} else {
				sc_padding = 0;								// パディング必要 
			}
		}


		/* 暗号鍵を持っていたら暗号化する。 */
		int si_EncryptSize = sendSize;														// 暗号化前のデータサイズ 
// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/22 Sky梶谷 START.
//		if( ( ( True == theVRcontroller.c_Cipher.isKeyExist() ) && ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice ) )					// E-mailでかつ暗号化ON
//		  ||( ( True == pc_RdsSetting->getCsrcTextEncryptionOn() ) && ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice ) )				// HTTPでかつ暗号化ON
//		  ||( ( True == pc_RdsSetting->getCsrcTextEncryptionOn() ) && ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) ) ){			// OAPでかつ暗号化ON.	// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
		Bool b_EncryptionOn = pc_RdsSetting->getCsrcTextEncryptionOn();					// CSRCの設定値に従う.
		// CSRAレポート発信が利用可で、「初期発信」中でなければ.
		if ( (True == RDSC_RptDataManager::newInstance()->isAvailableSendReport()) && (True != tskRdsEvent.isNowTrans(TYPE_RdsTrgKindIni)) ) {
			b_EncryptionOn = RDSC_CsraPreCheck::newInstance()->isCsraEncryptionOn();	// CSRAの設定値に従う.
		}
		if( ( ( True == theVRcontroller.c_Cipher.isKeyExist() ) && ( RDSE_TSK_ID_MIO == theRdsApl.e_CommDevice ) )	// E-mailでかつ暗号化ON
		  ||( ( True == b_EncryptionOn ) && ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice ) )						// HTTPでかつ暗号化ON
		  ||( ( True == b_EncryptionOn ) && ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) ) ){						// OAPでかつ暗号化ON.
// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/22 Sky梶谷 END.
			// AES暗号化対応 06.05.17 佐久間 ここから
//			theVRcontroller.c_Cipher.encrypt( (pFileData+offset), sendSize, &si_EncryptSize );	// 暗号化 
			// 暗号化 
			if ( b_firstEnc == True ) {							// 先頭データの場合 
				EPN_DBG_MESSAGE( (uchar*)"AES", (uint32_t)sendSize, (uchar*)" 先頭：データサイズ " );
				EPN_DBG_MESSAGE( (uchar*)"AES", (uint32_t)sc_padding, (uchar*)" 先頭：パディング必要か否か " );
				theVRcontroller.c_Cipher.encrypt_AES( 
										(pFileData+offset), 
										sendSize, 
										sc_padding, 
										NULL, 
										&sl_encDataSize 
				);
			} else {											// 後続データの場合 
				EPN_DBG_MESSAGE( (uchar*)"AES", (uint32_t)sendSize, (uchar*)" 後続：データサイズ " );
				EPN_DBG_MESSAGE( (uchar*)"AES", (uint32_t)sc_padding, (uchar*)" 後続：パディング必要か否か " );
				theVRcontroller.c_Cipher.encrypt_AES( 
										(pFileData+offset), 
										sendSize, 
										sc_padding, 
										puc_IVData, 
										&sl_encDataSize 
				);
			}
			si_EncryptSize = (int)sl_encDataSize;

			b_firstEnc = False;

			if ( puc_IVData != NULL ) {
				memcpy( puc_IVData, (char*)(pFileData+offset+sl_encDataSize-16), 16);	// 暗号化後のデータの最終16ByteをIVに格納 
			}
			// AES暗号化対応 ここまで 
		}

		/**/
		//printDBG_LOGtxMIOData( &pFileData[offset], si_EncryptSize );
		/**/
		
		/*ここでHTTP版のイベントデータ、およびRDコマンドデータを作成する*/
		if( ( RDSE_TSK_ID_HTTP == theRdsApl.e_CommDevice )
		||  ( RDSE_TSK_ID_OAP == theRdsApl.e_CommDevice ) )	// 2017/06/23 Sky MAv5 RQ-669 DCAとMFPの連携強化.
		{
			/*フレームナンバーが80番台でなければ、イベントデータを作成*/
			if( 0 == ( uc_TmpFrameNo & 0x80 ) )
			{
				memcpy( &e_WebDAVClient.uc_EventTmpBuff[ fileIndex ][0], pFileData+offset, si_EncryptSize ); 
				e_WebDAVClient.ui_EventTmpBuffSize[ fileIndex ] += si_EncryptSize;
				
			}
			/*フレームナンバーが80番台ならRDコマンドデータを作成*/
			else
			{
				memcpy( &e_WebDAVClient.uc_TmpBuff[ e_WebDAVClient.total_size ], pFileData+offset, si_EncryptSize ); 
				e_WebDAVClient.ui_TmpBuffSize += si_EncryptSize;
				e_WebDAVClient.total_size += si_EncryptSize;		// 送信サイズ. 
			}
		}
		else
		{
// 2010/07/17 higuchi Minerva warning 対応 Start.
			int	rcode = OK;
// 2010/07/17 higuchi Minerva warning 対応 End.

#ifdef ROM_DIV_3	/* marumoto 2006.3.27 NICFUM ROM_DIV */
			if(( pt_mioSendData == NULL )
			||(( pt_mioSendData != NULL )
			 &&( OK != (rcode = ((STATUS(*)(MioFileDesc,UINT16,char *,UINT32))pt_mioSendData)((MioFileDesc)fd, (UINT16)fileIndex, (char*)(pFileData+offset), si_EncryptSize )) )))
#else
			if ( OK != (rcode = epMioSendData((MioFileDesc)fd, fileIndex, (char*)(pFileData+offset), si_EncryptSize )) )
#endif
			{
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	Start.
				// 監査ログに保存.
				addOperationLogSession( rcode );
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	End.

				// 2011/05/05 yito CQ343107 start.
				EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)rcode, (uchar*)"＜ERROR mioSendData()＞" );	// 2011/05/05 yito CQ343107 ログ修正 .
				EPN_DBG_MESSAGE( (uchar*)"mioSendData", (uint32_t)fd, (uchar*)"mioSendData:fd" );
				EPN_DBG_MESSAGE( (uchar*)"mioSendData", (uint32_t)fileIndex, (uchar*)"mioSendData:fileIndex" );
				EPN_DBG_MESSAGE( (uchar*)"mioSendData", (uint32_t)offset, (uchar*)"mioSendData:offset" );
				EPN_DBG_MESSAGE( (uchar*)"mioSendData", (uint32_t)si_EncryptSize, (uchar*)"mioSendData:si_EncryptSize" );
				// 2011/05/05 yito CQ343107 end  .
				stopComm( MLog_Nothing3, RDSE_ERR_MIO_MioSendData, (ushort)rcode );					// シーケンス破棄（１分後再開）
				return( False );
			}

// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	Start.
			// 監査ログに保存.
			addOperationLogSession( rcode );
// 2016/02/01 CQ521285 監査ログHCD-PP対応	Sky平田	End.
		}
		remain -= sendSize;
		offset += sendSize;
//		e_WebDAVClient.total_size += sendSize;
	} while (remain > 0);

	return( True );
}
/*==================================================================================================
//	送信終了処理（MIOからの送信終了MSG受信）
//		※ taskEPNet_MIO2()：MIO からのメッセージ受信タスクからコールされる
==================================================================================================*/
void RDSC_RdsCommMIO::tx_transEnd( void )
{
	EPN_DBG_MESSAGE( (uchar*)"CSRC", e_NowCommMode, (uchar*)" ＜MIO 送信終了通知MSGを受信した＞" );
	if( MIO_Trans_WaitTransEndMSG_State == transState )
	{
		if ( e_Webdavtask.localTrg != TYPE_RdsTrgKindHB )
		{
// 2017/7/25 MAv5 CSRA接続_通信履歴 start.
// 2017/9/12 MAv5 AccuRev 1028998 start.
			if ( True == RDSC_CsraConnectionManager::newInstance()->isSending() ) {
// 2017/9/12 MAv5 AccuRev 1028998 end.
				theVRcontroller.m_c_CsraLog.setLogHttp(
						std::string(e_WebDAVClient.filename),
						MLog_Tx_NormalEnd
				);
				RDSD_DBG_CSRA_PERFORMANCE( "Log", 0, 0, 0, True );
// 管理システム統合化開発 RQ-811 要求仕様2-B 2021/01/21 Sky一ノ瀬 START.
			} else if( True == RDSC_CsrcConnectionManager::newInstance( RDSE_CommunicationType_Report )->isSending() ) {
				theVRcontroller.m_c_CsraLog_Report.setLogHttp(
						std::string(e_WebDAVClient.filename),
						MLog_Tx_NormalEnd
				);
// 管理システム統合化開発 RQ-811 要求仕様2-B 2021/01/21 Sky一ノ瀬 END.
			} else {
// 2021/01/20 Sky二口 [OP_BTS-30824][RQ-811]即時要求用の通信履歴Logの追加 START.				
				if( True == RDSC_CsrcConnectionManager::newInstance( RDSE_CommunicationType_Immediate )->isSending() ) {
					strncpy( (char*)tskRdsCommMIO.s_tmpMioHeader.Adr.From, "device@csrc.konicaminolta.com", strlen("device@csrc.konicaminolta.com") );	// From.
					strncpy( (char*)theRdsEmail.s_TxEmailHeader.adr		 , "host@csrc.konicaminolta.com",   strlen("host@csrc.konicaminolta.com") );	// To.
					// dateはデッドコード.
					// subjectは即時要求では取得できないため未設定.
				}
// 2021/01/20 Sky二口 [OP_BTS-30824][RQ-811]即時要求用の通信履歴Logの追加 END.				

				theVRcontroller.c_MailLog.setLog(								// 通信ログをセット.
						&s_tmpMioHeader.Adr.From[0],							// From.
						&theRdsEmail.s_TxEmailHeader.adr[0],					// To.
						&theRdsEmail.s_TxEmailHeader.date,						// Date.
						&theRdsEmail.s_TxEmailHeader.subject[0],				// Subject.
						NULL,													// Received.
						MLog_Tx_NormalEnd										// （結果）.
				);
			}
// 2017/7/25 MAv5 CSRA接続_通信履歴 end.
		}
// 2020/10/23 Sky二口 OP_BTS-28458 [RQ-811] START.
		if ( ( False == RDSC_CsraConnectionManager::newInstance()->isSending() )
		  && ( False == RDSC_CsrcConnectionManager::newInstance( RDSE_CommunicationType_Immediate )->isSending() ) ) {
			theRdsEmail.emailTransNormalEndProc( RDSE_TSK_ID_MIO, e_NowCommMode );
		}
// 2020/10/23 Sky二口 OP_BTS-28458 [RQ-811] END.

		stopComm( MLog_Tx_NormalEnd, RDSE_ERR_Email_Normal, 0 );
//2018/12/24 【FPT-VuLH13】【IT6_2.0】【EAGLEH】 No.36_FAX通信管理レポートの改善 Added Start.
		if (strlen(RDSC_ReadCommand::pPathFileReport))
		{
			int32_t si_ret = FILF_Remove((const char*)RDSC_ReadCommand::pPathFileReport, False);
			if (ERROR == si_ret)
			{
				EPN_DBG_MESSAGE((uchar*)"RDSC_ReadCommand::ReadXR() deleteReportFile call FILF_Remove() Error", 0, (uchar*)RDSC_ReadCommand::pPathFileReport);
			}
			else
			{
				EPN_DBG_MESSAGE((uchar*)"RDSC_ReadCommand::ReadXR() deleteReportFile call FILF_Remove() Success", 0, (uchar*)RDSC_ReadCommand::pPathFileReport);
			}
			memset(RDSC_ReadCommand::pPathFileReport, 0x00, sizeof(RDSC_ReadCommand::pPathFileReport));
		}
//2018/12/24 【FPT-VuLH13】【IT6_2.0】【EAGLEH】 No.36_FAX通信管理レポートの改善 Added End.
//2019/05/16 FPT-DongND4 [IT6_1.0MR2] [1070670]【Eagle】 added Start.
		if(strlen( RDSC_Inquiry::pathForCSRC_ZP))
		{
			remove( (const char*)RDSC_Inquiry::pathForCSRC_ZP );
			EPN_DBG_MESSAGE((uchar*)"Remove pathForCSRC_ZP Success", 0, (uchar*)RDSC_Inquiry::pathForCSRC_ZP);
			memset(RDSC_Inquiry::pathForCSRC_ZP, 0x00, sizeof(RDSC_Inquiry::pathForCSRC_ZP));
		}
//2019/05/16 FPT-DongND4 [IT6_1.0MR2] [1070670]【Eagle】 added End.
// 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Add Start.
		if(strlen( RDSC_Inquiry::pathForCSRC_ZV))
		{
			remove( (const char*)RDSC_Inquiry::pathForCSRC_ZV );
			EPN_DBG_MESSAGE((uchar*)"Remove pathForCSRC_ZV Success ", 0, (uchar*)RDSC_Inquiry::pathForCSRC_ZV);
			memset(RDSC_Inquiry::pathForCSRC_ZV, 0x00, sizeof(RDSC_Inquiry::pathForCSRC_ZV));
		}
// 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Add End.
	}
	else
	{
		EPN_DBG_MESSAGE( (uchar*)"CSRC", 0, (uchar*)" ＜ここに来るのはおかしい＞" );
		stopComm( MLog_Nothing, RDSE_ERR_MIO_ILLEGAL_STATE_ERR_3, RDSE_ERR_MIO_ILLEGAL_STATE_ERR_3 );	// シーケンス破棄（１分後再開） 
	}

//	theVRcontroller.endSq();										// テストメールシーケンス終了 
}

// 2017/6/27 MAv5 CSRA接続 start.
/*==================================================================================================
//	MIO とのメール送信シーケンス（Csra用）
==================================================================================================*/
void RDSC_RdsCommMIO::tx_transCsraSq( void )
{
	Bool	b_Ret = True;
	FILE*	fp = NULL;

	RDSD_DBG_CSRA_PERFORMANCE( "startHDDAccess_s", 0, 0, 0, True );
// MFP管理技術開発v5[KRQ-335][CSRA接続][AccuRev:1026466] start.
	RDSC_FileUtil::startHDDAccess();
// MFP管理技術開発v5[KRQ-335][CSRA接続][AccuRev:1026466] end.
	RDSD_DBG_CSRA_PERFORMANCE( "startHDDAccess_e", 0, 0, 0, True );
	do {

		RDSE_CmdDataCycle e_RdsCsraControlCycle = RDSE_CmdDataCycle_Unknown;
		std::string c_FileName("");
		std::string c_FilePath("");

// 管理システム統合化開発 RQ-811 要求仕様2-B 2021/01/21 Sky一ノ瀬 START.
		if ( True == RDSC_CsraConnectionManager::newInstance()->isSending() ) {
			// CSRAデータからファイル名、ファイルパスを取得し付与する処理.
			if (  True == RDSC_CsraConnectionManager::newInstance()->isExistToBeSendKind( RDSE_CsraConnectionKind_ConstantPeriod_Day ) ) {
				e_RdsCsraControlCycle = RDSE_CmdDataCycle_Day;
			} else if ( True == RDSC_CsraConnectionManager::newInstance()->isExistToBeSendKind( RDSE_CsraConnectionKind_ConstantPeriod_Week ) ) {
				e_RdsCsraControlCycle = RDSE_CmdDataCycle_Wday;
			} else if ( True == RDSC_CsraConnectionManager::newInstance()->isExistToBeSendKind( RDSE_CsraConnectionKind_ConstantPeriod_Month ) ) {
				e_RdsCsraControlCycle = RDSE_CmdDataCycle_Month;
			} else {
				RDSD_DBG_MESSAGE_STR( "isExistToBeSendKind() Error", 0, 0, 0 );
				b_Ret = False;
				break;
			}
			// DadaManagerに送信開始要求を行いファイル名を取得.
			b_Ret = RDSC_CmdDataManager::newInstance()->notify_SendStart( e_RdsCsraControlCycle, &c_FileName );
			if( False == b_Ret ) {
				RDSD_DBG_MESSAGE_STR( "notify_SendStart Error", b_Ret, 0, 0 );
				break;
			}
			// RQ-893 静的解析対応 [ID:260321] 2021/05/13 Sky大橋 START.
			// c_FileNameの文字数チェックをし、コピー元がコピー先サイズを超えていたら、ログを残し処理を抜ける.
			if ( c_FileName.length() > filename_max ) {
				RDSD_DBG_MESSAGE_STR( " c_FileName.length() over filename_max ", 0, 0, 0 );
				b_Ret = False;
				break;
			} else {
				// コピーしておく.
				strncpy( e_WebDAVClient.filename, c_FileName.c_str(), c_FileName.length() );
			}
			// RQ-893 静的解析対応 [ID:260321] 2021/05/13 Sky大橋 END.

			// ファイルOPENしてバッファに詰める.
			RDSC_CsraUtil::getCsraDirPath( &c_FilePath );
			c_FilePath += c_FileName;
		} else if ( True == RDSC_CsrcConnectionManager::newInstance( RDSE_CommunicationType_Report )->isSending() ) {
			// レポートデータからファイル名、ファイルパスを取得し付与する処理.
			// RptManagerに送信開始要求を行いファイル名を取得.
			b_Ret = RDSC_RptDataManager::newInstance()->notify_SendStart( &c_FileName );
			if( False == b_Ret ) {
				RDSD_DBG_MESSAGE_STR( "RDSC_RptDataManager::notify_SendStart Error", b_Ret, 0, 0 );
				break;
			}
			// RQ-893 静的解析対応 [ID:260321] 2021/05/13 Sky大橋 START.
			// c_FileNameの文字数チェックをし、コピー元がコピー先サイズを超えていたら、ログを残し処理を抜ける.
			if ( c_FileName.length() > filename_max ) {
				RDSD_DBG_MESSAGE_STR( " c_FileName.length() over filename_max ", 0, 0, 0 );
				b_Ret = False;
				break;
			} else {
				// コピーしておく.
				strncpy( e_WebDAVClient.filename, c_FileName.c_str(), c_FileName.length() );
			}
			// RQ-893 静的解析対応 [ID:260321] 2021/05/13 Sky大橋 END.
			
			// ファイルOPENしてバッファに詰める.
			RDSC_CsraReportUtil::getDirPath( &c_FilePath );
			c_FilePath += c_FileName;
		} else {
			// 処理なし(ここには入らない).
			RDSD_DBG_MESSAGE_STR( "tx_transCsraSq isSending Error", 0, 0, 0 );
			break;
		}
// 管理システム統合化開発 RQ-811 要求仕様2-B 2021/01/21 Sky一ノ瀬 END.

		fp = FILF_Fopen( c_FilePath.c_str(), "rb" );
		if( NULL == fp ) {
			RDSD_DBG_MESSAGE_STR( "FILF_Fopen() Error", 0, 0, 0 );
			b_Ret = False;
			break;
		}

		// 対象ファイルのサイズチェックを行い領域を確保する.
		FILF_Fseek( fp, 0, SEEK_END);
		const int64_t ul_FileSize = FILF_Ftell( fp );
		FILF_Fseek( fp, 0, SEEK_SET );

		RDSD_DBG_MESSAGE_STR( "FileSize=", ul_FileSize, e_RdsCsraControlCycle, 0 );

		// バッファ不足なのでバッファの追加が必要.
		if( (int64_t)sizeof( e_WebDAVClient.uc_Data ) <  ul_FileSize ) {
			RDSD_DBG_MESSAGE_STR( "Buffer Empty!!", sizeof( e_WebDAVClient.uc_Data ), ul_FileSize, 0 );
			break;
		}

		// 送信バッファに書き込み.
		if ( ERROR == FILF_Fread( e_WebDAVClient.uc_Data, 1, (size_t)ul_FileSize, fp ) ) {
			RDSD_DBG_MESSAGE_STR( "FILF_Fread() Error", ul_FileSize, 0, 0 );
			b_Ret = False;
			break;
		}

		// サイズを更新.
		e_WebDAVClient.sl_putTotal_size = (int32_t)ul_FileSize;  //[AR : 1026431] 【 Eagle 】【静的解析対応用課題】 2018/01/16 [HCL_Raghu] 

	} while(0);

	if( NULL != fp ) {
		FILF_Fclose( fp );
	}
	
	RDSD_DBG_CSRA_PERFORMANCE( "endHDDAccess_s", 0, 0, 0, True );
// MFP管理技術開発v5[KRQ-335][CSRA接続][AccuRev:1026466] start.
	RDSC_FileUtil::endHDDAccess();
// MFP管理技術開発v5[KRQ-335][CSRA接続][AccuRev:1026466] end.
	RDSD_DBG_CSRA_PERFORMANCE( "endHDDAccess_e", 0, 0, 0, True );

	RDSD_DBG_MESSAGE( b_Ret, 0, 0 );

	RDSD_DBG_CSRA_PERFORMANCE( "e_Webdavtask.reqNicMsgStart", 0, 0, 0, True );
	if( True == b_Ret ) {
		// 送信開始通知.
		e_Webdavtask.reqNicMsgStart();
	} else {
		// 送信開始失敗.
		std::string c_FileName(e_WebDAVClient.filename);
		memset( e_WebDAVClient.filename, 0, sizeof(e_WebDAVClient.filename) );
// 管理システム統合化開発 RQ-811 要求仕様2-B 2021/01/21 Sky一ノ瀬 START.
		if ( True == RDSC_CsraConnectionManager::newInstance()->isSending() ) {
			RDSC_CmdDataManager::newInstance()->notify_SendEnd( c_FileName, False );
		} else if ( True == RDSC_CsrcConnectionManager::newInstance( RDSE_CommunicationType_Report )->isSending() ) {
			RDSC_RptDataManager::newInstance()->notify_SendEnd( c_FileName, False );
		} else {
			// 処理なし(ここには入らない).
			RDSD_DBG_MESSAGE_STR( "tx_transCsraSq notify_SendEnd isSending Error", 0, 0, 0 );
		}
// 管理システム統合化開発 RQ-811 要求仕様2-B 2021/01/21 Sky一ノ瀬 END.

		
		theRdsApl.initJob( RDSE_TSK_ID_HTTP );
	}
}
// 2017/6/27 MAv5 CSRA接続 end.

// 2020/09/25 RQ-776 CWH 1-way通信方式の追加 start.
/*==================================================================================================
//	Collect command from block.
==================================================================================================*/
void RDSC_RdsCommMIO::getCmdCsraOneWay( char* pCmd )
{
	char		c_TempCmd[TYPD_CSRASetting_SendInfo_CommandMaxLen];
	char*		pc_tmpCommand = &c_TempCmd[0];
	ushort		us_CsrcCenterSendSetting;

	memset( c_TempCmd, 0, sizeof(c_TempCmd));

//2021/06/04 FPT-HaDV8 【IT6_2.2】【IT6_2.2-594】 Start.
	// Get machine type and feature version.
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	pc_SystemInfo->lock();
	TYPE_MachineType e_Type = pc_SystemInfo->getMachineType();
	TYPE_FuncVer e_FuncVer = pc_SystemInfo->getFuncVer2();
	pc_SystemInfo->unlock();
//2021/06/04 FPT-HaDV8 【IT6_2.2】【IT6_2.2-594】 End.
//2021/10/14 FPT-TinhNT16 【IT6-2.2MR1】 【制御構成変更_No.38】 Start.
	TYPE_CSRASettingComMode e_CSRACommMode = APIC_CSRASetting::newInstance()->getCSRASettingComMode();
//2021/10/14 FPT-TinhNT16 【IT6-2.2MR1】 【制御構成変更_No.38】 End.
	// +"A1 B0" Add a command group to calculate the total color counter.
	memcpy( pc_tmpCommand, " A1 B0", 6 );
	pc_tmpCommand += 6;

	// +"OC PC FC" Add a command group to calculate the total color counter.
	memcpy( pc_tmpCommand, " OC PC FC", 9 );
	pc_tmpCommand += 9;

	// 2020/11/13 FPT-ThanhPC7 【IT6_2.1】[OP_BTS-28952]【管理NO.70】 Start.
	// E-mail/HTTP One-way and initial call, maintenance end call, or scheduled call.
	//if( ((RDSE_TSK_ID_MIO  == theRdsApl.isRdsSystem() && True == theRdsApl.isSimplex()) ||
	//	(RDSE_TSK_ID_HTTP == theRdsApl.isRdsSystem() && True != pc_RdsSetting->getCsrcCommunicationMethod())) && 
	//	((True == tskRdsEvent.isNowTrans( TYPE_RdsTrgKindMnteEnd )) ||
	//	(True == tskRdsEvent.isNowTrans( TYPE_RdsTrgKindIni )) ||
	//	(True == tskRdsEvent.isNowTrans( TYPE_RdsTrgKindTimeOut ))) )
	//{
	// 2020/11/13 FPT-ThanhPC7 【IT6_2.1】[OP_BTS-28952]【管理NO.70】 End.	
		memcpy( pc_tmpCommand, " A6", 3 );
		pc_tmpCommand += 3;
	// 2020/11/13 FPT-ThanhPC7 【IT6_2.1】[OP_BTS-28952]【管理NO.70】 Start.
	//}
	// 2020/11/13 FPT-ThanhPC7 【IT6_2.1】[OP_BTS-28952]【管理NO.70】 end.
	// Get items to send on a regular basis.
	us_CsrcCenterSendSetting = pc_RdsSetting->getCsrcCenterSendSetting();
	us_CsrcCenterSendSetting = us_CsrcCenterSendSetting & 0xFFFF;

	// If there are items to be sent on a regular basis.
	if( us_CsrcCenterSendSetting )
	{
	// Copy the command string pointer into a temporary array.
	const char *cpcSendCommandStr[_S_Command_MAX_LEN] = { NULL };
//2021/06/04 FPT-HaDV8 【IT6_2.2】【IT6_2.2-594】 Start.
//	// Get machine type and feature version.
//	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
//	pc_SystemInfo->lock();
//	TYPE_MachineType e_Type = pc_SystemInfo->getMachineType();
//	TYPE_FuncVer e_FuncVer = pc_SystemInfo->getFuncVer2();
//	pc_SystemInfo->unlock();
//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 Start.
		if( TYPE_FuncVer_Expansion4 <= e_FuncVer )				// 機能バージョン2.3.
		{
			if( (TYPE_MachineTypeSparrow1SFP == e_Type) ||
				(TYPE_MachineTypeSparrow2SFP == e_Type) )
			{
				for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
				{
					cpcSendCommandStr[ ss_i ] = SendCommandStr_SparrowSFP_Exp_04[ ss_i ];
				}
			}
			else if( TYPE_MachineTypeSparrowBK0SFP == e_Type )
			{
				for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
				{
					cpcSendCommandStr[ ss_i ] = SendCommandStr_SparrowBkSFP_Exp_04[ ss_i ];
				}
			}
			else if( (TYPE_MachineTypeHeliosMLK3_CN == e_Type) ||
					 (TYPE_MachineTypeHeliosMLK2_5_CN == e_Type) ||
					 (TYPE_MachineTypeHeliosMLK2_5_CNDF == e_Type) ||
					 (TYPE_MachineTypeHeliosMLK2_CNDF == e_Type) )
			{
				for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
				{
					cpcSendCommandStr[ ss_i ] = SendCommandStr_HeliosMLK_CN_Exp_04[ ss_i ];
				}
			}
			else if( (TYPE_MachineTypeMinervaSSBKX == e_Type) ||
					 (TYPE_MachineTypeMinervaSSBK0_US == e_Type) ||
					 (TYPE_MachineTypeMinervaSSBK0_EU == e_Type) )
			{
				for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
				{
					cpcSendCommandStr[ ss_i ] = SendCommandStr_MinervaSSBK_Exp_04[ ss_i ];
				}
			}
			else
			{
				for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
				{
					cpcSendCommandStr[ ss_i ] = SendCommandStr_Exp_04[ ss_i ];
				}
			}
		}
//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 End.
//2021/03/09 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 制御構成変更_No.4 Start.
		else if( TYPE_FuncVer_Expansion3 <= e_FuncVer )				// 機能バージョン2.2.
		{
			if( (TYPE_MachineTypeSparrow1SFP == e_Type) ||
				(TYPE_MachineTypeSparrow2SFP == e_Type) )
			{
				for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
				{
					cpcSendCommandStr[ ss_i ] = SendCommandStr_SparrowSFP_Exp_03[ ss_i ];
				}
			}
			else if( TYPE_MachineTypeSparrowBK0SFP == e_Type )
			{
				for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
				{
					cpcSendCommandStr[ ss_i ] = SendCommandStr_SparrowBkSFP_Exp_03[ ss_i ];
				}
			}
			else if( (TYPE_MachineTypeHeliosMLK3_CN == e_Type) ||
					 (TYPE_MachineTypeHeliosMLK2_5_CN == e_Type) ||
					 (TYPE_MachineTypeHeliosMLK2_5_CNDF == e_Type) ||
					 (TYPE_MachineTypeHeliosMLK2_CNDF == e_Type) )
			{
				for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
				{
					cpcSendCommandStr[ ss_i ] = SendCommandStr_HeliosMLK_CN_Exp_03[ ss_i ];
				}
			}
			else if( (TYPE_MachineTypeMinervaSSBKX == e_Type) ||
					 (TYPE_MachineTypeMinervaSSBK0_US == e_Type) ||
					 (TYPE_MachineTypeMinervaSSBK0_EU == e_Type) )
			{
				for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
				{
					cpcSendCommandStr[ ss_i ] = SendCommandStr_MinervaSSBK_Exp_03[ ss_i ];
				}
			}
			else
			{
				for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
				{
					cpcSendCommandStr[ ss_i ] = SendCommandStr_Exp_03[ ss_i ];
				}
			}
		}
// 2020/11/13 FPT-ThanhPC7 【IT6_2.1】[OP_BTS-28952]【管理NO.70】 Start.
//		if( TYPE_FuncVer_Expansion2 <= e_FuncVer )				// 機能バージョン2.1.
		else if( TYPE_FuncVer_Expansion2 <= e_FuncVer )				// 機能バージョン2.1.
//2021/03/09 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 制御構成変更_No.4 End.
		{
			if( (TYPE_MachineTypeSparrow1SFP == e_Type) ||
				(TYPE_MachineTypeSparrow2SFP == e_Type) )
			{
				for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
				{
					cpcSendCommandStr[ ss_i ] = SendCommandStr_SparrowSFP_Exp_02[ ss_i ];
				}
			}
			else if( TYPE_MachineTypeSparrowBK0SFP == e_Type )
			{
				for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
				{
					cpcSendCommandStr[ ss_i ] = SendCommandStr_SparrowBkSFP_Exp_02[ ss_i ];
				}
			}
			else if( (TYPE_MachineTypeHeliosMLK3_CN == e_Type) ||
					 (TYPE_MachineTypeHeliosMLK2_5_CN == e_Type) ||
					 (TYPE_MachineTypeHeliosMLK2_5_CNDF == e_Type) ||
					 (TYPE_MachineTypeHeliosMLK2_CNDF == e_Type) )
			{
				for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
				{
					cpcSendCommandStr[ ss_i ] = SendCommandStr_HeliosMLK_CN_Exp_02[ ss_i ];
				}
			}
			else
			{
				for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
				{
					cpcSendCommandStr[ ss_i ] = SendCommandStr_Exp_02[ ss_i ];
				}
			}
		}	
//		if( TYPE_FuncVer_Expansion1 <= e_FuncVer )				// 機能バージョン2.0.
		else if( TYPE_FuncVer_Expansion1 <= e_FuncVer )				// 機能バージョン2.0.
// 2020/11/13 FPT-ThanhPC7 【IT6_2.1】[OP_BTS-28952]【管理NO.70】 end.
		{
			if( (TYPE_MachineTypeSparrow1SFP == e_Type) ||
				(TYPE_MachineTypeSparrow2SFP == e_Type) )
			{
				for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
				{
					cpcSendCommandStr[ ss_i ] = SendCommandStr_SparrowSFP_Exp_01[ ss_i ];
				}
			}
			else if( TYPE_MachineTypeSparrowBK0SFP == e_Type )
			{
				for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
				{
					cpcSendCommandStr[ ss_i ] = SendCommandStr_SparrowBkSFP[ ss_i ];
				}
			}
			else if( (TYPE_MachineTypeHeliosMLK3_CN == e_Type) ||
				 (TYPE_MachineTypeHeliosMLK2_5_CN == e_Type) ||
				 (TYPE_MachineTypeHeliosMLK2_5_CNDF == e_Type) ||
				 (TYPE_MachineTypeHeliosMLK2_CNDF == e_Type) )
			{
				for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
				{
				cpcSendCommandStr[ ss_i ] = SendCommandStr_HeliosMLK_CN[ ss_i ];
				}
			}
			else
			{
				for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
				{
					cpcSendCommandStr[ ss_i ] = SendCommandStr_Exp_01[ ss_i ];
				}
			}
		}
		else													// 機能バージョン1.0.
		{
			for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
			{
				if( (TYPE_MachineTypeSparrow1SFP == e_Type) ||
					(TYPE_MachineTypeSparrow2SFP == e_Type) )
				{
					cpcSendCommandStr[ ss_i ] = SendCommandStr_SparrowSFP[ ss_i ];
				}
				else
				{
					cpcSendCommandStr[ ss_i ] = SendCommandStr[ ss_i ];
				}
			}
		}

		//項目【1】(Sales count data)が選択されていた場合.
		if( us_CsrcCenterSendSetting & 0x0001 )
		{
			memcpy( pc_tmpCommand, cpcSendCommandStr[0], strlen( cpcSendCommandStr[0] ) );
			pc_tmpCommand += strlen( cpcSendCommandStr[0] );
		}

		//項目【2】(Error count data)が選択されていた場合.
		if( us_CsrcCenterSendSetting & 0x0002 )
		{
//2021/10/14 FPT-TinhNT16 【IT6-2.2MR1】 【制御構成変更_No.38】 Start.
			if( TYPE_CSRASettingComMode_OneWay == e_CSRACommMode )
			{
				string s_SendCommandStr;
				s_SendCommandStr.append(cpcSendCommandStr[1]);
				size_t found = s_SendCommandStr.find(" EN");
				if( found != string::npos ){
					s_SendCommandStr.replace(found, 3, "");
				}
				strncpy( (char*)pc_tmpCommand, s_SendCommandStr.c_str(), s_SendCommandStr.length() );

				pc_tmpCommand += s_SendCommandStr.length();
			} else {
				memcpy( pc_tmpCommand, cpcSendCommandStr[1], strlen( cpcSendCommandStr[1] ) );
				pc_tmpCommand += strlen( cpcSendCommandStr[1] );
			}
//2021/10/14 FPT-TinhNT16 【IT6-2.2MR1】 【制御構成変更_No.38】 End.

		}
		//項目【3】(Service count datar)が選択されていた場合.
		if( us_CsrcCenterSendSetting & 0x0004 )
		{
			memcpy( pc_tmpCommand, cpcSendCommandStr[2], strlen( cpcSendCommandStr[2] ) );
			pc_tmpCommand += strlen( cpcSendCommandStr[2] );
		}

		//項目【4】(Life count data Block, Life cycle data Block)が選択されていた場合.
		if( us_CsrcCenterSendSetting & 0x0008 )
		{
			memcpy( pc_tmpCommand, cpcSendCommandStr[3], strlen( cpcSendCommandStr[3] ) );
			pc_tmpCommand += strlen( cpcSendCommandStr[3] );
		}

		//項目【5】(CSRD-System data Block, Device config data)が選択されていた場合.
		if( us_CsrcCenterSendSetting & 0x0010 )
		{
			memcpy( pc_tmpCommand, cpcSendCommandStr[4], strlen( cpcSendCommandStr[4] ) );
			pc_tmpCommand += strlen( cpcSendCommandStr[4] );
		}
	
		//項目【6】(History data)が選択されていた場合.
		if( us_CsrcCenterSendSetting & 0x0020 )
		{
			memcpy( pc_tmpCommand, cpcSendCommandStr[5], strlen( cpcSendCommandStr[5] ) );
			pc_tmpCommand += strlen( cpcSendCommandStr[5] );
		}

		//項目【7】(EKC data Block)が選択されていた場合.
		//セキュリティ強化モードON時、DXコマンドはセンターへ通知できないため、.
		//セキュリティ強化モードON、かつセンターへの通知項目にDXコマンドが含まれていた場合、発信を中止する。.
		if(us_CsrcCenterSendSetting & 0x0040) 
		{
			if( True == isEnableStatus_ReadDX() )
			{
				memcpy( pc_tmpCommand, cpcSendCommandStr[6], strlen( cpcSendCommandStr[6] ) );
				pc_tmpCommand += strlen( cpcSendCommandStr[6] );
			}
			else
			{
				EPN_DBG_MESSAGE( (uchar*)"CSRC", (uint32_t)0, (uchar*)"DXコマンド送信条件に満たないため、DXコマンドの要求取り消し" );
			}
		}

		//項目【8】(Adjustment data Block)が選択されていた場合.
		if( us_CsrcCenterSendSetting & 0x0080 )
		{
			memcpy( pc_tmpCommand, cpcSendCommandStr[7], strlen( cpcSendCommandStr[7] ) );
			pc_tmpCommand += strlen( cpcSendCommandStr[7] );
		}

		//項目【9】(カバレッジ data Block)が選択されていた場合.
		if( us_CsrcCenterSendSetting & 0x0100 )
		{
			memcpy( pc_tmpCommand, cpcSendCommandStr[8], strlen( cpcSendCommandStr[8] ) );
			pc_tmpCommand += strlen( cpcSendCommandStr[8] );
		}
		//項目【10】(Component Information data Block)が選択されていた場合.
		if( us_CsrcCenterSendSetting & 0x0200 )
		{
			memcpy( pc_tmpCommand, cpcSendCommandStr[9], strlen( cpcSendCommandStr[9] ) );
			pc_tmpCommand += strlen( cpcSendCommandStr[9] );
		}

		if (us_CsrcCenterSendSetting & 0x0400)
		{
			memcpy(pc_tmpCommand, cpcSendCommandStr[10], strlen(cpcSendCommandStr[10]));
			pc_tmpCommand += strlen(cpcSendCommandStr[10]);
		}
	}

//2021/10/14 FPT-TinhNT16 【IT6-2.2MR1】 【制御構成変更_No.38】 Start.	
	if ( TYPE_CSRASettingComMode_OneWay == e_CSRACommMode ) {
		// +"ZO ZP" Add a command group to collect engine data and analysis, HW log.
		memcpy( pc_tmpCommand, " ZO ZP", strlen(" ZO ZP") );
		pc_tmpCommand += strlen(" ZO ZP");
	//2021/06/04 FPT-HaDV8 【IT6_2.2】【IT6_2.2-594】 Start.
		// + "ZU" add command Recovery tool diagnostic result information.
		if( TYPE_FuncVer_Expansion3 <= e_FuncVer )				// 機能バージョン2.2.
		{
			memcpy( pc_tmpCommand, " ZU", 3 );
			pc_tmpCommand += 3;
		// 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 Start.
			memcpy( pc_tmpCommand, " ZV", 3 );
			pc_tmpCommand += 3;
		// 2022/07/08 FPT-TinhNT16 【IT6_2.2MR3】【制御構成変更_No.43】 End.
		}
	//2021/06/04 FPT-HaDV8 【IT6_2.2】【IT6_2.2-594】 End.
	}
// 2022/08/18 【FPT-TinhNT16】 【IT6_2.3】【制御構成変更_No.62】 ソフトウェアDipSWの設定情報収集 Start.
	if( (TYPE_CSRASettingComMode_OneWay == e_CSRACommMode)
	 || (TYPE_CSRASettingComMode_OneWay_CN == e_CSRACommMode) )
	{
		if( TYPE_FuncVer_Expansion4 <= e_FuncVer )				// 機能バージョン2.3以降.
		{
			memcpy( pc_tmpCommand, " IA IB IC ID", strlen(" IA IB IC ID") );
			pc_tmpCommand += strlen(" IA IB IC ID");
		}
	}
// 2022/08/18 【FPT-TinhNT16】 【IT6_2.3】【制御構成変更_No.62】 ソフトウェアDipSWの設定情報収集 End.
//2021/10/14 FPT-TinhNT16 【IT6-2.2MR1】 【制御構成変更_No.38】 End.
	// 2020/12/24 FPT-ThanhPC7 【OP_BTS-30098】 Start.
	// + "S9" add command product recognize information.
	memcpy( pc_tmpCommand, " S9", 3 );
	pc_tmpCommand += 3;
	// 2020/12/24 FPT-ThanhPC7 【OP_BTS-30098】 End.
	// Get command resource.
	memcpy( pCmd, &c_TempCmd[0] , sizeof(c_TempCmd) );

}
// 2020/09/25 RQ-776 CWH 1-way通信方式の追加 end.

// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/22 Sky梶谷 START.
Bool RDSC_RdsCommMIO::createCsraReportFile( uchar *pc_Data )
{
	Bool result = False;

	// HDDアクセス開始
	RDSC_FileUtil::startHDDAccess();

	// 作成データをファイルに書き込む.
	string c_FilePath("");
	RDSC_CsraReportUtil::getDirPath( &c_FilePath );
	c_FilePath += RDSD_RptDataManagerDuringCreateFileName;

	// シリアルNo.の取得.
	uchar sc_TempSerialNo[ TYPD_MAX_SerialNoPrinter ] = {};
	APIC_RdsSetting::newInstance()->getMachineID( sc_TempSerialNo );
	string sc_SerialNo((const char*)sc_TempSerialNo);

	// 現在時刻の取得.
	time_t	s_CurrentTime = RDSC_CsraUtil::getCurrentTime();

	// ファイル名生成.
	string c_FileName;
	RDSC_CsraReportUtil::createFileName( sc_SerialNo, s_CurrentTime, &c_FileName );

	// 作成途中のファイルが残っていれば削除（作成中の電源断された場合など）.
	RDSC_CsraReportUtil::deleteFile( string(RDSD_RptDataManagerDuringCreateFileName) );

	// ファイル作成中.	
	RDSC_RptDataManager::newInstance()->updateFileStatus( c_FileName, RDSE_RptDataManagerFileState_DuringCreate );

	// ファイルオープン.
	FILE*	fp = NULL;
	fp = FILF_Fopen( c_FilePath.c_str(), "wb" );
	if ( NULL != fp ) {
		if( ERROR == FILF_Fwrite( (char*)pc_Data, static_cast<size_t>(strlen( (char*)pc_Data )), 1, fp ) ) {
			EPN_DBG_MESSAGE( (uchar*)"CSRC", 0, (uchar*)"FILF_Fwrite() Error" );
		}
		FILF_Fclose( fp );

		// リネームする.
		RDSC_CsraReportUtil::renameFile( string(RDSD_RptDataManagerDuringCreateFileName), c_FileName );

		// ファイルを新規登録する.
		RDSC_RptDataManager::newInstance()->NewFileRegistration( s_CurrentTime, c_FileName );

		result = True;													// 送信終了 

	} else {
		EPN_DBG_MESSAGE( (uchar*)"CSRC", 0, (uchar*)"FILF_Fopen() Error" );
	}

	// アイドル.
	RDSC_RptDataManager::newInstance()->updateFileStatus( c_FileName, RDSE_RptDataManagerFileState_Idle );

	// HDDアクセス終了.
	RDSC_FileUtil::endHDDAccess();

	return( result );
}

void RDSC_RdsCommMIO::tx_transCsraReportSq( void )
{
	bool    b_Trgflg = false;       // トリガ発信検出用フラグ 

	cancelTimer();
// 管理システム統合化開発 RQ-893(OP_BTS-32474) 2021/04/07 Sky梶谷 START.
	memset( e_WebDAVClient.uc_Data, 0, sizeof(e_WebDAVClient.uc_Data));									// バッファクリア 
// 管理システム統合化開発 RQ-893(OP_BTS-32474) 2021/04/07 Sky梶谷 END.

	// 応答発信[発信中]のときは、無視するようにする. 
	if ( False == theTrgMap.isExistOnGoingTrg( TYPE_RdsTrgKindRes_RDS ) ) {

		// MFP側からの発信要求があった場合（0 < ss_NumOfEventDataFiles）は、.
		// 特定のデータリード用コマンドを MFP側から自発的に送信する。       .
		for ( TYPE_RdsTrgKind e_RdsTrgKind = 0 ; TYPE_RdsTrgKindNumberOfTrg > e_RdsTrgKind ; e_RdsTrgKind++ ) {
			//下記発信は、MFPからの自発的な発信ではないのでトータルカラーカウンタは付与しない.
			//期日指定発信は自発的な発信だが、この関数内の、期日指定発信専用処理にてトータルカラーカウンタの要求コマンドを付与しているため、不要。
			if ( ( e_RdsTrgKind != TYPE_RdsTrgKindCenter )  &&
				 ( e_RdsTrgKind != TYPE_RdsTrgKindRes_RDS ) ) {
				b_Trgflg = theTrgMap.isExistOnGoingTrg( e_RdsTrgKind );
				if ( b_Trgflg ) {
					// 管理システム統合化開発 RQ-893(OP_BTS-32474) 2021/04/07 Sky梶谷 START.
					// 便乗発信により、前の送信でデータが送信されることがあるので、送信対象サブジェクトが存在しない時は送信の対象とはしない.
					if (( TYPE_RdsTrgKindWarnBody == e_RdsTrgKind ) && ( 0 == theMakeText.warnSubjectInfo )) {
						continue;
					}
					// 管理システム統合化開発 RQ-893(OP_BTS-32474) 2021/04/07 Sky梶谷 END.
					break;
				}
			}
		}
	}

    // トリガ発信対象が存在した場合（レポート発信用のファイルを作成する）.
	if ( b_Trgflg ) {

    	uchar uc_TmpHostCmd[2048] = {};
    	uint32_t ul_Length = 0;
			
		// "82 000000 "を付与.
	    const char uc_CsrcCycletmpFormat[] = "82 000000 ";
		memcpy( &uc_TmpHostCmd[0], &uc_CsrcCycletmpFormat[0], strlen( uc_CsrcCycletmpFormat ) );
			
		uchar	*puc_tmpCommand = &uc_TmpHostCmd[10];

		// RDフォーマットを付与.
		memcpy( puc_tmpCommand, (uchar*)"RD", 2 );	// +"RD".
		puc_tmpCommand += 2;
		ul_Length += 2;
			
		// トータルカラーカウンタを算出するコマンド群を付与.
		memcpy( puc_tmpCommand, (uchar*)" A1 B0 OC PC FC", 15 );
		puc_tmpCommand += 15;
		ul_Length += 15;
			
		// 警告発信の場合.
		if ( True == tskRdsEvent.isNowTrans( TYPE_RdsTrgKindWarnBody ) ) {
			if ( c_mkText_ModeWarn.b_JAMFrequencyFlag ) {
				memcpy( puc_tmpCommand, (uchar*)" Z0", 3 );	// +" Z0" 100JAM履歴情報付加.
				puc_tmpCommand += 3;
				ul_Length += 3;
			}
		}
		
		// 緊急発信の場合.
		if ( True == tskRdsEvent.isNowTrans( TYPE_RdsTrgKindTrbl ) ) {
			memcpy( puc_tmpCommand, (uchar*)" S7", 3 );	// +" S7" FWバージョン情報付加.
			puc_tmpCommand += 3;
			ul_Length += 3;
		}
			
		// コマンドの文字列長をセット.
    	char sc_Length[7] = {};
// [OP_BTS-36063] strcpy, strcat, sprintf対策 2021/10/27 Sky梶谷 START.
//		sprintf( &sc_Length[0], "%06X", ul_Length );
		snprintf( &sc_Length[0], sizeof(sc_Length), "%06X", ul_Length );
// [OP_BTS-36063] strcpy, strcat, sprintf対策 2021/10/27 Sky梶谷 END.
		memcpy( &uc_TmpHostCmd[3], &sc_Length[0], strlen( sc_Length ) );
			
		// エラーコード(エラーなし)、RD受信状態をセット.
    	RDSC_RdsEmail::RDSS_EmailCmdResult	e_cmdResult = {};
		e_cmdResult.ul_ErrCode = 0;						// エラーなし 
		e_cmdResult.ul_TypeCode = RDSD_ResType_ZR;		// RDを受信した状態 

		EPN_DBG_PRINTF_DIV( "[CommMIO::tx_transCsraReportSq]DBG ホストコマンド文字列 %s", uc_TmpHostCmd );
				
    	short	ss_NumOfCommandResFiles = 0;
    	short	ss_NumOfEventDataFiles = 0;
       	short   ss_NumOfAllFiles = 0;

		theRdsEmail.analizeHostCommand(
			RDSE_Email_CommModeRds,
			RDSE_AnaMode_OnlyRD,								// モード：RDコマンドのみ処理する。 
			uc_TmpHostCmd,										// ホストコマンド文字列 
			&e_cmdResult,										// 処置結果 
			&q_RD_Data,											// ReadCommand() にて読み込んだデータ 
			&ss_NumOfCommandResFiles,							// RD による応答ファイル数 
			&q_SendInfo											// 上記のファイルごとのファイルサイズ 
		);

		EPN_DBG_MESSAGE( (uchar*)"tx_transCsraReportSq()", (ushort)e_cmdResult.ul_ErrCode, (uchar*)" <<< e_cmdResult.ul_ErrCode" );

		ss_NumOfEventDataFiles = c_EvevntDataBuf.isNum();								// イベントデータファイル数 
		ss_NumOfAllFiles = (short)(ss_NumOfCommandResFiles + ss_NumOfEventDataFiles);

		EPN_DBG_MESSAGE((uchar*)"イベントファイル数", (uint32_t)ss_NumOfEventDataFiles, (uchar*)"");
		EPN_DBG_MESSAGE((uchar*)"RDファイル数", (uint32_t)ss_NumOfCommandResFiles, (uchar*)"");
	
		theRdsEmail.makeEmailData( e_NowCommMode );										// E-mailのヘッダーデータ作成、応答の確認で使用するデータ作成

		RDSC_CsrcConnectionManager::newInstance( RDSE_CommunicationType_Report )->changeNextState( RDSE_CsrcConnectionStateDetail_Interruption );	// Idleに戻す.
		RDSC_CsrcConnectionManager::newInstance( RDSE_CommunicationType_Report )->changeNextState( RDSE_CsrcConnectionStateDetail_Normal );			// 応答ファイル作成中に進める.

		Bool result = False;

		// E-mail情報作成.
		MioSendEmail		sendinfo;
		MioEmailSet			emset;
   		EmailAddrList		AddrTo = {"", "", NULL, NULL, 0};
    	EmailAddrList		AddrCc = {"", "", NULL, NULL, 0};
    	EmailAddrList		AddrBcc= {"", "", NULL, NULL, 0};
		MioMIMEBodyHeader*	pBodyHeader;
		if ( False != tx_makeMioSendInfoData(
						&sendinfo,
						&emset,
						&AddrTo,
						&AddrCc,
						&AddrBcc,
						&pBodyHeader,
						ss_NumOfEventDataFiles,
						ss_NumOfCommandResFiles,
						&q_SendInfo
						)
		) {
			// （４）mioSendData() テキスト送信 
        	int fd = ERROR;
			if( False != tx_transText( fd, ss_NumOfEventDataFiles, ss_NumOfCommandResFiles ) ) {
				
				// E-mail版フォーマットファイル作成.
				e_WebDAVClient.PutHeader( &emset );

				// レポート発信ファイルを作成する.
				result = createCsraReportFile( e_WebDAVClient.uc_Data );

			} else {
				EPN_DBG_MESSAGE( (uchar*)"CSRC", 0, (uchar*)"＜テキスト送信に失敗＞" );
			}

		} else {
			EPN_DBG_MESSAGE( (uchar*)"CSRC", 0, (uchar*)"＜E-mail情報作成に失敗＞" );
		}

		// バッファ開放.
		freeMIMEBodyHeader( emset.pMIMEBodyHeader );	// pBodyHeader. 

		if ( True == result ) {
			EPN_DBG_MESSAGE( (uchar*)"tx_transCsraReportSq", 0, (uchar*)"SetTimer起動" );			
			setTimer( 30*RDSD_Minute );		// 30分待ってみる(HTTP通信の最大遅延は300秒だが、フェールセーフとして30分にする). 
	    	transState = MIO_Trans_WaitTransEndMSG_State;	// 送信終了通知待ち 
		} else {
			if( MIO_Trans_WaitAnswer_State == transState ) {
				setTimer( 2*RDSD_Minute );	// ２分待ってみる.
				return;
			} else {
				stopComm( MLog_Nothing2, RDSE_ERR_MIO_TransCompleteERR, 0 );	// シーケンス破棄（１分後再開）
				return;
			}
		}

		// CSRA 片方向通信であれば、プレチェックを実施せず、直接、送信処理に移行する.
		RDSE_CsraCommMode e_CsraComm = RDSC_CsraUtil::getCsraCommMode();
		if ( RDSE_CsraCommMode_OneWay == e_CsraComm ) {
			tskRdsEvent.c_AutoReTrans.reset();			// 自動再発信停止状態のリセット.
			// 管理システム統合化開発 RQ-893 [OP_BTS-33048/33004] 2021/05/06 Sky梶谷 START.
			// [OP_BTS-32775]の対策を見直した.
//			// 管理システム統合化開発 RQ-893 [OP_BTS-32775] 2021/04/20 Sky大橋 START.
//			// トリガ発信対象が存在したままの場合、1回ファイル送信に失敗し2回目以降の再送信が行われる際、新たに同内容のCSRCレポートファイルが作成されてしまう.
//			// 双方向通信と同じく、ファイル作成完了後に通信関係の変数を全てクリアさせる.
//			theRdsEmail.emailTransNormalEndProc( RDSE_TSK_ID_MIO, e_NowCommMode );
//			stopComm( MLog_Tx_NormalEnd, RDSE_ERR_Email_Normal, 0 );
//			// 管理システム統合化開発 RQ-893 [OP_BTS-32775] 2021/04/20 Sky大橋 END.
			theTrgMap.transEnd();										/* 送信処理終了→[発信中]（発信が完了した）の要素を[済]に移動 */
			theTrgMap.resEnd();											/* 応答発信に関する発信マップの０をクリアする */
			// 管理システム統合化開発 RQ-893 [OP_BTS-33048/33004] 2021/05/06 Sky梶谷 END.
			RDSC_CsrcConnectionManager::newInstance( RDSE_CommunicationType_Report )->changeNextState( RDSE_CsrcConnectionStateDetail_Normal );			// プレチェック中に進める.
			RDSC_CsrcConnectionManager::newInstance( RDSE_CommunicationType_Report )->changeNextState( RDSE_CsrcConnectionStateDetail_Normal );			// 送信可能に進める.
			RDSC_CsrcConnectionManager::newInstance( RDSE_CommunicationType_Report )->sendStart();														// 送信開始に進める.
			// 管理システム統合化開発 RQ-893 [OP_BTS-32585] 2021/04/15 Sky大橋 START.
			theVRcontroller.m_c_CsraLog_Report.setLogHttpStartTime();
			// 管理システム統合化開発 RQ-893 [OP_BTS-32585] 2021/04/15 Sky大橋 END.
		}
	}

	// ステータス（レポート発信）が 応答ファイル作成中 であれば.
	if ( RDSE_CsrcConnectionState_MakeFile == RDSC_CsrcConnectionManager::newInstance( RDSE_CommunicationType_Report )->getState() ) {

		// 次回発信時刻を現在時刻として、プレチェックを実施する.
		time_t t_CurrentTime = RDSC_CsraUtil::getCurrentTime();												// 現在時刻の取得 time_t.
		tm t_CurrentTimeTm = {};
		if ( True == RDSC_TimeUtil::convertLocalTime( (const time_t)t_CurrentTime, &t_CurrentTimeTm ) ) {	// 現在時刻 time_t -> tm .
			TYPC_Time c_SendingTime;
			RDSC_TimeUtil::convertTmToTypeC( &t_CurrentTimeTm, &c_SendingTime );							// 現在時刻 tm -> TYPC_Time.

			// 次回送信（プレチェック）時刻を設定.
			APIC_CSRASetting::newInstance()->setNextSendingTimeReport( c_SendingTime );

			// プレチェック実施.
			RDSC_CsrcPreCheck::newInstance()->checkPreCheckTime();
		}
	}

	// ステータス（レポート発信）により処理を分岐させる.
	// 送信中　　：送信処理を実施する.
	// 送信中以外：通信関係の変数を全てクリアする.（発呼状態解除）.
	// ※通信中（発呼）状態を２つのステージに分けて考える.
	// 　１）通知（レポート）ファイル作成～プレチェック要求
	// 　２）プレチェック応答受信後のファイル送信処理.
	if ( True == RDSC_CsrcConnectionManager::newInstance( RDSE_CommunicationType_Report )->isSending() ) {
		tx_transCsraSq();
	}
	else {
		theRdsEmail.emailTransNormalEndProc( RDSE_TSK_ID_MIO, e_NowCommMode );
		stopComm( MLog_Tx_NormalEnd, RDSE_ERR_Email_Normal, 0 );
	}
}
// 管理システム統合化開発 RQ-811 要求仕様2-b 2020/12/22 Sky梶谷 END.
// 2019/07/23 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 ADD Start.
void printCSRC_SendCommandStr()
{
	printf("-*-*- SendCommandStr_Exp_01 -*-*-\n");

// 2019/08/27 SparrowBK OP_BTS-12328 機能毎コマンド文字列作成用テーブル不備対応 日立 江上 MOD Start.
	for( short ss_i = 0; ss_i < _S_Command_MAX_LEN; ss_i ++ )
// 2019/08/27 SparrowBK OP_BTS-12328 機能毎コマンド文字列作成用テーブル不備対応 日立 江上 MOD End.
	{
		printf("SendCommandStr_Exp_01[%02d] = %s\n", ss_i, SendCommandStr_Exp_01[ss_i]);
	}	

	printf("-* SendCommandStr_SparrowBkSFP *-\n");
	for( short ss_ii = 0; ss_ii < _S_Command_MAX_LEN; ss_ii ++ )
	{
		printf("SendCommandStr_SparrowBkSFP[%02d] = %s\n", ss_ii, SendCommandStr_SparrowBkSFP[ss_ii]);
	}

// 2019/09/17 【HeliosMLK】【Control No.1】【HELIOSMLK-73】 HIENG Start.
	printf("-* SendCommandStr_HeliosMLK_CN *-\n");
	for( ss_ii = 0; ss_ii < _S_Command_MAX_LEN; ss_ii ++ )
	{
		printf("SendCommandStr_HeliosMLK_CN[%02d] = %s\n", ss_ii, SendCommandStr_HeliosMLK_CN[ss_ii]);
	}
// 2019/09/17 【HeliosMLK】【Control No.1】【HELIOSMLK-73】 HIENG End.
	printf("-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-*-\n");
}
// 2019/07/23 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 ADD End.
