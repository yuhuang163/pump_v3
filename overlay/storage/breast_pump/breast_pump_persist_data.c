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
    pump_info->last_expert_mode              = 255;
    pump_info->last_regular_mode             = 255;
    pump_info->last_pump_mode                = BREAST_PUMP_PUMP_MODE_DEFAULT;
    for (int i = 0; i < BREAST_PUMP_MODE_MAX; i++) {
        pump_info->mode_update_flag[i] = 0;
        breast_pump_setting_reset(pump_info, i);
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
    uint16_t crc = CRC16(&store_pump_info->pump_info, sizeof(sys_pump_info_t));
    if (ret == 0 && store_pump_info->magic == STORAGE_BREAST_PUMP_INFO_MAGIC_NUM &&
        store_pump_info->crc == crc) {
        memcpy(pump_info, &store_pump_info->pump_info, sizeof(sys_pump_info_t));
    } else {
        if (ret == 0) {
            ret = -1;
        }
        reset_pump_info(args.ctx, pump_info);
        LOGE("read pump info failure! ret: %d, magic: 0x%x, crc: 0x%x, expect_crc: 0x%x",
             ret, store_pump_info->magic, store_pump_info->crc, crc);
    }
    u_free(store_pump_info);
    return ret;
}

int write_pump_mode_steps_info_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_mode_info_t *mode_info = (sys_pump_mode_info_t *)args.val;
    EXCEPTION_CHECK_INT(mode_info);
    breast_pump_mode_steps_info_storage_t *store_mode_info = u_malloz(sizeof(breast_pump_mode_steps_info_storage_t));
    EXCEPTION_CHECK_INT(store_mode_info);
    store_mode_info->magic = STORAGE_PUMP_MODE_STEPS_INFO_MAGIC_NUM;
    memcpy(&store_mode_info->mode_info, mode_info, sizeof(sys_pump_mode_info_t));
    store_mode_info->crc = CRC16(&store_mode_info->mode_info, sizeof(sys_pump_mode_info_t));
    ret = user_database_put(args.ctx, PERSIST_KEY_BREAST_PUMP_MODE_STEPS_INFO, (uint8_t *)store_mode_info,
                sizeof(breast_pump_mode_steps_info_storage_t));
    if (ret != 0) {
        LOGE("write pump mode steps info failure! ret: %d", ret);
    }
    u_free(store_mode_info);
    return ret;
}

void reset_pump_mode_steps_info(storage_manager_context_t *ctx, sys_pump_mode_info_t *mode_info)
{
    EXCEPTION_CHECK_VOID(ctx);
    EXCEPTION_CHECK_VOID(mode_info);
    memset(mode_info, 0, sizeof(sys_pump_mode_info_t));
    for (int i = 0; i < BREAST_PUMP_MODE_MAX; i++) {
        breast_pump_mode_steps_info_reset(mode_info, i);
    }
    store_func_args_t args = {
        .ctx = ctx,
        .arg1 = 0,
        .arg2 = 0,
        .val = mode_info,
    };
    LOG_STORE_WRAP("==> storage_reset_pump_mode_steps_info !!!!!!!!!!!!!!!");
    write_pump_mode_steps_info_to_flash(args);
}

