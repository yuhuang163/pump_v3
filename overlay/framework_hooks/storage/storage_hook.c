/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "persist_storage.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "factory_persist_data.h"
#include "system_persist_data.h"
#include "breast_pump_persist_data.h"
#include "data_bury_persist_data.h"
#include "resource_persist_data.h"


/*
* DEFINES
*****************************************************************************************
*/
#define STORAGE_RAM_SYNC_FLASH_INTERVAL_MS    (5 * 1000)  /// 20s
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
    {STORE_ID_SYS_REBOOT_REASON, write_sys_reboot_reason_to_flash},
    {STORE_ID_SYS_STATE_INFO, write_sys_state_info_to_flash},    
    {STORE_ID_USER_BIND_INFO, write_user_bind_info_to_flash},
    {STORE_ID_SYS_BIND_STATUS, write_bind_status_to_flash},
    {STORE_ID_FACTORY_DEV_INFO, write_factory_dev_info_to_flash},
    {STORE_ID_BREAST_PUMP_DATA_HEAD, write_pump_data_head_to_flash},
    {STORE_ID_BREAST_PUMP_DATA_BODY, write_pump_data_body_to_flash},
    {STORE_ID_DATA_BURY_FLASH_BANK, write_data_bury_flash_bank_to_flash},
    {STORE_ID_DATA_BURY_RING_META, write_data_bury_ring_meta_to_flash},
    {STORE_ID_DATA_BURY_RING_CHUNK, write_data_bury_ring_chunk_to_flash},
    {STORE_ID_IMG_RES_OTA_INFO, write_img_res_ota_info_to_flash},
#if IS_SUPPORT_LANG_PACK
    {STORE_ID_LANG_PACK_OTA_INFO, write_lang_pack_ota_info_to_flash},
#endif
};

const static get_store_process_handle_t get_store_process_list[] = {
    {STORE_ID_FACTORY_DEV_INFO, read_factory_dev_info_from_flash},
#if IS_SUPPORT_DEVICE_IMU
    {STORE_ID_FACTORY_IMU_CALI_DATA, read_imu_cali_data_from_flash},
#endif
    {STORE_ID_FACTORY_FLAG_INFO, read_factory_flag_info_from_flash},
    {STORE_ID_SYS_CONFIG_INFO, read_sys_config_info_from_flash},
    {STORE_ID_SYS_UTC_TIMESTAMP, read_utc_timestamp_from_flash},
    {STORE_ID_SYS_REBOOT_REASON, read_sys_reboot_reason_from_flash},
    {STORE_ID_SYS_STATE_INFO, read_sys_state_info_from_flash},    
    {STORE_ID_USER_BIND_INFO, read_user_bind_info_from_flash},
    {STORE_ID_SYS_BIND_STATUS, read_bind_status_from_flash},
    {STORE_ID_BREAST_PUMP_DATA_HEAD, read_pump_data_head_from_flash},
    {STORE_ID_BREAST_PUMP_DATA_BODY, read_pump_data_body_from_flash},
    {STORE_ID_DATA_BURY_FLASH_BANK, read_data_bury_flash_bank_from_flash},
    {STORE_ID_DATA_BURY_RING_META, read_data_bury_ring_meta_from_flash},
    {STORE_ID_DATA_BURY_RING_CHUNK, read_data_bury_ring_chunk_from_flash},  
    {STORE_ID_IMG_RES_OTA_INFO, read_img_res_ota_info_from_flash},
#if IS_SUPPORT_LANG_PACK
    {STORE_ID_LANG_PACK_OTA_INFO, read_lang_pack_ota_info_from_flash},
#endif
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
    {STORE_ID_SYS_REBOOT_REASON, write_sys_reboot_reason_to_ram},
    {STORE_ID_SYS_GUIDE_DEPLOYED, write_guide_deployed_to_ram},
    {STORE_ID_SYS_BATTERY_HEALTH, write_battery_health_to_ram},
    {STORE_ID_SYS_SYNC_SYS_TIME, write_sync_sys_time_to_ram},

    {STORE_ID_BREAST_PUMP_TIME, write_pump_time_to_ram},
    {STORE_ID_PUMP_SETTING_INFO, write_pump_setting_info_to_ram},
    {STORE_ID_PUMP_MODE_STEPS_INFO, write_pump_mode_steps_info_to_ram},
    {STORE_ID_PUMP_MODE_NAME_INFO, write_pump_mode_name_to_ram},
    {STORE_ID_PUMP_CONTROL_TYPE, write_pump_control_type_to_ram},
    {STORE_ID_SYS_ADAPTIVE_SWITCH, write_global_adaptive_switch_to_ram},
#if IS_SUPPORT_NIGHT_LIGHT
    {STORE_ID_SYS_NIGHT_LIGHT_GEAR, write_night_light_gear_to_ram},
    {STORE_ID_SYS_NIGHT_LIGHT_STARTUP_SWITCH, write_night_light_startup_switch_to_ram},
#endif
    {STORE_ID_SYS_BACK_LIGHT_LEVEL, write_back_light_level_to_ram},
    {STORE_ID_SYS_AIRPLANE_MODE_STATE, write_airplane_mode_state_to_ram},
    {STORE_ID_BREAST_PUMP_BURY_INFO, write_pump_buried_info_to_ram},
    {STORE_ID_DATA_BURY_RING_APPEND, write_data_bury_ring_event_to_ram},
    {STORE_ID_DATA_BURY_RING_POP_OLDEST, write_data_bury_ring_pop_oldest_to_ram},
    {STORE_ID_BREAST_PUMP_MODE, write_pump_mode_to_ram},
    {STORE_ID_BREAST_PUMP_MODE_WITH_TYPE, write_pump_mode_with_type_to_ram},
    {STORE_ID_MODE_NEW_STEP_INFO_FLAG, write_mode_new_step_info_flag_to_ram},
    {STORE_ID_SYS_BIND_STATUS, write_bind_status_to_ram},
    {STORE_ID_PUMP_MODE_ACTIVATE_STATUS, write_pump_mode_activate_to_ram},
    {STORE_ID_IMG_RES_OTA_INFO, write_img_res_ota_info_to_ram},
#if IS_SUPPORT_LANG_PACK
    {STORE_ID_LANG_PACK_OTA_INFO, write_lang_pack_ota_info_to_ram},
#endif
    {STORE_ID_SYS_BATTERY_PERCENT, write_battery_percent_to_ram},
    {STORE_ID_SYS_SEAL_LEAK_ENABLED, write_seal_leak_enabled_to_ram},
};

