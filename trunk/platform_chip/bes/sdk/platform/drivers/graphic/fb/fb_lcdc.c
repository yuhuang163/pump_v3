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
#ifdef __NuttX__
#include <nuttx/config.h>
#include <errno.h>
#include <assert.h>
#include <debug.h>
#include <stdlib.h>
#include <cmsis.h>
#include <nuttx/arch.h>
#include <nuttx/board.h>
#include <nuttx/video/fb.h>
#include <arch/board/board.h>
#include <nuttx/lcd/lcd.h>
#include <nuttx/irq.h>
#include <nuttx/cache.h>
#include <nuttx/power/pm.h>
#ifdef CONFIG_PM
#include <arch/board/pm_domain.h>
#endif
#else
#include "fb_drv.h"
#include "fb_lpcmd.h"
#include "gra_sem.h"
#ifdef RTOS
#include "cmsis_os.h"
#endif
#endif
#include "hal_timer.h"
#include "hal_lcdc.h"
#include "hal_dsi.h"
#include "hal_psc.h"
#include "hal_cache.h"
#include "cmsis_nvic.h"
#include "hal_cmu.h"
#include "hal_sysfreq.h"
#include "hal_trace.h"
#include "hal_location.h"
#include "hal_timer.h"
#include "hwtimer_list.h"

#include "lcdc_drv.h"
#include "lcd_conf.h"

#ifndef ARRAY_SIZE
#define ARRAY_SIZE(a) (sizeof(a) / sizeof(a[0]))
#endif

#ifdef CHIP_BEST2003_DSP
#undef SIMU
#endif

/**
 * Board LCDC Configuration
 */

#define LCDC_HEIGHT                 (BOARD_LCDC_HEIGHT)
#define LCDC_WIDTH                  ((BOARD_LCDC_WIDTH+15)/16*16)

/**
 * LCDC Layer Configuration
 */

/* Layer 1 format */

#define LAYER1_ID           LCDC_LAYER_DMA
#if defined(CONFIG_LCDC_L1_L8)
#define LCDC_L1_BPP         8
#define LCDC_L1_COLOR_FMT   FB_FMT_RGB8
#define LAYER1_FORMAT       LCDC_CFMT_PALETTE_8
#elif defined(CONFIG_LCDC_L1_RGB565)
#define LCDC_L1_BPP         16
#define LCDC_L1_COLOR_FMT   FB_FMT_RGB16_565
#define LAYER1_FORMAT       LCDC_CFMT_RGB565
#elif defined(CONFIG_LCDC_L1_RGB888)
#define LCDC_L1_BPP         24
#define LCDC_L1_COLOR_FMT   FB_FMT_RGB24
#define LAYER1_FORMAT       LCDC_CFMT_RGB888
#elif defined(CONFIG_LCDC_L1_ARGB8888)
#define LCDC_L1_BPP         32
#define LCDC_L1_COLOR_FMT   FB_FMT_RGB32
#define LAYER1_FORMAT       LCDC_CFMT_RGBA8888
#elif defined(CONFIG_LCDC_L1_YUV422_UYVY)
#define LCDC_L1_BPP         16
#define LCDC_L1_COLOR_FMT   FB_FMT_UYVY
#define LAYER1_FORMAT       LCDC_CFMT_YUV422_UYVY
#elif defined(CONFIG_LCDC_L1_YUV420_NV12)
#define LCDC_L1_BPP         8
#define LCDC_L1_COLOR_FMT   FB_FMT_NV12
#define LAYER1_FORMAT       LCDC_CFMT_YUV420_NV12
#else
#error "LCDC pixel format not supported"
#endif

/* Layer 2 format */

#ifdef CONFIG_LCDC_L2
#define LAYER2_ID           LCDC_LAYER_GRA
#if defined(CONFIG_LCDC_L2_L8)
#define LCDC_L2_BPP         8
#define LCDC_L2_COLOR_FMT   FB_FMT_RGB8
#define LAYER2_FORMAT       LCDC_CFMT_PALETTE_8
#elif defined(CONFIG_LCDC_L2_RGB565)
#define LCDC_L2_BPP         16
#define LCDC_L2_COLOR_FMT   FB_FMT_RGB16_565
#define LAYER2_FORMAT       LCDC_CFMT_RGB565
#elif defined(CONFIG_LCDC_L2_RGB888)
#define LCDC_L2_BPP         24
#define LCDC_L2_COLOR_FMT   FB_FMT_RGB24
#define LAYER2_FORMAT       LCDC_CFMT_RGB888
#elif defined(CONFIG_LCDC_L2_ARGB8888)
#define LCDC_L2_BPP         32
#define LCDC_L2_COLOR_FMT   FB_FMT_RGB32
#define LAYER2_FORMAT       LCDC_CFMT_RGBA8888
#else
#error "LCDC pixel format not supported"
#endif
#endif /* CONFIG_LCDC_L2 */

/* Framebuffer sizes in bytes */

#if LCDC_L1_BPP == 8
#define LCDC_L1_STRIDE      (LCDC_WIDTH)
#elif LCDC_L1_BPP == 16
#define LCDC_L1_STRIDE      (LCDC_WIDTH * 2)
#elif LCDC_L1_BPP == 24
#define LCDC_L1_STRIDE      (LCDC_WIDTH * 3)
#elif LCDC_L1_BPP == 32
#define LCDC_L1_STRIDE      (LCDC_WIDTH * 4)
#else
#error Undefined or unrecognized base resolution
#endif

#ifdef CONFIG_FB_DOUBLE_BUFFER
#define LCDC_L1_FBSIZE        (LCDC_L1_STRIDE * LCDC_HEIGHT * 2)
#else
#define LCDC_L1_FBSIZE        (LCDC_L1_STRIDE * LCDC_HEIGHT)
#endif

#ifdef CONFIG_LCDC_L2
#if LCDC_L2_BPP == 8
#define LCDC_L2_STRIDE    (LCDC_WIDTH)
#elif LCDC_L2_BPP == 16
#define LCDC_L2_STRIDE    (LCDC_WIDTH * 2)
#elif LCDC_L2_BPP == 24
#define LCDC_L2_STRIDE    (LCDC_WIDTH * 3)
#elif LCDC_L2_BPP == 32
#define LCDC_L2_STRIDE    (LCDC_WIDTH * 4)
#else
#error Undefined or unrecognized base resolution
#endif

#ifdef CONFIG_FB_DOUBLE_BUFFER
#define LCDC_L2_FBSIZE        (LCDC_L2_STRIDE * LCDC_HEIGHT * 2)
#else
#define LCDC_L2_FBSIZE        (LCDC_L2_STRIDE * LCDC_HEIGHT)
#endif

#else
#define LCDC_L2_FBSIZE (0)
#endif /* CONFIG_LCDC_L2 */

#ifdef CONFIG_FB_DOUBLE_BUFFER
#define LCDC_FB_MAX_SIZE        (LCDC_WIDTH * 4 * LCDC_HEIGHT * 2)
#else
#define LCDC_FB_MAX_SIZE        (LCDC_WIDTH * 4 * LCDC_HEIGHT)
#endif

#ifdef __NuttX__
#define LCDC_INFO    lcdinfo
#define LCDC_ERR     lcderr
#else
/* Debug option */
//#define GRAPHIC_DRV_DEBUG
//#define GRAPHIC_DRV_VERBOSE

