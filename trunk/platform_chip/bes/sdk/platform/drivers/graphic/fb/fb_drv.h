
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

#ifndef __FB_DRV_H__
#define __FB_DRV_H__

/****************************************************************************
 * Included Files
 ****************************************************************************/

#include "stdint.h"
#include "stdbool.h"
#include "string.h"
#include "fb_conf.h"

#define _FBIOC(x)   (x)
#define FAR

#define FB_DEBUG_LEVEL0
//#define FB_DEBUG_LEVEL1
//#define FB_DEBUG_LEVEL2

#ifdef FB_DEBUG_LEVEL0
#define FB_ERR(...)  TR_ERROR(TR_MOD(TEST), "//" __VA_ARGS__)
#else
#define FB_ERR(...)
#endif

#ifdef FB_DEBUG_LEVEL1
#define FB_REP(...)  TR_INFO(TR_MOD(TEST), "//" __VA_ARGS__)
#else
#define FB_REP(...)
#endif

#ifdef FB_DEBUG_LEVEL2
#define FB_INFO(...) TR_INFO(TR_MOD(TEST), "//" __VA_ARGS__)
#else
#define FB_INFO(...)
#endif

#define DEBUGASSERT(x) ASSERT((x), "%s %d", __func__, __LINE__)

/**
 * Public Types
 */
enum PANNEL_IOCTL_CMD {
    PANNEL_ENTER_IDLE,
    PANNEL_EXIT_IDLE,
    PANNEL_ENTER_DEEPSLEEP,
    PANNEL_EXIT_DEEPSLEEP,
    PANNEL_READSTATUS,
};

#if !defined(__NuttX__)
enum Status {
    ERROR = -1,
    OK = 0,
    ECANCELED = 1,
    ENOTTY = 2,
    EINVAL = 3,
    ENOSYS = 4,
};

#define LV_MATH_UDIV255(x) ((uint32_t)((uint32_t) (x) * 0x8081) >> 0x17)
#define LV_MATH_ABS(x) ((x) > 0 ? (x) : (-(x)))

/**
 * Pre-processor definitions
 */

/**
 *  Color format definitions.  The pretty much define the color pixel processing
 *  organization of the video controller.
 */

/* Monochrome Formats *******************************************************/

#define FB_FMT_Y1             0    /* BPP=1, monochrome */
#define FB_FMT_Y2             1    /* BPP=2, 2-bit uncompressed greyscale */
#define FB_FMT_Y4             2    /* BPP=4, 4-bit uncompressed greyscale */
#define FB_FMT_Y8             3    /* BPP=8, 8-bit uncompressed greyscale */
#define FB_FMT_Y16            4    /* BPP=16, 16-bit uncompressed greyscale */
#define FB_FMT_GREY           FB_FMT_Y8    /* BPP=8 */
#define FB_FMT_Y800           FB_FMT_Y8    /* BPP=8 */

#define FB_ISMONO(f)          ((f) >= FB_FMT_Y4) && (f) <= FB_FMT_Y16)

/* RGB video formats */

/* Standard RGB */

#define FB_FMT_RGB1           FB_FMT_Y1    /* BPP=1 */
#define FB_FMT_RGB4           5    /* BPP=4 */
#define FB_FMT_RGB8           6    /* BPP=8 RGB palette index */
#define FB_FMT_RGB8_222       7    /* BPP=8  R=2, G=2, B=2 */
#define FB_FMT_RGB8_332       8    /* BPP=8  R=3, G=3, B=2 */
#define FB_FMT_RGB12_444      9    /* BPP=12 R=4, G=4, B=4 */
#define FB_FMT_RGB16_555      10    /* BPP=16 R=5, G=5, B=5 (1 unused bit) */
#define FB_FMT_RGB16_565      11    /* BPP=16 R=6, G=6, B=5 */
#define FB_FMT_RGB24          12    /* BPP=24 */
#define FB_FMT_RGB32          13    /* BPP=32 */

/* Run length encoded RGB */

#define FB_FMT_RGBRLE4        14    /* BPP=4 */
#define FB_FMT_RGBRLE8        15    /* BPP=8 */

/* Raw RGB */

#define FB_FMT_RGBRAW         16    /* BPP=? */

/**
 * Raw RGB with arbitrary sample packing within a pixel. Packing and precision
 * of R, G and B components is determined by bit masks for each.
 */

#define FB_FMT_RGBBTFLD16     17    /* BPP=16 */
#define FB_FMT_RGBBTFLD24     18    /* BPP=24 */
#define FB_FMT_RGBBTFLD32     19    /* BPP=32 */
#define FB_FMT_RGBA16         20    /* BPP=16 Raw RGB with alpha */
#define FB_FMT_RGBA32         21    /* BPP=32 Raw RGB with alpha */

/* Raw RGB with a transparency field. Layout is as for standard RGB at 16 and
 * 32 bits per pixel but the msb in each pixel indicates whether the pixel is
 * transparent or not.
 */

#define FB_FMT_RGBT16         22    /* BPP=16 */
#define FB_FMT_RGBT32         23    /* BPP=32 */

#define FB_ISRGB(f)           ((f) >= FB_FMT_RGB1) && (f) <= FB_FMT_RGBT32)

