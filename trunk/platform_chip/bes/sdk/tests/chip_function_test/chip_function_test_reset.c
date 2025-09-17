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
#ifdef CHIP_FUNCTION_TEST_POWER_RESET
#include "cmsis.h"
#include "hal_timer.h"
#include "hal_trace.h"
 #include "string.h"
#include "hal_location.h"
#include "hal_norflash.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "pmu.h"

#define FUNCTION_TEST_RESET_COUNT  16
#define FUNCTION_TEST_MIGIC 0xAA554433
#define FUNCTION_TEST_VERSION  0x00000001
#ifdef FLASH_DUAL_CHIP
#define FUNCTION_TEST_SECTOR_SIZE 0x2000
#else
#define FUNCTION_TEST_SECTOR_SIZE 0x1000
#endif
#define ALIGN_N(a, n) (((a + n-1)/n)*n)
extern uint32_t __flash_end[];
typedef struct
{
    uint32_t magic;
    uint32_t version;
    uint32_t reset_count;

} FUNCITON_NV;

static uint32_t _get_reset_count(void)
{
    uint32_t base_addr;
    uint32_t start_addr;
    FUNCITON_NV function_nv;
    uint32_t *p;

    TRACE(1, "%s", __func__);
    base_addr = FLASH_NC_BASE;
    start_addr = base_addr | ((uint32_t)__flash_end & HAL_NORFLASH_ADDR_MASK);
    start_addr = (uint32_t)ALIGN_N(start_addr, FUNCTION_TEST_SECTOR_SIZE);
    p = (uint32_t*)start_addr;
    memcpy(&function_nv, p, sizeof(function_nv));
    if(function_nv.magic != FUNCTION_TEST_MIGIC)
    {
        function_nv.reset_count = 0;
    }

    return function_nv.reset_count;
}

static void _set_reset_count(uint32_t reset_count)
{
    uint32_t base_addr;
    uint32_t start_addr;
    FUNCITON_NV function_nv;

    //TRACE(1, "%s reset_count = %d", __func__, reset_count);
    base_addr = FLASH_NC_BASE;
    start_addr = base_addr | ((uint32_t)__flash_end & HAL_NORFLASH_ADDR_MASK);
    start_addr = (uint32_t)ALIGN_N(start_addr, FUNCTION_TEST_SECTOR_SIZE);

    function_nv.magic = FUNCTION_TEST_MIGIC;
    function_nv.version = FUNCTION_TEST_VERSION;
    function_nv.reset_count  = reset_count;

    hal_norflash_erase(HAL_FLASH_ID_0, start_addr, FUNCTION_TEST_SECTOR_SIZE);
    hal_norflash_write(HAL_FLASH_ID_0, start_addr, (uint8_t*)&function_nv, sizeof(function_nv));
    //TRACE(1, "%s reset_count = %d done", __func__, reset_count);
}

void _set_freq(uint32_t reset_count)
{
   enum HAL_CMU_FREQ_T freq;

   freq = reset_count % HAL_CMU_FREQ_QTY;
   hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, freq);
   TRACE(1, "%s: freq = %d", __func__, freq);
}

void function_test_power_reset(void)
{
    uint32_t reset_count;

    TRACE(1, "%s: begin", __func__);
    reset_count= _get_reset_count();
    if(reset_count >= FUNCTION_TEST_RESET_COUNT)
    {
        TRACE(1, "FUNCTION TEST POWER/RESET PASS");
        osDelay(10);
        _set_reset_count(0);
        pmu_shutdown();
    }
    else
    {
        _set_reset_count(reset_count + 1);
        _set_freq(reset_count);
        TR_INFO(TR_MOD(MAIN), "CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));
        TRACE(1, "FUNCTION TEST POWER/RESET RESET COUTNS: %d", reset_count);
        osDelay(10);
        pmu_reboot();
    }
}
#endif
