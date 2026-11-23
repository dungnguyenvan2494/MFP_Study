# `system/thr_it6/PanelHW` — Giải thích action của từng hàm

Thư mục: `IT6_Controller/KM3/application/mfp/system/thr_it6/PanelHW/`
(Số dòng = dòng trong file gốc. Comment gốc là tiếng Nhật, đã dịch/diễn giải.)

## 0. Tổng quan — chỉ 4 file .cpp được build

Theo `Makefile`, chỉ build 4 file; các file `Donau/Zeus/Thames/PPanelHard*`, `threshold/*.pix`, `rmp*.pic` là legacy/dữ liệu.

| File | Dòng | Vai trò |
|---|---|---|
| `THRC_PanelHardSoC.cpp` | 5959 | Lớp `CNVC_PanelHard` (singleton): **cổng duy nhất từ Controller xuống driver panel** (LED, backlight, buzzer, sound IC, touch calib, NFC/BLE, vibrator, lock display, thông tin loại panel…) + code SIO-packet legacy + code mô phỏng |
| `THRC_PanelSound.cpp` | 455 | Lớp `THRC_PanelSound` — phát âm qua `/dev/kmsnd` (**chỉ `#ifdef TORNADO`**, đường cũ; đường mới dùng sound IC trong `CNVC_PanelHard`) |
| `THRC_ccXPanel.cpp` | 802 | Dữ liệu chia sẻ “external panel” (`comXPanel`) + điều khiển Function (list print, memory check, test print…) |
| `THRC_tskXPanelif.cpp` | 4871 | Task `TASKXPANEL_IF` chu kỳ 100 ms: gửi trạng thái máy sang “external panel” và nhận lệnh/mode map về (giao thức MINET) |

> **Lưu ý khái niệm quan trọng:** `CNVC_PanelHard` là **panel vật lý** (màn hình + LED + buzzer + touch, qua `panelcom` driver).
> `XPanel` (ccXPanel/tskXPanelif) là **một giao diện dữ liệu “Rhein1P external panel”** giữa Controller và lớp engine/service (map địa chỉ `A_xxx`, mode-map, report) — **không phải** driver phần cứng panel.

### Quy ước build (quyết định nhánh code nào sống)
- `DEF_PANELDRV` (+ `QT_PANEL`) bật trên máy thật ⇒ các hàm `CNVC_PanelHard` đi qua `sysBspIoctlPanelSio(buffer, EXDB_PanelKindXxx)` → driver `panelcom`. Nhánh `#else` (`ptr_PanelLcdIF->…`, SIO packet) là **legacy**, không chạy trên HW.
- `FUM_LINUX` = build simulator Linux ⇒ hầu hết hàm HW trả về ngay / no-op.
- `TORNADO` = nhánh thực (tên cũ từ VxWorks, vẫn dùng làm cờ “chạy thật”); nhiều hàm chỉ có thân khi `TORNADO`.
- Hầu hết hàm có guard **`isSuspend`** (panel đã bị `communicationSusped(SUSPEND)`) ⇒ trả về ngay, không đụng driver — liên quan trực tiếp tới deep sleep.

---

## 1. `THRC_PanelHardSoC.cpp` — lớp `CNVC_PanelHard`

### 1.1 Vòng đời / khởi tạo

| Hàm (dòng) | Action |
|---|---|
| `CNVC_PanelHard(uchar* VRAMAddr)` (187) | Constructor. Lấy độ phân giải/kích thước panel (`getPanelResolutionX/Y`, `getPanelSize`; trên simulator đọc `/root/sim_ini.env`). Với `DEF_PANELDRV`+`QT_PANEL`: `VRAM=NULL`, `vram=NULL` (Qt tự lo framebuffer). Xóa `keyGetData`, đặt `keySetData_Rhein1[]` mặc định (SUB_SW_OFF…), `ptr_PanelLcdIF = PLBC_PanelLcdIF::newInstance()`, `uc_Connected=False`, reset biến sound (`s_playCount=0`, `isSoundMute=False`, semaphore=NULL), `muc_LastIllumiStatus=STANDBY`, `m_e_LockDisplayType=NON`. |
| `~CNVC_PanelHard()` (280) | `delete[]` `VRAM` và `vram` nếu có. |
| `newInstance()` (2418) | Singleton `static CNVC_PanelHard thePanelHard(NULL)`. |
| `init()` (308) | Khởi tạo LCD. Nếu `thePowerManagerTask.isPanelSleepEndLCDInitBusy()` (đang resume từ sleep) → `THRL_Drvrif_LCD_InitLCD()`, cache flush VRAM, DMA `PCI→SDRAM`, chờ xong, `InitSDRAMToLCD/StartSDRAMToLCD`, `SetDisplayON(1)`. Nếu không (boot thường) → `panelInit()`. Lần đầu: `setBackLight(True)` (thực ra `setBackLight` là *Do not use*, xem bên dưới). Nhánh `!DEF_PANELDRV`: `THRF_InitPanelSIO`, `keyScan`. |
| `init2()` (401, `TORNADO`) | Khởi tạo phần phụ: tạo mutex `semID_SoundIC`; nếu panel có sound IC (`THRD_PanelOP_SoundIC`) → spawn task `THR-PSOUND` (`soundTask`) và ghi version/checksum sound data vào `SystemInfo` (`setRomVersion/CheckSum(TYPE_RomPanelSoundData)`); nếu có NFC (hoặc BLE với `DEF_SparrowAIOSFPCus`) → spawn `NFC-AUTH` (`TASK_transNFCAuthData`) và `NFCBLE-COUNT` (`TASK_sendNFCBLECounter`); đăng ký `CNVC_RemotePanelSettingObserver` vào `APIC_RemotePanelSetting`; tạo mutex `semID_NFCPairingIC`. |
| `panelInit()` (2063) | Đưa panel về trạng thái ban đầu: tắt tất cả LED (IntMode, Scaner, Copy, Fax, Box, EnlargeDisp, PrintGreen, Data, PowerSave, Universal), **bật LED PrintRed**, `setLEDBrightness(100)`, `setBuzzer(Off)`. |
| `keyScan()` (475) | **Chỉ có thân khi `!DEF_PANELDRV`** (legacy): gom `keySetData_Rhein1` (OUT1/OUT2/BUZZER/SLEEP; SUB_SW ON/OFF theo `APIC_Power::isSubPowerOff()`), nếu đổi thì gửi socket `IJC_SOCKET_DATA_BUZZER_LED` tới sim-panel. Trên HW: rỗng. |

