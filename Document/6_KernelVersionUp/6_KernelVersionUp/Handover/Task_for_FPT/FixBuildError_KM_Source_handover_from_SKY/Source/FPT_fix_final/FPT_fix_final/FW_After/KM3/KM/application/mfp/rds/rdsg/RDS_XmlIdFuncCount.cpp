
// CSRC(rds)内のインクルード.
#include "RDS_XmlId.h"

// モジュール外のインクルード.
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "system/nvd/APIC_Counter.h"
// 2015/10/28 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(AQコマンド追加) START.
#include "system/sys/APIC_IWSAppInfo.h"
#include "system/nvd/APIC_ExtSetting.h"
// 2015/10/28 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(AQコマンド追加) END.

//*********************************************************************************
// ID（項目）追加実装時の注意点                                                   .
//                                                                                .
// 1. RDS_XmlMatrix.cppに追加をする                                               .
// 2. RDS_XmlIdFuncCount.cppにテーブルの追加  FuncTable_FuncCount                 .
//	       ※かならずIDが順に大きくなる位置に追加すること！！！                   .
//*********************************************************************************

// 機能カウンタ用のテーブル.
// ※機種による違いはRDS_XmlMatric.cppのテーブルで吸収すること.
typedef struct 
{
	type_id majorId;			// 分類1のID.
	type_id minorId;			// 分類2のID.
	type_id c_Id;				// 分類3のID.
// 2015/10/19 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加) START.
	type_cnt  cntType;			// カウンタ種別.
	TYPE_FuncUseCnt e_FuncNum;	// CommonAPIコール用Enum（Job関連）.
	TYPE_OperateCnt	e_OperateNum;	// CommonAPIコール用Enum（ユーザー操作関連）.
// 2015/10/19 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加) END.
}_IDtoFuncCountTable;

