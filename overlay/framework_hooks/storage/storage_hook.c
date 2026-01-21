/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "persist_storage.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "factory_persist_data.h"
#include "system_persist_data.h"
#include "breast_pump_persist_data.h"

/*
* DEFINES
*****************************************************************************************
*/
#define STORAGE_RAM_SYNC_FLASH_INTERVAL_MS    (10 * 1000)  /// 20s
#define STORAGE_PERSIST_TASK_POLICY1_NAME     "persist_policy1"  /// 存储策略1
#define STORAGE_RAM_SYNC_FLASH_TASK_NAME      "ram_sync_flash_task"  /// 内存同步闪存任务

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
const static put_store_process_handle_t put_store_process_list[] = {
    #if IS_SUPPORT_DEVICE_IMU
    {STORE_ID_FACTORY_IMU_CALI_DATA, write_imu_cali_data_to_flash},
    #endif
    {STORE_ID_FACTORY_DEPLOY_FLAG, write_factory_deploy_flag_to_flash},
    {STORE_ID_FACTORY_AGE_FLAG, write_factory_age_flag_to_flash},
    {STORE_ID_FACTORY_AGE_DISPLAY_FLAG, write_factory_age_display_flag_to_flash},
    #if IS_SUPPORT_DEVICE_FSENSOR
    {STORE_ID_FACTORY_FSENSOR_CALI_FLAG, write_factory_fsensor_cali_flag_to_flash},
    #endif
    {STORE_ID_FACTORY_FLAG_INFO, write_factory_flag_info_to_flash},
    {STORE_ID_SYS_CONFIG_INFO, write_sys_config_info_to_flash},
    {STORE_ID_SYS_UTC_TIMESTAMP, write_utc_timestamp_to_flash},
    {STORE_ID_SYS_STATE_INFO, write_sys_state_info_to_flash},    
    {STORE_ID_USER_BIND_INFO, write_user_bind_info_to_flash},
    {STORE_ID_FACTORY_DEV_INFO, write_factory_dev_info_to_flash},
};

const static get_store_process_handle_t get_store_process_list[] = {
    {STORE_ID_FACTORY_DEV_INFO, read_factory_dev_info_from_flash},
    #if IS_SUPPORT_DEVICE_IMU
    {STORE_ID_FACTORY_IMU_CALI_DATA, read_imu_cali_data_from_flash},
    #endif
    {STORE_ID_FACTORY_FLAG_INFO, read_factory_flag_info_from_flash},
    {STORE_ID_SYS_CONFIG_INFO, read_sys_config_info_from_flash},
    {STORE_ID_SYS_UTC_TIMESTAMP, read_utc_timestamp_from_flash},
    {STORE_ID_SYS_STATE_INFO, read_sys_state_info_from_flash},    
    {STORE_ID_USER_BIND_INFO, read_user_bind_info_from_flash},
};

const static del_store_process_handle_t del_store_process_list[] = {
    {STORE_ID_BREAST_PUMP_DATA_BODY, del_pump_data_body_from_flash},
};

const static put_store_ram_val_handle_t put_store_ram_val_list[] = {
    {STORE_ID_SYS_TIME_ZONE, write_sys_time_zone_to_ram},
    {STORE_ID_SYS_AP_MODE_ONOFF, write_ap_mode_onoff_to_ram},
    {STORE_ID_SYS_SILENT_UPGRADE, write_slient_upgrade_onoff_to_ram},
    {STORE_ID_SYS_UI_STYLE, write_sys_ui_style_to_ram},
    {STORE_ID_SYS_LOG_SWITCH, write_log_switch_to_ram},
    {STORE_ID_SYS_LANGUAGE, write_language_to_ram},
    {STORE_ID_SYS_GUIDE_DEPLOYED, write_guide_deployed_to_ram},
    {STORE_ID_SYS_BATTERY_HEALTH, write_battery_health_to_ram},
    {STORE_ID_SYS_SYNC_SYS_TIME, write_sync_sys_time_to_ram},

    {STORE_ID_BREAST_PUMP_TIME, write_pump_time_to_ram},
    {STORE_ID_PUMP_SETTING_INFO, write_pump_setting_info_to_ram},
    {STORE_ID_PUMP_CUSTOM_STEPS_PARAMETER_INFO, write_pump_custom_steps_parameter_to_ram},
    {STORE_ID_REMIND_PUMP_EXCEPTION, write_pump_exception_remind_to_ram},
    {STORE_ID_REMIND_PUMP_MODE_CHANGE, write_pump_mode_change_remind_to_ram},
    {STORE_ID_REMIND_PUMP_FINISH, write_pump_finish_remind_to_ram},
    {STORE_ID_PUMP_CONTROL_TYPE, write_pump_control_type_to_ram},
    {STORE_ID_SYS_NIGHT_LIGHT_INFO, write_night_light_info_to_ram},
    {STORE_ID_SYS_AIRPLANE_MODE_STATE, write_airplane_mode_state_to_ram},
    {STORE_ID_BREAST_PUMP_BURY_INFO, write_pump_buried_info_to_ram},
};

