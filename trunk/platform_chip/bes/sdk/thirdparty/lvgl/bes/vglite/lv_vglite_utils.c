/**
 * @file lv_vglite_utils.c
 *
 */

/**
 * Copyright 2022-2024 NXP
 *
 * SPDX-License-Identifier: MIT
 */

/*********************
 *      INCLUDES
 *********************/

#include "lv_vglite_utils.h"

#if LV_USE_DRAW_BES_VGLITE
#include "lv_vglite_buf.h"

/*********************
 *      DEFINES
 *********************/

/**********************
 *      TYPEDEFS
 **********************/

/**********************
 *  STATIC PROTOTYPES
 **********************/

/**********************
 *  STATIC VARIABLES
 **********************/

/**********************
 *      MACROS
 **********************/

/**********************
 *   GLOBAL FUNCTIONS
 **********************/

const char * vglite_error_to_string(vg_lite_error_t error)
{
    switch(error) {
            ENUM_TO_STRING(VG_LITE_SUCCESS);
            ENUM_TO_STRING(VG_LITE_INVALID_ARGUMENT);
            ENUM_TO_STRING(VG_LITE_OUT_OF_MEMORY);
            ENUM_TO_STRING(VG_LITE_NO_CONTEXT);
            ENUM_TO_STRING(VG_LITE_TIMEOUT);
            ENUM_TO_STRING(VG_LITE_OUT_OF_RESOURCES);
            ENUM_TO_STRING(VG_LITE_GENERIC_IO);
            ENUM_TO_STRING(VG_LITE_NOT_SUPPORT);
            ENUM_TO_STRING(VG_LITE_ALREADY_EXISTS);
            ENUM_TO_STRING(VG_LITE_NOT_ALIGNED);
            ENUM_TO_STRING(VG_LITE_FLEXA_TIME_OUT);
            ENUM_TO_STRING(VG_LITE_FLEXA_HANDSHAKE_FAIL);
        default:
            break;
    }

    return "VG_LITE_UKNOWN_ERROR";
}

void vglite_run(void)
{
    VGLITE_CHECK_ERROR(vg_lite_finish());
}

vg_lite_color_t vglite_get_color(lv_color32_t lv_col32, bool gradient)
{
    vg_lite_color_t vg_col32;

    /* Pre-multiply alpha */
    lv_col32.red = LV_UDIV255(lv_col32.red * lv_col32.alpha);
    lv_col32.green = LV_UDIV255(lv_col32.green * lv_col32.alpha);
    lv_col32.blue = LV_UDIV255(lv_col32.blue * lv_col32.alpha);

    if(!gradient)
        /* The color is in ABGR8888 format with red channel in the lower 8 bits. */
        vg_col32 = ((vg_lite_color_t)lv_col32.alpha << 24) | ((vg_lite_color_t)lv_col32.blue << 16) |
                   ((vg_lite_color_t)lv_col32.green << 8) | (vg_lite_color_t)lv_col32.red;
    else
        /* The gradient color is in ARGB8888 format with blue channel in the lower 8 bits. */
        vg_col32 = ((vg_lite_color_t)lv_col32.alpha << 24) | ((vg_lite_color_t)lv_col32.red << 16) |
                   ((vg_lite_color_t)lv_col32.green << 8) | (vg_lite_color_t)lv_col32.blue;

    return vg_col32;
}

