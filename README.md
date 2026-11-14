# Modular AI Smart Home System (ESP-IDF / C99)

Sistem Smart Home modular terpusat berbasis **Local-First Architecture** dengan kecerdasan buatan hibrida (Edge Intelligence + TinyML/Deep Learning) untuk mikrokontroler Espressif ESP32 series.

---

## 1. Arsitektur Sistem

```
+---------------------------------------------------------------------------------+
|                               BLYNK CLOUD & WEB APP                             |
+---------------------------------------------------------------------------------+
                                        ^
                                        | (Wi-Fi STA / HTTPS / WebSocket)
+---------------------------------------v-----------------------------------------+
|                          ESP32-S3 GATEWAY (16MB + 8MB PSRAM)                   |
|                                                                                 |
|   Core 0 (Network & Protocol):               Core 1 (Logic & AI):               |
|   - Wi-Fi STA / SoftAP (Captive Portal)      - Global Rule Engine               |
|   - ESP-NOW Radio Coexistence                - Local WebServer & WebSocket      |
|   - Modbus RTU Master (RS485 UART1)          - AI Autoencoder Anomaly Detector  |
|   - Tuya Local Client (Port 6668)            - Q-Table 1-Click Guided Recovery  |
|   - Blynk Cloud Bi-directional Sync          - PSRAM Tensor Arena (8MB)         |
+---------------------------------------------------------------------------------+
           | (ESP-NOW Raw Binary Frames <= 250B)             | (RS485 Modbus RTU)
           v                                                 v
+-----------------------------+               +-----------------------------------+
|  ESP32-C3 / C6 EDGE NODE   |               | STM32 / PZEM / Industri Sub-Node  |
|                             |               | - PZEM-004T v3.0 (AC Power)       |
| - Universal Firmware        |               | - PLC / Inverter / Din-Rail       |
| - Dynamic Pin Provisioning  |               +-----------------------------------+
| - Autonomous Reflex (<100ms)|
| - Deadband Edge Filter      |
| - Smart Failsafe Timer      |
+-----------------------------+
```

---

## 2. Struktur Direktori Proyek

```
modular-smart-home/
├── CMakeLists.txt              # Build system root
├── README.md                   # Dokumentasi arsitektur & panduan
├── common/                     # Shared single source of truth
│   └── include/
│       ├── system_types.h      # Enum mode sistem, chip, role, operator
│       ├── protocol.h          # C Struct biner packed (ESP-NOW)
│       ├── protocol_validator.h# Parser & guard validation (magic byte & MTU)
│       └── pinout_defs.h       # Pemetaan GPIO aman (anti-strapping pin)
├── gateway/                    # Firmware ESP32-S3 Gateway
│   ├── partitions.csv          # 16MB Flash: NVS, Dual OTA (4.5MB), LittleFS (6.75MB)
│   ├── CMakeLists.txt
│   └── main/
│       ├── CMakeLists.txt
│       ├── main.c              # State Machine 4 Mode & RTOS Dual-Core
│       ├── gateway_hal.h/.c    # Abstraksi RS485 UART, Tombol, LED RGB
│       ├── storage_mgr.h/.c    # NVS Flash & LittleFS storage engine
│       ├── ai_engine.h/.c      # PSRAM Tensor Arena, Autoencoder & Q-Table
│       ├── blynk_bridge.h/.c   # Cloud sync virtual pin & emergency alerts
│       └── external_protocols.h/.c # Modbus RTU Master & Tuya Local
├── node/                       # Firmware Universal ESP32-C3 / C6 Node
│   ├── partitions.csv          # 4MB Flash: NVS, Dual OTA (1.6MB), LittleFS (576KB)
│   ├── CMakeLists.txt
│   └── main/
│       ├── CMakeLists.txt
│       ├── main.c              # Discovery Beacon & Provisioning loop
│       ├── node_hal.h/.c       # Alokasi pin dinamis (GPIO, ADC, PWM)
│       ├── edge_filter.h/.c    # Deadband & moving average noise suppressor
│       └── local_reflex.h/.c   # Refleks otonom <100ms & Failsafe manager
├── storage_schema/             # Skema Database Memori & Konfigurasi
│   ├── nvs_schema.h            # Key-Value NVS (maks 15 karakter per ESP-IDF)
│   ├── storage_paths.h         # Mount point path LittleFS
│   ├── drivers_catalog.json    # Kamus 19 master driver komponen
│   ├── nodes_profile.json      # Skema profil node terdaftar di Gateway
│   └── rules_table.json        # Skema Global Rule Engine lintas node
└── tests/                      # Unit Test Mandiri (GCC Native PC)
    ├── test_protocol.c         # Verifikasi batas ukuran MTU ESP-NOW (<= 250B)
    ├── test_protocol_validator.c # Verifikasi deteksi paket korup & magic byte
    ├── test_edge_reflex.c      # Verifikasi filter deadband & trip <100ms
    └── test_ai_engine.c        # Verifikasi Autoencoder MSE & Q-Table Bellman
```

