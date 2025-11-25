# Device Tracker - Script gỡ cài đặt
# Chạy script này với quyền Administrator

Write-Host "=== Device Tracker - Gỡ cài đặt ===" -ForegroundColor Cyan
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

$taskName = "DeviceTracker"
$installDir = "$env:ProgramData\DeviceTracker"

# Xóa Task Scheduler
Write-Host "1. Xóa Task Scheduler..." -ForegroundColor Green
$task = Get-ScheduledTask -TaskName $taskName -ErrorAction SilentlyContinue
if ($task) {
    Unregister-ScheduledTask -TaskName $taskName -Confirm:$false
    Write-Host "   ✓ Đã xóa task '$taskName'" -ForegroundColor Gray
} else {
    Write-Host "   • Task không tồn tại" -ForegroundColor Yellow
}

# Xóa thư mục cài đặt
Write-Host ""
Write-Host "2. Xóa file cài đặt..." -ForegroundColor Green
if (Test-Path $installDir) {
    Remove-Item -Path $installDir -Recurse -Force
    Write-Host "   ✓ Đã xóa: $installDir" -ForegroundColor Gray
} else {
    Write-Host "   • Thư mục không tồn tại" -ForegroundColor Yellow
}

Write-Host ""
Write-Host "=== GỠ CÀI ĐẶT THÀNH CÔNG! ===" -ForegroundColor Green
Write-Host ""
Write-Host "Device Tracker đã được gỡ hoàn toàn khỏi hệ thống." -ForegroundColor Gray
Write-Host "Chương trình sẽ KHÔNG tự động chạy khi khởi động nữa." -ForegroundColor Gray
Write-Host ""

Read-Host "Nhấn Enter để thoát"
