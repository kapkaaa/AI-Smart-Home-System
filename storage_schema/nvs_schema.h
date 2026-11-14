#ifndef NVS_SCHEMA_H
#define NVS_SCHEMA_H

/*
 * ESP-IDF NVS Constraint:
 * - Namespace max 15 karakter
 * - Key max 15 karakter
 */

/* =========================================================================
 * GATEWAY NVS NAMESPACES & KEYS
 * ========================================================================= */

#define NVS_NS_GATEWAY_NET  "gw_net"
#define NVS_KEY_WIFI_SSID   "wifi_ssid"     /* string[33] */
#define NVS_KEY_WIFI_PASS   "wifi_pass"     /* string[65] */
#define NVS_KEY_BLYNK_TMPL  "blynk_tmpl"    /* string[32] */
#define NVS_KEY_BLYNK_NAME  "blynk_name"    /* string[32] */
#define NVS_KEY_BLYNK_AUTH  "blynk_auth"    /* string[33] */
#define NVS_KEY_ESPNOW_PMK  "espnow_pmk"    /* blob[16]   (Primary Master Key) */
#define NVS_KEY_ESPNOW_CH   "espnow_ch"     /* uint8_t    (Channel Wi-Fi 1-13) */

#define NVS_NS_GATEWAY_AUTH "gw_auth"
#define NVS_KEY_SU_PASS_HASH "su_phash"     /* blob[32]   (SHA-256 / PBKDF2) */
#define NVS_KEY_SU_SALT      "su_salt"      /* blob[16] */
#define NVS_KEY_USER_PHASH   "usr_phash"    /* blob[32] */
#define NVS_KEY_USER_SALT    "usr_salt"     /* blob[16] */

#define NVS_NS_GATEWAY_STATE "gw_state"
#define NVS_KEY_BOOT_MODE    "boot_mode"    /* uint8_t    (system_mode_t) */
#define NVS_KEY_BOOT_COUNT   "boot_cnt"     /* uint32_t   (Total boot) */
#define NVS_KEY_PANIC_COUNT  "panic_cnt"    /* uint8_t    (Reset berturut-turut) */
#define NVS_KEY_LAST_FAULT   "last_fault"   /* uint16_t   (Kode error terakhir) */

/* =========================================================================
 * NODE NVS NAMESPACES & KEYS (ESP32-C3 / C6)
 * ========================================================================= */

#define NVS_NS_NODE_CONFIG   "node_cfg"
#define NVS_KEY_NODE_ID      "node_id"      /* uint16_t   (ID unik assigned) */
#define NVS_KEY_ROLE         "role"         /* uint8_t    (node_role_t) */
#define NVS_KEY_GW_MAC       "gw_mac"       /* blob[6]    (MAC Address Gateway) */
#define NVS_KEY_PAIRED       "paired"       /* uint8_t    (0: Belum, 1: Paired) */
#define NVS_KEY_FAILSAFE     "failsafe"     /* uint8_t    (failsafe_action_t) */
#define NVS_KEY_PROV_BLOB    "prov_blob"    /* blob[sizeof(pkt_provision_cfg_t)] */

#endif /* NVS_SCHEMA_H */
