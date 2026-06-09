/*
* @Author: Owen
* @Date: 2026-06-05
* @Description: Momcozy 数据埋点：运行时写入 PSRAM 环(2048B)，关机整环快照至 Flash（8 槽），
*               BLE 建连后从 RAM 环 + Flash 槽位依次读取上报。
 */
#include "root_common.h"
#include "root_common_api.h"
#include "storage_manager.h"
#include "storage_manager_internal.h"
#include "data_bury_persist_data.h"
#include "resource_manager_api.h"

static storage_manager_context_t *s_bury_ctx;
static uint8_t                      s_ring_inited;
static uint8_t                      s_ring_dirty;
static data_bury_ring_meta_t        s_ring_meta;
static data_bury_flash_bank_t       s_flash_bank;
static uint8_t                      s_flash_bank_loaded;
static uint8_t                     *s_ring_data;
static uint8_t                     *s_flash_slot_data;
static uint8_t                      s_ring_data_in_psram;
static uint8_t                      s_flash_flush_loaded;
static uint8_t                      s_flash_flush_phys_slot;
static data_bury_ring_meta_t        s_flash_flush_meta;

static uint16_t data_bury_ring_meta_storage_crc(const data_bury_ring_meta_storage_t *store)
{
    uint8_t buf[sizeof(data_bury_ring_meta_t) + sizeof(uint16_t)];
    memcpy(buf, &store->meta, sizeof(store->meta));
    memcpy(buf + sizeof(store->meta), &store->event_count, sizeof(store->event_count));
    return CRC16(buf, sizeof(buf));
}

