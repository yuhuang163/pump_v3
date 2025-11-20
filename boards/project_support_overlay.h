#ifndef __PROJECT_SUPPORT_OVERLAY_H__
#define __PROJECT_SUPPORT_OVERLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

/// 设备支持
#define IS_SUPPORT_DEVICE_UART                           (1)
#define IS_SUPPORT_DEVICE_IRQ                            (1)
#define IS_SUPPORT_DEVICE_FLASH                          (1)
#define IS_SUPPORT_DEVICE_DATABASE                       (1)
#define IS_SUPPORT_DEVICE_INPUT_KEY                      (1)
#define IS_SUPPORT_DEVICE_POWER                          (1)
#define IS_SUPPORT_DEVICE_PMU                            (1)
#define IS_SUPPORT_DEVICE_BATTERY                        (1)
#define IS_SUPPORT_DEVICE_IMU                            (0)
#define IS_SUPPORT_DEVICE_FSENSOR                        (0)
#define IS_SUPPORT_DEVICE_PRESSURE_KEY                   (0)
#define IS_SUPPORT_DEVICE_AIR_PRESSURE                   (1)
#define IS_SUPPORT_DEVICE_MOTOR                          (1)
#define IS_SUPPORT_DEVICE_LED                            (1)
#define IS_SUPPORT_DEVICE_AUDIO                          (0)
#define IS_SUPPORT_DEVICE_LCD                            (1)
#define IS_SUPPORT_DEVICE_BLE                            (1)
#define IS_SUPPORT_DEVICE_FS                             (0)
#define IS_SUPPORT_DEVICE_WIFI                           (0)

/// 模块支持
#define IS_SUPPORT_MODULE_PBUS                           (1)
#define IS_SUPPORT_MODULE_PM                             (1)
#define IS_SUPPORT_MODULE_STORAGE                        (1)
#define IS_SUPPORT_MODULE_RES                            (0)
#define IS_SUPPORT_MODULE_TIME_SYSTEM                    (1)
#define IS_SUPPORT_MODULE_WM                             (1)
#define IS_SUPPORT_MODULE_UI                             (1)
#define IS_SUPPORT_MODULE_KEY_INPUT                      (1)
#define IS_SUPPORT_MODULE_SENSOR                         (1)
#define IS_SUPPORT_MODULE_BLE                            (1)
#define IS_SUPPORT_MODULE_OTA                            (0)
#define IS_SUPPORT_MODULE_MISC                           (1)
#define IS_SUPPORT_MODULE_MOTOR                          (1)
#define IS_SUPPORT_MODULE_BREAST_PUMP                    (1)
#define IS_SUPPORT_MODULE_TESTS                          (1)
#define IS_SUPPORT_MODULE_US_ALGO                        (1)
#define IS_SUPPORT_MODULE_AUDIO                          (0)
#define IS_SUPPORT_MODULE_AIRPLANE_MODE                  (1)
#define IS_SUPPORT_MODULE_DATA_BURIAL                    (0)
#define IS_SUPPORT_WEEKLY_STAT                           (0)
#define IS_SUPPORT_MODULE_PROTOCOL                       (1)
/// 日志功能
#define IS_SUPPORT_LOG_OUT                               (1)
#define IS_SUPPORT_LOG_COLOR                             (0)
/// 叠图功能
#define IS_SUPPORT_OVERLAY_IMG                           (1)
#define IS_SUPPORT_ASYNC_RESET_UI_PLAY_INFO              (1)
/// 振动马达震动功能
#define IS_SUPPORT_MOTOR_SHAKE                           (0)
/// 使用在线陀螺仪校准
#define IS_USE_GYRO_ONLINE_CALI                          (0) 
/// 使用IMU FIFO
#define IS_SUPPORT_IMU_FIFO                              (0)
/// 测试支持
#define IS_SUPPORT_PRESSURE_TEST                         (1)
/// 支持爬升策略
#define IS_SUPPORT_CLIMB_POLICY                          (1)
/// 支持算法采集
#define IS_SUPPORT_ALGO_SAMPLING                         (1)
#ifdef __cplusplus
}
#endif

#endif


