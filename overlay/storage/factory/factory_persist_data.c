/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "root_common_api.h" // IWYU pragma: keep
#include "factory_persist_data.h" // IWYU pragma: keep
#include "storage_manager.h" // IWYU pragma: keep
#include "storage_manager_internal.h" // IWYU pragma: keep

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
int storage_write_imu_cali_data(store_func_args_t *args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args);
    imu_cali_data_t *imu_cali = (imu_cali_data_t *)args->val;
    EXCEPTION_CHECK_INT(imu_cali);
    imu_cali_persist_data_t *store_imu = u_malloz(sizeof(imu_cali_persist_data_t));
    EXCEPTION_CHECK_INT(store_imu);
    store_imu->magic = STORAGE_IMU_CALIB_MAGIC_NUM;
    memcpy(&store_imu->imu_cali, imu_cali, sizeof(imu_cali_data_t));
    store_imu->crc = CRC16(&store_imu->imu_cali, sizeof(imu_cali_data_t));
    ret = user_database_put(args->ctx, STORE_ID_FACTORY_IMU_CALI_DATA, (uint8_t *)store_imu, sizeof(imu_cali_persist_data_t));
    if (ret == 0) { 
        imu_cali_data_t *ram_imu = &args->ctx->priv.factory_data.imu;
        memcpy(ram_imu, &store_imu->imu_cali, sizeof(imu_cali_data_t));
    } else {
        LOGE("write imu cali failure! ret: %d", ret);
    }
    u_free(store_imu);
    
    return ret;
}

/// 从flash中读取imu校准数据
int storage_read_imu_cali_data(store_func_args_t *args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args);
    imu_cali_data_t *imu_cali = (imu_cali_data_t *)args->val;
    EXCEPTION_CHECK_INT(imu_cali);
    imu_cali_persist_data_t *store_imu = u_malloz(sizeof(imu_cali_persist_data_t));
    EXCEPTION_CHECK_INT(store_imu);
    ret = user_database_get(args->ctx, STORE_ID_FACTORY_IMU_CALI_DATA, (uint8_t *)store_imu, sizeof(imu_cali_persist_data_t));
    uint16_t crc = CRC16(&store_imu->imu_cali, sizeof(imu_cali_data_t));
    if (ret == 0 && store_imu->magic == STORAGE_IMU_CALIB_MAGIC_NUM && store_imu->crc == crc) {
        memcpy(imu_cali, &store_imu->imu_cali, sizeof(imu_cali_data_t));
    } else {
        /// 如果读取失败，尝试从旧的系统信息中读取
        if (args->ctx->hook->read_old_sys_info) {
            ret = args->ctx->hook->read_old_sys_info(args->ctx, STORE_ID_FACTORY_IMU_CALI_DATA, (void *)imu_cali);
            if (ret == 0) {
                storage_write_imu_cali_data(args);
                goto CPL_EXIT;
            }
        }
        memset(imu_cali, 0, sizeof(imu_cali_data_t));
        LOGE("read imu cali failure! ret: %d, magic: %d, crc: %d, expect_crc: %d", ret, store_imu->magic, store_imu->crc, crc);
    }
CPL_EXIT:
    u_free(store_imu);

    return ret;
}
#endif

/// 写入工厂部署标志到flash
int storage_write_factory_deploy_flag(store_func_args_t *args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args);
    factory_flag_info_t *flag_info = &args->ctx->priv.factory_data.flag_info;
    flag_info->deploy_flag = (uint8_t)args->arg1;
    factory_flag_persist_data_t *store_flag = u_malloz(sizeof(factory_flag_persist_data_t));
    EXCEPTION_CHECK_INT(store_flag);
    store_flag->magic = STORAGE_FAC_FLAG_MAGIC_NUM;
    memcpy(&store_flag->flag_info, flag_info, sizeof(factory_flag_info_t));
    store_flag->crc = CRC16(&store_flag->flag_info, sizeof(factory_flag_info_t));
    ret = user_database_put(args->ctx, STORE_ID_FACTORY_FLAG_INFO, (uint8_t *)store_flag, sizeof(factory_flag_persist_data_t));
    if (ret != 0) {
        LOGE("write factory deploy flag failure! ret: %d", ret);
    }
    u_free(store_flag);

    return ret;
}

