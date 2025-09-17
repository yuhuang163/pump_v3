/***************************************************************************
 *
 * Copyright 2015-2023 BES.
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

#if defined(DSI_TEST)

#include "stdio.h"
#include "stdlib.h"
#include "string.h"
#include "cmsis.h"
#include "hal_timer.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_cache.h"
#include "hal_location.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "hal_cmu.h"
#include "hal_psc.h"
#include "hal_lcdc.h"
#include "hal_dsi.h"

#define LCD_RM69330
//#define LCD_RM69090
//#define LCD_ILI9881C
//#define LCD_GC9503NP

//#define TEST_VIDEO_MODE
//#define TEST_RGB565

#ifdef LCD_RM69330
#define LCD_WIDTH               454
#define LCD_HEIGHT              454
#ifdef TEST_VIDEO_MODE
#define LCD_HFP                 30
#define LCD_HBP                 30
#define LCD_VFP                 20
#define LCD_VBP                 20
#define LCD_HSYNC               1
#define LCD_VSYNC               1
#else /* for command mode */
#define LCD_HFP                 12
#define LCD_HBP                 10
#define LCD_VFP                 10
#define LCD_VBP                 10
#define LCD_HSYNC               10
#define LCD_VSYNC               2
#endif
#endif

#ifdef LCD_RM69090
#define LCD_WIDTH               368
#define LCD_HEIGHT              448
#define LCD_HFP                 12
#define LCD_HBP                 10
#define LCD_VFP                 10
#define LCD_VBP                 10
#define LCD_HSYNC               10
#define LCD_VSYNC               2
#endif

#ifdef LCD_ILI9881C
#define LCD_WIDTH               800
#define LCD_HEIGHT              800
#define LCD_HFP                 12
#define LCD_HBP                 10
#define LCD_VFP                 10
#define LCD_VBP                 10
#define LCD_HSYNC               10
#define LCD_VSYNC               2
#endif

#ifdef LCD_GC9503NP
#define LCD_WIDTH               480
#define LCD_HEIGHT              800
#define LCD_HFP                 80
#define LCD_HBP                 80
#define LCD_VFP                 24
#define LCD_VBP                 24
#define LCD_HSYNC               1
#define LCD_VSYNC               1
#define ENABLE_DSI_2LANE        1
#endif

#ifdef TEST_RGB565
#define LCDC_FORMAT             11
#define LCDC_STRIDE             (LCD_WIDTH * 2)
#else
#define LCDC_FORMAT             13
#define LCDC_STRIDE             (LCD_WIDTH * 4)
#endif
#define LCDC_SIZE               (LCDC_STRIDE*LCD_HEIGHT)

#ifdef TEST_VIDEO_MODE
#define LCD_HLW                 8
#define LCD_VLW                 4
#else
#define LCD_HLW                 0
#define LCD_VLW                 0
#endif
#define LCD_HACTIVE             LCD_WIDTH
#define LCD_VACTIVE             LCD_HEIGHT
#define LCD_VTOTAL              (LCD_VSYNC + LCD_VBP + LCD_VFP + \
                                 LCD_VACTIVE + LCD_VLW - 1)
#define LCD_HTOTAL              (LCD_HSYNC + LCD_HBP + LCD_HFP + \
                                 LCD_HACTIVE + LCD_HLW - 1)

#define LCDC_VSYNC_START        (LCD_VFP + LCD_HFP)
#define LCDC_VSYNC_END          (LCDC_VSYNC_START + 1)

#define NCOLORS 10

POSSIBLY_UNUSED
static uint32_t test_colors_RGBA8888[NCOLORS] = {
    0xffffffff, //white
    0xff000000, //black
    0xffff0000, //blue
    0xff00ff00, //green
    0xff0000ff, //red
    0xff90ee90, //light green
    0xffa52a2a, //brown
    0xffadd8e6, //light blue
    0xffee82ee, //violet
    0xff00ffff, //yellow
};

POSSIBLY_UNUSED
static uint16_t test_colors_RGB565[NCOLORS] = {
    0xffff, //white
    0x0000, //black
    0x001f, //red
    0x07e0, //green
    0xf800, //blue
    0x0011, //dark red
    0x8410, //gray
    0x06bf, //gold
    0x8010, //purple
    0x07ff, //yellow
};

#ifdef PSRAM_ENABLE
static uint8_t ALIGNED(64) frame_buf[LCDC_SIZE] PSRAM_NC_BSS_LOC;
#else
static uint8_t ALIGNED(64) frame_buf[LCDC_SIZE];
#endif

static uint32_t frame_start = 0;
static uint32_t frame_count = 0;
static bool framedone_irq_get = false;

/* TE irq */
static int te_gpio_pin = -1;
static bool te_irq_get = false;

static enum HAL_DSI_ID_E dsi_id = HAL_DSI_ID_0;
#ifdef TEST_VIDEO_MODE
static enum HAL_DSI_MODE_E dsi_mode = DSI_VIDEO_MODE;
#else
static enum HAL_DSI_MODE_E dsi_mode = DSI_COMMAND_MODE;
#endif
#ifdef ENABLE_DSI_2LANE
static enum HAL_DSI_LANE_E dsi_lane = DSI_LANE_2;
#else
static enum HAL_DSI_LANE_E dsi_lane = DSI_LANE_1;
#endif