#ifdef  GRAPHIC_DRV_VERBOSE
#ifndef GRAPHIC_DRV_DEBUG
#define GRAPHIC_DRV_DEBUG
#endif
#define LCDC_INFO(...)  TR_INFO( TR_MOD(TEST), "//"  __VA_ARGS__)
#else
#define LCDC_INFO(...)
#endif

#define LCDC_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)
#endif

/* LCDC layer */

#ifdef CONFIG_LCDC_L2
#define LCDC_NLAYERS 2
#else
#define LCDC_NLAYERS 1
#endif

#define LCDC_NOVERLAYS              LCDC_NLAYERS

uint32_t __attribute__((aligned(64))) layer1_buffer[LCDC_FB_MAX_SIZE/4] __attribute__((section(".psram_nc_bss")));
#define LCDC_BUFFER_L1 ((uint32_t)(&layer1_buffer[0]))
#ifdef CONFIG_LCDC_L2
uint32_t __attribute__((aligned(64))) layer2_buffer[LCDC_FB_MAX_SIZE/4] __attribute__((section(".psram_nc_bss")));
#define LCDC_BUFFER_L2 ((uint32_t)(&layer2_buffer[0]))
#endif

#define FBIOSET_FMT          _FBIOC(0x0030)
#define FBIOSET_GAMMA        _FBIOC(0x0031)

/*
 * cmd type for FBIOSND_CMD
 */
#define CMD_TYPE_READ   0
#define CMD_TYPE_WRITE  1
#define CMD_TYPE_DELAY  2

/****************************************************************************
 * Private Types
 ****************************************************************************/
struct lcdc_s {
    int lid;
#ifdef CONFIG_FB_OVERLAY
    struct fb_overlayinfo_s oinfo;
#endif
    bool is_yuv;
    volatile bool blank_trigger;
    volatile bool fmt_trigger;
    volatile bool ckey_trigger;
};

enum dispbuf_states_s {
    BUF_NONE = 0,
    BUF_INIT,
    BUF_ROTATE,
    BUF_READY,
    BUF_COMMIT,
};

struct paninfo_buf_s {
    void *addr;
    uint32_t yoffset;
    uint32_t current_frame;
    int pan_id;
    enum dispbuf_states_s state;
};

struct paninfo_s {
    volatile struct paninfo_buf_s queue[2];
    volatile int cnt;
    volatile int frame_num;
};

#ifdef CONFIG_LCDC_GAMMA_ENABLE
struct fb_gammainfo {
    bool enable;
    uint8_t *glut;
    uint16_t glut_cnt;
    volatile bool gamma_trigger;
};

static uint8_t gamma_values[256]; /* Table of 8-bit gamma values */
#endif

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
    struct lcdc_s layer[LCDC_NOVERLAYS + 1];

    struct paninfo_s panfo[LCDC_NOVERLAYS];

    struct lcd_dev_s *lcddev;
#ifdef CONFIG_FB_CMAP
    struct fb_cmap_s fb_cmap;
#endif
#ifdef CONFIG_LCDC_GAMMA_ENABLE
    struct fb_gammainfo fb_gamma;
#endif
#ifdef CONFIG_PM
    enum pm_state_e panel_state;
    volatile bool sleep;
#endif
    volatile bool transfering;
    volatile bool start;
};

/**
 * Private Function Prototypes
 */
static void lcdc_linit(uint8_t lid);

static int getvideoinfo(FAR struct fb_vtable_s *vtable,
            FAR struct fb_videoinfo_s *vinfo);
static int getplaneinfo(FAR struct fb_vtable_s *vtable,
            int planeno, FAR struct fb_planeinfo_s *pinfo);
static int setpower(FAR struct fb_vtable_s *vtable, int power);
static int getpower(FAR struct fb_vtable_s *vtable);
static int setchromakey(FAR struct fb_vtable_s *vtable,
                        FAR const struct fb_overlayinfo_s *oinfo);

uint32_t lcdc_memaddress(struct fb_overlayinfo_s *poverlay,
             uint32_t xpos, uint32_t ypos);
/**
 * The following is provided only if the video hardware signals vertical
 * synchronisation
 */

#ifdef CONFIG_FB_SYNC
static int waitforvsync(FAR struct fb_vtable_s *vtable);
#endif
#ifdef CONFIG_FB_PANDISPLAY
static int pan_display(FAR struct fb_vtable_s *vtable,
                       FAR struct fb_planeinfo_s *pinfo);
#endif

/**
 *  The following is provided only if the video hardware supports overlays
 */

#ifdef CONFIG_FB_OVERLAY
static int getoverlayinfo(FAR struct fb_vtable_s *vtable,
              int overlayno, FAR struct fb_overlayinfo_s *oinfo);
static int settransp(FAR struct fb_vtable_s *vtable,
             FAR const struct fb_overlayinfo_s *oinfo);
static int setblank(FAR struct fb_vtable_s *vtable,
            FAR const struct fb_overlayinfo_s *oinfo);
static int pan_overlay(FAR struct fb_vtable_s *vtable,
                       FAR const struct fb_overlayinfo_s *oinfo);
#endif /* CONFIG_FB_OVERLAY */

#ifdef CONFIG_FB_CMAP
static uint8_t red_clut[256];           /* Table of 8-bit red values */
static uint8_t green_clut[256];         /* Table of 8-bit green values */
static uint8_t blue_clut[256];          /* Table of 8-bit blue values */
#ifdef CONFIG_FB_TRANSPARENCY
static uint8_t transp_clut[256];        /* Table of 8-bit transparency */
#endif
static uint32_t rgb_clut[256];          /* Table of 24/32-bit rgb values */

static int getcmap(FAR struct fb_vtable_s *vtable,
                   FAR struct fb_cmap_s *cmap);
static int putcmap(FAR struct fb_vtable_s *vtable,
                   FAR const struct fb_cmap_s *cmap);
#endif

static int ioctl(FAR struct fb_vtable_s *vtable, int cmd, unsigned long arg);

/**
 * Private Data
 */

#ifdef __NuttX__
int gra_sem_init(sem_t *sem, int pshared, unsigned int value);
int gra_sem_destroy (sem_t *sem);
int gra_sem_wait(sem_t *sem);
int gra_sem_trywait(sem_t *sem);
int gra_sem_post(sem_t *sem);
int gra_sem_get_value(sem_t *sem, int *value);

int lpcmds_flush(void);
#endif

/* The semaphore for interrupt handling */

static sem_t g_semirq;

/**
 *  struct lcdcdev_s g_vtable - lcdc driver instance
*/