int read_pump_mode_steps_info_from_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_mode_info_t *mode_info = (sys_pump_mode_info_t *)args.val;
    EXCEPTION_CHECK_INT(mode_info);
    breast_pump_mode_steps_info_storage_t *store_mode_info = u_malloz(sizeof(breast_pump_mode_steps_info_storage_t));
    EXCEPTION_CHECK_INT(store_mode_info);
    ret = user_database_get(args.ctx, PERSIST_KEY_BREAST_PUMP_MODE_STEPS_INFO, (uint8_t *)store_mode_info,
                    sizeof(breast_pump_mode_steps_info_storage_t));
    uint16_t crc = CRC16(&store_mode_info->mode_info, sizeof(sys_pump_mode_info_t));
    if (ret == 0 && store_mode_info->magic == STORAGE_PUMP_MODE_STEPS_INFO_MAGIC_NUM &&
        store_mode_info->crc == crc) {
        memcpy(mode_info, &store_mode_info->mode_info, sizeof(sys_pump_mode_info_t));
    } else {
        if (ret == 0) {
            ret = -1;
        }
        reset_pump_mode_steps_info(args.ctx, mode_info);
        LOGE("read pump mode steps info failure! ret: %d, magic: 0x%x, crc: 0x%x, expect_crc: 0x%x",
             ret, store_mode_info->magic, store_mode_info->crc, crc);
    }
    u_free(store_mode_info);
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
    uint16_t crc = CRC16(&store_buried_info->buried_info_list, sizeof(sys_buried_info_list_t));
    if (ret == 0 && store_buried_info->magic == STORAGE_BREAST_PUMP_BURY_INFO_MAGIC_NUM &&
        store_buried_info->crc == crc) {
        memcpy(buried_info_list, &store_buried_info->buried_info_list, sizeof(sys_buried_info_list_t));
    } else {
        if (ret == 0) {
            ret = -1;
        }
        reset_pump_buried_info(args.ctx, buried_info_list);
        LOGE("read pump buried info failure! ret: %d, magic: 0x%x, crc: 0x%x, expect_crc: 0x%x",
             ret, store_buried_info->magic, store_buried_info->crc, crc);
    }
    u_free(store_buried_info);

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
    uint16_t crc = CRC16(&store_pump_data_head->pump_data_head, sizeof(breast_pump_data_head_t));
    if (ret == 0 && store_pump_data_head->magic == STORAGE_BREAST_PUMP_DATA_HEAD_MAGIC_NUM &&
        store_pump_data_head->crc == crc) {
        memcpy(pump_data_head, &store_pump_data_head->pump_data_head, sizeof(breast_pump_data_head_t));
    } else {
        if (ret == 0) {
            ret = -1;
        }
        reset_pump_data_head(args.ctx, pump_data_head);
        LOGE("read pump user data head failure! ret: %d, magic: 0x%x, crc: 0x%x, expect_crc: 0x%x",
             ret, store_pump_data_head->magic, store_pump_data_head->crc, crc);
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
    uint16_t crc = CRC16(&store_pump_data_body->pump_data_body, sizeof(pump_result_info_t));
    if (ret == 0 && store_pump_data_body->magic == STORAGE_BREAST_PUMP_DATA_BODY_MAGIC_NUM &&
        store_pump_data_body->crc == crc) {
        memcpy(pump_data_body, &store_pump_data_body->pump_data_body, sizeof(pump_result_info_t));
    } else {
        if (ret == 0) {
            ret = -1;
        }
        memset(pump_data_body, 0, sizeof(pump_result_info_t));
        LOGE("read pump data body failure! ret: %d, body_offset: %d, magic: 0x%x, crc: 0x%x, expect_crc: 0x%x",
             ret, body_offset, store_pump_data_body->magic, store_pump_data_body->crc, crc);
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
    sys_pump_mode_info_t *mode_info = &args.ctx->priv.sys_info.mode_info;
    uint32_t *pump_time = (uint32_t *)args.val;
    EXCEPTION_CHECK_INT(pump_time);
    uint8_t mode = args.arg1;
    if (mode >= BREAST_PUMP_MODE_MAX) {
        LOGE("mode: %d, out of range", mode);
        return -1;
    }
    mode_info->steps_info[mode].total_time = *pump_time;
    LOG_STORE_WRAP("write pump mode: %d, time: %d", mode, *pump_time);
    return 0;
}

int read_pump_time_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_mode_info_t *mode_info = &args.ctx->priv.sys_info.mode_info;
    uint32_t *pump_time = (uint32_t *)args.val;
    EXCEPTION_CHECK_INT(pump_time);
    uint8_t mode = args.arg1;
    if (mode >= BREAST_PUMP_MODE_MAX) {
        LOGE("mode: %d, out of range", mode);
        return -1;
    }
    *pump_time = (uint32_t)mode_info->steps_info[mode].total_time;
    // LOG_STORE_WRAP("read pump mode: %d, time: %d", mode, *pump_time);
    return 0;
}

