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
#ifdef GAMU_TEST_WITH_GPU

#include "hal_cache.h"
#include "hal_cmu.h"
#include "hal_gamu.h"
#include "hal_timer.h"
#include "hal_trace.h"
#include "hal_psram.h"
#include "hal_cache.h"
#include "stdio.h"
#include "string.h"
#include "hal_location.h"
#include "check_raw_image.h"
#include <math.h>
#include "vg_lite.h"
#include "hal_sysfreq.h"

#define GAMU_TEST_INFO(...) TR_INFO(TR_MOD(TEST), "//" __VA_ARGS__)
#define GAMU_TEST_REP(...)  TR_INFO(TR_MOD(TEST), "//" __VA_ARGS__)
#define GAMU_TEST_ERR(...)  TR_ERROR(TR_MOD(TEST), "//" __VA_ARGS__)

/**
 * Pre-processor Definitions
 */

#define _TEST_REP(...)  TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)

#define TC_INIT  volatile uint32_t time_s; \
                 volatile uint32_t time_e;
#define TC_START time_s = hal_sys_timer_get();
#define TC_END   time_e = hal_sys_timer_get();

#define _TSTR(s) #s

#define TC_REP(s) do{ \
                      _TEST_REP(_TSTR(s)":%d us", TICKS_TO_US(time_e - time_s)); \
                    }while(0)

#define GAMU_PHYBUFFER_SIZE    (464*454*4)

#if 1 //def HWTEST
static uint8_t  *gamu_phy_buff0 = (uint8_t *)(SYS_PSRAM0_BASE + 0x100000);
static uint8_t  *gamu_phy_buff1 = (uint8_t *)(SYS_PSRAM0_BASE + 0x200000);
//static uint8_t  *gamu_phy_buff0 = (uint8_t *)(PSRAM_NC_BASE + 0x100000);
//static uint8_t  *gamu_phy_buff1 = (uint8_t *)(PSRAM_NC_BASE + 0x200000);
#else
static uint8_t  gamu_phy_buff0[GAMU_PHYBUFFER_SIZE] ALIGNED(64) PSRAM_NC_BSS_LOC;
static uint8_t  gamu_phy_buff1[GAMU_PHYBUFFER_SIZE] ALIGNED(64) PSRAM_NC_BSS_LOC;
#endif

typedef struct {
    int w;
    int h;
}_point_t;

#define square(n)   (n * n)


static int point_in_circle( _point_t *p,_point_t *c, int r)
{
   //length to center
   int l = square(p->w - c->w) + square(p->h - c->h);
   l = sqrt(l);

   if (l < r ) {
       return 1;
   } else {
      return 0;
   }
}

//static uint32_t get_point_addr(uint32_t base, _point_t *p, int bpp)
uint32_t get_point_addr(uint32_t base, _point_t *p, int bpp)
{
   uint32_t addr = base + p->w*p->h*bpp/8;
   return addr;
}


static void* fb_malloc(uint32_t size, uint8_t aligns)
{
    uint32_t addr = gamu_phy_buff0;
 #if 0 
    addr &= ~(0xFF000000);
    addr |= 0x3c000000;
 #endif
    return addr;
}

static void* fb_malloc_01(uint32_t size, uint8_t aligns)
{
    uint32_t addr = gamu_phy_buff1;
    #if 0
    addr &= ~(0xFF000000);
    addr |= 0x3c000000;
    #endif
    return addr ;
}


/*
 * gamu_fb layout
            width = 512
    |---------------------|
    |(xoffset, yoffset)   |
    | -|----------------| |
    |  |                | |
    |  |                | |
    |  |     2r         | |
    |  |                | |
    |  |                | |
    |  |----------------| |
    |                     |
    |---------------------|
    xoffset = yoffset
*/

static gamu_fb_info_t  gamu_fb = {
    .id = 0,
    .width = 512,  // must be 512 pixels for 1502x
    .height = 512,
    .bpp = 24,//16,//32,
    .align_malloc = fb_malloc,
    .sqrtf = sqrtf,
    .radius = 227,//256,//,128,//48,
    .yoffset = 29, // line offset

    .used_vblocks = 0,
};

