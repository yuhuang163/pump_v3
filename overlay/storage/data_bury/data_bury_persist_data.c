/*
* @Author: Owen
* @Date: 2026-06-05
* @Description: Momcozy 数据埋点事件 Flash 环存储，根据埋点事件类型，存储到pram环，关机前存入flash
                现在我希望扩大flash存储，关机前将pram环数据满后存入flash，flash最多能存4份，ble连接后数据从flash中读取
 */
#include "root_common.h"
#include "root_common_api.h"
#include "storage_manager.h"
#include "storage_manager_internal.h"
#include "data_bury_persist_data.h"
#include "resource_manager_api.h"

#define DATA_BURY_RING_MAGIC_RAM           0x424C4542u /* 'BLEB' */

static storage_manager_context_t *s_bury_ctx;
static uint8_t                      s_ring_inited;
static uint8_t                      s_ring_dirty;
static data_bury_ring_meta_t        s_ring_meta;
static uint8_t                     *s_ring_data;
static uint8_t                      s_ring_data_in_psram;

static int data_bury_ring_buf_alloc(void)
{
    if (s_ring_data) {
        return 0;
    }
#if PSRAM_ENABLE
    s_ring_data = (uint8_t *)u_malloz_psram(DATA_BURY_RING_DATA_SIZE, RES_PSRAM_TYPE);
    if (s_ring_data) {
        s_ring_data_in_psram = 1;
        LOG_STORE_WRAP("-->data bury ring psram alloc ok size=%u", (unsigned)DATA_BURY_RING_DATA_SIZE);
        return 0;
    }
    LOGE("data bury ring u_malloz_psram(%u) fail, try sram heap", (unsigned)DATA_BURY_RING_DATA_SIZE);
#endif
    s_ring_data = (uint8_t *)u_malloz(DATA_BURY_RING_DATA_SIZE);
    s_ring_data_in_psram = 0;
    if (!s_ring_data) {
        LOGE("data bury ring alloc %u failed", (unsigned)DATA_BURY_RING_DATA_SIZE);
        return -1;
    }
    LOG_STORE_WRAP("-->data bury ring sram heap alloc ok size=%u", (unsigned)DATA_BURY_RING_DATA_SIZE);
    return 0;
}

static uint16_t data_bury_ring_used(uint16_t head, uint16_t tail)
{
    if (head >= tail) {
        return (uint16_t)(head - tail);
    }
    return (uint16_t)(DATA_BURY_RING_DATA_SIZE - tail + head);
}

static uint16_t data_bury_ring_advance(uint16_t pos, uint16_t delta)
{
    pos = (uint16_t)(pos + delta);
    if (pos >= DATA_BURY_RING_DATA_SIZE) {
        pos = (uint16_t)(pos - DATA_BURY_RING_DATA_SIZE);
    }
    return pos;
}

static int data_bury_ring_drop_oldest(void)
{
    if (!s_ring_data) {
        return -1;
    }
    uint16_t tail = s_ring_meta.tail;
    if (tail == s_ring_meta.head) {
        return -1;
    }
    if (tail + sizeof(data_bury_rec_hdr_t) > DATA_BURY_RING_DATA_SIZE) {
        s_ring_meta.tail = 0;
        s_ring_meta.head = 0;
        return -1;
    }
    data_bury_rec_hdr_t hdr;
    memcpy(&hdr, s_ring_data + tail, sizeof(hdr));
    if (hdr.payload_len == 0 || hdr.payload_len > DATA_BURY_EVENT_PAYLOAD_MAX) {
        s_ring_meta.tail = 0;
        s_ring_meta.head = 0;
        return -1;
    }
    uint16_t total = (uint16_t)(sizeof(data_bury_rec_hdr_t) + hdr.payload_len);
    s_ring_meta.tail = data_bury_ring_advance(tail, total);
    return 0;
}