static struct lcdcdev_s g_vtable = {
    .vtable = {
           .getvideoinfo = getvideoinfo,
           .getplaneinfo = getplaneinfo,
           .setpower = setpower,
           .getpower = getpower,
           .ioctl = ioctl,
           .setchromakey = setchromakey,
#ifdef CONFIG_FB_CMAP
           .getcmap = getcmap,
           .putcmap = putcmap,
#endif
#ifdef CONFIG_FB_SYNC
           .waitforvsync = waitforvsync,
#endif
#ifdef CONFIG_FB_PANDISPLAY
           .pandisplay = pan_display,
#endif
#ifdef CONFIG_FB_OVERLAY
           .getoverlayinfo = getoverlayinfo,
           .settransp = settransp,
           .setblank = setblank,
           .panoverlay = pan_overlay,
#endif /* CONFIG_FB_OVERLAY */
           },
    .pinfo = {
          .fblen = LCDC_L1_FBSIZE,
          .stride = LCDC_L1_STRIDE,
          .display = 0,
          .bpp = LCDC_L1_BPP,
          .xres_virtual = LCDC_WIDTH,
#ifdef CONFIG_FB_DOUBLE_BUFFER
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
          },
    .layer[0] = {
                 .lid = LAYER1_ID,
#ifdef CONFIG_FB_OVERLAY
                 .oinfo = {
                       .fblen = LCDC_L1_FBSIZE,
                       .stride = LCDC_L1_STRIDE,
                       .overlay = 0,
                       .bpp = LCDC_L1_BPP,
                       .blank = 0,
                       .chromakey = 0,
                       .color = 0,
                       .xres = LCDC_WIDTH,
                       .yres = LCDC_HEIGHT,
                       .xres_virtual = LCDC_WIDTH,
#ifdef CONFIG_FB_DOUBLE_BUFFER
                       .yres_virtual = LCDC_HEIGHT * 2,
#else
                       .yres_virtual = LCDC_HEIGHT,
#endif
                       .xoffset = 0,
                       .yoffset = 0,
                       .transp = {
                              .transp = 0x7f,
                              .transp_mode =
                              FB_CONST_ALPHA},
                       .sarea = {
                             .x = 0,
                             .y = 0,
                             .w = LCDC_WIDTH,
                             .h = LCDC_HEIGHT},
                 },
#endif
                 .blank_trigger = false,
                 .fmt_trigger   = false,
                 .ckey_trigger   = false,
    },
#ifdef CONFIG_LCDC_L2
    .layer[1] = {
                 .lid = LAYER2_ID,
#ifdef CONFIG_FB_OVERLAY
                 .oinfo = {
                       .overlay = 1,
                       .fblen = LCDC_L2_FBSIZE,
                       .stride = LCDC_L2_STRIDE,
                       .bpp = LCDC_L2_BPP,
                       .blank = 0,
                       .chromakey = 0,
                       .color = 0,
                       .xres = LCDC_WIDTH,
                       .yres = LCDC_HEIGHT,
                       .xres_virtual = LCDC_WIDTH,
#ifdef CONFIG_FB_DOUBLE_BUFFER
                       .yres_virtual = LCDC_HEIGHT * 2,
#else
                       .yres_virtual = LCDC_HEIGHT,
#endif
                       .xoffset = 0,
                       .yoffset = 0,
                       .transp = {
                              .transp = 0x7f,
                              .transp_mode =
                              FB_CONST_ALPHA},
                       .sarea = {
                             .x = 0,
                             .y = 0,
                             .w = LCDC_WIDTH,
                             .h = LCDC_HEIGHT},
                 },
#endif
                 .blank_trigger = false,
                 .fmt_trigger   = false,
                 .ckey_trigger   = false,
    },
#endif
#ifdef CONFIG_FB_CMAP
    .fb_cmap = {
            .first = 0,
            .len = 0,
            .red = red_clut,
            .green = green_clut,
            .blue = blue_clut,
#ifdef CONFIG_FB_TRANSPARENCY
            .transp = transp_clut,
#endif
    },
#endif
#ifdef CONFIG_LCDC_GAMMA_ENABLE
    .fb_gamma = {
            .enable = false,
            .glut = gamma_values,
            .glut_cnt = 0,
            .gamma_trigger = false,
    },
#endif
};

/* LCDC stride */
static uint32_t stride_layer_t[LCDC_NLAYERS] = {
    LCDC_L1_STRIDE,
#ifdef CONFIG_LCDC_L2
    LCDC_L2_STRIDE,
#endif
};

