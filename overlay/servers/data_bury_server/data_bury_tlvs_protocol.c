/*
 * Momcozy 数据埋点 Data Service TLV 组包（设备上报，协议 v1.0.2）
 * Flash 环 packed(LE) -> 联机 TLV(BE)：按 packed 偏移组包
 */

#include "root_common_api.h"
#include "data_bury_tlvs_protocol.h"
#include "data_bury_persist_defs.h"
#include "app_data_bury_protocol_defs.h"

typedef int (*data_bury_tlv_pack_handler_t)(const uint8_t *packed, uint16_t packed_len,
                                            data_bury_tlv_builder_t *builder);

static uint32_t bury_rd_u32(const uint8_t *b)
{
    return (uint32_t)b[0] | ((uint32_t)b[1] << 8) | ((uint32_t)b[2] << 16) | ((uint32_t)b[3] << 24);
}

static uint16_t bury_rd_u16(const uint8_t *b)
{
    return (uint16_t)(b[0] | (b[1] << 8));
}

void data_bury_tlv_builder_init(data_bury_tlv_builder_t *builder, uint8_t *buf, uint16_t cap)
{
    if (builder) {
        builder->buf = buf;
        builder->cap = cap;
        builder->len = 0;
    }
}

int data_bury_tlv_builder_add(data_bury_tlv_builder_t *builder, const data_bury_tlv_node_t *node)
{
    uint16_t need;

    if (!builder || !builder->buf || !node) {
        return -1;
    }
    if (node->length > 0u && !node->value) {
        return -1;
    }

    need = (uint16_t)(APP_DATA_BURY_PROTOCOL_TLV_HDR_SIZE + node->length);
    if ((uint32_t)builder->len + need > builder->cap) {
        return -1;
    }

    builder->buf[builder->len++] = node->type;
    builder->buf[builder->len++] = node->length;
    if (node->length > 0u) {
        memcpy(&builder->buf[builder->len], node->value, node->length);
        builder->len = (uint16_t)(builder->len + node->length);
    }
    return 0;
}

uint16_t data_bury_tlv_builder_len(const data_bury_tlv_builder_t *builder)
{
    return builder ? builder->len : 0u;
}

static int bury_builder_add_u8(data_bury_tlv_builder_t *builder, uint8_t type, uint8_t val)
{
    data_bury_tlv_node_t node = {
        .type = type,
        .length = 1u,
        .value = &val,
    };
    return data_bury_tlv_builder_add(builder, &node);
}

static int bury_builder_add_u16_be(data_bury_tlv_builder_t *builder, uint8_t type, uint16_t val)
{
    uint8_t be[2] = { (uint8_t)(val >> 8), (uint8_t)val };
    data_bury_tlv_node_t node = {
        .type = type,
        .length = 2u,
        .value = be,
    };
    return data_bury_tlv_builder_add(builder, &node);
}

static int bury_builder_add_u32_be(data_bury_tlv_builder_t *builder, uint8_t type, uint32_t val)
{
    uint8_t be[4] = {
        (uint8_t)(val >> 24),
        (uint8_t)((val >> 16) & 0xFFu),
        (uint8_t)((val >> 8) & 0xFFu),
        (uint8_t)(val & 0xFFu),
    };
    data_bury_tlv_node_t node = {
        .type = type,
        .length = 4u,
        .value = be,
    };
    return data_bury_tlv_builder_add(builder, &node);
}

static int bury_builder_add_packed_u8(data_bury_tlv_builder_t *builder, uint8_t type,
                                      const uint8_t *packed, uint8_t off, uint16_t packed_len)
{
    if (packed_len < (uint16_t)off + 1u) {
        return -1;
    }
    return bury_builder_add_u8(builder, type, packed[off]);
}

static int bury_builder_add_packed_u16(data_bury_tlv_builder_t *builder, uint8_t type,
                                       const uint8_t *packed, uint8_t off, uint16_t packed_len)
{
    if (packed_len < (uint16_t)off + 2u) {
        return -1;
    }
    return bury_builder_add_u16_be(builder, type, bury_rd_u16(packed + off));
}