/* Packed YUV Formats */

#define FB_FMT_AYUV           24    /* BPP=32  Combined YUV and alpha */
#define FB_FMT_CLJR           25    /* BPP=8   4 pixels packed into a uint32_t.
                                             *         YUV 4:1:1 with l< 8 bits
                                             *         per YUV sample */
#define FB_FMT_CYUV           26    /* BPP=16  UYVY except that height is
                                             *         reversed */
#define FB_FMT_IRAW           27    /* BPP=?   Intel uncompressed YUV */
#define FB_FMT_IUYV           28    /* BPP=16  Interlaced UYVY (line order
                                             *         0,2,4,.., 1,3,5...) */
#define FB_FMT_IY41           29    /* BPP=12  Interlaced Y41P (line order
                                             *         0,2,4,.., 1,3,5...) */
#define FB_FMT_IYU2           30    /* BPP=24 */
#define FB_FMT_HDYC           31    /* BPP=16  UYVY except uses the BT709
                                             *         color space  */
#define FB_FMT_UYVP           32    /* BPP=24? YCbCr 4:2:2, 10-bits per
                                             *         component in U0Y0V0Y1 order */
#define FB_FMT_UYVY           33    /* BPP=16  YUV 4:2:2 */
#define FB_FMT_UYNV           FB_FMT_UYVY    /* BPP=16  */
#define FB_FMT_Y422           FB_FMT_UYVY    /* BPP=16  */
#define FB_FMT_V210           34    /* BPP=32  10-bit 4:2:2 YCrCb */
#define FB_FMT_V422           35    /* BPP=16  Upside down version of UYVY */
#define FB_FMT_V655           36    /* BPP=16? 16-bit YUV 4:2:2 */
#define FB_FMT_VYUY           37    /* BPP=?   ATI Packed YUV Data */
#define FB_FMT_YUYV           38    /* BPP=16  YUV 4:2:2 */
#define FB_FMT_YUY2           FB_FMT_YUYV    /* BPP=16  YUV 4:2:2 */
#define FB_FMT_YUNV           FB_FMT_YUYV    /* BPP=16  YUV 4:2:2 */
#define FB_FMT_YVYU           39    /* BPP=16  YUV 4:2:2 */
#define FB_FMT_Y41P           40    /* BPP=12  YUV 4:1:1 */
#define FB_FMT_Y411           41    /* BPP=12  YUV 4:1:1 */
#define FB_FMT_Y211           42    /* BPP=8  */
#define FB_FMT_Y41T           43    /* BPP=12  Y41P LSB for transparency */
#define FB_FMT_Y42T           44    /* BPP=16  UYVY LSB for transparency */
#define FB_FMT_YUVP           45    /* BPP=24? YCbCr 4:2:2 Y0U0Y1V0 order */

#define FB_ISYUVPACKED(f)     ((f) >= FB_FMT_AYUV) && (f) <= FB_FMT_YUVP)

/* Packed Planar YUV Formats */

#define FB_FMT_YVU9           46    /* BPP=9   8-bit Y followed by 8-bit
                                             *         4x4 VU */
#define FB_FMT_YUV9           47    /* BPP=9? */
#define FB_FMT_IF09           48    /* BPP=9.5 YVU9 + 4x4 plane of delta
                                             *         relative to tframe. */
#define FB_FMT_YV16           49    /* BPP=16  8-bit Y followed by 8-bit
                                             *         2x1 VU */
#define FB_FMT_YV12           50    /* BPP=12  8-bit Y followed by 8-bit
                                             *         2x2 VU */
#define FB_FMT_I420           51    /* BPP=12  8-bit Y followed by 8-bit
                                             *         2x2 UV */
#define FB_FMT_IYUV           FB_FMT_I420    /* BPP=12 */
#define FB_FMT_NV12           52    /* BPP=12  8-bit Y followed by an
                                             *         interleaved 2x2 UV */
#define FB_FMT_NV21           53    /* BPP=12  NV12 with UV reversed */
#define FB_FMT_IMC1           54    /* BPP=12  YV12 except UV planes same
                                             *         stride as Y */
#define FB_FMT_IMC2           55    /* BPP=12  IMC1 except UV lines
                                             *         interleaved at half stride
                                             *         boundaries */
#define FB_FMT_IMC3           56    /* BPP=12  As IMC1 except that UV
                                             *         swapped */
#define FB_FMT_IMC4           57    /* BPP=12  As IMC2  except that UV
                                             *         swapped */
#define FB_FMT_CLPL           58    /* BPP=12  YV12 but including a level
                                             *         of indirection. */
#define FB_FMT_Y41B           59    /* BPP=12?  4:1:1 planar. */
#define FB_FMT_Y42B           60    /* BPP=16?  YUV 4:2:2 planar. */
#define FB_FMT_CXY1           61    /* BPP=12 */
#define FB_FMT_CXY2           62    /* BPP=16 */

#define FB_FMT_BGR565         63    /* BPP=16 BGR565 */
#define FB_FMT_BGR888         64    /* BPP=32 BGR888 */
#define FB_FMT_BGRA32         65    /* BPP=32 RBGA888 */
#define FB_FMT_ABGR32         66    /* BPP=32 ABGR888 */
#define FB_ISYUVPLANAR(f)     (((f) >= FB_FMT_AYUV) && (f) <= FB_FMT_YUVP)
#define FB_ISYUV(f)           (FB_ISYUVPACKED(f) || FB_ISYUVPLANAR(f))

