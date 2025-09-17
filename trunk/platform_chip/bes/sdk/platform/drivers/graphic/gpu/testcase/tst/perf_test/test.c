
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_location.h"
#include "hal_cache.h"
#include "gpu_port.h"
#include "gpu_common.h"
#include "dma2d.h"
#include "perf_test_cfg.h"

#ifdef RTOS
#include "cmsis_os.h"
#endif
#define GPU_REP(...) TR_INFO(TR_MOD(TEST), "//" __VA_ARGS__)
#define GPU_ERR(...) TR_ERROR(TR_MOD(TEST), "//" __VA_ARGS__)

#undef malloc
#define malloc gpu_malloc
#undef free
#define free gpu_free

static volatile int test_complete = 1;

#if 1
static uint32_t sram_buffer_[480 * 480] ALIGNED(64) SRAM_BSS_LOC;
#define sram_buffer sram_buffer_
#define sram_buffer_dest (sram_buffer_ + 480 * 240)
#endif
static uint32_t fb_width = 480;
static uint32_t fb_height = 240;

extern int evo_path_test_main(int argc, const char *argv[]);
extern int draw_chiness_char_test(vg_lite_buffer_t *fb);
extern int tiger_test_main_entry(int argc, const char *argv[]);
extern int gpu_draw_tiger_test(vg_lite_buffer_t *fb);
extern int rgba8etc2_test(void);
void dma2d_blit_test(uint16_t pixclk);

#if 1
#include "hal_iomux.h"
#include "hal_gpio.h"
static const struct HAL_IOMUX_PIN_FUNCTION_MAP gpu_gpio_iomux[] = {
    {HAL_IOMUX_PIN_P3_0, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLDOWN_ENABLE},
    {HAL_IOMUX_PIN_P3_1, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLDOWN_ENABLE},
    {HAL_IOMUX_PIN_P3_2, HAL_IOMUX_FUNC_GPIO, HAL_IOMUX_PIN_VOLTAGE_VIO, HAL_IOMUX_PIN_PULLDOWN_ENABLE},
};

void gpu_gpio_set(int pin)
{
    hal_gpio_pin_set((enum HAL_GPIO_PIN_T)gpu_gpio_iomux[pin].pin);
}
void gpu_gpio_clr(int pin)
{
    hal_gpio_pin_clr((enum HAL_GPIO_PIN_T)gpu_gpio_iomux[pin].pin);
}
void gpu_gpio_open(void)
{
    int i = 0;
    hal_iomux_init(gpu_gpio_iomux, ARRAY_SIZE(gpu_gpio_iomux));
    for (i = 0; i < ARRAY_SIZE(gpu_gpio_iomux); ++i)
    {
        hal_gpio_pin_set_dir((enum HAL_GPIO_PIN_T)gpu_gpio_iomux[i].pin, HAL_GPIO_DIR_OUT, 0);
    }
}
#endif

uint32_t test_get_sram_buffer(void)
{
    return sram_buffer_;
}

int gpu_open(void)
{
    // gpu_memory_setup();
    // gpu_hw_reset();
    // set_gpu_done_interrupt_handler();
    vg_lite_init(fb_width, fb_height);

    return 0;
}

void gpu_close(void)
{
    vg_lite_close();
}

int gpu_allocate_buffer(vg_lite_buffer_t *buffer)
{
    return vg_lite_allocate(buffer);
}

int gpu_free_buffer(vg_lite_buffer_t *buffer)
{
    return vg_lite_free(buffer);
}

int gpu_clear(vg_lite_buffer_t *buffer, uint32_t rgba8888)
{
    //vg_lite_rectangle_t rect = { 0, 0, 160, 160 };
    vg_lite_error_t error = VG_LITE_SUCCESS;
    GPU_REP("%s %d buffer:%p width :%d height: %d  stride:%d", __func__, __LINE__, buffer->memory, buffer->width, buffer->height, buffer->stride);

#if 1
    error = vg_lite_clear(buffer, 0, rgba8888);
    if (error)
    {
        GPU_REP("%s error:%d", __func__, error);
        return error;
    }
    //vg_lite_clear(buffer, &rect, rgba8888);
    TC_INIT
    TC_START
    vg_lite_finish();
    cpu_gpu_data_cache_invalid(buffer->memory, buffer->height * buffer->stride);

    TC_END
    TC_REP(clear_gcost);
#else
    memset(buffer->memory, 0xff, buffer->height * buffer->stride);
#endif
    while (0)
    {
        uint32_t *p = (uint32_t *)buffer->memory;
        for (int i = 0; i < buffer->width; i += 4)
        {
            GPU_REP("0x%x 0x%x 0x%x 0x%x \n", *p, *(p + 1), *(p + 2), *(p + 3));
            p += 4;
        }
    }
    return 0;
}

int gpu_blit(vg_lite_buffer_t *src, vg_lite_buffer_t *dest, vg_lite_blend_t blend)
{
    vg_lite_filter_t filter;
    // Initialize vglite.
    vg_lite_error_t error = VG_LITE_SUCCESS;

    filter = VG_LITE_FILTER_POINT;

#ifndef GPU_CURRENT_TEST_GPU_BLIT_LOOP
    GPU_REP("%s img_src:%p img_dest :%p ", __func__, src->memory, dest->memory);
    GPU_REP("%s src:%p width :%d height: %d ", __func__, src->memory, src->width, src->height);
#endif

    gpu_gpio_set(1);
    gpu_gpio_set(2);
    error = vg_lite_blit(dest, src, 0, blend, 0, filter);
    gpu_gpio_clr(1);
    if (error)
    {
        GPU_REP("%s error:%d", __func__, error);
        return error;
    }
    cpu_cache_flush(src->memory, src->height * src->stride);
    TC_INIT
    TC_START
    vg_lite_finish();
    TC_END
    cpu_gpu_data_cache_invalid(dest->memory, dest->height * dest->stride);
    gpu_gpio_clr(2);
    TC_REP(blit_gcost);
    while (0)
    {
        uint32_t *p = (uint32_t *)dest->memory;
        for (int i = 0; i < dest->width; i += 4)
        {
            GPU_REP("0x%x 0x%x 0x%x 0x%x \n", *p, *(p + 1), *(p + 2), *(p + 3));
            p += 4;
        }
    }

    return error;
}

int gpu_blit_rect(vg_lite_buffer_t *src, vg_lite_buffer_t *dest, vg_lite_blend_t blend,
                  vg_lite_rectangle_t *srect, vg_lite_rectangle_t *drect)
{
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    vg_lite_float_t scale_x = 1.0;
    vg_lite_float_t scale_y = 1.0;
    // Initialize vglite.
    vg_lite_error_t error = VG_LITE_SUCCESS;

    filter = VG_LITE_FILTER_POINT;

    GPU_REP("%s img_src:%p img_dest :%p ", __func__, src->memory, dest->memory);
    GPU_REP("%s src:%p width :%d height: %d ", __func__, src->memory, src->width, src->height);

    vg_lite_identity(&matrix);

    if (drect)
    {
        if ((drect->width != srect->width) || (drect->height != srect->height))
        { //need scaling
            scale_x = (vg_lite_float_t)drect->width / (vg_lite_float_t)srect->width;
            scale_y = (vg_lite_float_t)drect->height / (vg_lite_float_t)srect->height;
            vg_lite_scale(scale_x, scale_y, &matrix);
        }

        if (drect->x * drect->y != 0)
        { // need trans offset position
            vg_lite_translate(drect->x, drect->y, &matrix);
        }
    }

    error = vg_lite_blit_rect(dest, src, srect, &matrix, blend, 0, filter);
    if (error)
    {
        GPU_REP("%s error:%d", __func__, error);
        return error;
    }
    cpu_cache_flush(src->memory, src->height * src->stride);
    TC_INIT
    TC_START
    vg_lite_finish();
    TC_END
    cpu_gpu_data_cache_invalid(dest->memory, dest->height * dest->stride);
    TC_REP(blit_gcost);

    return error;
}

