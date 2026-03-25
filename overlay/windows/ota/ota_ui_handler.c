/*
* INCLUDE FILES
*****************************************************************************************
*/ 
#include "ui_page_define.h"  // IWYU pragma: keep
#include "window_screen_ota.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define OTA_DISPLAY_TASK_NAME              "ota_display_cmp"

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
static void ota_display_completed_cb(hdr_post_args_t *args)
{
   
}

static void ota_ui_after_policy(ui_manager_context_t *ui, int index)
{

}

static void ota_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    static int ani_interval = 0;
    ui_manager_context_t *ui = user_data;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            ota_ui_after_policy(ui, index);
            break;
        default:
            break;
    }
}

static void ota_ui_display_prepare(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_ota_prepare(ui);
}

static void ota_ui_display_remove(void *user_data)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_display_ota_remove(ui);
}

static void show_ota_ui(ui_proc_ota_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    if (proc->current_ui_type == OTA_UI_TYPE_UPDATE) {
        proc->last_progress_ani_time = get_timestamp();
        return;
    }
    ui_play_info_t play_info = {
        .tag = "ota_screen",
        .fps = OTA_ANIMATION_FREQ,
        .progress_cb = ota_animation_progress_cb,
        .display_prepare_cb = ota_ui_display_prepare,
        .display_remove_cb = ota_ui_display_remove,
        .user_data = proc,
    };
    proc->last_progress_ani_time = get_timestamp();
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->current_ui_type = OTA_UI_TYPE_UPDATE;
}

static void ota_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(OTA_DISPLAY_TASK_NAME);
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void start_ota_ui_animation(ui_proc_ota_t *proc)
{
    EXCEPTION_CHECK_VOID(proc->pm);
    while (get_timestamp_ms(0) < BOOT_LONG_PRESS_TIME) {
        if (proc->pm->is_system_power_on()) {
            break;
        }
        msleep(10);
    }
    LOG_UI("start ota ui animation.");
    show_ota_ui(proc);
}

void stop_ota_ui_animation(ui_proc_ota_t *proc)
{
    ota_ui_hdr_remove();
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_deinit);
    ui->hook->ui_animation_play_deinit(ui);
    proc->last_progress_ani_time = get_timestamp();
}