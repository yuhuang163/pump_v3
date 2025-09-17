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
 * DOC:The Display Controller operates in Slave mode and DMA mode.
 *
 * Slave mode is for the processor to read/write Smart Panel, and DMA mode is for
 *  the Display Controller to move pixels to either the Smart Panel or the Dumb Panels.
 * There are total of two layers that can be overlaid onto the display screen,
 *
 * The following operations will be done during DMA transfer:

 * image enhancement including contrast/saturation/brightness/hue adjust on the
 * two fore/back ground layes
 * palette color lookup on the two graphic paths, supporting both 4-bit/pixel and
 *  8-bit/pixel.
 * alpha blending in overlaid area for all paths, with configured alpha, alpha
 * combined with pixel and color key alpha.
 * partial display DMA when part of the picture is covered by another picture,
 * reducing bandwidth (refer to register bit descriptions for more information).

 * RGB gamma correction after alpha blending, up to two interfaces.
 * horizontal and vertical scaling and filtering on all of the graphic paths.
 * selec vertical mirror on all of the graphic objects.
 * dither function enable.
 *
 * The base addresses of the Display Controller registers is: 0x501c_0000.

 * LCDC hardware blocks:
 *
 *  (FGL source color fmt:RGB565,RGB1555,RGB888packed, RGB888unpacked, ARGB,YUV422packed,
 *   YUV422 planar, YUV420 planar, Palette color 4-bit per pixel, Palette color
 *   8-bit per pixel, RGBA )
 * (BGL source color fmt:RGB565,RGB1555,RGB888 packed,RGB888 unpacked,ARGB,YUV422
 *  packed,Palette color 4-bit per pixel,Palette color 8-bit per pixel,RGB8A )
 *  (OL color fmt:RGB565 RGB888 )
 *
 *  |FG Layer(panel video dma layer) |-->|
 *                                       | Alpha_FG (pix/c)
 *                                       V
 *                                      |BLENDING|-->|Smart Panel|-|dither|->->|dsi|
 *                                       ^     Alpha_O (pix/c)               |
 *                                       | Alpha_BG (pix/c)                  |
 *  |BG Layer(panel graphic dma layer)|->|             ^                     |
 *                                                     |                     |
 *  |TV Path(not used currently by lcdc driver)|-------|                     |
 *                                                                           |
 *                                                                           |
 *  |write back layer)|<------------------------------------------------------
 */

#include <stdio.h>
#include "string.h"
#include "cmsis.h"
#if defined(__NuttX__)
#include <nuttx/config.h>
#include <nuttx/video/fb.h>
#include <arch/board/board.h>
#include <nuttx/lcd/lcd.h>
#include <nuttx/irq.h>
#include <nuttx/cache.h>
#include <debug.h>
#include <nuttx/power/pm.h>
#if defined(CONFIG_PM)
#include <arch/board/pm_domain.h>
#endif
#define TC_INIT
#define TC_START
#define TC_END
#define TC_REP(s)
#else
#include "lcdc_fb.h"
#ifdef CONFIG_PM
#include "pm.h"
#endif
#endif
#include "hal_timer.h"
#include "lcdc.h"
#include "hal_lcdc.h"
#include "hal_dsi.h"
#include "hal_psc.h"
#include "hal_cache.h"
#include "cmsis_nvic.h"
#include "hal_cmu.h"
#include "hal_sysfreq.h"
#include "hal_trace.h"
#include "gra_sem.h"
#include "lpcmd.h"
#include "hal_location.h"
#include "hal_timer.h"
#include "hwtimer_list.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif

#if !defined(__NuttX__)
#include "graphic_conf.h"
#endif

#include "lcdc_drv.h"

#if defined(CONFIG_FBMEM_CACHED)
#define LCDC_DRV_WITH_CACHEOPS
#endif

static void fb_ratate_init(uint32_t lcd_x,uint32_t lcd_y);

#if defined(CONFIG_A7_DSP_BOOT_EARLY) && defined(CONFIG_FB_STATIC_BUFFER)
#define SUPPORT_REPLACE_FBMEM
#endif

#ifdef SUPPORT_REPLACE_FBMEM
#define ROTATE_USE_CPU(fbmem)   (fbmem != (void *)0x50000000)
#else
#ifdef CONFIG_ROTATE_CPU
#define ROTATE_USE_CPU(fbmem)   (true)
#else
#define ROTATE_USE_CPU(fbmem)   (false)
#endif
#endif

#if defined(CONFIG_ROTATE_CPU) && !defined(CONFIG_FB_DOUBLE_BUFFER)
#define ROTATE_DIRECT
#endif

#if defined(ROTATE_DIRECT) && defined(CONFIG_DSI_VIDEO_MODE)
#undef CONFIG_LCDC_TE_IRQ_ENABLE
#endif

#if defined(CONFIG_VIDEO_FB)
#if !(defined(CONFIG_LCDC_L1) || defined(CONFIG_LCDC_L2))
#define CONFIG_LCDC_L1_ARGB8888
#endif
#endif

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

#if defined(CHIP_BEST2003_DSP)
#undef SIMU
#endif

#if defined(SIMU) || defined(CONFIG_LCDC_TE_IRQ_ENABLE) && (defined(CONFIG_DSI_VIDEO_MODE) || defined(CONFIG_DSI_SLEEP) || defined(CONFIG_LCDC_SLEEP))
#define CONFIG_GPIO_TE_IRQ
#if defined(CONFIG_LCDC_SLEEP)
#define CONFIG_TRIGGER_FRAME_BY_SW
#endif
#endif

/**
 * Board LCDC Configuration
 */

#define LCDC_HEIGHT                 BOARD_LCDC_HEIGHT
#define LCDC_WIDTH                  BOARD_LCDC_WIDTH

#define CONFIG_LCDC_DITHER_RED     BOARD_LCDC_GCR_DRW

#define CONFIG_LCDC_DITHER_GREEN   BOARD_LCDC_GCR_DGW

#define CONFIG_LCDC_DITHER_BLUE    BOARD_LCDC_GCR_DBW

#define CONFIG_LCDC_L1_WIDTH BOARD_LCDC_WIDTH

#define CONFIG_LCDC_L2_WIDTH BOARD_LCDC_WIDTH

#define CONFIG_LCDC_L1_HEIGHT BOARD_LCDC_HEIGHT

#define CONFIG_LCDC_L2_HEIGHT BOARD_LCDC_HEIGHT

#define LCDC_LxWHPCR_WHSTPOS  (BOARD_LCDC_HSYNC + BOARD_LCDC_HBP - 1)

#define LCDC_LxWHPCR_WHSPPOS  (BOARD_LCDC_HSYNC + BOARD_LCDC_HBP + \
                                    LCDC_WIDTH - 1)

#define LCDC_LxWVPCR_WVSTPOS  (BOARD_LCDC_VSYNC + BOARD_LCDC_VBP - 1)

#define LCDC_LxWVPCR_WVSPPOS  (BOARD_LCDC_VSYNC + BOARD_LCDC_VBP + \
                                    LCDC_HEIGHT - 1)

#define LCDC_BPCR_AVBP        (LCDC_LxWVPCR_WVSTPOS)

#define LCDC_BPCR_AHBP        (LCDC_LxWHPCR_WHSTPOS)

#define LCDC_TWCR_PROC_TOTALH      (BOARD_LCDC_VSYNC + \
                                    BOARD_LCDC_VBP + \
                                    BOARD_LCDC_VFP - 1)
#define LCDC_TWCR_PROC_TOTALW      (BOARD_LCDC_HSYNC + \
                                    BOARD_LCDC_HBP + \
                                    BOARD_LCDC_HFP - 1)

#define LCDC_TWCR_TOTALH      (LCDC_TWCR_PROC_TOTALH + LCDC_HEIGHT)
#define LCDC_TWCR_TOTALW      (LCDC_TWCR_PROC_TOTALW + LCDC_WIDTH)

/* Synchronisation and Polarity */

#define LCDC_GCR_PCPOL        (BOARD_LCDC_GCR_PCPOL)
#define LCDC_GCR_DEPOL        (BOARD_LCDC_GCR_DEPOL)

#define LCDC_GCR_VSPOL        (BOARD_LCDC_GCR_VSPOL)
#define LCDC_GCR_HSPOL        (BOARD_LCDC_GCR_HSPOL)

/* Dither */

#define LCDC_GCR_DEN          (BOARD_LCDC_GCR_DEN)
#define LCDC_GCR_DBW          (BOARD_LCDC_GCR_DBW)
#define LCDC_GCR_DGW          (BOARD_LCDC_GCR_DGW)
#define STN32_LCDC_GCR_DRW    (BOARD_LCDC_GCR_DRW)

#define LCDC_LIPCR_LIPOS      (LCDC_TWCR_TOTALW)

/**
 * LCDC Layer Configuration
 */

#ifndef CONFIG_LCDC_DEFBACKLIGHT
#define CONFIG_LCDC_DEFBACKLIGHT 0xf0
#endif
#define LCDC_BACKLIGHT_OFF 0x00

/* Color/video formats */

/* Layer 1 format */

#if defined(CONFIG_LCDC_L1_L8)
#define LCDC_L1_BPP         8
#define LCDC_L1_COLOR_FMT   FB_FMT_RGB8
#define LCDC_L1PFCR_PF      LCDC_L1_COLOR_FMT
#define LCDC_L1CMAP
#elif defined(CONFIG_LCDC_L1_RGB565)
#define LCDC_L1_BPP         16
#define LCDC_L1_COLOR_FMT   FB_FMT_RGB16_565
#define LCDC_L1PFCR_PF      FB_FMT_RGB16_565
#elif defined(CONFIG_LCDC_L1_RGB888)
#define LCDC_L1_BPP         24
#define LCDC_L1_COLOR_FMT   FB_FMT_RGB24
#define LCDC_L1PFCR_PF      LCDC_L1_COLOR_FMT
#elif defined(CONFIG_LCDC_L1_ARGB8888)
#define LCDC_L1_BPP         32
#define LCDC_L1_COLOR_FMT   FB_FMT_RGB32
#define LCDC_L1PFCR_PF      LCDC_L1_COLOR_FMT
#elif defined(CONFIG_LCDC_L1_YUV422)
#define LCDC_L1_BPP         16
#define LCDC_L1_COLOR_FMT   FB_FMT_YUVP
#define LCDC_L1PFCR_PF      LCDC_L1_COLOR_FMT
#else
#error "LCDC pixel format not supported"
#endif

/* Layer 2 format */

#ifdef CONFIG_LCDC_L2
#if defined(CONFIG_LCDC_L2_L8)
#define LCDC_L2_BPP         8
#define LCDC_L2_COLOR_FMT   FB_FMT_RGB8
#define LCDC_L2PFCR_PF      LCDC_L2_COLOR_FMT
#define LCDC_L2CMAP
#elif defined(CONFIG_LCDC_L2_RGB565)
#define LCDC_L2_BPP         16
#define LCDC_L2_COLOR_FMT   FB_FMT_RGB16_565
#define LCDC_L2PFCR_PF      LCDC_L2_COLOR_FMT
#elif defined(CONFIG_LCDC_L2_RGB888)
#define LCDC_L2_BPP         24
#define LCDC_L2_COLOR_FMT   FB_FMT_RGB24
#define LCDC_L2PFCR_PF      LCDC_L2_COLOR_FMT
#elif defined(CONFIG_LCDC_L2_ARGB8888)
#define LCDC_L2_BPP         32
#define LCDC_L2_COLOR_FMT   FB_FMT_RGB32
#define LCDC_L2PFCR_PF      LCDC_L2_COLOR_FMT
#else
#error "LCDC pixel format not supported"
#endif
#endif /* CONFIG_LCDC_L2 */

/* Framebuffer sizes in bytes */

#if LCDC_L1_BPP == 8
#define LCDC_L1_STRIDE      (LCDC_WIDTH)
#elif LCDC_L1_BPP == 16
#define LCDC_L1_STRIDE      ((LCDC_WIDTH * 16 + 7) / 8)
#elif LCDC_L1_BPP == 24
#define LCDC_L1_STRIDE      ((LCDC_WIDTH * 24 + 7) / 8)
#elif LCDC_L1_BPP == 32
#define LCDC_L1_STRIDE      ((LCDC_WIDTH * 32 + 7) / 8)
#else
#error Undefined or unrecognized base resolution
#endif

#ifndef CONFIG_LCDC_FB_BPP
#define CONFIG_LCDC_FB_BPP LCDC_L1_BPP
#endif
/* LCDC only supports 8 bit per pixel overal */

#define LCDC_Lx_BYPP(n)       ((n) / 8)

#if defined(CONFIG_FB_DOUBLE_BUFFER)
#define LCDC_L1_FBSIZE        (LCDC_L1_STRIDE * LCDC_HEIGHT *2)
#else
#define LCDC_L1_FBSIZE        (LCDC_L1_STRIDE * LCDC_HEIGHT)
#endif

#ifdef CONFIG_LCDC_L2
#ifndef CONFIG_LCDC_L2_WIDTH
#define CONFIG_LCDC_L2_WIDTH LCDC_WIDTH
#endif

#if CONFIG_LCDC_L2_WIDTH > LCDC_WIDTH
#error Width of Layer 2 exceeds the width of the display
#endif

#ifndef CONFIG_LCDC_L2_HEIGHT
#define CONFIG_LCDC_L2_HEIGHT LCDC_HEIGHT
#endif

#if CONFIG_LCDC_L2_HEIGHT > LCDC_HEIGHT
#error Height of Layer 2 exceeds the height of the display
#endif

#if LCDC_L2_BPP == 8
#define LCDC_L2_STRIDE    (CONFIG_LCDC_L2_WIDTH)
#elif LCDC_L2_BPP == 16
#define LCDC_L2_STRIDE    ((CONFIG_LCDC_L2_WIDTH * 16 + 7) / 8)
#elif LCDC_L2_BPP == 24
#define LCDC_L2_STRIDE    ((CONFIG_LCDC_L2_WIDTH * 24 + 7) / 8)
#elif LCDC_L2_BPP == 32
#define LCDC_L2_STRIDE    ((CONFIG_LCDC_L2_WIDTH * 32 + 7) / 8)
#else
#error Undefined or unrecognized base resolution
#endif

#if defined(CONFIG_FB_DOUBLE_BUFFER)
#define LCDC_L2_FBSIZE        (LCDC_L2_STRIDE * CONFIG_LCDC_L2_HEIGHT * 2)
#else
#define LCDC_L2_FBSIZE        (LCDC_L2_STRIDE * CONFIG_LCDC_L2_HEIGHT)
#endif

#else
#define LCDC_L2_FBSIZE (0)
#endif

/* Total memory used for framebuffers */
#ifdef CONFIG_LCDC_L2
#define LCDC_TOTAL_FBSIZE     (LCDC_L1_FBSIZE + \
                                     LCDC_L2_FBSIZE)
#else
#define LCDC_TOTAL_FBSIZE     (LCDC_L1_FBSIZE)
#endif

/* Debug option */
//#define GRAPHIC_DRV_DEBUG
//#define GRAPHIC_DRV_VERBOSE

#ifdef  GRAPHIC_DRV_VERBOSE
#ifndef GRAPHIC_DRV_DEBUG
#define GRAPHIC_DRV_DEBUG
#endif
#define LCDC_INFO(...)  TR_INFO( TR_MOD(TEST), "//"  __VA_ARGS__)
#define LCDC_REGINFO(...)  TR_INFO( TR_MOD(TEST), __VA_ARGS__)
#else
#define LCDC_INFO(...)
#define LCDC_REGINFO(...)
#endif

#ifdef  GRAPHIC_DRV_DEBUG
#define LCDC_REP(...)  TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#else
#define LCDC_REP(...)
#endif

#define LCDC_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)

/* Preallocated LCDC framebuffers */

/**
 * Position the framebuffer memory in the center of the memory set aside.  We
 * will use any skirts before or after the framebuffer memory as a guard against
 * wild framebuffer writes.
 */

#define LCDC_BUFFER_SIZE      CONFIG_LCDC_FB_SIZE
#define LCDC_BUFFER_FREE      (LCDC_BUFFER_SIZE - LCDC_TOTAL_FBSIZE)
#define LCDC_BUFFER_START     (CONFIG_LCDC_FB_BASE + LCDC_BUFFER_FREE/2)

