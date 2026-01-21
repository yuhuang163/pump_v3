#ifndef __SENSOR_MANAGER_API_H__
#define __SENSOR_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "broadcast_manager_api.h"
#include "root_message.h"
#if IS_SUPPORT_DEVICE_IMU
#include "imu_module.h" // IWYU pragma: keep
#endif
#if IS_SUPPORT_DEVICE_FSENSOR
#include "force_sensor_module.h" // IWYU pragma: keep
#endif
#if IS_SUPPORT_DEVICE_AIR_PRESSURE
#include "air_pressure_module.h" // IWYU pragma: keep
#endif
#if IS_SUPPORT_DEVICE_LIGHT_SENSOR
#include "light_sensor_module.h" // IWYU pragma: keep
#endif

/// 定义各个传感器id.
typedef enum {
    ID_SENSOR_BEGIN = 0,
    ID_SENSOR_IMU,
    ID_SENSOR_FSENSOR,
    ID_SENSOR_AIR_PRESSURE,
    ID_SENSOR_LIGHT_SENSOR,
    ID_SENSOR_MAX
} sensor_module_desc_id_e;

/// 描述每个传感器对象节点.
typedef struct sensor_module_node {
    const char *name;
    sensor_module_desc_id_e id;
    int (*post_init)(void *ptr);
    void *module_ptr;
    uint32_t mem_usage;
    list_head_t list;
} sensor_module_node_t;

typedef void *(*sensor_register_fptr)(list_head_t *list_head);

/// 传感器事件类型
typedef enum {
    IMU_EVT_POLLING = 0,
    IMU_EVT_IRQ,
    F_SENSOR_EVT_POLLING,
    AIR_PRESSURE_EVT_POLLING,
    LIGHT_SENSOR_EVT_POLLING,
} sensor_report_type_e;

/// 传感器任务ID类型
typedef enum {
    SENSOR_TASK_ID_IMU_POLLING = 0,
    SENSOR_TASK_ID_FSENSOR_POLLING = 1,
    SENSOR_TASK_ID_AIR_PRESSURE_POLLING = 2,
    SENSOR_TASK_ID_LIGHT_SENSOR_POLLING = 3,
    SENSOR_TASK_ID_MAX
} sensor_task_id_e;

/// 传感器任务回调函数类型
typedef void (*on_sensor_task_cb_fptr)(void *user_data);

/// 传感器事件回调函数
typedef void (*on_sensor_evt_data_cb_fptr)(void *user_data, uint8_t *data, int data_len);

/// 传感器事件分发函数类型
typedef void (*dispatch_sensor_client_fptr)(sensor_wrapper_data_t *sensor_data,
     void *user_data);

struct broadcast_obj;
typedef struct sensor_manager_context {
/// private
    struct __sensor_manager_private {
        /// 传感器.
        uint16_t do_polling;
        os_mutex_id m_lock;
        thread_context_t thd;
        struct broadcast_obj *bcd;
        list_head_t client_list;
        list_head_t task_list;
        os_timer_id polling_timer;
    } priv;

/// public
    /// 传感器接口.
    void (*add_sensor_evt_lisenser)(void *user_data, int report_type, int step, on_sensor_evt_data_cb_fptr lisenser);
    void (*remove_sensor_evt_lisenser)(on_sensor_evt_data_cb_fptr lisenser);
    void (*add_sensor_task)(void *user_data, int task_id, uint32_t polling_period,
        int report_type, on_sensor_task_cb_fptr cb, dispatch_sensor_client_fptr dispatch_client_cb);
    void (*remove_sensor_task)(on_sensor_task_cb_fptr cb);
    const void *(*find_sensor)(sensor_module_desc_id_e id);
    int (*is_on_running)(void);
} sensor_manager_context_t;

/// 添加传感器.
void *add_new_sensor(const char *name, sensor_module_desc_id_e id, 
    int (*post_init_cb)(void *ptr), void *ptr, list_head_t *list_head, uint32_t mem_usage);
#define ADD_SENSOR(name, id, post_init_cb, ptr, list_head, mem_usage) \
    add_new_sensor(name, id, post_init_cb, ptr, list_head, mem_usage)

#ifdef __cplusplus
}
#endif

#endif 


