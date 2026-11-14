/**
 * @file gateway_hal.h
 * @brief Abstraksi Periferal Hardware untuk ESP32-S3 Gateway.
 */

#ifndef GATEWAY_HAL_H
#define GATEWAY_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include "../../common/include/system_types.h"
#include "../../common/include/pinout_defs.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Inisialisasi seluruh periferal Gateway (LED RGB, Tombol, RS485 UART).
 */
void gateway_hal_init(void);

/**
 * @brief Mengatur status LED indikator sesuai state machine mode.
 */
void gateway_hal_set_led_mode(system_mode_t mode);

/**
 * @brief Membaca status tombol pairing (GPIO 4).
 * @return true jika tombol ditekan (Active LOW).
 */
bool gateway_hal_is_pairing_pressed(void);

/**
 * @brief Membaca status tombol reset (GPIO 5).
 * @return true jika tombol ditekan (Active LOW).
 */
bool gateway_hal_is_reset_pressed(void);

/**
 * @brief Mengirim frame Modbus RTU melalui RS485 transceiver.
 * @param data Buffer data biner
 * @param len Panjang data
 * @return int Jumlah byte terkirim, atau <0 jika error
 */
int gateway_hal_rs485_send(const uint8_t *data, size_t len);

/**
 * @brief Menerima frame Modbus RTU dari RS485 transceiver.
 * @param buf Buffer penampung
 * @param max_len Kapasitas buffer
 * @param timeout_ms Waktu tunggu maksimum dalam ms
 * @return int Jumlah byte diterima
 */
int gateway_hal_rs485_recv(uint8_t *buf, size_t max_len, uint32_t timeout_ms);

#ifdef __cplusplus
}
#endif

#endif /* GATEWAY_HAL_H */