static int gpu_blit_scale_blend(vg_lite_buffer_t *src, vg_lite_buffer_t *dest,
                                vg_lite_float_t scale_x, vg_lite_float_t scale_y,
                                vg_lite_blend_t blend)
{
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    // Initialize vglite.
    vg_lite_error_t error = VG_LITE_SUCCESS;

    filter = VG_LITE_FILTER_POINT;

    GPU_REP("%s img_src:%p img_dest :%p ", __func__, src->memory, dest->memory);
    GPU_REP("%s src:%p width :%d height: %d ", __func__, src->memory, src->width, src->height);
    vg_lite_identity(&matrix);
    vg_lite_scale(scale_x, scale_x, &matrix);
    error = vg_lite_blit(dest, src, &matrix, blend, 0, filter);
    if (error)
    {
        GPU_REP("%s error:%d", __func__, error);
        return error;
    }
    cpu_cache_flush(src->memory, src->height * src->stride);
    TC_INIT
    TC_START
    vg_lite_finish();
    TC_END
    cpu_gpu_data_cache_invalid(dest->memory, dest->height * dest->stride);
    TC_REP(blit_scale_gcost);

    return error;
}
/*

 c2(move) (x,y)   c5(line to) (+w,0)
    x------------>--------X
    |                     |
 c5 |                     |c5(line to (+0,+h))
 (line to (+0,-h))        |
    |                     |
    ^                     V
    |                     |
    x-------<-------------x
    (0,h)       (w,h)
       c5(line to (-w,+0))
*/

static float rect_path_data[] = {
    0.000000f,
    0.000000f,
    32.000000f,
    0.000000f,
    0.000000f,
    200.000000f,
    -32.000000f,
    0.000000f,
    0.000000f,
    -200.000000f,
};

static uint8_t cmd[] = {
    2,
    5,
    5,
    5,
    5,
    0,
};

static void gpu_set_img_rectpath_data(vg_lite_buffer_t *image, float *data)
{
    data[0] = 0.000000f;
    data[1] = 0.000000f;
    data[2] = (float)image->width;
    data[3] = 0.000000f;
    data[4] = 0.000000f;
    data[5] = (float)image->height;
    data[6] = -1.0f * (float)image->width;
    data[7] = 0.000000f;
    data[8] = -1.0f * (float)image->height;
    data[9] = 0.000000f;
}

static int gpu_draw_scale_blend_pattern(vg_lite_buffer_t *src, vg_lite_buffer_t *dest,
                                        vg_lite_float_t scale_x, vg_lite_float_t scale_y,
                                        vg_lite_blend_t blend)
{
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    vg_lite_matrix_t matPath;
    vg_lite_path_t path;
    uint32_t data_size;

    // Initialize vglite.
    vg_lite_error_t error = VG_LITE_SUCCESS;

    filter = VG_LITE_FILTER_POINT;

    GPU_REP("%s img_src:%p img_dest :%p ", __func__, src->memory, dest->memory);
    GPU_REP("%s src:%p width :%d height: %d ", __func__, src->memory, src->width, src->height);
    vg_lite_identity(&matrix);
    vg_lite_scale(scale_x, scale_x, &matrix);

    vg_lite_identity(&matPath);
    vg_lite_scale(scale_x, scale_x, &matPath);

    data_size = vg_lite_path_calc_length(cmd, sizeof(cmd), VG_LITE_FP32);
    vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0);
    path.path = malloc(data_size);
    gpu_set_img_rectpath_data(src, rect_path_data);
    error = vg_lite_path_append(&path, cmd, rect_path_data, sizeof(cmd));

    if (error)
    {
        free(path.path);
        GPU_REP("%s path append error:%d", __func__, error);
        return error;
    }

    error = vg_lite_draw_pattern(dest, &path, VG_LITE_FILL_EVEN_ODD, &matPath, src, &matrix, blend, VG_LITE_PATTERN_COLOR, 0xffaabbcc, filter);

    if (error)
    {
        GPU_REP("%s error:%d", __func__, error);
        return error;
    }
    cpu_cache_flush(src->memory, src->height * src->stride);
    TC_INIT
    TC_START
    vg_lite_finish();
    TC_END
    cpu_gpu_data_cache_invalid(dest->memory, dest->height * dest->stride);
    TC_REP(gpu_draw_scale_blend_pattern_gcost);

    return error;
}

static int gpu_blit_rotation_blend(vg_lite_buffer_t *src, vg_lite_buffer_t *dest,
                                   vg_lite_float_t degree, vg_lite_blend_t blend)
{
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    // Initialize vglite.
    vg_lite_error_t error = VG_LITE_SUCCESS;

    filter = VG_LITE_FILTER_POINT;

    GPU_REP("%s img_src:%p img_dest :%p ", __func__, src->memory, dest->memory);
    GPU_REP("%s src:%p width :%d height: %d ", __func__, src->memory, src->width, src->height);
    cpu_cache_flush(src->memory, src->height * src->stride);
    TC_INIT
    vg_lite_identity(&matrix);
    vg_lite_translate((dest->width - src->width) / 2, (dest->height - src->height) / 2, &matrix);
    //vg_lite_translate(dest->width/2 ,dest->height/2,&matrix);
    vg_lite_rotate(degree, &matrix);
    error = vg_lite_blit(dest, src, &matrix, blend, 0, filter);
    if (error)
    {
        GPU_REP("%s error:%d", __func__, error);
        return error;
    }
    TC_START
    vg_lite_finish();
    TC_END
    cpu_gpu_data_cache_invalid(dest->memory, dest->height * dest->stride);
    TC_REP(blit_rotation_gcost);

    return error;
}

static int gpu_draw_pattern(vg_lite_buffer_t *src, vg_lite_buffer_t *dest,
                            vg_lite_float_t degree, vg_lite_blend_t blend)
{
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    vg_lite_matrix_t matPath;
    vg_lite_path_t path;
    uint32_t data_size;
    // Initialize vglite.
    vg_lite_error_t error = VG_LITE_SUCCESS;

    filter = VG_LITE_FILTER_POINT;

    GPU_REP("%s img_src:%p img_dest :%p ", __func__, src->memory, dest->memory);
    GPU_REP("%s src:%p width :%d height: %d ", __func__, src->memory, src->width, src->height);
    cpu_cache_flush(src->memory, src->height * src->stride);
    TC_INIT
    TC_START

    vg_lite_identity(&matrix);
    vg_lite_translate(dest->width / 2, dest->height / 2, &matrix);
    vg_lite_rotate(degree, &matrix);

    vg_lite_identity(&matPath);
    vg_lite_translate(dest->width / 2, dest->height / 2, &matPath);
    vg_lite_rotate(degree, &matPath);

    data_size = vg_lite_path_calc_length(cmd, sizeof(cmd), VG_LITE_FP32);
    vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0);
    path.path = malloc(data_size);
    gpu_set_img_rectpath_data(src, rect_path_data);
    error = vg_lite_path_append(&path, cmd, rect_path_data, sizeof(cmd));

    if (error)
    {
        free(path.path);
        GPU_REP("%s path append error:%d", __func__, error);
        return error;
    }

    error = vg_lite_draw_pattern(dest, &path, VG_LITE_FILL_EVEN_ODD, &matPath, src, &matrix, blend, VG_LITE_PATTERN_COLOR, 0xffaabbcc, filter);

    if (error)
    {
        GPU_REP("%s error:%d", __func__, error);
        return error;
    }
    vg_lite_finish();
    TC_END
    cpu_gpu_data_cache_invalid(dest->memory, dest->height * dest->stride);
    TC_REP(blit_rotation_draw_pattern_gcost);
    free(path.path);
    return error;
}

