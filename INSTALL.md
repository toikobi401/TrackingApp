# Hướng dẫn cài đặt tự động chạy khi khởi động

## 🚀 Cài đặt nhanh

### Bước 1: Build chương trình (nếu chưa có .exe)
```powershell
gcc -Wall -O2 main.c -o device_tracker.exe -I"curl-8.11.1_1-win64-mingw/include" -L"curl-8.11.1_1-win64-mingw/lib" -lcurl -lws2_32 -liphlpapi
```

### Bước 2: Chạy script cài đặt
1. **Right-click** vào file `install-startup.ps1`
2. Chọn **"Run with PowerShell"**
3. Nếu gặp lỗi về Execution Policy, chạy lệnh sau trong PowerShell (Administrator):
   ```powershell
   Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
   ```
4. Chạy lại `install-startup.ps1`

### Hoặc chạy bằng PowerShell:
```powershell
# Mở PowerShell với quyền Administrator
# Right-click vào PowerShell -> Run as Administrator

# Chuyển đến thư mục project
cd "C:\Users\PC\OneDrive\Desktop\LEARNING\Self Study\TrackApp"

# Chạy script cài đặt
.\install-startup.ps1
```

## ✅ Sau khi cài đặt

Chương trình sẽ:
- ✅ Copy các file cần thiết vào `C:\ProgramData\DeviceTracker\`
- ✅ Tạo Task Scheduler tên "DeviceTracker"
- ✅ Tự động chạy mỗi khi Windows khởi động (sau 30 giây)
- ✅ Gửi email với thông tin vị trí thiết bị

## 📍 Kiểm tra cài đặt

### Cách 1: Task Scheduler GUI
1. Nhấn `Win + R`
2. Gõ: `taskschd.msc`
3. Tìm task "DeviceTracker" trong danh sách

### Cách 2: PowerShell
```powershell
Get-ScheduledTask -TaskName "DeviceTracker"
```

### Cách 3: Test chạy thủ công
```powershell
Start-ScheduledTask -TaskName "DeviceTracker"
```

## 🔧 Quản lý

### Tạm dừng (Disable)
```powershell
Disable-ScheduledTask -TaskName "DeviceTracker"
```

### Bật lại (Enable)
```powershell
Enable-ScheduledTask -TaskName "DeviceTracker"
```

### Xem log lần chạy cuối
```powershell
Get-ScheduledTaskInfo -TaskName "DeviceTracker"
```

### Gỡ cài đặt hoàn toàn
```powershell
.\uninstall-startup.ps1
```

## 📧 Kiểm tra Email

Sau khi cài đặt và khởi động lại máy (hoặc test chạy), kiểm tra email của bạn.

Email sẽ có dạng:
```
Subject: [Device Tracker] Thiết bị WAR-MACHINE đã khởi động

=== THÔNG TIN THIẾT BỊ ===

Tên thiết bị: WAR-MACHINE
Người dùng: PC
Hệ điều hành: Windows (Version: 10.0 Build 19045)
Địa chỉ IP: 113.23.49.134
MAC Address: C8:7F:54:A4:4A:05
Vị trí: Hanoi, Vietnam (21.0184, 105.8461)
Thời gian: 2025-11-25 14:30:45
```

Bạn có thể click vào tọa độ để xem trên Google Maps!

## ⚠️ Lưu ý

1. **Internet Connection**: Chương trình cần kết nối internet để:
   - Lấy địa chỉ IP public
   - Lấy thông tin vị trí
   - Gửi email

2. **Delay 30 giây**: Task được cấu hình delay 30 giây sau khi boot để đảm bảo có internet.

3. **Chạy với quyền SYSTEM**: Task chạy với tài khoản SYSTEM để hoạt động ngay cả khi chưa đăng nhập.

4. **File config.h**: Đảm bảo đã cấu hình đúng email và SMTP password trước khi cài đặt.

## 🛠️ Troubleshooting

### Lỗi "Execution Policy"
```powershell
Set-ExecutionPolicy -ExecutionPolicy RemoteSigned -Scope CurrentUser
```

### Task không chạy
- Kiểm tra: `Get-ScheduledTaskInfo -TaskName "DeviceTracker"`
- Xem Last Result (0 = thành công)
- Chạy thử thủ công: `Start-ScheduledTask -TaskName "DeviceTracker"`

### Không nhận được email
- Kiểm tra config.h có đúng email và password không
- Test chạy thủ công: `.\device_tracker.exe`
- Xem output để biết lỗi cụ thể

### Muốn rebuild và cài lại
```powershell
# Gỡ cài đặt
.\uninstall-startup.ps1

# Build lại
gcc -Wall -O2 main.c -o device_tracker.exe -I"curl-8.11.1_1-win64-mingw/include" -L"curl-8.11.1_1-win64-mingw/lib" -lcurl -lws2_32 -liphlpapi

# Cài đặt lại
.\install-startup.ps1
```

## 📂 Vị trí file

- **Chương trình**: `C:\ProgramData\DeviceTracker\`
- **Task Scheduler**: Task Library → DeviceTracker
- **Source code**: Thư mục hiện tại

---

**Chúc bạn sử dụng thành công! 🎉**
