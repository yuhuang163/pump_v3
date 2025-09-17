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
#ifdef CSI_TEST
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "string.h"
#include "hal_location.h"
#include "hal_cache.h"
#include "hal_cmu.h"
#include "hal_iomux.h"
#include "hal_gpio.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_key.h"
#include "hal_i2c.h"
#include "hal_dsi.h"
#include "hal_lcdc.h"
#include "hal_csi.h"
#include "csi_dsi_conf.h"
#include "csi_host_app.h"
#include "video_player.h"
#include "csi_dsi_loop_app.h"

#if defined(CSI_DSI_FT_LOOP_TEST) || defined(DSI_CLOCK_TEST)

#define IN_IMAGE_WIDTH  480
#define IN_IMAGE_HEIGHT 480
#define WIDTH           480
#define HEIGHT          480
#define DISPLAY_NPLANES 2
#define DISPLAY_BPP     24

#define DISPLAY_FPS     60

#define CSI_REG_WRITE(addr, val) (*(volatile uint32_t *)(addr)=(val))
#define CSI_REG_READ(addr)       (*(volatile uint32_t *)(addr))
#define DSI_REG_WRITE(addr, val) (*(volatile uint32_t *)(addr)=(val))
#define DSI_REG_READ(addr)       (*(volatile uint32_t *)(addr))

enum DSP_CLOCK_USER_T {
    DSP_CLOCK_USER_CSI,
    DSP_CLOCK_USER_DSI,
};

static uint8_t dsp_clock_user_map = 0;

static POSSIBLY_UNUSED const struct HAL_DSI_CFG_T dsi_cfg = {
    .active_width                   = WIDTH,
    .active_height                  = HEIGHT,
    .h_front_porch                  = 13,
    .h_back_porch                   = 13,
    .v_front_porch                  = 13,
    .v_back_porch                   = 13,
    .total_width                    = WIDTH +32,
    .total_height                   = HEIGHT +32,

    .y_mem_pitch                    = IN_IMAGE_WIDTH*2, //yuv  123
    .uv_mem_pitch                   = 0,
    .c_mem_pitch                    = 0,
    .image_h_sa                     = 1,
    .image_v_sa                     = 1,
    .image_width                    = IN_IMAGE_WIDTH,
    .image_height                   = IN_IMAGE_HEIGHT,
    .zm_image_width                 = WIDTH,
    .zm_image_height                = HEIGHT,

    .g_mem_pitch                    = 60*4,
    .graphic_h_sa                   = 150,
    .graphic_v_sa                   = 150,
    .graphic_width                  = 60,
    .graphic_height                 = 60,
    .zm_graphic_width               = 120,
    .zm_graphic_height              = 120,

    .g_tv_mem_pitch                 = 80*4,
    .tvg_h_sa                       = 50,
    .tvg_v_sa                       = 50,
    .tvg_width                      = 80,
    .tvg_height                     = 80,
    .zm_tvg_width                   = 160,
    .zm_tvg_height                  = 160,

    .cursor_h_sa                    = 90,
    .cursor_v_sa                    = 90,
    .cursor_width                   = 28,
    .cursor_height                  = 28,
    .hwc_color1                     = 0x0000ff,
    .hwc_color2                     = 0x00ff00,

    .blankcolor                     = 0x104060,
    //.blankcolor                     = 0x000000,

    .cos0                           = 0x3fc2,
    .sin0                           = 0x05a4,
    .c_mult_s                       = 0x3000,
    .saturation                     = 0x5000,
    .brightness                     = 0xfff8,
    .contrast                       = 0x5000,

    .cfg_alpha_y                    = 0x00,
    .cfg_ckey_y                     = 0x16,
    .cfg_ckey_y1                    = 0x0,
    .cfg_ckey_y2                    = 0x60,
    .cfg_alpha_u                    = 0xff,
    .cfg_ckey_u                     = 0x80,
    .cfg_ckey_u1                    = 0x0,
    .cfg_ckey_u2                    = 0x80,
    .cfg_alpha_v                    = 0xff,
    .cfg_ckey_v                     = 0x80,
    .cfg_ckey_v1                    = 0x0,
    .cfg_ckey_v2                    = 0x80,
};