static gamu_fb_info_t  gamu_fb1 = {
    .id = 1,
    .width = 512,  // must be 512 pixels for 1502x
    .height = 512,
    .bpp = 24,//16,//32,
    .align_malloc = fb_malloc_01,
    .sqrtf = sqrtf,
    .radius = 227,//256,//,128,//48,
    .yoffset = 29, // line offset
    .used_vblocks = 0,
};

static void gamu_test_with_cpu(gamu_fb_info_t* gamu)
{
    _point_t p;
    _point_t c;
    uint32_t *pix;
    uint16_t *pix_16;
    uint8_t *pix_8;
    uint32_t addr;
    TRACE(0,"%s gamu:%p vmem:%p",__func__, gamu, gamu->vmem);

    if (gamu->vmem == 0) {
       TRACE(0,"%s vmem addr error", __func__);
       return;
    }
    c.w = gamu->yoffset + gamu->radius;
    c.h = gamu->yoffset + gamu->radius; 

    for (int i = 0; i < gamu->width * gamu->height; i++) {
        p.w = i%gamu->width;
        p.h = i/gamu->width;

        addr = get_point_addr(gamu->vmem, &p, gamu->bpp);
        if (addr == 0) {
           TRACE(0,"%s addr error", __func__);
           return;
        }
        if (gamu->bpp == 32) {
            pix = (uint32_t *)addr; 
            *pix = p.w * p.h;
        }

        if (gamu->bpp == 16) {
            pix_16 = (uint16_t *)addr; 
            *pix_16 = p.w * p.h;
        }  

        if (gamu->bpp == 24) {
            pix_8 = (uint8_t *)addr; 
            pix_8[0] = 0xff&(p.w * p.h);
            pix_8[1] = (0xff00&(p.w * p.h)) >> 8;
            pix_8[2] = (0xff0000&(p.w * p.h)) >> 16;
        }           
    }
    TRACE(0,"%s check result",__func__);

    //check result
    for (int i = 0; i < gamu->width * gamu->height; i++) {
        p.w = i%gamu->width;
        p.h = i/gamu->width;
        if (point_in_circle( &p,&c,gamu->radius)) {
            addr = get_point_addr(gamu->vmem, &p, gamu->bpp);
            if (gamu->bpp == 32) {
                pix = (uint32_t *)addr; 
                if (*pix != p.w * p.h){
                   GAMU_TEST_ERR("%s bpp32 fail", __func__);
                }
            }
    
            if (gamu->bpp == 16) {
                pix_16 = (uint16_t *)addr; 
                *pix_16 = p.w * p.h;
                if (*pix_16 != p.w * p.h){
                   GAMU_TEST_ERR("%s bpp16 fail", __func__);
                }
            }  
    
            if (gamu->bpp == 24) {
                pix_8 = (uint8_t *)addr; 
                if ((pix_8[0] != 0xff&(p.w * p.h)) ||
                (pix_8[1] != (0xff00&(p.w * p.h)) >> 8) ||
                (pix_8[2] != (0xff0000&(p.w * p.h) >> 16))) {
                   GAMU_TEST_ERR("%s bpp24 fail", __func__);
                }

            } 
        }
    }
}

static void gamu_test_with_dma2d(gamu_fb_info_t* gamu)
{

}

static vg_lite_buffer_t gamubuffer;

int rotate_test_with_gamu(vg_lite_buffer_t *fb);

static void gamu_test_with_gpu(gamu_fb_info_t* gamu)
{
     vg_lite_buffer_t *vb = &gamubuffer;   
     memset(&gamubuffer, 0, sizeof(vg_lite_buffer_t));

     if (gamu->bpp == 32) {
         vb->format = VG_LITE_RGBA8888;
     }
     if (gamu->bpp == 24) {
         vb->format = VG_LITE_RGB888;
     }
     if (gamu->bpp == 16) {
         vb->format = VG_LITE_RGB565;
     }

     vb->tiled = VG_LITE_LINEAR;
     vb->image_mode = VG_LITE_NORMAL_IMAGE_MODE;
     vb->transparency = VG_LITE_IMAGE_OPAQUE;
   
     vb->width = gamu->width;
     vb->height = gamu->height;
     vb->stride = gamu->stride;
   
     memset(&vb->yuv, 0,sizeof(vb->yuv));
     vb->handle = NULL; 
     vb->memory = gamu->vmem;
     vb->address = ADDR_CPU_TO_DEV(vb->memory);
     rotate_test_with_gamu(vb);
}

