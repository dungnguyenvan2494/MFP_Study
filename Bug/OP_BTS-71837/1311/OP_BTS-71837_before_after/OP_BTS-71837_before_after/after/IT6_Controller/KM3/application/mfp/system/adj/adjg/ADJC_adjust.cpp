//
//ファイル名：ADJC_adjust.cpp.
//
//概要：自動画像調整関係.
//

#include <sys/mman.h>
#include "system/sys/APIC_TaskPriority.h"
#include <errno.h>

#include "ADJC_adjust.h"
#include "system/thr/thrcom/ccbackup.h"
#include "system/thr/thrcom/ThrCommonDrvrIf.h"
#include "system/prt/APIC_Printer.h"

#define SLEEP_TIME          20000
#define MAX_SLEEP_COUNT     100

// 共有メモリ・オブジェクト名.
#define SHM_COMMAND    "/shmem-adjust-command"
#define SHM_RESPONSE   "/shmem-adjust-response"
#define SHM_IMAGE_R    "/shmem-adjust-image-r"
#define SHM_IMAGE_G    "/shmem-adjust-image-g"
#define SHM_IMAGE_B    "/shmem-adjust-image-b"
#define SHM_IMAGE_SIZE "/shmem-adjust-image-size"
#define SHM_SERIAL     "/shmem-adjust-serial"

// コマンド.
#define COMMAND_NONE    -1   // コマンドなし.
#define COMMAND_WAIT    0    // 待機.
#define COMMAND_END     1    // 終了.
#define COMMAND_ANALIZE 2    // 画像解析.

// レスポンス.
#define RESPONSE_NONE             0     // レスポンスなし.
#define RESPONSE_OK               1     // レスポンスOK.
#define RESPONSE_NG               2     // レスポンスNG.
#define RESPONSE_PARAM_NG         3     // レスポンスNG(入力パラメータ異常).
#define RESPONSE_ADJUST_NG        4     // レスポンスNG(調整不可).
#define RESPONSE_SYSTEM_NG        5     // レスポンスNG(調整値算出不可).
#define RESPONSE_PARAM_MISSMATCH  6     // レスポンスNG(入力パラメータ不整合).

// タスク.
#define ADJD_TASK_NAME            "tAdjustTask"       // タスク名.
#define ADJD_TASK_PRIORITY        TYPD_PRIONO_ADJ__tAdjustTask                 // 優先度.
#define ADJD_TASK_OPTIONS         VX_FP_TASK          // オプション.
#define ADJD_TASK_STACK_SIZE      (50 * 1024)         // スタックサイズ.

#define ADJD_QUEUE_MAX            20                  // キューサイズ.

// 要求通知ID.
#define ADJD_REQ_ID_AUTOANALYZE 0     // 自動画像調整の実行要求通知.

#define ADJD_STR_NUM            256  // 入力文字数.
#define ADJD_ADJUST_TABLE_NUM     2  // 調整項目数.

// 調整値範囲.
#define ADJD_ADJUST_RANGE_MIN  0
#define ADJD_ADJUST_RANGE_MAX  1
#define ADJD_ADJUST_RANGE_NUM  2

#define ADJD_INPUT_FILE_PATH "/tmp/input.json"                           // 入力ファイル名.

#define ADJD_ADJUST_FILE_PATH "/tmp/Adjust.txt"                          // 調整値ファイル名.
#define ADJD_OUTPUT_FILE_PATH "/tmp/output.json"                         // 測定結果ファイル名.
#define ADJD_OUTPUT_CORRECT_FILE_PATH "/tmp/Correct.json"                // 補正値ファイル名.

// 調整項目名.
const char* c_Adjust_TABLE[ADJD_ADJUST_TABLE_NUM] = {
	"sideEdgeAdjust",
	"leadingEdgeAdjust",
};

/*-------------------------------------------------------------------------------------------------*/
//   ADJC_adjustクラス：tAdjustTaskタスク.
/*-------------------------------------------------------------------------------------------------*/

/**
 * @brief コンストラクタ.
 * @return なし.
 */
ADJC_adjust::ADJC_adjust()
	: m_s_MsgId(0)
	, m_bl_AutoAnalyze(False)
	, m_bl_AppRunning(False)
{
	// メッセージキューＩＤ.
	MSG_Q_ID s_MsgId = SYSF_msgQCreate( (int32_t)ADJD_QUEUE_MAX, sizeof(ADJC_TaskMessageBox), MSG_Q_FIFO );
	if(NULL == s_MsgId){        // 生成失敗.
		printf("ADJC_adjust::ADJC_adjust SYSF_msgQCreate Fail.\n");
		return;
	}else{                                        // 生成成功.
		m_s_MsgId = s_MsgId;
	}
}

/**
 * @brief デストラクタ.
 * @return なし.
 */
ADJC_adjust::~ADJC_adjust()
{
}

