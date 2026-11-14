#include <stdio.h>
#include <assert.h>
#include "../gateway/main/ai_engine.h"

int main(void) {
    printf("[CHECK] Menjalankan uji AI Engine (Autoencoder & Q-Table)...\n");

    /* 1. Uji Inisialisasi Arena */
    assert(ai_engine_init() == true);
    printf("  [PASS] Alokasi Tensor Arena sukses.\n");

    /* 2. Uji Deteksi Kondisi Normal */
    float recon_error = 0.0f;
    /* Parameter sehat: heap 280KB, suhu 35C, latensi 5ms */
    bool is_anomaly = ai_engine_detect_anomaly(280.0f, 35.0f, 5.0f, &recon_error);
    printf("  [PASS] Metrik Normal -> Error: %f, Anomaly: %d\n", recon_error, is_anomaly);
    assert(is_anomaly == false);

    /* 3. Uji Deteksi Kondisi Anomali Kritis */
    /* Parameter kritis: heap 10KB (memory leak), suhu 82C (overheat), latensi 95ms */
    is_anomaly = ai_engine_detect_anomaly(10.0f, 82.0f, 95.0f, &recon_error);
    printf("  [PASS] Metrik Kritis -> Error: %f, Anomaly: %d\n", recon_error, is_anomaly);
    assert(is_anomaly == true);

    /* 4. Uji Q-Table Rekomendasi Solusi */
    fix_action_t fix_heap = ai_engine_get_best_fix(FAULT_STATE_HEAP_LOW);
    assert(fix_heap == FIX_ACTION_CLEAR_CACHE);
    printf("  [PASS] Rekomendasi Heap Kritis -> FIX_ACTION_CLEAR_CACHE\n");

    fix_action_t fix_heat = ai_engine_get_best_fix(FAULT_STATE_OVERHEAT);
    assert(fix_heat == FIX_ACTION_ENTER_MODE4);
    printf("  [PASS] Rekomendasi Overheat -> FIX_ACTION_ENTER_MODE4\n");

    /* 5. Uji Reinforcement Update */
    ai_engine_update_q_table(FAULT_STATE_HEAP_LOW, FIX_ACTION_RESTART_TASK, 2.0f);
    printf("  [PASS] Pembaruan bobot Q-learning (Bellman) berhasil dieksekusi.\n");

    printf("[SUCCESS] Seluruh uji AI Engine (Autoencoder & Q-Table) lulus 100%%.\n");
    return 0;
}
