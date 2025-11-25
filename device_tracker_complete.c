/*
 * Device Tracker Complete - All-in-One Version
 * Tích hợp: Tracker, Installer, Uninstaller
 * Chạy với quyền Administrator
 */

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <curl/curl.h>
#include "config.h"

#ifdef _WIN32
    #include <windows.h>
    #include <winsock2.h>
    #include <iphlpapi.h>
    #include <shlobj.h>
    #include <initguid.h>
    #include <sensors.h>
    #include <sensorsapi.h>
    #include <locationapi.h>
    #pragma comment(lib, "iphlpapi.lib")
    #pragma comment(lib, "ws2_32.lib")
    #pragma comment(lib, "shell32.lib")
    #pragma comment(lib, "ole32.lib")
    #pragma comment(lib, "sensorsapi.lib")
#endif

#define INSTALL_DIR "C:\\ProgramData\\DeviceTracker"
#define TASK_NAME "DeviceTracker"
#define CA_BUNDLE_URL "https://curl.se/ca/cacert.pem"

// Cấu trúc lưu thông tin thiết bị
typedef struct {
    char hostname[256];
    char username[256];
    char os_info[512];
    char ip_address[256];
    char mac_address[18];
    char location[512];
    char timestamp[64];
} DeviceInfo;

// Cấu trúc cho email payload
struct upload_status {
    int lines_read;
    char **payload_text;
    int total_lines;
};

// Forward declarations
int run_tracker();
int install_startup();
int uninstall_startup();
int check_admin();
void show_menu();
void download_ca_bundle();
int get_gps_location(double *latitude, double *longitude);
void request_location_permission();

// Callback function cho curl upload
static size_t payload_source(void *ptr, size_t size, size_t nmemb, void *userp) {
    struct upload_status *upload_ctx = (struct upload_status *)userp;
    const char *data;

    if ((size == 0) || (nmemb == 0) || ((size*nmemb) < 1)) {
        return 0;
    }

    if (upload_ctx->lines_read < upload_ctx->total_lines) {
        data = upload_ctx->payload_text[upload_ctx->lines_read];
        if (data) {
            size_t len = strlen(data);
            memcpy(ptr, data, len);
            upload_ctx->lines_read++;
            return len;
        }
    }

    return 0;
}

// Kiểm tra quyền Administrator
int check_admin() {
#ifdef _WIN32
    BOOL isAdmin = FALSE;
    PSID adminGroup = NULL;
    SID_IDENTIFIER_AUTHORITY ntAuthority = SECURITY_NT_AUTHORITY;
    
    if (AllocateAndInitializeSid(&ntAuthority, 2, SECURITY_BUILTIN_DOMAIN_RID,
        DOMAIN_ALIAS_RID_ADMINS, 0, 0, 0, 0, 0, 0, &adminGroup)) {
        CheckTokenMembership(NULL, adminGroup, &isAdmin);
        FreeSid(adminGroup);
    }
    return isAdmin;
#else
    return (geteuid() == 0);
#endif
}

// Download CA bundle nếu chưa có
void download_ca_bundle() {
    char ca_path[512];
    snprintf(ca_path, sizeof(ca_path), "%s\\curl-ca-bundle.crt", INSTALL_DIR);
    
    // Kiểm tra file đã tồn tại chưa
    FILE *check = fopen(ca_path, "r");
    if (check) {
        fclose(check);
        return; // Đã có rồi
    }
    
    printf("  Downloading CA certificates...\n");
    
    CURL *curl = curl_easy_init();
    if (curl) {
        FILE *fp = fopen(ca_path, "wb");
        if (fp) {
            curl_easy_setopt(curl, CURLOPT_URL, CA_BUNDLE_URL);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // Skip verify for download
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
            
            CURLcode res = curl_easy_perform(curl);
            fclose(fp);
            
            if (res == CURLE_OK) {
                printf("  [OK] CA bundle downloaded\n");
            } else {
                printf("  [ERROR] CA bundle download failed\n");
            }
        }
        curl_easy_cleanup(curl);
    }
}

// Lấy hostname
void get_hostname(char *hostname, size_t size) {
#ifdef _WIN32
    DWORD sz = (DWORD)size;
    GetComputerNameA(hostname, &sz);
#else
    gethostname(hostname, size);
#endif
}

// Lấy username
void get_username(char *username, size_t size) {
#ifdef _WIN32
    DWORD sz = (DWORD)size;
    GetUserNameA(username, &sz);
#else
    strncpy(username, getenv("USER") ? getenv("USER") : "unknown", size - 1);
    username[size - 1] = '\0';
#endif
}

