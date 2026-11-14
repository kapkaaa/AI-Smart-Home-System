/**
 * @file pinout_defs.h
 * @brief Pemetaan pinout aman (Safe Pinout) ESP32-S3 Gateway dan ESP32-C3/C6 Node.
 * @details Menghindari strapping pins dan pin Flash/PSRAM bawaan pabrik.
 */

#ifndef PINOUT_DEFS_H
#define PINOUT_DEFS_H

#include <stdint.h>

/* =========================================================================
 * 1. PINOUT ESP32-S3 GATEWAY (16MB Flash + 8MB Octal PSRAM)
 *    PERINGATAN: GPIO 26-37 digunakan secara eksklusif oleh Octal Flash/PSRAM!
 * ========================================================================= */

/* Tombol Fisik */
#define GW_PIN_BTN_PAIRING       4   /* Pull-Up: Ditekan 2 detik untuk Mode 1B */
#define GW_PIN_BTN_RESET         5   /* Pull-Up: Ditekan 5 detik untuk Factory Reset */

/* Indikator Status RGB */
#define GW_PIN_LED_RED           6   /* PWM / Digital: Indikator Fault (Mode 3/4) */
#define GW_PIN_LED_GREEN         7   /* PWM / Digital: Indikator Running (Mode 2A) */
#define GW_PIN_LED_BLUE          15  /* PWM / Digital: Indikator Setup (Mode 1) */

/* Jalur RS485 Modbus RTU (UART 1) */
#define GW_PIN_RS485_TX          17
#define GW_PIN_RS485_RX          18
#define GW_PIN_RS485_RTS         16  /* Direction Flow Control (DE/RE) */

/* Jalur I2C Bus Gateway */
#define GW_PIN_I2C_SDA           8
#define GW_PIN_I2C_SCL           9

/* =========================================================================
 * 2. PINOUT ESP32-C3 UNIVERSAL NODE (4MB Flash)
 *    PERINGATAN: GPIO 2, 8, 9 adalah strapping pins. GPIO 12-17 dipakai Flash.
 * ========================================================================= */

/* Bitmask GPIO yang aman dialokasikan secara dinamis oleh Gateway */
#define C3_SAFE_GPIO_MASK   (\
    (1ULL << 0)  | /* ADC1_CH0 / Digital IO */ \
    (1ULL << 1)  | /* ADC1_CH1 / Digital IO */ \
    (1ULL << 3)  | /* ADC1_CH3 / Digital IO */ \
    (1ULL << 4)  | /* I2C SDA / Digital IO   */ \
    (1ULL << 5)  | /* I2C SCL / Digital IO   */ \
    (1ULL << 6)  | /* Relay / PWM            */ \
    (1ULL << 7)  | /* Relay / PWM            */ \
    (1ULL << 10) | /* Digital In / Wall Switch*/ \
    (1ULL << 20) | /* UART RX (PZEM/Radar)   */ \
    (1ULL << 21)   /* UART TX (PZEM/Radar)   */ \
)

/* =========================================================================
 * 3. PINOUT ESP32-C6 UNIVERSAL NODE (4MB Flash, Wi-Fi 6 + Zigbee/Thread)
 * ========================================================================= */
#define C6_SAFE_GPIO_MASK   (\
    (1ULL << 0)  | (1ULL << 1)  | (1ULL << 2)  | (1ULL << 3)  | \
    (1ULL << 4)  | (1ULL << 5)  | (1ULL << 6)  | (1ULL << 7)  | \
    (1ULL << 18) | (1ULL << 19) | (1ULL << 20) | (1ULL << 21) | \
    (1ULL << 22) | (1ULL << 23)   \
)

/**
 * @brief Memeriksa apakah suatu GPIO aman digunakan pada chip tertentu.
 */
static inline bool is_gpio_safe(uint8_t chip_family, uint8_t gpio_num) {
    if (chip_family == 0x04) { /* CHIP_ESP32_C3 */
        return (gpio_num < 32) && ((C3_SAFE_GPIO_MASK & (1ULL << gpio_num)) != 0);
    } else if (chip_family == 0x05) { /* CHIP_ESP32_C6 */
        return (gpio_num < 32) && ((C6_SAFE_GPIO_MASK & (1ULL << gpio_num)) != 0);
    } else if (chip_family == 0x03) { /* CHIP_ESP32_S3 */
        /* Tolak pin Octal Flash & Strapping */
        if (gpio_num == 0 || gpio_num == 3 || gpio_num == 45 || gpio_num == 46) return false;
        if (gpio_num >= 26 && gpio_num <= 37) return false;
        return (gpio_num <= 48);
    }
    return false;
}

#endif /* PINOUT_DEFS_H */
