# Luồng update FW của Subset (chế độ "board setup")

`downloadFW()` không tự ghi FW. Nó là lệnh terminal ở lớp điều phối, gồm kiểm tra điều kiện, kích hoạt, chờ và đọc kết quả. Việc ghi thật nằm trong `fwupdate.cpp`, chạy ở các task riêng. Comment trong source bị lỗi encoding nên tôi suy ra nghĩa từ code và tên hàm. "第1体" gần như chắc chắn là 基板単体, tức board đứng riêng, dùng ở line sản xuất.

## 1. Tổng quan

```
main.cpp (boot)                       Terminal (line sản xuất)
  │ boottyp = USB download?             boardsetup()  → bl_BoardSetup = True
  └─ spawn task FWFileChk               downloadFW()  → luồng bên dưới
        stFileChkTask
        └─ chạy /etc/FWDL_grityinte.sh
           → uc_FileChkStat = OK / NG
```

Có hai điều kiện tiên quyết độc lập với `downloadFW`:

- **`boardsetup()`** phải được gọi trước để bật cờ `bl_BoardSetup`.
- **`stFileChkTask`** phải chạy xong, do `main.cpp` tự spawn khi boot USB download.
## 2. Bước 0: task kiểm tra toàn vẹn (chạy nền từ lúc boot)

