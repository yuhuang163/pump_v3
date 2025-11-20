/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "cmsis_os_api.h"
#include "root_common.h"
#include "ui_img_define.h"
#include "window_screen_pump.h"

/*
* DEFINES
*****************************************************************************************
*/
#define PUMP_COMPLETE_DISPLAY_TASK_NAME         "pump_complete_display"

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
/// 泵奶工作UI动画资源ID.
static const uint16_t s_pumping_res_array[1] = {
    UI_IMG_ID_HOME_SCREEN,
};

/// 泵奶结果UI动画资源ID.
static const uint16_t s_pump_result_res_array[1] = {
    UI_IMG_ID_PUMP_COMPLETE_TIP,
};

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void pumping_work_ui_policy(ui_manager_context_t *ui)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    breast_pump_manager_context_t *pump = proc->wm->priv.pump;
    EXCEPTION_CHECK_VOID(pump);
    /// 如果充电状态发生变化，则刷新UI.
    if (proc->charging_status != proc->wm->priv.misc->is_charge_plug_in()) {
        proc->charging_status = proc->wm->priv.misc->is_charge_plug_in();
        LOG_UI("charging status changed: %d", proc->charging_status);
        goto UI_DISPLAY_REFRESH;
    }

    /// 如果上次动画时间小于更新间隔，则不刷新UI.
    if (get_timestamp_ms(proc->last_progress_ani_time) < PUMP_RUNNING_TIME_UPDATE_INTERVAL) {
        return;
    }
    
    /// 如果泵奶停止且电池电量百分比未发生变化，则不刷新UI.
    if (pump->is_on_working() == 0 && proc->battery_percent == proc->wm->priv.misc->get_battery_percent()) {
        return;
    }

UI_DISPLAY_REFRESH: 
    proc->battery_percent = proc->wm->priv.misc->get_battery_percent();
    proc->last_progress_ani_time = get_timestamp();
    if (ui->priv.play_info.reset_play_count_cb) {
        ui->priv.play_info.reset_play_count_cb(&ui->priv.play_info);
    }
}

static void pumping_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            pumping_work_ui_policy(ui);
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            break;
        default:
            break;
    }
}

static void pump_result_display_completed_cb(hdr_post_args_t *args)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)args->user_data;
    EXCEPTION_CHECK_VOID(proc);
    window_manager_context_t *wm = proc->wm;
    EXCEPTION_CHECK_VOID(wm);
    if (wm->is_current_window(wm, ID_WINDOW_PUMP) == 0) {
        return;
    }
    
    if (proc->current_ui_type == PUMP_UI_TYPE_RESULT) {
        show_pump_home_ui(proc);
    }
}

static void pump_result_ui_policy(ui_manager_context_t *ui)
{
    ui_proc_pump_t *proc = (ui_proc_pump_t *)ui->priv.play_info.user_data;
    EXCEPTION_CHECK_VOID(proc);
    if (get_timestamp_ms(proc->last_progress_ani_time) >= PUMP_SUB_UI_DISPLAY_TIME) {
        POST_TASK_HANDLER_DELAYED_ARG1(pump_result_display_completed_cb, 10, proc,
            PUMP_COMPLETE_DISPLAY_TASK_NAME);
    }
}

static void pump_result_animation_progress_cb(void *user_data, int index, ui_ani_progress_state_e state)
{
    ui_manager_context_t *ui = user_data;
    EXCEPTION_CHECK_VOID(ui);
    ui_animation_play_t *ani = &ui->priv.play_info;
    switch (state) {
        case ANI_TIMER_DRAW_FINISH:
            pump_result_ui_policy(ui);
            break;
        case ANI_TIMER_DRAW_BEFORE:
            break;
        case ANI_TIMER_DRAW_AFTER:
            break;
        default:
            break;
    }
}

static void show_pump_result_loop(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    ui_play_info_t play_info = {
        .tag = "pump_result_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .res_array = (uint16_t *)s_pump_result_res_array,
        .res_count = LENGTH_OF_ARRAY(s_pumping_res_array),
        .progress_cb = pump_result_animation_progress_cb,
        .user_data = proc,
    };
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->last_progress_ani_time = get_timestamp();
    proc->current_ui_type = PUMP_UI_TYPE_RESULT;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void show_pump_home_ui(ui_proc_pump_t *proc)
{
    ui_manager_context_t *ui = proc->ui;
    EXCEPTION_CHECK_VOID(ui->hook && ui->hook->ui_animation_play_init);
    ui_play_info_t play_info = {
        .tag = "pumping_screen",
        .fps = PUMP_ANIMATION_FREQ,
        .res_array = (uint16_t *)s_pumping_res_array,
        .res_count = LENGTH_OF_ARRAY(s_pumping_res_array),
        .progress_cb = pumping_animation_progress_cb,
        .user_data = proc,
    };
    EXCEPTION_CHECK_VOID(proc->wm->priv.misc);
    proc->charging_status = proc->wm->priv.misc->is_charge_plug_in();
    ui->hook->ui_animation_play_init(ui, play_info);
    proc->last_progress_ani_time = get_timestamp();
    proc->current_ui_type = PUMP_UI_TYPE_PUMPING;
}

int switch_pump_status_ui(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump, int status)
{
    if (status == BREAST_PUMP_PAUSE) {
        LOG_UI("pump status:PAUSE => set_breast_pump_status:CONTINUE");
        pump->set_breast_pump_status(BREAST_PUMP_CONTINUE);
    } else if (status == BREAST_PUMP_IDLE || status == BREAST_PUMP_FINISH) {
        LOG_UI("first enter: ->current_ui_mode: %d", proc->current_ui_mode);
        pump->set_breast_pump_status(BREAST_PUMP_START);
    } else {
        LOG_UI("pump status:%d => set_breast_pump_status:PAUSE", pump->get_breast_pump_status_name(status));
        pump->set_breast_pump_status(BREAST_PUMP_PAUSE);
    }
    show_pump_home_ui(proc);
    return 0;
}

void show_pump_progress_time_policy(ui_proc_pump_t *proc, breast_pump_manager_context_t *pump)
{
    EXCEPTION_CHECK_VOID(pump);
    int pump_status = pump->get_breast_pump_status_async();
    LOG_UI("pump status: %s", pump->get_breast_pump_status_name(pump_status));
    if (PUMP_IS_GOING(pump_status)) {
        if (proc->pump_result.real_valid_time_sec < (SINGLE_PUMP_TIME_MAX / 1000)) {
            LOG_UI("=>set_breast_pump_status:PAUSE");
            pump->set_breast_pump_status(BREAST_PUMP_PAUSE);
        } else {
            LOG_UI("=>set_breast_pump_status:FINISH");
            pump->set_breast_pump_status(BREAST_PUMP_FINISH);
        }
    }

    if (IS_PUMPTIME_LESS_THEN_30S(&proc->pump_result)) {
        LOG_UI("PUMPTIME < 30s, do not loop show!");
    } else {
        show_pump_result_loop(proc);
    }
}

void pumping_ui_hdr_remove(void)
{
    REMOVE_TASK_HANDLER_DIRECTLY(PUMP_COMPLETE_DISPLAY_TASK_NAME);
}
