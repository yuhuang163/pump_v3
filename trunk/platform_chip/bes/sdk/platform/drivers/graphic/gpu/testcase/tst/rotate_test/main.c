#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "gpu_port.h"
#include "cmsis_nvic.h"
#include "gpu_common.h"
#include "hal_location.h"

#define printf GPU_ERR


#undef malloc
#define malloc  gpu_malloc
#undef free
#define free  gpu_free

#define gpu_get_render_buf get_fb_base

#include "flower_320_360.h"
#include "flower.h"

#define DEFAULT_SIZE   256.0f;
#define __func__ __FUNCTION__
static char *error_type[] =
{
    "VG_LITE_SUCCESS",
    "VG_LITE_INVALID_ARGUMENT",
    "VG_LITE_OUT_OF_MEMORY",
    "VG_LITE_NO_CONTEXT",
    "VG_LITE_TIMEOUT",
    "VG_LITE_OUT_OF_RESOURCES",
    "VG_LITE_GENERIC_IO",
    "VG_LITE_NOT_SUPPORT",
};
#define IS_ERROR(status)         (status > 0)
#define CHECK_ERROR(Function) \
    error = Function; \
    if (IS_ERROR(error)) \
    { \
        printf("[%s: %d] failed.error type is %s\n", __func__, __LINE__,error_type[error]);\
        goto ErrorHandler; \
    }
static int   fb_width = GPU_FB_WIDTH, fb_height = GPU_FB_HEIGHT;
static float fb_scale = 1.0f;

static vg_lite_buffer_t buffer;
static vg_lite_buffer_t * fb;
static vg_lite_buffer_t fb_dump;
static vg_lite_buffer_t srcImage;
static vg_lite_buffer_t dstImage;
static vg_lite_buffer_t dstTiledImage;


static void cleanup(void)
{
    if (buffer.handle != NULL) {
        vg_lite_free(&buffer);
    }

    vg_lite_close();
}


#define TILED_COUNT 4

/*                      
 * 4*4 tiled    pix(h,b,l,c)    
*/

void linear2Tiled_bpp16(uint8_t *linearData, uint32_t width, uint32_t height, uint8_t *tiledData)
{
    uint32_t twidth;
    uint32_t tlines;

    twidth = width/4;
    tlines = height/4;
    uint16_t *tdata = (uint16_t *)tiledData;
    uint16_t *ldata = (uint16_t *)linearData;
    //pix(h,b,l,c)  h: block line, b: block colum, l: line in block, c: colum in block
    for (int h=0; h< tlines; h++) {
        for (int b=0; b< twidth; b++) {
            for (int l = 0; l < 4; l++) { 
                for(int c = 0; c < 4; c++) {
                   *tdata = ldata[(4*h+l)*width + b*4 + c];    
                   tdata ++;
                }
            }
        }
    }
}

void linear2Tiled_bpp32(uint8_t *linearData, uint32_t width, uint32_t height, uint8_t *tiledData)
{
    uint32_t twidth;
    uint32_t tlines;

    twidth = width/4;
    tlines = height/4;
    uint32_t *tdata = (uint32_t *)tiledData;
    uint32_t *ldata = (uint32_t *)linearData;
    //pix(h,b,l,c)  h: block line, b: block colum, l: line in block, c: colum in block
    for (int h=0; h< tlines; h++) {
        for (int b=0; b< twidth; b++) {
            for (int l = 0; l < 4; l++) { 
                for(int c = 0; c < 4; c++) {
                   *tdata = ldata[(4*h+l)*width + b*4 + c];    
                   tdata ++;
                }
            }
        }
    }
}


void srcPixel2DstPixel(uint8_t *srcData,vg_lite_buffer_format_t srcFormat,
                        uint32_t w, uint32_t h,
                        uint8_t *dstData,vg_lite_buffer_format_t dstFormat)
{
    uint8_t *srcLine = srcData;
    uint8_t *dstLine = dstData;
    //rgba8888->rgba4444
    if(srcFormat==VG_LITE_RGBA8888&&dstFormat==VG_LITE_RGBA4444){
        uint8_t* srcPixel = srcLine;
        uint16_t* dstPixel = (uint16_t*)dstLine;
        uint32_t srcStride = 4 * w;
        uint32_t dstStride = 2 * w;
        for(int i=0;i<h;i++){
            srcPixel = srcLine;
            dstPixel = (uint16_t*)dstLine;
            for(int j=0;j<w;j++){
                *dstPixel = (((srcPixel[0]>>4)&0xF)<<0)| //r
                           (((srcPixel[1]>>4)&0xF)<<4)| //g
                           (((srcPixel[2]>>4)&0xF)<<8)| //b
                           (((srcPixel[3]>>4)&0xF)<<12); //a
                srcPixel+=4;
                dstPixel++;
            }
            srcLine+=srcStride;
            dstLine+=dstStride;
        }
    }
}

