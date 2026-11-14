/**
 * @file external_protocols.c
 * @brief Implementasi Modbus RTU & Tuya Local Gateway.
 */

#include "external_protocols.h"
#include "gateway_hal.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

uint16_t modbus_crc16(const uint8_t *buffer, size_t len) {
    uint16_t crc = 0xFFFF;
    for (size_t pos = 0; pos < len; pos++) {
        crc ^= (uint16_t)buffer[pos];
        for (int i = 8; i != 0; i--) {
            if ((crc & 0x0001) != 0) {
                crc >>= 1;
                crc ^= 0xA001;
            } else {
                crc >>= 1;
            }
        }
    }
    return crc;
}

bool modbus_master_read_holding(uint8_t slave_addr, uint16_t start_reg, uint16_t reg_count, uint16_t *out_regs) {
    if (!out_regs || reg_count == 0 || reg_count > 32) return false;

    uint8_t req[8];
    req[0] = slave_addr;
    req[1] = 0x03; /* Function 03: Read Holding Registers */
    req[2] = (uint8_t)(start_reg >> 8);
    req[3] = (uint8_t)(start_reg & 0xFF);
    req[4] = (uint8_t)(reg_count >> 8);
    req[5] = (uint8_t)(reg_count & 0xFF);

    uint16_t crc = modbus_crc16(req, 6);
    req[6] = (uint8_t)(crc & 0xFF);        /* CRC Low byte first */
    req[7] = (uint8_t)(crc >> 8);          /* CRC High byte */

    /* Kirim request ke transceiver RS485 */
    gateway_hal_rs485_send(req, sizeof(req));

    /* Tangkap respons: 1 (addr) + 1 (fn) + 1 (byte_count) + 2*reg_count + 2 (crc) */
    uint8_t resp_buf[128];
    size_t expected_len = 5 + (reg_count * 2);
    int rx_len = gateway_hal_rs485_recv(resp_buf, sizeof(resp_buf), 200);

    if (rx_len < (int)expected_len) return false;
    if (resp_buf[0] != slave_addr || resp_buf[1] != 0x03) return false;

    /* Verifikasi CRC respons */
    uint16_t rx_crc = (uint16_t)(resp_buf[rx_len - 2] | (resp_buf[rx_len - 1] << 8));
    if (modbus_crc16(resp_buf, rx_len - 2) != rx_crc) return false;

    /* Salin register output */
    for (uint16_t i = 0; i < reg_count; i++) {
        out_regs[i] = (uint16_t)((resp_buf[3 + (i * 2)] << 8) | resp_buf[4 + (i * 2)]);
    }

    return true;
}

bool tuya_local_parse_dps(const char *payload, const char *dps_key, float *out_val) {
    if (!payload || !dps_key || !out_val) return false;

    /* Cari key dalam format "key": value */
    char pattern[32];
    snprintf(pattern, sizeof(pattern), "\"%s\":", dps_key);
    char *match = strstr(payload, pattern);
    if (!match) return false;

    match += strlen(pattern);
    while (*match == ' ') match++;

    if (strncmp(match, "true", 4) == 0) {
        *out_val = 1.0f;
        return true;
    } else if (strncmp(match, "false", 5) == 0) {
        *out_val = 0.0f;
        return true;
    } else {
        *out_val = (float)atof(match);
        return true;
    }
}
