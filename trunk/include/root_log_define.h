#ifndef __ROOT_LOG_DEFINE_H__
#define __ROOT_LOG_DEFINE_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "project_log_filter.h" // IWYU pragma: keep
#include "project_support_overlay.h"

/// 日志颜色.
#if IS_SUPPORT_LOG_COLOR
#define LOG_COLOR_BLACK   "30"
#define LOG_COLOR_RED     "31"
#define LOG_COLOR_GREEN   "32"
#define LOG_COLOR_BROWN   "33"
#define LOG_COLOR_BLUE    "34"
#define LOG_COLOR_PURPLE  "35"
#define LOG_COLOR_CYAN    "36"
#define LOG_COLOR(COLOR)  "\033[0;" COLOR "m"
#define LOG_BOLD(COLOR)   "\033[1;" COLOR "m"
#define LOG_RESET_COLOR   "\033[0m"
#define LOG_COLOR_E       LOG_COLOR(LOG_COLOR_RED)
#define LOG_COLOR_W       LOG_COLOR(LOG_COLOR_BLUE)
#define LOG_COLOR_I       LOG_COLOR(LOG_COLOR_GREEN)
#define LOG_COLOR_K       LOG_COLOR(LOG_COLOR_CYAN)
#define LOG_COLOR_D
#define LOG_COLOR_V
#else
#define LOG_RESET_COLOR
#define LOG_COLOR_E
#define LOG_COLOR_W
#define LOG_COLOR_I
#define LOG_COLOR_D
#define LOG_COLOR_V
#endif /// LOG_COLOR_SUPPORT

/// public
extern uint16_t g_log_level_set;
extern uint32_t g_log_filter;

uint64_t platform_get_utc_time_ms(void);
uint32_t platform_get_utc_time(void);
int platform_get_tid(void);
void reset_log_filter(uint32_t filter);
void init_user_log_filter(void);
uint32_t get_log_filter(void);

/// log level define
typedef enum {
    US_LOG_LEVEL_NONE = 0,
    US_LOG_LEVEL_ERROR,
    US_LOG_LEVEL_WRAN,
    US_LOG_LEVEL_INFO,
    US_LOG_LEVEL_DEBUG,
    US_LOG_LEVEL_VERBOSE
} E_ID_LOG_LEVEL_T;

typedef enum{
    FILTER_NONE = 0,
    FILTER_PM = 1 << 1,
    FILTER_PBUS = 1 << 2,
    FILTER_WM = 1 << 3,
    FILTER_UI = 1 << 4,
    FILTER_KEY = 1 << 5,
    FILTER_SENSOR = 1 << 6,
    FILTER_RES = 1 << 7,
    FILTER_MISC = 1 << 8,
    FILTER_PMP = 1 << 9,
    FILTER_OTA = 1 << 10,
    FILTER_BLE = 1 << 11,
    FILTER_MOTOR = 1 << 12,
    FILTER_DB = 1 << 13,
    FILTER_BCR = 1 << 14,
    FILTER_HDR = 1 << 15,
    FILTER_TIME = 1 << 16,
    FILTER_ALGO = 1 << 17,
    FILTER_PROTOCOL = 1 << 18,
    FILTER_AUDIO = 1 << 19,
    FILTER_AIR = 1 << 20,
    FILTER_STORE = 1 << 21,
    FILTER_HG = 1 << 22,
    FILTER_LED = 1 << 23,
    FILTER_STEP = 1 << 24,
    FILTER_FULL = -1,
} log_filter_enum_t;

#define TRACE_FUNC_ENTRY(...) LOGV("+++ENTRY+++")
#define TRACE_FUNC_EXIT(...) LOGV("+++EXIT+++")

#define IS_LEVEL(x) if (g_log_level_set >= x)
#define IS_FILTER(x) if (g_log_filter & x)

#ifndef FORMAT_LOG_OUT_SYMBOL
#define FORMAT_LOG_OUT_SYMBOL "[%d][%d]"
#endif

#ifndef FORMAT_LOG_OUT_OBJ
#define FORMAT_LOG_OUT_OBJ platform_get_utc_time(), platform_get_tid()
#endif

#define PRINT_FORMAT(letter, format) \
  LOG_COLOR_##letter #letter FORMAT_LOG_OUT_SYMBOL "[%s][%s](): " format LOG_RESET_COLOR "\n"
