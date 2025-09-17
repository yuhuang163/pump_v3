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

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cmsis_os2.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "hal_uart.h"
#include "hal_rmt_ipc.h"

#include "st_test.h"

/* m55 to m33 */
#define ST_SENS_TEST_START      0x10000001
#define ST_SENS_TEST_STOP       0x10000002
#define ST_SENS_QUERY_RESULT    0x10000003

/* m33 to m55 */
#define ST_SENS_RET_OKAY        0x80000000
#define ST_SENS_RET_ERR         0x80000001

#ifdef CHIP_SUBSYS_SENS

#ifndef OPENAMP_ENABLE
static HAL_RMT_IPC_EP_T rmt_ipc_ep;
#endif

struct st_test_config {
    /* test cases */
    uint8_t uart_enable;
    uint8_t spi_enable;
    uint8_t i2c_enable;
};

static struct st_test_config st_config;

static void st_config_init_default(void)
{
    memset(&st_config, 0, sizeof(st_config));

    st_config.uart_enable = 1;
    st_config.spi_enable = 1;
    st_config.i2c_enable = 1;
}

static void st_config_init(void)
{
    st_config_init_default();
}

static void st_config_dump(void)
{
    eshell_putstring("SLT Config: \r\n");
    eshell_putstring("  Cases:\r\n");
    eshell_putstring("    UART      [%s]\r\n", st_config.uart_enable ? "Enabled" : "Disabled");
    eshell_putstring("    SPI       [%s]\r\n", st_config.spi_enable ? "Enabled" : "Disabled");
    eshell_putstring("    i2c       [%s]\r\n", st_config.spi_enable ? "Enabled" : "Disabled");
}

static bool st_running = false;

#ifdef OPENAMP_ENABLE
osStatus_t rpmsg_target_put_rawdata(const void *data, uint32_t size);
void *rpmsg_get_target_rawdata(void);
#else
static bool ipc_opened = false;

static unsigned int sens_rx_cb(const void *data, unsigned int len)
{
    uint32_t cmd = *((uint32_t *)data);
    if (cmd == ST_SENS_TEST_START) {
        st_running = true;
        ST_LOG("#### Recevied command 0x%x, start test.", cmd);
    } else if (cmd == ST_SENS_TEST_STOP) {
        st_running = false;
        ST_LOG("#### Recevied command 0x%x, stop test.", cmd);
    } else if (cmd == ST_SENS_QUERY_RESULT) {
        uint32_t errs = st_cases_check_errors();
        uint32_t r = errs ? ST_SENS_RET_ERR : ST_SENS_RET_OKAY;
        ST_LOG("#### Recevied command 0x%x, send result.", cmd);
        hal_rmt_ipc_send(rmt_ipc_ep, &r, sizeof(uint32_t));
    } else {
        ST_LOG("#### Recevied unkown command 0x%x", cmd);
    }
    return len;
}

static void sens_tx_cb(const void *data, unsigned int len)
{
}
#endif
static void st_test_works(void)
{
    uint32_t test_time = rand() % 10 + 1;

    ST_LOG("Start test..%ds", test_time);

    st_sysfreq_high();
    ST_LOG("CPU freq: %uM", hal_sys_timer_calc_cpu_freq(5, 1)/1000/1000);

    st_cases_loop_start();

    for (int i = 0; i < test_time*10 && st_running; i++){
        osDelay(100);
#ifdef OPENAMP_ENABLE
        uint32_t cmd = *(uint32_t *)rpmsg_get_target_rawdata();
        if(cmd == ST_SENS_TEST_STOP){
            st_running = false;
            break;
        }
        else if(cmd == ST_SENS_QUERY_RESULT){
            uint32_t errs = st_cases_check_errors();
            uint32_t r = errs ? ST_SENS_RET_ERR : ST_SENS_RET_OKAY;
            rpmsg_target_put_rawdata((char *)(&r),4);
        }
#endif
    }

    st_cases_loop_stop();

    st_sysfreq_sleep();
}

