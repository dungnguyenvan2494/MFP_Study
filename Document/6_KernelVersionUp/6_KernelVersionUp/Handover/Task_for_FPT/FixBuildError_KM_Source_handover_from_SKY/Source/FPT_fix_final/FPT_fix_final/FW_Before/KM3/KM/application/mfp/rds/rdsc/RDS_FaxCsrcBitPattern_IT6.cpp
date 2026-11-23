//
//
//ファイル名：RDS_FaxCsrcBitPattern_IT6.cpp  
//
//概要：FAX-CSRCの読出し要求時のBitPatternとコマンドの対応を定義する。
//
/*=================================================================================
  変更履歴.
=================================================================================*/
/*_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/
_/
_/ 2014/03/05 ktakahashi	Zeus5BK対応 新規作成(ソースはZeusフォルダより流用)
_/ 2014.05.27 Wada [Zeus5BK] UV-01385(WP 1460,1466).
_/ 2014.06.24 ktakahashi	Zeus5BK 制御構成変更No.19(UPコマンド追加).
_/ 2014/07/11 ktakahashi ZeusS 制御構成変更No.1 機種立ち上げ.
_/ 2014/09/01 mtoyoda    ZeusS UV-01178（Exp_01[] の Bit 17 に コマンドを追加.
_/                             カラー機種の場合は、 Bit 17 に BJ PH PI PJ PK を追加.
_/                             モノクロ機種の場合は Bit 17 に BJ PH PK       を追加.
_/ 2015/03/05 M.Adachi   [Helios]KRQ-101 認証機能有効化対応（Exp_02[] の Bit 16 に UMコマンドを追加）.
_/ 2015/03/04 日立 森本郁	MAv3[RQ-250]CQ510602対応.
_/								Bit 19にTC SV APを追加.
_/								Bit 20にZD ZE ZRを追加.
_/ 2015/07/03 M.Adachi   [ZeusS_ZX0]RQ-250 機能カウンタの拡張（MAv3マージ対応）.
_/                              IT5 機能バージョン３．０用テーブルの、
_/								Bit 19にTC SV APを追加.
_/								Bit 20にZD ZE ZRを追加.
_/ 2015/11/05 ktakahashi ZeusS_ZX0 制御構成変更_No.76-2 ネットワークエラーカウンタ対応.
_/								Bit 18にENを追加.
_/ 2015/10/14 HCL_Pawan [VenusMLK] WP_46 WBS.2 機器構成変更に伴う対応
_/								IT5 機能バージョン３．０用テーブル.
_/								Bit 17(Remove Command BJ)
_/								Bit 22(Remove Command XC XF XG XH)
_/ 2015/12/18 HCL_Pawan [VenusMLK] WP_145 WBS.8 機能バージョン
_/ 2016/03/14 HCLT_Pawan VenusMLK CQ525067 Added AQ command in FAX Bit 19
_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/_/*/


