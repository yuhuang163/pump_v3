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

#if defined(UTILS_ESHELL_EN)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "cmsis_os2.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_i2c.h"

#include "st_test.h"

#define TEST_I2C_DEBUG

#define I2C_TEST_SIZE      8

static uint8_t master_tx_buff[I2C_TEST_SIZE];
static uint8_t master_rx_buff[I2C_TEST_SIZE];
static uint8_t slave_tx_buff[I2C_TEST_SIZE];
static uint8_t slave_rx_buff[I2C_TEST_SIZE];
#define TEST_I2C_SLAVE_ADDR     0x66

static bool i2c_opened = false;
static uint32_t test_err = 0;

POSSIBLY_UNUSED
static int i2c_test_open(void)
{
    int ret;

    struct HAL_I2C_CONFIG_T i2c_cfg_master;
    memset(&i2c_cfg_master, 0, sizeof(i2c_cfg_master));
    i2c_cfg_master.mode = HAL_I2C_API_MODE_SIMPLE;
    i2c_cfg_master.speed = 400000;
    i2c_cfg_master.as_master = 1;

    struct HAL_I2C_CONFIG_T i2c_cfg_slave;
    memset(&i2c_cfg_slave, 0, sizeof(i2c_cfg_slave));
    i2c_cfg_slave.mode = HAL_I2C_API_MODE_SIMPLE;
    i2c_cfg_slave.speed = 400000;
    i2c_cfg_slave.as_master = 0;
    i2c_cfg_slave.use_sync = 1;
    i2c_cfg_slave.addr_as_slave = TEST_I2C_SLAVE_ADDR;

    ret = hal_i2c_open(HAL_I2C_ID_0, &i2c_cfg_master);
    if (ret) {
        ST_ERR("I2C0 bus open failed.");
        return ret;
    } else {
        ST_LOG("I2C0 bus open okay.");
    }
    hal_iomux_set_i2c0();
    ret = hal_i2c_open(HAL_I2C_ID_1, &i2c_cfg_slave);
    if (ret) {
        hal_i2c_close(HAL_I2C_ID_0);
        ST_ERR("I2C1 bus open failed.");
        return ret;
    } else {
        ST_LOG("I2C1 bus open okay.");
    }
    hal_iomux_set_i2c1();
#ifdef TEST_I2C_DEBUG
    ST_LOG("I2C open %s", ret ? "failed" : "okay");
#endif
    if (!ret)
        i2c_opened = true;
    return ret;
}

POSSIBLY_UNUSED
static int i2c_test_close(void)
{
    int ret = 0;
    ret = hal_i2c_close(HAL_I2C_ID_0);
#ifdef TEST_I2C_DEBUG
    ST_LOG("I2C0 close %s", ret ? "failed" : "okay");
#endif
    ret = hal_i2c_close(HAL_I2C_ID_1);
#ifdef TEST_I2C_DEBUG
    ST_LOG("I2C1 close %s", ret ? "failed" : "okay");
#endif
    i2c_opened = false;
    return ret;
}

static void test_i2c_slave(void *arg)
{
    uint32_t ret = 0;

    if (!i2c_opened)
        return;
    for (uint32_t i = 0; i < sizeof(master_tx_buff); i++){
        slave_tx_buff[i] = rand() & 0xff;
    }
    memset(&slave_rx_buff, 0, sizeof(slave_rx_buff));

    ret = hal_i2c_slv_simple_recv(HAL_I2C_ID_1, slave_rx_buff, sizeof(slave_rx_buff)-1, 1);
    if (ret){
        ST_LOG("i2c1 simple_recv failed:%d", ret);
        osThreadExit();
        return;
    }
    ret = hal_i2c_slv_simple_send(HAL_I2C_ID_1, slave_tx_buff, sizeof(slave_tx_buff), 1);
    if (ret){
        ST_LOG("i2c1 simple_recv failed:%d", ret);
        osThreadExit();
        return;
    }
    for(uint32_t i = 0; i < sizeof(slave_rx_buff)-1; i++){
        if(master_tx_buff[i] != slave_rx_buff[i]){
            ST_LOG("I2C_TEST SLAVE ERR %u:recv  %#x,send  %#x",
                    i, master_tx_buff[i], slave_rx_buff[i]);
            osThreadExit();
            return;
        }
    }
#ifdef TEST_I2C_DEBUG
    ST_LOG("SLAVE I2C1 TEST OKAY:  %d bytes", I2C_TEST_SIZE);
#endif
    osThreadExit();
}

