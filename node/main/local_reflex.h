/**
 * @file local_reflex.h
 * @brief Mesin Refleks Otonom Lokal (<100ms) & Manajer Failsafe di Node.
 */

#ifndef LOCAL_REFLEX_H
#define LOCAL_REFLEX_H

#include <stdint.h>
#include <stdbool.h>
#include "../../common/include/system_types.h"
#include "../../common/include/protocol.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    bool              enabled;
    uint16_t          sensor_driver_id;
    reflex_op_t       op;
    float             threshold;
    uint8_t           target_pin;
    uint8_t           action_val;
    bool              is_tripped;
    failsafe_action_t failsafe;
} local_reflex_engine_t;

/**
 * @brief Inisialisasi engine reflex dari paket provisioning.
 */
void local_reflex_init(local_reflex_engine_t *eng, const pkt_provision_cfg_t *cfg);

/**
 * @brief Evaluasi pembacaan sensor secara lokal real-time.
 * @return true jika reflex terpicu (tripped)
 */
bool local_reflex_evaluate(local_reflex_engine_t *eng, uint16_t driver_id, float value);

/**
 * @brief Mengeksekusi aksi failsafe darurat jika Gateway offline > 90 detik.
 */
void local_reflex_execute_failsafe(local_reflex_engine_t *eng);

#ifdef __cplusplus
}
#endif

#endif /* LOCAL_REFLEX_H */
