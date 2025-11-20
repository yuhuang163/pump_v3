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

#ifndef __ESHELL_ST_TEST_H__
#define __ESHELL_ST_TEST_H__

#include "cmsis_os2.h"

#include "eshell.h"
#include "app_eshell.h"

#define ST_LOG(...)   TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define ST_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)

extern void *frame_test_buf0;
extern void *frame_test_buf1;

#define frame_test_nc_buf0 ((void *)((uint32_t)frame_test_buf0 - PSRAM_BASE + PSRAM_NC_BASE))
#define frame_test_nc_buf1 ((void *)((uint32_t)frame_test_buf1 - PSRAM_BASE + PSRAM_NC_BASE))

struct st_test_case;

struct st_test_case
{
    char name[32];

    /* multi-threads testing */

    osThreadId_t thread;
    osThreadFunc_t thread_func;
    osPriority_t thread_priority;
    int thread_stack_size;

    osMutexId_t mutex;

    bool running;
    bool loop_enable;
    bool test_enable;
    int  test_errs;

    void (*init)(struct st_test_case *st_case);
    void (*exit)(struct st_test_case *st_case);
    void (*run_once)(struct st_test_case *st_case);
    void (*loop_start)(struct st_test_case *st_case);
    void (*loop_stop)(struct st_test_case *st_case);
    int  (*check_errors)(struct st_test_case *st_case);

    /* one-thread function testing */

    int  (*func_test)(struct st_test_case *st_case);
};

int st_test_case_register(struct st_test_case *st);

void st_cases_clear(void);
void st_cases_init(void);
void st_cases_exit(void);
void st_cases_run_once(void);
void st_cases_loop_start(void);
void st_cases_loop_stop(void);
int  st_cases_func_test(void);
int  st_cases_check_errors(void);

void st_test_lock(void);
void st_test_unlock(void);

void st_test_gpu_register(void);
void st_test_blend_register(void);
void st_test_spi_register(void);
void st_test_i2c_register(void);
void st_test_sram_register(void);
void st_test_psram_register(void);
void st_test_fs_register(void);
void st_test_emmc_register(void);
void st_test_efuse_register(void);
void st_test_flash_register(void);
void st_test_uart_register(void);

#if defined(__SYS_AS_MAIN__) && defined(SENSOR_HUB_TEST)
void st_test_sens_register(void);
#endif

void st_sysfreq_high(void);
void st_sysfreq_mid(void);
void st_sysfreq_low(void);
void st_sysfreq_sleep(void);

void st_case_init(struct st_test_case *st_case);
void st_case_exit(struct st_test_case *st_case);
void st_case_run_once(struct st_test_case *st_case);
void st_case_loop_start(struct st_test_case *st_case);
void st_case_loop_stop(struct st_test_case *st_case);
int  st_case_check_errors(struct st_test_case *st_case);

#endif


