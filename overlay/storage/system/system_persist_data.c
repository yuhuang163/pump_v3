/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "storage_manager_internal.h"

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int write_sys_config_info_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    sys_config_info_t *sys_config = (sys_config_info_t *)args.val;
    EXCEPTION_CHECK_INT(sys_config);
    sys_config_storage_t *store_sys_config = u_malloz(sizeof(sys_config_storage_t));
    EXCEPTION_CHECK_INT(store_sys_config);
    store_sys_config->magic = STORAGE_SYS_CONFIG_MAGIC_NUM;
    memcpy(&store_sys_config->config, sys_config, sizeof(sys_config_info_t));
    store_sys_config->crc = CRC16(&store_sys_config->config, sizeof(sys_config_info_t));
    ret = user_database_put(args.ctx, PERSIST_KEY_SYS_CONFIG_DATA, (uint8_t *)store_sys_config,
        sizeof(sys_config_storage_t));
    if (ret != 0) {
        LOGE("write sys config failure! ret: %d", ret);
    }
    u_free(store_sys_config);

    return ret;
}

void reset_sys_config_info(storage_manager_context_t *ctx, sys_config_info_t *sys_config)
{
    EXCEPTION_CHECK_VOID(sys_config);
    memset(sys_config, 0, sizeof(sys_config_info_t));
    sys_config->time_zone              = 0;
    sys_config->ap_mode_onoff          = 1;
    sys_config->slient_upgrade_onoff   = 0;
    sys_config->sys_ui_style           = 0;
    sys_config->log_switch             = 1;
    sys_config->language               = SYS_LANGUANGE_CHINESE;
    store_func_args_t args = {
        .ctx = ctx,
        .arg1 = 0,
        .arg2 = 0,
        .val = sys_config,
    };
    write_sys_config_info_to_flash(args);
}

int read_sys_config_info_from_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    sys_config_info_t *sys_config = (sys_config_info_t *)args.val;
    EXCEPTION_CHECK_INT(sys_config);
    sys_config_storage_t *store_sys_config = u_malloz(sizeof(sys_config_storage_t));
    EXCEPTION_CHECK_INT(store_sys_config);
    ret = user_database_get(args.ctx, PERSIST_KEY_SYS_CONFIG_DATA, (uint8_t *)store_sys_config,
        sizeof(sys_config_storage_t));
    uint16_t crc = CRC16(&store_sys_config->config, sizeof(sys_config_info_t));
    if (ret == 0 && store_sys_config->magic == STORAGE_SYS_CONFIG_MAGIC_NUM && store_sys_config->crc == crc) {
        memcpy(sys_config, &store_sys_config->config, sizeof(sys_config_info_t));
    } else {
        reset_sys_config_info(args.ctx, sys_config);
        LOGE("read sys config failure! ret: %d, magic: %d, crc: %d, expect_crc: %d", ret,
              store_sys_config->magic, store_sys_config->crc, crc);
    }
    u_free(store_sys_config);
    return ret;
}

int write_sys_state_info_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    sys_state_info_t *sys_state = (sys_state_info_t *)args.val;
    EXCEPTION_CHECK_INT(sys_state);
    sys_state_storage_t *store_sys_state = u_malloz(sizeof(sys_state_storage_t));
    EXCEPTION_CHECK_INT(store_sys_state);
    store_sys_state->magic = STORAGE_SYS_STATE_MAGIC_NUM;
    memcpy(&store_sys_state->state, sys_state, sizeof(sys_state_info_t));
    store_sys_state->crc = CRC16(&store_sys_state->state, sizeof(sys_state_info_t));
    ret = user_database_put(args.ctx, PERSIST_KEY_SYS_STATE_DATA, (uint8_t *)store_sys_state,
        sizeof(sys_state_storage_t));
    if (ret != 0) {
        LOGE("write sys state failure! ret: %d", ret);
    }
    u_free(store_sys_state);

    return ret;
}

