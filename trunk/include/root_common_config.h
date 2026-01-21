#ifndef __ROOT_COMMON_CONFIG_H__
#define __ROOT_COMMON_CONFIG_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "cmsis_os_api.h" // IWYU pragma: keep

/// 各关键业务线程优先级定义.
#define RT_PRIO_MAIN_THREAD                            OS_PRIORITY_REALTIME5
#define RT_PRIO_MISC_DISPATCH_THREAD                   OS_PRIORITY_BELOW_NORMAL
#define RT_PRIO_BREAST_PUMP_DISPATCH_THREAD            OS_PRIORITY_NORMAL
#define RT_PRIO_OTA_DISPATCH_THREAD                    OS_PRIORITY_REALTIME
#define RT_PRIO_LVGL_HANDLER_THREAD                    OS_PRIORITY_REALTIME6
#define RT_PRIO_BRC_DISPATCH_THREAD                    OS_PRIORITY_REALTIME
#define RT_PRIO_HDR_POST_THREAD                        OS_PRIORITY_REALTIME
#define RT_PRIO_PM_FREEZE_THREAD                       OS_PRIORITY_REALTIME
#define RT_PRIO_PM_SLEEP_THREAD                        OS_PRIORITY_REALTIME
#define RT_PRIO_KEY_INPUT_DISPATCH_THREAD              OS_PRIORITY_REALTIME5
#define RT_PRIO_HANG_DETECT_THREAD                     OS_PRIORITY_REALTIME5

#ifndef RT_PRIO_WINDOW_DISPATCH_THREAD
#define RT_PRIO_WINDOW_DISPATCH_THREAD                 OS_PRIORITY_REALTIME1
#endif

#ifndef RT_PRIO_WINDOW_FLUSH_THREAD
#define RT_PRIO_WINDOW_FLUSH_THREAD                    OS_PRIORITY_REALTIME1
#endif

#ifndef RT_PRIO_MOTOR_DISPATCH_THREAD
#define RT_PRIO_MOTOR_DISPATCH_THREAD                  OS_PRIORITY_REALTIME3
#endif

#ifndef RT_PRIO_BLE_DISPATCH_THREAD
#define RT_PRIO_BLE_DISPATCH_THREAD                    OS_PRIORITY_REALTIME3
#endif

#ifndef RT_PRIO_SENSOR_INPUT_DISPATCH_THREAD
#define RT_PRIO_SENSOR_INPUT_DISPATCH_THREAD           OS_PRIORITY_REALTIME7
#endif

#ifndef RT_PRIO_PRODUCT_TESTS_DISPATCH_THREAD
#define RT_PRIO_PRODUCT_TESTS_DISPATCH_THREAD           OS_PRIORITY_REALTIME4
#endif

#ifndef RT_PRIO_INTERRUPT_THREAD
#define RT_PRIO_INTERRUPT_THREAD                       OS_PRIORITY_REALTIME
#endif

#ifndef RT_PRIO_MOTOR_DRIVER_THREAD
#define RT_PRIO_MOTOR_DRIVER_THREAD                    OS_PRIORITY_NORMAL
#endif

#ifndef RT_PRIO_LCD_THREAD
#define RT_PRIO_LCD_THREAD                             OS_PRIORITY_REALTIME6
#endif

#ifndef RT_PRIO_FACTORY_UART_THREAD
#define RT_PRIO_FACTORY_UART_THREAD                    OS_PRIORITY_REALTIME
#endif

#ifndef RT_PRIO_KEY_DEVICE_THREAD
#define RT_PRIO_KEY_DEVICE_THREAD                      OS_PRIORITY_REALTIME
#endif

#ifndef RT_PRIO_CHARGE_DEVICE_THREAD
#define RT_PRIO_CHARGE_DEVICE_THREAD                   OS_PRIORITY_REALTIME
#endif

#ifndef RT_PRIO_LED_DEVICE_THREAD
#define RT_PRIO_LED_DEVICE_THREAD                      OS_PRIORITY_REALTIME
#endif

///  各关键业务线程栈深度, 默认线程单位栈深1KB
#ifndef TASK_SIZE_DEFAULT
#define TASK_SIZE_DEFAULT                              (1024)
#endif

#ifndef MAIN_THREAD_STACKSIZE
#define MAIN_THREAD_STACKSIZE                          (TASK_SIZE_DEFAULT * 16 + 0)
#endif

#ifndef HDR_THREAD_STACKSIZE
#define HDR_THREAD_STACKSIZE                           (TASK_SIZE_DEFAULT * 4 + 1024)
#endif

#ifndef BCR_THREAD_STACKSIZE
#define BCR_THREAD_STACKSIZE                           (TASK_SIZE_DEFAULT * 4 + 512)
#endif

#ifndef MOTOR_PLAY_THREAD_STACKSIZE
#define MOTOR_PLAY_THREAD_STACKSIZE                    (TASK_SIZE_DEFAULT * 4 + 0)
#endif

