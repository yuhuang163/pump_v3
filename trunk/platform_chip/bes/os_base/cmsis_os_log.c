/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "stdio.h"
#include <stdarg.h>
#include "hal_trace.h"
#include "cmsis_os_api.h"
#include "product_info.h"
#include "root_log_define.h"

/*
* DEFINES
*****************************************************************************************
*/
#define PLATFORM_PRINTF_LEN    512

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
char log_buf[PLATFORM_PRINTF_LEN];

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int platform_format_va(char *buf, unsigned int size, const char *fmt, va_list ap)
{
    int len;

    len = vsnprintf(buf, size, fmt, ap);
    if (len < 0) {
        len = 0;
    } else if (len >= size) {
        len = (size > 1) ? (size - 1) : 0;
    }
    return len;
}

static int platform_printf_internal(const char *fmt, va_list ap)
{
    int len = 0, ret = 0;
#if defined(CP_IN_SAME_EE) && !defined(CP_TRACE_ENABLE)
    if (get_cpu_id())
        return 0;
#endif
    len = platform_format_va(log_buf, sizeof(log_buf), fmt, ap);
    ret = hal_trace_output((unsigned char *)log_buf, len);

    return ret;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void platform_printf(const char *fmt, ...)
{
    va_list ap;
    va_start(ap, fmt);
    platform_printf_internal(fmt, ap);
    va_end(ap);
}

char *platform_log_header(void)
{
    static char log_header[32] = {0};
    snprintf(log_header, sizeof(log_header), "[%lld][%d]", platform_get_utc_time_ms(), platform_get_tid());
    return log_header;
}

void platform_log_init(void)
{
    /// 设置日志等级
#if IS_SUPPORT_LOG_OUT
    hal_trace_set_log_level(TR_LEVEL_ERROR);
#else
    hal_trace_set_log_level(TR_LEVEL_CRITICAL);
#endif

    /// 打印日志头
    LOG_STEP("********Welcome to root develop platform.********");
    char build_time[32] = {0};
    platform_get_build_time(build_time);
    LOG_STEP("BUILD_TIME: %s", build_time);
    LOG_STEP("DEVICE_NAME: %s", ROOT_DEVICE_NAME);
    LOG_STEP("FIRMWARE_TYPE: %c", ROOT_FIRMWARE_TYPE);
    LOG_STEP("FIRMWARE_VERSION: %d", ROOT_FIRMWARE_VERSION);
    LOG_STEP("HARDWARE_VERSION: %02d", ROOT_HARDWARE_VERSION);
    LOG_STEP("RES_VERSION: %d", ROOT_RES_VERSION);
    LOG_STEP("BLE_PROTOCOL_VERSION: %d", ROOT_BLE_PROTOCOL_VERSION);
    LOG_STEP("PRODUCT_ID: 0x%x", ROOT_PRODUCT_ID);
    LOG_STEP("****************************************************\n");
}