static _IDtoFuncCountTable FuncTable_FuncCount[] =
{
// 2015/10/19 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加) START.
// -------------------------------------------------------------------------------------------
// 本テーブルに新規追加する場合は以下のルールとする。.
//   ①追加されたカウンタがAPIC_Counter.hの「TYPE_FuncUseCnt (ジョブ関連）」に属する場合.
//     →cnt_type=RDS_TYPE_CNT_JOB   e_FuncNum=TYPE_FuncUseCntxxxxxx  e_OperateNum=TYPE_OperateCntSize.
//   ②追加されたカウンタがAPIC_Counter.hの「TYPE_OperateCnt (ユーザ操作関連）」に属する場合.
//     →cnt_type=RDS_TYPE_CNT_USER  e_FuncNum=TYPE_FuncUseCntSize    e_OperateNum=TYPE_OperateCnt_xxxxxx.
// -------------------------------------------------------------------------------------------
	//	major	minor	s		cnt_type    func(job)                                       		func(user).
	{	1,		1,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeNormal, 						TYPE_OperateCntSize },
	{	1,		1,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeNormal_2nd, 					TYPE_OperateCntSize },
	{	1,		1,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeOhp, 							TYPE_OperateCntSize },
	{	1,		1,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeLetterHead, 					TYPE_OperateCntSize },
	{	1,		1,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeSpecial, 						TYPE_OperateCntSize },
	{	1,		1,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeNotDuplex, 						TYPE_OperateCntSize },
	{	1,		1,		7,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeColor, 							TYPE_OperateCntSize },
	{	1,		1,		8,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeEnvelope, 						TYPE_OperateCntSize },
	{	1,		1,		9,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeThick1, 						TYPE_OperateCntSize },
	{	1,		1,		10,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeThick1_2nd, 					TYPE_OperateCntSize },
	{	1,		1,		11,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeThick1Plus, 					TYPE_OperateCntSize },
	{	1,		1,		12,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeThick1Plus_2nd, 				TYPE_OperateCntSize },
	{	1,		1,		13,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeThick2, 						TYPE_OperateCntSize },
	{	1,		1,		14,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeThick2_2nd, 					TYPE_OperateCntSize },
	{	1,		1,		15,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeThick3, 						TYPE_OperateCntSize },
	{	1,		1,		16,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeThick3_2nd, 					TYPE_OperateCntSize },
	{	1,		1,		17,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeThick4, 						TYPE_OperateCntSize },
	{	1,		1,		18,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeThick4_2nd, 					TYPE_OperateCntSize },
	{	1,		1,		19,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeUser1, 							TYPE_OperateCntSize },
	{	1,		1,		20,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeUser1_2nd, 						TYPE_OperateCntSize },
	{	1,		1,		21,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeUser2, 							TYPE_OperateCntSize },
	{	1,		1,		22,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeUser2_2nd, 						TYPE_OperateCntSize },
	{	1,		1,		23,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeUser3, 							TYPE_OperateCntSize },
	{	1,		1,		24,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeUser3_2nd, 						TYPE_OperateCntSize },
	{	1,		1,		25,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeUser4, 							TYPE_OperateCntSize },
	{	1,		1,		26,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeUser4_2nd, 						TYPE_OperateCntSize },
	{	1,		1,		27,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeUser5, 							TYPE_OperateCntSize },
	{	1,		1,		28,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeUser5_2nd, 						TYPE_OperateCntSize },
	{	1,		1,		29,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeUser6, 							TYPE_OperateCntSize },
	{	1,		1,		30,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeUser6_2nd, 						TYPE_OperateCntSize },
	{	1,		1,		32,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeIndex, 							TYPE_OperateCntSize },	// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
	{	1,		1,		33,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeThin, 							TYPE_OperateCntSize },	// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
	{	1,		1,		34,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeThin_2nd, 						TYPE_OperateCntSize },	// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
#ifdef DEF_A4_Cus	//20160503 VenusMLK CQ524880 HCL_Pawan Start.
	{	1,		1,		35,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeGloss1, 						TYPE_OperateCntSize },
	{	1,		1,		36,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeGloss1Plus, 					TYPE_OperateCntSize },
	{	1,		1,		37,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeGloss2, 						TYPE_OperateCntSize },
	{	1,		1,		38,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperPostCard, 							TYPE_OperateCntSize },
	{	1,		1,		39,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPaperTypeLabel, 							TYPE_OperateCntSize },							
#endif	//20160503 VenusMLK CQ524880 HCL_Pawan End.
	{ 	1,  	1, 		40,  	RDS_TYPE_CNT_JOB, 	TYPE_FuncUseCntCopyPaperTypePlainPaperPlus,        			TYPE_OperateCntSize },							//2018/02/26 【 IT6_1.0 】 【EAGLE】 【RQ-722】 HCLT-Raghu Start.
 	{ 	1, 		1,  	41, 	RDS_TYPE_CNT_JOB, 	TYPE_FuncUseCntCopyPaperTypePlainPaperPlus_2nd,        		TYPE_OperateCntSize },							//2018/02/26 【 IT6_1.0 】 【EAGLE】 【RQ-722】 HCLT-Raghu End.
//2021/03/09 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 制御構成変更_No.4 Start.
//#ifdef DEF_EagleZpCus	//2019/07/01【 IT6_2.0 】【EagleZ+】【機器構成変更_No.1】kurihara Start.
// 2021/07/16 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 OP_BTS-34558 Start.
//#if defined(DEF_EagleZpCus) || defined(DEF_MinervaSSBKCus)
#ifdef DEF_EagleZpCus
// 2021/07/16 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 OP_BTS-34558 End.
//2021/03/09 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 制御構成変更_No.4 End.
	{	1,		1,		42,		RDS_TYPE_CNT_JOB, 	TYPE_FuncUseCntCopyPaperTypeUser7,							TYPE_OperateCntSize },
	{	1,		1,		43,		RDS_TYPE_CNT_JOB, 	TYPE_FuncUseCntCopyPaperTypeUser7_2nd,						TYPE_OperateCntSize },
#endif		//2019/07/01【 IT6_2.0 】【EagleZ+】【機器構成変更_No.1】kurihara.
	{	1,		2,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyMagAms, 									TYPE_OperateCntSize },
	{	1,		2,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyMagFull, 								TYPE_OperateCntSize },
	{	1,		2,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyMagFixedZoom, 							TYPE_OperateCntSize },
	{	1,		2,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyMagMinimal, 								TYPE_OperateCntSize },
	{	1,		2,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyMagWithZoom, 							TYPE_OperateCntSize },
	{	1,		2,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyMagNotWithZoom, 							TYPE_OperateCntSize },
	{	1,		3,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyImageAdjustModeText, 					TYPE_OperateCntSize },
	{	1,		3,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyImageAdjustModeTextPhoto, 				TYPE_OperateCntSize },
	{	1,		3,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyImageAdjustModePhoto, 					TYPE_OperateCntSize },
	{	1,		3,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyImageAdjustModeMap, 						TYPE_OperateCntSize },
	{	1,		3,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyImageAdjustModePencil, 					TYPE_OperateCntSize },
	{	1,		3,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyImageAdjustModeCopyOriginal, 			TYPE_OperateCntSize },
	{	1,		4,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyOriginalNotDuplex, 						TYPE_OperateCntSize },
	{	1,		4,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyOriginalDuplex, 							TYPE_OperateCntSize },
	{	1,		4,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyOriginalDiv, 							TYPE_OperateCntSize },
	{	1,		4,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyOriginalBookOpen, 						TYPE_OperateCntSize },
	{	1,		4,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyOriginalZ, 								TYPE_OperateCntSize },
	{	1,		4,		7,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyOriginalSize, 							TYPE_OperateCntSize },
	{	1,		4,		8,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyOriginalNonStandard, 					TYPE_OperateCntSize },
	{	1,		4,		9,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyOriginalBookRepeatFrontCover, 			TYPE_OperateCntSize },
	{	1,		4,		10,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyOriginalBookRepeatFrontBackCover, 		TYPE_OperateCntSize },
	{	1,		4,		11,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyOriginalIndex, 							TYPE_OperateCntSize },	// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
	{	1,		4,		12,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyOriginalDeleteBlankPage, 				TYPE_OperateCntSize },	// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
	{	1,		4,		13,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyOriginalContinuousRead, 					TYPE_OperateCntSize },	// 2011/04/26 Iwata Minerva/Zeus UV-00866 項目追加Start.
	{	1,		4,		14,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyOriginalAutoImageRotation, 				TYPE_OperateCntSize },	
	{	1,		4,		15,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyOriginalCatalogOriginal, 				TYPE_OperateCntSize },	
	{	1,		4,		16,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyOriginalSameWidthEnable, 				TYPE_OperateCntSize },	
	{	1,		4,		17,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyOriginalMixedWidthEnable,				TYPE_OperateCntSize },	
	{	1,		4,		18,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyOriginalThinPaperMode, 					TYPE_OperateCntSize },		// 2011/04/26 Iwata Minerva/Zeus UV-00866 項目追加End.
	{	1,		5,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPage2in1, 								TYPE_OperateCntSize },
	{	1,		5,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPage4in1, 								TYPE_OperateCntSize },
	{	1,		5,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyPage8in1, 								TYPE_OperateCntSize },
	{	1,		6,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppBooklet, 								TYPE_OperateCntSize },
	{	1,		6,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppCardCopy, 							TYPE_OperateCntSize },
	{	1,		6,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppOhpIns, 								TYPE_OperateCntSize },
	{	1,		6,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppInterSheet, 							TYPE_OperateCntSize },
	{	1,		6,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppOriginalInsertion, 					TYPE_OperateCntSize },
	{	1,		6,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppChapter, 								TYPE_OperateCntSize },
	{	1,		6,		7,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppCover, 								TYPE_OperateCntSize },
	{	1,		6,		8,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppPiCoverSheet, 						TYPE_OperateCntSize },
	{	1,		6,		9,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppPiInterSheet, 						TYPE_OperateCntSize },
	{	1,		6,		10,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppMultiPage, 							TYPE_OperateCntSize },
	{	1,		6,		11,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppCatalogShot, 							TYPE_OperateCntSize },
	{	1,		6,		12,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppImageRepeatManual, 					TYPE_OperateCntSize },
	{	1,		6,		13,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppImageRepeatAuto, 						TYPE_OperateCntSize },
	{	1,		6,		14,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppImageRepeatStandard, 					TYPE_OperateCntSize },
	{	1,		6,		15,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppNPReverse, 							TYPE_OperateCntSize },
	{	1,		6,		16,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppMirror, 								TYPE_OperateCntSize },
	{	1,		6,		17,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppLusterCopyMode, 						TYPE_OperateCntSize },
	{	1,		6,		18,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppBaseColor, 							TYPE_OperateCntSize },
	{	1,		6,		19,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppNotRotate, 							TYPE_OperateCntSize },
	{	1,		6,		20,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppOverlay, 								TYPE_OperateCntSize },
	{	1,		6,		21,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppRegistOverlayRegImage, 				TYPE_OperateCntSize },
	{	1,		6,		22,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppRegistOverlayOutImage, 				TYPE_OperateCntSize },
	{	1,		6,		23,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppStamp, 								TYPE_OperateCntSize },
	{	1,		6,		24,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppPageStamp, 							TYPE_OperateCntSize },
	{	1,		6,		25,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppDateStamp, 							TYPE_OperateCntSize },
	{	1,		6,		26,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppHeaderFooterStamp, 					TYPE_OperateCntSize },
	{	1,		6,		27,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppRepeatStamp, 							TYPE_OperateCntSize },
	{	1,		6,		28,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppWatermark, 							TYPE_OperateCntSize },
	{	1,		6,		29,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppCopyGuard, 							TYPE_OperateCntSize },
	{	1,		6,		30,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppPasswordCopy, 						TYPE_OperateCntSize },
	{	1,		6,		31,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppCopyProtect, 							TYPE_OperateCntSize },
	{	1,		6,		32,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppConfirmPrint, 						TYPE_OperateCntSize },
	{	1,		6,		33,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppInterrupt, 							TYPE_OperateCntSize },
	{	1,		6,		34,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppJobProgram, 							TYPE_OperateCntSize },
	{	1,		6,		35,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntCopyAppPreview, 								TYPE_OperateCntSize },
	{	2,		1,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeNormal, 						TYPE_OperateCntSize },
	{	2,		1,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeNormal_2nd, 					TYPE_OperateCntSize },
	{	2,		1,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeOhp, 							TYPE_OperateCntSize },
	{	2,		1,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeLetterHead, 					TYPE_OperateCntSize },
	{	2,		1,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeSpecial, 						TYPE_OperateCntSize },
	{	2,		1,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeNotDuplex, 					TYPE_OperateCntSize },
	{	2,		1,		7,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeColor, 						TYPE_OperateCntSize },
	{	2,		1,		8,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeEnvelope, 						TYPE_OperateCntSize },
	{	2,		1,		9,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeThick1, 						TYPE_OperateCntSize },
	{	2,		1,		10,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeThick1_2nd, 					TYPE_OperateCntSize },
	{	2,		1,		11,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeThick1Plus, 					TYPE_OperateCntSize },
	{	2,		1,		12,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeThick1Plus_2nd, 				TYPE_OperateCntSize },
	{	2,		1,		13,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeThick2, 						TYPE_OperateCntSize },
	{	2,		1,		14,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeThick2_2nd, 					TYPE_OperateCntSize },
	{	2,		1,		15,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeThick3, 						TYPE_OperateCntSize },
	{	2,		1,		16,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeThick3_2nd, 					TYPE_OperateCntSize },
	{	2,		1,		17,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeThick4, 						TYPE_OperateCntSize },
	{	2,		1,		18,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeThick4_2nd, 					TYPE_OperateCntSize },
	{	2,		1,		19,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeUser1, 						TYPE_OperateCntSize },
	{	2,		1,		20,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeUser1_2nd, 					TYPE_OperateCntSize },
	{	2,		1,		21,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeUser2, 						TYPE_OperateCntSize },
	{	2,		1,		22,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeUser2_2nd, 					TYPE_OperateCntSize },
	{	2,		1,		23,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeUser3, 						TYPE_OperateCntSize },
	{	2,		1,		24,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeUser3_2nd, 					TYPE_OperateCntSize },
	{	2,		1,		25,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeUser4, 						TYPE_OperateCntSize },
	{	2,		1,		26,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeUser4_2nd, 					TYPE_OperateCntSize },
	{	2,		1,		27,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeUser5, 						TYPE_OperateCntSize },
	{	2,		1,		28,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeUser5_2nd, 					TYPE_OperateCntSize },
	{	2,		1,		29,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeUser6, 						TYPE_OperateCntSize },
	{	2,		1,		30,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeUser6_2nd, 					TYPE_OperateCntSize },
	{	2,		1,		32,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeIndex, 						TYPE_OperateCntSize },	// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
	{	2,		1,		33,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeThin, 							TYPE_OperateCntSize },	// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
	{	2,		1,		34,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeThin_2nd, 						TYPE_OperateCntSize },	// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
	{	2,		1,		35,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeDontcare, 						TYPE_OperateCntSize },	// 2013/02/20 satohiro ZM3rd UV-00685.
#ifdef DEF_A4_Cus	//20160503 VenusMLK CQ524880 HCL_Pawan Start.
	{	2,		1,		36,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeGloss1, 						TYPE_OperateCntSize },
	{	2,		1,		37,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeGloss1Plus, 					TYPE_OperateCntSize },
	{	2,		1,		38,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeGloss2, 						TYPE_OperateCntSize },
	{	2,		1,		39,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperPostCard, 							TYPE_OperateCntSize },
	{	2,		1,		40,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPaperTypeLabel,							TYPE_OperateCntSize },
#endif	//20160503 VenusMLK CQ524880 HCL_Pawan End.
	{ 	2,  	1, 		41,  	RDS_TYPE_CNT_JOB, 	TYPE_FuncUseCntPrintPaperTypePlainPaperPlus,        		TYPE_OperateCntSize },	//2018/02/26 【 IT6_1.0 】 【EAGLE】 【RQ-722】 HCLT-Raghu Start.
 	{ 	2,  	1,  	42,  	RDS_TYPE_CNT_JOB, 	TYPE_FuncUseCntPrintPaperTypePlainPaperPlus_2nd,        	TYPE_OperateCntSize },	//2018/02/26 【 IT6_1.0 】 【EAGLE】 【RQ-722】 HCLT-Raghu End.
//2021/03/09 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 制御構成変更_No.4 Start.
//#ifdef DEF_EagleZpCus	//2019/07/01【 IT6_2.0 】【EagleZ+】【機器構成変更_No.1】kurihara Start.
// 2021/07/16 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 OP_BTS-34558 Start.
//#if defined(DEF_EagleZpCus) || defined(DEF_MinervaSSBKCus)
#ifdef DEF_EagleZpCus
// 2021/07/16 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 OP_BTS-34558 End.
//2021/03/09 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 制御構成変更_No.4 End.
	{	2,		1,		43,		RDS_TYPE_CNT_JOB, 	TYPE_FuncUseCntPrintPaperTypeUser7,							TYPE_OperateCntSize },
	{	2,		1,		44,		RDS_TYPE_CNT_JOB, 	TYPE_FuncUseCntPrintPaperTypeUser7_2nd,						TYPE_OperateCntSize },
#endif		//2019/07/01【 IT6_2.0 】【EagleZ+】【機器構成変更_No.1】kurihara.
	{	2,		2,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPage2in1, 								TYPE_OperateCntSize },
	{	2,		2,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPage4in1, 								TYPE_OperateCntSize },
	{	2,		2,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPage6in1, 								TYPE_OperateCntSize },
	{	2,		2,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPage9in1, 								TYPE_OperateCntSize },
	{	2,		2,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintPage16in1, 								TYPE_OperateCntSize },
	{	2,		3,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintAppInterSheet, 							TYPE_OperateCntSize },
	{	2,		3,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintAppChapter, 							TYPE_OperateCntSize },
	{	2,		3,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintAppCover, 								TYPE_OperateCntSize },
	{	2,		3,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintAppPiCoverSheet, 						TYPE_OperateCntSize },
	{	2,		3,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintAppPiInterSheet, 						TYPE_OperateCntSize },
	{	2,		3,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintAppRegistOverlayOutImage, 				TYPE_OperateCntSize },
	{	2,		3,		7,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintAppStamp, 								TYPE_OperateCntSize },
	{	2,		3,		8,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintAppPageStamp, 							TYPE_OperateCntSize },
	{	2,		3,		9,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintAppDateStamp, 							TYPE_OperateCntSize },
	{	2,		3,		10,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintAppHeaderFooterStamp, 					TYPE_OperateCntSize },
	{	2,		3,		11,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintAppRepeatStamp, 						TYPE_OperateCntSize },
	{	2,		3,		12,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintAppCopyGuard, 							TYPE_OperateCntSize },
	{	2,		3,		13,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintAppPasswordCopy, 						TYPE_OperateCntSize },
	{	2,		3,		14,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintAppCopyProtect, 						TYPE_OperateCntSize },
	{	2,		3,		15,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintAppCarbonCopy, 							TYPE_OperateCntSize },	// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
	{	2,		4,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCnt1200dpiPrint, 								TYPE_OperateCntSize },			// 2014/10/16 H.Ookuma RQ-250対応 【機能利用頻度の収集機能（機能カウンタ）1200dpiでの印刷】.
	{	2,		4,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntDirectPrint, 								TYPE_OperateCntSize },			// 2014/10/16 H.Ookuma RQ-250対応【機能利用頻度の収集機能（機能カウンタ）ダイレクトプリント】.
	{	2,		4,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntAuthenticationPrint, 						TYPE_OperateCntSize },	// 2014/10/16 H.Ookuma RQ-250対応【機能利用頻度の収集機能（機能カウンタ）認証&プリント】.
	{	2,		4,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntEncryptionPDFPrint, 							TYPE_OperateCntSize },	// 2014/10/16 H.Ookuma RQ-250対応【機能利用頻度の収集機能（機能カウンタ）暗号化PDFプリント】.
	{	2,		4,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntGoogleCloudPrint, 							TYPE_OperateCntSize },		// 2014/10/16 H.Ookuma RQ-250対応【機能利用頻度の収集機能（機能カウンタ）Google Cloud Printでの印刷】.
	{	2,		4,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntIPPPrint, 									TYPE_OperateCntSize },				// 2014/10/16 H.Ookuma RQ-250対応【機能利用頻度の収集機能（機能カウンタ）IPP印刷】.
	{	3,		1,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxMagFull, 									TYPE_OperateCntSize },
	{	3,		1,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxMagFixedZoom, 							TYPE_OperateCntSize },
	{	3,		1,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxMagMinimal, 								TYPE_OperateCntSize },
	{	3,		1,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxMagWithZoom, 								TYPE_OperateCntSize },
	{	3,		2,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxPage2in1, 								TYPE_OperateCntSize },
	{	3,		2,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxPage4in1, 								TYPE_OperateCntSize },
	{	3,		2,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxPage8in1, 								TYPE_OperateCntSize },
	{	3,		3,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxAppInterSheet, 							TYPE_OperateCntSize },
	{	3,		3,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxAppChapter, 								TYPE_OperateCntSize },
	{	3,		3,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxAppCover, 								TYPE_OperateCntSize },
	{	3,		3,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxAppPiCoverSheet, 							TYPE_OperateCntSize },
	{	3,		3,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxAppPiInterSheet, 							TYPE_OperateCntSize },
	{	3,		3,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxAppRegistOverlayOutImage, 				TYPE_OperateCntSize },
	{	3,		3,		7,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxAppStamp, 								TYPE_OperateCntSize },
	{	3,		3,		8,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxAppPageStamp, 							TYPE_OperateCntSize },
	{	3,		3,		9,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxAppDateStamp, 							TYPE_OperateCntSize },
	{	3,		3,		10,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxAppHeaderFooterStamp, 					TYPE_OperateCntSize },
	{	3,		3,		11,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxAppRepeatStamp, 							TYPE_OperateCntSize },
	{	3,		3,		12,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxAppCopyGuard, 							TYPE_OperateCntSize },
	{	3,		3,		13,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxAppPasswordCopy, 							TYPE_OperateCntSize },
	{	3,		3,		14,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxAppCopyProtect, 							TYPE_OperateCntSize },
	{	3,		3,		15,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntBoxAppBooklet, 								TYPE_OperateCntSize },				// 2011/11/18 amiya UV-00933 BOX to PrintでのBooklet(小冊子)対応要望.
	{	4,		1,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanTypeScanToEmail, 						TYPE_OperateCntSize },
	{	4,		1,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanTypeScanToFTP, 							TYPE_OperateCntSize },
	{	4,		1,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanTypeScanToWebDAV, 						TYPE_OperateCntSize },
	{	4,		1,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanTypeScanToSMB, 							TYPE_OperateCntSize },
	{	4,		1,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanTypeScanToBox, 							TYPE_OperateCntSize },
	{	4,		1,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanTypeScanToUSB, 							TYPE_OperateCntSize },
	{	4,		1,		7,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanTypeScanToScanServer, 					TYPE_OperateCntSize },	// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
	{	4,		2,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanImageAdjustModeText, 					TYPE_OperateCntSize },
	{	4,		2,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanImageAdjustModeTextPhoto, 				TYPE_OperateCntSize },
	{	4,		2,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanImageAdjustModePhoto, 					TYPE_OperateCntSize },
	{	4,		2,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanImageAdjustModePencil, 					TYPE_OperateCntSize },
	{	4,		2,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanImageAdjustModeCopyOriginal, 			TYPE_OperateCntSize },
	{	4,		3,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanOriginalNotDuplex, 						TYPE_OperateCntSize },
	{	4,		3,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanOriginalDuplex, 							TYPE_OperateCntSize },
	{	4,		3,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanOriginalFront_Duplex, 					TYPE_OperateCntSize },
	{	4,		3,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanOriginalZ, 								TYPE_OperateCntSize },
	{	4,		3,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanOriginalLongPaper, 						TYPE_OperateCntSize },
	{	4,		3,		7,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanOriginalDirtReducation, 					TYPE_OperateCntSize },
	{	4,		3,		12,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanOriginalDeleteBlankPage, 				TYPE_OperateCntSize },	// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
	{	4,		3,		13,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanOriginalContinuousRead, 					TYPE_OperateCntSize },	// 2011/04/26 Iwata Minerva/Zeus UV-00866 項目追加Start.
	{	4,		3,		14,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanOriginalSameWidthEnable, 				TYPE_OperateCntSize },	
	{	4,		3,		15,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanOriginalMixedWidthEnable,				TYPE_OperateCntSize },	
	{	4,		3,		16,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanOriginalThinPaperMode, 					TYPE_OperateCntSize },		// 2011/04/26 Iwata Minerva/Zeus UV-00866 項目追加End.
	{	4,		4,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanResolution200, 							TYPE_OperateCntSize },
	{	4,		4,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanResolution300, 							TYPE_OperateCntSize },
	{	4,		4,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanResolution400, 							TYPE_OperateCntSize },
	{	4,		4,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanResolution600, 							TYPE_OperateCntSize },
	{	4,		5,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatJpeg, 							TYPE_OperateCntSize },
	{	4,		5,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatTiff, 							TYPE_OperateCntSize },
	{	4,		5,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatPdf, 							TYPE_OperateCntSize },
	{	4,		5,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatCompactPdf, 					TYPE_OperateCntSize },
	{	4,		5,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatXps, 							TYPE_OperateCntSize },
	{	4,		5,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatCompactXps, 					TYPE_OperateCntSize },
	{	4,		5,		7,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatSearchablePdf, 				TYPE_OperateCntSize },
	{	4,		5,		8,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatOutlinePdf, 					TYPE_OperateCntSize },
	{	4,		5,		9,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatEncryptionPDF_Pass, 			TYPE_OperateCntSize },
	{	4,		5,		10,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatEncryptionPDF_DigitalId, 		TYPE_OperateCntSize },
	{	4,		5,		11,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatStampCompositionText, 			TYPE_OperateCntSize },
	{	4,		5,		12,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatStampCompositionImage, 		TYPE_OperateCntSize },
	{	4,		5,		13,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatPdfA_1a, 						TYPE_OperateCntSize },	// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
	{	4,		5,		14,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatPdfA_1b, 						TYPE_OperateCntSize },	// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
	{	4,		5,		15,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatOOXML, 						TYPE_OperateCntSize },	// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
	{	4,		5,		16,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatLinearizedPdf, 				TYPE_OperateCntSize },	// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
	{	4,		5,		17,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatDOCX, 							TYPE_OperateCntSize },		// 2012/02/22 amiya 【ZeusMLK】UV-00973 紙資料をMicrosoft社のOffice(Word、Excel)形式のファイルに変換したい START.
	{	4,		5,		18,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatXLSX, 							TYPE_OperateCntSize },
	{	4,		5,		19,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatOCR, 							TYPE_OperateCntSize },
	{	4,		5,		20,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatDOCX_Text, 					TYPE_OperateCntSize },
	{	4,		5,		21,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatDOCX_Image, 					TYPE_OperateCntSize },
	{	4,		5,		22,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatDOCX_TextAndImage, 			TYPE_OperateCntSize },
	{	4,		5,		23,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatXLSX_TextAndImage, 			TYPE_OperateCntSize },
	{	4,		5,		24,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanFileFormatXLSX_Text, 					TYPE_OperateCntSize },	// 2012/02/22 amiya 【ZeusMLK】UV-00973 紙資料をMicrosoft社のOffice(Word、Excel)形式のファイルに変換したい END.
	{	4,		6,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanCommunicationURL, 						TYPE_OperateCntSize },
	{	4,		6,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanCommunicationFile, 						TYPE_OperateCntSize },
	{	4,		6,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanCommunicationEncryptionEmail, 			TYPE_OperateCntSize },
	{	4,		6,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanCommunicationDigitalSign, 				TYPE_OperateCntSize },
	{	4,		7,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanAppBookRepeat, 							TYPE_OperateCntSize },
	{	4,		7,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanAppDateStamp, 							TYPE_OperateCntSize },
	{	4,		7,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanAppPageStamp, 							TYPE_OperateCntSize },
	{	4,		7,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanAppHeaderFooterStamp, 					TYPE_OperateCntSize },
	{	4,		7,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanAppStamp, 								TYPE_OperateCntSize },
	{	4,		7,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanAppFilingNumber, 						TYPE_OperateCntSize },
	{	4,		7,		7,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanAppWithPrint, 							TYPE_OperateCntSize },
	{	4,		7,		8,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanAppPreview, 								TYPE_OperateCntSize },
	{	5,		1,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxTypeG3_Fax, 								TYPE_OperateCntSize },
	{	5,		1,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxTypeI_Fax, 								TYPE_OperateCntSize },
	{	5,		1,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxTypeIPAddr_Fax, 							TYPE_OperateCntSize },
	{	5,		1,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxTypeRightFax, 							TYPE_OperateCntSize },
// 2019/06/28 EagleZ+ 制御構成変更_No.36 MIF機対応 日立 江上 MOD Start.
////2018/12/11 FPT-DucDH4 【IT6_2.0】【EagleH】【RQ-272】 Fax機能をもつ全製品のT.38Faxプロトコルの対応 Start
//#if defined(DEF_EagleCus)
#if !defined(DEF_DenebMLKCus)
	{	5,		1,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxTypeIPFaxT38, 							TYPE_OperateCntSize },
#endif
//2018/12/11 FPT-DucDH4 【IT6_2.0】【EagleH】【RQ-272】 Fax機能をもつ全製品のT.38Faxプロトコルの対応 End
// 2019/06/28 EagleZ+ 制御構成変更_No.36 MIF機対応 日立 江上 MOD End.
	{	5,		2,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxImageAdjustModeText, 						TYPE_OperateCntSize },
	{	5,		2,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxImageAdjustModeTextPhoto, 				TYPE_OperateCntSize },
	{	5,		2,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxImageAdjustModePhoto, 					TYPE_OperateCntSize },
	{	5,		2,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxImageAdjustModePencil, 					TYPE_OperateCntSize },
	{	5,		2,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxImageAdjustModeCopyOriginal, 				TYPE_OperateCntSize },
	{	5,		3,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxOriginalNotDuplex, 						TYPE_OperateCntSize },
	{	5,		3,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxOriginalDuplex, 							TYPE_OperateCntSize },
	{	5,		3,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxOriginalFront_Duplex, 					TYPE_OperateCntSize },
	{	5,		3,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxOriginalZ, 								TYPE_OperateCntSize },
	{	5,		3,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxOriginalLongPaper, 						TYPE_OperateCntSize },
	{	5,		3,		7,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxOriginalDirtReducation, 					TYPE_OperateCntSize },
	{	5,		3,		12,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxOriginalDeleteBlankPage, 					TYPE_OperateCntSize },// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
	{	5,		3,		13,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxOriginalContinuousRead, 					TYPE_OperateCntSize },	// 2011/04/26 Iwata Minerva/Zeus UV-00866 項目追加Start.
	{	5,		3,		14,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxOriginalSameWidthEnable, 					TYPE_OperateCntSize },	
	{	5,		3,		15,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxOriginalMixedWidthEnable,					TYPE_OperateCntSize },	
	{	5,		3,		16,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxOriginalThinPaperMode, 					TYPE_OperateCntSize },	// 2011/04/26 Iwata Minerva/Zeus UV-00866 項目追加End.
	{	5,		4,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxResolution200_100, 						TYPE_OperateCntSize },
	{	5,		4,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxResolution200, 							TYPE_OperateCntSize },
	{	5,		4,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxResolution400, 							TYPE_OperateCntSize },
	{	5,		4,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxResolution600, 							TYPE_OperateCntSize },
	{	5,		5,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatJpeg, 							TYPE_OperateCntSize },
	{	5,		5,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatTiff, 							TYPE_OperateCntSize },
	{	5,		5,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatPdf, 							TYPE_OperateCntSize },
	{	5,		5,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatCompactPdf, 					TYPE_OperateCntSize },
	{	5,		5,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatXps, 							TYPE_OperateCntSize },
	{	5,		5,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatCompactXps, 					TYPE_OperateCntSize },
	{	5,		5,		7,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatSearchablePdf, 					TYPE_OperateCntSize },
	{	5,		5,		8,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatOutlinePdf, 					TYPE_OperateCntSize },
	{	5,		5,		9,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatEncryptionPDF_Pass, 			TYPE_OperateCntSize },
	{	5,		5,		10,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatEncryptionPDF_DigitalId, 		TYPE_OperateCntSize },
	{	5,		5,		11,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatStampCompositionText, 			TYPE_OperateCntSize },
	{	5,		5,		12,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatStampCompositionImag, 			TYPE_OperateCntSize },
	{	5,		5,		13,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatPdfA_1a, 						TYPE_OperateCntSize },	// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
	{	5,		5,		14,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatPdfA_1b, 						TYPE_OperateCntSize },	// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
	{	5,		5,		15,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatOOXML, 							TYPE_OperateCntSize },	// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
	{	5,		5,		16,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatLinearizedPdf, 					TYPE_OperateCntSize },	// 2011/01/13 sone Minerva/Zeus UV-00866 項目追加.
	{	5,		5,		17,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatDOCX, 							TYPE_OperateCntSize },			// 2012/02/22 amiya 【ZeusMLK】UV-00973 紙資料をMicrosoft社のOffice(Word、Excel)形式のファイルに変換したい START.
	{	5,		5,		18,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatXLSX, 							TYPE_OperateCntSize },
	{	5,		5,		19,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatOCR, 							TYPE_OperateCntSize },
	{	5,		5,		20,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatDOCX_Text, 						TYPE_OperateCntSize },
	{	5,		5,		21,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatDOCX_Image, 					TYPE_OperateCntSize },
	{	5,		5,		22,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatDOCX_TextAndImage, 				TYPE_OperateCntSize },
	{	5,		5,		23,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatXLSX_TextAndImage, 				TYPE_OperateCntSize },
	{	5,		5,		24,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxFileFormatXLSX_Text, 						TYPE_OperateCntSize },	// 2012/02/22 amiya 【ZeusMLK】UV-00973 紙資料をMicrosoft社のOffice(Word、Excel)形式のファイルに変換したい END.
	{	5,		6,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxCommSendQuick, 							TYPE_OperateCntSize },
	{	5,		6,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxCommSendMemory, 							TYPE_OperateCntSize },
	{	5,		6,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxCommSendFCode, 							TYPE_OperateCntSize },
	{	5,		6,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxCommSendPolling, 							TYPE_OperateCntSize },
	{	5,		6,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxCommReceivePolling, 						TYPE_OperateCntSize },
	{	5,		6,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxCommSendTimer, 							TYPE_OperateCntSize },
	{	5,		6,		7,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxCommSendPassward, 						TYPE_OperateCntSize },
	{	5,		6,		8,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxCommOverseasMode, 						TYPE_OperateCntSize },
	{	5,		6,		9,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxCommECMOff, 								TYPE_OperateCntSize },
	{	5,		6,		10,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxCommV34Off, 								TYPE_OperateCntSize },
	{	5,		6,		11,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxCommLineSel, 								TYPE_OperateCntSize },
	{	5,		6,		12,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxCommCallOrigin, 							TYPE_OperateCntSize },
	{	5,		7,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxAppBookRepeat, 							TYPE_OperateCntSize },
	{	5,		7,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxAppDateStamp, 							TYPE_OperateCntSize },
	{	5,		7,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxAppPageStamp, 							TYPE_OperateCntSize },
	{	5,		7,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxAppHeaderFooterStamp, 					TYPE_OperateCntSize },
	{	5,		7,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxAppStamp, 								TYPE_OperateCntSize },
	{	5,		7,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxAppFilingNumber, 							TYPE_OperateCntSize },
	{	5,		7,		7,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxAppWithPrint, 							TYPE_OperateCntSize },
	{	5,		7,		8,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntFaxAppPreview, 								TYPE_OperateCntSize },
	{	6,		1,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintScoreFunc, 								TYPE_OperateCntSize },		//2013/05/11 tadamasa UV-01234 tadamasa ぷりんと楽譜対応.
	{	6,		1,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintScoreFuncOutputNum, 					TYPE_OperateCntSize },		//2013/05/11 tadamasa UV-01234 tadamasa ぷりんと楽譜対応.
	{	6,		1,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintScoreFuncOutputNumOfSheet, 				TYPE_OperateCntSize },		//2013/05/11 tadamasa UV-01234 tadamasa ぷりんと楽譜対応.
	{	7,		1,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_NFC_Pairing },
	{	7,		1,		2,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_NFC_Login },
	{	7,		2,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_BLE_Pairing },
	{	7,		2,		2,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_BLE_Login },
	{	7,		3,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_QRCode },
	{	7,		10,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_BizhubRA_PanelLink },
	{	7,		10,		2,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_BizhubRA_KeybordLink },
	{	7,		10,		3,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_BizhubRA_AddressLink },
	{	7,		10,		4,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_BizhubRA_Scan },
	{	7,		10,		10,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_BizhubRA_AdminLogin },
	{	7,		10,		11,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_BizhubRA_ServiceLogin },
	{	7,		11,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPsMobilePrint,								TYPE_OperateCntSize },
	{	7,		12,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPinPrintAccumulation,						TYPE_OperateCntSize },								// 2017/01/12 AirPrint1.7対応 WP-891 HCL-Vinay Added.
	{	8,		1,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_WebBrowser },
	{	8,		1,		2,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_WebBrowser_AccessLog },
	{	8,		1,		3,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_WebBrowser_PdfViewer },
	{	8,		2,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Ubiquitous },
	{	8,		2,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntUbiquitousPrint,								TYPE_OperateCntSize },
	{	8,		2,		3,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Ubiquitous_RequestPrint },
	{	8,		2,		4,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Ubiquitous_Admin },					//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	8,		2,		5,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Ubiquitous_TopologyListUpdateNum },	//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	8,		2,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntUbiquitousPrintSpoolFileNum,					TYPE_OperateCntSize },								//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	8,		2,		7,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntUbiquitousPrintHddUsedSize,					TYPE_OperateCntSize },								//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	9,		1,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_User },
	{	9,		1,		2,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_Admin },
	{	9,		1,		3,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_Service },
	{	9,		2,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Browser_InfoView },
	{	9,		2,		11,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Browser_JobExecList },
	{	9,		2,		12,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Browser_JobHistoryList },
	{	9,		2,		13,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Browser_JobCommList },
	{	9,		2,		21,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Browser_BoxOpen },
	{	9,		2,		22,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Browser_SysBoxOpen },
	{	9,		2,		31,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Browser_DirectPrint },
	{	9,		2,		41,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Browser_ShortAddr },
	{	9,		2,		42,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Browser_GroupAddr },
	{	9,		2,		43,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Browser_ProgramAddr },
	{	9,		2,		44,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Browser_TempAddr },
	{	9,		2,		45,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Browser_EmailSubject },
	{	9,		2,		46,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Browser_EmailContent },
	{	9,		2,		51,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Browser_CustomizeSet },
	{	9,		2,		61,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Browser_BookMark },
	{	9,		2,		71,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Browser_SiteMap },
	{	9,		3,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_PC_InfoView },
	{	9,		3,		11,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_PC_JobExecList },
	{	9,		3,		12,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_PC_JobHistoryList },
	{	9,		3,		13,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_PC_JobCommList },
	{	9,		3,		21,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_PC_BoxOpen },
	{	9,		3,		22,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_PC_SysBoxOpen },
	{	9,		3,		31,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_PC_DirectPrint },
	{	9,		3,		41,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_PC_ShortAddr },
	{	9,		3,		42,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_PC_GroupAddr },
	{	9,		3,		43,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_PC_ProgramAddr },
	{	9,		3,		44,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_PC_TempAddr },
	{	9,		3,		45,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_PC_EmailSubject },
	{	9,		3,		46,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_PC_EmailContent },
	{	9,		3,		51,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_PC_CustomizeSet },
	{	9,		3,		61,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_PC_BookMark },
	{	9,		3,		71,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_PC_SiteMap },
	{	9,		4,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Tablet_InfoView },
	{	9,		4,		11,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Tablet_JobExecList },
	{	9,		4,		12,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Tablet_JobHistoryList },
	{	9,		4,		13,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Tablet_JobCommList },
	{	9,		4,		21,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Tablet_BoxOpen },
	{	9,		4,		22,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Tablet_SysBoxOpen },
	{	9,		4,		31,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Tablet_DirectPrint },
	{	9,		4,		41,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Tablet_ShortAddr },
	{	9,		4,		42,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Tablet_GroupAddr },
	{	9,		4,		43,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Tablet_ProgramAddr },
	{	9,		4,		44,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Tablet_TempAddr },
	{	9,		4,		45,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Tablet_EmailSubject },
	{	9,		4,		46,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Tablet_EmailContent },
	{	9,		4,		51,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Tablet_CustomizeSet },
	{	9,		4,		61,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Tablet_BookMark },
	{	9,		4,		71,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_U_Tablet_SiteMap },
	{	9,		5,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_PC_TabSelect },			// 2015/10/23 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加)_仕様変更.
	{	9,		5,		2,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_PC_ImportExport },
	{	9,		5,		3,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_PC_FontMacro },
	{	9,		5,		11,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_PC_SystemSet },
	{	9,		5,		21,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_PC_Security },
	{	9,		5,		31,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_PC_AuthTabSelect },		// 2015/10/23 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加)_仕様変更.
	{	9,		5,		32,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_PC_AuthUserRegist },
	{	9,		5,		33,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_PC_AuthTrackRegist },
	{	9,		5,		41,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_PC_Network },
	{	9,		5,		51,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_PC_Box },
	{	9,		5,		61,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_PC_PrinterSet },
	{	9,		5,		71,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_PC_AddrRegist },
	{	9,		5,		81,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_PC_FaxSet },
	{	9,		5,		91,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_PC_PurposesSet },
	{	9,		5,		101,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_PC_CustomizeSet },
	{	9,		5,		111,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_PC_SiteMap },
	{	9,		5,		121,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_PC_Login_CopyProtect },	// 2015/10/23 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加)_仕様変更.
	{	9,		5,		122,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_PC_Login_StampData },	// 2015/10/23 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加)_仕様変更.
	{	9,		5,		123,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_PC_Login_FontMacro },	// 2015/10/23 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加)_仕様変更.
	{	9,		6,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_Tablet_TabSelect },		// 2015/10/23 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加)_仕様変更.
	{	9,		6,		2,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_Tablet_ImportExport },
	{	9,		6,		3,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_Tablet_FontMacro },
	{	9,		6,		11,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_Tablet_SystemSet },
	{	9,		6,		21,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_Tablet_Security },
	{	9,		6,		31,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_Tablet_AuthTabSelect },	// 2015/10/23 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加)_仕様変更.
	{	9,		6,		32,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_Tablet_AuthUserRegist },
	{	9,		6,		33,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_Tablet_AuthTrackRegist },
	{	9,		6,		41,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_Tablet_Network },
	{	9,		6,		51,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_Tablet_Box },
	{	9,		6,		61,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_Tablet_PrinterSet },
	{	9,		6,		71,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_Tablet_AddrRegist },
	{	9,		6,		81,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_Tablet_FaxSet },
	{	9,		6,		91,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_Tablet_PurposesSet },
	{	9,		6,		101,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_Tablet_CustomizeSet },
	{	9,		6,		111,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_Tablet_SiteMap },
	{	9,		6,		121,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_Tablet_Login_CopyProtect },	// 2015/10/23 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加)_仕様変更.
	{	9,		6,		122,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_Tablet_Login_StampData },	// 2015/10/23 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加)_仕様変更.
	{	9,		6,		123,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PSWC_A_Tablet_Login_FontMacro },	// 2015/10/23 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加)_仕様変更.
	{	9,		7,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PC_CSRC },
	{	9,		7,		11,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PC_ClientFWUpdate },
	{	9,		8,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Tablet_CSRC },
	{	9,		8,		11,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Tablet_ClientFWUpdate },
//2023/10/13 FPT-HaDV8 Fix_制御構成変更_No.73 [IT6_23-1643] Start.
	{	11,		1,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCnt_VirusDetection,								TYPE_OperateCntSize },
	{	11,		2,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCnt_PasswordAttackDetection,					TYPE_OperateCntSize },
	{	11,		2,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCnt_AttestationAccessAttackDetection,			TYPE_OperateCntSize },
//2023/10/13 FPT-HaDV8 Fix_制御構成変更_No.73 [IT6_23-1643] End.
	{	15,		1,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Auth_Login },
	{	15,		1,		2,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Auth_Admin },
	{	15,		1,		3,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Auth_Service },
	{	15,		2,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintCounter,								TYPE_OperateCntSize },
	{	15,		2,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintUserChoice,							TYPE_OperateCntSize },
	{	15,		2,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintFaxSetting,							TYPE_OperateCntSize },
	{	15,		2,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintTrackCounter,						TYPE_OperateCntSize },
	{	15,		2,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintNetworkConfig,						TYPE_OperateCntSize },
	{	15,		2,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintProgramKey,							TYPE_OperateCntSize },
	{	15,		2,		7,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintGroupAddr,							TYPE_OperateCntSize },
	{	15,		2,		8,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintReduceAddr,							TYPE_OperateCntSize },
	{	15,		2,		9,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintTitleSubject,						TYPE_OperateCntSize },
	{	15,		2,		10,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintPrinterSetting,						TYPE_OperateCntSize },
	{	15,		2,		11,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintScanTx,								TYPE_OperateCntSize },
	{	15,		2,		12,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintUnitCheck,							TYPE_OperateCntSize },
	{	15,		2,		13,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintSendOperationLog,					TYPE_OperateCntSize },
	{	15,		2,		14,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintMachineInfo,							TYPE_OperateCntSize },
	{	15,		2,		15,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintServiceAdjust,						TYPE_OperateCntSize },
	{	15,		2,		16,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintParameter,							TYPE_OperateCntSize },
	{	15,		2,		17,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintServiceParameter,					TYPE_OperateCntSize },
	{	15,		2,		18,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintProtocolTraceNow,					TYPE_OperateCntSize },
	{	15,		2,		19,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintProtocolTraceErr,					TYPE_OperateCntSize },
	{	15,		2,		20,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintTotalCounterHistory,					TYPE_OperateCntSize },
	{	15,		2,		21,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintFEKDLKRequestCode,					TYPE_OperateCntSize },
	{	15,		2,		22,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintFEKDLKDefeasanceCompletionCode,		TYPE_OperateCntSize },
	{	15,		2,		23,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintFEKDLKRestorationDemandCode,			TYPE_OperateCntSize },
	{	15,		2,		24,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintPsFont,								TYPE_OperateCntSize },
	{	15,		2,		25,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintPclFont,								TYPE_OperateCntSize },
	{	15,		2,		26,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintPclDemo,								TYPE_OperateCntSize },
	{	15,		2,		27,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintOOXMLFont,							TYPE_OperateCntSize },
	{	15,		2,		28,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintBanner,								TYPE_OperateCntSize },
	{	15,		2,		29,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintComm,								TYPE_OperateCntSize },
	{	15,		2,		30,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintTx,									TYPE_OperateCntSize },
	{	15,		2,		31,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintRx,									TYPE_OperateCntSize },
	{	15,		2,		32,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintTxReserve,							TYPE_OperateCntSize },
	{	15,		2,		33,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintTxResult,							TYPE_OperateCntSize },
	{	15,		2,		34,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintSeqPollRxResult,						TYPE_OperateCntSize },
	{	15,		2,		35,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintConfidentialRx,						TYPE_OperateCntSize },
	{	15,		2,		36,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintBulletinTx,							TYPE_OperateCntSize },
	{	15,		2,		37,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintRelayResult,							TYPE_OperateCntSize },
	{	15,		2,		38,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintRelayReqReception,					TYPE_OperateCntSize },
	{	15,		2,		39,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintPollTxReserve,						TYPE_OperateCntSize },
	{	15,		2,		40,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintPollRxReserve,						TYPE_OperateCntSize },
	{	15,		2,		41,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintPollTx,								TYPE_OperateCntSize },
	{	15,		2,		42,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintPollRx,								TYPE_OperateCntSize },
	{	15,		2,		43,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintBroadcastTxReserve,					TYPE_OperateCntSize },
	{	15,		2,		44,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintBroadcastTx,							TYPE_OperateCntSize },
	{	15,		2,		45,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintSeqPollRxReserve,					TYPE_OperateCntSize },
	{	15,		2,		46,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintSeqPollRx,							TYPE_OperateCntSize },
	{	15,		2,		47,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintPcFaxErr,							TYPE_OperateCntSize },
	{	15,		2,		48,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintIfaxTxErr,							TYPE_OperateCntSize },
	{	15,		2,		49,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintIfaxRxText,							TYPE_OperateCntSize },
	{	15,		2,		50,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintIfaxRxErr,							TYPE_OperateCntSize },
	{	15,		2,		51,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintLogRdsJnlEmail,						TYPE_OperateCntSize },
	{	15,		2,		52,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintLogRdsJnlDataModem,					TYPE_OperateCntSize },
	{	15,		2,		53,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintDSNMassege,							TYPE_OperateCntSize },
	{	15,		2,		54,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintMDNMassege,							TYPE_OperateCntSize },
	{	15,		2,		55,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintSelfCheckResult,						TYPE_OperateCntSize },
	{	15,		2,		56,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintTPMSelfCheck,						TYPE_OperateCntSize },
	{	15,		2,		57,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintVirusScanRiskLog,					TYPE_OperateCntSize },
	{	15,		2,		58,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntListPrintVirusScanScanLog,					TYPE_OperateCntSize },
//2022/12/22 FPT-VuongVD2 IT6_2.3 制御構成変更_No.39-2 Start.	
	{	15,		2,		59,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntVirusScan_RiskLogList,						TYPE_OperateCntSize },
	{	15,		2,		60,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntVirusScan_ScanLogList,						TYPE_OperateCntSize },
	{	15,		2,		70,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PaperTypeAutoDetection },
	{	15,		2,		71,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_BasisWeightTableOrientation },
	{	15,		2,		72,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_BasisWeightDetection },
	{	15,		2,		73,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_BasisWeightClassThreshold },
	{	15,		2,		74,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_BasisWeightClassVariation },
	{	15,		2,		75,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_RecycledPaperIdentification },
	{	15,		2,		76,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_EnvelopeIdentification },
	{	15,		2,		77,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PlainPaperDetection },
	{	15,		2,		78,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ColorPaperJudgment },
	{	15,		2,		79,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PaperReplenishmentDetection },
	{	15,		2,		90,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_InstructedFromWWRSPF },
//2023/03/11 FPT-VuongVD2 IT6_2.3 OP_BTS-42282 Start.
//2023/03/03 FPT-VuongVD2 IT6_2.3 OP_BTS-42270 Start.
	{	15,		2,		91,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_AcquireWithFleetRMM },
	{	15,		2,		92,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_GetFromServiceScreen },
	{	15,		2,		93,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_SettingFromWWRSPF },
	{	15,		2,		94,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_SetWithFleetRMM },
	{	15,		2,		95,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_SetFromServiceScreen },
//2023/03/03 FPT-VuongVD2 IT6_2.3 OP_BTS-42270 End.
//2023/03/11 FPT-VuongVD2 IT6_2.3 OP_BTS-42282 End.
//2022/12/22 FPT-VuongVD2 IT6_2.3 制御構成変更_No.39-2 End.	
	{	15,		2,		101,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ExpertAdjustment_AELevel },
	{	15,		2,		102,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ExpertAdjustment_Printer },
	{	15,		2,		103,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ExpertAdjustment_Finisher },
	{	15,		2,		104,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ExpertAdjustment_Density },
	{	15,		2,		105,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ExpertAdjustment_ImageControl },
	{	15,		2,		106,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ExpertAdjustment_PaperPosition },
	{	15,		2,		107,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ExpertAdjustment_Color },
	{	15,		2,		108,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ExpertAdjustment_Gradation },
	{	15,		2,		109,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ExpertAdjustment_DensityBalance },
	{	15,		2,		110,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ExpertAdjustment_ScanArea },
	{	15,		2,		111,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ExpertAdjustment_ADF },
	{	15,		2,		112,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ExpertAdjustment_NoiseRemoveSet },
	{	15,		2,		113,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ExpertAdjustment_EndSlip },
	{	15,		2,		114,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ExpertAdjustment_UserPaperSet },
	{	15,		2,		115,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ExpertAdjustment_Erase },
	{	15,		2,		116,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ExpertAdjustment_PSdesignerSet },
	{	15,		2,		117,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ExpertAdjustment_DrumDry },
#ifdef DEF_A4_Cus	//20160503 VenusMLK CQ524880 HCL_Pawan Start.
	{	15,		2,		118,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ExpertAdjustment_MainScanAdjust },
	{	15,		2,		119,	RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ExpertAdjustment_TemporaryRescue },
#endif	//20160503 VenusMLK CQ524880 HCL_Pawan End.
	{	15,		3,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_UnitInfoView },
	{	15,		3,		2,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_LanguageKey },
	{	15,		3,		3,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_FuncSearchKey },
// 2015/10/19 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加) END.
	{	15,		4,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PowerSaveLowPower },			// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		4,		2,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PowerSaveSleep },				// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		4,		3,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PowerSaveDeepSleep },			// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		4,		4,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PowerSaveDeepSleepHigh },		// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		4,		5,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PowerSaveSubPowerOff },			// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		4,		6,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PowerSaveErP },					// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		5,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintNumPerJob_1,							TYPE_OperateCntSize },							// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		5,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintNumPerJob_2_5,							TYPE_OperateCntSize },							// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		5,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintNumPerJob_6_10,							TYPE_OperateCntSize },							// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		5,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintNumPerJob_11_25,						TYPE_OperateCntSize },							// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		5,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintNumPerJob_26_50,						TYPE_OperateCntSize },							// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		5,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintNumPerJob_51_100,						TYPE_OperateCntSize },							// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		5,		7,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintNumPerJob_101,							TYPE_OperateCntSize },							// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		6,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanDataSizePerJob_0_50KB,					TYPE_OperateCntSize },							// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		6,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanDataSizePerJob_50_100KB,					TYPE_OperateCntSize },							// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		6,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanDataSizePerJob_100_500KB,				TYPE_OperateCntSize },							// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		6,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanDataSizePerJob_500_1000KB,				TYPE_OperateCntSize },							// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		6,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanDataSizePerJob_1_5MB,					TYPE_OperateCntSize },							// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		6,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntScanDataSizePerJob_5MB,						TYPE_OperateCntSize },							// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		7,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintDataSizePerJob_0_100KB,					TYPE_OperateCntSize },							// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		7,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintDataSizePerJob_100_500KB,				TYPE_OperateCntSize },							// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		7,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintDataSizePerJob_500_1000KB,				TYPE_OperateCntSize },							// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		7,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintDataSizePerJob_1_5MB,					TYPE_OperateCntSize },							// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		7,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintDataSizePerJob_5_10MB,					TYPE_OperateCntSize },							// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		7,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntPrintDataSizePerJob_10MB,					TYPE_OperateCntSize },							// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		8,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_HddFreeSizeGeneral_0_20 },		// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		8,		2,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_HddFreeSizeGeneral_21_40 },		// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		8,		3,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_HddFreeSizeGeneral_41_60 },		// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		8,		4,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_HddFreeSizeGeneral_61_80 },		// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		8,		5,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_HddFreeSizeGeneral_81_100 },	// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		9,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_HddFreeSizeBox_0_20 },			// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		9,		2,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_HddFreeSizeBox_21_40 },			// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		9,		3,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_HddFreeSizeBox_41_60 },			// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		9,		4,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_HddFreeSizeBox_61_80 },			// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		9,		5,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_HddFreeSizeBox_81_100 },		// 2015/12/21 日立 森本郁 [MAv4][RQ-502]機能カウンタ追加対応.
	{	15,		10,		1,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntUbiquitousPrintDataSizePerJob_0_1KB,			TYPE_OperateCntSize },		//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	15,		10,		2,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntUbiquitousPrintDataSizePerJob_1KB_10KB,		TYPE_OperateCntSize },		//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	15,		10,		3,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntUbiquitousPrintDataSizePerJob_10KB_50KB,		TYPE_OperateCntSize },		//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	15,		10,		4,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntUbiquitousPrintDataSizePerJob_50KB_100KB,		TYPE_OperateCntSize },		//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	15,		10,		5,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntUbiquitousPrintDataSizePerJob_100KB_250KB,	TYPE_OperateCntSize },		//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	15,		10,		6,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntUbiquitousPrintDataSizePerJob_250KB_500KB,	TYPE_OperateCntSize },		//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	15,		10,		7,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntUbiquitousPrintDataSizePerJob_500KB_1MB,		TYPE_OperateCntSize },		//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	15,		10,		8,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntUbiquitousPrintDataSizePerJob_1MB_10MB,		TYPE_OperateCntSize },		//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	15,		10,		9,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntUbiquitousPrintDataSizePerJob_10MB_50MB,		TYPE_OperateCntSize },		//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	15,		10,		10,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntUbiquitousPrintDataSizePerJob_50MB_100MB,		TYPE_OperateCntSize },		//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	15,		10,		11,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntUbiquitousPrintDataSizePerJob_100MB_250MB,	TYPE_OperateCntSize },		//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	15,		10,		12,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntUbiquitousPrintDataSizePerJob_250MB_500MB,	TYPE_OperateCntSize },		//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	15,		10,		13,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntUbiquitousPrintDataSizePerJob_500MB_1GB,		TYPE_OperateCntSize },		//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	15,		10,		14,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntUbiquitousPrintDataSizePerJob_1GB_2GB,		TYPE_OperateCntSize },		//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	15,		10,		15,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntUbiquitousPrintDataSizePerJob_2GB_3GB,		TYPE_OperateCntSize },		//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	15,		10,		16,		RDS_TYPE_CNT_JOB,	TYPE_FuncUseCntUbiquitousPrintDataSizePerJob_3GB_OVER,		TYPE_OperateCntSize },		//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	15,		11,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Reboot},	//2021/10/15 FPT-NhuongNV1 IT6_MinevaSSBK 制御構成変更_No.40.
//2022/11/16 FPT-MinhNKN IT6_2.3 制御構成変更_No.51 Start.
	//{	15,		12,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ServicelessMaintenanceUsage},		//2022/01/24 FPT-NhuongNV1 IT6_2.2MR2 制御構成変更_No.33_1.
	{	15,		12,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Screen_FaxTxError},
	{	15,		12,		2,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Screen_FaxRxError},
	{	15,		12,		3,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Screen_ScanTxError},
	{	15,		12,		4,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Screen_JobListDetail_FaxTxError},
	{	15,		12,		5,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Screen_JobListDetail_FaxRxError},
	{	15,		12,		6,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Screen_JobListDetail_ScanTxError},
	{	15,		12,		7,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Screen_Home01},
	{	15,		12,		8,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Screen_Home02},
	{	15,		12,		9,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Screen_Home03},
	{	15,		12,		10,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Screen_Home04},
	{	15,		12,		11,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Screen_Home05},
	{	15,		12,		12,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Screen_Home06},
	{	15,		12,		13,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Screen_Home07},
	{	15,		12,		14,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Screen_Home08},
	{	15,		12,		15,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Screen_Home09},
	{	15,		12,		16,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Copy_Simple},
	{	15,		12,		17,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Copy},
	{	15,		12,		18,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Copy_IDCard},
	{	15,		12,		19,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Copy_ShadowErase},
	{	15,		12,		20,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_CreateBookmark},
	{	15,		12,		21,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Scan_SendEmail},
	{	15,		12,		22,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Scan_SaveToBox},
	{	15,		12,		23,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Scan_SendToPC},
	{	15,		12,		24,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Scan_IDCard},
	{	15,		12,		25,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Scan_EmailToSelf},
	{	15,		12,		26,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Scan_NotifyURL},
	{	15,		12,		27,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Scan_SendToSelf},
	{	15,		12,		28,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Fax},
	{	15,		12,		29,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Fax_Internet},
	{	15,		12,		30,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Fax_IP},
	{	15,		12,		31,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Fax_SIP},
	{	15,		12,		32,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Print_Secure},
	{	15,		12,		33,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Fax_Inbox},
	{	15,		12,		34,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Print_Auth},
	{	15,		12,		35,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Print_USB},
//2022/11/16 FPT-MinhNKN IT6_2.3 制御構成変更_No.51 End.
	{	15,		13,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_UserSetup},							//2022/01/24 FPT-NhuongNV1 IT6_2.2MR2 制御構成変更_No.34_1.
	{	15,		13,		2,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_StartFromSettingMenuScreen},		//2022/11/15 FPT-HuuTX IT6_2.3 制御構成変更_No.52.
	{	15,		13,		3,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_FailedCheckWWRSPF},					//2022/11/15 FPT-HuuTX IT6_2.3 制御構成変更_No.52.
//2022/12/22 FPT-VuongVD2 IT6_2.3 制御構成変更_No.39-2 Start.	
	{	15,		14,		1,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_CreateFolder_BasicStyle},
	{	15,		14,		2,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_SimpleCopy_BasicStyle},
	{	15,		14,		3,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Copy_BasicStyle},
	{	15,		14,		4,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_IDCardCopy_BasicStyle},
	{	15,		14,		5,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ShadowEraseCopy_BasicStyle},
	{	15,		14,		6,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_MakeABooklet_BasicStyle},
	{	15,		14,		7,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_CopyFullFunctionality_BasicStyle},
	{	15,		14,		8,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Scan_EmailTransmission_BasicStyle},
	{	15,		14,		9,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Scan_SaveInUserBox_BasicStyle},
	{	15,		14,		10,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Scan_SendToPC_BasicStyle},
	{	15,		14,		11,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_SaveToUSBMemory_BasicStyle},
	{	15,		14,		12,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Scan_IDCard_BasicStyle},
	{	15,		14,		13,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Scan_EmailMe_BasicStyle},
	{	15,		14,		14,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Scan_NotifyURL_BasicStyle},
	{	15,		14,		15,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Scan_SendToMe_BasicStyle},
	{	15,		14,		16,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Scan_FullFunctionality_BasicStyle},
	{	15,		14,		17,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PKIEncryptedScan_BasicStyle},
	{	15,		14,		18,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Fax_BasicStyle},
	{	15,		14,		19,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_InternetFax_BasicStyle},
	{	15,		14,		20,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_IPAddressFax_BasicStyle},
	{	15,		14,		21,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_IPFax_BasicStyle},
	{	15,		14,		22,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_FaxFullFeatured_BasicStyle},
	{	15,		14,		23,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_SecurityPrinting_BasicStyle},
	{	15,		14,		24,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PrintAfterViewingTheTax_BasicStyle},
	{	15,		14,		25,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_AuthenticationAndPrinting_BasicStyle},
	{	15,		14,		26,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_USBMemoryPrinting_BasicStyle},
	{	15,		14,		27,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PKIEncryptedDocumentPrint_BasicStyle},
	{	15,		14,		28,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Box_BasicStyle},
	{	15,		14,		29,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_UbiquitousPrint_BasicStyle},
	{	15,		14,		30,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_WebBrowserV2_BasicStyle},
	{	15,		14,		31,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Marketplace_BasicStyle},
	{	15,		14,		32,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_DownloadAPP_BasicStyle},
	{	15,		14,		33,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_AppGroup_BasicStyle},
	{	15,		14,		34,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_UsefulAPP_BasicStyle},
	{	15,		14,		35,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_Favorite_BasicStyle},	
	{	15,		14,		36,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_FavoriteList_BasicStyle},
	{	15,		14,		37,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_AddressBook_BasicStyle},
	{	15,		14,		38,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_PaperSettings_BasicStyle},
	{	15,		14,		39,		RDS_TYPE_CNT_USER,	TYPE_FuncUseCntSize,										TYPE_OperateCnt_ClassicStyleSwitch_BasicStyle},
//2022/12/22 FPT-VuongVD2 IT6_2.3 制御構成変更_No.39-2 End.	
};

