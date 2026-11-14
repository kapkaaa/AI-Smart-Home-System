#ifndef STORAGE_PATHS_H
#define STORAGE_PATHS_H

#define LITTLEFS_BASE_PATH          "/spiffs"

/* Konfigurasi & Driver */
#define PATH_DRIVERS_CATALOG        LITTLEFS_BASE_PATH "/drivers_catalog.json"
#define PATH_NODES_PROFILE          LITTLEFS_BASE_PATH "/nodes_profile.json"
#define PATH_RULES_TABLE            LITTLEFS_BASE_PATH "/rules_table.json"

/* Log & Diagnosa */
#define PATH_DIAGNOSTIC_LOG         LITTLEFS_BASE_PATH "/diagnostic.log"
#define PATH_CRASH_DUMP             LITTLEFS_BASE_PATH "/crash.dump"

/* Model AI Checkpoints (disimpan berkala 6-12 jam) */
#define PATH_AI_AUTOENCODER_BIN     LITTLEFS_BASE_PATH "/ai_ae.bin"
#define PATH_AI_LSTM_WEIGHTS_BIN    LITTLEFS_BASE_PATH "/ai_lstm.bin"
#define PATH_AI_QTABLE_BIN          LITTLEFS_BASE_PATH "/ai_qtable.bin"

#endif /* STORAGE_PATHS_H */
