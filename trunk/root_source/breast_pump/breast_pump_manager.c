/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "persist_storage.h"
#include "root_common.h"
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
        LOG_ALGO("save history");
    }

    if (need_save) {
        pump_result_info_t *pumping_info = breast_pump_obj->get_breast_pump_result(BREAST_PUMP_RESULT_PUMPING);
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

static int breast_pump_is_working(void)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    return m_info->do_work == 1;
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
        breast_pump_obj->set_breast_pump_mode(BREAST_PUMP_NORMAL_MODE_DEFAULT, 1);
        LOG_PMP("invalid mode, set default mode");
    } 

    if (step_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        memcpy(setting, breast_pump_obj->priv.setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
    } else {
        memcpy(setting, &breast_pump_obj->priv.setting[step_index], sizeof(pump_setting_t));
    }
}

static void set_breast_pump_setting(pump_setting_t *setting, uint8_t step_index)
{
    if (step_index >= ROOT_BREAST_PUMP_STEP_MAX) {
        LOGE("step_index >= ROOT_BREAST_PUMP_STEP_MAX");
        memcpy(breast_pump_obj->priv.setting, setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
    } else {
        #if IS_PUMP_STEPS_USE_SAME_SETTING
        for (int i = 0; i < ROOT_BREAST_PUMP_STEP_MAX; i++) {
            breast_pump_obj->priv.setting[i].level[0] = setting->level[0];
            breast_pump_obj->priv.setting[i].level[1] = setting->level[1];
        }
        breast_pump_obj->priv.setting[step_index].speed = setting->speed;
        #else
        memcpy(&breast_pump_obj->priv.setting[step_index], setting, sizeof(pump_setting_t));
        #endif
    }
    //通知app
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
        m_info->mode = BREAST_PUMP_NORMAL_MODE_DEFAULT;
    } else {
        m_info->mode = mode;
    }
    m_info->step_index = 0;
    LOG_PMP("new mode is: %d, name: %s", m_info->mode, breast_pump_obj->get_breast_pump_mode_name(m_info->mode));
    EXCEPTION_CHECK_VOID(st);
    if (is_store_mode) {
        st->put_store_ram_val(STORE_ID_BREAST_PUMP_MODE, 0, 0, &m_info->mode);
    }
    store_pump_setting_value_t store_pump_setting_value = {0};
    store_pump_setting_value.is_custom_trial_mode = m_info->is_custom_trial_mode;
    store_pump_setting_value.setting = breast_pump_obj->priv.setting;
    st->get_store_ram_val(STORE_ID_PUMP_SETTING_INFO, m_info->mode, ROOT_BREAST_PUMP_STEP_MAX, &store_pump_setting_value);
    for (int i = 0; i < ROOT_BREAST_PUMP_STEP_MAX; i++) {
        LOG_PMP("setting[%d]: speed: %d, level: %d, %d", i, breast_pump_obj->priv.setting[i].speed, 
            breast_pump_obj->priv.setting[i].level[0], breast_pump_obj->priv.setting[i].level[1]);
    }
    SEND_BROADCAST_ARGS2(ID_BROADCAST_BREAST_PUMP_STATE_CHANGE, BREAST_PUMP_BROADCAST_MODE_CHANGE, m_info->mode);
    //通知app
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
        set_breast_pump_mode(BREAST_PUMP_NORMAL_MODE_DEFAULT, 1);
    }
    return m_info->mode;
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

    LOG_PMP("enter work: %d", m_info->do_work);
    MUTEX_LOCK_BHT("set_status", breast_pump_obj->priv.breast_pump_state_lock);
    if (status >= BREAST_PUMP_IDLE && status <= BREAST_PUMP_RESET) {
        if (m_info->status != status) {
            m_info->status = status;
            LOG_PMP("status: %d, do_work: %d", status, m_info->do_work);

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
                    breast_pump_obj->reset_breast_pump_result();
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
                    LOG_PMP("brush finish, =>save brush result.");
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
    //通知app
    servers_manager_context_t *servers_ctx = (servers_manager_context_t *)find_module(ID_MODULE_SERVERS);
    EXCEPTION_CHECK_VOID(servers_ctx && servers_ctx->hook);
    servers_publish_data_args_t args = {
        .arg1 = OLD_PUBLISH_DATA_SYSTEM_STATUS_CHANGE,
        .arg2 = 0,
        .val = NULL,
    };
    servers_ctx->hook->app_old_protocol_server_data_publish(APP_OLD_COMM_CID_NOTIFY_STATUS, &args);

    MUTEX_UNLOCK_BHT("set_status", breast_pump_obj->priv.breast_pump_state_lock);
    LOG_PMP("exit work: %d", m_info->do_work);
}

static const breast_pump_result_t *get_curr_breast_pump_result(void)
{
    breast_pump_manager_context_t *ctx = breast_pump_obj;
    return (const breast_pump_result_t *)&ctx->priv.m_result;
}

static int get_last_breast_pump_result(breast_pump_result_t *result, uint8_t breast_pump_data_id)
{
    return 0;
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

static uint8_t get_is_custom_trial_mode(void)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    return m_info->is_custom_trial_mode;
}

static void set_is_custom_trial_mode(uint8_t is_custom_trial_mode)
{
    breast_pump_info_t *m_info = &breast_pump_obj->priv.m_info;
    m_info->is_custom_trial_mode = is_custom_trial_mode;
}

static void make_progress(void)
{
    static uint32_t notify_time = 0;
    int notify_id = -1;
    struct __breast_pump_hook *hook = breast_pump_obj->hook;
    if (hook && hook->make_progress_route) {
        notify_id = hook->make_progress_route(breast_pump_obj, &breast_pump_obj->priv.result_info[BREAST_PUMP_RESULT_PUMPING]);
        if (notify_id == RESULT_PUMP_FINISH && get_timestamp_ms(notify_time) > 2000) {
            notify_time = get_timestamp();
            SEND_BROADCAST_ARGS1(ID_BROADCAST_BREAST_PUMP_STATE_CHANGE, BREAST_PUMP_BROADCAST_COMPLETED);
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
                pump_result_info_t *pumping_info = breast_pump_obj->get_breast_pump_result(BREAST_PUMP_RESULT_PUMPING);
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
    ctx->priv.m_info.mode = BREAST_PUMP_MODE_INVALID;  
    ctx->priv.m_info.is_custom_trial_mode = 0; //重启默认试用模式为关闭
    /// 获取控制类型.
    if (ctx->priv.st) {
        uint8_t store_mode = 0;
        int ret = ctx->priv.st->get_store_ram_val(STORE_ID_BREAST_PUMP_MODE, 0, 0, &store_mode);
        if (ret == 0 && store_mode < BREAST_PUMP_MODE_MAX) {
            #if IS_PUMP_MODE_NOT_STORE_NORMAL_MODE
            if(store_mode == BREAST_PUMP_MIX_MODE || store_mode == BREAST_PUMP_LACTATION_MODE) {
                store_mode = BREAST_PUMP_NORMAL_MODE_DEFAULT;
            }
            #endif
            ctx->priv.m_info.mode = store_mode;
        } else {
            /// 读取失败或值无效，使用默认模式
            ctx->priv.m_info.mode = BREAST_PUMP_NORMAL_MODE_DEFAULT;
        }
        /// 获取泵奶设置.
        store_pump_setting_value_t store_pump_setting_value = {0};
        store_pump_setting_value.is_custom_trial_mode = ctx->priv.m_info.is_custom_trial_mode;
        store_pump_setting_value.setting = ctx->priv.setting;
        ctx->priv.st->get_store_ram_val(STORE_ID_PUMP_SETTING_INFO, ctx->priv.m_info.mode, ROOT_BREAST_PUMP_STEP_MAX, &store_pump_setting_value);
        LOG_PMP("get pump mode: %d", ctx->priv.m_info.mode);
        for (int i = 0; i < ROOT_BREAST_PUMP_STEP_MAX; i++) {
            LOG_PMP("setting[%d]: speed: %d, level: %d, %d", i, ctx->priv.setting[i].speed, 
                ctx->priv.setting[i].level[0], ctx->priv.setting[i].level[1]);
        }
    }
    /// 获取控制类型.
    if (ctx->priv.st) {
        int store_value = 0;
        ctx->priv.st->get_store_ram_val(STORE_ID_PUMP_CONTROL_TYPE, 0, 0, &store_value);
        ctx->priv.m_info.control_type = store_value;
        LOG_PMP("get pump control type: %d", ctx->priv.m_info.control_type);
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

    ptr->is_on_working = breast_pump_is_working;
    ptr->reset_breast_pump_result = reset_breast_pump_result;
    ptr->add_breast_pump_result_listener = breast_pump_add_result_listener;
    ptr->remove_breast_pump_result_listener = breast_pump_remove_result_listener;
    ptr->get_breast_pump_mode = get_breast_pump_mode;
    ptr->set_breast_pump_mode = set_breast_pump_mode;
    ptr->get_breast_pump_control_type = get_breast_pump_control_type;
    ptr->set_breast_pump_control_type = set_breast_pump_control_type;
    ptr->get_breast_pump_setting = get_breast_pump_setting;
    ptr->set_breast_pump_setting = set_breast_pump_setting;
    ptr->get_breast_pump_status_async = get_breast_pump_status_async;
    ptr->get_breast_pump_status = get_breast_pump_status;
    ptr->set_breast_pump_status = set_breast_pump_status;
    ptr->get_breast_pump_step_index = get_breast_pump_step_index;
    ptr->get_last_breast_pump_result = get_last_breast_pump_result;
    ptr->get_curr_breast_pump_result = get_curr_breast_pump_result;
    ptr->get_breast_pump_result = get_breast_pump_result;
    ptr->get_breast_pump_status_name = get_breast_pump_status_name;
    ptr->get_breast_pump_mode_name = get_breast_pump_mode_name;
    ptr->get_is_custom_trial_mode = get_is_custom_trial_mode;
    ptr->set_is_custom_trial_mode = set_is_custom_trial_mode;
    MUTEX_INIT(&ptr->priv.m_lock, breast_pump_client_mutex);
    MUTEX_INIT(&ptr->priv.breast_pump_state_lock, brusht_state_mutex);
    MUTEX_INIT(&ptr->priv.breast_pump_mode_lock, breast_pump_mode_lock);
    USE_INIT_LIST_HEAD(&ptr->priv.client_list);
    breast_pump_obj = ptr;

    return ADD_MODULE("brush", ID_MODULE_BREAST_PUMP, on_perpare_breast_pump_init, 
        on_post_breast_pump_init, ptr, list_head, sizeof(breast_pump_manager_context_t));
}