/// 写入工厂老化标志到flash
int storage_write_factory_age_flag(store_func_args_t *args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args);
    factory_flag_info_t *flag_info = &args->ctx->priv.factory_data.flag_info;
    flag_info->age_flag = (uint8_t)args->arg1;
    factory_flag_persist_data_t *store_flag = u_malloz(sizeof(factory_flag_persist_data_t));
    EXCEPTION_CHECK_INT(store_flag);
    store_flag->magic = STORAGE_FAC_FLAG_MAGIC_NUM;
    memcpy(&store_flag->flag_info, flag_info, sizeof(factory_flag_info_t));
    store_flag->crc = CRC16(&store_flag->flag_info, sizeof(factory_flag_info_t));
    ret = user_database_put(args->ctx, STORE_ID_FACTORY_FLAG_INFO, (uint8_t *)store_flag, sizeof(factory_flag_persist_data_t));
    if (ret != 0) {
        LOGE("write factory age flag failure! ret: %d", ret);
    }
    u_free(store_flag);

    return ret;
}

/// 写入工厂老化展示标志到flash
int storage_write_factory_age_display_flag(store_func_args_t *args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args);
    factory_flag_info_t *flag_info = &args->ctx->priv.factory_data.flag_info;
    flag_info->age_display = (uint8_t)args->arg1;
    factory_flag_persist_data_t *store_flag = u_malloz(sizeof(factory_flag_persist_data_t));
    EXCEPTION_CHECK_INT(store_flag);
    store_flag->magic = STORAGE_FAC_FLAG_MAGIC_NUM;
    memcpy(&store_flag->flag_info, flag_info, sizeof(factory_flag_info_t));
    store_flag->crc = CRC16(&store_flag->flag_info, sizeof(factory_flag_info_t));
    ret = user_database_put(args->ctx, STORE_ID_FACTORY_FLAG_INFO, (uint8_t *)store_flag, sizeof(factory_flag_persist_data_t));
    if (ret != 0) {
        LOGE("write factory age display flag failure! ret: %d", ret);
    }
    u_free(store_flag);

    return ret; 
}

#if IS_SUPPORT_DEVICE_FSENSOR
/// 写入工厂fsensor校准标志到flash
int storage_write_factory_fsensor_cali_flag(store_func_args_t *args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args);
    factory_flag_info_t *flag_info = &args->ctx->priv.factory_data.flag_info;
    flag_info->fsensor_cali_flag = (uint8_t)args->arg1;
    factory_flag_persist_data_t *store_flag = u_malloz(sizeof(factory_flag_persist_data_t));
    EXCEPTION_CHECK_INT(store_flag);
    store_flag->magic = STORAGE_FAC_FLAG_MAGIC_NUM;
    memcpy(&store_flag->flag_info, flag_info, sizeof(factory_flag_info_t));
    store_flag->crc = CRC16(&store_flag->flag_info, sizeof(factory_flag_info_t));
    ret = user_database_put(args->ctx, STORE_ID_FACTORY_FLAG_INFO, (uint8_t *)store_flag, sizeof(factory_flag_persist_data_t));
    if (ret != 0) {
        LOGE("write factory fsensor cali flag failure! ret: %d", ret);
    }
    u_free(store_flag);

    return ret;
}
#endif

/// 写入工厂标志信息到flash
int storage_write_factory_flag_info(store_func_args_t *args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args);
    factory_flag_info_t *flag_info = (factory_flag_info_t *)args->val;
    EXCEPTION_CHECK_INT(flag_info);
    factory_flag_persist_data_t *store_flag = u_malloz(sizeof(factory_flag_persist_data_t));
    EXCEPTION_CHECK_INT(store_flag);
    store_flag->magic = STORAGE_FAC_FLAG_MAGIC_NUM;
    memcpy(&store_flag->flag_info, flag_info, sizeof(factory_flag_info_t));
    store_flag->crc = CRC16(&store_flag->flag_info, sizeof(factory_flag_info_t));
    ret = user_database_put(args->ctx, STORE_ID_FACTORY_FLAG_INFO, (uint8_t *)store_flag, sizeof(factory_flag_persist_data_t));
    if (ret != 0) {
        LOGE("write factory flag info failure! ret: %d", ret);
    }
    u_free(store_flag);

    return ret;
}