int write_pump_mode_steps_info_to_ram(store_func_args_t args)
{
    struct storage_manager_context *st = args.ctx;
    EXCEPTION_CHECK_INT(st);
    sys_pump_mode_info_t *mode_info = &st->priv.sys_info.mode_info;
    EXCEPTION_CHECK_INT(mode_info);
    uint8_t mode = args.arg1;
    EXCEPTION_CHECK_INT(args.val);
    if (mode >= BREAST_PUMP_MODE_MAX) {
        LOGE("mode: %d, out of range", mode);
        return -1;
    }
    memcpy(&mode_info->steps_info[args.arg1], args.val, sizeof(pump_mode_steps_info_t));
    
    return 0;
}

int read_pump_mode_steps_info_from_ram(store_func_args_t args)
{
    struct storage_manager_context *st = args.ctx;
    EXCEPTION_CHECK_INT(st);
    sys_pump_mode_info_t *mode_info = &st->priv.sys_info.mode_info;
    pump_mode_steps_info_t *steps_info = (pump_mode_steps_info_t *)args.val;
    EXCEPTION_CHECK_INT(steps_info);
    uint8_t mode = args.arg1;
    if (mode >= BREAST_PUMP_MODE_MAX) {
        LOGE("mode: %d, out of range", mode);
        return -1;
    }
    memcpy(steps_info, &mode_info->steps_info[args.arg1], sizeof(pump_mode_steps_info_t));
    
    return 0;
}

int write_pump_mode_name_to_ram(store_func_args_t args)
{
    struct storage_manager_context *st = args.ctx;
    EXCEPTION_CHECK_INT(st);
    sys_pump_mode_info_t *mode_info = &st->priv.sys_info.mode_info;
    EXCEPTION_CHECK_INT(mode_info);
    uint8_t mode = args.arg1;
    if (mode >= BREAST_PUMP_MODE_MAX) {
        LOGE("mode: %d, out of range", mode);
        return -1;
    }
    char *mode_name = (char *)args.val;
    EXCEPTION_CHECK_INT(mode_name);
    strncpy(mode_info->mode_name[args.arg1], mode_name, PUMP_MODE_NAME_LEN - 1);
    mode_info->mode_name[args.arg1][PUMP_MODE_NAME_LEN - 1] = '\0';
    
    return 0;
}

int read_pump_mode_name_from_ram(store_func_args_t args)
{
    struct storage_manager_context *st = args.ctx;
    EXCEPTION_CHECK_INT(st);
    sys_pump_mode_info_t *mode_info = &st->priv.sys_info.mode_info;
    uint8_t mode = args.arg1;
    char *mode_name = (char *)args.val;
    EXCEPTION_CHECK_INT(mode_name);
    if (mode >= BREAST_PUMP_MODE_MAX) {
        LOGE("mode: %d, out of range", mode);
        return -1;
    }
    uint32_t mode_name_len = (args.arg2 > 0) ? args.arg2 : PUMP_MODE_NAME_LEN;
    strncpy(mode_name, mode_info->mode_name[args.arg1], mode_name_len - 1);
    mode_name[mode_name_len - 1] = '\0';

    return 0;
}

int write_pump_setting_info_to_ram(store_func_args_t args)
{
    struct storage_manager_context *st = args.ctx;
    EXCEPTION_CHECK_INT(st);
    pump_setting_t *setting = (pump_setting_t *)args.val;
    EXCEPTION_CHECK_INT(setting);
    sys_pump_info_t *pump_info = &st->priv.sys_info.pump_info;
    uint8_t mode = args.arg1;
    uint8_t mode_step = args.arg2;
    // LOG_STORE("mode: %d, step: %d", mode, mode_step);
    if (mode >= BREAST_PUMP_MODE_MAX) {
        LOGE("mode: %d, out of range", mode);
        return -1;
    }

    if (mode_step >= ROOT_BREAST_PUMP_STEP_MAX) {
        LOGE("mode_step: %d, out of range", mode_step);
        return -1;
    }

#if IS_PUMP_STEPS_USE_SAME_SETTING
    for (uint8_t i = 0; i < ROOT_BREAST_PUMP_STEP_MAX; i++) {
        pump_info->setting_wrap[mode].info[i].level[0] = setting->level[0];
        pump_info->setting_wrap[mode].info[i].level[1] = setting->level[1];
        pump_info->setting_wrap[mode].info[i].speed = setting->speed;
    }
#else
    memcpy(&pump_info->setting_wrap[mode].info[mode_step], setting, sizeof(pump_setting_t));
#endif 

    return 0;
}

