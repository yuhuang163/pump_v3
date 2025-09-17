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

#if defined(UTILS_ESHELL_EN) && defined(__SYS_AS_MAIN__) && defined(PSRAM_ENABLE)
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdarg.h>
#include "cmsis_os2.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_sysfreq.h"
#include "hal_location.h"
#include "gpu_conf.h"

#include "test_colors.h"
#include "lcdc_test.h"
#include "gpu_test.h"

#include "st_test.h"

static uint8_t *frame_buf0 = NULL;
static uint8_t *frame_buf1 = NULL;

static bool gpu_display_inited = false;

static void gpu_test_start(void)
{
    if (gpu_display_inited)
        return;

    frame_buf0 = frame_test_nc_buf0;
    frame_buf1 = frame_test_nc_buf1;

    st_test_lock();

    lcdc_test_init();
    lcdc_test_layer_init(0, frame_buf0);

    if (lcdc_test_get_bpp() == 16)
        gpu_test_open(lcdc_test_get_width(), lcdc_test_get_height(), VG_LITE_RGB565, NULL);
    else
        gpu_test_open(lcdc_test_get_width(), lcdc_test_get_height(), VG_LITE_RGBA8888, NULL);

    lcdc_test_start();

    gpu_display_inited = true;

    st_test_unlock();
}

static void gpu_test_stop(void)
{
    if (!gpu_display_inited)
        return;

    st_test_lock();

    lcdc_test_stop();
    lcdc_test_exit();
    gpu_test_close();

    gpu_display_inited = false;

    st_test_unlock();
}

static int gpu_function_test(struct st_test_case *st_case)
{
    int ret = 0;
    void *buf = frame_buf0;
    uint32_t colors[4] = {0xffffffff, 0xffff0000, 0xff00ff00, 0xff0000ff};
    int lcdc_w = lcdc_test_get_width();
    int lcdc_h = lcdc_test_get_height();

    /* test thread running, cancel function test */
    if (st_case->running)
        return 0;

    gpu_test_start();

    lcdc_test_buffer_set(0, buf);
    gpu_test_set_buf(buf);

    /* fill color test */
    for (int i = 0; i < 4; i++) {
        if (gpu_test_fill_color(colors[i%4]))
            ret++;
        lcdc_test_frame_xfer();
        lcdc_test_frame_wait_done();
        osDelay(500);
    }

    /* draw rect test */
    for (int i = 0; i < 60; i++) {
        uint32_t rotate = i * 2;
        gpu_test_fill_color(0xffff0000);
        if (gpu_test_draw_rect((lcdc_w-300)/2, (lcdc_h-300)/2, 300, 300, rotate, 0xff00cccc))
            ret++;
        lcdc_test_frame_xfer();
        lcdc_test_frame_wait_done();
        osDelay(15);
    }

    /* draw circle test */
    for (int i = 0; i < 60; i++) {
        float scale = 1.0f + (float)i * 0.05f;
        gpu_test_fill_color(0xffff0000);
        if (gpu_test_draw_arc_circle(lcdc_w/2, lcdc_h/2, (int)(scale*50.0f), 0, 360, 0xff00cccc))
            ret++;
        lcdc_test_frame_xfer();
        lcdc_test_frame_wait_done();
        osDelay(15);
    }

    /* draw tiger test */
    for (int i = 0; i < 60; i++) {
        float scale = 1.0f + (float)i * 0.05f;
        uint32_t rotate = i * 5;
        gpu_test_fill_color(0xff0000ff);
        if (gpu_test_draw_tiger(lcdc_w/2, lcdc_h/2, scale, rotate))
            ret++;
        lcdc_test_frame_xfer();
        lcdc_test_frame_wait_done();
        osDelay(15);
    }

    /* draw pacman test */
    for (int i = 0; i < 60; i++) {
        gpu_test_fill_color(0xffff0000);
        if (gpu_test_draw_pacman(i, lcdc_w/2, lcdc_h/2, 0, 0xff00cccc))
            ret++;
        lcdc_test_frame_xfer();
        lcdc_test_frame_wait_done();
        osDelay(15);
    }

    gpu_test_stop();

    if (ret != 0)
        st_case->test_errs++;
    ST_LOG("%s function test %s.", st_case->name, ret ? "FAILED" : "OKAY");
    return ret;
}