static POSSIBLY_UNUSED struct HAL_CSI_CONFIG_T csi_cfg;

uint32_t dsi_sensor_calc_pixel_clock(uint16_t hpixel, uint16_t vline, uint8_t frameRate);
uint32_t dsi_sensor_calc_bit_clock(uint16_t hpixel, uint16_t vline, uint8_t frameRate, uint8_t bpp, uint8_t lane_number);

static POSSIBLY_UNUSED uint32_t csi_ft_loop_handler(uint8_t *buf, uint32_t len, uint32_t err)
{
    return 0;
}

static POSSIBLY_UNUSED void csi_dsi_loop_enable(bool enable)
{
#define DSI_REG_LOOP_EN (1<<25)
    uint32_t addr = 0x58005064;
    uint32_t val = DSI_REG_READ(addr);

    if (enable) {
        val |= DSI_REG_LOOP_EN;
    } else {
        val &= ~DSI_REG_LOOP_EN;
    }
    DSI_REG_WRITE(addr, val);
}

void dsp_clock_request(enum DSP_CLOCK_USER_T user)
{
    int set = 0;

    if (dsp_clock_user_map == 0) {
        set = 1;
    }
    dsp_clock_user_map |= (1 << user);
    if (set) {
        hal_cmu_dsp_clock_enable();
        hal_cmu_dsp_reset_clear();
    }
}

void dsp_clock_release(enum DSP_CLOCK_USER_T user)
{
    if (dsp_clock_user_map == 0) {
        return;
    }
    dsp_clock_user_map &= ~(1 << user);
    if (dsp_clock_user_map == 0) {
        hal_cmu_dsp_clock_disable();
        hal_cmu_dsp_reset_set();
    }
}

void mipi_dsi_open(void)
{
    uint32_t total_width, total_height, width, height;
    uint32_t dsi_bclk, dsi_pclk;
    uint32_t fps, bpp, lane;

    dsp_clock_request(DSP_CLOCK_USER_DSI);

    total_width  = dsi_cfg.total_width;
    total_height = dsi_cfg.total_height;
    width        = dsi_cfg.active_width;
    height       = dsi_cfg.active_height;
    fps          = DISPLAY_FPS;
    bpp          = DISPLAY_BPP;
    lane         = DISPLAY_NPLANES;

    dsi_bclk = dsi_sensor_calc_bit_clock(total_width, total_height, fps, bpp, lane);
    dsi_pclk = dsi_sensor_calc_pixel_clock(total_width, total_height, fps);

    TRACE(1, "DSI INFO:");
    TRACE(1, "total_width=%d, total_height=%d, fps=%d, bpp=%d, lane=%d",
        total_width, total_height, fps, bpp, lane);
    TRACE(1, "width=%d, height=%d, bitclk=%d, pixclk=%d", width, height, dsi_bclk, dsi_pclk);

    hal_dsi_init_v2(width, DSI_MODE_VIDEO, lane, dsi_bclk, dsi_pclk);
}

void mipi_dsi_start(void)
{
    hal_dsi_start();
}

void mipi_dsi_stop(void)
{
}

void mipi_dsi_close(void)
{
    dsp_clock_release(DSP_CLOCK_USER_DSI);
}

static void wait_test_cmd(uint32_t cmd)
{
#define TEST_CMD_ADDR 0x400000F4
    volatile uint32_t val;
    uint32_t orgval;

    TRACE(1, "[%s]: WAIT CMD [%x]=%x", __func__, TEST_CMD_ADDR, cmd);
    orgval = *(volatile uint32_t *)TEST_CMD_ADDR;
    do {
        val = *(volatile uint32_t *)TEST_CMD_ADDR;
        if (val == cmd) {
            break;
        }
        hal_sys_timer_delay(MS_TO_TICKS(5));
    } while (1);
    *(volatile uint32_t *)TEST_CMD_ADDR = orgval;
    TRACE(1, "[%s]: done", __func__);
}

