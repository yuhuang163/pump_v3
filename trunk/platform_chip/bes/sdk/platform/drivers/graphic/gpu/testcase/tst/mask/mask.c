/*
 Resolution: 480 x 480
 Format: VG_LITE_RGBA8888
 Mask Operation: VG_CLEAR_MASK/VG_FILL_MASK/VG_SET_MASK/VG_UNION_MASK/VG_INTERSECT_MASK/VG_SUBTRACT_MASK
 Related APIs: vg_lite_create_mask_layer/vg_lite_fill_mask_layer/vg_lite_blend_mask_layer/vg_lite_generate_mask_layer_by_path/
               vg_lite_enable_mask/vg_lite_blit/vg_lite_draw/vg_lite_set_mask_layer
 Description: Test mask function.
 Image filter type is selected by hardware feature gcFEATURE_BIT_VG_IM_FILTER(ON: VG_LITE_FILTER_BI_LINEAR, OFF: VG_LITE_FILTER_POINT).
 */
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

#undef malloc
#define malloc  gpu_malloc
#undef free
#define free  gpu_free



#define DEFAULT_SIZE   480.0f;
#define __func__ __FUNCTION__
char *error_type[] =
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

static int   fb_width = 480, fb_height = 480;
static float fb_scale = 1.0f;

static vg_lite_buffer_t * fb;
static vg_lite_buffer_t image;
static vg_lite_mask_operation_t operations[] = {
    VG_CLEAR_MASK,
    VG_FILL_MASK,
    VG_SET_MASK,
    VG_UNION_MASK,
    VG_INTERSECT_MASK,
    VG_SUBTRACT_MASK,
};
static vg_lite_buffer_t buffer[sizeof(operations)/sizeof(operations[0])];

static uint8_t sides_cmd[] = {
    VLC_OP_MOVE,
    VLC_OP_SCWARC,
    VLC_OP_LINE,
    VLC_OP_SCWARC,
    VLC_OP_LINE,
    VLC_OP_SCWARC,
    VLC_OP_LINE,
    VLC_OP_SCWARC,

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
    int i;
    if (image.handle != NULL) {
        vg_lite_free(&image);
    }
    for (i = 0; i < sizeof(operations)/sizeof(operations[0]); i++) {
        if (buffer[i].handle != NULL) {
            vg_lite_free(&buffer[i]);
        }
    }
    vg_lite_clear_path(&path);
    vg_lite_close();
}


int vg_lite_load_raw_buffer(vg_lite_buffer_t * buffer, const char * raw);
extern uint32_t __landscape_start_flash[];
extern uint32_t __landscape_end_flash[];
static volatile int wait_dump = 1;

int mask_test_main_entry(int argc, const char * argv[])
{
    vg_lite_filter_t filter;
    vg_lite_matrix_t matrix;
    vg_lite_buffer_t mask_layer1;
    vg_lite_buffer_t mask_layer2;
    char fname[64];
    vg_lite_error_t error;
    int i;
    uint32_t data_size;
    filter = VG_LITE_FILTER_POINT;
    error = VG_LITE_SUCCESS;

    CHECK_ERROR(vg_lite_init(fb_width, fb_height));

#ifdef __TESTWITH_LOCALFS__
    if (vg_lite_load_raw(&image, "landscape.raw") != 0) {
        printf("load raw file error\n");
        cleanup();
        return -1;
    }
#else
    vg_lite_load_raw_buffer(&image,__landscape_start_flash);
#endif


    fb_scale = (float)fb_width / DEFAULT_SIZE;

    for(i = 0; i < sizeof(operations)/sizeof(operations[0]); i++) {

        buffer[i].width  = fb_width;
        buffer[i].height = fb_height;
        buffer[i].format = VG_LITE_RGBA8888;

        CHECK_ERROR(vg_lite_allocate(&buffer[i]));
        fb = &buffer[i];

        CHECK_ERROR(vg_lite_create_mask_layer(&mask_layer1, fb_width, fb_height));
        CHECK_ERROR(vg_lite_create_mask_layer(&mask_layer2, fb_width, fb_height));

        CHECK_ERROR(vg_lite_fill_mask_layer(&mask_layer1, 0, 0, fb_width/1.5, fb_height/1.5, 0x55));
        CHECK_ERROR(vg_lite_fill_mask_layer(&mask_layer2, 0, 0, fb_width, fb_height, 0x66));
        CHECK_ERROR(vg_lite_blend_mask_layer(&mask_layer1, &mask_layer2, operations[i], fb_width/3, fb_width/3, fb_width/5, fb_height/5));

        vg_lite_identity(&matrix);
        vg_lite_translate(fb_width/2, fb_width/2, &matrix);
        data_size = vg_lite_path_calc_length(sides_cmd, sizeof(sides_cmd), VG_LITE_FP32);
        CHECK_ERROR(vg_lite_init_path(&path, VG_LITE_FP32, VG_LITE_HIGH, data_size, NULL, 0, 0, 0, 0));
        path.path = malloc(data_size);
        CHECK_ERROR(vg_lite_path_append(&path, sides_cmd, sides_data_left, sizeof(sides_cmd)));

        CHECK_ERROR(vg_lite_set_stroke(&path, VG_LITE_CAP_ROUND, VG_LITE_JOIN_MITER, 4, 8, NULL, 0, 8, 0));
        CHECK_ERROR(vg_lite_update_stroke(&path));
        CHECK_ERROR(vg_lite_set_draw_path_type(&path, VG_LITE_DRAW_FILL_PATH));

        CHECK_ERROR(vg_lite_generate_mask_layer_by_path(&mask_layer1, operations[i], &path, VG_LITE_FILL_EVEN_ODD, 0x88, &matrix));

        CHECK_ERROR(vg_lite_enable_mask(1));
        CHECK_ERROR(vg_lite_set_mask_layer(&mask_layer1));

        CHECK_ERROR(vg_lite_clear(fb, NULL, 0xFFFF0000));
        vg_lite_identity(&matrix);
        vg_lite_scale(2, 2 ,&matrix);
        CHECK_ERROR(vg_lite_blit(fb, &image, &matrix, VG_LITE_BLEND_NONE, 0, filter));
        vg_lite_identity(&matrix);
        vg_lite_translate(240, 0, &matrix);
        CHECK_ERROR(vg_lite_draw(fb, &path, VG_LITE_FILL_EVEN_ODD, &matrix, VG_LITE_BLEND_NONE, 0xFF0000FF));
        CHECK_ERROR(vg_lite_finish());

        GPU_REP("%s fb mem w:%d h:%d", __func__, fb->width,fb->height);
        GPU_REP("%s fb mem start:%p  end:%p", __func__, fb->memory, (uint32_t)(fb->memory) + fb->height * fb->stride );

#ifdef PLD_PSRAMDUMP
        set_pld_dump_mem_description(fb->memory, fb->height * fb->stride, i, 1);
        trigger_pld_dump_mem();
#else
        sprintf(fname, "mask_%d.raw", i);
        vg_lite_save_raw(fname, fb);
#endif
        vg_lite_free(&buffer[i]);

        CHECK_ERROR(vg_lite_enable_mask(0));
        CHECK_ERROR(vg_lite_destroy_mask_layer(&mask_layer1));
        CHECK_ERROR(vg_lite_destroy_mask_layer(&mask_layer2));
    }

ErrorHandler:
    cleanup();
    return 0;
}
