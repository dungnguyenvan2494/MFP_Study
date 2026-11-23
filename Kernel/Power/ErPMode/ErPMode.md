# ErP Auto Power Off

```c
typedef uchar TYPE_OffPowerMode;							///< オフ時電源モード設定.
static const uchar TYPE_OffPowerModeErPAutoPowerOff =		0;		///< ErP Auto Power Off.
static const uchar TYPE_OffPowerModeSleep =					1;		///< スリープ.
```

Trong code ở đây, **ErP Auto Power Off** là chế độ tiết kiệm điện sâu hơn Sleep thường. “Power Off” là tên chế độ nguồn do firmware quản lý, không nhất thiết đồng nghĩa với tắt công tắc nguồn vật lý. Mã nguồn còn đặt cờ cấu hình ErP 0W, nhưng con số công suất thực tế phụ thuộc phần cứng và thiết kế máy.

Cấu hình **Off Power Mode** có hai lựa chọn: `ErP Auto Power Off` và `Sleep`, theo `APIC_Setting_SystemSetting.h:1026`.

Khi Weekly Timer yêu cầu chuyển sang chế độ ngoài giờ, `reserveErpModeReq(False)` sẽ đọc cấu hình này: chọn ErP thì đặt yêu cầu ErP; chọn Sleep thì không đặt yêu cầu ErP và đi theo Sleep thường. Xem `PWRC_PowerInfoManager.cpp:15395`.

```c
void PWRC_PowerInfo::reserveErpModeReq( Bool b_Force )
{
    e_ChangeERP = TYPE_ChangeERP_None;	// ERPへの移行手段(ERPモードへの移行なし). 
    if( b_Force == True )
    {
        e_ChangeERP = TYPE_ChangeERP_PowerKey;	// ERPへの移行手段(電源キー押下によるERPモード). 
// 2015/11/20【MAv4:RQ-502】機能カウンタ追加 日立 奈良 Start.
        _setPowerSaveCounter( PWRE_PowerSaveCounter_ErP );				// 省電力関連カウンタインクリメント要求処理.
// 2015/11/20【MAv4:RQ-502】機能カウンタ追加 日立 奈良 End.
    }
    if( pc_ApiSystemSetting != NULL && b_Force == False )	// 通常実行(WTによる実行). 
    {
        // オフ時電源モード設定の取得. 
        if( pc_ApiSystemSetting->getOffPowerMode() == TYPE_OffPowerModeErPAutoPowerOff )	// ErP Auto Power Off. 
        {
            e_ChangeERP = TYPE_ChangeERP_WeeklyTimer;	// ERPへの移行手段(ウィークリータイマーによるERPモード). 
        }
// 2016/08/08【MAv4:RQ-502】機能カウンタ追加(低電力→WTスリープをカウントしない不具合対応) 日立 西原 Start.
        _setPowerSaveCounter( PWRE_PowerSaveCounter_Sleep_ErP );
// 2016/08/08【MAv4:RQ-502】機能カウンタ追加(低電力→WTスリープをカウントしない不具合対応) 日立 西原 End.
    }
    if( b_PowerLogON == True )
    {
        DBUGPRINT("□■[Power][reserveErpModeReq] ChangeERP=[%d]■□\n",e_ChangeERP,0,0,0);
    }
    Bool b_Erp = ( e_ChangeERP == TYPE_ChangeERP_None ) ? False : True;
    if( st_PowerVariableLogTmp.b_ErpModeReq != b_Erp )
    {
        st_PowerVariableLogTmp.b_ErpModeReq = b_Erp;
        writePowerVariableLog( st_PowerVariableLogTmp );
    }
}
```

Luồng yêu cầu ErP được kiểm tra và thực thi tiếp trong `PWRC_PowerInfoManager.cpp:15467`, sau đó gọi phần điều khiển ErP của máy tại `PWRC_PowerImpColor.cpp:7590`. Trước khi thực thi, firmware cũng tính thời điểm sự kiện ErP gần nhất để đặt thời gian đánh thức nếu có.