/* LCDC framebuffer len */
POSSIBLY_UNUSED
static uint32_t fblen_layer_t[LCDC_NLAYERS] = {
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

/* LCDC pixel format lookup table */
static uint8_t fmt_layer_t[LCDC_NLAYERS + 1] = {
    LAYER1_FORMAT,
#ifdef CONFIG_LCDC_L2
    LAYER2_FORMAT,
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
#define pm_register(cb)           (0)
#define lcdc_pmcb_register(cb)    (0)
#else
extern int lcdc_pmcb_register(struct pm_callback_s *cb);
#endif

static void pm_panel_notify(struct pm_callback_s *cb, int domain,
                   enum pm_state_e pmstate);
static int pm_panel_prepare(struct pm_callback_s *cb, int domain,
                   enum pm_state_e pmstate);

static struct pm_callback_s panel_pmcb = {
    .notify = pm_panel_notify,
    .prepare = pm_panel_prepare,
};

__unused static void module_pm_activity(void)
{
}

static void pm_panel_notify(struct pm_callback_s *cb, int domain,
                   enum pm_state_e pmstate)
{
    FAR struct lcdcdev_s *priv = &g_vtable;

    /* save panel state */
    g_vtable.panel_state = pmstate;

    switch(pmstate) {
    case PM_NORMAL:
#ifdef CONFIG_PMU_AOD_CTRL
        pmu_dsi_aod_mode_enable(true);
#endif
        if(priv->sleep) {
            lcdc_exit_lowpower();
            priv->sleep = false;
        }
        break;
    case PM_IDLE:
#ifdef CONFIG_PMU_AOD_CTRL
        pmu_dsi_aod_mode_enable(false);
#endif
        break;
    case PM_STANDBY:
    case PM_SLEEP:
#ifdef CONFIG_PMU_AOD_CTRL
        pmu_dsi_aod_mode_enable(false);
#endif
        priv->sleep = true;
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

#endif //CONFIG_PM

/**
 * Private Lcdc Functions
 */

static bool isyuv(uint8_t fmt)
{
    if (fmt == LCDC_CFMT_YUV422_I422 || fmt == LCDC_CFMT_YUV420_I420 ||
        fmt == LCDC_CFMT_YUV420_NV12 || fmt == LCDC_CFMT_YUV420_P16) {
        return true;
    } else {
        return false;
    }
}

static uint8_t fmt_fb2lcdc(uint8_t fmt)
{
    switch (fmt) {
        case FB_FMT_RGB8:
            return LCDC_CFMT_PALETTE_8;
        case FB_FMT_RGB16_565:
            return LCDC_CFMT_RGB565;
        case FB_FMT_RGB24:
            return LCDC_CFMT_RGB888;
        case FB_FMT_RGB32:
            return LCDC_CFMT_RGBA8888;
        case FB_FMT_UYVY:
            return LCDC_CFMT_YUV422_UYVY;
        case FB_FMT_NV12:
            return LCDC_CFMT_YUV420_NV12;
        default:
            break;
    }
    LCDC_ERR("invalid layer format");
    return -1;
}

static uint8_t fmt2bpp(uint8_t fmt)
{
    switch (fmt) {
        case LCDC_CFMT_PALETTE_8:
            return 8;
        case LCDC_CFMT_RGB565:
            return 16;
        case LCDC_CFMT_RGB888:
            return 24;
        case LCDC_CFMT_RGBA8888:
            return 32;
        case LCDC_CFMT_YUV422_UYVY:
            return 16;
        case LCDC_CFMT_YUV420_NV12:
            return 8;
        default:
            break;
    }
    return -1;
}

static void reset_fmt(uint8_t overlay)
{
    uint8_t fmt = fmt_layer_t[overlay];
    uint8_t bpp = fmt2bpp(fmt);

    uint32_t stride = LCDC_WIDTH*(bpp/8);
    uint32_t fbsize = stride * LCDC_HEIGHT;
    if (fbsize > LCDC_FB_MAX_SIZE) {
        LCDC_ERR("overflow size of ioctl");
        return;
    }
    stride_layer_t[overlay] = stride;
    fblen_layer_t[overlay] = fbsize;

    if (isyuv(fmt))
        g_vtable.layer[overlay].is_yuv = true;
    else
        g_vtable.layer[overlay].is_yuv = false;

    g_vtable.layer[overlay].oinfo.stride = stride;
    g_vtable.layer[overlay].oinfo.fblen = fbsize;

    struct lcdc_layer_config_t config;
    memset(&config, 0, sizeof(config));

    config.enable = (g_vtable.layer[overlay].oinfo.blank == 0) ? true : false;
    config.format = fmt;
    config.width = LCDC_WIDTH;
    config.height = LCDC_HEIGHT;
    config.buffer = (uint8_t *)fbmem_layer_t[overlay];
    config.stride = stride;

    uint8_t lid = g_vtable.layer[overlay].lid;
    if (lcdc_layer_config(lid, &config) != 0)
        LCDC_ERR("layer config failed");
    LCDC_INFO("new layer format=%d new layer stride=%d changed overlay=%d\n", fmt, stride_layer_t[overlay], overlay);
}

static void lcdc_setup_layer_fbmem()
{
    uint32_t buffer_l1 = (uint32_t)LCDC_BUFFER_L1;
#ifdef CONFIG_LCDC_L2
    uint32_t buffer_l2 = (uint32_t)LCDC_BUFFER_L2;
#endif

    g_vtable.pinfo.fbmem = (void *)buffer_l1;
    fbmem_layer_t[0] = buffer_l1;
#ifdef CONFIG_LCDC_L2
    fbmem_layer_t[1] = buffer_l2;
#endif

#ifdef CONFIG_FB_OVERLAY
    g_vtable.layer[0].oinfo.fbmem = (uint8_t *)buffer_l1;
#endif

#ifdef CONFIG_LCDC_L2
#ifdef CONFIG_FB_OVERLAY
    g_vtable.layer[1].oinfo.fbmem = (uint8_t *)buffer_l2;
#endif
#endif
}

/**
 * lcdc_trigger_frame - trigger lcdc to generate one frame for display
 */
void lcdc_trigger_frame(void)
{
#ifndef CONFIG_DSI_VIDEO_MODE
        lcdc_xfer_frame();
#endif
}

void lcdc_enter_lowpower()
{
#ifdef CONFIG_DSI_VIDEO_MODE
    return;
#endif

    if(g_vtable.start) {
        LCDC_INFO("%s\n", __func__);
#ifdef CONFIG_LCDC_SLEEP
        lcdc_sleep();
#endif
        hal_sysfreq_req(HAL_SYSFREQ_USER_LCDC, HAL_CMU_FREQ_32K);
        g_vtable.start = false;
    }
}

void lcdc_exit_lowpower()
{
#ifdef CONFIG_DSI_VIDEO_MODE
    return;
#endif

    if(!g_vtable.start) {
        LCDC_INFO("%s\n", __func__);
#ifndef CONFIG_ARCH_CORTEXA7
#ifdef CONFIG_ARCH_CHIP_BES2003
        hal_sysfreq_req(HAL_SYSFREQ_USER_LCDC, HAL_CMU_FREQ_390M);
#else
        hal_sysfreq_req(HAL_SYSFREQ_USER_LCDC, HAL_CMU_FREQ_208M);
#endif
#endif
#ifdef CONFIG_LCDC_SLEEP
        lcdc_wakeup();
#endif
        g_vtable.start = true;
    }
}

#ifdef CONFIG_FB_PANDISPLAY

static void lcdc_pandisp_init(void)
{
    for (int i = 0; i < LCDC_NOVERLAYS; ++i) {
        g_vtable.panfo[i].cnt = 0;
        g_vtable.panfo[i].queue[0].state = BUF_NONE;
        g_vtable.panfo[i].queue[0].addr = g_vtable.layer[i].oinfo.fbmem;
    }
}

static inline __attribute__((always_inline)) void check_layer_config(struct lcdc_s *layer)
{
    uint8_t overlay = layer->oinfo.overlay;
    uint8_t lid = layer->lid;

    if (layer->blank_trigger) {
#ifdef CONFIG_ENTER_SLEEP_NO_FRAMES
        lcdc_exit_lowpower();
#endif
        if (layer->oinfo.blank)
            lcdc_layer_disable(lid);
        else
            lcdc_layer_enable(lid);
        layer->blank_trigger = false;
    }

    if (layer->fmt_trigger) {
#ifdef CONFIG_ENTER_SLEEP_NO_FRAMES
        lcdc_exit_lowpower();
#endif
        reset_fmt(overlay);
        layer->fmt_trigger = false;
    }

    if (layer->ckey_trigger) {
#ifdef CONFIG_ENTER_SLEEP_NO_FRAMES
        lcdc_exit_lowpower();
#endif
        lcdc_layer_chromakey_enable(lid, layer->oinfo.chromakey, 0, 0);
        layer->ckey_trigger = false;
    }

#ifdef CONFIG_LCDC_GAMMA_ENABLE
    struct fb_gammainfo *fb_gamma = &g_vtable.fb_gamma;
    if (fb_gamma->gamma_trigger) {
#ifdef CONFIG_ENTER_SLEEP_NO_FRAMES
        lcdc_exit_lowpower();
#endif
        if (fb_gamma->enable) {
            uint8_t *glut = fb_gamma->glut;
            uint16_t glut_cnt = fb_gamma->glut_cnt;
            lcdc_gamma_enable_by_lut(glut, glut_cnt);
        } else {
            lcdc_gamma_disable();
        }

        fb_gamma->gamma_trigger = false;
    }
#endif
}

static void lcdc_pandisp_exec(void)
{
    FAR struct lcdcdev_s* priv = &g_vtable;
    if (priv->transfering)
        return;

#ifdef CONFIG_PM
    if (g_vtable.sleep) {
        lcdc_enter_lowpower();

        int ret;
        do {
            ret = fb_remove_paninfo(&priv->vtable, FB_NO_OVERLAY);
        } while (ret == OK);

        for (int i = 0; i < LCDC_NOVERLAYS; ++i) {
            do {
                ret = fb_remove_paninfo(&priv->vtable, i);
            } while (ret == OK);
            priv->panfo[i].cnt = 0;
        }
        return;
    }
#endif

    int frame_nums = 0;
    uint32_t lk = int_lock();
    /*If there is ready data in the queue*/
    for (int i = 0; i < LCDC_NOVERLAYS; ++i) {
        LCDC_INFO("%s cnt=%d, panaddr = %08x,state = %d,frame_num = %d\n", __func__,priv->panfo[i].cnt,\
            (uint32_t)priv->panfo[i].queue[0].addr,priv->panfo[i].queue[0].state,priv->panfo[i].queue[0].current_frame);

        check_layer_config(&priv->layer[i]);

        if (priv->panfo[i].cnt > 0) {
            ++frame_nums;
            if (priv->panfo[i].queue[0].state == BUF_ROTATE)
                LCDC_ERR("frame cross vsync\n");
            if (priv->panfo[i].queue[0].state == BUF_READY) {
                if(priv->layer[i].is_yuv)
                    lcdc_layer_set_yuv_buffer(priv->layer[i].lid, (uint8_t *)priv->panfo[i].queue[0].addr, NULL, NULL, LCDC_WIDTH, LCDC_HEIGHT);
                else
                    lcdc_layer_set_buffer(priv->layer[i].lid, (uint8_t *)priv->panfo[i].queue[0].addr, stride_layer_t[i]);
                priv->panfo[i].queue[0].state = BUF_COMMIT;
            }
        }
    }
    if (frame_nums != 0) {
        priv->transfering = true;
        lcdc_trigger_frame();
    } else {
        lpcmds_flush();
#ifdef CONFIG_ENTER_SLEEP_NO_FRAMES
        lcdc_enter_lowpower();
#endif
    }
    int_unlock(lk);
}
#endif

static void lcdc_error_cb(int error)
{
    uint32_t lk = int_lock();
    LCDC_ERR("%s error %08x\n", __func__, error);
    LCDC_ERR("ERROR: configuration\n");
    int_unlock(lk);
}

static void lcdc_framedone_cb()
{
    uint32_t lk = int_lock();
    /* run commands in list */
    int frame_left = 0;
#ifdef CONFIG_FB_PANDISPLAY
#ifdef CONFIG_DSI_VIDEO_MODE
    for (int i = 0; i < LCDC_NOVERLAYS; ++i) {
        if (g_vtable.panfo[i].cnt > 0) {
            /*If there are two data in the queue, move the tail node to the head*/
            fb_remove_paninfo(&g_vtable.vtable, g_vtable.panfo[i].queue[0].pan_id);
            g_vtable.panfo[i].cnt--;
            uint8_t lid = g_vtable.layer[i].lid;
            uint8_t *dispbuff = g_vtable.panfo[i].queue[0].addr;
            uint32_t stride = stride_layer_t[i];
            if (g_vtable.layer[i].is_yuv)
                lcdc_layer_set_yuv_buffer(lid, (uint8_t *)dispbuff, NULL, NULL, LCDC_WIDTH, LCDC_HEIGHT);
            else
                lcdc_layer_set_buffer(lid, dispbuff, stride);
        }
    }
#else
    for (int i = 0; i < LCDC_NOVERLAYS; ++i) {
        if (g_vtable.panfo[i].cnt > 0 && g_vtable.panfo[i].queue[0].state == BUF_COMMIT) {
            /*If there are two data in the queue, move the tail node to the head*/
            fb_remove_paninfo(&g_vtable.vtable, g_vtable.panfo[i].queue[0].pan_id);
            if (g_vtable.panfo[i].cnt == 2) {
                ++frame_left;
                g_vtable.panfo[i].queue[0] = g_vtable.panfo[i].queue[1];
            }

            g_vtable.panfo[i].cnt--;
        }
    }
#endif /* CONFIG_DSI_VIDEO_MODE */
#endif /* CONFIG_FB_PANDISPLAY */

    /* Unlock the semaphore if locked */
    int ret = 0;
    gra_sem_get_value(&g_semirq, &ret);
    if (ret < 0)
        gra_sem_post(&g_semirq);

    g_vtable.transfering = false;

    int_unlock(lk);
}

/**
 * lcdc_irqconfig - Configure interrupts
 */

static void lcdc_irqconfig(void)
{
    gra_sem_init(&g_semirq, 0, 0);
    /* Initialize the semaphore for interrupt handling.  This waitsem
     * semaphore is used for signaling and, hence, should not have priority
     * inheritance enabled.
     */

    lcdc_register_framedone_cb(lcdc_framedone_cb);
    lcdc_register_error_cb(lcdc_error_cb);
}

#ifdef CONFIG_LCDC_TE_IRQ_ENABLE
static void lcdc_vsync_cb(void)
{
    LCDC_INFO("%s\n", __func__);
#ifdef CONFIG_FB_PANDISPLAY
    lcdc_pandisp_exec();
#endif
}

/**
 * lcdc_teirqconfig - config te interrupt
 */
static void lcdc_teirqconfig(void)
{
    lcdc_register_vsync_cb(lcdc_vsync_cb);
}
#endif

#ifdef CONFIG_FB_PANDISPLAY

/**
 * pan_display - lcdc switch display layer
 * @vtable : Reference to lcdc driver interface
 * @var : Reference to dispaly control structure
 *
 * Used for fb double buffer application,change display layer by xoffset and yoffset
 */
static int pan_display(FAR struct fb_vtable_s *vtable,
                       FAR struct fb_planeinfo_s *pinfo)
{
    FAR struct lcdcdev_s *priv = &g_vtable;
    FAR struct fb_planeinfo_s *pri_pinfo = &priv->pinfo;

    uint32_t dispbuff = (uint32_t)priv->pinfo.fbmem;

    if (vtable == NULL && pinfo->yoffset != 0) {
        dispbuff = pinfo->yoffset;
    } else {
#ifdef CONFIG_FB_DOUBLE_BUFFER
        if (pinfo->yoffset * pri_pinfo->stride > LCDC_FB_MAX_SIZE / 2 ||
            pinfo->xoffset > pri_pinfo->xres_virtual - priv->vinfo.xres) {
            LCDC_ERR("too much yoffset!!!");
            return -EINVAL;
        }
        dispbuff += priv->pinfo.stride * pinfo->yoffset;
#endif
    }

    uint32_t flags = int_lock();
    priv->panfo[0].frame_num++;
    LCDC_INFO("%s cnt=%d,frame_num:%d, pre panaddr = %08x,yoffset = %08x,fbmem = %08x,\n", __func__,priv->panfo[0].cnt, priv->panfo[0].frame_num, (uint32_t)priv->panfo[0].queue[0].addr,(uint32_t)pinfo->yoffset,(uint32_t)priv->pinfo.fbmem);
    /*If there is space in the queue, fill it with a new buffer*/
#ifndef CONFIG_FB_DOUBLE_BUFFER
    if (priv->panfo[0].cnt < 1
#else
    if (priv->panfo[0].cnt < 2
#endif
#ifdef CONFIG_PM
      && (priv->panel_state != PM_STANDBY && priv->panel_state != PM_SLEEP)
#endif
      ) {
        uint8_t next_buf = priv->panfo[0].cnt;
        priv->panfo[0].queue[next_buf].addr = (void *)dispbuff;
        priv->panfo[0].queue[next_buf].state = BUF_READY;
        priv->panfo[0].queue[next_buf].current_frame = priv->panfo[0].frame_num;
        priv->panfo[0].queue[next_buf].pan_id = FB_NO_OVERLAY;
        priv->panfo[0].cnt++;
#ifdef CONFIG_ENTER_SLEEP_NO_FRAMES
        lcdc_exit_lowpower();
#endif
        int_unlock(flags);
        return OK;
    } else {
        LCDC_ERR("%s Lossing frame_num:%d\n", __func__,priv->panfo[0].frame_num);
    }
    int_unlock(flags);
    return ERROR;
}
#endif

#ifdef CONFIG_FB_CMAP
static int getcmap(FAR struct fb_vtable_s *vtable,
                   FAR struct fb_cmap_s *cmap)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;
    ASSERT(vtable != NULL && priv == &g_vtable, "%s", __func__);

    if(cmap == NULL) {
        LCDC_ERR("%s error parameter\n", __func__);
        return ERROR;
    }

    memcpy(cmap, &priv->fb_cmap, sizeof(struct fb_cmap_s));
    return OK;
}

static int putcmap(FAR struct fb_vtable_s *vtable,
                   FAR const struct fb_cmap_s *cmap)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;
    ASSERT(vtable != NULL && priv == &g_vtable, "%s", __func__);

    if(cmap == NULL || cmap->red == NULL || cmap->green == NULL || cmap->blue == NULL) {
        LCDC_ERR("%s error parameter\n", __func__);
        return ERROR;
    }

    memcpy(priv->fb_cmap.red, cmap->red + cmap->first, cmap->len);
    memcpy(priv->fb_cmap.green, cmap->green + cmap->first, cmap->len);
    memcpy(priv->fb_cmap.blue, cmap->blue + cmap->first, cmap->len);
#ifdef CONFIG_FB_TRANSPARENCY
    memcpy(priv->fb_cmap.transp, cmap->transp + cmap->first, cmap->len);
#endif
    priv->fb_cmap.len = cmap->len;

    uint32_t color;
    uint8_t *rp = priv->fb_cmap.red;
    uint8_t *gp = priv->fb_cmap.green;
    uint8_t *bp = priv->fb_cmap.blue;
#ifdef CONFIG_FB_TRANSPARENCY
    uint8_t *tp = priv->fb_cmap.transp;
#endif
    for(int i = 0; i < cmap->len; ++i) {
        color = bp[i] << 16 | gp[i] << 8 | rp[i];
#ifdef CONFIG_FB_TRANSPARENCY
        color &= ~(0xff << 24);
        color |= tp[i] << 24;
#endif
        rgb_clut[i] = color;
    }

#ifdef CONFIG_ENTER_SLEEP_NO_FRAMES
    lcdc_exit_lowpower();
#endif
    lcdc_palette_enable(LCDC_CFMT_PALETTE_8, rgb_clut, priv->fb_cmap.len);
    return OK;
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

    return ((uint32_t) poverlay->fbmem) + offset;
}

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
    uint32_t x_offset = 0;
    uint32_t y_offset = 0;

    ASSERT(overlay < LCDC_NLAYERS, "%s", __func__);

    FAR struct lcdcdev_s *dev = &g_vtable;
    FAR struct lcdc_s *layer = &dev->layer[overlay];

    struct lcdc_layer_config_t config;
    uint8_t layerno = layer->lid;

    if (isyuv(fmt_layer_t[overlay]))
        layer->is_yuv = true;
    else
        layer->is_yuv = false;

    LCDC_ERR("buffer size %dx%d, offset [%d, %d]\n",
            LCDC_WIDTH, LCDC_HEIGHT, x_offset, y_offset);

    memset(&config, 0, sizeof(config));
    config.enable = layer->oinfo.blank == 1 ? false : true;
    config.format = fmt_layer_t[overlay];
    config.width = LCDC_WIDTH;
    config.height = LCDC_HEIGHT;
    config.offset_x = x_offset;
    config.offset_y = y_offset;
#if defined(CONFIG_DSI_VIDEO_MODE)
    config.buffer = (uint8_t *)(fbmem_layer_t[overlay] + stride_layer_t[overlay] * LCDC_HEIGHT);
#else
    config.buffer = (uint8_t *)fbmem_layer_t[overlay];
#endif
    config.stride = stride_layer_t[overlay];

    lcdc_layer_config(layerno, &config);
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

    memcpy(pinfo, &priv->pinfo, sizeof(struct fb_planeinfo_s));
    return OK;
}

static int setchromakey(FAR struct fb_vtable_s *vtable,
                        FAR const struct fb_overlayinfo_s *oinfo)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;
    ASSERT(vtable != NULL && priv == &g_vtable && oinfo != NULL, "%s", __func__);

    uint8_t overlay = oinfo->overlay;

    if (overlay < LCDC_NOVERLAYS) {

        if (overlay == FB_NO_OVERLAY)
            overlay = 0;

        struct lcdc_s *layer = &priv->layer[overlay];
        layer->oinfo.chromakey = oinfo->chromakey;
        layer->ckey_trigger = true;
        return OK;
    } else {
        LCDC_ERR("%s: error overlay", __func__);
        return ERROR;
    }
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
 * waitforvsync - wait for vsync signal
 * @vtable : Reference to lcdc driver interface
 *
 * Entrypoint ioctl FBIO_WAITFORSYNC
 */
#ifdef CONFIG_FB_SYNC
static int waitforvsync(FAR struct fb_vtable_s *vtable)
{
    ASSERT(vtable != NULL && vtable == &g_vtable.vtable, "%s", __func__);
    FAR struct lcdcdev_s* priv = &g_vtable;
#ifndef CONFIG_FB_DOUBLE_BUFFER
#ifdef CONFIG_LCDC_L2
    while (!(priv->panfo[0].cnt < 1) || !(priv->panfo[1].cnt < 1) || !lcdc_is_started())
#else
    while (!(priv->panfo[0].cnt < 1) || !lcdc_is_started())
#endif
#else
#ifdef CONFIG_LCDC_L2
    while (!(priv->panfo[0].cnt < 2) || !(priv->panfo[1].cnt < 2) || !lcdc_is_started())
#else
    while (!(priv->panfo[0].cnt < 2) || !lcdc_is_started())
#endif
#endif
    {
        gra_sem_wait(&g_semirq);
    }
    return 0;
}
#endif /* CONFIG_FB_SYNC */

static int checkoverlay(uint8_t *op)
{
    if (*op == FB_NO_OVERLAY) {
        *op = 0;
    } else if (*op < 0 || *op >= LCDC_NLAYERS) {
        LCDC_ERR("invalid overlays");
        return -EINVAL;
    }
    return 0;
}

/**
 * ioctl - self supported operation
 * @vtable : Reference to lcdc driver interface
 * @cmd : commands for ioctl
 * @arg : detail argument
 *
 * Entrypoint ioctl default
 */

static int ioctl(FAR struct fb_vtable_s *vtable, int cmd, unsigned long arg)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    if (!vtable || priv != &g_vtable)
        return -1;

    if (cmd == FBIOSET_FMT) {
        uint8_t overlay = *((uint8_t *)arg);
        if (checkoverlay(&overlay) < 0)
            return -EINVAL;

        uint8_t fmt = fmt_fb2lcdc(*((uint8_t *)(arg + 1)));
        if (fmt < 0)
            return -EINVAL;

        fmt_layer_t[overlay] = fmt;
        priv->layer[overlay].fmt_trigger = true;
        return OK;
    } else if(cmd == FBIOSET_GAMMA) {
#ifdef CONFIG_LCDC_GAMMA_ENABLE
        struct fb_gammainfo *fb_gamma = (struct fb_gammainfo *)arg;

        if (fb_gamma == NULL) {
            LCDC_ERR("%s: invalid gamma info pointer", __func__);
            return -EINVAL;
        }
        if (fb_gamma->glut == NULL) {
            LCDC_ERR("%s: invalid lut table pointer", __func__);
            return -EINVAL;
        }

        memcpy(priv->fb_gamma.glut, fb_gamma->glut, fb_gamma->glut_cnt);
        priv->fb_gamma.glut_cnt = fb_gamma->glut_cnt;
        priv->fb_gamma.gamma_trigger = true;
#else
        LCDC_ERR("config not support gamma");
        return -EINVAL;
#endif
    }

    LCDC_ERR("not supported cmd: 0x%x", cmd);
    return -EINVAL;
}

/**
 * getoverlayinfo - get lcdc overlay info
 * @vtable : Reference to lcdc driver interface
 * @overlayno : fb overlay index
 * @oinfo : Reference to dest fb overlay info structure
 *
 * Entrypoint ioctl FBIOGET_OVERLAYINFO */
#ifdef CONFIG_FB_OVERLAY
static int getoverlayinfo(FAR struct fb_vtable_s *vtable,
              int overlayno, FAR struct fb_overlayinfo_s *oinfo)
{
    FAR struct lcdcdev_s *priv = (FAR struct lcdcdev_s *)vtable;

    LCDC_INFO("vtable=%p overlay=%d oinfo=%p\n", vtable, overlayno, oinfo);
    ASSERT(vtable != NULL && priv == &g_vtable, "%s", __func__);

    if (overlayno < LCDC_NOVERLAYS) {

        if (overlayno == FB_NO_OVERLAY)
            overlayno = 0;

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
    uint8_t transp = oinfo->transp.transp;
    uint8_t transp_mode = oinfo->transp.transp_mode;
    LCDC_INFO("vtable=%p, overlay=%d, transp=%02x, transp_mode=%02x\n",
          vtable, oinfo->overlay, transp, transp_mode);

    ASSERT(vtable != NULL && priv == &g_vtable, "%s", __func__);

    if (transp_mode > 1) {
        LCDC_ERR("ERROR: Returning ENOSYS, transparency mode not supported\n");
        return -ENOSYS;
    }

    if (oinfo->overlay < LCDC_NOVERLAYS) {
        FAR struct lcdc_s *layer = &priv->layer[oinfo->overlay];

        layer->oinfo.transp.transp = transp;
        layer->oinfo.transp.transp_mode = transp_mode;

#ifdef CONFIG_ENTER_SLEEP_NO_FRAMES
        lcdc_exit_lowpower();
#endif
        /* Set LCDC blendmode and alpha value */
        lcdc_layer_blend(0, LCDC_BLEND_MODE0, transp_mode, transp);

#ifdef CONFIG_ENTER_SLEEP_NO_FRAMES
        lcdc_enter_lowpower();
#endif
        return OK;
    }

    LCDC_ERR("ERROR: Returning EINVAL\n");
    return -EINVAL;
}

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

    int overlay = oinfo->overlay;
    int blank = oinfo->blank;
    if (overlay < LCDC_NLAYERS) {
        priv->layer[overlay].oinfo.blank = blank;
        priv->layer[overlay].blank_trigger = true;
        return OK;
    }

    LCDC_ERR("ERROR: Returning EINVAL\n");
    return -EINVAL;
}

/**
 * pan_overlay - lcdc switch frame for specific layer
 * @vtable : Reference to lcdc driver interface
 * @oinfo : Reference to fb overlay info structure
 *
 * Entrypoint ioctl FBIOPAN_OVERLAY
 * Returns:   OK on success, -errno on failure.
 */
static int pan_overlay(FAR struct fb_vtable_s *vtable,
                       FAR const struct fb_overlayinfo_s *oinfo)
{
    ASSERT(vtable != NULL && vtable == (struct fb_vtable_s *)&g_vtable
           && oinfo != NULL, "%s", __func__);
    LCDC_INFO("vtable=%p, overlay=%d\n", vtable ,oinfo->overlay);

    int overlay = oinfo->overlay;

    if (overlay >= LCDC_NLAYERS) {
        LCDC_ERR("invalid overlay\n");
        return -EINVAL;
    }

    FAR struct lcdcdev_s *priv = &g_vtable;
    FAR struct fb_overlayinfo_s *priv_oinfo = &priv->layer[overlay].oinfo;

    uint32_t dispbuff = (uint32_t)priv_oinfo->fbmem;

    if (oinfo->fbmem != NULL) {
        dispbuff = (uint32_t)oinfo->fbmem;
    } else {
#ifdef CONFIG_FB_DOUBLE_BUFFER
        if (oinfo->yoffset * priv_oinfo->stride > LCDC_FB_MAX_SIZE / 2  ||
            oinfo->xoffset > priv_oinfo->xres_virtual - priv_oinfo->xres) {
            LCDC_ERR("too much yoffset!!!");
            return -EINVAL;
        }
        dispbuff += priv_oinfo->stride * oinfo->yoffset;
#endif
    }

    hal_cache_sync_all(HAL_CACHE_ID_D_CACHE);

    uint32_t flags = int_lock();
    priv->panfo[overlay].frame_num++;
    LCDC_INFO("%s cnt=%d,frame_num:%d, pre panaddr = %08x,yoffset = %08x,oinfo = %p,\n",
        __func__, priv->panfo[overlay].cnt, priv->panfo[overlay].frame_num, dispbuff, oinfo->yoffset, oinfo);
    /*If there is space in the queue, fill it with a new buffer*/
#ifndef CONFIG_FB_DOUBLE_BUFFER
    if (priv->panfo[overlay].cnt < 1
#else
    if (priv->panfo[overlay].cnt < 2
#endif
#ifdef CONFIG_PM
      && (priv->panel_state != PM_STANDBY && priv->panel_state != PM_SLEEP)
#endif
      ) {
        uint8_t next_buf = priv->panfo[overlay].cnt;
        priv->panfo[overlay].queue[next_buf].addr = (void *)dispbuff;
        priv->panfo[overlay].queue[next_buf].state = BUF_READY;
        priv->panfo[overlay].queue[next_buf].current_frame = priv->panfo[overlay].frame_num;
        priv->panfo[overlay].queue[next_buf].pan_id = overlay;
        priv->panfo[overlay].cnt++;
#ifdef CONFIG_ENTER_SLEEP_NO_FRAMES
        lcdc_exit_lowpower();
#endif
        int_unlock(flags);
        return OK;
    } else {
        LCDC_ERR("%s Lossing frame_num:%d\n", __func__,priv->panfo[overlay].frame_num);
    }
    int_unlock(flags);

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

/**
 * lcdc_initialize - Initialize the lcdc controller
 *
 * Returns:   OK on success, -errno on failure.
 */

FAR struct lcd_dev_s *board_lcd_getdev(int lcddev);
int lcdc_initialize()
{
    int ret = OK;
    if (g_initialized == true) {
        return ret;
    }
    g_vtable.lcddev = (struct lcd_dev_s *)board_lcd_getdev(0);

    lcdc_setup_layer_fbmem();

    /* Configure interrupts */
    LCDC_INFO("Configure interrupts\n");

    lcdc_irqconfig();

#ifdef CONFIG_LCDC_TE_IRQ_ENABLE
    LCDC_INFO("Configure te interrupts\n");
    lcdc_teirqconfig();
#endif

    /* Configure global lcdc register */
    LCDC_INFO("Configure global register\n");

    /* Initialize lcdc layer */

    LCDC_INFO("Initialize lcdc layer\n");
    lcdc_linit(0);
#ifdef CONFIG_LCDC_L2
    lcdc_linit(1);
#endif

    LCDC_INFO("Enabling the display\n");

    /* Set initialized state */
#ifdef CONFIG_PM
    g_vtable.sleep = false;
    lcdc_pmcb_register(&panel_pmcb);
#endif

#ifdef CONFIG_FB_PANDISPLAY
    LCDC_INFO("Init mutible buffer support.\n");
    lcdc_pandisp_init();
#endif

#ifndef CONFIG_DSI_VIDEO_MODE
#ifdef SIMU
    hw_timer = hwtimer_alloc(timer_handler, 0);
    hwtimer_start(hw_timer, MS_TO_TICKS(17));
#endif
#endif

    g_vtable.transfering = false;

#if defined(CONFIG_LCDC_DUAL_DSI) && defined(CONFIG_LCDC_L2)
    lcdc_layer_select(HAL_DSI_ID_0, (1 << LAYER1_ID));
    lcdc_layer_select(HAL_DSI_ID_1, (1 << LAYER2_ID));
#endif

    lcdc_start();
    g_vtable.start = true;

#ifdef CONFIG_ENTER_SLEEP_NO_FRAMES
    lcdc_enter_lowpower();
#endif

    LCDC_INFO("lcdc_initialize done\n");
    g_initialized = true;
    return ret;
}

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

    gra_sem_destroy(&g_semirq);

    lcdc_exit();

    /* Set initialized state */
    g_initialized = false;
}

static bool fb_initialized = false;
//#define CONFIG_LCD_SWAP_RB
static struct lcdc_config_t lcdc_config = {
#ifdef CONFIG_DSI_VIDEO_MODE
    .mode = LCDC_VIDEO_MODE,
#else
    .mode = LCDC_COMMAND_MODE,
#endif
#ifdef CONFIG_LCDC_USE_QSPI
    .bus = LCDC_BUS_QSPI,
    .bus_mhz = 96,
    .bus_lane = 4,
    .bus_port0 = 0,
    .bus_port1 = 0,
#else /* CONFIG_LCDC_USE_QSPI */
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
#endif /* CONFIG_LCDC_USE_DSI1 */
#endif /* CONFIG_LCDC_DUAL_DSI */
#ifdef DSIPLL_MHZ
    .bus_mhz = DSIPLL_MHZ,
#else
    .bus_mhz = 400,
#endif
#if defined(CONFIG_DSI_4LANE_MODE)
    .bus_lane = 4,
#elif defined(CONFIG_DSI_2LANE_MODE)
    .bus_lane = 2,
#else
    .bus_lane = 1,
#endif
#endif /* CONFIG_LCDC_USE_QSPI */
    .pix_mhz = 24,

    .lcd = {
        .width  = BOARD_LCDC_WIDTH,
        .height = BOARD_LCDC_HEIGHT,
        .hfp    = BOARD_LCDC_HFP,
        .hbp    = BOARD_LCDC_HBP,
        .hsync  = BOARD_LCDC_HSYNC,
        .vfp    = BOARD_LCDC_VFP,
        .vbp    = BOARD_LCDC_VBP,
        .vsync  = BOARD_LCDC_VSYNC,
#if defined(CONFIG_DSIPHY_OUTFMT_RGB888)
        .color_bits = 24,
#elif defined(CONFIG_DSIPHY_OUTFMT_RGB565)
        .color_bits = 16,
#elif defined(CONFIG_DSIPHY_OUTFMT_RGB666)
        .color_bits = 18,
#else
        #error "Wrong config";
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

int bes_lcdc_get_te_pin(void);
static int bes_lcdc_init(void)
{
#ifdef CONFIG_LCDC_TE_IRQ_ENABLE
    if (lcdc_config.mode == LCDC_COMMAND_MODE && lcdc_config.te_pin < 0) {
#ifdef LCD_TE_PIN
        lcdc_config.te_pin = LCD_TE_PIN;
#else
        lcdc_config.te_pin = bes_lcdc_get_te_pin();
#endif

    }
#endif

    return lcdc_init(&lcdc_config);
}

/****************************************************************************
 * Name: up_fbinitialize
 *
 * Description:
 *   Initialize the framebuffer video hardware associated with the display.
 *
 * Input Parameters:
 *   display - In the case of hardware with multiple displays, this
 *     specifies the display.  Normally this is zero.
 *
 * Returned Value:
 *   Zero is returned on success; a negated errno value is returned on any
 *   failure.
 *
 ****************************************************************************/
int board_lcd_initialize(void);

int up_fbinitialize(int display)
{
    int ret = 0;

    /* Custom LCD display with RGB interface */
    if (!fb_initialized) {
      ret = bes_lcdc_init();
      if (ret != LCDC_OK) {
          LCDC_ERR("Failed to init LCDC/DSI/QSPI.\n");
          return -1;
      }

      board_lcd_initialize();

      ret = lcdc_initialize();
      if (ret) {
          LCDC_ERR("Failed to enable LCDC\n");
      }
#ifdef CONFIG_LV_USE_DRAW_BES_VGLITE
      extern int vg_lite_init(int, int);
      vg_lite_init(LCDC_WIDTH, LCDC_HEIGHT);
#endif
    }
    fb_initialized = true;
    return ret;
}

/****************************************************************************
 * Name: up_fbgetvplane
 *
 * Description:
 *   Return a a reference to the framebuffer object for the specified video
 *   plane of the specified plane.  Many OSDs support multiple planes of video.
 *
 * Input Parameters:
 *   display - In the case of hardware with multiple displays, this
 *     specifies the display.  Normally this is zero.
 *   vplane - Identifies the plane being queried.
 *
 * Returned Value:
 *   A non-NULL pointer to the frame buffer access structure is returned on
 *   success; NULL is returned on any failure.
 *
 ****************************************************************************/

FAR struct fb_vtable_s *up_fbgetvplane(int display, int vplane)
{
    if (vplane == 0) {
        return &g_vtable.vtable;
    }
    return NULL;
}

/****************************************************************************
 * Name: up_fbuninitialize
 *
 * Description:
 *   Uninitialize the framebuffer support for the specified display.
 *
 * Input Parameters:
 *   display - In the case of hardware with multiple displays, this
 *     specifies the display.  Normally this is zero.
 *
 * Returned Value:
 *   None
 *
 ****************************************************************************/

void up_fbuninitialize(int display)
{
    if (fb_initialized) {
        lcdc_exit();
#ifdef CONFIG_LV_USE_DRAW_BES_VGLITE
        extern void vg_lite_close(void);
        vg_lite_close();
#endif
    }
    fb_initialized = false;
}