/******************************** LCD Init ****************************************/

#define lcd_send_cmd_data(...)          hal_dsi_send_cmd_data(dsi_id, ##__VA_ARGS__)
#define lcd_send_cmd_list(...)          hal_dsi_send_cmd_list(dsi_id, ##__VA_ARGS__)
#define lcd_send_cmd(...)               hal_dsi_send_cmd(dsi_id, ##__VA_ARGS__)
#define lcd_read_cmd_data(...)          hal_dsi_read_cmd(dsi_id, ##__VA_ARGS__)

POSSIBLY_UNUSED
static int lcd_read_id(uint32_t *id)
{
    int ret = 0;
    uint8_t *ids = (uint8_t *)id;
    memset(id, 0, sizeof(uint32_t));
    ret = lcd_read_cmd_data(0xda, &ids[0], 1);
    if (ret < 0)
        return -1;
    ret = lcd_read_cmd_data(0xdb, &ids[1], 1);
    if (ret < 0)
        return -1;
    ret = lcd_read_cmd_data(0xdc, &ids[2], 1);
    if (ret < 0)
        return -1;
    return 0;
}

POSSIBLY_UNUSED
static int lcd_read_status(uint8_t *st)
{
    int ret = 0;
    ret = lcd_read_cmd_data(0x0a, st, 1);
    if (ret < 0)
        return -1;
    return 0;
}

#ifdef LCD_RM69330
POSSIBLY_UNUSED
static int rm69330_init(void)
{
    uint16_t hstart = 14;
    uint16_t hend = LCD_WIDTH-1+hstart;
    uint16_t vstart = 0;
    uint16_t vend = LCD_HEIGHT-1+vstart;
    uint8_t lcd_status = 0;
    uint32_t lcd_id = 0;

    if (lcd_read_status(&lcd_status) < 0) {
        TRACE(0, "Failed to read LCD power status.");
        return -1;
    }
    TRACE(0, "LCD power status: %02x", lcd_status);

    if (lcd_read_id(&lcd_id) < 0) {
        TRACE(0, "Failed to read LCD id.");
        return -1;
    }
    TRACE(0, "LCD id: 0x%06x", lcd_id);

    lcd_send_cmd_data(0xfe, 0x1, 0x0, 0x0, 0x0, 0x0);
#ifdef TEST_VIDEO_MODE
    lcd_send_cmd_data(0xc2, 0x1, 0x3, 0x0, 0x0, 0x0);
#else
    lcd_send_cmd_data(0x35, 0x1, 0x0, 0x0, 0x0, 0x0);
#endif
#ifdef TEST_RGB565
    lcd_send_cmd_data(0x3a, 0x1, 0x55, 0x0, 0x0, 0x0);
#else
    lcd_send_cmd_data(0x3a, 0x1, 0x77, 0x0, 0x0, 0x0);
#endif
    lcd_send_cmd_data(0x36, 0x1, 0x8, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x51, 0x1, 0xcf, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x2a, 4, hstart>>8, hstart&0xff, hend>>8, hend&0xff);
    lcd_send_cmd_data(0x2b, 4, vstart>>8, vstart&0xff, vend>>8, vend&0xff);
    lcd_send_cmd(0x11);
    hal_sys_timer_delay(MS_TO_TICKS(240));
    lcd_send_cmd(0x29);
    hal_sys_timer_delay(MS_TO_TICKS(150));

    TRACE(0, "RM69330 is initialized.");
    return 0;
}
#endif /* LCD_RM69330 */

#ifdef LCD_RM69090
static int rm69090_init(void)
{
    lcd_send_cmd_data(0xfe, 0x1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x35, 0x1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x51, 0x1, 0xcf, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x2a, 4, 0x0, 0x10, 0x01, 0x7f);
    lcd_send_cmd_data(0x2b, 4, 0x0, 0x0, 0x01, 0xbf);
    lcd_send_cmd(0x11);
    hal_sys_timer_delay(MS_TO_TICKS(240));
    lcd_send_cmd(0x29);
    hal_sys_timer_delay(MS_TO_TICKS(150));
    return 0;
}
#endif