int read_pump_setting_info_from_ram(store_func_args_t args)
{
    struct storage_manager_context *st = args.ctx;
    EXCEPTION_CHECK_INT(st);
    sys_pump_info_t *pump_info = &st->priv.sys_info.pump_info;
    pump_setting_t *setting = (pump_setting_t *)args.val;
    EXCEPTION_CHECK_INT(setting);
    uint8_t mode = args.arg1;
    uint8_t mode_step = args.arg2;
    // LOG_STORE("mode: %d, step: %d", args.arg1, args.arg2);
    if (mode >= BREAST_PUMP_MODE_MAX) {
        LOGE("mode: %d, out of range", mode);
        return -1;
    }
    
    if (mode_step >= ROOT_BREAST_PUMP_STEP_MAX) {
        LOGE("mode_step: %d, out of range", mode_step);
        return -1;
    }
    memcpy(setting, &pump_info->setting_wrap[mode].info[mode_step], sizeof(pump_setting_t));

    return 0;
}

int write_pump_control_type_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    pump_info->control_type = args.arg1;
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

int write_pump_mode_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    uint8_t mode = args.arg1;
    if (mode >= BREAST_PUMP_MODE_MAX) {
        LOGE("mode: %d, out of range", mode);
        return -1;
    }
    pump_info->last_pump_mode = mode;
    LOG_STORE_WRAP("write pump mode: %d", pump_info->last_pump_mode);
    return 0;
}

int read_pump_mode_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = pump_info->last_pump_mode;
    return 0;
}

int write_pump_mode_with_type_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    uint8_t mode_type = args.arg1;
    uint8_t mode = args.arg2;
    if (mode_type >= BREAST_PUMP_MODE_TYPE_MAX) {
        LOGE("mode_type: %d, out of range", mode_type);
        return -1;
    }
    
    if (mode >= BREAST_PUMP_MODE_MAX) {
        LOGE("mode: %d, out of range", mode);
        return -1;
    }

    if (mode_type == BREAST_PUMP_MODE_TYPE_COMMON) {
        pump_info->last_regular_mode = mode;
    } else {
        pump_info->last_expert_mode = mode;
    }
    LOG_STORE_WRAP("type: %d, mode: %d", mode_type, mode);
    return 0;
}

int read_pump_mode_with_type_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    uint8_t mode_type = args.arg1;
    int *mode = (int *)args.val;
    EXCEPTION_CHECK_INT(mode);
    if (mode_type >= BREAST_PUMP_MODE_TYPE_MAX) {
        LOGE("mode_type: %d, out of range", mode_type);
        return -1;
    }
    if (mode_type == BREAST_PUMP_MODE_TYPE_COMMON) {
        *mode = (int)pump_info->last_regular_mode;
    } else {
        *mode = (int)pump_info->last_expert_mode;
    }
    return 0;
}

int write_mode_update_flag_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    if (args.arg1 < 0 || args.arg1 >= BREAST_PUMP_MODE_MAX) {
        LOGE("mode: %d, out of range", args.arg1);
        return -1;
    }
    pump_info->mode_update_flag[args.arg1] = 1;
    LOG_STORE_WRAP("write mode update flag: %d", pump_info->mode_update_flag[args.arg1]);
    return 0;
}

int read_mode_update_flag_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    if (args.arg1 < 0 || args.arg1 >= BREAST_PUMP_MODE_MAX) {
        LOGE("mode: %d, out of range", args.arg1);
        return -1;
    }
    *tmp = pump_info->mode_update_flag[args.arg1];
    return 0;
}

int write_pump_buried_info_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    EXCEPTION_CHECK_INT(args.val);
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