[main.cpp:1509-1511](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Subset/S-S800/Src/application/mfp/Linux/main.cpp#L1509-L1511) đọc `/proc/cmdline`. Nếu `boottyp` là USB download thì spawn `stFileChkTask`. Task `stFileChkTask` này ([fwupdate.cpp:14693](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Subset/S-S800/Src/application/mfp/system/thr/thr/fwupdate.cpp#L14693)) làm các việc sau:

1. Đặt `uc_FileChkStat = UNFINISH`.
2. Lấy đường dẫn `INDEX` của Rom download. Đường dẫn 

```
#define FWUP_MOUNT_POINT    "/ram1"

sprintf(sc_DownLoadDir,"%s/INDEX", FWUP_MOUNT_POINT);
```

3. Chạy `THRD_ETCPATH` để thêm `/etc` vào PATH. 
```
export PATH=$PATH:/etc
```
3. Chạy script `/etc/FWDL_grityinte.sh`, tức kiểm tra hash/integrity của file FW.
4. Kết quả là `uc_FileChkStat = OK` hoặc `NG`. Nếu OK thì xóa file tạm (`DEF_PRVNTFALSE_ERACE`).

Mục đích, theo lịch sử sửa RQ-482 ghi trong file, là **rút ngắn thời gian download** bằng cách kiểm tra song song với lúc boot.

## 3. `downloadFW()` từng bước

| #   | Hành động                                                                                                                                                                    | Nếu thất bại                    |
| --- | ---------------------------------------------------------------------------------------------------------------------------------------------------------------------------- | ------------------------------- |
| 1   | Poll `THRG_FileChkstat()` cho đến khi khác `UNFINISH`, ngủ 2 giây mỗi vòng. <br><br>Giá trị trả về của hàm `THRC_FileChkstat()` có liên quan đến hàm `stFileChkTask()` trên. | Không có timeout                |
| 2   | `THRL_CheckBoardsetupUpdateFW()` kiểm tra 3 điều kiện (xem mục 4)                                                                                                            | In `downloadFW NG` rồi return   |
| 3   | `THRL_BoardsetupUpdateFW()` kiểm tra FW có trong TAR và kích hoạt update                                                                                                     | In `downloadFW NG` rồi return   |
| 4   | Poll `THRL_GetFirmwareUpdateFinishChk()` cho đến `True`, `sleep(1)` mỗi vòng                                                                                                 | Không có timeout                |
| 5   | Đọc `THRL_GetFirmwareUpdateFailChk()`, kiểm tra bit `MSC \| MCPF`                                                                                                            | Bit nào set thì `downloadFW NG` |
| 6   | Không có lỗi                                                                                                                                                                 | In `downloadFW OK`              |

Chuỗi `downloadFW OK/NG` in ra bằng `printf` là tín hiệu để công cụ ngoài (line sản xuất) đọc. Nó không phải giá trị trả về.

## 4. Bước 2 chi tiết: `THRL_CheckBoardsetupUpdateFW`

Hàm này là cổng chung. Tất cả lệnh `downloadXxx` (MovieData, BootRom, LPPP, PowerCPU1/2, SSDController) và `SetMachineType` đều gọi nó trước. Cổng gồm 3 điều kiện:

1. `getCompletenessCheckState()` trả về `True`. Đây là cờ `bl_IntergrityOK` trong `Mediaif.cpp`.
2. `THRG_FileChkstat() == THRE_FILECHECK_OK`.
3. `THRG_IsBoardSetup() == True`, tức `boardsetup()` đã được gọi.

`FW_UPDATE_TARGET` được định nghĩa là `THRD_BoardsetupUpdateFW = FarmwareID_MSC | FarmwareID_MCPF`, nghĩa là **MFP controller (MSC) và Mecon (MCPF)**.
## 4.1. Tác dụng của cờ `bl_IntergrityOK`
`bl_IntergrityOK` (tên gốc viết sai chính tả, đúng ra là "Integrity") là cờ toàn cục **"file FW tải xuống có toàn vẹn và không bị làm giả hay không"**. Mặc định là `True` ([Mediaif.cpp:25](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Subset/S-S800/Src/application/mfp/Linux/Platform/Mediaif.cpp#L25)). Cờ chỉ bị chuyển sang `False` khi phát hiện bất thường, và đã sang `False` thì không có nơi nào set lại thành `True`.

### 4.1.1. Cờ được set thế nào

Ở đầu `main()` ([main.cpp:1293-1332](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Subset/S-S800/Src/application/mfp/Linux/main.cpp#L1293-L1332)), Subset kiểm tra một "chữ ký" gắn với đúng board này:

1. Đọc serial number của board (`THRL_Drvrif_SysBspBoardSerialNoPrint_Data`) và ghi ra `/tmp/serial_tmp`.
2. Chạy `openssl dgst -sha256 /tmp/serial_tmp` để tính SHA-256 của serial.
3. Đọc file `/tmp/PRVNTFALSE`, do bootloader hoặc script boot để lại. File này chứa hash mà tầng trước đã tính.
4. So sánh hai hash:
    - **File `/tmp/PRVNTFALSE` không tồn tại** thì `setCompletenessCheckState(0)`, tức NG.
    - **Hash không khớp** thì `setCompletenessCheckState(0)`, tức NG.
    - **Khớp** thì giữ nguyên `True`.
5. Xóa các file tạm `/tmp/PRVNTFALSE` và `/tmp/serial_tmp`.

Tên macro `PRVNTFALSE` là viết tắt của **PreventingFalsification** (chống làm giả), theo comment trong [fwupdate_DEF.h:5](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Subset/S-S800/Src/application/mfp/system/thr/thr/fwupdate_DEF.h#L5).

### 4.1.2. Mục đích

Từ ngày 2020/09/28 (OP_BTS-27780, 27963), luồng này chống một lỗ hổng: người dùng sửa giá trị trong INDEX rồi dùng FW đã bị sửa để download được vào máy. Ý tưởng là:

- Script check `FWDL_grityinte.sh` (chạy trong `stFileChkTask`) xác nhận hash của FW, và sau khi kiểm tra xong sẽ xóa file `PRVNTFALSE`. Vì vậy chuỗi "ghi file, so hash, xóa" phải khớp với board hiện tại.
- Hash gắn với serial board nên **không thể copy kết quả kiểm tra từ máy này sang máy khác**.
- Việc xóa file tạm ngay sau khi so sánh giúp kết quả kiểm tra không dùng lại được.

Lưu ý là dòng comment ở [main.cpp:1338-1344](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Subset/S-S800/Src/application/mfp/Linux/main.cpp#L1338-L1344) cho thấy tùy chọn dòng lệnh `-c` trước đây cũng set NG. Nó đã bị comment out vì "không còn được chạy bằng `-c`" nữa, nên hiện tại chỉ còn cơ chế hash serial ở trên.

### 4.1.3. Ai dùng cờ này (chỉ đọc qua `getCompletenessCheckState()`)

|Nơi dùng|Hành vi khi cờ `False`|
|---|---|
|[pcgcom.cpp:870](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Subset/S-S800/Src/application/mfp/panel/pcgcom.cpp#L870) (phát hiện media USB)|Hiện màn hình `gp_DspFirmwareErr` ("lỗi firmware, không tiếp tục được"), bật LED lỗi, dừng, không download|
|[pcgcom.cpp:1103](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Subset/S-S800/Src/application/mfp/panel/pcgcom.cpp#L1103) (luồng khác của panel)|Tương tự|
|[THRC_BoardSetup.cpp:193](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Subset/S-S800/Src/application/mfp/system/thr/thr/THRC_BoardSetup.cpp#L193) (`THRL_CheckBoardsetupUpdateFW`)|Cổng chung của mọi lệnh `downloadXxx` ở board setup trả về `False` và in `... NG`|

### 4.1.4. Phân biệt với `uc_FileChkStat`

Hai cờ cùng dùng trong cổng `THRL_CheckBoardsetupUpdateFW` nhưng khác nguồn gốc:

| `   bl_IntergrityOK` | `uc_FileChkStat`                                               |                                                                 |
| -------------------- | -------------------------------------------------------------- | --------------------------------------------------------------- |
| Kiểm tra gì          | Chữ ký hash theo serial board (chống làm giả kết quả kiểm tra) | Toàn vẹn của **chính các file FW** (script `FWDL_grityinte.sh`) |
| Khi nào tính         | Một lần, ở đầu `main()`                                        | Trong task nền `FWFileChk`, có trạng thái `UNFINISH/OK/NG`      |
| Giá trị ban đầu      | `True` (lạc quan)                                              | `UNFINISH`                                                      |

Cả hai phải đúng thì FW mới được update.

Phần mô tả mục đích ("chống làm giả") tôi suy ra từ tên macro `PRVNTFALSE`, các ghi chú OP_BTS-27780 và cách so hash theo serial. Tôi chưa đọc script `/etc/FWDL_grityinte.sh` và bootloader nên chưa xác nhận chính xác ai ghi `/tmp/PRVNTFALSE`. Nếu muốn, tôi có thể tìm script đó trong repo để nối nốt mắt xích này.

### 4.1.5. Kịch bản tấn công nếu không có cờ
Giả sử quy trình kiểm tra là: **script kiểm tra hash FW, nếu đạt thì ghi kết quả vào `/tmp/PRVNTFALSE`**, rồi Subset cho phép update.

1. Kẻ tấn công có một USB chứa FW đã bị sửa (chèn code, hoặc dùng FW cũ có lỗ hổng).
2. Họ sửa file `INDEX` trên USB để hash trong đó khớp với FW đã sửa. Script kiểm tra so hash trong INDEX với hash của file nên vẫn báo "OK".
3. Họ còn có thể tạo sẵn file `/tmp/PRVNTFALSE` giả với nội dung "đạt" để đánh lừa Subset.
4. Kết quả: FW giả được ghi vào máy.

Đây chính là lỗ hổng mà comment ghi: _"INDEXの値を書き換えることで、改造したFWがダウンロード可能となる脆弱性"_ (sửa giá trị INDEX thì FW đã chỉnh sửa vẫn download được).

### 4.1.6. Cờ chặn thế nào
Subset không tin file `PRVNTFALSE`. Nó tự tính lại chữ ký từ thứ **kẻ tấn công không giả được là serial thật của board**:

```
Subset tự đọc serial board  → SHA-256  →  hash_A
File /tmp/PRVNTFALSE (do tầng kiểm tra tạo) →  hash_B

hash_A == hash_B  → bl_IntergrityOK = True   (cho update)
khác / không có file → bl_IntergrityOK = False (chặn)
```
#### Ba ví dụ cụ thể

|Tình huống|Kết quả|
|---|---|
|Quy trình đúng: script kiểm tra xong rồi ghi hash theo serial của board này|Hash khớp, cờ `True`, update bình thường|
|Kẻ tấn công tự tạo `/tmp/PRVNTFALSE` với nội dung tùy ý|Hash không khớp serial, cờ `False`, màn hình `FirmwareErr`|
|Không có bước kiểm tra nào chạy (bị bỏ qua) nên không có file|Cờ `False`, bị chặn|
|Copy file `PRVNTFALSE` hợp lệ từ **máy khác** sang|Serial khác, hash lệch, cờ `False`|
Ý nghĩa của từng chi tiết trong code:

- **Hash theo serial board** làm chữ ký gắn với đúng máy này, nên không tái sử dụng được giữa các máy.
- **Xóa `/tmp/PRVNTFALSE` và `/tmp/serial_tmp` ngay sau khi so** làm chữ ký chỉ dùng một lần, tránh replay.
- **Mặc định `True` rồi chuyển `False` khi sai** dùng cách "bị chặn nếu có dấu hiệu bất thường". Nhược điểm là nếu hàm kiểm tra ở `main()` không chạy, cờ vẫn là `True`.

#### Hệ quả cho người dùng và line sản xuất

- **Người dùng cuối:** nếu FW bị làm giả thì màn hình `FirmwareErr` và LED lỗi bật, download bị dừng ngay từ panel ([pcgcom.cpp:870](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Subset/S-S800/Src/application/mfp/panel/pcgcom.cpp#L870)).
- **Line sản xuất (`downloadFW`, `downloadBootRom`...):** cờ `False` khiến cổng `THRL_CheckBoardsetupUpdateFW` trả về `False`, lệnh in `downloadFW NG`. Tránh việc board bị ghi nhầm FW không hợp lệ ngay từ line.

Cơ chế này chỉ chống được người không biết cách tính hash. Ai có quyền ghi vào `/tmp` **và** biết công thức (`sha256` của serial) vẫn tự tạo được file hợp lệ. Vì vậy nó đóng vai trò lớp phòng thủ bổ sung bên cạnh kiểm tra hash FW, không thay thế chữ ký số. Nếu bạn cần, tôi có thể tìm `FWDL_grityinte.sh` trong repo để xác nhận ai thực sự ghi file `PRVNTFALSE`.
## 5. Bước 3 chi tiết: `THRL_BoardsetupUpdateFW`

`THRL_BoardsetupUpdateFW()` kiểm tra bộ FW cần thiết có đủ không rồi kích hoạt update. Nó **không tự ghi FW** và không chờ kết quả. Hàm chỉ có 3 bước.

### Các bước

**1. Lấy danh sách FW có trong gói download**

- Gọi `isExistFarmwareDataTAR()`. Hàm này trả về một bitmask, mỗi bit ứng với một loại FW tìm thấy trong file TAR của Rom download (MSC, MCPF, ENG...).

**2. Kiểm tra gói có đủ FW cần thiết**

- Lấy mask cần thiết: `THRD_BoardsetupUpdateFW = FarmwareID_MSC | FarmwareID_MCPF`. Nghĩa là **MFP controller và Mecon**.
- Tính `ul_ExistFirmwareData & THRD_BoardsetupUpdateFW`, rồi so với mask cần thiết.
- Điều kiện là **cả hai bit phải có**. Thiếu bit nào thì log `Update Fw Check NG` và trả về `False`. Nếu gói có thêm FW khác thì bị bỏ qua, không lỗi.

**3. Kích hoạt update**

- Gọi `startUpdateFromTAR(MSC | MCPF)`. Hàm này chỉ **khởi động** quá trình ghi FW (spawn task ghi MSC, nạp MCPF cho Mecon) rồi return ngay.
- Trả về `True`.

### Sơ đồ

```
isExistFarmwareDataTAR()  → FW nào có trong TAR?
        │
        ▼
   có đủ MSC và MCPF?  ── không ──▶ return False
        │ có
        ▼
startUpdateFromTAR(MSC|MCPF)  → khởi động task ghi nền
        │
        ▼
    return True   (chỉ có nghĩa "đã bắt đầu")
```

### Điểm cần lưu ý

- **Giá trị trả về `True` không có nghĩa update thành công.** Nó chỉ có nghĩa "đã kích hoạt". Kết quả thật sự do `downloadFW()` đọc sau, qua `THRL_GetFirmwareUpdateFinishChk()` và `THRL_GetFirmwareUpdateFailChk()`.
- **Hàm chỉ chạy được sau khi cổng `THRL_CheckBoardsetupUpdateFW()` đã đạt.** Trong `downloadFW()` bước kiểm tra đó nằm ngay trước khi gọi hàm này. Bản thân hàm không tự kiểm tra lại toàn vẹn hay cờ board setup.
- **Lệnh gọi `startUpdateFromTAR` là hàm C-linkage** ([fwupdate.cpp:6120](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Subset/S-S800/Src/application/mfp/system/thr/thr/fwupdate.cpp#L6120)), bọc `FarmwareUpdate::startUpdateFromTAR`. Bên trong nó còn xóa download flag, ghi trước file CardVersion "lỗi", ghi cờ download vào SPI-Flash, rồi mới phân phối task (đã mô tả ở phần trước).
- **Không có xử lý nếu `startUpdateFromTAR` thất bại ngay lúc khởi động.** Hàm đó trả `ushort` nhưng ở đây giá trị bị bỏ qua, và `startUpdateFromTAR` luôn trả 0. Lỗi khi khởi động chỉ lộ ra sau, qua bit lỗi trong `farmwareUpdateFailChk` hoặc việc `bl_CompleteAllDownload` không bao giờ lên `True`.

# 6. Hàm `isExistFarmwareDataTAR` call `existFarmwareDataAtTAR`

## 6.1. Mục đích

Hàm này **quét gói FW (TAR) và trả về một bitmask** cho biết FW nào **vừa có trong gói, vừa áp dụng được cho máy này**. Nó không ghi gì xuống thiết bị. Kết quả `ul_Res` là danh sách "FW có thể download", dùng để quyết định download gì (ví dụ `THRL_BoardsetupUpdateFW` dùng nó để kiểm tra có đủ MSC và MCPF).

Nguyên tắc chung với mỗi FW: **(phần cứng/option có mặt trên máy) VÀ (file tương ứng có trong TAR) thì bật bit**. Chỉ tồn tại trong TAR chưa đủ.

## 6.2. Các bước

**1. Chọn chế độ**

- Nếu là OSS update riêng lẻ qua network, gọi `setOSSBinaryList()` và bật bit `OSS`. Không xét gì thêm trong nhánh này.
- Ngược lại, chạy luồng đầy đủ bên dưới.

**2. MSC (MFP controller)**

- `setMFPBinaryList()` đọc danh sách file trong TAR.
- **ROM debug bị chặn:** nếu không phải ROM Express (`bl_isExpressDownloadFlg == False`) thì trả về `0` ngay, tức không download gì cả.
- Lấy đường dẫn Rom (`getCardVersionPath`), rồi kiểm tra dữ liệu MSC có thật (`THRC_ExistExpressData`) để bật bit `MSC`.
- Ghi log có WebROM (file BRE) hay không, và đọc thông tin FW đang chạy (`ReadCurrentlyFWInfomation`) để dùng cho download differential.

**3. Nhóm FW phụ thuộc cấu hình máy** (bên trong `#if !defined(NVRAMLESS_PART_ONE)`)

|FW|Điều kiện bật bit|
|---|---|
|**MCPF** (Mecon)|`ChkMcpfFW()` thấy trong TAR|
|**ENG** (engine)|Engine không nằm trong MCPF, và `ChkEngFW(version hiện tại)` đạt|
|**IRC** (scanner)|Máy có scanner, IR không nằm trong MCPF, có IR1 trong TAR|
|**FNS** (finisher)|Đã nhận báo cáo cấu hình từ engine, xác định **loại finisher** bằng `switch`, và có image tương ứng trong TAR|
|**RU / ZU**|Máy báo có option RU/ZU và có trong TAR|
|**PK / PI** (punch/PI)|Loại finisher hỗ trợ, máy có chức năng, và có trong TAR|
|**SDL** (saddle)|Finisher có saddle và có trong TAR|
|**EDH** (ADF/DF)|Lấy loại DF (`THRL_GetDFImageType`), có trong TAR. Riêng dòng Sparrow SFP bị loại|
|**FAX 1-4**|Có FAX trong TAR, và từng line FAX thật sự gắn trên máy|
|**DSC1**|Máy có DSC1 và có trong TAR|

Phần `switch` finisher chiếm gần nửa hàm. Nó ánh xạ từng model finisher (FS517, FS537, FS540...) sang **loại image cần dùng** (`sc_ImgeType_FNS/SDL/PK/PI`). Một số model còn phải đọc version ROM của finisher để phân biệt phiên bản CPU (ARM hay Renesas).

**4. Nhóm chỉ khi boot USB download** Kiểm tra sự tồn tại file (`access()`) trong thư mục Rom:

- **BootRom** (`BOT`), **LPPP**, **PS-CPU**, **Panel micom** (`PMF`), **SSD controller FW**.
- Tên file khác nhau cho dòng Eagle0/EagleX.
- Với network update (InternetISW) nhóm này **không** được bật vì không cho download BootROM.

**5. Chốt kết quả**

- Lưu `ul_DownLoadType = ul_Res` và log, rồi trả về `ul_Res`. `ul_DownLoadType` sau đó được `startUpdateFromTAR` dùng để quyết định hiển thị Card Version có phải là download toàn bộ hay không.

## 6.3. Sơ đồ

```
        ┌─ OSS single update? ── có ─▶ chỉ bật bit OSS
        │
existFarmwareDataAtTAR
        │
        ├─ MSC     (chặn nếu ROM không phải Express)
        ├─ MCPF / ENG / IRC
        ├─ Finisher: FNS, RU, ZU, PK, PI, SDL   (switch theo loại finisher)
        ├─ EDH, FAX1-4, DSC1
        └─ (chỉ USB boot) BOT, LPPP, PSCPU, PMF, SSD
                    │
                    ▼
        ul_Res (bitmask)  →  ul_DownLoadType
```

## 6.4. Điểm cần lưu ý

- **Hàm có side effect ngoài việc trả bitmask:** nó set các biến static như `sc_ImgeType_FNS`, `sc_ImgeType_SDL`, `bl_IsOSSUpdate`, `ul_DownLoadType`. Vì thế các lệnh `downloadXxx` phải gọi nó trước rồi mới dùng các hàm `THRL_UpdateXxx`, đúng như code đã làm.
- **Nghi vấn `sc_DownLoadDir` chưa khởi tạo:** buffer này chỉ được `memset` và điền trong nhánh MSC. Nếu `setMFPBinaryList()` trả `False` hoặc đang ở nhánh OSS, mà vẫn rơi vào khối USB download (BOT/LPPP/...), `sprintf` sẽ dùng nội dung chưa khởi tạo. Cần kiểm tra thêm để biết trường hợp này có xảy ra thực tế không.
- **Kiểu dữ liệu lệch:** `bRet` kiểu `Bool` nhưng được so với `-1`, và nhánh lỗi `return False` (0) trả về như "không có FW nào". Các chỗ này có thể chưa đúng ý định ban đầu.
- **Điều kiện bật bit dựa trên cấu hình runtime từ engine** (`comENG`...). Trên board setup đứng riêng có thể các báo cáo này chưa có, nên các bit FNS/RU/ZU sẽ không lên. Điều này không ảnh hưởng luồng board setup vì nó chỉ đòi hỏi MSC và MCPF.

## 6.5. Nó check từng loại FW bằng cách thức nào

Hàm không có một cách kiểm tra chung cho mọi FW. Có **4 kỹ thuật**, mỗi FW dùng một hoặc nhiều kỹ thuật. Tôi đã đọc các helper chính, nhưng phần cuối của `ChkEngFW` (sau dòng 7709) thì chưa.

### 4 kỹ thuật kiểm tra

|Kỹ thuật|Hỏi điều gì|Ví dụ|
|---|---|---|
|**A. Máy có phần cứng/option đó không**|Đọc trạng thái runtime (`comENG`, `pc_SystemInfo`, `comFinisher`)|`isScanerExist()`, `bl_RUOption`, `comFinisher.bl_PunchFunction`, `THRF_IsDsc1Exist()`, `isFaxExistAll(line)`|
|**B. File có trong thư mục Rom không**|`access(path, F_OK)`|Toàn bộ nhóm USB-only (BOT, LPPP...) và mọi lời gọi `analyzeIndex`|
|**C. Chọn đúng file cho máy này**|Suy ra tên/loại image từ model máy, engine, finisher, DF|Bảng `switch` finisher, `JudgeMcpfFW`, `getDownloadMachineInfo`|
|**D. Đối chiếu version**|So version trong binary với version máy đang chạy|`ChkMcpfFW` (`cmpMcpfVersion`)|

Đa số FW là **A + C + B**. Chỉ MCPF có thêm D.

### `analyzeIndex()`, hàm dùng nhiều nhất

Nhận một `E_ImgeType` (enum loại image) và một chế độ (`E_AnalyzeType`). Các bước bên trong ([fwupdate.cpp:5722](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Subset/S-S800/Src/application/mfp/system/thr/thr/fwupdate.cpp#L5722)):

1. Tra `ImageTypeTable[loại]` để lấy **mã 3 ký tự** của loại đó (như `"EG1"`, `"FN3"`).
2. Ghép **tên file** theo quy tắc riêng của từng họ. Ví dụ: ENG là `<prefix>_<mã>.bin`, FAX/DSC/RU/ZU là `<mã>.bin`, finisher là `<prefix FS>_<mã>.bin`.
3. `access(tên file)`. Không có thì trả `False`.
4. Nếu chế độ là `e_FileOnly` thì trả `True` ngay tại đây.
5. Nếu chế độ là `e_IndexAndFile` thì mở tiếp file `INDEX`, quét tìm ký tự `@`, đọc 3 ký tự mã loại, rồi xem **cờ ngay sau đó phải là `'1'`**. Gặp mã `END` thì kết thúc, không thấy thì `False`.

Điểm quan trọng: **mọi lời gọi `analyzeIndex` trong `existFarmwareDataAtTAR` đều dùng `e_FileOnly`**. Nghĩa là các FW như IRC, FNS, RU, ZU, PK, PI, SDL, EDH, FAX, DSC1 chỉ được kiểm tra là **file có tồn tại** chứ chưa kiểm tra cờ trong INDEX. Việc INDEX hợp lệ hay không được xử lý ở chỗ khác (`readIndexFile` trong `setMFPBinaryList`, và script `FWDL_grityinte.sh` đã nói ở phần trước).

### Từng FW cụ thể

|FW|Cách kiểm tra|
|---|---|
|**MSC**|(1) `setMFPBinaryList()` đọc file `INDEX` cùng các INDEX phụ (OSL/CHR/NDJ/UPL/NPLxx) để dựng danh sách file cần ghi. (2) ROM không phải Express thì chặn. (3) `THRC_ExistExpressData`: phải có thư mục `conf`, rồi duyệt từng loại dữ liệu trong bảng `sc_ExpDataTypeTbl`, tìm trong danh sách vừa dựng, rồi `access()` từng file|
|**MCPF**|Phức tạp nhất, xem mục bên dưới|
|**ENG**|Chỉ khi engine không nằm trong MCPF. Từ engine type và machine type suy ra máy đích (`getDownloadMachineInfo`), từ chuỗi version ROM suy ra loại sản phẩm (`getDownloadMachineProductInfo`: Emu, thiết lập, sản xuất...). Hai giá trị này chọn image `EG1..EGD` hoặc `EGZ`|
|**IRC**|A (có scanner, IR không nằm trong MCPF) rồi B (`analyzeIndex IR1`)|
|**FNS**|Chờ `bl_SystemConfigRxEnd`, rồi C: `switch(ec_FinisherType)` chọn loại image. Riêng JS/JS506/FS540 phải đọc thêm byte "spec code" trong ROM version của finisher (ARM hay Renesas). Sau đó B|
|**RU/ZU/PK/PI/SDL**|A (có option, hoặc `bl_PunchFunction`, `bl_PIFunction`, `bl_FoldCenter`) rồi B. PK/PI/SDL còn phụ thuộc loại finisher đã chọn ở trên|
|**EDH (ADF)**|C (`THRL_GetDFImageType` cho biết loại DF) rồi B. Dòng Sparrow SFP bị loại bằng điều kiện machine type|
|**FAX 1-4**|B (có `FAX.bin`), rồi A cho **từng line** (`isFaxExistAll(line)`)|
|**DSC1**|A (`THRF_IsDsc1Exist`) rồi B|
|**BOT, LPPP, PSCPU, PMF, SSD**|Chỉ B (`access` từng file). Tên file đổi theo dòng Eagle0/EagleX. Tên file FW của SSD do `getSsdFwFileName` cấp. Chỉ chạy khi boot USB download|
|**OSS**|Chỉ `setOSSBinaryList()`, không có bước riêng nào khác|

### MCPF chi tiết (`ChkMcpfFW`, [fwupdate.cpp:8563](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Subset/S-S800/Src/application/mfp/system/thr/thr/fwupdate.cpp#L8563))

Đây là chỗ duy nhất có đối chiếu version:

1. Máy phải là kiểu MC-PF: cờ shared memory (ENG/IR/EDH) không được tắt hết, nếu không trả `False`.
2. Thu thập thông tin máy: engine type, machine type, version MC-PF đang chạy (`getRomVersion(TYPE_RomMCPFApp)`), có scanner không, loại DF.
3. `JudgeMcpfFW(...)` chọn **image type MCPF** (MC1..MC8) từ các thông tin đó.
4. **`loadFromTAR()` nạp cả file binary vào RAM** (`p_loadDataArea_MCPF`) ngay lúc kiểm tra.
5. Nếu file tương ứng không có thì thử dự phòng `MCZ` (file dùng chung). Có thì dùng `MCZ`, không có thì `False`.
6. Đọc version nằm trong binary (`readBinaryVersion` ở offset cố định).
7. Nếu loại sản phẩm đã xác định (khác `UNKNOWN`) thì `cmpMcpfVersion(version máy, version binary)`. Không khớp điều kiện so sánh thì `False`. Nếu chưa xác định loại sản phẩm thì bỏ qua bước so sánh này.

Cơ chế này chặn việc **ghi nhầm FW của dòng máy khác** (Emu/thiết lập/sản xuất khác nhau) hoặc **tải bản không tương thích lên**.

### Lưu ý

- **Hàm kiểm tra có side effect và tốn tài nguyên:** `ChkMcpfFW` nạp cả binary vào RAM, và mọi loại FW đều set các biến static `sc_ImgeType_*`. Điều này giải thích vì sao ở `startUpdateFromTAR` và `THRL_BoardsetupUpdateFW` phải gọi hàm này **trước**.
- **Kiểm tra "có file" không đồng nghĩa "file hợp lệ".** Nội dung file chỉ được xác thực ở lớp khác (`bl_IntergrityOK`, `stFileChkTask`), nên có thể nhìn thấy `Found ... in Tar` trong log rồi vẫn bị chặn ở cổng `THRL_CheckBoardsetupUpdateFW` sau đó.
- Các hàm con của ENG (`THRC_JudgeEngImageType`, `THRC_SetChkEngBinSize`, `THRC_JudgeSetEngDLFlg`) chỉ xuất hiện trong comment mô tả `ChkEngFW`. Tôi chưa đọc nội dung chúng nên chưa biết chi tiết bước kiểm tra kích thước binary và cờ download của ENG.

# 7. Bên trong `FarmwareUpdate::startUpdateFromTAR` ([fwupdate.cpp:2307](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Subset/S-S800/Src/application/mfp/system/thr/thr/fwupdate.cpp#L2307))

**Chuẩn bị:**

1. Xóa download flag (`THRG_InitDownloadStatus`, `clearDownloadingFlag`) và lưu `farmwareUpdateTypes`, `farmwareUpdateFinishChk`.
2. Quyết định hiển thị Card Version. Nếu update toàn bộ thì `e_DispOK`, nếu update một phần thì `e_DispNG`. Nếu không có MSC thì `e_NotUpdate`.
3. Ghi trước file CardVersion với giá trị "lỗi" (`makeCardVersionFile`). Nếu update dở dang thì máy hiển thị trạng thái NG.
4. Chỉ với network update: kiểm tra partition layout. Nếu cần dựng lại thì báo lỗi.
5. Với boot type khác network: `updateDownloadingFlg()` ghi cờ download vào SPI-Flash. Lỗi ở bước này thì fail toàn bộ.

**Vòng lặp phân phối** (`while (farmwareUpdateTypes)`):

- Bit MSC thì spawn task `taskMscUpdata` chạy `startMscUpdateTask`.
- Bit MC-family (Mecon/Finisher) thì `startUpdateMCFromTAR()`.

Tôi chưa xác nhận `FarmwareID_MCPF` nằm trong `FarmwareID_MCALL`. Nếu MCPF không nằm trong đó, luồng sẽ rẽ vào nhánh khác. Đây là điều cần kiểm tra.

## Ví dụ so sánh

Hàm này giống **người điều phối công trình** trước khi thợ bắt đầu sửa nhà:

1. Lau bảng, ghi danh sách việc cần làm.
2. Dán biển **"nhà đang sửa, chưa an toàn"** lên cửa. Nếu giữa chừng mất điện, người ta nhìn biển là biết nhà chưa sửa xong.
3. Kiểm tra công trường có làm được không.
4. Gọi từng tổ thợ (mỗi loại FW là một tổ) ra làm việc.
5. Báo "đã bắt đầu" rồi **đi luôn**. Hàm không đứng chờ thợ làm xong.

Ở đây, "FW" là phần mềm chạy trong từng bộ phận của máy (bộ điều khiển chính, scanner, finisher, fax...). "TAR" là file đóng gói chứa tất cả các bản FW mới.

## Các bước

|#|Việc làm|Giải thích đơn giản|
|---|---|---|
|1|Xóa cờ download cũ|Xóa ghi chép của lần update trước để bắt đầu sạch|
|2|Ghi lại "cần update những FW nào"|Tạo **hai danh sách**. Một danh sách để giao việc, một danh sách để theo dõi ai đã xong|
|3|Tăng số lần thử lại (chỉ với update qua mạng)|Đếm số lần đã thử|
|4|Quyết định cách ghi **Card Version**|Card Version là "số phiên bản" hiển thị của máy. Nếu update **toàn bộ** thì hiển thị phiên bản mới. Nếu chỉ update **một phần** thì đánh dấu phiên bản là "không trọn vẹn"|
|5|Ghi trước file Card Version dạng **"lỗi"**|Đây là "tấm biển cảnh báo". Nếu mất điện giữa chừng, máy hiện "update lỗi" thay vì hiện nhầm là đã update xong. Sau khi ghi xong file, giá trị trong bộ nhớ được trả về như cũ|
|6|Kiểm tra ổ đĩa (chỉ update qua mạng)|Nếu cần chia lại ổ đĩa thì **hủy update**, vì chia lại ổ sẽ xóa dữ liệu người dùng và bộ đếm|
|7|Ghi cờ "đang download" vào bộ nhớ Flash (không phải update mạng)|Nếu ghi được thì máy sau này biết "đang update dở". Ghi lỗi thì báo tất cả FW thất bại và **dừng**|
|8|**Giao việc** cho từng loại FW|Xem bảng bên dưới|
|9|Báo "đã khởi động"|Gọi `notificationEvent(FirstTime)` rồi kết thúc|
### Bước 8: giao việc thế nào

|Loại FW|Cách gọi tổ thợ|
|---|---|
|**MSC** (bộ điều khiển chính)|Tạo một **task nền mới** rồi để nó tự chạy|
|**IR (scanner), DSC**|Đọc file FW từ TAR vào RAM, rồi đưa cho module chuyên trách|
|**Mecon, finisher** (nhóm MC)|Gọi hàm khởi động riêng của nhóm này|
|**FAX 1-4**|Đọc file FW vào RAM, đưa cho module FAX của từng đường dây|
|**EDH (ADF)**|Tạo task nền riêng, vì phần này phải chờ lâu và làm treo màn hình panel nếu chạy chung|
|**OSS**|Tạo task nền riêng|

Vòng `while` lặp đi lặp lại: lấy một loại FW trong danh sách, giao việc, gạch tên khỏi danh sách, cho đến khi danh sách trống.

### Ý chính cần nhớ

- **Hàm chỉ "châm ngòi".** Việc ghi FW thật sự chạy nền. Vì vậy `THRL_BoardsetupUpdateFW` trả `True` chỉ có nghĩa là "đã bắt đầu", và `downloadFW()` phải chờ riêng.
- **Hai danh sách có vai trò khác nhau.** `farmwareUpdateTypes` là danh sách giao việc, bị gạch dần trong vòng lặp. `farmwareUpdateFinishChk` là danh sách theo dõi hoàn thành, mỗi FW làm xong sẽ tự gạch tên trong `notificationEvent`. Khi danh sách này rỗng thì tất cả đã xong.
- **Nguyên tắc an toàn:** ghi "lỗi" trước, chỉ ghi "thành công" khi mọi thứ đã hoàn tất.

### Một điểm cần lưu ý

Khi một FW không tìm thấy file (`loadFromTAR` trả NULL), hàm gọi `return` **ngay lập tức**. Các FW đứng sau nó trong danh sách **không được giao việc**, nhưng tên chúng vẫn nằm trong danh sách theo dõi. Nếu đúng như vậy thì cờ "tất cả đã xong" có thể **không bao giờ lên**. Trong luồng board setup, đây có thể là lý do `downloadFW()` bị kẹt ở vòng chờ không có timeout. Đây là suy luận từ code, chưa được kiểm chứng bằng chạy thực tế.

### Flow của `startUpdateFromTAR()`

Hàm có **4 điểm thoát sớm** (đánh dấu ❌) và một vòng phân phối task ở cuối.

```
startUpdateFromTAR(ul_Types, callback)
│
├─ [1] RESET
│     THRG_InitDownloadStatus()
│     clearDownloadingFlag()
│
├─ [2] GHI NHẬN YÊU CẦU
│     farmwareUpdateCallback = callback
│     farmwareUpdateTypes    = ul_Types & FarmwareID_ALL   ← danh sách "giao việc"
│     farmwareUpdateFinishChk = farmwareUpdateTypes        ← danh sách "theo dõi hoàn thành"
│
├─ [3] Boot = Network_Update ?
│        └─ có → APIC_InternetISW::incRetryCnt(1)
│
├─ [4] QUYẾT ĐỊNH CARD VERSION
│     ul_temp = ul_DownLoadType  trừ  BOT, LPPP, PSCPU, PMF, SSD, OSS
│     │
│     ├─ ul_temp có MSC ?
│     │    ├─ có → setCardVersionDisp(NG)
│     │    │        ├─ ul_temp == ul_Types → eCardVerDisp = DispOK   (update toàn bộ)
│     │    │        └─ khác               → eCardVerDisp = DispNG   (update một phần)
│     │    └─ không → eCardVerDisp = NotUpdate
│     │
│     └─ eCardVerDisp != DispOK ?
│          ├─ là OSS update? → getCardVersionFromBoot()
│          │                     └─ lỗi → notify(OSS, UpdateFailed) ❌ return
│          └─ edit_tempCardVersion_partOfChange()
│
├─ [5] GHI FILE CARD VERSION "LỖI" TRƯỚC
│     get_tempCardVersion(lưu bản gốc)
│     edit_tempCardVersion_updateNG()
│     makeCardVersionFile()          ← ghi file dạng "update NG"
│     set_tempCardVersion(khôi phục bản gốc trong RAM)
│
├─ [6] Boot = Network_Update ?
│        └─ có → checkRebuildingPartition(SSD) != 0 ?
│                  └─ có → setISWError(NEED_ISW)
│                          setAllUpdateErrFactor(RE_PARTITION_LAYOUT)
│                          notificationAllUpdateFailed()  ❌ return
│
├─ [7] Boot != Network_Update ?
│        └─ có → updateDownloadingFlg(types)   (ghi cờ vào SPI-Flash)
│                  └─ lỗi → setAllUpdateErrFactor(SPIFLASH_WRITE_ERR)
│                           notificationAllUpdateFailed()  ❌ return
│
├─ [8] uc_McpfErrFactor = NO_ERR
│     ul_DownLoadSelectedType = farmwareUpdateTypes
│
├─ [9] VÒNG PHÂN PHỐI   while (farmwareUpdateTypes != 0)
│     │   (mỗi vòng xử lý đúng MỘT nhánh, theo thứ tự ưu tiên)
│     │
│     ├─ MSC   → gạch bit, spawn task "taskMscUpdata" → startMscUpdateTask
│     ├─ nhóm IR/DSC
│     │     ├─ IRC  → loadFromTAR → NULL? ❌ notify(NotExistFarmwareData) return
│     │     │         ifIRCSub.init(...)
│     │     ├─ DSC1 → (tương tự) ifDSC1Sub.init(...)
│     │     └─ DSC2 → (tương tự) ifDSC2Sub.init(...)
│     ├─ nhóm MC (MCPF, ENG, FNS, RU, ZU, PK, PI, SDL)
│     │         → startUpdateMCFromTAR(bits)  ; gạch cả nhóm
│     ├─ FAX1..FAX4 → loadFromTAR → NULL? ❌ return
│     │              → APIC_Fax::instance(line)->startUpdateFirmware(...)
│     ├─ EDH   → spawn task "taskEdhUpdata"
│     ├─ DSC1  → (nhánh dự phòng, chỉ gạch bit)
│     ├─ OSS   → spawn task "taskOSSUpdate" → updateOSSTask
│     └─ khác  → farmwareUpdateTypes = 0
│
└─ [10] notificationEvent(0, FarmwareUpdateEvent_FirstTime)
        return
```

### Đọc sơ đồ này thế nào

**Giai đoạn chuẩn bị (bước 1-8):** không có FW nào được ghi cả. Chỉ ghi nhận yêu cầu, dán "biển cảnh báo" Card Version, và kiểm tra điều kiện cho phép update. Bất kỳ ❌ nào ở giai đoạn này đều kết thúc mà không có FW nào bị động đến.

**Giai đoạn phân phối (bước 9):** đây là nơi FW thật sự bắt đầu chạy, nhưng chỉ là bắt đầu. Mỗi nhánh làm một trong hai việc: spawn một task nền (MSC, EDH, OSS), hoặc nạp file vào RAM rồi trao cho module chuyên trách (IR, DSC, MC, FAX). Kết quả sau đó quay về `notificationEvent`.

**Bước 10** có tác dụng đặc biệt: `notificationEvent(0, FirstTime)` với `ul_Type = 0` không khớp FW nào trong `notificationEvent`, nhưng nó vẫn chạy đoạn kiểm tra cuối "`farmwareUpdateFinishChk == 0` thì hoàn tất". Nhờ vậy, nếu danh sách yêu cầu rỗng (hoặc mọi thứ đã xong ngay), quá trình hoàn tất vẫn được kích hoạt.

### Bốn điểm thoát sớm

|Điểm|Điều kiện|Ảnh hưởng|
|---|---|---|
|Bước 4|OSS update không đọc được Card Version từ `/boot`|Báo OSS lỗi, các FW khác chưa được giao|
|Bước 6|Update mạng cần chia lại partition|Báo **tất cả** thất bại|
|Bước 7|Ghi cờ SPI-Flash lỗi|Báo **tất cả** thất bại|
|Bước 9|`loadFromTAR` trả NULL (IRC/DSC/FAX...)|Chỉ báo FW đó, các FW còn lại trong danh sách **không được giao**|

Điểm thoát ở bước 9 là điểm khác biệt: các điểm ❌ ở bước 4-7 gọi `notificationAllUpdateFailed` để dọn hết trạng thái, còn ❌ ở bước 9 chỉ báo cho một FW. Vì vậy các bit của FW chưa được giao vẫn còn trong `farmwareUpdateFinishChk`, và như đã nói ở phần trước, cờ hoàn tất có thể không bao giờ lên.

## Output của `startUpdateMCFromTAR()`

### Hàm không trả về gì

Hàm có kiểu `void`. "Output" ở đây là **tác dụng phụ** (biến toàn cục, log, sự kiện). Kết quả thật sự của việc update đến **sau**, bất đồng bộ, qua `notificationEvent`.

### Mỗi lần gọi chỉ khởi động đúng MỘT FW

Hàm là một chuỗi `if / else if`. Dù truyền vào nhiều bit, nó chỉ xử lý **bit đầu tiên khớp** theo thứ tự ưu tiên:

`SDL → PK → PI → FNS → ZU → RU → MCPF → ENG`

Các FW còn lại được kích hoạt **tuần tự**: khi FW đang chạy xong, `notificationEvent` gọi lại `startUpdateMCFromTAR(farmwareUpdateFinishChk)` để chuyển sang FW kế tiếp.

### Kết quả ngay sau khi hàm chạy xong

|Tình huống|Output thu được|
|---|---|
|**Khởi động thành công** (SDL, PK, PI, FNS, ZU, RU, MCPF)|(1) `p_loadDataArea_<FW>` trỏ tới **buffer RAM chứa file FW** đọc từ TAR. (2) Log `"<FW> start."` (MCPF kèm tick). (3) Sub-module được `init` và **bắt đầu truyền FW** sang Mecon hoặc finisher, chạy nền|
|**Không đọc được file** (`loadFromTAR` trả NULL)|Gọi `notificationEvent(<FW>, NotExistFarmwareData)` rồi `return`. Không sub-module nào được khởi động|
|**ENG, kích thước hợp lệ**|`ifENGSub.init(buf)`. Hai cỡ: `BLOCK_NUM_PRT_EMU` (0xfc000) gọi `init(buf, True)`, `BLOCK_NUM_PRT` (0xbc000) gọi `init(buf)`. Kèm log kích thước|
|**ENG, kích thước lạ**|Chỉ ghi log `"... size is NG"`. **Không khởi động gì, không gửi notification**|
|**ENG, buffer NULL hoặc ENG nằm trong MCPF** (`b_SharedMemory_ENG == True`)|`notificationEvent(ENG, NotExistFarmwareData)` rồi `return`|
|Không bit nào khớp|Không làm gì|

### Ai nhận dữ liệu (đường truyền)

|FW|Đường truyền|
|---|---|
|**PK, PI, MCPF**|Luôn qua `ifMCPFSub.initSM(EC_SSCCode_xx, buffer)`: đưa qua **shared memory** cho Mecon|
|**SDL, FNS, ZU, RU**|Nếu `b_SharedMemory_ENG == True` thì đi qua `ifMCPFSub.initSM(...)`. Nếu không thì dùng sub-module riêng (`ifSDLSub`, `ifFNSSub`, `ifZUSub`, `ifRUSub`)|
|**ENG**|`ifENGSub.init(...)`|

### Kết quả cuối cùng đến sau, qua `notificationEvent`

Khi sub-module truyền xong (hoặc lỗi), nó gọi `notificationEvent(<FW>, Success/Failed)`. Lúc này mới có các output thực sự:

1. Bit của FW bị xóa khỏi `farmwareUpdateFinishChk`.
2. Buffer RAM được `free`, con trỏ set về NULL.
3. Nếu lỗi, bit tương ứng bật trong `farmwareUpdateFailChk`, và với MCPF còn ghi nguyên nhân (QSPI/ASIC/WDT) vào `uc_McpfErrFactor`.
4. Gọi tiếp `startUpdateMCFromTAR(farmwareUpdateFinishChk)` để chuyển sang FW kế. Quy tắc nối tiếp **khác nhau**: MCPF và ENG chỉ nối tiếp **khi thành công**. FNS, RU, ZU, PK, PI, SDL nối tiếp **cả khi thất bại**, để một FW hỏng không làm treo cả chuỗi (sửa OP_BTS-39341).
5. Khi `farmwareUpdateFinishChk == 0` thì chạy hậu xử lý và bật `bl_CompleteAllDownload`.

### Ví dụ trong luồng board setup

Với `MSC | MCPF`, MSC được xử lý riêng bằng task, nên phần MC chỉ còn MCPF:

```
startUpdateMCFromTAR(MCPF)
   → loadFromTAR(sc_ImgeType_MCPF)  →  p_loadDataArea_MCPF = buffer
   → log "MC-PF start. [Tick:xxxx]"
   → ifMCPFSub.initSM(EC_SSCCode_MCPF, buffer)   → bắt đầu truyền sang Mecon
   → return

... (nền) truyền xong ...
notificationEvent(MCPF, UpdateSuccess)
   → bỏ bit MCPF, free buffer
   → startUpdateMCFromTAR(phần còn lại)  → trống, không làm gì
   → nếu MSC cũng đã xong thì bl_CompleteAllDownload = True
```

### Lưu ý

- **ENG với kích thước lạ là một ngã ba im lặng:** chỉ ghi log rồi thoát, không notification, nên bit ENG không bao giờ được gạch khỏi `farmwareUpdateFinishChk` và tiến trình hoàn tất bị treo. Đây là suy luận từ code.
- **ENG không tự đọc file trong hàm này:** nó dùng `p_loadDataArea_ENG` đã có sẵn, và `uc_EGZ_Size`. Tôi suy ra hai biến này được set từ bước kiểm tra `ChkEngFW`, nhưng chưa đọc phần cuối hàm đó để xác nhận.
- **Có khả năng rò rỉ bộ nhớ ở MCPF:** `ChkMcpfFW` (lúc kiểm tra) đã nạp binary vào `p_loadDataArea_MCPF`. Hàm này gọi `loadFromTAR` lần nữa và gán đè lên con trỏ đó mà không thấy `free` trước. Tôi chưa đọc `loadFromTAR` nên chưa biết nó có tự xử lý buffer cũ không. Cần kiểm tra thêm.
## 7. Hai nhánh song song

**MSC** (`startMscUpdateTask`, [fwupdate.cpp:4673](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Subset/S-S800/Src/application/mfp/system/thr/thr/fwupdate.cpp#L4673)) chạy tuần tự các bước sau, bước nào lỗi thì gọi `notificationEvent(MSC, UpdateFailed)` và thoát:

```
UpdateBusy → changePermission → FwupFileCopyToSSD (copy FW ra SSD)
→ saveSelectLanguage → setIndexMscCheckSum + setDisableGrityinteFlg
  (bỏ qua nếu là ROM Express)
→ startBackupProc → check_pswc_font → sync
→ THRL_ReCopyBootFile (ghi lại /boot) → xóa cờ PluginIntegrityCheckNG
→ UpdateSuccess
```

**MCPF** (`startUpdateMCFromTAR`, [fwupdate.cpp:3513](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Subset/S-S800/Src/application/mfp/system/thr/thr/fwupdate.cpp#L3513)): dùng `loadFromTAR()` nạp image vào RAM, rồi `ifMCPFSub.initSM(EC_SSCCode_MCPF, ...)` gửi cho Mecon qua shared memory. Nếu `loadFromTAR` trả NULL thì báo `NotExistFarmwareData`.

## 8. Điểm hội tụ: `notificationEvent` ([fwupdate.cpp:3081](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Subset/S-S800/Src/application/mfp/system/thr/thr/fwupdate.cpp#L3081))

Mỗi FW xong (thành công hay thất bại) đều gọi hàm này. Hàm làm những việc sau:

- Với MSC, khi bắt đầu thì xóa thông tin FW differential. Khi thành công thì `BackupDownloadedFWInformation()` để chuẩn bị cho lần download differential sau.
- Xóa bit của FW đó khỏi `farmwareUpdateFinishChk` và free vùng RAM đã nạp.
- Với MCPF thành công thì kích hoạt FW Mecon-family tiếp theo. Nếu thất bại thì ghi nhận nguyên nhân (QSPI clock, ASIC comm, WDT) vào `uc_McpfErrFactor`.

**Khi `farmwareUpdateFinishChk == 0` (tất cả đã xong)**:

1. Nếu `farmwareUpdateFailChk == 0`, gọi `Clear_EnhancedNvramClear()`. Nếu lỗi thì đánh dấu `MSC` lỗi.
2. **Nếu vẫn không lỗi (thành công thật):**
    - Ghi Card Version.
    - Ghi lịch sử update.
    - Tính hash các file INDEX và BootROM rồi lưu vào Flash.
    - `setFWDLStatusInit()`.
3. **Nếu có lỗi:** với network update thì đặt mã lỗi ISW. Trường hợp đặc biệt là repartition xong nhưng download chưa xong thì set status rồi `SubsetReboot()`.
4. Đặt lại retry counter, xóa cờ NWDL và cờ Boot diag.
5. **Cuối cùng mới đặt `bl_CompleteAllDownload = True`.** Chính cờ này giải phóng vòng `sleep(1)` ở bước 4 của `downloadFW`.
6. Khi hoàn tất, nếu không lỗi thì `makeCardVersionFile()` ghi file CardVersion bình thường.

Comment tại [fwupdate.cpp:3420](vscode-webview://1v7kr62lml590cb3gndmegc6ml88sjdjv7262pvi3l6t2rv5t7fr/Subset/S-S800/Src/application/mfp/system/thr/thr/fwupdate.cpp#L3420) giải thích lý do: log OK của lệnh chỉ được phép in sau khi toàn bộ hậu xử lý xong (sửa năm 2018/09/14).

## 9. Nhận xét

- **Hai vòng chờ không có timeout** (bước 1 và bước 4 của `downloadFW`). Nếu `stFileChkTask` treo hoặc không bao giờ được spawn (không phải USB boot), thì `uc_FileChkStat` mãi `UNFINISH`. Nếu task update chết mà không gọi `notificationEvent`, cờ `bl_CompleteAllDownload` mãi `False`. Cả hai trường hợp đều làm terminal treo.
- **Kiểm tra kết quả chỉ nhìn 2 bit** (`MSC|MCPF`). Lỗi ở FW khác trong `farmwareUpdateFailChk` bị bỏ qua. Trong luồng này chỉ 2 loại được update nên chấp nhận được.
- **Điểm đáng chú ý là thứ tự ghi Card Version**: file lỗi được ghi trước, chỉ ghi bản tốt ở cuối. Nếu mất điện giữa chừng, máy sẽ báo update NG thay vì báo version mới.

Tôi chưa đọc nội dung `FwupFileCopyToSSD`, `existFarmwareDataAtTAR`, `loadFromTAR`, `ifMCPFSub.initSM` (giao thức ghi xuống Mecon) và script `FWDL_grityinte.sh`. Nếu bạn cần đào tiếp, phần nào đáng xem nhất là ghi MSC (copy, backup, ReCopyBootFile) hay giao tiếp Mecon? Tôi cũng có thể vẽ sequence diagram cho toàn luồng.