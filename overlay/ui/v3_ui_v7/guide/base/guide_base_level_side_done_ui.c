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
/** 与第九页同布局；背景/标题随第十二页模式；力度继承第十四页；右侧力度 1Hz 闪烁 */
#define GUIDE_LEVEL_SIDE_DONE_SCREEN_W                  (360)

#define GUIDE_LEVEL_SIDE_DONE_MODE_TITLE_Y              (64 + 4)
#define GUIDE_LEVEL_SIDE_DONE_MODE_TITLE_COLOR          (0xFFFFFF)

#define GUIDE_LEVEL_SIDE_DONE_TIME_DIGIT_COUNT          (2)
#define GUIDE_LEVEL_SIDE_DONE_TIME_SEG_WIDTH            (88)
#define GUIDE_LEVEL_SIDE_DONE_TIME_M_X                  (75 + 4)
#define GUIDE_LEVEL_SIDE_DONE_TIME_S_X                  (189 + 4)
#define GUIDE_LEVEL_SIDE_DONE_TIME_Y                    (122 + 4 + 31)
#define GUIDE_LEVEL_SIDE_DONE_TIME_COLON_Y              (128 + 4 + 31)
#define GUIDE_LEVEL_SIDE_DONE_TIME_COLOR                (0xFFFFFF)

#define GUIDE_LEVEL_SIDE_DONE_SPEED_GROUP_WIDTH         (47)
#define GUIDE_LEVEL_SIDE_DONE_SPEED_ICON_Y              (265 + 4)
#define GUIDE_LEVEL_SIDE_DONE_SPEED_VALUE_Y             (267 + 4)
#define GUIDE_LEVEL_SIDE_DONE_SPEED_COLOR               (0xFFFFFF)
#define GUIDE_LEVEL_SIDE_DONE_SPEED_ICON_VALUE_GAP      (6)
#define GUIDE_LEVEL_SIDE_DONE_SPEED_VALUE               (0)

#define GUIDE_LEVEL_SIDE_DONE_LEFT_LEVEL_X              (68 + 4)
#define GUIDE_LEVEL_SIDE_DONE_LEFT_LEVEL_Y              (267 + 4)
#define GUIDE_LEVEL_SIDE_DONE_LEFT_LEVEL_W              (58)
#define GUIDE_LEVEL_SIDE_DONE_RIGHT_LEVEL_X             (227 + 4)
#define GUIDE_LEVEL_SIDE_DONE_RIGHT_LEVEL_Y             (GUIDE_LEVEL_SIDE_DONE_LEFT_LEVEL_Y)
#define GUIDE_LEVEL_SIDE_DONE_RIGHT_LEVEL_W             (58)
#define GUIDE_LEVEL_SIDE_DONE_LEVEL_COLOR               (0xFFFFFF)

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static uint16_t guide_level_side_done_bg_res_id_get(uint8_t mode)
{
    switch (mode) {
        case BREAST_PUMP_LACTATION_MODE:
            return BASE_RES_ID_EXPRESSION_MODE_BG;
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            return BASE_RES_ID_PROGRAM_MODE_BG;
        case BREAST_PUMP_STIMULATION_MODE:
        default:
            return BASE_RES_ID_SIMULATION_MODE_BG;
    }
}

static void guide_level_side_done_title_get(uint8_t mode, char *buf, size_t buf_len)
{
    if (buf == NULL || buf_len == 0) {
        return;
    }
    switch (mode) {
        case BREAST_PUMP_LACTATION_MODE:
            snprintf(buf, buf_len, "Expression");
            break;
        case BREAST_PUMP_MILK_BOOST_MODE:
            snprintf(buf, buf_len, "Milk Boost");
            break;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            snprintf(buf, buf_len, "Power Pumping");
            break;
        case BREAST_PUMP_STIMULATION_MODE:
        default:
            snprintf(buf, buf_len, "Stimulation");
            break;
    }
}

static uint8_t guide_level_side_done_mode_from_proc(const ui_proc_guide_t *proc)
{
    uint8_t slot = GUIDE_MODE_SWITCH_SLOT_STIMULATION;

    if (proc != NULL && proc->ui_mode_index < GUIDE_MODE_SWITCH_SLOT_COUNT) {
        slot = (uint8_t)proc->ui_mode_index;
    }
    return guide_mode_switch_mode_from_slot(slot);
}