### 1.2 LED / Illumi

| Hàm (dòng) | Action |
|---|---|
| `getLEDKindDrv(LEDKind)` (2559) | Map LED logic → số LED của driver (`DEF_LED_CMD_LED01..LED12`): PrintGreen→01, PrintRed→02, Menu→03, Scaner/Guidance→04, Access→05, Preview→06, Universal→07, EnlargeDisp→08, IntMode→09, Data→10, VoiceGuide→11, còn lại→**12 = “không dùng”** (bị chặn ở các hàm gọi). |
| `getLED(led)` (558) | `LED_Data`: đọc từ `theLEDStatusTask.getStatus(THRE_LEDID_DATA)` (không đụng driver). Nếu `isSuspend` → `Off`. LED12 → `None`. Còn lại ioctl `EXDB_PanelKindGetLed` rồi suy ra Blink/On/Off. |
| `setLED(led,on)` (647) | `LED_Data` → bỏ qua (do LEDStatusTask điều khiển). Bỏ qua nếu `isSuspend`; bỏ qua **bật** LED khi `get_MainSwOFF_Flag()` (đang tắt nguồn chính); LED12 → bỏ qua. Còn lại ioctl `EXDB_PanelKindSetLed` (`LedOnOff`). |
| `setLEDBlink(led,time,on)` (2621) | Bỏ qua nếu `isSuspend`, nếu `time` ngoài `LEDBlinkTime01..04`, hoặc LED12. Điền `LED_SETTING{LedBlink, OnTime=OffTime=time}` → ioctl `EXDB_PanelKindSetLedBlink`. |
| `setLEDBrightness(level)` (745) | Bỏ qua nếu `isSuspend`. Map `Panel_LEDBright_Level100…20` → `DEF_LED_CMD_DUTY_xx` (mặc định NO=100%) → ioctl `EXDB_PanelKindsetLEDBrightness`. |
| `setIllumiLED(status)` (817) | Nếu panel không có option `THRD_PanelOP_IllumiLED` → return. Lưu `muc_LastIllumiStatus`. Nếu Android đang kết nối (`THRG_IsConnectAndroid`) thì chuyển tiếp qua `THRC_AndroidControlIf::THRC_AndroidSetIllumiLED`. Mở (lazy) `/dev/illumi_unit` và `ioctl(ILLUMI_SET_LED_ON_OFF)`. *(Đây là chỗ Controller điều khiển đèn illumi — lái qua `illumi_unit` driver kernel, không qua panelcom.)* |
| `getLastIllumiLEDStatus()` (863) | Trả `muc_LastIllumiStatus`. |
| `getLEDbit()` (2340) | `!DEF_PANELDRV`: trả bitmap LED từ `ptr_PanelLcdIF`; HW: `0`. |

### 1.3 Backlight

| Hàm (dòng) | Action |
|---|---|
| `setBackLight(on)` (877) | **Không dùng**: chỉ in “Do not use”, return 0. |
| `setBackLight2(on)` (892) | Hàm thật. Nếu `MainSwOFF` và `on==True` → return 1 (không cho bật). `on==False`: `backLight_state=BACKLIGHT_OFF` và **`setBuzzer(Off)`**; `on==True`: `BACKLIGHT_ON`. Nếu `MainSwOFF` và `on==False` thì OR cờ `BACKLIGHT_MAINSWOFF_FLAG` (panel biết nguồn chính tắt). `isSuspend` → không gọi driver (ret=-1). Còn lại ioctl `EXDB_PanelKindSetBackLight`; thành công + `on` → `THRF_setEventRecord(BackLightOn)`; luôn `SYSF_ReqTaskMonitorUserEvent(18000,21,on)`. |
| `setBackLightBrightness(level)` (977) | `isSuspend` → ret -1; else ioctl `EXDB_PanelKindSetBackLightBrightness`. |
| `getBackLightBrightness()` (1016) | `isSuspend` → `0xff`; else ioctl `EXDB_PanelKindGetBackLightInfo` → `Brightness`. Không DEF_PANELDRV → `0x7f`. |

### 1.4 Buzzer & Sound IC