/**
 * @brief インスタンス取得.
 * @return ADJC_adjustインスタンス(シングルトン)へのポインタ.
 */
ADJC_adjust* ADJC_adjust::newInstance()
{
	static ADJC_adjust c_Instance;
	return &c_Instance;
}

/**
 * @brief クラス初期化.
 * @return なし.
 */
void ADJC_adjust::initClass()
{
	// 自動画像調整アプリ起動待ち.
	WaitAppBoot();
	
	// タスク起動.
	taskSpawn();
}

/**
 * @brief スキャンモード取得.
 * @return スキャンモード.
 */
Bool ADJC_adjust::getScanMode()
{
	return m_bl_AutoAnalyze;
}

/**
 * @brief アプリ起動状態取得.
 * @return アプリ起動状態.
 */
Bool ADJC_adjust::getAppRunning()
{
	return m_bl_AppRunning;
}


/**
 * @brief オブザーバ登録.
 * @param[in] インスタンス.
 * @return なし.
 */
void ADJC_adjust::attachObserver(APIC_AdjustObserver* pc_AdjustObserver)
{
	if(NULL == pc_AdjustObserver){
		SYSF_logMsg("Assert Failed!\n",0,0,0,0,0,0);
		return ;
	}

//	m_pc_List.Lock();                           // オブザーバーロック.
	m_pc_List.add(pc_AdjustObserver);           // オブザーバーの追加.
//	m_pc_List.UnLock();                         // オブザーバーアンロック.
}

/**
 * @brief オブザーバ登録の解除.
 * @param[in] インスタンス.
 * @return なし.
 */
void ADJC_adjust::detachObserver(APIC_AdjustObserver* pc_AdjustObserver)
{
	if(NULL == pc_AdjustObserver){
		SYSF_logMsg("Assert Failed!\n",0,0,0,0,0,0);
		return ;
	}

//	m_pc_List.Lock();                           // オブザーバーロック.
	m_pc_List.del(pc_AdjustObserver);           // オブザーバーの削除.
//	m_pc_List.UnLock();                         // オブザーバーアンロック.
}

/**
 * @brief データ変化通知.
 * @param[in] ステータス.
 * @param[in] 測定結果.
 * @param[in] 測定結果サイズ.
 * @param[in] 補正値.
 * @param[in] 補正値サイズ.
 * @return なし.
 */
void ADJC_adjust::dataChanged(TYPE_AdjustStatus e_AdjustStatus, uchar *puc_OutputBuf, uint32_t ui32_OutputSize, uchar *puc_CorrectBuf, uint32_t ui32_CorrectSize)
{
	m_pc_List.Lock();
	APIC_AdjustObserver* pc_AdjustObserver = m_pc_List.findFirst(); // オブザーバーリストの先頭を設定.
	while(NULL != pc_AdjustObserver){
		pc_AdjustObserver->dataChanged(e_AdjustStatus, puc_OutputBuf, ui32_OutputSize, puc_CorrectBuf, ui32_CorrectSize);   // データ変化通知.
		pc_AdjustObserver = m_pc_List.findNext();
	}
	m_pc_List.UnLock();                     // オブザーバーアンロック.
}

/**
 * @brief タスク起動.
 * @return なし.
 */
void ADJC_adjust::taskSpawn()
{
	// タスク生成.
	if( ERROR == SYSF_taskSpawn( 
			ADJD_TASK_NAME, 
			ADJD_TASK_PRIORITY, 
			ADJD_TASK_OPTIONS, 
			ADJD_TASK_STACK_SIZE, 
			(FUNCPTR)entryTask, 
			(intptr_t)(this),0,0,0,0,0,0,0,0,0 ) ) {
		printf("tAdjustTask Start Fail.\n");
	}
}

/**
 * @brief タスクのエントリ関数.
 * @param[in] ADJC_adjustのポインタ.
 * @return なし.
 */
void ADJC_adjust::entryTask(ADJC_adjust* pc_Task)
{
	printf("ADJC_adjust::entryTask=%p\n", pc_Task);

	if(NULL == pc_Task){
		return;
	}
	
	pc_Task->messageloop();
}


/**
 * @brief 自タスクへのイベント発行.
 * @param[in] タスクメッセージ.
 * @return True：成功、False：失敗.
 */
Bool ADJC_adjust::sendMsg(ADJC_TaskMessageBox s_Msg)
{
	if(ERROR == SYSF_msgQSend(m_s_MsgId, (char*)&s_Msg, sizeof(s_Msg), WAIT_FOREVER, MSG_PRI_NORMAL)){
		printf("ADJC_adjust::sendMsg  SYSF_msgQSend Fail.\n");
		return False;
	}

	return True;
}

/**
 * @brief 自動画像調整の実行要求通知.
 * @param[in] 画像アドレス(R).
 * @param[in] 画像アドレス(G).
 * @param[in] 画像アドレス(B).
 * @param[in] 画像サイズ.
 * @return なし.
 */