#include "system/sys/OsWrapper/SYSC_OSWrapper.h"
#include "system/sys/APIC_System.h"

	 Bool	uc_RDS_BITDEFINE[] = {
		False,False,False,False,False,False,False,False,		// Bit 0 ～ Bit 7
		False,False,False,False,False,False,True,True,			// Bit 8 ～ Bit 15   2018/01/29 HCLT-Baba IT6_1.0  [Eagle] 制御構成変更_No.84  Modified.
		True, True, True, True, True, True, True, True,			// Bit 16 ～ Bit 23
		False,False,False,False,False,False,False,False };		// Bit 24 ～ Bit 31

	// IT5/IT6 機能バージョン１．０用テーブル.
	// uc_RDS_BITPATTERN_CD / uc_RDS_BITPATTERN_HIGHSECURITY_CD をベースに作成
	const char *uc_RDS_BITPATTERN[] = {
		" ", " ", " ", " ", " ", " ", " ", " ", 							// Bit 0 ～ Bit 7.
		" ", " ", " ", " ", " ", " ",									// Bit 8 ～ Bit 13.
// 2018/01/29 HCLT-Baba IT6_1.0  [Eagle] 制御構成変更_No.84  Start.		
// 2018/10/25 日立山口 AR1063812対応 Sparrow非対応コマンドを削除 Modify Start.
//2018/12/03【FPT-VuLH13】【IT6_2.0】【EAGLEH】 No.36_FAX通信管理レポートの改善 Added XR command Start.
#ifdef DEF_SparrowCus
// 2019/04/11 日立 森本(郁) [EagleH]OP_BTS-5208_FAXからの即時要求(全コマンド)で失敗する Start.
//		" XI XM XR",  															// Bit 14.
		" XI XM",  																// Bit 14.
#elif defined(DEF_EagleCus)
//		 " XI XJ XK XL XM XN XO XP XR",										// Bit 14.
		 " XI XJ XK XL XM XN XO XP",											// Bit 14.
// 2019/04/11 日立 森本(郁) [EagleH]OP_BTS-5208_FAXからの即時要求(全コマンド)で失敗する End.
#else
		" XI XJ XK XL XM XN XO XP",											// Bit 14.
#endif
//2018/12/03【FPT-VuLH13】【IT6_2.0】【EAGLEH】 No.36_FAX通信管理レポートの改善 Added XR command End.
// 2018/10/25 日立山口 AR1063812対応 Sparrow非対応コマンドを削除 Modify End.
// 2018/01/29 HCLT-Baba IT6_1.0  [Eagle] 制御構成変更_No.84  End.
		" DX", 																// Bit 15.
// 2017/10/23 HCLT-Vinay IT6_1.0 [Eagle] 制御構成変更_No.1	新規作成 Start.
#if defined(DEF_EagleCus) || defined(DEF_ZEUSS_CUS) || defined(DEF_ZEUSS_ZX0_CUS) || defined(DEF_ZEUSSBK12_CUS) || defined(DEF_ZEUSSBKX0_CUS) || \
	defined(DEF_ZEUSSBK2ND_12_CUS) || defined(DEF_ZEUSSBK2ND_ZX0_CUS)
		#ifdef DEF_A4_Cus
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 TK S8 S9 SA UL T4 T5 UP YO YP YQ UM", 	// Bit 16.
		#else
// 2019/03/04 y.hotta AR1071385 コマンド「TK01漢字」が正常終了してしまう Modify Start.
//		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK AM EG I0 I1 S7 TK S8 S9 SA UL T4 T5 UP YO YP YQ UM", 	// Bit 16.
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK AM EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 	// Bit 16.
		#endif
#else
// 2019/03/06 FPT-ChiNM [IT6_1.0MR2][1072141]【DenebMLK MR】Modification Start.
//		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 TK S8 S9 SA UL T4 T5 UP YO YP YQ UM", 	// Bit 16.

	#if defined(DEF_DenebMLKCus)
// 2019/03/29 FPT-DucDH4 [IT6_1.0][AR1071385] Modification Start.
		// " A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 TK S8 S9 SA UL T4 T5 YO YP YQ UM",		// Bit 16.
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL YO YP YQ UM", 	// Bit 16.
	#else
		// " A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 TK S8 S9 SA UL T4 T5 UP YO YP YQ UM",	// Bit 16.
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 	// Bit 16.
// 2019/03/29 FPT-DucDH4 [IT6_1.0][AR1071385] Modification End.
	#endif
// 2019/03/06 FPT-ChiNM [IT6_1.0MR2][1072141]【DenebMLK MR】Modification End.
#endif
// 2018/04/28 FPT-GiapNV IT6_1.0 [DenebMLK] 制御構成変更_No.1_製品名対応  Start
#if  defined(DEF_MonoCus) || defined(DEF_DenebMLKCus)
//		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 T1 T2 T3 BJ PH PK", 									// Bit 17.
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PK", 									// Bit 17.
// 2018/04/28 FPT-GiapNV IT6_1.0 [DenebMLK] 制御構成変更_No.1_製品名対応  End
#elif DEF_A4_Cus
//		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 T1 T2 T3 PH PI PJ PK", 								// Bit 17.
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 PH PI PJ PK", 								// Bit 17.
#else
//		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 T1 T2 T3 BJ PH PI PJ PK", 							// Bit 17.
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PI PJ PK", 							// Bit 17.
// 2019/03/04 y.hotta AR1071385 コマンド「TK01漢字」が正常終了してしまう Modify End.
#endif
#ifdef DEF_A4_Cus
		" E0 E1 E3 E4 E5 J0 EN", 												// Bit 18.	//2018/05/23 【IT6_1.0】【AR-1046539】HCLT-Jahnavi modified.
#else
		" E0 E3 E4 E5 J0 EN", 													// Bit 18.	//2018/05/23 【IT6_1.0】【AR-1046539】HCLT-Jahnavi modified.
#endif
// 2018/12/13 FPT-LinhVT3 【IT6_1.0】【DenebMLK】 AR1068962 Modification Start.
#ifdef DEF_DenebMLKCus
		" BX F0 F1 G2 H1 H2 HC TC SV AP AQ",											// Bit 19.
#else
		" BX F0 F1 G2 H1 H2 HC V5 TC SV AP AQ", 										// Bit 19.
#endif
// 2018/12/13 FPT-LinhVT3 【IT6_1.0】【DenebMLK】 AR1068962 Modification End.
// 2019/03/12 FPT-LinhVT3 【IT6_2.0】【EagleH】 OP_BTS_3339 Modification Start.
// 2019/06/27 y.hotta EagleH OP_BTS-7025 SparrowMFPのFAX通信で全コマンド取得が正常終了しない Modify Start.
//#ifdef DEF_EagleCus
//		" Z0 Z1 Z8 ZA ZG ZD ZE ZR ZC", 												// Bit 20.
//#else
		" Z0 Z1 Z8 Z9 ZA ZG ZD ZE ZR ZC", 											// Bit 20.
//#endif
// 2019/06/27 y.hotta EagleH OP_BTS-7025 SparrowMFPのFAX通信で全コマンド取得が正常終了しない Modify End.
// 2019/03/12 FPT-LinhVT3 【IT6_2.0】【EagleH】 OP_BTS_3339 Modification End.
// 2018/04/28 FPT-GiapNV IT6_1.0 [DenebMLK] 制御構成変更_No.1_製品名対応 Start
#if  defined(DEF_MonoCus) || defined(DEF_DenebMLKCus)
		" AX AY AZ V2 V4 I9 V6 V7 VA VB VD VE", 							// Bit 21.
// 2018/04/28 FPT-GiapNV IT6_1.0 [DenebMLK] 制御構成変更_No.1_製品名対応 End
#else
		" AX AY AZ V2 V3 V4 I9 V6 V7 VA VB VD VE",							// Bit 21.
#endif
#ifdef DEF_A4_Cus
		" X0 X1 X2 XB XE",													// Bit 22.
#else
		" X0 X1 X2 XB XC XE XF XG XH",										// Bit 22.
#endif
		" AR AS AT AU", 													// Bit 23.
		" ", " ", " ", " ", " ", " ", " ", " " };							// Bit 24 ～ Bit 31.

	const char *uc_RDS_BITPATTERN_HIGHSECURITY[] = {
		" ", " ", " ", " ", " ", " ", " ", " ", 							// Bit 0 ～ Bit 7.
// 2018/01/29 HCLT-Baba IT6_1.0  [Eagle] 制御構成変更_No.84  Start.
		" ", " ", " ", " ", " ", " ", 										// Bit 8 ～ Bit 13.
//2018/12/03【FPT-VuLH13】【IT6_2.0】【EAGLEH】 No.36_FAX通信管理レポートの改善 Added XR command Start.
// 2018/10/25 日立山口 AR1063812対応 Sparrow非対応コマンドを削除 Modify Start.
#ifdef DEF_SparrowCus
// 2019/04/11 日立 森本(郁) [EagleH]OP_BTS-5208_FAXからの即時要求(全コマンド)で失敗する Start.
//		" XI XM XR",  															// Bit 14.
		" XI XM",  																// Bit 14.
#elif defined(DEF_EagleCus)
//		" XI XJ XK XL XM XN XO XP XR",  										// Bit 14.
		" XI XJ XK XL XM XN XO XP",		  										// Bit 14.
// 2019/04/11 日立 森本(郁) [EagleH]OP_BTS-5208_FAXからの即時要求(全コマンド)で失敗する End.
#else
		" XI XJ XK XL XM XN XO XP",  											// Bit 14.
#endif
// 2018/10/25 日立山口 AR1063812対応 Sparrow非対応コマンドを削除 Modify End.
//2018/12/03【FPT-VuLH13】【IT6_2.0】【EAGLEH】 No.36_FAX通信管理レポートの改善 Added XR command End.
		" ", 																// Bit 15.
// 2018/01/29 HCLT-Baba IT6_1.0  [Eagle] 制御構成変更_No.84  End.
#if defined(DEF_EagleCus) || defined(DEF_ZEUSS_CUS) || defined(DEF_ZEUSS_ZX0_CUS) || defined(DEF_ZEUSSBK12_CUS) || defined(DEF_ZEUSSBKX0_CUS) || \
	defined(DEF_ZEUSSBK2ND_12_CUS) || defined(DEF_ZEUSSBK2ND_ZX0_CUS)
		#ifdef DEF_A4_Cus
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 TK S8 S9 SA UL T4 T5 UP YO YP YQ UM", 	// Bit 16.
		#else
// 2019/03/04 y.hotta AR1071385 コマンド「TK01漢字」が正常終了してしまう Modify Start.
//		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK AM EG I0 I1 S7 TK S8 S9 SA UL T4 T5 UP YO YP YQ UM", 	// Bit 16.
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK AM EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 	// Bit 16.
		#endif
#else
// 2019/03/06 FPT-ChiNM [IT6_1.0MR2][1072141]【DenebMLK MR】Modification Start.
//		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 TK S8 S9 SA UL T4 T5 UP YO YP YQ UM", 	// Bit 16.
	#if defined(DEF_DenebMLKCus)
// 2019/03/29 FPT-DucDH4 [IT6_1.0][AR1071385] Modification Start.
		// " A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 TK S8 S9 SA UL T4 T5 YO YP YQ UM",		// Bit 16.
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL YO YP YQ UM", 	// Bit 16.
	#else
		// " A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 TK S8 S9 SA UL T4 T5 UP YO YP YQ UM",	// Bit 16.
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 	// Bit 16.
	#endif
// 2019/03/29 FPT-DucDH4 [IT6_1.0][AR1071385] Modification End.
// 2019/03/06 FPT-ChiNM [IT6_1.0MR2][1072141]【DenebMLK MR】Modification End.
#endif
// 2018/04/28 FPT-GiapNV IT6_1.0 [DenebMLK] 制御構成変更_No.1_製品名対応  Start
#if  defined(DEF_MonoCus) || defined(DEF_DenebMLKCus)
//		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 T1 T2 T3 BJ PH PK",		 							// Bit 17.
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PK",		 							// Bit 17.
// 2018/04/28 FPT-GiapNV IT6_1.0 [DenebMLK] 制御構成変更_No.1_製品名対応  End
#elif DEF_A4_Cus
//		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 T1 T2 T3 PH PI PJ PK", 								// Bit 17.
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 PH PI PJ PK", 								// Bit 17.
#else
//		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 T1 T2 T3 BJ PH PI PJ PK", 							// Bit 17.
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PI PJ PK", 							// Bit 17.
// 2019/03/04 y.hotta AR1071385 コマンド「TK01漢字」が正常終了してしまう Modify End.
#endif
#ifdef DEF_A4_Cus
		" E0 E1 E3 E4 E5 J0 EN", 												// Bit 18.	//2018/05/23 【IT6_1.0】【AR-1046539】HCLT-Jahnavi modified.
#else
		" E0 E4 E3 E5 J0 EN", 													// Bit 18.	//2018/05/23 【IT6_1.0】【AR-1046539】HCLT-Jahnavi modified.
#endif
// 2018/12/13 FPT-LinhVT3 【IT6_1.0】【DenebMLK】 AR1068962 Modification Start.
#ifdef DEF_DenebMLKCus
		" BX F0 F1 G2 H1 H2 HC TC SV AP AQ",										// Bit 19.
#else
		" BX F0 F1 G2 H1 H2 HC V5 TC SV AP AQ", 									// Bit 19.
#endif
// 2018/12/13 FPT-LinhVT3 【IT6_1.0】【DenebMLK】 AR1068962 Modification End.
// 2019/03/12 FPT-LinhVT3 【IT6_2.0】【EagleH】 OP_BTS_3339 Modification Start.
// 2019/06/27 y.hotta EagleH OP_BTS-7025 SparrowMFPのFAX通信で全コマンド取得が正常終了しない Modify Start.
//#ifdef DEF_EagleCus
//		" Z0 Z1 Z8 ZA ZG ZD ZE ZR ZC", 												// Bit 20.
//#else
		" Z0 Z1 Z8 Z9 ZA ZG ZD ZE ZR ZC", 											// Bit 20.
//#endif
// 2019/06/27 y.hotta EagleH OP_BTS-7025 SparrowMFPのFAX通信で全コマンド取得が正常終了しない Modify End.
// 2019/03/12 FPT-LinhVT3 【IT6_2.0】【EagleH】 OP_BTS_3339 Modification End.
// 2018/04/28 FPT-GiapNV IT6_1.0 [DenebMLK] 制御構成変更_No.1_製品名対応  Start
#if  defined(DEF_MonoCus) || defined(DEF_DenebMLKCus)
		" AX AY AZ V2 V4 I9 V6 V7 VA VB VD VE", 							// Bit 21.
// 2018/04/28 FPT-GiapNV IT6_1.0 [DenebMLK] 制御構成変更_No.1_製品名対応  End
#else
		" AX AY AZ V2 V3 V4 I9 V6 V7 VA VB VD VE", 							// Bit 21.
#endif
#ifdef DEF_A4_Cus
		" X0 X1 X2 XB XE",													// Bit 22.
#else
		" X0 X1 X2 XB XC XE XF XG XH",										// Bit 22.
#endif
// 2017/10/23 HCLT-Vinay IT6_1.0 [Eagle] 制御構成変更_No.1	新規作成 End.
		" AR AS AT AU", 													// Bit 23.
		" ", " ", " ", " ", " ", " ", " ", " " };							// Bit 24 ～ Bit 31.

