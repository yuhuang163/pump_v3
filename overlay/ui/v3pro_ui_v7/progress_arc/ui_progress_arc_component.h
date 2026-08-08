#ifndef __UI_PROGRESS_ARC_COMPONENT_H__
#define __UI_PROGRESS_ARC_COMPONENT_H__

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

/*
* DEFINES
*****************************************************************************************
*/
#define UI_PROGRESS_ARC_SIZE                       (352)
#define UI_PROGRESS_ARC_WIDTH                      (11)
/** 底部开口角度，使圆环不闭合（越大两端越高） */
#define UI_PROGRESS_ARC_BOTTOM_GAP_DEG             (100)
/** 已展示 / 剩余两段圆弧之间的角度偏差 */
#define UI_PROGRESS_ARC_SEG_GAP_DEG                (12)
/** 除去底部开口后的可绘制总角度 */
#define UI_PROGRESS_ARC_TRACK_DEG \
    (360 - UI_PROGRESS_ARC_BOTTOM_GAP_DEG)
/** 开口居中于底部：弧起点落在 gap 右侧 */
#define UI_PROGRESS_ARC_ROTATION \
    (90 + (UI_PROGRESS_ARC_BOTTOM_GAP_DEG / 2))

#define UI_PROGRESS_ARC_REMAIN_COLOR_DEFAULT       (0x252525)
#define UI_PROGRESS_ARC_COUNTDOWN_COLOR_DEFAULT    (0x999999)
#define UI_PROGRESS_ARC_COUNTDOWN_Y_DEFAULT        (334)

typedef struct {
    /** 已完成段颜色（进度色） */
    uint32_t done_color;
    /** 剩余段颜色 */
    uint32_t remain_color;
    /** 底部倒计时文字颜色 */
    uint32_t countdown_color;
    /** 底部倒计时 Y 偏移（TOP_MID） */
    int16_t countdown_y;
    /** 是否显示底部倒计时：0 隐藏，1 显示 */
    uint8_t show_countdown;
} ui_progress_arc_style_t;

typedef struct {
    lv_obj_t *done_arc;
    lv_obj_t *remain_arc;
    lv_obj_t *countdown;
    uint32_t shown_done_color;
    uint32_t shown_remain_color;
    uint16_t shown_done_sweep;
    uint8_t shown_remain_sec;
} ui_progress_arc_info_t;

/*
* GLOBAL FUNCTION DECLARATIONS
*****************************************************************************************
*/
/** 用进度色初始化样式；剩余色 / 倒计时默认值已填好，show_countdown 默认开启 */
void ui_progress_arc_style_init(ui_progress_arc_style_t *style, uint32_t done_color);

/** 重置缓存状态（页面 prepare / 重新开始计时时调用） */
void ui_progress_arc_reset(ui_progress_arc_info_t *info);

/** 按 done_sweep 刷新双段圆弧；颜色由 style 传入 */
void ui_progress_arc_update(ui_progress_arc_info_t *info, lv_obj_t *parent,
    const ui_progress_arc_style_t *style, uint16_t done_sweep, uint8_t force_update);

/** 隐藏双段圆弧 */
void ui_progress_arc_hide(ui_progress_arc_info_t *info);

/**
 * 刷新底部倒计时文字。
 * style->show_countdown == 0 时隐藏；否则显示 remain_sec（文案 "N s"）。
 */
void ui_progress_arc_countdown_update(ui_progress_arc_info_t *info, lv_obj_t *parent,
    const ui_progress_arc_style_t *style, uint8_t remain_sec, uint8_t force_update);

/** 隐藏底部倒计时 */
void ui_progress_arc_countdown_hide(ui_progress_arc_info_t *info);

/**
 * 按时长进度刷新圆弧，并按 style->show_countdown 决定是否刷新倒计时。
 * progress / total 映射到轨道角度；elapsed_ms / duration_ms / countdown_max 用于秒数。
 */
void ui_progress_arc_time_update(ui_progress_arc_info_t *info, lv_obj_t *parent,
    const ui_progress_arc_style_t *style, uint32_t start_time, uint32_t duration_ms,
    uint8_t countdown_max, uint8_t force_update);

/** progress/total → done_sweep（含满轨与段间隙处理） */
uint16_t ui_progress_arc_sweep_from_progress(uint32_t progress, uint32_t total);

/** 倒计时剩余秒：在 duration 内从 countdown_max 递减到 1 */
uint8_t ui_progress_arc_remain_sec_get(uint32_t elapsed_ms, uint32_t duration_ms,
    uint8_t countdown_max);

#ifdef __cplusplus
}
#endif

#endif // __UI_PROGRESS_ARC_COMPONENT_H__