const static get_store_ram_val_handle_t get_store_ram_val_list[] = {
    {STORE_ID_FACTORY_DEPLOY_FLAG, read_factory_deploy_flag_from_ram},
    {STORE_ID_FACTORY_AGE_FLAG, read_factory_age_flag_from_ram},
    {STORE_ID_FACTORY_AGE_DISPLAY_FLAG, read_factory_age_display_flag_from_ram},
    {STORE_ID_FACTORY_SUCTION_TABLE, read_factory_suction_table_from_ram},
    {STORE_ID_FACTORY_MOTOR_TYPE, read_factory_motor_type_from_ram},
    {STORE_ID_FACTORY_BATTERY_TYPE, read_factory_battery_type_from_ram},
#if IS_SUPPORT_DEVICE_LIGHT_SENSOR_CALIB
    {STORE_ID_FACTORY_LIGHT_SENSOR_CALIB_DATA, read_factory_light_sensor_calib_data_from_ram},
#endif
#if IS_SUPPORT_DEVICE_FSENSOR
    {STORE_ID_FACTORY_FSENSOR_CALI_FLAG, read_factory_fsensor_cali_flag_from_ram},
#endif
    {STORE_ID_SYS_TIME_ZONE, read_sys_time_zone_from_ram},
    {STORE_ID_SYS_AP_MODE_ONOFF, read_ap_mode_onoff_from_ram},
    {STORE_ID_SYS_SILENT_UPGRADE, read_slient_upgrade_onoff_from_ram},
    {STORE_ID_SYS_UI_STYLE, read_sys_ui_style_from_ram},
    {STORE_ID_SYS_LOG_SWITCH, read_log_switch_from_ram},
    {STORE_ID_SYS_LANGUAGE, read_language_from_ram},
    {STORE_ID_SYS_REBOOT_REASON, read_sys_reboot_reason_from_ram},
    {STORE_ID_SYS_GUIDE_DEPLOYED, read_guide_deployed_from_ram},
    {STORE_ID_SYS_BATTERY_HEALTH, read_battery_health_from_ram},
    {STORE_ID_SYS_SYNC_SYS_TIME, read_sync_sys_time_from_ram},
    {STORE_ID_USER_BIND_INFO, user_is_binded_from_ram},
    {STORE_ID_BREAST_PUMP_TIME, read_pump_time_from_ram},
    {STORE_ID_PUMP_SETTING_INFO, read_pump_setting_info_from_ram},
    {STORE_ID_PUMP_MODE_STEPS_INFO, read_pump_mode_steps_info_from_ram},
    {STORE_ID_PUMP_MODE_NAME_INFO, read_pump_mode_name_from_ram},
    {STORE_ID_PUMP_CONTROL_TYPE, read_pump_control_type_from_ram},
    {STORE_ID_SYS_ADAPTIVE_SWITCH, read_global_adaptive_switch_from_ram},
#if IS_SUPPORT_NIGHT_LIGHT
    {STORE_ID_SYS_NIGHT_LIGHT_GEAR, read_night_light_gear_from_ram},
    {STORE_ID_SYS_NIGHT_LIGHT_STARTUP_SWITCH, read_night_light_startup_switch_from_ram},
#endif
    {STORE_ID_SYS_BACK_LIGHT_LEVEL, read_back_light_level_from_ram},
    {STORE_ID_SYS_AIRPLANE_MODE_STATE, read_airplane_mode_state_from_ram},
    {STORE_ID_BREAST_PUMP_DATA_HEAD, read_pump_data_head_from_ram},
    {STORE_ID_BREAST_PUMP_MODE, read_pump_mode_from_ram},
    {STORE_ID_BREAST_PUMP_MODE_WITH_TYPE, read_pump_mode_with_type_from_ram},
    {STORE_ID_MODE_NEW_STEP_INFO_FLAG, read_mode_new_step_info_flag_to_ram},
    {STORE_ID_SYS_BIND_STATUS, read_bind_status_from_ram},
    {STORE_ID_PUMP_MODE_ACTIVATE_STATUS, read_pump_mode_activate_from_ram},
    {STORE_ID_PUMP_MODE_STEPS_INFO_ID, read_pump_mode_steps_info_id_from_ram},
    {STORE_ID_DATA_BURY_RING_PENDING_COUNT, read_data_bury_ring_pending_count_from_ram},
    {STORE_ID_DATA_BURY_RING_PEEK_OLDEST, read_data_bury_ring_peek_oldest_from_ram},
    {STORE_ID_IMG_RES_OTA_INFO, read_img_res_ota_info_from_ram},
#if IS_SUPPORT_LANG_PACK
    {STORE_ID_LANG_PACK_OTA_INFO, read_lang_pack_ota_info_from_ram},
#endif
    {STORE_ID_SYS_BATTERY_PERCENT, read_battery_percent_from_ram},
    {STORE_ID_SYS_SEAL_LEAK_ENABLED, read_seal_leak_enabled_from_ram},
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
    info->sys_product_id = ROOT_PRODUCT_ID;
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
    breast_pump_data_head_t *pump_data_head = &ctx->priv.pump_data_head;
    memset(sys_info, 0, sizeof(sys_persist_info_t));
    store_func_args_t args = {
        .ctx = ctx,
        .arg1 = 0,
        .arg2 = 0,
        .val = NULL,
    };
    args.val = &sys_info->config;
    read_sys_config_info_from_flash(args);  /// 读取系统配置信息
    args.val = &sys_info->state;
    read_sys_state_info_from_flash(args);  /// 读取系统状态信息
    msleep(10);
    args.val = &sys_info->bind_info;
    read_user_bind_info_from_flash(args);  /// 读取用户绑定信息
    args.val = &sys_info->reboot_reason;
    read_sys_reboot_reason_from_flash(args);  /// 读取系统重启原因
    args.val = &sys_info->pump_info;
    read_pump_info_from_flash(args);  /// 读取泵奶配置信息
    msleep(10);
    args.val = &sys_info->mode_info;
    read_pump_mode_steps_info_from_flash(args);  /// 读取泵奶模式步骤信息
    args.val = &sys_info->buried_info_list;
    read_pump_buried_info_from_flash(args);  /// 读取泵奶埋点信息
    args.val = pump_data_head;
    read_pump_data_head_from_flash(args);  /// 读取泵奶数据头
    args.val = &ctx->priv.data_bury_info;
    data_bury_infor_storage_init(args);    /// 读取 Momcozy 埋点信息 + 环
    msleep(10);
    args.val = &ctx->priv.img_res_ota_info;
    read_img_res_ota_info_from_flash(args);  /// 读取图片资源OTA信息
#if IS_SUPPORT_LANG_PACK
    args.val = &ctx->priv.lang_pack_ota_info;
    read_lang_pack_ota_info_from_flash(args);  /// 读取语言包OTA断点（不碰 FDS_EXT 语言文件）
#endif
    ctx->priv.sys_info_crc[STORE_CRC_TYPE_SYS_CONFIG] = CRC16(&sys_info->config, sizeof(sys_config_info_t));
    ctx->priv.sys_info_crc[STORE_CRC_TYPE_SYS_STATE]  = CRC16(&sys_info->state, sizeof(sys_state_info_t));
    ctx->priv.sys_info_crc[STORE_CRC_TYPE_PUMP_INFO]  = CRC16(&sys_info->pump_info, sizeof(sys_pump_info_t));
    ctx->priv.sys_info_crc[STORE_CRC_TYPE_PUMP_MODE_STEPS_INFO]  = CRC16(&sys_info->mode_info, sizeof(sys_pump_mode_info_t));
    ctx->priv.sys_info_crc[STORE_CRC_TYPE_PUMP_BURY_INFO]  = CRC16(&sys_info->buried_info_list, sizeof(sys_buried_info_list_t));
    ctx->priv.sys_info_crc[STORE_CRC_TYPE_PUMP_USER_DATA_HEAD]  = CRC16(pump_data_head, sizeof(breast_pump_data_head_t));
    ctx->priv.sys_info_crc[STORE_CRC_TYPE_IMG_RES_OTA_INFO]  = CRC16(&ctx->priv.img_res_ota_info, sizeof(img_res_ota_info_t));
#if IS_SUPPORT_LANG_PACK
    ctx->priv.sys_info_crc[STORE_CRC_TYPE_LANG_PACK_OTA_INFO] = CRC16(&ctx->priv.lang_pack_ota_info, sizeof(img_res_ota_info_t));
#endif
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
        if (write_sys_config_info_to_flash(args) == 0) {
            ctx->priv.sys_info_crc[STORE_CRC_TYPE_SYS_CONFIG] = info_crc;
        }
        LOG_STORE("-->system config info sync flash");
    }
    ///  系统状态
    info_crc = CRC16(&sys_info->state, sizeof(sys_state_info_t));
    if (info_crc != ctx->priv.sys_info_crc[STORE_CRC_TYPE_SYS_STATE]) {
        args.val = &sys_info->state;
        if (write_sys_state_info_to_flash(args) == 0) {
            ctx->priv.sys_info_crc[STORE_CRC_TYPE_SYS_STATE] = info_crc;
        }
        LOG_STORE("-->system state info sync flash");
    }
    ///  泵奶信息
    info_crc = CRC16(&sys_info->pump_info, sizeof(sys_pump_info_t));
    if (info_crc != ctx->priv.sys_info_crc[STORE_CRC_TYPE_PUMP_INFO]) {
        args.val = &sys_info->pump_info;
        if (write_pump_info_to_flash(args) == 0) {
            ctx->priv.sys_info_crc[STORE_CRC_TYPE_PUMP_INFO] = info_crc;
        }
        LOG_STORE("-->pump info sync flash");
    }
    ///  泵奶模式步骤信息
    info_crc = CRC16(&sys_info->mode_info, sizeof(sys_pump_mode_info_t));
    if (info_crc != ctx->priv.sys_info_crc[STORE_CRC_TYPE_PUMP_MODE_STEPS_INFO]) {
        args.val = &sys_info->mode_info;
        if (write_pump_mode_steps_info_to_flash(args) == 0) {
            ctx->priv.sys_info_crc[STORE_CRC_TYPE_PUMP_MODE_STEPS_INFO] = info_crc;
        }
        LOG_STORE("-->pump mode steps info sync flash");
    }
    ///  泵奶埋点信息
    info_crc = CRC16(&sys_info->buried_info_list, sizeof(sys_buried_info_list_t));
    if (info_crc != ctx->priv.sys_info_crc[STORE_CRC_TYPE_PUMP_BURY_INFO]) {
        args.val = &sys_info->buried_info_list;
        if (write_pump_buried_info_to_flash(args) == 0) {
            ctx->priv.sys_info_crc[STORE_CRC_TYPE_PUMP_BURY_INFO] = info_crc;
        }
        LOG_STORE("-->pump buried info sync flash");
    }
    ///  图片资源OTA信息
    info_crc = CRC16(&ctx->priv.img_res_ota_info, sizeof(img_res_ota_info_t));
    if (info_crc != ctx->priv.sys_info_crc[STORE_CRC_TYPE_IMG_RES_OTA_INFO]) {
        args.val = &ctx->priv.img_res_ota_info;
        if (write_img_res_ota_info_to_flash(args) == 0) {
            ctx->priv.sys_info_crc[STORE_CRC_TYPE_IMG_RES_OTA_INFO] = info_crc;
        }
        LOG_STORE("-->img res ota info sync flash");
    }
