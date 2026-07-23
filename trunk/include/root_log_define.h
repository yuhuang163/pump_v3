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
uint16_t get_log_output_level(void);
uint64_t platform_get_utc_time_ms(void);
uint32_t platform_get_utc_time(void);
int platform_get_tid(void);
void reset_log_filter(uint32_t filter);
void init_user_log_filter(void);
uint32_t get_log_filter(void);
uint32_t get_timestamp(void);
uint32_t get_timestamp_ms(uint32_t time_tick);
void set_log_ouput_level(uint16_t level);
uint16_t get_log_output_level(void);

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
    FILTER_SRV = 1 << 23,
    FILTER_STEP = 1 << 24,
    FILTER_TEST = 1 << 25,
    FILTER_BURY = 1 << 26,
    FILTER_FULL = -1,
} log_filter_enum_t;

#define TRACE_FUNC_ENTRY(...) LOGV("+++ENTRY+++")
#define TRACE_FUNC_EXIT(...) LOGV("+++EXIT+++")

#define IS_LEVEL(x) if (g_log_level_set >= x)
#define IS_FILTER(x) if (g_log_filter & x)

#ifndef FORMAT_LOG_OUT_SYMBOL
#define FORMAT_LOG_OUT_SYMBOL "[%d][%d][%d]"
#endif

#ifndef FORMAT_LOG_OUT_OBJ
#define FORMAT_LOG_OUT_OBJ platform_get_utc_time(), get_timestamp_ms(0), platform_get_tid()
#endif

#define PRINT_FORMAT(letter, format) \
  LOG_COLOR_##letter #letter FORMAT_LOG_OUT_SYMBOL "[%s]%s(): " format LOG_RESET_COLOR "\n"
#define PRINT_WRAPPER(letter, format, TAG) \
  PRINT_FORMAT(letter, format), FORMAT_LOG_OUT_OBJ, TAG, __func__

/*
* 基础日志输出 (编译期等级裁剪, 运行时仍受 g_log_level_set 控制).
*/
#if US_LOG_COMPILE_LEVEL >= US_LOG_COMPILE_LEVEL_ERROR
#define LOGE_TAG(TAG, format, args...) IS_LEVEL(US_LOG_LEVEL_ERROR)    { PRINT_LOG(PRINT_WRAPPER(E, format, TAG), ##args); }
#else
#define LOGE_TAG(TAG, format, args...) do { } while(0)
#endif

#if US_LOG_COMPILE_LEVEL >= US_LOG_COMPILE_LEVEL_WRAN
#define LOGW_TAG(TAG, format, args...) IS_LEVEL(US_LOG_LEVEL_WRAN)     { PRINT_LOG(PRINT_WRAPPER(W, format, TAG), ##args); }
#else
#define LOGW_TAG(TAG, format, args...) do { } while(0)
#endif

#if US_LOG_COMPILE_LEVEL >= US_LOG_COMPILE_LEVEL_INFO
#define LOGI_TAG(TAG, format, args...) IS_LEVEL(US_LOG_LEVEL_INFO)     { PRINT_LOG(PRINT_WRAPPER(I, format, TAG), ##args); }
#else
#define LOGI_TAG(TAG, format, args...) do { } while(0)
#endif

#if US_LOG_COMPILE_LEVEL >= US_LOG_COMPILE_LEVEL_DEBUG
#define LOGD_TAG(TAG, format, args...) IS_LEVEL(US_LOG_LEVEL_DEBUG)    { PRINT_LOG(PRINT_WRAPPER(D, format, TAG), ##args); }
#else
#define LOGD_TAG(TAG, format, args...) do { } while(0)
#endif

#if US_LOG_COMPILE_LEVEL >= US_LOG_COMPILE_LEVEL_VERBOSE
#define LOGV_TAG(TAG, format, args...) IS_LEVEL(US_LOG_LEVEL_VERBOSE)  { PRINT_LOG(PRINT_WRAPPER(V, format, TAG), ##args); }
#define LOGK_TAG(TAG, format, args...) IS_LEVEL(US_LOG_LEVEL_VERBOSE)  { PRINT_LOG(PRINT_WRAPPER(K, format, TAG), ##args); }
#else
#define LOGV_TAG(TAG, format, args...) do { } while(0)
#define LOGK_TAG(TAG, format, args...) do { } while(0)
#endif

