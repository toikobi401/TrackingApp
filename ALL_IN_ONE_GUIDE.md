# Device Tracker - All-In-One Package 🚀

## 📦 Giới thiệu

**Device Tracker All-In-One** là phiên bản đóng gói hoàn chỉnh của hệ thống theo dõi thiết bị. Chỉ cần **1 file .exe** và **2 file dependencies**, bạn có thể cài đặt toàn bộ hệ thống trên bất kỳ máy Windows nào mà **KHÔNG CẦN** cài đặt compiler, MinGW, hay bất kỳ công cụ phát triển nào!

## ✨ Điểm khác biệt so với phiên bản cũ

| Tính năng | Phiên bản cũ | All-In-One v2.0 |
|-----------|--------------|------------------|
| **Số file cần thiết** | 10+ files | 3 files |
| **Cần compiler?** | ✅ Có (gcc) | ❌ Không |
| **Cần PowerShell scripts?** | ✅ Có | ❌ Không (tích hợp sẵn) |
| **Tự động cài dependencies** | ❌ Không | ✅ Có |
| **Menu tương tác** | ❌ Không | ✅ Có |
| **Kiểm tra trạng thái** | Khó | Dễ (Option 4) |
| **Chạy im lặng** | ✅ Có | ✅ Có (mwindows flag) |

## 📁 Cấu trúc Package

```
DeviceTracker_Package/
├── All_In_One.exe          ⭐ File chính - tích hợp TẤT CẢ
├── libcurl-x64.dll         📚 Library HTTP/SMTP (bắt buộc)
├── curl-ca-bundle.crt      🔐 SSL Certificates (tùy chọn, có thể tải tự động)
├── QUICKSTART.txt          ⚡ Hướng dẫn 30 giây
└── README.txt              📖 Hướng dẫn chi tiết
```

**Tổng dung lượng**: ~3-4 MB

## 🎯 Tính năng chính

### 1. **Tracker**
- Thu thập thông tin thiết bị (hostname, username, OS, IP, MAC)
- Lấy vị trí GPS chính xác (hoặc IP location làm backup)
- Gửi email tự động với Google Maps link

### 2. **Auto-Installer**
- Tự động copy files vào `C:\ProgramData\DeviceTracker\`
- Tự động tạo Task Scheduler chạy khi boot
- Tự động tải CA certificates nếu thiếu
- Kiểm tra và báo lỗi dependencies

### 3. **Uninstaller**
- Xóa Task Scheduler
- Xóa toàn bộ files đã cài đặt
- Dọn dẹp sạch sẽ hệ thống

### 4. **Status Checker**
- Kiểm tra quyền Administrator
- Kiểm tra files cài đặt
- Kiểm tra Task Scheduler status
- Hiển thị cấu hình email

## 🚀 Cách sử dụng

### Bước 1: Chuẩn bị
```
1. Copy thư mục DeviceTracker_Package sang thiết bị cần cài đặt
2. Đảm bảo có kết nối internet
```

### Bước 2: Chạy chương trình
```
1. Right-click vào All_In_One.exe
2. Chọn "Run as Administrator"
```

### Bước 3: Cài đặt Auto-Start
```
Menu → Chọn "2" (Cài đặt Auto-Start)
Chương trình sẽ tự động:
  ✓ Kiểm tra dependencies
  ✓ Copy files vào C:\ProgramData\DeviceTracker\
  ✓ Tải CA certificates (nếu cần)
  ✓ Tạo Task Scheduler
```

### Bước 4: Test
```
Option 1: Test ngay (không cần reboot)
  → Chọn "1" trong menu
  → Kiểm tra email sau 20-30 giây

Option 2: Test auto-start
  → Khởi động lại máy
  → Kiểm tra email sau ~1 phút
