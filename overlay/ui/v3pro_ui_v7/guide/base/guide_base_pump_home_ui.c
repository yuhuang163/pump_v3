/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
/** 参考第四页泵奶启动页：无波形，时间下移 31，固定 00:00 */
#define GUIDE_PUMP_HOME_SCREEN_W                    (360)
#define GUIDE_PUMP_HOME_BG_RES_ID                   (BASE_RES_ID_SIMULATION_MODE_BG)

#define GUIDE_PUMP_HOME_MODE_TITLE_COLOR            (0xFFFFFF)

#define GUIDE_PUMP_HOME_TIME_DIGIT_COUNT            (2)
#define GUIDE_PUMP_HOME_TIME_SEG_WIDTH              (88)
#define GUIDE_PUMP_HOME_TIME_M_X                    (75 + 4)
#define GUIDE_PUMP_HOME_TIME_S_X                    (189 + 4)
#define GUIDE_PUMP_HOME_TIME_Y                      (122 + 4 + 31)
#define GUIDE_PUMP_HOME_TIME_COLON_Y                (128 + 4 + 31)
#define GUIDE_PUMP_HOME_TIME_COLOR                  (0xFFFFFF)

#define GUIDE_PUMP_HOME_SPEED_GROUP_WIDTH           (47)
#define GUIDE_PUMP_HOME_SPEED_ICON_Y                (265 + 4)
#define GUIDE_PUMP_HOME_SPEED_VALUE_Y               (267 + 4)
#define GUIDE_PUMP_HOME_SPEED_COLOR                 (0xFFFFFF)
#define GUIDE_PUMP_HOME_SPEED_ICON_VALUE_GAP        (6)
#define GUIDE_PUMP_HOME_SPEED_VALUE                 (0) /// 0 基，界面显示 1

#define GUIDE_PUMP_HOME_LEFT_LEVEL_X                (68 + 4)
#define GUIDE_PUMP_HOME_LEFT_LEVEL_Y                (267 + 4)
#define GUIDE_PUMP_HOME_LEFT_LEVEL_W                (58)
#define GUIDE_PUMP_HOME_RIGHT_LEVEL_X               (227 + 4)
#define GUIDE_PUMP_HOME_RIGHT_LEVEL_Y               (GUIDE_PUMP_HOME_LEFT_LEVEL_Y)
#define GUIDE_PUMP_HOME_RIGHT_LEVEL_W               (58)
#define GUIDE_PUMP_HOME_LEVEL_COLOR                 (0xFFFFFF)
#define GUIDE_PUMP_HOME_LEFT_LEVEL                  (1)
#define GUIDE_PUMP_HOME_RIGHT_LEVEL                 (1)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_pump_home_bg_detach(guide_base_ui_info_t *info)
{
    const void *src;

    if (info->pump_home.bg == NULL || !lv_obj_is_valid(info->pump_home.bg)) {
        return;
    }
    src = lv_image_get_src(info->pump_home.bg);
    if (src == NULL) {
        return;
    }
    if (get_predecoded_bg(info->pump_home.shown_bg_id) == src) {
        lv_image_set_src(info->pump_home.bg, NULL);
        return;
    }
    ui_img_png_release_decoded_buf(info->pump_home.bg);
}

static void guide_pump_home_bg_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    lv_draw_buf_t *predecoded;

    EXCEPTION_CHECK_VOID(parent);
    if (info->pump_home.bg == NULL || !lv_obj_is_valid(info->pump_home.bg)) {
        info->pump_home.bg = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->pump_home.bg);
        lv_obj_move_background(info->pump_home.bg);
    }

    guide_pump_home_bg_detach(info);
    predecoded = get_predecoded_bg(GUIDE_PUMP_HOME_BG_RES_ID);
    if (predecoded != NULL) {
        lv_image_set_src(info->pump_home.bg, predecoded);
    } else if (info->ui != NULL) {
        ui_img_png_theme_set_src(info->ui, info->pump_home.bg, GUIDE_PUMP_HOME_BG_RES_ID,
            RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    }
    lv_obj_align(info->pump_home.bg, LV_ALIGN_CENTER, 0, 0);
    info->pump_home.shown_bg_id = GUIDE_PUMP_HOME_BG_RES_ID;
}

