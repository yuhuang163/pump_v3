/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_battery.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_LOW_POWER_COMP_BG                          (0)
#define LOW_POWER_ICON_Y                                      (118)
#define LOW_POWER_TEXT_Y                                      (189)
#define LOW_POWER_TEXT_COLOR                                  (0x240F1B)
#define LOW_POWER_BATTERY_FRAME_W                             (89)
#define LOW_POWER_BATTERY_FRAME_H                             (47)
#define LOW_POWER_BATTERY_CAP_MAX_W                           (59)
#define LOW_POWER_BATTERY_CAP_H                               (27)
#define LOW_POWER_BATTERY_CAP_X_OFFSET                        (10)
#define LOW_POWER_BATTERY_CAP_Y_OFFSET                        (10)
#define LOW_POWER_BATTERY_CAP_COLOR                           (0xE30509)
#define LOW_POWER_BATTERY_CAP_RADIUS                          (5)
#define LOW_POWER_BATTERY_CAP_WIDTH_BY_SEGMENT                (0)
#define LOW_POWER_BATTERY_CAP_SEGMENT_COUNT                   (5)
#define LOW_POWER_BATTERY_DEFAULT_PERCENT                     (3)

typedef struct {
    lv_obj_t * root;
    lv_obj_t * background;
    lv_obj_t * icon_container;
    lv_obj_t * cap_fill;
    lv_obj_t * frame_img;
    lv_obj_t * text;
    uint8_t percent;
    uint8_t force_update;
    uint8_t rsv[14];
} low_power_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_LOW_POWER_COMP_BG
static PSRAM_DATA_LOC uint8_t low_power_bg_data[] = {
    #include "comparison_png2txt/low_power_p1_360x360.txt"
};
#endif

static low_power_display_info_t *low_power_info;

