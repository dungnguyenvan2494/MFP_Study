// ================================================================================================.
//! \file THRC_Port.cpp ＢＩＯＳ・入出力ポート・ビット単位の入出力ポート・パルス駆動可能な出力ポート・ＳＩＯ・リアルタイム・クロック.
// ================================================================================================.

/*================================================================================================
	インクルード・ファイル
================================================================================================*/
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#ifndef SIMULATOR	// AsicSim対応.
#include <term.h>
#include <sys/syscall.h>
#endif				// SIMULATOR.
#include "system/thr/thrInclude/THRC_Port.h"
#include "system/PicCps/PIC.h"
#include "system/thr/thrcom/THRC_Vxworks.h"
#include "system/thr/thrInclude/THRC_ThreadInstance.h"
#include "system/thr/thrInclude/THRC_PortInstance.h"
#include "system/thr/thrInclude/THRC_Portdef.h"
#include "system/thr/thrInclude/THRC_BSPif.h"
#include "system/thr/thrInclude/THRC_BSPifCom.h"
#include "system/sys/OsWrapper/SYSC_OSWrapper.h"
#include "system/thr/thrcom/ThrCommonDrvrIf.h"
#include "system/thr/thrcom/THRC_SharedMemory.h"
#include "Linux/Platform/Mediaif.h"				// 2022/09/22 OP_BTS-39386 山口 RTC時刻初期化【暫定】.

#include "system/sys/APIC_RdsApi.h"
// 【追加】自動評価対応 [start] 2008.08.11 鈴木希 SIM_PRT,SIM_IRCを実機環境でも使用できる様にする.
#if defined( DEF_SimAutoTest )		// 自動評価で有効.
#include "system/cnv/CNVC_CnvIf.h"
#endif	// #if defined( DEF_SimAutoTest )		// 自動評価で有効.
// 【追加】自動評価対応 [ end ] 2008.08.11 鈴木希 SIM_PRT,SIM_IRCを実機環境でも使用できる様にする.
#ifdef SIMULATOR
#include "../../simulator/SimShared/include/SIMC_SimConnectionIf.h" // SIMとの接続クラス 2013.07.05 kanai.
#endif

#if defined(COSIM)
#include "system/thr/COSIM/THRC_CosimTestSim.h"
#include "system/thr/COSIM/THRC_CosimDebug.h"
#endif	// #if defined(COSIM).
#include  "system/thr/thrcom/THRC_ThreadDebugReg.h"
#include  "system/thr/thrcom/THRC_ThreadDebug.h"
#ifdef TORNADO				// VXWORKSで有効.
#include <linux/rtc.h>

extern unsigned char uc_SelectKeyFlag;				/* 第２パネルSelectキー確認用フラグ ijc_asic_config.cpp */
extern void wait_10us( uint32_t );
#endif	// #ifdef TORNADO	// VXWORKSで有効.

//extern uint32_t ul_asic7_pciMemAdr2[0x400];.
//extern uintptr_t ul_asic22_MemAdr[11];
extern uintptr_t ul_asic2014_MemAdr[2];
extern uintptr_t ul_A800_MemAdr[2];

// 2013/07/02 KM(Morikawa) SoC LinuxSim JFM Merge.
#if !defined(SIMULATOR) || defined(FUM_LINUX)	//!BSP暫定対応. 
// #ifndef SIMULATOR	//!BSP暫定対応.
/* ターミナル割り当て状態 */
#define BSPLIB_TERM_STATE_0  0	// CPU Bridge UART0 = CSRC	   ／ Asic22 UART2 = D103.
#define BSPLIB_TERM_STATE_1  1	// CPU Bridge UART0 = MSC Term ／ Asic22 UART2 = PIC Term.
#define BSPLIB_TERM_STATE_2  2	// CPU Bridge UART0 = CSRC	   ／ Asic22 UART2 = MSC Term.
#define BSPLIB_TERM_STATE_3  3	// CPU Bridge UART0 = MSC Term ／ Asic22 UART2 = D103.
#define BSPLIB_TERM_STATE_4  4	// CPU Bridge UART0 = PIC Term ／ Asic22 UART2 = MSC Term.

// ターミナルモード取得
int32_t getTermState(){
	
	static Bool b_GetTermState = False;
	static uchar uc_TermState = BSPLIB_TERM_STATE_0;

	if ( False == b_GetTermState ) {
		uc_TermState = (uchar)(THRL_Drvrif_GetTermState());
		b_GetTermState = True;
	}
	
	return uc_TermState;
}
/*GaiaPF ◆End 2010/08/27 iyanagi ターミナル切替対応*/
#endif

/*================================================================================================
	ASIC7 I/O 設定 レジスタ定義 
================================================================================================*/

//#define	ASIC7_REG_PORT_PIO1					*(ulong*)(ul_asic7_pciMemAdr2+ASIC7_PIO1 )	/*	ＰＩＯデータ設定レジスタ１	入力ポート　PA		*/.
//#define	ASIC7_REG_PORT_PIO2					*(ulong*)(ul_asic7_pciMemAdr2+ASIC7_PIO2 )	/*	ＰＩＯデータ設定レジスタ２	出力ポート	PB		*/.

/*================================================================================================
	ASIC22 I/O 設定 レジスタ定義 
================================================================================================*/
//#define ASIC22_REG_PIO_0R		*(ushort*)(&ul_asic22_MemAdr[0])	//PIOデータ設定レジスタ0.
//#define ASIC22_REG_PIO_1R		*(ushort*)(&ul_asic22_MemAdr[1])	//PIOデータ設定レジスタ1.
//#define ASIC22_REG_PIO_2R		*(ushort*)(&ul_asic22_MemAdr[2])	//PIOデータ設定レジスタ2.

//#define ASIC22_REG_PIO_4R		*(ushort*)(&ul_asic22_MemAdr[3])	//PIOデータ設定レジスタ4.
//#define ASIC22_REG_PIO_5R		*(ushort*)(&ul_asic22_MemAdr[4])	//PIOデータ設定レジスタ5.
//#define ASIC22_REG_PIO_6R		*(ushort*)(&ul_asic22_MemAdr[5])	//PIOデータ設定レジスタ6.

//#define ASIC22_REG_RST_0R		*(ushort*)(&ul_asic22_MemAdr[6])	//RSTコントロール設定レジスタ0.
//#define ASIC22_REG_RST_1R		*(ushort*)(&ul_asic22_MemAdr[7])	//RSTコントロール設定レジスタ1.
//#define ASIC22_REG_RST_2R		*(ushort*)(&ul_asic22_MemAdr[8])	//RSTコントロール設定レジスタ2.
//#define ASIC22_REG_RST_3R		*(ushort*)(&ul_asic22_MemAdr[9])	//RSTコントロール設定レジスタ3.
//#define ASIC22_REG_RST_4R		*(ushort*)(&ul_asic22_MemAdr[10])	//RSTコントロール設定レジスタ4.

/*================================================================================================
	ASIC2014 I/O 設定 レジスタ定義 
================================================================================================*/
#define ASIC2014_REG_PIO_0R		*(ushort*)(&ul_asic2014_MemAdr[0])	//PIOデータ設定レジスタ0.
#define ASIC2014_REG_PIO_4R		*(ushort*)(&ul_asic2014_MemAdr[1])	//PIOデータ設定レジスタ4.

/*================================================================================================
	A800 I/O 設定 レジスタ定義 
================================================================================================*/
#define A800_REG_PI_PIO			*(uint32_t*)(&ul_A800_MemAdr[0])		//PIO入力データレジスタ.
#define A800_REG_PO_PIO			*(uint32_t*)(&ul_A800_MemAdr[1])		//PIO出力データレジスタ.

/*================================================================================================
	内部関数の定義
================================================================================================*/
//	12/11/09 戸田	CPU選定評価基板対応　GPIOの入出力ポートを追加.
template <> CB_Port<uint32_t>& CB_Port_Instance<uint32_t>::newInstance(CB_PortTypeNum type)
{
	int32_t tNum = (int32_t)type;

	static	uint32_t			ul_DummyPIO;

#ifdef SIMULATOR
	static	uint32_t			ul_ASIC2_PORTAsim;
	static	uint32_t			ul_PortA;
	static	uint32_t			ul_PortC;
#else
#ifndef COSIM
	static	uint32_t			ul_PortC;
#endif
#endif
//	static	ulong			ul_asic7_pciMemAdr2[0x400];.
	static	uint32_t			ul_Asic7PIO0/* = (uint32_t)(ASIC7_REG_PORT_PIO1)*/;
	static	uint32_t			ul_Asic7PIO1/* = (uint32_t)(ASIC7_REG_PORT_PIO2)*/;
	
	static	CB_Port<uint32_t>	ptASICXX_DUMMY_PIO		( ul_DummyPIO, 0x00000000 );			// ダミーのポート.
	static	CB_Port<uint32_t>	ptASIC7_PORT01			( ul_Asic7PIO0, 0xffffffff );	// ポートA	R	入力ポート.
	static	CB_Port<uint32_t>	ptASIC7_PORT02			( ul_Asic7PIO1, 0x00000000 );	// ポートA	R/W	出力ポート.
#ifdef SIMULATOR
	static	CB_Port<uint32_t>	ptASIC2_PORTASim		( ul_ASIC2_PORTAsim, 0xffff00fC );		// ポートA	R/W	入出力ポート（入力ポートとして使用）.
	static	CB_Port<uint32_t>	ptASICX_PORTA			( ul_PortA, 0xffff00fC );				// ポートA	R/W	入出力ポート（入力ポートとして使用）.
	static	CB_Port<uint32_t>	ptASICX_PORTC			( ul_PortC, 0x00000000 );				// ポートC	W	出力ポート.
#else
#ifndef COSIM
	static	CB_Port<uint32_t>	ptASICX_PORTC			( ul_PortC, 0x00000000 );				// ポートC	W	出力ポート.
#endif
#endif
	static	uint32_t			ul_A800_MemAdr[2];
	static	CB_Port<uint32_t>	ptA800_PI_PIO			( A800_REG_PI_PIO,	0xffffffff );		// 入力ポート.
	static	CB_Port<uint32_t>	ptA800_PO_PIO			( A800_REG_PO_PIO,	0x00000000 );		// 入力ポート.

	static CB_Port<uint32_t>* vList[] = {
		&ptASICXX_DUMMY_PIO,			//		CBPTN_ptASICXX_DUMMY_PIO,.
		NULL,							//		CBPTN_ptASIC22_DUMMY_PIO,.
		NULL,							//		CBPTN_ptASIC22_DUMMY_PIO2,.
		NULL,							//		CBPTN_ptAsic22_PIO_0R,.
		NULL,							//		CBPTN_ptAsic22_PIO_1R,.
		NULL,							//		CBPTN_ptAsic22_PIO_2R,.
		NULL,							//		CBPTN_ptAsic22_PIO_4R,.
		NULL,							//		CBPTN_ptAsic22_PIO_5R,.
		NULL,							//		CBPTN_ptAsic22_PIO_6R,.
		NULL,							//		CBPTN_ptAsic22_RST_0R,.
		NULL,							//		CBPTN_ptAsic22_RST_1R,.
		NULL,							//		CBPTN_ptAsic22_RST_2R,.
		NULL,							//		CBPTN_ptAsic22_RST_3R,.
		NULL,							//		CBPTN_ptAsic22_RST_4R,.
		NULL,							//		CBPTN_ptAsic2014_PIO_0R,.
		NULL,							//		CBPTN_ptAsic2014_PIO_4R,.
		&ptA800_PI_PIO,					// 		CBPTN_ptA800_PI_PIO.
		&ptA800_PO_PIO,					// 		CBPTN_ptA800_PO_PIO.
		&ptASIC7_PORT01,				//		CBPTN_ptASIC7_PORT01,.
		&ptASIC7_PORT02,				//		CBPTN_ptASIC7_PORT02,.
#ifdef SIMULATOR						//	#ifdef SIMULATOR.
		&ptASIC2_PORTASim,				//		CBPTN_ptASIC2_PORTASim,.
		&ptASICX_PORTA,					//		CBPTN_ptASICX_PORTA,.
		&ptASICX_PORTC,					//		CBPTN_ptASICX_PORTC,.
#else									//	#else.
#ifndef COSIM							//	#ifndef COSIM.
		&ptASICX_PORTC,					//		CBPTN_ptASICX_PORTC,.
#endif									//	#endif.
#endif									//	#endif.
	};

    THRL_ThAssert( (Bool)( vList[ tNum ] != NULL ), "CB_Port<uintptr_t> tNum: %"PRId32"", tNum, 0, 0, 0 , CFA14_DetailCode0x086D );

	return *vList[ tNum ];
}

//	12/11/09 戸田	CPU選定評価基板対応　PIO、RSTの入出力ポートを削除.
//	15/04/09 Sky山下 CQ510921 ワーニング対応(PIO、RSTの入出力ポート用の処理を削除).
template <> CB_Port<ushort>& CB_Port_Instance<ushort>::newInstance(CB_PortTypeNum type)
{
	int32_t tNum = (int32_t)type;

	static	ushort			us_DummyAsic22PIO;
	static	ushort			us_DummyAsic22PIO2;
//	static	uintptr_t			ul_asic22_MemAdr[11];
	static	uintptr_t			ul_asic2014_MemAdr[2];

	static	CB_Port<ushort>	ptASIC22_DUMMY_PIO		( us_DummyAsic22PIO,	0x0000 );
	static	CB_Port<ushort>	ptASIC22_DUMMY_PIO2		( us_DummyAsic22PIO2,	0x0000 );
//	static	CB_Port<ushort>	ptAsic22_PIO_0R			( ASIC22_REG_PIO_0R,	0xffff );		// 入力ポート.
//	static	CB_Port<ushort>	ptAsic22_PIO_1R			( ASIC22_REG_PIO_1R,	0xffff );		// 入力ポート.
//	static	CB_Port<ushort>	ptAsic22_PIO_2R			( ASIC22_REG_PIO_2R,	0xffff );		// 入力ポート.
//	static	CB_Port<ushort>	ptAsic22_PIO_4R			( ASIC22_REG_PIO_4R,	0x0000 );		// 出力ポート.
//	static	CB_Port<ushort>	ptAsic22_PIO_5R			( ASIC22_REG_PIO_5R,	0x0000 );		// 出力ポート.
//	static	CB_Port<ushort>	ptAsic22_PIO_6R			( ASIC22_REG_PIO_6R,	0x0000 );		// 出力ポート.
//	static	CB_Port<ushort>	ptAsic22_RST_0R			( ASIC22_REG_RST_0R,	0xffe0 );		// RSTコントロール設定レジスタ0.
//	static	CB_Port<ushort>	ptAsic22_RST_1R			( ASIC22_REG_RST_1R,	0xffe0 );		// RSTコントロール設定レジスタ1.
//	static	CB_Port<ushort>	ptAsic22_RST_2R			( ASIC22_REG_RST_2R,	0xffe0 );		// RSTコントロール設定レジスタ2.
//	static	CB_Port<ushort>	ptAsic22_RST_3R			( ASIC22_REG_RST_3R,	0xff00 );		// RSTコントロール設定レジスタ3.
//	static	CB_Port<ushort>	ptAsic22_RST_4R			( ASIC22_REG_RST_4R,	0x0000 );		// RSTコントロール設定レジスタ4.
	static	CB_Port<ushort>	ptAsic2014_PIO_0R		( ASIC2014_REG_PIO_0R,	0xffff );		// 入力ポート.
	static	CB_Port<ushort>	ptAsic2014_PIO_4R		( ASIC2014_REG_PIO_4R,	0x0000 );		// 出力ポート.

	static CB_Port<ushort>* vList[] = {
		NULL,							//		CBPTN_ptASICXX_DUMMY_PIO,.
		&ptASIC22_DUMMY_PIO,			//		CBPTN_ptASIC22_DUMMY_PIO,.
		&ptASIC22_DUMMY_PIO2,			//		CBPTN_ptASIC22_DUMMY_PIO2,.
		NULL,				//		CBPTN_ptAsic22_PIO_0R,.
		NULL,				//		CBPTN_ptAsic22_PIO_1R,.
		NULL,				//		CBPTN_ptAsic22_PIO_2R,.
		NULL,				//		CBPTN_ptAsic22_PIO_4R,.
		NULL,				//		CBPTN_ptAsic22_PIO_5R,.
		NULL,				//		CBPTN_ptAsic22_PIO_6R,.
		NULL,				//		CBPTN_ptAsic22_RST_0R,.
		NULL,				//		CBPTN_ptAsic22_RST_1R,.
		NULL,				//		CBPTN_ptAsic22_RST_2R,.
		NULL,				//		CBPTN_ptAsic22_RST_3R,.
		NULL,				//		CBPTN_ptAsic22_RST_4R,.
		&ptAsic2014_PIO_0R,				//		CBPTN_ptAsic2014_PIO_0R,.
		&ptAsic2014_PIO_4R,				//		CBPTN_ptAsic2014_PIO_4R,.
		NULL,							// 		CBPTN_ptA800_PI_PIO.
		NULL,							// 		CBPTN_ptA800_PO_PIO.
		NULL,							//		CBPTN_ptASIC7_PORT01,.
		NULL,							//		CBPTN_ptASIC7_PORT02,.
#ifdef SIMULATOR						//	#ifdef SIMULATOR.
		NULL,							//		CBPTN_ptASIC2_PORTASim,.
		NULL,							//		CBPTN_ptASICX_PORTA,.
		NULL,							//		CBPTN_ptASICX_PORTC,.
#else									//	#else.
#ifndef COSIM							//	#ifndef COSIM.
		NULL,							//		CBPTN_ptASICX_PORTC,.
#endif									//	#endif.
#endif									//	#endif.
	};

    THRL_ThAssert( (Bool)( vList[ tNum ] != NULL ), "CB_Port<ushort> tNum: %"PRId32"", tNum, 0, 0, 0 , CFA14_DetailCode0x086E );

	return *vList[ tNum ];
}

