#ifndef __DATA_BURY_PERSIST_DATA_H__
#define __DATA_BURY_PERSIST_DATA_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "root_common_api.h"
#include "storage_manager.h"
#include "data_bury_persist_defs.h"

/* Flash 槽位索引 / 元数据 / 环区分片 */
int write_data_bury_flash_bank_to_flash(store_func_args_t args);
int read_data_bury_flash_bank_from_flash(store_func_args_t args);

void reset_data_bury_ring_meta(storage_manager_context_t *ctx, uint8_t slot, data_bury_ring_meta_t *meta);
int write_data_bury_ring_meta_to_flash(store_func_args_t args);
int read_data_bury_ring_meta_from_flash(store_func_args_t args);

int write_data_bury_use_session_boot_seq_to_flash(store_func_args_t args);
int read_data_bury_use_session_boot_seq_from_flash(store_func_args_t args);

int write_data_bury_ring_chunk_to_flash(store_func_args_t args);
int read_data_bury_ring_chunk_from_flash(store_func_args_t args);

/* 埋点持久化信息 + RAM 环：启动加载（val: data_bury_info_t *） */
int data_bury_infor_storage_init(store_func_args_t args);
/* RAM 环惰性初始化（仅环区，不读 boot_seq） */
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

#ifdef __cplusplus
}
#endif

#endif /* __DATA_BURY_PERSIST_DATA_H__ */
