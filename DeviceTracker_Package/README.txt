════════════════════════════════════════════════════════════════
   DEVICE TRACKER - ALL-IN-ONE PACKAGE v2.0
════════════════════════════════════════════════════════════════

📦 NỘI DUNG PACKAGE:

  ✓ All_In_One.exe        - Chương trình chính (TOÀN BỘ TRONG 1 FILE!)
  ✓ libcurl-x64.dll       - Thư viện HTTP/SMTP (bắt buộc)
  ✓ curl-ca-bundle.crt    - Certificate cho SSL (tùy chọn)

════════════════════════════════════════════════════════════════

🚀 HƯỚNG DẪN SỬ DỤNG CỰC KỲ ĐỠN GIẢN:

【BƯỚC 1】Copy toàn bộ thư mục này sang thiết bị mới

【BƯỚC 2】Right-click vào "All_In_One.exe"
         → Chọn "Run as Administrator"

【BƯỚC 3】Chọn chức năng từ menu:

   ┌──────────────────────────────────────┐
   │  1. Chạy Tracker (gửi email ngay)   │
   │  2. Cài đặt Auto-Start               │
   │  3. Gỡ cài đặt                       │
   │  4. Kiểm tra trạng thái              │
   │  5. Thoát                            │
   └──────────────────────────────────────┘

════════════════════════════════════════════════════════════════

✨ TÍNH NĂNG NỔI BẬT:

  ✅ KHÔNG CẦN cài đặt gì thêm!
  ✅ KHÔNG CẦN MinGW, Visual Studio, hay compiler!
  ✅ CHỈ CẦN 3 file này là đủ!
  ✅ Tự động copy dependencies vào hệ thống
  ✅ Tự động tạo Task Scheduler
  ✅ Tự động tải CA certificates nếu thiếu
  ✅ Chạy hoàn toàn IM LẶNG khi auto-start
  ✅ Gửi email với vị trí GPS chính xác

════════════════════════════════════════════════════════════════

📧 CẤU HÌNH EMAIL:

Email được cấu hình sẵn trong file All_In_One.exe:
  • Gửi từ:  datltthe194235@gmail.com
  • Gửi đến: ledat112228@gmail.com
  • SMTP:    smtp.gmail.com:587

Nếu muốn thay đổi email, cần rebuild từ source code!

════════════════════════════════════════════════════════════════

🎯 CÁCH HOẠT ĐỘNG:

Option 2 (Cài đặt Auto-Start) sẽ:

1. Copy All_In_One_Silent.exe vào C:\ProgramData\DeviceTracker\
2. Copy libcurl-x64.dll và curl-ca-bundle.crt
3. Tạo Task Scheduler tên "DeviceTracker"
4. Task chạy 30 giây sau khi Windows boot
5. Chương trình tự động:
   • CHỜ VÔ HẠN nếu chưa có internet (không timeout)
   • Kiểm tra internet mỗi 10 giây
   • Khi có mạng → Thu thập thông tin → Gửi email → Thoát
   
KHÔNG CẦN LO nếu máy boot mà chưa có internet!
Chương trình sẽ tự động chờ và gửi khi có mạng.

  1️⃣ Copy All_In_One.exe → C:\ProgramData\DeviceTracker\
  2️⃣ Copy libcurl-x64.dll → C:\ProgramData\DeviceTracker\
  3️⃣ Tải curl-ca-bundle.crt (nếu chưa có)
  4️⃣ Tạo Task Scheduler chạy khi Windows boot
  5️⃣ HOÀN TẤT! Từ giờ mỗi lần khởi động:
      → Đợi 30 giây (để có internet)
      → Thu thập thông tin thiết bị
      → Lấy vị trí GPS (hoặc IP location)
      → Gửi email đến ledat112228@gmail.com
      → Chạy HOÀN TOÀN IM LẶNG (không hiện cửa sổ)

════════════════════════════════════════════════════════════════

📧 ĐỊNH DẠNG EMAIL NHẬN ĐƯỢC:

