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
#include <stdio.h>
#include <string.h>
#include "cmsis_nvic.h"
#include "hal_uart.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_psc.h"
#include "hal_cache.h"
#include "hal_psram.h"
#include "heap_api.h"
#include "gpu_port.h"
#include "vg_lite.h"
#include "vg_lite_kernel.h"
#include "vg_lite_hw.h"
#include "vg_lite_hal.h"
#include "vg_lite_platform.h"
#include "hal_location.h"
#include "gpu_port.h"
#include "gpu_common.h"
//#include <vg_lite_ext.h>

uint32_t get_fb_base(void);

extern int vg_lite_save_raw(const char *name, vg_lite_buffer_t *buffer);
// extern void vg_lite_get_cmdbuf_info(uint32_t *start, uint32_t *bytes);
// extern uint32_t vg_lite_get_tsbuffer_size(void);
extern int fb_lcdc_display_buffer_test(uint8_t *buffer);
#ifdef CONFIG_FB_PANDISPLAY
uint32_t  fb_get_display_buffer(void);
uint32_t  fb_update_display_buffer(void);


int gpu_lcdc_display(vg_lite_buffer_t * buffer)
{
#ifdef TEST_WITH_LCD

    TC_INIT

    vg_lite_buffer_t display_buffer;
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    vg_lite_float_t y = SCREEN_OFFSET;
    vg_lite_float_t x = y;
    uint32_t start, end, times;

    filter = VG_LITE_FILTER_POINT;

    memcpy(&display_buffer, buffer, sizeof(vg_lite_buffer_t));
    display_buffer.width = BOARD_LCDC_WIDTH;
    display_buffer.height = BOARD_LCDC_HEIGHT;
    display_buffer.stride = BOARD_LCDC_WIDTH * CONFIG_LCDC_FB_BPP/8;
    display_buffer.format = GPU_FB_FORMAT;
    display_buffer.memory = fb_get_display_buffer();
    display_buffer.address = display_buffer.memory;

    vg_lite_identity(&matrix);
    vg_lite_translate(x, y, &matrix);    // The action is used to center
    
    TC_START

    vg_lite_blit(&display_buffer, buffer, &matrix, VG_LITE_BLEND_NONE, 0, filter);
    //vg_lite_rectangle_t rect = {0, 0, 200, 200};
    //vg_lite_blit_rect(&display_buffer, buffer, &rect, &matrix, VG_LITE_BLEND_NONE, 0xffff0000, filter);
    TC_END
    TC_REP(test_blit_drv_cost);

    TC_START
    vg_lite_finish();
    TC_END

    TC_REP(test_blit);
    fb_update_display_buffer();
#endif    
    return 0;
}

#else

int gpu_lcdc_display(vg_lite_buffer_t * buffer)
{
#ifdef TEST_WITH_LCD
    TC_INIT

    vg_lite_buffer_t display_buffer;
    //vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    vg_lite_float_t y = SCREEN_OFFSET;
    vg_lite_float_t x = y;
    vg_lite_rectangle_t blitRect = {0,0,BOARD_LCDC_WIDTH,BOARD_LCDC_HEIGHT};

    //filter = VG_LITE_FILTER_POINT;

    memcpy(&display_buffer,buffer, sizeof(vg_lite_buffer_t));
    display_buffer.width = BOARD_LCDC_WIDTH;
    display_buffer.height = BOARD_LCDC_HEIGHT;
    display_buffer.stride = BOARD_LCDC_WIDTH*CONFIG_LCDC_FB_BPP/8;
    display_buffer.memory = (void *)get_fb_base();
    display_buffer.address = get_fb_base();
#if 1
    vg_lite_identity(&matrix);
    vg_lite_translate(x,y,&matrix);
    TC_START
    //vg_lite_blit(&display_buffer, buffer,&matrix, VG_LITE_BLEND_NONE, 0, filter);
    vg_lite_blit_rect(&display_buffer,buffer,&blitRect,&matrix,VG_LITE_BLEND_SRC_OVER, 0xffff0000,VG_LITE_FILTER_POINT);

    TC_END
    TC_REP(test_blit_drv_cost);

    TC_START
    vg_lite_finish();
    TC_END
    GPU_REP("CALCULATE BLIT COST TIME######");
    TC_REP(test_blit);
#else
    struct fb_area_s sarea;
    struct fb_area_s darea;

    sarea.x = 0;
    sarea.y = 0;
    sarea.w = buffer->width;
    sarea.h = buffer->height;

    darea.x = x;
    darea.y = y;
    darea.w = buffer->width;
    darea.h = buffer->height;

    dma2d_blit_rect(buffer, &display_buffer,
                    &sarea,&darea);
#endif
    GPU_REP("###Render size: stride %d  %d x %d fb:%p\n",
             display_buffer.stride,display_buffer.width, display_buffer.height,
            display_buffer.memory);
    hal_cache_sync_all(HAL_CACHE_ID_D_CACHE);
    GPU_REP("****%s %d fb: %p", __func__, __LINE__, display_buffer.memory);
   // return 0;

#if 1//dsi
    fb_lcdc_display_buffer_test(NULL);//if use gpu mem copy,no need cpu copy
#else//qspi rgb 565
    uint16_t *lptr;
    uint16_t pix;

    lptr = (uint16_t*)sfb.memory;
    for(int i=0; i < sfb.height; i++){
        lptr = (uint16_t*)(i*sfb.stride + (uint32_t)sfb.memory);
        for(int j=0; j<sfb.stride/2; j++ ){
            pix = lptr[j];
            lptr[j] = ((pix & 0xff) << 8) | ((pix &0xff00) >> 8);
        }
    }
    qspi_display_frame_buf(&sfb);
#endif
    hal_sys_timer_delay(MS_TO_TICKS(50));
#endif
    return 0;

}
#endif

int gpu_save_raw(const char *name, vg_lite_buffer_t *buffer)
{
    int status = 1;
#ifdef TEST_WITH_FS
    return vg_lite_save_raw(name, buffer);
#endif
    return status;
}


/**
 * gpu_save_cmdbuffer - save gpu cmd buffer to file
 * vg_lite_get_tsbuffer_size
 */
void gpu_save_cmdbuffer(char * name)
{
 
}


void gpu_set_soft_break_point()
{
#ifdef TEST_WITH_SOFTBP
      extern void set_soft_break_point(void);
      set_soft_break_point();
#endif
}



