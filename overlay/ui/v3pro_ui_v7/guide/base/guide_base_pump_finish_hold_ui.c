/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "guide_base_ui_internal.h" // IWYU pragma: keep
#include "pump_stop_hold_ui.h" // IWYU pragma: keep

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void guide_pump_finish_hold_ui_remove(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info);
    if (info->stop_hold_active == 0 || info->ui == NULL) {
        info->stop_hold_active = 0;
        return;
    }
    ui_display_pump_stop_hold_remove(info->ui);
    info->stop_hold_active = 0;
}

void guide_pump_finish_hold_ui_build(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);
    if (info->stop_hold_active != 0) {
        return;
    }
    ui_display_pump_stop_hold_prepare(info->ui);
    info->stop_hold_active = 1;
}

void guide_pump_finish_hold_ui_update(guide_base_ui_info_t *info)
{
    EXCEPTION_CHECK_VOID(info && info->ui);
    if (info->stop_hold_active == 0) {
        return;
    }
    ui_display_pump_stop_hold_process(info->ui);
}