static vg_lite_error_t vg_lite_load_image_dst(vg_lite_buffer_t *buffer,
                                                vg_lite_buffer_format_t bufferFormat,
                                                uint32_t width,
                                                uint32_t height,
                                                uint8_t pixelByte,
                                                const uint8_t * data)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    unsigned char *pval8;
    int i;
    buffer->width = width;
    buffer->height = height;
    buffer->stride = width*pixelByte;
    buffer->format = bufferFormat;
    buffer->image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    buffer->transparency = VG_LITE_IMAGE_TRANSPARENT;
    CHECK_ERROR(vg_lite_allocate(buffer));
    pval8 = (unsigned char *)buffer->memory;
    memset(buffer->memory,0xff,buffer->stride * buffer->height);
    GPU_ERR("data=%p",data);
    for(i = 0;i < buffer->height;i++){
        memcpy(pval8, &data[i * buffer->stride], buffer->stride);
        pval8 += buffer->stride;
    }

    return error;
ErrorHandler:
    cleanup();
    return error;
}

static int angle = 0;

void vg_lite_rotate_img(vg_lite_buffer_t * fb)
{
    static int times = 0;
    TC_INIT
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_load_image_dst(&srcImage,VG_LITE_RGBA8888,320,360,4,flower_320_360);
    dstImage.width = 320;
    dstImage.height = 360;
    dstImage.stride = 320*2;
    dstImage.format = VG_LITE_RGBA4444;
    dstImage.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    dstImage.transparency = VG_LITE_IMAGE_TRANSPARENT;
    CHECK_ERROR(vg_lite_allocate(&dstImage));

#ifdef  PLD_SRC_SRAM_TEST
   // dstImage.memory = M55_SYS_RAM_BASE + ()0x100000;
    dstImage.memory = M55_SYS_RAM_BASE + 0xa0000;
    dstImage.address = ADDR_CPU_TO_DEV(dstImage.memory);
#endif

    srcPixel2DstPixel(srcImage.memory,VG_LITE_RGBA8888,320,360,dstImage.memory,VG_LITE_RGBA4444);

    dstTiledImage.width = 320;
    dstTiledImage.height = 360;
    dstTiledImage.stride = 320*2;
    dstTiledImage.format = VG_LITE_RGBA4444;
    dstTiledImage.tiled = VG_LITE_TILED;
    dstTiledImage.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    dstTiledImage.transparency = VG_LITE_IMAGE_TRANSPARENT;
    CHECK_ERROR(vg_lite_allocate(&dstTiledImage));

#ifdef PLD_SRC_SRAM_TEST 
    //dstTiledImage.memory = M55_SYS_RAM_BASE + 0x100000;
    dstTiledImage.memory = M55_SYS_RAM_BASE + 0xa0000 + 0x40000;
    dstTiledImage.address = ADDR_CPU_TO_DEV(dstTiledImage.memory);
#endif

    GPU_REP("dstTiledImage.memory=%p######",dstTiledImage.memory);

    error = vg_lite_blit(&dstTiledImage, &dstImage, 0, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_BI_LINEAR);
    TRACE(0,"%s blit err:%d", __func__, error);

    vg_lite_matrix_t matrix;
    
    // test tiled mode src
    angle = 0;

    //for(int i = 0 ; i < 7; i++) {
    for(int i = 0 ; i < 1; i++) {
        vg_lite_clear(fb,NULL,0xff000000);
        vg_lite_identity(&matrix);
        vg_lite_translate((fb->width)/2.0f, (fb->height)/2.0f, &matrix);
        vg_lite_rotate(angle, &matrix);
//        vg_lite_translate(-160, -180, &matrix);
        vg_lite_translate(-(fb->width)/2.0f, -(fb->height)/2.0f, &matrix);

        TC_START
        error = vg_lite_blit(fb, &dstTiledImage, &matrix, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_BI_LINEAR);
    TRACE(0,"%s blit err:%d", __func__, error);

        error = vg_lite_finish();
    TRACE(0,"%s finish err:%d", __func__, error);

        TC_END
        GPU_REP("CALCULATE vg_lite_rotate_img_linear COST TIME =%d angle=%d######",0,angle);
        TC_REP(vg_lite_rotate_img);  //6-9ms psram -> sram
        cpu_gpu_data_cache_invalid(fb->memory, fb->height * fb->stride);

        GPU_REP("%s fb mem w:%d h:%d", __func__, fb->width,fb->height);
        GPU_REP("%s fb mem start:%p  end:%p", __func__, fb->memory, (uint32_t)(fb->memory) + fb->height * fb->stride );

        vg_lite_identity(&matrix);
        vg_lite_blit(&fb_dump, fb, &matrix, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
        vg_lite_finish();
        int dump_length = fb_dump.height * fb_dump.stride;
        GPU_REP("%s dump mem w:%d h:%d  stride: %d l:%d", __func__, fb_dump.width,fb_dump.height, fb_dump.stride,dump_length);
        GPU_REP("%s dump mem start:%p  end:%p", __func__, fb_dump.memory, (uint32_t)(fb_dump.memory) + dump_length );
        
#ifdef PLD_PSRAMDUMP
        set_pld_dump_mem_description(fb_dump.memory, dump_length, i + times, 1);
        trigger_pld_dump_mem();
#endif
        times += 1;
        angle += 15;
      }

#if 0    
      // test linear mode src
      angle = 0;
      for(int i = 0 ; i < 7; i++) {
        vg_lite_clear(fb,NULL,0xff000000);
        vg_lite_identity(&matrix);
        vg_lite_translate((fb_width)/2.0f, (fb_height)/2.0f, &matrix);
        vg_lite_rotate(angle, &matrix);
        vg_lite_translate(-160, -180, &matrix);

        TC_START
        vg_lite_blit(fb, &dstImage, &matrix, VG_LITE_BLEND_SRC_OVER, 0xFF0000FF, VG_LITE_FILTER_BI_LINEAR);
        vg_lite_finish();
        TC_END
        GPU_REP("CALCULATE vg_lite_rotate_img_tiled  angle=%d######",angle);
        TC_REP(vg_lite_rotate_img);  //6-9ms psram -> sram

        cpu_gpu_data_cache_invalid(fb->memory, fb->height * fb->stride);
        GPU_REP("%s fb mem w:%d h:%d", __func__, fb->width,fb->height);
        GPU_REP("%s fb mem start:%p  end:%p", __func__, fb->memory, (uint32_t)(fb->memory) + fb->height * fb->stride );

        vg_lite_identity(&matrix);
        vg_lite_blit(&fb_dump, fb, &matrix, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
        vg_lite_finish();

       // GPU_REP("%s dump mem w:%d h:%d", __func__, fb_dump.width,fb_dump.height);
       // GPU_REP("%s dump mem start:%p  end:%p", __func__, fb_dump.memory, (uint32_t)(fb_dump.memory) + fb_dump.height * fb_dump.stride );
#ifdef PLD_PSRAMDUMP
        set_pld_dump_mem_description(fb_dump.memory, fb_dump.height * fb_dump.stride, 7+i, 1);
        trigger_pld_dump_mem();
#endif
        angle += 15;
      }
#endif
    vg_lite_free(&dstImage);
    vg_lite_free(&dstTiledImage);
    vg_lite_free(&srcImage);
    return error;
ErrorHandler:
   // cleanup();
    return error;
}

int rotate_test_entry_main(int argc, const char * argv[])
{

    uint32_t feature_check = 0;
    // vg_lite_filter_t filter;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    /* Initialize the draw. */
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));
    printf("Framebuffer size: %d x %d\n", fb_width, fb_height);

    /* Allocate the off-screen buffer. */
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = GPU_FB_FORMAT;
    CHECK_ERROR(vg_lite_allocate(&buffer));
    memcpy(&fb_dump,&buffer,sizeof(vg_lite_buffer_t));
    fb = &buffer;
#if 1 //def PLD_DEST_SRAM_TEST
    buffer.memory = M55_SYS_RAM_BASE;
    buffer.address = ADDR_CPU_TO_DEV(buffer.memory);
#endif

    vg_lite_rotate_img(fb);

ErrorHandler:
    cleanup();
    return 0;
}

