/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "restart_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PROGRESS_ARC_SIZE                       (50)
#define PROGRESS_ARC_WIDTH                      (5)
#define PROGRESS_ARC_BG_COLOR                   (0x262626)
#define PROGRESS_ARC_COLOR                      (0xFFFFFF)
#define PROGRESS_ARC_LOOP_MS_PER_DEG            (3)
#define PROGRESS_ARC_ANGLE_SPAN                 (90)
#define PROGRESS_ARC_TEXT_COLOR                 (0xFFFFFF)

typedef struct {
    lv_obj_t *root;
    lv_obj_t *progress_arc;
    lv_obj_t *label_text;
    uint32_t start_time;
    uint8_t force_update;
    uint8_t rsv[15];
} restart_ui_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static restart_ui_info_t *ui_info;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void restart_ui_root_init(void)
{
    if (ui_info->root == NULL || !lv_obj_is_valid(ui_info->root)) {
        ui_info->root = lv_obj_create(lv_screen_active());
    }
    lv_obj_set_size(ui_info->root, LV_PCT(100), LV_PCT(100));
    lv_obj_set_style_bg_color(ui_info->root, lv_color_black(), 0);
    lv_obj_set_style_bg_opa(ui_info->root, LV_OPA_COVER, 0);
    lv_obj_set_style_border_width(ui_info->root, 0, 0);
    lv_obj_set_style_pad_all(ui_info->root, 0, 0);
    lv_obj_align(ui_info->root, LV_ALIGN_CENTER, 0, 0);
    lv_obj_clear_flag(ui_info->root, LV_OBJ_FLAG_CLICKABLE);
    lv_obj_clear_flag(ui_info->root, LV_OBJ_FLAG_SCROLLABLE);
}

static void restart_ui_update(void)
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
    lv_obj_set_style_arc_opa(ui_info->progress_arc, LV_OPA_COVER, LV_PART_MAIN);
    lv_obj_set_style_arc_color(ui_info->progress_arc, lv_color_hex(PROGRESS_ARC_COLOR), LV_PART_INDICATOR);
    lv_obj_set_style_arc_width(ui_info->progress_arc, PROGRESS_ARC_WIDTH, LV_PART_INDICATOR);
    lv_obj_set_style_arc_opa(ui_info->progress_arc, LV_OPA_COVER, LV_PART_INDICATOR);
    lv_obj_align(ui_info->progress_arc, LV_ALIGN_TOP_MID, 0, PROGRESS_ARC_Y);
    lv_obj_remove_style(ui_info->progress_arc, NULL, LV_PART_KNOB);
    if (ui_info->start_time == 0) {
        ui_info->start_time = get_timestamp();
    }

    if (ui_info->label_text == NULL || !lv_obj_is_valid(ui_info->label_text)) {
        ui_info->label_text = lv_label_create(ui_info->root);
        EXCEPTION_CHECK_VOID(ui_info->label_text);
    }
    lv_label_set_text(ui_info->label_text, RESTARTING_TEXT);
    lv_obj_set_style_text_font(ui_info->label_text, UI_FONT_TITLE_MD, 0);
    lv_obj_set_style_text_color(ui_info->label_text, lv_color_hex(PROGRESS_ARC_TEXT_COLOR), 0);
    lv_obj_set_style_text_align(ui_info->label_text, LV_TEXT_ALIGN_CENTER, 0);
    lv_obj_align_to(ui_info->label_text, ui_info->progress_arc, LV_ALIGN_OUT_BOTTOM_MID, 0, 24);

    uint32_t elapsed = get_timestamp() - ui_info->start_time;
    uint16_t start_angle = (uint16_t)((elapsed / PROGRESS_ARC_LOOP_MS_PER_DEG) % 360);
    uint16_t end_angle = start_angle + PROGRESS_ARC_ANGLE_SPAN;
    lv_arc_set_angles(ui_info->progress_arc, start_angle, end_angle);
    lv_obj_invalidate(ui_info->progress_arc);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_restart_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (ui_info == NULL) {
        ui_info = (restart_ui_info_t *)u_malloz(sizeof(restart_ui_info_t));
        EXCEPTION_CHECK_VOID(ui_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    ui_info->start_time = 0;
    ui_info->force_update = true;
    restart_ui_root_init();
}

void ui_display_restart_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz && ui_info);
    LVGL_OBJ_DELETE(ui_info->root);
    u_free(ui_info);
    ui_info = NULL;
}

int ui_display_restart_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz && ui_info && ui_info->root);
    restart_ui_update();
    ui_info->force_update = false;
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> restart fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