// 2018/09/26 FPT-LucNH2 【 IT6_2.0 】【EagleH】【制御構成変更_No.4】 Start.
	// IT6 機能バージョン 2.0 用テーブル.
	// uc_RDS_BITPATTERN_CD / uc_RDS_BITPATTERN_HIGHSECURITY_CD をベースに作成
	const char *uc_RDS_BITPATTERN_Exp_01[] = {
		" ", " ", " ", " ", " ", " ", " ", " ", 							// Bit 0 ～ Bit 7.
		" ", " ", " ", " ", " ", " ",										// Bit 8 ～ Bit 13.
// 2018/01/29 HCLT-Baba IT6_1.0  [Eagle] 制御構成変更_No.84  Start.		
// 2018/07/27 FPT-LucNH2 IT6_1.0  [Eagle] 機器構成変更_No.92 (#3) Start.
//2018/12/03【FPT-VuLH13】【IT6_2.0】【EAGLEH】 No.36_FAX通信管理レポートの改善 Added XR command Start.
// 2019/06/27 y.hotta EagleH OP_BTS-7025 SparrowMFPのFAX通信で全コマンド取得が正常終了しない Modify Start.
// 2019/12/25 FPT-LucNH2 【IT6_2.0】 OP_BTS-14653  Modification Start.
#if defined(DEF_SparrowCus) || defined(DEF_HeliosMLKCus)
		" XI XM XR",  																// Bit 14.
// 2019/12/25 FPT-LucNH2 【IT6_2.0】 OP_BTS-14653  Modification End.
#elif defined(DEF_EagleCus)
// 2019/06/27 y.hotta EagleH OP_BTS-7025 SparrowMFPのFAX通信で全コマンド取得が正常終了しない Modify End.
		 " XI XJ XK XL XM XN XO XP XR",								// Bit 14.
#else
		 " XI XJ XK XL XM XN XO XP XR",										// Bit 14.
#endif
//2018/12/03【FPT-VuLH13】【IT6_2.0】【EAGLEH】 No.36_FAX通信管理レポートの改善 Added XR command End.
// 2018/07/27 FPT-LucNH2 IT6_1.0  [Eagle] 機器構成変更_No.92 (#3) End.
// 2018/01/29 HCLT-Baba IT6_1.0  [Eagle] 制御構成変更_No.84  End.
		" DX", 																// Bit 15.
// 2017/10/23 HCLT-Vinay IT6_1.0 [Eagle] 制御構成変更_No.1	新規作成 Start.
#if defined(DEF_EagleCus) || defined(DEF_ZEUSS_CUS) || defined(DEF_ZEUSS_ZX0_CUS) || defined(DEF_ZEUSSBK12_CUS) || defined(DEF_ZEUSSBKX0_CUS) || \
	defined(DEF_ZEUSSBK2ND_12_CUS) || defined(DEF_ZEUSSBK2ND_ZX0_CUS)
		#ifdef DEF_A4_Cus
// 2019/04/11 日立 森本(郁) [EagleH]OP_BTS-5208_FAXからの即時要求(全コマンド)で失敗する Start.
//		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 TK S8 S9 SA UL T4 T5 UP YO YP YQ UM", 		// Bit 16.
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 		// Bit 16.
// 2019/11/21 【HeliosMLK】【Machine No.23】【HELIOSMLK-1155】 HIENG Start.
		#elif defined(DEF_HeliosMLKCus)
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 		// Bit 16.
// 2019/11/21 【HeliosMLK】【Machine No.23】【HELIOSMLK-1155】 HIENG End.
		#else
//		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK AM EG I0 I1 S7 TK S8 S9 SA UL T4 T5 UP YO YP YQ UM", 	// Bit 16.
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK AM EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 	// Bit 16.
		#endif
#else
// 2019/06/27 y.hotta EagleH OP_BTS-7025 SparrowMFPのFAX通信で全コマンド取得が正常終了しない Modify Start.
//		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 TK S8 S9 SA UL T4 T5 UP YO YP YQ UM", 		// Bit 16.
//		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK AM EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 	// Bit 16.
	#if defined(DEF_DenebMLKCus)
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL YO YP YQ UM", 	// Bit 16.
	#else
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM",	// Bit 16.  //2019/07/11 y.hotta EagleH OP_BTS-7025 Bit 23で失敗する Modify.
	#endif
// 2019/06/27 y.hotta EagleH OP_BTS-7025 SparrowMFPのFAX通信で全コマンド取得が正常終了しない Modify End.
// 2019/04/11 日立 森本(郁) [EagleH]OP_BTS-5208_FAXからの即時要求(全コマンド)で失敗する End.
#endif

#ifdef DEF_MonoCus
// 2019/06/21 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 MOD Start.
// 2019/04/11 日立 森本(郁) [EagleH]OP_BTS-5208_FAXからの即時要求(全コマンド)で失敗する Start.
	#if defined(DEF_SparrowBKCus)
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 PH PK", 									// Bit 17.
	#else
//		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 T1 T2 T3 BJ PH PK", 									// Bit 17.
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PK", 									// Bit 17.
	#endif
// 2019/06/21 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 MOD End.

#elif DEF_A4_Cus
//		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 T1 T2 T3 PH PI PJ PK", 								// Bit 17.
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 PH PI PJ PK", 								// Bit 17.
#else
//		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 T1 T2 T3 BJ PH PI PJ PK", 							// Bit 17.
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PI PJ PK", 							// Bit 17.
// 2019/04/11 日立 森本(郁) [EagleH]OP_BTS-5208_FAXからの即時要求(全コマンド)で失敗する End.
#endif
#ifdef DEF_A4_Cus
		" E0 E1 E3 E4 E5 J0 EN", 											// Bit 18.	//2018/05/23 【IT6_1.0】【AR-1046539】HCLT-Jahnavi modified.
#else
		" E0 E3 E4 E5 J0 EN", 												// Bit 18.	//2018/05/23 【IT6_1.0】【AR-1046539】HCLT-Jahnavi modified.
#endif
		" BX F0 F1 G2 H1 H2 HC V5 TC SV AP AQ", 							// Bit 19.
// 2019/04/11 FPT-SangTX 【IT6_2.0】 OP_BTS-4908  Modification Start.
// 2019/03/12 FPT-LinhVT3 【IT6_2.0】【EagleH】 OP_BTS_3339 Modification Start.
//#ifdef DEF_EagleCus
//		" Z0 Z1 Z8 ZA ZG ZD ZE ZR ZC", 										// Bit 20.
//#else
		" Z0 Z1 Z8 Z9 ZA ZG ZD ZE ZR ZC", 									// Bit 20.
//#endif
// 2019/03/12 FPT-LinhVT3 【IT6_2.0】【EagleH】 OP_BTS_3339 Modification End.
// 2019/04/11 FPT-SangTX 【IT6_2.0】 OP_BTS-4908  Modification End.
#ifdef DEF_MonoCus
		" AX AY AZ V2 V4 I9 V6 V7 VA VB VD VE", 							// Bit 21.
#else
		" AX AY AZ V2 V3 V4 I9 V6 V7 VA VB VD VE",							// Bit 21.
#endif
// 2019/12/25 FPT-LucNH2 【IT6_2.0】 OP_BTS-14653  Modification Start.
#if defined(DEF_A4_Cus) || defined(DEF_HeliosMLKCus)
		" X0 X1 X2 XB XE",													// Bit 22.
// 2019/12/25 FPT-LucNH2 【IT6_2.0】 OP_BTS-14653  Modification End.
#else
		" X0 X1 X2 XB XC XE XF XG XH",										// Bit 22.
#endif
		" AR AS AT AU", 													// Bit 23.
		" ", " ", " ", " ", " ", " ", " ", " " };							// Bit 24 ～ Bit 31.

	const char *uc_RDS_BITPATTERN_HIGHSECURITY_Exp_01[] = {
		" ", " ", " ", " ", " ", " ", " ", " ", 							// Bit 0 ～ Bit 7.
// 2018/01/29 HCLT-Baba IT6_1.0  [Eagle] 制御構成変更_No.84  Start.
		" ", " ", " ", " ", " ", " ", 										// Bit 8 ～ Bit 13.
// 2018/07/27 FPT-LucNH2 IT6_1.0  [Eagle] 機器構成変更_No.92 (#3) Start.
//2018/12/03【FPT-VuLH13】【IT6_2.0】【EAGLEH】 No.36_FAX通信管理レポートの改善 Added XR command Start.
// 2019/06/27 y.hotta EagleH OP_BTS-7025 SparrowMFPのFAX通信で全コマンド取得が正常終了しない Modify Start.
// 2019/12/25 FPT-LucNH2 【IT6_2.0】 OP_BTS-14653  Modification Start.
#if defined(DEF_SparrowCus) || defined(DEF_HeliosMLKCus)
		" XI XM XR",  																// Bit 14.
// 2019/12/25 FPT-LucNH2 【IT6_2.0】 OP_BTS-14653  Modification End.
#elif defined(DEF_EagleCus)
// 2019/06/27 y.hotta EagleH OP_BTS-7025 SparrowMFPのFAX通信で全コマンド取得が正常終了しない Modify End.
		" XI XJ XK XL XM XN XO XP XR",  							// Bit 14.
#else
		" XI XJ XK XL XM XN XO XP XR",  										// Bit 14.
#endif
//2018/12/03【FPT-VuLH13】【IT6_2.0】【EAGLEH】 No.36_FAX通信管理レポートの改善 Added XR command End.
// 2018/07/27 FPT-LucNH2 IT6_1.0  [Eagle] 機器構成変更_No.92 (#3) End.
		" ", 																// Bit 15.
// 2018/01/29 HCLT-Baba IT6_1.0  [Eagle] 制御構成変更_No.84  End.
#if defined(DEF_EagleCus) || defined(DEF_ZEUSS_CUS) || defined(DEF_ZEUSS_ZX0_CUS) || defined(DEF_ZEUSSBK12_CUS) || defined(DEF_ZEUSSBKX0_CUS) || \
	defined(DEF_ZEUSSBK2ND_12_CUS) || defined(DEF_ZEUSSBK2ND_ZX0_CUS)
		#ifdef DEF_A4_Cus
// 2019/04/11 日立 森本(郁) [EagleH]OP_BTS-5208_FAXからの即時要求(全コマンド)で失敗する Start.
//		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 TK S8 S9 SA UL T4 T5 UP YO YP YQ UM", 		// Bit 16.
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 		// Bit 16.
// 2019/11/21 【HeliosMLK】【Machine No.23】【HELIOSMLK-1155】 HIENG Start.
		#elif defined(DEF_HeliosMLKCus)
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 		// Bit 16.
// 2019/11/21 【HeliosMLK】【Machine No.23】【HELIOSMLK-1155】 HIENG End.
		#else
//		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK AM EG I0 I1 S7 TK S8 S9 SA UL T4 T5 UP YO YP YQ UM", 	// Bit 16.
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK AM EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 	// Bit 16.
		#endif
#else
// 2019/06/27 y.hotta EagleH OP_BTS-7025 SparrowMFPのFAX通信で全コマンド取得が正常終了しない Modify Start.
//		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 TK S8 S9 SA UL T4 T5 UP YO YP YQ UM", 		// Bit 16.
//		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 		// Bit 16.
	#if defined(DEF_DenebMLKCus)
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL YO YP YQ UM", 	// Bit 16.
	#else
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM",	// Bit 16.  //2019/07/11 y.hotta EagleH OP_BTS-7025 Bit 23で失敗する Modify.
	#endif
// 2019/06/27 y.hotta EagleH OP_BTS-7025 SparrowMFPのFAX通信で全コマンド取得が正常終了しない Modify End.
// 2019/04/11 日立 森本(郁) [EagleH]OP_BTS-5208_FAXからの即時要求(全コマンド)で失敗する End.
#endif
#ifdef DEF_MonoCus
// 2019/06/21 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 MOD Start.
	#if defined(DEF_SparrowBKCus)
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 PH PK", 									// Bit 17.
	#else
//		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 T1 T2 T3 BJ PH PK",		 							// Bit 17.
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PK",		 									// Bit 17.
	#endif
// 2019/06/21 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 MOD End.
#elif DEF_A4_Cus
//		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 T1 T2 T3 PH PI PJ PK", 								// Bit 17.
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 PH PI PJ PK", 										// Bit 17.
#else
//		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 T1 T2 T3 BJ PH PI PJ PK", 							// Bit 17.
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PI PJ PK", 										// Bit 17.
// 2019/04/11 日立 森本(郁) [EagleH]OP_BTS-5208_FAXからの即時要求(全コマンド)で失敗する End.
#endif
#ifdef DEF_A4_Cus
		" E0 E1 E3 E4 E5 J0 EN", 											// Bit 18.	//2018/05/23 【IT6_1.0】【AR-1046539】HCLT-Jahnavi modified.
#else
		" E0 E4 E3 E5 J0 EN", 												// Bit 18.	//2018/05/23 【IT6_1.0】【AR-1046539】HCLT-Jahnavi modified.
#endif
		" BX F0 F1 G2 H1 H2 HC V5 TC SV AP AQ", 							// Bit 19.
// 2019/04/11 FPT-SangTX 【IT6_2.0】 OP_BTS-4908  Modification Start.
// 2019/03/12 FPT-LinhVT3 【IT6_2.0】【EagleH】 OP_BTS_3339 Modification Start.
//#ifdef DEF_EagleCus
//		" Z0 Z1 Z8 ZA ZG ZD ZE ZR ZC", 										// Bit 20.
//#else
		" Z0 Z1 Z8 Z9 ZA ZG ZD ZE ZR ZC", 									// Bit 20.
//#endif
// 2019/03/12 FPT-LinhVT3 【IT6_2.0】【EagleH】 OP_BTS_3339 Modification End.
// 2019/04/11 FPT-SangTX 【IT6_2.0】 OP_BTS-4908  Modification End.
#ifdef DEF_MonoCus
		" AX AY AZ V2 V4 I9 V6 V7 VA VB VD VE", 							// Bit 21.
#else
		" AX AY AZ V2 V3 V4 I9 V6 V7 VA VB VD VE", 							// Bit 21.
#endif
// 2019/12/25 FPT-LucNH2 【IT6_2.0】 OP_BTS-14653  Modification Start.
#if defined(DEF_A4_Cus) || defined(DEF_HeliosMLKCus)
		" X0 X1 X2 XB XE",													// Bit 22.
// 2019/12/25 FPT-LucNH2 【IT6_2.0】 OP_BTS-14653  Modification End.
#else
		" X0 X1 X2 XB XC XE XF XG XH",										// Bit 22.
#endif
// 2017/10/23 HCLT-Vinay IT6_1.0 [Eagle] 制御構成変更_No.1	新規作成 End.
		" AR AS AT AU", 													// Bit 23.
		" ", " ", " ", " ", " ", " ", " ", " " };							// Bit 24 ～ Bit 31.
