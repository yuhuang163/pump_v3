/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "persist_storage.h"
#include "root_common_api.h"
#include "breast_pump_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_PUMP_MODE_NOT_STORE_NORMAL_MODE          1 /// 是否存储常规模式，如果存储常规模式，则存储常规模式
/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static breast_pump_manager_context_t *breast_pump_obj = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int get_breast_pump_mode(void);
static void set_breast_pump_mode(uint32_t mode, uint8_t is_store_mode);
static void set_breast_pump_status(int status);
static int get_breast_pump_status(void);
static pump_result_info_t * get_breast_pump_result(breast_pump_result_id_e result_id);
static const char *get_breast_pump_mode_name(uint8_t mode);

static void on_motor_error_check_callback(uint8_t pump_id, motor_dev_status_e status)
{
    if (status == MOTOR_STATUS_STALL) {
        /// 检查对象和广播系统是否已初始化
        if (breast_pump_obj == NULL) {
            LOGE("breast_pump_obj is NULL, cannot handle motor stall");
            return;
        }
        
        /// 检查广播系统是否已初始化
        if (is_broadcast_inited()) {
            SEND_BROADCAST_ARGS1(ID_BROADCAST_BREAST_PUMP_STATE_CHANGE, BREAST_PUMP_BROADCAST_PUMP_ERROR);
        }
        
        set_breast_pump_status(BREAST_PUMP_PAUSE);
        LOG_PMP("motor error check callback, pump_id: %d, status: %d", pump_id, status);
    }
    else if (status == MOTOR_STATUS_OK) {
        
    }
}

static void on_sensor_data_pump_callback(void *user_data, uint8_t *data, int data_len)
{

}

static void notify_breast_pump_completed(void)
{
    int need_save = 1;
    breast_pump_manager_context_t *ctx = breast_pump_obj;
    algo_manager_context_t *algo = (algo_manager_context_t *)find_module(ID_MODULE_ALGO_HUB);
    /// 外部挂接点.
    struct __breast_pump_hook *hook = breast_pump_obj->hook;
    if (hook && hook->breast_pump_completed_route) {
        need_save = hook->breast_pump_completed_route(breast_pump_obj);
    }

    if (need_save) {
        pump_result_info_t *pumping_info = get_breast_pump_result(BREAST_PUMP_RESULT_PUMPING);
        USER_DATA_TO_PRIV_DATA(pumping_info, pdata);
        SEND_BROADCAST_ARGS4_EXTRA(ID_BROADCAST_PERSIST_INFO_UPDATE, 1, pdata, sizeof(pdata));
        memset(pumping_info, 0, sizeof(pump_result_info_t));
        LOG_PMP("save success, work valid time: %lu", pumping_info->work_valid_time);
    }
}

static int get_breast_pump_status_async(void)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    uint8_t status = m_info->status;
    if (status > BREAST_PUMP_RESET) {
        LOG_PMP("breast pump state: %d", status);
        status = BREAST_PUMP_IDLE;
    }
    return status;
}

static void start_breast_pump_work(uint8_t is_continue_work)
{
    int status = get_breast_pump_status_async();
    breast_pump_manager_context_t *ctx = breast_pump_obj;
    if (ctx->hook && ctx->hook->start_breast_pump_work_route) {
        if (ctx->hook->start_breast_pump_work_route(ctx, status) < 0) {
            return;
        }
    }

    /// 启动马达.
    CHECK_IF_VALID(ctx->priv.motor, {
        ctx->priv.motor->start_motor(is_continue_work ? MOTOR_POLICY_CONTINUE : MOTOR_POLICY_START);
    });

    /// 启动计时器
    STOP_TIMER(ctx->priv.breast_pump_work_timer);
    START_TIMER(ctx->priv.breast_pump_work_timer, PUMP_WORK_STEP_TIME_MS);
    LOG_PMP("start breast pump work, is_continue_work: %d", is_continue_work);
}

static void stop_breast_pump_work(int breast_pump_state)
{
    breast_pump_manager_context_t *ctx = breast_pump_obj;

    /// 关闭马达.
    CHECK_IF_VALID(ctx->priv.motor, {
        ctx->priv.motor->stop_motor();
    });

    /// 停止计时器
    STOP_TIMER(ctx->priv.breast_pump_work_timer);

    if (ctx->hook && ctx->hook->stop_breast_pump_work_route) {
        ctx->hook->stop_breast_pump_work_route(ctx, breast_pump_state);
    }
}