static void st_sens_start(void)
{
    st_sysfreq_high();

    st_cases_clear();

    if (st_config.uart_enable)
        st_test_uart_register();
    if (st_config.spi_enable)
        st_test_spi_register();
    if (st_config.i2c_enable)
        st_test_i2c_register();

#ifndef OPENAMP_ENABLE
    if (!ipc_opened) {
        hal_rmt_ipc_open(HAL_RMT_IPC_CORE_SENS_C0, HAL_RMT_IPC_CHAN_0, sens_rx_cb, sens_tx_cb, false, &rmt_ipc_ep);
        hal_rmt_ipc_start_recv(rmt_ipc_ep);
        ipc_opened = true;
    }
#endif

    ST_LOG("==== Start sensorhub test loop.");

    st_cases_init();

    while (1) {
#ifdef OPENAMP_ENABLE
        uint32_t cmd = *(uint32_t *)rpmsg_get_target_rawdata();
        if(cmd == ST_SENS_TEST_START)
            st_running = true;
        else if(cmd == ST_SENS_TEST_STOP)
            st_running = false;
#endif
        if (st_running)
            st_test_works();
        osDelay(50);
    }
}

static void st_sens_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  st_sens config show\r\n");
    eshell_putstring("  st_sens config clean\r\n");
    eshell_putstring("  st_sens config enable uart|spi\r\n");
    eshell_putstring("  st_sens config disable uart|spi\r\n");
    eshell_putstring("  st_sens start\r\n");
}

