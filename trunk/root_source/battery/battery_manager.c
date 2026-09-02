/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
#include "battery_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static battery_manager_context_t *thiz = NULL;

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
static battery_state_e get_battery_state(int percent, int charge_status)
{
    LOG_MISC("percent: %d, charge_status: %d", percent, charge_status);

    if (percent <= EMPTY_BAT_PERCENTAGE && charge_status != CHARGE_STATE_CHARGING) {
        return BATTERY_STATE_EMPTY;
    }

    if (percent <= LOW_BAT_PERCENTAGE && charge_status != CHARGE_STATE_CHARGING) {
        return BATTERY_STATE_LOW_POWER_SHUTDOWN;
    } 
    
    if (percent <= FORBID_START_BAT_PERCENTAGE && charge_status != CHARGE_STATE_CHARGING) {
        return BATTERY_STATE_FORBID_START;
    } 
    
    if (percent < LOW_POWER_RUN_PERCENTAGE && charge_status != CHARGE_STATE_CHARGING) {
        return BATTERY_STATE_LOW_POWER_RUN;
    } 

    return BATTERY_STATE_NONE;
}

static uint8_t local_is_vbus_plug_in(void)
{
    EXCEPTION_CHECK_UINT(thiz);
    if (thiz->priv.battery == NULL) {
        return 0;
    }
    DEVICE_IOCTL_HAS_RET(thiz->priv.battery, OP_CODE_BAT_VBUS_IS_PLUG_IN, ret);
    return ret > 0 ? 1 : 0;
}

static uint8_t local_get_battery_percent(void)
{
    EXCEPTION_CHECK_UINT(thiz);
    if (thiz->priv.battery == NULL) {
        return 100;
    }
    uint8_t val = 0;
    if (thiz->priv.battery) {   
        val = DEVICE_READ(thiz->priv.battery);
    } else {
        val = 100;
    }
    return (val > 100) ? 100 : val;
}

static uint8_t local_is_low_battery(void)
{
    EXCEPTION_CHECK_UINT(thiz);
    if (thiz->priv.battery_state == BATTERY_STATE_EMPTY ||
        thiz->priv.battery_state == BATTERY_STATE_LOW_POWER_SHUTDOWN) {
        return 1;
    }
    return 0;
}

static uint8_t local_get_charge_status(void)
{
    EXCEPTION_CHECK_UINT(thiz);
    EXCEPTION_CHECK_UINT(thiz->priv.battery);
    DEVICE_IOCTL_HAS_RET(thiz->priv.battery, OP_CODE_BAT_GET_CHARGE_STATUS, ret);
    return ret;
}

static uint8_t local_get_battery_state(void)
{
    EXCEPTION_CHECK_UINT(thiz);
    return thiz->priv.battery_state;
}

static void dispatch_battery_state_client(battery_manager_context_t *obj, battery_state_info_t *info)
{
    list_head_t *head = &obj->priv.client_list;
    MUTEX_LOCK(obj->priv.m_lock);
    LIST_FOR_EACH(pos, (head)) { 
        battery_evt_client_node_t *node = CONTAINER_OF(pos, battery_evt_client_node_t, list);
        if (node->cb) {
            node->cb(node->user_data, info);
        }
    }
    MUTEX_UNLOCK(obj->priv.m_lock);
}

static void add_battery_state_cb(void *user_data, on_battery_state_cb_ptr cb)
{
    battery_manager_context_t *obj = thiz;
    list_head_t *head = &obj->priv.client_list;
    MUTEX_LOCK(obj->priv.m_lock);
    LIST_FOR_EACH(pos, (head)) { 
        battery_evt_client_node_t *node = CONTAINER_OF(pos, battery_evt_client_node_t, list);
        if (node->cb == cb && node->user_data == user_data) {
            LOGW("ignore dumplicate add.");
            MUTEX_UNLOCK(obj->priv.m_lock);
            return;
        }
    }
    battery_evt_client_node_t *node = u_malloz(sizeof(battery_evt_client_node_t));
    node->cb = cb;
    node->user_data = user_data;
    __list_add(&node->list, head);
    LOG_MISC("add: user_data: %lx, cb: %lx", user_data, cb);
    MUTEX_UNLOCK(obj->priv.m_lock);
}