static void breast_pump_work_timer_cb(os_timer_id *timer)
{
    uint8_t msg = 0;

    MSG_QUEUE_SEND(breast_pump_obj->priv.breast_pump_thd.msg_id, &msg, 0);
}

static void reset_breast_pump_result(void)
{
    memset(&breast_pump_obj->priv.m_result, 0, sizeof(breast_pump_result_t));
    LOG_PMP("reset result.");
}

static void get_breast_pump_setting(pump_setting_t *setting, uint8_t step_index)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    if (m_info->mode >= BREAST_PUMP_MODE_MAX) {
        set_breast_pump_mode(BREAST_PUMP_PUMP_MODE_DEFAULT, 1);
        LOG_PMP("invalid mode, set default mode");
    } 

    if (step_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        LOGE("step_index out of range");
        return;
    }
#if IS_PUMP_STEPS_USE_SAME_SETTING
    setting->level[0] = breast_pump_obj->priv.setting[0].level[0];
    setting->level[1] = breast_pump_obj->priv.setting[0].level[1];
    setting->speed = breast_pump_obj->priv.setting[0].speed;
#else
    memcpy(setting, &breast_pump_obj->priv.setting[step_index], sizeof(pump_setting_t));
#endif
}

static void set_breast_pump_setting(pump_setting_t *setting, uint8_t step_index, bool is_need_store)
{
    if (step_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        LOGE("step_index out of range");
        return;
    } 

#if IS_PUMP_STEPS_USE_SAME_SETTING
    for (int i = 0; i < ROOT_BREAST_PUMP_STEP_MAX; i++) {
        breast_pump_obj->priv.setting[i].level[0] = setting->level[0];
        breast_pump_obj->priv.setting[i].level[1] = setting->level[1];
        breast_pump_obj->priv.setting[i].speed = setting->speed;
    }
#else
    memcpy(&breast_pump_obj->priv.setting[step_index], setting, sizeof(pump_setting_t));
#endif
    
    if (is_need_store) {
        struct storage_manager_context *st = breast_pump_obj->priv.st;
        if (st) {
            st->put_store_ram_val(STORE_ID_PUMP_SETTING_INFO, breast_pump_obj->priv.m_info.mode, 
                step_index, setting);
        }
    }

    /// 通知app
    servers_manager_context_t *servers_ctx = (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
    EXCEPTION_CHECK_VOID(servers_ctx);
    if (servers_ctx->hook) {
        servers_publish_data_args_t args = {
            .arg1 = OLD_PUBLISH_DATA_PUMP_LEVEL_CHANGE,
            .arg2 = 0,
            .val = NULL,
        };
        servers_ctx->hook->app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_STATUS, &args);
    }
}

static void set_breast_pump_mode(uint32_t mode, uint8_t is_store_mode)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    struct storage_manager_context *st = breast_pump_obj->priv.st;
    if (mode >= BREAST_PUMP_MODE_MAX) {
        m_info->mode = BREAST_PUMP_PUMP_MODE_DEFAULT;
    } else {
        m_info->mode = mode;
    }
    EXCEPTION_CHECK_VOID(st);
    st->put_store_ram_val(STORE_ID_BREAST_PUMP_MODE, mode, 0, NULL);
    if (PUMP_IS_COMMON_MODE(mode)) {
        m_info->common_mode = mode;
        m_info->last_common_mode = mode;
        st->put_store_ram_val(STORE_ID_BREAST_PUMP_MODE_WITH_TYPE, mode, BREAST_PUMP_MODE_TYPE_COMMON, NULL);
    } else if (PUMP_IS_BREASTFEEDING_MODE(mode)) {
        m_info->breastfeeding_mode = mode;
        m_info->last_breastfeeding_mode = mode;
        st->put_store_ram_val(STORE_ID_BREAST_PUMP_MODE_WITH_TYPE, mode, BREAST_PUMP_MODE_TYPE_BREASTFEEDING, NULL);
    }
    m_info->step_index = 0;
    LOG_PMP("new mode is: %d, name: %s", m_info->mode, get_breast_pump_mode_name(m_info->mode));
    /// 读取存储器中的泵奶设置
    for (uint8_t i = 0; i < ROOT_BREAST_PUMP_STEP_MAX; i++) {
        st->get_store_ram_val(STORE_ID_PUMP_SETTING_INFO, m_info->mode, i, &breast_pump_obj->priv.setting[i]);
    }
    SEND_BROADCAST_ARGS2(ID_BROADCAST_BREAST_PUMP_STATE_CHANGE, BREAST_PUMP_BROADCAST_MODE_CHANGE, m_info->mode);
    /// 通知app
    servers_manager_context_t *servers_ctx = (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
    EXCEPTION_CHECK_VOID(servers_ctx && servers_ctx->hook);
    servers_publish_data_args_t args = {
        .arg1 = OLD_PUBLISH_DATA_PUMP_MODE_CHANGE,
        .arg2 = 0,
        .val = NULL,
    };
    servers_ctx->hook->app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_STATUS, &args);

}

