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

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cmsis.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#include "hal_timer.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_cache.h"
#include "hal_location.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "hal_cmu.h"
#include "hal_psc.h"
#include "lcdc_drv.h"

#include "disp_conf.h"
#include "lcd_cmds.h"

//#define DISP_LCDC_DEBUG
#ifdef DISP_LCDC_DEBUG
#define DISP_DBG(...)  TR_INFO( TR_MOD(HAL), "//LCDC-DSI: "  __VA_ARGS__)
#else
#define DISP_DBG(...)
#endif
#define DISP_LOG(...)  TR_INFO( TR_MOD(HAL), "//LCDC-DSI: "  __VA_ARGS__)
#define DISP_ERR(...)  TR_INFO( TR_MOD(HAL), "//LCDC-DSI: "  __VA_ARGS__)

#ifdef CONFIG_LCDC_COMPRESS
#define LAYER1_ID      LCDC_LAYER_DEC
#else
#define LAYER1_ID      LCDC_LAYER_GRA
#endif
#define LAYER2_ID      LCDC_LAYER_DMA

static struct lcdc_config_t lcdc_config = {
#ifdef CONFIG_DSI_VIDEO_MODE
    .mode = LCDC_VIDEO_MODE,
#else
    .mode = LCDC_COMMAND_MODE,
#endif
#ifdef CONFIG_LCDC_DUAL_DSI
    .bus = LCDC_BUS_DUAL_DSI,
    .bus_port0 = HAL_DSI_ID_0,
    .bus_port1 = HAL_DSI_ID_1,
#else
    .bus = LCDC_BUS_DSI,
#ifdef CONFIG_LCDC_USE_DSI1
    .bus_port0 = HAL_DSI_ID_1,
    .bus_port1 = HAL_DSI_ID_1,
#else
    .bus_port0 = HAL_DSI_ID_0,
    .bus_port1 = HAL_DSI_ID_0,
#endif
#endif
#ifdef DSIPLL_MHZ
    .bus_mhz = DSIPLL_MHZ,
#else
    .bus_mhz = 400,
#endif
#ifdef CONFIG_DSI_4LANE_MODE
    .bus_lane = 4,
#elif defined(CONFIG_DSI_2LANE_MODE)
    .bus_lane = 2,
#else
    .bus_lane = 1,
#endif
    .pix_mhz = 24,

    .lcd = {
        .width  = LCD_WIDTH,
        .height = LCD_HEIGHT,
        .hfp    = LCD_HFP,
        .hbp    = LCD_HBP,
        .hsync  = LCD_HSYNC,
        .vfp    = LCD_VFP,
        .vbp    = LCD_VBP,
        .vsync  = LCD_VSYNC,
#ifdef CONFIG_LCD_RGB565
        .color_bits = 16,
#else
        .color_bits = 24,
#endif
#ifdef CONFIG_LCD_SWAP_RB
        .swap_rb = true,
#else
        .swap_rb = false,
#endif
    },

    .te_pin = -1,

    .framedone_cb = NULL,
    .vsync_cb = NULL,
    .error_cb = NULL,
};

static uint32_t frame_start = 0;
static uint32_t frame_count = 0;

#ifdef RTOS
POSSIBLY_UNUSED
static osSemaphoreId_t framedone_sem = NULL;
#else
POSSIBLY_UNUSED
static bool frame_xfer = false;
#endif

static uint8_t *new_framebuf = NULL;

static void (*disp_flush_done)(int err) = NULL;

/******************************** LCD Init ****************************************/

POSSIBLY_UNUSED
static int lcd_read_id(int port, uint32_t *id)
{
    int ret = 0;
    uint8_t *ids = (uint8_t *)id;
    memset(id, 0, sizeof(uint32_t));
    ret = lcdc_read_cmd(port, 0xda, &ids[0], 1);
    if (ret < 0)
        return -1;
    ret = lcdc_read_cmd(port, 0xdb, &ids[1], 1);
    if (ret < 0)
        return -1;
    ret = lcdc_read_cmd(port, 0xdc, &ids[2], 1);
    if (ret < 0)
        return -1;
    return 0;
}

POSSIBLY_UNUSED
static int lcd_read_status(int port, uint8_t *st)
{
    int ret = 0;
    ret = lcdc_read_cmd(port, 0x0a, st, 1);
    if (ret < 0)
        return -1;
    return 0;
}

