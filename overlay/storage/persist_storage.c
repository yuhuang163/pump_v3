/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "persist_data_internal.h" // IWYU pragma: keep
#include "factory_persist_data.h"
/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static put_store_process_handle_t put_store_process_list[] = {
    #if IS_SUPPORT_DEVICE_IMU
    {STORE_ID_FACTORY_IMU_CALI_DATA, storage_write_imu_cali_data},
    #endif
    {STORE_ID_FACTORY_DEPLOY_FLAG, storage_write_factory_deploy_flag},
    {STORE_ID_FACTORY_AGE_FLAG, storage_write_factory_age_flag},
    {STORE_ID_FACTORY_AGE_DISPLAY_FLAG, storage_write_factory_age_display_flag},
    #if IS_SUPPORT_DEVICE_FSENSOR
    {STORE_ID_FACTORY_FSENSOR_CALI_FLAG, storage_write_factory_fsensor_cali_flag},
    #endif
    {STORE_ID_SYS_CONFIG_INFO, storage_write_sys_config_info},
    {STORE_ID_SYS_UTC_TIMESTAMP, storage_write_utc_timestamp},
    {STORE_ID_SYS_STATE_INFO, storage_write_sys_state_info},    
    {STORE_ID_USER_BIND_INFO, storage_write_user_bind_info},
};

const static get_store_process_handle_t get_store_process_list[] = {
    {STORE_ID_FACTORY_DEV_INFO, storage_read_factory_dev_info},
    #if IS_SUPPORT_DEVICE_IMU
    {STORE_ID_FACTORY_IMU_CALI_DATA, storage_read_imu_cali_data},
    #endif
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
    #if IS_SUPPORT_DEVICE_FSENSOR
    {STORE_ID_FACTORY_FSENSOR_CALI_FLAG, storage_read_factory_fsensor_cali_flag},
    #endif
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

const static storage_manager_context_t *thiz = NULL;
/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int user_database_reset(void)
{
    EXCEPTION_CHECK_INT(thiz);
    // storage_reset_breath_pump_info(&ctx->priv.sys_info.breath_pump_info);
    // msleep(10);
    // storage_reset_user_bind_info(&thiz->priv.sys_info.bind_info);
    // msleep(10);
    // storage_reset_sys_config_info(&thiz->priv.sys_info.config);
    // msleep(10);
    // storage_reset_sys_state_info(&thiz->priv.sys_info.state);
    // msleep(10);
    // storage_reset_breath_pump_data_head(&ctx->priv.breath_pump_data_head);
    
    return 0;
}

static void hdr_storage_work_cb(hdr_post_args_t *args)
{
    if (args->arg1 == 0) {
        // system_persist_info_sync_flash(thiz);
    }
}
 
/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void on_storage_broadcast_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    storage_manager_context_t *st = context;
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

void on_post_persist_storage_init(storage_manager_context_t *ptr)
{
    /// 加载工厂数据
    init_factory_persist_info(ptr);
    /// 加载系统用户数据 
    init_system_persist_info(ptr);
    /// 加载系统版本信息
    init_system_ver_info(ptr);
}

void on_perpare_persist_storage_init(storage_manager_context_t *ptr)
{

}

void register_persist_storage_ctx(storage_manager_context_t *ptr)
{
    ptr->priv.put_store_process_list = put_store_process_list;
    ptr->priv.get_store_process_list = get_store_process_list;
    ptr->priv.put_store_ram_val_list = put_store_ram_val_list;
    ptr->priv.get_store_ram_val_list = get_store_ram_val_list;
    ptr->priv.put_store_process_list_len = LENGTH_OF_ARRAY(put_store_process_list);
    ptr->priv.get_store_process_list_len = LENGTH_OF_ARRAY(get_store_process_list);
    ptr->priv.put_store_ram_val_list_len = LENGTH_OF_ARRAY(put_store_ram_val_list);
    ptr->priv.get_store_ram_val_list_len = LENGTH_OF_ARRAY(get_store_ram_val_list);

    ptr->user_database_reset = user_database_reset;

    thiz = ptr;
}