static int gpu_blit_transall_blend(vg_lite_buffer_t *src, vg_lite_buffer_t *dest,
                                   vg_lite_matrix_t *matrix, vg_lite_blend_t blend)
{
    vg_lite_filter_t filter;
    vg_lite_error_t error = VG_LITE_SUCCESS;

    filter = VG_LITE_FILTER_POINT;

    GPU_REP("%s img_src:%p img_dest :%p ", __func__, src->memory, dest->memory);
    GPU_REP("%s src:%p width :%d height: %d ", __func__, src->memory, src->width, src->height);

    error = vg_lite_blit(dest, src, matrix, blend, 0, filter);
    if (error)
    {
        GPU_REP("%s error:%d", __func__, error);
        return error;
    }
    cpu_cache_flush(src->memory, src->height * src->stride);
    TC_INIT
    TC_START
    vg_lite_finish();
    TC_END
    cpu_gpu_data_cache_invalid(dest->memory, dest->height * dest->stride);
    TC_REP(blit_rotation_gcost);
    return error;
}

#define TEST_ALIGMENT 16
static void create_imgA4(vg_lite_buffer_t *buffer)
{
    uint32_t i;
    uint32_t block = 16;
    uint8_t *p = (uint8_t *)buffer->memory;
    uint8_t values[] = {
        0x00, 0x11, 0x22, 0x33,
        0x44, 0x55, 0x66, 0x77,
        0x88, 0x99, 0xaa, 0xbb,
        0xcc, 0xdd, 0xee, 0xff};

    for (i = 0; i < buffer->height; i++)
    {
        memset(p, values[(i / block) % TEST_ALIGMENT], buffer->stride);
        p += buffer->stride;
    }
}

static void create_imgA8(vg_lite_buffer_t *buffer)
{
    uint32_t i;
    uint8_t *p = (uint8_t *)buffer->memory;

    for (i = 0; i < buffer->height; i++)
    {
        memset(p, i, buffer->stride);
        p += buffer->stride;
    }
}

static vg_lite_buffer_t src;
static vg_lite_buffer_t dest;
static vg_lite_buffer_t sram_src;
static vg_lite_buffer_t sram_dest;