void ADJC_adjust::req_AutoAnalyze(uintptr_t ul_VadrrR, uintptr_t ul_VadrrG, uintptr_t ul_VadrrB, uint32_t ul_Size)
{
	ADJC_TaskMessageBox s_Msg = {};
	s_Msg.ul_ID = ADJD_REQ_ID_AUTOANALYZE;
	s_Msg.ul_VadrrR = ul_VadrrR;
	s_Msg.ul_VadrrG = ul_VadrrG;
	s_Msg.ul_VadrrB = ul_VadrrB;
	s_Msg.ul_Size = ul_Size;
	sendMsg(s_Msg);
	
	return;
}

/**
 * @brief 自動画像調整ファイル設定.
 * @param[in] ファイルパス.
 * @return なし.
 */
void ADJC_adjust::setAdjustFile(const char *psc_FilePath)
{
	struct stat sb;

	FILE *fp_read = fopen(psc_FilePath, "r"); 		/* 読み込みファイル */
	if(fp_read == NULL){
		printf("ADJC_adjust::setAdjustFile fopen faild.\n");
		return;
	}
	int32_t i32_fd = fileno(fp_read);

	if(fstat(i32_fd, &sb) != 0){
		printf("ADJC_adjust::setAdjustFile stat faild.\n");
		fclose(fp_read);
		return;
	}

	uchar *puc_Data = (unsigned char*)malloc(sb.st_size);
	if(puc_Data == 0){
		printf("ADJC_adjust::setAdjustFile malloc faild.\n");
		fclose(fp_read);
		return;
	}

	size_t readsize = fread(puc_Data, 1, sb.st_size, fp_read);
	if((int32_t)readsize != sb.st_size){
		printf("ADJC_adjust::setAdjustFile fread faild.size=%d\n", (int32_t)readsize);
		fclose(fp_read);
		free(puc_Data);
		return;
	}
	
	fclose(fp_read);
	
	// 書き込みモードでファイルを開く
	FILE *fp_write = fopen(ADJD_INPUT_FILE_PATH, "w");
	if(fp_write == NULL){
		printf("ADJC_adjust::setAdjustFile fopen faild.\n");
		free(puc_Data);
		return;
	}
	i32_fd = fileno(fp_write);
	
	size_t writesize = fwrite(puc_Data, 1, sb.st_size, fp_write);
	if(writesize != sb.st_size){
		printf("ADJC_adjust::setAdjustFile fwrite faild.\n");
	}
	
	free(puc_Data);

	fflush(fp_write);
	fsync(i32_fd);
	
	//ファイルを閉じる.
	fclose(fp_write);
	
	m_bl_AutoAnalyze = True;
	
	return;
}

/**
 * @brief 調整アプリ起動.
 * @return なし.
 */
void ADJC_adjust::RunApp()
{
	printf("Run AdjustApp.\n");
	const int i_runRet71837 = system("/bd/sdc1/AdjustApp&");	// [OP_BTS-71837]
	printf("OP_BTS-71837: RunApp spawn /bd/sdc1/AdjustApp ret=%d\n", i_runRet71837);
	// [OP_BTS-71837] AdjustApp runs from the USB: candidate holder of sdc. Report its pid/exe.
	system("( sleep 3; ps | grep '[A]djustApp' | sed 's/^/OP_BTS-71837: ps: /' ) &");
}

/**
 * @brief 自動画像調整アプリ起動待ち.
 * @return 1：成功、0：失敗.
 */
int32_t ADJC_adjust::WaitAppBoot()
{
	uint32_t ul_Count = 0;
	
	InitResponse();
	SendCommand(COMMAND_WAIT);
	RunApp();
	// 自動画像調整アプリ起動待ち.
	while(1){
		usleep(SLEEP_TIME);
		// レスポンス取得.
		int32_t i32_response = ReceiveResponse();
		if(i32_response == RESPONSE_OK){
			printf("ADJC_adjust::WaitAppBoot response=%d\n\n", i32_response);
			m_bl_AppRunning = True;
			break;
		}
		ul_Count++;
		if(ul_Count == MAX_SLEEP_COUNT){
			printf("ADJC_adjust::WaitAppBoot timeout!!!.\n");
			return 0;
		}
	}

	return 1;
}

/**
 * @brief 自動画像調整アプリ終了.
 * @return 1：成功、0：失敗.
 */
int32_t ADJC_adjust::EndApp()
{
	uint32_t ul_Count = 0;

	InitResponse();
	SendCommand(COMMAND_END);
	// 終了待ち.
	while(1){
		usleep(SLEEP_TIME);
		// レスポンス取得.
		int32_t i32_response = ReceiveResponse();
		if(i32_response == RESPONSE_OK){
			printf("ADJC_adjust::EndApp response=%d\n\n", i32_response);
			m_bl_AppRunning = False;
			break;
		}
		ul_Count++;
		if(ul_Count == MAX_SLEEP_COUNT){
			printf("ADJC_adjust::EndApp timeout!!!.\n");
			return 0;
		}
	}

	return 1;
}

