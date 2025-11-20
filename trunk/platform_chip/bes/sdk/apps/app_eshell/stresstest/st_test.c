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

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cmsis_os2.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_uart.h"
#include "hal_sysfreq.h"
#include "app_utils.h"

#include "st_test.h"


/********************* all cases ************************/

#define TEST_CASE_MAX_COUNT  12

struct st_test_case *st_cases[TEST_CASE_MAX_COUNT];
static uint8_t st_count = 0;

int st_test_case_register(struct st_test_case *st)
{
    if (st_count >= TEST_CASE_MAX_COUNT)
        return -1;

    st_cases[st_count] = st;
    st_count++;
    return 0;
}

void st_cases_clear(void)
{
    for (uint8_t i = 0; i < TEST_CASE_MAX_COUNT; i++)
        st_cases[i] = NULL;
    st_count = 0;
}

void st_cases_init(void)
{
    for (uint8_t i = 0; i < st_count; i++) {
        if (st_cases[i] && st_cases[i]->init) {
            st_cases[i]->init(st_cases[i]);
        }
    }
}

void st_cases_exit(void)
{
    for (uint8_t i = 0; i < st_count; i++) {
        if (st_cases[i] && st_cases[i]->exit) {
            st_cases[i]->exit(st_cases[i]);
        }
    }
}

void st_cases_run_once(void)
{
    for (uint8_t i = 0; i < st_count; i++) {
        if (st_cases[i] && st_cases[i]->run_once) {
            st_cases[i]->run_once(st_cases[i]);
        }
    }
}

void st_cases_loop_start(void)
{
    for (uint8_t i = 0; i < st_count; i++) {
        if (st_cases[i] && st_cases[i]->loop_start) {
            st_cases[i]->loop_start(st_cases[i]);
        }
    }
}

void st_cases_loop_stop(void)
{
    for (uint8_t i = 0; i < st_count; i++) {
        if (st_cases[i] && st_cases[i]->loop_stop) {
            st_cases[i]->loop_stop(st_cases[i]);
        }
    }
}

int st_cases_check_errors(void)
{
    int errs = 0;
    for (uint8_t i = 0; i < st_count; i++) {
        if (st_cases[i] && st_cases[i]->check_errors) {
            errs += st_cases[i]->check_errors(st_cases[i]);
        }
    }
    ST_ERR("ST Cases found %d errors.", errs);
    return errs;
}

int st_cases_func_test(void)
{
    int errs = 0;
    for (uint8_t i = 0; i < st_count; i++) {
        if (st_cases[i] && st_cases[i]->func_test) {
            if (st_cases[i]->func_test(st_cases[i]) != 0)
                errs++;
        }
    }
    ST_ERR("ST Cases function test %d cases failed.", errs);
    return errs;
}


/********************* common lock *********************************/

static osMutexId_t st_mutex_id = NULL;
void st_test_lock(void)
{
    if (st_mutex_id == NULL)
        st_mutex_id = osMutexNew(NULL);

    if (st_mutex_id != NULL)
        osMutexAcquire(st_mutex_id, osWaitForever);
}

void st_test_unlock(void)
{
    if (st_mutex_id != NULL)
        osMutexRelease(st_mutex_id);
}

/*********************** sysfreq control *****************************/

void st_sysfreq_high(void)
{
#ifdef CHIP_SUBSYS_SENS
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_104M);
#elif defined(CHIP_SUBSYS_BTH)
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_104M);
#else
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
#endif
}

void st_sysfreq_mid(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_104M);
}

void st_sysfreq_low(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_26M);
}

void st_sysfreq_sleep(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_32K);
    hal_sysfreq_req(APP_SYSFREQ_USER_ESHELL, HAL_CMU_FREQ_32K);
}

/******************** test case common functions **************************/

void st_case_init(struct st_test_case *st_case)
{
    osThreadAttr_t attr;

    if (!st_case)
        return;

    if (st_case->running)
        return;

    st_case->loop_enable = false;
    st_case->test_enable = false;
    st_case->test_errs = 0;

    st_case->mutex = osMutexNew(NULL);

    memset(&attr, 0, sizeof(osThreadAttr_t));
    attr.name = st_case->name;
    attr.attr_bits = osThreadDetached;
    attr.stack_size = st_case->thread_stack_size;
    attr.priority = st_case->thread_priority;

    st_case->running = true;
    st_case->thread = osThreadNew(st_case->thread_func, st_case, &attr);

    ST_LOG("%s test init.", st_case->name);
}

void st_case_exit(struct st_test_case *st_case)
{
    if (!st_case)
        return;
    if (!st_case->running)
        return;

    st_case->running = false;
    osDelay(100);
    if (osThreadGetState(st_case->thread) == osThreadRunning)
        osThreadTerminate(st_case->thread);
    st_case->thread = NULL;

    ST_LOG("%s test exit.", st_case->name);

    st_case->test_errs = 0;
    st_case->loop_enable = false;
    st_case->test_enable = false;

    if (st_case->mutex)
        osMutexDelete(&st_case->mutex);
    st_case->mutex = NULL;
}

void st_case_run_once(struct st_test_case *st_case)
{
    if (!st_case)
        return;
    if (!st_case->running)
        return;
    if (st_case->loop_enable)
        return;

    if (st_case->mutex)
        osMutexAcquire(st_case->mutex, osWaitForever);

    st_case->test_enable = true;

    if (st_case->mutex)
        osMutexRelease(st_case->mutex);

    while (st_case->test_enable)
        osDelay(1);
    ST_LOG("%s test run once.", st_case->name);
}

void st_case_loop_start(struct st_test_case *st_case)
{
    if (!st_case)
        return;
    if (!st_case->running)
        return;

    if (st_case->mutex)
        osMutexAcquire(st_case->mutex, osWaitForever);

    st_case->loop_enable = true;
    st_case->test_enable = true;
    ST_LOG("%s test loop start.", st_case->name);

    if (st_case->mutex)
        osMutexRelease(st_case->mutex);
}

void st_case_loop_stop(struct st_test_case *st_case)
{
    if (!st_case)
        return;
    if (!st_case->running)
        return;

    if (st_case->mutex)
        osMutexAcquire(st_case->mutex, osWaitForever);

    st_case->loop_enable = false;

    if (st_case->mutex)
        osMutexRelease(st_case->mutex);

    while (st_case->test_enable)
        osDelay(1);
    ST_LOG("%s test loop stop.", st_case->name);
}

int st_case_check_errors(struct st_test_case *st_case)
{
    if (!st_case)
        return 0;
    ST_LOG("%s test found %u errors.",
                st_case->name,
                st_case->test_errs);
    return st_case->test_errs;
}

#endif
