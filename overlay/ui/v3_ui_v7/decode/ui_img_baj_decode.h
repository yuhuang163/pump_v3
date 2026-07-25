#ifndef __UI_IMG_BAJ_DECODE_H__
#define __UI_IMG_BAJ_DECODE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep
#include "resource_manager_api.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DECLARATION
*********************************************************************************
*/
struct _lv_draw_buf_t;
typedef struct _lv_draw_buf_t lv_draw_buf_t;

struct _lv_obj_t;
typedef struct _lv_obj_t lv_obj_t;

struct ui_manager_context;
typedef struct ui_manager_context ui_manager_context_t;

/** 目标像素格式（BAJ 解码后的内存布局，供 LVGL draw_buf / lv_image 使用） */
typedef enum {
    /** 每像素 3 字节，顺序 R、G、B（`LV_COLOR_FORMAT_RGB888`） */
    UI_IMG_BAJ_DECODE_RGB888 = 0,
    /** 每像素 4 字节，`LV_COLOR_FORMAT_ARGB8888`（含 Alpha 通道） */
    UI_IMG_BAJ_DECODE_RGBA8888,
} ui_img_baj_decode_fmt_e;

/** 释放 lv_image 上由 `ui_img_baj_set_src` 设置的预解码 draw_buf */
void ui_img_baj_release_decoded_buf(lv_obj_t *obj);

/**
 * 将 BAJ 解码后设到 lv_image；换图前自动释放旧的预解码缓存。
 * 解码失败时回退为 LVGL memfs BAJ 路径。
 *
 * @return 0 软解码成功；负数 decode 错误（已设置 memfs 回退）
 */
int ui_img_baj_set_src(lv_obj_t *obj, const uint8_t *baj_data, size_t baj_size,
                                ui_img_baj_decode_fmt_e fmt);

/**
 * 从主题/基础资源加载 BAJ，解码设到 lv_image，并 unload 压缩源。
 *
 * @return 0 软解码成功；负数 参数/加载/decode 错误（decode 失败时已 memfs 回退）
 */
int ui_img_baj_theme_set_src(ui_manager_context_t *thiz, lv_obj_t *obj, uint16_t resource_id,
                                      res_theme_type_e theme_type, ui_img_baj_decode_fmt_e fmt);

#ifdef __cplusplus
}
#endif
#endif // __UI_IMG_BAJ_DECODE_H__
