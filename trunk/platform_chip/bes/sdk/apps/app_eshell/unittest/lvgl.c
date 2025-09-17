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

#if defined(UTILS_ESHELL_EN) && defined(ENABLE_LVGL)
#include "stdlib.h"
#include "string.h"
#include "cmsis_os.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "lvgl.h"
#include "lv_port_disp.h"
#include "lv_port_indev.h"
#include "bes_tp_hal.h"
#include "bes_demo_manager.h"
#include "app_utils.h"
#include "ut_test.h"

#if LV_USE_LOG && !LV_LOG_PRINTF
static void print_handler(const char *buf)
{
    hal_trace_output((const unsigned char *)buf, strlen(buf));
}
#endif

#if LV_MEM_CUSTOM == 1
#include "heap_api.h"
uint8_t *gBesLvglMemHandle = NULL;
static uint8_t gBesLvglMemPoll[LV_MEM_CUSTOM_SIZE];

void lv_custom_mem_init(void)
{
    gBesLvglMemHandle = heap_register(gBesLvglMemPoll, LV_MEM_CUSTOM_SIZE);
}
#endif

#ifndef UI_THREAD_STACK_SIZE
#define UI_THREAD_STACK_SIZE 3072*2
#endif

static void ui_thread(void const *argument);
osThreadDef(ui_thread, osPriorityHigh, 1, UI_THREAD_STACK_SIZE, "ui_thread");
osThreadId ui_thread_tid;

static void ui_thread(void const *argument)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_21, HAL_CMU_FREQ_208M);
    while(1){
        uint32_t time_till_next;
        time_till_next = lv_task_handler();
        if(time_till_next) {
            osDelay(time_till_next);
        }
    }
}

extern void lv_demo_benchmark(void);
extern void lv_demo_stress(void);
extern void lv_demo_widgets(void);
extern void bes_example_img_argb(void);
extern void bes_example_img_index4(void);
extern void bes_example_img_index8(void);
extern void bes_example_img_etc2(void);

static void lvgl_init(void)
{
#if LV_MEM_CUSTOM == 1
        lv_custom_mem_init();
#endif
    
        lv_init();
        lv_port_disp_init();
        bes_tp_hal_init();
        lv_port_indev_init();
    
        TRACE(1, "Welcome to LVGL\r\n");
        
#if LV_USE_LOG && !LV_LOG_PRINTF
            lv_log_register_print_cb(print_handler);
#endif
}

static void lvgl_demo(void)
{
#ifdef LVGL_BES_DEMO
        bes_demo_entry();
#else
        lv_demo_benchmark();
#endif
}

static void test_lvgl(void)
{
    lvgl_init();
    lvgl_demo();
    ui_thread_tid = osThreadCreate(osThread(ui_thread), NULL);
        if (ui_thread_tid == NULL)  {
            TRACE(0,"Failed to Create ui_thread\n");
            return;
        }
}

static void unitest_lvgl_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  utest_lvgl \r\n");
}

static void unitest_lvgl(int argc, char *argv[])
{
    if (argc != 1)
        goto usage;

    if (strncmp(argv[0], "utest_lvgl", 10) == 0) {
        test_lvgl();
    } else {
        goto usage;
    }

    return;

usage:
    unitest_lvgl_usage();
}

ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST, "utest_lvgl", "usage: utest_lvgl help",
                   unitest_lvgl);
#endif
