/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h"   // IWYU pragma: keep
#include "window_screen_pump.h" // IWYU pragma: keep
#include "breast_pump_ui.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_SUPPORT_LEVEL_COMP_BG                        (0)

#define PUMP_LEVEL_ARC_SEL_COLOR                        0xB3686D
#define PUMP_LEVEL_ARC_UNSEL_COLOR                      0xF7EFEF
#define PUMP_LEVEL_LINE_COLOR                           0xD9D9D9
#define PUMP_LEVEL_TEXT_COLOR                           0x393535
#define PUMP_LEVEL_VALUE_COLOR                          0x1C1C20
#define PUMP_LEVEL_LABLE_UNSEL_COLOR                    0xF7EFEF
#define PUMP_LEVEL_LABLE_SEL_COLOR                      0xB3686D
#define PUMP_LEVEL_TEXT_SEL_COLOR                       0xB3686D
#define PUMP_LEVEL_TEXT_UNSEL_COLOR                     0xFFFFFF

typedef struct {
    lv_obj_t * root;
#if IS_SUPPORT_LEVEL_COMP_BG
    lv_obj_t * background;
#endif
    lv_obj_t * arc;
    lv_obj_t * arc_progress;
    lv_obj_t * left_level_label;
    lv_obj_t * left_level_label_text;
    lv_obj_t * right_level_label;
    lv_obj_t * right_level_label_text;
    lv_obj_t * root_value;
    lv_obj_t * root_text;
} level_display_info_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
#if IS_SUPPORT_LEVEL_COMP_BG
static PSRAM_DATA_LOC uint8_t setting_bg_data[] = {
    // #include "comparison_png2txt/left_level_comp_360x360.txt"
    #include "comparison_png2txt/right_level_comp_360x360.txt"
    // #include "comparison_png2txt/speed_comp_360x360.txt"
};
#endif

static level_display_info_t *level_info;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void ui_display_pump_level_prepare(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (level_info == NULL) {
        level_info = (level_display_info_t *)u_malloz(sizeof(level_display_info_t));
        EXCEPTION_CHECK_VOID(level_info);
    }
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
    // pump_level_root_init();
}

void ui_display_pump_level_remove(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_VOID(thiz);
    if (level_info == NULL) {
        return;
    }
    LVGL_OBJ_DELETE(level_info->root);
    u_free(level_info);
    level_info = NULL;
}

int ui_display_pump_level(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    EXCEPTION_CHECK_INT(level_info->root);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    EXCEPTION_CHECK_INT(proc->pump);
    EXCEPTION_CHECK_INT(proc->motor);
    EXCEPTION_CHECK_INT(proc->st);
    uint8_t control_type = BREAST_PUMP_CONTROL_TYPE_BILATERAL;
    proc->st->get_store_ram_val(STORE_ID_PUMP_CONTROL_TYPE, 0, 0, &control_type);
    pump_setting_t setting;
    uint8_t time_index = proc->motor->get_runtime_policy_index(MOTOR_TIME_POLICY);
    proc->pump->get_breast_pump_setting(&setting, time_index >= ROOT_BREAST_PUMP_STEP_MAX ? 0 : time_index);
    uint8_t level = control_type == BREAST_PUMP_CONTROL_TYPE_LEFT ? setting.level[0] : setting.level[1];
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        thiz->priv.fps_start_time = get_timestamp();
        LOG_UI("===> pump left level fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_count = 0;
    }
    return 0;
}