/* Hardware cursor control */

#ifdef CONFIG_FB_HWCURSOR
#define FB_CUR_ENABLE       0x01    /* Enable the cursor */
#define FB_CUR_SETIMAGE     0x02    /* Set the cursor image */
#define FB_CUR_SETPOSITION  0x04    /* Set the position of the cursor */
#define FB_CUR_SETSIZE      0x08    /* Set the size of the cursor */
#define FB_CUR_XOR          0x10    /* Use XOR vs COPY ROP on image */
#endif

/* Hardware overlay acceleration */

#ifdef CONFIG_FB_OVERLAY
#define FB_ACCL_TRANSP      0x01    /* Hardware tranparency support */
#define FB_ACCL_CHROMA      0x02    /* Hardware chromakey support */
#define FB_ACCL_COLOR       0x04    /* Hardware color support */
#define FB_ACCL_AREA        0x08    /* Hardware support area selection */

#ifdef CONFIG_FB_OVERLAY_BLIT
#define FB_ACCL_BLIT        0x10    /* Hardware blit support */
#define FB_ACCL_BLEND       0x20    /* Hardware blend support */
#endif

/* Overlay transparency mode */

#define FB_CONST_ALPHA      0x00    /* Transparency by alpha value */
#define FB_PIXEL_ALPHA      0x01    /* Transparency by pixel alpha value */

#endif /* CONFIG_FB_OVERLAY */

/* FB character driver IOCTL commands */

/* ioctls */
#define FBIOGET_VIDEOINFO     _FBIOC(0x0001)  /* Get color plane info */
                                              /* Argument: writable struct
                                               *           fb_videoinfo_s */
#define FBIOGET_PLANEINFO     _FBIOC(0x0002)  /* Get video plane info */
                                              /* Argument: writable struct
                                               *           fb_planeinfo_s */

#ifdef CONFIG_FB_CMAP
#define FBIOGET_CMAP          _FBIOC(0x0003)  /* Get RGB color mapping */
                                              /* Argument: writable struct
                                               *           fb_cmap_s */
#define FBIOPUT_CMAP          _FBIOC(0x0004)  /* Put RGB color mapping */
                                              /* Argument: read-only struct
                                               *           fb_cmap_s */
#endif

#ifdef CONFIG_FB_HWCURSOR
#define FBIOGET_CURSOR        _FBIOC(0x0005)  /* Get cursor attributes */
                                              /* Argument: writable struct
                                               *           fb_cursorattrib_s */
#define FBIOPUT_CURSOR        _FBIOC(0x0006)  /* Set cursor attributes */
                                              /* Argument: read-only struct
                                               *           fb_setcursor_s */
#endif

#ifdef CONFIG_FB_UPDATE
#define FBIO_UPDATE           _FBIOC(0x0007)  /* Update a rectangular region in
                                               * the framebuffer
                                               * Argument: read-only struct
                                               *           fb_area_s */
#endif

#ifdef CONFIG_FB_SYNC
#define FBIO_WAITFORVSYNC     _FBIOC(0x0008)  /* Wait for vertical sync */
#endif

#ifdef CONFIG_FB_OVERLAY
#define FBIOGET_OVERLAYINFO   _FBIOC(0x0009)  /* Get video overlay info */
                                              /* Argument: writable struct
                                               *           fb_overlayinfo_s */
#define FBIO_SELECT_OVERLAY   _FBIOC(0x000a)  /* Select overlay */
                                              /* Argument: read-only
                                               *           unsigned long */
#define FBIOSET_TRANSP        _FBIOC(0x000b)  /* Set opacity or transparency
                                               * Argument: read-only struct
                                               *           fb_overlayinfo_s */
#define FBIOSET_CHROMAKEY     _FBIOC(0x000c)  /* Set chroma key
                                               * Argument: read-only struct
                                               *           fb_overlayinfo_s */
#define FBIOSET_COLOR         _FBIOC(0x000d)  /* Set color
                                               * Aŕgument: read-only struct
                                               *           fb_overlayinfo_s */
#define FBIOSET_BLANK         _FBIOC(0x000e)  /* Blank or unblank
                                               * Argument: read-only struct
                                               *           fb_overlayinfo_s */
#define FBIOSET_AREA          _FBIOC(0x000f)  /* Set active overlay area
                                               * Argument: read-only struct
                                               *           fb_overlayinfo_s */
#define FBIOSET_DESTAREA      _FBIOC(0x0010)  /* Set destination area on
                                               * primary FB.
                                               * Argument: read-only struct
                                               *           fb_overlayinfo_s */

#ifdef CONFIG_FB_OVERLAY_BLIT
#define FBIOSET_BLIT          _FBIOC(0x0011)  /* Blit area between overlays
                                               * Argument: read-only struct
                                               *           fb_overlayblit_s */
#define FBIOSET_BLEND         _FBIOC(0x0012)  /* Blend area between overlays
                                               * Argument: read-only struct
                                               *           fb_overlayblend_s */
#endif