static int bury_builder_add_packed_s16(data_bury_tlv_builder_t *builder, uint8_t type,
                                       const uint8_t *packed, uint8_t off, uint16_t packed_len)
{
    if (packed_len < (uint16_t)off + 2u) {
        return -1;
    }
    return bury_builder_add_u16_be(builder, type, (uint16_t)(int16_t)bury_rd_u16(packed + off));
}

static int bury_builder_add_packed_u32(data_bury_tlv_builder_t *builder, uint8_t type,
                                       const uint8_t *packed, uint8_t off, uint16_t packed_len)
{
    if (packed_len < (uint16_t)off + 4u) {
        return -1;
    }
    return bury_builder_add_u32_be(builder, type, bury_rd_u32(packed + off));
}

static int bury_builder_add_raw(data_bury_tlv_builder_t *builder, uint8_t type,
                                const uint8_t *raw, uint8_t raw_len)
{
    data_bury_tlv_node_t node = {
        .type = type,
        .length = raw_len,
        .value = raw,
    };
    return data_bury_tlv_builder_add(builder, &node);
}

#define BURY_ADD_U8(b, t, p, o)   bury_builder_add_packed_u8((b), (t), (p), (o), packed_len)
#define BURY_ADD_U16(b, t, p, o)  bury_builder_add_packed_u16((b), (t), (p), (o), packed_len)
#define BURY_ADD_S16(b, t, p, o)  bury_builder_add_packed_s16((b), (t), (p), (o), packed_len)
#define BURY_ADD_U32(b, t, p, o)  bury_builder_add_packed_u32((b), (t), (p), (o), packed_len)
#define BURY_PACK_HDR(b, p) \
    (BURY_ADD_U8((b), 0, (p), 0) != 0 || BURY_ADD_U32((b), 1, (p), 1) != 0 || BURY_ADD_U32((b), 2, (p), 5) != 0)

static int data_bury_tlv_pack_pump_startup(const uint8_t *packed, uint16_t packed_len,
                                           data_bury_tlv_builder_t *builder)
{
    if (packed_len < sizeof(data_bury_evt_pump_startup_t)) {
        return -1;
    }
    if (BURY_PACK_HDR(builder, packed) != 0 ||
        BURY_ADD_U16(builder, 3, packed, 9) != 0 || BURY_ADD_U8(builder, 4, packed, 11) != 0) {
        return -1;
    }
    return 0;
}

static int data_bury_tlv_pack_pump_shutdown(const uint8_t *packed, uint16_t packed_len,
                                            data_bury_tlv_builder_t *builder)
{
    if (packed_len < sizeof(data_bury_evt_pump_shutdown_t)) {
        return -1;
    }
    if (BURY_PACK_HDR(builder, packed) != 0 ||
        BURY_ADD_U8(builder, 3, packed, 9) != 0 || BURY_ADD_U8(builder, 4, packed, 10) != 0) {
        return -1;
    }
    return 0;
}

static int data_bury_tlv_pack_bt_connect(const uint8_t *packed, uint16_t packed_len,
                                         data_bury_tlv_builder_t *builder)
{
    if (packed_len < sizeof(data_bury_evt_bt_connect_t)) {
        return -1;
    }
    if (BURY_PACK_HDR(builder, packed) != 0 ||
        BURY_ADD_U8(builder, 3, packed, 9) != 0 || BURY_ADD_U16(builder, 4, packed, 10) != 0 ||
        BURY_ADD_U8(builder, 5, packed, 12) != 0 || BURY_ADD_U32(builder, 6, packed, 13) != 0 ||
        BURY_ADD_U8(builder, 7, packed, 17) != 0) {
        return -1;
    }
    return 0;
}

