/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "storage_internal_api.h" // IWYU pragma: keep

/*
 * DEFINES
 *****************************************************************************************
 */

/*
 * LOCAL VARIABLE DEFINITIONS
 *****************************************************************************************
 */
const static put_store_process_handle_t put_store_process_list[] = {
    {STORE_ID_FACTORY_IMU_CALI_DATA, storage_write_imu_cali_data},
    {STORE_ID_FACTORY_DEPLOY_FLAG, storage_write_factory_deploy_flag},
    {STORE_ID_FACTORY_AGE_FLAG, storage_write_factory_age_flag},
    {STORE_ID_FACTORY_AGE_DISPLAY_FLAG, storage_write_factory_age_display_flag},
    {STORE_ID_FACTORY_FSENSOR_CALI_FLAG, storage_write_factory_fsensor_cali_flag},
    {STORE_ID_SYS_CONFIG_INFO, storage_write_sys_config_info},
    {STORE_ID_SYS_UTC_TIMESTAMP, storage_write_utc_timestamp},
    {STORE_ID_SYS_STATE_INFO, storage_write_sys_state_info},    
    {STORE_ID_USER_BIND_INFO, storage_write_user_bind_info},
};

const static get_store_process_handle_t get_store_process_list[] = {
    {STORE_ID_FACTORY_DEV_INFO, storage_read_factory_dev_info},
    {STORE_ID_FACTORY_IMU_CALI_DATA, storage_read_imu_cali_data},
    {STORE_ID_SYS_CONFIG_INFO, storage_read_sys_config_info},
    {STORE_ID_SYS_UTC_TIMESTAMP, storage_read_utc_timestamp},
    {STORE_ID_SYS_STATE_INFO, storage_read_sys_state_info},    
    {STORE_ID_USER_BIND_INFO, storage_read_user_bind_info},
};

// const static del_store_process_handle_t del_store_process_list[] = {
//     {STORE_ID_BREATH_PUMP_DATA_BODY, storage_del_brush_data_body},
// };

const static put_store_ram_val_handle_t put_store_ram_val_list[] = {
    {STORE_ID_SYS_TIME_ZONE, storage_write_sys_time_zone},
    {STORE_ID_SYS_AP_MODE_ONOFF, storage_write_ap_mode_onoff},
    {STORE_ID_SYS_SILENT_UPGRADE, storage_write_slient_upgrade_onoff},
    {STORE_ID_SYS_UI_STYLE, storage_write_sys_ui_style},
    {STORE_ID_SYS_LOG_SWITCH, storage_write_log_switch},
    {STORE_ID_SYS_LANGUAGE, storage_write_language},
    {STORE_ID_SYS_SOUND_MODE_ONOFF, storage_write_sound_mode_onoff},
    {STORE_ID_SYS_MICROSEISM_ONOFF, storage_write_microseism_onoff},
    {STORE_ID_SYS_AI_MODE_ONOFF, storage_write_ai_mode_onoff},
    {STORE_ID_SYS_GUIDE_DEPLOYED, storage_write_guide_deployed},
    {STORE_ID_SYS_BATTERY_HEALTH, storage_write_battery_health},
    {STORE_ID_SYS_SYNC_SYS_TIME, storage_write_sync_sys_time},
};

const static get_store_ram_val_handle_t get_store_ram_val_list[] = {
    {STORE_ID_FACTORY_DEPLOY_FLAG, storage_read_factory_deploy_flag},
    {STORE_ID_FACTORY_AGE_FLAG, storage_read_factory_age_flag},
    {STORE_ID_FACTORY_AGE_DISPLAY_FLAG, storage_read_factory_age_display_flag},
    {STORE_ID_FACTORY_FSENSOR_CALI_FLAG, storage_read_factory_fsensor_cali_flag},
    {STORE_ID_SYS_TIME_ZONE, storage_read_sys_time_zone},
    {STORE_ID_SYS_AP_MODE_ONOFF, storage_read_ap_mode_onoff},
    {STORE_ID_SYS_SILENT_UPGRADE, storage_read_slient_upgrade_onoff},
    {STORE_ID_SYS_UI_STYLE, storage_read_sys_ui_style},
    {STORE_ID_SYS_LOG_SWITCH, storage_read_log_switch},
    {STORE_ID_SYS_LANGUAGE, storage_read_language},
    {STORE_ID_SYS_SOUND_MODE_ONOFF, storage_read_sound_mode_onoff},
    {STORE_ID_SYS_MICROSEISM_ONOFF, storage_read_microseism_onoff},
    {STORE_ID_SYS_AI_MODE_ONOFF, storage_read_ai_mode_onoff},
    {STORE_ID_SYS_GUIDE_DEPLOYED, storage_read_guide_deployed},
    {STORE_ID_SYS_BATTERY_HEALTH, storage_read_battery_health},
    {STORE_ID_SYS_SYNC_SYS_TIME, storage_read_sync_sys_time},
    {STORE_ID_USER_BIND_INFO, storage_user_is_binded},
};
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
int user_database_put(storage_context_t *ctx, database_key_t key, const uint8_t *buf, int buf_len)
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