/**
 * @brief コマンド送信.
 * @param[in] コマンド.
 * @return なし.
 */
int32_t ADJC_adjust::SendCommand(int32_t i32_command)
{
	printf("ADJC_adjust::SendCommand.\n");
	
	int32_t i32_fd = shm_open(SHM_COMMAND, O_CREAT | O_RDWR, 0666);
	if(i32_fd == -1){
		printf("ADJC_adjust::SendCommand shm_open faild.\n");
		return 0;
	}

	if(ftruncate(i32_fd, sizeof(int32_t)) == -1){
		printf("ADJC_adjust::SendCommand ftruncate faild.\n");
		close(i32_fd);
		return 0;
	}

	void *addr = mmap(0, sizeof(int32_t), PROT_READ|PROT_WRITE, MAP_SHARED, i32_fd, 0);
	close(i32_fd);
	if(addr == MAP_FAILED){
		printf("ADJC_adjust::SendCommand mmap faild.\n");
		return 0;
	}
	
	int32_t *pi32_walker = (int32_t *)addr;
	*pi32_walker = i32_command;
	
	if(munmap(addr, sizeof(int32_t)) == -1){
		printf("ADJC_adjust::SendCommand munmap failed.\n");
	}

	return 1;
}

/**
 * @brief 画像送信.
 * @param[in] タスクメッセージ.
 * @return 1：成功、0：失敗.
 */