static void *list_del_battery_state_condition_cb(void *data, list_head_t *pos)
{
    on_battery_state_cb_ptr cb = (on_battery_state_cb_ptr)data;
    battery_evt_client_node_t *node = CONTAINER_OF(pos, battery_evt_client_node_t, list);
    if ((node->cb) == (cb)) {
        LOG_MISC("del: user_data: %lx, cb: %lx", node->user_data, node->cb);
        return node;
    }
    return NULL;
}

static void remove_battery_state_cb(on_battery_state_cb_ptr cb)
{
    battery_manager_context_t *obj = thiz;
    list_head_t *head = &obj->priv.client_list;
    MUTEX_LOCK(obj->priv.m_lock);
    __list_del(head, cb, list_del_battery_state_condition_cb);
    MUTEX_UNLOCK(obj->priv.m_lock);
}

static void polling_battery_status(void *user_data)
{
    EXCEPTION_CHECK_VOID(thiz);
    battery_state_info_t info = {0};
    info.battery_percent = local_get_battery_percent();
    if (info.battery_percent != thiz->priv.battery_percent) {
        thiz->priv.battery_percent = info.battery_percent;
        storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
        if (st) {
            st->put_store_ram_val(STORE_ID_SYS_BATTERY_PERCENT, info.battery_percent, 0, NULL);
        }
    }
    info.charge_status = local_get_charge_status();
    info.battery_state = get_battery_state(info.battery_percent, info.charge_status);
    LOG_MISC("#now bat percent:%d, state:%d/%d, charging:%d/%d",
        info.battery_percent, thiz->priv.battery_state, info.battery_state,
        thiz->priv.is_vbus_plug_in, info.charge_status);
    if (thiz->priv.battery_state !=  info.battery_state) {
        thiz->priv.battery_state =  info.battery_state;
    }
    dispatch_battery_state_client(thiz, &info);
}

static void polling_battery_voltage_status(void *user_data)
{
    EXCEPTION_CHECK_VOID(thiz);
    EXCEPTION_CHECK_VOID(thiz->priv.battery);
    DEVICE_IOCTL_NO_RET(thiz->priv.battery, OP_CODE_BAT_POLL_VOLTAGE);
}

static void polling_charge_status(void *user_data)
{
    EXCEPTION_CHECK_VOID(thiz);
    power_manager_context_t *pm = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    EXCEPTION_CHECK_VOID(pm);
    uint8_t plug_status = local_is_vbus_plug_in();
    if (plug_status != thiz->priv.is_vbus_plug_in) {
        LOG_MISC("charge plug status change: %d -> %d", thiz->priv.is_vbus_plug_in, plug_status);
        if (thiz->priv.is_vbus_plug_in) {
            pm->pm_wakeup(PM_WAKEUP_REASON_CHARGE_PLUG_IN);
        } else {
            pm->pm_wakeup(PM_WAKEUP_REASON_CHARGE_PLUG_OUT);
        }
        thiz->priv.is_vbus_plug_in = plug_status;
    }
    uint8_t charge_status = local_get_charge_status();
    if (thiz->priv.charge_status != charge_status) {
        uint8_t prev_status = thiz->priv.charge_status;
        uint8_t prev_charging = BATTERY_IS_CHARGING(prev_status) ? 1u : 0u;
        uint8_t now_charging = BATTERY_IS_CHARGING(charge_status) ? 1u : 0u;

        thiz->priv.charge_status = charge_status;
        LOG_MISC("charge status change:%d", charge_status);
        if (!prev_charging && now_charging) {
            DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_BAT_CHARGE_START, 0, 0);
        } else if (prev_charging && !now_charging) {
            int end_reason;

            if (charge_status == CHARGE_STATE_ERROR) {
                end_reason = DATA_BURY_BAT_CHARGE_END_REASON_ADAPTER_MISMATCH;
            } else if (prev_status == CHARGE_STATE_FULL) {
                end_reason = DATA_BURY_BAT_CHARGE_END_REASON_FULL;
            } else {
                end_reason = DATA_BURY_BAT_CHARGE_END_REASON_UNPLUG;
            }
            DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_BAT_CHARGE_END, end_reason, 0);
        }
        polling_battery_status(user_data);
    }
}

