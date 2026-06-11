#ifndef __MOTOR_MANAGER_H__
#define __MOTOR_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

/// 马达状态事件客户端节点类型
typedef struct {
    void *user_data;
    on_motor_status_evt_cb_fptr cb;
    list_head_t list;
} motor_status_evt_client_node_t;

/*
* INTERNAL FUNCTION DECLARATION
*****************************************************************************************
*/
void motor_policy_init(struct motor_manager_context *thiz);
int change_motor_mode(struct motor_manager_context *thiz, uint8_t mode, uint8_t is_continue);
#ifdef __cplusplus
}
#endif

#endif 