//	12/11/09 戸田	CPU選定評価基板対応　ushort用からulong用へ移植.
/*	2014/09/24 sky_tksk Fiery省電力対応 ポート制御対象変更 ASIC777⇒CPU GPIO */
//	15/01/08 Sky	S72基板対応 IO変更対応.
//	17/09/08 Sky戸田 S800 UART、GPIO対応.
//  19/09/04 Sky	OP_BTS-12689 SEL_COLOR 3bit更新対応.
template <> CB_BitPort<uint32_t>& CB_BitPort<uint32_t>::newInstance(CB_BitPortTypeNum type)
{
	int32_t tNum = (int32_t)type;

#ifdef COSIM
	static	CB_BitPort<uint32_t>	portOut_COSIM_IMAGE_CODE	( CBBPTN_portOut_COSIM_IMAGE_CODE		 );	// 画像切換コード.
	static	CB_BitPort<uint32_t>	portOut_COSIM_IMAGE_SIZE	( CBBPTN_portOut_COSIM_IMAGE_SIZE		 );	// サイズ切換コード.
	static	CB_BitPort<uint32_t>	portOut_COSIM_IMAGE			( CBBPTN_portOut_COSIM_IMAGE			 );	// IR画像入力設定ポート.
	static	CB_BitPort<uint32_t>	portOut_COSIM_SCAN_IN		( CBBPTN_portOut_COSIM_SCAN_IN			 );	// スキャナ入力起動　Lowアクティブ.
	static	CB_BitPort<uint32_t>	portOut_COSIM_FIERY_IN		( CBBPTN_portOut_COSIM_FIERY_IN			 );	// Fiery入力起動　Lowアクティブ.
#endif

	static	CB_BitPort<uint32_t>	portPrinterIsPowerOn		( CBBPTN_portPrinterIsPowerOn		,	CBPTN_ptASICXX_DUMMY_PIO,	6,	1, true  );	// 型試で未使用	//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portIORST1					( CBBPTN_portIORST1					,	CBPTN_ptASICXX_DUMMY_PIO,	0,	1, true  );	// リセット信号１	"L"でリセットアクティブ	//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portIORST2					( CBBPTN_portIORST2					,	CBPTN_ptASICXX_DUMMY_PIO,	1,	1, true  );	// リセット信号２	"L"でリセットアクティブ	//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portFILE_DIMM_INIT_1		( CBBPTN_portFILE_DIMM_INIT_1		,	CBPTN_ptASICXX_DUMMY_PIO,	14,	1, false );	// ファイルメモリバックアップ制御信号１	//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portS_O_PD					( CBBPTN_portS_O_PD					,	CBPTN_ptASICXX_DUMMY_PIO,	16,	1, false );	// ソート（電子ソート）出力パワーダウン　//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portFILE_DIMM_INIT_2		( CBBPTN_portFILE_DIMM_INIT_2		,	CBPTN_ptASICXX_DUMMY_PIO,	19,	1, false );	// ファイルメモリバックアップ制御信号２	//【課題】2005/10/25	橋本	Mosel未使用.
#ifdef DEF_EDH_COLOR
	static	CB_BitPort<uint32_t>	portAE_TG					( CBBPTN_portAE_TG					,	CBPTN_ptASICXX_DUMMY_PIO,	19,	1, True  );	// リアルタイムAEトリガ信号.
	static	CB_BitPort<uint32_t>	portFB_AE3					( CBBPTN_portFB_AE3					,	CBPTN_ptASICXX_DUMMY_PIO,	18,	1, False );	// リアルタイムAEフィードバック信号[3].
	static	CB_BitPort<uint32_t>	portFB_AE2					( CBBPTN_portFB_AE2					,	CBPTN_ptASICXX_DUMMY_PIO,	13,	1, False );	// リアルタイムAEフィードバック信号[2].
	static	CB_BitPort<uint32_t>	portFB_AE1					( CBBPTN_portFB_AE1					,	CBPTN_ptASICXX_DUMMY_PIO,	7,	1, False );	// リアルタイムAEフィードバック信号[1].
	static	CB_BitPort<uint32_t>	portFB_AE0					( CBBPTN_portFB_AE0					,	CBPTN_ptASICXX_DUMMY_PIO,	6,	1, False );	// リアルタイムAEフィードバック信号[0].
#endif
	static	CB_BitPort<uint32_t>	portServiceChk				( CBBPTN_portServiceChk				,	CBPTN_ptASICXX_DUMMY_PIO,	0,	2, false );	//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portTroubleReset			( CBBPTN_portTroubleReset			,	CBPTN_ptASICXX_DUMMY_PIO,	2,	1, false );	// トラブルリセットＯＮ　//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portMemoryClear				( CBBPTN_portMemoryClear			,	CBPTN_ptASICXX_DUMMY_PIO,	3,	1, false );	// メモリクリアＯＮ　//【課題】2005/10/25	橋本	Mosel未使用.
#ifndef SIMULATOR
	static	CB_BitPort<uint32_t>	portInitial					( CBBPTN_portInitial				,	CBPTN_ptASICXX_DUMMY_PIO,	2,	1, false );	// イニシャルＯＮ　//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portTotalClear				( CBBPTN_portTotalClear				,	CBPTN_ptASICXX_DUMMY_PIO,	3,	1, false );	// トータルクリアＯＮ　//【課題】2005/10/25	橋本	Mosel未使用.
#else
	static	CB_BitPort<uint32_t>	portInitial					( CBBPTN_portInitial				,	CBPTN_ptASIC2_PORTASim	,	2,	1, false );	// イニシャルＯＮ　//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portTotalClear				( CBBPTN_portTotalClear				,	CBPTN_ptASIC2_PORTASim	,	3,	1, false );	// トータルクリアＯＮ　//【課題】2005/10/25	橋本	Mosel未使用.
#endif	// #ifndef SIMULATOR.

	static	CB_BitPort<uint32_t>	portOut_DEBUG_PIC_1			( CBBPTN_portOut_DEBUG_PIC_1		,	CBPTN_ptASICXX_DUMMY_PIO,	8,	1, true  );	// PICデバッグ用出力１	//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portOut_DEBUG_PIC_2			( CBBPTN_portOut_DEBUG_PIC_2		,	CBPTN_ptASICXX_DUMMY_PIO,	9,	1, true  );	// PICデバッグ用出力２	//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portOut_DEBUG_PIC_3			( CBBPTN_portOut_DEBUG_PIC_3		,	CBPTN_ptASICXX_DUMMY_PIO,	10,	1, true  );	// PICデバッグ用出力３	//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portOut_DEBUG_PIC_4			( CBBPTN_portOut_DEBUG_PIC_4		,	CBPTN_ptASICXX_DUMMY_PIO,	11,	1, true  );	// PICデバッグ用出力４	//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portSEL_IR					( CBBPTN_portSEL_IR					,	CBPTN_ptASICXX_DUMMY_PIO,	12,	1, false );	// H：ｺﾝﾄﾛｰﾗﾃﾞｰﾀ選択	/	L：IRﾃﾞｰﾀ選択//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portSEL_NT					( CBBPTN_portSEL_NT					,	CBPTN_ptASICXX_DUMMY_PIO,	13,	1, false );	// H：NTSｽｷｬﾝｱｳﾄ(IRﾀﾞｲﾚｸﾄ出力)	/	L：NewTｽｷｬﾝｱｳﾄ(ASIC1/3経由出力)//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portP_VEN					( CBBPTN_portP_VEN					,	CBPTN_ptASICXX_DUMMY_PIO,	14,	1, false );	// H：ｽｷｬﾝｱｳﾄ画像送信許可	/	L：ｽｷｬﾝｱｳﾄ画像送信禁止//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portCTS_TERM				( CBBPTN_portCTS_TERM				,	CBPTN_ptASICXX_DUMMY_PIO,	0,	1, true  );	// CTS_TERM	//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portCARDIN1					( CBBPTN_portCARDIN1				,	CBPTN_ptASICXX_DUMMY_PIO,	2,	1, true  );	// /CARDIN1		//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portFUN_LOCK				( CBBPTN_portFUN_LOCK				,	CBPTN_ptASICXX_DUMMY_PIO,	3,	1, false );	// /FAN_LOCK	FANLOCK検出	"H"ロック	//【課題】2005/10/25	橋本	Mosel未使用.
#ifndef SIMULATOR	// 実機で有効.
	static	CB_BitPort<uint32_t>	portMSC_DTR					( CBBPTN_portMSC_DTR				,	CBPTN_ptASICXX_DUMMY_PIO,	5,	1, false );	// MSC_DTR	//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portSSC_DSR					( CBBPTN_portSSC_DSR				,	CBPTN_ptASICXX_DUMMY_PIO,	6,	1, false );	// SSC_DSR	//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portNTF_CNCT				( CBBPTN_portNTF_CNCT				,	CBPTN_ptASICXX_DUMMY_PIO,	7,	1, true  );	// NTF_CNCT	//【課題】2005/10/25	橋本	Mosel未使用.
#else // シミュレータで有効.
	static	CB_BitPort<uint32_t>	portMSC_DTR					( CBBPTN_portMSC_DTR				,	CBPTN_ptASICXX_DUMMY_PIO,	5,	1, true  );	// MSC_DTR	//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portSSC_DSR					( CBBPTN_portSSC_DSR				,	CBPTN_ptASICXX_DUMMY_PIO,	6,	1, true  );	// SSC_DSR	//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portNTF_CNCT				( CBBPTN_portNTF_CNCT				,	CBPTN_ptASICXX_DUMMY_PIO,	7,	1, true  );	// NTF_CNCT	//【課題】2005/10/25	橋本	Mosel未使用.
#endif	// #ifdef SIMULATOR.
	static	CB_BitPort<uint32_t>	portCLOSE					( CBBPTN_portCLOSE					,	CBPTN_ptASICXX_DUMMY_PIO,	11,	1, true  );	// CLOSE	//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portOut_CLK_INITIAL			( CBBPTN_portOut_CLK_INITIAL		,	CBPTN_ptASICXX_DUMMY_PIO,	1,	6, true  );	// クロック初期出力　//【課題】Mosel	2005/10/14	橋本	起動時に	クロック制御ポート初期化で必要　ポートの定義が重複している.
	static	CB_BitPort<uint32_t>	portRTS						( CBBPTN_portRTS					,	CBPTN_ptASICXX_DUMMY_PIO,	17,	1, true  );	// 	//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portSSC_DTR					( CBBPTN_portSSC_DTR				,	CBPTN_ptASICXX_DUMMY_PIO,	17,	1, false );	// SSC_DTR	//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portIMG_THROUGH				( CBBPTN_portIMG_THROUGH			,	CBPTN_ptASICXX_DUMMY_PIO,	13,	1, true  );	// CPS5600ULTRA	IMG_THROUGH	Lo:解像度変換部経由	Hi:解像度変換部スルー.
	static	CB_BitPort<uint32_t>	portC_O_PD2					( CBBPTN_portC_O_PD2				,	CBPTN_ptASICXX_DUMMY_PIO,	5,	1, true  );	// コピヤ出力パワーダウン　	//【課題】2005/10/25	橋本	Mosel未使用.
	static	CB_BitPort<uint32_t>	portC_O_PD					( CBBPTN_portC_O_PD					,	CBPTN_ptASICXX_DUMMY_PIO,	2,	1, true  );	// コピヤ出力パワーダウン　	//【課題】2005/10/25	橋本	Mosel未使用.

#ifdef SIMULATOR	// SIMWINで有効.
	static	CB_BitPort<uint32_t>	portVendorCopyEnable		( CBBPTN_portVendorCopyEnable		,	CBPTN_ptASICX_PORTA		,	4,	1, false );	// ベンダーコピー許可.
	static	CB_BitPort<uint32_t>	portVendorPaperSize			( CBBPTN_portVendorPaperSize		,	CBPTN_ptASICX_PORTC		,	1,	1, true  );	// ベンダー用紙サイズ.
	static	CB_BitPort<uint32_t>	portVendorCopying			( CBBPTN_portVendorCopying			,	CBPTN_ptASICX_PORTC		,	2,	1, true  );	// ベンダー・コピー中.
	static	CB_BitPort<uint32_t>	ppppVendorPaperFeed			( CBBPTN_ppppVendorPaperFeed		,	CBPTN_ptASICX_PORTC		,	3,	1, true  );	// ベンダー、給紙カウント.
	static	CB_BitPort<uint32_t>	ppppVendorPaperEject		( CBBPTN_ppppVendorPaperEject		,	CBPTN_ptASICX_PORTC		,	4,	1, true  );	// ベンダー、排紙カウント.
#else
#ifndef COSIM
	static	CB_BitPort<uint32_t>	ppppVendorPaperFeed			( CBBPTN_ppppVendorPaperFeed		,	CBPTN_ptASICX_PORTC		,	3,	1, true  );	// ベンダー、給紙カウント.
	static	CB_BitPort<uint32_t>	ppppVendorPaperEject		( CBBPTN_ppppVendorPaperEject		,	CBPTN_ptASICX_PORTC		,	4,	1, true  );	// ベンダー、排紙カウント.
#endif
#endif	// #ifdef SIMULATOR	// SIMWINで有効.
	static	CB_BitPort<uint32_t>	portTHCV216_LOCKN			( CBBPTN_portTHCV216_LOCKN );	// Fiery's VbyOneHS Link Lock Status.
	static	CB_BitPort<uint32_t>	portAPC_STATE				( CBBPTN_portAPC_STATE	 );	// APC_STATE.
	static	CB_BitPort<uint32_t>	portX_IREQ_P				( CBBPTN_portX_IREQ_P	 );	// /X_IREQ_P output.
	static	CB_BitPort<uint32_t>	portSEL_COLOR0				( CBBPTN_portSEL_COLOR0	 );	// SEL_COLOR0		PICのクロック信号.
//	static	CB_BitPort<uint32_t>	portSEL_COLOR1				( CBBPTN_portSEL_COLOR1	 );	// SEL_COLOR1		PICのクロック信号.
//	static	CB_BitPort<uint32_t>	portSEL_COLOR2				( CBBPTN_portSEL_COLOR2	 );	// SEL_COLOR2		PICのクロック信号.
	static	CB_BitPort<uint32_t>	portDIV_SEL_GP6				( CBBPTN_portDIV_SEL_GP6 );	// DIV_SEL_GP6.
	static	CB_BitPort<uint32_t>	portXON_GP6					( CBBPTN_portXON_GP6	 );	// XON_GP6.
	static CB_BitPort<uint32_t>* vList[] = {
		NULL,							//		CBBPTN_NON						,.
										//		// ASIC22 入力ポート	ASIC22_REG_PIO_0R.
		NULL,							//		CBBPTN_portDIPSW_0				,.
		NULL,							//		CBBPTN_portDIPSW_1				,.
		NULL,							//		CBBPTN_portDIPSW_2				,.
		NULL,							//		CBBPTN_portDIPSW_3				,.
		NULL,							//		CBBPTN_portUtilityKey			,.
		NULL,							//		CBBPTN_portIRC_CNCT				,.
		NULL,							//		CBBPTN_portTERMINAL_SEL			,.
		NULL,							//		CBBPTN_portFUN_LOCK_1			,.
		NULL,							//		CBBPTN_portCPRDY				,.
		NULL,							//		CBBPTN_portIMGRDY				,.
		NULL,							//		CBBPTN_portWAKE_FIERY			,. /* 2014/09/24 takas Fiery省電力対応 ポート制御対象変更 ASIC777⇒CPU GPIO */
		NULL,							//		CBBPTN_portSUB_SW_ON			,.
		NULL,							//		CBBPTN_portREV_BOARD_ON			,.

										//		// ASIC22 入力ポート	ASIC22_REG_PIO_1R.
		NULL,							//		CBBPTN_portPOWER_MONITOR		,.
		NULL,							//		CBBPTN_portENG_BUSY				,.
		NULL,							//		CBBPTN_portD102CopyEnable		,.
		NULL,							//		CBBPTN_portKeyCntIsSet			,.
		NULL,							//		CBBPTN_portTotalCntIsSet		,.
		NULL,							//		CBBPTN_portSEC_SET2				,.
		NULL,							//		CBBPTN_portSEC_SET1				,.
		NULL,							//		CBBPTN_portPG_SEC				,. // 2017/09/08 Sky戸田 S800 UART、GPIO対応.
		NULL,							//		CBBPTN_portFIERY_CNCT			,.
		NULL,							//		CBBPTN_portA800_PG				,. // 2017/09/08 Sky戸田 S800 UART、GPIO対応.
		NULL,							//		CBBPTN_portACC_DETECT			,. // 2017/09/08 Sky戸田 S800 UART、GPIO対応.
		NULL,							//		CBBPTN_portACC_DETECT2			,. // 2017/09/08 Sky戸田 S800 UART、GPIO対応.
		NULL,							//		CBBPTN_portMC_PWR_EN			,. // 2017/09/08 Sky戸田 S800 UART、GPIO対応.

										//		// ASIC22 出力ポート	ASIC22_REG_PIO_4R.
		NULL,							//		CBBPTN_portLED_0				,.
		NULL,							//		CBBPTN_portLED_1_N				,.
		NULL,							//		CBBPTN_portLED_2				,.
		NULL,							//		CBBPTN_portLED_3				,.
		NULL,							//		CBBPTN_portLED_4_N				,.
		NULL,							//		CBBPTN_portLED_5				,.
#ifdef DEF_MinervaS_PF
		NULL,							//		CBBPTN_portPMS_PLL2_ENB			,.
#endif // #ifdef DEF_MinervaS_PF.
		NULL,							//		CBBPTN_portAC_POWER_DOWN		,.
#if defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS )
		&portSEL_COLOR0,				//		CBBPTN_portSEL_COLOR0			,.
//		&portSEL_COLOR1,				//		CBBPTN_portSEL_COLOR1			,.
//		&portSEL_COLOR2,				//		CBBPTN_portSEL_COLOR2			,.
#else // #if defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS ).
		NULL,							//		CBBPTN_portPOWER_OFF_PNL		,.
		NULL,							//		CBBPTN_portPOWER_OFF			,.
#endif // #if defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS ).

#ifdef COSIM							//	#ifdef COSIM.
		NULL,							//		CBBPTN_portCOSIM_ASIC1_SDRAM1_DUMP	,.
		NULL,							//		CBBPTN_portCOSIM_ASIC1_SDRAM0_DUMP	,.
		NULL,							//		CBBPTN_portCOSIM_ASIC0_SDRAM1_DUMP	,.
		NULL,							//		CBBPTN_portCOSIM_ASIC0_SDRAM0_DUMP	,.
		NULL,							//		CBBPTN_portCOSIM_DummyPicOut		,.
		NULL,							//		CBBPTN_portCOSIM_PEX_MODE_1			,.
		NULL,							//		CBBPTN_portCOSIM_PEX_MODE_0			,.
		NULL,							//		CBBPTN_portCOSIM_HsyncModel_Width	,.
		NULL,							//		CBBPTN_portCOSIM_HsyncModel_On		,.
#endif									//	#endif.

										//		// ASIC22 出力ポート	ASIC22_REG_PIO_5R.
		NULL,							//		CBBPTN_portFIF_O_PD				,.
		NULL,							//		CBBPTN_portMOTER_EN				,.
		NULL,							//		CBBPTN_ppppD103PaperFeed		,.
		NULL,							//		CBBPTN_portCTL_BUSY				,.
		NULL,							//		CBBPTN_ppppKeyCounter			,.
		NULL,							//		CBBPTN_ppppTotalCounter			,.
		NULL,							//		CBBPTN_portTERM_SEL				,.
		NULL,							//		CBBPTN_portSPEEKER_REM			,.
		NULL,							//		CBBPTN_portFUN_SPEED			,.
		NULL,							//		CBBPTN_portFUN_REM_1			,.
		NULL,							//		CBBPTN_portENGINE_SET			,.
		NULL,							//		CBBPTN_portONLINE				,.
#if !(defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS ))
		NULL,							//		CBBPTN_portSEL_COLOR			,.
#endif // #if !(defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS )).
		NULL,							//		CBBPTN_portOut_IREQ_PIC_OUT		,.
		NULL,							//		CBBPTN_portWAKE_FAXMFPC			,.
		NULL,							//		CBBPTN_portOut_IREQ_IR			,.
		NULL,							//		CBBPTN_portA800_RESET			,.	// 2017/09/08 Sky戸田 S800 UART、GPIO対応.
		NULL,							//		CBBPTN_portOut_IREQ_IR			,.	// 2017/09/08 Sky戸田 S800 UART、GPIO対応.

#ifdef COSIM							//	#ifdef COSIM.
		NULL,							//		CBBPTN_portOut_COSIM_TOD_SIM_OUT,.
#endif									//	#endif.
										//		// ASIC22 RSTポート		ASIC22_REG_RST_0R.
		NULL,							//		CBBPTN_portRST0_RST0			,.
		NULL,							//		CBBPTN_portRST1_RST0			,.
		NULL,							//		CBBPTN_portRST2_RST0			,.
		NULL,							//		CBBPTN_portRST1_PIC				,.
		NULL,							//		CBBPTN_portRST_0R				,.

										//		// ASIC22 RSTポート		ASIC22_REG_RST_4R.
		NULL,							//		CBBPTN_portLCDC_LVDS_EN			,.
		NULL,							//		CBBPTN_portIRC_RESET			,.
		NULL,							//		CBBPTN_portDOTCLKEN				,.
		NULL,							//		CBBPTN_portCK_EN_C				,.
		NULL,							//		CBBPTN_portCK_EN_B				,.
		NULL,							//		CBBPTN_portCK_EN_A				,.
		NULL,							//		CBBPTN_portF_PD_T				,.
		NULL,							//		CBBPTN_portF_PD_R				,.
		NULL,							//		CBBPTN_portPCIRST_CPS			,.
		NULL,							//		CBBPTN_portRST2_PIC				,.
		NULL,							//		CBBPTN_portRST_4R				,.

										// 		// ASIC2014 入力ポート	ASIC2014_REG_PIO_0R.
		NULL,							//		CBBPTN_portLOCKN				,.
										// 		// ASIC2014 出力ポート	ASIC2014_REG_PIO_4R.2015/09/14 morikawa Asic2014 UARTハード不具合対応。
		NULL,							//		CBBPTN_portCTS_DISABLE				,.

#ifdef COSIM							//	#ifdef COSIM.
		&portOut_COSIM_IMAGE_CODE	,	//		CBBPTN_portOut_COSIM_IMAGE_CODE	,.
		&portOut_COSIM_IMAGE_SIZE	,	//		CBBPTN_portOut_COSIM_IMAGE_SIZE	,.
		&portOut_COSIM_IMAGE		,	//		CBBPTN_portOut_COSIM_IMAGE		,.
		&portOut_COSIM_SCAN_IN		,	//		CBBPTN_portOut_COSIM_SCAN_IN	,.
		&portOut_COSIM_FIERY_IN		,	//		CBBPTN_portOut_COSIM_FIERY_IN	,.
#endif									//	#endif.

		NULL,							// 		CBBPTN_portMC_P_ON				,.
		NULL,							// 		CBBPTN_portIR_P_OFF				,.
		NULL,							// 		CBBPTN_portIR_MOTION_EN			,.
		NULL,							// 		CBBPTN_portLR_INT				,.
		NULL,							// 		CBBPTN_portLR_SOUT				,.
		NULL,							// 		CBBPTN_portPAGE_ON				,.
		NULL,							// 		CBBPTN_portREV_PAGE_ON			,.
#if defined(DEF_S800) && !defined(SIMULATOR)
		NULL,							// 		CBBPTN_portCLOSE				,.
#else /* defined(DEF_S800) && !defined(SIMULATOR) */
		&portCLOSE,						// 		CBBPTN_portCLOSE				,.
#endif /* defined(DEF_S800) && !defined(SIMULATOR) */
		NULL,							// 		CBBPTN_portDSET					,.
		NULL,							// 		CBBPTN_portWAKE_WLAN			,.
		NULL,							// 		CBBPTN_portWAKE_UART			,.
		&portTHCV216_LOCKN,				//		CBBPTN_portTHCV216_LOCKN		,.
		&portAPC_STATE,					//		CBBPTN_portAPC_STATE			,.
		&portX_IREQ_P,					//		CBBPTN_portX_IREQ_P				,.
		&portDIV_SEL_GP6,				//		CBBPTN_portDIV_SEL_GP6			,.
		&portXON_GP6,					//		CBBPTN_portXON_GP6				,.
										//		// ASIC22 ダミーポート.
		NULL,							//		CBBPTN_portINPUT_SEL			,.
		NULL,							//		CBBPTN_portPIC5600Type			,.
		NULL,							//		CBBPTN_portIO_IORST5			,.
		NULL,							//		CBBPTN_portIO_IORST4			,.
		NULL,							//		CBBPTN_portIOLCDC_RST			,.
		NULL,							//		CBBPTN_portLCD_SUSPEND			,.
		NULL,							//		CBBPTN_portPLL_RST_CPS			,.
		NULL,							//		CBBPTN_portIORESET1				,.
		NULL,							//		CBBPTN_portIORESET3				,.
		NULL,							//		CBBPTN_portIORESET2				,.
		NULL,							//		CBBPTN_portPOWER_REM			,.
		NULL,							//		CBBPTN_portIR_OEN				,.
		NULL,							//		CBBPTN_portPOWER_OFF0			,.
		NULL,							//		CBBPTN_portPOWER_OFF1			,.
		NULL,							//		CBBPTN_portDB7654				,.
		NULL,							//		CBBPTN_portKEY_SELECT			,.

		&portPrinterIsPowerOn		,	//		CBBPTN_portPrinterIsPowerOn	 	,.
		&portIORST1					,	//		CBBPTN_portIORST1             	,.
		&portIORST2					,	//		CBBPTN_portIORST2             	,.
		&portFILE_DIMM_INIT_1		,	//		CBBPTN_portFILE_DIMM_INIT_1	 	,.
		&portS_O_PD					,	//		CBBPTN_portS_O_PD		     	,.
		&portFILE_DIMM_INIT_2		,	//		CBBPTN_portFILE_DIMM_INIT_2	 	,.
#ifdef DEF_EDH_COLOR					//	#ifdef DEF_EDH_COLOR.
		&portAE_TG					,	//		CBBPTN_portAE_TG				,.
		&portFB_AE3					,	//		CBBPTN_portFB_AE3				,.
		&portFB_AE2					,	//		CBBPTN_portFB_AE2				,.
		&portFB_AE1					,	//		CBBPTN_portFB_AE1				,.
		&portFB_AE0					,	//		CBBPTN_portFB_AE0				,.
#endif									//	#endif.
		&portServiceChk				,	//		CBBPTN_portServiceChk   		,.
		&portTroubleReset			,	//		CBBPTN_portTroubleReset 		,.
		&portMemoryClear			,	//		CBBPTN_portMemoryClear  		,.
		&portInitial				,	//		CBBPTN_portInitial      		,.
		&portTotalClear				,	//		CBBPTN_portTotalClear   		,.
		&portOut_DEBUG_PIC_1		,	//		CBBPTN_portOut_DEBUG_PIC_1		,.
		&portOut_DEBUG_PIC_2		,	//		CBBPTN_portOut_DEBUG_PIC_2		,.
		&portOut_DEBUG_PIC_3		,	//		CBBPTN_portOut_DEBUG_PIC_3		,.
		&portOut_DEBUG_PIC_4		,	//		CBBPTN_portOut_DEBUG_PIC_4		,.
		&portSEL_IR					,	//		CBBPTN_portSEL_IR             	,.
		&portSEL_NT					,	//		CBBPTN_portSEL_NT             	,.
		&portP_VEN					,	//		CBBPTN_portP_VEN              	,.
		&portCTS_TERM				,	//		CBBPTN_portCTS_TERM         	,.
		&portCARDIN1				,	//		CBBPTN_portCARDIN1          	,.
		&portFUN_LOCK				,	//		CBBPTN_portFUN_LOCK				,.
		&portMSC_DTR				,	//		CBBPTN_portMSC_DTR          	,.
		&portSSC_DSR				,	//		CBBPTN_portSSC_DSR          	,.
		&portNTF_CNCT				,	//		CBBPTN_portNTF_CNCT         	,.
		&portOut_CLK_INITIAL		,	//		CBBPTN_portOut_CLK_INITIAL    	,.
		&portRTS					,	//		CBBPTN_portRTS		        	,.
		&portSSC_DTR				,	//		CBBPTN_portSSC_DTR          	,.
		&portIMG_THROUGH			,	//		CBBPTN_portIMG_THROUGH	        ,.
		&portC_O_PD2				,	//		CBBPTN_portC_O_PD2		        ,.
		&portC_O_PD					,	//		CBBPTN_portC_O_PD		        ,.

#ifdef SIMULATOR						//	#ifdef SIMULATOR	// SIMWINで有効.
		&portVendorCopyEnable		,	//		CBBPTN_portVendorCopyEnable 	,.
		&portVendorPaperSize		,	//		CBBPTN_portVendorPaperSize 		,.
		&portVendorCopying			,	//		CBBPTN_portVendorCopying   		,.
		&ppppVendorPaperFeed		,	//		CBBPTN_ppppVendorPaperFeed 		,.
		&ppppVendorPaperEject		,	//		CBBPTN_ppppVendorPaperEject		,.
#else									//	#else.
#ifndef COSIM							//	#ifndef COSIM.
		&ppppVendorPaperFeed		,	//		CBBPTN_ppppVendorPaperFeed 		,.
		&ppppVendorPaperEject		,	//		CBBPTN_ppppVendorPaperEject		,.
#endif									//	#endif.
#endif									//	#endif	// #ifdef SIMULATOR	// SIMWINで有効.
	};

    THRL_ThAssert( (Bool)( vList[ tNum ] != NULL ), "CB_BitPort<uint32_t> tNum: %"PRId32"", tNum, 0, 0, 0 , CFA14_DetailCode0x086F );

	return *vList[tNum];
}

//	12/11/09 戸田	CPU選定評価基板対応　ushort用からulong用へ移植.
/*	2014/09/24 sky_tksk Fiery省電力対応 ポート制御対象変更 ASIC777⇒CPU GPIO */
//	15/01/08 Sky	S72基板対応 IO変更対応.
//	17/09/08 Sky戸田 S800 UART、GPIO対応.
//  19/09/04 Sky	OP_BTS-12689 SEL_COLOR 3bit更新対応.
template <> CB_BitPort<ushort>& CB_BitPort<ushort>::newInstance(CB_BitPortTypeNum type)
{
	int32_t tNum = (int32_t)type;

	static	CB_BitPort<ushort>	portDIPSW_0					( CBBPTN_portDIPSW_0			 );		// DIPSW0			評価用.
	static	CB_BitPort<ushort>	portDIPSW_1					( CBBPTN_portDIPSW_1			 );		// DIPSW1			評価用.
	static	CB_BitPort<ushort>	portDIPSW_2					( CBBPTN_portDIPSW_2			 );		// DIPSW2			評価用.
	static	CB_BitPort<ushort>	portDIPSW_3					( CBBPTN_portDIPSW_3			 );		// DIPSW3			評価用.
	static	CB_BitPort<ushort>	portUtilityKey				( CBBPTN_portUtilityKey			 );		// UTILITY_KEY		ユーティリティキー押下信号.
	static	CB_BitPort<ushort>	portIRC_CNCT				( CBBPTN_portIRC_CNCT			 );		// IRC_CNCT			IR通信接続確認.
	static	CB_BitPort<ushort>	portTERMINAL_SEL			( CBBPTN_portTERMINAL_SEL		 );		// TERMINAL_SEL		ターミナル選択.
	static	CB_BitPort<ushort>	portFUN_LOCK_1				( CBBPTN_portFUN_LOCK_1			 );		// FAN1_LOCK		冷却ファンロック検出(エンジン)←機内FAN.
	static	CB_BitPort<ushort>	portCPRDY					( CBBPTN_portCPRDY				 );		// CPRDY			外部コントローラ(Fiery)用 Ready信号.
	static	CB_BitPort<ushort>	portIMGRDY					( CBBPTN_portIMGRDY				 );		// IMGRDY			画像処理系リセット解除完了.
	static	CB_BitPort<ushort>	portWAKE_FIERY				( CBBPTN_portWAKE_FIERY			 );		// CP_WAKEUP		FieryからのWAKEUP信号.
	static	CB_BitPort<ushort>	portSUB_SW_ON				( CBBPTN_portSUB_SW_ON			 );		// SUBSW_ON			副電源SWオン信号.
	static	CB_BitPort<ushort>	portREV_BOARD_ON			( CBBPTN_portREV_BOARD_ON		 );		// REV_BOARD_ON		同時両面基板装着検出(L: 装着)(CPS用).

	static	CB_BitPort<ushort>	portPOWER_MONITOR			( CBBPTN_portPOWER_MONITOR		 );		// POWER_MONITOR	電源シャットダウン信号.
	static	CB_BitPort<ushort>	portENG_BUSY				( CBBPTN_portENG_BUSY			 );		// ENG_BUSY			通信BUSY信号（メカコン）.
	static	CB_BitPort<ushort>	portD102CopyEnable			( CBBPTN_portD102CopyEnable		 );		// D103_SET			D103の装着検出.
	static	CB_BitPort<ushort>	portKeyCntIsSet				( CBBPTN_portKeyCntIsSet		 );		// SET_KEY_CNT		キーカウンタの装着検出.
	static	CB_BitPort<ushort>	portTotalCntIsSet			( CBBPTN_portTotalCntIsSet		 );		// SET_TOTAL_CNT	トータルカウンタの装着検出.
	static	CB_BitPort<ushort>	portSEC_SET2				( CBBPTN_portSEC_SET2			 );		// SEC_SET2			地紋セキュリティ基板装着(同時両面用）.
	static	CB_BitPort<ushort>	portSEC_SET1				( CBBPTN_portSEC_SET1			 );		// SEC_SET1			地紋セキュリティ基板装着.
	static	CB_BitPort<ushort>	portPG_SEC					( CBBPTN_portPG_SEC				 );		// PG_SEC			地紋基板のPowerGood.	// 2017/09/08 Sky戸田 S800 UART、GPIO対応.
	static	CB_BitPort<ushort>	portFIERY_CNCT				( CBBPTN_portFIERY_CNCT			 );		// FIERY_CNCT		Fiery基板装着.
	static	CB_BitPort<ushort>	portA800_PG					( CBBPTN_portA800_PG			 );		// A800_PG			A800基板のPGOOD.		// 2017/09/08 Sky戸田 S800 UART、GPIO対応.
	static	CB_BitPort<ushort>	portACC_DETECT				( CBBPTN_portACC_DETECT			 );		// ACC_DETECT		A800基板の接続確認.		// 2017/09/08 Sky戸田 S800 UART、GPIO対応.
	static	CB_BitPort<ushort>	portACC_DETECT2				( CBBPTN_portACC_DETECT2		 );		// ACC_DETECT2		A800基板、2枚目.		// 2017/09/08 Sky戸田 S800 UART、GPIO対応.
	static	CB_BitPort<ushort>	portMC_PWR_EN				( CBBPTN_portMC_PWR_EN			 );		// MC_PWR_EN		MC電源　投入信号.		// 2017/09/08 Sky戸田 S800 UART、GPIO対応.

	static	CB_BitPort<ushort>	portLOCKN					( CBBPTN_portLOCKN				 );		// LOCKN			ロックN信号.
	static	CB_BitPort<ushort>	portCTS_DISABLE				( CBBPTN_portCTS_DISABLE		 );		// CTS_DISABLE		2015/09/14 morikawa Asic2014 UARTハード不具合対応。

#ifdef COSIM							//	#ifdef COSIM.
	static	CB_BitPort<ushort>	portCOSIM_ASIC1_SDRAM1_DUMP	( CBBPTN_portCOSIM_ASIC1_SDRAM1_DUMP	 );		// COSIM	ASIC2枚目SDRAM1のダンプ	1:OFF, 0:ON.
	static	CB_BitPort<ushort>	portCOSIM_ASIC1_SDRAM0_DUMP	( CBBPTN_portCOSIM_ASIC1_SDRAM0_DUMP	 );		// COSIM	ASIC2枚目SDRAM0のダンプ	1:OFF, 0:ON.
	static	CB_BitPort<ushort>	portCOSIM_ASIC0_SDRAM1_DUMP	( CBBPTN_portCOSIM_ASIC0_SDRAM1_DUMP	 );		// COSIM	ASIC1枚目SDRAM1のダンプ	1:OFF, 0:ON.
	static	CB_BitPort<ushort>	portCOSIM_ASIC0_SDRAM0_DUMP	( CBBPTN_portCOSIM_ASIC0_SDRAM0_DUMP	 );		// COSIM	ASIC1枚目SDRAM0のダンプ	1:OFF, 0:ON.
	static	CB_BitPort<ushort>	portCOSIM_DummyPicOut		( CBBPTN_portCOSIM_DummyPicOut			 );		// COSIM	PICのダミーモデルの起動	1:OFF, 0:ON.
	static	CB_BitPort<ushort>	portCOSIM_PEX_MODE_1		( CBBPTN_portCOSIM_PEX_MODE_1			 );		// COSIM	PEX	MODE	変更用	ASIC	2枚目.
	static	CB_BitPort<ushort>	portCOSIM_PEX_MODE_0		( CBBPTN_portCOSIM_PEX_MODE_0			 );		// COSIM	PEX	MODE	変更用	ASIC	1枚目.
	static	CB_BitPort<ushort>	portCOSIM_HsyncModel_Width	( CBBPTN_portCOSIM_HsyncModel_Width		 );		// COSIM	HSYNCモデルのHSYNC幅の設定	0:640用,1:72用.
	static	CB_BitPort<ushort>	portCOSIM_HsyncModel_On		( CBBPTN_portCOSIM_HsyncModel_On		 );		// COSIM	HSYNCモデルの起動、接続	1:	起動接続.
#endif									//	#endif.
#ifdef COSIM							//	#ifdef COSIM.
	static	CB_BitPort<ushort>	portOut_COSIM_TOD_SIM_OUT	( CBBPTN_portOut_COSIM_TOD_SIM_OUT	 );	// TOD割り込み発生のための擬似出力　	0:Lo	アクティブ	, 1:Hi	非アクティブ.
#endif									//	#endif.
	static	CB_BitPort<ushort>	portINPUT_SEL				( CBBPTN_portINPUT_SEL		,	CBPTN_ptASIC22_DUMMY_PIO,	0,	1,	true  );
	static	CB_BitPort<ushort>	portPIC5600Type				( CBBPTN_portPIC5600Type	,	CBPTN_ptASIC22_DUMMY_PIO,	1,	1,	true  );
	static	CB_BitPort<ushort>	portIO_IORST5				( CBBPTN_portIO_IORST5		,	CBPTN_ptASIC22_DUMMY_PIO,	2,	1,	true  );
	static	CB_BitPort<ushort>	portIO_IORST4				( CBBPTN_portIO_IORST4		,	CBPTN_ptASIC22_DUMMY_PIO,	3,	1,	true  );
	static	CB_BitPort<ushort>	portIOLCDC_RST				( CBBPTN_portIOLCDC_RST		,	CBPTN_ptASIC22_DUMMY_PIO,	5,	1,	true  );
	static	CB_BitPort<ushort>	portLCD_SUSPEND				( CBBPTN_portLCD_SUSPEND	,	CBPTN_ptASIC22_DUMMY_PIO,	6,	1,	true  );
	static	CB_BitPort<ushort>	portPLL_RST_CPS				( CBBPTN_portPLL_RST_CPS	,	CBPTN_ptASIC22_DUMMY_PIO,	7,	1,	true  );
	static	CB_BitPort<ushort>	portIORESET1				( CBBPTN_portIORESET1		,	CBPTN_ptASIC22_DUMMY_PIO,	8,	1,	true  );
	static	CB_BitPort<ushort>	portIORESET3				( CBBPTN_portIORESET3		,	CBPTN_ptASIC22_DUMMY_PIO,	9,	1,	true  );
	static	CB_BitPort<ushort>	portIORESET2				( CBBPTN_portIORESET2		,	CBPTN_ptASIC22_DUMMY_PIO,	10,	1,	true  );
	static	CB_BitPort<ushort>	portPOWER_REM				( CBBPTN_portPOWER_REM		,	CBPTN_ptASIC22_DUMMY_PIO,	11,	1,	true  );
	static	CB_BitPort<ushort>	portIR_OEN					( CBBPTN_portIR_OEN			,	CBPTN_ptASIC22_DUMMY_PIO,	12,	1,	true  );		// 2007.12.25	add	tezuka.
	static	CB_BitPort<ushort>	portPOWER_OFF0				( CBBPTN_portPOWER_OFF0		,	CBPTN_ptASIC22_DUMMY_PIO,	13,	1,	true  );		// 2007.12.25	add	tezuka.
	static	CB_BitPort<ushort>	portPOWER_OFF1				( CBBPTN_portPOWER_OFF1		,	CBPTN_ptASIC22_DUMMY_PIO,	14,	1,	true  );		// 2007.12.25	add	tezuka.
	static	CB_BitPort<ushort>	portDB7654					( CBBPTN_portDB7654			,	CBPTN_ptASIC22_DUMMY_PIO2,	0,	4,	true  );		// 2007.12.25	add	tezuka.
	static	CB_BitPort<ushort>	portKEY_SELECT				( CBBPTN_portKEY_SELECT		,	CBPTN_ptASIC22_DUMMY_PIO2,	4,	1,	true  );		// 2008.7.2	add	sunami.

	static CB_BitPort<ushort>* vList[] = {
		NULL,							//		CBBPTN_NON						,.
										//		// ASIC22 入力ポート	ASIC22_REG_PIO_0R.
		&portDIPSW_0				,	//		CBBPTN_portDIPSW_0				,.
		&portDIPSW_1				,	//		CBBPTN_portDIPSW_1				,.
		&portDIPSW_2				,	//		CBBPTN_portDIPSW_2				,.
		&portDIPSW_3				,	//		CBBPTN_portDIPSW_3				,.
		&portUtilityKey				,	//		CBBPTN_portUtilityKey			,.
		&portIRC_CNCT				,	//		CBBPTN_portIRC_CNCT				,.
		&portTERMINAL_SEL			,	//		CBBPTN_portTERMINAL_SEL			,.
		&portFUN_LOCK_1				,	//		CBBPTN_portFUN_LOCK_1			,.
		&portCPRDY					,	//		CBBPTN_portCPRDY				,.
		&portIMGRDY					,	//		CBBPTN_portIMGRDY				,.
		&portWAKE_FIERY				,	//		CBBPTN_portWAKE_FIERY			,. /* 2014/09/24 takas Fiery省電力対応 ポート制御対象変更 ASIC777⇒CPU GPIO */
		&portSUB_SW_ON				,	//		CBBPTN_portSUB_SW_ON			,.
		&portREV_BOARD_ON			,	//		CBBPTN_portREV_BOARD_ON			,.

										//		// ASIC22 入力ポート	ASIC22_REG_PIO_1R.
		&portPOWER_MONITOR			,	//		CBBPTN_portPOWER_MONITOR		,.
		&portENG_BUSY				,	//		CBBPTN_portENG_BUSY				,.
		&portD102CopyEnable			,	//		CBBPTN_portD102CopyEnable		,.
		&portKeyCntIsSet			,	//		CBBPTN_portKeyCntIsSet			,.
		&portTotalCntIsSet			,	//		CBBPTN_portTotalCntIsSet		,.
		&portSEC_SET2				,	//		CBBPTN_portSEC_SET2				,.
		&portSEC_SET1				,	//		CBBPTN_portSEC_SET1				,.
		&portPG_SEC					,	//		CBBPTN_portPG_SEC				,.	// 2017/09/08 Sky戸田 S800 UART、GPIO対応.
		&portFIERY_CNCT				,	//		CBBPTN_portFIERY_CNCT			,.
		&portA800_PG				,	//		CBBPTN_portA800_PG				,.	// 2017/09/08 Sky戸田 S800 UART、GPIO対応.
		&portACC_DETECT				,	//		CBBPTN_portACC_DETECT			,.	// 2017/09/08 Sky戸田 S800 UART、GPIO対応.
		&portACC_DETECT2			,	//		CBBPTN_portACC_DETECT2			,.	// 2017/09/08 Sky戸田 S800 UART、GPIO対応.
		&portMC_PWR_EN				,	//		CBBPTN_portMC_PWR_EN			,.	// 2017/09/08 Sky戸田 S800 UART、GPIO対応.

										//		// ASIC22 出力ポート	ASIC22_REG_PIO_4R.
		NULL,	//		CBBPTN_portLED_0				,.
		NULL,	//		CBBPTN_portLED_1_N				,.
		NULL,	//		CBBPTN_portLED_2				,.
		NULL,	//		CBBPTN_portLED_3				,.
		NULL,	//		CBBPTN_portLED_4_N				,.
		NULL,	//		CBBPTN_portLED_5				,.
#ifdef DEF_MinervaS_PF
		NULL,							//		CBBPTN_portPMS_PLL2_ENB			,.
#endif // #ifdef DEF_MinervaS_PF.
		NULL,							//		CBBPTN_portAC_POWER_DOWN		,.
#if defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS )
		NULL,							//		CBBPTN_portSEL_COLOR0			,.
//		NULL,							//		CBBPTN_portSEL_COLOR1			,.
//		NULL,							//		CBBPTN_portSEL_COLOR2			,.
#else // #ifdef #if defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS ).
		NULL,							//		CBBPTN_portPOWER_OFF_PNL		,.
		NULL,							//		CBBPTN_portPOWER_OFF			,.
#endif // #ifdef #if defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS ).

#ifdef COSIM								//	#ifdef COSIM.
		&portCOSIM_ASIC1_SDRAM1_DUMP	,	//		CBBPTN_portCOSIM_ASIC1_SDRAM1_DUMP	,.
		&portCOSIM_ASIC1_SDRAM0_DUMP	,	//		CBBPTN_portCOSIM_ASIC1_SDRAM0_DUMP	,.
		&portCOSIM_ASIC0_SDRAM1_DUMP	,	//		CBBPTN_portCOSIM_ASIC0_SDRAM1_DUMP	,.
		&portCOSIM_ASIC0_SDRAM0_DUMP	,	//		CBBPTN_portCOSIM_ASIC0_SDRAM0_DUMP	,.
		&portCOSIM_DummyPicOut			,	//		CBBPTN_portCOSIM_DummyPicOut		,.
		&portCOSIM_PEX_MODE_1			,	//		CBBPTN_portCOSIM_PEX_MODE_1			,.
		&portCOSIM_PEX_MODE_0			,	//		CBBPTN_portCOSIM_PEX_MODE_0			,.
		&portCOSIM_HsyncModel_Width		,	//		CBBPTN_portCOSIM_HsyncModel_Width	,.
		&portCOSIM_HsyncModel_On		,	//		CBBPTN_portCOSIM_HsyncModel_On		,.
#endif										//	#endif.

										//		// ASIC22 出力ポート	ASIC22_REG_PIO_5R.
		NULL,	//		CBBPTN_portFIF_O_PD				,.
		NULL,	//		CBBPTN_portMOTER_EN				,.
		NULL,	//		CBBPTN_ppppD103PaperFeed		,.
		NULL,	//		CBBPTN_portCTL_BUSY				,.
		NULL,	//		CBBPTN_ppppKeyCounter			,.
		NULL,	//		CBBPTN_ppppTotalCounter			,.
		NULL,	//		CBBPTN_portTERM_SEL				,.
		NULL,	//		CBBPTN_portSPEEKER_REM			,.
		NULL,	//		CBBPTN_portFUN_SPEED			,.
		NULL,	//		CBBPTN_portFUN_REM_1			,.
		NULL,	//		CBBPTN_portENGINE_SET			,.
		NULL,	//		CBBPTN_portONLINE				,.
#if !(defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS ))
		NULL,							//		CBBPTN_portSEL_COLOR			,.
#endif // #if !(defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS )).
		NULL,	//		CBBPTN_portOut_IREQ_PIC_OUT		,.
		NULL,							//		CBBPTN_portWAKE_FAXMFPC			,.
		NULL,	//		CBBPTN_portOut_IREQ_IR			,.
		NULL,	//		CBBPTN_portA800_RESET			,.	// 2017/09/08 Sky戸田 S800 UART、GPIO対応.
		NULL,	//		CBBPTN_portOut_IREQ_IR			,.	// 2017/09/08 Sky戸田 S800 UART、GPIO対応.

#ifdef COSIM							//	#ifdef COSIM.
		&portOut_COSIM_TOD_SIM_OUT	,	//		CBBPTN_portOut_COSIM_TOD_SIM_OUT,.
#endif									//	#endif.
										//		// ASIC22 RSTポート		ASIC22_REG_RST_0R.
		NULL,	//		CBBPTN_portRST0_RST0			,.
		NULL,	//		CBBPTN_portRST1_RST0			,.
		NULL,	//		CBBPTN_portRST2_RST0			,.
		NULL,							//		CBBPTN_portRST1_PIC				,.
		NULL,							//		CBBPTN_portRST_0R				,.

										//		// ASIC22 RSTポート		ASIC22_REG_RST_4R.
		NULL,	//		CBBPTN_portLCDC_LVDS_EN			,.
		NULL,	//		CBBPTN_portIRC_RESET			,.
		NULL,	//		CBBPTN_portDOTCLKEN				,.
		NULL,	//		CBBPTN_portCK_EN_C				,.
		NULL,	//		CBBPTN_portCK_EN_B				,.
		NULL,	//		CBBPTN_portCK_EN_A				,.
		NULL,	//		CBBPTN_portF_PD_T				,.
		NULL,	//		CBBPTN_portF_PD_R				,.
		NULL,							//		CBBPTN_portPCIRST_CPS			,.
		NULL,							//		CBBPTN_portRST2_PIC				,.
		NULL,	//		CBBPTN_portRST_4R				,.
										// 		// ASIC2014 入力ポート	ASIC2014_REG_PIO_0R.
		&portLOCKN,						//		CBBPTN_portLOCKN				,.
		&portCTS_DISABLE,				//		CBBPTN_portCTS_DISABLE			,.2015/09/14 morikawa Asic2014 UARTハード不具合対応。

#ifdef COSIM							//	#ifdef COSIM.
		NULL,							//		CBBPTN_portOut_COSIM_IMAGE_CODE	,.
		NULL,							//		CBBPTN_portOut_COSIM_IMAGE_SIZE	,.
		NULL,							//		CBBPTN_portOut_COSIM_IMAGE		,.
		NULL,							//		CBBPTN_portOut_COSIM_SCAN_IN	,.
		NULL,							//		CBBPTN_portOut_COSIM_FIERY_IN	,.
#endif									//	#endif.

		NULL,							// 		CBBPTN_portMC_P_ON				,.
		NULL,							// 		CBBPTN_portIR_P_OFF				,.
		NULL,							// 		CBBPTN_portIR_MOTION_EN			,.
		NULL,							// 		CBBPTN_portLR_INT				,.
		NULL,							// 		CBBPTN_portLR_SOUT				,.
		NULL,							// 		CBBPTN_portPAGE_ON				,.
		NULL,							// 		CBBPTN_portREV_PAGE_ON			,.
		NULL,							// 		CBBPTN_portCLOSE				,.
		NULL,							// 		CBBPTN_portDSET					,.
		NULL,							// 		CBBPTN_portWAKE_WLAN			,.
		NULL,							// 		CBBPTN_portWAKE_UART			,.
		NULL						,	//		CBBPTN_portTHCV216_LOCKN		,.
		NULL						,	//		CBBPTN_portAPC_STATE			,.
		NULL						,	//		CBBPTN_portX_IREQ_P				,.
		NULL						,	//		CBBPTN_portDIV_SEL_GP6			,.
		NULL						,	//		CBBPTN_portXON_GP6				,
										//		// ASIC22 ダミーポート.
		&portINPUT_SEL				,	//		CBBPTN_portINPUT_SEL			,.
		&portPIC5600Type			,	//		CBBPTN_portPIC5600Type			,.
		&portIO_IORST5				,	//		CBBPTN_portIO_IORST5			,.
		&portIO_IORST4				,	//		CBBPTN_portIO_IORST4			,.
		&portIOLCDC_RST				,	//		CBBPTN_portIOLCDC_RST			,.
		&portLCD_SUSPEND			,	//		CBBPTN_portLCD_SUSPEND			,.
		&portPLL_RST_CPS			,	//		CBBPTN_portPLL_RST_CPS			,.
		&portIORESET1				,	//		CBBPTN_portIORESET1				,.
		&portIORESET3				,	//		CBBPTN_portIORESET3				,.
		&portIORESET2				,	//		CBBPTN_portIORESET2				,.
		&portPOWER_REM				,	//		CBBPTN_portPOWER_REM			,.
		&portIR_OEN					,	//		CBBPTN_portIR_OEN				,.
		&portPOWER_OFF0				,	//		CBBPTN_portPOWER_OFF0			,.
		&portPOWER_OFF1				,	//		CBBPTN_portPOWER_OFF1			,.
		&portDB7654					,	//		CBBPTN_portDB7654				,.
		&portKEY_SELECT				,	//		CBBPTN_portKEY_SELECT			,.

		NULL,							//		CBBPTN_portPrinterIsPowerOn	 	,.
		NULL,							//		CBBPTN_portIORST1             	,.
		NULL,							//		CBBPTN_portIORST2             	,.
		NULL,							//		CBBPTN_portFILE_DIMM_INIT_1	 	,.
		NULL,							//		CBBPTN_portS_O_PD		     	,.
		NULL,							//		CBBPTN_portFILE_DIMM_INIT_2	 	,.
#ifdef DEF_EDH_COLOR					//	#ifdef DEF_EDH_COLOR.
		NULL,							//		CBBPTN_portAE_TG				,.
		NULL,							//		CBBPTN_portFB_AE3				,.
		NULL,							//		CBBPTN_portFB_AE2				,.
		NULL,							//		CBBPTN_portFB_AE1				,.
		NULL,							//		CBBPTN_portFB_AE0				,.
#endif									//	#endif.
		NULL,							//		CBBPTN_portServiceChk   		,.
		NULL,							//		CBBPTN_portTroubleReset 		,.
		NULL,							//		CBBPTN_portMemoryClear  		,.
		NULL,							//		CBBPTN_portInitial      		,.
		NULL,							//		CBBPTN_portTotalClear   		,.
		NULL,							//		CBBPTN_portOut_DEBUG_PIC_1		,.
		NULL,							//		CBBPTN_portOut_DEBUG_PIC_2		,.
		NULL,							//		CBBPTN_portOut_DEBUG_PIC_3		,.
		NULL,							//		CBBPTN_portOut_DEBUG_PIC_4		,.
		NULL,							//		CBBPTN_portSEL_IR             	,.
		NULL,							//		CBBPTN_portSEL_NT             	,.
		NULL,							//		CBBPTN_portP_VEN              	,.
		NULL,							//		CBBPTN_portCTS_TERM         	,.
		NULL,							//		CBBPTN_portCARDIN1          	,.
		NULL,							//		CBBPTN_portFUN_LOCK				,.
		NULL,							//		CBBPTN_portMSC_DTR          	,.
		NULL,							//		CBBPTN_portSSC_DSR          	,.
		NULL,							//		CBBPTN_portNTF_CNCT         	,.
		NULL,							//		CBBPTN_portOut_CLK_INITIAL    	,.
		NULL,							//		CBBPTN_portRTS		        	,.
		NULL,							//		CBBPTN_portSSC_DTR          	,.
		NULL,							//		CBBPTN_portIMG_THROUGH	        ,.
		NULL,							//		CBBPTN_portC_O_PD2		        ,.
		NULL,							//		CBBPTN_portC_O_PD		        ,.

#ifdef SIMULATOR						//	#ifdef SIMULATOR	// SIMWINで有効.
		NULL,							//		CBBPTN_portVendorCopyEnable 	,.
		NULL,							//		CBBPTN_portVendorPaperSize 		,.
		NULL,							//		CBBPTN_portVendorCopying   		,.
		NULL,							//		CBBPTN_ppppVendorPaperFeed 		,.
		NULL,							//		CBBPTN_ppppVendorPaperEject		,.
#else									//	#else.
#ifndef COSIM							//	#ifndef COSIM.
		NULL,							//		CBBPTN_ppppVendorPaperFeed 		,.
		NULL,							//		CBBPTN_ppppVendorPaperEject		,.
#endif									//	#endif.
#endif									//	#endif	// #ifdef SIMULATOR	// SIMWINで有効.
	};

	THRL_ThAssert( (Bool)( vList[ tNum ] != NULL ), "CB_BitPort<ushort> tNum: %"PRId32"", tNum, 0, 0, 0 , CFA14_DetailCode0x0870 );

	return *vList[tNum];
}

