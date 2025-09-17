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
/**
 * LCD panel board init/un-init interfaces.
 */

#include <stdio.h>
#include "string.h"
#include "lcdc_fb.h"
#include "lcdc.h"
#include "hal_dsi.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_timer.h"
#include "hal_cmu.h"
#include "hal_trace.h"
#include "hal_lcdc.h"
#include "hal_lcdc_qspi.h"
#include "hal_psc.h"
#include "hal_location.h"
#include "lcdc_drv.h"

#include "lcd_conf.h"
#include "lcd_cmds.h"

static struct lcdc_config_t lcdc_config = {
#ifdef CONFIG_DSI_VIDEO_MODE
    .mode = LCDC_VIDEO_MODE,
#else
    .mode = LCDC_COMMAND_MODE,
#endif
#ifdef CONFIG_LCDC_QSPI
    .bus = LCDC_BUS_QSPI,
    .bus_mhz = 96,
    .bus_lane = 4,
    .bus_port0 = 0,
    .bus_port1 = 0,
#elif defined(CONFIG_LCDC_DSI)
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
#else
    .bus = LCDC_BUS_RGB,
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

    .irq_cb = NULL,
    .framedone_cb = NULL,
    .vsync_cb = NULL,
    .error_cb = NULL,
};

#define LCD_INFO(...)  TR_INFO( TR_MOD(TEST), "[LCD]"  __VA_ARGS__)

