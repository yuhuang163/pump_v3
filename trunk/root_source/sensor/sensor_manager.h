#ifndef __SENSOR_MANAGER_H__
#define __SENSOR_MANAGER_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_api.h" // IWYU pragma: keep

#define SENSOR_THREAD_POLLING_PERIOD         (10)

/// 传感器任务节点类型
typedef struct {
    void *user_data;
    uint32_t polling_period;  /// 轮询间隔(ms)
    uint32_t last_exec_time;  /// 上次执行时间(ms)
    on_sensor_task_cb_fptr cb;
    dispatch_sensor_client_fptr dispatch_client_cb;
    int task_id;
    int report_type;
    list_head_t list;
} sensor_task_node_t;

/// 传感器事件客户端节点类型
typedef struct {
    void *user_data;
    on_sensor_evt_data_cb_fptr cb;
    int report_type;
    int step;
    uint32_t old_cnt;
    list_head_t list;
} sensor_evt_client_node_t;

void *register_module_for_imu_new(list_head_t *list_head);
void *register_module_for_force_sensor_new(list_head_t *list_head);
void *register_module_for_air_pressure_new(list_head_t *list_head);
void *register_module_for_light_sensor_new(list_head_t *list_head);

#ifdef __cplusplus
}
#endif

#endif 