//	12/11/09 戸田	CPU選定評価基板対応　ushort用からulong用へ変更.
/*	2014/09/24 sky_tksk Fiery省電力対応 ポート制御対象変更 ASIC777⇒CPU GPIO */
//	15/01/08 Sky	S72基板対応 IO変更対応.
//	17/09/08 Sky戸田 S800 UART、GPIO対応.
//  19/09/04 Sky	OP_BTS-12689 SEL_COLOR 3bit更新対応.
template <> CB_BitPortN<THRD_Port_DataType>& CB_BitPortN<THRD_Port_DataType>::newInstance(CB_BitPortTypeNum type){
	int32_t tNum = (int32_t)type;

	static	CB_BitPortN<THRD_Port_DataType>	portLED_0					( CBBPTN_portLED_0				,	NULL );						// LED_OUT0			LED点灯信号.
	static	CB_BitPortN<THRD_Port_DataType>	portLED_1_N					( CBBPTN_portLED_1_N			,	NULL );						// LED_OUT1			LED点灯信号.
	static	CB_BitPortN<THRD_Port_DataType>	portLED_2					( CBBPTN_portLED_2				,	NULL );						// LED_OUT2			LED点灯信号.
	static	CB_BitPortN<THRD_Port_DataType>	portLED_3					( CBBPTN_portLED_3				,	NULL );						// LED_OUT3			LED点灯信号.
	static	CB_BitPortN<THRD_Port_DataType>	portLED_4_N					( CBBPTN_portLED_4_N			,	NULL );						// LED_OUT4			LED点灯信号.
	static	CB_BitPortN<THRD_Port_DataType>	portLED_5					( CBBPTN_portLED_5				,	NULL );						// LED_OUT5			LED点灯信号.
#ifdef DEF_MinervaS_PF
	static	CB_BitPortN<THRD_Port_DataType>	portPMS_PLL2_ENB			( CBBPTN_portPMS_PLL2_ENB		,	(char*)"PMS_PLL2_ENB" );	// PMS_PLL2_ENB		PLL2制御信号.
#endif // #ifdef DEF_MinervaS_PF.
	static	CB_BitPortN<THRD_Port_DataType> portAC_POWER_DOWN			( CBBPTN_portAC_POWER_DOWN		,	(char*)"ENG_PG" );			// ENG_PG			外部コント用PowerGood信号(L:MFPコント電源on).
#if defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS )
	static	CB_BitPortN<THRD_Port_DataType>	portSEL_COLOR0				( CBBPTN_portSEL_COLOR0			,	(char*)"SEL_COLOR0" );		// SEL_COLOR0		PICのクロック信号.
//	static	CB_BitPortN<THRD_Port_DataType>	portSEL_COLOR1				( CBBPTN_portSEL_COLOR1			,	(char*)"SEL_COLOR1" );		// SEL_COLOR1		PICのクロック信号.
//	static	CB_BitPortN<THRD_Port_DataType>	portSEL_COLOR2				( CBBPTN_portSEL_COLOR2			,	(char*)"SEL_COLOR2" );		// SEL_COLOR2		PICのクロック信号.
#else // #if defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS ).
	static	CB_BitPortN<THRD_Port_DataType> portPOWER_OFF_PNL			( CBBPTN_portPOWER_OFF_PNL		,	(char*)"POWER_OFF_PNL" );	// POWER_OFF_PNL	パネルのパワーOFF.
	static	CB_BitPortN<THRD_Port_DataType> portPOWER_OFF				( CBBPTN_portPOWER_OFF			,	(char*)"POWER_OFF" );		// POWER_OFF		評価基板上のCPLDへのPOWER_OFF信号.
#endif // #if defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS ).

	static	CB_BitPortN<THRD_Port_DataType> portFIF_O_PD				( CBBPTN_portFIF_O_PD			,	(char*)"ENG_RDY" );			// ENG_RDY			エンジン通信可(H:エンジン通信可).
	static	CB_BitPortN<THRD_Port_DataType> portMOTER_EN				( CBBPTN_portMOTER_EN			,	(char*)"MOTION_EN" );		// MOTION_EN		MSCからMCへ動作許可信号.
	static	CB_BitPortN<THRD_Port_DataType> ppppD103PaperFeed			( CBBPTN_ppppD103PaperFeed		,	(char*)"D103_FEED" );		// D103_FEED		D103のFEED信号.
	static	CB_BitPortN<THRD_Port_DataType> portCTL_BUSY				( CBBPTN_portCTL_BUSY			,	(char*)"CTLBUSY" );			// CTLBUSY			通信BUSY信号（メカコン）.
	static	CB_BitPortN<THRD_Port_DataType> ppppKeyCounter				( CBBPTN_ppppKeyCounter			,	(char*)"KEY_COUNT" );		// KEY_COUNT		キーカウンタのカウント信号.
	static	CB_BitPortN<THRD_Port_DataType> ppppTotalCounter			( CBBPTN_ppppTotalCounter		,	(char*)"TOTAL_COUNT" );		// TOTAL_COUNT		トータルカウンタのカウント信号.
	static	CB_BitPortN<THRD_Port_DataType> portTERM_SEL				( CBBPTN_portTERM_SEL			,	(char*)"TERM_SEL");			// TERM_SEL			ターミナル選択.
	static	CB_BitPortN<THRD_Port_DataType> portSPEEKER_REM				( CBBPTN_portSPEEKER_REM		,	(char*)"IO_SPEAKER_REM" );	// IO_SPEAKER_REM	スピーカ出力許可信号.
	static	CB_BitPortN<THRD_Port_DataType> portFUN_SPEED				( CBBPTN_portFUN_SPEED			,	(char*)"FUN_SPEED" );		// FUN_SPEED		FAN PWM制御.
	static	CB_BitPortN<THRD_Port_DataType> portFUN_REM_1				( CBBPTN_portFUN_REM_1			,	(char*)"FAN1_REM" );		// FAN1_REM			冷却ファン駆動信号.
	static	CB_BitPortN<THRD_Port_DataType> portENGINE_SET				( CBBPTN_portENGINE_SET			,	(char*)"ENG_STATUS1" );		// ENG_STATUS1		コント用EnginePowerStatus通知.
	static	CB_BitPortN<THRD_Port_DataType> portONLINE					( CBBPTN_portONLINE				,	(char*)"ENG_STATUS2" );		// ENG_STATUS2		Lコント用EnginePowerStatus通知.
#if !(defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS ))
	static	CB_BitPortN<THRD_Port_DataType>	portSEL_COLOR				( CBBPTN_portSEL_COLOR			,	(char*)"SEL_COLOR" );		// SEL_COLOR	    DEV0202クロック選択.
#endif // #if !(defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS )).
	static	CB_BitPortN<THRD_Port_DataType> portOut_IREQ_PIC_OUT		( CBBPTN_portOut_IREQ_PIC_OUT	,	(char*)"IO_IREQ_PIC" );		// IO_IREQ_PIC		PIC3400Great用IREQ信号.
	static	CB_BitPortN<THRD_Port_DataType> portWAKE_FAXMFPC			( CBBPTN_portWAKE_FAXMFPC		,	(char*)"/WAKE_FAXMFPC" );	// WAKE_FAXMFPC     FAX起動.
	static	CB_BitPortN<THRD_Port_DataType> portOut_IREQ_IR				( CBBPTN_portOut_IREQ_IR		,	(char*)"/SCAN_ON" );		// SCAN_ON			(EDH用)スキャン開始指示信号.
	static	CB_BitPortN<THRD_Port_DataType> portA800_RESET				( CBBPTN_portA800_RESET			,	(char*)"IO_IREQ_PIC" );		// A800_RESET		A800基板のリセット.	// 2017/09/08 Sky戸田 S800 UART、GPIO対応.
	static	CB_BitPortN<THRD_Port_DataType> portSEC_RESET				( CBBPTN_portSEC_RESET			,	(char*)"IO_IREQ_PIC" );		// SEC_RESET		地紋基板のリセット.	// 2017/09/08 Sky戸田 S800 UART、GPIO対応.


	static	CB_BitPortN<THRD_Port_DataType> portRST0_RST0				( CBBPTN_portRST0_RST0			,	(char*)"RST0_RST0" );		// RST0_RST0		DEV0103_RSTPLLJ(Fiery/DS基板).
	static	CB_BitPortN<THRD_Port_DataType> portRST1_RST0				( CBBPTN_portRST1_RST0			,	(char*)"RST1_RST0" );		// RST1_RST0		DEV0103_RSTPLLFORM0102(Fiery/DS基板).
	static	CB_BitPortN<THRD_Port_DataType> portRST2_RST0				( CBBPTN_portRST2_RST0			,	(char*)"RST2_RST0" );		// RST2_RST0		DEV0103_RSTXXX(Fiery/DS基板).
	static	CB_BitPortN<THRD_Port_DataType> portRST1_PIC				( CBBPTN_portRST1_PIC			,	(char*)"RST1_PIC" );			// RST1_PIC			PICのリセット.
	static	CB_BitPortN<THRD_Port_DataType> portRST_0R					( CBBPTN_portRST_0R				,	(char*)"RST_0R" );			// RST_0R			BootROM設定値を本体F/Wでリレーする時に16bitアクセスが必要.

	static	CB_BitPortN<THRD_Port_DataType> portLCDC_LVDS_EN			( CBBPTN_portLCDC_LVDS_EN		,	(char*)"LCDC_LVDS_EN" );	// LCDC_LVDS_EN		LCD用LVDS PD.
	static	CB_BitPortN<THRD_Port_DataType> portIRC_RESET				( CBBPTN_portIRC_RESET			,	(char*)"IRC_RESET" );		// IRC_RESET		IRCのリセット.
	static	CB_BitPortN<THRD_Port_DataType> portDOTCLKEN				( CBBPTN_portDOTCLKEN			,	(char*)"DOTCLKEN" );		// DOTCLKEN			DEV0201画素クロックCLK出力イネーブル信号.
	static	CB_BitPortN<THRD_Port_DataType> portCK_EN_C					( CBBPTN_portCK_EN_C			,	(char*)"CK_EN_C" );			// CK_EN_C			DEV0103(op.)のCLK_XXX起動.
	static	CB_BitPortN<THRD_Port_DataType> portCK_EN_B					( CBBPTN_portCK_EN_B			,	(char*)"CK_EN_B" );			// CK_EN_B			PCIEXPRESS_CLK起動.
	static	CB_BitPortN<THRD_Port_DataType> portCK_EN_A					( CBBPTN_portCK_EN_A			,	(char*)"CK_EN_A" );			// CK_EN_A			PCI_33CLK起動.
	static	CB_BitPortN<THRD_Port_DataType> portF_PD_T					( CBBPTN_portF_PD_T				,	(char*)"portF_PD_T" );		// portF_PD_T		外部ｺﾝﾄ出力LVDS_PD制御(H:出力可).
	static	CB_BitPortN<THRD_Port_DataType> portF_PD_R					( CBBPTN_portF_PD_R				,	(char*)"portF_PD_R" );		// portF_PD_R		外部ｺﾝﾄ入力LVDS_PD制御(H:入力可).
	static	CB_BitPortN<THRD_Port_DataType> portPCIRST_CPS				( CBBPTN_portPCIRST_CPS			,	(char*)"PCIRST_CPS" );		// PCIRST_CPS		CPS2007のReset(XRST_PCI)
	static	CB_BitPortN<THRD_Port_DataType> portRST2_PIC				( CBBPTN_portRST2_PIC			,	(char*)"RST2_PIC" );			// RST2_PIC			PICのリセット予備.
	static	CB_BitPortN<THRD_Port_DataType> portRST_4R					( CBBPTN_portRST_4R				,	(char*)"RST_4R" );			// RST_4R			BootROM設定値を本体F/Wでリレーする時に16bitアクセスが必要.
#if defined(DEF_S800) 
#if defined(SIMULATOR)	/* シミュレータで有効. */
	static	CB_BitPortN<THRD_Port_DataType> portCLOSE					( CBBPTN_portCLOSE				,	(char*)"CLOSE" );			// CLOSE			.	 
#endif /* defined(SIMULATOR)*/	/* シミュレータで有効. */
#else /* defined(DEF_S800) */
	static	CB_BitPortN<THRD_Port_DataType> portMC_P_ON					( CBBPTN_portMC_P_ON			,	(char*)"MC_P_ON" );			// MC_P_ON			. 
	static	CB_BitPortN<THRD_Port_DataType> portIR_P_OFF				( CBBPTN_portIR_P_OFF			,	(char*)"IR_P_OFF" );		// IR_P_OFF		 	.
	static	CB_BitPortN<THRD_Port_DataType> portIR_MOTION_EN			( CBBPTN_portIR_MOTION_EN		,	(char*)"IR_MOTION_EN" );	// IR_MOTION_EN	 	.
	static	CB_BitPortN<THRD_Port_DataType> portLR_INT					( CBBPTN_portLR_INT				,	(char*)"LR_INT" );			// LR_INT			.	 
	static	CB_BitPortN<THRD_Port_DataType> portLR_SOUT					( CBBPTN_portLR_SOUT			,	(char*)"LR_SOUT" );			// LR_SOUT		 	.
	static	CB_BitPortN<THRD_Port_DataType> portPAGE_ON					( CBBPTN_portPAGE_ON			,	(char*)"PAGE_ON" );			// PAGE_ON		 	.
	static	CB_BitPortN<THRD_Port_DataType> portREV_PAGE_ON				( CBBPTN_portREV_PAGE_ON		,	(char*)"REV_PAGE_ON" );		// REV_PAGE_ON	 	.
	static	CB_BitPortN<THRD_Port_DataType> portCLOSE					( CBBPTN_portCLOSE				,	(char*)"CLOSE" );			// CLOSE			.	 
	static	CB_BitPortN<THRD_Port_DataType> portDSET					( CBBPTN_portDSET				,	(char*)"DSET" );			// DSET			 	.
	static	CB_BitPortN<THRD_Port_DataType> portWAKE_WLAN				( CBBPTN_portWAKE_WLAN			,	(char*)"WAKE_WLAN" );		// WAKE_WLAN		.	 
	static	CB_BitPortN<THRD_Port_DataType> portWAKE_UART				( CBBPTN_portWAKE_UART			,	(char*)"WAKE_UART" );		// WAKE_UART		.	 
#endif /* defined(DEF_S800) */

	static CB_BitPortN<THRD_Port_DataType>* vList[] = {
		NULL,							//		CBBPTN_NON				,.

										//		// ASIC22 入力ポート	ASIC22_REG_PIO_0R.
		NULL,							//		CBBPTN_portDIPSW_0				,.
		NULL,							//		CBBPTN_portDIPSW_1				,.
		NULL,							//		CBBPTN_portDIPSW_2				,.
		NULL,							//		CBBPTN_portDIPSW_3				,.
		NULL,							//		CBBPTN_portUtilityKey			,.
		NULL,							//		CBBPTN_portIRC_CNCT				,.
		NULL,							//		CBBPTN_portTERMINAL_SEL			,.
		NULL,							//		CBBPTN_portFUN_LOCK_1			,.
		NULL,							//		CBBPTN_portCPRDY				,.
		NULL,							//		CBBPTN_portIMGRDY				,.
		NULL,							//		CBBPTN_portWAKE_FIERY			,. /* 2014/09/24 takas Fiery省電力対応 ポート制御対象変更 ASIC777⇒CPU GPIO */
		NULL,							//		CBBPTN_portSUB_SW_ON			,.
		NULL,							//		CBBPTN_portREV_BOARD_ON			,.

										//		// ASIC22 入力ポート	ASIC22_REG_PIO_1R.
		NULL,							//		CBBPTN_portPOWER_MONITOR		,.
		NULL,							//		CBBPTN_portENG_BUSY				,.
		NULL,							//		CBBPTN_portD102CopyEnable		,.
		NULL,							//		CBBPTN_portKeyCntIsSet			,.
		NULL,							//		CBBPTN_portTotalCntIsSet		,.
		NULL,							//		CBBPTN_portSEC_SET2				,.
		NULL,							//		CBBPTN_portSEC_SET1				,.
		NULL,							//		CBBPTN_portPG_SEC				,. 	// 2017/09/08 Sky戸田 S800 UART、GPIO対応.
		NULL,							//		CBBPTN_portFIERY_CNCT			,.
		NULL,							//		CBBPTN_portA800_PG				,.	// 2017/09/08 Sky戸田 S800 UART、GPIO対応.
		NULL,							//		CBBPTN_portACC_DETECT			,.	// 2017/09/08 Sky戸田 S800 UART、GPIO対応.
		NULL,							//		CBBPTN_portACC_DETECT2			,.	// 2017/09/08 Sky戸田 S800 UART、GPIO対応.
		NULL,							//		CBBPTN_portMC_PWR_EN			,.	// 2017/09/08 Sky戸田 S800 UART、GPIO対応.

										//		// ASIC22 出力ポート	ASIC22_REG_PIO_4R.
		&portLED_0					,	//		CBBPTN_portLED_0				,.
		&portLED_1_N				,	//		CBBPTN_portLED_1_N				,.
		&portLED_2					,	//		CBBPTN_portLED_2				,.
		&portLED_3					,	//		CBBPTN_portLED_3				,.
		&portLED_4_N				,	//		CBBPTN_portLED_4_N				,.
		&portLED_5					,	//		CBBPTN_portLED_5				,.
#ifdef DEF_MinervaS_PF
		&portPMS_PLL2_ENB			,	//		CBBPTN_portPMS_PLL2_ENB			,.
#endif // #ifdef DEF_MinervaS_PF.
		&portAC_POWER_DOWN			,	//		CBBPTN_portAC_POWER_DOWN		,.
#if defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS )
		&portSEL_COLOR0,					//		CBBPTN_portSEL_COLOR0			,.
//		&portSEL_COLOR1,					//		CBBPTN_portSEL_COLOR1			,.
//		&portSEL_COLOR2,					//		CBBPTN_portSEL_COLOR2			,.
#else // #if defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS ).
		&portPOWER_OFF_PNL			,	//		CBBPTN_portPOWER_OFF_PNL		,.
		&portPOWER_OFF				,	//		CBBPTN_portPOWER_OFF			,.
#endif // #if defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS ).

#ifdef COSIM								//	#ifdef COSIM.
		NULL,							//		CBBPTN_portCOSIM_ASIC1_SDRAM1_DUMP	,.
		NULL,							//		CBBPTN_portCOSIM_ASIC1_SDRAM0_DUMP	,.
		NULL,							//		CBBPTN_portCOSIM_ASIC0_SDRAM1_DUMP	,.
		NULL,							//		CBBPTN_portCOSIM_ASIC0_SDRAM0_DUMP	,.
		NULL,							//		CBBPTN_portCOSIM_DummyPicOut		,.
		NULL,							//		CBBPTN_portCOSIM_PEX_MODE_1			,.
		NULL,							//		CBBPTN_portCOSIM_PEX_MODE_0			,.
		NULL,							//		CBBPTN_portCOSIM_HsyncModel_Width	,.
		NULL,							//		CBBPTN_portCOSIM_HsyncModel_On		,.
#endif										//	#endif.

										//		// ASIC22 出力ポート	ASIC22_REG_PIO_5R.
		&portFIF_O_PD				,	//		CBBPTN_portFIF_O_PD				,.
		&portMOTER_EN				,	//		CBBPTN_portMOTER_EN				,.
		&ppppD103PaperFeed			,	//		CBBPTN_ppppD103PaperFeed		,.
		&portCTL_BUSY				,	//		CBBPTN_portCTL_BUSY				,.
		&ppppKeyCounter				,	//		CBBPTN_ppppKeyCounter			,.
		&ppppTotalCounter			,	//		CBBPTN_ppppTotalCounter			,.
		&portTERM_SEL				,	//		CBBPTN_portTERM_SEL				,.
		&portSPEEKER_REM			,	//		CBBPTN_portSPEEKER_REM			,.
		&portFUN_SPEED				,	//		CBBPTN_portFUN_SPEED			,.
		&portFUN_REM_1				,	//		CBBPTN_portFUN_REM_1			,.
		&portENGINE_SET				,	//		CBBPTN_portENGINE_SET			,.
		&portONLINE					,	//		CBBPTN_portONLINE				,.
#if !(defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS ))
		&portSEL_COLOR				,	//		CBBPTN_portSEL_COLOR			,.
#endif // #if !(defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS )).
		&portOut_IREQ_PIC_OUT		,	//		CBBPTN_portOut_IREQ_PIC_OUT		,.
		&portWAKE_FAXMFPC			,	//		CBBPTN_portWAKE_FAXMFPC			,.
		&portOut_IREQ_IR			,	//		CBBPTN_portOut_IREQ_IR			,.
		&portA800_RESET				,	//		CBBPTN_portA800_RESET			,.	// 2017/09/08 Sky戸田 S800 UART、GPIO対応.
		&portSEC_RESET				,	//		CBBPTN_portSEC_RESET			,.	// 2017/09/08 Sky戸田 S800 UART、GPIO対応.

#ifdef COSIM							//	#ifdef COSIM.
		NULL,							//		CBBPTN_portOut_COSIM_TOD_SIM_OUT,.
#endif									//	#endif.
										//		// ASIC22 RSTポート		ASIC22_REG_RST_0R.
		&portRST0_RST0				,	//		CBBPTN_portRST0_RST0			,.
		&portRST1_RST0				,	//		CBBPTN_portRST1_RST0			,.
		&portRST2_RST0				,	//		CBBPTN_portRST2_RST0			,.
		&portRST1_PIC				,	//		CBBPTN_portRST1_PIC				,.
		&portRST_0R					,	//		CBBPTN_portRST_0R				,.

										//		// ASIC22 RSTポート		ASIC22_REG_RST_4R.
		&portLCDC_LVDS_EN			,	//		CBBPTN_portLCDC_LVDS_EN			,.
		&portIRC_RESET				,	//		CBBPTN_portIRC_RESET			,.
		&portDOTCLKEN				,	//		CBBPTN_portDOTCLKEN				,.
		&portCK_EN_C				,	//		CBBPTN_portCK_EN_C				,.
		&portCK_EN_B				,	//		CBBPTN_portCK_EN_B				,.
		&portCK_EN_A				,	//		CBBPTN_portCK_EN_A				,.
		&portF_PD_T					,	//		CBBPTN_portF_PD_T				,.
		&portF_PD_R					,	//		CBBPTN_portF_PD_R				,.
		&portPCIRST_CPS				,	//		CBBPTN_portPCIRST_CPS			,.
		&portRST2_PIC				,	//		CBBPTN_portRST2_PIC				,.
		&portRST_4R					,	//		CBBPTN_portRST_4R				,.

										// 		// ASIC2014 入力ポート	ASIC2014_REG_PIO_0R.
		NULL,							//		CBBPTN_portLOCKN				,.
		NULL,							//		CBBPTN_portCTS_DISABLE			,.2015/09/14 morikawa Asic2014 UARTハード不具合対応。

#ifdef COSIM							//	#ifdef COSIM.
		NULL,							//		CBBPTN_portOut_COSIM_IMAGE_CODE	,.
		NULL,							//		CBBPTN_portOut_COSIM_IMAGE_SIZE	,.
		NULL,							//		CBBPTN_portOut_COSIM_IMAGE		,.
		NULL,							//		CBBPTN_portOut_COSIM_SCAN_IN	,.
		NULL,							//		CBBPTN_portOut_COSIM_FIERY_IN	,.
#endif									//	#endif.

#if defined(DEF_S800)
		NULL						,	// 		CBBPTN_portMC_P_ON				,.
		NULL						,	// 		CBBPTN_portIR_P_OFF				,.
		NULL						,	// 		CBBPTN_portIR_MOTION_EN			,.
		NULL						,	// 		CBBPTN_portLR_INT				,.
		NULL						,	// 		CBBPTN_portLR_SOUT				,.
		NULL						,	// 		CBBPTN_portPAGE_ON				,.
		NULL						,	// 		CBBPTN_portREV_PAGE_ON			,.
#if defined(SIMULATOR)	/* シミュレータで有効. */
		&portCLOSE					,	// 		CBBPTN_portCLOSE				,.
#else /* defined(SIMULATOR)*/	/* シミュレータで有効. */
		NULL						,	// 		CBBPTN_portCLOSE				,.
#endif /* defined(SIMULATOR)*/	/* シミュレータで有効. */
		NULL						,	// 		CBBPTN_portDSET					,.
		NULL						,	// 		CBBPTN_portWAKE_WLAN			,.
		NULL						,	// 		CBBPTN_portWAKE_UART			,.
#else /* defined(DEF_S800) */
		&portMC_P_ON				,	// 		CBBPTN_portMC_P_ON				,.	
		&portIR_P_OFF				,	// 		CBBPTN_portIR_P_OFF				,.
		&portIR_MOTION_EN			,	// 		CBBPTN_portIR_MOTION_EN			,.
		&portLR_INT					,	// 		CBBPTN_portLR_INT				,.
		&portLR_SOUT				,	// 		CBBPTN_portLR_SOUT				,.	
		&portPAGE_ON				,	// 		CBBPTN_portPAGE_ON				,.	
		&portREV_PAGE_ON			,	// 		CBBPTN_portREV_PAGE_ON			,.	
		&portCLOSE					,	// 		CBBPTN_portCLOSE				,.
		&portDSET					,	// 		CBBPTN_portDSET					,.
		&portWAKE_WLAN				,	// 		CBBPTN_portWAKE_WLAN			,.
		&portWAKE_UART				,	// 		CBBPTN_portWAKE_UART			,.
#endif /* defined(DEF_S800) */
		NULL						,	//		CBBPTN_portTHCV216_LOCKN		,.
		NULL						,	//		CBBPTN_portAPC_STATE			,.
		NULL						,	//		CBBPTN_portX_IREQ_P				,.
		NULL						,	//		CBBPTN_portDIV_SEL_GP6			,.
		NULL						,	//		CBBPTN_portXON_GP6				,.
										//		// ASIC22 ダミーポート.
		NULL,							//		CBBPTN_portINPUT_SEL			,.
		NULL,							//		CBBPTN_portPIC5600Type			,.
		NULL,							//		CBBPTN_portIO_IORST5			,.
		NULL,							//		CBBPTN_portIO_IORST4			,.
		NULL,							//		CBBPTN_portIOLCDC_RST			,.
		NULL,							//		CBBPTN_portLCD_SUSPEND			,.
		NULL,							//		CBBPTN_portPLL_RST_CPS			,.
		NULL,							//		CBBPTN_portIORESET1				,.
		NULL,							//		CBBPTN_portIORESET3				,.
		NULL,							//		CBBPTN_portIORESET2				,.
		NULL,							//		CBBPTN_portPOWER_REM			,.
		NULL,							//		CBBPTN_portIR_OEN				,.
		NULL,							//		CBBPTN_portPOWER_OFF0			,.
		NULL,							//		CBBPTN_portPOWER_OFF1			,.
		NULL,							//		CBBPTN_portDB7654				,.
		NULL,							//		CBBPTN_portKEY_SELECT			,.

		NULL,							//		CBBPTN_portPrinterIsPowerOn	 	,.
		NULL,							//		CBBPTN_portIORST1             	,.
		NULL,							//		CBBPTN_portIORST2             	,.
		NULL,							//		CBBPTN_portFILE_DIMM_INIT_1	 	,.
		NULL,							//		CBBPTN_portS_O_PD		     	,.
		NULL,							//		CBBPTN_portFILE_DIMM_INIT_2	 	,.
#ifdef DEF_EDH_COLOR					//	#ifdef DEF_EDH_COLOR.
		NULL,							//		CBBPTN_portAE_TG				,.
		NULL,							//		CBBPTN_portFB_AE3				,.
		NULL,							//		CBBPTN_portFB_AE2				,.
		NULL,							//		CBBPTN_portFB_AE1				,.
		NULL,							//		CBBPTN_portFB_AE0				,.
#endif									//	#endif.
		NULL,							//		CBBPTN_portServiceChk   		,.
		NULL,							//		CBBPTN_portTroubleReset 		,.
		NULL,							//		CBBPTN_portMemoryClear  		,.
		NULL,							//		CBBPTN_portInitial      		,.
		NULL,							//		CBBPTN_portTotalClear   		,.
		NULL,							//		CBBPTN_portOut_DEBUG_PIC_1		,.
		NULL,							//		CBBPTN_portOut_DEBUG_PIC_2		,.
		NULL,							//		CBBPTN_portOut_DEBUG_PIC_3		,.
		NULL,							//		CBBPTN_portOut_DEBUG_PIC_4		,.
		NULL,							//		CBBPTN_portSEL_IR             	,.
		NULL,							//		CBBPTN_portSEL_NT             	,.
		NULL,							//		CBBPTN_portP_VEN              	,.
		NULL,							//		CBBPTN_portCTS_TERM         	,.
		NULL,							//		CBBPTN_portCARDIN1          	,.
		NULL,							//		CBBPTN_portFUN_LOCK				,.
		NULL,							//		CBBPTN_portMSC_DTR          	,.
		NULL,							//		CBBPTN_portSSC_DSR          	,.
		NULL,							//		CBBPTN_portNTF_CNCT         	,.
		NULL,							//		CBBPTN_portOut_CLK_INITIAL    	,.
		NULL,							//		CBBPTN_portRTS		        	,.
		NULL,							//		CBBPTN_portSSC_DTR          	,.
		NULL,							//		CBBPTN_portIMG_THROUGH	        ,.
		NULL,							//		CBBPTN_portC_O_PD2		        ,.
		NULL,							//		CBBPTN_portC_O_PD		        ,.

#ifdef SIMULATOR						//	#ifdef SIMULATOR	// SIMWINで有効.
		NULL,							//		CBBPTN_portVendorCopyEnable 	,.
		NULL,							//		CBBPTN_portVendorPaperSize 		,.
		NULL,							//		CBBPTN_portVendorCopying   		,.
		NULL,							//		CBBPTN_ppppVendorPaperFeed 		,.
		NULL,							//		CBBPTN_ppppVendorPaperEject		,.
#else									//	#else.
#ifndef COSIM							//	#ifndef COSIM.
		NULL,							//		CBBPTN_ppppVendorPaperFeed 		,.
		NULL,							//		CBBPTN_ppppVendorPaperEject		,.
#endif									//	#endif.
#endif									//	#endif	// #ifdef SIMULATOR	// SIMWINで有効.
	};

	THRL_ThAssert( (Bool)( vList[ tNum ] != NULL ), "CB_BitPortN<THRD_Port_DataType> tNum: %"PRId32"", tNum, 0, 0, 0 , CFA14_DetailCode0x0871 );

	return *vList[tNum];
}