static int get_breast_pump_mode(void)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    if (m_info->mode >= BREAST_PUMP_MODE_MAX) {
        LOG_PMP("invalid, set default mode");
        set_breast_pump_mode(BREAST_PUMP_PUMP_MODE_DEFAULT, 1);
    }
    return m_info->mode;
}

static int get_breast_pump_mode_by_type(breast_pump_mode_type_e type, uint8_t is_last)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    if (type == BREAST_PUMP_MODE_TYPE_BREASTFEEDING) {
        LOG_PMP("get breastfeeding mode, is_last: %d, last_breastfeeding_mode: %d, breastfeeding_mode: %d", 
        is_last, m_info->last_breastfeeding_mode, m_info->breastfeeding_mode);
        return is_last ? m_info->last_breastfeeding_mode : m_info->breastfeeding_mode;
    }
    LOG_PMP("get common mode, is_last: %d, last_common_mode: %d, common_mode: %d", 
        is_last, m_info->last_common_mode, m_info->common_mode);
    return is_last ? m_info->last_common_mode : m_info->common_mode;
}

static int get_breast_pump_status(void)
{
    breast_pump_manager_context_t *ctx = breast_pump_obj;
    MUTEX_LOCK_BHT("get_status", ctx->priv.breast_pump_state_lock);
    int status = get_breast_pump_status_async();
    MUTEX_UNLOCK_BHT("get_status", ctx->priv.breast_pump_state_lock);

    return status;
}