#define FBIOPAN_OVERLAY       _FBIOC(0x0013)  /* Pan display for overlay
                                               * Argument: read-only struct
                                               *           fb_overlayinfo_s */

#endif /* CONFIG_FB_OVERLAY */

/* Specific Controls ********************************************************/

#define FBIOSET_POWER         _FBIOC(0x0014)  /* Set panel power
                                               * Argument:             int */
#define FBIOGET_POWER         _FBIOC(0x0015)  /* Get panel current power
                                               * Argument:            int* */
#define FBIOSET_FRAMERATE     _FBIOC(0x0016)  /* Set frame rate
                                               * Argument:             int */
#define FBIOGET_FRAMERATE     _FBIOC(0x0017)  /* Get frame rate
                                               * Argument:            int* */

#define FBIOPAN_DISPLAY       _FBIOC(0x0018)  /* Pan display
                                               * Argument: read-only struct
                                               *           fb_planeinfo_s* */

#define FBIOPAN_CLEAR         _FBIOC(0x0019)  /* Pan clear */
                                              /* Argument: read-only
                                               *           unsigned long */

#define FBIOSET_VSYNCOFFSET   _FBIOC(0x001a)  /* Set VSync offset in usec
                                               * Argument:             int */

/* Linux Support ************************************************************/

#define FBIOGET_VSCREENINFO   _FBIOC(0x001b)  /* Get video variable info */
                                              /* Argument: writable struct
                                               *           fb_var_screeninfo */
#define FBIOGET_FSCREENINFO   _FBIOC(0x001c)  /* Get video fix info */
                                              /* Argument: writable struct
                                               *           fb_fix_screeninfo */
#define FB_NO_OVERLAY -1

/**
 * If any dimension of the display exceeds 65,536 pixels, then the following
 * type will need to change:
 */
typedef uint16_t fb_coord_t;
typedef uint32_t __u32;
typedef uint16_t __u16;

struct fb_var_screeninfo {
    __u32 xres;        /* visible resolution           */
    __u32 yres;
    __u32 xres_virtual;    /* virtual resolution           */
    __u32 yres_virtual;
    __u32 xoffset;        /* offset from virtual to visible */
    __u32 yoffset;        /* resolution                   */

    __u32 bits_per_pixel;    /* guess what                   */
    __u32 grayscale;    /* 0 = color, 1 = grayscale,    */
    /* >1 = FOURCC                  */

    __u32 nonstd;        /* != 0 Non standard pixel format */

    __u32 activate;        /* see FB_ACTIVATE_*            */

    __u32 height;        /* height of picture in mm    */
    __u32 width;        /* width of picture in mm     */

    __u32 accel_flags;    /* (OBSOLETE) see fb_info.flags */

    /* Timing: All values in pixclocks, except pixclock (of course) */
    __u32 pixclock;        /* pixel clock in ps (pico seconds) */
    __u32 left_margin;    /* time from sync to picture    */
    __u32 right_margin;    /* time from picture to sync    */
    __u32 upper_margin;    /* time from sync to picture    */
    __u32 lower_margin;
    __u32 hsync_len;    /* length of horizontal sync    */
    __u32 vsync_len;    /* length of vertical sync      */
    __u32 sync;        /* see FB_SYNC_*                */
    __u32 vmode;        /* see FB_VMODE_*               */
    __u32 rotate;        /* angle we rotate counter clockwise */
    __u32 colorspace;    /* colorspace for FOURCC-based modes */
    __u32 reserved[4];    /* Reserved for future compatibility */
};

struct fb_fix_screeninfo {
    char id[16];        /* identification string eg "TT Builtin" */
    unsigned long smem_start;    /* Start of frame buffer mem */
    /* (physical address) */
    __u32 smem_len;        /* Length of frame buffer mem */
    __u32 type;        /* see FB_TYPE_*                */
    __u32 type_aux;        /* Interleave for interleaved Planes */
    __u32 visual;        /* see FB_VISUAL_*              */
    __u16 xpanstep;        /* zero if no hardware panning  */
    __u16 ypanstep;        /* zero if no hardware panning  */
    __u16 ywrapstep;    /* zero if no hardware ywrap    */
    __u32 line_length;    /* length of a line in bytes    */
    unsigned long mmio_start;    /* Start of Memory Mapped I/O   */
    /* (physical address) */
    __u32 mmio_len;        /* Length of Memory Mapped I/O  */
    __u32 accel;        /* Indicate to driver which     */
    /*  specific chip/card we have  */
    __u16 capabilities;    /* see FB_CAP_*                 */
    __u16 reserved[2];    /* Reserved for future compatibility */
};

/* This structure describes the overall video controller */

struct fb_videoinfo_s {
    uint8_t fmt;        /* see FB_FMT_*  */
    fb_coord_t xres;    /* Horizontal resolution in pixel columns */
    fb_coord_t yres;    /* Vertical resolution in pixel rows */
    uint8_t nplanes;    /* Number of color planes supported */
#ifdef CONFIG_FB_OVERLAY
    uint8_t noverlays;    /* Number of overlays supported */
#endif
#ifdef CONFIG_FB_MODULEINFO
    uint8_t moduleinfo[128];   /* Module information filled by vendor */
#endif
};

