/**
 * @file main.c
 * @brief Firmware Universal Node ESP32-C3 / ESP32-C6.
 * @details Menjalankan Zero-Config Discovery, Autonomous Local Reflex (<100ms),
 *          dan pelaporan telemetri hemat bandwidth via ESP-NOW.
 */

#include <stdio.h>
#include <string.h>
#include "../../common/include/system_types.h"
#include "../../common/include/protocol.h"
#include "../../common/include/protocol_validator.h"
#include "../../common/include/pinout_defs.h"
#include "node_hal.h"
#include "edge_filter.h"
#include "local_reflex.h"

#ifdef ESP_PLATFORM
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_now.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "nvs.h"

static const char *TAG = "UNIVERSAL_NODE";
#else
#define ESP_LOGI(tag, fmt, ...) printf("[%s] " fmt "\n", tag, ##__VA_ARGS__)
#define ESP_LOGW(tag, fmt, ...) printf("[%s][WARN] " fmt "\n", tag, ##__VA_ARGS__)
#define ESP_LOGE(tag, fmt, ...) printf("[%s][ERR] " fmt "\n", tag, ##__VA_ARGS__)
#endif

/* State Operasi Node */
static bool s_is_paired = false;
static uint16_t s_node_id = 0;
static uint8_t s_gateway_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
static local_reflex_engine_t s_reflex_engine;
static edge_filter_t s_filter;
static uint32_t s_last_gw_seen_ms = 0;

/**
 * @brief Callback saat paket ESP-NOW diterima oleh Node.
 */
static void on_espnow_data_recv(const uint8_t *mac_addr, const uint8_t *data, int data_len) {
    if (!protocol_validate_packet(data, (size_t)data_len)) {
        ESP_LOGW(TAG, "Paket tidak valid atau rusak dari MAC %02X:%02X:%02X...", mac_addr[0], mac_addr[1], mac_addr[2]);
        return;
    }

    const pkt_header_t *hdr = (const pkt_header_t *)data;

    switch (hdr->msg_type) {
        case PKT_TYPE_PROVISION_CFG: {
            const pkt_provision_cfg_t *cfg = (const pkt_provision_cfg_t *)data;
            ESP_LOGI(TAG, "Menerima konfigurasi Provisioning dari Gateway. Pin count: %d", cfg->pin_count);

            /* Simpan MAC penginduk Gateway */
            memcpy(s_gateway_mac, hdr->sender_mac, 6);

            /* Inisialisasi Hardware & Refleks */
            node_hal_configure_pins(cfg->pins, cfg->pin_count);
            local_reflex_init(&s_reflex_engine, cfg);
            edge_filter_init(&s_filter, (float)cfg->deadband_mv_or_munit, cfg->report_interval_ms);

            s_is_paired = true;
            s_last_gw_seen_ms = 1000; /* reset watchdog */
            break;
        }

        case PKT_TYPE_COMMAND: {
            const pkt_command_t *cmd = (const pkt_command_t *)data;
            ESP_LOGI(TAG, "Menerima Perintah: Pin %d -> Val %d (Tipe: %d)", cmd->target_pin, cmd->value, cmd->cmd_type);
            if (cmd->cmd_type == 0) {
                node_hal_write_digital(cmd->target_pin, (uint8_t)cmd->value);
            } else if (cmd->cmd_type == 1) {
                node_hal_set_pwm_duty(cmd->target_pin, (uint8_t)cmd->value);
            }
            s_last_gw_seen_ms = 1000;
            break;
        }

        case PKT_TYPE_TIME_SYNC: {
            const pkt_time_sync_t *ts = (const pkt_time_sync_t *)data;
            ESP_LOGI(TAG, "Sinkronisasi Waktu Gateway: Epoch %lu", (unsigned long)ts->epoch_seconds);
            s_last_gw_seen_ms = 1000;
            break;
        }

        default:
            break;
    }
}

/**
 * @brief Siklus Utama Node (FreeRTOS Task).
 */
void node_main_task(void *pvParameters) {
    (void)pvParameters;
    ESP_LOGI(TAG, "Firmware Universal Node Dimulai.");

    while (1) {
        if (!s_is_paired) {
            /* MODE BELUM TERDAFTAR: Kirim Beacon Discovery Request */
            ESP_LOGI(TAG, "Menyiarkan Discovery Beacon ke Gateway...");

            pkt_discovery_req_t req;
            memset(&req, 0, sizeof(req));
            req.header.magic = PROTOCOL_MAGIC;
            req.header.version = PROTOCOL_VERSION;
            req.header.msg_type = PKT_TYPE_DISCOVERY_REQ;
            req.header.seq_id = 0;
            req.chip_family = CHIP_ESP32_C3;
            req.flash_size_mb = 4;
            req.fw_major = 1;
            req.fw_minor = 0;
            req.safe_gpio_mask = (uint32_t)C3_SAFE_GPIO_MASK;

#ifdef ESP_PLATFORM
            uint8_t broadcast_mac[6] = {0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF};
            esp_now_send(broadcast_mac, (uint8_t *)&req, sizeof(req));
            vTaskDelay(pdMS_TO_TICKS(2000));
#else
            break; /* Keluar untuk mode test PC */
#endif
        } else {
            /* MODE TERDAFTAR: Baca Sensor, Jalankan Refleks Lokal, & Kirim Telemetri */

            /* Contoh pembacaan sensor pada pin analog / ADC */
            float current_reading = (float)node_hal_read_analog_mv(0) / 100.0f;

            /* 1. Evaluasi Refleks Otonom (<100ms tanpa cloud) */
            local_reflex_evaluate(&s_reflex_engine, 101, current_reading);

            /* 2. Cek apakah nilai perlu dilaporkan (Edge Noise Filtering) */
            uint32_t now_ms = 5000;
            if (edge_filter_should_report(&s_filter, current_reading, now_ms)) {
                pkt_telemetry_t telem;
                memset(&telem, 0, sizeof(telem));
                telem.header.magic = PROTOCOL_MAGIC;
                telem.header.version = PROTOCOL_VERSION;
                telem.header.msg_type = PKT_TYPE_TELEMETRY;
                telem.reading_count = 1;
                telem.readings[0].pin_num = 0;
                telem.readings[0].driver_id = 101;
                telem.readings[0].value = current_reading;

#ifdef ESP_PLATFORM
                esp_now_send(s_gateway_mac, (uint8_t *)&telem, sizeof(telem));
#endif
                ESP_LOGI(TAG, "Telemetri terkirim: %.2f", current_reading);
            }

#ifdef ESP_PLATFORM
            vTaskDelay(pdMS_TO_TICKS(100));
#else
            break;
#endif
        }
    }
}

#ifdef ESP_PLATFORM
void app_main(void) {
    nvs_flash_init();
    /* Wi-Fi & ESP-NOW Init */
    xTaskCreate(node_main_task, "node_main", 4096, NULL, 5, NULL);
}
#endif