// 2014/11/13 H.Ookuma RQ-250対応【機能利用頻度の収集機能（組合せカウンタ）】 start.
typedef struct
{
	type_id majorId;							// 分類1のID.
	type_id minorId;							// 分類2のID.
	type_id c_Id;								// 分類3のID.
	TYPE_FuncCombinationUseCntType e_scanType;	// CommonAPIコール用Enum(スキャン送信種別).
	TYPE_FuncCombinationUseCntFile e_fileType;	// CommonAPIコール用Enum(スキャンファイル種別).
}_IDtoFuncCountCombinationTable;

static _IDtoFuncCountCombinationTable FuncTable_FuncCountCombination[] =
{
	//	major	minor	c		// Scan Type										Scan File.
	{	4,		15,		1,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_JPEG},			// Scan　To　E-mail&JPEG.
	{	4,		15,		2,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_TIFF},			// Scan　To　E-mail&TIFF.
	{	4,		15,		3,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_PDF},			// Scan　To　E-mail&PDF.
	{	4,		15,		4,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_CPDF},			// Scan　To　E-mail&コンパクトPDF.
	{	4,		15,		5,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_XPS},			// Scan　To　E-mail&XPS.
	{	4,		15,		6,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_CXPS},			// Scan　To　E-mail&コンパクトXPS.
	{	4,		15,		7,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_SPDF},			// Scan　To　E-mail&サーチャブルPDF.
	{	4,		15,		8,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_OPDF},			// Scan　To　E-mail&アウトラインPDF.
	{	4,		15,		9,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_PDFPassword},	// Scan　To　E-mail&PDF暗号化(パスワード).
	{	4,		15,		10,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_PDFDigitalID},	// Scan　To　E-mail&PDF暗号化(デジタルID).
	{	4,		15,		11,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_StampText},		// Scan　To　E-mail&スタンプ合成方法(文字).
	{	4,		15,		12,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_StampImage},		// Scan　To　E-mail&スタンプ合成方法(画像).
	{	4,		15,		13,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_PDFA1a},			// Scan　To　E-mail&PDF/A-1a.
	{	4,		15,		14,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_PDFA1b},			// Scan　To　E-mail&PDF/A-1b.
	{	4,		15,		15,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_PPTX},			// Scan　To　E-mail&PPTX.
	{	4,		15,		16,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_PDFWeb},			// Scan　To　E-mail&PDFのWeb表示最適化.
	{	4,		15,		17,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_DOCX},			// Scan　To　E-mail&DOCX.
	{	4,		15,		18,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_XLSX},			// Scan　To　E-mail&XLSX.
	{	4,		15,		19,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_OCR},			// Scan　To　E-mail&OCR(文字認識).
	{	4,		15,		20,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_DOCXText},		// Scan　To　E-mail&DOCX(文字優先).
	{	4,		15,		21,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_DOCXImage},		// Scan　To　E-mail&DOCX(画像優先).
	{	4,		15,		22,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_DOCXImageText},	// Scan　To　E-mail&DOCX(画像と文字).
	{	4,		15,		23,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_XLSXImageText},	// Scan　To　E-mail&XLSX(画像と文字).
	{	4,		15,		24,		TYPE_FuncCombinationUseCntType_ScanToEmail,			TYPE_FuncCombinationUseCntFile_XLSXText},		// Scan　To　E-mail&XLSX(文字のみ).
	{	4,		15,		25,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_JPEG},			// Scan  To  FTP&JPEG.
	{	4,		15,		26,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_TIFF},			// Scan  To  FTP&TIFF.
	{	4,		15,		27,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_PDF},			// Scan  To  FTP&PDF.
	{	4,		15,		28,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_CPDF},			// Scan  To  FTP&コンパクトPDF.
	{	4,		15,		29,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_XPS},			// Scan  To  FTP&XPS.
	{	4,		15,		30,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_CXPS},			// Scan  To  FTP&コンパクトXPS.
	{	4,		15,		31,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_SPDF},			// Scan  To  FTP&サーチャブルPDF.
	{	4,		15,		32,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_OPDF},			// Scan  To  FTP&アウトラインPDF.
	{	4,		15,		33,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_PDFPassword},	// Scan  To  FTP&PDF暗号化(パスワード).
	{	4,		15,		34,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_PDFDigitalID},	// Scan  To  FTP&PDF暗号化(デジタルID).
	{	4,		15,		35,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_StampText},		// Scan  To  FTP&スタンプ合成方法(文字).
	{	4,		15,		36,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_StampImage},		// Scan  To  FTP&スタンプ合成方法(画像).
	{	4,		15,		37,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_PDFA1a},			// Scan  To  FTP&PDF/A-1a.
	{	4,		15,		38,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_PDFA1b},			// Scan  To  FTP&PDF/A-1b.
	{	4,		15,		39,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_PPTX},			// Scan  To  FTP&PPTX.
	{	4,		15,		40,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_PDFWeb},			// Scan  To  FTP&PDFのWeb表示最適化.
	{	4,		15,		41,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_DOCX},			// Scan  To  FTP&DOCX.
	{	4,		15,		42,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_XLSX},			// Scan  To  FTP&XLSX.
	{	4,		15,		43,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_OCR},			// Scan  To  FTP&OCR(文字認識).
	{	4,		15,		44,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_DOCXText},		// Scan  To  FTP&DOCX(文字優先).
	{	4,		15,		45,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_DOCXImage},		// Scan  To  FTP&DOCX(画像優先).
	{	4,		15,		46,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_DOCXImageText},	// Scan  To  FTP&DOCX(画像と文字).
	{	4,		15,		47,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_XLSXImageText},	// Scan  To  FTP&XLSX(画像と文字).
	{	4,		15,		48,		TYPE_FuncCombinationUseCntType_ScanToFTP,			TYPE_FuncCombinationUseCntFile_XLSXText},		// Scan  To  FTP&XLSX(文字のみ).
	{	4,		15,		49,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_JPEG},			// Scan  To  WebDAV&JPEG.
	{	4,		15,		50,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_TIFF},			// Scan  To  WebDAV&TIFF.
	{	4,		15,		51,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_PDF},			// Scan  To  WebDAV&PDF.
	{	4,		15,		52,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_CPDF},			// Scan  To  WebDAV&コンパクトPDF.
	{	4,		15,		53,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_XPS},			// Scan  To  WebDAV&XPS.
	{	4,		15,		54,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_CXPS},			// Scan  To  WebDAV&コンパクトXPS.
	{	4,		15,		55,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_SPDF},			// Scan  To  WebDAV&サーチャブルPDF.
	{	4,		15,		56,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_OPDF},			// Scan  To  WebDAV&アウトラインPDF.
	{	4,		15,		57,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_PDFPassword},	// Scan  To  WebDAV&PDF暗号化(パスワード).
	{	4,		15,		58,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_PDFDigitalID},	// Scan  To  WebDAV&PDF暗号化(デジタルID).
	{	4,		15,		59,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_StampText},		// Scan  To  WebDAV&スタンプ合成方法(文字).
	{	4,		15,		60,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_StampImage},		// Scan  To  WebDAV&スタンプ合成方法(画像).
	{	4,		15,		61,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_PDFA1a},			// Scan  To  WebDAV&PDF/A-1a.
	{	4,		15,		62,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_PDFA1b},			// Scan  To  WebDAV&PDF/A-1b.
	{	4,		15,		63,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_PPTX},			// Scan  To  WebDAV&PPTX.
	{	4,		15,		64,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_PDFWeb},			// Scan  To  WebDAV&PDFのWeb表示最適化.
	{	4,		15,		65,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_DOCX},			// Scan  To  WebDAV&DOCX.
	{	4,		15,		66,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_XLSX},			// Scan  To  WebDAV&XLSX.
	{	4,		15,		67,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_OCR},			// Scan  To  WebDAV&OCR(文字認識).
	{	4,		15,		68,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_DOCXText},		// Scan  To  WebDAV&DOCX(文字優先).
	{	4,		15,		69,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_DOCXImage},		// Scan  To  WebDAV&DOCX(画像優先).
	{	4,		15,		70,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_DOCXImageText},	// Scan  To  WebDAV&DOCX(画像と文字).
	{	4,		15,		71,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_XLSXImageText},	// Scan  To  WebDAV&XLSX(画像と文字).
	{	4,		15,		72,		TYPE_FuncCombinationUseCntType_ScanToWebDAV,		TYPE_FuncCombinationUseCntFile_XLSXText},		// Scan  To  WebDAV&XLSX(文字のみ).
	{	4,		15,		73,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_JPEG},			// Scan  To  PC(SMB）&JPEG.
	{	4,		15,		74,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_TIFF},			// Scan  To  PC(SMB）&TIFF.
	{	4,		15,		75,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_PDF},			// Scan  To  PC(SMB）&PDF.
	{	4,		15,		76,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_CPDF},			// Scan  To  PC(SMB）&コンパクトPDF.
	{	4,		15,		77,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_XPS},			// Scan  To  PC(SMB）&XPS.
	{	4,		15,		78,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_CXPS},			// Scan  To  PC(SMB）&コンパクトXPS.
	{	4,		15,		79,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_SPDF},			// Scan  To  PC(SMB）&サーチャブルPDF.
	{	4,		15,		80,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_OPDF},			// Scan  To  PC(SMB）&アウトラインPDF.
	{	4,		15,		81,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_PDFPassword},	// Scan  To  PC(SMB）&PDF暗号化(パスワード).
	{	4,		15,		82,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_PDFDigitalID},	// Scan  To  PC(SMB）&PDF暗号化(デジタルID).
	{	4,		15,		83,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_StampText},		// Scan  To  PC(SMB）&スタンプ合成方法(文字).
	{	4,		15,		84,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_StampImage},		// Scan  To  PC(SMB）&スタンプ合成方法(画像).
	{	4,		15,		85,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_PDFA1a},			// Scan  To  PC(SMB）&PDF/A-1a.
	{	4,		15,		86,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_PDFA1b},			// Scan  To  PC(SMB）&PDF/A-1b.
	{	4,		15,		87,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_PPTX},			// Scan  To  PC(SMB）&PPTX.
	{	4,		15,		88,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_PDFWeb},			// Scan  To  PC(SMB）&PDFのWeb表示最適化.
	{	4,		15,		89,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_DOCX},			// Scan  To  PC(SMB）&DOCX.
	{	4,		15,		90,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_XLSX},			// Scan  To  PC(SMB）&XLSX.
	{	4,		15,		91,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_OCR},			// Scan  To  PC(SMB）&OCR(文字認識).
	{	4,		15,		92,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_DOCXText},		// Scan  To  PC(SMB）&DOCX(文字優先).
	{	4,		15,		93,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_DOCXImage},		// Scan  To  PC(SMB）&DOCX(画像優先).
	{	4,		15,		94,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_DOCXImageText},	// Scan  To  PC(SMB）&DOCX(画像と文字).
	{	4,		15,		95,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_XLSXImageText},	// Scan  To  PC(SMB）&XLSX(画像と文字).
	{	4,		15,		96,		TYPE_FuncCombinationUseCntType_ScanToSMB,			TYPE_FuncCombinationUseCntFile_XLSXText},		// Scan  To  PC(SMB）&XLSX(文字のみ).
	{	4,		15,		97,		TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_JPEG},			// Scan  To  Box&JPEG.
	{	4,		15,		98,		TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_TIFF},			// Scan  To  Box&TIFF.
	{	4,		15,		99,		TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_PDF},			// Scan  To  Box&PDF.
	{	4,		15,		100,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_CPDF},			// Scan  To  Box&コンパクトPDF.
	{	4,		15,		101,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_XPS},			// Scan  To  Box&XPS.
	{	4,		15,		102,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_CXPS},			// Scan  To  Box&コンパクトXPS.
	{	4,		15,		103,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_SPDF},			// Scan  To  Box&サーチャブルPDF.
	{	4,		15,		104,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_OPDF},			// Scan  To  Box&アウトラインPDF.
	{	4,		15,		105,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_PDFPassword},	// Scan  To  Box&PDF暗号化(パスワード).
	{	4,		15,		106,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_PDFDigitalID},	// Scan  To  Box&PDF暗号化(デジタルID).
	{	4,		15,		107,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_StampText},		// Scan  To  Box&スタンプ合成方法(文字).
	{	4,		15,		108,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_StampImage},		// Scan  To  Box&スタンプ合成方法(画像).
	{	4,		15,		109,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_PDFA1a},			// Scan  To  Box&PDF/A-1a.
	{	4,		15,		110,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_PDFA1b},			// Scan  To  Box&PDF/A-1b.
	{	4,		15,		111,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_PPTX},			// Scan  To  Box&PPTX.
	{	4,		15,		112,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_PDFWeb},			// Scan  To  Box&PDFのWeb表示最適化.
	{	4,		15,		113,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_DOCX},			// Scan  To  Box&DOCX.
	{	4,		15,		114,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_XLSX},			// Scan  To  Box&XLSX.
	{	4,		15,		115,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_OCR},			// Scan  To  Box&OCR(文字認識).
	{	4,		15,		116,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_DOCXText},		// Scan  To  Box&DOCX(文字優先).
	{	4,		15,		117,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_DOCXImage},		// Scan  To  Box&DOCX(画像優先).
	{	4,		15,		118,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_DOCXImageText},	// Scan  To  Box&DOCX(画像と文字).
	{	4,		15,		119,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_XLSXImageText},	// Scan  To  Box&XLSX(画像と文字).
	{	4,		15,		120,	TYPE_FuncCombinationUseCntType_ScanToBox,			TYPE_FuncCombinationUseCntFile_XLSXText},		// Scan  To  Box&XLSX(文字のみ).
	{	4,		15,		121,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_JPEG},			// Scan  To  USB&JPEG.
	{	4,		15,		122,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_TIFF},			// Scan  To  USB&TIFF.
	{	4,		15,		123,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_PDF},			// Scan  To  USB&PDF.
	{	4,		15,		124,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_CPDF},			// Scan  To  USB&コンパクトPDF.
	{	4,		15,		125,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_XPS},			// Scan  To  USB&XPS.
	{	4,		15,		126,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_CXPS},			// Scan  To  USB&コンパクトXPS.
	{	4,		15,		127,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_SPDF},			// Scan  To  USB&サーチャブルPDF.
	{	4,		15,		128,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_OPDF},			// Scan  To  USB&アウトラインPDF.
	{	4,		15,		129,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_PDFPassword},	// Scan  To  USB&PDF暗号化(パスワード).
	{	4,		15,		130,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_PDFDigitalID},	// Scan  To  USB&PDF暗号化(デジタルID).
	{	4,		15,		131,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_StampText},		// Scan  To  USB&スタンプ合成方法(文字).
	{	4,		15,		132,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_StampImage},		// Scan  To  USB&スタンプ合成方法(画像).
	{	4,		15,		133,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_PDFA1a},			// Scan  To  USB&PDF/A-1a.
	{	4,		15,		134,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_PDFA1b},			// Scan  To  USB&PDF/A-1b.
	{	4,		15,		135,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_PPTX},			// Scan  To  USB&PPTX.
	{	4,		15,		136,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_PDFWeb},			// Scan  To  USB&PDFのWeb表示最適化.
	{	4,		15,		137,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_DOCX},			// Scan  To  USB&DOCX.
	{	4,		15,		138,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_XLSX},			// Scan  To  USB&XLSX.
	{	4,		15,		139,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_OCR},			// Scan  To  USB&OCR(文字認識).
	{	4,		15,		140,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_DOCXText},		// Scan  To  USB&DOCX(文字優先).
	{	4,		15,		141,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_DOCXImage},		// Scan  To  USB&DOCX(画像優先).
	{	4,		15,		142,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_DOCXImageText},	// Scan  To  USB&DOCX(画像と文字).
	{	4,		15,		143,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_XLSXImageText},	// Scan  To  USB&XLSX(画像と文字).
	{	4,		15,		144,	TYPE_FuncCombinationUseCntType_ScanToUSB,			TYPE_FuncCombinationUseCntFile_XLSXText},		// Scan  To  USB&XLSX(文字のみ).
	{	4,		15,		145,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_JPEG},			// Scan  To  ScanServer&JPEG.
	{	4,		15,		146,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_TIFF},			// Scan  To  ScanServer&TIFF.
	{	4,		15,		147,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_PDF},			// Scan  To  ScanServer&PDF.
	{	4,		15,		148,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_CPDF},			// Scan  To  ScanServer&コンパクトPDF.
	{	4,		15,		149,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_XPS},			// Scan  To  ScanServer&XPS.
	{	4,		15,		150,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_CXPS},			// Scan  To  ScanServer&コンパクトXPS.
	{	4,		15,		151,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_SPDF},			// Scan  To  ScanServer&サーチャブルPDF.
	{	4,		15,		152,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_OPDF},			// Scan  To  ScanServer&アウトラインPDF.
	{	4,		15,		153,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_PDFPassword},	// Scan  To  ScanServer&PDF暗号化(パスワード).
	{	4,		15,		154,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_PDFDigitalID},	// Scan  To  ScanServer&PDF暗号化(デジタルID).
	{	4,		15,		155,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_StampText},		// Scan  To  ScanServer&スタンプ合成方法(文字).
	{	4,		15,		156,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_StampImage},		// Scan  To  ScanServer&スタンプ合成方法(画像).
	{	4,		15,		157,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_PDFA1a},			// Scan  To  ScanServer&PDF/A-1a.
	{	4,		15,		158,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_PDFA1b},			// Scan  To  ScanServer&PDF/A-1b.
	{	4,		15,		159,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_PPTX},			// Scan  To  ScanServer&PPTX.
	{	4,		15,		160,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_PDFWeb},			// Scan  To  ScanServer&PDFのWeb表示最適化.
	{	4,		15,		161,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_DOCX},			// Scan  To  ScanServer&DOCX.
	{	4,		15,		162,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_XLSX},			// Scan  To  ScanServer&XLSX.
	{	4,		15,		163,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_OCR},			// Scan  To  ScanServer&OCR(文字認識).
	{	4,		15,		164,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_DOCXText},		// Scan  To  ScanServer&DOCX(文字優先).
	{	4,		15,		165,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_DOCXImage},		// Scan  To  ScanServer&DOCX(画像優先).
	{	4,		15,		166,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_DOCXImageText},	// Scan  To  ScanServer&DOCX(画像と文字).
	{	4,		15,		167,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_XLSXImageText},	// Scan  To  ScanServer&XLSX(画像と文字).
	{	4,		15,		168,	TYPE_FuncCombinationUseCntType_ScanToScanServer,	TYPE_FuncCombinationUseCntFile_XLSXText},		// Scan  To  ScanServer&XLSX(文字のみ).
};
// 2014/11/13 H.Ookuma RQ-250対応【機能利用頻度の収集機能（組合せカウンタ）】 end.

