/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "persist_storage.h"
#include "root_common_api.h" // IWYU pragma: keep
#include "storage_manager.h" // IWYU pragma: keep
#include "storage_manager_api.h"
#include "storage_manager_internal.h" // IWYU pragma: keep
#include "factory_persist_data.h" // IWYU pragma: keep
#include "pump_parameters_table.h" // IWYU pragma: keep
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
#if IS_SUPPORT_DEVICE_IMU
/// 写入imu校准数据到flash
int write_imu_cali_data_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    imu_cali_data_t *imu_cali = (imu_cali_data_t *)args.val;
    EXCEPTION_CHECK_INT(imu_cali);
    imu_cali_storage_t *store_imu = u_malloz(sizeof(imu_cali_storage_t));
    EXCEPTION_CHECK_INT(store_imu);
    store_imu->magic = STORAGE_IMU_CALIB_MAGIC_NUM;
    memcpy(&store_imu->imu_cali, imu_cali, sizeof(imu_cali_data_t));
    store_imu->crc = CRC16(&store_imu->imu_cali, sizeof(imu_cali_data_t));
    ret = user_database_put(args.ctx, STORE_ID_FACTORY_IMU_CALI_DATA, (uint8_t *)store_imu,
        sizeof(imu_cali_storage_t));
    if (ret != 0) { 
        LOGE("write imu cali failure! ret: %d", ret);
    }
    u_free(store_imu);
    return ret;
}

/// 从flash中读取imu校准数据
int read_imu_cali_data_from_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    imu_cali_data_t *imu_cali = (imu_cali_data_t *)args.val;
    EXCEPTION_CHECK_INT(imu_cali);
    imu_cali_storage_t *store_imu = u_malloz(sizeof(imu_cali_storage_t));
    EXCEPTION_CHECK_INT(store_imu);
    ret = user_database_get(args.ctx, STORE_ID_FACTORY_IMU_CALI_DATA, (uint8_t *)store_imu,
        sizeof(imu_cali_storage_t));
    uint16_t crc = CRC16(&store_imu->imu_cali, sizeof(imu_cali_data_t));
    if (ret == 0 && store_imu->magic == STORAGE_IMU_CALIB_MAGIC_NUM && store_imu->crc == crc) {
        memcpy(imu_cali, &store_imu->imu_cali, sizeof(imu_cali_data_t));
    } else {
        memset(imu_cali, 0, sizeof(imu_cali_data_t));
        LOGE("read imu cali failure! ret: %d, magic: %d, crc: %d, expect_crc: %d", ret, store_imu->magic,
             store_imu->crc, crc);
    }
    u_free(store_imu);
    return ret;
}
#endif

void reset_factory_flag_info(storage_manager_context_t *ctx, factory_flag_info_t *flag_info)
{
    EXCEPTION_CHECK_VOID(ctx);
    EXCEPTION_CHECK_VOID(flag_info);
    memset(flag_info, 0, sizeof(factory_flag_info_t));
    flag_info->deploy_flag = 0;
    flag_info->age_display = 0;
    flag_info->fsensor_cali_flag = 0;
    for (int i = 0; i < FACTORY_AGE_INFO_MAX; i++) {
        flag_info->age_info[i].age_flag = 0;
        flag_info->age_info[i].age_cycle = 0; 
        flag_info->age_info[i].age_time = 0;
    }
    store_func_args_t args = {
        .ctx = ctx,
        .arg1 = 0,
        .arg2 = 0,
        .val = flag_info,
    };
    LOG_STORE("reset factory flag info");
    write_factory_flag_info_to_flash(args);
}

int write_factory_deploy_flag_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    factory_flag_info_t *flag_info = &args.ctx->priv.factory_data.flag_info;
    flag_info->deploy_flag = (uint8_t)args.arg1;
    factory_flag_storage_t *store_flag = u_malloz(sizeof(factory_flag_storage_t));
    EXCEPTION_CHECK_INT(store_flag);
    store_flag->magic = STORAGE_FAC_FLAG_MAGIC_NUM;
    memcpy(&store_flag->flag_info, flag_info, sizeof(factory_flag_info_t));
    store_flag->crc = CRC16(&store_flag->flag_info, sizeof(factory_flag_info_t));
    ret = user_database_put(args.ctx, STORE_ID_FACTORY_FLAG_INFO, (uint8_t *)store_flag,
        sizeof(factory_flag_storage_t));
    if (ret != 0) {
        LOGE("write factory deploy flag failure! ret: %d", ret);
    }
    u_free(store_flag);

    return ret;
}

