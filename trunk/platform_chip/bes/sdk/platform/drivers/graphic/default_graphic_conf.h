
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
 * DOC: The LCDC Board Related Configuration.
 * NOTE: For 1600 FPGA TEST ONLY
 */

#ifndef _DEFAULT_GRAPHIC_CONF_H
#define _DEFAULT_GRAPHIC_CONF_H

#define DISPLAY_RM69330     1

//#define DISPLAY_GC9503NP    1
//#define DISPLAY_ST7701SN    1
//#define DISPLAY_ST7785M     1
//#define DISPLAY_A064        1
//#define DISPLAY_HX8394F     1
//#define DISPLAY_ICNA3310    1

#ifdef CHIP_BEST1503
#define CONFIG_LCDC_QSPI
#else
#define CONFIG_LCDC_DSI
//#define CONFIG_DSI_VIDEO_MODE
#endif

#ifndef COLOR_BIT_DEPTH
#define COLOR_BIT_DEPTH 32
#endif

#if (COLOR_BIT_DEPTH==32)
#define CONFIG_LCD_RGB888
#else
#define CONFIG_LCD_RGB565
#endif

#include "lcd/lcd_conf.h"

/**
 *
 * LCDC Feature Configuration
 *
 */
#ifndef ohos_build
#define CONFIG_ULPS_MODE_SUPPORT
#define CONFIG_LCDC_SLEEP
#define CONFIG_DSI_SLEEP
#endif

#ifdef CHIP_BEST2003
#undef CONFIG_DSI_SLEEP
#endif

#define CONFIG_LCDC_TE_IRQ_ENABLE

#ifndef USE_GAMU_FB
#define CONFIG_FB_DOUBLE_BUFFER
#endif

//#define CONFIG_FB_PANDISPLAY

//#define CONFIG_LCDC_AUTOREFRESH_MODE

#define CONFIG_FB_OVERLAY_BLIT
#define CONFIG_FB_OVERLAY

// #define CONFIG_LCDC_DITHER
// #define CONFIG_FB_CMAP
// #define CONFIG_FB_COLORDITHER
// #define CONFIG_FB_COLORADJCURVE
// #define CONFIG_FG_YUV_FMT

#define CONFIG_FB_UPDATE
// #define SIMU_TEST_CODE
// #define ONFIG_LCDC_L1_CHROMAKEYEN
// #define CONFIG_LCDC_L2_CHROMAKEYEN
// #define CONFIG_LCDC_CHROMAKEY=0x0000fe
// #define CONFIG_PM
// #define CONFIG_FB_VMIRRO


/**
 * Board  Configuration
 */

/* lcdc write back test switch, should removed at release */
//#define  WB_TEST

#define SCREEN_WIDTH                    LCD_WIDTH /* width of lcd panel */
#define SCREEN_OFFSET                   0

#ifdef CONFIG_DSI_VIDEO_MODE
#undef CONFIG_ULPS_MODE_SUPPORT
#undef CONFIG_LCDC_SLEEP
#undef CONFIG_DSI_SLEEP
#endif

#if defined(PROGRAMMER)
#define BOARD_LCDC_WIDTH                160
#define BOARD_LCDC_HEIGHT               32
#else
#define BOARD_LCDC_WIDTH                ((LCD_WIDTH + 15) / 16 * 16) /* aligned to 16 pixels for GPU/LCDC */
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

#ifdef CONFIG_LCD_RGB565
#define CONFIG_LCDC_FB_BPP (16)
#else
// #define CONFIG_LCDC_FB_BPP (24)
#define CONFIG_LCDC_FB_BPP (32)
#endif

#if defined(CONFIG_FB_DOUBLE_BUFFER)
#define CONFIG_LCDC_FB_SIZE         (BOARD_LCDC_HEIGHT*BOARD_LCDC_WIDTH*CONFIG_LCDC_FB_BPP/4)
#else
#define CONFIG_LCDC_FB_SIZE         (BOARD_LCDC_HEIGHT*BOARD_LCDC_WIDTH*CONFIG_LCDC_FB_BPP/8)
#endif

#define CONFIG_LCDC_FBWB_BPP  32

#ifndef ohos_build
extern uint32_t get_fb_base(void);
#define CONFIG_LCDC_FB_BASE       get_fb_base()
#else
#define CONFIG_LCDC_FB_BASE       0
#endif

#define CONFIG_LCDC_FBWB_BASE     (CONFIG_LCDC_FB_BASE +  \
                                  CONFIG_LCDC_FBWB_BPP/4 *BOARD_LCDC_WIDTH*BOARD_LCDC_HEIGHT)

#define CONFIG_LCDC_L1
//#define CONFIG_LCDC_L2
//#define CONFIG_LCDC_LWB

#if (CONFIG_LCDC_FB_BPP == 16)
    #define CONFIG_LCDC_L1_RGB565
    #define CONFIG_LCDC_L2_RGB565
    #define CONFIG_DSIPHY_OUTFMT_RGB565
#elif (CONFIG_LCDC_FB_BPP == 24)
    #define CONFIG_LCDC_L1_RGB888
    #define CONFIG_LCDC_L2_RGB888
    #define CONFIG_DSIPHY_OUTFMT_RGB888
#else
    #define CONFIG_LCDC_L1_ARGB8888
    #define CONFIG_LCDC_L2_ARGB8888
    #define CONFIG_DSIPHY_OUTFMT_RGB888
#endif

//#define CONFIG_LCDC_L1_YUV422

//#define CONFIG_LCDC_LO_RGB565
#define CONFIG_LCDC_LO_RGB888


#endif //_DEFAULT_GRAPHIC_CONF_H