#ifdef LCD_ILI9881C
static int ili9881c_init(void)
{
    // Set Power
    lcd_send_cmd_data(0xff, 3, 0x98, 0x81, 0x3, 0x0);
    lcd_send_cmd_data(0x01, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x02, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x03, 1, 0x73, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x04, 1, 0xd3, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x05, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x06, 1, 0xa, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x07, 1, 0xf, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x08, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x09, 1, 0x1, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x0a, 1, 0x1, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x0b, 1, 0x1, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x0c, 1, 0x1, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x0d, 1, 0x1, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x0e, 1, 0x1, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x0f, 1, 0x19, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x10, 1, 0x19, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x11, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x12, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x13, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x14, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x15, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x16, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x17, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x18, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x19, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x1a, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x1b, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x1c, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x1d, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x1e, 1, 0x40, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x1f, 1, 0x80, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x20, 1, 0x06, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x21, 1, 0x01, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x22, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x23, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x24, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x25, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x26, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x27, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x28, 1, 0x33, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x29, 1, 0x3, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x2a, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x2b, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x2c, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x2d, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x2e, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x2f, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x30, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x31, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x32, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x33, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x34, 1, 0x3, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x35, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x36, 1, 0x3, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x37, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x38, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x39, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x3a, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x3b, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x3c, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x3d, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x3e, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x3f, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x40, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x41, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x42, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x43, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x44, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x50, 1, 0x1, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x51, 1, 0x23, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x52, 1, 0x45, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x53, 1, 0x67, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x54, 1, 0x89, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x55, 1, 0xab, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x56, 1, 0x1, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x57, 1, 0x23, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x58, 1, 0x45, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x59, 1, 0x67, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x5a, 1, 0x89, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x5b, 1, 0xab, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x5c, 1, 0xcd, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x5d, 1, 0xef, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x5e, 1, 0x11, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x5f, 1, 0x8, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x60, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x61, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x62, 1, 0x1, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x63, 1, 0xd, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x64, 1, 0xc, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x65, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x66, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x67, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x68, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x69, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x6a, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x6b, 1, 0xf, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x6c, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x6d, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x6e, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x6f, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x70, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x71, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x72, 1, 0xe, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x73, 1, 0x6, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x74, 1, 0x7, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x75, 1, 0x8, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x76, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x77, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x78, 1, 0x1, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x79, 1, 0xd, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x7a, 1, 0xc, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x7b, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x7c, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x7d, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x7e, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x7f, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x80, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x81, 1, 0xf, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x82, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x83, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x84, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x85, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x86, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x87, 1, 0x2, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x88, 1, 0xe, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x89, 1, 0x6, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x8a, 1, 0x7, 0x0, 0x0, 0x0);

    lcd_send_cmd_data(0xff, 3, 0x98, 0x81, 0x1, 0x0);
    lcd_send_cmd_data(0x2e, 1, 0x50, 0x0, 0x0, 0x0);

    lcd_send_cmd_data(0xff, 3, 0x98, 0x81, 0x4, 0x0);
    lcd_send_cmd_data(0x6c, 1, 0x15, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x6e, 1, 0x2a, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x6f, 1, 0x33, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x3a, 1, 0xa4, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x8d, 1, 0x15, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x87, 1, 0xba, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x26, 1, 0x76, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xb2, 1, 0xd1, 0x0, 0x0, 0x0);

    lcd_send_cmd_data(0x00, 1, 0x80, 0x0, 0x0, 0x0);//set Lansel=1

    lcd_send_cmd_data(0xff, 3, 0x98, 0x81, 0x1, 0x0);
    lcd_send_cmd_data(0x22, 1, 0xa, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x53, 1, 0x57, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x55, 1, 0x61, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x50, 1, 0x9f, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x51, 1, 0x9a, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x31, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0x60, 1, 0xc, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xa0, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xa1, 1, 0x22, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xa2, 1, 0x32, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xa3, 1, 0x13, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xa4, 1, 0x17, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xa5, 1, 0x2b, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xa6, 1, 0x1f, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xa7, 1, 0x1f, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xa8, 1, 0xad, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xa9, 1, 0x1e, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xaa, 1, 0x29, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xab, 1, 0x9d, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xac, 1, 0x1d, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xad, 1, 0x1b, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xae, 1, 0x4f, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xaf, 1, 0x22, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xb0, 1, 0x2a, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xb1, 1, 0x5a, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xb2, 1, 0x67, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xb3, 1, 0x3f, 0x0, 0x0, 0x0);

    lcd_send_cmd_data(0xb7, 1, 0x3, 0x0, 0x0, 0x0);//set 2 Lane

    lcd_send_cmd_data(0xc0, 1, 0x0, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xc1, 1, 0x22, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xc2, 1, 0x32, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xc3, 1, 0x13, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xc4, 1, 0x17, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xc5, 1, 0x2b, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xc6, 1, 0x1f, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xc7, 1, 0x1f, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xc8, 1, 0xad, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xc9, 1, 0x1e, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xca, 1, 0x29, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xcb, 1, 0x9d, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xcc, 1, 0x1d, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xcd, 1, 0x1b, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xce, 1, 0x4f, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xcf, 1, 0x22, 0x0, 0x0, 0x0);

    lcd_send_cmd_data(0xd0, 1, 0x2a, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xd1, 1, 0x5a, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xd2, 1, 0x67, 0x0, 0x0, 0x0);
    lcd_send_cmd_data(0xd3, 1, 0x3f, 0x0, 0x0, 0x0);

    lcd_send_cmd_data(0xff, 3, 0x98, 0x81, 0x0, 0x0);
    lcd_send_cmd(0x11);
    hal_sys_timer_delay(MS_TO_TICKS(500));
    lcd_send_cmd(0x29);
    hal_sys_timer_delay(MS_TO_TICKS(50));
    return 0;
}
#endif