---

## 3. Pemetaan Pinout Hardware Aman (Safe GPIO)

### ESP32-S3 Gateway
| Fungsi | GPIO | Tipe | Keterangan |
|---|---|---|---|
| **Pairing Button** | GPIO 4 | Input (Pull-Up) | Ditekan 2 detik untuk masuk Mode 1B |
| **Reset Button** | GPIO 5 | Input (Pull-Up) | Ditekan 5 detik untuk Factory Reset |
| **Status LED Red** | GPIO 6 | Output | Indikator Fault (Mode 3/4) |
| **Status LED Green** | GPIO 7 | Output | Indikator Running (Mode 2A) |
| **Status LED Blue** | GPIO 15 | Output | Indikator Setup (Mode 1) |
| **RS485 TX** | GPIO 17 | UART 1 TX | Jalur Transmit Modbus |
| **RS485 RX** | GPIO 18 | UART 1 RX | Jalur Receive Modbus |
| **RS485 RTS (DE/RE)** | GPIO 16 | Output | Direction Flow Control Modbus Half-Duplex |
| **I2C SDA / SCL** | GPIO 8 / 9 | I2C | Bus sensor lokal / RTC eksternal |

> **PERINGATAN:** GPIO 26–37 terhubung ke Octal Flash & Octal PSRAM bawaan ESP32-S3 dan **DILARANG KERAS** digunakan sebagai GPIO biasa.

### ESP32-C3 Universal Node
- **Pin Aman Terproteksi**: GPIO 0, 1, 3 (ADC/IO), GPIO 4, 5 (I2C/IO), GPIO 6, 7 (Relay/PWM), GPIO 10 (Digital In), GPIO 20, 21 (UART TX/RX).
- **Strapping Pin Dihindari**: GPIO 2, 8, 9 tidak dialokasikan secara dinamis agar node tidak gagal boot.

---

## 4. State Machine 4 Mode Gateway

1. **Mode 1: Setup Mode**
   - **1A (Gateway Setup)**: Captive Portal Wi-Fi SoftAP `192.168.4.1` untuk konfigurasi router, kredensial Blynk, dan sandi Superuser.
   - **1B (Node Setup)**: Jendela waktu pairing terbuka 3 menit saat tombol pairing ditekan. Node baru mengirim discovery beacon dan menerima profil pin secara terpusat.
2. **Mode 2: Running Mode**
   - **2A (Full Hybrid)**: Wi-Fi terhubung, Blynk Cloud aktif, sinkronisasi dua arah normal.
   - **2B (Running-Faulty / Local Only)**: Internet/Blynk terputus. Kontrol lokal Web, ESP-NOW, dan Rule Engine tetap beroperasi 100% normal tanpa downtime.
3. **Mode 3: Fault Mode**
   - Terpicu saat anomali heap, suhu, atau watchdog hang terdeteksi oleh AI.
   - Isolasi task bermasalah, pengiriman alert ke Blynk Notification, dan membuka antarmuka teknisi (Abaikan atau Controlled Safe Reboot).