static void set_breast_pump_status(int status)
{
    power_manager_context_t *pm_ctx = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;

    // LOG_PMP("enter work: %d", m_info->do_work);
    MUTEX_LOCK_BHT("set_status", breast_pump_obj->priv.breast_pump_state_lock);
    if (status >= BREAST_PUMP_IDLE && status <= BREAST_PUMP_RESET) {
        if (m_info->status != status) {
            m_info->status = status;
            LOG_PMP("status: %d, do_work: %d, mode: %d", status, m_info->do_work, m_info->mode);
            switch (status) {
                case BREAST_PUMP_IDLE:
                    if (m_info->do_work) {
                        m_info->do_work = 0;
                        stop_breast_pump_work(status);
                        CHECK_IF_VALID(pm_ctx, {
                            pm_ctx->release_wakelock(PUMP_THD_WAKELOCK);
                        });
                    }
                    break;
                case BREAST_PUMP_START:
                    reset_breast_pump_result();
                    m_info->step_index = 0;
                    if (!m_info->do_work) {
                        m_info->do_work = 1;
                        CHECK_IF_VALID(pm_ctx, {
                            pm_ctx->acquire_wakelock(PUMP_THD_WAKELOCK, -1);
                        });
                        start_breast_pump_work(false);
                    }
                    break;
                case BREAST_PUMP_CONTINUE:
                    if (!m_info->do_work) {
                        m_info->do_work = 1;
                        CHECK_IF_VALID(pm_ctx, {
                            pm_ctx->acquire_wakelock(PUMP_THD_WAKELOCK, -1);
                        });
                        start_breast_pump_work(true);
                    }
                    break;
                case BREAST_PUMP_PAUSE:
                    if (m_info->do_work) {
                        m_info->do_work = 0;
                        stop_breast_pump_work(status);
                        CHECK_IF_VALID(pm_ctx, {
                            pm_ctx->release_wakelock(PUMP_THD_WAKELOCK);
                        });
                    }
                    break;
                case BREAST_PUMP_FINISH:
                    if (m_info->do_work) {
                        m_info->do_work = 0;
                        stop_breast_pump_work(status);
                        CHECK_IF_VALID(pm_ctx, {
                            pm_ctx->release_wakelock(PUMP_THD_WAKELOCK);
                        });
                    }
                    m_info->step_index = 0;
                    // LOG_PMP("brush finish, =>save brush result.");
                    notify_breast_pump_completed();
                    break;
                case BREAST_PUMP_FACTORY:
                    notify_breast_pump_completed();
                    if (m_info->do_work) {
                        m_info->do_work = 0;
                        stop_breast_pump_work(status);
                    }
                    break;
                case BREAST_PUMP_RESET: /// 用户app请求重置pump RESET后请将pump状态在设置为IDLE
                    if (m_info->do_work) {
                        m_info->do_work = 0;
                        notify_breast_pump_completed();
                        stop_breast_pump_work(status);
                        CHECK_IF_VALID(pm_ctx, {
                            pm_ctx->release_wakelock(PUMP_THD_WAKELOCK);
                        });
                        breast_pump_obj->hook->breast_pump_result_info_app_reset(breast_pump_obj, &breast_pump_obj->priv.result_info[BREAST_PUMP_RESULT_PUMPING]);
                    }
                    m_info->step_index = 0;
                    break;
            }
            SEND_BROADCAST_ARGS2(ID_BROADCAST_BREAST_PUMP_STATE_CHANGE, BREAST_PUMP_BROADCAST_STATE_CHANGE, status);
        }
    }
    /// 通知app
    servers_manager_context_t *servers_ctx = (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
    EXCEPTION_CHECK_VOID(servers_ctx && servers_ctx->hook);
    servers_publish_data_args_t args = {
        .arg1 = OLD_PUBLISH_DATA_SYSTEM_STATUS_CHANGE,
        .arg2 = 0,
        .val = NULL,
    };
    /// APP请求强制上报系统状态复位
    if (status == BREAST_PUMP_FINISH) {
        args.arg1 = OLD_PUBLISH_DATA_SYSTEM_STATUS_RESET;
        servers_ctx->hook->app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_STATUS, &args);
    }
    args.arg1 = OLD_PUBLISH_DATA_SYSTEM_STATUS_CHANGE;
    servers_ctx->hook->app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_STATUS, &args);
    MUTEX_UNLOCK_BHT("set_status", breast_pump_obj->priv.breast_pump_state_lock);
    // LOG_PMP("exit work: %d", m_info->do_work);
}

static pump_result_info_t * get_breast_pump_result(breast_pump_result_id_e result_id)
{
    breast_pump_manager_context_t *ctx = breast_pump_obj;
    if (result_id >= BREAST_PUMP_RESULT_MAX) {
        return NULL;
    }
    return &ctx->priv.result_info[result_id];
}

static void dispatch_to_client(void)
{
    list_head_t *head = &breast_pump_obj->priv.client_list;
    breast_pump_result_t result = {0};

    MUTEX_LOCK(breast_pump_obj->priv.m_lock);
    memcpy(&result, &breast_pump_obj->priv.m_result, sizeof(breast_pump_result_t));
    LIST_FOR_EACH(pos, (head)) {
        breast_pump_evt_client_node_t *node = CONTAINER_OF(pos, breast_pump_evt_client_node_t, list);
        if (node->cb) {
            node->cb(node->user_data, &result);
        }
    }
    MUTEX_UNLOCK(breast_pump_obj->priv.m_lock);
}

static void *list_del_condition_cb(void *data, list_head_t *pos)
{
    on_breast_pump_result_cb_ptr cb = (on_breast_pump_result_cb_ptr)data;
    breast_pump_evt_client_node_t *node = CONTAINER_OF(pos, breast_pump_evt_client_node_t, list);

    if ((node->cb) == (cb)) {
        return node;
    }
    return NULL;
}

static int breast_pump_add_result_listener(void *user_data, on_breast_pump_result_cb_ptr cb)
{
    list_head_t *head = &breast_pump_obj->priv.client_list;

    MUTEX_LOCK(breast_pump_obj->priv.m_lock);

    LIST_FOR_EACH(pos, (head)) {
        breast_pump_evt_client_node_t *node = CONTAINER_OF(pos, breast_pump_evt_client_node_t, list);
        if (node->cb == cb && node->user_data == user_data) {
            LOG_PMP("ignore duplicate add.");
            MUTEX_UNLOCK(breast_pump_obj->priv.m_lock);
            return -1;
        }
    }

    breast_pump_evt_client_node_t *node = u_malloz(sizeof(breast_pump_evt_client_node_t));
    node->cb = cb;
    node->user_data = user_data;
    __list_add(&node->list, head); 
    MUTEX_UNLOCK(breast_pump_obj->priv.m_lock);

    return 0;
}