vg_lite_blend_t vglite_get_blend_mode(lv_blend_mode_t lv_blend_mode)
{
    vg_lite_blend_t vg_blend_mode = VG_LITE_BLEND_NONE;

    if(vg_lite_query_feature(gcFEATURE_BIT_VG_LVGL_SUPPORT)) {
        switch(lv_blend_mode) {
            case LV_BLEND_MODE_NORMAL:
                vg_blend_mode = VG_LITE_BLEND_NORMAL_LVGL;
                break;
            case LV_BLEND_MODE_ADDITIVE:
                vg_blend_mode = VG_LITE_BLEND_ADDITIVE_LVGL;
                break;
            case LV_BLEND_MODE_SUBTRACTIVE:
                vg_blend_mode = VG_LITE_BLEND_SUBTRACT_LVGL;
                break;
            case LV_BLEND_MODE_MULTIPLY:
                vg_blend_mode = VG_LITE_BLEND_MULTIPLY_LVGL;
                break;
            default:
                VGLITE_ASSERT_MSG(false, "Unsupported blend mode.");
                break;
        }
    }
    else {
        switch(lv_blend_mode) {
            case LV_BLEND_MODE_NORMAL:
                vg_blend_mode = VG_LITE_BLEND_SRC_OVER;
                break;
            case LV_BLEND_MODE_ADDITIVE:
                vg_blend_mode = VG_LITE_BLEND_ADDITIVE;
                break;
            case LV_BLEND_MODE_SUBTRACTIVE:
                vg_blend_mode = VG_LITE_BLEND_SUBTRACT;
                break;
            case LV_BLEND_MODE_MULTIPLY:
                vg_blend_mode = VG_LITE_BLEND_MULTIPLY;
                break;
            default:
                VGLITE_ASSERT_MSG(false, "Unsupported blend mode.");
                break;
        }
    }

    return vg_blend_mode;
}

vg_lite_buffer_format_t vglite_get_buf_format(lv_color_format_t cf)
{
    vg_lite_buffer_format_t vg_buffer_format = VG_LITE_BGR565;

    switch(cf) {
        case LV_COLOR_FORMAT_L8:
            vg_buffer_format = VG_LITE_L8;
            break;
        case LV_COLOR_FORMAT_A4:
            vg_buffer_format = VG_LITE_A4;
            break;
        case LV_COLOR_FORMAT_A8:
            vg_buffer_format = VG_LITE_A8;
            break;
        case LV_COLOR_FORMAT_I1:
            vg_buffer_format = VG_LITE_INDEX_1;
            break;
        case LV_COLOR_FORMAT_I2:
            vg_buffer_format = VG_LITE_INDEX_2;
            break;
        case LV_COLOR_FORMAT_I4:
            vg_buffer_format = VG_LITE_INDEX_4;
            break;
        case LV_COLOR_FORMAT_I8:
            vg_buffer_format = VG_LITE_INDEX_8;
            break;
        case LV_COLOR_FORMAT_RGB565:
            vg_buffer_format = VG_LITE_BGR565;
            break;
        case LV_COLOR_FORMAT_YUY2:
            vg_buffer_format = VG_LITE_YUY2;
            break;
        case LV_COLOR_FORMAT_RGB565A8:
            vg_buffer_format = VG_LITE_ABGR8565;
            break;
        case LV_COLOR_FORMAT_RGB888:
            vg_buffer_format = VG_LITE_BGR888;
            break;
        case LV_COLOR_FORMAT_ARGB8888:
            vg_buffer_format = VG_LITE_BGRA8888;
            break;
        case LV_COLOR_FORMAT_XRGB8888:
            vg_buffer_format = VG_LITE_BGRX8888;
            break;

        default:
            VGLITE_ASSERT_MSG(false, "Unsupported color format.");
            break;
    }

    return vg_buffer_format;
}

uint32_t vglite_get_buf_stride(lv_color_format_t cf, uint32_t w)
{
    switch(cf) {
        case LV_COLOR_FORMAT_A4:
            return w / 2;
        case LV_COLOR_FORMAT_L8:
        case LV_COLOR_FORMAT_A8:
        case LV_COLOR_FORMAT_I1:
        case LV_COLOR_FORMAT_I2:
        case LV_COLOR_FORMAT_I4:
        case LV_COLOR_FORMAT_I8:
            return w;
        case LV_COLOR_FORMAT_RGB565:
        case LV_COLOR_FORMAT_YUY2:
            return w * 2;
        case LV_COLOR_FORMAT_RGB565A8:
        case LV_COLOR_FORMAT_RGB888:
            return w * 3;
        case LV_COLOR_FORMAT_ARGB8888:
        case LV_COLOR_FORMAT_XRGB8888:
            return w * 4;

        default:
            return w;
    }
    return w;
}