/**
 * This structure describes one color plane.  Some YUV formats may support
 * up to 4 planes
 */

struct fb_planeinfo_s {
    FAR void *fbmem;    /* Start of frame buffer memory */
    size_t fblen;        /* Length of frame buffer memory in bytes */
    fb_coord_t stride;    /* Length of a line in bytes */
    uint8_t display;    /* Display number */
    uint8_t bpp;        /* Bits per pixel */
    uint32_t   xres_virtual; /* Virtual Horizontal resolution in pixel columns */
    uint32_t   yres_virtual; /* Virtual Vertical resolution in pixel rows */
    uint32_t   xoffset;      /* Offset from virtual to visible resolution */
    uint32_t   yoffset;      /* Offset from virtual to visible resolution */
};

#ifdef CONFIG_FB_OVERLAY
/* This structure describes the transparency. */

struct fb_transp_s {
    uint8_t transp;        /* Transparency */
    uint8_t transp_mode;    /* Transparency mode */
};

/* This structure describes an area. */
struct fb_area_s {
    fb_coord_t x;        /* x-offset of the area */
    fb_coord_t y;        /* y-offset of the area */
    fb_coord_t w;        /* Width of the area */
    fb_coord_t h;        /* Height of the area */
};

/* This structure describes one overlay. */

struct fb_overlayinfo_s {
    FAR void   *fbmem;          /* Start of frame buffer memory */
    size_t     fblen;           /* Length of frame buffer memory in bytes */
    fb_coord_t stride;          /* Length of a line in bytes */
    uint8_t    overlay;         /* Overlay number */
    uint8_t    bpp;             /* Bits per pixel */
    uint32_t   xres;            /* Horizontal resolution in pixel columns */
    uint32_t   yres;            /* Vertical resolution in pixel rows */
    uint32_t   xres_virtual;    /* Virtual Horizontal resolution in pixel columns */
    uint32_t   yres_virtual;    /* Virtual Vertical resolution in pixel rows */
    uint32_t   xoffset;         /* Offset from virtual to visible resolution */
    uint32_t   yoffset;         /* Offset from virtual to visible resolution */
    uint8_t    blank;           /* Blank or unblank */
    uint32_t   chromakey;       /* Chroma key argb8888 formatted */
    uint32_t   color;           /* Color argb8888 formatted */
    struct fb_transp_s transp;  /* Transparency */
    struct fb_area_s sarea;     /* Selected area within the overlay */
    struct fb_area_s darea;     /* Destination area on the primary FB */
    uint32_t   accl;            /* Supported hardware acceleration */
};

#ifdef CONFIG_FB_OVERLAY_BLIT
/* This structure describes an overlay area within a whole overlay */

struct fb_overlayarea_s {
    uint8_t overlay;    /* Number overlay */
    struct fb_area_s area;    /* Overlay area */
};

/* This structure describes blit operation */

struct fb_overlayblit_s {
    struct fb_overlayarea_s dest;    /* The destination overlay area */
    struct fb_overlayarea_s src;    /* The source overlay area */
};

/* This structure describes blend operation */

struct fb_overlayblend_s {
    struct fb_overlayarea_s dest;    /* The destination overlay area */
    struct fb_overlayarea_s foreground;    /* The foreground overlay area */
    struct fb_overlayarea_s background;    /* The background overlay area */
};
#endif
#endif /* CONFIG_FB_OVERLAY */

/**
 * On video controllers that support mapping of a pixel palette value
 * to an RGB encoding, the following structure may be used to define
 * that mapping.
 */

#ifdef CONFIG_FB_CMAP
struct fb_cmap_s {
    uint16_t first;        /* Offset offset first color entry in tables */
    uint16_t len;        /* Number of color entries  in tables */

    /* Tables of  color component.  Any may be NULL if not used */

    uint8_t *red;        /* Table of 8-bit red values */
    uint8_t *green;        /* Table of 8-bit green values */
    uint8_t *blue;        /* Table of 8-bit blue values */
#ifdef CONFIG_FB_TRANSPARENCY
    uint8_t *transp;    /* Table of 8-bit transparency */
#endif
};
#endif

#ifdef CONFIG_FB_HWCURSOR
#ifdef CONFIG_FB_HWCURSORIMAGE
/**
 * If the video controller hardware supports a hardware cursor and
 * that hardware cursor supports user-provided images, then the
 * following structure may be used to provide the cursor image
 */

struct fb_cursorimage_s {
    fb_coord_t width;    /* Width of the cursor image in pixels */
    fb_coord_t height    /* Height of the cursor image in pixels */
    const uint8_t *image;    /* Pointer to image data */
};
#endif

/* The following structure defines the cursor position/size */

struct fb_cursorpos_s {
    fb_coord_t x;        /* X position in pixels */
    fb_coord_t y;        /* Y position in rows */
};

/**
 * If the hardware supports setting the cursor size, then this structure
 * is used to provide the size.
 */

#ifdef CONFIG_FB_HWCURSORSIZE
struct fb_cursorsize_s {
    fb_coord_t h;        /* Height in rows */
    fb_coord_t w;        /* Width in pixels */
};
#endif

/* The following are used to get/get the cursor attributes via IOCTL command. */

