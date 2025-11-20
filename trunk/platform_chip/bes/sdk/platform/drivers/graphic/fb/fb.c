
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
 * DOC: lcdc and dma2d driver test program with fb interface
 *
 * "/dev/fb0" is the device node path for lcdc framebuffer driver
 * "/dev/fb_dma2d" is the device node path for dma2d framebuffer driver
 *
 *  fb_graphic_test is the program entry of lcdc and dma2d driver test
 *
 *  features of lcdc and dma2d configured with target.mk
 *  lcdc_conf.h includes lcd panel related Configuration
 *
 * lcdc testcase: blit, blend, blend_scale, fill color, pan display,
 * rect update, video mirror, color key, color dither, color adjust
 * NOTE:lcdc testcases show the usage of lcdc driver interface on lcdc layers
 *
 * dma2d testcase: fill color, blend, blit , clut,  color fmt convertion
 * NOTE: dma2d testcase shows how to use dma2d functions on dynamicly allocated
 * user overlayers
 */

#include "lcdc_fb.h"

#include "cmsis_nvic.h"
#include "hal_cache.h"
#include "hal_psram.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_lcdc.h"
#include "heap_api.h"
#include "lcdc.h"
#include "string.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_location.h"
#include "hal_dsi.h"
#include "lcd_panel.h"
#include "lpcmd.h"

/**
 *      DEFINES
 */
#ifndef FBDEV_PATH

#define FBDEV_FB0_PATH "/dev/fb0"

#endif

#define FB_BUF_SIZE  (CONFIG_LCDC_FB_SIZE)

#if defined(CHIP_BEST2003)
#define fb_buf 0x3c000000
#elif defined(CHIP_BEST2009)
#define fb_buf  0x80000000
#else
#ifdef PSRAM_ENABLE
PSRAM_NC_BSS_LOC
#endif
static char fb_buf[FB_BUF_SIZE] ALIGNED(64) ;
#endif

//#define GPIO_TRIGGER_FOR_TEST

#ifdef GPIO_TRIGGER_FOR_TEST
static const struct HAL_IOMUX_PIN_FUNCTION_MAP pinmux_gpio[] = {
    {HAL_IOMUX_PIN_P8_7, HAL_IOMUX_FUNC_AS_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_NOPULL},
};
#endif

static void lcd_gpio_setup(void)
{
#ifdef GPIO_TRIGGER_FOR_TEST
    hal_iomux_init(pinmux_gpio, ARRAY_SIZE(pinmux_gpio));
    hal_gpio_pin_set_dir(HAL_IOMUX_PIN_P8_7, HAL_GPIO_DIR_OUT, 1);
    hal_gpio_pin_clr(HAL_IOMUX_PIN_P8_7);
#endif
}

void lcd_set_start_flag(void)
{
#ifdef GPIO_TRIGGER_FOR_TEST
    hal_gpio_pin_set(HAL_IOMUX_PIN_P8_7);
#endif
}

void lcd_set_end_flag(void)
{
#ifdef GPIO_TRIGGER_FOR_TEST
    hal_gpio_pin_clr(HAL_IOMUX_PIN_P8_7);
#endif
}

#define FB_MEM_RESULT_CHECK

//#define FB_SIM
#define WB_TEST

#ifdef  WITH_OS
#undef  WITH_OS
#endif

#ifdef FB_SIM
#define FB_INFO(...)
#define FB_REP(...)
#define FB_ERR(...)
#else
//#define FB_INFO(...)
#define FB_INFO(...) TR_INFO(TR_MOD(TEST), "//" __VA_ARGS__)
#define FB_REP(...)  TR_INFO(TR_MOD(TEST), "//" __VA_ARGS__)
#define FB_ERR(...)  TR_ERROR(TR_MOD(TEST), "//" __VA_ARGS__)
#endif

#define FAR
#define DEBUGASSERT(x) ASSERT((x), "%s %d", __func__, __LINE__)

#define fb_overlay_alloc(olptr, saddr, llen, lstride, lidx, pixbpp, lcolor, ltransp, ltransp_mode, laccl) \
    do {                                      \
        olptr->fbmem = (void *)saddr;           \
        olptr->fblen = llen;                     \
        olptr->stride = lstride;                   \
        olptr->overlay = lidx;                     \
        olptr->bpp = pixbpp;                       \
        olptr->color = lcolor;                     \
        olptr->transp.transp = ltransp;            \
        olptr->transp.transp_mode = ltransp_mode;  \
        olptr->accl = laccl;                       \
    } while (0)

typedef int16_t lv_coord_t;
typedef uint8_t lv_opa_t;

/** Represents an area of the screen. */
typedef struct {
    lv_coord_t x1;
    lv_coord_t y1;
    lv_coord_t x2;
    lv_coord_t y2;
} lv_area_t;

/**
 *  Private variables
 */

POSSIBLY_UNUSED
static heap_handle_t fb_mem_setup(uint32_t * base_addr, uint32_t size)
{
#ifndef FB_SIM
    memset(base_addr, 0, size);
#endif
    return heap_register(base_addr, size);
}

#if defined(WITH_OS)
static long int screensize = 0;
#endif

static uint32_t fb0fd = 0;    /*handle for fb0 */

static struct fb_overlayinfo_s g_oinfo[2];

/**
 * fb_ioctl  - lcdc framebuffer driver ioctrl interface
 *
 */

