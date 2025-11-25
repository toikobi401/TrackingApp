# Device Tracker 📍

Ứng dụng C theo dõi thiết bị laptop - Tự động gửi thông tin vị trí qua email khi khởi động.

## 🎯 Tính năng

- ✅ Tự động thu thập thông tin thiết bị:
  - Hostname và Username
  - Thông tin hệ điều hành
  - Địa chỉ IP công khai
  - MAC Address
  - Vị trí địa lý (quốc gia, thành phố, tọa độ GPS)
  - Timestamp

- ✅ Gửi email tự động với thông tin chi tiết
- ✅ Hỗ trợ Windows và Linux
- ✅ Sử dụng API miễn phí để xác định vị trí

## 📋 Yêu cầu

### Windows
- MinGW-w64 (GCC compiler cho Windows)
- libcurl (thư viện HTTP/SMTP)

### Linux
- GCC compiler
- libcurl development library

## 🔧 Cài đặt

### 1. Cài đặt Dependencies

#### Windows (MinGW):
```powershell
# Tải và cài đặt MinGW-w64 từ:
# https://www.mingw-w64.org/downloads/

# Cài đặt libcurl (sử dụng MSYS2):
pacman -S mingw-w64-x86_64-curl
```

#### Linux (Ubuntu/Debian):
```bash
sudo apt-get update
sudo apt-get install build-essential libcurl4-openssl-dev
```

#### Linux (Fedora/RHEL):
```bash
sudo dnf install gcc make libcurl-devel
```

### 2. Cấu hình Email

Mở file `config.h` và chỉnh sửa các thông tin:

```c
// Địa chỉ email nhận thông báo
#define EMAIL_TO "your-email@gmail.com"

// Địa chỉ email gửi
#define EMAIL_FROM "your-email@gmail.com"

// Thông tin SMTP
#define SMTP_SERVER "smtp.gmail.com"
#define SMTP_PORT 587

#define SMTP_USERNAME "your-email@gmail.com"
#define SMTP_PASSWORD "your-app-password"
```

### 3. Tạo App Password cho Gmail

1. Truy cập: https://myaccount.google.com/apppasswords
2. Đăng nhập tài khoản Gmail
3. Chọn "Mail" và "Windows Computer" (hoặc thiết bị khác)
4. Click "Generate"
5. Copy mật khẩu 16 ký tự và dán vào `SMTP_PASSWORD` trong `config.h`

**Lưu ý:** Bạn cần bật xác thực 2 bước (2FA) trước khi tạo App Password.

## 🏗️ Build Project

### Sử dụng Makefile (Khuyến nghị):
```bash
# Build project
make

# Clean build files
make clean

# Build và chạy
make run
```

### Build thủ công:

#### Windows:
```powershell
gcc -Wall -O2 main.c -o device_tracker.exe -lcurl -lws2_32 -liphlpapi
```

#### Linux:
```bash
gcc -Wall -O2 main.c -o device_tracker -lcurl
```

## 🚀 Sử dụng

### Chạy thủ công:
```bash
# Windows
.\device_tracker.exe

# Linux
./device_tracker
```

### Chạy tự động khi khởi động:

#### Windows (Task Scheduler):
1. Mở Task Scheduler (Win + R → `taskschd.msc`)
2. Click "Create Task"
3. Tab "General": Đặt tên "Device Tracker", chọn "Run with highest privileges"
4. Tab "Triggers": New → "At startup"
5. Tab "Actions": New → Chọn file `device_tracker.exe`
6. Tab "Conditions": Bỏ chọn "Start only if on AC power"
7. Click OK để lưu

#### Windows (Startup Folder):
```powershell
# Copy executable vào startup folder
copy device_tracker.exe "$env:APPDATA\Microsoft\Windows\Start Menu\Programs\Startup\"
```

#### Linux (systemd):
Tạo file `/etc/systemd/system/device-tracker.service`:
```ini
[Unit]
Description=Device Tracker Service
After=network-online.target
Wants=network-online.target

[Service]
Type=oneshot
ExecStart=/usr/local/bin/device_tracker
User=your-username

[Install]
WantedBy=multi-user.target
```

Enable service:
```bash
sudo systemctl daemon-reload
sudo systemctl enable device-tracker
sudo systemctl start device-tracker
```

#### Linux (crontab):
```bash
# Mở crontab
crontab -e

# Thêm dòng sau:
@reboot /path/to/device_tracker
```

## 📧 Định dạng Email

Email nhận được sẽ có dạng:

```
Subject: [Device Tracker] Thiết bị LAPTOP-XYZ đã khởi động

=== THÔNG TIN THIẾT BỊ ===

Tên thiết bị: LAPTOP-XYZ
Người dùng: john_doe
Hệ điều hành: Windows (Version: 10.0 Build 19045)
Địa chỉ IP: 123.45.67.89
MAC Address: AA:BB:CC:DD:EE:FF
Vị trí: Ho Chi Minh City, Vietnam (10.8231, 106.6297)
Thời gian: 2025-11-25 14:30:45

---
Email này được gửi tự động bởi Device Tracker
```

## 🔒 Bảo mật

⚠️ **Quan trọng:**
- File `config.h` chứa thông tin nhạy cảm (mật khẩu email)
- **KHÔNG** commit file này lên Git/GitHub
- Tạo file `.gitignore` với nội dung:
  ```
  config.h
  *.exe
  *.o
  device_tracker
  ```

## 🛠️ Troubleshooting

### Lỗi "Cannot find libcurl"
- Windows: Đảm bảo MinGW và libcurl đã được cài đặt đúng
- Linux: Cài đặt `libcurl4-openssl-dev`

### Lỗi gửi email
- Kiểm tra username/password SMTP
- Gmail: Đảm bảo đã tạo App Password (không dùng mật khẩu thường)
- Kiểm tra kết nối internet
- Thử port 465 (SSL) thay vì 587 (TLS)

### Không lấy được vị trí
- Kiểm tra kết nối internet
- API ip-api.com có thể bị giới hạn rate (45 requests/phút)
- Thử chạy lại sau vài phút

## 📝 Tùy chỉnh

### Thay đổi SMTP Server (Outlook/Yahoo):

**Outlook:**
```c
#define SMTP_SERVER "smtp-mail.outlook.com"
#define SMTP_PORT 587
```

**Yahoo:**
```c
#define SMTP_SERVER "smtp.mail.yahoo.com"
#define SMTP_PORT 587
```

### Thêm thông tin khác
Bạn có thể mở rộng struct `DeviceInfo` trong `main.c` để thêm thông tin như:
- Dung lượng ổ cứng
- RAM
- Thông tin CPU
- Danh sách process đang chạy

## 📜 License

MIT License - Sử dụng tự do cho mục đích cá nhân và thương mại.

## 🤝 Đóng góp

Mọi đóng góp đều được chào đón! Hãy tạo Pull Request hoặc Issue.

## ⚠️ Lưu ý pháp lý

- Chỉ sử dụng ứng dụng này trên thiết bị của chính bạn
- Không sử dụng để theo dõi thiết bị của người khác mà không có sự đồng ý
- Tuân thủ luật pháp về quyền riêng tư tại quốc gia của bạn

## 📞 Hỗ trợ

Nếu gặp vấn đề, vui lòng:
1. Kiểm tra phần Troubleshooting
2. Đọc kỹ file `config.h`
3. Kiểm tra log khi chạy chương trình
4. Tạo Issue trên GitHub

---

**Device Tracker** - Giữ an toàn thiết bị của bạn! 🛡️