struct fb_cursorattrib_s {
#ifdef CONFIG_FB_HWCURSORIMAGE
    uint8_t fmt;        /* Video format of cursor */
#endif
    struct fb_cursorpos_s pos;    /* Current cursor position */
#ifdef CONFIG_FB_HWCURSORSIZE
    struct fb_cursorsize_s mxsize;    /* Maximum cursor size */
    struct fb_cursorsize_s size;    /* Current size */
#endif
};

struct fb_setcursor_s {
    uint8_t flags;        /* See FB_CUR_* definitions */
    struct fb_cursorpos_s pos;    /* Cursor position */
#ifdef CONFIG_FB_HWCURSORSIZE
    struct fb_cursorsize_s size;    /* Cursor size */
#endif
#ifdef CONFIG_FB_HWCURSORIMAGE
    struct fb_cursorimage_s img;    /* Cursor image */
#endif
};
#endif

/**
 * The framebuffer "object" is accessed through within the OS via
 * the following vtable:
 */

struct fb_vtable_s {
    /* Get information about the video controller configuration and the
     * configuration of each color plane.
     */

    int (*getvideoinfo)(FAR struct fb_vtable_s *vtable,
                        FAR struct fb_videoinfo_s *vinfo);
    int (*getplaneinfo)(FAR struct fb_vtable_s *vtable, int planeno,
                        FAR struct fb_planeinfo_s *pinfo);

    /* open/close window. */

    int (*open)(FAR struct fb_vtable_s *vtable);
    int (*close)(FAR struct fb_vtable_s *vtable);

#ifdef CONFIG_FB_CMAP
    /* The following are provided only if the video hardware supports RGB
     * color mapping
     */

    int (*getcmap)(FAR struct fb_vtable_s *vtable,
                   FAR struct fb_cmap_s *cmap);
    int (*putcmap)(FAR struct fb_vtable_s *vtable,
                   FAR const struct fb_cmap_s *cmap);
#endif

#ifdef CONFIG_FB_HWCURSOR
    /* The following are provided only if the video hardware supports a
     * hardware cursor.
     */

    int (*getcursor)(FAR struct fb_vtable_s *vtable,
                     FAR struct fb_cursorattrib_s *attrib);
    int (*setcursor)(FAR struct fb_vtable_s *vtable,
                     FAR struct fb_setcursor_s *settings);
#endif

#ifdef CONFIG_FB_UPDATE
    /* The following are provided only if the video hardware need extera
     * notification to update display content.
     */

    int (*updatearea)(FAR struct fb_vtable_s *vtable,
                      FAR const struct fb_area_s *area);
#endif

#ifdef CONFIG_FB_SYNC
    /* The following are provided only if the video hardware signals
     * vertical sync.
     */

    int (*waitforvsync)(FAR struct fb_vtable_s *vtable);
#endif

#ifdef CONFIG_FB_OVERLAY
    /* Get information about the video controller configuration and the
     * configuration of each overlay.
     */

    int (*getoverlayinfo)(FAR struct fb_vtable_s *vtable, int overlayno,
                          FAR struct fb_overlayinfo_s *oinfo);

    /* The following are provided only if the video hardware supports
     * transparency
     */

    int (*settransp)(FAR struct fb_vtable_s *vtable,
                     FAR const struct fb_overlayinfo_s *oinfo);

    /* The following are provided only if the video hardware supports
     * chromakey
     */

    int (*setchromakey)(FAR struct fb_vtable_s *vtable,
                        FAR const struct fb_overlayinfo_s *oinfo);

    /* The following are provided only if the video hardware supports
     * filling the overlay with a color.
     */

    int (*setcolor)(FAR struct fb_vtable_s *vtable,
                    FAR const struct fb_overlayinfo_s *oinfo);

    /* The following allows to switch the overlay on or off */

    int (*setblank)(FAR struct fb_vtable_s *vtable,
                    FAR const struct fb_overlayinfo_s *oinfo);

    /* The following allows to set the active area for subsequently overlay
     * operations.
     */

    int (*setarea)(FAR struct fb_vtable_s *vtable,
                   FAR const struct fb_overlayinfo_s *oinfo);

    /* The following allows to set the display area for subsequently overlay
     * operations.
     */

    int (*setdestarea)(FAR struct fb_vtable_s *vtable,
                       FAR const struct fb_overlayinfo_s *oinfo);

#ifdef CONFIG_FB_OVERLAY_BLIT
    /* The following are provided only if the video hardware supports
     * blit operation between overlays.
     */

    int (*blit)(FAR struct fb_vtable_s *vtable,
                FAR const struct fb_overlayblit_s *blit);

    /* The following are provided only if the video hardware supports
     * blend operation between overlays.
     */

    int (*blend)(FAR struct fb_vtable_s *vtable,
                 FAR const struct fb_overlayblend_s *blend);
#endif

    /* The following allows to pan display for multiple buffers.
     */

    int (*panoverlay)(FAR struct fb_vtable_s *vtable,
                      FAR const struct fb_overlayinfo_s *oinfo);

#endif

    /* Pan display for multiple buffers. */

    int (*pandisplay)(FAR struct fb_vtable_s *vtable,
                      FAR struct fb_planeinfo_s *pinfo);

    /* Specific Controls ******************************************************/