static void gpu_test_thread(void *data)
{
    struct st_test_case *st_case = (struct st_test_case *)data;
    void *buf = frame_buf0;
    uint32_t frame_start, frame_ms;
    uint32_t frames = 0;
    float scale = 2.0;
    float step = 0.1;
    uint32_t rotate = 0;
    int errs = 0;
    int lcdc_w = lcdc_test_get_width();
    int lcdc_h = lcdc_test_get_height();

    gpu_test_start();

    while (st_case->running) {
        if (st_case->test_enable) {
            if (gpu_display_inited) {
                ST_LOG("---------------------------------------------------------");
                ST_LOG("#### Frame %u start.", frames);
                frame_start = hal_sys_timer_get();

                st_test_lock();

                lcdc_test_frame_xfer();

                if (frames % 2 == 0)
                    buf = frame_buf1;
                else
                    buf = frame_buf0;

                gpu_test_set_buf(buf);
                gpu_test_fill_color(0xff0000ff);
                errs = gpu_test_draw_tiger(lcdc_w/2, lcdc_h/2, scale, rotate);
                if (errs != 0)
                    st_case->test_errs++;

                if (scale >= 3.0 || scale <= 1.0)
                    step = -step;
                scale += step;
                rotate += 6;
                if (rotate > 360)
                    rotate = rotate % 360;

                lcdc_test_frame_wait_done();
                lcdc_test_buffer_set(0, buf);

                st_test_unlock();

                frame_ms = TICKS_TO_MS(hal_sys_timer_get() - frame_start);
                ST_LOG("#### Frame %u used %dms, dynamic fps %u",
                        frames, frame_ms, 1000/frame_ms);
                frames++;
            } else {
                osDelay(50);
            }
            if (st_case->mutex)
                osMutexAcquire(st_case->mutex, osWaitForever);
            if (!st_case->loop_enable)
                st_case->test_enable = false;
            if (st_case->mutex)
                osMutexRelease(st_case->mutex);

        } else {
            osDelay(10);
        }
    }

    gpu_test_stop();
    osThreadExit();
}

struct st_test_case gpu_case = {
    .name = "GPU",

    .thread = NULL,
    .thread_func = gpu_test_thread,
    .thread_priority = osPriorityHigh,
    .thread_stack_size = 3072,

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

    .func_test = gpu_function_test,
};

void st_test_gpu_register(void)
{
    st_test_case_register(&gpu_case);
}

void gpu_test_display_on(void)
{
    if (!gpu_case.running) {
        ST_ERR("GPU test is not initialized!");
        return;
    }
    gpu_test_start();
}

void gpu_test_display_off(void)
{
    if (!gpu_case.running) {
        ST_ERR("GPU test is not initialized!");
        return;
    }
    gpu_test_stop();
}

/************ eshell command *******************/

static void st_gpu_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  st_gpu init, start test thread\r\n");
    eshell_putstring("  st_gpu exit, stop test thread\r\n");
    eshell_putstring("  st_gpu run_once, run test one time\r\n");
    eshell_putstring("  st_gpu start, start test loop\r\n");
    eshell_putstring("  st_gpu stop, stop test loop\r\n");
    eshell_putstring("  st_gpu display on|off, turn on or off display\r\n");
}

static void st_gpu_cmd(int argc, char *argv[])
{
    if (argc < 2)
        goto usage;

    if (strncmp(argv[1], "init", 4) == 0)
        st_case_init(&gpu_case);
    else if (strncmp(argv[1], "exit", 4) == 0)
        st_case_exit(&gpu_case);
    else if (strncmp(argv[1], "run_once", 8) == 0)
        st_case_run_once(&gpu_case);
    else if (strncmp(argv[1], "start", 5) == 0)
        st_case_loop_start(&gpu_case);
    else if (strncmp(argv[1], "stop", 4) == 0)
        st_case_loop_stop(&gpu_case);
    else if (strncmp(argv[1], "display", 7) == 0) {
        if (argc < 3)
            goto usage;
        if (strncmp(argv[2], "on", 2) == 0)
            gpu_test_display_on();
        else if (strncmp(argv[2], "off", 3) == 0)
            gpu_test_display_off();
        else
            goto usage;
    } else
        goto usage;

    return;
usage:
    st_gpu_usage();
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST,
                "st_gpu",
                "usage: st_gpu help",
                st_gpu_cmd);

#endif
