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
    pump_info->custom_activate               = 0;
    pump_info->last_expert_mode              = BREAST_PUMP_MODE_INVALID;
    pump_info->last_regular_mode             = BREAST_PUMP_MODE_INVALID;
    pump_info->last_pump_mode                = BREAST_PUMP_PUMP_MODE_DEFAULT;
    pump_info->control_type                  = BREAST_PUMP_CONTROL_TYPE_BILATERAL;
    for (int i = 0; i < BREAST_PUMP_MODE_MAX; i++) {
        pump_info->new_step_info_flag[i] = false;
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
    LOG_STORE_WRAP("-->write pump data body success, body_offset: %d, record:%d,%d,%d",
        body_offset, pump_data_body->finish_timestamp, pump_data_body->mode, pump_data_body->work_valid_time);
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
    LOG_STORE_WRAP("-->read pump data body success, body_offset: %d, record:%d,%d,%d",
        body_offset, pump_data_body->finish_timestamp, pump_data_body->mode, pump_data_body->work_valid_time);
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

int write_pump_mode_activate_to_ram(store_func_args_t args)
{
    struct storage_manager_context *st = args.ctx;
    EXCEPTION_CHECK_INT(st);
    sys_pump_info_t *pump_info = &st->priv.sys_info.pump_info;
    EXCEPTION_CHECK_INT(pump_info);
    uint8_t mode = args.arg1;
    if (mode == BREAST_PUMP_CUSTOM_MODE) {
        pump_info->custom_activate = args.arg2 > 0 ? true : false; 
    }   
    return 0;
}

int read_pump_mode_activate_from_ram(store_func_args_t args)
{
    struct storage_manager_context *st = args.ctx;
    EXCEPTION_CHECK_INT(st);
    sys_pump_info_t *pump_info = &st->priv.sys_info.pump_info;
    uint8_t mode = args.arg1;
    uint32_t *custom_activate = (uint32_t *)args.val;
    if (mode != BREAST_PUMP_CUSTOM_MODE) {
        *custom_activate = true;
    } else {
        *custom_activate = pump_info->custom_activate;
    }
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
    int mode = args.arg1;
    uint8_t mode_step = args.arg2;
    if (mode >= BREAST_PUMP_MODE_MAX && mode != BREAST_PUMP_ALL_MODE_AND_PROGRAM) {
        LOGE("mode: %d, out of range", mode);
        return -1;
    }

    if (mode_step >= ROOT_BREAST_PUMP_STEP_MAX) {
        LOGE("mode_step: %d, out of range", mode_step);
        return -1;
    }
#if IS_SUPPORT_PUMP_GLOBAL_PARAM_SETTING
    if (mode == BREAST_PUMP_ALL_MODE_AND_PROGRAM) {
        for (int i = 0; i <= BREAST_PUMP_CUSTOM_MODE; i++) {
            memcpy(&pump_info->setting_wrap[i].info[mode_step], setting, sizeof(pump_setting_t));
        }
        return 0;
    }
    memcpy(&pump_info->setting_wrap[mode].info[mode_step], setting, sizeof(pump_setting_t));
#else
    memcpy(&pump_info->setting_wrap[mode].info[mode_step], setting, sizeof(pump_setting_t));
#endif
    // LOG_STORE_WRAP("mode: %d, step: %d, setting: %d(%d/%d)",
    //      mode, mode_step, setting->speed, setting->level[0], setting->level[1]);
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
    if (mode >= BREAST_PUMP_MODE_MAX) {
        LOGE("mode: %d, out of range", mode);
        return -1;
    }
    
    if (mode_step >= ROOT_BREAST_PUMP_STEP_MAX) {
        LOGE("mode_step: %d, out of range", mode_step);
        return -1;
    }
    memcpy(setting, &pump_info->setting_wrap[mode].info[mode_step], sizeof(pump_setting_t));
    // LOG_STORE_WRAP("mode: %d, step: %d, setting: %d(%d/%d)",
    //      mode, mode_step, setting->speed, setting->level[0], setting->level[1]);
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

int read_pump_mode_steps_info_id_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_mode_info_t *mode_info = &args.ctx->priv.sys_info.mode_info;
    uint8_t mode_idx = args.arg1;
    uint32_t *info_id = (uint32_t *)args.val;
    EXCEPTION_CHECK_INT(info_id);
    if (mode_idx >= BREAST_PUMP_MODE_MAX) {
        LOGE("mode_idx: %d, out of range", mode_idx);
        return -1;
    }
    *info_id = mode_info->steps_info[mode_idx].id.dat32;
    return 0;
}

int write_mode_new_step_info_flag_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    if (args.arg1 < 0 || args.arg1 >= BREAST_PUMP_MODE_MAX) {
        LOGE("mode: %d, out of range", args.arg1);
        return -1;
    }
    pump_info->new_step_info_flag[args.arg1] = args.arg2;
    LOG_STORE_WRAP("write new step info flag: %d", pump_info->new_step_info_flag[args.arg1]);
    return 0;
}

int read_mode_new_step_info_flag_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    sys_pump_info_t *pump_info = &args.ctx->priv.sys_info.pump_info;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    if (args.arg1 < 0 || args.arg1 >= BREAST_PUMP_MODE_MAX) {
        LOGE("mode: %d, out of range", args.arg1);
        return -1;
    }
    *tmp = pump_info->new_step_info_flag[args.arg1];
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