int32_t getFuncCount(char *p_str, TYPE_FuncUseCnt e_No )
{
	char temp[SIZE_TAG] = {""};

	APIC_FuncUseCnt *pc_FuncUseCnt = APIC_FuncUseCnt::newInstance();
	uint32_t ul_count = pc_FuncUseCnt->get(e_No);
	sprintf(temp, "%lu", ul_count );	// 2015/10/19 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加) "%u"→"%lu"に修正.

	memcpy(p_str, temp, SIZE_TAG);
	return strlen(temp);
}

// 2014/11/13 H.Ookuma RQ-250対応【機能利用頻度の収集機能（組合せカウンタ）】 start.
int32_t getFuncCountCombination(char *p_str, TYPE_FuncCombinationUseCntType e_scanType, TYPE_FuncCombinationUseCntFile e_fileType )	
{	
	char temp[SIZE_TAG] = {""};
	
	// CommonAPI使用.
	APIC_FuncCombinationUseCnt_Scan *pc_FuncCombinationUseCnt_Scan = APIC_FuncCombinationUseCnt_Scan::newInstance();
	uint32_t ul_count = pc_FuncCombinationUseCnt_Scan->get(e_scanType, e_fileType);
	sprintf(temp, "%lu", ul_count );	// 2015/10/19 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加) "%u"→"%lu"に修正.
	
	memcpy(p_str, temp, SIZE_TAG);
	return strlen(temp);
}	
// 2014/11/13 H.Ookuma RQ-250対応【機能利用頻度の収集機能（組合せカウンタ）】 end.
// 2015/10/19 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加) START.
int32_t getFuncCountUsr(char *p_str, TYPE_OperateCnt e_No )
{
	char temp[SIZE_TAG] = {""};

	APIC_OperateCnt *pc_OperateCnt = APIC_OperateCnt::newInstance();
	uint32_t ul_count = pc_OperateCnt->get(e_No);
	sprintf(temp, "%lu", ul_count );	// 2015/10/19 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加) "%u"→"%lu"に修正.

	memcpy(p_str, temp, SIZE_TAG);
	return strlen(temp);
}
// 2015/10/19 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加) END.
// 2016/7/13 ktakahashi IT5_3.3 制御構成変更_No.17 Start.
// (10)Emperor (1)言語別 (1)PS～(12)PCL5.
int32_t getFuncCounter_10_01_001(char *p_str, const RDSS_XmlMatrixId * p_xid)
{
	char temp[SIZE_TAG] = {""};
	TYPE_EmperorUseCntPdl e_data = TYPE_EmperorUseCntPdl_PS;

	switch( p_xid->c_Id )
	{
		case 1:		// PS.
			e_data = TYPE_EmperorUseCntPdl_PS;
			break;
		case 2:		// PCLXL.
			e_data = TYPE_EmperorUseCntPdl_PCLXL;
			break;
		case 3:		// XPS.
			e_data = TYPE_EmperorUseCntPdl_XPS;
			break;
		case 4:		// PDF.
			e_data = TYPE_EmperorUseCntPdl_PDF;
			break;
		case 5:		// OOXML.
			e_data = TYPE_EmperorUseCntPdl_OOXML;
			break;
		case 6:		// PPML.
			e_data = TYPE_EmperorUseCntPdl_PPML;
			break;
		case 7:		// XHTML.
			e_data = TYPE_EmperorUseCntPdl_XHTML;
			break;
		case 8:		// TIFF.
			e_data = TYPE_EmperorUseCntPdl_TIFF;
			break;
		case 9:		// JPEG.
			e_data = TYPE_EmperorUseCntPdl_JPEG;
			break;
		case 10:	// PWG.
			e_data = TYPE_EmperorUseCntPdl_PWG;
			break;
		case 11:	// URF.
			e_data = TYPE_EmperorUseCntPdl_URF;
			break;
		case 12:	// PCL5.
			e_data = TYPE_EmperorUseCntPdl_PCL5;
			break;
		default:
			break;
	}

	// カウンタ取得.
	APIC_EmperorUseCntPdl* pEmperorUseCntPdl = APIC_EmperorUseCntPdl::newInstance();
	uint32_t ul_count = pEmperorUseCntPdl->get( e_data );

	snprintf( temp, sizeof( temp ), "%d", ul_count );
	memcpy( p_str, temp, SIZE_TAG );
	return strlen( temp );
}

