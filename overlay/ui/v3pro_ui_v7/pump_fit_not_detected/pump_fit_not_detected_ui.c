/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "ui_img_resource.h" // IWYU pragma: keep
#include "pump_fit_not_detected_ui.h" // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_FIT_ND_BG_COLOR                            (0x000000)
#define PUMP_FIT_ND_TITLE_COLOR                         (0xFFFFFF)
#define PUMP_FIT_ND_DESC_COLOR                          (0x999999)
#define PUMP_FIT_ND_ANIM_FRAME_INVALID                  (0xFFFF)
#define PUMP_FIT_ND_ANIM_X                              (54)
#define PUMP_FIT_ND_ANIM_Y                              (64)
#define PUMP_FIT_ND_ANIM_REST_MS                        (2000)
#define PUMP_FIT_ND_ANIM_FRAME_INTERVAL_MS              (1000 / PUMP_ANIMATION_FREQ)

typedef struct {
    lv_obj_t *root;
    lv_obj_t *title;
    lv_obj_t *desc;
    lv_obj_t *anim;
    ui_manager_context_t *ui;
    res_img_info_t anim_info;
    lv_fs_path_ex_t anim_path;
    uint8_t anim_resting;
    uint16_t anim_start;
    uint16_t anim_end;
    uint16_t frame_id;
    uint16_t shown_frame_id;
    uint32_t last_frame_tick_ms;
    uint32_t anim_rest_start_ms;
    uint8_t force_update;
} pump_fit_not_detected_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static pump_fit_not_detected_ui_info_t *s_fit_nd_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_fit_nd_root_init(void)
{
    if (s_fit_nd_info->root == NULL || !lv_obj_is_valid(s_fit_nd_info->root)) {
        s_fit_nd_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(s_fit_nd_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(s_fit_nd_info->root, lv_color_hex(PUMP_FIT_ND_BG_COLOR), 0);
    lv_obj_set_style_bg_opa(s_fit_nd_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(s_fit_nd_info->root, 0, 0);
    lv_obj_set_style_pad_all(s_fit_nd_info->root, 0, 0);
    lv_obj_align(s_fit_nd_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(s_fit_nd_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(s_fit_nd_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static lv_obj_t *pump_fit_nd_label_create(lv_obj_t *parent, const lv_font_t *font,
    uint32_t color, lv_coord_t y, int32_t line_space)
{
    lv_obj_t *label = lv_label_create(parent);

    if (label == NULL) {
        return NULL;
    }
    lv_obj_set_style_text_font(label, font, 0);
    lv_obj_set_style_text_color(label, lv_color_hex(color), 0);
    lv_obj_set_style_text_align(label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_letter_space(label, 0, 0);
    if (line_space != 0) {
        lv_obj_set_style_text_line_space(label, line_space, 0);
    }
    lv_obj_align(label, LV_ALIGN_TOP_MID, 0, y);
    return label;
}

static void pump_fit_nd_anim_deinit(pump_fit_not_detected_ui_info_t *info)
{
    if (info == NULL) {
        return;
    }
    if (info->anim_info.data != NULL && info->ui != NULL) {
        ui_img_resource_deinit(info->ui, &info->anim_info);
        memset(&info->anim_info, 0, sizeof(info->anim_info));
    }
    if (info->anim != NULL && lv_obj_is_valid(info->anim)) {
        lv_image_set_src(info->anim, NULL);
    }
    info->shown_frame_id = PUMP_FIT_ND_ANIM_FRAME_INVALID;
}

static void pump_fit_nd_anim_frame_show(ui_manager_context_t *ui,
    pump_fit_not_detected_ui_info_t *info, uint16_t frame_id)
{
    EXCEPTION_CHECK_VOID(ui && info && info->anim);
    if (info->shown_frame_id == frame_id) {
        return;
    }
    if (info->anim_info.data != NULL) {
        ui_img_resource_deinit(ui, &info->anim_info);
        memset(&info->anim_info, 0, sizeof(info->anim_info));
    }
    ui_theme_img_resource_init(ui, frame_id, RES_THEME_TYPE_BASE, &info->anim_info);
    EXCEPTION_CHECK_VOID(info->anim_info.data);
    lv_fs_make_path_from_buffer(&info->anim_path, LV_FS_MEMFS_LETTER,
        info->anim_info.data, info->anim_info.size, "baj");
    lv_image_set_src(info->anim, (const void *)&info->anim_path);
    info->shown_frame_id = frame_id;
}

static void pump_fit_nd_anim_build(ui_manager_context_t *ui)
{
    EXCEPTION_CHECK_VOID(ui && s_fit_nd_info);
    s_fit_nd_info->anim = lv_image_create(s_fit_nd_info->root);
    EXCEPTION_CHECK_VOID(s_fit_nd_info->anim);
    s_fit_nd_info->anim_start = BASE_RES_ID_CHECK_FIT_ANIM_START;
    s_fit_nd_info->anim_end = BASE_RES_ID_CHECK_FIT_ANIM_END;
    s_fit_nd_info->frame_id = s_fit_nd_info->anim_start;
    s_fit_nd_info->shown_frame_id = PUMP_FIT_ND_ANIM_FRAME_INVALID;
    s_fit_nd_info->anim_resting = 0;
    s_fit_nd_info->last_frame_tick_ms = get_timestamp();
    pump_fit_nd_anim_frame_show(ui, s_fit_nd_info, s_fit_nd_info->frame_id);
    lv_obj_align(s_fit_nd_info->anim, LV_ALIGN_TOP_LEFT, PUMP_FIT_ND_ANIM_X, PUMP_FIT_ND_ANIM_Y);
}

static void pump_fit_nd_content_build(ui_manager_context_t *ui)
{
    EXCEPTION_CHECK_VOID(ui && s_fit_nd_info && s_fit_nd_info->root);

    s_fit_nd_info->title = pump_fit_nd_label_create(s_fit_nd_info->root, UI_FONT_TITLE,
        PUMP_FIT_ND_TITLE_COLOR, PUMP_FIT_NOT_DETECTED_TITLE_Y, 0);
    EXCEPTION_CHECK_VOID(s_fit_nd_info->title);
    lv_label_set_text(s_fit_nd_info->title, PUMP_FIT_NOT_DETECTED_TITLE);

    s_fit_nd_info->desc = pump_fit_nd_label_create(s_fit_nd_info->root, UI_FONT_HINT,
        PUMP_FIT_ND_DESC_COLOR, PUMP_FIT_NOT_DETECTED_DESC_Y,
        PUMP_FIT_NOT_DETECTED_DESC_LINE_SPACE);
    EXCEPTION_CHECK_VOID(s_fit_nd_info->desc);
    lv_label_set_text(s_fit_nd_info->desc, PUMP_FIT_NOT_DETECTED_DESC);

    pump_fit_nd_anim_build(ui);
    if (s_fit_nd_info->anim != NULL && lv_obj_is_valid(s_fit_nd_info->anim)) {
        lv_obj_move_to_index(s_fit_nd_info->anim, 0);
    }
}

static void pump_fit_nd_anim_tick(ui_manager_context_t *ui)
{
    pump_fit_not_detected_ui_info_t *info = s_fit_nd_info;
    uint32_t now;

    if (info == NULL || info->anim == NULL || !lv_obj_is_valid(info->anim)) {
        return;
    }
    now = get_timestamp();

    if (info->anim_resting) {
        if ((now - info->anim_rest_start_ms) < PUMP_FIT_ND_ANIM_REST_MS) {
            return;
        }
        info->anim_resting = 0;
        info->frame_id = info->anim_start;
        info->last_frame_tick_ms = now;
        pump_fit_nd_anim_frame_show(ui, info, info->frame_id);
        if (info->frame_id < info->anim_end) {
            info->frame_id++;
        }
        return;
    }

    if (info->last_frame_tick_ms != 0 &&
        (now - info->last_frame_tick_ms) < PUMP_FIT_ND_ANIM_FRAME_INTERVAL_MS) {
        return;
    }
    info->last_frame_tick_ms = now;
    pump_fit_nd_anim_frame_show(ui, info, info->frame_id);
    if (info->frame_id >= info->anim_end) {
        info->anim_resting = 1;
        info->anim_rest_start_ms = now;
        return;
    }
    info->frame_id++;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_fit_not_detected_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (s_fit_nd_info == NULL) {
        s_fit_nd_info = (pump_fit_not_detected_ui_info_t *)u_malloz(
            sizeof(pump_fit_not_detected_ui_info_t));
        EXCEPTION_CHECK_VOID(s_fit_nd_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    pump_fit_nd_root_init();
    s_fit_nd_info->ui = thiz;
    s_fit_nd_info->force_update = true;
    pump_fit_nd_content_build(thiz);
}

void ui_display_pump_fit_not_detected_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && s_fit_nd_info && s_fit_nd_info->root);
    pump_fit_nd_anim_deinit(s_fit_nd_info);
    LVGL_OBJ_DELETE(s_fit_nd_info->root);
    u_free(s_fit_nd_info);
    s_fit_nd_info = NULL;
}

int ui_display_pump_fit_not_detected_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && s_fit_nd_info && s_fit_nd_info->root);
    pump_fit_nd_anim_tick(thiz);
    s_fit_nd_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump fit not detected fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
