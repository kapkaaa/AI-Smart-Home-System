/**
 * @file node_hal.h
 * @brief Universal Hardware Abstraction Layer untuk Node ESP32-C3 / ESP32-C6.
 * @details Mengalokasikan mode GPIO secara dinamis berdasarkan instruksi Provisioning Gateway.
 */

#ifndef NODE_HAL_H
#define NODE_HAL_H

#include <stdint.h>
#include <stdbool.h>
#include "../../common/include/system_types.h"
#include "../../common/include/protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Mengkonfigurasi seluruh pin node sesuai daftar konfigurasi dari Gateway.
 * @param configs Array konfigurasi pin
 * @param count Jumlah pin yang dikonfigurasi (maksimal ESPNOW_MAX_PINS)
 * @return true jika seluruh alokasi pin valid dan berhasil
 */
bool node_hal_configure_pins(const pin_config_t *configs, uint8_t count);

/**
 * @brief Mengubah output digital (misal Relay / SSR).
 */
void node_hal_write_digital(uint8_t pin, uint8_t val);

/**
 * @brief Membaca input digital (misal Reed Switch / PIR / Push Button).
 */
uint8_t node_hal_read_digital(uint8_t pin);

/**
 * @brief Membaca tegangan analog dalam milivolt (ADC).
 */
uint32_t node_hal_read_analog_mv(uint8_t pin);

/**
 * @brief Mengatur duty cycle PWM (0 - 100%).
 */
void node_hal_set_pwm_duty(uint8_t pin, uint8_t duty_percent);

#ifdef __cplusplus
}
#endif

#endif /* NODE_HAL_H */
