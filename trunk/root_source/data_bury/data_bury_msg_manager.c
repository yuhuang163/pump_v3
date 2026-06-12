/*
 * 数据埋点延时消息池
 * delay_ms=0 -> 立即入 bury 线程队列；否则按 target_ticks 排序，到期后由 loop 触发
 */

#include "root_common_api.h"
#include "data_bury_msg_manager.h"
#include "data_bury_persist_defs.h"

#include <string.h>

#if DATA_BURY_MSG_POOL_ENABLE

#ifndef DATA_BURY_MSG_PROCESS_BATCH_MAX
#define DATA_BURY_MSG_PROCESS_BATCH_MAX             4u
#endif

static int data_bury_msg_is_user_event(uint16_t event)
{
    return (event > DATA_BURY_EVT_NONE && event != DATA_BURY_EVT_MSG_POOL_LOOP);
}

static void *data_bury_msg_payload_alloc(uint16_t size, uint8_t *in_psram)
{
    void *buf = NULL;

    if (in_psram) {
        *in_psram = 0u;
    }
#if PSRAM_ENABLE
    buf = u_malloz_psram(size, RES_PSRAM_TYPE);
    if (buf && in_psram) {
        *in_psram = 1u;
        return buf;
    }
#endif
    return u_malloz(size);
}

static void data_bury_msg_payload_free(void *buf, uint8_t in_psram)
{
    if (!buf) {
        return;
    }
    if (in_psram) {
        u_free_psram(buf, RES_PSRAM_TYPE);
    } else {
        u_free(buf);
    }
}

static void data_bury_msg_pool_entry_free_payload(data_bury_msg_pool_entry_t *entry)
{
    if (!entry) {
        return;
    }
    if (entry->event == DATA_BURY_EVT_PUBLISH && entry->body.publish.val) {
        data_bury_msg_payload_free(entry->body.publish.val, entry->body.publish.val_in_psram);
        entry->body.publish.val = NULL;
        entry->body.publish.val_in_psram = 0u;
    }
}

static int data_bury_msg_pool_payload_prepare(uint16_t event, data_bury_msg_pool_payload_t *payload_in,
                                              data_bury_msg_pool_payload_t *payload_out)
{
    if (!payload_out) {
        return -1;
    }
    memset(payload_out, 0, sizeof(*payload_out));

    if (event != DATA_BURY_EVT_PUBLISH) {
        return 0;
    }
    if (!payload_in || !payload_in->val || payload_in->arg1 <= 0) {
        return -2;
    }
    if ((uint16_t)payload_in->arg1 > DATA_BURY_EVENT_PAYLOAD_MAX) {
        return -3;
    }

    payload_out->publish_id = payload_in->publish_id;
    payload_out->arg1 = payload_in->arg1;
    payload_out->val = data_bury_msg_payload_alloc((uint16_t)payload_in->arg1, &payload_out->val_in_psram);
    if (!payload_out->val) {
        return -4;
    }
    memcpy(payload_out->val, payload_in->val, (uint16_t)payload_in->arg1);
    return 0;
}

static void data_bury_msg_build_from_entry(const data_bury_msg_pool_entry_t *entry, multi_func_msg_data_t *msg)
{
    memset(msg, 0, sizeof(*msg));
    msg->union_body.data_bury_extra.event_type = entry->event;
    if (entry->event == DATA_BURY_EVT_PUBLISH) {
        msg->union_body.data_bury_extra.publish_id = entry->body.publish.publish_id;
        msg->union_body.data_bury_extra.arg1 = entry->body.publish.arg1;
        msg->union_body.data_bury_extra.val = entry->body.publish.val;
        msg->union_body.data_bury_extra.val_in_psram = entry->body.publish.val_in_psram;
    }
}

static int data_bury_msg_dispatch_msg(data_bury_manager_context_t *ctx, multi_func_msg_data_t *msg)
{
    if (!ctx || !ctx->priv.enabled || !ctx->priv.bury_thd.msg_id || !msg) {
        return -1;
    }
    if (!data_bury_msg_is_user_event(msg->union_body.data_bury_extra.event_type)) {
        return -2;
    }
    return MSG_QUEUE_SEND(ctx->priv.bury_thd.msg_id, msg, 0);
}

static int data_bury_msg_dispatch_entry(data_bury_manager_context_t *ctx, data_bury_msg_pool_entry_t *entry)
{
    multi_func_msg_data_t msg = {0};
    int ret;

    data_bury_msg_build_from_entry(entry, &msg);
    ret = data_bury_msg_dispatch_msg(ctx, &msg);
    if (ret != 0) {
        LOGE("data bury dispatch entry fail event=0x%04x ret=%d", (unsigned)entry->event, ret);
        return ret;
    }
    if (entry->event == DATA_BURY_EVT_PUBLISH) {
        entry->body.publish.val = NULL;
        entry->body.publish.val_in_psram = 0u;
    }
    return 0;
}

