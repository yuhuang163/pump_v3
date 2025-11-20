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
        cfg = get_breast_pump_mode_para(i);
        if (cfg == NULL) {
            LOGE("cfg is NULL, mode: %d", i);
            continue;
        }
        pump_info->setting[i].speed = cfg->core.un.mapping.speed;
        pump_info->setting[i].level[0] = cfg->core.un.mapping.level;
        pump_info->setting[i].level[1] = cfg->core.un.mapping.level;
    }
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

int write_pump_setting_info_to_ram(store_func_args_t args)
{
    struct storage_manager_context *st = args.ctx;
    EXCEPTION_CHECK_INT(st);
    pump_setting_t *setting = (pump_setting_t *)args.val;
    EXCEPTION_CHECK_INT(setting);
    sys_pump_info_t *pump_info = &st->priv.sys_info.pump_info;
    LOG_STORE("write pump setting info: %d", args.arg1);
    uint8_t mode = args.arg1;
    if (mode >= BREAST_PUMP_MODE_MAX) {
        LOGE("write pump setting info: %d, out of range", args.arg1);
        return -1;
    }
    memcpy(&pump_info->setting[mode], setting, sizeof(pump_setting_t));
    return 0;
}

int read_pump_setting_info_from_ram(store_func_args_t args)
{
    struct storage_manager_context *st = args.ctx;
    EXCEPTION_CHECK_INT(st);
    sys_pump_info_t *pump_info = &st->priv.sys_info.pump_info;
    pump_setting_t *setting = (pump_setting_t *)args.val;
    EXCEPTION_CHECK_INT(setting);
    // LOG_STORE("read pump setting info: %d", args.arg1);
    uint8_t mode = args.arg1;
    if (mode >= BREAST_PUMP_MODE_MAX) {
        LOGE("read pump setting info: %d, out of range ", args.arg1);
        return -1;
    }
    memcpy(setting, &pump_info->setting[mode], sizeof(pump_setting_t));
    // LOG_STORE_WRAP("read pump setting info: %d, speed: %d, level: %d, %d",
    //      mode, setting->speed, setting->level[0], setting->level[1]);
    return 0;
}

int write_pump_control_type_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    pump_info->control_type = (uint8_t)args.arg1;
    LOG_STORE_WRAP("write pump control type: %d", args.arg1);
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

/// 测试刷牙数据存储
int fill_in_pump_data_info(int num)
{
    return 0;
}