#if LCDC_BUFFER_FREE < 0
#error "LCDC_BUFFER_SIZE not large enough for frame buffers"
#endif

/* Layer frame buffer */

#define LCDC_BUFFER_L1        LCDC_BUFFER_START
#define LCDC_ENDBUF_L1        (LCDC_BUFFER_L1 + LCDC_L1_FBSIZE)

#ifdef CONFIG_LCDC_L2
#define LCDC_BUFFER_L2      LCDC_ENDBUF_L1
#define LCDC_ENDBUF_L2      (LCDC_BUFFER_L2 + LCDC_L2_FBSIZE)
#else
#define LCDC_ENDBUF_L2      LCDC_ENDBUF_L1
#endif

/* LCDC layer */

#ifdef CONFIG_LCDC_L2
#define LCDC_NLAYERS 2
#else
#define LCDC_NLAYERS 1
#endif

#define LCDC_NOVERLAYS              LCDC_NLAYERS

/* Dithering */

#ifndef CONFIG_LCDC_DITHER_LEVEL
#define LCDC_DITHER_LEVEL     1
#else
#define LCDC_DITHER_LEVEL     CONFIG_LCDC_DITHER_LEVEL
#endif

#ifndef CONFIG_LCDC_DITHER_RED
#define LCDC_DITHER_RED     0
#else
#define LCDC_DITHER_RED     CONFIG_LCDC_DITHER_RED
#endif
#ifndef CONFIG_LCDC_DITHER_GREEN
#define LCDC_DITHER_GREEN   0
#else
#define LCDC_DITHER_GREEN   CONFIG_LCDC_DITHER_GREEN
#endif
#ifndef CONFIG_LCDC_DITHER_BLUE
#define LCDC_DITHER_BLUE    0
#else
#define LCDC_DITHER_BLUE    CONFIG_LCDC_DITHER_BLUE
#endif

/* Background color */

#ifndef CONFIG_LCDC_BACKCOLOR
#define LCDC_BACKCOLOR      0
#else
#define LCDC_BACKCOLOR      CONFIG_LCDC_BACKCOLOR
#endif

/* Layer default color */

#ifdef CONFIG_LCDC_L1_COLOR
#define LCDC_L1_COLOR        CONFIG_LCDC_L1_COLOR
#else
#define LCDC_L1_COLOR        0x000000
#endif

#ifdef CONFIG_LCDC_L2
#ifdef CONFIG_LCDC_L2_COLOR
#define LCDC_L2_COLOR        CONFIG_LCDC_L2_COLOR
#else
#define LCDC_L2_COLOR        0x000000
#endif
#endif

/* Calculate the size of the layers clut table */

#ifdef CONFIG_FB_CMAP
#ifdef LCDC_L1CMAP
#ifdef CONFIG_FB_TRANSPARENCY
#define LAYER_CLUT_SIZE LCDC_NCLUT * sizeof(uint32_t)
#else
#define LAYER_CLUT_SIZE LCDC_NCLUT * 3 * sizeof(uint8_t)
#endif
#endif
#ifdef LCDC_L2CMAP
#undef  LAYER_CLUT_SIZE
#ifdef CONFIG_FB_TRANSPARENCY
#define LAYER_CLUT_SIZE LCDC_NCLUT * sizeof(uint32_t) * 2
#else
#define LAYER_CLUT_SIZE LCDC_NCLUT * 3 * sizeof(uint8_t) * 2
#endif
#endif
#endif

#ifndef CONFIG_FB_CMAP
#if defined(LCDC_L1CMAP) || defined(LCDC_L2CMAP)
#undef LCDC_L1CMAP
#undef LCDC_L2CMAP
#error "Enable cmap to support the configured layer format!"
#endif
#endif

/* Layer argb clut register position */

#define LCDC_CLUT_ADD(n)            ((uint32_t)(n) << 24)
#define LCDC_CLUT_ALPHA(n)          LCDC_CLUT_ADD(n)
#define LCDC_CLUT_RED(n)            ((uint32_t)(n) << 16)
#define LCDC_CLUT_GREEN(n)          ((uint32_t)(n) << 8)
#define LCDC_CLUT_BLUE(n)           ((uint32_t)(n) << 0)
#define LCDC_CLUT_RGB888_MASK       0xffffff

/* Layer argb cmap conversion */

#define LCDC_CMAP_ALPHA(n)          ((uint32_t)(n) >> 24)
#define LCDC_CMAP_RED(n)            ((uint32_t)(n) >> 16)
#define LCDC_CMAP_GREEN(n)          ((uint32_t)(n) >> 8)
#define LCDC_CMAP_BLUE(n)           ((uint32_t)(n) >> 0)

/* Hardware acceleration support */

/* Acceleration support for LCDC overlays */

#ifdef CONFIG_LCDC_L1_CHROMAKEYEN
#define LCDC_L1_CHROMAEN    true
#define LCDC_L1_CHROMAKEY   CONFIG_LCDC_CHROMAKEY
#define LCDC_LCDC_ACCL_L1         FB_ACCL_TRANSP | FB_ACCL_CHROMA
#else
#define LCDC_L1_CHROMAEN    false
#define LCDC_L1_CHROMAKEY   0
#define LCDC_LCDC_ACCL_L1         FB_ACCL_TRANSP
#endif

#ifdef CONFIG_LCDC_L2_CHROMAKEYEN
#define LCDC_L2_CHROMAEN    true
#define LCDC_L2_CHROMAKEY   CONFIG_LCDC_CHROMAKEY
#define LCDC_LCDC_ACCL_L2         FB_ACCL_TRANSP | FB_ACCL_CHROMA
#else
#define LCDC_L2_CHROMAEN    false
#define LCDC_L2_CHROMAKEY   0
#define LCDC_LCDC_ACCL_L2         FB_ACCL_TRANSP
#endif

#define LCDC_BLIT_ACCL            0

#define LCDC_L1_ACCL              LCDC_LCDC_ACCL_L1
#ifdef CONFIG_LCDC_L2
#define LCDC_L2_ACCL              LCDC_LCDC_ACCL_L2
#endif

/* Color normalization */

#if defined(CONFIG_LCDC_L1_RGB565)
#define RGB888_R(x)               (((((x) >> 11) & 0x1f) * 527 + 23) >> 6)
#define RGB888_G(x)               (((((x) >> 5) & 0x3f) * 259 + 33) >> 6)
#define RGB888_B(x)               ((((x) & 0x1f) * 527 + 23) >> 6)
#define ARGB8888(x)               ((RGB888_R(x) << 16) | \
                                     (RGB888_G(x) << 8)  | \
                                      RGB888_B(x))
#else
#define ARGB8888(x)               (x)
#endif

/****************************************************************************
 * Private Types
 ****************************************************************************/

/**
 * enum layer_e   - This enumeration names each layer supported by the hardware
 * @LCDC_LAYER_L1  : LCDC  Layer 1
 * @LCDC_LAYER_L2  : LCDC  Layer 2
 * @LCDC_LAYER_O   : LCDC  WB Layer
 */

enum layer_e {
    LCDC_LAYER_L1 = 0,    /* LCDC  Layer 1 */
    LCDC_LAYER_L2,        /* LCDC  Layer 2 */
    LCDC_LAYER_O,        /* LCDC  WB Layer */
};

/**
 * struct lcdc_s  - LCDC General layer information
 * @layerno        :  layer number
 * @oinfo          :  Overlay info
 */

struct lcdc_s {
    int layerno;

#ifdef CONFIG_FB_OVERLAY
    struct fb_overlayinfo_s oinfo;
#endif

/**
 * @lock  : Ensure mutually exclusive access
 */
    sem_t *lock;
};

enum dispbuf_states_s {
    BUF_NONE,
    BUF_INIT,
    BUF_ROTATE,
    BUF_READY,
    BUF_COMMIT,
};

struct paninfo_buf_s {
    void *addr;
    uint32_t yoffset;
    uint32_t current_frame;
    enum dispbuf_states_s state;
#ifdef ROTATE_DIRECT
    uint8_t rotate_id;
#endif
};

struct paninfo_s {
    volatile struct paninfo_buf_s queue[2];
    volatile int cnt;
    volatile int frame_num;
#ifdef ROTATE_DIRECT
    volatile uint8_t pre_rotate_id;
#endif
};

/**
 * struct lcdcdev_s    - LCDC device driver interface
 * @vtable     :   Framebuffer operations interface
 * @vinfo      :   Framebuffer video information
 * @pinfo      :   Framebuffer plane information
 * @cmap       :   Cmap information
 * @cadjmap    :   Color adjust map information
 * @cdithermap :   Color dither map information
 * @layer[]    :   Layers information
 *
 */

struct lcdcdev_s {
    struct fb_vtable_s vtable;
    struct fb_videoinfo_s vinfo;
    struct fb_planeinfo_s pinfo;
#ifdef CONFIG_FB_CMAP
    struct fb_cmap_s cmap;
#endif
#ifdef CONFIG_FB_COLORADJCURVE
    struct fb_cadjmap_s cadjmap;
#endif
#ifdef CONFIG_FB_COLORDITHER
    struct fb_cdithermap_s cdithermap;
#endif
    struct lcdc_s layer[LCDC_NOVERLAYS + 1];

    struct paninfo_s panfo;

    int refresh_oneself;

    FAR struct lcd_dev_s *lcddev;
#ifdef CONFIG_PM
    enum pm_state_e panel_state;
    enum pm_state_e cpu_state;
    int ref_cnt;
    int ref_cnt_cmd;
    bool sleep;
#endif
};

/**
 * struct interrupt_s  - Interrupt handling
 * @irq      : irq number
 * @error    : Interrupt error status
 * @sem      : Semaphore for waiting for irq
*/

struct interrupt_s {
    int irq;
    int error;
    sem_t *sem;
    LCDC_IRQ_CALLBACK cb;
};

#ifdef CONFIG_LCDC_TE_IRQ_ENABLE
struct teinterrupt_s {
    int irq;
    int error;
    sem_t *sem;
    LCDC_IRQ_CALLBACK cb;
};
#endif

#if defined(CONFIG_FB_DRV_ROTATE)
static void fb_rotate_trigger(struct lcdcdev_s *priv);
static uint8_t get_rotate_buf();
static void free_rotate_buf(uint8_t i);

#ifndef CONFIG_FBDST_BUFFER_SECTION
#define CONFIG_FBDST_BUFFER_SECTION ".framebuffer"
#endif

#define FB_DST_LOC __attribute__((section(CONFIG_FBDST_BUFFER_SECTION)))

#if defined(ROTATE_DIRECT)
#define ROTATE_BUF_COUNT    2
FB_PIX_TYPE dst_fb_buf[ROTATE_BUF_COUNT][LCDC_WIDTH*LCDC_HEIGHT] FB_DST_LOC;
#else
FB_PIX_TYPE dst_fb_buf[LCDC_WIDTH*LCDC_HEIGHT] FB_DST_LOC;
#endif
//uint32_t *dst_fb_buf = (uint32_t *)0x20089000;
#endif

static struct lcdc_config_t *lcdc_config = NULL;

/**
 * Private Function Prototypes
 */
static int lcdc_waitforirq(void);

static void lcdc_ltransp(FAR struct lcdc_s *layer, uint8_t transp,
             uint32_t mode);

#ifdef CONFIG_LCDC_CHROMAKEY
static void lcdc_lchromakey_enable(FAR struct lcdc_s *layer,
                uint32_t chroma, uint32_t rchroma);
#endif

static void lcdc_linit(uint8_t lid);

#ifdef CONFIG_FB_CMAP
static void lcdc_lputclut(FAR struct lcdc_s *layer,
              FAR const struct fb_cmap_s *cmap);
static void lcdc_lgetclut(FAR struct lcdc_s *layer, FAR struct fb_cmap_s *cmap);
static void lcdc_lclutenable(FAR struct lcdc_s *layer, bool enable);
#endif

#ifdef CONFIG_FB_COLORADJCURVE
static void lcdc_lputcadjcurves(FAR struct lcdc_s *layer,
                FAR const struct fb_cadjmap_s *cadjmap);
static void lcdc_lgetcadjcurves(FAR struct lcdc_s *layer,
                FAR struct fb_cadjmap_s *cadjmap);
static void lcdc_lcadjcurves_enable(FAR struct lcdc_s *layer, bool enable);
#endif


static int getvideoinfo(FAR struct fb_vtable_s *vtable,
            FAR struct fb_videoinfo_s *vinfo);
static int getplaneinfo(FAR struct fb_vtable_s *vtable,
            int planeno, FAR struct fb_planeinfo_s *pinfo);
static int setpower(FAR struct fb_vtable_s *vtable, int power);
static int getpower(FAR struct fb_vtable_s *vtable);
#ifdef CONFIG_FB_CMAP
static int getcmap(FAR struct fb_vtable_s *vtable, FAR struct fb_cmap_s *cmap);
static int putcmap(FAR struct fb_vtable_s *vtable,
           FAR const struct fb_cmap_s *cmap);
#endif

#ifdef CONFIG_FB_COLORADJCURVE
static int getcadjmap(struct fb_vtable_s *vtable, struct fb_cadjmap_s *cadjmap);
static int putcadjmap(struct fb_vtable_s *vtable,
              const struct fb_cadjmap_s *cadjmap);
#endif /* CONFIG_FB_COLORADJCURVE */

#ifdef CONFIG_FB_COLORDITHER
static int putcdithermap(struct fb_vtable_s *vtable,
             const struct fb_cdithermap_s *cdithermap);
#endif /* CONFIG_FB_COLORADJCURVE */

uint32_t lcdc_memaddress(struct fb_overlayinfo_s *poverlay,
             uint32_t xpos, uint32_t ypos);
/**
 * The following is provided only if the video hardware signals vertical
 * synchronisation
 */

#ifdef CONFIG_FB_SYNC
static int waitforvsync(FAR struct fb_vtable_s *vtable);
#endif
#if defined(CONFIG_FB_PANDISPLAY)
static int pan_display(FAR struct fb_vtable_s *vtable,
                       FAR struct fb_planeinfo_s *pinfo);
#endif

#ifdef CONFIG_FB_VMIRRO

/**
 * video_mirror - lcdc mirro display layer
 * @vtable : Reference to lcdc driver interface
 * @var : Reference to dispaly control structure
 *
 */
static int video_mirror(FAR struct fb_vtable_s *vtable,
            FAR struct fb_var_screeninfo *var);

#endif

#ifdef CONFIG_FB_UPDATE
static int updatearea(FAR struct fb_vtable_s *vtable, const struct fb_area_s *area);
#endif

/**
 *  The following is provided only if the video hardware supports overlays
 */

#ifdef CONFIG_FB_OVERLAY
static int getoverlayinfo(FAR struct fb_vtable_s *vtable,
              int overlayno, FAR struct fb_overlayinfo_s *oinfo);
static int settransp(FAR struct fb_vtable_s *vtable,
             FAR const struct fb_overlayinfo_s *oinfo);
#ifdef CONFIG_LCDC_CHROMAKEY
static int setchromakey(FAR struct fb_vtable_s *vtable,
            FAR const struct fb_overlayinfo_s *oinfo);
#endif
static int setblank(FAR struct fb_vtable_s *vtable,
            FAR const struct fb_overlayinfo_s *oinfo);

#endif /* CONFIG_FB_OVERLAY */

/**
 * Private Data
 */

#ifdef CONFIG_FB_CMAP
/* The layers clut table entries */

static uint8_t g_redclut[LCDC_NCLUT];
static uint8_t g_greenclut[LCDC_NCLUT];
static uint8_t g_blueclut[LCDC_NCLUT];
#ifdef CONFIG_FB_TRANSPARENCY
static uint8_t g_transpclut[LCDC_NCLUT];
#endif
#endif /* CONFIG_FB_CMAP */

#ifdef CONFIG_FB_COLORADJCURVE
/* The color adjust curves table entries */
static uint8_t g_redcurve[LCDC_NCLUT];
static uint8_t g_greencurve[LCDC_NCLUT];
static uint8_t g_bluecurve[LCDC_NCLUT];
#endif /* CONFIG_FB_COLORADJCURVE */

#ifdef CONFIG_FB_COLORDITHER
/* The layers dither table entries */
static uint32_t g_cdither[4];
#endif /* CONFIG_FB_COLORDITHER */

/* The LCDC semaphore that enforces mutually exclusive access */

static sem_t g_lock;

/* The semaphore for interrupt handling */

static sem_t g_semirq;