| Hàm (dòng) | Action |
|---|---|
| `getBuzzerType(kind)` (3025) | Map `Panel_BuzzerKind_*` → `ENM_BUZZER_TYPE_*` (InputConfirm, InputInvalid, Base, NormalTerminate, PrepareComplete, Caution1-3, Error, Off). |
| `setBuzzer(kind)` (1056) | Nhánh 1 (panel kiểu slide **hoặc** có option Buzzer): dựng `BUZZER_SETTING{type, 2 kHz, VOLUME4}` → **`thePowerManagerTask.sendSetPanelBuzzer()`** (đi qua PowerManager, không ioctl trực tiếp). Nhánh 2 (slide-less, dùng sound IC): nếu volume trong `ps_SoundMap[kind]==0` hoặc kind==Off → `stopSound()`; trừ `InputConfirm`, nếu đang phát và ưu tiên thấp hơn/bằng thì bỏ qua (gọi callback COMPLETE); else `setSoundVol(vol)` + `playSound(category,number)`. |
| `setBuzzerVol(kind,vol)` (1114) | Nhánh 1: map `Panel_BuzzerVolume_Level1..5` → `DEF_BUZ_CMD_VOLUME5/4/4/2/1`, `OFF` → type=OFF, `0xff`; gửi qua `sendSetPanelBuzzer`. Nhánh 2: lưu `ps_SoundMap[kind].vol` (>5 ⇒ 0) rồi `setBuzzer(kind)`. |
| `setBuzzerToSoundMap(kind,cat,num)` (1177) | Gán category/number (<16) của file âm cho kind trong `ps_SoundMap`. |
| `playSound(cat,num)` (1203) | Bỏ qua (+ gọi callback COMPLETE) nếu `isSuspend`, tham số ≥16, không có sound IC, hoặc `isSoundMute`. Lấy `semID_SoundIC`; nếu đang phát thì kết thúc bản cũ (callback); ioctl `EXDB_PanelSoundPlay`; `s_playCount++`. |
| `stopSound()` (1287) | (không suspend, có sound IC) lấy sem, ioctl `EXDB_PanelSoundStop`, nếu đang phát: `s_playCount--` + callback COMPLETE. |
| `setSoundVol(vol)` (1342) | Kẹp vol ≤5; `0` ⇒ `isSoundMute=True` + `stopSound()`; map Level1..5 → `DEF_SOUND_CMD_VOLUME5..1`; ioctl `EXDB_PanelSoundVolume`. |
| `setSoundCallback(func)` (1407) | Lưu `fnc_callback`. |
| `soundTask()` (1427) | Task `THR-PSOUND`: vòng lặp 100 ms: lấy sem, ioctl `EXDB_PanelSoundStatus`; nếu `s_playCount≥1` và status IDLE ⇒ giảm count + callback COMPLETE. |
| `downloadSoundData(file,data,size,cb)` (1478) → `TASK_downloadSoundData` (1513) | Spawn task `DOWNLOAD-SOUND`. Task: nếu truyền file → mở `/bd/sdc1/<file>` (USB), kích thước 1..256 KB, đọc vào heap; nếu truyền data → dùng trực tiếp (≤256 KB); ioctl `EXDB_PanelSoundDownload`; gọi `cb(result, checksum)`; free nếu tự malloc. ⚠ Dòng 1481 dùng `getSlidePanelOptionInfo() && THRD_PanelOP_SoundIC` (**&& logic thay vì &** — kiểm tra option luôn đúng khi option≠0; có vẻ là bug nhỏ). |
| `getSoundDataVersion(buf)` (3179) / `getSoundDataCheckSum()` (3227) | Bỏ qua nếu `isSuspend`/không có sound IC; ioctl `EXDB_PanelSoundVersion` / `EXDB_PanelSoundCheckSum`. Version toàn `0xff` ⇒ chưa có âm ⇒ chuỗi rỗng; else ghép vào mẫu `THRD_PanelOP_SoundDataVersion`. |

### 1.5 Firmware / thông tin panel

| Hàm (dòng) | Action |
|---|---|
| `getLCDVersion(buf)` (2717) | `isSuspend` → return. ioctl `EXDB_PanelKindGetLCDVersion` (`KM_SOFTINFO`) rồi ghép `KmSoftVersion1/2` thành chuỗi 19 ký tự dạng `A00J-8900-G00-00`. |
| `getLCDCheckSum()` (2780) | ioctl `EXDB_PanelKindGetSoftInfo`; chuyển `PanelCheckSum` (hex string) → ushort. |
| `downloadPanelFW(file,cb)` (1648) → `TASK_downloadPanelFW` (1677) | Spawn task `DL_PANELFW`: mở `/bd/sdc1/<file>`, 1..256 KB, ioctl `EXDB_PanelFWDownload` (nạp firmware MCU panel), `cb(result,sum)`. |
| `PanelFWDownload(char*)` (1739, `extern "C"`) / `PanelFW_callback` (1734) | Lệnh console gọi `downloadPanelFW` với callback chỉ log. |
| `panel_eepromwrite/read(addr,data)` (1797/1819, `extern "C"`) | Console: ghi/đọc EEPROM trong panel (chỉ khi **không** phải panel slide; addr ≤0x7F; ioctl `EXDB_PanelEEPROMWrite/Read`; ghi xong đánh dấu `setWarpInfo_ChgMFPData(True)`). |
| `panelLogPrint()` (2030, `extern "C"`) | Console: ioctl `EXDB_Ele_PanelLog` rồi `cat /proc/km/panelcomdrv/info` (có `taskDelay` tránh lẫn log). |
| `THRF_PanelOutputDebug()` (2135) | Debug: ioctl `EXDB_PanelDataGet` rồi in trạng thái LED/duty/backlight. |
| `PanelSize()` (2010, `extern "C"`) | Console: in kích thước/độ phân giải. |

### 1.6 Loại panel / option (cache một lần)