uint16_t data_bury_event_payload_max_len(uint8_t cmd_id)
{
    switch (cmd_id) {
    case APP_BURY_CMD_PUMP_STARTUP:
        return (uint16_t)sizeof(data_bury_evt_pump_startup_t);
    case APP_BURY_CMD_PUMP_SHUTDOWN:
        return (uint16_t)sizeof(data_bury_evt_pump_shutdown_t);
    case APP_BURY_CMD_BT_CONNECT_EVENT:
        return (uint16_t)sizeof(data_bury_evt_bt_connect_t);
    case APP_BURY_CMD_PUMP_SESSION_START:
        return (uint16_t)sizeof(data_bury_evt_pump_session_start_t);
    case APP_BURY_CMD_PUMP_SESSION_END:
        return (uint16_t)DATA_BURY_EVT_PUMP_SESSION_END_MAX_LEN;
    case APP_BURY_CMD_PUMP_MANUAL_ADJUST:
        return (uint16_t)sizeof(data_bury_evt_pump_manual_adjust_t);
    case APP_BURY_CMD_PUMP_USER_EVENT:
        return (uint16_t)sizeof(data_bury_evt_pump_user_evt_t);
    case APP_BURY_CMD_MILK_EJECTION:
        return (uint16_t)sizeof(data_bury_evt_milk_ejection_t);
    case APP_BURY_CMD_BAT_CHARGE_START:
        return (uint16_t)sizeof(data_bury_evt_bat_charge_start_t);
    case APP_BURY_CMD_BAT_CHARGE_END:
        return (uint16_t)sizeof(data_bury_evt_bat_charge_end_t);
    case APP_BURY_CMD_EXCEPTION:
        return (uint16_t)sizeof(data_bury_evt_exception_t);
    case APP_BURY_CMD_FIRMWARE_UPDATE:
        return (uint16_t)sizeof(data_bury_evt_firmware_update_t);
    case APP_BURY_CMD_OVERFLOW:
        return (uint16_t)sizeof(data_bury_evt_overflow_t);
    case APP_BURY_CMD_MILK_VOLUME:
        return (uint16_t)DATA_BURY_EVT_MILK_VOLUME_MAX_LEN;
    case APP_BURY_CMD_HEAT_SESSION_START:
        return (uint16_t)sizeof(data_bury_evt_heat_session_start_t);
    case APP_BURY_CMD_HEAT_SESSION_END:
        return (uint16_t)sizeof(data_bury_evt_heat_session_end_t);
    case APP_BURY_CMD_MASSAGE_SESSION_START:
        return (uint16_t)sizeof(data_bury_evt_massage_session_start_t);
    case APP_BURY_CMD_MASSAGE_SESSION_END:
        return (uint16_t)sizeof(data_bury_evt_massage_session_end_t);
    case APP_BURY_CMD_WEAR_DETECTION:
        return (uint16_t)sizeof(data_bury_evt_wear_detection_t);
    default:
        return 0;
    }
}

void reset_data_bury_ring_meta(storage_manager_context_t *ctx, data_bury_ring_meta_t *meta)
{
    EXCEPTION_CHECK_VOID(ctx);
    EXCEPTION_CHECK_VOID(meta);
    memset(meta, 0, sizeof(*meta));
    store_func_args_t args = {
        .ctx  = ctx,
        .arg1 = 0,
        .arg2 = 0,
        .val  = meta,
    };
    write_data_bury_ring_meta_to_flash(args);
}

int write_data_bury_ring_meta_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    data_bury_ring_meta_t *meta = (data_bury_ring_meta_t *)args.val;
    EXCEPTION_CHECK_INT(meta);
    data_bury_ring_meta_storage_t *store = u_malloz(sizeof(data_bury_ring_meta_storage_t));
    EXCEPTION_CHECK_INT(store);
    store->magic = STORAGE_DATA_BURY_RING_META_MAGIC_NUM;
    memcpy(&store->meta, meta, sizeof(data_bury_ring_meta_t));
    store->crc = CRC16(&store->meta, sizeof(data_bury_ring_meta_t));
    ret = user_database_put(args.ctx, PERSIST_KEY_DATA_BURY_RING_META, (uint8_t *)store,
                            sizeof(data_bury_ring_meta_storage_t));
    if (ret != 0) {
        LOGE("write data bury ring meta failure! ret: %d", ret);
    }
    u_free(store);
    return ret;
}