const static get_store_ram_val_handle_t get_store_ram_val_list[] = {
    {STORE_ID_FACTORY_DEPLOY_FLAG, read_factory_deploy_flag_from_ram},
    {STORE_ID_FACTORY_AGE_FLAG, read_factory_age_flag_from_ram},
    {STORE_ID_FACTORY_AGE_DISPLAY_FLAG, read_factory_age_display_flag_from_ram},
    #if IS_SUPPORT_DEVICE_FSENSOR
    {STORE_ID_FACTORY_FSENSOR_CALI_FLAG, read_factory_fsensor_cali_flag_from_ram},
    #endif
    {STORE_ID_SYS_TIME_ZONE, read_sys_time_zone_from_ram},
    {STORE_ID_SYS_AP_MODE_ONOFF, read_ap_mode_onoff_from_ram},
    {STORE_ID_SYS_SILENT_UPGRADE, read_slient_upgrade_onoff_from_ram},
    {STORE_ID_SYS_UI_STYLE, read_sys_ui_style_from_ram},
    {STORE_ID_SYS_LOG_SWITCH, read_log_switch_from_ram},
    {STORE_ID_SYS_LANGUAGE, read_language_from_ram},
    {STORE_ID_SYS_GUIDE_DEPLOYED, read_guide_deployed_from_ram},
    {STORE_ID_SYS_BATTERY_HEALTH, read_battery_health_from_ram},
    {STORE_ID_SYS_SYNC_SYS_TIME, read_sync_sys_time_from_ram},
    {STORE_ID_USER_BIND_INFO, user_is_binded_from_ram},
    {STORE_ID_BREAST_PUMP_TIME, read_pump_time_from_ram},
    {STORE_ID_PUMP_SETTING_INFO, read_pump_setting_info_from_ram},
    {STORE_ID_PUMP_CUSTOM_STEPS_PARAMETER_INFO, read_pump_custom_steps_parameter_from_ram},
    {STORE_ID_PUMP_CONTROL_TYPE, read_pump_control_type_from_ram},
    {STORE_ID_REMIND_PUMP_EXCEPTION, read_pump_exception_remind_from_ram},
    {STORE_ID_REMIND_PUMP_MODE_CHANGE, read_pump_mode_change_remind_from_ram},
    {STORE_ID_REMIND_PUMP_FINISH, read_pump_finish_remind_from_ram},
    {STORE_ID_SYS_NIGHT_LIGHT_INFO, read_night_light_info_from_ram},
    {STORE_ID_SYS_AIRPLANE_MODE_STATE, read_airplane_mode_state_from_ram},
};

static storage_manager_context_t *thiz = NULL;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int init_system_ver_info(storage_manager_context_t *ctx)
{
    sec_sys_ver_t *info = &ctx->priv.ver_info;
    memset(info, 0, sizeof(sec_sys_ver_t));
    info->sys_local_protocol_id = ROOT_BLE_PROTOCOL_VERSION;
    info->sys_peer_app_protocol_id = 0;
    info->sys_product_id = DEFAULT_PID_CODE;
    info->ui_res_id = 101;

    info->sw_version = ROOT_FIRMWARE_VERSION;
    info->sw_version_type = ROOT_FIRMWARE_TYPE;
    info->hw_version = ROOT_HARDWARE_VERSION;
    info->soc_version = ROOT_SOC_PLAT;

    info->dfu_type = 0; 
    strcpy(info->device_name, ROOT_DEVICE_NAME);
    return 0;
}