```c
void PWRC_PowerInfo::chkExecErpMode()
{
    if( b_PowerSaveModeDoing == False )				// 節電モードに移行中ではない. 
    {
        // 現在の電源状態をErp Mode中に更新. 
        s_CurrentMode.b_ErpMode = True;
        
        // 現在までの省電力累積利用時間を記録する.
        UpdateAccumulatedEcoTime( PWRD_EcoTimeStatusErp, False );

        // ERPモードの開始日時を保存する. 
        SYSS_AbsTime sNowTime;
        time_t tNowTime = SYSC_Timer::getRtcTime();
        SYSC_Timer::timeConvert2( tNowTime, &sNowTime );
        if( pc_SystemInfo != NULL )
        {
            pc_SystemInfo->setERPStartTime( &sNowTime );
        
            // ERPへの移行手段を保存する. 
            pc_SystemInfo->setERPChangeType( e_ChangeERP );
        }
        // ログ｢機種固有部へErpモード実行通知｣を設定.
        _setPowerDbgSequenceLog( PWRD_ExecErpModeRequest );

        // Erpモードへの移行要求をクリアする. 
        resetErpModeReq();
        DBUGPRINT("(%010u) (sys:PowerControl) [chkExecErpMode]\n",SYSF_tickGet(),0,0,0);

        APIC_AuthDeviceCommon* pc_AuthDeviceCommon = APIC_AuthDeviceCommon::newInstance();
        if( pc_AuthDeviceCommon != NULL )
        {
            //Erpモードに移行することを通知.
            pc_AuthDeviceCommon->notifyErpAutoPowerOff();
        }
        if( pc_PowerImp != NULL )
        {
            // Erpモード実行通知. 
            pc_PowerImp->execErpMode();
        }
        else
        {
            DBUGPRINT("(%010u) (sys:PowerControl) PowerImp instance pointer is NULL!!!\n",SYSF_tickGet(),0,0,0);
        }
    }
}
```

## Hàm `chkExecErpMode`

`PWRC_PowerInfo::chkExecErpMode()` là bước ​**xác nhận và khởi chạy ErP ở tầng quản lý nguồn chung**​. Nó chỉ làm việc nếu chưa có chuyển Power Save nào đang diễn ra (`b_PowerSaveModeDoing == False`).
Khi được phép, hàm:

1. Đánh dấu trạng thái nội bộ là đang ở ErP.
2. Cập nhật thời gian tích lũy đã sử dụng ở chế độ ErP.
3. Lưu thời điểm bắt đầu ErP và nguyên nhân chuyển sang ErP.
4. Ghi log rồi xóa yêu cầu ErP đã được đặt trước đó.
5. Báo cho module xác thực biết máy sắp vào ErP.
6. Gọi `pc_PowerImp->execErpMode()` để phần điều khiển theo model thực hiện các thao tác phần cứng, như cập nhật LED, đặt wake-up timer và gửi yêu cầu chuyển nguồn.

Nói gọn: **`chkExecErpMode()` chốt trạng thái và thông báo các module; `execErpMode()` ở phần cứng/model mới xử lý các bước chuyển nguồn cụ thể.**

**Ví dụ dễ hiểu:** đặt Weekly Timer không sử dụng máy vào ban đêm, và `Off Power Mode = ErP Auto Power Off`. Đến giờ, máy kiểm tra điều kiện chuyển nguồn; nếu không có job hay điều kiện cấm, nó chuyển sang ErP để giảm điện năng hơn Sleep thông thường. Nếu đặt `Off Power Mode = Sleep`, cùng lịch đó chỉ đưa máy vào Sleep.

Lưu ý: lời gọi `reserveErpModeReq(True)` ở nhánh xử lý Power Key là yêu cầu ​**ép ErP do thao tác phím**​, khác với `reserveErpModeReq(False)` của Weekly Timer, vốn còn phụ thuộc cấu hình `Off Power Mode`.

## Hàm `execErpMode`

Ở mức high level, `execErpMode()` chuẩn bị máy và yêu cầu chuyển sang ​**ErP**​:

