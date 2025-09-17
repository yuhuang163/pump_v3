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
#include "cmsis_os.h"
#include <stdio.h>
#include <stdarg.h>
#include "stdlib.h"
#include "string.h"
#include "cmsis.h"
#include "cmsis_nvic.h"
#include "hwtimer_list.h"
#include "analog.h"
#include "pmu.h"
#include "hal_trace.h"
#include "hal_dma.h"
#include "hal_timer.h"
#include "hal_iomux.h"
#include "hal_sysfreq.h"
#include "hal_sleep.h"
#include "hal_norflash.h"
#include "hal_cache.h"
#include "hal_key.h"
#include "mpu_cfg.h"
#include "hal_uart.h"
#include "hal_bootmode.h"
#include "norflash_api.h"
#include "app_flash_api.h"
#include "crc32_c.h"
#include "string.h"
#include "hal_evr.h"
#ifdef CORE_DUMP_TO_FLASH
#include "coredump_section.h"
#endif
#ifdef USER_DATA_TO_FLASH
#include "user_data_section.h"
#endif

#ifdef TEST_HAL_PATTERN
#include "test_hal_main.h"
#endif
#ifdef __WATCHER_DOG_RESET__
#include "watchdog/watchdog.h"
#endif

static osThreadId main_thread_tid = NULL;
static uint8_t test_read_buf[1024] = {0};

int system_reset(void)
{
    osThreadSetPriority(main_thread_tid, osPriorityRealtime);
    osSignalSet(main_thread_tid, 0x8);
    return 0;
}

int signal_send_to_main_thread(uint32_t signals)
{
    osSignalSet(main_thread_tid, signals);
    return 0;
}

int system_shutdown(void)
{
    TRACE(0,"system_shutdown!!");
    osThreadSetPriority(main_thread_tid, osPriorityRealtime);
    osSignalSet(main_thread_tid, 0x4);
    return 0;
}

static int key_event_process(uint32_t key_code, uint8_t key_event)
{
    static int iic_sw_flag = 0;
    TRACE(2,"%s: key_code:%04x key_event:%d", __func__, key_code, key_event);

    if (key_event == HAL_KEY_EVENT_CLICK) {
        if (iic_sw_flag) {
            iic_sw_flag = 0;
        } else {
            iic_sw_flag = 1;
        }
    } else if (key_event == HAL_KEY_EVENT_DOUBLECLICK) {
        TRACE(0, "powerkey double click! Enter ble mode");
        hal_sw_bootmode_set(HAL_SW_BOOTMODE_ENTER_HIDE_BOOT);
        pmu_reboot();
    }

    if (iic_sw_flag) {
        hal_iomux_set_analog_i2c();
    } else {
        hal_iomux_set_uart0();
    }
    return 0;
}

static void chip_init(void)
{
    hwtimer_init();
    hal_sys_timer_open();
    hal_sys_timer_set_opened(true);

    hal_audma_open();
    hal_gpdma_open();

    hal_key_open(0, key_event_process);

#ifdef DEBUG
#if (DEBUG_PORT == 1)
    hal_iomux_set_uart0();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART0);
#endif
#if (DEBUG_PORT == 2)
    //hal_iomux_set_analog_i2c();
    hal_iomux_set_uart1();
    hal_trace_open(HAL_TRACE_TRANSPORT_UART1);
#endif
#endif

    hal_norflash_show_id_state(HAL_FLASH_ID_0, true);

    hal_iomux_ispi_access_init();

#if defined (EVR_TRACING) && !defined (SYSTEM_USE_FILE)
    hal_evr_init(0);
#endif

    mpu_cfg();

    pmu_open();
    analog_open();

#if defined(CHIP_BEST1503) && defined(RC_CLK_ENABLE)
    hal_sysfreq_select_clock(HAL_SYSFREQ_SELECT_CLOCK_RC);
#endif
}

int main(void)
{
    chip_init();

#ifdef __WATCHER_DOG_RESET__
    app_wdt_open(20);
#endif

    TRACE(0,"platform_tester \n");
    // no need only for test
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_104M);
    TR_INFO(TR_MOD(MAIN), "CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));

#ifdef USE_NORFLASH_API
    enum NORFLASH_API_RET_T result;
    result = norflash_api_init();
    if (result != NORFLASH_API_OK) {
        TRACE(0, "norflash_api_init fail ret=%d", result);
    }

    DUMP_INFO_T dump_info = {0};

#ifdef USER_DATA_TO_FLASH
    user_data_to_flash_init();

    user_data_read(&dump_info, sizeof(dump_info), 0);
    user_data_erase_section();
    TRACE(0, "dump_flag=0x%08X len:0x%X crc:0x%08X", dump_info.dump_flag, dump_info.len, dump_info.crc);
    TRACE(0, "user data read done\n");
#endif

#ifdef CORE_DUMP_TO_FLASH
    uint32_t crc = 0;
    coredump_to_flash_init();

    if (dump_info.dump_flag == CRASH_MAGIC) {
        TRACE(0, "Crash, need to dump data");
        memset(test_read_buf, 0, sizeof(test_read_buf));
        for (uint32_t i=0; i<1408; i++) {
            core_dump_read_data(test_read_buf, 1024, i*1024);
            crc = crc32_c(crc, test_read_buf, 1024);
        }
        if (crc == dump_info.crc) {
            TRACE(0, "CRC32 check success");
        } else {
            TRACE(0, "CRC32 check fail, crc:0x%08X dump_info.crc=0x%08X", crc, dump_info.crc);
        }
    } else {
        TRACE(0, "No Crash, don't need to dump data");
    }
    TRACE(0, "coredump data check done\n");
#endif
#endif // end of USE_NORFLASH_API

#ifdef CP_SUBSYS_TEST
    extern void cp_subsys_test(void);
    cp_subsys_test();
#endif

#ifdef TEST_HAL_PATTERN
    test_hal_pattern();
#endif

    main_thread_tid = osThreadGetId();
    TRACE(1,"main_thread_tid=%x", (uint32_t)main_thread_tid);

    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
    hal_sleep_start_stats(10000, 10000);

    TR_INFO(TR_MOD(MAIN), "CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));

    while (1)
    {
        TRACE(0, "m33 alive");

        osSignalWait(0x0, 5000);
        // osSignalWait(0x0, osWaitForever);
    }

    return 0;
}