int write_factory_age_flag_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    EXCEPTION_CHECK_INT(args.val);
    if (args.arg1 < 0 || args.arg1 >= FACTORY_AGE_INFO_MAX) {
        LOGE("age index: %d, out of range", args.arg1);
        return -1;
    }
    factory_flag_info_t *flag_info = &args.ctx->priv.factory_data.flag_info;
    factory_age_info_t *age_info = &flag_info->age_info[args.arg1];
    EXCEPTION_CHECK_INT(age_info);
    memcpy(age_info, (factory_age_info_t*)args.val, sizeof(factory_age_info_t));

    factory_flag_storage_t *store_flag = u_malloz(sizeof(factory_flag_storage_t));
    EXCEPTION_CHECK_INT(store_flag);
    store_flag->magic = STORAGE_FAC_FLAG_MAGIC_NUM;
    memcpy(&store_flag->flag_info, flag_info, sizeof(factory_flag_info_t));
    store_flag->crc = CRC16(&store_flag->flag_info, sizeof(factory_flag_info_t));
    ret = user_database_put(args.ctx, STORE_ID_FACTORY_FLAG_INFO, (uint8_t *)store_flag,
        sizeof(factory_flag_storage_t));
    if (ret != 0) {
        LOGE("write factory age flag failure! ret: %d", ret);
    }
    u_free(store_flag);

    return ret;
}

int write_factory_age_display_flag_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    factory_flag_info_t *flag_info = &args.ctx->priv.factory_data.flag_info;
    flag_info->age_display = (uint8_t)args.arg1;
    factory_flag_storage_t *store_flag = u_malloz(sizeof(factory_flag_storage_t));
    EXCEPTION_CHECK_INT(store_flag);
    store_flag->magic = STORAGE_FAC_FLAG_MAGIC_NUM;
    memcpy(&store_flag->flag_info, flag_info, sizeof(factory_flag_info_t));
    store_flag->crc = CRC16(&store_flag->flag_info, sizeof(factory_flag_info_t));
    ret = user_database_put(args.ctx, STORE_ID_FACTORY_FLAG_INFO, (uint8_t *)store_flag,
        sizeof(factory_flag_storage_t));
    if (ret != 0) {
        LOGE("write factory age display flag failure! ret: %d", ret);
    }
    u_free(store_flag);

    return ret; 
}

#if IS_SUPPORT_DEVICE_FSENSOR
int write_factory_fsensor_cali_flag_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    factory_flag_info_t *flag_info = &args.ctx->priv.factory_data.flag_info;
    flag_info->fsensor_cali_flag = args.arg1;
    factory_flag_storage_t *store_flag = u_malloz(sizeof(factory_flag_storage_t));
    EXCEPTION_CHECK_INT(store_flag);
    store_flag->magic = STORAGE_FAC_FLAG_MAGIC_NUM;
    memcpy(&store_flag->flag_info, flag_info, sizeof(factory_flag_info_t));
    store_flag->crc = CRC16(&store_flag->flag_info, sizeof(factory_flag_info_t));
    ret = user_database_put(args.ctx, STORE_ID_FACTORY_FLAG_INFO, (uint8_t *)store_flag,
        sizeof(factory_flag_storage_t));
    if (ret != 0) {
        LOGE("write factory fsensor cali flag failure! ret: %d", ret);
    }
    u_free(store_flag);

    return ret;
}
#endif

int write_factory_flag_info_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    factory_flag_info_t *flag_info = (factory_flag_info_t *)args.val;
    EXCEPTION_CHECK_INT(flag_info);
    factory_flag_storage_t *store_flag = u_malloz(sizeof(factory_flag_storage_t));
    EXCEPTION_CHECK_INT(store_flag);
    store_flag->magic = STORAGE_FAC_FLAG_MAGIC_NUM;

    memcpy(&store_flag->flag_info, flag_info, sizeof(factory_flag_info_t));
    store_flag->crc = CRC16(&store_flag->flag_info, sizeof(factory_flag_info_t));
    ret = user_database_put(args.ctx, STORE_ID_FACTORY_FLAG_INFO, (uint8_t *)store_flag,
        sizeof(factory_flag_storage_t));
    if (ret != 0) {
        LOGE("write factory flag info failure! ret: %d", ret);
    }
    else {
        /// 将flag_info写入ram
        factory_flag_info_t *flag_info_ram = &args.ctx->priv.factory_data.flag_info;
        EXCEPTION_CHECK_INT(flag_info_ram);
        memcpy(flag_info_ram, flag_info, sizeof(factory_flag_info_t));
        LOG_STORE("write factory flag info to ram success");
    }
    u_free(store_flag);
    return ret;
}