int fb_ioctl(uint32_t fd, int cmd, unsigned long arg)
{
    FAR struct fb_chardev_s *fb;
    int ret;

    //FB_INFO("%s fd:%x cmd: %d arg: %lx\n", __func__, fd, cmd, arg);

    /* Get the framebuffer instance */

    fb = (FAR struct fb_chardev_s *)fd;

    /* Process the IOCTL command */

    switch (cmd) {
    case FBIOGET_VIDEOINFO:    /* Get color plane info */
        {
            FAR struct fb_videoinfo_s *vinfo = (FAR struct fb_videoinfo_s *)((uintptr_t) arg);

            DEBUGASSERT(vinfo != 0 && fb->vtable != NULL && fb->vtable->getvideoinfo != NULL);
            ret = fb->vtable->getvideoinfo(fb->vtable, vinfo);
        } break;

    case FBIOGET_PLANEINFO:    /* Get video plane info */
        {
            FAR struct fb_planeinfo_s *pinfo = (FAR struct fb_planeinfo_s *)((uintptr_t) arg);

            DEBUGASSERT(pinfo != 0 && fb->vtable != NULL && fb->vtable->getplaneinfo != NULL);
            ret = fb->vtable->getplaneinfo(fb->vtable, fb->plane, pinfo);
        } break;

#ifdef CONFIG_FB_CMAP
    case FBIOGET_CMAP:    /* Get RGB color mapping */
        {
            FAR struct fb_cmap_s *cmap = (FAR struct fb_cmap_s *)((uintptr_t) arg);

            DEBUGASSERT(cmap != 0 && fb->vtable != NULL && fb->vtable->getcmap != NULL);
            ret = fb->vtable->getcmap(fb->vtable, cmap);
        } break;

    case FBIOPUT_CMAP:    /* Put RGB color mapping */
        {
            FAR const struct fb_cmap_s *cmap = (FAR struct fb_cmap_s *)((uintptr_t) arg);

            DEBUGASSERT(cmap != 0 && fb->vtable != NULL && fb->vtable->putcmap != NULL);
            ret = fb->vtable->putcmap(fb->vtable, cmap);
        } break;
#endif

#ifdef CONFIG_FB_COLORADJCURVE
    case FBIOGET_CADJMAP:    /* Get RGB color mapping */
        {
            FAR struct fb_cadjmap_s *cmap = (FAR struct fb_cadjmap_s *)((uintptr_t) arg);

            DEBUGASSERT(cmap != 0 && fb->vtable != NULL && fb->vtable->getcadjmap != NULL);
            ret = fb->vtable->getcadjmap(fb->vtable, cmap);
        } break;

    case FBIOPUT_CADJMAP:    /* Put RGB color mapping */
        {
            FAR const struct fb_cadjmap_s *cmap = (FAR struct fb_cadjmap_s *)((uintptr_t) arg);

            DEBUGASSERT(cmap != 0 && fb->vtable != NULL && fb->vtable->putcadjmap != NULL);
            ret = fb->vtable->putcadjmap(fb->vtable, cmap);
        } break;
#endif

#ifdef CONFIG_FB_COLORDITHER

    case FBIOPUT_CDITHER:    /* Put RGB color dither table */
        {
            FAR const struct fb_cdithermap_s *cdithermap = (FAR struct fb_cdithermap_s *)((uintptr_t) arg);

            DEBUGASSERT(cdithermap != 0 && fb->vtable != NULL && fb->vtable->putcdithermap != NULL);
            ret = fb->vtable->putcdithermap(fb->vtable, cdithermap);
        } break;
#endif

#ifdef CONFIG_FB_HWCURSOR
    case FBIOGET_CURSOR:    /* Get cursor attributes */
        {
            FAR struct fb_cursorattrib_s *attrib = (FAR struct fb_cursorattrib_s *)((uintptr_t) arg);

            DEBUGASSERT(attrib != 0 && fb->vtable != NULL && fb->vtable->getcursor != NULL);
            ret = fb->vtable->getcursor(fb->vtable, attrib);
        } break;

    case FBIOPUT_CURSOR:    /* Set cursor attibutes */
        {
            FAR struct fb_setcursor_s *cursor = (FAR struct fb_setcursor_s *)((uintptr_t) arg);

            DEBUGASSERT(cursor != 0 && fb->vtable != NULL && fb->vtable->setcursor != NULL);
            ret = fb->vtable->setcursor(fb->vtable, cursor);
        } break;
#endif

#ifdef CONFIG_FB_UPDATE
    case FBIO_UPDATE:    /* Update the LCD with the modified framebuffer data  */
        {
            struct fb_area_s *area = (FAR struct fb_area_s *)((uintptr_t) arg);

            DEBUGASSERT(fb->vtable != NULL && fb->vtable->updatearea != NULL);
            ret = fb->vtable->updatearea(fb->vtable, area);
        } break;
#endif

#ifdef CONFIG_FB_SYNC
    case FBIO_WAITFORVSYNC:    /* Wait upon vertical sync */
        {
            ret = fb->vtable->waitforvsync(fb->vtable);
        }
        break;
#endif

#ifdef CONFIG_FB_OVERLAY
    case FBIO_SELECT_OVERLAY:    /* Select video overlay */
        {
            struct fb_overlayinfo_s oinfo;

            DEBUGASSERT(fb->vtable != NULL && fb->vtable->getoverlayinfo != NULL);
            ret = fb->vtable->getoverlayinfo(fb->vtable, arg, &oinfo);
            if (ret == OK) {
                fb->fbmem = oinfo.fbmem;
                fb->fblen = oinfo.fblen;
                fb->bpp = oinfo.bpp;
            }
        }
        break;

    case FBIOGET_OVERLAYINFO:    /* Get video overlay info */
        {
            FAR struct fb_overlayinfo_s *oinfo = (FAR struct fb_overlayinfo_s *)((uintptr_t) arg);
            //FB_INFO("oinfo:%p fb:%p fb->vtable:%p fb->vtable->getoverlayinfo:%p ", oinfo, fb, fb->vtable, fb->vtable->getoverlayinfo);
            ASSERT(oinfo != 0 && fb->vtable != NULL && fb->vtable->getoverlayinfo != NULL, "%s", __func__);

            ret = fb->vtable->getoverlayinfo(fb->vtable, oinfo->overlay, oinfo);
        }
        break;

    case FBIOSET_TRANSP:    /* Set video overlay transparency */
        {
            FAR struct fb_overlayinfo_s *oinfo = (FAR struct fb_overlayinfo_s *)((uintptr_t) arg);

            DEBUGASSERT(oinfo != 0 && fb->vtable != NULL && fb->vtable->settransp != NULL);
            ret = fb->vtable->settransp(fb->vtable, oinfo);
        } break;

#ifdef CONFIG_LCDC_CHROMAKEY

    case FBIOSET_CHROMAKEY:    /* Set video overlay chroma key */
        {
            FAR struct fb_overlayinfo_s *oinfo = (FAR struct fb_overlayinfo_s *)((uintptr_t) arg);

            DEBUGASSERT(oinfo != 0 && fb->vtable != NULL && fb->vtable->setchromakey != NULL);
            ret = fb->vtable->setchromakey(fb->vtable, oinfo);
        } break;
#endif
    case FBIOSET_COLOR:    /* Set video overlay color */
        {
            FAR struct fb_overlayinfo_s *oinfo = (FAR struct fb_overlayinfo_s *)((uintptr_t) arg);

            DEBUGASSERT(oinfo != 0 && fb->vtable != NULL && fb->vtable->setcolor != NULL);
            ret = fb->vtable->setcolor(fb->vtable, oinfo);
        } break;

    case FBIOSET_BLANK:    /* Blank or unblank video overlay */
        {
            FAR struct fb_overlayinfo_s *oinfo = (FAR struct fb_overlayinfo_s *)((uintptr_t) arg);

            DEBUGASSERT(oinfo != 0 && fb->vtable != NULL && fb->vtable->setblank != NULL);
            ret = fb->vtable->setblank(fb->vtable, oinfo);
        } break;

    case FBIOSET_AREA:    /* Set active video overlay area */
        {
            FAR struct fb_overlayinfo_s *oinfo = (FAR struct fb_overlayinfo_s *)((uintptr_t) arg);

            DEBUGASSERT(oinfo != 0 && fb->vtable != NULL && fb->vtable->setarea != NULL);
            ret = fb->vtable->setarea(fb->vtable, oinfo);
        } break;

#ifdef CONFIG_FB_PANDISPLAY

    case FBIOPAN_DISPLAY:{
            FAR struct fb_planeinfo_s *var = (FAR struct fb_planeinfo_s *)((uintptr_t) arg);
            ret = fb->vtable->pandisplay(fb->vtable, var);
        } break;
#endif

#ifdef CONFIG_FB_VMIRRO

    case FBIOSET_VMIRRO:{
            FAR struct fb_var_screeninfo *var = (FAR struct fb_var_screeninfo *)((uintptr_t) arg);
            ret = fb->vtable->video_mirror(fb->vtable, var);
        } break;
#endif

#ifdef CONFIG_FB_OVERLAY_BLIT
    case FBIOSET_BLIT:    /* Blit operation between video overlays */
        {
            FAR struct fb_overlayblit_s *blit = (FAR struct fb_overlayblit_s *)((uintptr_t) arg);

            DEBUGASSERT(blit != 0 && fb->vtable != NULL && fb->vtable->blit != NULL);
            ret = fb->vtable->blit(fb->vtable, blit);
        } break;

    case FBIOSET_BLEND:    /* Blend operation between video overlays */
        {
            FAR struct fb_overlayblend_s *blend = (FAR struct fb_overlayblend_s *)((uintptr_t) arg);

            DEBUGASSERT(blend != 0 && fb->vtable != NULL && fb->vtable->blend != NULL);
            ret = fb->vtable->blend(fb->vtable, blend);
        } break;
#endif
#endif /* CONFIG_FB_OVERLAY */

    default:
        FB_ERR("ERROR: Unsupported IOCTL command: %d\n", cmd);
        ret = -ENOTTY;
        break;
    }

    return ret;
}

#if !defined(WITH_OS)
static int no_os_ioctl(uint32_t fd, int cmd, void *cmd_data)
{
    int ret = 0;

    ret = fb_ioctl(fd, cmd, (unsigned long)cmd_data);

    return ret;
}

POSSIBLY_UNUSED static char *no_os_mmap(void *addr, size_t length, int prot, int flags, int fd, int32_t offset)
{
    char *mapaddr = 0;

    struct fb_chardev_s *fb = (struct fb_chardev_s *)fd;
    mapaddr = (char *)fb->fbmem;
    return mapaddr;
}
#endif


#define ioctl no_os_ioctl
//#define mmap    no_os_mmap