/// 2018/09/26 FPT-LucNH2 【 IT6_2.0 】【EagleH】【制御構成変更_No.4】 End.

//2020/07/01 FPT-TinhNT16 【 IT6_2.1 】 制御構成変更_No.4 start.
	// IT6 機能バージョン 2.1 用テーブル.
	// uc_RDS_BITPATTERN_CD / uc_RDS_BITPATTERN_HIGHSECURITY_CD をベースに作成.
	const char *uc_RDS_BITPATTERN_Exp_02[] = {
		" ", " ", " ", " ", " ", " ", " ", " ", 													// Bit 0 ～ Bit 7.
		" ", " ", " ", " ", " ", " ",																// Bit 8 ～ Bit 13.
#if defined(DEF_SparrowCus) || defined(DEF_HeliosMLKCus)
		" XI XM XR",  																				// Bit 14.
#elif defined(DEF_EagleCus)
		 " XI XJ XK XL XM XN XO XP XR",																// Bit 14.
#else
		 " XI XJ XK XL XM XN XO XP XR",																// Bit 14.
#endif
		" DX", 																						// Bit 15.
#if defined(DEF_EagleCus) || defined(DEF_ZEUSS_CUS) || defined(DEF_ZEUSS_ZX0_CUS) || defined(DEF_ZEUSSBK12_CUS) || defined(DEF_ZEUSSBKX0_CUS) || \
	defined(DEF_ZEUSSBK2ND_12_CUS) || defined(DEF_ZEUSSBK2ND_ZX0_CUS)
		#ifdef DEF_A4_Cus
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 		// Bit 16.
		#elif defined(DEF_HeliosMLKCus)
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 		// Bit 16.
		#else
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK AM EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 	// Bit 16.
		#endif
#else
	#if defined(DEF_DenebMLKCus)
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL YO YP YQ UM", 			// Bit 16.
	#else
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM",		// Bit 16. 
	#endif
#endif
#ifdef DEF_MonoCus
	#if defined(DEF_SparrowBKCus)
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 PH PK", 												// Bit 17.
	#else
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PK", 											// Bit 17.
	#endif
#elif defined DEF_A4_Cus
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 PH PI PJ PK", 										// Bit 17.
#else
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PI PJ PK", 										// Bit 17.
#endif
#ifdef DEF_A4_Cus
		" E0 E1 E3 E4 E5 J0 EN", 																	// Bit 18.
#else
		" E0 E3 E4 E5 J0 EN", 																		// Bit 18.
#endif
		" BX F0 F1 G2 H1 H2 HC V5 TC SV AP AQ", 													// Bit 19.
//2020/11/24 FPT-TinhNT16 [CSRC] 【制御構成変更_No.81】 Start.
#ifdef DEF_EagleCus
		" Z0 Z1 Z8 Z9 ZA ZG ZD ZE ZR ZC ZT", 														// Bit 20.
#else
		" Z0 Z1 Z8 Z9 ZA ZG ZD ZE ZR ZC", 															// Bit 20.
#endif
//2020/11/24 FPT-TinhNT16 [CSRC] 【制御構成変更_No.81】 End.
#ifdef DEF_MonoCus
		" AX AY AZ V2 V4 I9 V6 V7 VA VB VD VE", 													// Bit 21.
#else
		" AX AY AZ V2 V3 V4 I9 V6 V7 VA VB VD VE",													// Bit 21.
#endif
#if defined(DEF_A4_Cus) || defined(DEF_HeliosMLKCus)
		" X0 X1 X2 XB XE",																			// Bit 22.
#else
		" X0 X1 X2 XB XC XE XF XG XH",																// Bit 22.
#endif
		" AR AS AT AU", 																			// Bit 23.
		" ", " ", " ", " ", " ", " ", " ", " " };													// Bit 24 ～ Bit 31.

	const char *uc_RDS_BITPATTERN_HIGHSECURITY_Exp_02[] = {
		" ", " ", " ", " ", " ", " ", " ", " ", 													// Bit 0 ～ Bit 7.
		" ", " ", " ", " ", " ", " ", 																// Bit 8 ～ Bit 13.
#if defined(DEF_SparrowCus) || defined(DEF_HeliosMLKCus)
		" XI XM XR",  																				// Bit 14.
#elif defined(DEF_EagleCus)
		" XI XJ XK XL XM XN XO XP XR",  															// Bit 14.
#else
		" XI XJ XK XL XM XN XO XP XR",  															// Bit 14.
#endif
		" ", 																						// Bit 15.
#if defined(DEF_EagleCus) || defined(DEF_ZEUSS_CUS) || defined(DEF_ZEUSS_ZX0_CUS) || defined(DEF_ZEUSSBK12_CUS) || defined(DEF_ZEUSSBKX0_CUS) || \
	defined(DEF_ZEUSSBK2ND_12_CUS) || defined(DEF_ZEUSSBK2ND_ZX0_CUS)
		#ifdef DEF_A4_Cus
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 		// Bit 16.
		#elif defined(DEF_HeliosMLKCus)
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 		// Bit 16.
		#else
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK AM EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 	// Bit 16.
		#endif
#else
	#if defined(DEF_DenebMLKCus)
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL YO YP YQ UM", 			// Bit 16.
	#else
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM",		// Bit 16.
	#endif
#endif
#ifdef DEF_MonoCus
	#if defined(DEF_SparrowBKCus)
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 PH PK", 												// Bit 17.
	#else
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PK",		 									// Bit 17.
	#endif
#elif defined DEF_A4_Cus
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 PH PI PJ PK", 										// Bit 17.
#else
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PI PJ PK", 										// Bit 17.
#endif
#ifdef DEF_A4_Cus
		" E0 E1 E3 E4 E5 J0 EN", 																	// Bit 18.
#else
		" E0 E4 E3 E5 J0 EN", 																		// Bit 18.
#endif
		" BX F0 F1 G2 H1 H2 HC V5 TC SV AP AQ", 													// Bit 19.
//2020/11/24 FPT-TinhNT16 [CSRC] 【制御構成変更_No.81】 Start.
#ifdef DEF_EagleCus
		" Z0 Z1 Z8 Z9 ZA ZG ZD ZE ZR ZC ZT", 														// Bit 20.
#else
		" Z0 Z1 Z8 Z9 ZA ZG ZD ZE ZR ZC", 															// Bit 20.
#endif
//2020/11/24 FPT-TinhNT16 [CSRC] 【制御構成変更_No.81】 End.
#ifdef DEF_MonoCus
		" AX AY AZ V2 V4 I9 V6 V7 VA VB VD VE", 													// Bit 21.
#else
		" AX AY AZ V2 V3 V4 I9 V6 V7 VA VB VD VE", 													// Bit 21.
#endif
#if defined(DEF_A4_Cus) || defined(DEF_HeliosMLKCus)
		" X0 X1 X2 XB XE",																			// Bit 22.
#else
		" X0 X1 X2 XB XC XE XF XG XH",																// Bit 22.
#endif
		" AR AS AT AU", 																			// Bit 23.
		" ", " ", " ", " ", " ", " ", " ", " " };													// Bit 24 ～ Bit 31.