static void guide_level_side_done_levels_from_proc(const ui_proc_guide_t *proc,
    uint8_t *left_level, uint8_t *right_level)
{
    uint8_t left = BREAST_PUMP_LEVEL_1;
    uint8_t right = BREAST_PUMP_LEVEL_1;

    if (proc != NULL) {
        left = proc->setting.level[GUIDE_LEVEL_ADJUST_SIDE_LEFT];
        right = proc->setting.level[GUIDE_LEVEL_ADJUST_SIDE_RIGHT];
    }
    if (left_level != NULL) {
        *left_level = left;
    }
    if (right_level != NULL) {
        *right_level = right;
    }
}

static void guide_level_side_done_bg_detach(guide_base_ui_info_t *info)
{
    const void *src;

    if (info->level_side_done.bg == NULL || !lv_obj_is_valid(info->level_side_done.bg)) {
        return;
    }
    src = lv_image_get_src(info->level_side_done.bg);
    if (src == NULL) {
        return;
    }
    if (get_predecoded_bg(info->level_side_done.shown_bg_id) == src) {
        lv_image_set_src(info->level_side_done.bg, NULL);
        return;
    }
    ui_img_png_release_decoded_buf(info->level_side_done.bg);
}

static void guide_level_side_done_bg_create(guide_base_ui_info_t *info, uint8_t mode)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    const uint16_t res_id = guide_level_side_done_bg_res_id_get(mode);
    lv_draw_buf_t *predecoded;

    EXCEPTION_CHECK_VOID(parent);
    if (info->level_side_done.bg == NULL || !lv_obj_is_valid(info->level_side_done.bg)) {
        info->level_side_done.bg = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->level_side_done.bg);
        lv_obj_move_background(info->level_side_done.bg);
    }

    guide_level_side_done_bg_detach(info);
    predecoded = get_predecoded_bg(res_id);
    if (predecoded != NULL) {
        lv_image_set_src(info->level_side_done.bg, predecoded);
    } else if (info->ui != NULL) {
        ui_img_png_theme_set_src(info->ui, info->level_side_done.bg, res_id,
            RES_THEME_TYPE_BASE, UI_IMG_PNG_DECODE_RGBA8888);
    }
    lv_obj_align(info->level_side_done.bg, LV_ALIGN_CENTER, 0, 0);
    info->level_side_done.shown_bg_id = res_id;
}

static void guide_level_side_done_title_create(guide_base_ui_info_t *info, uint8_t mode)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    char title[24] = {0};

    EXCEPTION_CHECK_VOID(parent);
    guide_level_side_done_title_get(mode, title, sizeof(title));
    if (info->level_side_done.mode_title == NULL ||
        !lv_obj_is_valid(info->level_side_done.mode_title)) {
        info->level_side_done.mode_title = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(info->level_side_done.mode_title);
        lv_obj_set_style_text_color(info->level_side_done.mode_title,
            lv_color_hex(GUIDE_LEVEL_SIDE_DONE_MODE_TITLE_COLOR), 0);
        lv_obj_set_style_text_font(info->level_side_done.mode_title,
            &lv_font_aeonik_soft_26_medium, 0);
        lv_obj_set_style_text_align(info->level_side_done.mode_title, LV_TEXT_ALIGN_CENTER, 0);
    }
    lv_label_set_text(info->level_side_done.mode_title, title);
    lv_obj_align(info->level_side_done.mode_title, LV_ALIGN_TOP_MID, 0,
        GUIDE_LEVEL_SIDE_DONE_MODE_TITLE_Y);
}

static void guide_level_side_done_time_pair_create(
    lv_obj_t *labels[GUIDE_LEVEL_SIDE_DONE_TIME_DIGIT_COUNT], lv_obj_t *parent, uint8_t value,
    lv_coord_t seg_x)
{
    const lv_coord_t cell_w =
        GUIDE_LEVEL_SIDE_DONE_TIME_SEG_WIDTH / GUIDE_LEVEL_SIDE_DONE_TIME_DIGIT_COUNT;
    char digit_text[2] = {0};
    int i;

    for (i = 0; i < GUIDE_LEVEL_SIDE_DONE_TIME_DIGIT_COUNT; i++) {
        if (labels[i] == NULL || !lv_obj_is_valid(labels[i])) {
            labels[i] = lv_label_create(parent);
            EXCEPTION_CHECK_VOID(labels[i]);
            lv_label_set_long_mode(labels[i], LV_LABEL_LONG_CLIP);
            lv_obj_set_style_text_align(labels[i], LV_TEXT_ALIGN_CENTER, 0);
            lv_obj_set_style_text_color(labels[i],
                lv_color_hex(GUIDE_LEVEL_SIDE_DONE_TIME_COLOR), 0);
            lv_obj_set_style_text_font(labels[i], &lv_font_aeonik_soft_69_semibold, 0);
            lv_obj_set_style_text_letter_space(labels[i], 0, 0);
        }
        digit_text[0] = (char)('0' + (i == 0 ? value / 10 : value % 10));
        digit_text[1] = '\0';
        lv_obj_set_width(labels[i], cell_w);
        lv_label_set_text(labels[i], digit_text);
        lv_obj_align(labels[i], LV_ALIGN_TOP_LEFT, seg_x + i * cell_w,
            GUIDE_LEVEL_SIDE_DONE_TIME_Y);
    }
}

