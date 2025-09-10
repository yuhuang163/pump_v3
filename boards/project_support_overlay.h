#ifndef __PROJECT_SUPPORT_OVERLAY_H__
#define __PROJECT_SUPPORT_OVERLAY_H__

#ifdef __cplusplus
extern "C" {
#endif

// device
#define IS_SUPPORT_DEVICE_LOG_UART                       (1)
#define IS_SUPPORT_DEVICE_IRQ                            (1)
#define IS_SUPPORT_DEVICE_EXT_FLASH                      (1)
#define IS_SUPPORT_DEVICE_DATABASE                       (1)
#define IS_SUPPORT_DEVICE_INPUT_KEY                      (1)
#define IS_SUPPORT_DEVICE_POWER                          (1)
#define IS_SUPPORT_DEVICE_PMU                            (1)
#define IS_SUPPORT_DEVICE_CHARGE                         (1)
#define IS_SUPPORT_DEVICE_BATTERY                        (1)
#define IS_SUPPORT_DEVICE_IMU                            (1)
#define IS_SUPPORT_DEVICE_MOTOR                          (1)
#define IS_SUPPORT_DEVICE_LOG_UART                       (1)
#define IS_SUPPORT_DEVICE_LED                            (1)
#define IS_SUPPORT_DEVICE_AUDIO                          (1)
#define IS_SUPPORT_DEVICE_LCD                            (1)
#define IS_SUPPORT_DEVICE_FSENSOR                        (1)
#define IS_SUPPORT_DEVICE_PRESSURE_KEY                   (1)
#define IS_SUPPORT_DEVICE_BLE                            (1)
#define IS_SUPPORT_DEVICE_FS                             (0)

// module
#define IS_SUPPORT_LOG_OUT                               (1)
#define IS_SUPPORT_LOG_COLOR                             (0)
#define IS_SUPPORT_MODULE_PBUS                           (1)
#define IS_SUPPORT_MODULE_PM                             (1)
#define IS_SUPPORT_MODULE_STORAGE                        (1)
#define IS_SUPPORT_MODULE_RES                            (1)
#define IS_SUPPORT_MODULE_TIME_SYSTEM                    (1)
#define IS_SUPPORT_MODULE_WM                             (1)
#define IS_SUPPORT_MODULE_INPUT                          (1)
#define IS_SUPPORT_MODULE_BLE                            (1)
#define IS_SUPPORT_MODULE_OTA                            (1)
#define IS_SUPPORT_MODULE_MISC                           (1)
#define IS_SUPPORT_MODULE_MOTOR                          (1)
#define IS_SUPPORT_MODULE_BREATH_PUMP                    (1)
#define IS_SUPPORT_MODULE_US_ALGO                        (1)
#define IS_SUPPORT_MODULE_AUDIO                          (1)
#define IS_SUPPORT_MODULE_AIRPLANE                       (1)
#define IS_SUPPORT_MODULE_DATA_BURIAL                    (1)
#define IS_SUPPORT_WEEKLY_STAT                           (1)

// function
#define IS_SUPPORT_MOTOR_SHAKE                           (1)
#define IS_USE_GYRO_ONLINE_CALI                          (1) 
#define IS_SUPPORT_IMU_FIFO                              (0)

// test
#define PRESSURE_TEST_SUPPORT                            (0)
#define BROADCAST_TEST_SUPPORT                           (0)
#define STORE_TEST_SUPPORT                               (1)

#ifdef __cplusplus
}
#endif

#endif


