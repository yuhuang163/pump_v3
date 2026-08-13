#ifndef __PROJECT_LOG_FILTER_H__
#define __PROJECT_LOG_FILTER_H__

#ifdef __cplusplus
extern "C" {
#endif


#define IS_SUPPORT_LOG_OUT                              (1)
#define IS_SUPPORT_LOG_COLOR                            (0)

/// 编译期日志等级 (预处理器常量, 须与 E_ID_LOG_LEVEL_T 枚举值一致)
#define US_LOG_COMPILE_LEVEL_NONE                       0
#define US_LOG_COMPILE_LEVEL_ERROR                      1
#define US_LOG_COMPILE_LEVEL_WRAN                       2
#define US_LOG_COMPILE_LEVEL_INFO                       3
#define US_LOG_COMPILE_LEVEL_DEBUG                      4
#define US_LOG_COMPILE_LEVEL_VERBOSE                    5

#ifndef US_LOG_COMPILE_LEVEL
#define US_LOG_COMPILE_LEVEL                            US_LOG_COMPILE_LEVEL_DEBUG
#endif

/// 模块日志编译期开关 (1=编译, 0=裁剪)
#define IS_SUPPORT_LOG_PM                               1
#define IS_SUPPORT_LOG_PBUS                             1 // 2KB
#define IS_SUPPORT_LOG_WM                               1
#define IS_SUPPORT_LOG_UI                               1
#define IS_SUPPORT_LOG_KEY                              1
#define IS_SUPPORT_LOG_RES                              1
#define IS_SUPPORT_LOG_MISC                             1
#define IS_SUPPORT_LOG_PMP                              1
#define IS_SUPPORT_LOG_MOTOR                            1 // 7KB
#define IS_SUPPORT_LOG_SENSOR                           1 // 1KB
#define IS_SUPPORT_LOG_DB                               1
#define IS_SUPPORT_LOG_BURY                             1 // 7KB
#define IS_SUPPORT_LOG_BCR                              1
#define IS_SUPPORT_LOG_HDR                              1
#define IS_SUPPORT_LOG_BLE                              1 // 9KB
#define IS_SUPPORT_LOG_OTA                              1 // 1KB
#define IS_SUPPORT_LOG_TIME                             1
#define IS_SUPPORT_LOG_ALGO                             1 // 5KB
#define IS_SUPPORT_LOG_PROTOCOL                         1 // 5KB
#define IS_SUPPORT_LOG_AUDIO                            0
#define IS_SUPPORT_LOG_AIR                              1 // 2KB
#define IS_SUPPORT_LOG_STORE                            1 // 9KB
#define IS_SUPPORT_LOG_HG                               1
#define IS_SUPPORT_LOG_SRV                              1 // 36KB
#define IS_SUPPORT_LOG_TEST                             1 // 5KB
#define IS_SUPPORT_LOG_STEP                             1

#define IS_SUPPORT_ALGO_UART_SAMPLE_LOG                 (0)

#if IS_SUPPORT_ALGO_UART_SAMPLE_LOG
#define LOG_FILTER (FILTER_NONE)
#define BSP_LOG_HEX_SUPPORT                             (0)  // 底层串口交互日志开关，量产前关闭 
#define BSP_LOG_SUPPORT                                 (0)
#else
#define LOG_FILTER_ALL ( \
    FILTER_BLE|FILTER_PBUS|FILTER_UI|FILTER_WM|FILTER_MOTOR|FILTER_STORE|FILTER_DB| \
    FILTER_PMP|FILTER_PM|FILTER_STEP|FILTER_KEY|FILTER_MISC|FILTER_AIR| \
    FILTER_ALGO|FILTER_SENSOR|FILTER_PROTOCOL|FILTER_SRV|FILTER_RES|FILTER_OTA|FILTER_HDR|FILTER_BURY  \
)

#define LOG_FILTER_LVGL ( \
    FILTER_UI|FILTER_WM \
)

#define LOG_FILTER_DEBUG ( \
    FILTER_NONE \
)

#define LOG_FILTER   LOG_FILTER_ALL

#if IS_SUPPORT_LOG_OUT
#define BSP_LOG_HEX_SUPPORT                             (1)  // 底层串口交互日志开关，量产前关闭 
#define BSP_LOG_SUPPORT                                 (1)
#else
#define BSP_LOG_HEX_SUPPORT                             (0)
#define BSP_LOG_SUPPORT                                 (0)
#endif

#endif

#ifdef __cplusplus
}
#endif

#endif

