/**
 * @file main.c
 * @brief Firmware ESP32-S3 Gateway Utama (Local-First Brain).
 * @details Mengelola State Machine 4 Mode, Dual-Core FreeRTOS, ESP-NOW Coexistence,
 *          Global Rule Engine, dan Inferensi AI di PSRAM.
 */

#include <stdio.h>
#include <string.h>
#include "../../common/include/system_types.h"
#include "../../common/include/protocol.h"
#include "../../common/include/protocol_validator.h"
#include "../../common/include/pinout_defs.h"
#include "gateway_hal.h"
#include "storage_mgr.h"
#include "ai_engine.h"
#include "blynk_bridge.h"
#include "external_protocols.h"

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_log.h"
#include "nvs_flash.h"

static const char *TAG = "GATEWAY_CORE";
#else
#define ESP_LOGI(tag, fmt, ...) printf("[%s] " fmt "\n", tag, ##__VA_ARGS__)
#define ESP_LOGW(tag, fmt, ...) printf("[%s][WARN] " fmt "\n", tag, ##__VA_ARGS__)
#define ESP_LOGE(tag, fmt, ...) printf("[%s][ERR] " fmt "\n", tag, ##__VA_ARGS__)
#endif

/* Status State Machine Gateway */
static system_mode_t s_current_mode = SYS_MODE_1A_GATEWAY_SETUP;
static bool s_pairing_window_open = false;
static uint32_t s_pairing_timer_ms = 0;

/**
 * @brief Transisi Mode State Machine Gateway.
 */
void gateway_switch_mode(system_mode_t new_mode) {
    if (s_current_mode == new_mode) return;

    ESP_LOGI(TAG, "Transisi State Machine: 0x%02X -> 0x%02X", s_current_mode, new_mode);
    s_current_mode = new_mode;
    gateway_hal_set_led_mode(new_mode);
    storage_mgr_set_boot_mode((uint8_t)new_mode);

    if (new_mode == SYS_MODE_3_FAULT) {
        /* Evaluasi AI dan kirim alert darurat */
        blynk_bridge_send_notification("PERINGATAN SISTEM GATEWAY", 
            "Terdeteksi anomali kritis. Sistem dialihkan ke Mode 3 (Fault Mode). Buka Web Lokal untuk opsi perbaikan.");
    }
}

/**
 * @brief Handler Paket Masuk ESP-NOW (Core 0).
 */
static void on_gateway_espnow_recv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    if (!protocol_validate_packet(data, (size_t)data_len)) {
        return;
    }

    const pkt_header_t *hdr = (const pkt_header_t *)data;

    switch (hdr->msg_type) {
        case PKT_TYPE_DISCOVERY_REQ: {
            if (s_current_mode == SYS_MODE_1B_NODE_SETUP || s_pairing_window_open) {
                ESP_LOGI(TAG, "Discovery Beacon diterima dari Node: %02X:%02X:%02X:%02X:%02X:%02X",
                         mac_addr[0], mac_addr[1], mac_addr[2], mac_addr[3], mac_addr[4], mac_addr[5]);

                /* Kirim Response Terima Pairing */
                pkt_discovery_resp_t resp;
                memset(&resp, 0, sizeof(resp));
                resp.header.magic = PROTOCOL_MAGIC;
                resp.header.version = PROTOCOL_VERSION;
                resp.header.msg_type = PKT_TYPE_DISCOVERY_RESP;
                resp.status = 1; /* Accepted */
                resp.assigned_channel = 1;

#ifdef ESP_PLATFORM
                esp_now_send(mac_addr, (uint8_t *)&resp, sizeof(resp));
#endif
            }
            break;
        }

        case PKT_TYPE_TELEMETRY: {
            const pkt_telemetry_t *telem = (const pkt_telemetry_t *)data;
            for (uint8_t i = 0; i < telem->reading_count; i++) {
                ESP_LOGI(TAG, "Telemetri Node: Driver %d = %.2f", 
                         telem->readings[i].driver_id, telem->readings[i].value);
                /* Teruskan nilai ke Blynk Cloud */
                blynk_bridge_write_vpin(i, telem->readings[i].value);
            }
            break;
        }

        case PKT_TYPE_HEARTBEAT: {
            const pkt_heartbeat_t *hb = (const pkt_heartbeat_t *)data;
            ESP_LOGI(TAG, "Node Heartbeat: Uptime %lu s, Heap %lu B, RSSI %d dBm",
                     (unsigned long)hb->uptime_sec, (unsigned long)hb->free_heap, hb->rssi);
            break;
        }

        default:
            break;
    }
}