| Hàm (dòng) | Action |
|---|---|
| `getPanelType()` (2871) | Lần đầu ioctl `EXDB_PanelKindGetPanelTypeStruct` → lưu vào `gs_panelType` (static) và **cache vĩnh viễn**; lỗi ⇒ `NULL`. Dữ liệu này do U-Boot đưa vào FDT (paneltype/paneltypecode/paneloption/panelsize). |
| `getPanelTypeOption()` (2903) | Đọc lại struct nhưng chỉ cập nhật `us_panelOption` (gọi từ `TASK_sendNFCBLECounter` để làm mới option). |
| `getPanelSize/ResolutionX/ResolutionY()` (1937/1965/1993) | Từ `getPanelType()` (size ÷100 inch; px). Simulator: giá trị đọc từ `sim_ini.env`. |
| `isSlidePanel()` (3074) | `!uc_isSlideLessPanel` (NULL ⇒ True). |
| `getSlidePanelOptionInfo()` (3103) | `us_panelOption` (NULL ⇒ 0). |
| `isPanelOptionNFC()` (3131) / `isPanelOptionBLE()` (3155) | Test bit `THRD_PanelOP_NFC` / `_BLE`. |
| `getTouchPanelType()` (3700) | `ul_paneltype` (PCGE_PanelType_*). |
| `is26dotErrorFixed()` (2934) | `ul_paneltype` 0 (TPI) hoặc 3 (GDP) mà là panel slide ⇒ False (còn lỗi 26 dot), còn lại True. |
| `testPanelType()` (2977) | Console in thông tin panel type. |

### 1.7 Touch / tablet / vibrator / suspend

| Hàm (dòng) | Action |
|---|---|
| `setCTSensor(on)` (1758) | Cảm biến tĩnh điện (CTSC): `state=1` chỉ khi cài đặt hệ thống `ElectrostaticSensor==ON` **và** `on`; `threshold` 0..4 theo `ElectrostaticSensorAdjust` (-2..+2); ioctl `EXDB_PanelKindSetCTSC`. |
| `runTouchPanelCalibration(req)` (1851) / `TouchPanelCalibreation(uchar)` (1883) | ioctl `EXDB_PanelCalibration`; nếu `req==True` và panel **không** phải điện trở (M/S Resistance) thì chờ 10 s. Hàm C là wrapper console. |
| `getPointTouchPanelResistanceCalibration(info)` (1907) | Chỉ với panel điện trở: ioctl `EXDB_PointTouchPanelCalibration` lấy điểm hiệu chỉnh. |
| `setDeadAreaOffset(offset)` (2999) | ioctl `EXDB_PanelOffset` (vùng chết touch). |
| `lockPanelTablet(e)` (3472) | ioctl `EXDB_PanelLockPanelTablet` (khóa/mở khóa tablet). |
| `getLockKey()` (2820) | `isSuspend` → False; ioctl `EXDB_PanelKindGetLockKey` → `LockKey1==ON`. |
| `setVibrator(pattern,amount)` (5800) | Map `THRD_PANEL_VIB_PATTERN_A..E` → 0/3/6/9/12, amount LOW/MID/HIGH → 0/3/7; pattern += amount; ioctl `EXDB_PanelSetVibrator`; tham số sai ⇒ -1. `dbg_setVibrator` (5898, console) cho phép nhập thô (pattern≤127, amount≤7, time≤15). |
| **`communicationSusped(state)`** (2093) | **Điểm then chốt cho deep sleep.** `state==SUSPEND` ⇒ `isSuspend=True` **trước** khi ioctl; gọi ioctl `EXDB_PanelCommunicaitonSuspend(state)` (panelcom dừng/khôi phục UART); `state==RESUME2` ⇒ `isSuspend=False` **sau** ioctl. Từ lúc `isSuspend=True` mọi hàm LED/backlight/sound/version ở trên đều no-op ⇒ không có traffic UART xuống panel khi hệ thống đi ngủ. |

### 1.8 Display / VRAM

| Hàm (dòng) | Action |
|---|---|
| `transLCDData(kind)` (2221) | Đẩy VRAM ra LCD. `FUM_LINUX`: gửi socket `IJC_SOCKET_DATA_VRAM` tới sim. HW: return nếu `!THRF_IsAsic_0Exist()`; `DMATransMode`: cache flush, `TransPCIToSDRAM`, chờ xong, `StartSDRAMToLCD`, `SetDisplayON(1)`. *(Trên HW Qt/panelcom các nhánh này gần như không dùng.)* |
| `isBusyTransLCDData()` (2299) | `!IsTransPCIToSDRAMDone()` (hoặc `False` ở build khác). |
| `getVramSendDataSize()` (2524) | Kích thước VRAM gửi sim = f(độ phân giải). |
| `getNetConnected/setNetConnected` (2513/2518) | Cờ `uc_Connected` (sim-panel qua mạng). |
| `getSimAutoTestConnected/set…` (2534/2539, `DEF_SimAutoTest`) | Cờ auto-test. |
| `lockDisplay(kind)` (3262) | Cố định màn hình bằng ảnh tĩnh (ví dụ “đang remote panel”): `getFixDisplayImageAddr()` nạp ảnh, assert `isDisplayControlEnable()`, `THRL_Drvrif_LockDisplay(addr)`. Ghi `THRL_SetPanelHardLog`. |
| `unlockDisplay()` (3293) | Assert `isDisplayControlEnable()`, `THRL_Drvrif_UnlockDisplay()`. |
| `getFixDisplayImageAddr(kind)` (3326) | Tra `CNVS_LockDisplayImgTable` lấy đường dẫn file ảnh; cache nếu cùng kind; mở/fstat/đọc vào buffer `psc_ALLOC_LockDisplay` (kiểm tra ≤`CNVD_LockDisplayBufferSize`); lỗi ở bất kỳ bước ⇒ `THRL_ThAssert` (gây FATAL, các DetailCode `0x0C8E..0x0C93`). |
| `isDisplayControlEnable()` (3496) | True **chỉ khi** `PowerDevice.GetPowerState(PCIe_BUS)==NORMAL`; ngược lại log lỗi, False. *(Chống truy cập ASIC khi bus PCIe đang ngủ — FA14 trong OP_BTS-24653.)* |
| `callDisplayControlEnable()` (5949) | Wrapper public của hàm trên (simulator ⇒ True). Sinh ra do OP_BTS-24653. |
| `CNVC_RemotePanelSettingObserver::newInstance()` (3421) | Singleton observer. |
| `…::LockStatusNotify(locked)` (3438) | Callback của `APIC_RemotePanelSetting`: nếu `callDisplayControlEnable()==False` thì bỏ qua; `locked` ⇒ `lockDisplay(RMP_CONNECT)`, else `unlockDisplay()`. |
| `ReadFileSimResolution(cx,cy,cs)` (5615, simulator) | Đọc `/root/sim_ini.env` (`SimResolutionX/Y`, `SimPanelSize`); lỗi ⇒ mặc định 800×480, 9.0″. |