void gpu_performance_test(void)
{
    volatile int stop = 1;
#if GPU_PERF_EVO_PATH
    evo_path_test_main(1, 0);
#endif
#if GPU_PERF_DRAW_PATH_TIGER
    tiger_test_main_entry(1, 0);
#endif
    gpu_open();
#if GPU_PERF_RGBA8ETC_TEST
    rgba8etc2_test();
#endif
    src.width = fb_width;
    src.height = fb_height;
    src.format = VG_LITE_RGBA8888;

    dest.width = fb_width;
    dest.height = fb_height;
    dest.format = VG_LITE_RGBA8888;

    sram_src.width = fb_width;
    sram_src.height = fb_height;
    sram_src.format = VG_LITE_RGBA8888;

    sram_dest.width = fb_width;
    sram_dest.height = fb_height;
    sram_dest.format = VG_LITE_RGBA8888;

    gpu_allocate_buffer(&src);
    gpu_allocate_buffer(&dest);

    memcpy(&sram_src, &src, sizeof(vg_lite_buffer_t));
    sram_src.memory = sram_buffer;
    sram_src.address = ADDR_CPU_TO_DEV(sram_src.memory);
    memcpy(&sram_dest, &src, sizeof(vg_lite_buffer_t));
    sram_dest.memory = sram_buffer_dest;
    sram_dest.address = ADDR_CPU_TO_DEV(sram_dest.memory);

#if 0
    {
        // gpu_clear(&src, 0xffff0000);
        // gpu_clear(&dest, 0x00000000);
        // memset(sram_dest.memory, 0 , sram_dest.stride*sram_dest.height + 64);
        memset(sram_dest.memory, 0, sram_dest.stride * sram_dest.height);
        memset(sram_src.memory, 0x5a, sram_dest.stride * sram_dest.height);
        memset(dest.memory, 0, dest.stride * dest.height);
        memset(src.memory, 0x5a, dest.stride * dest.height);
    }
#endif

#ifdef GPU_PERF_BLIT_TEST
    /*
     * copy rect area
    */

    //rgba8888->rgba8888

    sram_src.format = VG_LITE_RGBA8888;
    sram_src.stride = sram_src.width * 4;
    sram_dest.format = VG_LITE_RGBA8888;
    sram_dest.stride = sram_dest.width * 4;
    TRACE(3, "blit rgba8888->rgba8888 sram->sram");
    gpu_blit(&sram_src, &sram_dest, VG_LITE_BLEND_NONE);

    sram_src.format = VG_LITE_RGBA8888;
    sram_src.stride = sram_src.width * 4;
    dest.format = VG_LITE_RGBA8888;
    dest.stride = dest.width * 4;
    TRACE(3, "blit rgba8888->rgba8888 sram->psram");
    gpu_blit(&sram_src, &dest, VG_LITE_BLEND_NONE);

    src.format = VG_LITE_RGBA8888;
    src.stride = src.width * 4;
    sram_dest.format = VG_LITE_RGBA8888;
    sram_dest.stride = sram_dest.width * 3;
    TRACE(3, "blit rgba8888->rgb888 psram->sram");
    gpu_blit(&src, &sram_dest, VG_LITE_BLEND_NONE);

    src.format = VG_LITE_RGBA8888;
    src.stride = src.width * 4;
    dest.format = VG_LITE_RGBA8888;
    dest.stride = dest.width * 4;

    TRACE(3, "blit rgba8888->rgba8888 psram->psram");
    gpu_blit(&src, &dest, VG_LITE_BLEND_NONE);

    //rgba8888->rgb888
    sram_src.format = VG_LITE_RGBA8888;
    sram_src.stride = sram_src.width * 4;
    sram_dest.format = VG_LITE_RGB888;
    sram_dest.stride = sram_dest.width * 3;

    TRACE(3, "blit rgba8888->rgb888 sram->sram");
    gpu_blit(&sram_src, &sram_dest, VG_LITE_BLEND_NONE);

    sram_src.format = VG_LITE_RGBA8888;
    sram_src.stride = sram_src.width * 4;
    dest.format = VG_LITE_RGB888;
    dest.stride = dest.width * 4;
    TRACE(3, "blit rgba8888->rgba8888 sram->psram");
    gpu_blit(&sram_src, &dest, VG_LITE_BLEND_NONE);

    src.format = VG_LITE_RGBA8888;
    src.stride = src.width * 4;
    sram_dest.format = VG_LITE_RGB888;
    sram_dest.stride = sram_dest.width * 3;
    TRACE(3, "blit rgba8888->rgb888 psram->sram");
    gpu_blit(&src, &sram_dest, VG_LITE_BLEND_NONE);

    src.format = VG_LITE_RGBA8888;
    src.stride = src.width * 4;
    dest.format = VG_LITE_RGB888;
    dest.stride = dest.width * 3;

    TRACE(3, "blit rgba8888->rgb888 psram->psram");
    gpu_blit(&src, &dest, VG_LITE_BLEND_NONE);

    //rgba5658->rgb565
    sram_src.format = VG_LITE_RGBA5658;
    sram_src.stride = sram_src.width * 3;
    sram_dest.format = VG_LITE_RGB565;
    sram_dest.stride = sram_dest.width * 2;
    TRACE(3, "blit rgba5658->rgb565 sram->sram");
    gpu_blit(&sram_src, &sram_dest, VG_LITE_BLEND_NONE);

    src.format = VG_LITE_RGBA5658;
    src.stride = sram_src.width * 3;
    sram_dest.format = VG_LITE_RGB565;
    sram_dest.stride = sram_dest.width * 2;
    TRACE(3, "blit rgba5658->rgb565 psram->sram");
    gpu_blit(&src, &sram_dest, VG_LITE_BLEND_NONE);

    src.format = VG_LITE_RGBA5658;
    src.stride = src.width * 3;
    dest.format = VG_LITE_RGB565;
    dest.stride = dest.width * 2;
    TRACE(3, "blit rgba5658->rgb565 psram->psram");
    gpu_blit(&src, &dest, VG_LITE_BLEND_NONE);

    src.format = VG_LITE_RGBA5658;
    src.stride = sram_src.width * 3;
    dest.format = VG_LITE_RGB565;
    dest.stride = dest.width * 2;
    TRACE(3, "blit rgba5658->rgb565 sram->psram");
    gpu_blit(&sram_src, &dest, VG_LITE_BLEND_NONE);

    /*
     *area blending
    */

    sram_src.image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
    sram_src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    src.image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
    src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;

    //rgba8888->rgba8888

    sram_src.format = VG_LITE_RGBA8888;
    sram_src.stride = sram_src.width * 4;
    sram_dest.format = VG_LITE_RGBA8888;
    sram_dest.stride = sram_dest.width * 4;
    TRACE(3, "blit with blend  rgba8888->rgba8888 sram->sram");
    gpu_blit(&sram_src, &sram_dest, VG_LITE_BLEND_SRC_OVER);

    sram_src.format = VG_LITE_RGBA8888;
    sram_src.stride = sram_src.width * 4;
    dest.format = VG_LITE_RGBA8888;
    dest.stride = dest.width * 4;
    TRACE(3, "blit with blend  rgba8888->rgba8888 sram->psram");
    gpu_blit(&sram_src, &dest, VG_LITE_BLEND_SRC_OVER);

    src.format = VG_LITE_RGBA8888;
    src.stride = src.width * 4;
    sram_dest.format = VG_LITE_RGB888;
    sram_dest.stride = sram_dest.width * 3;
    TRACE(3, "blit with blend  rgba8888->rgb888 psram->sram");
    gpu_blit(&src, &sram_dest, VG_LITE_BLEND_SRC_OVER);

    src.format = VG_LITE_RGBA8888;
    src.stride = src.width * 4;
    dest.format = VG_LITE_RGBA8888;
    dest.stride = dest.width * 4;

    TRACE(3, "blit with blend  rgba8888->rgba8888 psram->psram");
    gpu_blit(&src, &dest, VG_LITE_BLEND_SRC_OVER);

    TRACE(3, "gpu perf test over");
    return; //for simu blit

#endif

#if GPU_PERF_BASIC_TEST
#if 0
    /*
     * blit rect area
    */

    TRACE(3,"blit rgba8888->rgba8888 psram->psram");
    uint32_t paddr = src.address;
    src.address += 8;
    src.memory = ADDR_DEV_TO_CPU(src.address);
    src.width = 320;
    src.stride = 320*4;
    src.height = 8;

    dest.width = 320;
    dest.stride = 320*4;
    dest.height = 8;

    gpu_blit(&src, &dest, VG_LITE_BLEND_NONE);


    dest.address += 8;
    dest.memory = ADDR_DEV_TO_CPU(dest.address);

    gpu_blit(&src, &dest, VG_LITE_BLEND_NONE);
#endif

#if 0
    TRACE(3,"blit rgba8888->rgba8888 sram->sram");
    gpu_blit(&sram_src, &sram_dest, VG_LITE_BLEND_NONE);

    TRACE(3,"blit rgba8888->rgba8888 psram->sram");
    gpu_blit(&src, &sram_dest, VG_LITE_BLEND_NONE);

    sram_dest.width  = fb_width;
    sram_dest.stride = (sram_dest.stride)*3/4;
    sram_dest.format = VG_LITE_RGB888;
    TRACE(3,"blit rgba8888->rgb888 sram->sram");
    gpu_blit(&sram_src, &sram_dest, VG_LITE_BLEND_NONE);

    TRACE(3,"blit rgba8888->rgb888 psram->sram");
    gpu_blit(&src, &sram_dest, VG_LITE_BLEND_NONE);

    dest.width  = fb_width;
    dest.stride = (sram_dest.stride)*3/4;
    dest.format = VG_LITE_RGB888;
    TRACE(3,"blit rgba8888->rgb888 psram->psram");
    gpu_blit(&src, &dest, VG_LITE_BLEND_NONE);

    TRACE(3,"blit rgba8888->rgb888 sram->psram");
    gpu_blit(&sram_src, &dest, VG_LITE_BLEND_NONE);
    while(stop);

#endif

    /*
     *fill color into target rect area
    */
    hal_sys_timer_delay_us(200000);
    TRACE(3,"set color to sram buffer 0x0A59F7");
    gpu_clear(&sram_src, 0x0A59F7);

    /*
     * render chinese character "龍" to target
    */
    TRACE(3,"draw chiness char  to sram buffer ");
    draw_chiness_char_test(&sram_dest);
    TRACE(3,"draw chiness char  to psram buffer ");
    draw_chiness_char_test(&dest);

    /*
     *draw tiger in psram
    */
    TRACE(3,"draw tiger to psram buffer w*d: %d * %d", dest.width , dest.height);
    gpu_draw_tiger_test(&dest);

    /*
     *draw tiger in sram
    */

    TRACE(3,"draw tiger to sram buffer w*d: %d * %d", sram_dest.width , sram_dest.height);
    gpu_draw_tiger_test(&sram_dest);



    sram_src.image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
    sram_src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    sram_src.width = 48;
    sram_src.height = 48;
    sram_src.format = VG_LITE_A4;
    sram_src.stride = 48 / 2;
    create_imgA4(&sram_src);
    TRACE(3, "blit blend A4->rgba8888 sram->sram");
    gpu_blit(&sram_src, &sram_dest, VG_LITE_BLEND_SRC_OVER);

    src.image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
    src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    src.width = 48;
    src.height = 48;
    src.format = VG_LITE_A4;
    src.stride = 48 / 2;
    create_imgA4(&src);
    TRACE(3, "blit blend A4->rgba8888 psram->sram");
    gpu_blit(&src, &sram_dest, VG_LITE_BLEND_SRC_OVER);

    src.image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
    src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    src.width = 48;
    src.height = 48;
    src.format = VG_LITE_A8;
    src.stride = 48;
    create_imgA8(&src);
    TRACE(3, "blit blend A8->rgba8888 psram->sram");
    gpu_blit(&src, &sram_dest, VG_LITE_BLEND_SRC_OVER);

    hal_sys_timer_delay_us(200000);

    /*
     * part of area blending
     * src： (48,48)，(368,368)
     * dest：(32,32)，(352,352)
    */
    vg_lite_rectangle_t srect;
    vg_lite_rectangle_t drect;
    srect.x = 48;
    srect.y = 48;
    srect.width = 320;
    srect.height = 320;
    drect.x = 32;
    drect.y = 32;
    drect.width = 320;
    drect.height = 320;

    src.width = fb_width;
    src.height = fb_height;
    src.format = VG_LITE_RGBA8888;
    src.image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
    src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    src.stride = src.width * 4;

    sram_dest.width = fb_width;
    sram_dest.height = fb_height;
    sram_dest.format = VG_LITE_RGBA8888;
    sram_dest.image_mode = VG_LITE_MULTIPLY_IMAGE_MODE;
    sram_dest.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    sram_dest.stride = src.width * 4;

    TRACE(3, "blit rect  psram->sram");

    gpu_blit_rect(&src, &sram_dest, VG_LITE_BLEND_SRC_OVER, &srect, &drect);

    /*
     * blend with scaling (0.7) by normal quality mode
    */
    src.width = fb_width;
    src.height = fb_height;
    src.stride = src.width * 4;
    src.format = VG_LITE_RGBA8888;
    src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    TRACE(3, "blit scale 0.7 0.7 blend  psram->sram");
    gpu_blit_scale_blend(&src, &sram_dest, 0.7, 0.7, VG_LITE_BLEND_SRC_OVER);

    src.width = fb_width;
    src.height = fb_height;
    src.stride = src.width * 4;
    src.format = VG_LITE_RGBA8888;
    src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    TRACE(3, "blit scale 0.7 0.7 blend  psram->psram");
    gpu_blit_scale_blend(&src, &dest, 0.7, 0.7, VG_LITE_BLEND_SRC_OVER);

    /*
     * blend with scaling(0.7) by high quality mode
    */
    src.width = fb_width;
    src.height = fb_height;
    src.stride = src.width * 4;
    src.format = VG_LITE_RGBA8888;
    src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    TRACE(3, "blit scale 0.7 0.7 blend  psram->sram");
    gpu_draw_scale_blend_pattern(&src, &sram_dest, 0.7, 0.7, VG_LITE_BLEND_SRC_OVER);

    src.width = fb_width;
    src.height = fb_height;
    src.stride = src.width * 4;
    src.format = VG_LITE_RGBA8888;
    src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    TRACE(3, "blit scale 0.7 0.7 blend  psram->psram");
    gpu_draw_scale_blend_pattern(&src, &dest, 0.7, 0.7, VG_LITE_BLEND_SRC_OVER);

    /*
     * blend with scaling (1.7) by normal quality mode
    */
    src.width = 240;
    src.height = 120;
    src.stride = src.width * 4;
    src.format = VG_LITE_RGBA8888;
    src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    TRACE(3, "blit scale 1.7 1.7 blend psram->sram");
    gpu_blit_scale_blend(&src, &sram_dest, 1.7, 1.7, VG_LITE_BLEND_SRC_OVER);

    src.width = 240;
    src.height = 120;
    src.stride = src.width * 4;
    src.format = VG_LITE_RGBA8888;
    src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    TRACE(3, "blit scale 1.7 1.7 blend psram->psram");
    gpu_blit_scale_blend(&src, &dest, 1.7, 1.7, VG_LITE_BLEND_SRC_OVER);
    /*
     * blend with scaling (1.7) by high quality mode
    */
    src.width = 240;
    src.height = 120;
    src.stride = src.width * 4;
    src.format = VG_LITE_RGBA8888;
    src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    TRACE(3, "blit scale 1.7 1.7 blend psram->sram");
    gpu_draw_scale_blend_pattern(&src, &sram_dest, 1.7, 1.7, VG_LITE_BLEND_SRC_OVER);

    src.width = 240;
    src.height = 120;
    src.stride = src.width * 4;
    src.format = VG_LITE_RGBA8888;
    src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    TRACE(3, "blit scale 1.7 1.7 blend psram->sram");
    gpu_draw_scale_blend_pattern(&src, &dest, 1.7, 1.7, VG_LITE_BLEND_SRC_OVER);

    /*
     * blend with rotation(89 degree) by normal quality mode
    */
    vg_lite_matrix_t matrix;

    src.width = 160;  //128;//160;
    src.height = 160; //128;//160;
    src.stride = src.width * 4;
    src.format = VG_LITE_RGBA8888;
    src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    TRACE(3, "blit rotate 89.0 blend psram->sram");
    gpu_blit_rotation_blend(&src, &sram_dest, 89.0, VG_LITE_BLEND_SRC_OVER);
    gpu_blit(&src, &sram_dest, VG_LITE_BLEND_SRC_OVER);

    src.width = 32;
    src.height = 240;
    src.stride = src.width * 4;
    src.format = VG_LITE_RGBA8888;
    src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    TRACE(3, "blit rotate 89.0 blend psram->sram");
    gpu_blit_rotation_blend(&src, &sram_dest, 89.0, VG_LITE_BLEND_SRC_OVER);
    gpu_blit(&src, &sram_dest, VG_LITE_BLEND_SRC_OVER);

    sram_src.width = 32;
    sram_src.height = 240;
    sram_src.stride = src.width * 4;
    sram_src.format = VG_LITE_RGBA8888;
    sram_src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    sram_src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    TRACE(3, "blit rotate 89.0 blend sram->sram");
    gpu_blit_rotation_blend(&sram_src, &sram_dest, 89.0, VG_LITE_BLEND_SRC_OVER);
    gpu_blit(&sram_src, &sram_dest, VG_LITE_BLEND_SRC_OVER);

    /*
     * blend with rotation(89 degree) by high quality mode
    */
    src.width = 32;
    src.height = 240;
    src.stride = src.width * 4;
    src.format = VG_LITE_RGBA8888;
    src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    TRACE(3, "draw pattern 89.0 psram->sram");
    gpu_draw_pattern(&src, &sram_dest, 89.0, VG_LITE_BLEND_SRC_OVER);
    /*
     *blend with matrix (3*3) transform by high quality
    */
    src.width = 240;
    src.height = 240;
    src.stride = 240 * 4;
    src.format = VG_LITE_RGBA8888;
    src.image_mode = VG_LITE_NORMAL_IMAGE_MODE;
    src.transparency_mode = VG_LITE_IMAGE_TRANSPARENT;
    //vg_lite_matrix_t matrix;
    vg_lite_identity(&matrix);
    vg_lite_translate(64, 32, &matrix);
    vg_lite_scale(0.5, 0.5, &matrix);
    vg_lite_rotate(30.0, &matrix);

    TRACE(3, "blit translate scale rotate blend psram->sram");
    gpu_blit_transall_blend(&src, &sram_dest, &matrix, VG_LITE_BLEND_SRC_OVER);

    hal_sys_timer_delay_us(200000);

#endif

#if 0
    src.width  = 240;
    src.height = 240;
    src.stride = 240*4 ;

    sram_src.width  = 80;
    sram_src.height = 80;
    sram_src.stride = 80*4 ;

    dest.width  = 240;
    dest.height = 240;
    dest.stride = 240*4 ;

    sram_dest.width  = 240;
    sram_dest.height = 240;
    sram_dest.stride = 240*4 ;

    vg_lite_float_t rscale = 0.1;
    for (int i = 0; i < 10; i++){
        rscale *= i;
        gpu_blit_scale(&src, &dest, rscale, rscale);
        gpu_blit_scale(&sram_src, &dest, rscale, rscale);
        gpu_blit_scale(&sram_src, &src, rscale, rscale);
        gpu_blit_scale(&sram_src, &sram_dest, rscale, rscale);
        gpu_blit_scale(&src, &sram_src, rscale, rscale);
    }
#endif

#if GPU_PERF_ROTATION_TEST
    vg_lite_float_t degree = 1.0;

    for (int i = 0; i <= 90; i++)
    {
        degree = 4.0 * i;

        TRACE(3, "rotate degree: %d /10", (int)degree * 10);
        gpu_blit_rotation_blend(&src, &dest, degree, VG_LITE_BLEND_SRC_OVER);
        gpu_blit_rotation_blend(&sram_src, &dest, degree, VG_LITE_BLEND_SRC_OVER);
        gpu_blit_rotation_blend(&sram_src, &sram_dest, degree, VG_LITE_BLEND_SRC_OVER);
        gpu_blit_rotation_blend(&src, &sram_dest, degree, VG_LITE_BLEND_SRC_OVER);
        hal_sys_timer_delay_us(200000);
    }
#endif

#if GPU_PERF_ROTATION_TILED_LINEAR_SRC_TEST
    vg_lite_float_t degree = 90.0;
    sram_src.width = 240;
    sram_src.height = 240;
    sram_src.stride = 240 * 4;
    src.width = 240;
    src.height = 240;
    src.stride = 240 * 4;
    //sram_src.height = fb_height;
    TRACE(3, " linear rotate degree: %d /10", (int)degree * 10);

    // gpu_blit_rotation_blend(&src, &dest, degree,VG_LITE_BLEND_SRC_OVER);
    // gpu_blit_rotation_blend(&sram_src, &dest, degree,VG_LITE_BLEND_SRC_OVER);
    gpu_blit_rotation_blend(&sram_src, &sram_dest, degree, VG_LITE_BLEND_SRC_OVER);
    gpu_blit_rotation_blend(&src, &sram_dest, degree, VG_LITE_BLEND_SRC_OVER);
    // hal_sys_timer_delay_us(200000);

    TRACE(3, "tiled src rotate degree: %d /10", (int)degree * 10);
    sram_src.tiled = VG_LITE_TILED;
    src.tiled = VG_LITE_TILED;
    // gpu_blit_rotation_blend(&src, &dest, degree,VG_LITE_BLEND_SRC_OVER);
    // gpu_blit_rotation_blend(&sram_src, &dest, degree,VG_LITE_BLEND_SRC_OVER);
    gpu_blit_rotation_blend(&sram_src, &sram_dest, degree, VG_LITE_BLEND_SRC_OVER);
    gpu_blit_rotation_blend(&src, &sram_dest, degree, VG_LITE_BLEND_SRC_OVER);
    // hal_sys_timer_delay_us(200000);
    test_complete = 0x5a5a5a5a;
    while (test_complete)
        ;

    sram_src.tiled = VG_LITE_LINEAR;
    src.tiled = VG_LITE_LINEAR;
#endif

    hal_sys_timer_delay_us(200000);

#if DMA2D_PERF_BLIT_TEST
    gpu_free_buffer(&src);
    gpu_free_buffer(&dest);
    dma2d_blit_test(DMA2D_PIX_CLK_MHZ);
    gpu_free_buffer(&src);
    gpu_free_buffer(&dest);
#endif
    gpu_close();

    TRACE(3, "test pass");
}

