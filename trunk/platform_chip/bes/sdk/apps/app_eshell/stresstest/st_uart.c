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
#include "hal_uart.h"
#include "hal_sysfreq.h"
#include "hal_cache.h"

#include "st_test.h"

#define TEST_UART_DEBUG

#ifdef CHIP_SUBSYS_SENS
#define UART_ID             (HAL_UART_ID_2)
#else
#define UART_ID             (HAL_UART_ID_1)
#endif
#define UART_BAUD           921600
#define UART_FIFO_SIZE      16

#define UART_TEST_SIZE      8
static char uart_txbuff[UART_TEST_SIZE] ALIGNED(8);
static char uart_rxbuff[UART_TEST_SIZE] ALIGNED(8);

static bool uart_opened = false;

POSSIBLY_UNUSED
static int uart_test_open(void)
{
    int ret = 0;
    struct HAL_UART_CFG_T cfg = {
        .parity             = HAL_UART_PARITY_NONE,
        .stop               = HAL_UART_STOP_BITS_1,
        .data               = HAL_UART_DATA_BITS_8,
        .flow               = HAL_UART_FLOW_CONTROL_NONE,
        .rx_level           = HAL_UART_FIFO_LEVEL_1_4,
        .tx_level           = HAL_UART_FIFO_LEVEL_1_2,
        .baud               = UART_BAUD,
        .dma_rx             = false,
        .dma_tx             = false,
        .dma_rx_stop_on_err = false,
    };

#ifdef CHIP_SUBSYS_SENS
    if (UART_ID == HAL_UART_ID_2)
        hal_iomux_set_uart2();
    else
#endif
    if (UART_ID == HAL_UART_ID_1)
        hal_iomux_set_uart1();
    else if (UART_ID == HAL_UART_ID_0)
        hal_iomux_set_uart0();
    else
        return -1;

    ret = hal_uart_open(UART_ID, &cfg);
#ifdef TEST_UART_DEBUG
    ST_LOG("UART open %s", ret ? "failed" : "okay");
#endif
    if (!ret)
        uart_opened = true;
    return ret;
}

POSSIBLY_UNUSED
static int uart_test_close(void)
{
    int ret = hal_uart_close(UART_ID);
#ifdef TEST_UART_DEBUG
    ST_LOG("UART close %s", ret ? "failed" : "okay");
#endif
    uart_opened = false;
    return ret;
}

static int uart_test_write(char *buf, int sz)
{
    int start = hal_sys_timer_get();
    int i = 0;
    while (i < sz && TICKS_TO_MS(hal_sys_timer_get()-start) < 100) {
        if (hal_uart_writable(UART_ID))
            hal_uart_putc(UART_ID, buf[i++]);
        else
            osDelay(1);
    }
    return (i == sz) ? 0 : -1;
}

static int uart_test_read(char *buf, int sz)
{
    int start = hal_sys_timer_get();
    int i = 0;
    while (i < sz && TICKS_TO_MS(hal_sys_timer_get()-start) < 100) {
        if (hal_uart_readable(UART_ID))
            buf[i++] = hal_uart_getc(UART_ID);
        else
            osDelay(1);
    }
    return (i == sz) ? 0 : -1;
}


POSSIBLY_UNUSED
static int uart_test_txrx(void)
{
    int ret = 0;

    if (!uart_opened)
        return 1;

    for (int i = 0; i < UART_TEST_SIZE; i++)
        uart_txbuff[i] = rand() & 0xff;
    memset(uart_rxbuff, 0, UART_TEST_SIZE);

    ret = uart_test_write(uart_txbuff, UART_TEST_SIZE);
    if (ret)
        return 1;

    osDelay(1);

    ret = uart_test_read(uart_rxbuff, UART_TEST_SIZE);
    if (ret)
        return 1;

    for (int i = 0; i < UART_TEST_SIZE; i++) {
        if (uart_txbuff[i] != uart_rxbuff[i]) {
            ST_ERR("UART TEST ERROR %02d: send 0x%x, recv 0x%x",
                        i, uart_txbuff[i] & 0xff, uart_rxbuff[i] & 0xff);
            return 1;
        }
    }

#ifdef TEST_UART_DEBUG
    ST_LOG("UART TEST OKAY:  %d bytes", UART_TEST_SIZE);
#endif
    return 0;
}

static int uart_function_test(struct st_test_case *st_case)
{
    int ret = 0;

    /* test thread running, cancel function test */
    if (st_case->running)
        return 0;

    uart_test_open();
    for (int i = 0; i < 50; i++) {
        if (uart_test_txrx())
            ret++;
    }
    uart_test_close();
    st_case->test_errs += ret;
    ST_LOG("%s function test %s.", st_case->name, ret ? "FAILED" : "OKAY");
    return ret;
}

static void uart_test_thread(void *data)
{
    struct st_test_case *st_case = (struct st_test_case *)data;
    uart_test_open();
    while (st_case->running) {
        if (st_case->test_enable) {
            if (uart_test_txrx())
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
    uart_test_close();
    osThreadExit();
}

struct st_test_case uart_case = {
    .name = "UART",

    .thread = NULL,
    .thread_func = uart_test_thread,
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

    .func_test = uart_function_test,
};

void st_test_uart_register(void)
{
    st_test_case_register(&uart_case);
}

/************ eshell command *******************/

static void st_uart_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  st_uart init, initialize uart and start test thread\r\n");
    eshell_putstring("  st_uart exit, close uart and stop test thread\r\n");
    eshell_putstring("  st_uart run_once, run test one time\r\n");
    eshell_putstring("  st_uart start, start test loop\r\n");
    eshell_putstring("  st_uart stop, stop test loop\r\n");
}

static void st_uart_cmd(int argc, char *argv[])
{
    if (argc < 2)
        goto usage;

    if (strncmp(argv[1], "init", 4) == 0)
        st_case_init(&uart_case);
    else if (strncmp(argv[1], "exit", 4) == 0)
        st_case_exit(&uart_case);
    else if (strncmp(argv[1], "run_once", 8) == 0)
        st_case_run_once(&uart_case);
    else if (strncmp(argv[1], "start", 5) == 0)
        st_case_loop_start(&uart_case);
    else if (strncmp(argv[1], "stop", 4) == 0)
        st_case_loop_stop(&uart_case);
    else
        goto usage;

    return;
usage:
    st_uart_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST,
                "st_uart",
                "usage: st_uart help",
                st_uart_cmd);

#endif
