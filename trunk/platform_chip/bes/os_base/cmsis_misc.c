/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "pmu.h"
#include "watchdog/watchdog.h"
#include "norflash_api.h"
#include "hal_bootmode.h"
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
static uint32_t s_bootmode = 0;

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

static char *platform_get_reboot_reason(uint32_t bootmode)
{
    if (bootmode & HAL_SW_BOOTMODE_REBOOT_FROM_CRASH) {
        return "crash/wdt";
    } else if (bootmode & HAL_SW_BOOTMODE_ENTER_HIDE_BOOT) {
        return "hide/ota boot";
    } else if (bootmode & HAL_SW_BOOTMODE_REBOOT) {
        return "reboot";
    }  else if (bootmode == HAL_SW_BOOTMODE_DLD_TRANS_UART) {
        return "power on/hw reset";
    } else {
        return "unknown";
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void platform_prepare_init(void)
{
    u_heap_init();
    os_mutex_module_init();
    s_bootmode = hal_sw_bootmode_get();
    LOGI("bootmode: %x, reason: %s", s_bootmode, platform_get_reboot_reason(s_bootmode));
}

bool platform_is_startup_caused_by_reboot(void)
{
    if (s_bootmode & HAL_SW_BOOTMODE_REBOOT) {
        return true;
    } else if (s_bootmode & HAL_SW_BOOTMODE_REBOOT_FROM_CRASH) {
        return true;
    } else if (s_bootmode & HAL_SW_BOOTMODE_ENTER_HIDE_BOOT) {
        return true;
    } 
    return false;
}

void platform_post_init(void) 
{
    platform_set_utc_time(PRODUCT_BUILD_TIMESTAMP, 8);
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
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_REBOOT);
    pmu_reboot();
}

uint32_t platform_get_utc_time(void)
{
    utc_time_t utc_time = {0};
    pmu_rtc_calendar_get((struct RTC_CALENDAR_FORMAT_T *)&utc_time);
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
    pmu_rtc_calendar_set((struct RTC_CALENDAR_FORMAT_T *)&utc_time);
}

void platform_get_build_time(char *build_time)
{
    sprintf(build_time, "%s %s", __DATE__, __TIME__);
}

uint32_t platform_int_lock_global(void)
{
    // LOGI("platform_int_lock_global");
    return int_lock_global();
}

void platform_int_unlock_global(uint32_t pri)
{
    // LOGI("platform_int_unlock_global: pri = %x", pri);
    int_unlock_global(pri);
}


int platform_flash_read(const char *partition, uint32_t addr, uint8_t *data, uint32_t size)
{
    if (strcmp(partition, PARTITION_FDS_NAME) == 0) {
        if ((addr + size) > FDS_END_ADDR) {
            LOGE("platform_flash_read: partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        return norflash_api_read(NORFLASH_API_MODULE_ID_LITTLEFS, addr, data, size);
    } else if (strcmp(partition, PARTITION_FACTORY_DATA_NAME) == 0) {
        if ((addr + size) > FACTORY_DATA_END_ADDR) {
            LOGE("platform_flash_read: partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        return norflash_api_read(NORFLASH_API_MODULE_ID_FACTORY_DATA, addr, data, size);
    }
    return -1;
}

int platform_flash_write(const char *partition, uint32_t addr, uint8_t *data, uint32_t size)
{
    if (strcmp(partition, PARTITION_FDS_NAME) == 0) {
        if ((addr + size) > FDS_END_ADDR) {
            LOGE("platform_flash_write: partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        return norflash_api_write(NORFLASH_API_MODULE_ID_LITTLEFS, addr, data, size, false);
    } else if (strcmp(partition, PARTITION_FACTORY_DATA_NAME) == 0) {
        if ((addr + size) > FACTORY_DATA_END_ADDR) {
            LOGE("platform_flash_write: partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        return norflash_api_write(NORFLASH_API_MODULE_ID_FACTORY_DATA, addr, data, size, false);
    }
    return -1;
}

int platform_flash_erase(const char *partition, uint32_t addr, uint32_t size)
{
    if (strcmp(partition, PARTITION_FDS_NAME) == 0) {
        if ((addr + size) > FDS_END_ADDR) {
            LOGE("platform_flash_erase: partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        return norflash_api_erase(NORFLASH_API_MODULE_ID_LITTLEFS, addr, size, false);
    } else if (strcmp(partition, PARTITION_FACTORY_DATA_NAME) == 0) {
        if ((addr + size) > FACTORY_DATA_END_ADDR) {
            LOGE("platform_flash_erase: partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        return norflash_api_erase(NORFLASH_API_MODULE_ID_FACTORY_DATA, addr, size, false);
    }
    return -1;
}