static int init_system_persist_info(storage_manager_context_t *ctx)
{
    sys_persist_info_t *sys_info = &ctx->priv.sys_info;
    memset(sys_info, 0, sizeof(sys_persist_info_t));
    store_func_args_t args = {
        .ctx = ctx,
        .arg1 = 0,
        .arg2 = 0,
        .val = NULL,
    };
    args.val = &sys_info->config;
    read_sys_config_info_from_flash(args);
    args.val = &sys_info->state;
    read_sys_state_info_from_flash(args);
    args.val = &sys_info->bind_info;
    read_user_bind_info_from_flash(args);
    args.val = &sys_info->pump_info;
    read_pump_info_from_flash(args);
    args.val = &sys_info->buried_info_list;
    read_pump_buried_info_from_flash(args);
    ctx->priv.sys_info_crc[STORE_CRC_TYPE_SYS_CONFIG] = CRC16(&sys_info->config, sizeof(sys_config_info_t));
    ctx->priv.sys_info_crc[STORE_CRC_TYPE_SYS_STATE]  = CRC16(&sys_info->state, sizeof(sys_state_info_t));
    ctx->priv.sys_info_crc[STORE_CRC_TYPE_PUMP_INFO]  = CRC16(&sys_info->pump_info, sizeof(sys_pump_info_t));
    ctx->priv.sys_info_crc[STORE_CRC_TYPE_PUMP_BURY_INFO]  = CRC16(&sys_info->buried_info_list, sizeof(sys_buried_info_list_t));
    return 0;
}

static int system_persist_info_sync_flash(storage_manager_context_t *ctx)
{
    sys_persist_info_t *sys_info = &ctx->priv.sys_info;
    uint16_t info_crc = 0;
    store_func_args_t args = {
        .ctx = ctx,
        .arg1 = 0,
        .arg2 = 0,
        .val = NULL,
    };

    ///  系统配置
    info_crc = CRC16(&sys_info->config, sizeof(sys_config_info_t));
    if (info_crc != ctx->priv.sys_info_crc[STORE_CRC_TYPE_SYS_CONFIG]) {
        args.val = &sys_info->config;
        write_sys_config_info_to_flash(args);
        LOG_STORE("-->system config info sync flash");
    }
    ///  系统状态
    info_crc = CRC16(&sys_info->state, sizeof(sys_state_info_t));
    if (info_crc != ctx->priv.sys_info_crc[STORE_CRC_TYPE_SYS_STATE]) {
        args.val = &sys_info->state;
        write_sys_state_info_to_flash(args);
        LOG_STORE("-->system state info sync flash");
    }
    ///  泵奶信息
    info_crc = CRC16(&sys_info->pump_info, sizeof(sys_pump_info_t));
    if (info_crc != ctx->priv.sys_info_crc[STORE_CRC_TYPE_PUMP_INFO]) {
        args.val = &sys_info->pump_info;
        write_pump_info_to_flash(args);
        LOG_STORE("-->pump info sync flash");
    }
    ///  泵奶埋点信息
    info_crc = CRC16(&sys_info->buried_info_list, sizeof(sys_buried_info_list_t));
    if (info_crc != ctx->priv.sys_info_crc[STORE_CRC_TYPE_PUMP_BURY_INFO]) {
        args.val = &sys_info->buried_info_list;
        write_pump_buried_info_to_flash(args);
        LOG_STORE("-->pump buried info sync flash");
    }
    return 0;
}

static int user_database_reset(void)
{
    EXCEPTION_CHECK_INT(thiz);
    reset_pump_info(thiz, &thiz->priv.sys_info.pump_info);
    msleep(10);
    reset_user_bind_info(thiz, &thiz->priv.sys_info.bind_info);
    msleep(10);
    reset_sys_config_info(thiz, &thiz->priv.sys_info.config);
    msleep(10);
    reset_sys_state_info(thiz, &thiz->priv.sys_info.state);
    msleep(10);
    reset_pump_data_head(thiz, &thiz->priv.pump_data_head);
    msleep(10);
    reset_pump_buried_info(thiz, &thiz->priv.sys_info.buried_info_list);
    LOGE("==> user database reset !!!!!!!!!!!!!!!");
    
    return 0;
}

static void hdr_storage_work_cb(hdr_post_args_t *args)
{
    if (args->arg1 == 0) {
        system_persist_info_sync_flash(thiz);
    }
}

static void storage_ram_sync_flash_cb(hdr_post_args_t *args)
{
    breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_VOID(pump);
    storage_manager_context_t *st = args->user_data;
    EXCEPTION_CHECK_VOID(st);
    if (pump->is_on_working()) {
        LOG_STORE("-->pump working, ignore ram sync flash");
        goto EXIT_LABEL;
    }
    system_persist_info_sync_flash(st);
EXIT_LABEL:
    POST_TASK_HANDLER_DELAYED_ARG1(storage_ram_sync_flash_cb, 
        STORAGE_RAM_SYNC_FLASH_INTERVAL_MS, st, STORAGE_RAM_SYNC_FLASH_TASK_NAME);
}