POSSIBLY_UNUSED static void psram_pll_init(void)
{
    // PLL will be enabled when setting mem freq for new chips
    hal_sysfreq_req(HAL_SYSFREQ_USER_APP_0, HAL_CMU_FREQ_208M);
    hal_cmu_init_pll_selection();
}

static void psram_init(void)
{
#ifndef __ARM_ARCH_ISA_ARM
#if defined(ROM_BUILD) || defined(PROGRAMMER)
    hal_cache_enable(HAL_CACHE_ID_I_CACHE);
    hal_cache_enable(HAL_CACHE_ID_D_CACHE);
    hal_cache_wrap_enable(HAL_CACHE_ID_I_CACHE);
    hal_cache_wrap_enable(HAL_CACHE_ID_D_CACHE);
    hal_cache_writebuffer_enable(HAL_CACHE_ID_D_CACHE);
    hal_cache_writeback_enable(HAL_CACHE_ID_D_CACHE);
#endif
#if defined(ROM_BUILD) || defined(PROGRAMMER) || !defined(PSRAM_ENABLE)
    psram_pll_init();
    hal_psram_init();
#endif
#endif
}

static uint32_t gamu_wrdata[16];
void gamu_test_entry()
{
  //  #if PSRAM_ENABLE
  //  psram_init();
  //  #endif

#if 0 
    // test with cpu
    hal_gamu_setup(&gamu_fb);
    hal_gamu_setup(&gamu_fb1);

    gamu_test_with_cpu(&gamu_fb);
    gamu_test_with_cpu(&gamu_fb1);
    
    gamu_fb.bpp = 32;
    gamu_fb1.bpp = 32;
    
    hal_gamu_setup(&gamu_fb);
    hal_gamu_setup(&gamu_fb1);
    gamu_test_with_cpu(&gamu_fb);
    gamu_test_with_cpu(&gamu_fb1);

    gamu_fb.bpp = 16;
    gamu_fb1.bpp = 16;
    hal_gamu_setup(&gamu_fb);
    hal_gamu_setup(&gamu_fb1);
    gamu_test_with_cpu(&gamu_fb);
    gamu_test_with_cpu(&gamu_fb1);
#endif

    //test with gpu
    void gpu_init();
    gpu_init();
#if 1
    hal_gamu_setup(&gamu_fb);
    hal_gamu_setup(&gamu_fb1);
    hal_gamu_set_fake_rdata(0xff0000);
    hal_gamu_set_fake_wrdata_buffer(gamu_wrdata);
    gamu_test_with_gpu(&gamu_fb);
    gamu_test_with_gpu(&gamu_fb1);
#else 
    #if 0
    gamu_fb.bpp = 32;
    gamu_fb1.bpp = 32;
    
    hal_gamu_setup(&gamu_fb);
    hal_gamu_setup(&gamu_fb1);
    hal_gamu_set_fake_rdata(0xff00ff00);
    hal_gamu_set_fake_wrdata_buffer(gamu_wrdata);
    gamu_test_with_gpu(&gamu_fb);
    gamu_test_with_gpu(&gamu_fb1);
    #endif
   #if 1
    gamu_fb.bpp = 16;
    gamu_fb1.bpp = 16;
    hal_gamu_setup(&gamu_fb);
    hal_gamu_setup(&gamu_fb1);
    hal_gamu_set_fake_rdata(0x0000ff);
    hal_gamu_set_fake_wrdata_buffer(gamu_wrdata);
    gamu_test_with_gpu(&gamu_fb);
    gamu_test_with_gpu(&gamu_fb1);
  #endif  
#endif
    //gamu_test_with_dma2d(&gamu_fb);
    //gamu_test_with_dma2d(&gamu_fb1);

    GAMU_TEST_REP("%s gamu test over", __func__);
}    

#else
void gamu_test_entry()
{
    
}   
#endif
