/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "window_screen_pump.h"
#include "breast_pump_ui.h"

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
void pump_level_custom_ui_prepare(ui_manager_context_t *thiz)
{
   ui_pump_level_custom_prepare(thiz);
   thiz->priv.fps_count = 0;
   thiz->priv.fps_start_time = get_timestamp();
}

void pump_level_custom_ui_remove(ui_manager_context_t *thiz)
{
    ui_pump_level_custom_remove(thiz);
}

int pump_level_custom_ui_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
    if (proc->info.control_type == BREAST_PUMP_CONTROL_TYPE_LEFT) {
        ui_pump_level_custom_process(thiz, proc->setting.level[0], 
            proc->level_change, true, proc->popup_ui_type);
    } else {
        ui_pump_level_custom_process(thiz, proc->setting.level[1],
            proc->level_change, false, proc->popup_ui_type);
    }
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        LOG_UI("===> pump level fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_start_time = get_timestamp();
        thiz->priv.fps_count = 0;
    }
    
    return 0;
}