static void breast_pump_remove_result_listener(on_breast_pump_result_cb_ptr cb)
{
    list_head_t *head = &breast_pump_obj->priv.client_list;

    MUTEX_LOCK(breast_pump_obj->priv.m_lock);
    __list_del(head, cb, list_del_condition_cb);
    MUTEX_UNLOCK(breast_pump_obj->priv.m_lock);
}


static const char *get_breast_pump_status_name(uint8_t status)
{
    switch (status) {
        default:
            return "@unknow";
        case BREAST_PUMP_IDLE:
            return "#IDLE#";
        case BREAST_PUMP_START:
            return "#START#";
        case BREAST_PUMP_PAUSE:
            return "#PAUSE#";
        case BREAST_PUMP_CONTINUE:
            return "#CONTINUE#";
        case BREAST_PUMP_FINISH:
            return "#FINISH#";
        case BREAST_PUMP_FACTORY:
            return "#FACTORY#";
        case BREAST_PUMP_RESET:
            return "#RESET#";
        case BREAST_PUMP_ERROR:
            return "#ERROR#";
    }
}

static const char *get_breast_pump_mode_name(uint8_t mode)
{
    switch (mode) {
        default:
            return "@unknow";
        case BREAST_PUMP_STIMULATION_MODE:
            return "@stimulation";
        case BREAST_PUMP_LACTATION_MODE:
            return "@pump";
        case BREAST_PUMP_MIX_MODE:
            return "@mix";
        case BREAST_PUMP_CUSTOM_MODE:
            return "@custom";
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            return "@pp_milk_track";
        case BREAST_PUMP_MILK_BOOST_MODE:
            return "@2+8_milk_boost";
        case BREAST_PUMP_LACTATION_START_MODE:
            return "@lactation_start";
        case BREAST_PUMP_REGULAR_PUMPING_MODE:
            return "@regular_pumping";
        case BREAST_PUMP_EMPTYING_BOOST_MODE:
            return "@emptying_boost";
        case BREAST_PUMP_RELAX_MODE:
            return "@relax";
    }
}

static uint8_t get_breast_pump_step_index(void)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    return m_info->step_index;
}

static int get_breast_pump_control_type(void)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    return m_info->control_type;
}

static void set_breast_pump_control_type(uint8_t control_type)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    m_info->control_type = control_type;
    if (breast_pump_obj->priv.st) {
        breast_pump_obj->priv.st->put_store_ram_val(STORE_ID_PUMP_CONTROL_TYPE, control_type, 0, NULL);
    }
    SEND_BROADCAST_ARGS2(ID_BROADCAST_BREAST_PUMP_STATE_CHANGE, BREAST_PUMP_BROADCAST_CONTROL_TYPE_CHANGE, control_type);
    servers_manager_context_t *servers_ctx =
        (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
    EXCEPTION_CHECK_VOID(servers_ctx && servers_ctx->hook);
    servers_publish_data_args_t args = {
        .arg1 = OLD_PUBLISH_DATA_PUMP_CONTROL_TYPE_CHANGE,
        .arg2 = 0,
        .val = NULL,
    };
    servers_ctx->hook->app_old_protocol_server_data_publish(
        APP_OLD_COMM_CID_NOTIFY_STATUS, &args);
    LOG_UI("switch pump control type: %d", control_type);

}

static int get_pump_mode_activate_state(uint8_t mode)
{
    if (mode != BREAST_PUMP_CUSTOM_MODE) {
        return true;
    }
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    return m_info->custom_activate;
}

static void set_pump_mode_activate_state(uint8_t mode, uint8_t status)
{
    if (mode != BREAST_PUMP_CUSTOM_MODE) {
        return;
    }
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    m_info->custom_activate = status > 0 ? true : false;
    struct storage_manager_context *st = breast_pump_obj->priv.st;
    if (st) {
        st->put_store_ram_val(STORE_ID_PUMP_MODE_ACTIVATE_STATUS, mode, m_info->custom_activate, NULL);
    }
}

