#ifndef __PROJECT_SUPPORT_OVERLAY_H__
#define __PROJECT_SUPPORT_OVERLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

// device
#define IS_SUPPORT_DEVICE_LOG_UART                       (0)
#define IS_SUPPORT_DEVICE_IRQ                            (1)
#define IS_SUPPORT_DEVICE_FLASH                          (1)
#define IS_SUPPORT_DEVICE_DATABASE                       (1)
#define IS_SUPPORT_DEVICE_INPUT_KEY                      (1)
#define IS_SUPPORT_DEVICE_POWER                          (1)
#define IS_SUPPORT_DEVICE_PMU                            (1)
#define IS_SUPPORT_DEVICE_BATTERY                        (1)
#define IS_SUPPORT_DEVICE_IMU                            (1)
#define IS_SUPPORT_DEVICE_AIR_PRESSURE                   (1)
#define IS_SUPPORT_DEVICE_MOTOR                          (1)
#define IS_SUPPORT_DEVICE_LED                            (1)
#define IS_SUPPORT_DEVICE_AUDIO                          (0)
#define IS_SUPPORT_DEVICE_LCD                            (1)
#define IS_SUPPORT_DEVICE_FSENSOR                        (1)
#define IS_SUPPORT_DEVICE_PRESSURE_KEY                   (0)
#define IS_SUPPORT_DEVICE_BLE                            (0)
#define IS_SUPPORT_DEVICE_FS                             (0)
#define IS_SUPPORT_DEVICE_WIFI                           (0)

// module
#define IS_SUPPORT_LOG_OUT                               (1)
#define IS_SUPPORT_LOG_COLOR                             (0)
#define IS_SUPPORT_MODULE_PBUS                           (1)
#define IS_SUPPORT_MODULE_PM                             (0)
#define IS_SUPPORT_MODULE_STORAGE                        (0)
#define IS_SUPPORT_MODULE_RES                            (0)
#define IS_SUPPORT_MODULE_TIME_SYSTEM                    (1)
#define IS_SUPPORT_MODULE_WM                             (0)
#define IS_SUPPORT_MODULE_KEY_INPUT                      (0)
#define IS_SUPPORT_MODULE_SENSOR                         (0)
#define IS_SUPPORT_MODULE_BLE                            (0)
#define IS_SUPPORT_MODULE_OTA                            (0)
#define IS_SUPPORT_MODULE_MISC                           (0)
#define IS_SUPPORT_MODULE_MOTOR                          (0)
#define IS_SUPPORT_MODULE_BREATH_PUMP                    (0)
#define IS_SUPPORT_MODULE_US_ALGO                        (0)
#define IS_SUPPORT_MODULE_AUDIO                          (0)
#define IS_SUPPORT_MODULE_AIRPLANE                       (0)
#define IS_SUPPORT_MODULE_DATA_BURIAL                    (0)
#define IS_SUPPORT_WEEKLY_STAT                           (0)

// function
#define IS_SUPPORT_MOTOR_SHAKE                           (1)
#define IS_USE_GYRO_ONLINE_CALI                          (1) 
#define IS_SUPPORT_IMU_FIFO                              (0)

// test

#ifdef __cplusplus
}
#endif

#endif


