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

#if defined(UTILS_ESHELL_EN) && defined(__SYS_AS_MAIN__) && defined(TEST_WITH_GPU)

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>
#include "hal_uart.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "hal_location.h"
#include "hal_timer.h"
#include "vg_lite.h"
#include "gpu_port.h"

#include "ut_test.h"
#include "gpu_blend.h"

#define TEST_WIDTH      96
#define TEST_HEIGHT     96

#define TEST_BUF_SIZE   (TEST_WIDTH * TEST_HEIGHT)

#ifdef PSRAM_ENABLE
static volatile uint32_t gpu_vg_buf[TEST_BUF_SIZE] ALIGNED(64) PSRAM_NC_BSS_LOC;
#else
static volatile uint32_t gpu_vg_buf[TEST_BUF_SIZE] ALIGNED(64);
#endif

static vg_lite_buffer_t vg_buf;

static char *error_type[] =
{
    "VG_LITE_SUCCESS",
    "VG_LITE_INVALID_ARGUMENT",
    "VG_LITE_OUT_OF_MEMORY",
    "VG_LITE_NO_CONTEXT",
    "VG_LITE_TIMEOUT",
    "VG_LITE_OUT_OF_RESOURCES",
    "VG_LITE_GENERIC_IO",
    "VG_LITE_NOT_SUPPORT",
};
#define IS_ERROR(status)         (status > 0)
#define CHECK_ERROR(Function) \
    error = Function; \
    if (IS_ERROR(error)) \
    { \
        UT_LOG("[%s: %d] failed, error type is %s\n", \
                __func__, __LINE__,error_type[error]); \
    }

static void gpu_test_init(void)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;

    CHECK_ERROR(vg_lite_init(TEST_WIDTH, TEST_HEIGHT));

    vg_buf.format = VG_LITE_RGBA8888;
    vg_buf.tiled = VG_LITE_LINEAR;
    vg_buf.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    vg_buf.width = (int32_t) TEST_WIDTH;
    vg_buf.height = (int32_t) TEST_HEIGHT;
    vg_buf.stride = (int32_t) (TEST_WIDTH*4);
    vg_buf.memory = (void *)gpu_vg_buf;
    vg_buf.address = (uint32_t)gpu_vg_buf;
    vg_buf.handle = NULL;
}

static void gpu_test_exit(void)
{
    vg_lite_close();
}

static void gpu_test_draw(float scale, uint32_t rotate, uint32_t color1, uint32_t color2, uint32_t bgcolor)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_matrix_t matrix;
    int8_t rect_path_data[] = {
        2, -48, -48, //move to
        4, 48, -48,  //line to
        4, 48, 48,   //line to
        4, -48, 48,  //line to
        0,
    };
    vg_lite_path_t path_rect = {
        {-48, -48,          // left,top
         48, 48},           // right,bottom
        VG_LITE_HIGH,        // quality
        VG_LITE_S8,          // -128 to 127 coordinate range
        {0},                 // uploaded
        sizeof(rect_path_data),   // path length
        rect_path_data,           // path data
        1};
    vg_lite_path_t path_circle = {
        {-48, -48,          // left,top
         48, 48},           // right,bottom
        VG_LITE_HIGH,        // quality
        VG_LITE_S8,          // -128 to 127 coordinate range
        {0},                 // uploaded
        sizeof(circle_path_data),   // path length
        circle_path_data,           // path data
        1};

    vg_lite_identity(&matrix);

    vg_lite_translate(TEST_WIDTH / 2.0f, TEST_HEIGHT / 2.0f, &matrix);

    vg_lite_scale(scale, scale, &matrix);

    rotate = rotate % 360;
    vg_lite_rotate((float)rotate, &matrix);

    vg_lite_clear(&vg_buf, NULL, bgcolor);

    CHECK_ERROR(vg_lite_draw(&vg_buf, &path_rect,
                VG_LITE_FILL_EVEN_ODD, &matrix,
                VG_LITE_BLEND_NONE, color1));
    CHECK_ERROR(vg_lite_finish());

    CHECK_ERROR(vg_lite_draw(&vg_buf, &path_circle,
                VG_LITE_FILL_EVEN_ODD, &matrix,
                VG_LITE_BLEND_MULTIPLY, color2));
    CHECK_ERROR(vg_lite_finish());
}

int gpu_blend_verify_test(int test_times)
{
    int ret = 0;

    gpu_test_init();

    for (int t = 1; t <= test_times; t++) {
        UT_LOG("GPU blend test %d ... start", t);

        /* scale 0.5, rotate 0 */
        gpu_test_draw(0.5, 0, 0x8000ff00, 0x800000ff, 0xffffffff);

        /* compare buffer data */
        for (uint32_t i = 0; i < TEST_BUF_SIZE; i++) {
            if (gpu_vg_buf[i] != blend_data_pattern1[i]) {
                UT_ERR("ERROR %5d: expect 0x%x, got 0x%x", i,
                        blend_data_pattern1[i],
                        gpu_vg_buf[i]);
                ret = -1;
                goto exit;
            }
        }

        /* scale 1.2, rotate 45 */
        gpu_test_draw(1.2, 45, 0x8000ff00, 0x800000ff, 0xffffffff);

        /* compare buffer data */
        for (uint32_t i = 0; i < TEST_BUF_SIZE; i++) {
            if (gpu_vg_buf[i] != blend_data_pattern2[i]) {
                UT_ERR("ERROR %5d: expect 0x%x, got 0x%x", i,
                        blend_data_pattern2[i],
                        gpu_vg_buf[i]);
                ret = -1;
                goto exit;
            }
        }

        UT_LOG("GPU blend test %d ... OKAY", t);
    };

exit:
    gpu_test_exit();
    return ret;
}

static void unitest_blend_usage(void)
{
    eshell_putstring("Usage:\r\n");
    eshell_putstring("  utest_blend [times]\r\n");
}

static void unitest_blend(int argc, char *argv[])
{
    int times = 10;

    if (argc > 1) {
        if (strncmp(argv[1], "help", 4) == 0) {
            unitest_blend_usage();
            return;
        }

        times = atoi(argv[1]);
    }

    gpu_blend_verify_test(times);
}
ESHELL_DEF_COMMAND(ESHELL_CMD_GRP_UNIT_TEST, "utest_blend", "usage: utest_blend help",
                   unitest_blend);

#endif
