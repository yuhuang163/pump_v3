/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PROGRESS_ARC_SIZE                       (28)    /* 直径（原80缩小20%） */
#define PROGRESS_ARC_WIDTH                      (3)    /* 圆弧线宽（原4加粗50%） */
#define PROGRESS_ARC_BG_COLOR                   (0xFFFFFF)    /* 背景圆弧颜色：黑色 */
#define PROGRESS_ARC_BG_OPA                     (38)        /* 背景圆弧透明度 15%（255*15/100≈38） */
#define PROGRESS_ARC_COLOR                      (0xB25962)    /* 圆弧颜色：粉色 */
#define PROGRESS_ARC_LOOP_MS_PER_DEG            (3)    /* ms per degree，越小转得越快（原15约5.4s/圈，现约2.2s/圈） */
#define PROGRESS_ARC_ANGLE_SPAN                 (90)   /* 25% of 360 */

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
typedef struct {
    lv_obj_t * root;
    lv_obj_t * progress_arc;
    lv_obj_t * label_text;
    uint32_t start_time;
    uint8_t force_update;
    uint8_t rsv[15];
} ui_power_off_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static ui_power_off_info_t *ui_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void ui_power_off_root_init(void)
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

static void ui_power_off_update(ui_manager_context_t *thiz)
{
    if (ui_info->progress_arc == NULL || !lv_obj_is_valid(ui_info->progress_arc)) {
        ui_info->progress_arc = lv_arc_create(ui_info->root);
        EXCEPTION_CHECK_VOID(ui_info->progress_arc);
    }
    lv_obj_set_size(ui_info->progress_arc, PROGRESS_ARC_SIZE, PROGRESS_ARC_SIZE);
    lv_obj_center(ui_info->progress_arc);
    lv_arc_set_bg_angles(ui_info->progress_arc, 0, 360);
    lv_obj_set_style_arc_color(ui_info->progress_arc, lv_color_hex(PROGRESS_ARC_BG_COLOR), LV_PART_MAIN);
    lv_obj_set_style_arc_width(ui_info->progress_arc, PROGRESS_ARC_WIDTH, LV_PART_MAIN);
    lv_obj_set_style_arc_opa(ui_info->progress_arc, PROGRESS_ARC_BG_OPA, LV_PART_MAIN);
    lv_obj_set_style_arc_color(ui_info->progress_arc, lv_color_hex(PROGRESS_ARC_COLOR), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(ui_info->progress_arc, PROGRESS_ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(ui_info->progress_arc, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_align(ui_info->progress_arc, LV_ALIGN_TOP_MID, 0, 152);
    lv_obj_remove_style(ui_info->progress_arc, NULL, LV_PART_KNOB);
    if (ui_info->start_time == 0) {
        ui_info->start_time = get_timestamp();
    }

    if (ui_info->label_text == NULL || !lv_obj_is_valid(ui_info->label_text)) {
        ui_info->label_text = lv_label_create(ui_info->root);
        EXCEPTION_CHECK_VOID(ui_info->label_text);
    }
    
    lv_label_set_text(ui_info->label_text, "Power off");
    lv_obj_set_style_text_font(ui_info->label_text, &lv_font_oppo_sans_14_medium, 0);
    lv_obj_set_style_text_line_space(ui_info->label_text, 9, 0);  /* line height 24px (font ~13 + 11) */
    lv_obj_align_to(ui_info->label_text, ui_info->progress_arc, LV_ALIGN_OUT_BOTTOM_MID, 0, 16);
    uint32_t elapsed = get_timestamp() - ui_info->start_time;
    uint16_t start_angle = (uint16_t)((elapsed / PROGRESS_ARC_LOOP_MS_PER_DEG) % 360);
    uint16_t end_angle = start_angle + PROGRESS_ARC_ANGLE_SPAN;
    lv_arc_set_angles(ui_info->progress_arc, start_angle, end_angle);
    lv_obj_invalidate(ui_info->progress_arc); /// 使圆弧/图片区域重绘
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_power_off_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (ui_info == NULL) {
        ui_info = (ui_power_off_info_t *)u_malloz(sizeof(ui_power_off_info_t));
        EXCEPTION_CHECK_VOID(ui_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    ui_power_off_root_init();
    ui_info->force_update = true;
}

void ui_display_power_off_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(ui_info);
    LVGL_OBJ_DELETE(ui_info->root);
    u_free(ui_info);
    ui_info = NULL;
}

int ui_display_power_off_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(ui_info);
    EXCEPTION_CHECK_INT(ui_info->root);
    ui_power_off_update(thiz);
    ui_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> power off fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
