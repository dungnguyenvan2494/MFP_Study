/* dbgAllClear Deep Dive — knowledge check (27 questions, 3 levels) */
(function () {
  'use strict';

  var LEVELS = { b: 'Beginner', i: 'Intermediate', a: 'Advanced' };

  var Q = [
    // ---------------- Beginner ----------------
    { l: 'b', q: 'NVRAM (Non-Volatile RAM) khác RAM thông thường (DRAM) ở điểm nào?',
      o: ['NVRAM nhanh hơn DRAM', 'NVRAM giữ dữ liệu khi mất nguồn, DRAM thì mất', 'NVRAM chỉ đọc, không ghi được', 'NVRAM chỉ dùng cho bootloader'],
      a: 1, e: 'Non-volatile nghĩa là dữ liệu còn sau khi mất nguồn. DRAM cần nguồn liên tục để giữ dữ liệu.' },
    { l: 'b', q: 'Trên các build Linux trong repo, DEVICE_CS1_BASE (gốc của vùng “NVRAM”) trỏ tới đâu?',
      o: ['Một chip NVRAM vật lý trên chip-select CS1', 'Một bộ đệm malloc(512 * 1024) trong RAM', 'Phân vùng /dev/kmsda12', 'Vùng SoftDipSW trên SPI-Flash'],
      a: 1, e: 'InitDevice.h định nghĩa DEVICE_CS1_BASE = ul_CS1_BaseAdr, và InitDevice.cpp gán ul_CS1_BaseAdr = malloc(512 * 1024) (KM3 dòng 209, Subset dòng 207/348). Tên “NVRAM” là di sản; dữ liệu bền vững nằm ở SPI-Flash/SSD.' },
    { l: 'b', q: 'Hàm memset(ptr, 0, n) làm gì?',
      o: ['Cấp phát n byte', 'Ghi byte 0 vào n byte liên tiếp bắt đầu từ ptr', 'Xoá file', 'Erase một sector flash'],
      a: 1, e: 'memset chỉ ghi giá trị lặp lại vào vùng nhớ có thể ghi trực tiếp. Nó không phải thao tác erase flash.' },
    { l: 'b', q: 'Vì sao hệ thống cần NVRAM backup?',
      o: ['Để tăng tốc boot', 'Để khôi phục dữ liệu thiết lập khi vùng chính bị hỏng/mất', 'Để lưu firmware', 'Để lưu log debug'],
      a: 1, e: 'Các class có dữ liệu backup sẽ đi vào nhánh SERR_Fatal (khôi phục) thay vì FactorySetup khi dữ liệu chính mất, xem CheckOfFile.' },
    { l: 'b', q: 'dbgAllClear có phải lệnh shell mặc định của VxWorks không?',
      o: ['Có, là lệnh built-in của VxWorks', 'Không, là hàm C++ của ứng dụng được gọi theo tên symbol', 'Có, là lệnh của U-Boot', 'Không, là script shell Linux'],
      a: 1, e: 'Shell (VxWorks hoặc cuShell giả lập trên Linux) tra tên symbol trong bảng symbol rồi gọi trực tiếp địa chỉ hàm.' },
    { l: 'b', q: 'Phiên bản Zeus KM3 hỗ trợ bao nhiêu giá trị uc_ClearCmdNo có xử lý?',
      o: ['3', '4', '6 (0 đến 5)', '8'],
      a: 2, e: 'switch có case 0, 1, 2, 3, 4, 5; mọi giá trị khác rơi vào default: break (không làm gì, không in gì).' },
    { l: 'b', q: 'Backup group nào KHÔNG nằm trên SPI-Flash trong bản KM3?',
      o: ['Group 1', 'Group 2', 'Group 3', 'Cả ba đều trên SPI-Flash'],
      a: 2, e: 'Group 1/2 là THRC_SPI_FlashCtrlBackup (SPI-Flash). Group 3 là FlashCtrlBackup c_NvramBackup trên phân vùng eMMC/SSD (Mediaif.cpp:1428–1432).' },
    { l: 'b', q: 'Giá trị byte của NVRAM Clear Flag khi bật là gì?',
      o: ['0x01', '0xFF', '0xAB', '0x55'],
      a: 2, e: 'NVDF_setNvramClearFlag(True) ghi 0xab; NVDF_checkNvramClearFlag() so sánh == 0xab.' },
    { l: 'b', q: 'FirstMark là gì?',
      o: ['Chuỗi "NVD10" (5 byte) ở offset 0 của vùng SoftDipSW', 'Magic number "R1" của mỗi class', 'Byte đầu tiên của U-Boot', 'Bộ đếm của backup group'],
      a: 0, e: 'NV_AreaGet() đọc 5 byte ở offset 0 qua THRG_getSoftDipSw và so với NVDD_NVRam_FirstMark = "NVD10".' },

    // ---------------- Intermediate ----------------
    { l: 'i', q: 'NVDF_memsetNvRam gọi Set_Cs2(UNLOCK) trước khi ghi. Trên các build Linux của repo, lời gọi này có tác dụng gì?',
      o: ['Tắt write-protect của chip NVRAM', 'Không có tác dụng: Set_Cs2 có thân hàm rỗng', 'Khoá semaphore', 'Mở file thiết bị /dev/nvram'],
      a: 1, e: 'Set_Cs2() rỗng tại KM3 Linux/Platform/InitDevice.cpp:420, Subset InitDevice.cpp:419 và InitDeviceIt6.cpp:205. Bản VxWorks cũ (sysLib.c) nằm trong #if 0.' },
    { l: 'i', q: 'Trong KM3 Mediaif.cpp, EraseNvramBackupArea(0, 1) làm gì?',
      o: ['Xoá mặt 1 của group 1', 'Xoá cả hai mặt của group 2', 'Xoá mặt 1 của group 2', 'Trả FALSE vì 0 không hợp lệ'],
      a: 1, e: 'ss_area1 = 0 rơi vào default (“両面削除”, xoá cả hai mặt); si_area2 = 1 chọn group 2.' },
    { l: 'i', q: '“Xoá” backup group 1/2 trên SPI-Flash thực chất là gì?',
      o: ['Erase vật lý các sector 4KB', 'Ghi 0x0000 vào trường erase-flag sau vùng dữ liệu (đánh dấu logic)', 'Ghi 0xFF toàn vùng', 'Xoá file trên SSD'],
      a: 1, e: 'eraseBackup() → setEraseEnable() ghi THRD_SPI_FlashEraseFuncEnable (0x0000), 2 byte, tại AreaAdress + 4 + DataSize. Comment trong code: không cần erase vì mọi bit đều OFF.' },
    { l: 'i', q: 'Zeus command 2 tác động lên vùng nào?',
      o: ['Chỉ SoftDipSW', 'Backup group 1, 2, 3', 'Thư mục NVData', 'Chỉ Clear Flag'],
      a: 1, e: 'case 2: EraseNvramBackupArea(0,0), (0,1), (0,2). Không đụng SoftDipSW và không đặt cờ.' },
    { l: 'i', q: 'Command Zeus nào xoá thư mục NVData ngay trong lúc chạy lệnh?',
      o: ['0', '1', '4', '5'],
      a: 1, e: 'Chỉ case 1 gọi FILF_RemoveDirTree(FILC_BaseDirIf::newInstance(FILE_DirTypeNVData)->getPath()). Case 0/4/5 chỉ đặt cờ; dữ liệu SSD được khởi tạo lại ở lần boot sau.' },
    { l: 'i', q: 'Clear Flag được tắt (ghi 0x00) ở đâu?',
      o: ['Cuối dbgAllClear', 'Đầu NVDSInit', 'Cuối NVDSInit, sau vòng FactorySetup, ghi FirstMark và SYSF_DBMediaMapDelete', 'Trong U-Boot'],
      a: 2, e: 'NVDG_FuncZeus.cpp:28760–28768 (nhánh TORNADO && !FUM_LINUX): if sc_InitFlgOfSSD == NVDE_On → SYSF_DBMediaMapDelete(), APIC_SaaSGWSetting::FactorySetup(), NVDF_setNvramClearFlag(False).' },
    { l: 'i', q: 'Vì sao cờ không được tắt ngay trong dbgAllClear?',
      o: ['Do quên', 'Vì cờ là tín hiệu cho lần boot sau; tắt muộn để nếu mất nguồn giữa lúc khởi tạo thì lần boot sau làm lại', 'Vì SPI-Flash không ghi được hai lần', 'Vì cần chờ U-Boot'],
      a: 1, e: 'Comment tại 28758: “NVRAM初期化中での主電源断を考慮し…NVRAM MagicNo書込みタイミングで行う” (tính đến mất nguồn trong lúc khởi tạo).' },
    { l: 'i', q: 'Trên build TORNADO, sc_InitFlgOfSSD được đặt NVDE_On khi nào?',
      o: ['Khi FirstMark sai', 'Khi NVDF_checkNvramClearFlag() trả True (cờ == 0xab)', 'Khi ROM version thay đổi', 'Luôn luôn'],
      a: 1, e: 'NVDG_FuncZeus.cpp:28233–28236. FirstMark sai chỉ đặt sc_InitFlg (cho class NVRAM/SPI-Flash), đây là hai cơ chế tách biệt.' },
    { l: 'i', q: 'Trên Zeus, gõ “dbgAllClear” không kèm tham số trong cuShell (giả sử symbol resolve được) sẽ chạy command nào?',
      o: ['Không chạy gì', 'Command 0 (tác động nhiều nhất)', 'Command 5', 'Báo lỗi thiếu tham số'],
      a: 1, e: 'ShellCtrl::Exec khởi tạo argv[] = {cmdline,} nên argv[1] = NULL; GetArgValue(NULL) trả 0 → uc_ClearCmdNo = 0. Lệnh không hỏi xác nhận.' },

    // ---------------- Advanced ----------------
    { l: 'a', q: 'Donau3 dbgAllClear gọi NVDF_memsetNvRam(NVDD_NVRam_Base, 0, 0x1ffef0). Trên build Linux, điều gì xảy ra?',
      o: ['Xoá đúng 2MB NVRAM vật lý', 'Ghi khoảng 2MB vào bộ đệm malloc 512KB, tràn heap khoảng 1.5MB', 'Không làm gì vì COSIM', 'Bị Set_Cs2 chặn lại'],
      a: 1, e: 'NVDD_NVRam_Base = DEVICE_CS1_BASE + 0x100 và DEVICE_CS1_BASE là malloc(512*1024). 0x100 + 0x1ffef0 = 0x1FFFF0, lớn hơn 0x80000 rất nhiều. Đây là kết luận INFERRED từ hai sự thật đã kiểm chứng.' },
    { l: 'a', q: 'Subset S-S800 gọi EraseNvramBackupArea(0) theo khai báo extern "C" một tham số. Bản được build lại có hai tham số. Hệ quả?',
      o: ['Trình biên dịch báo lỗi', 'Tham số si_area2 không xác định (giá trị rác trong thanh ghi), nên group bị xoá không xác định trước', 'Luôn xoá group 1', 'Linker chọn bản 1 tham số trong bsp_donau'],
      a: 1, e: 'Linkage C không mã hoá kiểu tham số, nên linker ghép hai khai báo khác chữ ký. bsp_donau không được Makefile nào tham chiếu; Linux/Platform/Mediaif.cpp (2 tham số) được build khi BASETYPE=hw.' },
    { l: 'a', q: 'Sau command 3 (chỉ zero SoftDipSW) rồi reboot, trạng thái khởi tạo là gì?',
      o: ['sc_InitFlg = On, sc_InitFlgOfSSD = Off: chỉ class NVRAM/SPI-Flash được kiểm tra lại', 'Cả hai cờ On', 'Không có gì thay đổi', 'Chỉ SSD được khởi tạo'],
      a: 0, e: 'Zero 16376 byte xoá cả FirstMark lẫn byte Clear Flag (offset 5 + offsetof). Class SPI-Flash mất magic "R1": FactorySetRequest, hoặc SERR_Fatal nếu còn backup.' },
    { l: 'a', q: 'FactorySetup hoàn tất nhưng mất nguồn trước NVDF_setNvramClearFlag(False). Lần boot sau?',
      o: ['Boot bình thường', 'Cờ vẫn 0xAB nên NVDSInit lại coi là NVRAM Clear: khởi tạo SSD lại và xoá DB lần nữa', 'Boot loop vĩnh viễn', 'Rơi vào U-Boot recovery'],
      a: 1, e: 'Đây chính là lý do cờ được tắt cuối cùng: quá trình được làm lại. Không có bằng chứng về boot loop trong code đã đọc.' },
    { l: 'a', q: 'Command 0 bị mất nguồn sau khi xoá group 1 nhưng trước khi xoá group 2. Trạng thái nào đúng nhất?',
      o: ['Toàn bộ đã bị xoá', 'SoftDipSW đã zero, G1 đã đánh dấu xoá, G2/G3 còn, cờ CHƯA bật', 'Chỉ cờ được bật', 'Không có gì thay đổi'],
      a: 1, e: 'Thứ tự case 0: setSoftDipSw (zero) → G1 → G2 → G3 → setNvramClearFlag(True). Không có transaction/rollback. Boot sau: FirstMark sai → sc_InitFlg On; cờ = 0 → SSD giữ nguyên (INFERRED).' },
    { l: 'a', q: 'Detail code TYPD_NewDetailCode_Cmn_D390_004B được ghi khi nào?',
      o: ['Command 0: zero SoftDipSW thất bại', 'Command 3: zero SoftDipSW thất bại', 'Command 5: zero SoftDipSW thất bại', 'Ghi Clear Flag thất bại'],
      a: 1, e: '004A ↔ case 0, 004B ↔ case 3, 004C ↔ case 5. Giá trị trả về của THRG_setSoftDipSw được OR vào bit 32–39. Lỗi ghi cờ dùng D390_0047 trong NVDC_SPIFlashDataManage::setFlag.' },
    { l: 'a', q: 'EraseNvramBackupArea trả về BOOL. dbgAllClear (cả ba bản) xử lý giá trị này thế nào?',
      o: ['Retry 3 lần', 'Bỏ qua: không kiểm tra giá trị trả về', 'Ghi detail code', 'Dừng lệnh và trả lỗi'],
      a: 1, e: 'Chỉ có printf “Fail to erase …” bên trong EraseNvramBackupArea. dbgAllClear vẫn tiếp tục và (case 0) vẫn bật cờ.' },
    { l: 'a', q: 'Nếu chỉ chạy command 4 (bật cờ, không xoá backup), class SSD có dữ liệu backup hợp lệ sẽ đi theo nhánh nào khi boot?',
      o: ['FactorySetRequest', 'SERR_Fatal + DeleteRecord, rồi NVDC_setWakeUpFatal() (đường khôi phục)', 'OK, giữ nguyên', 'Crash'],
      a: 1, e: 'NVDC_MediaAccessSSD::CheckOfFile (dòng 373): InitFlg On và có backup (judgment, magic, class data) → SERR_Fatal + DeleteRecord. Vòng FactorySetup gọi NVDC_setWakeUpFatal() cho SERR_Fatal có backup. Chi tiết hàm này chưa được truy vết (UNVERIFIED).' },
    { l: 'a', q: 'Vì sao có thể nói chuỗi “omit the first 256byte” trong Zeus dbgAllClear không mô tả đúng thao tác thực tế?',
      o: ['Vì Zeus xoá cả 256 byte đó', 'Vì Zeus zero vùng SoftDipSW từ offset 0 (16376 byte); 256 byte chỉ tương ứng với NVDD_NVRam_Base = CS1 + 0x100 của mô hình NVRAM cũ', 'Vì 256 byte là của U-Boot SPL', 'Vì THRG_setSoftDipSw tự bỏ qua 256 byte'],
      a: 1, e: 'THRG_setSoftDipSw(0, uc_Buff, sizeof(uc_Buff)) ghi từ offset 0 của vùng dữ liệu SoftDipSW. Không có đoạn code nào loại trừ 256 byte trong đường đi của Zeus.' }
  ];

  var state = { level: 'all', answers: {} };
  var root, scoreEl, resultEl;

  function visible() { return Q.map(function (q, i) { return i; }).filter(function (i) { return state.level === 'all' || Q[i].l === state.level; }); }

  function updateScore() {
    var idx = visible();
    var answered = 0, correct = 0;
    idx.forEach(function (i) {
      if (state.answers[i] !== undefined) { answered++; if (state.answers[i] === Q[i].a) correct++; }
    });
    scoreEl.textContent = 'Điểm: ' + correct + ' / ' + idx.length + '  ·  Đã trả lời: ' + answered;
    if (answered === idx.length && idx.length) {
      var pct = Math.round(correct / idx.length * 100);
      var msg = pct >= 85 ? 'Rất tốt: bạn nắm vững vòng đời dbgAllClear.' :
        pct >= 60 ? 'Khá: hãy xem lại các chương có câu sai.' : 'Nên đọc lại chương Zeus KM3, Clear Flag và Boot Sequence.';
      resultEl.innerHTML = '<div class="callout ' + (pct >= 60 ? 'ok' : 'warn') + '"><div class="callout-title">Kết quả: ' +
        correct + '/' + idx.length + ' (' + pct + '%)</div><p>' + msg + '</p></div>';
      resultEl.classList.add('show');
    } else {
      resultEl.classList.remove('show');
      resultEl.innerHTML = '';
    }
  }

  function render() {
    root.innerHTML = '';
    visible().forEach(function (i, n) {
      var item = Q[i];
      var wrap = document.createElement('div');
      wrap.className = 'q';
      var num = document.createElement('div');
      num.className = 'q-num';
      num.textContent = 'Câu ' + (n + 1) + ' · ' + LEVELS[item.l];
      var text = document.createElement('div');
      text.className = 'q-text';
      text.textContent = item.q;
      var opts = document.createElement('div');
      opts.className = 'q-opts';
      var exp = document.createElement('div');
      exp.className = 'q-explain';
      item.o.forEach(function (o, k) {
        var b = document.createElement('button');
        b.type = 'button';
        b.textContent = String.fromCharCode(65 + k) + '. ' + o;
        b.addEventListener('click', function () {
          if (state.answers[i] !== undefined) return;
          state.answers[i] = k;
          paint();
          updateScore();
        });
        opts.appendChild(b);
      });
      function paint() {
        var chosen = state.answers[i];
        if (chosen === undefined) return;
        wrap.classList.add('answered');
        Array.prototype.forEach.call(opts.children, function (b, k) {
          b.disabled = true;
          if (k === item.a) b.classList.add('correct');
          if (k === chosen && chosen !== item.a) b.classList.add('wrong');
        });
        exp.innerHTML = '';
        var strong = document.createElement('strong');
        strong.textContent = chosen === item.a ? 'Chính xác. ' : 'Chưa đúng. Đáp án: ' + String.fromCharCode(65 + item.a) + '. ';
        exp.appendChild(strong);
        exp.appendChild(document.createTextNode(item.e));
      }
      wrap.appendChild(num);
      wrap.appendChild(text);
      wrap.appendChild(opts);
      wrap.appendChild(exp);
      root.appendChild(wrap);
      paint();
    });
    updateScore();
  }

  function init() {
    root = document.getElementById('quiz-root');
    if (!root) return;
    scoreEl = document.getElementById('quiz-score');
    resultEl = document.getElementById('quiz-result');
    Array.prototype.forEach.call(document.querySelectorAll('[data-quiz-level]'), function (b) {
      b.addEventListener('click', function () {
        state.level = b.getAttribute('data-quiz-level');
        Array.prototype.forEach.call(document.querySelectorAll('[data-quiz-level]'), function (x) {
          x.setAttribute('aria-selected', x === b ? 'true' : 'false');
        });
        render();
      });
    });
    document.getElementById('quiz-reset').addEventListener('click', function () { state.answers = {}; render(); });
    var total = document.getElementById('quiz-total');
    if (total) total.textContent = String(Q.length);
    render();
  }

  if (document.readyState === 'complete') init(); else document.addEventListener('DOMContentLoaded', init);
})();