Subject: [Device Tracker] Thiết bị WAR-MACHINE đã khởi động

=== THÔNG TIN THIẾT BỊ ===

Tên thiết bị: xxxxxxx
Người dùng: PC
Hệ điều hành: Windows OS
Địa chỉ IP: xxxxxxx
MAC Address: xxxxxxx
Vị trí: GPS: xxxxxxx, xxxxxxx.487370 
Thời gian: 2025-11-25 14:30:45

Link Google Maps: https://www.google.com/maps?q=GPS: xxxxxxx, xxxxxxx

════════════════════════════════════════════════════════════════

🔍 KIỂM TRA SAU KHI CÀI ĐẶT:

1. Mở Task Scheduler (Win + R → taskschd.msc)
   → Tìm task "DeviceTracker"
   → Kiểm tra Status: Ready

2. Test ngay không cần reboot:
   → Chọn Option 1 trong menu
   → Kiểm tra email sau 10-20 giây

3. Test auto-start:
   → Khởi động lại máy
   → Kiểm tra email sau ~1 phút

════════════════════════════════════════════════════════════════

🛠️ TROUBLESHOOTING:

❌ "Thiếu quyền Administrator"
   → Right-click → Run as Administrator

❌ "Không tìm thấy libcurl-x64.dll"
   → Đảm bảo file .dll nằm CÙNG thư mục với All_In_One.exe

❌ "Không nhận được email"
   → Chọn Option 1 để test
   → Kiểm tra cấu hình email trong source code
   → Đảm bảo có kết nối internet

❌ "Task không chạy tự động"
   → Chọn Option 4 để kiểm tra trạng thái
   → Mở Task Scheduler xem chi tiết lỗi

════════════════════════════════════════════════════════════════

📂 FILE ĐƯỢC TẠO SAU KHI CÀI ĐẶT:

C:\ProgramData\DeviceTracker\
  ├── All_In_One.exe        (copy tự động)
  ├── libcurl-x64.dll       (copy tự động)
  └── curl-ca-bundle.crt    (tải tự động)

Task Scheduler:
  └── DeviceTracker         (tạo tự động)

════════════════════════════════════════════════════════════════

🗑️ GỠ CÀI ĐẶT:

Chọn Option 3 trong menu, hoặc thủ công:

1. Xóa Task Scheduler:
   schtasks /Delete /TN "DeviceTracker" /F

2. Xóa thư mục:
   rmdir /S /Q "C:\ProgramData\DeviceTracker"

════════════════════════════════════════════════════════════════

💡 LƯU Ý QUAN TRỌNG:

• File All_In_One.exe ĐÃ ĐƯỢC BUILD với flag -mwindows
  → Chạy HOÀN TOÀN im lặng, không hiện console

• Email configuration được compile vào trong .exe
  → Không thể thay đổi mà không rebuild

• Cần quyền Administrator để:
  → Copy file vào C:\ProgramData
  → Tạo Task Scheduler
  → Truy cập Windows Location Service

• GPS Location hoạt động tốt nhất khi:
  → Đã bật Location Service trong Windows
  → Có Wi-Fi bật (ngay cả khi dùng Ethernet)
  → Chạy ít nhất 2 lần để Windows cache location

════════════════════════════════════════════════════════════════

📞 HỖ TRỢ:

Nếu gặp vấn đề, kiểm tra:
  1. Option 4 - Trạng thái hệ thống
  2. Task Scheduler → DeviceTracker → History
  3. Event Viewer → Windows Logs → Application

════════════════════════════════════════════════════════════════

✅ ĐÃ TEST VÀ HOẠT ĐỘNG TRÊN:
  • Windows 10 (Build 19045)
  • Windows 11

🎉 CHÚC BẠN SỬ DỤNG THÀNH CÔNG!

════════════════════════════════════════════════════════════════
Version: 2.0 (All-In-One Package)
Build Date: 2025-11-25
Author: Device Tracker Team
════════════════════════════════════════════════════════════════