/**
 * struct fb_chardev_s lcdc_fb0fd - lcdc driver instance handle
 */
static struct fb_chardev_s lcdc_fb0fd;

static bool is_fb0(char *dev_path)
{
    return !memcmp(dev_path, FBDEV_FB0_PATH, strlen(FBDEV_FB0_PATH));
}

static void fbdev_init(uint32_t * fd, char *dev_path)
{
    uint32_t fbfd = 0;

    if (is_fb0(dev_path)) {
        fbfd = (uint32_t) & lcdc_fb0fd;
    } else {
        fbfd = -1;
    }

    *fd = fbfd;

    if (fbfd == -1) {
        FB_ERR("Error: cannot open framebuffer device %s", dev_path);
        return;
    }
    FB_REP("The framebuffer device %s was opened successfully.\n", dev_path);
}

void fbdev_exit(uint32_t *fd)
{
#if defined(NUTTX_OS)
    close(fbfd);
#else
#endif
}

static int fb_init(uint32_t * fd, char *dev_path)
{
    struct fb_chardev_s *fb = 0;
    int ret = 0;
    fbdev_init(fd, dev_path);

    fb = (struct fb_chardev_s *)(*fd);
    FB_INFO("%s fb:%p", __func__, fb);

    if (!is_fb0(dev_path)) {
        return ret;
    }

    if (fb) {
        // Get variable screen information
        g_oinfo[0].overlay = 0;
        ret = ioctl((uint32_t) fb, FBIOGET_OVERLAYINFO, &g_oinfo[0]);
        if (ret) {
            FB_ERR("Error reading overlay information");
            return ret;
        }

        g_oinfo[1].overlay = 1;
        ret = ioctl((uint32_t) fb, FBIOGET_OVERLAYINFO, &g_oinfo[1]);
        if (ret) {
            FB_ERR("Error reading overlay information");
            return ret;
        }
        FB_INFO("The framebuffer device was mapped to memory successfully.\n");
    }

    return ret;
}

int fb_open(void)
{
    fb_init(&fb0fd, FBDEV_FB0_PATH);
    return fb0fd;
}

void fb_close(int display)
{
    if (fb0fd >= 0) {
        fbdev_exit(&fb0fd);

        up_fbuninitialize(display);
    }
}

struct lcd_dev_s g_lcd = {0};


static void fb_lcd_set_disp_area(int x, int y, int w, int h)
{
    uint16_t hstart = 14 + x;
    uint16_t hend = hstart + w - 1;
    uint16_t vstart = y;
    uint16_t vend = vstart + h - 1;
    lpcmd_node_t cmds[] = {
        {0x2a, 4, LPCMD_TYPE_WRITE, { hstart >> 8, hstart & 0xff, hend >> 8, hend & 0xff }},
        {0x2b, 4, LPCMD_TYPE_WRITE, { vstart >> 8, vstart & 0xff, vend >> 8, vend & 0xff }},
    };

#ifdef CONFIG_LCDC_DSI
    hal_dsi_reset(0, w);
#endif

    lpcmds_register_cmds(cmds, ARRAY_SIZE(cmds), LPCMDS_BLOCK);
    lpcmds_flush();
}

static int fb_lcd_get_status(int flag)
{
    lpcmd_node_t cmds[] = { //{ 0xFE, 1, LPCMD_TYPE_WRITE, { 0x00 } },
                              { 0x0A, 1, LPCMD_TYPE_READ, { 0x0 } } };

    lpcmds_register_cmds(cmds, ARRAY_SIZE(cmds), flag);
    FB_INFO("[lcd] panel status register = 0x%x\n", cmds[0].params[0]);
    return cmds[0].params[0];
}

static void fb_lcd_enter_idle_mode(void)
{
    lpcmd_node_t cmds[] = { { 0xFF, 1, LPCMD_TYPE_WRITE, { 0x10 } },
                            { 0xFB, 1, LPCMD_TYPE_WRITE, { 0x01 } },
                            { 0x39, 0, LPCMD_TYPE_WRITE, { 0x0 } },
                            { 0x51, 1, LPCMD_TYPE_WRITE, { 128 } }};

    lpcmds_register_cmds(cmds, ARRAY_SIZE(cmds), LPCMDS_NONBLOCK);
    if (0x9c == fb_lcd_get_status(LPCMDS_BLOCK)) {
        FB_INFO("[lcd] enter idle mode failed, will retry 1 times.");
        lpcmds_register_cmds(cmds, ARRAY_SIZE(cmds), LPCMDS_NONBLOCK);
    }
    FB_INFO("[lcd] lcd enter idle mode, current power = %d\n",
        cmds[3].params[0]);
}

static void fb_lcd_exit_idle_mode(void)
{
    lpcmd_node_t cmds[] = { { 0xFF, 1, LPCMD_TYPE_WRITE, { 0x10 } },
                            { 0xFB, 1, LPCMD_TYPE_WRITE, { 0x01 } },
                            { 0x38, 0, LPCMD_TYPE_WRITE, { 0x0 } },
                            { 0x51, 1, LPCMD_TYPE_WRITE, { 128} } };

    lpcmds_register_cmds(cmds, ARRAY_SIZE(cmds), LPCMDS_NONBLOCK);
    if (0xdc == fb_lcd_get_status(LPCMDS_BLOCK)) {
        FB_INFO("[lcd] exit idle mode failed, will retry 1 times.");
        lpcmds_register_cmds(cmds, ARRAY_SIZE(cmds), LPCMDS_NONBLOCK);
    }

    FB_INFO("[lcd] lcd exit idle mode, current power = %d\n",
        cmds[3].params[0]);
}

static void fb_lcd_enter_standby_mode(void)
{
    lpcmd_node_t cmds[] = { //{ 0xFE, 1, LPCMD_TYPE_WRITE, { 0x00 } },
                                    { 0x51, 1, LPCMD_TYPE_WRITE, { 0x0 } },
                                    { 0x28, 0, LPCMD_TYPE_WRITE, { 0x0 } },
                                    { 0x10, 0, LPCMD_TYPE_WRITE, { 0x0 } },
                                  };

    lpcmds_register_cmds(cmds, ARRAY_SIZE(cmds),
                          LPCMDS_NONBLOCK);
    FB_INFO("[lcd] lcd enter deep standby\n");
    while (0x88 != fb_lcd_get_status(LPCMDS_BLOCK)) {
        FB_INFO("[lcd] enter deep mode failed, will retry 1 times.");
        lpcmds_register_cmds(cmds, ARRAY_SIZE(cmds),
                              LPCMDS_NONBLOCK);
    }
}

static void fb_lcd_exit_standby_mode(void)
{
    lpcmd_node_t cmds[] = { //{ 0xFE, 1, LPCMD_TYPE_WRITE, { 0x00 } },
                                    { 0x51, 1, LPCMD_TYPE_WRITE, { 0xff } },
                                    { 0x11, 0, LPCMD_TYPE_WRITE, { 0x0 } },
                                    { 6, 0, LPCMD_TYPE_DELAY, { 0x0 } },
                                    { 0x29, 0, LPCMD_TYPE_WRITE, { 0x0 } },
                                  };

    lpcmds_clear_buffer();
    lpcmds_register_cmds(cmds, ARRAY_SIZE(cmds), LPCMDS_NONBLOCK);
    FB_INFO("[lcd] lcd exit deep standby\n");
    /* direct flush just can do from task for exit deepsleep*/
    lpcmds_flush();

    while (0x9c != fb_lcd_get_status(LPCMDS_BLOCK|LPCMDS_FLUSH)) {
        FB_INFO("[lcd] exit deep mode failed, will retry 1 times.");
        lpcmds_register_cmds(cmds, ARRAY_SIZE(cmds), LPCMDS_NONBLOCK);
        lpcmds_flush();
    }
}

