/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "home_speed_adjust_ui.h" // IWYU pragma: keep
#include "home_level_adjust_ui.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep
#include "ui_shine_component.h" // IWYU pragma: keep
#include "ui_snapshot.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_HOME_SPEED_ADJUST_COMP_BG        (0) /// 是否启用对比背景图

/** 速度档位圆弧：五段，直径 352，线宽 11，段间距 8px */
#define HOME_SPEED_ARC_SIZE                         (352)
#define HOME_SPEED_ARC_WIDTH                        (11)
#define HOME_SPEED_ARC_SEG_COUNT                    (5)
/** 段间距 8px → 角间距约 8*180/(π*r)，r=(352-11)/2=170.5 ≈ 2.7°，取 3° */
#define HOME_SPEED_ARC_GAP_DEG                      (8)
#define HOME_SPEED_ARC_SEG_DEG                      (26)
#if IS_SUPPORT_HOME_SPEED_ADJUST_COMP_BG
#define HOME_SPEED_ARC_REMAIN_COLOR                 (0xFFFFFF)
#else
#define HOME_SPEED_ARC_REMAIN_COLOR                 (0x252525)
#endif
/** 除去段间间隙后的可绘制总角度 */
#define HOME_SPEED_ARC_TRACK_DEG \
    (HOME_SPEED_ARC_SEG_COUNT * HOME_SPEED_ARC_SEG_DEG + \
     (HOME_SPEED_ARC_SEG_COUNT - 1) * HOME_SPEED_ARC_GAP_DEG)
#define HOME_SPEED_ARC_BOTTOM_GAP_DEG \
    (360 - HOME_SPEED_ARC_TRACK_DEG)
/** 开口居中于底部：弧起点落在 gap 右侧 */
#define HOME_SPEED_ARC_ROTATION \
    (90 + (HOME_SPEED_ARC_BOTTOM_GAP_DEG / 2))

#define HOME_SPEED_VALUE_Y                          (127)
#define HOME_SPEED_VALUE_COLOR                      (0xFFFFFF)

#define HOME_SPEED_LABEL_COLOR                      (0x999999)

/** 底部提示图：与模式选择页一致 */
#define HOME_SPEED_TIP_H                            (63)
#define HOME_SPEED_TIP_Y                            (PANLE_DISPLAY_H - HOME_SPEED_TIP_H - 8)

#define HOME_SPEED_TIP_BASE_OPA                     (LV_OPA_70)

typedef struct {
    lv_obj_t *root;
#if IS_SUPPORT_HOME_SPEED_ADJUST_COMP_BG
    lv_obj_t *background;
#endif
    lv_obj_t *arcs[HOME_SPEED_ARC_SEG_COUNT];
    lv_obj_t *value_label;
    lv_obj_t *speed_label;
    lv_obj_t *tip_img;
    ui_shine_info_t tip_shine;
    lv_draw_buf_t *tip_buf;
    uint8_t tip_predecoded;
    uint8_t shown_speed;
    uint8_t shown_selected;
    uint8_t shown_mode_switch;
    uint8_t shown_mode_type;
    uint8_t pump_mode;
    uint8_t force_update;
    uint32_t accent_color;
    uint32_t shown_accent_color;
    uint32_t shown_label_color;
} home_speed_adjust_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_HOME_SPEED_ADJUST_COMP_BG
static PSRAM_NC_DATA_LOC uint8_t home_speed_adjust_bg_data[] = {
    #include "comparison_png2txt/speed_adjust12_352x352.txt"
};
#endif