extern int hw_blit(FAR struct dma2d_overlay_s *doverlay,
                   const struct fb_area_s *dareac,
                   FAR struct dma2d_overlay_s *soverlay,
                   FAR const struct fb_area_s *sareac);

static void fb_overlayinfo_setup(struct fb_overlayinfo_s *poinfo,
                                 uint16_t width,
                                 uint16_t height,
                                 uint8_t bpp,
                                 uint32_t *pbuffer)
{
    poinfo->fbmem = pbuffer;
    poinfo->fblen = width * height * bpp / 8;
    poinfo->stride = width * bpp / 8;
    poinfo->bpp = bpp;
}

static void dma2d_overlayinfo_setup(struct dma2d_overlay_s *poverlay,
                                    uint8_t cfmt,
                                    struct fb_overlayinfo_s *pfboverlay)
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

static void set_test_32bitcolor(uint32_t *buffer, uint16_t width, uint16_t height)
{
    int count = width * height;
    uint32_t *p = buffer;
    for (int i = 0; i < count; i++)
    {
        *(p++) = i;
        while (*(p + 1) != i)
        {
            *(p + 1) = i;
        }
    }
}

static void clear_test_32bitcolor(uint32_t *buffer, uint16_t width, uint16_t height)
{
    int count = width * height;
    uint32_t *p = buffer;
    for (int i = 0; i < count; i++)
    {
        *(p++) = 0;
    }
}

