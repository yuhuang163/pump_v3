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
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "lcdc_drv.h"

#include "fb_conf.h"
#include "fb_drv.h"
#include "fb_lcdc.h"

#include "gra_sem.h"

#include "lcd_cmds.h"

#define LCD_INFO(...)  TR_INFO( TR_MOD(TEST), "[LCD]"  __VA_ARGS__)
/* This structure describes the information of this module */

struct lcm_info
{
    uint16_t mid;
    uint16_t driver_version;
    uint16_t driver_id;
};

/* This structure describes the state of this driver */

struct panel_dev_s
{
    /* Publicly visible device structure */
    struct lcd_dev_s dev;

    /* Private LCD-specific information follows */

    uint8_t bpp;                /* Selected color depth */
    uint8_t power;              /* Current power setting */

    /* This is working memory allocated by the LCD driver for each LCD device
     * and for each color plane. This memory will hold one raster line of data.
     * The size of the allocated run buffer must therefore be at least
     * (bpp * xres / 8).  Actual alignment of the buffer must conform to the
     * bitwidth of the underlying pixel type.
     *
     * If there are multiple planes, they may share the same working buffer
     * because different planes will not be operate on concurrently.  However,
     * if there are multiple LCD devices, they must each have unique run
     * buffers.
     */

    uint16_t runbuffer[LCD_WIDTH];

    /* panel information */

    struct lcm_info lcm;
};

/****************************************************************************
 * Private Function Protototypes
 ****************************************************************************/
static struct panel_dev_s lcd;

/* Misc. Helpers */


/* LCD Data Transfer Methods */

static int panel_putrun(fb_coord_t row, fb_coord_t col,
                          const uint8_t *buffer, size_t npixels);
#ifndef CONFIG_LCD_NOGETRUN
static int panel_getrun(fb_coord_t row, fb_coord_t col,
                          uint8_t *buffer, size_t npixels);
#endif

/* LCD Configuration */

static int panel_getvideoinfo(struct lcd_dev_s *dev,
                                struct fb_videoinfo_s *vinfo);
static int panel_getplaneinfo(struct lcd_dev_s *dev,
                                unsigned int planeno,
                                struct lcd_planeinfo_s *pinfo);

/* LCD Specific Controls */

static int panel_getpower(struct lcd_dev_s *dev);
static int panel_setpower(struct lcd_dev_s *dev, int power);
static int panel_getcontrast(struct lcd_dev_s *dev);
static int panel_setcontrast(struct lcd_dev_s *dev,
                             unsigned int contrast);

/****************************************************************************
 * Private Data
 ****************************************************************************/

/****************************************************************************
 * Name:  panel_putrun
 *
 * Description:
 *   This method can be used to write a partial raster line to the LCD:
 *
 *   row     - Starting row to write to (range: 0 <= row < yres)
 *   col     - Starting column to write to (range: 0 <= col <= xres-npixels)
 *   buffer  - The buffer containing the run to be written to the LCD
 *   npixels - The number of pixels to write to the LCD
 *             (range: 0 < npixels <= xres-col)
 *
 ****************************************************************************/

static int panel_putrun(fb_coord_t row, fb_coord_t col,
                          const uint8_t *buffer, size_t npixels)
{
    LCD_INFO("[%s]: row: %d col: %d npixels: %d", __func__, row, col, npixels);
    return OK;
}

/****************************************************************************
 * Name:  panel_getrun
 *
 * Description:
 *   This method can be used to read a partial raster line from the LCD:
 *
 *  row     - Starting row to read from (range: 0 <= row < yres)
 *  col     - Starting column to read read (range: 0 <= col <= xres-npixels)
 *  buffer  - The buffer in which to return the run read from the LCD
 *  npixels - The number of pixels to read from the LCD
 *            (range: 0 < npixels <= xres-col)
 *
 ****************************************************************************/

#ifndef CONFIG_LCD_NOGETRUN
static int panel_getrun(fb_coord_t row, fb_coord_t col, uint8_t *buffer,
                         size_t npixels)
{
    LCD_INFO("[%s]: row: %d col: %d npixels: %d", __func__, row, col, npixels);
    return OK;
}
#endif

/****************************************************************************
 * Name:  panel_getvideoinfo
 *
 * Description:
 *   Get information about the LCD video controller configuration.
 *
 ****************************************************************************/

static int panel_getvideoinfo(struct lcd_dev_s *dev,
                                struct fb_videoinfo_s *vinfo)
{
    DEBUGASSERT(dev && vinfo);
    LCD_INFO("[%s]: not used", __func__);
    return OK;
}

/****************************************************************************
 * Name:  panel_getplaneinfo
 *
 * Description:
 *   Get information about the configuration of each LCD color plane.
 *
 ****************************************************************************/

static int panel_getplaneinfo(struct lcd_dev_s *dev,
                                unsigned int planeno,
                                struct lcd_planeinfo_s *pinfo)
{
    struct panel_dev_s *priv = (struct panel_dev_s *)dev;

    DEBUGASSERT(dev && pinfo && (planeno == 0 || planeno == 1));

    pinfo->putrun = panel_putrun;                  /* Put a run into LCD memory */
#ifndef CONFIG_LCD_NOGETRUN
    pinfo->getrun = panel_getrun;                  /* Get a run from LCD memory */
#endif
    pinfo->buffer = (uint8_t *)priv->runbuffer; /* Run scratch buffer */
    pinfo->bpp    = priv->bpp;                      /* Bits-per-pixel */
    return OK;
}

