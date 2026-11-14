#ifndef PROTOCOL_H
#define PROTOCOL_H

#include "system_types.h"

#define PROTOCOL_MAGIC        0xA5
#define PROTOCOL_VERSION      0x01
#define ESPNOW_MAX_READINGS   10
#define ESPNOW_MAX_PINS       8

/* Packet Types */
typedef enum {
    PKT_TYPE_DISCOVERY_REQ  = 0x01,
    PKT_TYPE_DISCOVERY_RESP = 0x02,
    PKT_TYPE_PROVISION_CFG  = 0x03,
    PKT_TYPE_TELEMETRY      = 0x04,
    PKT_TYPE_COMMAND        = 0x05,
    PKT_TYPE_HEARTBEAT      = 0x06,
    PKT_TYPE_TIME_SYNC      = 0x07,
    PKT_TYPE_ACK            = 0x08
} packet_type_t;

#pragma pack(push, 1)

/* Common Packet Header (10 bytes) */
typedef struct {
    uint8_t  magic;
    uint8_t  version;
    uint8_t  msg_type;       /* packet_type_t */
    uint8_t  seq_id;
    uint8_t  sender_mac[6];
} pkt_header_t;

/* Node -> Gateway: Discovery Request */
typedef struct {
    pkt_header_t header;
    uint8_t      chip_family;        /* chip_family_t */
    uint8_t      flash_size_mb;
    uint8_t      fw_major;
    uint8_t      fw_minor;
    uint32_t     safe_gpio_mask;     /* Bitmask pin aman */
} pkt_discovery_req_t;

/* Gateway -> Node: Discovery Response */
typedef struct {
    pkt_header_t header;
    uint8_t      status;             /* 0: Reject, 1: Accepted */
    uint8_t      assigned_channel;
    uint8_t      gateway_mac[6];
} pkt_discovery_resp_t;

/* Pin Mapping Item */
typedef struct {
    uint8_t  pin_num;
    uint8_t  pin_type;               /* pin_type_t */
    uint16_t driver_id;              /* Dari drivers_catalog.json */
} pin_config_t;

/* Gateway -> Node: Centralized Provisioning Config */
typedef struct {
    pkt_header_t header;
    uint8_t      role;               /* node_role_t */
    uint8_t      failsafe_action;    /* failsafe_action_t */
    uint8_t      pin_count;
    pin_config_t pins[ESPNOW_MAX_PINS];
    
    /* Autonomous Local Reflex */
    uint8_t      reflex_enabled;
    uint16_t     reflex_sensor_driver_id;
    uint8_t      reflex_operator;    /* reflex_op_t */
    float        reflex_threshold;
    uint8_t      reflex_target_pin;
    uint8_t      reflex_action_val;
    
    /* Edge Noise Filter & Reporting */
    uint16_t     deadband_mv_or_munit;
    uint32_t     report_interval_ms;
} pkt_provision_cfg_t;

/* Sensor Reading Item */
typedef struct {
    uint8_t  pin_num;
    uint16_t driver_id;
    float    value;
} sensor_reading_t;

/* Node -> Gateway: Telemetry */
typedef struct {
    pkt_header_t     header;
    uint32_t         timestamp_sec;
    uint8_t          reading_count;
    sensor_reading_t readings[ESPNOW_MAX_READINGS];
} pkt_telemetry_t;

/* Gateway -> Node: Command */
typedef struct {
    pkt_header_t header;
    uint8_t      target_pin;
    uint8_t      cmd_type;           /* 0: SET_DIGITAL, 1: SET_PWM, 2: PULSE */
    uint16_t     value;
} pkt_command_t;

/* Node -> Gateway: Heartbeat */
typedef struct {
    pkt_header_t header;
    uint32_t     uptime_sec;
    uint32_t     free_heap;
    int8_t       rssi;
    uint8_t      status_flags;       /* bit 0: reflex tripped, bit 1: sensor error */
} pkt_heartbeat_t;

/* Gateway -> Node: Time Sync */
typedef struct {
    pkt_header_t header;
    uint32_t     epoch_seconds;
    int16_t      tz_offset_min;
} pkt_time_sync_t;

/* Bilateral: ACK */
typedef struct {
    pkt_header_t header;
    uint8_t      ack_seq_id;
    uint8_t      status_code;        /* 0: OK, 1: ERROR, 2: INVALID_PARAM */
} pkt_ack_t;

#pragma pack(pop)

#endif /* PROTOCOL_H */