#if IS_SUPPORT_LANG_PACK
    ///  语言包OTA断点
    info_crc = CRC16(&ctx->priv.lang_pack_ota_info, sizeof(img_res_ota_info_t));
    if (info_crc != ctx->priv.sys_info_crc[STORE_CRC_TYPE_LANG_PACK_OTA_INFO]) {
        args.val = &ctx->priv.lang_pack_ota_info;
        if (write_lang_pack_ota_info_to_flash(args) == 0) {
            ctx->priv.sys_info_crc[STORE_CRC_TYPE_LANG_PACK_OTA_INFO] = info_crc;
        }
        LOG_STORE("-->lang pack ota info sync flash");
    }
#endif
    return 0;
}

static int user_database_reset(void)
{
    EXCEPTION_CHECK_INT(thiz);
    reset_pump_info(thiz, &thiz->priv.sys_info.pump_info);  /// 重置泵奶配置信息
    msleep(10);
    reset_pump_mode_steps_info(thiz, &thiz->priv.sys_info.mode_info);  /// 重置泵奶模式步骤信息
    msleep(10);
    reset_user_bind_info(thiz, &thiz->priv.sys_info.bind_info);  /// 重置用户绑定信息
    msleep(10);
    reset_sys_config_info(thiz, &thiz->priv.sys_info.config);  /// 重置系统配置信息
    msleep(10);
    reset_sys_state_info(thiz, &thiz->priv.sys_info.state);  /// 重置系统状态信息
    msleep(10);
    reset_pump_data_head(thiz, &thiz->priv.pump_data_head);  /// 重置泵奶数据头
    msleep(10);
    reset_pump_buried_info(thiz, &thiz->priv.sys_info.buried_info_list);  /// 重置泵奶埋点信息
    msleep(10);
    data_bury_ring_storage_reset(thiz);                                   /// 重置 Momcozy 埋点环
    msleep(10);
    reset_img_res_ota_info(thiz, &thiz->priv.img_res_ota_info);  /// 重置图片资源OTA信息
    LOGW("==> user database reset !!!!!!!!!!!!!!!");
    return 0;
}

