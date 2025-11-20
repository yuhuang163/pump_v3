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
    uint32_t i2c_enable;
    uint32_t spi_enable;
    uint32_t gpu_enable;
    uint32_t sram_enable;
    uint32_t psram_enable;
    uint32_t emmc_enable;
};

static struct st_test_config st_config;

osSemaphoreId_t st_monkey_sem = NULL;

static void st_config_dump(void)
{
    eshell_putstring("Stress Test Config: \r\n");
    eshell_putstring("  Cases:\r\n");
    eshell_putstring("    I2C     [%s]\r\n", st_config.i2c_enable ? "Enabled" : "Disabled");
    eshell_putstring("    SPI     [%s]\r\n", st_config.spi_enable ? "Enabled" : "Disabled");
    eshell_putstring("    GPU     [%s]\r\n", st_config.gpu_enable ? "Enabled" : "Disabled");
    eshell_putstring("    SRAM    [%s]\r\n", st_config.sram_enable ? "Enabled" : "Disabled");
    eshell_putstring("    PSRAM   [%s]\r\n", st_config.psram_enable ? "Enabled" : "Disabled");
    eshell_putstring("    EMMC    [%s]\r\n", st_config.emmc_enable ? "Enabled" : "Disabled");
}

static void st_config_init_default(void)
{
    memset(&st_config, 0, sizeof(st_config));

    st_config.i2c_enable = 1;
    st_config.spi_enable = 1;
    st_config.gpu_enable = 1;
    st_config.sram_enable = 1;
    st_config.psram_enable = 1;
    st_config.emmc_enable = 1;
}

static void st_config_init(void)
{
    st_config_init_default();
}

extern void gpu_test_display_on(void);
extern void gpu_test_display_off(void);

static bool st_running = false;
static osThreadId_t st_monkey_thread_id = NULL;

static void st_test_high_perf(void)
{
    uint32_t test_time = rand() % 10 + 1;

    ST_LOG("Start high performace test mode..%ds", test_time);

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

static void st_test_low_perf(void)
{
    uint32_t test_time = rand() % 10 + 1;

    ST_LOG("Start low performance test mode..%ds", test_time);

    st_sysfreq_mid();
    ST_LOG("CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 1)/1000/1000);

    st_cases_loop_start();

    osDelay(test_time * 1000);

    st_cases_loop_stop();
}

static void st_test_low_power(void)
{
    ST_LOG("Start low power test mode..");
    st_sysfreq_low();
    ST_LOG("CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 1)/1000/1000);
    for (int i = 3; i > 0; i--) {
        ST_LOG("%d...", i);
        osDelay(1000);
    }
}

static void st_test_sleep_mode(void)
{
    uint32_t test_time = rand() % 10 + 1;
    ST_LOG("Start sleep test mode..%ds", test_time);

    st_sysfreq_sleep();
    ST_LOG("CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 1)/1000/1000);

    if (st_monkey_sem) {
        osDelay(100);
        if (osSemaphoreGetCount(st_monkey_sem) == 1)
            osSemaphoreAcquire(st_monkey_sem, osWaitForever);
        osSemaphoreAcquire(st_monkey_sem, test_time*1000);
    } else {
        osDelay(test_time*1000);
    }
}

static void st_monkey_thread(void const *notused)
{
    st_sysfreq_high();

    if (!st_monkey_sem)
        st_monkey_sem = osSemaphoreNew(1, 0, NULL);

    /* close eshell for sleep mode test.
     * this is an one way trip..
     */
    eshell_close();

    st_cases_init();

    hal_sleep_start_stats(5000, 5000);

    osDelay(1000);

    while (st_running) {
        st_test_high_perf();
        st_test_sleep_mode();
        st_test_low_perf();
        st_test_sleep_mode();
        st_test_low_power();
    }

    hal_sleep_start_stats(0, 0);
    st_sysfreq_sleep();
}

static void st_monkey_start(void)
{
    osThreadAttr_t attr;

    if (st_monkey_thread_id != NULL)
        return;

    memset(&attr, 0, sizeof(osThreadAttr_t));
    attr.name = "monkey_test_thread";
    attr.attr_bits = osThreadDetached;
    attr.stack_size = 4096;
    attr.priority = osPriorityHigh;

    st_running = true;
    st_monkey_thread_id = osThreadNew((osThreadFunc_t)st_monkey_thread,
                                NULL, &attr);
}

static void st_monkey_exit(void)
{
    if (st_monkey_thread_id != NULL) {
        st_running = false;
        st_cases_check_errors();
        st_cases_exit();
        osDelay(3000);
        osThreadTerminate(st_monkey_thread_id);
        st_monkey_thread_id = NULL;
    }
}

static void st_monkey_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  st_monkey config show\r\n");
    eshell_putstring("  st_monkey config clean\r\n");
    eshell_putstring("  st_monkey config enable spi|gpu|emmc|sram|psram\r\n");
    eshell_putstring("  st_monkey config disable spi|gpu|emmc|sram|psram\r\n");
    eshell_putstring("  st_monkey start\r\n");
    eshell_putstring("  st_monkey stop\r\n");
}

static void st_monkey_cmd(int argc, char *argv[])
{
    static bool config_inited = false;

    if (argc < 2)
        goto usage;

    if (!config_inited) {
        st_config_init();
        config_inited = true;
    }

    if (strncmp(argv[1], "start", 5) == 0) {
        st_cases_clear();

        if (st_config.gpu_enable)
            st_test_gpu_register();
        if (st_config.i2c_enable)
            st_test_i2c_register();
        if (st_config.spi_enable)
            st_test_spi_register();
        if (st_config.sram_enable)
            st_test_sram_register();
        if (st_config.psram_enable)
            st_test_psram_register();
        if (st_config.emmc_enable)
            st_test_emmc_register();

        st_monkey_start();

    } else if (strncmp(argv[1], "stop", 4) == 0) {
        st_monkey_exit();

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

            if (strncmp(argv[3], "i2c", 3) == 0)
                st_config.i2c_enable = 1;
            else if (strncmp(argv[3], "spi", 3) == 0)
                st_config.spi_enable = 1;
            else if (strncmp(argv[3], "gpu", 3) == 0)
                st_config.gpu_enable = 1;
            else if (strncmp(argv[3], "emmc", 4) == 0)
                st_config.emmc_enable = 1;
            else if (strncmp(argv[3], "sram", 4) == 0)
                st_config.sram_enable = 1;
            else if (strncmp(argv[3], "psram", 5) == 0)
                st_config.psram_enable = 1;
            else
                goto usage;

        } else if (strncmp(argv[2], "disable", 7) == 0) {
            if (argc < 4)
                goto usage;

            if (strncmp(argv[3], "i2c", 3) == 0)
                st_config.i2c_enable = 0;
            else if (strncmp(argv[3], "spi", 3) == 0)
                st_config.spi_enable = 0;
            else if (strncmp(argv[3], "gpu", 3) == 0)
                st_config.gpu_enable = 0;
            else if (strncmp(argv[3], "emmc", 4) == 0)
                st_config.emmc_enable = 0;
            else if (strncmp(argv[3], "sram", 4) == 0)
                st_config.sram_enable = 0;
            else if (strncmp(argv[3], "psram", 5) == 0)
                st_config.psram_enable = 0;
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
    st_monkey_usage();
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST,
                "st_monkey",
                "usage: st_monkey help",
                st_monkey_cmd);
#endif