static int get_pump_mode_steps_update_state(uint8_t mode)
{
    return 0;
}

static void set_pump_mode_steps_update_state(uint8_t mode, uint8_t status)
{

}

static void get_pump_composite_state(pump_state_type_e type, int arg1, int arg2, void *out_data)
{
    int state = 0;
    switch (type) {
        case BREAST_PUMP_MODE_SETTING_INFO: {
            EXCEPTION_CHECK_VOID(out_data);
            pump_setting_t *setting = (pump_setting_t *)out_data;
            get_breast_pump_setting(setting, arg1);
            break;
        }
        case BREAST_PUMP_STEP_INDEX: {
            EXCEPTION_CHECK_VOID(out_data);
            int *step_index = (int *)out_data;
            *step_index  = get_breast_pump_step_index();
            break;
        }
        case BREAST_PUMP_WORK_STATE: {
            EXCEPTION_CHECK_VOID(out_data);
            int *work_state = (int *)out_data;
            *work_state = get_breast_pump_status();
            break;
        }
        case BREAST_PUMP_MODE_ACTIVATE_STATE: {
            EXCEPTION_CHECK_VOID(out_data);
            int *activate_state = (int *)out_data;
            *activate_state = get_pump_mode_activate_state(arg1);
            break;
        }
        case BREAST_PUMP_MODE_STEPS_UPDATE_STATE: {
            EXCEPTION_CHECK_VOID(out_data);
            int *update_state = (int *)out_data;
            *update_state  = get_pump_mode_steps_update_state(arg1);
            break;
        }
        case BREAST_PUMP_LEVEL_CONTROL_TYPE: {
            EXCEPTION_CHECK_VOID(out_data);
            int *control_type = (int *)out_data;
            *control_type  = get_breast_pump_control_type();
            break;
        }
        case BREAST_PUMP_IS_WORKING: {
            EXCEPTION_CHECK_VOID(out_data);
            int *is_working = (int *)out_data;
            breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
            *is_working = m_info->do_work == 1;
            break;
        }
        case BREAST_PUMP_WORK_MODE: {
            EXCEPTION_CHECK_VOID(out_data);
            int *mode_id = (int *)out_data;
            *mode_id = get_breast_pump_mode();
            break;
        }
        case BREAST_PUMP_COMMON_MODE: {
            EXCEPTION_CHECK_VOID(out_data);
            int *mode_id = (int *)out_data;
            *mode_id = get_breast_pump_mode_by_type(BREAST_PUMP_MODE_TYPE_COMMON, arg1);
            break;
        }
        case BREAST_PUMP_EXPERT_MODE: {
            EXCEPTION_CHECK_VOID(out_data);
            int *mode_id = (int *)out_data;
            *mode_id = get_breast_pump_mode_by_type(BREAST_PUMP_MODE_TYPE_BREASTFEEDING, arg1);
            break;
        }
        case BREAST_PUMP_WORK_MODE_NAME: {
            const char **name_str = (const char **)out_data;
            breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
            *name_str = get_breast_pump_mode_name(m_info->mode);
            break;
        }
        case BREAST_PUMP_MODE_NAME: {
            const char **name_str = (const char **)out_data;
            *name_str = get_breast_pump_mode_name(arg1);
            break;
        }
        case BREAST_PUMP_WORK_STATE_NAME: {
            const char **name_str = (const char **)out_data;
            breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
            *name_str = get_breast_pump_status_name(m_info->status);
            break;
        }
        case BREAST_PUMP_RESULT_INFO: {
            const pump_result_info_t **result_info = (const pump_result_info_t **)out_data;
            *result_info = get_breast_pump_result(arg1);
            break;
        }
        default:
            LOG_PMP("unknow type!!!!");
            break;
    }
}

static void set_pump_composite_state(pump_state_type_e type, int arg1, int arg2, void *in_data)
{
    switch (type) {
        case BREAST_PUMP_MODE_SETTING_INFO: 
            set_breast_pump_setting(in_data, arg1, arg2);
            break;
        case BREAST_PUMP_WORK_STATE: 
            set_breast_pump_status(arg1);
            break;
        case BREAST_PUMP_MODE_ACTIVATE_STATE: 
            set_pump_mode_activate_state(arg1, arg2);
            break;
        case BREAST_PUMP_MODE_STEPS_UPDATE_STATE:
            set_pump_mode_steps_update_state(arg1, arg2);
            break;
        case BREAST_PUMP_LEVEL_CONTROL_TYPE:
            set_breast_pump_control_type(arg1);
            break;
        case BREAST_PUMP_WORK_MODE: 
            set_breast_pump_mode(arg1, arg2);
            break;
        default:
            LOG_PMP("unknow type!!!!");
            break;
    }
}