bool vglite_buf_aligned(const void * buf, uint32_t width, uint32_t stride, lv_color_format_t cf)
{
    uint8_t buffer_align, stride_align;

    if (vg_lite_query_feature(gcFEATURE_BIT_VG_16PIXELS_ALIGN)) {
        switch(cf) {
            case LV_COLOR_FORMAT_A4:
            case LV_COLOR_FORMAT_I1:
            case LV_COLOR_FORMAT_I2:
            case LV_COLOR_FORMAT_I4:
                buffer_align = 8;
                stride_align = 8;
                break;
            case LV_COLOR_FORMAT_L8:
            case LV_COLOR_FORMAT_A8:
            case LV_COLOR_FORMAT_I8:
                buffer_align = 16;
                stride_align = 16;
                break;
            case LV_COLOR_FORMAT_RGB565:
            case LV_COLOR_FORMAT_YUY2:
                buffer_align = 32;
                stride_align = 32;
                break;
            case LV_COLOR_FORMAT_RGB565A8:
            case LV_COLOR_FORMAT_RGB888:
            case LV_COLOR_FORMAT_ARGB8888:
            case LV_COLOR_FORMAT_XRGB8888:
            default:
                buffer_align = 64;
                stride_align = 64;
                break;
        }
    } else {
        switch(cf) {
            case LV_COLOR_FORMAT_A4:
            case LV_COLOR_FORMAT_L8:
            case LV_COLOR_FORMAT_A8:
            case LV_COLOR_FORMAT_I1:
            case LV_COLOR_FORMAT_I2:
            case LV_COLOR_FORMAT_I4:
            case LV_COLOR_FORMAT_I8:
                buffer_align = 8;
                stride_align = 1;
                break;
            case LV_COLOR_FORMAT_RGB565:
                buffer_align = 8;
                stride_align = 2;
                break;
            case LV_COLOR_FORMAT_YUY2:
                buffer_align = 8;
                stride_align = 4;
                break;
            case LV_COLOR_FORMAT_RGB565A8:
            case LV_COLOR_FORMAT_RGB888:
                buffer_align = 8;
                stride_align = 3;
                break;
            case LV_COLOR_FORMAT_ARGB8888:
            case LV_COLOR_FORMAT_XRGB8888:
            default:
                buffer_align = 16;
                stride_align = 4;
                break;
        }
    }

    if (stride == 0)
        stride = vglite_get_buf_stride(cf, width);

    /* check buffer address alignment */
    if((uintptr_t)buf % buffer_align)
        return false;

    /* check stride alignment */
    if(stride % stride_align)
        return false;

    return true;
}

POSSIBLY_UNUSED
int buffer_copy(int src_w, int src_h, void *src, void *dst)
{
    vg_lite_error_t error = VG_LITE_SUCCESS;
    vg_lite_matrix_t matrix;
    vg_lite_buffer_t src_buf;
    vg_lite_buffer_t dst_buf;
    int dst_w = src_w;
    int dst_h = src_h;

    vg_lite_identity(&matrix);

    lv_memset(&src_buf, 0, sizeof(vg_lite_buffer_t));
    src_buf.format = VG_LITE_RGB888;
    src_buf.width = src_w;
    src_buf.height = src_h;
    src_buf.stride = src_w * 3;
    src_buf.memory = src;
    src_buf.address = ADDR_CPU_TO_DEV((uint32_t)src);

    lv_memset(&dst_buf, 0, sizeof(vg_lite_buffer_t));

    dst_buf.format = VG_LITE_RGB888;
    dst_buf.width = dst_w;
    dst_buf.height = dst_h;
    dst_buf.stride = dst_w * 3;
    dst_buf.memory = dst;
    dst_buf.address = ADDR_CPU_TO_DEV((uint32_t)dst);

    error = vg_lite_blit(&dst_buf, &src_buf, &matrix, VG_LITE_BLEND_NONE, 0, VG_LITE_FILTER_POINT);
    if (error == VG_LITE_SUCCESS)
        error = vg_lite_finish();

    return error;
}

/**********************
 *   STATIC FUNCTIONS
 **********************/

#endif /*LV_USE_DRAW_BES_VGLITE*/
