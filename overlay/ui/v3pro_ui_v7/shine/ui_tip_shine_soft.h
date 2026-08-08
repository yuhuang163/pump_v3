#ifndef __UI_TIP_SHINE_SOFT_H__
#define __UI_TIP_SHINE_SOFT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

struct _lv_draw_buf_t;
typedef struct _lv_draw_buf_t lv_draw_buf_t;
struct _lv_font_t;
typedef struct _lv_font_t lv_font_t;

/*
* GLOBAL FUNCTION DECLARATION
*****************************************************************************************
*/
/**
 * BES VGLITE 路径不支持 style bitmap_mask，软边需 CPU 烘焙到 tip_shine 像素。
 * 从 tip 生成白色扫光图（保留 alpha）。失败返回 NULL。
 */
lv_draw_buf_t *ui_tip_shine_buf_create(const lv_draw_buf_t *tip);

/** 复制 tip alpha，行宽=tip_w；调用方 u_free。失败返回 NULL。 */
uint8_t *ui_tip_shine_alpha_dup(const lv_draw_buf_t *tip, uint32_t *out_w, uint32_t *out_h);

/**
 * 从 TEXT_SHINE_MASK 取水平 alpha 剖面（居中裁切到 profile_w）。
 * mask_rgba 为空时用 smoothstep 剖面。
 */
void ui_tip_shine_profile_build(const lv_draw_buf_t *mask_rgba, uint8_t *profile, uint32_t profile_w);

/**
 * 按 tip_shine 相对 tip_mask 的 x，把 profile 软边写入 shine alpha。
 * shine_x：text/tip 相对 mask 的 x（text_x - mask_x）。
 */
void ui_tip_shine_apply_profile(lv_draw_buf_t *shine, const uint8_t *alpha_orig,
    uint32_t tip_w, uint32_t tip_h, int32_t shine_x, const uint8_t *profile, uint32_t profile_w);

/** 销毁扫光 buf。 */
void ui_tip_shine_buf_destroy(lv_draw_buf_t *shine);

/**
 * 将字库文案栅格为白色 ARGB8888（alpha=字形覆盖）。失败返回 NULL。
 * 用于主页 hint 等无 tip PNG、需与 tip 相同 CPU 软边扫光的场景。
 */
lv_draw_buf_t *ui_tip_shine_text_buf_create(const char *text, const lv_font_t *font,
    int32_t letter_space);

#ifdef __cplusplus
}
#endif
#endif /* __UI_TIP_SHINE_SOFT_H__ */
