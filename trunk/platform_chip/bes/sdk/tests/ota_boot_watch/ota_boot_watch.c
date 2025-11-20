/***************************************************************************
 *
 * Copyright 2015-2019 BES.
 * All rights reserved. All unpublished rights reserved.
 *
 * No part of this work may be used or reproduced in any form or by any
 * means, or stored in a database or retrieval system, without prior written
 * permission of BES.
 *
 * Use of this work is governed by a license granted by BES.
 * This work contains confidential and proprietary information of
 * BES. which is protected by copyright, trade secret,
 * trademark and other intellectual property rights.
 *
 ****************************************************************************/
#include "plat_addr_map.h"
#include "cmsis_nvic.h"
#include "hal_psc.h"
#include "hal_dma.h"
#include "hal_cache.h"
#include "hal_iomux.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_location.h"
#include "hal_norflash.h"
#include "main_entry.h"
#include "tool_msg.h"
#include "stdio.h"
#include "ota_boot_handle.h"

#ifdef BOOT_USE_STORAGE_NOFS
#include "fs/fs_nofs.h"
#include <string.h>
#ifndef O_RDWR
#define O_RDWR    02
#endif
#endif

#ifdef OTA_BARE_BOOT
#define OTA_TRACE(...)
#define OTA_TRACE_FLASH()
#else
#define OTA_TRACE(str, ...)             TRACE_IMM(str, ##__VA_ARGS__)
#define OTA_TRACE_FLASH()               TRACE_FLUSH()
#endif

#define DISABLE_ALL_IRQS()              NVIC_DisableAllIRQs()

#define USE_MAIN_FLASH                  (1<<0)
#define USE_BTH_FLASH                   (1<<1)
#define USE_SENSOR_FLASH                (1<<2)

extern uint32_t __ota_boot_info_start[];
extern uint32_t __ota_boot_info_bak_start[];

typedef void (*FLASH_ENTRY)(void);

static uint32_t flashMode = 0;

void flash_mode_init(void)
{
#ifndef CHIP_SUBSYS_BTH
    flashMode = flashMode | USE_MAIN_FLASH;
#endif

#ifdef SYS_USE_BTH_FLASH
    flashMode = flashMode | USE_BTH_FLASH;
#endif

#ifdef SYS_USE_SENSOR_FLASH
    flashMode = flashMode | USE_SENSOR_FLASH;
#endif
}

void flash_mode_clear(uint32_t mode)
{
    flashMode = flashMode & ~mode;
}

void main_flash_init(void)
{
#if !defined(SIMU) && !defined(FPGA)
    uint8_t flash_id[HAL_NORFLASH_DEVICE_ID_LEN];
    hal_norflash_get_id(HAL_FLASH_ID_0, flash_id, ARRAY_SIZE(flash_id));
    OTA_TRACE(TR_MOD(MAIN),"FLASH_ID: %02X-%02X-%02X", flash_id[0], flash_id[1], flash_id[2]);
#endif

    int ret = hal_norflash_init(HAL_FLASH_ID_0);
    ASSERT(ret == HAL_NORFLASH_OK, "%s: Failed to init flash: %d", __func__, ret);
}

void bth_flash_init(void)
{
    OTA_TRACE(0, "OTA boot to single flash 1 chip");
    OTA_TRACE_FLASH();
    #ifdef SYS_USE_BTH_FLASH
    int ret = hal_norflash_init(HAL_FLASH_ID_1);
    ASSERT(ret == HAL_NORFLASH_OK, "%s: Failed to enable chip 1 mode: %d", __func__, ret);
    #endif
}

void sensor_hub_flash_init(void)
{
#ifdef SYS_USE_SENSOR_FLASH
    hal_psc_sens_enable();
    hal_cmu_sens_clock_enable();
    hal_cmu_sens_reset_clear();
    osDelay(2);

    enum HAL_FLASH_ID_T id;
    enum HAL_NORFLASH_RET_T ret = HAL_NORFLASH_ERR;

    id = hal_norflash_addr_to_id(SENS_FLASH_BOOT_STRUCT_ADDR);
    ASSERT(id < HAL_FLASH_ID_NUM, "%s: addr not in flash: 0x%08X", __func__, SENS_FLASH_BOOT_STRUCT_ADDR);

    OTA_TRACE(0, "###sensor_hub_open flashid %d",id);
    if (id != HAL_FLASH_ID_0) {
        hal_norflash_force_wakeup(id);

        const struct HAL_NORFLASH_CONFIG_T cfg = {
            .source_clk = HAL_NORFLASH_SPEED_13M * 2,
            .speed = HAL_NORFLASH_SPEED_13M,
            .mode = HAL_NORFLASH_OP_MODE_STAND_SPI |
                    HAL_NORFLASH_OP_MODE_FAST_SPI |
                    HAL_NORFLASH_OP_MODE_PAGE_PROGRAM |
                    0,
            .opt =  HAL_NORFLASH_OPT_CALIB_FLASH_ID |
                    0,
        };

        ret = hal_norflash_open(id, &cfg);
        ASSERT(ret == HAL_NORFLASH_OK, "%s: Failed to enable chip 2 mode: %d", __func__, ret);
    }
    OTA_TRACE(0, "sensor flash open ret %d",ret);
#endif
}

