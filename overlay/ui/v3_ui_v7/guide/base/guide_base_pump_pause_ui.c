/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "ui_img_resource.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
/** 固定刺激模式泵奶暂停态，坐标对齐 pump_home_base_home_page_ui.c / animation.c */
#define GUIDE_PUMP_PAUSE_SCREEN_W                   (360)
#define GUIDE_PUMP_PAUSE_BG_RES_ID                  (BASE_RES_ID_SIMULATION_MODE_BG)

#define GUIDE_PUMP_PAUSE_MODE_TITLE_Y               (64 + 4)
#define GUIDE_PUMP_PAUSE_MODE_TITLE_COLOR           (0xFFFFFF)
#define GUIDE_PUMP_PAUSE_MODE_TITLE_TEXT            "Stimulation"

#define GUIDE_PUMP_PAUSE_WAVE_Y                     (190 - 46) /// 暂停态相对运行态上移
#define GUIDE_PUMP_PAUSE_WAVE_OPA                   (LV_OPA_60)
#define GUIDE_PUMP_PAUSE_WAVE_RES_ID                (BASE_RES_ID_STIMULATION_ANIM_START)

#define GUIDE_PUMP_PAUSE_PLAY_ICON_Y               (136)
#define GUIDE_PUMP_PAUSE_PLAY_ICON_RES_ID          (BASE_RES_ID_PLAY_ICON)

#define GUIDE_PUMP_PAUSE_SPEED_GROUP_WIDTH          (47)
#define GUIDE_PUMP_PAUSE_SPEED_ICON_Y               (265 + 4)
#define GUIDE_PUMP_PAUSE_SPEED_VALUE_Y              (267 + 4)
#define GUIDE_PUMP_PAUSE_SPEED_COLOR                (0xFFFFFF)
#define GUIDE_PUMP_PAUSE_SPEED_ICON_VALUE_GAP       (6)
#define GUIDE_PUMP_PAUSE_SPEED_VALUE                (0) /// 0 基，界面显示 1

#define GUIDE_PUMP_PAUSE_LEFT_LEVEL_X               (68 + 4)
#define GUIDE_PUMP_PAUSE_LEFT_LEVEL_Y               (267 + 4)
#define GUIDE_PUMP_PAUSE_LEFT_LEVEL_W               (58)
#define GUIDE_PUMP_PAUSE_RIGHT_LEVEL_X              (227 + 4)
#define GUIDE_PUMP_PAUSE_RIGHT_LEVEL_Y              (GUIDE_PUMP_PAUSE_LEFT_LEVEL_Y)
#define GUIDE_PUMP_PAUSE_RIGHT_LEVEL_W              (58)
#define GUIDE_PUMP_PAUSE_LEVEL_COLOR                (0xFFFFFF)
#define GUIDE_PUMP_PAUSE_LEFT_LEVEL                 (1)
#define GUIDE_PUMP_PAUSE_RIGHT_LEVEL                (1)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_pump_pause_bg_detach(guide_base_ui_info_t *info)
{
    const void *src;

    if (info->pump_pause.bg == NULL || !lv_obj_is_valid(info->pump_pause.bg)) {
        return;
    }
    src = lv_image_get_src(info->pump_pause.bg);
    if (src == NULL) {
        return;
    }
    if (get_predecoded_bg(info->pump_pause.shown_bg_id) == src) {
        lv_image_set_src(info->pump_pause.bg, NULL);
        return;
    }
    ui_img_png_release_decoded_buf(info->pump_pause.bg);
}

static void guide_pump_pause_bg_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    lv_draw_buf_t *predecoded;

    EXCEPTION_CHECK_VOID(parent);
    if (info->pump_pause.bg == NULL || !lv_obj_is_valid(info->pump_pause.bg)) {
        info->pump_pause.bg = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->pump_pause.bg);
        lv_obj_move_background(info->pump_pause.bg);
    }

    guide_pump_pause_bg_detach(info);
    predecoded = get_predecoded_bg(GUIDE_PUMP_PAUSE_BG_RES_ID);
    if (predecoded != NULL) {
        lv_image_set_src(info->pump_pause.bg, predecoded);
    } else if (info->ui != NULL) {
        ui_img_png_theme_set_src(info->ui, info->pump_pause.bg, GUIDE_PUMP_PAUSE_BG_RES_ID,
            RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    }
    lv_obj_align(info->pump_pause.bg, LV_ALIGN_CENTER, 0, 0);
    info->pump_pause.shown_bg_id = GUIDE_PUMP_PAUSE_BG_RES_ID;
}

