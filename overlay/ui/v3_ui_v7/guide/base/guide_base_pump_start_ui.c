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
/** 固定刺激模式泵奶首页布局，坐标对齐 pump_home_base_home_page_ui.c */
#define GUIDE_PUMP_START_SCREEN_W                   (360)
#define GUIDE_PUMP_START_BG_RES_ID                  (BASE_RES_ID_SIMULATION_MODE_BG)

#define GUIDE_PUMP_START_MODE_TITLE_COLOR           (0xFFFFFF)

#define GUIDE_PUMP_START_TIME_DIGIT_COUNT           (2)
#define GUIDE_PUMP_START_TIME_SEG_WIDTH             (88)
#define GUIDE_PUMP_START_TIME_M_X                   (75 + 4)
#define GUIDE_PUMP_START_TIME_S_X                   (189 + 4)
#define GUIDE_PUMP_START_TIME_Y                     (122 + 4)
#define GUIDE_PUMP_START_TIME_COLON_Y               (128 + 4)
#define GUIDE_PUMP_START_TIME_COLOR                 (0xFFFFFF)
#define GUIDE_PUMP_START_TIME_MAX_MINUTES           (99)

#define GUIDE_PUMP_START_WAVE_Y                     (190)
#define GUIDE_PUMP_START_WAVE_START_ID              (BASE_RES_ID_STIMULATION_ANIM_START)
#define GUIDE_PUMP_START_WAVE_END_ID                (BASE_RES_ID_STIMULATION_ANIM_END)
#define GUIDE_PUMP_START_ANIM_FREQ                  (20)
#define GUIDE_PUMP_START_ANIM_FRAME_INTERVAL_MS     (1000 / GUIDE_PUMP_START_ANIM_FREQ)
#define GUIDE_PUMP_START_ANIM_FRAME_RES_INVALID     (0xFFFF)

#define GUIDE_PUMP_START_SPEED_GROUP_WIDTH          (47)
#define GUIDE_PUMP_START_SPEED_ICON_Y               (265 + 4)
#define GUIDE_PUMP_START_SPEED_VALUE_Y              (267 + 4)
#define GUIDE_PUMP_START_SPEED_COLOR                (0xFFFFFF)
#define GUIDE_PUMP_START_SPEED_ICON_VALUE_GAP       (6)
#define GUIDE_PUMP_START_SPEED_VALUE                (0) /// 0 基，界面显示 1

#define GUIDE_PUMP_START_LEFT_LEVEL_X               (68 + 4)
#define GUIDE_PUMP_START_LEFT_LEVEL_Y               (267 + 4)
#define GUIDE_PUMP_START_LEFT_LEVEL_W               (58)
#define GUIDE_PUMP_START_RIGHT_LEVEL_X              (227 + 4)
#define GUIDE_PUMP_START_RIGHT_LEVEL_Y              (GUIDE_PUMP_START_LEFT_LEVEL_Y)
#define GUIDE_PUMP_START_RIGHT_LEVEL_W              (58)
#define GUIDE_PUMP_START_LEVEL_COLOR                (0xFFFFFF)
#define GUIDE_PUMP_START_LEFT_LEVEL                 (1)
#define GUIDE_PUMP_START_RIGHT_LEVEL                (1)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void guide_pump_start_bg_detach(guide_base_ui_info_t *info)
{
    const void *src;

    if (info->pump_start.bg == NULL || !lv_obj_is_valid(info->pump_start.bg)) {
        return;
    }
    src = lv_image_get_src(info->pump_start.bg);
    if (src == NULL) {
        return;
    }
    if (get_predecoded_bg(info->pump_start.shown_bg_id) == src) {
        lv_image_set_src(info->pump_start.bg, NULL);
        return;
    }
    ui_img_png_release_decoded_buf(info->pump_start.bg);
}

static void guide_pump_start_bg_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    lv_draw_buf_t *predecoded;

    EXCEPTION_CHECK_VOID(parent);
    if (info->pump_start.bg == NULL || !lv_obj_is_valid(info->pump_start.bg)) {
        info->pump_start.bg = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->pump_start.bg);
        lv_obj_move_background(info->pump_start.bg);
    }

    guide_pump_start_bg_detach(info);
    predecoded = get_predecoded_bg(GUIDE_PUMP_START_BG_RES_ID);
    if (predecoded != NULL) {
        lv_image_set_src(info->pump_start.bg, predecoded);
    } else if (info->ui != NULL) {
        ui_img_png_theme_set_src(info->ui, info->pump_start.bg, GUIDE_PUMP_START_BG_RES_ID,
            RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    }
    lv_obj_align(info->pump_start.bg, LV_ALIGN_CENTER, 0, 0);
    info->pump_start.shown_bg_id = GUIDE_PUMP_START_BG_RES_ID;
}

