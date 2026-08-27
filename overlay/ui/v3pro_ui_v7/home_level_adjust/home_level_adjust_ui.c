/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep
#include "home_level_adjust_ui.h" // IWYU pragma: keep
#include "ui_shine_component.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep
#include "ui_progress_arc_component.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_HOME_LEVEL_ADJUST_COMP_BG        (0)

/** 刺激模式力度颜色 */
#define HOME_STIMULATION_LEVEL_COLOR                (0xE2637E)
/** 吸乳模式力度颜色 */
#define HOME_LACTATION_LEVEL_COLOR                  (0x64A4F2)
/** 韵律模式力度颜色 */
#define HOME_RHYTHM_LEVEL_COLOR                     (0x9C78E8)

#define HOME_LEVEL_VALUE_Y                          (127)
#define HOME_LEVEL_VALUE_COLOR                      (0xFFFFFF)

#define HOME_LEVEL_DIR_COLOR                        (0x999999)

#define HOME_LEVEL_TIP_H                            (63)
/** 底部提示图 Y：贴底留边（与 home 模式选择页一致） */
#define HOME_LEVEL_TIP_Y                            (PANLE_DISPLAY_H - HOME_LEVEL_TIP_H - 8)

#define HOME_LEVEL_TIP_BASE_OPA                     (LV_OPA_70)

/** 相邻档位圆弧过渡时长 */
#define HOME_LEVEL_ARC_ANIM_MS                      (220)

typedef struct {
    lv_obj_t *root;
#if IS_SUPPORT_HOME_LEVEL_ADJUST_COMP_BG
    lv_obj_t *background;
#endif
    ui_progress_arc_info_t arc;
    lv_obj_t *value_label;
    lv_obj_t *dir_label;
    lv_obj_t *tip_img;
    ui_shine_info_t tip_shine;
    lv_draw_buf_t *tip_buf;
    uint8_t tip_predecoded;
    uint8_t shown_level;
    uint8_t shown_side;
    uint8_t shown_over_limit;
    uint8_t shown_mode_switch;
    uint8_t shown_mode_type;
    uint8_t anim_to_level;
    uint8_t pump_mode; /// 工作模式（强调色）
    uint8_t mode_type; /// 当前子模式（力度上限）
    uint8_t level_max; /// 当前子模式力度上限
    uint8_t force_update;
    uint32_t accent_color; /// 圆弧/过大提示强调色
    uint32_t shown_dir_color;
    uint16_t anim_from_sweep;
    uint16_t anim_to_sweep;
    uint32_t anim_start_time;
} home_level_adjust_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_HOME_LEVEL_ADJUST_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t home_level_adjust_bg_data[] = {
    #include "comparison_png2txt/left_level_adjust11_360x360.txt"
};
#endif