#define LOG_MODULE_PRINT(letter, tag, filter, format, args...) \
    do { IS_FILTER(filter) { PRINT_LOG(PRINT_WRAPPER(letter, format, tag), ##args); } } while(0)

#define _LOG_MODULE_IF_0(letter, tag, filter, format, args...) do { } while(0)
#define _LOG_MODULE_IF_1(letter, tag, filter, format, args...) LOG_MODULE_PRINT(letter, tag, filter, format, ##args)
#define _LOG_MODULE_IF(cond, letter, tag, filter, format, args...) \
    _LOG_MODULE_IF_##cond(letter, tag, filter, format, ##args)
#define LOG_MODULE_IF(enabled, letter, tag, filter, format, args...) \
    _LOG_MODULE_IF(enabled, letter, tag, filter, format, ##args)

/*
* 模块日志输出 (独立编译期开关 + 运行时 filter + PRINT_WRAPPER).
*/
#if IS_SUPPORT_LOG_OUT
#define LOG_PM(format, args...)         LOG_MODULE_IF(IS_SUPPORT_LOG_PM, W, "LOG_PM", FILTER_PM, format, ##args)
#define LOG_PBUS(format, args...)       LOG_MODULE_IF(IS_SUPPORT_LOG_PBUS, I, "LOG_PBUS", FILTER_PBUS, format, ##args)
#define LOG_WM(format, args...)         LOG_MODULE_IF(IS_SUPPORT_LOG_WM, I, "LOG_WM", FILTER_WM, format, ##args)
#define LOG_UI(format, args...)         LOG_MODULE_IF(IS_SUPPORT_LOG_UI, D, "LOG_UI", FILTER_UI, format, ##args)
#define LOG_KEY(format, args...)        LOG_MODULE_IF(IS_SUPPORT_LOG_KEY, W, "LOG_KEY", FILTER_KEY, format, ##args)
#define LOG_RES(format, args...)        LOG_MODULE_IF(IS_SUPPORT_LOG_RES, I, "LOG_RES", FILTER_RES, format, ##args)
#define LOG_MISC(format, args...)       LOG_MODULE_IF(IS_SUPPORT_LOG_MISC, I, "LOG_MISC", FILTER_MISC, format, ##args)
#define LOG_PMP(format, args...)        LOG_MODULE_IF(IS_SUPPORT_LOG_PMP, I, "LOG_PMP", FILTER_PMP, format, ##args)
#define LOG_MOTOR(format, args...)      LOG_MODULE_IF(IS_SUPPORT_LOG_MOTOR, I, "LOG_MOTOR", FILTER_MOTOR, format, ##args)
#define LOG_SENSOR(format, args...)     LOG_MODULE_IF(IS_SUPPORT_LOG_SENSOR, I, "LOG_SENSOR", FILTER_SENSOR, format, ##args)
#define LOG_DB(format, args...)         LOG_MODULE_IF(IS_SUPPORT_LOG_DB, W, "LOG_DB", FILTER_DB, format, ##args)
#define LOG_BURY(format, args...)       LOG_MODULE_IF(IS_SUPPORT_LOG_BURY, I, "LOG_BURY", FILTER_BURY, format, ##args)
#define LOG_BCR(format, args...)        LOG_MODULE_IF(IS_SUPPORT_LOG_BCR, I, "LOG_BCR", FILTER_BCR, format, ##args)
#define LOG_HDR(format, args...)        LOG_MODULE_IF(IS_SUPPORT_LOG_HDR, I, "LOG_HDR", FILTER_HDR, format, ##args)
#define LOG_BLE(format, args...)        LOG_MODULE_IF(IS_SUPPORT_LOG_BLE, W, "LOG_BLE", FILTER_BLE, format, ##args)
#define LOG_OTA(format, args...)        LOG_MODULE_IF(IS_SUPPORT_LOG_OTA, W, "LOG_OTA", FILTER_OTA, format, ##args)
#define LOG_TIME(format, args...)       LOG_MODULE_IF(IS_SUPPORT_LOG_TIME, I, "LOG_TIME", FILTER_TIME, format, ##args)
#define LOG_ALGO(format, args...)       LOG_MODULE_IF(IS_SUPPORT_LOG_ALGO, I, "LOG_ALGO", FILTER_ALGO, format, ##args)
#define LOG_PROTOCOL(format, args...)   LOG_MODULE_IF(IS_SUPPORT_LOG_PROTOCOL, I, "LOG_PTCL", FILTER_PROTOCOL, format, ##args)
#define LOG_AUDIO(format, args...)      LOG_MODULE_IF(IS_SUPPORT_LOG_AUDIO, I, "LOG_AUDIO", FILTER_AUDIO, format, ##args)
#define LOG_AIR(format, args...)        LOG_MODULE_IF(IS_SUPPORT_LOG_AIR, D, "LOG_AIR", FILTER_AIR, format, ##args)
#define LOG_STORE(format, args...)      LOG_MODULE_IF(IS_SUPPORT_LOG_STORE, W, "LOG_STORE", FILTER_STORE, format, ##args)
#define LOG_HG(format, args...)         LOG_MODULE_IF(IS_SUPPORT_LOG_HG, W, "LOG_HG", FILTER_HG, format, ##args)
#define LOG_SRV(format, args...)        LOG_MODULE_IF(IS_SUPPORT_LOG_SRV, I, "LOG_SRV", FILTER_SRV, format, ##args)
#define LOG_TEST(format, args...)       LOG_MODULE_IF(IS_SUPPORT_LOG_TEST, I, "LOG_TEST", FILTER_TEST, format, ##args)
/*
* 进度日志输出.
*/
#define LOG_STEP(format, args...)       LOG_MODULE_IF(IS_SUPPORT_LOG_STEP, W, "LOG_STEP", FILTER_STEP, format, ##args)

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
    if (get_log_output_level() < US_LOG_LEVEL_INFO) { \
        break; \
    } \
    PRINT_LOG("%s size=%d\r\n", tag, size); \
    for (int i = 0; i < size; i++) { \
        PRINT_LOG("%02X ", (unsigned)data[i]); \
        if ((i + 1) % 32 == 0) { \
            PRINT_LOG("\r\n"); \
        } \
    } \
    PRINT_LOG("\r\n"); \
} while (0)

#define LOG_PASS(format, args...) \
do { \
    if (get_log_output_level() < US_LOG_LEVEL_INFO) { \
        break; \
    } \
    PRINT_LOG(format, ##args); \
} while(0)

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
#define LOG_BURY(...)       do { } while(0)
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
#define LOG_TEST(...)       do { } while(0)
#define LOG_SRV(...)        do { } while(0)

#define LOG_STEP(format, args...) do { } while(0)

#define LOGE(format, args...) do { } while(0)
#define LOGW(format, args...) do { } while(0)
#define LOGI(format, args...) do { } while(0)
#define LOGD(format, args...) do { } while(0)
#define LOGV(format, args...) do { } while(0)
#define LOG_HEX(tag, data, size) do { } while(0)
#define LOG_PASS(format, args...) do { } while(0)

#endif /// LOG_OUT_SUPPORT


#ifdef __cplusplus
}
#endif

#endif 