static void make_progress(void)
{
    static uint32_t notify_time = 0;
    int notify_id = -1;
    struct __breast_pump_hook *hook = breast_pump_obj->hook;
    if (hook && hook->make_progress_route) {
        notify_id = hook->make_progress_route(breast_pump_obj, &breast_pump_obj->priv.result_info[BREAST_PUMP_RESULT_PUMPING]);
        if (notify_id == RESULT_PUMP_FINISH) {
            if (get_timestamp_ms(notify_time) > 10000) {
                notify_time = get_timestamp();
                SEND_BROADCAST_ARGS1(ID_BROADCAST_BREAST_PUMP_STATE_CHANGE, BREAST_PUMP_BROADCAST_COMPLETED);
            }
        } else if (notify_id == RESULT_PUMP_WILL_FINISH) {
            if (get_timestamp_ms(notify_time) > 10000) {
                notify_time = get_timestamp();
                SEND_BROADCAST_ARGS1(ID_BROADCAST_BREAST_PUMP_STATE_CHANGE, BREAST_PUMP_BROADCAST_PUMP_WILL_FINISH);
            }
        }
    }
}

static void breast_pump_work_thread(const void *args)
{
    uint8_t msg = 0;
    thread_context_t *thd = (thread_context_t *)args;
    CREATE_MSG_TYPE_QUEUE(&thd->msg_id, BREAST_PUMP_WORK_DISPATCH_MESQ_NUM, 1);
    breast_pump_manager_context_t *ctx = breast_pump_obj;
    CREATE_TIMER(&ctx->priv.breast_pump_work_timer, "breast_pump_work", ctx,
         OS_TIMER_PERIODIC, breast_pump_work_timer_cb);
    do {
        MSG_QUEUE_RECV(thd->msg_id, &msg, OS_WAIT_FOREVER);
        /// 进度单元.
        make_progress();
        /// callback结果给各个客户端,如更新UI，ble上报等.
        dispatch_to_client();

    } while (1);
}

static void on_breast_pump_broadcast_receiver(broadcast_mask_id_t id, void *context, int arg1, 
    int arg2, broadcast_priv_data_t *pdata)
{
    breast_pump_manager_context_t *bst_ctx = context;
    
    switch (id.code) {
        case ID_BROADCAST_PM_STATE_CHANGE:
            if (arg1 == PM_BROADCAST_SYS_SUSPEND) {
                LOG_PMP("PM_SUSPEND, reset breast pump result");
                pump_result_info_t *pumping_info = get_breast_pump_result(BREAST_PUMP_RESULT_PUMPING);
                if (pumping_info) {
                    memset(pumping_info, 0, sizeof(pump_result_info_t));
                }
            }
            break;
        default:
            break;
    }
}