//2020/07/01 FPT-TinhNT16 【 IT6_2.1 】 制御構成変更_No.4 end.

//2020/12/22 FPT-HaDV8 【 IT6_2.2 】 制御構成変更_No.4 Start.
	// IT6 機能バージョン 2.2 用テーブル.
	// uc_RDS_BITPATTERN_CD / uc_RDS_BITPATTERN_HIGHSECURITY_CD をベースに作成.
	const char *uc_RDS_BITPATTERN_Exp_03[] = {
		" ", " ", " ", " ", " ", " ", " ", " ", 													// Bit 0 ～ Bit 7.
		" ", " ", " ", " ", " ", " ",																// Bit 8 ～ Bit 13.
#if defined(DEF_SparrowCus) || defined(DEF_HeliosMLKCus)
		" XI XM XR",  																				// Bit 14.
#elif defined(DEF_EagleCus)
		 " XI XJ XK XL XM XN XO XP XR",																// Bit 14.
#else
		 " XI XJ XK XL XM XN XO XP XR",																// Bit 14.
#endif
		" DX", 																						// Bit 15.
#if defined(DEF_EagleCus) || defined(DEF_ZEUSS_CUS) || defined(DEF_ZEUSS_ZX0_CUS) || defined(DEF_ZEUSSBK12_CUS) || defined(DEF_ZEUSSBKX0_CUS) || \
	defined(DEF_ZEUSSBK2ND_12_CUS) || defined(DEF_ZEUSSBK2ND_ZX0_CUS)
		#ifdef DEF_A4_Cus
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 		// Bit 16.
// 2021/08/31 FPT-TrungHV7 [IT6_2.2][MinervaSSBK] OP_BTS-35109 Start.
//		#elif defined(DEF_HeliosMLKCus)
		#elif defined(DEF_HeliosMLKCus) || defined(DEF_MinervaSSBKCus)
// 2021/08/31 FPT-TrungHV7 [IT6_2.2][MinervaSSBK] OP_BTS-35109 End.
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 		// Bit 16.
		#else
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK AM EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 	// Bit 16.
		#endif
#else
	#if defined(DEF_DenebMLKCus)
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL YO YP YQ UM", 			// Bit 16.
	#else
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM",		// Bit 16. 
	#endif
#endif
#ifdef DEF_MonoCus
	#if defined(DEF_SparrowBKCus)
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 PH PK", 												// Bit 17.
	#else
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PK", 											// Bit 17.
	#endif
#elif defined DEF_A4_Cus
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 PH PI PJ PK", 										// Bit 17.
#else
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PI PJ PK", 										// Bit 17.
#endif
#ifdef DEF_A4_Cus
		" E0 E1 E3 E4 E5 J0 EN", 																	// Bit 18.
#else
		" E0 E3 E4 E5 J0 EN", 																		// Bit 18.
#endif
		" BX F0 F1 G2 H1 H2 HC V5 TC SV AP AQ", 													// Bit 19.
#ifdef DEF_EagleCus
		" Z0 Z1 Z8 Z9 ZA ZG ZD ZE ZR ZC ZT", 														// Bit 20.
#else
		" Z0 Z1 Z8 Z9 ZA ZG ZD ZE ZR ZC", 															// Bit 20.
#endif
#ifdef DEF_MonoCus
		" AX AY AZ V2 V4 I9 V6 V7 VA VB VD VE", 													// Bit 21.
#else
		" AX AY AZ V2 V3 V4 I9 V6 V7 VA VB VD VE",													// Bit 21.
#endif
#if defined(DEF_A4_Cus) || defined(DEF_HeliosMLKCus)
		" X0 X1 X2 XB XE",																			// Bit 22.
#else
		" X0 X1 X2 XB XC XE XF XG XH",																// Bit 22.
#endif
		" AR AS AT AU", 																			// Bit 23.
		" ", " ", " ", " ", " ", " ", " ", " " };													// Bit 24 ～ Bit 31.

	const char *uc_RDS_BITPATTERN_HIGHSECURITY_Exp_03[] = {
		" ", " ", " ", " ", " ", " ", " ", " ", 													// Bit 0 ～ Bit 7.
		" ", " ", " ", " ", " ", " ", 																// Bit 8 ～ Bit 13.
#if defined(DEF_SparrowCus) || defined(DEF_HeliosMLKCus)
		" XI XM XR",  																				// Bit 14.
#elif defined(DEF_EagleCus)
		" XI XJ XK XL XM XN XO XP XR",  															// Bit 14.
#else
		" XI XJ XK XL XM XN XO XP XR",  															// Bit 14.
#endif
		" ", 																						// Bit 15.
#if defined(DEF_EagleCus) || defined(DEF_ZEUSS_CUS) || defined(DEF_ZEUSS_ZX0_CUS) || defined(DEF_ZEUSSBK12_CUS) || defined(DEF_ZEUSSBKX0_CUS) || \
	defined(DEF_ZEUSSBK2ND_12_CUS) || defined(DEF_ZEUSSBK2ND_ZX0_CUS)
		#ifdef DEF_A4_Cus
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 		// Bit 16.
// 2021/08/31 FPT-TrungHV7 [IT6_2.2][MinervaSSBK] OP_BTS-35109 Start.
//		#elif defined(DEF_HeliosMLKCus)
		#elif defined(DEF_HeliosMLKCus) || defined(DEF_MinervaSSBKCus)
// 2021/08/31 FPT-TrungHV7 [IT6_2.2][MinervaSSBK] OP_BTS-35109 End.
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 		// Bit 16.
		#else
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK AM EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 	// Bit 16.
		#endif
#else
	#if defined(DEF_DenebMLKCus)
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL YO YP YQ UM", 			// Bit 16.
	#else
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM",		// Bit 16.
	#endif
#endif
#ifdef DEF_MonoCus
	#if defined(DEF_SparrowBKCus)
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 PH PK", 												// Bit 17.
	#else
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PK",		 									// Bit 17.
	#endif
#elif defined DEF_A4_Cus
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 PH PI PJ PK", 										// Bit 17.
#else
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PI PJ PK", 										// Bit 17.
#endif
#ifdef DEF_A4_Cus
		" E0 E1 E3 E4 E5 J0 EN", 																	// Bit 18.
#else
		" E0 E4 E3 E5 J0 EN", 																		// Bit 18.
#endif
		" BX F0 F1 G2 H1 H2 HC V5 TC SV AP AQ", 													// Bit 19.
#ifdef DEF_EagleCus
		" Z0 Z1 Z8 Z9 ZA ZG ZD ZE ZR ZC ZT", 														// Bit 20.
#else
		" Z0 Z1 Z8 Z9 ZA ZG ZD ZE ZR ZC", 															// Bit 20.
#endif
#ifdef DEF_MonoCus
		" AX AY AZ V2 V4 I9 V6 V7 VA VB VD VE", 													// Bit 21.
#else
		" AX AY AZ V2 V3 V4 I9 V6 V7 VA VB VD VE", 													// Bit 21.
#endif
#if defined(DEF_A4_Cus) || defined(DEF_HeliosMLKCus)
		" X0 X1 X2 XB XE",																			// Bit 22.
#else
		" X0 X1 X2 XB XC XE XF XG XH",																// Bit 22.
#endif
		" AR AS AT AU", 																			// Bit 23.
		" ", " ", " ", " ", " ", " ", " ", " " };													// Bit 24 ～ Bit 31.