int32_t ADJC_adjust::SendImage(ADJC_TaskMessageBox s_Msg)
{
	printf("ADJC_adjust::SendImage.\n");
	
	// シリアルNo.
	ushort us_EngineSerialID[ESID_Size] = {0};	// シリアルID情報.
	us_EngineSerialID[ ESID_1st ] = static_cast<ushort>( comENG.getCcPrinterInfo().getSsPrinterinfo( PRF_MachineNumberL ) );
	us_EngineSerialID[ ESID_2nd ] = static_cast<ushort>( comENG.getCcPrinterInfo().getSsPrinterinfo( PRF_MachineNumberU ) );
	us_EngineSerialID[ ESID_3rd ] = static_cast<ushort>( comENG.getCcPrinterInfo().getSsPrinterinfo( PRF_ShipmentCodeL  ) );
	us_EngineSerialID[ ESID_4th ] = static_cast<ushort>( comENG.getCcPrinterInfo().getSsPrinterinfo( PRF_ShipmentCodeU  ) );
	us_EngineSerialID[ ESID_5th ] = static_cast<ushort>( comENG.getCcPrinterInfo().getSsPrinterinfo( PRF_MachineCodeL   ) );
	us_EngineSerialID[ ESID_6th ] = static_cast<ushort>( comENG.getCcPrinterInfo().getSsPrinterinfo( PRF_MachineCodeU   ) );
	
	int32_t  i32_fd = shm_open(SHM_SERIAL, O_CREAT | O_RDWR, 0666);
	if(i32_fd == -1){
		printf("ADJC_adjust::SendImage shm_open faild.\n");
		return 0;
	}
	
	if(ftruncate(i32_fd, sizeof(us_EngineSerialID)) == -1){
		printf("ADJC_adjust::SendImage ftruncate faild.\n");
		close(i32_fd);
		return 0;
	}

	void *addr = mmap(0, sizeof(us_EngineSerialID), PROT_READ|PROT_WRITE, MAP_SHARED, i32_fd, 0);
	close(i32_fd);
	if(addr == MAP_FAILED){
		printf("ADJC_adjust::SendImage mmap faild.\n");
		return 0;
	}
	memcpy(addr, us_EngineSerialID, sizeof(us_EngineSerialID));
	
	if(munmap(addr, sizeof(us_EngineSerialID)) == -1){
		printf("ADJC_adjust::SendImage munmap failed.\n");
	}
	
	// 画像サイズ.
	i32_fd = shm_open(SHM_IMAGE_SIZE, O_CREAT | O_RDWR, 0666);
	if(i32_fd == -1){
		printf("ADJC_adjust::SendImage shm_open faild.\n");
		return 0;
	}
	
	if(ftruncate(i32_fd, sizeof(int32_t)) == -1){
		printf("ADJC_adjust::SendImage ftruncate faild.\n");
		close(i32_fd);
		return 0;
	}

	addr = mmap(0, sizeof(int32_t), PROT_READ|PROT_WRITE, MAP_SHARED, i32_fd, 0);
	close(i32_fd);
	if(addr == MAP_FAILED){
		printf("ADJC_adjust::SendImage mmap faild.\n");
		return 0;
	}
	
	int32_t *pi32_walker = (int32_t *)addr;
	*pi32_walker = s_Msg.ul_Size;
	if(munmap(addr, sizeof(int32_t)) == -1){
		printf("ADJC_adjust::SendImage munmap failed.\n");
	}
	
	// 画像(R).
	i32_fd = shm_open(SHM_IMAGE_R, O_CREAT | O_RDWR, 0666);
	if(i32_fd == -1){
		printf("ADJC_adjust::SendImage shm_open(SHM_IMAGE_R) faild.\n");
		return 0;
	}

	if(ftruncate(i32_fd, sizeof(void*)) == -1){
		printf("ADJC_adjust::SendImage ftruncate(SHM_IMAGE_R) faild.\n");
		close(i32_fd);
		return 0;
	}

	addr = mmap(0, sizeof(void*), PROT_READ|PROT_WRITE, MAP_SHARED, i32_fd, 0);
	close(i32_fd);
	if(addr == MAP_FAILED){
		printf("ADJC_adjust::SendImage mmap(SHM_IMAGE_R) faild.\n");
		return 0;
	}
	
	// アドレスを物理アドレスに変換.
	uintptr_t ul_PhysicalCheckAddr = THRL_Drvrif_GetPAddr((void *)s_Msg.ul_VadrrR);
//	printf("ul_PhysicalCheckAddr=%p\n", ul_PhysicalCheckAddr);
	memcpy(addr, &ul_PhysicalCheckAddr, sizeof(void *));
//	printf("ul_Vadrr=%p, ul_Size=%d\n", s_Msg.ul_Vadrr, s_Msg.ul_Size);
	if(munmap(addr, sizeof(void*)) == -1){
		printf("ADJC_adjust::SendImage munmap(SHM_IMAGE_R) failed.\n");
	}

	// 画像(G).
	i32_fd = shm_open(SHM_IMAGE_G, O_CREAT | O_RDWR, 0666);
	if(i32_fd == -1){
		printf("ADJC_adjust::SendImage shm_open(SHM_IMAGE_G) faild.\n");
		return 0;
	}

	if(ftruncate(i32_fd, sizeof(void*)) == -1){
		printf("ADJC_adjust::SendImage ftruncate(SHM_IMAGE_G) faild.\n");
		close(i32_fd);
		return 0;
	}

	addr = mmap(0, sizeof(void*), PROT_READ|PROT_WRITE, MAP_SHARED, i32_fd, 0);
	close(i32_fd);
	if(addr == MAP_FAILED){
		printf("ADJC_adjust::SendImage mmap(SHM_IMAGE_G) faild.\n");
		return 0;
	}
	
	// アドレスを物理アドレスに変換.
	ul_PhysicalCheckAddr = THRL_Drvrif_GetPAddr((void *)s_Msg.ul_VadrrG);
	memcpy(addr, &ul_PhysicalCheckAddr, sizeof(void *));
	if(munmap(addr, sizeof(void*)) == -1){
		printf("ADJC_adjust::SendImage munmap(SHM_IMAGE_G) failed.\n");
	}

	// 画像(B).
	i32_fd = shm_open(SHM_IMAGE_B, O_CREAT | O_RDWR, 0666);
	if(i32_fd == -1){
		printf("ADJC_adjust::SendImage shm_open(SHM_IMAGE_B) faild.\n");
		return 0;
	}

	if(ftruncate(i32_fd, sizeof(void*)) == -1){
		printf("ADJC_adjust::SendImage ftruncate(SHM_IMAGE_B) faild.\n");
		close(i32_fd);
		return 0;
	}

	addr = mmap(0, sizeof(void*), PROT_READ|PROT_WRITE, MAP_SHARED, i32_fd, 0);
	close(i32_fd);
	if(addr == MAP_FAILED){
		printf("ADJC_adjust::SendImage mmap(SHM_IMAGE_B) faild.\n");
		return 0;
	}
	
	// アドレスを物理アドレスに変換.
	ul_PhysicalCheckAddr = THRL_Drvrif_GetPAddr((void *)s_Msg.ul_VadrrB);
	memcpy(addr, &ul_PhysicalCheckAddr, sizeof(void *));
	if(munmap(addr, sizeof(void*)) == -1){
		printf("ADJC_adjust::SendImage munmap(SHM_IMAGE_B) failed.\n");
	}

	return 1;
}

/**
 * @brief レスポンス初期化.
 * @param[in] コマンド.
 * @return 1：成功、0：失敗.
 */
int32_t ADJC_adjust::InitResponse()
{
	printf("ADJC_adjust::InitResponse.\n");
	
	int32_t i32_fd = shm_open(SHM_RESPONSE, O_CREAT | O_RDWR, 0666);
	if(i32_fd == -1){
		printf("ADJC_adjust::InitResponse shm_open faild.\n");
		return 0;
	}

	if(ftruncate(i32_fd, sizeof(int32_t)) == -1){
		printf("ADJC_adjust::InitResponse ftruncate faild.\n");
		close(i32_fd);
		return 0;
	}

	void *addr = mmap(0, sizeof(int32_t), PROT_READ|PROT_WRITE, MAP_SHARED, i32_fd, 0);
	close(i32_fd);
	if(addr == MAP_FAILED){
		printf("ADJC_adjust::InitResponse mmap faild.\n");
		return 0;
	}
	
	int32_t *pi32_walker = (int32_t *)addr;
	*pi32_walker = RESPONSE_NONE;

	if(munmap(addr, sizeof(int32_t)) == -1){
		printf("ADJC_adjust::InitResponse munmap failed.\n");
	}

	return 1;
}