// (10)Emperor (2)OOXMLの生成アプリ別 (1)Word～(3)PowerPoint.
int32_t getFuncCounter_10_02_001(char *p_str, const RDSS_XmlMatrixId * p_xid)
{
	char temp[SIZE_TAG] = {""};
	TYPE_EmperorUseCntOoxmlApl e_data = TYPE_EmperorUseCntOoxmlApl_Word;

	switch( p_xid->c_Id )
	{
		case 1:		// Word.
			e_data = TYPE_EmperorUseCntOoxmlApl_Word;
			break;
		case 2:		// Excel.
			e_data = TYPE_EmperorUseCntOoxmlApl_Excel;
			break;
		case 3:		// PowerPoint.
			e_data = TYPE_EmperorUseCntOoxmlApl_PowerPoint;
			break;
		default:
			break;
	}

	// カウンタ取得.
	APIC_EmperorUseCntOoxmlApl* pEmperorUseCntOoxmlApl = APIC_EmperorUseCntOoxmlApl::newInstance();
	uint32_t ul_count = pEmperorUseCntOoxmlApl->get( e_data );

	snprintf( temp, sizeof( temp ), "%d", ul_count );
	memcpy( p_str, temp, SIZE_TAG );
	return strlen( temp );
}

// (10)Emperor (3)PDFバージョン別 (1)Ver1.0～(127)他.
int32_t getFuncCounter_10_03_001(char *p_str, const RDSS_XmlMatrixId * p_xid)
{
	char temp[SIZE_TAG] = {""};
	TYPE_EmperorUseCntPdfVersion e_data = TYPE_EmperorUseCntPdfVersion_1_0;

	switch( p_xid->c_Id )
	{
		case 1:		// Ver1.0.
			e_data = TYPE_EmperorUseCntPdfVersion_1_0;
			break;
		case 2:		// Ver1.1.
			e_data = TYPE_EmperorUseCntPdfVersion_1_1;
			break;
		case 3:		// Ver1.2.
			e_data = TYPE_EmperorUseCntPdfVersion_1_2;
			break;
		case 4:		// Ver1.3.
			e_data = TYPE_EmperorUseCntPdfVersion_1_3;
			break;
		case 5:		// Ver1.4.
			e_data = TYPE_EmperorUseCntPdfVersion_1_4;
			break;
		case 6:		// Ver1.5.
			e_data = TYPE_EmperorUseCntPdfVersion_1_5;
			break;
		case 7:		// Ver1.6.
			e_data = TYPE_EmperorUseCntPdfVersion_1_6;
			break;
		case 8:		// Ver1.7.
			e_data = TYPE_EmperorUseCntPdfVersion_1_7;
			break;
		case 9:		// Ver1.7-Level3.
			e_data = TYPE_EmperorUseCntPdfVersion_Ver1_7_Lev3;
			break;
		case 10:	// Ver1.7-Level8.
			e_data = TYPE_EmperorUseCntPdfVersion_Ver1_7_Lev8;
			break;
		case 11:	// Ver2.0.
			e_data = TYPE_EmperorUseCntPdfVersion_Ver2_0;
			break;
		case 127:	// 他.
			e_data = TYPE_EmperorUseCntPdfVersion_Other;
			break;
		default:
			break;
	}

	// カウンタ取得.
	APIC_EmperorUseCntPdfVersion* pEmperorUseCntPdfVersion = APIC_EmperorUseCntPdfVersion::newInstance();
	uint32_t ul_count = pEmperorUseCntPdfVersion->get( e_data );

	snprintf( temp, sizeof( temp ), "%d", ul_count );
	memcpy( p_str, temp, SIZE_TAG );
	return strlen( temp );
}

// (10)Emperor (4)Ooxmlアプリバージョン別 (1)Office2007～(127)他.
int32_t getFuncCounter_10_04_001(char *p_str, const RDSS_XmlMatrixId * p_xid)
{
	char temp[SIZE_TAG] = {""};
	TYPE_EmperorUseCntOoxmlVersion e_data = TYPE_EmperorUseCntOoxmlVersion_Office2007;

	switch( p_xid->c_Id )
	{
		case 1:		// Office2007.
			e_data = TYPE_EmperorUseCntOoxmlVersion_Office2007;
			break;
		case 2:		// Office2010.
			e_data = TYPE_EmperorUseCntOoxmlVersion_Office2010;
			break;
		case 3:		// Office2013.
			e_data = TYPE_EmperorUseCntOoxmlVersion_Office2013;
			break;
		case 4:		// Office2016.
			e_data = TYPE_EmperorUseCntOoxmlVersion_Office2016;
			break;
		case 5:		// Office2019.
			e_data = TYPE_EmperorUseCntOoxmlVersion_Office2019;
			break;
		case 127:	// 他.
			e_data = TYPE_EmperorUseCntOoxmlVersion_Other;
			break;
		default:
			break;
	}

	// カウンタ取得.
	APIC_EmperorUseCntOoxmlVersion* pEmperorUseCntOoxmlVersion = APIC_EmperorUseCntOoxmlVersion::newInstance();
	uint32_t ul_count = pEmperorUseCntOoxmlVersion->get( e_data );

	snprintf( temp, sizeof( temp ), "%d", ul_count );
	memcpy( p_str, temp, SIZE_TAG );
	return strlen( temp );
}

// Emperor機能カウンタ用テーブル.
static _IDtoFuncTable FuncTable_FuncCount_2[] =
{
	//	major	minor	s		func				.
	{	10,		1,		1,		getFuncCounter_10_01_001 },
	{	10,		1,		2,		getFuncCounter_10_01_001 },
	{	10,		1,		3,		getFuncCounter_10_01_001 },
	{	10,		1,		4,		getFuncCounter_10_01_001 },
	{	10,		1,		5,		getFuncCounter_10_01_001 },
	{	10,		1,		6,		getFuncCounter_10_01_001 },
	{	10,		1,		7,		getFuncCounter_10_01_001 },
	{	10,		1,		8,		getFuncCounter_10_01_001 },
	{	10,		1,		9,		getFuncCounter_10_01_001 },
	{	10,		1,		10,		getFuncCounter_10_01_001 },
	{	10,		1,		11,		getFuncCounter_10_01_001 },
	{	10,		1,		12,		getFuncCounter_10_01_001 },
	{	10,		2,		1,		getFuncCounter_10_02_001 },
	{	10,		2,		2,		getFuncCounter_10_02_001 },
	{	10,		2,		3,		getFuncCounter_10_02_001 },
	{	10,		3,		1,		getFuncCounter_10_03_001 },
	{	10,		3,		2,		getFuncCounter_10_03_001 },
	{	10,		3,		3,		getFuncCounter_10_03_001 },
	{	10,		3,		4,		getFuncCounter_10_03_001 },
	{	10,		3,		5,		getFuncCounter_10_03_001 },
	{	10,		3,		6,		getFuncCounter_10_03_001 },
	{	10,		3,		7,		getFuncCounter_10_03_001 },
	{	10,		3,		8,		getFuncCounter_10_03_001 },
	{	10,		3,		9,		getFuncCounter_10_03_001 },
	{	10,		3,		10,		getFuncCounter_10_03_001 },
	{	10,		3,		11,		getFuncCounter_10_03_001 },
	{	10,		3,		127,	getFuncCounter_10_03_001 },
	{	10,		4,		1,		getFuncCounter_10_04_001 },
	{	10,		4,		2,		getFuncCounter_10_04_001 },
	{	10,		4,		3,		getFuncCounter_10_04_001 },
	{	10,		4,		4,		getFuncCounter_10_04_001 },
	{	10,		4,		5,		getFuncCounter_10_04_001 },
	{	10,		4,		127,	getFuncCounter_10_04_001 },
};
// 2016/7/13 ktakahashi IT5_3.3 制御構成変更_No.17 End.

//2019/05/20 【EagleBK】【Control No.30】 HIENG Start.
//=================================================================================
//	関数	：isDisabledEagleBK_FuncCount.
//	機能	：check if the Function Counter command is disabled for EagleBK.
//	概要	：.
//	引数	：.
//			tempId : struct RDSS_XmlMatrixId.
//			e_Type : machine type.
//	戻り値	：.
//			True : the command is disabled.
//			False: the command is enabled.
//=================================================================================
Bool isDisabledEagleBK_FuncCount(RDSS_XmlMatrixId tempId, TYPE_MachineType e_Type)
{
	Bool isDisabled = False;

	if( (TYPE_MachineTypeEagleBK1 == e_Type) || (TYPE_MachineTypeEagleBK2 == e_Type) ){
		if( (1 == tempId.majorId && 6 == tempId.minorId && 8 == tempId.c_Id) ||
			(1 == tempId.majorId && 6 == tempId.minorId && 9 == tempId.c_Id) ||
			(2 == tempId.majorId && 3 == tempId.minorId && 4 == tempId.c_Id) ||
			(2 == tempId.majorId && 3 == tempId.minorId && 5 == tempId.c_Id) ||
			(3 == tempId.majorId && 3 == tempId.minorId && 4 == tempId.c_Id) ||
			(3 == tempId.majorId && 3 == tempId.minorId && 5 == tempId.c_Id) ){
			isDisabled = True;
		}
	}
// 2019/06/10 【EagleZplus】【EagleBKZplus】【Control No.1_4_6】 HIENG Start.
	else if(TYPE_MachineTypeEagleBKZplus == e_Type){
		if(15 == tempId.majorId && 2 == tempId.minorId && 107 == tempId.c_Id){
			isDisabled = True;
		}
	}
// 2019/06/10 【EagleZplus】【EagleBKZplus】【Control No.1_4_6】 HIENG End.
	return isDisabled;
}
//2019/05/20 【EagleBK】【Control No.30】 HIENG End.
//2021/03/09 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 制御構成変更_No.4 Start.
//=================================================================================
//	関数	：isDisabledMinervaSSBK_FuncCount.
//	機能	：check if the Function Counter command is disabled for MinervaSSBKX.
//	概要	：.
//	引数	：.
//			tempId : struct RDSS_XmlMatrixId.
//			e_Type : machine type.
//	戻り値	：.
//			True : the command is disabled.
//			False: the command is enabled.
//=================================================================================
Bool isDisabledMinervaSSBK_FuncCount(RDSS_XmlMatrixId tempId, TYPE_MachineType e_Type)
{
	Bool isDisabled = False;

		if( (TYPE_MachineTypeMinervaSSBKX == e_Type) || (TYPE_MachineTypeMinervaSSBK0_US == e_Type) || (TYPE_MachineTypeMinervaSSBK0_EU == e_Type) ){
			if(	15 == tempId.majorId && 2 == tempId.minorId && 107 == tempId.c_Id){
				isDisabled = True;
			}
		}
	return isDisabled;
}
//2021/03/09 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 制御構成変更_No.4 End.
// 2019/09/06 【HeliosMLK】【Control No.1】【HELIOSMLK-72】 HIENG Start.
//=================================================================================
//	関数	：isDisabledHeliosMLK_FuncCount.
//	機能	：check if the Function Counter command is disabled for HeliosMLK.
//	概要	：.
//	引数	：.
//			tempId : struct RDSS_XmlMatrixId.
//			e_Type : machine type.
//	戻り値	：.
//			True : the command is disabled.
//			False: the command is enabled.
//=================================================================================
Bool isDisabledHeliosMLK_FuncCount(RDSS_XmlMatrixId tempId, TYPE_MachineType e_Type)
{
	Bool isDisabled = False;

	if( (TYPE_MachineTypeHeliosMLK3_CN == e_Type) || (TYPE_MachineTypeHeliosMLK2_5_CN == e_Type) || (TYPE_MachineTypeHeliosMLK2_5_CNDF == e_Type)
			|| (TYPE_MachineTypeHeliosMLK2 == e_Type) || (TYPE_MachineTypeHeliosMLK2_5 == e_Type) || (TYPE_MachineTypeHeliosMLK3 == e_Type)
// 2019/12/04 【HeliosMLK】【Control No.52】【HELIOSMLK-1203】 HIENG Start.
			|| (TYPE_MachineTypeHeliosMLK2_CNDF == e_Type) ){
// 2019/12/04 【HeliosMLK】【Control No.52】【HELIOSMLK-1203】 HIENG End.
// 2019/10/04【 IT6_2.0 】【HeliosMLK】【機器構成変更_No.17】kurihara Start.
		if( (1 == tempId.majorId && 4 == tempId.minorId && 17 == tempId.c_Id) ||
			(1 == tempId.majorId && 4 == tempId.minorId && 18 == tempId.c_Id) ||
			(1 == tempId.majorId && 6 == tempId.minorId && 8 == tempId.c_Id) ||
			(1 == tempId.majorId && 6 == tempId.minorId && 9 == tempId.c_Id) ||
			(2 == tempId.majorId && 3 == tempId.minorId && 4 == tempId.c_Id) ||
			(2 == tempId.majorId && 3 == tempId.minorId && 5 == tempId.c_Id) ||
			(3 == tempId.majorId && 3 == tempId.minorId && 4 == tempId.c_Id) ||
			(3 == tempId.majorId && 3 == tempId.minorId && 5 == tempId.c_Id) ||
			(4 == tempId.majorId && 3 == tempId.minorId && 7 == tempId.c_Id) ||
			(4 == tempId.majorId && 3 == tempId.minorId && 15 == tempId.c_Id) ||
			(4 == tempId.majorId && 3 == tempId.minorId && 16 == tempId.c_Id) ||
			(5 == tempId.majorId && 3 == tempId.minorId && 7 == tempId.c_Id) ||
			(5 == tempId.majorId && 3 == tempId.minorId && 15 == tempId.c_Id) ||
			(5 == tempId.majorId && 3 == tempId.minorId && 16 == tempId.c_Id) ||
			(15 == tempId.majorId && 2 == tempId.minorId && 112 == tempId.c_Id) ){
			isDisabled = True;
		}
// 2019/10/04【 IT6_2.0 】【HeliosMLK】【機器構成変更_No.17】kurihara End.
		if ( (TYPE_MachineTypeHeliosMLK3_CN == e_Type) || (TYPE_MachineTypeHeliosMLK2_5_CN == e_Type) || (TYPE_MachineTypeHeliosMLK2_5_CNDF == e_Type)
// 2019/12/04 【HeliosMLK】【Control No.52】【HELIOSMLK-1203】 HIENG Start.
			|| (TYPE_MachineTypeHeliosMLK2_CNDF == e_Type) ){
// 2019/12/04 【HeliosMLK】【Control No.52】【HELIOSMLK-1203】 HIENG End.
			if( (15 == tempId.majorId && 2 == tempId.minorId && 103 == tempId.c_Id) ){
				isDisabled = True;
			}
		}
	}
	return isDisabled;
}
// 2019/09/06 【HeliosMLK】【Control No.1】【HELIOSMLK-72】 HIENG End.