/* The semaphore for te interrupt handling */

POSSIBLY_UNUSED static sem_t g_semirq_te;

/* This structure provides irq handling */

static struct interrupt_s g_interrupt = {
    .irq = 0,
    .error = OK,
    .sem = &g_semirq,
    .cb = NULL,
};

#ifdef CONFIG_LCDC_TE_IRQ_ENABLE
/* This structure provides dsi irq handling */

static struct teinterrupt_s g_teinterrupt = {
    .irq = 0,
    .error = OK,
    .sem = &g_semirq_te,
    .cb = NULL,
};
#endif

/**
 *  struct lcdcdev_s g_vtable - lcdc driver instance
*/

static struct lcdcdev_s g_vtable = {
    .vtable = {
           .getvideoinfo = getvideoinfo,
           .getplaneinfo = getplaneinfo
            ,
           .setpower = setpower,
           .getpower = getpower
#ifdef CONFIG_FB_SYNC
           ,
           .waitforvsync = waitforvsync
#endif
#if defined(CONFIG_FB_PANDISPLAY)
           ,
           .pandisplay = pan_display
#endif
#ifdef CONFIG_FB_VMIRRO
           ,
           .video_mirror = video_mirror
#endif
#ifdef CONFIG_FB_UPDATE
           ,
           .updatearea = updatearea
#endif
#ifdef CONFIG_FB_CMAP
           ,
           .getcmap = getcmap,
           .putcmap = putcmap
#endif
#ifdef CONFIG_FB_COLORADJCURVE
           ,
           .getcadjmap = getcadjmap,
           .putcadjmap = putcadjmap
#endif
#ifdef CONFIG_FB_COLORDITHER
           ,
           .putcdithermap = putcdithermap
#endif
#ifdef CONFIG_FB_OVERLAY
           ,
           .getoverlayinfo = getoverlayinfo,
           .settransp = settransp,

#ifdef CONFIG_LCDC_CHROMAKEY
           .setchromakey = setchromakey,
#endif
           .setblank = setblank,
#endif /* CONFIG_FB_OVERLAY */
           },
#ifdef CONFIG_LCDC_L2
    .pinfo = {
          .fblen = LCDC_L2_FBSIZE,
          .stride = LCDC_L2_STRIDE,
          .display = 0,
          .bpp = LCDC_L2_BPP},
    .vinfo = {
          .fmt = LCDC_L2_COLOR_FMT,
          .xres = LCDC_WIDTH,
          .yres = LCDC_HEIGHT,
          .nplanes = 1,
#ifdef CONFIG_FB_OVERLAY
          .noverlays = LCDC_NOVERLAYS
#endif
          }
#else
    .pinfo = {
          // .fbmem           = (void *)(LCDC_BUFFER_L1),
          .fblen = LCDC_L1_FBSIZE,
          .stride = LCDC_L1_STRIDE,
          .display = 0,
          .bpp = LCDC_L1_BPP,
          .xres_virtual = LCDC_WIDTH,
#if defined(CONFIG_FB_DOUBLE_BUFFER)
          .yres_virtual = LCDC_HEIGHT * 2,
#else
          .yres_virtual = LCDC_HEIGHT,
#endif
          .xoffset = 0,
          .yoffset = 0
          },
    .vinfo = {
          .fmt = LCDC_L1_COLOR_FMT,
          .xres = LCDC_WIDTH,
          .yres = LCDC_HEIGHT,
          .nplanes = 1,
#ifdef CONFIG_FB_OVERLAY
          .noverlays = LCDC_NOVERLAYS
#endif
          }
#endif /* CONFIG_LCDC_L2 */
    ,
#ifdef CONFIG_FB_CMAP
    .cmap = {
         .first = 0,
         .len = LCDC_NCLUT,
         .red = g_redclut,
         .green = g_greenclut,
         .blue = g_blueclut,
#ifdef CONFIG_FB_TRANSPARENCY
         .transp = g_transpclut
#endif
         }
    ,
#endif

#ifdef CONFIG_FB_COLORADJCURVE
    .cadjmap = {
            .first = 0,
            .len = LCDC_NCLUT,
            .red = g_redcurve,
            .green = g_greencurve,
            .blue = g_bluecurve,
            }
    ,
#endif

#ifdef CONFIG_FB_COLORDITHER
    .cdithermap = {
               .first = 0,
               .len = 4,
               .data = g_cdither,
               }
    ,
#endif
    .layer[LCDC_LAYER_L1] = {
                 .layerno = LCDC_LAYER_L1,
#ifdef CONFIG_FB_OVERLAY
                 .oinfo = {
                       // .fbmem            = (uint8_t *)LCDC_BUFFER_L1,
                       .fblen = LCDC_L1_FBSIZE,
                       .stride = LCDC_L1_STRIDE,
                       .overlay = LCDC_LAYER_L1,
                       .bpp = LCDC_L1_BPP,
                       .blank = 0,
                       .chromakey = 0,
                       .color = 0,
                       .transp = {
                              .transp = 0x7f,
                              .transp_mode =
                              FB_CONST_ALPHA},
                       .sarea = {
                             .x = 0,
                             .y = 0,
                             .w = LCDC_WIDTH,
                             .h = LCDC_HEIGHT},
                       .accl = LCDC_L1_ACCL},
#endif

                 .lock = &g_lock}
#ifdef CONFIG_LCDC_L2
    ,
    .layer[LCDC_LAYER_L2] = {
                 .layerno = LCDC_LAYER_L2,
#ifdef CONFIG_FB_OVERLAY
                 .oinfo = {
                       .overlay = LCDC_LAYER_L2,
                       // .fbmem            = (uint8_t *)LCDC_BUFFER_L2,
                       .fblen = LCDC_L2_FBSIZE,
                       .stride = LCDC_L2_STRIDE,
                       .bpp = LCDC_L2_BPP,
                       .blank = 0,
                       .chromakey = 0,
                       .color = 0,
                       .transp = {
                              .transp = 0x7f,
                              .transp_mode =
                              FB_CONST_ALPHA},
                       .sarea = {
                             .x = 0,
                             .y = 0,
                             .w = LCDC_WIDTH,
                             .h = LCDC_HEIGHT},
                       .accl = LCDC_L2_ACCL},
#endif

                 .lock = &g_lock}
#endif
};

/* LCDC stride */
static const uint32_t stride_layer_t[LCDC_NLAYERS] = {
    LCDC_L1_STRIDE,
#ifdef CONFIG_LCDC_L2
    LCDC_L2_STRIDE,
#endif
};

/* LCDC framebuffer len */
POSSIBLY_UNUSED
static const uint32_t fblen_layer_t[LCDC_NLAYERS] = {
    LCDC_L1_FBSIZE,
#ifdef CONFIG_LCDC_L2
    LCDC_L2_FBSIZE,
#endif
};

/* LCDC framebuffer */

static uint32_t fbmem_layer_t[LCDC_NLAYERS + 1] = {
    0,            //LCDC_BUFFER_L1
#ifdef CONFIG_LCDC_L2
    0,            // LCDC_BUFFER_L2
#endif
    0,
};

#ifdef CONFIG_LCDC_CHROMAKEY

/* LCDC default chromakey */

static const uint32_t chromakey_layer_t[LCDC_NLAYERS] = {
    LCDC_L1_CHROMAKEY,
#ifdef CONFIG_LCDC_L2
    LCDC_L2_CHROMAKEY,
#endif
};

/* LCDC chromakey enabled state */

static const bool chromakeyen_layer_t[LCDC_NLAYERS] = {
    LCDC_L1_CHROMAEN,
#ifdef CONFIG_LCDC_L2
    LCDC_L2_CHROMAEN,
#endif
};

#endif

/* LCDC pixel format lookup table */

static const uint32_t fmt_layer_t[LCDC_NLAYERS + 1] = {
    LCDC_L1PFCR_PF,
#ifdef CONFIG_LCDC_L2
    LCDC_L2PFCR_PF,
#endif
};

/* The initialized state of the driver */

static bool g_initialized;

/**
 * Private Power Functions
 */

#ifdef CONFIG_PM
#ifndef __unused
#define __unused __attribute__((unused))
#endif

#ifndef __NuttX__
#define pm_register(cb)   (0)
#define lcdc_pmcb_register(cb)              (0)
#else
extern int lcdc_pmcb_register(struct pm_callback_s *cb);
#endif

static void pm_notify(struct pm_callback_s *cb, int domain,
                   enum pm_state_e pmstate);
static int pm_prepare(struct pm_callback_s *cb, int domain,
                   enum pm_state_e pmstate);

static void pm_panel_notify(struct pm_callback_s *cb, int domain,
                   enum pm_state_e pmstate);
static int pm_panel_prepare(struct pm_callback_s *cb, int domain,
                   enum pm_state_e pmstate);

static struct pm_callback_s pm_lcdc = {
    .notify = pm_notify,
    .prepare = pm_prepare,
};

static struct pm_callback_s panel_pmcb = {
    .notify = pm_panel_notify,
    .prepare = pm_panel_prepare,
};



/**
 * pm_notify -  Notify the driver of new power state.
 * @cb : Returned to the driver. The driver version of the callback
 *         structure may include additional, driver-specific state data at
 *         the end of the structure.
 * @pmstate : the new PM state
 *
 * This callback is called after all drivers have had the opportunity to prepare
 * for the new power state.
 *
 */

static void pm_notify(struct pm_callback_s *cb, int domain,
              enum pm_state_e pmstate)
{

    FAR struct lcdcdev_s *priv = &g_vtable;

    if (domain != PM_DOMAIN_CPU) {
        return;
    }

    if (priv->cpu_state == pmstate) {
        return;
    }

    /* support for aod mode */
    if (pmstate > priv->cpu_state && priv->panel_state == PM_IDLE) {
      switch (pmstate) {
      case PM_IDLE:
      case PM_STANDBY:
      case PM_SLEEP:
          if (g_vtable.ref_cnt > 0 || g_vtable.ref_cnt_cmd > 0) {
              break;
          }

          lcd_sleep_disp_domain();
          break;
      default:
          break;
      }
    }

    priv->cpu_state = pmstate;
}

/**
 * pm_prepare  - Request the driver to prepare for a new power state
 * @cb : Returned to the driver. The driver version of the callback
 *       structure may include additional, driver-specific state data at
 *       the end of the structure.
 * @pmstate : Identifies the new PM state
 *
 *   This is a warning that the system is about to enter into a new power state.
 *   The driver should begin whatever operations that may be required to enter power
 *   state. The driver may abort the state change mode by returning a non-zero
 *   value from the callback function.
 *
 * Returns :
 *   Zero - (OK) means the event was successfully processed and that the
 *          driver is prepared for the PM state change.
 *
 *   Non-zero - means that the driver is not prepared to perform the tasks
 *              needed achieve this power setting and will cause the state
 *              change to be aborted. NOTE: The prepare() method will also
 *              be called when reverting from lower back to higher power
 *              consumption modes (say because another driver refused a
 *              lower power state change). Drivers are not permitted to
 *              return non-zero values when reverting back to higher power
 *              consumption modes!
 *
 */
static int pm_prepare(struct pm_callback_s *cb, int domain,
              enum pm_state_e pmstate)
{
    if (domain != PM_DOMAIN_CPU) {
      return OK;
    }

    if (pmstate > g_vtable.cpu_state && (pmstate > PM_NORMAL)) {
        if (g_vtable.ref_cnt > 0 || g_vtable.ref_cnt_cmd > 0) {
            return -EINVAL;
        } else {
            return OK;
        }
    } else {
        return OK;
    }
}

__unused static void module_pm_activity(void)
{
    //request sys clk
    //pm_activity(RPMSG_IO_PDOMAIN , RPMSG_IO_ACTIVITY_PRIORITY);
}

static void pm_panel_notify(struct pm_callback_s *cb, int domain,
                   enum pm_state_e pmstate)
{
    FAR struct lcdcdev_s *priv = &g_vtable;

    /* save panel state */
    g_vtable.panel_state = pmstate;

    switch(pmstate) {
    case PM_NORMAL:
        pmu_dsi_aod_mode_enable(true);
        break;
    case PM_IDLE:
        pmu_dsi_aod_mode_enable(false);
        break;
    case PM_STANDBY:
    case PM_SLEEP:
        /* clear cmds buffers, if panel enter standby mode */
        lpcmds_clear_buffer();

        priv->ref_cnt_cmd = 0;
        priv->ref_cnt = 0;
        pmu_dsi_aod_mode_enable(false);
        break;
    default:
        break;
    }
}

static int pm_panel_prepare(struct pm_callback_s *cb, int domain,
                   enum pm_state_e pmstate)
{
    return OK;
}

void lcdc_pm_ref_cnt_increase(void)
{
    /* if panel is off will drop setting cmds */
    if (g_vtable.panel_state == PM_STANDBY ||
        g_vtable.panel_state == PM_SLEEP) {
        return;
    }

    lcdc_wakeup_disp_domain();
    g_vtable.ref_cnt_cmd++;
}

void lcdc_pm_ref_cnt_decrease(void)
{
  if (g_vtable.ref_cnt_cmd > 0) {
      g_vtable.ref_cnt_cmd--;
  }
}

#endif //CONFIG_PM

/**
 * Private Lcdc Functions
 */

static void lcdc_setup_layer_fbmem(void *fbmem, uint32_t fbsize)
{
    uint32_t buffer_l1 = 0;
#ifdef CONFIG_LCDC_L2
    uint32_t buffer_l2 = 0;
#endif
    if (fbmem)
    {
        buffer_l1 = (uint32_t)fbmem;
#ifdef CONFIG_LCDC_L2
        buffer_l2 = (uint32_t)(fbmem + fbsize/2);
#endif
    }
    else
    {
        buffer_l1 = LCDC_BUFFER_L1;
#ifdef CONFIG_LCDC_L2
        buffer_l2 = LCDC_BUFFER_L2;
#endif
    }
    fbmem_layer_t[0] = (uint32_t) (buffer_l1);
#ifdef CONFIG_LCDC_L2
    g_vtable.pinfo.fbmem = (void *)(buffer_l2);
    fbmem_layer_t[1] = (uint32_t) g_vtable.pinfo.fbmem;
#else
    g_vtable.pinfo.fbmem = (void *)(buffer_l1);
#endif

#ifdef CONFIG_FB_OVERLAY
    g_vtable.layer[LCDC_LAYER_L1].oinfo.fbmem = (uint8_t *) buffer_l1;
#endif

#ifdef CONFIG_LCDC_L2
#ifdef CONFIG_FB_OVERLAY
    g_vtable.layer[LCDC_LAYER_L2].oinfo.fbmem = (uint8_t *) buffer_l2;
#endif
#endif
}

POSSIBLY_UNUSED static volatile int lcdc_busy = 0;
POSSIBLY_UNUSED static volatile int lcdc_irq_locked_count = 0;
POSSIBLY_UNUSED static sem_t  lcdc_work;

bool lcdc_lock(void)
{
#ifdef CONFIG_DSI_VIDEO_MODE
    return true;
#else
    int ret = 0;
    uint32_t lk = int_lock();
#ifdef GRAPHIC_DRV_VERBOSE
    gra_sem_get_value(&lcdc_work, &ret);
    LCDC_INFO("lcdc_lock %d,lcdc_busy=%d,lock_count=%d,in_isr=%d\n",ret,lcdc_busy,lcdc_irq_locked_count,in_isr());
#endif
    if (in_isr()) {
        gra_sem_get_value(&lcdc_work, &ret);
        if (ret <= 0) {
            LCDC_INFO("lcdc_lock %d,lcdc_busy=%d,lock_count=%d\n",ret,lcdc_busy,lcdc_irq_locked_count);
            int_unlock(lk);
            return false;
        } else {
            gra_sem_init(&lcdc_work,0, 0);
            lcdc_busy = true;
            lcdc_irq_locked_count++;
            int_unlock(lk);
            return true;
        }
    } else {
        ret = gra_sem_trywait(&lcdc_work);
        if (ret <  OK) {
            gra_sem_get_value(&lcdc_work, &ret);
            LCDC_INFO("lcdc_lock trywait %d,lcdc_busy=%d\n",ret,lcdc_busy);
            int_unlock(lk);
            return false;
        } else {
            lcdc_busy = true;
            int_unlock(lk);
            return true;
        }
    }
#endif
}

