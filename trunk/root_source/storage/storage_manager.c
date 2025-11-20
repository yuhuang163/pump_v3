/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "storage_manager.h" // IWYU pragma: keep

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
static int put_store_val(persist_store_id_e id, int arg1, int arg2, void *val_in)
{
    int ret = -2;
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
            ret = list[i].store_process(args);
            break;
        }
    }
    MUTEX_UNLOCK(thiz->priv.storage_rw_lock);
    if (ret != 0) {
        LOGE("==> put store val failed: %d", id);
    }
    return ret;
}

static int get_store_val(persist_store_id_e id, int arg1, int arg2,  void *val_out)
{
    int ret = -2;
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
            ret = list[i].store_process(args);
            break;
        }
    }
    MUTEX_UNLOCK(thiz->priv.storage_rw_lock);
    if (ret != 0) {
        LOGE("==> get store val failed: %d", id);
    }
    return ret;
}

static int del_store_val(persist_store_id_e id, int arg1, int arg2,  void *val_in)
{
    int ret = -2;
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
            ret = list[i].store_process(args);
            break;
        }
    }
    MUTEX_UNLOCK(thiz->priv.storage_rw_lock);
    if (ret != 0) {
        LOGE("==> del store val failed: %d", id);
    }

    return ret;
}       

static int get_store_ram_val(persist_store_id_e id, int arg1, int arg2, void *val_out)
{
    int ret = -2;
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
            ret = list[i].store_process(args);
            break;
        }
    }
    MUTEX_UNLOCK(thiz->priv.storage_rw_lock);
    if (ret != 0) {
        LOGE("==> get store ram val failed: %d", id);
    }
    return ret;
}

static void put_store_ram_val(persist_store_id_e id, int arg1, int arg2, void *val_in)
{
    int ret = -2;
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
            ret = list[i].store_process(args);
            break;
        }
    }
    MUTEX_UNLOCK(thiz->priv.storage_rw_lock);
    if (ret != 0) {
        LOGE("==> put store ram val failed: %d", id);
    }
}

static void on_storage_broadcast_receive(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    if (thiz->hook && thiz->hook->on_storage_broadcast_route) {
        thiz->hook->on_storage_broadcast_route(id, context, arg1, arg2, pdata);
    }
}
 
static int on_post_storage_init(void *ptr)
{
    storage_manager_context_t *ctx = ptr;
    /// 注册存储更新广播.
    ctx->priv.mbdc = u_malloz(sizeof(broadcast_obj_t));
    ctx->priv.mbdc->context = ctx;
    ctx->priv.mbdc->name = "storage_ctx";
    ctx->priv.mbdc->id.code |= ID_BROADCAST_SYS_PERSIST_INFO_SAVE;
    ctx->priv.mbdc->id.code |= ID_BROADCAST_PM_STATE_CHANGE;
    ctx->priv.mbdc->id.code |= ID_BROADCAST_SYS_POWER_STATE_CHANGE;
    ctx->priv.mbdc->receiver_cb = on_storage_broadcast_receive;
    register_broadcast_receiver(ctx->priv.mbdc);
    /// 钩子参数初始化
    if (ctx->hook && ctx->hook->on_post_init_route) {
        ctx->hook->on_post_init_route(ctx);
    }
    return 0;
}

static int on_perpare_storage_init(void *self)
{
    storage_manager_context_t *st = self;
    peripheral_bus_context_t *pbus = (peripheral_bus_context_t *)find_module(ID_MODULE_DEVICE_BUS);
    st->priv.db_dev = pbus->find_device(ID_DEVICE_DB);
    if (st->hook && st->hook->on_perpare_init_route) {
        st->hook->on_perpare_init_route(st);
    }
    return 0;
}

static int factory_reset(factory_reset_type_e is_reboot)
{
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    ui_manager_context_t *ui = (ui_manager_context_t *)find_module(ID_MODULE_UI);
    time_system_t *ts = (time_system_t *)find_module(ID_MODULE_TIME);
    LOG_STORE("reset db data.");
    thiz->user_database_reset();
    ///重置系统时间 
    ts->sync_time(PRODUCT_BUILD_TIMESTAMP); 
    if (is_reboot == RESET_NOT_REBOOT) {
        LOG_STORE("reset done,but not reboot.");
        return 1;
    } else {
        if (ui && ui->hook && ui->hook->set_ui_backlight) {
            ui->hook->set_ui_backlight(ui, 0);
        }
        wm->set_window_switch_forbid(wm, 1);
        wm->set_system_is_restting_device(wm, 1);   
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

static int storage_flash_protect(int protect_enable)
{
    if (protect_enable) {
        MUTEX_LOCK(thiz->priv.storage_rw_lock);
    } else {
        MUTEX_UNLOCK(thiz->priv.storage_rw_lock);
    }
    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int user_database_put(storage_manager_context_t *ctx, int database_id, const uint8_t *buf, int buf_len)
{
    int ret = -1;
    rt_device_node_t *db_dev = ctx->priv.db_dev;
    if (db_dev && db_dev->write) {
        uint32_t buf_wr[] = { database_id, (uint32_t)buf, (uint32_t)buf_len };
        ret = DEVICE_WRITE(db_dev, buf_wr, sizeof(buf_wr));
        if (ret == 0) {
            uint8_t *tmp_buf = u_malloz(buf_len);
            uint32_t buf_rd[] = {database_id, (uint32_t)tmp_buf, (uint32_t)buf_len };
            DEVICE_READ_ARGS(db_dev, buf_rd, sizeof(buf_rd));
            // LOG_STORE("write database id: %d, buf_len: %d", database_id, buf_len);
            // LOG_HEX("tx buf", tmp_buf, buf_len);
            // LOG_HEX("rx buf", buf, buf_len);
            if (memcmp(tmp_buf, buf, buf_len) != 0) {
                LOGE("storage database id %d fail, retry write.", database_id);
                ret = DEVICE_WRITE(db_dev, buf_rd, sizeof(buf_rd));
            }
            u_free(tmp_buf);
        }
    }
    return ret;
}

int user_database_get(storage_manager_context_t *ctx, int database_id, uint8_t *buf, int buf_len)
{
    int ret = -1;
    rt_device_node_t *db_dev = ctx->priv.db_dev;
    if (db_dev && db_dev->read) {
        uint32_t buf_rd[] = {database_id, (uint32_t)buf, (uint32_t)buf_len };
        ret = DEVICE_READ_ARGS(db_dev, buf_rd, sizeof(buf_rd));
        // LOG_STORE("read database id: %d, buf_len: %d", database_id, buf_len);
        // LOG_HEX("buf_rd", buf, buf_len);
    }
    return ret;
}

int user_database_remove(storage_manager_context_t *ctx, int database_id)
{
    int ret = -1;
    rt_device_node_t *db_dev = ctx->priv.db_dev;
    if (db_dev && db_dev->ioctl) {
        ret = DEVICE_IOCTL_ARGS(db_dev, OP_CODE_DB_DELETE, &database_id);
    }    
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
    ptr->flash_protect = storage_flash_protect;
    MUTEX_INIT(&ptr->priv.storage_rw_lock, storage_rw_lock);
    thiz = ptr;
    return ADD_MODULE("storage", ID_MODULE_STORAGE, on_perpare_storage_init, 
        on_post_storage_init, ptr, list_head, sizeof(storage_manager_context_t));
}
