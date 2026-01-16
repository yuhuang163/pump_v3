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
#define DISP_DBG(...)  TR_INFO( TR_MOD(HAL), "//LCDC-RGB: "  __VA_ARGS__)
#else
#define DISP_DBG(...)
#endif
#define DISP_LOG(...)  TR_INFO( TR_MOD(HAL), "//LCDC-RGB: "  __VA_ARGS__)
#define DISP_ERR(...)  TR_INFO( TR_MOD(HAL), "//LCDC-RGB: "  __VA_ARGS__)

#define LAYER1_ID      LCDC_LAYER_GRA

static struct lcdc_config_t lcdc_config = {
    .mode = LCDC_VIDEO_MODE,
    .bus = LCDC_BUS_RGB,
    .bus_port0 = 0,
    .bus_port1 = 0,
    .bus_mhz = 96,
    .bus_lane = 4,
#ifdef LCDCPIX_MHZ
    .pix_mhz = LCDCPIX_MHZ,
#else
    .pix_mhz = 24,
#endif

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
#ifdef CONFIG_LCD_CLK_INV
        .clk_inv = true,
#endif
#ifdef CONFIG_LCD_HSYNC_INV
        .hsync_inv = true,
#endif
#ifdef CONFIG_LCD_VSYNC_INV
        .vsync_inv = true,
#endif
    },

    .te_pin = -1,

    .framedone_cb = NULL,
    .vsync_cb = NULL,
    .error_cb = NULL,
};

static uint32_t frame_start = 0;
static uint32_t frame_count = 0;

static uint8_t *new_framebuf = NULL;

static void (*disp_flush_done)(int err) = NULL;

/******************************** LCD Init ****************************************/

#ifdef CONFIG_LCD_SPI_CMD
#define SCL     CONFIG_LCD_SPI_SCL
#define CS      CONFIG_LCD_SPI_CS
#define SDO     CONFIG_LCD_SPI_SDO
#define SDA     CONFIG_LCD_SPI_SDA