//	12/11/09 戸田	CPU選定評価基板対応　ushort用からulong用へ移植.
template <> CB_PulsePort<uint32_t>& CB_PulsePort<uint32_t>::newInstance(CB_PulsePortTypeNum type)
{
	int32_t tNum = (int32_t)type;

#ifdef SIMULATOR
	static	CB_PulsePort<uint32_t>		portVendorPaperFeed	( CBPPTN_portVendorPaperFeed	,	CBBPTN_ppppVendorPaperFeed	,	300/DEF_Timer30MSec,	300/DEF_Timer30MSec );	// ベンダー・給紙パルス.
	static	CB_PulsePort<uint32_t>		portVendorPaperEject( CBPPTN_portVendorPaperEject	,	CBBPTN_ppppVendorPaperEject	,	150/DEF_Timer30MSec,	150/DEF_Timer30MSec );	// ベンダー・排紙パルス.
#else									//	#else.
#ifndef COSIM							//	#ifndef COSIM.
	static	CB_PulsePort<uint32_t>		portVendorPaperFeed	( CBPPTN_portVendorPaperFeed	,	CBBPTN_ppppVendorPaperFeed	,	300/DEF_Timer30MSec,	300/DEF_Timer30MSec );	// ベンダー・給紙パルス.
	static	CB_PulsePort<uint32_t>		portVendorPaperEject( CBPPTN_portVendorPaperEject	,	CBBPTN_ppppVendorPaperEject	,	150/DEF_Timer30MSec,	150/DEF_Timer30MSec );	// ベンダー・排紙パルス.
#endif
#endif

	static CB_PulsePort<uint32_t>* vList[] = {
		NULL,							//		CBPPTN_portTotalCounter			,.
		NULL,							//		CBPPTN_portKeyCounter			,.
		NULL,							//		CBPPTN_portD102PaperFeed		,.
		NULL,							//		CBPPTN_portIREQ_IR				,.
		NULL,							//		CBPPTN_portLED_1				,.
		NULL,							//		CBPPTN_portLED_4 				,.

#ifdef SIMULATOR						//	#ifdef SIMULATOR	// SIMWINで有効.
		&portVendorPaperFeed		,	//		CBPPTN_portVendorPaperFeed 		,.
		&portVendorPaperEject		,	//		CBPPTN_portVendorPaperEject		,.
#else									//	#else.
#ifndef COSIM							//	#ifndef COSIM.
		&portVendorPaperFeed		,	//		CBPPTN_portVendorPaperFeed 		,.
		&portVendorPaperEject		,	//		CBPPTN_portVendorPaperEject		,.
#endif									//	#endif.
#endif									//	#endif	// #ifdef SIMULATOR	// SIMWINで有効.
	};

	THRL_ThAssert( (Bool)( vList[ tNum ] != NULL ), "CB_PulsePort<uintptr_t> tNum: %"PRId32"", tNum, 0, 0, 0 , CFA14_DetailCode0x0872 );

	return *vList[tNum];
}

//	12/11/09 戸田	CPU選定評価基板対応　ushort用からulong用へ移植.
template <> CB_PulsePort<ushort>& CB_PulsePort<ushort>::newInstance(CB_PulsePortTypeNum type)
{
	int32_t tNum = (int32_t)type;

	static	CB_PulsePort<ushort>	portTotalCounter	( CBPPTN_portTotalCounter	);	// トータルカウンター.
	static	CB_PulsePort<ushort>	portKeyCounter		( CBPPTN_portKeyCounter	 	);	// キーカウンター.
	static	CB_PulsePort<ushort>	portD102PaperFeed	( CBPPTN_portD102PaperFeed	);	// Ｄ１０３給紙パルス（03/10/18 morikawa 定義名称は102のまま）.
	static	CB_PulsePort<ushort>	portIREQ_IR			( CBPPTN_portIREQ_IR		);	// (EDH用)スキャン開始指示信号.
	static	CB_PulsePort<ushort>	portLED_1			( CBPPTN_portLED_1			);	// ウォッチドッグ.
	static	CB_PulsePort<ushort>	portLED_4			( CBPPTN_portLED_4			);	// ウォッチドッグ.

	static CB_PulsePort<ushort>* vList[] = {
		&portTotalCounter			,	//		CBPPTN_portTotalCounter			,.
		&portKeyCounter				,	//		CBPPTN_portKeyCounter			,.
		&portD102PaperFeed			,	//		CBPPTN_portD102PaperFeed		,.
		&portIREQ_IR				,	//		CBPPTN_portIREQ_IR				,.
		&portLED_1					,	//		CBPPTN_portLED_1				,.
		&portLED_4					,	//		CBPPTN_portLED_4 				,.

#ifdef SIMULATOR						//	#ifdef SIMULATOR	// SIMWINで有効.
		NULL,							//		CBPPTN_portVendorPaperFeed 		,.
		NULL,							//		CBPPTN_portVendorPaperEject		,.
#else									//	#else.
#ifndef COSIM							//	#ifndef COSIM.
		NULL,							//		CBPPTN_portVendorPaperFeed 		,.
		NULL,							//		CBPPTN_portVendorPaperEject		,.
#endif									//	#endif.
#endif									//	#endif	// #ifdef SIMULATOR	// SIMWINで有効.
	};

	THRL_ThAssert( (Bool)( vList[ tNum ] != NULL ), "CB_PulsePort<ushort> tNum: %"PRId32"", tNum, 0, 0, 0 , CFA14_DetailCode0x0873 );

	return *vList[tNum];
}

/*================================================================================================
［クラス］	ErrorCounter : エラーカウンタ
================================================================================================*/
ErrorCounter::ErrorCounter() {
	reset();
}

/*================================================================================================
［機能］	登録
--------------------------------------------------------------------------------------------------
［引数］	const uintptr_t& lineStatus		: ラインステータス
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	
［履歴］	
=================================================================================================*/
inline void ErrorCounter::regist( const uintptr_t& lineStatus )
{
	// パリティエラー.
	if ((lineStatus & SIO_LSR_PE) == SIO_LSR_PE) {
		++ peCount_;
	}
	
	// オーバランエラー.
	if ((lineStatus & SIO_LSR_OE) == SIO_LSR_OE) {
		++ oeCount_;
	}
	
	// フレーミングエラー.
	if ((lineStatus & SIO_LSR_FE) == SIO_LSR_FE) {
		++ feCount_;
	}
}

/*================================================================================================
［機能］	登録有無
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	登録有無(True=登録あり/False=登録なし)
［ｳｫｰｸｽﾙｰ］	
［履歴］	
=================================================================================================*/
Bool ErrorCounter::registered() const
{
	return (peCount_ > 0 || oeCount_ > 0 || feCount_ > 0) ? True : False;
}

/*================================================================================================
［機能］	リセット
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	
［履歴］	
=================================================================================================*/
void ErrorCounter::reset( )
{
	peCount_ = 0;
	oeCount_ = 0;
	feCount_ = 0;
}

/*================================================================================================
［クラス］	CB_SIO : シリアルポート用インターフェース基本クラス
================================================================================================*/
#define MAX_SIO_NUM  (32) // この値は足らなくなったら増やすこと.
static CB_SIO_ImplAsic* sioList_ImplAsic[MAX_SIO_NUM];
static int32_t i_sioListIdx_ImplAsic = 0;
static CB_SIO_Impl_Reg32* sioList_ImplReg32[MAX_SIO_NUM];
static int32_t i_sioListIdx_ImplReg32 = 0;

/*================================================================================================
 * ［機能］	UART受信スレッド起動関数
 *--------------------------------------------------------------------------------------------------
 * ［引数］	無し
 * ［戻値］	無し
 *================================================================================================*/
void spawnSioRecieveThread(
	void
)
{
	// ES基板検証 ASIC非装着起動対応.
	if( THRL_Drvrif_IsDeviceExist( THRE_ASIC_0 ) == False ) {
		return;
	}

	int32_t i = 0;
	// Asic22のUARTは、CB_SIO_ImplAsicインスタンスを使用する.
	for( i = 0; i < i_sioListIdx_ImplAsic; i++ ){
		sioList_ImplAsic[i]->spawnRecieveTh();
	}
	// PCITToUart(Fiery)は、CB_SIO_Impl_Reg32インスタンスを使用する(32Bitアクセスのため）.
	for( i = 0; i < i_sioListIdx_ImplReg32; i++ ){
		sioList_ImplReg32[i]->spawnRecieveTh();
	}
}

/*================================================================================================
 * ［機能］	UART受信スレッド CB_SIO_ImplAsic
 *--------------------------------------------------------------------------------------------------
 * ［引数］	...
 * ［戻値］	無し
 *================================================================================================*/
void* ReceiveTh_ImplAsic(
	CB_SIO_ImplAsic* sio
	...
)
{
// Str 2010/09/13 ft_kondo 8536環境Sim対応. 
#ifdef TORNADO
	int32_t ret = 0;
	off_t l_read_ret = 0;
	register uchar data[512];
	fd_set rfdset;
	int32_t i = 0;

	THRL_ThAssert( sio->getFd() != -1, "ReceiveTh_ImplAsic() fd(%"PRId32") is invalid. No.%d\n", sio->getFd(), sio->uc_ChNo , 0, 0, CFA14_DetailCode0x0874 );

	while( True ){
		FD_ZERO( &rfdset );
		FD_SET( sio->getFd(), &rfdset );

		ret = SYSF_select( sio->getFd() + 1, &rfdset, NULL, NULL, NULL );
		if( ret > 0 ){
			l_read_ret = SYSF_read( sio->getFd(), (char*)data, sizeof(data) );
			if( l_read_ret > 0 ){
				if ( (ASIC22_SIO_DSC1 == sio->uc_ChNo) && (False == THRF_IsDsc1Exist()) ) {
					/* 地紋1の受信データは読み飛ばす */
					logMsg( "@@ %"PRId32" : ReceiveTh_ImplAsic::%s Dsc1 skip\n", SYSF_tickGet(), (uintptr_t)__FUNCTION__, (uintptr_t)3, (uintptr_t)4, (uintptr_t)5, (uintptr_t)6 );
				} else if ( (ASIC22_SIO_DSC2 == sio->uc_ChNo) && (False == THRF_IsDsc2Exist()) ) {
					/* 地紋2の受信データは読み飛ばす */
					logMsg( "@@ %"PRId32" : ReceiveTh_ImplAsic::%s Dsc2 skip\n", SYSF_tickGet(), (uintptr_t)__FUNCTION__, (uintptr_t)3, (uintptr_t)4, (uintptr_t)5, (uintptr_t)6 );
				} else {
					for( i = 0; i < l_read_ret; i++ ){
						sio->rxQueue << data[i];
					}
				}
			}
		}
	}
#else 
	return 0;
#endif // #ifdef TORNADO
// End 2010/09/13 ft_kondo 8536環境Sim対応. 
}

/*================================================================================================
 * ［機能］	UART受信スレッド CB_SIO_Impl_Reg32(Fiery)
 *--------------------------------------------------------------------------------------------------
 * ［引数］	...
 * ［戻値］	無し
 *================================================================================================*/
void* ReceiveTh_ImplReg32(
	CB_SIO_Impl_Reg32* sio
	...
)
{
	int32_t ret = 0;
	off_t l_read_ret = 0;
	register uchar data[512];
	fd_set rfdset;
	int32_t i = 0;

	THRL_ThAssert( sio->getFd() != -1, "ReceiveTh_ImplReg32() fd(%"PRId32") is invalid. No.%"PRId32"\n", sio->getFd(), sio->sioUARTCLOCK , 0, 0, CFA14_DetailCode0x0875 );

	while( True ){
		FD_ZERO( &rfdset );
		FD_SET( sio->getFd(), &rfdset );

		ret = SYSF_select( sio->getFd() + 1, &rfdset, NULL, NULL, NULL );
		if( ret > 0 ){
			l_read_ret = SYSF_read( sio->getFd(), (char*)data, sizeof(data) );
			if( l_read_ret > 0 ){
				for( i = 0; i < l_read_ret; i++ ){
					sio->rxQueue << data[i];
				}
			}
		}
	}
}

/*================================================================================================
 * ［機能］	全キューサイズ表示
 *--------------------------------------------------------------------------------------------------
 * ［引数］	無し
 * ［戻値］	無し
 *================================================================================================*/
void sio_printusedmax(
	void
)
{
	for( int32_t i = 0; i < i_sioListIdx_ImplAsic; i++ ){
		sioList_ImplAsic[i]->dumpUsedQueueNum();
	}
}

// ================================================================================================.
//! <!--機能--> コンストラクタ.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param txQueue........送信キューバッファ.
//! \param rxQueue........受信キューバッファ.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//	07/03/23 冷水	 新規作成.
// ================================================================================================.
CB_SIO_Impl::CB_SIO_Impl(	CT_Queue<uchar>& txQueue,
							CT_Queue<uchar>& rxQueue
						)
:	txQueue( txQueue ),
	rxQueue( rxQueue )
{
}

// ================================================================================================.
//! <!--機能--> デストラクタ.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//	07/03/23 冷水	 新規作成.
// ================================================================================================.
CB_SIO_Impl::~CB_SIO_Impl()
{
}

// ================================================================================================.
//! <!--機能--> メンバ関数（全てダミー定義）：呼ばれることがあればASSERTします.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//	07/03/23 冷水	 新規作成.
// ================================================================================================.
// 初期化.
void CB_SIO_Impl::init( uchar /*mode*/, Bool /*rxEnable*/ )
{
	THRL_ThAssert( false, "CB_SIO_Impl::init() : illegal call function." , 0, 0, 0, 0, CFA14_DetailCode0x0876 );
}
// 初期化 NECマクロ初期化パターン ステップ1.
void CB_SIO_Impl::init1( ushort /*us_sioBaudRate*/ )
{
	THRL_ThAssert( false, "CB_SIO_Impl::init1() : illegal call function." , 0, 0, 0, 0, CFA14_DetailCode0x0877 );
}
// 初期化 Fujitsuマクロ初期化パターン ステップ1.
void CB_SIO_Impl::init1_F( uint32_t /*us_sioBaudRate*/ )
{
	THRL_ThAssert( false, "CB_SIO_Impl::init1_F() : illegal call function." , 0, 0, 0, 0, CFA14_DetailCode0x0878 );
}
// 初期化 NECマクロ初期化パターン ステップ2.
void CB_SIO_Impl::init2()
{
	THRL_ThAssert( false, "CB_SIO_Impl::init2() : illegal call function." , 0, 0, 0, 0, CFA14_DetailCode0x0879 );
}
// 転送開始.
void CB_SIO_Impl::startTransfer()
{
	THRL_ThAssert( false, "CB_SIO_Impl::startTransfer() : illegal call function." , 0, 0, 0, 0, CFA14_DetailCode0x087A );
}
// ＰＩＣ転送開始.
void CB_SIO_Impl::startTransferPIC()
{
	THRL_ThAssert( false, "CB_SIO_Impl::startTransferPIC() : illegal call function." , 0, 0, 0, 0, CFA14_DetailCode0x087B );
}
// 送信割込処理.
void CB_SIO_Impl::txInterrupt()
{
	THRL_ThAssert( false, "CB_SIO_Impl::txInterrupt() : illegal call function." , 0, 0, 0, 0, CFA14_DetailCode0x087C );
}
// 受信割込処理.
void CB_SIO_Impl::rxInterrupt()
{
	THRL_ThAssert( false, "CB_SIO_Impl::rxInterrupt() : illegal call function." , 0, 0, 0, 0, CFA14_DetailCode0x087D );
}
#ifdef TORNADO			// VXWORKSで有効.
// 送信割込処理.
void CB_SIO_Impl::txInterrupt2( void )
//void CB_SIO_Impl::txInterrupt2(MV_SIO_CHAN *pChan).
{
	THRL_ThAssert( false, "CB_SIO_Impl::txInterrupt2() : illegal call function." , 0, 0, 0, 0, CFA14_DetailCode0x087E );
}
// 受信割込処理.
void CB_SIO_Impl::rxInterrupt2( void )
//void CB_SIO_Impl::rxInterrupt2(MV_SIO_CHAN *pChan).
{
	THRL_ThAssert( false, "CB_SIO_Impl::rxInterrupt2() : illegal call function." , 0, 0, 0, 0, CFA14_DetailCode0x087F );
}
#endif	// #ifdef TORNADO.
// EMI テストの開始.
void CB_SIO_Impl::startEmiTest( uchar /*uc_Data*/ )
{
	THRL_ThAssert( false, "CB_SIO_Impl::startEmiTest() : illegal call function." , 0, 0, 0, 0, CFA14_DetailCode0x0880 );
}
// EMI テストの終了.
void CB_SIO_Impl::endEmiTest()
{
	THRL_ThAssert( false, "CB_SIO_Impl::endEmiTest() : illegal call function." , 0, 0, 0, 0, CFA14_DetailCode0x0881 );
}
//Str【メモリバス通信対応 : K.Natsume : 2013.11.14】
// ＩＲ転送開始.
void CB_SIO_Impl::startTransferIR()
{
	THRL_ThAssert( false, "CB_SIO_Impl::startTransferIR() : illegal call function." , 0, 0, 0, 0, CFA14_DetailCode0x0883 );
}
//End【メモリバス通信対応 : K.Natsume : 2013.11.14】
//Str【CPS分離用MINET通信ログ : K.Natsume : 2014.01.08】
uint32_t CB_SIO_Impl::getsioUARTCLOCK()
{
//	THRL_ThAssert( false, "CB_SIO_Impl::getsioUARTCLOCK() : illegal call function." , 0, 0, 0, 0, CFA14_DetailCode0x0884 );
	return (uint32_t)0;
}
//End【CPS分離用MINET通信ログ : K.Natsume : 2014.01.08】

// 共有メモリバス送信開始要求
void CB_SIO_Impl::startTransferSharedMemory()
{
}
	
// 共有メモリバス送信完了処理
void CB_SIO_Impl::endTransferSharedMemory()
{
}

// 共有メモリバス受信処理
void CB_SIO_Impl::receiveSharedMemory()
{
}

// ファンクションタイプ設定.
void CB_SIO_Impl::setFunctionType( uchar uc_FuncType )
{
#if !defined(FUM_LINUX) /* SimLinMC */
	THRL_ThAssert( false, "CB_SIO_Impl::setFunctionType() : illegal call function." , 0, 0, 0, 0, CFA14_DetailCode0x0763 );	// コードは仮
#else /* defined(FUM_LINUX) */
	printf("[%s] illegal call function.\n", __FUNCTION__ );

#endif /* !defined(FUM_LINUX) */
}
// ファンクションタイプ取得.
uchar CB_SIO_Impl::getFunctionType()
{
#if !defined(FUM_LINUX) /* SimLinMC */
	THRL_ThAssert( false, "CB_SIO_Impl::getFunctionType() : illegal call function." , 0, 0, 0, 0, CFA14_DetailCode0x0764 );	// コードは仮
#else /* defined(FUM_LINUX) */
	printf("[%s] illegal call function.\n", __FUNCTION__ );
#endif /* !defined(FUM_LINUX) */
	return 0;
}
// 共有メモリの書き込み領域の開始アドレスを設定
void CB_SIO_Impl::setWriteSharedMemoryOffsetAddress( ushort us_Address )
{
#if !defined(FUM_LINUX) /* SimLinMC */
	THRL_ThAssert( false, "CB_SIO_Impl::setWriteSharedMemoryOffsetAddress() : illegal call function." , 0, 0, 0, 0, CFA14_DetailCode0x0765 );	// コードは仮
#else /* defined(FUM_LINUX) */
	printf("[%s] illegal call function.\n", __FUNCTION__ );
#endif /* !defined(FUM_LINUX) */
}
// 共有メモリの読み込み領域の開始アドレスを設定
void CB_SIO_Impl::setReadSharedMemoryOffsetAddress( ushort us_Address )
{
#if !defined(FUM_LINUX) /* SimLinMC */
	THRL_ThAssert( false, "CB_SIO_Impl::setReadSharedMemoryOffsetAddress() : illegal call function." , 0, 0, 0, 0, CFA14_DetailCode0x0766 );	// コードは仮
#else /* defined(FUM_LINUX) */
	printf("[%s] illegal call function.\n", __FUNCTION__ );
#endif /* !defined(FUM_LINUX) */
}
// 共有メモリの通信領域サイズを設定
void CB_SIO_Impl::setTransDataSize( ushort us_Size )
{
}

