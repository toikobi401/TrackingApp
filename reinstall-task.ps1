# Script cài đặt lại Task Scheduler với DeviceTracker.exe
# Chạy với quyền Administrator

Write-Host "=== Cài đặt lại Device Tracker Task ===" -ForegroundColor Cyan
Write-Host ""

# Kiểm tra quyền Administrator
$isAdmin = ([Security.Principal.WindowsPrincipal] [Security.Principal.WindowsIdentity]::GetCurrent()).IsInRole([Security.Principal.WindowsBuiltInRole]::Administrator)
if (-not $isAdmin) {
    Write-Host "LỖI: Script cần quyền Administrator!" -ForegroundColor Red
    Write-Host "Right-click vào file này → Run as Administrator" -ForegroundColor Yellow
    Read-Host "Nhấn Enter để thoát"
    exit
}

$taskName = "DeviceTracker"
$installDir = "C:\ProgramData\DeviceTracker"
$exePath = "$installDir\DeviceTracker.exe"
$currentDir = Get-Location

# Bước 1: Copy DeviceTracker.exe vào thư mục cài đặt
Write-Host "1. Copy DeviceTracker.exe..." -ForegroundColor Green
if (Test-Path "DeviceTracker.exe") {
    Copy-Item "DeviceTracker.exe" -Destination $exePath -Force
    Write-Host "   ✓ Đã copy DeviceTracker.exe" -ForegroundColor Gray
} else {
    Write-Host "   ✗ Không tìm thấy DeviceTracker.exe" -ForegroundColor Red
    Write-Host "   Vui lòng build trước: gcc -Wall -O2 device_tracker_complete.c -o DeviceTracker.exe ..." -ForegroundColor Yellow
    Read-Host "Nhấn Enter để thoát"
    exit
}

# Bước 2: Xóa Task cũ nếu có
Write-Host ""
Write-Host "2. Xóa Task Scheduler cũ..." -ForegroundColor Green
$existingTask = Get-ScheduledTask -TaskName $taskName -ErrorAction SilentlyContinue
if ($existingTask) {
    Unregister-ScheduledTask -TaskName $taskName -Confirm:$false
    Write-Host "   ✓ Đã xóa task cũ" -ForegroundColor Gray
} else {
    Write-Host "   • Task chưa tồn tại" -ForegroundColor Yellow
}

# Bước 3: Tạo Task mới
Write-Host ""
Write-Host "3. Tạo Task Scheduler mới..." -ForegroundColor Green

$action = New-ScheduledTaskAction -Execute $exePath -Argument "--tracker" -WorkingDirectory $installDir
$trigger = New-ScheduledTaskTrigger -AtStartup
$trigger.Delay = "PT30S"
$settings = New-ScheduledTaskSettingsSet `
    -AllowStartIfOnBatteries `
    -DontStopIfGoingOnBatteries `
    -StartWhenAvailable `
    -RunOnlyIfNetworkAvailable `
    -ExecutionTimeLimit (New-TimeSpan -Minutes 5)
$principal = New-ScheduledTaskPrincipal -UserId "SYSTEM" -LogonType ServiceAccount -RunLevel Highest

Register-ScheduledTask `
    -TaskName $taskName `
    -Action $action `
    -Trigger $trigger `
    -Settings $settings `
    -Principal $principal `
    -Description "Tự động gửi thông tin vị trí thiết bị qua email khi khởi động" `
    -Force | Out-Null

Write-Host "   ✓ Đã tạo Task Scheduler" -ForegroundColor Gray

# Bước 4: Kiểm tra
Write-Host ""
Write-Host "4. Kiểm tra Task..." -ForegroundColor Green
$task = Get-ScheduledTask -TaskName $taskName -ErrorAction SilentlyContinue
if ($task) {
    Write-Host "   ✓ Task Name: $taskName" -ForegroundColor Gray
    Write-Host "   ✓ Trạng thái: $($task.State)" -ForegroundColor Gray
    Write-Host "   ✓ File: $exePath" -ForegroundColor Gray
} else {
    Write-Host "   ✗ Không thể tạo task" -ForegroundColor Red
    Read-Host "Nhấn Enter để thoát"
    exit
}

# Bước 5: Test chạy
Write-Host ""
Write-Host "=== CÀI ĐẶT THÀNH CÔNG! ===" -ForegroundColor Green
Write-Host ""
Write-Host "Bạn có muốn test chạy ngay không? (Y/N): " -ForegroundColor Yellow -NoNewline
$response = Read-Host

if ($response -eq "Y" -or $response -eq "y") {
    Write-Host ""
    Write-Host "Đang test chạy Task..." -ForegroundColor Cyan
    Start-ScheduledTask -TaskName $taskName
    
    Start-Sleep -Seconds 3
    
    $taskInfo = Get-ScheduledTaskInfo -TaskName $taskName
    Write-Host ""
    Write-Host "Kết quả:" -ForegroundColor Cyan
    Write-Host "  • LastRunTime: $($taskInfo.LastRunTime)" -ForegroundColor Gray
    Write-Host "  • LastTaskResult: $($taskInfo.LastTaskResult)" -ForegroundColor Gray
    
    if ($taskInfo.LastTaskResult -eq 0) {
        Write-Host ""
        Write-Host "✓ Test THÀNH CÔNG! Kiểm tra email của bạn." -ForegroundColor Green
    } else {
        Write-Host ""
        Write-Host "✗ Test THẤT BẠI. Mã lỗi: $($taskInfo.LastTaskResult)" -ForegroundColor Red
        Write-Host "  Hãy kiểm tra config.h và thử chạy thủ công: .\DeviceTracker.exe" -ForegroundColor Yellow
    }
}

Write-Host ""
Write-Host "=== HOÀN TẤT! ===" -ForegroundColor Green
Write-Host ""
Read-Host "Nhấn Enter để thoát"