static home_level_adjust_ui_info_t *level_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void home_level_adjust_root_init(void)
{
    if (level_info->root == NULL || !lv_obj_is_valid(level_info->root)) {
        level_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(level_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(level_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(level_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(level_info->root, 0, 0);
    lv_obj_set_style_pad_all(level_info->root, 0, 0);
    lv_obj_align(level_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(level_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(level_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

#if IS_SUPPORT_HOME_LEVEL_ADJUST_COMP_BG
static void home_level_adjust_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;

    (void)thiz;
    if (level_info->background == NULL || !lv_obj_is_valid(level_info->background)) {
        level_info->background = lv_image_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->background);
        lv_obj_move_background(level_info->background);
    }
    if (status == 0) {
        lv_obj_add_flag(level_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_fs_path_ex_t bg_img_path;
        lv_obj_clear_flag(level_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, home_level_adjust_bg_data,
            sizeof(home_level_adjust_bg_data), "png");
        lv_image_set_src(level_info->background, (const void *)&bg_img_path);
        lv_obj_align(level_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void home_level_adjust_bg_update(ui_manager_context_t *thiz)
{
    (void)thiz;
}
#endif

static uint16_t home_level_arc_visual_sweep_get(void)
{
    uint32_t elapsed;
    uint16_t from = level_info->anim_from_sweep;
    uint16_t to = level_info->anim_to_sweep;

    if (from == to) {
        return to;
    }

    elapsed = get_timestamp_ms(level_info->anim_start_time);
    if (elapsed >= HOME_LEVEL_ARC_ANIM_MS) {
        level_info->anim_from_sweep = to;
        return to;
    }

    if (to >= from) {
        return (uint16_t)(from + ((uint32_t)(to - from) * elapsed) / HOME_LEVEL_ARC_ANIM_MS);
    }
    return (uint16_t)(from - ((uint32_t)(from - to) * elapsed) / HOME_LEVEL_ARC_ANIM_MS);
}

static void home_level_arc_anim_sync(uint8_t target_level, uint8_t level_max, bool snap)
{
    uint16_t target_sweep = ui_progress_arc_sweep_from_progress(target_level, level_max);

    /* 仅首次进入/切换左右侧硬切；模式切换与调档一律从当前视觉位置插值 */
    if (snap || level_info->anim_to_level == 0xFF) {
        level_info->anim_from_sweep = target_sweep;
        level_info->anim_to_sweep = target_sweep;
        level_info->anim_to_level = target_level;
        level_info->anim_start_time = get_timestamp();
        return;
    }

    if (level_info->anim_to_level == target_level &&
        level_info->anim_to_sweep == target_sweep) {
        return;
    }

    level_info->anim_from_sweep = home_level_arc_visual_sweep_get();
    level_info->anim_to_sweep = target_sweep;
    level_info->anim_to_level = target_level;
    level_info->anim_start_time = get_timestamp();
}

static void home_level_arc_update(ui_proc_home_t *proc, uint8_t level_max, uint32_t accent_color)
{
    ui_progress_arc_style_t style;
    uint16_t done_sweep;
    bool side_changed;

    side_changed = (level_info->shown_side != 0xFF &&
        level_info->shown_side != proc->level_adjust_side);
    home_level_arc_anim_sync(proc->level_arc_level, level_max, side_changed);
    done_sweep = home_level_arc_visual_sweep_get();

    ui_progress_arc_style_init(&style, accent_color);
    style.show_countdown = false;
    ui_progress_arc_update(&level_info->arc, level_info->root, &style,
        done_sweep, level_info->force_update);
}
static void home_level_value_update(ui_proc_home_t *proc)
{
    char num_buf[8] = {0};

    if (level_info->value_label == NULL || !lv_obj_is_valid(level_info->value_label)) {
        level_info->value_label = lv_label_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->value_label);
        level_info->shown_level = 0xFF;
    }
    if (level_info->force_update == false && level_info->shown_level == proc->level_value) {
        return;
    }

    snprintf(num_buf, sizeof(num_buf), "%u", proc->level_value);
    lv_label_set_text(level_info->value_label, num_buf);
    lv_obj_set_style_text_color(level_info->value_label,
        lv_color_hex(HOME_LEVEL_VALUE_COLOR), 0);
    lv_obj_set_style_text_font(level_info->value_label, UI_FONT_NUM_LG, 0);
    lv_obj_set_style_text_align(level_info->value_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(level_info->value_label, LV_ALIGN_TOP_MID, 0, HOME_LEVEL_VALUE_Y);
    level_info->shown_level = proc->level_value;
}

static void home_level_dir_update(ui_proc_home_t *proc, uint32_t accent_color)
{
    char switch_buf[32] = {0};
    const char *text;
    const lv_font_t *font;
    uint32_t color;
    uint8_t mode_switch = proc->sub_mode_switch;
    uint32_t expect_color;

    if (level_info->dir_label == NULL || !lv_obj_is_valid(level_info->dir_label)) {
        level_info->dir_label = lv_label_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->dir_label);
        level_info->shown_side = 0xFF;
        level_info->shown_over_limit = 0xFF;
        level_info->shown_mode_switch = 0xFF;
        level_info->shown_mode_type = 0xFF;
    }

    if (proc->level_over_limit || mode_switch) {
        expect_color = accent_color;
    } else {
        expect_color = HOME_LEVEL_DIR_COLOR;
    }
    if (level_info->force_update == false &&
        level_info->shown_side == proc->level_adjust_side &&
        level_info->shown_over_limit == proc->level_over_limit &&
        level_info->shown_mode_switch == mode_switch &&
        level_info->shown_mode_type == proc->sub_mode_type &&
        level_info->shown_dir_color == expect_color) {
        return;
    }

    if (proc->level_over_limit) {
        text = HOME_LEVEL_OVER_LIMIT_TEXT;
        color = accent_color;
        font = UI_FONT_BODY_SM;
    } else if (mode_switch) {
        snprintf(switch_buf, sizeof(switch_buf), HOME_LEVEL_MODE_SWITCH_TEXT_FMT,
            home_level_adjust_mode_type_name_get(proc->sub_mode_type));
        text = switch_buf;
        color = accent_color;
        font = UI_FONT_TITLE;
    } else if (proc->level_adjust_side == HOME_LEVEL_ADJUST_SIDE_RIGHT) {
        text = HOME_LEVEL_DIR_RIGHT_TEXT;
        color = HOME_LEVEL_DIR_COLOR;
        font = UI_FONT_TITLE;
    } else {
        text = HOME_LEVEL_DIR_LEFT_TEXT;
        color = HOME_LEVEL_DIR_COLOR;
        font = UI_FONT_TITLE;
    }

    lv_label_set_text(level_info->dir_label, text);
    lv_obj_set_style_text_color(level_info->dir_label, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(level_info->dir_label, font, 0);
    lv_obj_set_style_text_align(level_info->dir_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(level_info->dir_label, LV_ALIGN_TOP_MID, 0, HOME_LEVEL_DIR_Y);
    lv_obj_set_style_text_line_space(level_info->dir_label, HOME_LEVEL_DIR_LINE_SPACE, 0);
    level_info->shown_side = proc->level_adjust_side;
    level_info->shown_over_limit = proc->level_over_limit;
    level_info->shown_mode_switch = mode_switch;
    level_info->shown_mode_type = proc->sub_mode_type;
    level_info->shown_dir_color = color;
}

static lv_draw_buf_t *home_level_tip_predecode(void)
{
    const ui_img_data_t *tip;

    if (level_info->tip_buf != NULL) {
        return level_info->tip_buf;
    }
    tip = ui_font_img_level_adjust_tip_get();
    if (tip == NULL || tip->data == NULL || tip->size == 0U) {
        return NULL;
    }
    level_info->tip_buf = ui_img_png_decode_draw_buf(tip->data, tip->size,
        UI_IMG_PNG_DECODE_RGBA8888);
    return level_info->tip_buf;
}

static void home_level_tip_rebuild(void)
{
    lv_draw_buf_t *predecoded;
    ui_shine_style_t style;

    if (level_info->tip_img != NULL && lv_obj_is_valid(level_info->tip_img) &&
        level_info->tip_shine.mask != NULL && lv_obj_is_valid(level_info->tip_shine.mask) &&
        level_info->tip_shine.shine != NULL && lv_obj_is_valid(level_info->tip_shine.shine)) {
        return; /* anim handles sync */
    }

    predecoded = home_level_tip_predecode();

    if (level_info->tip_img == NULL || !lv_obj_is_valid(level_info->tip_img)) {
        level_info->tip_img = lv_image_create(level_info->root);
        EXCEPTION_CHECK_VOID(level_info->tip_img);
        lv_obj_clear_flag(level_info->tip_img, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(level_info->tip_img, predecoded);
            level_info->tip_predecoded = 1;
        }
        lv_obj_set_style_image_opa(level_info->tip_img, HOME_LEVEL_TIP_BASE_OPA, 0);
        lv_obj_align(level_info->tip_img, LV_ALIGN_TOP_MID, 0, HOME_LEVEL_TIP_Y);
    }

    ui_shine_style_init(&style);
    style.mask_y = HOME_LEVEL_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;

    ui_shine_prepare_from_tip(&level_info->tip_shine, &style, predecoded);
    ui_shine_update(&level_info->tip_shine, level_info->root, &style, level_info->tip_img, 1);
    lv_obj_move_foreground(level_info->tip_img);
    ui_shine_show(&level_info->tip_shine);
}

static void home_level_tip_update(void)
{
    ui_shine_style_t style;

    if (level_info->force_update) {
        home_level_tip_rebuild();
    }
    ui_shine_style_init(&style);
    style.mask_y = HOME_LEVEL_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_anim_update(&level_info->tip_shine, &style, level_info->tip_img, 0);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
uint8_t home_level_adjust_max_get(uint8_t mode_type)
{
    switch (mode_type) {
        case PUMP_MODE_STIMULATION:
            return BREAST_PUMP_LEVEL_5;
        case PUMP_MODE_LACTATION:
        case PUMP_MODE_MIX:
        case PUMP_MODE_IDLE:
        default:
            return BREAST_PUMP_LEVEL_15;
    }
}

uint8_t home_level_adjust_mode_type_resolve(uint8_t pump_mode, uint8_t runtime_mode_type)
{
    if (PUMP_IS_COMMON_MODE(pump_mode)) {
        /* BREAST_PUMP_STIMULATION/LACTATION/MIX 与 PUMP_MODE_* 对齐 */
        return pump_mode;
    }
    return runtime_mode_type;
}

const char *home_level_adjust_mode_type_name_get(uint8_t mode_type)
{
    switch (mode_type) {
        case PUMP_MODE_STIMULATION:
            return UI_STR_MODE_STIMULATION;
        case PUMP_MODE_LACTATION:
            return UI_STR_MODE_EXPRESSION;
        case PUMP_MODE_MIX:
            return UI_STR_MODE_MIXED;
        case PUMP_MODE_IDLE:
            return UI_STR_MODE_RELAX;
        default:
            return UI_STR_MODE_EXPRESSION;
    }
}

uint32_t home_level_adjust_accent_color_get(uint8_t pump_mode)
{
    switch (pump_mode) {
        case BREAST_PUMP_MILK_BOOST_MODE:
        case BREAST_PUMP_CUSTOM_MODE:
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            return HOME_RHYTHM_LEVEL_COLOR;
        case BREAST_PUMP_STIMULATION_MODE:
            return HOME_STIMULATION_LEVEL_COLOR;
        case BREAST_PUMP_LACTATION_MODE:
            return HOME_LACTATION_LEVEL_COLOR;
        default:
            return HOME_STIMULATION_LEVEL_COLOR;
    }
}

void ui_display_home_level_adjust_prepare(ui_manager_context_t *thiz)
{
    ui_proc_home_t *proc;
    int pump_mode = 0;

    EXCEPTION_CHECK_VOID(thiz);
    if (level_info == NULL) {
        level_info = (home_level_adjust_ui_info_t *)u_malloz(sizeof(home_level_adjust_ui_info_t));
        EXCEPTION_CHECK_VOID(level_info);
    }
    proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc && proc->pump);

    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    level_info->pump_mode = (uint8_t)pump_mode;
    level_info->mode_type = proc->sub_mode_type;
    level_info->level_max = home_level_adjust_max_get(level_info->mode_type);
    level_info->accent_color = home_level_adjust_accent_color_get(level_info->pump_mode);
    LOG_UI("level adjust prepare mode=%d type=%d max=%d color=0x%06X",
        level_info->pump_mode, level_info->mode_type, level_info->level_max,
        level_info->accent_color);

    level_info->force_update = true;
    level_info->shown_level = 0xFF;
    level_info->shown_side = 0xFF;
    level_info->shown_over_limit = 0xFF;
    level_info->shown_mode_switch = 0xFF;
    level_info->shown_mode_type = 0xFF;
    level_info->anim_to_level = 0xFF;
    level_info->shown_dir_color = 0;
    ui_progress_arc_reset(&level_info->arc);
    home_level_adjust_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_home_level_adjust_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && level_info && level_info->root);
    ui_shine_remove(&level_info->tip_shine);
    if (level_info->tip_img != NULL && lv_obj_is_valid(level_info->tip_img)) {
        lv_image_set_src(level_info->tip_img, NULL);
    }
    if (level_info->tip_buf != NULL) {
        lv_draw_buf_destroy(level_info->tip_buf);
        level_info->tip_buf = NULL;
    }
    level_info->tip_predecoded = 0;
    LVGL_OBJ_DELETE(level_info->root);
    u_free(level_info);
    level_info = NULL;
}

int ui_display_home_level_adjust_process(ui_manager_context_t *thiz)
{
    ui_proc_home_t *proc;
    uint8_t level_max;

    EXCEPTION_CHECK_INT(thiz && level_info && level_info->root);
    proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->pump);

    level_max = home_level_adjust_max_get(proc->sub_mode_type);
    if (level_info->mode_type != proc->sub_mode_type ||
        level_info->level_max != level_max) {
        /* 模式切换：数字可跳变，进度条从当前扫角渐进到新目标，不硬切 */
        level_info->mode_type = proc->sub_mode_type;
        level_info->level_max = level_max;
    }

    home_level_adjust_bg_update(thiz);
    home_level_arc_update(proc, level_info->level_max, level_info->accent_color);
    home_level_value_update(proc);
    home_level_dir_update(proc, level_info->accent_color);
    home_level_tip_update();
    level_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> home level adjust fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
