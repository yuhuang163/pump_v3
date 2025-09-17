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
#include "hal_location.h"

#include "st_test.h"

#ifdef CHIP_HAS_EMMC

#include "host.h"
#include "card.h"

#ifdef PSRAM_ENABLE
#define TEST_SIZE     (64*1024)
static uint8_t read_buf[TEST_SIZE] ALIGNED(64) PSRAM_NC_BSS_LOC;
static uint8_t write_buf[TEST_SIZE] ALIGNED(64) PSRAM_NC_BSS_LOC;
#else
#define TEST_SIZE     (16*1024)
static uint8_t read_buf[TEST_SIZE] ALIGNED(64) SRAM_BSS_LOC;
static uint8_t write_buf[TEST_SIZE] ALIGNED(64) SRAM_BSS_LOC;
#endif

static uint32_t test_offset = 0x100;

extern bool emmc_is_inited(void);
extern int emmc_init_default(void);
extern int emmc_write(uint32_t offset, uint32_t size, uint32_t addr);
extern int emmc_read(uint32_t offset, uint32_t size, uint32_t addr);
extern int emmc_erase(uint32_t offset, uint32_t size);
extern uint32_t emmc_block_count(void);
extern uint32_t emmc_block_size(void);

static int emmc_test_wr(void)
{
    int errs = 0;

    if (emmc_block_count() == 0 || emmc_block_size() == 0)
        return 0;

    if (test_offset + (TEST_SIZE / emmc_block_size()) > emmc_block_count())
        test_offset = 0x100;

    for (int i = 0; i < TEST_SIZE; i++)
        write_buf[i] = rand() & 0xff;
    memset(read_buf, 0, TEST_SIZE);

    emmc_write(test_offset, TEST_SIZE, (uint32_t)write_buf);
    emmc_read(test_offset, TEST_SIZE, (uint32_t)read_buf);
    for (int i = 0; i < TEST_SIZE; i++) {
        if (read_buf[i] != write_buf[i]) {
            //ST_ERR("EMMC check error %d: write %x, read %x\r\n",
            //        i, write_buf[i] & 0xff, read_buf[i] & 0xff);
            errs++;
        }
    }

    if (errs > 0)
        ST_LOG("EMMC Write & Read test, got %d errors.\r\n", errs);
    else
        ST_LOG("EMMC Write & Read test, %u bytes.\r\n", TEST_SIZE);

    test_offset += TEST_SIZE / emmc_block_size();
    return errs;
}

#endif /* CHIP_HAS_EMMC */

static int emmc_function_test(struct st_test_case *st_case)
{
    int ret = 0;

    /* test thread running, cancel function test */
    if (st_case->running)
        return 0;

#ifdef CHIP_HAS_EMMC
    if (!emmc_is_inited())
        emmc_init_default();

    for (int i = 0; i < 100; i++)
        ret += emmc_test_wr();
#endif

    st_case->test_errs += ret;
    ST_LOG("%s function test %s.", st_case->name, ret ? "FAILED" : "OKAY");
    return ret;
}

static void emmc_test_thread(void *data)
{
    struct st_test_case *st_case = (struct st_test_case *)data;

#ifdef CHIP_HAS_EMMC
    if (!emmc_is_inited())
        emmc_init_default();
#endif

    while (st_case->running) {
        if (st_case->test_enable) {
#ifdef CHIP_HAS_EMMC
            if (emmc_test_wr())
                st_case->test_errs++;
#endif
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

struct st_test_case emmc_case = {
    .name = "EMMC",

    .thread = NULL,
    .thread_func = emmc_test_thread,
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

    .func_test = emmc_function_test,
};

void st_test_emmc_register(void)
{
    st_test_case_register(&emmc_case);
}

/************ eshell command *******************/

static void st_emmc_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  st_emmc init, start test thread\r\n");
    eshell_putstring("  st_emmc exit, stop test thread\r\n");
    eshell_putstring("  st_emmc run_once, run test one time\r\n");
    eshell_putstring("  st_emmc start, start test loop\r\n");
    eshell_putstring("  st_emmc stop, stop test loop\r\n");
}

static void st_emmc_cmd(int argc, char *argv[])
{
    if (argc < 2)
        goto usage;

    if (strncmp(argv[1], "init", 4) == 0)
        st_case_init(&emmc_case);
    else if (strncmp(argv[1], "exit", 4) == 0)
        st_case_exit(&emmc_case);
    else if (strncmp(argv[1], "run_once", 8) == 0)
        st_case_run_once(&emmc_case);
    else if (strncmp(argv[1], "start", 5) == 0)
        st_case_loop_start(&emmc_case);
    else if (strncmp(argv[1], "stop", 4) == 0)
        st_case_loop_stop(&emmc_case);
    else
        goto usage;

    return;
usage:
    st_emmc_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST,
                "st_emmc",
                "usage: st_emmc help",
                st_emmc_cmd);

#endif