#define PRINT_WRAPPER(letter, format, TAG) \
  PRINT_FORMAT(letter, format), FORMAT_LOG_OUT_OBJ, TAG, __func__

/*
* 基础日志输出.
*/
#define LOGE_TAG(TAG, format, args...) IS_LEVEL(US_LOG_LEVEL_ERROR)    { PRINT_LOG(PRINT_WRAPPER(E, format, TAG), ##args); }
#define LOGW_TAG(TAG, format, args...) IS_LEVEL(US_LOG_LEVEL_WRAN)     { PRINT_LOG(PRINT_WRAPPER(W, format, TAG), ##args); }
#define LOGI_TAG(TAG, format, args...) IS_LEVEL(US_LOG_LEVEL_INFO)     { PRINT_LOG(PRINT_WRAPPER(I, format, TAG), ##args); }
#define LOGD_TAG(TAG, format, args...) IS_LEVEL(US_LOG_LEVEL_DEBUG)    { PRINT_LOG(PRINT_WRAPPER(D, format, TAG), ##args); }
#define LOGV_TAG(TAG, format, args...) IS_LEVEL(US_LOG_LEVEL_VERBOSE)  { PRINT_LOG(PRINT_WRAPPER(V, format, TAG), ##args); }
#define LOGK_TAG(TAG, format, args...) IS_LEVEL(US_LOG_LEVEL_VERBOSE)  { PRINT_LOG(PRINT_WRAPPER(K, format, TAG), ##args); }
/*
* 模块日志输出.
*/
#if IS_SUPPORT_LOG_OUT
#define LOG_PM(...)         do { IS_FILTER(FILTER_PM)       { LOGW_TAG("LOG_PM", __VA_ARGS__); } } while(0)
#define LOG_PBUS(...)       do { IS_FILTER(FILTER_PBUS)     { LOGI_TAG("LOG_PBUS", __VA_ARGS__); } } while(0)
#define LOG_WM(...)         do { IS_FILTER(FILTER_WM)       { LOGI_TAG("LOG_WM", __VA_ARGS__); } } while(0)
#define LOG_UI(...)         do { IS_FILTER(FILTER_UI)       { LOGD_TAG("LOG_UI", __VA_ARGS__); } } while(0)
#define LOG_KEY(...)        do { IS_FILTER(FILTER_KEY)      { LOGW_TAG("LOG_KEY", __VA_ARGS__); } } while(0)
#define LOG_RES(...)        do { IS_FILTER(FILTER_RES)      { LOGI_TAG("LOG_RES", __VA_ARGS__); } } while(0)
#define LOG_MISC(...)       do { IS_FILTER(FILTER_MISC)     { LOGI_TAG("LOG_MISC", __VA_ARGS__); } } while(0)
#define LOG_PMP(...)        do { IS_FILTER(FILTER_PMP)      { LOGI_TAG("LOG_PMP", __VA_ARGS__); } } while(0)
#define LOG_MOTOR(...)      do { IS_FILTER(FILTER_MOTOR)    { LOGI_TAG("LOG_MOTOR", __VA_ARGS__); } } while(0)
#define LOG_SENSOR(...)     do { IS_FILTER(FILTER_SENSOR)   { LOGI_TAG("LOG_FSENSOR", __VA_ARGS__); } } while(0)
#define LOG_DB(...)         do { IS_FILTER(FILTER_DB)       { LOGW_TAG("LOG_DB", __VA_ARGS__); } } while(0)
#define LOG_BCR(...)        do { IS_FILTER(FILTER_BCR)      { LOGI_TAG("LOG_BCR", __VA_ARGS__); } } while(0)
#define LOG_HDR(...)        do { IS_FILTER(FILTER_HDR)      { LOGI_TAG("LOG_HDR", __VA_ARGS__); } } while(0)
#define LOG_BLE(...)        do { IS_FILTER(FILTER_BLE)      { LOGW_TAG("LOG_BLE", __VA_ARGS__); } } while(0)
#define LOG_OTA(...)        do { IS_FILTER(FILTER_OTA)      { LOGW_TAG("LOG_OTA", __VA_ARGS__); } } while(0)
#define LOG_TIME(...)       do { IS_FILTER(FILTER_TIME)     { LOGI_TAG("LOG_TIME", __VA_ARGS__); } } while(0)
#define LOG_ALGO(...)       do { IS_FILTER(FILTER_ALGO)     { LOGI_TAG("LOG_ALGO", __VA_ARGS__); } } while(0)
#define LOG_PROTOCOL(...)   do { IS_FILTER(FILTER_PROTOCOL) { LOGI_TAG("LOG_PROTOCOL", __VA_ARGS__); } } while(0)
#define LOG_AUDIO(...)      do { IS_FILTER(FILTER_AUDIO)    { LOGI_TAG("LOG_AUDIO", __VA_ARGS__); } } while(0)
#define LOG_AIR(...)        do { IS_FILTER(FILTER_AIR)      { LOGD_TAG("LOG_AIR", __VA_ARGS__); } } while(0)
#define LOG_STORE(...)      do { IS_FILTER(FILTER_STORE)    { LOGW_TAG("LOG_STORE", __VA_ARGS__); } } while(0)
#define LOG_HG(...)         do { IS_FILTER(FILTER_HG)       { LOGW_TAG("LOG_HG", __VA_ARGS__); } } while(0)
#define LOG_LED(...)        do { IS_FILTER(FILTER_LED)      { LOGW_TAG("LOG_LED", __VA_ARGS__); } } while(0)

