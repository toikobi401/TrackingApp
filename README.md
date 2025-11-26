# Device Tracker - All-In-One 📍

Ứng dụng C theo dõi thiết bị - Tự động gửi thông tin vị trí qua email khi khởi động Windows.

## ✨ Tính năng

- ✅ **Tracker**: Thu thập thông tin thiết bị (hostname, username, OS, IP, MAC, GPS location)
- ✅ **Auto-Install**: Cài đặt tự động chạy khi Windows boot
- ✅ **Auto-Retry**: Chờ kết nối internet vô hạn, tự động gửi email khi có mạng
- ✅ **Silent Mode**: Chạy hoàn toàn im lặng trong background
- ✅ **Self-Contained**: Tất cả tích hợp trong 1 file .exe

## 📦 Cấu trúc Project

```
TrackingApp/
├── all_in_one.c              # Source code chính
├── config.h                  # Cấu hình email
├── All_In_One.exe            # Executable với console
├── All_In_One_Silent.exe     # Executable silent (cho Task Scheduler)
├── libcurl-x64.dll           # Dependency HTTP/SMTP
├── curl-ca-bundle.crt        # SSL certificates
├── ALL_IN_ONE_GUIDE.md       # Hướng dẫn chi tiết
├── DeviceTracker_Package/    # Package deployment
│   ├── All_In_One.exe
│   ├── All_In_One_Silent.exe
│   ├── libcurl-x64.dll
│   ├── curl-ca-bundle.crt
│   ├── README.txt
│   └── QUICKSTART.txt
└── curl-8.11.1_1-win64-mingw/ # Thư viện curl (chỉ cần khi compile)
```

## 🚀 Cách sử dụng

### Deployment (cho người dùng cuối)

1. Copy thư mục `DeviceTracker_Package` sang máy cần cài đặt
2. Right-click `All_In_One.exe` → Run as Administrator
3. Chọn **Option 2** (Install Auto-Start)
4. Hoàn tất! Mỗi lần boot sẽ tự động gửi email

### Development (cho developer)

#### 1. Cấu hình Email

Mở `config.h` và chỉnh sửa:

```c
#define EMAIL_TO "your-email@gmail.com"
#define EMAIL_FROM "your-email@gmail.com"
#define SMTP_USERNAME "your-email@gmail.com"
#define SMTP_PASSWORD "your-app-password"
```

**Lưu ý**: Với Gmail, tạo App Password tại https://myaccount.google.com/apppasswords

#### 2. Build Project

```powershell
# Console version
gcc -Wall -O2 all_in_one.c -o All_In_One.exe ^
    -I"curl-8.11.1_1-win64-mingw/include" ^
    -L"curl-8.11.1_1-win64-mingw/lib" ^
    -lcurl -lws2_32 -liphlpapi -lshell32 -lole32

# Silent version (không console)
gcc -Wall -O2 all_in_one.c -o All_In_One_Silent.exe ^
    -I"curl-8.11.1_1-win64-mingw/include" ^
    -L"curl-8.11.1_1-win64-mingw/lib" ^
    -lcurl -lws2_32 -liphlpapi -lshell32 -lole32 -mwindows

# Copy vào package
Copy-Item "All_In_One.exe" "DeviceTracker_Package\" -Force
Copy-Item "All_In_One_Silent.exe" "DeviceTracker_Package\" -Force
```

## 🎯 Cách hoạt động

### Option 2: Install Auto-Start

Khi chọn Option 2, chương trình sẽ:

1. Copy `All_In_One_Silent.exe` vào `C:\ProgramData\DeviceTracker\`
2. Tạo Task Scheduler với lệnh: `All_In_One_Silent.exe --starter`
3. Task chạy 30 giây sau khi boot với SYSTEM account

### Startup Mode (`--starter`)

Khi Windows khởi động:

```
1. Task Scheduler chạy: All_In_One_Silent.exe --starter
2. Chương trình bắt đầu:
   │
   ├─ Kiểm tra internet...
   │  │
   │  ├─ CÓ INTERNET:
   │  │  ├─ Collect device info (hostname, IP, GPS, MAC...)
   │  │  ├─ Gửi email
   │  │  └─ Thoát ✓
   │  │
   │  └─ KHÔNG CÓ INTERNET:
   │     ├─ Đợi 10 giây
   │     └─ Kiểm tra lại (lặp vô hạn cho đến khi có mạng)
```

## 📧 Email Format

```
Subject: [Device Tracker] Device WAR-MACHINE has started

=== DEVICE INFORMATION ===

Device name: WAR-MACHINE
User: PC
Operating system: Windows OS
IP address: 113.23.49.134
MAC Address: C8:7F:54:A4:4A:05
Location: GPS: 21.341440, 106.487427 (Accurate)
Time: 2025-11-26 14:30:45

Google Maps Link: https://www.google.com/maps?q=GPS: 21.341440, 106.487427
```

## 🔧 Yêu cầu

### Deployment
- Windows 7/8/10/11
- Quyền Administrator (để cài đặt Task Scheduler)
- Kết nối internet (để gửi email)

### Development
- MinGW-w64 (GCC compiler)
- libcurl library (đã có trong `curl-8.11.1_1-win64-mingw/`)

## 📖 Documentation

- `ALL_IN_ONE_GUIDE.md` - Hướng dẫn chi tiết đầy đủ
- `DeviceTracker_Package/README.txt` - Hướng dẫn cho người dùng cuối
- `DeviceTracker_Package/QUICKSTART.txt` - Hướng dẫn nhanh 30 giây

## 🗑️ Gỡ cài đặt

1. Chạy `All_In_One.exe` as Administrator
2. Chọn **Option 3** (Uninstall Auto-Start)
3. Xóa thư mục `C:\ProgramData\DeviceTracker\`

Hoặc thủ công:
```powershell
schtasks /Delete /TN "DeviceTracker" /F
Remove-Item "C:\ProgramData\DeviceTracker" -Recurse -Force
```

## 🛡️ Security Notes

- Email password được hardcode trong `config.h` (sử dụng App Password, không phải password chính)
- Chương trình chạy với SYSTEM account (để hoạt động khi chưa login)
- SSL certificates được verify qua `curl-ca-bundle.crt`

## 📝 License

Dự án cá nhân - Sử dụng tự do.

---

**Author**: PC  
**Date**: November 2025  
**Version**: 2.0 (All-In-One with Infinite Wait)