/**
 * @brief Task Core 0: Radio, Jaringan, dan Protokol Luar.
 */
void core0_network_task(void *pvParameters) {
    (void)pvParameters;
    ESP_LOGI(TAG, "Core 0 Task Berjalan (Wi-Fi, ESP-NOW, Modbus, Blynk).");

    while (1) {
        /* 1. Kelola Jendela Pairing Mode 1B (Timeout 3 Menit) */
        if (s_pairing_window_open) {
            s_pairing_timer_ms += 500;
            if (s_pairing_timer_ms >= (3 * 60 * 1000)) {
                s_pairing_window_open = false;
                s_pairing_timer_ms = 0;
                gateway_switch_mode(SYS_MODE_2A_RUNNING);
                ESP_LOGI(TAG, "Jendela Pairing ditutup otomatis.");
            }
        }

        /* 2. Cek Tombol Fisik Pairing (GPIO 4) */
        if (gateway_hal_is_pairing_pressed()) {
            s_pairing_window_open = true;
            s_pairing_timer_ms = 0;
            gateway_switch_mode(SYS_MODE_1B_NODE_SETUP);
        }

        /* 3. Cek Status Koneksi Blynk */
        if (s_current_mode == SYS_MODE_2A_RUNNING && !blynk_bridge_is_connected()) {
            gateway_switch_mode(SYS_MODE_2B_RUNNING_LOCAL);
        } else if (s_current_mode == SYS_MODE_2B_RUNNING_LOCAL && blynk_bridge_is_connected()) {
            gateway_switch_mode(SYS_MODE_2A_RUNNING);
        }

#ifdef ESP_PLATFORM
        vTaskDelay(pdMS_TO_TICKS(500));
#else
        break;
#endif
    }
}

/**
 * @brief Task Core 1: Logika, Global Rule Engine, dan Inferensi AI.
 */
void core1_logic_ai_task(void *pvParameters) {
    (void)pvParameters;
    ESP_LOGI(TAG, "Core 1 Task Berjalan (Rule Engine & PSRAM AI).");

    while (1) {
        /* Evaluasi Kesehatan Sistem secara berkala via AI Autoencoder */
        float recon_error = 0.0f;
        float current_heap_kb = 240.0f;
        float chip_temp = 38.0f;
        float task_lat = 8.0f;

        bool is_anomaly = ai_engine_detect_anomaly(current_heap_kb, chip_temp, task_lat, &recon_error);
        if (is_anomaly && s_current_mode != SYS_MODE_3_FAULT) {
            ESP_LOGW(TAG, "AI Autoencoder mendeteksi anomali Gateway (Error: %f)", recon_error);
            /* Catat ke log diagnostik LittleFS */
            storage_mgr_log_diagnostic("AI_ANOMALY", "Reconstruction error melebihi batas aman");
            gateway_switch_mode(SYS_MODE_3_FAULT);
        }

#ifdef ESP_PLATFORM
        vTaskDelay(pdMS_TO_TICKS(2000));
#else
        break;
#endif
    }
}

#ifdef ESP_PLATFORM
void app_main(void) {
    /* 1. Inisialisasi Storage NVS & LittleFS */
    storage_mgr_init();
    storage_mgr_increment_boot_count();

    /* 2. Inisialisasi Hardware */
    gateway_hal_init();

    /* 3. Alokasi AI di PSRAM */
    ai_engine_init();

    /* 4. Inisialisasi Jembatan Blynk */
    blynk_bridge_init("BLYNK_AUTH_PLACEHOLDER");

    /* 5. Tentukan Mode Awal */
    uint8_t last_mode = storage_mgr_get_boot_mode();
    gateway_switch_mode((system_mode_t)last_mode);

    /* 6. Luncurkan Task ke masing-masing Core */
    xTaskCreatePinnedToCore(core0_network_task, "core0_net", 4096, NULL, 5, NULL, 0);
    xTaskCreatePinnedToCore(core1_logic_ai_task, "core1_ai", 8192, NULL, 4, NULL, 1);
}
#endif
