#ifndef __DATA_BURY_MSG_MANAGER_H__
#define __DATA_BURY_MSG_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

#include "data_bury_manager_api.h"

#ifndef DATA_BURY_MSG_POOL_ENABLE
#define DATA_BURY_MSG_POOL_ENABLE                   1
#endif

#if DATA_BURY_MSG_POOL_ENABLE

void data_bury_msg_mgr_init(data_bury_manager_context_t *ctx);
void data_bury_msg_mgr_clear(data_bury_manager_context_t *ctx);
void data_bury_msg_mgr_process_due(data_bury_manager_context_t *ctx, uint8_t in_thread);
void data_bury_sys_msg_remove(data_bury_manager_context_t *ctx, uint16_t event);
int data_bury_sys_msg_put(data_bury_manager_context_t *ctx, uint16_t event, uint32_t delay_ms,
                          data_bury_msg_pool_payload_t *payload);

#endif /* DATA_BURY_MSG_POOL_ENABLE */

#ifdef __cplusplus
}
#endif

#endif /* __DATA_BURY_MSG_MANAGER_H__ */
