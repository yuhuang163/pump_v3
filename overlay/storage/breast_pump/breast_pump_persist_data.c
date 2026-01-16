/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "storage_manager.h" // IWYU pragma: keep
#include "storage_manager_internal.h" // IWYU pragma: keep
#include "breast_pump_mode.h" // IWYU pragma: keep
#include "persist_storage.h" // IWYU pragma: keep
#include "breast_pump_persist_data.h" // IWYU pragma: keep

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

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int write_pump_info_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = (sys_pump_info_t *)args.val;
    EXCEPTION_CHECK_INT(pump_info);
    breast_pump_info_storage_t *store_pump_info = u_malloz(sizeof(breast_pump_info_storage_t));
    EXCEPTION_CHECK_INT(store_pump_info);
    store_pump_info->magic = STORAGE_BREAST_PUMP_INFO_MAGIC_NUM;
    memcpy(&store_pump_info->pump_info, pump_info, sizeof(sys_pump_info_t));
    store_pump_info->crc = CRC16(&store_pump_info->pump_info, sizeof(sys_pump_info_t));
    ret = user_database_put(args.ctx, PERSIST_KEY_BREAST_PUMP_INFO, (uint8_t *)store_pump_info,
                    sizeof(breast_pump_info_storage_t));
    if (ret != 0) {
        LOGE("write pump info failure! ret: %d", ret);
    }
    u_free(store_pump_info);

    return ret;
}

void reset_pump_info(storage_manager_context_t *ctx, sys_pump_info_t *pump_info)
{
    const motor_policy_cfg_t *cfg = NULL;
    pump_info->is_remind_exception           = 1;
    pump_info->is_remind_mode_change         = 1;
    pump_info->is_remind_pump_finish         = 1;
    pump_info->control_type                  = BREAST_PUMP_CONTROL_TYPE_LEFT;
    for (int i = 0; i < BREAST_PUMP_MODE_MAX; i++) {
        pump_info->pump_time[i] = breast_pump_mode_work_time_get(i);
        breast_pump_setting_reset(pump_info, i);
    }
    breast_pump_custom_step_para_reset(pump_info);
    EXCEPTION_CHECK_VOID(ctx);
    store_func_args_t args = {
        .ctx = ctx,
        .arg1 = 0,
        .arg2 = 0,
        .val = pump_info,
    };
    LOG_STORE_WRAP("==> storage_reset_pump_info !!!!!!!!!!!!!!!");
    write_pump_info_to_flash(args);
}

int read_pump_info_from_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = (sys_pump_info_t *)args.val;
    EXCEPTION_CHECK_INT(pump_info);
    breast_pump_info_storage_t *store_pump_info = u_malloz(sizeof(breast_pump_info_storage_t));
    EXCEPTION_CHECK_INT(store_pump_info);
    ret = user_database_get(args.ctx, PERSIST_KEY_BREAST_PUMP_INFO, (uint8_t *)store_pump_info,
                    sizeof(breast_pump_info_storage_t));
    if (ret == 0) {
        memcpy(pump_info, &store_pump_info->pump_info, sizeof(sys_pump_info_t));
    } else {
        reset_pump_info(args.ctx, pump_info);
        LOGE("read pump info failure! ret: %d", ret);
    }
    u_free(store_pump_info);

    return ret;
}

int write_pump_data_head_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    breast_pump_data_head_t *pump_data_head = (breast_pump_data_head_t *)args.val;
    EXCEPTION_CHECK_INT(pump_data_head);
    breast_pump_data_head_storage_t *store_pump_data_head = u_malloz(sizeof(breast_pump_data_head_storage_t));
    EXCEPTION_CHECK_INT(store_pump_data_head);
    store_pump_data_head->magic = STORAGE_BREAST_PUMP_DATA_HEAD_MAGIC_NUM;
    memcpy(&store_pump_data_head->pump_data_head, pump_data_head, sizeof(breast_pump_data_head_t));
    store_pump_data_head->crc = CRC16(&store_pump_data_head->pump_data_head, sizeof(breast_pump_data_head_t));
    ret = user_database_put(args.ctx, PERSIST_KEY_BREAST_PUMP_DATA_HEAD, (uint8_t *)store_pump_data_head,
                sizeof(breast_pump_data_head_storage_t));
    if (ret != 0) {
        LOGE("write pump data head failure! ret: %d", ret);
    }
    u_free(store_pump_data_head);

    return ret;
}