static void check_test_32bitcolor(uint32_t *buffer, uint16_t width, uint16_t height)
{
    int count = width * height;
    uint32_t *p = buffer;
    uint32_t v = 0;
    for (int i = 0; i < count; i++)
    {
        v = *(p++);
        if (v != i)
        {
            TRACE(3, "%s fail 0x%x !!!!!! 0x%x", __func__, i, v);
            //break;
            test_complete = 0x5a5a5a5a;
            //while(test_complete);
        }
    }
}

extern void hal_cmu_2d_mod_clock_cfg(uint16_t freqmhz);
extern void print_vcore_info(void);
uint8_t dma2d_cache_sync(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len);
uint8_t dma2d_cache_invalidate(enum HAL_CACHE_ID_T id, uint32_t start_address, uint32_t len);

bool is_lcdc_busy(void);
//{
//    return 0;
//}

void dma2d_blit(vg_lite_buffer_t *vglsrc, vg_lite_buffer_t *vgldest)
{
    static int dma_opened = 0;
    struct dma2d_overlay_s doverlay;
    struct fb_area_s darea;
    struct dma2d_overlay_s soverlay;
    struct fb_area_s sarea;
    struct fb_overlayinfo_s doinfo;
    struct fb_overlayinfo_s soinfo;
    uint32_t *dest_buffer;
    uint32_t *src_buffer;
    if (dma_opened == 0)
    {
        dma_opened = 1;
        dma2dinitialize();
        TRACE(3, "dma2d clk : %d Mhz \r\n", DMA2D_PIX_CLK_MHZ);
    }
    while (is_lcdc_busy())
        ;
    hal_cmu_2d_mod_clock_cfg(DMA2D_PIX_CLK_MHZ);
    dest_buffer = ADDR_DEV_TO_CPU(vgldest->address);
    src_buffer = ADDR_DEV_TO_CPU(vglsrc->address);
    fb_overlayinfo_setup(&doinfo, vgldest->width, vgldest->height, vgldest->stride / vgldest->width * 8, dest_buffer);
    fb_overlayinfo_setup(&soinfo, vglsrc->width, vglsrc->height, vglsrc->stride / vglsrc->width * 8, src_buffer);
    dma2d_overlayinfo_setup(&doverlay, FB_FMT_RGB32, &doinfo);
    dma2d_overlayinfo_setup(&soverlay, FB_FMT_RGB32, &soinfo);

    fb_area_setup(&darea, 0, 0, vgldest->width, vgldest->height);
    fb_area_setup(&sarea, 0, 0, vglsrc->width, vglsrc->height);
    TRACE(3, "blit 8888->8888   src:%p dest:%p", src_buffer, dest_buffer);
    while (hw_blit(&doverlay, &darea, &soverlay, &sarea))
        ;
}