bool lcdc_unlock(void)
{
#ifdef CONFIG_DSI_VIDEO_MODE
    return true;
#else
    int ret = 0;
    uint32_t lk = int_lock();
#ifdef GRAPHIC_DRV_VERBOSE
    gra_sem_get_value(&lcdc_work, &ret);
    LCDC_INFO("lcdc_unlock %d,lcdc_busy=%d,lock_count=%d,in_isr=%d\n",ret,lcdc_busy,lcdc_irq_locked_count,in_isr());
#endif
    if (in_isr() && lcdc_irq_locked_count) {
        gra_sem_init(&lcdc_work,0, 1);
        lcdc_irq_locked_count--;
    } else {
        gra_sem_get_value(&lcdc_work, &ret);
        if (ret < 1) {
            ret = gra_sem_post(&lcdc_work);
        } else {
            LCDC_INFO("lcdc_unlock %d,lcdc_busy=%d,lock_count=%d\n",ret,lcdc_busy,lcdc_irq_locked_count);
        }
    }
    lcdc_busy = false;
    int_unlock(lk);
    return true;
#endif
}

bool is_lcdc_busy(void)
{
    return lcdc_busy == 1;
}

void lcdc_enter_lowpower()
{
#ifdef CONFIG_DSI_VIDEO_MODE
    return;
#endif
    LCDC_INFO("%s\n", __func__);
#ifdef CONFIG_LCDC_SLEEP
    lcdc_sleep();
#endif
    hal_sysfreq_req(HAL_SYSFREQ_USER_LCDC, HAL_CMU_FREQ_32K);
}

void lcdc_exit_lowpower()
{
#ifdef CONFIG_DSI_VIDEO_MODE
    return;
#endif
    LCDC_INFO("%s\n", __func__);
    hal_sysfreq_req(HAL_SYSFREQ_USER_LCDC, HAL_CMU_FREQ_104M);
#ifdef CONFIG_LCDC_SLEEP
    lcdc_wakeup();
#endif
}

void lcdc_exit_lowpower_with_cb(LCDC_IRQ_CALLBACK cb, void *param)
{
#ifdef CONFIG_DSI_VIDEO_MODE
    if (cb)
        cb((int)param);
    return;
#endif
    LCDC_INFO("%s\n", __func__);
    hal_sysfreq_req(HAL_SYSFREQ_USER_LCDC, HAL_CMU_FREQ_104M);
#ifdef CONFIG_LCDC_SLEEP
    lcdc_wakeup();
#endif
    if (cb)
        cb((int)param);
}

#if defined(CONFIG_FB_PANDISPLAY)

static void lcdc_pandisp_init(void)
{
    FAR struct lcdcdev_s* priv = &g_vtable;

    priv->panfo.cnt = 0;
    priv->panfo.queue[0].state = BUF_NONE;
    priv->panfo.queue[0].addr = priv->pinfo.fbmem;
#ifdef ROTATE_DIRECT
    priv->panfo.pre_rotate_id = 0xff;
#endif
}

static void lcdc_pandisp_exec(void)
{
    FAR struct lcdcdev_s* priv = &g_vtable;
    uint32_t lk = int_lock();

    /*If there is ready data in the queue*/
    LCDC_REP("%s cnt=%d, panaddr = %08x,state = %d,frame_num = %d\n", __func__,priv->panfo.cnt,\
        priv->panfo.queue[0].addr,priv->panfo.queue[0].state,priv->panfo.queue[0].current_frame);
#ifdef ROTATE_DIRECT
    LCDC_REP("%s pre_rotate_id=%d, rotate_id = %d\n", __func__,priv->panfo.pre_rotate_id, priv->panfo.queue[0].rotate_id);
#endif

#if defined(CONFIG_LCDC_TE_IRQ_ENABLE)
    fb_pollnotify(&g_vtable.vtable);
#endif

    if (priv->panfo.cnt > 0) {
        /*The buffer of the sending head node*/
        //ASSERT(priv->panfo.queue[0].state != BUF_ROTATE,"frame cross vsync\n");
        if (priv->panfo.queue[0].state == BUF_ROTATE)
            LCDC_ERR("frame cross vsync\n");
        if (priv->panfo.queue[0].state == BUF_READY)
        {
            //ASSERT(priv->panfo.queue[0].state == BUF_INIT, "queue status is wrong :%d",priv->panfo.queue[0].state);
            lcdc_exit_lowpower();
            lcdc_layer_set_buffer(0, (uint8_t *)priv->panfo.queue[0].addr);
            priv->pinfo.yoffset = priv->panfo.queue[0].yoffset;
            priv->panfo.queue[0].state = BUF_COMMIT;
#ifdef ROTATE_DIRECT
            ASSERT(priv->panfo.pre_rotate_id != priv->panfo.queue[0].rotate_id, \
                "Wrong rotate id, pre is:%d, current is:%d",priv->panfo.pre_rotate_id, \
                priv->panfo.queue[0].rotate_id);
            if (priv->panfo.pre_rotate_id != 0xff)
                free_rotate_buf(priv->panfo.pre_rotate_id);
            priv->panfo.pre_rotate_id = priv->panfo.queue[0].rotate_id;
#endif
            lcdc_trigger_frame();
        }
        else
        {
#if defined(CONFIG_FB_DRV_ROTATE) && !defined(CONFIG_DSI_VIDEO_MODE) && !defined(ROTATE_DIRECT)
            fb_rotate_trigger(priv);
#endif
        }
    }
    else if (priv->refresh_oneself > 0) {
        lcdc_trigger_frame();
    }
    int_unlock(lk);
}

#ifdef CONFIG_FB_SYNC
static int lcdc_pandisp_trywait(void)
{
    FAR struct lcdcdev_s* priv = &g_vtable;

    /*Check if the queue is empty*/
#if defined(CONFIG_DSI_VIDEO_MODE) && !defined(CONFIG_FB_DRV_ROTATE)
    return (priv->panfo.cnt < 1) ? OK : ERROR;
#else
    return (priv->panfo.cnt < 2) ? OK : ERROR;
#endif
}
#endif
#endif

static void lcdc_irq_cb(uint32_t regval, uint32_t flags)
{
    FAR struct interrupt_s *priv = &g_interrupt;
    uint32_t lk = int_lock();
    LCDC_REP("%s framedone = %d, cnt:%d, state= %08x\n", __func__, hal_lcdc_irq_is_framedone(regval),
                g_vtable.panfo.cnt, g_vtable.panfo.queue[0].state);
    if (flags != 0) {
        LCDC_ERR("%s dsi underflow or overflow :%x\n", __func__, flags);
    }
#ifdef ROTATE_DIRECT
    LCDC_REP("%s pre_rotate_id=%d, rotate_id = %d\n", __func__,
            g_vtable.panfo.pre_rotate_id, g_vtable.panfo.queue[0].rotate_id);
#endif
    if (hal_lcdc_irq_is_error(regval)) {
        priv->error = -ECANCELED;
        LCDC_ERR("%s error regval = %08x\n", __func__, regval);
        LCDC_ERR("ERROR: configuration\n");
    }
    if (regval & 0xffffff00) {
        if (hal_lcdc_irq_is_framedone(regval)) {
            /* Register reload interrupt */
            if ((hal_lcdc_irq_is_framedone_irq0(regval)
                 || hal_lcdc_irq_is_framedone_irq1(regval))) {
                lpcmds_flush();

#if defined(CONFIG_FB_PANDISPLAY)
                fb_pollnotify(&g_vtable.vtable);
                if (g_vtable.refresh_oneself > 0) {
                    g_vtable.refresh_oneself = 0;
                } else if (g_vtable.panfo.cnt > 0 && g_vtable.panfo.queue[0].state == BUF_COMMIT) {
                    /*If there are two data in the queue, move the tail node to the head*/
                    g_vtable.panfo.queue[0].state = BUF_NONE;
                    if (g_vtable.panfo.cnt == 2) {
                        g_vtable.panfo.queue[0] = g_vtable.panfo.queue[1];
                    }

                    g_vtable.panfo.cnt--;
                }

#ifdef CONFIG_PM
                if (g_vtable.ref_cnt > 0) {
                    g_vtable.ref_cnt--;
                }
#endif
#if defined(CONFIG_FB_DRV_ROTATE)
#ifdef ROTATE_DIRECT
                lcdc_pandisp_exec();
#else
                fb_rotate_trigger(&g_vtable);
#endif
#endif
#endif
                priv->error = OK;
            }  else {
                LCDC_REP("%s regval = %08x\n", __func__, regval);
                priv->error = -EINVAL;
            }
            /* frame transfer is done, enter low power mode */
            lcdc_enter_lowpower();
            lcdc_unlock();
            if (priv->cb) {
                priv->cb(priv->error);
            }
            /* Unlock the semaphore if locked */
            int ret = 0;
            gra_sem_get_value(priv->sem, &ret);
            if (ret < 1)
                gra_sem_post(priv->sem);
        }
    }
    int_unlock(lk);
}


/**
 * lcdc_waitforirq - Helper waits until the lcdc irq occurs.
 */
__attribute__((unused)) static int lcdc_waitforirq(void)
{
    int ret = OK;
    FAR struct interrupt_s *priv = &g_interrupt;
    TC_INIT
    LCDC_REGINFO("lcdc_waitforirq();");
    TC_START
    ret = gra_sem_wait(priv->sem);

    if (ret < 0) {
        LCDC_ERR("ERROR: gra_sem_wait() failed\n");
    }

    ret = priv->error;
    TC_END
    TC_REP(lcdccost);
    return ret;
}

/**
 * lcdc_irqconfig - Configure interrupts
 */

static void lcdc_irqconfig(void)
{
    /* Initialize the LCDC semaphore that enforces mutually exclusive access */
    gra_sem_init(&g_lock, 0, 1);
    gra_sem_init(&lcdc_work, 0, 1);
    /* Initialize the semaphore for interrupt handling.  This waitsem
     * semaphore is used for signaling and, hence, should not have priority
     * inheritance enabled.
     */
    gra_sem_init(g_interrupt.sem, 0, 1);
    //gra_sem_setprotocol(g_interrupt.sem, SEM_PRIO_NONE);

    lcdc_register_irq_cb(lcdc_irq_cb);
}

#ifdef CONFIG_LCDC_TE_IRQ_ENABLE
static void lcdc_vsync_cb(void)
{
    LCDC_INFO("%s\n", __func__);
    FAR struct teinterrupt_s *priv = &g_teinterrupt;
#if defined(CONFIG_FB_PANDISPLAY)
    lcdc_pandisp_exec();
#endif
    priv->error = OK;
    if (priv->cb) {
        priv->cb(priv->error);
    }

    /* Unlock the semaphore if locked */
    int ret = 0;
    gra_sem_get_value(priv->sem, &ret);
    if (ret < 1)
        gra_sem_post(priv->sem);
    if (ret < 0) {
        LCDC_ERR("ERROR: gra_sem_post() failed at dsi interrupt\n");
    }
}

/**
 * lcdc_teirqconfig - config te interrupt
 */
static void lcdc_teirqconfig(void)
{
    /* Initialize the semaphore for interrupt handling.  This waitsem
     * semaphore is used for signaling and, hence, should not have priority
     * inheritance enabled.
     */
    gra_sem_init(g_teinterrupt.sem, 0, 0);

    lcdc_register_vsync_cb(lcdc_vsync_cb);
}
#endif

/**
 * lcdc_ltransp - Change layer transparency.
 * @layer  : Reference to the layer control structure
 * @transp : Transparency
 * @mode   : Transparency mode
 *
 */
static void lcdc_ltransp(FAR struct lcdc_s *layer, uint8_t transp,
             uint32_t mode)
{
    ASSERT(layer->layerno < LCDC_NLAYERS, "%s", __func__);
    FAR struct lcdcdev_s *dev = &g_vtable;

    dev->layer[layer->layerno].oinfo.transp.transp = transp;
    dev->layer[layer->layerno].oinfo.transp.transp_mode = mode;

    lcdc_layer_blend(0, 0, mode, transp);
}

#ifdef CONFIG_LCDC_CHROMAKEY
/**
 * lcdc_lchromakey - Change layer chromakey.
 * @layer    : Reference to the layer control structure
 * @chroma   : chromakey
 * @rchroma  : color to replace chromakey
 *
 */
static void lcdc_lchromakey_enable(FAR struct lcdc_s *layer,
                uint32_t chroma, uint32_t rchroma)
{
#if 1
    uint32_t rgb, rrgb;
    uint8_t r, g, b;
    uint8_t rr, rg, rb;
    ASSERT(layer->layerno < LCDC_NLAYERS, "%s", __func__);

    /* Set chromakey */

#ifdef CONFIG_FB_CMAP
    r = g_vtable.cmap.red[chroma];
    g = g_vtable.cmap.green[chroma];
    b = g_vtable.cmap.blue[chroma];
    rgb = ((r << 16) | (g << 8) | b);
    rr = g_vtable.cmap.red[rchroma];
    rg = g_vtable.cmap.green[rchroma];
    rb = g_vtable.cmap.blue[rchroma];
    rrgb = ((rr << 16) | (gr << 8) | rb);
#else
//  rgb = ARGB8888(chroma);
    rgb = chroma;

    r = (rgb & 0xff0000) >> 16;
    g = (rgb & 0x00ff00) >> 8;
    b = (rgb & 0xff);

#if 1
    rgb = ((b << 16) | (g << 8) | r);
#endif

    rrgb = rchroma;
    rr = (rrgb & 0xff0000) >> 16;
    rg = (rrgb & 0x00ff00) >> 8;
    rb = (rrgb & 0xff);

#if 1
    rrgb = ((rb << 16) | (rg << 8) | rr);
#endif

#endif

    LCDC_INFO("set LCDC_L%dCKCR=%08x\n", layer->layerno + 1, rgb);

    lcdc_chromakey_enable(rgb, rrgb, 0);
#endif
}

#endif //CONFIG_LCDC_CHROMAKEY

/**
 * lcdc_lclutenable - Disable or enable the layer clut support
 * @layer  : Reference to the layer control structure
 * @enable : Enable or disable
 */
#ifdef CONFIG_FB_CMAP
static void lcdc_lclutenable(FAR struct lcdc_s *layer, bool enable)
{

}

/**
 * lcdc_setclut - Configure foreground and background clut table
 * @cmap  : fb cmap
 */
int lcdc_setclut(FAR const struct fb_cmap_s *cmap)
{
    int n;
    uint32_t crreg;
    uint32_t citem;
    struct lcdcdev_s *priv = &g_vtable;

    for (n = cmap->first; n < cmap->len - 1 && n < LCDC_NCLUT; n++) {
        /* Update the layer clut entry, will be automatically loaded before
         * blit operation becomes active
         */

        priv->cmap.blue[n] = cmap->blue[n];
        priv->cmap.green[n] = cmap->green[n];
        priv->cmap.red[n] = cmap->red[n];

        citem =
            ((cmap->red[n] << 16) | (cmap->green[n] << 8) | cmap->
             blue[n]) & 0xffffff;
        //LCDC_INFO("n=%d, red=%02x, green=%02x, blue=%02x\n", n, cmap->red[n],
        //        cmap->green[n], cmap->blue[n]);

        hal_lcdc_write_clut(citem, n);

    }

    return OK;
}

/**
 * lcdc_lputclut - Update the clut layer register during blank period.
 * @layer  : Reference to the layer control structure
 * @cmap   : Color map
 *
 */
static void lcdc_lputclut(FAR struct lcdc_s *layer,
              FAR const struct fb_cmap_s *cmap)
{
    int n;
    uint32_t flags;
    /* Disable clut during register update */

    lcdc_lclutenable(layer, false);

    /* Update the clut registers. Ensure operation is atomic or in interrupt
     * protected context.
     * */
    flags = int_lock();
    lcdc_setclut(cmap);
    int_unlock(flags);
    /* Enable clut after register update */

    lcdc_lclutenable(layer, true);
}

/**
 * lcdc_lgetclut - Copy the layers color lookup table.
 * @layer  : Reference to the layer control structure
 * @cmap   : Color map
 *
 */
