/*
 * Device Tracker - All-In-One Installer & Tracker
 * 
 * Single .exe file to:
 * - Automatically download and install all dependencies
 * - Install program into system
 * - Run tracker and send email
 * - Complete uninstallation
 * 
 * NO OTHER FILES NEEDED!
 * Just run with Administrator privileges
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
int check_internet_connection();
int run_starter_mode();

// ============================================
// TẢI CA BUNDLE BẰNG CURL
// ============================================
int download_ca_bundle(const char *output_path) {
    printf("  • Downloading CA Bundle from %s...\n", CA_BUNDLE_URL);
    
    CURL *curl = curl_easy_init();
    if (!curl) {
        printf("  ✗ Cannot initialize curl\n");
        return 0;
    }
    
    FILE *fp = fopen(output_path, "wb");
    if (!fp) {
        printf("  ✗ Cannot create file\n");
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
        printf("  ✗ Download error: %s\n", curl_easy_strerror(res));
        DeleteFileA(output_path);
        return 0;
    }
    
    printf("  ✓ CA Bundle downloaded successfully\n");
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
    
    printf("\n=== CHECKING DEPENDENCIES ===\n\n");
    
    // Create INSTALL_DIR if not exists
    CreateDirectoryA(INSTALL_DIR, NULL);
    
    // 1. Check libcurl DLL
    printf("1. libcurl-x64.dll:\n");
    if (GetFileAttributesA(dll_path) != INVALID_FILE_ATTRIBUTES) {
        printf("  ✓ Already available in %s\n", INSTALL_DIR);
    } else {
        printf("  • Not found in installation folder\n");
        printf("  • Searching...\n");
        
        // Try copy from current directory
        if (GetFileAttributesA(local_dll) != INVALID_FILE_ATTRIBUTES) {
            printf("  • Found in current directory, copying...\n");
            if (CopyFileA(local_dll, dll_path, FALSE)) {
                printf("  ✓ Copied successfully\n");
            } else {
                printf("  ✗ Cannot copy: %lu\n", GetLastError());
                return 0;
            }
        }
        // Try copy from curl directory
        else if (GetFileAttributesA(curl_dll) != INVALID_FILE_ATTRIBUTES) {
            printf("  • Found in curl directory, copying...\n");
            if (CopyFileA(curl_dll, dll_path, FALSE)) {
                printf("  ✓ Copied successfully\n");
            } else {
                printf("  ✗ Cannot copy: %lu\n", GetLastError());
                return 0;
            }
        } else {
            printf("\n");
            printf("  ╔═══════════════════════════════════════════════╗\n");
            printf("  ║  ✗ libcurl-x64.dll NOT FOUND                 ║\n");
            printf("  ╚═══════════════════════════════════════════════╝\n");
            printf("\n");
            printf("  Please:\n");
            printf("  1. Place 'libcurl-x64.dll' file in folder:\n");
            printf("     %s\n", exe_dir);
            printf("  2. OR ensure 'curl-8.11.1_1-win64-mingw' folder exists\n");
            printf("\n");
            printf("  DLL file can be obtained from:\n");
            printf("  • curl-8.11.1_1-win64-mingw\\bin\\ folder\n");
            printf("  • Download from: https://curl.se/windows/\n");
            printf("\n");
            return 0;
        }
    }
    
    // 2. Check CA Bundle
    printf("\n2. curl-ca-bundle.crt:\n");
    if (GetFileAttributesA(ca_path) != INVALID_FILE_ATTRIBUTES) {
        printf("  ✓ Already available\n");
    } else {
        printf("  • Not found\n");
        
        // Try copy from current directory
        if (GetFileAttributesA(local_ca) != INVALID_FILE_ATTRIBUTES) {
            printf("  • Found in current directory, copying...\n");
            if (CopyFileA(local_ca, ca_path, FALSE)) {
                printf("  ✓ Copied successfully\n");
            } else {
                printf("  ✗ Cannot copy, trying to download from internet...\n");
                if (!download_ca_bundle(ca_path)) {
                    printf("  ⚠ WARNING: No CA Bundle\n");
                    printf("  → Program will run but SSL may have issues\n");
                }
            }
        } else {
            printf("  • Downloading from internet...\n");
            if (!download_ca_bundle(ca_path)) {
                printf("  ⚠ WARNING: Cannot download CA Bundle\n");
                printf("  → Program will run but SSL may have issues\n");
            }
        }
    }
    
    printf("\n✓ Check complete!\n");
    return 1;
}

// ============================================
// KIỂM TRA KẾT NỐI INTERNET
// ============================================
int check_internet_connection() {
    CURL *curl;
    CURLcode res;
    int connected = 0;
    
    curl_global_init(CURL_GLOBAL_DEFAULT);
    curl = curl_easy_init();
    
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "http://www.google.com");
        curl_easy_setopt(curl, CURLOPT_NOBODY, 1L);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 5L);
        curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
        
        res = curl_easy_perform(curl);
        connected = (res == CURLE_OK);
        
        curl_easy_cleanup(curl);
    }
    
    curl_global_cleanup();
    return connected;
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
    
    // Try multiple APIs with different methods
    const char *apis[] = {
        "https://api.ipify.org",
        "http://api.ipify.org",  // HTTP fallback
        "https://checkip.amazonaws.com",
        "http://ifconfig.me/ip"
    };
    
    for (int i = 0; i < 4; i++) {
        FILE *fp = tmpfile();
        if (!fp) continue;
        
        curl = curl_easy_init();
        if (curl) {
            curl_easy_setopt(curl, CURLOPT_URL, apis[i]);
            curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
            curl_easy_setopt(curl, CURLOPT_TIMEOUT, 15L);  // Increased timeout
            curl_easy_setopt(curl, CURLOPT_FOLLOWLOCATION, 1L);
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYPEER, 0L);  // Disable SSL verify
            curl_easy_setopt(curl, CURLOPT_SSL_VERIFYHOST, 0L);
            
            res = curl_easy_perform(curl);
            curl_easy_cleanup(curl);
            
            if (res == CURLE_OK) {
                rewind(fp);
                if (fgets(ip, size, fp)) {
                    ip[strcspn(ip, "\r\n")] = 0;
                    // Remove any whitespace
                    char *start = ip;
                    while (*start == ' ' || *start == '\t') start++;
                    if (start != ip) memmove(ip, start, strlen(start) + 1);
                    
                    fclose(fp);
                    if (strlen(ip) > 0) {
                        return 1;  // Success!
                    }
                }
            }
        }
        fclose(fp);
    }
    
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
    printf("\n=== LOCATION ACCESS PERMISSION REQUIRED ===\n\n");
    printf("For highest accuracy, please:\n");
    printf("1. Open Windows Settings\n");
    printf("2. Go to Privacy & Security → Location\n");
    printf("3. Enable 'Location services'\n");
    printf("4. Enable 'Let apps access your location'\n\n");
    
    printf("Opening Windows Settings...\n");
    system("start ms-settings:privacy-location");
    
    printf("\nAfter enabling, press Enter to continue...");
    getchar();
}

int get_gps_location(double *latitude, double *longitude) {
#ifdef _WIN32
    printf("  • Getting GPS location (may take 5-10 seconds)...\n");
    
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
                printf("  • GPS unavailable, using IP location\n");
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
    printf("\n=== COLLECTING DEVICE INFO ===\n\n");
    
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
        printf("✗ Cannot get IP\n");
    }
    
    get_mac_address(info->mac_address, sizeof(info->mac_address));
    printf("✓ MAC: %s\n", info->mac_address);
    
    printf("\n=== GETTING LOCATION ===\n\n");
    
    double lat = 0, lon = 0;
    int has_gps = get_gps_location(&lat, &lon);
    
    if (has_gps) {
        snprintf(info->location, sizeof(info->location), 
                 "GPS: %.6f, %.6f (Accurate)", lat, lon);
    } else {
        if (get_location_from_ip(info->ip_address, info->location, sizeof(info->location))) {
            printf("✓ Location from IP: %s\n", info->location);
        } else {
            strcpy(info->location, "Unknown");
            printf("✗ Cannot get location\n");
        }
    }
    
    get_timestamp(info->timestamp, sizeof(info->timestamp));
    printf("✓ Time: %s\n", info->timestamp);
}

// ============================================
// GỬI EMAIL
// ============================================
int send_email(const DeviceInfo *info) {
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx;
    
    printf("\n=== SENDING EMAIL ===\n\n");
    
    // Create email content
    char subject[256];
    snprintf(subject, sizeof(subject), "[Device Tracker] Device %s has started", info->hostname);
    
    char body[2048];
    snprintf(body, sizeof(body),
        "=== DEVICE INFORMATION ===\r\n\r\n"
        "Device name: %s\r\n"
        "User: %s\r\n"
        "Operating system: %s\r\n"
        "IP address: %s\r\n"
        "MAC Address: %s\r\n"
        "Location: %s\r\n"
        "Time: %s\r\n\r\n"
        "Google Maps Link: https://www.google.com/maps?q=%s\r\n\r\n"
        "---\r\n"
        "This email was sent automatically by Device Tracker\r\n",
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
        
        printf("Sending to %s...\n", EMAIL_TO);
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            printf("✗ Error: %s\n", curl_easy_strerror(res));
        } else {
            printf("✓ Email sent successfully!\n");
        }
        
        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }
    
    return (res == CURLE_OK);
}

// ============================================
// STARTER MODE - Chạy khi Windows khởi động
// ============================================
int run_starter_mode() {
    DeviceInfo info;
    
    // Đảm bảo dependencies
    ensure_dependencies();
    
    // Vòng lặp chờ internet VÔ HẠN
    while (1) {
        if (check_internet_connection()) {
            // Có internet rồi, collect info và gửi
            collect_device_info(&info);
            
            if (send_email(&info)) {
                // Gửi thành công → THOÁT
                return 1;
            }
            
            // Gửi thất bại, thử lại sau 30 giây
            Sleep(30000);
        } else {
            // Chưa có internet, đợi 10 giây rồi kiểm tra lại
            Sleep(10000);
        }
    }
    
    return 0;
}

// ============================================
// CHẠY TRACKER
// ============================================
int run_tracker() {
    DeviceInfo info;
    
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║         RUN DEVICE TRACKER             ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    // Ensure dependencies
    if (!ensure_dependencies()) {
        printf("\n✗ Missing required dependencies!\n");
        return 0;
    }
    
    // Collect info
    collect_device_info(&info);
    
    // Send email
    if (send_email(&info)) {
        printf("\n✓ COMPLETE! Email has been sent.\n");
        return 1;
    } else {
        printf("\n✗ Error sending email.\n");
        return 0;
    }
}

// ============================================
// CÀI ĐẶT AUTO-START
// ============================================
int install_startup() {
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║      INSTALL AUTO-START TRACKER         ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    printf("\n=== STEP 1: ENSURE DEPENDENCIES ===\n");
    
    if (!ensure_dependencies()) {
        printf("\n✗ Cannot install due to missing dependencies!\n");
        return 0;
    }
    
    printf("\n=== STEP 2: COPY EXECUTABLE FILE ===\n");
    
    // Lấy đường dẫn của file .exe hiện tại
    char current_exe[MAX_PATH];
    GetModuleFileNameA(NULL, current_exe, MAX_PATH);
    
    // Tạo đường dẫn Silent version
    char exe_dir[MAX_PATH];
    strcpy(exe_dir, current_exe);
    char *last_slash = strrchr(exe_dir, '\\');
    if (last_slash) *last_slash = '\0';
    
    char silent_exe[MAX_PATH];
    snprintf(silent_exe, sizeof(silent_exe), "%s\\All_In_One_Silent.exe", exe_dir);
    
    char install_exe[512];
    snprintf(install_exe, sizeof(install_exe), "%s\\All_In_One_Silent.exe", INSTALL_DIR);
    
    // Check if Silent version exists
    if (GetFileAttributesA(silent_exe) != INVALID_FILE_ATTRIBUTES) {
        printf("  Copy from: %s\n", silent_exe);
        printf("  Copy to: %s\n", install_exe);
        
        if (CopyFileA(silent_exe, install_exe, FALSE)) {
            printf("  ✓ All_In_One_Silent.exe copied successfully\n");
        } else {
            printf("  ✗ Cannot copy Silent.exe file\n");
            return 0;
        }
    } else {
        // Fallback: copy current file if no Silent version
        printf("  ⚠ All_In_One_Silent.exe not found\n");
        printf("  → Using current file: %s\n", current_exe);
        
        snprintf(install_exe, sizeof(install_exe), "%s\\All_In_One.exe", INSTALL_DIR);
        
        if (CopyFileA(current_exe, install_exe, FALSE)) {
            printf("  ✓ Copied successfully\n");
        } else {
            printf("  ✗ Cannot copy .exe file\n");
            return 0;
        }
    }
    
    printf("\n=== STEP 3: CREATE TASK SCHEDULER ===\n");
    
    // Xóa task cũ nếu có
    char cmd[1024];
    snprintf(cmd, sizeof(cmd), 
             "schtasks /Delete /TN \"%s\" /F >nul 2>&1", 
             TASK_NAME);
    system(cmd);
    
    // Tạo task mới với argument --starter
    snprintf(cmd, sizeof(cmd),
             "schtasks /Create /TN \"%s\" /TR \"\\\"%s\\\" --starter\" "
             "/SC ONSTART /DELAY 0000:30 /RL HIGHEST /RU SYSTEM /F",
             TASK_NAME, install_exe);
    
    int result = system(cmd);
    
    if (result == 0) {
        printf("  ✓ Task Scheduler created successfully\n");
        printf("\n");
        printf("╔════════════════════════════════════════╗\n");
        printf("║      INSTALLATION SUCCESSFUL! ✓          ║\n");
        printf("╚════════════════════════════════════════╝\n");
        printf("\n");
        printf("From now on, every time Windows starts:\n");
        printf("  • Program auto-runs after 30 seconds\n");
        printf("  • Collects device and location info\n");
        printf("  • Sends email to %s\n", EMAIL_TO);
        printf("  • Runs completely SILENT in background\n");
        printf("\n");
        return 1;
    } else {
        printf("  ✗ Cannot create Task Scheduler\n");
        return 0;
    }
}

// ============================================
// GỠ CÀI ĐẶT
// ============================================
int uninstall_startup() {
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║           UNINSTALL TRACKER            ║\n");
    printf("╚════════════════════════════════════════╝\n");
    
    printf("\n1. Delete Task Scheduler...\n");
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "schtasks /Delete /TN \"%s\" /F", TASK_NAME);
    
    int result = system(cmd);
    if (result == 0) {
        printf("  ✓ Task deleted\n");
    } else {
        printf("  • Task does not exist\n");
    }
    
    printf("\n2. Delete installation files...\n");
    printf("  Deleting folder: %s\n", INSTALL_DIR);
    
    char rmdir_cmd[512];
    snprintf(rmdir_cmd, sizeof(rmdir_cmd), "rmdir /S /Q \"%s\" 2>nul", INSTALL_DIR);
    system(rmdir_cmd);
    
    printf("  ✓ Files deleted\n");
    
    printf("\n");
    printf("╔════════════════════════════════════════╗\n");
    printf("║       UNINSTALL SUCCESSFUL! ✓          ║\n");
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
    printf("║           SYSTEM STATUS                ║\n");
    printf("╚════════════════════════════════════════╝\n");
    printf("\n");
    
    // Check Admin privileges
    printf("1. Administrator privileges:\n");
    if (check_admin()) {
        printf("  ✓ Running with Admin rights\n");
    } else {
        printf("  ✗ NO Admin rights\n");
        printf("  → Please run again with 'Run as Administrator'\n");
    }
    
    // Check installation directory
    printf("\n2. Installation folder:\n");
    if (GetFileAttributesA(INSTALL_DIR) != INVALID_FILE_ATTRIBUTES) {
        printf("  ✓ Exists: %s\n", INSTALL_DIR);
        
        // Check files
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
        printf("  ✗ Does not exist\n");
    }
    
    // Check Task Scheduler
    printf("\n3. Task Scheduler:\n");
    char cmd[512];
    snprintf(cmd, sizeof(cmd), "schtasks /Query /TN \"%s\" >nul 2>&1", TASK_NAME);
    
    if (system(cmd) == 0) {
        printf("  ✓ Task '%s' is active\n", TASK_NAME);
        
        // Get detailed info
        snprintf(cmd, sizeof(cmd), 
                 "schtasks /Query /TN \"%s\" /FO LIST /V 2>nul | findstr \"Status: Next Last\"",
                 TASK_NAME);
        system(cmd);
    } else {
        printf("  ✗ Task not installed\n");
    }
    
    // Email Configuration
    printf("\n4. Email Configuration:\n");
    printf("  • SMTP Server: %s:%d\n", SMTP_SERVER, SMTP_PORT);
    printf("  • From: %s\n", EMAIL_FROM);
    printf("  • To: %s\n", EMAIL_TO);
    
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
    printf("  1. 🚀 Run Tracker (Send location now)\n");
    printf("  2. 📥 Install Auto-Start (Run on boot)\n");
    printf("  3. 🗑️  Uninstall Auto-Start\n");
    printf("  4. 📊 Check Status\n");
    printf("  5. 🚪 Exit\n");
    printf("\n");
    printf("════════════════════════════════════════\n");
}

// ============================================
// MAIN
// ============================================
int main(int argc, char *argv[]) {
    // If run with --starter (from Task Scheduler - Startup Mode)
    if (argc > 1 && strcmp(argv[1], "--starter") == 0) {
        // Run starter mode: wait for internet, send email, exit
        return run_starter_mode();
    }
    
    // If run with --tracker (manual run without console)
    if (argc > 1 && strcmp(argv[1], "--tracker") == 0) {
        // Run tracker and exit
        run_tracker();
        return 0;
    }
    
    // Check Administrator privileges
    if (!check_admin()) {
        printf("\n");
        printf("════════════════════════════════════════\n");
        printf("  ⚠️  WARNING: ADMINISTRATOR RIGHTS REQUIRED\n");
        printf("════════════════════════════════════════\n");
        printf("\n");
        printf("Please:\n");
        printf("  1. Close this window\n");
        printf("  2. Right-click on All_In_One.exe\n");
        printf("  3. Select 'Run as Administrator'\n");
        printf("\n");
        printf("Press Enter to exit...");
        getchar();
        return 1;
    }
    
    // Main menu
    int choice;
    while (1) {
        show_menu();
        printf("Select option (1-5): ");
        
        if (scanf("%d", &choice) != 1) {
            while (getchar() != '\n');
            printf("\n✗ Invalid choice!\n");
            Sleep(1000);
            continue;
        }
        while (getchar() != '\n');
        
        switch (choice) {
            case 1:
                run_tracker();
                printf("\nPress Enter to continue...");
                getchar();
                break;
                
            case 2:
                install_startup();
                printf("\nPress Enter to continue...");
                getchar();
                break;
                
            case 3:
                uninstall_startup();
                printf("\nPress Enter to continue...");
                getchar();
                break;
                
            case 4:
                check_status();
                printf("\nPress Enter to continue...");
                getchar();
                break;
                
            case 5:
                printf("\nGoodbye!\n");
                return 0;
                
            default:
                printf("\n✗ Invalid choice!\n");
                Sleep(1000);
        }
    }
    
    return 0;
}