static void test_i2c_master(void *arg)
{
    uint32_t ret = 0;

    if (!i2c_opened)
        return;

    for (uint32_t i = 0; i < sizeof(master_tx_buff); i++){
        master_tx_buff[i] = rand() & 0xff;
    }
    memset(&master_rx_buff, 0, sizeof(master_rx_buff));

    ret = hal_i2c_simple_recv(HAL_I2C_ID_0, TEST_I2C_SLAVE_ADDR, master_tx_buff, sizeof(master_tx_buff),
                        master_rx_buff, sizeof(master_rx_buff));
    if (ret){
        ST_LOG("i2c0_mst_read failed:%d", ret);
        osThreadExit();
        return;
    }
    for(uint32_t i = 0; i < sizeof(master_rx_buff); i++){
        if(master_rx_buff[i] != slave_tx_buff[i]){
            ST_LOG("I2C_TEST MASTER ERR %u:recv  %#x,send  %#x",
                    i, master_rx_buff[i], slave_tx_buff[i]);
            osThreadExit();
            return;
        }
    }
#ifdef TEST_I2C_DEBUG
    ST_LOG("MASTER I2C0 TEST OKAY:  %d bytes", I2C_TEST_SIZE);
#endif
    osThreadExit();
}

static int i2c_loopback_thread_create(void)
{
    if (!i2c_opened)
        return 1;

    osThreadAttr_t threadattr_test_i2c_slave;
    osThreadAttr_t threadattr_test_i2c_master;

    memset(&threadattr_test_i2c_slave, 0, sizeof(osThreadAttr_t));
    threadattr_test_i2c_slave.name = "test_i2c_slave";
    threadattr_test_i2c_slave.attr_bits = osThreadDetached;
    threadattr_test_i2c_slave.stack_size = 1024;
    threadattr_test_i2c_slave.priority = osPriorityRealtime;
    osThreadNew((osThreadFunc_t)test_i2c_slave, NULL, &threadattr_test_i2c_slave);

    memset(&threadattr_test_i2c_master, 0, sizeof(osThreadAttr_t));
    threadattr_test_i2c_master.name = "test_i2c_master";
    threadattr_test_i2c_master.attr_bits = osThreadDetached;
    threadattr_test_i2c_master.stack_size = 1024;
    threadattr_test_i2c_master.priority = osPriorityRealtime;
    osThreadNew((osThreadFunc_t)test_i2c_master, NULL, &threadattr_test_i2c_master);

    return 0;
}

static int i2c_function_test(struct st_test_case *st_case)
{
    int ret = 0;

    /* test thread running, cancel function test */
    if (st_case->running)
        return 0;

    i2c_test_open();
    for (int i = 0; i < 50; i++)
        i2c_loopback_thread_create();

    i2c_test_close();
    st_case->test_errs += test_err;
    ST_LOG("%s function test %s.", st_case->name, st_case->test_errs ? "FAILED" : "OKAY");
    return ret;
}

static void i2c_test_thread(void *data)
{
    struct st_test_case *st_case = (struct st_test_case *)data;
    i2c_test_open();
    while (st_case->running) {
        if (st_case->test_enable) {
            i2c_loopback_thread_create();

            if (st_case->mutex)
                osMutexAcquire(st_case->mutex, osWaitForever);
            if (!st_case->loop_enable)
                st_case->test_enable = false;
            if (st_case->mutex)
                osMutexRelease(st_case->mutex);
            st_case->test_errs+=test_err;
        }
        osDelay(500);
    }
    i2c_test_close();
    osThreadExit();
}

struct st_test_case i2c_case = {
    .name = "I2C",

    .thread = NULL,
    .thread_func = i2c_test_thread,
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

    .func_test = i2c_function_test,
};

void st_test_i2c_register(void)
{
    st_test_case_register(&i2c_case);
}

/************ eshell command *******************/

static void st_i2c_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  st_i2c init, initialize I2C and start test thread\r\n");
    eshell_putstring("  st_i2c exit, close I2C and stop test thread\r\n");
    eshell_putstring("  st_i2c run_once, run test one time\r\n");
    eshell_putstring("  st_i2c start, start test loop\r\n");
    eshell_putstring("  st_i2c stop, stop test loop\r\n");
}

static void st_i2c_cmd(int argc, char *argv[])
{
    if (argc < 2)
        goto usage;

    if (strncmp(argv[1], "init", 4) == 0)
        st_case_init(&i2c_case);
    else if (strncmp(argv[1], "exit", 4) == 0)
        st_case_exit(&i2c_case);
    else if (strncmp(argv[1], "run_once", 8) == 0)
        st_case_run_once(&i2c_case);
    else if (strncmp(argv[1], "start", 5) == 0)
        st_case_loop_start(&i2c_case);
    else if (strncmp(argv[1], "stop", 4) == 0)
        st_case_loop_stop(&i2c_case);
    else
        goto usage;

    return;
usage:
    st_i2c_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST,
                "st_i2c",
                "usage: st_i2c help",
                st_i2c_cmd);

#endif