static void guide_level_side_done_time_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent && info->ui);
    guide_level_side_done_time_pair_create(info->level_side_done.time_m, parent, 0,
        GUIDE_LEVEL_SIDE_DONE_TIME_M_X);
    guide_level_side_done_time_pair_create(info->level_side_done.time_s, parent, 0,
        GUIDE_LEVEL_SIDE_DONE_TIME_S_X);

    if (info->level_side_done.time_colon == NULL ||
        !lv_obj_is_valid(info->level_side_done.time_colon)) {
        info->level_side_done.time_colon = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->level_side_done.time_colon);
        ui_img_baj_theme_set_src(info->ui, info->level_side_done.time_colon,
            BASE_RES_ID_CLOCK_COLON_ICON, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    lv_obj_align(info->level_side_done.time_colon, LV_ALIGN_TOP_MID, 0,
        GUIDE_LEVEL_SIDE_DONE_TIME_COLON_Y);
}

static void guide_level_side_done_speed_create(guide_base_ui_info_t *info)
{
    lv_obj_t *parent = guide_page_parent_get(info);
    char speed_text[8] = {0};
    lv_coord_t icon_w;
    lv_coord_t value_w;
    lv_coord_t total_w;
    lv_coord_t start_x;

    EXCEPTION_CHECK_VOID(parent && info->ui);
    if (info->level_side_done.speed_icon == NULL ||
        !lv_obj_is_valid(info->level_side_done.speed_icon)) {
        info->level_side_done.speed_icon = lv_image_create(parent);
        EXCEPTION_CHECK_VOID(info->level_side_done.speed_icon);
        ui_img_baj_theme_set_src(info->ui, info->level_side_done.speed_icon,
            BASE_RES_ID_SPEED_ICON, RES_THEME_TYPE_BASE, UI_IMG_BAJ_DECODE_RGBA8888);
    }
    if (info->level_side_done.speed_value == NULL ||
        !lv_obj_is_valid(info->level_side_done.speed_value)) {
        info->level_side_done.speed_value = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(info->level_side_done.speed_value);
        lv_obj_set_style_text_color(info->level_side_done.speed_value,
            lv_color_hex(GUIDE_LEVEL_SIDE_DONE_SPEED_COLOR), 0);
        lv_obj_set_style_text_font(info->level_side_done.speed_value,
            &lv_font_aeonik_soft_26_medium, 0);
    }

    snprintf(speed_text, sizeof(speed_text), "%u",
        (unsigned)(GUIDE_LEVEL_SIDE_DONE_SPEED_VALUE + 1));
    lv_label_set_text(info->level_side_done.speed_value, speed_text);
    lv_obj_update_layout(info->level_side_done.speed_icon);
    lv_obj_update_layout(info->level_side_done.speed_value);
    icon_w = lv_obj_get_width(info->level_side_done.speed_icon);
    value_w = lv_obj_get_width(info->level_side_done.speed_value);
    total_w = icon_w + GUIDE_LEVEL_SIDE_DONE_SPEED_ICON_VALUE_GAP + value_w;
    if (total_w > GUIDE_LEVEL_SIDE_DONE_SPEED_GROUP_WIDTH) {
        total_w = GUIDE_LEVEL_SIDE_DONE_SPEED_GROUP_WIDTH;
    }
    start_x = (GUIDE_LEVEL_SIDE_DONE_SCREEN_W - total_w) / 2;
    lv_obj_align(info->level_side_done.speed_icon, LV_ALIGN_TOP_LEFT, start_x,
        GUIDE_LEVEL_SIDE_DONE_SPEED_ICON_Y);
    lv_obj_align(info->level_side_done.speed_value, LV_ALIGN_TOP_LEFT,
        start_x + icon_w + GUIDE_LEVEL_SIDE_DONE_SPEED_ICON_VALUE_GAP,
        GUIDE_LEVEL_SIDE_DONE_SPEED_VALUE_Y);
}

