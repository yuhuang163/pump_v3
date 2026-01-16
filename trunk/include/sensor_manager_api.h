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

/// 传感器事件类型
typedef enum {
    IMU_EVT_POLLING = 0,
    IMU_EVT_IRQ,
    F_SENSOR_EVT_POLLING,
    AIR_PRESSURE_EVT_POLLING,
    LIGHT_SENSOR_EVT_POLLING,
} sensor_report_type_e;

/// 传感器事件回调函数
typedef void (*on_sensor_evt_data_cb_fptr)(void *user_data, uint8_t *data, int data_len);

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
        os_timer_id polling_timer;
        #if IS_SUPPORT_DEVICE_IMU  
        imu_module_data_t imu;
        #endif
        #if IS_SUPPORT_DEVICE_FSENSOR
        force_sensor_module_data_t f_sensor;
        #endif
        #if IS_SUPPORT_DEVICE_AIR_PRESSURE
        air_pressure_module_data_t air_pressure;
        #endif
        #if IS_SUPPORT_DEVICE_LIGHT_SENSOR
        light_sensor_module_data_t light_sensor;
        #endif
    } priv;

/// public
    /// 传感器接口.
    void (*add_sensor_evt_lisenser)(void *user_data, int report_type, int step, on_sensor_evt_data_cb_fptr lisenser);
    void (*remove_sensor_evt_lisenser)(on_sensor_evt_data_cb_fptr lisenser);
    int (*is_on_running)(void);
    /// 外部钩子函数.
    struct __sensor_manager_hook {
        int (*on_perpare_init_route)(struct sensor_manager_context *ctx);
        int (*on_post_init_route)(struct sensor_manager_context *ctx);
        /// 广播路由
        int (*on_sensor_receiver_route)(broadcast_mask_id_t id, void *context, int arg1, int arg2, void *user_data);
        /// 线程路由
        int (*sensor_thread_route)(struct sensor_manager_context *ctx, sensor_wrapper_data_t *sensor_data);
    } *hook;
    #if IS_SUPPORT_DEVICE_IMU
    imu_module_context_t imu;
    #endif
} sensor_manager_context_t;


#ifdef __cplusplus
}
#endif

#endif 


