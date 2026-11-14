/**
 * @file local_reflex.c
 * @brief Implementasi Refleks Otonom Tanpa Latensi Gateway.
 */

#include "local_reflex.h"
#include "node_hal.h"
#include <stdio.h>

void local_reflex_init(local_reflex_engine_t *eng, const pkt_provision_cfg_t *cfg) {
    if (!eng || !cfg) return;

    eng->enabled          = (cfg->reflex_enabled != 0);
    eng->sensor_driver_id = cfg->reflex_sensor_driver_id;
    eng->op               = (reflex_op_t)cfg->reflex_operator;
    eng->threshold        = cfg->reflex_threshold;
    eng->target_pin       = cfg->reflex_target_pin;
    eng->action_val       = cfg->reflex_action_val;
    eng->is_tripped       = false;
    eng->failsafe         = (failsafe_action_t)cfg->failsafe_action;
}

bool local_reflex_evaluate(local_reflex_engine_t *eng, uint16_t driver_id, float value) {
    if (!eng || !eng->enabled) return false;
    if (eng->sensor_driver_id != driver_id) return false;

    bool condition_met = false;

    switch (eng->op) {
        case OP_GREATER_THAN:
            condition_met = (value > eng->threshold);
            break;
        case OP_LESS_THAN:
            condition_met = (value < eng->threshold);
            break;
        case OP_EQUAL:
            condition_met = (value == eng->threshold);
            break;
        default:
            break;
    }

    if (condition_met) {
        /* Eksekusi instan di pin target (misal putus relay beban berlebih) */
        node_hal_write_digital(eng->target_pin, eng->action_val);
        eng->is_tripped = true;
        return true;
    }

    return false;
}

void local_reflex_execute_failsafe(local_reflex_engine_t *eng) {
    if (!eng) return;

    switch (eng->failsafe) {
        case FAILSAFE_ALL_OFF:
            /* Putus aktuator demi keamanan kebakaran/overheat */
            node_hal_write_digital(eng->target_pin, 0);
            break;

        case FAILSAFE_HOLD_LAST:
            /* Biarkan kondisi pin tidak diubah */
            break;

        case FAILSAFE_MANUAL_BYPASS:
            /* Mode saklar manual / bypass wall switch */
            break;

        default:
            break;
    }
}
