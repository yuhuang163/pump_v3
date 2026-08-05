/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "pump_stop_hold_ui.h" // IWYU pragma: keep
#include "ui_progress_arc_component.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_PUMP_STOP_HOLD_COMP_BG           (0) /// 是否启用对比背景图

#define PUMP_STOP_HOLD_ARC_DONE_COLOR               (0xEF4C4E)

#define PUMP_STOP_HOLD_ICON_Y                       (94)
#define PUMP_STOP_HOLD_TITLE_Y                      (200)
#define PUMP_STOP_HOLD_TITLE_COLOR                  (0xFFFFFF)
#define PUMP_STOP_HOLD_TITLE_TEXT                   "Press and hold to\nend pumping."

/** 页面展示倒计时（LONG_2S 后再持续约 2s，文案为 3s/2s/1s） */
#define PUMP_STOP_HOLD_DURATION_MS                  (2000)
#define PUMP_STOP_HOLD_COUNTDOWN_MAX                (3)

typedef struct {
    lv_obj_t *root;
#if IS_SUPPORT_PUMP_STOP_HOLD_COMP_BG
    lv_obj_t *background;
#endif
    ui_progress_arc_info_t arc;
    lv_obj_t *icon;
    lv_obj_t *title;
    lv_draw_buf_t *icon_buf;
    uint32_t start_time;
    uint8_t force_update;
    uint8_t completed;
    uint8_t rsv[10];
} pump_stop_hold_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_PUMP_STOP_HOLD_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t pump_stop_hold_bg_data[] = {
    #include "comparison_png2txt/pump_home_23_352x352.txt"
};
#endif

static pump_stop_hold_ui_info_t *hold_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_stop_hold_root_init(void)
{
    if (hold_info->root == NULL || !lv_obj_is_valid(hold_info->root)) {
        hold_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(hold_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(hold_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(hold_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(hold_info->root, 0, 0);
    lv_obj_set_style_pad_all(hold_info->root, 0, 0);
    lv_obj_align(hold_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(hold_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(hold_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

#if IS_SUPPORT_PUMP_STOP_HOLD_COMP_BG
static void pump_stop_hold_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;

    (void)thiz;
    if (hold_info->background == NULL || !lv_obj_is_valid(hold_info->background)) {
        hold_info->background = lv_image_create(hold_info->root);
        EXCEPTION_CHECK_VOID(hold_info->background);
        lv_obj_move_background(hold_info->background);
    }
    if (status == 0) {
        lv_obj_add_flag(hold_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_fs_path_ex_t bg_img_path;
        lv_obj_clear_flag(hold_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, pump_stop_hold_bg_data,
            sizeof(pump_stop_hold_bg_data), "png");
        lv_image_set_src(hold_info->background, (const void *)&bg_img_path);
        lv_obj_align(hold_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void pump_stop_hold_bg_update(ui_manager_context_t *thiz)
{
    (void)thiz;
}
#endif

static void pump_stop_hold_arc_update(void)
{
    ui_progress_arc_style_t style;

    ui_progress_arc_style_init(&style, PUMP_STOP_HOLD_ARC_DONE_COLOR);
    style.show_countdown = true;
    ui_progress_arc_time_update(&hold_info->arc, hold_info->root, &style,
        hold_info->start_time, PUMP_STOP_HOLD_DURATION_MS,
        PUMP_STOP_HOLD_COUNTDOWN_MAX, hold_info->force_update);
}

static void pump_stop_hold_icon_update(ui_manager_context_t *thiz)
{
    if (hold_info->force_update == false) {
        return;
    }

    if (hold_info->icon == NULL || !lv_obj_is_valid(hold_info->icon)) {
        hold_info->icon = lv_image_create(hold_info->root);
        EXCEPTION_CHECK_VOID(hold_info->icon);
    }
    if (hold_info->icon_buf == NULL) {
        hold_info->icon_buf = ui_img_baj_theme_decode_draw_buf(thiz,
            BASE_RES_ID_POWER_KEY_ICON, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (hold_info->icon_buf != NULL) {
        lv_image_set_src(hold_info->icon, hold_info->icon_buf);
    } else {
        ui_img_baj_theme_set_src(thiz, hold_info->icon, BASE_RES_ID_POWER_KEY_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(hold_info->icon, LV_ALIGN_TOP_MID, 0, PUMP_STOP_HOLD_ICON_Y);
}

static void pump_stop_hold_title_update(void)
{
    if (hold_info->force_update == false) {
        return;
    }

    if (hold_info->title == NULL || !lv_obj_is_valid(hold_info->title)) {
        hold_info->title = lv_label_create(hold_info->root);
        EXCEPTION_CHECK_VOID(hold_info->title);
    }
    lv_label_set_text(hold_info->title, PUMP_STOP_HOLD_TITLE_TEXT);
    lv_obj_set_style_text_color(hold_info->title, lv_color_hex(PUMP_STOP_HOLD_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(hold_info->title, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(hold_info->title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_set_style_text_line_space(hold_info->title, 8, 0);
    lv_obj_align(hold_info->title, LV_ALIGN_TOP_MID, 0, PUMP_STOP_HOLD_TITLE_Y);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_stop_hold_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (hold_info == NULL) {
        hold_info = (pump_stop_hold_ui_info_t *)u_malloz(sizeof(pump_stop_hold_ui_info_t));
        EXCEPTION_CHECK_VOID(hold_info);
    }
    hold_info->force_update = true;
    hold_info->completed = false;
    hold_info->start_time = get_timestamp();
    ui_progress_arc_reset(&hold_info->arc);
    pump_stop_hold_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_pump_stop_hold_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && hold_info && hold_info->root);
    if (hold_info->icon_buf != NULL) {
        if (hold_info->icon != NULL && lv_obj_is_valid(hold_info->icon)) {
            lv_image_set_src(hold_info->icon, NULL);
        }
        lv_draw_buf_destroy(hold_info->icon_buf);
        hold_info->icon_buf = NULL;
    } else {
        ui_img_baj_release_decoded_buf(hold_info->icon);
    }
    LVGL_OBJ_DELETE(hold_info->root);
    u_free(hold_info);
    hold_info = NULL;
}

int ui_display_pump_stop_hold_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && hold_info && hold_info->root);
    pump_stop_hold_bg_update(thiz);
    pump_stop_hold_arc_update();
    pump_stop_hold_icon_update(thiz);
    pump_stop_hold_title_update();
    hold_info->force_update = false;

    if (hold_info->completed == false &&
        get_timestamp_ms(hold_info->start_time) >= PUMP_STOP_HOLD_DURATION_MS) {
        hold_info->completed = true;
        if (thiz->priv.play_info.progress_cb) {
            thiz->priv.play_info.progress_cb(thiz, 0, ANI_POPUP_DISPLAY_FINISH);
        }
    }

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump stop hold fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