/// 从flash中读取工厂标志信息
int storage_read_factory_flag_info(store_func_args_t *args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args);
    factory_flag_info_t *flag_info = (factory_flag_info_t *)args->val;
    EXCEPTION_CHECK_INT(flag_info);
    factory_flag_persist_data_t *store_flag = u_malloz(sizeof(factory_flag_persist_data_t));
    EXCEPTION_CHECK_INT(store_flag);
    ret = user_database_get(args->ctx, STORE_ID_FACTORY_FLAG_INFO, (uint8_t *)store_flag, sizeof(factory_flag_persist_data_t));
    uint16_t crc = CRC16(&store_flag->flag_info, sizeof(factory_flag_info_t));
    if (ret == 0 && store_flag->magic == STORAGE_FAC_FLAG_MAGIC_NUM && store_flag->crc == crc) {
        memcpy(flag_info, &store_flag->flag_info, sizeof(factory_flag_info_t));
    } else {
        /// 如果读取失败，尝试从旧的系统信息中读取
        if (args->ctx->hook->read_old_sys_info) {
            ret = args->ctx->hook->read_old_sys_info(args->ctx, STORE_ID_FACTORY_FLAG_INFO, (void *)flag_info);
            if (ret == 0) {
                storage_write_factory_flag_info(args);
                goto CPL_EXIT;
            }
        }
        memset(flag_info, 0, sizeof(factory_flag_info_t));
        LOGE("read factory flag failure! ret: %d, magic: %d, crc: %d, expect_crc: %d", ret, store_flag->magic, store_flag->crc, crc);
    }
CPL_EXIT:
    u_free(store_flag);

    return ret;
}

/// 写入工厂设备信息到flash
int storage_write_factory_dev_info(store_func_args_t *args)
{
    EXCEPTION_CHECK_INT(args);
    return 0; 
}

/// 从flash中读取工厂设备信息到ram
int storage_read_factory_dev_info(store_func_args_t *args)
{
    return 0;
}

/**
 * @brief 从ram中读取工厂标志信息
 */
int storage_read_factory_deploy_flag(store_func_args_t *args)
{
    EXCEPTION_CHECK_INT(args);
    factory_flag_info_t *flag_info = &args->ctx->priv.factory_data.flag_info;
    return flag_info->deploy_flag;
}

int storage_read_factory_age_flag(store_func_args_t *args)
{
    EXCEPTION_CHECK_INT(args);
    factory_flag_info_t *flag_info = &args->ctx->priv.factory_data.flag_info;
    return flag_info->age_flag;
}

int storage_read_factory_age_display_flag(store_func_args_t *args)
{
    EXCEPTION_CHECK_INT(args);
    factory_flag_info_t *flag_info = &args->ctx->priv.factory_data.flag_info;
    return flag_info->age_display;
}

#if IS_SUPPORT_DEVICE_FSENSOR
int storage_read_factory_fsensor_cali_flag(store_func_args_t *args)
{
    EXCEPTION_CHECK_INT(args);
    factory_flag_info_t *flag_info = &args->ctx->priv.factory_data.flag_info;
    return flag_info->fsensor_cali_flag;
}
#endif

/// 初始化工厂信息
int init_factory_persist_info(storage_manager_context_t *ctx)
{
    factory_data_store_t *factory_data = &ctx->priv.factory_data;
    store_func_args_t args = {
        .ctx = ctx,
        .arg1 = 0,
        .arg2 = 0,
        .val = NULL,
    };
    /// 初始化工厂设备信息
    storage_read_factory_dev_info(&args);

    /// 初始化工厂标志信息
    storage_read_factory_flag_info(&args);
#if IS_SUPPORT_DEVICE_IMU
    /// 初始化工厂imu校准数据
    storage_read_imu_cali_data(&args);
#endif

    return 0;
}
