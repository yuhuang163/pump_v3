/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "stdio.h"
#include <stdarg.h>
#include "hal_trace.h"
#include "cmsis_os_api.h"

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