// 送信パルス番号設定
void CB_SIO_Impl::setTxPulseNo( uchar uc_PulseNo )
{
}

// 受信パルス番号設定
void CB_SIO_Impl::setRxPulseNo( uchar uc_PulseNo )
{
}

// 2014/11/04 ft_kondo 静的解析対応 ID:19709. 
// 書込み待ち時間取得
uint32_t CB_SIO_Impl::getWriteSharedMemoryTickCnt( ushort uc_MemoryNo )
{
#if !defined(FUM_LINUX) /* SimLinMC */
	printf("[%s] illegal call function.\n", __FUNCTION__ );
#endif /* !defined(FUM_LINUX) */
	return 0;
}

// 書込み待ち時間クリア.
void CB_SIO_Impl::clearWriteSharedMemoryTickCnt( ushort uc_MemoryNo )
{
}

// 2014/11/04 ft_kondo 静的解析対応 ID:19710. 
// 読取異常状態取得
Bool CB_SIO_Impl::getReadSharedMemoryError()
{
#if !defined(FUM_LINUX) /* SimLinMC */
	printf("[%s] illegal call function.\n", __FUNCTION__ );
#endif /* !defined(FUM_LINUX) */
	return False;
}
// 読取異常状態クリア.
void CB_SIO_Impl::clearReadSharedMemoryError()
{
}


/*================================================================================================
［クラス］	CB_SIO_Impl_Reg32 : シリアルポート用インターフェース実装 32bitレジスタ依存（旧バージョン互換） 
================================================================================================*/
#define FIERY_UART_TFIFO_MAX	128

/*================================================================================================
［機能］	コンストラクタ
--------------------------------------------------------------------------------------------------
［引数］	
［戻値］	（自身）
================================================================================================*/
CB_SIO_Impl_Reg32::CB_SIO_Impl_Reg32(	EV_IntNumber ev_TxIntNumber,
				EV_IntNumber ev_RxIntNumber,
				uint32_t& sioUARTCLOCK,
				uint32_t& sioDivision1,
				uint32_t& sioDivision2,
				uint32_t& sioRXData,
				uint32_t& sioTXData,
				uint32_t& sioIRID,
				uint32_t& sioFIFOCtrl,
				uint32_t& sioLineCtrl,
				uint32_t& sioModemCtrl,
				uint32_t& sioLineStatus,
				uint32_t& sioModemStatus,
				uint32_t& sioScratch,
				CT_Queue<uchar>& txQueue,
				CT_Queue<uchar>& rxQueue,
				E_UART_Type uartType
			)
:	CB_SIO_Impl(txQueue,rxQueue),
	ev_TxIntNumber( ev_TxIntNumber ),
	ev_RxIntNumber( ev_RxIntNumber ),
	sioUARTCLOCK( sioUARTCLOCK ),
	sioDivision1( sioDivision1 ),
	sioDivision2( sioDivision2 ),
	sioRXData( sioRXData ),
	sioTXData( sioTXData ),
	sioIRID( sioIRID ),
	sioFIFOCtrl(sioFIFOCtrl),
	sioLineCtrl(sioLineCtrl),
	sioModemCtrl(sioModemCtrl),
	sioLineStatus(sioLineStatus),
	sioModemStatus(sioModemStatus),
	sioScratch(sioScratch),
	nowSending( False ),
#ifdef SIMULATOR
	simConnection( NULL ), // SIMとの接続クラス 2013.07.05 kanai.
#endif
	UART_Type( uartType )
{
	//【START Fiery送受信処理の再設計 yashiro】.
	fd = -1;
	if( i_sioListIdx_ImplReg32 >= MAX_SIO_NUM ){
		fprintf( stderr, "CB_SIO sioList_ImplAsic overflow. %"PRId32"\n", i_sioListIdx_ImplReg32 );
	}else{
		sioList_ImplReg32[ i_sioListIdx_ImplReg32++ ] = this;
	}
	//【END Fiery送受信処理の再設計 yashiro】.

#ifdef COSIM
	if( ( this->sioUARTCLOCK != 1 ) && ( this->sioUARTCLOCK != 2 ) && ( this->sioUARTCLOCK != 3 ) ) {
		this->sioUARTCLOCK = 0;	// COSIMの時は、エンジンとIRCもシミュレータにする.
	}
#endif

}

/*================================================================================================
［機能］	転送開始
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	実施済み：2000/7/26：出山
［履歴］	04/02/19 morikawa Rhien1　Fiery高速シリアル対応
　　	  04/06/11 ikawa	不要な処理を削除
		  09/09/11 越野		Soc対応.
　　	  09/11/18 nakagawa	SYSF_write()の戻り値チェック（WT指摘対応）.
================================================================================================*/
void CB_SIO_Impl_Reg32::startTransfer()
{
	register uchar data = 0;
	if( nowSending == False ) {
		// 現在送信中でない.
		if( txQueue.isExist() == True ) {
// 治具の時もPICのターミナル使用のため条件変更.
// #if !defined(SIMULATOR) && !defined(COSIM)	// シミュレータでないかつCOSIMでない.
#if !defined(SIMULATOR)	// COSIMでUARTの試験をする際はこのコンパイルスイッチを使用すること.

/////////	以下は実機での処理	/////////////.
			if( sioUARTCLOCK == 4 ) {			// EPNet Marvel (BSPのドライバを使用).
				short ss_CreScope = 0;
				if ( THRL_Drvrif_IsLineMode() == 1 ) {	/* ラインモードの場合 */
						while( txQueue.isExist() == True ) {				// CreScopeに対応しないため、送信キューに入れたとしてもここで抜いてしまう.
							txQueue >> data;
						}
						nowSending = False;				// 現在未送信.
				} else {					/* ラインモードでない場合 */
					if (THRL_Drvrif_GetDipswValue(4) == ss_CreScope) {	/* （型試以降 && DIPSW OFF）||（型試以前 && DIPSW ON） CreScope使用 */
						nowSending = True;				// 現在送信中.
					} else {								/* （型試以降 && DIPSW ON）||（型試以前 && DIPSW OFF） Terminal 使用 */
						while( txQueue.isExist() == True ) {				// CreScopeに対応しないため、送信キューに入れたとしてもここで抜いてしまう.
							txQueue >> data;
						}
						nowSending = False;				// 現在未送信.
					}
				}
			} else if( sioUARTCLOCK == 9 ) {		// XPanel Marvel (BSPのドライバを使用).
				short ss_CreScope = 0;
				if( uc_SelectKeyFlag == 0 ) {	/* 通常モードの場合 */
						while( txQueue.isExist() == True ) {				// XPanelに対応しないため、送信キューに入れたとしてもここで抜いてしまう.
							txQueue >> data;
						}
						nowSending = False;				// 現在未送信.
				} else {						/* 外部パネル接続モードの場合 */
					if( THRL_Drvrif_GetDipswValue(4) == ss_CreScope ) {	/* （型試以降 && DIPSW OFF）||（型試以前 && DIPSW ON） CreScope使用 */
						nowSending = True;				// 現在送信中.
					} else {								/* （型試以降 && DIPSW ON）||（型試以前 && DIPSW OFF） Terminal 使用 */
						while( txQueue.isExist() == True ) {				// XPanelに対応しないため、送信キューに入れたとしてもここで抜いてしまう.
						txQueue >> data;
						}
						nowSending = False;				// 現在未送信.
					}
				}
			} else {
				uchar	buff[4096];
				register ushort	idx = 0;

				if( sioUARTCLOCK != 8 ) {
					if ( UART_Type != UART_Type_ASIC110 ){
						// ダミー領域への書き込みしかないため、このままで問題なし.
						// 下記関数削除のため、直接ダミー領域へ書き込む.
						THRL_ThLogMsgDebug( "Send empty interrupt OK" );
						sioIRID = (sioIRID | SIO_IER_ETBEI);
						// ThWriteReg( &sioIRID, (sioIRID | SIO_IER_ETBEI), "送信空割り込みを許可" );.
					}
				} else {
					// Fieryのシリアルは高速のため、64byteの送信FIFOを使用する.
					while( txQueue.isExist() == True ) {
						txQueue >> data;
						buff[idx++] = data;
						if( idx >= 4095 ) { break; }
					}
					if( idx > 0 ){
						if( fd != -1 ){
							nowSending = True;			// 現在送信中.
							if( SYSF_write( fd, (char*)buff, idx ) < 0 ){
								THRL_ThLogMsgDebug( "SYSF_write() Error!!!\n" );
							}
							nowSending = False;			// 現在未送信.
						}
					}
				}
			}
#else				// シミュレータorCOSIMで有効.
			uchar	buff[4096];
			register ushort	idx = 0;
			switch( sioUARTCLOCK ) {
//Str【メモリバス通信対応 : K.Natsume : 2013.11.14】
				case 11:buff[idx++] = IJC_SOCKET_DATA_IRC;	break;
//End【メモリバス通信対応 : K.Natsume : 2013.11.14】
				case 1:	buff[idx++] = IJC_SOCKET_DATA_IRC;	break;
				case 2:	buff[idx++] = IJC_SOCKET_DATA_PRT;	break;
				case 0x0A:buff[idx++] = IJC_SOCKET_DATA_MCPF;	break;	// (A=sioMCPFmode).
#if defined(DEF_EDH_COLOR) // 【Rhein2マージ撲滅】05/09/06 Div.12 k-kawa.
				case 0x30:buff[idx++] = IJC_SOCKET_DATA_IRC;	break;
				case 0x40:
					// DSC_SIM対応 start 2013.07.05 kanai.
					if (simConnection == NULL) {
						simConnection = SIMC_SimConnectionIf::getInstance(SIMC_SimConnectionIf::CONNECTION_DSC1);
						simConnection->setCB_SIO_Impl(this);
						simConnection->connect();
					}
					simConnection->send();
					return;
					break;
				case 0x50:
					if (simConnection == NULL) {
						simConnection = SIMC_SimConnectionIf::getInstance(SIMC_SimConnectionIf::CONNECTION_DSC2);
						simConnection->setCB_SIO_Impl(this);
						simConnection->connect();
					}
					simConnection->send();
					return;
					break;
					// DSC_SIM対応 end 2013.07.05 kanai.
#endif	// #if defined(DEF_EDH_COLOR).
				default: break;
			}
			while( txQueue.isExist() == True ) {
				txQueue >> data;
				if( sioUARTCLOCK == 3 ) {
					// ＰＩＣへデータ送信する処理追加必要★★.
#ifdef COSIM
					extern uchar uc_MonitorPicComu;	// 0:通常 , 1: PICとの通信をログ表示.
					if( uc_MonitorPicComu == 1 ){
						dbgCosimLogMsg("CB_SIO::startTransfer %d [0x%x] ", data, data );
					}
#endif
					PutTransDataPIC( data );
				} else {
					buff[idx++] = data;
				}
				if( idx >= 4095 ) { break; }
			} // データ在るだけ読み出し.

#if defined(COSIM)
			if( buff[0] == IJC_SOCKET_DATA_IRC ) {
				IRC_SIMReceiveMessage_NT( (int32_t)idx-1, (uchar*)&buff[1] );
			} else {			
				PRT_SIMReceiveMessage_NT( (int32_t)idx-1, (uchar*)&buff[1] );
			}
#else
			socketSendMessage( (char*)buff, idx );	// メッセージを送信する.
#endif	// #if defined(COSIM).
			
#endif	// #if !defined(SIMULATOR) && !defined(COSIM)		// シミュレータでない.
		}
	} else {
#ifdef TORNADO	// sawayanagi Debug.
		if( sioUARTCLOCK == 4 ) {			// EPNet Marvel (BSPのドライバを使用).

			short ss_Terminal = 1;
			if ( THRL_Drvrif_IsLineMode() == 1 ) {	/* ラインモードの場合 */
				while( txQueue.isExist() == True ) {				// CreScopeに対応しないため、送信キューに入れたとしてもここで抜いてしまう.
					txQueue >> data;
				}
				nowSending = False;				// 現在未送信.
			} else {					/* ラインモードでない場合 */
				if (THRL_Drvrif_GetDipswValue(4) == ss_Terminal) {	/* （型試以降 && DIPSW ON）||（型試以前 && DIPSW OFF） Terminal 使用 */
					while( txQueue.isExist() == True ) {				// CreScopeに対応しないため、送信キューに入れたとしてもここで抜いてしまう.
						txQueue >> data;
					}
					nowSending = False;				// 現在未送信.
				}
			}
		} else if( sioUARTCLOCK == 9 ) {	// Rhein1P XPanel (BSPのドライバを使用).
			short ss_Terminal = 1;
			if( uc_SelectKeyFlag == 0 ) {	/* 通常モードの場合 */
				while( txQueue.isExist() == True ) {				// XPanelに対応しないため、送信キューに入れたとしてもここで抜いてしまう.
					txQueue >> data;
				}
				nowSending = False;					// 現在未送信.
			} else {						/* 外部パネル接続モードの場合 */
				if( THRL_Drvrif_GetDipswValue(4) == ss_Terminal ) {	/* （型試以降 && DIPSW ON）||（型試以前 && DIPSW OFF） Terminal 使用 */
					while( txQueue.isExist() == True ) {				// XPanelに対応しないため、送信キューに入れたとしてもここで抜いてしまう.
						txQueue >> data;
					}
					nowSending = False;				// 現在未送信.
				}
			}
		}
#endif	// #ifdef TORNADO	// sawayanagi Debug.
	}
}

/*================================================================================================
［機能］	ＰＩＣ転送開始
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
================================================================================================*/
void CB_SIO_Impl_Reg32::startTransferPIC()
{
	register uchar data = 0;
	if( txQueue.isExist() == True ) {
		while( txQueue.isExist() == True ) {
			txQueue >> data;
			// ＰＩＣへデータ送信する処理追加必要★★.
#ifdef COSIM
			extern uchar uc_MonitorPicComu;	// 0:通常 , 1: PICとの通信をログ表示.
			if( uc_MonitorPicComu == 1 ){
				dbgCosimLogMsg("CB_SIO::startTransferPIC %d [0x%x]", data, data );
			}
#endif
			PutTransDataPIC( data );
		}
	}
}
//Str【メモリバス通信対応 : K.Natsume : 2013.11.14】
//Str【ES検証機環境マージ対応 : K.Natsume : 2014.02.06】
#if	defined(DEF_Helios_PF) && !defined(SIMULATOR)
extern void PutTransDataIRM( uchar uc_data );
extern void PutTransDataIRC( uchar uc_data );
#endif // defined(DEF_Helios_PF) && !defined(SIMULATOR)
//End【ES検証機環境マージ対応 : K.Natsume : 2014.02.06】
/*================================================================================================
［機能］	ＩＲ転送開始
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
================================================================================================*/
void CB_SIO_Impl_Reg32::startTransferIR()
{
	register uchar data = 0;
	if( txQueue.isExist() == True ) {
		while( txQueue.isExist() == True ) {
			txQueue >> data;
//Str【ES検証機環境マージ対応 : K.Natsume : 2014.02.06】
#if	defined(DEF_Helios_PF) && !defined(SIMULATOR)
			if( sioUARTCLOCK == 1 ) {
				PutTransDataIRM( data );	// IRC→IRMへデータセット.
			} else {
				PutTransDataIRC( data );	// IRM→IRCへデータセット.
			}
#endif
//End【ES検証機環境マージ対応 : K.Natsume : 2014.02.06】
		}
	}
}
//End【メモリバス通信対応 : K.Natsume : 2013.11.14】