int read_data_bury_ring_meta_from_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    data_bury_ring_meta_t *meta = (data_bury_ring_meta_t *)args.val;
    EXCEPTION_CHECK_INT(meta);
    data_bury_ring_meta_storage_t *store = u_malloz(sizeof(data_bury_ring_meta_storage_t));
    EXCEPTION_CHECK_INT(store);
    ret = user_database_get(args.ctx, PERSIST_KEY_DATA_BURY_RING_META, (uint8_t *)store,
                            sizeof(data_bury_ring_meta_storage_t));
    uint16_t crc = CRC16(&store->meta, sizeof(data_bury_ring_meta_t));
    if (ret == 0 && store->magic == STORAGE_DATA_BURY_RING_META_MAGIC_NUM && store->crc == crc) {
        memcpy(meta, &store->meta, sizeof(data_bury_ring_meta_t));
    } else {
        if (ret == 0) {
            ret = -1;
        }
        reset_data_bury_ring_meta(args.ctx, meta);
        LOGE("read data bury ring meta failure! ret: %d, magic: 0x%x, crc: 0x%x, expect_crc: 0x%x",
             ret, store->magic, store->crc, crc);
    }
    u_free(store);
    return ret;
}

int write_data_bury_ring_chunk_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    uint16_t chunk_index = (uint16_t)args.arg1;
    if (chunk_index >= DATA_BURY_RING_CHUNK_NUM) {
        LOGE("data bury chunk_index: %u out of range", (unsigned)chunk_index);
        return -1;
    }
    const uint8_t *chunk_src = (const uint8_t *)args.val;
    EXCEPTION_CHECK_INT(chunk_src);
    data_bury_ring_chunk_storage_t *store = u_malloz(sizeof(data_bury_ring_chunk_storage_t));
    EXCEPTION_CHECK_INT(store);
    store->magic = STORAGE_DATA_BURY_RING_CHUNK_MAGIC_NUM;
    memcpy(store->chunk, chunk_src, DATA_BURY_RING_CHUNK_SIZE);
    store->crc = CRC16(store->chunk, DATA_BURY_RING_CHUNK_SIZE);
    ret = user_database_put(args.ctx, PERSIST_KEY_DATA_BURY_RING_CHUNK_BEGIN + chunk_index,
                            (uint8_t *)store, sizeof(data_bury_ring_chunk_storage_t));
    if (ret != 0) {
        LOGE("write data bury ring chunk failure! ret: %d, chunk: %u", ret, (unsigned)chunk_index);
    }
    u_free(store);
    LOG_STORE_WRAP("-->write data bury ring chunk %u", (unsigned)chunk_index);
    return ret;
}

int read_data_bury_ring_chunk_from_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    uint16_t chunk_index = (uint16_t)args.arg1;
    if (chunk_index >= DATA_BURY_RING_CHUNK_NUM) {
        LOGE("data bury chunk_index: %u out of range", (unsigned)chunk_index);
        return -1;
    }
    uint8_t *chunk_dst = (uint8_t *)args.val;
    EXCEPTION_CHECK_INT(chunk_dst);
    data_bury_ring_chunk_storage_t *store = u_malloz(sizeof(data_bury_ring_chunk_storage_t));
    EXCEPTION_CHECK_INT(store);
    ret = user_database_get(args.ctx, PERSIST_KEY_DATA_BURY_RING_CHUNK_BEGIN + chunk_index,
                            (uint8_t *)store, sizeof(data_bury_ring_chunk_storage_t));
    uint16_t crc = CRC16(store->chunk, DATA_BURY_RING_CHUNK_SIZE);
    if (ret == 0 && store->magic == STORAGE_DATA_BURY_RING_CHUNK_MAGIC_NUM && store->crc == crc) {
        memcpy(chunk_dst, store->chunk, DATA_BURY_RING_CHUNK_SIZE);
    } else {
        if (ret == 0) {
            ret = -1;
        }
        memset(chunk_dst, 0, DATA_BURY_RING_CHUNK_SIZE);
        LOGE("read data bury ring chunk failure! ret: %d, chunk: %u, magic: 0x%x",
             ret, (unsigned)chunk_index, store->magic);
    }
    u_free(store);
    return ret;
}

