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

#if defined(UTILS_ESHELL_EN)

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "cmsis_os2.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_location.h"
#include "hal_iomux.h"
#include "hal_spi.h"
#include "hal_sysfreq.h"
#include "hal_cache.h"

#include "st_test.h"

#define TEST_SPI_DEBUG

#ifdef CHIP_SUBSYS_SENS
#define SPI_RATE (12000000)
#else
#define SPI_RATE (24000000)
#endif

#define TEST_WITH_DMA

#define TEST_SIZE  128

static char txbuff[TEST_SIZE] ALIGNED(8);
static char rxbuff[TEST_SIZE] ALIGNED(8);

POSSIBLY_UNUSED
static int spi_test_open(void)
{
    struct HAL_SPI_CFG_T spi_cfg = {0};
    int ret;

    spi_cfg.clk_delay_half = true;          /* clk phase */
    spi_cfg.clk_polarity = false;            /* clk polarity in steady state*/
    spi_cfg.slave = false;
#ifdef TEST_WITH_DMA
    spi_cfg.dma_rx = true;
    spi_cfg.dma_tx = true;
#else
    spi_cfg.dma_rx = false;
    spi_cfg.dma_tx = false;
#endif
    spi_cfg.rx_sep_line = false;
    spi_cfg.cs = 0;
    spi_cfg.rate = SPI_RATE;
    spi_cfg.rx_bits = 32;
    spi_cfg.tx_bits = 32;
    spi_cfg.rx_frame_bits = 0;
    spi_cfg.samp_delay = 1;
#if (SPI_RATE == 48000000)
    spi_cfg.samp_delay = 1;
#else
    spi_cfg.samp_delay = 0;
#endif

    hal_iomux_set_spi();
    ret = hal_spi_open(&spi_cfg);

#ifdef TEST_SPI_DEBUG
    ST_LOG("SPI bus open [%d]", ret);
#endif
    return ret;
}

static void spi_test_close(void)
{
    hal_spi_close(1);
#ifdef TEST_SPI_DEBUG
    ST_LOG("SPI bus close");
#endif
}

static void spi_wait_send_recv_done(int error)
{
    int timeout = 1000;
    if(error != 0)
        ST_ERR("SPI ERROR: %d", error);
    while (hal_spi_busy() && timeout-- > 0)
        osDelay(1);
}

static void spi_dma_recv_callback(int error)
{
    if(error != 0)
        ST_ERR("SPI DMA ERROR: %d", error);
}

static int spi_test_txrx_dma(void)
{
    int ret;

#if 0
    memset(txbuff, rand() & 0xff , TEST_SIZE);
#else
    for (int i = 0; i < TEST_SIZE; i++)
        txbuff[i] = rand() & 0xff;
#endif
    memset(rxbuff, 0, TEST_SIZE);

    hal_cache_sync(HAL_CACHE_ID_D_CACHE, (uint32_t)txbuff, TEST_SIZE);
    hal_cache_sync(HAL_CACHE_ID_D_CACHE, (uint32_t)rxbuff, TEST_SIZE);

    ret = hal_spi_dma_recv(txbuff , rxbuff, TEST_SIZE, spi_dma_recv_callback);
    spi_wait_send_recv_done(ret);

    if (ret)
        return 1;

    hal_cache_invalidate(HAL_CACHE_ID_D_CACHE, (uint32_t)rxbuff, TEST_SIZE);

    for (int i = 0; i < TEST_SIZE; i++) {
        if (txbuff[i] != rxbuff[i]) {
            ST_ERR("SPI DMA TEST ERROR: send 0x%x, recv 0x%x", txbuff[i] & 0xff, rxbuff[i] & 0xff);
            return 1;
        }
    }
#ifdef TEST_SPI_DEBUG
    ST_LOG("SPI DMA TEST: %d bytes", TEST_SIZE);
#endif
    return 0;
}


POSSIBLY_UNUSED
static int spi_test_txrx_nodma(void)
{
    int ret;

#if 0
    memset(txbuff, rand() & 0xff, TEST_SIZE);
#else
    for (int i = 0; i < TEST_SIZE; i++)
        txbuff[i] = rand() & 0xff;
#endif
    memset(rxbuff, 0, TEST_SIZE);

    ret = hal_spi_recv(&txbuff , rxbuff, TEST_SIZE);
    spi_wait_send_recv_done(ret);

    if (ret)
        return 1;

    for (int i = 0; i < TEST_SIZE; i++) {
        if (txbuff[i] != rxbuff[i]) {
            ST_ERR("SPI NO-DMA TEST ERROR: send 0x%x, recv 0x%x", txbuff[i] & 0xff, rxbuff[i] & 0xff);
            return 1;
        }
    }
#ifdef TEST_SPI_DEBUG
    ST_LOG("SPI NO-DMA TEST: %d bytes", TEST_SIZE);
#endif
    return 0;
}

static int spi_function_test(struct st_test_case *st_case)
{
    int ret = 0;

    /* test thread running, cancel function test */
    if (st_case->running)
        return 0;

    spi_test_open();
    for (int i = 0; i < 50; i++) {
#ifdef TEST_WITH_DMA
        if (spi_test_txrx_dma())
            ret++;
#else
        if (spi_test_txrx_nodma())
            ret++;
#endif
    }
    spi_test_close();
    st_case->test_errs += ret;
    ST_LOG("%s function test %s.", st_case->name, ret ? "FAILED" : "OKAY");
    return ret;
}

static void spi_test_thread(void *data)
{
    struct st_test_case *st_case = (struct st_test_case *)data;
    spi_test_open();
    while (st_case->running) {
        if (st_case->test_enable) {
#ifdef TEST_WITH_DMA
            if (spi_test_txrx_dma())
                st_case->test_errs++;
#else
            if (spi_test_txrx_nodma())
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
    spi_test_close();
    osThreadExit();
}

struct st_test_case spi_case = {
    .name = "SPI",

    .thread = NULL,
    .thread_func = spi_test_thread,
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

    .func_test = spi_function_test,
};

void st_test_spi_register(void)
{
    st_test_case_register(&spi_case);
}

/************ eshell command *******************/

static void st_spi_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  st_spi init, initialize spi and start test thread\r\n");
    eshell_putstring("  st_spi exit, close spi and stop test thread\r\n");
    eshell_putstring("  st_spi run_once, run test one time\r\n");
    eshell_putstring("  st_spi start, start test loop\r\n");
    eshell_putstring("  st_spi stop, stop test loop\r\n");
}

static void st_spi_cmd(int argc, char *argv[])
{
    if (argc < 2)
        goto usage;

    if (strncmp(argv[1], "init", 4) == 0)
        st_case_init(&spi_case);
    else if (strncmp(argv[1], "exit", 4) == 0)
        st_case_exit(&spi_case);
    else if (strncmp(argv[1], "run_once", 8) == 0)
        st_case_run_once(&spi_case);
    else if (strncmp(argv[1], "start", 5) == 0)
        st_case_loop_start(&spi_case);
    else if (strncmp(argv[1], "stop", 4) == 0)
        st_case_loop_stop(&spi_case);
    else
        goto usage;

    return;
usage:
    st_spi_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST,
                "st_spi",
                "usage: st_spi help",
                st_spi_cmd);

#endif
