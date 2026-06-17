#ifndef __COMMON_UI_JPG_DECODE_H__
#define __COMMON_UI_JPG_DECODE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DECLARATION
*********************************************************************************
*/
struct _lv_draw_buf_t;
typedef struct _lv_draw_buf_t lv_draw_buf_t;

/** 目标像素格式（JPG 解码后的内存布局，供 LVGL draw_buf / lv_image 使用） */
typedef enum {
    /** 每像素 3 字节，顺序 R、G、B（`LV_COLOR_FORMAT_RGB888`） */
    COMMON_UI_JPG_DECODE_RGB888 = 0,
    /** 每像素 4 字节，`LV_COLOR_FORMAT_ARGB8888`，alpha 固定为 0xFF */
    COMMON_UI_JPG_DECODE_RGBA8888,
} common_ui_jpg_decode_fmt_e;

typedef struct {
    lv_draw_buf_t *draw_buf;
    uint32_t width;
    uint32_t height;
} common_ui_jpg_decode_result_t;

/**
 * 将 JPG 压缩数据解码为 `lv_draw_buf_t`。
 *
 * @param jpg_data  JPG 文件字节（可为资源包内嵌数据）
 * @param jpg_size  数据长度
 * @param fmt       输出格式：RGB888 或 RGBA8888（ARGB8888，A 固定为 0xFF）
 * @param out       输出；成功时 out->draw_buf 由本接口分配，调用方须 `common_ui_jpg_decode_release`
 * @return          0 成功；负数为参数/硬解/内存/格式错误码
 */
int common_ui_jpg_decode(const uint8_t *jpg_data, size_t jpg_size, common_ui_jpg_decode_fmt_e fmt,
                         common_ui_jpg_decode_result_t *out);

/** 释放 `common_ui_jpg_decode` 成功返回的 draw_buf（内部为 `lv_draw_buf_destroy`） */
void common_ui_jpg_decode_release(lv_draw_buf_t *draw_buf);

#ifdef __cplusplus
}
#endif
#endif// __COMMON_UI_JPG_DECODE_H__