POSSIBLY_UNUSED
static int putarea(fb_coord_t row_start, fb_coord_t row_end,
               fb_coord_t col_start, fb_coord_t col_end,
               FAR const uint8_t *buffer)
{
    uint16_t width,height;
    uint32_t fbmem = (uint32_t)buffer;
    col_start &= 0xfffe;
    row_start &= 0xfffe;
    width = (col_end - col_start +1)&0xfffe;
    height = (row_end - row_start +1)&0xfffe;

    fb_lcd_set_disp_area(col_start, row_start, width, height);

#ifdef CONFIG_FB_VMIRRO
    if(in_vmirro_mode()) {
        fbmem += (row_end*BOARD_LCDC_WIDTH*CONFIG_LCDC_FB_BPP/8)+
                  col_start*CONFIG_LCDC_FB_BPP/8;
    }
    else
#endif
    {
        fbmem += (row_start*BOARD_LCDC_WIDTH*CONFIG_LCDC_FB_BPP/8)+
                  col_start*CONFIG_LCDC_FB_BPP/8;
    }

    /* Configure Layer start addr register */
    hal_lcdc_lstartaddr(LCDC_LAYER_LFORE, fbmem);
    /* Configure Layer size register */
    hal_lcdc_lsize(LCDC_LAYER_LFORE, width, height);
    hal_lcdc_lsize(LCDC_LAYER_SPU, width, height);
    hal_lcdc_lzoom_set(LCDC_LAYER_LFORE, width, height);
    return 0;
}

static int fb_lcd_getplaneinfo(FAR struct lcd_dev_s *dev,
                               unsigned int planeno,
                               FAR struct lcd_planeinfo_s *pinfo)
{

  pinfo->putarea = putarea;
  return OK;
}

static int fb_lcd_setpower(FAR struct lcd_dev_s *dev, int power)
{
    lpcmd_node_t cmds[] = {
        { 0x51, 1, LPCMD_TYPE_WRITE, {power & 0xff} },
    };
    lpcmds_register_cmds(cmds, ARRAY_SIZE(cmds), LPCMDS_NONBLOCK);
    FB_INFO("set power: %d\n", power);
    return OK;
}

static int fb_lcd_ioctl(FAR struct lcd_dev_s *dev, PANNEL_IOCTL_CMD cmd)
{
    TRACE(0,"ioctl: %d\n", cmd);
    switch(cmd) {
        case PANNEL_ENTER_IDLE:
            fb_lcd_enter_idle_mode();
            break;
        case PANNEL_EXIT_IDLE:
            fb_lcd_exit_idle_mode();
            break;
        case PANNEL_ENTER_DEEPSLEEP:
            fb_lcd_enter_standby_mode();
            break;
        case PANNEL_EXIT_DEEPSLEEP:
            fb_lcd_exit_standby_mode();
            break;
        case PANNEL_READSTATUS:
            return fb_lcd_get_status(LPCMDS_BLOCK);
            break;
        default :
            break;
    }
    return OK;
}


/**
 * board_lcd_getdev - Return a reference to the LCD object for the specified LCD Device.
 *   This allows support for multiple LCD devices.
 * @lcddev - Number of the LDC Device.
 *
 * Returns: Reference to the LCD object if exist otherwise NULL
 *
 */

FAR struct lcd_dev_s *board_lcd_getdev(int lcddev)
{
    g_lcd.getplaneinfo = fb_lcd_getplaneinfo;
    g_lcd.setpower     = fb_lcd_setpower;
    g_lcd.pannel_ioctl = fb_lcd_ioctl;
    return &g_lcd;
}

/**
 * up_fbinitialize - Initialize the framebuffer video hardware associated with the display.
 * @display : In the case of hardware with multiple displays, this
 *     specifies the display.  Normally this is zero.
 *
 * Returns: Zero is returned on success; a negated errno value is returned on any
 *   failure.
 */

static bool initialized = false;
int up_fbinitialize(int display)
{
  int ret = OK;

  if (initialized)
      return ret;

  board_lcd_initialize();
  FB_INFO("FBSIZE: %d\n",FB_BUF_SIZE );
  FB_INFO("FBW: %d\n",BOARD_LCDC_WIDTH );
  FB_INFO("FBH: %d\n",BOARD_LCDC_HEIGHT);

  ret = lcdc_initialize((void *)fb_buf, FB_BUF_SIZE);
  initialized = (ret >= OK);
  return ret;
}

/**
 * up_fbgetvplane -  Return a a reference to the framebuffer object for the specified video
 *   plane of the specified plane.  Many OSDs support multiple planes of video.
 * @display : In the case of hardware with multiple displays, this
 *     specifies the display.  Normally this is zero.
 * @vplane : Identifies the plane being queried.
 *
 * Returns: A non-NULL pointer to the frame buffer access structure is returned on
 *   success; NULL is returned on any failure.
 *
 */

FAR struct fb_vtable_s *up_fbgetvplane(int display, int vplane)
{
    return lcdcgetvplane(vplane);
}

/**
 * up_fbuninitialize - Uninitialize the framebuffer support for the specified display.
 * @display : In the case of hardware with multiple displays, this
 *     specifies the display.  Normally this is zero.
 *
 */

void up_fbuninitialize(int display)
{
    if (initialized) {
        board_lcd_uninitialize();
        lcdcuninitialize();
        initialized = false;
    }
}

/**
 * get_fb_base - Get the fb buffer base address.
 *
 */
uint32_t get_fb_base(void)
{
    return (uint32_t)fb_buf;
}

static void fb_wait_log_out(void)
{
  //  hal_sys_timer_delay_us(5000);
}

POSSIBLY_UNUSED
static int check_overlay_color(struct fb_overlayinfo_s *oinfo, uint32_t color)
{
    #ifdef WB_TEST

    int ret = 0;
    int count = oinfo->fblen * 8 / oinfo->bpp;
    volatile uint32_t val;

    for (int i = 0; i < count; i++) {
         val = (((uint32_t *) oinfo->fbmem)[i] & 0xffffff);
        if (val != color) {
            FB_REP("%s fail pcount:%d fbp:%p oinfo->fbmem[%d] color: %x  != %x", __func__, count,oinfo->fbmem,i,((uint32_t *) oinfo->fbmem)[i], color);
            //TRACE(3, "%s",__func__);
            //while(1);
            ret = -1;
             break;
        }
    }

    FB_REP("%s pass  fbmem:%p", __func__, oinfo->fbmem);
    return ret;
    #else
    return 0;
    #endif
}

#ifdef CONFIG_FB_COLORDITHER
static int check_overlay_color_565(struct fb_overlayinfo_s *oinfo, uint16_t color)
{
    #ifdef WB_TEST
    int ret = 0;
    int count = oinfo->fblen * 8 / oinfo->bpp;

    for (int i = 0; i < count; i++) {
        if ((((uint16_t *) oinfo->fbmem)[i] & 0xffff) != color) {
            FB_REP("fail [%d] 0x%x  != 0x%x", i, ((uint16_t *) oinfo->fbmem)[i], color);
            ret = -1;
            break;
        }
    }

    FB_REP("%s pass  fbmem:%p", __func__, oinfo->fbmem);
    return ret;
    #else
    return 0;
    #endif
}
#endif

#ifdef CONFIG_FB_COLORADJCURVE
static uint8_t cadjcurves_table[256 * 3];
#endif

#ifdef CONFIG_FB_COLORDITHER
static uint32_t cdither_table[4];
#endif

#ifdef CONFIG_FB_COLORDITHER

#define RAND_MAX 2147483647
static uint32_t seed = 1;
static uint32_t rand()
{
    // Based on Knuth "The Art of Computer Programming"
    seed = seed * 1103515245 + 12345;
    return ((uint32_t) (seed / 65536) % (RAND_MAX + 1));
}
#endif

int fb_register(int display, int plane);
extern int lcdc_update(void);

uint8_t yuv_layer[] = {
    0x80,
    0x0,
    0x80,
    0xFF,
};