void reset_sys_state_info(storage_manager_context_t *ctx, sys_state_info_t *sys_state)
{
    EXCEPTION_CHECK_VOID(sys_state);
    memset(sys_state, 0, sizeof(sys_state_info_t));
    sys_state->guide_deployed = 0;
    sys_state->sync_sys_time = 0;
    sys_state->battery_health = 100;
    sys_state->airplane_mode_state = 0;
    sys_state->night_light_adaptive_switch = 1; //默认开启自适应
    store_func_args_t args = {
        .ctx = ctx,
        .arg1 = 0,
        .arg2 = 0,
        .val = sys_state,
    };
    write_sys_state_info_to_flash(args);
}

int read_sys_state_info_from_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    sys_state_info_t *sys_state = (sys_state_info_t *)args.val;
    EXCEPTION_CHECK_INT(sys_state);
    sys_state_storage_t *store_sys_state = u_malloz(sizeof(sys_state_storage_t));
    EXCEPTION_CHECK_INT(store_sys_state);
    ret = user_database_get(args.ctx, PERSIST_KEY_SYS_STATE_DATA, (uint8_t *)store_sys_state,
        sizeof(sys_state_storage_t));
    uint16_t crc = CRC16(&store_sys_state->state, sizeof(sys_state_info_t));
    if (ret == 0 && store_sys_state->magic == STORAGE_SYS_STATE_MAGIC_NUM && store_sys_state->crc == crc) {
        memcpy(sys_state, &store_sys_state->state, sizeof(sys_state_info_t));
    } else {
        reset_sys_state_info(args.ctx, sys_state);
        LOGE("read sys state failure! ret: %d, magic: %d, crc: %d, expect_crc: %d", ret,
              store_sys_state->magic, store_sys_state->crc, crc);
    }
    u_free(store_sys_state);
    return ret;
}

int write_user_bind_info_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    user_bind_info_t *ram_bind_info = &args.ctx->priv.sys_info.bind_info;
    EXCEPTION_CHECK_INT(ram_bind_info);
    user_bind_info_t *bind_info = (user_bind_info_t *)args.val;
    EXCEPTION_CHECK_INT(bind_info);
    user_bind_storage_t *store_bind_info = u_malloz(sizeof(user_bind_storage_t));
    EXCEPTION_CHECK_INT(store_bind_info);
    store_bind_info->magic = STORAGE_USER_BIND_MAGIC_NUM;
    memcpy(&store_bind_info->bind_info, bind_info, sizeof(user_bind_info_t));
    store_bind_info->crc = CRC16(&store_bind_info->bind_info, sizeof(user_bind_info_t));
    ret = user_database_put(args.ctx, PERSIST_KEY_SYS_BIND_INFO, (uint8_t *)store_bind_info,
        sizeof(user_bind_storage_t));
    if (ret != 0) {
        LOGE("write user bind info failure! ret: %d", ret);
    } else {
        memcpy(ram_bind_info, bind_info, sizeof(user_bind_info_t));
    }
    u_free(store_bind_info);
 
    return ret;
}

void reset_user_bind_info(storage_manager_context_t *ctx, user_bind_info_t *user_bind)
{
    memset(user_bind, 0, sizeof(user_bind_info_t));
    store_func_args_t args = {
        .ctx = ctx,
        .arg1 = 0,
        .arg2 = 0,
        .val = user_bind,
    };
    write_user_bind_info_to_flash(args);
}

int read_user_bind_info_from_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    user_bind_info_t *bind_info = (user_bind_info_t *)args.val;
    EXCEPTION_CHECK_INT(bind_info);
    user_bind_storage_t *store_bind_info = u_malloz(sizeof(user_bind_storage_t));
    EXCEPTION_CHECK_INT(store_bind_info);
    ret = user_database_get(args.ctx, PERSIST_KEY_SYS_BIND_INFO, (uint8_t *)store_bind_info,
        sizeof(user_bind_storage_t));
    uint16_t crc = CRC16(&store_bind_info->bind_info, sizeof(user_bind_info_t));
    if (ret == 0 && store_bind_info->magic == STORAGE_USER_BIND_MAGIC_NUM && store_bind_info->crc == crc) {
        memcpy(bind_info, &store_bind_info->bind_info, sizeof(user_bind_info_t));
    } else {
        reset_user_bind_info(args.ctx, bind_info);
        LOGE("read user bind info failure! ret: %d, magic: %d, crc: %d, expect_crc: %d", ret,
             store_bind_info->magic, store_bind_info->crc, crc);
    }
    u_free(store_bind_info);
    return ret;
}

