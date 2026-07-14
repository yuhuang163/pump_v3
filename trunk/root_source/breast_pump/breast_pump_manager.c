/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h"
#include "breast_pump_mode.h" // IWYU pragma: keep
#include "breast_pump_manager.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/
#define IS_PUMP_MODE_NOT_STORE_NORMAL_MODE          1 /// 是否存储常规模式，如果存储常规模式，则存储常规模式
#define PUMP_NOTIFY_THROTTLE_MS                     5000
#define PUMP_WILL_FINISH_TIP_THROTTLE_MS            10000
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
static void set_breast_pump_mode(uint32_t mode, uint8_t is_store_mode, uint8_t is_report);
static void set_breast_pump_status(int status, int is_report);
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
        
        set_breast_pump_status(BREAST_PUMP_PAUSE, 1);
        LOG_PMP("motor error check callback, pump_id: %d, status: %d", pump_id, status);
    } else if (status == MOTOR_STATUS_OK) {
        
    }
}

static void notify_breast_pump_completed(void)
{
    int need_save = 1;
    breast_pump_manager_context_t *ctx = breast_pump_obj;
    algo_manager_context_t *algo = (algo_manager_context_t *)find_module(ID_MODULE_ALGO_HUB);
    struct __breast_pump_hook *hook = breast_pump_obj->hook;
    if (hook && hook->breast_pump_completed_route) {
        need_save = hook->breast_pump_completed_route(breast_pump_obj);
    }

    if (need_save) {
        pump_result_info_t *pumping_info = get_breast_pump_result(BREAST_PUMP_RESULT_HISTORY);
        USER_DATA_TO_PRIV_DATA(pumping_info, pdata);
        SEND_BROADCAST_ARGS4_EXTRA(ID_BROADCAST_PERSIST_INFO_UPDATE, 1, pdata, sizeof(pdata));
        LOG_PMP("save success, work total time: %lu", pumping_info->work_valid_time);
    }
    /// 重置常规模式
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    if (PUMP_IS_COMMON_MODE(m_info->mode)) {
        m_info->mode = BREAST_PUMP_STIMULATION_MODE;
    }
    m_info->common_mode = BREAST_PUMP_STIMULATION_MODE;
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
    breast_pump_manager_context_t *ctx = breast_pump_obj;
    if (ctx->hook && ctx->hook->start_breast_pump_work_route) {
        ctx->hook->start_breast_pump_work_route(ctx, 
            is_continue_work ? BREAST_PUMP_CONTINUE : BREAST_PUMP_START);
    }

    /// 启动马达.
    CHECK_IF_VALID(ctx->priv.motor, {
        ctx->priv.motor->start_motor(is_continue_work ? MOTOR_POLICY_CONTINUE : MOTOR_POLICY_START);
    });

    STOP_TIMER(ctx->priv.breast_pump_work_timer);
    START_TIMER(ctx->priv.breast_pump_work_timer, PUMP_WORK_STEP_TIME_MS);
    LOG_PMP("is_continue_work: %d", is_continue_work);
}

static void stop_breast_pump_work(int breast_pump_state)
{
    breast_pump_manager_context_t *ctx = breast_pump_obj;
    LOG_PMP("stop breast pump work, state: %d", breast_pump_state);
    /// 关闭马达.
    CHECK_IF_VALID(ctx->priv.motor, {
        ctx->priv.motor->stop_motor(breast_pump_state == BREAST_PUMP_PAUSE ? MOTOR_POLICY_PAUSE : MOTOR_POLICY_FINISH);
    });

    STOP_TIMER(ctx->priv.breast_pump_work_timer);
    START_TIMER(ctx->priv.breast_pump_work_timer, PUMP_IDLE_STEP_TIME_MS);
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
    memset(&breast_pump_obj->priv.result_info[BREAST_PUMP_RESULT_PUMPING], 0, sizeof(pump_result_info_t));
    LOG_PMP("reset result.");
}

static void get_breast_pump_setting(pump_setting_t *setting, uint8_t step_index)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    if (m_info->mode >= BREAST_PUMP_MODE_MAX) {
        set_breast_pump_mode(BREAST_PUMP_PUMP_MODE_DEFAULT, 1, 1);
        LOG_PMP("invalid mode, set default mode");
    } 

    if (step_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        LOGE("step_index out of range");
        return;
    }
    memcpy(setting, &breast_pump_obj->priv.setting[step_index], sizeof(pump_setting_t));

}

