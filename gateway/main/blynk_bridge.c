/**
 * @file blynk_bridge.c
 * @brief Implementasi Jembatan Blynk Cloud (Core 0 Network).
 */

#include "blynk_bridge.h"
#include <stdio.h>
#include <string.h>

#ifdef ESP_PLATFORM
#include "esp_log.h"
static const char *TAG = "BLYNK_BRIDGE";
#endif

static bool s_blynk_online = false;
static char s_auth_token[34] = {0};

void blynk_bridge_init(const char *auth_token) {
    if (auth_token) {
        strncpy(s_auth_token, auth_token, sizeof(s_auth_token) - 1);
    }
#ifdef ESP_PLATFORM
    ESP_LOGI(TAG, "Blynk Bridge initialized with token: %.4s****", s_auth_token);
#else
    printf("[BLYNK_BRIDGE] Inisialisasi token: %.4s****\n", s_auth_token);
#endif
    s_blynk_online = true;
}

bool blynk_bridge_is_connected(void) {
    return s_blynk_online;
}

void blynk_bridge_write_vpin(uint8_t vpin, float value) {
    if (!s_blynk_online) return;
#ifdef ESP_PLATFORM
    ESP_LOGI(TAG, "Sync to Blynk: V%d = %.2f", vpin, value);
#else
    printf("[BLYNK] V%d = %.2f\n", vpin, value);
#endif
}

void blynk_bridge_send_notification(const char *title, const char *body) {
    if (!s_blynk_online) return;
#ifdef ESP_PLATFORM
    ESP_LOGW(TAG, "Blynk Notification Pushed: [%s] %s", title, body);
#else
    printf("[BLYNK_NOTIF] [%s] %s\n", title, body);
#endif
}
