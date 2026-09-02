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
#define LOW_POWER_ICON_Y                                      (94)
#define LOW_POWER_TEXT_COLOR                                  (0xFFFFFF)
#define LOW_POWER_COUNTDOWN_Y                                 (331)
#define LOW_POWER_COUNTDOWN_COLOR                             (0x999999)
#define LOW_POWER_COUNTDOWN_MAX                               (3)

typedef struct {
    lv_obj_t * root;
    lv_obj_t * background;
    lv_obj_t * icon;
    lv_obj_t * text;
    lv_obj_t * countdown;
    uint32_t start_time;
    uint8_t force_update;
    uint8_t show_countdown;
    uint8_t shown_value;
    uint8_t rsv[13];
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

static const char *low_power_text;

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

static void low_power_icon_update(ui_manager_context_t *thiz)
{
    if (low_power_info->force_update == false) {
        return;
    }

    if (low_power_info->icon == NULL || !lv_obj_is_valid(low_power_info->icon)) {
        low_power_info->icon = lv_image_create(low_power_info->root);
        EXCEPTION_CHECK_VOID(low_power_info->icon);
    }

    ui_img_baj_theme_set_src(thiz, low_power_info->icon,
        BASE_RES_ID_LOW_POWER_PAGE_ICON, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    lv_obj_clear_flag(low_power_info->icon, LV_OBJ_FLAG_HIDDEN);
    lv_obj_align(low_power_info->icon, LV_ALIGN_TOP_MID, 0, LOW_POWER_ICON_Y);
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
    lv_label_set_text(low_power_info->text, low_power_text ? low_power_text : "");
    lv_obj_set_style_text_color(low_power_info->text, lv_color_hex(LOW_POWER_TEXT_COLOR), 0);
    lv_obj_set_style_text_font(low_power_info->text, UI_FONT_TITLE, 0);
    lv_obj_set_style_text_line_space(low_power_info->text, 8, 0);
    lv_obj_set_style_text_align(low_power_info->text, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(low_power_info->text, LV_ALIGN_TOP_MID, 0, LOW_POWER_TEXT_Y);
}

static void low_power_countdown_update(void)
{
    char num_buf[8] = {0};
    uint8_t num;
    uint8_t show_num;

    if (low_power_info->show_countdown == 0) {
        if (low_power_info->countdown != NULL && lv_obj_is_valid(low_power_info->countdown)) {
            lv_obj_add_flag(low_power_info->countdown, LV_OBJ_FLAG_HIDDEN);
        }
        return;
    }

    if (low_power_info->countdown == NULL || !lv_obj_is_valid(low_power_info->countdown)) {
        low_power_info->countdown = lv_label_create(low_power_info->root);
        EXCEPTION_CHECK_VOID(low_power_info->countdown);
    }

    num = get_timestamp_ms(low_power_info->start_time) / 1000;
    show_num = (num > LOW_POWER_COUNTDOWN_MAX) ? 0 : (LOW_POWER_COUNTDOWN_MAX - num);
    if (low_power_info->force_update == false &&
        low_power_info->shown_value == show_num) {
        return;
    }

    snprintf(num_buf, sizeof(num_buf), UI_STR_COUNTDOWN_SEC_FMT,
        (unsigned)(show_num == 0 ? 1 : show_num));
    lv_label_set_text(low_power_info->countdown, num_buf);
    lv_obj_set_style_text_color(low_power_info->countdown, lv_color_hex(LOW_POWER_COUNTDOWN_COLOR), 0);
    lv_obj_set_style_text_font(low_power_info->countdown, UI_FONT_HINT, 0);
    lv_obj_set_style_text_align(low_power_info->countdown, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(low_power_info->countdown, LV_ALIGN_TOP_MID, 0, LOW_POWER_COUNTDOWN_Y);
    low_power_info->shown_value = show_num;
    lv_obj_clear_flag(low_power_info->countdown, LV_OBJ_FLAG_HIDDEN);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_battery_low_power_prepare(ui_manager_context_t *thiz, uint8_t show_countdown,
    const char *text)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (low_power_info == NULL) {
        low_power_info = (low_power_display_info_t *)u_malloz(sizeof(low_power_display_info_t));
        EXCEPTION_CHECK_VOID(low_power_info);
    }
    low_power_text = (text != NULL) ? text : LOW_POWER_PAGE_TEXT;
    low_power_info->force_update = true;
    low_power_info->show_countdown = show_countdown;
    low_power_info->shown_value = 0xFF;
    low_power_info->start_time = get_timestamp();
    low_power_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_battery_low_power_restart_countdown(void)
{
    if (low_power_info == NULL) {
        return;
    }
    low_power_info->start_time = get_timestamp();
    low_power_info->shown_value = 0xFF;
}

void ui_display_battery_low_power_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && low_power_info && low_power_info->root);
    ui_img_baj_release_decoded_buf(low_power_info->icon);
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
    low_power_countdown_update();
    low_power_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        LOG_UI("===> battery low power fps=%d", thiz->priv.fps_count);
        thiz->priv.fps_start_time = get_timestamp();
        thiz->priv.fps_count = 0;
    }
    return thiz->priv.fps_count;
}
