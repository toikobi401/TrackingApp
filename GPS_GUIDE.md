# Hướng dẫn GPS - Vị trí chính xác 📍

## 🎯 Chức năng GPS mới

Phiên bản mới đã tích hợp **Windows Location Service** để lấy vị trí GPS chính xác thay vì chỉ dựa vào IP address!

## 📊 So sánh

| Phương pháp | Độ chính xác | Ưu điểm | Nhược điểm |
|-------------|--------------|---------|------------|
| **GPS (Mới)** | ±10m | Cực kỳ chính xác | Cần Location Service |
| **IP-based (Cũ)** | ±5-50km | Không cần quyền | Không chính xác |

## 🔐 Bật Location Service

### Lần đầu tiên chạy:

1. Khi chạy DeviceTracker.exe, chương trình sẽ tự động:
   - Thử lấy GPS trước
   - Nếu không được, mở Windows Settings
   - Yêu cầu bạn bật Location

2. **Trong Windows Settings**:
   ```
   Privacy → Location
   ├── Location for this device: ON
   └── Let apps access your location: ON
   ```

3. Nhấn Enter trong chương trình để tiếp tục

### Cách bật thủ công:

**Windows 11**:
```
Settings → Privacy & security → Location → ON
```

**Windows 10**:
```
Settings → Privacy → Location → ON
```

**Hoặc dùng lệnh**:
```powershell
# Mở Settings Location
start ms-settings:privacy-location
```

## 🚀 Cách hoạt động

### Khi chạy Tracker:

1. **Thử GPS trước** (độ chính xác cao)
   ```
   Đang lấy vị trí GPS...
   Location GPS: 21.018400, 105.846100 ✓ Chính xác
   ```

2. **Nếu GPS không có** → Fallback sang IP
   ```
   Không lấy được GPS, dùng vị trí từ IP...
   Location (từ IP): Hanoi, Vietnam (21.0184, 105.8461)
   ```

### Trong Email:

**Có GPS**:
```
Vị trí: GPS: 21.018432, 105.846587 (Chính xác)
Link Google Maps: https://www.google.com/maps?q=21.018432,105.846587
```

**Không có GPS**:
```
Vị trí: Hanoi, Vietnam (21.0184, 105.8461)
Link Google Maps: https://www.google.com/maps?q=Hanoi, Vietnam (21.0184, 105.8461)
```

## ⚙️ Technical Details

### Windows Location Service sử dụng:

- ✅ **GPS** (nếu có GPS hardware)
- ✅ **Wi-Fi positioning** (dựa vào Wi-Fi networks xung quanh)
- ✅ **Cell tower triangulation** (nếu có modem)
- ✅ **IP address** (fallback cuối cùng)

### PowerShell Script:

Chương trình sử dụng `System.Device.Location.GeoCoordinateWatcher`:

```powershell
Add-Type -AssemblyName System.Device
$loc = New-Object System.Device.Location.GeoCoordinateWatcher
$loc.Start()
Start-Sleep -Seconds 5
$loc.Position.Location.Latitude
$loc.Position.Location.Longitude
```

## 🔍 Kiểm tra Location Service

### PowerShell:
```powershell
# Kiểm tra trạng thái
Get-Service -Name lfsvc

# Start service nếu stopped
Start-Service -Name lfsvc
```

### Services:
```
Win + R → services.msc
Tìm: "Geolocation Service"
Status: Running
```

## 🐛 Troubleshooting

### ❌ Không lấy được GPS

**Nguyên nhân**:
1. Location Service chưa bật
2. App không có quyền truy cập location
3. Thiết bị không có GPS/Wi-Fi

**Giải pháp**:
```powershell
# 1. Bật Location Service
start ms-settings:privacy-location

# 2. Chạy lại DeviceTracker với Admin
Start-Process -FilePath ".\DeviceTracker.exe" -Verb RunAs

# 3. Chọn option 1 để test
```

### ❌ GPS timeout (5 giây)

**Nguyên nhân**: Thiết bị đang tìm vị trí lần đầu

**Giải pháp**:
- Chạy lại sau vài phút
- Đảm bảo Wi-Fi đang bật
- Chương trình sẽ tự động fallback sang IP

### ❌ "Location for this device is turned off"

**Giải pháp**:
```
Settings → Privacy → Location
Bật: "Location for this device" (cần Admin)
```

## 📱 Thiết bị nào hỗ trợ?

### ✅ Laptop có Wi-Fi
- Dùng Wi-Fi positioning
- Độ chính xác: ±20-100m

### ✅ Laptop có GPS
- Dùng GPS thực
- Độ chính xác: ±5-10m

### ✅ Desktop có Wi-Fi
- Dùng Wi-Fi positioning
- Độ chính xác: ±50-200m

### ⚠️ Desktop không có Wi-Fi
- Fallback sang IP
- Độ chính xác: ±5-50km

## 🎯 Best Practices

### Để có độ chính xác cao nhất:

1. ✅ Bật Location Service
2. ✅ Bật Wi-Fi (ngay cả khi dùng Ethernet)
3. ✅ Cho phép Windows cập nhật location database
4. ✅ Chạy chương trình ít nhất 2 lần (lần đầu Windows cache location)

### Cho auto-start:

1. Cài đặt với option 2 trong menu
2. Location sẽ được lấy tự động mỗi khi boot
3. Không cần bật Settings thủ công sau lần đầu

## 📧 Email Format

### GPS Chính xác:
```
Subject: [Device Tracker] Thiết bị WAR-MACHINE đã khởi động

=== THÔNG TIN THIẾT BỊ ===

Tên thiết bị: WAR-MACHINE
Người dùng: PC
Hệ điều hành: Windows OS
Địa chỉ IP: 113.23.49.134
MAC Address: C8:7F:54:A4:4A:05
Vị trí: GPS: 21.018432, 105.846587 (Chính xác) ⭐
Thời gian: 2025-11-25 14:30:45

Link Google Maps: https://www.google.com/maps?q=GPS: 21.018432, 105.846587 (Chính xác)
```

### IP-based Fallback:
```
Vị trí: Hanoi, Vietnam (21.0184, 105.8461)
```

## 🔒 Privacy & Security

### Chương trình:
- ✅ CHỈ lấy location khi chạy
- ✅ KHÔNG theo dõi liên tục
- ✅ KHÔNG lưu trữ lịch sử
- ✅ KHÔNG gửi data đến bên thứ 3 (trừ email của bạn)

### Windows Location Service:
- Tuân thủ Windows Privacy Policy
- Bạn có thể tắt bất cứ lúc nào
- Check log: Settings → Privacy → Location → Location history

---

**Nâng cấp thành công! Giờ đây bạn có vị trí GPS chính xác! 🎯**