int write_pump_buried_info_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    sys_buried_info_list_t *buried_info_list = (sys_buried_info_list_t *)args.val;
    EXCEPTION_CHECK_INT(buried_info_list);
    breast_pump_buried_info_storage_t *store_buried_info = u_malloz(sizeof(breast_pump_buried_info_storage_t));
    EXCEPTION_CHECK_INT(store_buried_info);
    store_buried_info->magic = STORAGE_BREAST_PUMP_BURY_INFO_MAGIC_NUM;
    memcpy(&store_buried_info->buried_info_list, buried_info_list, sizeof(sys_buried_info_list_t));
    store_buried_info->crc = CRC16(&store_buried_info->buried_info_list, sizeof(sys_buried_info_list_t));
    ret = user_database_put(args.ctx, PERSIST_KEY_BREAST_PUMP_BURY_INFO, (uint8_t *)store_buried_info,
                sizeof(breast_pump_buried_info_storage_t));
    if (ret != 0) {
        LOGE("write pump buried info failure! ret: %d", ret);
    }
    u_free(store_buried_info);

    return ret;
}

void reset_pump_buried_info(storage_manager_context_t *ctx, sys_buried_info_list_t *buried_info_list)
{
    EXCEPTION_CHECK_VOID(ctx);
    memset(buried_info_list, 0, sizeof(sys_buried_info_list_t));
    store_func_args_t args = {
        .ctx = ctx,
        .arg1 = 0,
        .arg2 = 0,
        .val = buried_info_list,
    };
    LOG_STORE_WRAP("==> storage_reset_pump_buried_info !!!!!!!!!!!!!!!");
    write_pump_buried_info_to_flash(args);
}

int read_pump_buried_info_from_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    sys_buried_info_list_t *buried_info_list = (sys_buried_info_list_t *)args.val;
    EXCEPTION_CHECK_INT(buried_info_list);
    breast_pump_buried_info_storage_t *store_buried_info = u_malloz(sizeof(breast_pump_buried_info_storage_t));
    EXCEPTION_CHECK_INT(store_buried_info);
    ret = user_database_get(args.ctx, PERSIST_KEY_BREAST_PUMP_BURY_INFO, (uint8_t *)store_buried_info,
                    sizeof(breast_pump_buried_info_storage_t));
    if (ret == 0) {
        memcpy(buried_info_list, &store_buried_info->buried_info_list, sizeof(sys_buried_info_list_t));
    } else {
        reset_pump_buried_info(args.ctx, buried_info_list);
        LOGE("read pump buried info failure! ret: %d", ret);
    }
    u_free(store_buried_info);

    return ret;
}

void reset_pump_data_head(storage_manager_context_t *ctx, breast_pump_data_head_t *pump_data_head)
{
    EXCEPTION_CHECK_VOID(ctx);
    memset(pump_data_head, 0, sizeof(breast_pump_data_head_t));
    store_func_args_t args = {
        .ctx = ctx,
        .arg1 = 0,
        .arg2 = 0,
        .val = pump_data_head,
    };
    write_pump_data_head_to_flash(args);
}

int read_pump_data_head_from_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    breast_pump_data_head_t *pump_data_head = (breast_pump_data_head_t *)args.val;
    EXCEPTION_CHECK_INT(pump_data_head);
    breast_pump_data_head_storage_t *store_pump_data_head = u_malloz(sizeof(breast_pump_data_head_storage_t));
    EXCEPTION_CHECK_INT(store_pump_data_head);
    ret = user_database_get(args.ctx, PERSIST_KEY_BREAST_PUMP_DATA_HEAD, (uint8_t *)store_pump_data_head,
                            sizeof(breast_pump_data_head_storage_t));
    if (ret == 0) {
        memcpy(pump_data_head, &store_pump_data_head->pump_data_head, sizeof(breast_pump_data_head_t));
    } else {
        memset(pump_data_head, 0, sizeof(breast_pump_data_head_t));
        LOGE("read pump data head failure! ret: %d", ret);
    }
    u_free(store_pump_data_head);

    return ret;
}