void dsi_clock_test(int times, uint32_t tx_intv_ms, int wait_cmd)
{
#define DSI_TST_CMD 0x29
    int i;
    uint8_t cmd = DSI_TST_CMD;

    TRACE(1, "[%s]: start, times=%d, tx_intv_ms=%d, wait_cmd=%d",
        __func__, times, tx_intv_ms, wait_cmd);

    hal_cmu_simu_init();

    mipi_dsi_open();

    if (wait_cmd) {
        wait_test_cmd(0x90);
    }

    mipi_dsi_start();

    for (i = 0; i < times; i++) {
        TRACE(1, "CSI SEND[%d]: LP CMD[%x]", i, cmd);
        hal_sys_timer_delay(MS_TO_TICKS(5));
        hal_dsi_send_cmd(cmd);

        TRACE(1, "CSI SEND[%d]: HS CMD[%x]", i, cmd);
        hal_sys_timer_delay(US_TO_TICKS(50));
        hal_dsi_start_hs_clock();
        hal_dsi_send_hscmd(DSI_TST_CMD);

        hal_sys_timer_delay(MS_TO_TICKS(2));
        hal_dsi_stop_hs_clock();

        if (tx_intv_ms > 0) {
            hal_sys_timer_delay(MS_TO_TICKS(tx_intv_ms));
        }
    }

    mipi_dsi_stop();
    mipi_dsi_close();

    TRACE(1, "[%s]: done", __func__);

    hal_cmu_simu_pass();
    for(;;);
}

int csi_dsi_ft_loop(void)
{
    int err = 0;
    uint32_t addr, regval;

    TRACE(1, "[%s]: start", __func__);

    dsp_clock_request(DSP_CLOCK_USER_CSI);

    // config CSI controller
    memset((void *)&csi_cfg, 0, sizeof(csi_cfg));
    csi_cfg.fmt = CSI_PIX_FMT_YUV422;
    csi_cfg.width = 800;
    csi_cfg.height = 200;
    csi_cfg.pp_mode = 0;
    csi_cfg.data_lane_num = CSI_DATA_LANE_NUM_2;
    csi_cfg.data_ptr = (uint8_t *)CSI_DATA_BUF_ADDR;
    csi_cfg.data_size = CSI_DATA_BUF_SIZE;
    csi_cfg.handler = csi_ft_loop_handler;

    hal_csi_open(&csi_cfg);

#if 1
    // CSI TIMING
    CSI_REG_WRITE(0x58006000, 0x02010001);
    // CSI WIDTH & HEIGHT
    CSI_REG_WRITE(0x58006064, 0x003200C8);
#endif

    addr = 0x58006000;
    TRACE(1, "CSI REG[%x] = %08x", addr, CSI_REG_READ(addr));
    addr = 0x58006068;
    TRACE(1, "CSI REG[%x] = %08x", addr, CSI_REG_READ(addr));
    addr = 0x58006064;
    TRACE(1, "CSI REG[%x] = %08x", addr, CSI_REG_READ(addr));

    hal_csi_start();

    mipi_dsi_open();

#ifdef MTEST_WAIT_CMD
    TRACE(1, "Now waiting cmd before DSI starting ...");
    wait_test_cmd(0x90);
#endif

    mipi_dsi_start();

#if 1
    DSI_REG_WRITE(0x58005000, 0x1F060369);
    DSI_REG_WRITE(0x58005004, 0x81CF0ECB);
    DSI_REG_WRITE(0x58005008, 0x19801C01);
    DSI_REG_WRITE(0x5800500C, 0x58043F09);
    DSI_REG_WRITE(0x58005048, 0x2B400000);
    DSI_REG_WRITE(0x58005060, 0x20802010);
    DSI_REG_WRITE(0x58005064, 0x02060A10);
#endif
    //csi_dsi_loop_enable(true);

    addr = 0x58005000;
    TRACE(1, "DSI REG[%x] = %08x", addr, DSI_REG_READ(addr));

    addr = 0x58005004;
    TRACE(1, "DSI REG[%x] = %08x", addr, DSI_REG_READ(addr));

    addr = 0x58005008;
    TRACE(1, "DSI REG[%x] = %08x", addr, DSI_REG_READ(addr));

    addr = 0x5800500C;
    TRACE(1, "DSI REG[%x] = %08x", addr, DSI_REG_READ(addr));

    addr = 0x58005048;
    TRACE(1, "DSI REG[%x] = %08x", addr, DSI_REG_READ(addr));

    addr = 0x58005064;
    TRACE(1, "DSI REG[%x] = %08x", addr, DSI_REG_READ(addr));

    volatile int trycnt = 2;
    do {
        hal_sys_timer_delay(MS_TO_TICKS(1));

        addr = 0x58006088;
        regval = DSI_REG_READ(addr);
        TRACE(1, "==> RESULT: CSI_REG[%x] = %08x", addr, regval);
        regval &= 0x3;
        if ((regval & (1<<0)) == 0) {
            err |= 0x1;
        } else {
            err &= ~0x1;
        }
        if ((regval & (1<<1)) == 0) {
            err |= 0x2;
        } else {
            err &= ~0x2;
        }

        addr = 0x58006084;
        regval = DSI_REG_READ(addr);
        TRACE(1, "==> RESULT: CSI_REG[%x] = %08x", addr, regval);
        if ((regval & 0xFFFF) != 0x9A55) {
            err |= 0x04;
        } else {
            err &= ~0x04;
        }
        if (((regval >> 16) & 0xFFFF) != 0x9A55) {
            err |= 0x08;
        } else {
            err &= ~0x08;
        }
        if (!err) {
            break;
        }
        trycnt--;
        if (trycnt <= 0) {
            TRACE(1, "==> timeout");
            break;
        }
    } while (1);

    if (!err) {
        TRACE(1, "[%s]: test success !!!", __func__);
    } else {
        TRACE(1, "[%s]: test error ###, err=0x%x", __func__, err);
    }

    hal_csi_stop();
    hal_csi_close();
    dsp_clock_release(DSP_CLOCK_USER_CSI);

    mipi_dsi_stop();
    mipi_dsi_close();

    return err;
}

