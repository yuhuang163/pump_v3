/***************************************************************************
 *
 * Copyright 2022-2023 BES.
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

#include "../inc/png_dec.h"
#include "cmsis_nvic.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_sysfreq.h"
#include "hal_psram.h"
#include "hal_cache.h"
#include "stdio.h"
#include "string.h"
#include "hal_location.h"
#include "gra_wakelock.h"
#include "check_raw_image.h"
#include "lodepng.h"
#include "heap_api.h"
#include "dma2d.h"
#include "hal_dma2d.h"

#define PNGDEC_DRV_WITH_CACHEOPS

#define  GRAPHIC_DRV_DEBUG
#ifdef  GRAPHIC_DRV_DEBUG
#define PNGDEC_INFO(...) TR_INFO(TR_MOD(TEST), "//" __VA_ARGS__)
#define PNGDEC_REG(...) TR_INFO(TR_MOD(TEST), __VA_ARGS__)

#else
#define PNGDEC_INFO(...)
#define PNGDEC_REG(...)

#endif

#define PNGDEC_REP(...)  TR_INFO(TR_MOD(TEST), "//" __VA_ARGS__)
#define PNGDEC_ERR(...)  TR_ERROR(TR_MOD(TEST), "//" __VA_ARGS__)

#ifdef GRAPHICDRV_TIME_STATIS

#define _TEST_REP(...)  TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)

#define TC_INIT  volatile uint32_t time_s; \
                 volatile uint32_t time_e;
#define TC_START time_s = hal_sys_timer_get();
#define TC_END   time_e = hal_sys_timer_get();

#define _TSTR(s) #s

#define TC_REP(s) do{ \
                      _TEST_REP(_TSTR(s)":%d us", TICKS_TO_US(time_e - time_s)); \
                    }while(0)

#else

#define TC_INIT
#define TC_START
#define TC_END
#define TC_REP(s)

#endif


#if 0
static uint8_t pngpixes[500*500*4]  ALIGNED(8) PSRAM_NC_BSS_LOC;
static uint8_t pngpixes_decode[500*500*4]  ALIGNED(8) PSRAM_NC_BSS_LOC;
#else
//static uint8_t *pngpixes = (uint8_t *)(SYS_PSRAM0_BASE + 0x200008);
//static uint8_t *pngpixes_decode = (uint8_t *)(SYS_PSRAM0_BASE + 0x400008);

static uint8_t *pngpixes = (uint8_t *)(PSRAM_NC_BASE + 0x200008);
static uint8_t *pngpixes_decode = (uint8_t *)(PSRAM_NC_BASE + 0x400008);
static uint8_t *bl_fb = (uint8_t *)(PSRAM_NC_BASE + 0x600008);
static uint8_t *blend_pixes = (uint8_t *)(PSRAM_NC_BASE + 0x700008);
#define const_alpha   0x0f
#define PNG2D_BLEND_TEST
#define CONST_2DBLEND_MODE_TEST
#endif
//static uint8_t pngfile[1024*50]  ALIGNED(8) PSRAM_NC_BSS_LOC;
static struct png_file_data pngfile;
static struct png_pix_buffer dest;


void *pixbufalloc(uint32_t size, uint8_t aligns)
{
     if (size > 512*512*4)
     {
         PNGDEC_INFO("%s %d no png space !!!!!!",__func__,__LINE__);
         return NULL;
     }
     return pngpixes_decode;
}

extern int dma2d_blit_config(FAR struct dma2d_overlay_s *doverlay,
              const struct fb_area_s *dareac,
              FAR struct dma2d_overlay_s *soverlay,
              FAR const struct fb_area_s *sareac);

extern int dma2d_blend_config(FAR struct dma2d_overlay_s *doverlay,
               const struct fb_area_s *dareac,
               FAR struct dma2d_overlay_s *foverlay,
               const struct fb_area_s *fareac,
               FAR struct dma2d_overlay_s *boverlay,
               FAR const struct fb_area_s *bareac);

extern int dma2d_start(void);

static void  fb_overlayinfo_setup(struct fb_overlayinfo_s* poinfo,
                                  uint16_t width,
                                  uint16_t height,
                                  uint8_t bpp,
                                  uint32_t *  pbuffer)
{
    poinfo->fbmem = pbuffer;
    poinfo->fblen = width * height * bpp/8;
    poinfo->stride = width * bpp /8;
    poinfo->bpp = bpp;
}


static void dma2d_overlayinfo_setup(struct dma2d_overlay_s * poverlay,
                                    uint8_t cfmt,
                                    struct fb_overlayinfo_s* pfboverlay )
{
    poverlay->oinfo = pfboverlay;
    poverlay->fmt = cfmt;
    poverlay->xres = pfboverlay->stride * 8 / pfboverlay->bpp;
    poverlay->yres = pfboverlay->fblen / pfboverlay->stride;
}

static void fb_area_setup(struct fb_area_s *darea,
                          uint16_t offset_x,
                          uint16_t offset_y,
                          uint16_t width,
                          uint16_t height)
{
    darea->x = offset_x;
    darea->y = offset_y;
    darea->w = width;
    darea->h = height;
}

static void dma2d_blend_test(void *obuffer)
{
    struct fb_area_s barea;
    struct fb_area_s farea;
    struct fb_area_s darea;
    struct dma2d_overlay_s boverlay;
    struct dma2d_overlay_s foverlay;
    struct dma2d_overlay_s doverlay;
    struct fb_overlayinfo_s boinfo;
    struct fb_overlayinfo_s foinfo;
    struct fb_overlayinfo_s doinfo;
    uint32_t * bl_mem;

    TRACE(3,"%s \r\n", __func__);
    dma2dinitialize();

    bl_mem = (uint32_t *)bl_fb;
    //set color to bl frame buffer
    for ( int i = 0; i < 480*480; i++ ) {
        bl_mem[i] = 0x1f00ff00; //rgba
    }

    fb_overlayinfo_setup(&boinfo, 480, 480, 32, bl_mem);
    fb_overlayinfo_setup(&foinfo, 480, 480, 32, 0);
    fb_overlayinfo_setup(&doinfo, 480, 480, 32, obuffer);

    dma2d_overlayinfo_setup(&boverlay, FB_FMT_RGB32, &boinfo);
    dma2d_overlayinfo_setup(&foverlay, FB_FMT_RGB32, &foinfo);
    dma2d_overlayinfo_setup(&doverlay, FB_FMT_RGB32, &doinfo);

    #if 1 //test const alpha blend mode
    boverlay.transp_mode = 0;
    if (boverlay.transp_mode == 0) { // const alpha
        boverlay.oinfo->transp.transp = const_alpha;
    }
    #else
    boverlay.transp_mode = 1;
    #endif

    fb_area_setup(&barea, 0,0,480,480);
    fb_area_setup(&farea, 0,0,480,480);
    fb_area_setup(&darea, 0,0,480,480);

    TRACE(3,"png->dma2d blend fl%d+bl%d->ol%d dma2d out:%p",0,1,2,obuffer);
    dma2d_blend_config(&doverlay,&darea,&foverlay,&farea,&boverlay,&barea);
}




static void dma2d_blit_test(void *obuffer)
{
    struct dma2d_overlay_s doverlay;
    struct fb_area_s darea;
    struct dma2d_overlay_s soverlay;
    struct fb_area_s sarea;
    struct fb_overlayinfo_s doinfo;
    struct fb_overlayinfo_s soinfo;

    TRACE(3,"%s \r\n", __func__);

    dma2dinitialize();


   // TRACE(3,"dma2d clk : %d Mhz \r\n", 100);
   // hal_cmu_2d_mod_clock_cfg(pixclk);
    //while(1)
    {

    fb_overlayinfo_setup(&doinfo, 480, 480, 32, obuffer);
    fb_overlayinfo_setup(&soinfo, 480, 480, 32, 0);
    dma2d_overlayinfo_setup(&doverlay, FB_FMT_RGB32, &doinfo);
    dma2d_overlayinfo_setup(&soverlay, FB_FMT_RGB32, &soinfo);

    fb_area_setup(&darea, 0,0,480,480);
    fb_area_setup(&sarea, 0,0,480,480);

    TRACE(3,"blit sram->sram  8888->8888   pngdec dest:%p", obuffer);
    dma2d_blit_config(&doverlay, &darea, &soverlay, &sarea);
   }

}

static void *dma2d_setup_wbfb(uint32_t size, uint8_t aligns)
{
     if (size > 500*500*4)
     {
         PNGDEC_INFO("%s %d no png space !!!!!!",__func__,__LINE__);
         return NULL;
     }
     #if 0
         dma2d_blit_test(pngpixes_decode);
     #else
         dma2d_blend_test(pngpixes_decode);
     #endif
     return pngpixes_decode;
}

void trigger_lines_handler(void)
{
    PNGDEC_INFO("%s %d",__func__,__LINE__);
}

//static uint8_t pngdec_base[1024*1024*2]  ALIGNED(32) PSRAM_NC_BSS_LOC;
#if 1
static uint8_t * pngdec_base = (uint8_t *)PSRAM_NC_BASE;
static uint8_t * pngfiledata = (uint8_t *)(PSRAM_NC_BASE + 0x800000);
//static uint8_t * pngfiledata = (uint8_t *)(PSRAM_NC_BASE + 0x800000 + 1);
#else
static uint8_t * pngdec_base = (uint8_t *)SYS_PSRAM0_BASE;
static uint8_t * pngfiledata = (uint8_t *)(SYS_PSRAM0_BASE + 0x800000);
#endif
static heap_handle_t pngdec_heap;

POSSIBLY_UNUSED
static heap_handle_t pngdec_mem_setup(uint32_t * base_addr, uint32_t size)
{
#ifndef PNGDEC_SIM
    memset(base_addr, 0, size);
#endif
    return heap_register(base_addr, size);
}

void *lodepng_malloc(uint32_t size)
{
    void * addr;
    //TRACE(0,"%s s:%d ",__func__, size);
    addr = heap_malloc(pngdec_heap, size + 8);
    //TRACE(0,"%s mem:%p ",__func__, addr);
    return addr;
}

void lodepng_free(void *mem)
{
  //TRACE(0,"%s ptr:%p ",__func__, mem);
  if (mem){
      heap_free(pngdec_heap, mem);
  }  else {
   // *(int*)0 = 0xbad;
  }
}

void* lodepng_realloc(void* ptr, size_t new_size)
{
  //TRACE(0,"%s ptr:%p ns:%d",__func__, ptr, new_size);
  uint8_t * buff =  lodepng_malloc(new_size);
  if (ptr) {
     memcpy(buff,ptr,new_size);
     lodepng_free(ptr);
  } else {
    //*(int*)0 = 0xbad;
  }
  return buff;
}

void hal_psram_init(void)
{

}

#ifdef TEST_WITH_FS
static void save_png_file(const char *name, char *buffer, uint32_t size)
{
    FILE * fp;

    fp = fopen(name, "wb");

    if (fp != NULL) {

        // Save buffer info.
        fwrite(buffer, 1, size, fp);

        fclose(fp);
        fp = NULL;
    } else {
       PNGDEC_INFO("%s %d fail to open file",__func__,__LINE__);
    }

}
#endif

static void set_dest_colorfmt(struct png_pix_buffer *dest, enum PNG_PIX_FMT_T fmt  )
{
   dest->fmt = fmt;
}

enum PNG_DEC_PIX_FMT_T colorfmt[] ={
   #if 0
    PNGDEC_PIX_FMT_RGB565,
    PNGDEC_PIX_FMT_ARGB565,
    PNGDEC_PIX_FMT_RGBA565,
    PNGDEC_PIX_FMT_ARGB888,
   #endif
    PNGDEC_PIX_FMT_RGB888,
    PNGDEC_PIX_FMT_RGBA888
};

static char *color_string[] ={
    "PNGDEC_PIX_FMT_RGB565",
    "PNGDEC_PIX_FMT_ARGB565",
    "PNGDEC_PIX_FMT_RGBA565",
    "PNGDEC_PIX_FMT_ARGB888",
    "PNGDEC_PIX_FMT_RGB888",
    "PNGDEC_PIX_FMT_RGBA888"
};

struct image_size {
    uint16_t w;
    uint16_t h;
};

//#define USE_LODEPNG_ENCODE

static struct image_size pngsize[] ={
    {480,480},
#if 0 //def USE_LODEPNG_ENCODE
    {500,500},
    {320, 240},
    {120,1},
    {600,600},
#endif
};

#define COLOR_ABS(x) ((x) > 0 ? (x) : (-(x)))

static bool is_same_color(uint8_t s, uint8_t d)
{
    return (COLOR_ABS((s-d)) < 2);
}

static int check_result(struct png_pix_buffer *buff,uint32_t *src, char *dest)
{
    uint32_t *p0 = src;
    uint32_t *p_bpp32;
    uint8_t  *p_bpp8;
    uint16_t *p_bpp16;
    uint32_t  color = 0;

    if(buff->fmt == PNGDEC_PIX_FMT_RGBA888) {

        for(int i = 0; i < buff->height; i++) {
            p_bpp32 = (uint32_t *)dest + i*buff->pitch/4;
            p0 = (uint32_t *)src + i*buff->width;;

            for(int j = 0; j < buff->width; j++) {

               color = *p_bpp32;

        #ifdef PNG2D_BLEND_TEST
               // without check alpha
               uint8_t *s = p0;
               uint8_t *d = &color;
               color = argb888torgba888(color);

               if (!(is_same_color(s[0], d[0]) && is_same_color(s[1], d[1]) && is_same_color(s[2], d[2]))) {
                    TRACE(0,"%s png decoder err i:%d j:%d [%x|%x]",__func__, i,j, *(p0), color);
                    return -1;
               }

        #else
               if (!buff->globle_alpha) {
                  if (*p0 != color) {
                     TRACE(0,"%s png decoder err i:%d j:%d [%x|%x]",__func__, i,j, *(p0), color);
                     return -1;
                  }
               } else {
                  if ((((*p0) & (~(0xff000000))) | (buff->alpha << 24) ) !=  color) {
                     TRACE(0,"%s png decoder err i:%d j:%d [%x|%x]",__func__, i,j, *(p0), color);
                     return -1;
                  }
               }
        #endif
               p_bpp32 += 1;
               p0 += 1;

            }
        }
    }

    if(buff->fmt == PNGDEC_PIX_FMT_ARGB888) {

        for(int i = 0; i < buff->height; i++) {
            p_bpp32 = (uint32_t *)dest + i*buff->pitch/4;
            p0 = (uint32_t *)src + i*buff->width;;

            for(int j = 0; j < buff->width; j++) {

               color = *p_bpp32;
               color = argb888torgba888(color);

               if (!buff->globle_alpha) {
                  if (*p0 != color) {
                     TRACE(0,"%s png decoder err i:%d[%x|%x]",__func__, i, *(p0), color);
                     return -1;
                  }
               } else {
                  if ((((*p0) & (~(0xff000000))) | (buff->alpha << 24) ) !=  color) {
                     TRACE(0,"%s png decoder err i:%d[%x|%x]",__func__, i, *(p0), color);
                     return -1;
                  }
               }
               p_bpp32 += 1;
               p0 += 1;

            }
        }
    }

    if(buff->fmt == PNGDEC_PIX_FMT_RGB565) {

        for(int i = 0; i < buff->height; i++) {
            p_bpp16 = (uint16_t *)dest + i*buff->pitch/2;
            p0 = (uint32_t *)src + i*buff->width;;

            for(int j = 0; j < buff->width; j++) {
               color = *p0;
               color = rgba888torgb565(color);

               if (!rgb565_equal(*p_bpp16, 0xffff&color)) {
                  return -1;
               }
               p_bpp16 += 1;
               p0 += 1;
            }
        }
    }

    if(buff->fmt == PNGDEC_PIX_FMT_RGB888) {

        for(int i = 0; i < buff->height; i++) {
            p_bpp8 = (uint8_t *)dest + i*buff->pitch;
            p0 = (uint32_t *)src + i*buff->width;;

            for(int j = 0; j < buff->width; j++) {
               color = *p0;
               color = color & 0xffffff;//rgba888torgb565a(color);

               if ((p_bpp8[0]|(p_bpp8[1] << 8)|(p_bpp8[2] << 16)) != color) {
                      TRACE(0,"%s png decoder err i:%d[%x|%x]",__func__, i,
                           (p_bpp8[0]|(p_bpp8[1] << 8)|(p_bpp8[2] << 16)) ,color);
                      return -1;
               }
               p_bpp8 += 3;
               p0 += 1;
            }
        }
    }

    if(buff->fmt == PNGDEC_PIX_FMT_RGBA565) {

        for(int i = 0; i < buff->height; i++) {
            p_bpp8 = (uint8_t *)dest + i*buff->pitch;
            p0 = (uint32_t *)src + i*buff->width;;

            for(int j = 0; j < buff->width; j++) {
               color = *p0;
               color = rgba888torgb565a(color);

               if (!buff->globle_alpha) {
                   if (!rgba565_equal((p_bpp8[0]|(p_bpp8[1] << 8)), 0xffff&color)) {
                      TRACE(0,"rgba565_equal i:%d |%x|%x",i, (0xffffff&(p_bpp8[0]|(p_bpp8[1] << 8)|p_bpp8[2] << 16)), 0xffffff&color);
                      return -1;
                   }
               } else {
                   color &=~(0xff0000);
                   color |= (buff->alpha) << 16;
                   if (!rgba565_equal((p_bpp8[0]|(p_bpp8[1] << 8)|(p_bpp8[2] << 16)), 0xffffff&color)) {
                      return -1;
                   }
               }
               p_bpp8 += 3;
               p0 += 1;

            }
        }
    }

    if(buff->fmt == PNGDEC_PIX_FMT_ARGB565) {

        for(int i = 0; i < buff->height; i++) {
            p_bpp8 = (uint8_t *)dest + i*buff->pitch;
            p0 = (uint32_t *)src + i*buff->width;;

            for(int j = 0; j < buff->width; j++) {
               color = *p0;
               color = rgba888toargb565(color);

               if (!buff->globle_alpha) {
                   if (!argb565_equal((0xffff00&(p_bpp8[0]|(p_bpp8[1] << 8)|(p_bpp8[2] << 16))), 0xffff00&color)) {
                      TRACE(0,"argb565_equal i:%d |%x|%x",i, (0xffffff&(p_bpp8[0]|(p_bpp8[1] << 8)|p_bpp8[2] << 16)), 0xffffff&color);
                      return -1;
                   }
               } else {
                   color &=~(0x0000ff);
                   color |= buff->alpha;
                   if (!argb565_equal((p_bpp8[0]|(p_bpp8[1] << 8)|(p_bpp8[2] << 16)), 0xffffff&color)) {
                       return -1;
                   }
               }
               p_bpp8 += 3;
               p0 += 1;

            }
        }
    }


    return 0;
}


unsigned lodepng_encode_memory(unsigned char** out, size_t* outsize, const unsigned char* image,
                               unsigned w, unsigned h, LodePNGColorType colortype, unsigned bitdepth);
extern uint32_t __pngimage_start_flash[];
extern uint32_t __pngimage_end_flash[];

void pngdec_test_entry_(void)
{
    uint8_t *p0;
    uint8_t *p1;
    unsigned char *out;
    uint32_t * v;
    size_t outsize;
    int psc = sizeof(pngsize)/sizeof(pngsize[0]);
    int err;
#ifndef USE_LODEPNG_ENCODE
    int fsz = ( (uint32_t)__pngimage_end_flash - (uint32_t)__pngimage_start_flash);
#endif
    PNGDEC_INFO("%s %d",__func__,__LINE__);
    hal_psram_init();
    pngdec_heap = pngdec_mem_setup(pngdec_base, 1024*1024*2);
    PNGDEC_INFO("%s %d",__func__,__LINE__);

    for (int t = 0; t < psc; t++) {
    //create png file
        v = pngpixes;

        for(int c=0; c < pngsize[t].w*pngsize[t].h; c++) {
           *v++ = c;
        }

    #ifdef USE_LODEPNG_ENCODE
        lodepng_encode_memory(&out, &outsize, pngpixes,
                              pngsize[t].w, pngsize[t].h, LCT_RGBA, 8);
        pngfile.data = out;
        pngfile.size = outsize;
    #else
        //read png file
        pngfile.data = pngfiledata;
        memcpy(pngfile.data,__pngimage_start_flash, fsz); //out;
        pngfile.size = fsz;
    #endif


        dest.trigger_lines = 0;
        dest.alpha = 0x7F;
        dest.globle_alpha = false;
        dest.pixbufalloc = pixbufalloc;
        dest.trigger_lines_handler = trigger_lines_handler;
        //decode
        pngdec_initialize();
        PNGDEC_INFO("%s %d",__func__,__LINE__);

        int count = sizeof(colorfmt)/sizeof(colorfmt[0]);

        for (int i = 0; i < count; i++) {
            set_dest_colorfmt(&dest,colorfmt[i]);

        //    dest.pitch = 2400;
            dest.pitch = 0;

            #if 0
            //create png file
            if (i%2) {
                v = pngpixes;

                for(int c=0; c < pngsize[t].w*pngsize[t].h; c++) {
                   *v++ = c;
                }
            } else {
                 v = pngpixes;

                for(int c=0; c < pngsize[t].w*pngsize[t].h; c++) {
                   *v++ = 0xffffffff - c;
                }
            }
            #endif

            err = pngdec(&pngfile, &dest);
            if (err) {
                continue;
            }
            PNGDEC_INFO("%s %d",__func__,__LINE__);

            //check result
            #if 0 //blend test
                p0 = blend_pixes;
            #else
                p0 = pngpixes;
            #endif
            p1 = pngpixes_decode;

            if (check_result(&dest,p0,p1)) {
               PNGDEC_INFO("%s  cfmt:%s decode fail",__func__, color_string[colorfmt[i]]);
               PNGDEC_INFO("%s  size: %d x %d  decode fail",__func__,pngsize[t].w, pngsize[t].h);
            } else {
               PNGDEC_INFO("%s %d cfmt:%s decode pass",__func__,__LINE__, color_string[colorfmt[i]]);
               PNGDEC_INFO("%s  size: %d x %d  decode pass",__func__,pngsize[t].w, pngsize[t].h);
            }

        }
        // globle alpha test
        dest.globle_alpha = true;

        for (int i = 0; i < count; i++){
            set_dest_colorfmt(&dest,colorfmt[i]);
            dest.pitch = 0;

            #if 0
            //create png file
            if (i%2) {
                v = pngpixes;

                for(int c=0; c < pngsize[t].w*pngsize[t].h; c++) {
                   *v++ = c;
                }
            } else {
                 v = pngpixes;

                for(int c=0; c < pngsize[t].w*pngsize[t].h; c++) {
                   *v++ = 0xffffffff - c;
                }
            }
            #endif
            err = pngdec(&pngfile, &dest);
            if (err) {
                continue;
            }
            PNGDEC_INFO("%s %d",__func__,__LINE__);

            //check result
            p0 = pngpixes;
            p1 = pngpixes_decode;

            if (check_result(&dest,p0,p1)) {
               PNGDEC_INFO("%s %d cfmt:%s decode fail",__func__,__LINE__, color_string[colorfmt[i]]);
            } else {
               PNGDEC_INFO("%s %d cfmt:%s decode pass",__func__,__LINE__, color_string[colorfmt[i]]);
            }

        }
    }

}

void pngdec_test_entry(void)
{
    uint8_t *p0;
    uint8_t *p1;
    unsigned char *out;
    uint32_t * v;
    size_t outsize;
    struct dma2d_pngfb dma2dfb;
    int err;
#ifndef USE_LODEPNG_ENCODE
    int fsz = ( (uint32_t)__pngimage_end_flash - (uint32_t)__pngimage_start_flash);
#endif
    PNGDEC_INFO("%s %d",__func__,__LINE__);
    hal_psram_init();
    pngdec_heap = pngdec_mem_setup(pngdec_base, 1024*1024*2);
    PNGDEC_INFO("%s %d",__func__,__LINE__);
    //create png file
    v = pngpixes;

    for(int c=0; c < pngsize[0].w*pngsize[0].h; c++) {
       *v++ = c;
    }
    #ifdef USE_LODEPNG_ENCODE
        lodepng_encode_memory(&out, &outsize, pngpixes,
                              pngsize[0].w, pngsize[0].h, LCT_RGBA, 8);
        pngfile.data = out;
        pngfile.size = outsize;
    #else
        //read png file
        pngfile.data = pngfiledata;
        memcpy(pngfile.data,__pngimage_start_flash, fsz); //out;
        pngfile.size = fsz;
    #endif
        dest.trigger_lines = 0;
        dest.alpha = 0x7F;
        dest.globle_alpha = false;
        dest.pixbufalloc = dma2d_setup_wbfb;
        dest.trigger_lines_handler = trigger_lines_handler;
        dest.fmt = PNGDEC_PIX_FMT_RGBA888;
        dest.pitch = 0;
        dest.dma2dfb = &dma2dfb;
        dest.dma2dfb->lid = DMA2D_LAYER_LFORE;
        dest.dma2d_start = dma2d_start;
        //decode
        pngdec_initialize();
#if 0
        if (dest->dma2dfb) {
            hal_dma2d_pngfb_enable(dest->dma2dfb->lid, 1);
        }
#endif
        PNGDEC_INFO("%s %d",__func__,__LINE__);

        err = pngdec(&pngfile, &dest);
        if (err) {
            PNGDEC_INFO("%s %d err:%d",__func__,__LINE__,err);
            return;
        }
        PNGDEC_INFO("%s %d",__func__,__LINE__);
    #if 1
        //check result
        #ifdef PNG2D_BLEND_TEST
             p0 = blend_pixes;
             unsigned char r;
             unsigned char g;
             unsigned char b;
             unsigned char a;
             unsigned char rb;
             unsigned char gb;
             unsigned char bb;
             #ifdef CONST_2DBLEND_MODE_TEST
                 //const pixel alpha: flc(1-a) + blc*a
                 //ra = a
                 for (int i = 0; i < 480*480; i += 4  )
                 {
                     r = pngpixes[i];
                     g = pngpixes[i + 1];
                     b = pngpixes[i + 2];
                     a = const_alpha;
                     rb = bl_fb[i];
                     gb = bl_fb[i + 1];
                     bb = bl_fb[i + 2];
                     p0[i] = (r * (255-a)) / 255;
                     p0[i + 1] = (g * (255 - a)) / 255;
                     p0[i + 2] = (b * (255-a)) / 255;
                     p0[i] += (rb * a) / 255;
                     p0[i + 1] += (gb * a) / 255;
                     p0[i + 2] += (bb * a) / 255;
                 }
             #else
                 // pixel alpha: flc(1-a) + blc*a
                 for (int i = 0; i < 480*480; i += 4  )
                 {
                     r = pngpixes[i];
                     g = pngpixes[i + 1];
                     b = pngpixes[i + 2];
                     a = bl_fb[3];
                     rb = bl_fb[i];
                     gb = bl_fb[i + 1];
                     bb = bl_fb[i + 2];
                     p0[i] = (r * (255-a)) / 255;
                     p0[i + 1] = (g * (255 - a)) / 255;
                     p0[i + 2] = (b * (255-a)) / 255;
                     p0[i] += (rb * a) / 255;
                     p0[i + 1] += (gb * a) / 255;
                     p0[i + 2] += (bb * a) / 255;
                 }
             #endif
        #else
             p0 = pngpixes;
        #endif

        p1 = pngpixes_decode;

        if (check_result(&dest,p0,p1)) {
           PNGDEC_INFO("%s  cfmt:%s decode fail",__func__, color_string[colorfmt[1]]);
           PNGDEC_INFO("%s  size: %d x %d  decode fail",__func__,pngsize[0].w, pngsize[0].h);
        } else {
           PNGDEC_INFO("%s %d cfmt:%s decode pass",__func__,__LINE__, color_string[colorfmt[1]]);
           PNGDEC_INFO("%s  size: %d x %d  decode pass",__func__,pngsize[0].w, pngsize[0].h);
        }
        // globle alpha test
        dest.globle_alpha = true;

        dest.pitch = 0;
        err = pngdec(&pngfile, &dest);
        if (err) {
            PNGDEC_INFO("%s %d err:%d",__func__,__LINE__,err);
            return;
        }
        PNGDEC_INFO("%s %d",__func__,__LINE__);

        //check result
        p1 = pngpixes_decode;
        #ifdef PNG2D_BLEND_TEST
             p0 = blend_pixes;
             #ifdef CONST_2DBLEND_MODE_TEST
                 //const pixel alpha: flc(1-a) + blc*a
                 for (int i = 0; i < 480*480; i += 4  )
                 {
                     r = pngpixes[i];
                     g = pngpixes[i + 1];
                     b = pngpixes[i + 2];
                     a = const_alpha;
                     rb = bl_fb[i];
                     gb = bl_fb[i + 1];
                     bb = bl_fb[i + 2];
                     p0[i] = (r * (255-a)) / 255;
                     p0[i + 1] = (g * (255 - a)) / 255;
                     p0[i + 2] = (b * (255-a)) / 255;
                     p0[i] += (rb * a) / 255;
                     p0[i + 1] += (gb * a) / 255;
                     p0[i + 2] += (bb * a) / 255;
                 }
             #else
                 // pixel alpha: flc(1-a) + blc*a
                 for (int i = 0; i < 480*480; i += 4  )
                 {
                     r = pngpixes[i];
                     g = pngpixes[i + 1];
                     b = pngpixes[i + 2];
                     a = bl_fb[3];
                     rb = bl_fb[i];
                     gb = bl_fb[i + 1];
                     bb = bl_fb[i + 2];
                     p0[i] = (r * (255-a)) / 255;
                     p0[i + 1] = (g * (255 - a)) / 255;
                     p0[i + 2] = (b * (255-a)) / 255;
                     p0[i] += (rb * a) / 255;
                     p0[i + 1] += (gb * a) / 255;
                     p0[i + 2] += (bb * a) / 255;
                 }
             #endif

        #else
             p0 = pngpixes;
        #endif

        if (check_result(&dest,p0,p1)) {
           PNGDEC_INFO("%s %d cfmt:%s decode fail",__func__,__LINE__, color_string[colorfmt[1]]);
        } else {
           PNGDEC_INFO("%s %d cfmt:%s decode pass",__func__,__LINE__, color_string[colorfmt[1]]);
        }

    #endif
}