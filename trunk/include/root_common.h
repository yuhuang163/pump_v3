#ifndef __ROOT_COMMON_H__
#define __ROOT_COMMON_H__

#ifdef __cplusplus
extern "C" {
#endif


/*******************************************************************************
 * Include files
 ******************************************************************************/
#include <stdint.h> // IWYU pragma: keep
#include <stdbool.h>
#include <stddef.h>

#define TO_STR(str)                         #str
#define TO_PRINT(x)                         ((int32_t)(x))

#define LENGTH_OF_ARRAY(x)                  (sizeof(x) /sizeof(x[0]))

#ifndef abs
#define abs(x)                              ((x) < 0 ? -(x) : (x))
#endif

/// 异常检查宏定义.
#define LOG_EXCP      LOGW
#define EXCEPTION_CHECK_VOID(x)     __EXCEPTION_POINTER_CHECK("", x, ;)
#define EXCEPTION_CHECK_INT(x)      __EXCEPTION_POINTER_CHECK("", x, -1)
#define EXCEPTION_CHECK_UINT(x)     __EXCEPTION_POINTER_CHECK("", x, 0)
#define EXCEPTION_CHECK_BOOL(x)     __EXCEPTION_POINTER_CHECK("", x, false)
#define EXCEPTION_CHECK_POINT(x)    __EXCEPTION_POINTER_CHECK("", x, NULL)
#define __EXCEPTION_POINTER_CHECK(tag, x, r) \
do { \
    if (!(x)) { \
        LOG_EXCP("%s:null pointer. line: %d", tag, __LINE__); \
        return r; \
    } \
} while (0)

#ifndef SET_BIT
#define SET_BIT(W,B)                ((W) |= (uint32_t)(1U << (B)))
#endif

#ifndef CLR_BIT
#define CLR_BIT(W, B)               ((W) &= (~((uint32_t)1U << (B))))
#endif

#ifndef MIN
#define MIN(a, b)                   ((a) < (b) ? (a) : (b))
#endif

#ifndef MAX
#define MAX(a, b)                   ((a) < (b) ? (b) : (a))
#endif

#ifndef BIT_MASK
#define BIT_MASK(x)                 (1 << (x))
#endif

/// 计算CRC16.
#define CRC16(data, len)            crc16_compute((const uint8_t *)(data), len, NULL)

// 将hex转换为ascii
#define HEX_TO_ASCII(hex, len, out)  hex_to_ascii((const unsigned char *)(hex), len, (char *)(out))

/// 空指针检查判断.
#define CHECK_IF_VALID(x, closure) \
do { \
    if (x) { \
        {closure} \
    } \
} while (0)

#define INTERVAL_SECOND_EACH_TIME_ZONE  3600    /// sec, (60*60)

#if IS_SUPPORT_IMU_FIFO
#define dump_imu(__imu) \
do { \
    LOGW("acc(%d, %d, %d) gyro(%d, %d, %d)\n", \
        (__imu)->un.fifo.data[0].acc.x, (__imu)->un.fifo.data[0].acc.y, (__imu)->un.fifo.data[0].acc.z,  \
        (__imu)->un.fifo.data[0].gyro.x, (__imu)->un.fifo.data[0].gyro.y, (__imu)->un.fifo.data[0].gyro.z); \
} while (0)
#else
#define dump_imu(__imu) \
do { \
    LOGW("acc(%d, %d, %d) gyro(%d, %d, %d)\n", \
        (__imu)->un.raw.acc.x, (__imu)->un.raw.acc.y, (__imu)->un.raw.acc.z,  \
        (__imu)->un.raw.gyro.x, (__imu)->un.raw.gyro.y, (__imu)->un.raw.gyro.z); \
} while (0)
#endif

#define dump_fsensor(data) \
do { \
    f_sensor_data_t *__fsensor = (f_sensor_data_t *)data; \
    LOGW("fsensor adc(%d, %d) value(%d, %d)\n", __fsensor->adc[0], \
        __fsensor->adc[1], __fsensor->value[0], __fsensor->value[1]); \
} while (0)

/// 通用帧率打印方法.
#define PRINTF_FPS(tag) LOG_FPS(tag, LOGW)
#define LOG_FPS(tag, log) \
do { \
    static uint16_t fps = 0; \
    static uint32_t time_last = 0; \
    uint32_t current = get_timestamp(); \
    if (time_last == 0) { \
        time_last = current; \
    }  \
    else if (current - time_last >= 1000) { \
        log("[fps-%s: %d]", (tag), fps); \
        fps = 0; \
        time_last = 0; \
    }  \
    else { \
        fps++; \
    } \
} while (0)

/*
 * GLOBAL FUNCTION DECLARATION
 *****************************************************************************************
 */
unsigned short crc16_compute(unsigned char const *p_data, unsigned int size, unsigned short const *p_crc);
void hex_to_ascii(const unsigned char *hex, int len, char *out);
#ifdef __cplusplus
}
#endif

#endif 