/**
 * @brief レスポンス受信.
 * @return レスポンス.
 */
int32_t ADJC_adjust::ReceiveResponse()
{
	int32_t i32_fd = shm_open(SHM_RESPONSE, O_RDWR, 0666);
	if(i32_fd == -1){
		printf("ADJC_adjust::ReceiveResponse shm_open faild.\n");
		return 0;
	}

	void *addr = mmap(0, sizeof(int32_t), PROT_READ|PROT_WRITE, MAP_SHARED, i32_fd, 0);
	close(i32_fd);
	if(addr == MAP_FAILED){
		printf("ADJC_adjust::ReceiveResponse mmap faild.\n");
		return 0;
	}
	
	int32_t *pi32_walker = (int32_t *)addr;
	int32_t i32_response = *pi32_walker;
	if(munmap(addr, sizeof(int32_t)) == -1){
		printf("ADJC_adjust::ReceiveResponse munmap failed.\n");
	}
	
	return i32_response;
}

/**
 * @brief 画像解析送信.
 * @param[in] タスクメッセージ.
 * @return レスポンス、0：失敗.
 */
int32_t ADJC_adjust::SendAnalyzeImage(ADJC_TaskMessageBox s_Msg)
{
	uint32_t ul_Count = 0;
	int32_t i32_response = 0;

	InitResponse();
	SendImage(s_Msg);
	SendCommand(COMMAND_ANALIZE);
	// 自動画像調整アプリ完了待ち.
	while(1){
		usleep(SLEEP_TIME);
		// レスポンス取得.
		i32_response = ReceiveResponse();
		if(i32_response != RESPONSE_NONE){
			printf("ADJC_adjust::SendAnalyzeImage response=%d\n\n", i32_response);
			break;
		}
		ul_Count++;
		if(ul_Count == MAX_SLEEP_COUNT){
			printf("ADJC_adjust::SendAnalyzeImage timeout!!!.\n");
			return 0;
		}
	}

	return i32_response;
}

/**
 * @brief ファイルデータ取得.
 * @param[in] ファイルパス.
 * @param[out] ファイルデータへのポインタ.
 * @param[out] ファイルサイズ.
 * @return ファイルデータへのポインタ.
 */
uchar *ADJC_adjust::GetFileData(const char *psc_FilePath, uint32_t *ui32_Size)
{
	struct stat sb;

	FILE *fp_read = fopen(psc_FilePath, "r"); 		/* 読み込みファイル */
	if(fp_read == NULL){
		printf("ADJC_adjust::GetFileData fopen faild.\n");
		return 0;
	}
	int32_t i32_fd = fileno(fp_read);

	if(fstat(i32_fd, &sb) != 0){
		printf("ADJC_adjust::GetFileData stat faild.\n");
		fclose(fp_read);
		return 0;
	}

	uchar *puc_Data = (unsigned char*)malloc(sb.st_size);
	if(puc_Data == 0){
		printf("ADJC_adjust::GetFileData malloc faild.\n");
		fclose(fp_read);
		return 0;
	}

	size_t readsize = fread(puc_Data, 1, sb.st_size, fp_read);
	if((int32_t)readsize != sb.st_size){
		printf("ADJC_adjust::GetFileData fread faild.size=%d\n", (int32_t)readsize);
		fclose(fp_read);
		free(puc_Data);
		return 0;
	}
	
	*ui32_Size = sb.st_size;
	
	fclose(fp_read);
	
	return puc_Data;
}

/**
 * @brief 画像解析.
 * @param[in] タスクメッセージ.
 * @return 1：成功、0：失敗.
 */