static void data_bury_msg_deliver_entry(data_bury_manager_context_t *ctx, data_bury_msg_pool_entry_t *entry,
                                        uint8_t in_thread)
{
    multi_func_msg_data_t msg = {0};

    if (!ctx || !entry || !ctx->priv.enabled || !data_bury_msg_is_user_event(entry->event)) {
        data_bury_msg_pool_entry_free_payload(entry);
        return;
    }

    data_bury_msg_build_from_entry(entry, &msg);

    if (in_thread && ctx->hook && ctx->hook->data_bury_thread_route) {
        if (entry->event == DATA_BURY_EVT_PUBLISH) {
            entry->body.publish.val = NULL;
            entry->body.publish.val_in_psram = 0u;
        }
        ctx->hook->data_bury_thread_route(ctx, &msg);
        return;
    }
    if (data_bury_msg_dispatch_msg(ctx, &msg) != 0) {
        LOGE("data bury deliver failed event=0x%04x in_thread=%u", (unsigned)entry->event, (unsigned)in_thread);
        data_bury_msg_pool_entry_free_payload(entry);
        return;
    }
    if (entry->event == DATA_BURY_EVT_PUBLISH) {
        entry->body.publish.val = NULL;
        entry->body.publish.val_in_psram = 0u;
    }
}

static void data_bury_msg_pool_insert(data_bury_manager_context_t *ctx, const data_bury_msg_pool_entry_t *entry,
                                      uint8_t index)
{
    ctx->priv.msg_pool[index] = *entry;
    ctx->priv.msg_pool[index].is_use = 1u;
}

static void data_bury_msg_pool_remove_at(data_bury_manager_context_t *ctx, uint8_t index)
{
    data_bury_msg_pool_entry_t tmp_pool[DATA_BURY_MSG_POOL_SIZE];
    uint16_t cp_len;

    data_bury_msg_pool_entry_free_payload(&ctx->priv.msg_pool[index]);
    if (index + 1u < ctx->priv.msg_cnt) {
        cp_len = (uint16_t)(sizeof(data_bury_msg_pool_entry_t) * (ctx->priv.msg_cnt - (index + 1u)));
        memcpy(tmp_pool, &ctx->priv.msg_pool[index + 1u], cp_len);
        memcpy(&ctx->priv.msg_pool[index], tmp_pool, cp_len);
    }
    ctx->priv.msg_cnt--;
    memset(&ctx->priv.msg_pool[ctx->priv.msg_cnt], 0, sizeof(data_bury_msg_pool_entry_t));
}

static int data_bury_msg_pool_add(data_bury_manager_context_t *ctx, const data_bury_msg_pool_entry_t *entry)
{
    uint32_t target_ticks;
    uint8_t i;
    data_bury_msg_pool_entry_t item;

    if (!ctx || !entry || ctx->priv.msg_cnt >= DATA_BURY_MSG_POOL_SIZE) {
        LOGE("data bury msg pool full, lost event=0x%04x", entry ? (unsigned)entry->event : 0u);
        return -1;
    }

    item = *entry;
    item.is_use = 1u;
    target_ticks = entry->target_ticks;
    if (ctx->priv.msg_cnt == 0u) {
        data_bury_msg_pool_insert(ctx, &item, 0);
        ctx->priv.msg_cnt = 1u;
        return 0;
    }

    for (i = 0; i < ctx->priv.msg_cnt; i++) {
        if (target_ticks < ctx->priv.msg_pool[i].target_ticks) {
            data_bury_msg_pool_entry_t tmp_pool[DATA_BURY_MSG_POOL_SIZE];
            uint16_t cp_len = (uint16_t)(sizeof(data_bury_msg_pool_entry_t) * (ctx->priv.msg_cnt - i));

            memcpy(tmp_pool, &ctx->priv.msg_pool[i], cp_len);
            memcpy(&ctx->priv.msg_pool[i + 1u], tmp_pool, cp_len);
            data_bury_msg_pool_insert(ctx, &item, i);
            ctx->priv.msg_cnt++;
            return 0;
        }
    }

    data_bury_msg_pool_insert(ctx, &item, ctx->priv.msg_cnt);
    ctx->priv.msg_cnt++;
    return 0;
}

void data_bury_msg_mgr_init(data_bury_manager_context_t *ctx)
{
    if (!ctx) {
        return;
    }
    MUTEX_LOCK(ctx->priv.m_lock);
    memset(ctx->priv.msg_pool, 0, sizeof(ctx->priv.msg_pool));
    ctx->priv.msg_cnt = 0;
    ctx->priv.msg_inited = 1u;
    MUTEX_UNLOCK(ctx->priv.m_lock);
}

