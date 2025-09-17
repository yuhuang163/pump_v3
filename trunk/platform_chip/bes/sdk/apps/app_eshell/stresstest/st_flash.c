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
#include "hal_cache.h"
#include "hal_timer.h"
#include "hal_norflash.h"
#include "hal_sleep.h"
#include "hal_location.h"
#include "cmsis.h"

#include "st_test.h"

#define TEST_SIZE    0x4000                /* 16KB */
static uint8_t test_buf[TEST_SIZE];

#if 1
static uint8_t flash_buf[TEST_SIZE]
            FLASH_RODATA_LOC ALIGNED(TEST_SIZE) = {0};
static uint32_t flash_test_base = (uint32_t)flash_buf;
#else
extern uint32_t __flash_end[];
static uint32_t flash_test_base = (uint32_t)__flash_end;
#endif

static enum HAL_FLASH_ID_T st_flash_id = HAL_FLASH_ID_0;

/* flash open */
static void st_flash_test_open(void)
{
    flash_test_base = ALIGN(FLASH_C_TO_NC(flash_test_base), TEST_SIZE) - FLASH_NC_BASE;
    ST_LOG("FLASH test offset: 0x%x", flash_test_base);
}

static int st_flash_test_rw(uint8_t pattern)
{
    uint32_t lock;
    int ret = 0;
    int errs = 0;

    if (!flash_test_base){
        return 0;
    }
    lock = int_lock();

    /* test erase */
    ret = hal_norflash_erase(st_flash_id, flash_test_base, TEST_SIZE);
    if (ret != 0) {
        ST_LOG("FLASH erase failed on 0x%x, size 0x%x", flash_test_base, TEST_SIZE);
        errs++;
        goto exit;
    }

    /* test write */
    memset(test_buf, pattern, TEST_SIZE);
    ret = hal_norflash_write(st_flash_id, flash_test_base, test_buf, TEST_SIZE);
    if (ret != 0) {
        ST_LOG("FLASH write failed on 0x%x, size 0x%x", flash_test_base, TEST_SIZE);
        errs++;
        goto exit;
    }

    /* test read */
    memset(test_buf, 0, TEST_SIZE);
    ret = hal_norflash_read(st_flash_id, flash_test_base, test_buf, TEST_SIZE);
    if (ret != 0) {
        ST_LOG("FLASH read failed on 0x%x, size 0x%x", flash_test_base, TEST_SIZE);
        errs++;
        goto exit;
    }

    /* compare data with pattern */
    for (int i = 0; i < TEST_SIZE; i++) {
        if (test_buf[i] != pattern)
            errs++;
    }

exit:
    int_unlock(lock);

    if (errs != 0)
        ST_ERR("FLASH test FAILED on 0x%x, size 0x%x",
                flash_test_base, TEST_SIZE);
    else
        ST_LOG("FLASH test OKAY on 0x%x, size 0x%x", flash_test_base, TEST_SIZE);
    return errs;
}

static void flash_test_thread(void *data)
{
    struct st_test_case *st_case = (struct st_test_case *)data;
    st_flash_test_open();
    while (st_case->running) {
        if (st_case->test_enable) {
            st_test_lock();
            if (st_flash_test_rw(0x5a))
                st_case->test_errs++;
            if (st_flash_test_rw(0xa5))
                st_case->test_errs++;
            st_test_unlock();

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

struct st_test_case flash_case = {
    .name = "flash",

    .thread = NULL,
    .thread_func = flash_test_thread,
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
    .func_test = NULL,
};

void st_test_flash_register(void)
{
    st_test_case_register(&flash_case);
}

/************ eshell command *******************/

static void st_flash_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  st_flash init, initialize efuse and start test thread\r\n");
    eshell_putstring("  st_flash exit, close efuse and stop test thread\r\n");
    eshell_putstring("  st_flash run_once, run test one time\r\n");
    eshell_putstring("  st_flash start, start test loop\r\n");
    eshell_putstring("  st_flash stop, stop test loop\r\n");
}

static void st_flash_cmd(int argc, char *argv[])
{
    if (argc < 2)
        goto usage;

    if (strncmp(argv[1], "init", 4) == 0)
        st_case_init(&flash_case);
    else if (strncmp(argv[1], "exit", 4) == 0)
        st_case_exit(&flash_case);
    else if (strncmp(argv[1], "run_once", 8) == 0)
        st_case_run_once(&flash_case);
    else if (strncmp(argv[1], "start", 5) == 0)
        st_case_loop_start(&flash_case);
    else if (strncmp(argv[1], "stop", 4) == 0)
        st_case_loop_stop(&flash_case);
    else
        goto usage;

    return;
usage:
    st_flash_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST,
                "st_flash",
                "usage: st_flash help",
                st_flash_cmd);
#endif
