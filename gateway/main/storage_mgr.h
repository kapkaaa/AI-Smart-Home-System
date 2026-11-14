/**
 * @file storage_mgr.h
 * @brief Manajer Penyimpanan NVS & LittleFS untuk ESP32-S3 Gateway.
 */

#ifndef STORAGE_MGR_H
#define STORAGE_MGR_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "../../storage_schema/nvs_schema.h"
#include "../../storage_schema/storage_paths.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inisialisasi NVS Flash dan mount partisi LittleFS.
 * @return true jika berhasil
 */
bool storage_mgr_init(void);

/**
 * @brief Membaca mode boot sistem terakhir dari NVS.
 */
uint8_t storage_mgr_get_boot_mode(void);

/**
 * @brief Menyimpan mode boot sistem ke NVS.
 */
void storage_mgr_set_boot_mode(uint8_t mode);

/**
 * @brief Menambah counter boot sistem dan memeriksa crash beruntun.
 * @return uint32_t total boot count
 */
uint32_t storage_mgr_increment_boot_count(void);

/**
 * @brief Membaca kredensial Wi-Fi dari NVS.
 */
bool storage_mgr_get_wifi_creds(char *ssid, size_t ssid_len, char *pass, size_t pass_len);

/**
 * @brief Menyimpan kredensial Wi-Fi ke NVS.
 */
bool storage_mgr_save_wifi_creds(const char *ssid, const char *pass);

/**
 * @brief Mencatat log diagnostik / crash ke LittleFS circular buffer.
 */
void storage_mgr_log_diagnostic(const char *event_tag, const char *msg);

#ifdef __cplusplus
}
#endif

#endif /* STORAGE_MGR_H */