### 1.9 Các bit tín hiệu cổng nối tiếp “service”

| Hàm (dòng) | Action |
|---|---|
| `getTroubleReset()` (2355), `getInitial()` (2372), `getMemoryClear()` (2389), `getTotalClear()` (2406) | Đọc công tắc service trên board: ghi `CBBPTN_portServiceChk` (1 hoặc 2) → chờ 20 ×10 µs → đọc bit port tương ứng (`TroubleReset/Initial/MemoryClear/TotalClear`). Simulator ⇒ False. |

### 1.10 NFC / BLE

| Hàm (dòng) | Action |
|---|---|
| `transNFCPairingData(cb)` (3533) | Chờ 20 ms; nếu panel có NFC (hoặc BLE) và `cb`≠NULL, sem hợp lệ ⇒ `thePowerManagerTask.sendSetPairingData(cb)` (đi qua PowerManager); không có NFC ⇒ cb(OK). |
| `TASK_transNFCPairingData(cb)` (3592) | Thực thi gửi: lấy `semID_NFCPairingIC`, malloc 1 KB, lặp 3 loại dữ liệu (0=NDEF, 1=PairingData, 2=BLE advertise) lấy từ `APIC_NFCPairingSetting/APIC_BLESetting`, ioctl `EXDB_PanelKindNFCPairingDataTrans` từng loại; dừng khi lỗi; `cb(result)`; free + give sem. |
| `setNFCPairingEnable / setNFCAuthEnable / setBLEPairingEnable / setBLEAuthEnable` (3681/3778/3797/3818) | Nếu panel có NFC (hoặc BLE) ⇒ gọi `setPairingAuthDeviceSetting(type, enable)`. |
| `setPairingAuthDeviceSetting(type,en)` (3841) | Giữ bitmask `uc_PairingAuthEnable` (bit=1 là **vô hiệu**); ioctl `EXDB_PanelKindPossibleNFCPairing`; bảo vệ bằng `semID_NFCPairingIC`. |
| `setBLEMfpModelInfo(data,size)` (3893) | BLE only: kiểm 1..19 byte, copy vào static, spawn `BLE-MFPMODEL` (`TASK_setBLEMfpModelInfo`, 3931) gửi `dataType=3` qua ioctl `EXDB_PanelKindNFCPairingDataTrans`. |
| `TASK_transNFCAuthData()` (3728) | Task `NFC-AUTH` vòng vô hạn: ioctl `EXDB_PanelKindNFCAuthDataTrans` (blocking chờ dữ liệu xác thực từ panel); `result==2` & đang `PowerSavePanel` ⇒ tạm chặn; `result==1` ⇒ `changePowerMode(CardDeviceAccess)` rồi `APIC_NfcBleAuth::requestAuth(auth,random)`. |
| `TASK_sendNFCBLECounter()` (3990) | Task `NFCBLE-COUNT`: gọi `getPanelTypeOption()`, rồi mỗi 500 ms ioctl `EXDB_PanelKindNFCBLEFunctionCount` và tăng counter `countFuncUseCnt(NFC_Pairing/BLE_Pairing/NFC_Login/BLE_Login)` theo bit. |

### 1.11 Code SIO-packet legacy (`!DEF_PANELDRV`) và mô phỏng

Không chạy trên HW hiện tại (HW dùng `panelcom`):

| Hàm (dòng) | Action |
|---|---|
| `THRF_InitPanelSIO()` (4121) | Reset bộ đệm, lấy `ptr_PanelSioIF`. |
| `THRF_TransDataPanelSIO()` (4153) | Rút packet từ `sendPacket`, thêm header `0xFE` + checksum, ghi ra `sysBspWritePanelSio`. |
| `THRF_ReceiveDataPanelSIO()` (4220) | Đọc `sysBspReadPanelSio`, ghép với phần dư lần trước, gọi `THRF_PanelReceiveDataAnalyze` lặp để tách lệnh, đẩy vào `recvPacket`; lỗi → `THRL_ThAssertLog2` với DetailCode **`0xD403`** (sai checksum) / **`0xD404`** (lỗi extract) + `DebugForever`. |
| `THRF_PanelReceiveDataAnalyze(...)` (4354) | Tìm header `0xFD`, đọc độ dài, kiểm tra độ dài/command code/checksum; mã lỗi 1=header, 2=độ dài, 3=command, 4=checksum, 10=dữ liệu chưa đủ. |
| `THRF_SioConfirmCheckSum` (4046) / `THRF_MakeCheckSum` (4097) | `sum = (~Σ + 1) & 0x7F` — kiểm tra/sinh checksum. |
| `THRF_IsTimeout()` (4880) | Kiểm timeout hồi đáp. |
| `THRC_PanelSioSimReadSIO/WriteSIO` (4654/4686) | Vòng đệm giả lập panel (trả version `A00J8900`/`G0000000`, ACK/NACK, touch…). |
| `getUSBKeyCode` (4913/5246), `sysBspUsbKbdLedSet` (4923) | Chuyển mã bàn phím USB → mã key; điều khiển LED bàn phím USB (build đặc biệt). |

