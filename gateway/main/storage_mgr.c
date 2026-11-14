/**
 * @file storage_mgr.c
 * @brief Implementasi Storage Manager untuk NVS & LittleFS.
 */

#include "storage_mgr.h"
#include <stdio.h>
#include <string.h>

#ifdef ESP_PLATFORM
#include "esp_spiffs.h"
#include "nvs_flash.h"
#include "nvs.h"
#include "esp_log.h"

static const char *TAG = "STORAGE_MGR";

bool storage_mgr_init(void) {
    ESP_LOGI(TAG, "Inisialisasi NVS Flash...");
    esp_err_t err = nvs_flash_init();
    if (err == ESP_ERR_NVS_NO_FREE_PAGES || err == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        nvs_flash_erase();
        err = nvs_flash_init();
    }
    if (err != ESP_OK) return false;

    ESP_LOGI(TAG, "Mount partisi LittleFS / SPIFFS...");
    esp_vfs_spiffs_conf_t conf = {
        .base_path = LITTLEFS_BASE_PATH,
        .partition_label = "storage",
        .max_files = 5,
        .format_if_mount_failed = true
    };
    err = esp_vfs_spiffs_register(&conf);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Gagal mount LittleFS: %s", esp_err_to_name(err));
        return false;
    }
    ESP_LOGI(TAG, "LittleFS berhasil dimount pada %s", LITTLEFS_BASE_PATH);
    return true;
}

uint8_t storage_mgr_get_boot_mode(void) {
    nvs_handle_t h;
    uint8_t mode = 0x01; /* Default SYS_MODE_1A */
    if (nvs_open(NVS_NS_GATEWAY_STATE, NVS_READONLY, &h) == ESP_OK) {
        nvs_get_u8(h, NVS_KEY_BOOT_MODE, &mode);
        nvs_close(h);
    }
    return mode;
}

void storage_mgr_set_boot_mode(uint8_t mode) {
    nvs_handle_t h;
    if (nvs_open(NVS_NS_GATEWAY_STATE, NVS_READWRITE, &h) == ESP_OK) {
        nvs_set_u8(h, NVS_KEY_BOOT_MODE, mode);
        nvs_commit(h);
        nvs_close(h);
    }
}

uint32_t storage_mgr_increment_boot_count(void) {
    nvs_handle_t h;
    uint32_t cnt = 0;
    if (nvs_open(NVS_NS_GATEWAY_STATE, NVS_READWRITE, &h) == ESP_OK) {
        nvs_get_u32(h, NVS_KEY_BOOT_COUNT, &cnt);
        cnt++;
        nvs_set_u32(h, NVS_KEY_BOOT_COUNT, cnt);
        nvs_commit(h);
        nvs_close(h);
    }
    return cnt;
}

bool storage_mgr_get_wifi_creds(char *ssid, size_t ssid_len, char *pass, size_t pass_len) {
    nvs_handle_t h;
    if (nvs_open(NVS_NS_GATEWAY_NET, NVS_READONLY, &h) != ESP_OK) return false;
    esp_err_t err1 = nvs_get_str(h, NVS_KEY_WIFI_SSID, ssid, &ssid_len);
    esp_err_t err2 = nvs_get_str(h, NVS_KEY_WIFI_PASS, pass, &pass_len);
    nvs_close(h);
    return (err1 == ESP_OK && err2 == ESP_OK);
}

bool storage_mgr_save_wifi_creds(const char *ssid, const char *pass) {
    nvs_handle_t h;
    if (nvs_open(NVS_NS_GATEWAY_NET, NVS_READWRITE, &h) != ESP_OK) return false;
    nvs_set_str(h, NVS_KEY_WIFI_SSID, ssid);
    nvs_set_str(h, NVS_KEY_WIFI_PASS, pass);
    nvs_commit(h);
    nvs_close(h);
    return true;
}

void storage_mgr_log_diagnostic(const char *event_tag, const char *msg) {
    FILE *f = fopen(PATH_DIAGNOSTIC_LOG, "a");
    if (f) {
        fprintf(f, "[%s] %s\n", event_tag, msg);
        fclose(f);
    }
}

#else
/* Simulasi / Test Host */
static uint8_t s_mock_boot_mode = 0x01;
static uint32_t s_mock_boot_cnt = 0;

bool storage_mgr_init(void) { return true; }
uint8_t storage_mgr_get_boot_mode(void) { return s_mock_boot_mode; }
void storage_mgr_set_boot_mode(uint8_t mode) { s_mock_boot_mode = mode; }
uint32_t storage_mgr_increment_boot_count(void) { return ++s_mock_boot_cnt; }
bool storage_mgr_get_wifi_creds(char *s, size_t sl, char *p, size_t pl) { (void)s; (void)sl; (void)p; (void)pl; return false; }
bool storage_mgr_save_wifi_creds(const char *s, const char *p) { (void)s; (void)p; return true; }
void storage_mgr_log_diagnostic(const char *t, const char *m) { printf("[MOCK_LOG][%s] %s\n", t, m); }
#endif
