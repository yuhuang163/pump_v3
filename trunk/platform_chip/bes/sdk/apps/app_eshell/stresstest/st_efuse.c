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
#include "hal_analogif.h"
#include "pmu.h"

static int efuse_function_test(struct st_test_case *st_case)
{
    int ret = 0;
    unsigned short val;
    unsigned short tmp_val[16] = {0};

    /* test thread running, cancel function test */
    if (st_case->running)
        return 0;

    for (int i = 0; i < 16; i++) {
        pmu_get_efuse((enum PMU_EFUSE_PAGE_T)i, &val);
        tmp_val[i] = val;
    }

    for (int i = 0; i < 16; i++) {
        pmu_get_efuse((enum PMU_EFUSE_PAGE_T)i, &val);
        if(tmp_val[i] != val)
            ret++;
    }
    st_case->test_errs += ret;
    ST_LOG("%s function test %s.", st_case->name, ret ? "FAILED" : "OKAY");
    return ret;
}

static void efuse_test_thread(void *data)
{
    unsigned int i;
    unsigned short val;
    unsigned short tmp_val[16] = {0};
    struct st_test_case *st_case = (struct st_test_case *)data;

    for (i = 0; i < 16; i++) {
        pmu_get_efuse((enum PMU_EFUSE_PAGE_T)i, &val);
        tmp_val[i] = val;
        ST_LOG("EFUSE test tmp_val[%d] Efuse %d: 0x%04x", i, i, tmp_val[i]);
    }

    while (st_case->running) {
        if (st_case->test_enable) {
            for (i = 0; i < 16; i++) {
                pmu_get_efuse((enum PMU_EFUSE_PAGE_T)i, &val);
                if(tmp_val[i] != val){
                    ST_ERR("EFUSE test check ERROR!");
                    st_case->test_errs++;
                }
            }
            if(st_case->test_errs == 0)
                ST_LOG("EFUSE test check 32 bytes OKAY.");

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


struct st_test_case efuse_case = {
    .name = "EFUSE",

    .thread = NULL,
    .thread_func = efuse_test_thread,
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

    .func_test = efuse_function_test,
};

void st_test_efuse_register(void)
{
    st_test_case_register(&efuse_case);
}

/************ eshell command *******************/

static void unitest_usage(void)
{
    ST_LOG("Usage: \r\n");
    ST_LOG("  st_efuse init, start test thread\r\n");
    ST_LOG("  st_efuse once, run test one time\r\n");
    ST_LOG("  st_efuse start, start test loop\r\n");
    ST_LOG("  st_efuse stop, stop test loop\r\n");
    ST_LOG("  st_efuse exit, stop test thread\r\n");
}

static void st_efuse_cmd(int argc, char *argv[])
{
    if(argc < 2)
        goto usage;

    if(strncmp(argv[1], "init", 4) == 0){
        st_case_init(&efuse_case);
    }else if(strncmp(argv[1], "once", 4) == 0){
        st_case_run_once(&efuse_case);
    }else if(strncmp(argv[1], "start", 5) == 0){
        st_case_loop_start(&efuse_case);
    }else if(strncmp(argv[1], "stop", 4) == 0){
        st_case_loop_stop(&efuse_case);
    }else if(strncmp(argv[1], "exit", 4) == 0){
        st_case_exit(&efuse_case);
    }else
        goto usage;
    return;
usage:
    unitest_usage();
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST,
                "st_efuse",
                "usage: st_efuse help",
                st_efuse_cmd);
#endif