static int data_bury_tlv_pack_pump_session_start(const uint8_t *packed, uint16_t packed_len,
                                                 data_bury_tlv_builder_t *builder)
{
    if (packed_len < sizeof(data_bury_evt_pump_session_start_t)) {
        return -1;
    }
    if (BURY_PACK_HDR(builder, packed) != 0 ||
        BURY_ADD_U16(builder, 3, packed, 9) != 0 || BURY_ADD_U8(builder, 4, packed, 11) != 0 ||
        BURY_ADD_U8(builder, 5, packed, 12) != 0 || BURY_ADD_U32(builder, 6, packed, 13) != 0 ||
        BURY_ADD_U16(builder, 7, packed, 17) != 0 || BURY_ADD_U8(builder, 8, packed, 19) != 0 ||
        BURY_ADD_U16(builder, 0x09, packed, 20) != 0) {
        return -1;
    }
    return 0;
}

static int data_bury_tlv_pack_pump_session_end(const uint8_t *packed, uint16_t packed_len,
                                               data_bury_tlv_builder_t *builder)
{
    uint8_t used;
    uint16_t program_off;

    if (packed_len < DATA_BURY_EVT_PUMP_SESSION_END_FIXED_LEN) {
        return -1;
    }
    used = packed[22];
    program_off = DATA_BURY_EVT_PUMP_SESSION_END_PROGRAM_OFF;
    if (packed_len < DATA_BURY_EVT_PUMP_SESSION_END_PAYLOAD_LEN(used)) {
        return -1;
    }
    if (BURY_PACK_HDR(builder, packed) != 0 ||
        BURY_ADD_U16(builder, 3, packed, 9) != 0 || BURY_ADD_U8(builder, 4, packed, 11) != 0 ||
        BURY_ADD_U8(builder, 5, packed, 12) != 0 || BURY_ADD_U32(builder, 6, packed, 13) != 0 ||
        BURY_ADD_U16(builder, 7, packed, 17) != 0 || BURY_ADD_U8(builder, 8, packed, 19) != 0 ||
        BURY_ADD_U8(builder, 9, packed, 20) != 0 || BURY_ADD_U8(builder, 10, packed, 21) != 0 ||
        BURY_ADD_U8(builder, 11, packed, 22) != 0 || BURY_ADD_U16(builder, 0x0Eu, packed, 23) != 0) {
        return -1;
    }
    if (used > 0u) {
        uint16_t program_body_len = (uint16_t)used * 6u;
        uint8_t program_body[DATA_BURY_PUMP_SESSION_MAX_MODES * 6u];
        uint8_t i;

        if (program_body_len > 255u) {
            return -1;
        }
        for (i = 0; i < used; i++) {
            uint32_t pid = bury_rd_u32(packed + program_off + (uint16_t)i * 6u);
            uint16_t pdu = bury_rd_u16(packed + program_off + (uint16_t)i * 6u + 4u);

            program_body[(uint16_t)i * 6u] = (uint8_t)(pid >> 24);
            program_body[(uint16_t)i * 6u + 1u] = (uint8_t)(pid >> 16);
            program_body[(uint16_t)i * 6u + 2u] = (uint8_t)(pid >> 8);
            program_body[(uint16_t)i * 6u + 3u] = (uint8_t)pid;
            program_body[(uint16_t)i * 6u + 4u] = (uint8_t)(pdu >> 8);
            program_body[(uint16_t)i * 6u + 5u] = (uint8_t)pdu;
        }
        if (bury_builder_add_raw(builder, 0x0Du, program_body, (uint8_t)program_body_len) != 0) {
            return -1;
        }
    }
    return 0;
}

static int data_bury_tlv_pack_pump_manual_adjust(const uint8_t *packed, uint16_t packed_len,
                                                 data_bury_tlv_builder_t *builder)
{
    if (packed_len < sizeof(data_bury_evt_pump_manual_adjust_t)) {
        return -1;
    }
    if (BURY_PACK_HDR(builder, packed) != 0 ||
        BURY_ADD_U16(builder, 3, packed, 9) != 0 || BURY_ADD_U8(builder, 4, packed, 11) != 0 ||
        BURY_ADD_U8(builder, 5, packed, 12) != 0 || BURY_ADD_U8(builder, 6, packed, 13) != 0 ||
        BURY_ADD_U8(builder, 7, packed, 14) != 0) {
        return -1;
    }
    if (packed[12] == DATA_BURY_ADJUST_SUCTION_LEVEL) {
        if (BURY_ADD_U16(builder, 0x09, packed, 15) != 0 ||
            BURY_ADD_U16(builder, 0x0A, packed, 17) != 0) {
            return -1;
        }
    }
    return 0;
}

