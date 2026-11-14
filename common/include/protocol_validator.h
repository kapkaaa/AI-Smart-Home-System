#ifndef PROTOCOL_VALIDATOR_H
#define PROTOCOL_VALIDATOR_H

#include "protocol.h"
#include <stddef.h>

/* Validasi ukuran paket berdasarkan tipe */
static inline size_t protocol_get_expected_size(packet_type_t type) {
    switch (type) {
        case PKT_TYPE_DISCOVERY_REQ:  return sizeof(pkt_discovery_req_t);
        case PKT_TYPE_DISCOVERY_RESP: return sizeof(pkt_discovery_resp_t);
        case PKT_TYPE_PROVISION_CFG:  return sizeof(pkt_provision_cfg_t);
        case PKT_TYPE_TELEMETRY:      return sizeof(pkt_telemetry_t);
        case PKT_TYPE_COMMAND:        return sizeof(pkt_command_t);
        case PKT_TYPE_HEARTBEAT:      return sizeof(pkt_heartbeat_t);
        case PKT_TYPE_TIME_SYNC:      return sizeof(pkt_time_sync_t);
        case PKT_TYPE_ACK:            return sizeof(pkt_ack_t);
        default:                      return 0;
    }
}

/* Validasi buffer yang diterima dari ESP-NOW */
static inline bool protocol_validate_packet(const uint8_t *buf, size_t len) {
    if (buf == NULL || len < sizeof(pkt_header_t)) {
        return false;
    }

    const pkt_header_t *hdr = (const pkt_header_t *)buf;

    /* Cek Magic Byte dan Versi Protokol */
    if (hdr->magic != PROTOCOL_MAGIC || hdr->version != PROTOCOL_VERSION) {
        return false;
    }

    /* Cek kecocokan ukuran payload dengan tipe pesan */
    size_t expected_size = protocol_get_expected_size((packet_type_t)hdr->msg_type);
    if (expected_size == 0 || len < expected_size) {
        return false;
    }

    return true;
}

#endif /* PROTOCOL_VALIDATOR_H */