#ifndef BLE_THREAD_STACKSIZE
#define BLE_THREAD_STACKSIZE                           (TASK_SIZE_DEFAULT * 4 + 0)
#endif

#ifndef BST_THREAD_STACKSIZE
#define BST_THREAD_STACKSIZE                           (TASK_SIZE_DEFAULT * 4 + 0)
#endif

#ifndef LVGL_RENDER_THREAD_STACKSIZE
#define LVGL_RENDER_THREAD_STACKSIZE                   (TASK_SIZE_DEFAULT * 4 + 0)
#endif

#ifndef WM_THREAD_STACKSIZE
#define WM_THREAD_STACKSIZE                            (TASK_SIZE_DEFAULT * 4 + 0)
#endif

#ifndef OTA_THREAD_STACKSIZE
#define OTA_THREAD_STACKSIZE                           (TASK_SIZE_DEFAULT * 4 + 0)
#endif

#ifndef INPUT_KEY_THREAD_STACKSIZE
#define INPUT_KEY_THREAD_STACKSIZE                     (TASK_SIZE_DEFAULT * 4 + 0)
#endif

#ifndef INPUT_SENSOR_THREAD_STACKSIZE
#define INPUT_SENSOR_THREAD_STACKSIZE                  (TASK_SIZE_DEFAULT * 4 + 0)
#endif

#ifndef HG_CHECK_THREAD_STACKSIZE
#define HG_CHECK_THREAD_STACKSIZE                      (TASK_SIZE_DEFAULT * 3 + 0)
#endif

#ifndef HG_DETECT_THREAD_STACKSIZE
#define HG_DETECT_THREAD_STACKSIZE                     (TASK_SIZE_DEFAULT * 3 + 0)
#endif

#ifndef MISC_THREAD_STACKSIZE
#define MISC_THREAD_STACKSIZE                          (TASK_SIZE_DEFAULT * 3 + 0)
#endif

#ifndef PM_SUSPEND_THREAD_STACKSIZE
#define PM_SUSPEND_THREAD_STACKSIZE                    (TASK_SIZE_DEFAULT * 3 + 512)
#endif

#ifndef PM_FREEZE_THREAD_STACKSIZE
#define PM_FREEZE_THREAD_STACKSIZE                     (TASK_SIZE_DEFAULT * 3 + 512)
#endif

#ifndef INTERRUPT_THREAD_STACKSIZE
#define INTERRUPT_THREAD_STACKSIZE                     (TASK_SIZE_DEFAULT)
#endif

#ifndef MOTOR_DRIVER_THREAD_STACKSIZE
#define MOTOR_DRIVER_THREAD_STACKSIZE                  (TASK_SIZE_DEFAULT * 1 + 0)
#endif

#ifndef LCD_UART_THREAD_STACKSIZE
#define LCD_UART_THREAD_STACKSIZE                      (TASK_SIZE_DEFAULT * 1 + 0)
#endif

#ifndef FACTORY_UART_THREAD_STACKSIZE
#define FACTORY_UART_THREAD_STACKSIZE                  (TASK_SIZE_DEFAULT * 1 + 0)
#endif

#ifndef KEY_DEVICE_THREAD_STACKSIZE
#define KEY_DEVICE_THREAD_STACKSIZE                    (TASK_SIZE_DEFAULT * 1 + 0)
#endif

#ifndef CHARGE_DEVICE_THREAD_STACKSIZE
#define CHARGE_DEVICE_THREAD_STACKSIZE                 (TASK_SIZE_DEFAULT * 1 + 0)
#endif

#ifndef LED_DEVICE_THREAD_STACKSIZE
#define LED_DEVICE_THREAD_STACKSIZE                    (TASK_SIZE_DEFAULT * 1 + 0)
#endif

/// 各关键业务消息队列数量定义.
#define DISP_DISPATCH_EVT_MESQ_NUM                    (10)
#define INPUT_DISPATCH_KEY_EVT_MESQ_NUM               (2)
#define INPUT_DISPATCH_SENSOR_EVT_MESQ_NUM            (2)
#define BLE_WORK_DISPATCH_MESQ_NUM                    (15)
#define OTA_WORK_DISPATCH_MESQ_NUM                    (4)
#define MISC_WORK_DISPATCH_MESQ_NUM                   (2)
#define IOT_WORK_DISPATCH_MESQ_NUM                    (2)
#define MOTOR_IAP_DISPATCH_MESQ_NUM                   (2)
#define MOTOR_PLAY_MESQ_NUM                           (2)
#define PRODUCT_TEST_EVT_MESQ_NUM                     (2)

#define BREAST_PUMP_WORK_DISPATCH_MESQ_NUM            (2)
#define PM_WORK_DISPATCH_MESQ_NUM                     (1)
#define HDR_POST_MESQ_NUM                             (8)
#define BCR_THD_MESQ_NUM                              (30)
#define ALGO_WORK_MESQ_NUM                            (2)
#define AUDIO_PLAY_STREAM_MESQ_NUM                    (2)

#ifdef __cplusplus
}
#endif

#endif 