#ifdef LCD_GC9503NP
struct lcd_init_cmd {
    uint8_t cmd;
    uint8_t param_cnt;
    uint8_t param_data[64];
    uint8_t delay_ms;
};
static struct lcd_init_cmd gc9503np_cmds[] = {
    {0xF0, 5, {0x55, 0xAA, 0x52, 0x08, 0x00}, 0},
    {0xF6, 2, {0x5A, 0x87}, 0},
    {0xC1, 1, {0x3F}, 0},
    {0xC2, 1, {0x0E}, 0},
    {0xC6, 1, {0xF8}, 0},
    {0xC9, 1, {0x10}, 0},
    {0xCD, 1, {0x25}, 0},
    {0x86, 4, {0x88, 0xA3, 0xA3, 0x31}, 0},
    {0x87, 3, {0x04, 0x03, 0x66}, 0},
    {0xAC, 1, {0x65}, 0},
    {0xF8, 1, {0x8A}, 0},
    {0xA7, 1, {0x47}, 0},
    {0xA0, 1, {0xFF}, 0},
    {0xB1, 1, {0x04}, 0},
    {0xFA, 4, {0x08, 0x08, 0x08, 0x04}, 0},
    {0x71, 1, {0x48}, 0},
    {0x72, 1, {0x48}, 0},
    {0x73, 2, {0x00, 0x44}, 0},
    {0xA3, 1, {0xEE}, 0},
    {0xFD, 3, {0x3C, 0x3C, 0x00}, 0},
    {0x97, 1, {0xEE}, 0},
    {0x83, 1, {0x93}, 0},
    {0x77, 2, {0x00, 0x08}, 0},
    {0x9A, 1, {0x98}, 0},
    {0x9B, 1, {0x98}, 0},
    {0x82, 2, {0x6F, 0x6F}, 0},
    {0x80, 1, {0x0D}, 0},
    {0x7A, 2, {0x13, 0x1A}, 0},
    {0x7B, 2, {0x13, 0x1A}, 0},
    {0x6D, 32, {0x1E, 0x1D, 0x1D, 0x1D, 0x1D, 0x1E, 0x02, 0x1E, 0x19, 0x1E, 0x1A, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1A, 0x1E, 0x19, 0x1E, 0x01, 0x1E, 0x1D, 0x1D, 0x1D, 0x1D, 0x1E}, 0},
    {0x60, 8, {0x18, 0x08, 0x10, 0x70, 0x18, 0x07, 0x10, 0x70}, 0},
    {0x64, 16, {0x18, 0x06, 0x03, 0x22, 0x70, 0x03, 0x18, 0x05, 0x03, 0x23, 0x70, 0x03, 0x10, 0x70, 0x10, 0x70}, 0},
    {0x65, 16, {0x18, 0x04, 0x03, 0x23, 0x70, 0x03, 0x18, 0x03, 0x03, 0x23, 0x70, 0x03, 0x10, 0x70, 0x10, 0x70}, 0},
    {0x66, 16, {0x18, 0x02, 0x03, 0x23, 0x70, 0x03, 0x18, 0x01, 0x03, 0x23, 0x70, 0x03, 0x10, 0x70, 0x10, 0x70}, 0},
    {0x67, 16, {0x18, 0x00, 0x03, 0x23, 0x70, 0x03, 0x10, 0x01, 0x03, 0x23, 0x70, 0x03, 0x10, 0x70, 0x10, 0x70}, 0},
    {0x68, 13, {0x00, 0x08, 0x1D, 0x08, 0x1C, 0x00, 0x00, 0x08, 0x11, 0x08, 0x10, 0x00, 0x00}, 0},
    {0x69, 7, {0x14, 0x22, 0x14, 0x22, 0x14, 0x22, 0x08}, 0},
    {0x6A, 8, {0x08, 0x20, 0x48, 0x16, 0x01, 0x23, 0x45, 0x67}, 0},
    {0x6B, 1, {0xA7}, 0},
    {0xD1, 52, {0x00, 0x00, 0x00, 0x0D, 0x00, 0x25, 0x00, 0x3D, 0x00, 0x58, 0x00, 0x9D, 0x00, 0xDE, 0x01, 0x2E, 0x01, 0x68, 0x01, 0xB5, 0x01, 0xEA, 0x02, 0x3A, 0x02, 0x74, 0x02, 0x76, 0x02, 0xAB, 0x02, 0xE0, 0x03, 0x01, 0x03, 0x26, 0x03, 0x40, 0x03, 0x65, 0x03, 0x7E, 0x03, 0x9F, 0x03, 0xB2, 0x03, 0xCA, 0x03, 0xE4, 0x03, 0xFF}, 0},
    {0xD2, 52, {0x00, 0x00, 0x00, 0x05, 0x00, 0x11, 0x00, 0x23, 0x00, 0x3E, 0x00, 0x7D, 0x00, 0xB7, 0x01, 0x0D, 0x01, 0x45, 0x01, 0x93, 0x01, 0xCC, 0x02, 0x20, 0x02, 0x5D, 0x02, 0x5F, 0x02, 0x95, 0x02, 0xCB, 0x02, 0xEB, 0x03, 0x13, 0x03, 0x2F, 0x03, 0x58, 0x03, 0x73, 0x03, 0x97, 0x03, 0xAD, 0x03, 0xC6, 0x03, 0xE3, 0x03, 0xFF}, 0},
    {0xD3, 52, {0x00, 0x00, 0x00, 0x02, 0x00, 0x07, 0x00, 0x10, 0x00, 0x1A, 0x00, 0x53, 0x00, 0x98, 0x00, 0xF7, 0x01, 0x2F, 0x01, 0x7B, 0x01, 0xB4, 0x02, 0x01, 0x02, 0x3C, 0x02, 0x3E, 0x02, 0x6F, 0x02, 0xA2, 0x02, 0xBA, 0x02, 0xDC, 0x02, 0xF0, 0x03, 0x0D, 0x03, 0x22, 0x03, 0x42, 0x03, 0x5D, 0x03, 0x85, 0x03, 0xBC, 0x03, 0xFF}, 0},
    {0xD4, 52, {0x00, 0x00, 0x00, 0x0D, 0x00, 0x25, 0x00, 0x3D, 0x00, 0x58, 0x00, 0x9D, 0x00, 0xDE, 0x01, 0x2E, 0x01, 0x68, 0x01, 0xB5, 0x01, 0xEA, 0x02, 0x3A, 0x02, 0x74, 0x02, 0x76, 0x02, 0xAB, 0x02, 0xE0, 0x03, 0x01, 0x03, 0x26, 0x03, 0x40, 0x03, 0x65, 0x03, 0x7E, 0x03, 0x9F, 0x03, 0xB2, 0x03, 0xCA, 0x03, 0xE4, 0x03, 0xFF}, 0},
    {0xD5, 52, {0x00, 0x00, 0x00, 0x05, 0x00, 0x11, 0x00, 0x23, 0x00, 0x3E, 0x00, 0x7D, 0x00, 0xB7, 0x01, 0x0D, 0x01, 0x45, 0x01, 0x93, 0x01, 0xCC, 0x02, 0x20, 0x02, 0x5D, 0x02, 0x5F, 0x02, 0x95, 0x02, 0xCB, 0x02, 0xEB, 0x03, 0x13, 0x03, 0x2F, 0x03, 0x58, 0x03, 0x73, 0x03, 0x97, 0x03, 0xAD, 0x03, 0xC6, 0x03, 0xE3, 0x03, 0xFF}, 0},
    {0xD6, 52, {0x00, 0x00, 0x00, 0x02, 0x00, 0x07, 0x00, 0x10, 0x00, 0x1A, 0x00, 0x53, 0x00, 0x98, 0x00, 0xF7, 0x01, 0x2F, 0x01, 0x7B, 0x01, 0xB4, 0x02, 0x01, 0x02, 0x3C, 0x02, 0x3E, 0x02, 0x6F, 0x02, 0xA2, 0x02, 0xBA, 0x02, 0xDC, 0x02, 0xF0, 0x03, 0x0D, 0x03, 0x22, 0x03, 0x42, 0x03, 0x5D, 0x03, 0x85, 0x03, 0xBC, 0x03, 0xFF}, 0},
    {0x11, 0, {0x00}, 120},
    {0x6D, 32, {0x1E, 0x10, 0x0E, 0x0C, 0x0A, 0x1E, 0x02, 0x1E, 0x19, 0x1E, 0x1A, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1E, 0x1A, 0x1E, 0x19, 0x1E, 0x01, 0x1E, 0x09, 0x0B, 0x0D, 0x0F, 0x1E}, 0},
    {0x77, 2, {0x00, 0x00}, 0},
    {0xA6, 1, {0x56}, 0},
    {0x29, 0, {0x00}, 150},
};