//2020/12/22 FPT-HaDV8 【 IT6_2.2 】 制御構成変更_No.4 End.

//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 Start.
	// IT6 機能バージョン 2.3 用テーブル.
	// uc_RDS_BITPATTERN_CD / uc_RDS_BITPATTERN_HIGHSECURITY_CD をベースに作成.
	const char *uc_RDS_BITPATTERN_Exp_04[] = {
		" ", " ", " ", " ", " ", " ", " ", " ", 													// Bit 0 ～ Bit 7.
		" ", " ", " ", " ", " ", " ",																// Bit 8 ～ Bit 13.
#if defined(DEF_SparrowCus) || defined(DEF_HeliosMLKCus)
		" XI XM XR",  																				// Bit 14.
#elif defined(DEF_EagleCus)
		 " XI XJ XK XL XM XN XO XP XR",																// Bit 14.
#else
		 " XI XJ XK XL XM XN XO XP XR",																// Bit 14.
#endif
		" DX", 																						// Bit 15.
#if defined(DEF_EagleCus) || defined(DEF_ZEUSS_CUS) || defined(DEF_ZEUSS_ZX0_CUS) || defined(DEF_ZEUSSBK12_CUS) || defined(DEF_ZEUSSBKX0_CUS) || \
	defined(DEF_ZEUSSBK2ND_12_CUS) || defined(DEF_ZEUSSBK2ND_ZX0_CUS)
		#ifdef DEF_A4_Cus
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 		// Bit 16.
		#elif defined(DEF_HeliosMLKCus) || defined(DEF_MinervaSSBKCus)
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 		// Bit 16.
		#else
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK AM EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 	// Bit 16.
		#endif
#else
	#if defined(DEF_DenebMLKCus)
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL YO YP YQ UM", 			// Bit 16.
	#else
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM",		// Bit 16. 
	#endif
#endif
#ifdef DEF_MonoCus
	#if defined(DEF_SparrowBKCus)
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 PH PK", 												// Bit 17.
	#else
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PK", 											// Bit 17.
	#endif
#elif defined DEF_A4_Cus
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 PH PI PJ PK", 										// Bit 17.
#else
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PI PJ PK", 										// Bit 17.
#endif
#ifdef DEF_A4_Cus
		" E0 E1 E3 E4 E5 J0 EN", 																	// Bit 18.
#else
		" E0 E3 E4 E5 J0 EN", 																		// Bit 18.
#endif
		" BX F0 F1 G2 H1 H2 HC V5 TC SV AP AQ", 													// Bit 19.
#ifdef DEF_EagleCus
		" Z0 Z1 Z8 Z9 ZA ZG ZD ZE ZR ZC ZT", 														// Bit 20.
#else
		" Z0 Z1 Z8 Z9 ZA ZG ZD ZE ZR ZC", 															// Bit 20.
#endif
#ifdef DEF_MonoCus
		" AX AY AZ V2 V4 I9 V6 V7 VA VB VD VE", 													// Bit 21.
#else
		" AX AY AZ V2 V3 V4 I9 V6 V7 VA VB VD VE",													// Bit 21.
#endif
#if defined(DEF_A4_Cus) || defined(DEF_HeliosMLKCus)
		" X0 X1 X2 XB XE",																			// Bit 22.
#else
		" X0 X1 X2 XB XC XE XF XG XH",																// Bit 22.
#endif
		" AR AS AT AU", 																			// Bit 23.
		" ", " ", " ", " ", " ", " ", " ", " " };													// Bit 24 ～ Bit 31.

	const char *uc_RDS_BITPATTERN_HIGHSECURITY_Exp_04[] = {
		" ", " ", " ", " ", " ", " ", " ", " ", 													// Bit 0 ～ Bit 7.
		" ", " ", " ", " ", " ", " ", 																// Bit 8 ～ Bit 13.
#if defined(DEF_SparrowCus) || defined(DEF_HeliosMLKCus)
		" XI XM XR",  																				// Bit 14.
#elif defined(DEF_EagleCus)
		" XI XJ XK XL XM XN XO XP XR",  															// Bit 14.
#else
		" XI XJ XK XL XM XN XO XP XR",  															// Bit 14.
#endif
		" ", 																						// Bit 15.
#if defined(DEF_EagleCus) || defined(DEF_ZEUSS_CUS) || defined(DEF_ZEUSS_ZX0_CUS) || defined(DEF_ZEUSSBK12_CUS) || defined(DEF_ZEUSSBKX0_CUS) || \
	defined(DEF_ZEUSSBK2ND_12_CUS) || defined(DEF_ZEUSSBK2ND_ZX0_CUS)
		#ifdef DEF_A4_Cus
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 		// Bit 16.
		#elif defined(DEF_HeliosMLKCus) || defined(DEF_MinervaSSBKCus)
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 		// Bit 16.
		#else
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK AM EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM", 	// Bit 16.
		#endif
#else
	#if defined(DEF_DenebMLKCus)
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL YO YP YQ UM", 			// Bit 16.
	#else
		" A0 A1 A5 A9 AA AB AC AD AF AG AH AI AJ AK EG I0 I1 S7 S8 S9 SA UL UP YO YP YQ UM",		// Bit 16.
	#endif
#endif
#ifdef DEF_MonoCus
	#if defined(DEF_SparrowBKCus)
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 PH PK", 												// Bit 17.
	#else
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PK",		 									// Bit 17.
	#endif
#elif defined DEF_A4_Cus
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 PH PI PJ PK", 										// Bit 17.
#else
		" B0 B9 BG BH BI OC PC F3 F6 FC WC P1 BJ PH PI PJ PK", 										// Bit 17.
#endif
#ifdef DEF_A4_Cus
		" E0 E1 E3 E4 E5 J0 EN", 																	// Bit 18.
#else
		" E0 E4 E3 E5 J0 EN", 																		// Bit 18.
#endif
		" BX F0 F1 G2 H1 H2 HC V5 TC SV AP AQ", 													// Bit 19.
#ifdef DEF_EagleCus
		" Z0 Z1 Z8 Z9 ZA ZG ZD ZE ZR ZC ZT", 														// Bit 20.
#else
		" Z0 Z1 Z8 Z9 ZA ZG ZD ZE ZR ZC", 															// Bit 20.
#endif
#ifdef DEF_MonoCus
		" AX AY AZ V2 V4 I9 V6 V7 VA VB VD VE", 													// Bit 21.
#else
		" AX AY AZ V2 V3 V4 I9 V6 V7 VA VB VD VE", 													// Bit 21.
#endif
#if defined(DEF_A4_Cus) || defined(DEF_HeliosMLKCus)
		" X0 X1 X2 XB XE",																			// Bit 22.
#else
		" X0 X1 X2 XB XC XE XF XG XH",																// Bit 22.
#endif
		" AR AS AT AU", 																			// Bit 23.
		" ", " ", " ", " ", " ", " ", " ", " " };													// Bit 24 ～ Bit 31.