static home_speed_adjust_ui_info_t *speed_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void home_speed_adjust_root_init(void)
{
    if (speed_info->root == NULL || !lv_obj_is_valid(speed_info->root)) {
        speed_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(speed_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(speed_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(speed_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(speed_info->root, 0, 0);
    lv_obj_set_style_pad_all(speed_info->root, 0, 0);
    lv_obj_align(speed_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(speed_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(speed_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

#if IS_SUPPORT_HOME_SPEED_ADJUST_COMP_BG
static void home_speed_adjust_bg_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;

    (void)thiz;
    if (speed_info->background == NULL || !lv_obj_is_valid(speed_info->background)) {
        speed_info->background = lv_image_create(speed_info->root);
        EXCEPTION_CHECK_VOID(speed_info->background);
        lv_obj_move_background(speed_info->background);
    }
    if (status == 0) {
        lv_obj_add_flag(speed_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_fs_path_ex_t bg_img_path;
        lv_obj_clear_flag(speed_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER,
            home_speed_adjust_bg_data, sizeof(home_speed_adjust_bg_data), "png");
        lv_image_set_src(speed_info->background, (const void *)&bg_img_path);
        lv_obj_align(speed_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#else
static void home_speed_adjust_bg_update(ui_manager_context_t *thiz)
{
    (void)thiz;
}
#endif

static void home_speed_arc_style_init(lv_obj_t *arc, uint32_t color)
{
    lv_obj_set_size(arc, HOME_SPEED_ARC_SIZE, HOME_SPEED_ARC_SIZE);
    lv_obj_center(arc);
    lv_arc_set_rotation(arc, HOME_SPEED_ARC_ROTATION);
    lv_arc_set_bg_angles(arc, 0, 0);
    lv_obj_set_style_arc_width(arc, HOME_SPEED_ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_color(arc, lv_color_hex(color), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(arc, HOME_SPEED_ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(arc, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_set_style_arc_rounded(arc, true, LV_PART_INDICATOR);
    lv_obj_remove_style(arc, NULL, LV_PART_KNOB);
    lv_obj_clear_flag(arc, LV_OBJ_FLAG_CLICKABLE);
}

/** 展示档位数：内部 speed 0 起，界面显示 1 起；点亮前 N 段 */
static uint8_t home_speed_selected_count_get(uint8_t speed)
{
    uint8_t selected = speed + 1;

    if (selected > HOME_SPEED_ARC_SEG_COUNT) {
        selected = HOME_SPEED_ARC_SEG_COUNT;
    }
    if (selected > BREAST_PUMP_SPEED_MAX) {
        selected = BREAST_PUMP_SPEED_MAX;
    }
    return selected;
}

static void home_speed_arc_update(ui_proc_home_t *proc, uint32_t accent_color)
{
    uint8_t i;
    uint8_t selected;
    bool need_update;

    selected = home_speed_selected_count_get(proc->setting.speed);
    need_update = (speed_info->force_update ||
        speed_info->shown_accent_color != accent_color ||
        speed_info->shown_selected != selected);

    for (i = 0; i < HOME_SPEED_ARC_SEG_COUNT; i++) {
        uint16_t start = (uint16_t)(i * (HOME_SPEED_ARC_SEG_DEG + HOME_SPEED_ARC_GAP_DEG));
        uint16_t end = (uint16_t)(start + HOME_SPEED_ARC_SEG_DEG);
        uint32_t color = ((i + 1) == selected) ? accent_color : HOME_SPEED_ARC_REMAIN_COLOR;
        bool created = false;

        if (speed_info->arcs[i] == NULL || !lv_obj_is_valid(speed_info->arcs[i])) {
            speed_info->arcs[i] = lv_arc_create(speed_info->root);
            EXCEPTION_CHECK_VOID(speed_info->arcs[i]);
            home_speed_arc_style_init(speed_info->arcs[i], color);
            created = true;
        }

        if (created || need_update) {
            lv_obj_set_style_arc_color(speed_info->arcs[i], lv_color_hex(color),
                LV_PART_INDICATOR);
            lv_arc_set_angles(speed_info->arcs[i], start, end);
            lv_obj_invalidate(speed_info->arcs[i]);
        }
    }

    speed_info->shown_selected = selected;
    speed_info->shown_accent_color = accent_color;
}

static void home_speed_value_update(ui_proc_home_t *proc)
{
    char num_buf[8] = {0};
    uint8_t display_speed = proc->setting.speed + 1;

    if (speed_info->value_label == NULL || !lv_obj_is_valid(speed_info->value_label)) {
        speed_info->value_label = lv_label_create(speed_info->root);
        EXCEPTION_CHECK_VOID(speed_info->value_label);
        speed_info->shown_speed = 0xFF;
    }
    if (speed_info->force_update == false && speed_info->shown_speed == proc->setting.speed) {
        return;
    }

    snprintf(num_buf, sizeof(num_buf), "%u", display_speed);
    lv_label_set_text(speed_info->value_label, num_buf);
    lv_obj_set_style_text_color(speed_info->value_label,
        lv_color_hex(HOME_SPEED_VALUE_COLOR), 0);
    lv_obj_set_style_text_font(speed_info->value_label, UI_FONT_NUM_LG, 0);
    lv_obj_set_style_text_align(speed_info->value_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(speed_info->value_label, LV_ALIGN_TOP_MID, 0, HOME_SPEED_VALUE_Y);
    speed_info->shown_speed = proc->setting.speed;
}

static void home_speed_label_update(ui_proc_home_t *proc, uint32_t accent_color)
{
    char switch_buf[32] = {0};
    const char *text;
    uint32_t color;
    uint8_t mode_switch = proc->sub_mode_switch;

    if (speed_info->speed_label == NULL || !lv_obj_is_valid(speed_info->speed_label)) {
        speed_info->speed_label = lv_label_create(speed_info->root);
        EXCEPTION_CHECK_VOID(speed_info->speed_label);
        speed_info->shown_mode_switch = 0xFF;
        speed_info->shown_mode_type = 0xFF;
    }

    if (mode_switch) {
        snprintf(switch_buf, sizeof(switch_buf), HOME_SPEED_MODE_SWITCH_TEXT_FMT,
            home_level_adjust_mode_type_name_get(proc->sub_mode_type));
        text = switch_buf;
        color = accent_color;
    } else {
        text = HOME_SPEED_LABEL_TEXT;
        color = HOME_SPEED_LABEL_COLOR;
    }

    if (speed_info->force_update == false &&
        speed_info->shown_mode_switch == mode_switch &&
        speed_info->shown_mode_type == proc->sub_mode_type &&
        speed_info->shown_label_color == color) {
        return;
    }

    lv_label_set_text(speed_info->speed_label, text);
    lv_obj_set_style_text_color(speed_info->speed_label, lv_color_hex(color), 0);
    lv_obj_set_style_text_font(speed_info->speed_label, UI_FONT_TITLE, 0);
    lv_obj_set_style_text_align(speed_info->speed_label, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align(speed_info->speed_label, LV_ALIGN_TOP_MID, 0, HOME_SPEED_LABEL_Y);
    speed_info->shown_mode_switch = mode_switch;
    speed_info->shown_mode_type = proc->sub_mode_type;
    speed_info->shown_label_color = color;
}

static lv_draw_buf_t *home_speed_tip_predecode(void)
{
    const ui_img_data_t *tip;

    if (speed_info->tip_buf != NULL) {
        return speed_info->tip_buf;
    }
    tip = ui_font_img_mode_select_tip_get();
    if (tip == NULL || tip->data == NULL || tip->size == 0U) {
        return NULL;
    }
    speed_info->tip_buf = ui_img_png_decode_draw_buf(tip->data, tip->size,
        UI_IMG_PNG_DECODE_RGBA8888);
    return speed_info->tip_buf;
}

static void home_speed_tip_rebuild(void)
{
    lv_draw_buf_t *predecoded;
    ui_shine_style_t style;

    if (speed_info->tip_img != NULL && lv_obj_is_valid(speed_info->tip_img) &&
        speed_info->tip_shine.mask != NULL && lv_obj_is_valid(speed_info->tip_shine.mask) &&
        speed_info->tip_shine.shine != NULL && lv_obj_is_valid(speed_info->tip_shine.shine)) {
        return; /* anim handles sync */
    }

    predecoded = home_speed_tip_predecode();

    if (speed_info->tip_img == NULL || !lv_obj_is_valid(speed_info->tip_img)) {
        speed_info->tip_img = lv_image_create(speed_info->root);
        EXCEPTION_CHECK_VOID(speed_info->tip_img);
        lv_obj_clear_flag(speed_info->tip_img, LV_OBJ_FLAG_CLICKABLE);
        if (predecoded != NULL) {
            lv_image_set_src(speed_info->tip_img, predecoded);
            speed_info->tip_predecoded = 1;
        }
        lv_obj_set_style_image_opa(speed_info->tip_img, HOME_SPEED_TIP_BASE_OPA, 0);
        lv_obj_align(speed_info->tip_img, LV_ALIGN_TOP_MID, 0, HOME_SPEED_TIP_Y);
    }

    ui_shine_style_init(&style);
    style.mask_y = HOME_SPEED_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;

    ui_shine_prepare_from_tip(&speed_info->tip_shine, &style, predecoded);
    ui_shine_update(&speed_info->tip_shine, speed_info->root, &style, speed_info->tip_img, 1);
    lv_obj_move_foreground(speed_info->tip_img);
    ui_shine_show(&speed_info->tip_shine);
}

static void home_speed_tip_update(void)
{
    ui_shine_style_t style;

    if (speed_info->force_update) {
        home_speed_tip_rebuild();
    }
    ui_shine_style_init(&style);
    style.mask_y = HOME_SPEED_TIP_Y;
    style.screen_w = PANLE_DISPLAY_W;
    ui_shine_anim_update(&speed_info->tip_shine, &style, speed_info->tip_img, 0);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_home_speed_adjust_prepare(ui_manager_context_t *thiz)
{
    ui_proc_home_t *proc;
    int pump_mode = 0;

    EXCEPTION_CHECK_VOID(thiz);
    if (speed_info == NULL) {
        speed_info = (home_speed_adjust_ui_info_t *)u_malloz(sizeof(home_speed_adjust_ui_info_t));
        EXCEPTION_CHECK_VOID(speed_info);
    }
    proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc && proc->pump);

    READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    speed_info->pump_mode = (uint8_t)pump_mode;
    speed_info->accent_color = home_level_adjust_accent_color_get(speed_info->pump_mode);
    LOG_UI("speed adjust prepare mode=%d color=0x%06X speed=%d",
        speed_info->pump_mode, speed_info->accent_color, proc->setting.speed);

    speed_info->force_update = true;
    speed_info->shown_speed = 0xFF;
    speed_info->shown_selected = 0xFF;
    speed_info->shown_mode_switch = 0xFF;
    speed_info->shown_mode_type = 0xFF;
    speed_info->shown_accent_color = 0;
    speed_info->shown_label_color = 0;
    home_speed_adjust_root_init();
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_home_speed_adjust_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && speed_info && speed_info->root);
    ui_shine_remove(&speed_info->tip_shine);
    if (speed_info->tip_img != NULL && lv_obj_is_valid(speed_info->tip_img)) {
        lv_image_set_src(speed_info->tip_img, NULL);
    }
    if (speed_info->tip_buf != NULL) {
        lv_draw_buf_destroy(speed_info->tip_buf);
        speed_info->tip_buf = NULL;
    }
    speed_info->tip_predecoded = 0;
    LVGL_OBJ_DELETE(speed_info->root);
    u_free(speed_info);
    speed_info = NULL;
}

int ui_display_home_speed_adjust_process(ui_manager_context_t *thiz)
{
    ui_proc_home_t *proc;

    EXCEPTION_CHECK_INT(thiz && speed_info && speed_info->root);
    proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc && proc->pump);

    home_speed_arc_update(proc, speed_info->accent_color);
    home_speed_value_update(proc);
    home_speed_label_update(proc, speed_info->accent_color);
    home_speed_tip_update();
    home_speed_adjust_bg_update(thiz);
    speed_info->force_update = false;

    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> home speed adjust fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