static int gc9503np_init(void)
{
    uint8_t lcd_status = 0;
    uint32_t lcd_id = 0;

    if (lcd_read_status(&lcd_status) < 0) {
        TRACE(0, "Failed to read LCD power status.");
        return -1;
    }
    TRACE(0, "LCD power status: %02x", lcd_status);

    if (lcd_read_id(&lcd_id) < 0) {
        TRACE(0, "Failed to read LCD id.");
        return -1;
    }
    TRACE(0, "LCD id: 0x%06x", lcd_id);

    for (int i = 0; i < sizeof(gc9503np_cmds) / sizeof(struct lcd_init_cmd); i++) {
        if (gc9503np_cmds[i].param_cnt > 0)
            lcd_send_cmd_list(gc9503np_cmds[i].cmd, gc9503np_cmds[i].param_cnt, gc9503np_cmds[i].param_data);
        else
            lcd_send_cmd(gc9503np_cmds[i].cmd);
        if (gc9503np_cmds[i].delay_ms > 0)
            hal_sys_timer_delay(MS_TO_TICKS(gc9503np_cmds[i].delay_ms));
    }
    return 0;
}
#endif

#ifdef CHIP_BEST1600
#ifdef LCD_GC9503NP
#define GPIO_RST_PIN HAL_GPIO_PIN_PMU_P1_3
#else
#define GPIO_RST_PIN HAL_GPIO_PIN_P4_2
#endif
#define GPIO_1V8_PIN HAL_GPIO_PIN_PMU_P2_1
#define GPIO_2V8_PIN HAL_GPIO_PIN_PMU_P2_2
#define GPIO_3V3_PIN HAL_GPIO_PIN_PMU_P2_3
#elif defined(CHIP_BEST1700)
#define GPIO_RST_PIN HAL_GPIO_PIN_P4_2
#define GPIO_1V8_PIN HAL_GPIO_PIN_PMU_P2_1
#define GPIO_2V8_PIN HAL_GPIO_PIN_PMU_P2_2
#define GPIO_3V3_PIN HAL_GPIO_PIN_PMU_P2_3
#elif defined(CHIP_BEST1502X)
#define GPIO_RST_PIN HAL_GPIO_PIN_P8_0
#define GPIO_1V8_PIN HAL_GPIO_PIN_P9_1
#elif defined(CHIP_BEST1503)
#define GPIO_RST_PIN HAL_GPIO_PIN_P8_0
#define GPIO_1V8_PIN HAL_GPIO_PIN_P9_2
#else
#define GPIO_RST_PIN HAL_GPIO_PIN_P8_4
#endif

