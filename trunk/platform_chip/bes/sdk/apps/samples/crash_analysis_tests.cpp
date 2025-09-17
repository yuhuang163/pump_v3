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

#include "crash_analysis_tests.h"

#include "hal_trace.h"
#include <stdio.h>
#include "cmsis_os2.h"
#include "cmsis.h"
#include <cstring>
#include "hal_timer.h"

#define GET_CURRENT_TICKS()     hal_sys_timer_get()
#define GET_CURRENT_MS()        TICKS_TO_MS(GET_CURRENT_TICKS())
#define TEST_THREAD_STACKSIZE   1024U

static void deadlock_test_timer_cb(void *arg);
static void thread_test(void *arg);

static osThreadId_t thread_test_id = NULL;
static osTimerId_t deadlock_timer_id = NULL;
static osMutexId_t deadlock_mutex1_id = NULL;
static osMutexId_t deadlock_mutex2_id = NULL;

static const osThreadAttr_t test_thread_attr = {
    .name = "test",
    .attr_bits = osThreadDetached,
    .cb_mem = NULL,
    .cb_size = 0U,
    .stack_size = TEST_THREAD_STACKSIZE,
    .priority = osPriorityHigh,
    .tz_module = 1U,            // indicate calls to secure mode
    .reserved = 0U,
};

void data_access_error_test(char* cmdParam, uint32_t cmdParam_len)
{
    uint32_t *test_ptr = (uint32_t *)0x0000;
    *test_ptr = 0x1;
    ASSERT(false, "%s", __func__);
}

void instruction_bus_error_test(char *cmdParam, uint32_t cmdParam_len)
{
    int value = 0x4000000;

    if (cmdParam != NULL)
    {
        sscanf(cmdParam, "%x", &value);
    }

    void (*bad_ptr)(void) = (void (*)(void))value;
    bad_ptr();
    ASSERT(false, "%s", __func__);
}

void data_bus_error_test(char *cmdParam, uint32_t cmdParam_len)
{
    int value = 0x4000000;

    if (cmdParam != NULL)
    {
        sscanf(cmdParam, "%x", &value);
    }

    uint32_t *bad_ptr = (uint32_t *)value;
    *bad_ptr = 0x1;
    ASSERT(false, "%s", __func__);
}

static void test_func(uint8_t cnt)
{}

void usage_fault_test(char *cmdParam, uint32_t cmdParam_len)
{
    void (*bad_ptr)(void) = (void (*)())((uint32_t)test_func + 1);
    bad_ptr();
    ASSERT(false, "%s", __func__);
}

void hard_fault_test(char *cmdParam, uint32_t cmdParam_len)
{
    uint32_t pri = int_lock_global();

    void (*bad_ptr)(void) = (void (*)())((uint32_t)test_func+1);
    bad_ptr();
    ASSERT(false, "%s", __func__);

    int_unlock_global(pri);
}

static void deadlock_test_timer_cb(void *arg)
{
    osMutexAcquire(deadlock_mutex1_id, osWaitForever);
    osDelay(100);
    TRACE(0, "%s try to get deadlock mutex2", __func__);
    osMutexAcquire(deadlock_mutex2_id, osWaitForever);
    TRACE(0, "%s got deadlock mutex2", __func__);

    osMutexRelease(deadlock_mutex1_id);
    osMutexRelease(deadlock_mutex2_id);
}

static void thread_test(void *argument)
{
    while (1)
    {
        osMutexAcquire(deadlock_mutex2_id, osWaitForever);
        osThreadYield();
        osMutexAcquire(deadlock_mutex1_id, osWaitForever);

        osMutexRelease(deadlock_mutex2_id);
        osMutexRelease(deadlock_mutex1_id);
    }
}

void dead_lock_test(char *cmdParam, uint32_t cmdParam_len)
{
    if (NULL == deadlock_timer_id)
    {
        TRACE(0, "before create timer");
        deadlock_timer_id = osTimerNew(deadlock_test_timer_cb, osTimerOnce, NULL, NULL);
        if (NULL == deadlock_timer_id)
        {
            TRACE(0, "Warning: failed to Create deadlock_timer");
        }
        else
        {
            osTimerStart(deadlock_timer_id, 1000);
        }
    }
    else
    {
        TRACE(0, "deadlock_timer has been Created");
    }

    if (NULL == deadlock_mutex1_id)
    {
        deadlock_mutex1_id = osMutexNew(NULL);
        if (NULL == deadlock_mutex1_id)
        {
            TRACE(0, "Warning: failed to Create deadlock_mutex1");
        }
    }
    else
    {
        TRACE(0, "deadlock_mutex1 has been Created");
    }

    if (NULL == deadlock_mutex2_id)
    {
        deadlock_mutex2_id = osMutexNew(NULL);
        if (NULL == deadlock_mutex2_id)
        {
            TRACE(0, "Warning: failed to Create deadlock_mutex2");
        }
    }
    else
    {
        TRACE(0, "deadlock_mutex2 has been Created");
    }

    if (NULL == thread_test_id)
    {
        thread_test_id = osThreadNew(thread_test, NULL, &test_thread_attr);
        if (NULL == thread_test_id)
        {
            TRACE(0, "Warning: failed to Create thread_test");
        }
    }
    else
    {
        TRACE(0, "thread_test has been Created");
    }
}

void stack_overflow_test(char *cmdParam, uint32_t cmdParam_len)
{
    uint8_t test_buf[10];
    uint8_t test_cnt = GET_CURRENT_MS();
    memset(test_buf, 0xbb, test_cnt);
    DUMP8("%02x ", test_buf, test_cnt);
}
