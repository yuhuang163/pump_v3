#ifndef __PROJECT_CONFIG_OVERLAY_H__
#define __PROJECT_CONFIG_OVERLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

///  各关键业务线程栈深度, 默认线程单位栈深1KB
#define TASK_SIZE_DEFAULT                              (1024)
/// 主业务线程栈深度
#define MAIN_THREAD_STACKSIZE                          (TASK_SIZE_DEFAULT * 5 + 0)
/// 投递线程栈深度
#define HDR_THREAD_STACKSIZE                           (TASK_SIZE_DEFAULT * 3 + 0)
/// 广播线程栈深度
#define BCR_THREAD_STACKSIZE                           (TASK_SIZE_DEFAULT * 3 + 0)
/// 马达线程深度
#define MOTOR_PLAY_THREAD_STACKSIZE                    (TASK_SIZE_DEFAULT * 2 + 0)
/// 蓝牙线程深度
#define BLE_THREAD_STACKSIZE                           (TASK_SIZE_DEFAULT * 2 + 0)
/// 吸奶线程深度
#define BTH_THREAD_STACKSIZE                           (TASK_SIZE_DEFAULT * 2 + 0)
/// lvgl线程深度
#define LVGL_RENDER_THREAD_STACKSIZE                   (TASK_SIZE_DEFAULT * 2 + 0)
/// 窗口线程深度
#define WM_THREAD_STACKSIZE                            (TASK_SIZE_DEFAULT * 2 + 0)
/// ota线程深度
#define OTA_THREAD_STACKSIZE                           (TASK_SIZE_DEFAULT * 2 + 0) 
/// 按键线程深度
#define INPUT_KEY_THREAD_STACKSIZE                     (TASK_SIZE_DEFAULT * 2 + 0)
/// 传感器线程深度
#define INPUT_SENSOR_THREAD_STACKSIZE                  (TASK_SIZE_DEFAULT * 10 + 0)
/// 死锁检查线程深度
#define HG_CHECK_THREAD_STACKSIZE                      (TASK_SIZE_DEFAULT * 2 + 0)
/// 死锁检测线程深度
#define HG_DETECT_THREAD_STACKSIZE                     (TASK_SIZE_DEFAULT * 2 + 0)
/// 杂项线程深度
#define MISC_THREAD_STACKSIZE                          (TASK_SIZE_DEFAULT * 2 + 0)
/// 挂起线程深度
#define PM_SUSPEND_THREAD_STACKSIZE                    (TASK_SIZE_DEFAULT * 2 + 0)
/// 冻结线程深度 
#define PM_FREEZE_THREAD_STACKSIZE                     (TASK_SIZE_DEFAULT * 2 + 0)
/// 中断线程深度
#define INTERRUPT_THREAD_STACKSIZE                     (TASK_SIZE_DEFAULT * 1 + 0)
/// 马达驱动线程深度
#define MOTOR_DRIVER_THREAD_STACKSIZE                  (TASK_SIZE_DEFAULT * 2 + 0)

/// 中断消息队列数量定义
#define INTERRUPT_MESQ_NUM                             (10)
/// lcd spi 消息队列数量定义
#define LCD_SPI_MESQ_NUM                               (1)
/// led qspi 消息队列数量定义
#define LED_QSPI_MESQ_NUM                              (1)
/// adc 消息队列数量定义
#define ADC_MESQ_NUM                                   (1)
/// 音频流控消息队列数量定义
#define PWM_FLOW_MESQ_NUM                              (2)
/// 波形发生器消息队列数量定义
#define WAVE_GENERATOR_MESQ_NUM                        (1)
/// 挂起超时
#define PM_SUSPEND_TIME_S                              (10)

/// 文件名长度
#define ROOT_FILENAME_LEN                              128

/// 力传感器周期计数
#define FSENSOR_CYCLE_CNT                              (50)
/// IMU fifo大小
#define US_IMU_FIFO_SIZE                               (160)
/// IMU数据间隔
#define US_IMU_DATA_INTERVAL                           (20)

#ifdef __cplusplus
}
#endif

#endif