static int data_bury_ring_load_chunks_from_flash(storage_manager_context_t *ctx)
{
    if (!s_ring_data) {
        return -1;
    }
    uint8_t *chunk_buf = (uint8_t *)u_malloz(DATA_BURY_RING_CHUNK_SIZE);
    if (!chunk_buf) {
        LOGE("data bury ring load chunk_buf alloc %u failed", (unsigned)DATA_BURY_RING_CHUNK_SIZE);
        return -1;
    }
    store_func_args_t args = {
        .ctx  = ctx,
        .arg2 = 0,
        .val  = chunk_buf,
    };
    for (uint16_t i = 0; i < DATA_BURY_RING_CHUNK_NUM; i++) {
        args.arg1 = (int)i;
        if (read_data_bury_ring_chunk_from_flash(args) != 0) {
            u_free(chunk_buf);
            return -1;
        }
        uint32_t off = (uint32_t)i * DATA_BURY_RING_CHUNK_SIZE;
        uint32_t copy_len = DATA_BURY_RING_CHUNK_SIZE;
        if (off + copy_len > DATA_BURY_RING_DATA_SIZE) {
            copy_len = DATA_BURY_RING_DATA_SIZE - off;
        }
        memcpy(s_ring_data + off, chunk_buf, copy_len);
    }
    u_free(chunk_buf);
    return 0;
}

static int data_bury_ring_save_chunks_to_flash(storage_manager_context_t *ctx)
{
    if (!s_ring_data) {
        return -1;
    }
    uint8_t *chunk_buf = (uint8_t *)u_malloz(DATA_BURY_RING_CHUNK_SIZE);
    if (!chunk_buf) {
        LOGE("data bury ring save chunk_buf alloc %u failed", (unsigned)DATA_BURY_RING_CHUNK_SIZE);
        return -1;
    }
    store_func_args_t args = {
        .ctx  = ctx,
        .arg2 = 0,
        .val  = chunk_buf,
    };
    for (uint16_t i = 0; i < DATA_BURY_RING_CHUNK_NUM; i++) {
        uint32_t off = (uint32_t)i * DATA_BURY_RING_CHUNK_SIZE;
        uint32_t copy_len = DATA_BURY_RING_CHUNK_SIZE;
        if (off + copy_len > DATA_BURY_RING_DATA_SIZE) {
            copy_len = DATA_BURY_RING_DATA_SIZE - off;
        }
        memcpy(chunk_buf, s_ring_data + off, copy_len);
        if (off + DATA_BURY_RING_CHUNK_SIZE > DATA_BURY_RING_DATA_SIZE) {
            memset(chunk_buf + copy_len, 0, DATA_BURY_RING_CHUNK_SIZE - copy_len);
        }
        args.arg1 = (int)i;
        if (write_data_bury_ring_chunk_to_flash(args) != 0) {
            u_free(chunk_buf);
            return -1;
        }
    }
    u_free(chunk_buf);
    return 0;
}

static void data_bury_ring_reset_ram(void)
{
    memset(&s_ring_meta, 0, sizeof(s_ring_meta));
    if (s_ring_data) {
        memset(s_ring_data, 0, DATA_BURY_RING_DATA_SIZE);
    }
    s_ring_dirty = 0;
}

int data_bury_ring_storage_reset(storage_manager_context_t *ctx)
{
    EXCEPTION_CHECK_INT(ctx);
    if (data_bury_ring_buf_alloc() != 0) {
        return -1;
    }
    data_bury_ring_reset_ram();
    reset_data_bury_ring_meta(ctx, &s_ring_meta);
    if (data_bury_ring_save_chunks_to_flash(ctx) != 0) {
        return -1;
    }
    s_ring_inited = 1;
    s_ring_dirty  = 0;
    s_bury_ctx    = ctx;
    return 0;
}