void dma2d_blit_rect(vg_lite_buffer_t *vglsrc, vg_lite_buffer_t *vgldest,
                     struct fb_area_s *sarea, struct fb_area_s *darea)
{
    static int dma_opened = 0;
    struct dma2d_overlay_s doverlay;
    struct dma2d_overlay_s soverlay;
    struct fb_overlayinfo_s doinfo;
    struct fb_overlayinfo_s soinfo;
    uint32_t *dest_buffer;
    uint32_t *src_buffer;
    uint8_t cfmt = 0;

    if (dma_opened == 0)
    {
        dma_opened = 1;
        dma2dinitialize();
        TRACE(3, "dma2d clk : %d Mhz \r\n", DMA2D_PIX_CLK_MHZ);
    }

    while (is_lcdc_busy())
        ;

    hal_cmu_2d_mod_clock_cfg(DMA2D_PIX_CLK_MHZ);
    dest_buffer = ADDR_DEV_TO_CPU(vgldest->address);
    src_buffer = ADDR_DEV_TO_CPU(vglsrc->address);
    fb_overlayinfo_setup(&doinfo, vgldest->width, vgldest->height, vgldest->stride / vgldest->width * 8, dest_buffer);
    fb_overlayinfo_setup(&soinfo, vglsrc->width, vglsrc->height, vglsrc->stride / vglsrc->width * 8, src_buffer);

    if (doinfo.bpp == 32)
    {
        cfmt = FB_FMT_RGB32;
    }
    else if (doinfo.bpp == 24)
    {
        cfmt = FB_FMT_RGB24;
    }
    else if (doinfo.bpp == 16)
    {
        cfmt = FB_FMT_RGB16_565;
    }
    else
    {
        cfmt = FB_FMT_RGB8;
    }

    dma2d_overlayinfo_setup(&doverlay, cfmt, &doinfo);

    if (soinfo.bpp == 32)
    {
        cfmt = FB_FMT_RGB32;
    }
    else if (soinfo.bpp == 24)
    {
        cfmt = FB_FMT_RGB24;
    }
    else if (soinfo.bpp == 16)
    {
        cfmt = FB_FMT_RGB16_565;
    }
    else
    {
        cfmt = FB_FMT_RGB8;
    }

    dma2d_overlayinfo_setup(&soverlay, cfmt, &soinfo);

    TRACE(3, "blit rect bpp:%d|%d src:%p dest:%p", doinfo.bpp, soinfo.bpp, src_buffer, dest_buffer);
    while (hw_blit(&doverlay, darea, &soverlay, sarea))
        ;
}

void dma2d_blit_test(uint16_t pixclk)
{
    struct dma2d_overlay_s doverlay;
    struct fb_area_s darea;
    struct dma2d_overlay_s soverlay;
    struct fb_area_s sarea;
    struct fb_overlayinfo_s doinfo;
    struct fb_overlayinfo_s soinfo;
    uint32_t *dest_buffer;
    uint32_t *src_buffer;

    src.width = 480;
    src.height = 240;
    src.format = VG_LITE_RGBA8888;

    dest.width = 480;
    dest.height = 240;
    dest.format = VG_LITE_RGBA8888;
    TRACE(3, "%s \r\n", __func__);

    gpu_allocate_buffer(&src);
    gpu_allocate_buffer(&dest);
    dma2dinitialize();

    TRACE(3, "dma2d clk : %d Mhz \r\n", pixclk);

    hal_cmu_2d_mod_clock_cfg(pixclk);
    //while(1)
    {

       // blit from sram to sram argb8888 -> rgb888
        dest_buffer = sram_buffer_dest;
        src_buffer = sram_buffer;
        fb_overlayinfo_setup(&doinfo, 480, 240, 24, dest_buffer);
        fb_overlayinfo_setup(&soinfo, 480, 240, 32, src_buffer);
        dma2d_overlayinfo_setup(&doverlay, FB_FMT_RGB24, &doinfo);
        dma2d_overlayinfo_setup(&soverlay, FB_FMT_RGB32, &soinfo);

        fb_area_setup(&darea, 0, 0, 480, 240);
        fb_area_setup(&sarea, 0, 0, 480, 240);

        TRACE(3, "blit sram->sram  8888->888");
        TRACE(3, "blit sram->sram  8888->888   src:%p dest:%p", src_buffer, dest_buffer);
        hw_blit(&doverlay, &darea, &soverlay, &sarea);

        // blit from sram to sram argb8888 -> argb8888

        //clear_test_32bitcolor(sram_buffer, 480, 240);
        //check_test_32bitcolor(sram_buffer, 480, 240);

        dest_buffer = sram_buffer_dest;
        src_buffer = sram_buffer;
        set_test_32bitcolor(sram_buffer, 480, 240);
        fb_overlayinfo_setup(&doinfo, 480, 240, 32, dest_buffer);
        fb_overlayinfo_setup(&soinfo, 480, 240, 32, src_buffer);
        dma2d_overlayinfo_setup(&doverlay, FB_FMT_RGB32, &doinfo);
        dma2d_overlayinfo_setup(&soverlay, FB_FMT_RGB32, &soinfo);

        fb_area_setup(&darea, 0, 0, 480, 240);
        fb_area_setup(&sarea, 0, 0, 480, 240);

        TRACE(3, "blit sram->sram  8888->8888   src:%p dest:%p", src_buffer, dest_buffer);
        hw_blit(&doverlay, &darea, &soverlay, &sarea);
        check_test_32bitcolor(dest_buffer, 480, 240);

        //hal_sys_timer_delay_us(100000);
        osDelay(5);
        // blit from psram to sram argb8888 -> rgb888

        dest_buffer = sram_buffer_dest;
        src_buffer = ADDR_DEV_TO_CPU(src.address);
        fb_overlayinfo_setup(&doinfo, 480, 240, 24, dest_buffer);
        fb_overlayinfo_setup(&soinfo, 480, 240, 32, src_buffer);
        dma2d_overlayinfo_setup(&doverlay, FB_FMT_RGB24, &doinfo);
        dma2d_overlayinfo_setup(&soverlay, FB_FMT_RGB32, &soinfo);

        fb_area_setup(&darea, 0, 0, 480, 240);
        fb_area_setup(&sarea, 0, 0, 480, 240);

        TRACE(3, "blit psram->sram  8888->888   src:%p dest:%p", src_buffer, dest_buffer);
        hw_blit(&doverlay, &darea, &soverlay, &sarea);

        // blit from psram to sram argb8888 -> argb8888

        dest_buffer = sram_buffer_dest;
        src_buffer = ADDR_DEV_TO_CPU(src.address);
        fb_overlayinfo_setup(&doinfo, 480, 240, 32, dest_buffer);
        fb_overlayinfo_setup(&soinfo, 480, 240, 32, src_buffer);
        dma2d_overlayinfo_setup(&doverlay, FB_FMT_RGB32, &doinfo);
        dma2d_overlayinfo_setup(&soverlay, FB_FMT_RGB32, &soinfo);

        fb_area_setup(&darea, 0, 0, 480, 240);
        fb_area_setup(&sarea, 0, 0, 480, 240);

        TRACE(3, "blit psram->sram  8888->8888   src:%p dest:%p", src_buffer, dest_buffer);
        hw_blit(&doverlay, &darea, &soverlay, &sarea);

        // blit from sram to psram argb8888 -> rgb8888

        dest_buffer = ADDR_DEV_TO_CPU(dest.address);
        src_buffer = sram_buffer;
        fb_overlayinfo_setup(&doinfo, 480, 240, 32, dest_buffer);
        fb_overlayinfo_setup(&soinfo, 480, 240, 32, src_buffer);
        dma2d_overlayinfo_setup(&doverlay, FB_FMT_RGB32, &doinfo);
        dma2d_overlayinfo_setup(&soverlay, FB_FMT_RGB32, &soinfo);

        fb_area_setup(&darea, 0, 0, 480, 240);
        fb_area_setup(&sarea, 0, 0, 480, 240);

        TRACE(3, "blit sram->psram  8888->8888   src:%p dest:%p", src_buffer, dest_buffer);
        hw_blit(&doverlay, &darea, &soverlay, &sarea);

        // blit from psram to psram argb8888 -> rgb888

        dest_buffer = ADDR_DEV_TO_CPU(dest.address);
        src_buffer = ADDR_DEV_TO_CPU(src.address);
        fb_overlayinfo_setup(&doinfo, 480, 240, 24, dest_buffer);
        fb_overlayinfo_setup(&soinfo, 480, 240, 32, src_buffer);
        dma2d_overlayinfo_setup(&doverlay, FB_FMT_RGB24, &doinfo);
        dma2d_overlayinfo_setup(&soverlay, FB_FMT_RGB32, &soinfo);

        fb_area_setup(&darea, 0, 0, 480, 240);
        fb_area_setup(&sarea, 0, 0, 480, 240);

        TRACE(3, "blit psram->psram  8888->888   src:%p dest:%p", src_buffer, dest_buffer);
        hw_blit(&doverlay, &darea, &soverlay, &sarea);

        // blit from psram to psram argb8888 -> argb8888

        dest_buffer = ADDR_DEV_TO_CPU(dest.address);
        src_buffer = sram_buffer;

        fb_overlayinfo_setup(&doinfo, 480, 240, 32, dest_buffer);
        fb_overlayinfo_setup(&soinfo, 480, 240, 32, src_buffer);
        // fb_overlayinfo_setup(&doinfo, 480, 120, 32, dest_buffer);
        // fb_overlayinfo_setup(&soinfo, 480, 120, 32, src_buffer);
        dma2d_overlayinfo_setup(&doverlay, FB_FMT_RGB32, &doinfo);
        dma2d_overlayinfo_setup(&soverlay, FB_FMT_RGB32, &soinfo);

        set_test_32bitcolor(src_buffer, 480, 240);
        //set_test_32bitcolor(src_buffer, 480, 120);
        //check_test_32bitcolor(src_buffer, 480, 240);

        //TRACE(0,"%s %d",__func__,__LINE__);
        //while(1);

        dma2d_cache_sync(HAL_CACHE_ID_D_CACHE, src_buffer, 480 * 240 * 4);

        //clear_test_32bitcolor(dest_buffer, 480, 120);
        clear_test_32bitcolor(dest_buffer, 480, 240);
        dma2d_cache_sync(HAL_CACHE_ID_D_CACHE, dest_buffer, 480 * 240 * 4);

        fb_area_setup(&darea, 0, 0, 480, 240);
        fb_area_setup(&sarea, 0, 0, 480, 240);

        //fb_area_setup(&darea, 0,0,480,120);
        //fb_area_setup(&sarea, 0,0,480,120);
        //fb_area_setup(&darea, 0,0,32,1);
        //fb_area_setup(&sarea, 0,0,32,1);
        TRACE(3, "blit sram->psram  8888->8888   src:%p dest:%p", src_buffer, dest_buffer);
        while (
            hw_blit(&doverlay, &darea, &soverlay, &sarea));

        dma2d_cache_invalidate(HAL_CACHE_ID_D_CACHE, dest_buffer, 480 * 240 * 4);
        check_test_32bitcolor(dest_buffer, 480, 240);
        //check_test_32bitcolor(dest_buffer, 480, 120);

        while (1)
            ;
        osDelay(100);
    }
}

