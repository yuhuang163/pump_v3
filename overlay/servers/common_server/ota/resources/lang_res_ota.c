/*
* INCLUDE FILES
*****************************************************************************************
*/
#include "root_common_api.h" // IWYU pragma: keep
#include "common_tlvs.h" // IWYU pragma: keep
#include "lang_res_ota.h" // IWYU pragma: keep
#include "ui_lang_pack.h"

#if IS_SUPPORT_LANG_PACK

/*
* DEFINES
*****************************************************************************************
*/
#define OTA_TIME_PERIOD_MS                  (10)
#define __OTA_LANG_WRITE_TIMEOUT_TASK       "__ota_lang_write_timeout_task"

/*
* LOCAL VARIABLE DEFINITIONS
*****************************************************************************************
*/
static os_timer_id ota_timer_id = NULL;
static uint8_t ota_retry_count = 0;
static img_res_ota_info_t ota_info = {0};
static uint32_t ota_timestamp = 0;

/*
* LOCAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
static void lang_pack_notify_upgrade_error(ota_manager_context_t *ctx, uint16_t error_code)
{
    servers_publish_data_args_t args = {0};
    args.arg1 = error_code;
    args.arg2 = (int)ota_info.recv_size;
    ota_tlvs_upgrade_status_notify(&args);
    if (error_code == OTA_STATUS_CODE_DATA_BLOCK_TIMEOUT) {
        return;
    }

    if (ctx) {
        ctx->end_ota_upgrade(OTA_OBJECT_LANGUAGE_PACK, 0);
    }
    remove_task_handler(__OTA_LANG_WRITE_TIMEOUT_TASK);
    if (ota_timer_id != NULL) {
        STOP_TIMER(ota_timer_id);
        DELETE_TIMER(ota_timer_id);
        ota_timer_id = NULL;
    }
}

static void ota_lang_write_timeout_post_task_handler(hdr_post_args_t *args)
{
    (void)args;
    ota_manager_context_t *ctx = (ota_manager_context_t *)find_module(ID_MODULE_OTA);
    EXCEPTION_CHECK_VOID(ctx);
    if (!ctx->is_ota_upgrading()) {
        LOGE("not ota upgrading");
        return;
    }
    ota_info_t progress = {0};
    ctx->get_ota_info(&progress);
    if (progress.total_size > 0 && progress.recv_size >= progress.total_size) {
        return;
    }
    LOGE("timeout, retry_count: %d, recv=%u, total=%u", ota_retry_count, progress.recv_size,
         progress.total_size);
    protocol_manager_context_t *protocol_ctx = (protocol_manager_context_t *)find_module(ID_MODULE_PROTOCOL);
    if (protocol_ctx) {
        protocol_ctx->comm_protocol_assemble_reset();
    }
    ota_retry_count++;
    if (ota_retry_count < 3) {
        lang_pack_notify_upgrade_error(ctx, OTA_STATUS_CODE_DATA_BLOCK_TIMEOUT);
        return;
    }
    lang_pack_notify_upgrade_error(ctx, OTA_STATUS_CODE_FAIL);
    memset(&ota_info, 0, sizeof(ota_info));
}

static void on_write_lang_pack_data_cb(void *context, uint32_t offset_addr, uint8_t *data, uint16_t data_len)
{
    ota_manager_context_t *ctx = (ota_manager_context_t *)context;

    if (offset_addr + data_len > ota_info.total_size ||
        offset_addr + data_len > LANG_PACK_OTA_MAX_SIZE) {
        LOGE("out of range, offset=0x%x, len=%u", offset_addr, data_len);
        lang_pack_notify_upgrade_error(ctx, OTA_STATUS_CODE_DATA_SIZE_ERROR);
        return;
    }
    if (ui_lang_pack_write_at(offset_addr, data, data_len) != 0) {
        LOGE("write lang pack failed");
        lang_pack_notify_upgrade_error(ctx, OTA_STATUS_CODE_DATA_WRITE_ERROR);
        return;
    }
    uint32_t end_offset = offset_addr + data_len;
    if (end_offset > ota_info.recv_size) {
        ota_info.recv_size = end_offset;
    }
    if (ctx) {
        ctx->set_ota_upgrade_progress(OTA_OBJECT_LANGUAGE_PACK, ota_info.total_size, ota_info.recv_size);
    }
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    if (storage_ctx) {
        storage_ctx->put_store_ram_val(STORE_ID_LANG_PACK_OTA_INFO, 0, 0, &ota_info);
    }
    ota_timestamp = get_timestamp();
    ota_retry_count = 0;
}

static int lang_pack_integrity_check(ota_manager_context_t *ctx)
{
    if (ota_info.recv_size != ota_info.total_size) {
        LOGE("integrity check failed, recv=%u, total=%u", ota_info.recv_size, ota_info.total_size);
        lang_pack_notify_upgrade_error(ctx, OTA_STATUS_CODE_DATA_SIZE_ERROR);
        return -1;
    }

    uint8_t *buf = (uint8_t *)u_malloz(OTA_BLOCK_SIZE);
    if (buf == NULL) {
        LOGE("integrity check malloc failed");
        lang_pack_notify_upgrade_error(ctx, OTA_STATUS_CODE_FAIL);
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
        if (ui_lang_pack_read_at(verified_size, buf, read_len) != 0) {
            LOGE("integrity check read failed, offset=0x%x", verified_size);
            ret = -1;
            break;
        }
        CRC32_UPDATE(crc, buf, read_len);
        verified_size += read_len;
        if (verified_size % (OTA_BLOCK_SIZE * 10) == 0) {
            msleep(5);
        }
    }
    LOG_OTA("lang pack %d/%d", verified_size, ota_info.total_size);
    u_free(buf);
    if (ret != 0) {
        return ret;
    }

    crc ^= 0xFFFFFFFF;
    if (crc != ota_info.crc32) {
        LOGE("integrity check failed, crc=0x%x, expect=0x%x", crc, ota_info.crc32);
        if (ctx) {
            lang_pack_notify_upgrade_error(ctx, OTA_STATUS_CODE_DATA_CRC_ERROR);
        }
        return -1;
    }

    return 0;
}

static void ota_timeout_timer_handler(void *arg)
{
    (void)arg;
    if (get_timestamp_ms(ota_timestamp) > 1000) {
        POST_TASK_HANDLER_NO_DELAY(ota_lang_write_timeout_post_task_handler, NULL,
                                   __OTA_LANG_WRITE_TIMEOUT_TASK);
        ota_timestamp = get_timestamp();
    }
}

/*
* GLOBAL FUNCTION DEFINITIONS
*****************************************************************************************
*/
int start_lang_pack_upgrade(ota_img_info_t img_info, uint32_t *recv_size)
{
    ota_manager_context_t *ctx = (ota_manager_context_t *)find_module(ID_MODULE_OTA);
    EXCEPTION_CHECK_INT(ctx);
    EXCEPTION_CHECK_INT(recv_size);
    if (img_info.total_size == 0 || img_info.total_size > LANG_PACK_OTA_MAX_SIZE) {
        LOGE("invalid total_size=%u", img_info.total_size);
        return -1;
    }

    (void)ui_lang_pack_erase_ready();

    ota_info.img_id = img_info.img_id;
    ota_info.total_size = img_info.total_size;
    ota_info.crc32 = img_info.crc32;
    ota_info.version = img_info.version;
    storage_manager_context_t *storage_ctx = (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
    img_res_ota_info_t resume_info = {0};
    if (storage_ctx) {
        storage_ctx->get_store_ram_val(STORE_ID_LANG_PACK_OTA_INFO, 0, 0, &resume_info);
    }
    LOG_OTA("lang id:%d/%d; total:%d/%d; crc:%d/%d; version:%d/%d",
            img_info.img_id, resume_info.img_id,
            img_info.total_size, resume_info.total_size,
            img_info.crc32, resume_info.crc32,
            img_info.version, resume_info.version);

    ota_info.recv_size = resume_info.recv_size;
    if (memcmp(&ota_info, &resume_info, sizeof(img_res_ota_info_t)) == 0) {
        *recv_size = resume_info.recv_size;
        LOG_OTA("lang resume from offset:%d", *recv_size);
    } else {
        *recv_size = 0;
        ota_info.recv_size = 0;
        (void)ui_lang_pack_delete_pack();
        if (storage_ctx) {
            storage_ctx->put_store_ram_val(STORE_ID_LANG_PACK_OTA_INFO, 0, 0, &ota_info);
        }
        LOG_OTA("start new lang pack upgrade");
    }

    ctx->set_ota_upgrade_progress(OTA_OBJECT_LANGUAGE_PACK, ota_info.total_size, ota_info.recv_size);
    ctx->start_ota_upgrade(OTA_OBJECT_LANGUAGE_PACK, on_write_lang_pack_data_cb);
    ota_timestamp = get_timestamp();
    if (ota_timer_id == NULL) {
        CREATE_TIMER(&ota_timer_id, "ota_lang_timer", NULL, OS_TIMER_PERIODIC, ota_timeout_timer_handler);
    }
    if (ota_timer_id != NULL) {
        START_TIMER(ota_timer_id, OTA_TIME_PERIOD_MS);
    }
    ota_retry_count = 0;
    return 0;
}

int end_lang_pack_upgrade(uint32_t img_id)
{
    ota_manager_context_t *ctx = (ota_manager_context_t *)find_module(ID_MODULE_OTA);
    EXCEPTION_CHECK_INT(ctx);
    if (ota_timer_id != NULL) {
        STOP_TIMER(ota_timer_id);
        DELETE_TIMER(ota_timer_id);
        ota_timer_id = NULL;
    }
    ota_retry_count = 0;
    if (img_id != ota_info.img_id) {
        LOGE("img_id mismatch, expect=0x%x, got=0x%x", ota_info.img_id, img_id);
        ctx->end_ota_upgrade(OTA_OBJECT_LANGUAGE_PACK, 0);
        return -1;
    }

    int ret = lang_pack_integrity_check(ctx);
    if (ret != 0) {
        ctx->end_ota_upgrade(OTA_OBJECT_LANGUAGE_PACK, 0);
    } else {
        lang_pack_header_t hdr = {0};
        if (ui_lang_pack_read_at(0, &hdr, sizeof(hdr)) != 0 ||
            !ui_lang_pack_is_header_compatible(&hdr)) {
            LOGE("lang pack header invalid after crc");
            ctx->end_ota_upgrade(OTA_OBJECT_LANGUAGE_PACK, 0);
            ret = -1;
        } else if (ui_lang_pack_install_container() != 0) {
            LOGE("explode lang pack to FDS keys failed");
            ctx->end_ota_upgrade(OTA_OBJECT_LANGUAGE_PACK, 0);
            ret = -1;
        } else if (ui_lang_pack_write_ready(&hdr) != 0) {
            LOGE("write lang ready failed");
            ctx->end_ota_upgrade(OTA_OBJECT_LANGUAGE_PACK, 0);
            ret = -1;
        } else {
            ctx->end_ota_upgrade(OTA_OBJECT_LANGUAGE_PACK, 1);
            storage_manager_context_t *storage_ctx =
                (storage_manager_context_t *)find_module(ID_MODULE_STORAGE);
            if (storage_ctx) {
                ota_info.recv_size = ota_info.total_size;
                storage_ctx->put_store_ram_val(STORE_ID_LANG_PACK_OTA_INFO, 0, 0, &ota_info);
            }
        }
    }
    return ret;
}
#endif /* IS_SUPPORT_LANG_PACK */