static void lcdc_lgetclut(FAR struct lcdc_s *layer, FAR struct fb_cmap_s *cmap)
{
    int n;
    FAR struct fb_cmap_s *priv_cmap = &g_vtable.cmap;

    /* Copy from internal cmap */

    for (n = cmap->first; n < cmap->len && n < LCDC_NCLUT; n++) {

#ifdef CONFIG_FB_TRANSPARENCY
        cmap->transp[n] = priv_cmap->transp[n];
#endif
        cmap->red[n] = priv_cmap->red[n];
        cmap->green[n] = priv_cmap->green[n];
        cmap->blue[n] = priv_cmap->blue[n];

        LCDC_INFO
            ("color = %d, transp=%02x, red=%02x, green=%02x, blue=%02x\n",
             n,
#ifdef CONFIG_FB_TRANSPARENCY
             cmap->transp[n],
#endif
             cmap->red[n], cmap->green[n], cmap->blue[n]);
    }
}
#endif /* CONFIG_FB_CMAP */

#ifdef CONFIG_FB_COLORADJCURVE

static void lcdc_lputcadjcurves(FAR struct lcdc_s *layer,
                FAR const struct fb_cadjmap_s *cadjmap);
static void lcdc_lgetcadjcurves(FAR struct lcdc_s *layer,
                FAR struct fb_cadjmap_s *cadjmap);

static void lcdc_lcadjcurves_enable(FAR struct lcdc_s *layer, bool enable)
{
    hal_lcdc_lcadjcurves_enable(layer->layerno, enable);
}

/**
 * lcdc_cadjcurves_enable - enable/disable foreground and background color adjust curves
 */
void lcdc_cadjcurves_enable(bool enable)
{
    FAR struct lcdc_s *layer = &g_vtable.layer[0];
    lcdc_lcadjcurves_enable(layer, enable);
}

/**
 * lcdc_setcadjcurves - Configure foreground and background color adjust curves table
 * @cadjmap  : fb color adjust curves
 *
 */
static int lcdc_setcadjcurves(FAR const struct fb_cadjmap_s *cadjmap)
{
    int n;
    uint32_t crreg;
    uint32_t citem;
    struct lcdcdev_s *priv = &g_vtable;

    for (n = cadjmap->first; n < cadjmap->len && n <= LCDC_NCLUT; n++) {
        priv->cadjmap.blue[n] = cadjmap->blue[n];
        priv->cadjmap.green[n] = cadjmap->green[n];
        priv->cadjmap.red[n] = cadjmap->red[n];
        citem =
            ((cadjmap->red[n] << 16) | (cadjmap->green[n] << 8) |
             cadjmap->blue[n]) & 0xffffff;

        LCDC_INFO("n=%d, red=%02x, green=%02x, blue=%02x\n", n,
              cadjmap->red[n], cadjmap->green[n], cadjmap->blue[n]);

        hal_lcdc_write_gamma(citem, n);
        // hal_sys_timer_delay_us(1000);
    }

    return OK;
}

/**
 * lcdc_lputcadjcurves - Update the clut layer register during blank period.
 * @layer  : Reference to the layer control structure
 * @cmap   : Color map
 *
 */
static void lcdc_lputcadjcurves(FAR struct lcdc_s *layer,
                FAR const struct fb_cadjmap_s *cadjmap)
{
    int n;
    uint32_t flags;

    if (layer->layerno != 0) {
        return;
    }
    flags = int_lock();
    if (layer->layerno == 0) {
        lcdc_setcadjcurves(cadjmap);
    }
    int_unlock(flags);

    lcdc_lcadjcurves_enable(layer, true);

}

/**
 * lcdc_lgetcadjcurves - Copy the layers color lookup table.
 * @layer  : Reference to the layer control structure
 * @cmap   : Color map
 */
static void lcdc_lgetcadjcurves(FAR struct lcdc_s *layer,
                FAR struct fb_cadjmap_s *cadjmap)
{
    int n;
    FAR struct fb_cadjmap_s *priv_cmap = &g_vtable.cadjmap;

    /* Copy from internal cmap */

    for (n = cadjmap->first; n < cadjmap->len && n < LCDC_NCLUT; n++) {

        cadjmap->red[n] = priv_cmap->red[n];
        cadjmap->green[n] = priv_cmap->green[n];
        cadjmap->blue[n] = priv_cmap->blue[n];

        LCDC_INFO("color = %d, red=%02x, green=%02x, blue=%02x\n", n,
              cadjmap->red[n], cadjmap->green[n], cadjmap->blue[n]);
    }
}
#endif /* CONFIG_FB_COLORADJCURVE */

#if defined(CONFIG_FB_PANDISPLAY)

/**
 * pan_display - lcdc switch display layer
 * @vtable : Reference to lcdc driver interface
 * @var : Reference to dispaly control structure
 *
 * Used for fb double buffer application,change display layer by xoffset and yoffset
 */

static bool fb_replace_ok = false;
static int pan_display(FAR struct fb_vtable_s *vtable,
                       FAR struct fb_planeinfo_s *pinfo)
{
    FAR struct lcdcdev_s *priv = &g_vtable;
    FAR struct fb_planeinfo_s *pri_pinfo = &priv->pinfo;

#ifdef SUPPORT_REPLACE_FBMEM
    if (pinfo->yoffset == UINT_MAX || fb_replace_ok)
    {
        memcpy((void *)0x50000000,priv->pinfo.fbmem,priv->pinfo.fblen);
        free(priv->pinfo.fbmem);
        lcdc_setup_layer_fbmem((void *)0x50000000,priv->pinfo.fblen);
        fb_replace_ok = false;
        return OK;
    }
#endif
    uint32_t dispbuff = (uint32_t)priv->pinfo.fbmem;

    if (vtable == NULL && pinfo->yoffset != 0) {
        dispbuff = pinfo->yoffset;
    } else {
#if defined(CONFIG_FB_DOUBLE_BUFFER)
        if (pinfo->yoffset > pri_pinfo->yres_virtual - priv->vinfo.yres ||
            pinfo->xoffset > pri_pinfo->xres_virtual - priv->vinfo.xres) {
            return -EINVAL;
        }
#endif

#if defined(CONFIG_FB_DOUBLE_BUFFER) || defined(CONFIG_DISCONTNUOUS_FB_DOUBLE_BUFFER)
        #ifdef USE_GAMU_FB
            if (priv->pinfo.bpp == 24) {
                dispbuff = (uint32_t)priv->pinfo.fbmem + 0x100000;
            } else {
                dispbuff = (uint32_t)priv->pinfo.fbmem + priv->pinfo.stride * pinfo->yoffset;
            }
        #else
            dispbuff = (uint32_t)priv->pinfo.fbmem + priv->pinfo.stride * pinfo->yoffset;
        #endif
#else
        if (pinfo->yoffset != 0)
            dispbuff = pinfo->yoffset;
#endif
    }

#if defined(ROTATE_DIRECT)
    uint8_t id = get_rotate_buf();
    if (id == 0xff)
        return ERROR;
    cpu_rotate_90((uint32_t)dispbuff,(uint32_t)dst_fb_buf[id],LCDC_HEIGHT,LCDC_WIDTH);
    dispbuff = (uint32_t)dst_fb_buf[id];
#endif

    uint32_t flags = int_lock();
    priv->panfo.frame_num++;
    LCDC_REP("%s cnt=%d,frame_num:%d, pre panaddr = %08x,yoffset = %08x,fbmem = %08x,\n", __func__,priv->panfo.cnt, priv->panfo.frame_num, priv->panfo.queue[0].addr,pinfo->yoffset,priv->pinfo.fbmem);
    /*If there is space in the queue, fill it with a new buffer*/
#if defined(CONFIG_DSI_VIDEO_MODE) && !defined(CONFIG_FB_DRV_ROTATE)
    if(priv->panfo.cnt < 1
#else
    if(priv->panfo.cnt < 2
#endif
#ifdef CONFIG_PM
      && (priv->panel_state != PM_STANDBY && priv->panel_state != PM_SLEEP)
#endif
      ) {
        uint8_t next_buf = priv->panfo.cnt;
        priv->panfo.queue[next_buf].yoffset = pinfo->yoffset;
        priv->panfo.queue[next_buf].addr = (void *)dispbuff;
        priv->panfo.queue[next_buf].state = BUF_INIT;
        priv->panfo.queue[next_buf].current_frame = priv->panfo.frame_num;
        priv->panfo.cnt++;
#if defined(CONFIG_FB_DRV_ROTATE)
#if defined(CONFIG_ROTATE_DMA)
        if (!ROTATE_USE_CPU(priv->pinfo.fbmem))
            lcdc_cache_sync(dispbuff, LCDC_L1_STRIDE*LCDC_HEIGHT);
#endif
#if defined(ROTATE_DIRECT)
        LCDC_REP("%s pre_rotate_id=%d, rotate_id = %d\n", __func__,priv->panfo.pre_rotate_id, id);
        priv->panfo.queue[next_buf].rotate_id = id;
        priv->panfo.queue[next_buf].state = BUF_READY;
#elif !defined(CONFIG_DSI_VIDEO_MODE)
        fb_rotate_trigger(priv);
#endif
#else
        lcdc_cache_sync(dispbuff, LCDC_L1_STRIDE*LCDC_HEIGHT);
        priv->panfo.queue[next_buf].state = BUF_READY;
#endif
#ifdef CONFIG_PM
        /* first to post buffer, need wakeup lcdc/dsi */
        if (priv->panel_state == PM_IDLE) {
            /* It's more realistic to wake up  */
            lcdc_wakeup_disp_domain();
        }

        priv->ref_cnt++;
#endif
        int_unlock(flags);
        return OK;
    }
    else
        LCDC_ERR("%s Lossing frame_num:%d\n", __func__,priv->panfo.frame_num);
    int_unlock(flags);
#if defined(ROTATE_DIRECT)
    free_rotate_buf(id);
#endif
    return ERROR;
}
#endif

#ifdef CONFIG_FB_VMIRRO

static bool vmirro_mode = false;
/**
 * video_mirror - lcdc set mirror display layer enable
 * @vtable : Reference to lcdc driver interface
 * @var : Reference to dispaly control structure
 *
 */
static int video_mirror(FAR struct fb_vtable_s *vtable,
            FAR struct fb_var_screeninfo *var)
{
    int ret = 0;
    uint8_t lid = 0;
    struct fb_overlayinfo_s *poverlay;
    int32_t lsa = 0;

    if (var->xoffset == 0 && var->yoffset == 0) {
        lid = 0;
    } else {
        lid = 1;
    }

    poverlay = &g_vtable.layer[lid].oinfo;

    vmirro_mode = (var->rotate == 180) ? true : false;

    LCDC_INFO("%s enable lid:%d", __func__, lid);

    if (vmirro_mode) {
        lsa = lcdc_memaddress(poverlay, poverlay->sarea.x,
                            poverlay->sarea.h - 1);
    } else {
        lsa = lcdc_memaddress(poverlay, poverlay->sarea.x,
                            poverlay->sarea.y);
    }

    lcdc_layer_set_buffer(lid, (uint8_t *)lsa);
    LCDC_INFO("%s enable lid:%d lsa:%p", __func__, lid, lsa);
    hal_lcdc_set_vmirro_enable(lid, vmirro_mode);
    return ret;
}

bool in_vmirro_mode(void)
{
    return vmirro_mode;
}

#endif

/**
 * lcdc_memaddress - Helper to calculate the layer memory address
 * @oinfo - Reference to overlay information
 * @xpos  - x-Offset
 * @ypos  - y-Offset
 *
 * Returns: start memory address
 *
 */
uint32_t lcdc_memaddress(struct fb_overlayinfo_s * poverlay,
             uint32_t xpos, uint32_t ypos)
{
    uint32_t offset;

    offset = xpos * (poverlay->bpp / 8) + poverlay->stride * ypos;

    //LCDC_INFO("0x%x, offset=%d\n", ((uint32_t) poverlay->fbmem) + offset,
    //      offset);
    return ((uint32_t) poverlay->fbmem) + offset;
}

#ifdef CONFIG_FB_UPDATE
    #ifdef USE_GAMU_FB
    #else
        #define FULL_SCREEN_REDRAWN
    #endif
/**
 * updatearea - lcdc update display with area
 * @vtable : Reference to lcdc driver interface
 * @area   : Reference to area to be displayed
 */
static int updatearea(FAR struct fb_vtable_s *vtable, const struct fb_area_s *area)
{
    struct fb_overlayinfo_s *poverlay;
    FAR struct lcdcdev_s *dev = &g_vtable;
    FAR struct lcdc_s *layer = &dev->layer[LCDC_LAYER_L1];
    poverlay = &layer->oinfo;

    if (vtable != NULL) {
#if defined(CONFIG_FB_PANDISPLAY)
#if defined(CONFIG_FB_DOUBLE_BUFFER)
        return 0;
#else
        struct fb_planeinfo_s pinfo = {
            .yoffset = 0,
        };
        LCDC_REP("updatearea:x:%d,y:%d  w: %d h:%d", area->x,area->y, area->w,area->h);
        while (pan_display(vtable, &pinfo) == ERROR)
        {
            lcdc_waitforirq();
        }
        return OK;
#endif
#endif
    }

#if defined(CONFIG_FB_DOUBLE_BUFFER)
    uint8_t *display_buffer = (uint8_t *)lcdc_memaddress(poverlay, area->x,area->y+
        dev->pinfo.yoffset);
#else
    uint8_t *display_buffer = (uint8_t *)lcdc_memaddress(poverlay, area->x,area->y);
#if defined(CONFIG_FB_PANDISPLAY)
#if defined(CONFIG_DSI_VIDEO_MODE) && defined(CONFIG_FB_DRV_ROTATE)
    return 0;
#endif
    if (dev->panfo.queue[0].yoffset != 0)
        display_buffer = (uint8_t *)dev->panfo.queue[0].yoffset;
#elif defined(ROTATE_DIRECT)
/*
        uint32_t current_tick = hal_fast_sys_timer_get();
        if (times_to_line(FAST_TICKS_TO_US(current_tick - prev_framedone_ticks)) > (area->x+area->w))
        {
        }
*/
        lcdc_waitforirq();
        display_buffer = (uint8_t *)lcdc_memaddress(poverlay, 0,0);
        cpu_rotate_area_90((uint32_t)display_buffer,(uint32_t)dst_fb_buf,area->x,area->y, area->w,area->h);
        //cpu_rotate_90((uint32_t)display_buffer,(uint32_t)dst_fb_buf,LCDC_HEIGHT,LCDC_WIDTH);
        return 0;
#endif
#endif

    uint32_t flags;
    flags = int_lock();
    if (0 == area->x && area->w == LCDC_WIDTH)
        lcdc_cache_sync((uint32_t)display_buffer, LCDC_L1_STRIDE*area->h);
    else
    {
        for(int i = 0; i < area->h; i ++)
          {
              lcdc_cache_sync((uint32_t)display_buffer + i * poverlay->stride, area->w * poverlay->bpp / 8);
          }
    }
    int_unlock(flags);

#if defined(CONFIG_DSI_VIDEO_MODE)
    return 0;
#endif

#if !defined(FULL_SCREEN_REDRAWN) // !whole screen redrawn
    lcdc_exit_lowpower();
    struct lcd_planeinfo_s pinfo = {0};
    if (dev->lcddev != NULL && dev->lcddev->getplaneinfo != NULL)
        dev->lcddev->getplaneinfo(dev->lcddev,0,&pinfo);

    if (pinfo.putarea != NULL)
    {
        pinfo.putarea(area->y,area->y+area->h-1,
                area->x,area->x+area->w-1,poverlay->fbmem + poverlay->stride * dev->pinfo.yoffset);
    }
    else
    {
        uint16_t tmphend,tmpvend;
        tmphend = area->x+area->w-1;
        tmpvend = area->y+area->h-1;
        struct fb_area_s tarea;
        tarea.x = area->x;
        tarea.y = area->y;
        tarea.w = area->w;
        tarea.h = area->h;

        tarea.x &= 0xfffe;
        tarea.y &= 0xfffe;
        tarea.w = (tmphend - tarea.x +2)&0xfffe;
        tarea.h = (tmpvend - tarea.y +2)&0xfffe;

        uint16_t hstart = 14 + tarea.x;
        uint16_t hend = hstart+tarea.w -1;
        uint16_t vstart = tarea.y;
        uint16_t vend = vstart+tarea.h-1;
        uint8_t hparams[4] = { hstart>>8, hstart&0xff, hend>>8, hend&0xff };
        uint8_t vparams[4] = { vstart>>8, vstart&0xff, vend>>8, vend&0xff };

#ifdef CONFIG_LCDC_DSI
        hal_dsi_reset(lcdc_config->bus_port0, tarea.w);
        lcdc_send_cmd_list(lcdc_config->bus_port0, 0x2a, 4, hparams);
        lcdc_send_cmd_list(lcdc_config->bus_port0, 0x2b, 4, vparams);
#elif defined(CONFIG_LCDC_DUAL_DSI)
        hal_dsi_reset(lcdc_config->bus_port0, tarea.w);
        lcdc_send_cmd_list(lcdc_config->bus_port0, 0x2a, 4, hparams);
        lcdc_send_cmd_list(lcdc_config->bus_port0, 0x2b, 4, vparams);
        hal_dsi_reset(lcdc_config->bus_port1, tarea.w);
        lcdc_send_cmd_list(lcdc_config->bus_port1, 0x2a, 4, hparams);
        lcdc_send_cmd_list(lcdc_config->bus_port1, 0x2b, 4, vparams);
#else
        lcdc_send_cmd_list(lcdc_config->bus_port0, 0x2a, 4, hparams);
        lcdc_send_cmd_list(lcdc_config->bus_port0, 0x2b, 4, vparams);
#endif

        display_buffer = (uint8_t *)lcdc_memaddress(poverlay, tarea.x,tarea.y+dev->pinfo.yoffset);

        lcdc_layer_set_buffer(LCDC_LAYER_LFORE, display_buffer);
        lcdc_layer_resize(LCDC_LAYER_LFORE, tarea.w, tarea.h);
    }
#endif
    /* update Layer start addr register */
    //lcdc_layer_enable(layer->layerno);
    LCDC_INFO("updatearea:x:%d,y:%d  w: %d h:%d", area->x,area->y, area->w,area->h);
    return lcdc_update();
}
#endif

/**
 * lcdc_linit - Initialize layer to their default states.
 * @layer : Reference to the layer control structure
 *
 * Initialize:
 * - ``-layer framebuffer
 * - ``-layer pixelformat
 * - ``-layer defaultcolor
 * - ``-layer chromakey
 * - ``-layer transparency
 * - ``-layer clut
 *
 */
static void lcdc_linit(uint8_t overlay)
{
    struct fb_videoinfo_s devvinfo;
    uint32_t real_screen_w = BOARD_LCDC_WIDTH;
    uint32_t real_screen_h = BOARD_LCDC_HEIGHT;
    uint32_t x_offset = 0;
    uint32_t y_offset = 0;

    ASSERT(overlay < LCDC_NLAYERS, "%s", __func__);

    FAR struct lcdcdev_s *dev = &g_vtable;
    FAR struct lcdc_s *layer = &dev->layer[overlay];

    struct lcdc_layer_config_t config;
    uint8_t layerno = layer->layerno;

    if (g_vtable.lcddev != NULL && g_vtable.lcddev->getvideoinfo != NULL) {
        g_vtable.lcddev->getvideoinfo(g_vtable.lcddev, &devvinfo);
        real_screen_w = devvinfo.xres;
        real_screen_h = devvinfo.yres;
    }
#ifdef LCD_WIDTH
    real_screen_w = LCD_WIDTH;
#endif
#ifdef LCD_HEIGHT
    real_screen_h = LCD_HEIGHT;
#endif

#if defined(CONFIG_LCDC_ALIGN_MIDDLE)
    if (rel_screen_w > LCD_WIDTH)
        x_offset += (BOARD_LCDC_WIDTH - real_screen_w) / 2;
    if (rel_screen_h > LCD_HEIGHT)
        y_offset += (BOARD_LCDC_HEIGHT - real_screen_h) / 2;
#endif

    LCDC_ERR("buffer size %dx%d, panel size %dx%d, offset [%d, %d]\n",
            BOARD_LCDC_WIDTH, BOARD_LCDC_HEIGHT, real_screen_w, real_screen_h, x_offset, y_offset);

    memset(&config, 0, sizeof(config));

    config.enable = false;
    config.format = fmt_layer_t[layerno];
    config.width = real_screen_w;
    config.height = real_screen_h;
    config.offset_x = x_offset;
    config.offset_y = y_offset;
    config.buffer = (uint8_t *)fbmem_layer_t[layerno];
    config.stride = stride_layer_t[layerno];

    lcdc_layer_config(layerno, &config);

    /* Layers default transparency */
    lcdc_ltransp(layer, layer->oinfo.transp.transp,
             layer->oinfo.transp.transp_mode);

    /* Layers chromakey */
#ifdef CONFIG_LCDC_CHROMAKEY
    LCDC_INFO("%s chromkey:0x%x", __func__, chromakey_layer_t[overlay]);
    lcdc_lchromakey_enable(layer, chromakey_layer_t[overlay], 0);
#endif

#ifdef CONFIG_FB_CMAP
    /* Disable clut by default */
    if (dev->vinfo.fmt == FB_FMT_RGB8) {
        /* Initialize LCDC clut register */
        lcdc_lputclut(layer, &g_vtable.cmap);

        /* Configure the clut register */
        lcdc_lclutenable(layer, true);
    }
#endif

    /* Finally enable the layer */
    lcdc_layer_enable(layer->layerno);
}

/****************************************************************************
 * Public Functions
 ****************************************************************************/

/**
 * getvideoinfo - Get the videoinfo for the framebuffer
 * @vtable : Reference to lcdc driver interface
 * @vinfo  : Reference to fb video info structure
 *
 * Entrypoint ioctl FBIOGET_VIDEOINFO
 * Get the videoinfo for the framebuffer
 *
 * Returns:   OK on success, -EINVAL on failure.
 */
static int getvideoinfo(struct fb_vtable_s *vtable,
            struct fb_videoinfo_s *vinfo)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    LCDC_INFO("vtable=%p vinfo=%p\n", vtable, vinfo);
    ASSERT(vtable != NULL && priv == &g_vtable
           && vinfo != NULL, "%s", __func__);

    memcpy(vinfo, &priv->vinfo, sizeof(struct fb_videoinfo_s));

#ifdef CONFIG_FB_MODULEINFO
    struct fb_videoinfo_s devvinfo;

    ASSERT(priv->lcddev != NULL && priv->lcddev->getvideoinfo != NULL,
           "%s", __func__);

    priv->lcddev->getvideoinfo(priv->lcddev, &devvinfo);
    memcpy(vinfo->moduleinfo, &devvinfo.moduleinfo,
           sizeof(devvinfo.moduleinfo));
#endif
    return OK;
}