static void guide_pump_pause_title_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent);
    if (info->pump_pause.mode_title != NULL && lv_obj_is_valid(info->pump_pause.mode_title)) {
        return;
    }
    info->pump_pause.mode_title = lv_label_create(parent);
    EXCEPTION_CHECK_VOID(info->pump_pause.mode_title);
    lv_label_set_text(info->pump_pause.mode_title, GUIDE_PUMP_PAUSE_MODE_TITLE_TEXT);
    lv_obj_set_style_text_color(info->pump_pause.mode_title,
        lv_color_hex(GUIDE_PUMP_PAUSE_MODE_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(info->pump_pause.mode_title, &lv_font_aeonik_soft_26_medium, 0);
    lv_obj_set_style_text_align(info->pump_pause.mode_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(info->pump_pause.mode_title, LV_ALIGN_TOP_MID, 0, GUIDE_PUMP_PAUSE_MODE_TITLE_Y);
}

static void guide_pump_pause_wave_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    lv_fs_path_ex_t img_path;

    EXCEPTION_CHECK_VOID(parent && info->ui);
    if (info->pump_pause.wave == NULL || !lv_obj_is_valid(info->pump_pause.wave)) {
        info->pump_pause.wave = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->pump_pause.wave);
    }

    if (info->pump_pause.wave_info.data != NULL) {
        ui_img_resource_deinit(info->ui, &info->pump_pause.wave_info);
        memset(&info->pump_pause.wave_info, 0, sizeof(info->pump_pause.wave_info));
    }

    ui_theme_img_resource_init(info->ui, GUIDE_PUMP_PAUSE_WAVE_RES_ID, RES_THEME_TYPE_BASE,
        &info->pump_pause.wave_info);
    EXCEPTION_CHECK_VOID(info->pump_pause.wave_info.data);
    lv_fs_make_path_from_buffer(&img_path, LV_FS_MEMFS_LETTER,
        info->pump_pause.wave_info.data, info->pump_pause.wave_info.size, "baj");
    lv_image_set_src(info->pump_pause.wave, (const void *)&img_path);
    lv_obj_set_style_opa(info->pump_pause.wave, GUIDE_PUMP_PAUSE_WAVE_OPA, 0);
    lv_obj_align(info->pump_pause.wave, LV_ALIGN_TOP_MID, 0, GUIDE_PUMP_PAUSE_WAVE_Y);
}

static void guide_pump_pause_play_icon_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent && info->ui);
    if (info->pump_pause.play_icon == NULL || !lv_obj_is_valid(info->pump_pause.play_icon)) {
        info->pump_pause.play_icon = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->pump_pause.play_icon);
        ui_img_baj_theme_set_src(info->ui, info->pump_pause.play_icon,
            GUIDE_PUMP_PAUSE_PLAY_ICON_RES_ID, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(info->pump_pause.play_icon, LV_ALIGN_TOP_MID, 0, GUIDE_PUMP_PAUSE_PLAY_ICON_Y);
    lv_obj_move_foreground(info->pump_pause.play_icon);
}

