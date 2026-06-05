/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h" // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
#include "window_screen_home.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
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
void ui_display_home_pump_both_level_prepare(ui_manager_context_t *thiz)
{
    common_pump_both_level_ui_prepare(thiz);
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_home_pump_both_level_remove(ui_manager_context_t *thiz)
{
    common_pump_both_level_ui_remove(thiz);
}

int ui_display_home_pump_both_level_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    ui_proc_home_t *proc = (ui_proc_home_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    common_pump_both_level_ui_process(thiz, proc->setting.level,
        proc->both_level_change, proc->popup_ui_type);
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        LOG_UI("===> home pump both level fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_start_time = get_timestamp();
        thiz->priv.fps_count = 0;
    }
    return 0;
}
