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
#ifndef __LVGL_DISP_CONF_H__
#define __LVGL_DISP_CONF_H__

#ifdef CONFIG_LCDC_DSI
//#define CONFIG_DSI_VIDEO_MODE
//#define CONFIG_LCDC_USE_DSI0
//#define CONFIG_LCDC_USE_DSI1
//#define CONFIG_DSI_2LANE_MODE
//#define CONFIG_DSI_4LANE_MODE
#endif
//#define CONFIG_LCDC_ALIGN_MIDDLE
//#define CONFIG_LCD_RGB565
//#define CONFIG_LCD_RGB888
#define CONFIG_LCD_SWAP_RB

/* Enable framebuffer compress.
 * 1: NON_SAMPLE, compress ratio is 2.0 for XRGB8888;
 * 2: H_SAMPLE, compress ratio is 2.6 for XRGB8888;
 */
//#define CONFIG_LCDC_COMPRESS    1

#include "lcd_conf.h"

// #define LCDC_WIDTH      ((LCD_WIDTH + 15) / 16 * 16)
#define LCDC_WIDTH      LCD_WIDTH  /// gpu no need align root 20260211
#define LCDC_HEIGHT     LCD_HEIGHT

#ifdef CONFIG_LCD_RGB565
#define LCDC_FORMAT     LCDC_CFMT_RGB565
#define LCDC_STRIDE     (LCDC_WIDTH * 2)
#elif defined(CONFIG_LCD_RGB888)
#define LCDC_FORMAT     LCDC_CFMT_RGB888
#define LCDC_STRIDE     (LCDC_WIDTH * 3)
#else
#define LCDC_FORMAT     LCDC_CFMT_RGBA8888
#define LCDC_STRIDE     (LCDC_WIDTH * 4)
#endif
#define LCDC_SIZE       (LCDC_STRIDE * LCDC_HEIGHT)

#ifdef CONFIG_LCDC_COMPRESS
#if !(CONFIG_LCDC_COMPRESS == 1) && !(CONFIG_LCDC_COMPRESS == 2)
#error "CONFIG_LCDC_COMPRESS must be 1 or 2."
#endif
#endif

#endif