//=================================================================================
//	関数	：getOneData_FuncCount.
//	機能	：機能カウンタの数値を文字列に変換して取得する.
//	引数	：.
//			p_str : 文字列を格納する領域(1024バイトの領域を期待).
//			xid   : テーブルから検索する分類ID用構造体のアドレス.
//	戻り値	：.
//			0以上 :正常値.
//			-1    :異常値.
//=================================================================================
int32_t getOneData_FuncCount(char *p_str, const RDSS_XmlMatrixId *xid)
{
	if( NULL == p_str || NULL == xid)
	{
		// ありえないがバカよけ.
		return -1;
	}
//2018/01/24 【 IT6_1.0 】【Sparrow】 【制御構成変更_No.206-1】 HCLT-Jahnavi Start.
	APIC_SystemInfo* pc_SystemInfo	= APIC_SystemInfo::newInstance();
	pc_SystemInfo->lock();
	TYPE_MachineType e_Type = pc_SystemInfo->getMachineType();
	pc_SystemInfo->unlock();
//2018/01/24 【 IT6_1.0 】【Sparrow】 【制御構成変更_No.206-1】 HCLT-Jahnavi End.		
	// テーブル比較用.
	int max = sizeof(FuncTable_FuncCount) / sizeof(FuncTable_FuncCount[0]);
	RDSS_XmlMatrixId tempId = *xid;
	_IDtoFuncCountTable matchTable;
	_IDtoFuncCountCombinationTable c_matchTable; // Scan組合せテーブル RQ-250対応【機能利用頻度の収集機能（組合せカウンタ）】.

	memset(&matchTable, 0, sizeof(matchTable) );
	memset(&c_matchTable, 0, sizeof(c_matchTable) ); // Scan組合せテーブル初期化 RQ-250対応【機能利用頻度の収集機能（組合せカウンタ）】.

//	2014.12.01 H.Ookuma RQ-250対応【機能利用頻度の収集機能（組合せカウンタ）】 start.
	if( 4 == tempId.majorId && 15 == tempId.minorId ) {
		c_matchTable = FuncTable_FuncCountCombination[tempId.c_Id-1]; // CQ.509684 スキャン組合せ時カウンタが正しく取得できない.
		// 関数コール.

// 2019/07/23 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 MOD Start.
//2018/01/24 【 IT6_1.0 】【Sparrow】 【制御構成変更_No.206-1】 HCLT-Jahnavi Start.
		if( ( (TYPE_MachineTypeSparrow1SFP == e_Type) ||
			  (TYPE_MachineTypeSparrow2SFP == e_Type) || 
			  (TYPE_MachineTypeSparrowBK0SFP == e_Type )
			) && (168 >= tempId.c_Id)
		){
			return -1;
		}
		else
		{
		return getFuncCountCombination(p_str, c_matchTable.e_scanType, c_matchTable.e_fileType);
		}
//2018/01/24 【 IT6_1.0 】【Sparrow】 【制御構成変更_No.206-1】 HCLT-Jahnavi End.
	}
// 2019/07/23 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 MOD End.
//	2014.12.01 H.Ookuma RQ-250対応【機能利用頻度の収集機能（組合せカウンタ）】 end.

// 2016/7/13 ktakahashi IT5_3.3 制御構成変更_No.17 Start.
	// Emperor利用頻度カウンター（言語別）取得.
	if( 10 == tempId.majorId ) {

		// テーブル比較用.
		short ss_tableSize = sizeof( FuncTable_FuncCount_2 ) / sizeof( FuncTable_FuncCount_2[0] );
		_IDtoFuncTable matchTable_2 = { 0 };

		for( short ss_i = 0; ss_i < ss_tableSize; ss_i++ )
		{
			if(  ( FuncTable_FuncCount_2[ss_i].majorId == tempId.majorId )
			  && ( FuncTable_FuncCount_2[ss_i].minorId == tempId.minorId )
			  && ( FuncTable_FuncCount_2[ss_i].c_Id == tempId.c_Id ) )
			{
				matchTable_2 = FuncTable_FuncCount_2[ss_i];
			}
		}
	
		if( NULL == matchTable_2.p_func )
		{
			// テーブルに対象のIDが無いとき.
			return -1;
		}
		
		// 関数コール.
		return ( *( matchTable_2.p_func ) )( p_str, xid );
	}
// 2016/7/13 ktakahashi IT5_3.3 制御構成変更_No.17 End.
// 2017/11/10 HCLT-Vinay IT6_1.0 制御構成変更_No.6 Start.
//2018/01/24 【 IT6_1.0 】【Sparrow】 【制御構成変更_No.206-1】 HCLT-Jahnavi Start.
//	APIC_SystemInfo* pc_SystemInfo	= APIC_SystemInfo::newInstance();
//	pc_SystemInfo->lock();
//	TYPE_MachineType e_Type = pc_SystemInfo->getMachineType();
//	pc_SystemInfo->unlock();
//2018/01/24 【 IT6_1.0 】【Sparrow】 【制御構成変更_No.206-1】 HCLT-Jahnavi END.
	// ★Todo:高速化できない？.
	for(int i = 0; i < max; i++)
	{
		if(FuncTable_FuncCount[i].majorId == tempId.majorId
		  && FuncTable_FuncCount[i].minorId == tempId.minorId
		  && FuncTable_FuncCount[i].c_Id == tempId.c_Id)
		{

//2019/05/20 【EagleBK】【Control No.30】 HIENG Start.
			if( True == isDisabledEagleBK_FuncCount(tempId, e_Type) ) {
				continue;
			}
//2019/05/20 【EagleBK】【Control No.30】 HIENG End.
//2021/03/09 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 制御構成変更_No.4 Start.
			if( True == isDisabledMinervaSSBK_FuncCount(tempId, e_Type) ) {
				continue;
			}
//2021/03/09 FPT-TrungHV7 【IT6_2.2】【MinervaSSBK】 制御構成変更_No.4 End.
// 2019/09/06 【HeliosMLK】【Control No.1】【HELIOSMLK-72】 HIENG Start.
			if( True == isDisabledHeliosMLK_FuncCount(tempId, e_Type) ){
				continue;
			}
// 2019/09/06 【HeliosMLK】【Control No.1】【HELIOSMLK-72】 HIENG End.
			if( (TYPE_MachineTypeEagle1 == e_Type) || (TYPE_MachineTypeEagle2 == e_Type) || (TYPE_MachineTypeEagle2_5 == e_Type)  ){
                if( (1 == tempId.majorId && 6 == tempId.minorId && 8 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 9 == tempId.c_Id) ||
					(2 == tempId.majorId && 3 == tempId.minorId && 4 == tempId.c_Id) ||
					(2 == tempId.majorId && 3 == tempId.minorId && 5 == tempId.c_Id) ||
					(3 == tempId.majorId && 3 == tempId.minorId && 4 == tempId.c_Id) ||
					(3 == tempId.majorId && 3 == tempId.minorId && 5 == tempId.c_Id) ){
                    continue;
				}
			}

			// 2017/11/10 HCLT-Vinay IT6_1.0 制御構成変更_No.6 End.
//2018/01/24 【 IT6_1.0 】【Sparrow】 【制御構成変更_No.206-1】 HCLT-Jahnavi Start.
//20180418【IT6_1.0】【Eagle】【制御構成変更_No.114】 HCLT-Rajeshwari Added Start.
			if( (7 == tempId.majorId && 12 == tempId.minorId && 1 == tempId.c_Id) && (TYPE_AirPrintLicence_Ver1_7 > pc_SystemInfo->getAirPrintLicenceStatus()) ){
                    continue;
                }
//20180418【IT6_1.0】【Eagle】【制御構成変更_No.114】 HCLT-Rajeshwari Added End.

// 2019/07/23 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 MOD Start.
			if( (TYPE_MachineTypeSparrow1SFP == e_Type)   ||
				(TYPE_MachineTypeSparrow2SFP == e_Type)   ||
				(TYPE_MachineTypeSparrowBK0SFP == e_Type)
			){
                if( (1 == tempId.majorId && 1 == tempId.minorId && 1 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 2 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 4 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 5 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 6 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 7 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 8 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 9 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 10 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 11 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 12 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 13 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 14 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 19 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 20 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 21 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 22 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 23 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 24 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 25 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 26 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 27 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 28 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 35 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 36 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 37 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 38 == tempId.c_Id) ||
					(1 == tempId.majorId && 1 == tempId.minorId && 39 == tempId.c_Id) ||
					(1 == tempId.majorId && 2 == tempId.minorId && 1 == tempId.c_Id) ||
					(1 == tempId.majorId && 2 == tempId.minorId && 2 == tempId.c_Id) ||
					(1 == tempId.majorId && 2 == tempId.minorId && 3 == tempId.c_Id) ||
					(1 == tempId.majorId && 2 == tempId.minorId && 4 == tempId.c_Id) ||
					(1 == tempId.majorId && 2 == tempId.minorId && 5 == tempId.c_Id) ||
					(1 == tempId.majorId && 2 == tempId.minorId && 6 == tempId.c_Id) ||
					(1 == tempId.majorId && 3 == tempId.minorId && 1 == tempId.c_Id) ||
					(1 == tempId.majorId && 3 == tempId.minorId && 2 == tempId.c_Id) ||
					(1 == tempId.majorId && 3 == tempId.minorId && 3 == tempId.c_Id) ||
					(1 == tempId.majorId && 3 == tempId.minorId && 4 == tempId.c_Id) ||
					(1 == tempId.majorId && 3 == tempId.minorId && 5 == tempId.c_Id) ||
					(1 == tempId.majorId && 3 == tempId.minorId && 6 == tempId.c_Id) ||
					(1 == tempId.majorId && 4 == tempId.minorId && 1 == tempId.c_Id) ||
					(1 == tempId.majorId && 4 == tempId.minorId && 2 == tempId.c_Id) ||
					(1 == tempId.majorId && 4 == tempId.minorId && 3 == tempId.c_Id) ||
					(1 == tempId.majorId && 4 == tempId.minorId && 4 == tempId.c_Id) ||
					(1 == tempId.majorId && 4 == tempId.minorId && 7 == tempId.c_Id) ||
					(1 == tempId.majorId && 4 == tempId.minorId && 8 == tempId.c_Id) ||
					(1 == tempId.majorId && 4 == tempId.minorId && 9 == tempId.c_Id) ||
					(1 == tempId.majorId && 4 == tempId.minorId && 10 == tempId.c_Id) ||
					(1 == tempId.majorId && 4 == tempId.minorId && 12 == tempId.c_Id) ||
					(1 == tempId.majorId && 4 == tempId.minorId && 13 == tempId.c_Id) ||
					(1 == tempId.majorId && 4 == tempId.minorId && 16 == tempId.c_Id) ||
					(1 == tempId.majorId && 5 == tempId.minorId && 1 == tempId.c_Id) ||
					(1 == tempId.majorId && 5 == tempId.minorId && 2 == tempId.c_Id) ||
					(1 == tempId.majorId && 5 == tempId.minorId && 3 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 1 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 2 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 4 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 5 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 6 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 7 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 10 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 12 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 14 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 15 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 16 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 17 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 18 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 20 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 21 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 22 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 23 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 24 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 25 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 26 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 27 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 28 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 31 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 32 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 33 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 34 == tempId.c_Id) ||
					(1 == tempId.majorId && 6 == tempId.minorId && 35 == tempId.c_Id) ||
					(2 == tempId.majorId && 3 == tempId.minorId && 6 == tempId.c_Id) ||
					(3 == tempId.majorId && 1 == tempId.minorId && 2 == tempId.c_Id) ||
					(3 == tempId.majorId && 1 == tempId.minorId && 3 == tempId.c_Id) ||
					(3 == tempId.majorId && 1 == tempId.minorId && 4 == tempId.c_Id) ||
					(3 == tempId.majorId && 1 == tempId.minorId && 5 == tempId.c_Id) ||
					(3 == tempId.majorId && 2 == tempId.minorId && 1 == tempId.c_Id) ||
					(3 == tempId.majorId && 2 == tempId.minorId && 2 == tempId.c_Id) ||
					(3 == tempId.majorId && 2 == tempId.minorId && 3 == tempId.c_Id) ||
					(3 == tempId.majorId && 3 == tempId.minorId && 1 == tempId.c_Id) ||
					(3 == tempId.majorId && 3 == tempId.minorId && 2 == tempId.c_Id) ||
					(3 == tempId.majorId && 3 == tempId.minorId && 3 == tempId.c_Id) ||
					(3 == tempId.majorId && 3 == tempId.minorId && 6 == tempId.c_Id) ||
					(3 == tempId.majorId && 3 == tempId.minorId && 7 == tempId.c_Id) ||
					(3 == tempId.majorId && 3 == tempId.minorId && 8 == tempId.c_Id) ||
					(3 == tempId.majorId && 3 == tempId.minorId && 9 == tempId.c_Id) ||
					(3 == tempId.majorId && 3 == tempId.minorId && 10 == tempId.c_Id) ||
					(3 == tempId.majorId && 3 == tempId.minorId && 11 == tempId.c_Id) ||
					(3 == tempId.majorId && 3 == tempId.minorId && 14 == tempId.c_Id) ||
					(3 == tempId.majorId && 3 == tempId.minorId && 15 == tempId.c_Id) ||
					(4 == tempId.majorId && 1 == tempId.minorId && 1 == tempId.c_Id) ||
					(4 == tempId.majorId && 1 == tempId.minorId && 2 == tempId.c_Id) ||
					(4 == tempId.majorId && 1 == tempId.minorId && 3 == tempId.c_Id) ||
					(4 == tempId.majorId && 1 == tempId.minorId && 4 == tempId.c_Id) ||
					(4 == tempId.majorId && 1 == tempId.minorId && 5 == tempId.c_Id) ||
					(4 == tempId.majorId && 1 == tempId.minorId && 6 == tempId.c_Id) ||
					(4 == tempId.majorId && 1 == tempId.minorId && 7 == tempId.c_Id) ||
					(4 == tempId.majorId && 2 == tempId.minorId && 1 == tempId.c_Id) ||
					(4 == tempId.majorId && 2 == tempId.minorId && 2 == tempId.c_Id) ||
					(4 == tempId.majorId && 2 == tempId.minorId && 3 == tempId.c_Id) ||
					(4 == tempId.majorId && 2 == tempId.minorId && 4 == tempId.c_Id) ||
					(4 == tempId.majorId && 2 == tempId.minorId && 5 == tempId.c_Id) ||
					(4 == tempId.majorId && 3 == tempId.minorId && 1 == tempId.c_Id) ||
					(4 == tempId.majorId && 3 == tempId.minorId && 2 == tempId.c_Id) ||
					(4 == tempId.majorId && 3 == tempId.minorId && 3 == tempId.c_Id) ||
					(4 == tempId.majorId && 3 == tempId.minorId && 6 == tempId.c_Id) ||
					(4 == tempId.majorId && 3 == tempId.minorId && 12 == tempId.c_Id) ||
					(4 == tempId.majorId && 3 == tempId.minorId && 13 == tempId.c_Id) ||
					(4 == tempId.majorId && 3 == tempId.minorId && 14 == tempId.c_Id) ||
					(4 == tempId.majorId && 4 == tempId.minorId && 2 == tempId.c_Id) ||
					(4 == tempId.majorId && 4 == tempId.minorId && 3 == tempId.c_Id) ||
					(4 == tempId.majorId && 4 == tempId.minorId && 4 == tempId.c_Id) ||
					(4 == tempId.majorId && 4 == tempId.minorId && 5 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 1 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 2 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 3 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 4 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 5 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 6 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 7 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 8 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 9 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 10 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 11 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 12 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 13 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 14 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 15 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 16 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 17 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 18 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 19 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 20 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 21 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 22 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 23 == tempId.c_Id) ||
					(4 == tempId.majorId && 5 == tempId.minorId && 24 == tempId.c_Id) ||
					(4 == tempId.majorId && 6 == tempId.minorId && 1 == tempId.c_Id) ||
					(4 == tempId.majorId && 6 == tempId.minorId && 2 == tempId.c_Id) ||
					(4 == tempId.majorId && 6 == tempId.minorId && 3 == tempId.c_Id) ||
					(4 == tempId.majorId && 6 == tempId.minorId && 4 == tempId.c_Id) ||
					(4 == tempId.majorId && 7 == tempId.minorId && 1 == tempId.c_Id) ||
					(4 == tempId.majorId && 7 == tempId.minorId && 2 == tempId.c_Id) ||
					(4 == tempId.majorId && 7 == tempId.minorId && 3 == tempId.c_Id) ||
					(4 == tempId.majorId && 7 == tempId.minorId && 4 == tempId.c_Id) ||
					(4 == tempId.majorId && 7 == tempId.minorId && 5 == tempId.c_Id) ||
					(4 == tempId.majorId && 7 == tempId.minorId && 6 == tempId.c_Id) ||
					(4 == tempId.majorId && 7 == tempId.minorId && 7 == tempId.c_Id) ||
					(4 == tempId.majorId && 7 == tempId.minorId && 8 == tempId.c_Id) ||
					(4 == tempId.majorId && 15 == tempId.minorId && 168 >= tempId.c_Id) ||
				    (5 == tempId.majorId && 1 == tempId.minorId && 1 == tempId.c_Id) ||
					(5 == tempId.majorId && 1 == tempId.minorId && 2 == tempId.c_Id) ||
					(5 == tempId.majorId && 1 == tempId.minorId && 3 == tempId.c_Id) ||
					(5 == tempId.majorId && 1 == tempId.minorId && 5 == tempId.c_Id) ||
					(5 == tempId.majorId && 1 == tempId.minorId && 6 == tempId.c_Id) ||				// 2019/06/28 EagleZ+ 制御構成変更_No.36 MIF機対応 日立 江上 ADD.
					(5 == tempId.majorId && 2 == tempId.minorId && 1 == tempId.c_Id) ||
					(5 == tempId.majorId && 2 == tempId.minorId && 2 == tempId.c_Id) ||
					(5 == tempId.majorId && 2 == tempId.minorId && 3 == tempId.c_Id) ||
					(5 == tempId.majorId && 2 == tempId.minorId && 4 == tempId.c_Id) ||
					(5 == tempId.majorId && 2 == tempId.minorId && 5 == tempId.c_Id) ||
					(5 == tempId.majorId && 3 == tempId.minorId && 1 == tempId.c_Id) ||
					(5 == tempId.majorId && 3 == tempId.minorId && 2 == tempId.c_Id) ||
					(5 == tempId.majorId && 3 == tempId.minorId && 3 == tempId.c_Id) ||
					(5 == tempId.majorId && 3 == tempId.minorId && 6 == tempId.c_Id) ||
					(5 == tempId.majorId && 3 == tempId.minorId && 12 == tempId.c_Id) ||
					(5 == tempId.majorId && 3 == tempId.minorId && 13 == tempId.c_Id) ||
					(5 == tempId.majorId && 3 == tempId.minorId && 14 == tempId.c_Id) ||
					(5 == tempId.majorId && 4 == tempId.minorId && 1 == tempId.c_Id) ||
					(5 == tempId.majorId && 4 == tempId.minorId && 2 == tempId.c_Id) ||
					(5 == tempId.majorId && 4 == tempId.minorId && 4 == tempId.c_Id) ||
					(5 == tempId.majorId && 4 == tempId.minorId && 5 == tempId.c_Id)  ||
					(5 == tempId.majorId && 5 == tempId.minorId && 24 >= tempId.c_Id) ||
					(5 == tempId.majorId && 6 == tempId.minorId && 12 >= tempId.c_Id)  ||
					(5 == tempId.majorId && 7 == tempId.minorId && 8 >= tempId.c_Id ) ||
				    (7 == tempId.majorId && 12 == tempId.minorId && 1 == tempId.c_Id) ||
					(9 == tempId.majorId && 2 == tempId.minorId && 21 == tempId.c_Id) ||
					(9 == tempId.majorId && 3 == tempId.minorId && 21 == tempId.c_Id) ||
					(9 == tempId.majorId && 3 == tempId.minorId && 22 == tempId.c_Id) ||
					(9 == tempId.majorId && 3 == tempId.minorId && 41 == tempId.c_Id) ||
					(9 == tempId.majorId && 3 == tempId.minorId && 42 == tempId.c_Id) ||
					(9 == tempId.majorId && 3 == tempId.minorId && 43 == tempId.c_Id) ||
					(9 == tempId.majorId && 3 == tempId.minorId && 44 == tempId.c_Id) ||
					(9 == tempId.majorId && 3 == tempId.minorId && 45 == tempId.c_Id) ||
					(9 == tempId.majorId && 3 == tempId.minorId && 46 == tempId.c_Id) ||
					(9 == tempId.majorId && 4 == tempId.minorId && 21 == tempId.c_Id) ||
					(9 == tempId.majorId && 4 == tempId.minorId && 22 == tempId.c_Id) ||
					(9 == tempId.majorId && 4 == tempId.minorId && 41 == tempId.c_Id) ||
					(9 == tempId.majorId && 4 == tempId.minorId && 42 == tempId.c_Id) ||
					(9 == tempId.majorId && 4 == tempId.minorId && 43 == tempId.c_Id) ||
					(9 == tempId.majorId && 5 == tempId.minorId && 51 == tempId.c_Id) ||
					(9 == tempId.majorId && 5 == tempId.minorId && 71 == tempId.c_Id) ||
					(9 == tempId.majorId && 5 == tempId.minorId && 81 == tempId.c_Id) ||
					(9 == tempId.majorId && 6 == tempId.minorId && 51 == tempId.c_Id) ||
					(9 == tempId.majorId && 6 == tempId.minorId && 71 == tempId.c_Id) ||
					(9 == tempId.majorId && 6 == tempId.minorId && 81 == tempId.c_Id) ||
					(15 == tempId.majorId && 2 == tempId.minorId && 3 == tempId.c_Id) ||
					(15 == tempId.majorId && 2 == tempId.minorId && 7 == tempId.c_Id) ||
					(15 == tempId.majorId && 2 == tempId.minorId && 8 == tempId.c_Id) ||
					(15 == tempId.majorId && 2 == tempId.minorId && 11 == tempId.c_Id) ||
					(15 == tempId.majorId && 2 == tempId.minorId && 13 == tempId.c_Id) ||
					(15 == tempId.majorId && 2 == tempId.minorId && 18 == tempId.c_Id) ||
					(15 == tempId.majorId && 2 == tempId.minorId && 19 == tempId.c_Id) ||
					(15 == tempId.majorId && 2 == tempId.minorId && ((29 <= tempId.c_Id) && (52 >= tempId.c_Id )))||
					(15 == tempId.majorId && 6 == tempId.minorId && 6 >= tempId.c_Id) ||
					(15 == tempId.majorId && 7 == tempId.minorId && 6 >= tempId.c_Id) ||
					(15 == tempId.majorId && 8 == tempId.minorId && 5 >= tempId.c_Id) ||
					(15 == tempId.majorId && 9 == tempId.minorId && 5 >= tempId.c_Id) ){
					continue;
				}
			}
//2018/01/24 【 IT6_1.0 】【Sparrow】 【制御構成変更_No.206-1】 HCLT-Jahnavi End.
// 2019/07/23 SparrowBK 制御構成変更_No.6 マシンタイプ対応 日立 江上 MOD End.

// 2019/06/28 EagleZ+ 制御構成変更_No.36 MIF機対応 日立 江上 ADD Start.
			if( (TYPE_MachineTypeSparrow2AIO == e_Type) ||
				(TYPE_MachineTypeSparrow2SFP == e_Type) ){
				if(	(5 == tempId.majorId && 1 == tempId.minorId && 6 == tempId.c_Id)
 				){
					continue;
				}
			}
// 2019/06/28 EagleZ+ 制御構成変更_No.36 MIF機対応 日立 江上 ADD End.
//2023/03/11 FPT-VuongVD2 IT6_2.3 OP_BTS-42282 Start.
#ifdef DEF_EagleCus
			pc_SystemInfo->lock();
			TYPE_ModelType e_Model = pc_SystemInfo->getModelType();
			pc_SystemInfo->unlock();
			if(TYPE_ModelTypeIII != e_Model){
				if( (15 == tempId.majorId && 2 == tempId.minorId && 70 == tempId.c_Id) ||
					(15 == tempId.majorId && 2 == tempId.minorId && 78 == tempId.c_Id) ||
					(15 == tempId.majorId && 2 == tempId.minorId && 79 == tempId.c_Id) ){
						continue;
				}
			}
#else
			if( (15 == tempId.majorId && 2 == tempId.minorId && 70 == tempId.c_Id) ||
				(15 == tempId.majorId && 2 == tempId.minorId && 78 == tempId.c_Id) ||
				(15 == tempId.majorId && 2 == tempId.minorId && 79 == tempId.c_Id) ){
					continue;
			}
#endif
//2023/03/11 FPT-VuongVD2 IT6_2.3 OP_BTS-42282 End.
//2023/04/13 FPT-NhuongNV1 IT6_2.3 OP_BTS-42600 Start.
#if defined (DEF_A4_Cus) || defined (DEF_HeliosMLKCus) || defined(DEF_DenebMLKCus)
			if( (15 == tempId.majorId && 2 == tempId.minorId && 71 == tempId.c_Id) ||
				(15 == tempId.majorId && 2 == tempId.minorId && 72 == tempId.c_Id) ||
				(15 == tempId.majorId && 2 == tempId.minorId && 73 == tempId.c_Id) ||
				(15 == tempId.majorId && 2 == tempId.minorId && 74 == tempId.c_Id) ||
				(15 == tempId.majorId && 2 == tempId.minorId && 75 == tempId.c_Id) ||
				(15 == tempId.majorId && 2 == tempId.minorId && 76 == tempId.c_Id) ||
				(15 == tempId.majorId && 2 == tempId.minorId && 77 == tempId.c_Id) ){
					continue;
				}
#endif
//2023/04/13 FPT-NhuongNV1 IT6_2.3 OP_BTS-42600 End.
			matchTable = FuncTable_FuncCount[i];
		}
	}
	
// 2015/10/19 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加) START.
	if( RDS_TYPE_CNT_JOB == matchTable.cntType )
	{
		// 関数コール（Job）.
		return getFuncCount(p_str, matchTable.e_FuncNum);
	}
	else if( RDS_TYPE_CNT_USER == matchTable.cntType )
	{
		// 関数コール（User）.
		return getFuncCountUsr(p_str, matchTable.e_OperateNum);
	}
	else
	{
		// テーブルに対象のIDが無いとき.
		return -1;
	}
// 2015/10/19 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(機能カウンタ追加) END.
}
// 2015/10/28 M.Adachi [ZeusS_ZX0]制御構成変更_No.76_リサーチ強化(AQコマンド追加) START.
typedef struct
{
	type_id majorId;			// 分類1のID.
	type_id minorId;			// 分類2のID.
	type_id c_Id;				// 分類3のID.
	int32_t (*p_func)(char *, const RDSS_XmlMatrixId * );		// 関数ポインタ.
}_IDtoFuncTableAppUseFreqCount;
//=================================================================================
//	関数	：getOneData_AppUseFreqCount_IwsName.
//	機能	：IWSアプリ利用頻度カウンタのアプリ名＋識別情報を取得する.
//	引数	：.
//			p_str : 文字列を格納する領域(1024バイトの領域を期待).
//			xid   : テーブルから検索する分類ID用構造体のアドレス.
//	戻り値	：.
//			1以上 :正常値（文字列長）.
//			0     :アプリ登録なし
//=================================================================================
int32_t getOneData_AppUseFreqCount_IwsName(char *p_str, const RDSS_XmlMatrixId *xid )
{
	char temp[SIZE_TAG] = {""};
	RDSS_XmlMatrixId tmpxId = *xid;
	APIC_IWSAppInfo* pc_IWSAppInfo = APIC_IWSAppInfo::newInstance();
	APIC_IwsCnt* pc_IwsCnt = APIC_IwsCnt::newInstance();

	// IWSアプリ情報取得
	TYPS_IwsAppInfo tempAppInfo = { 0 };
	TYPS_IwsCntInfo tempCntInfo = { { 0 } };	// 2016/01/18 ktakahashi warning除去対応.

	pc_IWSAppInfo->lock();
	int ret = pc_IWSAppInfo->getApplication( (int)tmpxId.minorId , tempAppInfo );
	if( TYPD_IwsResult_OK == ret ){
		pc_IwsCnt->getApplicationInfo( tempAppInfo.sc_ID, &tempCntInfo );	// アプリ利用頻度カウンタ IWS識別情報取得.
	}else{
		// 情報が取れなければリターン.
		pc_IWSAppInfo->unlock();
		return 0;	// 情報が無くても構成情報は出力するので0を返す.
	}
	pc_IWSAppInfo->unlock();

	// IWS:アプリ名＋識別情報.
	// 2019/03/06 FPT-ChiNM [IT6_1.0MR2][1071376]【Sparrow MR】【課題1058518別件提起】【管理No.623】【制御構成変更_No.210】Modification Start.
	//// 2015/11/24 M.Adachi [ZeusS_ZX0]CQ519679_静的解析対応 START.
	//snprintf(temp, sizeof( temp ),	"%s%s%d"	, tempCntInfo.sc_AppID		// アプリケーションID.
	//											, tempCntInfo.sc_AppName	// アプリケーション名.
	//											, tempCntInfo.e_AppType );	// アプリケーション種別.
	//// 2015/11/24 M.Adachi [ZeusS_ZX0]CQ519679_静的解析対応 END.

	snprintf(temp, sizeof(temp), "%d_%s_%s", tempCntInfo.e_AppType		// アプリケーション種別.
											, tempCntInfo.sc_AppID		// アプリケーションID.
											, tempCntInfo.sc_AppName);	// アプリケーション名.
	// 2019/03/06 FPT-ChiNM [IT6_1.0MR2][1071376]【Sparrow MR】【課題1058518別件提起】【管理No.623】【制御構成変更_No.210】Modification End.

	memcpy(p_str, temp, SIZE_TAG);
	return strlen(temp);
}
//=================================================================================
//	関数	：getOneData_AppUseFreqCount_IwsFreq.
//	機能	：IWSアプリ利用頻度カウンタの利用頻度を取得する.
//	引数	：.
//			p_str : 文字列を格納する領域(1024バイトの領域を期待).
//			xid   : テーブルから検索する分類ID用構造体のアドレス.
//	戻り値	：.
//			1以上 :正常値（文字列長）.
//=================================================================================
int32_t getOneData_AppUseFreqCount_IwsFreq(char *p_str, const RDSS_XmlMatrixId *xid )
{
	char temp[SIZE_TAG] = {""};
	RDSS_XmlMatrixId tmpxId = *xid;
	APIC_IWSAppInfo* pc_IWSAppInfo = APIC_IWSAppInfo::newInstance();
	APIC_IwsCnt* pc_IwsCnt = APIC_IwsCnt::newInstance();
	uint32_t ul_count = 0;

	// IWSアプリ情報取得
	TYPS_IwsAppInfo tempAppInfo = { 0 };

	pc_IWSAppInfo->lock();
	int ret = pc_IWSAppInfo->getApplication( (int)tmpxId.minorId , tempAppInfo );
	if( TYPD_IwsResult_OK == ret ){
		ul_count = pc_IwsCnt->get( tempAppInfo.sc_ID, TYPE_IwsCnt_AppStartUp );	// IWSアプリ利用頻度取得.
	}else{
		ul_count = 0;	// アプリ登録なしの場合は0とする.
	}
	pc_IWSAppInfo->unlock();

	// IWS:利用頻度.
	sprintf(temp, "%lu", ul_count );

	memcpy(p_str, temp, SIZE_TAG);
	return strlen(temp);
}
//=================================================================================
//	関数	：getOneData_AppUseFreqCount_IwsPrintCnt.
//	機能	：IWSアプリ利用頻度カウンタの実行時の印刷実行回数を取得する.
//	引数	：.
//			p_str : 文字列を格納する領域(1024バイトの領域を期待).
//			xid   : テーブルから検索する分類ID用構造体のアドレス.
//	戻り値	：.
//			1以上 :正常値（文字列長）.
//=================================================================================
int32_t getOneData_AppUseFreqCount_IwsPrintCnt(char *p_str, const RDSS_XmlMatrixId *xid )
{
	char temp[SIZE_TAG] = {""};
	RDSS_XmlMatrixId tmpxId = *xid;
	APIC_IWSAppInfo* pc_IWSAppInfo = APIC_IWSAppInfo::newInstance();
	APIC_IwsCnt* pc_IwsCnt = APIC_IwsCnt::newInstance();
	uint32_t ul_count = 0;

	// IWSアプリ情報取得
	TYPS_IwsAppInfo tempAppInfo = { 0 };

	pc_IWSAppInfo->lock();
	int ret = pc_IWSAppInfo->getApplication( (int)tmpxId.minorId , tempAppInfo );
	if( TYPD_IwsResult_OK == ret ){
		ul_count = pc_IwsCnt->get( tempAppInfo.sc_ID, TYPE_IwsCnt_PrintJob );	// IWSアプリ利用頻度取得.
	}else{
		ul_count = 0;	// アプリ登録なしの場合は0とする.
	}
	pc_IWSAppInfo->unlock();

	// IWS:印刷実行回数.
	sprintf(temp, "%lu", ul_count );

	memcpy(p_str, temp, SIZE_TAG);
	return strlen(temp);
}
//=================================================================================
//	関数	：getOneData_AppUseFreqCount_OapName.
//	機能	：OpenAPIアプリ利用頻度カウンタのアプリ名＋識別情報を取得する.
//	引数	：.
//			p_str : 文字列を格納する領域(1024バイトの領域を期待).
//			xid   : テーブルから検索する分類ID用構造体のアドレス.
//	戻り値	：.
//			1以上 :正常値（文字列長）.
//=================================================================================
int32_t getOneData_AppUseFreqCount_OapName(char *p_str, const RDSS_XmlMatrixId *xid )
{
	char temp[SIZE_TAG] = {""};
	RDSS_XmlMatrixId tmpxId = *xid;
	APIC_OpenApiCnt* pc_OpenApiCnt = APIC_OpenApiCnt::newInstance();

	// OpenAPI情報取得.
	TYPS_OpenApiCntInfo	t_OpenApiCntInfo = { { 0 } };	// 2016/01/18 ktakahashi warning除去対応.

	pc_OpenApiCnt->lock();
	pc_OpenApiCnt->getApplicationInfo(  tmpxId.minorId, &t_OpenApiCntInfo);
	pc_OpenApiCnt->unlock();

	// getApplicationInfo()で取得した情報を使用するため、isOpenAPIappExist()は使用しない.
	if( 0 == t_OpenApiCntInfo.ss_AppNo ){
		// ss_AppNoが0の場合はアプリ登録されていないのでリターン.
		return 0;	// 情報が無くても構成情報は出力するので0を返す.
	}
			
	// OpenAPI:アプリ名＋識別情報.
	// 2015/11/24 M.Adachi [ZeusS_ZX0]CQ519679_静的解析対応 START.
	snprintf(temp, sizeof( temp ), "%s%d%d%d%s%d%s%s"	, t_OpenApiCntInfo.sc_NameUTF8			// アプリケーション名.
								  						, t_OpenApiCntInfo.s_Version.e_Type			// アプリケーションタイプ.
								  						, t_OpenApiCntInfo.s_Version.ss_Major		// メジャーバージョン.
								  						, t_OpenApiCntInfo.s_Version.ss_Minor		// マイナーバージョン.
								  						, t_OpenApiCntInfo.s_Version.sc_Special		// スペシャル.
								  						, t_OpenApiCntInfo.ss_AppNo					// アプリケーション番号.
								  						, t_OpenApiCntInfo.uc_VendorCode			// ベンダーコード.
								  						, t_OpenApiCntInfo.uc_ApplicationCode );	// アプリケーションコード.
	// 2015/11/24 M.Adachi [ZeusS_ZX0]CQ519679_静的解析対応 END.

	memcpy(p_str, temp, SIZE_TAG);
	return strlen(temp);
}
//=================================================================================
//	関数	：isOpenAPIappExist.
//	機能	：OpenAPIアプリ情報が登録されているか判定する.
//	引数	：.
//			uc_Id : アプリケーションNo（minor_id）.
//	戻り値	：.
//			True :アプリ登録あり.
//			False:アプリ登録なし.
//=================================================================================
static Bool isOpenAPIappExist(const uchar uc_Id )
{
	TYPS_OpenApiCntInfo	t_OpenApiCntInfo = { { 0 } };	// 2016/01/18 ktakahashi warning除去対応.

	APIC_OpenApiCnt* pc_OpenApiCnt = APIC_OpenApiCnt::newInstance();

	// OpenAPI情報取得.
	pc_OpenApiCnt->lock();
	pc_OpenApiCnt->getApplicationInfo(  uc_Id, &t_OpenApiCntInfo);
	pc_OpenApiCnt->unlock();

	if( 0 == t_OpenApiCntInfo.ss_AppNo ){
		// ss_AppNoが0の場合はアプリ登録されていない.
		return False;	// アプリ登録なし.
	}
	
	return True;	// アプリ登録あり.
}

