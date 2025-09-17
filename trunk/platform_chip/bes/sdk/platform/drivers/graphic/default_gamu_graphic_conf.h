
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

#ifndef _DEFAULT_GAMU_GRAPHIC_CONF_H
#define _DEFAULT_GAMU_GRAPHIC_CONF_H

#define DISPLAY_RM69330     1

//#define DISPLAY_GC9503NP    1
//#define DISPLAY_ST7701SN    1
//#define DISPLAY_ST7785M     1

#if defined(CHIP_BEST2003) || defined(CHIP_BEST1502X)
#define CONFIG_CLKP_REPAIR_GPIO 0xff //HAL_IOMUX_PIN_P3_7
#endif
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

#define CONFIG_GPIO_TE_IRQ
#define CONFIG_LCDC_TE_IRQ_ENABLE

#ifdef CHIP_BEST2003
#undef CONFIG_DSI_SLEEP
#endif

#define USE_GAMU_FB
#define CONFIG_FB_UPDATE

#ifndef CONFIG_FB_UPDATE
    #error "gamu solution needs CONFIG_FB_UPDATE defined"
#endif

#define CONFIG_FB_DOUBLE_BUFFER
#define CONFIG_FB_PANDISPLAY

#ifdef CONFIG_FB_PANDISPLAY
    #define CONFIG_LCDC_AUTOREFRESH_MODE
    #ifndef CONFIG_FB_DOUBLE_BUFFER
        #error "pandisplay mode needs CONFIG_FB_DOUBLE_BUFFER defined"
    #endif
#endif

#define CONFIG_FB_OVERLAY
#define CONFIG_FB_OVERLAY_BLIT

// #define CONFIG_LCDC_DITHER
// #define CONFIG_FB_CMAP
// #define CONFIG_FB_COLORDITHER
// #define CONFIG_FB_COLORADJCURVE
// #define CONFIG_FG_YUV_FMT

//#define CONFIG_FB_PANDISPLAY
// #define SIMU_TEST_CODE
// #define ONFIG_LCDC_L1_CHROMAKEYEN
// #define CONFIG_LCDC_L2_CHROMAKEYEN
// #define CONFIG_LCDC_CHROMAKEY=0x0000fe
// #define CONFIG_PM
#define CONFIG_FB_VMIRRO

/**
 * Board  Configuration
 */

/* lcdc write back test switch, should removed at release */
//#define  WB_TEST

#define BOARD_LCDC_WIDTH               512  //gpu width
#define BOARD_LCDC_HEIGHT              512
#define SCREEN_OFFSET                  29
#define SCREEN_WIDTH                   454 //screen
#define SCREEN_HEIGHT                  454

/*
 * PROGRAMMER, just used to verify the working status of dsi
*/
#if defined(PROGRAMMER)
#define SCREEN_OFFSET                  0
#define SCREEN_WIDTH                   64
#define SCREEN_HEIGHT                  64
#endif


#define BOARD_LCDC_HFP                  12
#define BOARD_LCDC_HBP                  10
#define BOARD_LCDC_VFP                  10
#define BOARD_LCDC_VBP                  10
#define BOARD_LCDC_HSYNC                10
#define BOARD_LCDC_VSYNC                2


#define BOARD_LCDC_GCR_DEN              1
#define BOARD_LCDC_GCR_DBW              5
#define BOARD_LCDC_GCR_DGW              6
#define BOARD_LCDC_GCR_DRW              5

// #define CONFIG_LCDC_FB_BPP (16)
#define CONFIG_LCDC_FB_BPP (24)
// #define CONFIG_LCDC_FB_BPP (32)

#define CONFIG_LCDC_FBWB_BPP            24
#define CONFIG_LCDC_FB_BPP              24

#ifdef CONFIG_FB_DOUBLE_BUFFER
#define CONFIG_LCDC_FB_SIZE         (BOARD_LCDC_HEIGHT*BOARD_LCDC_WIDTH*CONFIG_LCDC_FB_BPP/4)
#else
#define CONFIG_LCDC_FB_SIZE         (BOARD_LCDC_HEIGHT*BOARD_LCDC_WIDTH*CONFIG_LCDC_FB_BPP/8)
#endif

extern uint32_t get_fb_base(void);

#define CONFIG_LCDC_FB_BASE       get_fb_base()

#define CONFIG_LCDC_FBWB_BASE     (CONFIG_LCDC_FB_BASE +  \
                                  CONFIG_LCDC_FBWB_BPP/4 *BOARD_LCDC_WIDTH*BOARD_LCDC_HEIGHT)

#define CONFIG_LCDC_L1

#if (CONFIG_LCDC_FB_BPP == 24)
    #define CONFIG_LCDC_L1_RGB888
    #define CONFIG_LCDC_L2_RGB888
#elif (CONFIG_LCDC_FB_BPP == 16)
    #define CONFIG_LCDC_L1_RGB565
    #define CONFIG_LCDC_L2_RGB565
#else
    #define CONFIG_LCDC_L1_ARGB8888
    #define CONFIG_LCDC_L2_ARGB8888
#endif

//#define CONFIG_LCDC_LO_RGB888


#endif //_DEFAULT_GAMU_GRAPHIC_CONF_H