1. Cập nhật trạng thái LED theo trạng thái nguồn hiện tại, với cờ ErP bật.
2. Ghi log cho biết yêu cầu thực thi ErP đã bắt đầu.
3. Tính thời điểm gần nhất máy cần thức dậy. Nếu có thời điểm hợp lệ, đăng ký thời gian đó với RTC để máy có thể tự thức đúng lúc.
4. Với một số model/cấu hình DipSW, yêu cầu quạt hộp chuyển sang mức `LOW`
5. Gọi `req_DeepSleepShift(THRE_ReqShiftType_Erp, 0)` để chuyển yêu cầu ErP xuống tầng quản lý nguồn phần cứng.
6. Khôi phục thông tin ngữ cảnh ghi nhận mã chi tiết.

Tóm lại, hàm này chủ yếu ​**chuẩn bị wake-up timer, xử lý LED/quạt theo cấu hình, rồi gửi yêu cầu ErP xuống tầng thấp hơn**​. Nó không tự triển khai toàn bộ quá trình tắt nguồn phần cứng ngay trong hàm này.

## Hàm `reserveErpModeReq()`

`reserveErpModeReq()` ​**đặt hoặc xóa yêu cầu ErP**​, chưa trực tiếp chuyển máy sang ErP.

1. Đầu tiên, hàm xóa yêu cầu/nguyên nhân cũ bằng cách đặt `e_ChangeERP = TYPE_ChangeERP_None`.
2. Nếu `b_Force == True`, hàm luôn đặt yêu cầu ErP với nguyên nhân `TYPE_ChangeERP_PowerKey` và cập nhật bộ đếm ErP. Trường hợp này không cần đọc cấu hình `Off Power Mode`.
3. Nếu `b_Force == False` và có `pc_ApiSystemSetting`, hàm đọc `Off Power Mode`:
   * Chọn `ErP Auto Power Off` thì đặt yêu cầu ErP với nguyên nhân `TYPE_ChangeERP_WeeklyTimer`.
   * Chọn `Sleep` thì không đặt yêu cầu ErP.
   * Dù chọn ErP hay Sleep, nhánh này vẫn gọi `_setPowerSaveCounter(PWRE_PowerSaveCounter_Sleep_ErP)` để cập nhật bộ đếm phù hợp.
4. Cuối cùng, hàm ghi log nếu bật log và cập nhật trạng thái ErP vào log biến khi trạng thái có thay đổi.

## Hàm `_chkPowerModeToDeepSleepTimeout`

* `_chkPowerModeToDeepSleepTimeout()` định kỳ quyết định có tiếp tục đếm, đặt lại hay dừng bộ đếm.

Ở mức high level, hàm kiểm tra định kỳ:

1. Máy có đang ở Sleep hoặc SubPowerOff, kể cả các trạng thái chuyển tiếp tương ứng, không?
2. Có yêu cầu ErP đang chờ không? Nếu có, kiểm tra thời điểm thức dậy kế tiếp và khả năng vào DeepSleep; nếu phù hợp thì tiếp tục kiểm tra bộ đếm theo nhánh ErP.
3. Nếu không có yêu cầu ErP, kiểm tra máy chưa DeepSleep và không bị cấm DeepSleep; sau đó đánh giá các điều kiện như cờ cấm, sự kiện thức dậy sắp đến, bộ đếm Good Morning và trạng thái USB.
4. Nếu trạng thái nguồn hoặc điều kiện không còn phù hợp, dừng hoặc reset bộ đếm.

### Sự khác nhau giữa **Sleep** và **SubPowerOff**

Trong firmware này, **Sleep** và **SubPowerOff** đều là trạng thái tiết kiệm điện, nhưng không giống nhau:

| Trạng thái                                    | Ý nghĩa                                                                                                                                                                                                                                        |
| ------------------------------------------------- | -------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------------- |
| `TYPE_PowerSaveStatusSleep`(`8`)        | Máy đang ở​**Sleep**​. Thiết bị được giữ ở trạng thái ngủ để có thể thức dậy theo các sự kiện được phép, chẳng hạn phím, timer hoặc sự kiện mạng tùy cấu hình/model.                                 |
| `TYPE_PowerSaveStatusSubPowerOff`(`12`) | ​**Nguồn phụ đã tắt**​. Đây là trạng thái sâu hơn Sleep thông thường; phần lớn thiết bị không hoạt động, chỉ còn các mạch/nguồn cần thiết để phát hiện những điều kiện bật lại được hỗ trợ. |