static void set_breast_pump_setting(pump_setting_t *setting, uint8_t step_index, bool is_need_store)
{
    if (step_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        LOGE("step_index out of range");
        return;
    } 
#if IS_SUPPORT_PUMP_GLOBAL_PARAM_SETTING
    int mode = BREAST_PUMP_ALL_MODE_AND_PROGRAM;
#else
    int mode = breast_pump_obj->priv.m_info.mode;
#endif
    memcpy(&breast_pump_obj->priv.m_result.setting, setting, sizeof(pump_setting_t));
#if IS_PUMP_STEPS_USE_SAME_SETTING
    for (int i = 0; i < ROOT_BREAST_PUMP_STEP_MAX; i++) {
        breast_pump_obj->priv.setting[i].level[0] = setting->level[0];
        breast_pump_obj->priv.setting[i].level[1] = setting->level[1];
        breast_pump_obj->priv.setting[i].speed = setting->speed;
        if (is_need_store && breast_pump_obj->priv.st) {
            breast_pump_obj->priv.st->put_store_ram_val(STORE_ID_PUMP_SETTING_INFO,
                mode, i, &breast_pump_obj->priv.setting[i]);
        }
    }
#else
    memcpy(&breast_pump_obj->priv.setting[step_index], setting, sizeof(pump_setting_t));
    if (is_need_store && breast_pump_obj->priv.st) {
        breast_pump_obj->priv.st->put_store_ram_val(STORE_ID_PUMP_SETTING_INFO,
            mode, step_index, setting);
    }
#endif
}

static void set_breast_pump_mode(uint32_t mode, uint8_t is_store_mode, uint8_t is_report)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    struct storage_manager_context *st = breast_pump_obj->priv.st;
    motor_manager_context_t *motor_ctx = breast_pump_obj->priv.motor;

    if (mode >= BREAST_PUMP_MODE_MAX) {
        mode = BREAST_PUMP_PUMP_MODE_DEFAULT;
    } 

    m_info->mode = mode;
    if (motor_ctx) {
        motor_ctx->change_motor_mode(m_info->mode, m_info->do_work);
    }
    
    if (breast_pump_obj->hook && breast_pump_obj->hook->mode_change_route) {
        breast_pump_obj->hook->mode_change_route(breast_pump_obj, m_info->mode);
    }

    if (st) {
        st->put_store_ram_val(STORE_ID_BREAST_PUMP_MODE, mode, 0, NULL); /// 更新存储器中的泵奶模式
    }
    
    for (uint8_t i = 0; i < ROOT_BREAST_PUMP_STEP_MAX; i++) {
        if (st) {
            st->get_store_ram_val(STORE_ID_PUMP_SETTING_INFO, m_info->mode, i, &breast_pump_obj->priv.setting[i]);
        }
    }

    if (is_report) {
        breast_pump_obj->priv.m_result.setting_mode = m_info->mode;
    }
    LOG_PMP("new mode is: %d, name: %s, store:%d, report:%d", m_info->mode, 
            get_breast_pump_mode_name(m_info->mode), is_store_mode, is_report);
}

static int get_breast_pump_mode(void)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    if (m_info->mode >= BREAST_PUMP_MODE_MAX) {
        LOG_PMP("invalid, set default mode");
        set_breast_pump_mode(BREAST_PUMP_PUMP_MODE_DEFAULT, 0, 0);
    }
    return m_info->mode;
}

static int get_breast_pump_mode_by_type(breast_pump_mode_type_e type, uint8_t is_last)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    /// 追奶模式
    if (type == BREAST_PUMP_MODE_TYPE_BREASTFEEDING) {
        if (is_last == 0 && PUMP_IS_BREASTFEEDING_MODE(m_info->mode)) {
            return m_info->mode;
        }
        return m_info->breastfeeding_mode;
    }
    
    /// 常规模式
    if (is_last == 0 && PUMP_IS_COMMON_MODE(m_info->mode)) {
        return m_info->mode;
    }

    return m_info->common_mode;
}

static int get_breast_pump_status(void)
{
    breast_pump_manager_context_t *ctx = breast_pump_obj;
    MUTEX_LOCK_BHT("get_status", ctx->priv.breast_pump_state_lock);
    int status = get_breast_pump_status_async();
    MUTEX_UNLOCK_BHT("get_status", ctx->priv.breast_pump_state_lock);

    return status;
}