int read_factory_flag_info_from_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    factory_flag_info_t *flag_info = (factory_flag_info_t *)args.val;
    EXCEPTION_CHECK_INT(flag_info);
    factory_flag_storage_t *store_flag = u_malloz(sizeof(factory_flag_storage_t));
    EXCEPTION_CHECK_INT(store_flag);
    ret = user_database_get(args.ctx, STORE_ID_FACTORY_FLAG_INFO, (uint8_t *)store_flag,
        sizeof(factory_flag_storage_t));
    uint16_t crc = CRC16(&store_flag->flag_info, sizeof(factory_flag_info_t));
    if (ret == 0 && store_flag->magic == STORAGE_FAC_FLAG_MAGIC_NUM && store_flag->crc == crc) {
        memcpy(flag_info, &store_flag->flag_info, sizeof(factory_flag_info_t));
    } else {
        reset_factory_flag_info(args.ctx, flag_info);
        LOGE("read factory flag failure! ret: %d, magic: 0x%x,crc: 0x%x, expect_crc: 0x%x", ret, store_flag->magic, store_flag->crc, crc);
    }
    u_free(store_flag);
    return ret;
}

int write_factory_dev_info_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    factory_dev_info_t *device_info = (factory_dev_info_t *)args.val;
    EXCEPTION_CHECK_INT(device_info);
    factory_dev_info_storage_t *store_flag = u_malloz(sizeof(factory_dev_info_storage_t));
    EXCEPTION_CHECK_INT(store_flag);
    store_flag->magic = STORAGE_FAC_DEVINFO_MAGIC_NUM;
    memcpy(&store_flag->dev_info, device_info, sizeof(factory_dev_info_t));
    store_flag->crc = CRC16(&store_flag->dev_info, sizeof(factory_dev_info_t));
    ret = factory_flash_put(args.ctx, (uint8_t *)store_flag, sizeof(factory_dev_info_storage_t));
    if (ret == 0) {
        memcpy(&args.ctx->priv.factory_data.dev_info, device_info, sizeof(factory_dev_info_t));
    } else {
        LOGE("write factory device_info failure! ret: %d", ret);
    }
    u_free(store_flag);
    return ret;
}

int read_factory_dev_info_from_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    factory_dev_info_t *device_info = (factory_dev_info_t *)args.val;
    EXCEPTION_CHECK_INT(device_info);
    factory_dev_info_storage_t *store_flag = u_malloz(sizeof(factory_dev_info_storage_t));
    EXCEPTION_CHECK_INT(store_flag);
    ret = factory_flash_get(args.ctx, (uint8_t *)store_flag, sizeof(factory_dev_info_storage_t));
    LOG_STORE("read factory device_info from flash, ret: %d", ret);
    if (ret == 0) {
        // 仅在读取成功时计算和验证 CRC
        uint16_t crc = CRC16(&store_flag->dev_info, sizeof(factory_dev_info_t));
        if (store_flag->magic == STORAGE_FAC_DEVINFO_MAGIC_NUM && store_flag->crc == crc) {
            memcpy(device_info, &store_flag->dev_info, sizeof(factory_dev_info_t));
        } else {
            memset(device_info, 0, sizeof(factory_dev_info_t));
            LOGE("read factory device_info failure! magic: 0x%x, crc: 0x%x, expect_crc: 0x%x", store_flag->magic, store_flag->crc, crc);
            ret = -1; // 标记验证失败
        }
    } else {
        // 读取失败，清零输出缓冲区
        memset(device_info, 0, sizeof(factory_dev_info_t));
        LOGE("read factory device_info failure! flash read failed, ret: %d", ret);
    }
    u_free(store_flag);
    return ret;
}

static void copy_preset_suction_table(factory_suction_para_t *dst,
                                      const suction_parameters_t *long_tbl,
                                      const suction_parameters_t *short_tbl,
                                      const uint16_t (*freq_tbl)[SUCTION_SPEED_MAX])
{
    memcpy(dst->long_suction_table, long_tbl, sizeof(pump1_long_suction_table));
    memcpy(dst->short_suction_table, short_tbl, sizeof(pump1_short_suction_table));
    memcpy(dst->long_pump_freq_table, freq_tbl, sizeof(pump1_long_pump_freq_table));
}