static int lcd_init(void)
{
    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] =
    {
        {(enum HAL_IOMUX_PIN_T)GPIO_RST_PIN, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
#ifdef SIMU
    return 0;
#endif

#ifdef GPIO_1V8_PIN
    pinmux[0].pin = (enum HAL_IOMUX_PIN_T)GPIO_1V8_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_1V8_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_1V8_PIN);
#endif
#ifdef GPIO_2V8_PIN
    pinmux[0].pin = (enum HAL_IOMUX_PIN_T)GPIO_2V8_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_2V8_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_2V8_PIN);
#endif
#ifdef GPIO_3V3_PIN
    pinmux[0].pin = (enum HAL_IOMUX_PIN_T)GPIO_3V3_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_3V3_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_3V3_PIN);
#endif
    pinmux[0].pin = (enum HAL_IOMUX_PIN_T)GPIO_RST_PIN;
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));
    hal_gpio_pin_set_dir(GPIO_RST_PIN, HAL_GPIO_DIR_OUT, 0);
    hal_gpio_pin_set(GPIO_RST_PIN);
#if defined(LCD_RM69330) || defined(LCD_GC9503NP)
    hal_sys_timer_delay(MS_TO_TICKS(100));
    hal_gpio_pin_clr(GPIO_RST_PIN);
    hal_sys_timer_delay(MS_TO_TICKS(5));
    hal_gpio_pin_set(GPIO_RST_PIN);
    hal_sys_timer_delay(MS_TO_TICKS(100));
#endif

#if defined(LCD_RM69330)
    return rm69330_init();
#elif defined(LCD_RM69090)
    return rm69090_init();
#elif defined(LCD_ILI9881C)
    return ili9881c_init();
#elif defined(LCD_GC9503NP)
    return gc9503np_init();
#endif
    return 0;
}

/*********************************** LCDC control **************************************/

static void lcdc_isr(void)
{
    uint32_t irqstat = hal_lcdc_get_irqstat();
    uint32_t dsistat = hal_dsi_get_irqstate(dsi_id);
    uint32_t dsiflags = hal_dsi_get_unov_flags(dsi_id);
    TRACE(0, "lcdc irq 0x%08x, dsi irq 0x%08x, dsi flags 0x%08x", irqstat, dsistat, dsiflags);
    if (hal_lcdc_irq_is_framedone(irqstat)) {
        hal_lcdc_framedone_irqstat_clr();
        framedone_irq_get = true;
#ifdef TEST_VIDEO_MODE
        TRACE(0, "frame %u transfer done, used %uus", frame_count,
                  FAST_TICKS_TO_US(hal_fast_sys_timer_get() - frame_start));
        frame_count++;
        frame_start = hal_fast_sys_timer_get();
#endif
    }
}

static int lcdc_wait_framedone_irq(void)
{
    TRACE(0, "wait for framedone irq...");
    while (!framedone_irq_get && FAST_TICKS_TO_MS(hal_fast_sys_timer_get()-frame_start) < 500)
        hal_sys_timer_delay_us(10);
    framedone_irq_get = false;
    if (FAST_TICKS_TO_MS(hal_fast_sys_timer_get()-frame_start) >= 500)
        return -1;
    return 0;
}

static void te_gpio_irq(enum HAL_GPIO_PIN_T pin)
{
    te_irq_get = true;
}

POSSIBLY_UNUSED
static void lcdc_wait_te_irq(void)
{
#ifdef SIMU
    return;
#endif
    TRACE(0, "wait for TE irq...");
    while (!te_irq_get)
        hal_sys_timer_delay_us(10);
    te_irq_get = false;
}