    /* Set the frequency of the framebuffer update panel (0: disable refresh) */

    int (*setframerate)(FAR struct fb_vtable_s *vtable, int rate);

    /* Get the frequency of the framebuffer update panel (0: disable refresh) */

    int (*getframerate)(FAR struct fb_vtable_s *vtable);

    /* Get the panel power status (0: full off). */

    int (*getpower)(FAR struct fb_vtable_s *vtable);

    /* Enable/disable panel power (0: full off). */

    int (*setpower)(FAR struct fb_vtable_s *vtable, int power);

    /* Passthrough the unknown ioctl commands. */

    int (*ioctl)(FAR struct fb_vtable_s *vtable, int cmd, unsigned long arg);

    /* Pointer to framebuffer device private data. */

    FAR void *priv;
};

/* This structure describes one color plane.  Some YUV formats may support
 * up to 4 planes (although they probably wouldn't be used on LCD hardware).
 * The framebuffer driver provides the video memory address in its
 * corresponding fb_planeinfo_s structure.  The LCD driver, instead, provides
 * methods to transfer data to/from the LCD color plane.
 */

struct lcd_planeinfo_s
{
  /* LCD Data Transfer ******************************************************/

  /* This method can be used to write a partial raster line to the LCD:
   *
   *  row     - Starting row to write to (range: 0 <= row < yres)
   *  col     - Starting column to write to (range: 0 <= col <= xres-npixels)
   *  buffer  - The buffer containing the run to be written to the LCD
   *  npixels - The number of pixels to write to the LCD
   *            (range: 0 < npixels <= xres-col)
   */

  int (*putrun)(fb_coord_t row, fb_coord_t col, FAR const uint8_t *buffer,
                size_t npixels);

  /* This method can be used to write a rectangular area to the LCD:
   *
   *  row_start - Starting row to write to (range: 0 <= row < yres)
   *  row_end   - Ending row to write to (range: row_start <= row < yres)
   *  col_start - Starting column to write to (range: 0 <= col <= xres)
   *  col_end   - Ending column to write to
   *              (range: col_start <= col_end < xres)
   *  buffer    - The buffer containing the area to be written to the LCD
   *
   * NOTE: this operation may not be supported by the device, in which case
   * the callback pointer will be NULL. In that case, putrun() should be
   * used.
   */

  int (*putarea)(fb_coord_t row_start, fb_coord_t row_end,
                 fb_coord_t col_start, fb_coord_t col_end,
                 FAR const uint8_t *buffer);

  /* This method can be used to read a partial raster line from the LCD:
   *
   *  row     - Starting row to read from (range: 0 <= row < yres)
   *  col     - Starting column to read read
   *            (range: 0 <= col <= xres-npixels)
   *  buffer  - The buffer in which to return the run read from the LCD
   *  npixels - The number of pixels to read from the LCD
   *            (range: 0 < npixels <= xres-col)
   */

  int (*getrun)(fb_coord_t row, fb_coord_t col, FAR uint8_t *buffer,
                size_t npixels);

  /* This method can be used to read a rectangular area from the LCD:
   *
   *  row_start - Starting row to read from (range: 0 <= row < yres)
   *  row_end   - Ending row to read from (range: row_start <= row < yres)
   *  col_start - Starting column to read from (range: 0 <= col <= xres)
   *  col_end   - Ending column to read from
   *              (range: col_start <= col_end < xres)
   *  buffer    - The buffer where the data will be written
   *
   * NOTE: this operation may not be supported by the device, in which case
   * the callback pointer will be NULL. In that case, getrun() should be
   * used.
   */

  int (*getarea)(fb_coord_t row_start, fb_coord_t row_end,
                 fb_coord_t col_start, fb_coord_t col_end,
                 FAR uint8_t *buffer);

  /* Plane color characteristics ********************************************/

  /* This is working memory allocated by the LCD driver for each LCD device
   * and for each color plane.  This memory will hold one raster line of
   * data. The size of the allocated run buffer must therefore be at least
   * (bpp * xres / 8).  Actual alignment of the buffer must conform to the
   * bitwidth of the underlying pixel type.
   *
   * If there are multiple planes, they may share the same working buffer
   * because different planes will not be operate on concurrently.  However,
   * if there are multiple LCD devices, they must each have unique run
   * buffers.
   */

  uint8_t *buffer;

  /* This is the number of bits in one pixel.  This may be one of {1, 2, 4,
   * 8, 16, 24, or 32} unless support for one or more of those resolutions
   * has been disabled.
   */

  uint8_t  bpp;
};

struct lcd_dev_s
{
  /* LCD Configuration ******************************************************/

  /* Get information about the LCD video controller configuration and the
   * configuration of each LCD color plane.
   */

  int (*getvideoinfo)(FAR struct lcd_dev_s *dev,
         FAR struct fb_videoinfo_s *vinfo);
  int (*getplaneinfo)(FAR struct lcd_dev_s *dev, unsigned int planeno,
         FAR struct lcd_planeinfo_s *pinfo);

  /* LCD RGB Mapping ********************************************************/

