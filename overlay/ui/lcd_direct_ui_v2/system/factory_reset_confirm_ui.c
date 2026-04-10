/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_system.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_FACTORY_RESET_COMP_BG                  (0)

#define FACTORY_RESET_CONFIRM_FIRST_TEXT_COLOR            (0x8C8C8C)
#define FACTORY_RESET_CONFIRM_TEXT_COLOR                  (0xFFFFFF)
#define FACTORY_RESET_CONFIRM_ARC_SIZE                    (360)    // 圆弧尺寸
#define FACTORY_RESET_CONFIRM_ARC_WIDTH                   (12)      // 圆弧粗细
#define FACTORY_RESET_CONFIRM_ARC_COLOR                   (0xB25962) // 圆弧颜色（匹配吸奶器粉色）
#define FACTORY_RESET_CONFIRM_ARC_START_ANGLE             (270)    
#define FACTORY_RESET_CONFIRM_ARC_FULL_SWEEP              (360)    
#define FACTORY_RESET_CONFIRM_DISPLAY_TIME                (3000)  
#define FACTORY_RESET_CONFIRM_DISPLAY_STEP_TIME           (100)
#define FACTORY_RESET_CONFIRM_DISPLAY_MAX_VALUE           ((FACTORY_RESET_CONFIRM_DISPLAY_TIME / FACTORY_RESET_CONFIRM_DISPLAY_STEP_TIME) - 2)

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_FACTORY_RESET_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * text[3];
    lv_obj_t * arc;                    /// 圆弧进度条对象
    uint32_t start_time;
    uint16_t ring_progress;            /// 圆弧进度
    uint8_t force_update;
    uint8_t display_progress;
    uint8_t rsv[12];
} ui_factory_reset_confirm_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static ui_factory_reset_confirm_info_t *ui_info;
#if IS_SUPPORT_FACTORY_RESET_COMP_BG
static PSRAM_DATA_LOC uint8_t factory_reset_confirm_bg_data[] = {
    #include "comparison_png2txt/factory_reset_popup_360x360.txt"
};
#endif

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_factory_reset_confirm_root_init(void)
{
    if (ui_info->root == NULL || !lv_obj_is_valid(ui_info->root)) {
        ui_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(ui_info->root, LV_PCT(100), LV_PCT(100));          /// 设置容器尺寸为屏幕的100%（自动适配屏幕分辨率）
    lv_obj_set_style_bg_color(ui_info->root, lv_color_black(), 0);     /// 设置背景颜色为黑色
    lv_obj_set_style_bg_opa(ui_info->root, LV_OPA_COVER, 0);           /// 设置背景不透明度为完全不透明（覆盖下层）
    lv_obj_set_style_border_width(ui_info->root, 0, 0);                /// 设置边框宽度为0（无边框）
    lv_obj_set_style_pad_all(ui_info->root, 0, 0);                     /// 设置所有内边距为0（子对象紧贴边缘）
    lv_obj_align(ui_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(ui_info->root, LV_OBJ_FLAG_CLICKABLE); /// 让 root 不吃事件，防止挡子控件
    lv_obj_clear_flag(ui_info->root, LV_OBJ_FLAG_SCROLLABLE); /// 禁止滚动
}

static void ui_factory_reset_confirm_update(ui_manager_context_t *thiz)
{
    if (ui_info->force_update == false) {
        return;
    }

    if (ui_info->text[0] == NULL || !lv_obj_is_valid(ui_info->text[0])) {
        ui_info->text[0] = lv_label_create(ui_info->root);
        EXCEPTION_CHECK_VOID(ui_info->text[0]);
        lv_label_set_text(ui_info->text[0], "Reset settings");
        lv_obj_set_style_text_align(ui_info->text[0], LV_TEXT_ALIGN_CENTER, 0);  /// 居中对齐
        lv_obj_set_style_text_line_space(ui_info->text[0], 4, 0); /// 设置行间距（像素）
        lv_obj_set_style_text_color(ui_info->text[0], lv_color_hex(FACTORY_RESET_CONFIRM_FIRST_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(ui_info->text[0], &lv_font_oppo_sans_16_medium, 0);
        // lv_obj_set_style_text_opa(ui_info->text[0], 140, 0); /// 设置文本透明度为140
        lv_obj_align(ui_info->text[0], LV_ALIGN_TOP_MID, 0, 144);
    }

    if (ui_info->text[1] == NULL || !lv_obj_is_valid(ui_info->text[1])) {
        ui_info->text[1] = lv_label_create(ui_info->root);
        EXCEPTION_CHECK_VOID(ui_info->text[1]);
        lv_label_set_text(ui_info->text[1], "Press and hold the round\nbutton for 3s");
        lv_obj_set_style_text_align(ui_info->text[1], LV_TEXT_ALIGN_CENTER, 0);  /// 居中对齐
        lv_obj_set_style_text_line_space(ui_info->text[1], 4, 0); /// 设置行间距（像素）
        lv_obj_set_style_text_color(ui_info->text[1], lv_color_hex(FACTORY_RESET_CONFIRM_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(ui_info->text[1], &lv_font_oppo_sans_20_medium, 0);
        lv_obj_align(ui_info->text[1], LV_ALIGN_TOP_MID, 0, 173);
    }


    if (ui_info->text[2] == NULL || !lv_obj_is_valid(ui_info->text[2])) {
        ui_info->text[2] = lv_label_create(ui_info->root);
        EXCEPTION_CHECK_VOID(ui_info->text[2]);
        lv_label_set_text(ui_info->text[2], "Press to exit");
        lv_obj_set_style_text_align(ui_info->text[2], LV_TEXT_ALIGN_CENTER, 0);  /// 居中对齐
        lv_obj_set_style_text_line_space(ui_info->text[2], 4, 0); /// 设置行间距（像素）
        lv_obj_set_style_text_color(ui_info->text[2], lv_color_hex(FACTORY_RESET_CONFIRM_TEXT_COLOR), 0);
        lv_obj_set_style_text_font(ui_info->text[2], &lv_font_oppo_sans_16_medium, 0);
        lv_obj_align(ui_info->text[2], LV_ALIGN_TOP_MID, 0, 302);
    }
}

static void ui_factory_reset_confirm_arc_update(ui_manager_context_t *thiz, uint8_t display_progress)
{
    if (ui_info->arc == NULL || !lv_obj_is_valid(ui_info->arc)) {
        ui_info->arc = lv_arc_create(ui_info->root);
        EXCEPTION_CHECK_VOID(ui_info->arc);
        lv_obj_set_size(ui_info->arc, FACTORY_RESET_CONFIRM_ARC_SIZE, FACTORY_RESET_CONFIRM_ARC_SIZE); /// 设置弧线大小
        lv_obj_center(ui_info->arc); /// 设置弧线居中
        lv_arc_set_bg_angles(ui_info->arc, 0, FACTORY_RESET_CONFIRM_ARC_FULL_SWEEP);  /// 背景弧为完整闭环
        lv_arc_set_angles(ui_info->arc, 0, 0);  /// 前景弧初始角度
        lv_arc_set_rotation(ui_info->arc, FACTORY_RESET_CONFIRM_ARC_START_ANGLE);  /// 从 270 度开始绘制
        lv_obj_set_style_arc_color(ui_info->arc, lv_color_hex(FACTORY_RESET_CONFIRM_ARC_COLOR), LV_PART_MAIN);  /// 背景弧颜色
        lv_obj_set_style_arc_width(ui_info->arc, FACTORY_RESET_CONFIRM_ARC_WIDTH, LV_PART_MAIN);
        lv_obj_set_style_arc_opa(ui_info->arc, LV_OPA_TRANSP, LV_PART_MAIN);  /// 背景弧透明
        lv_obj_set_style_arc_color(ui_info->arc, lv_color_hex(FACTORY_RESET_CONFIRM_ARC_COLOR), LV_PART_INDICATOR);  /// 前景弧颜色
        lv_obj_set_style_arc_width(ui_info->arc, FACTORY_RESET_CONFIRM_ARC_WIDTH, LV_PART_INDICATOR);
        lv_obj_set_style_arc_opa(ui_info->arc, LV_OPA_COVER, LV_PART_INDICATOR);  /// 前景弧不透明
        lv_obj_remove_style(ui_info->arc, NULL, LV_PART_KNOB); /// 去掉旋钮
    }

    /// 隐藏或显示圆弧
    if (display_progress == 0) {
        lv_obj_add_flag(ui_info->arc, LV_OBJ_FLAG_HIDDEN); /// 隐藏
        return;
    } else {
        lv_obj_clear_flag(ui_info->arc, LV_OBJ_FLAG_HIDDEN); /// 显示
    }

    /// 计算圆弧进度
    uint16_t ring_progress = get_timestamp_ms(ui_info->start_time) / FACTORY_RESET_CONFIRM_DISPLAY_STEP_TIME;
    if (ui_info->force_update == false && ui_info->ring_progress == ring_progress) {
        return;
    }
    ui_info->ring_progress = ring_progress;

    if (ring_progress >= FACTORY_RESET_CONFIRM_DISPLAY_MAX_VALUE) {
        lv_obj_set_style_arc_opa(ui_info->arc, LV_OPA_COVER, LV_PART_MAIN);      /// 最后一帧直接显示完整闭环
        lv_obj_set_style_arc_opa(ui_info->arc, LV_OPA_TRANSP, LV_PART_INDICATOR);
        lv_arc_set_angles(ui_info->arc, 0, 0);
        return;
    }

    lv_obj_set_style_arc_opa(ui_info->arc, LV_OPA_TRANSP, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(ui_info->arc, LV_OPA_COVER, LV_PART_INDICATOR);

    int16_t end_angle = 0;
    if (ring_progress > 0) {
        end_angle = (FACTORY_RESET_CONFIRM_ARC_FULL_SWEEP * ring_progress) / FACTORY_RESET_CONFIRM_DISPLAY_MAX_VALUE;
    }
    lv_arc_set_angles(ui_info->arc, 0, end_angle);
}

#if IS_SUPPORT_FACTORY_RESET_COMP_BG
static void ui_factory_reset_confirm_background_update(ui_manager_context_t *thiz)
{
    static uint32_t start_time = 0;
    static uint8_t status = 0;
    if (ui_info->background == NULL || !lv_obj_is_valid(ui_info->background)) {
        ui_info->background = lv_image_create(ui_info->root);
        EXCEPTION_CHECK_VOID(ui_info->background);
    }
    lv_fs_path_ex_t bg_img_path;
    if (status == 0) {
        lv_obj_add_flag(ui_info->background, LV_OBJ_FLAG_HIDDEN);
    } else {
        lv_obj_clear_flag(ui_info->background, LV_OBJ_FLAG_HIDDEN);
        lv_fs_make_path_from_buffer(&bg_img_path, LV_FS_MEMFS_LETTER, factory_reset_confirm_bg_data,
            sizeof(factory_reset_confirm_bg_data), "png");
        lv_image_set_src(ui_info->background, (const void *)&bg_img_path);
        lv_obj_align(ui_info->background, LV_ALIGN_CENTER, 0, 0);
    }

    if (get_timestamp() - start_time > 1000) {
        start_time = get_timestamp();
        status = (status == 0) ? 1 : 0;
    }
}
#endif

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_factory_reset_confirm_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (ui_info == NULL) {
        ui_info = (ui_factory_reset_confirm_info_t *)u_malloz(sizeof(ui_factory_reset_confirm_info_t));
        EXCEPTION_CHECK_VOID(ui_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    ui_factory_reset_confirm_root_init();
    ui_info->force_update = true;
}

void ui_display_factory_reset_confirm_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(ui_info);
    LVGL_OBJ_DELETE(ui_info->root);
    u_free(ui_info);
    ui_info = NULL;
}

int ui_display_factory_reset_confirm_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(ui_info);
    EXCEPTION_CHECK_INT(ui_info->root);
    ui_proc_system_t *proc = (ui_proc_system_t *)thiz->priv.play_info.user_data;
#if IS_SUPPORT_FACTORY_RESET_COMP_BG
    ui_factory_reset_confirm_background_update(thiz);
#endif
    ui_factory_reset_confirm_update(thiz);
    if (proc->display_fac_rst_confirm_progress != ui_info->display_progress) {
        ui_info->display_progress = proc->display_fac_rst_confirm_progress;
        ui_info->start_time = get_timestamp();
    }
    ui_factory_reset_confirm_arc_update(thiz, ui_info->display_progress);
    ui_info->force_update = false;
    thiz->priv.fps_count++;
    
    if (get_timestamp_ms(ui_info->start_time) >= FACTORY_RESET_CONFIRM_DISPLAY_TIME &&
        ui_info->display_progress == 1) {
        ui_animation_play_t *ani = &thiz->priv.play_info;
        EXCEPTION_CHECK_INT(ani);
        LOG_UI("factory reset confirm process completed");
        ani->progress_cb(thiz, 0, ANI_TIMER_DRAW_FINISH);
    }

    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> factory reset confirm fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
