/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_img_define.h"   // IWYU pragma: keep
#include "ui_page_define.h" // IWYU pragma: keep
#include "ui_manager.h" // IWYU pragma: keep
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
void ui_display_pump_both_level_prepare(ui_manager_context_t *thiz)
{
#if IS_USE_SPECTRA_SUCTION_CURVE
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    int pump_mode = BREAST_PUMP_STIMULATION_MODE;
    if (proc && proc->pump) {
        READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    }
    uint8_t level_max = pump_mode == BREAST_PUMP_STIMULATION_MODE ? BREAST_PUMP_LEVEL_5 : BREAST_PUMP_LEVEL_15;
    common_pump_both_level_ui_prepare(thiz, level_max);
#else
    common_pump_both_level_ui_prepare(thiz);
#endif
    thiz->priv.fps_count = 0;
    thiz->priv.fps_start_time = get_timestamp();
}

void ui_display_pump_both_level_remove(ui_manager_context_t *thiz)
{
    common_pump_both_level_ui_remove(thiz);
}

int ui_display_pump_both_level_process(ui_manager_context_t *thiz)
{
    EXCEPTION_CHECK_INT(thiz);
    ui_proc_pump_t *proc = (ui_proc_pump_t *)thiz->priv.play_info.user_data;
    EXCEPTION_CHECK_INT(proc);
#if IS_USE_SPECTRA_SUCTION_CURVE
    int pump_mode = BREAST_PUMP_STIMULATION_MODE;
    if (proc->pump) {
        READ_PUMP_WORK_MODE(proc->pump, pump_mode);
    }
    uint8_t level_max = pump_mode == BREAST_PUMP_STIMULATION_MODE ?
        BREAST_PUMP_LEVEL_5 : BREAST_PUMP_LEVEL_15;
    common_pump_both_level_ui_sync_level_max(level_max);
#endif
    common_pump_both_level_ui_process(thiz, proc->setting.level,
        proc->both_level_change, proc->popup_ui_type);
    thiz->priv.fps_count++;
    if (get_timestamp() - thiz->priv.fps_start_time >= 1000) {
        LOG_UI("===> pump both level fps: %d", thiz->priv.fps_count);
        thiz->priv.fps_start_time = get_timestamp();
        thiz->priv.fps_count = 0;
    }   
    return 0;
}