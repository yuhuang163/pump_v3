/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "window_screen_factory_reset.h"

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
void stop_factory_reset_ui_animation(ui_proc_factory_reset_t *proc)
{
    factory_reset_ui_hdr_remove();
    factory_reset_confirm_ui_hdr_remove();
    EXCEPTION_CHECK_VOID(proc);
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui && ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    proc->last_progress_ani_time = get_timestamp();
}