/*
* 进度日志输出.
*/
#define LOG_STEP(...)    do { IS_FILTER(FILTER_STEP)        { LOGW_TAG("LOG_STEP", __VA_ARGS__); } } while(0)

/*
* 通用日志输出.
*/
#define LOGE(format, args...) do { LOGE_TAG("LOGE", format, ##args); } while(0)
#define LOGW(format, args...) do { LOGW_TAG("LOGW", format, ##args); } while(0)
#define LOGI(format, args...) do { LOGI_TAG("LOGI", format, ##args); } while(0)
#define LOGD(format, args...) do { LOGD_TAG("LOGD", format, ##args); } while(0)
#define LOGV(format, args...) do { LOGV_TAG("LOGV", format, ##args); } while(0)

#define LOG_HEX(tag, data, size)  \
do { \
    uint32_t lenght = (size * 3) + 8;\
    if (lenght < 512) { lenght = 512;}\
    char *log_str = u_malloz(lenght);\
    if (log_str == NULL) { break; }\
    for (uint16_t i = 0; i < size; i++) { \
        sprintf((log_str + (i * 3)) , "%02X ", data[i]); \
    } \
    PRINT_LOG("%s:%s\r\n", tag, log_str); \
    u_free(log_str);\
} while (0)



#else /// LOG_OUT_SUPPORT

#define LOG_PM(...)         do { } while(0)
#define LOG_PBUS(...)       do { } while(0)
#define LOG_WM(...)         do { } while(0)
#define LOG_UI(...)         do { } while(0)
#define LOG_KEY(...)        do { } while(0)
#define LOG_RES(...)        do { } while(0)
#define LOG_MISC(...)       do { } while(0)
#define LOG_PMP(...)        do { } while(0)
#define LOG_MOTOR(...)      do { } while(0)
#define LOG_SENSOR(...)     do { } while(0)
#define LOG_DB(...)         do { } while(0)
#define LOG_BCR(...)        do { } while(0)
#define LOG_HDR(...)        do { } while(0)
#define LOG_BLE(...)        do { } while(0)
#define LOG_OTA(...)        do { } while(0)
#define LOG_TIME(...)       do { } while(0)
#define LOG_ALGO(...)       do { } while(0)
#define LOG_PROTOCOL(...)   do { } while(0)
#define LOG_AUDIO(...)      do { } while(0)
#define LOG_AIR(...)        do { } while(0)
#define LOG_STORE(...)      do { } while(0)
#define LOG_HG(...)         do { } while(0)
#define LOG_LED(...)        do { } while(0)
#define LOG_STEP(format, args...) do { } while(0)

#define LOGE(format, args...) do { } while(0)
#define LOGW(format, args...) do { } while(0)
#define LOGI(format, args...) do { } while(0)
#define LOGD(format, args...) do { } while(0)
#define LOGV(format, args...) do { } while(0)

#endif /// LOG_OUT_SUPPORT


#ifdef __cplusplus
}
#endif

#endif 
