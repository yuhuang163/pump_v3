#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include "vg_lite.h"
#include "vg_lite_util.h"
#include "hal_trace.h"
#include "gpu_port.h"
#include "gpu_common.h"

#define GPU_REP(...)   TR_INFO( TR_MOD(TEST), "//" __VA_ARGS__)
#define GPU_ERR(...)   TR_ERROR( TR_MOD(TEST), "//" __VA_ARGS__)

#define DEFAULT_SIZE   454.0f;
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

static uint8_t sides_cmd[] = {
    VLC_OP_MOVE,
    VLC_OP_SCCWARC,
    VLC_OP_LINE,
    VLC_OP_SCCWARC,
    VLC_OP_LINE,
    VLC_OP_SCCWARC,
    VLC_OP_LINE,
    VLC_OP_SCCWARC,

    VLC_OP_END
};

float sides_data_left[] = {
    50, 0,
    50, 50, 0, 0, 50,
    0, 100,
    50, 50, 0, 50, 150,
    200, 150,
    50, 50, 0, 250, 100,
    250, 50,
    50, 50, 0, 200, 0
};

static vg_lite_path_t path;
static void cleanup(void)
{
    if (buffer.handle != NULL) {
        vg_lite_free(&buffer);
    }

    vg_lite_clear_path(&path);

    vg_lite_close();
}

int arcpath_test_main_entry(int argc, const char * argv[])
{
    uint32_t feature_check = 0;
    vg_lite_filter_t filter;
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_matrix_t matrix;
    uint32_t data_size;
    /* Initialize the draw. */
    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

    filter = VG_LITE_FILTER_POINT;

    fb_scale = (float)fb_width / DEFAULT_SIZE;
    printf("Framebuffer size: %d x %d\n", fb_width, fb_height);

    /* Allocate the off-screen buffer. */
    buffer.width  = fb_width;
    buffer.height = fb_height;
    buffer.format = GPU_FB_FORMAT;
    CHECK_ERROR(vg_lite_allocate(&buffer));
    fb = &buffer;

    CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));

    vg_lite_identity(&matrix);

    data_size = vg_lite_path_calc_length(sides_cmd, sizeof(sides_cmd), VG_LITE_FP32);
    vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0);
    path.path = malloc(data_size);
    CHECK_ERROR(vg_lite_path_append(&path, sides_cmd, sides_data_left, sizeof(sides_cmd)));

    CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, 0xFF0000FF));

    CHECK_ERROR(vg_lite_finish());
    gpu_lcdc_display(fb);
    gpu_save_raw("arcpath.raw", fb);

ErrorHandler:
    cleanup();
    return 0;
}
