#ifndef __UI_SHINE_COMPONENT_H__
#define __UI_SHINE_COMPONENT_H__

#ifdef __cplusplus
extern "C" {
#endif

/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_base.h" // IWYU pragma: keep

struct _lv_obj_t;
typedef struct _lv_obj_t lv_obj_t;
struct _lv_draw_buf_t;
typedef struct _lv_draw_buf_t lv_draw_buf_t;
struct _lv_font_t;
typedef struct _lv_font_t lv_font_t;

/*
* DEFINES
*****************************************************************************************
*/
/** 与 TEXT_SHINE_MASK / 设计稿对齐的默认光带宽度 */
#define UI_SHINE_MASK_W_DEFAULT                    (140)
#define UI_SHINE_PERIOD_MS_DEFAULT                 (3200)
/** 与 LV_OPA_80 相同（255*80%≈204），勿写成字面量 80 */
#define UI_SHINE_IMG_OPA_DEFAULT                   (204)
#define UI_SHINE_SCREEN_W_DEFAULT                  (360)

typedef struct {
    /** 扫光裁剪窗宽度（像素） */
    uint16_t mask_w;
    /** 扫光裁剪窗 Y（相对 parent TOP_LEFT） */
    int16_t mask_y;
    /** 动画扫过的屏幕宽度（X_MAX） */
    int16_t screen_w;
    /** 一轮扫光时长 ms */
    uint16_t period_ms;
    /** 亮层 image opa（LV_OPA_xx，常用 LV_OPA_80=204） */
    uint8_t shine_opa;
} ui_shine_style_t;

typedef struct {
    lv_obj_t *mask;                 /// 扫光裁剪窗（ADDITIVE）
    lv_obj_t *shine;                /// 窗内亮层图
    lv_draw_buf_t *shine_buf;       /// 整幅 tip/文案白色扫光缓冲
    uint8_t *alpha;                 /// 内容 alpha（tip 宽）
    uint8_t *profile;               /// 水平软边剖面
    uint32_t alpha_w;
    uint32_t alpha_h;
    int16_t mask_x;
    uint32_t mask_tick_ms;
} ui_shine_info_t;

/*
* GLOBAL FUNCTION DECLARATIONS
*****************************************************************************************
*/
/** 填入默认样式（宽 140 / period 3200 / opa 80 / screen 360）；调用方再改 mask_y 等 */
void ui_shine_style_init(ui_shine_style_t *style);

/** 清零对象指针与缓冲（不 destroy；页面 memset 前可先 remove） */
void ui_shine_reset(ui_shine_info_t *info);

/**
 * 从 tip PNG（ARGB）准备 alpha + 窗口 shine_buf + profile。
 * tip 为空或失败时不改已有缓冲。
 */
void ui_shine_prepare_from_tip(ui_shine_info_t *info, const ui_shine_style_t *style,
    const lv_draw_buf_t *tip);

/**
 * 从字库文案栅格后准备（临时 text buf 用完即销毁）。
 */
void ui_shine_prepare_from_text(ui_shine_info_t *info, const ui_shine_style_t *style,
    const char *text, const lv_font_t *font, int32_t letter_space);

/**
 * 创建/刷新 mask+shine；content 为 tip_img 或 hint_text（取其 x/y 对齐）。
 * 已存在且 force_update==0 时直接返回（位移交给 anim）。
 */
void ui_shine_update(ui_shine_info_t *info, lv_obj_t *parent, const ui_shine_style_t *style,
    lv_obj_t *content, uint8_t force_update);

/**
 * 扫光位移动画。paused!=0 时不更新。
 * content 用于计算 shine 相对采样 x。
 */
void ui_shine_anim_update(ui_shine_info_t *info, const ui_shine_style_t *style,
    lv_obj_t *content, uint8_t paused);

/** 隐藏扫光窗 */
void ui_shine_hide(ui_shine_info_t *info);

/** 显示扫光窗并提到前景 */
void ui_shine_show(ui_shine_info_t *info);

/** 释放缓冲并删除 mask/shine 对象 */
void ui_shine_remove(ui_shine_info_t *info);

#ifdef __cplusplus
}
#endif

#endif /* __UI_SHINE_COMPONENT_H__ */
