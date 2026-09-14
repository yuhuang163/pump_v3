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
#define BLE_THREAD_STACKSIZE                           (TASK_SIZE_DEFAULT * 4 + 0)
/// 吸奶线程深度
#define BST_THREAD_STACKSIZE                           (TASK_SIZE_DEFAULT * 3 + 0)
/// lvgl线程深度
#if CFG_LVGL_ENABLE
#define LVGL_RENDER_THREAD_STACKSIZE                   (TASK_SIZE_DEFAULT * 16 + 0)
#else
#define LVGL_RENDER_THREAD_STACKSIZE                   (TASK_SIZE_DEFAULT * 2 + 0)
#endif
/// 窗口线程深度
#if CFG_LVGL_ENABLE
#define WM_THREAD_STACKSIZE                            (TASK_SIZE_DEFAULT * 4 + 0)
#else
#define WM_THREAD_STACKSIZE                            (TASK_SIZE_DEFAULT * 2 + 0)
#endif
/// ota线程深度
#define OTA_THREAD_STACKSIZE                           (TASK_SIZE_DEFAULT * 2 + 0) 
/// 按键线程深度
#define INPUT_KEY_THREAD_STACKSIZE                     (TASK_SIZE_DEFAULT * 2 + 0)
/// 传感器线程深度
#define INPUT_SENSOR_THREAD_STACKSIZE                  (TASK_SIZE_DEFAULT * 2 + 0)
/// 产品测试线程深度
#define INPUT_PRODUCT_TESTS_THREAD_STACKSIZE           (TASK_SIZE_DEFAULT * 2 + 0)
/// 数据埋点线程深度
#define INPUT_DATA_BURY_THREAD_STACKSIZE               (TASK_SIZE_DEFAULT * 4 + 0)
/// 死锁检查线程深度
#define HG_CHECK_THREAD_STACKSIZE                      (TASK_SIZE_DEFAULT * 2 + 0)
/// 死锁检测线程深度
#define HG_DETECT_THREAD_STACKSIZE                     (TASK_SIZE_DEFAULT * 2 + 0)
/// 杂项线程深度
#define MISC_THREAD_STACKSIZE                          (TASK_SIZE_DEFAULT * 4 + 0)
/// 挂起线程深度
#define PM_SUSPEND_THREAD_STACKSIZE                    (TASK_SIZE_DEFAULT * 4 + 0)
/// 冻结线程深度 
#define PM_FREEZE_THREAD_STACKSIZE                     (TASK_SIZE_DEFAULT * 2 + 0)
/// 中断线程深度
#define INTERRUPT_THREAD_STACKSIZE                     (TASK_SIZE_DEFAULT * 2 + 0)
/// 马达驱动线程深度
#define MOTOR_DRIVER_THREAD_STACKSIZE                  (TASK_SIZE_DEFAULT * 4 + 0)
/// 工厂串口线程深度
#define FACTORY_UART_THREAD_STACKSIZE                  (TASK_SIZE_DEFAULT * 2 + 0)
/// 按键设备线程深度
#define KEY_DEVICE_THREAD_STACKSIZE                    (TASK_SIZE_DEFAULT * 2 + 0)
/// 充电设备线程深度
#define CHARGE_DEVICE_THREAD_STACKSIZE                 (TASK_SIZE_DEFAULT * 2 + 0)
/// led设备线程深度
#define LED_DEVICE_THREAD_STACKSIZE                    (TASK_SIZE_DEFAULT * 2 + 0)
/// lcd线程深度
#define LCD_THREAD_STACKSIZE                           (TASK_SIZE_DEFAULT * 2 + 0)
/// 服务器线程深度
#define SERVERS_THREAD_STACKSIZE                       (TASK_SIZE_DEFAULT * 4 + 0)
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
/// 按键设备消息队列数量定义
#define KEY_DEVICE_MESQ_NUM                            (1)
/// 服务器消息队列数量定义
#define SERVERS_WORK_DISPATCH_MESQ_NUM                 (10)
/// 挂起超时
#if 1
#define PM_SUSPEND_TIME_S                              (15 * 60 * 1000) /// 15分钟
#define PM_IDLE_TIME_S                                 (12 * 60 * 1000) /// 12分钟
#define PM_SUSPEND_LOW_BAT_TIME_S                      (1 * 15 * 1000) /// 10秒
#define PM_SUSPEND_WILL_ENTER_TIME_S                   (3000) /// 3秒
#else
#define PM_SUSPEND_TIME_S                              (2 * 15 * 1000) /// 1分
#define PM_IDLE_TIME_S                                 (1 * 15 * 1000) /// 30秒
#define PM_SUSPEND_LOW_BAT_TIME_S                      (2 * 15 * 1000) /// 1分
#define PM_SUSPEND_WILL_ENTER_TIME_S                   (3000) /// 3秒
#endif
/// 屏幕串口消息队列数量定义
#define LCD_UART_MESQ_NUM                              (1)
/// 文件名长度
#define ROOT_FILENAME_LEN                              128
/// 力传感器周期计数
#define FSENSOR_CYCLE_CNT                              (50)
/// IMU fifo大小
#define US_IMU_FIFO_SIZE                               (160)
/// IMU数据间隔
#define US_IMU_DATA_INTERVAL                           (20)
/// 吸奶器泵阀配置
#define BREAST_PUMP_NUM                                (2)
#define BREAST_PUMP_VALVE_FREQ                         (1000)
#define BREAST_PUMP_VALVE_DUTY                         (100)
#define BREAST_PUMP_PUMP_FREQ                          (1000)
/// 叠图最大数量
#define OVERLAY_IMG_NUM_MAX                            (2)
/// 气压传感器数量
#define AIR_PRESSURE_NUM                               (2)
/// LVGL定时器周期
#define LVGL_TIME_TICK_PERIOD                          (20) /// 单位ms
/// 开机长按时间
#define BOOT_LONG_PRESS_TIME                           (3000)
/// 屏幕宽度
#define PANLE_DISPLAY_W                                (360)
/// 屏幕高度
#define PANLE_DISPLAY_H                                (360)
/// 资源存储类型
#define IMG_RES_STORAGE_INTERNAL_FLASH                 0
#define IMG_RES_STORAGE_EXTERNAL_FLASH                 1
#define V3_IMG_RES_STORAGE_TYPE                        1
/// 操作失败弹窗展示时间
#define OPERATION_FAILURE_DISPLAY_TIMEOUT              (1000) /// 1s
/// 操作成功弹窗展示时间
#define OPERATION_SUCCESS_DISPLAY_TIMEOUT              (1000) /// 1s
/// LED快闪时间
#define LED_FAST_BLINK_TIME                            (100)  /// 100ms
/// LED中速闪时间
#define LED_MEDIUM_BLINK_TIME                          (200)  /// 200ms
/// LED慢闪时间
#define LED_SLOW_BLINK_TIME                            (500) /// 1000ms
/// 按键灯呼吸完整周期（灭→亮→灭），单位ms
#define LED_BREATH_CYCLE_TIME                           (4000)
/// 关机提醒时间
#define POWER_OFF_REMINDER_TIME                        (3000) /// 3秒
/// OTA单块原数据大小
#define OTA_BLOCK_SIZE                                 (4096)
/// 默认力度值
#define DEFAULT_LEVEL_VALUE                            (1)
/// 默认速度值
#define DEFAULT_SPEED_VALUE                            (0)
/// IDLE 状态时，亮度最大值
#define IDLE_BRIGHTNESS_MAX_VALUE                      (50)
#ifdef __cplusplus
}
#endif

#endif



