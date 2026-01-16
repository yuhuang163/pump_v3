#ifndef __BSP_COMMON_H__
#define __BSP_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
* Include files
******************************************************************************/
// #include "bsp_config.h"
#include "stdint.h" // IWYU pragma: keep
#include "plat_types.h" // IWYU pragma: keep
#include "root_common_api.h" // IWYU pragma: keep

#ifndef IRAM_ATTR
#define IRAM_ATTR
#endif

typedef enum {
    INT_NUM_NONE     = -1,
    INT_NUM_KEY_1    = 0,
    INT_NUM_KEY_2,
    INT_NUM_KEY_3,
    INT_NUM_KEY_4,
    INT_NUM_KEY_5,
    INT_NUM_KEY_6,
    INT_NUM_TOUCH_KEY,
    INT_NUM_KNOB_LEFT,
    INT_NUM_KNOB_RIGHT,

    INT_NUM_IMU       = 0x10,
    INT_NUM_F_SENSOR  = 0x11,
    INT_NUM_CHARGE    = 0x12,
    INT_NUM_AUDIO     = 0x13,
    INT_NUM_CHARGE_PD = 0x14,
} interrupt_number_e;

typedef enum {
    BSP_OK       = 0x00,                /**< Operation is OK. */
    BSP_ERROR    = 0x01,                /**< Parameter error or operation is not supported. */
    BSP_BUSY     = 0x02,                /**< Driver is busy. */
    BSP_TIMEOUT  = 0x03,                 /**< Timeout occurred. */
    BSP_FULL     = 0x04,                 /**< . */
    BSP_OVERFLOW = 0x05,                 /**< . */
    BSP_ERR_POINTER_NULL = 0x06,         /**< Invalid pointer supplied. */
    BSP_INVALID_PARAMETER = 0x07,        /**< Invalid parameter. */
    BSP_NOT_READY = 0x08,                 /**< Peripherals are not ready. */
    BSP_ERR_NO_MEM = 0x09                 /**< No memory. */
} bsp_status_e;

uint16_t get_log_output_level(void);
#if BSP_LOG_HEX_SUPPORT
#define BSP_LOG_HEX(tag, data, size)  \
do { \
    if (get_log_output_level() < US_LOG_LEVEL_INFO) { \
        break; \
    } \
    uint32_t lenght = (size * 3) + 8;\
    if (lenght < 512) { lenght = 512;}\
    char *log_str = u_malloz(lenght);\
    if (log_str == NULL) { break; }\
    const uint8_t *hex_data__ = (const uint8_t *)(data); \
    for (uint16_t i = 0; i < size; i++) { \
        sprintf((log_str + (i * 3)) , "%02X ", (unsigned)hex_data__[i]); \
    } \
    PRINT_LOG("[%d][%d]:%s:%s\r\n", platform_get_utc_time(), get_timestamp_ms(0), tag, log_str); \
    u_free(log_str);\
} while (0)
#else
#define BSP_LOG_HEX(tag, data, size)
#endif

#if BSP_LOG_SUPPORT
#define BSP_LOG(tag, format,...)  \
do { \
    if (get_log_output_level() < US_LOG_LEVEL_INFO) { \
        break; \
    } \
    PRINT_LOG("[%d][%d]:%s:", platform_get_utc_time(), get_timestamp_ms(0),tag); \
    PRINT_LOG(format,##__VA_ARGS__); \
    PRINT_LOG("\r\n"); \
} while (0)
#else
#define BSP_LOG(tag, format,...)
#endif

#ifdef __cplusplus
}
#endif

#endif 
