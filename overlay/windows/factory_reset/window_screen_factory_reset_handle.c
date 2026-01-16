/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "breast_pump_manager_api.h"
#include "input_key_code.h" // IWYU pragma: keep
#include "root_common_api.h" // IWYU pragma: keep
#include "windows_manager.h" 
#include "windows_map_define.h" // IWYU pragma: keep
#include "window_screen_factory_reset.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                            factory_reset
#define PAGE_NAME_STR                               "factory_reset"
#define FACTORY_RESET_DELAY_TIME                    (1000 * 3)
#define FACTORY_RESET_TASK_NAME                     "factory_reset_task"

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
INIT_WINDOW_CONTEXT();

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void factory_reset_led_blink_policy(ui_manager_context_t *ui)
{
   led_blink_info_t blink_info = {
    .led_index = PUMP_LED_ALL_ID,
    .period = 500,
    .continue_time = 3000,
    .first_status = LED_STATUS_OFF,
    .finish_status = LED_STATUS_OFF,
   };
   LOG_UI("factory reset led blink policy");
   ui->hook->set_led_blink(ui, blink_info);
}

static void factory_reset_cb(hdr_post_args_t *args)
{
    if (!GET_WINDOW_CONTEXT()) {
        LOG_WM("window context not present, ignore.");
        return;
    }

    ui_proc_factory_reset_t *proc = (ui_proc_factory_reset_t *)args->user_data;
    stop_factory_reset_ui_animation(proc);
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    pm->acquire_wakelock("reboot", 10);
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    LOGW("==========RESET_AND_REBOOT==========");
    st->factory_reset(RESET_AND_REBOOT);
}

static int entry_event_cb(window_lift_cycle_e new_state,
    const struct window_context *thiz, const struct window_context *old_scr)
{
    ui_proc_factory_reset_t *proc = CONTAINER_OF(thiz, ui_proc_factory_reset_t, w_ctx);
    breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    window_manager_context_t *wm = proc->wm;
    ui_manager_context_t *ui = proc->ui;

    if (new_state == ID_SCR_STATE_ON_RESUME) {

    } else if (new_state == ID_SCR_STATE_ON_CREATE) {
        LOG_WM("==>%s window on create", PAGE_NAME_STR);
        ui->animation_play_status_update(ui, false); /// 清除动画播放标志
        factory_reset_led_blink_policy(ui);
        start_factory_reset_ui_animation(proc);
        if (pump && pump->is_on_working()) {
            LOG_WM("factory reset, stop breast pump work.");
            pump->set_breast_pump_status(BREAST_PUMP_IDLE);
        }
        POST_TASK_HANDLER_DELAYED_ARG1(factory_reset_cb, FACTORY_RESET_DELAY_TIME, proc, FACTORY_RESET_TASK_NAME);
    }
    return 0;
}

static int exit_event_cb(window_lift_cycle_e new_state,
                         const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_factory_reset_t *proc = CONTAINER_OF(thiz, ui_proc_factory_reset_t, w_ctx);
    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE:
            LOG_WM("==>%s window on pause", PAGE_NAME_STR);
            stop_factory_reset_ui_animation(proc);
            break;
        case ID_SCR_STATE_ON_DESTORY:
            LOG_WM("%s window exit: u_free: %p", PAGE_NAME_STR, proc);
            u_free(proc);
            proc = NULL;
            SET_WINDOW_CONTEXT(NULL);
            LOG_WM("%s window exit--end", PAGE_NAME_STR);
            break;
        default:
            break;
    }
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
window_enter_result_e PAGE_HANDLE_NAME(PAGE_STATIC_NAME)(window_manager_context_t *wm, uint8_t window_id)
{
    LOG_WM("%s window enter, time: %ld.", PAGE_NAME_STR, get_timestamp());
    ui_proc_factory_reset_t *proc = u_malloz(sizeof(ui_proc_factory_reset_t));
    SET_WINDOW_CONTEXT(proc);
    proc->margic = UI_WINDOW_MARGIC_CODE;
    proc->current_ui_type = FACTORY_RESET_UI_TYPE_NONE;
    proc->wm = wm;
    proc->ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    proc->pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    window_context_t *w_ctx = &proc->w_ctx;
    w_ctx->new_window_id = window_id;
    w_ctx->evt_cb_entry = entry_event_cb;
    w_ctx->evt_cb_exit = exit_event_cb;
    w_ctx->not_push_history = 1;
    wm->dispatch_new_window(w_ctx);
    return ID_WINDOW_DISPLAY_OK;
}

