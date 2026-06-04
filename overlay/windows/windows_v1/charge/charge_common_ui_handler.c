/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "ui_page_define.h"  // IWYU pragma: keep
#include "window_screen_charge.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
 
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
void stop_charge_common_ui(ui_proc_charge_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    proc->last_progress_ani_time = get_timestamp();
    if (ui->hook && ui->hook->set_ui_backlight_level_rate) {
        ui->hook->set_ui_backlight_level_rate(ui, 100);
    }
}