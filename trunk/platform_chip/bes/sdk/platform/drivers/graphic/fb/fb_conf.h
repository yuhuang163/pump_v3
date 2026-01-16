
/***************************************************************************
 *
 * Copyright 2015-2021 BES.
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
 * The FB driver default LCDC/LCD configurations.
 */

#ifndef __FB_CONF_H__
#define __FB_CONF_H__
/**
 *
 * LCDC Feature Configuration
 *
 */
#define CONFIG_LCDC_TE_IRQ_ENABLE
#define CONFIG_ENTER_SLEEP_NO_FRAMES
#define CONFIG_LCDC_SLEEP

#ifndef USE_GAMU_FB
#define CONFIG_FB_DOUBLE_BUFFER
#endif

#define CONFIG_FB_PANDISPLAY

#define CONFIG_FB_OVERLAY_BLIT
#define CONFIG_FB_OVERLAY

/**
 * Board  Configuration
 */

#ifdef CONFIG_DSI_VIDEO_MODE
#undef CONFIG_ULPS_MODE_SUPPORT
#undef CONFIG_LCDC_SLEEP
#undef CONFIG_DSI_SLEEP
#endif

#include "lcd_conf.h"

#if defined(PROGRAMMER)
#define BOARD_LCDC_WIDTH                160
#define BOARD_LCDC_HEIGHT               32
#else
#define BOARD_LCDC_WIDTH                LCD_WIDTH
#define BOARD_LCDC_HEIGHT               LCD_HEIGHT
#endif
#define BOARD_LCDC_HFP                  LCD_HFP
#define BOARD_LCDC_HBP                  LCD_HBP
#define BOARD_LCDC_VFP                  LCD_VFP
#define BOARD_LCDC_VBP                  LCD_VBP
#define BOARD_LCDC_HSYNC                LCD_HSYNC
#define BOARD_LCDC_VSYNC                LCD_VSYNC

#define BOARD_LCDC_GCR_DEN              1
#define BOARD_LCDC_GCR_DBW              5
#define BOARD_LCDC_GCR_DGW              6
#define BOARD_LCDC_GCR_DRW              5

#define CONFIG_LCDC_L1
#define CONFIG_LCDC_L1_ARGB8888
//#define CONFIG_LCDC_L2

/**
 * Feature
 */
//#define CONFIG_LCDC_GAMMA_ENABLE
//#define CONFIG_FB_CMAP
//#define CONFIG_FB_TRANSPARENCY

#define CONFIG_DSIPHY_OUTFMT_RGB888
//#define CONFIG_DSIPHY_OUTFMT_RGB565
//#define CONFIG_DSIPHY_OUTFMT_RGB666

#ifdef CONFIG_DSIPHY_OUTFMT_RGB565
#define CONFIG_LCD_RGB565
#endif

#ifdef CONFIG_DSIPHY_OUTFMT_RGB666
#define CONFIG_LCD_RGB666
#endif

#endif /* __FB_CONF_H__ */