int write_utc_timestamp_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    utc_timestamp_storage_t *store_utc = u_malloz(sizeof(utc_timestamp_storage_t));
    EXCEPTION_CHECK_INT(store_utc);
    store_utc->magic = STORAGE_UTC_TIMESTAMP_MAGIC_NUM;
    store_utc->utc_timestamp = (uint32_t)args.arg1;
    store_utc->crc = CRC16(&store_utc->utc_timestamp, sizeof(uint32_t));
    ret = user_database_put(args.ctx, PERSIST_KEY_UTC_TIMESTEMP, (uint8_t *)store_utc,
        sizeof(utc_timestamp_storage_t));
    if (ret != 0) {
        LOGE("write utc timestamp failure! ret: %d", ret);
    }
    u_free(store_utc);
    return ret;
}

int read_utc_timestamp_from_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    uint32_t *tmp = (uint32_t *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    utc_timestamp_storage_t *store_utc = u_malloz(sizeof(utc_timestamp_storage_t));
    EXCEPTION_CHECK_INT(store_utc);
    ret = user_database_get(args.ctx, PERSIST_KEY_UTC_TIMESTEMP, (uint8_t *)store_utc,
        sizeof(utc_timestamp_storage_t));
    uint16_t crc = CRC16(&store_utc->utc_timestamp, sizeof(uint32_t));
    if (ret == 0 && store_utc->magic == STORAGE_UTC_TIMESTAMP_MAGIC_NUM && store_utc->crc == crc) {
        *tmp = store_utc->utc_timestamp;
    } else {
        *tmp = 0;
        LOGE("read utc timestamp failure! ret: %d, magic: %d, crc: %d, expect_crc: %d", ret, store_utc->magic, store_utc->crc, crc);
    }
    u_free(store_utc);

    return ret;
}

int write_sys_time_zone_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_config_info_t *sys_config = &args.ctx->priv.sys_info.config;
    sys_config->time_zone = (int8_t)args.arg1;
    LOG_STORE_WRAP("write sys time zone: %d", args.arg1);
    return 0;
}

int write_ap_mode_onoff_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_config_info_t *sys_config = &args.ctx->priv.sys_info.config;
    sys_config->ap_mode_onoff = (uint8_t)args.arg1;
    LOG_STORE_WRAP("write ap mode onoff: %d", args.arg1);
    return 0;
}

int write_slient_upgrade_onoff_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_config_info_t *sys_config = &args.ctx->priv.sys_info.config;
    sys_config->slient_upgrade_onoff = (uint8_t)args.arg1;
    LOG_STORE_WRAP("write slient upgrade onoff: %d", args.arg1);
    return 0;
}

int write_sys_ui_style_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_config_info_t *sys_config = &args.ctx->priv.sys_info.config;
    sys_config->sys_ui_style = (uint8_t)args.arg1;
    LOG_STORE_WRAP("write sys ui style: %d", args.arg1);
    return 0;
}

int write_log_switch_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_config_info_t *sys_config = &args.ctx->priv.sys_info.config;
    sys_config->log_switch = (uint8_t)args.arg1;
    LOG_STORE_WRAP("write sys log switch: %d", args.arg1);
    return 0;
}

int write_language_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_config_info_t *sys_config = &args.ctx->priv.sys_info.config;
    sys_config->language = (uint8_t)args.arg1;
    LOG_STORE_WRAP("write sys language: %d", args.arg1);
    return 0;
}

int write_guide_deployed_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_state_info_t *sys_state = &args.ctx->priv.sys_info.state;
    sys_state->guide_deployed = (uint8_t)args.arg1;
    LOG_STORE_WRAP("write guide deployed: %d", args.arg1);
    return 0;
}