static void guide_level_side_done_level_create(lv_obj_t **label, lv_obj_t *parent,
    const char *prefix, uint8_t level, lv_coord_t x, lv_coord_t y, lv_coord_t width,
    lv_text_align_t align)
{
    char text[8] = {0};

    if (*label == NULL || !lv_obj_is_valid(*label)) {
        *label = lv_label_create(parent);
        EXCEPTION_CHECK_VOID(*label);
        lv_label_set_long_mode(*label, LV_LABEL_LONG_CLIP);
        lv_obj_set_width(*label, width);
        lv_obj_set_style_text_color(*label, lv_color_hex(GUIDE_LEVEL_SIDE_DONE_LEVEL_COLOR), 0);
        lv_obj_set_style_text_font(*label, &lv_font_aeonik_soft_26_medium, 0);
        lv_obj_set_style_text_align(*label, align, 0);
        lv_obj_set_style_text_letter_space(*label, 3, 0);
    }
    snprintf(text, sizeof(text), "%s%u", prefix, (unsigned)level);
    lv_label_set_text(*label, text);
    lv_obj_align(*label, LV_ALIGN_TOP_LEFT, x, y);
}

static void guide_level_side_done_levels_create(guide_base_ui_info_t *info,
    uint8_t left_level, uint8_t right_level)
{
    lv_obj_t *parent = guide_page_parent_get(info);

    EXCEPTION_CHECK_VOID(parent);
    guide_level_side_done_level_create(&info->level_side_done.left_level, parent, "L",
        left_level, GUIDE_LEVEL_SIDE_DONE_LEFT_LEVEL_X, GUIDE_LEVEL_SIDE_DONE_LEFT_LEVEL_Y,
        GUIDE_LEVEL_SIDE_DONE_LEFT_LEVEL_W, LV_TEXT_ALIGN_RIGHT);
    guide_level_side_done_level_create(&info->level_side_done.right_level, parent, "R",
        right_level, GUIDE_LEVEL_SIDE_DONE_RIGHT_LEVEL_X, GUIDE_LEVEL_SIDE_DONE_RIGHT_LEVEL_Y,
        GUIDE_LEVEL_SIDE_DONE_RIGHT_LEVEL_W, LV_TEXT_ALIGN_LEFT);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_level_side_done_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    guide_level_side_done_bg_detach(info);
    ui_img_baj_release_decoded_buf(info->level_side_done.time_colon);
    ui_img_baj_release_decoded_buf(info->level_side_done.speed_icon);
    memset(&info->level_side_done, 0, sizeof(info->level_side_done));
}

void guide_level_side_done_ui_build(guide_base_ui_info_t *info)
{
    uint8_t mode = BREAST_PUMP_STIMULATION_MODE;
    uint8_t left_level = BREAST_PUMP_LEVEL_1;
    uint8_t right_level = BREAST_PUMP_LEVEL_1;
    ui_proc_guide_t *proc = NULL;

    EXCEPTION_CHECK_VOID(info);
    if (info->ui != NULL) {
        proc = (ui_proc_guide_t *)info->ui->priv.play_info.user_data;
        mode = guide_level_side_done_mode_from_proc(proc);
        guide_level_side_done_levels_from_proc(proc, &left_level, &right_level);
    }
    guide_level_side_done_bg_create(info, mode);
    guide_level_side_done_title_create(info, mode);
    guide_level_side_done_time_create(info);
    guide_level_side_done_speed_create(info);
    guide_level_side_done_levels_create(info, left_level, right_level);
}

void guide_level_side_done_ui_update(guide_base_ui_info_t *info, ui_proc_guide_t *proc)
{
    lv_opa_t opa;

    EXCEPTION_CHECK_VOID(info && proc);
    if (info->level_side_done.right_level == NULL ||
        !lv_obj_is_valid(info->level_side_done.right_level)) {
        return;
    }
    opa = (proc->state_blink_visible != 0) ? LV_OPA_COVER : LV_OPA_TRANSP;
    lv_obj_set_style_text_opa(info->level_side_done.right_level, opa, 0);
}
