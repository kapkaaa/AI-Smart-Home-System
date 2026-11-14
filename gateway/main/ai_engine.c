/**
 * @file ai_engine.c
 * @brief Implementasi Autoencoder Anomaly Detector & Q-Table Guided Fix.
 */

#include "ai_engine.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#ifdef ESP_PLATFORM
#include "esp_heap_caps.h"
#include "esp_log.h"
static const char *TAG = "AI_ENGINE";
#endif

#define PSRAM_ARENA_SIZE    (256 * 1024) /* 256 KB alokasi PSRAM awal */
#define ANOMALY_MSE_LIMIT   0.05f        /* Ambang batas reconstruction error */
#define Q_ALPHA             0.2f         /* Learning rate */

static uint8_t *s_psram_arena = NULL;

/* 
 * Baseline Subspace Autoencoder:
 * Dilatih pada data operasi normal Gateway ESP32-S3:
 * mu: [Heap normal ~280KB (0.93), Suhu normal ~35C (0.25), Latensi normal ~5ms (0.05)]
 * v_subspace: vektor korelasi variasi beban normal
 */
static const float s_mu[3] = { 0.93f, 0.25f, 0.05f };
static const float s_v[3]  = { 0.7071f, -0.7071f, 0.0f };

/* Q-Table Matriks (4 States x 4 Actions) */
static float s_q_table[FAULT_STATE_MAX][FIX_ACTION_MAX] = {
    /* HEAP_LOW   */ { 0.2f, 0.9f, 0.1f, 0.5f }, /* Prior: Clear Cache terbaik */
    /* TASK_HANG  */ { 0.9f, 0.1f, 0.3f, 0.6f }, /* Prior: Restart Task terbaik */
    /* OVERHEAT   */ { 0.1f, 0.1f, 0.1f, 0.9f }, /* Prior: Mode 4 Safe Boot terbaik */
    /* RADIO_FAIL */ { 0.2f, 0.1f, 0.9f, 0.4f }  /* Prior: Reset Radio terbaik */
};

bool ai_engine_init(void) {
#ifdef ESP_PLATFORM
    s_psram_arena = (uint8_t *)heap_caps_malloc(PSRAM_ARENA_SIZE, MALLOC_CAP_SPIRAM);
    if (!s_psram_arena) {
        ESP_LOGE(TAG, "Gagal mengalokasikan 256KB di PSRAM!");
        return false;
    }
    ESP_LOGI(TAG, "Tensor Arena AI 256KB berhasil dialokasikan di PSRAM.");
#else
    s_psram_arena = (uint8_t *)malloc(PSRAM_ARENA_SIZE);
#endif
    return (s_psram_arena != NULL);
}

bool ai_engine_detect_anomaly(float free_heap_kb, float chip_temp_c, float task_latency_ms, float *out_recon_error) {
    /* Normalisasi Input ke rentang [0.0, 1.0] */
    float x[3];
    x[0] = free_heap_kb / 300.0f;
    if (x[0] > 1.0f) { x[0] = 1.0f; }
    if (x[0] < 0.0f) { x[0] = 0.0f; }

    x[1] = (chip_temp_c - 20.0f) / 60.0f;
    if (x[1] > 1.0f) { x[1] = 1.0f; }
    if (x[1] < 0.0f) { x[1] = 0.0f; }

    x[2] = task_latency_ms / 100.0f;
    if (x[2] > 1.0f) { x[2] = 1.0f; }
    if (x[2] < 0.0f) { x[2] = 0.0f; }

    /* 1. Proyeksi Deviasi ke Subspace Normal (Encoder) */
    float diff[3];
    float dot = 0.0f;
    for (int i = 0; i < 3; i++) {
        diff[i] = x[i] - s_mu[i];
        dot += diff[i] * s_v[i];
    }

    /* 2. Rekonstruksi Vektor dari Subspace (Decoder) */
    float mse = 0.0f;
    for (int i = 0; i < 3; i++) {
        float x_hat = s_mu[i] + dot * s_v[i];
        float err = x[i] - x_hat;
        mse += err * err;
    }
    mse /= 3.0f;

    if (out_recon_error) {
        *out_recon_error = mse;
    }

    return (mse > ANOMALY_MSE_LIMIT);
}

fix_action_t ai_engine_get_best_fix(fault_state_t fault) {
    if (fault >= FAULT_STATE_MAX) return FIX_ACTION_ENTER_MODE4;

    fix_action_t best_act = FIX_ACTION_RESTART_TASK;
    float max_q = -9999.0f;

    for (int a = 0; a < FIX_ACTION_MAX; a++) {
        if (s_q_table[fault][a] > max_q) {
            max_q = s_q_table[fault][a];
            best_act = (fix_action_t)a;
        }
    }
    return best_act;
}

void ai_engine_update_q_table(fault_state_t fault, fix_action_t action, float reward) {
    if (fault >= FAULT_STATE_MAX || action >= FIX_ACTION_MAX) return;

    /* Update Bellman Equation: Q(s,a) = Q(s,a) + alpha * (reward - Q(s,a)) */
    float old_q = s_q_table[fault][action];
    s_q_table[fault][action] = old_q + Q_ALPHA * (reward - old_q);
}

bool ai_engine_save_q_table(void) {
    /* Simpan matriks Q-Table ke LittleFS (binary dump) */
    FILE *f = fopen("/spiffs/ai_qtable.bin", "wb");
    if (!f) return false;
    fwrite(s_q_table, sizeof(float), FAULT_STATE_MAX * FIX_ACTION_MAX, f);
    fclose(f);
    return true;
}
