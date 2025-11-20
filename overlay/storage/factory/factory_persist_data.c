/*
 * INCLUDE FILES
 *****************************************************************************************
 */
#include "root_common_api.h" // IWYU pragma: keep
#include "storage_manager.h" // IWYU pragma: keep
#include "storage_manager_api.h"
#include "storage_manager_internal.h" // IWYU pragma: keep
#include "factory_persist_data.h" // IWYU pragma: keep

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
    factory_flag_info_t *flag_info = &args.ctx->priv.factory_data.flag_info;
    flag_info->age_flag = (uint8_t)args.arg1;
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
        memset(flag_info, 0, sizeof(factory_flag_info_t));
        LOGE("read factory flag failure! ret: %d, magic: %d, crc: %d, expect_crc: %d", ret, store_flag->magic, store_flag->crc, crc);
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
    ret = user_database_put(args.ctx, STORE_ID_FACTORY_DEV_INFO, (uint8_t *)store_flag,
        sizeof(factory_dev_info_storage_t));
    if (ret != 0) {
        LOGE("write factory device_info failure! ret: %d", ret);
    }
    u_free(store_flag);
    return 0; 
}

int read_factory_dev_info_from_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    factory_dev_info_t *device_info = (factory_dev_info_t *)args.val;
    EXCEPTION_CHECK_INT(device_info);
    factory_dev_info_storage_t *store_flag = u_malloz(sizeof(factory_dev_info_storage_t));
    EXCEPTION_CHECK_INT(store_flag);
    ret = user_database_get(args.ctx, STORE_ID_FACTORY_DEV_INFO, (uint8_t *)store_flag,
        sizeof(factory_dev_info_storage_t));
    uint16_t crc = CRC16(&store_flag->dev_info, sizeof(factory_dev_info_t));
    if (ret == 0 && store_flag->magic == STORAGE_FAC_DEVINFO_MAGIC_NUM && store_flag->crc == crc) {
        memcpy(device_info, &store_flag->dev_info, sizeof(factory_dev_info_t));
    } else {
        memset(device_info, 0, sizeof(factory_dev_info_t));
        LOGE("read factory device_info failure! ret: %d, magic: %d, crc: %d, expect_crc: %d", ret, store_flag->magic, store_flag->crc, crc);
    }
    u_free(store_flag);
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
    factory_flag_info_t *flag_info = &args.ctx->priv.factory_data.flag_info;
    int *tmp = (int *)args.val;
    EXCEPTION_CHECK_INT(tmp);
    *tmp = flag_info->age_flag;
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
    return 0;
}
