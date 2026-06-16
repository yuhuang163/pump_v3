/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
#include "windows_manager.h" // IWYU pragma: keep
#include "airplane_mode.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static airplane_mode_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int enter_airplane_mode(int enter_reason)
{   
    ota_manager_context_t *ota_ctx = (ota_manager_context_t *)find_module(ID_MODULE_OTA);
    if (ota_ctx && ota_ctx->is_ota_upgrading()) {
        LOG_AIR("ota upgrading, forbid enter airplane");
        return -1;
    }
    LOG_AIR("enter_reason: %d", enter_reason);
    thiz->priv.enter_state = enter_reason;
    thiz->set_airplane_mode(enter_reason);
    if (thiz->hook && thiz->hook->enter_airplane_mode_route) {
        thiz->hook->enter_airplane_mode_route(thiz, enter_reason);
    }
    /// 触发飞机模式进入埋点 (EVT_009: 飞机模式进入)
    {
        data_bury_trigger_user_evt_t bury_evt = {
            .evt_source = DATA_BURY_EVT_SOURCE_LOCAL,
            .evt_type = DATA_BURY_USER_EVT_TRAVEL_LOCK,
        };
        DATA_BURY_EVENT_TRIGGER_VAL(DATA_BURY_TRIGGER_EVT_PUMP_USER_EVENT, 0, 0, &bury_evt);
    }
    return 0;
}

static int exit_airplane_mode(int exit_reason)
{
    int wait_retry = 0;
    LOG_AIR("exit_reason: %d", exit_reason);
    thiz->priv.is_backgroud_check = false;
    while (1) {
        if (!thiz->priv.wm->is_current_window(thiz->priv.wm, ID_WINDOW_BEGIN) || wait_retry > 60) {
            break;
        }
        wait_retry++;
        LOG_AIR("wait_retry: %d", wait_retry);
        msleep(50);
    }
    thiz->set_airplane_mode(AIRPLANE_MODE_EXIT);
    if (thiz->hook && thiz->hook->exit_airplane_mode_route) {
        thiz->hook->exit_airplane_mode_route(thiz, exit_reason);
    }
    return 0;
}

static int is_on_airplane_mode(void)
{
    return thiz->priv.is_on;
}

static void set_airplane_mode(int state)
{
    thiz->priv.is_on = state > 0 ? true : false;
    LOG_AIR("state: %d", state);
}

static void on_aiplane_mode_receive_internal(broadcast_mask_id_t id, void *context, int arg1, int arg2,
                                            broadcast_priv_data_t *pdata)
{
    airplane_mode_context_t *airplane = (airplane_mode_context_t *)context;
    if (airplane->hook && airplane->hook->on_airplane_mode_receiver_route) {
        airplane->hook->on_airplane_mode_receiver_route(id, context, arg1, arg2, pdata);
    }
    /// 广播监听泵状态改变.
    if (id.code == ID_BROADCAST_BREAST_PUMP_STATE_CHANGE) {
        if (arg1 == BREAST_PUMP_BROADCAST_STATE_CHANGE) {
            if (arg2 == BREAST_PUMP_CONTINUE || arg2 == BREAST_PUMP_START || arg2 == BREAST_PUMP_FACTORY) {
                airplane->priv.is_breast_pump_working = 1;
            } else {
                airplane->priv.is_breast_pump_working = 0;
            }
            LOG_AIR("is_breast_pump_working: %d", airplane->priv.is_breast_pump_working);
        }
    } 
}

static int on_post_airplane_init(void *ptr)
{
    airplane_mode_context_t *airplane = (airplane_mode_context_t *)ptr;
    /// 获取飞机模式状态.
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (st) {
        int store_val = 0;
        st->get_store_ram_val(STORE_ID_SYS_AIRPLANE_MODE_STATE, 0, 0, &store_val);
        airplane->priv.is_on = store_val > 0 ? true : false;
    }
    /// 广播监听泵状态改变.
    airplane->priv.bcd = u_malloz(sizeof(broadcast_obj_t));
    airplane->priv.bcd->context = airplane;
    airplane->priv.bcd->name = "aiplane_mode";
    airplane->priv.bcd->id.code |= ID_BROADCAST_BREAST_PUMP_STATE_CHANGE;
    airplane->priv.bcd->receiver_cb = on_aiplane_mode_receive_internal;
    register_broadcast_receiver(airplane->priv.bcd);
    if (airplane->hook && airplane->hook->on_post_init_route) {
        airplane->hook->on_post_init_route(airplane);
    }
    return 0;
}

static int on_perpare_airplane_init(void *self)
{
    airplane_mode_context_t *airplane = (airplane_mode_context_t *)thiz;
    airplane->priv.wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    airplane->priv.pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    airplane->priv.pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    airplane->priv.misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    airplane->priv.time_sys = (time_system_t *)find_module(ID_MODULE_TIME);
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_airplane_mode_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, airplane_mode_context_t);

    ptr->enter = enter_airplane_mode;
    ptr->exit = exit_airplane_mode;
    ptr->is_on_airplane_mode = is_on_airplane_mode;
    ptr->set_airplane_mode = set_airplane_mode;
    thiz = ptr;

    return ADD_MODULE("ap_mode", ID_MODULE_AIRPLANE_MODE, on_perpare_airplane_init, 
        on_post_airplane_init, ptr, list_head, sizeof(airplane_mode_context_t));
}

