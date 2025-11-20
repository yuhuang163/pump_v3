/***************************************************************************
 *
 * Copyright 2015-2022 BES.
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

#if defined(UTILS_ESHELL_EN) && defined(__SYS_AS_MAIN__)

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cmsis_os2.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hal_uart.h"
#include "pmu.h"
#include "hal_key.h"

#include "st_test.h"

struct st_test_config {
    /* test cases */
    uint8_t spi_enable;
    uint8_t gpu_enable;
    uint8_t blend_enable;
    uint8_t flash_enable;
    uint8_t sram_enable;
    uint8_t psram_enable;
    uint8_t emmc_enable;
    uint8_t efuse_enable;

#ifdef SENSOR_HUB_TEST
    uint8_t sensorhub_enable;
#endif
};

static struct st_test_config st_config;
static int timeout_secs = 60;

static void st_config_init_default(void)
{
    memset(&st_config, 0, sizeof(st_config));

    st_config.spi_enable = 1;
    st_config.gpu_enable = 1;
    st_config.blend_enable = 0;
    st_config.flash_enable = 0;
    st_config.sram_enable = 1;
    st_config.psram_enable = 1;
    st_config.emmc_enable = 1;
    st_config.efuse_enable = 1;

#ifdef SENSOR_HUB_TEST
    st_config.sensorhub_enable = 1;
#endif
}

static void st_config_init(void)
{
    st_config_init_default();
}

static void st_config_dump(void)
{
    eshell_putstring("SLT Config: \r\n");
    eshell_putstring("  Cases:\r\n");
    eshell_putstring("    SPI       [%s]\r\n", st_config.spi_enable ? "Enabled" : "Disabled");
    eshell_putstring("    GPU       [%s]\r\n", st_config.gpu_enable ? "Enabled" : "Disabled");
    eshell_putstring("    BLEND     [%s]\r\n", st_config.blend_enable ? "Enabled" : "Disabled");
    eshell_putstring("    FLASH     [%s]\r\n", st_config.flash_enable ? "Enabled" : "Disabled");
    eshell_putstring("    SRAM      [%s]\r\n", st_config.sram_enable ? "Enabled" : "Disabled");
    eshell_putstring("    PSRAM     [%s]\r\n", st_config.psram_enable ? "Enabled" : "Disabled");
    eshell_putstring("    EMMC      [%s]\r\n", st_config.emmc_enable ? "Enabled" : "Disabled");
    eshell_putstring("    EFUSE     [%s]\r\n", st_config.efuse_enable ? "Enabled" : "Disabled");
#ifdef SENSOR_HUB_TEST
    eshell_putstring("    SENSORHUB [%s]\r\n", st_config.sensorhub_enable ? "Enabled" : "Disabled");
#endif
}

extern void gpu_test_display_on(void);
extern void gpu_test_display_off(void);

static bool st_running = false;
static osThreadId_t st_slt_thread_id = NULL;