static int data_bury_ring_buf_alloc(void)
{
    if (s_ring_data && s_flash_slot_data) {
        return 0;
    }
#if PSRAM_ENABLE
    if (!s_ring_data) {
        s_ring_data = (uint8_t *)u_malloz_psram(DATA_BURY_RING_DATA_SIZE, RES_PSRAM_TYPE);
        if (s_ring_data) {
            s_ring_data_in_psram = 1;
            LOG_STORE_WRAP("-->data bury ring psram alloc ok size=%u", (unsigned)DATA_BURY_RING_DATA_SIZE);
        } else {
            LOGE("data bury ring u_malloz_psram(%u) fail, try sram heap", (unsigned)DATA_BURY_RING_DATA_SIZE);
        }
    }
    if (!s_flash_slot_data) {
        s_flash_slot_data = (uint8_t *)u_malloz_psram(DATA_BURY_RING_DATA_SIZE, RES_PSRAM_TYPE);
        if (!s_flash_slot_data) {
            LOGE("data bury flash slot u_malloz_psram(%u) fail", (unsigned)DATA_BURY_RING_DATA_SIZE);
        }
    }
#endif
    if (!s_ring_data) {
        s_ring_data = (uint8_t *)u_malloz(DATA_BURY_RING_DATA_SIZE);
        s_ring_data_in_psram = 0;
    }
    if (!s_flash_slot_data) {
        s_flash_slot_data = (uint8_t *)u_malloz(DATA_BURY_RING_DATA_SIZE);
    }
    if (!s_ring_data || !s_flash_slot_data) {
        LOGE("data bury ring alloc failed ring=%p flash=%p", s_ring_data, s_flash_slot_data);
        return -1;
    }
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

static uint16_t data_bury_ring_count_records(const data_bury_ring_meta_t *meta, const uint8_t *ring_buf)
{
    uint16_t cnt = 0;
    uint16_t pos;

    if (!meta || !ring_buf || meta->head == meta->tail) {
        return 0;
    }
    pos = meta->tail;
    while (pos != meta->head) {
        if (pos + sizeof(data_bury_rec_hdr_t) > DATA_BURY_RING_DATA_SIZE) {
            break;
        }
        data_bury_rec_hdr_t hdr;
        memcpy(&hdr, ring_buf + pos, sizeof(hdr));
        if (hdr.payload_len == 0 || hdr.payload_len > DATA_BURY_EVENT_PAYLOAD_MAX) {
            break;
        }
        {
            uint16_t total = (uint16_t)(sizeof(hdr) + hdr.payload_len);
            if ((uint32_t)pos + total > DATA_BURY_RING_DATA_SIZE) {
                break;
            }
            cnt++;
            pos = data_bury_ring_advance(pos, total);
        }
    }
    return cnt;
}

static int data_bury_ring_drop_oldest(data_bury_ring_meta_t *meta, uint8_t *ring_buf)
{
    uint16_t tail;

    if (!meta || !ring_buf) {
        LOGE("data bury drop oldest invalid meta=%p buf=%p", meta, ring_buf);
        return -1;
    }
    tail = meta->tail;
    if (tail == meta->head) {
        LOGE("data bury drop oldest empty ring");
        return -1;
    }
    if (tail + sizeof(data_bury_rec_hdr_t) > DATA_BURY_RING_DATA_SIZE) {
        LOGE("data bury drop oldest corrupt tail=%u head=%u", (unsigned)tail, (unsigned)meta->head);
        meta->tail = 0;
        meta->head = 0;
        return -1;
    }
    {
        data_bury_rec_hdr_t hdr;
        memcpy(&hdr, ring_buf + tail, sizeof(hdr));
        if (hdr.payload_len == 0 || hdr.payload_len > DATA_BURY_EVENT_PAYLOAD_MAX) {
            LOGE("data bury drop oldest corrupt cmd=0x%02x len=%u tail=%u",
                 hdr.cmd_id, (unsigned)hdr.payload_len, (unsigned)tail);
            meta->tail = 0;
            meta->head = 0;
            return -1;
        }
        {
            uint16_t total = (uint16_t)(sizeof(data_bury_rec_hdr_t) + hdr.payload_len);
            meta->tail = data_bury_ring_advance(tail, total);
        }
    }
    return 0;
}

static int data_bury_flash_bank_ensure_loaded(storage_manager_context_t *ctx)
{
    store_func_args_t args;

    if (s_flash_bank_loaded) {
        return 0;
    }
    args.ctx  = ctx;
    args.arg1 = 0;
    args.arg2 = 0;
    args.val  = &s_flash_bank;
    if (read_data_bury_flash_bank_from_flash(args) != 0) {
        memset(&s_flash_bank, 0, sizeof(s_flash_bank));
    }
    s_flash_bank_loaded = 1;
    return 0;
}

static int data_bury_flash_bank_save(storage_manager_context_t *ctx)
{
    store_func_args_t args = {
        .ctx  = ctx,
        .arg1 = 0,
        .arg2 = 0,
        .val  = &s_flash_bank,
    };
    return write_data_bury_flash_bank_to_flash(args);
}

static int data_bury_ring_chunk_key(uint8_t slot, uint16_t chunk_index)
{
    if (slot >= DATA_BURY_FLASH_SLOT_NUM || chunk_index >= DATA_BURY_RING_CHUNK_NUM) {
        return -1;
    }
    return (int)(PERSIST_KEY_DATA_BURY_RING_CHUNK_BEGIN + slot * DATA_BURY_RING_CHUNK_NUM + chunk_index);
}

static int data_bury_ring_meta_slot_key(uint8_t slot)
{
    if (slot >= DATA_BURY_FLASH_SLOT_NUM) {
        return -1;
    }
    return (int)(PERSIST_KEY_DATA_BURY_RING_META_SLOT_BEGIN + slot);
}

static int data_bury_ring_load_slot_chunks(storage_manager_context_t *ctx, uint8_t slot, uint8_t *dst)
{
    uint8_t *chunk_buf;
    store_func_args_t args;
    uint16_t i;

    if (!dst) {
        LOGE("data bury load chunks dst null slot=%u", (unsigned)slot);
        return -1;
    }
    chunk_buf = (uint8_t *)u_malloz(DATA_BURY_RING_CHUNK_SIZE);
    if (!chunk_buf) {
        LOGE("data bury load chunks alloc fail slot=%u", (unsigned)slot);
        return -1;
    }
    args.ctx  = ctx;
    args.arg2 = (int)slot;
    args.val  = chunk_buf;
    for (i = 0; i < DATA_BURY_RING_CHUNK_NUM; i++) {
        uint32_t off;
        uint32_t copy_len;

        args.arg1 = (int)i;
        if (read_data_bury_ring_chunk_from_flash(args) != 0) {
            LOGE("data bury load chunks read fail slot=%u chunk=%u", (unsigned)slot, (unsigned)i);
            u_free(chunk_buf);
            return -1;
        }
        off = (uint32_t)i * DATA_BURY_RING_CHUNK_SIZE;
        copy_len = DATA_BURY_RING_CHUNK_SIZE;
        if (off + copy_len > DATA_BURY_RING_DATA_SIZE) {
            copy_len = DATA_BURY_RING_DATA_SIZE - off;
        }
        memcpy(dst + off, chunk_buf, copy_len);
    }
    u_free(chunk_buf);
    return 0;
}

static int data_bury_ring_save_slot_chunks(storage_manager_context_t *ctx, uint8_t slot, const uint8_t *src)
{
    uint8_t *chunk_buf;
    store_func_args_t args;
    uint16_t i;

    if (!src) {
        LOGE("data bury save chunks src null slot=%u", (unsigned)slot);
        return -1;
    }
    chunk_buf = (uint8_t *)u_malloz(DATA_BURY_RING_CHUNK_SIZE);
    if (!chunk_buf) {
        LOGE("data bury save chunks alloc fail slot=%u", (unsigned)slot);
        return -1;
    }
    args.ctx  = ctx;
    args.arg2 = (int)slot;
    args.val  = chunk_buf;
    for (i = 0; i < DATA_BURY_RING_CHUNK_NUM; i++) {
        uint32_t off;
        uint32_t copy_len;

        off = (uint32_t)i * DATA_BURY_RING_CHUNK_SIZE;
        copy_len = DATA_BURY_RING_CHUNK_SIZE;
        if (off + copy_len > DATA_BURY_RING_DATA_SIZE) {
            copy_len = DATA_BURY_RING_DATA_SIZE - off;
        }
        memcpy(chunk_buf, src + off, copy_len);
        if (off + DATA_BURY_RING_CHUNK_SIZE > DATA_BURY_RING_DATA_SIZE) {
            memset(chunk_buf + copy_len, 0, DATA_BURY_RING_CHUNK_SIZE - copy_len);
        }
        args.arg1 = (int)i;
        if (write_data_bury_ring_chunk_to_flash(args) != 0) {
            LOGE("data bury save chunks write fail slot=%u chunk=%u", (unsigned)slot, (unsigned)i);
            u_free(chunk_buf);
            return -1;
        }
    }
    u_free(chunk_buf);
    return 0;
}

static int data_bury_ring_read_slot_meta(storage_manager_context_t *ctx, uint8_t slot,
                                         data_bury_ring_meta_t *meta, uint16_t *event_count)
{
    store_func_args_t args = {
        .ctx  = ctx,
        .arg1 = (int)slot,
        .arg2 = 0,
        .val  = meta,
    };
    data_bury_ring_meta_storage_t store;
    int key;
    int ret;

    if (!meta) {
        LOGE("data bury read meta null meta slot=%u", (unsigned)slot);
        return -1;
    }
    key = data_bury_ring_meta_slot_key(slot);
    if (key < 0) {
        LOGE("data bury read meta invalid slot=%u", (unsigned)slot);
        return -1;
    }
    ret = user_database_get(ctx, key, (uint8_t *)&store, sizeof(store));
    if (ret == 0 && store.magic == STORAGE_DATA_BURY_RING_META_MAGIC_NUM
        && store.crc == data_bury_ring_meta_storage_crc(&store)) {
        memcpy(meta, &store.meta, sizeof(*meta));
        if (event_count) {
            *event_count = store.event_count;
        }
        return 0;
    }
    LOGE("data bury read meta fail slot=%u ret=%d magic=0x%x", (unsigned)slot, ret, store.magic);
    return -1;
}

static int data_bury_ring_write_slot_meta(storage_manager_context_t *ctx, uint8_t slot,
                                          const data_bury_ring_meta_t *meta, uint16_t event_count)
{
    store_func_args_t args = {
        .ctx  = ctx,
        .arg1 = (int)slot,
        .arg2 = 0,
        .val  = (void *)meta,
    };
    (void)event_count;
    return write_data_bury_ring_meta_to_flash(args);
}

static int data_bury_ring_flash_slot_pending_count(storage_manager_context_t *ctx, uint8_t slot)
{
    data_bury_ring_meta_t meta;
    const data_bury_ring_meta_t *active_meta;
    const uint8_t *ring_src;
    uint8_t *tmp_buf = NULL;
    int cnt;

    if (s_flash_flush_loaded && s_flash_flush_phys_slot == slot) {
        active_meta = &s_flash_flush_meta;
        ring_src    = s_flash_slot_data;
    } else {
        if (data_bury_ring_read_slot_meta(ctx, slot, &meta, NULL) != 0) {
            return 0;
        }
        active_meta = &meta;
        ring_src    = NULL;
    }
    if (active_meta->head == active_meta->tail) {
        return 0;
    }
    if (!ring_src) {
        uint8_t *load_dst = s_flash_slot_data;
        /* 正在 flush 某槽时，统计其它槽不得覆盖 s_flash_slot_data */
        if (s_flash_flush_loaded && slot != s_flash_flush_phys_slot) {
            tmp_buf = (uint8_t *)u_malloz(DATA_BURY_RING_DATA_SIZE);
            if (!tmp_buf) {
                LOGE("data bury flash slot pending tmp alloc fail slot=%u flush_slot=%u",
                     (unsigned)slot, (unsigned)s_flash_flush_phys_slot);
                return 0;
            }
            load_dst = tmp_buf;
        }
        if (data_bury_ring_load_slot_chunks(ctx, slot, load_dst) != 0) {
            LOGE("data bury flash slot pending load fail slot=%u", (unsigned)slot);
            u_free(tmp_buf);
            return 0;
        }
        ring_src = load_dst;
    }
    cnt = (int)data_bury_ring_count_records(active_meta, ring_src);
    u_free(tmp_buf);
    return cnt;
}

static int data_bury_ring_ensure_flash_flush_slot(storage_manager_context_t *ctx)
{
    uint8_t phys;

    if (s_flash_bank.count == 0) {
        LOGE("data bury ensure flash flush no pending slots");
        return -1;
    }
    phys = s_flash_bank.read_slot;
    if (s_flash_flush_loaded && s_flash_flush_phys_slot == phys) {
        /* 槽数据已在 s_flash_slot_data，仅刷新 meta（pop 后 tail 以内存为准） */
        return 0;
    }
    s_flash_flush_loaded = 0;
    if (data_bury_ring_read_slot_meta(ctx, phys, &s_flash_flush_meta, NULL) != 0) {
        LOGE("data bury ensure flash flush read meta fail slot=%u", (unsigned)phys);
        return -1;
    }
    if (s_flash_flush_meta.head == s_flash_flush_meta.tail) {
        LOGE("data bury ensure flash flush empty slot=%u", (unsigned)phys);
        return -1;
    }
    if (data_bury_ring_load_slot_chunks(ctx, phys, s_flash_slot_data) != 0) {
        LOGE("data bury ensure flash flush load chunks fail slot=%u", (unsigned)phys);
        return -1;
    }
    s_flash_flush_loaded     = 1;
    s_flash_flush_phys_slot  = phys;
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

void reset_data_bury_flash_bank(storage_manager_context_t *ctx, data_bury_flash_bank_t *bank)
{
    EXCEPTION_CHECK_VOID(ctx);
    EXCEPTION_CHECK_VOID(bank);
    memset(bank, 0, sizeof(*bank));
    {
        store_func_args_t args = {
            .ctx  = ctx,
            .arg1 = 0,
            .arg2 = 0,
            .val  = bank,
        };
        write_data_bury_flash_bank_to_flash(args);
    }
}

int write_data_bury_flash_bank_to_flash(store_func_args_t args)
{
    int ret = 0;
    data_bury_flash_bank_storage_t *store;

    EXCEPTION_CHECK_INT(args.ctx);
    data_bury_flash_bank_t *bank = (data_bury_flash_bank_t *)args.val;
    EXCEPTION_CHECK_INT(bank);
    store = u_malloz(sizeof(data_bury_flash_bank_storage_t));
    EXCEPTION_CHECK_INT(store);
    store->magic = STORAGE_DATA_BURY_FLASH_BANK_MAGIC_NUM;
    memcpy(&store->bank, bank, sizeof(data_bury_flash_bank_t));
    store->crc = CRC16(&store->bank, sizeof(data_bury_flash_bank_t));
    ret = user_database_put(args.ctx, PERSIST_KEY_DATA_BURY_FLASH_BANK, (uint8_t *)store,
                            sizeof(data_bury_flash_bank_storage_t));
    if (ret != 0) {
        LOGE("write data bury flash bank failure! ret: %d", ret);
    }
    u_free(store);
    return ret;
}

int read_data_bury_flash_bank_from_flash(store_func_args_t args)
{
    int ret = 0;
    data_bury_flash_bank_storage_t *store;

    EXCEPTION_CHECK_INT(args.ctx);
    data_bury_flash_bank_t *bank = (data_bury_flash_bank_t *)args.val;
    EXCEPTION_CHECK_INT(bank);
    store = u_malloz(sizeof(data_bury_flash_bank_storage_t));
    EXCEPTION_CHECK_INT(store);
    ret = user_database_get(args.ctx, PERSIST_KEY_DATA_BURY_FLASH_BANK, (uint8_t *)store,
                            sizeof(data_bury_flash_bank_storage_t));
    {
        uint16_t crc = CRC16(&store->bank, sizeof(data_bury_flash_bank_t));
        if (ret == 0 && store->magic == STORAGE_DATA_BURY_FLASH_BANK_MAGIC_NUM && store->crc == crc) {
            memcpy(bank, &store->bank, sizeof(data_bury_flash_bank_t));
        } else {
            if (ret == 0) {
                ret = -1;
            }
            reset_data_bury_flash_bank(args.ctx, bank);
            LOGE("read data bury flash bank failure! ret: %d, magic: 0x%x", ret, store->magic);
        }
    }
    u_free(store);
    return ret;
}

void reset_data_bury_ring_meta(storage_manager_context_t *ctx, uint8_t slot, data_bury_ring_meta_t *meta)
{
    data_bury_ring_meta_storage_t empty_store;

    EXCEPTION_CHECK_VOID(ctx);
    EXCEPTION_CHECK_VOID(meta);
    memset(meta, 0, sizeof(*meta));
    memset(&empty_store, 0, sizeof(empty_store));
    empty_store.magic = STORAGE_DATA_BURY_RING_META_MAGIC_NUM;
    memcpy(&empty_store.meta, meta, sizeof(*meta));
    empty_store.event_count = 0;
    empty_store.crc = data_bury_ring_meta_storage_crc(&empty_store);
    if (slot < DATA_BURY_FLASH_SLOT_NUM) {
        int key = data_bury_ring_meta_slot_key(slot);
        if (key >= 0) {
            (void)user_database_put(ctx, key, (uint8_t *)&empty_store, sizeof(empty_store));
        }
    }
}

int write_data_bury_ring_meta_to_flash(store_func_args_t args)
{
    int ret = 0;
    uint8_t slot;
    data_bury_ring_meta_storage_t *store;
    const data_bury_ring_meta_t *meta;

    EXCEPTION_CHECK_INT(args.ctx);
    slot = (uint8_t)args.arg1;
    if (slot >= DATA_BURY_FLASH_SLOT_NUM) {
        LOGE("data bury meta slot: %u out of range", (unsigned)slot);
        return -1;
    }
    meta = (const data_bury_ring_meta_t *)args.val;
    EXCEPTION_CHECK_INT(meta);
    store = u_malloz(sizeof(data_bury_ring_meta_storage_t));
    EXCEPTION_CHECK_INT(store);
    const uint8_t *ring_src = s_ring_data;

    store->magic = STORAGE_DATA_BURY_RING_META_MAGIC_NUM;
    memcpy(&store->meta, meta, sizeof(data_bury_ring_meta_t));
    if (s_flash_slot_data && s_flash_flush_loaded && s_flash_flush_phys_slot == slot) {
        ring_src = s_flash_slot_data;
    }
    store->event_count = data_bury_ring_count_records(meta, ring_src);
    store->crc = data_bury_ring_meta_storage_crc(store);
    ret = user_database_put(args.ctx, data_bury_ring_meta_slot_key(slot), (uint8_t *)store,
                            sizeof(data_bury_ring_meta_storage_t));
    if (ret != 0) {
        LOGE("write data bury ring meta failure! ret: %d, slot: %u", ret, (unsigned)slot);
    }
    u_free(store);
    return ret;
}

int read_data_bury_ring_meta_from_flash(store_func_args_t args)
{
    int ret = 0;
    uint8_t slot;
    data_bury_ring_meta_t *meta;
    data_bury_ring_meta_storage_t *store;

    EXCEPTION_CHECK_INT(args.ctx);
    slot = (uint8_t)args.arg1;
    if (slot >= DATA_BURY_FLASH_SLOT_NUM) {
        LOGE("data bury meta slot: %u out of range", (unsigned)slot);
        return -1;
    }
    meta = (data_bury_ring_meta_t *)args.val;
    EXCEPTION_CHECK_INT(meta);
    store = u_malloz(sizeof(data_bury_ring_meta_storage_t));
    EXCEPTION_CHECK_INT(store);
    ret = user_database_get(args.ctx, data_bury_ring_meta_slot_key(slot), (uint8_t *)store,
                            sizeof(data_bury_ring_meta_storage_t));
    if (ret == 0 && store->magic == STORAGE_DATA_BURY_RING_META_MAGIC_NUM
        && store->crc == data_bury_ring_meta_storage_crc(store)) {
        memcpy(meta, &store->meta, sizeof(data_bury_ring_meta_t));
    } else {
        if (ret == 0) {
            ret = -1;
        }
        reset_data_bury_ring_meta(args.ctx, slot, meta);
        LOGE("read data bury ring meta failure! ret: %d, slot: %u", ret, (unsigned)slot);
    }
    u_free(store);
    return ret;
}

int write_data_bury_ring_chunk_to_flash(store_func_args_t args)
{
    int ret = 0;
    uint8_t slot;
    uint16_t chunk_index;
    const uint8_t *chunk_src;
    data_bury_ring_chunk_storage_t *store;
    int key;

    EXCEPTION_CHECK_INT(args.ctx);
    chunk_index = (uint16_t)args.arg1;
    slot = (uint8_t)args.arg2;
    if (slot >= DATA_BURY_FLASH_SLOT_NUM || chunk_index >= DATA_BURY_RING_CHUNK_NUM) {
        LOGE("data bury chunk slot=%u index=%u out of range", (unsigned)slot, (unsigned)chunk_index);
        return -1;
    }
    chunk_src = (const uint8_t *)args.val;
    EXCEPTION_CHECK_INT(chunk_src);
    key = data_bury_ring_chunk_key(slot, chunk_index);
    if (key < 0) {
        LOGE("data bury write chunk invalid key slot=%u chunk=%u", (unsigned)slot, (unsigned)chunk_index);
        return -1;
    }
    store = u_malloz(sizeof(data_bury_ring_chunk_storage_t));
    EXCEPTION_CHECK_INT(store);
    store->magic = STORAGE_DATA_BURY_RING_CHUNK_MAGIC_NUM;
    memcpy(store->chunk, chunk_src, DATA_BURY_RING_CHUNK_SIZE);
    store->crc = CRC16(store->chunk, DATA_BURY_RING_CHUNK_SIZE);
    ret = user_database_put(args.ctx, key, (uint8_t *)store, sizeof(data_bury_ring_chunk_storage_t));
    if (ret != 0) {
        LOGE("write data bury ring chunk failure! ret: %d, slot: %u chunk: %u",
             ret, (unsigned)slot, (unsigned)chunk_index);
    }
    u_free(store);
    LOG_STORE_WRAP("-->write data bury ring chunk slot=%u idx=%u", (unsigned)slot, (unsigned)chunk_index);
    return ret;
}

int read_data_bury_ring_chunk_from_flash(store_func_args_t args)
{
    int ret = 0;
    uint8_t slot;
    uint16_t chunk_index;
    uint8_t *chunk_dst;
    data_bury_ring_chunk_storage_t *store;
    int key;

    EXCEPTION_CHECK_INT(args.ctx);
    chunk_index = (uint16_t)args.arg1;
    slot = (uint8_t)args.arg2;
    if (slot >= DATA_BURY_FLASH_SLOT_NUM || chunk_index >= DATA_BURY_RING_CHUNK_NUM) {
        LOGE("data bury chunk slot=%u index=%u out of range", (unsigned)slot, (unsigned)chunk_index);
        return -1;
    }
    chunk_dst = (uint8_t *)args.val;
    EXCEPTION_CHECK_INT(chunk_dst);
    key = data_bury_ring_chunk_key(slot, chunk_index);
    if (key < 0) {
        LOGE("data bury read chunk invalid key slot=%u chunk=%u", (unsigned)slot, (unsigned)chunk_index);
        return -1;
    }
    store = u_malloz(sizeof(data_bury_ring_chunk_storage_t));
    EXCEPTION_CHECK_INT(store);
    ret = user_database_get(args.ctx, key, (uint8_t *)store, sizeof(data_bury_ring_chunk_storage_t));
    {
        uint16_t crc = CRC16(store->chunk, DATA_BURY_RING_CHUNK_SIZE);
        if (ret == 0 && store->magic == STORAGE_DATA_BURY_RING_CHUNK_MAGIC_NUM && store->crc == crc) {
            memcpy(chunk_dst, store->chunk, DATA_BURY_RING_CHUNK_SIZE);
        } else {
            if (ret == 0) {
                ret = -1;
            }
            memset(chunk_dst, 0, DATA_BURY_RING_CHUNK_SIZE);
            LOGE("read data bury ring chunk failure! ret: %d, slot: %u chunk: %u",
                 ret, (unsigned)slot, (unsigned)chunk_index);
        }
    }
    u_free(store);
    return ret;
}

static void data_bury_ring_reset_ram(void)
{
    memset(&s_ring_meta, 0, sizeof(s_ring_meta));
    if (s_ring_data) {
        memset(s_ring_data, 0, DATA_BURY_RING_DATA_SIZE);
    }
    s_ring_dirty         = 0;
    s_flash_flush_loaded = 0;
}

int data_bury_ring_storage_reset(storage_manager_context_t *ctx)
{
    uint8_t slot;

    EXCEPTION_CHECK_INT(ctx);
    if (data_bury_ring_buf_alloc() != 0) {
        LOGE("data bury storage reset alloc fail");
        return -1;
    }
    data_bury_ring_reset_ram();
    memset(&s_flash_bank, 0, sizeof(s_flash_bank));
    s_flash_bank_loaded = 1;
    reset_data_bury_flash_bank(ctx, &s_flash_bank);
    for (slot = 0; slot < DATA_BURY_FLASH_SLOT_NUM; slot++) {
        data_bury_ring_meta_t empty_meta;
        reset_data_bury_ring_meta(ctx, slot, &empty_meta);
        if (data_bury_ring_save_slot_chunks(ctx, slot, s_ring_data) != 0) {
            LOGE("data bury storage reset clear slot=%u fail", (unsigned)slot);
            return -1;
        }
    }
    s_ring_inited = 1;
    s_bury_ctx    = ctx;
    return 0;
}

static int data_bury_ring_storage_init_internal(storage_manager_context_t *ctx)
{
    EXCEPTION_CHECK_INT(ctx);
    if (s_ring_inited && s_bury_ctx == ctx) {
        return 0;
    }
    if (data_bury_ring_buf_alloc() != 0) {
        LOGE("data bury storage init alloc fail");
        return -1;
    }
    s_bury_ctx = ctx;
    data_bury_ring_reset_ram();
    s_flash_bank_loaded = 0;
    (void)data_bury_flash_bank_ensure_loaded(ctx);
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
    boot_args     = args;
    boot_args.val = &info->boot_seq;
    (void)read_data_bury_use_session_boot_seq_from_flash(boot_args);
    return data_bury_ring_storage_init_internal(args.ctx);
}

int data_bury_ring_storage_init(storage_manager_context_t *ctx)
{
    return data_bury_ring_storage_init_internal(ctx);
}

int data_bury_ring_sync_to_flash(storage_manager_context_t *ctx)
{
    uint8_t phys;
    uint16_t event_count;
    store_func_args_t args;

    EXCEPTION_CHECK_INT(ctx);
    if (!s_ring_inited) {
        data_bury_ring_storage_init(ctx);
    }
    if (s_ring_meta.head == s_ring_meta.tail) {
        return 0;
    }
    if (data_bury_flash_bank_ensure_loaded(ctx) != 0) {
        LOGE("data bury sync flash bank load fail");
        return -1;
    }
    s_flash_flush_loaded = 0;

    phys = s_flash_bank.write_slot;
    event_count = data_bury_ring_count_records(&s_ring_meta, s_ring_data);
    args.ctx  = ctx;
    args.arg1 = (int)phys;
    args.arg2 = 0;
    args.val  = &s_ring_meta;
    if (write_data_bury_ring_meta_to_flash(args) != 0) {
        LOGE("data bury sync write meta fail slot=%u", (unsigned)phys);
        return -1;
    }
    if (data_bury_ring_save_slot_chunks(ctx, phys, s_ring_data) != 0) {
        LOGE("data bury sync save chunks fail slot=%u", (unsigned)phys);
        return -1;
    }
    LOG_STORE_WRAP("-->data bury sync slot=%u events=%u count=%u",
                   (unsigned)phys, (unsigned)event_count, (unsigned)s_flash_bank.count);

    if (s_flash_bank.count >= DATA_BURY_FLASH_SLOT_NUM) {
        s_flash_bank.read_slot = (uint8_t)((s_flash_bank.read_slot + 1u) % DATA_BURY_FLASH_SLOT_NUM);
    } else {
        s_flash_bank.count++;
    }
    s_flash_bank.write_slot = (uint8_t)((s_flash_bank.write_slot + 1u) % DATA_BURY_FLASH_SLOT_NUM);
    if (data_bury_flash_bank_save(ctx) != 0) {
        LOGE("data bury sync save flash bank fail slot=%u", (unsigned)phys);
        return -1;
    }

    data_bury_ring_reset_ram();
    return 0;
}

static int data_bury_ring_append_event(storage_manager_context_t *ctx, uint8_t svc_id, uint8_t cmd_id,
                                       const void *payload, uint16_t len)
{
    uint16_t needed;
    uint16_t head;

    EXCEPTION_CHECK_INT(ctx);
    if (!payload || len == 0) {
        LOGE("data bury append invalid payload=%p len=%u", payload, (unsigned)len);
        return -1;
    }
    {
        uint16_t max_len = data_bury_event_payload_max_len(cmd_id);
        if (max_len == 0 || len > max_len || len > DATA_BURY_EVENT_PAYLOAD_MAX) {
            LOGE("data bury append invalid cmd=0x%02x len=%u max=%u",
                 cmd_id, (unsigned)len, (unsigned)max_len);
            return -2;
        }
    }
    if (!s_ring_inited || s_bury_ctx != ctx) {
        if (data_bury_ring_storage_init(ctx) != 0) {
            LOGE("data bury append init fail cmd=0x%02x", cmd_id);
            return -4;
        }
    }
    if (!s_ring_data) {
        LOGE("data bury append ring buf null cmd=0x%02x", cmd_id);
        return -4;
    }

    needed = (uint16_t)(sizeof(data_bury_rec_hdr_t) + len);
    if (needed > DATA_BURY_RING_DATA_SIZE) {
        LOGE("data bury append record too large cmd=0x%02x len=%u needed=%u",
             cmd_id, (unsigned)len, (unsigned)needed);
        return -2;
    }

    for (;;) {
        uint16_t used = data_bury_ring_used(s_ring_meta.head, s_ring_meta.tail);
        uint16_t free_bytes = (uint16_t)(DATA_BURY_RING_DATA_SIZE - used);
        if (free_bytes >= needed) {
            break;
        }
        if (data_bury_ring_drop_oldest(&s_ring_meta, s_ring_data) != 0) {
            break;
        }
    }

    head = s_ring_meta.head;
    if (head + needed > DATA_BURY_RING_DATA_SIZE) {
        if (s_ring_meta.tail > 0) {
            uint16_t used = data_bury_ring_used(s_ring_meta.head, s_ring_meta.tail);
            memmove(s_ring_data, s_ring_data + s_ring_meta.tail, used);
            s_ring_meta.head = used;
            s_ring_meta.tail = 0;
            head             = s_ring_meta.head;
        }
        if (head + needed > DATA_BURY_RING_DATA_SIZE) {
            LOGE("data bury append ring full cmd=0x%02x needed=%u head=%u tail=%u",
                 cmd_id, (unsigned)needed, (unsigned)head, (unsigned)s_ring_meta.tail);
            return -3;
        }
    }

    {
        data_bury_rec_hdr_t hdr = {
            .svc_id      = svc_id,
            .cmd_id      = cmd_id,
            .payload_len = len,
        };
        memcpy(s_ring_data + head, &hdr, sizeof(hdr));
        memcpy(s_ring_data + head + sizeof(hdr), payload, len);
        s_ring_meta.head = (uint16_t)(head + needed);
        s_ring_dirty     = 1;
    }

    LOG_STORE_WRAP("-->data bury append cmd=0x%02x len=%u head=%u tail=%u",
                   cmd_id, (unsigned)len, (unsigned)s_ring_meta.head, (unsigned)s_ring_meta.tail);
    return 0;
}

int data_bury_ring_is_empty(storage_manager_context_t *ctx)
{
    uint16_t pending = 0;
    store_func_args_t args = {
        .ctx = ctx,
        .val = &pending,
    };
    (void)read_data_bury_ring_pending_count_from_ram(args);
    return pending == 0 ? 1 : 0;
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

static uint16_t data_bury_ring_pending_count_ram(void)
{
    return data_bury_ring_count_records(&s_ring_meta, s_ring_data);
}

static int data_bury_ring_peek_from_meta(const data_bury_ring_meta_t *meta, const uint8_t *ring_buf,
                                         data_bury_rec_hdr_t *hdr, uint8_t *out_payload, uint16_t out_buf_len)
{
    uint16_t tail;

    if (!meta || !ring_buf || !hdr) {
        LOGE("data bury peek invalid meta=%p buf=%p hdr=%p", meta, ring_buf, hdr);
        return -1;
    }
    if (meta->tail == meta->head) {
        LOGE("data bury peek empty ring head=%u tail=%u", (unsigned)meta->head, (unsigned)meta->tail);
        return -1;
    }
    tail = meta->tail;
    if (tail + sizeof(data_bury_rec_hdr_t) > DATA_BURY_RING_DATA_SIZE) {
        LOGE("data bury peek corrupt tail=%u head=%u", (unsigned)tail, (unsigned)meta->head);
        return -1;
    }
    memcpy(hdr, ring_buf + tail, sizeof(*hdr));
    if (hdr->payload_len == 0 || hdr->payload_len > DATA_BURY_EVENT_PAYLOAD_MAX) {
        LOGE("data bury peek corrupt cmd=0x%02x len=%u head=%u tail=%u",
             hdr->cmd_id, (unsigned)hdr->payload_len, (unsigned)meta->head, (unsigned)tail);
        return -1;
    }
    if (out_payload && out_buf_len >= hdr->payload_len) {
        memcpy(out_payload, ring_buf + tail + sizeof(*hdr), hdr->payload_len);
    } else if (out_payload) {
        LOGE("data bury peek buf too small need=%u got=%u cmd=0x%02x",
             (unsigned)hdr->payload_len, (unsigned)out_buf_len, hdr->cmd_id);
        return -2;
    }
    return 0;
}

static int data_bury_ring_peek_oldest(storage_manager_context_t *ctx, data_bury_rec_hdr_t *hdr,
                                      uint8_t *out_payload, uint16_t out_buf_len)
{
    EXCEPTION_CHECK_INT(ctx);
    EXCEPTION_CHECK_INT(hdr);
    if (!s_ring_inited || s_bury_ctx != ctx) {
        if (data_bury_ring_storage_init(ctx) != 0) {
            LOGE("data bury peek oldest init fail");
            return -4;
        }
    }
    if (!s_ring_data) {
        LOGE("data bury peek oldest ring buf null");
        return -4;
    }
    if (s_ring_meta.head != s_ring_meta.tail) {
        int ret = data_bury_ring_peek_from_meta(&s_ring_meta, s_ring_data, hdr, out_payload, out_buf_len);
        if (ret != 0 && data_bury_ring_drop_oldest(&s_ring_meta, s_ring_data) == 0) {
            LOGW("data bury peek ram recover head=%u tail=%u",
                 (unsigned)s_ring_meta.head, (unsigned)s_ring_meta.tail);
            s_ring_dirty = 1;
            ret = data_bury_ring_peek_from_meta(&s_ring_meta, s_ring_data, hdr, out_payload, out_buf_len);
        }
        return ret;
    }
    if (data_bury_flash_bank_ensure_loaded(ctx) != 0 || s_flash_bank.count == 0) {
        LOGE("data bury peek oldest no flash pending bank_loaded=%u count=%u",
             (unsigned)s_flash_bank_loaded, (unsigned)s_flash_bank.count);
        return -1;
    }
    if (data_bury_ring_ensure_flash_flush_slot(ctx) != 0) {
        LOGE("data bury peek oldest ensure flash slot fail read_slot=%u",
             (unsigned)s_flash_bank.read_slot);
        return -1;
    }
    {
        int ret = data_bury_ring_peek_from_meta(&s_flash_flush_meta, s_flash_slot_data, hdr,
                                                out_payload, out_buf_len);
        if (ret != 0 && data_bury_ring_drop_oldest(&s_flash_flush_meta, s_flash_slot_data) == 0) {
            LOGW("data bury peek flash recover slot=%u head=%u tail=%u",
                 (unsigned)s_flash_flush_phys_slot,
                 (unsigned)s_flash_flush_meta.head, (unsigned)s_flash_flush_meta.tail);
            (void)data_bury_ring_write_slot_meta(ctx, s_flash_flush_phys_slot, &s_flash_flush_meta, 0);
            if (s_flash_flush_meta.head == s_flash_flush_meta.tail) {
                data_bury_ring_meta_t empty_meta;
                reset_data_bury_ring_meta(ctx, s_flash_flush_phys_slot, &empty_meta);
                s_flash_bank.count--;
                if (s_flash_bank.count > 0) {
                    s_flash_bank.read_slot =
                        (uint8_t)((s_flash_bank.read_slot + 1u) % DATA_BURY_FLASH_SLOT_NUM);
                } else {
                    s_flash_bank.read_slot = s_flash_bank.write_slot;
                }
                (void)data_bury_flash_bank_save(ctx);
                s_flash_flush_loaded = 0;
            }
            ret = data_bury_ring_peek_oldest(ctx, hdr, out_payload, out_buf_len);
        }
        return ret;
    }
}

static int data_bury_ring_pop_oldest(storage_manager_context_t *ctx)
{
    data_bury_rec_hdr_t hdr;
    int ret;

    EXCEPTION_CHECK_INT(ctx);
    if (s_ring_meta.head != s_ring_meta.tail) {
        ret = data_bury_ring_peek_from_meta(&s_ring_meta, s_ring_data, &hdr, NULL, 0);
        if (ret != 0) {
            LOGE("data bury pop ram peek fail ret=%d head=%u tail=%u",
                 ret, (unsigned)s_ring_meta.head, (unsigned)s_ring_meta.tail);
            return -1;
        }
        {
            uint16_t total = (uint16_t)(sizeof(hdr) + hdr.payload_len);
            s_ring_meta.tail = data_bury_ring_advance(s_ring_meta.tail, total);
            s_ring_dirty     = 1;
        }
        LOG_STORE_WRAP("-->data bury pop ram cmd=0x%02x len=%u pending=%u",
                       hdr.cmd_id, (unsigned)hdr.payload_len, (unsigned)data_bury_ring_pending_count_ram());
        return 0;
    }

    if (data_bury_flash_bank_ensure_loaded(ctx) != 0 || s_flash_bank.count == 0) {
        LOGE("data bury pop no flash pending bank_loaded=%u count=%u",
             (unsigned)s_flash_bank_loaded, (unsigned)s_flash_bank.count);
        return -1;
    }
    if (data_bury_ring_ensure_flash_flush_slot(ctx) != 0) {
        LOGE("data bury pop ensure flash slot fail read_slot=%u", (unsigned)s_flash_bank.read_slot);
        return -1;
    }
    ret = data_bury_ring_peek_from_meta(&s_flash_flush_meta, s_flash_slot_data, &hdr, NULL, 0);
    if (ret != 0) {
        LOGE("data bury pop flash peek fail slot=%u ret=%d",
             (unsigned)s_flash_flush_phys_slot, ret);
        return -1;
    }
    {
        uint16_t total = (uint16_t)(sizeof(hdr) + hdr.payload_len);
        s_flash_flush_meta.tail = data_bury_ring_advance(s_flash_flush_meta.tail, total);
    }
    if (data_bury_ring_write_slot_meta(ctx, s_flash_flush_phys_slot, &s_flash_flush_meta, 0) != 0) {
        LOGE("data bury pop flash write meta fail slot=%u", (unsigned)s_flash_flush_phys_slot);
        return -1;
    }

    if (s_flash_flush_meta.head == s_flash_flush_meta.tail) {
        data_bury_ring_meta_t empty_meta;
        reset_data_bury_ring_meta(ctx, s_flash_flush_phys_slot, &empty_meta);
        s_flash_bank.count--;
        if (s_flash_bank.count > 0) {
            s_flash_bank.read_slot = (uint8_t)((s_flash_bank.read_slot + 1u) % DATA_BURY_FLASH_SLOT_NUM);
        } else {
            s_flash_bank.read_slot = s_flash_bank.write_slot;
        }
        (void)data_bury_flash_bank_save(ctx);
        s_flash_flush_loaded = 0;
        LOG_STORE_WRAP("-->data bury flash slot %u drained, left=%u",
                       (unsigned)s_flash_flush_phys_slot, (unsigned)s_flash_bank.count);
    } else {
        LOG_STORE_WRAP("-->data bury pop flash slot=%u cmd=0x%02x len=%u",
                       (unsigned)s_flash_flush_phys_slot, hdr.cmd_id, (unsigned)hdr.payload_len);
    }
    return 0;
}

int write_data_bury_ring_event_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    data_bury_ring_append_args_t *append_args = (data_bury_ring_append_args_t *)args.val;
    EXCEPTION_CHECK_INT(append_args);
    if (!append_args->payload || append_args->payload_len == 0) {
        LOGE("data bury write ring invalid append payload=%p len=%u",
             append_args->payload, (unsigned)append_args->payload_len);
        return -1;
    }
    return data_bury_ring_append_event(args.ctx, append_args->svc_id, append_args->cmd_id,
                                       append_args->payload, append_args->payload_len);
}

int read_data_bury_ring_pending_count_from_ram(store_func_args_t args)
{
    uint16_t cnt;
    uint8_t i;

    EXCEPTION_CHECK_INT(args.ctx);
    uint16_t *pending_count = (uint16_t *)args.val;
    EXCEPTION_CHECK_INT(pending_count);

    if (!s_ring_inited || s_bury_ctx != args.ctx) {
        (void)data_bury_ring_storage_init(args.ctx);
    }

    cnt = data_bury_ring_pending_count_ram();
    if (data_bury_flash_bank_ensure_loaded(args.ctx) == 0 && s_flash_bank.count > 0) {
        for (i = 0; i < s_flash_bank.count; i++) {
            uint8_t phys = (uint8_t)((s_flash_bank.read_slot + i) % DATA_BURY_FLASH_SLOT_NUM);
            cnt = (uint16_t)(cnt + (uint16_t)data_bury_ring_flash_slot_pending_count(args.ctx, phys));
        }
    }
    *pending_count = cnt;
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