static int lcd_panel_init(int port)
{
    uint8_t lcd_status = 0;
    uint32_t lcd_id = 0;
    struct lcd_command *cmds = lcd_init_cmds;
    int count = sizeof(lcd_init_cmds) / sizeof(struct lcd_command);
#ifdef SIMU
    return 0;
#endif

    if (lcd_read_status(port, &lcd_status) < 0) {
        DISP_ERR("Failed to read LCD power status.");
        return -1;
    }
    DISP_LOG("LCD power status: %02x", lcd_status);

    if (lcd_read_id(port, &lcd_id) < 0) {
        DISP_ERR("Failed to read LCD id.");
        return -1;
    }
    DISP_LOG("LCD id: 0x%06x", lcd_id);

    for (int i = 0; i < count; i++) {
        if (cmds[i].param_cnt > 0)
            lcdc_send_cmd_list(port, cmds[i].cmd, cmds[i].param_cnt, cmds[i].param_data);
        else
            lcdc_send_cmd(port, cmds[i].cmd);
        if (cmds[i].delay_ms > 0)
            hal_sys_timer_delay(MS_TO_TICKS(cmds[i].delay_ms));
    }

    DISP_LOG("LCD [%s] is initialized.", LCD_NAME);
    return 0;
}

static void lcd_power_on(void)
{
#ifndef SIMU
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] = {
        {(enum HAL_IOMUX_PIN_T)LCD_RST_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };

#ifdef LCD_LED_PIN
    pinmux[0].pin = LCD_LED_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(LCD_LED_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(LCD_LED_PIN);
#endif
#ifdef LCD_1V8_PIN
    pinmux[0].pin = (enum HAL_IOMUX_PIN_T)LCD_1V8_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(LCD_1V8_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(LCD_1V8_PIN);
#endif
#ifdef LCD_2V8_PIN
    pinmux[0].pin = (enum HAL_IOMUX_PIN_T)LCD_2V8_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(LCD_2V8_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(LCD_2V8_PIN);
#endif
#ifdef LCD_3V3_PIN
    pinmux[0].pin = (enum HAL_IOMUX_PIN_T)LCD_3V3_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(LCD_3V3_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(LCD_3V3_PIN);
#endif
    pinmux[0].pin = (enum HAL_IOMUX_PIN_T)LCD_RST_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(LCD_RST_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(LCD_RST_PIN);
#ifdef LCD_RST_PIN2
    pinmux[0].pin = (enum HAL_IOMUX_PIN_T)LCD_RST_PIN2;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(LCD_RST_PIN2, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(LCD_RST_PIN2);
#endif
    hal_sys_timer_delay(MS_TO_TICKS(10));
    hal_gpio_pin_clr(LCD_RST_PIN);
#ifdef LCD_RST_PIN2
    hal_gpio_pin_clr(LCD_RST_PIN2);
#endif
    hal_sys_timer_delay(MS_TO_TICKS(2));
    hal_gpio_pin_set(LCD_RST_PIN);
#ifdef LCD_RST_PIN2
    hal_gpio_pin_set(LCD_RST_PIN2);
#endif
    hal_sys_timer_delay(MS_TO_TICKS(10));
#endif
}

/*********************************** LCDC control **************************************/

static void lcdc_framedone_cb(void)
{
    DISP_DBG("frame %u transfer done, used %uus", frame_count,
              FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start));
    frame_count++;
#ifdef CONFIG_DSI_VIDEO_MODE
    frame_start = hal_fast_sys_timer_get();
    if (new_framebuf) {
        if (disp_flush_done)
            disp_flush_done(0);

        new_framebuf = NULL;
    }
#else
#ifdef RTOS
    if (framedone_sem)
        osSemaphoreRelease(framedone_sem);
#else
    frame_xfer = false;
#endif
    lcdc_sleep();
#endif
}

static void lcdc_vsync_cb(void)
{
    if (!new_framebuf)
        return;

    lcdc_xfer_frame();
    frame_start = hal_fast_sys_timer_get();

    if (disp_flush_done)
        disp_flush_done(0);

    new_framebuf = NULL;
}

int disp_lcdc_init(void *buf, void (*flush_cb)(int))
{
    struct lcdc_layer_config_t config;
    int offs_x = 0;
    int offs_y = 0;

#ifdef CONFIG_LCDC_ALIGN_MIDDLE
    if (LCDC_WIDTH > LCD_WIDTH) {
        offs_x = (LCDC_WIDTH - LCD_WIDTH) / 2;
    }
    if (LCDC_HEIGHT > LCD_HEIGHT) {
        offs_y = (LCDC_HEIGHT - LCD_HEIGHT) / 2;
    }
#endif

    lcd_power_on();

    if (lcdc_config.mode == LCDC_COMMAND_MODE && lcdc_config.te_pin < 0) {
#ifdef LCD_TE_PIN
        lcdc_config.te_pin = LCD_TE_PIN;
#else
        lcdc_config.te_pin = (enum HAL_GPIO_PIN_T)hal_iomux_get_dsi_te_pin();
#endif
    }

    if (lcdc_init(&lcdc_config) != LCDC_OK)
        ASSERT(0, "LCDC init failed!");

    lcdc_register_framedone_cb(lcdc_framedone_cb);
    if (lcdc_config.mode == LCDC_COMMAND_MODE)
        lcdc_register_vsync_cb(lcdc_vsync_cb);

    if (lcd_panel_init(lcdc_config.bus_port0))
        ASSERT(0, "LCD0 init failed!");
    if (lcdc_config.bus == LCDC_BUS_DUAL_DSI &&
        lcd_panel_init(lcdc_config.bus_port1))
        ASSERT(0, "LCD1 init failed!");

    memset(&config, 0, sizeof(config));
#ifdef CONFIG_LCDC_COMPRESS
#if (CONFIG_LCDC_COMPRESS == 2)
    config.dec_mode = LCDC_DEC_H_SAMPLE;
#else
    config.dec_mode = LCDC_DEC_NON_SAMPLE;
#endif
    config.dec_format = LCDC_DEC_XRGB;
#endif
    config.buf_width = LCDC_WIDTH;
    config.buf_height = LCDC_HEIGHT;
    config.width = LCD_WIDTH;
    config.height = LCD_HEIGHT;
    config.offset_x = offs_x;
    config.offset_y = offs_y;
    config.format = LCDC_FORMAT;
    config.stride = LCDC_STRIDE;
    config.buffer = buf;
    config.enable = true;
    lcdc_layer_config(LAYER1_ID, &config);

    if (lcdc_config.bus == LCDC_BUS_DUAL_DSI) {
        config.format = LCDC_FORMAT;
        config.stride = LCDC_STRIDE;
        config.buffer = buf;
        config.enable = true;
        lcdc_layer_config(LAYER2_ID, &config);
        lcdc_layer_select(lcdc_config.bus_port0, (1 << LAYER1_ID));
        lcdc_layer_select(lcdc_config.bus_port1, (1 << LAYER2_ID));
    }

#ifdef RTOS
    if (!framedone_sem)
        framedone_sem = osSemaphoreNew(1, 1, NULL);
#else
    frame_xfer = false;
#endif

    frame_count = 0;
    disp_flush_done = flush_cb;

    lcdc_start();
    DISP_LOG("display lcdc initialized.");
    return 0;
}

void disp_lcdc_exit(void)
{
    lcdc_stop();
    lcdc_exit();
}

void disp_lcdc_send_frame(const void *buf)
{
    uint32_t lock;

#ifndef CONFIG_DSI_VIDEO_MODE
#ifdef RTOS
    if (framedone_sem)
        osSemaphoreAcquire(framedone_sem, -1);
#else
    while (frame_xfer)
        hal_sys_timer_delay(1);
    frame_xfer = true;
#endif
    lcdc_wakeup();
#endif

    lock = int_lock();
    new_framebuf = (uint8_t *)buf;
    lcdc_layer_set_buffer(LAYER1_ID, new_framebuf, LCDC_STRIDE);
    if (lcdc_config.bus == LCDC_BUS_DUAL_DSI)
        lcdc_layer_set_buffer(LAYER2_ID, new_framebuf, LCDC_STRIDE);
    int_unlock(lock);

    DISP_DBG("switch to framebuffer %p", new_framebuf);
#ifdef SIMU
#ifndef CONFIG_DSI_VIDEO_MODE
    lcdc_vsync_cb();
#endif
#endif
}