int user_database_get(storage_context_t *ctx, database_key_t key, uint8_t *buf, int buf_len)
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

int user_database_remove(storage_context_t *ctx, database_key_t key)
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

int user_database_reset(storage_context_t *ctx)
{
    // storage_reset_breath_pump_info(&ctx->priv.sys_info.breath_pump_info);
    msleep(10);
    storage_reset_user_bind_info(&ctx->priv.sys_info.bind_info);
    msleep(10);
    storage_reset_sys_config_info(&ctx->priv.sys_info.config);
    msleep(10);
    storage_reset_sys_state_info(&ctx->priv.sys_info.state);
    msleep(10);
    // storage_reset_breath_pump_data_head(&ctx->priv.breath_pump_data_head);
    
    return 0;
}

int put_store_val(persist_store_id_e id, int arg1, int arg2, const void *val_in)
{
    int ret = 0;
    storage_context_t *ctx = get_storage_manager_context();
    MUTEX_LOCK(ctx->priv.storage_rw_lock);
    const put_store_process_handle_t *list = put_store_process_list;
    for (int i = 0; i < LENGTH_OF_ARRAY(put_store_process_list); i++) {
        if (list[i].store_id == id) {
            ret = list[i].store_process(arg1, arg2, val_in);
            break;
        }
    }
    MUTEX_UNLOCK(ctx->priv.storage_rw_lock);

    return ret;
}

int get_store_val(persist_store_id_e id, int arg1, int arg2,  void *val_out)
{
    int ret = 0;
    storage_context_t *ctx = get_storage_manager_context();
    MUTEX_LOCK(ctx->priv.storage_rw_lock);
    const get_store_process_handle_t *list = get_store_process_list;
    for (int i = 0; i < LENGTH_OF_ARRAY(get_store_process_list); i++) {
        if (list[i].store_id == id) {
            ret = list[i].store_process(arg1, arg2, val_out);
            break;
        }
    }
    MUTEX_UNLOCK(ctx->priv.storage_rw_lock);
    return ret;
}

int del_store_val(persist_store_id_e id, int arg1, int arg2,  const void *val_in)
{
    int ret = 0;
    // storage_context_t *ctx = get_storage_manager_context();
    // MUTEX_LOCK(ctx->priv.storage_rw_lock);
    // const del_store_process_handle_t *list = del_store_process_list;
    // for (int i = 0; i < LENGTH_OF_ARRAY(del_store_process_list); i++) {
    //     if (list[i].store_id == id) {
    //         ret = list[i].store_process(arg1, arg2, val_in);
    //         break;
    //     }
    // }
    // MUTEX_UNLOCK(ctx->priv.storage_rw_lock);

    return ret;
}       

int get_store_ram_val(persist_store_id_e id)
{
    int ret = 0;
    storage_context_t *ctx = get_storage_manager_context();
    MUTEX_LOCK(ctx->priv.storage_rw_lock);
    const get_store_ram_val_handle_t *list = get_store_ram_val_list;
    for (int i = 0; i < LENGTH_OF_ARRAY(get_store_ram_val_list); i++) {
        if (list[i].store_id == id) {
            ret = list[i].store_process();
            break;
        }
    }
    MUTEX_UNLOCK(ctx->priv.storage_rw_lock);

    return ret;
}

void put_store_ram_val(persist_store_id_e id, int args)
            {
    storage_context_t *ctx = get_storage_manager_context();
    MUTEX_LOCK(ctx->priv.storage_rw_lock);
    const put_store_ram_val_handle_t *list = put_store_ram_val_list;
    for (int i = 0; i < LENGTH_OF_ARRAY(put_store_ram_val_list); i++) {
        if (list[i].store_id == id) {
            list[i].store_process(args);
            break;
        }
    }
    MUTEX_UNLOCK(ctx->priv.storage_rw_lock);
}