static void on_storage_broadcast_route(broadcast_mask_id_t id, void *context, int arg1, int arg2, broadcast_priv_data_t *pdata)
{
    storage_manager_context_t *st = context;
    window_manager_context_t *wm = (window_manager_context_t *)find_module(ID_MODULE_WM);
    pump_result_info_t *user_data = NULL;
    
    switch (id.code) {
        case ID_BROADCAST_PM_STATE_CHANGE:
            if (arg1 == ID_SYS_PM_SUSPEND) {
                REMOVE_TASK_HANDLER_DIRECTLY(STORAGE_RAM_SYNC_FLASH_TASK_NAME);
                POST_TASK_HANDLER_DELAYED_ARG2(hdr_storage_work_cb, 500, 0, st, STORAGE_PERSIST_TASK_POLICY1_NAME);
            } else if (arg1 == ID_SYS_PM_RESUME) {
                POST_TASK_HANDLER_DELAYED_ARG1(storage_ram_sync_flash_cb, 
                    STORAGE_RAM_SYNC_FLASH_INTERVAL_MS, st, STORAGE_RAM_SYNC_FLASH_TASK_NAME);
            }
            break;
        case ID_BROADCAST_SYS_PERSIST_INFO_SAVE:
                PRIV_DATA_TO_USER_DATA_V2(user_data, pdata, pump_result_info_t);
                if (user_data) {
                    LOG_STORE("-->save insert_one");
                    POST_TASK_HANDLER_DELAYED_ARG2(hdr_storage_work_cb, 0, 1, user_data, STORAGE_PERSIST_TASK_POLICY1_NAME);
                }
            break;
        case ID_BROADCAST_SYS_POWER_STATE_CHANGE:
            if (arg1 == ID_BROADCAST_SYS_POWER_OFF) {
                LOG_STORE("-->system shutdown");
                POST_TASK_HANDLER_DELAYED_ARG2(hdr_storage_work_cb, 0, 0, st, STORAGE_PERSIST_TASK_POLICY1_NAME);
            } else if (arg1 == ID_BROADCAST_SYS_POWER_ON) {
                POST_TASK_HANDLER_DELAYED_ARG1(storage_ram_sync_flash_cb, 
                    STORAGE_RAM_SYNC_FLASH_INTERVAL_MS, st, STORAGE_RAM_SYNC_FLASH_TASK_NAME);
            }
            break;
        default:
                break;
        }
}

static int on_post_storage_init_route(storage_manager_context_t *ctx)
{
    /// 加载工厂数据
    init_factory_persist_info(ctx);
    /// 加载系统用户数据 
    init_system_persist_info(ctx);
    /// 加载系统版本信息
    init_system_ver_info(ctx);
    return 0;
}

static int on_perpare_storage_init_route(storage_manager_context_t *ctx)
{
    return 0;
}


/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
void register_storage_hook(storage_manager_context_t *ctx)
{
    ctx->hook = u_malloz(sizeof(struct __storage_hook));
    ctx->hook->on_post_init_route = on_post_storage_init_route;
    ctx->hook->on_perpare_init_route = on_perpare_storage_init_route;
    ctx->hook->on_storage_broadcast_route = on_storage_broadcast_route;

    ctx->priv.put_store_process_list = put_store_process_list;
    ctx->priv.get_store_process_list = get_store_process_list;
    ctx->priv.put_store_ram_val_list = put_store_ram_val_list;
    ctx->priv.get_store_ram_val_list = get_store_ram_val_list;
    ctx->priv.del_store_process_list = del_store_process_list; 
    ctx->priv.put_store_process_list_len = LENGTH_OF_ARRAY(put_store_process_list);
    ctx->priv.get_store_process_list_len = LENGTH_OF_ARRAY(get_store_process_list);
    ctx->priv.put_store_ram_val_list_len = LENGTH_OF_ARRAY(put_store_ram_val_list);
    ctx->priv.get_store_ram_val_list_len = LENGTH_OF_ARRAY(get_store_ram_val_list);
    ctx->priv.del_store_process_list_len = LENGTH_OF_ARRAY(del_store_process_list);

    ctx->user_database_reset = user_database_reset;

    thiz = ctx;
}

 