/**
 * @file gateway_hal.c
 * @brief Implementasi Hardware Abstraction Layer (HAL) untuk ESP32-S3 Gateway.
 */

#include "gateway_hal.h"
#include <stdio.h>
#include <string.h>

#ifdef ESP_PLATFORM
#include "driver/gpio.h"
#include "driver/uart.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

static const char *TAG = "GW_HAL";
#define RS485_UART_PORT   UART_NUM_1
#define RS485_BUF_SIZE    256

void gateway_hal_init(void) {
    ESP_LOGI(TAG, "Inisialisasi GPIO & RS485 Gateway...");

    /* 1. Inisialisasi LED RGB */
    gpio_config_t led_conf = {
        .pin_bit_mask = (1ULL << GW_PIN_LED_RED) | (1ULL << GW_PIN_LED_GREEN) | (1ULL << GW_PIN_LED_BLUE),
        .mode = GPIO_MODE_OUTPUT,
        .pull_up_en = GPIO_PULLUP_DISABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&led_conf);

    /* 2. Inisialisasi Tombol (Active LOW dengan internal pull-up) */
    gpio_config_t btn_conf = {
        .pin_bit_mask = (1ULL << GW_PIN_BTN_PAIRING) | (1ULL << GW_PIN_BTN_RESET),
        .mode = GPIO_MODE_INPUT,
        .pull_up_en = GPIO_PULLUP_ENABLE,
        .pull_down_en = GPIO_PULLDOWN_DISABLE,
        .intr_type = GPIO_INTR_DISABLE
    };
    gpio_config(&btn_conf);

    /* 3. Inisialisasi UART1 untuk RS485 Half-Duplex */
    uart_config_t uart_conf = {
        .baud_rate = 9600,
        .data_bits = UART_DATA_8_BITS,
        .parity    = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT
    };
    uart_param_config(RS485_UART_PORT, &uart_conf);
    uart_set_pin(RS485_UART_PORT, GW_PIN_RS485_TX, GW_PIN_RS485_RX, GW_PIN_RS485_RTS, UART_PIN_NO_CHANGE);
    uart_driver_install(RS485_UART_PORT, RS485_BUF_SIZE, RS485_BUF_SIZE, 0, NULL, 0);
    uart_set_mode(RS485_UART_PORT, UART_MODE_RS485_HALF_DUPLEX);

    gateway_hal_set_led_mode(SYS_MODE_1A_GATEWAY_SETUP);
}

void gateway_hal_set_led_mode(system_mode_t mode) {
    switch (mode) {
        case SYS_MODE_1A_GATEWAY_SETUP:
        case SYS_MODE_1B_NODE_SETUP:
            /* Biru Aktif */
            gpio_set_level(GW_PIN_LED_RED, 0);
            gpio_set_level(GW_PIN_LED_GREEN, 0);
            gpio_set_level(GW_PIN_LED_BLUE, 1);
            break;

        case SYS_MODE_2A_RUNNING:
            /* Hijau Aktif */
            gpio_set_level(GW_PIN_LED_RED, 0);
            gpio_set_level(GW_PIN_LED_GREEN, 1);
            gpio_set_level(GW_PIN_LED_BLUE, 0);
            break;

        case SYS_MODE_2B_RUNNING_LOCAL:
            /* Kuning (Merah + Hijau) */
            gpio_set_level(GW_PIN_LED_RED, 1);
            gpio_set_level(GW_PIN_LED_GREEN, 1);
            gpio_set_level(GW_PIN_LED_BLUE, 0);
            break;

        case SYS_MODE_3_FAULT:
        case SYS_MODE_4_RECOVERY:
        default:
            /* Merah Aktif */
            gpio_set_level(GW_PIN_LED_RED, 1);
            gpio_set_level(GW_PIN_LED_GREEN, 0);
            gpio_set_level(GW_PIN_LED_BLUE, 0);
            break;
    }
}

bool gateway_hal_is_pairing_pressed(void) {
    return (gpio_get_level(GW_PIN_BTN_PAIRING) == 0);
}

bool gateway_hal_is_reset_pressed(void) {
    return (gpio_get_level(GW_PIN_BTN_RESET) == 0);
}

int gateway_hal_rs485_send(const uint8_t *data, size_t len) {
    return uart_write_bytes(RS485_UART_PORT, (const char *)data, len);
}

int gateway_hal_rs485_recv(uint8_t *buf, size_t max_len, uint32_t timeout_ms) {
    return uart_read_bytes(RS485_UART_PORT, buf, max_len, pdMS_TO_TICKS(timeout_ms));
}

#else
/* Simulasi / Test Mode Non-ESP */
static system_mode_t s_mock_mode = SYS_MODE_1A_GATEWAY_SETUP;

void gateway_hal_init(void) {
    printf("[MOCK_HAL] Gateway HAL Initialized.\n");
}

void gateway_hal_set_led_mode(system_mode_t mode) {
    s_mock_mode = mode;
    printf("[MOCK_HAL] LED Indicator diubah ke mode: 0x%02X\n", mode);
}

bool gateway_hal_is_pairing_pressed(void) { return false; }
bool gateway_hal_is_reset_pressed(void) { return false; }

int gateway_hal_rs485_send(const uint8_t *data, size_t len) {
    (void)data; return (int)len;
}

int gateway_hal_rs485_recv(uint8_t *buf, size_t max_len, uint32_t timeout_ms) {
    (void)buf; (void)max_len; (void)timeout_ms; return 0;
}
#endif