int write_pump_data_body_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    uint16_t body_offset = args.arg1;
    if (body_offset >= ROOT_BREAST_PUMP_DATA_BODY_NUM) {
        LOGE("body_offset: %d, out of range", body_offset);
        return -1;
    }

    pump_result_info_t *pump_data_body = (pump_result_info_t *)args.val;
    EXCEPTION_CHECK_INT(pump_data_body);
    breast_pump_body_storage_t *store_pump_data_body = u_malloz(sizeof(breast_pump_body_storage_t));
    EXCEPTION_CHECK_INT(store_pump_data_body);
    store_pump_data_body->magic = STORAGE_BREAST_PUMP_DATA_BODY_MAGIC_NUM;
    memcpy(&store_pump_data_body->pump_data_body, pump_data_body, sizeof(pump_result_info_t));
    store_pump_data_body->crc = CRC16(&store_pump_data_body->pump_data_body, sizeof(pump_result_info_t));
    ret = user_database_put(args.ctx, PERSIST_KEY_BREAST_PUMP_DATA_BODY_BEGIN + body_offset, 
                            (uint8_t *)store_pump_data_body, sizeof(breast_pump_body_storage_t));
    if (ret != 0) {
        LOGE("write pump data body failure! ret: %d", ret);
    }
    u_free(store_pump_data_body);

    return ret;
}

int read_pump_data_body_from_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    uint16_t body_offset = args.arg1;
    if (body_offset >= ROOT_BREAST_PUMP_DATA_BODY_NUM) {
        LOGE("body_offset: %d, out of range", body_offset);
        return -1;
    }
    pump_result_info_t *pump_data_body = (pump_result_info_t *)args.val;
    EXCEPTION_CHECK_INT(pump_data_body);
    breast_pump_body_storage_t *store_pump_data_body = u_malloz(sizeof(breast_pump_body_storage_t));
    EXCEPTION_CHECK_INT(store_pump_data_body);
    ret = user_database_get(args.ctx, PERSIST_KEY_BREAST_PUMP_DATA_BODY_BEGIN + body_offset, 
                            (uint8_t *)store_pump_data_body, sizeof(breast_pump_body_storage_t));
    if (ret == 0) {
        memcpy(pump_data_body, &store_pump_data_body->pump_data_body, sizeof(pump_result_info_t));
    } else {
        LOGE("read pump data body failure! ret: %d, body_offset: %d", ret, body_offset);
    }
    u_free(store_pump_data_body);

    return ret;
}

int del_pump_data_body_from_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    uint16_t body_offset = args.arg1;
    if (body_offset >= ROOT_BREAST_PUMP_DATA_BODY_NUM) {
        LOGE("body_offset: %d, out of range", body_offset);
        return -1;
    }

    ret = user_database_remove(args.ctx, PERSIST_KEY_BREAST_PUMP_DATA_BODY_BEGIN + body_offset);
    if (ret != 0) {
        LOGE("del pump data body failure! ret: %d, body_offset: %d", ret, body_offset);
    }

    return ret;
}

int write_pump_time_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    pump_info->pump_time[args.arg1] = (uint16_t)args.arg2;
    LOG_STORE_WRAP("write pump mode: %d, time: %d", args.arg1, args.arg2);
    return 0;
}

int write_pump_exception_remind_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    pump_info->is_remind_exception = (uint8_t)args.arg1;
    LOG_STORE_WRAP("write pump exception remind: %d", args.arg1);
    return 0;
}

int write_pump_mode_change_remind_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    pump_info->is_remind_mode_change = (uint8_t)args.arg1;
    LOG_STORE_WRAP("write pump mode change remind: %d", args.arg1);
    return 0;
}

int write_pump_finish_remind_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    pump_info->is_remind_pump_finish = (uint8_t)args.arg1;
    LOG_STORE_WRAP("write pump finish remind: %d", args.arg1);
    return 0;
}

int read_pump_time_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    LOG_STORE_WRAP("read pump mode: %d, time: %d", args.arg1, pump_info->pump_time[args.arg1]);
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = pump_info->pump_time[args.arg1];
    return 0;
}

int read_pump_exception_remind_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = pump_info->is_remind_exception;
    return 0;
}

int read_pump_mode_change_remind_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = pump_info->is_remind_mode_change;
    return 0;
}

int read_pump_finish_remind_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = pump_info->is_remind_pump_finish;
    return 0;
}