void boot_flash_deinit(void)
{
    enum HAL_NORFLASH_RET_T ret = HAL_NORFLASH_ERR;

    if (flashMode & USE_MAIN_FLASH) {
        ret = hal_norflash_close(HAL_FLASH_ID_0);
        ASSERT(ret == HAL_NORFLASH_OK, "%s: Failed to enable chip 0 mode: %d", __func__, ret);
    }
    if (flashMode & USE_BTH_FLASH) {
        #ifdef SYS_USE_BTH_FLASH
        ret = hal_norflash_close(HAL_FLASH_ID_1);
        #endif
        ASSERT(ret == HAL_NORFLASH_OK, "%s: Failed to close chip 1 mode: %d", __func__, ret);
    }
    if (flashMode & USE_SENSOR_FLASH) {
        #ifdef SYS_USE_SENSOR_FLASH
        enum HAL_FLASH_ID_T id = hal_norflash_addr_to_id(SENS_FLASH_BOOT_STRUCT_ADDR);
        ret = hal_norflash_close(id);
        #endif
        ASSERT(ret == HAL_NORFLASH_OK, "%s: Failed to close chip 2 mode: %d", __func__, ret);
    }
}

void boot_flash_init(void)
{
    flash_mode_init();
    OTA_TRACE(0, "boot_flash_init %d", flashMode);

    if (flashMode & USE_MAIN_FLASH) {
        main_flash_init();
    }
    if (flashMode & USE_BTH_FLASH) {
        bth_flash_init();
    }
    if (flashMode & USE_SENSOR_FLASH) {
        sensor_hub_flash_init();
    }
}

const FLASH_OTA_BOOT_INFO_T * get_boot_info()
{
    const FLASH_OTA_BOOT_INFO_T *ctrl_m, *ctrl_b, *ctrl;
    ctrl_m = (const FLASH_OTA_BOOT_INFO_T *)&__ota_boot_info_start[0];
    ctrl_b = (const FLASH_OTA_BOOT_INFO_T *)&__ota_boot_info_bak_start[0];

    if (ctrl_m->magicNumber == BOOT_MAGIC_NUMBER || ctrl_m->magicNumber == OTA_COPY_BLOCK_MAGIC) {
        ctrl = ctrl_m;
    } else if (ctrl_b->magicNumber == BOOT_MAGIC_NUMBER || ctrl_m->magicNumber == OTA_COPY_BLOCK_MAGIC) {
        ctrl = ctrl_b;
    } else {
        OTA_TRACE(0, "WARNING: Both ctrl block is bad: [%p]=0x%08X [%p]=0x%08X",
            &ctrl_m->magicNumber, ctrl_m->magicNumber, &ctrl_b->magicNumber, ctrl_b->magicNumber);
        ctrl = NULL;
    }

    if (ctrl) {
        OTA_TRACE(0, "Use ctrl block %s at %p",
            (ctrl == ctrl_m) ? "MAIN" : "BACKUP", ctrl);
    }
    return ctrl;
}

#ifdef BOOT_USE_STORAGE_NOFS
#define MOUNT_DEV "/dev/emmc0_2"
#define MOUNT_TRAGET "/data/emmc0_2"
POSSIBLY_UNUSED static void device_nonblock_wr_test()
{
    int ret = hal_aos_nofs_mount(MOUNT_DEV, MOUNT_TRAGET);
    ASSERT(ret == 0, "mount failed");

    int fd = hal_aos_nofs_open(MOUNT_TRAGET, O_RDWR);
    ASSERT(fd > 0, "open failed ret=%d", fd);

    #define INBUFF_LEN  2048
    static char buff[INBUFF_LEN];
    for(int i = 0; i < INBUFF_LEN; i++) {
        buff[i] = 'F';
    }
    ret = hal_aos_nofs_write(fd, 10, buff, INBUFF_LEN);
    ASSERT(ret == INBUFF_LEN, "write failed ret=%d", ret);

    static char buff1[2048];
    memset(buff1, 0x00, INBUFF_LEN);
    ret = hal_aos_nofs_read(fd, 10, buff1, INBUFF_LEN);
    ASSERT(ret == INBUFF_LEN, "write failed ret=%d", ret);
    for(int i = 0; i < INBUFF_LEN; i++) {
        ASSERT(buff1[i] == 'F', "read failed, need F");
    }

    ret = hal_aos_nofs_close(fd);
    ASSERT(ret == 0, "close failed");

    ret = hal_aos_nofs_umount(MOUNT_TRAGET);
    ASSERT(ret == 0, "umount failed");
    OTA_TRACE(0, "OTA FILE TEST SUCCEED!");
}