int32_t ADJC_adjust::AnalyzeImage(ADJC_TaskMessageBox s_Msg)
{
	if(m_bl_AutoAnalyze == False){
		dataChanged(TYPE_AdjustStatus_Fail_Analysis, NULL, 0, NULL, 0);
		return 0;
	}
	
	// 画像解析送信.
	int32_t i32_Ret = SendAnalyzeImage(s_Msg);
	
	switch(i32_Ret){
	case RESPONSE_OK:
		{
			uint32_t ui32_OutputSize = 0;
			uint32_t ui32_CorrectSize = 0;
			uchar *puc_Output = NULL;
			uchar *puc_Correct = NULL;
			puc_Output = GetFileData(ADJD_OUTPUT_FILE_PATH, &ui32_OutputSize);
			if(access(ADJD_OUTPUT_CORRECT_FILE_PATH, F_OK) == 0){
				puc_Correct = GetFileData(ADJD_OUTPUT_CORRECT_FILE_PATH, &ui32_CorrectSize);
			}
			printf("ADJC_adjust::AnalyzeImage puc_Output=%p ui32_OutputSize=%d puc_Correct=%p, ui32_CorrectSize=%d\n", puc_Output, ui32_OutputSize, puc_Correct, ui32_CorrectSize);
//			if(puc_Output)
//				printf("%s\n", puc_Output);
//			if(puc_Correct)
//				printf("%s\n", puc_Correct);
			if(access(ADJD_ADJUST_FILE_PATH, F_OK) == 0){
				// 調整値設定.
				if(SetAdjustData() == 1){
					dataChanged(TYPE_AdjustStatus_Done_Analysis, puc_Output, ui32_OutputSize, puc_Correct, ui32_CorrectSize);
				}else{
					dataChanged(TYPE_AdjustStatus_Disable_Analysis, puc_Output, ui32_OutputSize, puc_Correct, ui32_CorrectSize);
				}
			}else{
				dataChanged(TYPE_AdjustStatus_Done_Analysis, puc_Output, ui32_OutputSize, puc_Correct, ui32_CorrectSize);
			}
		}
		break;
	case RESPONSE_NG:
		dataChanged(TYPE_AdjustStatus_Fail_Analysis, NULL, 0, NULL, 0);
		break;
	case RESPONSE_PARAM_NG:
		dataChanged(TYPE_AdjustStatus_Strange_Parameters, NULL, 0, NULL, 0);
		break;
	case RESPONSE_ADJUST_NG:
//		dataChanged(TYPE_AdjustStatus_OverRange_Offset, NULL, 0, NULL, 0);
		{
			uint32_t ui32_OutputSize = 0;
			uchar *puc_Output = NULL;
			puc_Output = GetFileData(ADJD_OUTPUT_FILE_PATH, &ui32_OutputSize);
//			if(puc_Output)
//				printf("%s\n", puc_Output);
			dataChanged(TYPE_AdjustStatus_OverRange_Offset, puc_Output, ui32_OutputSize, NULL, 0);
		}
		break;
	case RESPONSE_SYSTEM_NG:
		dataChanged(TYPE_AdjustStatus_Disable_Calculate, NULL, 0, NULL, 0);
		break;
	case RESPONSE_PARAM_MISSMATCH:
		dataChanged(TYPE_AdjustStatus_Missmatch_Parameters, NULL, 0, NULL, 0);
//		{
//			uint32_t ui32_OutputSize = 0;
//			uchar *puc_Output = NULL;
//			puc_Output = GetFileData(ADJD_OUTPUT_FILE_PATH, &ui32_OutputSize);
//			if(puc_Output)
//				printf("%s\n", puc_Output);
//			dataChanged(TYPE_AdjustStatus_Missmatch_Parameters, puc_Output, ui32_OutputSize, NULL, 0);
//		}
		break;
	default:
		printf("ADJC_adjust::AnalyzeImage unknown response. %d\n", i32_Ret);
		dataChanged(TYPE_AdjustStatus_Fail_Analysis, NULL, 0, NULL, 0);
		break;
	}
	
	// ファイル削除.
	system("rm -f "ADJD_INPUT_FILE_PATH);
	system("rm -f "ADJD_ADJUST_FILE_PATH);
	system("rm -f "ADJD_OUTPUT_FILE_PATH);
	system("rm -f "ADJD_OUTPUT_CORRECT_FILE_PATH);
	sync();
	sync();
	sync();
	
	m_bl_AutoAnalyze = False;
	
	printf("ADJC_adjust::AnalyzeImage is finished.\n");
	
	return i32_Ret;
}

/**
 * @brief 調整値設定.
 * @return 1：成功、0：失敗.
 */
