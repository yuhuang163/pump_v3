/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "common_tlvs.h" // IWYU pragma: keep
#include "img_res_ota.h" // IWYU pragma: keep

/*
* DEFINES
*****************************************************************************************
*/

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static img_res_ota_info_t ota_info = {0};
static uint32_t img_res_last_erased_addr = UINT32_MAX;

/*
* GLOBAL VARIABLE DEFINITIONS
*****************************************************************************************
*/

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static int img_res_erase_sectors_if_needed(uint32_t start_addr, uint16_t data_len)
{
    uint32_t sector_addr = start_addr & ~(EXTERNAL_FLASH_SECTOR_SIZE - 1); /// 按扇区对齐
    uint32_t end_sector_addr = (start_addr + data_len - 1) & ~(EXTERNAL_FLASH_SECTOR_SIZE - 1); /// 按扇区对齐
    for (; sector_addr <= end_sector_addr; sector_addr += EXTERNAL_FLASH_SECTOR_SIZE) {
        if (img_res_last_erased_addr == UINT32_MAX || sector_addr > img_res_last_erased_addr) {
            if (platform_flash_erase(PARTITION_IMG_EXT_DATA_NAME, sector_addr,
                                     EXTERNAL_FLASH_SECTOR_SIZE) != 0) {
                LOGE("img res ota erase sector failed, addr=0x%x", sector_addr);
                return -1;
            }
            img_res_last_erased_addr = sector_addr;
        }
    }
    return 0;
}

static void img_res_notify_upgrade_error(uint16_t error_code)
{
    servers_publish_data_args_t args = {0};
    args.arg1 = error_code;
    ota_tlvs_upgrade_status_notify(&args);
}

static void on_write_img_data_cb(void *context, uint32_t offset_addr, uint8_t *data, uint16_t data_len)
{
    ota_manager_context_t *ctx = (ota_manager_context_t *)context;
    if (offset_addr + data_len > THEME_IMG_RES_SIZE) {
        LOGE("out of range, offset=0x%x, len=%u", offset_addr, data_len);
        return;
    }
    uint32_t start_addr = THEME_IMG_RES_START_ADDR + offset_addr;
    if (img_res_erase_sectors_if_needed(start_addr, data_len) != 0) {
        if (ctx) {
            ctx->end_ota_upgrade(OTA_OBJECT_IMAGE_RESOURCE, 0);
            img_res_notify_upgrade_error(OTA_STATUS_CODE_DATA_WRITE_ERROR);
        }
        return;
    }
    int ret = platform_flash_write(PARTITION_IMG_EXT_DATA_NAME, start_addr, data, data_len);
    if (ctx) {
        if (ret != 0) {
            ctx->end_ota_upgrade(OTA_OBJECT_IMAGE_RESOURCE, 0);
            img_res_notify_upgrade_error(OTA_STATUS_CODE_DATA_WRITE_ERROR);
        } else {
            uint32_t end_offset = offset_addr + data_len;
            if (end_offset > ota_info.recv_size) {
                ota_info.recv_size = end_offset;
            }
            ctx->set_ota_upgrade_progress(OTA_OBJECT_IMAGE_RESOURCE, ota_info.total_size, ota_info.recv_size);
            storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
            if (storage_ctx) {
                storage_ctx->put_store_ram_val(STORE_ID_IMG_RES_OTA_INFO, 0, 0, &ota_info);
            }
        }
    }
}

