/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "pmu.h"
#include "hal_trace.h"
#include "watchdog/watchdog.h"
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
void platform_prepare_init(void)
{
    u_heap_init();
    os_mutex_module_init();
    platform_set_utc_time(PRODUCT_BUILD_TIMESTAMP, 8);
    LOGI("platform_prepare_init");
}

void platform_post_init(void) 
{
    // TODO: Implement platform post-initialization logic
}

void platform_sysinfo_print(void *user_data)
{
    u_heap_info_output();
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
    utc_time_t utc_time = {0};
    pmu_rtc_calendar_get(&utc_time);
    return utc_time_to_timestamp(&utc_time);
}

uint64_t platform_get_utc_time_ms(void)
{
    return platform_get_utc_time() * 1000;
}

void platform_set_utc_time(uint32_t timestemp, int timezone)
{
    utc_time_t utc_time = {0};
    timestamp_to_utc_time(timestemp, &utc_time);
    LOGI("utc_time: %d-%d-%d %d:%d:%d", utc_time.year, utc_time.month, 
        utc_time.day, utc_time.hour, utc_time.minute, utc_time.second);
    pmu_rtc_calendar_set(&utc_time);
}

void platform_get_build_time(char *build_time)
{
    sprintf(build_time, "%s %s", __DATE__, __TIME__);
}