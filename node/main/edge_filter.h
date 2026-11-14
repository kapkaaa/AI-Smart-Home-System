/**
 * @file edge_filter.h
 * @brief Algoritma Penyaringan Noise Sensor di Sisi Edge (Node).
 * @details Mencegah banjir paket ESP-NOW jika perubahan nilai sensor tidak signifikan.
 */

#ifndef EDGE_FILTER_H
#define EDGE_FILTER_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    float last_sent_val;
    float deadband_threshold;
    uint32_t last_sent_time_ms;
    uint32_t force_interval_ms;
} edge_filter_t;

/**
 * @brief Inisialisasi filter deadband.
 */
void edge_filter_init(edge_filter_t *f, float threshold, uint32_t max_interval_ms);

/**
 * @brief Memeriksa apakah nilai baru layak dilaporkan ke Gateway.
 * @param f Pointer objek filter
 * @param current_val Nilai sensor pembacaan sekarang
 * @param now_ms Waktu sistem sekarang (milidetik)
 * @return true jika nilai melewati threshold deadband ATAU sudah mencapai batas waktu interval
 */
bool edge_filter_should_report(edge_filter_t *f, float current_val, uint32_t now_ms);

#ifdef __cplusplus
}
#endif

#endif /* EDGE_FILTER_H */
