#ifndef __PROJECT_SUPPORT_OVERLAY_H__
#define __PROJECT_SUPPORT_OVERLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

/// 模块支持
#define IS_SUPPORT_MODULE_PBUS                           (1)
#define IS_SUPPORT_MODULE_PM                             (1)
#define IS_SUPPORT_MODULE_STORAGE                        (1)
#define IS_SUPPORT_MODULE_TIME_SYSTEM                    (1)
#define IS_SUPPORT_MODULE_WM                             (1)
#define IS_SUPPORT_MODULE_UI                             (1)
#define IS_SUPPORT_MODULE_KEY_INPUT                      (1)
#define IS_SUPPORT_MODULE_SENSOR                         (1)
#define IS_SUPPORT_MODULE_BLE                            (1)
#define IS_SUPPORT_MODULE_OTA                            (1)
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
#define IS_SUPPORT_MODULE_SERVERS                        (1)
#define IS_SUPPORT_MODULE_BATTERY                        (1)

/// 设备支持
#if IS_SUPPORT_MODULE_BATTERY
#define IS_SUPPORT_DEVICE_BATTERY                        (1)
#else
#define IS_SUPPORT_DEVICE_BATTERY                        (0)
#endif
#define IS_SUPPORT_DEVICE_IRQ                            (1)
#define IS_SUPPORT_DEVICE_FLASH                          (1)
#define IS_SUPPORT_DEVICE_DATABASE                       (1)
#define IS_SUPPORT_DEVICE_INPUT_KEY                      (1)
#define IS_SUPPORT_DEVICE_TOUCH_KEY                      (1)
#define IS_SUPPORT_DEVICE_POWER                          (1)
#define IS_SUPPORT_DEVICE_PMU                            (1)
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
/// 长按软复位
#define IS_SUPPORT_KEY_SOFT_RESET                        (1)
/// 支持LVGL
#if CFG_LVGL_ENABLE
#define IS_SUPPORT_LVGL                                  (1)
#else
#define IS_SUPPORT_LVGL                                  (0)
#endif

#if (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V1)
/// 支持PD充电
#define IS_SUPPORT_PD_CHARGE                             (1)
/// 支持旋钮
#define IS_SUPPORT_DEVICE_KNOB                           (1)
/// 支持电池直采
#define IS_SUPPORT_BATTERY_DIRECT_SAMPLING               (0)
/// 支持UART
#define IS_SUPPORT_DEVICE_UART                           (1)
/// 支持光传感器
#define IS_SUPPORT_DEVICE_LIGHT_SENSOR                   (1)
/// 支持资源管理
#define IS_SUPPORT_MODULE_RES                            (1)
/// 支持自适应背光  
#define IS_SUPPORT_ADAPTIVE_DIMMING                      (1)
/// 支持硬件版本自适应
#define IS_SUPPORT_HARDWARE_VERSION_ADAPTIVE             (0)
/// 支持漏气提醒
#define IS_SUPPORT_AIR_LEAKAGE_WARNING                   (0)
#elif (V3_HARDWARE_VERSION == V3_2710IMC_HARDWARE_V2)
/// 支持PD充电
#define IS_SUPPORT_PD_CHARGE                             (1)
/// 支持旋钮
#define IS_SUPPORT_DEVICE_KNOB                           (1)
/// 支持电池直采
#define IS_SUPPORT_BATTERY_DIRECT_SAMPLING               (0)
/// 支持UART
#define IS_SUPPORT_DEVICE_UART                           (1)
/// 支持光传感器
#define IS_SUPPORT_DEVICE_LIGHT_SENSOR                   (1)
/// 支持资源管理
#define IS_SUPPORT_MODULE_RES                            (1)
/// 支持自适应背光  
#define IS_SUPPORT_ADAPTIVE_DIMMING                      (1)
/// 支持硬件版本自适应
#define IS_SUPPORT_HARDWARE_VERSION_ADAPTIVE             (1)
/// 支持漏气提醒
#define IS_SUPPORT_AIR_LEAKAGE_WARNING                   (0)
#else
/// 支持PD充电
#define IS_SUPPORT_PD_CHARGE                             (0)
/// 支持旋钮
#define IS_SUPPORT_DEVICE_KNOB                           (0)
/// 支持电池直采
#define IS_SUPPORT_BATTERY_DIRECT_SAMPLING               (1)
/// 支持UART
#define IS_SUPPORT_DEVICE_UART                           (1)
/// 支持光传感器
#define IS_SUPPORT_DEVICE_LIGHT_SENSOR                   (0)
/// 支持资源管理
#define IS_SUPPORT_MODULE_RES                            (0)
/// 支持自适应背光  
#define IS_SUPPORT_ADAPTIVE_DIMMING                      (0)
/// 支持漏气提醒
#define IS_SUPPORT_AIR_LEAKAGE_WARNING                   (0)
#endif

#ifdef __cplusplus
}
#endif

#endif