void data_bury_msg_mgr_clear(data_bury_manager_context_t *ctx)
{
    uint8_t i;

    if (!ctx || !ctx->priv.msg_inited) {
        return;
    }
    MUTEX_LOCK(ctx->priv.m_lock);
    for (i = 0; i < ctx->priv.msg_cnt; i++) {
        data_bury_msg_pool_entry_free_payload(&ctx->priv.msg_pool[i]);
    }
    memset(ctx->priv.msg_pool, 0, sizeof(ctx->priv.msg_pool));
    ctx->priv.msg_cnt = 0;
    MUTEX_UNLOCK(ctx->priv.m_lock);
}

void data_bury_msg_mgr_process_due(data_bury_manager_context_t *ctx, uint8_t in_thread)
{
    uint32_t cur_ticks;
    uint8_t processed;

    if (!ctx || !ctx->priv.msg_inited || !ctx->priv.enabled) {
        return;
    }

    processed = 0;
    cur_ticks = get_timestamp();
    MUTEX_LOCK(ctx->priv.m_lock);
    while (processed < DATA_BURY_MSG_PROCESS_BATCH_MAX && ctx->priv.msg_cnt > 0u) {
        data_bury_msg_pool_entry_t entry;

        if (!ctx->priv.msg_pool[0].is_use) {
            break;
        }
        if (cur_ticks < ctx->priv.msg_pool[0].target_ticks) {
            break;
        }

        entry = ctx->priv.msg_pool[0];
        if (entry.event == DATA_BURY_EVT_PUBLISH) {
            ctx->priv.msg_pool[0].body.publish.val = NULL;
        }
        data_bury_msg_pool_remove_at(ctx, 0);
        MUTEX_UNLOCK(ctx->priv.m_lock);
        data_bury_msg_deliver_entry(ctx, &entry, in_thread);
        processed++;
        if (!ctx->priv.enabled) {
            return;
        }
        MUTEX_LOCK(ctx->priv.m_lock);
        cur_ticks = get_timestamp();
    }
    MUTEX_UNLOCK(ctx->priv.m_lock);
}

void data_bury_sys_msg_remove(data_bury_manager_context_t *ctx, uint16_t event)
{
    int i;

    if (!ctx || !ctx->priv.msg_inited || !data_bury_msg_is_user_event(event)) {
        return;
    }

    MUTEX_LOCK(ctx->priv.m_lock);
    for (i = 0; i < (int)ctx->priv.msg_cnt; i++) {
        if (!ctx->priv.msg_pool[i].is_use) {
            break;
        }
        if (ctx->priv.msg_pool[i].event == event) {
            data_bury_msg_pool_remove_at(ctx, (uint8_t)i);
            i--;
        }
    }
    MUTEX_UNLOCK(ctx->priv.m_lock);
}

int data_bury_sys_msg_put(data_bury_manager_context_t *ctx, uint16_t event, uint32_t delay_ms,
                          data_bury_msg_pool_payload_t *payload)
{
    data_bury_msg_pool_entry_t entry = {0};
    data_bury_msg_pool_payload_t payload_copy = {0};
    int ret;

    if (!ctx || !ctx->priv.enabled) {
        return -1;
    }
    if (!data_bury_msg_is_user_event(event)) {
        LOGE("data bury invalid event=0x%04x", (unsigned)event);
        return -2;
    }

    ret = data_bury_msg_pool_payload_prepare(event, payload, &payload_copy);
    if (ret != 0) {
        LOGE("data bury msg prepare fail event=0x%04x ret=%d", (unsigned)event, ret);
        return ret;
    }

    entry.event = event;
    entry.target_ticks = get_timestamp() + delay_ms;
    if (event == DATA_BURY_EVT_PUBLISH) {
        entry.body.publish = payload_copy;
    }

    if (delay_ms == 0u) {
        if (!ctx->priv.enabled) {
            LOGW("data bury put disabled before dispatch event=0x%04x", (unsigned)event);
            data_bury_msg_pool_entry_free_payload(&entry);
            return -1;
        }
        ret = data_bury_msg_dispatch_entry(ctx, &entry);
        if (ret != 0) {
            data_bury_msg_pool_entry_free_payload(&entry);
        }
        return ret;
    }

    if (!ctx->priv.enabled) {
        LOGW("data bury put disabled before pool add event=0x%04x delay=%u", (unsigned)event,
             (unsigned)delay_ms);
        data_bury_msg_pool_entry_free_payload(&entry);
        return -1;
    }

    MUTEX_LOCK(ctx->priv.m_lock);
    ret = data_bury_msg_pool_add(ctx, &entry);
    MUTEX_UNLOCK(ctx->priv.m_lock);

    if (ret != 0) {
        data_bury_msg_pool_entry_free_payload(&entry);
        return ret;
    }

    if (ctx->priv.msg_cnt >= DATA_BURY_MSG_WARN_NUM) {
        LOGW("data bury msg pool cnt=%u warn>=%u", (unsigned)ctx->priv.msg_cnt,
             (unsigned)DATA_BURY_MSG_WARN_NUM);
    }
    return 0;
}

#endif /* DATA_BURY_MSG_POOL_ENABLE */