static const char *low_power_text = "Battery too low\nConnect to a charger";

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void low_power_root_init(void)
{
    if (low_power_info->root == NULL || !lv_obj_is_valid(low_power_info->root)) {
        low_power_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(low_power_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(low_power_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(low_power_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(low_power_info->root, 0, 0);
    lv_obj_set_style_pad_all(low_power_info->root, 0, 0);
    lv_obj_align(low_power_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(low_power_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(low_power_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

#if IS_SUPPORT_LOW_POWER_COMP_BG
static void low_power_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;

    if (low_power_info->background == NULL || !lv_obj_is_valid(low_power_info->background)) {
        low_power_info->background = lv_image_create(low_power_info->root);
        EXCEPTION_CHECK_VOID(low_power_info->background);
    }

    if (status == 0) {
        lv_obj_add_flag(low_power_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_fs_path_ex_t bg_img_path;
        lv_obj_clear_flag(low_power_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, low_power_bg_data,
            sizeof(low_power_bg_data), "png");
        lv_image_set_src(low_power_info->background, (const void *)&bg_img_path);
        lv_obj_align(low_power_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void low_power_bg_update(ui_manager_context_t *thiz)
{
    /// V7 已无 COMMON_BG 快照；root 黑底即可
    (void)thiz;
    if (low_power_info->background != NULL && lv_obj_is_valid(low_power_info->background)) {
        lv_obj_add_flag(low_power_info->background, LV_OBJ_FLAG_HIDDEN);
    }
}
#endif

static lv_coord_t low_power_cap_width_get(uint8_t percent)
{
#if LOW_POWER_BATTERY_CAP_WIDTH_BY_SEGMENT
    uint8_t segments = 0;

    if (percent >= 81) {
        segments = 5;
    } else if (percent >= 61) {
        segments = 4;
    } else if (percent >= 41) {
        segments = 3;
    } else if (percent >= 21) {
        segments = 2;
    } else if (percent >= 5) {
        segments = 1;
    }

    if (segments == 0) {
        return 0;
    }

    return (lv_coord_t)(LOW_POWER_BATTERY_CAP_MAX_W * segments) / LOW_POWER_BATTERY_CAP_SEGMENT_COUNT;
#else
    if (percent == 0) {
        return 0;
    }

    lv_coord_t width = ((lv_coord_t)LOW_POWER_BATTERY_CAP_MAX_W * percent + 50) / 100;
    if (width < 1) {
        width = 1;
    }
    if (width > LOW_POWER_BATTERY_CAP_MAX_W) {
        width = LOW_POWER_BATTERY_CAP_MAX_W;
    }
    return width;
#endif
}

static lv_coord_t low_power_cap_radius_get(lv_coord_t width)
{
    lv_coord_t radius = LOW_POWER_BATTERY_CAP_RADIUS;
    lv_coord_t max_radius = width / 2;

    if (max_radius < radius) {
        radius = max_radius;
    }
    if (radius < 1) {
        radius = 1;
    }
    return radius;
}

static uint8_t low_power_battery_percent_get(ui_manager_context_t *thiz)
{
    ui_proc_battery_t *proc = NULL;

    if (thiz != NULL) {
        proc = (ui_proc_battery_t *)thiz->priv.play_info.user_data;
    }
    if (proc != NULL && proc->battery != NULL && proc->battery->get_battery_percent != NULL) {
        return proc->battery->get_battery_percent();
    }
    return LOW_POWER_BATTERY_DEFAULT_PERCENT;
}

static void low_power_icon_container_ensure(void)
{
    if (low_power_info->icon_container != NULL &&
        lv_obj_is_valid(low_power_info->icon_container)) {
        lv_obj_align(low_power_info->icon_container, LV_ALIGN_TOP_MID, 0, LOW_POWER_ICON_Y);
        return;
    }

    low_power_info->frame_img = NULL;
    low_power_info->cap_fill = NULL;
    low_power_info->icon_container = lv_obj_create(low_power_info->root);
    EXCEPTION_CHECK_VOID(low_power_info->icon_container);
    lv_obj_set_size(low_power_info->icon_container, LOW_POWER_BATTERY_FRAME_W, LOW_POWER_BATTERY_FRAME_H);
    lv_obj_set_style_bg_opa(low_power_info->icon_container, LV_OPA_TRANSP, 0);
    lv_obj_set_style_border_width(low_power_info->icon_container, 0, 0);
    lv_obj_set_style_pad_all(low_power_info->icon_container, 0, 0);
    lv_obj_clear_flag(low_power_info->icon_container, LV_OBJ_FLAG_SCROLLABLE);
    lv_obj_align(low_power_info->icon_container, LV_ALIGN_TOP_MID, 0, LOW_POWER_ICON_Y);
}

static void low_power_cap_fill_update(uint8_t percent)
{
    const lv_coord_t fill_width = low_power_cap_width_get(percent);

    if (low_power_info->cap_fill == NULL || !lv_obj_is_valid(low_power_info->cap_fill)) {
        low_power_info->cap_fill = lv_obj_create(low_power_info->icon_container);
        EXCEPTION_CHECK_VOID(low_power_info->cap_fill);
        lv_obj_set_style_border_width(low_power_info->cap_fill, 0, 0);
        lv_obj_set_style_pad_all(low_power_info->cap_fill, 0, 0);
        lv_obj_set_style_bg_opa(low_power_info->cap_fill, LV_OPA_COVER, 0);
        lv_obj_clear_flag(low_power_info->cap_fill, LV_OBJ_FLAG_SCROLLABLE);
    }

    if (fill_width == 0) {
        lv_obj_add_flag(low_power_info->cap_fill, LV_OBJ_FLAG_HIDDEN);
        return;
    }

    lv_obj_clear_flag(low_power_info->cap_fill, LV_OBJ_FLAG_HIDDEN);
    lv_obj_set_size(low_power_info->cap_fill, fill_width, LOW_POWER_BATTERY_CAP_H);
    lv_obj_set_style_radius(low_power_info->cap_fill, low_power_cap_radius_get(fill_width), 0);
    lv_obj_set_style_bg_color(low_power_info->cap_fill, lv_color_hex(LOW_POWER_BATTERY_CAP_COLOR), 0);
    lv_obj_align(low_power_info->cap_fill, LV_ALIGN_TOP_LEFT,
        LOW_POWER_BATTERY_CAP_X_OFFSET, LOW_POWER_BATTERY_CAP_Y_OFFSET);
}

static void low_power_frame_update(ui_manager_context_t *thiz)
{
    if (low_power_info->frame_img == NULL || !lv_obj_is_valid(low_power_info->frame_img)) {
        low_power_info->frame_img = lv_image_create(low_power_info->icon_container);
        EXCEPTION_CHECK_VOID(low_power_info->frame_img);
    }

    ui_img_baj_theme_set_src(thiz, low_power_info->frame_img,
        BASE_RES_ID_BATTERY_LOW_POWER_ICON, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_clear_flag(low_power_info->frame_img, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(low_power_info->frame_img, LV_ALIGN_TOP_LEFT, 0, 0);
    lv_obj_move_foreground(low_power_info->frame_img);
}

static void low_power_icon_update(ui_manager_context_t *thiz)
{
    const uint8_t percent = low_power_battery_percent_get(thiz);

    if (!low_power_info->force_update && low_power_info->percent == percent) {
        return;
    }

    low_power_icon_container_ensure();
    EXCEPTION_CHECK_VOID(low_power_info->icon_container);
    low_power_cap_fill_update(percent);
    if (low_power_info->force_update) {
        low_power_frame_update(thiz);
    }
    low_power_info->percent = percent;
}

static void low_power_text_update(void)
{
    if (low_power_info->force_update == false) {
        return;
    }

    if (low_power_info->text == NULL || !lv_obj_is_valid(low_power_info->text)) {
        low_power_info->text = lv_label_create(low_power_info->root);
        EXCEPTION_CHECK_VOID(low_power_info->text);
    }
    lv_label_set_text(low_power_info->text, low_power_text);
    lv_obj_set_style_text_color(low_power_info->text, lv_color_hex(LOW_POWER_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(low_power_info->text, &lv_font_aeonik_soft_24_medium, 0);
    lv_obj_set_style_text_line_space(low_power_info->text, 4, 0);
    lv_obj_set_style_text_align(low_power_info->text, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(low_power_info->text, LV_ALIGN_TOP_MID, 0, LOW_POWER_TEXT_Y);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_battery_low_power_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (low_power_info == NULL) {
        low_power_info = (low_power_display_info_t *)u_malloz(sizeof(low_power_display_info_t));
        EXCEPTION_CHECK_VOID(low_power_info);
    }
    low_power_info->force_update = true;
    low_power_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_battery_low_power_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && low_power_info && low_power_info->root);
    ui_img_baj_release_decoded_buf(low_power_info->frame_img);
    LVGL_OBJ_DELETE(low_power_info->root);
    u_free(low_power_info);
    low_power_info = NULL;
}

int ui_display_battery_low_power_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && low_power_info && low_power_info->root);
    low_power_bg_update(thiz);
    low_power_icon_update(thiz);
    low_power_text_update();
    low_power_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        LOG_UI("===> battery low power fps=%d", thiz->priv.fps_count);
        thiz->priv.fps_start_time = get_timestamp();
        thiz->priv.fps_count = 0;
    }
    return thiz->priv.fps_count;
}