static void set_breast_pump_status(int status, int is_report)
{
    power_manager_context_t *pm_ctx = (power_manager_context_t *)find_module(ID_MODULE_POWER);
    motor_manager_context_t *motor_ctx = (motor_manager_context_t *)find_module(ID_MODULE_MOTOR);
    EXCEPTION_CHECK_VOID(motor_ctx);
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
                    // LOG_PMP("brush finish, =>save brush result.");
                    notify_breast_pump_completed();
                    WRITE_PUMP_IS_START_FROM_APP(breast_pump_obj, false);
                    break;
                case BREAST_PUMP_FACTORY:
                    if (!m_info->do_work) {
                        m_info->do_work = 1;
                        CHECK_IF_VALID(pm_ctx, {
                            pm_ctx->acquire_wakelock(PUMP_THD_WAKELOCK, -1);
                        });
                        start_breast_pump_work(false);
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
                    }
                    break;
            }
            SEND_BROADCAST_ARGS2(ID_BROADCAST_BREAST_PUMP_STATE_CHANGE, BREAST_PUMP_BROADCAST_STATE_CHANGE, status);
        }
    }
    
    if (is_report) {
        breast_pump_obj->priv.m_result.status = status;
    }
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

static void dispatch_to_client(breast_pump_manager_context_t *ctx)
{
    list_head_t *head = &ctx->priv.client_list;
    breast_pump_result_t result = {0};
    MUTEX_LOCK(ctx->priv.m_lock);
    memcpy(&result, &ctx->priv.m_result, sizeof(breast_pump_result_t));
    LIST_FOR_EACH(pos, (head)) {
        breast_pump_evt_client_node_t *node = CONTAINER_OF(pos, breast_pump_evt_client_node_t, list);
        if (node->cb) {
            node->cb(node->user_data, &result);
        }
    }
    MUTEX_UNLOCK(ctx->priv.m_lock);
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
    motor_manager_context_t *motor = breast_pump_obj->priv.motor;
    if (motor) {
        m_info->step_index = motor->get_runtime_policy_index(MOTOR_TIME_POLICY);
    }
    return m_info->step_index;
}

static uint32_t get_breast_pump_step_remaining_time(void)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    motor_manager_context_t *motor = breast_pump_obj->priv.motor;
    if (motor) {
        m_info->step_remaining_time = motor->get_time_policy_remaining_time();
    }
    return m_info->step_remaining_time;
}

static int get_breast_pump_control_type(void)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    return m_info->control_type;
}

static void set_breast_pump_control_type(uint8_t control_type, uint8_t is_report)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    breast_pump_result_t *result = &breast_pump_obj->priv.m_result;
    m_info->control_type = control_type;
    if (breast_pump_obj->priv.st) {
        breast_pump_obj->priv.st->put_store_ram_val(STORE_ID_PUMP_CONTROL_TYPE, control_type, 0, NULL);
    }
    if (is_report) {
        result->control_type = control_type;
    }
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

static void set_pump_status_start_from_app(uint8_t status)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    m_info->is_start_from_app = status > 0 ? true : false;
}

static uint8_t get_pump_status_start_from_app(void)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    return m_info->is_start_from_app;
}

static uint8_t get_pump_control_source(void)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    return m_info->control_source;
}

static void set_pump_control_source(uint8_t source)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    m_info->control_source = source;
}

static int get_pump_mode_steps_update_state(uint8_t mode)
{
    return 0;
}

static void set_pump_mode_steps_update_state(uint8_t mode, uint8_t status)
{

}

