/**
 * @file edge_filter.c
 * @brief Implementasi Deadband & Reporting Throttle Filter.
 */

#include "edge_filter.h"
#include <math.h>

void edge_filter_init(edge_filter_t *f, float threshold, uint32_t max_interval_ms) {
    if (!f) return;
    f->last_sent_val = -999999.0f; /* Force kirim pertama */
    f->deadband_threshold = threshold;
    f->last_sent_time_ms = 0;
    f->force_interval_ms = max_interval_ms;
}

bool edge_filter_should_report(edge_filter_t *f, float current_val, uint32_t now_ms) {
    if (!f) return false;

    /* Cek interval batas waktu paksa (heartbeat periodic) */
    if ((now_ms - f->last_sent_time_ms) >= f->force_interval_ms) {
        f->last_sent_val = current_val;
        f->last_sent_time_ms = now_ms;
        return true;
    }

    /* Cek selisih nilai absolut melewati batas deadband */
    float diff = fabsf(current_val - f->last_sent_val);
    if (diff >= f->deadband_threshold) {
        f->last_sent_val = current_val;
        f->last_sent_time_ms = now_ms;
        return true;
    }

    return false;
}