static int on_post_breast_pump_init(void *ptr)
{
    breast_pump_manager_context_t *ctx = ptr;
    memset(&ctx->priv.m_info , 0, sizeof(breast_pump_info_t));
    ctx->priv.m_info.breastfeeding_mode = BREAST_PUMP_CUSTOM_MODE;
    ctx->priv.m_info.common_mode = BREAST_PUMP_STIMULATION_MODE;
    EXCEPTION_CHECK_INT(ctx->priv.motor);
    struct rt_device_node *motor_dev;
    motor_dev = ctx->priv.pbus->find_device(ID_DEVICE_MOTOR);
    /// 检查 motor_dev 是否已初始化，避免空指针访问
    EXCEPTION_CHECK_INT(motor_dev);
    DEVICE_IOCTL_ARGS(motor_dev, OP_CODE_MOTOR_ERROR_CHECK_REGISTER_CALLBACK, on_motor_error_check_callback);
    if (ctx->priv.st) {
        int store_value = BREAST_PUMP_PUMP_MODE_DEFAULT;
        ctx->priv.st->get_store_ram_val(STORE_ID_BREAST_PUMP_MODE, 0, 0, &store_value);
        if (store_value >= BREAST_PUMP_MODE_MAX) {
            store_value = BREAST_PUMP_PUMP_MODE_DEFAULT;      
        }
        set_breast_pump_mode(store_value, 0); /// 设置泵奶模式
        /// 读取存储器中的常规模式
        ctx->priv.st->get_store_ram_val(STORE_ID_BREAST_PUMP_MODE_WITH_TYPE, BREAST_PUMP_MODE_TYPE_COMMON, 0, &store_value);
        if (store_value >= BREAST_PUMP_MODE_MAX) {
            store_value = BREAST_PUMP_MODE_INVALID;
        } else {
            ctx->priv.m_info.common_mode = store_value;
        }
        /// 读取存储器中的追奶模式
        ctx->priv.st->get_store_ram_val(STORE_ID_BREAST_PUMP_MODE_WITH_TYPE, BREAST_PUMP_MODE_TYPE_BREASTFEEDING, 0, &store_value);
        if (store_value >= BREAST_PUMP_MODE_MAX) {
            store_value = BREAST_PUMP_MODE_INVALID;
        } else {
            ctx->priv.m_info.breastfeeding_mode = store_value;
        }
        /// 读取存储器中的控制类型
        ctx->priv.st->get_store_ram_val(STORE_ID_PUMP_CONTROL_TYPE, 0, 0, &store_value);
        if (store_value >= BREAST_PUMP_CONTROL_TYPE_MAX) {
            store_value = BREAST_PUMP_CONTROL_TYPE_LEFT;
        }
        set_breast_pump_control_type(store_value);
        /// 读取存储器中个性化韵律激活状态
        ctx->priv.st->get_store_ram_val(STORE_ID_PUMP_MODE_ACTIVATE_STATUS, BREAST_PUMP_CUSTOM_MODE, 0, &store_value);
        ctx->priv.m_info.custom_activate = store_value > 0 ? true : false;
        LOG_PMP("custom_activate: %d", ctx->priv.m_info.custom_activate);
        if (ctx->priv.m_info.custom_activate == false && ctx->priv.m_info.breastfeeding_mode == BREAST_PUMP_CUSTOM_MODE) {
            ctx->priv.m_info.breastfeeding_mode = BREAST_PUMP_REGULAR_PUMPING_MODE;
            ctx->priv.st->put_store_ram_val(STORE_ID_BREAST_PUMP_MODE_WITH_TYPE, BREAST_PUMP_REGULAR_PUMPING_MODE,
                 BREAST_PUMP_MODE_TYPE_BREASTFEEDING, NULL);
        }
    }

    /// 注册广播监听.
    ctx->priv.m_bdc.context = ctx;
    ctx->priv.m_bdc.name = "breast_pump_manager";
    ctx->priv.m_bdc.id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    ctx->priv.m_bdc.receiver_cb = on_breast_pump_broadcast_receiver;
    register_broadcast_receiver(&ctx->priv.m_bdc);
    RUN_NEW_THREAD(&ctx->priv.breast_pump_thd, "breast_pump_thd", breast_pump_work_thread, 
        RT_PRIO_BREAST_PUMP_DISPATCH_THREAD, BST_THREAD_STACKSIZE);
    return 0;
}

static int on_perpare_breast_pump_init(void *self)
{
    breast_pump_manager_context_t *ctx = (breast_pump_manager_context_t *)self;
    ctx->priv.st = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    ctx->priv.motor = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    ctx->priv.pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);

    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void *register_module_for_breast_pump_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, breast_pump_manager_context_t);
    ptr->add_breast_pump_result_listener = breast_pump_add_result_listener;
    ptr->remove_breast_pump_result_listener = breast_pump_remove_result_listener;
    ptr->get_pump_composite_state = get_pump_composite_state;
    ptr->set_pump_composite_state = set_pump_composite_state;
    MUTEX_INIT(&ptr->priv.m_lock, breast_pump_client_mutex);
    MUTEX_INIT(&ptr->priv.breast_pump_state_lock, brusht_state_mutex);
    MUTEX_INIT(&ptr->priv.breast_pump_mode_lock, breast_pump_mode_lock);
    USE_INIT_LIST_HEAD(&ptr->priv.client_list);
    breast_pump_obj = ptr;

    return ADD_MODULE("brush", ID_MODULE_BREAST_PUMP, on_perpare_breast_pump_init, 
        on_post_breast_pump_init, ptr, list_head, sizeof(breast_pump_manager_context_t));
}