/*================================================================================================
［機能］	受信タスク起動
----------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし
［履歴］	2010/12/10 iyanagi	FieryUSBtoUART対応.
			2011/12/02 窪川 Asic30装着時はFieryUSBドライバへのパラメータ設定処理をスキップ.
			2014/09/30 Sky	Asic2014対応.
			2015/10/06 Sky	システム統合SoC(S800)対応.
			2017/04/07 Sky	ZeusSMLKPF対応.
==================================================================================================*/
void CB_SIO_Impl_Reg32::spawnRecieveTh()
{
// 2013/07/02 KM(Morikawa) SoC LinuxSim JFM Merge.
#if defined(TORNADO) && !defined(FUM_LINUX)
// #if defined(TORNADO)
// 実機モード.
	char* devFile = NULL;
	char* tskName = NULL;
	// 2011/12/19 ft_kondo 静的解析 CID:13060. 
	THR_ID ret = -1;

	switch( sioUARTCLOCK ){
		case 8:		// USBToUART(Fiery).
			if( THRE_AsicKind_A800 == THRL_Drvrif_GetAsicKind( THRE_AsicUID_Asic_2 ) ) {
				devFile = (char*)"/dev/sbst0A800drvr0";// udevルールによる固定デバイス名に対応.
				tskName = (char*)"ReceiveUSBToFiery";
			}
			else if ( THRE_AsicKind_Asic2014 == THRL_Drvrif_GetAsicKind( THRE_AsicUID_Asic_3 ) ) { 
				devFile = (char*)"/dev/sbst02014drvr0";// udevルールによる固定デバイス名に対応.
				tskName = (char*)"ReceiveUSBToFiery";
			}
			else{
				devFile = tskName = NULL;
				fd = -1;
			}
			break;
		default:
			devFile = tskName = NULL;
			fd = -1;
			break;
	}

	if( devFile != NULL && fd == -1 && THRF_IsFieryOption() ){// Asic2枚目（Fiery）のチェックとしておく.

// Fiery装着暫定対応.
		uint32_t i;
		for ( i = 0; i < 100; i++ ) {
			if ( access( devFile, F_OK ) == 0 ) {
				break;
			}
			delayTask( 10 );
		}
		printf(">>[Fiery device file] wait roop count = %"PRIu32".\n", i);
// Fiery装着暫定対応.

		fd = SYSF_open( devFile, O_RDWR, 0644 );
		THRL_ThAssert( fd != -1, "CB_SIO open ERR %d", errno , 0, 0, 0, CFA14_DetailCode0x0883 );

		// 受信タスクを起動する.
		ret = SYSF_taskSpawn( tskName, 17, VX_FP_TASK, 8192, (FUNCPTR)ReceiveTh_ImplReg32, (intptr_t)this, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
		THRL_ThAssert( ret != ERROR, "CB_SIO::CB_SIO() SYSF_taskSpawn() ERR\n" , 0, 0, 0, 0, CFA14_DetailCode0x0884 );
	}
#endif // #if defined(TORNADO).
}

/*================================================================================================
［機能］	過去最大使用キューサイズの表示（デバッグ用）
----------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし
==================================================================================================*/
void CB_SIO_Impl_Reg32::dumpUsedQueueNum()
{
	printf( " TxQueue:%8d RxQueue:%8d\n", txQueue.isNum(), rxQueue.isNum() );
}

/*================================================================================================
［クラス］	CB_SIO_ImplAsic : シリアルポート用インターフェース実装クラス
================================================================================================*/
// ================================================================================================.
//! <!--機能--> コンストラクタ.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param c_BiosIF....BIOS I/Fクラスへの参照.
//! \param uc_ChNo.....チャンネル番号.
//! \param c_Tx........送信キューバッファ.
//! \param c_Rx........受信キューバッファ.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//	07/03/23 冷水	 新規作成.
//	12/11/12 sasaki	 CPU選定評価基板対応 Asic22対応 UART変更対応.
//	14/10/21 Sky O.Suzuki [CQ503465]静的解析対応.
//  17/09/14 ADS佐藤 IT6向けターミナルモード切り替え対応.
// ================================================================================================.
CB_SIO_ImplAsic::CB_SIO_ImplAsic( /*CB_SIO_BiosIF& c_BiosIF,*/
									uchar uc_ChNo,
									CT_Queue<uchar>& c_Tx,
									CT_Queue<uchar>& c_Rx,
									UART_FUNCTION_TYPE uc_FunctionType
								)
:	CB_SIO_Impl(c_Tx,c_Rx),
/*	c_BiosIF(c_BiosIF),*/
//	→ 静的解析対応(CIM444013) 2011/05/23 saka.
	fd(0),
//	← 静的解析対応.
// 14/10/21 Sky O.Suzuki [CQ503465]静的解析対応.
	uc_ChNo(uc_ChNo),
	uc_FunctionType(uc_FunctionType),
//Str ASIC777対応 : 谷(ICS) : 2013/10/29.
#if defined(S800_VIDEO_MCPF)
	us_SharedMemoryTransDataSize( 512 ), // 仮
	us_SharedMemoryWriteOffsetAddress( 512 ),
#else // S800_VIDEO_MCPF.
	us_SharedMemoryTransDataSize( 20 ), // 仮
	us_SharedMemoryWriteOffsetAddress( 20 ),
#endif // S800_VIDEO_MCPF.
	us_SharedMemoryReadOffsetAddress( 0 ),
	uc_TxPulseNo( THRD_SHAREDMEMORY_PULSENO2 ),
	uc_RxPulseNo( THRD_SHAREDMEMORY_PULSENO1 ),
	b_NowSendingSharedMemory1( False ),
	b_NowSendingSharedMemory2( False ),
	ul_WriteSharedMemory1TickCnt( 0 ),
	ul_WriteSharedMemory2TickCnt( 0 ),
	b_SharedMemoryTransError( False )
//End ASIC777対応 : 谷(ICS) : 2013/10/29.
{
	uchar		uc_RegistCallbackFlg = 1;

#ifdef TORNADO
	if( UART_FUNCTION_TYPE_PICTERM == uc_FunctionType ){
		uc_RegistCallbackFlg = 0;	/* PICターミナルは常に無効化する */
	}
	else if( UART_FUNCTION_TYPE_MSCTERM == uc_FunctionType ){
		uc_RegistCallbackFlg = 0;	/* MSCターミナルは常に無効化する */
	}
	else {
		/* それ以外は常に有効 */
	}
#endif // TORNADO.

	if ( uc_RegistCallbackFlg ) {
		fd = -1;
		if( i_sioListIdx_ImplAsic >= MAX_SIO_NUM ){
			fprintf( stderr, "CB_SIO sioList_ImplAsic overflow. %"PRId32"\n", i_sioListIdx_ImplAsic );
		}else{
			sioList_ImplAsic[ i_sioListIdx_ImplAsic++ ] = this;
		}
	}

	// Linux追加.
	semID = SYSF_semBCreate(SEM_Q_FIFO, SEM_FULL);
	if(NULL == semID) {
		fprintf(stderr, "CB_SIO_ImplAsic<constructor> Ch:%d func:%02X SYSF_semBCreate:NULL\n", uc_ChNo, uc_FunctionType);
	}
}

// ================================================================================================.
//! <!--機能--> デストラクタ.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//	07/03/23 冷水	 新規作成.
// ================================================================================================.
CB_SIO_ImplAsic::~CB_SIO_ImplAsic()
{
	// Linux追加.
	SYSF_semDelete(semID);
}

// ================================================================================================.
//! <!--機能--> 送信開始要求.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//	07/03/23 冷水	 新規作成.
//	09/11/18 仲川	 SYSF_write()の戻り値チェック（WT指摘対応）.
//	12/11/12 sasaki	 CPU選定評価基板対応 Asic22対応 UART変更対応.
//	13/02/13 toda	 CPU選定評価基板対応 D103管理装置には送信しない(暫定).
//  17/09/14 ADS佐藤 IT6向けターミナルモード切り替え対応.
// ================================================================================================.
void CB_SIO_ImplAsic::startTransfer()
{
	register uchar data = 0;			// データ廃棄用バッファ.
	uchar		   uc_transferflg = 1;	// 転送フラグ.

#if defined(TORNADO)
	uchar buff[4096];
#endif // #if defined(TORNADO).
	register ushort	idx = 0;

	// Linux追加.
	SYSF_semTake(semID, WAIT_FOREVER);

#ifdef TORNADO
	if( UART_FUNCTION_TYPE_PICTERM == uc_FunctionType ){
		uc_transferflg = 0;	/* PICターミナルは常に無効化する */
	}
	else if( UART_FUNCTION_TYPE_MSCTERM == uc_FunctionType ){
		uc_transferflg = 0;	/* MSCターミナルは常に無効化する */
	}
	else {
		/* それ以外は常に有効 */
	}
#endif // TORNADO.

	if( uc_transferflg ) {
// 【追加】自動評価対応 [start] 2008.08.11 鈴木希 SIM_PRT,SIM_IRCを実機環境でも使用できる様にする.
#if defined( DEF_SimAutoTest )		// 自動評価で有効.
		CNVC_PanelHardIf* pc_PanelHard = CNVC_PanelHardIf::newInstance();
		if( pc_PanelHard->getSimAutoTestConnected() == True
		 && ( uc_ChNo == ASIC22_SIO_ENGDATA || uc_ChNo == ASIC22_SIO_IR || uc_ChNo == ASIC22_SIO_EDH ) ) {
			switch( uc_ChNo ) {
				case ASIC22_SIO_ENGDATA:buff[idx++] = IJC_SOCKET_DATA_PRT;	break;
				case ASIC22_SIO_IR:		buff[idx++] = IJC_SOCKET_DATA_IRC;	break;
				case ASIC22_SIO_EDH:	buff[idx++] = IJC_SOCKET_DATA_IRC;	break;
			}
			while( txQueue.isExist() == True ) {
				txQueue >> data;
				buff[idx++] = data;
				if( idx >= 4095 ) { break; }
			} // データ在るだけ読み出し.
			socketSendMessage( (char*)buff, idx );	// メッセージを送信する.
		}
		else
		{
			// 送信開始.
			THRL_Drvrif_Asic22_sioTransStart( uc_ChNo );
//			c_BiosIF.sioTransStart( uc_ChNo );.
		}
#else	// #if defined( DEF_SimAutoTest )		// 自動評価で有効.

#if defined(TORNADO)
	// 実機モード.
//Str ASIC777対応 : tadamasa : 2014/3/05.
		// 共有メモリ（メモリバス通信）
		if( uc_FunctionType == UART_FUNCTION_TYPE_SHAREDMEMORY )
		{
			startTransferSharedMemory();
		}
		// UART
		else
		{
			while( txQueue.isExist() == True ) {
				txQueue >> data;
				buff[idx++] = data;
				if( idx >= 4095 ) { break; }
			}
			if( idx > 0 ){
				if( fd != -1 ){
					if( SYSF_write( fd, (char*)buff, idx ) < 0 ){
						THRL_ThLogMsgDebug( "SYSF_write() Error!!!\n" );
					}
				}
			}
		}
//End ASIC777対応 : tadamasa : 2014/3/05.
#else // #if defined(TORNADO).
	// シミュレータモード.
		// 送信開始.
		THRL_Drvrif_Asic22_sioTransStart( uc_ChNo );
//		c_BiosIF.sioTransStart( uc_ChNo );.
#endif // #if defined(TORNADO).

#endif	// #if defined( DEF_SimAutoTest )		// 自動評価で有効.
// 【追加】自動評価対応 [ end ] 2008.08.11 鈴木希 SIM_PRT,SIM_IRCを実機環境でも使用できる様にする.
	} else {
		// 無効な通信機能からの呼び出しのため、キューデータを廃棄して送信処理終了.
		while( txQueue.isExist() == True ) {
			txQueue >> data;
		}
	}

	// Linux追加.
	SYSF_semGive(semID);
}

// ================================================================================================.
//! <!--機能--> 共有メモリバス送信開始要求.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//!	通信フォーマット
//!		第1バイト			  ：有効データサイズ().
//!		第2バイト～第N+1バイト：データ.
//!		第N+2バイト 		  ：第1バイト～第N+1バイトの加算値.
//!	注意事項.
//!		この関数を呼び出す場合には、呼び出し元でセマフォをロックする必要がある.
//!		それを行わないと、転送中フラグの制御がおかしくなる為.
//! <!--履歴-->.
//	14/03/05 忠政		 新規作成.
//	17/06/08 Sky		 S800 共有メモリ通信対応.
//	18/06/26 Pwr田川	 節電復帰時に大量出力する「MCPF init fault」ログ無効.
// ================================================================================================.
uint32_t ul_dbgdatachk = 0;
#ifdef DEF_A4_Cus
extern CC_MCPF comMCPF;
#endif
void CB_SIO_ImplAsic::startTransferSharedMemory()
{
#if defined(TORNADO)
	setSharedMemLog("startTransferSharedMemory:b_NowSendingSharedMemory1=%d uc_TxPulseNo=%d\n",b_NowSendingSharedMemory1, uc_TxPulseNo);

#if defined( DEF_A4_Cus ) && !defined( DEF_SparrowCus )
#if 1
	if( (uc_ChNo == 8) && (uc_TxPulseNo != 8) ){
		int32_t ic;
		for(ic = 0;ic < 20;ic ++){
			printf("%s line:%d MCPF init fault comMCPF.b_SubUnitConnectReq:%d\n",__func__,__LINE__,comMCPF.getSubUnitConnectReq());
		}
	}
#else
	if( (uc_ChNo == 8) && (uc_TxPulseNo != 8) ){
		int32_t ic;
		for(ic = 0;ic < 200;ic ++){
			printf("%s line:%d MCPF init fault waiting comMCPF.b_SubUnitConnectReq:%d\n",__func__,__LINE__,comMCPF.getSubUnitConnectReq());
			sleep(1);
			if( uc_TxPulseNo == 8 )break;
		}
		if(ic == 100){
			printf("%s line:%d MCPF init fault 100 comMCPF.b_SubUnitConnectReq:%d\n",__func__,__LINE__,comMCPF.getSubUnitConnectReq());
			return;
		}
		sleep(1);
	}
#endif
#endif

	if(False == b_NowSendingSharedMemory1){						// 転送中でない.(割り込みからの呼び出しの場合には事前にFalseにしておく) 
#if defined(S800_VIDEO_MCPF)
		// 実機モード.
		uchar uc_Buff[THRD_SHAREDMEMORY_WORKDATA_SIZE];			// 送信バッファ。Zeus5Bkでは最大164バイト使用。
																// 今後のことを考え、メモリの最大サイズは1024バイト。それを送受信で分けるので512とする.
		short ss_DataNum = txQueue.isNum();						// 送信データＱｕｅｕｅ.
		ushort us_Idx  = 0;										// 送信データＩｎｄｅｘ情報.
		
		setSharedMemLog("startTransferSharedMemory:ss_DataNum=%d\n",ss_DataNum );
		
		//us_SharedMemoryTransDataSize(データのサイズ)が3バイト以下(データサイズは最低4バイト必要)かチェック.
		if( THRD_SHAREDMEMORY_DATA_INFO_SIZE >= us_SharedMemoryTransDataSize ){
					setSharedMemLog( "startTransferSharedMemory() 1 us_SharedMemoryTransDataSize[%d]  Error!!!\n",us_SharedMemoryTransDataSize );
					b_SharedMemoryTransError = True;
		}
		
		// 最大転送データサイズが、超えていないことをチェックする.
		if( us_SharedMemoryTransDataSize > THRD_SHAREDMEMORY_TRANS_MAX_DATA_SIZE  ){
			setSharedMemLog( "startTransferSharedMemory() 2 us_SharedMemoryTransDataSize[%d]   Error!!!\n" ,us_SharedMemoryTransDataSize);
			b_SharedMemoryTransError = True;
		}
		
		if( ss_DataNum > (us_SharedMemoryTransDataSize - THRD_SHAREDMEMORY_DATA_INFO_SIZE) ) {
			setSharedMemLog( "startTransferSharedMemory() us_SharedMemoryTransDataSize[%d] ss_DataNum[%d]\n" ,us_SharedMemoryTransDataSize,ss_DataNum);
			ss_DataNum = us_SharedMemoryTransDataSize - THRD_SHAREDMEMORY_DATA_INFO_SIZE;			// 送信データ長ＭＡＸ補正（デフォルト：システム＝２０Ｂｙｔｅ／サブユニット＝１６４Ｂｙｔ）.
			// 先頭Ｂｙｔｅ：送信長／終端Ｂｙｔｅ：チェックサム情報を引いた（３Ｂｙｔｅ）レングスが送信データ数となる.
		}
		
				
		if( ss_DataNum > 0 ) {
			uchar uc_DataSum = 0x00;							// 送信データチェックサム情報.
			ushort us_DataNum = 0;								// 型変換用uchar変数.
			uchar uc_Data = 0;									// データ廃棄用バッファ.
			
			us_DataNum = ss_DataNum;
			
			uc_Buff[0] = (us_DataNum >> 0) & 0x00FF;		// チェックサムの1バイト目にデータサイズを加算する.
			uc_DataSum += uc_Buff[0];						// チェックサムを加算する.
			uc_Buff[1] = (us_DataNum >> 8) & 0x00FF;		// チェックサムの2バイト目にデータサイズを加算する.
			uc_DataSum += uc_Buff[1];						// チェックサムを加算する.
			
			for(us_Idx = 0; us_Idx < us_DataNum; us_Idx++){
				if( txQueue.isExist() == True ) {
					txQueue >> uc_Data;							// 送信データセット.
					uc_Buff[2+us_Idx] = uc_Data;				// Queから取り出したデータを設定.
					uc_DataSum = uc_DataSum + uc_Data;			// チェックサム加算.
					if(ul_dbgdatachk==1){setSharedMemLog( "startTransferSharedMemory() us_Idx[%d] uc_Data[%d] uc_DataSum[%d]\n" ,us_Idx,uc_Data,uc_DataSum);}
				} else {
					setSharedMemLog("startTransferSharedMemory() Que Num Error!!!\n" );
					b_SharedMemoryTransError = True;
					break;
				}
			}
			
									
			setSharedMemLog("startTransferSharedMemory:uc_DataSum=%d\n",uc_DataSum );
			uc_Buff[2+us_Idx] = uc_DataSum;						// チェックサムセット.

		
			if( b_SharedMemoryTransError == False ){
				//uc_TxPulseNoやgetWriteSharedMemoryOffsetAddressはインスタンスごとに設定されている.
				if( THRF_writeSharedMemory( THRD_SHAREDMEMORY_NO1, getWriteSharedMemoryOffsetAddress(), uc_Buff, us_Idx+3, uc_TxPulseNo ) < 0 ) {
					setSharedMemLog( "SYSF_writeSharedMemory() Error!!!\n" );
					b_SharedMemoryTransError = True;
				} else {
					// startTransferSharedMemory()が呼び出される場合、呼び出し元で、セマフォロックされるので、
					// このタイミングで共有メモリをセットしても問題ない.
					b_NowSendingSharedMemory1 = True;					// 共有メモリ１書込み中状態セット.
					ul_WriteSharedMemory1TickCnt = SYSF_tickGet();	// 共有メモリ１書込み時のTick時間セット.
					if( ul_WriteSharedMemory1TickCnt == 0 ) ul_WriteSharedMemory1TickCnt = 1;	// Tick時間補正.
				}
			}
		}
#else // defined(S800_VIDEO_MCPF).
		// 実機モード.
		uchar uc_Buff[THRD_SHAREDMEMORY_WORKDATA_SIZE];			// 送信バッファ。Zeus5Bkでは最大164バイト使用。
																// 今後のことを考え、メモリの最大サイズは1024バイト。それを送受信で分けるので512とする.
		short ss_DataNum = txQueue.isNum();						// 送信データＱｕｅｕｅ.
		uchar uc_Idx  = 0;										// 送信データＩｎｄｅｘ情報.
		
		setSharedMemLog("startTransferSharedMemory:ss_DataNum=%d\n",ss_DataNum );
		
		//us_SharedMemoryTransDataSize(データのサイズ)が2バイト以下(データサイズは最低3バイト必要)かチェック.
		if( THRD_SHAREDMEMORY_DATA_INFO_SIZE >= us_SharedMemoryTransDataSize ){
					setSharedMemLog( "startTransferSharedMemory() 1 us_SharedMemoryTransDataSize[%d]  Error!!!\n",us_SharedMemoryTransDataSize );
					b_SharedMemoryTransError = True;
		}
		
		// 最大転送データサイズが、255(ucharサイズ)を超えていないことをチェックする.
		// 255はあえて直値にしています。defineにしても結局charサイズである旨を記載することになるのであまり意味がない為。
		if( us_SharedMemoryTransDataSize > 255  ){
			setSharedMemLog( "startTransferSharedMemory() 2 us_SharedMemoryTransDataSize[%d]   Error!!!\n" ,us_SharedMemoryTransDataSize);
			b_SharedMemoryTransError = True;
		}
		
		if( ss_DataNum > (us_SharedMemoryTransDataSize - THRD_SHAREDMEMORY_DATA_INFO_SIZE) ) {
			setSharedMemLog( "startTransferSharedMemory() us_SharedMemoryTransDataSize[%d] ss_DataNum[%d]\n" ,us_SharedMemoryTransDataSize,ss_DataNum);
			ss_DataNum = us_SharedMemoryTransDataSize - THRD_SHAREDMEMORY_DATA_INFO_SIZE;			// 送信データ長ＭＡＸ補正（デフォルト：システム＝２０Ｂｙｔｅ／サブユニット＝１６４Ｂｙｔ）.
			// 先頭Ｂｙｔｅ：送信長／終端Ｂｙｔｅ：チェックサム情報を引いた（２Ｂｙｔｅ）レングスが送信データ数となる.
		}
		
				
		if( ss_DataNum > 0 ) {
			uchar uc_DataSum = 0x00;							// 送信データチェックサム情報.
			uchar uc_DataNum = 0;								// 型変換用uchar変数.
			uchar uc_Data = 0;									// データ廃棄用バッファ.
			
			uc_DataNum = (uchar)ss_DataNum;
			
			uc_Buff[0] = uc_DataNum;				// チェックサムの1バイト目にデータサイズを加算する.
													// ss_DataNumをuc_Buff[0]に入れるとキャストが発生する為、このような処理にしている.
			uc_DataSum += uc_DataNum;				// チェックサムを加算する.
			
			for(uc_Idx = 0; uc_Idx < uc_DataNum;uc_Idx++){
				if( txQueue.isExist() == True ) {
					txQueue >> uc_Data;							// 送信データセット.
					uc_Buff[1+uc_Idx] = uc_Data;				// Queから取り出したデータを
					uc_DataSum = uc_DataSum + uc_Data;			// チェックサム加算.
					if(ul_dbgdatachk==1){setSharedMemLog( "startTransferSharedMemory() uc_Idx[%d] uc_Data[%d] uc_DataSum[%d]\n" ,uc_Idx,uc_Data,uc_DataSum);}
				} else {
					setSharedMemLog("startTransferSharedMemory() Que Num Error!!!\n" );
					b_SharedMemoryTransError = True;
					break;
				}
			}
			
									
			setSharedMemLog("startTransferSharedMemory:uc_DataSum=%d\n",uc_DataSum );
			uc_Buff[1+uc_Idx] = uc_DataSum;						// チェックサムセット.

		
			if( b_SharedMemoryTransError == False ){
				//uc_TxPulseNoやgetWriteSharedMemoryOffsetAddressはインスタンスごとに設定されている.
				if( THRF_writeSharedMemory( THRD_SHAREDMEMORY_NO1, getWriteSharedMemoryOffsetAddress(), uc_Buff, uc_Idx+2, uc_TxPulseNo ) < 0 ) {
					setSharedMemLog( "SYSF_writeSharedMemory() Error!!!\n" );
					b_SharedMemoryTransError = True;
				} else {
					// startTransferSharedMemory()が呼び出される場合、呼び出し元で、セマフォロックされるので、
					// このタイミングで共有メモリをセットしても問題ない.
					b_NowSendingSharedMemory1 = True;					// 共有メモリ１書込み中状態セット.
					ul_WriteSharedMemory1TickCnt = SYSF_tickGet();	// 共有メモリ１書込み時のTick時間セット.
					if( ul_WriteSharedMemory1TickCnt == 0 ) ul_WriteSharedMemory1TickCnt = 1;	// Tick時間補正.
				}
			}
		}
#endif // defined(S800_VIDEO_MCPF).
	}
#endif // #if defined(TORNADO).
}

// ================================================================================================.
//! <!--機能--> 共有メモリバス送信完了処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//	14/03/05 忠政		 新規作成.
//	17/06/08 Sky		 S800 共有メモリ通信対応.
// ================================================================================================.
void CB_SIO_ImplAsic::endTransferSharedMemory()
{
#if defined(TORNADO)
	SYSF_semTake(semID, WAIT_FOREVER);
	setSharedMemLog( "endTransferSharedMemory:uc_TxPulseNo=[%d] uc_FunctionType=[%d]\n",uc_TxPulseNo,uc_FunctionType );

	// 実機モード.
	if( UART_FUNCTION_TYPE_SHAREDMEMORY == uc_FunctionType ) {
		switch( uc_TxPulseNo ) {
			case THRD_SHAREDMEMORY_PULSENO2: 					//ch2：CTL→MCPF 書込み完了.
			case THRD_SHAREDMEMORY_PULSENO4: 					//ch4：CTL→ENG 書込み完了.
			case THRD_SHAREDMEMORY_PULSENO6: 					//ch6：CTL→IR 書込み完了.
			case THRD_SHAREDMEMORY_PULSENO8: 					//ch8：CTL→DF 書込み完了.
				b_NowSendingSharedMemory1 = False;				// 共有メモリ１送信完了：次送信許可.
				ul_WriteSharedMemory1TickCnt = 0;				// 送信完了待ちTick時間.
				startTransferSharedMemory();					// 残データ送信.
				setSharedMemLog( "endTransferSharedMemory:PULSEN2,4,6,8\n" );
				break;
			case THRD_SHAREDMEMORY_PULSENO9: 					//ch9：F/W DL メモリ１書き込み完了.
				b_NowSendingSharedMemory1 = False;				// 共有メモリ１送信完了（Ｆ／Ｗ ＤＬ専用）.
				ul_WriteSharedMemory1TickCnt = 0;				// 送信完了待ちTick時間.
				setSharedMemLog( "endTransferSharedMemory:9\n" );
				break;
			case THRD_SHAREDMEMORY_PULSEN10: 					//ch10：F/W DL メモリ２書き込み完了ス.
				b_NowSendingSharedMemory2 = False;				// 共有メモリ２送信完了（Ｆ／Ｗ ＤＬ専用）.
				ul_WriteSharedMemory2TickCnt = 0;				// 送信完了待ちTick時間.
				setSharedMemLog( "endTransferSharedMemory:10\n" );
				break;
			default:
     		   // パルス番号が不正.
        		setSharedMemLog("endTransferSharedMemory() Not PULSENO Error!!!\n" );
        		b_SharedMemoryTransError = True;
				b_NowSendingSharedMemory1 = False;				// エラー時はFATALとなるが、念の為、フラグを落とす.
				break;
		}
	}else{
		// 共有メモリ通信でないのに、呼ばれたのでエラー.
		setSharedMemLog( "endTransferSharedMemory() Not SHAREDMEMORY Error!!!\n" );
		b_SharedMemoryTransError = True;
	}
	SYSF_semGive(semID);
#endif // #if defined(TORNADO).

}

// ================================================================================================.
//! <!--機能--> 共有メモリバス受信完了処理.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param なし.
//! <!--戻値-->.
//! \return なし.
//! <!--詳細-->.
//! \n.
//!	通信フォーマット
//!		第1バイト			  ：有効データサイズ().
//!		第2バイト～第N+1バイト：データ.
//!		第N+2バイト 		  ：第1バイト～第N+1バイトの加算値.
//!	チェックサム
//!		チェックサムは下4桁採用する.
//!		1バイトのデータが最大162個加算され、それにデータサイズ分の162さらに加算される.
//!		チェックサム値は4桁を超えることがない為、下4桁のみ取得する処理は不要.
//! <!--履歴-->.
//	14/03/05 忠政		 新規作成.
//	17/06/08 Sky		 S800 共有メモリ通信対応.
// ================================================================================================.
void CB_SIO_ImplAsic::receiveSharedMemory()
{
#if defined(TORNADO)
	SYSF_semTake(semID, WAIT_FOREVER);
	setSharedMemLog("receiveSharedMemory:uc_FunctionType=[%d]\n",uc_FunctionType );

	if( UART_FUNCTION_TYPE_SHAREDMEMORY == uc_FunctionType )
	{
		/* 2017/01/20 shortからint32_t変更 */
		int32_t ss_Ret = 0;									// Readの戻り値用.
		uchar uc_Data[THRD_SHAREDMEMORY_WORKDATA_SIZE]; 	// 読み出しデータ格納バッファ。Zeus5Bkでは最大164バイト使用。
															// メモリの最大サイズは1024バイト。それを送受信で分けるので512とする.
		
		// 第１引数：共有メモリ先頭アドレスからの読み出し先オフセットアドレス／第２引数：読み出しデータ格納ポインタ／第３引数：読み出し最大サイズ.
		ss_Ret = THRF_readSharedMemory( THRD_SHAREDMEMORY_NO1, getReadSharedMemoryOffsetAddress(), uc_Data, getTransDataSize(), getRxPulseNo() );

        if( ss_Ret == -1 ){
			setSharedMemLog("receiveSharedMemory() Data Read Error!!!\n" );
             b_SharedMemoryTransError = True;  
        }else if( ss_Ret > getTransDataSize() ){	//最大転送データサイズを超えていないか確認.
			setSharedMemLog("receiveSharedMemory() Data Size Error!!!\n" );
             b_SharedMemoryTransError = True;
        }
        
        if(b_SharedMemoryTransError==False){
			if( ss_Ret > THRD_SHAREDMEMORY_DATA_INFO_SIZE ) {							// データサイズとチェックサムとデータで3バイトは最低必要である為、2以上で比較する.
#if defined(S800_VIDEO_MCPF)
				ushort us_ReadDataSize = 0;											// 読み出しデータサイズ用.
				us_ReadDataSize = ((ushort)uc_Data[1] << 8) | (ushort)uc_Data[0];	// 1バイト目に格納されている受信データサイズ取得.
				uchar uc_ChkSum = uc_Data[0] + uc_Data[1];							// データサイズをチェックサムに加算.
					
				for( ushort us_cnt = 2; us_cnt <= (us_ReadDataSize+1); us_cnt++ ) {
					if(ul_dbgdatachk==1){setSharedMemLog("receiveSharedMemory:uc_Data=[%x]\n",uc_Data[us_cnt] );}
					rxQueue << uc_Data[us_cnt];					// 受信データ格納.
					
					uc_ChkSum = uc_ChkSum + uc_Data[us_cnt];
					if(ul_dbgdatachk==1){setSharedMemLog("receiveSharedMemory:uc_ChkSum=[%x]\n",uc_ChkSum );}
				}
				//加算したデータと読み出したデータのチェックサムを比較する.
				if( uc_ChkSum != uc_Data[us_ReadDataSize+2] ){
					setSharedMemLog("receiveSharedMemory() Check Sum Error!!![%x]!=[%x] \n",uc_ChkSum,uc_Data[us_ReadDataSize+2] );
					b_SharedMemoryTransError = True;												// チェックサム異常検知.
				}
#else // defined(S800_VIDEO_MCPF).
				uchar uc_ReadDataSize  = 0;				// 読み出しデータサイズ用.
				uc_ReadDataSize = uc_Data[0];			// 1バイト目に格納されている受信データサイズ取得.
				uchar uc_ChkSum = uc_Data[0];			// データサイズをチェックサムに加算.
					
				for( uchar uc_cnt = 1; uc_cnt <= uc_ReadDataSize; uc_cnt++ ) {
					if(ul_dbgdatachk==1){setSharedMemLog("receiveSharedMemory:uc_Data=[%x]\n",uc_Data[uc_cnt] );}
					rxQueue << uc_Data[uc_cnt];					// 受信データ格納.
					
					uc_ChkSum = uc_ChkSum + uc_Data[uc_cnt];
					if(ul_dbgdatachk==1){setSharedMemLog("receiveSharedMemory:uc_ChkSum=[%x]\n",uc_ChkSum );}
				}
				//加算したデータと読み出したデータのチェックサムを比較する.
				if( uc_ChkSum != uc_Data[uc_ReadDataSize+1] ){
					setSharedMemLog("receiveSharedMemory() Check Sum Error!!![%x]!=[%x] \n",uc_ChkSum,uc_Data[uc_ReadDataSize+1] );
					b_SharedMemoryTransError = True;												// チェックサム異常検知.
				}
#endif // defined(S800_VIDEO_MCPF).
			}else{
				setSharedMemLog("receiveSharedMemory() DataSize < 3  Error!!!\n" );
				b_SharedMemoryTransError = True;
			}
		}
	}else{
		setSharedMemLog("receiveSharedMemory() uc_FunctionType Error!!!\n" );
		b_SharedMemoryTransError = True;		
	}
	SYSF_semGive(semID);
#endif // #if defined(TORNADO).
}

//Str【CPS分離用MINET通信ログ : K.Natsume : 2014.01.08】
uint32_t CB_SIO_ImplAsic::getsioUARTCLOCK()
{
	return 1;
}
//End【CPS分離用MINET通信ログ : K.Natsume : 2014.01.08】

/*================================================================================================
［機能］	受信タスク起動
----------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし
			14/03/14 kurachi Zeus5BK ASIC777 UART変更対応.
			15/01/08 Sky	 S72基板対応 UART変更対応.
			15/10/06 Sky	 システム統合SoC(S800)対応.
			17/09/08 Sky戸田 S800 UART、GPIO対応.
==================================================================================================*/
void CB_SIO_ImplAsic::spawnRecieveTh()
{
#if defined(TORNADO)
// 実機モード.
	char* devFile = NULL;
	char* tskName = NULL;

	switch( uc_ChNo ){
#if defined(DEF_S800Emu) || !defined(DEF_DenebMLKCus) 
		case ASIC22_SIO_EDH:		/* EDHターミナル */
			devFile = (char*)"/dev/ttyEDH";
			tskName = (char*)"ReceiveUart2";
			break;
#endif // #if defined(DEF_S800Emu) || !defined(DEF_DenebMLKCus).
		case ASIC22_SIO_DSC1:		/* 地紋セキュリティ1 */
			// 2019/04/16 AR.1071437 突然フリーズ問題① DSC非装着の場合タスク生成しない.
			if(True == THRF_IsDsc1Exist()){
				devFile = (char*)"/dev/ttySEC1";
				tskName = (char*)"ReceiveUart3";
			}
			break;
		case ASIC22_SIO_DSC2:		/* 地紋セキュリティ2 */
			// 2019/04/16 AR.1071437 突然フリーズ問題① DSC非装着の場合タスク生成しない.
			if(True == THRF_IsDsc2Exist()){
				devFile = (char*)"/dev/ttySEC2";
				tskName = (char*)"ReceiveUart4";
			}
			break;
		case ASIC22_SIO_D103:		/* D103 */
			devFile = (char*)"/dev/ttyD103";
			tskName = (char*)"ReceiveUart5";
			break;
		default:
			/* M/CとIRターミナルは共有メモリ通信になったため無効化する */
			/* PICターミナルはターミナル乗っ取りモードで十分のため無効化する。*/
			/* MSCターミナルはIT5から無効化しており、IT6でも同様に無効化する */
			break;
	}

	if( devFile != NULL && fd == -1 ){
		fd = SYSF_open( devFile, O_RDWR, 0644 );
		THRL_ThAssert( fd != -1, "CB_SIO open ERR %d", errno , 0, 0, 0, CFA14_DetailCode0x0885 );

		// 受信タスクを起動する.
		THR_ID ret = SYSF_taskSpawn( tskName, 17, VX_FP_TASK, 8192, (FUNCPTR)ReceiveTh_ImplAsic, (intptr_t)this, 0, 0, 0, 0, 0, 0, 0, 0, 0 );
		THRL_ThAssert( ret != ERROR, "CB_SIO::CB_SIO() SYSF_taskSpawn() ERR\n" , 0, 0, 0, 0, CFA14_DetailCode0x0886 );
	}
#endif // #if defined(TORNADO).
}

/*================================================================================================
［機能］	過去最大使用キューサイズの表示（デバッグ用）
----------------------------------------------------------------------------------------------------
［引数］	なし
［戻値］	なし
==================================================================================================*/
void CB_SIO_ImplAsic::dumpUsedQueueNum()
{
	printf( " No.:%4d TxQueue:%8d RxQueue:%8d\n", uc_ChNo, txQueue.isNum(), rxQueue.isNum() );
}

// ================================================================================================.
//! <!--機能--> 送信キャラクタ取得（BIOS部からのコールバック）.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param	puc_Chr....⇒送信キャラクタ.
//! <!--戻値-->.
//! \return	True:データあり／False:データなし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//	07/03/23 冷水	 新規作成.
// ================================================================================================.
Bool CB_SIO_ImplAsic::getTxChar( uchar* puc_Chr )
{
	Bool	blRet = False;

	// 送信データ有無判定.
	if(txQueue.isExist()){
		// データセット.
		txQueue >> *puc_Chr;
		blRet = True;
	}
	return blRet;
}

// ================================================================================================.
//! <!--機能--> 受信キャラクタ設定（BIOS部からのコールバック）.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param	uc_Chr....受信キャラクタ.
//! <!--戻値-->.
//! \return	なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//	07/03/23 冷水	 新規作成.
// ================================================================================================.
void CB_SIO_ImplAsic::setRxChar( uchar uc_Chr )
{
	// データセット.
	rxQueue << uc_Chr;
}

// ================================================================================================.
//! <!--機能--> 受信エラー設定（BIOS部からのコールバック）.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param	uc_Err....エラー情報.
//! <!--戻値-->.
//! \return	なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//	07/03/23 冷水	 新規作成.
// ================================================================================================.
void CB_SIO_ImplAsic::setRxErr( uchar uc_Err )
{
	uint32_t	ulWork = 0;

	// フレーミングエラー.
	if(uc_Err & SIO_ERR_OVERRUN){
		ulWork |= SIO_LSR_OE;
	}
	// オーバランエラー.
	if(uc_Err & SIO_ERR_FLAMING){
		ulWork |= SIO_LSR_FE;
	}
	// パリティエラー.
	if(uc_Err & SIO_ERR_PARITY){
		ulWork |= SIO_LSR_PE;
	}
	// エラーカウント処理.
	c_ErrorCounter.regist(ulWork);
}

//Str ASIC777対応 : 谷(ICS) : 2013/10/29.
// ================================================================================================
//! <!--機能--> ファンクションタイプ設定.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param	uc_FuncType....ファンクションタイプ.
//! <!--戻値-->
//! \return	なし.
//! <!--詳細-->
//! \n
//! <!--履歴-->
//	2013/10/29 新規作成.
// ================================================================================================
void CB_SIO_ImplAsic::setFunctionType( uchar uc_FuncType )
{
	uc_FunctionType = uc_FuncType;
}

// ================================================================================================
//! <!--機能--> ファンクションタイプ取得.
//! <!--------------------------------------------------------------------------------------------->
//! <!--引数-->
//! \param	なし.
//! <!--戻値-->
//! \return	uchar ファンクションタイプ.
//! <!--詳細-->
//! \n
//! <!--履歴-->
//	2013/10/29 新規作成.
// ================================================================================================
uchar CB_SIO_ImplAsic::getFunctionType()
{
	return uc_FunctionType;
}
//End ASIC777対応 : 谷(ICS) : 2013/10/29.

// ================================================================================================.
//! <!--機能--> コンストラクタ.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param	c_sioImpl....シリアルポート用インターフェース インスタンス.
//! <!--戻値-->.
//! \return	なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//	07/03/23 冷水	 新規作成.
// ================================================================================================.
CB_SIO::CB_SIO(	CB_SIO_Impl& c_sioImpl )
		: c_sioImpl(c_sioImpl)
		, txQueue(c_sioImpl.txQueue)
		, rxQueue(c_sioImpl.rxQueue)
		, c_ErrorCounter(c_sioImpl.c_ErrorCounter)
{
}

// ================================================================================================.
//! <!--機能--> デストラクタ.
//! <!--------------------------------------------------------------------------------------------->.
//! <!--引数-->.
//! \param	なし.
//! <!--戻値-->.
//! \return	なし.
//! <!--詳細-->.
//! \n.
//! <!--履歴-->.
//	07/03/23 冷水	 新規作成.
// ================================================================================================.
CB_SIO::~CB_SIO()
{
}

#ifdef SIMULATOR
// not support.
/*================================================================================================
［クラス］	CB_RTC_LOG : リアルタイム・クロック　ログ
 2019/04/02 Sky 【EagleL】品証問題:時計がずれる問題の仕込みログ対応.
================================================================================================*/
class CB_RTC_LOG {
private:
	CB_RTC_LOG() { }
public:
	// ログ保存.
	void save( const CT_Time& now_time, const CT_Time& update_time ) {};
	// ログ出力.
	void print( void ) { }
	// インスタンス(Singleton).
	static CB_RTC_LOG& newInstance( void ) {
		static CB_RTC_LOG instance;
		return instance;
	}
};
#else	// #ifdef SIMULATOR.
/*================================================================================================
［クラス］	CB_RTC_LOG : リアルタイム・クロック　ログ
 2019/04/02 Sky 【EagleL】品証問題:時計がずれる問題の仕込みログ対応.
================================================================================================*/
#include <algorithm>
#include <vector>
extern void bt_saveFile( int fd );
extern void flushDbgStdouttoUSBFile();
// output.
#define CB_RTC_LOG_ERR(fmt, ...) printf( "(%010u)(BSP)[CB_RTC_LOG::%s] " fmt, SYSF_tickGet(), __FUNCTION__, ##__VA_ARGS__ )
//#define CB_RTC_LOG_DBG(fmt, ...) CB_RTC_LOG_ERR( fmt, ##__VA_ARGS__ )
#define CB_RTC_LOG_DBG(fmt, ...)
// define.
#define RTC_LOG_DIRECTORY		"/ata0a/RtcLog"		// ログディレクトリ.
#define RTC_LOG_FILE_NAME_EXT	".log"				// ログファイル拡張子.
#define RTC_LOG_FILE_NAME_FMT	"%"PRIu32".log"		// ログファイル名.
#define RTC_LOG_FILES_MAX		10					// ログ最大件数.
#define RTC_LOG_PATH_NAME_MAX	40					// ログパス名最大.
// class.
class CB_RTC_LOG {
private:
	// member.
	std::vector<uint32_t>	m_lstLogIndex;				// ログのインデックスリスト
	SEM_ID					m_semID;					// セマフォ.

	// インデックスからのログファイル名変換.
	void getLogPathName( uint32_t index, char *buf, int buf_size ) {
		snprintf( buf, buf_size, RTC_LOG_DIRECTORY "/" RTC_LOG_FILE_NAME_FMT, index );
	}

	// ファイル名からインデックス変換.
	uint32_t convertFilenameToLogIndex( const char* name ) {
		uint32_t val = 0;
		while ( '\0' != *name && *name != '.' ) {
			if ( ( !isdigit(*name) )				// 0-9以外.
			  || ( '0' == *name && 0 == val ) ) {	// 0はじまり.
				return 0;
			}
			val = (val * 10) + (*name - '0');
			name++;
		}
		// 拡張子のチェック.
		if ( 0 == strcmp( name, RTC_LOG_FILE_NAME_EXT ) ) {
			return val;
		}
		return 0;
	}

	// ログファイルリストの読込み.
	void load( void ) {
		CB_RTC_LOG_DBG( "start \n" );
		m_lstLogIndex.clear();
		// ログリストの読込み.
		DIR *pDir = SYSF_Opendir( RTC_LOG_DIRECTORY );
		if ( NULL != pDir ) {
			struct dirent *pDirent;
			struct dirent  st_Dirent;
			// ディレクトリ内のエントリーをループ.
			while ( SYSF_Readdir_r( pDir, &st_Dirent, &pDirent ) == 0 ) {
				if ( NULL == pDirent ) {
					break;
				}
				CB_RTC_LOG_DBG( "readdir file:%s\n", pDirent->d_name );
				uint32_t index = convertFilenameToLogIndex( pDirent->d_name );
				if ( 0 != index ) {
					CB_RTC_LOG_DBG( "list add :%d\n", index );
					m_lstLogIndex.push_back( index );
				} else {
					CB_RTC_LOG_DBG( "ignore: %s\n", pDirent->d_name );
				}
			}
			// クローズ.
			SYSF_Closedir( pDir );
		}
		else {
			CB_RTC_LOG_DBG( "mkdir %s\n", RTC_LOG_DIRECTORY );
			// ディレクトリ作成.
			if ( ERROR == SYSF_Mkdir( RTC_LOG_DIRECTORY ) ) {
				CB_RTC_LOG_ERR( "error: mkdir %s\n", RTC_LOG_DIRECTORY );
			}
		}
		// ソート.
		std::sort( m_lstLogIndex.begin(), m_lstLogIndex.end() );
		// 古いログファイルを削除.
		removeOldLog();
		CB_RTC_LOG_DBG( "end \n" );
	}

	// 新しいログファイルオープンする.
	int openNewLog() {
		CB_RTC_LOG_DBG( "start \n" );
		int new_fd;
		uint32_t new_index = 1;
		if ( 0 < m_lstLogIndex.size() ) {
			new_index = m_lstLogIndex.back() + 1;
		}
		char new_name[RTC_LOG_PATH_NAME_MAX] = {0};
		getLogPathName( new_index, new_name, sizeof(new_name) );
		CB_RTC_LOG_DBG( "open %s\n", new_name );
		new_fd = SYSF_Open( new_name, O_CREAT | O_RDWR, 0666 );
		if ( 0 > new_fd ) {
			CB_RTC_LOG_ERR( "error: open %s\n", new_name );
		} else {
			CB_RTC_LOG_DBG( "push_back %d\n", new_index );
			m_lstLogIndex.push_back( new_index );
		}
		CB_RTC_LOG_DBG( "end %d\n", new_fd );
		return new_fd;
	}

	// 古いログファイルを削除する.
	void removeOldLog() {
		CB_RTC_LOG_DBG( "start \n" );
		Bool sync_flg = False;
		// 上限を超える場合は、削除する.
		while ( RTC_LOG_FILES_MAX < m_lstLogIndex.size() ) {
			uint32_t rm_index = m_lstLogIndex.front();
			char rm_name[RTC_LOG_PATH_NAME_MAX] = {0};
			getLogPathName( rm_index, rm_name, sizeof(rm_name) );
			CB_RTC_LOG_DBG( "remove %s\n", rm_name );
			if ( ERROR == SYSF_Remove( rm_name ) ) {
				CB_RTC_LOG_ERR( "error: remove %s\n", rm_name );
			}
			m_lstLogIndex.erase( m_lstLogIndex.begin() );	// 先頭削除.
			sync_flg = True;
		}
		// sync.
		if ( sync_flg ) {
			CB_RTC_LOG_DBG( "sync\n" );
			sync();
		}
		CB_RTC_LOG_DBG( "end \n" );
	}

	// コンストラクタ.
	CB_RTC_LOG() : m_lstLogIndex( RTC_LOG_FILES_MAX * 2, 0 ) {
		CB_RTC_LOG_DBG( "start \n" );
		// セマフォ生成.
		m_semID = SYSF_semBCreate( SEM_Q_FIFO, SEM_FULL );
		if ( NULL == m_semID ) {
			CB_RTC_LOG_ERR( "error: sem create\n" );
		}
		CB_RTC_LOG_DBG( "end \n" );
	}

	// ロック.
	void lock( void ) {
		SYSF_semTake( m_semID, WAIT_FOREVER );
	}

	// アンロック.
	void unlock( void ) {
		SYSF_semGive( m_semID );
	}

public:
	// ログ保存.
	void save( const CT_Time& now_time, const CT_Time& update_time ) {
		CB_RTC_LOG_DBG( "start \n" );
		lock();	// ロック.

		load();	// ログファイルリストの読込み.

		// 新しいログファイル.
		int fd = openNewLog();
		if( fd < 0 ) {
			unlock();	// アンロック.
			return;
		}

		char wbuf[64];
		int wsize = 0;
		// システム時刻.
		memset( wbuf, 0, sizeof(wbuf) );
		time_t sys_time = 0;
		struct tm sys_tm = {};
		sys_time = time( NULL );
		SYSF_localtime_r( &sys_time, &sys_tm );
		wsize = snprintf( wbuf, sizeof(wbuf), "system now: %04d/%02d/%02d %02d:%02d:%02d\n",
						 sys_tm.tm_year + 1900, sys_tm.tm_mon + 1, sys_tm.tm_mday,
						 sys_tm.tm_hour, sys_tm.tm_min, sys_tm.tm_sec );
		CB_RTC_LOG_DBG( "%s", wbuf );
		if ( 0 > SYSF_Write( fd, wbuf, (size_t)wsize ) ) {
			CB_RTC_LOG_ERR( "error: write system time, errno:%d\n", SYSF_errnoGet() );
		}

		// 現在時刻(RTC).
		memset( wbuf, 0, sizeof(wbuf) );
		wsize = snprintf( wbuf, sizeof(wbuf), "rtc now   : %04d/%02d/%02d %02d:%02d:%02d\n",
						 now_time.year, now_time.month, now_time.day,
						 now_time.hour, now_time.minute, now_time.second );
		CB_RTC_LOG_DBG( "%s", wbuf );
		if ( 0 > SYSF_Write( fd, wbuf, (size_t)wsize ) ) {
			CB_RTC_LOG_ERR( "error: write rtc now time, errno:%d\n", SYSF_errnoGet() );
		}

		// 更新時刻(RTC).
		memset( wbuf, 0, sizeof(wbuf) );
		wsize = snprintf( wbuf, sizeof(wbuf), "rtc update: %04d/%02d/%02d %02d:%02d:%02d\n",
						 update_time.year, update_time.month, update_time.day,
						 update_time.hour, update_time.minute, update_time.second );
		CB_RTC_LOG_DBG( "%s", wbuf );
		if ( 0 > SYSF_Write( fd, wbuf, (size_t)wsize ) ) {
			CB_RTC_LOG_ERR( "error: write rtc update time, errno:%d\n", SYSF_errnoGet() );
		}

		// バックトレース出力.
		memset( wbuf, 0, sizeof(wbuf) );
		wsize = snprintf( wbuf, sizeof(wbuf), "call trace:\n" );
		if ( 0 > SYSF_Write( fd, wbuf, (size_t)wsize ) ) {
			CB_RTC_LOG_ERR( "error: write call trace, errno:%d\n", SYSF_errnoGet() );
		}
		CB_RTC_LOG_DBG( "bt_saveFile()\n" );
		bt_saveFile( fd );

		// fsync & close.
		CB_RTC_LOG_DBG( "sync & close\n" );
		SYSF_fsync( fd );
		SYSF_Close( fd );

		// 古いログの削除.
		removeOldLog();

		unlock();	// アンロック.

		CB_RTC_LOG_DBG( "end \n" );
	}

	// ログ出力.
	void print( void ) {
		lock();	// ロック.

		load();	// ログファイルリストの読込み.

		printf( "========== BSP ==========\n" );
		printf( " RTC Update Log.\n" );
		printf( "========== BSP ==========\n" );

		std::vector<uint32_t>::const_iterator ite = m_lstLogIndex.begin();
		while( ite != m_lstLogIndex.end() ) {
			char prt_name[ RTC_LOG_PATH_NAME_MAX ] = {0};
			char cmd[ 8 + RTC_LOG_PATH_NAME_MAX ] = {0};
			getLogPathName( *ite, prt_name, sizeof(prt_name) );
			printf( "File Name: %s\n", prt_name );
			snprintf( cmd, sizeof(cmd), "cat %s", prt_name );
			flushDbgStdouttoUSBFile();
			system( cmd );
			flushDbgStdouttoUSBFile();
			printf( "------\n" );
			ite++;
		}

		printf("\n\n");

		unlock();	// アンロック.
	}

	// インスタンス(Singleton).
	static CB_RTC_LOG& newInstance( void ) {
		static CB_RTC_LOG instance;
		return instance;
	}
};
#endif	// #ifdef SIMULATOR.

// ログ出力.
extern "C" void printRtcLog( void )
{
	CB_RTC_LOG::newInstance().print();
}


/*================================================================================================
［クラス］	CB_RTC : リアルタイム・クロック
================================================================================================*/
/*================================================================================================
［機能］	初期化
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	実施済み：2000/7/26：出山
［履歴］	06/02/10：morikawa MoselのRTC正式対応。usrCppInit.cのsysRtcInitで行うので、ここでは何もしない。
================================================================================================*/
void CB_RTC::init()
{
}
/*================================================================================================
［機能］	読み込み
--------------------------------------------------------------------------------------------------
［引数］	CT_Time& time : 時刻（年・月・日・時間）
［戻値］	int : RTC時刻取得成功 0、エラー -1
［ｳｫｰｸｽﾙｰ］	実施済み：2000/7/26：出山
［履歴］	06/02/10：morikawa MoselのRTC正式対応。sysLib.cのsysRtcSetの処理をそのまま持ってくる。
			06/03/07 morikawa RTC-703の仕様に従った制御に修正　不要なコメントアウト処理を削除 
			06/07/19：R.Suzuki 64498 RTCの設定変更ができない対策 NVRAM Writeプロテクト制御の対応 
			07/03/05：R.Suzuki intLock中にsemTakeしてしまうのでintLockをやめる対応 
			12/10/21：堀井 CQ.374256  RTCリードエラー追加対応.
================================================================================================*/
int32_t CB_RTC::read( CT_Time& ct_time )
{
#ifndef TORNADO		// ＰＣシミュレータで有効.
	time_t now = 0;
	struct tm tim;
	time( (time_t*)&now );
	SYSF_localtime_r( &now, &tim );
	ct_time.year   = (short)tim.tm_year;	// 1970-2038.
	ct_time.month  = (uchar)tim.tm_mon;	// 1-12.
	ct_time.day    = (uchar)tim.tm_mday;	// 1-31.
	ct_time.hour   = (uchar)tim.tm_hour;	// 0-23.
	ct_time.minute = (uchar)tim.tm_min;	// 0-59.
	ct_time.second = (uchar)tim.tm_sec;	// 0-59.
	return 0;
#else				// VXWORKSで有効.
	struct rtc_time rtc;
	int32_t ret;
#ifndef COSIM	 //2010/11/26 COSIM対応 Start.
 	ushort retry = 0;
	static ushort retry_ext = 0;	// 2012/10/21 CQ.374256 堀井 RTCリードエラー追加対応.
	if(True == bl_isDriverAvailable){
	do {
		ret = SYSF_ioctl( rtc_fd, RTC_RD_TIME, (intptr_t)&rtc );
		retry++;
		if( ret == -1 ) {
			logMsg( "@@ %"PRId32" : CB_RTC::%s - retry %d\n", SYSF_tickGet(), (uintptr_t)__FUNCTION__, (uintptr_t)retry, (uintptr_t)4, (uintptr_t)5, (uintptr_t)6 );
		}
	} while( ret == -1 && retry < DEF_CB_RTC_RETRY_MAX );
	// 2012/10/21 CQ.374256 堀井 RTCリードエラー追加対応 start.
	if(ret == -1){	// リードに失敗した場合.
		retry_ext++;
		THRL_ThAssert( (retry_ext < 3) && bl_isInitializeEndFlag, "ioctl RTC_RD_TIME ERR errno:%d", SYSF_errnoGet(), 0, 0, 0 , CFA14_DetailCode0x0767 );	// 3回目のリトライ失敗でFA14.
		return -1;
	}else{			// リードに成功した場合、リトライ回数を0にする.
		retry_ext = 0;
	}
	// 2012/10/21 CQ.374256 堀井 RTCリードエラー追加対応 end.
	ct_time.year   = (short)(rtc.tm_year + 1900);	// 1970-2038.
	ct_time.month  = (uchar)(rtc.tm_mon + 1);		// 1-12.
	ct_time.day    = (uchar)rtc.tm_mday;			// 1-31.
	ct_time.hour   = (uchar)rtc.tm_hour;			// 0-23.
	ct_time.minute = (uchar)rtc.tm_min;				// 0-59.
	ct_time.second = (uchar)rtc.tm_sec;				// 0-59.
	}else{
		ct_time.year   = (short)2017;	// 1970-2038.
		ct_time.month  = (uchar)12;		// 1-12.
		ct_time.day    = (uchar)24;		// 1-31.
		ct_time.hour   = (uchar)20;		// 0-23.
		ct_time.minute = (uchar)30;		// 0-59.
		ct_time.second = (uchar)40;		// 0-59.
	}
	return 0;
#else
	ct_time.year   = (short)2010;	// 1970-2038.
	ct_time.month  = (uchar)1;		// 1-12.
	ct_time.day    = (uchar)10;		// 1-31.
	ct_time.hour   = (uchar)20;		// 0-23.
	ct_time.minute = (uchar)30;			// 0-59.
	ct_time.second = (uchar)40;			// 0-59.
	return 0;
#endif //2010/11/09  COSIM対応 End.
#endif // #ifdef TORNADO.
}

/*================================================================================================
［機能］	現在時刻の取得
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	CT_Time : 時刻（年・月・日・時間）
［ｳｫｰｸｽﾙｰ］	実施済み：2000/7/26：出山
--------------------------------------------------------------------------------------------------
［履歴］	2001/12/26 澤柳 bootromにてコンパイルがとおらないための措置
================================================================================================*/
CT_Time CB_RTC::getNowTime()
{
	CT_Time time;
	read( time );	// 読み込み.
	return	time;
}

/*================================================================================================
［機能］	現在時刻の取得（時・分）
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	CT_HourMinite : 時・分
［ｳｫｰｸｽﾙｰ］	実施済み：2000/7/26：出山
--------------------------------------------------------------------------------------------------
［履歴］	2001/12/26 澤柳 bootromにてコンパイルがとおらないための措置
================================================================================================*/
CT_HourMinite CB_RTC::getNowHourMinute()
{
	CT_Time time;
	read( time );	// 読み込み.
	return	CT_HourMinite( time.hour, time.minute );
}

/*================================================================================================
［機能］	書き込み
--------------------------------------------------------------------------------------------------
［引数］	CT_Time& time : 時刻（年・月・日・時間）
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	実施済み：2000/7/26：出山
［履歴］	06/02/10：morikawa MoselのRTC正式対応。sysLib.cのsysRtcSetの処理をそのまま持ってくる。
			06/03/07：morikawa RTC-703の仕様に従った制御に修正　不要なコメントアウト処理を削除　
			06/07/19：R.Suzuki 64498 RTCの設定変更ができない対策 NVRAM Writeプロテクト制御の対応 
			11/10/14: 河村 CQ356781 RTCはエラーを返す事があるのでリトライを行う.
			2019/04/02 Sky 【EagleL】品証問題:時計がずれる問題の仕込みログ対応.
================================================================================================*/
void CB_RTC::write( CT_Time& ct_time )
{
#ifndef TORNADO		// ＰＣシミュレータで有効.
	ct_time.second = 0;	// For Warning.
	// ＰＣシミュレータでは何も行わない.
#else				// VXWORKSで有効.
	struct rtc_time rtc;
	int32_t ret;
	ushort retry = 0;
	if( False == bl_isDriverAvailable ){ return; }
	rtc.tm_year = ct_time.year - 1900;
	rtc.tm_mon = ct_time.month - 1;
	rtc.tm_mday = ct_time.day;
	rtc.tm_hour = ct_time.hour;
	rtc.tm_min = ct_time.minute;
	rtc.tm_sec = ct_time.second;
#ifndef COSIM	 //2010/11/26 COSIM対応 Start.

	// 2019/04/02 Sky 【EagleL】品証問題:時計がずれる問題の仕込みログ対応.
	CT_Time now_time;
	read( now_time );	// 読み込み.
	CB_RTC_LOG::newInstance().save( now_time, ct_time );		// ログ保存.

	// 曜日算出.	// 2015/11/06 Harada CQ.519898 曜日(tm_wday)が設定されていないため、曜日の算出を行う.
	time_t tt = 0;
	struct tm st_tm;
	memset(&st_tm, 0, sizeof(st_tm));
	st_tm.tm_year  = rtc.tm_year;
	st_tm.tm_mon   = rtc.tm_mon;
	st_tm.tm_mday  = rtc.tm_mday;
	st_tm.tm_hour  = rtc.tm_hour;
	st_tm.tm_min   = rtc.tm_min;
	st_tm.tm_sec   = rtc.tm_sec;
	st_tm.tm_isdst = -1;
	// 日付・時刻の再計算.
	tt = mktime(&st_tm);
	if(-1 != tt){
		rtc.tm_wday = st_tm.tm_wday;	// 曜日 [0:日 1:月 ... 6:土].
	} else {
		printf("@@ %s mktime ERR errno:%d\n", __FUNCTION__, SYSF_errnoGet());
	}

	do {
		ret = SYSF_ioctl( rtc_fd, RTC_SET_TIME, (intptr_t)&rtc );
		retry++;
		if( ret == -1 ) {
			logMsg( "@@ %"PRId32" : CB_RTC::%s - retry %d\n", SYSF_tickGet(), (uintptr_t)__FUNCTION__, (uintptr_t)retry, (uintptr_t)4, (uintptr_t)5, (uintptr_t)6 );
		}
	} while( ret == -1 && retry < DEF_CB_RTC_RETRY_MAX );
	THRL_ThAssert( ret != -1, "ioctl RTC_SET_TIME ERR errno:%d", SYSF_errnoGet(), 0, 0, 0 , CFA14_DetailCode0x0768 );
#endif 	 //2010/11/26 COSIM対応 End.
#endif	// #ifdef TORNADO.
	setBackUpTime(True , False);			// 2022/09/22 OP_BTS-39386 山口 RTC時刻初期化【暫定】.
}

static void    add_sec_to_rtc_time(struct rtc_time *rtc_tm, uint32_t sec);
static void    calcAddDate( struct rtc_time *tStartDT, int32_t iSpan, struct rtc_time *tEndDT );
static Bool    checkUruuYear( int32_t iYear );

/*================================================================================================
［機能］	アラーム設定
--------------------------------------------------------------------------------------------------
［引数］	long return_time : RTCへ設定する復帰時間
［戻値］	無し
［ｳｫｰｸｽﾙｰ］ 未実施
［履歴］	10/07/30 kurachi Minerva初版.
			11/10/14 河村 CQ356781 RTCはエラーを返す事があるのでリトライを行う.
================================================================================================*/
void CB_RTC::setAlarm(int32_t return_time)
{
// Str 2010/09/13 ft_kondo 8536環境Sim対応.
#ifdef TORNADO			// 実機で有効.
	struct rtc_time rtc_tm;
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	struct rtc_time rtc_tm_rd;		/* 2019/07/09 OP_BTS-5418 Harada 【詳細コード対応】076A 新詳細コード対応 */
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
#endif // #ifndef TORNADO.
// End 2010/09/13 ft_kondo 8536環境Sim対応.
	if( return_time < 0 ) {	/* 2020/06/11 OP_BTS-24850 河村 - FA14(076A)対策 */
		printf( "%s : warning! return_time = %d\n", __FUNCTION__, return_time );
		return_time = 2147483647;	/* 32bit max(0x7fffffff) */
	}
// Str 2019/10/25 日立 Thread OP_BTS-15330 オーバーフロー対応.
	int32_t sec = return_time/200;    // * 5 /1000; Tick→秒.
// End 2019/10/25 日立 Thread OP_BTS-15330 オーバーフロー対応.
// Str 2010/09/13 ft_kondo 8536環境Sim対応.
#ifdef TORNADO			// 実機で有効.
	ushort retry = 0;
	int32_t ret;
	if( False == bl_isDriverAvailable ){ return; }
	/************************************************************
	現在の RTC 時刻を読み出して rtc_tm にセットする。
	************************************************************/
	do {
		ret = SYSF_ioctl( rtc_fd, RTC_RD_TIME, (intptr_t)&rtc_tm );
		retry++;
		if( ret == -1 ) {
			logMsg( "@@ %"PRId32" : CB_RTC::%s 1 - retry %d\n", SYSF_tickGet(), (uintptr_t)__FUNCTION__, (uintptr_t)retry, (uintptr_t)4, (uintptr_t)5, (uintptr_t)6 );
		}
	} while( ret == -1 && retry < DEF_CB_RTC_RETRY_MAX );
	THRL_ThAssert( ret != -1, "ioctl RTC_RD_TIME ERR errno:%d", SYSF_errnoGet(), 0, 0, 0 , CFA14_DetailCode0x0769 );
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	rtc_tm_rd = rtc_tm;		/* 2019/07/09 OP_BTS-5418 Harada 【詳細コード対応】076A 新詳細コード対応 */
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).

	/************************************************************
	rtc_tm の値を sec秒進める。
	************************************************************/
	add_sec_to_rtc_time(&rtc_tm, sec);

	/************************************************************
	sec 秒後にアラームが発生するように設定する。
	************************************************************/
	retry = 0;
	do {
		ret = SYSF_ioctl( rtc_fd, RTC_ALM_SET, (intptr_t)&rtc_tm);
		retry++;
		if( ret == -1 ) {
			logMsg( "@@ %"PRId32" : CB_RTC::%s 2 - retry %d\n", SYSF_tickGet(), (uintptr_t)__FUNCTION__, (uintptr_t)retry, (uintptr_t)4, (uintptr_t)5, (uintptr_t)6 );
		}
	} while( ret == -1 && retry < DEF_CB_RTC_RETRY_MAX );
	/* 2019/07/09 OP_BTS-5418 Harada 【詳細コード対応】076A 新詳細コード対応 */
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	if ( ret == -1 ) {
		int32_t si_errno = SYSF_errnoGet();		// errno.
		SYSF_logMsg( "(%010u)(BSP) @@ : CB_RTC::%s ioctl RTC_ALM_SET ERR  errno:%d, return_time:%d\n", SYSF_tickGet(), __FUNCTION__, si_errno, return_time );
		SYSF_logMsg( "(%010u)(BSP) @@ : CB_RTC::%s ioctl RTC_ALM_SET ERR  RTC_RD_TIME tm_year:%d, tm_mon:%d, tm_mday:%d\n", SYSF_tickGet(), __FUNCTION__, rtc_tm_rd.tm_year, rtc_tm_rd.tm_mon, rtc_tm_rd.tm_mday );
		SYSF_logMsg( "(%010u)(BSP) @@ : CB_RTC::%s ioctl RTC_ALM_SET ERR  RTC_RD_TIME tm_hour:%d, tm_min:%d, tm_sec :%d\n", SYSF_tickGet(), __FUNCTION__, rtc_tm_rd.tm_hour, rtc_tm_rd.tm_min, rtc_tm_rd.tm_sec );
		SYSF_logMsg( "(%010u)(BSP) @@ : CB_RTC::%s ioctl RTC_ALM_SET ERR  RTC_ALM_SET tm_year:%d, tm_mon:%d, tm_mday:%d\n", SYSF_tickGet(), __FUNCTION__, rtc_tm.tm_year, rtc_tm.tm_mon, rtc_tm.tm_mday );
		SYSF_logMsg( "(%010u)(BSP) @@ : CB_RTC::%s ioctl RTC_ALM_SET ERR  RTC_ALM_SET tm_hour:%d, tm_min:%d, tm_sec :%d\n", SYSF_tickGet(), __FUNCTION__, rtc_tm.tm_hour, rtc_tm.tm_min, rtc_tm.tm_sec );

		uint64_t ull_NewDetailCode = 0x0;
		Bool bl_ret = True;
		ull_NewDetailCode = ((((uint64_t)0x22					<< 56) & 0xff00000000000000)		// モジュール番号.
						   | (((uint64_t)0x01b					<< 44) & 0x00fff00000000000)		// 通し番号.
						   | (((uint64_t)si_errno				<< 36) & 0x00000ff000000000)		// errno.
						   | (((uint64_t)rtc_tm_rd.tm_year		<< 28) & 0x0000000ff0000000)		// RTC時刻を読み出し後の rtc_time構造体 tm_year 年 - 1900          RTC日付レジスタ(RTC_DR)の上位16bit確認用.
						   | (((uint64_t)rtc_tm_rd.tm_mday 		<< 20) & 0x000000000ff00000)		// RTC時刻を読み出し後の rtc_time構造体 tm_mday 月内の日付 (1-31)  RTC日付レジスタ(RTC_DR)の下位16bit確認用.
						   | (((uint64_t)rtc_tm_rd.tm_hour 		<< 12) & 0x00000000000ff000)		// RTC時刻を読み出し後の rtc_time構造体 tm_hour 時間 (0-23)        RTC時刻レジスタ(RTC_TR)の上位16bit確認用.
						   | (((uint64_t)rtc_tm_rd.tm_min		<<  4) & 0x0000000000000ff0));		// RTC時刻を読み出し後の rtc_time構造体 tm_min  分 (0-59)          RTC時刻レジスタ(RTC_TR)の下位16bit確認用.
		SYSF_logMsg( "(%010u)(BSP) ## ioctl RTC_ALM_SET ERR NewDetailCode:%#llx\n", SYSF_tickGet() , ull_NewDetailCode );
		bl_ret = APIC_setNewDetailCode( TYPE_FatalDetailCode_undefined, ull_NewDetailCode );		// 詳細コード登録.
		if (False == bl_ret) {
			SYSF_logMsg( "(%010u)(BSP) ## %s %d : APIC_setNewDetailCode NG\n", SYSF_tickGet() , __FUNCTION__, __LINE__, 3,4,5,6 );
		}
	}
#endif	// #if defined(TORNADO) && !defined(FUM_LINUX).
	THRL_ThAssert( ret != -1, "ioctl RTC_ALM_SET ERR errno:%d", SYSF_errnoGet(), 0, 0, 0 , CFA14_DetailCode0x076A );
#endif // #ifndef TORNADO.
// End 2010/09/13 ft_kondo 8536環境Sim対応.
}

