#ifndef SYSTEM_TYPES_H
#define SYSTEM_TYPES_H

#include <stdint.h>
#include <stdbool.h>

/* Gateway Operation Modes */
typedef enum {
    SYS_MODE_1A_GATEWAY_SETUP = 0x01,
    SYS_MODE_1B_NODE_SETUP    = 0x02,
    SYS_MODE_2A_RUNNING       = 0x10,
    SYS_MODE_2B_RUNNING_LOCAL = 0x11,
    SYS_MODE_3_FAULT          = 0x20,
    SYS_MODE_4_RECOVERY       = 0x30
} system_mode_t;

/* Chip Families */
typedef enum {
    CHIP_UNKNOWN    = 0x00,
    CHIP_ESP32      = 0x01,
    CHIP_ESP32_S2   = 0x02,
    CHIP_ESP32_S3   = 0x03,
    CHIP_ESP32_C3   = 0x04,
    CHIP_ESP32_C6   = 0x05,
    CHIP_ESP8266    = 0x06,
    CHIP_STM32      = 0x10,
    CHIP_RP2040     = 0x11,
    CHIP_TUYA       = 0x20
} chip_family_t;

/* Node Roles */
typedef enum {
    ROLE_UNPROVISIONED = 0x00,
    ROLE_SENSOR        = 0x01,
    ROLE_ACTUATOR      = 0x02,
    ROLE_HYBRID        = 0x03
} node_role_t;

/* Failsafe Actions (Koneksi Gateway Hilang) */
typedef enum {
    FAILSAFE_ALL_OFF       = 0x00,
    FAILSAFE_HOLD_LAST     = 0x01,
    FAILSAFE_MANUAL_BYPASS = 0x02
} failsafe_action_t;

/* Reflex Condition Operator */
typedef enum {
    OP_GREATER_THAN = 0x01,
    OP_LESS_THAN    = 0x02,
    OP_EQUAL        = 0x03
} reflex_op_t;

/* Pin IO Types */
typedef enum {
    PIN_TYPE_UNUSED    = 0x00,
    PIN_TYPE_DIGITAL_IN  = 0x01,
    PIN_TYPE_DIGITAL_OUT = 0x02,
    PIN_TYPE_ANALOG_IN   = 0x03,
    PIN_TYPE_PWM_OUT     = 0x04,
    PIN_TYPE_I2C_SDA     = 0x05,
    PIN_TYPE_I2C_SCL     = 0x06,
    PIN_TYPE_UART_TX     = 0x07,
    PIN_TYPE_UART_RX     = 0x08
} pin_type_t;

#endif /* SYSTEM_TYPES_H */