static void get_pump_composite_state(pump_state_type_e type, int arg1, int arg2, void *out_data, int data_size)
{
    int state = 0;
    switch (type) {
        case BREAST_PUMP_MODE_SETTING_INFO: {
            EXCEPTION_CHECK_VOID(out_data);
            if (data_size != sizeof(pump_setting_t)) {
                LOG_PMP("data_size error, ignore.");
                return;
            }
            pump_setting_t *setting = (pump_setting_t *)out_data;
            get_breast_pump_setting(setting, arg1);
            break;
        }
        case BREAST_PUMP_STEP_INDEX: {
            EXCEPTION_CHECK_VOID(out_data);
            if (data_size != sizeof(int)) {
                LOG_PMP("data_size error, ignore.");
                return;
            }
            int *step_index = (int *)out_data;
            *step_index  = get_breast_pump_step_index();
            break;
        }
        case BREAST_PUMP_WORK_STATE: {
            EXCEPTION_CHECK_VOID(out_data);
            if (data_size != sizeof(int)) {
                LOG_PMP("data_size error, ignore.");
                return;
            }
            int *work_state = (int *)out_data;
            *work_state = get_breast_pump_status();
            break;
        }
        case BREAST_PUMP_MODE_ACTIVATE_STATE: {
            EXCEPTION_CHECK_VOID(out_data);
            if (data_size != sizeof(int)) {
                LOG_PMP("data_size error, ignore.");
                return;
            }
            int *activate_state = (int *)out_data;
            *activate_state = get_pump_mode_activate_state(arg1);
            break;
        }
        case BREAST_PUMP_MODE_STEPS_UPDATE_STATE: {
            EXCEPTION_CHECK_VOID(out_data);
            if (data_size != sizeof(int)) {
                LOG_PMP("data_size error, ignore.");
                return;
            }
            int *update_state = (int *)out_data;
            *update_state  = get_pump_mode_steps_update_state(arg1);
            break;
        }
        case BREAST_PUMP_LEVEL_CONTROL_TYPE: {
            EXCEPTION_CHECK_VOID(out_data);
            if (data_size != sizeof(int)) {
                LOG_PMP("data_size error, ignore.");
                return;
            }
            int *control_type = (int *)out_data;
            *control_type  = get_breast_pump_control_type();
            break;
        }
        case BREAST_PUMP_IS_WORKING: {
            EXCEPTION_CHECK_VOID(out_data);
            if (data_size != sizeof(int)) {
                LOG_PMP("data_size error, ignore.");
                return;
            }
            int *is_working = (int *)out_data;
            breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
            *is_working = m_info->do_work == 1;
            break;
        }
        case BREAST_PUMP_WORK_MODE: {
            EXCEPTION_CHECK_VOID(out_data);
            if (data_size != sizeof(int)) {
                LOG_PMP("data_size error, ignore.");
                return;
            }
            int *mode_id = (int *)out_data;
            *mode_id = get_breast_pump_mode();
            break;
        }
        case BREAST_PUMP_COMMON_MODE: {
            EXCEPTION_CHECK_VOID(out_data);
            if (data_size != sizeof(int)) {
                LOG_PMP("data_size error, ignore.");
                return;
            }
            int *mode_id = (int *)out_data;
            *mode_id = get_breast_pump_mode_by_type(BREAST_PUMP_MODE_TYPE_COMMON, arg1);
            break;
        }
        case BREAST_PUMP_EXPERT_MODE: {
            EXCEPTION_CHECK_VOID(out_data);
            if (data_size != sizeof(int)) {
                LOG_PMP("data_size error, ignore.");
                return;
            }
            int *mode_id = (int *)out_data;
            *mode_id = get_breast_pump_mode_by_type(BREAST_PUMP_MODE_TYPE_BREASTFEEDING, arg1);
            break;
        }
        case BREAST_PUMP_WORK_MODE_NAME: {
            EXCEPTION_CHECK_VOID(out_data);
            if (data_size != sizeof(char *)) {  
                LOG_PMP("data_size error, ignore.");
                return;
            }
            const char **name_str = (const char **)out_data;
            breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
            *name_str = get_breast_pump_mode_name(m_info->mode);
            break;
        }
        case BREAST_PUMP_MODE_NAME: {
            EXCEPTION_CHECK_VOID(out_data);
            if (data_size != sizeof(char *)) {
                LOG_PMP("data_size error, ignore.");
                return;
            }
            const char **name_str = (const char **)out_data;
            *name_str = get_breast_pump_mode_name(arg1);
            break;
        }
        case BREAST_PUMP_WORK_STATE_NAME: {
            EXCEPTION_CHECK_VOID(out_data);
            if (data_size != sizeof(char *)) {
                LOG_PMP("data_size error, ignore.");
                return;
            }
            const char **name_str = (const char **)out_data;
            breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
            *name_str = get_breast_pump_status_name(m_info->status);
            break;
        }
        case BREAST_PUMP_RESULT_INFO: {
            EXCEPTION_CHECK_VOID(out_data);
            if (data_size != sizeof(pump_result_info_t *)) {
                LOG_PMP("data_size error, ignore.");
                return;
            }
            const pump_result_info_t **result_info = (const pump_result_info_t **)out_data;
            *result_info = get_breast_pump_result(arg1);
            break;
        }
        case BREAST_PUMP_IS_START_FROM_APP: {
            EXCEPTION_CHECK_VOID(out_data);
            if (data_size != sizeof(int)) {
                LOG_PMP("data_size error, ignore.");
                return;
            }
            int *is_start_from_app = (int *)out_data;
            *is_start_from_app = get_pump_status_start_from_app();
            break;
        }
        case BREAST_PUMP_CONTROL_SOURCE: {
            EXCEPTION_CHECK_VOID(out_data);
            if (data_size != sizeof(int)) {
                LOG_PMP("data_size error, ignore.");
                return;
            }
            int *control_source = (int *)out_data;
            *control_source = get_pump_control_source();
            break;
        }
        default:
            LOG_PMP("unknow type!!!!");
            break;
    }
}

