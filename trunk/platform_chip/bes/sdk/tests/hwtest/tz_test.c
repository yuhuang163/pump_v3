/***************************************************************************
 *
 * Copyright 2015-2019 BES.
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
#ifdef TZ_TEST
#include "cmsis.h"
#include "string.h"
#include "hal_trace.h"
#include "cmsis_os2.h"

#include "tz_interface.h"
#include "tz_trace_ns.h"

#include "hal_norflash.h"

volatile int counterA;
volatile int counterB;
volatile int counterC;

static int callbackA (int val)  {
    return (val);
}

static int callbackB (int val)  {
#if defined(SLEEP_TEST) && (SLEEP_TEST)
    return (val+1);
#else
    uint32_t flags;

    flags = osThreadFlagsWait (1U, osFlagsWaitAny, osWaitForever);
    if (flags == 1U)  {
        return (val+1);
    }  else {
        return (0);
    }
#endif
}

#if !(defined(SLEEP_TEST) && (SLEEP_TEST))
static osThreadId_t ThreadA_Id;
static osThreadId_t ThreadB_Id;
static osThreadId_t ThreadC_Id;

void ThreadA (void *argument);
void ThreadB (void *argument);
void ThreadC (void *argument);

__attribute__((noreturn))
void ThreadA (void *argument)  {
    (void)argument;

    for (;;) {
        counterA = func1 (counterA); //counterA+3
        TRACE_IMM(0, "%s, counterA after func1:%d", __func__, counterA);
        counterA = func2 (callbackA, counterA);//callbackA(counterA+1) + 2
        TRACE_IMM(0, "%s, counterA after func2:%d", __func__, counterA);
        osDelay(2U);
        //break;
    }
    while (1);
}

__attribute__((noreturn))
void ThreadB (void *argument)  {
    (void)argument;

    for (;;) {
        counterB = func1 (counterB); //counterA+3
        TRACE_IMM(0, "%s, counterB after func1:%d", __func__, counterB);
        counterB = func2 (callbackB, counterB);//callbackB(counterA+1) + 2
        TRACE_IMM(0, "%s, counterB after func2:%d", __func__, counterB);
        //break;
    }
    while (1);
}

__attribute__((noreturn))
void ThreadC (void *argument) {
    (void)argument;

    for (;;) {
        counterC = counterC + 1;
        if ((counterC % 0x10) == 0)  {
            osThreadFlagsSet (ThreadB_Id, 1);
        }
        osDelay(1U);
    }
}

static const osThreadAttr_t ThreadAttr = {
    .tz_module = 1U,                  // indicate calls to secure mode
};
#endif // !SLEEP_TEST

static int tz_flash_read()
{
    enum HAL_NORFLASH_RET_T ret;
    uint32_t magic_num;
    ret = hal_norflash_read(HAL_FLASH_ID_0, FLASH_BASE, (uint8_t *)(&magic_num), sizeof(magic_num));
    if (ret != HAL_NORFLASH_OK) {
        TRACE(0, "FLASH read error:%d", ret);
        return -1;
    }

    TRACE(0, "FLASH read success. magic_num:0x%x", magic_num);
    return 0;
}

void tz_test (void)
{
    int ret;
    char str[30];

    cmns_trace_init();

    TRACE_IMM(0, "%s, cmns_trace_init done", __func__);

    // Let idle task run
#ifdef SIMU
    osDelay(2);
#else
    osDelay(10);
#endif

    memset(str, 0, sizeof(str));

    ret = get_demo_string(str, sizeof(str));
    TRACE(0, "cmse demo string(ret:%d):%s", ret, str);

    tz_flash_read();

#if defined(SLEEP_TEST) && (SLEEP_TEST)
    TRACE(0, "%s, counterA %d", __func__, counterA);
    counterA = func1 (counterA); //counterA+3
    TRACE_IMM(0, "%s, counterA after func1:%d", __func__, counterA);
    counterA = func2 (callbackA, counterA);//callbackA(counterA+1) + 2
    TRACE_IMM(0, "%s, counterA after func2:%d", __func__, counterA);

    counterB = func1 (counterB); //cnt+3
    TRACE_IMM(0, "%s, counterB after func1:%d", __func__, counterB);
    counterB = func2 (callbackB, counterB);//callbackB(cnt+1) + 2
    TRACE_IMM(0, "%s, counterB after func2:%d", __func__, counterB);
#else
    TRACE(0, "%s, counterA %d", __func__, counterA);
    ThreadA_Id = osThreadNew(ThreadA, NULL, &ThreadAttr);
    ThreadB_Id = osThreadNew(ThreadB, NULL, &ThreadAttr);
    ThreadC_Id = osThreadNew(ThreadC, NULL, NULL);
#endif
}
#endif /*TZ_TEST*/
