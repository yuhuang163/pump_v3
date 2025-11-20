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

#include "st_test.h"

#ifdef CHIP_HAS_EMMC
#define ST_CONFIG_ON_EMMC
#endif

typedef enum {
    ST_TEST_NULL = 0,
    ST_TEST_LIGHT_WORKING,     /* light test mode, run test cases once per-second. */
    ST_TEST_LIGHT_NODISP,      /* light test mode, without display working. */
    ST_TEST_HEAVY_WORKING,     /* heavy test mode, loop to run test cases. */
    ST_TEST_DEEP_SLEEP,        /* deep sleep test mode, enter deep sleep, then wakeup. */
    ST_TEST_PSRAM_FULLSCAN,    /* PSRAM full scan test mode. */
    ST_TEST_REBOOT,            /* reboot test mode. */

    ST_TEST_STAGES_MAX,
} ST_TEST_STAGE;

#define ST_CONFIG_MAGIC             0x5A5A6785
#define DEFAULT_REBOOT_INTERVAL     (15 * 60) /* 15min */
#define DEFAULT_REBOOT_TIMES        20

struct st_test_config {
    uint32_t magic;

    /* test cases */
    uint32_t i2c_enable;
    uint32_t spi_enable;
    uint32_t gpu_enable;
    uint32_t sram_enable;
    uint32_t psram_enable;
    uint32_t emmc_enable;
    uint32_t efuse_enable;

    /* test stage settings */
    uint32_t light_mode_time;  /* light test mode, testing seconds, 0 for disabled */
    uint32_t nodisp_mode_time; /* light test without display, testing seconds, 0 for disabled */
    uint32_t heavy_mode_time;  /* heavy test mode, testing seconds, 0 for disabled */
    uint32_t sleep_mode_time;  /* sleep test mode, testing seconds, 0 for disabled */
    uint32_t psram_mode_time;  /* PSRAM full test mode, testing seconds, 0 for disabled */

    uint32_t reboot_enable;    /* enable reboot test, 1 for enabled, 0 for disabled */
    uint32_t reboot_interval;  /* reboot interval time in seconds */
    uint32_t reboot_times;     /* reboot test times */
    uint32_t reboot_counter;   /* reboot test counter, if reach 'reboot_times', stop reboot */
    uint32_t reboot_testing;   /* in reboot testing stage or not */
};

static char st_config_data[512] ALIGNED(32);
static struct st_test_config *st_config = (struct st_test_config *)&st_config_data[0];

static void st_config_dump(void)
{
    eshell_putstring("Stress Test Config: \r\n");
    eshell_putstring("  Cases:\r\n");
    eshell_putstring("    I2C    [%s]\r\n", st_config->i2c_enable ? "Enabled" : "Disabled");
    eshell_putstring("    SPI    [%s]\r\n", st_config->spi_enable ? "Enabled" : "Disabled");
    eshell_putstring("    GPU    [%s]\r\n", st_config->gpu_enable ? "Enabled" : "Disabled");
    eshell_putstring("    SRAM   [%s]\r\n", st_config->sram_enable ? "Enabled" : "Disabled");
    eshell_putstring("    PSRAM  [%s]\r\n", st_config->psram_enable ? "Enabled" : "Disabled");
    eshell_putstring("    EMMC   [%s]\r\n", st_config->emmc_enable ? "Enabled" : "Disabled");
    eshell_putstring("    EFUSE  [%s]\r\n", st_config->efuse_enable ? "Enabled" : "Disabled");
    eshell_putstring("  Settings:\r\n");
    eshell_putstring("    light mode:       %u seconds\r\n", st_config->light_mode_time);
    eshell_putstring("    nodisp mode:      %u seconds\r\n", st_config->nodisp_mode_time);
    eshell_putstring("    heavy mode:       %u seconds\r\n", st_config->heavy_mode_time);
    eshell_putstring("    sleep mode:       %u seconds\r\n", st_config->sleep_mode_time);
    eshell_putstring("    psram fullscan:   %u seconds\r\n", st_config->psram_mode_time);
    eshell_putstring("    reboot enable:    %s\r\n", st_config->reboot_enable ? "Enabled" : "Disabled");
    eshell_putstring("    reboot testing:   %s\r\n", st_config->reboot_testing ? "Yes" : "No");
    eshell_putstring("    reboot interval:  %u seconds\r\n", st_config->reboot_interval);
    eshell_putstring("    reboot times:     %u\r\n", st_config->reboot_times);
    eshell_putstring("    reboot counter:   %u\r\n", st_config->reboot_counter);
}

