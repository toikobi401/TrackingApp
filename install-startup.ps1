# Device Tracker - Script cài đặt tự động chạy khi khởi động
# Chạy script này với quyền Administrator

Write-Host "=== Device Tracker - Cài đặt Auto-Start ===" -ForegroundColor Cyan
Write-Host ""

# Kiểm tra quyền Administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "CẢNH BÁO: Script cần quyền Administrator!" -ForegroundColor Red
    Write-Host "Vui lòng chạy lại với quyền Administrator (Right-click -> Run as Administrator)" -ForegroundColor Yellow
    Write-Host ""
    Read-Host "Nhấn Enter để thoát"
    exit
}

# Đường dẫn thư mục cài đặt
$installDir = "$env:ProgramData\DeviceTracker"
$currentDir = Get-Location

Write-Host "1. Tạo thư mục cài đặt..." -ForegroundColor Green
if (-not (Test-Path $installDir)) {
    New-Item -ItemType Directory -Path $installDir -Force | Out-Null
    Write-Host "   ✓ Đã tạo: $installDir" -ForegroundColor Gray
} else {
    Write-Host "   ✓ Thư mục đã tồn tại: $installDir" -ForegroundColor Gray
}

# Kiểm tra file cần thiết
$requiredFiles = @(
    "device_tracker.exe",
    "libcurl-x64.dll",
    "curl-ca-bundle.crt"
)

Write-Host ""
Write-Host "2. Kiểm tra file cần thiết..." -ForegroundColor Green
$allFilesExist = $true
foreach ($file in $requiredFiles) {
    if (Test-Path $file) {
        Write-Host "   ✓ $file" -ForegroundColor Gray
    } else {
        Write-Host "   ✗ THIẾU: $file" -ForegroundColor Red
        $allFilesExist = $false
    }
}

if (-not $allFilesExist) {
    Write-Host ""
    Write-Host "LỖI: Thiếu file cần thiết. Vui lòng build lại project!" -ForegroundColor Red
    Read-Host "Nhấn Enter để thoát"
    exit
}

# Copy file vào thư mục cài đặt
Write-Host ""
Write-Host "3. Copy file vào thư mục cài đặt..." -ForegroundColor Green
foreach ($file in $requiredFiles) {
    Copy-Item -Path $file -Destination $installDir -Force
    Write-Host "   ✓ Đã copy: $file" -ForegroundColor Gray
}

# Tạo Task Scheduler
Write-Host ""
Write-Host "4. Tạo Task Scheduler để chạy khi khởi động..." -ForegroundColor Green

$taskName = "DeviceTracker"
$exePath = Join-Path $installDir "device_tracker.exe"

# Xóa task cũ nếu tồn tại
$existingTask = Get-ScheduledTask -TaskName $taskName -ErrorAction SilentlyContinue
if ($existingTask) {
    Unregister-ScheduledTask -TaskName $taskName -Confirm:$false
    Write-Host "   ✓ Đã xóa task cũ" -ForegroundColor Gray
}

# Tạo action
$action = New-ScheduledTaskAction -Execute $exePath -WorkingDirectory $installDir

# Tạo trigger (chạy khi startup với delay 30 giây)
$trigger = New-ScheduledTaskTrigger -AtStartup
$trigger.Delay = "PT30S"  # Delay 30 giây để đảm bảo có kết nối internet

# Tạo settings
$settings = New-ScheduledTaskSettingsSet `
    -AllowStartIfOnBatteries `
    -DontStopIfGoingOnBatteries `
    -StartWhenAvailable `
    -RunOnlyIfNetworkAvailable `
    -ExecutionTimeLimit (New-TimeSpan -Minutes 5)

# Tạo principal (chạy với highest privileges)
$principal = New-ScheduledTaskPrincipal -UserId "SYSTEM" -LogonType ServiceAccount -RunLevel Highest

# Đăng ký task
Register-ScheduledTask `
    -TaskName $taskName `
    -Action $action `
    -Trigger $trigger `
    -Settings $settings `
    -Principal $principal `
    -Description "Tự động gửi thông tin vị trí thiết bị qua email khi khởi động" `
    -Force | Out-Null

Write-Host "   ✓ Đã tạo Task Scheduler" -ForegroundColor Gray

# Kiểm tra task đã được tạo
$task = Get-ScheduledTask -TaskName $taskName -ErrorAction SilentlyContinue
if ($task) {
    Write-Host ""
    Write-Host "=== CÀI ĐẶT THÀNH CÔNG! ===" -ForegroundColor Green
    Write-Host ""
    Write-Host "Thông tin:" -ForegroundColor Cyan
    Write-Host "  • Task Name: $taskName" -ForegroundColor Gray
    Write-Host "  • Vị trí: $installDir" -ForegroundColor Gray
    Write-Host "  • Trạng thái: " -NoNewline -ForegroundColor Gray
    Write-Host $task.State -ForegroundColor Yellow
    Write-Host "  • Chạy khi: Khởi động Windows (delay 30 giây)" -ForegroundColor Gray
    Write-Host ""
    Write-Host "Từ giờ, mỗi khi bật máy, chương trình sẽ tự động:" -ForegroundColor Cyan
    Write-Host "  1. Thu thập thông tin thiết bị" -ForegroundColor White
    Write-Host "  2. Lấy địa chỉ IP và vị trí GPS" -ForegroundColor White
    Write-Host "  3. Gửi email đến: " -NoNewline -ForegroundColor White
    
    # Đọc email từ config.h nếu có
    if (Test-Path "config.h") {
        $configContent = Get-Content "config.h" -Raw
        if ($configContent -match '#define EMAIL_TO "([^"]+)"') {
            Write-Host $matches[1] -ForegroundColor Green
        }
    } else {
        Write-Host "(xem trong config.h)" -ForegroundColor Gray
    }
    
    Write-Host ""
    Write-Host "Bạn có muốn test chạy ngay bây giờ không? (Y/N): " -ForegroundColor Yellow -NoNewline
    $response = Read-Host
    
    if ($response -eq "Y" -or $response -eq "y") {
        Write-Host ""
        Write-Host "Đang chạy test..." -ForegroundColor Cyan
        Start-ScheduledTask -TaskName $taskName
        Start-Sleep -Seconds 2
        
        # Kiểm tra last run result
        $taskInfo = Get-ScheduledTaskInfo -TaskName $taskName
        Write-Host "Last Run Time: $($taskInfo.LastRunTime)" -ForegroundColor Gray
        Write-Host "Last Result: $($taskInfo.LastTaskResult)" -ForegroundColor Gray
        
        if ($taskInfo.LastTaskResult -eq 0) {
            Write-Host "✓ Test thành công!" -ForegroundColor Green
        }
    }
    
    Write-Host ""
    Write-Host "=== HOÀN TẤT! ===" -ForegroundColor Green
    Write-Host ""
    Write-Host "Để gỡ cài đặt, chạy: " -NoNewline -ForegroundColor Gray
    Write-Host ".\uninstall-startup.ps1" -ForegroundColor Yellow
    
} else {
    Write-Host ""
    Write-Host "LỖI: Không thể tạo Task Scheduler!" -ForegroundColor Red
}

Write-Host ""
Read-Host "Nhấn Enter để thoát"