  /* The following are provided only if the video hardware supports RGB color
   * mapping
   */

#ifdef CONFIG_FB_CMAP
  int (*getcmap)(FAR struct lcd_dev_s *dev, FAR struct fb_cmap_s *cmap);
  int (*putcmap)(FAR struct lcd_dev_s *dev,
         FAR const struct fb_cmap_s *cmap);
#endif

  /* Cursor Controls ********************************************************/

  /* The following are provided only if the video hardware supports a
   * hardware cursor
   */

#ifdef CONFIG_FB_HWCURSOR
  int (*getcursor)(FAR struct lcd_dev_s *dev,
        FAR struct fb_cursorattrib_s *attrib);
  int (*setcursor)(FAR struct lcd_dev_s *dev,
        FAR struct fb_setcursor_s *settings);
#endif

  /* LCD Specific Controls **************************************************/

  /* Get the LCD panel power status (0: full off - CONFIG_LCD_MAXPOWER: full
   * on).  On backlit LCDs, this setting may correspond to the backlight
   * setting.
   */

  int (*getpower)(struct lcd_dev_s *dev);

  /* Enable/disable LCD panel power (0: full off - CONFIG_LCD_MAXPOWER: full
   * on).  On backlit LCDs, this setting may correspond to the backlight
   * setting.
   */

  int (*setpower)(struct lcd_dev_s *dev, int power);

  /* Get the current contrast setting (0-CONFIG_LCD_MAXCONTRAST) */

  int (*getcontrast)(struct lcd_dev_s *dev);

  /* Set LCD panel contrast (0-CONFIG_LCD_MAXCONTRAST) */

  int (*setcontrast)(struct lcd_dev_s *dev, unsigned int contrast);

  /* Set LCD panel frame rate (0: disable refresh) */

  int (*setframerate)(struct lcd_dev_s *dev, int rate);

  /* Get LCD panel frame rate (0: disable refresh) */

  int (*getframerate)(struct lcd_dev_s *dev);

  int (*pannel_ioctl)(struct lcd_dev_s *dev, enum PANNEL_IOCTL_CMD cmd);
};

/**
 * Public Data
 */

#ifdef __cplusplus
extern "C" {
#endif

/**
 * Public Function Prototypes
 */
/****************************************************************************
 * Name: poll
 *
 * Description:
 *   poll() waits for available chance to register framebuffer to driver
 *
 ****************************************************************************/

    void poll(void);

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

    int fb_remove_paninfo(FAR struct fb_vtable_s *vtable, int overlay);

/**
 * up_fbinitialize - Initialize the framebuffer video hardware associated with the display.
 * @display : In the case of hardware with multiple displays, this
 *             specifies the display.  Normally this is zero.
 *
 *    There are multiple logic paths that may call up_fbinitialize() so any
 *   implementation of up_fbinitialize() should be tolerant of being called
 *   multiple times.
 *
 * Returns: Zero is returned on success; a negated errno value is returned on any
 *   failure.
 *
 */

    int up_fbinitialize(int display);

/**
 * up_fbgetvplane -  Return a a reference to the framebuffer object for the specified video
 *   plane of the specified plane.  Many OSDs support multiple planes of video.
 * @display : In the case of hardware with multiple displays, this
 *             specifies the display.  Normally this is zero.
 * @vplane  : Identifies the plane being queried.
 *
 * Returns: A non-NULL pointer to the frame buffer access structure is returned on
 *   success; NULL is returned on any failure.
 *
 */

    FAR struct fb_vtable_s *up_fbgetvplane(int display, int vplane);

/**
 * up_fbuninitialize - Uninitialize the framebuffer support for the specified display.
 * @display : In the case of hardware with multiple displays, this
 *             specifies the display.  Normally this is zero.
 *
 * Returns:   None
 *
 */

    void up_fbuninitialize(int display);

/**
 * fb_dev_register - Register the framebuffer character device at /dev/fbN where N is the
 *   display number if the devices supports only a single plane.  If the
 *   hardware supports multiple color planes, then the device will be
 *   registered at /dev/fbN.M where N is the again display number but M
 *   is the display plane.
 * @display : The display number for the case of boards supporting multiple
 *             displays or for hardware that supports multiple
 *             layers (each layer is consider a display).  Typically zero.
 * @plane   : Identifies the color plane on hardware that supports separate
 *             framebuffer "planes" for each color component.
 *
 * Returns: Zero (OK) is returned success; a negated errno value is returned on any
 *   failure.
 *
 ****************************************************************************/

    int fb_dev_register(int display, int plane);

/**
 * fb_open - Initialize the default framebuffer character device.
 * Returns: the framebuffer device fd number;
 *
 */
    int fb_open(void);

/**
 * fb_close - Uninitialize the default framebuffer character device.
 *
 */
    void fb_close(int display);

/**
 * fb_ioctl - The standard ioctl method.
 *
 */

    int fb_ioctl(uint32_t fd, int cmd, unsigned long arg);

/**
 * get_fb_base - Get the fb buffer base address.
 *
 */
    uint32_t get_fb_base(void);

/**
 * fb_lcdc_vmirro_enable - enable lcdc vmirro mode.
 *
 */
   int fb_lcdc_vmirro_enable(bool vmirro);

   void fb_pollnotify(FAR struct fb_vtable_s *vtable);

#ifdef __cplusplus
}
#endif

#endif
#endif                /* __FB_DRV_H__ */