static int data_bury_ring_storage_init_internal(storage_manager_context_t *ctx)
{
    store_func_args_t args;

    EXCEPTION_CHECK_INT(ctx);
    if (s_ring_inited && s_bury_ctx == ctx) {
        return 0;
    }
    if (data_bury_ring_buf_alloc() != 0) {
        return -1;
    }
    s_bury_ctx = ctx;
    args.ctx  = ctx;
    args.arg1 = 0;
    args.arg2 = 0;
    args.val  = &s_ring_meta;
    if (read_data_bury_ring_meta_from_flash(args) != 0) {
        data_bury_ring_reset_ram();
    }
    if (s_ring_meta.head > DATA_BURY_RING_DATA_SIZE || s_ring_meta.tail > DATA_BURY_RING_DATA_SIZE) {
        data_bury_ring_reset_ram();
        reset_data_bury_ring_meta(ctx, &s_ring_meta);
    }
    if (data_bury_ring_load_chunks_from_flash(ctx) != 0) {
        memset(s_ring_data, 0, DATA_BURY_RING_DATA_SIZE);
    }
    s_ring_inited = 1;
    s_ring_dirty  = 0;
    return 0;
}

int data_bury_infor_storage_init(store_func_args_t args)
{
    data_bury_info_t *info;
    store_func_args_t boot_args;

    EXCEPTION_CHECK_INT(args.ctx);
    info = (data_bury_info_t *)args.val;
    EXCEPTION_CHECK_INT(info);
    memset(info, 0, sizeof(*info));
    boot_args        = args;
    boot_args.val    = &info->boot_seq;
    (void)read_data_bury_use_session_boot_seq_from_flash(boot_args);
    return data_bury_ring_storage_init_internal(args.ctx);
}

int data_bury_ring_storage_init(storage_manager_context_t *ctx)
{
    return data_bury_ring_storage_init_internal(ctx);
}

int data_bury_ring_sync_to_flash(storage_manager_context_t *ctx)
{
    EXCEPTION_CHECK_INT(ctx);
    if (!s_ring_inited) {
        data_bury_ring_storage_init(ctx);
    }
    store_func_args_t args = {
        .ctx  = ctx,
        .arg1 = 0,
        .arg2 = 0,
        .val  = &s_ring_meta,
    };
    if (write_data_bury_ring_meta_to_flash(args) != 0) {
        return -1;
    }
    if (data_bury_ring_save_chunks_to_flash(ctx) != 0) {
        return -1;
    }
    s_ring_dirty = 0;
    return 0;
}

static int data_bury_ring_append_event(storage_manager_context_t *ctx, uint8_t svc_id, uint8_t cmd_id,
                                const void *payload, uint16_t len)
{
    EXCEPTION_CHECK_INT(ctx);
    if (!payload || len == 0) {
        return -1;
    }
    uint16_t max_len = data_bury_event_payload_max_len(cmd_id);
    if (max_len == 0 || len > max_len || len > DATA_BURY_EVENT_PAYLOAD_MAX) {
        return -2;
    }
    if (!s_ring_inited || s_bury_ctx != ctx) {
        if (data_bury_ring_storage_init(ctx) != 0) {
            return -4;
        }
    }
    if (!s_ring_data) {
        return -4;
    }

    uint16_t needed = (uint16_t)(sizeof(data_bury_rec_hdr_t) + len);
    if (needed > DATA_BURY_RING_DATA_SIZE) {
        return -2;
    }

    for (;;) {
        uint16_t used = data_bury_ring_used(s_ring_meta.head, s_ring_meta.tail);
        uint16_t free_bytes = (uint16_t)(DATA_BURY_RING_DATA_SIZE - used);
        if (free_bytes >= needed) {
            break;
        }
        if (data_bury_ring_drop_oldest() != 0) {
            break;
        }
    }

    uint16_t head = s_ring_meta.head;
    if (head + needed > DATA_BURY_RING_DATA_SIZE) {
        if (s_ring_meta.tail > 0) {
            uint16_t shift = (uint16_t)(DATA_BURY_RING_DATA_SIZE - head);
            memmove(s_ring_data + shift, s_ring_data, head);
            if (s_ring_meta.tail <= head) {
                s_ring_meta.tail = (uint16_t)(s_ring_meta.tail + shift);
            }
            head = shift;
            s_ring_meta.head = head;
        }
        if (head + needed > DATA_BURY_RING_DATA_SIZE) {
            return -3;
        }
    }

    data_bury_rec_hdr_t hdr = {
        .svc_id      = svc_id,
        .cmd_id      = cmd_id,
        .payload_len = len,
    };
    memcpy(s_ring_data + head, &hdr, sizeof(hdr));
    memcpy(s_ring_data + head + sizeof(hdr), payload, len);
    s_ring_meta.head = (uint16_t)(head + needed);
    s_ring_dirty     = 1;

    LOG_STORE_WRAP("-->data bury append cmd=0x%02x len=%u head=%u tail=%u",
                   cmd_id, (unsigned)len, (unsigned)s_ring_meta.head, (unsigned)s_ring_meta.tail);
    return 0;
}