---

## 2. `THRC_PanelSound.cpp` — lớp `THRC_PanelSound` (chỉ `TORNADO`)

Phát âm qua device `THRD_SOUND_DEVICE_FILE` (kmsnd) bằng ioctl `KMSND_IOCSPLAY/ASYNCPLAY/STOP/VOLUME/STATUS`. Các hàm trên đều rỗng nếu không `TORNADO`.

| Hàm (dòng) | Action |
|---|---|
| `THRC_PanelSound()` (101) | Mở device; lỗi ⇒ `b_isExistDevice=False`; tạo mutex. |
| `~THRC_PanelSound()` (134) | Đóng fd. |
| `newInstance()` (152) | Singleton. |
| `init()` (172) | Spawn task `THR-PSOUND` (`panel_sound_task`); báo `APIC_Music::setDeviceState(Ready/NG)`. |
| `panel_sound_task()` (69) | Mỗi 20 tick: lấy sem; khi driver không còn “playing” (`statusDriver()!=1`) thì giảm `s_playCount` và callback `PLAY_COMPLETE`. |
| `play(data,size,volume)` (205) | Không có device ⇒ `NOT_EXIST_DEVICE`; driver trả 65534 ⇒ `SUSPEND`; kẹp volume 1..3 → `{-10,1,12} dB`; ioctl `VOLUME` rồi `ASYNCPLAY` (size ≤32767); kết thúc lượt cũ (callback), `s_playCount++`. |
| `stop()` (277) | ioctl `STOP`, giải phóng lượt đang phát (callback). |
| `status()` (326) | `NOT_EXIST_DEVICE` / `SUSPEND` / `PLAY` / `IDLE`. |
| `setCallback(f)` (356) | Lưu callback. |
| `statusDriver()` (373) | ioctl `KMSND_IOCSSTATUS` → `status` (65534 = suspend). |
| `cb_test / snd_play / snd_stop / snd_status` (403–455) | Console test: đọc file ≤1 MB, phát/dừng/hiện trạng thái. |

---

## 3. `THRC_ccXPanel.cpp` — dữ liệu chia sẻ “external panel”

| Hàm (dòng) | Action |
|---|---|
| `CC_XPanelTransData()` (32) | Khởi tạo `us_MPCPaperSizeCode=0`. |
| `CC_XPanelReceiveData()` (46), `initRDModeInfo()` (57) | Khởi tạo `FuncCode/FuncPara=0` và gọi toàn bộ `init*` để xóa dữ liệu nhận. |
| `initServiceID/CTID/DTID/TelNo/CenterTelNo/Address/ModemInitCmd/RdsSmtpLogIn/RdsSmtpPasswd/RdsSmtpDmain/SMTPServer/POP3Server/POP3User/POP3PassWord` (83–221) | Mỗi hàm xóa (`'\0'`) buffer chuỗi tương ứng (ID dịch vụ, ID CT/DT, số điện thoại, địa chỉ mail, lệnh init modem, thông tin SMTP/POP3…). |
| `CC_XPanelDataRequest()` (236) | Xóa các cờ yêu cầu/bận (Total/Trouble/Warning/CopyMode/Size counter, PMCounter/PrinterIndi/PrinterInfo). |
| `endPMCounter/endPrinterIndi/endPrinterInfo` (251/261/271) | Đẩy lệnh kết thúc `RC_PMCounterSendEnd / RC_PrinterIndiSendEnd / RC_PrinterInfoSendEnd` vào `comXPanel.ct_Command`. |
| `ans_FunctionComplete` (278) | Callback kết thúc function: OK ⇒ `FunctionEnd()` else `FunctionAbnormalStop()`. |
| `ans_listoutputComplete` (287), `ans_HDDFormatNVRAM` (291), `ans_HDDComplete` (295) | Kết thúc list-print → `FunctionEnd`; format HDD logic xong → `req_hddFormatWithNvramClear` rồi `FunctionEnd`. |
| `ans_start` (301) | Job NULL ⇒ `FunctionAbnormalStop`. `ans_detectPlatenOriginalSizeFix` (307), `ans_cancel` (550): rỗng. |
| `getCharSetForCommonAPI()` (318) | Tiếng Nhật ⇒ SJIS; ngôn ngữ khác ⇒ ISO-8859-1. |
| `CC_XPanel::FunctionStart()` (345) | Gọi khi nhận report “Function Start” từ external panel. Lưu FuncCode/FuncPara vào `comENG`; `FNC_NotSet` ⇒ AbnormalStop. Nhánh: **list print** (`FNC_TotalCounter`… → `APIC_ListPrint::req_startListPrint(TYPE_ListPrint*)`), **memory/bus check** (`MemoryRWCheck1/2`, `CompressExpandCheck`, `Memory_PRTBusCheck` → `cc_MEMFunction.start()`; `HardDiskFormat1/2` → `formatHdd`…), **test print/X-Rite** (`pc_PrinterImpColor->req_startFunction`), còn lại ⇒ `c_SystemServiceFunction->req_startFunction`. ⚠ Hai chỗ (`checkJobReserve()==False` rồi `FunctionEnd()`; `MemFuncStatusNOWExec` rồi `AbnormalStop()`) **không `return`** nên code chạy tiếp. |
| `FunctionStop()` (560) | Dừng function: list print ⇒ không làm gì; memory check ⇒ `cc_MEMFunction.stop()`; test-print ⇒ `req_cancel` job scan/print hiện tại (bỏ qua nếu đang `Deleting`); còn lại ⇒ `ENGC_PrinterIf::req_stopFunction`. |
| `FunctionEnd()` (598) / `FunctionAbnormalStop()` (609) | Đẩy `CC_FunctionStop` / `CC_FunctionAbnormalStop` vào `ct_Command` (báo kết quả về external panel). |
| `chkTestPrintFunctionMode(mode)` (619) | True nếu mode là function cần **cấp giấy** (test print, gamma/X-Rite auto adjust print…). |
| `chkModeRequest(addr)` (779) | Test-and-clear bit yêu cầu trong `uc_ModeRequestTbl[addr>>7][addr&0xF]` (bit = `(addr&0x70)>>4`); True nếu có yêu cầu gửi lại map `addr`. |
| `InitializeEnd()` (799) | Đẩy `RC_MemoryClearEnd` (báo hoàn tất xóa bộ nhớ). |