static void set_pump_composite_state(pump_state_type_e type, int arg1, int arg2, int arg3, void *in_data)
{
    switch (type) {
        case BREAST_PUMP_MODE_SETTING_INFO: 
            set_breast_pump_setting(in_data, arg1, arg2);
            break;
        case BREAST_PUMP_WORK_STATE: 
            set_breast_pump_status(arg1, arg2);
            break;
        case BREAST_PUMP_MODE_ACTIVATE_STATE: 
            set_pump_mode_activate_state(arg1, arg2);
            break;
        case BREAST_PUMP_MODE_STEPS_UPDATE_STATE:
            set_pump_mode_steps_update_state(arg1, arg2);
            break;
        case BREAST_PUMP_LEVEL_CONTROL_TYPE:
            set_breast_pump_control_type(arg1, arg2);
            break;
        case BREAST_PUMP_WORK_MODE: 
            set_breast_pump_mode(arg1, arg2, arg3);
            break;
        case BREAST_PUMP_IS_START_FROM_APP:
            set_pump_status_start_from_app(arg1);
            break;
        case BREAST_PUMP_CONTROL_SOURCE:
            set_pump_control_source(arg1);
            break;
        default:
            LOG_PMP("unknow type!!!!");
            break;
    }
}

static bool send_throttled_pump_broadcast(breast_pump_manager_context_t *ctx,
    uint32_t *notify_time, uint32_t throttle_ms, breast_pump_broadcast_id_e broadcast_id)
{
    if (get_timestamp_ms(*notify_time) <= throttle_ms) {
        return false;
    }
    *notify_time = get_timestamp();
    SEND_BROADCAST_ARGS1(ID_BROADCAST_BREAST_PUMP_STATE_CHANGE, broadcast_id);
    return true;
}

static void make_progress(breast_pump_manager_context_t *ctx)
{
    struct __breast_pump_hook *hook = ctx->hook;
    if (!hook || !hook->make_progress_route) {
        return;
    }

    int notify_id = hook->make_progress_route(ctx, &ctx->priv.result_info[BREAST_PUMP_RESULT_PUMPING]);

    switch (notify_id) {
    case RESULT_PUMP_FINISH:
        WRITE_PUMP_WORK_STATUS(ctx, BREAST_PUMP_FINISH, 1);
        send_throttled_pump_broadcast(ctx, &ctx->priv.completed_notify_time,
            PUMP_NOTIFY_THROTTLE_MS, BREAST_PUMP_BROADCAST_COMPLETED);
        DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_PUMP_SESSION_END, DATA_BURY_END_REASON_AUTO_COMPLETE, 0);
        break;
    case RESULT_PUMP_TIMEOUT:
        WRITE_PUMP_WORK_STATUS(ctx, BREAST_PUMP_FINISH, 1);
        send_throttled_pump_broadcast(ctx, &ctx->priv.completed_notify_time,
            PUMP_NOTIFY_THROTTLE_MS, BREAST_PUMP_BROADCAST_TIMEOUT);
        DATA_BURY_EVENT_TRIGGER(DATA_BURY_TRIGGER_EVT_PUMP_SESSION_END, DATA_BURY_END_REASON_AUTO_COMPLETE, 0);
        break;
    case RESULT_PUMP_WILL_FINISH:
        send_throttled_pump_broadcast(ctx, &ctx->priv.will_finish_notify_time,
            PUMP_NOTIFY_THROTTLE_MS, BREAST_PUMP_BROADCAST_PUMP_WILL_FINISH);
        break;
    case RESULT_PUMP_AUTO_PAUSE:
        WRITE_PUMP_WORK_STATUS(ctx, BREAST_PUMP_PAUSE, 1);
        SEND_BROADCAST_ARGS1(ID_BROADCAST_BREAST_PUMP_STATE_CHANGE, BREAST_PUMP_BROADCAST_AUTO_PAUSE_REQUEST);
        break;
    case RESULT_PUMP_RUNNING_30M_TIP:
        SEND_BROADCAST_ARGS1(ID_BROADCAST_BREAST_PUMP_STATE_CHANGE, BREAST_PUMP_BROADCAST_RUNNING_30M_TIP);
        break;
    case RESULT_PUMP_WILL_FINISH_TIP:
        if (send_throttled_pump_broadcast(ctx, &ctx->priv.will_finish_tip_notify_time,
                PUMP_WILL_FINISH_TIP_THROTTLE_MS, BREAST_PUMP_BROADCAST_PUMP_WILL_FINISH_TIP)) {
            LOG_PMP("will finish tip, notify.");
        }
        break;
    default:
        break;
    }
}