/**
 * getplaneinfo - Get the planeinfo for the framebuffer
 * @vtable : Reference to lcdc driver interface
 * @pinfo  :  Reference to fb plane info structure
 *
 * Entrypoint ioctl FBIOGET_PLANEINFO
 * Get the planeinfo for the framebuffer
 *
 * Returns:   OK on success, -EINVAL on failure.
 */
static int getplaneinfo(struct fb_vtable_s *vtable, int planeno,
            struct fb_planeinfo_s *pinfo)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    ASSERT(vtable != NULL && priv == &g_vtable, "%s", __func__);
    //LCDC_INFO("vtable=%p planeno=%d pinfo=%p\n", vtable, planeno, pinfo);

    if (planeno == 0) {
        int display = pinfo->display;
	(void) display;
        memcpy(pinfo, &priv->pinfo, sizeof(struct fb_planeinfo_s));
#if defined(CONFIG_DISCONTNUOUS_FB_DOUBLE_BUFFER)
#if !defined(CONFIG_FB_DOUBLE_BUFFER)
        if (display == 1) {
            unsigned int mem1 = (uint32_t)pinfo->fbmem;
            pinfo->display = 1;
            pinfo->fbmem = memalign(64, pinfo->fblen+pinfo->stride);
            ASSERT(pinfo->fbmem!=NULL, "LCDC:fbmem2 alloc fail!!!\n");
            if (pinfo->fbmem > mem1) {
                pinfo->fbmem = mem1 + (((uint32_t)pinfo->fbmem-mem1)/pinfo->stride+1)*pinfo->stride;
            }
            else {
                ASSERT(0, "LCDC:fbmem2 should larger than mem1!!!\n");
            }
#ifdef TEST_WITH_GPU
            ASSERT((((uint32_t)pinfo->fbmem%64)==0), "LCDC:fbmem2 is not 64B align!!!\n");
#endif
        }
#endif
#endif
        return OK;
    }

    LCDC_ERR("ERROR: Returning EINVAL\n");
    return -EINVAL;
}

/**
 * setpower - set panel power.
 * @vtable : Reference to lcdc driver interface
 * @power  : Power to set.
 *
 * Returns:   OK on success, -EINVAL on failure.
 */
static int setpower(FAR struct fb_vtable_s *vtable, int power)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    ASSERT(vtable != NULL && priv == &g_vtable, "%s", __func__);
    ASSERT(priv->lcddev != NULL && priv->lcddev->setpower != NULL,
           "%s", __func__);

    return priv->lcddev->setpower(priv->lcddev, power);
}

/**
 * setpower - get panel power.
 * @vtable : Reference to lcdc driver interface
 *
 * Returns:   The current power for panel.
 */
static int getpower(FAR struct fb_vtable_s *vtable)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    ASSERT(vtable != NULL && priv == &g_vtable, "%s", __func__);
    ASSERT(priv->lcddev != NULL && priv->lcddev->getpower != NULL,
           "%s", __func__);

    return priv->lcddev->getpower(priv->lcddev);
}

/**
 * getcmap - Get a range of CLUT values for the LCD
 * @vtable : Reference to lcdc driver interface
 * @cmap  :  Reference to dest color table structure
 *
 * Entrypoint ioctl FBIOGET_CMAP
 * Get a range of CLUT values for the LCD
 *
 * Returns:   OK on success, -EINVAL on failure.
 *
 */
#ifdef CONFIG_FB_CMAP
static int getcmap(struct fb_vtable_s *vtable, struct fb_cmap_s *cmap)
{
    int ret;
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    ASSERT(vtable != NULL && priv == &g_vtable
           && cmap != NULL, "%s", __func__);
    LCDC_INFO("vtable=%p cmap=%p\n", vtable, cmap);

    if (priv->vinfo.fmt != FB_FMT_RGB8) {
        LCDC_ERR
            ("ERROR: CLUT is not supported for the pixel format: %d\n",
             priv->vinfo.fmt);
        ret = -EINVAL;
    } else if (cmap->first >= LCDC_NCLUT) {
        LCDC_ERR("ERROR: only %d color table entries supported\n",
             LCDC_NCLUT);
        ret = -EINVAL;
    } else {
        /* Currently, there is no api to set color map for each overlay
         * separately. LCDC layers can have different color maps. Get the cmap
         * from the main overlay.
         */

        FAR struct lcdc_s *layer;
#ifdef CONFIG_LCDC_L2
        layer = &priv->layer[LCDC_LAYER_L2];
#else
        layer = &priv->layer[LCDC_LAYER_L1];
#endif

        gra_sem_wait(layer->lock);

        lcdc_lgetclut(layer, cmap);

        gra_sem_post(layer->lock);
        ret = OK;
    }

    return ret;
}

/**
 * putcmap  - Set a range of the CLUT values for the LCD
 * @vtable : Reference to lcdc driver interface
 * @cmap  :  Reference to source color table structure
 *
 * Entrypoint ioctl FBIOPUT_CMAP
 * Set a range of the CLUT values for the LCD
 *
 * Returns:   OK on success, -EINVAL on failure.
 */
static int putcmap(struct fb_vtable_s *vtable, const struct fb_cmap_s *cmap)
{
    int ret;
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    ASSERT(vtable != NULL && priv == &g_vtable
           && cmap != NULL, "%s", __func__);
    LCDC_INFO("vtable=%p cmap=%p\n", vtable, cmap);

    if (priv->vinfo.fmt != FB_FMT_RGB8) {
        LCDC_ERR
            ("ERROR: CLUT is not supported for the pixel format: %d\n",
             priv->vinfo.fmt);
        ret = -EINVAL;
    } else if (cmap->first >= LCDC_NCLUT) {
        LCDC_ERR("ERROR: only %d color table entries supported\n",
             LCDC_NCLUT);
        ret = -EINVAL;
    } else {
        /* Currently, there is no api to set color map for each overlay
         * separately. LCDC layers can have different color maps, but is shared
         * for now.
         */

        int n;
        FAR struct fb_cmap_s *priv_cmap = &g_vtable.cmap;

        /* First copy to internal cmap */

        for (n = cmap->first; n < cmap->len && n < LCDC_NCLUT; n++) {
            priv_cmap->red[n] = cmap->red[n];
            priv_cmap->green[n] = cmap->green[n];
            priv_cmap->blue[n] = cmap->blue[n];
#ifdef CONFIG_FB_TRANSPARENCY
            /* Not supported by LCDC */

            priv_cmap->transp[n] = cmap->transp[n];
#endif
        }

        priv_cmap->first = cmap->first;
        priv_cmap->len = cmap->len;

        /* Update the layer clut register */
        gra_sem_wait(&g_lock);
        for (n = 0; n < LCDC_NLAYERS; n++) {
            FAR struct lcdc_s *layer = &priv->layer[n];
            lcdc_lputclut(layer, priv_cmap);
        }
        gra_sem_post(&g_lock);
        ret = OK;
    }

    return ret;
}
#endif /* CONFIG_FB_CMAP */

/**
 * getcadjmap - Get color adjuct curves  for the LCD
 * @vtable : Reference to lcdc driver interface
 * @cadjmap  :  Reference to source color adjust table structure
 *
 * Entrypoint ioctl FBIOGET_CMAP
 * Get color adjuct curves  for the LCD
 *
 * Returns:   OK on success, -EINVAL on failure.
 */
#ifdef CONFIG_FB_COLORADJCURVE
static int getcadjmap(struct fb_vtable_s *vtable, struct fb_cadjmap_s *cadjmap)
{
    int ret;
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    ASSERT(vtable != NULL && priv == &g_vtable
           && cadjmap != NULL, "%s", __func__);
    LCDC_INFO("vtable=%p cadjmap=%p\n", vtable, cadjmap);

    if (cadjmap->first >= LCDC_NCLUT) {
        LCDC_ERR("ERROR: only %d color table entries supported\n",
             LCDC_NCLUT);
        ret = -EINVAL;
    } else {
        /* Currently, there is no api to set color map for each overlay
         * separately. LCDC layers can have different color maps. Get the cmap
         * from the main overlay.
         */

        FAR struct lcdc_s *layer;
#ifdef CONFIG_LCDC_L2
        layer = &priv->layer[LCDC_LAYER_L2];
#else
        layer = &priv->layer[LCDC_LAYER_L1];
#endif

        gra_sem_wait(layer->lock);

        lcdc_lgetcadjcurves(layer, cadjmap);

        gra_sem_post(layer->lock);
        ret = OK;
    }

    return ret;
}

/**
 * putcadjmap - Set color adjust curves for the LCD
 * @vtable : Reference to lcdc driver interface
 * @cadjmap  :  Reference to dest color adjust table structure
 *
 *  Entrypoint ioctl FBIOPUT_CADJMAP
 * Set color adjust curves for the LCD
 *
 * Returns:   OK on success, -EINVAL on failure.
 */
static int putcadjmap(struct fb_vtable_s *vtable,
              const struct fb_cadjmap_s *cadjmap)
{
    int ret;
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    ASSERT(vtable != NULL && priv == &g_vtable
           && cadjmap != NULL, "%s", __func__);
    LCDC_INFO("vtable=%p cadjmap=%p\n", vtable, cadjmap);

    if (cadjmap->first >= LCDC_NCLUT) {
        LCDC_ERR("ERROR: only %d color table entries supported\n",
             LCDC_NCLUT);
        ret = -EINVAL;
    } else {
        /* Currently, there is no api to set color map for each overlay
         * separately. LCDC layers can have different color maps, but is shared
         * for now.
         */

        int n;
        FAR struct fb_cadjmap_s *priv_cmap = &g_vtable.cadjmap;

        /* First copy to internal cmap */

        for (n = cadjmap->first; n < cadjmap->len && n < LCDC_NCLUT;
             n++) {
            priv_cmap->red[n] = cadjmap->red[n];
            priv_cmap->green[n] = cadjmap->green[n];
            priv_cmap->blue[n] = cadjmap->blue[n];
        }

        priv_cmap->first = cadjmap->first;
        priv_cmap->len = cadjmap->len;

        /* Update the layer clut register */
        gra_sem_wait(&g_lock);
        for (n = 0; n < LCDC_NLAYERS; n++) {
            FAR struct lcdc_s *layer = &priv->layer[n];
            lcdc_lputcadjcurves(layer, priv_cmap);
        }
        gra_sem_post(&g_lock);
        ret = OK;
    }

    return ret;
}
#endif /* CONFIG_FB_COLORADJCURVE */