int read_pump_data_head_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    breast_pump_data_head_t *pump_data_head = (breast_pump_data_head_t *)args.val;
    EXCEPTION_CHECK_INT(pump_data_head);
    memcpy(pump_data_head, &args.ctx->priv.pump_data_head, sizeof(breast_pump_data_head_t));
    return 0;
}

int fill_in_pump_data_info(int num)
{
    return 0;
}

/**
 * @brief 完整的泵奶数据写入和读取测试函数
 * @param write_count 要写入的记录数量（0表示使用默认值：测试未满和已满情况）
 * @return 0: 成功, -1: 失败
 * 
 * 测试场景：
 * 1. 未满情况：写入少量数据（如10条）
 * 2. 已满情况：写入超过ROOT_BREAST_PUMP_DATA_BODY_NUM的数据（如150条）
 * 3. 验证循环缓冲区逻辑：读取所有数据并验证正确性
 */
int test_pump_data_write_read_complete(int write_count)
{
    storage_manager_context_t *ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (ctx == NULL) {
        LOGE("test_pump_data: storage context not found");
        return -1;
    }

    /// 如果write_count为0，使用默认测试值
    if (write_count == 0) {
        write_count = ROOT_BREAST_PUMP_DATA_BODY_NUM + 10; /// 测试未满和已满情况
    }

    LOGI("==========================================");
    LOGI("Start pump data write/read complete test");
    LOGI("Plan to write %d records (buffer size: %d)", write_count, ROOT_BREAST_PUMP_DATA_BODY_NUM);
    LOGI("==========================================");

    /// 步骤1: 清空数据头（可选，用于干净测试）
    breast_pump_data_head_t pump_data_head = {0};
    reset_pump_data_head(ctx, &pump_data_head);
    LOGI("Data head cleared");

    /// 步骤2: 写入测试数据
    LOGI("Start writing test data...");
    uint32_t base_timestamp = platform_get_utc_time();
    int write_success_count = 0;
    int write_fail_count = 0;

    for (int i = 0; i < write_count; i++) {
        /// 准备测试数据
        pump_result_info_t test_data = {0};
        test_data.mode = (i % 3) + 1; /// 模式：1, 2, 3循环
        test_data.timestamp = base_timestamp + i * 3600; /// 每条记录间隔1小时
        test_data.setting_time = 1800000; /// 30分钟（毫秒）
        test_data.target_valid_time = 1800000;
        test_data.target_total_time = 1800000;
        test_data.work_total_time = 1800000 - (i * 1000); /// 递减的工作时间
        test_data.work_valid_time = 1800000 - (i * 1000);
        test_data.current_mode_work_time = 1800000 - (i * 1000);
        test_data.report_flag = (i % 2); /// 0或1
        /// 设置一些测试参数
        for (int j = 0; j < ROOT_BREAST_PUMP_STEP_MAX; j++) {
            for (int k = 0; k < BREAST_PUMP_NUM; k++) {
                test_data.setting[j].level[k] = ((i + j + k) % 10) + 1; /// 力度等级1-10
            }
            test_data.setting[j].speed = (i + j) % BREAST_PUMP_SPEED_MAX; /// 速度
        }

        /// 计算写入索引（在更新数据头之前）
        uint8_t write_index;
        if (pump_data_head.num < ROOT_BREAST_PUMP_DATA_BODY_NUM) {
            /// 未满：写入到新位置
            write_index = pump_data_head.num;  /// 下一个写入位置
        } else {
            /// 已满：写入到head位置（覆盖最旧数据）
            write_index = pump_data_head.head;
        }

        /// 先写入数据体（成功后再更新数据头）
        store_func_args_t args = {
            .ctx = ctx,
            .arg1 = write_index,
            .arg2 = 0,
            .val = &test_data,
        };
        int ret = write_pump_data_body_to_flash(args);
        if (ret != 0) {
            LOGE("Write data body failed [%d]: ret=%d, index=%d, stop writing", i, ret, write_index);
            write_fail_count++;
            break;  /// 写入失败，退出后续的写入
        }

        /// 数据体写入成功，更新数据头
        if (pump_data_head.num < ROOT_BREAST_PUMP_DATA_BODY_NUM) {
            pump_data_head.num += 1;
            pump_data_head.tail = pump_data_head.num - 1;
            if (pump_data_head.num == 1) {
                pump_data_head.head = 0;
            }
        } else {
            pump_data_head.head = (pump_data_head.head + 1) % ROOT_BREAST_PUMP_DATA_BODY_NUM;
            pump_data_head.tail = (pump_data_head.tail + 1) % ROOT_BREAST_PUMP_DATA_BODY_NUM;
            pump_data_head.num = ROOT_BREAST_PUMP_DATA_BODY_NUM;
        }

        /// 写入数据头
        args.arg1 = 0;
        args.arg2 = 0;
        args.val = &pump_data_head;
        ret = write_pump_data_head_to_flash(args);
        if (ret != 0) {
            LOGE("Write data head failed [%d]: ret=%d, stop writing", i, ret);
            write_fail_count++;
            break;  /// 写入失败，退出后续的写入
        }

        write_success_count++;
        if ((i + 1) % 20 == 0 || i == write_count - 1) {
            LOGI("Written: %d/%d (success: %d, failed: %d), num=%d, head=%d, tail=%d", 
                 i + 1, write_count, write_success_count, write_fail_count,
                 pump_data_head.num, pump_data_head.head, pump_data_head.tail);
        }
    }

    LOGI("Write completed: success=%d, failed=%d", write_success_count, write_fail_count);
    LOGI("Final data head: num=%d, head=%d, tail=%d", 
         pump_data_head.num, pump_data_head.head, pump_data_head.tail);

    /// 步骤3: 读取并验证数据
    LOGI("==========================================");
    LOGI("Start reading and verifying data...");
    LOGI("==========================================");

    /// 读取数据头
    breast_pump_data_head_t read_head = {0};
    store_func_args_t read_args = {
        .ctx = ctx,
        .arg1 = 0,
        .arg2 = 0,
        .val = &read_head,
    };
    int ret = read_pump_data_head_from_flash(read_args);
    if (ret != 0) {
        LOGE("Read data head failed: ret=%d", ret);
        return -1;
    }

    LOGI("Read data head: num=%d, head=%d, tail=%d", 
         read_head.num, read_head.head, read_head.tail);

    /// 验证数据头一致性
    if (read_head.num != pump_data_head.num || 
        read_head.head != pump_data_head.head || 
        read_head.tail != pump_data_head.tail) {
        LOGE("Data head mismatch! Write: num=%d,head=%d,tail=%d; Read: num=%d,head=%d,tail=%d",
             pump_data_head.num, pump_data_head.head, pump_data_head.tail,
             read_head.num, read_head.head, read_head.tail);
        return -1;
    }
    LOGI("Data head verification passed");

    /// 读取所有数据并验证
    int read_success_count = 0;
    int read_fail_count = 0;
    int verify_fail_count = 0;
    /// 实际成功写入的记录数（用于验证）
    uint32_t actual_write_count = write_success_count;

    LOGI("Start reading %d records... (actual written: %d)", read_head.num, actual_write_count);

    for (uint16_t num = 1; num <= read_head.num; num++) {
        /// 计算循环缓冲区索引
        uint8_t body_index = (read_head.tail + ROOT_BREAST_PUMP_DATA_BODY_NUM - (num - 1)) % ROOT_BREAST_PUMP_DATA_BODY_NUM;

        /// 读取数据体
        pump_result_info_t read_data = {0};
        read_args.arg1 = body_index;
        read_args.val = &read_data;
        ret = read_pump_data_body_from_flash(read_args);
        if (ret != 0) {
            LOGE("Read data body failed [num=%d, index=%d]: ret=%d", num, body_index, ret);
            read_fail_count++;
            continue;
        }

        /// 验证数据（根据实际成功写入的记录数计算期望值）
        /// 注意：由于循环缓冲区，最新的记录对应num=1
        /// 由于写入失败，数据头中的num可能大于actual_write_count
        /// 需要根据read_head.num和actual_write_count的关系来判断哪些记录是有效的
        
        /// 如果num > actual_write_count，说明这条记录位置可能没有有效数据（写入失败）
        if (num > actual_write_count) {
            /// 检查数据是否为空，如果为空则跳过验证
            if (read_data.timestamp == 0 && read_data.mode == 0 && read_data.work_total_time == 0) {
                LOGW("Skip verification [num=%d, index=%d]: num > actual_write_count (%d) and data is empty", 
                     num, body_index, actual_write_count);
                continue;
            }
            /// 如果数据不为空，可能是之前写入的旧数据，也跳过验证（因为无法确定对应关系）
            LOGW("Skip verification [num=%d, index=%d]: num > actual_write_count (%d), data may be stale", 
                 num, body_index, actual_write_count);
            continue;
        }
        
        /// num <= actual_write_count，说明这条记录应该有有效数据
        /// num=1对应最新的记录（i=actual_write_count-1），num=num对应最旧的记录
        uint32_t expected_i = actual_write_count - num;
        
        /// 如果expected_i超出范围，说明这条记录可能是写入失败留下的空数据，跳过验证
        if (expected_i >= (uint32_t)write_count) {
            LOGW("Skip verification [num=%d, index=%d]: expected_i=%lu >= write_count=%d (may be failed write)", 
                 num, body_index, expected_i, write_count);
            continue;
        }
        
        /// 检查数据是否为空（可能是写入失败留下的空数据）
        if (read_data.timestamp == 0 && read_data.mode == 0 && read_data.work_total_time == 0) {
            LOGW("Skip verification [num=%d, index=%d]: data is empty (may be failed write)", 
                 num, body_index);
            continue;
        }

        /// 验证关键字段
        uint32_t expected_timestamp = base_timestamp + expected_i * 3600;
        uint8_t expected_mode = (expected_i % 3) + 1;
        uint32_t expected_work_time = 1800000 - (expected_i * 1000);

        bool verify_ok = true;
        if (read_data.timestamp != expected_timestamp) {
            LOGW("Timestamp mismatch [num=%d]: expected=%lu, actual=%lu", 
                 num, expected_timestamp, read_data.timestamp);
            verify_ok = false;
        }
        if (read_data.mode != expected_mode) {
            LOGW("Mode mismatch [num=%d]: expected=%d, actual=%d", 
                 num, expected_mode, read_data.mode);
            verify_ok = false;
        }
        if (read_data.work_total_time != expected_work_time) {
            LOGW("Work time mismatch [num=%d]: expected=%lu, actual=%lu", 
                 num, expected_work_time, read_data.work_total_time);
            verify_ok = false;
        }

        if (verify_ok) {
            read_success_count++;
            if (num <= 5 || num > read_head.num - 5 || num % 20 == 0) {
                LOGI("Verification passed [num=%d, index=%d]: mode=%d, timestamp=%lu, work_time=%lu", 
                     num, body_index, read_data.mode, read_data.timestamp, read_data.work_total_time);
            }
        } else {
            verify_fail_count++;
            LOGE("Verification failed [num=%d, index=%d]: mode=%d/%d, timestamp=%lu/%lu, work_time=%lu/%lu",
                 num, body_index, read_data.mode, expected_mode,
                 read_data.timestamp, expected_timestamp,
                 read_data.work_total_time, expected_work_time);
        }
    }

    /// 步骤4: 输出测试结果
    LOGI("==========================================");
    LOGI("Test result summary");
    LOGI("==========================================");
    LOGI("Write: success=%d, failed=%d", write_success_count, write_fail_count);
    LOGI("Read: success=%d, failed=%d", read_success_count, read_fail_count);
    LOGI("Verify: passed=%d, failed=%d", read_success_count, verify_fail_count);
    LOGI("Data head: num=%d, head=%d, tail=%d", read_head.num, read_head.head, read_head.tail);
    LOGI("==========================================");

    if (write_fail_count == 0 && read_fail_count == 0 && verify_fail_count == 0) {
        LOGI("All tests passed!");
        return 0;
    } else {
        LOGE("Test failed! Write failed=%d, Read failed=%d, Verify failed=%d", 
             write_fail_count, read_fail_count, verify_fail_count);
        return -1;
    }
}
