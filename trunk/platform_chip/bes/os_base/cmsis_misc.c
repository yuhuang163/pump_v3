/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_log_define.h"
#include "stdio.h"
#include "string.h" // IWYU pragma: keep
#include "pmu.h"
#include "watchdog/watchdog.h"

#include "hal_trace.h"
#include "cmsis_os_api.h"    // IWYU pragma: keep
#include "root_common_api.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define WDT_TIMEOUT_MS                   (6)
#define WDT_REFRESH_INTERVAL_MS          (WDT_TIMEOUT_MS / 2 * 1000)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void platform_wdt_init(void) 
{
    app_wdt_open(15);
}

static void platform_wdt_refresh(void) 
{    
    watchdog_ping();
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void msleep(uint32_t time_ms)
{
    os_delay_wrapper(time_ms);
}

void platform_prepare_init(void)
{
    u_heap_init();
    os_mutex_module_init();
}

void platform_post_init(void) 
{
    // TODO: Implement platform post-initialization logic
}

void platform_sysinfo_print(void *user_data)
{
    u_heap_info_output();
}

uint32_t get_timestamp(void)
{
    return os_kernel_systick_wrapper();
}

int platform_get_tid(void)
{
    os_thread_id tid = os_thread_get_id_wrapper();
    return ((int)tid) % 1000;
}

void platform_system_reboot(void)
{
    LOGW("======bes_restart!======");
    pmu_reboot();
}

uint32_t platform_get_utc_time(void)
{
    return 0;
}

uint64_t platform_get_utc_time_ms(void)
{
    return 0;
}

void platform_set_utc_time(uint32_t timestemp, int timezone)
{
    // TODO: Implement UTC time setting logic
    (void)timestemp;  // Suppress unused parameter warning
    (void)timezone;   // Suppress unused parameter warning
}

void platform_get_build_time(char *build_time)
{
    sprintf(build_time, "%s %s", __DATE__, __TIME__);
}