static void st_test_heavy(void)
{
    uint32_t test_time = rand() % 10 + 1;

    ST_LOG("Start heavy test mode..%ds", test_time);

    st_sysfreq_high();
    ST_LOG("CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 1)/1000/1000);

    if (st_config.gpu_enable)
        gpu_test_display_on();

    st_cases_loop_start();

    osDelay(test_time * 1000);

    st_cases_loop_stop();

    if (st_config.gpu_enable)
        gpu_test_display_off();

    st_sysfreq_mid();
}

static void st_test_sleep(void)
{
    uint32_t test_time = rand() % 3 + 1;
    ST_LOG("Start sleep test mode..%ds", test_time);

    st_sysfreq_sleep();
    ST_LOG("CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 1)/1000/1000);

    osDelay(test_time*1000);
}

static void st_slt_thread(void const *notused)
{
    int errs = 0;
    uint32_t start = hal_sys_timer_get();

    st_sysfreq_high();

    /* close eshell, slt test is one way trip */
    eshell_close();

    st_cases_clear();

    if (st_config.blend_enable)
        st_test_blend_register();
    if (st_config.flash_enable)
        st_test_flash_register();
    if (st_config.spi_enable)
        st_test_spi_register();
    if (st_config.sram_enable)
        st_test_sram_register();
    if (st_config.psram_enable)
        st_test_psram_register();
    if (st_config.emmc_enable)
        st_test_emmc_register();
    if (st_config.efuse_enable)
        st_test_efuse_register();
    if (st_config.gpu_enable)
        st_test_gpu_register();
#ifdef SENSOR_HUB_TEST
    if (st_config.sensorhub_enable)
        st_test_sens_register();
#endif

    st_sysfreq_high();
    ST_LOG("CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 1)/1000/1000);

    ST_LOG("==== Start function tests.");

    st_cases_func_test();
    errs = st_cases_check_errors();
    if (errs != 0)
        goto exit;

    ST_LOG("==== Start multi-threads tests.");

    st_cases_init();

    hal_sleep_start_stats(5000, 5000);
    osDelay(500);

    while (st_running) {
        st_test_heavy();
        st_test_sleep();

        if (TICKS_TO_MS(hal_sys_timer_get()-start) >= timeout_secs * 1000)
            break;
    }

    hal_sleep_start_stats(0, 0);
    st_sysfreq_high();

    errs = st_cases_check_errors();
    st_cases_exit();

exit:
    osDelay(500);
    st_cases_clear();

    ST_LOG("==== Total time %us\r\n", TICKS_TO_MS(hal_sys_timer_get()-start)/1000);
    if (errs != 0)
        ST_LOG("SLT TEST FAILED!\r\n");
    else
        ST_LOG("SLT TEST SUCCESS!\r\n");

    /* wait here */
    hal_trace_output_enable(false);
    st_sysfreq_sleep();
    while (1)
        osDelay(1000);
}

static void st_slt_start(void)
{
    osThreadAttr_t attr;

    if (st_slt_thread_id != NULL)
        return;

    memset(&attr, 0, sizeof(osThreadAttr_t));
    attr.name = "slt_test_thread";
    attr.attr_bits = osThreadDetached;
    attr.stack_size = 4096;
    attr.priority = osPriorityHigh;

    st_running = true;
    st_slt_thread_id = osThreadNew((osThreadFunc_t)st_slt_thread,
                                NULL, &attr);
}

static void st_slt_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  st_slt config show\r\n");
    eshell_putstring("  st_slt config clean\r\n");
    eshell_putstring("  st_slt config enable spi|gpu|emmc|sram|psram|efuse|blend\r\n");
    eshell_putstring("  st_slt config disable spi|gpu|emmc|sram|psram|efuse|blend\r\n");
    eshell_putstring("  st_slt start [TIMEOUT_SECONDS]\r\n");
}

static void st_slt_cmd(int argc, char *argv[])
{
    static bool config_inited = false;

    if (argc < 2)
        goto usage;

    if (!config_inited) {
        st_config_init();
        config_inited = true;
    }

    if (strncmp(argv[1], "start", 5) == 0) {

        if (argc > 2)
            timeout_secs = atoi(argv[2]);

        st_slt_start();

    } else if (strncmp(argv[1], "config", 6) == 0) {
        if (argc < 3)
            goto usage;

        if (strncmp(argv[2], "show", 4) == 0) {
            st_config_dump();

        } else if (strncmp(argv[2], "clean", 5) == 0) {
            st_config_init_default();

        } else if (strncmp(argv[2], "enable", 6) == 0) {
            if (argc < 4)
                goto usage;

            if (strncmp(argv[3], "spi", 3) == 0)
                st_config.spi_enable = 1;
            else if (strncmp(argv[3], "gpu", 3) == 0)
                st_config.gpu_enable = 1;
            else if (strncmp(argv[3], "blend", 3) == 0)
                st_config.blend_enable = 1;
            else if (strncmp(argv[3], "flash", 3) == 0)
                st_config.flash_enable = 1;
            else if (strncmp(argv[3], "emmc", 4) == 0)
                st_config.emmc_enable = 1;
            else if (strncmp(argv[3], "sram", 4) == 0)
                st_config.sram_enable = 1;
            else if (strncmp(argv[3], "psram", 5) == 0)
                st_config.psram_enable = 1;
            else if (strncmp(argv[3], "efuse", 5) == 0)
                st_config.efuse_enable = 1;
            else
                goto usage;

        } else if (strncmp(argv[2], "disable", 7) == 0) {
            if (argc < 4)
                goto usage;

            if (strncmp(argv[3], "spi", 3) == 0)
                st_config.spi_enable = 0;
            else if (strncmp(argv[3], "gpu", 3) == 0)
                st_config.gpu_enable = 0;
            else if (strncmp(argv[3], "blend", 3) == 0)
                st_config.blend_enable = 0;
            else if (strncmp(argv[3], "flash", 3) == 0)
                st_config.flash_enable = 0;
            else if (strncmp(argv[3], "emmc", 4) == 0)
                st_config.emmc_enable = 0;
            else if (strncmp(argv[3], "sram", 4) == 0)
                st_config.sram_enable = 0;
            else if (strncmp(argv[3], "psram", 5) == 0)
                st_config.psram_enable = 0;
            else if (strncmp(argv[3], "efuse", 5) == 0)
                st_config.efuse_enable = 0;
            else
                goto usage;

        } else {
            goto usage;
        }

    } else {
        goto usage;
    }

    return;
usage:
    st_slt_usage();
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST,
                "st_slt",
                "usage: st_slt help",
                st_slt_cmd);
#endif