int csi_dsi_ft_loop_test(int total_times, int pass_times)
{
    int err = 0, i;
    int pass_cnt = 0;

    TRACE(1, "[%s]: total_times=%d, pass_times=%d", __func__, total_times, pass_times);
    for (i = 0; i < total_times; i++) {
        err = csi_dsi_ft_loop();
        TRACE(1, "====> [CSI_DSI_FT_LOOP_TEST][%d/%d]: err=0x%x", i, total_times, err);
        if (!err) {
            pass_cnt++;
        }
    }
    TRACE(1, "[%s]: pass=[%d/%d]", __func__, pass_cnt, total_times);
    if (pass_cnt >= pass_times) {
        hal_cmu_simu_pass();
        err = 0;
        TRACE(1, "[%s]: ====> TEST SUCCESS !!!", __func__);
    } else {
        err = 1;
        hal_cmu_simu_fail();
        TRACE(1, "[%s]: ====> TEST ERROR, %d ###", __func__, err);
    }
    for(;;);
    return err;
}

#endif /* defined(CSI_DSI_FT_LOOP_TEST) */

void csi_host_app_test(void)
{
    csi_host_app_init();
    if (!csi_host_app_is_calibed()) {
        csi_host_app_on(true);
        osDelay(10);
        csi_host_app_on(false);
        osDelay(10);
    }
    csi_host_app_on(true);

#ifndef RTOS
extern void csi_thread(void const *arguments);
    while(1) {
        csi_thread(NULL);
        osDelay(10);
    }
#endif

#if 0
    while (1) {
        TRACE(1, "[%s]: lcdc start", __func__);
        hal_lcdc_start();
        osDelay(1000);
    }
#endif

}

#ifdef VPLAYER_TEST
#ifdef DSI_TEST
#error "DSI_TEST conflicts with VPLAYER_TEST !"
#endif
void vplayer_test(void)
{
    csi_dsi_loop_app_init();
    csi_dsi_loop_app_on(true);

#ifndef RTOS
    while (1) {
        csi_dsi_loop_app_run();
    }
#endif
}
#endif

void csi_test(void)
{
#ifdef VPLAYER_TEST
    vplayer_test();
#elif defined(CSI_DSI_FT_LOOP_TEST)
    csi_dsi_ft_loop_test(10, 5);
#elif defined(DSI_CLOCK_TEST)
    dsi_clock_test(10, 0, 1);
    //dsi_clock_test(0x7fffffff, 1000, 0);
#else
    csi_host_app_test();
#endif
}

#endif