static void hdr_storage_work_cb(hdr_post_args_t *args)
{
    if (args->arg1 == 0) {
        system_persist_info_sync_flash(thiz);
    }
}

static void on_power_off_force_save_route(storage_manager_context_t *ctx)
{
    EXCEPTION_CHECK_VOID(ctx);
    /// 同步电池电量
    battery_manager_context_t *battery = (battery_manager_context_t *)find_module(ID_MODULE_BATTERY);
    EXCEPTION_CHECK_VOID(battery);
    uint32_t percent = battery->get_battery_percent();
    ctx->priv.sys_info.state.battery_percent = percent;
    system_persist_info_sync_flash(ctx);
    if (data_bury_ring_sync_to_flash(ctx) == 0) {
        LOG_STORE("-->data bury ring sync flash on power off");
    }
    LOG_STORE("-->power off force save success");
}

static void pump_user_data_sync_flash_cb(hdr_post_args_t *args)
{
    EXCEPTION_CHECK_VOID(args);
    EXCEPTION_CHECK_VOID(thiz);
    breast_pump_data_head_t *pump_data_head = &thiz->priv.pump_data_head;
    uint8_t write_index;
    if (pump_data_head->num < ROOT_BREAST_PUMP_DATA_BODY_NUM) {
        /// 缓冲区未满，下一条写到末尾
        write_index = pump_data_head->num;
    } else {
        /// 缓冲区已满，覆盖最旧数据
        write_index = pump_data_head->head;
    }

    const pump_result_info_t *result_info = (pump_result_info_t *)args->user_data;
    if (result_info == NULL) {
        breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
        EXCEPTION_CHECK_VOID(pump);
        READ_PUMP_RESULT_INFO(pump, BREAST_PUMP_RESULT_PUMPING, &result_info);
    }
    EXCEPTION_CHECK_VOID(result_info);

    store_func_args_t storage_args = {
        .ctx = thiz,
        .arg1 = write_index,
        .arg2 = 0,
        .val = (void *)result_info,
    };
    /// 先写入body，成功后再更新head，避免半更新状态
    int ret = write_pump_data_body_to_flash(storage_args);
    if (ret != 0) {
        LOGW("write pump data body failure, retry once. ret: %d, index: %d", ret, write_index);
        ret = write_pump_data_body_to_flash(storage_args);
        if (ret != 0) {
            LOGE("write pump data body failure! ret: %d, index: %d", ret, write_index);
            return;
        }
    }

    breast_pump_data_head_t head_backup = *pump_data_head;

    /// 更新泵奶数据头 - 实现循环缓冲区逻辑
    if (pump_data_head->num < ROOT_BREAST_PUMP_DATA_BODY_NUM) {
        pump_data_head->num += 1;
        pump_data_head->tail = write_index;
        if (pump_data_head->num == 1) {
            pump_data_head->head = 0;
        }
    } else {
        pump_data_head->head = (pump_data_head->head + 1) % ROOT_BREAST_PUMP_DATA_BODY_NUM;
        pump_data_head->tail = write_index;
        pump_data_head->num = ROOT_BREAST_PUMP_DATA_BODY_NUM;
    }

    storage_args.arg1 = 0;
    storage_args.val = pump_data_head;
    /// 写入泵奶数据头
    ret = write_pump_data_head_to_flash(storage_args);
    if (ret != 0) {
        LOGW("write pump data head failure, retry once. ret: %d", ret);
        ret = write_pump_data_head_to_flash(storage_args);
        if (ret != 0) {
            LOGE("write pump data head failure! ret: %d", ret);
            /// 回滚内存中的head状态，并删除刚写入的body，避免半更新
            *pump_data_head = head_backup;
            store_func_args_t del_args = {
                .ctx = thiz,
                .arg1 = write_index,
                .arg2 = 0,
                .val = NULL,
            };
            int del_ret = del_pump_data_body_from_flash(del_args);
            if (del_ret != 0) {
                LOGE("rollback pump data body failure! ret: %d, index: %d", del_ret, write_index);
            }
        }
    }
}

