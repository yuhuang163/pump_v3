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

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int write_img_res_ota_info_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    img_res_ota_info_t *img_res_ota_info = (img_res_ota_info_t *)args.val;
    EXCEPTION_CHECK_INT(img_res_ota_info);
    img_res_ota_storage_t *store_img_res_ota_info = u_malloz(sizeof(img_res_ota_storage_t));
    EXCEPTION_CHECK_INT(store_img_res_ota_info);
    store_img_res_ota_info->magic = STORAGE_IMG_RES_OTA_MAGIC_NUM;
    memcpy(&store_img_res_ota_info->res_ota_info, img_res_ota_info, sizeof(img_res_ota_info_t));
    store_img_res_ota_info->crc = CRC16(&store_img_res_ota_info->res_ota_info, sizeof(img_res_ota_info_t));
    ret = user_database_put(args.ctx, PERSIST_KEY_IMG_RES_OTA_INFO, (uint8_t *)store_img_res_ota_info,
        sizeof(img_res_ota_storage_t));
    if (ret != 0) {
        LOGE("write img res ota info failure! ret: %d", ret);
    }
    u_free(store_img_res_ota_info);

    return ret;
}

void reset_img_res_ota_info(storage_manager_context_t *ctx, img_res_ota_info_t *img_res_ota_info)
{
    EXCEPTION_CHECK_VOID(img_res_ota_info);
    memset(img_res_ota_info, 0, sizeof(img_res_ota_info_t));
    store_func_args_t args = {
        .ctx = ctx,
        .arg1 = 0,
        .arg2 = 0,
        .val = img_res_ota_info,
    };
    write_img_res_ota_info_to_flash(args);
}

int read_img_res_ota_info_from_flash(store_func_args_t args)
{   
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    img_res_ota_info_t *img_res_ota_info = (img_res_ota_info_t *)args.val;
    EXCEPTION_CHECK_INT(img_res_ota_info);
    img_res_ota_storage_t *store_img_res_ota_info = u_malloz(sizeof(img_res_ota_storage_t));
    EXCEPTION_CHECK_INT(store_img_res_ota_info);
    ret = user_database_get(args.ctx, PERSIST_KEY_IMG_RES_OTA_INFO, (uint8_t *)store_img_res_ota_info,
        sizeof(img_res_ota_storage_t));
    uint16_t crc = CRC16(&store_img_res_ota_info->res_ota_info, sizeof(img_res_ota_info_t));
    if (ret == 0 && store_img_res_ota_info->magic == STORAGE_IMG_RES_OTA_MAGIC_NUM && store_img_res_ota_info->crc == crc) {
        memcpy(img_res_ota_info, &store_img_res_ota_info->res_ota_info, sizeof(img_res_ota_info_t));
    } else {
        reset_img_res_ota_info(args.ctx, img_res_ota_info);
        LOGE("read img res ota info failure! ret: %d, magic: %d, crc: %d, expect_crc: %d", ret,
              store_img_res_ota_info->magic, store_img_res_ota_info->crc, crc);
    }
    u_free(store_img_res_ota_info);
    return ret;
}

int write_img_res_ota_info_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    img_res_ota_info_t *img_res_ota_info = (img_res_ota_info_t *)args.val;
    EXCEPTION_CHECK_INT(img_res_ota_info);
    memcpy(&args.ctx->priv.img_res_ota_info, img_res_ota_info, sizeof(img_res_ota_info_t));
    return 0;
}

int read_img_res_ota_info_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    img_res_ota_info_t *img_res_ota_info = (img_res_ota_info_t *)args.val;
    EXCEPTION_CHECK_INT(img_res_ota_info);
    memcpy(img_res_ota_info, &args.ctx->priv.img_res_ota_info, sizeof(img_res_ota_info_t));
    return 0;
}

#if IS_SUPPORT_LANG_PACK
int write_lang_pack_ota_info_to_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    img_res_ota_info_t *lang_pack_ota_info = (img_res_ota_info_t *)args.val;
    EXCEPTION_CHECK_INT(lang_pack_ota_info);
    lang_pack_ota_storage_t *store_info = u_malloz(sizeof(lang_pack_ota_storage_t));
    EXCEPTION_CHECK_INT(store_info);
    store_info->magic = STORAGE_LANG_PACK_OTA_MAGIC_NUM;
    memcpy(&store_info->res_ota_info, lang_pack_ota_info, sizeof(img_res_ota_info_t));
    store_info->crc = CRC16(&store_info->res_ota_info, sizeof(img_res_ota_info_t));
    ret = user_database_put(args.ctx, PERSIST_KEY_LANG_PACK_OTA_INFO, (uint8_t *)store_info,
                            sizeof(lang_pack_ota_storage_t));
    if (ret != 0) {
        LOGE("write lang pack ota info failure! ret: %d", ret);
    }
    u_free(store_info);
    return ret;
}

void reset_lang_pack_ota_info(storage_manager_context_t *ctx, img_res_ota_info_t *lang_pack_ota_info)
{
    EXCEPTION_CHECK_VOID(lang_pack_ota_info);
    memset(lang_pack_ota_info, 0, sizeof(img_res_ota_info_t));
    store_func_args_t args = {
        .ctx = ctx,
        .arg1 = 0,
        .arg2 = 0,
        .val = lang_pack_ota_info,
    };
    write_lang_pack_ota_info_to_flash(args);
}

int read_lang_pack_ota_info_from_flash(store_func_args_t args)
{
    int ret = 0;
    EXCEPTION_CHECK_INT(args.ctx);
    img_res_ota_info_t *lang_pack_ota_info = (img_res_ota_info_t *)args.val;
    EXCEPTION_CHECK_INT(lang_pack_ota_info);
    lang_pack_ota_storage_t *store_info = u_malloz(sizeof(lang_pack_ota_storage_t));
    EXCEPTION_CHECK_INT(store_info);
    ret = user_database_get(args.ctx, PERSIST_KEY_LANG_PACK_OTA_INFO, (uint8_t *)store_info,
                            sizeof(lang_pack_ota_storage_t));
    uint16_t crc = CRC16(&store_info->res_ota_info, sizeof(img_res_ota_info_t));
    if (ret == 0 && store_info->magic == STORAGE_LANG_PACK_OTA_MAGIC_NUM && store_info->crc == crc) {
        memcpy(lang_pack_ota_info, &store_info->res_ota_info, sizeof(img_res_ota_info_t));
    } else {
        reset_lang_pack_ota_info(args.ctx, lang_pack_ota_info);
        LOGE("read lang pack ota info failure! ret: %d", ret);
    }
    u_free(store_info);
    return ret;
}

int write_lang_pack_ota_info_to_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    img_res_ota_info_t *lang_pack_ota_info = (img_res_ota_info_t *)args.val;
    EXCEPTION_CHECK_INT(lang_pack_ota_info);
    memcpy(&args.ctx->priv.lang_pack_ota_info, lang_pack_ota_info, sizeof(img_res_ota_info_t));
    return 0;
}

int read_lang_pack_ota_info_from_ram(store_func_args_t args)
{
    EXCEPTION_CHECK_INT(args.ctx);
    img_res_ota_info_t *lang_pack_ota_info = (img_res_ota_info_t *)args.val;
    EXCEPTION_CHECK_INT(lang_pack_ota_info);
    memcpy(lang_pack_ota_info, &args.ctx->priv.lang_pack_ota_info, sizeof(img_res_ota_info_t));
    return 0;
}
#endif /* IS_SUPPORT_LANG_PACK */