#ifdef CONFIG_FB_COLORADJCURVE
static int lcdc_set_cadjmap(void)
{
    int ret = 0;
    struct fb_cadjmap_s cadjmap;
    cadjmap.first = 0;
    cadjmap.len = 256;
    cadjmap.blue = &cadjcurves_table[0];
    cadjmap.red = &cadjcurves_table[256];
    cadjmap.green = &cadjcurves_table[512];

    for (int n = cadjmap.first; n < cadjmap.len; n++) {
        cadjmap.blue[n] = n / 2;
        cadjmap.green[n] = n / 2;
        cadjmap.red[n] = n / 2;
    }

    ret = ioctl(fb0fd, FBIOPUT_CADJMAP, &cadjmap);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
    return ret;
}
#endif

#ifdef CONFIG_FB_COLORDITHER
static int lcdc_set_cdithermap(void)
{
    int ret = 0;

    struct fb_cdithermap_s cdithermap;
    cdithermap.first = 0;
    cdithermap.len = 4;
    cdithermap.data = &cdither_table[0];
    for (int n = cdithermap.first; n < cdithermap.len; n++) {
        cdithermap.data[n] = rand();
        cdithermap.data[n] = 0xffffffff;
    }
    ret = ioctl(fb0fd, FBIOPUT_CDITHER, &cdithermap);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }

    return ret;
}
#endif // CONFIG_FB_COLORDITHER

static int lcdc_setarea_test(struct fb_overlayinfo_s *oinfo)
{
    int ret = 0;
#ifdef LCDC_SETAREA_TEST
    FB_REP("lcdc set layer area ");
    oinfo->overlay = 1;
    oinfo->sarea.x = 0;
    oinfo->sarea.y = 0;
    oinfo->sarea.w = oinfo->stride * 8 / oinfo->bpp;
    oinfo->sarea.h = oinfo->fblen / oinfo->stride;
    ret = ioctl(fb0fd, FBIOSET_AREA, &oinfo);

    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
#endif
    return ret;
}

#ifdef CONFIG_FB_UPDATE
POSSIBLY_UNUSED
static int lcdc_update_rect_test(struct fb_overlayinfo_s *oinfo, uint32_t ecolor)
{
    int ret = 0;
    struct fb_area_s uarea;
    uarea.x = 0;
    uarea.y = 0;
    uarea.w = 10;
    uarea.h = 1;

    ret = ioctl(fb0fd, FBIO_UPDATE, &uarea);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
    uarea.x = 0;
    uarea.y = 0;
    uarea.w = 120;
    uarea.h = 120;

    ret = ioctl(fb0fd, FBIO_UPDATE, &uarea);
    if (ret) {
        FB_ERR("Error %s %d", __func__, __LINE__);
        return ret;
    }
    return ret;
}
#endif

#include "hal_dsi.h"
#include "hal_sleep.h"
#include "hal_sysfreq.h"
#include "../../utils/hwtimer_list/hwtimer_list.h"

extern void sleep_test_register_loop_callback(void (*cb)(void));
extern int fb_single_rect_draw_test(uint32_t fb_fd);

#define DSI_1S_FRAME_RATE                   60
#if 1
#define DSI_1S_SYS_FREQ                     HAL_CMU_FREQ_104M
#define DSI_1S_CPU_WORKING_TIME_MS          16
#else
#define DSI_1S_SYS_FREQ                     HAL_CMU_FREQ_208M
#define DSI_1S_CPU_WORKING_TIME_MS          12
#endif

enum FB_DSI_1S_TIMER_EVENT_T {
    FB_DSI_1S_DRAW_START,
};

enum FB_DSI_1S_STATE_T {
    FB_DSI_1S_STATE_LCDC_DONE   = (1 << 0),
    FB_DSI_1S_STATE_GOT_TE      = (1 << 1),
};

enum FB_DSI_1S_STATE_T dsi_1s_state;
static bool dsi_1s_lcdc_done;
static bool dsi_1s_draw_start;
static bool dsi_1s_got_te;
static uint8_t dsi_1s_te_cnt;
static uint32_t dsi_1s_te_time;
static uint32_t dsi_1s_draw_time;
static HWTIMER_ID dsi_1s_timer;

static void fb_dsi_1s_timer_cb(void *param)
{
    enum FB_DSI_1S_TIMER_EVENT_T evt = (enum FB_DSI_1S_TIMER_EVENT_T)param;
    TRACE(0, "%s: evt=%d", __func__, evt);

    if (evt == FB_DSI_1S_DRAW_START) {
        dsi_1s_draw_start = true;
    } else {
        ASSERT(false, "%s: Bad evt=%d", __func__, evt);
    }
    hal_cpu_wake_lock(HAL_CPU_WAKE_LOCK_USER_12);
}

static void fb_dsi_1s_lcdc_irq_cb(int error)
{
    TRACE(0, "%s: error=%d", __func__, error);
    dsi_1s_lcdc_done = true;
    hal_cpu_wake_lock(HAL_CPU_WAKE_LOCK_USER_12);
}

static void fb_dsi_1s_te_gpio_irq(int error)
{
    dsi_1s_te_cnt++;
    if (dsi_1s_te_cnt >= DSI_1S_FRAME_RATE) {
        dsi_1s_te_time = hal_sys_timer_get();
        dsi_1s_got_te = true;
        hal_cpu_wake_lock(HAL_CPU_WAKE_LOCK_USER_12);

        TRACE(0, "Got %u te irq", dsi_1s_te_cnt);
        dsi_1s_te_cnt = 0;
    }
}

static void fb_dsi_1s_loop(void)
{
    uint32_t lock;
    bool lcdc_done;
    bool got_te;
    bool draw_start;
    uint32_t time;
    uint32_t interval;

    lock = int_lock();
    lcdc_done = dsi_1s_lcdc_done;
    dsi_1s_lcdc_done = false;
    got_te = dsi_1s_got_te;
    dsi_1s_got_te = false;
    draw_start = dsi_1s_draw_start;
    dsi_1s_draw_start = false;
    hal_cpu_wake_unlock(HAL_CPU_WAKE_LOCK_USER_12);
    int_unlock(lock);

    if (lcdc_done) {
        dsi_1s_state |= FB_DSI_1S_STATE_LCDC_DONE;

        time = hal_sys_timer_get();
        interval = time - dsi_1s_draw_time;

        TRACE(0, "Draw+LCDC+DSI: %u ms", TICKS_TO_MS(interval));
        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_32K);
    }
    if (got_te) {
        dsi_1s_state |= FB_DSI_1S_STATE_GOT_TE;
    }
    if ((dsi_1s_state & (FB_DSI_1S_STATE_LCDC_DONE | FB_DSI_1S_STATE_GOT_TE)) ==
            (FB_DSI_1S_STATE_LCDC_DONE | FB_DSI_1S_STATE_GOT_TE)) {
        uint32_t frame_ms = 1000 / DSI_1S_FRAME_RATE;
        uint32_t draw_time_ms = frame_ms - (DSI_1S_CPU_WORKING_TIME_MS % frame_ms);
        uint32_t draw_time = MS_TO_TICKS(draw_time_ms);

        dsi_1s_state = 0;
        time = hal_sys_timer_get();
        interval = time - dsi_1s_te_time;
        if (interval < draw_time) {
            interval = draw_time - interval;
            hwtimer_update_then_start(dsi_1s_timer, fb_dsi_1s_timer_cb, (void *)FB_DSI_1S_DRAW_START, interval);
        } else {
            draw_start = true;
        }
    }
    if (draw_start) {
        hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, DSI_1S_SYS_FREQ);

        time = hal_sys_timer_get();
        interval = time - dsi_1s_draw_time;
        dsi_1s_draw_time = time;
        TRACE(0, "Draw interval: %u ms", TICKS_TO_MS(interval));

        lcd_set_start_flag();
        fb_single_rect_draw_test(fb0fd);
        lcd_set_end_flag();
    }
}

int fb_dsi_1s_test(void)
{
    int ret = 0;

    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, DSI_1S_SYS_FREQ);
    dsi_1s_timer = hwtimer_alloc(fb_dsi_1s_timer_cb, NULL);
    ASSERT(dsi_1s_timer, "Failed to alloc hwtimer");
    dsi_1s_draw_time = hal_sys_timer_get();
    lcdc_irq_callback_register(fb_dsi_1s_lcdc_irq_cb);
    sleep_test_register_loop_callback(fb_dsi_1s_loop);
    lcdc_teirq_callback_register(fb_dsi_1s_te_gpio_irq);

    fb_register(0, 0);
    fb_init(&fb0fd, FBDEV_FB0_PATH);
    lcd_gpio_setup();
    lcd_set_start_flag();
    fb_single_rect_draw_test(fb0fd);
    lcd_set_end_flag();
    return ret;
}