int32_t ADJC_adjust::SetAdjustData()
{
	int32_t i32_AdjustNo[ADJD_ADJUST_TABLE_NUM] = {0};      // 調整No.
	int32_t i32_AdjustValue[ADJD_ADJUST_TABLE_NUM] = {0};   // 調整値(0.1mm単位).
	int32_t i32_AdjustRange[ADJD_ADJUST_TABLE_NUM] = {0};   // 調整範囲(0.1mm単位).
	Bool bl_Valid[ADJD_ADJUST_TABLE_NUM] = {False};
	char sc_str[ADJD_STR_NUM] = {0};
	int32_t i32_Ret = 1;

	// 読み込みモードでファイルを開く.
	FILE *fp_read = fopen(ADJD_ADJUST_FILE_PATH, "r");
	if(fp_read == NULL){
		printf("ADJC_adjust::SetAdjustData fopen faild.\n");
		return 0;
	}

	// 対象行の値を得る.
	while(fgets(sc_str, ADJD_STR_NUM, fp_read) != NULL){
		char sc_delim[] = ",";
		char *psc_token;
		// 調整項目名.
		psc_token = strtok(sc_str, sc_delim);
		if(psc_token == NULL){
			printf("ADJC_adjust::SetAdjustData psc_token(AdjustTable) is NULL.\n");
			i32_Ret = 0;
			continue;
		}
		for(int32_t i = 0; i < ADJD_ADJUST_TABLE_NUM; i++){
			if(strcmp(psc_token, c_Adjust_TABLE[i]) == 0){
				// 調整No.
				psc_token = strtok(NULL, sc_delim);
				if(psc_token == NULL){
					printf("ADJC_adjust::SetAdjustData psc_token(AdjustNo) is NULL.\n");
					i32_Ret = 0;
					break;
				}
				i32_AdjustNo[i] = atoi(psc_token);
				// 調整値.
				psc_token = strtok(NULL, sc_delim);
				if(psc_token == NULL){
					printf("ADJC_adjust::SetAdjustData psc_token(AdjustValue) is NULL.\n");
					i32_Ret = 0;
					break;
				}
				i32_AdjustValue[i] = atoi(psc_token);
				// 調整範囲.
				psc_token = strtok(NULL, sc_delim);
				if(psc_token == NULL){
					printf("ADJC_adjust::SetAdjustData psc_token(AdjustRange) is NULL.\n");
					i32_Ret = 0;
					break;
				}
				i32_AdjustRange[i] = atoi(psc_token);
				bl_Valid[i] = True;
				break;
			}
		}
	}

	// ファイルを閉じる.
	fclose(fp_read);

	// 調整値の範囲確認.
	for(int32_t i = 0; i < ADJD_ADJUST_TABLE_NUM; i++){
		if(bl_Valid[i] == True){
			// 現在の設定値を反映.
			i32_AdjustValue[i] = APIC_PrinterServiceSetting::newInstance()->getPrinterAdjust(i32_AdjustNo[i]) + i32_AdjustValue[i];
			// 0.2mm単位で設定を行うため、奇数の場合、偶数に丸める.
			if(i32_AdjustValue[i] > 0){
				if(i32_AdjustValue[i] % 2 == 1){
					printf("ADJC_adjust::SetAdjustData odd number no=%d, val=%d\n", i, i32_AdjustValue[i]);
					i32_AdjustValue[i]++;
					printf("ADJC_adjust::SetAdjustData fix number no=%d, val=%d\n", i, i32_AdjustValue[i]);
				}
			}else{
				if(i32_AdjustValue[i] % 2 == -1){
					printf("ADJC_adjust::SetAdjustData odd number no=%d, val=%d\n", i, i32_AdjustValue[i]);
					i32_AdjustValue[i]--;
					printf("ADJC_adjust::SetAdjustData fix number no=%d, val=%d\n", i, i32_AdjustValue[i]);
				}
			}
			// 調整範囲(i32_AdjustRange)は0.1mm単位の正数なので、例として30の場合は-3.0mm～3.0mmまでが、有効になる.
			if(i32_AdjustValue[i] < -i32_AdjustRange[i]){
				printf("ADJC_adjust::SetAdjustData Ajust Range err! no=%d, val=%d\n", i, i32_AdjustValue[i]);
				bl_Valid[i] = False;
				i32_Ret = 0;
			}else if(i32_AdjustRange[i] < i32_AdjustValue[i]){
				printf("ADJC_adjust::SetAdjustData Ajust Range err! no=%d, val=%d\n", i, i32_AdjustValue[i]);
				bl_Valid[i] = False;
				i32_Ret = 0;
			}
		}
	}
	// 調整値設定.
	for(int32_t i = 0; i < ADJD_ADJUST_TABLE_NUM; i++){
		if(bl_Valid[i] == True){
			APIC_PrinterServiceSetting::newInstance()->setPrinterAdjust(i32_AdjustNo[i], (short)i32_AdjustValue[i]);
			printf("ADJC_adjust::SetAdjustData setPrinterAdjust no=%d, val=%d, TYPE_PrinterAdjust=%d\n", i, i32_AdjustValue[i], i32_AdjustNo[i]);
		}
	}
	
	return i32_Ret;
}

/**
 * @brief メッセージループ.
 * @return なし.
 */
void ADJC_adjust::messageloop()
{
	printf("ADJC_adjust::messageloop start.\n");
	
	for(;;) {
		ADJC_TaskMessageBox s_Msg = {};

		if(ERROR == SYSF_msgQReceive(m_s_MsgId, (char*)&s_Msg, sizeof(s_Msg), WAIT_FOREVER)){
			return;
		}
		switch(s_Msg.ul_ID){
		case ADJD_REQ_ID_AUTOANALYZE:          // 自動画像調整の実行要求通知.
			AnalyzeImage(s_Msg);
			break;
		default:
			printf("ADJC_adjust::messageloop no registration.\n");
			break;
		}
	}
}