//=================================================================================
//	関数	：getOneData_AppUseFreqCount_OapFreq.
//	機能	：OpenAPIアプリ利用頻度カウンタの利用頻度を取得する.
//	引数	：.
//			p_str : 文字列を格納する領域(1024バイトの領域を期待).
//			xid   : テーブルから検索する分類ID用構造体のアドレス.
//	戻り値	：.
//			1以上 :正常値（文字列長）.
//=================================================================================
int32_t getOneData_AppUseFreqCount_OapFreq(char *p_str, const RDSS_XmlMatrixId *xid )
{
	char temp[SIZE_TAG] = {""};
	uint32_t ul_count = 0;
	RDSS_XmlMatrixId tmpxId = *xid;
	APIC_OpenApiCnt* pc_OpenApiCnt = APIC_OpenApiCnt::newInstance();

	// アプリNoが有効か判定
	if( False == isOpenAPIappExist( tmpxId.minorId ) ){
		ul_count = 0;	// アプリ登録なしの場合は0とする.
		sprintf(temp, "%lu", ul_count );
		memcpy(p_str, temp, SIZE_TAG);
		return strlen(temp);
	}

	// OpenAPI利用頻度取得.
	pc_OpenApiCnt->lock();
	ul_count = pc_OpenApiCnt->get(  tmpxId.minorId );
	pc_OpenApiCnt->unlock();

	sprintf(temp, "%lu", ul_count );
	memcpy(p_str, temp, SIZE_TAG);
	return strlen(temp);
}
//=================================================================================
//	関数	：getOneData_AppUseFreqCount_OapPullPrint.
//	機能	：OpenAPIアプリ利用頻度カウンタのPullPrintアプリ実行回数を取得する.
//	引数	：.
//			p_str : 文字列を格納する領域(1024バイトの領域を期待).
//			xid   : テーブルから検索する分類ID用構造体のアドレス（未使用）.
//	戻り値	：.
//			1以上 :正常値（文字列長）.
//=================================================================================
int32_t getOneData_AppUseFreqCount_OapPullPrint(char *p_str, const RDSS_XmlMatrixId *xid )
{
	char temp[SIZE_TAG] = {""};
	APIC_OpenApiCnt* pc_OpenApiCnt = APIC_OpenApiCnt::newInstance();

	// OpenAPI利用頻度取得.
	pc_OpenApiCnt->lock();
	uint32_t ul_count = pc_OpenApiCnt->getPullPrintJob();
	pc_OpenApiCnt->unlock();

	sprintf(temp, "%lu", ul_count );
	memcpy(p_str, temp, SIZE_TAG);
	return strlen(temp);
}