static int data_bury_tlv_pack_pump_user_evt(const uint8_t *packed, uint16_t packed_len,
                                            data_bury_tlv_builder_t *builder)
{
    if (packed_len < sizeof(data_bury_evt_pump_user_evt_t)) {
        return -1;
    }
    if (BURY_PACK_HDR(builder, packed) != 0 ||
        BURY_ADD_U16(builder, 3, packed, 9) != 0 || BURY_ADD_U8(builder, 4, packed, 11) != 0 ||
        BURY_ADD_U8(builder, 5, packed, 12) != 0) {
        return -1;
    }
    return 0;
}

static int data_bury_tlv_pack_milk_ejection(const uint8_t *packed, uint16_t packed_len,
                                            data_bury_tlv_builder_t *builder)
{
    if (packed_len < sizeof(data_bury_evt_milk_ejection_t)) {
        return -1;
    }
    if (BURY_PACK_HDR(builder, packed) != 0 ||
        BURY_ADD_U16(builder, 3, packed, 9) != 0 || BURY_ADD_U16(builder, 4, packed, 11) != 0 ||
        BURY_ADD_U16(builder, 5, packed, 13) != 0 || BURY_ADD_U32(builder, 6, packed, 15) != 0 ||
        BURY_ADD_U16(builder, 7, packed, 19) != 0 || BURY_ADD_U32(builder, 8, packed, 21) != 0) {
        return -1;
    }
    return 0;
}

static int data_bury_tlv_pack_bat_charge_start(const uint8_t *packed, uint16_t packed_len,
                                               data_bury_tlv_builder_t *builder)
{
    if (packed_len < sizeof(data_bury_evt_bat_charge_start_t)) {
        return -1;
    }
    if (BURY_PACK_HDR(builder, packed) != 0 ||
        BURY_ADD_U8(builder, 3, packed, 9) != 0 || BURY_ADD_U16(builder, 4, packed, 10) != 0 ||
        BURY_ADD_S16(builder, 5, packed, 12) != 0 || BURY_ADD_U8(builder, 6, packed, 14) != 0) {
        return -1;
    }
    return 0;
}

static int data_bury_tlv_pack_bat_charge_end(const uint8_t *packed, uint16_t packed_len,
                                             data_bury_tlv_builder_t *builder)
{
    if (packed_len < sizeof(data_bury_evt_bat_charge_end_t)) {
        return -1;
    }
    if (BURY_PACK_HDR(builder, packed) != 0 ||
        BURY_ADD_U8(builder, 3, packed, 9) != 0 || BURY_ADD_U16(builder, 4, packed, 10) != 0 ||
        BURY_ADD_S16(builder, 5, packed, 12) != 0 || BURY_ADD_U8(builder, 6, packed, 14) != 0 ||
        BURY_ADD_U8(builder, 7, packed, 15) != 0) {
        return -1;
    }
    return 0;
}

static int data_bury_tlv_pack_exception(const uint8_t *packed, uint16_t packed_len,
                                        data_bury_tlv_builder_t *builder)
{
    if (packed_len < sizeof(data_bury_evt_exception_t)) {
        return -1;
    }
    if (BURY_PACK_HDR(builder, packed) != 0 ||
        BURY_ADD_U16(builder, 3, packed, 9) != 0 || BURY_ADD_U32(builder, 4, packed, 11) != 0) {
        return -1;
    }
    return 0;
}

static int data_bury_tlv_pack_firmware_update(const uint8_t *packed, uint16_t packed_len,
                                              data_bury_tlv_builder_t *builder)
{
    if (packed_len < sizeof(data_bury_evt_firmware_update_t)) {
        return -1;
    }
    if (BURY_PACK_HDR(builder, packed) != 0 ||
        BURY_ADD_U16(builder, 3, packed, 9) != 0 || BURY_ADD_U8(builder, 4, packed, 11) != 0 ||
        BURY_ADD_U16(builder, 5, packed, 12) != 0) {
        return -1;
    }
    return 0;
}