```

## 📧 Email Configuration

Email được cấu hình sẵn trong source code (`config.h`):

```c
#define EMAIL_TO "ledat112228@gmail.com"
#define EMAIL_FROM "datltthe194235@gmail.com"
#define SMTP_SERVER "smtp.gmail.com"
#define SMTP_PORT 587
#define SMTP_USERNAME "datltthe194235@gmail.com"
#define SMTP_PASSWORD "your-app-password"
```

**Lưu ý**: Nếu muốn thay đổi email, cần rebuild từ source code!

### Định dạng email nhận được:

```
Subject: [Device Tracker] Thiết bị WAR-MACHINE đã khởi động

=== THÔNG TIN THIẾT BỊ ===

Tên thiết bị: WAR-MACHINE
Người dùng: PC
Hệ điều hành: Windows OS
Địa chỉ IP: 113.23.49.134
MAC Address: C8:7F:54:A4:4A:05
Vị trí: GPS: 21.341423, 106.487370 (Chính xác)
Thời gian: 2025-11-25 14:30:45

Link Google Maps: https://www.google.com/maps?q=GPS: 21.341423, 106.487370
```

## 🛠️ Build từ Source Code

Nếu muốn tùy chỉnh và build lại:

### Yêu cầu:
- MinGW-w64 (gcc compiler)
- curl development package

### Build command:
```bash
gcc -Wall -O2 all_in_one.c -o All_In_One.exe -mwindows \
    -I"curl-8.11.1_1-win64-mingw/include" \
    -L"curl-8.11.1_1-win64-mingw/lib" \
    -lcurl -lws2_32 -liphlpapi -lshell32 -lole32
```

### Flags quan trọng:
- `-mwindows`: Chạy im lặng (không hiện console window)
- `-O2`: Optimization level 2
- `-Wall`: Hiển thị tất cả warnings

### Sau khi build:
```bash
# Copy vào package
copy All_In_One.exe DeviceTracker_Package\
copy curl-8.11.1_1-win64-mingw\bin\libcurl-x64.dll DeviceTracker_Package\
copy curl-ca-bundle.crt DeviceTracker_Package\
```

## 📂 Files được tạo sau khi cài đặt

### System Files:
```
C:\ProgramData\DeviceTracker\
├── All_In_One.exe        (copy từ package)
├── libcurl-x64.dll       (copy từ package)
└── curl-ca-bundle.crt    (tải tự động hoặc copy)
```

### Task Scheduler:
```
Task Name: DeviceTracker
Trigger: At Startup (delay 30s)
Action: C:\ProgramData\DeviceTracker\All_In_One.exe --tracker
Run Level: Highest (Administrator)
User: SYSTEM
```

## 🔍 Troubleshooting

### ❌ "Thiếu quyền Administrator"
**Nguyên nhân**: Chạy không có quyền Admin

**Giải pháp**:
```
Right-click vào All_In_One.exe
→ Chọn "Run as Administrator"
```

### ❌ "Không tìm thấy libcurl-x64.dll"
**Nguyên nhân**: File .dll không cùng thư mục với .exe

**Giải pháp**:
```
Đảm bảo libcurl-x64.dll nằm trong:
1. Cùng thư mục với All_In_One.exe
2. Hoặc trong thư mục curl-8.11.1_1-win64-mingw\bin\
```

### ❌ "Không nhận được email"
**Nguyên nhân**: Có thể do:
- Không có internet
- Email/password sai
- Gmail App Password hết hạn

**Giải pháp**:
```
1. Chọn Option 1 để test và xem lỗi chi tiết
2. Kiểm tra internet connection
3. Verify email configuration trong source code
4. Tạo App Password mới cho Gmail
```

### ❌ "Task không chạy tự động"
**Nguyên nhân**: Task Scheduler bị lỗi

**Giải pháp**:
```
1. Chọn Option 4 để kiểm tra trạng thái
2. Mở Task Scheduler (Win + R → taskschd.msc)
3. Tìm task "DeviceTracker"
4. Xem History tab để biết lỗi cụ thể
5. Test chạy thủ công: Right-click → Run
```

### ❌ "Access Violation Error (0xC0000005)"
**Nguyên nhân**: Thiếu DLL dependencies

**Giải pháp**:
```
Chạy Option 4 để kiểm tra:
  ✓ libcurl-x64.dll phải có trong C:\ProgramData\DeviceTracker\
  ✓ curl-ca-bundle.crt (tùy chọn nhưng nên có)
