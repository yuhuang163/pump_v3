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
#ifdef CP_IN_SAME_EE

#include "hwtest.h"
#include "cmsis.h"
#include "hal_cache.h"
#include "hal_cmu.h"
#include "hal_location.h"
#include "hal_mcu2cp.h"
#include "plat_addr_map.h"
#include "mpu_cfg.h"
#include "system_cp.h"

// CP_TEST

//#define CP_SLEEP_TEST

#define CP_FLASH_ACCESS_START               (FLASH_BASE + 0x10000)
#define MCU_FLASH_ACCESS_START              (FLASH_BASE)

static uint32_t CP_BSS_LOC cp_data;
static volatile uint32_t CP_BSS_LOC cp_rx_cnt;
static uint32_t mcu_data;
static volatile uint32_t mcu_rx_cnt;

static unsigned int CP_TEXT_SRAM_LOC cp_rx_irq(const void *data, unsigned int len)
{
    cp_data = *(uint32_t *)data;

    TRACE(0, TRC_STR2(CP_DATA_LOC, "cp_rx_irq: len=%u 0x%08X"), len, cp_data);

    hal_mcu2cp_rx_done_cp(HAL_RMT_IPC_CHAN_0);

#ifdef SIMU
    hal_sys_timer_delay_us(100);
#else
    hal_sys_timer_delay(MS_TO_TICKS(1000));
#endif

    cp_data += 0x1;
    hal_mcu2cp_send_cp(HAL_RMT_IPC_CHAN_0, (unsigned char *)&cp_data, sizeof(cp_data));

    uint32_t size;

    size = (cp_rx_cnt & 0x3);
    if (size == 0) {
        size = 12;
    } else if (size == 1) {
        size = 56;
    } else if (size == 2) {
        size = 360;
    } else {
        size = 0x4000;
    }
#ifdef ICACHECP_CTRL_BASE
    hal_cachecp_invalidate(HAL_CACHE_ID_I_CACHE, CP_FLASH_ACCESS_START, size);
    hal_cachecp_invalidate(HAL_CACHE_ID_D_CACHE, CP_FLASH_ACCESS_START, size);
#endif

    cp_rx_cnt++;

    return len;
}

static void CP_TEXT_SRAM_LOC cp_tx_irq(const void *data, unsigned int len)
{
    uint32_t val;

    val = *(uint32_t *)data;

    TRACE(0, TRC_STR2(CP_DATA_LOC, "cp_tx_irq: len=%u 0x%08X"), len, val);
}

static void CP_TEXT_SRAM_LOC NOINLINE cp_main_loop(void)
{
#ifdef CP_MPU_CFG
#ifndef CP_FLASH_ACCESS
#error "CP_FLASH_ACCESS should be set if CP_MPU_CFG=1"
#endif
    mpu_cfg_cp();
#endif

    hal_mcu2cp_open_cp(HAL_RMT_IPC_CHAN_0, cp_rx_irq, cp_tx_irq, true);

    hal_mcu2cp_start_recv_cp(HAL_RMT_IPC_CHAN_0);

    while (1) {
#ifdef CP_SLEEP_TEST
        __WFI();
#else
        if (cp_rx_cnt) {
            for (uint32_t i = 0; i < 0x10000; i += 4) {
                *(volatile uint32_t *)(FLASH_BASE + 0x10000 + i);
            }
        } else {
            __WFI();
        }
#endif
    }
}

static void cp_start(void)
{
    system_cp_init(true);

    cp_main_loop();
}

static unsigned int mcu_rx_irq(const void *data, unsigned int len)
{
    mcu_data = *(uint32_t *)data;

    TRACE(0, "%s: len=%u 0x%08X", __FUNCTION__, len, mcu_data);

    hal_mcu2cp_rx_done_mcu(HAL_RMT_IPC_CHAN_0);

#ifdef SIMU
    hal_sys_timer_delay_us(200);
#else
    hal_sys_timer_delay(MS_TO_TICKS(2000));
#endif

    mcu_data += 0x10000;
    hal_mcu2cp_send_mcu(HAL_RMT_IPC_CHAN_0, (unsigned char *)&mcu_data, sizeof(mcu_data));

    uint32_t size;

    size = (mcu_rx_cnt & 0x3);
    if (size == 0) {
        size = 12;
    } else if (size == 1) {
        size = 56;
    } else if (size == 2) {
        size = 360;
    } else {
        size = 0x4000;
    }
    hal_cache_invalidate(HAL_CACHE_ID_I_CACHE, CP_FLASH_ACCESS_START, size);
    hal_cache_invalidate(HAL_CACHE_ID_D_CACHE, CP_FLASH_ACCESS_START, size);

    mcu_rx_cnt++;

    return len;
}

static void mcu_tx_irq(const void *data, unsigned int len)
{
    uint32_t val;

    val = *(uint32_t *)data;

    TRACE(0, "%s: len=%u 0x%08X", __FUNCTION__, len, val);
}

void cp_test(void)
{
    uint32_t val;
    uint32_t sp;
    uint32_t entry;

    sp = RAMCP_BASE + RAMCP_SIZE;
    entry = (uint32_t)cp_start;

    TRACE(0, "%s: sp=0x%08X entry=0x%08X", __func__, sp, entry);

    hal_cmu_cp_enable(sp, entry);

    hal_mcu2cp_open_mcu(HAL_RMT_IPC_CHAN_0, mcu_rx_irq, mcu_tx_irq, true);

    hal_mcu2cp_start_recv_mcu(HAL_RMT_IPC_CHAN_0);

    val = 0x3333CCCC;
    hal_mcu2cp_send_mcu(HAL_RMT_IPC_CHAN_0, (unsigned char *)&val, sizeof(val));

    while (1) {
        if (mcu_rx_cnt) {
            for (uint32_t i = 0; i < 0x10000; i += 4) {
                *(volatile uint32_t *)(FLASH_BASE + i);
            }
        } else {
            __WFI();
        }
    }
}

#endif