---

## 4. `THRC_tskXPanelif.cpp` — task giao tiếp external panel

### 4.1 Task và vòng chu kỳ

| Hàm (dòng) | Action |
|---|---|
| `XPLC_taskXPanel::newInstance()` (483) | Singleton. |
| `initClass()` (490) | Tạo msgQ, watchdog; spawn task **`TASKXPANEL_IF`** (stack 10 KB); đặt timer đánh thức đầu tiên `DEF_ENGIF_CYCLIC_MS` (30 ms). |
| `cyclicLoop()` (558) | Chạy trong ngữ cảnh watchdog-timer: gửi `TM_TimerWakeUp_Sys` (NO_WAIT) vào msgQ và **tự đặt lại** timer 100 ms ⇒ chu kỳ 100 ms. |
| `taskXPanel_IF()` (529) | Vòng `msgQReceive(WAIT_FOREVER)`; `TM_TimerWakeUp_Sys` ⇒ `procTimerWakeUp()`. |
| `_lock()/_unlock()` (514/517) | Rỗng. |
| `procTimerWakeUp()` (582) | Nếu engine chưa sẵn sàng (`CcNotReadyEng.BlTransStart`) ⇒ return. Chưa kết nối (`ifXPanelSub.bl_Connect==False`) ⇒ `ifXPanel.receiveAanalize()` + `ifXPanelSub.connect()` (bắt tay). Đã kết nối ⇒ `procXPanelTransfer()` (gửi) rồi `procXPanelReceive()` (nhận). |

### 4.2 Gửi trạng thái → external panel

| Hàm (dòng) | Action |
|---|---|
| `procXPanelTransfer()` (655) | Hơn 2000 dòng; với **từng địa chỉ map** (`A_ProcessCaution`, `A_FatalDetail`, `A_PrinterJamPaperInfo1`, `A_SystemType2`, `A_PrinterDoor2`, `A_ModeInfo`, `A_UnitSetInfo2`, `A_UnitNewInfo`, `A_xPUCounterInfoRD`, `A_MFPROMVersion`…) dựng giá trị `data` bằng cách OR các mask từ `comENG.getRx()`, `comWarning`, `APIC_*` (cửa, jam, cảnh báo, tuổi thọ IU/TC/belt/fuser, version ROM, số lô IU…) rồi gọi `_XPanelTransMap()`. Phần lớn chạy mỗi chu kỳ; các khối chuỗi lớn (lot no, ROM version) chỉ chạy khi `uc_TX_Num==1`. |
| `_XPanelTransMap(isString, addr, param)` (3809) | Gửi **chỉ khi cần**: có mode-request (`chkModeRequest`) **hoặc** giá trị khác lần trước (`ifXPanel.us_TransMap[addr]`) **hoặc** lần gửi đầu (`bl_FirstSend`). Chuỗi ⇒ `transStringMap`. |
| `_shortTouShortBitDForLPHAdminInfo` (608), `_clearString` (628), `_getUnitSetStatus` (642), `_getUnitLifeStatus` (4717), `_getEjectBinInfo3` (4736), `_getFromMapAddressToFeedCode` (4790), `_getPSCodeMSCToENG` (4827), `_PMCounterJoin/Split` (3832/3849), `_getTonerStatus` (3872, `#if 0`) | Hàm tiện ích chuyển đổi mã: trạng thái lắp ráp/tuổi thọ IU, thông tin khay ra, map địa chỉ → khay giấy, **mã cỡ giấy MSC → mã engine**, ghép/tách bộ đếm PM 14+13 bit (`low=14 bit, high 13 bit<<14`). |

### 4.3 Nhận lệnh từ external panel

