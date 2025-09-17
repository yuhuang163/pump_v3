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
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "cmsis_os2.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_psram.h"

#include "psram_test.h"
#include "st_test.h"

static int psram_function_test(struct st_test_case *st_case)
{
    int ret = 0;

    /* test thread running, cancel function test */
    if (st_case->running)
        return 0;

    ret += psram_test_8bits(PSRAM_BASE, PSRAM_SIZE, TEST_FIX2_PATTERN);
    ret += psram_test_16bits(PSRAM_BASE, PSRAM_SIZE, TEST_FIX2_PATTERN);
    ret += psram_test_32bits(PSRAM_BASE, PSRAM_SIZE, TEST_FIX2_PATTERN);
    ret += psram_test_8bits(PSRAM_BASE, PSRAM_SIZE, TEST_INC_PATTERN);
    ret += psram_test_16bits(PSRAM_BASE, PSRAM_SIZE, TEST_INC_PATTERN);
    ret += psram_test_32bits(PSRAM_BASE, PSRAM_SIZE, TEST_INC_PATTERN);
    st_case->test_errs += ret;
    ST_LOG("%s function test %s.", st_case->name, ret ? "FAILED" : "OKAY");
    return ret;
}

int psram_full_scan_test(int timeout_ms)
{
    int ret = 0;
    uint32_t start = hal_sys_timer_get();
    while (TICKS_TO_MS(hal_sys_timer_get()-start) < timeout_ms) {
        ret += psram_test_8bits(PSRAM_BASE, PSRAM_SIZE, TEST_FIX2_PATTERN);
        ret += psram_test_16bits(PSRAM_BASE, PSRAM_SIZE, TEST_FIX2_PATTERN);
        ret += psram_test_32bits(PSRAM_BASE, PSRAM_SIZE, TEST_FIX2_PATTERN);
        ret += psram_test_8bits(PSRAM_BASE, PSRAM_SIZE, TEST_INC_PATTERN);
        ret += psram_test_16bits(PSRAM_BASE, PSRAM_SIZE, TEST_INC_PATTERN);
        ret += psram_test_32bits(PSRAM_BASE, PSRAM_SIZE, TEST_INC_PATTERN);
    }
    return ret;
}

extern int test_psram_wr(bool random_pattern, uint32_t timeout_ms);

static void psram_test_thread(void *data)
{
    struct st_test_case *st_case = (struct st_test_case *)data;
    while (st_case->running) {
        if (st_case->test_enable) {
            if (test_psram_wr(true, 0))
                st_case->test_errs++;

            if (st_case->mutex)
                osMutexAcquire(st_case->mutex, osWaitForever);
            if (!st_case->loop_enable)
                st_case->test_enable = false;
            if (st_case->mutex)
                osMutexRelease(st_case->mutex);
        }
        osDelay(50);
    }
    osThreadExit();
}

struct st_test_case psram_case = {
    .name = "PSRAM",

    .thread = NULL,
    .thread_func = psram_test_thread,
    .thread_priority = osPriorityNormal,
    .thread_stack_size = 1024,

    .mutex = NULL,

    .running = false,
    .loop_enable = false,
    .test_enable = false,
    .test_errs = 0,

    .init = st_case_init,
    .exit = st_case_exit,
    .run_once = st_case_run_once,
    .loop_start = st_case_loop_start,
    .loop_stop = st_case_loop_stop,
    .check_errors = st_case_check_errors,
    .func_test = psram_function_test,
};

void st_test_psram_register(void)
{
    st_test_case_register(&psram_case);
}

/************ eshell command *******************/

static void st_psram_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  st_psram init, start test thread\r\n");
    eshell_putstring("  st_psram exit, stop test thread\r\n");
    eshell_putstring("  st_psram run_once, run test one time\r\n");
    eshell_putstring("  st_psram start, start test loop\r\n");
    eshell_putstring("  st_psram stop, stop test loop\r\n");
}

static void st_psram_cmd(int argc, char *argv[])
{
    if (argc < 2)
        goto usage;

    if (strncmp(argv[1], "init", 4) == 0)
        st_case_init(&psram_case);
    else if (strncmp(argv[1], "exit", 4) == 0)
        st_case_exit(&psram_case);
    else if (strncmp(argv[1], "run_once", 8) == 0)
        st_case_run_once(&psram_case);
    else if (strncmp(argv[1], "start", 5) == 0)
        st_case_loop_start(&psram_case);
    else if (strncmp(argv[1], "stop", 4) == 0)
        st_case_loop_stop(&psram_case);
    else
        goto usage;

    return;
usage:
    st_psram_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST,
                "st_psram",
                "usage: st_psram help",
                st_psram_cmd);

#endif