void vg_lite_clear_img(vg_lite_buffer_t * fb)
{
    static int times = 0;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_matrix_t matrix;

    for(int i = 0 ; i < 1; i++) {
        vg_lite_clear(fb,NULL,0xff1a2a3a);

        error = vg_lite_finish();

        GPU_REP("%s fb mem w:%d h:%d", __func__, fb->width,fb->height);
        GPU_REP("%s fb mem start:%p  end:%p", __func__, fb->memory, (uint32_t)(fb->memory) + fb->height * fb->stride );

        vg_lite_identity(&matrix);
        vg_lite_blit(&fb_dump, fb, &matrix, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
        vg_lite_finish();

        int dump_length = fb_dump.height * fb_dump.stride;
        GPU_REP("%s dump mem w:%d h:%d  stride: %d l:%d", __func__, fb_dump.width,fb_dump.height, fb_dump.stride,dump_length);
        GPU_REP("%s dump mem start:%p  end:%p", __func__, fb_dump.memory, (uint32_t)(fb_dump.memory) + dump_length );
        
#ifdef PLD_PSRAMDUMP
        set_pld_dump_mem_description(fb_dump.memory, dump_length, i + times, 1);
        trigger_pld_dump_mem();
#endif
        times += 1;
      }

}



int rotate_test_with_gamu(vg_lite_buffer_t *fb)
{
    // vg_lite_filter_t filter;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    /* Initialize the draw. */
    CHECK_ERROR(vg_lite_init(fb->width, fb->height));
    /* Allocate the off-screen buffer. */
    
    buffer.width  = fb->width;
    buffer.height = fb->height;
    buffer.format = fb->format;

    CHECK_ERROR(vg_lite_allocate(&buffer));
    memcpy(&fb_dump,&buffer,sizeof(vg_lite_buffer_t));

    printf("dump Framebuffer size: %d x %d\n  sride: %d", buffer.width, buffer.height,buffer.stride );

    #if 1
    vg_lite_rotate_img(fb);
    #else
    vg_lite_clear_img(fb);
    #endif

ErrorHandler:
    cleanup();
    return 0;
}
