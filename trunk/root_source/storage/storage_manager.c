/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "storage_manager.h" // IWYU pragma: keep
#include "persist_data_internal.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static storage_manager_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int put_store_val(persist_store_id_e id, int arg1, int arg2, const void *val_in)
{
    int ret = 0;
    store_func_args_t args = {
        .ctx = thiz,
        .arg1 = arg1,
        .arg2 = arg2,
        .val = val_in,
    };
    EXCEPTION_CHECK_INT(thiz);
    const put_store_process_handle_t *list = thiz->priv.put_store_process_list;
    uint16_t list_len = thiz->priv.put_store_process_list_len;
    EXCEPTION_CHECK_INT(list);
    MUTEX_LOCK(thiz->priv.storage_rw_lock);
    for (int i = 0; i < list_len; i++) {
        if (list[i].store_id == id) {
            ret = list[i].store_process(&args);
            break;
        }
    }
    MUTEX_UNLOCK(thiz->priv.storage_rw_lock);

    return ret;
}

static int get_store_val(persist_store_id_e id, int arg1, int arg2,  void *val_out)
{
    int ret = 0;
    store_func_args_t args = {
        .ctx = thiz,
        .arg1 = arg1,
        .arg2 = arg2,
        .val = val_out,
    };
    EXCEPTION_CHECK_INT(thiz);
    const get_store_process_handle_t *list = thiz->priv.get_store_process_list;
    uint16_t list_len = thiz->priv.get_store_process_list_len;
    EXCEPTION_CHECK_INT(list);
    MUTEX_LOCK(thiz->priv.storage_rw_lock);
    for (int i = 0; i < list_len; i++) {
        if (list[i].store_id == id) {
            ret = list[i].store_process(&args);
            break;
        }
    }
    MUTEX_UNLOCK(thiz->priv.storage_rw_lock);
    return ret;
}

static int del_store_val(persist_store_id_e id, int arg1, int arg2,  const void *val_in)
{
    int ret = 0;
    store_func_args_t args = {
        .ctx = thiz,
        .arg1 = arg1,
        .arg2 = arg2,
        .val = val_in,
    };
    EXCEPTION_CHECK_INT(thiz);
    MUTEX_LOCK(thiz->priv.storage_rw_lock);
    const del_store_process_handle_t *list = thiz->priv.del_store_process_list;
    uint16_t list_len = thiz->priv.del_store_process_list_len;
    EXCEPTION_CHECK_INT(list);
    for (int i = 0; i < list_len; i++) {
        if (list[i].store_id == id) {
            ret = list[i].store_process(&args);
            break;
        }
    }
    MUTEX_UNLOCK(thiz->priv.storage_rw_lock);

    return ret;
}       

static int get_store_ram_val(persist_store_id_e id, int arg1, int arg2, void *val_out)
{
    int ret = 0;
    store_func_args_t args = {
        .ctx = thiz,
        .arg1 = arg1,
        .arg2 = arg2,
        .val = val_out,
    };
    EXCEPTION_CHECK_INT(thiz);
    const get_store_ram_val_handle_t *list = thiz->priv.get_store_ram_val_list;
    uint16_t list_len = thiz->priv.get_store_ram_val_list_len;
    EXCEPTION_CHECK_INT(list);
    MUTEX_LOCK(thiz->priv.storage_rw_lock);
    for (int i = 0; i < list_len; i++) {
        if (list[i].store_id == id) {
            ret = list[i].store_process(&args);
            break;
        }
    }
    MUTEX_UNLOCK(thiz->priv.storage_rw_lock);

    return ret;
}

static void put_store_ram_val(persist_store_id_e id, int arg1, int arg2, const void *val_in)
{
    EXCEPTION_CHECK_VOID(thiz);
    store_func_args_t args = {
        .ctx = thiz,
        .arg1 = arg1,
        .arg2 = arg2,
        .val = val_in,
    };
    const put_store_ram_val_handle_t *list = thiz->priv.put_store_ram_val_list;
    uint16_t list_len = thiz->priv.put_store_ram_val_list_len;
    EXCEPTION_CHECK_VOID(list);
    MUTEX_LOCK(thiz->priv.storage_rw_lock);
    for (int i = 0; i < list_len; i++) {
        if (list[i].store_id == id) {
            list[i].store_process(&args);
            break;
        }
    }
    MUTEX_UNLOCK(thiz->priv.storage_rw_lock);
}

