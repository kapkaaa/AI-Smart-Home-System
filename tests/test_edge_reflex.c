#include <stdio.h>
#include <assert.h>
#include "../node/main/edge_filter.h"
#include "../node/main/local_reflex.h"
#include "../node/main/node_hal.h"

int main(void) {
    printf("[CHECK] Menjalankan uji Edge Filter & Autonomous Local Reflex...\n");

    /* 1. Uji Deadband Filter */
    edge_filter_t filter;
    edge_filter_init(&filter, 0.5f, 10000); /* Deadband 0.5 unit, interval 10 detik */

    /* Bacaan pertama wajib dilaporkan */
    assert(edge_filter_should_report(&filter, 220.0f, 1000) == true);
    printf("  [PASS] Bacaan awal berhasil dilaporkan.\n");

    /* Perubahan kecil (0.2V) dalam 1 detik -> Harus di-suppress (noise filter) */
    assert(edge_filter_should_report(&filter, 220.2f, 2000) == false);
    printf("  [PASS] Noise kecil berhasil disaring (suppressed).\n");

    /* Perubahan besar (1.5V) -> Harus dilaporkan */
    assert(edge_filter_should_report(&filter, 221.7f, 3000) == true);
    printf("  [PASS] Perubahan signifikan (> deadband) berhasil dilaporkan.\n");

    /* Perubahan kecil tapi waktu sudah lewat 10 detik -> Harus dilaporkan (Force interval) */
    assert(edge_filter_should_report(&filter, 221.8f, 15000) == true);
    printf("  [PASS] Force interval timeout berhasil dilaporkan.\n");

    /* 2. Uji Autonomous Local Reflex (Overcurrent Trip < 100ms) */
    pkt_provision_cfg_t cfg = {
        .reflex_enabled = 1,
        .reflex_sensor_driver_id = 101, /* PZEM-004T */
        .reflex_operator = OP_GREATER_THAN,
        .reflex_threshold = 16.0f,      /* Batas 16 Ampere */
        .reflex_target_pin = 4,         /* Relay Utama */
        .reflex_action_val = 0,         /* 0 = Matikan Beban */
        .failsafe_action = FAILSAFE_ALL_OFF
    };

    local_reflex_engine_t reflex;
    local_reflex_init(&reflex, &cfg);

    /* Set kondisi awal relay ON */
    node_hal_write_digital(4, 1);
    assert(node_hal_read_digital(4) == 1);

    /* Arus normal 10A -> Relay tetap ON */
    bool tripped = local_reflex_evaluate(&reflex, 101, 10.0f);
    assert(tripped == false);
    assert(node_hal_read_digital(4) == 1);
    printf("  [PASS] Arus normal (10A) tidak memicu refleks.\n");

    /* Arus beban lebih 18.5A -> Langsung trip matikan relay (val 0) */
    tripped = local_reflex_evaluate(&reflex, 101, 18.5f);
    assert(tripped == true);
    assert(node_hal_read_digital(4) == 0);
    assert(reflex.is_tripped == true);
    printf("  [PASS] Beban lebih (18.5A > 16.0A) sukses memutus relay seketika.\n");

    printf("[SUCCESS] Seluruh uji Edge Filter & Local Reflex lulus 100%%.\n");
    return 0;
}