`SubPowerOff` ​**không nhất thiết là công tắc nguồn chính đã tắt**​. Code theo dõi riêng trạng thái công tắc chính (`getMainSwOFF()`), nên không nên đánh đồng SubPowerOff với mất nguồn hoàn toàn.

Trong mã, hai trạng thái này thường được xử lý tương tự khi xét chuyển tiếp DeepSleep/ErP, nhưng vẫn là hai trạng thái riêng. Enum và giá trị được khai báo trong `APIC_Power.h`.

## Hàm `_chkEnablePowerModeToDeepSleepCounter()`

Hàm đang được chọn là `PWRC_PowerSequence.cpp:11881`. Ở mức high level, hàm kiểm tra bộ đếm chờ đã đủ để bắt đầu chuyển sang **DeepSleep hoặc ErP** chưa.

1. Nếu công tắc nguồn chính đã tắt, hàm dừng ngay, không yêu cầu chuyển tiếp.
2. Bộ đếm chỉ tăng khi trạng thái nguồn là `Sleep` hoặc `SubPowerOff`; nếu không, bộ đếm được đặt lại về 0.
3. Khi bộ đếm đạt ngưỡng và hệ thống không đang xử lý một lần chuyển nguồn khác, hàm dừng bộ đếm và kiểm tra lại lịch đánh thức.

Có **ba lý do** cho đoạn xử lý đó:

1. **Chờ máy ở Sleep/SubPowerOff ổn định đủ 1 giây.**
   Bộ đếm chỉ tăng khi trạng thái là `Sleep` hoặc `SubPowerOff`, mỗi lần tăng theo chu kỳ 500 ms. Ngưỡng `PWRD_PowerModeToDeepSleepTimeout` được định nghĩa là `1000 ms` trong `PWRC_PowerInfoManager.h:58`. Code không ghi chú rõ vì sao chọn đúng 1 giây; có thể khẳng định đây là khoảng chờ trước khi yêu cầu chuyển tiếp, chứ không nên khẳng định mục đích thiết kế sâu hơn.
2. **Không chạy chồng với một chuyển trạng thái khác.**
   Điều kiện `_getPowerSaveDoing() == False` nghĩa là quy trình power-save hiện không bận. Nếu đang chuyển trạng thái, hàm chưa gửi yêu cầu chuẩn bị DeepSleep/ErP.
3. **Dừng bộ đếm để chỉ gửi yêu cầu một lần, rồi cập nhật timer đánh thức.**
   Khi đủ điều kiện, bộ đếm được đặt về `-1`, nên các chu kỳ 500 ms sau không lặp lại yêu cầu. Trước khi gửi yêu cầu phần cứng, hàm gọi `procWeeklyTimerExecDecision()`, hàm này tính thời gian còn lại của Weekly Timer và Lunch Timer rồi đăng ký timer gần nhất nếu còn hạn, tại `PWRC_PowerInfoManager.cpp:15746`. Nhờ vậy việc vào chế độ ngủ sâu không làm bỏ lỡ lịch đánh thức đang đếm.

Sau đó, tùy `b_Erp`, hàm yêu cầu phần cứng **chuẩn bị ErP** hoặc ​**chuẩn bị DeepSleep**​. Đây vẫn là yêu cầu chuẩn bị; không phải chính đoạn này trực tiếp đưa phần cứng vào trạng thái ngủ.

4. Theo tham số `b_Erp`, hàm chọn nhánh:

* `True`: yêu cầu phần điều khiển theo model ​**chuẩn bị ErP**​.
* `False`: yêu cầu ​**chuẩn bị DeepSleep**​.

5. Nếu không lấy được đối tượng điều khiển phần cứng (`pc_PowerImp`), hàm chỉ ghi log lỗi.

Nói gọn, đây là ​**bước chờ và khởi phát yêu cầu chuẩn bị**​. Nó không tự thực hiện lệnh ngủ cuối cùng; phần model/hardware trả lời qua callback, rồi luồng nguồn tiếp tục.

