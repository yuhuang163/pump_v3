/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "windows_manager.h" 
#include "windows_internal_api.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define PAGE_STATIC_NAME                dummy
#define ENTRY_AP_MODE_TIMES             (10)

typedef struct {
    window_context_t args;
    broadcast_obj_t receiver;
    misc_manager_context_t *misc;
    storage_context_t *st;
    airplane_mode_context_t *ap;
    resource_manager_context_t *res;
} ui_proc_dummy_t;

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
DECLARE_UI_PAGE_OBJ(PAGE_STATIC_NAME);
static ui_page_handle_t *p_window_handle = &PAGE_CONTEXT_NAME(PAGE_STATIC_NAME);
INIT_WINDOW_CONTEXT();

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void draw(ui_page_obj_t *return_menu, void *argv)
{
    LOG_UI("dummy page");
}

static void timer_cb(void *args)
{
    LOG_UI("dummy page");
}

ui_page_handle_t PAGE_CONTEXT_NAME(PAGE_STATIC_NAME) = {
    .draw = draw,
    .timer_handler = timer_cb,
};

static int wakup_enter_new_window_policy(ui_proc_dummy_t *proc, int wakeup_reason)
{
    window_manager_context_t *wm = proc->args.wm;
    airplane_mode_context_t *airplane = (airplane_mode_context_t *)find_module(ID_MODULE_AP_MODE);
    power_manager_context_t *pm =  (power_manager_context_t *)find_module(ID_MODULE_POWER);
    storage_context_t *st = (storage_context_t *)find_module(ID_MODULE_STORAGE);
    multi_func_msg_data_t msg = {0};
    
    if (!(pm && pm->is_background_wakeup())) {
        if (airplane && airplane->is_on_airplane_mode()) {
            LOG_UI("dispatch airplane mode exit ui");
            if (wakeup_reason == PM_WAKEUP_REASON_CHARGE_PLUG_IN) {
                proc->ap->exit(wakeup_reason);
            } else {
                LOG_UI("dispatch airplane mode");
                msg.union_body.extra.window_id = ID_WINDOW_AIRPLANE_MODE;
                wm->dispatch_new_window_args(wm, &msg);
            }
            return 0;
        }
    }

    if (!st->get_store_ram_val(STORE_ID_FACTORY_DEPLOY_FLAG)) {
        LOG_UI("goto fac: true");
        msg.union_body.extra.window_id = ID_WINDOW_FACTORY_MODE;
        wm->dispatch_new_window_args(wm, &msg);
        return 0;
    }

    if (proc->misc) {
        if (proc->misc->show_battery_ui_if_need(wm, false)) {
            LOG_UI("battery,show_ui_if_need: true");
            return 0;
        }
    }

    if (!is_window_deployed()) {
        LOG_UI("goto wizard: true");
        msg.union_body.extra.window_id = ID_WINDOW_GUIDE;
        msg.union_body.extra.is_background = 1;
        wm->dispatch_new_window_args(wm, &msg);
        return 0;
    }

    if(wakeup_reason == PM_WAKEUP_REASON_IMU_IRQ) {
        wm->priv.wake_up_without_operate ++;

        if (wm->priv.wake_up_without_operate >= ENTRY_AP_MODE_TIMES) {
            airplane_mode_context_t *ap = (airplane_mode_context_t *)find_module(ID_MODULE_AP_MODE);
            LOG_UI("goto ap mode: true");
            wm->priv.wake_up_without_operate = 0;
            ap->enter();
            return 0;
        }
    } else {
        wm->priv.wake_up_without_operate = 0;
    }

    return -1;
}

static void on_dummy_receive_internal(broadcast_mask_id_t id, void *context,
                                 int pm_state, pm_wakeup_reason_e wakeup_reason, broadcast_priv_data_t *pdata)
{
    ui_proc_dummy_t *proc = context;
    window_manager_context_t *wm = proc->args.wm;
    multi_func_msg_data_t msg = {0};

    if (pm_state != ID_SYS_PM_RESUME) {
        LOG_UI("ignore: %d\n", wakeup_reason);
        return;
    }

    if (proc == NULL) {
        LOG_UI("dummy is destory");
        return;
    }

    LOG_UI("#PM_RESUME@wakeup_reason: %d\n", wakeup_reason);
    if (wakup_enter_new_window_policy(proc, wakeup_reason) == 0) {
        return;
    }

    msg.union_body.extra.pm_wakeup_reason = wakeup_reason;
    msg.union_body.extra.window_id = ID_WINDOW_BREATH_PUMP;
    msg.union_body.extra.is_background = 1;
    wm->dispatch_new_window_args(wm, &msg);
}

static void on_dummy_window_receive(broadcast_mask_id_t id, void *context,
                                 int32_t pm_state, int32_t wakeup_reason, broadcast_priv_data_t *pdata)
{
    window_dispatch_lock(__func__);

    IS_WINDOW_CONTEXT_PRESENT({
        switch (id.code) {
            case ID_BROADCAST_PM_STATE_CHANGE: {
                LOG_UI("received PM_STATE_CHANGE.");
                on_dummy_receive_internal(id, context, pm_state, (pm_wakeup_reason_e)wakeup_reason, pdata);
            }
            break;
            case ID_BROADCAST_BREATH_PUMP_MODE_CHANGE: {
                LOG_UI("received BREATH_PUMP_MODE_CHANGE.");
                break;
            }
            default:
            break;
        }
    }, LOG_UI);

    window_dispatch_unlock(__func__);
}


static int exit_event_cb(window_lift_cycle_e new_state, 
    const struct window_context *thiz, const struct window_context *new_scr)
{
    ui_proc_dummy_t *proc = CONTAINER_OF(thiz, ui_proc_dummy_t, args);

    switch (new_state) {
        case ID_SCR_STATE_ON_PAUSE:
            LOG_UI("ON_PAUSE\n");
            unregister_broadcast_receiver(&proc->receiver);
        break;
        case ID_SCR_STATE_ON_DESTORY:
            window_dispatch_lock("dummy-exit");
            LOG_UI("ON_DESTORY--begin\n");
            u_free(proc);
            SET_WINDOW_CONTEXT(NULL, LOG_UI);
            LOG_UI("ON_DESTORY--end\n");
            window_dispatch_unlock("dummy-exit");
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
    LOG_UI("set backlight off");
    wm->set_backlight(0);
    ui_proc_dummy_t *proc = u_malloz(sizeof(ui_proc_dummy_t));
    proc->misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    proc->st = (storage_context_t *)find_module(ID_MODULE_STORAGE);
    proc->ap = (airplane_mode_context_t *)find_module(ID_MODULE_AP_MODE);
    proc->res = (resource_manager_context_t *)find_module(ID_MODULE_RES);
    SET_WINDOW_CONTEXT(proc, LOG_UI);

    // 监听PM状态改变.
    LOG_UI("register receiver\n");
    proc->receiver.id.code = ID_BROADCAST_PM_STATE_CHANGE;
    proc->receiver.context = proc;
    proc->receiver.name = "dummy_screen";
    proc->receiver.receiver_cb = on_dummy_window_receive;
    register_broadcast_receiver(&proc->receiver);

    window_context_t *args = &proc->args;
    args->wm = wm;
    args->new_window_id = window_id;
    args->evt_cb_exit = exit_event_cb;
    args->not_push_history = 1;
    args->user_data = NULL;
    dispatch_new_window(args);

    return ID_WINDOW_DISPLAY_OK;
}