// Lấy thông tin OS
void get_os_info(char *os_info, size_t size) {
#ifdef _WIN32
    snprintf(os_info, size, "Windows OS");
#else
    struct utsname buffer;
    if (uname(&buffer) == 0) {
        snprintf(os_info, size, "%s %s %s", buffer.sysname, buffer.release, buffer.machine);
    } else {
        strncpy(os_info, "Unknown OS", size - 1);
    }
#endif
}

// Lấy địa chỉ IP public
int get_public_ip(char *ip, size_t size) {
    CURL *curl;
    CURLcode res;
    char ca_path[512];
    snprintf(ca_path, sizeof(ca_path), "%s\\curl-ca-bundle.crt", INSTALL_DIR);
    
    FILE *fp = tmpfile();
    if (!fp) return 0;
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org");
        curl_easy_setopt(curl, CURLOPT_CAINFO, ca_path);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if (res == CURLE_OK) {
            rewind(fp);
            if (fgets(ip, size, fp)) {
                ip[strcspn(ip, "\r\n")] = 0;
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

// Lấy vị trí từ IP
int get_location_from_ip(const char *ip, char *location, size_t size) {
    CURL *curl;
    CURLcode res;
    char url[512];
    char ca_path[512];
    snprintf(ca_path, sizeof(ca_path), "%s\\curl-ca-bundle.crt", INSTALL_DIR);
    
    snprintf(url, sizeof(url), "http://ip-api.com/json/%s?fields=status,country,regionName,city,lat,lon,isp", ip);
    
    FILE *fp = tmpfile();
    if (!fp) return 0;
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_CAINFO, ca_path);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if (res == CURLE_OK) {
            rewind(fp);
            char buffer[1024];
            if (fgets(buffer, sizeof(buffer), fp)) {
                char *country = strstr(buffer, "\"country\":\"");
                char *city = strstr(buffer, "\"city\":\"");
                char *lat = strstr(buffer, "\"lat\":");
                char *lon = strstr(buffer, "\"lon\":");
                
                if (country && city && lat && lon) {
                    country += 11;
                    city += 8;
                    lat += 6;
                    lon += 6;
                    
                    char country_str[64] = {0};
                    char city_str[64] = {0};
                    sscanf(country, "%63[^\"]", country_str);
                    sscanf(city, "%63[^\"]", city_str);
                    
                    double latitude, longitude;
                    sscanf(lat, "%lf", &latitude);
                    sscanf(lon, "%lf", &longitude);
                    
                    snprintf(location, size, "%s, %s (%.4f, %.4f)", 
                            city_str, country_str, latitude, longitude);
                    fclose(fp);
                    return 1;
                }
            }
        }
    }
    fclose(fp);
    return 0;
}

// Request location permission
void request_location_permission() {
    printf("  Opening Windows Settings...\n");
    printf("  Please enable 'Location' in Privacy settings.\n");
    
    // Open Settings for user to enable Location
    ShellExecuteA(NULL, "open", "ms-settings:privacy-location", NULL, NULL, SW_SHOWNORMAL);
    
    printf("  After enabling, press Enter to continue...\n");
    while (getchar() != '\n'); // Clear buffer
}

// Lấy vị trí GPS từ Windows bằng PowerShell
int get_gps_location(double *latitude, double *longitude) {
#ifdef _WIN32
    char cmd[1024];
    char tempFile[MAX_PATH];
    
    // Tạo file temp
    GetTempPathA(MAX_PATH, tempFile);
    strcat(tempFile, "gps_temp.txt");
    
    // PowerShell script để lấy GPS
    snprintf(cmd, sizeof(cmd),
        "powershell -Command \"Add-Type -AssemblyName System.Device; "
        "$loc = New-Object System.Device.Location.GeoCoordinateWatcher; "
        "$loc.Start(); "
        "Start-Sleep -Seconds 5; "
        "if ($loc.Position.Location.IsUnknown -eq $false) { "
        "  '{0},{1}' -f $loc.Position.Location.Latitude, $loc.Position.Location.Longitude "
        "} else { "
        "  'UNKNOWN' "
        "}\" > \"%s\" 2>nul", tempFile);
    
    // Chạy PowerShell
    int result = system(cmd);
    
    if (result == 0) {
        // Đọc kết quả
        FILE *fp = fopen(tempFile, "r");
        if (fp) {
            char buffer[256];
            if (fgets(buffer, sizeof(buffer), fp)) {
                buffer[strcspn(buffer, "\r\n")] = 0;
                
                if (strstr(buffer, "UNKNOWN") == NULL && strlen(buffer) > 0) {
                    // Parse latitude,longitude
                    if (sscanf(buffer, "%lf,%lf", latitude, longitude) == 2) {
                        fclose(fp);
                        DeleteFileA(tempFile);
                        return 1;
                    }
                }
            }
            fclose(fp);
        }
        DeleteFileA(tempFile);
    }
#endif
    return 0;
}

// Lấy MAC address
void get_mac_address(char *mac, size_t size) {
#ifdef _WIN32
    IP_ADAPTER_INFO AdapterInfo[16];
    DWORD dwBufLen = sizeof(AdapterInfo);
    
    DWORD dwStatus = GetAdaptersInfo(AdapterInfo, &dwBufLen);
    if (dwStatus == ERROR_SUCCESS) {
        PIP_ADAPTER_INFO pAdapterInfo = AdapterInfo;
        snprintf(mac, size, "%02X:%02X:%02X:%02X:%02X:%02X",
                pAdapterInfo->Address[0], pAdapterInfo->Address[1],
                pAdapterInfo->Address[2], pAdapterInfo->Address[3],
                pAdapterInfo->Address[4], pAdapterInfo->Address[5]);
        return;
    }
#endif
    strncpy(mac, "Not Available", size - 1);
    mac[size - 1] = '\0';
}

// Lấy timestamp
void get_timestamp(char *timestamp, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(timestamp, size, "%Y-%m-%d %H:%M:%S", t);
}

// Collect device info
void collect_device_info(DeviceInfo *info) {
    printf("Collecting device information...\n");
    
    get_hostname(info->hostname, sizeof(info->hostname));
    printf("  Hostname: %s\n", info->hostname);
    
    get_username(info->username, sizeof(info->username));
    printf("  Username: %s\n", info->username);
    
    get_os_info(info->os_info, sizeof(info->os_info));
    printf("  OS: %s\n", info->os_info);
    
    printf("  Getting IP address...\n");
    if (get_public_ip(info->ip_address, sizeof(info->ip_address))) {
        printf("  IP: %s\n", info->ip_address);
    } else {
        strncpy(info->ip_address, "Unable to get IP", sizeof(info->ip_address) - 1);
    }
    
    // Try GPS location first
    printf("  Getting GPS location...\n");
    double gps_lat = 0, gps_lon = 0;
    int has_gps = get_gps_location(&gps_lat, &gps_lon);
    
    if (has_gps && gps_lat != 0 && gps_lon != 0) {
        // GPS accurate
        snprintf(info->location, sizeof(info->location), 
                "GPS: %.6f, %.6f (Accurate)", gps_lat, gps_lon);
        printf("  GPS Location: %.6f, %.6f [OK] Accurate\n", gps_lat, gps_lon);
    } else {
        // No GPS, use IP-based location
        printf("  GPS unavailable, using IP-based location...\n");
        if (strlen(info->ip_address) > 0 && strcmp(info->ip_address, "Unable to get IP") != 0) {
            if (get_location_from_ip(info->ip_address, info->location, sizeof(info->location))) {
                printf("  Location (from IP): %s\n", info->location);
            } else {
                strncpy(info->location, "Unable to determine location", sizeof(info->location) - 1);
            }
        } else {
            strncpy(info->location, "Unable to determine location", sizeof(info->location) - 1);
        }
    }
    
    get_mac_address(info->mac_address, sizeof(info->mac_address));
    printf("  MAC: %s\n", info->mac_address);
    
    get_timestamp(info->timestamp, sizeof(info->timestamp));
    printf("  Time: %s\n", info->timestamp);
}

// Gửi email
int send_email(const DeviceInfo *info) {
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx;
    char ca_path[512];
    snprintf(ca_path, sizeof(ca_path), "%s\\curl-ca-bundle.crt", INSTALL_DIR);
    
    char **payload_text = malloc(20 * sizeof(char*));
    for (int i = 0; i < 20; i++) {
        payload_text[i] = malloc(512);
    }
    
    int line = 0;
    sprintf(payload_text[line++], "To: %s\r\n", EMAIL_TO);
    sprintf(payload_text[line++], "From: %s\r\n", EMAIL_FROM);
    sprintf(payload_text[line++], "Subject: [Device Tracker] Thiết bị %s đã khởi động\r\n", info->hostname);
    sprintf(payload_text[line++], "\r\n");
    sprintf(payload_text[line++], "=== THÔNG TIN THIẾT BỊ ===\r\n");
    sprintf(payload_text[line++], "\r\n");
    sprintf(payload_text[line++], "Tên thiết bị: %s\r\n", info->hostname);
    sprintf(payload_text[line++], "Người dùng: %s\r\n", info->username);
    sprintf(payload_text[line++], "Hệ điều hành: %s\r\n", info->os_info);
    sprintf(payload_text[line++], "Địa chỉ IP: %s\r\n", info->ip_address);
    sprintf(payload_text[line++], "MAC Address: %s\r\n", info->mac_address);
    sprintf(payload_text[line++], "Vị trí: %s\r\n", info->location);
    sprintf(payload_text[line++], "Thời gian: %s\r\n", info->timestamp);
    sprintf(payload_text[line++], "\r\n");
    sprintf(payload_text[line++], "Link Google Maps: https://www.google.com/maps?q=%s\r\n", info->location);
    sprintf(payload_text[line++], "\r\n");
    sprintf(payload_text[line++], "---\r\n");
    sprintf(payload_text[line++], "Email này được gửi tự động bởi Device Tracker\r\n");
    
    upload_ctx.lines_read = 0;
    upload_ctx.payload_text = payload_text;
    upload_ctx.total_lines = line;
    
    curl = curl_easy_init();
    if (curl) {
        char smtp_url[256];
        snprintf(smtp_url, sizeof(smtp_url), "smtp://%s:%d", SMTP_SERVER, SMTP_PORT);
        
        curl_easy_setopt(curl, CURLOPT_URL, smtp_url);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_CAINFO, ca_path);
        curl_easy_setopt(curl, CURLOPT_USERNAME, SMTP_USERNAME);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, SMTP_PASSWORD);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, EMAIL_FROM);
        
        recipients = curl_slist_append(recipients, EMAIL_TO);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        
        printf("\nSending email...\n");
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            fprintf(stderr, "Email error: %s\n", curl_easy_strerror(res));
        } else {
            printf("Email sent successfully!\n");
        }
        
        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }
    
    for (int i = 0; i < 20; i++) {
        free(payload_text[i]);
    }
    free(payload_text);
    
    return (res == CURLE_OK) ? 1 : 0;
}