static void guide_pump_home_title_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent);
    if (info->pump_home.mode_title != NULL && lv_obj_is_valid(info->pump_home.mode_title)) {
        return;
    }
    info->pump_home.mode_title = lv_label_create(parent);
    EXCEPTION_CHECK_VOID(info->pump_home.mode_title);
    lv_label_set_text(info->pump_home.mode_title, GUIDE_PUMP_HOME_MODE_TITLE_TEXT);
    lv_obj_set_style_text_color(info->pump_home.mode_title,
        lv_color_hex(GUIDE_PUMP_HOME_MODE_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(info->pump_home.mode_title, UI_FONT_TITLE, 0);
    lv_obj_set_style_text_align(info->pump_home.mode_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(info->pump_home.mode_title, LV_ALIGN_TOP_MID, 0, GUIDE_PUMP_HOME_MODE_TITLE_Y);
}

static void guide_pump_home_time_pair_create(lv_obj_t *labels[GUIDE_PUMP_HOME_TIME_DIGIT_COUNT],
    lv_obj_t *parent, uint8_t value, lv_coord_t seg_x)
{
    const lv_coord_t cell_w = GUIDE_PUMP_HOME_TIME_SEG_WIDTH / GUIDE_PUMP_HOME_TIME_DIGIT_COUNT;
    char digit_text[2] = {0};
    int i;

    for (i = 0; i < GUIDE_PUMP_HOME_TIME_DIGIT_COUNT; i++) {
        if (labels[i] == NULL || !lv_obj_is_valid(labels[i])) {
            labels[i] = lv_label_create(parent);
            EXCEPTION_CHECK_VOID(labels[i]);
            lv_label_set_long_mode(labels[i], LV_LABEL_LONG_CLIP);
            lv_obj_set_style_text_align(labels[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(labels[i], lv_color_hex(GUIDE_PUMP_HOME_TIME_COLOR), 0);
            lv_obj_set_style_text_font(labels[i], UI_FONT_NUM_LG, 0);
            lv_obj_set_style_text_letter_space(labels[i], 0, 0);
        }
        digit_text[0] = (char)('0' + (i == 0 ? value / 10 : value % 10));
        digit_text[1] = '\0';
        lv_obj_set_width(labels[i], cell_w);
        lv_label_set_text(labels[i], digit_text);
        lv_obj_align(labels[i], LV_ALIGN_TOP_LEFT, seg_x + i * cell_w, GUIDE_PUMP_HOME_TIME_Y);
    }
}

static void guide_pump_home_time_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent && info->ui);
    guide_pump_home_time_pair_create(info->pump_home.time_m, parent, 0, GUIDE_PUMP_HOME_TIME_M_X);
    guide_pump_home_time_pair_create(info->pump_home.time_s, parent, 0, GUIDE_PUMP_HOME_TIME_S_X);

    if (info->pump_home.time_colon == NULL || !lv_obj_is_valid(info->pump_home.time_colon)) {
        info->pump_home.time_colon = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->pump_home.time_colon);
        ui_img_baj_theme_set_src(info->ui, info->pump_home.time_colon, BASE_RES_ID_CLOCK_COLON_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(info->pump_home.time_colon, LV_ALIGN_TOP_MID, 0, GUIDE_PUMP_HOME_TIME_COLON_Y);
}

static void guide_pump_home_speed_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    char speed_text[8] = {0};
    lv_coord_t icon_w;
    lv_coord_t value_w;
    lv_coord_t total_w;
    lv_coord_t start_x;

    EXCEPTION_CHECK_VOID(parent && info->ui);
    if (info->pump_home.speed_icon == NULL || !lv_obj_is_valid(info->pump_home.speed_icon)) {
        info->pump_home.speed_icon = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->pump_home.speed_icon);
        ui_img_baj_theme_set_src(info->ui, info->pump_home.speed_icon, BASE_RES_ID_SPEED_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (info->pump_home.speed_value == NULL || !lv_obj_is_valid(info->pump_home.speed_value)) {
        info->pump_home.speed_value = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(info->pump_home.speed_value);
        lv_obj_set_style_text_color(info->pump_home.speed_value,
            lv_color_hex(GUIDE_PUMP_HOME_SPEED_COLOR), 0);
        lv_obj_set_style_text_font(info->pump_home.speed_value, UI_FONT_TITLE, 0);
    }

    snprintf(speed_text, sizeof(speed_text), "%u", (unsigned)(GUIDE_PUMP_HOME_SPEED_VALUE + 1));
    lv_label_set_text(info->pump_home.speed_value, speed_text);
    lv_obj_update_layout(info->pump_home.speed_icon);
    lv_obj_update_layout(info->pump_home.speed_value);
    icon_w = lv_obj_get_width(info->pump_home.speed_icon);
    value_w = lv_obj_get_width(info->pump_home.speed_value);
    total_w = icon_w + GUIDE_PUMP_HOME_SPEED_ICON_VALUE_GAP + value_w;
    if (total_w > GUIDE_PUMP_HOME_SPEED_GROUP_WIDTH) {
        total_w = GUIDE_PUMP_HOME_SPEED_GROUP_WIDTH;
    }
    start_x = (GUIDE_PUMP_HOME_SCREEN_W - total_w) / 2;
    lv_obj_align(info->pump_home.speed_icon, LV_ALIGN_TOP_LEFT, start_x, GUIDE_PUMP_HOME_SPEED_ICON_Y);
    lv_obj_align(info->pump_home.speed_value, LV_ALIGN_TOP_LEFT,
        start_x + icon_w + GUIDE_PUMP_HOME_SPEED_ICON_VALUE_GAP, GUIDE_PUMP_HOME_SPEED_VALUE_Y);
}

static void guide_pump_home_level_create(lv_obj_t **label, lv_obj_t *parent, const char *prefix,
    uint8_t level, lv_coord_t x, lv_coord_t y, lv_coord_t width, lv_text_align_t align)
{
    char text[8] = {0};

    if (*label == NULL || !lv_obj_is_valid(*label)) {
        *label = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(*label);
        lv_label_set_long_mode(*label, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(*label, width);
        lv_obj_set_style_text_color(*label, lv_color_hex(GUIDE_PUMP_HOME_LEVEL_COLOR), 0);
        lv_obj_set_style_text_font(*label, UI_FONT_TITLE, 0);
        lv_obj_set_style_text_align(*label, align, 0);
        lv_obj_set_style_text_letter_space(*label, 3, 0);
    }
    snprintf(text, sizeof(text), "%s%u", prefix, (unsigned)level);
    lv_label_set_text(*label, text);
    lv_obj_align(*label, LV_ALIGN_TOP_LEFT, x, y);
}

static void guide_pump_home_levels_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent);
    guide_pump_home_level_create(&info->pump_home.left_level, parent, "L",
        GUIDE_PUMP_HOME_LEFT_LEVEL, GUIDE_PUMP_HOME_LEFT_LEVEL_X, GUIDE_PUMP_HOME_LEFT_LEVEL_Y,
        GUIDE_PUMP_HOME_LEFT_LEVEL_W, LV_TEXT_ALIGN_RIGHT);
    guide_pump_home_level_create(&info->pump_home.right_level, parent, "R",
        GUIDE_PUMP_HOME_RIGHT_LEVEL, GUIDE_PUMP_HOME_RIGHT_LEVEL_X, GUIDE_PUMP_HOME_RIGHT_LEVEL_Y,
        GUIDE_PUMP_HOME_RIGHT_LEVEL_W, LV_TEXT_ALIGN_LEFT);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_pump_home_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_pump_home_bg_detach(info);
    ui_img_baj_release_decoded_buf(info->pump_home.time_colon);
    ui_img_baj_release_decoded_buf(info->pump_home.speed_icon);
    memset(&info->pump_home, 0, sizeof(info->pump_home));
}

void guide_pump_home_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_pump_home_bg_create(info);
    guide_pump_home_title_create(info);
    guide_pump_home_time_create(info);
    guide_pump_home_speed_create(info);
    guide_pump_home_levels_create(info);
}

void guide_pump_home_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc)
{
    lv_opa_t opa;

    EXCEPTION_CHECK_VOID(info && proc);
    if (info->pump_home.mode_title == NULL || !lv_obj_is_valid(info->pump_home.mode_title)) {
        return;
    }
    opa = (proc->state_blink_visible != 0) ? LV_OPA_COVER : LV_OPA_TRANSP;
    lv_obj_set_style_text_opa(info->pump_home.mode_title, opa, 0);
}
