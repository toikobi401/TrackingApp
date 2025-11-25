# Device Tracker - All-in-One Version 🚀

## 📦 File duy nhất: `DeviceTracker.exe`

Tất cả chức năng đã được tích hợp vào 1 file `.exe` duy nhất!

## ✨ Tính năng

✅ **Tracker**: Gửi thông tin vị trí thiết bị qua email ngay lập tức  
✅ **Auto-Install**: Tự động cài đặt chạy khi Windows khởi động  
✅ **Auto-Download**: Tự động tải CA certificates khi cần  
✅ **Uninstall**: Gỡ cài đặt hoàn toàn  
✅ **Status Check**: Kiểm tra trạng thái cài đặt  

## 🚀 Cách sử dụng

### Bước 1: Chạy với quyền Administrator
```
Right-click vào DeviceTracker.exe → Run as Administrator
```

### Bước 2: Chọn chức năng từ menu

```
╔════════════════════════════════════════╗
║     DEVICE TRACKER - ALL-IN-ONE        ║
╚════════════════════════════════════════╝

1. Chạy Tracker (Gửi vị trí ngay)
2. Cài đặt Auto-Start (Chạy khi boot)
3. Gỡ cài đặt Auto-Start
4. Kiểm tra trạng thái
5. Thoát
```

## 📋 Hướng dẫn chi tiết

### Option 1: Chạy Tracker ngay
- Chọn `1` để gửi thông tin vị trí ngay lập tức
- Chương trình sẽ:
  1. Tự động tải CA certificates (nếu cần)
  2. Thu thập thông tin thiết bị
  3. Lấy IP và vị trí GPS
  4. Gửi email

### Option 2: Cài đặt Auto-Start
- Chọn `2` để cài đặt chạy tự động khi Windows boot
- Chương trình sẽ:
  1. Tạo thư mục `C:\ProgramData\DeviceTracker\`
  2. Copy file `DeviceTracker.exe` vào thư mục đó
  3. Tải CA certificates
  4. Tạo Task Scheduler để chạy khi khởi động
  5. **Từ giờ, mỗi lần bật máy sẽ tự động gửi email!**

### Option 3: Gỡ cài đặt
- Chọn `3` để gỡ cài đặt hoàn toàn
- Xóa Task Scheduler và toàn bộ file

### Option 4: Kiểm tra trạng thái
- Chọn `4` để xem:
  - Quyền Administrator
  - Trạng thái cài đặt
  - Task Scheduler
  - Cấu hình email

## 🔧 Yêu cầu

### Trước khi chạy:
1. ✅ Đã cấu hình email trong `config.h`
2. ✅ Đã build file `DeviceTracker.exe`
3. ✅ Có kết nối internet (để tải CA certificates và gửi email)

### Build từ source:
```powershell
gcc -Wall -O2 device_tracker_complete.c -o DeviceTracker.exe -I"curl-8.11.1_1-win64-mingw/include" -L"curl-8.11.1_1-win64-mingw/lib" -lcurl -lws2_32 -liphlpapi -lshell32 -lole32
```

## 📧 Cấu hình Email

Trước khi sử dụng, chỉnh sửa file `config.h`:

```c
#define EMAIL_TO "your-email@gmail.com"
#define EMAIL_FROM "your-email@gmail.com"

#define SMTP_SERVER "smtp.gmail.com"
#define SMTP_PORT 587

#define SMTP_USERNAME "your-email@gmail.com"
#define SMTP_PASSWORD "your-app-password"
```

**Lưu ý**: Với Gmail, cần tạo **App Password** tại:  
https://myaccount.google.com/apppasswords

## 🎯 Ưu điểm của phiên bản All-in-One

✅ **Chỉ 1 file .exe duy nhất** - Không cần DLL riêng  
✅ **Tự động tải CA bundle** - Không cần file .crt riêng  
✅ **Menu trực quan** - Dễ sử dụng  
✅ **Tích hợp đầy đủ** - Tracker + Installer + Uninstaller  
✅ **Kiểm tra quyền Admin** - Tự động báo nếu thiếu quyền  

## 📂 Cấu trúc sau khi cài đặt

```
C:\ProgramData\DeviceTracker\
├── DeviceTracker.exe (được copy vào đây)
└── curl-ca-bundle.crt (tự động tải)
```

Task Scheduler: **DeviceTracker**  
- Trigger: At Startup (delay 30 giây)
- Run Level: Highest (Administrator)
- Action: Chạy `DeviceTracker.exe --tracker`

## 🔍 Kiểm tra Task Scheduler

### PowerShell:
```powershell
Get-ScheduledTask -TaskName "DeviceTracker"
```

### GUI:
```
Win + R → taskschd.msc → Tìm "DeviceTracker"
```

### Test chạy thủ công:
```powershell
Start-ScheduledTask -TaskName "DeviceTracker"
```

## 📧 Định dạng Email nhận được

```
Subject: [Device Tracker] Thiết bị WAR-MACHINE đã khởi động

=== THÔNG TIN THIẾT BỊ ===

Tên thiết bị: WAR-MACHINE
Người dùng: PC
Hệ điều hành: Windows OS
Địa chỉ IP: 113.23.49.134
MAC Address: C8:7F:54:A4:4A:05
Vị trí: Hanoi, Vietnam (21.0184, 105.8461)
Thời gian: 2025-11-25 14:30:45

Link Google Maps: https://www.google.com/maps?q=Hanoi, Vietnam (21.0184, 105.8461)

---
Email này được gửi tự động bởi Device Tracker
```

## 🛠️ Troubleshooting

### Lỗi "Cần quyền Administrator"
→ Right-click → Run as Administrator

### Không nhận được email
→ Chọn option 1 để test, xem lỗi cụ thể
→ Kiểm tra config.h đã đúng email/password chưa

### Task không chạy tự động
→ Chọn option 4 để kiểm tra trạng thái
→ Test thủ công: `Start-ScheduledTask -TaskName "DeviceTracker"`

### Muốn build lại
→ Chỉnh sửa config.h
→ Build lại bằng lệnh gcc ở trên
→ Chọn option 3 để gỡ cài đặt cũ
→ Chọn option 2 để cài đặt lại

## 🎉 Kết luận

Chỉ cần **1 file `DeviceTracker.exe`** và bạn có đầy đủ:
- ✅ Tracker hoàn chỉnh
- ✅ Cài đặt tự động
- ✅ Gỡ cài đặt
- ✅ Kiểm tra trạng thái

**Không cần thêm file DLL hoặc script PowerShell nào nữa!**

---

Made with ❤️ for Device Security
