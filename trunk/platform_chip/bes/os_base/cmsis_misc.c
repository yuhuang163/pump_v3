/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "pmu.h"
#include "watchdog/watchdog.h"
#include "norflash_api.h"
#include "app_flash_api.h"
#include "hal_bootmode.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "apps.h"            // IWYU pragma: keep
#include "bt_drv_interface.h"
#include "nvrecord_dev.h"
#include "bt_drv_reg_op.h"

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
    enum NORFLASH_API_MODULE_ID_T mod_id;
    uint32_t base_addr = 0;
    uint32_t offset = 0;
    enum NORFLASH_API_RET_T ret;
    
    if (strcmp(partition, PARTITION_FDS_NAME) == 0) {
        if ((addr + size) > FDS_END_ADDR) {
            LOGE("platform_flash_read: partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        mod_id = NORFLASH_API_MODULE_ID_LITTLEFS;
    } else if (strcmp(partition, PARTITION_FACTORY_DATA_NAME) == 0) {
        if ((addr + size) > FACTORY_DATA_END_ADDR) {
            LOGE("platform_flash_read: partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        mod_id = NORFLASH_API_MODULE_ID_FACTORY_DATA;
    }
    else if (strcmp(partition, PARTITION_PICTURE_DATA_INT_NAME) == 0) {
        if ((addr + size) > PICTURE_DATA_INT_END_ADDR) {
            LOGE("platform_flash_read: partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        mod_id = NORFLASH_API_MODULE_ID_LUTE_PICTURE_RESOURCES;
    }
    else if (strcmp(partition, PARTITION_PICTURE_DATA_EXT_NAME) == 0) {
        LOGE("platform_flash_read: partition = %s, addr = %x, size = %x", partition, addr, size);
        return -1;
    }
    else {
        return -1;
    }
    
    /// 获取模块基地址，将绝对地址转换为相对偏移
    ret = norflash_api_get_base_addr(mod_id, &base_addr);
    if (ret != NORFLASH_API_OK) {
        LOGE("platform_flash_read: get base addr failed, ret = %d", ret);
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
    enum NORFLASH_API_MODULE_ID_T mod_id;
    uint32_t base_addr = 0;
    uint32_t offset = 0;
    enum NORFLASH_API_RET_T ret;
    
    if (strcmp(partition, PARTITION_FDS_NAME) == 0) {
        if ((addr + size) > FDS_END_ADDR) {
            LOGE("platform_flash_write: partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        mod_id = NORFLASH_API_MODULE_ID_LITTLEFS;
    } else if (strcmp(partition, PARTITION_FACTORY_DATA_NAME) == 0) {
        if ((addr + size) > FACTORY_DATA_END_ADDR) {
            LOGE("platform_flash_write: partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        mod_id = NORFLASH_API_MODULE_ID_FACTORY_DATA;
    }
    else if (strcmp(partition, PARTITION_PICTURE_DATA_INT_NAME) == 0) {
        if ((addr + size) > PICTURE_DATA_INT_END_ADDR) {
            LOGE("platform_flash_write: partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        mod_id = NORFLASH_API_MODULE_ID_LUTE_PICTURE_RESOURCES;
    }
    else if (strcmp(partition, PARTITION_PICTURE_DATA_EXT_NAME) == 0) {
        LOGE("platform_flash_write: partition = %s, addr = %x, size = %x", partition, addr, size);
        return -1;
    }
    else {
        return -1;
    }
    
    /// 获取模块基地址，将绝对地址转换为相对偏移
    ret = norflash_api_get_base_addr(mod_id, &base_addr);
    if (ret != NORFLASH_API_OK) {
        LOGE("platform_flash_write: get base addr failed, ret = %d", ret);
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
    enum NORFLASH_API_MODULE_ID_T mod_id;
    uint32_t base_addr = 0;
    uint32_t offset = 0;
    enum NORFLASH_API_RET_T ret;
    
    if (strcmp(partition, PARTITION_FDS_NAME) == 0) {
        if ((addr + size) > FDS_END_ADDR) {
            LOGE("platform_flash_erase: partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        mod_id = NORFLASH_API_MODULE_ID_LITTLEFS;
    } else if (strcmp(partition, PARTITION_FACTORY_DATA_NAME) == 0) {
        if ((addr + size) > FACTORY_DATA_END_ADDR) {
            LOGE("platform_flash_erase: partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        mod_id = NORFLASH_API_MODULE_ID_FACTORY_DATA;
    }
    else if (strcmp(partition, PARTITION_PICTURE_DATA_INT_NAME) == 0) {
        if ((addr + size) > PICTURE_DATA_INT_END_ADDR) {
            LOGE("platform_flash_erase: partition = %s, addr = %x, size = %x", partition, addr, size);
            return -1;
        }
        mod_id = NORFLASH_API_MODULE_ID_LUTE_PICTURE_RESOURCES;
    }
    else if (strcmp(partition, PARTITION_PICTURE_DATA_EXT_NAME) == 0) {
        LOGE("platform_flash_erase: partition = %s, addr = %x, size = %x", partition, addr, size);
        return -1;
    }
    else {
        return -1;
    }
    
    /// 获取模块基地址，将绝对地址转换为相对偏移
    ret = norflash_api_get_base_addr(mod_id, &base_addr);
    if (ret != NORFLASH_API_OK) {
        LOGE("platform_flash_erase: get base addr failed, ret = %d", ret);
        return -1;
    }
    
    /// 计算相对偏移（addr是绝对地址，需要减去基地址）
    offset = addr - base_addr;
    
    /// 使用app_flash_erase支持跨扇区/跨块擦除
    app_flash_erase(mod_id, offset, size);
    
    return 0;
}

/// ======================== 蓝牙测试相关函数封装 ========================

/*
* EXTERNAL FUNCTION DECLARATIONS (蓝牙测试相关)
*****************************************************************************************
*/
// 前向声明，避免头文件依赖问题
// app_enter_signalingtest_mode 和 app_enter_non_signalingtest_mode 已在 apps.h 中声明
extern int app_factorymode_bt_xtalcalib_proc(void);

void platform_bt_enter_signalingtest_mode(void)
{
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
    app_enter_signalingtest_mode();
}

void platform_bt_enter_nosignalingtest_mode(void)
{
    hal_sw_bootmode_set(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
    app_enter_non_signalingtest_mode();
}

int platform_bt_xtal_calib_proc_start(void)
{
    // 前向声明，函数在 app_factory_bt.cpp 中定义（需要 __FACTORY_MODE_SUPPORT__ 宏）
    extern int app_factorymode_bt_xtalcalib_proc(void);
#ifdef __FACTORY_MODE_SUPPORT__
    //return app_factorymode_bt_xtalcalib_proc();
#else
    // 如果 FACTORY_MODE 未启用，返回错误
    return 0;
#endif
}

unsigned char platform_bt_xtal_calib_set(uint32_t fcap)
{
/*   btdrv_vco_test_start(0);
    osDelay(200);
    btdrv_rf_set_xtal_fcap(fcap, 1);//todo
    osDelay(200);*/ 
    nvrec_dev_set_xtal_fcap(fcap);
    return 0;
}

unsigned char platform_bt_xtal_calib_get(uint32_t *fcap)
{
    nvrec_dev_get_xtal_fcap(fcap);
    osDelay(200);

    return 0;
}

unsigned char platform_bt_xtal_calib_end(void)
{
    /*nvrec_dev_set_xtal_fcap(btdrv_rf_get_xtal_fcap());
    osDelay(200);
    btdrv_vco_test_stop();
    osDelay(200);*/
    return 0;
}

int platform_bt_get_rssi_value(int32_t *rssi)
{
    rx_agc_t rx_val;
    //bool ret = bt_drv_reg_op_read_rssi_in_dbm(0x80, &rx_val);
    //if (ret) {
    //    *rssi = rx_val.rssi;
    //}
    bool ret = bt_drv_reg_op_read_ble_rssi_in_dbm(0x00, &rx_val);
    if (ret) {
        *rssi = rx_val.rssi;
        return 0;
    }
    return -1;
}
