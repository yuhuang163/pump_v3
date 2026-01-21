#ifndef __IMU_MODULE_H__
#define __IMU_MODULE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

#if IS_SUPPORT_IMU_FIFO
#define IMU_POLLING_PERIOD                   (200)
#define LOG_IMU_FIFO_PERIOD                  (260)
#else
#define IMU_POLLING_PERIOD                   (20)
#define LOG_IMU_FIFO_PERIOD                  (500)
#endif

/// 中断事件类型
typedef enum {
    IMU_IRQ_EVENT_CB_AIRPLANE = 0,
    IMU_IRQ_EVENT_CB_IOT,
    IMU_IRQ_EVENT_CB_CHARGE, 
    IMU_IRQ_EVENT_CB_MAX
} imu_irq_event_cb_e;

/// 中断事件回调函数
typedef void (*on_imu_irq_callback_fptr)(void *user_data);

struct rt_device_node;
/// IMU传感器上下文
typedef struct imu_sensor_context {
    /// private
    struct __imu_sensor_private {
        on_imu_irq_callback_fptr irq_client_cb[IMU_IRQ_EVENT_CB_MAX];
        void *irq_client_token[IMU_IRQ_EVENT_CB_MAX];
        struct rt_device_node *dev;
        bool irq_finished;
        uint32_t polling_cnt;
    } priv;

    /// public
    const imu_cali_data_t *(*get_cali_data)(void);
    int (*write_calibration)(const imu_cali_data_t *data);
    void (*add_irq_callback)(uint8_t irq_num, void *token, on_imu_irq_callback_fptr cb);
    void (*remove_irq_callback)(uint8_t irq_num, void *token);
    void (*irq_enable)(void *context);
    void (*irq_disable)(void *context);
} imu_sensor_context_t;

#ifdef __cplusplus
}
#endif

#endif 