// Run tracker
int run_tracker() {
    printf("\n=== Device Tracker ===\n");
    printf("Device Location Tracking Application\n\n");
    
    curl_global_init(CURL_GLOBAL_ALL);
    
    // Download CA bundle nếu cần
    download_ca_bundle();
    
    DeviceInfo info;
    memset(&info, 0, sizeof(DeviceInfo));
    collect_device_info(&info);
    
    printf("\n");
    if (send_email(&info)) {
        printf("\n[OK] Complete! Information sent to %s\n", EMAIL_TO);
    } else {
        printf("\n[ERROR] Failed to send email.\n");
    }
    
    curl_global_cleanup();
    return 0;
}

// Install auto-start
int install_startup() {
    if (!check_admin()) {
        printf("\n[ERROR] Administrator privileges required!\n");
        printf("Please run again as Administrator.\n");
        return 1;
    }
    
    printf("\n=== INSTALL AUTO-START ===\n\n");
    
    // Create directory
    printf("1. Creating install directory...\n");
    CreateDirectoryA(INSTALL_DIR, NULL);
    printf("   [OK] %s\n", INSTALL_DIR);
    
    // Copy current exe to directory
    printf("\n2. Copying files...\n");
    char exePath[MAX_PATH];
    char destPath[MAX_PATH];
    GetModuleFileNameA(NULL, exePath, MAX_PATH);
    snprintf(destPath, sizeof(destPath), "%s\\device_tracker.exe", INSTALL_DIR);
    
    if (CopyFileA(exePath, destPath, FALSE)) {
        printf("   [OK] Executable copied\n");
    } else {
        printf("   [ERROR] Copy failed\n");
        return 1;
    }
    
    // Download CA bundle
    printf("\n3. Downloading CA certificates...\n");
    download_ca_bundle();
    
    // Create Task Scheduler
    printf("\n4. Creating Task Scheduler...\n");
    char cmd[1024];
    snprintf(cmd, sizeof(cmd),
        "schtasks /Create /TN \"%s\" /TR \"\\\"%s\\\" --tracker\" "
        "/SC ONSTART /DELAY 0000:30 /RL HIGHEST /F",
        TASK_NAME, destPath);
    
    int result = system(cmd);
    if (result == 0) {
        printf("   [OK] Task Scheduler created\n");
    } else {
        printf("   [ERROR] Task creation failed\n");
        return 1;
    }
    
    printf("\n=== INSTALLATION SUCCESSFUL! ===\n");
    printf("\nProgram will run automatically on Windows startup.\n");
    printf("Email will be sent to: %s\n", EMAIL_TO);
    
    return 0;
}