int read_factory_suction_table_from_flash(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    factory_suction_para_t *suction_table = (factory_suction_para_t *)args.val;
    EXCEPTION_CHECK_INT(suction_table);

#if IS_SUPPORT_DEVICE_LIGHT_SENSOR_CALIB
    uint8_t motor_type = args.ctx->priv.factory_data.dev_info.motor_type;
#else
    uint8_t motor_type = FACTORY_MOTOR_TYPE_DEFAULT;
#endif

    switch (motor_type) {
    case FACTORY_MOTOR_TYPE_SHIGUANG:
        copy_preset_suction_table(suction_table, pump1_long_suction_table, pump1_short_suction_table,
                                  pump1_long_pump_freq_table);
        LOG_STORE("suction table: shiguang (motor_type=%u)", motor_type);
        break;
    case FACTORY_MOTOR_TYPE_LONGYI:
        copy_preset_suction_table(suction_table, pump2_long_suction_table, pump2_short_suction_table,
                                  pump2_long_pump_freq_table);
        LOG_STORE("suction table: longyi (motor_type=%u)", motor_type);
        break;
    default:
#if IS_USE_NEW_SUCTION
        copy_preset_suction_table(suction_table, pump1_long_suction_table, pump1_short_suction_table,
                                  pump1_long_pump_freq_table);
        LOG_STORE("suction table: default SHIGUANG (motor_type=%u) ", motor_type);
#else
        copy_preset_suction_table(suction_table, pump2_long_suction_table, pump2_short_suction_table,
                                  pump2_long_pump_freq_table);
        LOG_STORE("suction table: default longyi (motor_type=%u)", motor_type);
#endif
        break;
    }
    return 0;
}

int read_factory_deploy_flag_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    factory_flag_info_t *flag_info = &args.ctx->priv.factory_data.flag_info;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = flag_info->deploy_flag;
    return 0;
}

int read_factory_age_flag_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    if (args.arg1 < 0 || args.arg1 >= FACTORY_AGE_INFO_MAX) {
        LOGE("age index: %d, out of range", args.arg1);
        return -1;
    }
    factory_flag_info_t *flag_info = &args.ctx->priv.factory_data.flag_info;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = flag_info->age_info[args.arg1].age_flag;
    return 0;
}

int read_factory_age_time_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    if (args.arg1 < 0 || args.arg1 >= FACTORY_AGE_INFO_MAX) {
        LOGE("age index: %d, out of range", args.arg1);
        return -1;
    }
    factory_flag_info_t *flag_info = &args.ctx->priv.factory_data.flag_info;
    EXCEPTION_CHECK_INT(flag_info);
    uint32_t *tmp = (uint32_t *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = flag_info->age_info[args.arg1].age_time;
    return 0;
}

int read_factory_age_cycle_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    if (args.arg1 < 0 || args.arg1 >= FACTORY_AGE_INFO_MAX) {
        LOGE("age index: %d, out of range", args.arg1);
        return -1;
    }
    factory_flag_info_t *flag_info = &args.ctx->priv.factory_data.flag_info;
    EXCEPTION_CHECK_INT(flag_info);
    uint32_t *tmp = (uint32_t *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = flag_info->age_info[args.arg1].age_cycle;
    return 0;
}

int read_factory_age_display_flag_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    factory_flag_info_t *flag_info = &args.ctx->priv.factory_data.flag_info;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = flag_info->age_display;
    return 0;
}

int read_factory_suction_table_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    factory_suction_para_t **suction_table = (factory_suction_para_t **)args.val;
    *suction_table = &args.ctx->priv.factory_data.factory_suction_para;
    return 0;
}

int read_factory_motor_type_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    factory_dev_info_t *device_info = &args.ctx->priv.factory_data.dev_info;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    #if IS_SUPPORT_DEVICE_LIGHT_SENSOR_CALIB
    *tmp = device_info->motor_type;
    #else
    *tmp = FACTORY_MOTOR_TYPE_DEFAULT;
    #endif
    return 0;
}

int read_factory_battery_type_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    factory_dev_info_t *device_info = &args.ctx->priv.factory_data.dev_info;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    #if IS_SUPPORT_DEVICE_LIGHT_SENSOR_CALIB
    *tmp = device_info->battery_type;
    #else
    *tmp = FACTORY_BATTERY_TYPE_DEFAULT;
    #endif
    return 0;
}

#if IS_SUPPORT_DEVICE_FSENSOR
int read_factory_fsensor_cali_flag_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    factory_flag_info_t *flag_info = &args.ctx->priv.factory_data.flag_info;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = flag_info->fsensor_cali_flag;
    return 0;
}
#endif

int init_factory_persist_info(storage_manager_context_t *ctx)
{
    factory_data_store_t *factory_data = &ctx->priv.factory_data;
    store_func_args_t args = {
        .ctx = ctx,
        .val = NULL,
        .arg1 = 0,
        .arg2 = 0,
    };
    /// 初始化工厂设备信息
    args.val = &factory_data->dev_info;
    read_factory_dev_info_from_flash(args);
    /// 初始化工厂标志信息
    args.val = &factory_data->flag_info;
    read_factory_flag_info_from_flash(args);
#if IS_SUPPORT_DEVICE_IMU
    /// 初始化工厂imu校准数据
    args.val = &factory_data->imu;
    read_imu_cali_data_from_flash(args);
#endif
    /// 初始吸力参数表信息
    args.val = &factory_data->factory_suction_para;
    read_factory_suction_table_from_flash(args);
    return 0;
}