static void st_config_init_default(void)
{
    memset(st_config_data, 0, sizeof(st_config_data));

    st_config->magic = ST_CONFIG_MAGIC;
    st_config->i2c_enable = 1;
    st_config->spi_enable = 1;
    st_config->gpu_enable = 1;
    st_config->sram_enable = 1;
    st_config->psram_enable = 1;
    st_config->emmc_enable = 1;
    st_config->efuse_enable = 0;

    st_config->light_mode_time = 10;
    st_config->nodisp_mode_time = 5;
    st_config->sleep_mode_time = 10;
    st_config->heavy_mode_time = 10;
    st_config->psram_mode_time = 10;

    st_config->reboot_enable = 0;
    st_config->reboot_interval = DEFAULT_REBOOT_INTERVAL;
    st_config->reboot_times = DEFAULT_REBOOT_TIMES;
    st_config->reboot_counter = 0;
    st_config->reboot_testing = 0;
}

#ifdef ST_CONFIG_ON_EMMC

extern bool emmc_is_inited(void);
extern int emmc_init_default();
extern int emmc_write(uint32_t offset, uint32_t size, uint32_t addr);
extern int emmc_read(uint32_t offset, uint32_t size, uint32_t addr);

static void st_config_init(void)
{
    /* init emmc for config loading and emmc test */
    if (!emmc_is_inited())
        emmc_init_default();

    memset(st_config_data, 0, sizeof(st_config_data));

    /* load config from emmc 0x0 */
    emmc_read(0, sizeof(st_config_data), (uint32_t)st_config_data);

    if (st_config->magic != ST_CONFIG_MAGIC)
        st_config_init_default();
}

static void st_config_save(void)
{
    st_config->magic = ST_CONFIG_MAGIC;
    emmc_write(0, sizeof(st_config_data), (uint32_t)st_config_data);
}

#else

static void st_config_init(void)
{
    st_config_init_default();
}

static void st_config_save(void)
{
}

#endif

extern void gpu_test_display_on(void);
extern void gpu_test_display_off(void);

static bool st_running = false;
static osThreadId_t st_main_thread_id = NULL;

static void st_test_light(void)
{
    uint32_t start, test_ms;

    if (st_config->light_mode_time == 0)
        return;

    ST_LOG("Start light test mode..");

    if (st_config->gpu_enable)
        gpu_test_display_on();

    for (int i = 0; i < st_config->light_mode_time && st_running; i++) {
        st_sysfreq_mid();
        start = hal_sys_timer_get();

        st_cases_run_once();

        test_ms = TICKS_TO_MS(hal_sys_timer_get() - start);
        st_sysfreq_sleep();

        if (test_ms < 1000)
            osDelay(1000-test_ms);
    }

    if (st_config->gpu_enable)
        gpu_test_display_off();
}

static void st_test_nodisp(void)
{
    uint32_t start, test_ms;

    if (st_config->nodisp_mode_time == 0)
        return;

    ST_LOG("Start no-display test mode..");

    for (int i = 0; i < st_config->nodisp_mode_time && st_running; i++) {
        st_sysfreq_mid();
        start = hal_sys_timer_get();

        st_cases_run_once();

        test_ms = TICKS_TO_MS(hal_sys_timer_get() - start);
        st_sysfreq_sleep();

        if (test_ms < 1000)
            osDelay(1000-test_ms);
    }
}

