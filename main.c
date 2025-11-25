/*
 * Device Tracker - Laptop Location Tracker
 * Tự động gửi thông tin vị trí thiết bị qua email khi khởi động
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
    #pragma comment(lib, "iphlpapi.lib")
    #pragma comment(lib, "ws2_32.lib")
#else
    #include <unistd.h>
    #include <sys/utsname.h>
    #include <ifaddrs.h>
    #include <netdb.h>
#endif

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

// Lấy thông tin hostname
void get_hostname(char *hostname, size_t size) {
#ifdef _WIN32
    DWORD sz = (DWORD)size;
    GetComputerNameA(hostname, &sz);
#else
    gethostname(hostname, size);
#endif
}

// Lấy thông tin username
void get_username(char *username, size_t size) {
#ifdef _WIN32
    DWORD sz = (DWORD)size;
    GetUserNameA(username, &sz);
#else
    strncpy(username, getenv("USER") ? getenv("USER") : "unknown", size - 1);
    username[size - 1] = '\0';
#endif
}

// Lấy thông tin hệ điều hành
void get_os_info(char *os_info, size_t size) {
#ifdef _WIN32
    OSVERSIONINFOA osvi;
    ZeroMemory(&osvi, sizeof(OSVERSIONINFOA));
    osvi.dwOSVersionInfoSize = sizeof(OSVERSIONINFOA);
    
    snprintf(os_info, size, "Windows (Version: %ld.%ld Build %ld)",
             osvi.dwMajorVersion, osvi.dwMinorVersion, osvi.dwBuildNumber);
#else
    struct utsname buffer;
    if (uname(&buffer) == 0) {
        snprintf(os_info, size, "%s %s %s", buffer.sysname, buffer.release, buffer.machine);
    } else {
        strncpy(os_info, "Unknown OS", size - 1);
    }
#endif
}

// Lấy địa chỉ IP public qua API
int get_public_ip(char *ip, size_t size) {
    CURL *curl;
    CURLcode res;
    
    FILE *fp = tmpfile();
    if (!fp) return 0;
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, "https://api.ipify.org");
        curl_easy_setopt(curl, CURLOPT_CAINFO, "curl-ca-bundle.crt");
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if (res == CURLE_OK) {
            rewind(fp);
            if (fgets(ip, size, fp)) {
                // Remove newline
                ip[strcspn(ip, "\r\n")] = 0;
                fclose(fp);
                return 1;
            }
        }
    }
    fclose(fp);
    return 0;
}

// Lấy vị trí địa lý từ IP
int get_location_from_ip(const char *ip, char *location, size_t size) {
    CURL *curl;
    CURLcode res;
    char url[512];
    
    snprintf(url, sizeof(url), "http://ip-api.com/json/%s?fields=status,country,regionName,city,lat,lon,isp", ip);
    
    FILE *fp = tmpfile();
    if (!fp) return 0;
    
    curl = curl_easy_init();
    if (curl) {
        curl_easy_setopt(curl, CURLOPT_URL, url);
        curl_easy_setopt(curl, CURLOPT_CAINFO, "curl-ca-bundle.crt");
        curl_easy_setopt(curl, CURLOPT_WRITEDATA, fp);
        curl_easy_setopt(curl, CURLOPT_TIMEOUT, 10L);
        
        res = curl_easy_perform(curl);
        curl_easy_cleanup(curl);
        
        if (res == CURLE_OK) {
            rewind(fp);
            char buffer[1024];
            if (fgets(buffer, sizeof(buffer), fp)) {
                // Simple parsing - tìm country, city, lat, lon
                char *country = strstr(buffer, "\"country\":\"");
                char *city = strstr(buffer, "\"city\":\"");
                char *lat = strstr(buffer, "\"lat\":");
                char *lon = strstr(buffer, "\"lon\":");
                
                if (country && city && lat && lon) {
                    country += 11;
                    city += 8;
                    lat += 6;  // Skip "lat":
                    lon += 6;  // Skip "lon":
                    
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
#else
    struct ifaddrs *ifaddr, *ifa;
    if (getifaddrs(&ifaddr) == -1) {
        strncpy(mac, "Unknown", size - 1);
        return;
    }
    
    for (ifa = ifaddr; ifa != NULL; ifa = ifa->ifa_next) {
        if (ifa->ifa_addr && ifa->ifa_addr->sa_family == AF_PACKET) {
            // Lấy MAC address đầu tiên tìm được
            break;
        }
    }
    freeifaddrs(ifaddr);
#endif
    strncpy(mac, "Not Available", size - 1);
    mac[size - 1] = '\0';
}

// Lấy timestamp hiện tại
void get_timestamp(char *timestamp, size_t size) {
    time_t now = time(NULL);
    struct tm *t = localtime(&now);
    strftime(timestamp, size, "%Y-%m-%d %H:%M:%S", t);
}

// Thu thập tất cả thông tin thiết bị
void collect_device_info(DeviceInfo *info) {
    printf("Đang thu thập thông tin thiết bị...\n");
    
    get_hostname(info->hostname, sizeof(info->hostname));
    printf("  Hostname: %s\n", info->hostname);
    
    get_username(info->username, sizeof(info->username));
    printf("  Username: %s\n", info->username);
    
    get_os_info(info->os_info, sizeof(info->os_info));
    printf("  OS: %s\n", info->os_info);
    
    printf("  Đang lấy địa chỉ IP...\n");
    if (get_public_ip(info->ip_address, sizeof(info->ip_address))) {
        printf("  IP: %s\n", info->ip_address);
        
        printf("  Đang xác định vị trí...\n");
        if (get_location_from_ip(info->ip_address, info->location, sizeof(info->location))) {
            printf("  Location: %s\n", info->location);
        } else {
            strncpy(info->location, "Không xác định được vị trí", sizeof(info->location) - 1);
        }
    } else {
        strncpy(info->ip_address, "Không lấy được IP", sizeof(info->ip_address) - 1);
        strncpy(info->location, "Không xác định được vị trí", sizeof(info->location) - 1);
    }
    
    get_mac_address(info->mac_address, sizeof(info->mac_address));
    printf("  MAC: %s\n", info->mac_address);
    
    get_timestamp(info->timestamp, sizeof(info->timestamp));
    printf("  Time: %s\n", info->timestamp);
}

// Gửi email với thông tin thiết bị
int send_email(const DeviceInfo *info) {
    CURL *curl;
    CURLcode res = CURLE_OK;
    struct curl_slist *recipients = NULL;
    struct upload_status upload_ctx;
    
    // Tạo nội dung email
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
    sprintf(payload_text[line++], "---\r\n");
    sprintf(payload_text[line++], "Email này được gửi tự động bởi Device Tracker\r\n");
    
    upload_ctx.lines_read = 0;
    upload_ctx.payload_text = payload_text;
    upload_ctx.total_lines = line;
    
    curl = curl_easy_init();
    if (curl) {
        // Cấu hình SMTP
        char smtp_url[256];
        snprintf(smtp_url, sizeof(smtp_url), "smtp://%s:%d", SMTP_SERVER, SMTP_PORT);
        
        curl_easy_setopt(curl, CURLOPT_URL, smtp_url);
        curl_easy_setopt(curl, CURLOPT_USE_SSL, (long)CURLUSESSL_ALL);
        curl_easy_setopt(curl, CURLOPT_CAINFO, "curl-ca-bundle.crt");
        curl_easy_setopt(curl, CURLOPT_USERNAME, SMTP_USERNAME);
        curl_easy_setopt(curl, CURLOPT_PASSWORD, SMTP_PASSWORD);
        curl_easy_setopt(curl, CURLOPT_MAIL_FROM, EMAIL_FROM);
        
        recipients = curl_slist_append(recipients, EMAIL_TO);
        curl_easy_setopt(curl, CURLOPT_MAIL_RCPT, recipients);
        
        curl_easy_setopt(curl, CURLOPT_READFUNCTION, payload_source);
        curl_easy_setopt(curl, CURLOPT_READDATA, &upload_ctx);
        curl_easy_setopt(curl, CURLOPT_UPLOAD, 1L);
        
        curl_easy_setopt(curl, CURLOPT_VERBOSE, 1L);
        
        printf("\nĐang gửi email...\n");
        res = curl_easy_perform(curl);
        
        if (res != CURLE_OK) {
            fprintf(stderr, "Lỗi gửi email: %s\n", curl_easy_strerror(res));
        } else {
            printf("Email đã được gửi thành công!\n");
        }
        
        curl_slist_free_all(recipients);
        curl_easy_cleanup(curl);
    }
    
    // Giải phóng bộ nhớ
    for (int i = 0; i < 20; i++) {
        free(payload_text[i]);
    }
    free(payload_text);
    
    return (res == CURLE_OK) ? 1 : 0;
}

int main(int argc, char *argv[]) {
    printf("=== Device Tracker ===\n");
    printf("Ứng dụng theo dõi vị trí thiết bị\n\n");
    
    // Khởi tạo curl
    curl_global_init(CURL_GLOBAL_ALL);
    
    // Thu thập thông tin
    DeviceInfo info;
    memset(&info, 0, sizeof(DeviceInfo));
    collect_device_info(&info);
    
    // Gửi email
    printf("\n");
    if (send_email(&info)) {
        printf("\n✓ Hoàn tất! Thông tin đã được gửi đến %s\n", EMAIL_TO);
    } else {
        printf("\n✗ Có lỗi xảy ra khi gửi email.\n");
        printf("Vui lòng kiểm tra cấu hình SMTP trong config.h\n");
    }
    
    // Dọn dẹp
    curl_global_cleanup();
    
    return 0;
}
