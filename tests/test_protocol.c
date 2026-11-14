#include <stdio.h>
#include <assert.h>
#include "../common/include/system_types.h"
#include "../common/include/protocol.h"

#define ESPNOW_MAX_PAYLOAD 250

int main(void) {
    printf("[CHECK] Verifikasi ukuran paket ESP-NOW...\n");

    /* Header check */
    assert(sizeof(pkt_header_t) == 10);
    printf("  pkt_header_t: %zu bytes\n", sizeof(pkt_header_t));

    /* Packet size checks <= 250 */
    assert(sizeof(pkt_discovery_req_t) <= ESPNOW_MAX_PAYLOAD);
    printf("  pkt_discovery_req_t: %zu bytes\n", sizeof(pkt_discovery_req_t));

    assert(sizeof(pkt_discovery_resp_t) <= ESPNOW_MAX_PAYLOAD);
    printf("  pkt_discovery_resp_t: %zu bytes\n", sizeof(pkt_discovery_resp_t));

    assert(sizeof(pkt_provision_cfg_t) <= ESPNOW_MAX_PAYLOAD);
    printf("  pkt_provision_cfg_t: %zu bytes\n", sizeof(pkt_provision_cfg_t));

    assert(sizeof(pkt_telemetry_t) <= ESPNOW_MAX_PAYLOAD);
    printf("  pkt_telemetry_t: %zu bytes\n", sizeof(pkt_telemetry_t));

    assert(sizeof(pkt_command_t) <= ESPNOW_MAX_PAYLOAD);
    printf("  pkt_command_t: %zu bytes\n", sizeof(pkt_command_t));

    assert(sizeof(pkt_heartbeat_t) <= ESPNOW_MAX_PAYLOAD);
    printf("  pkt_heartbeat_t: %zu bytes\n", sizeof(pkt_heartbeat_t));

    assert(sizeof(pkt_time_sync_t) <= ESPNOW_MAX_PAYLOAD);
    printf("  pkt_time_sync_t: %zu bytes\n", sizeof(pkt_time_sync_t));

    assert(sizeof(pkt_ack_t) <= ESPNOW_MAX_PAYLOAD);
    printf("  pkt_ack_t: %zu bytes\n", sizeof(pkt_ack_t));

    printf("[PASS] Semua ukuran paket binary valid dan muat dalam batas MTU ESP-NOW (<= %d bytes).\n", ESPNOW_MAX_PAYLOAD);
    return 0;
}