/****************************************************************************
 * Name:  panel_getpower
 ****************************************************************************/

static int panel_getpower(struct lcd_dev_s *dev)
{
    struct panel_dev_s *priv = (struct panel_dev_s *)dev;

    LCD_INFO("power: %d", priv->power);
    return priv->power;
}

/****************************************************************************
 * Name:  set_backlight
 ****************************************************************************/

static void set_backlight(struct lcd_dev_s *dev, int value)
{
    struct panel_dev_s *priv = (struct panel_dev_s *)dev;

    if (value > 0)
        priv->power = value & 0xff;
    else
        priv->power = 0;

    struct lcdc_config_t *lcdc_config;
    if (lcdc_get_config(&lcdc_config) < 0) {
        LCD_INFO("[%s]: get lcdc config failed", __func__);
    }

    if (lcdc_config) {
        lcdc_send_cmd_list(lcdc_config->bus_port0, 0x51, 1, &(priv->power));
#ifdef CONFIG_LCDC_DUAL_DSI
        if (lcdc_config->bus == LCDC_BUS_DUAL_DSI)
            lcdc_send_cmd_list(lcdc_config->bus_port1, 0x51, 1, &(priv->power));
#endif
    }
}

/****************************************************************************
 * Name:  panel_setpower
 ****************************************************************************/

static int panel_setpower(struct lcd_dev_s *dev, int power)
{
    set_backlight(dev, (power & 0xff));
    return OK;
}

/****************************************************************************
 * Name:  panel_getcontrast
 *
 * Description:
 *   Get the current contrast setting (0-CONFIG_LCD_MAXCONTRAST).
 *
 ****************************************************************************/

static int panel_getcontrast(struct lcd_dev_s *dev)
{
    LCD_INFO("Not implemented");
    return -ENOSYS;
}

/****************************************************************************
 * Name:  panel_setcontrast
 *
 * Description:
 *   Set LCD panel contrast (0-CONFIG_LCD_MAXCONTRAST).
 *
 ****************************************************************************/

static int panel_setcontrast(struct lcd_dev_s *dev,
                              unsigned int contrast)
{
    LCD_INFO("contrast: %d", contrast);
    return -ENOSYS;
}

/****************************************************************************
 * Private Data
 ****************************************************************************/

typedef struct board_lcddev_s
{
    struct lcd_dev_s *lcd;
    sem_t lock;
} board_lcddev_t;

static board_lcddev_t g_lcddev;
static bool dev_initialized = false;
/****************************************************************************
 * Name:  board_lcd_getdev
 *
 * Description:
 *   Return a a reference to the LCD object for the specified LCD.  This
 *   allows support for multiple LCD devices.
 *
 ****************************************************************************/

struct lcd_dev_s *board_lcd_getdev(int devno)
{
    if (!dev_initialized) {
        /* Initialize the driver data structure */

        lcd.dev.getvideoinfo = panel_getvideoinfo;
        lcd.dev.getplaneinfo = panel_getplaneinfo;
        lcd.dev.getpower     = panel_getpower;
        lcd.dev.setpower     = panel_setpower;
        lcd.dev.getcontrast  = panel_getcontrast;
        lcd.dev.setcontrast  = panel_setcontrast;

        g_lcddev.lcd = &lcd.dev;
        dev_initialized = true;
    }
    return g_lcddev.lcd;
}

int board_trylock_panel(void)
{
    return gra_sem_trywait(&g_lcddev.lock);
}

void board_unlock_panel(void)
{
    gra_sem_post(&g_lcddev.lock);
}

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
#ifdef SIMU
    return 0;
#endif
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
        LCD_INFO("LCD cmd: %02x", cmds[i].cmd);
        if (cmds[i].delay_ms > 0)
            hal_sys_timer_delay(MS_TO_TICKS(cmds[i].delay_ms));
    }

    LCD_INFO("LCD [%s] is initialized.", LCD_NAME);
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

static void display_dsi_mipi_sel_func(bool sel)
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

    /* LCD panel power up */
    lcd_power_up();

    struct lcdc_config_t *lcdc_config;
    if (lcdc_get_config(&lcdc_config) < 0) {
        LCD_INFO("[%s]: get lcdc config failed", __func__);
    }
#if defined(CONFIG_LCDC_DSI)
#if defined(CONFIG_CLKP_REPAIR_GPIO) && (CONFIG_CLKP_REPAIR_GPIO != 0xff)
    dsi_clkp_repair_gpio(lcdc_config->bus_port0);
#endif
#endif

    if (lcd_panel_init(lcdc_config->bus_port0)) {
        LCD_INFO("LCD0 init failed!");
    }
    if (lcdc_config->bus == LCDC_BUS_DUAL_DSI &&
        lcd_panel_init(lcdc_config->bus_port1)) {
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

    struct lcdc_config_t *lcdc_config;
    if (lcdc_get_config(&lcdc_config) < 0) {
        LCD_INFO("[%s]: get lcdc config failed", __func__);
    }

    lcdc_send_cmd(lcdc_config->bus_port0, LCD_DISPLAY_OFF);
    if (lcdc_config->bus == LCDC_BUS_DUAL_DSI)
        lcdc_send_cmd(lcdc_config->bus_port1, LCD_DISPLAY_OFF);
    hal_sys_timer_delay(MS_TO_TICKS(10));
}

int bes_lcdc_get_te_pin(void)
{
    return hal_iomux_get_dsi_te_pin();
}
