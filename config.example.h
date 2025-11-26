/*
 * Device Tracker - Configuration File (EXAMPLE)
 * Cấu hình thông tin email và SMTP server
 * 
 * HƯỚNG DẪN SỬ DỤNG:
 * 1. Copy file này thành "config.h"
 * 2. Thay thế các giá trị YOUR_* bằng thông tin thực của bạn
 * 3. Với Gmail, tạo App Password tại: https://myaccount.google.com/apppasswords
 */

#ifndef CONFIG_H
#define CONFIG_H

// ===== CẤU HÌNH EMAIL =====
// Địa chỉ email nhận thông báo
#define EMAIL_TO "YOUR_RECEIVER_EMAIL@gmail.com"

// Địa chỉ email gửi (thường là email SMTP của bạn)
#define EMAIL_FROM "YOUR_SENDER_EMAIL@gmail.com"


// ===== CẤU HÌNH SMTP SERVER =====
// Đối với Gmail:
#define SMTP_SERVER "smtp.gmail.com"
#define SMTP_PORT 587  // Cổng TLS (hoặc 465 cho SSL)

// Username SMTP (thường là địa chỉ email)
#define SMTP_USERNAME "YOUR_SENDER_EMAIL@gmail.com"

// Password SMTP
// CHÚ Ý: Với Gmail, bạn cần sử dụng "App Password" thay vì mật khẩu thường
// Tạo App Password tại: https://myaccount.google.com/apppasswords
#define SMTP_PASSWORD "your_app_password_here"


/*
 * HƯỚNG DẪN CẤU HÌNH:
 * 
 * 1. Gmail:
 *    - Server: smtp.gmail.com
 *    - Port: 587 (TLS) hoặc 465 (SSL)
 *    - Bật xác thực 2 bước
 *    - Tạo App Password tại: https://myaccount.google.com/apppasswords
 * 
 * 2. Outlook/Hotmail:
 *    - Server: smtp-mail.outlook.com
 *    - Port: 587
 *    - Username: địa chỉ email đầy đủ
 * 
 * 3. Yahoo Mail:
 *    - Server: smtp.mail.yahoo.com
 *    - Port: 587
 *    - Cần tạo App Password
 * 
 * 4. Custom SMTP Server:
 *    - Thay đổi SMTP_SERVER và SMTP_PORT theo server của bạn
 */

#endif // CONFIG_H
