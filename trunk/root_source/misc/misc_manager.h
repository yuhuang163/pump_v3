#ifndef __MISC_MANAGER_H__
#define __MISC_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

/// 杂项任务节点结构
typedef struct {
    void *user_data;
    int task_id;
    uint32_t polling_period;  /// 轮询间隔(ms)
    uint32_t last_exec_time;  /// 上次执行时间(ms)
    misc_work_msg_id_mask_t msg_id_mask;  /// 消息ID位掩码，表示该任务关心哪些消息ID
    on_misc_task_cb_fptr cb;  /// 轮询回调函数
    list_head_t list;
} misc_task_node_t;

#ifdef __cplusplus
}
#endif

#endif 