// Uninstall
int uninstall_startup() {
    if (!check_admin()) {
        printf("\n[ERROR] Administrator privileges required!\n");
        return 1;
    }
    
    printf("\n=== UNINSTALL ===\n\n");
    
    // Remove Task Scheduler
    printf("1. Removing Task Scheduler...\n");
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "schtasks /Delete /TN \"%s\" /F", TASK_NAME);
    
    int result = system(cmd);
    if (result == 0) {
        printf("   [OK] Task Scheduler removed\n");
    } else {
        printf("   [INFO] Task does not exist\n");
    }
    
    // Remove files
    printf("\n2. Removing installation files...\n");
    char exePath[MAX_PATH];
    char caPath[MAX_PATH];
    snprintf(exePath, sizeof(exePath), "%s\\device_tracker.exe", INSTALL_DIR);
    snprintf(caPath, sizeof(caPath), "%s\\curl-ca-bundle.crt", INSTALL_DIR);
    
    DeleteFileA(exePath);
    DeleteFileA(caPath);
    RemoveDirectoryA(INSTALL_DIR);
    printf("   [OK] Files removed\n");
    
    printf("\n=== UNINSTALL SUCCESSFUL! ===\n");
    printf("\nProgram has been completely removed.\n");
    
    return 0;
}