#ifdef CONFIG_FB_COLORDITHER
/**
 *lcdc_setcadjcurves - Configure color dither table
 *@cadjmap : fb color adjust curves
 */
static int lcdc_setcdither(FAR const struct fb_cdithermap_s *cdithermap)
{
    int n;
    uint32_t crreg;
    uint32_t citem;
    struct lcdcdev_s *priv = &g_vtable;

    for (n = cdithermap->first; n < cdithermap->len && n < 4; n++) {
        priv->cdithermap.data[n] = cdithermap->data[n];
        citem = priv->cdithermap.data[n];

        hal_lcdc_dither_write(n, citem);
        // hal_sys_timer_delay_us(1000);
    }

    return OK;
}

/**
 * putcadjmap - Set color adjust curves for the LCD
 * @vtable : Reference to lcdc driver interface
 * @cadjmap  :  Reference to source color adjust table structure
 *
 * Entrypoint ioctl FBIOPUT_CADJMAP
 * Set color adjust curves for the LCD
 *
 * Returns:   OK on success, -EINVAL on failure.
 */
static int putcdithermap(struct fb_vtable_s *vtable,
             const struct fb_cdithermap_s *cdithermap)
{
    int ret;
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    ASSERT(vtable != NULL && priv == &g_vtable
           && cdithermap != NULL, "%s", __func__);
    LCDC_INFO("vtable=%p cdithermap=%p\n", vtable, cdithermap);

    if (cdithermap->first >= 4) {
        LCDC_ERR
            ("ERROR: only %d color dither  table entries supported\n",
             4);
        ret = -EINVAL;
    } else {
        /* Currently, there is no api to set color map for each overlay
         * separately. LCDC layers can have different color maps, but is shared
         * for now.
         */

        int n;
        FAR struct fb_cdithermap_s *priv_cmap = &g_vtable.cdithermap;

        /* First copy to internal cmap */

        for (n = cdithermap->first; n < cdithermap->len && n < 4; n++) {
            priv_cmap->data[n] = cdithermap->data[n];
            //LCDC_REP("cdithermap->data[%d]: 0x%x", n, cdithermap->data[n]);
        }

        priv_cmap->first = cdithermap->first;
        priv_cmap->len = cdithermap->len;

        /* Update the layer clut register */
        gra_sem_wait(&g_lock);
        lcdc_setcdither(priv_cmap);
        gra_sem_post(&g_lock);
        ret = OK;
    }

    return ret;
}
#endif /* CONFIG_FB_COLORDITHER */

/**
 * waitforvsync - wait for vsync signal
 * @vtable : Reference to lcdc driver interface
 *
 * Entrypoint ioctl FBIO_WAITFORSYNC
 */
#ifdef CONFIG_FB_SYNC
extern int lcdc_waitforirq(void);

#ifdef CONFIG_LCDC_TE_IRQ_ENABLE
/**
 * waitfor_vsync - Helper waits until the dsi te irq occurs.
 */
static int wait_for_vsync(void)
{
    int ret = OK;
    FAR struct teinterrupt_s *priv = &g_teinterrupt;
    //TC_INIT
    LCDC_REGINFO("wait_for_vsync;");
    //TC_START
    ret = gra_sem_wait(priv->sem);

    if (ret < 0) {
        LCDC_ERR("ERROR: gra_sem_wait() failed\n");
    }

    ret = priv->error;

    return ret;
}
#endif

static int waitforvsync(FAR struct fb_vtable_s *vtable)
{
    int ret;

    ASSERT(vtable != NULL && vtable == &g_vtable.vtable, "%s", __func__);
    /* Wait upon vertical synchronization. */
#if defined(CONFIG_FB_PANDISPLAY)
    ret = lcdc_pandisp_trywait();
#elif defined(CONFIG_LCDC_TE_IRQ_ENABLE)
    ret = wait_for_vsync();
#else /* just use framedone */
    ret = lcdc_waitforirq();
#endif
    return ret;
}
#endif /* CONFIG_FB_SYNC */

/**
 * getoverlayinfo - get lcdc overlay info
 * @vtable : Reference to lcdc driver interface
 * @overlayno : fb overlay index
 * @oinfo : Reference to dest fb overlay info structure
 *
 * Entrypoint ioctl FBIOGET_OVERLAYINFO
 */
#ifdef CONFIG_FB_OVERLAY
static int getoverlayinfo(FAR struct fb_vtable_s *vtable,
              int overlayno, FAR struct fb_overlayinfo_s *oinfo)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    LCDC_INFO("vtable=%p overlay=%d oinfo=%p\n", vtable, overlayno, oinfo);
    ASSERT(vtable != NULL && priv == &g_vtable, "%s", __func__);

    if (overlayno < LCDC_NOVERLAYS) {
        FAR struct lcdc_s *layer = &priv->layer[overlayno];
        memcpy(oinfo, &layer->oinfo, sizeof(struct fb_overlayinfo_s));
        LCDC_INFO("oinfo->fbmem=%p\n", oinfo->fbmem);
        return OK;
    }

    LCDC_ERR("ERROR: Returning EINVAL\n");
    return -EINVAL;
}

/**
 * settransp - set fb overlay transparency
 * @vtable : Reference to lcdc driver interface
 * @oinfo : Reference to fb overlay info structure
 *
 * Entrypoint ioctl FBIOSET_TRANSP
 */

static int settransp(FAR struct fb_vtable_s *vtable,
             FAR const struct fb_overlayinfo_s *oinfo)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;
    LCDC_INFO("vtable=%p, overlay=%d, transp=%02x, transp_mode=%02x\n",
          vtable, oinfo->overlay, oinfo->transp.transp,
          oinfo->transp.transp_mode);

    ASSERT(vtable != NULL && priv == &g_vtable, "%s", __func__);

    if (oinfo->transp.transp_mode > 1) {
        LCDC_ERR
            ("ERROR: Returning ENOSYS, transparency mode not supported\n");
        return -ENOSYS;
    }

    if (oinfo->overlay < LCDC_NOVERLAYS) {
        FAR struct lcdc_s *layer = &priv->layer[oinfo->overlay];

        gra_sem_wait(layer->lock);
        layer->oinfo.transp.transp = oinfo->transp.transp;
        layer->oinfo.transp.transp_mode = oinfo->transp.transp_mode;

        /* Set LCDC blendmode and alpha value */
        lcdc_ltransp(layer, layer->oinfo.transp.transp,
                     layer->oinfo.transp.transp_mode);

        gra_sem_post(layer->lock);
        return OK;
    }

    LCDC_ERR("ERROR: Returning EINVAL\n");
    return -EINVAL;
}

#ifdef CONFIG_LCDC_CHROMAKEY

/**
 * setchromakey - set fb layers chromakey
 * @vtable : Reference to lcdc driver interface
 * @oinfo : Reference to fb overlay info structure
 *
 * Entrypoint ioctl FBIOSET_CHROMAKEY
 * Returns:   OK on success, -errno on failure.
 */
static int setchromakey(FAR struct fb_vtable_s *vtable,
            FAR const struct fb_overlayinfo_s *oinfo)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;
    ASSERT(vtable != NULL && priv == &g_vtable
           && oinfo != NULL, "%s", __func__);
    LCDC_INFO("vtable=%p, overlay=%d, chromakey=%08x\n", vtable,
          oinfo->overlay, oinfo->chromakey);

    if (oinfo->overlay < LCDC_NLAYERS) {
        int ret;
        FAR struct lcdc_s *layer = &priv->layer[oinfo->overlay];

#ifndef CONFIG_LCDC_L1_CHROMAKEY
        if (oinfo->overlay == LCDC_LAYER_L1) {
            return -ENOSYS;
        }
#endif
#ifndef CONFIG_LCDC_L2_CHROMAKEY
        if (oinfo->overlay == LCDC_LAYER_L2) {
            return -ENOSYS;
        }
#endif

        gra_sem_wait(layer->lock);
#ifdef CONFIG_FB_CMAP
        if (oinfo->chromakey >= g_vtable.cmap.len) {
            LCDC_ERR("ERROR: Clut index %d is out of range\n",
                 oinfo->chromakey);
            ret = -EINVAL;
        } else
#endif
        {
            layer->oinfo.chromakey = oinfo->chromakey;

            /* Set chromakey */
            lcdc_lchromakey_enable(layer, layer->oinfo.chromakey, 0);
            ret = OK;
        }

        gra_sem_post(layer->lock);
        return ret;
    }

    LCDC_ERR("ERROR: Returning EINVAL\n");
    return -EINVAL;
}

#endif

/**
 * setblank - blank fb layer
 * @vtable : Reference to lcdc driver interface
 * @oinfo : Reference to fb overlay info structure
 *
 * Entrypoint ioctl FBIOSET_BLANK
 * Returns:   OK on success, -errno on failure.
 */
static int setblank(FAR struct fb_vtable_s *vtable,
            FAR const struct fb_overlayinfo_s *oinfo)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    ASSERT(vtable != NULL && priv == &g_vtable
           && oinfo != NULL, "%s", __func__);
    LCDC_INFO("vtable=%p, overlay=%d, blank=%02x\n", vtable ,oinfo->overlay ,oinfo->blank);

    if (oinfo->overlay < LCDC_NLAYERS) {
        FAR struct lcdc_s *layer = &priv->layer[oinfo->overlay];
        gra_sem_wait(layer->lock);
        layer->oinfo.blank = oinfo->blank;

        /* Enable or disable layer */

        if (layer->oinfo.blank == 0)
            lcdc_layer_enable(layer->layerno);
        gra_sem_post(layer->lock);
        return OK;
    }

    LCDC_ERR("ERROR: Returning EINVAL\n");
    return -EINVAL;
}

#endif /* CONFIG_FB_OVERLAY */

/**
 * lcdcreset - Reset LCDC via APB2RSTR
 *
 * Returns:   OK on success, -errno on failure.
 *
 */
void lcdcreset(void)
{
    lcdc_reset();
}

#if defined(SIMU) && !defined(CONFIG_DSI_VIDEO_MODE)
static HWTIMER_ID hw_timer = NULL;
static void timer_handler(void *param)
{
    lcdc_vsync_cb();
    hwtimer_start(hw_timer, MS_TO_TICKS(17));
}
#endif

#ifdef SUPPORT_REPLACE_FBMEM
static HWTIMER_ID fb_replace_timer = NULL;
static void fb_replace_handler(void *param)
{
    hwtimer_free(fb_replace_timer);
    fb_replace_timer = NULL;
    fb_replace_ok = true;
}
#endif

/**
 * lcdc_initialize - Initialize the lcdc controller
 *
 * Returns:   OK on success, -errno on failure.
 */

FAR struct lcd_dev_s *board_lcd_getdev(int lcddev);
int lcdc_initialize(void *fbmem, uint32_t fbsize)
{
    int ret = OK;
    LCDC_REP("Initialize LCDC driver fbmem:%p,fbsize:%d\n",fbmem,fbsize);
    if (g_initialized == true) {
        return ret;
    }
    lcdc_get_config(&lcdc_config);
    if (lcdc_config == NULL) {
        LCDC_REP("Failed to get lcdc_config.");
        return -1;
    }
    g_vtable.lcddev = (struct lcd_dev_s *)board_lcd_getdev(0);

#ifdef SUPPORT_REPLACE_FBMEM
    if (fbmem == NULL)
        fbmem = memalign(64,CONFIG_LCDC_FB_SIZE);
    ASSERT(fbmem, "fbmem malloc fail!!");
    memset(fbmem, 0, CONFIG_LCDC_FB_SIZE);
    if (fb_replace_timer == NULL)
        fb_replace_timer = hwtimer_alloc(fb_replace_handler, 0);
    hwtimer_start(fb_replace_timer, MS_TO_TICKS(5*1000));
#endif

    lcdc_setup_layer_fbmem(fbmem, fbsize);

    /* Configure interrupts */
    LCDC_INFO("Configure interrupts\n");

    lcdc_irqconfig();

#if defined(CONFIG_LCDC_TE_IRQ_ENABLE)
    LCDC_INFO("Configure dsi interrupts\n");
    lcdc_teirqconfig();
#endif

    /* Configure global lcdc register */
    LCDC_INFO("Configure global register\n");

#ifdef CONFIG_LCDC_DITHER
    lcdc_dither_enable(true, LCDC_DITHER_LEVEL, LCDC_DITHER_RED, LCDC_DITHER_GREEN,
                       LCDC_DITHER_BLUE);
#else
    lcdc_dither_disable();
#endif

#ifdef CONFIG_FB_CMAP
    /* Cleanup clut */
    memset(&g_redclut, 0, LCDC_NCLUT);
    memset(&g_blueclut, 0, LCDC_NCLUT);
    memset(&g_greenclut, 0, LCDC_NCLUT);
#ifdef CONFIG_FB_TRANSPARENCY
    memset(&g_transpclut, 0, LCDC_NCLUT);
#endif
#endif /* CONFIG_FB_CMAP */

    /* Initialize lcdc layer */

    LCDC_INFO("Initialize lcdc layer\n");
    lcdc_linit(LCDC_LAYER_L1);
#ifdef CONFIG_LCDC_L2
    lcdc_linit(LCDC_LAYER_L2);
#endif

    fb_ratate_init(g_vtable.vinfo.xres,g_vtable.vinfo.yres);

#if defined(CONFIG_FB_DRV_ROTATE)
    memset((void *)dst_fb_buf, 0, sizeof(dst_fb_buf));
    lcdc_layer_set_buffer(LCDC_LAYER_L1, dst_fb_buf);
    lcdc_cache_sync((uint32_t)dst_fb_buf, LCDC_L1_STRIDE*LCDC_HEIGHT);
#endif

#ifdef CONFIG_DSI_VIDEO_MODE
#if defined(CONFIG_FB_PANDISPLAY) && defined(CONFIG_FB_DOUBLE_BUFFER)
    lcdc_layer_set_buffer(LCDC_LAYER_L1, (fbmem + fbsize/2));
    lcdc_cache_sync((uint32_t)(fbmem + fbsize/2), LCDC_L1_STRIDE*LCDC_HEIGHT);
#endif
#endif

    /* Enable the backlight */
#ifdef CONFIG_LCD_BACKLIGHT
    backlight(true);
#endif

    LCDC_INFO("Enabling the display\n");

    /* Set initialized state */
#ifdef CONFIG_PM
    g_vtable.ref_cnt = 0;
    g_vtable.ref_cnt_cmd = 0;
    pmu_dsi_aod_mode_enable(true);
    pm_register(&pm_lcdc);
    lcdc_pmcb_register(&panel_pmcb);
#endif

#if defined(CONFIG_FB_PANDISPLAY)
    LCDC_INFO("Init mutible buffer support.\n");
    lcdc_pandisp_init();
#endif

    lcdc_start();

#ifndef CONFIG_DSI_VIDEO_MODE
#if defined(SIMU)
    hw_timer = hwtimer_alloc(timer_handler, 0);
    hwtimer_start(hw_timer, MS_TO_TICKS(17));
#endif
    lcdc_enter_lowpower();
#endif
    LCDC_REP("lcdc_initialize done\n");
    g_initialized = true;
    return ret;
}

int lcdc_irq_callback_register(LCDC_IRQ_CALLBACK cb)
{
    g_interrupt.cb = cb;
    return 0;
}

#ifdef CONFIG_LCDC_TE_IRQ_ENABLE
int lcdc_teirq_callback_register(LCDC_IRQ_CALLBACK cb)
{
    g_teinterrupt.cb = cb;
    return 0;
}
#endif

/**
 * lcdcgetvplane - Get a reference to the lcdc driver interface for the specified video
 * @vplane : video plane number
 *
 * Returns reference to the lcdc driver interface for the specified video
 * plane.
 *
 */
struct fb_vtable_s *lcdcgetvplane(int vplane)
{
    LCDC_INFO("vplane: %d\n", vplane);

    if (vplane == 0) {
        return &g_vtable.vtable;
    }

    return NULL;
}

/**
 * lcdcuninitialize - Uninitialize the framebuffer lcdc driver
 *
 * Uninitialize the framebuffer driver.  Bad things will happen if you
 * call this without first calling fb_initialize()!
 */
