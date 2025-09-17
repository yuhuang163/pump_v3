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

#include "RTOS_tests.h"

#include "cmsis_os2.h"
#include "hal_trace.h"
#include "hal_timer.h"

#define FLAGS_MSK               0x00000001U
#define TEST_THREAD_STACKSIZE   1024U

static void testThread(void *argu);
static void senderThread(void *argu);
static void receiverThread(void *argu);
static void demo_timer_once_cb(void *para);
static void demo_timer_periodic_cb(void *para);
static void semaThread(void *argu);

static osEventFlagsId_t demo_evt_id;
static osTimerId_t demo_timer_once_id;
static osTimerId_t demo_timer_periodic_id;
static osSemaphoreId_t demo_semaphore_sid;

static const osThreadAttr_t test_thread_attr = {
    .name = "test",
    .attr_bits = osThreadDetached,
    .cb_mem = NULL,
    .cb_size = 0U,
    .stack_size = TEST_THREAD_STACKSIZE,
    .priority = osPriorityAboveNormal,
    .tz_module = 1U,            // indicate calls to secure mode
    .reserved = 0U,
};

void os_thread_create_demo_test(char* cmdParam, uint32_t cmdParam_len)
{
    osThreadId_t test_tid;

    test_tid = osThreadNew(testThread, NULL, &test_thread_attr);
    TRACE(1, "create testThread: %p\n", test_tid);
}

static void testThread(void *argu)
{
    TRACE(1, "%s %d", __func__, __LINE__);
}

void os_event_demo_test(char* cmdParam, uint32_t cmdParam_len)
{
    osThreadId_t sender_tid;
    osThreadId_t receiver_tid;

    demo_evt_id = osEventFlagsNew(NULL);
    sender_tid = osThreadNew(senderThread, NULL, &test_thread_attr);
    TRACE(1, "create senderThread: %p\n", sender_tid);
    receiver_tid = osThreadNew(receiverThread, NULL, &test_thread_attr);
    TRACE(1, "create receiverThread: %p\n", receiver_tid);
}

static void senderThread(void *argu)
{
    while(1)
    {
        osDelay(1000U);
        osEventFlagsSet(demo_evt_id, FLAGS_MSK);
        TRACE(1, "%s %d", __func__, __LINE__);
        osThreadYield();
    }
}

static void receiverThread(void *argu)
{
    while(1)
    {
        osEventFlagsWait(demo_evt_id, FLAGS_MSK, osFlagsWaitAny, osWaitForever);
        TRACE(1, "%s %d", __func__, __LINE__);
    }
}

void os_timer_demo_test(char* cmdParam, uint32_t cmdParam_len)
{
    TRACE(0, "%s: start osTimerOnce test, current time = %dms ", __func__, GET_CURRENT_MS());
    demo_timer_once_id = osTimerNew(demo_timer_once_cb, osTimerOnce, NULL, NULL);
    if (osOK != osTimerStart(demo_timer_once_id, 1000U))
    {
        TRACE(0, "%s: osTimerStart failed", __func__);
    }

    osDelay(1500U);

    TRACE(0, "%s: start osTimerPeriodic test, current time = %dms", __func__, GET_CURRENT_MS());
    demo_timer_periodic_id = osTimerNew(demo_timer_periodic_cb, osTimerPeriodic, NULL, NULL);
    if (osOK != osTimerStart(demo_timer_periodic_id, 1000U))
    {
        TRACE(0, "%s: osTimerStart failed", __func__);
    }
}

static void demo_timer_once_cb(void *para)
{
    TRACE(1, "%s: current time = %d", __func__, GET_CURRENT_MS());
}

static void demo_timer_periodic_cb(void *para)
{
    TRACE(1, "%s: current time = %d", __func__, GET_CURRENT_MS());
}

void os_semaphore_demo_test(char* cmdParam, uint32_t cmdParam_len)
{
    osThreadId_t semaThread_tid;

    demo_semaphore_sid = osSemaphoreNew(2U, 1U, NULL);
    semaThread_tid = osThreadNew(semaThread, NULL, &test_thread_attr);
    TRACE(1, "create semaThread: %p\n", semaThread_tid);
}

static void semaThread(void *argu)
{
    osStatus_t val;

    while(1)
    {
        val = osSemaphoreAcquire(demo_semaphore_sid, 10U);
        switch(val)
        {
            case osOK:
                TRACE(0, "acquire success!");
                osSemaphoreRelease(demo_semaphore_sid);
                break;
            case osErrorResource:
                TRACE(0, "Error resource!");
                break;
            case osErrorParameter:
                TRACE(0, "Error parameter!");
                break;
            default:
                break;
        }
        TRACE(1, "%s %d", __func__, __LINE__);
        osDelay(500U);
        osThreadYield();
    }
}