static void breast_pump_work_thread(const void *args)
{
    uint8_t msg = 0;
    thread_context_t *thd = (thread_context_t *)args;
    CREATE_MSG_TYPE_QUEUE(&thd->msg_id, BREAST_PUMP_WORK_DISPATCH_MESQ_NUM, 1);
    breast_pump_manager_context_t *ctx = breast_pump_obj;
    breast_pump_info_t *m_info = &ctx->priv.m_info;
    CREATE_TIMER(&ctx->priv.breast_pump_work_timer, "breast_pump_work", ctx,
         OS_TIMER_PERIODIC, breast_pump_work_timer_cb);
    START_TIMER(ctx->priv.breast_pump_work_timer, PUMP_IDLE_STEP_TIME_MS);
    do {
        MSG_QUEUE_RECV(thd->msg_id, &msg, OS_WAIT_FOREVER);
        if (m_info->do_work == 1) {
            make_progress(ctx);  /// 进度单元.
        }
        dispatch_to_client(ctx); /// callback结果给各个客户端,如更新UI，ble上报等.
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
        case ID_BROADCAST_SYS_STATE_CHANGE:    
            if (arg1 == SYSTEM_BROADCAST_UTC_TIME_CHANGE) {
                if (abs(arg2) > 60) {
                    LOG_PMP("reset pump history result finish timestamp");
                    pump_result_info_t *history_info = get_breast_pump_result(BREAST_PUMP_RESULT_HISTORY);
                    if (history_info) {
                        history_info->finish_timestamp = 0;
                    }
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
    EXCEPTION_CHECK_INT(ctx);
    memset(&ctx->priv.m_info , 0, sizeof(breast_pump_info_t));
    struct rt_device_node *motor_dev = ctx->priv.pbus->find_device(ID_DEVICE_MOTOR);
    if (motor_dev) {
        DEVICE_IOCTL_ARGS(motor_dev, OP_CODE_MOTOR_ADD_STALLED_CALLBACK, on_motor_error_check_callback);
    }
    breast_pump_info_t *m_info = &ctx->priv.m_info;
    if (ctx->priv.st) {
        int store_value = BREAST_PUMP_PUMP_MODE_DEFAULT;
        /// 读取模式/韵律
        ctx->priv.st->get_store_ram_val(STORE_ID_BREAST_PUMP_MODE, 0, 0, &store_value);
        if (store_value >= BREAST_PUMP_MODE_MAX) {
            store_value = BREAST_PUMP_PUMP_MODE_DEFAULT;   
        } else {
            if (PUMP_IS_COMMON_MODE(store_value)) {
                store_value = BREAST_PUMP_PUMP_MODE_DEFAULT;
            }
        }
        m_info->mode = store_value;
        /// 读取存储器中的泵奶设置
        for (uint8_t i = 0; i < ROOT_BREAST_PUMP_STEP_MAX; i++) {
            ctx->priv.st->get_store_ram_val(STORE_ID_PUMP_SETTING_INFO, m_info->mode, i, &breast_pump_obj->priv.setting[i]);
        }

        #if 0
        /// 读取存储器中的常规模式
        ctx->priv.st->get_store_ram_val(STORE_ID_BREAST_PUMP_MODE_WITH_TYPE, BREAST_PUMP_MODE_TYPE_COMMON, 0, &store_value);
        if (store_value > BREAST_PUMP_NORMAL_MODE_UPPER) {
            m_info->common_mode = BREAST_PUMP_NORMAL_MODE_DEFAULT;
        } else {
            // m_info->common_mode = store_value;
            m_info->common_mode = BREAST_PUMP_NORMAL_MODE_DEFAULT;
        }
        #else
        m_info->common_mode = BREAST_PUMP_NORMAL_MODE_DEFAULT;
        #endif

        /// 读取存储器中的追奶模式
        ctx->priv.st->get_store_ram_val(STORE_ID_BREAST_PUMP_MODE_WITH_TYPE, BREAST_PUMP_MODE_TYPE_BREASTFEEDING, 0, &store_value);
        if (store_value > BREAST_PUMP_EXPERT_MODE_UPPER) {
            m_info->breastfeeding_mode = BREAST_PUMP_MODE_INVALID;
        } else {
            m_info->breastfeeding_mode = store_value;
        }
        /// 读取存储器中的控制类型
        ctx->priv.st->get_store_ram_val(STORE_ID_PUMP_CONTROL_TYPE, 0, 0, &store_value);
        if (store_value >= BREAST_PUMP_CONTROL_TYPE_MAX) {
            store_value = BREAST_PUMP_CONTROL_TYPE_BILATERAL;
        }
        m_info->control_type = store_value;
        /// 读取存储器中个性化韵律激活状态
        ctx->priv.st->get_store_ram_val(STORE_ID_PUMP_MODE_ACTIVATE_STATUS, BREAST_PUMP_CUSTOM_MODE, 0, &store_value);
        m_info->custom_activate = store_value > 0 ? true : false;
        LOG_PMP("custom_activate: %d, mode:%d, common:%d, expert:%d", m_info->custom_activate, m_info->mode,
            m_info->common_mode, m_info->breastfeeding_mode);
        if (m_info->custom_activate == false && m_info->breastfeeding_mode == BREAST_PUMP_CUSTOM_MODE) {
            m_info->breastfeeding_mode = BREAST_PUMP_MODE_INVALID;
            ctx->priv.st->put_store_ram_val(STORE_ID_BREAST_PUMP_MODE_WITH_TYPE, BREAST_PUMP_MODE_TYPE_BREASTFEEDING, 
                BREAST_PUMP_REGULAR_PUMPING_MODE, NULL);
        }
        /// 同步上一次的泵奶记录
        time_system_t *time_sys = (time_system_t *)find_module(ID_MODULE_TIME);
        if (time_sys) {
            if (time_sys->get_last_time_sync_utc() > 0) {
                breast_pump_data_head_t pump_data_head = {0};
                ctx->priv.st->get_store_ram_val(STORE_ID_BREAST_PUMP_DATA_HEAD, 0, 0, &pump_data_head);
                pump_result_info_t *result_info = &ctx->priv.result_info[BREAST_PUMP_RESULT_HISTORY];
                if (pump_data_head.num > 0) {
                    ctx->priv.st->get_store_val(STORE_ID_BREAST_PUMP_DATA_BODY, 
                        pump_data_head.tail, 0, result_info);
                    LOG_PMP("sync last pump record: num:%d, head:%d, tail:%d, timestamp:%d, mode:%d, work time:%d", 
                        pump_data_head.num, pump_data_head.head, pump_data_head.tail,
                        result_info->finish_timestamp, result_info->mode, result_info->work_valid_time);
                } else {
                    LOGE("num:%d, head:%d, tail:%d", pump_data_head.num, pump_data_head.head, pump_data_head.tail);
                }
            } 
        } 
    } else {
        ctx->priv.m_info.breastfeeding_mode = BREAST_PUMP_MODE_INVALID;
        ctx->priv.m_info.common_mode = BREAST_PUMP_MODE_INVALID;
    }
    /// 注册广播监听.
    ctx->priv.m_bdc.context = ctx;
    ctx->priv.m_bdc.name = "breast_pump_manager";
    ctx->priv.m_bdc.id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    ctx->priv.m_bdc.id.code |= ID_BROADCAST_SYS_STATE_CHANGE;
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
    ctx->priv.battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
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
