/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_snapshot.h" // IWYU pragma: keep
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_img_resource.h" // IWYU pragma: keep
#include "ui_img_png_decode.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define UI_PREDECODE_BG_COUNT    3
#define UI_PREDECODE_ICON_COUNT  3

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
/// 模式背景预解码（NATIVE），页面按资源 ID 直接取用
static const uint16_t predecode_bg_ids[UI_PREDECODE_BG_COUNT] = {
    BASE_RES_ID_SIMULATION_MODE_BG,
    BASE_RES_ID_EXPRESSION_MODE_BG,
    BASE_RES_ID_PROGRAM_MODE_BG,
};

static lv_draw_buf_t *lv_predecode_bg[UI_PREDECODE_BG_COUNT] = {NULL};

/// 模式 ICON 预解码（ARGB8888），页面按资源 ID 直接取用
static const uint16_t predecode_icon_ids[UI_PREDECODE_ICON_COUNT] = {
    BASE_RES_ID_SIMULATION_ICON,
    BASE_RES_ID_EXPRESSION_ICON,
    BASE_RES_ID_PROGRAM_ICON,
};

static lv_draw_buf_t *lv_predecode_icon[UI_PREDECODE_ICON_COUNT] = {NULL};

/// 文案扫光蒙层预解码（ARGB8888）
static lv_draw_buf_t *lv_predecode_text_shine_mask = NULL;

/// 0..(BG+ICON)，最后一档为 shine；完成后再 +1 表示 done
#define UI_PREDECODE_STEP_DONE  (UI_PREDECODE_BG_COUNT + UI_PREDECODE_ICON_COUNT + 1)
static uint8_t s_predecode_step = 0;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_predecode_bg_one(ui_manager_context_t *thiz, uint8_t index)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (index >= UI_PREDECODE_BG_COUNT || lv_predecode_bg[index] != NULL) {
        return;
    }
    lv_predecode_bg[index] = ui_img_png_theme_decode_draw_buf(thiz, predecode_bg_ids[index],
        RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_NATIVE);
}

static void ui_predecode_icon_one(ui_manager_context_t *thiz, uint8_t index)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (index >= UI_PREDECODE_ICON_COUNT || lv_predecode_icon[index] != NULL) {
        return;
    }
    lv_predecode_icon[index] = ui_img_png_theme_decode_draw_buf(thiz, predecode_icon_ids[index],
        RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
}

static void ui_predecode_text_shine_mask_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (lv_predecode_text_shine_mask != NULL) {
        return;
    }
    lv_predecode_text_shine_mask = ui_img_png_theme_decode_draw_buf(thiz,
        BASE_RES_ID_TEXT_SHINE_MASK, RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
lv_draw_buf_t *get_predecoded_bg(uint16_t res_id)
{
    for (uint8_t i = 0; i < UI_PREDECODE_BG_COUNT; i++) {
        if (predecode_bg_ids[i] == res_id) {
            return lv_predecode_bg[i];
        }
    }
    return NULL;
}

lv_draw_buf_t *get_predecoded_icon(uint16_t res_id)
{
    for (uint8_t i = 0; i < UI_PREDECODE_ICON_COUNT; i++) {
        if (predecode_icon_ids[i] == res_id) {
            return lv_predecode_icon[i];
        }
    }
    return NULL;
}

lv_draw_buf_t *get_predecoded_text_shine_mask(void)
{
    return lv_predecode_text_shine_mask;
}

bool ui_snap_shot_prepare_step(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_BOOL(thiz);

    if (s_predecode_step >= UI_PREDECODE_STEP_DONE) {
        return false;
    }

    if (s_predecode_step < UI_PREDECODE_BG_COUNT) {
        ui_predecode_bg_one(thiz, s_predecode_step);
    } else if (s_predecode_step < (UI_PREDECODE_BG_COUNT + UI_PREDECODE_ICON_COUNT)) {
        ui_predecode_icon_one(thiz, (uint8_t)(s_predecode_step - UI_PREDECODE_BG_COUNT));
    } else {
        ui_predecode_text_shine_mask_prepare(thiz);
    }

    s_predecode_step++;
    if (s_predecode_step >= UI_PREDECODE_STEP_DONE) {
        LOG_UI("snap shot predecode done");
        return false;
    }
    return true;
}

void ui_snap_shot_perpare(struct ui_manager_context *thiz)
{
    while (ui_snap_shot_prepare_step(thiz)) {
    }
}
