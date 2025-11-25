/*
 * Device Tracker - All-In-One Installer & Tracker
 * 
 * File .exe DUY NHẤT để:
 * - Tự động tải và cài đặt tất cả dependencies
 * - Cài đặt chương trình vào hệ thống
 * - Chạy tracker gửi email
 * - Gỡ cài đặt hoàn toàn
 * 
 * KHÔNG CẦN bất kỳ file khác!
 * Chỉ cần chạy với quyền Administrator
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
    #pragma comment(lib, "iphlpapi.lib")
    #pragma comment(lib, "ws2_32.lib")
    #pragma comment(lib, "shell32.lib")
    #pragma comment(lib, "ole32.lib")
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
int ensure_dependencies();
int get_gps_location(double *latitude, double *longitude);
void request_location_permission();

// ============================================
// TẢI CA BUNDLE BẰNG CURL
// ============================================
int download_ca_bundle(const char *output_path) {
    printf("  • Đang tải CA Bundle từ %s...\n", CA_BUNDLE_URL);
    
    CURL *curl = curl_easy_init();
    if (!curl) {
        printf("  ✗ Không thể khởi tạo curl\n");
        return 0;
    }
    
    FILE *fp = fopen(output_path, "wb");
    if (!fp) {
        printf("  ✗ Không thể tạo file\n");
        curl_easy_cleanup(curl);
        return 0;
    }
    
    curl_easy_setopt(curl, CURLOPT_URL, CA_BUNDLE_URL);
    curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
    curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
    curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L); // Tạm tắt verify vì chưa có CA bundle
    curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
    
    CURLcode res = curl_easy_perform(curl);
    
    fclose(fp);
    curl_easy_cleanup(curl);
    
    if (res != CURLE_OK) {
        printf("  ✗ Lỗi tải: %s\n", curl_easy_strerror(res));
        DeleteFileA(output_path);
        return 0;
    }
    
    printf("  ✓ Tải CA Bundle thành công\n");
    return 1;
}

// ============================================
// ĐẢM BẢO DEPENDENCIES
// ============================================
int ensure_dependencies() {
    char dll_path[512];
    char ca_path[512];
    char exe_dir[MAX_PATH];
    char local_dll[512];
    char local_ca[512];
    
    // Lấy thư mục của file .exe hiện tại
    GetModuleFileNameA(NULL, exe_dir, MAX_PATH);
    char *last_slash = strrchr(exe_dir, '\\');
    if (last_slash) *last_slash = '\0';
    
    snprintf(dll_path, sizeof(dll_path), "%s\\libcurl-x64.dll", INSTALL_DIR);
    snprintf(ca_path, sizeof(ca_path), "%s\\curl-ca-bundle.crt", INSTALL_DIR);
    
    // Đường dẫn local (cùng thư mục với All_In_One.exe)
    snprintf(local_dll, sizeof(local_dll), "%s\\libcurl-x64.dll", exe_dir);
    snprintf(local_ca, sizeof(local_ca), "%s\\curl-ca-bundle.crt", exe_dir);
    
    // Đường dẫn curl folder
    char curl_dll[512];
    snprintf(curl_dll, sizeof(curl_dll), "%s\\curl-8.11.1_1-win64-mingw\\bin\\libcurl-x64.dll", exe_dir);
    
    printf("\n=== KIỂM TRA DEPENDENCIES ===\n\n");
    
    // Tạo thư mục INSTALL_DIR nếu chưa có
    CreateDirectoryA(INSTALL_DIR, NULL);
    
    // 1. Kiểm tra libcurl DLL
    printf("1. libcurl-x64.dll:\n");
    if (GetFileAttributesA(dll_path) != INVALID_FILE_ATTRIBUTES) {
        printf("  ✓ Đã có sẵn trong %s\n", INSTALL_DIR);
    } else {
        printf("  • Chưa có trong thư mục cài đặt\n");
        printf("  • Đang tìm kiếm...\n");
        
        // Thử copy từ thư mục hiện tại
        if (GetFileAttributesA(local_dll) != INVALID_FILE_ATTRIBUTES) {
            printf("  • Tìm thấy trong thư mục hiện tại, đang copy...\n");
            if (CopyFileA(local_dll, dll_path, FALSE)) {
                printf("  ✓ Copy thành công\n");
            } else {
                printf("  ✗ Không thể copy: %lu\n", GetLastError());
                return 0;
            }
        }
        // Thử copy từ thư mục curl
        else if (GetFileAttributesA(curl_dll) != INVALID_FILE_ATTRIBUTES) {
            printf("  • Tìm thấy trong thư mục curl, đang copy...\n");
            if (CopyFileA(curl_dll, dll_path, FALSE)) {
                printf("  ✓ Copy thành công\n");
            } else {
                printf("  ✗ Không thể copy: %lu\n", GetLastError());
                return 0;
            }
        } else {
            printf("\n");
            printf("  ╔═══════════════════════════════════════════════╗\n");
            printf("  ║  ✗ KHÔNG TÌM THẤY libcurl-x64.dll            ║\n");
            printf("  ╚═══════════════════════════════════════════════╝\n");
            printf("\n");
            printf("  Vui lòng:\n");
            printf("  1. Đặt file 'libcurl-x64.dll' vào thư mục:\n");
            printf("     %s\n", exe_dir);
            printf("  2. HOẶC đảm bảo thư mục 'curl-8.11.1_1-win64-mingw' tồn tại\n");
            printf("\n");
            printf("  File DLL có thể lấy từ:\n");
            printf("  • Thư mục curl-8.11.1_1-win64-mingw\\bin\\\n");
            printf("  • Download từ: https://curl.se/windows/\n");
            printf("\n");
            return 0;
        }
    }
    
    // 2. Kiểm tra CA Bundle
    printf("\n2. curl-ca-bundle.crt:\n");
    if (GetFileAttributesA(ca_path) != INVALID_FILE_ATTRIBUTES) {
        printf("  ✓ Đã có sẵn\n");
    } else {
        printf("  • Chưa có\n");
        
        // Thử copy từ thư mục hiện tại
        if (GetFileAttributesA(local_ca) != INVALID_FILE_ATTRIBUTES) {
            printf("  • Tìm thấy trong thư mục hiện tại, đang copy...\n");
            if (CopyFileA(local_ca, ca_path, FALSE)) {
                printf("  ✓ Copy thành công\n");
            } else {
                printf("  ✗ Không thể copy, đang thử tải từ internet...\n");
                if (!download_ca_bundle(ca_path)) {
                    printf("  ⚠ CẢNH BÁO: Không có CA Bundle\n");
                    printf("  → Chương trình vẫn chạy nhưng SSL có thể gặp vấn đề\n");
                }
            }
        } else {
            printf("  • Đang tải từ internet...\n");
            if (!download_ca_bundle(ca_path)) {
                printf("  ⚠ CẢNH BÁO: Không thể tải CA Bundle\n");
                printf("  → Chương trình vẫn chạy nhưng SSL có thể gặp vấn đề\n");
            }
        }
    }
    
    printf("\n✓ Kiểm tra hoàn tất!\n");
    return 1;
}

// ============================================
// CALLBACK CHO CURL
// ============================================
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

// ============================================
// KIỂM TRA QUYỀN ADMINISTRATOR
// ============================================
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

// ============================================
// LẤY THÔNG TIN HỆ THỐNG
// ============================================
void get_hostname(char *hostname, size_t size) {
#ifdef _WIN32
    DWORD sz = (DWORD)size;
    GetComputerNameA(hostname, &sz);
#else
    gethostname(hostname, size);
#endif
}

void get_username(char *username, size_t size) {
#ifdef _WIN32
    DWORD sz = (DWORD)size;
    GetUserNameA(username, &sz);
#else
    strncpy(username, getenv("USER") ? getenv("USER") : "unknown", size - 1);
    username[size - 1] = '\0';
#endif
}

void get_os_info(char *os_info, size_t size) {
#ifdef _WIN32
    snprintf(os_info, size, "Windows OS");
#endif
}

int get_public_ip(char *ip, size_t size) {
    CURL *curl;
    CURLcode res;
    
    FILE *fp = tmpfile();
    if (!fp) return 0;
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org");
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
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

int get_location_from_ip(const char *ip, char *location, size_t size) {
    CURL *curl;
    CURLcode res;
    char url[512];
    
    snprintf(url, sizeof(url), "http://ip-api.com/json/%s?fields=status,country,regionName,city,lat,lon", ip);
    
    FILE *fp = tmpfile();
    if (!fp) return 0;
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if (res == CURLE_OK) {
            rewind(fp);
            char buffer[2048];
            if (fgets(buffer, sizeof(buffer), fp)) {
                // Parse JSON thủ công
                char *city = strstr(buffer, "\"city\":\"");
                char *region = strstr(buffer, "\"regionName\":\"");
                char *country = strstr(buffer, "\"country\":\"");
                char *lat = strstr(buffer, "\"lat\":");
                char *lon = strstr(buffer, "\"lon\":");
                
                if (city && country && lat && lon) {
                    city += 8;
                    char *city_end = strchr(city, '"');
                    if (region) {
                        region += 14;
                        char *region_end = strchr(region, '"');
                        if (region_end) *region_end = 0;
                    }
                    country += 11;
                    char *country_end = strchr(country, '"');
                    
                    lat += 6;
                    lon += 6;
                    
                    if (city_end) *city_end = 0;
                    if (country_end) *country_end = 0;
                    
                    double latitude = atof(lat);
                    double longitude = atof(lon);
                    
                    snprintf(location, size, "%s, %s (%.4f, %.4f)", 
                             city, country, latitude, longitude);
                    
                    fclose(fp);
                    return 1;
                }
            }
        }
    }
    fclose(fp);
    return 0;
}

void request_location_permission() {
    printf("\n=== YÊU CẦU QUYỀN TRUY CẬP VÍ TRÍ ===\n\n");
    printf("Để có độ chính xác cao nhất, vui lòng:\n");
    printf("1. Mở Windows Settings\n");
    printf("2. Vào Privacy & Security → Location\n");
    printf("3. Bật 'Location services'\n");
    printf("4. Bật 'Let apps access your location'\n\n");
    
    printf("Đang mở Windows Settings...\n");
    system("start ms-settings:privacy-location");
    
    printf("\nSau khi bật, nhấn Enter để tiếp tục...");
    getchar();
}

int get_gps_location(double *latitude, double *longitude) {
#ifdef _WIN32
    printf("  • Đang lấy vị trí GPS (có thể mất 5-10 giây)...\n");
    
    FILE *fp = _popen(
        "powershell -NoProfile -Command \""
        "Add-Type -AssemblyName System.Device; "
        "$loc = New-Object System.Device.Location.GeoCoordinateWatcher; "
        "$loc.Start(); "
        "Start-Sleep -Seconds 5; "
        "if($loc.Position.Location.IsUnknown) { "
        "  Write-Output 'UNKNOWN' "
        "} else { "
        "  Write-Output ($loc.Position.Location.Latitude.ToString('F6')); "
        "  Write-Output ($loc.Position.Location.Longitude.ToString('F6')) "
        "}; "
        "$loc.Stop()\""
    , "r");
    
    if (fp) {
        char line1[128], line2[128];
        if (fgets(line1, sizeof(line1), fp)) {
            line1[strcspn(line1, "\r\n")] = 0;
            if (strcmp(line1, "UNKNOWN") == 0) {
                _pclose(fp);
                printf("  • GPS không khả dụng, sử dụng IP location\n");
                return 0;
            }
            
            if (fgets(line2, sizeof(line2), fp)) {
                line2[strcspn(line2, "\r\n")] = 0;
                *latitude = atof(line1);
                *longitude = atof(line2);
                _pclose(fp);
                
                if (*latitude != 0 && *longitude != 0) {
                    printf("  ✓ GPS: %.6f, %.6f\n", *latitude, *longitude);
                    return 1;
                }
            }
        }
        _pclose(fp);
    }
#endif
    return 0;
}

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

void get_timestamp(char *timestamp, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(timestamp, size, "%Y-%m-%d %H:%M:%S", t);
}

// ============================================
// THU THẬP THÔNG TIN THIẾT BỊ
// ============================================
void collect_device_info(DeviceInfo *info) {
    printf("\n=== THU THẬP THÔNG TIN THIẾT BỊ ===\n\n");
    
    get_hostname(info->hostname, sizeof(info->hostname));
    printf("✓ Hostname: %s\n", info->hostname);
    
    get_username(info->username, sizeof(info->username));
    printf("✓ Username: %s\n", info->username);
    
    get_os_info(info->os_info, sizeof(info->os_info));
    printf("✓ OS: %s\n", info->os_info);
    
    if (get_public_ip(info->ip_address, sizeof(info->ip_address))) {
        printf("✓ IP: %s\n", info->ip_address);
    } else {
        strcpy(info->ip_address, "Unknown");
        printf("✗ Không lấy được IP\n");
    }
    
    get_mac_address(info->mac_address, sizeof(info->mac_address));
    printf("✓ MAC: %s\n", info->mac_address);
    
    printf("\n=== LẤY VỊ TRÍ ===\n\n");
    
    double lat = 0, lon = 0;
    int has_gps = get_gps_location(&lat, &lon);
    
    if (has_gps) {
        snprintf(info->location, sizeof(info->location), 
                 "GPS: %.6f, %.6f (Chính xác)", lat, lon);
    } else {
        if (get_location_from_ip(info->ip_address, info->location, sizeof(info->location))) {
            printf("✓ Vị trí từ IP: %s\n", info->location);
        } else {
            strcpy(info->location, "Unknown");
            printf("✗ Không lấy được vị trí\n");
        }
    }
    
    get_timestamp(info->timestamp, sizeof(info->timestamp));
    printf("✓ Thời gian: %s\n", info->timestamp);
}

// ============================================
// GỬI EMAIL
// ============================================
int send_email(const DeviceInfo *info) {
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx;
    
    printf("\n=== GỬI EMAIL ===\n\n");
    
    // Tạo email content
    char subject[256];
    snprintf(subject, sizeof(subject), "[Device Tracker] Thiết bị %s đã khởi động", info->hostname);
    
    char body[2048];
    snprintf(body, sizeof(body),
        "=== THÔNG TIN THIẾT BỊ ===\r\n\r\n"
        "Tên thiết bị: %s\r\n"
        "Người dùng: %s\r\n"
        "Hệ điều hành: %s\r\n"
        "Địa chỉ IP: %s\r\n"
        "MAC Address: %s\r\n"
        "Vị trí: %s\r\n"
        "Thời gian: %s\r\n\r\n"
        "Link Google Maps: https://www.google.com/maps?q=%s\r\n\r\n"
        "---\r\n"
        "Email này được gửi tự động bởi Device Tracker\r\n",
        info->hostname, info->username, info->os_info,
        info->ip_address, info->mac_address, info->location,
        info->timestamp, info->location
    );
    
    // Tạo payload
    static char *payload_text[20];
    char from_line[256], to_line[256], subject_line[512];
    
    snprintf(from_line, sizeof(from_line), "From: <%s>\r\n", EMAIL_FROM);
    snprintf(to_line, sizeof(to_line), "To: <%s>\r\n", EMAIL_TO);
    snprintf(subject_line, sizeof(subject_line), "Subject: %s\r\n", subject);
    
    payload_text[0] = from_line;
    payload_text[1] = to_line;
    payload_text[2] = subject_line;
    payload_text[3] = "\r\n";
    payload_text[4] = body;
    
    upload_ctx.lines_read = 0;
    upload_ctx.payload_text = payload_text;
    upload_ctx.total_lines = 5;
    
    // Cấu hình libcurl
    char ca_path[512];
    snprintf(ca_path, sizeof(ca_path), "%s\\curl-ca-bundle.crt", INSTALL_DIR);
    
    curl = curl_easy_init();
    if (curl) {
        char smtp_url[256];
        snprintf(smtp_url, sizeof(smtp_url), "smtp://%s:%d", SMTP_SERVER, SMTP_PORT);
        
        curl_easy_setopt(curl, CURLOPT_URL, smtp_url);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_USERNAME, SMTP_USERNAME);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, SMTP_PASSWORD);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, EMAIL_FROM);
        
        recipients = curl_slist_append(recipients, EMAIL_TO);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        
        if (GetFileAttributesA(ca_path) != INVALID_FILE_ATTRIBUTES) {
            curl_easy_setopt(curl, CURLOPT_CAINFO, ca_path);
        }
        
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 0L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 30L);
        
        printf("Đang gửi đến %s...\n", EMAIL_TO);
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            printf("✗ Lỗi: %s\n", curl_easy_strerror(res));
        } else {
            printf("✓ Gửi email thành công!\n");
        }
        
        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }
    
    return (res == CURLE_OK);
}

// ============================================
// CHẠY TRACKER
// ============================================
int run_tracker() {
    DeviceInfo info;
    
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║         CHẠY DEVICE TRACKER            ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    // Đảm bảo dependencies
    if (!ensure_dependencies()) {
        printf("\n✗ Thiếu dependencies cần thiết!\n");
        return 0;
    }
    
    // Thu thập thông tin
    collect_device_info(&info);
    
    // Gửi email
    if (send_email(&info)) {
        printf("\n✓ HOÀN TẤT! Email đã được gửi.\n");
        return 1;
    } else {
        printf("\n✗ Có lỗi khi gửi email.\n");
        return 0;
    }
}

// ============================================
// CÀI ĐẶT AUTO-START
// ============================================
int install_startup() {
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║      CÀI ĐẶT AUTO-START TRACKER        ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    printf("\n=== BƯỚC 1: ĐẢM BẢO DEPENDENCIES ===\n");
    
    if (!ensure_dependencies()) {
        printf("\n✗ Không thể cài đặt do thiếu dependencies!\n");
        return 0;
    }
    
    printf("\n=== BƯỚC 2: COPY FILE THỰC THI ===\n");
    
    // Lấy đường dẫn của file .exe hiện tại
    char current_exe[MAX_PATH];
    GetModuleFileNameA(NULL, current_exe, MAX_PATH);
    
    char install_exe[512];
    snprintf(install_exe, sizeof(install_exe), "%s\\All_In_One.exe", INSTALL_DIR);
    
    printf("  Copy từ: %s\n", current_exe);
    printf("  Copy đến: %s\n", install_exe);
    
    if (CopyFileA(current_exe, install_exe, FALSE)) {
        printf("  ✓ Copy thành công\n");
    } else {
        printf("  ✗ Không thể copy file .exe\n");
        return 0;
    }
    
    printf("\n=== BƯỚC 3: TẠO TASK SCHEDULER ===\n");
    
    // Xóa task cũ nếu có
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), 
             "schtasks /Delete /TN \"%s\" /F >nul 2>&1", 
             TASK_NAME);
    system(cmd);
    
    // Tạo task mới
    snprintf(cmd, sizeof(cmd),
             "schtasks /Create /TN \"%s\" /TR \"\\\"%s\\\" --tracker\" "
             "/SC ONSTART /DELAY 0000:30 /RL HIGHEST /RU SYSTEM /F",
             TASK_NAME, install_exe);
    
    int result = system(cmd);
    
    if (result == 0) {
        printf("  ✓ Tạo Task Scheduler thành công\n");
        printf("\n");
        printf("╔════════════════════════════════════════╗\n");
        printf("║      CÀI ĐẶT THÀNH CÔNG! ✓             ║\n");
        printf("╚════════════════════════════════════════╝\n");
        printf("\n");
        printf("Từ giờ, mỗi khi Windows khởi động:\n");
        printf("  • Chương trình tự động chạy sau 30 giây\n");
        printf("  • Thu thập thông tin thiết bị và vị trí\n");
        printf("  • Gửi email đến %s\n", EMAIL_TO);
        printf("  • Chạy hoàn toàn IM LẶNG trong nền\n");
        printf("\n");
        return 1;
    } else {
        printf("  ✗ Không thể tạo Task Scheduler\n");
        return 0;
    }
}

// ============================================
// GỠ CÀI ĐẶT
// ============================================
int uninstall_startup() {
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║           GỠ CÀI ĐẶT TRACKER           ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    printf("\n1. Xóa Task Scheduler...\n");
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "schtasks /Delete /TN \"%s\" /F", TASK_NAME);
    
    int result = system(cmd);
    if (result == 0) {
        printf("  ✓ Đã xóa Task\n");
    } else {
        printf("  • Task không tồn tại\n");
    }
    
    printf("\n2. Xóa file cài đặt...\n");
    printf("  Xóa thư mục: %s\n", INSTALL_DIR);
    
    char rmdir_cmd[512];
    snprintf(rmdir_cmd, sizeof(rmdir_cmd), "rmdir /S /Q \"%s\" 2>nul", INSTALL_DIR);
    system(rmdir_cmd);
    
    printf("  ✓ Đã xóa file\n");
    
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║       GỠ CÀI ĐẶT THÀNH CÔNG! ✓         ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("\n");
    
    return 1;
}

// ============================================
// KIỂM TRA TRẠNG THÁI
// ============================================
void check_status() {
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║         TRẠNG THÁI HỆ THỐNG            ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("\n");
    
    // Kiểm tra quyền Admin
    printf("1. Quyền Administrator:\n");
    if (check_admin()) {
        printf("  ✓ Đang chạy với quyền Admin\n");
    } else {
        printf("  ✗ KHÔNG có quyền Admin\n");
        printf("  → Vui lòng chạy lại với 'Run as Administrator'\n");
    }
    
    // Kiểm tra thư mục cài đặt
    printf("\n2. Thư mục cài đặt:\n");
    if (GetFileAttributesA(INSTALL_DIR) != INVALID_FILE_ATTRIBUTES) {
        printf("  ✓ Tồn tại: %s\n", INSTALL_DIR);
        
        // Kiểm tra các file
        char dll_path[512], ca_path[512], exe_path[512];
        snprintf(dll_path, sizeof(dll_path), "%s\\libcurl-x64.dll", INSTALL_DIR);
        snprintf(ca_path, sizeof(ca_path), "%s\\curl-ca-bundle.crt", INSTALL_DIR);
        snprintf(exe_path, sizeof(exe_path), "%s\\All_In_One.exe", INSTALL_DIR);
        
        printf("    • libcurl-x64.dll: %s\n", 
               GetFileAttributesA(dll_path) != INVALID_FILE_ATTRIBUTES ? "✓" : "✗");
        printf("    • curl-ca-bundle.crt: %s\n", 
               GetFileAttributesA(ca_path) != INVALID_FILE_ATTRIBUTES ? "✓" : "✗");
        printf("    • All_In_One.exe: %s\n", 
               GetFileAttributesA(exe_path) != INVALID_FILE_ATTRIBUTES ? "✓" : "✗");
    } else {
        printf("  ✗ Chưa tồn tại\n");
    }
    
    // Kiểm tra Task Scheduler
    printf("\n3. Task Scheduler:\n");
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "schtasks /Query /TN \"%s\" >nul 2>&1", TASK_NAME);
    
    if (system(cmd) == 0) {
        printf("  ✓ Task '%s' đang hoạt động\n", TASK_NAME);
        
        // Lấy thông tin chi tiết
        snprintf(cmd, sizeof(cmd), 
                 "schtasks /Query /TN \"%s\" /FO LIST /V 2>nul | findstr \"Status: Next Last\"",
                 TASK_NAME);
        system(cmd);
    } else {
        printf("  ✗ Task chưa được cài đặt\n");
    }
    
    // Cấu hình Email
    printf("\n4. Cấu hình Email:\n");
    printf("  • SMTP Server: %s:%d\n", SMTP_SERVER, SMTP_PORT);
    printf("  • Từ: %s\n", EMAIL_FROM);
    printf("  • Đến: %s\n", EMAIL_TO);
    
    printf("\n");
}

// ============================================
// MENU CHÍNH
// ============================================
void show_menu() {
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║   DEVICE TRACKER - ALL-IN-ONE v2.0     ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("\n");
    printf("  1. 🚀 Chạy Tracker (Gửi vị trí ngay)\n");
    printf("  2. 📥 Cài đặt Auto-Start (Chạy khi boot)\n");
    printf("  3. 🗑️  Gỡ cài đặt Auto-Start\n");
    printf("  4. 📊 Kiểm tra trạng thái\n");
    printf("  5. 🚪 Thoát\n");
    printf("\n");
    printf("════════════════════════════════════════\n");
}

// ============================================
// MAIN
// ============================================
int main(int argc, char *argv[]) {
    // Nếu chạy với --tracker (từ Task Scheduler)
    if (argc > 1 && strcmp(argv[1], "--tracker") == 0) {
        // Chạy tracker và thoát
        run_tracker();
        return 0;
    }
    
    // Kiểm tra quyền Administrator
    if (!check_admin()) {
        printf("\n");
        printf("════════════════════════════════════════\n");
        printf("  ⚠️  CẢNH BÁO: CẦN QUYỀN ADMINISTRATOR\n");
        printf("════════════════════════════════════════\n");
        printf("\n");
        printf("Vui lòng:\n");
        printf("  1. Đóng cửa sổ này\n");
        printf("  2. Right-click vào All_In_One.exe\n");
        printf("  3. Chọn 'Run as Administrator'\n");
        printf("\n");
        printf("Nhấn Enter để thoát...");
        getchar();
        return 1;
    }
    
    // Menu chính
    int choice;
    while (1) {
        show_menu();
        printf("Chọn chức năng (1-5): ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("\n✗ Lựa chọn không hợp lệ!\n");
            Sleep(1000);
            continue;
        }
        while (getchar() != '\n');
        
        switch (choice) {
            case 1:
                run_tracker();
                printf("\nNhấn Enter để tiếp tục...");
                getchar();
                break;
                
            case 2:
                install_startup();
                printf("\nNhấn Enter để tiếp tục...");
                getchar();
                break;
                
            case 3:
                uninstall_startup();
                printf("\nNhấn Enter để tiếp tục...");
                getchar();
                break;
                
            case 4:
                check_status();
                printf("\nNhấn Enter để tiếp tục...");
                getchar();
                break;
                
            case 5:
                printf("\nTạm biệt!\n");
                return 0;
                
            default:
                printf("\n✗ Lựa chọn không hợp lệ!\n");
                Sleep(1000);
        }
    }
    
    return 0;
}