static void st_test_heavy(void)
{
    if (st_config->heavy_mode_time == 0)
        return;

    ST_LOG("Start heavy test mode..");

    st_sysfreq_high();
    ST_LOG("CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 1)/1000/1000);

    if (st_config->gpu_enable)
        gpu_test_display_on();

    st_cases_loop_start();

    for (int i = 0; i < st_config->heavy_mode_time && st_running; i++)
        osDelay(1000);

    st_cases_loop_stop();

    if (st_config->gpu_enable)
        gpu_test_display_off();

    st_sysfreq_sleep();
}

static void st_test_sleep(void)
{
    if (st_config->sleep_mode_time == 0)
        return;

    ST_LOG("Start sleep test mode..");

    st_sysfreq_sleep();
    ST_LOG("CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 1)/1000/1000);

    /* close eshell for deep sleep */
    eshell_close();

    hal_sleep_enter_sleep();

    for (int i = 0; i < st_config->sleep_mode_time && st_running; i++)
        osDelay(1000);

    /* re-open eshell */
    eshell_open(HAL_UART_ID_1, true);

    osDelay(1000);
}

extern int psram_full_scan_test(int timeout_ms);
static void st_test_psram_fullscan(void)
{
    if (st_config->psram_mode_time == 0)
        return;

    st_sysfreq_high();

    psram_full_scan_test(st_config->psram_mode_time * 1000);

    st_sysfreq_sleep();
}

static void st_main_thread(void const *notused)
{
    uint32_t test_start, test_end, test_ms;
    ST_TEST_STAGE stage = ST_TEST_LIGHT_WORKING;

    st_sysfreq_high();

    test_start = hal_sys_timer_get();

    st_cases_init();

    if (st_config->reboot_enable && st_config->reboot_testing)
        stage = ST_TEST_REBOOT;
    else
        stage = ST_TEST_LIGHT_WORKING;

    hal_sleep_start_stats(10000, 10000);
    st_sysfreq_sleep();

    osDelay(1000);

    while (st_running) {
        switch (stage) {
            case ST_TEST_REBOOT:
                if (st_config->reboot_enable && st_config->reboot_testing) {
                    st_config->reboot_counter++;
                    if (st_config->reboot_counter >= st_config->reboot_times) {
                        st_config->reboot_testing = 0;
                        st_config->reboot_counter = 0;
                    }
                    st_config_save();
                    osDelay(1000);
                    pmu_reboot();
                    while(1);
                }
                break;

            case ST_TEST_LIGHT_WORKING:
                st_test_light();
                stage = ST_TEST_LIGHT_NODISP;
                break;

            case ST_TEST_LIGHT_NODISP:
                st_test_nodisp();
                stage = ST_TEST_HEAVY_WORKING;
                break;

            case ST_TEST_HEAVY_WORKING:
                st_test_heavy();
                stage = ST_TEST_DEEP_SLEEP;
                break;

            case ST_TEST_DEEP_SLEEP:
                st_test_sleep();
                stage = ST_TEST_PSRAM_FULLSCAN;
                break;

            case ST_TEST_PSRAM_FULLSCAN:
                st_test_psram_fullscan();
                stage = ST_TEST_LIGHT_WORKING;
                /* check if need to enter reboot testing stage */
                if (st_config->reboot_enable) {
                    test_end = hal_sys_timer_get();
                    test_ms = TICKS_TO_MS(test_end - test_start);
                    if (test_ms / 1000 >= st_config->reboot_interval) {
                        st_config->reboot_testing = 1;
                        stage = ST_TEST_REBOOT;
                    }
                }
                break;

            default:
                break;
        }
        osDelay(1000);
    }

    hal_sleep_start_stats(0, 0);
    st_sysfreq_sleep();
}

static void st_main_start(void)
{
    if (st_main_thread_id != NULL)
        return;

    osThreadAttr_t threadattr_eshell_st_main;

    memset(&threadattr_eshell_st_main, 0, sizeof(osThreadAttr_t));
    threadattr_eshell_st_main.name = "main_test_thread";
    threadattr_eshell_st_main.attr_bits = osThreadDetached;
    threadattr_eshell_st_main.stack_size = 2048;
    threadattr_eshell_st_main.priority = osPriorityHigh;

    st_running = true;
    st_main_thread_id = osThreadNew((osThreadFunc_t)st_main_thread,
                                NULL, &threadattr_eshell_st_main);
}

static void st_main_exit(void)
{
    if (st_main_thread_id != NULL) {
        st_running = false;
        st_cases_check_errors();
        st_cases_exit();
        osDelay(3000);
        osThreadTerminate(st_main_thread_id);
        st_main_thread_id = NULL;
    }
}

static void st_main_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  st_main config show\r\n");
    eshell_putstring("  st_main config clean\r\n");
    eshell_putstring("  st_main config enable spi|gpu|emmc|sram|psram|reboot|efuse\r\n");
    eshell_putstring("  st_main config disable spi|gpu|emmc|sram|psram|reboot|efuse\r\n");
    eshell_putstring("  st_main config light SECONDS\r\n");
    eshell_putstring("  st_main config nodisp SECONDS\r\n");
    eshell_putstring("  st_main config heavy SECONDS\r\n");
    eshell_putstring("  st_main config sleep SECONDS\r\n");
    eshell_putstring("  st_main config psram SECONDS\r\n");
    eshell_putstring("  st_main config reboot SECONDS TIMES\r\n");
    eshell_putstring("  st_main start\r\n");
    eshell_putstring("  st_main stop\r\n");
}

static void st_main_cmd(int argc, char *argv[])
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

        if (st_config->gpu_enable)
            st_test_gpu_register();
        if (st_config->i2c_enable)
            st_test_i2c_register();
        if (st_config->spi_enable)
            st_test_spi_register();
        if (st_config->sram_enable)
            st_test_sram_register();
        if (st_config->psram_enable)
            st_test_psram_register();
        if (st_config->emmc_enable)
            st_test_emmc_register();
        if (st_config->efuse_enable)
            st_test_efuse_register();

        st_main_start();

    } else if (strncmp(argv[1], "stop", 4) == 0) {
        st_main_exit();

    } else if (strncmp(argv[1], "config", 6) == 0) {
        bool new_config = true;
        uint32_t t, v;

        if (argc < 3)
            goto usage;

        if (strncmp(argv[2], "show", 4) == 0) {
            st_config_dump();
            new_config = false;

        } else if (strncmp(argv[2], "clean", 5) == 0) {
            st_config_init_default();

        } else if (strncmp(argv[2], "enable", 6) == 0) {
            if (argc < 4)
                goto usage;

            if (strncmp(argv[3], "i2c", 3) == 0)
                st_config->i2c_enable = 1;
            else if (strncmp(argv[3], "spi", 3) == 0)
                st_config->spi_enable = 1;
            else if (strncmp(argv[3], "gpu", 3) == 0)
                st_config->gpu_enable = 1;
            else if (strncmp(argv[3], "emmc", 4) == 0)
                st_config->emmc_enable = 1;
            else if (strncmp(argv[3], "sram", 4) == 0)
                st_config->sram_enable = 1;
            else if (strncmp(argv[3], "psram", 5) == 0)
                st_config->psram_enable = 1;
            else if (strncmp(argv[3], "reboot", 6) == 0)
                st_config->reboot_enable = 1;
            else if (strncmp(argv[3], "efuse", 5) == 0)
                st_config->efuse_enable = 1;
            else
                goto usage;

        } else if (strncmp(argv[2], "disable", 7) == 0) {
            if (argc < 4)
                goto usage;

            if (strncmp(argv[3], "i2c", 3) == 0)
                st_config->i2c_enable = 0;
            else if (strncmp(argv[3], "spi", 3) == 0)
                st_config->spi_enable = 0;
            else if (strncmp(argv[3], "gpu", 3) == 0)
                st_config->gpu_enable = 0;
            else if (strncmp(argv[3], "emmc", 4) == 0)
                st_config->emmc_enable = 0;
            else if (strncmp(argv[3], "sram", 4) == 0)
                st_config->sram_enable = 0;
            else if (strncmp(argv[3], "psram", 5) == 0)
                st_config->psram_enable = 0;
            else if (strncmp(argv[3], "reboot", 6) == 0)
                st_config->reboot_enable = 0;
            else if (strncmp(argv[3], "efuse", 5) == 0)
                st_config->efuse_enable = 0;
            else
                goto usage;

        } else if (strncmp(argv[2], "light", 5) == 0) {
            if (argc < 4)
                goto usage;

            t = atoi(argv[3]);
            if (t > 900000)
                t = 900000;

            st_config->light_mode_time = t;

        } else if (strncmp(argv[2], "nodisp", 6) == 0) {
            if (argc < 4)
                goto usage;

            t = atoi(argv[3]);
            if (t > 900000)
                t = 900000;

            st_config->nodisp_mode_time = t;

        } else if (strncmp(argv[2], "heavy", 5) == 0) {
            if (argc < 4)
                goto usage;

            t = atoi(argv[3]);
            if (t > 900000)
                t = 900000;

            st_config->heavy_mode_time = t;

        } else if (strncmp(argv[2], "sleep", 5) == 0) {
            if (argc < 4)
                goto usage;

            t = atoi(argv[3]);
            if (t > 900000)
                t = 900000;

            st_config->sleep_mode_time = t;

        } else if (strncmp(argv[2], "psram", 5) == 0) {
            if (argc < 4)
                goto usage;

            t = atoi(argv[3]);
            if (t > 900000)
                t = 900000;

            st_config->psram_mode_time = t;

        } else if (strncmp(argv[2], "reboot", 6) == 0) {
            if (argc < 5)
                goto usage;

            t = atoi(argv[3]);
            if (t < 60)
                t = 60;
            if (t > 900000)
                t = 900000;

            v = atoi(argv[4]);
            if (v < 1)
                v = 1;
            if (v > 1000)
                v = 1000;

            st_config->reboot_interval = t;
            st_config->reboot_times = v;

        } else {
            goto usage;
        }

        if (new_config)
            st_config_save();

    } else {
        goto usage;
    }

    return;
usage:
    st_main_usage();
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST,
                "st_main",
                "usage: st_main help",
                st_main_cmd);
#endif