//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 End.

// 2019/06/27 y.hotta EagleH OP_BTS-7025 SparrowMFPのFAX通信で全コマンド取得が正常終了しない Add Start.
// FaxBitPattern出力処理（差分箇所のみ表示する）.
void printPatternDiff( const char **psc_CurVer, const char **psc_CurVerHigh, const char **psc_BefVer, int32_t i32_Size )
{
	for(int32_t i32_Idx=0; i32_Idx < i32_Size; i32_Idx ++){
		// 現バージョンのテーブルは前バージョンと差分があれば表示.
		if( 0 != strcmp(psc_BefVer[i32_Idx], psc_CurVer[i32_Idx]) ){
			printf(" [%d]=%s\n", i32_Idx, psc_CurVer[i32_Idx]);
		}
		// セキュリティ強化用は現バージョンのテーブルと差分があれば表示.
		if( 0 != strcmp(psc_CurVer[i32_Idx], psc_CurVerHigh[i32_Idx]) ){
			printf(" HiSec[%d]=%s\n", i32_Idx, psc_CurVerHigh[i32_Idx]);
		}
	}
}
// FaxBitPattern出力コマンド.
void printCSRCFaxBitPattern()
{
	int32_t i32_Size = 32;

	printf("BITDEFINE[]=");
	for(int32_t i32_Idx=0; i32_Idx < i32_Size; i32_Idx ++){
		printf(" %u", uc_RDS_BITDEFINE[i32_Idx]);
	}
	
	// 空のダミーテーブルを用意.
	const char *sc_Dummy[] = {" "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," ",
	                          " "," "," "," "," "," "," "," "," "," "," "," "," "," "," "," "};
	
	// 初版を表示.
	printf("\nBITPATTERN");
	printPatternDiff( uc_RDS_BITPATTERN, uc_RDS_BITPATTERN_HIGHSECURITY, sc_Dummy, i32_Size );
	
	// 機能拡張１版を表示.
	printf("BITPATTERN_Exp_01");
	printPatternDiff( uc_RDS_BITPATTERN_Exp_01, uc_RDS_BITPATTERN_HIGHSECURITY_Exp_01, uc_RDS_BITPATTERN, i32_Size );

//2020/07/01 FPT-TinhNT16 【 IT6_2.1 】 制御構成変更_No.4 start.
	// 機能拡張2版を表示.
	printf("BITPATTERN_Exp_02");
	printPatternDiff( uc_RDS_BITPATTERN_Exp_02, uc_RDS_BITPATTERN_HIGHSECURITY_Exp_02, uc_RDS_BITPATTERN_Exp_01, i32_Size );
//2020/07/01 FPT-TinhNT16 【 IT6_2.1 】 制御構成変更_No.4 end.

//2020/12/22 FPT-HaDV8 【 IT6_2.2 】 制御構成変更_No.4 Start.
	// 機能拡張3版を表示.
	printf("BITPATTERN_Exp_03");
	printPatternDiff( uc_RDS_BITPATTERN_Exp_03, uc_RDS_BITPATTERN_HIGHSECURITY_Exp_03, uc_RDS_BITPATTERN_Exp_02, i32_Size );
//2020/12/22 FPT-HaDV8 【 IT6_2.2 】 制御構成変更_No.4 End.

//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 Start.
	// 機能拡張4版を表示.
	printf("BITPATTERN_Exp_04");
	printPatternDiff( uc_RDS_BITPATTERN_Exp_04, uc_RDS_BITPATTERN_HIGHSECURITY_Exp_04, uc_RDS_BITPATTERN_Exp_03, i32_Size );
//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 End.
}
// 2019/06/27 y.hotta EagleH OP_BTS-7025 SparrowMFPのFAX通信で全コマンド取得が正常終了しない Add End.
// 2019/07/23 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 ADD Start.
// FaxBitPattern出力コマンド2.
void printCSRCFaxBitPattern2()
{
	int32_t i32_Size = 32;

	printf("BITDEFINE[]=");
	for(int32_t i32_Idx=0; i32_Idx < i32_Size; i32_Idx ++){
		printf(" %u", uc_RDS_BITDEFINE[i32_Idx]);
	}

	// 初版を表示.
	printf("\nBITPATTERN\n");
	for(i32_Idx=0; i32_Idx < i32_Size; i32_Idx ++){
		printf("uc_RDS_BITPATTERN[%02d]=%s\n", i32_Idx, uc_RDS_BITPATTERN[i32_Idx]);
	}
	for(i32_Idx=0; i32_Idx < i32_Size; i32_Idx ++){
		printf("uc_RDS_BITPATTERN_HIGHSECURITY[%02d]=%s\n", i32_Idx, uc_RDS_BITPATTERN_HIGHSECURITY[i32_Idx]);
	}

	// 機能拡張１版を表示.
	printf("BITPATTERN_Exp_01\n");
	for(i32_Idx=0; i32_Idx < i32_Size; i32_Idx ++){
		printf("uc_RDS_BITPATTERN_Exp_01[%02d]=%s\n", i32_Idx, uc_RDS_BITPATTERN_Exp_01[i32_Idx]);
	}
	for(i32_Idx=0; i32_Idx < i32_Size; i32_Idx ++){
		printf("uc_RDS_BITPATTERN_HIGHSECURITY_Exp_01[%02d]=%s\n", i32_Idx, uc_RDS_BITPATTERN_HIGHSECURITY_Exp_01[i32_Idx]);
	}

//2020/07/01 FPT-TinhNT16 【 IT6_2.1 】 制御構成変更_No.4 start.
	// 機能拡張2版を表示.
	printf("BITPATTERN_Exp_02\n");
	for(i32_Idx=0; i32_Idx < i32_Size; i32_Idx ++){
		printf("uc_RDS_BITPATTERN_Exp_02[%02d]=%s\n", i32_Idx, uc_RDS_BITPATTERN_Exp_02[i32_Idx]);
	}
	for(i32_Idx=0; i32_Idx < i32_Size; i32_Idx ++){
		printf("uc_RDS_BITPATTERN_HIGHSECURITY_Exp_02[%02d]=%s\n", i32_Idx, uc_RDS_BITPATTERN_HIGHSECURITY_Exp_02[i32_Idx]);
	}
//2020/07/01 FPT-TinhNT16 【 IT6_2.1 】 制御構成変更_No.4 end.

//2020/12/22 FPT-HaDV8 【 IT6_2.2 】 制御構成変更_No.4 Start.
	// 機能拡張2版を表示.
	printf("BITPATTERN_Exp_03\n");
	for(i32_Idx=0; i32_Idx < i32_Size; i32_Idx ++){
		printf("uc_RDS_BITPATTERN_Exp_03[%02d]=%s\n", i32_Idx, uc_RDS_BITPATTERN_Exp_03[i32_Idx]);
	}
	for(i32_Idx=0; i32_Idx < i32_Size; i32_Idx ++){
		printf("uc_RDS_BITPATTERN_HIGHSECURITY_Exp_03[%02d]=%s\n", i32_Idx, uc_RDS_BITPATTERN_HIGHSECURITY_Exp_03[i32_Idx]);
	}
//2020/12/22 FPT-HaDV8 【 IT6_2.2 】 制御構成変更_No.4 End.

//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 Start.
	// 機能拡張4版を表示.
	printf("BITPATTERN_Exp_04\n");
	for(i32_Idx=0; i32_Idx < i32_Size; i32_Idx ++){
		printf("uc_RDS_BITPATTERN_Exp_04[%02d]=%s\n", i32_Idx, uc_RDS_BITPATTERN_Exp_04[i32_Idx]);
	}
	for(i32_Idx=0; i32_Idx < i32_Size; i32_Idx ++){
		printf("uc_RDS_BITPATTERN_HIGHSECURITY_Exp_04[%02d]=%s\n", i32_Idx, uc_RDS_BITPATTERN_HIGHSECURITY_Exp_04[i32_Idx]);
	}
//2022/06/07 FPT-NhuongNV1 【 IT6_2.3 】 制御構成変更_No.4 End.

}
// 2019/07/23 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 ADD End.