static int data_bury_tlv_pack_overflow(const uint8_t *packed, uint16_t packed_len,
                                       data_bury_tlv_builder_t *builder)
{
    if (packed_len < sizeof(data_bury_evt_overflow_t)) {
        return -1;
    }
    if (BURY_PACK_HDR(builder, packed) != 0 ||
        BURY_ADD_U16(builder, 3, packed, 9) != 0 || BURY_ADD_U16(builder, 4, packed, 11) != 0 ||
        BURY_ADD_U32(builder, 5, packed, 13) != 0) {
        return -1;
    }
    return 0;
}

static int data_bury_tlv_pack_milk_volume(const uint8_t *packed, uint16_t packed_len,
                                          data_bury_tlv_builder_t *builder)
{
    uint16_t sc;

    if (packed_len < DATA_BURY_EVT_MILK_VOLUME_FIXED_LEN) {
        return -1;
    }
    sc = bury_rd_u16(packed + 11);
    if (packed_len < (uint16_t)(DATA_BURY_EVT_MILK_VOLUME_FIXED_LEN + sc * 2u)) {
        return -1;
    }
    if (BURY_PACK_HDR(builder, packed) != 0 ||
        BURY_ADD_U16(builder, 3, packed, 9) != 0 || BURY_ADD_U16(builder, 4, packed, 11) != 0) {
        return -1;
    }
    if (sc > 0u) {
        uint16_t arr_bytes = (uint16_t)(sc * 2u);
        uint8_t arr[DATA_BURY_PUMP_SESSION_MAX_MODES * 2u];
        uint16_t i;

        if (arr_bytes > 255u) {
            return -1;
        }
        for (i = 0; i < sc; i++) {
            uint16_t v = bury_rd_u16(packed + 13 + i * 2u);
            arr[i * 2u] = (uint8_t)(v >> 8);
            arr[i * 2u + 1u] = (uint8_t)v;
        }
        if (bury_builder_add_raw(builder, 5u, arr, (uint8_t)arr_bytes) != 0) {
            return -1;
        }
    }
    return 0;
}

static int data_bury_tlv_pack_heat_session_start(const uint8_t *packed, uint16_t packed_len,
                                                 data_bury_tlv_builder_t *builder)
{
    if (packed_len < sizeof(data_bury_evt_heat_session_start_t)) {
        return -1;
    }
    if (BURY_PACK_HDR(builder, packed) != 0 ||
        BURY_ADD_U16(builder, 3, packed, 9) != 0 || BURY_ADD_U8(builder, 4, packed, 11) != 0 ||
        BURY_ADD_U8(builder, 5, packed, 12) != 0) {
        return -1;
    }
    return 0;
}

static int data_bury_tlv_pack_heat_session_end(const uint8_t *packed, uint16_t packed_len,
                                               data_bury_tlv_builder_t *builder)
{
    if (packed_len < sizeof(data_bury_evt_heat_session_end_t)) {
        return -1;
    }
    if (BURY_PACK_HDR(builder, packed) != 0 ||
        BURY_ADD_U16(builder, 3, packed, 9) != 0 || BURY_ADD_U8(builder, 4, packed, 11) != 0 ||
        BURY_ADD_U8(builder, 5, packed, 12) != 0) {
        return -1;
    }
    return 0;
}

static int data_bury_tlv_pack_massage_session_start(const uint8_t *packed, uint16_t packed_len,
                                                    data_bury_tlv_builder_t *builder)
{
    if (packed_len < sizeof(data_bury_evt_massage_session_start_t)) {
        return -1;
    }
    if (BURY_PACK_HDR(builder, packed) != 0 ||
        BURY_ADD_U16(builder, 3, packed, 9) != 0 || BURY_ADD_U8(builder, 4, packed, 11) != 0) {
        return -1;
    }
    return 0;
}

static int data_bury_tlv_pack_massage_session_end(const uint8_t *packed, uint16_t packed_len,
                                                  data_bury_tlv_builder_t *builder)
{
    if (packed_len < sizeof(data_bury_evt_massage_session_end_t)) {
        return -1;
    }
    if (BURY_PACK_HDR(builder, packed) != 0 ||
        BURY_ADD_U16(builder, 3, packed, 9) != 0 || BURY_ADD_U8(builder, 4, packed, 11) != 0) {
        return -1;
    }
    return 0;
}

