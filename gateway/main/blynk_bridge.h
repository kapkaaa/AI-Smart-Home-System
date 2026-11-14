/**
 * @file blynk_bridge.h
 * @brief Antarmuka Sinkronisasi Dua Arah Gateway ke Blynk Cloud.
 */

#ifndef BLYNK_BRIDGE_H
#define BLYNK_BRIDGE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inisialisasi jembatan Blynk Cloud.
 * @param auth_token Token autentikasi perangkat Blynk
 */
void blynk_bridge_init(const char *auth_token);

/**
 * @brief Memeriksa status konektivitas ke Blynk Cloud.
 */
bool blynk_bridge_is_connected(void);

/**
 * @brief Sinkronisasi nilai sensor/aktuator ke Virtual Pin Blynk.
 * @param vpin Virtual pin (V0 - V255)
 * @param value Nilai float yang dikirimkan
 */
void blynk_bridge_write_vpin(uint8_t vpin, float value);

/**
 * @brief Mengirim notifikasi darurat / laporan AI Mode 3 ke smartphone pengguna via Blynk.
 */
void blynk_bridge_send_notification(const char *title, const char *body);

#ifdef __cplusplus
}
#endif

#endif /* BLYNK_BRIDGE_H */