void gpu_current_test(void)
{
#ifdef GPU_CURRENT_TEST_CPU_WHILE1
    goto test_done;
#endif
    gpu_open();
    gpu_gpio_open();
#ifdef GPU_CURRENT_TEST_GPU_ENABLE
    gpu_clks_enter_gating();
    goto test_done;
#endif

    src.width = fb_width;
    src.height = fb_height;
    src.format = VG_LITE_RGBA8888;

    dest.width = fb_width;
    dest.height = fb_height;
    dest.format = VG_LITE_RGBA8888;

    sram_src.width = fb_width;
    sram_src.height = fb_height;
    sram_src.format = VG_LITE_RGBA8888;

    sram_dest.width = fb_width;
    sram_dest.height = fb_height;
    sram_dest.format = VG_LITE_RGBA8888;

    gpu_allocate_buffer(&src);
    gpu_allocate_buffer(&dest);

    memcpy(&sram_src, &src, sizeof(vg_lite_buffer_t));
    sram_src.memory = sram_buffer;
    sram_src.address = ADDR_CPU_TO_DEV(sram_src.memory);
    memcpy(&sram_dest, &src, sizeof(vg_lite_buffer_t));
    sram_dest.memory = sram_buffer_dest;
    sram_dest.address = ADDR_CPU_TO_DEV(sram_dest.memory);

    sram_src.format = VG_LITE_RGBA8888;
    TRACE(3, "blit rgba8888->rgba8888 sram->sram once");
    gpu_blit(&sram_src, &sram_dest, VG_LITE_BLEND_NONE);

#ifdef GPU_CURRENT_TEST_GPU_BLIT_ONCE
    goto test_done;
#endif

#ifdef GPU_CURRENT_TEST_GPU_BLIT_60HZ
    sram_src.format = VG_LITE_RGBA8888;
    TRACE(3, "blit rgba8888->rgba8888 sram->sram 60hz");
    while (1)
    {
        gpu_blit(&sram_src, &sram_dest, VG_LITE_BLEND_NONE);
        hal_sys_timer_delay(MS_TO_TICKS(16));
    }
#endif

#ifdef GPU_CURRENT_TEST_GPU_BLIT_1HZ
    sram_src.format = VG_LITE_RGBA8888;
    TRACE(3, "blit rgba8888->rgba8888 sram->sram 1hz");
    while (1)
    {
        gpu_gpio_set(0);
        gpu_blit(&sram_src, &sram_dest, VG_LITE_BLEND_NONE);
        gpu_gpio_clr(0);

        gpu_clks_enter_gating();
        hal_sys_timer_delay(MS_TO_TICKS(1000));
        gpu_clks_exit_gating();
        TRACE(0, "blit ......");
    }
#endif

#ifdef GPU_CURRENT_TEST_GPU_BLIT_LOOP
    sram_src.format = VG_LITE_RGBA8888;
    TRACE(3, "blit rgba8888->rgba8888 sram->sram LOOP");
    while (1)
    {
        gpu_blit(&sram_src, &sram_dest, VG_LITE_BLEND_NONE);
    }
#endif

test_done:

    TRACE(3, "test b while");

    while (1)
    {
        osDelay(20000);
    }

    TRACE(3, "test b gpu close");
    gpu_close();
    TRACE(3, "test a gpu close");

    TRACE(3, "test pass");
}
