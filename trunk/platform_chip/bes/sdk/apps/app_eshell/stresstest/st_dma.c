/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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
#include "cmsis.h"
#include "cmsis_os2.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_cache.h"
#include "hal_dma.h"
#include "hal_location.h"

#include "st_test.h"

#define TEST_DMA_SIZE       2048
#define TEST_DMA_CHANNELS   2

static uint8_t src_buf[TEST_DMA_CHANNELS][TEST_DMA_SIZE] ALIGNED(8) PSRAM_NC_BSS_LOC;
static uint8_t dst_buf[TEST_DMA_CHANNELS][TEST_DMA_SIZE] ALIGNED(8) PSRAM_NC_BSS_LOC;
static int dma_ch[TEST_DMA_CHANNELS];
static bool dma_working[32];

static void dma_irq_handler(uint8_t chan, uint32_t remain_tsize,
        uint32_t error, struct HAL_DMA_DESC_T *lli)
{
    if (error) {
        ST_ERR("ERROR: DMA copy error!");
    } else {
        /* release the DMA channel. */
        hal_dma_free_chan(chan);
    }
    dma_working[chan] = false;
}

static void dma_start_cb(uint8_t chan)
{
    //ST_LOG("Test DMA copy start, chan[%d] used.\r\n", chan);
}

static int test_dma_start(int index)
{
    enum HAL_DMA_RET_T ret = HAL_DMA_OK;
    struct HAL_DMA_DESC_T dma_desc = {0};
    volatile uint32_t *src_p, *dst_p;
    uint32_t src_addr = (uint32_t)&src_buf[index][0];
    uint32_t dst_addr = (uint32_t)&dst_buf[index][0];
    uint32_t test_size = TEST_DMA_SIZE;

    /* fill source addresses by random pattern */
    for (uint32_t i = 0; i < test_size; i += 4) {
        src_p = (volatile uint32_t *)(src_addr + i);
        dst_p = (volatile uint32_t *)(dst_addr + i);
        *src_p = rand() & 0xffffffff;
        *dst_p = 0;
        __DSB();
    }

    dma_ch[index] = -1;

    /* desc config */
    struct HAL_DMA_CH_CFG_T cfg;
    memset(&cfg, 0, sizeof(cfg));

    cfg.type = HAL_DMA_FLOW_M2M_DMA;

    /* src config. */
    cfg.src = src_addr;
    cfg.src_tsize = test_size / HAL_DMA_BSIZE_4;
    cfg.src_bsize = HAL_DMA_BSIZE_4;
    cfg.src_width = HAL_DMA_WIDTH_WORD;
    cfg.src_periph = HAL_DMA_PERIPH_NULL;

    /* dst config. */
    cfg.dst = dst_addr;
    cfg.dst_bsize = HAL_DMA_BSIZE_4;
    cfg.dst_width = HAL_DMA_WIDTH_WORD;
    cfg.dst_periph = HAL_DMA_PERIPH_NULL;

    cfg.try_burst = 1;
    cfg.handler = dma_irq_handler;
    cfg.start_cb = dma_start_cb;
    cfg.ch = hal_dma_get_chan(HAL_GPDMA_MEM, HAL_DMA_HIGH_PRIO);
    if (cfg.ch == HAL_DMA_CHAN_NONE) {
        ST_ERR("ERROR: all DMA channels are busy.");
        return 1;
    }

    /* cofig: dma_desc. src, dst, lli, and ctrl */
    ret = hal_dma_init_desc(&dma_desc, &cfg, NULL, 1);
    if (ret) {
        hal_dma_free_chan(cfg.ch);
        ST_ERR("ERROR: DMA init failed. ret = %d", ret);
        return 1;
    }

    dma_working[cfg.ch] = true;

    /* start */
    ret = hal_dma_sg_start(&dma_desc, &cfg);
    if (ret) {
        hal_dma_free_chan(cfg.ch);
        ST_ERR("ERROR: DMA start failed.");
        dma_working[cfg.ch] = false;
        return 1;
    }
    dma_ch[index] = cfg.ch;
    return 0;
}

static int test_dma_check(int index)
{
    int err = 0;
    volatile uint32_t *src_p, *dst_p;
    int timeout = 200;
    uint32_t src_addr = (uint32_t)&src_buf[index][0];
    uint32_t dst_addr = (uint32_t)&dst_buf[index][0];
    uint32_t test_size = TEST_DMA_SIZE;

    if (dma_ch[index] < 0)
        return 0;

    while(dma_working[dma_ch[index]] && timeout > 0) {
        osDelay(1);
        timeout--;
    }

    for (uint32_t i = 0; i < test_size; i += 4) {
        src_p = (volatile uint32_t *)(src_addr + i);
        dst_p = (volatile uint32_t *)(dst_addr + i);
        if (*dst_p != *src_p) {
            ST_ERR("ERROR: addr %p, val 0x%x, expect 0x%x",
                        dst_p, *dst_p, *src_p);
            err++;
        }
    }

    dma_ch[index] = -1;

    if (err > 0)
        ST_ERR("DMA Test [%d] failed, got %d errors", index, err);
    else
        ST_LOG("DMA Test [%d] is OKAY! 0x%08x -> 0x%08x", index, src_addr, dst_addr);
    return err;
}

static int dma_test_xfer(void)
{
    int err = 0;

    for (int i = 0; i < TEST_DMA_CHANNELS; i++)
        test_dma_start(i);

    for (int i = 0; i < TEST_DMA_CHANNELS; i++) {
        if (test_dma_check(i))
            err++;
    }
    return err;
}

static int dma_function_test(struct st_test_case *st_case)
{
    int ret = 0;

    /* test thread running, cancel function test */
    if (st_case->running)
        return 0;

    for (int i = 0; i < 4; i++)
        ret += dma_test_xfer();

    st_case->test_errs += ret;

    ST_LOG("%s function test %s.", st_case->name, ret ? "FAILED" : "OKAY");
    return ret;
}

static void dma_test_thread(void *data)
{
    struct st_test_case *st_case = (struct st_test_case *)data;
    hal_dma_open();
    while (st_case->running) {
        if (st_case->test_enable) {
            if (dma_test_xfer())
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

struct st_test_case dma_case = {
    .name = "DMA",

    .thread = NULL,
    .thread_func = dma_test_thread,
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
    .func_test = dma_function_test,
};

void st_test_dma_register(void)
{
    st_test_case_register(&dma_case);
}

/************ eshell command *******************/

static void st_dma_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  st_dma init, start test thread\r\n");
    eshell_putstring("  st_dma exit, stop test thread\r\n");
    eshell_putstring("  st_dma run_once, run test one time\r\n");
    eshell_putstring("  st_dma start, start test loop\r\n");
    eshell_putstring("  st_dma stop, stop test loop\r\n");
}

static void st_dma_cmd(int argc, char *argv[])
{
    if (argc < 2)
        goto usage;

    if (strncmp(argv[1], "init", 4) == 0)
        st_case_init(&dma_case);
    else if (strncmp(argv[1], "exit", 4) == 0)
        st_case_exit(&dma_case);
    else if (strncmp(argv[1], "run_once", 8) == 0)
        st_case_run_once(&dma_case);
    else if (strncmp(argv[1], "start", 5) == 0)
        st_case_loop_start(&dma_case);
    else if (strncmp(argv[1], "stop", 4) == 0)
        st_case_loop_stop(&dma_case);
    else
        goto usage;

    return;
usage:
    st_dma_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST,
                "st_dma",
                "usage: st_dma help",
                st_dma_cmd);

#endif
