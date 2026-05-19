#ifndef __PROJECT_LOG_FILTER_H__
#define __PROJECT_LOG_FILTER_H__

#ifdef __cplusplus
extern "C" {
#endif


#define IS_SUPPORT_LOG_OUT                              (1)
#define IS_SUPPORT_LOG_COLOR                            (0)

#define IS_SUPPORT_ALGO_UART_SAMPLE_LOG                 (0)

#if IS_SUPPORT_ALGO_UART_SAMPLE_LOG
#define LOG_FILTER (FILTER_NONE)
#define BSP_LOG_HEX_SUPPORT                             (0)  //底层串口交互日志开关，量产前关闭 
#define BSP_LOG_SUPPORT                                 (0)
#else
#define LOG_FILTER_ALL (\
    FILTER_BLE|FILTER_PBUS|FILTER_UI|FILTER_WM|FILTER_MOTOR|FILTER_STORE|FILTER_DB| \
    FILTER_PMP|FILTER_PM|FILTER_STEP|FILTER_KEY|FILTER_MISC|FILTER_AIR| \
    FILTER_ALGO|FILTER_SENSOR|FILTER_PROTOCOL|FILTER_SRV|FILTER_RES|FILTER_OTA|FILTER_HDR  \
)

#define LOG_FILTER_LVGL (\
    FILTER_UI|FILTER_WM \
)

#define LOG_FILTER   LOG_FILTER_ALL

#if IS_SUPPORT_LOG_OUT
#define BSP_LOG_HEX_SUPPORT                             (1)  //底层串口交互日志开关，量产前关闭 
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