int data_bury_ring_is_empty(storage_manager_context_t *ctx)
{
    (void)ctx;
    return (s_ring_meta.head == s_ring_meta.tail) ? 1 : 0;
}

int write_data_bury_use_session_boot_seq_to_flash(store_func_args_t args)
{
    int ret = 0;

    EXCEPTION_CHECK_INT(args.ctx);
    uint16_t *boot_seq = (uint16_t *)args.val;
    EXCEPTION_CHECK_INT(boot_seq);
    data_bury_use_session_boot_seq_storage_t *store_boot_seq =
        u_malloz(sizeof(data_bury_use_session_boot_seq_storage_t));
    EXCEPTION_CHECK_INT(store_boot_seq);
    store_boot_seq->magic = STORAGE_DATA_BURY_USE_SESSION_BOOT_SEQ_MAGIC_NUM;
    store_boot_seq->boot_seq = *boot_seq;
    store_boot_seq->crc = CRC16(&store_boot_seq->boot_seq, sizeof(store_boot_seq->boot_seq));
    ret = user_database_put(args.ctx, PERSIST_KEY_DATA_BURY_USE_SESSION_BOOT_SEQ, (uint8_t *)store_boot_seq,
                            sizeof(data_bury_use_session_boot_seq_storage_t));
    if (ret != 0) {
        LOGE("write data bury use_session boot_seq failure! ret: %d", ret);
    }
    u_free(store_boot_seq);

    return ret;
}

void reset_data_bury_use_session_boot_seq(storage_manager_context_t *ctx, uint16_t *boot_seq)
{
    if (boot_seq) {
        *boot_seq = 0u;
    }
    EXCEPTION_CHECK_VOID(ctx);
    store_func_args_t args = {
        .ctx  = ctx,
        .arg1 = 0,
        .arg2 = 0,
        .val  = boot_seq,
    };
    LOG_STORE_WRAP("==> reset data bury use_session boot_seq");
    write_data_bury_use_session_boot_seq_to_flash(args);
}

int read_data_bury_use_session_boot_seq_from_flash(store_func_args_t args)
{
    int ret = 0;

    EXCEPTION_CHECK_INT(args.ctx);
    uint16_t *boot_seq = (uint16_t *)args.val;
    EXCEPTION_CHECK_INT(boot_seq);
    data_bury_use_session_boot_seq_storage_t *store_boot_seq =
        u_malloz(sizeof(data_bury_use_session_boot_seq_storage_t));
    EXCEPTION_CHECK_INT(store_boot_seq);
    ret = user_database_get(args.ctx, PERSIST_KEY_DATA_BURY_USE_SESSION_BOOT_SEQ, (uint8_t *)store_boot_seq,
                            sizeof(data_bury_use_session_boot_seq_storage_t));
    uint16_t crc = CRC16(&store_boot_seq->boot_seq, sizeof(store_boot_seq->boot_seq));
    if (ret == 0 && store_boot_seq->magic == STORAGE_DATA_BURY_USE_SESSION_BOOT_SEQ_MAGIC_NUM
        && store_boot_seq->crc == crc) {
        *boot_seq = store_boot_seq->boot_seq;
    } else {
        if (ret == 0) {
            ret = -1;
        }
        reset_data_bury_use_session_boot_seq(args.ctx, boot_seq);
        LOGE("read data bury use_session boot_seq failure! ret: %d, magic: 0x%x, crc: 0x%x, expect_crc: 0x%x",
             ret, store_boot_seq->magic, store_boot_seq->crc, crc);
    }
    u_free(store_boot_seq);
    return ret;
}

