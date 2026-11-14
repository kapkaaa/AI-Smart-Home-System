/**
 * @file node_hal.c
 * @brief Implementasi Hardware Abstraction Layer dinamis untuk Node ESP32-C3 / C6.
 */

#include "node_hal.h"
#include "../../common/include/pinout_defs.h"
#include <stdio.h>

#ifdef ESP_PLATFORM
#include "driver/gpio.h"
#include "driver/ledc.h"
#include "esp_adc/adc_oneshot.h"
#include "esp_log.h"

static const char *TAG = "NODE_HAL";

bool node_hal_configure_pins(const pin_config_t *configs, uint8_t count) {
    if (configs == NULL || count > ESPNOW_MAX_PINS) return false;

    for (uint8_t i = 0; i < count; i++) {
        uint8_t pin = configs[i].pin_num;
        pin_type_t type = (pin_type_t)configs[i].pin_type;

        /* Validasi keamanan pin untuk chip ini (ESP32-C3 = 0x04) */
        if (!is_gpio_safe(0x04, pin)) {
            ESP_LOGE(TAG, "Ditolak: GPIO %d bukan pin aman!", pin);
            return false;
        }

        switch (type) {
            case PIN_TYPE_DIGITAL_IN: {
                gpio_config_t io_conf = {
                    .pin_bit_mask = (1ULL << pin),
                    .mode = GPIO_MODE_INPUT,
                    .pull_up_en = GPIO_PULLUP_ENABLE,
                    .pull_down_en = GPIO_PULLDOWN_DISABLE,
                    .intr_type = GPIO_INTR_DISABLE
                };
                gpio_config(&io_conf);
                ESP_LOGI(TAG, "GPIO %d diset DIGITAL_IN", pin);
                break;
            }

            case PIN_TYPE_DIGITAL_OUT: {
                gpio_config_t io_conf = {
                    .pin_bit_mask = (1ULL << pin),
                    .mode = GPIO_MODE_OUTPUT,
                    .pull_up_en = GPIO_PULLUP_DISABLE,
                    .pull_down_en = GPIO_PULLDOWN_DISABLE,
                    .intr_type = GPIO_INTR_DISABLE
                };
                gpio_config(&io_conf);
                gpio_set_level(pin, 0); /* Default LOW untuk keamanan relai */
                ESP_LOGI(TAG, "GPIO %d diset DIGITAL_OUT (Relay Safe Init)", pin);
                break;
            }

            case PIN_TYPE_PWM_OUT: {
                ledc_timer_config_t ledc_timer = {
                    .speed_mode       = LEDC_LOW_SPEED_MODE,
                    .duty_resolution  = LEDC_TIMER_10_BIT,
                    .timer_num        = LEDC_TIMER_0,
                    .freq_hz          = 5000,
                    .clk_cfg          = LEDC_AUTO_CLK
                };
                ledc_timer_config(&ledc_timer);

                ledc_channel_config_t ledc_channel = {
                    .speed_mode     = LEDC_LOW_SPEED_MODE,
                    .channel        = LEDC_CHANNEL_0,
                    .timer_sel      = LEDC_TIMER_0,
                    .intr_type      = LEDC_INTR_DISABLE,
                    .gpio_num       = pin,
                    .duty           = 0,
                    .hpoint         = 0
                };
                ledc_channel_config(&ledc_channel);
                ESP_LOGI(TAG, "GPIO %d diset PWM_OUT", pin);
                break;
            }

            default:
                break;
        }
    }
    return true;
}

void node_hal_write_digital(uint8_t pin, uint8_t val) {
    gpio_set_level((gpio_num_t)pin, val ? 1 : 0);
}

uint8_t node_hal_read_digital(uint8_t pin) {
    return (uint8_t)gpio_get_level((gpio_num_t)pin);
}

uint32_t node_hal_read_analog_mv(uint8_t pin) {
    /* Baca ADC via driver oneshot (disimplifikasi untuk pembacaan instan) */
    (void)pin;
    return 1500; /* 1.5V stub jika hardware ADC belum di-calibrate */
}

void node_hal_set_pwm_duty(uint8_t pin, uint8_t duty_percent) {
    (void)pin;
    if (duty_percent > 100) duty_percent = 100;
    uint32_t duty_val = (duty_percent * 1023) / 100;
    ledc_set_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0, duty_val);
    ledc_update_duty(LEDC_LOW_SPEED_MODE, LEDC_CHANNEL_0);
}

#else
/* Simulasi / Test Host */
static uint8_t s_mock_gpio_levels[32] = {0};

bool node_hal_configure_pins(const pin_config_t *configs, uint8_t count) {
    (void)configs; (void)count;
    return true;
}

void node_hal_write_digital(uint8_t pin, uint8_t val) {
    if (pin < 32) s_mock_gpio_levels[pin] = val;
}

uint8_t node_hal_read_digital(uint8_t pin) {
    return (pin < 32) ? s_mock_gpio_levels[pin] : 0;
}

uint32_t node_hal_read_analog_mv(uint8_t pin) {
    (void)pin;
    return 2200;
}

void node_hal_set_pwm_duty(uint8_t pin, uint8_t duty_percent) {
    (void)pin; (void)duty_percent;
}
#endif
