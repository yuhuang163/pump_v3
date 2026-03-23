/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "pmu.h"
#include "watchdog/watchdog.h"
#include "norflash_api.h"
#include "app_flash_api.h"
#include "spi_flash.h"
#include "hal_bootmode.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "apps.h"            // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define WDT_TIMEOUT_MS                   (6)
#define WDT_REFRESH_INTERVAL_MS          (WDT_TIMEOUT_MS / 2 * 1000)
#define EXTERNAL_FLASH_ID                0

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
        return "null";
    }
}

static char *platform_get_boot_cause(enum PMU_BOOT_CAUSE_T cause)
{
    switch (cause) {
        case PMU_BOOT_CAUSE_POWER_KEY:
            return "power key";
        case PMU_BOOT_CAUSE_DIG_WDT:
            return "dig wdt";
        case PMU_BOOT_CAUSE_DIG_REBOOT:
            return "dig reboot";
        case PMU_BOOT_CAUSE_AC_IN:
            return "ac in";
        case PMU_BOOT_CAUSE_AC_OUT:
            return "ac out";
        case PMU_BOOT_CAUSE_RTC:
            return "rtc";
        case PMU_BOOT_CAUSE_PMU_WDT:
            return "pmu wdt";
        case PMU_BOOT_CAUSE_RESET_KEY:
            return "reset key";
        case PMU_BOOT_CAUSE_LONG_PRESS:
            return "long press";
        case PMU_BOOT_CAUSE_VCHG_COMMAND:
            return "vchg command";
        case PMU_BOOT_CAUSE_VCORE_LOW:
            return "vcore low";
        case PMU_BOOT_CAUSE_VBAT_OVP:
            return "vbat ovp";
        default:
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
    enum PMU_BOOT_CAUSE_T boot_cause = pmu_boot_cause_get();
    LOGI("bootmode: %x-%s, boot cause: %x-%s", s_bootmode, platform_get_reboot_reason(s_bootmode),
        boot_cause, platform_get_boot_cause(boot_cause));
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

void platform_sleep_hook(uint8_t cpu_sleep, uint8_t bus_sleep, uint8_t subsys_sleep)
{
    uint8_t busy = 100 - (cpu_sleep + bus_sleep + subsys_sleep);
    LOG_PASS("busy: %d, cpu_sleep: %d, bus_sleep: %d, subsys_sleep: %d\r\n", busy, cpu_sleep, bus_sleep, subsys_sleep);
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
    int mod_id, ret;
    uint32_t base_addr = 0;
    uint32_t offset = 0;
    EXCEPTION_CHECK_INT(data);
    if (strcmp(partition, PARTITION_FDS_NAME) == 0) {
        if ((addr + size) > FDS_END_ADDR) {
            LOGE("read failed, partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        mod_id = NORFLASH_API_MODULE_ID_LITTLEFS;
    } else if (strcmp(partition, PARTITION_FACTORY_DATA_NAME) == 0) {
        if ((addr + size) > FACTORY_DATA_END_ADDR) {
            LOGE("read failed, partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        mod_id = NORFLASH_API_MODULE_ID_FACTORY_DATA;
    } else if (strcmp(partition, PARTITION_IMG_INT_DATA_NAME) == 0) {
        if ((addr + size) > IMG_INT_DATA_END_ADDR) {
            LOGE("read failed, partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        mod_id = NORFLASH_API_MODULE_ID_LUTE_PICTURE_RESOURCES;
    } else if (strcmp(partition, PARTITION_IMG_EXT_DATA_NAME) == 0) {
        ret = spi_norflash_read(EXTERNAL_FLASH_ID, (EXTERNAL_FLASH_BASE_ADDR + addr), data, size);
        if (ret != 0) {
            LOGE("spi_norflash_read failed, ret = %d", ret);
            return -1;
        }
        return 0;
    } else {
        LOGE("not supported partition = %s, addr = %x, size = %x", partition, addr, size);
        return -1;
    }
    
    /// 获取模块基地址，将绝对地址转换为相对偏移
    ret = norflash_api_get_base_addr(mod_id, &base_addr);
    if (ret != NORFLASH_API_OK) {
        LOGE("read failed, get base addr failed, ret = %d", ret);
        return -1;
    }
    
    /// 计算相对偏移（addr是绝对地址，需要减去基地址）
    offset = addr - base_addr;
    
    /// 使用app_flash_read支持跨扇区读取
    app_flash_read(mod_id, offset, data, size);
    
    return 0;
}

int platform_flash_write(const char *partition, uint32_t addr, uint8_t *data, uint32_t size)
{
    int mod_id, ret;
    uint32_t base_addr = 0;
    uint32_t offset = 0;
    if (strcmp(partition, PARTITION_FDS_NAME) == 0) {
        if ((addr + size) > FDS_END_ADDR) {
            LOGE("write failed, partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        mod_id = NORFLASH_API_MODULE_ID_LITTLEFS;
    } else if (strcmp(partition, PARTITION_FACTORY_DATA_NAME) == 0) {
        if ((addr + size) > FACTORY_DATA_END_ADDR) {
            LOGE("write failed, partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        mod_id = NORFLASH_API_MODULE_ID_FACTORY_DATA;
    } else if (strcmp(partition, PARTITION_IMG_INT_DATA_NAME) == 0) {
        if ((addr + size) > IMG_INT_DATA_END_ADDR) {
            LOGE("write failed, partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        mod_id = NORFLASH_API_MODULE_ID_LUTE_PICTURE_RESOURCES;
    } else if (strcmp(partition, PARTITION_IMG_EXT_DATA_NAME) == 0) {
        ret = spi_norflash_write(EXTERNAL_FLASH_ID, (EXTERNAL_FLASH_BASE_ADDR + addr), data, size);
        if (ret != HAL_NORFLASH_OK) {
            LOGE("write failed, ret = %d", ret);
            return -1;
        }
        return 0;
    } else {
        LOGE("not supported partition = %s, addr = %x, size = %x", partition, addr, size);
        return -1;
    }
    
    /// 获取模块基地址，将绝对地址转换为相对偏移
    ret = norflash_api_get_base_addr(mod_id, &base_addr);
    if (ret != NORFLASH_API_OK) {
        LOGE("write failed, get base addr failed, ret = %d", ret);
        return -1;
    }
    
    /// 计算相对偏移（addr是绝对地址，需要减去基地址）
    offset = addr - base_addr;
    
    /// 使用app_flash_program支持跨扇区写入（同步写入）
    app_flash_program(mod_id, offset, data, size, true);
    
    return 0;
}

int platform_flash_erase(const char *partition, uint32_t addr, uint32_t size)
{
    int mod_id, ret;
    uint32_t base_addr = 0;
    uint32_t offset = 0;
    
    if (strcmp(partition, PARTITION_FDS_NAME) == 0) {
        /// 检测地址和大小是否4KB对齐
        if (addr % 4096 != 0 || size % 4096 != 0) {
            LOGE("erase failed, addr = %x, size = %x, not aligned to 4096", addr, size);
            //return -1;
        }
        if ((addr + size) > FDS_END_ADDR) {
            LOGE("erase failed, partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        mod_id = NORFLASH_API_MODULE_ID_LITTLEFS;
    } else if (strcmp(partition, PARTITION_FACTORY_DATA_NAME) == 0) {
        if ((addr + size) > FACTORY_DATA_END_ADDR) {
            LOGE("erase failed, partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        mod_id = NORFLASH_API_MODULE_ID_FACTORY_DATA;
    } else if (strcmp(partition, PARTITION_IMG_INT_DATA_NAME) == 0) {
        if ((addr + size) > IMG_INT_DATA_END_ADDR) {
            LOGE("erase failed, partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        mod_id = NORFLASH_API_MODULE_ID_LUTE_PICTURE_RESOURCES;
    } else if (strcmp(partition, PARTITION_IMG_EXT_DATA_NAME) == 0) {
        if (addr % EXTERNAL_FLASH_SECTOR_SIZE != 0 || size % EXTERNAL_FLASH_SECTOR_SIZE != 0) {
            LOGE("erase failed, addr = %x, size = %x, not aligned to 4096", addr, size);
            return -1;
        }
        ret = spi_norflash_erase(EXTERNAL_FLASH_ID, (EXTERNAL_FLASH_BASE_ADDR + addr), size);
        if (ret != 0) {
            LOGE("erase failed, ret = %d", ret);
            return -1;
        }
        return 0;
    } else {
        LOGE("not supported partition = %s, addr = %x, size = %x", partition, addr, size);
        return -1;
    }
    
    /// 获取模块基地址，将绝对地址转换为相对偏移
    ret = norflash_api_get_base_addr(mod_id, &base_addr);
    if (ret != NORFLASH_API_OK) {
        LOGE("erase failed, get base addr failed, ret = %d", ret);
        return -1;
    }
    
    /// 计算相对偏移（addr是绝对地址，需要减去基地址）
    offset = addr - base_addr;
    
    /// 使用app_flash_erase支持跨扇区/跨块擦除
    app_flash_erase(mod_id, offset, size);
    
    return 0;
}