/*================================================================================================
［機能］	アラーム割り込み設定
--------------------------------------------------------------------------------------------------
［引数］	Bool bl_Enable : 割り込み（有効：True、無効：False）
［戻値］	無し
［ｳｫｰｸｽﾙｰ］ 未実施
［履歴］	10/07/30 kurachi Minerva初版.
			11/10/14 河村 CQ356781 RTCはエラーを返す事があるのでリトライを行う.
================================================================================================*/
void CB_RTC::setAlarmInterrupt( Bool bl_Enable )
{
// Str 2010/09/13 ft_kondo 8536環境Sim対応.
#ifdef TORNADO			// 実機で有効.
	int32_t ret;
#endif // #ifndef TORNADO.
// End 2010/09/13 ft_kondo 8536環境Sim対応.
	if( False == bl_isDriverAvailable ){ return; }
	if( bl_Enable == True ){
// Str 2010/09/13 ft_kondo 8536環境Sim対応.
#ifdef TORNADO			// 実機で有効.
		ushort retry = 0;
		do {
			ret = SYSF_ioctl( rtc_fd, RTC_AIE_ON, 0);
			retry++;
			if( ret == -1 ) {
				logMsg( "@@ %"PRId32" : CB_RTC::%s - retry %d\n", SYSF_tickGet(), (uintptr_t)__FUNCTION__, (uintptr_t)retry, (uintptr_t)4, (uintptr_t)5, (uintptr_t)6 );
			}
		} while( ret == -1 && retry < DEF_CB_RTC_RETRY_MAX );
		THRL_ThAssert( ret != -1, "ioctl RTC_AIE_ON ERR errno:%d", SYSF_errnoGet(), 0, 0, 0 , CFA14_DetailCode0x076B );
#endif // #ifndef TORNADO.
// End 2010/09/13 ft_kondo 8536環境Sim対応.
	}else{
// Str 2010/09/13 ft_kondo 8536環境Sim対応.
#ifdef TORNADO			// 実機で有効.
		ushort retry = 0;
		do {
			ret = SYSF_ioctl( rtc_fd, RTC_AIE_OFF, 0);
			retry++;
			if( ret == -1 ) {
				logMsg( "@@ %"PRId32" : CB_RTC::%s - retry %d\n", SYSF_tickGet(), (uintptr_t)__FUNCTION__, (uintptr_t)retry, (uintptr_t)4, (uintptr_t)5, (uintptr_t)6 );
			}
		} while( ret == -1 && retry < DEF_CB_RTC_RETRY_MAX );
		THRL_ThAssert( ret != -1, "ioctl RTC_AIE_OFF ERR errno:%d", SYSF_errnoGet(), 0, 0, 0 , CFA14_DetailCode0x076C );
#endif // #ifndef TORNADO.
// End 2010/09/13 ft_kondo 8536環境Sim対応.
	}
}

/*================================================================================================
［機能］	初期化完了設定
--------------------------------------------------------------------------------------------------
［引数］	Bool bl_flag : 初期化完了フラグ（完了：True、未完了：False）
［戻値］	無し
［ｳｫｰｸｽﾙｰ］ 未実施
［履歴］	2012/10/21 堀井 新規作成 CQ.374256
================================================================================================*/
void CB_RTC::setInitializeEndFlag( Bool bl_flag )
{
	bl_isInitializeEndFlag = bl_flag;
}

// 2022/09/22 OP_BTS-39386 山口 RTC時刻初期化【暫定】.
/*================================================================================================
［機能］	RTC時刻のバックアップ.	
--------------------------------------------------------------------------------------------------
［引数］	bl_update  True : 強制的に更新 / False : 前回更新から1時間経過で更新.
			bl_backup_check	True : バックアップ時刻とRTC時刻を比較する / False : 比較しない.
［戻値］	なし.
［履歴］	新規作成 
			2023/04/06 山口 OP_BTS-44083 起動時もバックアップ時刻の更新は1時間に1度に変更.
================================================================================================*/
void  CB_RTC::setBackUpTime( Bool bl_update , Bool bl_backup_check){
#if defined(TORNADO) && !defined(FUM_LINUX) /*実機*/
	unsigned char uc_timebuf[7];
	unsigned char uc_backbuf[7];
	struct rtc_time rtc;
	time_t tt = 0;
	struct tm st_tm;
	CT_Time now_time;
	memset(&st_tm, 0, sizeof(st_tm));
	memset( uc_timebuf, 0, sizeof(uc_timebuf) );
	memset( uc_backbuf, 0, sizeof(uc_backbuf) );
	unsigned char uc_data = 0;		
	uint64_t ull_NewDetailCode = 0x0;
	Bool bl_ret = True;
	
	SYSF_semTake( m_semID_InitialTime, WAIT_FOREVER );

	// 時間が先祖返りしたフラグ取得.
	if ( 0 != THRG_getBspThrRegion( 0x4D, (void*)&uc_data, sizeof(uc_data) ) ) {
		// エラー .
		SYSF_logMsg( "## %s %d : THRG_getBspThrRegion ERR\n", __FUNCTION__, __LINE__, 3,4,5,6 );
	}
	// バックアップ時刻取得.
	if ( 0 == THRG_getBspThrRegion( 0x46, (void*)&uc_backbuf, sizeof(uc_backbuf) ) ) {
		// 時間が先祖返りしたフラグが立っていた場合、詳細コード登録.
		if (uc_data == 1) {
			ull_NewDetailCode = ((((uint64_t)0x22					<< 56) & 0xff00000000000000)		// モジュール番号.
							   | (((uint64_t)0x02C					<< 44) & 0x00fff00000000000)		// 通し番号.
							   | (((uint64_t)uc_backbuf[0]			<< 36) & 0x00000ff000000000)		// 年.
							   | (((uint64_t)(uc_backbuf[1]+1)		<< 28) & 0x0000000ff0000000)		// 月.
							   | (((uint64_t)uc_backbuf[2]	 		<< 20) & 0x000000000ff00000)		// 日.
							   | (((uint64_t)uc_backbuf[3]	 		<< 12) & 0x00000000000ff000)		// 時.
							   | (((uint64_t)uc_backbuf[4]			<<  4) & 0x0000000000000ff0)		// 分.
							   | (((uint64_t)1						<<  0) & 0x000000000000000f));		// 先祖返り.
			SYSF_logMsg( "(%010u)(BSP) ## RTC Time Throwback NewDetailCode:%#llx\n", SYSF_tickGet() , ull_NewDetailCode );
			bl_ret = APIC_setNewDetailCode( TYPE_FatalDetailCode_undefined, ull_NewDetailCode );		// 詳細コード登録.
			if (False == bl_ret) {
				SYSF_logMsg( "(%010u)(BSP) ## %s %d : APIC_setNewDetailCode NG\n", SYSF_tickGet() , __FUNCTION__, __LINE__, 3,4,5,6 );
			}
			uc_data = 0;
			if ( 0 != THRG_setBspThrRegion( 0x4D, (void*)&uc_data, sizeof(uc_data) )) {
				// エラー .
				SYSF_logMsg( "## %s %d : THRG_setBspThrRegion ERR\n", __FUNCTION__, __LINE__, 3,4,5,6 );
			}
		}
	}
	
	now_time = getNowTime();
	
	// rtc時間変換 .
	rtc.tm_year = now_time.year - 1900;
	rtc.tm_mon = now_time.month - 1;
	rtc.tm_mday = now_time.day;
	rtc.tm_hour = now_time.hour;
	rtc.tm_min = now_time.minute;
	rtc.tm_sec = now_time.second;
	
	// 曜日算出 .
	st_tm.tm_year  = rtc.tm_year;
	st_tm.tm_mon   = rtc.tm_mon;
	st_tm.tm_mday  = rtc.tm_mday;
	st_tm.tm_hour  = rtc.tm_hour;
	st_tm.tm_min   = rtc.tm_min;
	st_tm.tm_sec   = rtc.tm_sec;
	st_tm.tm_isdst = -1;
	// 日付・時刻の再計算.
	tt = mktime(&st_tm);
	rtc.tm_wday = st_tm.tm_wday;	// 曜日 [0:日 1:月 ... 6:土].
	
	// バックアップ書き込み用に変換.
	uc_timebuf[0] = (char)( rtc.tm_year & 0xFF );		// 年.
	uc_timebuf[1] = (char)( rtc.tm_mon & 0xFF );		// 月.
	uc_timebuf[2] = (char)( rtc.tm_mday & 0xFF );		// 日.
	uc_timebuf[3] = (char)( rtc.tm_hour & 0xFF );		// 時.
	uc_timebuf[4] = (char)( rtc.tm_min & 0xFF );		// 分.
	uc_timebuf[5] = (char)( rtc.tm_sec & 0xFF );		// 秒.
	uc_timebuf[6] = (char)( rtc.tm_wday & 0xFF );		// 曜日.
	
	// 更新フラグがFalseの場合、1時間に一度更新 .
	if(bl_update == False ){
		// RTC時刻とバックアップ時刻を比較.
		if(uc_backbuf[2] == uc_timebuf[2] && uc_backbuf[3] == uc_timebuf[3] ){
			SYSF_semGive( m_semID_InitialTime );
			return ;
		}
	}

	if ( 0 != THRG_getBspThrRegion( 0x4D, (void*)&uc_data, sizeof(uc_data) ) ) {
		// エラー .
		SYSF_logMsg( "## %s %d : THRG_getBspThrRegion ERR\n", __FUNCTION__, __LINE__, 3,4,5,6 );
	}
	if( (bl_backup_check == True) 
	   && ( (uc_backbuf[0] > uc_timebuf[0])		// RTC時刻とバックアップ時刻の比較.
	     || (uc_backbuf[0] == uc_timebuf[0] && uc_backbuf[1] > uc_timebuf[1]) 
	     || (uc_backbuf[0] == uc_timebuf[0] && uc_backbuf[1] == uc_timebuf[1] && uc_backbuf[2] > uc_timebuf[2] ) 
	     || (uc_backbuf[0] == uc_timebuf[0] && uc_backbuf[1] == uc_timebuf[1] && uc_backbuf[2] == uc_timebuf[2] && uc_backbuf[3] > uc_timebuf[3] ) ) ) {
		
		ull_NewDetailCode = ((((uint64_t)0x22					<< 56) & 0xff00000000000000)		// モジュール番号.
						   | (((uint64_t)0x02C					<< 44) & 0x00fff00000000000)		// 通し番号.
						   | (((uint64_t)uc_backbuf[0]			<< 36) & 0x00000ff000000000)		// 年.
						   | (((uint64_t)(uc_backbuf[1]+1)		<< 28) & 0x0000000ff0000000)		// 月.
						   | (((uint64_t)uc_backbuf[2]	 		<< 20) & 0x000000000ff00000)		// 日.
						   | (((uint64_t)uc_backbuf[3]	 		<< 12) & 0x00000000000ff000)		// 時.
						   | (((uint64_t)uc_backbuf[4]			<<  4) & 0x0000000000000ff0)		// 分.
						   | (((uint64_t)2						<<  0) & 0x000000000000000f));		// RTC時刻が過去に戻っている.
		SYSF_logMsg( "(%010u)(BSP) ## Backup Time strange NewDetailCode:%#llx\n", SYSF_tickGet() , ull_NewDetailCode );
		bl_ret = APIC_setNewDetailCode( TYPE_FatalDetailCode_undefined, ull_NewDetailCode );		// 詳細コード登録.
		if (False == bl_ret) {
			SYSF_logMsg( "(%010u)(BSP) ## %s %d : APIC_setNewDetailCode NG\n", SYSF_tickGet() , __FUNCTION__, __LINE__, 3,4,5,6 );
		}
	}
	
	
	// バックアップファイル更新.
	if ( 0 != THRG_setBspThrRegion( 0x46, &uc_timebuf, sizeof(uc_timebuf) ) ) {
		// エラー .
		SYSF_logMsg( "## %s %d : THRG_setBspThrRegion ERR\n", __FUNCTION__, __LINE__, 3,4,5,6 );
	}
	// 2022/11/25 山口 OP_BTS-40732 RTC FAXロールバック正式対応.
	// メモリ上のバックアップデータを削除.
	clearRtcFaxBackupData();		
	SYSF_semGive( m_semID_InitialTime );
#endif // defined(TORNADO) && !defined(FUM_LINUX).
}
// 2022/11/25 山口 OP_BTS-40732 RTC FAXロールバック正式対応.
extern THRC_PowerManager thePowerManagerTask;

