#ifndef __PROJECT_LOG_FILTER_H__
#define __PROJECT_LOG_FILTER_H__

#ifdef __cplusplus
extern "C" {
#endif

#define LOG_FILTER (\
    FILTER_BLE|FILTER_PBUS|FILTER_UI|FILTER_WM|FILTER_MOTOR|FILTER_STORE|FILTER_DB| \
    FILTER_PMP|FILTER_PM|FILTER_STEP\
)

#define BSP_LOG_HEX_SUPPORT                     (1)  //底层串口交互日志开关，量产前关闭 
#define BSP_LOG_SUPPORT                         (1)

// #define PRINT_LOG(...)

#ifdef __cplusplus
}
#endif

#endif