static uint16_t data_bury_ring_pending_count(storage_manager_context_t *ctx)
{
    (void)ctx;
    if (!s_ring_data) {
        return 0;
    }
    uint16_t cnt = 0;
    uint16_t pos = s_ring_meta.tail;
    while (pos != s_ring_meta.head) {
        if (pos + sizeof(data_bury_rec_hdr_t) > DATA_BURY_RING_DATA_SIZE) {
            break;
        }
        data_bury_rec_hdr_t hdr;
        memcpy(&hdr, s_ring_data + pos, sizeof(hdr));
        if (hdr.payload_len == 0 || hdr.payload_len > DATA_BURY_EVENT_PAYLOAD_MAX) {
            break;
        }
        uint16_t total = (uint16_t)(sizeof(hdr) + hdr.payload_len);
        if ((uint32_t)pos + total > DATA_BURY_RING_DATA_SIZE) {
            break;
        }
        cnt++;
        pos = data_bury_ring_advance(pos, total);
    }
    return cnt;
}

static int data_bury_ring_peek_oldest(storage_manager_context_t *ctx, data_bury_rec_hdr_t *hdr,
                                 uint8_t *out_payload, uint16_t out_buf_len)
{
    EXCEPTION_CHECK_INT(ctx);
    EXCEPTION_CHECK_INT(hdr);
    if (!s_ring_inited || s_bury_ctx != ctx) {
        if (data_bury_ring_storage_init(ctx) != 0) {
            return -4;
        }
    }
    if (!s_ring_data) {
        return -4;
    }
    if (s_ring_meta.tail == s_ring_meta.head) {
        return -1;
    }
    uint16_t tail = s_ring_meta.tail;
    if (tail + sizeof(data_bury_rec_hdr_t) > DATA_BURY_RING_DATA_SIZE) {
        return -1;
    }
    memcpy(hdr, s_ring_data + tail, sizeof(*hdr));
    if (hdr->payload_len == 0 || hdr->payload_len > DATA_BURY_EVENT_PAYLOAD_MAX) {
        return -1;
    }
    if (out_payload && out_buf_len >= hdr->payload_len) {
        memcpy(out_payload, s_ring_data + tail + sizeof(*hdr), hdr->payload_len);
    } else if (out_payload) {
        return -2;
    }
    return 0;
}

static int data_bury_ring_pop_oldest(storage_manager_context_t *ctx)
{
    EXCEPTION_CHECK_INT(ctx);
    data_bury_rec_hdr_t hdr;
    if (data_bury_ring_peek_oldest(ctx, &hdr, NULL, 0) != 0) {
        return -1;
    }
    uint16_t total = (uint16_t)(sizeof(hdr) + hdr.payload_len);
    s_ring_meta.tail = data_bury_ring_advance(s_ring_meta.tail, total);
    s_ring_dirty     = 1;
    LOG_STORE_WRAP("-->data bury pop cmd=0x%02x len=%u head=%u tail=%u pending=%u",
                   hdr.cmd_id, (unsigned)hdr.payload_len, (unsigned)s_ring_meta.head,
                   (unsigned)s_ring_meta.tail, (unsigned)data_bury_ring_pending_count(ctx));
    return 0;
}

int write_data_bury_ring_event_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    data_bury_ring_append_args_t *append_args = (data_bury_ring_append_args_t *)args.val;
    EXCEPTION_CHECK_INT(append_args);
    if (!append_args->payload || append_args->payload_len == 0) {
        return -1;
    }
    return data_bury_ring_append_event(args.ctx, append_args->svc_id, append_args->cmd_id,
                                       append_args->payload, append_args->payload_len);
}

int read_data_bury_ring_pending_count_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    uint16_t *pending_count = (uint16_t *)args.val;
    EXCEPTION_CHECK_INT(pending_count);
    *pending_count = data_bury_ring_pending_count(args.ctx);
    return 0;
}

int write_data_bury_ring_pop_oldest_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    (void)args.val;
    return data_bury_ring_pop_oldest(args.ctx);
}

int read_data_bury_ring_peek_oldest_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    data_bury_ring_peek_args_t *peek_args = (data_bury_ring_peek_args_t *)args.val;
    EXCEPTION_CHECK_INT(peek_args);
    EXCEPTION_CHECK_INT(peek_args->hdr);
    return data_bury_ring_peek_oldest(args.ctx, peek_args->hdr, peek_args->payload,
                                      peek_args->payload_buf_len);
}