static void storage_ram_sync_flash_cb(hdr_post_args_t *args)
{
    breast_pump_manager_context_t *pump = (breast_pump_manager_context_t *)find_module(ID_MODULE_BREAST_PUMP);
    EXCEPTION_CHECK_VOID(pump);
    storage_manager_context_t *st = args->user_data;
    EXCEPTION_CHECK_VOID(st);
    int is_pump_working = 0; 
    READ_PUMP_IS_WORKING(pump, is_pump_working);
    if (is_pump_working) {
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

    switch (id.code) {
        case ID_BROADCAST_PM_STATE_CHANGE:
            if (arg1 == PM_BROADCAST_SYS_SUSPEND) {
                REMOVE_TASK_HANDLER_DIRECTLY(STORAGE_RAM_SYNC_FLASH_TASK_NAME);
                POST_TASK_HANDLER_DELAYED_ARG2(hdr_storage_work_cb, 500, 0, st, STORAGE_PERSIST_TASK_POLICY1_NAME);
            } else if (arg1 == PM_BROADCAST_SYS_RESUME) {
                POST_TASK_HANDLER_DELAYED_ARG1(storage_ram_sync_flash_cb, 
                    STORAGE_RAM_SYNC_FLASH_INTERVAL_MS, st, STORAGE_RAM_SYNC_FLASH_TASK_NAME);
            }
            break;
        case ID_BROADCAST_PERSIST_INFO_UPDATE: {
            pump_result_info_t *user_data = NULL;    
            PRIV_DATA_TO_USER_DATA(user_data, pdata, pump_result_info_t);
            if (user_data) {
                LOG_STORE("-->save insert_one");
                POST_TASK_HANDLER_DELAYED_ARG2(pump_user_data_sync_flash_cb, 0, 1, user_data, STORAGE_PERSIST_TASK_POLICY1_NAME);
            }
            break;
        }
        case ID_BROADCAST_SYS_STATE_CHANGE:
            if (arg1 == SYSTEM_BROADCAST_POWER_OFF) {
                LOG_STORE("-->system shutdown");
                POST_TASK_HANDLER_DELAYED_ARG2(hdr_storage_work_cb, 0, 0, st, STORAGE_PERSIST_TASK_POLICY1_NAME);
            } else if (arg1 == SYSTEM_BROADCAST_POWER_ON) {
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
    ctx->hook->on_power_off_force_save_route = on_power_off_force_save_route;

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

 