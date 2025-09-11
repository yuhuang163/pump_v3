#ifndef __SENSOR_MANAGER_API_H__
#define __SENSOR_MANAGER_API_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

/// 传感器事件类型
typedef enum {
    IMU_EVT_POLLING = 0,
    IMU_EVT_IRQ,
    F_SENSOR_EVT_POLLING,
} imu_report_type_e;

/// 中断事件类型
typedef enum {
    IRQ_EVENT_CB_AIRPLANE = 0,
    IRQ_EVENT_CB_IOT,
    IRQ_EVENT_CB_CHARGE, 
    IRQ_EVENT_CB_MAX
} irq_event_cb_e;

/// 传感器事件回调函数
typedef void (*on_sensor_evt_data_cb_fptr)(void *user_data, uint8_t *data, int data_len);
/// 中断事件回调函数
typedef void (*on_imu_irq_callback_fptr)(void *user_data);

struct rt_device_node;
struct broadcast_obj;
typedef struct input_dispatcher_context {
/// private
    struct __sensor_manager_private {
        /// 传感器.
        uint16_t do_polling;
        os_mutex_id m_lock;
        thread_context_t thd;
        list_head_t client_list;
        on_imu_irq_callback_fptr irq_client_cb[IRQ_EVENT_CB_MAX];
        void *irq_client_token[IRQ_EVENT_CB_MAX];
        os_timer_id polling_timer;
        struct broadcast_obj *bcd;
        struct rt_device_node *sensor;
        bool imu_irq_finished;
        uint32_t imu_polling_cnt;
        uint32_t f_sensor_polling_cnt;
    } priv;

/// public
    /// 传感器接口.
    void (*add_sensor_evt_lisenser)(void *user_data, int report_type, int step, on_sensor_evt_data_cb_fptr lisenser);
    void (*remove_sensor_evt_lisenser)(on_sensor_evt_data_cb_fptr lisenser);
    const imu_cali_data_t *(*get_imu_cali_data)(void);
    int (*write_imu_calibration)(const imu_cali_data_t *data);
    void (*add_imu_irq_callback)(uint8_t irq_num, void *token, on_imu_irq_callback_fptr cb);
    void (*remove_imu_irq_callback)(uint8_t irq_num, void *token);
    void (*imu_irq_enable)(void *context);
    void (*imu_irq_disable)(void *context);
    int (*is_on_running)(void);
} sensor_manager_context_t;


#ifdef __cplusplus
}
#endif

#endif 