// FAX送受信時にRTC時刻を取得.メモリ上にそのデータを格納.
void CB_RTC::getRtcFaxBackupData( void){
	
	struct rtc_time rtc;
	time_t tt = 0;
	struct tm st_tm;
	CT_Time now_time;
	unsigned char uc_timebuf[7];
	memset( uc_timebuf, 0, sizeof(uc_timebuf) );
	memset(&st_tm, 0, sizeof(st_tm));
	SYSF_semTake( m_semID_InitialTime, WAIT_FOREVER );
	now_time = getNowTime();
	rtc.tm_year = now_time.year - 1900;
	rtc.tm_mon = now_time.month - 1;
	rtc.tm_mday = now_time.day;
	rtc.tm_hour = now_time.hour;
	rtc.tm_min = now_time.minute;
	rtc.tm_sec = now_time.second;
	
	// 曜日算出 .
	st_tm.tm_year  = rtc.tm_year;
	st_tm.tm_mon   = rtc.tm_mon;
	st_tm.tm_mday  = rtc.tm_mday;
	st_tm.tm_hour  = rtc.tm_hour;
	st_tm.tm_min   = rtc.tm_min;
	st_tm.tm_sec   = rtc.tm_sec;
	st_tm.tm_isdst = -1;
	// 日付・時刻の再計算.
	tt = mktime(&st_tm);
	rtc.tm_wday = st_tm.tm_wday;	// 曜日 [0:日 1:月 ... 6:土].
		
	uc_timebuf[0] = (unsigned char)( rtc.tm_year & 0xFF );	// 年.
	uc_timebuf[1] = (unsigned char)( rtc.tm_mon & 0xFF );		// 月.
	uc_timebuf[2] = (unsigned char)( rtc.tm_mday & 0xFF );	// 日.
	uc_timebuf[3] = (unsigned char)( rtc.tm_hour & 0xFF );	// 時.
	uc_timebuf[4] = (unsigned char)( rtc.tm_min & 0xFF );		// 分.
	uc_timebuf[5] = (unsigned char)( rtc.tm_sec & 0xFF );		// 秒.
	uc_timebuf[6] = (unsigned char)( rtc.tm_wday & 0xFF );	// 曜日.
	
	thePowerManagerTask.setRtcFaxBackupDataRegion( uc_timebuf );
	
	SYSF_semGive( m_semID_InitialTime );
}

// メモリ情報のクリア処理.
void CB_RTC::clearRtcFaxBackupData( void ){
	
	unsigned char uc_timebuf[7];
	memset( uc_timebuf, 0, sizeof(uc_timebuf) );
	thePowerManagerTask.setRtcFaxBackupDataRegion( uc_timebuf );
}
	
// Str 2010/09/13 ft_kondo 8536環境Sim対応.
#ifdef TORNADO			// 実機で有効.
static void
add_sec_to_rtc_time(struct rtc_time *rtc_tm, uint32_t sec)
{
	/*GaiaPF ◆Start 2010/09/24 kurachi CQ:333040対策（RTCアラーム設定でトラブルC-FA14）*/
	int32_t t_sec, t_min, t_hour;
	struct rtc_time tEndDT;
    struct tm  tTm;

    /* ここではsecの単位は秒 */

	t_sec = sec + rtc_tm->tm_sec;
    rtc_tm->tm_sec = (uchar)(t_sec % 60);
    sec = t_sec / 60;
    t_min = sec + rtc_tm->tm_min;
    rtc_tm->tm_min = (uchar)(t_min % 60);
    sec = t_min / 60;
    t_hour = sec + rtc_tm->tm_hour;
    rtc_tm->tm_hour = (uchar)(t_hour % 24);
    sec = t_hour / 24;

    /* ここまででsecの単位は日になる */

	calcAddDate( rtc_tm, sec, &tEndDT );
	memset(&tTm, 0, sizeof(tTm));
	tTm.tm_mday = rtc_tm->tm_mday = tEndDT.tm_mday;
	tTm.tm_mon  = rtc_tm->tm_mon = tEndDT.tm_mon;
	tTm.tm_year = rtc_tm->tm_year = tEndDT.tm_year;
	tTm.tm_isdst = -1;

	/* 曜日を算出する */
	if (mktime(&tTm) != -1) {
		rtc_tm->tm_wday = tTm.tm_wday;  /* [0:日 1:月 ... 6:土] */
	}

	/*GaiaPF ◆End 2010/09/24 kurachi CQ:333040対策（RTCアラーム設定でトラブルC-FA14）*/
}
/*================================================================================================
［機能］	日付算出（ある日から指定日数後の日付を算出する）
--------------------------------------------------------------------------------------------------
［引数］	struct rtc_time *tStartDT : ある日
			int iSpan                 : 指定日数
			struct rtc_time *tEndDT   : 算出後の日付
［戻値］	無し
［ｳｫｰｸｽﾙｰ］ 未実施
［履歴］	10/09/24 kurachi Minerva初版（application\mfp\pcm\src\cgi\pcmCgiFunc_PSL_AS.c内の
										  AS2_NTP_calcAddDate()を流用）
================================================================================================*/
static void calcAddDate( struct rtc_time *tStartDT, int32_t iSpan, struct rtc_time *tEndDT )
{
	/*                  1  2  3  4  5  6  7  8  9 10 11 12*/
	int32_t		tDays[] = {31,28,31,30,31,30,31,31,30,31,30,31};	/* 1ヶ月の日数 */
	int32_t		iYear;												/* 年 */
	int32_t		iMonth;												/* 月 */
	int32_t		iDay;												/* 日 */
	int32_t		iLoop;												/* ループカウンタ */

	/* [1] 開始日を終了日にコピー */
	iYear = tStartDT->tm_year + 1900;
	iMonth = tStartDT->tm_mon + 1;
	iDay = tStartDT->tm_mday;
	/* [2] 指定年の閏年の判定 */
	if (checkUruuYear(iYear) == True) {
		tDays[1] = 29;
	} else {
		tDays[1] = 28;
	}

	/* [3]-1.指定期間後の日付の算出(日加算値≧0の場合) */
	if (iSpan >= 0) {
		for (iLoop = 1; iLoop <= iSpan; iLoop++) {
			iDay = iDay + 1;
			if (iDay > tDays[iMonth - 1]) {
				iDay = 1;
				iMonth = iMonth + 1;
				if (iMonth > 12) {
					iYear = iYear + 1;
					iMonth = 1;
				}
			}

			/* ◆ 閏年の判定も考慮すべき→1月1日の時点で算出 */
			if (iMonth == 1 && iDay == 1) {
				if (checkUruuYear(iYear) == True) {
					tDays[1] = 29;
				} else {
					tDays[1] = 28;
				}
			}
		}
	/* [3]-2.指定期間後の日付の算出(日加算値＜0の場合) */
	} else {
		for (iLoop = iSpan; iLoop < 0 ; iLoop++) {
			iDay = iDay - 1;
			if (iDay < 1) {
				iMonth = iMonth - 1;
				if (iMonth < 1) {
					iYear = iYear - 1;
					iMonth = 12;
				}
				iDay = tDays[iMonth - 1];
			}

			/* ◆ 閏年の判定も考慮すべき→12月31日の時点で算出 */
			if (iMonth == 12 && iDay == 31) {
				if (checkUruuYear(iYear) == True) {
					tDays[1] = 29;
				} else {
					tDays[1] = 28;
				}
			}
		}
	}
	/* [4] 引数への終了日のコピー */
	tEndDT->tm_year = iYear - 1900;
	tEndDT->tm_mon  = iMonth - 1;
	tEndDT->tm_mday = iDay;
}
/*================================================================================================
［機能］	閏年の判定
--------------------------------------------------------------------------------------------------
［引数］	int iYear : 年
［戻値］	True : 閏年である
			False : 閏年でない
［ｳｫｰｸｽﾙｰ］ 未実施
［履歴］	10/09/24 kurachi Minerva初版（application\mfp\pcm\src\cgi\pcmCgiFunc_PSL_AS.c内の
										  AS2_NTP_checkUruYear()を流用）
================================================================================================*/
static Bool checkUruuYear( int32_t iYear )
{
	/* ◆ 閏年チェック */
	/* 西暦÷4で余りありの場合は閏年でない */
	if ((iYear % 4) != 0) {
			return False;		/* 閏年でない */
	}
	/* 西暦÷100で余りなしの場合 */
	if ((iYear % 100) == 0) {
		/* 西暦÷400で余りありの場合は閏年でない */
		/* (例：2000,2400,2800→閏年/2100,2200,2300→普通の年) */
		if ((iYear % 400) != 0) {
			return False;		/* 閏年でない */
		}
	}
	/* その他の場合は 閏年と判断してよい */

	return True;				/* 閏年である */
}
#endif // #ifndef TORNADO.
// End 2010/09/13 ft_kondo 8536環境Sim対応.


/*================================================================================================
［機能］	ブリッジのポート制御
--------------------------------------------------------------------------------------------------
［引数］	Bool state
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	未実施
================================================================================================*/
void portOut_LED_PIC(Bool state)
{
	CB_BitPortN<THRD_Port_DataType>::newInstance( CBBPTN_portLED_2 )	= (ushort)( ( state == True ) ? 1 : 0 );
}

/*================================================================================================
［機能］	
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［ｳｫｰｸｽﾙｰ］	未実施
================================================================================================*/
void procMemoryClear()
{
}

/*================================================================================================
［機能］	ポート番号一覧出力（本体、Subsetと実装をあわせること）.
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	無し
［履歴］	2017/07/22 Sky井柳 [IT5_4.1][AR1026735]新規作成：ポート番号ズレ確認用.
================================================================================================*/
void printPortNumLst( void )
{
	printf( "CB_PortTypeNum List\n" );
	printf( "  CBPTN_ptASICXX_DUMMY_PIO[%d]\n",CBPTN_ptASICXX_DUMMY_PIO );
	printf( "  CBPTN_ptASIC22_DUMMY_PIO[%d]\n",CBPTN_ptASIC22_DUMMY_PIO );
	printf( "  CBPTN_ptASIC22_DUMMY_PIO2[%d]\n",CBPTN_ptASIC22_DUMMY_PIO2 );
	printf( "  CBPTN_ptAsic22_PIO_0R[%d]\n",CBPTN_ptAsic22_PIO_0R );
	printf( "  CBPTN_ptAsic22_PIO_1R[%d]\n",CBPTN_ptAsic22_PIO_1R );
	printf( "  CBPTN_ptAsic22_PIO_2R[%d]\n",CBPTN_ptAsic22_PIO_2R );
	printf( "  CBPTN_ptAsic22_PIO_4R[%d]\n",CBPTN_ptAsic22_PIO_4R );
	printf( "  CBPTN_ptAsic22_PIO_5R[%d]\n",CBPTN_ptAsic22_PIO_5R );
	printf( "  CBPTN_ptAsic22_PIO_6R[%d]\n",CBPTN_ptAsic22_PIO_6R );
	printf( "  CBPTN_ptAsic22_RST_0R[%d]\n",CBPTN_ptAsic22_RST_0R );
	printf( "  CBPTN_ptAsic22_RST_1R[%d]\n",CBPTN_ptAsic22_RST_1R );
	printf( "  CBPTN_ptAsic22_RST_2R[%d]\n",CBPTN_ptAsic22_RST_2R );
	printf( "  CBPTN_ptAsic22_RST_3R[%d]\n",CBPTN_ptAsic22_RST_3R );
	printf( "  CBPTN_ptAsic22_RST_4R[%d]\n",CBPTN_ptAsic22_RST_4R );
	printf( "  CBPTN_ptAsic2014_PIO_0R[%d]\n",CBPTN_ptAsic2014_PIO_0R );
	printf( "  CBPTN_ptAsic2014_PIO_4R[%d]\n",CBPTN_ptAsic2014_PIO_4R );
	printf( "  CBPTN_ptASIC7_PORT01[%d]\n",CBPTN_ptASIC7_PORT01 );
	printf( "  CBPTN_ptASIC7_PORT02[%d]\n",CBPTN_ptASIC7_PORT02 );
	printf( "\n" );

	printf( "CB_BitPortTypeNum List\n" );
	printf( "  CBBPTN_NON[%d]\n",CBBPTN_NON );
	printf( "  CBBPTN_MAX[%d]\n",CBBPTN_MAX );
	
	printf( "  CBBPTN_portDIPSW_0[%d]\n",CBBPTN_portDIPSW_0 );
	printf( "  CBBPTN_portDIPSW_1[%d]\n",CBBPTN_portDIPSW_1 );
	printf( "  CBBPTN_portDIPSW_2[%d]\n",CBBPTN_portDIPSW_2 );
	printf( "  CBBPTN_portDIPSW_3[%d]\n",CBBPTN_portDIPSW_3 );
	printf( "  CBBPTN_portUtilityKey[%d]\n",CBBPTN_portUtilityKey );
	printf( "  CBBPTN_portIRC_CNCT[%d]\n",CBBPTN_portIRC_CNCT );
	printf( "  CBBPTN_portTERMINAL_SEL[%d]\n",CBBPTN_portTERMINAL_SEL );
	printf( "  CBBPTN_portFUN_LOCK_1[%d]\n",CBBPTN_portFUN_LOCK_1 );
	printf( "  CBBPTN_portCPRDY[%d]\n",CBBPTN_portCPRDY );
	printf( "  CBBPTN_portIMGRDY[%d]\n",CBBPTN_portIMGRDY );
	printf( "  CBBPTN_portWAKE_FIERY[%d]\n",CBBPTN_portWAKE_FIERY );
	printf( "  CBBPTN_portSUB_SW_ON[%d]\n",CBBPTN_portSUB_SW_ON );
	printf( "  CBBPTN_portREV_BOARD_ON[%d]\n",CBBPTN_portREV_BOARD_ON );

	printf( "  CBBPTN_portPOWER_MONITOR[%d]\n",CBBPTN_portPOWER_MONITOR );
	printf( "  CBBPTN_portENG_BUSY[%d]\n",CBBPTN_portENG_BUSY );
	printf( "  CBBPTN_portD102CopyEnable[%d]\n",CBBPTN_portD102CopyEnable );
	printf( "  CBBPTN_portKeyCntIsSet[%d]\n",CBBPTN_portKeyCntIsSet );
	printf( "  CBBPTN_portTotalCntIsSet[%d]\n",CBBPTN_portTotalCntIsSet );
	printf( "  CBBPTN_portSEC_SET2[%d]\n",CBBPTN_portSEC_SET2 );
	printf( "  CBBPTN_portSEC_SET1[%d]\n",CBBPTN_portSEC_SET1 );
	printf( "  CBBPTN_portFIERY_CNCT[%d]\n",CBBPTN_portFIERY_CNCT );

	printf( "  CBBPTN_portLED_0[%d]\n",CBBPTN_portLED_0 );
	printf( "  CBBPTN_portLED_1_N[%d]\n",CBBPTN_portLED_1_N );
	printf( "  CBBPTN_portLED_2[%d]\n",CBBPTN_portLED_2 );
	printf( "  CBBPTN_portLED_3[%d]\n",CBBPTN_portLED_3 );
	printf( "  CBBPTN_portLED_4_N[%d]\n",CBBPTN_portLED_4_N );
	printf( "  CBBPTN_portLED_5[%d]\n",CBBPTN_portLED_5 );
#ifdef DEF_MinervaS_PF
	printf( "  CBBPTN_portPMS_PLL2_ENB[%d]\n",CBBPTN_portPMS_PLL2_ENB );
#endif // #ifdef DEF_MinervaS_PF.
	printf( "  CBBPTN_portAC_POWER_DOWN[%d]\n",CBBPTN_portAC_POWER_DOWN );
#if defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS )
	printf( "  CBBPTN_portSEL_COLOR0[%d]\n",CBBPTN_portSEL_COLOR0 );
//	printf( "  CBBPTN_portSEL_COLOR1[%d]\n",CBBPTN_portSEL_COLOR1 );
//	printf( "  CBBPTN_portSEL_COLOR2[%d]\n",CBBPTN_portSEL_COLOR2 );
#else // #if defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS ).
	printf( "  CBBPTN_portPOWER_OFF_PNL[%d]\n",CBBPTN_portPOWER_OFF_PNL );
	printf( "  CBBPTN_portPOWER_OFF[%d]\n",CBBPTN_portPOWER_OFF );
#endif // #if defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS ).

	printf( "  CBBPTN_portFIF_O_PD[%d]\n",CBBPTN_portFIF_O_PD );
	printf( "  CBBPTN_portMOTER_EN[%d]\n",CBBPTN_portMOTER_EN );
	printf( "  CBBPTN_ppppD103PaperFeed[%d]\n",CBBPTN_ppppD103PaperFeed );
	printf( "  CBBPTN_portCTL_BUSY[%d]\n",CBBPTN_portCTL_BUSY );
	printf( "  CBBPTN_ppppKeyCounter[%d]\n",CBBPTN_ppppKeyCounter );
	printf( "  CBBPTN_ppppTotalCounter[%d]\n",CBBPTN_ppppTotalCounter );
	printf( "  CBBPTN_portTERM_SEL[%d]\n",CBBPTN_portTERM_SEL );
	printf( "  CBBPTN_portSPEEKER_REM[%d]\n",CBBPTN_portSPEEKER_REM );
	printf( "  CBBPTN_portFUN_SPEED[%d]\n",CBBPTN_portFUN_SPEED );
	printf( "  CBBPTN_portFUN_REM_1[%d]\n",CBBPTN_portFUN_REM_1 );
	printf( "  CBBPTN_portENGINE_SET[%d]\n",CBBPTN_portENGINE_SET );
	printf( "  CBBPTN_portONLINE[%d]\n",CBBPTN_portONLINE );
#if !(defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS ))
	printf( "  CBBPTN_portSEL_COLOR[%d]\n",CBBPTN_portSEL_COLOR );
#endif // #if !(defined( DEF_MinervaS_PF ) || defined( DEF_ZEUSSBKX0_CUS )).
	printf( "  CBBPTN_portOut_IREQ_PIC_OUT[%d]\n",CBBPTN_portOut_IREQ_PIC_OUT );
	printf( "  CBBPTN_portWAKE_FAXMFPC[%d]\n",CBBPTN_portWAKE_FAXMFPC );
	printf( "  CBBPTN_portOut_IREQ_IR[%d]\n",CBBPTN_portOut_IREQ_IR );

	printf( "  CBBPTN_portRST0_RST0[%d]\n",CBBPTN_portRST0_RST0 );
	printf( "  CBBPTN_portRST1_RST0[%d]\n",CBBPTN_portRST1_RST0 );
	printf( "  CBBPTN_portRST2_RST0[%d]\n",CBBPTN_portRST2_RST0 );
	printf( "  CBBPTN_portRST1_PIC[%d]\n",CBBPTN_portRST1_PIC );
	printf( "  CBBPTN_portRST_0R[%d]\n",CBBPTN_portRST_0R );

	printf( "  CBBPTN_portLCDC_LVDS_EN[%d]\n",CBBPTN_portLCDC_LVDS_EN );
	printf( "  CBBPTN_portIRC_RESET[%d]\n",CBBPTN_portIRC_RESET );
	printf( "  CBBPTN_portDOTCLKEN[%d]\n",CBBPTN_portDOTCLKEN );
	printf( "  CBBPTN_portCK_EN_C[%d]\n",CBBPTN_portCK_EN_C );
	printf( "  CBBPTN_portCK_EN_B[%d]\n",CBBPTN_portCK_EN_B );
	printf( "  CBBPTN_portCK_EN_A[%d]\n",CBBPTN_portCK_EN_A );
	printf( "  CBBPTN_portF_PD_T[%d]\n",CBBPTN_portF_PD_T );
	printf( "  CBBPTN_portF_PD_R[%d]\n",CBBPTN_portF_PD_R );
	printf( "  CBBPTN_portPCIRST_CPS[%d]\n",CBBPTN_portPCIRST_CPS );
	printf( "  CBBPTN_portRST2_PIC[%d]\n",CBBPTN_portRST2_PIC );
	printf( "  CBBPTN_portRST_4R[%d]\n",CBBPTN_portRST_4R );

	printf( "  CBBPTN_portLOCKN[%d]\n",CBBPTN_portLOCKN );
	printf( "  CBBPTN_portCTS_DISABLE[%d]\n",CBBPTN_portCTS_DISABLE );

	printf( "  CBBPTN_portINPUT_SEL[%d]\n",CBBPTN_portINPUT_SEL );
	printf( "  CBBPTN_portPIC5600Type[%d]\n",CBBPTN_portPIC5600Type );
	printf( "  CBBPTN_portIO_IORST5[%d]\n",CBBPTN_portIO_IORST5 );
	printf( "  CBBPTN_portIO_IORST4[%d]\n",CBBPTN_portIO_IORST4 );
	printf( "  CBBPTN_portIOLCDC_RST[%d]\n",CBBPTN_portIOLCDC_RST );
	printf( "  CBBPTN_portLCD_SUSPEND[%d]\n",CBBPTN_portLCD_SUSPEND );
	printf( "  CBBPTN_portPLL_RST_CPS[%d]\n",CBBPTN_portPLL_RST_CPS );
	printf( "  CBBPTN_portIORESET1[%d]\n",CBBPTN_portIORESET1 );
	printf( "  CBBPTN_portIORESET3[%d]\n",CBBPTN_portIORESET3 );
	printf( "  CBBPTN_portIORESET2[%d]\n",CBBPTN_portIORESET2 );
	printf( "  CBBPTN_portPOWER_REM[%d]\n",CBBPTN_portPOWER_REM );
	printf( "  CBBPTN_portIR_OEN[%d]\n",CBBPTN_portIR_OEN );
	printf( "  CBBPTN_portPOWER_OFF0[%d]\n",CBBPTN_portPOWER_OFF0 );
	printf( "  CBBPTN_portPOWER_OFF1[%d]\n",CBBPTN_portPOWER_OFF1 );
	printf( "  CBBPTN_portDB7654[%d]\n",CBBPTN_portDB7654 );
	printf( "  CBBPTN_portKEY_SELECT[%d]\n",CBBPTN_portKEY_SELECT );

	printf( "  CBBPTN_portPrinterIsPowerOn[%d]\n",CBBPTN_portPrinterIsPowerOn );
	printf( "  CBBPTN_portIORST1[%d]\n",CBBPTN_portIORST1 );
	printf( "  CBBPTN_portIORST2[%d]\n",CBBPTN_portIORST2 );
	printf( "  CBBPTN_portFILE_DIMM_INIT_1[%d]\n",CBBPTN_portFILE_DIMM_INIT_1 );
	printf( "  CBBPTN_portS_O_PD[%d]\n",CBBPTN_portS_O_PD );
	printf( "  CBBPTN_portFILE_DIMM_INIT_2[%d]\n",CBBPTN_portFILE_DIMM_INIT_2 );
#ifdef DEF_EDH_COLOR
	printf( "  CBBPTN_portAE_TG[%d]\n",CBBPTN_portAE_TG );
	printf( "  CBBPTN_portFB_AE3[%d]\n",CBBPTN_portFB_AE3 );
	printf( "  CBBPTN_portFB_AE2[%d]\n",CBBPTN_portFB_AE2 );
	printf( "  CBBPTN_portFB_AE1[%d]\n",CBBPTN_portFB_AE1 );
	printf( "  CBBPTN_portFB_AE0[%d]\n",CBBPTN_portFB_AE0 );
#endif
	printf( "  CBBPTN_portServiceChk[%d]\n",CBBPTN_portServiceChk );
	printf( "  CBBPTN_portTroubleReset[%d]\n",CBBPTN_portTroubleReset );
	printf( "  CBBPTN_portMemoryClear[%d]\n",CBBPTN_portMemoryClear );
	printf( "  CBBPTN_portInitial[%d]\n",CBBPTN_portInitial );
	printf( "  CBBPTN_portTotalClear[%d]\n",CBBPTN_portTotalClear );
	printf( "  CBBPTN_portOut_DEBUG_PIC_1[%d]\n",CBBPTN_portOut_DEBUG_PIC_1 );
	printf( "  CBBPTN_portOut_DEBUG_PIC_2[%d]\n",CBBPTN_portOut_DEBUG_PIC_2 );
	printf( "  CBBPTN_portOut_DEBUG_PIC_3[%d]\n",CBBPTN_portOut_DEBUG_PIC_3 );
	printf( "  CBBPTN_portOut_DEBUG_PIC_4[%d]\n",CBBPTN_portOut_DEBUG_PIC_4 );
	printf( "  CBBPTN_portSEL_IR[%d]\n",CBBPTN_portSEL_IR );
	printf( "  CBBPTN_portSEL_NT[%d]\n",CBBPTN_portSEL_NT );
	printf( "  CBBPTN_portP_VEN[%d]\n",CBBPTN_portP_VEN );
	printf( "  CBBPTN_portCTS_TERM[%d]\n",CBBPTN_portCTS_TERM );
	printf( "  CBBPTN_portCARDIN1[%d]\n",CBBPTN_portCARDIN1 );
	printf( "  CBBPTN_portFUN_LOCK[%d]\n",CBBPTN_portFUN_LOCK );
	printf( "  CBBPTN_portMSC_DTR[%d]\n",CBBPTN_portMSC_DTR );
	printf( "  CBBPTN_portSSC_DSR[%d]\n",CBBPTN_portSSC_DSR );
	printf( "  CBBPTN_portNTF_CNCT[%d]\n",CBBPTN_portNTF_CNCT );
	printf( "  CBBPTN_portCLOSE[%d]\n",CBBPTN_portCLOSE );
	printf( "  CBBPTN_portOut_CLK_INITIAL[%d]\n",CBBPTN_portOut_CLK_INITIAL );
	printf( "  CBBPTN_portRTS[%d]\n",CBBPTN_portRTS );
	printf( "  CBBPTN_portSSC_DTR[%d]\n",CBBPTN_portSSC_DTR );
	printf( "  CBBPTN_portIMG_THROUGH[%d]\n",CBBPTN_portIMG_THROUGH );
	printf( "  CBBPTN_portC_O_PD2[%d]\n",CBBPTN_portC_O_PD2 );
	printf( "  CBBPTN_portC_O_PD[%d]\n",CBBPTN_portC_O_PD );
	printf( "\n" );

}

/*================================================================================================
［機能］	デバッグターミナル有効判定.
--------------------------------------------------------------------------------------------------
［引数］	無し
［戻値］	True:デバッグターミナル有効, False:デバッグターミナル無効.
［履歴］	新規作成. 2017/11/08 ishiga [Eagle] 起動時間短縮(再設計).
================================================================================================*/
Bool THRF_isEnable_DebugTerm( void )
{
	Bool bl_Enable = False;
	switch( getTermState() ) {
		case BSPLIB_TERM_STATE_0:
			bl_Enable = False;		// 無効.
			break;
		case BSPLIB_TERM_STATE_1:
		case BSPLIB_TERM_STATE_2:
		case BSPLIB_TERM_STATE_3:
		case BSPLIB_TERM_STATE_4:
		default:
			bl_Enable = True;		// 有効.
			break;
	}
	return bl_Enable;
}

