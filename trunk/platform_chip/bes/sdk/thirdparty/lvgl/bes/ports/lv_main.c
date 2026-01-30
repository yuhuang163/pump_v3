/***************************************************************************
 *
 * Copyright 2015-2024 BES.
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

#include <string.h>
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_sysfreq.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "lvgl.h"
#include "lv_demos.h"
#include "lv_examples.h"
#include "lv_hal_disp.h"
#include "lv_hal_indev.h"
#include "src/osal/lv_os_private.h"
#include "lv_main.h"

#define TIME_SCALE   1

static uint32_t hal_tick_get(void)
{
    return TICKS_TO_MS(hal_sys_timer_get() / TIME_SCALE);
}

static void hal_delay_ms(uint32_t ms)
{
#ifdef RTOS
    osDelay(ms * TIME_SCALE);
#else
    hal_sys_timer_delay(MS_TO_TICKS(ms * TIME_SCALE));
#endif
}

#if LV_USE_LOG && !LV_LOG_PRINTF
static void print_cb(lv_log_level_t level, const char *buf)
{
    hal_trace_output((const unsigned char *)buf, strlen(buf));
}
#endif

static lv_obj_t * demo_list;
static char demo_name[100];

static void demo_event_handler(lv_event_t * e)
{
    lv_event_code_t code = lv_event_get_code(e);
    lv_obj_t * obj = lv_event_get_target(e);
    if(code == LV_EVENT_CLICKED) {
        strcpy(demo_name, lv_list_get_button_text(demo_list, obj));
        LV_LOG_USER("Run demo: %s", demo_name);

        char *p[1] = { demo_name };
        lv_demos_create(p, 1);
    }
}

POSSIBLY_UNUSED
static void lv_demo_list(void)
{
    demo_list = lv_list_create(lv_screen_active());
    lv_obj_set_size(demo_list, 240, 320);
    lv_obj_center(demo_list);

    lv_obj_t *btn, *title;

    title = lv_list_add_text(demo_list, "DEMOS");
    lv_obj_set_style_text_font(title, &lv_font_montserrat_24, 0);

    btn = lv_list_add_button(demo_list, LV_SYMBOL_DUMMY, "benchmark");
    lv_obj_set_style_text_font(btn, &lv_font_montserrat_20, 0);
    lv_obj_add_event_cb(btn, demo_event_handler, LV_EVENT_CLICKED, NULL);

    btn = lv_list_add_button(demo_list, LV_SYMBOL_DUMMY, "stress");
    lv_obj_set_style_text_font(btn, &lv_font_montserrat_20, 0);
    lv_obj_add_event_cb(btn, demo_event_handler, LV_EVENT_CLICKED, NULL);

    btn = lv_list_add_button(demo_list, LV_SYMBOL_DUMMY, "widgets");
    lv_obj_set_style_text_font(btn, &lv_font_montserrat_20, 0);
    lv_obj_add_event_cb(btn, demo_event_handler, LV_EVENT_CLICKED, NULL);

#if LV_USE_DEMO_MUSIC
    btn = lv_list_add_button(demo_list, LV_SYMBOL_DUMMY, "music");
    lv_obj_set_style_text_font(btn, &lv_font_montserrat_20, 0);
    lv_obj_add_event_cb(btn, demo_event_handler, LV_EVENT_CLICKED, NULL);
#endif

#if LV_USE_DEMO_FLEX_LAYOUT
    btn = lv_list_add_button(demo_list, LV_SYMBOL_DUMMY, "flex_layout");
    lv_obj_set_style_text_font(btn, &lv_font_montserrat_20, 0);
    lv_obj_add_event_cb(btn, demo_event_handler, LV_EVENT_CLICKED, NULL);
#endif

#if LV_USE_DEMO_MULTILANG
    btn = lv_list_add_button(demo_list, LV_SYMBOL_DUMMY, "multilang");
    lv_obj_set_style_text_font(btn, &lv_font_montserrat_20, 0);
    lv_obj_add_event_cb(btn, demo_event_handler, LV_EVENT_CLICKED, NULL);
#endif

#if LV_USE_DEMO_TRANSFORM
    btn = lv_list_add_button(demo_list, LV_SYMBOL_DUMMY, "transform");
    lv_obj_set_style_text_font(btn, &lv_font_montserrat_20, 0);
    lv_obj_add_event_cb(btn, demo_event_handler, LV_EVENT_CLICKED, NULL);
#endif
}

void lv_example_vector(int w, int h);
static void lv_app_main(void *notused)
{
    lv_init();

#if LV_USE_LOG && !LV_LOG_PRINTF
    lv_log_register_print_cb(print_cb);
#endif

    lv_tick_set_cb(hal_tick_get);
    lv_delay_set_cb(hal_delay_ms);

    lv_hal_disp_init();
#ifndef LVGL_NO_TOUCHSCREEN
    lv_hal_indev_init();
#endif

#ifdef LVGL_RUN_DEMO_LIST
    lv_demo_list();
#elif defined(LVGL_RUN_DEMO_STRESS)
    lv_demo_stress();
#elif defined(LVGL_RUN_DEMO_WIDGETS)
    lv_demo_widgets();
#elif defined(LVGL_RUN_DEMO_MUSIC)
    lv_demo_music();
#elif defined(LVGL_RUN_DEMO_BENCHMARK8)
    extern void lv_demo_benchmark8(void);
    lv_demo_benchmark8();
#elif defined(LVGL_RUN_DEMO_VECTOR)
    extern void lv_example_vector(int w, int h);
    lv_example_vector(400, 400);
#elif defined(LVGL_RUN_DEMO_FREETYPE)
    extern void lv_example_freetype(void);
    lv_example_freetype();
#elif defined(LVGL_RUN_DEMO_IMAGE)
    extern void lv_example_image(void);
    lv_example_image();
#else
    extern void lv_example_image(void);
    lv_example_image();
    // lv_demo_benchmark();
#endif

    while (1) {
        uint32_t ms_till_next = lv_timer_handler();
#ifdef RTOS
        osDelay(ms_till_next);
#else
        hal_sys_timer_delay(MS_TO_TICKS(ms_till_next));
#endif
    }

    lv_deinit();
}

void lvgl_test(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_1, HAL_CMU_FREQ_208M);
    TRACE(0, "CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));

    TRACE(0, "Welcome to LVGL\r\n");

#ifdef RTOS
#if LV_USE_OS == LV_OS_CUSTOM
    lv_thread_t main_thread;
    lv_thread_init(&main_thread, "main", LV_THREAD_PRIO_HIGHEST, lv_app_main, (32*1024), NULL);
#else
    osThreadAttr_t attr;

    memset(&attr, 0, sizeof(osThreadAttr_t));
    attr.name = "lv_main";
    attr.attr_bits = osThreadDetached;
    attr.stack_size = 24*1024;
    attr.priority = osPriorityRealtime;

    osThreadNew((osThreadFunc_t)lv_app_main, NULL, &attr);
#endif

    while (1) {
        osDelay(5000);
    }
#else
    lv_app_main(NULL);
#endif
}

void lv_main(void)
{
    lvgl_test();
}
