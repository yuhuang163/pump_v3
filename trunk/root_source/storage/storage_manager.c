/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "storage_manager.h" // IWYU pragma: keep
#include "storage_internal_api.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static storage_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void hdr_storage_work_cb(hdr_post_args_t *args)
{
    if (args->arg1 == 0) {
    system_persist_info_sync_flash(thiz);
    } else if (args->arg1 == 1) {

    }
}
 
static void on_storage_save_receive(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    storage_context_t *st = context;
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    breath_pump_data_body_t *user_data = NULL;
    
    switch (id.code) {
        case ID_BROADCAST_PM_STATE_CHANGE:
            if (arg1 == ID_SYS_PM_SUSPEND) {
                POST_TASK_HANDLER_DELAYED_ARG2(hdr_storage_work_cb, 500, 0, st, "persist_store_task1");
            }
            break;
        case ID_BROADCAST_SYS_SHUTDOWN:
            LOG_STORE("-->save persist_store");
            POST_TASK_HANDLER_DELAYED_ARG2(hdr_storage_work_cb, 0, 0, st, "persist_store_task2");
            break;
        case ID_BROADCAST_SYS_PERSIST_INFO_SAVE:
                PRIV_DATA_TO_USER_DATA_V2(user_data, pdata, breath_pump_data_body_t);
                if (user_data) {
                    LOG_STORE("-->save insert_one");
                    POST_TASK_HANDLER_DELAYED_ARG2(hdr_storage_work_cb, 0, 1, user_data, "persist_store_task3");
                }
            break;
        default:
            break;
    }
}
 
static int on_post_init(void *ptr)
{
    storage_context_t *ctx = ptr;
    /// 加载工厂数据
    init_factory_persist_info(ctx);
    /// 加载系统用户数据 
    init_system_persist_info(ctx);
    /// 加载系统版本信息
    init_system_ver_info(ctx);
    /// 注册存储更新广播.
    ctx->priv.mbdc = u_malloz(sizeof(broadcast_obj_t));
    ctx->priv.mbdc->context = ctx;
    ctx->priv.mbdc->name = "storage_ctx";
    ctx->priv.mbdc->id.code |= ID_BROADCAST_SYS_PERSIST_INFO_SAVE;
    ctx->priv.mbdc->id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    ctx->priv.mbdc->id.code |= ID_BROADCAST_SYS_SHUTDOWN;
    #if IS_SUPPORT_DEEP_SLEEP_FUNCTION
    ctx->priv.mbdc->id.code |= ID_BROADCAST_PM_FREEZE_STATE_CHANGE;
    #endif
    ctx->priv.mbdc->receiver_cb = on_storage_save_receive;
    register_broadcast_receiver(ctx->priv.mbdc);

    return 0;
}

static int on_perpare_init(void *self)
{
    storage_context_t *st = self;
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    st->priv.db_dev = pbus->find_device(ID_DEVICE_DB);

    return 0;
}

static int factory_reset(factory_reset_type_e is_reboot)
{
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    time_system_t *ts = (time_system_t *)find_module(ID_MODULE_TIME);
    LOG_STORE("reset db data.");
    user_database_reset(thiz);
    ///重置系统时间 
    ts->sync_time(PRODUCT_BUILD_TIMESTAMP); 
    if (is_reboot == RESET_NOT_REBOOT) {
        LOG_STORE("reset done,but not reboot.");
        return 1;
    } else {
        wm->set_backlight(0);
        wm->set_window_switch_forbid(1);
        wm->set_is_restting_device(1);   
        msleep(1000);
        __system_reboot(__REBOOT_REASON_FACTORY_RESET);
    }

    return 0;
}

static sys_persist_info_t *get_persist_sys_info(void)
{
    return &thiz->priv.sys_info;
}

static sec_sys_ver_t *get_persist_ver_info(void)
{
    return &thiz->priv.ver_info;
}

static factory_data_store_t *get_factory_data(void)
{
    return &thiz->priv.factory_data;
}


/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
storage_context_t *get_storage_manager_context(void)
{
    return thiz;
}

/// 存储实例注册接口.
void *register_module_for_storage_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, storage_context_t);
    ptr->factory_reset = factory_reset;
    ptr->get_persist_sys_info = get_persist_sys_info;
    ptr->get_persist_ver_info = get_persist_ver_info;
    ptr->put_store_val = put_store_val;
    ptr->get_store_val = get_store_val;
    ptr->del_store_val = del_store_val;
    ptr->get_store_ram_val = get_store_ram_val;
    ptr->put_store_ram_val = put_store_ram_val;
    ptr->get_factory_data = get_factory_data;
    MUTEX_INIT(&ptr->priv.db_rw_lock, db_rw_lock);
    MUTEX_INIT(&ptr->priv.storage_rw_lock, storage_rw_lock);
    MUTEX_INIT(&ptr->priv.breath_pump_data_rw_lock, breath_pump_data_rw_lock);
    thiz = ptr;
    return ADD_MODULE("storage", ID_MODULE_STORAGE, on_perpare_init, 
        on_post_init, ptr, list_head, sizeof(storage_context_t));
}