| Hàm (dòng) | Action |
|---|---|
| `procXPanelReceive()` (2756) | Vòng `while(1)` `ifXPanel.receiveAanalize()` đến khi hết gói, phân loại: ModeMap0/1/2 ⇒ `procXPanel_ModeMapN`; String ⇒ `procXPanel_ModeMapString`; `ModeRequest` ⇒ `comENG.setModeRequest`; **Report** ⇒ phát message cho `TID_OutputEng` (`TM_PrintStartNotice`, `FeedStart`, `PrintEnd`, `PreEject`, `ExpoEnd`, `EjectEnd`, `BundleEject`, `ImageAdjustReport`, `CleaningReport`) hoặc `TID_MachineMan` (`PRTPowerModShift`); `RC_ModeReset` ⇒ `bl_TransStart=True` (hoàn tất kết nối); `CC_MainOff` ⇒ `SystemServiceFunction->reboot()`; `RC_FunctionStart/Stop` ⇒ `comXPanel.FunctionStart/Stop`; các report yêu cầu dữ liệu (`RequestTotalCtFromPRT`, `PMCounterRequest`, `PrinterIndiReq`, `PrinterInfoReq`, `…ContinueReq/StopReq`) ⇒ bật cờ `cc_XPanelDataRequest.*` hoặc gọi `comENG.getCcEngDataRequest()/getCcEngSensorCheck()`; report khởi tạo dữ liệu (≈2866-2869) ⇒ `req_dataInitialize(SystemErrorClear/DataClear, ans_Initialize)` rồi `InitializeEnd()` gửi `RC_MemoryClearEnd`. |
| `procXPanel_ModeMap0(adr,data,str)` (2930) | `A_SystemType`: chọn **vùng đích** → `_fixJapanSetting/_fixUSSetting/_fixEuropeSetting/_fixOther1..4Setting` và cờ factory-adjust; `A_SystemType2`: có HDD ⇒ `setHddExist`, spool Auto/Off, và `req_hddFormatWithNvramClear`; `A_ModeInfo`: admin mode, choice-changing, `req_setServiceModeOn`; `A_FunctionCode`: lưu mã function. |
| `procXPanel_ModeMap1` (3018) | `A_FunctionModeInfo/SetNumInfo/PrinterChoice/EpNetMode/EpNetStatus`, thông tin khay giấy 1–4 + khay tay (`A_xTrayFeedInfo1/2` → loại giấy, cỡ giấy, auto-detect…), `A_NulliDetection`. |
| `procXPanel_ModeMap2` (3137) | Các giá trị điều chỉnh: `A_FuserLoopAdjust`, `A_1Pass/2Pass2ndTransAdjust1/2/3`, `A_1stTransAdjust`… lưu vào `APIC_PrinterSetting`/engine. |
| `procXPanel_ModeMapString` (3202) | Nhận khối chuỗi: `A_TimeString` (đặt **RTC** + timezone), `A_PrinterIndi/Indi2`, `A_PrinterInfo`, `A_TroubleCounter/WarningCounter/CopyModeCounter` (xóa/đặt bộ đếm, PM limit), `A_LogTrouble/LogJam` (xóa log), `A_LPHChipIndi`, `A_EngineCounterInfo` (reset tuổi thọ con lăn…), `A_ColorAdjustData`, `A_Xrite…Data1/2`, `A_RD*` (RDS: modem init, SMTP, POP3, mail, dip-sw), `A_ServiceModeData`, `A_ServiceChoice`. |
| `ans_Initialize(ok)` (2745), `ans_XPanelsetServiceModeOn` (2920), `ans_HDDFormatNVRAM` (2926) | Callback hoàn tất (hầu hết rỗng / chỉ gửi `InitializeEnd`). |
| `_fixJapanSetting` (4251) … `_fixOther4Setting` (4628) | Đặt **mặc định theo vùng bán** (`setSystemType(JP/US/EU/Other)`, cỡ Foolscap, đơn vị, kích cỡ giấy mặc định, ngôn ngữ, …) qua `APIC_MachineSetting/PrinterServiceSetting/PrintJobSetting/SystemSetting`. |
| `_changeComSetting()` (4699) | Tắt auto-detect cỡ giấy khay 1, khay tay = giấy thường. |

---

## 5. Sơ đồ phụ thuộc rút gọn

```
App/UI/PowerManager ──► CNVC_PanelHard::setLED/…/setBackLight2/playSound/…
                           │  (guard: isSuspend, MainSwOFF, PCIe_BUS==NORMAL)
                           ▼
             sysBspIoctlPanelSio(buf, EXDB_PanelKindXxx)
                           ▼
                kernel panelcom driver ──UART──► MCU panel (LED/Backlight/Buzzer/Sound/Touch)
          (illumi: /dev/illumi_unit;  buzzer/NFC pairing: qua thePowerManagerTask)

Engine/Service ◄─MINET─► TASKXPANEL_IF (100 ms) ─ procXPanelTransfer/Receive ─ comXPanel (Function*, ListPrint, MemCheck)
```

## 6. Điểm đáng chú ý (đã tìm thấy khi đọc)

1. **`isSuspend` là cơ chế chính** chặn truy cập panel khi ngủ (`communicationSusped`), nhưng `getPanelType()` (cache) và các hàm đọc port service/`getTroubleReset` **không** có guard.
2. `downloadSoundData` kiểm option bằng `&&` thay vì `&` (dòng 1481).
3. `CC_XPanel::FunctionStart` thiếu `return` sau `FunctionEnd()`/`FunctionAbnormalStop()` ở hai nhánh (xem 3).
4. `TASK_transNFCAuthData` là vòng vô hạn dựa vào ioctl blocking — nếu `panelcom` bị suspend nó phụ thuộc hành vi của driver (không có `isSuspend` check trong task).
5. `setBackLight2(False)` tự động gọi `setBuzzer(Off)` — tắt backlight cũng làm tắt buzzer.
6. Khi `MainSwOFF`, `setLED(on=True)` và `setBackLight2(True)` bị chặn (chỉ cho tắt) — tránh panel sáng lại khi đang tắt nguồn.

*Chưa kiểm chứng: nội dung chi tiết từng `case` trong 4 hàm `procXPanel_ModeMap*` (hàng trăm địa chỉ) chỉ được đọc ở mức nhóm chức năng; muốn biết từng địa chỉ `A_xxx` cụ thể cần đọc tiếp các đoạn 2930–3809 của `THRC_tskXPanelif.cpp`.*
