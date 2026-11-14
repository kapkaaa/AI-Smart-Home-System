/**
 * @file external_protocols.h
 * @brief Driver Protokol Ekosistem Luar (Modbus RTU & Tuya Local Key).
 */

#ifndef EXTERNAL_PROTOCOLS_H
#define EXTERNAL_PROTOCOLS_H

#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief Menghitung checksum Modbus CRC-16 (Polinomial 0xA001).
 */
uint16_t modbus_crc16(const uint8_t *buffer, size_t len);

/**
 * @brief Membaca register Modbus RTU melalui RS485 transceiver.
 * @param slave_addr ID perangkat budak (1 - 247)
 * @param start_reg Alamat register awal
 * @param reg_count Jumlah register yang dibaca
 * @param out_regs Buffer penampung nilai register 16-bit
 * @return true jika respons valid dan lolos uji CRC
 */
bool modbus_master_read_holding(uint8_t slave_addr, uint16_t start_reg, uint16_t reg_count, uint16_t *out_regs);

/**
 * @brief Parsing pesan lokal perangkat Tuya (port 6668).
 * @param payload Payload JSON teks terdekripsi
 * @param dps_key Nomor Data Point (misal "1" untuk power, "101" untuk sensor)
 * @param out_val Pointer nilai keluaran
 * @return true jika dps_key ditemukan
 */
bool tuya_local_parse_dps(const char *payload, const char *dps_key, float *out_val);

#ifdef __cplusplus
}
#endif

#endif /* EXTERNAL_PROTOCOLS_H */
