/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common.h"
#include "ui_img_define.h"
#include "window_screen_pump.h"

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_BREASTFEEDING_MODE_DISPLAY_TASK_NAME       "brtf_mode_display_cmp"
#define PUMP_MODE_DISPLAY_TASK_NAME                     "pump_mode_display_cmp"

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
/// 泵奶追奶UI动画资源ID.
static const uint16_t s_pump_breastfeeding_mode_res_array[1] = {
    UI_IMG_ID_SETTINGS_SUBMENU,
};

/// 泵奶模式UI动画资源ID.
static const uint16_t s_pump_mode_res_array[1] = {
    UI_ID_PUMP_MODE_TIP
};
/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pump_mode_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_VOID(wm);
    if (wm->is_current_window(wm, ID_WINDOW_PUMP) == 0) {
        return;
    }

    if (proc->current_ui_type == PUMP_UI_TYPE_COMMON_MODE) {
        show_pump_home_ui(proc);
    }
}

static void pump_mode_ui_policy(ui_manager_context_t *ui)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (get_timestamp_ms(proc->last_progress_ani_time) >= PUMP_SUB_UI_DISPLAY_TIME) {
        POST_TASK_HANDLER_DELAYED_ARG1(pump_mode_display_completed_cb, 10, proc,
            PUMP_MODE_DISPLAY_TASK_NAME);
    } 
}

static void pump_mode_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_animation_play_t *ani = &ui->priv.play_info;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            pump_mode_ui_policy(ui);
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            break;
        default:
            break;
    }
}

static void show_pump_mode_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    ui_play_info_t play_info = {
        .tag = "pump_mode_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .res_array = (uint16_t *)s_pump_mode_res_array,
        .res_count = 1,
        .progress_cb = pump_mode_animation_progress_cb,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->last_progress_ani_time = get_timestamp();
    proc->current_ui_type = PUMP_UI_TYPE_COMMON_MODE;
}

static void pump_breastfeeding_mode_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_VOID(wm);
    if (wm->is_current_window(wm, ID_WINDOW_PUMP) == 0) {
        return;
    }

    if (proc->current_ui_type == PUMP_UI_TYPE_BREASTFEEDING_MODE) {
        show_pump_home_ui(proc);
    }
}

static void pump_breastfeeding_mode_ui_policy(ui_manager_context_t *ui)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (get_timestamp_ms(proc->last_progress_ani_time) >= PUMP_SUB_UI_DISPLAY_TIME) {
        POST_TASK_HANDLER_DELAYED_ARG1(pump_breastfeeding_mode_display_completed_cb, 10, proc,
            PUMP_BREASTFEEDING_MODE_DISPLAY_TASK_NAME);
    } 
}

static void pump_breastfeeding_mode_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_animation_play_t *ani = &ui->priv.play_info;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            pump_breastfeeding_mode_ui_policy(ui);
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            break;
        default:
            break;
    }
}

static void show_pump_breastfeeding_mode_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    ui_play_info_t play_info = {
        .tag = "brtf_mode_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .res_array = (uint16_t *)s_pump_breastfeeding_mode_res_array,
        .res_count = 1,
        .progress_cb = pump_breastfeeding_mode_animation_progress_cb,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->last_progress_ani_time = get_timestamp();
    proc->current_ui_type = PUMP_UI_TYPE_BREASTFEEDING_MODE;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int switch_pump_common_mode_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump, 
                                breast_pump_info_t *pump_info)
{
    if (proc->info.common_mode < BREAST_PUMP_MIX_MODE) {
        proc->info.common_mode++;
    } else {
        proc->info.common_mode = BREAST_PUMP_STIMULATION_MODE;
    }
    pump->set_breast_pump_mode(proc->info.common_mode);
    proc->current_ui_mode = proc->info.common_mode;
    show_pump_mode_ui(proc);
    LOG_UI("switch pump common mode: %d, mode: %s", proc->info.common_mode, pump->get_breast_pump_mode_name(proc->info.common_mode));
    return 0;
}

int switch_pump_breastfeeding_mode_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump,
                        breast_pump_info_t *pump_info)
{
    if (proc->info.breastfeeding_mode < BREAST_PUMP_CUSTOM_MODE || proc->info.breastfeeding_mode >= BREAST_PUMP_LACTATION_START_MODE) {
        proc->info.breastfeeding_mode = BREAST_PUMP_CUSTOM_MODE;
    } else {
        proc->info.breastfeeding_mode++;
    }
    pump->set_breast_pump_mode(proc->info.breastfeeding_mode);
    proc->current_ui_mode = proc->info.breastfeeding_mode;
    show_pump_breastfeeding_mode_ui(proc);
    LOG_UI("switch pump breastfeeding mode: %d, mode: %s", proc->info.breastfeeding_mode, 
        pump->get_breast_pump_mode_name(proc->info.breastfeeding_mode));
    return 0;
}

void pump_mode_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_MODE_DISPLAY_TASK_NAME);
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_BREASTFEEDING_MODE_DISPLAY_TASK_NAME);
}