/**
 * fb_lcdc_test -  lcdc driver test entry
 */

extern int fb_rect_draw_test(uint32_t fb_fd);

int fb_lcdc_test(void)
{
    int ret = 0;
    POSSIBLY_UNUSED uint32_t ecolor;

    fb_register(0, 0);
    fb_init(&fb0fd, FBDEV_FB0_PATH);
#if 1
    while (1) {        //lcdc test case
        fb_rect_draw_test(fb0fd);
        hal_sys_timer_delay_us(2000 * 1000);
    }
    return ret;
#endif
        struct fb_var_screeninfo var;
        struct fb_overlayinfo_s foinfo;
        struct fb_overlayinfo_s boinfo;

        ret = ioctl(fb0fd, FBIOGET_VIDEOINFO, &var);

        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }

        FB_REP("lcdc set foreground fb0 color with green color");

        foinfo.overlay = 0;
        ret = ioctl(fb0fd, FBIOGET_OVERLAYINFO, &foinfo);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }

        fb_wait_log_out();

        FB_REP("display foreground layer ");

        foinfo.transp.transp = 0x00;    // background alpha
        foinfo.transp.transp_mode = FB_CONST_ALPHA;
        ret = ioctl(fb0fd, FBIOSET_TRANSP, &foinfo);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }

/*chromakey test, fill flayer data with chromakey*/
#ifdef CONFIG_LCDC_L1_CHROMAKEYEN
        uint32_t *pixs = (uint32_t *) foinfo.fbmem;
        int count = 240;    // foinfo.fblen/foinfo.stride;
        for (int i = 0; i < count; i++) {
            pixs[i] = CONFIG_LCDC_CHROMAKEY;
        }
#endif

#ifdef CONFIG_FB_PANDISPLAY
        /*display flayer */
        var.yoffset = 0;
        var.xoffset = 0;

        ret = ioctl(fb0fd, FBIOPAN_DISPLAY, &var);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
         //   return ret;
        }

#ifdef CONFIG_FG_YUV_FMT
        ecolor = 0xff0000;
#else
        ecolor = 0xff0000;
#endif
#ifdef CONFIG_FB_UPDATE

        ret = lcdc_update_rect_test(&foinfo, ecolor);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
          //  return ret;
        }
#endif
        /*display blayer */
        var.yoffset = var.yres;
        ret = ioctl(fb0fd, FBIOPAN_DISPLAY, &var);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
         //   return ret;
        }


#ifdef CONFIG_FG_YUV_FMT
        ecolor = 0xff00;
#else
        ecolor = 0xff00;
#endif
#ifdef CONFIG_FB_UPDATE
        ret = lcdc_update_rect_test(&boinfo, ecolor);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
          //  return ret;
        }
#endif
#else

        /*display flayer only */
        foinfo.transp.transp = 0x00;    // background alpha
        foinfo.transp.transp_mode = FB_CONST_ALPHA;    // FB_PIXEL_ALPHA;//FB_CONST_ALPHA;
        ret = ioctl(fb0fd, FBIOSET_TRANSP, &foinfo);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
         //   return ret;
        }

        lcdc_update();
#endif // CONFIG_FB_PANDISPLAY

        fb_wait_log_out();

        ret = lcdc_setarea_test(&boinfo);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
         //   return ret;
        }
        FB_REP("display  background layer ");

        fb_wait_log_out();

#ifdef LCDC_BLANK_TEST
        /*display blayer only */
        foinfo.overlay = 0;
        foinfo.blank = 1;
        ret = ioctl(fb0fd, FBIOSET_BLANK, &foinfo);

        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }

        boinfo.overlay = 1;
        boinfo.blank = 0;

        ret = ioctl(fb0fd, FBIOSET_BLANK, &boinfo);

        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }

        foinfo.transp.transp = 0xff;    // 0xff;//background alpha
        foinfo.transp.transp_mode = FB_CONST_ALPHA;
        ret = ioctl(fb0fd, FBIOSET_TRANSP, &foinfo);
        if (ret) {
            FB_ERR("Error %s %d", __func__, __LINE__);
            return ret;
        }
#endif

        /*blend display b(a =0x7f) + f(0xff - 0x7f) layers */
        foinfo.transp.transp = 0x7f;
        foinfo.transp.transp_mode = FB_CONST_ALPHA;
#ifdef CONFIG_FB_COLORDITHER
        ecolor = 0x8e8686;
#else
        ecolor = 0x7f7f00;
#endif

#if defined(WITH_OS)
        munmap(fbp, screensize);
        close(fp);
#endif
        FB_REP("lcdc test pass ");
        return ret;
#if 0
    }
#endif
}


int fb_buffer_draw_test(uint32_t fb_fd,uint8_t *buffer);
int fb_gpu_test_init(void);
static bool is_fb_setup = false;

int fb_lcdc_display_buffer_test(uint8_t *buffer)
{
    if (!is_fb_setup) {
       fb_gpu_test_init();
      // is_fb_setup = true;
    }
    fb_buffer_draw_test(fb0fd,buffer);
    return 0;
}
#ifdef CONFIG_FB_PANDISPLAY
static struct fb_var_screeninfo  screen_ctx;
static struct fb_planeinfo_s     screen_fb;

uint32_t  fb_update_display_buffer(void)
{
    int ret;
    TRACE(3,"%s %d",__func__, __LINE__);
    ret = ioctl(fb0fd, FBIOPAN_DISPLAY, &screen_fb);
    if (ret) {
       FB_ERR("%s pan display err:%d",__func__, ret);
       return ret;
    }
    screen_fb.yoffset = (screen_fb.yoffset > 0) ? 0: screen_ctx.yres;
    return ret;
}

uint32_t  fb_get_display_buffer(void)
{
    int ret;
    uint32_t fbmem = (uint32_t)NULL;
    struct fb_videoinfo_s vinfo;
    TRACE(3,"%s %d",__func__, __LINE__);

    if (!is_fb_setup) {
       fb_gpu_test_init();
       is_fb_setup = true;

       screen_ctx.yoffset = 0;
       screen_ctx.xoffset = 0;
       vinfo.noverlays = 0;
       ret = ioctl(fb0fd, FBIOGET_VIDEOINFO, &vinfo);
       if (ret) {
          FB_ERR("%s get video info err:%d",__func__, ret);
          return 0;
       }
       screen_ctx.xres = vinfo.xres;
       screen_ctx.yres = vinfo.yres;

       TRACE(3,"%s %d",__func__, __LINE__);
       ret = ioctl(fb0fd, FBIOGET_PLANEINFO, &screen_fb);
       if (ret) {
          FB_ERR("%s get plane info err:%d",__func__, ret);
          return 0;
       }
       screen_fb.yoffset = 0;
       screen_fb.xoffset = 0;
       TRACE(3,"%s yres: %d",__func__, screen_ctx.yres);
       TRACE(3,"%s stride: %d",__func__, screen_fb.stride);

    }

    fbmem = (uint32_t)screen_fb.fbmem;
    if (screen_fb.yoffset > 0) {
        #ifdef USE_GAMU_FB
            if (screen_fb.bpp == 24) {
                fbmem += 0x100000;
            } else {
                fbmem += screen_ctx.yres * screen_fb.stride;
            }
        #else
        fbmem += screen_ctx.yres * screen_fb.stride;
        #endif
        if (fbmem%64){
            fbmem = (fbmem + 63) & ~63;
        }
    }
    TRACE(3,"%s fbmem:0x%x",__func__, fbmem);
    return fbmem;
}
#endif