4. **Mode 4: Recovery Mode (Safe Boot)**
   - Minimal Emergency Web Server aktif dari PROGMEM (anti-gagal meski LittleFS rusak).
   - Fitur 1-Click Guided Fix berbasis rekomendasi AI Q-Table.

---

## 5. Fitur Unggulan Firmware Node

1. **Zero-Config Discovery**: Node baru cukup di-flash firmware yang sama persis (`node/main/main.c`). Saat dinyalakan, node menyiarkan hardware descriptor.
2. **Autonomous Local Reflex (<100ms)**: Evaluasi sensor langsung di sisi Node. Jika arus beban PZEM > ambang batas (misal 16A), relai diputus dalam hitungan milidetik tanpa menunggu konfirmasi dari Gateway atau internet.
3. **Deadband Noise Filter**: Mencegah spam paket radio 2.4 GHz. Node hanya melapor jika delta nilai sensor melewati batas deadband atau batas waktu paksa tercapai.
4. **Smart Failsafe**: Jika koneksi Gateway hilang > 90 detik, aktuator otomatis beralih ke state aman (OFF untuk pemanas/pompa, HOLD untuk penerangan).

---

## 6. AI Engine di PSRAM ESP32-S3

1. **Subspace Autoencoder Anomaly Detection**:
   - Memantau metrik internal `[free_heap_kb, chip_temp_c, task_latency_ms]`.
   - Menghitung reconstruction error (MSE).
   - Metrik normal menghasilkan MSE `~0.000002` (jauh di bawah batas aman `0.05`).
   - Kondisi kritis (misal memory leak < 10KB atau overheat > 80C) menghasilkan MSE `~0.273`, otomatis memicu peringatan dini sebelum Gateway kernel panic.
2. **Reinforcement Q-Table (Bellman Equation)**:
   - Merekomendasikan tindakan 1-Click Guided Fix berdasarkan riwayat keberhasilan teknisi sebelumnya:
     - `FAULT_STATE_HEAP_LOW` -> `FIX_ACTION_CLEAR_CACHE`
     - `FAULT_STATE_TASK_HANG` -> `FIX_ACTION_RESTART_TASK`
     - `FAULT_STATE_OVERHEAT`  -> `FIX_ACTION_ENTER_MODE4`
     - `FAULT_STATE_RADIO_FAIL`-> `FIX_ACTION_RESET_RADIO`

---

## 7. Cara Menjalankan Unit Test (PC Host)

Seluruh logika protokol, validator biner, deadband filter, refleks otonom, dan AI engine dapat diuji langsung di PC menggunakan compiler GCC:

```bash
cd tests
# 1. Test Ukuran Paket MTU ESP-NOW
gcc -Wall -Wextra -I"../common/include" test_protocol.c -o test_protocol.exe && ./test_protocol.exe

# 2. Test Guard Validasi Header & Buffer Biner
gcc -Wall -Wextra -I"../common/include" test_protocol_validator.c -o test_protocol_validator.exe && ./test_protocol_validator.exe

# 3. Test Edge Noise Filter & Local Reflex <100ms
gcc -Wall -Wextra -I"../common/include" -I"../node/main" test_edge_reflex.c ../node/main/edge_filter.c ../node/main/local_reflex.c ../node/main/node_hal.c -o test_edge_reflex.exe && ./test_edge_reflex.exe

# 4. Test PSRAM AI Autoencoder & Q-Table Guided Fix
gcc -Wall -Wextra -I"../gateway/main" test_ai_engine.c ../gateway/main/ai_engine.c -o test_ai_engine.exe && ./test_ai_engine.exe
```

---

## 8. Build & Flash ke Hardware (ESP-IDF v5.x)

### Flash ESP32-S3 Gateway:
```bash
cd gateway
idf.py set-target esp32s3
idf.py build
idf.py -p COMx flash monitor
```

### Flash ESP32-C3 / C6 Node:
```bash
cd node
idf.py set-target esp32c3   # atau esp32c6
idf.py build
idf.py -p COMy flash monitor
```
