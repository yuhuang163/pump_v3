/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
#include "sensor_hub.h"
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_cmu.h"
#include "hal_rmt_ipc.h"
#ifdef SENS_FLASH_BOOT
#include "hal_norflash.h"
#endif
#include "hal_psc.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "rx_sens_trc.h"
#include "string.h"
#include "system_subsys.h"

#ifndef SENS_FLASH_BOOT_STRUCT_ADDR
#define SENS_FLASH_BOOT_STRUCT_ADDR         SENS_FLASH_NC_BASE
#endif

extern uint32_t __sensor_hub_code_start_flash[];
extern uint32_t __sensor_hub_code_end_flash[];

static HAL_RMT_IPC_EP_T rmt_ipc_ep;

int sensor_hub_open(SENSOR_HUB_RX_IRQ_HANDLER rx_hdlr, SENSOR_HUB_TX_IRQ_HANDLER tx_hdlr)
{
    uint32_t code_start;
    POSSIBLY_UNUSED int ret;
    uint32_t entry;
    uint32_t sp;

    hal_psc_sens_enable();
    hal_cmu_sens_clock_enable();
    hal_cmu_sens_reset_clear();

    osDelay(2);

    hal_cmu_sens_ram_cfg();

#ifdef SENS_FLASH_BOOT
    enum HAL_FLASH_ID_T id;

    id = hal_norflash_addr_to_id(SENS_FLASH_BOOT_STRUCT_ADDR);
    ASSERT(id < HAL_FLASH_ID_NUM, "%s: addr not in flash: 0x%08X", __func__, SENS_FLASH_BOOT_STRUCT_ADDR);

    if (id != HAL_FLASH_ID_0) {
        hal_norflash_force_wakeup(id);

#ifdef NO_SUBSYS_FLASH_ADDR_ACCESS
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
        ASSERT(ret == HAL_NORFLASH_OK, "%s: Failed to open flash: %d", __func__, ret);
#endif
    }

    ret = subsys_check_boot_struct(SENS_FLASH_BOOT_STRUCT_ADDR, &code_start);
    if (ret) {
        ASSERT(false, "%s: Bad subsys boot struct: %d", __func__, ret);
        return 1;
    }

#elif defined(SENS_LOAD_FILE_NAME)
    const char *file_name = TO_STRING(SENS_LOAD_FILE_NAME);

    ret = subsys_loader_load_image_fs(file_name, NULL, &code_start);
    if (ret) {
        ASSERT(false, "%s: Failed to load subsys simple image from %s: %d", __func__, file_name, ret);
        return 2;
    }

#else // !(SENS_FLASH_BOOT || SENS_LOAD_FILE_NAME)
    uint32_t addr;
    uint32_t len;
    int check_magic;

#ifdef SENS_LOAD_FLASH_ADDR
    addr = SENS_LOAD_FLASH_ADDR;
    len = SENS_LOAD_FLASH_SIZE;
    check_magic = true;
#else
    addr = (uint32_t)__sensor_hub_code_start_flash;
    len = (uint32_t)__sensor_hub_code_end_flash - addr;
    check_magic = false;
#endif

    ret = subsys_loader_load_image(addr, len, check_magic, NULL, &code_start);
    if (ret) {
        ASSERT(false, "%s: Failed to load subsys simple image: %d", __func__, ret);
        return 3;
    }

    ASSERT(code_start > SENS_RAM_BASE && code_start < (SENS_RAM_BASE + SENS_RAM_SIZE),
           "Invalid SENSOR RAM config!!! code_start %08x (RAM base %08x size %08x)",
            code_start, SENS_RAM_BASE, SENS_RAM_SIZE);

#endif // !(SENS_FLASH_BOOT || SENS_LOAD_FILE_NAME)

#ifndef NO_SENS_IPC_WHEN_OPEN
    ret = hal_rmt_ipc_open(HAL_RMT_IPC_CORE_SENS_C0, HAL_RMT_IPC_CHAN_0, rx_hdlr, tx_hdlr, false, &rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_open failed: %d", ret);

    ret = hal_rmt_ipc_start_recv(rmt_ipc_ep);
    ASSERT(ret == 0, "hal_rmt_ipc_start_recv failed: %d", ret);

#ifdef SENS_TRC_TO_MCU
    sensor_hub_trace_server_open();
#endif
#endif

    sp = SENS_MAILBOX_BASE;
    entry = code_start | 1;

    hal_cmu_sens_start_cpu(sp, entry);

    return 0;
}

int sensor_hub_opened(void)
{
    return hal_rmt_ipc_opened(rmt_ipc_ep);
}

int sensor_hub_close(void)
{
    TR_INFO(0, "%s", __FUNCTION__);

#ifdef SENS_TRC_TO_MCU
    sensor_hub_trace_server_close();
#endif

    hal_rmt_ipc_close(rmt_ipc_ep);

    hal_cmu_sens_reset_set();
    hal_cmu_sens_clock_disable();
    hal_psc_sens_disable();

    return 0;
}

int sensor_hub_send_seq(const void *data, unsigned int len, unsigned int *seq)
{
    return hal_rmt_ipc_send_seq(rmt_ipc_ep, data, len, seq);
}

int sensor_hub_send(const void *data, unsigned int len)
{
    return hal_rmt_ipc_send(rmt_ipc_ep, data, len);
}

int sensor_hub_tx_active(unsigned int seq)
{
    return hal_rmt_ipc_tx_active(rmt_ipc_ep, seq);
}