static void guide_pump_start_title_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent);
    if (info->pump_start.mode_title != NULL && lv_obj_is_valid(info->pump_start.mode_title)) {
        return;
    }
    info->pump_start.mode_title = lv_label_create(parent);
    EXCEPTION_CHECK_VOID(info->pump_start.mode_title);
    lv_label_set_text(info->pump_start.mode_title, GUIDE_PUMP_START_MODE_TITLE_TEXT);
    lv_obj_set_style_text_color(info->pump_start.mode_title,
        lv_color_hex(GUIDE_PUMP_START_MODE_TITLE_COLOR), 0);
    lv_obj_set_style_text_font(info->pump_start.mode_title, UI_FONT_TITLE, 0);
    lv_obj_set_style_text_align(info->pump_start.mode_title, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(info->pump_start.mode_title, LV_ALIGN_TOP_MID, 0, GUIDE_PUMP_START_MODE_TITLE_Y);
}

static void guide_pump_start_time_pair_update(lv_obj_t *labels[GUIDE_PUMP_START_TIME_DIGIT_COUNT],
    lv_obj_t *parent, uint8_t value, lv_coord_t seg_x)
{
    const lv_coord_t cell_w = GUIDE_PUMP_START_TIME_SEG_WIDTH / GUIDE_PUMP_START_TIME_DIGIT_COUNT;
    char digit_text[2] = {0};
    int i;

    for (i = 0; i < GUIDE_PUMP_START_TIME_DIGIT_COUNT; i++) {
        if (labels[i] == NULL || !lv_obj_is_valid(labels[i])) {
            labels[i] = lv_label_create(parent);
            EXCEPTION_CHECK_VOID(labels[i]);
            lv_label_set_long_mode(labels[i], LV_LABEL_LONG_CLIP);
            lv_obj_set_style_text_align(labels[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(labels[i], lv_color_hex(GUIDE_PUMP_START_TIME_COLOR), 0);
            lv_obj_set_style_text_font(labels[i], UI_FONT_NUM_LG, 0);
            lv_obj_set_style_text_letter_space(labels[i], 0, 0);
        }
        digit_text[0] = (char)('0' + (i == 0 ? value / 10 : value % 10));
        digit_text[1] = '\0';
        lv_obj_set_width(labels[i], cell_w);
        lv_label_set_text(labels[i], digit_text);
        lv_obj_align(labels[i], LV_ALIGN_TOP_LEFT, seg_x + i * cell_w, GUIDE_PUMP_START_TIME_Y);
    }
}

static void guide_pump_start_time_set(guide_base_ui_info_t *info, uint8_t minutes, uint8_t seconds)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent);
    if (info->pump_start.time_inited != 0 &&
        info->pump_start.shown_minutes == minutes &&
        info->pump_start.shown_seconds == seconds) {
        return;
    }

    guide_pump_start_time_pair_update(info->pump_start.time_m, parent, minutes,
        GUIDE_PUMP_START_TIME_M_X);
    guide_pump_start_time_pair_update(info->pump_start.time_s, parent, seconds,
        GUIDE_PUMP_START_TIME_S_X);
    info->pump_start.shown_minutes = minutes;
    info->pump_start.shown_seconds = seconds;
    info->pump_start.time_inited = 1;
}

static void guide_pump_start_time_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent && info->ui);
    guide_pump_start_time_set(info, 0, 0);

    if (info->pump_start.time_colon == NULL || !lv_obj_is_valid(info->pump_start.time_colon)) {
        info->pump_start.time_colon = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->pump_start.time_colon);
        ui_img_baj_theme_set_src(info->ui, info->pump_start.time_colon, BASE_RES_ID_CLOCK_COLON_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(info->pump_start.time_colon, LV_ALIGN_TOP_MID, 0, GUIDE_PUMP_START_TIME_COLON_Y);
}