POSSIBLY_UNUSED
static void lcd_spi_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] = {
        {(enum HAL_IOMUX_PIN_T)SCL, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {(enum HAL_IOMUX_PIN_T)CS, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {(enum HAL_IOMUX_PIN_T)SDA, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
        {(enum HAL_IOMUX_PIN_T)SDO, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(CS, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(SCL, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set_dir(SDA, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set_dir(SDO, HAL_GPIO_DIR_IN, 0);
}

POSSIBLY_UNUSED
static void lcd_write_cmd(uint16_t cmd)
{
    hal_gpio_pin_clr(CS);
    for (int i = 0; i < 9; i++) {
        hal_gpio_pin_clr(SCL);
        if (cmd & 0x0100)
            hal_gpio_pin_set(SDA);
        else
            hal_gpio_pin_clr(SDA);
        hal_sys_timer_delay_us(10);
        hal_gpio_pin_set(SCL);
        hal_sys_timer_delay_us(10);
        cmd = cmd << 1;
    }
    hal_gpio_pin_clr(SCL);
    hal_gpio_pin_set(CS);
}

POSSIBLY_UNUSED
static void lcd_write_data(uint16_t data)
{
    data |= 0x100;
    hal_gpio_pin_clr(CS);
    for (int i = 0; i < 9; i++) {
        hal_gpio_pin_clr(SCL);
        if (data & 0x0100)
            hal_gpio_pin_set(SDA);
        else
            hal_gpio_pin_clr(SDA);
        hal_sys_timer_delay_us(10);
        hal_gpio_pin_set(SCL);
        hal_sys_timer_delay_us(10);
        data = data << 1;
    }
    hal_gpio_pin_clr(SCL);
    hal_gpio_pin_set(CS);
}

POSSIBLY_UNUSED
static void lcd_spi_read(uint16_t cmd, uint8_t *data, int len)
{
    /* CS active */
    hal_gpio_pin_clr(CS);

    /* send command */
    for (int i = 0; i < 9; i++) {
        hal_gpio_pin_clr(SCL);
        if (cmd & 0x0100)
            hal_gpio_pin_set(SDA);
        else
            hal_gpio_pin_clr(SDA);
        hal_sys_timer_delay_us(10);
        hal_gpio_pin_set(SCL);
        hal_sys_timer_delay_us(10);
        cmd = cmd << 1;
    }

    /* dummy clock cycle */
    if (len > 1) {
        hal_gpio_pin_clr(SCL);
        hal_sys_timer_delay_us(10);
        hal_gpio_pin_set(SCL);
        hal_sys_timer_delay_us(10);
    } else {
        hal_sys_timer_delay_us(20);
    }

    /* receive data */
    for (int i = 0; i < len; i++) {
        data[i] = 0;
        for (int j = 0; j < 8; j++) {
            hal_gpio_pin_clr(SCL);
            hal_sys_timer_delay_us(10);
            if (hal_gpio_pin_get_val(SDO))
                data[i] |= (1 << (7 - j));
            hal_gpio_pin_set(SCL);
            hal_sys_timer_delay_us(10);
        }
    }

    /* CS deactive */
    hal_gpio_pin_set(CS);
}

POSSIBLY_UNUSED
static void lcd_spi_write(uint16_t cmd, uint8_t *data, int len)
{
    /* send command */
    hal_gpio_pin_clr(CS);
    for (int i = 0; i < 9; i++) {
        hal_gpio_pin_clr(SCL);
        if (cmd & 0x0100)
            hal_gpio_pin_set(SDA);
        else
            hal_gpio_pin_clr(SDA);
        hal_sys_timer_delay_us(10);
        hal_gpio_pin_set(SCL);
        hal_sys_timer_delay_us(10);
        cmd = cmd << 1;
    }
    hal_gpio_pin_clr(SCL);
    hal_gpio_pin_set(CS);
    hal_sys_timer_delay_us(50);

    if (!data || len == 0)
        return;

    /* send data */
    hal_gpio_pin_clr(CS);
    for (int j = 0; j < len; j++) {
        uint16_t t = data[j] | 0x100;
        for (int i = 0; i < 9; i++) {
            hal_gpio_pin_clr(SCL);
            if (t & 0x0100)
                hal_gpio_pin_set(SDA);
            else
                hal_gpio_pin_clr(SDA);
            hal_sys_timer_delay_us(10);
            hal_gpio_pin_set(SCL);
            hal_sys_timer_delay_us(10);
            t = t << 1;
        }
    }
    hal_gpio_pin_clr(SCL);
    hal_gpio_pin_set(CS);
}

#else /* CONFIG_LCD_SPI_CMD */

POSSIBLY_UNUSED
static void lcd_spi_init(void)
{
}

POSSIBLY_UNUSED
static void lcd_spi_read(uint16_t cmd, uint8_t *data, int len)
{
}

POSSIBLY_UNUSED
static void lcd_spi_write(uint16_t cmd, uint8_t *data, int len)
{
}
#endif /* !CONFIG_LCD_SPI_CMD */

POSSIBLY_UNUSED
static int lcd_read_id(uint32_t *id)
{
    uint8_t *ids = (uint8_t *)id;
    memset(id, 0, sizeof(uint32_t));
#if 0
    lcd_spi_read(0xda, &ids[0], 1);
    lcd_spi_read(0xdb, &ids[1], 1);
    lcd_spi_read(0xdc, &ids[2], 1);
#else
    lcd_spi_read(0x04, ids, 3);
#endif
    return 0;
}

POSSIBLY_UNUSED
static int lcd_read_status(uint8_t *st)
{
    lcd_spi_read(0x0a, st, 1);
    return 0;
}

static int lcd_panel_init(void)
{
#ifndef SIMU
    uint8_t lcd_status = 0;
    uint32_t lcd_id = 0;
    struct lcd_command *cmds = lcd_init_cmds;
    int count = sizeof(lcd_init_cmds) / sizeof(struct lcd_command);

    lcd_spi_init();

    if (lcd_read_status(&lcd_status) < 0) {
        DISP_ERR("Failed to read LCD power status.");
        return -1;
    }
    DISP_LOG("LCD power status: %02x", lcd_status);

    if (lcd_read_id(&lcd_id) < 0) {
        DISP_ERR("Failed to read LCD id.");
        return -1;
    }
    DISP_LOG("LCD id: 0x%06x", lcd_id);

    for (int i = 0; i < count; i++) {
        lcd_spi_write(cmds[i].cmd, cmds[i].param_data, cmds[i].param_cnt);
        if (cmds[i].delay_ms > 0)
            hal_sys_timer_delay(MS_TO_TICKS(cmds[i].delay_ms));
    }

    DISP_LOG("LCD [%s] is initialized.", LCD_NAME);
#endif
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
    frame_start = hal_fast_sys_timer_get();

    if (new_framebuf) {
        if (disp_flush_done)
            disp_flush_done(0);

        new_framebuf = NULL;
    }
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

    if (lcd_panel_init()) {
        ASSERT(0, "LCD0 init failed!");
    }

    if (lcdc_init(&lcdc_config) != LCDC_OK) {
        ASSERT(0, "LCDC init failed!");
    }

    lcdc_register_framedone_cb(lcdc_framedone_cb);

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
    uint32_t lock = int_lock();
    new_framebuf = (uint8_t *)buf;
    lcdc_layer_set_buffer(LAYER1_ID, new_framebuf, LCDC_STRIDE);
    int_unlock(lock);

    DISP_DBG("switch to framebuffer %p", new_framebuf);
}

