
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <math.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "hal_trace.h"
#include "hal_timer.h"
#include "hal_location.h"
#include "gpu_port.h"
#include "gpu_common.h"

#ifdef RTOS
#include "cmsis_os.h"
#endif
#define GPU_REP(...)   TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define GPU_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)

#define printf GPU_ERR

#if DDRLESS_FPGA
#include "grad.h"
#endif

#define DEFAULT_SIZE   320.0f;

#define DEFAULT_WIDTH 320
#define DEFAULT_HEIGHT 480

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
//static int fb_width = DEFAULT_WIDTH, fb_height = DEFAULT_HEIGHT;
static int fb_width = GPU_FB_WIDTH, fb_height = GPU_FB_HEIGHT;
static float fb_scale = 1.0f;
int frames = 1; //Frames to render

static vg_lite_buffer_t buffer;     //offscreen framebuffer object for rendering.
static vg_lite_buffer_t * fb;
static vg_lite_buffer_t image;

/*
 *-----*
 /       \
 /         \
 *           *
 |          /
 |         X
 |          \
 *           *
 \         /
 \       /
 *-----*
 */
static char path_data[] = {
    2, -5, -10, // moveto   -5,-10
    4, 5, -10,  // lineto    5,-10
    4, 10, -5,  // lineto   10, -5
    4, 0, 0,    // lineto    0,  0
    4, 10, 5,   // lineto   10,  5
    4, 5, 10,   // lineto    5, 10
    4, -5, 10,  // lineto   -5, 10
    4, -10, 5,  // lineto  -10,  5
    4, -10, -5, // lineto  -10, -5
    0, // end
};

static vg_lite_path_t path = {
    {-10, -10, // left,top
        10, 10}, // right,bottom
    VG_LITE_HIGH, // quality
    VG_LITE_S8, // -128 to 127 coordinate range
    {0}, // uploaded
    sizeof(path_data), // path length
    path_data, // path data
    1
};

static  void cleanup(void)
{
    if (buffer.handle != NULL) {
        // Free the buffer memory.
        vg_lite_free(&buffer);
    }

    if (image.handle != NULL) {
        // Free the image memory.
        vg_lite_free(&image);
    }

    vg_lite_close();
}

int vg_lite_load_raw_buffer(vg_lite_buffer_t * buffer, const char * raw);
extern uint32_t __landscape_start_flash[];
extern uint32_t __landscape_end_flash[];

int pattern_fill_test_main(int argc, const char * argv[])
{
    int i = 0;
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix, matPath;

    /* Initialize vg_lite engine. */
    vg_lite_error_t error = VG_LITE_SUCCESS;
    char filename[30];

    TC_INIT
    GPU_REP("%s", __func__);

    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    filter = VG_LITE_FILTER_POINT;
#ifdef __TESTWITH_LOCALFS__
    /* Load the image. */
    if (vg_lite_load_raw(&image, "landscape.raw") != 0) {
        printf("load raw file error\n");
        cleanup();
        return -1;
    }
#else
    vg_lite_load_raw_buffer(&image,__landscape_start_flash);
#endif

    // Allocate the off-screen buffer.
    buffer.width  = fb_width;
    buffer.height = fb_height;
   // buffer.format = VG_LITE_RGB565;
    buffer.format = GPU_FB_FORMAT;

    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    fb_scale = (float)fb_width / DEFAULT_SIZE;

    printf("Framebuffer size: %d x %d\n", fb_width, fb_height);

    while (i < frames)
    {
        // Clear the buffer with blue.
        CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));
        // Build a 33 degree rotation matrix from the center of the buffer.
        vg_lite_identity(&matrix);
        vg_lite_translate(fb_width / 2.0f, fb_height / 4.0f, &matrix);
        vg_lite_rotate(33.0f, &matrix);
        vg_lite_scale(0.4f, 0.4f, &matrix);
        vg_lite_translate(fb_width / -2.0f, fb_height / -4.0f, &matrix);
        vg_lite_identity(&matPath);
        vg_lite_translate(fb_width / 2.0f, fb_height / 4.0f, &matPath);
        vg_lite_scale(10, 10, &matPath);

        //vg_lite_blit(&image, &matrix, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
        // Fill the path using an image.
        CHECK_ERROR(vg_lite_draw_pattern(fb, &path, VG_LITE_FILL_EVEN_ODD, &matPath, &image, &matrix, VG_LITE_BLEND_NONE, VG_LITE_PATTERN_COLOR, 0xffaabbcc, filter));

        vg_lite_identity(&matrix);
        vg_lite_translate(fb_width / 2.0f, fb_height / 1.2f, &matrix);
        vg_lite_rotate(33.0f, &matrix);
        vg_lite_scale(0.4f, 0.4f, &matrix);
        vg_lite_translate(fb_width / -1.5f, fb_height / -2.0f, &matrix);
        vg_lite_identity(&matPath);
        vg_lite_translate(fb_width / 2.0f, fb_height / 1.3f, &matPath);
        vg_lite_scale(10, 10, &matPath);

        TC_START

        CHECK_ERROR(vg_lite_draw_pattern(fb, &path, VG_LITE_FILL_EVEN_ODD, &matPath, &image, &matrix, VG_LITE_BLEND_NONE, VG_LITE_PATTERN_PAD, 0xffaabbcc, filter));
        CHECK_ERROR(vg_lite_finish());

        TC_END
        TC_REP(draw_pattern_cost);
        // Save fb raw file.
        sprintf(filename, "patternFill%d.raw",i);

        gpu_save_raw(filename, fb);

        GPU_REP("%s dump mem w:%d h:%d", __func__, fb->width,fb->height);
        GPU_REP("%s dump mem start:%p  end:%p", __func__, fb->memory, (uint32_t)(fb->memory) + fb->height * fb->stride );

#ifdef PLD_PSRAMDUMP
        set_pld_dump_mem_description(fb->memory, fb->height * fb->stride, i, 1);
        trigger_pld_dump_mem();
#endif
        gpu_lcdc_display(fb);
        printf("frame:%d Done!\n",i);
        i++;
    }

ErrorHandler:
    // Cleanup.
    cleanup();
    return 0;
}