static void guide_pump_start_wave_prepare(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent);
    if (info->pump_start.wave == NULL || !lv_obj_is_valid(info->pump_start.wave)) {
        info->pump_start.wave = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->pump_start.wave);
        lv_obj_set_style_opa(info->pump_start.wave, LV_OPA_COVER, 0);
    }
    lv_obj_align(info->pump_start.wave, LV_ALIGN_TOP_MID, 0, GUIDE_PUMP_START_WAVE_Y);
}

static void guide_pump_start_wave_frame_show(guide_base_ui_info_t *info, uint16_t frame_id)
{
    lv_fs_path_ex_t img_path;

    EXCEPTION_CHECK_VOID(info->ui && info->pump_start.wave);
    if (info->pump_start.shown_frame_res_id == frame_id) {
        return;
    }

    if (info->pump_start.wave_info.data != NULL) {
        ui_img_resource_deinit(info->ui, &info->pump_start.wave_info);
        memset(&info->pump_start.wave_info, 0, sizeof(info->pump_start.wave_info));
    }

    ui_theme_img_resource_init(info->ui, frame_id, RES_THEME_TYPE_BASE, &info->pump_start.wave_info);
    EXCEPTION_CHECK_VOID(info->pump_start.wave_info.data);
    lv_fs_make_path_from_buffer(&img_path, LV_FS_MEMFS_LETTER,
        info->pump_start.wave_info.data, info->pump_start.wave_info.size, "baj");
    lv_image_set_src(info->pump_start.wave, (const void *)&img_path);
    info->pump_start.shown_frame_res_id = frame_id;
}

static void guide_pump_start_wave_create(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);
    guide_pump_start_wave_prepare(info);
    info->pump_start.frame_res_id = GUIDE_PUMP_START_WAVE_START_ID;
    info->pump_start.shown_frame_res_id = GUIDE_PUMP_START_ANIM_FRAME_RES_INVALID;
    info->pump_start.last_frame_tick_ms = 0;
    guide_pump_start_wave_frame_show(info, info->pump_start.frame_res_id);
    info->pump_start.frame_res_id++;
}

static void guide_pump_start_wave_frame_update(guide_base_ui_info_t *info)
{
    const uint32_t now = get_timestamp();

    if (info->pump_start.last_frame_tick_ms != 0 &&
        (now - info->pump_start.last_frame_tick_ms) < GUIDE_PUMP_START_ANIM_FRAME_INTERVAL_MS) {
        return;
    }
    info->pump_start.last_frame_tick_ms = now;

    if (info->pump_start.frame_res_id > GUIDE_PUMP_START_WAVE_END_ID) {
        info->pump_start.frame_res_id = GUIDE_PUMP_START_WAVE_START_ID;
    }
    guide_pump_start_wave_frame_show(info, info->pump_start.frame_res_id);
    info->pump_start.frame_res_id++;
}

static void guide_pump_start_time_tick_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc)
{
    uint32_t elapsed_sec;
    uint8_t minutes;
    uint8_t seconds;

    EXCEPTION_CHECK_VOID(proc);
    elapsed_sec = get_timestamp_ms(proc->page_enter_time) / 1000U;
    minutes = (uint8_t)((elapsed_sec / 60U) % (GUIDE_PUMP_START_TIME_MAX_MINUTES + 1U));
    seconds = (uint8_t)(elapsed_sec % 60U);
    guide_pump_start_time_set(info, minutes, seconds);
}