POSSIBLY_UNUSED static void device_nonblock_read_test()
{
    int ret = hal_aos_nofs_mount(MOUNT_DEV, MOUNT_TRAGET);
    ASSERT(ret == 0, "mount failed");

    int fd = hal_aos_nofs_open(MOUNT_TRAGET, O_RDWR);
    ASSERT(fd > 0, "open failed ret=%d", fd);

    static char buff[INBUFF_LEN];

    ret = hal_aos_nofs_read(fd, 10, buff, INBUFF_LEN);
    ASSERT(ret == INBUFF_LEN, "write failed ret=%d", ret);
    for(int i = 0; i < 20; i++){
        DUMP8("%02x ",buff + i * 50, 50);
    }
    DUMP8("%02x ",buff + 1000, 24);

    ret = hal_aos_nofs_close(fd);
    ASSERT(ret == 0, "close failed");

    ret = hal_aos_nofs_umount(MOUNT_TRAGET);
    ASSERT(ret == 0, "umount failed");
}
#endif

void ota_boot(void)
{
    const struct boot_struct_t *boot_struct;
    FLASH_ENTRY entry;
    bool retry_done = false;

_retry_normal:
    // Normal boot
    OTA_TRACE(0, "---> Normal boot");

#ifdef FLASH_DEC_ENABLE
    hal_norflash_dec_enable(HAL_FLASH_ID_0);
    hal_cache_invalidate(HAL_CACHE_ID_I_CACHE, (uint32_t)(FLASH_NC_BASE + FLASH_DEC_OFFSET), FLASH_DEC_SIZE);
    hal_cache_invalidate(HAL_CACHE_ID_D_CACHE, (uint32_t)(FLASH_NC_BASE + FLASH_DEC_OFFSET), FLASH_DEC_SIZE);
    OTA_TRACE(0, "DEC ENABLE");
#elif defined(FLASH_CRYPTO_ENABLE) && (CHIP_FLASH_CTRL_VER >= 9)
    hal_norflash_crypto_enable(HAL_FLASH_ID_0, HAL_NORFLASH_CRYPTO_ID_0);
#if defined(FLASH_CRYPTO_ADDR0) && defined(FLASH_CRYPTO_SIZE0)
    hal_cache_invalidate(HAL_CACHE_ID_I_CACHE, (uint32_t)(FLASH_NC_BASE + FLASH_CRYPTO_ADDR0), FLASH_CRYPTO_SIZE0);
    hal_cache_invalidate(HAL_CACHE_ID_D_CACHE, (uint32_t)(FLASH_NC_BASE + FLASH_CRYPTO_ADDR0), FLASH_CRYPTO_SIZE0);
#endif
    OTA_TRACE(0, OTA_BOOT_STR("CRYPTO ENABLE"));
#endif

#ifdef BOOT_USE_STORAGE_NOFS
#if 0
    // device_nonblock_wr_test();//boot write,boot read
    device_nonblock_read_test();//app write,boot read
#endif
#endif

    boot_struct = (const struct boot_struct_t *)(FLASH_NC_BASE + OTA_CODE_OFFSET);

    OTA_TRACE(0, "app addr:0x%x 0x%x", (FLASH_NC_BASE + OTA_CODE_OFFSET), boot_struct->hdr.magic);

    if (boot_struct->hdr.magic == BOOT_MAGIC_NUMBER) {
        entry = (FLASH_ENTRY)((uint32_t)(&boot_struct->hdr + 1));
        entry = (FLASH_ENTRY)((uint32_t)entry | 1);
        OTA_TRACE_FLASH();
        // Disable all IRQs
        DISABLE_ALL_IRQS();
        flash_mode_clear(USE_MAIN_FLASH);
        boot_flash_deinit();
        entry();
        OTA_TRACE(0, "Returning from normal code");
    } else {
        OTA_TRACE(0, "*** ERROR: Code magic is bad: [%p]=0x%08X",
            &boot_struct->hdr.magic, boot_struct->hdr.magic);
        if (retry_done) {
            goto _err;
        } else {
            retry_done = true;
            OTA_TRACE(0, "Retrying normal boot");
            goto _retry_normal;
        }
    }
_err:
    boot_flash_deinit();
    return;
}

#ifdef APP_BOOT_GPIO_CONTROL
#include "hal_iomux.h"
void programmer_set_gpio_up(void)
{
    static const struct HAL_IOMUX_PIN_FUNCTION_MAP gpio_up = {
        HAL_IOMUX_PIN_P4_5, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLUP_ENABLE
    };

    hal_iomux_init((struct HAL_IOMUX_PIN_FUNCTION_MAP *)&gpio_up, 1);

    hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)gpio_up.pin, HAL_GPIO_DIR_OUT, 1);
}
#endif

// GDB can set a breakpoint on the main function only if it is
// declared as below, when linking with STD libraries.

// GDB can set a breakpoint on the main function only if it is
// declared as below, when linking with STD libraries.
int MAIN_ENTRY(void)
{

    hal_audma_open();
    hal_gpdma_open();

#if (DEBUG_PORT == 2)
    hal_iomux_set_analog_i2c();
    hal_iomux_set_uart1();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART1);
#else
    hal_iomux_set_uart0();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART0);
#endif

#ifdef APP_BOOT_GPIO_CONTROL
    programmer_set_gpio_up();
#endif

    boot_flash_init();
    boot2_watch_ota_handler(get_boot_info());
    ota_boot();

    SAFE_PROGRAM_STOP();
    return 0;
}
