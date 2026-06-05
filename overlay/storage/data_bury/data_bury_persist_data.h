#ifndef __DATA_BURY_PERSIST_DATA_H__
#define __DATA_BURY_PERSIST_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_api.h"
#include "storage_manager.h"
#include "data_bury_persist_defs.h"

/* Flash 元数据 / 环区分片（模式同 breast_pump_persist_data head/body） */
void reset_data_bury_ring_meta(storage_manager_context_t *ctx, data_bury_ring_meta_t *meta);
int write_data_bury_ring_meta_to_flash(store_func_args_t args);
int read_data_bury_ring_meta_from_flash(store_func_args_t args);

int write_data_bury_ring_chunk_to_flash(store_func_args_t args);
int read_data_bury_ring_chunk_from_flash(store_func_args_t args);

/* RAM 环：启动加载、整环落盘、清空 */
int data_bury_ring_storage_init(storage_manager_context_t *ctx);
int data_bury_ring_storage_reset(storage_manager_context_t *ctx);
int data_bury_ring_sync_to_flash(storage_manager_context_t *ctx);

/*
 * RAM 环业务接口（store_func_args_t，经 put/get_store_ram_val 分发）
 * 用法同 write_pump_time_to_ram / read_pump_time_from_ram
 */
/** val: data_bury_ring_append_args_t * */
int write_data_bury_ring_event_to_ram(store_func_args_t args);
/** val: uint16_t * 待上报条数 */
int read_data_bury_ring_pending_count_from_ram(store_func_args_t args);
/** val 可 NULL；发送成功后 pop 最旧一条 */
int write_data_bury_ring_pop_oldest_to_ram(store_func_args_t args);
/** val: data_bury_ring_peek_args_t *；payload 可为 NULL 仅读 hdr */
int read_data_bury_ring_peek_oldest_from_ram(store_func_args_t args);

int data_bury_ring_is_empty(storage_manager_context_t *ctx);

#define WRITE_DATA_BURY_RING_APPEND(ctx, append_args) \
    (ctx)->put_store_ram_val(STORE_ID_DATA_BURY_RING_APPEND, 0, 0, (void *)(append_args))

#define READ_DATA_BURY_RING_PENDING_COUNT(ctx, count_ptr) \
    (ctx)->get_store_ram_val(STORE_ID_DATA_BURY_RING_PENDING_COUNT, 0, 0, (void *)(count_ptr))

#define WRITE_DATA_BURY_RING_POP_OLDEST(ctx) \
    (ctx)->put_store_ram_val(STORE_ID_DATA_BURY_RING_POP_OLDEST, 0, 0, NULL)

#define READ_DATA_BURY_RING_PEEK_OLDEST(ctx, peek_args) \
    (ctx)->get_store_ram_val(STORE_ID_DATA_BURY_RING_PEEK_OLDEST, 0, 0, (void *)(peek_args))

#ifdef __cplusplus
}
#endif

#endif /* __DATA_BURY_PERSIST_DATA_H__ */