static void st_sens_cmd(int argc, char *argv[])
{
    static bool config_inited = false;

    if (argc < 2)
        goto usage;

    if (!config_inited) {
        st_config_init();
        config_inited = true;
    }

    if (strncmp(argv[1], "start", 5) == 0) {

        st_sens_start();

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

            if (strncmp(argv[3], "uart", 4) == 0)
                st_config.uart_enable = 1;
            else if (strncmp(argv[3], "spi", 3) == 0)
                st_config.spi_enable = 1;
            else if (strncmp(argv[3], "i2c", 3) == 0)
                st_config.i2c_enable = 1;
            else
                goto usage;

        } else if (strncmp(argv[2], "disable", 7) == 0) {
            if (argc < 4)
                goto usage;

            if (strncmp(argv[3], "uart", 4) == 0)
                st_config.uart_enable = 0;
            else if (strncmp(argv[3], "spi", 3) == 0)
                st_config.spi_enable = 0;
            else if (strncmp(argv[3], "i2c", 3) == 0)
                st_config.i2c_enable = 0;
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
    st_sens_usage();
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST,
                "st_sens",
                "usage: st_sens help",
                st_sens_cmd);

#endif /* CHIP_SUBSYS_SENS */


#if defined(__SYS_AS_MAIN__) && defined(SENSOR_HUB_TEST)

#ifndef OPENAMP_ENABLE
static HAL_RMT_IPC_EP_T rmt_ipc_ep;
#endif

#ifdef OPENAMP_ENABLE
extern osStatus_t rpmsg_controller_put_rawdata(const void *data, uint32_t size);
extern void *rpmsg_get_controller_rawdata(void);
#else
static bool ipc_opened = false;
static int st_result = -1;
static unsigned int st_sens_data_rx_cb(const void *data, unsigned int len)
{
    uint32_t ret = *((uint32_t *)data);
    if (ret == ST_SENS_RET_OKAY)
        st_result = 0;
    else
        st_result = 1;
    return len;
}

static void st_sens_data_tx_cb(const void *data, unsigned int len)
{
}

static void st_sens_open_ipc(void)
{
    if (!ipc_opened) {
        hal_rmt_ipc_open(HAL_RMT_IPC_CORE_SENS_C0, HAL_RMT_IPC_CHAN_0, st_sens_data_rx_cb, st_sens_data_tx_cb, false, &rmt_ipc_ep);
        hal_rmt_ipc_start_recv(rmt_ipc_ep);
        ipc_opened = true;
    }
}
#endif

static int st_sens_check_errors(struct st_test_case *st_case)
{
    uint32_t start = hal_sys_timer_get();
    uint32_t cmd = ST_SENS_QUERY_RESULT;

    if (!st_case)
        return 0;
#ifdef OPENAMP_ENABLE
    rpmsg_controller_put_rawdata((char *)(&cmd),4);
    while (TICKS_TO_MS(hal_sys_timer_get()-start) < 200)
        osDelay(10);
    cmd = *(uint32_t *)rpmsg_get_controller_rawdata();

    if (cmd == ST_SENS_RET_OKAY)
        st_case->test_errs = 0;
    else
        st_case->test_errs = 1;
#else
    st_result = -1;

    st_sens_open_ipc();

    hal_rmt_ipc_send(rmt_ipc_ep, &cmd, sizeof(uint32_t));
    while (st_result < 0 && TICKS_TO_MS(hal_sys_timer_get()-start) < 200)
        osDelay(10);

    if (st_result)
        st_case->test_errs = 1;
    else
        st_case->test_errs = 0;
#endif

    ST_LOG("%s test found %u errors.",
                st_case->name,
                st_case->test_errs);
    return st_case->test_errs;
}

static void st_sens_loop_start(struct st_test_case *st_case)
{
    uint32_t cmd = ST_SENS_TEST_START;
    if (!st_case)
        return;

#ifdef OPENAMP_ENABLE
    rpmsg_controller_put_rawdata((char *)(&cmd),4);
    osDelay(100);
#else
    st_sens_open_ipc();
    hal_rmt_ipc_send(rmt_ipc_ep, &cmd, sizeof(uint32_t));
#endif

    ST_LOG("%s test loop start.", st_case->name);
}

static void st_sens_loop_stop(struct st_test_case *st_case)
{
    uint32_t cmd = ST_SENS_TEST_STOP;
    if (!st_case)
        return;

#ifdef OPENAMP_ENABLE
    rpmsg_controller_put_rawdata((char *)(&cmd),4);
    osDelay(100);
#else
    st_sens_open_ipc();
    hal_rmt_ipc_send(rmt_ipc_ep, &cmd, sizeof(uint32_t));
#endif

    ST_LOG("%s test loop stop.", st_case->name);
}

struct st_test_case sens_case = {
    .name = "SENS",

    .thread = NULL,
    .thread_func = NULL,
    .thread_priority = 0,
    .thread_stack_size = 0,

    .mutex = NULL,

    .running = false,
    .loop_enable = false,
    .test_enable = false,
    .test_errs = 0,

    .init = NULL,
    .exit = NULL,
    .run_once = NULL,
    .loop_start = st_sens_loop_start,
    .loop_stop = st_sens_loop_stop,
    .check_errors = st_sens_check_errors,

    .func_test = NULL,
};

void st_test_sens_register(void)
{
    st_test_case_register(&sens_case);
}

static void st_sens_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  st_sens start, start sensorhub test loop\r\n");
    eshell_putstring("  st_sens stop, stop sensorhub test loop\r\n");
}

static void st_sens_cmd(int argc, char *argv[])
{
    if (argc < 2)
        goto usage;

    if (strncmp(argv[1], "start", 5) == 0)
        st_sens_loop_start(&sens_case);
    else if (strncmp(argv[1], "stop", 4) == 0)
        st_sens_loop_stop(&sens_case);
    else
        goto usage;

    return;
usage:
    st_sens_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST,
                "st_sens",
                "usage: st_sens help",
                st_sens_cmd);

#endif /* __SYS_AS_MAIN__ && SENSOR_HUB_TEST */

#endif