void lcdcuninitialize(void)
{
    /* Disable all lcdc interrupts */
    lcdc_stop();

#ifdef SUPPORT_REPLACE_FBMEM
    if (fb_replace_timer)
        hwtimer_free(fb_replace_timer);
    fb_replace_timer = NULL;
#endif

    /* clear semaphores */
    gra_sem_destroy(&g_lock);
    gra_sem_destroy(g_interrupt.sem);

#if defined(CONFIG_LCDC_TE_IRQ_ENABLE)
    gra_sem_destroy(g_teinterrupt.sem);
#endif

    lcdc_exit();

    lcdc_config = NULL;

    /* Set initialized state */
    g_initialized = false;
}

/**
 * lcd_backlight - turn the backlight on and off
 *
 * Provide this interface to turn the backlight on and off.
 *
 */
#ifdef CONFIG_LCD_BACKLIGHT
void backlight(bool blon)
{
    /* Set default backlight level CONFIG_STM32_LCDC_DEFBACKLIGHT */

    LCDC_ERR("ERROR: Not supported\n");
}
#endif

static bool lcdc_address_nocached(uint32_t addr, uint32_t len)
{
#ifdef LCDC_DRV_WITH_CACHEOPS
    return false;
#endif
#if defined(PSRAM_NC_BASE) && defined(PSRAM_SIZE) && (PSRAM_SIZE > 0)
    if (addr >= PSRAM_NC_BASE && addr < (PSRAM_NC_BASE+PSRAM_SIZE))
        return true;
#endif
#if defined(PSRAMUHS_NC_BASE) && defined(PSRAMUHS_SIZE) && (PSRAMUHS_SIZE > 0)
    if (addr >= PSRAMUHS_NC_BASE && addr < (PSRAMUHS_NC_BASE+PSRAMUHS_SIZE))
        return true;
#endif
#if defined(M55_DTCM_BASE) && defined(M55_DTCM_SIZE) && (M55_DTCM_SIZE > 0)
    if (addr >= M55_DTCM_BASE && addr < (M55_DTCM_BASE+M55_DTCM_SIZE))
        return true;
#endif
#if defined(M55C0_DTCM_BASE) && defined(M55C0_DTCM_SIZE) && (M55C0_DTCM_SIZE > 0)
    if (addr >= M55C0_DTCM_BASE && addr < (M55C0_DTCM_BASE+M55C0_DTCM_SIZE))
        return true;
#endif
#if defined(M55C1_DTCM_BASE) && defined(M55C1_DTCM_SIZE) && (M55C1_DTCM_SIZE > 0)
    if (addr >= M55C1_DTCM_BASE && addr < (M55C1_DTCM_BASE+M55C1_DTCM_SIZE))
        return true;
#endif
    return true;
}

/**lcdc_cache_sync - Flush cache to external memory
 * @id : cache ID
 * @start_address  buffer start address
 * @len: buffer length
 *
 * NOTE: make sure that the buffer must allocted in a safe flush mem range
 *
    |       len            |      used range
  | s  |    32*n        |  e  |   safe range
    32                     32
*/
uint8_t lcdc_cache_sync(uint32_t start_address, uint32_t len)
{
    if (lcdc_address_nocached(start_address, len))
        return 0;
#ifndef __ARM_ARCH_ISA_ARM
    return hal_cache_sync(HAL_CACHE_ID_D_CACHE, start_address, len);
#else
#if defined(__NuttX__)
#if defined(CONFIG_SMP)
    up_invalidate_dcache(start_address, start_address+len);
#else
    up_clean_dcache(start_address,start_address+len);
#endif
    return 0;
#endif
    return 0;
#endif
}

/**lcdc_cache_invalidate - sync external memory to cpu
 * @id : cache ID
 * @start_address  buffer start address
 * @len: buffer length
 *
 * NOTE: make sure that the buffer must allocted in a safe flush mem range
 *
    |       len            |      used range
  | s  |    32*n        |  e  |   safe range
    32                     32
*/
uint8_t lcdc_cache_invalidate(uint32_t start_address, uint32_t len)
{
    if (lcdc_address_nocached(start_address, len))
        return 0;
#ifndef __ARM_ARCH_ISA_ARM
    return hal_cache_invalidate(HAL_CACHE_ID_D_CACHE, start_address, len);
#else
#if defined(__NuttX__)
    up_invalidate_dcache(start_address, start_address+len);
    return 0;
#endif
    return 0;
#endif
}

/**
 * lcdc_trigger_frame - trigger lcdc to generate one frame for display
 */
void lcdc_trigger_frame(void)
{
#ifndef CONFIG_DSI_VIDEO_MODE
    if (lcdc_lock()) {
        uint32_t flags = int_lock();
        lcdc_xfer_frame();
        int_unlock(flags);
    }
#endif
}

/**
 * lcdc_update - wait lcdc idle and trigger lcdc to generate one frame for display
 */
int lcdc_update(void)
{
    int ret = 0;
    LCDC_INFO("%s enter", __func__);

#ifdef CONFIG_PM
    module_pm_activity();
#endif

    lcdc_waitforirq();
    lcdc_exit_lowpower();

    lcdc_trigger_frame();

    LCDC_INFO("%s exit", __func__);
    return ret;
}

/**
 * lcdc_update_area - tirger color_p as the fb frame for display
 */
int lcdc_update_area(const struct fb_area_s *area, const uint32_t *color_p)
{
#if defined(CONFIG_FB_PANDISPLAY)
    if (area == NULL) {
        struct fb_planeinfo_s pinfo = {
            .yoffset = (uint32_t)color_p,
        };
        while (pan_display(NULL, &pinfo) == ERROR) {
            lcdc_waitforirq();
        }
        return 0;
    }
    else
#endif
    {
        /** Represents an area of the screen. */
        uint32_t flags;
        flags = int_lock();
        if (area != NULL) {
            if (0 == area->x && area->w == LCDC_WIDTH) {
                lcdc_cache_sync((uint32_t)color_p, LCDC_L1_STRIDE*area->h);
            } else {
                for(int i = 0; i < area->h; i ++) {
                      lcdc_cache_sync((uint32_t)color_p + i * LCDC_L1_STRIDE, area->w * LCDC_L1_BPP / 8);
                  }
            }
        } else {
            lcdc_cache_sync((uint32_t)color_p, LCDC_L1_STRIDE*LCDC_HEIGHT);
        }
        int_unlock(flags);
        lcdc_waitforirq();
        lcdc_exit_lowpower();
        lcdc_layer_set_buffer(0, (uint8_t *)color_p);
        lcdc_trigger_frame();
    }
    return 0;
}

struct fb_overlayinfo_s *lcdc_get_oinfo(int lid)
{
    FAR struct lcdcdev_s *dev = &g_vtable;
    return &dev->layer[lid].oinfo;

}

void lcdc_need_refresh_onec(void)
{
#ifdef CONFIG_PM
    FAR struct lcdcdev_s *dev = &g_vtable;
    if (dev->panel_state == PM_NORMAL || dev->panel_state == PM_IDLE) {
        dev->ref_cnt++;
        dev->refresh_oneself = 1;
    }
#endif
}

void lcd_sleep_disp_domain(void)
{
#ifdef CONFIG_PM
    FAR struct lcdcdev_s *priv = &g_vtable;
    uint32_t irqflag = int_lock();
  /* when cpu sleep will close dsi and lcdc */
  if (priv->sleep == false) {
      lcdc_sleep();
      priv->sleep = true;
  }
  int_unlock(irqflag);
#endif
}

void lcdc_wakeup_disp_domain(void)
{
#ifdef CONFIG_PM
    FAR struct lcdcdev_s *priv = &g_vtable;
    uint32_t irqflag = int_lock();

    if (true == priv->sleep) {
        priv->sleep = false;
        int_unlock(irqflag);
        lcdc_wakeup();
    } else {
        int_unlock(irqflag);
    }
#endif
}

#if defined(CONFIG_FB_DRV_ROTATE)
static osThreadId fb_rotate_tid = NULL;
static sem_t new_fb_sem;

#if defined(ROTATE_DIRECT)
uint8_t buf_bitmap = 0;
//#include <nuttx/spinlock.h>
//spinlock_t g_bitmap_lock;

static uint8_t get_rotate_buf()
{
    uint32_t lk = int_lock();
    //irqstate_t lk = spin_lock_irqsave(&g_bitmap_lock);
    for (int i = 0; i < ROTATE_BUF_COUNT; i++) {
        if ((buf_bitmap & (1<<i)) == 0) {
            buf_bitmap |= (1<<i);
            int_unlock(lk);
            //spin_unlock_irqrestore(&g_bitmap_lock, lk);
            return i;
        }
    }
    int_unlock(lk);
    //spin_unlock_irqrestore(&g_bitmap_lock, lk);
    //ASSERT(0,"no free buf buf_bitmap:0x%x",buf_bitmap);
    LCDC_ERR("no free rotate buf buf_bitmap:0x%x",buf_bitmap);
    return 0xff;
}
static void free_rotate_buf(uint8_t i)
{
    uint32_t lk = int_lock();
    //irqstate_t lk = spin_lock_irqsave(&g_bitmap_lock);
    ASSERT((buf_bitmap & (1<<i)) != 0,"buf_bitmap:0x%x, buf id:%d not used",buf_bitmap,i);
    buf_bitmap &= ~(1<<i);
    int_unlock(lk);
    //spin_unlock_irqrestore(&g_bitmap_lock, lk);
}
#endif

static void dma_irq_done_cb(void *dst_buf)
{
    struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)&g_vtable;
    ASSERT(priv->panfo.cnt > 0,"cnt error:%d\n",priv->panfo.cnt);
    ASSERT(priv->panfo.queue[0].state == BUF_ROTATE,"state error:%d\n",priv->panfo.queue[0].state);
    uint32_t tmp = (uint32_t)priv->panfo.queue[0].addr;
    priv->panfo.queue[0].addr = dst_buf;
    priv->panfo.queue[0].state = BUF_READY;
}

static void fb_rotate_trigger(struct lcdcdev_s *priv)
{
    uint32_t lk = int_lock();
    if (priv->panfo.cnt>0 && priv->panfo.queue[0].state == BUF_INIT)
    {
#if defined(CONFIG_ROTATE_DMA)
        if (!ROTATE_USE_CPU(priv->pinfo.fbmem)) {
            priv->panfo.queue[0].state = BUF_ROTATE;
            dma2d_rotate_90((uint32_t)priv->panfo.queue[0].addr,(uint32_t)dst_fb_buf,LCDC_HEIGHT,LCDC_WIDTH, dma_irq_done_cb);
        }
        else
#endif
        {
            gra_sem_post(&new_fb_sem);
        }
    }
    int_unlock(lk);
}

static void fb_rotate(const void *arg)
{
    osEvent evt;
    uint32_t signals = 0;
    struct lcdcdev_s *priv = (struct lcdcdev_s *)arg;
    while(1)
    {
        gra_sem_wait(&new_fb_sem);
        {
            if (priv->panfo.cnt > 0) {
                LCDC_REP("%s cnt=%d, panaddr = %08x,state = %d\n", __func__,priv->panfo.cnt, priv->panfo.queue[0].addr,priv->panfo.queue[0].state);
                /*The buffer of the sending head node*/
                //ASSERT(priv->panfo.queue[0].state == BUF_INIT, "queue status is wrong :%d",priv->panfo.queue[0].state);
                uint32_t lk = int_lock();
                if (priv->panfo.queue[0].state == BUF_INIT)
                {
                    priv->panfo.queue[0].state = BUF_ROTATE;
                    int_unlock(lk);
                    cpu_rotate_90((uint32_t)priv->panfo.queue[0].addr,(uint32_t)dst_fb_buf,LCDC_HEIGHT,LCDC_WIDTH);
                    priv->panfo.queue[0].addr = dst_fb_buf;
                    priv->panfo.queue[0].state = BUF_READY;
                }
                else
                    int_unlock(lk);
            }
        }
        if (!ROTATE_USE_CPU(priv->pinfo.fbmem))
            return ;
    }
    return ;
}

osThreadDef(fb_rotate, (osPriorityAboveNormal), 1, (4096), "fb_rotate");

static void fb_ratate_init(uint32_t lcd_x,uint32_t lcd_y)
{
    g_vtable.vinfo.xres = lcd_y;
    g_vtable.vinfo.yres = lcd_x;
    g_vtable.pinfo.xres_virtual = lcd_y;
    g_vtable.pinfo.yres_virtual = lcd_x;
    g_vtable.pinfo.stride = g_vtable.pinfo.xres_virtual * g_vtable.pinfo.bpp / 8;
#if defined(CONFIG_FB_DOUBLE_BUFFER)
    g_vtable.pinfo.yres_virtual *= 2;
#endif
#ifdef CONFIG_FB_OVERLAY
    g_vtable.layer[0].oinfo.stride = g_vtable.pinfo.stride;
    g_vtable.layer[0].oinfo.sarea.x = lcd_y;
    g_vtable.layer[0].oinfo.sarea.y = lcd_x;
    //hal_lcdc_lpfc_fmt(g_vtable.layer[0]->layerno, FB_FMT_RGB24);
#ifdef CONFIG_LCDC_L2
    g_vtable.layer[1].oinfo.stride = g_vtable.pinfo.stride;
    g_vtable.layer[1].oinfo.sarea.x = lcd_y;
    g_vtable.layer[1].oinfo.sarea.y = lcd_x;
    //hal_lcdc_lpfc_fmt(g_vtable.layer[1]->layerno, FB_FMT_RGB24);
#endif
#endif
#if !defined(ROTATE_DIRECT)
    if (ROTATE_USE_CPU(g_vtable.pinfo.fbmem))
    {
        gra_sem_init(&new_fb_sem, 0, 0);
        fb_rotate_tid = osThreadCreate(osThread(fb_rotate), &g_vtable);
    }
#endif
}

#else
static void fb_ratate_init(uint32_t lcd_x,uint32_t lcd_y) { };
#endif

#ifndef __NuttX__
void lcdc_pannel_entery_idle_mode(void)
{
    FAR struct lcdcdev_s *priv = &g_vtable;

    ASSERT(priv->lcddev!=NULL,"lcd dev is NULL\n");
    if (priv->lcddev->pannel_ioctl)
        priv->lcddev->pannel_ioctl(priv->lcddev, PANNEL_ENTER_IDLE);
}

void lcdc_pannel_exit_idle_mode(void)
{
    FAR struct lcdcdev_s *priv = &g_vtable;

    ASSERT(priv->lcddev!=NULL,"lcd dev is NULL\n");
    if (priv->lcddev->pannel_ioctl)
        priv->lcddev->pannel_ioctl(priv->lcddev, PANNEL_EXIT_IDLE);
}

void lcdc_pannel_enter_deepstandby(void)
{
    FAR struct lcdcdev_s *priv = &g_vtable;

    ASSERT(priv->lcddev!=NULL,"lcd dev is NULL\n");
    if (priv->lcddev->pannel_ioctl)
        priv->lcddev->pannel_ioctl(priv->lcddev, PANNEL_ENTER_DEEPSLEEP);
}

void lcdc_pannel_exit_deepstandby(void)
{
    FAR struct lcdcdev_s *priv = &g_vtable;

    ASSERT(priv->lcddev!=NULL,"lcd dev is NULL\n");
    if (priv->lcddev->pannel_ioctl)
        priv->lcddev->pannel_ioctl(priv->lcddev, PANNEL_EXIT_DEEPSLEEP);
}

int lcdc_pannel_read_status(void)
{
    FAR struct lcdcdev_s *priv = &g_vtable;

    ASSERT(priv->lcddev!=NULL,"lcd dev is NULL\n");
    if (priv->lcddev->pannel_ioctl)
        return priv->lcddev->pannel_ioctl(priv->lcddev, PANNEL_READSTATUS);
    return 0;
}

int lcdc_pannel_setpower(int power)
{
    FAR struct lcdcdev_s *priv = &g_vtable;

    ASSERT(priv->lcddev!=NULL,"lcd dev is NULL\n");
    if (priv->lcddev->setpower)
        return priv->lcddev->setpower(priv->lcddev, power);
    return 0;
}
#endif

#ifdef LCDC_SIMU_TEST_CODE
#include "lcdc_sim_test.h"
#endif //LCDC_SIMU_TEST_CODE