static int data_bury_tlv_pack_wear_detection(const uint8_t *packed, uint16_t packed_len,
                                             data_bury_tlv_builder_t *builder)
{
    if (packed_len < sizeof(data_bury_evt_wear_detection_t)) {
        return -1;
    }
    if (BURY_PACK_HDR(builder, packed) != 0 ||
        BURY_ADD_U8(builder, 3, packed, 9) != 0 || BURY_ADD_U32(builder, 4, packed, 10) != 0) {
        return -1;
    }
    return 0;
}
#undef BURY_PACK_HDR
#undef BURY_ADD_U32
#undef BURY_ADD_S16
#undef BURY_ADD_U16
#undef BURY_ADD_U8

static data_bury_tlv_pack_handler_t data_bury_tlv_get_pack_handler(uint8_t cmd_id)
{
    switch (cmd_id) {
    case APP_BURY_CMD_PUMP_STARTUP:
        return data_bury_tlv_pack_pump_startup;
    case APP_BURY_CMD_PUMP_SHUTDOWN:
        return data_bury_tlv_pack_pump_shutdown;
    case APP_BURY_CMD_BT_CONNECT_EVENT:
        return data_bury_tlv_pack_bt_connect;
    case APP_BURY_CMD_PUMP_SESSION_START:
        return data_bury_tlv_pack_pump_session_start;
    case APP_BURY_CMD_PUMP_SESSION_END:
        return data_bury_tlv_pack_pump_session_end;
    case APP_BURY_CMD_PUMP_MANUAL_ADJUST:
        return data_bury_tlv_pack_pump_manual_adjust;
    case APP_BURY_CMD_PUMP_USER_EVENT:
        return data_bury_tlv_pack_pump_user_evt;
    case APP_BURY_CMD_MILK_EJECTION:
        return data_bury_tlv_pack_milk_ejection;
    case APP_BURY_CMD_BAT_CHARGE_START:
        return data_bury_tlv_pack_bat_charge_start;
    case APP_BURY_CMD_BAT_CHARGE_END:
        return data_bury_tlv_pack_bat_charge_end;
    case APP_BURY_CMD_EXCEPTION:
        return data_bury_tlv_pack_exception;
    case APP_BURY_CMD_FIRMWARE_UPDATE:
        return data_bury_tlv_pack_firmware_update;
    case APP_BURY_CMD_OVERFLOW:
        return data_bury_tlv_pack_overflow;
    case APP_BURY_CMD_MILK_VOLUME:
        return data_bury_tlv_pack_milk_volume;
    case APP_BURY_CMD_HEAT_SESSION_START:
        return data_bury_tlv_pack_heat_session_start;
    case APP_BURY_CMD_HEAT_SESSION_END:
        return data_bury_tlv_pack_heat_session_end;
    case APP_BURY_CMD_MASSAGE_SESSION_START:
        return data_bury_tlv_pack_massage_session_start;
    case APP_BURY_CMD_MASSAGE_SESSION_END:
        return data_bury_tlv_pack_massage_session_end;
    case APP_BURY_CMD_WEAR_DETECTION:
        return data_bury_tlv_pack_wear_detection;
    default:
        return NULL;
    }
}

uint16_t data_bury_tlv_packed_to_wire(uint8_t cmd_id, const uint8_t *packed, uint16_t packed_len,
                                      uint8_t *out, uint16_t out_cap)
{
    data_bury_tlv_builder_t builder;
    data_bury_tlv_pack_handler_t handler;

    if (!packed || !out || out_cap == 0u) {
        return 0;
    }

    handler = data_bury_tlv_get_pack_handler(cmd_id);
    if (!handler) {
        return 0;
    }

    data_bury_tlv_builder_init(&builder, out, out_cap);
    if (handler(packed, packed_len, &builder) != 0) {
        return 0;
    }
    return data_bury_tlv_builder_len(&builder);
}