int write_pump_custom_steps_parameter_to_ram(store_func_args_t args)
{
    struct storage_manager_context *st = args.ctx;
    EXCEPTION_CHECK_INT(st);
    user_def_pump_steps_data_t *custom_steps_parameter = (user_def_pump_steps_data_t *)args.val;
    EXCEPTION_CHECK_INT(custom_steps_parameter);
    sys_pump_info_t *pump_info = &st->priv.sys_info.pump_info;

    memcpy(&pump_info->custom_steps_parameter, custom_steps_parameter, sizeof(user_def_pump_steps_data_t));
    return 0;
}

int read_pump_custom_steps_parameter_from_ram(store_func_args_t args)
{
    struct storage_manager_context *st = args.ctx;
    EXCEPTION_CHECK_INT(st);
    sys_pump_info_t *pump_info = &st->priv.sys_info.pump_info;
    user_def_pump_steps_data_t *custom_steps_parameter = (user_def_pump_steps_data_t *)args.val;
    EXCEPTION_CHECK_INT(custom_steps_parameter);
    // copy from ram
    memcpy(custom_steps_parameter, &pump_info->custom_steps_parameter, sizeof(user_def_pump_steps_data_t));
    return 0;
}

int write_pump_setting_info_to_ram(store_func_args_t args)
{
    struct storage_manager_context *st = args.ctx;
    EXCEPTION_CHECK_INT(st);
    pump_setting_t *setting = (pump_setting_t *)args.val;
    EXCEPTION_CHECK_INT(setting);
    sys_pump_info_t *pump_info = &st->priv.sys_info.pump_info;
    LOG_STORE("write pump setting info: %d, %d", args.arg1, args.arg2);
    uint8_t mode = args.arg1;
    uint8_t mode_step = args.arg2;
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            if (mode_step >= ROOT_BREAST_PUMP_STEP_MAX) {
                memcpy(pump_info->stimulation_setting, setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
            } else {
                memcpy(&pump_info->stimulation_setting[mode_step], setting, sizeof(pump_setting_t));
            }
            break;
        case BREAST_PUMP_LACTATION_MODE:
            if (mode_step >= ROOT_BREAST_PUMP_STEP_MAX) {
                memcpy(pump_info->pump_setting, setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
            } else {
                memcpy(&pump_info->pump_setting[mode_step], setting, sizeof(pump_setting_t));
            }
            break;
        case BREAST_PUMP_MIX_MODE:
            if (mode_step >= ROOT_BREAST_PUMP_STEP_MAX) {
                memcpy(pump_info->mix_setting, setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
            } else {
                memcpy(&pump_info->mix_setting[mode_step], setting, sizeof(pump_setting_t));
            }
            break;
        case BREAST_PUMP_CUSTOM_MODE:
            if (mode_step >= ROOT_BREAST_PUMP_STEP_MAX) {
                memcpy(pump_info->custom_setting, setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
            } else {
                memcpy(&pump_info->custom_setting[mode_step], setting, sizeof(pump_setting_t));
            }
            break;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            if (mode_step >= ROOT_BREAST_PUMP_STEP_MAX) {
                memcpy(pump_info->pp_milk_track_setting, setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
            } else {
                memcpy(&pump_info->pp_milk_track_setting[mode_step], setting, sizeof(pump_setting_t));
            }
            break;
        case BREAST_PUMP_MILK_BOOST_MODE:
            if (mode_step >= ROOT_BREAST_PUMP_STEP_MAX) {
                memcpy(pump_info->milk_boost_setting, setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
            } else {
                memcpy(&pump_info->milk_boost_setting[mode_step], setting, sizeof(pump_setting_t));
            }
            break;
        case BREAST_PUMP_LACTATION_START_MODE:
            if (mode_step >= ROOT_BREAST_PUMP_STEP_MAX) {
                memcpy(pump_info->lactation_start_setting, setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
            } else {
                memcpy(&pump_info->lactation_start_setting[mode_step], setting, sizeof(pump_setting_t));
            }
            break;
        default:
            LOGE("write pump setting info: %d, out of range", args.arg1);
            return -1;
    }
    return 0;
}

int read_pump_setting_info_from_ram(store_func_args_t args)
{
    struct storage_manager_context *st = args.ctx;
    EXCEPTION_CHECK_INT(st);
    sys_pump_info_t *pump_info = &st->priv.sys_info.pump_info;
    pump_setting_t *setting = (pump_setting_t *)args.val;
    EXCEPTION_CHECK_INT(setting);
    LOG_STORE("read pump setting info: %d, %d", args.arg1, args.arg2);
    uint8_t mode = args.arg1;
    uint8_t mode_step = args.arg2;
    switch (mode) {
        case BREAST_PUMP_STIMULATION_MODE:
            if (mode_step >= ROOT_BREAST_PUMP_STEP_MAX) {
                memcpy(setting, pump_info->stimulation_setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
            } else {
                memcpy(setting, &pump_info->stimulation_setting[mode_step], sizeof(pump_setting_t));
            }
            break;
        case BREAST_PUMP_LACTATION_MODE:
            if (mode_step >= ROOT_BREAST_PUMP_STEP_MAX) {
                memcpy(setting, pump_info->pump_setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
            } else {
                memcpy(setting, &pump_info->pump_setting[mode_step], sizeof(pump_setting_t));
            }
            break;
        case BREAST_PUMP_MIX_MODE:
            if (mode_step >= ROOT_BREAST_PUMP_STEP_MAX) {
                memcpy(setting, pump_info->mix_setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
            } else {
                memcpy(setting, &pump_info->mix_setting[mode_step], sizeof(pump_setting_t));
            }
            break;
        case BREAST_PUMP_CUSTOM_MODE:
            if (mode_step >= ROOT_BREAST_PUMP_STEP_MAX) {
                memcpy(setting, pump_info->custom_setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
            } else {
                memcpy(setting, &pump_info->custom_setting[mode_step], sizeof(pump_setting_t));
            }
            break;
        case BREAST_PUMP_PP_MILK_TRACK_MODE:
            if (mode_step >= ROOT_BREAST_PUMP_STEP_MAX) {
                memcpy(setting, pump_info->pp_milk_track_setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
            } else {
                memcpy(setting, &pump_info->pp_milk_track_setting[mode_step], sizeof(pump_setting_t));
            }
            break;
        case BREAST_PUMP_MILK_BOOST_MODE:
            if (mode_step >= ROOT_BREAST_PUMP_STEP_MAX) {
                memcpy(setting, pump_info->milk_boost_setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
            } else {
                memcpy(setting, &pump_info->milk_boost_setting[mode_step], sizeof(pump_setting_t));
            }
            break;
        case BREAST_PUMP_LACTATION_START_MODE:  
            if (mode_step >= ROOT_BREAST_PUMP_STEP_MAX) {
                memcpy(setting, pump_info->lactation_start_setting, sizeof(pump_setting_t) * ROOT_BREAST_PUMP_STEP_MAX);
            } else {
                memcpy(setting, &pump_info->lactation_start_setting[mode_step], sizeof(pump_setting_t));
            }
            break;
        default:
            LOGE("read pump setting info: %d, out of range ", args.arg1);
            return -1;
    }
    return 0;
}

int write_pump_control_type_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    pump_info->control_type = *(uint8_t *)args.val;
    LOG_STORE_WRAP("write pump control type: %d", pump_info->control_type);
    return 0;
}

int read_pump_control_type_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = pump_info->control_type;
    return 0;
}

int write_pump_buried_info_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    uint16_t buried_index = args.ctx->priv.sys_info.buried_info_list.buried_index;
    if (buried_index >= ROOT_BREAST_PUMP_BURY_DATA_NUM) {
        LOGE("buried_index: %d, out of range, reset to 0", buried_index);
        buried_index = 0;
    }
    sys_buried_info_t *buried_info = (sys_buried_info_t *)args.val;
    sys_buried_info_list_t *buried_info_list = &args.ctx->priv.sys_info.buried_info_list;
    EXCEPTION_CHECK_INT(buried_info_list);
    memcpy(&buried_info_list->buried_info[buried_index], buried_info, sizeof(sys_buried_info_t));
    // 增加索引并立即回绕，避免存储越界索引
    args.ctx->priv.sys_info.buried_info_list.buried_index = (buried_index + 1) % ROOT_BREAST_PUMP_BURY_DATA_NUM;
    return 0;
}

/// 测试刷牙数据存储
int fill_in_pump_data_info(int num)
{
    return 0;
}