static void charge_event_callback(charge_event_e event)
{
    misc_manager_context_t *misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    EXCEPTION_CHECK_VOID(misc);
    misc->trigger_misc_work(MISC_WORK_MSG_ID_CHARGE_INTERRUPT);
}

static void battery_health_change_callback(uint8_t health)
{
    storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    EXCEPTION_CHECK_VOID(st);
    st->put_store_ram_val(STORE_ID_SYS_BATTERY_HEALTH, health, 0, NULL);
}

static int on_post_battery_init(void *ptr)
{
    battery_manager_context_t *ctx = ptr;
    thiz->priv.battery_state = BATTERY_STATE_NONE;
    thiz->priv.is_vbus_plug_in = local_is_vbus_plug_in();
    thiz->priv.charge_status = local_get_charge_status();
    if (ctx->priv.battery) {
        storage_manager_context_t *st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
        if (st) {
            int health = 0, percent = 0;
            st->get_store_ram_val(STORE_ID_SYS_BATTERY_HEALTH, 0, 0, &health);
            if (health < 90 || health > 100) {
                health = 100;
            }
            st->get_store_ram_val(STORE_ID_SYS_BATTERY_PERCENT, 0, 0, &percent);
            if (percent > 100) {
                percent = 100;
            }
            DEVICE_IOCTL_ARGS_NO_RET(ctx->priv.battery, OP_CODE_BAT_HEALTH_SET, &health);
            DEVICE_IOCTL_ARGS_NO_RET(ctx->priv.battery, OP_CODE_BAT_PERCENT_UPDATE, &percent);
            LOG_MISC("sync battery health: %d, percent: %d", health, percent);
        }
        DEVICE_IOCTL_ARGS_NO_RET(ctx->priv.battery, OP_CODE_BAT_ADD_CHARGE_EVENT_CALLBACK, charge_event_callback);
        DEVICE_IOCTL_ARGS_NO_RET(ctx->priv.battery, OP_CODE_BAT_ADD_HEALTH_CHANGE_CALLBACK, battery_health_change_callback);
        DEVICE_IOCTL_NO_RET(thiz->priv.battery, OP_CODE_BAT_POLL_VOLTAGE);
    }
    misc_manager_context_t *misc = ctx->priv.misc;
    if (misc) {
#if IS_SUPPORT_DEVICE_CHARGE
        misc->add_misc_task(misc, MISC_TASK_ID_CHARGE_POLLING, 1000, 
            MISC_WORK_MSG_ID_TIMER | MISC_WORK_MSG_ID_CHARGE_INTERRUPT, polling_charge_status);
#endif
        misc->add_misc_task(misc, MISC_TASK_ID_BATTERY_POLLING, 1500, 
            MISC_WORK_MSG_ID_TIMER, polling_battery_status);
        misc->add_misc_task(misc, MISC_TASK_ID_BATTERY_VOLTAGE_POLLING, 0, 
            MISC_WORK_MSG_ID_TIMER, polling_battery_voltage_status);
    }
    return 0;
}

static int on_perpare_battery_init(void *self)
{
    battery_manager_context_t *ptr = self;
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    ptr->priv.misc = (misc_manager_context_t *)find_module(ID_MODULE_MISC);
    ptr->priv.battery = pbus->find_device(ID_DEVICE_BATTERY);
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_battery_manager_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, battery_manager_context_t);
    MUTEX_INIT(&ptr->priv.m_lock, battery_state_mutex);
    USE_INIT_LIST_HEAD(&ptr->priv.client_list);
    ptr->priv.battery_percent = 5;
    ptr->is_vbus_plug_in = local_is_vbus_plug_in;
    ptr->is_low_battery = local_is_low_battery;
    ptr->get_charge_status = local_get_charge_status;
    ptr->get_battery_state = local_get_battery_state;
    ptr->get_battery_percent = local_get_battery_percent;
    ptr->add_battery_state_cb = add_battery_state_cb;
    ptr->remove_battery_state_cb = remove_battery_state_cb;
    thiz = ptr;
    return ADD_MODULE("battery", ID_MODULE_BATTERY, on_perpare_battery_init, on_post_battery_init, 
        ptr, list_head, sizeof(battery_manager_context_t));
}
