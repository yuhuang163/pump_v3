#ifndef __COMMON_WAVE_UI_H__
#define __COMMON_WAVE_UI_H__

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
#define COMMON_WAVE_UI_WIDTH                314
#define COMMON_WAVE_UI_HEIGHT               21
#define COMMON_WAVE_UI_NARROW_COUNT         8
#define COMMON_WAVE_UI_MIX_SHORT_COUNT      5
#define COMMON_WAVE_UI_WIDE_FACTOR_DEFAULT  3
#define COMMON_WAVE_UI_LINE_WIDTH_DEFAULT   2
#define COMMON_WAVE_UI_SHADOW_Y_OFFSET_DEFAULT 3
#define COMMON_WAVE_UI_SHADOW_COLOR_DEFAULT 0xFFE5E7
#define COMMON_WAVE_UI_MIN_PERIOD           4
/** 目标 20fps */
#define COMMON_WAVE_UI_DEFAULT_ANIM_PERIOD_MS 50

typedef struct common_wave_ui common_wave_ui_t;

/** 波形类型：与泵奶模式一一对应 */
typedef enum {
    COMMON_WAVE_TYPE_STIMULATION = 0,
    COMMON_WAVE_TYPE_LACTATION,
    COMMON_WAVE_TYPE_MIX,
} common_wave_type_e;

typedef enum {
    COMMON_WAVE_ANIM_NONE = 0,
    COMMON_WAVE_ANIM_SCROLL,
} common_wave_anim_mode_e;

typedef struct {
    uint32_t color;
    uint8_t line_width;
    int16_t amplitude;
    uint16_t period_px;
    common_wave_type_e type;
    common_wave_anim_mode_e anim_mode;
    uint16_t anim_period_ms;
    uint8_t shadow_enable;
    uint32_t shadow_color;
    int16_t shadow_y_offset;
} common_wave_ui_cfg_t;

/*
* GLOBAL FUNCTION DECLARATION
*********************************************************************************
*/
void common_wave_ui_cfg_init_default(common_wave_ui_cfg_t *cfg);
common_wave_ui_t *common_wave_ui_create(lv_obj_t *parent, const common_wave_ui_cfg_t *cfg);
void common_wave_ui_delete(common_wave_ui_t *wave);
lv_obj_t *common_wave_ui_get_obj(const common_wave_ui_t *wave);
lv_obj_t *common_wave_ui_get_shadow_obj(const common_wave_ui_t *wave);
void common_wave_ui_set_type(common_wave_ui_t *wave, common_wave_type_e type);
void common_wave_ui_set_amplitude(common_wave_ui_t *wave, int16_t amplitude);
void common_wave_ui_set_narrow_period_px(common_wave_ui_t *wave, uint16_t period_x100);
void common_wave_ui_set_color(common_wave_ui_t *wave, uint32_t color_hex);
void common_wave_ui_start_anim(common_wave_ui_t *wave, common_wave_anim_mode_e mode);
void common_wave_ui_stop_anim(common_wave_ui_t *wave);
void common_wave_ui_refresh(common_wave_ui_t *wave);

#ifdef __cplusplus
}
#endif

#endif /* __COMMON_WAVE_UI_H__ */
