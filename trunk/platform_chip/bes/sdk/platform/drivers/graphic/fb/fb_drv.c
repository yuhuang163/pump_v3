
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


#include "cmsis_nvic.h"
#include "hal_cache.h"
#include "hal_psram.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_lcdc.h"
#include "heap_api.h"
#include "string.h"
#include "stdio.h"
#include "hal_gpio.h"
#include "hal_iomux.h"
#include "hal_location.h"
#include "hal_dsi.h"

#include "fb_drv.h"
#include "fb_lcdc.h"
#include "fb_panel.h"
#include "fb_lpcmd.h"

#include "gra_sem.h"
/**
 *      DEFINES
 */
#ifndef FBDEV_PATH

#define FBDEV_FB0_PATH "/dev/fb0"

#endif

#define FB_BUF_SIZE  (CONFIG_LCDC_FB_SIZE)

#if defined(DDR_ENABLE)
#define FB_LOC       DDR_BSS_LOC
#elif defined(PSRAMUHS_ENABLE)
#define FB_LOC       PSRAMUHS_NC_BSS_LOC
#elif defined(PSRAM_ENABLE)
#define FB_LOC       PSRAM_NC_BSS_LOC
#else
#define FB_LOC
#endif

#ifdef FB_TEST
static char *fb_buf = NULL;
static sem_t frame_send;
static bool sem_initialized = false;
#else
static bool is_fb_setup = false;
static char fb_buf[FB_BUF_SIZE] ALIGNED(64) FB_LOC;
#endif

#ifdef  WITH_OS
#undef  WITH_OS
#endif

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
static char devname[16];

/**
 * struct fb_chardev_s - This structure defines one framebuffer device.
 *
 * Note that which is everything in this structure is constant data set up and
 * initialization time.  Therefore, no there is requirement for serialized access to this
 * structure.
 */

struct fb_chardev_s {
    FAR struct fb_vtable_s *vtable;    /* Framebuffer interface */
    FAR void *fbmem;    /* Start of frame buffer memory */
    size_t fblen;        /* Size of the framebuffer */
    uint8_t plane;        /* Video plan number */
    uint8_t bpp;        /* Bits per pixel */
};

static struct fb_chardev_s fb_priv;

/****************************************************************************
 * Name: fb_remove_paninfo
 * Description:
 *   Remove a frame from pan info queue of the specified overlay.
 *
 * Input Parameters:
 *   vtable  - Pointer to framebuffer's virtual table.
 *   overlay - Overlay index.
 *
 * Returned Value:
 *   Zero is returned on success; a negated errno value is returned on any
 *   failure.
 ****************************************************************************/

int fb_remove_paninfo(FAR struct fb_vtable_s *vtable, int overlay)
{
    if (gra_sem_post(&frame_send) < 0)
        FB_ERR("%s: semaphore post failed", __func__);
    return 0;
}

void poll(void)
{
    if (!sem_initialized) {
        if(gra_sem_init(&frame_send, 0, 2) < 0)
            FB_ERR("%s: semaphore initialized failed", __func__);
        sem_initialized = true;
    }
    if (gra_sem_wait(&frame_send) < 0)
        FB_ERR("%s: semaphore wait failed", __func__);
}

/**
 * fb_ioctl  - lcdc framebuffer driver ioctrl interface
 *
 */

int fb_ioctl(uint32_t fd, int cmd, unsigned long arg)
{
    FAR struct fb_chardev_s *fb;
    int ret = -1;

    FB_INFO("%s fd:%x cmd: %d arg: %lx\n", __func__, fd, cmd, arg);

    /* Get the framebuffer instance */

    fb = &fb_priv;

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

    case FBIOSET_POWER:    /* Set power */
        {
            DEBUGASSERT(fb->vtable != NULL && fb->vtable->setpower != NULL);
            ret = fb->vtable->setpower(fb->vtable, (int)arg);
        } break;

    case FBIOGET_POWER:    /* Set power */
        {
            DEBUGASSERT(fb->vtable != NULL && fb->vtable->getpower != NULL);
            FAR int *power = (FAR int *)((uintptr_t)arg);
            *power = fb->vtable->getpower(fb->vtable);
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
            ASSERT(oinfo != 0 && fb->vtable != NULL && fb->vtable->getoverlayinfo != NULL, "%s", __func__);

            ret = fb->vtable->getoverlayinfo(fb->vtable, oinfo->overlay, oinfo);
        }
        break;

    case FBIOPAN_OVERLAY:
        {
            FAR struct fb_overlayinfo_s *oinfo = (FAR struct fb_overlayinfo_s *)((uintptr_t) arg);
            ASSERT(oinfo != 0 && fb->vtable != NULL && fb->vtable->panoverlay != NULL, "%s", __func__);

            ret = fb->vtable->panoverlay(fb->vtable, oinfo);
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
        if (fb->vtable->ioctl != NULL) {
            ret = fb->vtable->ioctl(fb->vtable, cmd, arg);
        } else {
            FB_ERR("ERROR: Unsupported IOCTL command: %d\n", cmd);
            ret = -1;
        }
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

#ifndef FB_TEST
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
#endif

/**
 * get_fb_base - Get the fb buffer base address.
 *
 */
uint32_t get_fb_base(void)
{
    return (uint32_t)fb_buf;
}

#ifndef FB_TEST
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

static int fb_register_device(int display, int plane,
                       FAR struct fb_vtable_s *vtable)
{
    fb_priv.plane  = plane;
    fb_priv.vtable = vtable;

    /* Initialize the frame buffer instance. */

    vtable->priv = (void *)&fb_priv;
    snprintf(devname, 16, "/dev/fb%d", display);
    return OK;
}

/**
 * fb_dev_register - Register the framebuffer character device
 *
 * Register the framebuffer character device at /dev/fbN where N is the
 *   display number if the devices supports only a single plane.  If the
 *   hardware supports multiple color planes, then the device will be
 *   registered at /dev/fbN.M where N is the again display number but M
 *   is the display plane.
 */

int fb_dev_register(int display, int plane)
{
    FAR struct fb_vtable_s *vtable;
    int ret;

    /* Initialize the frame buffer device. */

    ret = up_fbinitialize(display);
    if (ret < 0) {
        FB_ERR("ERROR: up_fbinitialize() failed for display %d: %d\n", display, ret);
        return ret;
    }

    vtable = up_fbgetvplane(display, plane);
    if (vtable == NULL) {
        FB_ERR("ERROR: up_fbgetvplane() failed, vplane=%d\n", plane);
        return -EINVAL;
    }

    return fb_register_device(display, plane, vtable);
}