static void lcd_power_up(void)
{
#ifndef SIMU
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] = {
        {LCD_RST_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };

    LCD_INFO("Set LCD pins ...");

#ifdef LCD_LED_PIN
    pinmux[0].pin = LCD_LED_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(LCD_LED_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(LCD_LED_PIN);
    LCD_INFO("LED_PIN:%d ",LCD_LED_PIN);
#endif

#ifdef LCD_1V8_PIN
    pinmux[0].pin = LCD_1V8_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(LCD_1V8_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(LCD_1V8_PIN);
    LCD_INFO("1V8_PIN:%d ",LCD_1V8_PIN);
#endif

#ifdef LCD_2V8_PIN
    pinmux[0].pin = LCD_2V8_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(LCD_2V8_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(LCD_2V8_PIN);
    LCD_INFO("2V8_PIN:%d ",LCD_2V8_PIN);
#endif

#ifdef LCD_3V3_PIN
    pinmux[0].pin = LCD_3V3_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(LCD_3V3_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(LCD_3V3_PIN);
    LCD_INFO("3V3_PIN:%d ",LCD_3V3_PIN);
#endif

    pinmux[0].pin = LCD_RST_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(LCD_RST_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(LCD_RST_PIN);
    LCD_INFO("RST_PIN:%d ",LCD_RST_PIN);
#ifdef LCD_RST_PIN2
    pinmux[0].pin = LCD_RST_PIN2;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(LCD_RST_PIN2, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(LCD_RST_PIN2);
    LCD_INFO("RST_PIN2:%d ",LCD_RST_PIN2);
#endif
    hal_sys_timer_delay(MS_TO_TICKS(10));
    hal_gpio_pin_clr(LCD_RST_PIN);
#ifdef LCD_RST_PIN2
    hal_gpio_pin_clr(LCD_RST_PIN2);
#endif
    hal_sys_timer_delay(MS_TO_TICKS(1));
    hal_gpio_pin_set(LCD_RST_PIN);
#ifdef LCD_RST_PIN2
    hal_gpio_pin_set(LCD_RST_PIN2);
#endif
    hal_sys_timer_delay(MS_TO_TICKS(10));
    LCD_INFO("Reset panel: %s", LCD_NAME);
#endif
}

POSSIBLY_UNUSED
static int lcd_read_id(uint8_t port, uint32_t *id)
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
static int lcd_read_status(uint8_t port, uint8_t *st)
{
    int ret = 0;
    ret = lcdc_read_cmd(port, 0x0a, st, 1);
    if (ret < 0)
        return -1;
    return 0;
}

static int lcd_panel_init(uint8_t port)
{
#ifndef SIMU
    uint32_t id;
    uint8_t st;
    struct lcd_command *cmds = lcd_init_cmds;
    int count = sizeof(lcd_init_cmds) / sizeof(struct lcd_command);

    if (lcd_read_status(port, &st)) {
        LCD_INFO("Failed to read LCD power status.");
        return -1;
    }
    if (lcd_read_id(port, &id)) {
        LCD_INFO("Failed to read LCD id.");
        return -1;
    }
    LCD_INFO("LCD [%s]: status %02x, id %06x", LCD_NAME, st, id);

    for (int i = 0; i < count; i++) {
        if (cmds[i].param_cnt > 0)
            lcdc_send_cmd_list(port, cmds[i].cmd, cmds[i].param_cnt, cmds[i].param_data);
        else
            lcdc_send_cmd(port, cmds[i].cmd);
        if (cmds[i].delay_ms > 0)
            hal_sys_timer_delay(MS_TO_TICKS(cmds[i].delay_ms));
    }

    LCD_INFO("LCD [%s] is initialized.", LCD_NAME);
#endif
    return 0;
}

#if defined(CONFIG_CLKP_REPAIR_GPIO) && (CONFIG_CLKP_REPAIR_GPIO != 0xff)
static void dsi_clkp_repair_gpio(uint8_t dsi_id)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] =
    {
        {CONFIG_CLKP_REPAIR_GPIO, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(CONFIG_CLKP_REPAIR_GPIO, HAL_GPIO_DIR_OUT, 0);
    hal_dsi_start_hs_clock(dsi_id);
    hal_sys_timer_delay(US_TO_TICKS(1));
    hal_gpio_pin_set_dir(CONFIG_CLKP_REPAIR_GPIO, HAL_GPIO_DIR_IN, 0);
}
#endif

#if defined(CHIP_BEST2009)
#define DSI_FUNC_SEL_PIN   (HAL_GPIO_PIN_P0_6)
#define DSI_FUNC_OE_PIN    (HAL_GPIO_PIN_P0_7)

void display_dsi_mipi_sel_func(bool sel)
{
    hal_gpio_pin_set_dir(DSI_FUNC_SEL_PIN, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_set_dir(DSI_FUNC_OE_PIN, HAL_GPIO_DIR_OUT, 1);
    hal_sys_timer_delay(US_TO_TICKS(500));
    if (sel) {
        hal_gpio_pin_set(DSI_FUNC_SEL_PIN);
        hal_sys_timer_delay(US_TO_TICKS(250));

        hal_gpio_pin_clr(DSI_FUNC_OE_PIN);
        hal_sys_timer_delay(US_TO_TICKS(250));
    } else {
        hal_gpio_pin_clr(DSI_FUNC_SEL_PIN);
        hal_sys_timer_delay(US_TO_TICKS(250));

        hal_gpio_pin_clr(DSI_FUNC_OE_PIN);
        hal_sys_timer_delay(US_TO_TICKS(250));
    }
}
#endif

int board_lcd_initialize(void)
{
    LCD_INFO("[%s]: Start", __func__);
#if defined(CHIP_BEST2009)
    display_dsi_mipi_sel_func(0);
#endif

#ifdef CONFIG_LCDC_TE_IRQ_ENABLE
    if (lcdc_config.mode == LCDC_COMMAND_MODE && lcdc_config.te_pin < 0) {
#ifdef LCD_TE_PIN
        lcdc_config.te_pin = LCD_TE_PIN;
#else
        lcdc_config.te_pin = hal_iomux_get_dsi_te_pin();
#endif
    }
#endif

    /* LCD panel power up */
    lcd_power_up();

    lcdc_init(&lcdc_config);

#if defined(CONFIG_LCDC_DSI)
#if defined(CONFIG_CLKP_REPAIR_GPIO) && (CONFIG_CLKP_REPAIR_GPIO != 0xff)
    dsi_clkp_repair_gpio(lcdc_config.bus_port0);
#endif
#endif

    if (lcd_panel_init(lcdc_config.bus_port0)) {
        LCD_INFO("LCD0 init failed!");
    }
    if (lcdc_config.bus == LCDC_BUS_DUAL_DSI &&
        lcd_panel_init(lcdc_config.bus_port1)) {
        LCD_INFO("LCD1 init failed!");
    }

    LCD_INFO("[%s]: Done", __func__);
    return OK;
}

#define LCD_DISPLAY_OFF     0x28
void board_lcd_uninitialize(void)
{
    /* display off */
    lcdc_wakeup();
    lcdc_send_cmd(lcdc_config.bus_port0, LCD_DISPLAY_OFF);
    if (lcdc_config.bus == LCDC_BUS_DUAL_DSI)
        lcdc_send_cmd(lcdc_config.bus_port1, LCD_DISPLAY_OFF);
    hal_sys_timer_delay(MS_TO_TICKS(10));
}

