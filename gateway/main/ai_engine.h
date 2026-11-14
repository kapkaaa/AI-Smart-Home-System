/**
 * @file ai_engine.h
 * @brief Mesin Inferensi AI Berbasis PSRAM ESP32-S3.
 * @details Menjalankan Autoencoder Anomaly Detection dan Q-Table Guided Fix di PSRAM 8MB.
 */

#ifndef AI_ENGINE_H
#define AI_ENGINE_H

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C" {
#endif

/* Kategori Kesalahan untuk State Q-Table */
typedef enum {
    FAULT_STATE_HEAP_LOW     = 0,
    FAULT_STATE_TASK_HANG    = 1,
    FAULT_STATE_OVERHEAT     = 2,
    FAULT_STATE_RADIO_FAIL   = 3,
    FAULT_STATE_MAX          = 4
} fault_state_t;

/* Aksi Rekomendasi Solusi */
typedef enum {
    FIX_ACTION_RESTART_TASK  = 0,
    FIX_ACTION_CLEAR_CACHE   = 1,
    FIX_ACTION_RESET_RADIO   = 2,
    FIX_ACTION_ENTER_MODE4   = 3,
    FIX_ACTION_MAX           = 4
} fix_action_t;

/**
 * @brief Inisialisasi memori AI di PSRAM (8MB).
 * @return true jika alokasi PSRAM berhasil
 */
bool ai_engine_init(void);

/**
 * @brief Deteksi anomali metrik Gateway via Autoencoder reconstruction error.
 * @param free_heap_kb Sisa heap dalam Kilobyte
 * @param chip_temp_c Suhu internal ESP32-S3 dalam Celcius
 * @param task_latency_ms Latensi loop task dalam milidetik
 * @param out_recon_error Pointer float penampung skor error
 * @return true jika terdeteksi anomali (MSE > ambang batas)
 */
bool ai_engine_detect_anomaly(float free_heap_kb, float chip_temp_c, float task_latency_ms, float *out_recon_error);

/**
 * @brief Mengambil rekomendasi solusi 1-Click Fix terbaik berdasarkan Q-Table.
 */
fix_action_t ai_engine_get_best_fix(fault_state_t fault);

/**
 * @brief Memperbarui bobot Q-Table setelah aksi dicoba oleh teknisi (Reinforcement Learning).
 * @param fault State kesalahan
 * @param action Tindakan yang diambil
 * @param reward +1.0 jika berhasil pulih, -1.0 jika gagal
 */
void ai_engine_update_q_table(fault_state_t fault, fix_action_t action, float reward);

/**
 * @brief Menyimpan tabel bobot Q-Table ke file LittleFS.
 */
bool ai_engine_save_q_table(void);

#ifdef __cplusplus
}
#endif

#endif /* AI_ENGINE_H */