static void guide_pump_pause_speed_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    char speed_text[8] = {0};
    lv_coord_t icon_w;
    lv_coord_t value_w;
    lv_coord_t total_w;
    lv_coord_t start_x;

    EXCEPTION_CHECK_VOID(parent && info->ui);
    if (info->pump_pause.speed_icon == NULL || !lv_obj_is_valid(info->pump_pause.speed_icon)) {
        info->pump_pause.speed_icon = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->pump_pause.speed_icon);
        ui_img_baj_theme_set_src(info->ui, info->pump_pause.speed_icon, BASE_RES_ID_SPEED_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (info->pump_pause.speed_value == NULL || !lv_obj_is_valid(info->pump_pause.speed_value)) {
        info->pump_pause.speed_value = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(info->pump_pause.speed_value);
        lv_obj_set_style_text_color(info->pump_pause.speed_value,
            lv_color_hex(GUIDE_PUMP_PAUSE_SPEED_COLOR), 0);
        lv_obj_set_style_text_font(info->pump_pause.speed_value, &lv_font_aeonik_soft_26_medium, 0);
    }

    snprintf(speed_text, sizeof(speed_text), "%u", (unsigned)(GUIDE_PUMP_PAUSE_SPEED_VALUE + 1));
    lv_label_set_text(info->pump_pause.speed_value, speed_text);
    lv_obj_update_layout(info->pump_pause.speed_icon);
    lv_obj_update_layout(info->pump_pause.speed_value);
    icon_w = lv_obj_get_width(info->pump_pause.speed_icon);
    value_w = lv_obj_get_width(info->pump_pause.speed_value);
    total_w = icon_w + GUIDE_PUMP_PAUSE_SPEED_ICON_VALUE_GAP + value_w;
    if (total_w > GUIDE_PUMP_PAUSE_SPEED_GROUP_WIDTH) {
        total_w = GUIDE_PUMP_PAUSE_SPEED_GROUP_WIDTH;
    }
    start_x = (GUIDE_PUMP_PAUSE_SCREEN_W - total_w) / 2;
    lv_obj_align(info->pump_pause.speed_icon, LV_ALIGN_TOP_LEFT, start_x, GUIDE_PUMP_PAUSE_SPEED_ICON_Y);
    lv_obj_align(info->pump_pause.speed_value, LV_ALIGN_TOP_LEFT,
        start_x + icon_w + GUIDE_PUMP_PAUSE_SPEED_ICON_VALUE_GAP, GUIDE_PUMP_PAUSE_SPEED_VALUE_Y);
}

static void guide_pump_pause_level_create(lv_obj_t **label, lv_obj_t *parent, const char *prefix,
    uint8_t level, lv_coord_t x, lv_coord_t y, lv_coord_t width, lv_text_align_t align)
{
    char text[8] = {0};

    if (*label == NULL || !lv_obj_is_valid(*label)) {
        *label = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(*label);
        lv_label_set_long_mode(*label, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(*label, width);
        lv_obj_set_style_text_color(*label, lv_color_hex(GUIDE_PUMP_PAUSE_LEVEL_COLOR), 0);
        lv_obj_set_style_text_font(*label, &lv_font_aeonik_soft_26_medium, 0);
        lv_obj_set_style_text_align(*label, align, 0);
        lv_obj_set_style_text_letter_space(*label, 3, 0);
    }
    snprintf(text, sizeof(text), "%s%u", prefix, (unsigned)level);
    lv_label_set_text(*label, text);
    lv_obj_align(*label, LV_ALIGN_TOP_LEFT, x, y);
}

static void guide_pump_pause_levels_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent);
    guide_pump_pause_level_create(&info->pump_pause.left_level, parent, "L",
        GUIDE_PUMP_PAUSE_LEFT_LEVEL, GUIDE_PUMP_PAUSE_LEFT_LEVEL_X, GUIDE_PUMP_PAUSE_LEFT_LEVEL_Y,
        GUIDE_PUMP_PAUSE_LEFT_LEVEL_W, LV_TEXT_ALIGN_RIGHT);
    guide_pump_pause_level_create(&info->pump_pause.right_level, parent, "R",
        GUIDE_PUMP_PAUSE_RIGHT_LEVEL, GUIDE_PUMP_PAUSE_RIGHT_LEVEL_X, GUIDE_PUMP_PAUSE_RIGHT_LEVEL_Y,
        GUIDE_PUMP_PAUSE_RIGHT_LEVEL_W, LV_TEXT_ALIGN_LEFT);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_pump_pause_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_pump_pause_bg_detach(info);
    if (info->ui != NULL && info->pump_pause.wave_info.data != NULL) {
        ui_img_resource_deinit(info->ui, &info->pump_pause.wave_info);
    }
    ui_img_baj_release_decoded_buf(info->pump_pause.play_icon);
    ui_img_baj_release_decoded_buf(info->pump_pause.speed_icon);
    memset(&info->pump_pause, 0, sizeof(info->pump_pause));
}

void guide_pump_pause_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_pump_pause_bg_create(info);
    guide_pump_pause_title_create(info);
    guide_pump_pause_wave_create(info);
    guide_pump_pause_play_icon_create(info);
    guide_pump_pause_speed_create(info);
    guide_pump_pause_levels_create(info);
}