// Show menu
void show_menu() {
    printf("\n+========================================+\n");
    printf("|     DEVICE TRACKER - ALL-IN-ONE        |\n");
    printf("+========================================+\n");
    printf("\n");
    printf("1. Run Tracker (Send location now)\n");
    printf("2. Install Auto-Start (Run on boot)\n");
    printf("3. Uninstall Auto-Start\n");
    printf("4. Check Status\n");
    printf("5. Exit\n");
    printf("\n");
    printf("Choice: ");
}

// Check status
void check_status() {
    printf("\n=== SYSTEM STATUS ===\n\n");
    
    // Check admin rights
    printf("Admin Rights: %s\n", check_admin() ? "[OK] Yes" : "[NO] No");
    
    // Check install directory
    DWORD attr = GetFileAttributesA(INSTALL_DIR);
    printf("Install Directory: %s\n", 
           (attr != INVALID_FILE_ATTRIBUTES) ? "[OK] Exists" : "[NO] Not installed");
    
    // Check Task Scheduler
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "schtasks /Query /TN \"%s\" >nul 2>&1", TASK_NAME);
    int taskExists = (system(cmd) == 0);
    printf("Task Scheduler: %s\n", taskExists ? "[OK] Installed" : "[NO] Not installed");
    
    // Check config
    printf("\nEmail Configuration:\n");
    printf("  Send to: %s\n", EMAIL_TO);
    printf("  SMTP Server: %s:%d\n", SMTP_SERVER, SMTP_PORT);
    
    printf("\n");
}

// Main
int main(int argc, char *argv[]) {
    // Nếu có tham số --tracker, chạy tracker trực tiếp (cho Task Scheduler)
    if (argc > 1 && strcmp(argv[1], "--tracker") == 0) {
        return run_tracker();
    }
    
    // Nếu không có tham số, hiển thị menu
    int choice;
    
    while (1) {
        show_menu();
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n'); // Clear buffer
            printf("\n[ERROR] Invalid choice!\n");
            continue;
        }
        
        switch (choice) {
            case 1:
                run_tracker();
                printf("\nPress Enter to continue...");
                getchar(); getchar();
                break;
                
            case 2:
                install_startup();
                printf("\nPress Enter to continue...");
                getchar(); getchar();
                break;
                
            case 3:
                uninstall_startup();
                printf("\nPress Enter to continue...");
                getchar(); getchar();
                break;
                
            case 4:
                check_status();
                printf("\nPress Enter to continue...");
                getchar(); getchar();
                break;
                
            case 5:
                printf("\nGoodbye!\n");
                return 0;
                
            default:
                printf("\n[ERROR] Invalid choice!\n");
                printf("\nPress Enter to continue...");
                getchar(); getchar();
        }
    }
    
    return 0;
}