//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36 Start.
//=================================================================================
//	関数	：getOneData_AppUseFreqCount_UbiOperationHistoryStorage.
//	機能	：サーバレスユビキタスプリントのユーザ操作履歴(ストレージ機能)取得.
//	引数	：.
//			p_str : 文字列を格納する領域(1024バイトの領域を期待).
//			xid   : テーブルから検索する分類ID用構造体のアドレス（未使用）.
//	戻り値	：.
//			0以上 :正常値（文字列長）.
//			0未満 :異常値
//=================================================================================
int32_t getOneData_AppUseFreqCount_UbiOperationHistoryStorage(char *p_str, const RDSS_XmlMatrixId *xid )
{
	
	// 情報確保用一時変数定義.
	char		c_temp[SIZE_TAG]	= {""};
	uint32_t	ui_temp 			= 0;
	TYPC_Time	c_Time;
	uchar		uc_index;

	// RDSS_XmlMatrixIdの一時変数.
	RDSS_XmlMatrixId tempId = *xid;

	// tempId.minorIdの範囲チェック.
	if( tempId.minorId < 1 || 100 < tempId.minorId )
	{
		// 範囲外の場合、不正値を返却して終了.
		return -1;
	}
	else
	{
		// tempId.minorIdが正常範囲である場合、getに指定するインデックスに変換する.
		// インデックスは0～99.
		uc_index = tempId.minorId - 1;
	}

	// インスタンスの取得.
	APIC_UbiquitousPrintOpHistoryList* pc_UbiquitousPrintOpHistoryList = APIC_UbiquitousPrintOpHistoryList::newInstance();

	// ユーザ操作履歴(ストレージ機能)の構造体.
	TYPS_UbiquitousPrintOpHistorySt s_OpHistory = {0};

	// サーバレスユビキタスプリントのユーザ操作履歴(ストレージ機能)の取得.
	// minorIdを履歴番号の特定に、c_Idを取得するデータ種別の特定に使用する.
	pc_UbiquitousPrintOpHistoryList->lock();
	// ユーザ操作履歴(ストレージ機能)の構造体に値を取得する.
	pc_UbiquitousPrintOpHistoryList->get( uc_index, &s_OpHistory );
	pc_UbiquitousPrintOpHistoryList->unlock();

	switch( tempId.c_Id )
	{
	case 1:		// 処理開始時間.
		c_Time  = s_OpHistory.m_c_StartTime;
		break;
	case 2:		// 処理完了時間.
		c_Time  = s_OpHistory.m_c_EndTime;
		break;
	case 3:		// 処理種別.
		ui_temp = s_OpHistory.m_us_FuncKind;
		break;
	case 4:		// 処理結果.
		ui_temp = s_OpHistory.m_us_Result;
		break;
	case 5:		// ユーザーID.
		ui_temp = s_OpHistory.m_ul_UserID;
		break;
	case 6:		// アクセスポート.
		ui_temp = s_OpHistory.m_e_Port;
		break;
	case 7:		// 印刷設定：部数.
		ui_temp = s_OpHistory.m_sl_NumOfCopies;
		break;
	case 8:		// 印刷設定：カラー.
		ui_temp = s_OpHistory.m_e_ColorMode;
		break;
	case 9:		// 印刷設定：両面.
		ui_temp = s_OpHistory.m_e_PrintFace;
		break;
	case 10:	// 印刷設定：PS/PCL.
		ui_temp = s_OpHistory.m_e_LANGUAGE;
		break;
	case 11:	// ファイルサイズ.
		ui_temp = s_OpHistory.m_ul_PdlSize;
		break;
	default:	// c_Idが不正な値.
		// 不正値を返却して終了.
		return -1;
	}

	// 取得したデータをc_tempに込める処理.
	switch( tempId.c_Id )
	{
	case 1:		// 処理開始時間 YYYYMMDDhhmmss.
	case 2:		// 処理完了時間 YYYYMMDDhhmmss.
		snprintf( c_temp, sizeof( c_temp ), "%04d%02d%02d%02d%02d%02d", c_Time.ss_Year
																	, c_Time.sc_Month
																	, c_Time.sc_Day
																	, c_Time.sc_Hour
																	, c_Time.sc_Minute
																	, c_Time.sc_Second );
		break;
	case 3:		// 処理種別.
	case 4:		// 処理結果.
	case 5:		// ユーザーID.
	case 6:		// アクセスポート.
	case 7:		// 印刷設定：部数.
	case 8:		// 印刷設定：カラー.
	case 9:		// 印刷設定：両面.
	case 10:	// 印刷設定：PS/PCL.
	case 11:	// ファイルサイズ.
		snprintf(c_temp, sizeof( c_temp ), "%u", ui_temp );
		break;
	default:	// c_Idが不正な値.
		// 不正値を返却して終了の処理とするが、ここには入らないはず.
		return -1;
	}

	memcpy(p_str, c_temp, SIZE_TAG);
	return strlen(c_temp);
}
//=================================================================================
//	関数	：getOneData_AppUseFreqCount_UbiOperationHistoryClient.
//	機能	：サーバレスユビキタスプリントのユーザ操作履歴(クライアント機能)取得.
//	引数	：.
//			p_str : 文字列を格納する領域(1024バイトの領域を期待).
//			xid   : テーブルから検索する分類ID用構造体のアドレス（未使用）.
//	戻り値	：.
//			0以上 :正常値（文字列長）.
//			0未満 :異常値
//=================================================================================
int32_t getOneData_AppUseFreqCount_UbiOperationHistoryClient(char *p_str, const RDSS_XmlMatrixId *xid )
{
	
	// 情報確保用一時変数定義.
	char		c_temp[SIZE_TAG]	= {""};
	uint32_t	ui_temp 			= 0;
	TYPC_Time	c_Time;
	uchar		uc_index;

	// RDSS_XmlMatrixIdの一時変数.
	RDSS_XmlMatrixId tempId = *xid;

	// tempId.minorIdの範囲チェック.
	if( tempId.minorId < 1 || 100 < tempId.minorId )
	{
		// 範囲外の場合、不正値を返却して終了.
		return -1;
	}
	else
	{
		// tempId.minorIdが正常範囲である場合、getに指定するインデックスに変換する.
		// インデックスは0～99.
		uc_index = tempId.minorId - 1;
	}

	// インスタンスの取得.
	APIC_UbiquitousPrintOpHistoryList* pc_UbiquitousPrintOpHistoryList = APIC_UbiquitousPrintOpHistoryList::newInstance();

	// ユーザ操作履歴(クライアント機能)の構造体.
	TYPS_UbiquitousPrintOpHistoryCl s_OpHistory = {0};

	// サーバレスユビキタスプリントのユーザ操作履歴(ストレージ機能)の取得.
	// minorIdを履歴番号の特定に、c_Idを取得するデータ種別の特定に使用する.
	pc_UbiquitousPrintOpHistoryList->lock();
	// ユーザ操作履歴(クライアント機能)の構造体に値を取得する.
	pc_UbiquitousPrintOpHistoryList->get( uc_index, &s_OpHistory );
	pc_UbiquitousPrintOpHistoryList->unlock();

	switch( tempId.c_Id )
	{
	case 1:		// 処理開始時間.
		c_Time  = s_OpHistory.m_c_StartTime;
		break;
	case 2:		// 処理完了時間.
		c_Time  = s_OpHistory.m_c_EndTime;
		break;
	case 3:		// 処理種別.
		ui_temp = s_OpHistory.m_us_FuncKind;
		break;
	case 4:		// 処理結果.
		ui_temp = s_OpHistory.m_us_Result;
		break;
	case 5:		// ユーザーID.
		ui_temp = s_OpHistory.m_ul_UserID;
		break;
	case 6:		// 対象文書数.
		ui_temp = s_OpHistory.m_ul_DocumentNum;
		break;
	case 7:		// 対象枚数.
		ui_temp = s_OpHistory.m_ul_PrintNum;
		break;
	default:	// c_Idが不正な値.
		// 不正値を返却して終了.
		return -1;
	}

	// 取得したデータをc_tempに込める処理.
	switch( tempId.c_Id )
	{
	case 1:		// 処理開始時間 YYYYMMDDhhmmss.
	case 2:		// 処理完了時間 YYYYMMDDhhmmss.
		snprintf( c_temp, sizeof( c_temp ), "%d%02d%02d%02d%02d%02d", c_Time.ss_Year
																	, c_Time.sc_Month
																	, c_Time.sc_Day
																	, c_Time.sc_Hour
																	, c_Time.sc_Minute
																	, c_Time.sc_Second );
		break;
	case 3:		// 処理種別.
	case 4:		// 処理結果.
	case 5:		// ユーザーID.
	case 6:		// 対象文書数.
	case 7:		// 対象枚数.
		snprintf(c_temp, sizeof( c_temp ), "%u", ui_temp );
		break;
	default:	// c_Idが不正な値.
		// 不正値を返却して終了の処理とするが、ここには入らないはず.
		return -1;
	}

	memcpy(p_str, c_temp, SIZE_TAG);
	return strlen(c_temp);
}
//=================================================================================
//	関数	：getOneData_AppUseFreqCount_UbiOperationHistoryTopology.
//	機能	：サーバレスユビキタスプリントのユーザ操作履歴(トポロジーマネージャー機能)取得.
//	引数	：.
//			p_str : 文字列を格納する領域(1024バイトの領域を期待).
//			xid   : テーブルから検索する分類ID用構造体のアドレス（未使用）.
//	戻り値	：.
//			1以上 :正常値（文字列長）.
//=================================================================================
int32_t getOneData_AppUseFreqCount_UbiOperationHistoryTopology(char *p_str, const RDSS_XmlMatrixId *xid )
{
	
	// 情報確保用一時変数定義.
	char		c_temp[SIZE_TAG]	= {""};
	uint32_t	ui_temp 			= 0;
	uchar*		uc_temp;
	TYPC_Time	c_Time;
	uchar		uc_index;

	// RDSS_XmlMatrixIdの一時変数.
	RDSS_XmlMatrixId tempId = *xid;

	// tempId.minorIdの範囲チェック.
	if( tempId.minorId < 1 || 100 < tempId.minorId )
	{
		// 範囲外の場合、不正値を返却して終了.
		return -1;
	}
	else
	{
		// tempId.minorIdが正常範囲である場合、getに指定するインデックスに変換する.
		// インデックスは0～99.
		uc_index = tempId.minorId - 1;
	}

	// インスタンスの取得.
	APIC_UbiquitousPrintOpHistoryList* pc_UbiquitousPrintOpHistoryList = APIC_UbiquitousPrintOpHistoryList::newInstance();

	// ユーザ操作履歴(トポロジーマネージャー機能)の構造体.
	TYPS_UbiquitousPrintOpHistoryTm s_OpHistory = {0};

	// サーバレスユビキタスプリントのユーザ操作履歴(ストレージ機能)の取得.
	// minorIdを履歴番号の特定に、c_Idを取得するデータ種別の特定に使用する.
	pc_UbiquitousPrintOpHistoryList->lock();
	// ユーザ操作履歴(トポロジーマネージャー機能)の構造体に値を取得する.
	pc_UbiquitousPrintOpHistoryList->get( uc_index, &s_OpHistory );
	pc_UbiquitousPrintOpHistoryList->unlock();

	switch( tempId.c_Id )
	{
	case 1:		// 処理開始時間.
		c_Time  = s_OpHistory.m_c_StartTime;
		break;
	case 2:		// 処理完了時間.
		c_Time  = s_OpHistory.m_c_EndTime;
		break;
	case 3:		// 処理種別.
		ui_temp = s_OpHistory.m_us_FuncKind;
		break;
	case 4:		// 処理結果.
		ui_temp = s_OpHistory.m_us_Result;
		break;
	case 5:		// 相手先アドレス.
		uc_temp = s_OpHistory.m_uc_IPAdr;
		break;
	default:	// c_Idが不正な値.
		// 不正値を返却して終了.
		return -1;
	}

	// 取得したデータをc_tempに込める処理.
	switch( tempId.c_Id )
	{
	case 1:		// 処理開始時間 YYYYMMDDhhmmss.
	case 2:		// 処理完了時間 YYYYMMDDhhmmss.
		snprintf( c_temp, sizeof( c_temp ), "%d%02d%02d%02d%02d%02d", c_Time.ss_Year
																	, c_Time.sc_Month
																	, c_Time.sc_Day
																	, c_Time.sc_Hour
																	, c_Time.sc_Minute
																	, c_Time.sc_Second );
		break;
	case 3:		// 処理種別.
	case 4:		// 処理結果.
		snprintf(c_temp, sizeof( c_temp ), "%u", ui_temp );
		break;
	case 5:		// 相手先アドレス.
		snprintf(c_temp, sizeof( c_temp ), "%s",  uc_temp );
		break;
	default:	// c_Idが不正な値.
		// 不正値を返却して終了の処理とするが、ここには入らないはず.
		return -1;
	}

	memcpy(p_str, c_temp, SIZE_TAG);
	return strlen(c_temp);
}
//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36 End.


static _IDtoFuncTableAppUseFreqCount FuncTable_AppUseFreqCount[] =
{
	{	1,		1,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		1,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		1,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		2,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		2,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		2,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		3,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		3,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		3,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		4,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		4,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		4,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		5,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		5,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		5,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		6,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		6,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		6,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		7,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		7,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		7,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		8,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		8,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		8,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		9,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		9,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		9,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		10,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		10,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		10,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		11,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		11,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		11,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		12,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		12,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		12,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		13,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		13,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		13,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		14,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		14,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		14,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		15,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		15,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		15,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		16,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		16,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		16,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		17,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		17,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		17,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		18,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		18,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		18,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		19,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		19,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		19,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		20,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		20,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		20,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		21,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		21,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		21,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		22,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		22,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		22,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		23,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		23,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		23,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		24,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		24,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		24,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		25,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		25,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		25,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		26,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		26,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		26,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		27,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		27,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		27,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		28,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		28,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		28,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		29,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		29,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		29,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		30,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		30,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		30,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		31,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		31,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		31,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		32,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		32,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		32,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		33,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		33,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		33,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		34,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		34,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		34,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		35,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		35,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		35,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		36,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		36,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		36,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		37,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		37,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		37,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		38,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		38,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		38,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		39,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		39,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		39,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		40,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		40,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		40,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		41,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		41,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		41,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		42,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		42,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		42,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		43,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		43,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		43,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		44,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		44,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		44,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		45,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		45,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		45,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		46,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		46,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		46,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		47,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		47,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		47,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		48,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		48,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		48,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		49,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		49,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		49,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	1,		50,		1,		getOneData_AppUseFreqCount_IwsName },
	{	1,		50,		2,		getOneData_AppUseFreqCount_IwsFreq },
	{	1,		50,		3,		getOneData_AppUseFreqCount_IwsPrintCnt },
	{	2,		1,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		1,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		2,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		2,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		3,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		3,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		4,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		4,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		5,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		5,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		6,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		6,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		7,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		7,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		8,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		8,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		9,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		9,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		10,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		10,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		11,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		11,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		12,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		12,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		13,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		13,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		14,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		14,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		15,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		15,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		16,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		16,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		17,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		17,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		18,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		18,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		19,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		19,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		20,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		20,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		21,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		21,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		22,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		22,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		23,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		23,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		24,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		24,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		25,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		25,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		26,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		26,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		27,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		27,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		28,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		28,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		29,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		29,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		30,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		30,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		31,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		31,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		32,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		32,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		33,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		33,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		34,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		34,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		35,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		35,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		36,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		36,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		37,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		37,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		38,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		38,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		39,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		39,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		40,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		40,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		41,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		41,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		42,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		42,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		43,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		43,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		44,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		44,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		45,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		45,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		46,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		46,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		47,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		47,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		48,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		48,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		49,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		49,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		50,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		50,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		51,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		51,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		52,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		52,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		53,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		53,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		54,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		54,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		55,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		55,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		56,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		56,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		57,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		57,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		58,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		58,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		59,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		59,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		60,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		60,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		61,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		61,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		62,		1,		getOneData_AppUseFreqCount_OapName },
	{	2,		62,		2,		getOneData_AppUseFreqCount_OapFreq },
	{	2,		90,		1,		getOneData_AppUseFreqCount_OapPullPrint },
	{	3,		1,		1,		getOneData_AppUseFreqCount_UbiOperationHistoryStorage  },	//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	4,		1,		1,		getOneData_AppUseFreqCount_UbiOperationHistoryClient   },	//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
	{	5,		1,		1,		getOneData_AppUseFreqCount_UbiOperationHistoryTopology },	//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36.
};

//=================================================================================
//	関数	：getOneData_AppUseFreqCount.
//	機能	：IWSアプリ利用頻度カウンタの数値を文字列に変換して取得する.
//	引数	：.
//			p_str : 文字列を格納する領域(1024バイトの領域を期待).
//			xid   : テーブルから検索する分類ID用構造体のアドレス.
//			info  : アプリ利用頻度カウンタ情報
//	戻り値	：.
//			0以上 :正常値.
//			-1    :異常値.
//=================================================================================
int32_t getOneData_AppUseFreqCount(char *p_str, const RDSS_XmlMatrixId *xid )
{

	if( NULL == p_str || NULL == xid )
	{
		// ありえないがバカよけ.
		return -1;
	}

	// テーブル比較用.
	int max = sizeof(FuncTable_AppUseFreqCount) / sizeof(FuncTable_AppUseFreqCount[0]);
	RDSS_XmlMatrixId tempId = *xid;
	_IDtoFuncTableAppUseFreqCount matchTable = { 0 };

//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36 Start.
	// ユーザ走査履歴はテーブルに全て含めると2300行近くになるため、可読性や走査効率の点で問題がある.
	// majorIdが3～5である時点で、対応する関数を特定できるため.
	// 各履歴の先頭となる、3-1-1、4-1-1、5-1-1のみをテーブルに登録することとした.
	// 上記の経緯より、ユーザ操作履歴取得の場合は、minorIdとc_Idを1にして走査する.
	// 取得された関数をコールするreturn時は、引数のxidを使用するため、tempIdの値を変更することに問題は無い.
	if(3 <= tempId.majorId && tempId.majorId <= 5)
	{
		tempId.minorId = 1;
		tempId.c_Id = 1;
	}
//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36 End.
//2018/08/10 FPT TruongNV6 IT6_1.0 [Sparrow] fix bug 1047377 Start.
	APIC_SystemInfo* pc_SystemInfo = APIC_SystemInfo::newInstance();
	pc_SystemInfo->lock();
	TYPE_MachineType e_Type = pc_SystemInfo->getMachineType();
	pc_SystemInfo->unlock();

	if ( (TYPE_MachineTypeSparrow2AIO == e_Type) || 
		 (TYPE_MachineTypeSparrow2SFP == e_Type) ) 
	{
		if (1 == tempId.majorId) 
		{
			return -1;
		}
	}
//2018/08/10 FPT TruongNV6 IT6_1.0 [Sparrow] fix bug 1047377 End.
	for(int i = 0; i < max; i++)
	{
		if(FuncTable_AppUseFreqCount[i].majorId == tempId.majorId
		  && FuncTable_AppUseFreqCount[i].minorId == tempId.minorId
		  && FuncTable_AppUseFreqCount[i].c_Id == tempId.c_Id)
		{
			matchTable = FuncTable_AppUseFreqCount[i];
		}
	}

	if( NULL == matchTable.p_func )
	{
		// テーブルに対象のIDが無いとき.
		return -1;
	}

	// 関数コール.
	return (*(matchTable.p_func))(p_str, xid);

}

//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36 Start.
//=================================================================================
//	関数	：getFuncPointer_UbiOperationHistory.
//	機能	：ユーザ操作履歴の取得関数のポインタを取得する.
//	引数	：.
//			p_str : 文字列を格納する領域(1024バイトの領域を期待).
//			xid   : テーブルから検索する分類ID用構造体のアドレス.
//			info  : アプリ利用頻度カウンタ情報
//	戻り値	：.
//			戻り値 long 引数(char *, const RDSS_XmlMatrixId * )の関数ポインタ.
//=================================================================================
GetXMLDataFuncPointer getFuncPointer_UbiOperationHistory( const RDSS_XmlMatrixId *xid )
{

	if( NULL == xid )
	{
		// ありえないがバカよけ.
		return NULL;
	}

	// テーブル比較用.
	int max = sizeof(FuncTable_AppUseFreqCount) / sizeof(FuncTable_AppUseFreqCount[0]);
	RDSS_XmlMatrixId tempId = *xid;
	_IDtoFuncTableAppUseFreqCount matchTable = { 0 };

	// ユーザ走査履歴はテーブルに全て含めると2300行近くになるため、可読性や走査効率の点で問題がある.
	// majorIdが3～5である時点で、対応する関数を特定できるため.
	// 各履歴の先頭となる、3-1-1、4-1-1、5-1-1のみをテーブルに登録することとした.
	// 上記の経緯より、ユーザ操作履歴取得の場合は、minorIdとc_Idを1にして走査する.
	// 取得された関数をコールするreturn時は、引数のxidを使用するため、tempIdの値を変更することに問題は無い.
	if(3 <= tempId.majorId && tempId.majorId <= 5)
	{
		tempId.minorId = 1;
		tempId.c_Id = 1;
	}

	for(int i = 0; i < max; i++)
	{
		if(FuncTable_AppUseFreqCount[i].majorId == tempId.majorId
		  && FuncTable_AppUseFreqCount[i].minorId == tempId.minorId
		  && FuncTable_AppUseFreqCount[i].c_Id == tempId.c_Id)
		{
			matchTable = FuncTable_AppUseFreqCount[i];
		}
	}

	// 関数ポインタの返却.
	return matchTable.p_func;

}
//2016/11/16 H.Tokuno IT5_3.4 制御構成変更_No.36 End.
// eof.