POSSIBLY_UNUSED
static void lcdc_te_irq_init(void)
{
    static const struct HAL_GPIO_IRQ_CFG_T gpiocfg = {
        .irq_enable = true,
        .irq_debounce = false,
        .irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING,
        .irq_handler = te_gpio_irq,
        .irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
    };

    te_gpio_pin = (enum HAL_GPIO_PIN_T)hal_iomux_get_dsi_te_pin();

    struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux[] =
    {
        {(enum HAL_IOMUX_PIN_T)te_gpio_pin, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
    };
    hal_iomux_init(pinmux, ARRAY_SIZE(pinmux));

    hal_gpio_pin_set_dir(te_gpio_pin, HAL_GPIO_DIR_IN, 0);
    hal_gpio_setup_irq(te_gpio_pin, &gpiocfg);
}

POSSIBLY_UNUSED
static void lcdc_te_irq_free(void)
{
    static const struct HAL_GPIO_IRQ_CFG_T gpiocfg = {
        .irq_enable = false,
        .irq_debounce = false,
        .irq_polarity = HAL_GPIO_IRQ_POLARITY_HIGH_RISING,
        .irq_handler = NULL,
        .irq_type = HAL_GPIO_IRQ_TYPE_EDGE_SENSITIVE,
    };

    hal_gpio_setup_irq(te_gpio_pin, &gpiocfg);
}

static void lcdc_dsi_init(void)
{
    int irq = hal_lcdc_irqn();

#ifdef TEST_RGB565
    hal_dsi_init(dsi_id, LCD_WIDTH, dsi_mode, dsi_lane, DSI_RGB565);
#else
    hal_dsi_init(dsi_id, LCD_WIDTH, dsi_mode, dsi_lane, DSI_RGB888);
#endif

    if (lcd_init())
        ASSERT(0, "LCD init failed.");

    hal_lcdc_disp_pn_reset();

#ifdef TEST_VIDEO_MODE
    hal_lcdc_video_mode_enable(true);
#else
    hal_lcdc_smpn_enable();
#endif

    hal_lcdc_stby_vsync_set(0x240, 1, 5);
    hal_lcdc_porch_vsync_set(32, 32, 5);
    hal_lcdc_pix_vsync_set(30, 31);

    /* Configure LCDC HBP/HFP/VBP/VFP/HTOTAL/VTOTAL */
    hal_lcdc_bgl_hporch_set(LCD_HBP, LCD_HFP);
    hal_lcdc_bgl_vporch_set(LCD_VBP, LCD_VFP);
    hal_lcdc_bgl_vhtotal_set(LCD_HTOTAL, LCD_VTOTAL);
#ifdef TEST_VIDEO_MODE
    /* vsync pulse */
    hal_lcdc_lvsepxcount(LCDC_VSYNC_START, LCDC_VSYNC_END);
#endif

    /* Layer SPU */
    hal_lcdc_lsize(LCDC_LAYER_SPU, LCD_WIDTH, LCD_HEIGHT);
    hal_lcdc_lcolor(LCDC_LAYER_SPU, 0);
#ifndef TEST_VIDEO_MODE
    hal_lcdcl_enable(LCDC_LAYER_SPU, true);
#endif

#ifdef TEST_RGB565
    hal_lcdc_smpn_mode_set(5);
#else
    hal_lcdc_smpn_mode_set(3);
#endif

    hal_lcdc_disp_mem_enable(true);

    hal_lcdc_panelpath_dmatrans_trigger_enable(LCDC_LAYER_LFORE, true);

    hal_lcdc_set_sync_line_pixs(5);
    hal_lcdc_set_sync_lines(0);

    hal_lcdc_smpn_vsync_delaymode_set(1);

    hal_lcdc_disp_extra_delay(0);
    hal_lcdc_sl_dmaburst_length_set(0x0f);
    hal_lcdc_ol_dmaburst_length_set(8);

    /* Configure dither */
    hal_lcdc_dither_s4x8(true);
    hal_lcdc_dither(false, 1, 0, 0, 0);

#ifdef TEST_RGB565
    hal_lcdc_disp_color_fmt(DSI_RGB565);
#else
    hal_lcdc_disp_color_fmt(DSI_RGB888);
#endif
    /* Configure dsi byte order as rgb */
    hal_lcdc_disp_dsi_data_swap(DSI_BD_RGB);

    /* Configure enable axi bus arbiter fast mode */
    hal_lcdc_axifastmode_enable(true);

    hal_lcdc_disp_avsync_trigger_disable(true);
    hal_lcdc_disp_svsync_trigger_disable(true);

    /* Layer Foreground (0) */
    hal_lcdc_lstartaddr(LCDC_LAYER_LFORE, (uint32_t)frame_buf);
    hal_lcdc_lsize(LCDC_LAYER_LFORE, LCD_WIDTH, LCD_HEIGHT);
    hal_lcdc_lpitch(LCDC_LAYER_LFORE, LCDC_STRIDE);
    hal_lcdc_lzoom_set(LCDC_LAYER_LFORE, LCD_WIDTH, LCD_HEIGHT);
#ifdef TEST_VIDEO_MODE
    hal_lcdc_sponscreen_set(LCDC_LAYER_LFORE, 0, 0);
#else
    hal_lcdc_sponscreen_set(LCDC_LAYER_LFORE, 1, 1);
#endif
    hal_lcdc_lpfc_fmt(LCDC_LAYER_LFORE, LCDC_FORMAT);
    hal_lcdc_lpfc_ltransp(LCDC_LAYER_LFORE, 0, 0);
    hal_lcdcl_enable(LCDC_LAYER_LFORE, true);
    hal_lcdc_select_top(LCDC_LAYER_LFORE);

    /* lcdc framedone irq */
    hal_lcdc_irq_disable();
    NVIC_SetVector((IRQn_Type)irq, (uint32_t) lcdc_isr);
    NVIC_SetPriority((IRQn_Type)irq, IRQ_PRIORITY_NORMAL);
    NVIC_ClearPendingIRQ((IRQn_Type)irq);
    NVIC_EnableIRQ((IRQn_Type)irq);

#ifndef TEST_VIDEO_MODE
    /* lcd te irq */
    lcdc_te_irq_init();

    /* enable frame transfer */
    hal_lcdc_gen_frame_enable(true);
#else
    /* enable video mode frame auto-transfer */
    hal_lcdcl_dumb_enable(true);
    hal_lcdc_dumb_vsync_mode(2);
    hal_lcdc_source_switch(true);
#endif

    hal_lcdc_framedone_irq_enable(true);

    frame_count = 0;
    hal_sys_timer_delay(MS_TO_TICKS(100));
    TRACE(0, "lcdc initialized.");
}

POSSIBLY_UNUSED
static void lcdc_dsi_exit(void)
{
    int irq = hal_lcdc_irqn();

    hal_lcdc_framedone_irq_enable(false);
    hal_lcdc_gen_frame_enable(false);

    hal_lcdc_irq_disable();
    NVIC_ClearPendingIRQ((IRQn_Type)irq);
    NVIC_DisableIRQ((IRQn_Type)irq);

#ifndef TEST_VIDEO_MODE
    lcdc_te_irq_free();
#endif

    hal_dsi_exit(dsi_id);
}

static void lcdc_dsi_sleep(void)
{
    hal_dsi_enter_ulps_mode(dsi_id);
    hal_dsi_sleep(dsi_id);
    hal_lcdc_sleep();
}

static void lcdc_dsi_wakeup(void)
{
    hal_lcdc_wakeup();
    hal_dsi_wakeup(dsi_id);
    hal_dsi_exit_ulps_mode(dsi_id);
}

POSSIBLY_UNUSED
static int lcdc_frame_send(void *framebuffer)
{
    TRACE(0, "frame %u transfer start", frame_count);

    lcdc_dsi_wakeup();

    hal_lcdc_lstartaddr(0, (uint32_t)framebuffer);

    lcdc_wait_te_irq();

    hal_dsi_start_hs_clock(dsi_id);
    hal_lcdc_gen_start();

    frame_start = hal_fast_sys_timer_get();

    if (lcdc_wait_framedone_irq()) {
        TRACE(0, "frame transfer failed!");
        return -1;
    }

    TRACE(0, "frame %u transfer done, used %uus", frame_count,
                FAST_TICKS_TO_US(hal_fast_sys_timer_get()-frame_start));
    frame_count++;

    lcdc_dsi_sleep();
    return 0;
}

#define TEST_COLOR_BAR

#ifdef TEST_COLOR_BAR
POSSIBLY_UNUSED
static void make_color_bar(uint8_t *buf, uint16_t width, uint16_t height, bool vertical)
{
#ifdef TEST_RGB565
    uint16_t *color_bar = test_colors_RGB565;
    uint16_t *p = (uint16_t *)buf;
#else
    uint32_t *color_bar = test_colors_RGBA8888;
    uint32_t *p = (uint32_t *)buf;
#endif
    int bar_w = width / NCOLORS;
    int bar_h = height / NCOLORS;
    for (int row = 0; row < height; row++) {
        for (int col = 0; col < width; col++) {
            int n;
            if (vertical)
                n = col / bar_w;
            else
                n = row / bar_h;
            *p++ = color_bar[n];
        }
    }
}
#else
POSSIBLY_UNUSED
static void fill_color(uint8_t *buf, uint16_t width, uint16_t height, int index)
{
#ifdef TEST_RGB565
    uint16_t *p = (uint16_t *)buf;
    uint16_t color = test_colors_RGB565[index % NCOLORS];
#else
    uint32_t *p = (uint32_t *)buf;
    uint32_t color = test_colors_RGBA8888[index % NCOLORS];
#endif
    for (int i = 0; i < (width * height); i++)
        p[i] = color;
}
#endif

void dsi_test(void)
{
    hal_sysfreq_req(HAL_SYSFREQ_USER_INIT, HAL_CMU_FREQ_208M);
    TRACE(0, "CPU freq: %u", hal_sys_timer_calc_cpu_freq(5, 0));

    TRACE(0, "dsi_lcdc_test");

    lcdc_dsi_init();

    for (uint32_t i = 0; ;i++) {
#ifdef TEST_COLOR_BAR
        make_color_bar(frame_buf, LCD_WIDTH, LCD_HEIGHT, (i/10) % 2);
#else
        fill_color(frame_buf, LCD_WIDTH, LCD_HEIGHT, (i/10)%10);
#endif
#ifndef TEST_VIDEO_MODE
        lcdc_frame_send(frame_buf);
#endif
        hal_sys_timer_delay(MS_TO_TICKS(100));
    }
    while (1);
}

#endif