static void guide_pump_start_speed_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    char speed_text[8] = {0};
    lv_coord_t icon_w;
    lv_coord_t value_w;
    lv_coord_t total_w;
    lv_coord_t start_x;

    EXCEPTION_CHECK_VOID(parent && info->ui);
    if (info->pump_start.speed_icon == NULL || !lv_obj_is_valid(info->pump_start.speed_icon)) {
        info->pump_start.speed_icon = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->pump_start.speed_icon);
        ui_img_baj_theme_set_src(info->ui, info->pump_start.speed_icon, BASE_RES_ID_SPEED_ICON,
            RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (info->pump_start.speed_value == NULL || !lv_obj_is_valid(info->pump_start.speed_value)) {
        info->pump_start.speed_value = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(info->pump_start.speed_value);
        lv_obj_set_style_text_color(info->pump_start.speed_value,
            lv_color_hex(GUIDE_PUMP_START_SPEED_COLOR), 0);
        lv_obj_set_style_text_font(info->pump_start.speed_value, UI_FONT_TITLE, 0);
    }

    snprintf(speed_text, sizeof(speed_text), "%u", (unsigned)(GUIDE_PUMP_START_SPEED_VALUE + 1));
    lv_label_set_text(info->pump_start.speed_value, speed_text);
    lv_obj_update_layout(info->pump_start.speed_icon);
    lv_obj_update_layout(info->pump_start.speed_value);
    icon_w = lv_obj_get_width(info->pump_start.speed_icon);
    value_w = lv_obj_get_width(info->pump_start.speed_value);
    total_w = icon_w + GUIDE_PUMP_START_SPEED_ICON_VALUE_GAP + value_w;
    if (total_w > GUIDE_PUMP_START_SPEED_GROUP_WIDTH) {
        total_w = GUIDE_PUMP_START_SPEED_GROUP_WIDTH;
    }
    start_x = (GUIDE_PUMP_START_SCREEN_W - total_w) / 2;
    lv_obj_align(info->pump_start.speed_icon, LV_ALIGN_TOP_LEFT, start_x, GUIDE_PUMP_START_SPEED_ICON_Y);
    lv_obj_align(info->pump_start.speed_value, LV_ALIGN_TOP_LEFT,
        start_x + icon_w + GUIDE_PUMP_START_SPEED_ICON_VALUE_GAP, GUIDE_PUMP_START_SPEED_VALUE_Y);
}

static void guide_pump_start_level_create(lv_obj_t **label, lv_obj_t *parent, const char *prefix,
    uint8_t level, lv_coord_t x, lv_coord_t y, lv_coord_t width, lv_text_align_t align)
{
    char text[8] = {0};

    if (*label == NULL || !lv_obj_is_valid(*label)) {
        *label = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(*label);
        lv_label_set_long_mode(*label, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(*label, width);
        lv_obj_set_style_text_color(*label, lv_color_hex(GUIDE_PUMP_START_LEVEL_COLOR), 0);
        lv_obj_set_style_text_font(*label, UI_FONT_TITLE, 0);
        lv_obj_set_style_text_align(*label, align, 0);
        lv_obj_set_style_text_letter_space(*label, 3, 0);
    }
    snprintf(text, sizeof(text), "%s%u", prefix, (unsigned)level);
    lv_label_set_text(*label, text);
    lv_obj_align(*label, LV_ALIGN_TOP_LEFT, x, y);
}

static void guide_pump_start_levels_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent);
    guide_pump_start_level_create(&info->pump_start.left_level, parent, UI_STR_SIDE_L,
        GUIDE_PUMP_START_LEFT_LEVEL, GUIDE_PUMP_START_LEFT_LEVEL_X, GUIDE_PUMP_START_LEFT_LEVEL_Y,
        GUIDE_PUMP_START_LEFT_LEVEL_W, LV_TEXT_ALIGN_RIGHT);
    guide_pump_start_level_create(&info->pump_start.right_level, parent, UI_STR_SIDE_R,
        GUIDE_PUMP_START_RIGHT_LEVEL, GUIDE_PUMP_START_RIGHT_LEVEL_X, GUIDE_PUMP_START_RIGHT_LEVEL_Y,
        GUIDE_PUMP_START_RIGHT_LEVEL_W, LV_TEXT_ALIGN_LEFT);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_pump_start_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_pump_start_bg_detach(info);
    if (info->ui != NULL && info->pump_start.wave_info.data != NULL) {
        ui_img_resource_deinit(info->ui, &info->pump_start.wave_info);
    }
    ui_img_baj_release_decoded_buf(info->pump_start.time_colon);
    ui_img_baj_release_decoded_buf(info->pump_start.speed_icon);
    memset(&info->pump_start, 0, sizeof(info->pump_start));
}

void guide_pump_start_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_pump_start_bg_create(info);
    guide_pump_start_title_create(info);
    guide_pump_start_time_create(info);
    guide_pump_start_wave_create(info);
    guide_pump_start_speed_create(info);
    guide_pump_start_levels_create(info);
}

void guide_pump_start_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc)
{
    EXCEPTION_CHECK_VOID(info && proc);
    guide_pump_start_wave_frame_update(info);
    guide_pump_start_time_tick_update(info, proc);
}