int write_battery_health_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_state_info_t *sys_state = &args.ctx->priv.sys_info.state;
    sys_state->battery_health = (uint8_t)args.arg1;
    LOG_STORE_WRAP("write battery health: %d", args.arg1);
    return 0;
}

int write_airplane_mode_state_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_state_info_t *sys_state = &args.ctx->priv.sys_info.state;
    sys_state->airplane_mode_state = (uint8_t)args.arg1;
    LOG_STORE_WRAP("write airplane mode state: %d", args.arg1);
    return 0;
}

int write_night_light_adaptive_switch_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_state_info_t *sys_state = &args.ctx->priv.sys_info.state;
    sys_state->night_light_adaptive_switch = (uint8_t)args.arg1;
    LOG_STORE_WRAP("write night light adaptive switch: %d", args.arg1);
    return 0;
}   

int write_sync_sys_time_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_state_info_t *sys_state = &args.ctx->priv.sys_info.state;
    sys_state->sync_sys_time = (uint32_t)args.arg1;
    LOG_STORE_WRAP("write sync sys time: %d", args.arg1);
    return 0;
}

int write_log_filter_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_state_info_t *sys_state = &args.ctx->priv.sys_info.state;
    sys_state->log_filter = (uint32_t)args.arg1;
    LOG_STORE_WRAP("write sys log filter: %d", args.arg1);
    return 0;
}

int read_sys_time_zone_from_ram(store_func_args_t args)
{   
    EXCEPTION_CHECK_INT(args.ctx);
    sys_config_info_t *sys_config = &args.ctx->priv.sys_info.config;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = sys_config->time_zone;
    return 0;
}

int read_ap_mode_onoff_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_config_info_t *sys_config = &args.ctx->priv.sys_info.config;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = sys_config->ap_mode_onoff;
    return 0;
}

int read_slient_upgrade_onoff_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_config_info_t *sys_config = &args.ctx->priv.sys_info.config;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = sys_config->slient_upgrade_onoff;
    return 0;
}

int read_sys_ui_style_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_config_info_t *sys_config = &args.ctx->priv.sys_info.config;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = sys_config->sys_ui_style;
    return 0;
}

int read_log_switch_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_config_info_t *sys_config = &args.ctx->priv.sys_info.config;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = sys_config->log_switch;
    return 0;
}

int read_language_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_config_info_t *sys_config = &args.ctx->priv.sys_info.config;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = sys_config->language;
    return 0;
}

int read_guide_deployed_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_state_info_t *sys_state = &args.ctx->priv.sys_info.state;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = sys_state->guide_deployed;
    return 0;
}

int read_battery_health_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_state_info_t *sys_state = &args.ctx->priv.sys_info.state;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = sys_state->battery_health;
    return 0;
}

int read_airplane_mode_state_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_state_info_t *sys_state = &args.ctx->priv.sys_info.state;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = sys_state->airplane_mode_state;
    return 0;
}

int read_night_light_adaptive_switch_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_state_info_t *sys_state = &args.ctx->priv.sys_info.state;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = sys_state->night_light_adaptive_switch;
    return 0;
}

int read_sync_sys_time_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_state_info_t *sys_state = &args.ctx->priv.sys_info.state;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = sys_state->sync_sys_time;
    return 0;
}

int read_log_filter_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_state_info_t *sys_state = &args.ctx->priv.sys_info.state;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = sys_state->log_filter;
    return 0;
}

int user_is_binded_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    user_bind_info_t *bind_info = &args.ctx->priv.sys_info.bind_info;
    if (bind_info->user_id[0] && bind_info->random[0] && bind_info->aes_key[0]) {
        return 1;
    }
    return 0;    
}

int write_bind_status_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    user_bind_info_t *bind_info = &args.ctx->priv.sys_info.bind_info;
    bind_info->bind_status = (uint8_t)args.arg1;
    LOG_STORE_WRAP("write bind status: %d", args.arg1);
    return 0;
}

int read_bind_status_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    user_bind_info_t *bind_info = &args.ctx->priv.sys_info.bind_info;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = bind_info->bind_status;
    LOG_STORE_WRAP("read bind status: %d", *tmp);
    return 0;
}