int fb_lcdc_vmirro_enable(bool vmirro)
{
#ifdef CONFIG_FB_VMIRRO
    struct fb_var_screeninfo var;
    var.xoffset = 0;
    var.yoffset = 0;
    if (vmirro) {
        var.rotate = 180;
    } else {
        var.rotate = 0;
    }
    if (fb0fd) {
        fb_ioctl(fb0fd, FBIOSET_VMIRRO,
              (unsigned long)((uintptr_t)&var));
    } else {
        TRACE(0,"%s err: fb not open", __func__);
    }
#endif
    return 0;
}

#define FB_LOOP_TEST

int fb_gpu_test_init(void)
{
    fb_register(0, 0);
    fb_init(&fb0fd, FBDEV_FB0_PATH);
    return 0;
}

//#define  LCDC_TEST

int fb_graphic_test(void)
{
    int ret = 0;
    FB_REP("lcdc test ");
//    hal_cache_disable(HAL_CACHE_ID_D_CACHE);

#ifdef TEST_WITH_LCD

    ret = fb_lcdc_test();
    if (ret) {
        FB_REP("lcdc test fail !!!!!!");
        ret = -1;
    }
#endif
    return ret;
}

void fb_pollnotify(FAR struct fb_vtable_s *vtbl)
{
}

/**
 * fb_register - Register the framebuffer character device
 *
 * Register the framebuffer character device at /dev/fbN where N is the
 *   display number if the devices supports only a single plane.  If the
 *   hardware supports multiple color planes, then the device will be
 *   registered at /dev/fbN.M where N is the again display number but M
 *   is the display plane.
 */

int fb_register(int display, int plane)
{
    FAR struct fb_chardev_s *fb;
    struct fb_videoinfo_s vinfo;
    struct fb_planeinfo_s pinfo;
#ifdef CONFIG_FB_OVERLAY
    struct fb_overlayinfo_s oinfo;
#endif
    // char devname[16];
    // uint8_t nplanes;

    int ret;

    /* Allocate a framebuffer state instance */

#if 0
    fb = (FAR struct fb_chardev_s *)
        kmm_zalloc(sizeof(struct fb_chardev_s));
#else
    if (display == 0) {
        fb = &lcdc_fb0fd;
    } else {
        FB_ERR("%s only one framebuffer !!!!!!", __func__);
    }

#endif

    if (fb == NULL) {
        return -1;
    }

    /* Initialize the frame buffer device. */
    /*dsi interface setup*/

    ret = up_fbinitialize(display);
    if (ret < 0) {
        FB_ERR("ERROR: up_fbinitialize() failed for display %d: %d\n", display, ret);
        goto errout_with_fb;
    }

    DEBUGASSERT((unsigned)plane <= UINT8_MAX);
    fb->plane = plane;

    fb->vtable = up_fbgetvplane(display, plane);
    if (fb->vtable == NULL) {
        FB_ERR("ERROR: up_fbgetvplane() failed, vplane=%d\n", plane);
        goto errout_with_fb;
    }

    /* Initialize the frame buffer instance. */

    DEBUGASSERT(fb->vtable->getvideoinfo != NULL);
    ret = fb->vtable->getvideoinfo(fb->vtable, &vinfo);
    if (ret < 0) {
        FB_ERR("ERROR: getvideoinfo() failed: %d\n", ret);
        goto errout_with_fb;
    }
#ifdef WITH_OS
    uint8_t nplanes;
    nplanes = vinfo.nplanes;
    DEBUGASSERT(vinfo.nplanes > 0 && (unsigned)plane < vinfo.nplanes);
#endif

    DEBUGASSERT(fb->vtable->getplaneinfo != NULL);
    ret = fb->vtable->getplaneinfo(fb->vtable, plane, &pinfo);
    if (ret < 0) {
        FB_ERR("ERROR: getplaneinfo() failed: %d\n", ret);
        goto errout_with_fb;
    }

    fb->fbmem = pinfo.fbmem;
    fb->fblen = pinfo.fblen;
    fb->bpp = pinfo.bpp;

    /* Clear the framebuffer memory */
#ifndef FB_SIM
    #ifdef USE_GAMU_FB
        if (fb->bpp == 24) {
            memset(pinfo.fbmem, 0, pinfo.fblen/2);
            memset(pinfo.fbmem + 0x100000, 0, pinfo.fblen/2);

        } else {
            memset(pinfo.fbmem, 0, pinfo.fblen);
        }

    #else
        memset(pinfo.fbmem, 0, pinfo.fblen);
    #endif
#endif

#ifdef CONFIG_FB_OVERLAY
    /* Initialize first overlay but do not select */

    DEBUGASSERT(fb->vtable->getoverlayinfo != NULL);
    ret = fb->vtable->getoverlayinfo(fb->vtable, 0, &oinfo);
    if (ret < 0) {
        FB_ERR("ERROR: getoverlayinfo() failed: %d\n", ret);
        goto errout_with_fb;
    }

    /* Clear the overlay memory. Necessary when plane 0 and overlay 0
     * different.
     */

#ifndef FB_SIM
  //  memset(oinfo.fbmem, 0, oinfo.fblen);
#endif
#endif

#ifdef WITH_OS
    /* Register the framebuffer device */

    if (nplanes < 2) {
        snprintf(devname, 16, "/dev/fb%d", display);
    } else {
        snprintf(devname, 16, "/dev/fb%d.%d", display, plane);
    }

    ret = register_driver(devname, &fb_fops, 0666, (FAR void *)fb);
#endif

    if (ret < 0) {
        FB_ERR("ERROR: register_driver() failed: %d\n", ret);
        goto errout_with_fb;
    }

    return OK;

errout_with_fb:
#if 0
    kmm_free(fb);
#endif
    return ret;
}

#include "hal_lcdc.h"
#include "lpcmd.h"

#define PICTURE_BUFFER_ATT POSSIBLY_UNUSED static const

PICTURE_BUFFER_ATT uint8_t background_368_448[] = {
#include "display_resource/RGB888_unpacked_368x448.txt"
};

struct fb_planeinfo_s g_pinfo;
struct fb_videoinfo_s g_vinfo;

static bool lcd_frame_done = 0;
static uint32_t lcd_frame_count = 0;
static uint32_t lcd_te_count = 0;
static uint32_t yoffset = 1;
static uint32_t first_framedone_time = 0;

#ifdef FB_DSI_60HZ_TEST_CURRENT_TEST_1HZ_REFRESH
#include "hal_iomux.h"
#include "hal_gpio.h"
static const struct HAL_IOMUX_PIN_FUNCTION_MAP fb_gpio_iomux[] = {
    {HAL_IOMUX_PIN_P0_0, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLDOWN_ENABLE},
    {HAL_IOMUX_PIN_P0_1, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLDOWN_ENABLE},
    {HAL_IOMUX_PIN_P0_2, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLDOWN_ENABLE},
};

void fb_gpio_set(int pin)
{
    hal_gpio_pin_set((enum HAL_GPIO_PIN_T)fb_gpio_iomux[pin].pin);
}
void fb_gpio_clr(int pin)
{
    hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)fb_gpio_iomux[pin].pin);
}
void fb_gpio_open(void)
{
    int i = 0;
    hal_iomux_init(fb_gpio_iomux, ARRAY_SIZE(fb_gpio_iomux));
    for (i = 0; i < ARRAY_SIZE(fb_gpio_iomux); ++i) {
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)fb_gpio_iomux[i].pin, HAL_GPIO_DIR_OUT, 0);
    }
}
#endif

static void fb_dsi_60hz_lcdc_irq_cb(int error)
{
    if (lcd_frame_done == 0)
        first_framedone_time = GET_CURRENT_MS();
    lcd_frame_done = 1;
    lcd_frame_count ++;

#if defined(FB_DSI_60HZ_TEST_CURRENT_TEST_1HZ_REFRESH)
    fb_gpio_clr(1);
#endif

#if 0//!defined(CONFIG_TRIGGER_FRAME_BY_SW)
    if (g_pinfo.yres_virtual == g_vinfo.yres)
        yoffset = 0;
    uint8_t *row = (uint8_t *)g_pinfo.fbmem + g_pinfo.stride * yoffset;
    if (yoffset < g_vinfo.yres)
        yoffset += 1;
    else
        yoffset = 0;
    hal_lcdc_lstartaddr(0, (uint32_t)row);
#if !defined(CONFIG_DSI_VIDEO_MODE)
    hal_lcdc_start_dsi();
#endif
#endif
}

