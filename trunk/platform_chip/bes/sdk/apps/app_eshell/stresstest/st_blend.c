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

#include "st_test.h"

extern int gpu_blend_verify_test(int times);

static int blend_function_test(struct st_test_case *st_case)
{
    /* test thread running, cancel function test */
    if (st_case->running)
        return 0;

    int ret = gpu_blend_verify_test(50);
    if (ret != 0)
        st_case->test_errs++;
    ST_LOG("%s function test %s.", st_case->name, ret ? "FAILED" : "OKAY");
    return ret;
}

static void blend_test_thread(void *data)
{
    struct st_test_case *st_case = (struct st_test_case *)data;
    while (st_case->running) {
        if (st_case->test_enable) {
            st_test_lock();
            if (gpu_blend_verify_test(5))
                st_case->test_errs++;
            st_test_unlock();

            if (st_case->mutex)
                osMutexAcquire(st_case->mutex, osWaitForever);
            if (!st_case->loop_enable)
                st_case->test_enable = false;
            if (st_case->mutex)
                osMutexRelease(st_case->mutex);
        }
        osDelay(10);
    }
    osThreadExit();
}

struct st_test_case blend_case = {
    .name = "GPU-Blend",

    .thread = NULL,
    .thread_func = blend_test_thread,
    .thread_priority = osPriorityNormal,
    .thread_stack_size = 2048,

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

    .func_test = blend_function_test,
};

void st_test_blend_register(void)
{
    st_test_case_register(&blend_case);
}

/************ eshell command *******************/

static void st_blend_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  st_blend init, start test thread\r\n");
    eshell_putstring("  st_blend exit, stop test thread\r\n");
    eshell_putstring("  st_blend run_once, run test one time\r\n");
    eshell_putstring("  st_blend start, start test loop\r\n");
    eshell_putstring("  st_blend stop, stop test loop\r\n");
}

static void st_blend_cmd(int argc, char *argv[])
{
    if (argc < 2)
        goto usage;

    if (strncmp(argv[1], "init", 4) == 0)
        st_case_init(&blend_case);
    else if (strncmp(argv[1], "exit", 4) == 0)
        st_case_exit(&blend_case);
    else if (strncmp(argv[1], "run_once", 8) == 0)
        st_case_run_once(&blend_case);
    else if (strncmp(argv[1], "start", 5) == 0)
        st_case_loop_start(&blend_case);
    else if (strncmp(argv[1], "stop", 4) == 0)
        st_case_loop_stop(&blend_case);
    else
        goto usage;

    return;
usage:
    st_blend_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST,
                "st_blend",
                "usage: st_blend help",
                st_blend_cmd);

#endif
