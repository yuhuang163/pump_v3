#ifndef __UI_WAVE_H__
#define __UI_WAVE_H__

#ifdef __cplusplus
extern "C" {
#endif

/*******************************************************************************
* Include files
******************************************************************************/
#include "root_common_base.h" // IWYU pragma: keep

struct _lv_obj_t;
typedef struct _lv_obj_t lv_obj_t;

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_WAVE_OBJECT_ENABLE            (0) // 是否支持动画效果
#define UI_WAVE_WIDTH                314
#define UI_WAVE_HEIGHT               21
#define UI_WAVE_NARROW_COUNT         8
#define UI_WAVE_MIX_SHORT_COUNT      5
#define UI_WAVE_WIDE_FACTOR_DEFAULT  3
#define UI_WAVE_LINE_WIDTH_DEFAULT   2
#define UI_WAVE_SHADOW_Y_OFFSET_DEFAULT 3
#define UI_WAVE_SHADOW_COLOR_DEFAULT 0xFFE5E7
#define UI_WAVE_MIN_PERIOD           4
/** 目标 20fps */
#define UI_WAVE_DEFAULT_ANIM_PERIOD_MS 50

typedef struct ui_wave ui_wave_t;

/** 波形类型：与泵奶模式一一对应 */
typedef enum {
    UI_WAVE_TYPE_STIMULATION = 0,
    UI_WAVE_TYPE_LACTATION,
    UI_WAVE_TYPE_MIX,
} ui_wave_type_e;

typedef enum {
    UI_WAVE_ANIM_NONE = 0,
    UI_WAVE_ANIM_SCROLL,
} ui_wave_anim_mode_e;

typedef struct {
    uint32_t color;
    uint8_t line_width;
    int16_t amplitude;
    uint16_t period_px;
    ui_wave_type_e type;
    ui_wave_anim_mode_e anim_mode;
    uint16_t anim_period_ms;
    uint8_t shadow_enable;
    uint32_t shadow_color;
    int16_t shadow_y_offset;
} ui_wave_cfg_t;

/*
* GLOBAL FUNCTION DECLARATION
*********************************************************************************
*/
void ui_wave_cfg_init_default(ui_wave_cfg_t *cfg);
ui_wave_t *ui_wave_create(lv_obj_t *parent, const ui_wave_cfg_t *cfg);
void ui_wave_delete(ui_wave_t *wave);
lv_obj_t *ui_wave_get_obj(const ui_wave_t *wave);
lv_obj_t *ui_wave_get_shadow_obj(const ui_wave_t *wave);
void ui_wave_set_type(ui_wave_t *wave, ui_wave_type_e type);

#ifdef __cplusplus
}
#endif

#endif /* __UI_WAVE_H__ */