static void ulps_exit_hander(int fb)
{
#if defined(FB_DSI_60HZ_TEST_CURRENT_TEST_1HZ_REFRESH)
    fb_gpio_clr(0);
    fb_gpio_set(1);
#endif
    hal_lcdc_lstartaddr(0, (uint32_t)fb);
    lcdc_trigger_frame();
}

static void fb_dsi_60hz_te_gpio_irq(int error)
{
    int refresh = 1;
    if (lcd_frame_done > 0)
    {
        lcd_te_count++;

        if (lcd_te_count >= 61)
        {
            uint32_t current_ms = GET_CURRENT_MS();
            if (current_ms - first_framedone_time >0) {
                uint32_t frame_vsync_rate = 1000 / ((current_ms - first_framedone_time) / lcd_te_count);
                uint32_t frame_done_rate = 1000 / ((current_ms - first_framedone_time) / lcd_frame_count);
                first_framedone_time = current_ms;
                FB_REP("framedone rate:%d, vsync rate:%d\n",frame_done_rate, frame_vsync_rate);
            }
            lcd_te_count = 0;
            lcd_frame_count = 0;

        }
#if defined(FB_DSI_60HZ_TEST_CURRENT_TEST_1HZ_REFRESH)
        else {
            refresh = 0;
        }
#endif

        if (refresh) {
            if (g_pinfo.yres_virtual == g_vinfo.yres)
                yoffset = 0;
            uint8_t *row = (uint8_t *)g_pinfo.fbmem + g_pinfo.stride * yoffset;
            if (yoffset < g_vinfo.yres)
                yoffset += 1;
            else
                yoffset = 0;
#if defined(FB_DSI_60HZ_TEST_CURRENT_TEST_1HZ_REFRESH)
            fb_gpio_set(0);
#endif
            lcdc_exit_lowpower_with_cb(ulps_exit_hander,row);
        }
    }
}

static HWTIMER_ID dsi_lpcmd_timer;
static bool lpcmd_test_update;

static void fb_dsi_lpcmd_timer_cb(void *param)
{
    lpcmd_test_update = true;
}

static void do_lpcmd(void)
{
    if (lpcmd_test_update)
    {
        static bool onoff = 0;
        TRACE(0, "%s: %s", __func__, onoff?"on":"off");
        if (onoff == 0)
        {
            lcdc_pannel_enter_deepstandby();
        }
        else
        {
            lcdc_exit_lowpower();
            lcdc_pannel_exit_deepstandby();
            if (g_pinfo.yres_virtual == g_vinfo.yres)
                yoffset = 0;
            uint8_t *row = (uint8_t *)g_pinfo.fbmem + g_pinfo.stride * yoffset;
            if (yoffset < g_vinfo.yres)
                yoffset += 1;
            else
                yoffset = 0;

            hal_lcdc_lstartaddr(0, (uint32_t)row);
            lcdc_trigger_frame();
        }
        onoff = !onoff;
        lpcmd_test_update = false;
        hwtimer_start(dsi_lpcmd_timer, MS_TO_TICKS(1000));
    }
}

int  fb_dsi_60hz_test()
{
    lcdc_irq_callback_register(fb_dsi_60hz_lcdc_irq_cb);
    lcdc_teirq_callback_register(fb_dsi_60hz_te_gpio_irq);
    dsi_lpcmd_timer = hwtimer_alloc(fb_dsi_lpcmd_timer_cb, NULL);
#ifndef CONFIG_DSI_VIDEO_MODE
    hwtimer_start(dsi_lpcmd_timer, MS_TO_TICKS(1000));
#endif
    sleep_test_register_loop_callback(do_lpcmd);

    fb_register(0, 0);
    fb_init(&fb0fd, FBDEV_FB0_PATH);

#if defined(FB_DSI_60HZ_TEST_CURRENT_TEST_1HZ_REFRESH)
    fb_gpio_open();
#endif

    int ret = fb_ioctl(fb0fd, FBIOGET_VIDEOINFO,
                (unsigned long)((uintptr_t)&g_vinfo));

    FB_REP("VideoInfo:%d\n",ret);
    FB_REP("      fmt: %u\n", g_vinfo.fmt);
    FB_REP("     xres: %u\n", g_vinfo.xres);
    FB_REP("     yres: %u\n", g_vinfo.yres);
    FB_REP("  nplanes: %u\n", g_vinfo.nplanes);

    ret = fb_ioctl(fb0fd, FBIOGET_PLANEINFO,
                (unsigned long)((uintptr_t)&g_pinfo));
    FB_REP("PlaneInfo (plane 0):%d\n",ret);
    FB_REP("    fbmem: %p\n", g_pinfo.fbmem);
    FB_REP("    fblen: %lu\n", (unsigned long)g_pinfo.fblen);
    FB_REP("   stride: %u\n", g_pinfo.stride);
    FB_REP("  display: %u\n", g_pinfo.display);
    FB_REP("      bpp: %u\n", g_pinfo.bpp);

    int xoff = (g_vinfo.xres - 368) /2;
    int yoff = (g_vinfo.yres - 448) /2;

    memset(g_pinfo.fbmem,0x00,g_pinfo.fblen);

    if (xoff >= 0 && yoff >= 0)
    {
        for (int y = 0; y < 448; y++)
          {
            uint8_t *dist_addr = (uint8_t *)g_pinfo.fbmem + g_pinfo.stride * (yoff+y) + (g_pinfo.bpp/8)*xoff;
            uint8_t *src_addr = (uint8_t *)background_368_448 + 368*4*y;
            memcpy(dist_addr,src_addr,368*4);
          }
        if (g_pinfo.yres_virtual == (g_vinfo.yres * 2))
            memcpy(g_pinfo.fbmem + g_pinfo.fblen/2, g_pinfo.fbmem, g_pinfo.fblen/2);
    }
    else
    {
        for (int y = 0; y < (yoff<0?g_vinfo.yres:448); y++)
          {
            memcpy(g_pinfo.fbmem+y*g_pinfo.stride, (uint8_t *)background_368_448 + 368*4*y, g_pinfo.stride);
          }
        if (g_pinfo.yres_virtual == (g_vinfo.yres * 2))
            memcpy(g_pinfo.fbmem + g_pinfo.fblen/2, g_pinfo.fbmem, g_pinfo.fblen/2);
    }

    struct fb_area_s area;
    area.x = 0;
    area.y = 0;
    area.w = g_vinfo.xres;
    area.h = g_vinfo.yres;
    ret = fb_ioctl(fb0fd, FBIO_UPDATE,
                (unsigned long)((uintptr_t)&area));
    FB_REP("fb_ioctl(FBIO_UPDATE) ret: %d\n",ret);
    return ret;
}

static HWTIMER_ID dsi_60hz_cpu_sim_timer;

static void fb_dsi_60hz_cpu_sim_timer_cb(void *param)
{
    lcd_te_count++;
    lcd_frame_count ++;
    if (lcd_te_count == 61)
    {
        FB_REP("frame rate:%d\n",lcd_frame_count);
        lcd_te_count = 0;
        lcd_frame_count = 0;
    }
    hwtimer_start(dsi_60hz_cpu_sim_timer, MS_TO_TICKS(16));
}

static void do_dsi_60hz_cpu_sim_sleep_cb(void)
{
}

int  fb_dsi_60hz_cpu_sim_test()
{
    FB_REP("fb dsi 60hz cpu sim start\n");

    lcd_te_count = 0;
    lcd_frame_count = 0;

    dsi_60hz_cpu_sim_timer = hwtimer_alloc(fb_dsi_60hz_cpu_sim_timer_cb, NULL);
    hwtimer_start(dsi_60hz_cpu_sim_timer, MS_TO_TICKS(16));
    sleep_test_register_loop_callback(do_dsi_60hz_cpu_sim_sleep_cb);

    return 0;
}