static void on_storage_save_receive(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    on_storage_broadcast_route(id, context, arg1, arg2, pdata);
}
 
static int on_post_storage_init(void *ptr)
{
    storage_manager_context_t *ctx = ptr;
    /// 初始化持久化存储
    on_post_persist_storage_init(ctx);
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

static int on_perpare_storage_init(void *self)
{
    storage_manager_context_t *st = self;
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    st->priv.db_dev = pbus->find_device(ID_DEVICE_DB);
    on_perpare_persist_storage_init(st);
    return 0;
}

static int factory_reset(factory_reset_type_e is_reboot)
{
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    time_system_t *ts = (time_system_t *)find_module(ID_MODULE_TIME);
    LOG_STORE("reset db data.");
    thiz->user_database_reset();
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
int user_database_put(storage_manager_context_t *ctx, database_key_t key, const uint8_t *buf, int buf_len)
{
    int ret = -1;
    rt_device_node_t *db_dev = ctx->priv.db_dev;
    MUTEX_LOCK(ctx->priv.db_rw_lock);
    if (db_dev && db_dev->write) {
        uint32_t buf_wr[] = { (uint32_t)key, (uint32_t)buf, (uint32_t)buf_len };
        ret = DEVICE_WRITE(db_dev, buf_wr, sizeof(buf_wr));
        if (ret == 0) {
            uint8_t *tmp_buf = u_malloz(buf_len);
            uint32_t buf_rd[] = { (uint32_t)key, (uint32_t)tmp_buf, (uint32_t)buf_len };
            DEVICE_READ_ARGS(db_dev, buf_rd, sizeof(buf_rd));
            if (memcmp(tmp_buf, buf, buf_len) != 0) {
                LOGE("storage key 0x%x fail, retry write.", key);
                ret = DEVICE_WRITE(db_dev, buf_rd, sizeof(buf_rd));
            }
            u_free(tmp_buf);
        } else {
            LOGE("write key failure: 0x%x, buf_len: %d, ret: %d", key, buf_len, ret);
        }
    }
    MUTEX_UNLOCK(ctx->priv.db_rw_lock);

    return ret;
}

int user_database_get(storage_manager_context_t *ctx, database_key_t key, uint8_t *buf, int buf_len)
{
    int ret = -1;
    rt_device_node_t *db_dev = ctx->priv.db_dev;
    MUTEX_LOCK(ctx->priv.db_rw_lock);
    if (db_dev && db_dev->read) {
        uint32_t buf_rd[] = { (uint32_t)key, (uint32_t)buf, (uint32_t)buf_len };
        ret = db_dev->read(db_dev, buf_rd, sizeof(buf_rd));
    }
    MUTEX_UNLOCK(ctx->priv.db_rw_lock);
    
    return ret;
}

int user_database_remove(storage_manager_context_t *ctx, database_key_t key)
{
    int ret = -1;
    rt_device_node_t *db_dev = ctx->priv.db_dev;

    MUTEX_LOCK(ctx->priv.db_rw_lock);
    if (db_dev && db_dev->ioctl) {
        ret = DEVICE_IOCTL_ARGS(db_dev, OP_CODE_DB_REMOTE, &key);
    }
    MUTEX_UNLOCK(ctx->priv.db_rw_lock);
    
    return ret;
}

/// 存储实例注册接口.
void *register_module_for_storage_new(list_head_t *list_head)
{
    MALLOC_FOR_PTR(ptr, storage_manager_context_t);
    ptr->factory_reset = factory_reset;

    ptr->put_store_val = put_store_val;
    ptr->get_store_val = get_store_val;
    ptr->del_store_val = del_store_val;
    ptr->get_store_ram_val = get_store_ram_val;
    ptr->put_store_ram_val = put_store_ram_val;
    ptr->get_persist_sys_info = get_persist_sys_info;
    ptr->get_persist_ver_info = get_persist_ver_info;
    ptr->get_factory_data = get_factory_data;
    register_persist_storage_ctx(ptr);
 
    MUTEX_INIT(&ptr->priv.db_rw_lock, db_rw_lock);
    MUTEX_INIT(&ptr->priv.storage_rw_lock, storage_rw_lock);
    MUTEX_INIT(&ptr->priv.breath_pump_data_rw_lock, breath_pump_data_rw_lock);
    thiz = ptr;
    return ADD_MODULE("storage", ID_MODULE_STORAGE, on_perpare_storage_init, 
        on_post_storage_init, ptr, list_head, sizeof(storage_manager_context_t));
}
