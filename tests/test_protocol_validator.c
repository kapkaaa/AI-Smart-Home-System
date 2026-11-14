#include <stdio.h>
#include <string.h>
#include <assert.h>
#include "../common/include/protocol_validator.h"

int main(void) {
    printf("[CHECK] Menjalankan uji validasi paket protokol...\n");

    /* 1. Uji NULL dan buffer terlalu kecil */
    assert(protocol_validate_packet(NULL, 0) == false);
    uint8_t tiny_buf[5] = {0};
    assert(protocol_validate_packet(tiny_buf, sizeof(tiny_buf)) == false);
    printf("  [PASS] Reject buffer NULL dan buffer di bawah ukuran header.\n");

    /* 2. Uji Magic Byte salah */
    pkt_header_t bad_magic_hdr = {
        .magic = 0xFF, /* Seharusnya 0xA5 */
        .version = PROTOCOL_VERSION,
        .msg_type = PKT_TYPE_HEARTBEAT,
        .seq_id = 1
    };
    assert(protocol_validate_packet((uint8_t *)&bad_magic_hdr, sizeof(bad_magic_hdr)) == false);
    printf("  [PASS] Reject Magic Byte tidak valid.\n");

    /* 3. Uji Versi salah */
    pkt_header_t bad_ver_hdr = {
        .magic = PROTOCOL_MAGIC,
        .version = 0x99,
        .msg_type = PKT_TYPE_HEARTBEAT,
        .seq_id = 1
    };
    assert(protocol_validate_packet((uint8_t *)&bad_ver_hdr, sizeof(bad_ver_hdr)) == false);
    printf("  [PASS] Reject versi protokol tidak valid.\n");

    /* 4. Uji Ukuran terpotong (Truncated Packet) */
    pkt_discovery_req_t req;
    memset(&req, 0, sizeof(req));
    req.header.magic = PROTOCOL_MAGIC;
    req.header.version = PROTOCOL_VERSION;
    req.header.msg_type = PKT_TYPE_DISCOVERY_REQ;
    /* Kirim ukuran kurang dari sizeof(pkt_discovery_req_t) */
    assert(protocol_validate_packet((uint8_t *)&req, sizeof(req) - 1) == false);
    printf("  [PASS] Reject paket terpotong / corrupt length.\n");

    /* 5. Uji Paket Valid Penuh */
    assert(protocol_validate_packet((uint8_t *)&req, sizeof(req)) == true);
    printf("  [PASS] Accept paket discovery req valid penuh (%zu bytes).\n", sizeof(req));

    /* 6. Uji Paket Telemetri Valid */
    pkt_telemetry_t telem;
    memset(&telem, 0, sizeof(telem));
    telem.header.magic = PROTOCOL_MAGIC;
    telem.header.version = PROTOCOL_VERSION;
    telem.header.msg_type = PKT_TYPE_TELEMETRY;
    assert(protocol_validate_packet((uint8_t *)&telem, sizeof(telem)) == true);
    printf("  [PASS] Accept paket telemetri valid (%zu bytes).\n", sizeof(telem));

    printf("[SUCCESS] Seluruh uji Fase 2 (Protokol Validator) lulus 100%%.\n");
    return 0;
}