/// 完整性校验
static int img_res_integrity_check(ota_manager_context_t *ctx)
{
    if (ota_info.recv_size != ota_info.total_size) {
        LOGE("integrity check failed, recv=%u, total=%u",
             ota_info.recv_size, ota_info.total_size);
        if (ctx) {
            img_res_notify_upgrade_error(OTA_STATUS_CODE_DATA_SIZE_ERROR);
        }
        return -1;
    }

    uint8_t *buf = (uint8_t *)u_malloz(OTA_BLOCK_SIZE);
    if (buf == NULL) {
        LOGE("integrity check malloc failed");
        return -1;
    }

    uint32_t crc = 0xFFFFFFFF;
    uint32_t verified_size = 0;
    int ret = 0;

    while (verified_size < ota_info.total_size) {
        uint32_t read_len = ota_info.total_size - verified_size;
        if (read_len > OTA_BLOCK_SIZE) {
            read_len = OTA_BLOCK_SIZE;
        }

        if (platform_flash_read(PARTITION_IMG_EXT_DATA_NAME,
                                THEME_IMG_RES_START_ADDR + verified_size,
                                buf, read_len) != 0) {
            LOGE("integrity check read failed, offset=0x%x", verified_size);
            ret = -1;
            break;
        }

        CRC32_UPDATE(crc, buf, read_len);
        verified_size += read_len;
        if (verified_size % (OTA_BLOCK_SIZE * 10) == 0) {
            msleep(5); /// 留喂狗时间
        }
    }

    u_free(buf);

    if (ret != 0) {
        return ret;
    }

    crc ^= 0xFFFFFFFF;
    if (crc != ota_info.crc32) {
        LOGE("integrity check failed, crc=0x%x, expect=0x%x", crc, ota_info.crc32);
        if (ctx) {
            img_res_notify_upgrade_error(OTA_STATUS_CODE_DATA_CRC_ERROR);
        }
        return -1;
    }

    return 0;
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int start_img_res_upgrade(ota_img_info_t img_info, uint32_t *recv_size)
{
    ota_manager_context_t *ctx = (ota_manager_context_t *)find_module(ID_MODULE_OTA);
    EXCEPTION_CHECK_INT(ctx);
    if (img_info.total_size == 0 || img_info.total_size > THEME_IMG_RES_SIZE) {
        LOGE("invalid total_size=%u", img_info.total_size);
        return -1;
    }
    ota_info.img_id = img_info.img_id;
    ota_info.total_size = img_info.total_size;
    ota_info.crc32 = img_info.crc32;
    ota_info.version = img_info.version;
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    img_res_ota_info_t img_res_ota_info = {0};
    if (storage_ctx) {
        storage_ctx->get_store_ram_val(STORE_ID_IMG_RES_OTA_INFO, 0, 0, &img_res_ota_info);
    }

    ota_info.recv_size =  img_res_ota_info.recv_size;
    if (memcmp(&ota_info, &img_res_ota_info, sizeof(img_res_ota_info_t)) == 0) {
        *recv_size = img_res_ota_info.recv_size; /// 断点续传
    } else {
        *recv_size = 0; /// 重新开始
        ota_info.recv_size = 0;
        if (storage_ctx) {
            storage_ctx->put_store_ram_val(STORE_ID_IMG_RES_OTA_INFO, 0, 0, &ota_info);
        }
    }

    if (*recv_size > 0) {
        img_res_last_erased_addr = THEME_IMG_RES_START_ADDR + ((*recv_size - 1) / EXTERNAL_FLASH_SECTOR_SIZE) *
                    EXTERNAL_FLASH_SECTOR_SIZE; /// 按扇区对齐
    } else {
        img_res_last_erased_addr = UINT32_MAX; /// 重新开始
    }

    ctx->start_ota_upgrade(OTA_OBJECT_IMAGE_RESOURCE, on_write_img_data_cb);
    
    return 0;
}

int end_img_res_upgrade(uint32_t img_id)
{
    ota_manager_context_t *ctx = (ota_manager_context_t *)find_module(ID_MODULE_OTA);
    EXCEPTION_CHECK_INT(ctx);
    if (img_id != ota_info.img_id) {
        LOGE("img_id mismatch, expect=0x%x, got=0x%x", ota_info.img_id, img_id);
        ctx->end_ota_upgrade(OTA_OBJECT_IMAGE_RESOURCE, 0);
        return -1;
    }

    int ret = img_res_integrity_check(ctx);
    if (ret != 0) {
        ctx->end_ota_upgrade(OTA_OBJECT_IMAGE_RESOURCE, 0);
    } else {
        ctx->end_ota_upgrade(OTA_OBJECT_IMAGE_RESOURCE, 1);
        storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
        if (storage_ctx) {
            img_res_ota_info_t img_res_ota_info = {0};
            img_res_ota_info.img_id = ota_info.img_id;
            img_res_ota_info.total_size = ota_info.total_size;
            img_res_ota_info.version = ota_info.version;
            img_res_ota_info.crc32 = ota_info.crc32;
            img_res_ota_info.recv_size = ota_info.total_size;
            storage_ctx->put_store_ram_val(STORE_ID_IMG_RES_OTA_INFO, 0, 0, &img_res_ota_info);
        }
    }
    img_res_last_erased_addr = UINT32_MAX;
    return ret;
}