```

## 🔐 Bảo mật

### ⚠️ Lưu ý quan trọng:

1. **Email credentials** được compile vào file .exe
   - App Password nên được tạo riêng cho ứng dụng này
   - Không nên share file .exe ra ngoài

2. **Quyền Administrator** được yêu cầu để:
   - Copy files vào `C:\ProgramData`
   - Tạo Task Scheduler
   - Truy cập Windows Location Service

3. **Task chạy với tài khoản SYSTEM**
   - Có quyền cao nhất trên hệ thống
   - Chạy ngay cả khi chưa đăng nhập

4. **SSL/TLS** được sử dụng cho:
   - SMTP connection (port 587 STARTTLS)
   - HTTPS để lấy IP và location

## 📊 So sánh với các phương án khác

### Option 1: DeviceTracker.exe (phiên bản đơn giản)
```
Ưu điểm:
  ✓ Nhẹ nhàng, đơn giản
  ✓ Không có menu, chạy trực tiếp

Nhược điểm:
  ✗ Không có installer tích hợp
  ✗ Cần PowerShell scripts riêng
  ✗ Khó kiểm tra trạng thái
```

### Option 2: All_In_One.exe ⭐ (recommended)
```
Ưu điểm:
  ✓ Tích hợp đầy đủ tính năng
  ✓ Menu tương tác trực quan
  ✓ Tự động cài đặt dependencies
  ✓ Kiểm tra trạng thái dễ dàng
  ✓ Không cần files phụ trợ

Nhược điểm:
  • Kích thước lớn hơn một chút (~90KB vs ~80KB)
```

### Option 3: PowerShell Scripts
```
Ưu điểm:
  ✓ Dễ chỉnh sửa
  ✓ Không cần compile

Nhược điểm:
  ✗ Cần nhiều files (.ps1, .exe, .dll, .crt)
  ✗ Execution Policy có thể chặn
  ✗ Dễ bị antivirus chặn
```

## 📈 Roadmap & Future Features

### Version 3.0 (Planned):
- [ ] GUI thay vì console menu
- [ ] Tùy chỉnh email config từ file external
- [ ] Hỗ trợ nhiều email recipients
- [ ] Screenshot tự động
- [ ] Upload ảnh lên cloud
- [ ] Web dashboard để xem history
- [ ] Hỗ trợ Linux/Mac

## 🤝 Contributing

Nếu muốn đóng góp:

1. Fork repository
2. Tạo branch mới: `git checkout -b feature/your-feature`
3. Commit changes: `git commit -am 'Add new feature'`
4. Push: `git push origin feature/your-feature`
5. Tạo Pull Request

## 📝 License

MIT License - Free to use, modify, and distribute

## 👨‍💻 Author

Device Tracker Team
- Email: ledat112228@gmail.com
- Build Date: 2025-11-25
- Version: 2.0 (All-In-One Package)

## 🎉 Changelog

### v2.0 (2025-11-25) - All-In-One Package
- ✨ Tích hợp toàn bộ chức năng vào 1 file .exe
- ✨ Menu tương tác trực quan
- ✨ Tự động cài đặt dependencies
- ✨ Kiểm tra trạng thái hệ thống (Option 4)
- ✨ Tự động tải CA certificates
- 🐛 Sửa lỗi Access Violation khi thiếu DLL
- 🔒 Chạy hoàn toàn im lặng với -mwindows flag

### v1.5 (2025-11-24) - GPS Support
- ✨ Tích hợp Windows Location Service
- ✨ Lấy vị trí GPS chính xác
- ✨ Fallback sang IP location nếu không có GPS
- 📧 Thêm Google Maps link vào email

### v1.0 (2025-11-23) - Initial Release
- ✅ Thu thập thông tin thiết bị
- ✅ Gửi email qua SMTP
- ✅ Auto-start với Task Scheduler
- ✅ IP-based location tracking

---

**💡 Tip**: Đọc file `QUICKSTART.txt` để bắt đầu nhanh trong 30 giây!
